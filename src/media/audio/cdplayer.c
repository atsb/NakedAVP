#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#if !defined(_WIN32)
#include <sys/types.h>
#endif

#include <SDL3/SDL.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#include "fixer.h"
#include "files.h"
#include "win95/cd_player.h"
#include "cdplayer.h"

/*
 * Gibbon: Feed the PCM to an SDL3 audio stream.
 */

#define CDMUSIC_RATE        48000
#define CDMUSIC_CHANNELS    2
#define CDMUSIC_BYTES_FRAME (CDMUSIC_CHANNELS * (int)sizeof(int16_t))
#define CDMUSIC_MAX_TRACK   26
#define CDMUSIC_IO_BUFFER   (32 * 1024)
#define CDMUSIC_CALLBACK_CHUNK 8192

int CDPlayerVolume;
int CDTrackMax = 0;

static SDL_AudioStream *MusicStream;
static SDL_Mutex *MusicMutex;
static uint8_t *MusicPCM;
static size_t MusicPCMBytes;
static size_t MusicPCMPosition;
static int MusicCurrentTrack = -1;
static int MusicLoop;
static int MusicPlaying;
static int MusicEnabled;
static int MusicVolume = CDDA_VOLUME_DEFAULT;

typedef struct MusicIO
{
	FILE *file;
} MusicIO;

static int music_read(void *opaque, uint8_t *buf, int buf_size)
{
	MusicIO *io = (MusicIO *)opaque;
	size_t n;

	if (!io || !io->file || buf_size <= 0)
		return AVERROR(EIO);

	n = fread(buf, 1, (size_t)buf_size, io->file);
	if (n == 0)
	{
		if (feof(io->file))
			return AVERROR_EOF;
		return AVERROR(EIO);
	}

	return (int)n;
}

static int64_t music_seek(void *opaque, int64_t offset, int whence)
{
	MusicIO *io = (MusicIO *)opaque;
	int origin = whence & 0xFFFF;

	if (!io || !io->file)
		return -1;

	if (whence & AVSEEK_SIZE)
	{
#if defined(_WIN32)
		__int64 old_pos = _ftelli64(io->file);
		__int64 end_pos;
		if (_fseeki64(io->file, 0, SEEK_END) != 0)
			return -1;
		end_pos = _ftelli64(io->file);
		_fseeki64(io->file, old_pos, SEEK_SET);
		return (int64_t)end_pos;
#else
		off_t old_pos = ftello(io->file);
		off_t end_pos;
		if (fseeko(io->file, 0, SEEK_END) != 0)
			return -1;
		end_pos = ftello(io->file);
		fseeko(io->file, old_pos, SEEK_SET);
		return (int64_t)end_pos;
#endif
	}

#if defined(_WIN32)
	if (_fseeki64(io->file, offset, origin) != 0)
		return -1;
	return (int64_t)_ftelli64(io->file);
#else
	if (fseeko(io->file, (off_t)offset, origin) != 0)
		return -1;
	return (int64_t)ftello(io->file);
#endif
}

static int append_pcm(uint8_t **buffer, size_t *size, size_t *capacity,
	const uint8_t *data, size_t bytes)
{
	size_t required;
	size_t new_capacity;
	uint8_t *new_buffer;

	if (bytes == 0)
		return 1;

	if (*size > SIZE_MAX - bytes)
		return 0;

	required = *size + bytes;
	if (required <= *capacity)
	{
		memcpy(*buffer + *size, data, bytes);
		*size = required;
		return 1;
	}

	new_capacity = (*capacity != 0) ? *capacity : (size_t)(CDMUSIC_RATE * CDMUSIC_BYTES_FRAME);
	while (new_capacity < required)
	{
		if (new_capacity > SIZE_MAX / 2)
		{
			new_capacity = required;
			break;
		}
		new_capacity *= 2;
	}

	new_buffer = (uint8_t *)realloc(*buffer, new_capacity);
	if (!new_buffer)
		return 0;

	*buffer = new_buffer;
	*capacity = new_capacity;
	memcpy(*buffer + *size, data, bytes);
	*size = required;
	return 1;
}

