#include "movie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !defined(_WIN32)
#include <sys/types.h>
#endif

#include <SDL3/SDL.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "files.h"

#include "oglfunc.h"

extern SDL_Window *window;
extern SDL_Surface *surface;
extern void FlipBuffers(void);
extern unsigned char GotAnyKey;
extern int DebouncedGotAnyKey;
extern unsigned char KeyboardInput[];

struct AVPMovie
{
    AVFormatContext *format;
    FILE *file;
    AVIOContext *io;
    unsigned char *io_buffer;
    AVCodecContext *video;
    AVCodecContext *audio;
    struct SwsContext *sws;
    struct SwrContext *swr;
    AVFrame *frame;
    AVPacket *packet;

    int video_stream;
    int audio_stream;
    int width;
    int height;
    int64_t frame_count;
    AVRational video_time_base;
    AVRational video_rate;
    int frame_number;
    int eof;

    SDL_AudioStream *audio_stream_handle;
    SDL_AudioDeviceID audio_device;
    int audio_enabled;
    uint8_t *audio_buffer;
    int audio_buffer_size;

    int64_t last_video_pts_ms;
};

static int movie_read(void *opaque, uint8_t *buf, int buf_size)
{
    FILE *file = (FILE *)opaque;
    size_t n = fread(buf, 1, (size_t)buf_size, file);
    if (n == 0)
    {
        if (feof(file))
            return AVERROR_EOF;
        return AVERROR(EIO);
    }
    return (int)n;
}

static int64_t movie_seek(void *opaque, int64_t offset, int whence)
{
    FILE *file = (FILE *)opaque;
    int origin = whence & 0xFFFF;
    long long pos;

    if (whence & AVSEEK_SIZE)
    {
#if defined(_WIN32)
        __int64 old = _ftelli64(file);
        if (_fseeki64(file, 0, SEEK_END) != 0)
            return -1;
        pos = _ftelli64(file);
        _fseeki64(file, old, SEEK_SET);
#else
        off_t old = ftello(file);
        if (fseeko(file, 0, SEEK_END) != 0)
            return -1;
        pos = (long long)ftello(file);
        fseeko(file, old, SEEK_SET);
#endif
        return pos;
    }

#if defined(_WIN32)
    if (_fseeki64(file, offset, origin) != 0)
        return -1;
    return _ftelli64(file);
#else
    if (fseeko(file, (off_t)offset, origin) != 0)
        return -1;
    return (int64_t)ftello(file);
#endif
}

static int open_codec(AVFormatContext *format, int stream_index, AVCodecContext **out)
{
    AVStream *stream = format->streams[stream_index];
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext *ctx;

    if (!codec)
        return 0;

    ctx = avcodec_alloc_context3(codec);
    if (!ctx)
        return 0;

    if (avcodec_parameters_to_context(ctx, stream->codecpar) < 0 ||
        avcodec_open2(ctx, codec, NULL) < 0)
    {
        avcodec_free_context(&ctx);
        return 0;
    }

    *out = ctx;
    return 1;
}

static void close_audio(AVPMovie *movie)
{
    if (movie->audio_stream_handle)
    {
        SDL_DestroyAudioStream(movie->audio_stream_handle);
        movie->audio_stream_handle = NULL;
        movie->audio_device = 0;
    }
    if (movie->swr)
    {
        swr_free(&movie->swr);
    }
    free(movie->audio_buffer);
    movie->audio_buffer = NULL;
    movie->audio_buffer_size = 0;
}

