/*KJL***************************************
*    Language Internationalization Code    *
***************************************KJL*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "gamedef.h"


#include "langenum.h"
#include "language.h"
#include "huffman.hpp"

// DHM 12 Nov 97: hooks for C++ string handling code:
#include "strtab.hpp"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "avp_menus.h"


#ifdef AVP_DEBUG_VERSION
	#define USE_LANGUAGE_TXT 0
#else
	#define USE_LANGUAGE_TXT 1
#endif

static char EmptyString[]="";

static char *TextStringPtr[MAX_NO_OF_TEXTSTRINGS] = { EmptyString };
static char *TextBufferPtr;

void InitTextStrings(void)
{
	char *filename;
	char *textPtr;
	int i;

	/* language select here! */
	GLOBALASSERT(AvP.Language>=0);
	GLOBALASSERT(AvP.Language<I_MAX_NO_OF_LANGUAGES);
	
#if MARINE_DEMO
	filename = "menglish.txt";
#elif ALIEN_DEMO
	filename = "aenglish.txt";
#elif USE_LANGUAGE_TXT
	filename = "language.txt";
#else
	filename = LanguageFilename[AvP.Language];
#endif
	TextBufferPtr = LoadTextFile(filename);
		
	if (TextBufferPtr == NULL) {
		/* NOTE:
		   if this load fails, then most likely the game is not 
		   installed correctly. 
		   SBF
		  */ 
		fprintf(stderr, "ERROR: unable to load %s language text file\n",
			 filename);
		exit(1);
	}
	
	if (!strncmp (TextBufferPtr, "REBCRIF1", 8))
	{
		textPtr = (char*)HuffmanDecompress((HuffmanPackage*)(TextBufferPtr)); 		
		DeallocateMem(TextBufferPtr);
		TextBufferPtr=textPtr;
	}
	else
	{
		textPtr = TextBufferPtr;
	}

	AddToTable( EmptyString );

	for (i=1; i<MAX_NO_OF_TEXTSTRINGS; i++)
	{	
		/* scan for a quote mark */
		while (*textPtr++ != '"')  {
			if (*textPtr == '@') {
				// should be an error as this language file
				// doesn't match the game.
				return; /* '@' should be EOF */
			}
		}

		/* now pointing to a text string after quote mark*/
		TextStringPtr[i] = textPtr;

		/* scan for a quote mark */
		while (*textPtr != '"')
		{	
			textPtr++;
		}

		/* change quote mark to zero terminator */
		*textPtr = 0;
		textPtr++;

		AddToTable( TextStringPtr[i] );
	}
}

void KillTextStrings(void)
{
	UnloadTextFile(LanguageFilename[AvP.Language],TextBufferPtr);

	UnloadTable();
}

char *GetTextString(enum TEXTSTRING_ID stringID)
{
	// Not good.
	// These strings do not exist in data.
	if (stringID > MIN_NEW_TEXTSTRINGS && stringID < MAX_NEW_TEXTSTRINGS) {
		switch (stringID) {
			case TEXTSTRING_MAINMENU_EXITGAME_HELP_NEW: return "Exit the game.";
			case TEXTSTRING_MAINMENU_CREDITS_NEW: return "Credits";
			case TEXTSTRING_MAINMENU_CREDITS_HELP_NEW: return "View the credits.";
			default: break;
		}
	}

	LOCALASSERT(stringID<MAX_NO_OF_TEXTSTRINGS);
	if (stringID < MAX_NO_OF_TEXTSTRINGS) {
		return TextStringPtr[stringID];
	}
	return EmptyString;
}
