#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "cheatmodes.h"
#include "net.h"
#include "opengl.h"
#include "pldnet.h"

#include "avp_menudata.h"
#include "avp_menus.h"
#include "avp_envinfo.h"

#include "hud_layout.h"
#include "avp_userprofile.h"
#include "huffman.hpp"

#include "hudgfx.h"
#include "usr_io.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "iofocus.h"
#include <time.h>
#include "gammacontrol.h"
#include "avp_mp_config.h"
#include "psnd.h"
#include "savegame.h"
#include "game.h"
#include "avp_menugfx.hpp"
#include "avp_intro.h"
#include "fmv.h"

/* used to get file time */
#include <sys/types.h>
#include <sys/stat.h>

int SelectDirectDrawObject(void *pGUID);
                    
extern void StartMenuBackgroundBink(void);
extern int PlayMenuBackgroundBink(void);
extern void EndMenuBackgroundBink(void);


/* KJL 11:22:37 23/06/98 - Hopefully these will be the final menus! */

extern int IDemandSelect(void);

extern char *GetVideoModeDescription(void);
extern char *GetVideoModeDescription2(void);
extern char *GetVideoModeDescription3(void);
extern void PreviousVideoMode(void);
extern void PreviousVideoMode2(void);
extern void NextVideoMode(void);
extern void NextVideoMode2(void);
extern void SaveVideoModeSettings(void);
extern void LoadDeviceAndVideoModePreferences(void);
extern void SaveDeviceAndVideoModePreferences(void);

extern void MakeSelectSessionMenu(void);

extern void MakeInGameMenu(void);
extern void MakeMarineKeyConfigMenu(void);
extern void MakePredatorKeyConfigMenu(void);
extern void MakeAlienKeyConfigMenu(void);
extern void MakeUserProfileSelectMenu(void);

extern void SaveKeyConfiguration(void);


extern void D3D_DrawSliderBar(int x, int y, int alpha);
extern void D3D_DrawSlider(int x, int y, int alpha);
extern void D3D_FadeDownScreen(int brightness, int colour);
extern void PlayIntroSequence(void);

extern void MinimalNetCollectMessages(void);

extern int DirectPlay_HostGame(char *playerName, char *sessionName,int species,int gamestyle,int level);
extern int DirectPlay_JoinGame(void);
extern int DirectPlay_ConnectToSession(int sessionNumber, char *playerName);
extern int DirectPlay_Disconnect(void);

extern void ShowSplashScreens(void);
extern void Show_WinnerScreen(void);

extern void GetNextAllowedSpecies(int* species,BOOL search_forwards);
static void SetBriefingTextForMultiplayer();
int NumberOfAvailableLevels(I_PLAYER_TYPE playerID);
int LevelMostLikelyToPlay(I_PLAYER_TYPE playerID);
int MaxDifficultyLevelAllowed(I_PLAYER_TYPE playerID, int level);

int CloudTable[128][128];

extern char MP_Config_Name[];

void HandlePostGameFMVs(void);
void HandlePreGameFMVs(void);
extern void AvP_UpdateMenus(void);
static void SetupNewMenu(enum AVPMENU_ID menuID);
static void RenderMenu(void);
static void RenderBriefingScreenInfo(void);
static void RenderEpisodeSelectMenu(void);
static void RenderKeyConfigurationMenu(void);
static void RenderUserProfileSelectMenu(void);
static void RenderLoadGameMenu(void);
static void RenderConfigurationDescriptionString();
static void ActUponUsersInput(void);
static void InteractWithMenuElement(enum AVPMENU_ELEMENT_INTERACTION_ID interactionID);
static void RenderMenuElement(AVPMENU_ELEMENT *elementPtr, int e, int y);
static int HeightOfMenuElement(AVPMENU_ELEMENT *elementPtr);
void DisplayVideoModeUnavailableScreen(void);
void CheckForCredits(void);
void DoCredits(void);
BOOL RollCreditsText(int position, unsigned char *textPtr);
extern void SelectMenuDisplayMode(void);
static void InitMainMenusBackdrop(void);
extern void DrawMainMenusBackdrop(void);
static void TestValidityOfCheatMenu(void);
void SetBriefingTextForEpisode(int episode, I_PLAYER_TYPE playerID);
void SetBriefingTextToBlank(void);
void CheckForKeysWithMultipleAssignments(void);
void HandleCheatModeFeatures(void);
void ShowMenuFrameRate(void);
static void KeyboardEntryQueue_Clear(void);
static void KeyboardEntryQueue_StartProcessing(void);

static void PasteFromClipboard(char* Text,int MaxTextLength);
/* KJL 11:23:03 23/06/98 - Requirements

	1.      'Floating' over backdrop, possibly using translucency effects
	2.      Quick to write!
	3.      Quick to change
	4.      Certain menus need to be used in-game - e.g. key configuration
*/

/* KJL 11:27:18 23/06/98 - Available menus

	1.      Main Menu
		
		Start Singleplayer
		Start Multiplayer 
		Gameplay Options
		Audio/Visual Options
		Exit
						  
	2.      Singleplayer
		
		Alien
		Marine
		Predator
		Exit

	3.      Multiplayer

		-> Complex

	4.      Gameplay Options

	5.      Audio/Visual Options
		
 */


/* KJL 11:57:21 23/06/98 - 


	Need mouse position code (?) etc. In-game menus using mouse? Ugh.
	No mouse then.

*/
SAVE_SLOT_HEADER SaveGameSlot[NUMBER_OF_SAVE_SLOTS];


extern int VideoModeNotAvailable;

extern int RealFrameTime;
extern unsigned char KeyboardInput[];
extern unsigned char DebouncedKeyboardInput[];
extern int DebouncedGotAnyKey;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern int TimeScale;

static AVP_MENUS AvPMenus;
extern AVPMENU  AvPMenusData[];

extern int AlienEpisodeToPlay;
extern int MarineEpisodeToPlay;
extern int PredatorEpisodeToPlay;
extern int UserProfileNumber;
AVP_USER_PROFILE *UserProfilePtr;

SESSION_DESC SessionData[MAX_NO_OF_SESSIONS];
int NumberOfSessionsFound;
extern NETGAME_GAMEDATA netGameData;

static int InputIsDebounced = 0;
static int KeyDepressedCounter = 0;
extern int CloakingPhase;
static int EpisodeSelectScrollOffset;
static int MaximumSelectableLevel;
static int KeyConfigSelectionColumn;

static int Brightness[16];

static unsigned char MultipleAssignments[2][32];

static char *GetDescriptionOfKey(unsigned char key);
static int OkayToPlayNextEpisode(void);
static PLAYER_INPUT_CONFIGURATION PlayerInputPrimaryConfig;
static PLAYER_INPUT_CONFIGURATION PlayerInputSecondaryConfig;
CONTROL_METHODS PlayerControlMethods;
JOYSTICK_CONTROL_METHODS PlayerJoystickControlMethods;

static void RenderScrollyMenu();
static void RenderHelpString();
static void CheckForLoadGame();

static void UpdateMultiplayerConfigurationMenu();

static char KeyboardEntryQueue_ProcessCharacter(void);

static BOOL LaunchingMplayer=FALSE;

static int MultiplayerConfigurationIndex; //just used for the configuration deletion stuff
static const char* MultiplayerConfigurationName=0; //ditto

extern int DebuggingCommandsActive;

int AvP_MainMenus(void)
{
	#if 0
	SaveDefaultPrimaryConfigs();
	#else
	LoadDefaultPrimaryConfigs();
	#endif
	
	SoundSys_ResetFadeLevel();
	SoundSys_Management();	
	
	TimeScale = ONE_FIXED;



 	if (!LobbiedGame)  // Edmond
		CheckForCredits();
	
	TimeStampedMessage("start of menus");
	// open a 640x480x16 screen     
	SelectMenuDisplayMode();
	TimeStampedMessage("after SelectMenuDisplayMode");

	
	


	InitialiseMenuGfx();
	TimeStampedMessage("after InitialiseMenuGfx");

	// inform backdrop code
	InitMainMenusBackdrop();
	TimeStampedMessage("after InitMainMenusBackdrop");


	#if PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO
	if (AvP.LevelCompleted)
	{
		Show_WinnerScreen();
	}
	#endif

	LoadAllAvPMenuGfx();
	TimeStampedMessage("after LoadAllAvPMenuGfx");



	ResetFrameCounter();

 	if (!LobbiedGame)	// Edmond
		PlayIntroSequence();
	
	if (VideoModeNotAvailable)
	{	
		LoadGameRequest = SAVELOAD_REQUEST_NONE;
		DisplayVideoModeUnavailableScreen();
	}
	VideoModeNotAvailable = 0;

	if(AvP.LevelCompleted && CheatMode_Active == CHEATMODE_NONACTIVE && !DebuggingCommandsActive)
	{
		HandlePostGameFMVs();
		OkayToPlayNextEpisode();
		AvP.LevelCompleted = 0;
		AvPMenus.MenusState = MENUSSTATE_MAINMENUS;
	}
	else if(!LobbiedGame)
	{
		if (UserProfileNumber==-1)
		{
			SetupNewMenu(AVPMENU_USERPROFILESELECT);
		}
		else
		{
			SetupNewMenu(AVPMENU_MAIN);
		}
		AvPMenus.MenusState = MENUSSTATE_MAINMENUS;
	}
	else
	{
 		SetupNewMenu(AVPMENU_USERPROFILESELECT);
	  //AvPMenus.MenusState = MENUSSTATE_STARTGAME;	
		
	}

	CheatMode_Active = CHEATMODE_NONACTIVE;
	

	TimeStampedMessage("starting general menus");

	while(AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		CheckForWindowsMessages();
		DrawMainMenusBackdrop();
		ReadUserInput();
		AvP_UpdateMenus();
   //	BezierCurve();
		
		ShowMenuFrameRate();

		FlipBuffers();
		FrameCounterHandler();
		PlayMenuMusic();
		#if 0
		{
			extern int EffectsSoundVolume;
			SoundSys_ChangeVolume(EffectsSoundVolume);
		}
		#endif
		SoundSys_Management();
		UpdateGammaSettings();

		CheckForLoadGame();
	}
	
	if (AvPMenus.MenusState==MENUSSTATE_OUTSIDEMENUS)
	{
		//Don't bother showing credits if we are just exiting in order to start a game
		//using mplayer. The credits will get shown later after the player has actually
		//played the game
		if(!LaunchingMplayer)
		{
 			if (!LobbiedGame)	// Edmond
				DoCredits();
		}
	}
	TimeStampedMessage("ready to exit menus");
	
	EndMenuMusic();
	EndMenuBackgroundBink();
	TimeStampedMessage("after EndMenuMusic");

	#if PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO
	if ((AvPMenus.MenusState != MENUSSTATE_STARTGAME)) ShowSplashScreens();
	TimeStampedMessage("after ShowSplashScreens");
	#endif
	ReleaseAllAvPMenuGfx();

	TimeStampedMessage("after ReleaseAllAvPMenuGfx");

	SoundSys_StopAll();
	SoundSys_Management();

	if (CheatMode_Active == CHEATMODE_NONACTIVE) HandlePreGameFMVs();

	HandleCheatModeFeatures();
	AvP.LevelCompleted = 0;

	return (AvPMenus.MenusState == MENUSSTATE_STARTGAME);
}
void HandlePostGameFMVs(void)
{
	switch(AvP.PlayerType)
	{
		case I_Marine:
		{
			if (MarineEpisodeToPlay==MAX_NO_OF_BASIC_MARINE_EPISODES-1)
			{
				ClearScreenToBlack();
				FlipBuffers();
				ClearScreenToBlack();
				PlayBinkedFMV("FMVs/marineoutro.bik");
			}
			break;
		}
		case I_Alien:
		{
			if (AlienEpisodeToPlay==MAX_NO_OF_BASIC_ALIEN_EPISODES-1)
			{
				ClearScreenToBlack();
				FlipBuffers();
				ClearScreenToBlack();
				PlayBinkedFMV("FMVs/alienoutro.bik");
			}
			break;
		}
		case I_Predator:
		{
			if (PredatorEpisodeToPlay==MAX_NO_OF_BASIC_PREDATOR_EPISODES-1)
			{
				ClearScreenToBlack();
				FlipBuffers();
				ClearScreenToBlack();
				PlayBinkedFMV("FMVs/predatoroutro.bik");
			}
			break;
		}
	}
}
void HandlePreGameFMVs(void)
{
	if (AvPMenus.MenusState == MENUSSTATE_STARTGAME && LoadGameRequest == SAVELOAD_REQUEST_NONE)
	{
		extern char LevelName[];
		if (!stricmp("derelict", LevelName))
		{
			ClearScreenToBlack();
			FlipBuffers();
			ClearScreenToBlack();
			PlayBinkedFMV("FMVs/marineintro.bik");
		}
		else if (!stricmp("temple", LevelName))
		{
			ClearScreenToBlack();
			FlipBuffers();
			ClearScreenToBlack();
			PlayBinkedFMV("FMVs/alienintro.bik");
		}
		else if (!stricmp("fall", LevelName))
		{
			ClearScreenToBlack();
			FlipBuffers();
			ClearScreenToBlack();
			PlayBinkedFMV("FMVs/predatorintro.bik");
		}
	}
}

extern void QuickSplashScreens(void)
{
	SelectMenuDisplayMode();
	if (AvP.LevelCompleted)
	{
		Show_WinnerScreen();
	}
	ShowSplashScreens();
}

extern void AvP_TriggerInGameMenus(void)
{
	AvPMenus.MenusState = MENUSSTATE_INGAMEMENUS;
	SetupNewMenu(AVPMENU_INGAME);

	/* KJL 16:32:55 21/07/98 - tell the console to go away */
	if (IOFOCUS_AcceptTyping()) IOFOCUS_Toggle();
//	SoundSys_PauseOn();
}

int AvP_InGameMenus(void)
{
	if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
	{
		D3D_FadeDownScreen(ONE_FIXED/8,0);
		AvP_UpdateMenus();
		UpdateGammaSettings();

		return 1;
	}
	else return 0;
}

