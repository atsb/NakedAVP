/*******************************************************************
 *
 *    DESCRIPTION: 	gadget.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 13/11/97 
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "gadget.h"
#include "rootgadg.hpp"
#include "r2base.h"
#include "hudgadg.hpp"
#include "ahudgadg.hpp"
#include "indexfnt.hpp"
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

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
#if UseGadgets

// class Gadget
// public:

/*virtual*/ Gadget :: ~Gadget()
{
	// ensure virtual destructor

	// empty
}

#if debug
void Gadget :: Render_Report
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha			
)
{
	// use to textprint useful information about a call to "Render"
	textprint
	(
		"%s::Render at(%i,%i) clip(%i,%i,%i,%i) a=%i\n",
		DebugName,
		R2Pos . x,
		R2Pos . y,
		R2Rect_Clip . x0,
		R2Rect_Clip . y0,
		R2Rect_Clip . x1,
		R2Rect_Clip . y1,
		FixP_Alpha
	);
}
#endif

// protected:

// end of class Gadget

extern void GADGET_Init(void)
{
	/* expects to be called at program boot-up time */

	/* PRECONDITION */
	{
		GLOBALASSERT( RootGadget :: GetRoot() == NULL );
	}

	/* CODE */
	{
		new RootGadget;
	}
}


extern void GADGET_UnInit(void)
{
	/* expects to be called at program shutdown time */

	/* PRECONDITION */
	{
		GLOBALASSERT( RootGadget :: GetRoot() );
	}

	/* CODE */
	{
		delete RootGadget :: GetRoot();
	}
}


extern void GADGET_Render(void)
{
	/* expects to be called within the rendering part of the main loop */

	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		#if 0
		textprint("GADGET_Render()\n");
		#endif

		// under construction...
		GLOBALASSERT( RootGadget :: GetRoot() );
		RootGadget :: GetRoot() -> Render
		(
			r2pos :: Origin, // const struct r2pos& R2Pos,
			r2rect :: PhysicalScreen(), // const struct r2rect& R2Rect_Clip,
			ONE_FIXED // int FixP_Alpha
		);

		#if 0
		// Test all the fonts:
		{
			SCString* pSCString_Test = new SCString("FONT TEST STRING");

			for (int i=0;i<NUM_FONTS;i++)
			{
				IndexedFont* pFont = IndexedFont :: GetFont( (fonts)i );
				GLOBALASSERT(pFont);

				r2pos R2Pos_TempCursor(0,i*20);

				pFont -> Render_Unclipped
				(
					R2Pos_TempCursor, // struct r2pos& R2Pos_Cursor,
					ONE_FIXED, // int FixP_Alpha,
					*pSCString_Test// const SCString& SCStr
				);
			}

			pSCString_Test -> R_Release();
		}
		#endif
	}
}


extern void GADGET_ScreenModeChange_Setup(void)
{
	/* expects to be called immediately before anything happens to the screen
	mode */

	/* PRECONDITION */
	{
	}

	/* CODE */
	{
	}
}


extern void GADGET_ScreenModeChange_Cleanup(void)
{
	/* expects to be called immediately after anything happens to the screen
	mode */


	/* PRECONDITION */
	{
		GLOBALASSERT( RootGadget :: GetRoot() );
	}

	/* CODE */
	{
		RootGadget :: GetRoot() -> RefreshHUD();
	}
}

extern void GADGET_NewOnScreenMessage( ProjChar* messagePtr )
{
	/* PRECONDITION */
	{
		GLOBALASSERT( messagePtr );
		GLOBALASSERT( RootGadget :: GetRoot() );
	}

	/* CODE */
	{
		if ( RootGadget :: GetRoot() -> GetHUD() )
		{
			SCString* pSCString_New = new SCString( messagePtr );

			RootGadget :: GetRoot() -> GetHUD() -> AddTextReport
			(
				pSCString_New
			);

			pSCString_New -> R_Release();
		}
	}
}										

extern void RemoveTheConsolePlease(void)
{
	AlienHUDGadget *HUD = (AlienHUDGadget*)RootGadget::GetRoot()->GetHUD();
	HUD->pTextReportGadg->Disappear();
}


#endif // UseGadgets

void SCString :: SendToScreen(void)
{
	// adds this as a new on-screen message
	#if UseGadgets
	/* PRECONDITION */
	{
		GLOBALASSERT( RootGadget :: GetRoot() );
	}

	/* CODE */
	{
		if ( RootGadget :: GetRoot() -> GetHUD() )
		{
			RootGadget :: GetRoot() -> GetHUD() -> AddTextReport
			(
				this
			);
		}
	}
	#else
	{
		// do nothing
	}
	#endif // UseGadgets
}





/* Internal function definitions ***********************************/
