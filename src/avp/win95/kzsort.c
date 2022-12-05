#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"

#include "kzsort.h"
#include "kshape.h"
#include "pldnet.h"
#include "avpview.h"

#include "d3d_render.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "avp_userprofile.h"

#define PENTIUM_PROFILING_ON 0
#if PENTIUM_PROFILING_ON
#include "pentime.h"
#else
#define ProfileStart();
#define ProfileStop(x);
#endif
#define MultipleExecuteBuffers Yes

extern int *ItemPointers[maxpolyptrs];
extern int ItemCount;

extern int ScanDrawMode;
extern int ZBufferMode;
extern int NumVertices;
extern int WireFrameMode;
extern int DrawingAReflection;

struct KItem KItemList[maxpolyptrs];
#if 0
static struct KItem KItemList2[maxpolyptrs];
#endif

static struct KObject VisibleModules[MAX_NUMBER_OF_VISIBLE_MODULES];
static struct KObject VisibleModules2[MAX_NUMBER_OF_VISIBLE_MODULES];
static struct KObject *SortedModules;
static struct KObject VisibleObjects[maxobjects];

static int PointIsInModule(VECTORCH *pointPtr,MODULE *modulePtr);

/*KJL*****************************
* externs for new shape function *
*****************************KJL*/
int *MorphedObjectPointsPtr=0;

#if 0
static void MergeItems(struct KItem *src1, int n1, struct KItem *src2, int n2, struct KItem *dest)
{
	/* merge the 2 sorted lists: at src1, length n1, and at src2, length n2, into dest */

	while (n1>0 && n2>0) /* until one list is exhausted */
	{
		if (src1->SortKey < src2->SortKey)
		{
			/* src1 is nearer */
			*dest++ = *src1++;
			n1--;
		}
	 	else
		{
			/* src2 is nearer */
			*dest++ = *src2++;
			n2--;
		}
	}

	if (n1==0)
	{
	   /* remainder in srce2 goes into dest */
	   while (n2>0)
	   {
			*dest++ = *src2++;
			n2--;
	   }
	}
	else
	{
	   /* remainder in srce1 goes into dest */
	   while (n1>0)
	   {
			*dest++ = *src1++;
			n1--;
	   }
	}
}
#endif

