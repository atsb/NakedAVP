/*
	
	consvar.hpp

	Console variables : i.e. values that can be adjusted at the console

*/

#ifndef _consvar_hpp
#define _consvar_hpp 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

	#ifndef _conssym_hpp
	#include "conssym.hpp"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/

	// Abstract base class of console variable
	// It's abstract because the Get/SetValue functions are pure virtual.
	// There's a "standard" derived class defined within CONSVAR.CPP
	// which has a direct link to a global int, and allows direct set/get
	// For other variables (which might require additional processing),
	// derive another class from the base and implement the Set/Get pairs

	// Use single words only as symbols; the command interpreter looks
	// for spaces

	class ConsoleVariable : public ConsoleSymbol
	{
	public:
		// Factory method:
		static ConsoleVariable* MakeSimpleConsoleVariable_Int
		(
			int& Value_ToUse,
			ProjChar* pProjCh_Symbol_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int MinVal_New,
			int MaxVal_New,
			OurBool Cheat = FALSE
		);
		static ConsoleVariable* MakeSimpleConsoleVariable_FixP
		(
			int& Value_ToUse,
			ProjChar* pProjCh_Symbol_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int MinVal_New, // fixed point value
			int MaxVal_New,	// fixed point value
			OurBool Cheat = FALSE
		);

		

		virtual int GetValue(void) const = 0;
		virtual void SetValue(int Val_New) = 0;
		virtual void SetValue(float Val_New) = 0;

		static OurBool Process( ProjChar* pProjCh_In );
			// used for proccesing input text.  Could decide that the user
			// was requesting the value of a variable, or was setting a new
			// value etc; if so, acts accordingly.
			// return value = was any processing performed?

		static void ListAllVariables(void);

		static void CreateAll(void);
			// hook to create all the console variables
			// (to make it easy to add new ones)

	protected:
		ConsoleVariable
		(
			ProjChar* pProjCh_Symbol_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int MinVal_New,
			int MaxVal_New,
			OurBool Cheat = FALSE
		);

	private:
		void Display(void);
			// used by the list display and to interrogate an individual variable

		void ProcessSetValue
		(
			int Val_New
		);
		void ProcessSetValue
		(
			float Val_New
		);
			// used by command processor; sets the value and outputs
			// a message

		void OutputResultOfSetValue( int OldVal );

		virtual SCString* MakeRangeString(void) = 0;
		virtual SCString* MakeValueString(int Val) = 0;


	// Data:
	protected:
		int MinVal;
		int MaxVal;

	private:		
		SCString* pSCString_Description;

		static List <ConsoleVariable*> List_pConsoleVar;
	public:
		~ConsoleVariable();
	};	// suggested naming: "ConsoleVar"

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
