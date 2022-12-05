#include "mem3dc.h"

#if DBGMALLOC

#if 1

// try and turn C++ new/delete tracking on such that
// we can do a malloc dump when the global objects
// with associated memory allocated is recored, the
// deallocation is recored, and then a malloc dump
// is done

// note that some global objects wont have their memory
// allocations/deallocations in the constructor/destructor
// tracked through record_malloc/record_free, but since
// global objects are deconstructed in the reverse order
// from construction, the deallocation type in the destructor
// will correspond to the allocation type in the constructor

int __cpp_new_recording = 0;

class DebugObject
{
public:
	DebugObject();
	~DebugObject();
};

DebugObject::DebugObject()
{
	__cpp_new_recording = 1;
}

DebugObject::~DebugObject()
{
	__cpp_new_recording = 0;
	DumpMallocInfo(DUMPTOFILE);
}

static DebugObject dbo;

#else

int __cpp_new_recording = 1;

#endif

#endif