static int decode_audio_frame(AVFrame *frame, SwrContext *swr,
	int source_rate, uint8_t **pcm, size_t *pcm_bytes, size_t *pcm_capacity)
{
	int max_samples;
	int temp_bytes;
	int converted;
	uint8_t *temp = NULL;

	max_samples = (int)av_rescale_rnd(
		swr_get_delay(swr, source_rate) + frame->nb_samples,
		CDMUSIC_RATE, source_rate, AV_ROUND_UP);
	if (max_samples <= 0)
		return 1;

	temp_bytes = av_samples_get_buffer_size(NULL, CDMUSIC_CHANNELS,
		max_samples, AV_SAMPLE_FMT_S16, 1);
	if (temp_bytes <= 0)
		return 0;

	temp = (uint8_t *)malloc((size_t)temp_bytes);
	if (!temp)
		return 0;

	converted = swr_convert(swr, &temp, max_samples,
		(const uint8_t **)frame->extended_data, frame->nb_samples);
	if (converted > 0)
	{
		size_t out_bytes = (size_t)converted * CDMUSIC_BYTES_FRAME;
		if (!append_pcm(pcm, pcm_bytes, pcm_capacity, temp, out_bytes))
		{
			free(temp);
			return 0;
		}
	}

	free(temp);
	return 1;
}

static int flush_resampler(SwrContext *swr, int source_rate,
	uint8_t **pcm, size_t *pcm_bytes, size_t *pcm_capacity)
{
	for (;;)
	{
		int max_samples = (int)av_rescale_rnd(
			swr_get_delay(swr, source_rate),
			CDMUSIC_RATE, source_rate, AV_ROUND_UP);
		int temp_bytes;
		int converted;
		uint8_t *temp;

		if (max_samples <= 0)
			break;

		temp_bytes = av_samples_get_buffer_size(NULL, CDMUSIC_CHANNELS,
			max_samples, AV_SAMPLE_FMT_S16, 1);
		if (temp_bytes <= 0)
			return 0;

		temp = (uint8_t *)malloc((size_t)temp_bytes);
		if (!temp)
			return 0;

		converted = swr_convert(swr, &temp, max_samples, NULL, 0);
		if (converted > 0)
		{
			size_t out_bytes = (size_t)converted * CDMUSIC_BYTES_FRAME;
			if (!append_pcm(pcm, pcm_bytes, pcm_capacity, temp, out_bytes))
			{
				free(temp);
				return 0;
			}
		}

		free(temp);
		if (converted <= 0)
			break;
	}

	return 1;
}

