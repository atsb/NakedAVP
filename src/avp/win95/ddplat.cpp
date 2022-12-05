/*KJL**************************************************************************************
* ddplat.cpp - this contains all the display code for the HUD, menu screens and so forth. *
*                                                                                         *
**************************************************************************************KJL*/
extern "C" {

#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "dxlog.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "equipmnt.h"

#include "huddefs.h"
#include "hudgfx.h"

#include "font.h"

#include "kshape.h"
#include "chnktexi.h"
#include "awtexld.h"
#include "ffstdio.h"



#include "d3d_hud.h"

extern "C++" 
{
#include "r2base.h"
#include "indexfnt.hpp"

#include "projload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?
#include "chnkload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?
#include "pcmenus.h"
};

//#include "alt_tab.h"

extern int ScanDrawMode;
extern int ZBufferMode;
extern IMAGEHEADER ImageHeaderArray[];
int BackdropImage;
//#define UseLocalAssert Yes
//#include "ourasert.h"


int UsingDataBase = 0;



/* HUD globals */
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

#if 0 // SBF - unused
static int TrackerPolyBuffer[25];
static int ScanlinePolyBuffer[25];
static int MotionTrackerWidth;
static int MotionTrackerTextureSize;
static int MotionTrackerCentreY;
static int MotionTrackerCentreX;
static RECT MT_BarDestRect;    
static int MT_BlipHeight;
static int MT_BlipWidth;
struct LittleMDescTag *MTLittleMPtr;
#endif

enum HUD_RES_ID HUDResolution;

/* display co-ords, etc. */
#include "hud_data.h"



static struct DDGraphicTag PauseDDInfo;															
static struct DDGraphicTag E3FontDDInfo;

    
/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
void PlatformSpecificInitMarineHUD(void);
void PlatformSpecificInitPredatorHUD(void);

void PlatformSpecificExitingHUD(void);
void PlatformSpecificEnteringHUD(void);

void BLTMotionTrackerToHUD(int scanLineSize);
void BLTMotionTrackerBlipToHUD(int x, int y, int brightness);

#if 0
static void BLTDigitToHUD(char digit, int x, int y, int font);
void BLTPredatorOverlayToHUD(void);
static void DrawMotionTrackerPoly(void);
static void BLTPredatorDigitToHUD(char digit, int x, int y, int font);
#endif

void BLTGunSightToScreen(int screenX, int screenY, enum GUNSIGHT_SHAPE gunsightShape);
void BLTWeaponToHUD(PLAYER_WEAPON_DATA* weaponPtr);
int CueWeaponFrameFromSequence(struct WeaponFrameTag *weaponFramePtr, int timeOutCounter, int weaponIDNumber);



void BLTPredatorNumericsToHUD(void);

void LoadDDGraphic(struct DDGraphicTag *DDGfxPtr, char *Filename);

#if 0 // SBF - unused
static void SetupScanlinePoly(char const *filenamePtr, int width);
#endif

extern void D3D_InitialiseMarineHUD(void);
extern void D3D_BLTMotionTrackerToHUD(int scanLineSize);
extern void D3D_BLTMotionTrackerBlipToHUD(int x, int y, int brightness);
extern void D3D_BLTDigitToHUD(char digit, int x, int y, int font);
extern void D3D_BLTGunSightToHUD(int screenX, int screenY, enum GUNSIGHT_SHAPE gunsightShape);

extern void LoadCommonTextures(void);
/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/


void LoadDDGraphic(struct DDGraphicTag *DDGfxPtr, char *Filename)
{
	fprintf(stderr, "LoadDDGraphic(%p, %s)\n", DDGfxPtr, Filename);
}

/****************************************
*          SETTING UP THE HUD           *
****************************************/
void PlatformSpecificInitMarineHUD(void)
{
// SBF
//	if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferOn==ZBufferMode))

	{
		D3D_InitialiseMarineHUD();
		LoadCommonTextures();
//		ChromeImageNumber = CL_LoadImageOnce("Common\\chromelike.RIM",LIO_D3DTEXTURE|LIO_RELATIVEPATH|LIO_RESTORABLE);
		return;
	}

#if 0 // SBF - unused	
	//SelectGenTexDirectory(ITI_TEXTURE);

	/* set game mode: different, though for multiplayer game */
	if(AvP.Network==I_No_Network)
		cl_pszGameMode = "marine";
	else
		cl_pszGameMode = "multip";

	//	SetCurrentImageGroup(0);

	//	load_rif("avp_huds\\marine.rif");

	//	copy_chunks_from_environment(0);
	
	/* load HUD gfx */
	int gfxID = NO_OF_MARINE_HUD_GFX;
	if (ScreenDescriptorBlock.SDB_Width>=800)
	{
		HUDResolution = HUD_RES_HI;
		/* load Medres gfx */
		while(gfxID--)			      
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		HiresMarineHUDGfxFilenamePtr[gfxID]
			);	
		}
		
		TrackerPolyBuffer[3] = CL_LoadImageOnce("trakHiRz",(ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)|LIO_TRANSPARENT|LIO_RIFFPATH|LIO_RESTORABLE);
		MotionTrackerWidth = 244;
		MotionTrackerTextureSize = 243<<16;
		MTLittleMPtr = &HiresHUDLittleM;
		
		SetupScanlinePoly("scanhirz",MotionTrackerTextureSize);

		LoadDDGraphic(&E3FontDDInfo,"e3fontMR");	
	}
	else if (ScreenDescriptorBlock.SDB_Width>=640)
	{
		HUDResolution = HUD_RES_MED;
		/* load Medres gfx */
		while(gfxID--)			      
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		MedresMarineHUDGfxFilenamePtr[gfxID]
			);	
		}
		
		TrackerPolyBuffer[3] = CL_LoadImageOnce("trakMdRz",(ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)|LIO_TRANSPARENT|LIO_RIFFPATH|LIO_RESTORABLE);
		MotionTrackerWidth = 195;
		MotionTrackerTextureSize = 194<<16;
		MTLittleMPtr = &MedresHUDLittleM;
		
		SetupScanlinePoly("scanmdrz",MotionTrackerTextureSize);

		LoadDDGraphic(&E3FontDDInfo,"e3fontMR");	
	}
	else
	{
		HUDResolution = HUD_RES_LO;
	
		/* load lores gfx */
		while(gfxID--)			      
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		LoresMarineHUDGfxFilenamePtr[gfxID]
			);	
		}
		TrackerPolyBuffer[3] = CL_LoadImageOnce("tracker",(ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)|LIO_TRANSPARENT|LIO_RIFFPATH|LIO_RESTORABLE);
		MotionTrackerWidth = 97;
		MotionTrackerTextureSize = 96<<16;
		MTLittleMPtr = &LoresHUDLittleM;
		
		/* lores scanline slightly smaller than tracker... */
		SetupScanlinePoly("scan",MotionTrackerTextureSize-65536);

		LoadDDGraphic(&E3FontDDInfo,"e3font");	
	}

	TrackerPolyBuffer[0] = I_2dTexturedPolygon;
	TrackerPolyBuffer[2] = iflag_nolight|iflag_ignore0;
	
	ScanlinePolyBuffer[0] = I_2dTexturedPolygon;
	ScanlinePolyBuffer[2] = iflag_nolight|iflag_ignore0;
			
	/* screen dest of blue bar under motion tracker */
	MT_BarDestRect.bottom = ScreenDescriptorBlock.SDB_Height-1;
	MT_BarDestRect.top = MT_BarDestRect.bottom - HUDDDInfo[MARINE_HUD_GFX_BLUEBAR].SrcRect.bottom;
	MT_BarDestRect.left = 0;//MotionTrackerWidth/4;
	MT_BarDestRect.right = MT_BarDestRect.left + HUDDDInfo[MARINE_HUD_GFX_BLUEBAR].SrcRect.right;
		
	/* centre of motion tracker */
	MotionTrackerCentreY = MT_BarDestRect.top+1;
	MotionTrackerCentreX = (MT_BarDestRect.left+MT_BarDestRect.right)/2;
	
	/* motion tracker blips */
	MT_BlipHeight = HUDDDInfo[MARINE_HUD_GFX_MOTIONTRACKERBLIP].SrcRect.bottom/5;
	MT_BlipWidth = HUDDDInfo[MARINE_HUD_GFX_MOTIONTRACKERBLIP].SrcRect.right;


	LoadDDGraphic(&PauseDDInfo,"paused");	
