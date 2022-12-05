/*******************************************************************
 *
 *    DESCRIPTION: 	chtcodes.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 28/11/97 by moving the cheat code processor
 *				from out of SCSTRING.CPP
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "scstring.hpp"
#include "gadget.h"
#include "strutil.h"

#include "module.h"
#include "stratdef.h"
#include "dynblock.h"
#include "equipmnt.h"
#include "rootgadg.hpp"
#include "hudgadg.hpp"
//#include "introut.hpp"
#include "modcmds.hpp"

#include "bh_types.h"

#include "consvar.hpp"

#include "missions.hpp"

#include "textexp.hpp"

#include "refobj.hpp"

#include "debuglog.hpp"

#include "trepgadg.hpp"

#include "conscmnd.hpp"

#include "consbind.hpp"

extern "C"
{
	#include "weapons.h"
	#include "avp_menus.h"
};


	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/

/* Constants *******************************************************/

/* Macros **********************************************************/

/* Imported function prototypes ************************************/

/* Imported data ***************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
		extern int DebuggingCommandsActive;
	
		extern int bEnableTextprint;

		extern SCENE Global_Scene;

		#if 0
		extern OurBool			DaveDebugOn;
		extern FDIEXTENSIONTAG	FDIET_Dummy;
		extern IFEXTENSIONTAG	IFET_Dummy;
		extern FDIQUAD			FDIQuad_WholeScreen;
		extern FDIPOS			FDIPos_Origin;
		extern FDIPOS			FDIPos_ScreenCentre;
		extern IFOBJECTLOCATION IFObjLoc_Origin;
		extern UncompressedGlobalPlotAtomID UGPAID_StandardNull;
		extern IFCOLOUR			IFColour_Dummy;
 		extern IFVECTOR			IFVec_Zero;
		#endif
#ifdef __cplusplus
	};
#endif



/* Exported globals ************************************************/

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/
	namespace Cheats
	{
		void ToggleImmortality(void);
		void CommitSuicide(void);
	};

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
void SCString :: ProcessAnyCheatCodes(void)
{
	#if UseGadgets

	#if 0
	{
		char Msg[256];
		sprintf
		(
			Msg,
			"Process for cheat codes \"%s\"",
			pProjCh_Val
		);
		GADGET_NewOnScreenMessage(Msg);
	}
	#endif
	// Processing for the "console variable" system:
	{
		if
		(
			ConsoleVariable :: Process( pProjCh_Val )
		)
		{
			// then this has been processed; stop
			return;
		}
	}

	// Processing for the "console commands" system:
	{
		if
		(
			ConsoleCommand :: Process( pProjCh_Val )
		)
		{
			// then this has been processed; stop
			return;
		}
	}
	// Expansion-related commands:
	{
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"EXP+ ",
				5
			)
		)
		{
			TextExpansion :: AddExpansion
			(
				pProjCh_Val+5
					// ProjChar* pProjCh_ToParse
			);
			return;
		}
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"EXP- ",
				5
			)
		)
		{
			TextExpansion :: TryToRemoveExpansion
			(
				pProjCh_Val+5
					// ProjChar* pProjCh_ToParse
			);
			return;
		}
	}

	// Key-binding related commands:
	{
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"BIND ",
				5
			)
		)
		{
			KeyBinding :: ParseBindCommand
			(
				pProjCh_Val+5
					// ProjChar* pProjCh_ToParse
			);
			return;
		}
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"UNBIND ",
				7
			)
		)
		{
			KeyBinding :: ParseUnbindCommand
			(
				pProjCh_Val+7
					// ProjChar* pProjCh_ToParse
			);
			return;
		}
	}
	#ifndef AVP_DEBUG_VERSION // allow debug commands without -debug
	#ifndef AVP_DEBUG_FOR_FOX // allow debug commands without -debug
	if (DebuggingCommandsActive)
	#endif
	#endif
	{
		if
		(
			#ifndef AVP_DEBUG_VERSION 
			STRUTIL_SC_Strequal
			(
				pProjCh_Val,
				"GOD" //ProjChar* pProjCh_2
			)
			#else // allow case insensitive
			STRUTIL_SC_Strequal_Insensitive
			(
				pProjCh_Val,
				"GOD" //ProjChar* pProjCh_2
			)
			#endif
		)
		{
			Cheats :: ToggleImmortality();
			return;
		}
	}
	#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
	// Module commands:
	{
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"MODULE ",
				7
			)
		)
		{
			// Then pass the rest of the string as an argument to the module
			// teleport code:

			ModuleCommands :: TryToTeleport
			(
				pProjCh_Val+7
				//char* UpperCasePotentialModuleName
			);
			return;
		}
		if
		(
			0 == _strnicmp
			(
				pProjCh_Val,
				"MOD ",
				4
			)
		)
		{
			// Then pass the rest of the string as an argument to the module
			// teleport code:

			ModuleCommands :: TryToTeleport
			(
				pProjCh_Val+4
				//char* UpperCasePotentialModuleName
			);
			return;
		}
	}



	if
	(
		STRUTIL_SC_Strequal_Insensitive
		(
			pProjCh_Val,
			"SUICIDE" //ProjChar* pProjCh_2
		)
	)
	{
		Cheats :: CommitSuicide();
		return;
	}





	if
	(
		STRUTIL_SC_Strequal_Insensitive
		(
			pProjCh_Val,
			"DONE IT" //ProjChar* pProjCh_2
		)
	)
	{
		MissionObjective :: TestCompleteNext();
	}


	// Unimplemented cheat code ideas:	
		//empty


	#endif
	#endif // UseGadgets
}

/* Internal function definitions ***********************************/
// namespace Cheats
void Cheats :: ToggleImmortality(void)
{
	// immortality cheat
	#if 1
	if ( PlayerStatusPtr->IsImmortal )
	{
		GADGET_NewOnScreenMessage("IMMORTALITY DISABLED");
		// LOCALISEME();
		PlayerStatusPtr->IsImmortal = 0;
	}
	else
	{
		GADGET_NewOnScreenMessage("IMMORTALITY ENABLED");
		// LOCALISEME();
		PlayerStatusPtr->IsImmortal = 1;
	}		
	#endif
}

void Cheats :: CommitSuicide(void)
{
	// First, disable immortality:
	PlayerStatusPtr->IsImmortal = 0;


	// Then apply lots of damage:		

	CauseDamageToObject(Player->ObStrategyBlock, &certainDeath, ONE_FIXED,NULL);
}
