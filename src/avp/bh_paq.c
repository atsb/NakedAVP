/*------------------------Patrick 18/2/97-----------------------------
  Source file for Predator-Alien and Queen AI behaviour functions....
  --------------------------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pfarlocs.h"
#include "pvisible.h"
#include "pheromon.h"
#include "bh_far.h"
#include "bh_pred.h"
#include "bh_paq.h"
#include "bh_debri.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "psnd.h"
#include "weapons.h"

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;

/* prototypes for this file */
static void Execute_PAQNS_Wait(STRATEGYBLOCK *sbPtr);
static void Execute_PAQNS_Approach(STRATEGYBLOCK *sbPtr);
static void Execute_PAQNS_Attack(STRATEGYBLOCK *sbPtr);
static void Execute_PAQNS_Avoidance(STRATEGYBLOCK *sbPtr);
static void Execute_PAQNS_Wander(STRATEGYBLOCK *sbPtr);
static void Execute_Dying(STRATEGYBLOCK *sbPtr);

static void Execute_PAQFS_Wait(STRATEGYBLOCK *sbPtr);
static void Execute_PAQFS_Hunt(STRATEGYBLOCK *sbPtr);
static void Execute_PAQFS_Wander(STRATEGYBLOCK *sbPtr);
static void ProcessFarPAQTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule);

static void SetPAQAnimationSequence(STRATEGYBLOCK *sbPtr, PAQ_SEQUENCE seq, int rate);
static int PAQShouldAttackPlayer(void);

/*------------------------Patrick 18/2/97-----------------------------
  Predator-Alien & Queen behaviour shell functions
  --------------------------------------------------------------------*/

void InitPredAlBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_PAQ *toolsData; 
	int i;

	LOCALASSERT(sbPtr);
	LOCALASSERT(bhdata);
	toolsData = (TOOLS_DATA_PAQ *)bhdata; 

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network) 
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* make the assumption that the loader has initialised the strategy 
	block sensibly... 
	so just set the shapeIndex from the tools data & copy the name id*/
	sbPtr->shapeIndex = toolsData->shapeIndex;
	for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];

	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      		
		/* zero linear velocity in dynamics block */
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
	}
	else
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* Initialise alien's stats */
	{
		NPC_DATA *NpcData;

		NpcData=GetThisNpcData(I_NPC_PredatorAlien);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
	}
	/* create, initialise and attach a predator-alien/queen data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(PAQ_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		PAQ_STATUS_BLOCK *paqStatus = (PAQ_STATUS_BLOCK *)sbPtr->SBdataptr;
		NPC_InitMovementData(&(paqStatus->moveData));
		NPC_InitWanderData(&(paqStatus->wanderData));
     	paqStatus->health = PRAL_STARTING_HEALTH;
     	paqStatus->nearSpeed = PRAL_NEAR_SPEED;
     	paqStatus->damageInflicted = PRAL_NEAR_DAMAGE;   		
   		sbPtr->integrity = paqStatus->health;
   		paqStatus->FarBehaviourState = PAQFS_Wait;  		
   		paqStatus->NearBehaviourState = PAQNS_Wait;
		paqStatus->stateTimer = 0;
		InitShapeAnimationController(&paqStatus->ShpAnimCtrl, GetShapeData(sbPtr->shapeIndex));	
	}
	else
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}		   	   	   	   
}

void PAQBehaviour(STRATEGYBLOCK *sbPtr)
{
	/* get the paq status block */
	PAQ_STATUS_BLOCK *paqStatusPointer;
	    
	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(paqStatusPointer);	          		

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	} 

	/* zero our velocity */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	
	InitWaypointSystem(0);

	if(sbPtr->SBdptr) 
	{
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
		switch(paqStatusPointer->NearBehaviourState)
		{
			case(PAQNS_Wait):
			{
				Execute_PAQNS_Wait(sbPtr);
				break;
			}
			case(PAQNS_Approach):
			{
				Execute_PAQNS_Approach(sbPtr);
				break;
			}
			case(PAQNS_Attack):
			{
				Execute_PAQNS_Attack(sbPtr);
				break;
			}
			case(PAQNS_Avoidance):
			{
				textprint("paq avoidance \n");
				Execute_PAQNS_Avoidance(sbPtr);
				break;
			}
			case(PAQNS_Wander):
			{
				Execute_PAQNS_Wander(sbPtr);
				break;
			}
			case(PAQNS_Dying):
			{
				Execute_Dying(sbPtr);
				break;
			}
			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}
	}
	else
	{
		/* NB if this assert fires, may have just run out of displayblocks */
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] == 0);		
		switch(paqStatusPointer->FarBehaviourState)
		{
			case(PAQFS_Wait):
			{
				Execute_PAQFS_Wait(sbPtr);
				break;
			}
			case(PAQFS_Hunt):
			{
				Execute_PAQFS_Hunt(sbPtr);
				break;
			}
			case(PAQFS_Wander):
			{
				Execute_PAQFS_Wander(sbPtr);
				break;
			}
			case(PAQFS_Dying):
			{
				Execute_Dying(sbPtr);
				break;
			}
			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}			
		
		/* check here to see if paq is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);

			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->alienTrigger = 1;
		}
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);

			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("PAQ MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

	/* test here to see if the paq is dead: if so, remove it */
	if((paqStatusPointer->NearBehaviourState == PAQNS_Dying)&&(paqStatusPointer->stateTimer <= 0))
		DestroyAnyStrategyBlock(sbPtr);
}

void MakePAQNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	PAQ_STATUS_BLOCK *paqStatusPointer;    

	LOCALASSERT(sbPtr);
	dynPtr = sbPtr->DynPtr;
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(paqStatusPointer);	          		
    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr = NULL;
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;		
	if(dPtr==NULL) return; /* if cannot allocate displayblock, leave far */

	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					

	/* need to initialise positional information in the new display block */ 
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;
	
	/* pa-q data block init */
	paqStatusPointer->stateTimer = 0;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* set up starting state and sequence */
	dPtr->ShapeAnimControlBlock = &paqStatusPointer->ShpAnimCtrl;
	if(PAQShouldAttackPlayer())
	{
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPC_InitWanderData(&(paqStatusPointer->wanderData));
		paqStatusPointer->NearBehaviourState = PAQNS_Approach;
		paqStatusPointer->stateTimer = 0;	
		SetPAQAnimationSequence(sbPtr,PaqSQ_Run,10);
	}
	else
	{
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPC_InitWanderData(&(paqStatusPointer->wanderData));
		paqStatusPointer->NearBehaviourState = PAQNS_Wait;
		paqStatusPointer->stateTimer = PAQ_NEARWAITTIME;	
		SetPAQAnimationSequence(sbPtr,PaqSQ_Stand,10);
	}
}

void MakePAQFar(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	int i;
	
	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);	          		
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;	

	NPC_InitWanderData(&(paqStatusPointer->wanderData));
	
	/* set up starting state and sequence */
	if(paqStatusPointer->NearBehaviourState==PAQNS_Dying)
	{
		DestroyAnyStrategyBlock(sbPtr);
		return;
	}
	
	if(PAQShouldAttackPlayer())
	{
		paqStatusPointer->FarBehaviourState = PAQFS_Hunt;
		paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;	
	}
	else
	{
		paqStatusPointer->FarBehaviourState = PAQFS_Wander;
		paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;	
	}
}

void PAQIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;

	PAQ_STATUS_BLOCK *paqStatusPointer;    

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
  LOCALASSERT(paqStatusPointer);	          		
	LOCALASSERT(sbPtr->containingModule); 
					
	/* if we're dying, do nothing */
	if(paqStatusPointer->NearBehaviourState==PAQNS_Dying)
	{
		/* PAQFS should be dying, too */
		return;
	}				
	
	if(!(sbPtr->SBdptr))
	{
		DestroyAnyStrategyBlock(sbPtr);
		return;
	}

	#define QUEEN_PITCH_CHANGE 150
	#define PREDALIEN_PITCH_CHANGE 300

	if (sbPtr->I_SBtype == I_BehaviourQueenAlien)
	{
		switch (rand % 5)
		{
			case 0:
				Sound_Play(SID_ALIEN_SCREAM,"dp",&(sbPtr->DynPtr->Position),QUEEN_PITCH_CHANGE + pitch);
				break;
			case 1:
			 	Sound_Play(SID_ALIEN_HIT,"dp",&(sbPtr->DynPtr->Position),QUEEN_PITCH_CHANGE + pitch);
				break;
			case 2:
			 	Sound_Play(SID_ALIEN_HIT2,"dp",&(sbPtr->DynPtr->Position),QUEEN_PITCH_CHANGE + pitch);
				break;
			case 3:
			 	Sound_Play(SID_ALIEN_HISS1,"dp",&(sbPtr->DynPtr->Position),QUEEN_PITCH_CHANGE + pitch);
				break;
			default:
				Sound_Play(SID_ALIEN_HISS,"dp",&(sbPtr->DynPtr->Position),QUEEN_PITCH_CHANGE + pitch);
	 			break;
		}
	}
	else
	{
		switch (rand % 7)
		{
			case 0:
			 	Sound_Play(SID_PRED_SNARL,"dp",&(sbPtr->DynPtr->Position),pitch);
				break;
			case 1:
			 	Sound_Play(SID_PRED_HISS,"dp",&(sbPtr->DynPtr->Position),pitch);
				break;
			case 2:
			 	Sound_Play(SID_PRED_SHORTROAR,"dp",&(sbPtr->DynPtr->Position),pitch);
 				break;
			case 3:
			 	Sound_Play(SID_PRED_SCREAM1,"dp",&(sbPtr->DynPtr->Position),pitch);
	 			break;
			case 4:
			 	Sound_Play(SID_RIP,"dp",&(sbPtr->DynPtr->Position),pitch);
				break;
			case 5:
			 	Sound_Play(SID_PRED_SLASH,"dp",&(sbPtr->DynPtr->Position),pitch);
				break;
			default:
			 	Sound_Play(SID_HIT_FLESH,"dp",&(sbPtr->DynPtr->Position),pitch);
				break;
		}

		switch ((rand >> 4) % 5)
		{					
			case 0:
			 	Sound_Play(SID_ALIEN_SCREAM,"dp",&(sbPtr->DynPtr->Position),-PREDALIEN_PITCH_CHANGE+pitch);
				break;
			case 1:
			 	Sound_Play(SID_ALIEN_HIT,"dp",&(sbPtr->DynPtr->Position),-PREDALIEN_PITCH_CHANGE+pitch);
				break;
			case 2:
			 	Sound_Play(SID_ALIEN_HIT2,"dp",&(sbPtr->DynPtr->Position),-PREDALIEN_PITCH_CHANGE+pitch);
		 		break;
			case 3:
			 	Sound_Play(SID_ALIEN_HISS1,"dp",&(sbPtr->DynPtr->Position),-PREDALIEN_PITCH_CHANGE+pitch);
				break;
			default:
				Sound_Play(SID_ALIEN_HISS,"dp",&(sbPtr->DynPtr->Position),-PREDALIEN_PITCH_CHANGE+pitch);
				break;
		}
	}
		
	/* reduce pa-q health */
	//paqStatusPointer->health -= damage; 	
	if(sbPtr->SBDamageBlock.Health > 0) return; 

	/* we have been killed... */
	paqStatusPointer->NearBehaviourState=PAQNS_Dying;    
	paqStatusPointer->FarBehaviourState=PAQFS_Dying;    
	paqStatusPointer->stateTimer=PAQ_DIETIME;    
	SetPAQAnimationSequence(sbPtr,PaqSQ_Dying,10);

	/* stop motion */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
	/* turn off collisions */
	if(sbPtr->DynPtr)
	{
		sbPtr->DynPtr->IsStatic	= 1;
		sbPtr->DynPtr->DynamicsType	= DYN_TYPE_NO_COLLISIONS;
		sbPtr->DynPtr->GravityOn = 0;
	}		
}



/*------------------------Patrick 18/2/97-----------------------------
  Predator-Alien & Queen far state behaviour functions
  --------------------------------------------------------------------*/

static void Execute_PAQFS_Wait(STRATEGYBLOCK *sbPtr)
{
	/* do absolutely nothing at all */
}

static void Execute_PAQFS_Hunt(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	
	paqStatusPointer->stateTimer -= NormalFrameTime;	
	if(paqStatusPointer->stateTimer > 0) return;
			
	/* check for state changes */
	if(!PAQShouldAttackPlayer())
	{
		/* we should be wandering */
		paqStatusPointer->FarBehaviourState = PAQFS_Wander;
		paqStatusPointer->stateTimer = 0;	/* forces execution of new state next frame */
		return;
	}

	/* get the target */
	targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr);	
	if(!targetModule)
	{
		paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;
		return;		
	}

	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarPAQTargetModule(sbPtr, targetModule);
	/* reset timer */
	paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;
}

static void Execute_PAQFS_Wander(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	
	paqStatusPointer->stateTimer -= NormalFrameTime;	
	if(paqStatusPointer->stateTimer > 0) return;
			
	/* check for state changes */
	if(!PAQShouldAttackPlayer())
	{
		/* we should be wandering */
		paqStatusPointer->FarBehaviourState = PAQFS_Hunt;
		paqStatusPointer->stateTimer = 0;	/* forces execution of new state next frame */
		return;
	}

	/* get the target */
	targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,NULL);

	if(!targetModule)
	{
		paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;
		return;		
	}

	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarPAQTargetModule(sbPtr, targetModule);
	/* reset timer */
	paqStatusPointer->stateTimer = PAQ_FAR_MOVE_TIME;
}



static void ProcessFarPAQTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
	NPC_TARGETMODULESTATUS targetStatus;
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(targetModule);
	LOCALASSERT((sbPtr->I_SBtype == I_BehaviourPredatorAlien)||(sbPtr->I_SBtype == I_BehaviourQueenAlien));
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);
	LOCALASSERT(paqStatusPointer);
	    
	targetStatus = GetTargetAIModuleStatus(sbPtr, targetModule);	
	switch(targetStatus)
	{
		case(NPCTM_NoEntryPoint):
		{
			/* do nothing */
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_NormalRoom):
		{
			/* locate to target	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_AirDuct):
		{
			/* do nothing - pa-q's can't go in air ducts	*/
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_LiftTeleport):
		{
			/* do nothing - pa-q's can't go into lifts	*/
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_ProxDoorOpen):
		{
			/* locate to target: don't need to move thro'quick, as door is constantly retriggered	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}		
		case(NPCTM_ProxDoorNotOpen):
		{
			MODULE *renderModule;
			renderModule=*(targetModule->m_module_ptrs);
			/* trigger the door, and set timer to quick so we can catch the door when it's open */
			((PROXDOOR_BEHAV_BLOCK *)renderModule->m_sbptr->SBdataptr)->alienTrigger = 1;
			break;
		}
		case(NPCTM_LiftDoorOpen):
		{
			/* do nothing - can't use lifts	*/
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_LiftDoorNotOpen):
		{
		   /*  do nothing - can't open lift doors */
			FarNpc_FlipAround(sbPtr);
		   	break;
		}
		case(NPCTM_SecurityDoorOpen):
		{
			/* locate to target, and move thro' quick as we can't retrigger	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_SecurityDoorNotOpen):
		{
			MODULE *renderModule;
			renderModule=*(targetModule->m_module_ptrs);
			/* do some door opening stuff here. Door should stay open for long enough
			for us to catch it open next time */
			RequestState((renderModule->m_sbptr),1,0);
			break;
		}
		default:
		{
			LOCALASSERT(1==0);
			break;
		}
	}		
}


/*------------------------Patrick 18/2/97-----------------------------
  Predator-Alien & Queen near state behaviour functions
  --------------------------------------------------------------------*/

