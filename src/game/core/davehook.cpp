/*******************************************************************
 *
 *    DESCRIPTION: 	davehook.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 18/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "davehook.h"


#include "r2base.h"
	// hooks to R2 code

#include "gadget.h"
	// hooks to gadgets code

#include "daemon.h"
	// hooks to daemon code

#include "rentrntq.h"

//#include "ammo666.hpp"

#include "iofocus.h"

#include "font.h"

#include "hudgadg.hpp"

#include "consvar.hpp"
#include "conscmnd.hpp"

#include "missions.hpp"

#include "indexfnt.hpp"
	// Includes for console variables:
	#include "textexp.hpp"

	// Includes for console commands:
	#include "consvar.hpp"
	#include "modcmds.hpp"
	#include "trepgadg.hpp"

	#include "consbind.hpp"

	#include "consbtch.hpp"

	
	#define UseLocalAssert Yes
	#include "ourasert.h"

#include "avp_menus.h"
/* Version settings ************************************************/

/* Constants *******************************************************/

/* Macros **********************************************************/

/* Imported function prototypes ************************************/

/* Imported data ***************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

		extern unsigned char KeyboardInput[];

		extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
		extern int VideoModeColourDepth;

		extern int bEnableTextprint;
		extern int bEnableTextprintXY;
		extern signed int HUDTranslucencyLevel;

#ifdef __cplusplus
	};
#endif



/* Exported globals ************************************************/
	#if 0
	int FixP_Test = (ONE_FIXED/2);
	#endif

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/
	namespace Testing
	{
		void VVTest(void);
		void VITest(int);
		int IVTest(void);
		int IITest(int);

		void DumpRefCounts(void);
		void DumpVideoMode(void);
	};

	static int bFirstFrame = No;

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
/*static*/ void ConsoleVariable :: CreateAll(void)
{
	// hook to create all the console variables
	// (to make it easy to add new ones)


	MakeSimpleConsoleVariable_Int
	(
		TextExpansion ::  bVerbose, // int& Value_ToUse,
		"EXPV", // ProjChar* pProjCh_ToUse,
		"(VERBOSE REPORTS OF TEXT EXPANSIONS)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	#if 0
	MakeSimpleConsoleVariable_Int
	(
		bEnableTextprint, // int& Value_ToUse,
		"TEXT", // ProjChar* pProjCh_ToUse,
		"(ENABLE/DISABLE DIAGNOSTIC TEXT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);

	MakeSimpleConsoleVariable_Int
	(
		bEnableTextprintXY, // int& Value_ToUse,
		"TEXTXY", // ProjChar* pProjCh_ToUse,
		"(ENABLE/DISABLE POSITIONED TEXT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1  // int MaxVal_New
	);
	MakeSimpleConsoleVariable_Int
	(
		HUDTranslucencyLevel, // int& Value_ToUse,
		"HUDALPHA", // ProjChar* pProjCh_ToUse,
		"(OPACITY OF HEAD-UP-DISPLAY)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		255  // int MaxVal_New
	);

	MakeSimpleConsoleVariable_FixP
	(
		FixP_Test, // int& Value_ToUse,
		"FIXPTEST", // ProjChar* pProjCh_ToUse,
		"(A TEST)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	MakeSimpleConsoleVariable_FixP
	(
		Daemon :: DaemonTimeScale, // int& Value_ToUse,
		"IO-TIME", // ProjChar* pProjCh_ToUse,
		"TIMESCALE FOR USER INTERFACE", // ProjChar* pProjCh_Description_ToUse
		655, // int MinVal_New,
		65536  // int MaxVal_New
	);

	MakeSimpleConsoleVariable_Int
	(
		KeyBinding :: bEcho,
		"ECHO-BIND",
		"(ENABLES ECHOING OF STRINGS BOUND TO KEYS)",
		0,
		1
	);

	MakeSimpleConsoleVariable_Int
	(
		BatchFileProcessing :: bEcho,
		"ECHO-BATCH",
		"(ENABLES ECHOING OF BATCH FILES)",
		0,
		1
	);
	#endif


}

/*static*/ void ConsoleCommand :: CreateAll(void)
{
	Make
	(
		"LISTCMD",
		"LIST ALL CONSOLE COMMANDS",
		ConsoleCommand :: ListAll
	);

	Make
	(
		"LISTEXP",
		"LIST ALL TEXT EXPANSIONS",
		TextExpansion :: ListAll
	);

	Make
	(
		"LISTVAR",
		"LIST ALL CONSOLE VARIABLES",
		ConsoleVariable :: ListAllVariables
	);



	// Need to add
	#if 0
		static void AttemptToBind
		(
			SCString* pSCString_Key, // description of key
			SCString* pSCString_ToBind // string to be bound
		);
		static void AttemptToUnbind
		(			
			SCString* pSCString_Key // description of key
		);
	#endif


	Make
	(
		"LISTBIND",
		"LIST ALL KEY BINDINGS",
		KeyBinding::ListAllBindings
	);
	Make
	(
		"UNBIND-ALL",
		"GET RID OF ALL KEY BINDINGS",
		KeyBinding::UnbindAll
	);
	#if 0
	Make
	(
		"LISTMOD",
		"LIST ALL MODULES",
		ModuleCommands :: ListModules
	);
	Make
	(
		"C-KILL",
		"CLEAR TEXT REPORT QUEUE",
		TextReportGadget :: ClearTheQueue
	);

	Make
	(
		"D-REFDUMP",
		"DIAGNOSTICS ON REFERENCE COUNTS",
		Testing :: DumpRefCounts
	);

	Make
	(
		"VIDMODE",
		"DUMP INFO ON VIDEO MODE",
		Testing :: DumpVideoMode
	);
	#endif
	#if 0
	Make
	(
		"VVTEST",
		"TEST COMMAND",
		Testing :: VVTest
	);
	Make
	(
		"VITEST",
		"TEST COMMAND",
		Testing :: VITest
	);
	Make
	(
		"IVTEST",
		"TEST COMMAND",
		Testing :: IVTest
	);
	Make
	(
		"IITEST",
		"TEST COMMAND",
		Testing :: IITest
	);
	#endif


}

void DAVEHOOK_Init(void)
{
#if 0
	static SCString* pSCString_TestLeak = new SCString("this is a test memory leak");
#endif

	MissionHacks :: TestInit();

	{
		DAEMON_Init();

//		AmmoDaemon :: Init();
	}

	new IndexedFont_HUD(DATABASE_MESSAGE_FONT);
	
	GADGET_Init();

	#if UseGadgets
	ConsoleVariable :: CreateAll();
	ConsoleCommand :: CreateAll();
	#endif

}

void DAVEHOOK_UnInit(void)
{
	IndexedFont :: UnloadFont(DATABASE_MESSAGE_FONT);

	GADGET_UnInit();
}

void DAVEHOOK_Maintain(void)
{
	{
//		AmmoDaemon :: Maintain();

		DAEMON_Maintain();
	}

	#if KeyBindingUses_KEY_ID
	{
		KeyBinding :: Maintain();
	}
	#endif

	// Hacked in input support:
	{
		#if 0
		if ( KeyboardInput[ KEY_J ] )
		{
			// Test jitter hack
			HUDGadget* pHUD = HUDGadget :: GetHUD();

			if ( pHUD )
			{
				pHUD -> Jitter(ONE_FIXED);
			}

		}
		#endif

		#if 0
		if ( KeyboardInput[ KEY_CR ] )
		{
			IOFOCUS_Toggle();

			#if 0
			// toggle typing/control mode
			textprint("\n\n\n\nTOGGLE TYPING MODE\n");
			#endif
		}
		#endif
	}

	if ( bFirstFrame )
	{
		RE_ENTRANT_QUEUE_WinMain_FlushMessagesWithoutProcessing();
		// this is a hack to ensure that none of the keypresses used
		// in the menu get through to the first frame of the game and 
		// for example, switch to typing mode (for CR presses)
		
		bFirstFrame = No;
	}
	else
	{
		// Flush the WinProc messages:
		RE_ENTRANT_QUEUE_WinMain_FlushMessages();
	}
}

void DAVEHOOK_ScreenModeChange_Setup(void)
{
}

void DAVEHOOK_ScreenModeChange_Cleanup(void)
{
	R2BASE_ScreenModeChange_Cleanup();
	GADGET_ScreenModeChange_Cleanup();

	bFirstFrame = Yes;
		// to ensure a flush without processing of messages in first frame, so as to
		// avoid carriage returns/enter from menu selections triggering typing mode

	// Run program-generated batch file:
	#if !(PREDATOR_DEMO|MARINE_DEMO||ALIEN_DEMO||DEATHMATCH_DEMO)
	BatchFileProcessing :: Run("config.cfg");

	// Run user-generated batch file:
	BatchFileProcessing :: Run("startup.cfg");
	#endif
}


/* Internal function definitions ***********************************/
void Testing :: VVTest(void)
{
	textprint("Testing :: VVTest()\n");
}
void Testing :: VITest(int i)
{
	textprint("Testing :: VITest(%i)\n",i);
}
int Testing :: IVTest(void)
{
	textprint("Testing :: IVTest()\n");

	return 180;
}
int Testing :: IITest(int i)
{
	textprint("Testing :: IITest(%i)\n",i);

	return (i*2);
}


// Diagnostic hook for reference counting system:
void Testing :: DumpRefCounts(void)
{
	#if TrackReferenceCounted
	{
		SCString* pSCString_Feedback = new SCString("DUMPING REFCOUNT INFO");
		pSCString_Feedback -> SendToScreen();
		pSCString_Feedback -> R_Release();

		LogFile tempLog("REFDUMP.TXT");
		RefCountObject :: DumpAll(tempLog);
	}
	#else
	{
		SCString* pSCString_Feedback = new SCString("REFCOUNT INFO DISABLED AT COMPILE-TIME");
		pSCString_Feedback -> SendToScreen();
		pSCString_Feedback -> R_Release();
	}
	#endif
}

void Testing :: DumpVideoMode(void)
{
	char msg[256];
	sprintf
	(
		msg,
		"VIDEO MODE:%iX%iX%i",
		ScreenDescriptorBlock . SDB_Width,
		ScreenDescriptorBlock . SDB_Height,
		VideoModeColourDepth
	);
	
	SCString* pSCString_Feedback = new SCString(msg);
	pSCString_Feedback -> SendToScreen();
	pSCString_Feedback -> R_Release();
}
