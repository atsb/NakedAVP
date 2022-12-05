/*******************************************************************
 *
 *    DESCRIPTION: 	teletype.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 17/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "teletype.hpp"	
#include "daemon.h"
#include "inline.h"
#include "trepgadg.hpp"

	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/
    #define SupportTeletypeSound    Yes

    #if UseGadgets
		#include "indexfnt.hpp"

        #if SupportTeletypeSound
            #include "psnd.h"
            #include "psndproj.h"
        #endif
    #endif

/* Constants *******************************************************/
	#define FIXP_PIXELS_PER_SECOND	(ONE_FIXED * 768 * 16)

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
#if UseGadgets
class TeletypeDaemon : public Daemon
{
public:
	TeletypeDaemon
	(
		TeletypeGadget* pTeletypeGadg
	);
	~TeletypeDaemon();

	ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);

	OurBool HasFinishedPrinting(void);
		// so it can trigger next line to print...

	int CursorXOffset(void);

private:
	TeletypeGadget* pTeletypeGadg_Val;
	OurBool fFinished_Val;
	int FixP_TotalPixels;
		// total pixels within the string to be drawn

	int FixP_PixelsCovered;
		// pixels covered so far; also equals the x-offset of the cursor.

    #if SupportTeletypeSound
    int SoundHandle;
    #endif

};
// Inline functions:
	inline OurBool TeletypeDaemon::HasFinishedPrinting(void)
	{
		// so it can trigger next line to print...
		return fFinished_Val;
	}
	inline int TeletypeDaemon::CursorXOffset(void)
	{
		return OUR_FIXED_TO_INT( FixP_PixelsCovered );
	}

namespace TeletypeCursor
{
	void Render
	(
		const struct r2pos& R2Pos,
		const struct r2rect& R2Rect_Clip,
		int FixP_Alpha
	);
};

#endif // UseGadgets

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
#if UseGadgets
// class TeletypeGadget : public Gadget
// public:
void TeletypeGadget :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	#if 0
	Render_Report
	(
		R2Pos,
		R2Rect_Clip,
		FixP_Alpha		
	);
	#endif
	#if 0
	textprint
	(
		"Teletype:\"%s\"\n",
		pSCString_Val -> pProjCh()
	);
    #endif

    GLOBALASSERT( p666 );
    int Int_CursorXOffset = p666 -> CursorXOffset();

	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
	GLOBALASSERT( pLetterFont );

	r2pos R2Pos_Temp_Cursor = R2Pos;

	r2rect R2Rect_TeletypeClip = R2Rect_Clip;

	if
	(
		R2Rect_TeletypeClip . Width() > Int_CursorXOffset
	)
	{
		R2Rect_TeletypeClip . SetWidth( Int_CursorXOffset );
	}

	pLetterFont -> RenderString_Clipped
	(
		R2Pos_Temp_Cursor,
		R2Rect_TeletypeClip,
		FixP_Alpha,
		*pSCString_Val
	);

	if
	(
		!p666 -> HasFinishedPrinting()
	)
	{
		// then render cursor:
		struct r2pos R2Pos_Cursor = R2Pos;
		R2Pos_Cursor . x += Int_CursorXOffset;

		TeletypeCursor :: Render
		(
			R2Pos_Cursor,
			R2Rect_Clip,
			FixP_Alpha
		);
	}
}

TeletypeGadget :: TeletypeGadget
(
	TextReportGadget* pTextReportGadg,
	// parent
	SCString* pSCString
) : Gadget
	(
		#if debug
		"TeletypeGadget"
		#endif
	)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pTextReportGadg );
		GLOBALASSERT( pSCString );
	}

	/* CODE */
	{
		pTextReportGadg_Val = pTextReportGadg;
		pSCString_Val = pSCString;
		pSCString_Val -> R_AddRef();

		p666 = new TeletypeDaemon
		(
			this
		);
		GLOBALASSERT( p666 );
	}
}

TeletypeGadget :: ~TeletypeGadget()
{
	pSCString_Val -> R_Release();

	GLOBALASSERT( p666 );

	delete p666;
}

OurBool TeletypeGadget :: HasFinishedPrinting(void)
{
	// so that the next line knows when to begin
	GLOBALASSERT( p666 );
	return p666 -> HasFinishedPrinting();
}

void TeletypeGadget :: InformParentOfTeletypeCompletion(void)
{
	pTextReportGadg_Val -> TeletypeCompletionHook();
}

