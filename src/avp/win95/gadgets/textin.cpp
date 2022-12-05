/*******************************************************************
 *
 *    DESCRIPTION: 	textin.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 21/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include <ctype.h>

#include "3dc.h"
#include "textin.hpp"

	#if SupportCompletion
		#include "conssym.hpp"
	#endif

	#if LimitedLineLength
		#include "strutil.h"
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
	/*static*/ OurBool TextInputState :: bOverwrite_Val = No;

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
TextInputState :: ~TextInputState()
{
	#if SupportHistory
	while ( List_pSCString_History. size() > 0)
	{
		List_pSCString_History . first_entry() -> R_Release();

		List_pSCString_History . delete_first_entry();		
	}
	#endif
}

SCString& TextInputState :: GetCurrentState(void)
{
	// returns a ref to a copy of the "string under construction" in its current state

	#if LimitedLineLength
	return
	(
		*(
			new SCString( &ProjCh[0] )
		)
	);
	#else
	return 
	(
		*(
			new SCString
			(
				List_ProjChar
			)
		)
	);
	#endif
}

void TextInputState :: CharTyped
(
	char Ch
		// note that this _is _ a char
)
{
	// Need to spot special characters...
	// Convert to ProjChars???

	// LOCALISEME();

	if ( Ch == '\r' )
	{
		ProcessCarriageReturn();
		// Special processing for carriage return:
		return;
	}

	// Reject certain characters using <CTYPE.H>:
	#if 0
	if
	(
		( !isgraph(Ch) )
		&&
		( Ch != ' ' )
	)
	{
		return;
	}

	// Should only be printable characters, with only SPACE accepted
	// for marking space (i.e. no tabs, carriage returns etc.)

	// Potentially convert to upper case:
	if ( bForceUpperCase_Val )
	{
		Ch = toupper(Ch);
	}
	#endif
	if (Ch<32) return;
	// LOCALISEME();
	// we assume ProjChar == char at about this point...

	// It also ought not to be a null character:
	GLOBALASSERT( Ch );

	if ( Ch == ' ' )
	{
		// When a space is typed, check to see if there's an expansion string
		// (and continue processing):
		TextExpansion :: TestForExpansions
		(
			this
		);
	}

	
	// add to list at cursor point...
	#if LimitedLineLength
	{
		GLOBALASSERT( CursorPos >= 0 );
		GLOBALASSERT( CursorPos <= NumChars );

		GLOBALASSERT( 0 == ProjCh[ NumChars ] );
		GLOBALASSERT( NumChars <= MAX_LENGTH_INPUT );

		if ( bOverwrite() )
		{
			if
			(
				bOvertypeAt
				(
					Ch, // ProjChar ProjCh_ToInsert,
					CursorPos // int Pos
				)
			)
			{
				CursorPos++;
			}
			else
			{
				// can't overtype; the line is full
				// some kind of error beep?
				TextEntryError();			
			}			
		}
		else
		{
			if
			(
				bInsertAt
				(
					Ch, // ProjChar ProjCh_ToInsert,
					CursorPos // int Pos
				)
			)
			{
				// cursor pos is updated internally by the fn call
			}
			else
			{
				// can't insert; the line is full
				// some kind of error beep?
				TextEntryError();			
			}
		}
		#if 0
		if ( CursorPos == NumChars )
		{
			GLOBALASSERT( 0 == ProjCh[ CursorPos ] );

			// insert character at the end if you can:
			if ( NumChars < MAX_LENGTH_INPUT )
			{
				NumChars++;
				ProjCh[ CursorPos++ ] = Ch;
				ProjCh[ CursorPos ] = 0;
			}
			else
			{
				// can't insert; the line is full
				// some kind of error beep?
				TextEntryError();
			}
		}
		else
		{
			GLOBALASSERT( 0 != ProjCh[ CursorPos ] );

			if ( bOverwrite )
			{
				// overwrite character at this point in the list
				// and move cursor to the right
				ProjCh[ CursorPos++ ] = Ch;
			}
			else
			{
				// Try to insert the character within the string; is there space?
				if ( NumChars < MAX_LENGTH_INPUT )
				{
					// Advance all to the right of the cursor, starting with the final
					// character in the string:
					
					for
					(
						int i=NumChars;
						i>=CursorPos;
						i--
					)
					{
						ProjCh[ i + 1 ] = ProjCh[ i ];
					}

					ProjCh[ CursorPos++ ] = Ch;
					NumChars++;
				}
				else
				{
					// can't insert; the line is full
					// some kind of error beep?
					TextEntryError();
				}
			}
		}
		#endif
	}
	#else // LimitedLineLength
	{
		GLOBALASSERT( CursorPos <= List_ProjChar . size() );

		if ( CursorPos ==  List_ProjChar . size() )
		{
			// insert character at the end
			List_ProjChar . add_entry_end( Ch );

			CursorPos++;
		}
		else
		{
			GLOBALASSERT( CursorPos < List_ProjChar . size() );

			if ( bOverwrite )
			{
				// overwrite character at this point in the list
				// and move cursor to the right
			}
			else
			{
				#if 0
				// insert character inside the list
				// and move cursor to the right
				List_ProjChar[ CursorPos ] . 
				
				add_entry_end( Ch );
				#endif
			}
		}
	}
	#endif // LimitedLineLength
}