int InGameMenusAreRunning(void)
{
	return (AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS);
}
extern void AvP_UpdateMenus(void)
{
//      DrawAvPMenuGfx(AVPMENUGFX_BIG_AVP_LOGO,50,50,16384);
//      DrawAvPMenuGfx(AVPMENUGFX_ALIENS_LOGO,10,75,16384,AVPMENUFORMAT_LEFTJUSTIFIED);
//      DrawAvPMenuGfx(AVPMENUGFX_PREDATOR_LOGO,380,77,16384,AVPMENUFORMAT_LEFTJUSTIFIED);
//      RenderMenuText("VS",330,95,16384,AVPMENUFORMAT_CENTREJUSTIFIED);

	#if 0
	if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYERJOINGAME2)
	{
		MinimalNetCollectMessages();
		/* KJL 19:49:04 05/07/98 - ugh, there goes the interface */
		{
			extern int MP_GameStyle;
			extern int MP_LevelNumber;
			extern int MP_Species;
			MP_GameStyle = netGameData.gameType;
			MP_LevelNumber = netGameData.levelNumber;
			switch (MP_Species)
			{
				default:
				case 0:
					netGameData.myCharacterType = NGCT_Marine;
					netGameData.myNextCharacterType = NGCT_Marine;
					AvP.PlayerType = I_Marine;
					break;
				case 1:
					netGameData.myCharacterType = NGCT_Predator;
					netGameData.myNextCharacterType = NGCT_Predator;
					AvP.PlayerType = I_Predator;
					break;
				case 2:
					netGameData.myCharacterType = NGCT_Alien;
					netGameData.myNextCharacterType = NGCT_Alien;
					AvP.PlayerType = I_Alien;
					break;
			}
		}
		AddNetMsg_PlayerDescription();

		NetSendMessages();
	}
	#endif
	
	if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_SPECIES_JOIN ||
	    AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_CONFIG_JOIN)
	{
		MinimalNetCollectMessages();
		if(AvP.Network==I_Host)
		{	/*
			We have become host while in the process of joining.
			This is bad.
			Best leave the game and return to the main menus.
			*/
			DirectPlay_Disconnect();
			SetupNewMenu(AVPMENU_MAIN);
			return;
		}

		if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_SPECIES_JOIN)
		{
			/* KJL 19:49:04 05/07/98 - ugh, there goes the interface */
			{
//				extern int MP_GameStyle;
//				extern int MP_LevelNumber;
				extern int MP_Species;
//				MP_GameStyle = netGameData.gameType;
//				MP_LevelNumber = netGameData.levelNumber;
				
				GetNextAllowedSpecies(&MP_Species,TRUE);
				netGameData.myCharacterSubType=NGSCT_General;
				switch (MP_Species)
				{
					default:
					case 0:
						netGameData.myCharacterType = NGCT_Marine;
						netGameData.myNextCharacterType = NGCT_Marine;
						AvP.PlayerType = I_Marine;
						break;
					case 1:
						netGameData.myCharacterType = NGCT_Predator;
						netGameData.myNextCharacterType = NGCT_Predator;
						AvP.PlayerType = I_Predator;
						break;
					case 2:
						netGameData.myCharacterType = NGCT_Alien;
						netGameData.myNextCharacterType = NGCT_Alien;
						AvP.PlayerType = I_Alien;
						break;

					case 3:	//various marine subtypes
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
						netGameData.myCharacterType = NGCT_Marine;
						netGameData.myNextCharacterType = NGCT_Marine;
						AvP.PlayerType = I_Marine;
						netGameData.myCharacterSubType =(NETGAME_SPECIALISTCHARACTERTYPE) (MP_Species-2);
						break;
				}
			}
			AddNetMsg_PlayerDescription();

			NetSendMessages();
		}
	}
	else if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_SPECIES_HOST)
	{
		extern int MP_Species;
		GetNextAllowedSpecies(&MP_Species,TRUE);
	}
	else if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_JOINING)
	{
		//check status of joining game
		int retval=0;
		if(LobbiedGame)
		{
			extern char MP_PlayerName[];
			retval=DirectPlay_ConnectingToLobbiedGame(MP_PlayerName);
			if(!retval)
			{
				//player has aborted , go back a menu
				SetupNewMenu(AVPMENU_MULTIPLAYER_LOBBIEDCLIENT);
				return ;
			}	
		}
		else
		{
			retval=DirectPlay_ConnectingToSession();
			if(!retval)
			{
				//player has aborted , go back a menu
				SetupNewMenu(AVPMENU_MULTIPLAYER);
				return ;
			}	
		}

		if(retval==AVPMENU_MULTIPLAYER_CONFIG_JOIN)
		{
			//successfully joined
			SetupNewMenu(AVPMENU_MULTIPLAYER_CONFIG_JOIN);
		}
	}
	else if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYERSELECTSESSION)
	{
		extern BOOL DirectPlay_UpdateSessionList(int * SelectedItem);
		int selection=AvPMenus.CurrentlySelectedElement;

		if(DirectPlay_UpdateSessionList(&selection))
		{
			//session list has changed , so we need to set the menu again
			SetupNewMenu(AVPMENU_MULTIPLAYERSELECTSESSION);
			//adjust the selected menu item
			AvPMenus.CurrentlySelectedElement=selection;
		}
	}



	/* render menu; episode select goes through a separate system */
	switch (AvPMenus.CurrentMenu)
	{
		case AVPMENU_LOADGAME:
		case AVPMENU_SAVEGAME:
		{
			RenderLoadGameMenu();
			RenderHelpString();
			break;
		}
		
		case AVPMENU_MARINELEVELS:
		case AVPMENU_PREDATORLEVELS:
		case AVPMENU_ALIENLEVELS:
		{
			RenderEpisodeSelectMenu();
			break;
		}
		case AVPMENU_USERPROFILESELECT:
		{
			RenderUserProfileSelectMenu();
			RenderHelpString();
			break;
		}
		case AVPMENU_USERPROFILEDELETE:
		{
			int i;
			AVP_USER_PROFILE *profilePtr = GetFirstUserProfile();
			time_t FileTime = profilePtr->FileTime;
			for (i=0; i<UserProfileNumber; i++)
				profilePtr = GetNextUserProfile();
			
			RenderMenuText(profilePtr->Name,MENU_CENTREX,MENU_CENTREY-100,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
			RenderSmallMenuText(ctime(&FileTime),MENU_CENTREX,MENU_CENTREY-70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
			
			RenderMenu();
			RenderHelpString();
			break;

		}
		case AVPMENU_MULTIPLAYER_DELETECONFIG :
		{
			//show the name of the configuration we're trying to delete
			RenderMenuText(MultiplayerConfigurationName,MENU_CENTREX,MENU_CENTREY-100,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
			
			RenderMenu();
			RenderHelpString();
			break;
		}

		case AVPMENU_ALIENKEYCONFIG:
		case AVPMENU_MARINEKEYCONFIG:
		case AVPMENU_PREDATORKEYCONFIG:
		{
			CheckForKeysWithMultipleAssignments();
			RenderKeyConfigurationMenu();

			if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
			{
				RenderHelpString();
			}
		
			break;
		}
		case AVPMENU_SKIRMISH_CONFIG :
		case AVPMENU_MULTIPLAYER_CONFIG :
		case AVPMENU_MULTIPLAYER_CONFIG_JOIN :
		{
			UpdateMultiplayerConfigurationMenu();
			RenderScrollyMenu();
			RenderHelpString();
 			break;
		}
		case AVPMENU_MULTIPLAYEROPENADDRESS :
		{
			RenderMenu();
			RenderHelpString();
			break;
		}

		case AVPMENU_MULTIPLAYER_LOADIPADDRESS :
		{
			RenderScrollyMenu();
			break;
		}

		case AVPMENU_MULTIPLAYER_LOADCONFIG :
		{
			RenderScrollyMenu();
			RenderConfigurationDescriptionString();
			RenderHelpString();
 			break;
 		}
		case AVPMENU_INGAMEAVOPTIONS:
		{
			int scale = DIV_FIXED(ScreenDescriptorBlock.SDB_Height,480);
			int h = scale/2048;
			int offset = scale/4096;
			D3D_DrawColourBar(offset, offset+h,  ONE_FIXED, 0, 0);
			D3D_DrawColourBar(offset*2+h, offset*2+h*2,  0, ONE_FIXED, 0);
			D3D_DrawColourBar(ScreenDescriptorBlock.SDB_Height-offset*2-h*2, ScreenDescriptorBlock.SDB_Height-offset*2-h,  0, 0, ONE_FIXED);
			D3D_DrawColourBar(ScreenDescriptorBlock.SDB_Height-offset-h, ScreenDescriptorBlock.SDB_Height-offset, ONE_FIXED, ONE_FIXED, ONE_FIXED);
			RenderMenu();
			break;
		}

		case AVPMENU_CHEATOPTIONS:
		{
			TestValidityOfCheatMenu();
			RenderMenu();
			RenderHelpString();
			break;
		}
		default:
		{
			RenderMenu();
			RenderHelpString();
			break;
		}
	}
	ActUponUsersInput();

}

static void SetupNewMenu(enum AVPMENU_ID menuID)
{
	enum AVPMENU_ID previousMenuID = AvPMenus.CurrentMenu;
	AvPMenus.CurrentMenu = menuID;

	/* set pointer to the start of the menu's element data */
	AvPMenus.MenuElements = AvPMenusData[menuID].MenuElements; // could use default

	AvPMenus.FontToUse = AvPMenusData[menuID].FontToUse;
	AvPMenus.CurrentlySelectedElement = 0;
	AvPMenus.UserEnteringText = 0;
	AvPMenus.UserEnteringNumber = 0;
	AvPMenus.UserChangingKeyConfig = 0;
	AvPMenus.PositionInTextField = 0;
	AvPMenus.WidthLeftForText = 0;


	/* menu specific stuff */
	switch (menuID)
	{
		case AVPMENU_MAIN:
		{
			GetSettingsFromUserProfile();
			SaveUserProfile(UserProfilePtr);
			
			if (DebuggingCommandsActive)
			{
				if (AnyCheatModesAllowed())
				{
					// turn on cheats
					SetupNewMenu(AVPMENU_DEBUG_MAIN_WITHCHEATS);
				}							   
				else
				{
					SetupNewMenu(AVPMENU_DEBUG_MAIN);
				}
			}
			else
			{
				if (AnyCheatModesAllowed())
				{
					// turn on cheats
					SetupNewMenu(AVPMENU_MAIN_WITHCHEATS);
				}
			}
			break;
		}
		
		case AVPMENU_VIDEOMODE:
		{	
			LoadDeviceAndVideoModePreferences();
			break;
		}

		case AVPMENU_USERPROFILESELECT:
		{
			InitialiseGammaSettings(128);
			EpisodeSelectScrollOffset=0;
			ExamineSavedUserProfiles();
			MakeUserProfileSelectMenu();
			break;
		}
		case AVPMENU_USERPROFILEENTERNAME:
		{
			AvPMenus.UserEnteringText = 1;
			KeyboardEntryQueue_Clear();
			AvPMenus.MenuElements->c.TextPtr = UserProfilePtr->Name;
			UserProfilePtr->Name[0] = 0;
			AvPMenus.WidthLeftForText = 0; //will be calculated properly when menus are drawn
			break;
		}
		case AVPMENU_MULTIPLAYER_SAVECONFIG:
		{
			AvPMenus.UserEnteringText = 1;
			KeyboardEntryQueue_Clear();
			AvPMenus.WidthLeftForText = 0; //will be calculated properly when menus are drawn
			break;
		}

		case AVPMENU_SINGLEPLAYER:
		{
			break;
		}
	
		case AVPMENU_MARINELEVELS:
		{
			MarineEpisodeToPlay=0;
			EpisodeSelectScrollOffset=0;
			AvPMenus.MenuElements->b.MaxSliderValue = NumberOfAvailableLevels(I_Marine);
			*AvPMenus.MenuElements->c.SliderValuePtr = LevelMostLikelyToPlay(I_Marine);
			break;
		}
		case AVPMENU_PREDATORLEVELS:
		{
			PredatorEpisodeToPlay=0;
			EpisodeSelectScrollOffset=0;
			AvPMenus.MenuElements->b.MaxSliderValue = NumberOfAvailableLevels(I_Predator);
			*AvPMenus.MenuElements->c.SliderValuePtr = LevelMostLikelyToPlay(I_Predator);
			break;
		}
		case AVPMENU_ALIENLEVELS:
		{
			AlienEpisodeToPlay=0;
			EpisodeSelectScrollOffset=0;
			AvPMenus.MenuElements->b.MaxSliderValue = NumberOfAvailableLevels(I_Alien);
			*AvPMenus.MenuElements->c.SliderValuePtr = LevelMostLikelyToPlay(I_Alien);
			break;
		}
		case AVPMENU_MULTIPLAYERSELECTSESSION:
		{
			if(previousMenuID!=AVPMENU_MULTIPLAYERSELECTSESSION)
			{
				//save ip address (if it has been set)
				extern char IPAddressString[]; 
				SaveIPAddress(IP_Address_Name,IPAddressString);
				DirectPlay_JoinGame();
			}
			MakeSelectSessionMenu();
			break;
		}
		
		case AVPMENU_MULTIPLAYER_SKIRMISH :
		{
			extern char MP_Config_Description[];
			netGameData.skirmishMode=TRUE;
			LoadMultiplayerConfiguration(GetTextString(TEXTSTRING_PREVIOUSGAME_FILENAME));
			MP_Config_Description[0]=0;
			break;
		}
		
		case AVPMENU_MULTIPLAYER_CONNECTION:
		{
			extern char MP_Config_Description[];
			//skirmishMode must be false
			netGameData.skirmishMode=FALSE;
			
			LoadMultiplayerConfiguration(GetTextString(TEXTSTRING_PREVIOUSGAME_FILENAME));
			MP_Config_Description[0]=0;
			

			if(LobbiedGame)
			{
				//use alternative multiplayer menus for lobbied games
				if(LobbiedGame==LobbiedGame_Server)
				{
					SetupNewMenu(AVPMENU_MULTIPLAYER_LOBBIEDSERVER);
				}
				else
				{
					SetupNewMenu(AVPMENU_MULTIPLAYER_LOBBIEDCLIENT);
				}
				return;
			}
			
			DirectPlay_EnumConnections();
			MakeConnectionSelectMenu();
			break;
		}

		case AVPMENU_INGAME:
		case AVPMENU_INNETGAME:
		{
			if(AvP.Network != I_No_Network)
			{
				//in a multiplayer game set up a menu without the restart mission option
				menuID=AVPMENU_INNETGAME;
				AvPMenus.CurrentMenu = menuID;
				AvPMenus.MenuElements = AvPMenusData[menuID].MenuElements; // could use default
				AvPMenus.FontToUse = AvPMenusData[menuID].FontToUse;
			}
			MakeInGameMenu();
			break;
		}
		case AVPMENU_MARINEKEYCONFIG:
		{
			MakeMarineKeyConfigMenu();
			PlayerInputPrimaryConfig = MarineInputPrimaryConfig;
			PlayerInputSecondaryConfig = MarineInputSecondaryConfig;
			EpisodeSelectScrollOffset=0;
			KeyConfigSelectionColumn = 0;
			break;
		}
		case AVPMENU_PREDATORKEYCONFIG:
		{
			MakePredatorKeyConfigMenu();
			PlayerInputPrimaryConfig = PredatorInputPrimaryConfig;
			PlayerInputSecondaryConfig = PredatorInputSecondaryConfig;
			EpisodeSelectScrollOffset=0;
			KeyConfigSelectionColumn = 0;
			break;
		}
		case AVPMENU_ALIENKEYCONFIG:
		{
			MakeAlienKeyConfigMenu();
			PlayerInputPrimaryConfig = AlienInputPrimaryConfig;
			PlayerInputSecondaryConfig = AlienInputSecondaryConfig;
			EpisodeSelectScrollOffset=0;
			KeyConfigSelectionColumn = 0;
			break;
		}
		case AVPMENU_CONTROLS:
		{
			PlayerControlMethods = ControlMethods;
			break;
		}
		case AVPMENU_JOYSTICKCONTROLS:
		{
			PlayerJoystickControlMethods = JoystickControlMethods;
			break;
		}

		case AVPMENU_MULTIPLAYERJOINGAME:
		{
			extern char IPAddressString[]; 
			extern char CommandLineIPAddressString[]; 
			strcpy(IPAddressString,CommandLineIPAddressString);
			IP_Address_Name[0] = 0;

			if(netGameData.connectionType!=CONN_TCPIP)  
			{
				////for non tcpip games skip to the select session menu
				SetupNewMenu(AVPMENU_MULTIPLAYERSELECTSESSION);
				return;
	
			}
			

			break;
		}

		case AVPMENU_MULTIPLAYER :
		{
			break;
		}

		case AVPMENU_MULTIPLAYER_LOADCONFIG :
		{
			extern AVPMENU_ELEMENT* AvPMenu_Multiplayer_LoadConfig;
			if(!BuildLoadMPConfigMenu())
			{
				SetupNewMenu(AVPMENU_MULTIPLAYER_CONFIG);
				return;
			}
			AvPMenus.MenuElements=AvPMenu_Multiplayer_LoadConfig;
			break;
 		}

		case AVPMENU_MULTIPLAYER_CONFIG :
		{
			//need to set the menu we return to , according to whether this is
			//a lobbied game or not.
			if(LobbiedGame)
			{
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu=AVPMENU_MULTIPLAYER_LOBBIEDSERVER;
			}
			else if(netGameData.skirmishMode)
			{
				//for skirmish games , use skirmish config menu instead
				SetupNewMenu(AVPMENU_SKIRMISH_CONFIG);
				return;
			}
			else
			{
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu=AVPMENU_MULTIPLAYER;
			}
			break;
		}

		case AVPMENU_MULTIPLAYER_CONFIG_JOIN :
		{
			//need to set the menu we return to , according to whether this is
			//a lobbied game or not.
			if(LobbiedGame)
			{
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu=AVPMENU_MULTIPLAYER_LOBBIEDCLIENT;
			}
			else
			{
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu=AVPMENU_MULTIPLAYER;
			}
			break;
		}

		case AVPMENU_MULTIPLAYER_LOADIPADDRESS :
		{
			extern AVPMENU_ELEMENT* AvPMenu_Multiplayer_LoadIPAddress;
			AvPMenus.MenuElements=AvPMenu_Multiplayer_LoadIPAddress;
			break;
		}

		case AVPMENU_MULTIPLAYEROPENADDRESS :
		{
			extern void MakeOpenIPAddressMenu();
			MakeOpenIPAddressMenu();
			break;
		}
		
		case AVPMENU_SAVEGAME:
		case AVPMENU_LOADGAME:
		{
			ScanSaveSlots();
			break;
		}

		default:
			break;
	}

	/* count the number of elements in the menu */
	{
		AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;
		AvPMenus.NumberOfElementsInMenu=0;
		AvPMenus.MenuHeight=0;
		do
		{
			elementPtr->Brightness = BRIGHTNESS_OF_DARKENED_ELEMENT;

			AvPMenus.MenuHeight += HeightOfMenuElement(elementPtr);

			AvPMenus.NumberOfElementsInMenu++;
			elementPtr++;
		}
		while(elementPtr->ElementID != AVPMENU_ELEMENT_ENDOFMENU);
	}       

	switch(menuID)
	{
		case AVPMENU_CHEATOPTIONS:
		{
			CheatMode_Active = 0;
			break;
		}
		
		case AVPMENU_MARINEKEYCONFIG:
		case AVPMENU_ALIENKEYCONFIG:
		case AVPMENU_PREDATORKEYCONFIG:
			AvPMenus.MenuHeight += 50;
			break;
		case AVPMENU_LEVELBRIEFING_BASIC:
		{
			int episodeToPlay;
			switch (AvP.PlayerType)
			{
				case I_Marine:
				{
					episodeToPlay = MarineEpisodeToPlay;
					break;
				}
				case I_Predator:
				{
					episodeToPlay = PredatorEpisodeToPlay;
					break;
				}
				case I_Alien:
				{
					episodeToPlay = AlienEpisodeToPlay;
					break;
				}
			}
			
			/* find available difficulty levels */
			AvPMenus.NumberOfElementsInMenu=MaxDifficultyLevelAllowed(AvP.PlayerType, episodeToPlay);
			/* highlight a suitable level of difficulty */
			if (episodeToPlay)
			{
				AvPMenus.CurrentlySelectedElement = AvPMenus.NumberOfElementsInMenu-1;
			}
			else // default to medium difficulty for first level
			{
				AvPMenus.CurrentlySelectedElement = 1;
			}
			
			SetBriefingTextForEpisode(episodeToPlay, AvP.PlayerType);

			break;
		}
		case AVPMENU_LEVELBRIEFING_BONUS:
		{
			int episodeToPlay;
			switch (AvP.PlayerType)
			{
				case I_Marine:
				{
					episodeToPlay = MarineEpisodeToPlay;
					break;
				}
				case I_Predator:
				{
					episodeToPlay = PredatorEpisodeToPlay;
					break;
				}
				case I_Alien:
				{
					episodeToPlay = AlienEpisodeToPlay;
					break;
				}
			}
			
			SetBriefingTextForEpisode(episodeToPlay, AvP.PlayerType);
			break;
		}
		default:
			SetBriefingTextToBlank();
			break;
	}
}


static void RenderMenu(void)
{
	AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;
	int e;
	int y;
	
	
	if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		if ((AvPMenus.CurrentMenu == AVPMENU_LEVELBRIEFING_BASIC)
		  ||(AvPMenus.CurrentMenu == AVPMENU_LEVELBRIEFING_BONUS))
		{
			y = MENU_BOTTOMYEDGE - AvPMenus.MenuHeight;
			RenderBriefingScreenInfo();
		}
		else
		{
			y = MENU_CENTREY - (AvPMenus.MenuHeight)/2;
		}
	}
	else // in game menus
	{
		y = (ScreenDescriptorBlock.SDB_Height - AvPMenus.MenuHeight)/2;
	}

	for (e = 0; e<AvPMenus.NumberOfElementsInMenu; e++, elementPtr++)
	{
		int targetBrightness;

		if (e==AvPMenus.CurrentlySelectedElement)
		{
			targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
		}
		else
		{ 
			targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
		}

		if (targetBrightness > elementPtr->Brightness)
		{
			elementPtr->Brightness+=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness>targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
		}
		else
		{
			elementPtr->Brightness-=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness<targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
			
		}
		
		RenderMenuElement(elementPtr,e,y);
		y += HeightOfMenuElement(elementPtr);
	}

	/* Render Menu Title */
	if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
		
#if 1 // and now we've been told to remove the "Gamers Edition" etc. :)
		// main menu subtitle e.g. "Gamers Edition" etc.
		if (AvPMenusData[AvPMenus.CurrentMenu].MenuTitle==TEXTSTRING_MAINMENU_TITLE)
			RenderMenuText(GetTextString(TEXTSTRING_MAINMENU_SUBTITLE),MENU_CENTREX,100,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
#endif
	}

	#if 0
	else
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		Hardware_RenderSmallMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}
	#endif

}
static void RenderBriefingScreenInfo(void)
{
	enum TEXTSTRING_ID textID;
	switch (AvP.PlayerType)
	{
		case I_Marine:
		{
			textID = MarineEpisodeToPlay+TEXTSTRING_MARINELEVELS_1;
			break;
		}
		case I_Predator:
		{
			textID = PredatorEpisodeToPlay+TEXTSTRING_PREDATORLEVELS_1;
			break;
		}
		case I_Alien:
		{
			textID = AlienEpisodeToPlay+TEXTSTRING_ALIENLEVELS_1;
			break;
		}
	}
	RenderMenuText(GetTextString(textID),MENU_LEFTXEDGE,120,ONE_FIXED,AVPMENUFORMAT_LEFTJUSTIFIED);
#if 0
	switch (AvP.PlayerType)
	{
		case I_Marine:
		{
			textID = MarineEpisodeToPlay+TEXTSTRING_LEVELBRIEFING_MARINELEVELS_1;
			break;
		}
		case I_Predator:
		{
			textID = PredatorEpisodeToPlay+TEXTSTRING_LEVELBRIEFING_PREDATORLEVELS_1;
			break;
		}
		case I_Alien:
		{
			textID = AlienEpisodeToPlay+TEXTSTRING_LEVELBRIEFING_ALIENLEVELS_1;
			break;
		}
	}
	RenderMenuText(GetTextString(textID),MENU_LEFTXEDGE,180,ONE_FIXED/2,AVPMENUFORMAT_LEFTJUSTIFIED);
#endif	
	RenderBriefingText(ScreenDescriptorBlock.SDB_Height/2,ONE_FIXED);
}
/* KJL 12:11:18 24/09/98 - specialised code to handle episode selection screen, which
has features which make it too awkward to add to the general system */
static void RenderEpisodeSelectMenu(void)
{
	AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];
	int currentEpisode = *(elementPtr->c.SliderValuePtr);
	int centrePosition = (currentEpisode)*65536+EpisodeSelectScrollOffset;
	enum AVPMENUGFX_ID graphicID;
	I_PLAYER_TYPE playerID;
	int i;
	int numberOfBasicLevels;
	
	switch (AvPMenus.CurrentMenu)
	{
		default:
		{
			LOCALASSERT(0);/* Panic */
		}
		case AVPMENU_MARINELEVELS:
		{
			graphicID = AVPMENUGFX_MARINE_EPISODE1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_MARINE_EPISODES;
			playerID = I_Marine;
			break;
		}
		case AVPMENU_PREDATORLEVELS:
		{
			graphicID = AVPMENUGFX_PREDATOR_EPISODE1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_PREDATOR_EPISODES;
			playerID = I_Predator;
			break;
		}
		case AVPMENU_ALIENLEVELS:
		{
			graphicID = AVPMENUGFX_ALIEN_EPISODE1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_ALIEN_EPISODES;
			playerID = I_Alien;
			break;
		}
	}
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}
	for (i=0; i<=elementPtr->b.MaxSliderValue; i++)
	{
		int y;

		y = MUL_FIXED(i*65536-centrePosition,100);

		if (y>=-150 && y<=150)
		{
			char *textPtr = GetTextString(elementPtr->a.TextDescription+i);
			int b;
			int targetBrightness;
			
			if (i==currentEpisode)
			{
				targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
			}
			else
			{ 
				targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
			}

			if (targetBrightness > Brightness[i])
			{
				Brightness[i]+=BRIGHTNESS_CHANGE_SPEED;
				if(Brightness[i]>targetBrightness)
				{
					Brightness[i] = targetBrightness;
				}
			}
			else
			{
				Brightness[i]-=BRIGHTNESS_CHANGE_SPEED;
				if(Brightness[i]<targetBrightness)
				{
					Brightness[i] = targetBrightness;
				}
			}

			b=Brightness[i];
			{
				int yCoord = MENU_CENTREY+y-60;

				RenderMenuText_Clipped(textPtr,MENU_LEFTXEDGE+150, yCoord, b,AVPMENUFORMAT_LEFTJUSTIFIED,MENU_CENTREY-60-100,MENU_CENTREY-60+180);
				DrawAvPMenuGfx_Clipped(graphicID+i, MENU_LEFTXEDGE, yCoord, b,AVPMENUFORMAT_LEFTJUSTIFIED, MENU_CENTREY-60-100, MENU_CENTREY-60+180);

				if (MaximumSelectableLevel>=i)
				{
					char *completedTextPtr;

					if (i<numberOfBasicLevels)
					{
						completedTextPtr = GetTextString(TEXTSTRING_NOTYETCOMPLETED+UserProfilePtr->LevelCompleted[playerID][i]);
					}
					else
					{
						if (UserProfilePtr->LevelCompleted[playerID][i])
						{
							completedTextPtr = GetTextString(TEXTSTRING_COMPLETED);
						}	
						else
						{
							completedTextPtr = GetTextString(TEXTSTRING_NOTYETCOMPLETED);
						}
					}
					
					RenderSmallMenuText
					(
						completedTextPtr,
						MENU_LEFTXEDGE+150,
						yCoord+30,
						b,
						AVPMENUFORMAT_LEFTJUSTIFIED
					);
				}
				else
				{
					if (i == elementPtr->b.MaxSliderValue)
					{
						RenderSmallMenuText
						(
							GetTextString(TEXTSTRING_NOTYETAVAILABLE_2),
							MENU_LEFTXEDGE+150,
							yCoord+30,
							b,
							AVPMENUFORMAT_LEFTJUSTIFIED
						);
					}
					else
					{
						RenderSmallMenuText
						(
							GetTextString(TEXTSTRING_NOTYETAVAILABLE_1),
							MENU_LEFTXEDGE+150,
							yCoord+30,
							b,
							AVPMENUFORMAT_LEFTJUSTIFIED
						);
					}
				}
				/*
					,MENU_CENTREY-60-100,
					MENU_CENTREY-60+180
				);*/
			}
		}
	}

	if (EpisodeSelectScrollOffset>0)
	{
		EpisodeSelectScrollOffset -= MUL_FIXED(EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset<0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}
	else if (EpisodeSelectScrollOffset<0)
	{
		EpisodeSelectScrollOffset += MUL_FIXED(-EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset>0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}

	
}
static void RenderKeyConfigurationMenu(void)
{
	AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;//AvPMenus.CurrentlySelectedElement];
	int centrePosition;
	int i;
	int centreY = ScreenDescriptorBlock.SDB_Height/2+25;
	int y;
	
	if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		int b;
		if (AvPMenus.CurrentlySelectedElement>=2)
		{
			b = ONE_FIXED;
		}
		else
		{
			b = ONE_FIXED/4;
		}
		
		RenderKeyConfigRectangle(b);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}
	else
	{
		int b;
		if (AvPMenus.CurrentlySelectedElement>=2)
		{
			b = ONE_FIXED;
		}
		else
		{
			b = ONE_FIXED/4;
		}
		
		Hardware_RenderKeyConfigRectangle(b);
	}		       
	y = centreY-160;
	for (i = 0; i<2; i++, elementPtr++)
	{
		int targetBrightness;

		if (i==AvPMenus.CurrentlySelectedElement)
		{
			targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
		}
		else
		{ 
			targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
		}

		if (targetBrightness > elementPtr->Brightness)
		{
			elementPtr->Brightness+=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness>targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
		}
		else
		{
			elementPtr->Brightness-=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness<targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
			
		}
		
		RenderMenuElement(elementPtr,i,y);
		y += HeightOfMenuElement(elementPtr);
	}
	centrePosition = (AvPMenus.CurrentlySelectedElement)*ONE_FIXED;
	if (centrePosition<2*ONE_FIXED) centrePosition = 2*ONE_FIXED;
	for (i=2; i<AvPMenus.NumberOfElementsInMenu; i++,elementPtr++)
	{

		y = MUL_FIXED(i*65536-centrePosition,20);

		if (y>=-100 && y<=100)
		{
//			char *textPtr = GetTextString(elementPtr->TextDescription);
			int targetBrightness;

			if (i==AvPMenus.CurrentlySelectedElement)
			{
				targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
			}
			else
			{ 
				targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
			}

			#if 0
			if (targetBrightness > elementPtr->Brightness)
			{
				elementPtr->Brightness+=BRIGHTNESS_CHANGE_SPEED;
				if(elementPtr->Brightness>targetBrightness)
				{
					elementPtr->Brightness = targetBrightness;
				}
			}
			else
			{
				elementPtr->Brightness-=BRIGHTNESS_CHANGE_SPEED;
				if(elementPtr->Brightness<targetBrightness)
				{
					elementPtr->Brightness = targetBrightness;
				}
				
			}
			#else
			elementPtr->Brightness = targetBrightness;
			#endif
			RenderMenuElement(elementPtr, i, centreY+y);
			#if 0
			if (AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
			{
				Hardware_RenderSmallMenuText(textPtr,MENU_LEFTXEDGE+150,centreY+y,b,AVPMENUFORMAT_LEFTJUSTIFIED/*,MENU_CENTREY-60-100,MENU_CENTREY-60+180*/);
			}
			else
			{
				RenderSmallMenuText(textPtr,MENU_LEFTXEDGE+150,centreY+y,b,AVPMENUFORMAT_LEFTJUSTIFIED/*,MENU_CENTREY-60-100,MENU_CENTREY-60+180*/);
			}
			#endif

		}
	}

	if (EpisodeSelectScrollOffset>0)
	{
		EpisodeSelectScrollOffset -= MUL_FIXED(EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset<0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}
	else if (EpisodeSelectScrollOffset<0)
	{
		EpisodeSelectScrollOffset += MUL_FIXED(-EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset>0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}
}

static void RenderScrollyMenu()
{
	AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;
	{
		//draw the title
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}

	{
		int first=AvPMenus.CurrentlySelectedElement;
		int last=AvPMenus.CurrentlySelectedElement;
		int i;
		int y;
		BOOL done=FALSE;

		int available_above=(MENU_HEIGHT-HeightOfMenuElement(&elementPtr[AvPMenus.CurrentlySelectedElement]))/2;
		int available_below=available_above;

		//work out the first and last element to be drawn
		do
		{
			done=TRUE;
			if(first-1>=0)
			{
				int h=HeightOfMenuElement(&elementPtr[first-1]);
				if(h<=available_above)
				{
					available_above-=h;
					first--;
					done=FALSE;
				}
				else
				{
					available_below+=available_above;
					available_above=0;
				}
			}
			if(first==0)
			{
				//no more elements above selected element
				available_below+=available_above;
				available_above=0;
			}

			if(last+1<AvPMenus.NumberOfElementsInMenu)
			{
				int h=HeightOfMenuElement(&elementPtr[last+1]);
				if(h<=available_below)
				{
					available_below-=h;
					last++;
					done=FALSE;
				}
			}
			if(last==(AvPMenus.NumberOfElementsInMenu-1))
			{
				//no more elements below selected element
				available_above+=available_below;
				available_below=0;
			}
		}
		while(!done);
		
		
		//draw the appropriate elements
		elementPtr=&elementPtr[first];
		y=MENU_TOPY;
		for(i=first;i<=last;i++,elementPtr++)
		{
//			char *textPtr = GetTextString(elementPtr->TextDescription);
			int targetBrightness;

			if (i==AvPMenus.CurrentlySelectedElement)
			{
				targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
			}
			else
			{ 
				targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
			}

			elementPtr->Brightness = targetBrightness;
			
			RenderMenuElement(elementPtr, i, y);
			y+=HeightOfMenuElement(elementPtr);
		}
		
	}
	
}


static void RenderUserProfileSelectMenu(void)
{
	AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];
	int currentEpisode = *(elementPtr->c.SliderValuePtr);
	int centrePosition = (currentEpisode)*65536+EpisodeSelectScrollOffset;
	int i;
	AVP_USER_PROFILE *profilePtr = GetFirstUserProfile();
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}

	for (i=0; i<=elementPtr->b.MaxSliderValue; i++, profilePtr = GetNextUserProfile())
	{
		int y;

		y = MUL_FIXED(i*65536-centrePosition,80);

		if (y>=-150 && y<=150)
		{
			char *textPtr = profilePtr->Name;
			time_t FileTime = profilePtr->FileTime;
			int b;
			int targetBrightness;

			if (i==currentEpisode)
			{
				targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
			}
			else
			{ 
				targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
			}

			if (targetBrightness > Brightness[i])
			{
				Brightness[i]+=BRIGHTNESS_CHANGE_SPEED;
				if(Brightness[i]>targetBrightness)
				{
					Brightness[i] = targetBrightness;
				}
			}
			else
			{
				Brightness[i]-=BRIGHTNESS_CHANGE_SPEED;
				if(Brightness[i]<targetBrightness)
				{
					Brightness[i] = targetBrightness;
				}
			}
			b=Brightness[i];
			RenderMenuText_Clipped(textPtr,MENU_CENTREX,MENU_CENTREY+y-60,b,AVPMENUFORMAT_CENTREJUSTIFIED,MENU_CENTREY-60-100,MENU_CENTREY-30+150);
			if (i > 0)
				RenderSmallMenuText(ctime(&FileTime),MENU_CENTREX,MENU_CENTREY+y-30,b,AVPMENUFORMAT_CENTREJUSTIFIED);
		}
	}

	if (EpisodeSelectScrollOffset>0)
	{
		EpisodeSelectScrollOffset -= MUL_FIXED(EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset<0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}
	else if (EpisodeSelectScrollOffset<0)
	{
		EpisodeSelectScrollOffset += MUL_FIXED(-EpisodeSelectScrollOffset*2+8192,RealFrameTime<<1);
		if (EpisodeSelectScrollOffset>0)
		{
			EpisodeSelectScrollOffset=0;
		}
	}

	
}

static void RenderLoadGameMenu(void)
{
	AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;
	int e;
	int y;
	int (*RenderText)(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format);
	
	if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		y = MENU_CENTREY - (AvPMenus.MenuHeight)/2;
		RenderText = RenderSmallMenuText;
	}
	else // in game menus
	{
		y = (ScreenDescriptorBlock.SDB_Height - AvPMenus.MenuHeight)/2;
		RenderText = Hardware_RenderSmallMenuText;
	}

	for (e = 0; e<AvPMenus.NumberOfElementsInMenu; e++, elementPtr++)
	{
		char buffer[100];
		SAVE_SLOT_HEADER *slotPtr = &SaveGameSlot[e];
		int targetBrightness;

		if (e==AvPMenus.CurrentlySelectedElement)
		{
			if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
			{
				Hardware_RenderHighlightRectangle(MENU_LEFTXEDGE,y-2,MENU_RIGHTXEDGE,y+4+HUD_FONT_HEIGHT*2,0,128,0);
			}
			else
			{
				RenderHighlightRectangle(MENU_LEFTXEDGE,y-2,MENU_RIGHTXEDGE,y+4+HUD_FONT_HEIGHT*2,0,128,0);
			}
			targetBrightness = BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT;
		}
		else
		{ 
			targetBrightness = BRIGHTNESS_OF_DARKENED_ELEMENT;
		}

		if (targetBrightness > elementPtr->Brightness)
		{
			elementPtr->Brightness+=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness>targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
		}
		else
		{
			elementPtr->Brightness-=BRIGHTNESS_CHANGE_SPEED;
			if(elementPtr->Brightness<targetBrightness)
			{
				elementPtr->Brightness = targetBrightness;
			}
			
		}
		
		sprintf(buffer,"%d.",e+1);
		RenderText(buffer,MENU_LEFTXEDGE+20,y,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
		
		if (slotPtr->SlotUsed)
		{
			int textID;
			int numberOfBasicEpisodes;
			switch(slotPtr->Species)
			{
				case I_Marine:
				{
					textID = TEXTSTRING_MARINELEVELS_1;
					numberOfBasicEpisodes = MAX_NO_OF_BASIC_MARINE_EPISODES;
					break;
				}
				case I_Alien:
				{
					textID = TEXTSTRING_ALIENLEVELS_1;
					numberOfBasicEpisodes = MAX_NO_OF_BASIC_ALIEN_EPISODES;
					break;
				}
				case I_Predator:
				{
					textID = TEXTSTRING_PREDATORLEVELS_1;
					numberOfBasicEpisodes = MAX_NO_OF_BASIC_PREDATOR_EPISODES;
					break;
				}
			}
		

			sprintf(buffer,"%s",GetTextString(TEXTSTRING_MULTIPLAYER_MARINE+slotPtr->Species));
			RenderText(buffer,MENU_LEFTXEDGE+30,y,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
			
			sprintf(buffer,"%s",GetTextString(textID+slotPtr->Episode));
			RenderText(buffer,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);

			if (numberOfBasicEpisodes>slotPtr->Episode)
			{
				sprintf(buffer,"%s",GetTextString(TEXTSTRING_DIFFICULTY_EASY+slotPtr->Difficulty));
				RenderText(buffer,MENU_RIGHTXEDGE-30,y,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
			}

			sprintf(buffer, "%s %02d:%02d:%02d",GetTextString(TEXTSTRING_GAMESTATS_TIMEELAPSED),slotPtr->ElapsedTime_Hours,slotPtr->ElapsedTime_Minutes,slotPtr->ElapsedTime_Seconds);
			RenderText(buffer,MENU_LEFTXEDGE+30,y+HUD_FONT_HEIGHT+1,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);

			sprintf(buffer, "%s: %d",GetTextString(TEXTSTRING_SAVEGAME_SAVESLEFT),slotPtr->SavesLeft);
			RenderText(buffer,MENU_CENTREX,y+HUD_FONT_HEIGHT+1,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			RenderText(ctime(&slotPtr->TimeStamp),MENU_RIGHTXEDGE-30,y+HUD_FONT_HEIGHT+1,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
		}
		else
		{
			RenderText(GetTextString(TEXTSTRING_SAVEGAME_EMPTYSLOT),MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
		}

		y += HeightOfMenuElement(elementPtr);
	}


	/* Render Menu Title */
	if (AvPMenus.MenusState == MENUSSTATE_MAINMENUS)
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		RenderMenuText(textPtr,MENU_CENTREX,70,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}
	else
	{
		char *textPtr = GetTextString(AvPMenusData[AvPMenus.CurrentMenu].MenuTitle);
		AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];
		y = (ScreenDescriptorBlock.SDB_Height - AvPMenus.MenuHeight)/2 - 30;
		RenderText(textPtr,MENU_CENTREX,y,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
		y = (ScreenDescriptorBlock.SDB_Height + AvPMenus.MenuHeight)/2 + 20;
		RenderText(GetTextString(elementPtr->HelpString),MENU_CENTREX,y,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
	}
}

static void RenderHelpString()
{
	AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];

	if(elementPtr->HelpString!=TEXTSTRING_BLANK && AvPMenus.MenusState != MENUSSTATE_INGAMEMENUS)
	{
		RECT area;
		//draw the attached string at the bottom of the screen

		area.left=MENU_LEFTXEDGE;
		area.right=MENU_RIGHTXEDGE;
		area.top=420;
		area.bottom=ScreenDescriptorBlock.SDB_Height;

		RenderSmallFontString_Wrapped(GetTextString(elementPtr->HelpString),&area,BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT,0,0);
		
	}
}

static void RenderConfigurationDescriptionString()
{
	const char* text=GetMultiplayerConfigDescription(AvPMenus.CurrentlySelectedElement);
	if(text)
	{
		RECT area;
		//draw the text at the bottom of the screen
		//now at the top.

		area.left=MENU_LEFTXEDGE;
		area.right=MENU_RIGHTXEDGE;
		area.top=0;
		area.bottom=60;

		RenderSmallFontString_Wrapped(text,&area,BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT,0,0);
	}
}


static void ActUponUsersInput(void)
{
	static int BackspaceTimer=0;
	//Set up a keyboard repeat rate thingy for deleting long strings
	if(KeyboardInput[KEY_BACKSPACE])
	{
		BackspaceTimer+=RealFrameTime;
	}
	else
	{
		BackspaceTimer=0;
	}
	
	if (AvPMenus.UserEnteringText)
	{
		AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];

		if (DebouncedKeyboardInput[KEY_ESCAPE] || DebouncedKeyboardInput[KEY_CR])
		{
			elementPtr->c.TextPtr[AvPMenus.PositionInTextField] = 0;
			AvPMenus.UserEnteringText = 0;

			// KJL 10:09:35 09/02/00 - when the user has entered their name,
			// move down to the next option. If the user enters a null
			// string, replace it with a placeholder name		
			if((AvPMenus.CurrentMenu == AVPMENU_USERPROFILEENTERNAME)
 			 ||(AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER_SKIRMISH)
 			 ||(AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYER))
			{
				if(AvPMenus.PositionInTextField==0)
				{
					strcpy(elementPtr->c.TextPtr,"DeadMeat");
				}
				AvPMenus.CurrentlySelectedElement++;
			}
		}
		else if (DebouncedKeyboardInput[KEY_BACKSPACE] || DebouncedKeyboardInput[KEY_LEFT])
		{
			if (AvPMenus.PositionInTextField>0)
			{
				elementPtr->c.TextPtr[--AvPMenus.PositionInTextField] = 0;
			}
		}
		else if(BackspaceTimer>ONE_FIXED/2)
		{
			//check for backspace being held down for a long time
			while(BackspaceTimer>ONE_FIXED/2)
			{
				BackspaceTimer-=ONE_FIXED/20;
				if (AvPMenus.PositionInTextField>0)
				{
					elementPtr->c.TextPtr[--AvPMenus.PositionInTextField] = 0;
				}
			}
		}
		else
		{
			//allow Ctrl+V to paste from the clipboard (really just for pasting in ip addresses)
			if((KeyboardInput[KEY_LEFTCTRL] || KeyboardInput[KEY_RIGHTCTRL]) && KeyboardInput[KEY_V])
			{
				PasteFromClipboard(elementPtr->c.TextPtr,elementPtr->b.MaxTextLength);
				AvPMenus.PositionInTextField = strlen(elementPtr->c.TextPtr);
			}
			else if (AvPMenus.PositionInTextField<elementPtr->b.MaxTextLength)
			{
				char c=0;
				KeyboardEntryQueue_StartProcessing();
				
				while((c=KeyboardEntryQueue_ProcessCharacter()))
				{
					if (AvPMenus.PositionInTextField<elementPtr->b.MaxTextLength)
					{
						//see if there is room for this character
						if(AvPMenus.FontToUse==AVPMENU_FONT_BIG && elementPtr->ElementID !=AVPMENU_ELEMENT_TEXTFIELD_SMALLWRAPPED)
						{
							//using large font
							//allocate 32 pixels for each new character for the moment
							//the true amount used will be worked out when the font is drawn
							//Might cause a slight glitch for fast typists , but I forgot about 
							//the damned kerned fonts until after I'd written most of this
							if(AvPMenus.WidthLeftForText<32) break;
							AvPMenus.WidthLeftForText-=32;

						}
						else
						{
							extern char AAFontWidths[256];
							//using small font
							if(AvPMenus.WidthLeftForText<AAFontWidths[(unsigned char)c]) break;
							AvPMenus.WidthLeftForText-=AAFontWidths[(unsigned char)c];
						}

						elementPtr->c.TextPtr[AvPMenus.PositionInTextField++] = c;
						elementPtr->c.TextPtr[AvPMenus.PositionInTextField] = 0;
					}
				}

			}
		
			KeyboardEntryQueue_Clear();
			
		}
	}
	else if (AvPMenus.UserEnteringNumber)
	{
		AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];

		if (DebouncedKeyboardInput[KEY_ESCAPE] || DebouncedKeyboardInput[KEY_CR])
		{
			AvPMenus.UserEnteringNumber = 0;
		}
		else if (DebouncedKeyboardInput[KEY_BACKSPACE] || DebouncedKeyboardInput[KEY_LEFT])
		{
			(*elementPtr->c.NumberPtr)/=10;
		}
		else
		{
			char c=0;
			
			KeyboardEntryQueue_StartProcessing();
			while((c=KeyboardEntryQueue_ProcessCharacter()))
			{
				if (AvPMenus.PositionInTextField<elementPtr->b.MaxTextLength)
				{
					if(c>='0' && c<='9')
					{
						(*elementPtr->c.NumberPtr)*=10;
						(*elementPtr->c.NumberPtr)+=c-'0';

						if((*elementPtr->c.NumberPtr)>elementPtr->b.MaxValue)
						{
							(*elementPtr->c.NumberPtr)=elementPtr->b.MaxValue;
						}
					}
				}
			}
		}
		KeyboardEntryQueue_Clear();
	}
	else if (AvPMenus.UserChangingKeyConfig)
	{
		if (DebouncedKeyboardInput[KEY_ESCAPE])
		{
			AvPMenus.UserChangingKeyConfig = 0;
		}
		else
		{
			signed int key,selectedKey=-1;

			// see if a valid key has been pressed
			for (key = 0 ; key <= MAX_NUMBER_OF_INPUT_KEYS ; key++)
			{
				if (!(key == KEY_ESCAPE) &&
//					!(key >= KEY_F1 && key <= KEY_F12) &&
					!(key >= KEY_0 && key <= KEY_9) )
				{
					if (DebouncedKeyboardInput[key])
					{
						selectedKey = key;
						break;
					}
				}
			}
			
			if (AvPMenus.ChangingPrimaryConfig)
			{
				if (selectedKey!=-1)
				{
					*(((unsigned char*)&PlayerInputPrimaryConfig)+AvPMenus.CurrentlySelectedElement-2) = selectedKey;
					AvPMenus.UserChangingKeyConfig=0;
				}
			}
			else // changing Secondary
			{
				if (selectedKey!=-1)
				{
					*(((unsigned char*)&PlayerInputSecondaryConfig)+AvPMenus.CurrentlySelectedElement-2) = selectedKey;
					AvPMenus.UserChangingKeyConfig=0;
				}
			}
		}
	}
	else
	{	
		if (DebouncedKeyboardInput[KEY_ESCAPE] && (AvPMenus.CurrentMenu != AVPMENU_MAIN && AvPMenus.CurrentMenu != AVPMENU_INGAME))
		{
			//if (AvPMenus.CurrentMenu == AVPMENU_MULTIPLAYERJOINGAME2)
			switch(AvPMenus.CurrentMenu)
			{
				case AVPMENU_MULTIPLAYER_CONFIG_JOIN:
				{
					AddNetMsg_PlayerLeaving();
					NetSendMessages();
					if(!LobbiedGame)
					{
						DirectPlay_Disconnect();
					}
					break;
				}
				case AVPMENU_DETAILLEVELS:
				{
					SetDetailLevelsFromMenu();
					SaveUserProfile(UserProfilePtr);
					if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
					{
						SetupNewMenu(AVPMENU_INGAMEAVOPTIONS);
					}
					break;
				}
				case AVPMENU_INGAMEAVOPTIONS:
				case AVPMENU_MAINMENUAVOPTIONS:
				{
					SaveUserProfile(UserProfilePtr);
					break;
				}
				case AVPMENU_CHEATOPTIONS:
				{
					CheatMode_Active = CHEATMODE_NONACTIVE;
					break;	
				}

				case AVPMENU_MULTIPLAYER_CONFIG :
				case AVPMENU_SKIRMISH_CONFIG :
				{
					//reload the previous multiplayer configuration
					extern char MP_Config_Description[];
					LoadMultiplayerConfiguration(GetTextString(TEXTSTRING_PREVIOUSGAME_FILENAME));
					MP_Config_Description[0]=0;
					break;
				}
				
				default:
					break;
			}


			if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
			{
				SetupNewMenu(AVPMENU_INGAME);
			}
			else
			{
				SetupNewMenu(AvPMenusData[AvPMenus.CurrentMenu].ParentMenu);
			}
		}
		else if (IDemandSelect()) // select element
		{
			if (InputIsDebounced)
			{
				InteractWithMenuElement(AVPMENU_ELEMENT_INTERACTION_SELECT);
				InputIsDebounced = 0;
			}
		}
		else if (IDemandGoForward()) // previous element
		{
			if (InputIsDebounced)
			{
				switch (AvPMenus.CurrentMenu)
				{
					case AVPMENU_USERPROFILESELECT:
					case AVPMENU_MARINELEVELS:
					case AVPMENU_PREDATORLEVELS:
					case AVPMENU_ALIENLEVELS:
					{
						InteractWithMenuElement(AVPMENU_ELEMENT_INTERACTION_DECREASE);
						break;
					}
					case AVPMENU_MARINEKEYCONFIG:
				    case AVPMENU_PREDATORKEYCONFIG:
					case AVPMENU_ALIENKEYCONFIG:
					{
						if (AvPMenus.CurrentlySelectedElement>0)
						{
							AvPMenus.CurrentlySelectedElement--;
							Sound_Play(SID_MENUS_CHANGE_ITEM,"r");
						}
						break;
					}
					default:
					{
						AvPMenus.CurrentlySelectedElement--;
						if (AvPMenus.CurrentlySelectedElement<0)
						{
							AvPMenus.CurrentlySelectedElement= AvPMenus.NumberOfElementsInMenu-1;
						}
						Sound_Play(SID_MENUS_CHANGE_ITEM,"r");
						break;
					}
				}
				InputIsDebounced = 0;
			}
			else
			{
				KeyDepressedCounter += RealFrameTime;
			}

		}
		else if (IDemandGoBackward()) // next element
		{
			if (InputIsDebounced)
			{
				switch (AvPMenus.CurrentMenu)
				{
					case AVPMENU_USERPROFILESELECT:
			 	       case AVPMENU_MARINELEVELS:
					case AVPMENU_PREDATORLEVELS:
					case AVPMENU_ALIENLEVELS:
					{
						InteractWithMenuElement(AVPMENU_ELEMENT_INTERACTION_INCREASE);
						break;
					}
					case AVPMENU_MARINEKEYCONFIG:
				    case AVPMENU_PREDATORKEYCONFIG:
					case AVPMENU_ALIENKEYCONFIG:
					{
						if (AvPMenus.CurrentlySelectedElement<AvPMenus.NumberOfElementsInMenu-1)
						{
							AvPMenus.CurrentlySelectedElement++;
							Sound_Play(SID_MENUS_CHANGE_ITEM,"r");
						}
						break;
					}
					default:
					{
						AvPMenus.CurrentlySelectedElement++;
						if (AvPMenus.CurrentlySelectedElement>=AvPMenus.NumberOfElementsInMenu)
						{
							AvPMenus.CurrentlySelectedElement= 0;
						}
						Sound_Play(SID_MENUS_CHANGE_ITEM,"r");
						break;
					}
				}

				InputIsDebounced = 0;
			}
			else
			{
				KeyDepressedCounter += RealFrameTime;
			}
		}
		else if (IDemandTurnLeft()) 
		{
			if (InputIsDebounced)
			{
				InteractWithMenuElement(AVPMENU_ELEMENT_INTERACTION_DECREASE);
				InputIsDebounced = 0;
			}
			else
			{
				KeyDepressedCounter += RealFrameTime;
			}
		}
		else if (IDemandTurnRight()) 
		{
			if (InputIsDebounced)
			{
				InteractWithMenuElement(AVPMENU_ELEMENT_INTERACTION_INCREASE);
				InputIsDebounced = 0;
			}
			else
			{
				KeyDepressedCounter += RealFrameTime;
			}
		}
		else if (DebouncedKeyboardInput[KEY_BACKSPACE]) 
		{
			switch (AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement].ElementID)
			{
				case AVPMENU_ELEMENT_KEYCONFIG:
				{
					if (!KeyConfigSelectionColumn)
					{
						*(((unsigned char*)&PlayerInputPrimaryConfig)+AvPMenus.CurrentlySelectedElement-2) = KEY_VOID;
					}
					else // changing Secondary
					{
						*(((unsigned char*)&PlayerInputSecondaryConfig)+AvPMenus.CurrentlySelectedElement-2) = KEY_VOID;
					}
					break;
				}
				case AVPMENU_ELEMENT_USERPROFILE:
				{
					if (UserProfileNumber)
					{
						SetupNewMenu(AVPMENU_USERPROFILEDELETE);
					}
					break;
				}
				case AVPMENU_ELEMENT_LOADMPCONFIG :
				{
					//take note of the current configuration
					MultiplayerConfigurationIndex=AvPMenus.CurrentlySelectedElement;
					MultiplayerConfigurationName=AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement].c.TextPtr;
					//setup the delete configuration menu
					SetupNewMenu(AVPMENU_MULTIPLAYER_DELETECONFIG);
					break;
				}
				default:
					break;
			}
		}
		else  
		{
			InputIsDebounced=1;
			KeyDepressedCounter = 0;
		}

		if (KeyDepressedCounter>ONE_FIXED)
			InputIsDebounced = 1;
	}

}
static void InteractWithMenuElement(enum AVPMENU_ELEMENT_INTERACTION_ID interactionID)
{
	AVPMENU_ELEMENT *elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];

	if (interactionID==AVPMENU_ELEMENT_INTERACTION_SELECT)
	{
		Sound_Play(SID_MENUS_SELECT_ITEM,"r");
	}
	else
	{
		Sound_Play(SID_MENUS_CHANGE_ITEM,"r");
	}
	switch(elementPtr->ElementID)
	{
		default:
		GLOBALASSERT("UNKNOWN MENU ELEMENT"==0);
		case AVPMENU_ELEMENT_GOTOMENU:
		case AVPMENU_ELEMENT_GOTOMENU_GFX:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				//Check to see if we are on one of the menus for entering multiplayer name
				if(AvPMenus.CurrentMenu==AVPMENU_MULTIPLAYER ||
				   AvPMenus.CurrentMenu==AVPMENU_MULTIPLAYER_LOBBIEDSERVER)
				{
					//save profile , so that multiplayer name is remembered
					SaveUserProfile(UserProfilePtr);
				}
			
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}
		case AVPMENU_ELEMENT_SAVEMPCONFIG :
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				if(MP_Config_Name[0])
				{
					SaveMultiplayerConfiguration(MP_Config_Name);
				}
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}
		case AVPMENU_ELEMENT_TEXTFIELD:
		{
			AvPMenus.UserEnteringText = 1;
			KeyboardEntryQueue_Clear();
			AvPMenus.PositionInTextField = strlen(elementPtr->c.TextPtr);
			elementPtr->c.TextPtr[AvPMenus.PositionInTextField] = 0;
			AvPMenus.WidthLeftForText = 0; //will be calculated properly when menus are drawn
			break;
		}
		case AVPMENU_ELEMENT_TEXTFIELD_SMALLWRAPPED:
		{
			AvPMenus.UserEnteringText = 1;
			KeyboardEntryQueue_Clear();
			AvPMenus.PositionInTextField = strlen(elementPtr->c.TextPtr);
			AvPMenus.WidthLeftForText = 0; //will be calculated properly when menus are drawn
			break;
		}

		case AVPMENU_ELEMENT_NUMBERFIELD:
		{
			if(interactionID == AVPMENU_ELEMENT_INTERACTION_DECREASE)
			{
				(*elementPtr->c.NumberPtr)--;
				if(*elementPtr->c.NumberPtr<0)
				{
					*elementPtr->c.NumberPtr=0;
				}
			}
			else if(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE)
			{
				(*elementPtr->c.NumberPtr)++;
				if(*elementPtr->c.NumberPtr>elementPtr->b.MaxValue)
				{
					*elementPtr->c.NumberPtr=elementPtr->b.MaxValue;
				}
				
			}
			else
			{
				*elementPtr->c.NumberPtr=0;
				AvPMenus.UserEnteringNumber = 1;
				KeyboardEntryQueue_Clear();
			}
			break;
		}
		case AVPMENU_ELEMENT_DUMMYTEXTFIELD:
		case AVPMENU_ELEMENT_DUMMYTEXTSLIDER:
		case AVPMENU_ELEMENT_DUMMYTEXTSLIDER_POINTER:
		case AVPMENU_ELEMENT_DUMMYNUMBERFIELD:
		case AVPMENU_ELEMENT_TOGGLE:
		{
			break;
		}
		case AVPMENU_ELEMENT_SLIDER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				if (*elementPtr->c.SliderValuePtr<elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr+=1;
				}
			}
			else
			{
				if (*elementPtr->c.SliderValuePtr>0)
				{
					*elementPtr->c.SliderValuePtr-=1;
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_TEXTSLIDER:
		case AVPMENU_ELEMENT_TEXTSLIDER_POINTER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				*elementPtr->c.SliderValuePtr+=1;
				if (*elementPtr->c.SliderValuePtr>elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr=0;
				}
			}
			else
			{
				*elementPtr->c.SliderValuePtr-=1;
				if (*elementPtr->c.SliderValuePtr<0)
				{
					*elementPtr->c.SliderValuePtr=elementPtr->b.MaxSliderValue;
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_SPECIES_TEXTSLIDER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				*elementPtr->c.SliderValuePtr+=1;
				if (*elementPtr->c.SliderValuePtr>elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr=0;
				}
				GetNextAllowedSpecies(elementPtr->c.SliderValuePtr,TRUE);
			}
			else
			{
				*elementPtr->c.SliderValuePtr-=1;
				if (*elementPtr->c.SliderValuePtr<0)
				{
					*elementPtr->c.SliderValuePtr=elementPtr->b.MaxSliderValue;
				}
				GetNextAllowedSpecies(elementPtr->c.SliderValuePtr,FALSE);
			}
			break;
		}
		case AVPMENU_ELEMENT_CHEATMODE_SPECIES_TEXTSLIDER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				*elementPtr->c.SliderValuePtr+=1;
				if (*elementPtr->c.SliderValuePtr>elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr=0;
				}
				CheatMode_GetNextAllowedSpecies(elementPtr->c.SliderValuePtr,TRUE);
			}
			else
			{
				*elementPtr->c.SliderValuePtr-=1;
				if (*elementPtr->c.SliderValuePtr<0)
				{
					*elementPtr->c.SliderValuePtr=elementPtr->b.MaxSliderValue;
				}
				CheatMode_GetNextAllowedSpecies(elementPtr->c.SliderValuePtr,FALSE);
			}
			break;
		}
		case AVPMENU_ELEMENT_CHEATMODE_TEXTSLIDER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				*elementPtr->c.SliderValuePtr+=1;
				if (*elementPtr->c.SliderValuePtr>elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr=0;
				}
				CheatMode_GetNextAllowedMode(elementPtr->c.SliderValuePtr,TRUE);
			}
			else
			{
				*elementPtr->c.SliderValuePtr-=1;
				if (*elementPtr->c.SliderValuePtr<0)
				{
					*elementPtr->c.SliderValuePtr=elementPtr->b.MaxSliderValue;
				}
				CheatMode_GetNextAllowedMode(elementPtr->c.SliderValuePtr,FALSE);
			}
			break;
		}
		case AVPMENU_ELEMENT_CHEATMODE_ENVIRONMENT_TEXTSLIDER:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				*elementPtr->c.SliderValuePtr+=1;
				if (*elementPtr->c.SliderValuePtr>elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr=0;
				}
				CheatMode_GetNextAllowedEnvironment(elementPtr->c.SliderValuePtr,TRUE);
			}
			else
			{
				*elementPtr->c.SliderValuePtr-=1;
				if (*elementPtr->c.SliderValuePtr<0)
				{
					*elementPtr->c.SliderValuePtr=elementPtr->b.MaxSliderValue;
				}
				CheatMode_GetNextAllowedEnvironment(elementPtr->c.SliderValuePtr,FALSE);
			}
			break;
		}

		case AVPMENU_ELEMENT_QUITGAME:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					AvP.MainLoopRunning = 0;
				}
				AvPMenus.MenusState = MENUSSTATE_OUTSIDEMENUS;
			}
			break;
		}
		
		case AVPMENU_ELEMENT_USERPROFILE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				/* a profile has been selected */
				int i;
				UserProfilePtr = GetFirstUserProfile();
				for (i=0; i<UserProfileNumber; i++)
					UserProfilePtr = GetNextUserProfile();

  				if (UserProfileNumber)
  				{
 					// Edmond too
 				 	GetSettingsFromUserProfile();
 					// Edmond
 					if (LobbiedGame == LobbiedGame_Server)
 					{
 						SetupNewMenu(AVPMENU_MULTIPLAYER_CONFIG);			// Edmond
 					}
 					else if (LobbiedGame == LobbiedGame_Client)
 					{
 						SetupNewMenu(AVPMENU_MULTIPLAYER_LOBBIEDCLIENT);
 					}
 					else
 						SetupNewMenu(AVPMENU_MAIN);
  				}
				else
				{
					SetupNewMenu(AVPMENU_USERPROFILEENTERNAME);
				}
			}
			else if (interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE)
			{
				if (*elementPtr->c.SliderValuePtr<elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr+=1;
					EpisodeSelectScrollOffset-=ONE_FIXED;
				}
			}
			else
			{
				if (*elementPtr->c.SliderValuePtr>0)
				{
					*elementPtr->c.SliderValuePtr-=1;
					EpisodeSelectScrollOffset+=ONE_FIXED;
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_USERPROFILE_DELETE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				DeleteUserProfile(UserProfileNumber);
				SetupNewMenu(AVPMENU_USERPROFILESELECT);
			}
			break;  
		}
		case AVPMENU_ELEMENT_DELETEMPCONFIG:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				DeleteMultiplayerConfigurationByIndex(MultiplayerConfigurationIndex);
				//go back to the load config menu
				SetupNewMenu(AVPMENU_MULTIPLAYER_LOADCONFIG);
				
			}
			break;  
		}
		case AVPMENU_ELEMENT_DIFFICULTYLEVEL:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				if (AvPMenus.CurrentMenu == AVPMENU_LEVELBRIEFING_BASIC)
				{
					AvP.Difficulty = AvPMenus.CurrentlySelectedElement;
				}
				else
				{
					AvP.Difficulty = 1;
				}
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
			}
			break;
		}

		case AVPMENU_ELEMENT_ALIENEPISODE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT
			  &&MaximumSelectableLevel>=*elementPtr->c.SliderValuePtr)
			{
				AvP.PlayerType = I_Alien;
				SetLevelToLoadForAlien(AlienEpisodeToPlay);

				if (AlienEpisodeToPlay<MAX_NO_OF_BASIC_ALIEN_EPISODES)
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BASIC);
				}
				else
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BONUS);
				}
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu = AVPMENU_ALIENLEVELS;
				break;
			}
			/* else let it fall through */

		}
		case AVPMENU_ELEMENT_MARINEEPISODE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT
				&&MaximumSelectableLevel>=*elementPtr->c.SliderValuePtr)
			{
				AvP.PlayerType = I_Marine;
				SetLevelToLoadForMarine(MarineEpisodeToPlay);

				if (MarineEpisodeToPlay<MAX_NO_OF_BASIC_MARINE_EPISODES)
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BASIC);
				}
				else
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BONUS);
				}
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu = AVPMENU_MARINELEVELS;
				break;
			}
			/* else let it fall through */
		}
		case AVPMENU_ELEMENT_PREDATOREPISODE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT
				&&MaximumSelectableLevel>=*elementPtr->c.SliderValuePtr)
			{
				AvP.PlayerType = I_Predator;
				SetLevelToLoadForPredator(PredatorEpisodeToPlay);

				if (PredatorEpisodeToPlay<MAX_NO_OF_BASIC_PREDATOR_EPISODES)
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BASIC);
				}
				else
				{
					SetupNewMenu(AVPMENU_LEVELBRIEFING_BONUS);
				}
				AvPMenusData[AvPMenus.CurrentMenu].ParentMenu = AVPMENU_PREDATORLEVELS;
				break;
			}
			/* else let it fall through */
		}
		{
			/* This code is reached when an EPISODE element has been
			activated but not "selected" i.e. the user wants to change
			the current episode, not start playing the current episode */
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE)
			{
				if (*elementPtr->c.SliderValuePtr<elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr+=1;
					EpisodeSelectScrollOffset-=ONE_FIXED;
				}
			}
			else
			{
				if (*elementPtr->c.SliderValuePtr>0)
				{
					*elementPtr->c.SliderValuePtr-=1;
					EpisodeSelectScrollOffset+=ONE_FIXED;
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_BUTTONSETTING:
		{
			
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				break;
			}
			/* else let it fall through */

			if (interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE)
			{
				if (*elementPtr->c.SliderValuePtr<elementPtr->b.MaxSliderValue)
				{
					*elementPtr->c.SliderValuePtr+=1;
					EpisodeSelectScrollOffset-=ONE_FIXED;
				}
			}
			else
			{
				if (*elementPtr->c.SliderValuePtr>0)
				{
					*elementPtr->c.SliderValuePtr-=1;
					EpisodeSelectScrollOffset+=ONE_FIXED;
				}
			}
			break;
		}

		case AVPMENU_ELEMENT_STARTMARINEDEMO:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
				AvP.PlayerType = I_Marine;
				AvP.Difficulty = 1;
				SetLevelToLoad(AVP_ENVIRONMENT_INVASION);
			}
			break;
		}
		case AVPMENU_ELEMENT_STARTPREDATORDEMO:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
				AvP.PlayerType = I_Predator;
				AvP.Difficulty = 1;
				SetLevelToLoad(AVP_ENVIRONMENT_INVASION_P);
			}
			break;
		}
		case AVPMENU_ELEMENT_STARTALIENDEMO:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
				AvP.PlayerType = I_Alien;
				AvP.Difficulty = 1;
				SetLevelToLoad(AVP_ENVIRONMENT_INVASION_A);
			}
			break;
		}
		case AVPMENU_ELEMENT_STARTLEVELWITHCHEAT:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
				AvP.PlayerType = CheatMode_Species;
				AvP.Difficulty = 1;
				SetLevelToLoadForCheatMode(CheatMode_Environment);
			}
			break;
		}
		case AVPMENU_ELEMENT_STARTMPGAME:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				extern int DirectPlay_HostGame(char *playerName, char *sessionName,int species,int gamestyle,int level);
				extern char MP_PlayerName[];
				extern char MP_SessionName[];
				extern int MP_Species;
				extern char MP_Config_Description[];
