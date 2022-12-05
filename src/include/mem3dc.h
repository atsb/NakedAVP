/* mem3dc.h */
#ifndef MEM3DC_H_INCLUDED
#define MEM3DC_H_INCLUDED

#ifdef __cplusplus

	extern "C" {

#endif

#include "fixer.h" // make sure system headers get included first, because windows headers use Yes/No
#include "system.h"
#include <stddef.h>

/* defines */
#if 1
#define DBGMALLOC 0
#else
	#ifdef _DEBUG /* standard compiler command line debugging-ON switch */
		#define DBGMALLOC 1
	#elif defined(NDEBUG) /* standard compiler command line debugging-OFF switch */
		#define DBGMALLOC 0
	#elif defined(_DBGMALLOC) /* alternate compiler command line switch */
		#define DBGMALLOC _DBGMALLOC
	#else /* default switch */
		#define DBGMALLOC 1
	#endif
#endif

/* parameters for DumpMallocInfo */
#define PARTIALDUMP 0    /* print outstanding mallocs number and total memory allocated */
#define DUMPTOSCREEN  1  /* print all outstanding mallocs to screen */
#define DUMPTOFILE 2     /* write outstanding malloc details to file (filename defined with MALLOCDUMPFILE) */
#define CPPGLOBAL 0x100000 /* line numbers offset by this value if the malloc is as part of a constructor for a C++ global whose dealloc may not be recorded */

/* JH - 30.5.97
I noticed that the MALLOC_RECORD structure has char[40]
for the filename. Since I know that on the PC, the __FILE__
macro results in a string compiled into the executable, and
evaulates to a pointer to that string, we do not need to make
a separate copy of the string for each malloc - just store the
pointer.
So, on PC this reduces the data size for the malloc records from 1.04Mb to 320K ! */

#define COPY_FILENAME 0 /* new behavior */

/* platform specific memory allocation and deallocation declarations */
extern void *AllocMem(size_t __size);
extern void DeallocMem(void *__ptr);

/* mem.c public functions */
#if COPY_FILENAME
extern void record_free(void *ptr, char string[], unsigned long lineno);
extern void *record_malloc(long size, char string[], unsigned long lineno);
#else /* new prototypes to take just pointers - dunno if it's really necessary */
extern void record_free(void *ptr, char const * string, unsigned long lineno);
extern void *record_malloc(long size, char const * string, unsigned long lineno);
#endif
extern void DumpMallocInfo(int type);
extern void DumpBoundsCheckInfo(int type);
extern void DumpInfo(int type);

#if DBGMALLOC
#define AllocateMem(x) record_malloc(x,__FILE__, __LINE__) 
#define DeallocateMem(x) record_free(x,__FILE__, __LINE__)

#ifdef __cplusplus

/* JH 30/5/97 - 2/6/97
Overloaded new and delete to use record_malloc and record_free
Notes:
1.
Although these are declared as inline, C++ files which do not include this
header will still use the overloaded operators new and delete (as long as at
least one C++ file includes this header), although the lack of the macro for
new will mean that you will not have file and line number information in the
malloc record.
2.
Since it is not possible to have a user defined delete operator which takes
extra parameters, the malloc record will not be able to track the file and
line number of delete operations. For this reason, it is also necessary to
overload the default operator new, so that corresponding delete operations
(which will go through the record_free function) cause the memory to be
deallocated in the same way.
3.
Global C++ objects may have constructors which call new and delete.
Since their deconstructors will only be called after the function 'main' or
'WinMain' has returned and after all functions specified with calls to atexit
have returned, it is not possible to gruarantee a dump of the malloc info
after they have been destroyed. I have introduced a global C++ object with a
constructor and decostructor, which turn malloc recording on and off
respectively. This will help prevent misreported memory leaks, because global
objects contructed before this special object will be destroyed after it,
hence any associated memory allocation and deallocation will not be recorded
in the same way. A malloc dump called from the destructor of this special
object will not misreport memory leaks for some global objects (those which
happen to be constructed after the special object and deconstructed before
it), though it will report the outstanding allocations as being from the
constructor of a global C++ object. This is a intended as a warning - these
outstanding allocations are probably not leaks, since they will be
deconstructed fully before the program terminates.
*/

extern "C++" {

extern int __cpp_new_recording;

inline void * operator new(size_t s, char const * file, unsigned long line)
{
	return
		__cpp_new_recording
			? record_malloc(s,file,line)
			: record_malloc(s,file,line+CPPGLOBAL)
		;
}
inline void * operator new(size_t s)
{
	return
		__cpp_new_recording
			? record_malloc(s,"Unknown file (C++ new)",0)
			: record_malloc(s,"Unknown file (C++ new)",CPPGLOBAL)
		;
}
inline void operator delete(void * p)
{
	record_free(p,"Unknown file (C++ delete)",0);
}
#ifndef _MSC_VER
inline void * operator new[](size_t s, char const * file, unsigned long line)
{
	return
		__cpp_new_recording
			? record_malloc(s,file,line)
			: record_malloc(s,file,line+CPPGLOBAL)
		;
}
inline void * operator new[](size_t s)
{
	return
		__cpp_new_recording
			? record_malloc(s,"Unknown file (C++ new[])",0)
			: record_malloc(s,"Unknown file (C++ new[])",CPPGLOBAL)
		;
}
inline void operator delete[](void * p)
{
	record_free(p,"Unknown file (C++ delete[])",0);
}
#endif

#define new new(__FILE__,__LINE__)

}

#endif

#else
#define AllocateMem(x) AllocMem(x) 
#define DeallocateMem(x) DeallocMem(x) 
#endif


#ifdef __cplusplus

	};

#endif


#endif