#endif // SBF
}

void PlatformSpecificInitPredatorHUD(void)
{
	//SelectGenTexDirectory(ITI_TEXTURE);
	/* set game mode: different, though for multiplayer game */
	if(AvP.Network==I_No_Network)
	{
		cl_pszGameMode = "predator";
		/* load in sfx */
		LoadCommonTextures();
	}
	else
	{
		cl_pszGameMode = "multip";
		/* load in sfx */
		LoadCommonTextures();
		//load marine stuff as well
		D3D_InitialiseMarineHUD();
	}
	return;

#if 0 // SBF - unused
	int gfxID = NO_OF_PREDATOR_HUD_GFX;
	
	if (ScreenDescriptorBlock.SDB_Width>=640)
	{
		HUDResolution = HUD_RES_MED;
		/* load Medres gfx */
		while(gfxID--)
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		MedresPredatorHUDGfxFilenamePtr[gfxID]
			);
		}
		LoadDDGraphic(&E3FontDDInfo,"e3fontmr");	
	}
	else
	{
		/* load Lores gfx */
		int gfxID = NO_OF_PREDATOR_HUD_GFX;
		while(gfxID--)
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		LoresPredatorHUDGfxFilenamePtr[gfxID]
			);
		}
		LoadDDGraphic(&E3FontDDInfo,"e3font");	
	}
  	LoadDDGraphic(&PauseDDInfo,"paused");	
#endif // SBF
}


void PlatformSpecificInitAlienHUD(void)
{
	//SelectGenTexDirectory(ITI_TEXTURE);
	/* set game mode: different, though for multiplayer game */
	if(AvP.Network==I_No_Network)
	{
		cl_pszGameMode = "alien";
		LoadCommonTextures();
	}
	else
	{
		cl_pszGameMode = "multip";
		/* load in sfx */
		LoadCommonTextures();
		//load marine stuff as well
		D3D_InitialiseMarineHUD();
	}

	return;

#if 0 // SBF - unused	
	int gfxID = NO_OF_ALIEN_HUD_GFX;

	if (ScreenDescriptorBlock.SDB_Width==640)
	{
		HUDResolution = HUD_RES_MED;
		/* load Medres gfx */
		while(gfxID--)			      
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		MedresAlienHUDGfxFilenamePtr[gfxID]
			);	
		}
	   	LoadDDGraphic(&E3FontDDInfo,"e3fontmr");	
	}
	else
	{
		HUDResolution = HUD_RES_LO;

		/* load lores gfx */
		while(gfxID--)			      
	    {
			HUDDDInfo[gfxID].LPDDS = 0; // ensure 0 just in case one doesn't load and we try to delete it
			LoadDDGraphic
			(
				&HUDDDInfo[gfxID],
	    		LoresAlienHUDGfxFilenamePtr[gfxID]
			);	
		}
		LoadDDGraphic(&E3FontDDInfo,"e3font");	
	}
	LoadDDGraphic(&PauseDDInfo,"paused");	
#endif // SBF
}


/*JH 14/5/97*****************************
*            KILLING THE HUD            *
************************************JH**/


void PlatformSpecificKillMarineHUD(void)
{
	int gfxID = NO_OF_MARINE_HUD_GFX;
	
	while(gfxID--)			      
    {
		if (HUDDDInfo[gfxID].hBackup)
		{
			ATRemoveSurface(HUDDDInfo[gfxID].LPDDS);
			AwDestroyBackupTexture( HUDDDInfo[gfxID].hBackup );
		}
		if (HUDDDInfo[gfxID].LPDDS)
//			HUDDDInfo[gfxID].LPDDS->Release();
			fprintf(stderr, "PlatformSpecificKillMarineHUD: HUDDDInfo[gfxID].LPDDS\n");
			
		HUDDDInfo[gfxID].LPDDS = 0;
		HUDDDInfo[gfxID].hBackup = 0;
	}
	
	if (PauseDDInfo.hBackup)
	{
		ATRemoveSurface(PauseDDInfo.LPDDS);
		AwDestroyBackupTexture( PauseDDInfo.hBackup );
	}
	if (PauseDDInfo.LPDDS)
//		PauseDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillMarineHUD: PauseDDInfo.LPDDS\n");
		
	PauseDDInfo.LPDDS = 0;
	PauseDDInfo.hBackup = 0;
	
	if (E3FontDDInfo.hBackup)
	{
		ATRemoveSurface(E3FontDDInfo.LPDDS);
		AwDestroyBackupTexture( E3FontDDInfo.hBackup );
	}
	if (E3FontDDInfo.LPDDS)
//		E3FontDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillMarineHUD: E3FontDDInfo.LPDDS\n");
		
	E3FontDDInfo.LPDDS = 0;
	E3FontDDInfo.hBackup = 0;
}