void TextInputState :: Key_Backspace(void)
{
	if ( CursorPos > 0)
	{
		-- CursorPos;

		Key_Delete();
	}
	else
	{	
		TextEntryError();
	}
}
void TextInputState :: Key_End(void)
{
	// Put cursor to far right of input text:
	#if LimitedLineLength
	if (NumChars > 0)
	{
		CursorPos = NumChars;

	}
	else
	{
		GLOBALASSERT( CursorPos == 0);
	}
	#else
	CursorPos = List_ProjChar . size();
	#endif
}
void TextInputState :: Key_Home(void)
{
	// Put cursor to far left of input text:
	CursorPos = 0;
}
void TextInputState :: Key_Left(void)
{
	// Move cursor to left, if possible:
	if ( CursorPos > 0)
	{
		-- CursorPos;
	}
}
void TextInputState :: Key_Right(void)
{
	// Move cursor to right, if possible:
	#if LimitedLineLength
	if ( CursorPos < NumChars )
	{
		++ CursorPos;

	}
	#else
	if ( CursorPos < List_ProjChar . size() )
	{
		++ CursorPos;
	}
	#endif
}
void TextInputState :: Key_Delete(void)
{
	#if LimitedLineLength
	{
		if
		(
			( NumChars > 0 )
			&&
			( CursorPos < NumChars )
		)
		{
			DeleteAt( CursorPos );
		}
		else
		{
			// No characters to delete:
			TextEntryError();
		}
	}
	#else
	{
		// empty for the moment
	}
	#endif
}

/*static*/ void TextInputState :: ToggleTypingMode(void)
{
	// toggles overwrite/insert mode
	bOverwrite_Val = !bOverwrite_Val;

	// UNIMPLEMENTED: Notification of this as a "cursor change"
}

// Protected methods:
TextInputState :: TextInputState
(
	OurBool bForceUpperCase,
	char* pProjCh_Init
) :
	#if LimitedLineLength
	NumChars(0),
	#else
	List_ProjChar(),
	#endif
	#if SupportHistory
	List_pSCString_History(),
	pSCString_CurrentHistory(NULL),
	#endif
	#if SupportAutomation
	ManualPos(0),
	#endif
	#if SupportCompletion
	pConsoleSym_CurrentCompletion(NULL),
	#endif
	CursorPos(0),
	bForceUpperCase_Val( bForceUpperCase )
{
	GLOBALASSERT( pProjCh_Init );

	#if LimitedLineLength
	{
		STRUTIL_SC_SafeCopy
		(
			&ProjCh[0],
			MAX_SIZE_INPUT,	// unsigned int MaxSize,

			pProjCh_Init
		);

		// Set number of characters from the internal value rather than
		// from the input in case of truncation:
		NumChars = STRUTIL_SC_Strlen(&ProjCh[0]);
		GLOBALASSERT( NumChars < MAX_LENGTH_INPUT );
	}
	#else
		#error Unimplemented
	#endif
}

void TextInputState :: TryToInsertAt
(
	SCString* pSCString_ToInsert,
	int Pos
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSCString_ToInsert );

		GLOBALASSERT( Pos >= 0 );
		
		#if LimitedLineLength
		GLOBALASSERT( Pos <= MAX_LENGTH_INPUT );
		#endif
	}

	/* CODE */
	{
		#if LimitedLineLength
		{
			ProjChar* pProjCh_I = pSCString_ToInsert -> pProjCh();

			while
			(
				( *pProjCh_I )
				&&
				( Pos < MAX_LENGTH_INPUT )
			)
			{
				bInsertAt
				(
					*(pProjCh_I++),
					Pos++
				);
			}
		}
		#else
		{
			#error Not implemented
		}
		#endif
	}
}