static int init_audio(AVPMovie *movie)
{
    SDL_AudioSpec dst_spec;

    if (!movie->audio || !movie->audio_enabled)
        return 1;

    memset(&dst_spec, 0, sizeof(dst_spec));
    dst_spec.format = SDL_AUDIO_S16;
    dst_spec.channels = 2;
    dst_spec.freq = 48000;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && !SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        fprintf(stderr, "FMV: SDL audio init failed: %s\n", SDL_GetError());
        return 0;
    }

    movie->audio_stream_handle = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &dst_spec, NULL, NULL);
    if (!movie->audio_stream_handle)
    {
        fprintf(stderr, "FMV: unable to open audio stream: %s\n", SDL_GetError());
        return 0;
    }

    movie->audio_device = SDL_GetAudioStreamDevice(movie->audio_stream_handle);
    if (!SDL_ResumeAudioStreamDevice(movie->audio_stream_handle))
    {
        fprintf(stderr, "FMV: unable to start audio stream: %s\n", SDL_GetError());
        close_audio(movie);
        return 0;
    }

    {
        AVChannelLayout out_layout = AV_CHANNEL_LAYOUT_STEREO;
        AVChannelLayout in_layout;

        av_channel_layout_default(&in_layout, movie->audio->ch_layout.nb_channels);
        if (movie->audio->ch_layout.nb_channels > 0)
            av_channel_layout_copy(&in_layout, &movie->audio->ch_layout);

        if (swr_alloc_set_opts2(&movie->swr,
                                &out_layout, AV_SAMPLE_FMT_S16, 48000,
                                &in_layout, movie->audio->sample_fmt,
                                movie->audio->sample_rate, 0, NULL) < 0 ||
            !movie->swr || swr_init(movie->swr) < 0)
        {
            av_channel_layout_uninit(&in_layout);
            fprintf(stderr, "FMV: unable to initialise audio resampler\n");
            close_audio(movie);
            return 0;
        }
        av_channel_layout_uninit(&in_layout);
    }

    return 1;
}