void PlatformSpecificKillPredatorHUD(void)
{
	/* load HUD gfx */
	int gfxID = NO_OF_PREDATOR_HUD_GFX;

	while(gfxID--)			      
    {
		if (HUDDDInfo[gfxID].hBackup)
		{
			ATRemoveSurface(HUDDDInfo[gfxID].LPDDS);
			AwDestroyBackupTexture( HUDDDInfo[gfxID].hBackup );
		}
		if (HUDDDInfo[gfxID].LPDDS)
//			HUDDDInfo[gfxID].LPDDS->Release();
			fprintf(stderr, "PlatformSpecificKillPredatorHUD: HUDDDInfo[gfxID].LPDDS\n");
			
		HUDDDInfo[gfxID].LPDDS = 0;
		HUDDDInfo[gfxID].hBackup = 0;
	}
	
	if (PauseDDInfo.hBackup)
	{
		ATRemoveSurface(PauseDDInfo.LPDDS);
		AwDestroyBackupTexture( PauseDDInfo.hBackup );
	}
	if (PauseDDInfo.LPDDS)
//		PauseDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillPredatorHUD: PauseDDInfo.LPDDS\n");
		
	PauseDDInfo.LPDDS = 0;
	PauseDDInfo.hBackup = 0;
	
	if (E3FontDDInfo.hBackup)
	{
		ATRemoveSurface(E3FontDDInfo.LPDDS);
		AwDestroyBackupTexture( E3FontDDInfo.hBackup );
	}
	if (E3FontDDInfo.LPDDS)
//		E3FontDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillPredatorHUD: E3FontDDInfo.LPDDS\n");
		
	E3FontDDInfo.LPDDS = 0;
	E3FontDDInfo.hBackup = 0;
}


void PlatformSpecificKillAlienHUD(void)
{
	int gfxID = NO_OF_ALIEN_HUD_GFX;
	while(gfxID--)			      
    {
		if (HUDDDInfo[gfxID].hBackup)
		{
			ATRemoveSurface(HUDDDInfo[gfxID].LPDDS);
			AwDestroyBackupTexture( HUDDDInfo[gfxID].hBackup );
		}
		if (HUDDDInfo[gfxID].LPDDS)
//			HUDDDInfo[gfxID].LPDDS->Release();
			fprintf(stderr, "PlatformSpecificKillAlienHUD: HUDDDInfo[gfxID].LPDDS\n");
			
		HUDDDInfo[gfxID].LPDDS = 0;
		HUDDDInfo[gfxID].hBackup = 0;
	}
	
	if (PauseDDInfo.hBackup)
	{
		ATRemoveSurface(PauseDDInfo.LPDDS);
		AwDestroyBackupTexture( PauseDDInfo.hBackup );
	}
	if (PauseDDInfo.LPDDS)
//		PauseDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillAlienHUD: PauseDDInfo.LPDDS\n");
		
	PauseDDInfo.LPDDS = 0;
	PauseDDInfo.hBackup = 0;
	
	if (E3FontDDInfo.hBackup)
	{
		ATRemoveSurface(E3FontDDInfo.LPDDS);
		AwDestroyBackupTexture( E3FontDDInfo.hBackup );
	}
	if (E3FontDDInfo.LPDDS)
//		E3FontDDInfo.LPDDS->Release();	
		fprintf(stderr, "PlatformSpecificKillAlienHUD: E3FontDDInfo.LPDDS\n");
		
	E3FontDDInfo.LPDDS = 0;
	E3FontDDInfo.hBackup = 0;
}


/*********************/
/* RUNTIME HUD STUFF */
/*********************/

void PlatformSpecificExitingHUD(void)
{
#if 0
	/* KJL 11:37:19 06/14/97 - draw whatever is in the execute buffer */
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		WriteEndCodeToExecuteBuffer();
		UnlockExecuteBufferAndPrepareForUse();
		ExecuteBuffer();
		EndD3DScene();
	}
#endif
}

void PlatformSpecificEnteringHUD(void)
{
	/* JH 13/5/97 */
	/* Flush the ZBuffer so the weapons don't sink into the wall! */
	#if SupportZBuffering
	if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferMode != ZBufferOff))
	{
		//		FlushD3DZBuffer();
	}
	#endif

#if 0
	/* KJL 11:37:49 06/14/97 - reinit execute buffer */
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		BeginD3DScene();
		LockExecuteBuffer();
	}
#endif
}

/*KJL**********************
* MARINE DRAWING ROUTINES *
**********************KJL*/
void BLTMotionTrackerToHUD(int scanLineSize)
{
 //	if (VideoModeType_8 != VideoModeTypeScreen) return;
	if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferOn==ZBufferMode))
	{
		D3D_BLTMotionTrackerToHUD(scanLineSize);
	}
	return;
	
}

void BLTMotionTrackerBlipToHUD(int x, int y, int brightness)
{
	if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferOn==ZBufferMode))
	{
		D3D_BLTMotionTrackerBlipToHUD(x,y,brightness);
	}
	return;

}


/*KJL*******************
* Draw numerics to HUD *
*******************KJL*/
extern void BLTMarineNumericsToHUD(enum MARINE_HUD_DIGIT digitsToDraw)
{
   	int digit = digitsToDraw;
    struct DigitPropertiesTag *propertiesPtr;

	
	if (HUDResolution == HUD_RES_LO)
	{
		propertiesPtr = &LoresMarineHUDDigitProperties[digit];
	}
	else if (HUDResolution == HUD_RES_MED)
	{
		propertiesPtr = &MedresMarineHUDDigitProperties[digit];
	}
	else
	{
		propertiesPtr = &HiresMarineHUDDigitProperties[digit];
	}

    do
	{
    	/* paranoia check */
    	LOCALASSERT(ValueOfHUDDigit[digit]>=0 && ValueOfHUDDigit[digit]<=9);
		
		if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferOn==ZBufferMode))
		{
	    	D3D_BLTDigitToHUD
			(
				ValueOfHUDDigit[digit],
				propertiesPtr->X,
			    propertiesPtr->Y,
		        propertiesPtr->Font
		    );
		}
		propertiesPtr--;
    }
    while(digit--);
}	