static void MergeObjects(struct KObject *src1, int n1, struct KObject *src2, int n2, struct KObject *dest)
{
	/* merge the 2 sorted lists: at src1, length n1, and at src2, length n2, into dest */

	while (n1>0 && n2>0) /* until one list is exhausted */
	{
		if (src1->SortKey < src2->SortKey)
		{
			/* src1 is nearer */
			*dest++ = *src1++;
			n1--;
		}
	 	else
		{
			/* src2 is nearer */
			*dest++ = *src2++;
			n2--;
		}
	}

	if (n1==0)
	{
	   /* remainder in srce2 goes into dest */
	   while (n2>0)
	   {
			*dest++ = *src2++;
			n2--;
	   }
	}
	else
	{
	   /* remainder in srce1 goes into dest */
	   while (n1>0)
	   {
			*dest++ = *src1++;
			n1--;
	   }
	}
}
#if 0
static void ZSortItems(void)
{
	unsigned int partitionSize;
	unsigned int noOfPasses = 0;
	unsigned int noOfItems = ItemCount;
	struct KItem *mergeFrom = &KItemList[0];
	struct KItem *mergeTo = &KItemList2[0];
	struct KItem *mergeTemp;
	unsigned int offSet;

	for (partitionSize=1;partitionSize<noOfItems;partitionSize*=2)
	{
		/* for each partition size...
		   loop through partition pairs and merge */

		/* initialise partition and destination offsets */
		offSet = 0;

		/* do merges for this partition size,
		omitting the last merge if the second partition is incomplete  */
		while((offSet+(partitionSize*2)) <= noOfItems)
		{
			MergeItems(
				(mergeFrom+offSet),
				partitionSize,
				(mergeFrom+offSet+partitionSize),
				partitionSize,
				(mergeTo+offSet) );

			offSet += partitionSize*2;
		}

		/* At this stage, there's less than 2 whole partitions
		left in the array.  If there's no data left at all, then
		there's nothing left to do.  However, if there's any data
		left at the end of the array, we need to do something with it:

		If there's more than a full partition, merge it against the remaining
		partial partition.  If there's less than a full partition, just copy
		it across (via the MergeItems fn): it will be merged in again during a
		later pass.

		*/

		if((offSet+partitionSize) < noOfItems)
		{
			/* merge full partition against a partial partition */
			MergeItems(
				(mergeFrom+offSet),
				partitionSize,
				(mergeFrom+offSet+partitionSize),
				(noOfItems - (offSet+partitionSize)),
				(mergeTo+offSet) );
		}
		else if(offSet < noOfItems)
		{
			/* pass the incomplete partition thro' the merge fn
			   to copy it across */
			MergeItems(
				(mergeFrom+offSet),
				(noOfItems-offSet),
				(mergeFrom+offSet),	/* this is a dummy parameter ... */
				0,
				(mergeTo+offSet) );
		}

		/* count number of passes */
		noOfPasses++;
		/* swap source and destination */
		mergeTemp = mergeFrom;
		mergeFrom = mergeTo;
		mergeTo = mergeTemp;
	}

	/* check where the final list is, and move if neccesary */
	if (noOfPasses%2 == 1)
	{
		unsigned int i;
		/* final list is in the auxiliary buffer, so move it back */
		for(i=0;i<noOfItems;i++)
		{
			KItemList[i] = KItemList2[i];
		}
	}
}
#endif
void SortModules(unsigned int noOfItems)
{
	unsigned int partitionSize;
	unsigned int noOfPasses = 0;

	struct KObject *mergeFrom = &VisibleModules[0];
	struct KObject *mergeTo = &VisibleModules2[0];
	struct KObject *mergeTemp;
	
	unsigned int offSet;

	for (partitionSize=1;partitionSize<noOfItems;partitionSize*=2)
	{
		/* for each partition size...
		   loop through partition pairs and merge */

		/* initialise partition and destination offsets */
		offSet = 0;

		/* do merges for this partition size,
		omitting the last merge if the second partition is incomplete  */
		while((offSet+(partitionSize*2)) <= noOfItems)
		{
			MergeObjects(
				(mergeFrom+offSet),
				partitionSize,
				(mergeFrom+offSet+partitionSize),
				partitionSize,
				(mergeTo+offSet) );

			offSet += partitionSize*2;
		}

		/* At this stage, there's less than 2 whole partitions
		left in the array.  If there's no data left at all, then
		there's nothing left to do.  However, if there's any data
		left at the end of the array, we need to do something with it:

		If there's more than a full partition, merge it against the remaining
		partial partition.  If there's less than a full partition, just copy
		it across (via the MergeObjects fn): it will be merged in again during a
		later pass.

		*/

		if((offSet+partitionSize) < noOfItems)
		{
			/* merge full partition against a partial partition */
			MergeObjects(
				(mergeFrom+offSet),
				partitionSize,
				(mergeFrom+offSet+partitionSize),
				(noOfItems - (offSet+partitionSize)),
				(mergeTo+offSet) );
		}
		else if(offSet < noOfItems)
		{
			/* pass the incomplete partition thro' the merge fn
			   to copy it across */
			MergeObjects(
				(mergeFrom+offSet),
				(noOfItems-offSet),
				(mergeFrom+offSet),	/* this is a dummy parameter ... */
				0,
				(mergeTo+offSet) );
		}

		/* count number of passes */
		noOfPasses++;
		/* swap source and destination */
		mergeTemp = mergeFrom;
		mergeFrom = mergeTo;
		mergeTo = mergeTemp;
	}

	/* check where the final list is, and move if neccesary */
	if (noOfPasses%2 == 1)
	{
		/* final list is in the auxiliary buffer */
		SortedModules = VisibleModules2;
	}
	else
	{
		SortedModules = VisibleModules;
	}
}