//				extern int MP_GameStyle;
//				extern int MP_LevelNumber;

				//save the 'Previous Game' multiplayer configuration
				strcpy(MP_Config_Description,GetTextString(TEXTSTRING_PREVIOUSGAME_FILENAME));
				SaveMultiplayerConfiguration(GetTextString(TEXTSTRING_PREVIOUSGAME_FILENAME));

				
				AvP.Difficulty = 1;

				if (DirectPlay_HostGame(MP_PlayerName,MP_SessionName,MP_Species,netGameData.gameType,netGameData.levelNumber))
				{
					AvPMenus.MenusState = MENUSSTATE_STARTGAME;
					if(netGameData.gameType==NGT_Coop)
						SetLevelToLoadForCooperative(netGameData.levelNumber);
					else
						SetLevelToLoadForMultiplayer(netGameData.levelNumber);
	
					
					SetBriefingTextForMultiplayer();
					
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_JOINMPGAME:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
				netGameData.myStartFlag = 1;	    
//				netGameData.myGameState = NGS_Playing;
				if(netGameData.gameType==NGT_Coop)
					SetLevelToLoadForCooperative(netGameData.levelNumber);
				else
					SetLevelToLoadForMultiplayer(netGameData.levelNumber);
				SetBriefingTextForMultiplayer();
			}
			break;
		}
		case AVPMENU_ELEMENT_LISTCHOICE:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				extern char MP_PlayerName[];
				extern char MP_SessionName[];
				int s = AvPMenus.CurrentlySelectedElement;

				if(SessionData[s].AllowedToJoin)
				{
					//copy the session name , leaving of the player count information
					char * braket_pos;
					strcpy(MP_SessionName,SessionData[s].Name);
										
					braket_pos=strrchr(MP_SessionName,'(');
					if(braket_pos) *braket_pos=0;

					if(DirectPlay_ConnectToSession(s,MP_PlayerName))
						SetupNewMenu(elementPtr->b.MenuToGoTo);
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_JOINLOBBIED :
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				//save profile , so that multiplayer name is remembered
				SaveUserProfile(UserProfilePtr);

				InitAVPNetGameForJoin();
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}

		case AVPMENU_ELEMENT_LOADMPCONFIG:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				LoadMultiplayerConfigurationByIndex(AvPMenus.CurrentlySelectedElement);
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}

		case AVPMENU_ELEMENT_LOADIPADDRESS:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				LoadIPAddress(elementPtr->c.TextPtr);
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}

		case AVPMENU_ELEMENT_CONNECTIONCHOICE :
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
 				netGameData.connectionType=elementPtr->c.Value;
				if(netGameData.connectionType == CONN_Mplayer)
				{
					//exit the game and launch the mplayer stuff
				fprintf(stderr, "STUB: InteractWithMenuElement (launching mplayer...)\n");
				#if 0
					LaunchingMplayer=TRUE;
					LaunchMplayer();
				#endif					
					AvP.MainLoopRunning = 0;
					AvPMenus.MenusState = MENUSSTATE_OUTSIDEMENUS; 
					break;
				}											   
				else
				{
 					SetupNewMenu(elementPtr->b.MenuToGoTo);
				}
						
			}
			break;
		}
		
		case AVPMENU_ELEMENT_VIDEOMODE:
		{
			if ((interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			  ||(interactionID == AVPMENU_ELEMENT_INTERACTION_INCREASE))
			{
				NextVideoMode2();
			}
			else
			{
				PreviousVideoMode2();
			}
			break;
		}
		case AVPMENU_ELEMENT_VIDEOMODEOK:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				SaveDeviceAndVideoModePreferences();
				SetupNewMenu(elementPtr->b.MenuToGoTo);
			}
			break;
		}
		case AVPMENU_ELEMENT_RESUMEGAME:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
			}
			break;
		}
		case AVPMENU_ELEMENT_RESTARTGAME:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvP.RestartLevel=1;
				AvPMenus.MenusState = MENUSSTATE_STARTGAME;
			}
			break;
		}

		case AVPMENU_ELEMENT_KEYCONFIG:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				AvPMenus.UserChangingKeyConfig = 1;
				if (KeyConfigSelectionColumn)
				{
					AvPMenus.ChangingPrimaryConfig = 0;
				}
				else
				{
					AvPMenus.ChangingPrimaryConfig = 1;
				}
			}
			else
			{
				KeyConfigSelectionColumn=!KeyConfigSelectionColumn;     
			}
			break;
		}
		case AVPMENU_ELEMENT_KEYCONFIGOK:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				switch (AvPMenus.CurrentMenu)
				{
					case AVPMENU_MARINEKEYCONFIG:
					{
						MarineInputPrimaryConfig   = PlayerInputPrimaryConfig;
						MarineInputSecondaryConfig = PlayerInputSecondaryConfig;
						{
							extern int AutoWeaponChangeOn_Temp;
							extern int AutoWeaponChangeOn;
							AutoWeaponChangeOn = AutoWeaponChangeOn_Temp;
						}
						break;
					}
					case AVPMENU_PREDATORKEYCONFIG:
					{
						PredatorInputPrimaryConfig       = PlayerInputPrimaryConfig;
						PredatorInputSecondaryConfig = PlayerInputSecondaryConfig;
						break;
					}
					case AVPMENU_ALIENKEYCONFIG:
					{
						AlienInputPrimaryConfig   = PlayerInputPrimaryConfig;
						AlienInputSecondaryConfig = PlayerInputSecondaryConfig;
						break;
					}
					case AVPMENU_CONTROLS:
					{
						ControlMethods = PlayerControlMethods;
						break;
					}
					case AVPMENU_JOYSTICKCONTROLS:
					{
						JoystickControlMethods = PlayerJoystickControlMethods;
						break;
					}
					
					default:
						break;
				}
								
				
				SaveUserProfile(UserProfilePtr);

				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					SetupNewMenu(AVPMENU_INGAME);
				}
				else
				{
					SetupNewMenu(AVPMENU_OPTIONS);
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_RESETKEYCONFIG:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				switch (AvPMenus.CurrentMenu)
				{
					case AVPMENU_MARINEKEYCONFIG:
					{
						PlayerInputPrimaryConfig = DefaultMarineInputPrimaryConfig;
						PlayerInputSecondaryConfig = DefaultMarineInputSecondaryConfig;
						break;
					}
					case AVPMENU_PREDATORKEYCONFIG:
					{
						PlayerInputPrimaryConfig = DefaultPredatorInputPrimaryConfig;
						PlayerInputSecondaryConfig = DefaultPredatorInputSecondaryConfig;
						break;
					}
					case AVPMENU_ALIENKEYCONFIG:
					{
						PlayerInputPrimaryConfig = DefaultAlienInputPrimaryConfig;
						PlayerInputSecondaryConfig = DefaultAlienInputSecondaryConfig;
						break;
					}
					case AVPMENU_CONTROLS:
					{
						PlayerControlMethods = DefaultControlMethods;
						break;
					}
					case AVPMENU_JOYSTICKCONTROLS:
					{
						PlayerJoystickControlMethods = DefaultJoystickControlMethods;
						break;
					}
					
					default:
						break;
				}
			}
			break;
		}

		case AVPMENU_ELEMENT_RESETMPCONFIG :
		{
			//reset the multiplayer configuration
			extern void SetDefaultMultiplayerConfig();
			SetDefaultMultiplayerConfig();
			break;
		}

		case AVPMENU_ELEMENT_SAVESETTINGS:
		{
			if (interactionID == AVPMENU_ELEMENT_INTERACTION_SELECT)
			{
				SetDetailLevelsFromMenu();
				SaveUserProfile(UserProfilePtr);
				
				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					if(AvPMenus.CurrentMenu == AVPMENU_DETAILLEVELS)
					{
						SetupNewMenu(AVPMENU_INGAMEAVOPTIONS);
					}
					else
					{
						SetupNewMenu(AVPMENU_INGAME);
					}
				}
				else
				{
					SetupNewMenu(AvPMenusData[AvPMenus.CurrentMenu].ParentMenu);
				}
			}
			break;
		}
	
		case AVPMENU_ELEMENT_LOADGAME:
		{
			int slot = AvPMenus.CurrentlySelectedElement;
			if(slot>=0 && slot<NUMBER_OF_SAVE_SLOTS)
			{
				if(SaveGameSlot[slot].SlotUsed)
				{
					//set the request
					LoadGameRequest = slot;
					if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
					{
						//leave the menus
						AvPMenus.MenusState = MENUSSTATE_STARTGAME;	
					}
				}
			}
			break;
		}
		case AVPMENU_ELEMENT_SAVEGAME:
		{
			int slot = AvPMenus.CurrentlySelectedElement;
			if(slot>=0 && slot<NUMBER_OF_SAVE_SLOTS)
			{
				//set the request
				SaveGameRequest = slot;
				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					//leave the menus
					AvPMenus.MenusState = MENUSSTATE_STARTGAME;	
				}
			}
			break;
		}
	}
}


