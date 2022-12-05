/*------------------------Patrick 26/11/96-----------------------------
  Source file for FAR AI alien behaviour etc....
  NB some of the functions in this file are re-used for other NPC AI.
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

#include "pheromon.h"
#include "bh_pred.h"
#include "bh_alien.h"
#include "bh_far.h"
#include "pfarlocs.h"
#include "bh_gener.h"
#include "pvisible.h"
#include "bh_marin.h"
#include "weapons.h"
#include "showcmds.h"
#include "pldnet.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "dxlog.h"

/* prototypes for this file */
static void Execute_AFS_Hunt(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Wait(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Retreat(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Wander(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Approach(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Attack(STRATEGYBLOCK *sbPtr);
static void Execute_AFS_Avoidance(STRATEGYBLOCK *sbPtr);
static int ProcessFarAlienTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule);
extern void AlienNearState_Dormant(STRATEGYBLOCK *sbPtr);
extern void AlienNearState_Awakening(STRATEGYBLOCK *sbPtr);
extern void AlienNearState_Taunting(STRATEGYBLOCK *sbPtr);

/* external global variables used in this file */
extern int NormalFrameTime;
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
static int entryPointFailures = 0;
extern int ShowHiveState;

extern SCENE Global_Scene;
extern SCENEMODULE **Global_ModulePtr;
static MODULE **Global_ModuleArrayPtr;

extern void Execute_Alien_Dying(STRATEGYBLOCK *sbPtr);

/*--------------------Patrick 9/12/96-----------------------
  Far Alien behaviour execution shell.
  Behaviour is defined by a set of states: the AFS_....
  enumeration defined in bh_alien.h.  
  In addition, the far alien state is defined by a timer: ie
  any state may 'timeout', possibly forcing a state change. 
  ----------------------------------------------------------*/
void FarAlienBehaviour(STRATEGYBLOCK *sbPtr)
{
	ALIEN_STATUS_BLOCK *alienStatusPointer;    
	char *descriptor;

	LOCALASSERT(sbPtr);
 	/* a precondition: there should be no display block */
 	LOCALASSERT(!(sbPtr->SBdptr));

 	/* get the alien's status block */
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	
	/* execute far behaviour state... */
	switch(alienStatusPointer->BehaviourState)
	{
   		case(ABS_Wait):
		{	
			Execute_AFS_Wait(sbPtr);
   			descriptor="Waiting";
   			break;
   		}
		case(ABS_Approach):
		case(ABS_Jump):
		{
			Execute_AFS_Approach(sbPtr);
			descriptor="Approaching";
			break;
		}
		case(ABS_Hunt):
		{
			Execute_AFS_Hunt(sbPtr);
   			descriptor="Hunting";
   			break;
   		}
   		case(ABS_Retreat):
		{	
			Execute_AFS_Retreat(sbPtr);
   			descriptor="Retreating";
   			break;
   		}
   		case(ABS_Wander):
		{	
			Execute_AFS_Wander(sbPtr);
   			descriptor="Wandering";
   			break;
   		}
		case(ABS_Attack):
		case(ABS_Pounce):
		{
			Execute_AFS_Attack(sbPtr);
			descriptor="Attacking";
			break;
		}
		case(ABS_Avoidance):
		{
			Execute_AFS_Avoidance(sbPtr);
			descriptor="Avoiding";
			break;
		}
		case(ABS_Dying):
		{
   			descriptor="Dying";
			Execute_Alien_Dying(sbPtr);
			break;
		}
		case ABS_Dormant:
		{
			AlienNearState_Dormant(sbPtr);
			descriptor="Dormant";
			break;		
		}		
		case ABS_Awakening:
		{
			AlienNearState_Awakening(sbPtr);
			descriptor="Awakening";
			break;		
		}
		case ABS_Taunting:
		{
			AlienNearState_Taunting(sbPtr);
			descriptor="Taunting";
			break;		
		}
		default:
		{
			descriptor=NULL;
			LOCALASSERT(1==0); /* should never get here */
		}
	}
	
	/* check here to see if the alien is in a doorway....
	If so, and it is a proximity door, make sure it is open. */
	{
		MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);

		if(doorType == MDT_ProxDoor) {
			((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->alienTrigger = 1;
		}
	}

	if (ShowHiveState) {
		/* Alien position print. */

		MODULE *thisModule = sbPtr->containingModule;
		
		LOCALASSERT(thisModule);

		PrintDebuggingText("This FAR %s ALIEN is in module %d, %s\n",descriptor,thisModule->m_index,thisModule->name);

	}
		
	/* make sure that we are inside a module:
	if this fires it means that a far alien is not inside
	the module it is supposed to be in */
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
			textprint("FAR ALIEN MODULE CONTAINMENT FAILURE \n");

			LOGDXFMT(("Alien containment failure: %s alien is in %s, position is %d,%d,%d:\nModule extents are: %d:%d, %d:%d, %d:%d",
				descriptor,
				thisModule->name,localCoords.vx,localCoords.vy,localCoords.vz,
				thisModule->m_maxx,thisModule->m_minx,thisModule->m_maxy,thisModule->m_miny,
				thisModule->m_maxz,thisModule->m_minz));

			LOCALASSERT(1==0);
		}  
	}
	#endif

	/* textprint("NO ENTRY POINT COUNT %d \n", entryPointFailures);	*/


}

/*--------------------Patrick 9/12/96-----------------------
  Execute far alien hunting behaviour....
  This is basically the default alien behaviour, following
  the player's pheromone trail.

  On hunting behaviour, the alien moves between modules
  using the pre-computed module locations list.  After
  being relocated to a new module, the alien waits for 'x'
  seconds (having a good sniff around) then decides which
  module to move into next.

  NB only passable modules are updated with player smell,
  so aliens won't get stuck behind non-automatic doors.
  ----------------------------------------------------------*/
static void Execute_AFS_Hunt(STRATEGYBLOCK *sbPtr)
{
	ALIEN_STATUS_BLOCK *alienStatusPointer;    
	AIMODULE *targetModule = 0;

 	/* get the alien's status block */
	LOCALASSERT(sbPtr);
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	/* Decrement the Far state timer */
	alienStatusPointer->FarStateTimer -= NormalFrameTime;
		
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(alienStatusPointer->FarStateTimer>0) return;
		
	/* check the alien hive, to see itf we've switched to regroup:
	if so, reset the alien far behaviour state to retreat, with the alien far 
	state timer set to 0, forcing a retreating movement next frame... */
	if(NPCHive.currentState == HS_Regroup)
	{
		alienStatusPointer->BehaviourState = ABS_Retreat;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
	}

	/* check to see if the player is invisible, etc... */
//	if ((!AlienIsAwareOfTarget(sbPtr))
//		||(alienStatusPointer->Target!=Player->ObStrategyBlock))
	if ((!AlienIsAwareOfTarget(sbPtr))&&(alienStatusPointer->Target==Player->ObStrategyBlock)
		&&(NPC_IsDead(Player->ObStrategyBlock)))
	{
		alienStatusPointer->BehaviourState = ABS_Wander;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
	}
	
	/* get the target */
	targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,1);

	/* if there is no target module, it means that the alien is trapped in an
	unlinked module. In this case, reset the timer and return. */			
	if(!targetModule)
	{
		alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;

		#if 0
		/* Better have a handler for this. */
		alienStatusPointer->BehaviourState = ABS_Dormant;
		alienStatusPointer->CurveTimeOut = 0;
		if (HModelSequence_Exists(&alienStatusPointer->HModelController,HMSQT_AlienStand,ASSS_Dormant)) {
			SetAlienShapeAnimSequence_Core(sbPtr,HMSQT_AlienStand,ASSS_Dormant,-1,ONE_FIXED);
		} else {
			SetAlienShapeAnimSequence_Core(sbPtr,HMSQT_AlienStand,ASSS_Standard,ONE_FIXED,(ONE_FIXED>>2));
		}
		#else
		alienStatusPointer->BehaviourState = ABS_Wander;
		alienStatusPointer->CurveTimeOut = 0;
		#endif
		return;		
	}

	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));
	
	#if 0
	ProcessFarAlienTargetModule(sbPtr, targetModule);
	/* reset the timer */
	alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
	#else
	alienStatusPointer->FarStateTimer = ProcessFarAlienTargetModule(sbPtr, targetModule);
	#endif
}


