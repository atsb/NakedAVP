#include "3dc.h"
#include "module.h"
#include "platform.h"
#include "kshape.h"
#include "progress_bar.h"
#include "chnktexi.h"
#include "awtexld.h"
#include "ffstdio.h"
#include "inline.h"
#include "gamedef.h"
#include "psnd.h"
extern "C"
{
#include "language.h"
#include "avp_menus.h"
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern int DebouncedGotAnyKey;

extern void MinimalNetCollectMessages(void);
extern void NetSendMessages(void);

extern void ThisFramesRenderingHasBegun(void);
extern void ThisFramesRenderingHasFinished(void);

extern int AAFontImageNumber;
extern int FadingGameInAfterLoading;

extern void InGameFlipBuffers();

extern void RenderStringCentred(char *stringPtr, int centreX, int y, int colour);

extern void BltImage(RECT *dest, DDSurface *image, RECT *src);
extern int CreateOGLTexture(D3DTexture *, unsigned char *);
extern int CreateIMGSurface(D3DTexture *, unsigned char *);
};

static const char* Loading_Bar_Empty_Image_Name="Menus\\Loadingbar_empty.rim";
static const char* Loading_Bar_Full_Image_Name="Menus\\Loadingbar_full.rim";

static DDSurface* LoadingBarEmpty;
static DDSurface* LoadingBarFull;
static RECT LoadingBarEmpty_DestRect;
static RECT LoadingBarEmpty_SrcRect;
static RECT LoadingBarFull_DestRect;
static RECT LoadingBarFull_SrcRect;

static int CurrentPosition=0;
static int LoadingInProgress=0;

static void Draw_Progress_Bar(void) {

	ThisFramesRenderingHasBegun();

	ColourFillBackBuffer(0);

	if (LoadingInProgress != 0 && LoadingBarEmpty != NULL) {
		BltImage(&LoadingBarEmpty_DestRect,LoadingBarEmpty,&LoadingBarEmpty_SrcRect);
	}

	if (LoadingBarFull != NULL) {
		BltImage(&LoadingBarFull_DestRect,LoadingBarFull,&LoadingBarFull_SrcRect);
	}

	FlushD3DZBuffer();

	RenderBriefingText(ScreenDescriptorBlock.SDB_Height/2, ONE_FIXED);

	if (LoadingInProgress == 0) {
		RenderStringCentred(GetTextString(TEXTSTRING_INGAME_PRESSANYKEYTOCONTINUE), ScreenDescriptorBlock.SDB_Width/2, (ScreenDescriptorBlock.SDB_Height*23)/24-9, 0xffffffff);
	}

	ThisFramesRenderingHasFinished();

	InGameFlipBuffers();
}

void Start_Progress_Bar()
{
	LoadingBarEmpty = NULL;
	LoadingBarFull = NULL;

	AAFontImageNumber = CL_LoadImageOnce("Common\\aa_font.RIM",LIO_D3DTEXTURE|LIO_RELATIVEPATH|LIO_RESTORABLE);

	/* load other graphics */
	{
		char buffer[100];
		CL_GetImageFileName(buffer, 100,Loading_Bar_Empty_Image_Name, LIO_RELATIVEPATH);
		
		//see if graphic can be found in fast file
		size_t fastFileLength;
		void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			LoadingBarEmpty = AwCreateSurface
							(
								"pxf",
								pFastFileData,
								fastFileLength,
								0
							);
		}
		else
		{
			//load graphic from rim file
			LoadingBarEmpty = AwCreateSurface
							(
								"sf",
								buffer,
								0
							);
		}
	}
	{
		char buffer[100];
		CL_GetImageFileName(buffer, 100,Loading_Bar_Full_Image_Name, LIO_RELATIVEPATH);
		
		//see if graphic can be found in fast file
		size_t fastFileLength;
		void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			LoadingBarFull = AwCreateSurface
							(
								"pxf",
								pFastFileData,
								fastFileLength,
								0
							);
		}
		else
		{
			//load graphic from rim file
			LoadingBarFull = AwCreateSurface
							(
								"sf",
								buffer,
								0
							);
		}
	}
	
	CreateOGLTexture(LoadingBarEmpty, NULL);
	CreateOGLTexture(LoadingBarFull, NULL);

	//draw initial progress bar
	LoadingBarEmpty_SrcRect.left=0;
	LoadingBarEmpty_SrcRect.right=639;
	LoadingBarEmpty_SrcRect.top=0;
	LoadingBarEmpty_SrcRect.bottom=39;
	LoadingBarEmpty_DestRect.left=0;
	LoadingBarEmpty_DestRect.right=ScreenDescriptorBlock.SDB_Width-1;
	LoadingBarEmpty_DestRect.top=(ScreenDescriptorBlock.SDB_Height *11)/12;
	LoadingBarEmpty_DestRect.bottom=ScreenDescriptorBlock.SDB_Height-1;

	LoadingBarFull_SrcRect.left=0;
	LoadingBarFull_SrcRect.right=0;
	LoadingBarFull_SrcRect.top=0;
	LoadingBarFull_SrcRect.bottom=39;
	LoadingBarFull_DestRect.left=0;
	LoadingBarFull_DestRect.right=0;
	LoadingBarFull_DestRect.top=(ScreenDescriptorBlock.SDB_Height *11)/12;
	LoadingBarFull_DestRect.bottom=ScreenDescriptorBlock.SDB_Height-1;

	CurrentPosition=0;
	LoadingInProgress=1;

	Draw_Progress_Bar();
}

