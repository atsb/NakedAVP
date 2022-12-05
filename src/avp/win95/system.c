/* inits the system and controls environment and player loading */

#include "3dc.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_marin.h"
#include "game.h"
#include "gameplat.h"
#include "lighting.h"
#include "messagehistory.h"
#include "particle.h"
#include "pldnet.h"
#define UseLocalAssert Yes
#include "ourasert.h"

/* patrick 5/12/96 */
#include "bh_far.h"
#include "pheromon.h"
#include "huddefs.h"
#include "hud.h"
//#include "hudgfx.h"
#include "fmv.h"
#include "font.h"
#include "bh_gener.h"
#include "pvisible.h"
#include "system.h"

#include "projload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?
#include "chnkload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?

#include "ffstdio.h" // fast file stdio
#include "avp_menus.h"
#include "cdplayer.h"

/*------------Patrick 1/6/97---------------
New sound system 
-------------------------------------------*/
#include "psndplat.h"
#include "progress_bar.h"
#include "bh_rubberduck.h"
#include "game_statistics.h"
#include "cdtrackselection.h"


// EXTERNS



extern int WindowMode;
extern int VideoMode;

extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

extern SHAPEHEADER** mainshapelist;

extern int NumActiveBlocks;
extern int NumOnScreenBlocks;
extern DISPLAYBLOCK *ActiveBlockList[];

extern MODULEMAPBLOCK AvpCompiledMaps[];
extern MAPHEADER TestMap[];
extern MAPHEADER Map[];
extern MAPHEADER * staticmaplist[];
extern MAPBLOCK8 Player_and_Camera_Type8[];

extern SCENEMODULE **Global_ModulePtr;
extern SCENEMODULE *MainSceneArray[];

extern void (*SetVideoMode[]) (void);
extern int HWAccel;
extern int Resolution;
extern void SetupVision(void);
extern void ReInitHUD(void);
extern void CheckCDStatus(void);

extern void DeallocateSoundsAndPoolAllocatedMemory();


/*Globals */

int WindowRequestMode;
int VideoRequestMode;
int ZBufferRequestMode;
int RasterisationRequestMode;
int SoftwareScanDrawRequestMode;
int DXMemoryRequestMode;
WINSCALEXY TopLeftSubWindow;
WINSCALEXY ExtentXYSubWindow;


// static 

static int ReadModuleMapList(MODULEMAPBLOCK *mmbptr);
RIFFHANDLE env_rif = INVALID_RIFFHANDLE;
RIFFHANDLE player_rif = INVALID_RIFFHANDLE;
RIFFHANDLE alien_weapon_rif = INVALID_RIFFHANDLE;
RIFFHANDLE marine_weapon_rif = INVALID_RIFFHANDLE;
RIFFHANDLE predator_weapon_rif = INVALID_RIFFHANDLE;

void ProcessSystemObjects();

/*

 Video Mode at start-up

 The source to set up the initial video mode obviously
 depends on the platform, and so this routine is best placed
 in this file

*/

/* THIS IS FOR THE MENU SCREEN */

void InitialVideoMode(void)