/*-----------------------Patrick 9/12/96--------------------------
  Far alien waiting behaviour functions:
  Do nothing: when it becomes visible, it will switch to attack, or
  near wait and then wander...
  ----------------------------------------------------------------*/
static void Execute_AFS_Wait(STRATEGYBLOCK *sbPtr)
{
	/* do nothing */

	//#if ULTRAVIOLENCE
	/* Look, now there might be no enemies. */
	#if 0
	/* ...I think not. */

	ALIEN_STATUS_BLOCK *alienStatusPointer;    

	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	alienStatusPointer->BehaviourState = ABS_Hunt;
	alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/

	#endif
}

static void Execute_AFS_Approach(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    

	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	/* For the moment, switch to attack or to hunt. */

	if (Validate_Target(alienStatusPointer->Target,alienStatusPointer->Target_SBname)==0) {
		/* Whoops, no target. */	
		//GLOBALASSERT(0);
		/* Go back to hunt. */
		alienStatusPointer->BehaviourState = ABS_Hunt;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
		/* Should check for this in AlienBehaviour. */
	}

	/* See if we're in the same module. */

	if (alienStatusPointer->Target->containingModule->m_aimodule!=sbPtr->containingModule->m_aimodule) {
		/* Go back to hunt. */
		alienStatusPointer->BehaviourState = ABS_Hunt;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
	} else {
		/* Go to attack. */
		alienStatusPointer->BehaviourState = ABS_Attack;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
	}

}

