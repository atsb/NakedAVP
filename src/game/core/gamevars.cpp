/* KJL 11:10:15 28/01/98 - 

	This file contains game-specific console variables
	
 */
//#include "rentrntq.h"
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"

#include "davehook.h"


#include "r2base.h"
	// hooks to R2 code

#include "gadget.h"
	// hooks to gadgets code

#include "daemon.h"
	// hooks to daemon code

#include "rentrntq.h"

//#include "ammo666.hpp"

//#include "iofocus.h"

//#include "statpane.h"

//#include "font.h"

//#include "hudgadg.hpp"

#include "consvar.hpp"
#include "conscmnd.hpp"

#include "equipmnt.h"
#include "pldnet.h"
#include "avp_menus.h"

extern "C"
{

/* KJL 11:48:45 28/01/98 - used to scale NormalFrameTime, so the game can be slowed down */
extern int TimeScale;
extern int MotionTrackerScale;
extern int LeanScale;

extern int CrouchIsToggleKey;
extern int CloakingMode;
extern int LogConsoleTextToFile;

extern int PlanarGravity;


extern int GlobalLevelOfDetail_Hierarchical;
extern int JoystickEnabled;

extern int SkyColour_R;
extern int SkyColour_G;
extern int SkyColour_B;

extern int DrawCompanyLogos;

extern int QuantumObjectDieRollOveride;

extern int WireFrameMode;
extern int LightScale;

extern int MotionTrackerSpeed; 
extern int MotionTrackerVolume;

extern int DrawFullBright;

extern void ChangeToMarine();
extern void ChangeToAlien();
extern void ChangeToPredator();

extern int SentrygunSpread;
int PlaySounds;
int DopplerShiftIsOn;

extern int UseExtrapolation;

extern int DebuggingCommandsActive;

extern int AutoWeaponChangeOn;

void CreateGameSpecificConsoleVariables(void)
{
	TimeScale = 65536;
	if (AvP.PlayerType==I_Alien)
	{
		LeanScale=ONE_FIXED*3;
	}
	else
	{
		LeanScale=ONE_FIXED;
	}
	CrouchIsToggleKey = 0;
	CloakingMode=0;
	LogConsoleTextToFile=0;
	JoystickEnabled=0;
	GlobalLevelOfDetail_Hierarchical = ONE_FIXED;
	DrawCompanyLogos=0;
	LightScale = ONE_FIXED;
	DopplerShiftIsOn=1;
	PlaySounds=1;
	DrawFullBright=0;

	#ifndef AVP_DEBUG_VERSION // allow debug commands without -debug
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
		ConsoleVariable :: MakeSimpleConsoleVariable_FixP
		(
			TimeScale, // int& Value_ToUse,
			"TIMESCALE", // ProjChar* pProjCh_ToUse,
			"1.0 IS NORMAL", // ProjChar* pProjCh_Description_ToUse
			655, // int MinVal_New,
			65536*4, // int MaxVal_New
			IsACheat
		);
		ConsoleVariable :: MakeSimpleConsoleVariable_FixP
		(
			LeanScale, // int& Value_ToUse,
			"LEANSCALE", // ProjChar* pProjCh_ToUse,
			"1.0 IS DEFAULT, HIGHER MEANS MORE TILT", // ProjChar* pProjCh_Description_ToUse
			0, // int MinVal_New,
			65536*10,  // int MaxVal_New
			IsACheat
		);

		ConsoleVariable :: MakeSimpleConsoleVariable_Int
		(
			WireFrameMode, // int& Value_ToUse,
			"WIREFRAMEMODE", // ProjChar* pProjCh_ToUse,
			"0 = OFF, 1 = ENVIRONMENT, 2 = OBJECTS, 3 = EVERYTHING", // ProjChar* pProjCh_Description_ToUse
			 0, // int MinVal_New,
			 3, // int MaxVal_New
			IsACheat
		);
		ConsoleVariable :: MakeSimpleConsoleVariable_Int
		(
			(int&)DopplerShiftIsOn, // int& Value_ToUse,
			"DOPPLERSHIFT", // ProjChar* pProjCh_ToUse,
			"", // ProjChar* pProjCh_Description_ToUse
			 0, // int MinVal_New,
			 1, // int MaxVal_New
			IsACheat
		);
		ConsoleVariable :: MakeSimpleConsoleVariable_Int
		(
			SkyColour_R, // int& Value_ToUse,
			"SKY_RED", // ProjChar* pProjCh_ToUse,
			"SET RED COMPONENT OF SKY COLOUR", // ProjChar* pProjCh_Description_ToUse
			0, // int MinVal_New,
			255,  // int MaxVal_New
			IsACheat
		);

		ConsoleVariable :: MakeSimpleConsoleVariable_Int
		(
			SkyColour_G, // int& Value_ToUse,
			"SKY_GREEN", // ProjChar* pProjCh_ToUse,
			"SET GREEN COMPONENT OF SKY COLOUR", // ProjChar* pProjCh_Description_ToUse
			0, // int MinVal_New,
			255,  // int MaxVal_New
			IsACheat
		);

		ConsoleVariable :: MakeSimpleConsoleVariable_Int
		(
			SkyColour_B, // int& Value_ToUse,
			"SKY_BLUE", // ProjChar* pProjCh_ToUse,
			"SET BLUE COMPONENT OF SKY COLOUR", // ProjChar* pProjCh_Description_ToUse
			0, // int MinVal_New,
			255,  // int MaxVal_New
			IsACheat
		);
		ConsoleVariable :: MakeSimpleConsoleVariable_FixP
		(
			MotionTrackerSpeed, // int& Value_ToUse,
			"MOTIONTRACKERSPEED", // ProjChar* pProjCh_ToUse,
			"1.0 IS NORMAL", // ProjChar* pProjCh_Description_ToUse
			0, // int MinVal_New,
			65536*16, // int MaxVal_New
			IsACheat
		);
	}

	#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
	ConsoleVariable :: MakeSimpleConsoleVariable_FixP
	(
		MotionTrackerScale, // int& Value_ToUse,
		"MOTIONTRACKERSCALE", // ProjChar* pProjCh_ToUse,
		"1.0 IS FULL SIZE", // ProjChar* pProjCh_Description_ToUse
		26214, // int MinVal_New,
		655360  // int MaxVal_New
	);



	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		CloakingMode, // int& Value_ToUse,
		"CLOAKINGMODE", // ProjChar* pProjCh_ToUse,
		"0 MEANS TRANSLUCENCY WAVES, 1 MEANS SPECKLED", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		LogConsoleTextToFile, // int& Value_ToUse,
		"LOGCONSOLE", // ProjChar* pProjCh_ToUse,
		"ENABLE/DISABLE LOGGING CONSOLE TEXT TO FILE", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		JoystickEnabled, // int& Value_ToUse,
		"JOYSTICKENABLED", // ProjChar* pProjCh_ToUse,
		"ENABLE/DISABLE JOYSTICK", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		PlanarGravity,
		"PLANARGRAVITY",
		"",
		0,
		1
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		DrawCompanyLogos,
		"DRAW_LOGOS",
		"",
		0,
		1
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_FixP
	(
		GlobalLevelOfDetail_Hierarchical, // int& Value_ToUse,
		"LOD_HIERARCHICAL", // ProjChar* pProjCh_ToUse,
		"1.0 IS NORMAL, 0 MEANS ALWAYS USE MOST DETAILED LEVEL, HIGHER NUMBERS MEAN LOWER DETAIL MODELS ARE USED AT CLOSER DISTANCES", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536*100 // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_FixP
	(
		LightScale, // int& Value_ToUse,
		"LIGHTSCALE", // ProjChar* pProjCh_ToUse,
		"1.0 IS NORMAL", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536*100 // int MaxVal_New
	);



	
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		QuantumObjectDieRollOveride, // int& Value_ToUse,
		"QUANTUM_ROLL", // ProjChar* pProjCh_ToUse,
		"FORCE QUANTUM OBJECT DIE ROLL (-1 MEANS DON'T FORCE ROLL)", // ProjChar* pProjCh_Description_ToUse
		 -1, // int MinVal_New,
		 65535 // int MaxVal_New
	);

	ConsoleCommand :: Make
	(
		"MORPH_ALIEN",
		"BECOME AN ALIEN",
		ChangeToAlien
	);
	ConsoleCommand :: Make
	(
		"MORPH_MARINE",
		"BECOME A MARINE",
		ChangeToMarine
	);
	ConsoleCommand :: Make
	(
		"MORPH_PREDATOR",
		"BECOME A PREDATOR",
		ChangeToPredator
	);

	//various network scoring options
	
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.baseKillValue, // int& Value_ToUse,
		"NETSCORE_BASEKILLVALUE", // ProjChar* pProjCh_ToUse,
		"SET BASE VALUE FOR KILL/SUICIDE", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 255 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.characterKillValues[NGCT_Marine], // int& Value_ToUse,
		"NETSCORE_MARINEVALUE", // ProjChar* pProjCh_ToUse,
		"SET RELATIVE VALUE OF MARINE", // ProjChar* pProjCh_Description_ToUse
		 1, // int MinVal_New,
		 255 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.characterKillValues[NGCT_Predator], // int& Value_ToUse,
		"NETSCORE_PREDATORVALUE", // ProjChar* pProjCh_ToUse,
		"SET RELATIVE VALUE OF PREDATOR", // ProjChar* pProjCh_Description_ToUse
		 1, // int MinVal_New,
		 255 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.characterKillValues[NGCT_Alien], // int& Value_ToUse,
		"NETSCORE_ALIENVALUE", // ProjChar* pProjCh_ToUse,
		"SET RELATIVE VALUE OF ALIEN", // ProjChar* pProjCh_Description_ToUse
		 1, // int MinVal_New,
		 255 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.useDynamicScoring, // int& Value_ToUse,
		"NETSCORE_USEDYNAMICSCORING", // ProjChar* pProjCh_ToUse,
		"TURN DYNAMIC SCORING ON AND OFF", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.useCharacterKillValues, // int& Value_ToUse,
		"NETSCORE_USECHARVALUES", // ProjChar* pProjCh_ToUse,
		"TURN RELATIVE SCORES FOR CHARACTER TYPES ON AND OFF", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		netGameData.invulnerableTime, // int& Value_ToUse,
		"NETSTAT_INVULNERABLETIME", // ProjChar* pProjCh_ToUse,
		"SET INVULNERABILITY TIME AFTER RESPAWN (IN SECONDS)", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 255 // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)AvP.Difficulty, // int& Value_ToUse,
		"DIFFICULTY", // ProjChar* pProjCh_ToUse,
		"SET DIFFICULTY LEVEL (WILL REQUIRE LEVEL RESTART)", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 2 // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)netGameData.sendDecals, // int& Value_ToUse,
		"NETOPTION_SENDDECALS", // ProjChar* pProjCh_ToUse,
		"SHOULD DECALS BE SENT ACROSS THE NETWORK?", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);
	
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)SentrygunSpread, // int& Value_ToUse,
		"SENTRYGUN_SPREAD", // ProjChar* pProjCh_ToUse,
		"Angle in degrees for random deviation in direction", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 360 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)PlaySounds, // int& Value_ToUse,
		"PLAYSOUNDS", // ProjChar* pProjCh_ToUse,
		"", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)DrawFullBright, // int& Value_ToUse,
		"DRAWFULLBRIGHT", // ProjChar* pProjCh_ToUse,
		"", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_FixP
	(
		netGameData.sendFrequency, // int& Value_ToUse,
		"NETSENDFREQUENCY", // ProjChar* pProjCh_ToUse,
		"0 MEANS SEND EVERY FRAME", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536 // int MaxVal_New
	);
	#endif