static int decode_track(FILE *file, uint8_t **out_pcm, size_t *out_bytes)
{
	AVFormatContext *format = NULL;
	AVCodecContext *codec_ctx = NULL;
	const AVCodec *codec;
	AVFrame *frame = NULL;
	AVPacket *packet = NULL;
	SwrContext *swr = NULL;
	AVIOContext *io = NULL;
	unsigned char *io_buffer = NULL;
	AVChannelLayout out_layout = AV_CHANNEL_LAYOUT_STEREO;
	AVChannelLayout in_layout;
	MusicIO music_io;
	uint8_t *pcm = NULL;
	size_t pcm_bytes = 0;
	size_t pcm_capacity = 0;
	int audio_stream = -1;
	int i;
	int result = 0;

	if (!file || !out_pcm || !out_bytes)
		return 0;

	memset(&in_layout, 0, sizeof(in_layout));
	music_io.file = file;

	format = avformat_alloc_context();
	if (!format)
		goto done;

	io_buffer = (unsigned char *)av_malloc(CDMUSIC_IO_BUFFER);
	if (!io_buffer)
		goto done;

	io = avio_alloc_context(io_buffer, CDMUSIC_IO_BUFFER, 0, &music_io,
		music_read, NULL, music_seek);
	if (!io)
		goto done;

	io_buffer = NULL;
	format->pb = io;
	format->flags |= AVFMT_FLAG_CUSTOM_IO;

	if (avformat_open_input(&format, NULL, NULL, NULL) < 0)
		goto done;

	if (avformat_find_stream_info(format, NULL) < 0)
		goto done;

	for (i = 0; i < (int)format->nb_streams; ++i)
	{
		if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_stream = i;
			break;
		}
	}

	if (audio_stream < 0)
		goto done;

	codec = avcodec_find_decoder(format->streams[audio_stream]->codecpar->codec_id);
	if (!codec)
		goto done;

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
		goto done;

	if (avcodec_parameters_to_context(codec_ctx,
		format->streams[audio_stream]->codecpar) < 0)
		goto done;

	if (codec_ctx->sample_rate <= 0 || codec_ctx->ch_layout.nb_channels <= 0)
		goto done;

	if (avcodec_open2(codec_ctx, codec, NULL) < 0)
		goto done;

	if (av_channel_layout_copy(&in_layout, &codec_ctx->ch_layout) < 0)
	{
		av_channel_layout_default(&in_layout, codec_ctx->ch_layout.nb_channels);
	}

	if (swr_alloc_set_opts2(&swr,
		&out_layout, AV_SAMPLE_FMT_S16, CDMUSIC_RATE,
		&in_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
		0, NULL) < 0 || !swr || swr_init(swr) < 0)
		goto done;

	frame = av_frame_alloc();
	packet = av_packet_alloc();
	if (!frame || !packet)
		goto done;

	for (;;)
	{
		int ret = av_read_frame(format, packet);
		if (ret < 0)
		{
			if (ret == AVERROR_EOF)
			{
				if (avcodec_send_packet(codec_ctx, NULL) < 0)
					goto done;

				for (;;)
				{
					ret = avcodec_receive_frame(codec_ctx, frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						break;
					if (ret < 0)
						goto done;
					if (!decode_audio_frame(frame, swr, codec_ctx->sample_rate,
						&pcm, &pcm_bytes, &pcm_capacity))
						goto done;
				}

				break;
			}
			goto done;
		}

		if (packet->stream_index == audio_stream)
		{
			ret = avcodec_send_packet(codec_ctx, packet);
			av_packet_unref(packet);
			if (ret < 0)
				goto done;

			for (;;)
			{
				ret = avcodec_receive_frame(codec_ctx, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					break;
				if (ret < 0)
					goto done;
				if (!decode_audio_frame(frame, swr, codec_ctx->sample_rate,
					&pcm, &pcm_bytes, &pcm_capacity))
					goto done;
			}
		}
		else
		{
			av_packet_unref(packet);
		}
	}

	if (!flush_resampler(swr, codec_ctx->sample_rate,
		&pcm, &pcm_bytes, &pcm_capacity))
		goto done;

	if (pcm_bytes == 0)
		goto done;

	*out_pcm = pcm;
	*out_bytes = pcm_bytes;
	pcm = NULL;
	result = 1;

done:
	free(pcm);
	if (swr)
		swr_free(&swr);
	av_channel_layout_uninit(&in_layout);
	if (frame)
		av_frame_free(&frame);
	if (packet)
		av_packet_free(&packet);
	if (codec_ctx)
		avcodec_free_context(&codec_ctx);
	if (format)
		avformat_close_input(&format);
	if (io)
		avio_context_free(&io);
	if (io_buffer)
		av_free(io_buffer);

	return result;
}

static int find_track_file(int track, char *path, size_t path_size)
{
	static const char *extensions[] = { ".ogg", ".mp3", ".flac", ".wav" };
	static const char *directories[] = { "cd_tracks", "music" };
	int direct_track = track;
	int alternate_track = track - 1;
	int pass;
	int ext;
	FILE *test_file;

	if (!path || path_size == 0 || track <= 0)
		return 0;

	for (pass = 0; pass < 2; ++pass)
	{
		int file_track = (pass == 0) ? direct_track : alternate_track;
		if (file_track <= 0)
			continue;

		for (ext = 0; ext < (int)(sizeof(extensions) / sizeof(extensions[0])); ++ext)
		{
			int written = snprintf(path, path_size, "%s/track%02d%s",
				directories[0], file_track, extensions[ext]);
			if (written <= 0 || (size_t)written >= path_size)
				continue;

			test_file = OpenGameFile(path, FILEMODE_READONLY, FILETYPE_OPTIONAL);
			if (test_file)
			{
				fclose(test_file);
				return 1;
			}

			written = snprintf(path, path_size, "%s/track%02d%s",
				directories[1], file_track, extensions[ext]);
			if (written <= 0 || (size_t)written >= path_size)
				continue;

			test_file = OpenGameFile(path, FILEMODE_READONLY, FILETYPE_OPTIONAL);
			if (test_file)
			{
				fclose(test_file);
				return 1;
			}
		}
	}

	path[0] = '\0';
	return 0;
}

static void set_music_volume_locked(void)
{
	if (MusicVolume < CDDA_VOLUME_MIN)
		MusicVolume = CDDA_VOLUME_MIN;
	if (MusicVolume > CDDA_VOLUME_MAX)
		MusicVolume = CDDA_VOLUME_MAX;
}

static void MusicStreamCallback(void *userdata, SDL_AudioStream *stream,
	int additional_amount, int total_amount)
{
	uint8_t buffer[CDMUSIC_CALLBACK_CHUNK];
	int remaining = additional_amount;
	(void)userdata;
	(void)total_amount;

	while (remaining > 0)
	{
		int request = remaining;
		int produced = 0;

		if (request > (int)sizeof(buffer))
			request = (int)sizeof(buffer);

		if (MusicMutex)
			SDL_LockMutex(MusicMutex);

		while (produced < request)
		{
			size_t available;
			size_t chunk;
			float gain;
			int i;

			if (!MusicPlaying || !MusicPCM || MusicPCMBytes == 0)
			{
				memset(buffer + produced, 0, (size_t)(request - produced));
				produced = request;
				break;
			}

			if (MusicPCMPosition >= MusicPCMBytes)
			{
				if (MusicLoop)
					MusicPCMPosition = 0;
				else
				{
					MusicPlaying = 0;
					memset(buffer + produced, 0, (size_t)(request - produced));
					produced = request;
					break;
				}
			}

			available = MusicPCMBytes - MusicPCMPosition;
			chunk = (size_t)(request - produced);
			if (chunk > available)
				chunk = available;
			chunk -= chunk % CDMUSIC_BYTES_FRAME;
			if (chunk == 0)
			{
				MusicPCMPosition = MusicPCMBytes;
				continue;
			}

			memcpy(buffer + produced, MusicPCM + MusicPCMPosition, chunk);
			MusicPCMPosition += chunk;
			produced += (int)chunk;

			gain = (float)MusicVolume / (float)CDDA_VOLUME_MAX;
			if (gain < 0.0f) gain = 0.0f;
			if (gain > 1.0f) gain = 1.0f;

			if (gain < 0.999f)
			{
				int16_t *samples = (int16_t *)(buffer + produced - (int)chunk);
				int sample_count = (int)chunk / (int)sizeof(int16_t);
				for (i = 0; i < sample_count; ++i)
					samples[i] = (int16_t)((float)samples[i] * gain);
			}
		}

		if (MusicMutex)
			SDL_UnlockMutex(MusicMutex);

		if (!SDL_PutAudioStreamData(stream, buffer, produced))
			break;

		remaining -= produced;
		if (produced <= 0)
			break;
	}
}

static int load_track(int track, uint8_t **pcm, size_t *pcm_bytes, char *path, size_t path_size)
{
	FILE *file;
	int ok;

	if (!find_track_file(track, path, path_size))
		return 0;

	file = OpenGameFile(path, FILEMODE_READONLY, FILETYPE_OPTIONAL);
	if (!file)
		return 0;

	ok = decode_track(file, pcm, pcm_bytes);
	fclose(file);
	return ok;
}

void CheckCDVolume(void)
{
	int volume = CDPlayerVolume;

	if (volume < CDDA_VOLUME_MIN)
		volume = CDDA_VOLUME_MIN;
	if (volume > CDDA_VOLUME_MAX)
		volume = CDDA_VOLUME_MAX;

	CDPlayerVolume = volume;

	if (!MusicMutex)
	{
		MusicVolume = volume;
		return;
	}

	SDL_LockMutex(MusicMutex);
	MusicVolume = volume;
	SDL_UnlockMutex(MusicMutex);
}

void CDDA_Start(void)
{
	SDL_AudioSpec spec;

	if (MusicStream)
		return;

	CDPlayerVolume = CDDA_VOLUME_DEFAULT;
	MusicVolume = CDDA_VOLUME_DEFAULT;
	MusicEnabled = 0;
	MusicPlaying = 0;
	MusicCurrentTrack = -1;
	CDTrackMax = 0;

	/* Gibbon: Discover replacements before creating SDL device. */
	CDDA_CheckNumberOfTracks();
	if (CDTrackMax == 0)
	{
		fprintf(stderr, "CDDA: no local music tracks found (cd_tracks/trackXX.ogg)\n");
		return;
	}

	if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && !SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, "CDDA: SDL audio init failed: %s\n", SDL_GetError());
		CDTrackMax = 0;
		return;
	}

	MusicMutex = SDL_CreateMutex();
	if (!MusicMutex)
	{
		fprintf(stderr, "CDDA: unable to create audio mutex: %s\n", SDL_GetError());
		return;
	}

	memset(&spec, 0, sizeof(spec));
	spec.format = SDL_AUDIO_S16;
	spec.channels = CDMUSIC_CHANNELS;
	spec.freq = CDMUSIC_RATE;

	MusicStream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
		MusicStreamCallback, NULL);
	if (!MusicStream)
	{
		fprintf(stderr, "CDDA: unable to open audio stream: %s\n", SDL_GetError());
		SDL_DestroyMutex(MusicMutex);
		MusicMutex = NULL;
		return;
	}

	if (!SDL_ResumeAudioStreamDevice(MusicStream))
	{
		fprintf(stderr, "CDDA: unable to start audio stream: %s\n", SDL_GetError());
		SDL_DestroyAudioStream(MusicStream);
		MusicStream = NULL;
		SDL_DestroyMutex(MusicMutex);
		MusicMutex = NULL;
		return;
	}

	MusicEnabled = 1;
	fprintf(stderr, "CDDA: local music backend enabled (tracks 1-%d)\n", CDTrackMax - 1);
}

