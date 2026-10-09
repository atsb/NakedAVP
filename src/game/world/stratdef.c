#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "dynblock.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "bh_alien.h"
#include "bh_marin.h"
#include "bh_xeno.h"
#include "bh_corpse.h"
#include "bh_debri.h"
#include "pldnet.h"
#include "maths.h"
/* 
	this attaches runtime and precompiled object
	strategyblocks
*/

/*** exported globals *************/

int NumActiveStBlocks;
STRATEGYBLOCK  *ActiveStBlockList[maxstblocks];


/*** static globals ***************/

static int NumFreeStBlocks;
static STRATEGYBLOCK *FreeStBlockList[maxstblocks];
static STRATEGYBLOCK **FreeStBlockListPtr = &FreeStBlockList[maxstblocks-1];
STRATEGYBLOCK FreeStBlockData[maxstblocks];
static STRATEGYBLOCK  **ActiveStBlockListPtr = &ActiveStBlockList[0];

unsigned int IncrementalSBname;

/*

 Support functions for Strategy Blocks

*/

void InitialiseStrategyBlocks(void)
{
	STRATEGYBLOCK *FreeBlkPtr = &FreeStBlockData[0];

	FreeStBlockListPtr = &FreeStBlockList[maxstblocks-1];

	for(NumFreeStBlocks=0; NumFreeStBlocks < maxstblocks; NumFreeStBlocks++) 
		{
			FreeStBlockList[NumFreeStBlocks] = FreeBlkPtr;
			FreeBlkPtr->SBflags.destroyed_but_preserved=0;
			#if debug
			FreeBlkPtr->SBIsValid = 0;
			#endif
			FreeBlkPtr++;
		}
	
    /* KJL 17:31:18 11/13/96 - seems like a logical place to initialise dynamics blocks */
    InitialiseDynamicsBlocks();
    
    NumActiveStBlocks = 0;
    ActiveStBlockListPtr = &ActiveStBlockList[0];

	IncrementalSBname=0;
}


STRATEGYBLOCK* AllocateStrategyBlock(void)
{
	STRATEGYBLOCK *FreeBlkPtr = 0;		/* Default to null ptr */
	int *sptr;
	int i;

	if(NumFreeStBlocks) 
		{
			FreeBlkPtr = *FreeStBlockListPtr--;

			NumFreeStBlocks--;					/* One less free block */

			/* Clear the block */

			sptr = (int *)FreeBlkPtr;
			for(i = sizeof(STRATEGYBLOCK)/4; i!=0; i--)
				{
					*sptr++ = 0;
				}
		}

	return(FreeBlkPtr);
}


void DeallocateStrategyBlock(STRATEGYBLOCK *sptr)
{
	int a;
	/* Reset name */

	for(a = 0; a < SB_NAME_LENGTH; a++) {	
		sptr->SBname[a] = '\0';
	}													

	FreeStBlockListPtr++;

	*FreeStBlockListPtr = sptr;

	NumFreeStBlocks++;						/* One more free block */
}


STRATEGYBLOCK* CreateActiveStrategyBlock(void)
{
	STRATEGYBLOCK *sb;
	sb = AllocateStrategyBlock();

	if(sb) 
  	{
  		#if debug
  		GLOBALASSERT(sb->SBIsValid == 0);
  		sb->SBIsValid = 1;
  		#endif

  		*ActiveStBlockListPtr++ = sb;
  		NumActiveStBlocks++;
  	}

	return sb;
}


int DestroyActiveStrategyBlock(STRATEGYBLOCK* sb)
{
	int j = -1;
	int i;

	/* If the block ptr is OK, search the Active Blocks List */

	if(sb) 
	{
		for(i = 0; i < NumActiveStBlocks && j!=0; i++) 
		{

			if(ActiveStBlockList[i] == sb) 
			{
				ActiveStBlockList[i] = ActiveStBlockList[NumActiveStBlocks-1];
				NumActiveStBlocks--;
				ActiveStBlockListPtr--;

				if(!sb->SBflags.preserve_until_end_of_level)
				{
					DeallocateStrategyBlock(sb);		/* Back to Free List */
				}
				else
				{
					sb->SBflags.destroyed_but_preserved=1;
				}	
					

				j = 0;												/* Flag OK */
			}
		}
	}

	return(j);
}