int LengthOfMenuText(char *textPtr);
int LengthOfSmallMenuText(char *textPtr);

static void RenderMenuElement(AVPMENU_ELEMENT *elementPtr, int e, int y)
{
	int (*RenderText)(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format);
	int (*RenderText_Coloured)(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int r, int g, int b);
	int (*MenuTextLength)(char *textPtr);
	
	if (AvPMenus.FontToUse==AVPMENU_FONT_BIG)
	{
		RenderText = RenderMenuText;
		MenuTextLength = LengthOfMenuText;
	}
	else
	{
		if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
		{
			RenderText = Hardware_RenderSmallMenuText;
			RenderText_Coloured = Hardware_RenderSmallMenuText_Coloured;

		}
		else
		{
			RenderText = RenderSmallMenuText;
			RenderText_Coloured = RenderSmallMenuText_Coloured;
		}
		MenuTextLength = LengthOfSmallMenuText;
	}

	switch(elementPtr->ElementID)
	{
		default:
		GLOBALASSERT("UNKNOWN MENU ELEMENT"==0);
		case AVPMENU_ELEMENT_GOTOMENU:
		case AVPMENU_ELEMENT_QUITGAME:
		case AVPMENU_ELEMENT_STARTMPGAME:
		case AVPMENU_ELEMENT_JOINMPGAME:
		case AVPMENU_ELEMENT_VIDEOMODEOK:
		case AVPMENU_ELEMENT_RESUMEGAME:
		case AVPMENU_ELEMENT_RESTARTGAME:
		case AVPMENU_ELEMENT_KEYCONFIGOK:
		case AVPMENU_ELEMENT_RESETKEYCONFIG:
		case AVPMENU_ELEMENT_STARTMARINEDEMO:
		case AVPMENU_ELEMENT_STARTPREDATORDEMO:
		case AVPMENU_ELEMENT_STARTALIENDEMO:
		case AVPMENU_ELEMENT_STARTLEVELWITHCHEAT:
		case AVPMENU_ELEMENT_SAVEMPCONFIG:
		case AVPMENU_ELEMENT_DIFFICULTYLEVEL:
		case AVPMENU_ELEMENT_JOINLOBBIED:
		case AVPMENU_ELEMENT_CONNECTIONCHOICE :
		case AVPMENU_ELEMENT_USERPROFILE_DELETE:
		case AVPMENU_ELEMENT_RESETMPCONFIG:
		case AVPMENU_ELEMENT_DELETEMPCONFIG:
		case AVPMENU_ELEMENT_SAVESETTINGS:
		{
			char *textPtr = GetTextString(elementPtr->a.TextDescription);
			RenderText(textPtr,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			break;
		}
		#if 0
		case AVPMENU_ELEMENT_STARTALIENGAME:
		case AVPMENU_ELEMENT_STARTMARINEGAME:
		case AVPMENU_ELEMENT_STARTPREDATORGAME:
		{
			char *textPtr = GetTextString(elementPtr->a.TextDescription);
			RenderText(textPtr,MENU_LEFTXEDGE,MENU_BOTTOMYEDGE,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
			break;
		}
	
		case AVPMENU_ELEMENT_MARINEEPISODE:
		case AVPMENU_ELEMENT_ALIENEPISODE:
		case AVPMENU_ELEMENT_PREDATOREPISODE:
		{
			char *textPtr = GetTextString(elementPtr->a.TextDescription+*(elementPtr->SliderValuePtr));
			RenderText(textPtr,MENU_LEFTXEDGE,MENU_TOPYEDGE,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
			break;
		}
		#endif

		case AVPMENU_ELEMENT_DUMMYTEXTSLIDER:
		case AVPMENU_ELEMENT_DUMMYTEXTSLIDER_POINTER:
		case AVPMENU_ELEMENT_TEXTSLIDER:
		case AVPMENU_ELEMENT_TEXTSLIDER_POINTER:
		case AVPMENU_ELEMENT_SPECIES_TEXTSLIDER:
		case AVPMENU_ELEMENT_CHEATMODE_TEXTSLIDER:
		case AVPMENU_ELEMENT_CHEATMODE_SPECIES_TEXTSLIDER:
		case AVPMENU_ELEMENT_CHEATMODE_ENVIRONMENT_TEXTSLIDER:
		{
			char *textPtr = "";
			if(elementPtr->ElementID == AVPMENU_ELEMENT_TEXTSLIDER_POINTER ||
			   elementPtr->ElementID == AVPMENU_ELEMENT_DUMMYTEXTSLIDER_POINTER)	
			{
				//we have a pointer to the strings rather than the first string index
				if(elementPtr->d.TextSliderStringPointer)
				{
					textPtr = elementPtr->d.TextSliderStringPointer[*(elementPtr->c.SliderValuePtr)];
				}
			}
			else
			{
				//we have the index of the first string
				textPtr = GetTextString(elementPtr->d.FirstTextSliderString+*(elementPtr->c.SliderValuePtr));
			}
			
			if(elementPtr->a.TextDescription!=TEXTSTRING_BLANK)
			{
		
				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					RenderText
					(
						GetTextString(elementPtr->a.TextDescription),
						MENU_CENTREX-MENU_ELEMENT_SPACING,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_RIGHTJUSTIFIED
					);
					RenderText
					(
						textPtr,
						MENU_CENTREX+MENU_ELEMENT_SPACING,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_LEFTJUSTIFIED
					);

				}
				else
				{
					int length = MenuTextLength(textPtr);

					if (length>(ScreenDescriptorBlock.SDB_Width-MENU_CENTREX-MENU_ELEMENT_SPACING*2))
					{
						RenderText
						(
							GetTextString(elementPtr->a.TextDescription),
							ScreenDescriptorBlock.SDB_Width-MENU_ELEMENT_SPACING*2-length,
							y,
							elementPtr->Brightness,
							AVPMENUFORMAT_RIGHTJUSTIFIED
						);
						RenderText
						(
							textPtr,
							ScreenDescriptorBlock.SDB_Width-MENU_ELEMENT_SPACING,
							y,
							elementPtr->Brightness,
							AVPMENUFORMAT_RIGHTJUSTIFIED
						);
					}
					else
					{

						RenderText
						(
							GetTextString(elementPtr->a.TextDescription),
							MENU_CENTREX-MENU_ELEMENT_SPACING,
							y,																		 
							elementPtr->Brightness,
							AVPMENUFORMAT_RIGHTJUSTIFIED
						);
						RenderText
						(
							textPtr,
							MENU_CENTREX+MENU_ELEMENT_SPACING,
							y,
							elementPtr->Brightness,
							AVPMENUFORMAT_LEFTJUSTIFIED
						);
					}
				}
			}
			else
			{
				RenderText
				(
					textPtr,
					MENU_CENTREX,
					y,
					elementPtr->Brightness,
					AVPMENUFORMAT_CENTREJUSTIFIED
				);
			}
			break;
		}
		case AVPMENU_ELEMENT_DUMMYTEXTFIELD:
		case AVPMENU_ELEMENT_TEXTFIELD:
		{
			if (elementPtr->a.TextDescription==TEXTSTRING_BLANK)
			{
				if (AvPMenus.UserEnteringText && e==AvPMenus.CurrentlySelectedElement)
				{
					int b = GetSin(CloakingPhase&4095);
					int x = RenderText(elementPtr->c.TextPtr,MENU_CENTREX,y,elementPtr->Brightness/2,AVPMENUFORMAT_CENTREJUSTIFIED);
					x = RenderText("I",x,y,MUL_FIXED(b,b),AVPMENUFORMAT_CENTREJUSTIFIED);

					//work out how much space was left over
					if(AvPMenus.PositionInTextField)
						AvPMenus.WidthLeftForText = MENU_RIGHTXEDGE -x;
					else
						AvPMenus.WidthLeftForText = MENU_RIGHTXEDGE -MENU_CENTREX;
				}
				else
				{
					RenderText(elementPtr->c.TextPtr,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
				}
			}
			else
			{
				RenderText(GetTextString(elementPtr->a.TextDescription),MENU_CENTREX-MENU_ELEMENT_SPACING,y,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
			
				if (AvPMenus.UserEnteringText && e==AvPMenus.CurrentlySelectedElement)
				{
					int b = GetSin(CloakingPhase&4095);
					int x = RenderText(elementPtr->c.TextPtr,MENU_CENTREX+MENU_ELEMENT_SPACING,y,elementPtr->Brightness/2,AVPMENUFORMAT_LEFTJUSTIFIED);
					x = RenderText("I",x,y,MUL_FIXED(b,b),AVPMENUFORMAT_LEFTJUSTIFIED);
					
					//work out how much space was left over
					if(AvPMenus.PositionInTextField)
						AvPMenus.WidthLeftForText = MENU_RIGHTXEDGE -x;
					else
						AvPMenus.WidthLeftForText = MENU_RIGHTXEDGE -(MENU_CENTREX+MENU_ELEMENT_SPACING);
				}
				else
				{
					RenderText(elementPtr->c.TextPtr,MENU_CENTREX+MENU_ELEMENT_SPACING,y,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
				}
			}
			break;
		}

		case AVPMENU_ELEMENT_TEXTFIELD_SMALLWRAPPED:
		{
			if (elementPtr->a.TextDescription==TEXTSTRING_BLANK)
			{
				RECT area;
				area.left=MENU_LEFTXEDGE;
				area.right=MENU_RIGHTXEDGE;
				area.top=y;
				area.bottom=y+4*HUD_FONT_HEIGHT;
				
				if (AvPMenus.UserEnteringText && e==AvPMenus.CurrentlySelectedElement)
				{
					int output_x,output_y;
					int b = GetSin(CloakingPhase&4095);
					RenderSmallFontString_Wrapped(elementPtr->c.TextPtr,&area,elementPtr->Brightness/2,&output_x,&output_y);
					output_x = RenderSmallMenuText("I",output_x,output_y,MUL_FIXED(b,b),AVPMENUFORMAT_LEFTJUSTIFIED);
					
					//work out how much space was left over
					if(area.bottom - output_y < HUD_FONT_HEIGHT)
					{
						//no space left
						AvPMenus.WidthLeftForText= 0;
					}
					else if(area.bottom - output_y >= 2*HUD_FONT_HEIGHT)
					{
						//at least a whole line left
						AvPMenus.WidthLeftForText= area.right-area.left;
					}
					else
					{
						//on the last line
						AvPMenus.WidthLeftForText= area.right-output_x;
					}
				}
				else
				{
					RenderSmallFontString_Wrapped(elementPtr->c.TextPtr,&area,elementPtr->Brightness,0,0);
				}
			}
			else
			{
				RECT area;
				area.left=MENU_CENTREX+MENU_ELEMENT_SPACING;
				area.right=MENU_RIGHTXEDGE;
				area.top=y;
				area.bottom=y+4*HUD_FONT_HEIGHT;
				
				RenderText(GetTextString(elementPtr->a.TextDescription),MENU_CENTREX-MENU_ELEMENT_SPACING,y+HUD_FONT_HEIGHT,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
			
				if (AvPMenus.UserEnteringText && e==AvPMenus.CurrentlySelectedElement)
				{
					int output_x,output_y;
					int b = GetSin(CloakingPhase&4095);
					RenderSmallFontString_Wrapped(elementPtr->c.TextPtr,&area,elementPtr->Brightness/2,&output_x,&output_y);
					output_x = RenderSmallMenuText("I",output_x,output_y,MUL_FIXED(b,b),AVPMENUFORMAT_LEFTJUSTIFIED);
					
					//work out how much space was left over
					if(area.bottom - output_y < HUD_FONT_HEIGHT)
					{
						//no space left
						AvPMenus.WidthLeftForText= 0;
					}
					else if(area.bottom - output_y >= 2*HUD_FONT_HEIGHT)
					{
						//at least a whole line left
						AvPMenus.WidthLeftForText= area.right-area.left;
					}
					else
					{
						//on the last line
						AvPMenus.WidthLeftForText= area.right-output_x;
					}
				}
				else
				{
					RenderSmallFontString_Wrapped(elementPtr->c.TextPtr,&area,elementPtr->Brightness,0,0);
				}
			}
			break;
		}

		case AVPMENU_ELEMENT_DUMMYNUMBERFIELD:
		case AVPMENU_ELEMENT_NUMBERFIELD:
		{
			{
				static char NumberString[50];
				if(*elementPtr->c.NumberPtr!=0 || elementPtr->NumberFieldZeroString==TEXTSTRING_BLANK)
				{
					sprintf(NumberString,"%d",*elementPtr->c.NumberPtr);
					if(elementPtr->d.NumberFieldUnitsString!=TEXTSTRING_BLANK)
					{
						//add the text for the unit type	
						strcat(NumberString," ");
						strcat(NumberString,GetTextString(elementPtr->d.NumberFieldUnitsString));
					}
				}
				else
				{
					//use the special string to represent 0
					sprintf(NumberString,"%s",GetTextString(elementPtr->NumberFieldZeroString));
				}
				
				if (elementPtr->a.TextDescription==TEXTSTRING_BLANK)
				{
					if (AvPMenus.UserEnteringNumber && e==AvPMenus.CurrentlySelectedElement)
					{
						int b = GetSin(CloakingPhase&4095);
						int x = RenderText(NumberString,MENU_CENTREX,y,elementPtr->Brightness/2,AVPMENUFORMAT_CENTREJUSTIFIED);
						RenderText("I",x,y,MUL_FIXED(b,b),AVPMENUFORMAT_CENTREJUSTIFIED);
					}
					else
					{
						RenderText(NumberString,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
					}
				}
				else
				{
					RenderText(GetTextString(elementPtr->a.TextDescription),MENU_CENTREX-MENU_ELEMENT_SPACING,y,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
			
					if (AvPMenus.UserEnteringNumber && e==AvPMenus.CurrentlySelectedElement)
					{
						int b = GetSin(CloakingPhase&4095);
						int x = RenderText(NumberString,MENU_CENTREX+MENU_ELEMENT_SPACING,y,elementPtr->Brightness/2,AVPMENUFORMAT_LEFTJUSTIFIED);
						RenderText("I",x,y,MUL_FIXED(b,b),AVPMENUFORMAT_LEFTJUSTIFIED);
					}
					else
					{
						RenderText(NumberString,MENU_CENTREX+MENU_ELEMENT_SPACING,y,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
					}
				}
			}
			break;
		}
		
		case AVPMENU_ELEMENT_GOTOMENU_GFX:
		{
			DrawAvPMenuGfx(elementPtr->a.GfxID,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			break;
		}
		case AVPMENU_ELEMENT_LISTCHOICE:
		{
			RenderText(elementPtr->c.TextPtr,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			break;
		}
		case AVPMENU_ELEMENT_LOADMPCONFIG:
		{
			RenderText(elementPtr->c.TextPtr,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			break;
		}
		case AVPMENU_ELEMENT_LOADIPADDRESS:
		{
			RenderText(elementPtr->c.TextPtr,MENU_CENTREX,y,elementPtr->Brightness,AVPMENUFORMAT_CENTREJUSTIFIED);
			break;
		}
		
		case AVPMENU_ELEMENT_TOGGLE:
		{
			break;
		}
		case AVPMENU_ELEMENT_SLIDER:
		{
			int x = MENU_CENTREX+MENU_ELEMENT_SPACING+3;
			x+=(201*(*elementPtr->c.SliderValuePtr))/elementPtr->b.MaxSliderValue;
			RenderText(GetTextString(elementPtr->a.TextDescription),MENU_CENTREX-MENU_ELEMENT_SPACING,y,elementPtr->Brightness,AVPMENUFORMAT_RIGHTJUSTIFIED);
			if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
			{
				D3D_DrawSliderBar(MENU_CENTREX+MENU_ELEMENT_SPACING,y+1,elementPtr->Brightness);
				D3D_DrawSlider(x,y+4,elementPtr->Brightness);
			}
			else
			{
				DrawAvPMenuGfx(AVPMENUGFX_SLIDERBAR,MENU_CENTREX+MENU_ELEMENT_SPACING,y+1,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
				DrawAvPMenuGfx(AVPMENUGFX_SLIDER,x,y+4,elementPtr->Brightness,AVPMENUFORMAT_LEFTJUSTIFIED);
			}


			break;
		}
		case AVPMENU_ELEMENT_VIDEOMODE:
		{
/*			RenderText
			(
				GetTextString(elementPtr->a.TextDescription),
				MENU_CENTREX-MENU_ELEMENT_SPACING,
				y,
				elementPtr->Brightness,
				AVPMENUFORMAT_RIGHTJUSTIFIED
			);	 */
			RenderText
			(
				GetVideoModeDescription2(),
				MENU_CENTREX,
				y,
				elementPtr->Brightness,
				AVPMENUFORMAT_CENTREJUSTIFIED
			);
			RenderText
			(
				GetVideoModeDescription3(),
				MENU_CENTREX,
				y+MENU_FONT_HEIGHT+MENU_ELEMENT_SPACING,
				elementPtr->Brightness,
				AVPMENUFORMAT_CENTREJUSTIFIED
			);
			break;
		}
		case AVPMENU_ELEMENT_KEYCONFIG:
		{
			unsigned char *primaryKey = ((unsigned char*)&PlayerInputPrimaryConfig)+e-2;
			unsigned char *secondaryKey = ((unsigned char*)&PlayerInputSecondaryConfig)+e-2;
			if (e==AvPMenus.CurrentlySelectedElement)
			{
				int x,g;
				if (KeyConfigSelectionColumn)
				{
					x = MENU_RIGHTXEDGE;
				}
				else
				{
					x = MENU_CENTREX;
				}
				if (AvPMenus.UserChangingKeyConfig)
				{
					g = 255;
				}
				else
				{
					g = 128;
				}

				if(AvPMenus.MenusState == MENUSSTATE_INGAMEMENUS)
				{
					Hardware_RenderHighlightRectangle(x-100,y-4,x+4,y+19,0,g,0);
				}
				else
				{
					RenderHighlightRectangle(x-100,y-4,x+4,y+19,0,g,0);
				}
			}
			if (AvPMenus.UserChangingKeyConfig && e==AvPMenus.CurrentlySelectedElement)
			{
				int b = GetSin(CloakingPhase&4095);
				if (AvPMenus.ChangingPrimaryConfig)
				{
					RenderText("_",MENU_CENTREX,y,MUL_FIXED(b,b),AVPMENUFORMAT_RIGHTJUSTIFIED);
					RenderText
					(
						GetDescriptionOfKey(*secondaryKey),
						MENU_RIGHTXEDGE,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_RIGHTJUSTIFIED
					);

				}
				else
				{
					RenderText
					(
						GetDescriptionOfKey(*primaryKey),
						MENU_CENTREX,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_RIGHTJUSTIFIED
					);
					RenderText("_",MENU_RIGHTXEDGE,y,MUL_FIXED(b,b),AVPMENUFORMAT_RIGHTJUSTIFIED);

				}

			}
			else
			{
				if (MultipleAssignments[0][e-2])
				{
					RenderText_Coloured
					(
						GetDescriptionOfKey(*primaryKey),
						MENU_CENTREX,
						y,
						ONE_FIXED,
						AVPMENUFORMAT_RIGHTJUSTIFIED,
						ONE_FIXED,
						ONE_FIXED,
						0
					);
				}
				else
				{
					RenderText
					(
						GetDescriptionOfKey(*primaryKey),
						MENU_CENTREX,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_RIGHTJUSTIFIED
					);
				}
				if (MultipleAssignments[1][e-2])
				{
					RenderText_Coloured
					(
						GetDescriptionOfKey(*secondaryKey),
						MENU_RIGHTXEDGE,
						y,
						ONE_FIXED,
						AVPMENUFORMAT_RIGHTJUSTIFIED,
						ONE_FIXED,
						ONE_FIXED,
						0
					);
				}
				else
				{
					RenderText
					(
						GetDescriptionOfKey(*secondaryKey),
						MENU_RIGHTXEDGE,
						y,
						elementPtr->Brightness,
						AVPMENUFORMAT_RIGHTJUSTIFIED
					);
				}
			}

			RenderText
			(
				GetTextString(elementPtr->a.TextDescription),
				MENU_LEFTXEDGE,
				y,
				elementPtr->Brightness,
				AVPMENUFORMAT_LEFTJUSTIFIED
			);


		}
	}
}

static int HeightOfMenuElement(AVPMENU_ELEMENT *elementPtr)
{
	int h = MENU_ELEMENT_SPACING;

	switch(elementPtr->ElementID)
	{
		default:
		{
			if (AvPMenus.FontToUse==AVPMENU_FONT_BIG)
			{
				h += MENU_FONT_HEIGHT;
			}
			else
			{
				h = HUD_FONT_HEIGHT+4;
			}
			break;
		}
		case AVPMENU_ELEMENT_LOADGAME:
		case AVPMENU_ELEMENT_SAVEGAME:
		{			
			h= HUD_FONT_HEIGHT*2+4;
			break;
		}
		case AVPMENU_ELEMENT_VIDEOMODE:
		{
			h += MENU_FONT_HEIGHT;
			h *=2;
			break;
		}

		case AVPMENU_ELEMENT_TEXTFIELD_SMALLWRAPPED :
		{
			h+=HUD_FONT_HEIGHT*4;
			break;
		}

		#if 0
		case AVPMENU_ELEMENT_STARTALIENGAME:
		case AVPMENU_ELEMENT_STARTMARINEGAME:
		case AVPMENU_ELEMENT_STARTPREDATORGAME:
		#endif
		/*
		case AVPMENU_ELEMENT_DUMMYTEXTFIELD:
		case AVPMENU_ELEMENT_TEXTFIELD:
		{
			h += MENU_FONT_HEIGHT*2;
			break;
		}
		*/
		
		case AVPMENU_ELEMENT_GOTOMENU_GFX:
		{
			h += HeightOfMenuGfx(elementPtr->a.GfxID);
			break;
		}
#if 0
		case AVPMENU_ELEMENT_SLIDER:
		{
			h += HeightOfMenuGfx(AVPMENUGFX_SLIDER);
			break;
		}
#endif
	}
	return h;
}

static char *GetDescriptionOfKey(unsigned char key)
{
	static char KeyDescBuffer[2]="\0\0";
	char *textPtr;
	
	switch (key)
	{
		case KEY_UP:
			textPtr = GetTextString(TEXTSTRING_KEYS_UP);
			break;
		case KEY_DOWN:
			textPtr = GetTextString(TEXTSTRING_KEYS_DOWN);
			break;
		case KEY_LEFT:
			textPtr = GetTextString(TEXTSTRING_KEYS_LEFT);
			break;
		case KEY_RIGHT:
			textPtr = GetTextString(TEXTSTRING_KEYS_RIGHT);
			break;
		case KEY_CR:
			textPtr = GetTextString(TEXTSTRING_KEYS_RETURN);
			break;
		case KEY_TAB:
			textPtr = GetTextString(TEXTSTRING_KEYS_TAB);
			break;
		case KEY_INS:
			textPtr = GetTextString(TEXTSTRING_KEYS_INSERT);
			break;
		case KEY_DEL:
			textPtr = GetTextString(TEXTSTRING_KEYS_DELETE);
			break;
		case KEY_END:
			textPtr = GetTextString(TEXTSTRING_KEYS_END);
			break;
		case KEY_HOME:
			textPtr = GetTextString(TEXTSTRING_KEYS_HOME);
			break;
		case KEY_PAGEUP:
			textPtr = GetTextString(TEXTSTRING_KEYS_PGUP);
			break;
		case KEY_PAGEDOWN:
			textPtr = GetTextString(TEXTSTRING_KEYS_PGDOWN);
			break;
		case KEY_BACKSPACE:
			textPtr = GetTextString(TEXTSTRING_KEYS_BACKSP);
			break;
		case KEY_COMMA:
			textPtr = GetTextString(TEXTSTRING_KEYS_COMMA);
			break;
		case KEY_FSTOP:
			textPtr = GetTextString(TEXTSTRING_KEYS_PERIOD);
			break;
		case KEY_SPACE:
			textPtr = GetTextString(TEXTSTRING_KEYS_SPACE);
			break;
		case KEY_LMOUSE:
			textPtr = GetTextString(TEXTSTRING_KEYS_LMOUSE);
			break;
		case KEY_RMOUSE:
			textPtr = GetTextString(TEXTSTRING_KEYS_RMOUSE);
			break;
		case KEY_MMOUSE:
			textPtr = GetTextString(TEXTSTRING_KEYS_MMOUSE);
			break;
		case KEY_MOUSEBUTTON4:
			textPtr = GetTextString(TEXTSTRING_KEYS_MOUSEBUTTON4);
			break;
		case KEY_MOUSEWHEELUP:
			textPtr = GetTextString(TEXTSTRING_KEYS_MOUSEWHEELUP);
			break;
		case KEY_MOUSEWHEELDOWN:
			textPtr = GetTextString(TEXTSTRING_KEYS_MOUSEWHEELDOWN);
			break;
		case KEY_LEFTALT:
			textPtr = GetTextString(TEXTSTRING_KEYS_LALT);
			break;
		case KEY_RIGHTALT:
			textPtr = GetTextString(TEXTSTRING_KEYS_RALT);
			break;
		case KEY_LEFTCTRL:
			textPtr = GetTextString(TEXTSTRING_KEYS_LCTRL);
			break;
		case KEY_RIGHTCTRL:
			textPtr = GetTextString(TEXTSTRING_KEYS_RCTRL);
			break;
		case KEY_LEFTSHIFT:
			textPtr = GetTextString(TEXTSTRING_KEYS_LSHIFT);
			break;
		case KEY_RIGHTSHIFT:
			textPtr = GetTextString(TEXTSTRING_KEYS_RSHIFT);
			break;
		case KEY_CAPS:
			textPtr = GetTextString(TEXTSTRING_KEYS_CAPS);
			break;
		case KEY_NUMLOCK:
			textPtr = GetTextString(TEXTSTRING_KEYS_NUMLOCK);
			break;
		case KEY_SCROLLOK:
			textPtr = GetTextString(TEXTSTRING_KEYS_SCRLOCK);
			break;
		case KEY_NUMPAD0:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD0);
			break;
		case KEY_NUMPAD1:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD1);
			break;
		case KEY_NUMPAD2:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD2);
			break;
		case KEY_NUMPAD3:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD3);
			break;
		case KEY_NUMPAD4:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD4);
			break;
		case KEY_NUMPAD5:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD5);
			break;
		case KEY_NUMPAD6:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD6);
			break;
		case KEY_NUMPAD7:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD7);
			break;
		case KEY_NUMPAD8:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD8);
			break;
		case KEY_NUMPAD9:
			textPtr = GetTextString(TEXTSTRING_KEYS_PAD9);
			break;
		case KEY_NUMPADSUB:
			textPtr = GetTextString(TEXTSTRING_KEYS_PADSUB);
			break;
		case KEY_NUMPADADD:
			textPtr = GetTextString(TEXTSTRING_KEYS_PADADD);
			break;
		case KEY_NUMPADDEL:
			textPtr = GetTextString(TEXTSTRING_KEYS_PADDEL);
			break;
		case KEY_JOYSTICK_BUTTON_1:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_1);
			break;
		case KEY_JOYSTICK_BUTTON_2:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_2);
			break;
		case KEY_JOYSTICK_BUTTON_3:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_3);
			break;
		case KEY_JOYSTICK_BUTTON_4:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_4);
			break;
		case KEY_JOYSTICK_BUTTON_5:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_5);
			break;
		case KEY_JOYSTICK_BUTTON_6:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_6);
			break;
		case KEY_JOYSTICK_BUTTON_7:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_7);
			break;
		case KEY_JOYSTICK_BUTTON_8:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_8);
			break;
		case KEY_JOYSTICK_BUTTON_9:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_9);
			break;
		case KEY_JOYSTICK_BUTTON_10:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_10);
			break;
		case KEY_JOYSTICK_BUTTON_11:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_11);
			break;
		case KEY_JOYSTICK_BUTTON_12:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_12);
			break;
		case KEY_JOYSTICK_BUTTON_13:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_13);
			break;
		case KEY_JOYSTICK_BUTTON_14:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_14);
			break;
		case KEY_JOYSTICK_BUTTON_15:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_15);
			break;
		case KEY_JOYSTICK_BUTTON_16:
			textPtr = GetTextString(TEXTSTRING_KEYS_JOYSTICKBUTTON_16);
			break;
		case KEY_LBRACKET:	
			textPtr = GetTextString(TEXTSTRING_KEYS_LBRACKET);
			break;
		case KEY_RBRACKET:	
			textPtr = GetTextString(TEXTSTRING_KEYS_RBRACKET);
			break;
		case KEY_SEMICOLON:	
			textPtr = GetTextString(TEXTSTRING_KEYS_SEMICOLON);
			break;
		case KEY_APOSTROPHE:	
			textPtr = GetTextString(TEXTSTRING_KEYS_APOSTROPHE);
			break;
		case KEY_GRAVE:		
			textPtr = GetTextString(TEXTSTRING_KEYS_GRAVE);
			break;
		case KEY_BACKSLASH:	
			textPtr = GetTextString(TEXTSTRING_KEYS_BACKSLASH);
			break;
		case KEY_SLASH:
			textPtr = GetTextString(TEXTSTRING_KEYS_SLASH);
			break;
		case KEY_NUMPADENTER:
			textPtr = GetTextString(TEXTSTRING_KEYS_NUMPADENTER);
			break;
		case KEY_NUMPADDIVIDE:
			textPtr = GetTextString(TEXTSTRING_KEYS_NUMPADDIVIDE);
			break;
		case KEY_NUMPADMULTIPLY:
			textPtr = GetTextString(TEXTSTRING_KEYS_NUMPADMULTIPLY);
			break;
		case KEY_CAPITAL:
			textPtr = GetTextString(TEXTSTRING_KEYS_CAPITAL);
			break;
		case KEY_MINUS:
			textPtr = GetTextString(TEXTSTRING_KEYS_MINUS);
			break;
		case KEY_EQUALS:
			textPtr = GetTextString(TEXTSTRING_KEYS_EQUALS);
			break;
		case KEY_LWIN:
			textPtr = GetTextString(TEXTSTRING_KEYS_LWIN);
			break;
		case KEY_RWIN:
			textPtr = GetTextString(TEXTSTRING_KEYS_RWIN);
			break;
		case KEY_APPS:
			textPtr = GetTextString(TEXTSTRING_KEYS_APPS);
			break;
		case KEY_F1:
			textPtr = GetTextString(TEXTSTRING_KEYS_F1);
			break;
		case KEY_F2:
			textPtr = GetTextString(TEXTSTRING_KEYS_F2);
			break;
		case KEY_F3:								 
			textPtr = GetTextString(TEXTSTRING_KEYS_F3);
			break;
		case KEY_F4:
			textPtr = GetTextString(TEXTSTRING_KEYS_F4);
			break;
		case KEY_F5:
			textPtr = GetTextString(TEXTSTRING_KEYS_F5);
			break;
		case KEY_F6:
			textPtr = GetTextString(TEXTSTRING_KEYS_F6);
			break;
		case KEY_F7:
			textPtr = GetTextString(TEXTSTRING_KEYS_F7);
			break;
		case KEY_F8:
			textPtr = GetTextString(TEXTSTRING_KEYS_F8);
			break;
		case KEY_F9:
			textPtr = GetTextString(TEXTSTRING_KEYS_F9);
			break;
		case KEY_F10:
			textPtr = GetTextString(TEXTSTRING_KEYS_F10);
			break;
		case KEY_F11:
			textPtr = GetTextString(TEXTSTRING_KEYS_F11);
			break;									  
		case KEY_F12:
			textPtr = GetTextString(TEXTSTRING_KEYS_F12);
			break;

		case KEY_A_UMLAUT:
			KeyDescBuffer[0] = 196; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_O_UMLAUT:
			KeyDescBuffer[0] = 214; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_U_UMLAUT:
			KeyDescBuffer[0] = 220; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_BETA:
			KeyDescBuffer[0] = 223; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_PLUS:
			KeyDescBuffer[0] = '+'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_HASH:
			KeyDescBuffer[0] = '#'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_UPSIDEDOWNEXCLAMATION:
			KeyDescBuffer[0] = 161; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_C_CEDILLA:
			KeyDescBuffer[0] = 199; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_N_TILDE:
			KeyDescBuffer[0] = 209; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_RIGHTBRACKET:
			KeyDescBuffer[0] = ')'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_ASTERISK:
			KeyDescBuffer[0] = '*'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_DOLLAR:
			KeyDescBuffer[0] = '$'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_U_GRAVE:
			KeyDescBuffer[0] = 217; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_EXCLAMATION:
			KeyDescBuffer[0] = '!'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_COLON:
			KeyDescBuffer[0] = ':';
			textPtr = KeyDescBuffer;
			break;
		case KEY_DIACRITIC_GRAVE:
			KeyDescBuffer[0] = 96; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_DIACRITIC_ACUTE:
			KeyDescBuffer[0] = 180; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_DIACRITIC_CARET:
			KeyDescBuffer[0] = '^'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_DIACRITIC_UMLAUT:
			KeyDescBuffer[0] = 168; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_ORDINAL:
			KeyDescBuffer[0] = 186; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;
		case KEY_LESSTHAN:
			KeyDescBuffer[0] = '<'; // Windows Latin 1 (ANSI)
			textPtr = KeyDescBuffer;
			break;

		case KEY_VOID:
			KeyDescBuffer[0] = 0;
			textPtr = KeyDescBuffer;
			break;


		default: // alpha-numeric case
		{
			if (key >= KEY_A && key <= KEY_Z)
			{
				KeyDescBuffer[0] = key - KEY_A + 'A';
			}
			else if (key >= KEY_0 && key <= KEY_9)
			{
				KeyDescBuffer[0] = key - KEY_0 + '0';
			}
			else // um, no idea - send blank
			{
				KeyDescBuffer[0] = 0;
			}
			textPtr = KeyDescBuffer;
			break;
		}

	}
	return textPtr;
}

