/*******************************************************************
 *
 *    DESCRIPTION: 	consvar.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 17/12/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "consvar.hpp"
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
	/*static*/ List <ConsoleVariable*> ConsoleVariable :: List_pConsoleVar;

/* Internal type definitions ***************************************/
	class ConsoleVariable_Simple_Int : public ConsoleVariable
	{
	public:
		ConsoleVariable_Simple_Int
		(
			int& Value_ToUse,
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int MinVal_New,
			int MaxVal_New,
			OurBool Cheat = FALSE

		);

		int GetValue(void) const;
		void SetValue(int Val_New);
		void SetValue(float Val_New);

	private:
		SCString* MakeRangeString(void);
		SCString* MakeValueString(int Val);

	private:
		int& theValue;
	};

	class ConsoleVariable_Simple_FixP : public ConsoleVariable
	{
	public:
		ConsoleVariable_Simple_FixP
		(
			int& Value_ToUse,
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int MinVal_New,
			int MaxVal_New,
			OurBool Cheat = FALSE

		);

		int GetValue(void) const;
		void SetValue(int Val_New);
		void SetValue(float Val_New);

	private:
		SCString* MakeRangeString(void);
		SCString* MakeValueString(int Val);

		static float FixP2Float(int FixP)
		{
			return
			(
				((float) FixP) / (ONE_FIXED)
			);
		}

	private:
		int& theValue;
	};

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class ConsoleVariable
// Factory method:
/*static*/ ConsoleVariable* ConsoleVariable :: MakeSimpleConsoleVariable_Int
(
	int& Value_ToUse,
	ProjChar* pProjCh_Symbol_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int MinVal_New,
	int MaxVal_New,
	OurBool Cheat

)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pProjCh_Symbol_ToUse );
		GLOBALASSERT( pProjCh_Description_ToUse );
	}

	/* CODE */
	{
		return new ConsoleVariable_Simple_Int
		(
			Value_ToUse,
			pProjCh_Symbol_ToUse,
			pProjCh_Description_ToUse,
			MinVal_New,
			MaxVal_New,
			Cheat
		);
	}
}

/*static*/ ConsoleVariable* ConsoleVariable :: MakeSimpleConsoleVariable_FixP
(
	int& Value_ToUse,
	ProjChar* pProjCh_Symbol_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int MinVal_New,
	int MaxVal_New,
	OurBool Cheat

)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pProjCh_Symbol_ToUse );
		GLOBALASSERT( pProjCh_Description_ToUse );
	}

	/* CODE */
	{
		return new ConsoleVariable_Simple_FixP
		(
			Value_ToUse,
			pProjCh_Symbol_ToUse,
			pProjCh_Description_ToUse,
			MinVal_New,
			MaxVal_New,
			Cheat
		);
	}
}

ConsoleVariable :: ~ConsoleVariable()
{
	pSCString_Description ->R_Release();

	// remove from the list
    List_pConsoleVar . delete_entry
    (
    	this
    );
}

/*static*/ OurBool ConsoleVariable :: Process( ProjChar* pProjCh_In )
{
	// used for proccesing input text.  Could decide that the user
	// was requesting the value of a variable, or was setting a new
	// value etc; if so, acts accordingly.
	// return value = was any processing performed?

	// Check to see if there's a match between the entire
	// input string and each console command
	{
		for
		(
			LIF<ConsoleVariable*> oi(&List_pConsoleVar);
			!oi.done();
			oi.next()
		)
		{
			if
			(
				oi() -> ThisIsACheat
				?
				STRUTIL_SC_Strequal //case sensitive comparisons for cheats
				(
					pProjCh_In,
					oi() -> pSCString_Symbol -> pProjCh()
				)
				:
				STRUTIL_SC_Strequal_Insensitive
				(
					pProjCh_In,
					oi() -> pSCString_Symbol -> pProjCh()
				)
			)
			{
				oi() -> Display();
				return Yes;
			}
		}				
	}

	// Otherwise check to see if there's a match between the front
	// of the console command up to the first space (if any)
	// with the rest being treated as a number
	{
		// Find the point in the input text where the first word
		// ends (if there is one...):
		ProjChar* pProjCh_Search = pProjCh_In;
		int NumChars = 0;

		while
		(
			(*pProjCh_Search != '\0')
			&&
			(*pProjCh_Search != ' ')
		)
		{
			pProjCh_Search++;
			NumChars++;
		}

		if ( *pProjCh_Search == '\0' )
		{
			// then there were no word breaks; stop

			return No;
		}

		if ( NumChars < 1 )
		{
			return No;
		}

		for
		(
			LIF<ConsoleVariable*> oi(&List_pConsoleVar);
			!oi.done();
			oi.next()
		)
		{
		
			// LOCALISEME():
			if
			(
				0 == strncmp
				(
					pProjCh_In,
					oi() -> pSCString_Symbol -> pProjCh(),
					NumChars
				)
			)
			{
				if (strchr(pProjCh_Search,'.'))
				{
					// interpret as fraction
					float NewValue = atof(pProjCh_Search);

					oi() -> ProcessSetValue( NewValue );

					return Yes;
				}
				else
				{
					// interpret as int
					int NewValue = atoi(pProjCh_Search);

					oi() -> ProcessSetValue( NewValue );

					return Yes;

				}
			}
		}
	}


	// If you get here, no processing has been performed:
	return No;

}