int TextInputState :: bOvertypeAt
(
	ProjChar ProjCh_In,
	int Pos_Where
)
{
	// return value: was overtype succesful?

	/* PRECONDITION */
	{
		// Function parameter validity:
			GLOBALASSERT( ProjCh_In );
				// mustn't be a null terminator  

			GLOBALASSERT( Pos_Where >= 0 );
			
			#if LimitedLineLength
			GLOBALASSERT( Pos_Where <= MAX_LENGTH_INPUT );
			#endif

		// The data-representation invariant:
			#if LimitedLineLength
			GLOBALASSERT( CursorPos >= 0 );
			GLOBALASSERT( CursorPos <= NumChars );

			GLOBALASSERT( 0 == ProjCh[ NumChars ] );
			GLOBALASSERT( NumChars <= MAX_LENGTH_INPUT );
			#endif
	}

	/* CODE */
	{
		#if LimitedLineLength
		{
			if ( Pos_Where == NumChars )
			{
				GLOBALASSERT( 0 == ProjCh[ Pos_Where ] );

				// insert character at the end if you can:
				if ( NumChars < MAX_LENGTH_INPUT )
				{
					NumChars++;
					ProjCh[ Pos_Where ] = ProjCh_In;
					ProjCh[ Pos_Where+1 ] = 0;

					#if SupportAutomation
					FullyManual();
					#endif

					return Yes;
				}
				else
				{
					// can't; the line is full
					return No;
				}
			}
			else
			{
				GLOBALASSERT( 0 != ProjCh[ Pos_Where ] );

				// overwrite character at this point in the list
				ProjCh[ Pos_Where ] = ProjCh_In;

				#if SupportAutomation
				FullyManual();
				#endif

				return Yes;
			}
		}
		#else
		{
			#error Not implemented
		}
		#endif
	}
}


int TextInputState :: bInsertAt
(
	ProjChar ProjCh_In,
	int Pos_Where
)
{
	// return value: was insertion succesful?

	/* PRECONDITION */
	{
		// Function parameter validity:
			GLOBALASSERT( ProjCh_In );
				// mustn't be a null terminator  

			GLOBALASSERT( Pos_Where >= 0 );
			
			#if LimitedLineLength
			GLOBALASSERT( Pos_Where <= MAX_LENGTH_INPUT );
			#endif

		// The data-representation invariant:
			#if LimitedLineLength
			GLOBALASSERT( CursorPos >= 0 );
			GLOBALASSERT( CursorPos <= NumChars );

			GLOBALASSERT( 0 == ProjCh[ NumChars ] );
			GLOBALASSERT( NumChars <= MAX_LENGTH_INPUT );
			#endif
	}

	/* CODE */
	{
		#if LimitedLineLength
		{
			if ( Pos_Where == NumChars )
			{
				GLOBALASSERT( 0 == ProjCh[ Pos_Where ] );

				// insert character at the end if you can:
				if ( NumChars < MAX_LENGTH_INPUT )
				{
					NumChars++;
					ProjCh[ Pos_Where ] = ProjCh_In;
					ProjCh[ Pos_Where+1 ] = 0;

					// The cursor may also get dragged along:
					if ( CursorPos >= Pos_Where )
					{
						CursorPos++;
					}

					#if SupportAutomation
					FullyManual();
					#endif

					return Yes;
				}
				else
				{
					// can't; the line is full
					return No;
				}
			}
			else
			{
				GLOBALASSERT( 0 != ProjCh[ Pos_Where ] );

				// Try to insert the character within the string; is there space?
				if ( NumChars < MAX_LENGTH_INPUT )
				{
					// Advance all to the right of the cursor, starting with the final
					// character in the string:
					
					for
					(
						int i=NumChars;
						i>=Pos_Where;
						i--
					)
					{
						ProjCh[ i + 1 ] = ProjCh[ i ];
					}

					ProjCh[ Pos_Where ] = ProjCh_In;
					NumChars++;

					// The cursor may also get dragged along:
					if ( CursorPos >= Pos_Where )
					{
						CursorPos++;
					}

					#if SupportAutomation
					FullyManual();
					#endif

					return Yes;
				}
				else
				{
					// can't insert; the line is full
					return No;
				}
			}
		}
		#else
		{
			#error Not implemented
		}
		#endif
	}
}

