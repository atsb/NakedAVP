#ifndef AVP_MOVIE_H
#define AVP_MOVIE_H

#include <stdint.h>

/*
 * Small, format-independent movie interface used by NakedAVP.
 * FFmpeg supplies the Bink (.bik) and Smacker (.smk) demuxers/decoders;
 * the game itself only deals with decoded RGB frames and PCM audio.
 */
typedef struct AVPMovie AVPMovie;

AVPMovie *AVPMovie_Open(const char *filename, int with_audio);
void AVPMovie_Close(AVPMovie *movie);

int AVPMovie_Width(const AVPMovie *movie);
int AVPMovie_Height(const AVPMovie *movie);
int AVPMovie_FrameNumber(const AVPMovie *movie);
int AVPMovie_FrameCount(const AVPMovie *movie);
int AVPMovie_FrameDurationMs(const AVPMovie *movie);

int AVPMovie_NextVideoFrame(AVPMovie *movie, uint8_t *dst, int dst_width, int dst_height,
                            int dst_stride, int64_t *pts_ms);

int AVPMovie_FeedAudio(AVPMovie *movie);

int AVPMovie_SeekFrame(AVPMovie *movie, int frame);

int AVPMovie_PlayFullscreen(const char *filename);

int AVPMovie_PresentSurface(AVPMovie *movie, const uint8_t *rgb, int width, int height, int stride);

#endif