static void Execute_AFS_Attack(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    

	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	/* For the moment, switch to hunt, or smite the target. */

	if (Validate_Target(alienStatusPointer->Target,alienStatusPointer->Target_SBname)==0) {
		/* Whoops, no target. */	
		GLOBALASSERT(0);
		/* Should check for this in AlienBehaviour. */
	}

	/* See if we're in the same module. */

	if (alienStatusPointer->Target->containingModule->m_aimodule!=sbPtr->containingModule->m_aimodule) {
		/* Go back to hunt. */
		alienStatusPointer->BehaviourState = ABS_Hunt;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
	} else {
		/* Decrement the Far state timer */
		alienStatusPointer->FarStateTimer -= NormalFrameTime;
		
		/* check if far state timer has timed-out. If so, it is time 
		to do something. Otherwise just return. */
		if(alienStatusPointer->FarStateTimer>0) return;
	
		#if 0
		CauseDamageToObject(alienStatusPointer->Target,&TemplateAmmo[AMMO_NPC_ALIEN_CLAW].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		#endif
		/* Kersplat. */
		alienStatusPointer->FarStateTimer=ALIEN_ATTACKTIME;
		/* Cunning, eh? */

	}

}

static void Execute_AFS_Avoidance(STRATEGYBLOCK *sbPtr) {

	/* No obstacles in far behaviour. */
	ALIEN_STATUS_BLOCK *alienStatusPointer;    

	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	Initialise_AvoidanceManager(sbPtr,&alienStatusPointer->avoidanceManager);
	
	/* Go directly to hunt.  Do not pass GO. */
	alienStatusPointer->BehaviourState = ABS_Hunt;
	alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/

}

static void Execute_AFS_Retreat(STRATEGYBLOCK *sbPtr)
{
	ALIEN_STATUS_BLOCK *alienStatusPointer;    
	AIMODULE *targetModule = 0;

 	/* get the alien's status block */
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	/* Decrement the Far state timer */
	alienStatusPointer->FarStateTimer -= NormalFrameTime;
		
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(alienStatusPointer->FarStateTimer>0) return;

	/* check the alien hive, to see if we've switched to attack:
	if so, reset the alien far behaviour state to attack, with the alien far 
	state timer set to 0, forcing a movement next frame... 
	NB if we can't attack the player, hunting function will automatically
	switch to wander */
	if(NPCHive.currentState == HS_Attack)
	{
		alienStatusPointer->BehaviourState = ABS_Hunt;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
	}

	/* get the target module... */
	targetModule = FarNPC_GetTargetAIModuleForRetreat(sbPtr);

	/* if there is no target module, reset the timer and return. */			
	if(!targetModule)
	{
		alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
		return;		
	}
	
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));

	#if 0	
	ProcessFarAlienTargetModule(sbPtr, targetModule);
	/* reset the timer */
	alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;			
	#else
	alienStatusPointer->FarStateTimer = ProcessFarAlienTargetModule(sbPtr, targetModule);
	#endif
}

static void Execute_AFS_Wander(STRATEGYBLOCK *sbPtr)
{
	ALIEN_STATUS_BLOCK *alienStatusPointer;    
	AIMODULE *targetModule = 0;

 	/* get the alien's status block */
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);

	/* Decrement the Far state timer */
	alienStatusPointer->FarStateTimer -= NormalFrameTime;
		
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(alienStatusPointer->FarStateTimer>0) return;

	/* check for state changes:
	if hive says retreat, then retreat, regardless of whether or not we can see the player 
	otherwise, if we can see the player, go to hunt */
	if(NPCHive.currentState == HS_Regroup)
	{
		alienStatusPointer->BehaviourState = ABS_Retreat;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
	}
	
	/* see if we want to switch to attack */
	if(AlienIsAwareOfTarget(sbPtr))
	{
		alienStatusPointer->BehaviourState = ABS_Hunt;
		alienStatusPointer->FarStateTimer = 0; /* forces execution of new state next frame*/
		return;
	}
	/* That used to be 100% for ULTRAVIOLENCE. But,
	now, there might concievably be NO targets, -> wander. */	

	/* get the target module... */
	targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,NULL,1);

	/* if there is no target module, reset the timer and return. */			
	if(!targetModule)
	{
		alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
		return;		
	}
	
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));
	
	#if 0
	ProcessFarAlienTargetModule(sbPtr, targetModule);
	/* reset the timer */
	alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;			
	#else
	alienStatusPointer->FarStateTimer = ProcessFarAlienTargetModule(sbPtr, targetModule);
	#endif
}




/*--------------------Patrick 27/1/97----------------------
  This function is used by the various far alien behaviour 
  functions to move an alien NPC: it decides whether and how 
  to move an alien into a given target.
  
  2/7/97 extra bit:
  If a location fails, we flip the y-orientation of the npc,
  so that wandering behaviour can find a new path;
  ----------------------------------------------------------*/
static int ProcessFarAlienTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
	NPC_TARGETMODULESTATUS targetStatus;
	ALIEN_STATUS_BLOCK *alienStatusPointer;
	VECTORCH oldPos;

	LOCALASSERT(sbPtr);
	LOCALASSERT(targetModule);
	LOCALASSERT(sbPtr->I_SBtype == I_BehaviourAlien);

 	/* get the alien's status block */
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	
	oldPos=sbPtr->DynPtr->Position;
		
	/* get the target module's status, and decide what to do */
	targetStatus = GetTargetAIModuleStatus(sbPtr, targetModule,1);
	switch(targetStatus)
	{
		case(NPCTM_NoEntryPoint):
		{
			/* do nothing: can't get in. */
			FarNpc_FlipAround(sbPtr);
			entryPointFailures++;
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
			/* locate to target	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_LiftTeleport):
		{
			/* do nothing: aliens can't use lifts */
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_ProxDoorOpen):
		{
			/* locate to target	*/
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
			#if 0
			/* do nothing: don't really want to go into a lift, or we'll get trapped */
			FarNpc_FlipAround(sbPtr);
			#else
			/* Another pre-written screw up! */
			LocateFarNPCInAIModule(sbPtr, targetModule);
			#endif
			break;
		}
		case(NPCTM_LiftDoorNotOpen):
		{
		   /*  do nothing - well, there's nothing we can do, really*/
			FarNpc_FlipAround(sbPtr);
		   	break;
		}
		case(NPCTM_SecurityDoorOpen):
		{
			/* locate to target	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_SecurityDoorNotOpen):
		{
		   /*  do nothing - well, there's nothing we can do, really*/
			FarNpc_FlipAround(sbPtr);
		   	break;
		}
		default:
		{
			LOCALASSERT(1==0);
		}
	}
	/* Now, deduce how far it's moved... */
	oldPos.vx-=sbPtr->DynPtr->Position.vx;
	oldPos.vy-=sbPtr->DynPtr->Position.vy;
	oldPos.vz-=sbPtr->DynPtr->Position.vz;
	{
		int distance;

		distance=Approximate3dMagnitude(&oldPos);

		if (distance==0) {
			return(ALIEN_FAR_MOVE_TIME);
		} else {
			/* How long? */
			return(DIV_FIXED(distance,(ALIEN_FORWARDVELOCITY>>1)));
		}
	}
	return(ALIEN_FAR_MOVE_TIME);
}


/*-----------------------------------------------------------------------
	
	FUNCTIONS USED BY ALL NPC FAR BEHAVIOUR FUNCTIONS

-------------------------------------------------------------------------*/


/*--------------------Patrick 27/1/97----------------------
  This function relocates an NPC into a target module.
  If the module is visible it uses the entry point. If not
  it uses an auxilary location, or the entry point if there
  aren't any.

  2/7/97: added a bit to update the npc orientation when
  moving. This orientation is used for wandering behaviour
  ----------------------------------------------------------*/
