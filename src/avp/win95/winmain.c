/* Main designed spec for use with windows95*/

#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"
#include "usr_io.h"
#include "font.h"

/* JH 27/1/97 */
#include "comp_shp.h"

#include "chnkload.hpp"
#include "npcsetup.h" /* JH 30/4/97 */
#include "krender.h"
#include "pldnet.h"
#include "avpview.h"
#include "scrshot.hpp"
#include "language.h"
#include "huddefs.h"
#include "vision.h"
#include "pcmenus.h"
#include "multmenu.h"
#include "menudefs.h"
#include "database.h"
#include "avp_menus.h"
#include "kshape.h"
#define UseLocalAssert Yes

#include "ourasert.h" 

#include "ffstdio.h" // fast file stdio

#include "davehook.h"

#include "rebmenus.hpp"
#include "intro.hpp"
#include "showcmds.h"

#include "consbind.hpp"

#include "AvpReg.hpp"
#include "mempool.h"
#include "GammaControl.h"

#include "CDTrackSelection.h"

/*------------Patrick 1/6/97---------------
New sound system 
-------------------------------------------*/
#include "psndplat.h"

#define FRAMEAV 100

#include "AvP_UserProfile.h"

#define PROFILING_ON 0
#if PROFILING_ON
#include "pentime.h"
#endif

#if 0
#undef PrintDebuggingText
extern int PrintDebuggingText(const char* t, ...);
#endif

/*

 externs for commonly used global variables and arrays

*/
extern int VideoMode;
extern DISPLAYBLOCK* ActiveBlockList[];
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern void (*SetVideoMode[]) (void);
extern int FrameRate;

extern int WindowRequestMode;

extern int NumActiveBlocks;
int HWAccel = 0;

#if debug
#define MainTextPrint 1
extern int alloc_cnt, deall_cnt;
extern int ItemCount;
int DebugFontLoaded = 0;

extern BOOL ForceLoad_Alien;
extern BOOL ForceLoad_Marine;
extern BOOL ForceLoad_Predator;
extern BOOL ForceLoad_Hugger;
extern BOOL ForceLoad_Queen;
extern BOOL ForceLoad_Civvie;
extern BOOL ForceLoad_PredAlien;
extern BOOL ForceLoad_Xenoborg;
extern BOOL ForceLoad_Pretorian;
extern BOOL ForceLoad_SentryGun;


BOOL UseMouseCentreing = FALSE;
#else
#define MainTextPrint 0
#endif

BOOL KeepMainRifFile=FALSE;

extern void LoadKeyConfiguration();
extern void DeInitialisePlayer();

extern int AvP_MainMenus(void);
extern int AvP_InGameMenus(void);

extern IngameKeyboardInput_ClearBuffer(void);

HINSTANCE AVP_HInstance, hInst;
int AVP_NCmd;

extern unsigned long TotalMemAllocated;

char LevelName[] = {"predbit6\0QuiteALongNameActually"};
static ELO ELOLevelToLoad = {&LevelName};

int QuickStartMultiplayer=1;

int VideoModeNotAvailable=0;

extern int DebuggingCommandsActive;

extern void BuildMultiplayerLevelNameArray();