/*static*/ void ConsoleVariable :: ListAllVariables(void)
{
	SCString* pSCString_Temp = new SCString("LIST OF ALL CONSOLE VARIABLES:");

	pSCString_Temp -> SendToScreen();

	pSCString_Temp ->R_Release();

	for
	(
		LIF<ConsoleVariable*> oi(&List_pConsoleVar);
		!oi.done();
		oi.next()
	)
	{
		oi() -> Display();
	}	
}
// protected:
ConsoleVariable :: ConsoleVariable
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int MinVal_New,
	int MaxVal_New,
	OurBool Cheat

) :	ConsoleSymbol
	(
		pProjCh_ToUse
	),
	MinVal(MinVal_New),
	MaxVal(MaxVal_New),
	pSCString_Description
	(
		new SCString( pProjCh_Description_ToUse )
			// constructor for the SCString adds the required reference
	)
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		ThisIsACheat = Cheat;
	
		// add to list of all console variables
	    List_pConsoleVar . add_entry
	    (
	    	this
	    );
	}
}

// private:
void ConsoleVariable :: Display(void)
{
	// used by the list display and to interrogate an individual variable
	SCString* pSCString_Temp1 = new SCString
	(
		" = "
	);

	SCString* pSCString_Temp2 = MakeValueString( GetValue() );

	SCString* pSCString_Temp3 = MakeRangeString();

	SCString* pSCString_Out = new SCString
	(
		pSCString_Symbol,
		pSCString_Temp1,
		pSCString_Temp2,
		pSCString_Temp3,
		pSCString_Description
	);

	pSCString_Temp3 ->R_Release();
	pSCString_Temp2 ->R_Release();
	pSCString_Temp1 ->R_Release();

	pSCString_Out -> SendToScreen();

	pSCString_Out ->R_Release();
}


void ConsoleVariable :: ProcessSetValue
(
	int Val_New
)
{
	// used by command processor; sets the value and outputs
	// a message

	int OldValue = GetValue();

	SetValue( Val_New );

	OutputResultOfSetValue( OldValue );

}

void ConsoleVariable :: ProcessSetValue
(
	float Val_New
)
{
	// used by command processor; sets the value and outputs
	// a message

	int OldValue = GetValue();

	SetValue( Val_New );

	OutputResultOfSetValue( OldValue );
}

void ConsoleVariable :: OutputResultOfSetValue( int OldVal )
{
	int NewValue = GetValue();

	// Output result
	{
		SCString* pSCString_Temp1 = new SCString
		(
			" "
		);

		SCString* pSCString_Temp2 = MakeValueString
		(
			OldVal
		);

		SCString* pSCString_Temp3 = new SCString
		(
			" -> "
		);

		SCString* pSCString_Temp4 = MakeValueString
		(
			NewValue
		);

		SCString* pSCString_Out = new SCString
		(
			pSCString_Symbol,
			pSCString_Temp1,
			pSCString_Temp2,
			pSCString_Temp3,
			pSCString_Temp4
		);

		pSCString_Temp4 ->R_Release();
		pSCString_Temp3 ->R_Release();
		pSCString_Temp2 ->R_Release();
		pSCString_Temp1 ->R_Release();

		pSCString_Out -> SendToScreen();
		
		pSCString_Out ->R_Release();

	}
	
}


/* Internal function definitions ***********************************/
ConsoleVariable_Simple_Int :: ConsoleVariable_Simple_Int
(
	int& Value_ToUse,
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int MinVal_New,
	int MaxVal_New,
	OurBool Cheat
) :	ConsoleVariable
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		MinVal_New,
		MaxVal_New,
		Cheat
	),
	theValue( Value_ToUse )
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pProjCh_ToUse );
		GLOBALASSERT( pProjCh_Description_ToUse );
	}

	/* CODE */
	{
	}
}