void TeletypeGadget :: DirectRenderCursor
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	// called by parent so that it can render its cursor even if 
	// it's finished printing - so that the last message can have
	// a flashing cursor

	GLOBALASSERT( HasFinishedPrinting() );

	GLOBALASSERT( p666 );

    int Int_CursorXOffset = p666 -> CursorXOffset();

	struct r2pos R2Pos_Cursor = R2Pos;
	R2Pos_Cursor . x += Int_CursorXOffset;

	TeletypeCursor :: Render
	(
		R2Pos_Cursor,
		R2Rect_Clip,
		FixP_Alpha
	);	
}


// private:

#endif // UseGadgets

/* Internal function definitions ***********************************/
#if UseGadgets
// class TeletypeDaemon : public CoordinateWithStrategy
// public:
TeletypeDaemon :: TeletypeDaemon
(
	TeletypeGadget* pTeletypeGadg
) : Daemon
	(
		Yes // OurBool fActive
	)
{
	GLOBALASSERT( pTeletypeGadg );

	#if SupportTeletypeSound
	SoundHandle = SOUND_NOACTIVEINDEX;
	#endif
	
	pTeletypeGadg_Val = pTeletypeGadg;

	fFinished_Val = No;

	FixP_TotalPixels = 
	#if 1
	OUR_INT_TO_FIXED
	(
		pTeletypeGadg -> GetStringWithoutReference() -> CalcSize
		(
			I_Font_TeletypeLettering
		) . w
	);
	#else
	OUR_INT_TO_FIXED
	(
		10
		*		
		pTeletypeGadg -> GetStringWithoutReference() -> GetNumChars()
		
	);
	#endif

	FixP_PixelsCovered = 0;

    #if SupportTeletypeSound
    // Try to start looping teletype sound:
    Sound_Play
    (
        SID_TELETEXT,
        "el",
        &SoundHandle
    );
        // SOUND_NOACTIVEINDEX used as error value
    #endif //SupportTeletypeSound
}

TeletypeDaemon :: ~TeletypeDaemon()
{
    #if SupportTeletypeSound
    if ( SoundHandle != SOUND_NOACTIVEINDEX )
    {
        Sound_Stop
        (
            SoundHandle
        );

        SoundHandle = SOUND_NOACTIVEINDEX;
    }
    #endif // SupportTeletypeSound
}

ACTIVITY_RETURN_TYPE TeletypeDaemon :: Activity(ACTIVITY_INPUT)
{
	#if 0
	textprint("TeletypeDaemon :: Activity(%i)\n",FixP_Time);
    #endif

	int FixP_PixelsThisFrame = MUL_FIXED(FIXP_PIXELS_PER_SECOND,FixP_Time);

	#if 0
	textprint
	(
		"FixP_PixelsToPrint = %i\n",FixP_PixelsThisFrame
	);

	textprint
	(
		"FixP_TotalPixels = %i\n",FixP_TotalPixels
	);
    #endif

	FixP_PixelsCovered += FixP_PixelsThisFrame;


	
	if
	(
		FixP_PixelsCovered >= FixP_TotalPixels  
	)
	{
		// Teletype has finished:
		FixP_PixelsCovered = FixP_TotalPixels;

		fFinished_Val = Yes;		
		
		Stop();

        #if SupportTeletypeSound
        if ( SoundHandle != SOUND_NOACTIVEINDEX )
        {
            Sound_Stop
            (
                SoundHandle
            );
            SoundHandle = SOUND_NOACTIVEINDEX;
        }
        #endif // SupportTeletypeSound

		// Tell text report line that it can trigger next string in the queue
		// (if there is one):
		pTeletypeGadg_Val -> InformParentOfTeletypeCompletion();

	}

	ACTIVITY_RVAL_CHANGE
}
// private:


// namespace TeletypeCursor
void TeletypeCursor :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	#if 1
		#define TELETYPE_CURSOR_WIDTH (10)
	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
	GLOBALASSERT( pLetterFont );	

	r2rect R2Rect_Area = r2rect
	(
		R2Pos,
		TELETYPE_CURSOR_WIDTH,
		pLetterFont -> GetHeight()
	);

	R2Rect_Clip . Clip
	(
		R2Rect_Area
	);

	if
	(
		R2Rect_Area . bHasArea()
	)
	{
		#if 0
		textprint
		(
			"TeletypeCursor R2Rect_Area = (%i,%i,%i,%i)\n",
			R2Rect_Area . x0,
			R2Rect_Area . y0,
			R2Rect_Area . x1,
			R2Rect_Area . y1
		);
		#endif

		R2Rect_Area . AlphaFill
		(
			255, // unsigned char R,
			255, // unsigned char G,
			255, // unsigned char B,
			(FixP_Alpha/256) // unsigned char translucency
		);
	}
	#endif
}

#endif // UseGadgets
