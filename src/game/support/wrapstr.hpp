/*
	
	wrapstr.hpp

	Support for word-wrapping strings: the heart of it takes a string,
	a font, and a width, and creates a list of strings objects guaranteed
	to have width less than or equal to the specified width when displayed with
	the specified font.

	This guarantee is not absolute; it can fail to be upheld if there's a word
	in the string that's wider than the width you give it (and thus can't fit
	without being broken)

	There is no support for mixing fonts.

	There is no support for carriage returns, tabs, etc.

*/

#ifndef _wrapstr_hpp
#define _wrapstr_hpp 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

	#ifndef _scstring
	#include "scstring.hpp"
	#endif

	#ifndef _indexfnt
	#include "indexfnt.hpp"
	#endif

	#ifndef _reflist_hpp
	#include "reflist.hpp"
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
	namespace WordWrap
	{
		/* Deprecated version; to be phased out once we have interators for RefLists
		since you have to manually release references as you destroy all/parts of the list
		*/
		extern List<SCString*>* DeprecatedMake
		(
			const SCString& SCString_In,

			const IndexedFont& IndexedFnt_In,

			int W_FirstLine_In,
			int W_Subsequently_In
				// widths to wrap with.
				// They are different in order to support paragraph starts of various kinds
				// e.g. bulleting
		);

		extern RefList<SCString>* Make
		(
			const SCString& SCString_In,

			const IndexedFont& IndexedFnt_In,

			int W_FirstLine_In,
			int W_Subsequently_In
				// widths to wrap with.
				// They are different in order to support paragraph starts of various kinds
				// e.g. bulleting
		);

		int bWhitespace( ProjChar ProjCh );

		int bWithinWord( ProjChar* pProjCh_Test );

	};


/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