STRATEGYBLOCK * AttachNewStratBlock
(
	MODULE* moptr,
	MODULEMAPBLOCK* momptr,
	DISPLAYBLOCK* dptr
)
{
	/*oh for a constructor*/
	/* fails if any of the above has a 
		 stratgey block attached*/

	STRATEGYBLOCK* sptr;
	int i;

	GLOBALASSERT(momptr || dptr);

	sptr = CreateActiveStrategyBlock();
	if (sptr == 0) return 0;

	InitialiseSBValues(sptr);

	for(i = 0; i < SB_NAME_LENGTH; i++);
		{	
			sptr->SBname[i] = '\0';
		}													

	sptr->SBmomptr = momptr;

	if(moptr)
		{
/*			GLOBALASSERT(!moptr->m_sbptr); HACK*/
			moptr->m_sbptr = sptr;
			sptr->SBmoptr = moptr;
		}

	if(dptr)
		{
			GLOBALASSERT(!dptr->ObStrategyBlock);
			dptr->ObStrategyBlock = sptr;
			sptr->SBdptr = dptr;
		}
	else
		{
			sptr->SBflags.no_displayblock = 1;
		}


	
	return(sptr);
	
}	


void InitialiseSBValues(STRATEGYBLOCK* sptr)
{
	sptr->I_SBtype = I_BehaviourNull;
	sptr->SBdataptr = (void *)0x0;

	sptr->SBDamageBlock.Health=0;
	sptr->SBDamageBlock.Armour=0;
	sptr->SBDamageBlock.SB_H_flags.AcidResistant=0;
	sptr->SBDamageBlock.SB_H_flags.FireResistant=0;
	sptr->SBDamageBlock.SB_H_flags.ElectricResistant=0;
	sptr->SBDamageBlock.SB_H_flags.PerfectArmour=0;
	sptr->SBDamageBlock.SB_H_flags.ElectricSensitive=0;
	sptr->SBDamageBlock.SB_H_flags.Indestructable=0;
	
	sptr->SBflags.please_destroy_me = 0;
	sptr->SBflags.no_displayblock = 0;
	sptr->SBflags.request_operate = 0;
	sptr->SBflags.preserve_until_end_of_level = 0;
	sptr->SBflags.destroyed_but_preserved = 0;
	sptr->SBflags.not_on_motiontracker = 0;
	
	sptr->integrity = 0;
 
	sptr->maintainVisibility = 0;		  /* patrRWH - function to search thgough the list of active*/
	sptr->containingModule = (MODULE *)0; /* patrstrat blocks and return the pointer*/
	sptr->shapeIndex = 0;				  /* patr*/

	sptr->SBmoptr = (MODULE*)0x0;
	sptr->SBmomptr = (MODULEMAPBLOCK*)0x0;
	sptr->SBmorphctrl = (MORPHCTRL*)0x0;

	sptr->SBdptr=NULL;

	sptr->name=0;
	
}


/* 
RWH - function to search thgough the list of active
strat blocks and return the pointer
*/


STRATEGYBLOCK* FindSBWithName(char* id_name)
{	
	int stratblock = NumActiveStBlocks;
	int i;
	GLOBALASSERT(stratblock);

	if(!id_name)
		return NULL;
	
	//If the name is all 0`s I want to return a null pointer - Richard.
	for(i=0;i<SB_NAME_LENGTH;i++)
	{
		if(id_name[i]) break;
	}
	if(i==SB_NAME_LENGTH)
	{
		return NULL;
	}

	while(--stratblock >= 0)
		{
			STRATEGYBLOCK* sbptr = ActiveStBlockList[stratblock];
			GLOBALASSERT(sbptr);

			if(sbptr->SBname)
				{
					if(NAME_ISEQUAL(sbptr->SBname, id_name))
					{
						return((sbptr));
					}
				}
		}
	// we have to return null for lifts - so that 
	// we know that the lift is outside the env
	return(NULL);
}				
			 