{
/*
	Note that video modes are now only
	REQUESTED, not set.  
	Sound will also be dealt with in the 
	same way.
*/


	ScreenDescriptorBlock.SDB_Flags = SDB_Flag_Raw256 | SDB_Flag_MIP;

    /*
		Setup for Windows mode.  Note that
		TopLeftSubWindow and ExtentXYSubWindow
		should be set to sensible defaults even
		if we are in FullScreen mode.
		Also: TopLeftSubWindow and ExtentXYSubWindow
		are proportions of the current Windows display
		extents, set in floating point in a range of
		0.0-1.0 THIS CODE IS INTENDED FOR PENTIUM TARGETS
		ONLY, AND THEREFORE USES FLOATING PT.  IT MUST
		NOT
		 LEAK.
	*/

    /*
	  Note this is now only a request mode.
	*/

    #if 1
    WindowRequestMode = WindowModeFullScreen;
	#else
    WindowRequestMode = WindowModeSubWindow;
	#endif

    TopLeftSubWindow.x = 0.3;
	TopLeftSubWindow.y = 0.3;
	ExtentXYSubWindow.x = 0.6;
	ExtentXYSubWindow.y = 0.6;

/*
	Experimental settings for other 
	request modes affecting rendering.
*/

	/*VideoRequestMode = VideoMode_DX_640x480x8;  for menus - is this guaranteed? */
	VideoRequestMode = AvP.MenuVideoRequestMode;
	
	/* JH 20/5/97
		begin in minmal h/w configuration - for menus
		- don't need any h/w 3d, but do need to know
		about h/w direct draw and what video modes
		will be available */

	ZBufferRequestMode = RequestZBufferNever;

    RasterisationRequestMode = RequestDefaultRasterisation;

    SoftwareScanDrawRequestMode = RequestScanDrawDirectDraw; 

    DXMemoryRequestMode = RequestSystemMemoryAlways;

    /*
		IMPORTANT!!!! In the Windows 95 version,
		SetVideoMode MUST NOT be called from this
		routine, since InitialVideoMode is called
		first in WinMain to prepare for windows
		initialisation and the actual DirectX 
		initialisation (done through SetVideoMode)
		must be done after windows initialisation
	*/

}




/****************** AVP Change Display Modes*/

/*
	This function is intended to allow YOU,
	the user, to obtain your heart's fondest desires
	by one simple call.  Money? Love? A better job?
	It's all here, you have only to ask...
	No, I was lying actually.
	In fact, this should allow you to change
	display modes cleanly.  Pass your request modes
	(as normally set up in system.c).  For all entries
	which you do not want to change, simply pass
	the current global value (e.g. ZBufferRequestMode
	in the NewZBufferMode entry).

    Note that the function must always be passed the
	HINSTANCE and nCmdShow from winmain.
*/

/*
	Note that this function will NOT
	reinitialise the DirectDraw object
	or switch to or from a hardware DD
	device, but it will release and rebuild
	all the Direct3D objects.
*/

/*
	Note that you MUST be in the right
	directory for a texture reload before you
	call this, and normal operations CAN change
	the directory...
*/

/*
	NOTE!!! If you start in DirectDraw mode
	and go to Direct3D mode, this function
	CANNOT POSSIBLY WORK WITHOUT A FULL SHAPE 
	RELOAD, since the shape data is overwritten
	during DirectDraw initialisation!!!!

    NOTE ALSO: TEXTURE RELOAD MAY BE DODGY 
	WITHOUT A SHAPE RELOAD!!!
*/


int AVP_ChangeDisplayMode
		(
			HINSTANCE hInst, 
			int nCmd, 
			int NewVideoMode, 
			int NewWindowMode,
			int NewZBufferMode, 
			int NewRasterisationMode, 
			int NewSoftwareScanDrawMode, 
			int NewDXMemoryMode
		)
{
	BOOL ChangeWindow = No;

	/*
		Shut down DirectX objects and destroy
		the current window, if necessary.
	*/

    if (NewWindowMode != WindowMode)
		{
		  ChangeWindow = Yes;
		}

    /* JH 30/5/97 - added this line back in so that d3d is cleaned up properly when the
	   display is changed back to 8-but for the menus */
	/* JH 3/6/97 - don't quit kill off the images - still keep buffers in system memory
	   that are not linked to direct draw */
    MinimizeAllImages();
    ReleaseDirect3DNotDDOrImages();

    finiObjectsExceptDD();

    if (ChangeWindow)
      ExitWindowsSystem(); 


	/*
		Set the request modes and actual modes
		according to the passed values.
	*/

    VideoRequestMode = NewVideoMode;
    WindowRequestMode = NewWindowMode;
	ZBufferRequestMode = NewZBufferMode;
	RasterisationRequestMode = NewRasterisationMode;
	SoftwareScanDrawRequestMode = NewSoftwareScanDrawMode;
	DXMemoryRequestMode = NewDXMemoryMode;

    VideoMode = VideoRequestMode;
	WindowMode = WindowRequestMode;

	/* this may reconstruct the dd object depending
	   on the rasterisation request mode and whether
	   a hardware dd driver is selected or could be
	   available - JH 20/5/97 */
	ChangeDirectDrawObject();

	/*
		Recreate the window, allowing
		for possible change in WindowMode.
	*/

    if (ChangeWindow)
	  {
	   	BOOL rc = InitialiseWindowsSystem
		(
			hInst, 
			nCmd, 
			WinInitChange
		);

       	if (rc == FALSE)
	     	return rc;
	  }

	/*
		Set the video mode again.  This
		will handle all changes to DirectDraw
		objects, all Direct3D initialisation,
		and other request modes such as
		zbuffering.
	*/
/*
	SetVideoMode[VideoMode]();
*/


    return TRUE;
}


