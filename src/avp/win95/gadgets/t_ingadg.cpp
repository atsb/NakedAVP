/*******************************************************************
 *
 *    DESCRIPTION: 	t_ingadg.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 23/1/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "t_ingadg.hpp"

	#if UseGadgets
		#include "indexfnt.hpp"
		#include "coordstr.hpp"
		#include "inline.h"
		#include "trepgadg.hpp"
	#endif
	
	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/

/* Constants *******************************************************/
	#define I_Font_TextEntry (DATABASE_MESSAGE_FONT)
	#define WIDTH_OF_INSERTION_CARET (2)
	#define FIXP_SPEED_OF_FADEIN	((ONE_FIXED * 3) / 4)

	#define WIDTH_OF_TEXT_ENTRY_LINE	(TEXT_REPORT_MAX_W)

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
// class AVPTextInputState : public TextInputState
// public:
void AVPTextInputState :: Key_Up(void)
{
	#if SupportHistory
	History_SelectPrv();
	#else
	// empty for the moment
	#endif
}
void AVPTextInputState :: Key_Down(void)
{
	#if SupportHistory
	History_SelectNxt();
	#else
	// empty for the moment
	#endif
}

void AVPTextInputState :: Key_Tab(void)
{
	#if SupportCompletion
	Completion_SelectNxt();
	#endif
}

// AVP-specific processing for carriage return:
void AVPTextInputState :: ProcessCarriageReturn(void)
{
	SCString* pSCStr = &GetCurrentState();

	Clear();

	#if SupportHistory
	AddToHistory
	(
		*pSCStr
	);
	#endif

	pSCStr -> ProcessAnyCheatCodes();

	pSCStr -> R_Release();
}

void AVPTextInputState :: TextEntryError(void)
{
	// called e.g. when no more text can be typed, or no more deleted
	// may make a beep noise

	textprint("TextEntryError()\n");
		// replace with a beep?
}

