/*

	STRUTIL.C

	Created 13/11/97 by David Malcolm from Headhunter code: more carefully specified
	versions of the C string library functions, to act on ProjChars
	rather than chars

*/
#include <ctype.h>

#include "3dc.h"

#include "strutil.h"

#if 0
#include "hhfile.h"

#include "hhcolour.h"

#include "daveserr.h"

#include "inouttxt.h"
#include "menutxt.h"

#include "davelog.h"
#endif

#define UseLocalAssert Yes
#include "ourasert.h" 



/* VERSION DEFINES */
#define LogStringTables		No

#define MAX_ENTRIES_PER_STRING_TABLE (200)
#define LimitedStringLengths	Yes
#if LimitedStringLengths
#define MAX_STRING_LENGTH			(30000)
#endif

/* TYPE DEFINITIONS */
#if 0
static HHMCTC HHMCTS_Terminator={'\0',LogCol_OpaqueBlack};
#endif

/* EXPORTED GLOBALS */

#if 0
HHStringTable*	pHHST_UserInterface = NULL;
HHStringTable*	pHHST_MenuText = NULL;

HHMCTC* HHMCTS_Blank=&HHMCTS_Terminator;
#endif

/* EXPORTED FUNCTION PROTOTYPES */
#if 0
void STRUTIL_Init(void)
{
	pHHST_UserInterface=STRUTIL_LoadStringTable("hh/testdata/inout.txt",OnFailure_ExitSystem);

	LOCALASSERT(pHHST_UserInterface);
		/* 
			If error loading text file containing the error message text strings,
			there is no real way to generate an error message...
		*/
	LOCALASSERT
	(
		STRUTIL_GetNumEntries
		(
			pHHST_UserInterface
		)
		==
		NUM_IN_OUT_TXT_MESSAGES
	);

	/* If we reach here, user interface messages have been loaded Ok... */


	pHHST_MenuText = STRUTIL_LoadStringTable
	(
		"hh/testdata/menu.txt",
		OnFailure_ExitSystem
	);
	LOCALASSERT( pHHST_MenuText );
	LOCALASSERT
	(
		STRUTIL_GetNumEntries
		(
			pHHST_MenuText
		)
		==
		NUM_MENU_TXT_MESSAGES
	);

}

void STRUTIL_Destroy(void)
{
	LOCALASSERT(pHHST_UserInterface);

	STRUTIL_UnloadStringTable(pHHST_UserInterface);

	LOCALASSERT(pHHST_MenuText);

	STRUTIL_UnloadStringTable(pHHST_MenuText);

}
#endif
									
void STRUTIL_SC_WriteTerminator(ProjChar* pProjCh)
{
	/* Supplied as a function in case we switch to double-byte character sets */

	*pProjCh='\0';

}

#if 0
void STRUTIL_MC_WriteTerminator
(
	HHMCTC* pHHMCTC
)
{
	GLOBALASSERT(pHHMCTC);
	/* Supplied as a function in case we switch to double-byte character sets */

	pHHMCTC->Char='\0';
}
#endif


/* Ansi to HHTS conversion ********************************************/
OurBool STRUTIL_ANSI_To_ProjChar
(
	ProjChar* pProjCh_Out,
	unsigned int MaxSize, /* includes NULL-terminator; truncates after this */
	
	LPTSTR lptszANSI_In
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(pProjCh_Out);
		GLOBALASSERT(lptszANSI_In);
	}

	/* CODE */
	{
		/* For the moment: */
		return STRUTIL_SC_SafeCopy
		(
			pProjCh_Out,
			MaxSize,

			lptszANSI_In
		);
	}
}

OurBool STRUTIL_ProjChar_To_ANSI
(
	LPTSTR lptszANSI_Out,
	unsigned int MaxSize, /* includes NULL-terminator; truncates after this */

ProjChar* pProjCh_In		
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(lptszANSI_Out);
		GLOBALASSERT(pProjCh_In);
	}

	/* CODE */
	{
		/* For the moment: */
		return STRUTIL_SC_SafeCopy
		(
			lptszANSI_Out,
			MaxSize,

			pProjCh_In
		);
	}
}

unsigned int STRUTIL_SC_Strlen
(
	const ProjChar* String
)
{
	GLOBALASSERT(String);

	return strlen(String);
}