/* KJL 12:21:51 02/11/97 - This routine is too big and ugly. Split & clean up required! */
void KRenderItems(VIEWDESCRIPTORBLOCK *VDBPtr)
{
	extern int NumOnScreenBlocks;
	extern DISPLAYBLOCK *OnScreenBlockList[];
	int numOfObjects = NumOnScreenBlocks;
	int numVisMods=0;
	int numVisObjs=0;
	ProfileStart();
	while(numOfObjects)
	{
		extern DISPLAYBLOCK *Player;

		DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
		MODULE *modulePtr = objectPtr->ObMyModule;

		/* if it's a module, which isn't inside another module */
		if (modulePtr && !(modulePtr->m_flags & m_flag_slipped_inside))
		{
			#if 1
 			if(PointIsInModule(&(VDBPtr->VDB_World),modulePtr))
			{
				VisibleModules[numVisMods].DispPtr = objectPtr;
				VisibleModules[numVisMods].SortKey = smallint;
//				textprint("fog is %d in player's module\n",modulePtr->m_flags & MODULEFLAG_FOG);
			}
			#else
			// these tests should really be done using the camera (VDB) position
			extern MODULE *playerPherModule;
			if (playerPherModule == modulePtr)
			{
				VisibleModules[numVisMods].DispPtr = objectPtr;
				VisibleModules[numVisMods].SortKey = smallint;
			}
			#endif
			else
			{
				VECTORCH position;
				VECTORCH dist;

				position.vx = modulePtr->m_world.vx - Player->ObWorld.vx;
				position.vy = modulePtr->m_world.vy - Player->ObWorld.vy;
				position.vz = modulePtr->m_world.vz - Player->ObWorld.vz;

			    {
				    int minX = modulePtr->m_minx + position.vx;
				    int maxX = modulePtr->m_maxx + position.vx;

					if (maxX<0) /* outside maxX side */
					{
						dist.vx = maxX;
					}
					else if (minX>0) /* outside minX faces */
					{
						dist.vx = minX;
					}
					else /* between faces */
					{
						dist.vx = 0;
					}
				}
			    {
				    int minY = modulePtr->m_miny + position.vy;
				    int maxY = modulePtr->m_maxy + position.vy;

					if (maxY<0) /* outside maxY side */
					{
						dist.vy = maxY;
					}
					else if (minY>0) /* outside minY faces */
					{
						dist.vy = minY;
					}
					else /* between faces */
					{
						dist.vy = 0;
					}		  
				}
			    {
				    int minZ = modulePtr->m_minz + position.vz;
				    int maxZ = modulePtr->m_maxz + position.vz;

					if (maxZ<0) /* outside maxZ side */
					{
						dist.vz = maxZ;
					}
					else if (minZ>0) /* outside minZ faces */
					{
						dist.vz = minZ;
					}
					else /* between faces */
					{
						dist.vz = 0;
					}		  
				}

				VisibleModules[numVisMods].DispPtr = objectPtr;
				#if 1
				VisibleModules[numVisMods].SortKey = Magnitude(&dist);
				#else
				VisibleModules[numVisMods].SortKey = MUL_FIXED(dist.vx,dist.vx)
												   + MUL_FIXED(dist.vy,dist.vy)
												   + MUL_FIXED(dist.vz,dist.vz);
				#endif
			}

   			if(numVisMods>MAX_NUMBER_OF_VISIBLE_MODULES)
			{
				/* outside the environment! */
				textprint("MAX_NUMBER_OF_VISIBLE_MODULES (%d) exceeded!\n",MAX_NUMBER_OF_VISIBLE_MODULES);
				textprint("Possibly outside the environment!\n");
//				LOCALASSERT(0);
				return;
			}

			numVisMods++;
   		}
		else /* it's just an object */
		{
			VisibleObjects[numVisObjs].DispPtr = objectPtr;
			/* this sort key defaults to the object not being drawn, ie. a grenade
			behind a closed door (so there is no module behind door) would still be
			in the OnScreenBlockList but need not be drawn. */
			VisibleObjects[numVisObjs].SortKey = 0X7FFFFFFF;
			numVisObjs++;
		}
   	}
	ProfileStop("SORTSETUP");
	textprint("numvismods %d\n",numVisMods);
	textprint("numvisobjs %d\n",numVisObjs);

	ProfileStart();
	#if 1
	{
		int numMods = numVisMods;
		
		while(numMods)
		{
			int n = numMods;

			int furthestModule=0;
			int furthestDistance=0;

		   	while(n)
			{
				n--;
				if (furthestDistance < VisibleModules[n].SortKey)
				{
					furthestDistance = VisibleModules[n].SortKey;
					furthestModule = n;
				}
			}
		
			numMods--;

			VisibleModules2[numMods] = VisibleModules[furthestModule];
			VisibleModules[furthestModule] = VisibleModules[numMods];
			SortedModules = VisibleModules2;
		}
	}
	#else
	SortModules(numVisMods);
	#endif
	ProfileStop("MODULESORT");

	ProfileStart();
	{
#if FOG_ON
		int fogDistance = 0x7f000000;
#endif

		int o = numVisObjs;
		while(o--)
		{	
			DISPLAYBLOCK *objectPtr = VisibleObjects[o].DispPtr;
			int maxX = objectPtr->ObWorld.vx + objectPtr->ObRadius; 
			int minX = objectPtr->ObWorld.vx - objectPtr->ObRadius; 
			int maxZ = objectPtr->ObWorld.vz + objectPtr->ObRadius; 
			int minZ = objectPtr->ObWorld.vz - objectPtr->ObRadius; 
			int maxY = objectPtr->ObWorld.vy + objectPtr->ObRadius; 
			int minY = objectPtr->ObWorld.vy - objectPtr->ObRadius; 
			
			int numMods = 0;
			while(numMods<numVisMods)
			{
				MODULE *modulePtr = SortedModules[numMods].DispPtr->ObMyModule;

				if (maxX >= modulePtr->m_minx+modulePtr->m_world.vx) 
				if (minX <= modulePtr->m_maxx+modulePtr->m_world.vx) 
			    if (maxZ >= modulePtr->m_minz+modulePtr->m_world.vz) 
			    if (minZ <= modulePtr->m_maxz+modulePtr->m_world.vz)
			    if (maxY >= modulePtr->m_miny+modulePtr->m_world.vy) 
			    if (minY <= modulePtr->m_maxy+modulePtr->m_world.vy) 
				{
					VisibleObjects[o].SortKey=numMods;
					break;
				}
				numMods++;
			}
			 #if 0
			/* find nearest module which is fogged */
			if(modulePtr->m_flags & MODULEFLAG_FOG)
			{
				if (fogDistance>SortedModules[numMods].SortKey)
					fogDistance = SortedModules[numMods].SortKey;
			}
			#endif
			if (CurrentVisionMode == VISION_MODE_PRED_SEEALIENS && objectPtr->ObStrategyBlock)
			{
				if (objectPtr->ObStrategyBlock->I_SBtype == I_BehaviourAlien)
					VisibleObjects[o].DrawBeforeEnvironment = 0;//1;
			}
			else
			{
				VisibleObjects[o].DrawBeforeEnvironment = 0;
			}
		}
#if FOG_ON
		if (fogDistance<0) fogDistance=0;
		SetFogDistance(fogDistance);
#endif	
	}
	ProfileStop("OBJS IN MOD TESTS");
	DrawingAReflection=0;
	{
		int numMods = numVisMods;
		#if 1
		{
			int o = numVisObjs;
			CheckWireFrameMode(WireFrameMode&2);
			while(o)
			{
				o--;

				if(VisibleObjects[o].DrawBeforeEnvironment)
				{
					DISPLAYBLOCK *dptr = VisibleObjects[o].DispPtr;
					AddShape(VisibleObjects[o].DispPtr,VDBPtr);	
					#if MIRRORING_ON
					if (MirroringActive && !dptr->HModelControlBlock)
					{
						ReflectObject(dptr);

						MakeVector(&dptr->ObWorld, &VDBPtr->VDB_World, &dptr->ObView);
						RotateVector(&dptr->ObView, &VDBPtr->VDB_Mat);

				  		DrawingAReflection=1;
				  		AddShape(dptr,VDBPtr);
				  		DrawingAReflection=0;
						ReflectObject(dptr);
					}
					#endif
				}
			}
		}

				ClearTranslucentPolyList();

		if (MOTIONBLUR_CHEATMODE)
		{
			for (numMods=0; numMods<numVisMods; numMods++)
			{
				MODULE *modulePtr = SortedModules[numMods].DispPtr->ObMyModule;
				
				CheckWireFrameMode(WireFrameMode&1);
		  		AddShape(SortedModules[numMods].DispPtr,VDBPtr);
				#if MIRRORING_ON
				if (MirroringActive)
				{
					DISPLAYBLOCK *dptr = SortedModules[numMods].DispPtr;
					{
						ReflectObject(dptr);

						MakeVector(&dptr->ObWorld, &VDBPtr->VDB_World, &dptr->ObView);
						RotateVector(&dptr->ObView, &VDBPtr->VDB_Mat);

					  	DrawingAReflection=1;
				  		AddShape(dptr,VDBPtr);
			  			DrawingAReflection=0;
		 				ReflectObject(dptr);
					}
				}
				#endif
				CheckWireFrameMode(WireFrameMode&2);
				{
					int o = numVisObjs;
					while(o)
					{
						o--;

						if(VisibleObjects[o].SortKey == numMods && !VisibleObjects[o].DrawBeforeEnvironment)
						{
							DISPLAYBLOCK *dptr = VisibleObjects[o].DispPtr;
							AddShape(VisibleObjects[o].DispPtr,VDBPtr);	
							#if MIRRORING_ON
							if (MirroringActive && !dptr->HModelControlBlock)
							{
								ReflectObject(dptr);

								MakeVector(&dptr->ObWorld, &VDBPtr->VDB_World, &dptr->ObView);
								RotateVector(&dptr->ObView, &VDBPtr->VDB_Mat);

					  			DrawingAReflection=1;
						  		AddShape(dptr,VDBPtr);
					  			DrawingAReflection=0;
								ReflectObject(dptr);
							}
							#endif
						}
					}
				}


				{
		 			D3D_DrawWaterTest(modulePtr);
				}
			}
		}
		else
		{
			while(numMods--)
			{
				MODULE *modulePtr = SortedModules[numMods].DispPtr->ObMyModule;
				
				CheckWireFrameMode(WireFrameMode&1);
		  		AddShape(SortedModules[numMods].DispPtr,VDBPtr);
				#if MIRRORING_ON
				if (MirroringActive)
				{
					DISPLAYBLOCK *dptr = SortedModules[numMods].DispPtr;
					{
						ReflectObject(dptr);

						MakeVector(&dptr->ObWorld, &VDBPtr->VDB_World, &dptr->ObView);
						RotateVector(&dptr->ObView, &VDBPtr->VDB_Mat);

					  	DrawingAReflection=1;
				  		AddShape(dptr,VDBPtr);
			  			DrawingAReflection=0;
		 				ReflectObject(dptr);
					}
				}
				#endif
				CheckWireFrameMode(WireFrameMode&2);
				{
					int o = numVisObjs;
					while(o)
					{
						o--;

						if(VisibleObjects[o].SortKey == numMods && !VisibleObjects[o].DrawBeforeEnvironment)
						{
							DISPLAYBLOCK *dptr = VisibleObjects[o].DispPtr;
							AddShape(VisibleObjects[o].DispPtr,VDBPtr);	
							#if MIRRORING_ON
							if (MirroringActive && !dptr->HModelControlBlock)
							{
								ReflectObject(dptr);

								MakeVector(&dptr->ObWorld, &VDBPtr->VDB_World, &dptr->ObView);
								RotateVector(&dptr->ObView, &VDBPtr->VDB_Mat);

					  			DrawingAReflection=1;
						  		AddShape(dptr,VDBPtr);
					  			DrawingAReflection=0;
								ReflectObject(dptr);
							}
							#endif
						}
					}
				}


				{
		 			D3D_DrawWaterTest(modulePtr);
				}
			}
		}
//	 			OutputTranslucentPolyList();
		#endif
		#if 0
		{
			int numMods = numVisMods;
			while(numMods--)
		  		AddShape(SortedModules[numMods].DispPtr,VDBPtr);
		}
		#endif
		/* KJL 12:51:00 13/08/98 - scan for hierarchical objects which aren't going to be drawn,
		and update their timers */
		{
			int o = numVisObjs;
			while(o)
			{
				o--;

				if(VisibleObjects[o].SortKey == 0x7fffffff)
				{
					if(VisibleObjects[o].DispPtr->HModelControlBlock)
					{
						DoHModelTimer(VisibleObjects[o].DispPtr->HModelControlBlock);
					}
				}
			}
		}
		
	#if MIRRORING_ON
		if (MirroringActive)
		{
			DrawingAReflection=1;
			RenderPlayersImageInMirror();
			DrawingAReflection=0;
		}

	#endif

		#if 0
		if (ScanDrawMode != ScanDrawDirectDraw)
		{
			WriteEndCodeToExecuteBuffer();
		 	UnlockExecuteBufferAndPrepareForUse();
			ExecuteBuffer();
			EndD3DScene();
		}
		#endif

		if (ScanDrawMode == ScanDrawDirectDraw) 
		{
			UnlockSurface();
		}

	}
}

