
#include "3dc.h"
#include "module.h"

#include "stratdef.h"
#include "sfx.h"

#define UseLocalAssert Yes
#include "ourasert.h"


/* globals for export */

int NumActiveBlocks;
DISPLAYBLOCK *ActiveBlockList[maxobjects];




/*
 Object Block Lists et al
*/

static int NumFreeBlocks;
static DISPLAYBLOCK *FreeBlockList[maxobjects];
static DISPLAYBLOCK **FreeBlockListPtr = &FreeBlockList[maxobjects-1];
static DISPLAYBLOCK FreeBlockData[maxobjects];
static DISPLAYBLOCK **ActiveBlockListPtr = &ActiveBlockList[0];


/*
 Texture Animation Block Extensions
*/

static int NumFreeTxAnimBlocks;
static TXACTRLBLK *FreeTxAnimBlockList[maxTxAnimblocks];
static TXACTRLBLK **FreeTxAnimBlockListPtr = &FreeTxAnimBlockList[maxTxAnimblocks-1];
static TXACTRLBLK FreeTxAnimBlockData[maxTxAnimblocks];



/*
 Light Block Extensions
*/

static int NumFreeLightBlocks;
static LIGHTBLOCK *FreeLightBlockList[maxlightblocks];
static LIGHTBLOCK **FreeLightBlockListPtr = &FreeLightBlockList[maxlightblocks-1];
static LIGHTBLOCK FreeLightBlockData[maxlightblocks];



/*

 To create the free block list, pointers to "FreeBlockData[]" must be copied
 to "FreeBlockList[]".

 Also:

 "NumFreeBlocks" must be updated as "FreeBlockList[]" is created.
 "NumActiveBlocks" must be initialised to zero.

*/
void InitialiseObjectBlocks(void)
{

	DISPLAYBLOCK *FreeBlkPtr = &FreeBlockData[0];

	NumActiveBlocks = 0;

	FreeBlockListPtr   = &FreeBlockList[maxobjects-1];
	ActiveBlockListPtr = &ActiveBlockList[0];

	for(NumFreeBlocks = 0; NumFreeBlocks<maxobjects; NumFreeBlocks++) {

		FreeBlockList[NumFreeBlocks] = FreeBlkPtr;

		FreeBlkPtr++;

	}
}


/*

 "AllocateObjectBlock()" is identical to the routine "GetBlock"

*/

DISPLAYBLOCK* AllocateObjectBlock(void)
{

	DISPLAYBLOCK *FreeBlkPtr = 0;		/* Default to null ptr */
	int *sptr;
	int i;


	if(NumFreeBlocks) {

		FreeBlkPtr = *FreeBlockListPtr--;

		NumFreeBlocks--;					/* One less free block */

		/* Clear the block */

		sptr = (int *)FreeBlkPtr;
		for(i = sizeof(DISPLAYBLOCK)/4; i!=0; i--)
			*sptr++ = 0;
	}

	return(FreeBlkPtr);
}


/*

 "DeallocateObjectBlock()" is identical to the routine "ReturnBlock"

*/

void DeallocateObjectBlock(DISPLAYBLOCK *dblockptr)
{
	/* Deallocate the Display Block */

	FreeBlockListPtr++;
	*FreeBlockListPtr = dblockptr;

	NumFreeBlocks++;						/* One more free block */
}


/*
 "CreateActiveObject()" calls "AllocateObjectBlock()". An active object is
 passed into the view and strategy routines unless flagged otherwise

 WARNING!

 An active object must ALWAYS be deallocated by "DestroyActiveObject()".
*/

DISPLAYBLOCK* CreateActiveObject(void)
{

	DISPLAYBLOCK *dblockptr;


	dblockptr = AllocateObjectBlock();

	if(dblockptr) {

		*ActiveBlockListPtr++ = dblockptr;

		NumActiveBlocks++;


	}

	return dblockptr;
}


/*

 DestroyActiveObject()

 Remove the block from "ActiveBlockList".
 Use the array model because it's clearer.

 This function returns 0 if successful, -1 if not

*/


