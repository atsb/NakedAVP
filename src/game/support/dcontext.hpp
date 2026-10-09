/*
	
	DCONTEXT.HPP

	Created 14/1/98 by DHM:

	Dump-contexts for use by debugging routines.
	
	The idea is that debug-dump routines take a reference
	to one of these, signifying how they should output;
	these are in separate headers to avoid dependencies.

	There are derived classes for the screen and for log files.

	Analagous to the class CDumpContext in Microsoft's MFC.

*/

#ifndef _dcontext
#define _dcontext 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	class R_DumpContext
	{
	public:
		virtual int dputs(char const * const buf) = 0;
		virtual int dprintf(char const * format, ... ) = 0;
		virtual int vdprintf(char const * format, va_list ap ) = 0;
	};

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif


