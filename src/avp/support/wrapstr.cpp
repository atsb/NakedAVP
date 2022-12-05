/*******************************************************************
 *
 *    DESCRIPTION: 	wrapstr.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 6/8/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "wrapstr.hpp"
#include "strutil.h"

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
// namespace WordWrap
List<SCString*>* WordWrap :: DeprecatedMake
(
	const SCString& SCString_In,

	const IndexedFont& IndexedFnt_In,

	int W_FirstLine_In,
	int W_Subsequently_In
		// widths to wrap with.
		// They are different in order to support paragraph starts of various kinds
		// e.g. bulleting
)
{
	GLOBALASSERT( W_FirstLine_In >= 0 );
	GLOBALASSERT( W_Subsequently_In >= 0 );

	// The list to return is initially empty:
	List<SCString*>* pList_pSCString_Return = new List<SCString*>;

	#if 1
	{
		// Iterate through the string:
		ProjChar* pProjCh_I = SCString_In . pProjCh();

		ProjChar* pProjCh_StartOfLine = pProjCh_I;
		int CharsInLine = 0;

		ProjChar* pProjCh_StartOfNextLine = 0;
		int CharsInLine_Good = 0;
			// update this whenever you have a "good" wrap position i.e.
			// one where there's whitespace

		int W_Available = W_FirstLine_In;

		while
		(
			!STRUTIL_SC_fIsTerminator
			(
				pProjCh_I
			)
		)
		{
			// Determine if adding this character to the line being
			// formed will make the line too long to fit:
			if
			(
				IndexedFnt_In . CalcSize
				(
					pProjCh_StartOfLine,
					( CharsInLine + 1 )
				) . w
				>
				W_Available
			)
			{
				// It won't fit:
				if ( CharsInLine_Good > 0 )
				{
					GLOBALASSERT( pProjCh_StartOfNextLine );

					// If so, flush the current line by creating a new SCString
					// for it and start a new line with this as the first character
					pList_pSCString_Return -> add_entry_end
					(
						new SCString
						(
							pProjCh_StartOfLine,
							CharsInLine_Good
						)
					);

					pProjCh_StartOfLine = pProjCh_StartOfNextLine;
					CharsInLine = (CharsInLine - CharsInLine_Good );
					CharsInLine_Good = 0;
						// Note that we don't advance pProjCh_I, but the variant does
						// decrease since pProjCh_StartOfLine will advance, reducing the number
						// of characters left to add.

					W_Available = W_Subsequently_In;

					continue;
				}
				else
				{
					// Emergency:  what we have won't fit even by itself in the given
					// width.  Continue processing until you reach a "good" breaking point:
				}
			}

			// Otherwise, add this character to the line being formed:
			{
				if
				(
					bWhitespace( *pProjCh_I )
				)
				{
					// Whitespace character:
					if
					(
						(CharsInLine == 0)
					)
					{
						// Ignore leading whitespace in a line:
						pProjCh_StartOfLine++;
						pProjCh_I++;
					}
					else
					{
						// Add this character to the line being generated; it is an acceptable
						// breaking point:
						pProjCh_I++;
						CharsInLine++;

						CharsInLine_Good = CharsInLine;
						pProjCh_StartOfNextLine = pProjCh_I;						
					}
				}
				else if (*pProjCh_I=='\n')
				{
					pProjCh_I++;
					CharsInLine++;
					
					CharsInLine_Good = CharsInLine;
					pProjCh_StartOfNextLine = pProjCh_I;						

					pList_pSCString_Return -> add_entry_end
					(
						new SCString
						(
							pProjCh_StartOfLine,
							CharsInLine_Good
						)
					);

					pProjCh_StartOfLine = pProjCh_StartOfNextLine;
					CharsInLine = (CharsInLine - CharsInLine_Good );
					CharsInLine_Good = 0;
					W_Available = W_Subsequently_In;


				}
				else
				{
					// Non-whitespace character:

					// Add this character to the line being generated; it is not an acceptable
					// breaking point:
					CharsInLine++;
					pProjCh_I++;
				}
			}
		}

		// Flush the final line that was formed:
		pList_pSCString_Return -> add_entry_end
		(
			new SCString
			(
				pProjCh_StartOfLine,
				CharsInLine
			)
		);


	}
	#else
	{
		// Iterate through the string:
		ProjChar* pProjCh_I = SCString_In . pProjCh();
		ProjChar* pProjCh_StartOfLine = pProjCh_I;
		
		int CharsInLine = 0;

		int W_Available = W_FirstLine_In;

		while
		(
			!STRUTIL_SC_fIsTerminator
			(
				pProjCh_I
			)
		)
		{
			// Determine if adding this character to the line being
			// formed will make the line too long to fit:
			if
			(
				IndexedFnt_In . CalcSize
				(
					pProjCh_StartOfLine,
					( CharsInLine + 1 )
				) . w
				>
				W_Available
			)
			{
				// It won't fit:
				if ( CharsInLine > 0 )
				{
					// If so, flush the current line by creating a new SCString
					// for it and start a new line with this as the first character
					pList_pSCString_Return -> add_entry_end
					(
						new SCString
						(
							pProjCh_StartOfLine,
							CharsInLine
						)
					);

					pProjCh_StartOfLine = pProjCh_I;
					CharsInLine = 0;
						// Note that we don't advance pProjCh_I, but the variant does
						// decrease since pProjCh_StartOfLine will advance, reducing the number
						// of characters left to add.

					W_Available = W_Subsequently_In;
				}
				else
				{
					// Emergency:  the character we have won't fit even by itself in the given
					// width.  Make a line containing just this character (to avoid infinite loops):
					pList_pSCString_Return -> add_entry_end
					(
						new SCString
						(
							pProjCh_StartOfLine,
							1
						)
					);

					pProjCh_I++;

					pProjCh_StartOfLine = pProjCh_I;
					CharsInLine = 0;

					W_Available = W_Subsequently_In;
				}
			}
			else
			{
				// Otherwise, add this character to the line being formed:
				if
				(
					(CharsInLine == 0)
					&&
					bWhitespace( *pProjCh_I )
				)
				{
					// Ignore leading whitespace in a line:
					pProjCh_StartOfLine++;
				}
				else
				{
					// Add this character to the line being generated:
					CharsInLine ++;
				}

				pProjCh_I++;

			}
		}

		// Flush the final line that was formed:
		pList_pSCString_Return -> add_entry_end
		(
			new SCString
			(
				pProjCh_StartOfLine,
				CharsInLine
			)
		);


	}
	#endif

	return pList_pSCString_Return;
}

int WordWrap :: bWhitespace( ProjChar ProjCh )
{
	// LOCALISEME();
	return ( ProjCh == ' ');
}

int WordWrap :: bWithinWord( ProjChar* pProjCh_Test )
{
	GLOBALASSERT( pProjCh_Test );

	if ( bWhitespace(*pProjCh_Test) )
	{
		return No;
	}

	GLOBALASSERT
	(
		!STRUTIL_SC_fIsTerminator
		(
			pProjCh_Test
		)
	);

	return 
	(
		!bWhitespace( *(pProjCh_Test++) )
	);
		// i.e you're within a word iff
		// neither you nor the next char are whitespace
}


/* Internal function definitions ***********************************/