void LocateFarNPCInModule(STRATEGYBLOCK *sbPtr, MODULE *targetModule)
{
	int noOfAuxLocs;
	VECTORCH *auxLocsList; 
	int noOfEntryPoints;
	FARENTRYPOINT *entryPointsList;
	FARENTRYPOINT *targetEntryPoint;
	VECTORCH newPosition;

	/* a pre-condition... */
	GLOBALASSERT(ModuleIsPhysical(targetModule));

	/* now: a few tests for npc's that are generated... (aliens and marines) */
	if((sbPtr->I_SBtype==I_BehaviourAlien)||(sbPtr->I_SBtype==I_BehaviourMarine))
	{
		if((PherAi_Buf[(targetModule->m_index)]) >= MAX_GENERATORNPCSPERMODULE)		 
		{
			/* do nothing (since there are only a few auxilary locs per module) */
			return;	
		}

		if(ModuleCurrVisArray[(targetModule->m_index)])
		{
			/* the target is visible... */
			if(NumGeneratorNPCsVisible() >= MAX_VISIBLEGENERATORNPCS)
			{
				/* do nothing: there are already enough visible npcs */
				return;
			}
		}
	}
	
	/* now move the npc to it's target... */
	noOfAuxLocs = FALLP_AuxLocs[(targetModule->m_index)].numLocations;
	auxLocsList = FALLP_AuxLocs[(targetModule->m_index)].locationsList;  
	noOfEntryPoints = FALLP_EntryPoints[(targetModule->m_index)].numEntryPoints;
	entryPointsList = FALLP_EntryPoints[(targetModule->m_index)].entryPointsList;  
	
	/* find the entry point for the target */
	LOCALASSERT(sbPtr->containingModule);
	targetEntryPoint = GetModuleEP(targetModule,(sbPtr->containingModule));
	LOCALASSERT(targetEntryPoint);
	
	/* if it's visible, use the entry point.
	if it's not visible, use an auxilary location. If there aren't any auxilary
	locations, use the entry point. */

	if(ModuleCurrVisArray[(targetModule->m_index)])
	{
		newPosition = targetEntryPoint->position;
   	}
	else
	{
   		/* pick an auxilary location: if there aren't any, use the entry point */
		if(noOfAuxLocs)
		{
			int targetLocInx;
   			int npcHeight;
   			targetLocInx = FastRandom() % noOfAuxLocs;
   			newPosition = auxLocsList[targetLocInx];
   			/* move up 1/2 npc height, plus a bit more(100). this only applies
   			to auxilary locations, not eps */			
			npcHeight = (mainshapelist[sbPtr->shapeIndex]->shapemaxy 
   				- mainshapelist[sbPtr->shapeIndex]->shapeminy)/2;
   			if(npcHeight>1000) npcHeight = 1000;   					
   			newPosition.vy -=(npcHeight + 100); 	 
   		}
		else newPosition = targetEntryPoint->position;
   	}
   	
   	/* now set the alien's new position and current module. 
	   NB this is world position + alien height in y + a little extra in y to make sure */
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);

		dynPtr->Position = newPosition;
		dynPtr->Position.vx += targetModule->m_world.vx;
		dynPtr->Position.vy += targetModule->m_world.vy;
		dynPtr->Position.vz += targetModule->m_world.vz;
		dynPtr->PrevPosition = dynPtr->Position;

	   	dynPtr->OrientEuler.EulerX = 0;
	   	dynPtr->OrientEuler.EulerZ = 0;
	   	{
			VECTORCH vec; 
			vec.vx = targetModule->m_world.vx - sbPtr->containingModule->m_world.vx;
			vec.vz = targetModule->m_world.vz - sbPtr->containingModule->m_world.vz;
			vec.vy = 0;
			Normalise(&vec);
	   		dynPtr->OrientEuler.EulerY = ArcTan(vec.vx, vec.vz);
		}
	}
	/* finally, update the alien's module */
	sbPtr->containingModule = targetModule;	
}

