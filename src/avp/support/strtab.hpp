/*
	
	strtab.hpp

*/

#ifndef _strtab
#define _strtab 1

	#ifndef _projtext
	#include "projtext.h"
	#endif

	#ifndef _langenum_h_
	#include "langenum.h"
	#endif

	#ifdef __cplusplus
		#ifndef _scstring
		#include "scstring.hpp"
		#endif
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/
	#define OnlyOneStringTable	Yes

/* Constants  ***********************************************************/

/* Macros ***************************************************************/
	#if OnlyOneStringTable
		#define STRINGTABLE_DECL_SPECIFIER	static
	#else
		#define STRINGTABLE_DECL_SPECIFIER
	#endif

/* Type definitions *****************************************************/
	#ifdef __cplusplus
	class StringTable
	{
	public:
		STRINGTABLE_DECL_SPECIFIER SCString& GetSCString
		(
			enum TEXTSTRING_ID stringID
		);

		STRINGTABLE_DECL_SPECIFIER void Add( ProjChar* pProjChar );

		#if OnlyOneStringTable
		static void Unload(void);
		#else
		StringTable();
		~StringTable();
		#endif

	private:
		STRINGTABLE_DECL_SPECIFIER unsigned int NumStrings;
		STRINGTABLE_DECL_SPECIFIER SCString* pSCString[ MAX_NO_OF_TEXTSTRINGS ];
	};
	#endif // __cplusplus

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	#if OnlyOneStringTable
	extern void AddToTable( ProjChar* pProjChar );
	extern void UnloadTable(void);
	#endif


/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