extern int WindowMode;
void exit_break_point_fucntion ()
{
	#if debug
	if (WindowMode == WindowModeSubWindow)
	{
		__asm int 3;
	}
	#endif
}	

 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine, int nCmdShow)
{
	char * instr;
	#if debug
	int level_to_load = I_Num_Environments;
	char * command_line = lpCmdLine;
	#endif
						
	AVP_HInstance = hInst = hInstance;
	AVP_NCmd = nCmdShow;

	EnumerateCardsAndVideoModes();

	LoadCDTrackList(); //load list of cd tracks assigned to levels , from a text file

	
//	CDDA_Start();
//	CDDA_Play(1);

	SetFastRandom();
	
	/**** init game now ONLY sets up varibles for the whole
				game. If you want to put something in it it must
				be something that only needs to be called once
	****/
	#if debug && 1//!PREDATOR_DEMO
	{
		#if OverrideOldMenus
		{
			HWAccel = 1;
 		}
		#else
		if(strstr(command_line, "-h"))
		{
			HWAccel = 1;
		}
		#endif

		//see if any extra npc rif files should be loaded
		{
			char* strpos=strstr(command_line, "-l");
			if(strpos)
			{
				while(strpos)
				{
					strpos+=2;
					if(*strpos>='a' && *strpos<='z')
					{
						while(*strpos>='a' && *strpos<='z')
						{
							switch (*strpos)
							{
								case 'a':
									ForceLoad_Alien = TRUE;
									break;
								case 'm':
									ForceLoad_Marine = TRUE;
									break;
								case 'p':
									ForceLoad_Predator = TRUE;
									break;
								case 'h':
									ForceLoad_Hugger = TRUE;
									break;
								case 'q':
									ForceLoad_Queen = TRUE;
									break;
								case 'c':
									ForceLoad_Civvie = TRUE;
									break;
								case 'x':
									ForceLoad_Xenoborg = TRUE;
									break;
								case 't':
									ForceLoad_Pretorian = TRUE;
									break;
								case 'r':
									ForceLoad_PredAlien = TRUE;
									break;
								case 's':
									ForceLoad_SentryGun = TRUE;
									break;
							}
							strpos++;
						}
					}
					else
					{
						ForceLoad_Alien = TRUE;
					}
					strpos=strstr(strpos,"-l");
				}
			}
		}

		#ifdef AVP_DEBUG_VERSION
		if (strstr(command_line, "-intro"))	WeWantAnIntro();
		if (strstr(command_line, "-qm"))
		{
			QuickStartMultiplayer = 1;
		}
		else if (strstr(command_line, "-qa"))
		{
			QuickStartMultiplayer = 2;
		}
		else if (strstr(command_line, "-qp"))
		{
			QuickStartMultiplayer = 3;
		}
		else
		{
			QuickStartMultiplayer = 0;
		}

		if(strstr(command_line,"-keeprif"))
		{
			KeepMainRifFile=TRUE;			
		}

		if (strstr(command_line, "-m"))
		{
			UseMouseCentreing = 1;
		}
		#endif //AVP_DEBUG_VERSION
	}
	#else
	{
		#if OverrideOldMenus
		{
			HWAccel = 1;
		}
		#endif
	}
	#endif

	if(strstr(command_line,"-server"))
	{
		extern int DirectPlay_InitLobbiedGame();
		//game has been launched by mplayer , we best humour it
		LobbiedGame=LobbiedGame_Server;	
		if(!DirectPlay_InitLobbiedGame())
		{
			exit(0x6364);
		}		
	}
	else if(strstr(command_line,"-client"))
	{
		extern int DirectPlay_InitLobbiedGame();
		//ditto
		LobbiedGame=LobbiedGame_Client;			
		if(!DirectPlay_InitLobbiedGame())
		{
			exit(0x6364);
		}		
	}
	else if (strstr(command_line, "-debug"))
	{
		DebuggingCommandsActive = 1;
	}

	if(instr = strstr(command_line, "-ip"))
	{								  
		char buffer[100];
		extern char CommandLineIPAddressString[]; 

		sscanf(instr, "-ip %s", &buffer);
		strncpy(CommandLineIPAddressString,buffer,15);
		CommandLineIPAddressString[15] = 0;
	}
	
		
 	// Modified by Edmond for mplayer demo
 	#if MPLAYER_DEMO
 	if (!LobbiedGame)
 	{
 		MessageBox(NULL, "This demo can only be launched from Mplayer.", "Oh no!", MB_OK);
 		exit(33445);
 	}
 	#endif
 	
 	#if PLAY_INTRO//(MARINE_DEMO||ALIEN_DEMO||PREDATOR_DEMO)
  	if (!LobbiedGame)  // Edmond
	 	WeWantAnIntro();
	#endif
	GetPathFromRegistry();

	/* JH 28/5/97 */
	/* Initialise 'fast' file system */
	#if MARINE_DEMO
	ffInit("fastfile\\mffinfo.txt","fastfile\\");
	#elif ALIEN_DEMO
	ffInit("alienfastfile\\ffinfo.txt","alienfastfile\\");
	#else
	ffInit("fastfile\\ffinfo.txt","fastfile\\");
	#endif

	InitGame();


    /*** Define video mode for windows initialisation ***/
	InitialVideoMode();

	/****** Put in by John to sort out easy sub window mode ******/
	/****** REMOVE FOR GAME!!!!! ******/

	#if debug && 1//!PREDATOR_DEMO
	if(strstr(command_line, "-w"))
	{
		WindowRequestMode = WindowModeSubWindow;
		if (!HWAccel)
			RasterisationRequestMode = RequestSoftwareRasterisation;
	}
	if(instr = strstr(command_line, "-s"))
		sscanf(instr, "-s%d", &level_to_load);
	
	#endif

	Env_List[0] = &(ELOLevelToLoad);
	level_to_load = 0;

	/*******	System initialisation **********/
	InitialiseSystem(hInstance, nCmdShow);
	InitialiseRenderer();
	
	InitOptionsMenu(); /* by this time we know all about the video card, etc */

	LoadKeyConfiguration();
	/********** Grab The Video mode **********/
	/* JH - nope, not yet; not until we start the menus
		(or if debugging, start the game), do we need to 
		set the initial video mode */
	
  	
	/*-------------------Patrick 2/6/97-----------------------
	Start the sound system
	----------------------------------------------------------*/
	SoundSys_Start();
	CDDA_Start();
	
	/* kill mouse cursor */
	SetCursor(NULL);

	/* load language file and setup text string access */
	InitTextStrings();
	
	BuildMultiplayerLevelNameArray();//sort out multiplayer level names
	
	ChangeDirectDrawObject();
	AvP.LevelCompleted = 0;
	LoadSounds("PLAYER"); 

	#if PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO
	if(AvP_MainMenus())
	#else
	while(AvP_MainMenus())
	#endif
	{
		int menusActive=0;
		int thisLevelHasBeenCompleted=0;
		#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
		if(instr = strstr(command_line, "-n"))
		{								  
			sscanf(instr, "-n %s", &LevelName);
		}
		#endif

		#if debug
		if(UseMouseCentreing)
		{
			//Start thread that recentres mouse , making it easier to play
			//in subwindow mode
			InitCentreMouseThread();
		}
		#endif
//		Env_List[0] = &(ELOLevelToLoad);
//		level_to_load = 0;
//		AvP.PlayerType = I_Marine;


		/* turn off any special effects */
		d3d_light_ctrl.ctrl = LCCM_NORMAL;
		d3d_overlay_ctrl.ctrl = OCCM_NORMAL;
	
		/* JH 20/5/97
			The video mode is no longer set when exiting the menus
			(not necessary if user selects EXIT)
			So it is set here */

		/********** Grab The Video mode **********/
		GetCorrectDirectDrawObject();
		
		if(!SetGameVideoMode())
		{
			VideoModeNotAvailable=1;
			continue;
		}

	    /* Dubious restart hack for DirectDraw problems */
		/* JH - I'm not sure this is really necessary
			- it only comes into play if you try and set
			a video mode which is not supported
			BUT we are never going to try and do that
			- or are we? */
	    HandleVideoModeRestarts(hInstance, nCmdShow);


		/* Check Gamma Settings are correct after video mode change */
		InitialiseGammaSettings(RequestedGammaSetting);




		/**** init the chunk loaders ***************/

		// no longer required here
		
		// Load precompiled shapes 
	    start_of_loaded_shapes = load_precompiled_shapes();
		
		/***********  Load up the character stuff *******/

		InitCharacter();

		/* KJL 17:56:14 26/02/98 - load a font required for Dave's HUD */
//		LoadPFFont(DATABASE_MESSAGE_FONT);
	
		/***********  Read in the env Map	 **************/
		
		#if debug
  		if(level_to_load != I_Num_Environments)
  		{
  			if((level_to_load < 0) || (level_to_load > I_Num_Environments))
 				#ifdef MPLAYER_DEMO
 					level_to_load = I_Dml1;
 				#else
 					level_to_load = I_Sp1;
 				#endif
  
  			AvP.CurrentEnv = AvP.StartingEnv = level_to_load;
  		}
		#endif
		
		LoadRifFile(); /* sets up a map*/
		#if debug
		DebugFontLoaded = 1;		
		#endif 																					 	

		/*********** Process the data ************/
		AssignAllSBNames();
		StartGame();

//		UnloadRifFile();//deletes environment File_Chunk since it is no longer needed
		

		/* Patrick 26/6/97 
		Load the game sounds here: should be done after loading everthing
		else, incase sounds take up system memory */
//		LoadSounds("PLAYER"); 

		/* JH 28/5/97 */
		/* remove resident loaded 'fast' files */
		ffcloseall();
		/*********** Play the game ***************/		 

		/* KJL 15:43:25 03/11/97 - run until this boolean is set to 0 */
		AvP.MainLoopRunning = 1;
		
		ScanImagesForFMVs();

		ResetFrameCounter();
		Game_Has_Loaded();
		ResetFrameCounter();

		if(AvP.Network!=I_No_Network)
		{
			/*Need to choose a starting position for the player , but first we must look
			through the network messages to find out which generator spots are currently clear*/
			netGameData.myGameState = NGS_Playing;
			MinimalNetCollectMessages();
			TeleportNetPlayerToAStartingPosition(Player->ObStrategyBlock,1);
		}
		
		IngameKeyboardInput_ClearBuffer();
		while(AvP.MainLoopRunning) 
		{

			#if debug
			#if 0
			DumpBoundsCheckInfo(DUMPTOSCREEN);
			#endif
			#endif

		 	CheckForWindowsMessages();
			CursorHome();

			#if debug
			if (memoryInitialisationFailure)
			{
				textprint("Initialisation not completed - out of memory!\n");
				GLOBALASSERT(1 == 0);
			}
			#endif

			switch(AvP.GameMode)
			{
				case I_GM_Playing:
				{
					if ((!menusActive || (AvP.Network!=I_No_Network && !netGameData.skirmishMode)) && !AvP.LevelCompleted)
					{
						#if MainTextPrint 		/* debugging stuff */
						{

							if (ShowDebuggingText.FPS) ReleasePrintDebuggingText("FrameRate = %d fps\n",FrameRate);
							if (ShowDebuggingText.Environment) ReleasePrintDebuggingText("Environment %s\n", Env_List[AvP.CurrentEnv]->main);
							if (ShowDebuggingText.Coords) ReleasePrintDebuggingText("Player World Coords: %d,%d,%d\n",Player->ObWorld.vx,Player->ObWorld.vy,Player->ObWorld.vz);

							{
								PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
								PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
								TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
								if (ShowDebuggingText.GunPos)
								{
									PrintDebuggingText("Gun Position x:%d,y:%d,z:%d\n",twPtr->RestPosition.vx,twPtr->RestPosition.vy,twPtr->RestPosition.vz);
								}
							}
						}
						#endif  /* MainTextPrint */
						
						DoAllShapeAnimations();

						UpdateGame();

						#if 1
						#if PROFILING_ON 
					  	ProfileStart();
						#endif
						AvpShowViews();
						#if PROFILING_ON 
					  	ProfileStop("SHOW VIEW");
						#endif

						//Do screen shot here so that text and  hud graphics aren't shown
						#if PROFILING_ON 
					  	ProfileStart();
						#endif
						MaintainHUD();
						#if PROFILING_ON 
					  	ProfileStop("RENDER HUD");
						#endif

						#if debug
						FlushTextprintBuffer();
						#endif

						//check cd status
						CheckCDAndChooseTrackIfNeeded();
						
						// check to see if we're pausing the game;
						// if so kill off any sound effects
						if(InGameMenusAreRunning() && ( (AvP.Network!=I_No_Network && netGameData.skirmishMode) || (AvP.Network==I_No_Network))	)
							SoundSys_StopAll();
					}
					else
					{
						ReadUserInput();
//						UpdateAllFMVTextures();	
						SoundSys_Management();
						#if SOFTWARE_RENDERER
						FlushSoftwareZBuffer();
						#else
						FlushD3DZBuffer();
						#endifÿ
						{
							extern void ThisFramesRenderingHasBegun(void);
							ThisFramesRenderingHasBegun();
						}
					}

					{
						menusActive = AvP_InGameMenus();
						if(AvP.RestartLevel) menusActive=0;
					}
					if (AvP.LevelCompleted)
					{
						SoundSys_FadeOutFast();
						DoCompletedLevelStatisticsScreen();
						thisLevelHasBeenCompleted = 1;
					}

					{
						/* after this call, no more graphics can be drawn until the next frame */
						extern void ThisFramesRenderingHasFinished(void);
						ThisFramesRenderingHasFinished();
					}

					/* JH 5/6/97 - this function may draw translucent polygons to the whole screen */
					//HandleD3DScreenFading();
					#else
					{

					ColourFillBackBuffer(0);
					}
					#endif

					
					InGameFlipBuffers();

					FrameCounterHandler();
					{
						PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

						if (!menusActive && playerStatusPtr->IsAlive && !AvP.LevelCompleted)
						{
							DealWithElapsedTime();
						}
					}
					break;
				}
				case I_GM_Menus:
				{	
					AvP.GameMode = I_GM_Playing;
					//StartGameMenus();
					LOCALASSERT(AvP.Network == I_No_Network);
					//AccessDatabase(0);
					break;
				}
				#if 0
				case I_GM_Paused:
				{
					{
						extern void DoPcPause(void);
						DoPcPause();
					}
					break;
				}
				#endif 

				default:
				{
					GLOBALASSERT(2<1);
					break;
				}
			}
			
			if(AvP.RestartLevel)
			{
				AvP.RestartLevel=0;
				AvP.LevelCompleted = 0;
				FixCheatModesInUserProfile(UserProfilePtr);
				RestartLevel();
			}
			

		}// end of main game loop
		{
			AvP.LevelCompleted = thisLevelHasBeenCompleted;
		}

		FixCheatModesInUserProfile(UserProfilePtr);

		#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
		TimeStampedMessage("We're out of the main loop");
	
		/* KJL 17:56:14 26/02/98 - unload a font required for Dave's HUD */
//		UnloadFont(&AvpFonts[DATABASE_MESSAGE_FONT]);
		#if 1
		{
			extern void CloseFMV(void);
			CloseFMV();
			ReleaseAllFMVTextures();
		}
		#endif

		/* DHM 23/3/98 */
//		REBMENUS_ProjectSpecific_EndOfMainLoopHook();

		/* DHM 8/4/98 */
		CONSBIND_WriteKeyBindingsToConfigFile();
		TimeStampedMessage("After key bindings writen");

		/* CDF 2/10/97 */
		DeInitialisePlayer();
		TimeStampedMessage("After DeInitialisePlayer");

		DeallocatePlayersMirrorImage();
		TimeStampedMessage("After DeallocatePlayersMirrorImage");
				
		/* KJL 15:26:43 03/12/97 - clear data */
		KillHUD();
		TimeStampedMessage("After KillHUD");

		Destroy_CurrentEnvironment();
		TimeStampedMessage("After Destroy_CurrentEnvironment");
		DeallocateAllImages();
		TimeStampedMessage("After DeallocateAllImages");
		EndNPCs(); /* JH 30/4/97 - unload npc rifs */
		TimeStampedMessage("After EndNPCs");
		ExitGame();
		#endif
		/* Patrick 26/6/97
		Stop and remove all game sounds here, since we are returning to the menus */
		SoundSys_StopAll();
		ResetEaxEnvironment();
		//make sure the volume gets reset for the menus
		SoundSys_ResetFadeLevel();

		TimeStampedMessage("After SoundSys_StopAll");

//		SoundSys_RemoveAll(); 

		TimeStampedMessage("After SoundSys_RemoveAll");
		CDDA_Stop();
		TimeStampedMessage("After CDDA_Stop");

		/* netgame support */
		if(AvP.Network != I_No_Network) 
		{
			/* we cleanup and reset our game mode here, at the end of the game loop, as other 
			clean-up functions need to know if we've just exited a netgame */
			EndAVPNetGame();
			//EndOfNetworkGameScreen();
		}

		//need to get rid of the player rifs before we can clear the memory pool
				
		ClearMemoryPool();
	
		#if debug
		if(UseMouseCentreing)
		{
			//Stop thread that recentres mouse , making it easier to play
			//in subwindow mode
			FinishCentreMouseThread();
		}
		#endif
		
		if(LobbiedGame)
		{
			/*
			We have been playing a lobbied game , and have now diconnected.
			Since we can't start a new multiplayer game , exit to avoid confusion
			*/
			break;
		}
	}
	#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
	TimeStampedMessage("After Menus");

	/* Added 28/1/98 by DHM: hook for my code on program shutdown */
	{
		DAVEHOOK_UnInit();
	}
	TimeStampedMessage("After DAVEHOOK_UnInit");
	
	/*-------------------Patrick 2/6/97-----------------------
	End the sound system
	----------------------------------------------------------*/
	SoundSys_StopAll();
  	SoundSys_RemoveAll(); 

	
	


	#else
	QuickSplashScreens();
	#endif
	#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
	TimeStampedMessage("After SoundSys_End");
	TimeStampedMessage("After CDDA_End");
	/* unload language file */
//	KillTextStrings();
// 	TimeStampedMessage("After KillTextStrings");
	ExitSystem();
	TimeStampedMessage("After ExitSystem");

//	ffKill(); /* to avoid misreported memory leaks */
	TimeStampedMessage("After ffKill");
	#else
	SoundSys_End();
	ReleaseDirect3D();
	//TimeStampedMessage("after ReleaseDirect3D");
	
	/* Kill windows procedures */
	ExitWindowsSystem();
	//TimeStampedMessage("after ExitWindowsSystem");

	#endif
 	CDDA_End();
	ClearMemoryPool();

	return(0);
}

