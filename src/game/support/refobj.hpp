/*
	
	refobj.hpp

	Created by DHM for Headhunter; copied into AVP

	Reference-counted base class to be used by e.g. strings

*/

#ifndef _refobj
#define _refobj 1


	#ifndef __fail_h
	#include "fail.h"
	#endif

	

#ifdef __cplusplus
	extern "C" {
#endif


/* Version settings *****************************************************/
	#define TrackReferenceCounted No
		/*
			This is a debug option that has a fair amount of run-time
			cost.

			It maintains a list of all reference-counted objects, and
			data on when references were added or released.

			Data on a particular object can be sent to a log file;
			there is a function to do this for all that exist.			
		*/

/* Macros ***************************************************************/

#if TrackReferenceCounted
	#define R_AddRef()		imp_R_AddRef(__FILE__,__LINE__)
	#define R_Release()		imp_R_Release(__FILE__,__LINE__)
#endif
	
/* Imported globals *****************************************************/

	extern char const* refobj_fail_addref;
	extern char const* refobj_fail_release;
	extern char const* refobj_fail_destructor;


/* Type definitions *****************************************************/
	class R_DumpContext; // fully declared in DCONTEXT.HPP

	class RefCountObject_TrackData; // fully declared within REFOBJ.CPP

	class RefCountObject
	{
	// {{{ Private data
	private:
		int RefCount;

		#if TrackReferenceCounted
		RefCountObject_TrackData* pTrackData;
			// done as a non-NULL pointer rather than a ref so as to hide
			// definition (avoiding compiler dependency)
		#endif

	// }}}


	// {{{ Private functions
	private:
		// Private fns for the tracking system are complex and defined in REFOBJ.CPP
		#if TrackReferenceCounted
		void Track_Construct(void);
		void Track_R_AddRef(char* theFilename, int theLineNum);
		void Track_R_Release(char* theFilename, int theLineNum);
		void Track_Destroy(void);
		#endif
	// }}}

	public:
		RefCountObject() :
			#if TrackReferenceCounted
			pTrackData(NULL),
			#endif
			RefCount(1)
		{
			#if TrackReferenceCounted
			Track_Construct();
			#endif
		}

		#if TrackReferenceCounted
		void imp_R_AddRef(char* theFilename, int theLineNum)
		{
			#ifndef NDEBUG
			if ( RefCount <= 0)
			{
				fail(refobj_fail_addref);
			}
			#endif
			RefCount++;

			Track_R_AddRef(theFilename,theLineNum);
		}

		void imp_R_Release(char* theFilename, int theLineNum)
		{
			#ifndef NDEBUG
			if ( RefCount <= 0)
			{
				fail(refobj_fail_release);
			}
			#endif

			Track_R_Release(theFilename,theLineNum);

			if ( (--RefCount) == 0 )
			{
				delete this;
			}
		}
		#else
		void R_AddRef(void)
		{
			#ifndef NDEBUG
			if ( RefCount <= 0)
			{
				fail(refobj_fail_addref);
			}
			#endif
			RefCount++;
		}

		void R_Release(void)
		{
			#ifndef NDEBUG
			if ( RefCount <= 0)
			{
				fail(refobj_fail_release);
			}
			#endif

			if ( (--RefCount) == 0 )
			{
				delete this;
			}
		}
		#endif

		#if debug
		int CheckRef() const
		{
			return RefCount;
		}
			// Handy way to examine reference count for debugging only
		#endif

		#if TrackReferenceCounted
		virtual void DumpIDForReferenceDump(R_DumpContext& theContext) const = 0;
		void ReferenceDump(R_DumpContext& theContext) const;
		static void DumpAll(R_DumpContext& theContext);
		#endif

	protected:
		// Destructors must be protected; only derived classes may use the
		// destructor and (we hope) only in the R_Release() method:
		virtual ~RefCountObject()
		{
			#ifndef NDEBUG
			if ( RefCount != 0 )
			{
				fail(refobj_fail_destructor);
			}
			#endif

			#if TrackReferenceCounted
			Track_Destroy();
			#endif
		}
	};



/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