void TextInputState ::  DeleteAt( int Pos )
{
	#if LimitedLineLength
	{
		GLOBALASSERT( NumChars > 0 );
		GLOBALASSERT( Pos < NumChars );

		// Move all characters to the right of the deletion point one space to the left
		// (this will overwrite the character at the deletion point )

		for
		(
			int i=Pos;
			i<NumChars;
			i++
		)
		{
			ProjCh[ i ] = ProjCh[ i + 1 ];
		}

		NumChars--;			

		// The cursor may also get dragged along:
		if ( CursorPos > Pos )
		{
			CursorPos--;
		}		

		#if SupportAutomation
		FullyManual();
		#endif

	}
	#else
	{
		#error Not implemented
	}
	#endif
}

void TextInputState :: Clear(void)
{
	#if LimitedLineLength
	NumChars = 0;
	ProjCh[0] = 0;
	#else
	while ( List_ProjChar . size() > 0 )
	{
		List_ProjChar . delete_first_entry();
	}
	#endif

	CursorPos = 0;

	#if SupportAutomation
	FullyManual();
	#endif
}

void TextInputState :: SetString
(
	SCString& SCString_ToUse
)
{
	// does not affect ManualPos()

	STRUTIL_SC_SafeCopy
	(
		&ProjCh[0],
		MAX_SIZE_INPUT,	// unsigned int MaxSize,

		SCString_ToUse . pProjCh()
	);

	if
	(
		SCString_ToUse . GetNumChars() < MAX_SIZE_INPUT
	)
	{
		NumChars = SCString_ToUse . GetNumChars();
	}	
	else
	{
		NumChars = MAX_LENGTH_INPUT;
	}

	CursorPos = NumChars;
}

#if SupportHistory
void TextInputState :: History_SelectNxt(void)
{
	textprint("TextInputState :: History_SelectNxt()\n");

	if ( List_pSCString_History . size() > 0)
	{
		// Iterate through until we find the next match of all the manually typed
		// characters with this history position, or we get back to where we started...
		SCString* pSCString_History_New = GetNxtMatchingHistory();

		if ( pSCString_History_New )
		{
			pSCString_CurrentHistory = pSCString_History_New;

			// Set to the chosen historic string:
			SetString
			(
				*pSCString_CurrentHistory // SCString& SCString_ToUse
			);
		}
		else
		{
			TextEntryError();
		}
	}
	else
	{
		// No history available:
		GLOBALASSERT( NULL == pSCString_CurrentHistory );

		TextEntryError();
	}
}

void TextInputState :: History_SelectPrv(void)
{
	textprint("TextInputState :: History_SelectPrv()\n");

#if 1
	if ( List_pSCString_History . size() > 0)
	{
		// Iterate through until we find the next match of all the manually typed
		// characters with this history position, or we get back to where we started...
		SCString* pSCString_History_New = GetPrvMatchingHistory();

		if ( pSCString_History_New )
		{
			pSCString_CurrentHistory = pSCString_History_New;

			// Set to the chosen historic string:
			SetString
			(
				*pSCString_CurrentHistory // SCString& SCString_ToUse
			);
		}
		else
		{
			TextEntryError();
		}
	}
	else
	{
		// No history available:
		GLOBALASSERT( NULL == pSCString_CurrentHistory );

		TextEntryError();
	}
#else
	if ( List_pSCString_History . size() > 0)
	{
		// Are we already cycling through the list of history:
		if ( pSCString_CurrentHistory )
		{
			if
			(
				pSCString_CurrentHistory == List_pSCString_History . first_entry()
			)
			{
				pSCString_CurrentHistory = List_pSCString_History . last_entry();
			}
			else
			{
				pSCString_CurrentHistory = List_pSCString_History . prev_entry( pSCString_CurrentHistory );
			}
		}
		else
		{
			pSCString_CurrentHistory = List_pSCString_History . last_entry();
		}

		// Set to the chosen historic string:
		SetString
		(
			*pSCString_CurrentHistory // SCString& SCString_ToUse
		);
	}
	else
	{
		// No history available:
		GLOBALASSERT( NULL == pSCString_CurrentHistory );

		TextEntryError();
	}
#endif

}

void TextInputState :: AddToHistory
(
	SCString& SCString_ToAdd
)
{
	if ( SCString_ToAdd . GetNumChars() < 1)
	{
		// Reject adding empty strings to the history
		return;
	}
	
	SCString* StringCopy = new SCString(SCString_ToAdd.pProjCh());

	List_pSCString_History . add_entry_end( StringCopy );

	if ( List_pSCString_History . size() > MAX_LINES_HISTORY )
	{
		List_pSCString_History . first_entry() -> R_Release();
		List_pSCString_History . delete_first_entry();
	}

	pSCString_CurrentHistory = NULL;
}
#endif

