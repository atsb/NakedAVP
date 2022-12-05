/*******************************************************************
 *
 *    DESCRIPTION: 	conscmnd.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 28/1/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "conscmnd.hpp"
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
	/*static*/ List<ConsoleCommand*> ConsoleCommand :: List_pConsoleCommand;

/* Internal type definitions ***************************************/

	class ConsoleCommand_VoidVoid : public ConsoleCommand
	{
	public:
		ConsoleCommand_VoidVoid
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (*f) (void),
			OurBool Cheat = FALSE
		);
		void Execute(ProjChar* pProjCh_In);

	private:
		void (*theFn) (void);
	};
	class ConsoleCommand_VoidInt : public ConsoleCommand
	{
	public:
		ConsoleCommand_VoidInt
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (*f) (int),
			OurBool Cheat = FALSE
		);
		void Execute(ProjChar* pProjCh_In);
	private:
		void (*theFn) (int);
	};
	class ConsoleCommand_IntVoid : public ConsoleCommand
	{
	public:
		ConsoleCommand_IntVoid
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int (*f) (void),
			OurBool Cheat = FALSE
		);
		void Execute(ProjChar* pProjCh_In);
	private:
		int (*theFn) (void);
	};
	class ConsoleCommand_IntInt : public ConsoleCommand
	{
	public:
		ConsoleCommand_IntInt
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int (*f) (int),
			OurBool Cheat = FALSE
		);
		void Execute(ProjChar* pProjCh_In);
	private:
		int (*theFn) (int);
	};
	class ConsoleCommand_VoidCharP : public ConsoleCommand
	{
	public:
		ConsoleCommand_VoidCharP
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (*f) (char*),
			OurBool Cheat = FALSE
		);
		void Execute(ProjChar* pProjCh_In);
	private:
		void (*theFn) (char*);
	};


/* Internal function ProjChar* pProjCh_In*/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class ConsoleCommand : public ConsoleSymbol
// public:

// Various factory methods:
/*static*/ void ConsoleCommand :: Make
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (&f) (void),
	OurBool Cheat
)
{
	new ConsoleCommand_VoidVoid
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		f,
		Cheat
	);
}
/*static*/ void ConsoleCommand :: Make
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (&f) (int),
	OurBool Cheat
)
{
	new ConsoleCommand_VoidInt
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		f,
		Cheat
	);
}
/*static*/ void ConsoleCommand :: Make
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int (&f) (void),
	OurBool Cheat

)
{
	new ConsoleCommand_IntVoid
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		f,
		Cheat
	);
}

/*static*/ void ConsoleCommand :: Make
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int (&f) (int),
	OurBool Cheat
)
{
	new ConsoleCommand_IntInt
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		f,
		Cheat
	);
}
/*static*/ void ConsoleCommand :: Make
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (&f) (char*),
	OurBool Cheat
)
{
	new ConsoleCommand_VoidCharP
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		f,
		Cheat
	);
}

/*static*/ OurBool ConsoleCommand :: Process( ProjChar* pProjCh_In )
{
	// used for proccesing input text.
	// return value = was any processing performed?

	GLOBALASSERT( pProjCh_In );

	OurBool bProcessed = No;

	// Parse into words; find the first word.  Iterate through the commands
	// looking for a match:

	{
		ProjChar *commandPtr = pProjCh_In;
		ProjChar *argumentPtr = pProjCh_In;

		while(*argumentPtr!=0 && *argumentPtr!=' ')
		{
			argumentPtr++;	
		}

		// if we found a space at the end of the first word
		// change it to a terminator, and point to the text after it
		if (*argumentPtr) *argumentPtr++=0;	
		
		// otherwise we'll be pointing to a terminator anyway
		

		if ( *commandPtr )
		{
			// Iterate through the console commands; looking for a match
			{
				for
				(
					LIF<ConsoleCommand*> oi(&List_pConsoleCommand);
					!oi . done();
					oi . next()
				)
				{
					GLOBALASSERT(oi());
					GLOBALASSERT(oi()-> pSCString_Symbol );

					
					if
					(
						oi()->ThisIsACheat ? 

						STRUTIL_SC_Strequal //case sensitive comparisons for cheats
						(
							oi() -> pSCString_Symbol -> pProjCh(),
							commandPtr
						)
						:
						STRUTIL_SC_Strequal_Insensitive //case insensitive otherwise
						(
							oi() -> pSCString_Symbol -> pProjCh(),
							commandPtr
						)
					)
					{
						// Got match
						bProcessed = Yes;

						// Execute the function:
						{
							oi() -> Execute(argumentPtr);
						}
					}
				}
			}
			if (*argumentPtr) *(--argumentPtr)=' ';
		}


	}

	return bProcessed;
}


