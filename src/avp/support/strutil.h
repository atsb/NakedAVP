/*
	
	strutil.h

	Created 13/11/97 by David Malcolm: more carefully specified
	versions of the C string library functions, to act on ProjChars
	rather than chars

*/

#ifndef _strutil
#define _strutil 1

	#ifndef _projtext
	#include "projtext.h"
	#endif

	#ifndef _ourbool
	#include "ourbool.h"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	/* String manipulation **********************************************/
		extern void STRUTIL_SC_WriteTerminator
		(
			ProjChar* pProjCh
		);

		extern OurBool STRUTIL_SC_fIsTerminator
		(
			const ProjChar* pProjCh
		);

	/* Ansi to HHTS conversion ********************************************/
		/* Return value = Yes iff no truncation occurred i.e. the whole string was copied */

 		#if 0
 		extern OurBool STRUTIL_ProjChar_To_ANSI
		(
			LPTSTR lptszANSI_Out,
			unsigned int MaxSize, /* includes NULL-terminator; truncates after this */

			ProjChar* pProjCh_In		
		);

 		extern OurBool STRUTIL_ANSI_To_ProjChar
		(
			ProjChar* pProjCh_Out,
			unsigned int MaxSize, /* includes NULL-terminator; truncates after this */
			
			LPTSTR lptszANSI_In
		);
		#endif

	/* Emulation of <string.h> *******************************************/
		extern unsigned int STRUTIL_SC_Strlen
		(
			const ProjChar* pProjCh_In
		);

		extern ProjChar* STRUTIL_SC_StrCpy
		(
			ProjChar* pProjCh_Dst,
			const ProjChar* pProjCh_Src
		);

		extern void STRUTIL_SC_FastCat
		(
			ProjChar* pProjCh_Dst,
			const ProjChar* pProjCh_Src_0,
			const ProjChar* pProjCh_Src_1			
		);
			/* This function assumes the destination area is large enough;
			it copies Src0 followed by Src1 to the dest area.
			*/

		extern OurBool STRUTIL_SC_Strequal
		(
			const ProjChar* pProjCh_1,
			const ProjChar* pProjCh_2
		);

		extern OurBool STRUTIL_SC_Strequal_Insensitive
		(
			const ProjChar* pProjCh_1,
			const ProjChar* pProjCh_2
		);
		/*
			these functions copy at most MaxSize chars from Src to Dst; this INCLUDES
			space used by a NULL terminator (so that you can pass it an array
			for the destination together with its size.  The return value is
			whether the	entire string was copied.
		*/
		extern OurBool STRUTIL_SC_SafeCopy
		(
			ProjChar* pProjCh_Dst,
			unsigned int MaxSize,

			const ProjChar* pProjCh_Src
		);

		extern void STRUTIL_SC_SafeCat
		(
			ProjChar* pProjCh_Dst,
			unsigned int MaxSize,

			const ProjChar* pProjCh_Add
		);
		
		extern size_t STRUTIL_SC_NumBytes
		(
			const ProjChar* pProjCh
		);



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