void LocateFarNPCInAIModule(STRATEGYBLOCK *sbPtr, AIMODULE *targetModule)
{
	int noOfAuxLocs;
	VECTORCH *auxLocsList; 
	int noOfEntryPoints;
	FARENTRYPOINT *entryPointsList;
	FARENTRYPOINT *targetEntryPoint;
	VECTORCH newPosition;
	MODULE *renderModule;
	int targetLocInx;

	SCENEMODULE *smptr;

	smptr = Global_ModulePtr[Global_Scene];
	Global_ModuleArrayPtr = smptr->sm_marray;

	/* now: a few tests for npc's that are generated... (aliens and marines) */
	if((sbPtr->I_SBtype==I_BehaviourAlien)||(sbPtr->I_SBtype==I_BehaviourMarine))
	{
		if((PherAi_Buf[(targetModule->m_index)]) >= MAX_GENERATORNPCSPERMODULE)		 
		{
			/* do nothing (since there are only a few auxilary locs per module) */
			return;	
		}

		//if(ModuleCurrVisArray[(*(targetModule->m_module_ptrs))->m_index])
		if (AIModuleIsVisible(targetModule)) 
		{
			/* the target is visible... */
			if(NumGeneratorNPCsVisible() >= MAX_VISIBLEGENERATORNPCS)
			{
				/* do nothing: there are already enough visible npcs */
				return;
			}
		}
	}
	
	/* now move the npc to it's target... */
	noOfAuxLocs = FALLP_AuxLocs[(targetModule->m_index)].numLocations;
	auxLocsList = FALLP_AuxLocs[(targetModule->m_index)].locationsList;  
	noOfEntryPoints = FALLP_EntryPoints[(targetModule->m_index)].numEntryPoints;
	entryPointsList = FALLP_EntryPoints[(targetModule->m_index)].entryPointsList;  
	
	/* find the entry point for the target */
	LOCALASSERT(sbPtr->containingModule);
	targetEntryPoint = GetAIModuleEP(targetModule,(sbPtr->containingModule->m_aimodule));
	LOCALASSERT(targetEntryPoint);
	
	/* if it's visible, use the entry point.
	if it's not visible, use an auxilary location. If there aren't any auxilary
	locations, use the entry point. */

	//if(ModuleCurrVisArray[(*(targetModule->m_module_ptrs))->m_index])
	if (AIModuleIsVisible(targetModule)) 
	{
		newPosition = targetEntryPoint->position;
		targetLocInx=-1;
   	}
	else
	{
   		/* pick an auxilary location: if there aren't any, use the entry point */
		if(noOfAuxLocs)
		{
			#if 0 
   			int npcHeight;
			#endif
   			targetLocInx = FastRandom() % noOfAuxLocs;
   			newPosition = auxLocsList[targetLocInx];
   			/* move up 1/2 npc height, plus a bit more(100). this only applies
   			to auxilary locations, not eps */			
			#if 0 
			npcHeight = (mainshapelist[sbPtr->shapeIndex]->shapemaxy 
   				- mainshapelist[sbPtr->shapeIndex]->shapeminy)/2;
   			if(npcHeight>1000) npcHeight = 1000;
   			newPosition.vy -=(npcHeight + 100); 	 
			#endif

			if(AvP.Network != I_No_Network)
			{
				//Multiplayer game
				//send this new position to the other players
				AddNetMsg_FarAlienPosition(sbPtr,targetModule->m_index,targetLocInx,FALSE);
			}
   		}
		else {
			newPosition = targetEntryPoint->position;
			if(AvP.Network != I_No_Network)
			{
				//Multiplayer game
				//send this new position to the other players
				AddNetMsg_FarAlienPosition(sbPtr,targetModule->m_index,sbPtr->containingModule->m_aimodule->m_index,TRUE);
			}
		}
   	}
	
	{
		VECTORCH temp_Pos;

		temp_Pos.vx=newPosition.vx+targetModule->m_world.vx;
		temp_Pos.vy=newPosition.vy+targetModule->m_world.vy;
		temp_Pos.vz=newPosition.vz+targetModule->m_world.vz;
		
		renderModule=ModuleFromPosition(&temp_Pos,sbPtr->containingModule);
		if (renderModule==NULL) {
			#if 0
			LOGDXFMT(("Right, here comes the assert.\nNoOfAuxLocs %d.\nTargetLocInx %d\n"
			,noOfAuxLocs,targetLocInx));
			if (*(targetModule->m_module_ptrs)) {
				LOGDXFMT(("TargetModule %s.\nContainingModule %s.\n",(*(targetModule->m_module_ptrs))->name,sbPtr->containingModule->name));
			} else {
				LOGDXFMT(("TargetModule not found.\nContainingModule %s.\n",sbPtr->containingModule->name));
			}
			LOGDXFMT(("I really should crash out here.\n"));

			//GLOBALASSERT(renderModule);
			NewOnScreenMessage("DODGED A BULLET.\n");
			#endif
			return;
		}
	}
	   	
   	/* now set the alien's new position and current module. 
	   NB this is world position + alien height in y + a little extra in y to make sure */
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);

		dynPtr->Position = newPosition;
		dynPtr->Position.vx += targetModule->m_world.vx;
		dynPtr->Position.vy += targetModule->m_world.vy;
		dynPtr->Position.vz += targetModule->m_world.vz;
		dynPtr->PrevPosition = dynPtr->Position;

	   	dynPtr->OrientEuler.EulerX = 0;
	   	dynPtr->OrientEuler.EulerZ = 0;
	   	{
			VECTORCH vec; 
			vec.vx = targetModule->m_world.vx - sbPtr->containingModule->m_world.vx;
			vec.vz = targetModule->m_world.vz - sbPtr->containingModule->m_world.vz;
			vec.vy = 0;
			Normalise(&vec);
	   		dynPtr->OrientEuler.EulerY = ArcTan(vec.vx, vec.vz);
		}
	}
	/* finally, update the alien's module */
	sbPtr->containingModule = renderModule;	

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
			textprint("FAR ALIEN MODULE CONTAINMENT FAILURE \n");

			LOGDXFMT(("Alien containment failure: alien is in %s, position is %d,%d,%d:\nModule extents are: %d:%d, %d:%d, %d:%d",
				thisModule->name,localCoords.vx,localCoords.vy,localCoords.vz,
				thisModule->m_maxx,thisModule->m_minx,thisModule->m_maxy,thisModule->m_miny,
				thisModule->m_maxz,thisModule->m_minz));

			LOCALASSERT(1==0);
		}  
	}
	#endif
}


/*--------------------Patrick 10/12/96----------------------
  This function returns the status of a passed target 
  module that an NPC might want to move to.
  ----------------------------------------------------------*/

