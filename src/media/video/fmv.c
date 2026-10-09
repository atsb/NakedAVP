/* KJL 15:25:20 8/16/97
 *
 * smacker.c - functions to handle FMV playback
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "fmv.h"
#include "movie.h"
#include <SDL3/SDL.h>
#include "avp_menus.h"
#include "avp_userprofile.h"
#include "oglfunc.h" // move this into opengl.c

#define UseLocalAssert 1
#include "ourasert.h"

int VolumeOfNearestVideoScreen;
int PanningOfNearestVideoScreen;

extern char *ScreenBuffer;
extern int GotAnyKey;
extern void DirectReadKeyboard(void);
extern IMAGEHEADER ImageHeaderArray[];
#if MaxImageGroups > 1
extern int NumImagesArray[];
#else
extern int NumImages;
#endif

void PlayFMV(char *filenamePtr);

void FindLightingValueFromFMV(unsigned short *bufferPtr);
void FindLightingValuesFromTriggeredFMV(unsigned char *bufferPtr, FMVTEXTURE *ftPtr);

int SmackerSoundVolume = ONE_FIXED / 512;
int MoviesAreActive;
int IntroOutroMoviesAreActive = 1;

int FmvColourRed;
int FmvColourGreen;
int FmvColourBlue;

void ReleaseFMVTexture(FMVTEXTURE *ftPtr);
void UpdateFMVTexture(FMVTEXTURE *ftPtr);

void PlayBinkedFMV(char *filenamePtr)
{
	if (!filenamePtr)
		return;
	AVPMovie_PlayFullscreen(filenamePtr);
}

static AVPMovie *MenuBackgroundMovie;
static unsigned char *MenuBackgroundRGB;
static int64_t MenuBackgroundStartTicks;
static int64_t MenuBackgroundNextPTS;
static int MenuBackgroundReady;

static int StartMenuBackgroundMovie(void)
{
	if (MenuBackgroundMovie)
		return 1;

	MenuBackgroundMovie = AVPMovie_Open("FMVs/Menubackground.bik", 0);
	if (!MenuBackgroundMovie)
	{
		MenuBackgroundReady = 0;
		return 0;
	}

	MenuBackgroundRGB = (unsigned char *)malloc(640 * 480 * 3);
	if (!MenuBackgroundRGB)
	{
		AVPMovie_Close(MenuBackgroundMovie);
		MenuBackgroundMovie = NULL;
		MenuBackgroundReady = 0;
		return 0;
	}

	MenuBackgroundStartTicks = (int64_t)SDL_GetTicks();
	MenuBackgroundNextPTS = 0;
	MenuBackgroundReady = 0;
	return 1;
}

void StartMenuBackgroundBink(void)
{
	(void)StartMenuBackgroundMovie();
}

int PlayMenuBackgroundBink(void)
{
	int64_t now;
	int64_t elapsed;
	int64_t pts;

	if (!MenuBackgroundMovie || !MenuBackgroundRGB)
		return 0;

	now = (int64_t)SDL_GetTicks();
	elapsed = now - MenuBackgroundStartTicks;

	if (MenuBackgroundReady && elapsed < MenuBackgroundNextPTS)
		return 1;

	pts = 0;
	if (!AVPMovie_NextVideoFrame(MenuBackgroundMovie, MenuBackgroundRGB,
								 640, 480, 640 * 3, &pts))
	{
		if (!AVPMovie_SeekFrame(MenuBackgroundMovie, 0) ||
			!AVPMovie_NextVideoFrame(MenuBackgroundMovie, MenuBackgroundRGB,
									 640, 480, 640 * 3, &pts))
		{
			MenuBackgroundReady = 0;
			return 0;
		}
		MenuBackgroundStartTicks = now;
		elapsed = 0;
	}

	MenuBackgroundReady = 1;
	MenuBackgroundNextPTS = pts + AVPMovie_FrameDurationMs(MenuBackgroundMovie);
	return 1;
}

int PresentMenuBackgroundBink(void)
{
	if (!MenuBackgroundMovie || !MenuBackgroundRGB || !MenuBackgroundReady)
		return 0;
	return AVPMovie_PresentSurface(MenuBackgroundMovie, MenuBackgroundRGB,
								   640, 480, 640 * 3);
}

void EndMenuBackgroundBink(void)
{
	if (MenuBackgroundMovie)
	{
		AVPMovie_Close(MenuBackgroundMovie);
		MenuBackgroundMovie = NULL;
	}
	free(MenuBackgroundRGB);
	MenuBackgroundRGB = NULL;
	MenuBackgroundReady = 0;
	MenuBackgroundStartTicks = 0;
	MenuBackgroundNextPTS = 0;
}

/* KJL 12:45:23 10/08/98 - FMVTEXTURE stuff */
#define MAX_NO_FMVTEXTURES 10
FMVTEXTURE FMVTexture[MAX_NO_FMVTEXTURES];
int NumberOfFMVTextures;