/*
	MakeSimpleConsoleVariable_Int
	(
		bEnableTextprint, // int& Value_ToUse,
		"TEXT", // ProjChar* pProjCh_ToUse,
		"(ENABLE/DISABLE DIAGNOSTIC TEXT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);
*/

	ConsoleVariable :: MakeSimpleConsoleVariable_FixP
	(
		MotionTrackerVolume, // int& Value_ToUse,
		"MOTIONTRACKERVOLUME", // ProjChar* pProjCh_ToUse,
		"1.0 IS NORMAL", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		(int&)UseExtrapolation, // int& Value_ToUse,
		"EXTRAPOLATE_MOVEMENT", // ProjChar* pProjCh_ToUse,
		"TURN EXTRAPOLATION FOR MOVEMENT OF NETWORK OPPONENTS ON AND OFF", // ProjChar* pProjCh_Description_ToUse
		 0, // int MinVal_New,
		 1 // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		CrouchIsToggleKey, // int& Value_ToUse,
		"CROUCHMODE", // ProjChar* pProjCh_ToUse,
		"0 MEANS HOLD DOWN MODE, 1 MEANS TOGGLE MODE", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		AutoWeaponChangeOn,
		"AUTOWEAPONCHANGE",
		"SET TO 0 IF YOU DON'T WANT TO CHANGE TO NEWLY GAINED WEAPONS AUTOMATICALLY. OTHERWISE SET TO 1.",
		0,
		1
	);
	

}

}; // extern "C"
