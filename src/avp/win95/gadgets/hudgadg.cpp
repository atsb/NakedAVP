/*******************************************************************
 *
 *    DESCRIPTION: 	hudgadg.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 14/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "hudgadg.hpp"
//#include "mhudgadg.hpp"
#include "ahudgadg.hpp"
//#include "phudgadg.hpp"
#include "trepgadg.hpp"
	
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
#if UseGadgets
	// private:
	/*static*/ HUDGadget* HUDGadget :: pSingleton;
#endif

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
#if UseGadgets
// HUD Gadget is an abstract base class for 3 types of HUD; one for each species
// It's abstract because the Render() method remains pure virtual
// class HUDGadget : public Gadget
// public:

// Factory method:
/*static*/ HUDGadget* HUDGadget :: MakeHUD
(
	I_PLAYER_TYPE IPlayerType_ToMake
)
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		#if 0
		switch ( IPlayerType_ToMake )
		{
			case I_Marine:
				return new MarineHUDGadget();

			case I_Predator:
				return new PredatorHUDGadget();

			case I_Alien:
				return new AlienHUDGadget();

			default:
				GLOBALASSERT(0);
				return NULL;
		}
		#else
		return new AlienHUDGadget();
		#endif
	}
}


// Destructor:
/*virtual*/ HUDGadget :: ~HUDGadget()
{
	/* PRECONDITION */
	{
		GLOBALASSERT( this == pSingleton );
	}

	/* CODE */
	{
		pSingleton = NULL;
	}
}


// protected:
// Constructor is protected since an abstract class
HUDGadget :: HUDGadget
(
	#if debug
	char* DebugName
	#endif
) : Gadget
	(
		#if debug
		DebugName
		#endif		
	)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( NULL == pSingleton );
	}

	/* CODE */
	{
		#if 0
		pSCString_Current = NULL;
		#endif

		pSingleton = this;
	}
}
#endif // UseGadgets

/* Internal function definitions ***********************************/
