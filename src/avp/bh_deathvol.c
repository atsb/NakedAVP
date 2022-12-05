
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_deathvol.h"
#include "dynamics.h"
#include "weapons.h"

#define UseLocalAssert Yes
#include "ourasert.h"

extern int NumActiveBlocks;
extern DISPLAYBLOCK* ActiveBlockList[];
extern DAMAGE_PROFILE DeathVolumeDamage;
extern int NormalFrameTime;

void* DeathVolumeBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	DEATH_VOLUME_BEHAV_BLOCK* dv_bhv;
	DEATH_VOLUME_TOOLS_TEMPLATE* dv_tt;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	dv_bhv=(DEATH_VOLUME_BEHAV_BLOCK*)AllocateMem(sizeof(DEATH_VOLUME_BEHAV_BLOCK));
	if(!dv_bhv)
	{
		memoryInitialisationFailure = 1;
		return 0;
	}
	dv_bhv->bhvr_type=I_BehaviourDeathVolume;

	dv_tt=(DEATH_VOLUME_TOOLS_TEMPLATE*)bhdata;

	//copy stuff from tools template
	COPY_NAME(sbptr->SBname, dv_tt->nameID);
	dv_bhv->volume_min=dv_tt->volume_min;
	dv_bhv->volume_max=dv_tt->volume_max;
	dv_bhv->damage_per_second=dv_tt->damage_per_second;
	dv_bhv->active=dv_tt->active;
	dv_bhv->collision_required=dv_tt->collision_required;

	
	return (void*)dv_bhv;

}



void DeathVolumeBehaveFun(STRATEGYBLOCK* vol_sbptr)
{
	DEATH_VOLUME_BEHAV_BLOCK* dv_bhv;	
 	GLOBALASSERT(vol_sbptr);
	dv_bhv = (DEATH_VOLUME_BEHAV_BLOCK*)vol_sbptr->SBdataptr;
	GLOBALASSERT((dv_bhv->bhvr_type == I_BehaviourDeathVolume));
	
	if(dv_bhv->active)
	{	
		int i;
		STRATEGYBLOCK* sbPtr;
		DYNAMICSBLOCK* dynPtr;
		int miny,maxy;

		for(i=0;i<NumActiveBlocks;i++)
		{
			//search for objects that have has a collision this frame
			//(or all objects if collisions aren't required)
			DISPLAYBLOCK* dptr = ActiveBlockList[i];

			sbPtr=ActiveBlockList[i]->ObStrategyBlock;
			if(!sbPtr) continue;
			if(!sbPtr->DynPtr) continue;
			dynPtr=sbPtr->DynPtr;

			if(dv_bhv->collision_required)
			{
				if(!dynPtr->CollisionReportPtr) continue;
			}

			//is the object within the death volume?
			//check a vertical line against the death volume's bounding box
			
			//first check the object's centre x and centre z values against the volume
			if(dptr->ObWorld.vx<dv_bhv->volume_min.vx) continue;
			if(dptr->ObWorld.vx>dv_bhv->volume_max.vx) continue;
			if(dptr->ObWorld.vz<dv_bhv->volume_min.vz) continue;
			if(dptr->ObWorld.vz>dv_bhv->volume_max.vz) continue;

			//now check  the object's vertical extents for overlap with the death volume bounding box
			miny=dptr->ObWorld.vy+dptr->ObMinY;
			maxy=dptr->ObWorld.vy+dptr->ObMaxY;
			if(max(miny,dv_bhv->volume_min.vy) > min(maxy,dv_bhv->volume_max.vy)) continue;

			/*
			if(dynPtr->Position.vx > dv_bhv->volume_min.vx &&
			   dynPtr->Position.vx < dv_bhv->volume_max.vx &&
			   dynPtr->Position.vz > dv_bhv->volume_min.vz &&
			   dynPtr->Position.vz < dv_bhv->volume_max.vz &&
			   dynPtr->Position.vy > dv_bhv->volume_min.vy &&
			   dynPtr->Position.vy < dv_bhv->volume_max.vy)
			*/
			{
				//finally see if the object is one of the types that can be harmed by the death volume
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
					extern DPID myNetworkKillerId;
					extern DPID AVPDPNetID;

					//this is a neutral source of damage (for cooperative multiplayer games)
					myNetworkKillerId = 0;
					
					if(dv_bhv->damage_per_second)
					{
						//all new damage volumes.
						VECTORCH direction={0,-ONE_FIXED,0};
						DAMAGE_PROFILE damage = DeathVolumeDamage;
						damage.Penetrative = dv_bhv->damage_per_second;
						CauseDamageToObject(sbPtr,&damage,NormalFrameTime,&direction);
					}
					else
					{
						//kill the creature/player
						VECTORCH direction={0,-ONE_FIXED,0};
						CauseDamageToObject(sbPtr,&certainDeath,ONE_FIXED,&direction);
					}

					//reset network killer id
					myNetworkKillerId = AVPDPNetID;
		
				}
			
			}
		}
		
		
	}

}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct death_volume_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL active;

}DEATH_VOLUME_SAVE_BLOCK;


void LoadStrategy_DeathVolume(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	DEATH_VOLUME_BEHAV_BLOCK* dv_bhv;	
	DEATH_VOLUME_SAVE_BLOCK* block = (DEATH_VOLUME_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourDeathVolume) return;

	dv_bhv = (DEATH_VOLUME_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
 	dv_bhv->active = block->active;
}

void SaveStrategy_DeathVolume(STRATEGYBLOCK* sbPtr)
{
	DEATH_VOLUME_SAVE_BLOCK *block;
	DEATH_VOLUME_BEHAV_BLOCK* dv_bhv;
	
	dv_bhv = (DEATH_VOLUME_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
 	block->active = dv_bhv->active;

}

