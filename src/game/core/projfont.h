/*
	
	projfont.h

	Defines a project-specific enum for all possible fonts that can loaded
	by the program; used by the indexed-font code.

	Also defines a macro for the maximum number of fonts "IndexedFonts_MAX_NUMBER_OF_FONTS";
	this macro must evaluate at compile time to a constant integer.

	It is assumed that font indices are in the range:

		[ 0, IndexedFonts_MAX_NUMBER_OF_FONTS )

	i.e. inclusive on the LHS, exclusive on the RHS

	(AVP version created 19/11/97)
*/

#ifndef _projfont
#define _projfont 1

	#ifndef _font_h_included
	#include "font.h"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/
	#define IndexedFonts_MAX_NUMBER_OF_FONTS ( NUM_FONTS )

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	typedef enum fonts FontIndex;

	

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
