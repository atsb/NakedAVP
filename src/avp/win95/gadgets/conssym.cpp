/*******************************************************************
 *
 *    DESCRIPTION: 	conssym.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 26/1/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "conssym.hpp"

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
	/*static*/ List <ConsoleSymbol*> ConsoleSymbol :: List_pConsoleSym;

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class ConsoleSymbol
// public:

// protected:
ConsoleSymbol :: ConsoleSymbol
(
	ProjChar* pProjCh_ToUse
) :	pSCString_Symbol
	(
		new SCString( pProjCh_ToUse )
			// constructor for the SCString adds the required reference
	)
{
    List_pConsoleSym . add_entry
    (
    	this
    );
}

ConsoleSymbol :: ~ConsoleSymbol()
{
	pSCString_Symbol ->R_Release();

	// remove from the list
    List_pConsoleSym . delete_entry
    (
    	this
    );
}




/* Internal function definitions ***********************************/