#if 0 /* SBF - TODO: remove */
static void BLTDigitToHUD(char digit, int x, int y, int font)
{
//	HRESULT ddrval;
	struct HUDFontDescTag *FontDescPtr;
 	RECT srcRect;
	int gfxID;
	
	switch (font)
	{
		case MARINE_HUD_FONT_MT_SMALL:
	  	case MARINE_HUD_FONT_MT_BIG:
		{
		   	gfxID = MARINE_HUD_GFX_TRACKERFONT;
			y+=MotionTrackerCentreY;
		    x+=MotionTrackerCentreX;
			break;
		}
		case MARINE_HUD_FONT_RED:
		case MARINE_HUD_FONT_BLUE:
		{
			if (x<0) x+=ScreenDescriptorBlock.SDB_Width;
		   	gfxID = MARINE_HUD_GFX_NUMERALS;
			break;
		}
		case ALIEN_HUD_FONT:
		{
			gfxID = ALIEN_HUD_GFX_NUMBERS;
			break;
		}
		default:
			LOCALASSERT(0);
			break;
	}

	
	if (HUDResolution == HUD_RES_LO)
	{
		FontDescPtr = &LoresHUDFontDesc[font];
	}
	else if (HUDResolution == HUD_RES_MED)
	{
		FontDescPtr = &MedresHUDFontDesc[font];
	}
	else
	{
		FontDescPtr = &HiresHUDFontDesc[font];
	}
	
	srcRect.top = digit*FontDescPtr->Height;
	srcRect.bottom =srcRect.top + FontDescPtr->Height;
	srcRect.left = FontDescPtr->XOffset;
   	srcRect.right = srcRect.left + FontDescPtr->Width;
/*	   
   	ddrval = lpDDSBack->BltFast
   	(
   		x,y,
   		HUDDDInfo[gfxID].LPDDS,
   		&srcRect,
   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
   	);
	
   	if(ddrval != DD_OK)
   	{
   		ReleaseDirect3D();
   		exit(0x666004);
	}
*/
	fprintf(stderr, "BLTDigitToHUD(%d, %d, %d, %d)\n", digit, x, y, font);	
}		  
#endif


void BLTGunSightToScreen(int screenX, int screenY, enum GUNSIGHT_SHAPE gunsightShape)
{
	if ((ScanDrawMode != ScanDrawDirectDraw) && (ZBufferOn==ZBufferMode))
	{
		D3D_BLTGunSightToHUD(screenX,screenY,gunsightShape);
		return;
	}
}


#if 0 /* SBF - TODO: remove this directdraw code */


/*KJL************************
* PREDATOR DRAWING ROUTINES *
************************KJL*/
void BLTPredatorOverlayToHUD(void)
{
	/* KJL 11:37:19 06/14/97 - draw whatever is in the execute buffer */
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		WriteEndCodeToExecuteBuffer();
		UnlockExecuteBufferAndPrepareForUse();
		ExecuteBuffer();
		EndD3DScene();
	}

//  	HRESULT ddrval;
	if ((ScreenDescriptorBlock.SDB_Height ==200) ||(ScreenDescriptorBlock.SDB_Width ==320) )
	{  	
/*
	   	ddrval = lpDDSBack->BltFast
	   	(
	   		0,
	   		13,
	   		HUDDDInfo[PREDATOR_HUD_GFX_TOP].LPDDS,
	   		&(HUDDDInfo[PREDATOR_HUD_GFX_TOP].SrcRect),
	   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	   	);
	   	ddrval = lpDDSBack->BltFast
	   	(
	   		0,
	   		136,
	   		HUDDDInfo[PREDATOR_HUD_GFX_BOTTOM].LPDDS,
	   		&(HUDDDInfo[PREDATOR_HUD_GFX_BOTTOM].SrcRect),
	   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	   	);
*/	   	
		fprintf(stderr, "BLTPredatorOverlayToHUD: blit 1\n");
	}	
	else if ((ScreenDescriptorBlock.SDB_Height ==480) ||(ScreenDescriptorBlock.SDB_Width ==640) )
	{  	
/*
	   	ddrval = lpDDSBack->BltFast
	   	(
	   		1,
	   		13*2,
	   		HUDDDInfo[PREDATOR_HUD_GFX_TOP].LPDDS,
	   		&(HUDDDInfo[PREDATOR_HUD_GFX_TOP].SrcRect),
	   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	   	);
	   	ddrval = lpDDSBack->BltFast
	   	(
	   		1,
	   		136*2+80,
	   		HUDDDInfo[PREDATOR_HUD_GFX_BOTTOM].LPDDS,
	   		&(HUDDDInfo[PREDATOR_HUD_GFX_BOTTOM].SrcRect),
	   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	   	);
*/
		fprintf(stderr, "BLTPredatorOverlayToHUD: blit 2\n");
	}	
}
void BLTPredatorNumericsToHUD(void)
{
   	int digit = MAX_NO_OF_PREDATOR_HUD_DIGITS;

    struct DigitPropertiesTag *propertiesPtr;
	
	if (HUDResolution == HUD_RES_LO)
	{
		propertiesPtr = &LoresPredatorHUDDigitProperties[digit];
	}
	else 
	{
		propertiesPtr = &MedresPredatorHUDDigitProperties[digit];
	}
   
    while(digit)
    {
		digit--;
		propertiesPtr--;
    	/* paranoia check */
    	LOCALASSERT(ValueOfHUDDigit[digit]>=0 && ValueOfHUDDigit[digit]<=9);
        
        BLTPredatorDigitToHUD
		(
			ValueOfHUDDigit[digit],
			propertiesPtr->X,
			propertiesPtr->Y,
	       	propertiesPtr->Font
	    );
    }
}	
static void BLTPredatorDigitToHUD(char digit, int x, int y, int font)
{
//	HRESULT ddrval;
 	RECT srcRect;

	srcRect.top = digit*12;
	srcRect.bottom = digit*12+12;
	
   	srcRect.left = 0;
   	srcRect.right = HUDDDInfo[font].SrcRect.right;

/*	   
   	ddrval = lpDDSBack->BltFast
   	(
   		x,
   		y,
   		HUDDDInfo[font].LPDDS,
   		&srcRect,
   		DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
   	);
		
   	if(ddrval != DD_OK)
   	{
   		ReleaseDirect3D();
   		exit(0x666004);
	}
*/
	fprintf(stderr, "BLTPredatorDigitToHUD(%d, %d, %d, %d)\n", digit, x, y, font);	
}		  