#if SupportAutomation
void TextInputState :: FullyManual(void)
{
	ManualPos = NumChars;

	#if SupportHistory
	pSCString_CurrentHistory = NULL;
	#endif

	#if SupportCompletion
	pConsoleSym_CurrentCompletion = NULL;
	#endif
}
OurBool TextInputState :: bManualMatch
(
	ProjChar* pProjCh
) const
{
	/* Returns true iff there's a match with the manually-typed prefix of
	the current state string and the input comparison string
	*/

	GLOBALASSERT( pProjCh );

	int Count = ManualPos;

	const ProjChar* String1 = pProjCh;
	const ProjChar* String2 = &ProjCh[0];


	while 
	(
		( Count > 0 )
		&&
		(*String1!='\0')
		&&
		(*String2!='\0')
	)
	{
		if
		(
			(*String1)
			!=
			(*String2)
		)
		{
			return No;
		}
		String1++;
		String2++;
		Count--;
	}

	if ( Count > 0 )
	{
		// One or more of the strings has terminated...
		return
		(
			(*String1)
			==
			(*String2)
		);
	}
	else
	{
		// There was a match in the first n characters...
		return Yes;
	}

}
OurBool TextInputState :: bManualMatchInsensitive
(
	ProjChar* pProjCh
) const
{
	/* Returns true iff there's a match with the manually-typed prefix of
	the current state string and the input comparison string
	*/

	GLOBALASSERT( pProjCh );

	int Count = ManualPos;

	const ProjChar* String1 = pProjCh;
	const ProjChar* String2 = &ProjCh[0];


	while 
	(
		( Count > 0 )
		&&
		(*String1!='\0')
		&&
		(*String2!='\0')
	)
	{
		if
		(
			(tolower(*String1))
			!=
			(tolower(*String2))
		)
		{
			return No;
		}
		String1++;
		String2++;
		Count--;
	}

	if ( Count > 0 )
	{
		// One or more of the strings has terminated...
		return
		(
			(tolower(*String1))
			==
			(tolower(*String2))
		);
	}
	else
	{
		// There was a match in the first n characters...
		return Yes;
	}

}

#endif

#if SupportCompletion
void TextInputState :: Completion_SelectNxt(void)
{
	#if 1
	textprint("TextInputState :: Completion_SelectNxt()\n");
	#endif

	ConsoleSymbol* pConsoleSym_Completion_New = GetNxtMatchingCompletion();

	if ( pConsoleSym_Completion_New )
	{
		pConsoleSym_CurrentCompletion = pConsoleSym_Completion_New;

		// Set to the chosen completion string:
		SetString
		(
			*(pConsoleSym_Completion_New->GetpSCString()) // SCString& SCString_ToUse
		);
	}
	else
	{
		TextEntryError();
	}
}
void TextInputState :: Completion_SelectPrv(void)
{
	#if 1
	textprint("TextInputState :: Completion_SelectPrv()\n");
	#endif

	ConsoleSymbol* pConsoleSym_Completion_New = GetPrvMatchingCompletion();

	if ( pConsoleSym_Completion_New )
	{
		pConsoleSym_CurrentCompletion = pConsoleSym_Completion_New;

		// Set to the chosen completion string:
		SetString
		(
			*(pConsoleSym_Completion_New->GetpSCString()) // SCString& SCString_ToUse
		);
	}
	else
	{
		TextEntryError();
	}

}
#endif


#if SupportHistory
// private:
SCString* TextInputState :: GetNxtMatchingHistory(void) const
{
	SCString* pSCString_Return = NULL;

	if ( pSCString_CurrentHistory )
	{
		// Find next matching one:
		SCString* pSCString_I = pSCString_CurrentHistory;

		while ( 1 )
		{
			// Advance to next entry (in a circular fashion)
			if
			(
				pSCString_I == List_pSCString_History . last_entry()
			)
			{
				pSCString_I = List_pSCString_History . first_entry();
			}
			else
			{
				pSCString_I = List_pSCString_History . next_entry( pSCString_I );
			}

			// Break if you've wrapped around:
			if ( pSCString_I == pSCString_CurrentHistory )
			{
				break;
			}
			else
			{
				// Check for a match:
				if
				(
					bManualMatchInsensitive( pSCString_I -> pProjCh() )
				)
				{
					pSCString_Return = pSCString_I;
					break;
				}
			}
		}

	}
	else
	{
		// Find first matching one:
		CLIF<SCString*> oi(&List_pSCString_History);

		while ( 1 )
		{
			if ( oi . done() )
			{
				break;
			}

			if
			(
				bManualMatchInsensitive( oi() -> pProjCh() )
			)
			{
				pSCString_Return = oi();
				break;
			}
			else
			{
				oi . next();
			}
		}
	}

	return pSCString_Return;
}

