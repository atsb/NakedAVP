/*******************************************************************
 *
 *    DESCRIPTION: 	ahudgadg.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 14/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "ahudgadg.hpp"

	#if UseGadgets
		#include "trepgadg.hpp"
		#include "t_ingadg.hpp"
		#include "iofocus.h"
	#endif

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
#if UseGadgets
// class AlienHUDGadget : public HUDGadget
// public:
void AlienHUDGadget :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	#if 0
	textprint
	(
		"AlienHUDGadget :: Render at (%i,%i) clipped (%i,%i,%i,%i) alpha=%i\n",
		R2Pos . x,
		R2Pos . y,
		R2Rect_Clip . x0,
		R2Rect_Clip . y0,
		R2Rect_Clip . x1,
		R2Rect_Clip . y1,
		FixP_Alpha
	);
	#endif

	pTextReportGadg -> UpdateLineTimes();

	struct r2pos R2Pos_TextReport = pTextReportGadg -> GetPos_Rel
	(
		R2Rect_Clip
	);

	GLOBALASSERT( pTextReportGadg );
	{
		pTextReportGadg -> Render
		(
			R2Pos_TextReport,
			R2Rect_Clip,
			FixP_Alpha
		);
	}

	// Render the text entry line iff input focus is set to text entry:
	GLOBALASSERT( pTextEntryGadg );
	if
	(
		IOFOCUS_AcceptTyping()
	)
	{
		// Force the text report gadget onto the screen
	   	pTextReportGadg	-> ForceOnScreen();

		// Render the text entry gadget:
		pTextEntryGadg -> Render
		(
			r2pos
			(
				R2Pos_TextReport . x,
				R2Pos_TextReport . y +  pTextReportGadg -> GetSize
				(
					R2Rect_Clip
				) . h
			),
			R2Rect_Clip,
			FixP_Alpha
		);
	}
	else
	{
		// Tell the text entry gadget it's not being rendered
		// (so it can fade out internally; however nothing will appear on-screen)
		pTextEntryGadg -> DontRender();
	}

}

AlienHUDGadget :: AlienHUDGadget
(
) : HUDGadget
	(
		#if debug
		"AlienHUDGadget"
		#endif
	)
{
	pTextReportGadg = new TextReportGadget();

	pTextEntryGadg = new TextEntryGadget();

}


AlienHUDGadget :: ~AlienHUDGadget()
{
	delete pTextEntryGadg;
	delete pTextReportGadg;
}

void AlienHUDGadget :: AddTextReport
(
	SCString* pSCString_ToAdd
		// ultimately turn into an MCString
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSCString_ToAdd );

		GLOBALASSERT( pTextReportGadg );
	}

	/* CODE */
	{
		pTextReportGadg -> AddTextReport
		(
			pSCString_ToAdd
		);
	}
}
void AlienHUDGadget :: ClearTheTextReportQueue(void)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pTextReportGadg );
	}

	/* CODE */
	{
		pTextReportGadg -> ClearQueue();
	}
}

#if EnableStatusPanels
void AlienHUDGadget :: RequestStatusPanel
(
	enum StatusPanelIndex I_StatusPanel
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( I_StatusPanel < NUM_STATUS_PANELS );
	}

	/* CODE */
	{
		// empty for the moment
	}
}

void AlienHUDGadget :: NoRequestedPanel(void)
{
	// empty for the moment
}
#endif // EnableStatusPanels

void AlienHUDGadget :: CharTyped
(
	char Ch
		// note that this _is _ a char
)
{
	GLOBALASSERT( pTextEntryGadg );

	pTextEntryGadg -> CharTyped
	(
		Ch
	);
}

void AlienHUDGadget :: Key_Backspace(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Backspace();
}
void AlienHUDGadget :: Key_End(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_End();
}
void AlienHUDGadget :: Key_Home(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Home();
}
void AlienHUDGadget :: Key_Left(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Left();
}
void AlienHUDGadget :: Key_Up(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Up();
}
void AlienHUDGadget :: Key_Right(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Right();
}
void AlienHUDGadget :: Key_Down(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Down();
}
void AlienHUDGadget :: Key_Delete(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Delete();
}
void AlienHUDGadget :: Key_Tab(void)
{
	GLOBALASSERT( pTextEntryGadg );
	pTextEntryGadg -> Key_Tab();
}

void AlienHUDGadget :: Jitter(int FixP_Magnitude)
{
	// empty for now
}

void AlienHUDGadget :: SetString(const char* text)
{
	SCString* string = new SCString(text);
	pTextEntryGadg -> SetString(*string);
	string->R_Release();
}


extern "C"
{
void BringDownConsoleWithSayTypedIn()
{
	//bring down console if it isn't already down
	if(!IOFOCUS_AcceptTyping()) IOFOCUS_Toggle();
		
	//put "SAY " in the console
	((AlienHUDGadget*)HUDGadget :: GetHUD())->SetString("SAY ");
}

void BringDownConsoleWithSaySpeciesTypedIn()
{
	//bring down console if it isn't already down
	if(!IOFOCUS_AcceptTyping()) IOFOCUS_Toggle();
		
	//put "SAY_SPECIES " in the console
	((AlienHUDGadget*)HUDGadget :: GetHUD())->SetString("SAY_SPECIES ");
}
};

// private:
#endif // UseGadgets

/* Internal function definitions ***********************************/
