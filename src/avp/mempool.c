#include "ourasert.h"
#include "mem3dc.h"
#include "mempool.h"


#if USE_LEVEL_MEMORY_POOL

#define MAX_NUM_MEMORY_BLOCK 64
#define MEMORY_BLOCK_SIZE (8192*1024)

static char* MemoryBlocks[MAX_NUM_MEMORY_BLOCK];
static int CurrentMemoryBlock =-1;

static char* MemoryPoolPtr=0;
static unsigned int MemoryLeft=0;

static size_t AllocationCount=0;
static size_t LargestRequest=0;
static size_t MemoryRequested=0;
static size_t MemoryWasted=0;

void* PoolAllocateMem(unsigned int amount)
{
	char* retval;

	GLOBALASSERT(amount<=MEMORY_BLOCK_SIZE)
	if (amount > MEMORY_BLOCK_SIZE)
	{
		// fatal error
		return NULL;
	}
	
	// align up
	amount = (amount + 7) & ~7;

	if(amount>MemoryLeft)
	{
		MemoryWasted += MemoryLeft;

		CurrentMemoryBlock++;
		GLOBALASSERT(CurrentMemoryBlock<MAX_NUM_MEMORY_BLOCK);
		if (CurrentMemoryBlock >= MAX_NUM_MEMORY_BLOCK)
		{
			// fatal error
			return NULL;
		}
		MemoryBlocks[CurrentMemoryBlock]=AllocateMem(MEMORY_BLOCK_SIZE);
		GLOBALASSERT(MemoryBlocks[CurrentMemoryBlock]!=NULL);
		if (MemoryBlocks[CurrentMemoryBlock] == NULL)
		{
			// fatal error
			return NULL;
		}
		
		MemoryLeft=MEMORY_BLOCK_SIZE;
		MemoryPoolPtr=MemoryBlocks[CurrentMemoryBlock];
	}

	if (amount > LargestRequest) {
		LargestRequest = amount;
	}

	MemoryRequested+=amount;
	AllocationCount++;

	retval=MemoryPoolPtr;
	MemoryLeft-=amount;
	MemoryPoolPtr+=amount;
	return (void*)retval;
	
}


void ClearMemoryPool()
{
	int i;

#if !defined(NDEBUG)
	printf("%d blocks in use, %u bytes left in current block\n", CurrentMemoryBlock + 1, MemoryLeft);
	printf("%zu requests, %zu bytes requested, %zu bytes wasted, %zu largest request\n", AllocationCount, MemoryRequested, MemoryWasted, LargestRequest);
#endif

	for(i=0;i<=CurrentMemoryBlock;i++)
	{
		DeallocateMem(MemoryBlocks[i]);
		MemoryBlocks[i]=0;
	}
	CurrentMemoryBlock=-1;
	MemoryPoolPtr=0;
	MemoryLeft=0;

	MemoryRequested = 0;
	MemoryWasted = 0;
	LargestRequest = 0;
	AllocationCount = 0;
}

#endif
