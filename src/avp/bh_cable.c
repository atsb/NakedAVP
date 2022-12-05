
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_cable.h"
#include "dynamics.h"
#include "pvisible.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "plat_shp.h"

extern int NormalFrameTime;
extern char *ModuleCurrVisArray;

void* PowerCableBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	POWER_CABLE_BEHAV_BLOCK* pc_bhv;
	POWER_CABLE_TOOLS_TEMPLATE* pc_tt;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	pc_bhv=(POWER_CABLE_BEHAV_BLOCK*)AllocateMem(sizeof(POWER_CABLE_BEHAV_BLOCK));
	if(!pc_bhv)
	{
		memoryInitialisationFailure = 1;
		return 0;
	}
	pc_bhv->bhvr_type=I_BehaviourPowerCable;

	pc_tt=(POWER_CABLE_TOOLS_TEMPLATE*)bhdata;

	//copy stuff from tools template
	COPY_NAME(sbptr->SBname, pc_tt->nameID);
	pc_bhv->position = pc_tt->position;
	pc_bhv->max_charge = pc_tt->max_charge;
	pc_bhv->current_charge = pc_tt->current_charge;
	pc_bhv->recharge_rate = pc_tt->recharge_rate;

	pc_bhv->position.vy+=10; //temporarily move cable down in case rounding errors have put cable just outside of module
	sbptr->containingModule=ModuleFromPosition(&(pc_bhv->position),0);
	pc_bhv->position.vy-=10;
	
	GLOBALASSERT(sbptr->containingModule);
	
	
	return (void*)pc_bhv;
}

void PowerCableBehaveFun(STRATEGYBLOCK* sbptr)
{
	POWER_CABLE_BEHAV_BLOCK* pc_bhv;	
 	GLOBALASSERT(sbptr);
	pc_bhv = (POWER_CABLE_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((pc_bhv->bhvr_type == I_BehaviourPowerCable));

	//see if player can get health from cable
	//player must be an alien
	if(AvP.PlayerType==I_Alien)
	{
		//the cable needs to be in a near module
		GLOBALASSERT(sbptr->containingModule);
		if(ModuleCurrVisArray[(sbptr->containingModule->m_index)])
		{ 
			//is player close enough
			int distance=VectorDistance(&Player->ObWorld,&pc_bhv->position);
			if(distance<CABLE_HEALTH_DISTANCE)
			{
				//give the player some health
				int health_gained;
				int current_health;
				int max_health;
				NPC_DATA *NpcData;
				switch (AvP.Difficulty) {
					case I_Easy:
						NpcData=GetThisNpcData(I_PC_Alien_Easy);
						break;
					default:
					case I_Medium:
						NpcData=GetThisNpcData(I_PC_Alien_Medium);
						break;
					case I_Hard:
						NpcData=GetThisNpcData(I_PC_Alien_Hard);
						break;
					case I_Impossible:
						NpcData=GetThisNpcData(I_PC_Alien_Impossible);
						break;
				}
				
				LOCALASSERT(NpcData);
				
				max_health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
				current_health=Player->ObStrategyBlock->SBDamageBlock.Health;
				
				health_gained=min(pc_bhv->current_charge,max_health-current_health);

				pc_bhv->current_charge-=health_gained;
				Player->ObStrategyBlock->SBDamageBlock.Health+=health_gained;
				{
					/* access the extra data hanging off the strategy block */
					PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
					GLOBALASSERT(playerStatusPtr);
					playerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
				}
			}
		}
	}

	//increase charge if currently below maximum
	if(pc_bhv->current_charge<pc_bhv->max_charge)
	{
		if(pc_bhv->recharge_rate)
		{
			pc_bhv->current_charge+=MUL_FIXED(pc_bhv->recharge_rate,NormalFrameTime);
			if(pc_bhv->current_charge>pc_bhv->max_charge)
			{
				pc_bhv->current_charge=pc_bhv->max_charge;
			}
		}
	}
}