int DestroyActiveObject(DISPLAYBLOCK *dblockptr)
{
	int i, light;
	TXACTRLBLK *taptr;

	/* If the block ptr is OK, search the Active Blocks List */
	if(dblockptr) {

		for(i = 0; i < NumActiveBlocks; i++) {

			if(ActiveBlockList[i] == dblockptr) {

				ActiveBlockList[i] = ActiveBlockList[NumActiveBlocks-1];
				NumActiveBlocks--;
				ActiveBlockListPtr--;

				DestroyActiveVDB(dblockptr->ObVDBPtr);	/* Checks for null */

				if(dblockptr->ObNumLights) {
					for(light = dblockptr->ObNumLights - 1; light != -1; light--)
						DeleteLightBlock(dblockptr->ObLights[light], dblockptr);
				}

				/* If no SB, deallocate any Texture Animation Blocks */

				if(dblockptr->ObStrategyBlock == 0) {

					if(dblockptr->ObTxAnimCtrlBlks) {

						taptr = dblockptr->ObTxAnimCtrlBlks;

						while(taptr) {

							DeallocateTxAnimBlock(taptr);

							taptr = taptr->tac_next;

						}

					}

				}


				/* Deallocate the Lazy Morphed Points Array Pointer */

				#if (SupportMorphing && LazyEvaluationForMorphing)
				if(dblockptr->ObMorphedPts) {
					DeallocateMem(dblockptr->ObMorphedPts);
					dblockptr->ObMorphedPts = 0;
				}
				#endif

				/* KJL 16:52:43 06/01/98 - dealloc sfx block if one exists */
				if(dblockptr->SfxPtr)
				{
					DeallocateSfxBlock(dblockptr->SfxPtr);
				}

				DeallocateObjectBlock(dblockptr);		/* Back to Free List */

				/* If this is the current landscape, clear the pointer */

				return 0;
			}
		}
	}

	return -1;
}




/*

 Support Functions for Texture Animation Blocks

*/

void InitialiseTxAnimBlocks(void)

{

	TXACTRLBLK *FreeBlkPtr = &FreeTxAnimBlockData[0];


	FreeTxAnimBlockListPtr = &FreeTxAnimBlockList[maxTxAnimblocks-1];

	for(NumFreeTxAnimBlocks=0; NumFreeTxAnimBlocks < maxTxAnimblocks; NumFreeTxAnimBlocks++) {

		FreeTxAnimBlockList[NumFreeTxAnimBlocks] = FreeBlkPtr;

		FreeBlkPtr++;

	}

}


/*

 Allocate a Texture Animation Block

*/

TXACTRLBLK* AllocateTxAnimBlock(void)

{

	TXACTRLBLK *FreeBlkPtr = 0;		/* Default to null ptr */
	int *sptr;
	int i;


	if(NumFreeTxAnimBlocks) {

		FreeBlkPtr = *FreeTxAnimBlockListPtr--;

		NumFreeTxAnimBlocks--;					/* One less free block */

		/* Clear the block */

		sptr = (int *)FreeBlkPtr;
		for(i = sizeof(TXACTRLBLK)/4; i!=0; i--)
			*sptr++ = 0;

	}

	return FreeBlkPtr;

}


/*

 Deallocate a Texture Animation Block

*/

void DeallocateTxAnimBlock(TXACTRLBLK *TxAnimblockptr)

{

	FreeTxAnimBlockListPtr++;

	*FreeTxAnimBlockListPtr = TxAnimblockptr;

	NumFreeTxAnimBlocks++;						/* One more free block */

}


/*

 Add a Texture Animation Block to a Display Block

*/

void AddTxAnimBlock(DISPLAYBLOCK *dptr, TXACTRLBLK *taptr)

{

	TXACTRLBLK *taptr_tmp;


	if(dptr->ObTxAnimCtrlBlks) {

		taptr_tmp = dptr->ObTxAnimCtrlBlks;

		while(taptr_tmp->tac_next)
			taptr_tmp = taptr_tmp->tac_next;

		taptr_tmp->tac_next = taptr;

	}

	else dptr->ObTxAnimCtrlBlks = taptr;

}





/*

 Support functions for Light Blocks

*/

void InitialiseLightBlocks(void)

{

	LIGHTBLOCK *FreeBlkPtr = &FreeLightBlockData[0];


	FreeLightBlockListPtr = &FreeLightBlockList[maxlightblocks-1];

	for(NumFreeLightBlocks=0; NumFreeLightBlocks < maxlightblocks; NumFreeLightBlocks++) {

		FreeLightBlockList[NumFreeLightBlocks] = FreeBlkPtr;

		FreeBlkPtr++;

	}

}


LIGHTBLOCK* AllocateLightBlock(void)