NPC_TARGETMODULESTATUS GetTargetAIModuleStatus(STRATEGYBLOCK *sbPtr, AIMODULE *targetModule, int alien)
{
	MODULEDOORTYPE doorStatus;
	MODULE *renderModule;

	/* first check for entry point from current module */
	{
		FARENTRYPOINT *targetEntryPoint;
		targetEntryPoint = GetAIModuleEP(targetModule,(sbPtr->containingModule->m_aimodule));

		if(targetEntryPoint == (FARENTRYPOINT *)0) return NPCTM_NoEntryPoint;			

		if (!alien) {
			if (targetEntryPoint->alien_only) {
				return NPCTM_NoEntryPoint;
			}
		}
	}
	
	renderModule=*(targetModule->m_module_ptrs);

	doorStatus = (ModuleIsADoor(renderModule));

	switch(doorStatus)
	{
		case(MDT_ProxDoor):
		{	
 			if(GetState(renderModule->m_sbptr)) 
 				return NPCTM_ProxDoorOpen;
			else
				return NPCTM_ProxDoorNotOpen;

			break;
		}

		case(MDT_LiftDoor):
		{	
 			if(GetState(renderModule->m_sbptr)) 
 				return NPCTM_LiftDoorOpen;
			else
				return NPCTM_LiftDoorNotOpen;

			break;
		}

		case(MDT_SecurityDoor):
		{	
 			if(GetState(renderModule->m_sbptr)) 
 				return NPCTM_SecurityDoorOpen;
			else
				return NPCTM_SecurityDoorNotOpen;

			break;
		}

		default:
		{
			LOCALASSERT(doorStatus==MDT_NotADoor);
		}

	}
	
	/* now check for lift */
	if(sbPtr->I_SBtype == I_BehaviourLift) return NPCTM_LiftTeleport;

	/* check for air duct */
	if(renderModule->m_flags & MODULEFLAG_AIRDUCT) return NPCTM_AirDuct;
	
	/* at this point, we know it's a room (or stairs) ... */
	return NPCTM_NormalRoom;	 	
}

/* Patrick 1/7/97-----------------------------------------
  A suit of functions for general far NPC use which return a 
  target module for hunting, wandering, and retreating
  ----------------------------------------------------------*/

AIMODULE *FarNPC_GetTargetAIModuleForHunt(STRATEGYBLOCK *sbPtr, int alien)
{
	AIMODULE **AdjModuleRefPtr;
	int AdjModuleIndex;
	unsigned int highestSmell = 0;
	AIMODULE* targetModule = (AIMODULE *)0;

	LOCALASSERT(sbPtr);	
	if(sbPtr->containingModule==NULL) return targetModule;
	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

	/* check that there is a list of adjacent modules, and that it is not
	empty (ie points to zero) */
	if(AdjModuleRefPtr)	
	{
		while(*AdjModuleRefPtr != 0)
		{
			/* get the index */
			AdjModuleIndex = (*AdjModuleRefPtr)->m_index;

			if (CheckAdjacencyValidity((*AdjModuleRefPtr), sbPtr->containingModule->m_aimodule,alien)) {
				/* if this adjacent module's smell value is higher than
				the current 'highest smell' record the new module as the
				target. */
				if(PherPl_ReadBuf[AdjModuleIndex] > highestSmell)
				{						
					highestSmell = PherPl_ReadBuf[AdjModuleIndex];
					targetModule = *AdjModuleRefPtr;							
				}
			}
			/* next adjacent module reference pointer */
			AdjModuleRefPtr++;
		}
	}
	/* Consider my module being the target. */
	if (PherPl_ReadBuf[sbPtr->containingModule->m_aimodule->m_index] > highestSmell) {
		targetModule=sbPtr->containingModule->m_aimodule;
	}

	return targetModule;
}

/* Patrick 2/7/96: this function returns a module for wandering to */
AIMODULE *FarNPC_GetTargetAIModuleForWander(STRATEGYBLOCK *sbPtr, AIMODULE *exception, int alien)
{
	AIMODULE **AdjModuleRefPtr;
	DYNAMICSBLOCK *dynPtr;
	AIMODULE* targetModule = (AIMODULE *)0;
	int bestDirn = -100000;	/* lower than the lowest */
	VECTORCH npcDirn;

	/* some checks */
	if(!sbPtr) return targetModule;	
	if(!sbPtr) return targetModule;	
	dynPtr = sbPtr->DynPtr;
	if(!dynPtr) return targetModule;	

	/* get npc 2d directional vector */
	npcDirn.vx = GetSin(dynPtr->OrientEuler.EulerY);
	npcDirn.vz = GetCos(dynPtr->OrientEuler.EulerY);
	npcDirn.vy = 0;
	Normalise(&npcDirn);

	/* init adjacent module pointer */
	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

	/* check that there is a list of adjacent modules, and that it is not
	empty (ie points to zero) */
	if(AdjModuleRefPtr)	
	{
		while(*AdjModuleRefPtr != 0)
		{
			AIMODULE *nextAdjModule = *AdjModuleRefPtr;
			VECTORCH moduleDirn;	
			int thisDirn;

			if (CheckAdjacencyValidity((*AdjModuleRefPtr), sbPtr->containingModule->m_aimodule,alien)) {
				moduleDirn.vx = nextAdjModule->m_world.vx - sbPtr->containingModule->m_world.vx;
				moduleDirn.vz = nextAdjModule->m_world.vz - sbPtr->containingModule->m_world.vz;
				moduleDirn.vy = 0;
				Normalise(&moduleDirn);
	
				thisDirn = DotProduct(&npcDirn,&moduleDirn);
				if( (thisDirn>bestDirn) && (exception!=nextAdjModule))
				{
					targetModule = nextAdjModule;
					bestDirn = thisDirn;
				}
			}
			AdjModuleRefPtr++;
		}
	}
	return targetModule;
}