/*KJL*********************
* ALIEN DRAWING ROUTINES *
*********************KJL*/
extern void BLTAlienOverlayToHUD(void)
{
	/* KJL 11:37:19 06/14/97 - draw whatever is in the execute buffer */
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		WriteEndCodeToExecuteBuffer();
		UnlockExecuteBufferAndPrepareForUse();
		ExecuteBuffer();
		EndD3DScene();
	}

	/* KJL 10:24:49 7/17/97 - no overlay, please */
	return;

//	HRESULT ddrval;
	if ((ScreenDescriptorBlock.SDB_Height ==200)&&(ScreenDescriptorBlock.SDB_Width ==320))
	{
/*
	 	ddrval = lpDDSBack->BltFast
		(
		  	0,
			0,
			HUDDDInfo[ALIEN_HUD_GFX_TOP].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_TOP].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	0,
			25,
			HUDDDInfo[ALIEN_HUD_GFX_LEFT].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_LEFT].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	320-48,
			25,
			HUDDDInfo[ALIEN_HUD_GFX_RIGHT].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_RIGHT].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	0,
			160,
			HUDDDInfo[ALIEN_HUD_GFX_BOTTOM].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_BOTTOM].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
*/
		fprintf(stderr, "BLTAlienOverlayToHUD: blit 1\n");	    
	}
	else if ((ScreenDescriptorBlock.SDB_Height ==480)&&(ScreenDescriptorBlock.SDB_Width ==640))
	{
/*
	 	ddrval = lpDDSBack->BltFast
		(
		  	0,
			0,
			HUDDDInfo[ALIEN_HUD_GFX_TOP].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_TOP].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	0,
			52,
			HUDDDInfo[ALIEN_HUD_GFX_LEFT].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_LEFT].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	640-97,
			52,
			HUDDDInfo[ALIEN_HUD_GFX_RIGHT].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_RIGHT].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
		ddrval = lpDDSBack->BltFast
		(
		  	0,
			480-97,
			HUDDDInfo[ALIEN_HUD_GFX_BOTTOM].LPDDS,
			&(HUDDDInfo[ALIEN_HUD_GFX_BOTTOM].SrcRect),
	        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	    );
*/
		fprintf(stderr, "BLTAlienOverlayToHUD: blit 2\n");
	}

}
void BLTAlienNumericsToHUD(void)
{

	if ((ScreenDescriptorBlock.SDB_Height ==200)&&(ScreenDescriptorBlock.SDB_Width ==320))
	{
	   	int digit = MAX_NO_OF_ALIEN_HUD_DIGITS;
    
	    while(digit)
	    {
			digit--;
	    	/* paranoia check */
	    	LOCALASSERT(ValueOfHUDDigit[digit]>=0 && ValueOfHUDDigit[digit]<=9);
				
	    	BLTDigitToHUD
			(
				ValueOfHUDDigit[digit],
				LoresAlienHUDDigitProperties[digit].X,
				LoresAlienHUDDigitProperties[digit].Y,
		        LoresAlienHUDDigitProperties[digit].Font
		    );
	    }
	}
	else if ((ScreenDescriptorBlock.SDB_Height ==480)&&(ScreenDescriptorBlock.SDB_Width ==640))
	{
	   	int digit = MAX_NO_OF_ALIEN_HUD_DIGITS;
    
	    while(digit)
	    {
			digit--;
	    	/* paranoia check */
	    	LOCALASSERT(ValueOfHUDDigit[digit]>=0 && ValueOfHUDDigit[digit]<=9);
				
	    	BLTDigitToHUD
			(
				ValueOfHUDDigit[digit],
				MedresAlienHUDDigitProperties[digit].X,
				MedresAlienHUDDigitProperties[digit].Y,
		        MedresAlienHUDDigitProperties[digit].Font
		    );
	    }
	}
}	




void BLTPausedToScreen(void)
{
/*
	lpDDSBack->BltFast
	(
	  	(ScreenDescriptorBlock.SDB_Width-PauseDDInfo.SrcRect.right)/2,
		(ScreenDescriptorBlock.SDB_Height-PauseDDInfo.SrcRect.bottom)/2,
		PauseDDInfo.LPDDS,
		&(PauseDDInfo.SrcRect),
        DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
    );
*/
	fprintf(stderr, "BLTPausedToScreen()\n");	
}



void LoadDDGraphic(struct DDGraphicTag *DDGfxPtr, char *Filename)
{
	/*
		set up the direct draw surface. we can take the width and height
		from the imageheader image
	*/

	GLOBALASSERT(DDGfxPtr);
    GLOBALASSERT(Filename);
    
	// get the filename that we need
	char szAbsFileName[MAX_PATH];
	char * pszRet = CL_GetImageFileName(szAbsFileName,sizeof szAbsFileName / sizeof szAbsFileName[0], Filename, LIO_DDSURFACE|LIO_SYSMEM|LIO_TRANSPARENT|LIO_CHROMAKEY|LIO_RIFFPATH|LIO_RESTORABLE);
	GLOBALASSERT(pszRet);
	
	// we'll put the width and height in here
	unsigned nWidth, nHeight;
	
	// is it in a fast file?
	unsigned nFastFileLen;
	void const * pFastFileData = ffreadbuf(szAbsFileName,&nFastFileLen);
	
	if (pFastFileData)
	{
		DDGfxPtr->LPDDS =
			AwCreateSurface
			(
				"pxfXYB",
				pFastFileData,
				nFastFileLen,
				AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
				&nWidth,
				&nHeight,
				&DDGfxPtr->hBackup
			);
	}
	else
	{
		DDGfxPtr->LPDDS =
			AwCreateSurface
			(
				"sfXYB",
				&szAbsFileName[0],
				AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
				&nWidth,
				&nHeight,
				&DDGfxPtr->hBackup
			);
	}
	
	GLOBALASSERT(DDGfxPtr->LPDDS);
	GLOBALASSERT(DDGfxPtr->hBackup);
	ATIncludeSurface(DDGfxPtr->LPDDS,DDGfxPtr->hBackup);

 	// set the rectangle size for blitting before padding to 4x4 has been done
 	DDGfxPtr->SrcRect.left = 0;
	DDGfxPtr->SrcRect.right = nWidth;
	DDGfxPtr->SrcRect.top = 0;
	DDGfxPtr->SrcRect.bottom = nHeight;

 	/*move the width and height to four byte bounadries*/

	GLOBALASSERT((DDGfxPtr->SrcRect.right > 0));
	GLOBALASSERT((DDGfxPtr->SrcRect.bottom > 0));
}

	
/* JH 3/6/97 functions to remove dd surfaces from hud graphics
   so that the video mode can be completely changed,
   but then everything can still be restored */
