#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_selfdest.h"
#include "dynamics.h"
#include "weapons.h"
#include "particle.h"
#include "psnd.h"

#define UseLocalAssert Yes
#include "ourasert.h"


extern int NormalFrameTime;

void* SelfDestructBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	SELF_DESTRUCT_BEHAV_BLOCK* sd_bhv;
	SELF_DESTRUCT_TOOLS_TEMPLATE* sd_tt;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	sd_bhv=(SELF_DESTRUCT_BEHAV_BLOCK*)AllocateMem(sizeof(SELF_DESTRUCT_BEHAV_BLOCK));
	if(!sd_bhv)
	{
		memoryInitialisationFailure = 1;
		return 0;
	}
	sd_bhv->bhvr_type=I_BehaviourSelfDestruct;

	sd_tt=(SELF_DESTRUCT_TOOLS_TEMPLATE*)bhdata;

	//copy stuff from tools template
	COPY_NAME(sbptr->SBname, sd_tt->nameID);
	sd_bhv->timer=sd_tt->timer;
	sd_bhv->active=FALSE;
	
	return (void*)sd_bhv;

}



void SelfDestructBehaveFun(STRATEGYBLOCK* sbptr)
{
	SELF_DESTRUCT_BEHAV_BLOCK* sd_bhv;	
 	GLOBALASSERT(sbptr);
	sd_bhv = (SELF_DESTRUCT_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((sd_bhv->bhvr_type == I_BehaviourSelfDestruct));
	
	if(sd_bhv->active)
	{	
		sd_bhv->timer-=NormalFrameTime;
		if(sd_bhv->timer<=0)
		{
			int i;

			/* KJL 16:20:57 27/08/98 - let's do some pyrotechnics */
			Sound_Play(SID_NICE_EXPLOSION,"d",&(Player->ObWorld));
			MakeVolumetricExplosionAt(&Player->ObWorld,EXPLOSION_HUGE);
			Player->ObStrategyBlock->SBDamageBlock.IsOnFire=1;
			
			//blow up everone
			for (i=0; i<NumActiveStBlocks; i++)
			{
				STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];
				if(sbPtr->I_SBtype==I_BehaviourAlien ||
				   sbPtr->I_SBtype==I_BehaviourQueenAlien ||
				   sbPtr->I_SBtype==I_BehaviourFaceHugger ||
				   sbPtr->I_SBtype==I_BehaviourPredator ||
				   sbPtr->I_SBtype==I_BehaviourXenoborg ||
				   sbPtr->I_SBtype==I_BehaviourMarine ||
				   sbPtr->I_SBtype==I_BehaviourSeal ||
				   sbPtr->I_SBtype==I_BehaviourPredatorAlien ||
				   sbPtr->I_SBtype==I_BehaviourAlien ||
				   sbPtr->I_SBtype==I_BehaviourMarinePlayer ||
				   sbPtr->I_SBtype==I_BehaviourPredatorPlayer || 
				   sbPtr->I_SBtype==I_BehaviourAlienPlayer) 
				{
			 		{
			 			//kill the creature/player
			 			VECTORCH direction={0,-ONE_FIXED,0};
			 			CauseDamageToObject(sbPtr,&certainDeath,ONE_FIXED,&direction);
			 		
					}
				}
			}
			sd_bhv->active = FALSE;
		}
	}
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct self_destruct_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int timer; //in fixed point seconds
	BOOL active;
}SELF_DESTRUCT_SAVE_BLOCK;

#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV sd_bhv

void LoadStrategy_SelfDestruct(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	SELF_DESTRUCT_BEHAV_BLOCK* sd_bhv;	
	SELF_DESTRUCT_SAVE_BLOCK* block = (SELF_DESTRUCT_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourSelfDestruct) return;

	sd_bhv = (SELF_DESTRUCT_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_LOAD(timer);
	COPYELEMENT_LOAD(active);
}

void SaveStrategy_SelfDestruct(STRATEGYBLOCK* sbPtr)
{
	SELF_DESTRUCT_SAVE_BLOCK *block;
	SELF_DESTRUCT_BEHAV_BLOCK* sd_bhv;
	
	sd_bhv = (SELF_DESTRUCT_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	COPYELEMENT_SAVE(timer);
	COPYELEMENT_SAVE(active);

}