{

	LIGHTBLOCK *FreeBlkPtr = 0;		/* Default to null ptr */
	int *lptr;
	int i;


	if(NumFreeLightBlocks) {

		FreeBlkPtr = *FreeLightBlockListPtr--;

		NumFreeLightBlocks--;					/* One less free block */

		/* Clear the block */

		lptr = (int *)FreeBlkPtr;
		for(i = sizeof(LIGHTBLOCK)/4; i!=0; i--)
			*lptr++ = 0;

	}

	return(FreeBlkPtr);

}


void DeallocateLightBlock(LIGHTBLOCK *lptr)

{

	/* Not all lights come from the free light list */

	if(lptr->LightFlags & LFlag_WasNotAllocated) return;


	/* Make sure that this light IS from the free light list */

	GLOBALASSERT(
		(lptr >= FreeLightBlockData) &&
		(lptr < &FreeLightBlockData[maxlightblocks])
	);


	/* Ok to return the light */

	FreeLightBlockListPtr++;

	*FreeLightBlockListPtr = lptr;

	NumFreeLightBlocks++;						/* One more free block */

}


/*

 See if there are any free slots in the dptr light block array.
 If there are, allocate a light block, place it in the list and return
 the pointer to the caller.

 A late addition is the passing of a light block (from somewhere, it does
 not matter where). This light block is then added rather than one being
 allocated from the free light block list.

*/

LIGHTBLOCK* AddLightBlock(DISPLAYBLOCK *dptr, LIGHTBLOCK *lptr_to_add)

{

	LIGHTBLOCK **larrayptr;
	LIGHTBLOCK **freelarrayptr;
	LIGHTBLOCK *lptr = 0;
	int i, lfree;


	/* Are there any free slots? */

	lfree = No;

	larrayptr = &dptr->ObLights[0];
	freelarrayptr = NULL;
	
	for(i = MaxObjectLights; i!=0 && lfree == No; i--) {

		if(*larrayptr == 0) {

			freelarrayptr = larrayptr;
			lfree = Yes;

		}

		larrayptr++;

	}

	if(lfree) {

		if(lptr_to_add) {

			lptr = lptr_to_add;

		}

		else {

			lptr = AllocateLightBlock();

		}

		if(lptr)
		{
			*freelarrayptr = lptr;
			dptr->ObNumLights++;
		}

	}

	return lptr;

}


/*

 To delete a light block, copy the end block to the new free slot and
 reduce the count by one. Make sure the end block array entry is cleared.

*/

void DeleteLightBlock(LIGHTBLOCK *lptr, DISPLAYBLOCK *dptr)
{

	int i, larrayi;

	DeallocateLightBlock(lptr);

	/* What is lptr's array index? */

	larrayi = -1;							/* null value */

	for(i = 0; i < dptr->ObNumLights; i++)
		if(dptr->ObLights[i] == lptr) larrayi = i;



	/* Proceed only if lptr has been found in the array */

	if(larrayi != -1) {

		/* Copy the end block to that of lptr */

		dptr->ObLights[larrayi] = dptr->ObLights[dptr->ObNumLights - 1];

		/* Clear the end block array entry */

		dptr->ObLights[dptr->ObNumLights - 1] = 0;

		/* One less light in the dptr list */

		dptr->ObNumLights--;

	}
}




/*

 When running the parallel strategies, display and light block deallocation
 must only be done at the end of the frame, AFTER processor synchronisation
 and BEFORE the shadow copy.

*/

int DisplayAndLightBlockDeallocation(void)
{

	DISPLAYBLOCK **activeblocksptr;
	DISPLAYBLOCK *dptr;
	int i, j;
	LIGHTBLOCK *lptr;

	if(NumActiveBlocks) {

		activeblocksptr = &ActiveBlockList[NumActiveBlocks - 1];

		for(i = NumActiveBlocks; i!=0; i--) {

			dptr = *activeblocksptr--;

			/* Deallocate Object? */

			if(dptr->ObFlags2 & ObFlag2_Deallocate) {

				DestroyActiveObject(dptr);

			}


			/* Deallocate any Lights? */

			else {

				if(dptr->ObNumLights) {

					for(j = dptr->ObNumLights - 1; j > -1; j--) {

						lptr = dptr->ObLights[j];

						if(lptr->LightFlags & LFlag_Deallocate) {

							DeleteLightBlock(dptr->ObLights[j], dptr);

						}

					}

				}

			}

		}

	}
	return 0;
}