//void ReleaseDirect3DNotDDOrImages(void)
//{
//    RELEASE(d3d.lpD3DViewport);
//    RELEASE(d3d.lpD3DDevice);
//    RELEASE(d3d.lpD3D);
//}


//empty functions for hooks
// hooks for doing stuff after drawing and
// after a flip

void ProjectSpecificItemListPostProcessing(void){;}
void ProjectSpecificBufferFlipPostProcessing(void){;}



/*******************************************************************************************/
/*******************************************************************************************/

/***************						GAME AND ENIVROMENT CONTROL 					**************************/



void InitCharacter()
{
	/*** RWH cleans up the character initialisation 
			 it would be nice if this can be called when
			 we load up a game of a different character
	***/
	
	// load charcater specific rif and sounds


	if(player_rif != INVALID_RIFFHANDLE)
	{
			// we already have a player loaded - delete the bastard
			avp_undo_rif_load(player_rif);
	}
	if(alien_weapon_rif != INVALID_RIFFHANDLE)
	{
			// we already have a player loaded - delete the bastard
		avp_undo_rif_load(alien_weapon_rif);
	}
	if(marine_weapon_rif != INVALID_RIFFHANDLE)
	{
			// we already have a player loaded - delete the bastard
		avp_undo_rif_load(marine_weapon_rif);
	}
	if(predator_weapon_rif != INVALID_RIFFHANDLE)
	{
			// we already have a player loaded - delete the bastard
		avp_undo_rif_load(predator_weapon_rif);
	}
	
	#if MaxImageGroups==1
	InitialiseTextures();
	#else
	SetCurrentImageGroup(0);
	DeallocateCurrentImages();
	#endif
	
	Start_Progress_Bar();

	
	Set_Progress_Bar_Position(PBAR_HUD_START);

	switch(AvP.Network)
	{
		case I_No_Network:
		{
			

			// set up the standard single player game
			switch(AvP.PlayerType)
				{
					case I_Marine:
						{
							marine_weapon_rif = avp_load_rif("avp_huds/marwep.rif");
							Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.25);
							player_rif = avp_load_rif("avp_huds/marine.rif");
							break;
						}
					case I_Predator:
						{
							predator_weapon_rif = avp_load_rif("avp_huds/pred_hud.rif");
							Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.25);
							player_rif = avp_load_rif("avp_huds/predator.rif");
							break;
						}

					case I_Alien:
						{
							#if ALIEN_DEMO
							alien_weapon_rif = avp_load_rif("alienavp_huds/alien_hud.rif");
							Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.25);
							player_rif = avp_load_rif("alienavp_huds/alien.rif");
							#else
							alien_weapon_rif = avp_load_rif("avp_huds/alien_hud.rif");
							Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.25);
							player_rif = avp_load_rif("avp_huds/alien.rif");
							#endif
							break;
						}
					default:
						{
							GLOBALASSERT(2<1);
						}
				}
				break;
			}
			default:
			{
				

				
				// set up a multiplayer game - here becuse we might end
				// up with a cooperative game
				//load all weapon rifs
				marine_weapon_rif = avp_load_rif("avp_huds/marwep.rif");
				predator_weapon_rif = avp_load_rif("avp_huds/pred_hud.rif");
				alien_weapon_rif = avp_load_rif("avp_huds/alien_hud.rif");
				
				Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.25);
				player_rif = avp_load_rif("avp_huds/multip.rif");
			}
	}
	Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.5);

	#if MaxImageGroups>1
	SetCurrentImageGroup(0);
	#endif
	copy_rif_data(player_rif,CCF_IMAGEGROUPSET,PBAR_HUD_START+PBAR_HUD_INTERVAL*.5,PBAR_HUD_INTERVAL*.25);
	
	Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL*.75);
	
	
	if(alien_weapon_rif!=INVALID_RIFFHANDLE)
		copy_rif_data(alien_weapon_rif,CCF_LOAD_AS_HIERARCHY_IF_EXISTS|CCF_IMAGEGROUPSET|CCF_DONT_INITIALISE_TEXTURES,PBAR_HUD_START+PBAR_HUD_INTERVAL*.5,PBAR_HUD_INTERVAL*.25);

	if(marine_weapon_rif!=INVALID_RIFFHANDLE)
		copy_rif_data(marine_weapon_rif,CCF_LOAD_AS_HIERARCHY_IF_EXISTS|CCF_IMAGEGROUPSET|CCF_DONT_INITIALISE_TEXTURES,PBAR_HUD_START+PBAR_HUD_INTERVAL*.5,PBAR_HUD_INTERVAL*.25);

	if(predator_weapon_rif!=INVALID_RIFFHANDLE)
		copy_rif_data(predator_weapon_rif,CCF_LOAD_AS_HIERARCHY_IF_EXISTS|CCF_IMAGEGROUPSET|CCF_DONT_INITIALISE_TEXTURES,PBAR_HUD_START+PBAR_HUD_INTERVAL*.5,PBAR_HUD_INTERVAL*.25);

	Set_Progress_Bar_Position(PBAR_HUD_START+PBAR_HUD_INTERVAL);
	//copy_chunks_from_environment(0);

	/*KJL*************************************
	*   Setup generic data for weapons etc   *
	*************************************KJL*/

 	InitialiseEquipment();
	InitHUD();
	
}