#if 0
unsigned int STRUTIL_MC_Strlen
(
	HHMCTC* MCString
)
{
	unsigned int Count=0;

	while (MCString->Char!='\0')
	{
		MCString++;
		Count++;
	}
	return Count;
}
#endif


ProjChar* STRUTIL_SC_StrCpy
(
	ProjChar* pProjCh_Dst,
	const ProjChar* pProjCh_Src
)
{
	GLOBALASSERT(pProjCh_Dst);
	GLOBALASSERT(pProjCh_Src);

	return (strcpy(pProjCh_Dst,pProjCh_Src));
}

void STRUTIL_SC_FastCat
(
	ProjChar* pProjCh_Dst,
	const ProjChar* pProjCh_Src_0,
	const ProjChar* pProjCh_Src_1			
)
{
	/* This function assumes the destination area is large enough;
	it copies Src0 followed by Src1 to the dest area.
	*/

	/* PRECONDITION */
	{
		GLOBALASSERT( pProjCh_Dst );
		GLOBALASSERT( pProjCh_Src_0 );
		GLOBALASSERT( pProjCh_Src_1 );
	}

	/* CODE */
	{
		while ( *pProjCh_Src_0 )
		{
			*( pProjCh_Dst++ ) = *( pProjCh_Src_0++ );
		}
		while ( *pProjCh_Src_1 )
		{
			*( pProjCh_Dst++ ) = *( pProjCh_Src_1++ );
		}

		/* Write terminator */
		*pProjCh_Dst = 0;
	}
}


OurBool STRUTIL_SC_Strequal
(
	const ProjChar* String1,
	const ProjChar* String2
)
{
#if 0
	DAVELOG("Comparing strings");
	DAVELOG(String1);
	DAVELOG(String2);
#endif

	while 
	(
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
	}

	return 
	(
		(*String1)
		==
		(*String2)
	);
}

OurBool STRUTIL_SC_Strequal_Insensitive
(
	const ProjChar* String1,
	const ProjChar* String2
)
{

	while 
	(
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
	}

	return 
	(
		(tolower(*String1))
		==
		(tolower(*String2))
	);
}


#if 0
void STRUTIL_MC_MakeMCTS
(
	HHMCTC* DestString,
	ProjChar* InputString,
	LogicalColour LogCol
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(DestString);
		GLOBALASSERT(InputString);
		GLOBALASSERT(LogCol<NUM_BASE_COLOURS);
	}

	/* CODE */
	{

		while (*InputString)
		{
			DestString->Char=*InputString;
			DestString->LogCol=LogCol;

			InputString++;
			DestString++;
		}

		STRUTIL_MC_WriteTerminator(DestString);
	}
}
#endif

void STRUTIL_SC_SafeCat
(
	ProjChar* pProjCh_Dst,
	unsigned int MaxSize,

	const ProjChar* pProjCh_Add
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(pProjCh_Dst);
		GLOBALASSERT(pProjCh_Add);
		GLOBALASSERT(MaxSize>0);
	}

	/* CODE */
	{
		unsigned int MaxNonTerminatingCharsToUse = (MaxSize - 1);

		while
		(
			(*pProjCh_Dst)
			&&
			(MaxNonTerminatingCharsToUse>0)
		)
		{
			pProjCh_Dst++;
			MaxNonTerminatingCharsToUse--;
		}

		while
		(
			(*pProjCh_Add)
			&&
			(MaxNonTerminatingCharsToUse>0)
		)
		{
			*pProjCh_Dst = *pProjCh_Add;

			pProjCh_Add++;
			pProjCh_Dst++;

			MaxNonTerminatingCharsToUse--;

			STRUTIL_SC_WriteTerminator(pProjCh_Dst);
		}
	}
}

size_t STRUTIL_SC_NumBytes
(
	const ProjChar* String
)
{
	return
	(
		sizeof(ProjChar)
		*
		(STRUTIL_SC_Strlen(String)+1)
	);
}

#if 0
size_t STRUTIL_MC_NumBytes(HHMCTC* MCString)
{
	return STRUTIL_MC_LengthToNumBytes
	(
		STRUTIL_MC_Strlen(MCString)
	);
}

size_t STRUTIL_MC_NumBytesFromSC
(
	ProjChar* pProjCh
)
{
	return STRUTIL_MC_LengthToNumBytes
	(
		STRUTIL_SC_Strlen(pProjCh)
	);	
}