static int OkayToPlayNextEpisode(void)
{
	switch(AvP.PlayerType)
	{
		default:
			GLOBALASSERT(0);
		case I_Marine:
		{
			if (UserProfilePtr->LevelCompleted[AvP.PlayerType][MarineEpisodeToPlay] < AvP.Difficulty+1)
			{
				UserProfilePtr->LevelCompleted[AvP.PlayerType][MarineEpisodeToPlay]=AvP.Difficulty+1;
			}
			SaveUserProfile(UserProfilePtr);
			SetupNewMenu(AVPMENU_MARINELEVELS);
			return 1;
			#if 0
			if (MarineEpisodeToPlay<MAX_NO_OF_BASIC_MARINE_EPISODES-1)
			{
				MarineEpisodeToPlay++;
				SetLevelToLoadForMarine(MarineEpisodeToPlay);
				return 1;
			}
			#endif
			break;
		}
		case I_Predator:
		{
			if (UserProfilePtr->LevelCompleted[AvP.PlayerType][PredatorEpisodeToPlay] < AvP.Difficulty+1)
			{
				UserProfilePtr->LevelCompleted[AvP.PlayerType][PredatorEpisodeToPlay]=AvP.Difficulty+1;
			}
			SaveUserProfile(UserProfilePtr);
			SetupNewMenu(AVPMENU_PREDATORLEVELS);
			return 1;
			#if 0
			if (PredatorEpisodeToPlay<MAX_NO_OF_BASIC_PREDATOR_EPISODES-1)
			{
				PredatorEpisodeToPlay++;
				SetLevelToLoadForPredator(PredatorEpisodeToPlay);
				return 1;
			}
			#endif
			break;
		}
		case I_Alien:
		{
			if (UserProfilePtr->LevelCompleted[AvP.PlayerType][AlienEpisodeToPlay] < AvP.Difficulty+1)
			{
				UserProfilePtr->LevelCompleted[AvP.PlayerType][AlienEpisodeToPlay]=AvP.Difficulty+1;
			}
			SaveUserProfile(UserProfilePtr);
			SetupNewMenu(AVPMENU_ALIENLEVELS);
			return 1;
			#if 0
			if (AlienEpisodeToPlay<MAX_NO_OF_BASIC_ALIEN_EPISODES-1)
			{
				AlienEpisodeToPlay++;
				SetLevelToLoadForAlien(AlienEpisodeToPlay);
				return 1;
			}
			#endif
			break;
		}
	}       
	return 0;
	
}