#if 0
static int ObjectIsInModule(DISPLAYBLOCK *objectPtr,MODULE *modulePtr)
{
	int objectSize = objectPtr->ObRadius;
	VECTORCH position = objectPtr->ObWorld;

	position.vx -= modulePtr->m_world.vx;
	position.vy -= modulePtr->m_world.vy;
	position.vz -= modulePtr->m_world.vz;

	if (position.vx + objectSize >= modulePtr->m_minx) 
    	if (position.vx - objectSize <= modulePtr->m_maxx) 
		    if (position.vz + objectSize >= modulePtr->m_minz) 
			    if (position.vz - objectSize <= modulePtr->m_maxz) 
				    if (position.vy + objectSize >= modulePtr->m_miny) 
					    if (position.vy - objectSize <= modulePtr->m_maxy)
							return 1;

	return 0;

}
#endif

static int PointIsInModule(VECTORCH *pointPtr,MODULE *modulePtr)
{
	VECTORCH position = *pointPtr;
	position.vx -= modulePtr->m_world.vx;
	position.vy -= modulePtr->m_world.vy;
	position.vz -= modulePtr->m_world.vz;

	if (position.vx >= modulePtr->m_minx) 
    	if (position.vx <= modulePtr->m_maxx) 
		    if (position.vz >= modulePtr->m_minz) 
			    if (position.vz <= modulePtr->m_maxz) 
				    if (position.vy >= modulePtr->m_miny) 
					    if (position.vy <= modulePtr->m_maxy)
							return 1;
	return 0;

}