static STRATEGYBLOCK SB_Preserved[MAX_PRESERVED_SB];
static int Num_SB_Preserved;


void InitPreservedSBs()
{
	Num_SB_Preserved = 0;
}


void PreserveStBlocksInModule(MODULE* containing_mod)
{
	//	this used within level - find objects in module
	// all will have sbs

	int i;
	int max_x, min_x, max_y, min_y, max_z, min_z;	

	max_x = containing_mod->m_maxx + containing_mod->m_world.vx;
	min_x = containing_mod->m_minx + containing_mod->m_world.vx;
	max_y = containing_mod->m_maxy + containing_mod->m_world.vy;
	min_y = containing_mod->m_miny + containing_mod->m_world.vy;
	max_z = containing_mod->m_maxz + containing_mod->m_world.vz;
	min_z = containing_mod->m_minz + containing_mod->m_world.vz;

	GLOBALASSERT(Num_SB_Preserved == 0);

	for(i = 0; i < NumActiveStBlocks && Num_SB_Preserved < MAX_PRESERVED_SB; i++)
		{
			VECTORCH obj_world;
			STRATEGYBLOCK	*sbptr;
			DYNAMICSBLOCK	*dynptr;			

			sbptr = ActiveStBlockList[i];

			if(!(dynptr = sbptr->DynPtr))
				continue;
			
			obj_world = dynptr->Position;

			if(obj_world.vx < max_x)
				if(obj_world.vx > min_x)
					if(obj_world.vz < max_z)
						if(obj_world.vz > min_z)
							if(obj_world.vy < max_y)
								if(obj_world.vy > min_y)
									{
										// copy name into somthing
										if(sbptr->I_SBtype == I_BehaviourMarinePlayer ||
												sbptr->I_SBtype == I_BehaviourMarinePlayer ||
												sbptr->I_SBtype == I_BehaviourMarinePlayer)
											{
												SB_Preserved[Num_SB_Preserved] = *sbptr;
	
												Num_SB_Preserved++;
											}
									}
		}
}


BOOL SBNeededForNextEnv(STRATEGYBLOCK* sbptr)
{
	int i = 0;	
	
	if(Num_SB_Preserved == 0)
		return(0);

	if (sbptr->I_SBtype!=I_BehaviourMarinePlayer) 
		if (sbptr->I_SBtype!=I_BehaviourAlienPlayer) 
			if (sbptr->I_SBtype!=I_BehaviourPredatorPlayer) 
	 			if (NAME_ISNULL(sbptr->SBname)) 
					return(0);

	for(i = 0; i < Num_SB_Preserved; i++)
		{
			STRATEGYBLOCK	*pres_sbptr;

			pres_sbptr = &SB_Preserved[i];
			
			if(NAME_ISEQUAL(pres_sbptr->SBname, sbptr->SBname))
				return(1);
  		}

	return(0);
}