void CDDA_End(void)
{
	if (!MusicStream)
		return;

	if (MusicMutex)
		SDL_LockMutex(MusicMutex);
	MusicPlaying = 0;
	MusicEnabled = 0;
	MusicCurrentTrack = -1;
	MusicPCMPosition = 0;
	free(MusicPCM);
	MusicPCM = NULL;
	MusicPCMBytes = 0;
	if (MusicMutex)
		SDL_UnlockMutex(MusicMutex);

	SDL_DestroyAudioStream(MusicStream);
	MusicStream = NULL;

	if (MusicMutex)
	{
		SDL_DestroyMutex(MusicMutex);
		MusicMutex = NULL;
	}

	CDTrackMax = 0;
}

void CDDA_ChangeVolume(int volume)
{
	if (volume < CDDA_VOLUME_MIN || volume > CDDA_VOLUME_MAX)
		return;

	CDPlayerVolume = volume;
	if (!MusicMutex)
	{
		MusicVolume = volume;
		return;
	}

	SDL_LockMutex(MusicMutex);
	MusicVolume = volume;
	SDL_UnlockMutex(MusicMutex);
}

int CDDA_GetCurrentVolumeSetting(void)
{
	int volume;

	if (!MusicMutex)
		return MusicVolume;

	SDL_LockMutex(MusicMutex);
	volume = MusicVolume;
	SDL_UnlockMutex(MusicMutex);
	return volume;
}

