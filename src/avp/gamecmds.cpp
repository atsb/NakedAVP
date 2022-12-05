#include "3dc.h"
#include "conscmnd.hpp"
#include "strutil.h"

// Includes for the actual commands:
//#include "consvar.hpp"
//#include "modcmds.hpp"
//#include "textexp.hpp"
//#include "trepgadg.hpp"

#define UseLocalAssert Yes
#include "ourasert.h"

extern "C"
{
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"

#include "showcmds.h"
#include "version.h"
#include "equipmnt.h"
#include "cheat.h"
#include "cd_player.h"
#include "dynblock.h"
#include "bh_rubberduck.h"
#include "pvisible.h"
#include "pldnet.h"

#include "lighting.h"
#include "paintball.h"		  
#include "decal.h"
#include "consolelog.hpp"
#include "psndplat.h"
#include "avp_menus.h"
#include "detaillevels.h"
#include "savegame.h"


int DebuggingCommandsActive=0;
extern void GimmeCharge(void);

// just change these to prototypes etc.
extern void QuickLoad(void)
{
	//set the load request
	LoadGameRequest = 0; //(that's slot 0 - not false)
}
extern void QuickSave(void)
{
	//set the save request
	SaveGameRequest = 0; //(that's slot 0 - not false)
}

void ConsoleCommandLoad(int slot)
{
	if(slot>=1 && slot<=NUMBER_OF_SAVE_SLOTS)
	{
		LoadGameRequest = slot-1;
	}
}

void ConsoleCommandSave(int slot)
{
	if(slot>=1 && slot<=NUMBER_OF_SAVE_SLOTS)
	{
		SaveGameRequest = slot-1;
	}
}
extern void DisplaySavesLeft(void);



struct DEBUGGINGTEXTOPTIONS ShowDebuggingText;

extern void ChangeNetGameType_Individual();
extern void ChangeNetGameType_Coop();
extern void ChangeNetGameType_LastManStanding();
extern void ChangeNetGameType_PredatorTag();
extern void ShowNearestPlayersName(void);
extern void ScreenShot(void);
extern void CastAlienBot(void);
extern void CastMarineBot(int weapon);
extern void CastPredoBot(int weapon);
extern void CastPredAlienBot(void);
extern void CastPraetorianBot(void);
extern void CastXenoborg(void);

extern int ShowMultiplayerScoreTimer;

static void ShowFPS(void)
{
	ShowDebuggingText.FPS = ~ShowDebuggingText.FPS;
}
#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
static void ShowEnvironment(void)
{
	ShowDebuggingText.Environment = ~ShowDebuggingText.Environment;
}
static void ShowCoords(void)
{
	ShowDebuggingText.Coords = ~ShowDebuggingText.Coords;
}
static void ShowModule(void)
{
	ShowDebuggingText.Module = ~ShowDebuggingText.Module;
}
static void ShowTarget(void)
{
	ShowDebuggingText.Target = ~ShowDebuggingText.Target;
}
static void ShowNetworking(void)
{
	ShowDebuggingText.Networking = ~ShowDebuggingText.Networking;
}
static void ShowDynamics(void)
{
	ShowDebuggingText.Dynamics = ~ShowDebuggingText.Dynamics;
}
static void ShowGunPos(void)
{
	ShowDebuggingText.GunPos = ~ShowDebuggingText.GunPos;
}
static void ShowTears(void)
{
	ShowDebuggingText.Tears = ~ShowDebuggingText.Tears;
}
static void ShowSounds(void)
{
	ShowDebuggingText.Sounds = ~ShowDebuggingText.Sounds;
}
#endif
static void ShowPolyCount(void)
{
	ShowDebuggingText.PolyCount = ~ShowDebuggingText.PolyCount;
}


extern void ChangeToMarine();
#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
static void ChangeToSpecialist_General()
{
	netGameData.myCharacterSubType=NGSCT_General;
	ChangeToMarine();
}
static void ChangeToSpecialist_PulseRifle()
{
	netGameData.myCharacterSubType=NGSCT_PulseRifle;
	ChangeToMarine();
}
static void ChangeToSpecialist_Smartgun()
{
	if(netGameData.allowSmartgun)
	{
		netGameData.myCharacterSubType=NGSCT_Smartgun;
	}
	ChangeToMarine();
}
static void ChangeToSpecialist_Flamer()
{
	if(netGameData.allowFlamer)
	{
		netGameData.myCharacterSubType=NGSCT_Flamer;
	}
	ChangeToMarine();
}
static void ChangeToSpecialist_Sadar()
{
	if(netGameData.allowSadar)
	{
		netGameData.myCharacterSubType=NGSCT_Sadar;
	}
	ChangeToMarine();
}
static void ChangeToSpecialist_GrenadeLauncher()
{
	if(netGameData.allowGrenadeLauncher)
	{
		netGameData.myCharacterSubType=NGSCT_GrenadeLauncher;
	}
	ChangeToMarine();
}
static void ChangeToSpecialist_Minigun()
{
	if(netGameData.allowMinigun)
	{
		netGameData.myCharacterSubType=NGSCT_Minigun;
	}
	ChangeToMarine();
}
static void ChangeToSpecialist_Frisbee()
{
	if(netGameData.allowSmartDisc)
	{
		netGameData.myCharacterSubType=NGSCT_Frisbee;
	}
	ChangeToMarine();
}

static void ChangeToSpecialist_Pistols()
{
	if(netGameData.allowPistols)
	{
		netGameData.myCharacterSubType=NGSCT_Pistols;
	}
	ChangeToMarine();
}

static void ForceAssertionFailure(void)
{
	LOCALASSERT("This assertion has been forced to stop the game"==0);
}
#endif

extern void ShowMultiplayerScores(void)
{
	ShowMultiplayerScoreTimer=5*ONE_FIXED;
}

extern void AddNetMsg_ChatBroadcast(char *string,BOOL same_species_only);

static void DoMultiplayerSay(char* string)
{
	AddNetMsg_ChatBroadcast(string,FALSE);
}

static void DoMultiplayerSaySpecies(char* string)
{
	AddNetMsg_ChatBroadcast(string,TRUE);
}


		
static void CDCommand_Play(int track)
{
	if(!CDDA_IsOn()) CDDA_SwitchOn();

	CDDA_Stop();
	CDDA_Play(track);
}
void CDCommand_PlayLoop(int track)
{
	if(!CDDA_IsOn()) CDDA_SwitchOn();

	CDDA_Stop();
	CDDA_PlayLoop(track);
}

static void CDCommand_Stop(void)
{
	CDDA_Stop();
}

static void CDCommand_Volume(int volume)
{
	if (volume>=0 && volume<=127)
	{
		CDDA_ChangeVolume(volume);
	}
	else
	{
		// say the volume setting is incorrect
	}
}


#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
static void GunX(int x)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RestPosition.vx = x;
}
static void GunY(int y)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RestPosition.vy = y;
}
static void GunZ(int z)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RestPosition.vz = z;
}
#endif