void AddPreservedSBsToActiveList()
{
	int i;
	STRATEGYBLOCK *new_sbptr;

	for(i = 0; i < Num_SB_Preserved; i++)
		{
			new_sbptr =	CreateActiveStrategyBlock();

			*new_sbptr = SB_Preserved[i];

			
			if(new_sbptr->I_SBtype == I_BehaviourMarinePlayer ||
					new_sbptr->I_SBtype == I_BehaviourMarinePlayer ||
					new_sbptr->I_SBtype == I_BehaviourMarinePlayer)

				{
					DYNAMICSBLOCK *playerDynPtr;

					Player->ObStrategyBlock = new_sbptr;

				 	playerDynPtr = Player->ObStrategyBlock->DynPtr;
					// Need to copy some of the preserved SB info into the appropriate places

					Player->ObWorld = playerDynPtr->Position;
					Player->ObMat 	= playerDynPtr->OrientMat;
					Player->ObEuler = playerDynPtr->OrientEuler;

					playerDynPtr->PrevPosition    = playerDynPtr->Position;
					playerDynPtr->PrevOrientMat 	= playerDynPtr->OrientMat;
					playerDynPtr->PrevOrientEuler = playerDynPtr->OrientEuler;
			 	
					playerDynPtr->LinVelocity.vx = 0;
					playerDynPtr->LinVelocity.vy = 0;
					playerDynPtr->LinVelocity.vz = 0;

					playerDynPtr->LinImpulse.vx = 0;
					playerDynPtr->LinImpulse.vy = 0;
					playerDynPtr->LinImpulse.vz = 0;

					playerDynPtr->AngVelocity.EulerX = 0;
					playerDynPtr->AngVelocity.EulerY = 0;
					playerDynPtr->AngVelocity.EulerZ = 0;

					playerDynPtr->AngImpulse.EulerX = 0;
					playerDynPtr->AngImpulse.EulerY = 0;
					playerDynPtr->AngImpulse.EulerZ = 0;
					
				}

			// we will almost certainly need
			// some clean up here. esp for the
			// LIFT_FLOOR_SWITCHES and for objects
			// whose shape reference has changed
		}
}						


void TeleportPreservedSBsToNewEnvModule(MODULE *new_pos, MODULE* old_pos, int orient_change)
{
	int i;
	VECTORCH mod_offset;

	mod_offset.vx = new_pos->m_world.vx - old_pos->m_world.vx;
	mod_offset.vy = new_pos->m_world.vy - old_pos->m_world.vy;
	mod_offset.vz = new_pos->m_world.vz - old_pos->m_world.vz;
 
	for(i = 0; i < Num_SB_Preserved; i++)
		{
			VECTORCH obj_world;
			VECTORCH pos_rel; 
			STRATEGYBLOCK	*sbptr;
			DYNAMICSBLOCK	*dynptr;			

			sbptr = &SB_Preserved[i];
			
			dynptr = sbptr->DynPtr;
			GLOBALASSERT(dynptr);

			obj_world = sbptr->DynPtr->Position;
			
			
			{
			  // okay we need to find our relative position to the moduke
		  	int cos;
 	  		int sin;
				int angle;
				MATRIXCH mat;


				pos_rel.vx = dynptr->Position.vx - old_pos->m_world.vx; 
				pos_rel.vy = dynptr->Position.vy - old_pos->m_world.vy; 
				pos_rel.vz = dynptr->Position.vz - old_pos->m_world.vz; 

				if(orient_change == 1 || orient_change == -3)
					angle = 1024;
				else if(orient_change == 2 || orient_change == -2)
					angle = 2048;
				else if(orient_change == 3 || orient_change == -1)
					angle = 3072;
				else
					angle = 0;

		  	cos = GetCos(angle);
 	  		sin = GetSin(angle);

				mat.mat11 = cos;		 
	 	  	mat.mat12 = 0;
		  	mat.mat13 = -sin;
 		  	mat.mat21 = 0;	  	
 	  		mat.mat22 = 65536;	  	
 	  		mat.mat23 = 0;	  	
 	  		mat.mat31 = sin;	  	
 	  		mat.mat32 = 0;	  	
 	  		mat.mat33 = cos;	
			
				// rotate the relative object about the center of the
				// module and rotate the abject about its own y-axis
				
				RotateVector(&pos_rel, &mat);
		  	MatrixMultiply(&dynptr->OrientMat,&mat,&dynptr->OrientMat);
			 	MatrixToEuler(&dynptr->OrientMat, &dynptr->OrientEuler);
			}

#if 0 
			dynptr->Position.vx = mod_offset.vx; 
			dynptr->Position.vy = mod_offset.vy; 
			dynptr->Position.vz = mod_offset.vz; 

			dynptr->PrevPosition.vx = mod_offset.vx; 
			dynptr->PrevPosition.vy = mod_offset.vy; 
			dynptr->PrevPosition.vz = mod_offset.vz; 
#endif

			dynptr->Position.vx = pos_rel.vx + new_pos->m_world.vx; 
			dynptr->Position.vy = pos_rel.vy + new_pos->m_world.vy; 
			dynptr->Position.vz = pos_rel.vz + new_pos->m_world.vz; 

			dynptr->PrevPosition.vx = -pos_rel.vx + old_pos->m_world.vx; 
			dynptr->PrevPosition.vy = -pos_rel.vy + old_pos->m_world.vy; 
			dynptr->PrevPosition.vz = -pos_rel.vz + old_pos->m_world.vz; 
	
		}
 }