AIMODULE *FarNPC_GetTargetAIModuleForRetreat(STRATEGYBLOCK *sbPtr)
{
	extern unsigned int PlayerSmell;
	
	AIMODULE **AdjModuleRefPtr;
	AIMODULE* targetModule = (AIMODULE *)0;
	unsigned int targetSmell = PlayerSmell + 1;	/* should be higher than any smell anywhere this frame */
	unsigned int targetNumAdj = 0;

	LOCALASSERT(sbPtr);	
	if(sbPtr->containingModule==NULL) return targetModule;	
	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

	/* check that there is a list of adjacent modules, and that it is not
	empty (ie points to zero) */
	if(AdjModuleRefPtr)	
	{
		while(*AdjModuleRefPtr != 0)
		{

			/* get the index */
			int AdjModuleIndex = (*AdjModuleRefPtr)->m_index;
			int AdjModuleSmell = PherPl_ReadBuf[AdjModuleIndex];
			int AdjModuleNumAdjacencies	= NumAdjacentModules((*AdjModuleRefPtr));

			/* if this adjacent module's smell value is lower than
			the current 'highest smell' record the new module as the
			target.  If they're equal, tie-break on number of adjacencies*/
			if( (!targetModule) ||
				(AdjModuleSmell < targetSmell)||
				((AdjModuleSmell == targetSmell) && (AdjModuleNumAdjacencies > targetNumAdj))
			  )
			{						
				targetSmell = PherPl_ReadBuf[AdjModuleIndex];
				targetModule = *AdjModuleRefPtr;
				targetNumAdj = NumAdjacentModules((*AdjModuleRefPtr));							
			}
			/* next adjacent module reference pointer */
			AdjModuleRefPtr++;
		}
	}
	return targetModule;
}

AIMODULE *FarNPC_GetTargetAIModuleForGlobalHunt(STRATEGYBLOCK *sbPtr)
{
	AIMODULE **AdjModuleRefPtr;
	int AdjModuleIndex;
	unsigned int highestSmell = 0;
	AIMODULE* targetModule = (AIMODULE *)0;

	LOCALASSERT(sbPtr);	
	if(sbPtr->containingModule==NULL) {
		return targetModule;
	}
	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

	/* check that there is a list of adjacent modules, and that it is not
	empty (ie points to zero) */
	if(AdjModuleRefPtr)	
	{
		while(*AdjModuleRefPtr != 0)
		{
			/* get the index */
			AdjModuleIndex = (*AdjModuleRefPtr)->m_index;

			/* if this adjacent module's smell value is higher than
			the current 'highest smell' record the new module as the
			target. */
			if(PherAls_ReadBuf[AdjModuleIndex] > highestSmell)
			{						
				highestSmell = PherAls_ReadBuf[AdjModuleIndex];
				targetModule = *AdjModuleRefPtr;							
			}
			/* next adjacent module reference pointer */
			AdjModuleRefPtr++;
		}
	}

	if (highestSmell<PherAls_ReadBuf[sbPtr->containingModule->m_aimodule->m_index]) {
		return (sbPtr->containingModule->m_aimodule);
	} else {
		return (targetModule);
	}
}

/* CDF 28/5/98 */
AIMODULE *FarNPC_GetTargetAIModuleForMarineRespond(STRATEGYBLOCK *sbPtr)
{
	AIMODULE **AdjModuleRefPtr;
	int AdjModuleIndex;
	unsigned int highestSmell = 0;
	AIMODULE* targetModule = (AIMODULE *)0;

	LOCALASSERT(sbPtr);	
	if(sbPtr->containingModule==NULL) return targetModule;
	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

	/* check that there is a list of adjacent modules, and that it is not
	empty (ie points to zero) */
	if(AdjModuleRefPtr)	
	{
		while(*AdjModuleRefPtr != 0)
		{
			/* get the index */
			AdjModuleIndex = (*AdjModuleRefPtr)->m_index;

			if (CheckAdjacencyValidity((*AdjModuleRefPtr), sbPtr->containingModule->m_aimodule,0)) {
				/* if this adjacent module's smell value is higher than
				the current 'highest smell' record the new module as the
				target. */
				if(PherMars_ReadBuf[AdjModuleIndex] > highestSmell)
				{						
					highestSmell = PherMars_ReadBuf[AdjModuleIndex];
					targetModule = *AdjModuleRefPtr;							
				}
			}
			/* next adjacent module reference pointer */
			AdjModuleRefPtr++;
		}
	}
	return targetModule;
}


/* Patrick 2/7/97:
This function turns the npc around (in y) a random amount: this is used to
turn the npc around if it reaches an impasse in the environment, and the
orientation is used to select a target module for wandering behaviour */
void FarNpc_FlipAround(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
 	/* get the dynamics block */
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);
			
	dynPtr->OrientEuler.EulerY += (1024 + FastRandom()%1024);
	dynPtr->OrientEuler.EulerY &= wrap360;
}