SCString* TextInputState :: GetPrvMatchingHistory(void) const
{
	SCString* pSCString_Return = NULL;

	if ( pSCString_CurrentHistory )
	{
		// Find prev matching one:
		SCString* pSCString_I = pSCString_CurrentHistory;

		while ( 1 )
		{
			// Back to prev entry (in a circular fashion)
			if
			(
				pSCString_I == List_pSCString_History . first_entry()
			)
			{
				pSCString_I = List_pSCString_History . last_entry();
			}
			else
			{
				pSCString_I = List_pSCString_History . prev_entry( pSCString_I );
			}

			// Break if you've wrapped around:
			if ( pSCString_I == pSCString_CurrentHistory )
			{
				break;
			}
			else
			{
				// Check for a match:
				if
				(
					bManualMatchInsensitive( pSCString_I -> pProjCh() )
				)
				{
					pSCString_Return = pSCString_I;
					break;
				}
			}
		}

	}
	else
	{
		// Find final matching one:
		CLIB<SCString*> oi(&List_pSCString_History);

		while ( 1 )
		{
			if ( oi . done() )
			{
				break;
			}

			if
			(
				bManualMatchInsensitive( oi() -> pProjCh() )
			)
			{
				pSCString_Return = oi();
				break;
			}
			else
			{
				oi . next();
			}
		}
	}

	return pSCString_Return;
}
#endif

#if SupportCompletion
// private:
ConsoleSymbol* TextInputState :: GetNxtMatchingCompletion(void) const
{
	ConsoleSymbol* pConsoleSym_Return = NULL;

	if ( pConsoleSym_CurrentCompletion )
	{
		GLOBALASSERT( ConsoleSymbol :: List_pConsoleSym . contains(pConsoleSym_CurrentCompletion ) );

		// Find next matching one:
		ConsoleSymbol* pConsoleSym_I = pConsoleSym_CurrentCompletion;

		while ( 1 )
		{
			// Advance to next entry (in a circular fashion)
			if
			(
				pConsoleSym_I == ConsoleSymbol :: List_pConsoleSym . last_entry()
			)
			{
				pConsoleSym_I = ConsoleSymbol :: List_pConsoleSym . first_entry();
			}
			else
			{
				pConsoleSym_I = ConsoleSymbol :: List_pConsoleSym . next_entry( pConsoleSym_I );
			}

			// Break if you've wrapped around:
			if ( pConsoleSym_I == pConsoleSym_CurrentCompletion )
			{
				break;
			}
			else
			{
				// Check for a match:
				if
				(
					pConsoleSym_I->ThisIsACheat
					?
					bManualMatch( pConsoleSym_I -> GetpSCString() -> pProjCh() )
					:
					bManualMatchInsensitive( pConsoleSym_I -> GetpSCString() -> pProjCh() )
				)
				{
					pConsoleSym_Return = pConsoleSym_I;
					break;
				}
			}
		}

	}
	else
	{
		// Find first matching one:
		CLIF<ConsoleSymbol*> oi(&ConsoleSymbol :: List_pConsoleSym);

		while ( 1 )
		{
			if ( oi . done() )
			{
				break;
			}

			if
			(
				oi()->ThisIsACheat
				?
				bManualMatch( oi() -> GetpSCString() -> pProjCh() )
				:
				bManualMatchInsensitive( oi() -> GetpSCString() -> pProjCh() )
			)
			{
				pConsoleSym_Return = oi();
				break;
			}
			else
			{
				oi . next();
			}
		}
	}

	return pConsoleSym_Return;
}
ConsoleSymbol* TextInputState :: GetPrvMatchingCompletion(void) const
{
	return NULL;
		// don't implement until you do a full rewrite with templates for circular
		// iteration through lists...
}
#endif


/* Internal function definitions ***********************************/
#if 0

		while (1)
		{
			// Check for match:
			if
			(
				bManualMatch
				(
					pSCString_I -> pProjCh()
				)
			)
			{
				// acceptable
				break;
			}
			else
			{	// not acceptable; try next
			}

			// Find next:
		}


#endif