void Set_Progress_Bar_Position(int pos)
{
	int NewPosition = DIV_FIXED(pos,PBAR_LENGTH);
	if(NewPosition>CurrentPosition)
	{
		CurrentPosition=NewPosition;
		LoadingBarFull_SrcRect.left=0;
		LoadingBarFull_SrcRect.right=MUL_FIXED(639,NewPosition);
		LoadingBarFull_SrcRect.top=0;
		LoadingBarFull_SrcRect.bottom=39;
		LoadingBarFull_DestRect.left=0;
		LoadingBarFull_DestRect.right=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,NewPosition);
		LoadingBarFull_DestRect.top=(ScreenDescriptorBlock.SDB_Height *11)/12;
		LoadingBarFull_DestRect.bottom=ScreenDescriptorBlock.SDB_Height-1;

		Draw_Progress_Bar();

		/*
		If this is a network game , then check the received network messages from 
		time to time (~every second).
		Has nothing to do with the progress bar , but this is a convenient place to
		do the check.
		*/
		
		if(AvP.Network != I_No_Network)
		{
			static int LastSendTime;
			int time=GetTickCount();
			if(time-LastSendTime>1000 || time<LastSendTime)
			{
				//time to check our messages 
				LastSendTime=time;

				MinimalNetCollectMessages();
				//send messages , mainly  needed so that the host will send the game description
				//allowing people to join while the host is loading
				NetSendMessages();
			}
		}
		
	}
}

extern "C"
{

void Game_Has_Loaded(void)
{
	extern int NormalFrameTime;

	SoundSys_StopAll();
	SoundSys_Management();

	LoadingInProgress = 0;

	int f = 65536;
	ResetFrameCounter();
	do
	{
		CheckForWindowsMessages();
		ReadUserInput();
	
		ColourFillBackBufferQuad
		(
			0,
			0,
			(ScreenDescriptorBlock.SDB_Height*11)/12,
			ScreenDescriptorBlock.SDB_Width-1,
			ScreenDescriptorBlock.SDB_Height-1
		);

		if (f)
		{
			LoadingBarFull_SrcRect.left=0;
			LoadingBarFull_SrcRect.right=639;
			LoadingBarFull_SrcRect.top=0;
			LoadingBarFull_SrcRect.bottom=39;
			LoadingBarFull_DestRect.left=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,(ONE_FIXED-f)/2);
			LoadingBarFull_DestRect.right=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,f)+LoadingBarFull_DestRect.left;

			int h = MUL_FIXED((ScreenDescriptorBlock.SDB_Height)/24,ONE_FIXED-f);
			LoadingBarFull_DestRect.top=(ScreenDescriptorBlock.SDB_Height *11)/12+h;
			LoadingBarFull_DestRect.bottom=ScreenDescriptorBlock.SDB_Height-1-h;

			f-=NormalFrameTime;
			if (f<0) f=0;
		} else {
			LoadingBarFull_SrcRect.left=0;
			LoadingBarFull_SrcRect.right=0;
			LoadingBarFull_SrcRect.top=0;
			LoadingBarFull_SrcRect.bottom=0;
			LoadingBarFull_DestRect.left=0;
			LoadingBarFull_DestRect.right=0;
			LoadingBarFull_DestRect.top=0;
			LoadingBarFull_DestRect.bottom=0;
		}

		Draw_Progress_Bar();

		FrameCounterHandler();

		/* If in a network game then we may as well check the network messages while waiting*/
		if(AvP.Network != I_No_Network)
		{
			MinimalNetCollectMessages();

			//send messages , mainly  needed so that the host will send the game description
			//allowing people to join while the host is loading
			NetSendMessages();
			
		}
		
	}
	while(!DebouncedGotAnyKey);

	FadingGameInAfterLoading=ONE_FIXED;

	if (LoadingBarFull != NULL) {
		ReleaseDDSurface(LoadingBarFull);
		LoadingBarFull = NULL;
	}

	if (LoadingBarEmpty != NULL) {
		ReleaseDDSurface(LoadingBarEmpty);
		LoadingBarEmpty = NULL;
	}
}

};