/*static*/ void ConsoleCommand :: ListAll(void)
{
	SCString* pSCString_Temp = new SCString("LIST OF ALL CONSOLE COMMANDS:");
		// LOCALISEME()

	pSCString_Temp -> SendToScreen();

	pSCString_Temp ->R_Release();

	for
	(
		LIF<ConsoleCommand*> oi(&List_pConsoleCommand);
		!oi.done();
		oi.next()
	)
	{
		oi() -> Display();
	}	
}

/*virtual*/ ConsoleCommand :: ~ConsoleCommand()
{
	pSCString_Description -> R_Release();

	List_pConsoleCommand . delete_entry(this);

}

void ConsoleCommand :: Display(void) const
{
	SCString* pSCString_Temp1 = new SCString("\"");
	SCString* pSCString_Temp2 = new SCString("\" ");

	SCString* pSCString_Feedback = new SCString
	(
		pSCString_Temp1,
		pSCString_Symbol,
		pSCString_Temp2,
		pSCString_Description
	);

	pSCString_Temp2 -> R_Release();
	pSCString_Temp1 -> R_Release();

	pSCString_Feedback -> SendToScreen();

	pSCString_Feedback -> R_Release();
}


// protected:
ConsoleCommand :: ConsoleCommand
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	OurBool Cheat
) : ConsoleSymbol(pProjCh_ToUse),
	pSCString_Description( new SCString(pProjCh_Description_ToUse) )
{
	ThisIsACheat = Cheat;
	List_pConsoleCommand . add_entry(this);
}

void ConsoleCommand :: EchoResult(int Result)
{
	SCString* pSCString_Feedback = new SCString(Result);
	pSCString_Feedback -> SendToScreen();
	pSCString_Feedback -> R_Release();
}

int ConsoleCommand :: GetArg(ProjChar* pProjCh_Arg)
{
	GLOBALASSERT( pProjCh_Arg );

	return atoi(pProjCh_Arg);
}


/* Internal function definitions ***********************************/
// class ConsoleCommand_VoidVoid : public ConsoleCommand
// public:
ConsoleCommand_VoidVoid :: ConsoleCommand_VoidVoid
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (*f) (void),
	OurBool Cheat

) : ConsoleCommand
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		Cheat
	),
	theFn(f)
{
}
void ConsoleCommand_VoidVoid :: Execute(ProjChar* pProjCh_In)
{
	GLOBALASSERT(theFn);
	GLOBALASSERT(pProjCh_In);
	(*theFn)();
}
// class ConsoleCommand_VoidInt : public ConsoleCommand
// public:
ConsoleCommand_VoidInt :: ConsoleCommand_VoidInt
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (*f) (int),
	OurBool Cheat

) : ConsoleCommand
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		Cheat
	),
	theFn(f)
{
}
void ConsoleCommand_VoidInt :: Execute(ProjChar* pProjCh_In)
{
	GLOBALASSERT(theFn);
	GLOBALASSERT(pProjCh_In);
	(*theFn)
	(
		GetArg(pProjCh_In)
	);
}
// class ConsoleCommand_IntVoid : public ConsoleCommand
// public:
ConsoleCommand_IntVoid :: ConsoleCommand_IntVoid
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int (*f) (void),
	OurBool Cheat

) : ConsoleCommand
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		Cheat
	),
	theFn(f)
{
}
void ConsoleCommand_IntVoid :: Execute(ProjChar* pProjCh_In)
{
	GLOBALASSERT(theFn);
	GLOBALASSERT(pProjCh_In);
	EchoResult
	(
		(*theFn)()
	);
}
// class ConsoleCommand_IntInt : public ConsoleCommand
// public:
ConsoleCommand_IntInt :: ConsoleCommand_IntInt
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	int (*f) (int),
	OurBool Cheat
) : ConsoleCommand
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		Cheat
	),
	theFn(f)
{
}
void ConsoleCommand_IntInt :: Execute(ProjChar* pProjCh_In)
{
	GLOBALASSERT(theFn);
	GLOBALASSERT(pProjCh_In);

	EchoResult
	(
		(*theFn)
		(
			GetArg(pProjCh_In)
		)
	);
}


ConsoleCommand_VoidCharP :: ConsoleCommand_VoidCharP
(
	ProjChar* pProjCh_ToUse,
	ProjChar* pProjCh_Description_ToUse,
	void (*f) (char*),
	OurBool Cheat

) : ConsoleCommand
	(
		pProjCh_ToUse,
		pProjCh_Description_ToUse,
		Cheat
	),
	theFn(f)
{
}
void ConsoleCommand_VoidCharP :: Execute(ProjChar* pProjCh_In)
{
	GLOBALASSERT(theFn);
	GLOBALASSERT(pProjCh_In);
	(*theFn)
	(
		pProjCh_In
	);
}