extern void create_strategies_from_list ();
extern void AssignAllSBNames();

void RestartLevel()
{
	//get the cd to start again at the beginning of the play list.
	ResetCDPlayForLevel();
	
	CleanUpPheromoneSystem();
	// now deallocate the module vis array
	DeallocateModuleVisArrays();
	
	/* destroy the VDB list */	
	InitialiseVDBs();
	InitialiseTxAnimBlocks(); 
	
	
	// deallocate strategy and display blocks
	{
		int i ;

		i = maxstblocks;
		DestroyAllStrategyBlocks();
		while(i--)
			ActiveStBlockList[i] = NULL;

		i = maxobjects;
	 	InitialiseObjectBlocks();
		while(i --)
			ActiveBlockList[i] = NULL;
	}

	//stop all sound
	SoundSys_StopAll();
	
	//reset the displayblock for modules to 0
	{
		int i=2;
		while(MainScene.sm_module[i].m_type!=mtype_term)
		{
			MainScene.sm_module[i].m_dptr=0;
			i++;
		}

	}
 	
	// set the Onscreenbloock lsit to zero
 	NumOnScreenBlocks = 0;
 	
 	//start reinitialising stuff
 	
// 	InitialiseEquipment();
//	InitHUD();
	
	ProcessSystemObjects();
	
	create_strategies_from_list ();
	AssignAllSBNames();
	
	SetupVision();
	InitObjectVisibilities();
	InitPheromoneSystem();
	InitHive();
	InitSquad();
	
	/* KJL 14:22:41 17/11/98 - reset HUD data, such as where the crosshair is,
	whether the Alien jaw is on-screen, and so on */
	ReInitHUD();
	
	InitialiseParticleSystem();
	InitialiseSfxBlocks();
	InitialiseLightElementSystem();
	CreateRubberDucks();
	InitialiseTriggeredFMVs();

	CheckCDStatus();

	/*Make sure we don't get a slow frame when we restart , since this can cause problems*/
	ResetFrameCounter();

	CurrentGameStats_Initialise();
	MessageHistory_Initialise();
	
	if(AvP.Network!=I_No_Network)
	{
		TeleportNetPlayerToAStartingPosition(Player->ObStrategyBlock,1);
	}
	else
	{
		//make sure the visibilities are up to date
		extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;
		Global_VDB_Ptr->VDB_World = Player->ObWorld;
		AllNewModuleHandler();
		DoObjectVisibilities();
	}
}

