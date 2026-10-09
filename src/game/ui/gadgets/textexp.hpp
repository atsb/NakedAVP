/*
	
	textexp.hpp

	Text Expansions for typing

*/

#ifndef _textexp
#define _textexp 1

	#ifndef _scstring
	#include "scstring.hpp"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	// When typing, whenever space is pressed, the program searches
	// the last word typed to see if it matches one of the expansions
	// If it does, then the word is replaced by the expanded version
	// E.g. "SG" could be defined to expand to "SENTRYGUN"

	class TextInputState; // fully declared in TEXTIN.HPP

	class TextExpansion
	{
	public:
		

		void Display(void);
			// sends info on this expansion to the screen

		static void AddExpansion
		(
			ProjChar* pProjCh_ToParse
		);
		static void TryToRemoveExpansion
		(
			ProjChar* pProjCh_ToParse
		);

		static void TestForExpansions
		(
			TextInputState* pTextInputState_In
		);
			// Called by the typing code whenever a word is completed
			// This function is a friend to the class TextInputState

		static void ListAll(void);

		static int bVerbose;
			// public global so it can be accessed via a console variable

	private:
		TextExpansion
		(
			SCString* pSCString_Short,
			SCString* pSCString_Expansion
		);

		static void TryToRemoveExpansion
		(
			SCString* pSCString_Word
		);
			// assumed the word is already parsed/doesn't contain whitespace

		SCString* pSCString_Short_Val;
		SCString* pSCString_Expansion_Val;

		SCString* pSCString_Description_Val;
			// a string of the form: "<shortform>" -> "<longform>"

		static List<TextExpansion*> List_pTextExp;

	public:
		~TextExpansion();
	};  // suggested naming: TextExp

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/





/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif



#endif