void RenderThisDisplayblock(DISPLAYBLOCK *dbPtr)
{
	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

  	AddShape(dbPtr,VDBPtr);
}

void RenderThisHierarchicalDisplayblock(DISPLAYBLOCK *dbPtr)
{
	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

  	AddHierarchicalShape(dbPtr,VDBPtr);
	#if MIRRORING_ON
	if (MirroringActive && dbPtr->ObStrategyBlock)
	{
		ReflectObject(dbPtr);

		MakeVector(&dbPtr->ObWorld, &VDBPtr->VDB_World, &dbPtr->ObView);
		RotateVector(&dbPtr->ObView, &VDBPtr->VDB_Mat);

	  	AddHierarchicalShape(dbPtr,VDBPtr);
		ReflectObject(dbPtr);
	}
	#endif
}





/* KJL 10:33:03 7/9/97 - this code no longer used */

#if 0
void OutputKItem(int *shapeitemptr)
{
	/* Allocate space in the Item Data array */
	if (IPtsArrSize)
	{
		int *itemDataPtr = AllocateItemData(IHdrSize + IPtsArrSize + ITrmSize);

		if(itemDataPtr)
		{
	   		POLYHEADER *mypolyheader = (POLYHEADER*) itemDataPtr;
			struct KItem * const currentItemPtr = &KItemList[ItemCount];

			currentItemPtr->PolyPtr = mypolyheader;			   

			{
				POLYHEADER* polyheader = (POLYHEADER *)shapeitemptr;
				int *offsetPtr = (int*) &polyheader->Poly1stPt;
				
				int maxZ = smallint;
				int minZ = bigint;
				
				if (MorphedObjectPointsPtr)
				{
					while(*offsetPtr != Term)
					{
						VECTORCH v = *(VECTORCH*)((int *)MorphedObjectPointsPtr + *offsetPtr);
						int z;

						z =  MUL_FIXED(LToVMat.mat13, v.vx);
						z += MUL_FIXED(LToVMat.mat23, v.vy);
						z += MUL_FIXED(LToVMat.mat33, v.vz);
						z += Global_ODB_Ptr->ObView.vz;
						
						if(z > maxZ) maxZ = z;
						else if(z < minZ) minZ = z;
						
					   	offsetPtr++;
					}
				}
				else
				{
					while(*offsetPtr != Term)
					{
						int z = *((int *)RotatedPts + *offsetPtr + iz);
						
						if(z > maxZ) maxZ = z;
						else if(z < minZ) minZ = z;
						
					   	offsetPtr++;
					}
				}
			
				if (polyheader->PolyFlags & iflag_sortnearz) currentItemPtr->SortKey = minZ;
				else if (polyheader->PolyFlags & iflag_sortfarz) currentItemPtr->SortKey = maxZ +10;
				else currentItemPtr->SortKey = maxZ;
			}

			ItemCount++;

			/* Write out the Item Header */

			mypolyheader->PolyItemType    = *shapeitemptr++;
			mypolyheader->PolyNormalIndex = *shapeitemptr++;
			mypolyheader->PolyFlags       = *shapeitemptr++;
			mypolyheader->PolyColour      = ItemColour;

			/* Write out the Item Points Array */
			{
				int *ptsArrayPtr = &PointsArray[0];
				int i = IPtsArrSize;

				itemDataPtr = &mypolyheader->Poly1stPt;
				
				do
				{
					*itemDataPtr++ = *ptsArrayPtr++;
				}
				while(--i);
			}

			/* Write out the Item Terminator */
			*itemDataPtr = Term;
		}
	}
}