static void Execute_PAQNS_Wait(STRATEGYBLOCK *sbPtr)
{	
	PAQ_STATUS_BLOCK *paqStatusPointer;    

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	
	/* test for attack */
	if(PAQShouldAttackPlayer())
	{
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		paqStatusPointer->NearBehaviourState = PAQNS_Approach;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Run,10);					
		paqStatusPointer->stateTimer = 0;			 				
		return;
	}

	/* still waiting: decrement timer */
	paqStatusPointer->stateTimer-=NormalFrameTime;
	
	if(paqStatusPointer->stateTimer<=0)
	{
		/* waiting has expired: go to wander */
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPC_InitWanderData(&(paqStatusPointer->wanderData));
		paqStatusPointer->NearBehaviourState = PAQNS_Wander;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Run,10);			
		paqStatusPointer->stateTimer = 0;		
	}
}

static void Execute_PAQNS_Approach(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	VECTORCH velocityDirection = {0,0,0};
	DYNAMICSBLOCK *dynPtr;
	VECTORCH targetPosition;
	int targetIsAirduct = 0;
	
	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	LOCALASSERT(sbPtr->DynPtr);

	dynPtr = sbPtr->DynPtr;
				
	/* check for state changes... */
	if(!(PAQShouldAttackPlayer()))
	{
		paqStatusPointer->NearBehaviourState = PAQNS_Wait;
		paqStatusPointer->stateTimer = PAQ_NEARWAITTIME;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Stand,10);
		return;
	}
		
	/* check if we want to close-attack */
	if(VectorDistance((&Player->ObStrategyBlock->DynPtr->Position),(&sbPtr->DynPtr->Position)) < PAQ_CLOSE_ATTACK_RANGE)
	{			
		paqStatusPointer->NearBehaviourState = PAQNS_Attack;  		
		paqStatusPointer->stateTimer = PAQ_NEAR_CLOSEATTACK_TIME;
		
		/* choice of two animations for pred-alien */
		if(sbPtr->I_SBtype==I_BehaviourPredatorAlien)
		{
			if(FastRandom()&0x01) SetPAQAnimationSequence(sbPtr,PaqSQ_Attack,10);
			else SetPAQAnimationSequence(sbPtr,PaqSQ_Attack2,10);
		}
		else SetPAQAnimationSequence(sbPtr,PaqSQ_Attack,10);
		
		return;
	}

	/* Do some approach sound */
  {
		unsigned int random=FastRandom();
   	  
		switch (random & 255)
	  {
	    case 0:
	    {
	      Sound_Play(SID_ALIEN_SCREAM,"d",&dynPtr->Position);
	      break;
	  	}
	    case 1:
	    {
	      Sound_Play(SID_ALIEN_HISS,"d",&dynPtr->Position);
	    	break;
	    }
	    case 2:
	    {
	    	Sound_Play(SID_ALIEN_HISS1,"d",&dynPtr->Position);
	      break;
	    }
		  case 3:
	    {
	      Sound_Play(SID_ALIEN_HIT,"d",&dynPtr->Position);
	      break;
	  	}
    	case 4:
	    {
	     	Sound_Play(SID_ALIEN_HIT2,"d",&dynPtr->Position);
	    	break;
	    }
		     
	    default:
	    {
	      break;
	    }
	 	}

		if(sbPtr->I_SBtype==I_BehaviourPredatorAlien)
		{
			switch ((random >> 8) & 255)
	   	{
			  case 0:
		    {
		      Sound_Play(SID_PRED_SNARL,"d",&dynPtr->Position);
		     	break;
		    }
		    case 1:
		    {
		      Sound_Play(SID_PRED_SCREAM1,"d",&dynPtr->Position);
		      break;
		    }
		    case 2:
		    {
		      Sound_Play(SID_PRED_LOUDROAR,"d",&dynPtr->Position);
		      break;
		    }
				case 3:
		    {
		      Sound_Play(SID_PRED_SHORTROAR,"d",&dynPtr->Position);
		      break;
 			  }
 	    	default:
 	    	{
 	    		break;
 	    	}
 	  	}  
 		}
	}


	/*still approaching then... first, find point to aim for*/
	NPCGetMovementTarget(sbPtr, Player->ObStrategyBlock, &targetPosition, &targetIsAirduct);	
	NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,1);
	NPCSetVelocity(sbPtr, &velocityDirection, paqStatusPointer->nearSpeed);	

	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(paqStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.environment)
		{
			/* go to avoidance */
			NPC_InitMovementData(&(paqStatusPointer->moveData));
			NPCGetAvoidanceDirection(sbPtr, &(paqStatusPointer->moveData.avoidanceDirn),&obstruction);						
			paqStatusPointer->NearBehaviourState = PAQNS_Avoidance;  		
			paqStatusPointer->stateTimer = NPC_AVOIDTIME;
			/* no sequence change required */
			return;
		}
		if(obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage, ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(paqStatusPointer->moveData), &targetPosition, &velocityDirection))
	{
		/* go to avoidance */
		NPC_OBSTRUCTIONREPORT obstruction = {1,0,0};
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPCGetAvoidanceDirection(sbPtr, &(paqStatusPointer->moveData.avoidanceDirn),&obstruction);						
		paqStatusPointer->NearBehaviourState = PAQNS_Avoidance;  		
		paqStatusPointer->stateTimer = NPC_AVOIDTIME;
		/* no sequence change required */
		return;
	}
}