int ConsoleVariable_Simple_Int :: GetValue(void) const
{
	return theValue;
}

void ConsoleVariable_Simple_Int :: SetValue(int Val_New)
{
	// Ensure bounded:
	if ( Val_New > MaxVal )
	{
		Val_New = MaxVal;
	}

	if ( Val_New < MinVal )
	{
		Val_New = MinVal;
	}

	theValue = Val_New;
}

void ConsoleVariable_Simple_Int :: SetValue(float Val_New_F )
{
	int Val_New = int(Val_New_F);

	// Ensure bounded:
	if ( Val_New > MaxVal )
	{
		Val_New = MaxVal;
	}

	if ( Val_New < MinVal )
	{
		Val_New = MinVal;
	}

	theValue = Val_New;
}

// private:
SCString* ConsoleVariable_Simple_Int :: MakeRangeString(void)
{
	SCString* pSCString_Temp2_1 = new SCString
	(
		" INT:("
	);

	SCString* pSCString_Temp2_2 = MakeValueString
	(
		MinVal
	);

	SCString* pSCString_Temp2_3 = new SCString
	(
		","
	);

	SCString* pSCString_Temp2_4 = MakeValueString
	(
		MaxVal
	);

	SCString* pSCString_Temp2_5 = new SCString
	(
		") "
	);

	SCString* pSCString_Return = new SCString
	(
		pSCString_Temp2_1,
		pSCString_Temp2_2,
		pSCString_Temp2_3,
		pSCString_Temp2_4,
		pSCString_Temp2_5
	);

	pSCString_Temp2_5 ->R_Release();
	pSCString_Temp2_4 ->R_Release();
	pSCString_Temp2_3 ->R_Release();
	pSCString_Temp2_2 ->R_Release();
	pSCString_Temp2_1 ->R_Release();		

	return pSCString_Return;
}

SCString* ConsoleVariable_Simple_Int :: MakeValueString(int Val)
{
	return new SCString
	(
		Val
	);
}


ConsoleVariable_Simple_FixP :: ConsoleVariable_Simple_FixP
(
	int& Value_ToUse,
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int MinVal_New,
	int MaxVal_New,
	OurBool Cheat
) :	ConsoleVariable
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		MinVal_New,
		MaxVal_New,
		Cheat
	),
	theValue( Value_ToUse )
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pProjCh_ToUse );
		GLOBALASSERT( pProjCh_Description_ToUse );
	}

	/* CODE */
	{
	}
}

int ConsoleVariable_Simple_FixP :: GetValue(void) const
{
	return theValue;
}

void ConsoleVariable_Simple_FixP :: SetValue(int Val_New)
{
	Val_New *= ONE_FIXED;

	// Ensure bounded:
	if ( Val_New > MaxVal )
	{
		Val_New = MaxVal;
	}

	if ( Val_New < MinVal )
	{
		Val_New = MinVal;
	}

	theValue = Val_New;
}

void ConsoleVariable_Simple_FixP :: SetValue(float Val_New_F )
{
	int Val_New = int(Val_New_F * ONE_FIXED);

	// Ensure bounded:
	if ( Val_New > MaxVal )
	{
		Val_New = MaxVal;
	}

	if ( Val_New < MinVal )
	{
		Val_New = MinVal;
	}

	theValue = Val_New;
}

// private:
SCString* ConsoleVariable_Simple_FixP :: MakeRangeString(void)
{
	SCString* pSCString_Temp2_1 = new SCString
	(
		" FRAC:("
	);

	SCString* pSCString_Temp2_2 = MakeValueString
	(
		MinVal
	);

	SCString* pSCString_Temp2_3 = new SCString
	(
		","
	);

	SCString* pSCString_Temp2_4 = MakeValueString
	(
		MaxVal
	);

	SCString* pSCString_Temp2_5 = new SCString
	(
		") "
	);

	SCString* pSCString_Return = new SCString
	(
		pSCString_Temp2_1,
		pSCString_Temp2_2,
		pSCString_Temp2_3,
		pSCString_Temp2_4,
		pSCString_Temp2_5
	);

	pSCString_Temp2_5 ->R_Release();
	pSCString_Temp2_4 ->R_Release();
	pSCString_Temp2_3 ->R_Release();
	pSCString_Temp2_2 ->R_Release();
	pSCString_Temp2_1 ->R_Release();		

	return pSCString_Return;
}

SCString* ConsoleVariable_Simple_FixP :: MakeValueString(int Val)
{
	return new SCString
	(
		FixP2Float( Val )
	);
}
