#include "3dc.h"

#include <stdlib.h>

#define UseLocalAssert  No

#include "ourasert.h"

#if debug
int alloc_cnt = 0;
int deall_cnt = 0;
#endif

void *AllocMem(size_t __size);
void DeallocMem(void *__ptr);

/* Note: Never use AllocMem directly !   */
/* Instead use AllocateMem() which is a  */
/* macro defined in mem3dc.h that allows */
/* for debugging info.                   */

void* AllocMem(size_t __size) {
	GLOBALASSERT(__size > 0);
	if (__size == 0)
		__size = 1;
	void* p = calloc(1, __size);
	if (!p)
	{
		printf("ERROR in AllocMem");
	}
	return p;
}

/* Note: Never use DeallocMem directly !  */
/* Instead use DeallocateMem() which is a */
/* macro defined in mem3dc.h that allows  */
/* for debugging info.                    */

void DeallocMem(void *__ptr)
{
	#if debug
	deall_cnt++;
	#endif

	if(__ptr) free(__ptr);

	#if debug
	else {

		textprint("ERROR - freeing null ptr\n");
		WaitForReturn();

	}
	#endif
};