static AVPMovie *TriggeredFMVMovie;
static unsigned char *TriggeredFMVRGB;
static int TriggeredFMVMessage;
static int64_t TriggeredFMVStartTicks;
static int64_t TriggeredFMVLastPTS;

void ScanImagesForFMVs(void)
{

	extern void SetupFMVTexture(FMVTEXTURE * ftPtr);
	int i;
	IMAGEHEADER *ihPtr;
	NumberOfFMVTextures = 0;

#if MaxImageGroups > 1
	for (j = 0; j < MaxImageGroups; j++)
	{
		if (NumImagesArray[j])
		{
			ihPtr = &ImageHeaderArray[j * MaxImages];
			for (i = 0; i < NumImagesArray[j]; i++, ihPtr++)
			{
#else
	{
		if (NumImages)
		{
			ihPtr = &ImageHeaderArray[0];
			for (i = 0; i < NumImages; i++, ihPtr++)
			{
#endif
				char *strPtr;
				if ((strPtr = strstr(ihPtr->ImageName, "FMVs")))
				{
					char filename[30];
					{
						char *filenamePtr = filename;
						do
						{
							*filenamePtr++ = *strPtr;
						} while (*strPtr++ != '.');

						*filenamePtr++ = 's';
						*filenamePtr++ = 'm';
						*filenamePtr++ = 'k';
						*filenamePtr = 0;
					}

					// if (smackHandle)
					//{
					//	FMVTexture[NumberOfFMVTextures].IsTriggeredPlotFMV = 0;
					// }
					// else
					{
						FMVTexture[NumberOfFMVTextures].IsTriggeredPlotFMV = 1;
					}

					{
						// FMVTexture[NumberOfFMVTextures].SmackHandle = smackHandle;
						FMVTexture[NumberOfFMVTextures].ImagePtr = ihPtr;
						FMVTexture[NumberOfFMVTextures].StaticImageDrawn = 0;
						SetupFMVTexture(&FMVTexture[NumberOfFMVTextures]);
						NumberOfFMVTextures++;

						if (NumberOfFMVTextures == MAX_NO_FMVTEXTURES)
						{
							break;
						}
					}
				}
			}
		}
	}
}

void UpdateAllFMVTextures(void)
{
	int i;

	if (TriggeredFMVMovie && TriggeredFMVRGB)
	{
		int64_t pts = 0;
		int64_t now = (int64_t)SDL_GetTicks();
		int should_decode = (TriggeredFMVLastPTS < 0);

		if (!should_decode)
		{
			int64_t elapsed = now - TriggeredFMVStartTicks;
			should_decode = (TriggeredFMVLastPTS < elapsed);
		}

		if (should_decode)
		{
			if (AVPMovie_NextVideoFrame(TriggeredFMVMovie, TriggeredFMVRGB,
										128, 96, 128 * 3, &pts))
			{
				AVPMovie_FeedAudio(TriggeredFMVMovie);
				TriggeredFMVLastPTS = pts + AVPMovie_FrameDurationMs(TriggeredFMVMovie);

				{
					unsigned long long red = 0, green = 0, blue = 0;
					int pixels = 128 * 96;
					const unsigned char *p = TriggeredFMVRGB;
					for (i = 0; i < pixels; ++i, p += 3)
					{
						red += p[0];
						green += p[1];
						blue += p[2];
					}
					FmvColourRed = (int)((red / 48) * 16);
					FmvColourGreen = (int)((green / 48) * 16);
					FmvColourBlue = (int)((blue / 48) * 16);
				}
			}
			else
			{
				AVPMovie_Close(TriggeredFMVMovie);
				TriggeredFMVMovie = NULL;
				TriggeredFMVLastPTS = -1;
			}
		}
	}

	for (i = 0; i < NumberOfFMVTextures; ++i)
	{
		UpdateFMVTexture(&FMVTexture[i]);
	}
}

void ReleaseAllFMVTextures(void)
{
	int i = NumberOfFMVTextures;

	if (TriggeredFMVMovie)
	{
		AVPMovie_Close(TriggeredFMVMovie);
		TriggeredFMVMovie = NULL;
	}
	free(TriggeredFMVRGB);
	TriggeredFMVRGB = NULL;
	TriggeredFMVMessage = 0;
	TriggeredFMVLastPTS = -1;

	while (i--)
		ReleaseFMVTexture(&FMVTexture[i]);
}

int NextFMVTextureFrame(FMVTEXTURE *ftPtr, void *bufferPtr)
{
	int w = 128;

	if (TriggeredFMVMovie && TriggeredFMVRGB &&
		ftPtr->IsTriggeredPlotFMV && ftPtr->MessageNumber == TriggeredFMVMessage)
	{
		int x, y;
		unsigned char *dst = (unsigned char *)bufferPtr;
		const unsigned char *src = TriggeredFMVRGB;

		/* The game texture is RGBA, while the decoder supplies RGB24. */
		for (y = 0; y < 96; ++y)
		{
			for (x = 0; x < 128; ++x)
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = 255;
			}
		}
		ftPtr->StaticImageDrawn = 1;
		return 1;
	}

	if (!ftPtr->StaticImageDrawn)
	{
		int i = w * 96 / 4;
		unsigned int seed = FastRandom();
		int *ptr = (int *)bufferPtr;
		do
		{
			seed = ((seed * 1664525) + 1013904223);
			*ptr++ = seed;
		} while (--i);
		ftPtr->StaticImageDrawn = 1;
	}
	return 1;
}

void UpdateFMVTexturePalette(FMVTEXTURE *ftPtr)
{
	// unsigned char *c;
	int i;
	//
	// if (MoviesAreActive && ftPtr->SmackHandle)
	//{
	//}
	// else
	{
		{
			unsigned int seed = FastRandom();
			for (i = 0; i < 256; i++)
			{
				int l = (seed & (seed >> 24) & (seed >> 16));
				seed = ((seed * 1664525) + 1013904223);
				ftPtr->SrcPalette[i].peRed = l;
				ftPtr->SrcPalette[i].peGreen = l;
				ftPtr->SrcPalette[i].peBlue = l;
			}
		}
	}
}

extern void StartTriggerPlotFMV(int number)
{
	char buffer[64];
	int i;

	if (number <= 0)
		return;

	snprintf(buffer, sizeof(buffer), "FMVs/message%d.smk", number);

	if (TriggeredFMVMovie)
	{
		AVPMovie_Close(TriggeredFMVMovie);
		TriggeredFMVMovie = NULL;
	}

	TriggeredFMVMovie = AVPMovie_Open(buffer, 1);
	if (!TriggeredFMVMovie)
	{
		fprintf(stderr, "FMV: unable to open %s\n", buffer);
		TriggeredFMVMessage = 0;
		return;
	}

	if (!TriggeredFMVRGB)
		TriggeredFMVRGB = (unsigned char *)malloc(128 * 96 * 3);
	if (!TriggeredFMVRGB)
	{
		AVPMovie_Close(TriggeredFMVMovie);
		TriggeredFMVMovie = NULL;
		return;
	}

	TriggeredFMVMessage = number;
	TriggeredFMVStartTicks = (int64_t)SDL_GetTicks();
	TriggeredFMVLastPTS = -1;

	for (i = 0; i < NumberOfFMVTextures; ++i)
	{
		if (FMVTexture[i].IsTriggeredPlotFMV)
		{
			FMVTexture[i].MessageNumber = number;
			FMVTexture[i].StaticImageDrawn = 0;
		}
	}
}

extern void StartFMVAtFrame(int number, int frame)
{
	if (!TriggeredFMVMovie || TriggeredFMVMessage != number)
	{
		StartTriggerPlotFMV(number);
	}
	if (TriggeredFMVMovie)
	{
		AVPMovie_SeekFrame(TriggeredFMVMovie, frame);
		TriggeredFMVStartTicks = (int64_t)SDL_GetTicks();
		TriggeredFMVLastPTS = -1;
	}
}

extern void GetFMVInformation(int *messageNumberPtr, int *frameNumberPtr)
{
	if (messageNumberPtr)
		*messageNumberPtr = TriggeredFMVMessage;
	if (frameNumberPtr)
		*frameNumberPtr = TriggeredFMVMovie ? AVPMovie_FrameNumber(TriggeredFMVMovie) : 0;
}

extern void InitialiseTriggeredFMVs(void)
{
	int i = NumberOfFMVTextures;

	if (TriggeredFMVMovie)
	{
		AVPMovie_Close(TriggeredFMVMovie);
		TriggeredFMVMovie = NULL;
	}
	TriggeredFMVMessage = 0;
	TriggeredFMVLastPTS = -1;

	while (i--)
	{
		if (FMVTexture[i].IsTriggeredPlotFMV)
		{
			FMVTexture[i].MessageNumber = 0;
			FMVTexture[i].StaticImageDrawn = 0;
		}
	}
}

void FindLightingValuesFromTriggeredFMV(unsigned char *bufferPtr, FMVTEXTURE *ftPtr)
{
	unsigned int totalRed = 0;
	unsigned int totalBlue = 0;
	unsigned int totalGreen = 0;

	int pixels = 128 * 96; // 64*48;//256*192;
	do
	{
		unsigned char source = (*bufferPtr++);
		totalBlue += ftPtr->SrcPalette[source].peBlue;
		totalGreen += ftPtr->SrcPalette[source].peGreen;
		totalRed += ftPtr->SrcPalette[source].peRed;
	} while (--pixels);

	FmvColourRed = totalRed / 48 * 16;
	FmvColourGreen = totalGreen / 48 * 16;
	FmvColourBlue = totalBlue / 48 * 16;
}

void SetupFMVTexture(FMVTEXTURE *ftPtr)
{
	if (ftPtr->PalettedBuf == NULL)
	{
		ftPtr->PalettedBuf = (unsigned char *)calloc(1, 128 * 128 + 128 * 128 * 4);
	}

	if (ftPtr->RGBBuf == NULL)
	{
		if (ftPtr->PalettedBuf == NULL)
		{
			return;
		}

		ftPtr->RGBBuf = &ftPtr->PalettedBuf[128 * 128];
	}
}

void UpdateFMVTexture(FMVTEXTURE *ftPtr)
{
	int pixels = 128 * 96;
	unsigned char *srcPtr;
	unsigned char *dstPtr;

	if (!ftPtr->PalettedBuf)
		SetupFMVTexture(ftPtr);
	if (!ftPtr->PalettedBuf || !ftPtr->RGBBuf)
		return;

	if (TriggeredFMVMovie && TriggeredFMVRGB &&
		ftPtr->IsTriggeredPlotFMV && ftPtr->MessageNumber == TriggeredFMVMessage)
	{
		/* NextFMVTextureFrame writes RGBA directly into RGBBuf. */
		if (!NextFMVTextureFrame(ftPtr, ftPtr->RGBBuf))
			return;
	}
	else if (!ftPtr->StaticImageDrawn)
	{
		NextFMVTextureFrame(ftPtr, ftPtr->PalettedBuf);
		UpdateFMVTexturePalette(ftPtr);
		srcPtr = ftPtr->PalettedBuf;
		dstPtr = ftPtr->RGBBuf;
		do
		{
			unsigned char source = *srcPtr++;
			*dstPtr++ = ftPtr->SrcPalette[source].peRed;
			*dstPtr++ = ftPtr->SrcPalette[source].peGreen;
			*dstPtr++ = ftPtr->SrcPalette[source].peBlue;
			*dstPtr++ = 255;
		} while (--pixels);
	}

	pglBindTexture(GL_TEXTURE_2D, ftPtr->ImagePtr->D3DTexture->id);
	pglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96,
					 GL_RGBA, GL_UNSIGNED_BYTE, ftPtr->RGBBuf);
}

void ReleaseFMVTexture(FMVTEXTURE *ftPtr)
{
	ftPtr->MessageNumber = 0;

	if (ftPtr->PalettedBuf != NULL)
	{
		free(ftPtr->PalettedBuf);
		ftPtr->PalettedBuf = NULL;
	}

	ftPtr->RGBBuf = NULL;
}