/* perhaps not a final solution since it will be occupying memory */

void MinimizeAllDDGraphics(void)
{
	/* do all in array - don't care how many actually are used
	   because the array is static (hence initially filled with zeros)
	   The release functions should replace with NULL a pointer
	   that is no longer valid */

	int gfxID = sizeof HUDDDInfo / sizeof (DDGraphicTag); // number of DDGraphicTags in array
	
	while(gfxID--)			      
    {
		if (HUDDDInfo[gfxID].LPDDS)
		{
			ATRemoveSurface(HUDDDInfo[gfxID].LPDDS);
//			HUDDDInfo[gfxID].LPDDS->Release();
			fprintf(stderr, "MinimizeAllDDGraphics: HUDDDInfo[gfxID].LPDDS\n");
			HUDDDInfo[gfxID].LPDDS = 0;
		}
	}
	
	if (PauseDDInfo.LPDDS)
	{
		ATRemoveSurface(PauseDDInfo.LPDDS);
//		PauseDDInfo.LPDDS->Release();
		fprintf(stderr, "MinimizeAllDDGraphics: PauseDDInfo.LPDDS\n");	
		PauseDDInfo.LPDDS = 0;
	}
	
	if (E3FontDDInfo.LPDDS)
	{
		ATRemoveSurface(E3FontDDInfo.LPDDS);
//		E3FontDDInfo.LPDDS->Release();	
		fprintf(stderr, "MinimizeAllDDGraphics: E3FontDDInfo.LPDDS\n");
		E3FontDDInfo.LPDDS = 0;
	}
}

void RestoreAllDDGraphics(void)
{
	/* do all in array - don't care how many actually are used
	   because the array is static (hence initially filled with zeros)
	   The release functions should replace with NULL a pointer
	   that is no longer valid */

	int gfxID = sizeof HUDDDInfo / sizeof (DDGraphicTag); // number of DDGraphicTags in array
	
	while(gfxID--)			      
    {
		if (HUDDDInfo[gfxID].hBackup)
		{
			HUDDDInfo[gfxID].LPDDS = AwCreateSurface("rf",HUDDDInfo[gfxID].hBackup,AW_TLF_PREVSRC|AW_TLF_CHROMAKEY);
			GLOBALASSERT(HUDDDInfo[gfxID].LPDDS);
			ATIncludeSurface(HUDDDInfo[gfxID].LPDDS,HUDDDInfo[gfxID].hBackup);
		}
	}
	
	if (PauseDDInfo.hBackup)
	{
		PauseDDInfo.LPDDS = AwCreateSurface("rf",PauseDDInfo.hBackup,AW_TLF_PREVSRC|AW_TLF_CHROMAKEY);
		GLOBALASSERT(PauseDDInfo.LPDDS);
		ATIncludeSurface(PauseDDInfo.LPDDS,PauseDDInfo.hBackup);
	}
	
	if (E3FontDDInfo.hBackup)
	{
		E3FontDDInfo.LPDDS = AwCreateSurface("rf",E3FontDDInfo.hBackup,AW_TLF_PREVSRC|AW_TLF_CHROMAKEY);
		GLOBALASSERT(E3FontDDInfo.LPDDS);
		ATIncludeSurface(E3FontDDInfo.LPDDS,E3FontDDInfo.hBackup);
	}
}
	

/************************** FONTS *************************/
/**********************************************************/
/**********************************************************/

//static int BLTFontCharToHUD(PFFONT* font , int xdest, int ydest, char todraw);

LPDIRECTDRAWSURFACE FontLPDDS[NUM_FONTS];

PFFONT AvpFonts[] =
{
	{
 		FontLPDDS[0],
 	 	"menufont.bmp",
	 	18,			// font height
	 	118,		// num chars
		I_FONT_UCLC_NUMERIC,
		{0}			//flags
		
 	},
	{
 		FontLPDDS[1],
 	 	"Common\\fontdark.RIM",
	 	14,			// font height
	 	59,		// num chars
		I_FONT_UC_NUMERIC
 	},
	{
 		FontLPDDS[2],
 	 	"Common\\fontlite.RIM",
	 	14,			// font height
	 	59,		// num chars
		I_FONT_UC_NUMERIC
 	},
	{
 		FontLPDDS[3],
 	 	"Common\\dbfont.RIM",
	 	11,	 // font height
	 	59,	 // num chars
		I_FONT_UC_NUMERIC
 	}
};

extern int VideoModeColourDepth;

