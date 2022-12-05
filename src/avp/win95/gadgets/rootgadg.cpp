/*******************************************************************
 *
 *    DESCRIPTION: 	rootgadg.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 14/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "rootgadg.hpp"

#if UseGadgets
	#include "hudgadg.hpp"
		
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
		extern signed int HUDTranslucencyLevel;
			// ranges from 0 to 255 inclusive ; convert to fixed point...


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

	// private:
	/*static*/ RootGadget* RootGadget :: pSingleton = NULL;

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class RootGadget : public Gadget
// friend extern void GADGET_Init(void);
// friend extern void GADGET_UnInit(void);
	// friend functions: these get permission in order to allow
	// construction/destruction

// public:
void RootGadget :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		#if 0
		textprint
		(
			"RootGadget :: Render at (%i,%i) clipped (%i,%i,%i,%i) alpha=%i\n",
			R2Pos . x,
			R2Pos . y,
			R2Rect_Clip . x0,
			R2Rect_Clip . y0,
			R2Rect_Clip . x1,
			R2Rect_Clip . y1,
			FixP_Alpha
		);
		#endif

		if ( pHUDGadg )
		{
			// HUDTranslucencyLevel ranges from 0 to 255 inclusive ; convert to fixed point...

			GLOBALASSERT( HUDTranslucencyLevel >= 0);
			GLOBALASSERT( HUDTranslucencyLevel <= 255 );

			pHUDGadg -> Render
			(
				R2Pos,
				R2Rect_Clip,
				(HUDTranslucencyLevel << 8) // int FixP_Alpha
			);
		}
	}
}

void RootGadget :: RefreshHUD(void)
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		// For the moment, destroy any HUD:
		if ( pHUDGadg )
		{
			delete pHUDGadg;
			pHUDGadg = NULL;
		}

		GLOBALASSERT( NULL == pHUDGadg );

		// And then recreate if necessary:
		{
			extern AVP_GAME_DESC AvP;		 /* game description */

			if
			(
				AvP.GameMode == I_GM_Playing
			)
			{
				pHUDGadg = HUDGadget :: MakeHUD
				(
					AvP.PlayerType // I_PLAYER_TYPE IPlayerType_ToMake
				);
			}
		}
	}
}



// private:
RootGadget :: RootGadget
(
) : Gadget
	(
		#if debug
		"RootGadget"
		#endif
	)		
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSingleton == NULL );
	}

	/* CODE */
	{
		pHUDGadg = NULL;

		pSingleton = this;
	}
}

RootGadget :: ~RootGadget()
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSingleton == this );
	}

	/* CODE */
	{
		pSingleton = NULL;

		if ( pHUDGadg )
		{
			delete pHUDGadg;
		}
	}
}



/* Internal function definitions ***********************************/



#endif // UseGadgets