int NumberOfAvailableLevels(I_PLAYER_TYPE playerID)
{
	int i=0;
	int maximumLevel;
	int numberOfBasicLevels;
	int maxDifficulty=0x7fffffff;

	switch(playerID)
	{
		case I_Marine:
		{
			maximumLevel = MAX_NO_OF_MARINE_EPISODES-1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_MARINE_EPISODES-1;
			break;
		}
		case I_Predator:
		{
			maximumLevel = MAX_NO_OF_PREDATOR_EPISODES-1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_PREDATOR_EPISODES-1;
			break;
		}
		case I_Alien:
		{
			maximumLevel = MAX_NO_OF_ALIEN_EPISODES-1;
			numberOfBasicLevels = MAX_NO_OF_BASIC_ALIEN_EPISODES-1;
			break;
		}
	}

	#if 0 // force all level access
	while (i<maximumLevel && (UserProfilePtr->LevelCompleted[playerID][i]=3))
		i++;
	UserProfilePtr->LevelCompleted[playerID][i]=3;
	#else

	while (i<numberOfBasicLevels && UserProfilePtr->LevelCompleted[playerID][i])
		i++;
		
	MaximumSelectableLevel = i;
	{
		int l;
		for (l=0; l<=numberOfBasicLevels; l++)
		{
			if (UserProfilePtr->LevelCompleted[playerID][l]<maxDifficulty)
			{
				maxDifficulty = UserProfilePtr->LevelCompleted[playerID][l];
			}
		}
		switch(maxDifficulty)
		{
			case AVP_DIFFICULTY_LEVEL_EASY:
			{
				MaximumSelectableLevel = maximumLevel-3;
				i = maximumLevel;
				break;
			} 
			case AVP_DIFFICULTY_LEVEL_MEDIUM:
			{
				MaximumSelectableLevel = maximumLevel-1;
				i = maximumLevel;
				break;
			} 
			case AVP_DIFFICULTY_LEVEL_HARD:
			{
				MaximumSelectableLevel = maximumLevel;
				i = maximumLevel;
				break;
			} 
			default:
				break;
		}
	}
	//while (i<maximumLevel && (UserProfilePtr->LevelCompleted[playerID][i]!=0))
	//	i++;
	#endif

	return i;
}
int LevelMostLikelyToPlay(I_PLAYER_TYPE playerID)
{
	int i=0;
	int maximumLevel;

	switch(playerID)
	{
		case I_Marine:
		{
			maximumLevel = MAX_NO_OF_MARINE_EPISODES-1;
			break;
		}
		case I_Predator:
		{
			maximumLevel = MAX_NO_OF_PREDATOR_EPISODES-1;
			break;
		}
		case I_Alien:
		{
			maximumLevel = MAX_NO_OF_ALIEN_EPISODES-1;
			break;
		}
	}

	while (i<maximumLevel && (UserProfilePtr->LevelCompleted[playerID][i]!=0))
		i++;

	return i;
}