AVPMovie *AVPMovie_Open(const char *filename, int with_audio)
{
    AVPMovie *movie;
    AVStream *stream;
    int i;

    movie = (AVPMovie *)calloc(1, sizeof(*movie));
    if (!movie)
        return NULL;

    movie->video_stream = -1;
    movie->audio_stream = -1;
    movie->audio_enabled = with_audio;
    movie->frame = av_frame_alloc();
    movie->packet = av_packet_alloc();
    if (!movie->frame || !movie->packet)
        goto fail;

    movie->file = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_PERM);
    if (!movie->file)
    {
        fprintf(stderr, "FMV: game file not found: %s\n", filename);
        goto fail;
    }

    movie->format = avformat_alloc_context();
    if (!movie->format)
        goto fail;

    movie->io_buffer = (unsigned char *)av_malloc(32 * 1024);
    if (!movie->io_buffer)
        goto fail;

    movie->io = avio_alloc_context(movie->io_buffer, 32 * 1024, 0, movie->file,
                                   movie_read, NULL, movie_seek);
    if (!movie->io)
        goto fail;
    movie->io_buffer = NULL;
    movie->format->pb = movie->io;
    movie->format->flags |= AVFMT_FLAG_CUSTOM_IO;

    if (avformat_open_input(&movie->format, NULL, NULL, NULL) < 0 ||
        avformat_find_stream_info(movie->format, NULL) < 0)
        goto fail;

    for (i = 0; i < (int)movie->format->nb_streams; ++i)
    {
        stream = movie->format->streams[i];
        if (movie->video_stream < 0 && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            movie->video_stream = i;
        else if (movie->audio_stream < 0 && stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            movie->audio_stream = i;
    }

    if (movie->video_stream < 0 || !open_codec(movie->format, movie->video_stream, &movie->video))
        goto fail;

    stream = movie->format->streams[movie->video_stream];
    movie->width = movie->video->width;
    movie->height = movie->video->height;
    movie->video_time_base = stream->time_base;
    movie->video_rate = av_guess_frame_rate(movie->format, stream, NULL);
    if (movie->video_rate.num <= 0 || movie->video_rate.den <= 0)
        movie->video_rate = (AVRational){15, 1};
    if (stream->nb_frames > 0)
        movie->frame_count = stream->nb_frames;
    else if (movie->video->framerate.num > 0)
        movie->frame_count = (int64_t)(av_q2d(movie->video->framerate) *
                                       ((double)movie->format->duration / AV_TIME_BASE));

    if (movie->audio_stream >= 0 && movie->audio_enabled)
    {
        if (!open_codec(movie->format, movie->audio_stream, &movie->audio))
            movie->audio_stream = -1;
        else if (!init_audio(movie))
            movie->audio_stream = -1;
    }

    return movie;

fail:
    AVPMovie_Close(movie);
    return NULL;
}

void AVPMovie_Close(AVPMovie *movie)
{
    if (!movie)
        return;
    close_audio(movie);
    if (movie->sws)
        sws_freeContext(movie->sws);
    if (movie->video)
        avcodec_free_context(&movie->video);
    if (movie->audio)
        avcodec_free_context(&movie->audio);
    if (movie->frame)
        av_frame_free(&movie->frame);
    if (movie->packet)
        av_packet_free(&movie->packet);
    if (movie->format)
        avformat_close_input(&movie->format);
    if (movie->io)
        avio_context_free(&movie->io);
    if (movie->file)
        fclose(movie->file);
    if (movie->io_buffer)
        av_free(movie->io_buffer);
    free(movie);
}

int AVPMovie_Width(const AVPMovie *movie) { return movie ? movie->width : 0; }
int AVPMovie_Height(const AVPMovie *movie) { return movie ? movie->height : 0; }
int AVPMovie_FrameNumber(const AVPMovie *movie) { return movie ? movie->frame_number : 0; }
int AVPMovie_FrameCount(const AVPMovie *movie)
{
    if (!movie || movie->frame_count > 0x7fffffff)
        return 0x7fffffff;
    return movie ? (int)movie->frame_count : 0;
}

int AVPMovie_FrameDurationMs(const AVPMovie *movie)
{
    if (!movie || movie->video_rate.num <= 0 || movie->video_rate.den <= 0)
        return 66;
    return (int)((1000LL * movie->video_rate.den + movie->video_rate.num / 2) / movie->video_rate.num);
}

static int convert_frame(AVPMovie *movie, uint8_t *dst, int dst_width, int dst_height,
                         int dst_stride)
{
    int scaled_width, scaled_height, x, y;
    struct SwsContext *sws;
    uint8_t *out_data[4];
    int out_linesize[4];

    if (!dst || dst_width <= 0 || dst_height <= 0)
        return 0;

    if ((int64_t)movie->width * dst_height > (int64_t)movie->height * dst_width)
    {
        scaled_width = dst_width;
        scaled_height = (int)((int64_t)movie->height * dst_width / movie->width);
    }
    else
    {
        scaled_height = dst_height;
        scaled_width = (int)((int64_t)movie->width * dst_height / movie->height);
    }

    x = (dst_width - scaled_width) / 2;
    y = (dst_height - scaled_height) / 2;
    memset(dst, 0, (size_t)dst_stride * dst_height);

    sws = sws_getCachedContext(movie->sws,
                               movie->width, movie->height, movie->video->pix_fmt,
                               scaled_width, scaled_height, AV_PIX_FMT_RGB24,
                               SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws)
        return 0;
    movie->sws = sws;

    out_data[0] = dst + (size_t)y * dst_stride + x * 3;
    out_linesize[0] = dst_stride;
    out_data[1] = out_data[2] = out_data[3] = NULL;
    out_linesize[1] = out_linesize[2] = out_linesize[3] = 0;

    sws_scale(movie->sws, (const uint8_t *const *)movie->frame->data,
              movie->frame->linesize, 0, movie->height, out_data, out_linesize);
    return 1;
}

int AVPMovie_NextVideoFrame(AVPMovie *movie, uint8_t *dst, int dst_width, int dst_height,
                            int dst_stride, int64_t *pts_ms)
{
    int ret;

    if (!movie || !movie->video || movie->eof)
        return 0;

    for (;;)
    {
        ret = avcodec_receive_frame(movie->video, movie->frame);
        if (ret == 0)
        {
            int64_t pts = movie->frame->best_effort_timestamp;
            if (pts == AV_NOPTS_VALUE)
                pts = movie->frame_number;
            if (pts_ms)
                *pts_ms = av_rescale_q(pts, movie->video_time_base, (AVRational){1, 1000});
            if (!convert_frame(movie, dst, dst_width, dst_height, dst_stride))
                return 0;
            movie->frame_number++;
            return 1;
        }
        if (ret == AVERROR_EOF)
        {
            movie->eof = 1;
            return 0;
        }
        if (ret != AVERROR(EAGAIN))
            return 0;

        ret = av_read_frame(movie->format, movie->packet);
        if (ret < 0)
        {
            avcodec_send_packet(movie->video, NULL);
            continue;
        }

        if (movie->packet->stream_index == movie->video_stream)
        {
            ret = avcodec_send_packet(movie->video, movie->packet);
            av_packet_unref(movie->packet);
            if (ret < 0)
                return 0;
        }
        else if (movie->packet->stream_index == movie->audio_stream)
        {
            avcodec_send_packet(movie->audio, movie->packet);
            av_packet_unref(movie->packet);
        }
        else
        {
            av_packet_unref(movie->packet);
        }
    }
}

int AVPMovie_FeedAudio(AVPMovie *movie)
{
    int ret;

    if (!movie || !movie->audio || !movie->audio_stream_handle || !movie->swr)
        return 1;

    while ((ret = avcodec_receive_frame(movie->audio, movie->frame)) == 0)
    {
        int max_samples = (int)av_rescale_rnd(swr_get_delay(movie->swr, movie->audio->sample_rate) +
                                                  movie->frame->nb_samples,
                                              48000,
                                              movie->audio->sample_rate, AV_ROUND_UP);
        int channels = 2;
        int bytes = av_samples_get_buffer_size(NULL, channels, max_samples,
                                               AV_SAMPLE_FMT_S16, 1);
        uint8_t *out = (uint8_t *)realloc(movie->audio_buffer, bytes);
        int converted;
        if (!out)
            return 0;
        movie->audio_buffer = out;
        movie->audio_buffer_size = bytes;

        converted = swr_convert(movie->swr, &movie->audio_buffer, max_samples,
                                (const uint8_t **)movie->frame->extended_data,
                                movie->frame->nb_samples);
        if (converted > 0)
        {
            int out_bytes = converted * channels * (int)sizeof(int16_t);
            if (!SDL_PutAudioStreamData(movie->audio_stream_handle,
                                        movie->audio_buffer, out_bytes))
                return 0;
        }
    }

    if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return 0;

    return 1;
}

int AVPMovie_SeekFrame(AVPMovie *movie, int frame)
{
    AVStream *stream;
    int64_t ts;

    if (!movie || frame < 0)
        return 0;

    stream = movie->format->streams[movie->video_stream];
    ts = av_rescale_q(frame, (AVRational){movie->video_rate.den, movie->video_rate.num},
                      stream->time_base);

    if (av_seek_frame(movie->format, movie->video_stream, ts, AVSEEK_FLAG_BACKWARD) < 0)
        return 0;

    avcodec_flush_buffers(movie->video);
    if (movie->audio)
        avcodec_flush_buffers(movie->audio);
    movie->frame_number = frame;
    movie->eof = 0;
    return 1;
}

int AVPMovie_PresentSurface(AVPMovie *movie, const uint8_t *rgb, int width, int height, int stride)
{
    int x, y;
    uint16_t *pixels;
    int pitch_pixels;

    (void)movie;

    if (!surface || surface->w != 640 || surface->h != 480 ||
        !rgb || width != 640 || height != 480 || stride < width * 3)
        return 0;

    if (SDL_MUSTLOCK(surface) && !SDL_LockSurface(surface))
        return 0;

    pixels = (uint16_t *)surface->pixels;
    pitch_pixels = surface->pitch / 2;

    for (y = 0; y < 480; ++y)
    {
        const uint8_t *src = rgb + (size_t)y * stride;
        for (x = 0; x < 640; ++x)
        {
            uint8_t r = src[x * 3 + 0];
            uint8_t g = src[x * 3 + 1];
            uint8_t b = src[x * 3 + 2];
            pixels[y * pitch_pixels + x] = (uint16_t)((r >> 3) << 11) |
                                           (uint16_t)((g >> 2) << 5) |
                                           (uint16_t)(b >> 3);
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
    return 1;
}

int AVPMovie_PlayFullscreen(const char *filename)
{
    AVPMovie *movie;
    uint8_t *rgb = NULL;
    int64_t pts_ms = 0;
    int64_t start_ms;
    int64_t last_pts = -1;
    int done = 0;

    movie = AVPMovie_Open(filename, 1);
    if (!movie)
        return 0;

    rgb = (uint8_t *)malloc(640 * 480 * 3);
    if (!rgb)
    {
        AVPMovie_Close(movie);
        return 0;
    }

    start_ms = (int64_t)SDL_GetTicks();

    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                free(rgb);
                AVPMovie_Close(movie);
                exit(0);
            }
            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
            {
                done = 1;
            }
        }

        if (!AVPMovie_NextVideoFrame(movie, rgb, 640, 480, 640 * 3, &pts_ms))
            break;

        AVPMovie_FeedAudio(movie);

        if (pts_ms > 0)
        {
            int64_t target = start_ms + pts_ms;
            int64_t now = (int64_t)SDL_GetTicks();
            while (now < target && !done)
            {
                SDL_Delay((Uint32)((target - now) > 2 ? 2 : (target - now)));
                now = (int64_t)SDL_GetTicks();
            }
        }
        else if (last_pts >= 0)
        {
            SDL_Delay((Uint32)(1000.0 * movie->video_rate.den / movie->video_rate.num));
        }
        last_pts = pts_ms;

        if (AVPMovie_PresentSurface(movie, rgb, 640, 480, 640 * 3))
            FlipBuffers();
    }

    free(rgb);
    AVPMovie_Close(movie);
    return 1;
}