static ELO JunkEnv; /* This is not needed */
ELO* Env_List[I_Num_Environments] = { &JunkEnv };

/**** Construct filename and go for it ***************/

void catpathandextension(char*, char*);
void DestroyActiveBlockList(void);
void InitialiseObjectBlocks(void);	


char EnvFileName[100];
char LevelDir[100];

void ProcessSystemObjects()
{
	int i;

	MODULEMAPBLOCK* mmbptr= &AvpCompiledMaps[0];
	STRATEGYBLOCK* sbptr;

	/* PC Loading.
		 1 LoadRif File 
		 			a sets up precompiled shapes
					b	sets up other loaded shapes
					c sets maps ans SBs for loaded maps
					
		 2 
	*/	


	#if TestRiffLoaders
	ReadMap(Map);							 /* for chunck loader*/
	ReadModuleMapList(mmbptr);
	#else
	#if SupportModules
	ReadModuleMapList(mmbptr);
	#endif /*SupportModules*/
	ReadMap(Map);	
	#endif

	/*HACK HACK*/

	sbptr = AttachNewStratBlock((MODULE*)NULL,
															(MODULEMAPBLOCK*)&Player_and_Camera_Type8[0],
															Player);
	AssignRunTimeBehaviours(sbptr);

	#if SupportModules

	Global_ModulePtr = MainSceneArray;
	PreprocessAllModules();
	i = GetModuleVisArrays();
	if(i == No) textprint("GetModuleVisArrays() failed\n");


	/*WaitForReturn();*/

	#endif
}

static int ReadModuleMapList(MODULEMAPBLOCK *mmbptr)
{
	MODULE m_temp;

	DISPLAYBLOCK *dptr;
	STRATEGYBLOCK *sbptr;
	/* this automatically attaches sbs to dbs */

	while(mmbptr->MapType != MapType_Term)
		{
			m_temp.m_mapptr = mmbptr;
			m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
			m_temp.m_dptr = NULL;
			AllocateModuleObject(&m_temp); 
			dptr = m_temp.m_dptr;
			LOCALASSERT(dptr); /* if this fires, cannot allocate displayblock */
			dptr->ObMyModule = NULL;
			sbptr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dptr);
			/* enable compile in behaviours here */
			AssignRunTimeBehaviours(sbptr);

			mmbptr++;
		}

	return(0);
}
	

void UnloadRifFile()
{
	unload_rif(env_rif);
}  


void ChangeEnvironmentToEnv(I_AVP_ENVIRONMENTS env_to_load)
{

	GLOBALASSERT(env_to_load != AvP.CurrentEnv);
 
	GLOBALASSERT(Env_List[env_to_load]);

	Destroy_CurrentEnvironment(); 
	/* Patrick: 26/6/97
	Stop and remove all sounds here */	
	SoundSys_StopAll();
	SoundSys_RemoveAll(); 
	CDDA_Stop();

	// Loading functions
	AvP.CurrentEnv = env_to_load;
	LoadRifFile();

}