int CDDA_CheckNumberOfTracks(void)
{
	int track;
	int highest = 0;
	char path[64];

	for (track = 1; track <= CDMUSIC_MAX_TRACK; ++track)
	{
		if (find_track_file(track, path, sizeof(path)))
			highest = track;
	}

	CDTrackMax = highest > 0 ? highest + 1 : 0;
	return highest;
}

int CDDA_IsOn(void)
{
	return MusicEnabled && MusicStream != NULL;
}

int CDDA_IsPlaying(void)
{
	int playing = 0;

	if (!MusicMutex)
		return 0;

	SDL_LockMutex(MusicMutex);
	playing = MusicPlaying;
	SDL_UnlockMutex(MusicMutex);
	return playing;
}

void CDDA_Play(int CDDATrack)
{
	uint8_t *pcm = NULL;
	size_t pcm_bytes = 0;
	char path[64];

	if (!CDDA_IsOn() || CDDATrack <= 0)
		return;

	if (CDTrackMax != 0 && CDDATrack >= CDTrackMax)
		return;

	if (!load_track(CDDATrack, &pcm, &pcm_bytes, path, sizeof(path)))
	{
		fprintf(stderr, "CDDA: unable to load track %d\n", CDDATrack);
		return;
	}

	if (MusicMutex)
		SDL_LockMutex(MusicMutex);

	MusicPlaying = 0;
	MusicLoop = 0;
	MusicCurrentTrack = CDDATrack;
	MusicPCMPosition = 0;
	free(MusicPCM);
	MusicPCM = pcm;
	MusicPCMBytes = pcm_bytes;
	MusicPlaying = 1;

	if (MusicMutex)
		SDL_UnlockMutex(MusicMutex);

	if (MusicStream)
		SDL_ClearAudioStream(MusicStream);

	fprintf(stderr, "CDDA: playing track %d (%s, %.1f sec)\n",
		CDDATrack, path, (double)pcm_bytes / (double)(CDMUSIC_RATE * CDMUSIC_BYTES_FRAME));
}