void KShapeItemsInstr(SHAPEINSTR *shapeinstrptr)
{
	int numitems= shapeinstrptr->sh_numitems;
	int **shapeitemarrayptr = shapeinstrptr->sh_instr_data;

	while(numitems)
	{
		int clip_output;
		int *shapeitemptr = *shapeitemarrayptr;
		POLYHEADER *pheaderPtr = (POLYHEADER*) shapeitemptr;

		GLOBALASSERT(*shapeitemptr < I_Last);
	   

		if((Global_ODB_Ptr->ObFlags & ObFlag_BFCRO)
		 ||(Global_ODB_Ptr->ObFlags & ObFlag_ParrallelBFC))
		{
			if((pheaderPtr->PolyFlags & iflag_viewdotpos) == 0)
			{
				ItemOCSBlock.ocs_flags |= ocs_flag_outcoded;
			}
			else
			{
				ItemOCSBlock.ocs_flags = ocs_flag_nobfc;
				OutcodeItem[*shapeitemptr](shapeitemptr, &ItemOCSBlock);
			}
		}
		else
		{
			ItemOCSBlock.ocs_flags = 0;
			OutcodeItem[*shapeitemptr](shapeitemptr, &ItemOCSBlock);
		}


		if((ItemOCSBlock.ocs_flags & ocs_flag_outcoded) == 0)
		{
			#if support3dtextures
			if((pheaderPtr->PolyFlags & iflag_tx2dor3d)
				&& pheaderPtr->PolyItemType == I_2dTexturedPolygon)
				pheaderPtr->PolyItemType = I_3dTexturedPolygon;
			#if SupportZBuffering
			if((pheaderPtr->PolyFlags & iflag_tx2dor3d)
				&& pheaderPtr->PolyItemType == I_ZB_2dTexturedPolygon)
				pheaderPtr->PolyItemType = I_ZB_3dTexturedPolygon;
			#endif
			#endif

			#if SupportGouraud3dTextures
			if((pheaderPtr->PolyFlags & iflag_tx2dor3d)
				&& pheaderPtr->PolyItemType == I_Gouraud2dTexturedPolygon)
				pheaderPtr->PolyItemType = I_Gouraud3dTexturedPolygon;
			#if SupportZBuffering
			if((pheaderPtr->PolyFlags & iflag_tx2dor3d)
				&& pheaderPtr->PolyItemType == I_ZB_Gouraud2dTexturedPolygon)
				pheaderPtr->PolyItemType = I_ZB_Gouraud3dTexturedPolygon;
			#endif
			#endif
	   		
			/* KJL 14:08:52 04/26/97 - hack to draw everything as a certain poly type */
			#if 0
			pheaderPtr->PolyItemType = I_2dTexturedPolygon;
			#endif

			/* KJL 17:15:46 06/07/97 - I'm not sure that we need 3dTexturedPolys */
			if(pheaderPtr->PolyItemType == I_3dTexturedPolygon)
				pheaderPtr->PolyItemType = I_2dTexturedPolygon;

			if(ItemOCSBlock.ocs_clipstate == ocs_cs_totally_on)
			{
				CreateItemPtsArray[*shapeitemptr](shapeitemptr, &PointsArray[0]);
				
				if(Global_ShapeNormals)
					ItemColour = ItemLightingModel[*shapeitemptr](shapeitemptr);
				else
					ItemColour = pheaderPtr->PolyColour;

				OutputKItem(shapeitemptr);
			}
			else
			{
				#if 0
				/* Because in d3d polygons are rendered as triangles
				   the splits made to turn n-gons into triangles must
				   be invariant with the clipping performed on the poly,
				   otherwise artefacts will occur as polygons are clipped
				   to fit into screen space, such as the apparent intensity
				   changing on a clipper-created vertex.
				   This is difficult to achieve; the best way, which seems
				   to work well is to split the quads into triangles before
				   clipping - splitting them across the same diagonal as the
				   renderer does. */
				int num_verts = 0;
				if (ScanDrawDirectDraw != ScanDrawMode)
				{
					/* count the number of verts - see if its a quad */
					int * ptsptr = &((POLYHEADER *)shapeitemptr)->Poly1stPt;
					while (Term != *ptsptr++)
						++num_verts;
					/* shouldn't have pentagons etc. at this stage */
					GLOBALASSERT(num_verts<=4);
				}
				/* FIXME:
				   I am not doing this quad splitting on textures with iflag_txanim set
				   This is rather more complicated than I care to imagine - the uv
				   co-ords which also need splitting are generated in a function
				   called CreateTxAnimUVArray which also seems to do a lot more than
				   just that. Bloody Chris Humphries.
				   Anyway, I think the lighting artefact will be less noticeable if the texture is animating */
				if (4==num_verts && !(((POLYHEADER *)shapeitemptr)->PolyFlags & iflag_txanim))
					/* need to split quads in d3d mode -
					   note that num_verts will be 0 in scandraw mode
					   so this condition will always be false */
				{
					/* get pointers to the output items - we may need to adjust the sort key */
					struct KItem * triangle1 = 0;
					struct KItem * triangle2 = 0;
					/* For a quad we create the triangles (0,1,3) and (1,2,3) */
					/* (see item.c - PrepareTriangleArray_4 functions) */
					/* copy the header data for the quad into this buffer */
					int item_array_buf[sizeof(POLYHEADER)+sizeof(int)*4]; /* 4 verts + polyheader & term */
					/* save the previous uv data in this buffer and modify the actual uv data */
					int uv_array_store[8]; /* 4 verts, U and V foreach vert */
					/* this points to the uv data we are going to save and modify */
					int * uv_array_ptr = NULL; /* will set this non-null if we have a textured item type */
					if (
						I_2dTexturedPolygon == *shapeitemptr ||
						I_Gouraud2dTexturedPolygon == *shapeitemptr ||
						I_3dTexturedPolygon == *shapeitemptr ||
						I_UnscaledSprite == *shapeitemptr ||
						I_ScaledSprite == *shapeitemptr ||
						I_ZB_2dTexturedPolygon == *shapeitemptr ||
						I_ZB_Gouraud2dTexturedPolygon == *shapeitemptr ||
						I_ZB_3dTexturedPolygon == *shapeitemptr ||
						I_Gouraud3dTexturedPolygon == *shapeitemptr ||
						I_ZB_Gouraud3dTexturedPolygon == *shapeitemptr
					) /* textured item type - has uv coords */
						uv_array_ptr = Global_ShapeTextures[((POLYHEADER *)shapeitemptr)->PolyColour >> TxDefn];
					/* copy the header data for the quad into the buffer */
					/* ...and the first two verts since these are the same for the first triangle as for the quad */
					memcpy(item_array_buf,shapeitemptr,sizeof(POLYHEADER)-sizeof(int)+2*sizeof(int));
					/* save the uv data */
					if (uv_array_ptr) memcpy(uv_array_store,uv_array_ptr,sizeof uv_array_store);
					/* select vertices for the first triangle */
					#define PFP_OFFSET (sizeof(POLYHEADER)/sizeof(int)-1) /* normally 4 but what if polyheader changes */
					LOCALASSERT(item_array_buf[PFP_OFFSET+0] == shapeitemptr[PFP_OFFSET+0]);
					LOCALASSERT(item_array_buf[PFP_OFFSET+1] == shapeitemptr[PFP_OFFSET+1]);
					item_array_buf[PFP_OFFSET+2] = shapeitemptr[PFP_OFFSET+3];
					item_array_buf[PFP_OFFSET+3] = Term;
					if (uv_array_ptr)
					{
						/* select the uv coords for the first triangle */
						LOCALASSERT(uv_array_ptr[0] == uv_array_store[0]); /* U */
						LOCALASSERT(uv_array_ptr[1] == uv_array_store[1]); /* V */
						LOCALASSERT(uv_array_ptr[2] == uv_array_store[2]); /* U */
						LOCALASSERT(uv_array_ptr[3] == uv_array_store[3]); /* V */
						uv_array_ptr[4] = uv_array_store[6]; /* U */
						uv_array_ptr[5] = uv_array_store[7]; /* V */
					}
					/* output it in the usual way */
					if(ItemOCSBlock.ocs_ptsoutstate == ocs_pout_2d)
					{
						CreateItemPtsArray[*item_array_buf](item_array_buf, &ClipPointsArray0[0]);
						clip_output = Clip2d[*item_array_buf](item_array_buf);
					}
					else
					{
						/* also ensure both polygons are rendered in 3d */
						((POLYHEADER *)item_array_buf)->PolyFlags &= ~iflag_tx2dor3d;
						CreateItemPtsArray_Clip3d[*item_array_buf](item_array_buf, &ClipPointsArray0[0]);
						clip_output = Clip3d[*item_array_buf](item_array_buf);
					}
					if(clip_output)
					{

						if(Global_ShapeNormals)
							ItemColour = ItemLightingModel[*item_array_buf](item_array_buf);
						else
							ItemColour = pheaderPtr->PolyColour;

						triangle1 = OutputKItem(item_array_buf);
					}
					/* copy the header data for the quad into the buffer again
					   because the outcode function fuck around with it,,, */
					memcpy(item_array_buf,shapeitemptr,sizeof(POLYHEADER)-sizeof(int));
					/* select vertices for the second triangle */
					item_array_buf[PFP_OFFSET+0] = shapeitemptr[PFP_OFFSET+1];
					item_array_buf[PFP_OFFSET+1] = shapeitemptr[PFP_OFFSET+2];
					LOCALASSERT(item_array_buf[PFP_OFFSET+2] == shapeitemptr[PFP_OFFSET+3]);
					LOCALASSERT(item_array_buf[PFP_OFFSET+3] == Term);
					if (uv_array_ptr)
					{
						/* select the uv coords for the first triangle */
						uv_array_ptr[0] = uv_array_store[2]; /* U */
						uv_array_ptr[1] = uv_array_store[3]; /* V */
						uv_array_ptr[2] = uv_array_store[4]; /* U */
						uv_array_ptr[3] = uv_array_store[5]; /* V */
						LOCALASSERT(uv_array_ptr[4] == uv_array_store[6]); /* U */
						LOCALASSERT(uv_array_ptr[5] == uv_array_store[7]); /* V */
					}
					/* output it in the usual way */
					if(ItemOCSBlock.ocs_ptsoutstate == ocs_pout_2d)
					{
						CreateItemPtsArray[*item_array_buf](item_array_buf, &ClipPointsArray0[0]);
						clip_output = Clip2d[*item_array_buf](item_array_buf);
					}
					else
					{
						/* also ensure both polygons are rendered in 3d */
						((POLYHEADER *)item_array_buf)->PolyFlags &= ~iflag_tx2dor3d;
						CreateItemPtsArray_Clip3d[*item_array_buf](item_array_buf, &ClipPointsArray0[0]);
						clip_output = Clip3d[*item_array_buf](item_array_buf);
					}
					if(clip_output)
					{

						if(Global_ShapeNormals)
							ItemColour = ItemLightingModel[*item_array_buf](item_array_buf);
						else
							ItemColour = pheaderPtr->PolyColour;

						triangle2 = OutputKItem(item_array_buf);
					}
					/* restore the uv data */
					if (uv_array_ptr) memcpy(uv_array_ptr,uv_array_store,sizeof uv_array_store);
					/* if both triangles have been output,
					   we may need to change the sort keys on one or both,
					   so that far z and near z values will hopefully be
					   the same as if it was output as a quad */
					if (triangle1 && triangle2)
					{
						if (((POLYHEADER *)shapeitemptr)->PolyFlags & iflag_sortnearz)
						{
							/* take minimum sort keys */
							if (triangle1->SortKey < triangle2->SortKey)
								triangle2->SortKey = triangle1->SortKey;
							else
								triangle1->SortKey = triangle2->SortKey;
						}
						else
						{
							/* since sortfarz and no sort flag both take the largest z value
							   take maximum sort keys */
							if (triangle1->SortKey > triangle2->SortKey)
								triangle2->SortKey = triangle1->SortKey;
							else
								triangle1->SortKey = triangle2->SortKey;
						}
					}
				}
				else
				#endif
				{
					if(ItemOCSBlock.ocs_ptsoutstate == ocs_pout_2d)
					{
						CreateItemPtsArray[*shapeitemptr](shapeitemptr, &ClipPointsArray0[0]);
						clip_output = Clip2d[*shapeitemptr](shapeitemptr);
					}
					else
					{
						CreateItemPtsArray_Clip3d[*shapeitemptr](shapeitemptr, &ClipPointsArray0[0]);
						clip_output = Clip3d[*shapeitemptr](shapeitemptr);
					}
					if(clip_output)
					{

						if(Global_ShapeNormals)
							ItemColour = ItemLightingModel[*shapeitemptr](shapeitemptr);
						else
							ItemColour = pheaderPtr->PolyColour;

						OutputKItem(shapeitemptr);
					}
				}
			}
		}

		shapeitemarrayptr++;			/* next polygon etc. */
		numitems--;
	}
}
#endif