// RWH 5/6/97 these next two functions were 1, changed and 2, added
// to deal with deallocating stblocks at the end of behaviours
// this stops problems with accessing defunct strategy blocks
// via collision reports


void DestroyAnyStrategyBlock(STRATEGYBLOCK *sbptr) 
{
	GLOBALASSERT(sbptr);

	sbptr->SBflags.please_destroy_me = 1;
}		


void RemoveDestroyedStrategyBlocks(void)
{
	/*
	Go backwards through the strategy block.
	This should prevent any strategy blocks from being skipped when they
	get shuffled down from the end of the array.
	*/
	int i = NumActiveStBlocks;

	while(i)
	{
		STRATEGYBLOCK* sbptr = ActiveStBlockList[--i];
	
		if(sbptr->SBflags.please_destroy_me)
		{
			RemoveBehaviourStrategy(sbptr);
		} else {
		
			/* Also... gibb aliens? */
			if (sbptr->I_SBtype==I_BehaviourAlien) {

				ALIEN_STATUS_BLOCK *alienStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbptr->SBdataptr);    

				if (alienStatusPointer->GibbFactor>0) {
					Extreme_Gibbing(sbptr,alienStatusPointer->HModelController.section_data,alienStatusPointer->GibbFactor);
					alienStatusPointer->GibbFactor=0;
				} else if (alienStatusPointer->GibbFactor<0) {
					KillRandomSections(alienStatusPointer->HModelController.section_data,alienStatusPointer->GibbFactor);
					alienStatusPointer->GibbFactor=0;
				}

			} else if (
				(sbptr->I_SBtype==I_BehaviourMarine)
				|| (sbptr->I_SBtype==I_BehaviourSeal)
				) {

				MARINE_STATUS_BLOCK *marineStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbptr->SBdataptr);    

				if (marineStatusPointer->GibbFactor>0) {
					Extreme_Gibbing(sbptr,marineStatusPointer->HModelController.section_data,marineStatusPointer->GibbFactor);
					marineStatusPointer->GibbFactor=0;
				} else if (marineStatusPointer->GibbFactor<0) {
					KillRandomSections(marineStatusPointer->HModelController.section_data,-(marineStatusPointer->GibbFactor));
					marineStatusPointer->GibbFactor=0;
				}
			} else if (sbptr->I_SBtype==I_BehaviourPredator) {
				PREDATOR_STATUS_BLOCK *predatorStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbptr->SBdataptr);    

				if (predatorStatusPointer->GibbFactor>0) {
					Extreme_Gibbing(sbptr,predatorStatusPointer->HModelController.section_data,predatorStatusPointer->GibbFactor);
					predatorStatusPointer->GibbFactor=0;
				} else if (predatorStatusPointer->GibbFactor<0) {
					KillRandomSections(predatorStatusPointer->HModelController.section_data,-(predatorStatusPointer->GibbFactor));
					predatorStatusPointer->GibbFactor=0;
				}
			} else if (sbptr->I_SBtype==I_BehaviourXenoborg) {
				XENO_STATUS_BLOCK *xenoStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				xenoStatusPointer=(XENO_STATUS_BLOCK *)(sbptr->SBdataptr);    

				if (xenoStatusPointer->GibbFactor>0) {
					Extreme_Gibbing(sbptr,xenoStatusPointer->HModelController.section_data,xenoStatusPointer->GibbFactor);
					xenoStatusPointer->GibbFactor=0;
				} else if (xenoStatusPointer->GibbFactor<0) {
					KillRandomSections(xenoStatusPointer->HModelController.section_data,-(xenoStatusPointer->GibbFactor));
					xenoStatusPointer->GibbFactor=0;
				}
			} else if (sbptr->I_SBtype==I_BehaviourNetCorpse) {

				NETCORPSEDATABLOCK *corpseStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				corpseStatusPointer=(NETCORPSEDATABLOCK *)(sbptr->SBdataptr);    
				if(corpseStatusPointer->GibbFactor)
				{
					/*get a seed for the RNG so that we can send gibbing to other
					network players*/
					int seed=FastRandom();
					SetSeededFastRandom(seed);

					if(AvP.Network != I_No_Network)
					{
						AddNetMsg_Gibbing(sbptr,corpseStatusPointer->GibbFactor,seed);
					}

					if (corpseStatusPointer->GibbFactor>0) {
						Extreme_Gibbing(sbptr,corpseStatusPointer->HModelController.section_data,corpseStatusPointer->GibbFactor);
						corpseStatusPointer->GibbFactor=0;
					} else if (corpseStatusPointer->GibbFactor<0) {
						KillRandomSections(corpseStatusPointer->HModelController.section_data,-(corpseStatusPointer->GibbFactor));
						corpseStatusPointer->GibbFactor=0;
					}
				}
			} else if (sbptr->I_SBtype==I_BehaviourHierarchicalFragment) {

				HDEBRIS_BEHAV_BLOCK *debrisStatusPointer;
				LOCALASSERT(sbptr);	
				LOCALASSERT(sbptr->DynPtr);	
		
				debrisStatusPointer=(HDEBRIS_BEHAV_BLOCK *)(sbptr->SBdataptr);    

				if (debrisStatusPointer->GibbFactor>0) {
					Extreme_Gibbing(sbptr,debrisStatusPointer->HModelController.section_data,debrisStatusPointer->GibbFactor);
					debrisStatusPointer->GibbFactor=0;
				} else if (debrisStatusPointer->GibbFactor<0) {
					KillRandomSections(debrisStatusPointer->HModelController.section_data,-(debrisStatusPointer->GibbFactor));
					debrisStatusPointer->GibbFactor=0;
				}
			}
		}
	}
}