void IntegrateNewEnvironment()
{
	int i;
	MODULEMAPBLOCK* mmbptr= &AvpCompiledMaps[0];

	// elements we need form processsystemobjects

	ReadMap(Map);							 /* for chunck loader*/
	ReadModuleMapList(mmbptr);

	Global_ModulePtr = MainSceneArray;
	PreprocessAllModules();
	i = GetModuleVisArrays();
	if(i == No) textprint("GetModuleVisArrays() failed\n");

 
	// elements from start game for AI

	InitObjectVisibilities();
	InitPheromoneSystem();
	BuildFarModuleLocs();
	InitHive();

	AssignAllSBNames();

	/* KJL 20:54:55 05/15/97 - setup player vision (alien wideangle, etc) */
	SetupVision();

	UnloadRifFile();//deletes environment File_Chunk since it is no longer needed

	/* Patrick: 26/6/97
	Load our sounds for the new env */	
	LoadSounds("PLAYER");

	/* remove resident loaded 'fast' files */
	ffcloseall();

	ResetFrameCounter();
}


const char GameDataDirName[20] = {"avp_rifs"};
const char FileNameExtension[5] =  {".rif"};
 
void LoadRifFile()
{
	
	char file_and_path[100];
	int i = 0;
	
	Set_Progress_Bar_Position(PBAR_LEVEL_START);
	
	// clear the dir names

	for(i = 0; i < 100; i++)
	  {
	  	file_and_path[i] = (char)0;
			EnvFileName[i] = (char)0;
			LevelDir[i] = (char)0;
	  }

	// Set up the dirname for the Rif load
				
	catpathandextension(&file_and_path[0], (char *)&GameDataDirName[0]);
	catpathandextension(&file_and_path[0], Env_List[AvP.CurrentEnv]->main); /* root of the file name,smae as dir*/
	catpathandextension(&file_and_path[0], (char *)&FileNameExtension[0]);	/* extension*/
	
	env_rif = avp_load_rif((const char*)&file_and_path[0]);
	Set_Progress_Bar_Position(PBAR_LEVEL_START+PBAR_LEVEL_INTERVAL*.4);
	
	if(INVALID_RIFFHANDLE == env_rif)
	  {
			finiObjects();
			exit(0x3421);
				
	  };

	#if MaxImageGroups>1
	SetCurrentImageGroup(2); // FOR ENV
	#endif
	copy_rif_data(env_rif,CCF_ENVIRONMENT,PBAR_LEVEL_START+PBAR_LEVEL_INTERVAL*.4,PBAR_LEVEL_INTERVAL*.6);
	//setup_shading_tables();
}

int Destroy_CurrentEnvironment(void)
{
	// RWH destroys all en specific data

	// function to change environment when we 
	// are playing a game	- environmnet reset
	
	// this stores all info we need

	TimeStampedMessage("Beginning Destroy_CurrentEnvironment");
	//CreateLevelMetablocks(AvP.CurrentEnv);
	TimeStampedMessage("After CreateLevelMetablocks");

	/*----------------------Patrick 14/3/97-----------------------
	  Clean up AI systems at end of level
	--------------------------------------------------------------*/

	{
		int i ;

		i = maxstblocks;
		DestroyAllStrategyBlocks();
		while(i--)
			ActiveStBlockList[i] = NULL;

		i = maxobjects;
	 	InitialiseObjectBlocks();
		while(i --)
			ActiveBlockList[i] = NULL;
	}
	TimeStampedMessage("After object blocks");
	
	//Get rid of all sounds
	//Deallocate memory for all shapes and hierarchy animations
	DeallocateSoundsAndPoolAllocatedMemory();
	
	KillFarModuleLocs();
	TimeStampedMessage("After KillFarModuleLocs");
	CleanUpPheromoneSystem();
	TimeStampedMessage("After CleanUpPheromoneSystem");
	
	#if MaxImageGroups>1
	SetCurrentImageGroup(2); // FOR ENV
	TimeStampedMessage("After SetCurrentImageGroup");

	DeallocateCurrentImages();
	TimeStampedMessage("After DeallocateCurrentImages");
	#endif
	// now deasllocate the module vis array
	DeallocateModuleVisArrays();
	TimeStampedMessage("After DeallocateModuleVisArrays");

		

	/* destroy the VDB list */	
	InitialiseVDBs();
	TimeStampedMessage("After InitialiseVDBs");

	
	InitialiseTxAnimBlocks(); // RUN THE npcS ON OUR OWN
	TimeStampedMessage("After InitialiseTxAnimBlocks");


	/* frees the memory from the env load*/
	DeallocateModules();
	TimeStampedMessage("After DeallocateModules");

	avp_undo_rif_load(env_rif);
	TimeStampedMessage("After avp_undo_rif_load");


	// set the Onscreenbloock lsit to zero
 	NumOnScreenBlocks = 0;
 
 	return(0);
}



