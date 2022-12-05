/* KJL 15:25:20 8/16/97
 *
 * smacker.c - functions to handle FMV playback
 *
 */
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "fmv.h"
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
#if MaxImageGroups>1
extern int NumImagesArray[];
#else
extern int NumImages;
#endif

void PlayFMV(char *filenamePtr);

void FindLightingValueFromFMV(unsigned short *bufferPtr);
void FindLightingValuesFromTriggeredFMV(unsigned char *bufferPtr, FMVTEXTURE *ftPtr);

int SmackerSoundVolume=ONE_FIXED/512;
int MoviesAreActive;
int IntroOutroMoviesAreActive=1;

int FmvColourRed;
int FmvColourGreen;
int FmvColourBlue;

void ReleaseFMVTexture(FMVTEXTURE *ftPtr);


/* KJL 12:45:23 10/08/98 - FMVTEXTURE stuff */
#define MAX_NO_FMVTEXTURES 10
FMVTEXTURE FMVTexture[MAX_NO_FMVTEXTURES];
int NumberOfFMVTextures;

void ScanImagesForFMVs(void)
{
	
	extern void SetupFMVTexture(FMVTEXTURE *ftPtr);
	int i;
	IMAGEHEADER *ihPtr;
	NumberOfFMVTextures=0;

	#if MaxImageGroups>1
	for (j=0; j<MaxImageGroups; j++)
	{
		if (NumImagesArray[j])
		{
			ihPtr = &ImageHeaderArray[j*MaxImages];
			for (i = 0; i<NumImagesArray[j]; i++, ihPtr++)
			{
	#else
	{
		if(NumImages)
		{
			ihPtr = &ImageHeaderArray[0];
			for (i = 0; i<NumImages; i++, ihPtr++)
			{
	#endif
				char *strPtr;
				if((strPtr = strstr(ihPtr->ImageName,"FMVs")))
				{
					char filename[30];
					{
						char *filenamePtr = filename;
						do
						{
							*filenamePtr++ = *strPtr;
						}
						while(*strPtr++!='.');

						*filenamePtr++='s';
						*filenamePtr++='m';
						*filenamePtr++='k';
						*filenamePtr=0;
					}
					
					//if (smackHandle)
					//{
					//	FMVTexture[NumberOfFMVTextures].IsTriggeredPlotFMV = 0;
					//}
					//else
					{
						FMVTexture[NumberOfFMVTextures].IsTriggeredPlotFMV = 1;
					}

					{
						//FMVTexture[NumberOfFMVTextures].SmackHandle = smackHandle;
						FMVTexture[NumberOfFMVTextures].ImagePtr = ihPtr;
						FMVTexture[NumberOfFMVTextures].StaticImageDrawn=0;
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
	extern void UpdateFMVTexture(FMVTEXTURE *ftPtr);
	int i = NumberOfFMVTextures;

	while(i--)
	{
		UpdateFMVTexture(&FMVTexture[i]);
	}

}

void ReleaseAllFMVTextures(void)
{	
	extern void UpdateFMVTexture(FMVTEXTURE *ftPtr);
	int i = NumberOfFMVTextures;

	while(i--)
	{
		ReleaseFMVTexture(&FMVTexture[i]);
	}

}


int NextFMVTextureFrame(FMVTEXTURE *ftPtr, void *bufferPtr)
{
	int w = 128;
	
	if (!ftPtr->StaticImageDrawn)
	{
		int i = w*96/4;
		unsigned int seed = FastRandom();
		int *ptr = (int*)bufferPtr;
		do
		{
			seed = ((seed*1664525)+1013904223);
			*ptr++ = seed;
		}
		while(--i);
		ftPtr->StaticImageDrawn=1;
	}
	FindLightingValuesFromTriggeredFMV((unsigned char*)bufferPtr,ftPtr);
	return 1;

}

void UpdateFMVTexturePalette(FMVTEXTURE *ftPtr)
{
	//unsigned char *c;
	int i;
	//
	//if (MoviesAreActive && ftPtr->SmackHandle)
	//{
	//}
	//else
	{
	  	{
			unsigned int seed = FastRandom();
			for(i=0;i<256;i++)
			{   
				int l = (seed&(seed>>24)&(seed>>16));
				seed = ((seed*1664525)+1013904223);
				ftPtr->SrcPalette[i].peRed=l;
				ftPtr->SrcPalette[i].peGreen=l;
		   		ftPtr->SrcPalette[i].peBlue=l;
		 	}	
		}
	}
}

extern void StartTriggerPlotFMV(int number)
{
	(void) number;
	
	//int i = NumberOfFMVTextures;
	//char buffer[25];
	//
	//if (CheatMode_Active != CHEATMODE_NONACTIVE) return;
	//
	//sprintf(buffer,"FMVs//message%d.smk",number);
	//{
	//	FILE* file=fopen(buffer,"rb");
	//	if(!file)
	//	{
	//		return;
	//	}
	//	fclose(file);
	//}
	//while(i--)
	//{
	//	if (FMVTexture[i].IsTriggeredPlotFMV)
	//	{
	//		FMVTexture[i].MessageNumber = number;
	//	}
	//}
}

extern void StartFMVAtFrame(int number, int frame)
{
}

extern void GetFMVInformation(int *messageNumberPtr, int *frameNumberPtr)
{
	//int i = NumberOfFMVTextures;
	//
	//while(i--)
	//{
	//	if (FMVTexture[i].IsTriggeredPlotFMV)
	//	{
	//		if(FMVTexture[i].SmackHandle)
	//		{
	//			*messageNumberPtr = FMVTexture[i].MessageNumber;
	//			*frameNumberPtr = 0;
	//			return;
	//		}
	//	}
	//}

	*messageNumberPtr = 0;
	*frameNumberPtr = 0;
}


extern void InitialiseTriggeredFMVs(void)
{
	//int i = NumberOfFMVTextures;
	//while(i--)
	//{
	//	if (FMVTexture[i].IsTriggeredPlotFMV)
	//	{
	//		if(FMVTexture[i].SmackHandle)
	//		{
	//			FMVTexture[i].MessageNumber = 0;
	//		}
	//
	//		FMVTexture[i].SmackHandle = 0;
	//	}
	//}
}

void FindLightingValuesFromTriggeredFMV(unsigned char *bufferPtr, FMVTEXTURE *ftPtr)
{
	unsigned int totalRed=0;
	unsigned int totalBlue=0;
	unsigned int totalGreen=0;
	
	int pixels = 128*96;//64*48;//256*192;
	do
	{
		unsigned char source = (*bufferPtr++);
		totalBlue += ftPtr->SrcPalette[source].peBlue;
		totalGreen += ftPtr->SrcPalette[source].peGreen;
		totalRed += ftPtr->SrcPalette[source].peRed; 
	}
	while(--pixels);

	FmvColourRed = totalRed/48*16;
	FmvColourGreen = totalGreen/48*16;
	FmvColourBlue = totalBlue/48*16;

}

void SetupFMVTexture(FMVTEXTURE *ftPtr)
{
	if (ftPtr->PalettedBuf == NULL)
	{
		ftPtr->PalettedBuf = (unsigned char*) calloc(1, 128*128+128*128*4);
	}
	
	if (ftPtr->RGBBuf == NULL)
	{
		if (ftPtr->PalettedBuf == NULL)
		{
			return;
		}
		
		ftPtr->RGBBuf = &ftPtr->PalettedBuf[128*128];
	}
}

void UpdateFMVTexture(FMVTEXTURE *ftPtr)
{
	unsigned char *srcPtr;
	unsigned char *dstPtr;
	
	int pixels = 128*96;//64*48;//256*192;
	
	// get the next frame into the paletted buffer
	if (!NextFMVTextureFrame(ftPtr, &ftPtr->PalettedBuf[0]))
	{
		return;
	}
	
	// update the texture palette
	UpdateFMVTexturePalette(ftPtr);
	
	srcPtr = &ftPtr->PalettedBuf[0];
	dstPtr = &ftPtr->RGBBuf[0];
	
	// not using paletted textures, so convert to rgb manually
	do
	{
		unsigned char source = (*srcPtr++);
		dstPtr[0] = ftPtr->SrcPalette[source].peRed;
		dstPtr[1] = ftPtr->SrcPalette[source].peGreen;
		dstPtr[2] = ftPtr->SrcPalette[source].peBlue;
		dstPtr[3] = 255;
		
		dstPtr += 4;
	} while(--pixels);
	
//#warning move this into opengl.c
	// update the opengl texture
	pglBindTexture(GL_TEXTURE_2D, ftPtr->ImagePtr->D3DTexture->id);
	
	pglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96, GL_RGB, GL_UNSIGNED_BYTE, &ftPtr->RGBBuf[0]);
}

void ReleaseFMVTexture(FMVTEXTURE *ftPtr)
{
	ftPtr->MessageNumber = 0;

	//if (FMVTexture[i].SrcTexture)
	//{
	//	ReleaseD3DTexture(FMVTexture[i].SrcTexture);
	//	FMVTexture[i].SrcTexture=0;
	//}
	//if (FMVTexture[i].SrcSurface)
	//{
	//	ReleaseDDSurface(FMVTexture[i].SrcSurface);
	//	FMVTexture[i].SrcSurface=0;
	//}
	//if (FMVTexture[i].DestTexture)
	//{	
	//	ReleaseD3DTexture(FMVTexture[i].DestTexture);
	//	FMVTexture[i].DestTexture = 0;
	//}

	if (ftPtr->PalettedBuf != NULL)
	{
		free(ftPtr->PalettedBuf);
		ftPtr->PalettedBuf = NULL;
	}
	
	ftPtr->RGBBuf = NULL;
}