void LoadFont(PFFONT *pffont)
{
	GLOBALASSERT(pffont);
	GLOBALASSERT(pffont->filename);
	
	// get the filename that we need
	char szAbsFileName[MAX_PATH];
	char * pszRet = CL_GetImageFileName(szAbsFileName,sizeof szAbsFileName / sizeof szAbsFileName[0], pffont->filename, LIO_DDSURFACE|LIO_SYSMEM|LIO_CHROMAKEY|LIO_TRANSPARENT
		// hack for the moment so that the menu font is correctly loaded into an 8-bit vid mode
		|(strchr(pffont->filename,'\\') ?  LIO_RELATIVEPATH : LIO_RIFFPATH));
	GLOBALASSERT(pszRet);
	
	
	// we'll put the width and height in here
	unsigned nWidth, nHeight;
	
	// is it in a fast file?
	unsigned nFastFileLen;
	void const * pFastFileData = ffreadbuf(szAbsFileName,&nFastFileLen);
	
	if (pFastFileData)
	{
		pffont->data =
			AwCreateSurface
			(
				"pxfXYB",
				pFastFileData,
				nFastFileLen,
				AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
				&nWidth,
				&nHeight,
				&pffont->hBackup
			);
	}
	else
	{
		pffont->data =
			AwCreateSurface
			(
				"sfXYB",
				&szAbsFileName[0],
				AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
				&nWidth,
				&nHeight,
				&pffont->hBackup
			);
	}
	
	GLOBALASSERT(pffont->data);
	GLOBALASSERT(pffont->hBackup);
	
	ATIncludeSurface(pffont->data,pffont->hBackup);
	
	pffont->fttexBitDepth = VideoModeColourDepth;
	
	pffont->fttexWidth = nWidth;
	pffont->fttexHeight = nHeight;
	
	GLOBALASSERT((nHeight > 0));
	GLOBALASSERT((nWidth > 0));

	pffont->flags.loaded = 1;
}


void * FontLock(PFFONT const * pFont, unsigned * pPitch)
{
	GLOBALASSERT(pFont);
	GLOBALASSERT(pFont->data);
	
	fprintf(stderr, "FontLock(%p, %p)\n", pFont, pPitch);
/*	
	DDSURFACEDESC ddsd;
	memset(&ddsd,0,sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	HRESULT hResult = pFont->data->Lock(NULL,&ddsd,DDLOCK_NOSYSLOCK,NULL);
	GLOBALASSERT(DD_OK == hResult);
	
	*pPitch = ddsd.lPitch;
	return ddsd.lpSurface;
*/
	return NULL;	
}

void FontUnlock(PFFONT const * pFont)
{
	GLOBALASSERT(pFont);
	GLOBALASSERT(pFont->data);
	
//	HRESULT hResult = pFont->data->Unlock(NULL);
//	GLOBALASSERT(DD_OK == hResult);
	fprintf(stderr, "FontUnlock(%p)\n", pFont);
}


void UnloadFont(PFFONT *pffont)
{
	GLOBALASSERT(pffont);
	
	if (pffont->hBackup)
	{
		ATRemoveSurface(pffont->data);
		AwDestroyBackupTexture(pffont->hBackup);
		pffont->hBackup = NULL;
	}
	
	if(pffont->data)
	{
		ReleaseDDSurface(pffont->data);
		pffont->data = NULL;
	}

	IndexedFont_Proportional_PF :: PFUnLoadHook
	(
		(FontIndex)
		(
			(pffont - &AvpFonts[0]) / sizeof(PFFONT)
		)
			// FontIndex I_Font_Old,
			// very ugly way to get at the index
	);
}


void FillCharacterSlot(int u, int v,
											int width, int height,
											int charnum, PFFONT* font)

{

	/*
		 simply set the srcRect.top to null in order to tell the drawing easily that
		 this char dosn't exist
	*/

	GLOBALASSERT(width > -1);
	GLOBALASSERT(height > -1);
	GLOBALASSERT(u > -1);
	GLOBALASSERT(v > -1);

	GLOBALASSERT(font);
	GLOBALASSERT(charnum < font->num_chars_in_font);
	font->srcRect[charnum].left = u;
	font->srcRect[charnum].right = u + width;
	font->srcRect[charnum].top = v;
	font->srcRect[charnum].bottom = v + height;
}



int BLTFontOffsetToHUD(PFFONT* font , int xdest, int ydest, int offset)
{
//	HRESULT ddrval;

	RECT *rect = &(font->srcRect[offset]);

	if(rect->right - rect->left <= 0)
		return 0;

	if(rect->bottom - rect->top <= 0)
		return(rect->right - rect->left	+ 1);

/*
  	ddrval = lpDDSBack->BltFast(xdest, ydest,	font->data, rect,	DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
  	
  	LOGDXERR(ddrval);
  	
	if(ddrval != DD_OK)
		{
			LOGDXERR(ddrval);
			GLOBALASSERT(0);
			finiObjects();
			exit(ddrval);
		}
*/
	fprintf(stderr, "BLTFontOffsetToHUD(%p, %d, %d, %d)\n", font, xdest, ydest, offset);
	
	return(font->srcRect[offset].right - font->srcRect[offset].left);
}



#endif // SBF





void YClipMotionTrackerVertices(struct VertexTag *v1, struct VertexTag *v2);
void XClipMotionTrackerVertices(struct VertexTag *v1, struct VertexTag *v2);