int MaxDifficultyLevelAllowed(I_PLAYER_TYPE playerID, int level)
{
	if (level == 0) return 3;
	else return UserProfilePtr->LevelCompleted[playerID][level-1];
}
void DisplayVideoModeUnavailableScreen(void)
{
	do
	{
		CheckForWindowsMessages();
		DrawMainMenusBackdrop();
		ReadUserInput();
		{
			RECT area;
			//draw the attached string at the bottom of the screen

			area.left=MENU_LEFTXEDGE;
			area.right=MENU_RIGHTXEDGE;
			area.top=0;
			area.bottom=ScreenDescriptorBlock.SDB_Height;

			RenderSmallFontString_Wrapped(GetTextString(TEXTSTRING_NOTENOUGHMEMORY),&area,BRIGHTNESS_OF_HIGHLIGHTED_ELEMENT,0,0);
			
		}
				
		ShowMenuFrameRate();

		FlipBuffers();
		FrameCounterHandler();
		PlayMenuMusic();
	}
	while(!DebouncedGotAnyKey);
}


void CheckForCredits(void)
{
#if 0
	FILE *fp = OpenGameFile("credits.txt", FILEMODE_READONLY, FILETYPE_PERM);
	
	if (!fp)
	{
		char message[100];
		sprintf(message,"Unable to access credits.txt\n");
		MessageBox(NULL,message,"AvP Error",MB_OK+MB_SYSTEMMODAL);
		exit(0x111);
		return;
	}
	else
	{
		fclose(fp);
	}
#endif	
}

void DoCredits(void)
{
	int position = 300*2048;
	BOOL FinishedCredits = FALSE;
	
	char *creditsBufferPtr = LoadTextFile("credits.txt");
	char *creditsPtr;
	
	if (!creditsBufferPtr) return;

	if (!strncmp (creditsBufferPtr, "REBCRIF1", 8))
	{
		creditsPtr = (char*)HuffmanDecompress((HuffmanPackage*)(creditsBufferPtr)); 		
		DeallocateMem(creditsBufferPtr);
		creditsBufferPtr=creditsPtr;
	}
	else
	{
		creditsPtr = creditsBufferPtr;
	}


	do
	{
		CheckForWindowsMessages();
		DrawMainMenusBackdrop();
		ReadUserInput();
	
		FinishedCredits = !RollCreditsText(position,creditsPtr+4);
		ShowMenuFrameRate();

		FlipBuffers();
		FrameCounterHandler();
		PlayMenuMusic();
	   	position -= RealFrameTime;
	}
	while(!DebouncedGotAnyKey && !FinishedCredits);

	UnloadTextFile("credits.txt",creditsPtr);
}

BOOL RollCreditsText(int position, unsigned char *textPtr)
{
	int y=0;

	while(*textPtr!='#')
	{
		char buffer1[96];
		char buffer2[96];
		int i;
		int centredText,splitText;
		
										
		{
			int yy = 60+y+position/2048;
			
			if (yy >= 60 && yy <= 400)
			{
				int b;
				{
					if (*textPtr=='}')
					{
						textPtr++;
						centredText = 1;
					}
					else
					{
						centredText = 0;
					}

					for (i=0; i<80 && (*textPtr!='\r')&&(*textPtr!='\n')&&(*textPtr!='\0')&&(*textPtr!='|'); i++)
					{
						buffer1[i] = *textPtr++;
		//				if (buffer1[i]>='a' && buffer1[i]<='z') buffer1[i] += 'A' - 'a';
					}
					buffer1[i]=0;

					if (*textPtr=='|')
					{
						textPtr++;
						for (i=0; i<80 && (*textPtr!='\r')&&(*textPtr!='\n')&&(*textPtr!='\0'); i++)
						{
							buffer2[i] = *textPtr++;
						}
						buffer2[i]=0;
						splitText = 1;
					}
					else
					{
						splitText = 0;
					}

					textPtr++;
					while(*textPtr=='\n') textPtr++;

				}
				
				if (yy<92)
				{
					b = (yy-60)*2048;
				}
				else if (yy>400-32)
				{
					b = (400-yy)*2048;
				}
				else 
				{
					b = ONE_FIXED;
				}

				if (splitText)
				{
					RenderSmallMenuText(buffer1,MENU_CENTREX-MENU_ELEMENT_SPACING,yy,b,AVPMENUFORMAT_RIGHTJUSTIFIED);
					RenderSmallMenuText(buffer2,MENU_CENTREX+MENU_ELEMENT_SPACING,yy,b,AVPMENUFORMAT_LEFTJUSTIFIED);
				}
				else
				{
					if (centredText)
					{
						RenderSmallMenuText(buffer1,MENU_CENTREX,yy,b,AVPMENUFORMAT_CENTREJUSTIFIED);
					}
					else
					{
						RenderSmallMenuText(buffer1,MENU_LEFTXEDGE,yy,b,AVPMENUFORMAT_LEFTJUSTIFIED);
					}
				}

			}
			else
			{
				while(*textPtr++!='\n');
			}
		}
		y+=15;
	}

	if(y+(position/2048)<0)
	{
		//finished
		return FALSE;
	}
	return TRUE;
}
/* KJL 17:50:06 24/06/98 - setup video mode for Menus */

extern int VideoModeTypeScreen;
extern int RasterisationRequestMode;
extern int VideoMode;
extern int WindowMode;
extern int WindowRequestMode;

extern int DXMemoryMode;
extern int DXMemoryRequestMode;

extern void SelectMenuDisplayMode(void)
{
    MinimizeAllImages();
	TimeStampedMessage("after MinimizeAllImages");
	MinimizeAllDDGraphics();
	TimeStampedMessage("after MinimizeAllDDGraphics");

    ReleaseDirect3DNotDDOrImages();
	TimeStampedMessage("after ReleaseDirect3DNotDDOrImages");

    finiObjectsExceptDD();
	TimeStampedMessage("after finiObjectsExceptDD");

  	finiObjects();
	SelectDirectDrawObject(NULL);

	DXMemoryRequestMode = RequestSystemMemoryAlways;
	DXMemoryMode = SystemMemoryPreferred;

	RasterisationRequestMode = RequestSoftwareRasterisation;
	VideoMode = VideoMode_DX_640x480x15;
	VideoModeTypeScreen = VideoModeType_15;
	WindowMode = WindowRequestMode;
	ScreenDescriptorBlock.SDB_Width = 640;
	ScreenDescriptorBlock.SDB_Height = 480;
	ScreenDescriptorBlock.SDB_ScreenDepth = VideoModeType_15;

	#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
	ChangeDirectDrawObject();
	#endif
	TimeStampedMessage("after ChangeDirectDrawObject");

	GenerateDirectDrawSurface();
	TimeStampedMessage("after GenerateDirectDrawSurface");

	{
		extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
		Global_VDB_Ptr = 0;
	}
}






















#if 0
	 


struct STARDESC
{       
	int X;
	int Y;
	int Chasing;
	int Fleeing;
};
#define NUMBER_OF_STARS 128

struct STARDESC Star[NUMBER_OF_STARS];

static void DrawStar(int x, int y)
{
	extern unsigned char *ScreenBuffer;
	extern long BackBufferPitch;

	int xi,xf,xfm,yi,yf,yfm;

	xi=x/65536;
	yi=y/65536;
	xf = x - xi*65536;
	yf = y - yi*65536;
	xfm = ONE_FIXED - xf;
	yfm = ONE_FIXED - yf;

	*(unsigned short*)(ScreenBuffer + ((xi+0)*2 + (yi+0)*BackBufferPitch)) = (unsigned short)WhiteOfBrightness(MUL_FIXED(xfm,yfm));
	*(unsigned short*)(ScreenBuffer + ((xi+0)*2 + (yi+1)*BackBufferPitch)) = (unsigned short)WhiteOfBrightness(MUL_FIXED(xfm,yf));
	*(unsigned short*)(ScreenBuffer + ((xi+1)*2 + (yi+0)*BackBufferPitch)) = (unsigned short)WhiteOfBrightness(MUL_FIXED(xf,yfm));
	*(unsigned short*)(ScreenBuffer + ((xi+1)*2 + (yi+1)*BackBufferPitch)) = (unsigned short)WhiteOfBrightness(MUL_FIXED(xf,yf));

}
#endif

static void InitMainMenusBackdrop(void)
{
	#if 0
	int i;
	for(i=0; i<NUMBER_OF_STARS; i++)
	{
		Star[i].X = (FastRandom()&65535)*640;
		Star[i].Y = (FastRandom()&65535)*480;
		Star[i].Chasing = (FastRandom()&127);
		Star[i].Fleeing = (FastRandom()&127);
	}
	#endif
	#if 0
	{
		int x,y;
		for (x=0; x<128; x++)
		for (y=0; y<128; y++)
		CloudTable[x][y]=(FastRandom()&32767)+32768;
	}
	#endif
	StartMenuBackgroundBink();
}



