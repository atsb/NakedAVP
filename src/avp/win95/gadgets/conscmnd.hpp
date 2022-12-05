/*
	
	conscmnd.hpp

*/

#ifndef _conscmnd
#define _conscmnd 1

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
	class ConsoleCommand : public ConsoleSymbol
	{
	public:
		static void CreateAll(void);

		// Various factory methods:
		static void Make
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (&f) (void),
			OurBool Cheat = FALSE
		);
		static void Make
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (&f) (int),
			OurBool Cheat = FALSE
		);
		static void Make
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int (&f) (void),
			OurBool Cheat = FALSE
		);
		static void Make
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			int (&f) (int),
			OurBool Cheat = FALSE
		);
		static void Make
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			void (&f) (char*),
			OurBool Cheat = FALSE
		);

		static OurBool Process( ProjChar* pProjCh_In );
			// used for proccesing input text.
			// return value = was any processing performed?

		static void ListAll(void);

		virtual void Execute( ProjChar* pProjCh_In ) = 0;

		

		void Display(void) const;


	protected:
		ConsoleCommand
		(
			ProjChar* pProjCh_ToUse,
			ProjChar* pProjCh_Description_ToUse,
			OurBool Cheat = FALSE
		);

		void EchoResult(int Result);
		int GetArg(ProjChar* pProjCh_Arg);

	private:		
		SCString* pSCString_Description;

		static List <ConsoleCommand*> List_pConsoleCommand;
	public:
		virtual ~ConsoleCommand();
	};
	

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