#if 0 /* SBF - not used */
static void DrawMotionTrackerPoly(void)
{
	struct VertexTag vertex[4];
	int widthCos,widthSin;

	{
		int angle = 4095 - Player->ObEuler.EulerY;
	
		widthCos = MUL_FIXED
				   (
				   		MotionTrackerWidth,
				   		GetCos(angle)
				   );
		widthSin = MUL_FIXED
				   (
				   		MotionTrackerWidth,
						GetSin(angle)
				   );
	}			
	
	/* I've put these -1s in here to help clipping 45 degree cases,
	where two vertices can end up around the clipping line of Y=0 */
	vertex[0].X = (-widthCos - (-widthSin))/2;
	vertex[0].Y = (-widthSin + (-widthCos))/2 -1;
	vertex[0].U	= 0;
	vertex[0].V	= 0;
	vertex[1].X = (widthCos - (-widthSin))/2;
	vertex[1].Y = (widthSin + (-widthCos))/2 -1;
	vertex[1].U = MotionTrackerTextureSize;
	vertex[1].V	= 0;
	vertex[2].X = (widthCos - widthSin)/2;
	vertex[2].Y = (widthSin + widthCos)/2 -1;
	vertex[2].U = MotionTrackerTextureSize;
	vertex[2].V = MotionTrackerTextureSize;
	vertex[3].X = ((-widthCos) - widthSin)/2;
	vertex[3].Y = ((-widthSin) + widthCos)/2 -1;
	vertex[3].U = 0;
	vertex[3].V = MotionTrackerTextureSize;

	/* clip to Y<=0 */
	YClipMotionTrackerVertices(&vertex[0],&vertex[1]);
	YClipMotionTrackerVertices(&vertex[1],&vertex[2]);
	YClipMotionTrackerVertices(&vertex[2],&vertex[3]);
	YClipMotionTrackerVertices(&vertex[3],&vertex[0]);

	/* translate into screen coords */
	vertex[0].X += MotionTrackerCentreX;
	vertex[1].X += MotionTrackerCentreX;
	vertex[2].X += MotionTrackerCentreX;
	vertex[3].X += MotionTrackerCentreX;
	vertex[0].Y += MotionTrackerCentreY;
	vertex[1].Y += MotionTrackerCentreY;
	vertex[2].Y += MotionTrackerCentreY;
	vertex[3].Y += MotionTrackerCentreY;
	#if 0
	textprint("%d %d   %d %d\n%d %d   %d %d\n%d %d   %d %d\n%d %d   %d %d\n",
		vertex[0].X,vertex[0].Y,vertex[0].U,vertex[0].V,
		vertex[1].X,vertex[1].Y,vertex[1].U,vertex[1].V,
		vertex[2].X,vertex[2].Y,vertex[2].U,vertex[2].V,
		vertex[3].X,vertex[3].Y,vertex[3].U,vertex[3].V);
	#endif
	
	/* dodgy offset 'cos I'm not x clipping */
	if (vertex[0].X==-1) vertex[0].X = 0;
	if (vertex[1].X==-1) vertex[1].X = 0;
	if (vertex[2].X==-1) vertex[2].X = 0;
	if (vertex[3].X==-1) vertex[3].X = 0;
		
	/* setup polygon in item format */
	{
		int *itemDataPtr = TrackerPolyBuffer+4;
		*itemDataPtr++ = vertex[3].X;
		*itemDataPtr++ = vertex[3].Y;
		*itemDataPtr++ = vertex[3].U;
		*itemDataPtr++ = vertex[3].V;

		*itemDataPtr++ = vertex[2].X;
		*itemDataPtr++ = vertex[2].Y;
		*itemDataPtr++ = vertex[2].U;
		*itemDataPtr++ = vertex[2].V;
		
		*itemDataPtr++ = vertex[1].X;
		*itemDataPtr++ = vertex[1].Y;
		*itemDataPtr++ = vertex[1].U;
		*itemDataPtr++ = vertex[1].V;

		*itemDataPtr++ = vertex[0].X;
		*itemDataPtr++ = vertex[0].Y;
		*itemDataPtr++ = vertex[0].U;
		*itemDataPtr++ = vertex[0].V;

		*itemDataPtr = Term;


		/* draw polygon */
		Draw_Item_2dTexturePolygon(TrackerPolyBuffer);
	}
}
#endif /* SBF */

void YClipMotionTrackerVertices(struct VertexTag *v1, struct VertexTag *v2)
{
	char vertex1Inside=0,vertex2Inside=0;

	if (v1->Y<0) vertex1Inside = 1;
	if (v2->Y<0) vertex2Inside = 1;

	/* if both vertices inside clip region no clipping required */
	if (vertex1Inside && vertex2Inside) return;

	/* if both vertices outside clip region no action required 
	(the other lines will be clipped) */
	if (!vertex1Inside && !vertex2Inside) return;

	/* okay, let's clip */
	if (vertex1Inside)
	{
		int lambda = DIV_FIXED(v1->Y,v2->Y - v1->Y);

		v2->X = v1->X - MUL_FIXED(v2->X - v1->X,lambda);
		v2->Y=0;

		v2->U = v1->U - MUL_FIXED(v2->U - v1->U,lambda);
		v2->V = v1->V - MUL_FIXED(v2->V - v1->V,lambda);
	}
	else
	{
		int lambda = DIV_FIXED(v2->Y,v1->Y - v2->Y);

		v1->X = v2->X - MUL_FIXED(v1->X - v2->X,lambda);
		v1->Y=0;

		v1->U = v2->U - MUL_FIXED(v1->U - v2->U,lambda);
		v1->V = v2->V - MUL_FIXED(v1->V - v2->V,lambda);
	}
}
void XClipMotionTrackerVertices(struct VertexTag *v1, struct VertexTag *v2)
{
	char vertex1Inside=0,vertex2Inside=0;

	if (v1->X>0) vertex1Inside = 1;
	if (v1->X>0) vertex1Inside = 1;

	/* if both vertices inside clip region no clipping required */
	if (vertex1Inside && vertex2Inside) return;

	/* if both vertices outside clip region no action required 
	(the other lines will be clipped) */
	if (!vertex1Inside && !vertex2Inside) return;

	/* okay, let's clip */
	if (vertex1Inside)
	{
		int lambda = DIV_FIXED(v1->X,v2->X - v1->X);

		v2->Y = v1->Y - MUL_FIXED(v2->Y - v1->Y,lambda);
		v2->X=0;

		v2->U = v1->U - MUL_FIXED(v2->U - v1->U,lambda);
		v2->V = v1->V - MUL_FIXED(v2->V - v1->V,lambda);
	}
	else
	{
		int lambda = DIV_FIXED(v2->X,v1->X - v2->X);

		v1->Y = v2->Y - MUL_FIXED(v1->Y - v2->Y,lambda);
		v1->X=0;

		v1->U = v2->U - MUL_FIXED(v1->U - v2->U,lambda);
		v1->V = v2->V - MUL_FIXED(v1->V - v2->V,lambda);
	}
}				    

#if 0 // SBF - unused
static void SetupScanlinePoly(char const *filenamePtr, int width)
{
	int imageNumber;
	int height;

	imageNumber = CL_LoadImageOnce(filenamePtr, (ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)|LIO_TRANSPARENT|LIO_RIFFPATH|LIO_RESTORABLE);
	height = width/2;
								
	ScanlinePolyBuffer[3] = imageNumber;

	ScanlinePolyBuffer[6] = 0;
	ScanlinePolyBuffer[7] = height;

	ScanlinePolyBuffer[10] = width;
	ScanlinePolyBuffer[11] = height;

	ScanlinePolyBuffer[14] = width;
	ScanlinePolyBuffer[15] = 0;

	ScanlinePolyBuffer[18] = 0;
	ScanlinePolyBuffer[19] = 0;

	ScanlinePolyBuffer[20] = Term;
}
#endif // SBF

}; // extern 