// class TextEntryGadget : public Gadget
// public:
void TextEntryGadget :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	#if 0
	textprint
	(
		"TextEntryGadget :: Render at (%i,%i) clipped (%i,%i,%i,%i) alpha=%i\n",
		R2Pos . x,
		R2Pos . y,
		R2Rect_Clip . x0,
		R2Rect_Clip . y0,
		R2Rect_Clip . x1,
		R2Rect_Clip . y1,
		FixP_Alpha
	);
	#endif

	// If you're being rendered, then you ought to be fading in
	p666_FadeIn -> SetTarget_FixP
	(
		ONE_FIXED
	);

	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TextEntry );
	GLOBALASSERT( pLetterFont );

	// Calculate number of lines required:
	int NumberOfLines = 0;
	
	#if LimitedLineLength
	{
		NumberOfLines = 1;
		
		int widthLeft = WIDTH_OF_TEXT_ENTRY_LINE;

		const ProjChar* pProjCh = theState . GetProjChar();

		//go through the characters in the string , working out how much space is required
		while ( *pProjCh )
		{
			widthLeft -= pLetterFont->GetWidth(*pProjCh);
									
			pProjCh++;

			if (pLetterFont->GetMaxWidth()>=widthLeft)
			{
				// need to go to the next line:
				widthLeft = WIDTH_OF_TEXT_ENTRY_LINE;
				NumberOfLines++;
			}			
		}
		if (NumberOfLines<1) { NumberOfLines = 1; }
		if (NumberOfLines>5) { NumberOfLines = 5; }
	}
	#else
	{
		int TotalWidthGuess = theState . GetNumChars() * (pLetterFont -> GetMaxWidth()+1);

		if (TotalWidthGuess>0)
		{
			NumberOfLines = ( TotalWidthGuess / WIDTH_OF_TEXT_ENTRY_LINE )+1;
		}

		if (NumberOfLines<1) { NumberOfLines = 1; }
		if (NumberOfLines>5) { NumberOfLines = 5; }
	}
	#endif

	// Work out the area the gadget occupies, clipped with the passed clip area:
	r2rect R2Rect_Area
	(
		R2Pos,
		WIDTH_OF_TEXT_ENTRY_LINE,
		( pLetterFont -> GetHeight() * NumberOfLines )
	);

	R2Rect_Clip . Clip( R2Rect_Area );

	// Add alpha-channeled poly:
	{
		int translucency =
		(
			MUL_FIXED
			(
				p666_FadeIn -> GetCoord_FixP(),
				FixP_Alpha
			)
			>> 8
		);		

		if (translucency<0) translucency = 0;
		if (translucency>255) translucency = 255;

		R2Rect_Area . AlphaFill
		(
			0xbf, // unsigned char R,
			0xbf, // unsigned char G,
			0, // unsigned char B,
			translucency
		);
	}

	struct r2pos R2Pos_Char = R2Pos;
	int i = 0;

	#if LimitedLineLength
	{
		int X_EndOfLine = R2Pos.x + WIDTH_OF_TEXT_ENTRY_LINE;

		const ProjChar* pProjCh = theState . GetProjChar();

		while ( *pProjCh )
		{
			if ( i == theState . GetCursorPos() )
			{
				// Add cursor to the left of this character
				r2rect R2Rect_Cursor
				(
					R2Pos_Char,
					// r2size:
					(
						TextInputState :: bOverwrite()
						?
						(
							pLetterFont -> CalcSize( *pProjCh )
						)
						:
						r2size
						(
							WIDTH_OF_INSERTION_CARET,
							pLetterFont -> GetHeight()
						)
					)
				);

				R2Rect_Area . Clip( R2Rect_Cursor );

				if
				(
					R2Rect_Cursor . bValidPhys()
					&&
					R2Rect_Cursor . bHasArea()
				)
				{
					R2Rect_Cursor . AlphaFill
					(
						255, // unsigned char R,
						255, // unsigned char G,
						255, // unsigned char B,
						(FixP_Alpha * 2 / (256*3))// unsigned char translucency
					);
				}
			}

			pLetterFont -> RenderChar_Clipped
			(
				R2Pos_Char,
				R2Rect_Area,
				FixP_Alpha,
				*pProjCh // ProjChar ProjCh
			);

			i++;
			pProjCh++;

			if (R2Pos_Char.x+pLetterFont->GetMaxWidth()>=X_EndOfLine)
			{
				// wrap to next line:
				R2Pos_Char.x = R2Pos.x;
				R2Pos_Char.y += pLetterFont -> GetHeight();
			}			
		}
	}
	#else // LimitedLineLength
	{
		for
		(
			LIF<ProjChar> oi(&(List_ProjChar));
			!oi.done();
			oi.next()
		)
		{
			#if 1
			if ( i == theState . GetCursorPos() )
			{
				// Add cursor to the left of this character
				r2rect R2Rect_Cursor
				(
					R2Pos_Char,
					// r2size:
					(
						bOverwrite
						?
						(
							pLetterFont -> CalcSize( oi() )
						)
						:
						r2size
						(
							WIDTH_OF_INSERTION_CARET,
							pLetterFont -> GetHeight()
						)
					)
				);

				R2Rect_Area . Clip( R2Rect_Cursor );

				R2Rect_Cursor . AlphaFill
				(
					255, // unsigned char R,
					255, // unsigned char G,
					255, // unsigned char B,
					(FixP_Alpha * 2 / (256*3))// unsigned char translucency
				);
			}

			pLetterFont -> RenderString_Clipped
			(
				R2Pos_Char,
				R2Rect_Area,
				FixP_Alpha,
				oi() // ProjChar ProjCh
			);

			#else
			textprintXY
			(
				R2Pos_Char . x,
				R2Pos_Char . y,
				"%c",oi()
			);

			R2Pos_Char . x += 10;
			// LOCALISEME();
			#endif

			i++;
		}
	}
	#endif // LimitedLineLength

	if ( i == theState . GetCursorPos() )
	{
		// Add cursor after the end of the characters:
		r2rect R2Rect_Cursor
		(
			R2Pos_Char,
			r2size
			(
				(
					TextInputState :: bOverwrite()
					?
					pLetterFont -> GetMaxWidth()
					:
					WIDTH_OF_INSERTION_CARET
				),
				pLetterFont -> GetHeight()
			)
		);

		R2Rect_Area . Clip( R2Rect_Cursor );

		if
		(
			R2Rect_Cursor . bValidPhys()
			&&
			R2Rect_Cursor . bHasArea()
		)
		{
			R2Rect_Cursor . AlphaFill
			(
				255, // unsigned char R,
				255, // unsigned char G,
				255, // unsigned char B,
				(FixP_Alpha * 2 / (256*3))// unsigned char translucency
			);
		}
	}

	#if 0
	// For the moment, add the cursor on the line below...
	textprintXY
	(
		R2Pos . x + (CursorPos * 10),
		R2Pos . y + 10,
		"^%s",
		(
			bOverwrite
			?
			"(ovr)"
			:
			"(ins)"
		)
	);
	#endif

	// Diagnostics:
	#if 0
	{
		#if LimitedLineLength
		{
			textprint
			(
				"CursorPos=%i NumChars=%i\n",
				CursorPos,
				NumChars
			);
			
			for (unsigned int i=0;i<MAX_SIZE_INPUT;i++)
			{
				textprint
				(
					"ProjCh[%2i]=\'%c\' 0x%2x %s %s\n",
					i,
					ProjCh[i],
					ProjCh[i],
					(
						( i == CursorPos )
						?
						"   CPos"
						:
						"notCPos"
					),
					(
						( i == NumChars )
						?
						"   NumC"
						:
						"notNumC"
					)
				);
			}

		}
		#else
		{
			textprint
			(
				"CursorPos=%i\nList size=%i\n",
				CursorPos,
				List_ProjChar . size()
			);
		}
		#endif
	}
	#endif

}

void TextEntryGadget :: DontRender(void)
{
	// Make it fade out:

	p666_FadeIn -> SetTarget_FixP
	(
		0
	);
}


TextEntryGadget :: TextEntryGadget
(
) : Gadget
	(
		#if debug
		"TextEntryGadget"
		#endif
	),
	p666_FadeIn
	(
		new AcyclicFixedSpeedHoming
		(
			0, // int Int_InitialCoord,
			0, // int Int_TargetCoord,
			FIXP_SPEED_OF_FADEIN // int FixP_Speed
				// must be >= zero
		)
	),
	theState()
{
}


TextEntryGadget :: ~TextEntryGadget()
{
	delete p666_FadeIn;
}

#endif // UseGadgets

/* Internal function definitions ***********************************/