void DestroyAllStrategyBlocks(void)
{
	int i = 0;	
	int a = NumActiveStBlocks;
	
	
	while(i < a)
		{
			RemoveBehaviourStrategy(ActiveStBlockList[i++]);
		}

	//get rid of all the strategyblocks that have been preserved until the end of the level

	for(i=0;i<maxstblocks;i++)
	{
		if(FreeStBlockData[i].SBflags.destroyed_but_preserved)
		{
	   		FreeStBlockData[i].SBflags.destroyed_but_preserved=0;
	   		FreeStBlockData[i].SBflags.preserve_until_end_of_level=0;
			DeallocateStrategyBlock(&FreeStBlockData[i]);
	   	}
	}

}			

void AssignNewSBName(STRATEGYBLOCK *sbPtr) {

	IncrementalSBname++;

	if(IncrementalSBname>0xffffff)
	{
		IncrementalSBname=1;
	}

	*((int *)&sbPtr->SBname[4])=IncrementalSBname;
	
	if(AvP.Network != I_No_Network)
	{
		//modify name to ensure uniqueness between players
		extern DPID AVPDPNetID;
		sbPtr->SBname[SB_NAME_LENGTH-1]=+10+PlayerIdInPlayerList(AVPDPNetID); /* Just to make sure... */

	}
	else
	{
		sbPtr->SBname[SB_NAME_LENGTH-1]=1; /* Just to make sure... */
	}


}

void GivePlayerCloakAway(void) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

	playerStatusPtr->cloakPositionGivenAway = 1;
	playerStatusPtr->cloakPositionGivenAwayTimer = PLAYERCLOAK_POSTIONGIVENAWAYTIME;
}