static void MakeRotatingLight(void)
{
	MakeLightElement(&Player->ObWorld,LIGHTELEMENT_ROTATING);
}

#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
static void RestartMultiplayer(void)
{
	/* obviously have to be in a network game... */
	if (AvP.Network==I_No_Network) return;

	int seed=FastRandom();
	AddNetMsg_RestartNetworkGame(seed);
	RestartNetworkGame(seed);
}

static void CompleteLevel(void)
{
	AvP.LevelCompleted = 1;
}
#endif


void CreateGameSpecificConsoleCommands(void)
{
	ShowDebuggingText.FPS = 0;
	ShowDebuggingText.Environment = 0;
	ShowDebuggingText.Coords = 0;
	ShowDebuggingText.Module = 0;
	ShowDebuggingText.Target = 0;
	ShowDebuggingText.Networking = 0;
	ShowDebuggingText.Dynamics = 0;
	ShowDebuggingText.GunPos = 0;
	ShowDebuggingText.Tears = 0;
	ShowDebuggingText.PolyCount = 0;

	#ifndef AVP_DEBUG_VERSION 
	BOOL IsACheat = TRUE;
	#else
	BOOL IsACheat = FALSE;
	#endif
	
	#ifndef AVP_DEBUG_VERSION // allow debug commands without -debug
	#ifndef AVP_DEBUG_FOR_FOX // allow debug commands without -debug
	if (DebuggingCommandsActive)
	#endif
	#endif
	{
		ConsoleCommand::Make
		(
			"GIVEALLWEAPONS",
			"BE CAREFUL WHAT YOU WISH FOR",
			GiveAllWeaponsCheat,
			IsACheat
		);


		/* KJL 14:51:09 29/03/98 - show commands */
		ConsoleCommand::Make
		(
			"SHOWFPS",
			"DISPLAY THE FRAMERATE",
			ShowFPS,
			IsACheat
		);

		ConsoleCommand::Make
		(
			"SHOWPOLYCOUNT",
			"DISPLAY NUMBER OF LANDSCAPE POLYS, AND NUMBER OF POLYS ACTUALLY RENDERED",
			ShowPolyCount,
			IsACheat
		);
		ConsoleCommand::Make
		(
			"LIGHT",
			"CREATE A LIGHT",
			MakeRotatingLight,
			IsACheat
		);	 
		ConsoleCommand :: Make
		(
			"GIMME_CHARGE",
			"GRANTS FULL FIELD CHARGE",
			GimmeCharge,
			IsACheat
		);
		ConsoleCommand :: Make
		(
			"ALIENBOT",
			"CREATES ALIEN BOT. SAME AS ANY OTHER ALIEN.",
			CastAlienBot,
			IsACheat
		);

		ConsoleCommand :: Make
		(
			"MARINEBOT",
			"CREATES MARINE BOT. SAME AS A GENERATED MARINE.",
			CastMarineBot,
			IsACheat
		);

		ConsoleCommand :: Make
		(
			"PREDOBOT",
			"CREATES PREDATOR BOT.",
			CastPredoBot,
			IsACheat
		);

		ConsoleCommand :: Make
		(
			"PREDALIENBOT",
			"CREATES PREDATOR ALIEN BOT.",
			CastPredAlienBot,
			IsACheat
		);

		ConsoleCommand :: Make
		(
			"PRAETORIANBOT",
			"CREATES PRAETORIAN GUARD BOT.",
			CastPraetorianBot,
			IsACheat
		);

		ConsoleCommand :: Make
		(
			"XENOBORG",
			"THEY'RE ALL BOTS ANYWAY...",
			CastXenoborg,
			IsACheat
		);


	}

	#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
	ConsoleCommand::Make
	(
		"SHOWENV",
		"DISPLAY THE ENVIRONMENT NAME",
		ShowEnvironment
	);

	ConsoleCommand::Make
	(
		"SHOWCOORDS",
		"DISPLAY THE PLAYERS CURRENT POSITION",
		ShowCoords
	);

	ConsoleCommand::Make
	(
		"SHOWMODULE",
		"DISPLAY THE PLAYERS CURRENT MODULE",
		ShowModule
	);

	ConsoleCommand::Make
	(
		"SHOWTARGET",
		"DISPLAY THE CURRENT TARGET POSITION",
		ShowTarget
	);

	ConsoleCommand::Make
	(
		"SHOWNETWORKING",
		"DISPLAY NETWORKING DEBUG TEXT",
		ShowNetworking
	);
	
	ConsoleCommand::Make
	(
		"SHOWDYNAMICS",
		"DISPLAY DYNAMICS DEBUG TEXT",
		ShowDynamics
	);

	ConsoleCommand::Make
	(
		"SHOWGUNPOS",
		"DISPLAY GUN OFFSET COORDS",
		ShowGunPos
	);

	ConsoleCommand::Make
	(
		"SHOWTEARS",
		"MAKE TEARS AND LINKING ERRORS APPEAR BRIGHT GREEN",
		ShowTears
	);

	ConsoleCommand::Make
	(
		"SHOWSOUNDS",
		"DISPLAY NUMBER OF ACTIVE SOUNDS",
		ShowSounds
	);


	#if 1
	ConsoleCommand::Make
	(
		"GUNX",
		"CHANGE POSITION",
		GunX
	);
	ConsoleCommand::Make
	(
		"GUNY",
		"CHANGE POSITION",
		GunY
	);
	ConsoleCommand::Make
	(
		"GUNZ",
		"CHANGE POSITION",
		GunZ
	);
	ConsoleCommand::Make
	(
		"DUCKBOT",
		"MAKE A RUBBER DUCK",
		CreateRubberDuckBot
	);

	ConsoleCommand::Make
	(
		"FORCEASSERTIONFAILURE",
		"MAKE AN ASSERTION FIRE, EXITING THE GAME",
		ForceAssertionFailure
	);
	#endif

	ConsoleCommand::Make
	(
		"RESTARTMULTIPLAYER",
		"RESTARTS A NETWORK GAME FROM SCRATCH",
		RestartMultiplayer
	);

	ConsoleCommand::Make
	(
		"PAINTBALL",
		"TOGGLES PAINTBALLMODE ON/OFF",
		TogglePaintBallMode
	);

	ConsoleCommand::Make
	(
		"BUG",
		"ADD A BUG REPORT TO CONSOLELOG.TXT",
		OutputBugReportToConsoleLogfile
	);
	ConsoleCommand::Make
	(
		"REMOVEDECALS",
		"DELETES ALL PRE-DECALS",
		RemoveAllFixedDecals
	);
	
	ConsoleCommand::Make
	(
		"TURN3DSOUNDHARDWAREOFF",
		"DEACTIVATES 3D SOUND IN HARDWARE",
		PlatDontUse3DSoundHW
	);
	ConsoleCommand::Make
	(
		"TURN3DSOUNDHARDWAREON",
		"ACTIVATES 3D SOUND IN HARDWARE",
		PlatUse3DSoundHW
	);

	ConsoleCommand::Make
	(
		"NETGAME_INDIVIDUAL",
		"CHANGE NETWORK GAME TYPE",
		ChangeNetGameType_Individual
	);
	ConsoleCommand::Make
	(
		"NETGAME_COOP",
		"CHANGE NETWORK GAME TYPE",
		ChangeNetGameType_Coop
	);
	ConsoleCommand::Make
	(
		"NETGAME_LASTMANSTANDING",
		"CHANGE NETWORK GAME TYPE",
		ChangeNetGameType_LastManStanding
	);
	ConsoleCommand::Make
	(
		"NETGAME_PREDATORTAG",
		"CHANGE NETWORK GAME TYPE",
		ChangeNetGameType_PredatorTag
	);


	ConsoleCommand::Make
	(
		"TRIGGER_PLOT_FMV",
		"",
		StartTriggerPlotFMV
	);

	
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_GENERAL",
		"Become a general marine (can use all weapons)",
		ChangeToSpecialist_General
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_PULSERIFLE",
		"Become a pulserifle marine",
		ChangeToSpecialist_PulseRifle
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_SMARTGUN",
		"Become a smartgun marine",
		ChangeToSpecialist_Smartgun
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_FLAMER",
		"Become a flamethrower marine",
		ChangeToSpecialist_Flamer
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_SADAR",
		"Become a sadar marine",
		ChangeToSpecialist_Sadar
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_GRENADELAUNCHER",
		"Become a grenade launcher marine",
		ChangeToSpecialist_GrenadeLauncher
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_MINIGUN",
		"Become a minigun marine",
		ChangeToSpecialist_Minigun
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_SD",
		"Become an SD marine",
		ChangeToSpecialist_Frisbee
	);
	ConsoleCommand::Make
	(
		"SPECIALISTMARINE_PISTOLS",
		"Become a pistol marine",
		ChangeToSpecialist_Pistols
	);

	
	#if 1
	ConsoleCommand::Make
	(
		"COMPLETE_LEVEL",
		"",
		CompleteLevel
	);
	#endif
	#endif

	/* KJL 15:52:41 29/03/98 - version info */
	ConsoleCommand::Make
	(
		"VERSION",
		"",
		GiveVersionDetails
	);

	ConsoleCommand::Make
	(
		"SAY",
		"BROADCAST MESSAGE",
		DoMultiplayerSay
	);

	ConsoleCommand::Make
	(
		"SAY_SPECIES",
		"BROADCAST MESSAGE",
		DoMultiplayerSaySpecies
	);

 	ConsoleCommand::Make
	(
		"CDSTOP",
		"STOP THE CD PLAYING",
		CDCommand_Stop
	);

	ConsoleCommand::Make
	(
		"CDPLAY",
		"SELECT A TRACK TO PLAY",
		CDCommand_Play
	);
	ConsoleCommand::Make
	(
		"CDPLAYLOOP",
		"SELECT A TRACK TO PLAY LOOPED",
		CDCommand_PlayLoop
	);
	
	ConsoleCommand::Make
	(
		"CDVOLUME",
		"SELECT SOUND LEVEL 0 TO 127",
		CDCommand_Volume
	);
	ConsoleCommand::Make
	(
		"ID_PLAYER",
		"Get name of player nearest centre of screen",
		ShowNearestPlayersName
	);
	ConsoleCommand::Make
	(
		"SHOW_SCORE",
		"Show frag table",
		ShowMultiplayerScores
	);

	ConsoleCommand::Make
	(
		"DETAIL_LEVEL_MAX",
		"",
		SetToDefaultDetailLevels
	);
	
	ConsoleCommand::Make
	(
		"DETAIL_LEVEL_MIN",
		"",
		SetToMinimalDetailLevels
	);

	ConsoleCommand::Make
	(
		"SCREENSHOT",
		"",
		ScreenShot
	);
	
	ConsoleCommand::Make
	(
		"QUICKSAVE",
		"",
		QuickSave
	);
	ConsoleCommand::Make
	(
		"QUICKLOAD",
		"",
		QuickLoad
	);
	ConsoleCommand::Make
	(
		"SAVE",
		"Save game to slot 1-8",
		ConsoleCommandSave
	);
	ConsoleCommand::Make
	(
		"LOAD",
		"Load game from slot 1-8",
		ConsoleCommandLoad
	);
	ConsoleCommand::Make
	(
		"SAVESLEFT",
		"",
		DisplaySavesLeft
	);

}	

} // extern "C"