size_t STRUTIL_MC_LengthToNumBytes(unsigned int Length)
{
	return
	(
		sizeof(HHMCTC)
		*
		(Length+1)
	);
}


void STRUTIL_MC_CatHHTSOntoMCTS
(
	HHMCTC* DestString,
	ProjChar* InputString,
	LogicalColour LogCol
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(DestString);
		GLOBALASSERT(InputString);
		GLOBALASSERT(LogCol<NUM_BASE_COLOURS);
	}

	/* CODE */
	{
		while (DestString->Char)
		{
			DestString++;
		}

		while (*InputString)
		{
			DestString->Char=*InputString;
			DestString->LogCol=LogCol;

			InputString++;
			DestString++;

			STRUTIL_MC_WriteTerminator(DestString);
		}
	}
}

void STRUTIL_MC_SafeMakeMCTS
(
	HHMCTC* DestString,
	unsigned int MaxSize,

	ProjChar* InputString,
	LogicalColour LogCol
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(DestString);
		GLOBALASSERT(InputString);
		GLOBALASSERT(LogCol<NUM_BASE_COLOURS);
		GLOBALASSERT(MaxSize>0);
	}

	/* CODE */
	{
		unsigned int MaxNonTerminatingCharsToUse = (MaxSize - 1);

		while
		(
			(*InputString)
			&&
			(MaxNonTerminatingCharsToUse > 0)
		)
		{
			DestString->Char=*InputString;
			DestString->LogCol=LogCol;

			InputString++;
			DestString++;
			MaxNonTerminatingCharsToUse--;
		}

		STRUTIL_MC_WriteTerminator(DestString);
	}
}

void STRUTIL_MC_SafeCatHHTSOntoMCTS
(
	HHMCTC* DestString,
	unsigned int MaxSize,

	ProjChar* AddString,
	LogicalColour LogCol
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(DestString);
		GLOBALASSERT(AddString);
		GLOBALASSERT(LogCol<NUM_BASE_COLOURS);
		GLOBALASSERT(MaxSize>0);
	}

	/* CODE */
	{
		unsigned int MaxNonTerminatingCharsToUse = (MaxSize - 1);

		while
		(
			(DestString->Char)
			&&
			(MaxNonTerminatingCharsToUse>0)
		)
		{
			DestString++;
			MaxNonTerminatingCharsToUse--;
		}

		while
		(
			(*AddString)
			&&
			(MaxNonTerminatingCharsToUse>0)
		)
		{
			DestString->Char=*AddString;
			DestString->LogCol=LogCol;

			AddString++;
			DestString++;

			MaxNonTerminatingCharsToUse--;

			STRUTIL_MC_WriteTerminator(DestString);
		}
	}
}


OurBool fValidHHMCTS(HHMCTC* MCString)
{
	GLOBALASSERT(MCString);

	{
		unsigned int CharCount=0;

		while 
		(
			(MCString->Char!='\0')
#if LimitedStringLengths
			&&
			(CharCount<MAX_STRING_LENGTH)
#endif
		)
		{
			if (MCString->LogCol>=NUM_BASE_COLOURS)
			{
				DAVELOG("Invalid colour in MC String");
				return No;			
			}
			MCString++;
		}

		#if LimitedStringLengths
		{
			if (CharCount>=MAX_STRING_LENGTH)
			{
				DAVELOG("MC String too long...");
				return No;
			}
			else
			{
				return Yes;
			}
		}
		#else
		{
			return Yes;
		}
		#endif

	}
}

HHMCTC STRUTIL_SC_To_MC
(
	ProjChar Char,
	LogicalColour LogCol
)
{
	HHMCTC ReturnVal;
		
	ReturnVal. Char			= Char;
	ReturnVal. LogCol	= LogCol;

	return ReturnVal;
}
#endif

