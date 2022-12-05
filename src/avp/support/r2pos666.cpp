/*******************************************************************
 *
 *    DESCRIPTION: 	r2pos666.cpp - Daemonic r2 positions
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 1/12/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "r2pos666.hpp"
#include "inline.h"

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

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class R2PosDaemon : public Daemon
// public:
R2PosDaemon :: R2PosDaemon
(
	r2pos R2Pos_Int_Initial,
	OurBool bActive
) : Daemon( bActive ),
	R2Pos_Int_Current( R2Pos_Int_Initial ),
	R2Pos_FixP_Current
	(
		OUR_INT_TO_FIXED( R2Pos_Int_Initial . x ),
		OUR_INT_TO_FIXED( R2Pos_Int_Initial . y )		
	)
{

}

void R2PosDaemon :: SetPos_Int(const r2pos R2Pos_Int_New )
{
	R2Pos_Int_Current = R2Pos_Int_New;
	R2Pos_FixP_Current = r2pos
	(
		OUR_INT_TO_FIXED( R2Pos_Int_Current . x ),
		OUR_INT_TO_FIXED( R2Pos_Int_Current . y )
	);
}

void R2PosDaemon :: SetPos_FixP(const r2pos R2Pos_FixP_New )
{
	R2Pos_FixP_Current = R2Pos_FixP_New;
	R2Pos_Int_Current = r2pos
	(
		OUR_FIXED_TO_INT( R2Pos_FixP_Current . x ),
		OUR_FIXED_TO_INT( R2Pos_FixP_Current . y )
	);
}

// Activity remains pure virtual...


// private:
#if 0
r2pos R2Pos_Int_Current;
r2pos R2Pos_FixP_Current;
#endif


/* Internal function definitions ***********************************/