extern void DrawMainMenusBackdrop(void)
{
	#if 1
	if (!PlayMenuBackgroundBink())
	{
		DrawAvPMenuGfx(AVPMENUGFX_BACKDROP,0,0,ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
	}
	else
	{
		extern unsigned char *ScreenBuffer;
		unsigned int *screenPtr = (unsigned int*)ScreenBuffer;
		int i;	  

		i = ScreenDescriptorBlock.SDB_Width * 60 /2;
		do
		{
			*screenPtr++=0; 
		}
		while(--i);

		screenPtr+=ScreenDescriptorBlock.SDB_Width * 360/2;

		i = ScreenDescriptorBlock.SDB_Width * 60 /2;
		do
		{
			*screenPtr++=0; 
		}
		while(--i);
	}



	#else
	extern DDPIXELFORMAT DisplayPixelFormat;
	extern unsigned char *ScreenBuffer;
	extern long BackBufferPitch;
//      LockSurfaceAndGetBufferPointer();
	{
		unsigned int *screenPtr;
		int i;	  
		int blue = (DisplayPixelFormat.dwBBitMask/4) & DisplayPixelFormat.dwBBitMask;				      
		
		blue = blue | (blue<<16);
		screenPtr = (unsigned int *)ScreenBuffer;
		i = ScreenDescriptorBlock.SDB_Width * 60 /2;
		do
		{
			*screenPtr++=0; 
		}
		while(--i);
		#if 1
		#if 1
		i = ScreenDescriptorBlock.SDB_Width * 360/2;
		do
		{
			*screenPtr=0;   
			screenPtr++;
		}
		while(--i);
		#else
		{
		unsigned short *s=(unsigned short*)screenPtr;
		i = ScreenDescriptorBlock.SDB_Width * 360;
		do
		{
			unsigned short a;
			a =  MUL_FIXED(((*s)&DisplayPixelFormat.dwRBitMask),50000) & DisplayPixelFormat.dwRBitMask;
			a |= MUL_FIXED(((*s)&DisplayPixelFormat.dwGBitMask),50000) & DisplayPixelFormat.dwGBitMask;
			a |= MUL_FIXED(((*s)&DisplayPixelFormat.dwBBitMask),50000) & DisplayPixelFormat.dwBBitMask;
			*s++=a;
		}
		while(--i);
		}
		#endif
		#else

		#if 0
		if (AvPMenus.CurrentMenu == AVPMENU_MARINELEVELS)
		{
			DrawAvPMenuGfx(AVPMENUGFX_MARINE_EPISODE1+MarineEpisodeToPlay,0,60,ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		else if (AvPMenus.CurrentMenu == AVPMENU_PREDATORLEVELS)
		{
			DrawAvPMenuGfx(AVPMENUGFX_PREDATOR_EPISODE1+PredatorEpisodeToPlay,0,60,ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		else if (AvPMenus.CurrentMenu == AVPMENU_ALIENLEVELS)
		{
			DrawAvPMenuGfx(AVPMENUGFX_ALIEN_EPISODE1+AlienEpisodeToPlay,0,60,ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		else
		#endif
		{
			DrawAvPMenuGfx(AVPMENUGFX_BACKDROP,0,0,ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		screenPtr+=ScreenDescriptorBlock.SDB_Width * 360/2;
		#endif
		i = ScreenDescriptorBlock.SDB_Width * 60 /2;
		do
		{
			*screenPtr++=0; 
		}
		while(--i);

	}
	#if 0
	{
		int i;

		for(i=0; i<NUMBER_OF_STARS; i++)
		{
			int x,y;

			x = Star[i].X;
			y = Star[i].Y;
			if ( (x<ONE_FIXED || x>638*ONE_FIXED)
			   ||(y<61*ONE_FIXED || y>418*ONE_FIXED))
			{
				Star[i].X = (FastRandom()&65535)*640;
				Star[i].Y = ((FastRandom()&65535)*360)+60;
				Star[i].Chasing = (FastRandom()&127);
				Star[i].Fleeing = (FastRandom()&127);
			}
			else
			{
				int s = Star[i].Chasing;
				int f = Star[i].Fleeing;
				DrawStar(x,y);									      
//			      Star[i].X -= MUL_FIXED(Star[i].X - 320*ONE_FIXED,RealFrameTime*4);
 //			     Star[i].Y -= MUL_FIXED(Star[i].Y - 240*ONE_FIXED,RealFrameTime*4);

				if ((Star[i].X/131072 == Star[f].X/131072)
				  &&(Star[i].Y/131072 == Star[f].Y/131072))
				{
					Star[i].X = (FastRandom()&65535)*640;
					Star[i].Y = ((FastRandom()&65535)*360)+60;
				}
				else
				{
					Star[i].X -= MUL_FIXED(Star[i].X - Star[s].X,RealFrameTime*4);
					Star[i].Y -= MUL_FIXED(Star[i].Y - Star[s].Y,RealFrameTime*4);
					Star[i].X += MUL_FIXED(Star[i].X - Star[f].X,RealFrameTime);
					Star[i].Y += MUL_FIXED(Star[i].Y - Star[f].Y,RealFrameTime);
				}

				
			}		
		}

	}
	#endif
	//UnlockSurface();
	#endif
}

int WhiteOfBrightness(int brightness)
{
	fprintf(stderr, "WhiteOfBrightness(%d)\n", brightness);
	
	return 0;
#if 0
	extern DDPIXELFORMAT DisplayPixelFormat;
	int a;

	a = MUL_FIXED(DisplayPixelFormat.dwRBitMask,brightness) & DisplayPixelFormat.dwRBitMask;
	a |= MUL_FIXED(DisplayPixelFormat.dwGBitMask,brightness) & DisplayPixelFormat.dwGBitMask;
	a |= MUL_FIXED(DisplayPixelFormat.dwBBitMask,brightness) & DisplayPixelFormat.dwBBitMask;

	return a;
#endif	
}

void RenderPixel(int x,int y,int r,int g,int b)
{
	fprintf(stderr, "RenderPixel(%d, %d, %d, %d, %d)\n", x, y, r, g, b);
#if 0
	extern DDPIXELFORMAT DisplayPixelFormat;
	extern unsigned char *ScreenBuffer;
	extern long BackBufferPitch;

	unsigned short colour;

	colour  = MUL_FIXED(DisplayPixelFormat.dwRBitMask,r<<8) & DisplayPixelFormat.dwRBitMask;
	colour |= MUL_FIXED(DisplayPixelFormat.dwGBitMask,g<<8) & DisplayPixelFormat.dwGBitMask;
	colour |= MUL_FIXED(DisplayPixelFormat.dwBBitMask,b<<8) & DisplayPixelFormat.dwBBitMask;
	
	
	*(unsigned short*) (ScreenBuffer + (x)*2 + (y)*BackBufferPitch)  = colour;
#endif	
}
#if 0
void BezierCurve(void)
{
	int i;
	extern unsigned char *ScreenBuffer;
	extern long BackBufferPitch;
	static b=0;
	for (i=0; i<=255; i++)
	{
		int u = ((i*65536)/255);
		int m = MUL_FIXED(u,u);
		int l = MUL_FIXED(2*u,ONE_FIXED-u);

		int a;
		
		a = MUL_FIXED(255,m)+MUL_FIXED(b,l);
		if (a<0) a=0;
		if (a>255) a=255;

		m = MUL_FIXED(a*256,a*256);
		l = MUL_FIXED(2*a*256,ONE_FIXED-a*256);

		a;
		
		a = MUL_FIXED(255,m)+MUL_FIXED(b,l);
		if (a<0) a=0;
		if (a>255) a=255;

   		*(unsigned short*)(ScreenBuffer + ((i+0)*2 + (400-a)*BackBufferPitch)) = 0xffff;
   		//*(unsigned short*)(ScreenBuffer + ((i+0)*2 + (400-i)*BackBufferPitch)) = 0xff;

   		*(unsigned short*)(ScreenBuffer + ((i+0)*2 + (400-i)*BackBufferPitch)) = 0xffff;
   		*(unsigned short*)(ScreenBuffer + ((i+50)*2 + (400-i)*BackBufferPitch)) = 0xffff;
   		*(unsigned short*)(ScreenBuffer + ((i+100)*2 + (400-i)*BackBufferPitch)) = 0xffff;


	}
//		PlayWithGammaSettings(b);
	b++;
	if (b>=255) b=0;
}			   
#endif



static void UpdateMultiplayerConfigurationMenu()
{
	/*update the displayed levels according to the cuurent game type*/
	
	AVPMENU_ELEMENT *elementPtr = AvPMenus.MenuElements;
	
	//search for the level name element
	do
	{
		GLOBALASSERT(elementPtr->ElementID!=AVPMENU_ELEMENT_ENDOFMENU);
		elementPtr++;

	}while(elementPtr->a.TextDescription!=TEXTSTRING_MULTIPLAYER_ENVIRONMENT);

	
	if(netGameData.skirmishMode)
	{
		//force 'cooperative' game if using skirmish mode
		netGameData.gameType=NGT_Coop;
	}
	
	if(netGameData.gameType!=NGT_Coop)
	{
		elementPtr->b.MaxSliderValue = NumMultiplayerLevels-1;
		elementPtr->d.TextSliderStringPointer = MultiplayerLevelNames;

		//make sure the level number is within bounds
		netGameData.levelNumber%=NumMultiplayerLevels;
	}
	else
	{
		elementPtr->b.MaxSliderValue = NumCoopLevels-1;
		elementPtr->d.TextSliderStringPointer = CoopLevelNames;

		//make sure the level number is within bounds
		netGameData.levelNumber%=NumCoopLevels;
	}


	//see if selected element is the gamestyle element
	elementPtr = &AvPMenus.MenuElements[AvPMenus.CurrentlySelectedElement];

	if(elementPtr->a.TextDescription==TEXTSTRING_MULTIPLAYER_GAMESTYLE)
	{
		//change the helpstring according to the game style
		switch(netGameData.gameType)
		{
			case NGT_Individual :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_DEATHMATCH;
				break;

			case NGT_CoopDeathmatch :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_DEATHMATCHTEAM;
				break;
			
			case NGT_LastManStanding :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_LASTMANSTANDING;
				break;
			
			case NGT_PredatorTag :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_PREDATORTAG;
				break;
			
			case NGT_Coop :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_COOPERATIVE;
				break;

			case NGT_AlienTag :
				elementPtr->HelpString=TEXTSTRING_MPHELP_GAMESTYLE_ALIENTAG;
				break;
		}
	}

	//also need to fill in customLevelName
	strcpy(netGameData.customLevelName,GetCustomMultiplayerLevelName(netGameData.levelNumber,netGameData.gameType));

}

static void TestValidityOfCheatMenu(void)
{
	CheatMode_GetNextAllowedMode(AvPMenus.MenuElements[0].c.SliderValuePtr,TRUE);
	CheatMode_GetNextAllowedSpecies(AvPMenus.MenuElements[1].c.SliderValuePtr,TRUE);
	CheatMode_GetNextAllowedEnvironment(AvPMenus.MenuElements[2].c.SliderValuePtr,TRUE);
}

static unsigned char *BriefingTextString[5];
static unsigned char BlankLine[]="";

void SetBriefingTextForEpisode(int episode, I_PLAYER_TYPE playerID)
{	
	int i,s;

	switch(playerID)
	{
		default:
		case I_Marine:
		{
			s = TEXTSTRING_MARINELEVELSBRIEFING_1;
			break;
		}
		case I_Predator:
		{
			s = TEXTSTRING_PREDATORLEVELSBRIEFING_1;
			break;
		}
		case I_Alien:
		{
			s = TEXTSTRING_ALIENLEVELSBRIEFING_1;
			break;
		}
	}
	
	for(i=0; i<5; i++)
	{
		BriefingTextString[i] = GetTextString(s+i+ episode*5);
	}
}

static char MultiplayerBriefing[3][100];

static void AddMultiplayerBriefingString(const char* text)
{
	int shortest = 0;
	int shortest_length =1000;
	int i;

	for(i=0;i<3;i++)
	{
		int length = strlen(MultiplayerBriefing[i]);
		if(length<shortest_length)
		{
			shortest = i;
			shortest_length = length;
		}
	}

	if(shortest_length + 3 + strlen(text)>=100) return;

	if(shortest_length>0)
	{
		strcat(MultiplayerBriefing[shortest]," , ");
	}
	strcat(MultiplayerBriefing[shortest],text);
}

static BOOL IsFlamerInLevel(int level)
{
	if(level == AVP_ENVIRONMENT_SUBWAY_MP ||
	   level == AVP_ENVIRONMENT_SUBWAY_COOP)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL IsMinigunInLevel(int level)
{
	if(level == AVP_ENVIRONMENT_SEWER)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL IsSadarInLevel(int level)
{
	if(level == AVP_ENVIRONMENT_MASSACRE ||
	   level == AVP_ENVIRONMENT_MEATFACTORY_MP ||
	   level == AVP_ENVIRONMENT_MEATFACTORY_COOP ||
	   level == AVP_ENVIRONMENT_HADLEYSHOPE_MP ||
	   level == AVP_ENVIRONMENT_HADLEYSHOPE_COOP ||
	   level == AVP_ENVIRONMENT_HIVE ||
	   level == AVP_ENVIRONMENT_HIVE_COOP ||
	   level == AVP_ENVIRONMENT_KENS_COOP ||
	   level == AVP_ENVIRONMENT_LEADWORKS_MP ||
	   level == AVP_ENVIRONMENT_LEADWORKS_COOP)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL IsSkeeterInLevel(int level)
{
	if(level == AVP_ENVIRONMENT_LEADWORKS_MP ||
	   level == AVP_ENVIRONMENT_LEADWORKS_COOP)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL IsSmartgunInLevel(int level)
{
	if(level == AVP_ENVIRONMENT_NOSTROMO_MP) return FALSE;
	return TRUE;
}


static void SetBriefingTextForMultiplayer()
{
	int level = NumberForCurrentLevel();

	int num_not_available = 0;

	if(netGameData.customLevelName[0] != 0)
	{
		//custom level
		BriefingTextString[0] = netGameData.customLevelName;
		return;
	}
	
	MultiplayerBriefing[0][0] = 0;
	MultiplayerBriefing[1][0] = 0;
	MultiplayerBriefing[2][0] = 0;

	if(!netGameData.allowSmartgun || !IsSmartgunInLevel(level))
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_SMARTGUN));
		num_not_available++;
	}
	if(!netGameData.allowFlamer || !IsFlamerInLevel(level))
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_FLAMETHROWER));
		num_not_available++;
	}
	if(!netGameData.allowSadar || !IsSadarInLevel(level))
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_SADAR));
		num_not_available++;
	}
	if(!netGameData.allowGrenadeLauncher)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_GRENADELAUNCHER));
		num_not_available++;
	}
	if(!netGameData.allowMinigun || !IsMinigunInLevel(level))
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_MINIGUN));
		num_not_available++;
	}
	if(!netGameData.allowSmartDisc || !IsSkeeterInLevel(level))
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_SKEETER));
		num_not_available++;
	}
	if(!netGameData.allowPistols)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_MARINE_PISTOL));
		num_not_available++;
	}
	if(!netGameData.allowDisc)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_DISC));
		num_not_available++;
	}
	if(!netGameData.allowPistol)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_BRIEFING_PREDATOR_PISTOL));
		num_not_available++;
	}
	if(!netGameData.allowPlasmaCaster)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_SHOULDERCANNON));
		num_not_available++;
	}
	if(!netGameData.allowSpeargun)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_RIFLE));
		num_not_available++;
	}
	if(!netGameData.allowMedicomp)
	{
		AddMultiplayerBriefingString(GetTextString(TEXTSTRING_INGAME_MEDICOMP));
		num_not_available++;
	}

	if(netGameData.gameType!=NGT_Coop)
	{
		BriefingTextString[0] = GetTextString(netGameData.levelNumber + TEXTSTRING_MULTIPLAYERLEVELS_1);
		
	}
	else
	{
		BriefingTextString[0] = GetTextString(netGameData.levelNumber + TEXTSTRING_COOPLEVEL_1);
	}
	
	if(num_not_available)
	{
		BriefingTextString[1] = GetTextString(TEXTSTRING_BRIEFING_UNAVAILABLE_WEAPONS);
		BriefingTextString[2] = MultiplayerBriefing[0];
		BriefingTextString[3] = MultiplayerBriefing[1];
		BriefingTextString[4] = MultiplayerBriefing[2];
	}
}

void SetBriefingTextToBlank(void)
{	
	int i;

	for(i=0; i<5; i++)
	{
		BriefingTextString[i] = BlankLine;
	}
}

void RenderBriefingText(int centreY, int brightness)
{
	int lengthOfLongestLine=-1;
	int x,y,i;

	for(i=0; i<5; i++)
	{
		int length = 0;
		{
			char *ptr = BriefingTextString[i];

			while(*ptr)
			{
				length+=AAFontWidths[(unsigned char)(*ptr++)];
			}
		}
		
		if (lengthOfLongestLine < length)
		{
			lengthOfLongestLine = length;
		}
	}

	x = (ScreenDescriptorBlock.SDB_Width-lengthOfLongestLine)/2;
	y = centreY - 3*HUD_FONT_HEIGHT;
	for(i=0; i<5; i++)
	{
		if (AvPMenus.MenusState != MENUSSTATE_MAINMENUS)
		{
			Hardware_RenderSmallMenuText(BriefingTextString[i], x, y, brightness, AVPMENUFORMAT_LEFTJUSTIFIED/*,MENU_CENTREY-60-100,MENU_CENTREY-60+180*/);
		}
		else
		{
			RenderSmallMenuText(BriefingTextString[i], x, y, brightness, AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		if (i) y+=HUD_FONT_HEIGHT;
		else y+=HUD_FONT_HEIGHT*2;
	}
}


void CheckForKeysWithMultipleAssignments(void)
{
	unsigned char *configPtr[2];
	int column,row;

	configPtr[0] = (unsigned char*)&PlayerInputPrimaryConfig;
	configPtr[1] = (unsigned char*)&PlayerInputSecondaryConfig;

	for (column=0; column<=1; column++)
	{
		for (row=0; row<32; row++)
		{
			int innerColumn,innerRow;

			MultipleAssignments[column][row] = 0;

			for (innerColumn=0; innerColumn<=1; innerColumn++)
			for (innerRow=0; innerRow<32; innerRow++)
			{
				if (innerRow==row) continue;
				// && innerColumn==column) continue;
				if ( (configPtr[column])[row]==(configPtr[innerColumn])[innerRow] )
				{
					MultipleAssignments[column][row] = 1;
				}
			}
		}
	}
}

void HandleCheatModeFeatures(void)
{
	switch(CheatMode_Active)
	{
		case CHEATMODE_WARPSPEED:
		{
			TimeScale = 2*ONE_FIXED;
			AvP.Difficulty = 0;
			break;
		}
		case CHEATMODE_LANDOFTHEGIANTS:
		{
			TimeScale =ONE_FIXED/2;
			break;
		}
		case CHEATMODE_IMPOSSIBLEMISSION:
		{
			AvP.Difficulty = 3;
			break;
		}
		case CHEATMODE_UNDERWATER:
		{
			TimeScale = (ONE_FIXED*7)/10;
			break;
		}
		default:
			break;
	}
}

void ShowMenuFrameRate(void)
{
#if 0
	extern NormalFrameTime;
	char buffer[8];
	
	sprintf(buffer,"%d fps",65536/NormalFrameTime);
	RenderSmallMenuText(buffer,20,20,ONE_FIXED,	AVPMENUFORMAT_LEFTJUSTIFIED);
#endif
}

#define MAX_ITEMS_IN_KEYBOARDENTRYQUEUE 8
static char KeyboardEntryQueue[MAX_ITEMS_IN_KEYBOARDENTRYQUEUE];
static int NumberOfItemsInKeyboardEntryQueue;
static int KeyboardEntryQueue_ProcessingIndex;

void KeyboardEntryQueue_Add(char c)
{
	if (c<32) return;

	if (NumberOfItemsInKeyboardEntryQueue<MAX_ITEMS_IN_KEYBOARDENTRYQUEUE)
	{
		KeyboardEntryQueue[NumberOfItemsInKeyboardEntryQueue++] = c;
	}
}

static void KeyboardEntryQueue_Clear(void)
{
	int i;
	for (i=0; i<MAX_ITEMS_IN_KEYBOARDENTRYQUEUE; i++)
	{
		KeyboardEntryQueue[i] = 0;
	}
	NumberOfItemsInKeyboardEntryQueue = 0;
}
  
static void KeyboardEntryQueue_StartProcessing(void)
{
	KeyboardEntryQueue_ProcessingIndex = 0;
}

static char KeyboardEntryQueue_ProcessCharacter(void)
{
	if (KeyboardEntryQueue_ProcessingIndex==MAX_ITEMS_IN_KEYBOARDENTRYQUEUE) return 0;

	return KeyboardEntryQueue[KeyboardEntryQueue_ProcessingIndex++];
}



/*------------------------------------**
** Loading and saving main level info **
**------------------------------------*/


extern int AlienEpisodeToPlay;
extern int MarineEpisodeToPlay;
extern int PredatorEpisodeToPlay;

void SaveLevelHeader()
{
	LEVEL_SAVE_BLOCK* block = (LEVEL_SAVE_BLOCK*) GetPointerForSaveBlock(sizeof(LEVEL_SAVE_BLOCK));

	//fill in the header
	block->header.type = SaveBlock_MainHeader;
	block->header.size = sizeof(*block);

	//fill in the main block
	strncpy(block->AvP_Save_String,"AVPSAVE0",8);

	block->Species = AvP.PlayerType;

	switch(block->Species)
	{
		case I_Marine :
			block->Episode = MarineEpisodeToPlay;
			break;

		case I_Alien :
			block->Episode = AlienEpisodeToPlay;
			break;

		case I_Predator :
			block->Episode = PredatorEpisodeToPlay;
			break;
	}

	block->ElapsedTime_Hours =(unsigned char) AvP.ElapsedHours;
	block->ElapsedTime_Minutes =(unsigned char) AvP.ElapsedMinutes;
	block->ElapsedTime_Seconds = AvP.ElapsedSeconds;

	block->Difficulty = AvP.Difficulty;
	block->NumberOfSavesLeft = (unsigned char) NumberOfSavesLeft;
}

void LoadLevelHeader(SAVE_BLOCK_HEADER* header)
{
	LEVEL_SAVE_BLOCK* block =(LEVEL_SAVE_BLOCK*) header;

	if(block->header.size!=sizeof(*block)) return;

	AvP.ElapsedHours = block->ElapsedTime_Hours;
	AvP.ElapsedMinutes = block->ElapsedTime_Minutes;
	AvP.ElapsedSeconds = block->ElapsedTime_Seconds;
}

static void GetHeaderInfoForSaveSlot(SAVE_SLOT_HEADER* save_slot, const GameDirectoryFile *gdf)
{
	LEVEL_SAVE_BLOCK block;
	unsigned int file_size;
	unsigned char filename[100];
	FILE *file;

	save_slot->SlotUsed = 0;

	sprintf(filename, "%s%s", USER_PROFILES_PATH, gdf->filename);
	file = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_CONFIG);

	if (file==NULL)
	{
		//failed to load (probably doesn't exist)
		return;
	}
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	if(file_size < sizeof(LEVEL_SAVE_BLOCK))
	{
		//obviously not much of a save file then...
		fclose(file);
		return;

	}

	save_slot->TimeStamp = gdf->timestamp;
   	
	//load the level header
	fread(&block, sizeof(block), 1, file);
	fclose(file);

	//a few checks
	if(block.header.type != SaveBlock_MainHeader ||
	   block.header.size != sizeof(block) ||
	   strncmp(block.AvP_Save_String,"AVPSAVE0",8))
	{
		//no good.
		return;
	}	

	//appears to be a reasonable file
	save_slot->SlotUsed = TRUE;

	//copy stuff from the block

	save_slot->Species = block.Species;
	save_slot->Episode = block.Episode;
	save_slot->ElapsedTime_Hours = block.ElapsedTime_Hours;
	save_slot->ElapsedTime_Minutes = block.ElapsedTime_Minutes;
	save_slot->ElapsedTime_Seconds = (block.ElapsedTime_Seconds >> 16);
	save_slot->Difficulty = block.Difficulty;
	save_slot->SavesLeft = block.NumberOfSavesLeft;
}

void ScanSaveSlots(void)
{
	unsigned char pattern[100], *ptr;
	int i;
	void *gd;
	GameDirectoryFile *gdf;
	
	sprintf(pattern, "%s_?.sav", UserProfilePtr->Name);
	gd = OpenGameDirectory(USER_PROFILES_PATH, pattern, FILETYPE_CONFIG);
	if (gd == NULL)
		return;

	while ((gdf = ScanGameDirectory(gd)) != NULL) {
		if ((gdf->attr & FILEATTR_DIRECTORY) != 0)
			continue;
		if ((gdf->attr & FILEATTR_READABLE) == 0)
			continue;

		ptr = strrchr(gdf->filename, '.');
		if (ptr == NULL)
			continue;
		ptr--;

		i = *ptr - '1';
		if (i >= 0 && (i < NUMBER_OF_SAVE_SLOTS)) {
			GetHeaderInfoForSaveSlot(&SaveGameSlot[i], gdf);
		}		
	}
}

extern void GetFilenameForSaveSlot(int i, unsigned char *filenamePtr)
{
	sprintf(filenamePtr,"%s%s_%d.sav",USER_PROFILES_PATH,UserProfilePtr->Name,i+1);
}

static void CheckForLoadGame()
{
	if(LoadGameRequest >=0 && LoadGameRequest<NUMBER_OF_SAVE_SLOTS)
	{
		SAVE_SLOT_HEADER* save_slot = &SaveGameSlot[LoadGameRequest];	
		if(save_slot->SlotUsed)
		{
			AvP.PlayerType = save_slot->Species;
			AvP.Difficulty = save_slot->Difficulty;

			switch(AvP.PlayerType)
			{
				case I_Marine :
					MarineEpisodeToPlay = save_slot->Episode;
					SetLevelToLoadForMarine(MarineEpisodeToPlay);
					break;

				case I_Alien :
					AlienEpisodeToPlay = save_slot->Episode;
					SetLevelToLoadForAlien(AlienEpisodeToPlay);
				
					break;
	
				case I_Predator :
					PredatorEpisodeToPlay = save_slot->Episode;
					SetLevelToLoadForPredator(PredatorEpisodeToPlay);
					break;
		
			}
			SetBriefingTextForEpisode(save_slot->Episode, AvP.PlayerType);
			AvPMenus.MenusState = MENUSSTATE_STARTGAME;
		}
		else
		{
			//cancel request
			LoadGameRequest = SAVELOAD_REQUEST_NONE;
		}
	}
}


static void PasteFromClipboard(char* Text,int MaxTextLength)
{
	fprintf(stderr, "PasteFromClipboard(%p, %d)\n", Text, MaxTextLength);	
#if 0
	HANDLE hGlobal;
	if(!Text)
	{
		return;
	}

	if(IsClipboardFormatAvailable(CF_TEXT))
	{
		OpenClipboard(0);
		hGlobal = GetClipboardData(CF_TEXT);
		if(hGlobal)
		{
			char* pGlobal = GlobalLock(hGlobal);
			if(pGlobal)
			{
				strncpy(Text,pGlobal,MaxTextLength-1);
				Text[MaxTextLength-1] = 0;
				GlobalUnlock(hGlobal);
			}
		}
		CloseClipboard();
	}
#endif	
}