OurBool STRUTIL_SC_SafeCopy
(
	ProjChar* pProjCh_Dst,
	unsigned int MaxSize,

	const ProjChar* pProjCh_Src
)
{
	GLOBALASSERT(pProjCh_Dst);
	GLOBALASSERT(MaxSize > 0);
	GLOBALASSERT(pProjCh_Src);

	{
		unsigned int MaxNonTerminatingCharsToCopy = (MaxSize - 1);

		while
		(
			(MaxNonTerminatingCharsToCopy > 0 )
			&&
			(*pProjCh_Src != 0)
		)
		{
			MaxNonTerminatingCharsToCopy--;
			*(pProjCh_Dst++) = *(pProjCh_Src++);
		}

		STRUTIL_SC_WriteTerminator(pProjCh_Dst);

		return ( STRUTIL_SC_fIsTerminator(pProjCh_Src) );
	}

}

#if 0
OurBool STRUTIL_MC_SafeCopy
(
	HHMCTC* pHHMCTC_Dst,
	unsigned int MaxSize,

	HHMCTC* pHHMCTC_Src

)
{
	/* PRECONDITION */
	{
		GLOBALASSERT(pHHMCTC_Dst);
		GLOBALASSERT(MaxSize > 0);
		GLOBALASSERT(pHHMCTC_Src);
	}

	/* CODE */
	{
		#if 1
		HHMCTC* pHHMCTC_StoredDst = pHHMCTC_Dst;
		#endif

		{
			unsigned int MaxNonTerminatingCharsToCopy = (MaxSize - 1);

			while
			(
				(MaxNonTerminatingCharsToCopy > 0 )
				&&
				(!STRUTIL_MC_fIsTerminator(pHHMCTC_Src))
			)
			{
				MaxNonTerminatingCharsToCopy--;
				*(pHHMCTC_Dst++) = *(pHHMCTC_Src++);
			}

			STRUTIL_MC_WriteTerminator(pHHMCTC_Dst);

				#if 0
				{
					char temp[256];
					sprintf
					(
						temp,
						"STRUTIL_MC_SafeCopy maxsize %i",
						MaxSize
					);
					DAVELOG(temp);

					{
						HHMCTC* pHHMCTC = pHHMCTC_StoredDst;
						char* pCh_Out = &temp[0];
						while ( pHHMCTC -> Char != '\0' )
						{
							*(pCh_Out++) = ( *(pHHMCTC++) ).Char;
						}
						*pCh_Out = '\0';
					}
					DAVELOG(temp);
				}
				#endif

			return ( STRUTIL_MC_fIsTerminator(pHHMCTC_Src) );

		}
	}

}
#endif

OurBool STRUTIL_SC_fIsTerminator
(
	const ProjChar* pProjCh
)
{
	GLOBALASSERT(pProjCh);
	return (*pProjCh == '\0');
}

#if 0
OurBool STRUTIL_MC_fIsTerminator(HHMCTC* pHHMCTC)
{
	GLOBALASSERT(pHHMCTC);
	return ((pHHMCTC->Char) == '\0');
}


HHMCTC* STRUTIL_MC_StrCpy
(
	HHMCTC* pHHMCTC_Dst,
	HHMCTC* pHHMCTC_Src
)
{
	GLOBALASSERT(pHHMCTC_Dst);
	GLOBALASSERT(pHHMCTC_Src);

	while
	(
		pHHMCTC_Src->Char != '\0'
	)
	{
		*(pHHMCTC_Dst++) = *(pHHMCTC_Src++);
	}
	pHHMCTC_Dst->Char = '\0';
}

HHMCTC* STRUTIL_MC_FastCat
(
	HHMCTC* pHHMCTC_Dst,
	HHMCTC* pHHMCTC_Src_0,
	HHMCTC* pHHMCTC_Src_1
)
{
	/* This function assumes the destination area is large enough;
	it copies Src0 followed by Src1 to the dest area.
	*/

	/* PRECONDITION */
	{
		GLOBALASSERT( pHHMCTC_Dst );
		GLOBALASSERT( pHHMCTC_Src_0 );
		GLOBALASSERT( pHHMCTC_Src_1 );
	}

	/* CODE */
	{
		while
		(
			(!STRUTIL_MC_fIsTerminator(pHHMCTC_Src_0))
		)
		{
			*(pHHMCTC_Dst++) = *(pHHMCTC_Src_0++);
		}

		while
		(
			(!STRUTIL_MC_fIsTerminator(pHHMCTC_Src_1))
		)
		{
			*(pHHMCTC_Dst++) = *(pHHMCTC_Src_1++);
		}

		STRUTIL_MC_WriteTerminator(pHHMCTC_Dst);

		return pHHMCTC_Dst;
	}
}


#endif