void CDDA_PlayLoop(int CDDATrack)
{
	uint8_t *pcm = NULL;
	size_t pcm_bytes = 0;
	char path[64];

	if (!CDDA_IsOn() || CDDATrack <= 0)
		return;

	if (CDTrackMax != 0 && CDDATrack >= CDTrackMax)
		return;

	if (!load_track(CDDATrack, &pcm, &pcm_bytes, path, sizeof(path)))
	{
		fprintf(stderr, "CDDA: unable to load track %d\n", CDDATrack);
		return;
	}

	if (MusicMutex)
		SDL_LockMutex(MusicMutex);

	MusicPlaying = 0;
	MusicLoop = 1;
	MusicCurrentTrack = CDDATrack;
	MusicPCMPosition = 0;
	free(MusicPCM);
	MusicPCM = pcm;
	MusicPCMBytes = pcm_bytes;
	MusicPlaying = 1;

	if (MusicMutex)
		SDL_UnlockMutex(MusicMutex);

	if (MusicStream)
		SDL_ClearAudioStream(MusicStream);

	fprintf(stderr, "CDDA: looping track %d (%s, %.1f sec)\n",
		CDDATrack, path, (double)pcm_bytes / (double)(CDMUSIC_RATE * CDMUSIC_BYTES_FRAME));
}

void CDDA_Stop(void)
{
	if (!MusicMutex)
		return;

	SDL_LockMutex(MusicMutex);
	MusicPlaying = 0;
	MusicLoop = 0;
	MusicPCMPosition = 0;
	MusicCurrentTrack = -1;
	SDL_UnlockMutex(MusicMutex);

	if (MusicStream)
		SDL_ClearAudioStream(MusicStream);
}

void CDDA_SwitchOn(void)
{
	if (MusicStream)
	{
		CDDA_CheckNumberOfTracks();
		MusicEnabled = (CDTrackMax > 0);
	}
}

void CDDA_SwitchOff(void)
{
	CDDA_Stop();
	MusicEnabled = 0;
}

void PlatCDDAManagement(void)
{
	/* Gibbon: SDL3 drives the stream asynchronously */
}

void CDDA_Management(void)
{
	PlatCDDAManagement();
}