static void Execute_PAQNS_Attack(STRATEGYBLOCK *sbPtr)
{
	VECTORCH orientationDirn;
	PAQ_STATUS_BLOCK *paqStatusPointer; 
	DYNAMICSBLOCK *dynPtr;   
	int i;

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	LOCALASSERT(sbPtr->DynPtr);
	
	dynPtr = sbPtr->DynPtr;	          		

	/* Orientate towards player, just to make sure we're facing */
	orientationDirn.vx = Player->ObWorld.vx - sbPtr->DynPtr->Position.vx;
	orientationDirn.vy = 0;
	orientationDirn.vz = Player->ObWorld.vz - sbPtr->DynPtr->Position.vz;
	i = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
	
	if(VectorDistance(&(sbPtr->DynPtr->Position), &(Player->ObStrategyBlock->DynPtr->Position)) > PAQ_CLOSE_ATTACK_RANGE)
	{
		/* switch to approach state: don't need to directly test if we should switch to wander,
		as approach state will do this anyway... */	
		NPC_InitMovementData(&(paqStatusPointer->moveData));
   		paqStatusPointer->NearBehaviourState = PAQNS_Approach;
		paqStatusPointer->stateTimer = 0;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Run,10);
		return;  		
	}

	/* Make the attack sound */
	if (paqStatusPointer->stateTimer == PAQ_NEAR_CLOSEATTACK_TIME)
	{

		if(sbPtr->I_SBtype==I_BehaviourPredatorAlien)
		{
			#define PREDALIEN_ATTACK_PITCH 350

			unsigned int rand=FastRandom();

 	  	switch (rand & 7)
 	  	{
 	  		case 0:
 	  		{
 	    		Sound_Play(SID_SWIPE,"dp",&dynPtr->Position,(rand&255)-128-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
 	    	case 1:
 	    	{
 	    		Sound_Play(SID_SWIPE2,"dp",&dynPtr->Position,(rand&255)-128-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
 	    	case 2:
 	    	{
 	    		Sound_Play(SID_SWIPE3,"dp",&dynPtr->Position,(rand&255)-128-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
				case 3:
 	    	{
 	    		Sound_Play(SID_TAIL,"dp",&dynPtr->Position,(rand & 255)-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
				case 4:
 	    	{
 	    		Sound_Play(SID_PRED_SLASH,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
				case 5:
 	    	{
 	    		Sound_Play(SID_RIP,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
		 		case 6:
 	    	{
 	    		Sound_Play(SID_SWISH,"dp",&dynPtr->Position,-(rand & 255)-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
				case 7:
 	    	{
 	    		Sound_Play(SID_ALIEN_HISS,"dp",&dynPtr->Position,-(rand & 255)-PREDALIEN_ATTACK_PITCH);					
 	    		break;
 	    	}
			 	default:
 				{
 					break;
 				}
			}
		}
		else
		{
			unsigned int rand=FastRandom();

 	  	switch (rand & 7)
 	  	{
 	  		case 0:
 	  		{
 	    		Sound_Play(SID_SWIPE,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
 	    	case 1:
 	    	{
 	    		Sound_Play(SID_SWIPE2,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
 	    	case 2:
 	    	{
 	    		Sound_Play(SID_SWIPE3,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
				case 3:
 	    	{
 	    		Sound_Play(SID_TAIL,"dp",&dynPtr->Position,-(rand & 255));					
 	    		break;
 	    	}
				case 4:
 	    	{
 	    		Sound_Play(SID_ALIEN_HISS1,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
				case 5:
 	    	{
 	    		Sound_Play(SID_ALIEN_SCREAM,"dp",&dynPtr->Position,(rand&255)-128);					
 	    		break;
 	    	}
		 		case 6:
 	    	{
 	    		Sound_Play(SID_SWISH,"dp",&dynPtr->Position,-(rand & 255));					
 	    		break;
 	    	}
				case 7:
 	    	{
 	    		Sound_Play(SID_ALIEN_HISS,"dp",&dynPtr->Position,-(rand & 255));					
 	    		break;
 	    	}
			 	default:
 				{
 					break;
 				}
			}
		}
	}

	/* Decrement the near state timer */
	paqStatusPointer->stateTimer -= NormalFrameTime;
	if(paqStatusPointer->stateTimer > 0)
	{
		/* DAMAGE PLAYER HERE: NB USE STATUS BLOCK DAMAGEINFLICTED*/
		CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_NPC_PAQ_CLAW].MaxDamage, ONE_FIXED,NULL);
		/* Note, might want to personalise this more. *
		 * I'd love to use a #warning here.           */
		paqStatusPointer->stateTimer = PAQ_NEAR_CLOSEATTACK_TIME;
	}
}

static void Execute_PAQNS_Avoidance(STRATEGYBLOCK *sbPtr)
{
	int terminateState = 0;
	PAQ_STATUS_BLOCK *paqStatusPointer;    

	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);	          		
		
	/* set velocity */
	LOCALASSERT((paqStatusPointer->moveData.avoidanceDirn.vx!=0)||
				(paqStatusPointer->moveData.avoidanceDirn.vy!=0)||
				(paqStatusPointer->moveData.avoidanceDirn.vz!=0));
	NPCSetVelocity(sbPtr, &(paqStatusPointer->moveData.avoidanceDirn), (paqStatusPointer->nearSpeed));

	/* next, decrement state timer */
	paqStatusPointer->stateTimer -= NormalFrameTime;
	if(paqStatusPointer->stateTimer <= 0) terminateState = 1;

	/* and check for an impeding collision */
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(paqStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.anySingleObstruction)
		{
			terminateState = 1;
		}
	}
	
	if(terminateState)
	{
		if(PAQShouldAttackPlayer())
		{
			/* switch to approach */
			NPC_InitMovementData(&(paqStatusPointer->moveData));
			paqStatusPointer->NearBehaviourState = PAQNS_Approach;  		
			paqStatusPointer->stateTimer = 0;
			/* no sequence change required */
		}
		else
		{
			/* switch to wander */
			NPC_InitMovementData(&(paqStatusPointer->moveData));
			NPC_InitWanderData(&(paqStatusPointer->wanderData));
			paqStatusPointer->NearBehaviourState = PAQNS_Wander;  		
			paqStatusPointer->stateTimer = 0;
			/* no sequence change required */
		}
	}
}

static void Execute_PAQNS_Wander(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	VECTORCH velocityDirection = {0,0,0};
	
	LOCALASSERT(sbPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	
	/* should we change to approach state? */
	if(PAQShouldAttackPlayer())
	{
		/* doesn't require a sequence change */
		NPC_InitMovementData(&(paqStatusPointer->moveData));	 		
		paqStatusPointer->NearBehaviourState = PAQNS_Approach;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Run,10);
		paqStatusPointer->stateTimer = 0;
		return;
	}

	/* wander target aquisition: if no target, or moved module */
	LOCALASSERT(sbPtr->containingModule);
	if(paqStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPC_FindAIWanderTarget(sbPtr,&(paqStatusPointer->wanderData),&(paqStatusPointer->moveData));
	}
	else if(paqStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_index)
	{
		NPC_FindAIWanderTarget(sbPtr,&(paqStatusPointer->wanderData),&(paqStatusPointer->moveData));
	}
	
	/* if we still haven't got one, go to wait */
	if(paqStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		paqStatusPointer->NearBehaviourState = PAQNS_Wait;
		paqStatusPointer->stateTimer = PAQ_NEARWAITTIME;
		SetPAQAnimationSequence(sbPtr,PaqSQ_Stand,10);
		return;
	}
	

	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(paqStatusPointer->wanderData.worldPosition),1);
	NPCSetVelocity(sbPtr, &velocityDirection, paqStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(paqStatusPointer->moveData),&obstruction,&destructableObject);
		if((obstruction.environment)||(obstruction.otherCharacter))
		{
			/* go to avoidance */
			NPC_InitMovementData(&(paqStatusPointer->moveData));
			NPCGetAvoidanceDirection(sbPtr, &(paqStatusPointer->moveData.avoidanceDirn),&obstruction);						
			paqStatusPointer->NearBehaviourState = PAQNS_Avoidance;  		
			paqStatusPointer->stateTimer = NPC_AVOIDTIME;
			/* no sequence change required */
			return;
		}
		if(obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage, ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(paqStatusPointer->moveData), &(paqStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		NPC_OBSTRUCTIONREPORT obstruction = {1,0,0};
		/* go to avoidance */
		NPC_InitMovementData(&(paqStatusPointer->moveData));
		NPCGetAvoidanceDirection(sbPtr, &(paqStatusPointer->moveData.avoidanceDirn),&obstruction);						
		paqStatusPointer->NearBehaviourState = PAQNS_Avoidance;  		
		paqStatusPointer->stateTimer = NPC_AVOIDTIME;
		/* no sequence change required */
		return;
	}
}

static void Execute_Dying(STRATEGYBLOCK *sbPtr)
{
	PAQ_STATUS_BLOCK *paqStatusPointer;    
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);
	paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(paqStatusPointer);
	
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	sbPtr->DynPtr->LinImpulse.vx = 0;
	sbPtr->DynPtr->LinImpulse.vy = 0;
	sbPtr->DynPtr->LinImpulse.vz = 0;
	
	paqStatusPointer->stateTimer -= NormalFrameTime;
}

/* Patrick 7/7/97 ----------------------------------------------------
PAQ beaviour support functions
----------------------------------------------------------------------*/
static void SetPAQAnimationSequence(STRATEGYBLOCK *sbPtr, PAQ_SEQUENCE seq, int rate)
{
	SHAPEANIMATIONCONTROLDATA sacd;
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->SBdptr);
	
	InitShapeAnimationControlData(&sacd);
	sacd.seconds_per_frame = ONE_FIXED/rate;
	sacd.sequence_no = seq; 
	sacd.default_start_and_end_frames = 1;
	sacd.reversed = 0;
	if(seq==PaqSQ_Dying) sacd.stop_at_end = 1;
	else sacd.stop_at_end = 0;

	SetShapeAnimationSequence(sbPtr->SBdptr, &sacd);

	/* add follow on sequence ... */
	if(seq==PaqSQ_Dying)
	{
		sacd.sequence_no = PaqSQ_Dead;
		sacd.stop_at_end = 1;
		SetNextShapeAnimationSequence(sbPtr->SBdptr, &sacd);
	}
}

static int PAQShouldAttackPlayer(void)
{
	/* test for player being cloaked */
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);

		if((playerStatusPtr->cloakOn==1)&&(playerStatusPtr->cloakPositionGivenAway==0))
		return 0;
	}

	return 1;
}