#if 0
void InitEnvironmentFromLoad(void) 
{
	// in DB menus - we only destroy the current environment
	// after we have selected a leve, to load - WE could
	// be going TO ANY ENV or CHARACTER here (ughh) 

	// this is an entire game destroy (with no save) killing
	// both the env and the character followed by a complete
	// game restart 
	
	// environment clean up - sets up the load info
	Destroy_CurrentEnvironment();
	// then the REST
	DestroyAllStrategyBlocks();
	#if MaxImageGroups>1
	SetCurrentImageGroup(0); // FOR ENV
	DeallocateCurrentImages();
	#endif
	/* Patrick: 26/6/97
	Stop and remove all sounds here */	
	SoundSys_StopAll();
	SoundSys_RemoveAll(); 
	CDDA_Stop();

	// start the loading - we load the player
	InitCharacter();	// intis the char
	LoadRifFile();    // env

	// do all the ness processing
	// start games calles FormatSaveBuffer and
	// Process System Objects

	AssignAllSBNames();
	
	// Set the timer, or we have just taken
	// 10 secs for the frame

	/***** No need to do frame counter stuff in a computer! *****/

	/* Patrick: 26/6/97
	Load our sounds for the new env */	
	LoadSounds("PLAYER");
}



/************************ SAVE AND LOAD **********************/








void LoadGameFromFile(void)
{
	// now we right to a file
	char * savename = "slot1.AvP";
	FILE* fp = fopen(savename, "rb");
	if(fp == NULL)
		return;
	fread(&AvP, sizeof(AVP_GAME_DESC), 1, fp);
	fread(&save_game_buffer, SAVEBUFFERSIZE, 1, fp);
	UnpackSaveBuffer();
	fclose(fp);
}


void SaveGameToFile(void)
{
	char * savename = "slot1.AvP";
	FILE* fp = fopen(savename, "wb");
	CreateLevelMetablocks(AvP.CurrentEnv);
	PackSaveBuffer();
	fwrite(&AvP, sizeof(AVP_GAME_DESC), 1, fp);
	fwrite(&save_game_buffer, SAVEBUFFERSIZE, 1, fp);
	fclose(fp);
}

#endif

// project spec game exit
void ExitGame(void)
{
	if(player_rif != INVALID_RIFFHANDLE)
	{
		avp_undo_rif_load(player_rif);
	  	player_rif=INVALID_RIFFHANDLE;

	}
	
	if(alien_weapon_rif != INVALID_RIFFHANDLE)
	{
		avp_undo_rif_load(alien_weapon_rif);
		alien_weapon_rif=INVALID_RIFFHANDLE;
	}
	if(marine_weapon_rif != INVALID_RIFFHANDLE)
	{
		avp_undo_rif_load(marine_weapon_rif);
		marine_weapon_rif=INVALID_RIFFHANDLE;
	}
	if(predator_weapon_rif != INVALID_RIFFHANDLE)
	{
		avp_undo_rif_load(predator_weapon_rif);
		predator_weapon_rif=INVALID_RIFFHANDLE;
	}
	#if MaxImageGroups>1
	SetCurrentImageGroup(0);
	DeallocateCurrentImages();
	#endif
}
