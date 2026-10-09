/* Patrick 9/7/97 ----------------------------------------------------
  Source file for Facehugger AI behaviour functions....
  --------------------------------------------------------------------*/
#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "game_statistics.h"
#include "dynblock.h"
#include "dynamics.h"
#include "comp_shp.h"
#include "bh_types.h"
#include "bh_pred.h"
#include "bh_fhug.h"
#include "bh_debri.h"
#include "pfarlocs.h"
#include "pvisible.h"
#include "weapons.h"
#include "psnd.h"
#include "psndplat.h"
#include "targeting.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "showcmds.h"
#include "sfx.h"

#define HUGGER_STATE_PRINT	0

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;

extern ACTIVESOUNDSAMPLE ActiveSounds[];

/* prototypes for this file */
static void Execute_FHNS_Approach(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Attack(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Wait(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Avoidance(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Dying(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Float(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_Jumping(STRATEGYBLOCK *sbPtr);
static void Execute_FHNS_AboutToJump(STRATEGYBLOCK *sbPtr);

void Wake_Hugger(STRATEGYBLOCK *sbPtr);

static int HuggerShouldAttackPlayer(void);
static void SetHuggerAnimationSequence(STRATEGYBLOCK *sbPtr, HUGGER_SUBSEQUENCES seq, int length);
static void KillFaceHugger(STRATEGYBLOCK *sbPtr,DAMAGE_PROFILE *damage);

#if 0
static int InContactWithPlayer(DYNAMICSBLOCK *dynPtr);
#endif
static void JumpAtPlayer(STRATEGYBLOCK *sbPtr);

extern SECTION *GetHierarchyFromLibrary(const char *rif_name);

/* -------------------------------------------------------------------
   Initilaiser, damage, and visibility functions + behaviour shell
  --------------------------------------------------------------------*/
void InitFacehuggerBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_FACEHUGGER *toolsData; 
	int i;

	LOCALASSERT(bhdata);
	LOCALASSERT(sbPtr);
	toolsData = (TOOLS_DATA_FACEHUGGER *)bhdata; 

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network) 
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* make the assumption that the loader has initialised the strategy block sensibly... 
	so just set the shapeIndex from the tools data & copy the name id*/
	sbPtr->shapeIndex = toolsData->shapeIndex;
	for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];

	sbPtr->SBdptr=NULL;
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_NPC);
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
		dynPtr->Mass=10;
	}
	else
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* Initialise hugger's stats */
	{
		NPC_DATA *NpcData;

		NpcData=GetThisNpcData(I_NPC_FaceHugger);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
	}
	/* create, initialise and attach a facehugger data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(FACEHUGGER_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		FACEHUGGER_STATUS_BLOCK *facehuggerStatus = (FACEHUGGER_STATUS_BLOCK *)sbPtr->SBdataptr;

		NPC_InitMovementData(&(facehuggerStatus->moveData));
     	facehuggerStatus->health = FACEHUGGER_STARTING_HEALTH;
   		sbPtr->integrity = facehuggerStatus->health;
		facehuggerStatus->stateTimer = 0;
		facehuggerStatus->DoomTimer = 0;
		facehuggerStatus->CurveRadius = 0;
		facehuggerStatus->CurveLength = 0;
		facehuggerStatus->CurveTimeOut = 0;
		facehuggerStatus->jumping = 0;
	
		root_section=GetHierarchyFromLibrary("hnpchugger");
		Create_HModel(&facehuggerStatus->HModelController,root_section);
		InitHModelSequence(&facehuggerStatus->HModelController,0,0,ONE_FIXED);
		ProveHModel_Far(&facehuggerStatus->HModelController,sbPtr);
		
		if (toolsData->startInactive==0) {
	   		facehuggerStatus->nearBehaviourState = FHNS_Approach;
	   		SetHuggerAnimationSequence(sbPtr,HSS_Stand,ONE_FIXED);	
		} else {
	   		facehuggerStatus->nearBehaviourState = FHNS_Floating;
	   		SetHuggerAnimationSequence(sbPtr,HSS_Floats,(ONE_FIXED<<1));
			sbPtr->DynPtr->GravityOn=0;
		}

		facehuggerStatus->soundHandle = SOUND_NOACTIVEINDEX;
		facehuggerStatus->soundHandle2 = SOUND_NOACTIVEINDEX;
		
		for(i=0;i<SB_NAME_LENGTH;i++) facehuggerStatus->death_target_ID[i] = toolsData->death_target_ID[i];
		facehuggerStatus->death_target_sbptr=0;
		facehuggerStatus->death_target_request=toolsData->death_target_request;
	}
	else
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
		   	   	   	   
}

void FacehuggerBehaviour(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    

	LOCALASSERT(sbPtr);
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(facehuggerStatusPointer);	          		

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		if (facehuggerStatusPointer->soundHandle2 != SOUND_NOACTIVEINDEX) {
			Sound_Stop(facehuggerStatusPointer->soundHandle2);
		}
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	} 

	/* set velocity to zero */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (sbPtr->SBDamageBlock.IsOnFire) {

		CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);
		
		if (facehuggerStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&facehuggerStatusPointer->soundHandle2,127);
		} else {
			if (ActiveSounds[facehuggerStatusPointer->soundHandle2].soundIndex!=SID_FIRE) {
				Sound_Stop(facehuggerStatusPointer->soundHandle2);
			} else {
				Sound_Update3d(facehuggerStatusPointer->soundHandle2,&(sbPtr->DynPtr->Position));
			}
		}
	} else {
		Sound_Stop(facehuggerStatusPointer->soundHandle2);
	}

	/* No far behaviour for facehuggerss */
	if(sbPtr->SBdptr) 
	{
		if(sbPtr->maintainVisibility) LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
		switch(facehuggerStatusPointer->nearBehaviourState)
		{
			case(FHNS_Approach):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Approaching...\n");
				#endif
				Execute_FHNS_Approach(sbPtr);
				break;
			}
			case(FHNS_Attack):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Attacking...\n");
				#endif
				Execute_FHNS_Attack(sbPtr);
				break;
			}
			case(FHNS_Wait):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Waiting...\n");
				#endif
				Execute_FHNS_Wait(sbPtr);
				break;
			}
			case(FHNS_Avoidance):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Avoiding...\n");
				#endif
				Execute_FHNS_Avoidance(sbPtr);
				break;
			}
			case(FHNS_Dying):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Dying...\n");
				#endif
				Execute_FHNS_Dying(sbPtr);
				break;
			}
			case(FHNS_Floating):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Floating...\n");
				#endif
				Execute_FHNS_Float(sbPtr);
				break;
			}
			case(FHNS_Jumping):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger Jumping...\n");
				#endif
				Execute_FHNS_Jumping(sbPtr);
				break;
			}
			case(FHNS_AboutToJump):
			{
				#if HUGGER_STATE_PRINT
					PrintDebuggingText("Hugger AboutToJump...\n");
				#endif
				Execute_FHNS_AboutToJump(sbPtr);
				break;
			}
   			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}
	}

	if((facehuggerStatusPointer->nearBehaviourState == FHNS_Dying)&&(facehuggerStatusPointer->stateTimer <= 0)) {
		if (facehuggerStatusPointer->soundHandle2 != SOUND_NOACTIVEINDEX)	{
			Sound_Stop(facehuggerStatusPointer->soundHandle2);
		}
		DestroyAnyStrategyBlock(sbPtr);
	} else if (facehuggerStatusPointer->DoomTimer>(ONE_FIXED*FACEHUGGER_EXPIRY_TIME)) {
		/* Kill facehugger */
		sbPtr->SBDamageBlock.Health=0;
		if (facehuggerStatusPointer->nearBehaviourState != FHNS_Dying) {
			KillFaceHugger(sbPtr,NULL);
		}
	}

}

void MakeFacehuggerNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    

	LOCALASSERT(sbPtr);
	dynPtr = sbPtr->DynPtr;
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(facehuggerStatusPointer);	          		
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
	if(dPtr==NULL) return; /* cannot create displayblock, so leave far */

	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					

	/* need to initialise positional information in the new display block */ 
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* facehugger data block init */
	facehuggerStatusPointer->stateTimer = 0;
	facehuggerStatusPointer->CurveRadius = 0;
	facehuggerStatusPointer->CurveLength = 0;
	facehuggerStatusPointer->CurveTimeOut = 0;
	facehuggerStatusPointer->jumping = 0;
   	
   	/* state and sequence init */
	//dPtr->ShapeAnimControlBlock = &facehuggerStatusPointer->ShpAnimCtrl;
	dPtr->HModelControlBlock=&facehuggerStatusPointer->HModelController;
   	if ((facehuggerStatusPointer->nearBehaviourState!=FHNS_Floating)
   		&&(facehuggerStatusPointer->nearBehaviourState!=FHNS_Dying)) {
   		if(HuggerShouldAttackPlayer())
		{
			NPC_InitMovementData(&(facehuggerStatusPointer->moveData));
   			facehuggerStatusPointer->nearBehaviourState = FHNS_Approach;
   			SetHuggerAnimationSequence(sbPtr,HSS_Run,(ONE_FIXED*2)/3);	
		}
		else
   		{
			NPC_InitMovementData(&(facehuggerStatusPointer->moveData));
   			facehuggerStatusPointer->nearBehaviourState = FHNS_Wait;
   			SetHuggerAnimationSequence(sbPtr,HSS_Stand,ONE_FIXED);	
		}
	}
	ProveHModel(dPtr->HModelControlBlock,dPtr);

}

void MakeFacehuggerFar(STRATEGYBLOCK *sbPtr)
{
	int i;	
	LOCALASSERT(sbPtr->SBdptr != NULL);
	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;
	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;	
}

void FacehuggerIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    

	LOCALASSERT(sbPtr);
	//Sound_Play(SID_ACID_SPRAY,"dp",&(sbPtr->DynPtr->Position),(FastRandom() & 255) - 128);

 	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(facehuggerStatusPointer);	          		

	#if 0
	/* reduce facehugger health */
	if(facehuggerStatusPointer->health>0) facehuggerStatusPointer->health -= damage; 	
	#endif											 

	if (facehuggerStatusPointer->nearBehaviourState==FHNS_Floating) {
		Wake_Hugger(sbPtr);
	}

	/* check if we've been killed */
	if ( (sbPtr->SBDamageBlock.Health <= 0)&&(facehuggerStatusPointer->nearBehaviourState!=FHNS_Dying) )
	{
		CurrentGameStats_CreatureKilled(sbPtr,NULL);
		KillFaceHugger(sbPtr,damage);
		return;
	}

}

static void KillFaceHugger(STRATEGYBLOCK *sbPtr,DAMAGE_PROFILE *damage)
{
	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	
	LOCALASSERT(sbPtr);
	fhugStatusPointer=(FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);

	fhugStatusPointer->nearBehaviourState=FHNS_Dying;
	fhugStatusPointer->stateTimer=FACEHUGGER_DYINGTIME<<1;

	/* Stop any hugger sound if playing */
	if (fhugStatusPointer->soundHandle != SOUND_NOACTIVEINDEX)
	{
		Sound_Stop(fhugStatusPointer->soundHandle);			
	}

	//Sound_Play(SID_BUGDIE3,"d",&(dynPtr->Position));

	#if 1
	{
		PLAYER_STATUS *playerStatusPointer= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		
		if (playerStatusPointer->MyFaceHugger==sbPtr) {
			/* You cheater! */
			playerStatusPointer->MyFaceHugger=NULL;
		}
	}
	#endif

	/*If face hugger has a death target ,send a request*/
	if(fhugStatusPointer->death_target_sbptr)
	{
		RequestState(fhugStatusPointer->death_target_sbptr,fhugStatusPointer->death_target_request, 0);
	} 
	
	/* More restrained death. */
	{
		/* switch sequence */
		if (damage) {
			if ( (damage->Impact==0)
			   &&(damage->Cutting==0)	
			   &&(damage->Penetrative==0)	
			   &&(damage->Fire!=0)
			   &&(damage->Electrical==0)
			   &&(damage->Acid==0))
			{
				SetHuggerAnimationSequence(sbPtr,HSS_DieOnFire,FACEHUGGER_DYINGTIME>>3);
			} else {
				SetHuggerAnimationSequence(sbPtr,HSS_Dies,FACEHUGGER_DYINGTIME>>3);
			}
		} else {
			SetHuggerAnimationSequence(sbPtr,HSS_Dies,FACEHUGGER_DYINGTIME>>3);
		}
		fhugStatusPointer->HModelController.Looped=0;
		fhugStatusPointer->HModelController.LoopAfterTweening=0;
		/* switch state */
		fhugStatusPointer->nearBehaviourState=FHNS_Dying;
		fhugStatusPointer->stateTimer=FACEHUGGER_DYINGTIME;

		/* stop motion */
		LOCALASSERT(sbPtr->DynPtr);
		dynPtr->Friction	= 400000;
		dynPtr->LinImpulse.vx=sbPtr->DynPtr->LinVelocity.vx;
		dynPtr->LinImpulse.vy=sbPtr->DynPtr->LinVelocity.vy;
		dynPtr->LinImpulse.vz=sbPtr->DynPtr->LinVelocity.vz;
		dynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
		/* Okay... */

	}
}


/* -------------------------------------------------------------------
   Behaviour state functions
  --------------------------------------------------------------------*/

static void Execute_FHNS_Approach(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH targetPos;
	//int approachingAirDuct = 0;

	LOCALASSERT(sbPtr);
	fhugStatusPointer=(FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);

	/* Stop any hugger attack sound if playing */
	if (fhugStatusPointer->soundHandle != SOUND_NOACTIVEINDEX &&
			ActiveSounds[fhugStatusPointer->soundHandle].soundIndex != SID_FHUG_MOVE)
	{
		Sound_Stop(fhugStatusPointer->soundHandle);			
	}

	/* Start the hugger movement sound if needed */
	if (fhugStatusPointer->soundHandle == SOUND_NOACTIVEINDEX)
	{

		Sound_Play(SID_FHUG_MOVE,"ed",&fhugStatusPointer->soundHandle,&dynPtr->Position);
	}

	/* do climb on walls, etc */
	dynPtr->UseStandardGravity=1;
	/* If you think I'm going to let facehuggers climb on walls for *
	 * ONE SECOND, you are INSANE, Jack!!! */
 	
 	/* target acquisition ? */
	{
		extern DISPLAYBLOCK *Player;
		if (sbPtr->SBDamageBlock.IsOnFire==0) {
			targetPos=Player->ObStrategyBlock->DynPtr->Position;
		} else {
			targetPos=Player->ObStrategyBlock->DynPtr->Position;

			targetPos.vx+=((FastRandom()&8191)-4096);
			targetPos.vy+=((FastRandom()&8191)-4096);
			targetPos.vz+=((FastRandom()&8191)-4096);
		}
	}

	/* translate target into hugger local space */
	{
		MATRIXCH toLocalSpaceMatrix = dynPtr->OrientMat;
		TransposeMatrixCH(&toLocalSpaceMatrix);
		
		targetPos.vx -= dynPtr->Position.vx;
		targetPos.vy -= dynPtr->Position.vy;
		targetPos.vz -= dynPtr->Position.vz;
		RotateVector(&targetPos,&toLocalSpaceMatrix);
	}

	/* Fix vy. */
	targetPos.vy=0;

	/* tracking movement */
	{
		int distanceToTarget = Magnitude(&targetPos);
	    if (dynPtr->IsInContactWithFloor)
        {
			int offset;

			if (fhugStatusPointer->CurveTimeOut<=0)
			{
				fhugStatusPointer->CurveLength = distanceToTarget;
				fhugStatusPointer->CurveRadius = ((FastRandom()&16383)-8192)*2;
				fhugStatusPointer->CurveTimeOut= ONE_FIXED*3;
			} else {
				fhugStatusPointer->CurveTimeOut-=NormalFrameTime;
			}

			offset = 
				MUL_FIXED
				(
					fhugStatusPointer->CurveRadius,
					GetCos((1024*(distanceToTarget)/fhugStatusPointer->CurveLength)&4095)
				);

			dynPtr->LinVelocity.vx = 
				WideMulNarrowDiv
				(
					FACEHUGGER_NEAR_SPEED,
					targetPos.vx,
					distanceToTarget
				)
				-WideMulNarrowDiv
				(
					offset,
					targetPos.vz,
					distanceToTarget
				);
				
				
			dynPtr->LinVelocity.vz = 
				WideMulNarrowDiv
				(
					FACEHUGGER_NEAR_SPEED,
					targetPos.vz,
					distanceToTarget
				)+
				WideMulNarrowDiv
				(
					offset,
					targetPos.vx,
					distanceToTarget
				);

		 	RotateVector(&dynPtr->LinVelocity,&dynPtr->OrientMat);
			/* align to velocity */
			
			NPCOrientateToVector(sbPtr,&dynPtr->LinVelocity,NPC_TURNRATE,NULL);
							
			/* within test for in contact with floor: test if
			jump flag is set- if so switch anim back to run... */
			if(fhugStatusPointer->jumping==1)
			{
				fhugStatusPointer->jumping = 0;
				SetHuggerAnimationSequence(sbPtr,HSS_Run,(ONE_FIXED*2)/3);
			}
		}
	}		

	/* check for state changes: 
	firstly, are we in contact with the player? */
	#if 0
	if(InContactWithPlayer(dynPtr)&&HuggerShouldAttackPlayer())
	{
		fhugStatusPointer->nearBehaviourState = FHNS_Attack;
		fhugStatusPointer->stateTimer = FACEHUGGER_NEARATTACKTIME;
		SetHuggerAnimationSequence(sbPtr,HSS_Attack,ONE_FIXED);		
		dynPtr->DynamicsType = DYN_TYPE_NO_COLLISIONS; 	/* turn off collisons */	
		dynPtr->GravityOn = 0;							/* turn off gravity */
		sbPtr->maintainVisibility = 0;					/* turn off visibility support- be carefull! */

		/* Attach to player! */
		{
			PLAYER_STATUS *playerStatusPointer= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
			
			playerStatusPointer->MyFaceHugger=sbPtr;
		}
		return;
	}
	#endif

	/* is player visible?: if not, go to wait */
	if(!HuggerShouldAttackPlayer())
	{
		fhugStatusPointer->nearBehaviourState = FHNS_Wait;
		fhugStatusPointer->stateTimer = 0;
		return;
	}

	#if 1
	/* should we jump at the player? */
	if (sbPtr->SBDamageBlock.IsOnFire==0) {
		int distanceToPlayer = VectorDistance(&(dynPtr->Position),&(Player->ObStrategyBlock->DynPtr->Position));
		if((distanceToPlayer<=FACEHUGGER_JUMPDISTANCE)&&(dynPtr->IsInContactWithFloor))
		{
			#if 0
			JumpAtPlayer(sbPtr);
			#else
			fhugStatusPointer->nearBehaviourState = FHNS_AboutToJump;
			fhugStatusPointer->stateTimer = 0;
			#endif
			return;
		}
	}
	#endif

	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(fhugStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.environment)
		{
			/* go to avoidance */
			NPC_InitMovementData(&(fhugStatusPointer->moveData));
			NPCGetAvoidanceDirection(sbPtr, &(fhugStatusPointer->moveData.avoidanceDirn),&obstruction);						
			fhugStatusPointer->nearBehaviourState = FHNS_Avoidance;  		
			fhugStatusPointer->stateTimer = NPC_AVOIDTIME;
			/* no sequence change required */
			return;
		}
		if(obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);

			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_ALIEN_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);

		}
	}

	{
		VECTORCH velocityDirection = dynPtr->LinVelocity;
		Normalise(&velocityDirection);

		if(NPC_CannotReachTarget(&(fhugStatusPointer->moveData), &targetPos, &velocityDirection))
		{
			/* go to avoidance */
			NPC_OBSTRUCTIONREPORT obstruction = {1,0,0};
			NPC_InitMovementData(&(fhugStatusPointer->moveData));
			NPCGetAvoidanceDirection(sbPtr, &(fhugStatusPointer->moveData.avoidanceDirn),&obstruction);						
			fhugStatusPointer->nearBehaviourState = FHNS_Avoidance;  		
			fhugStatusPointer->stateTimer = NPC_AVOIDTIME;
			/* no sequence change required */
			return;
		}
	}
}

void PlotFaceHugger(STRATEGYBLOCK *sbPtr) {

	extern void RenderThisDisplayblock(DISPLAYBLOCK *dbPtr);
	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];

	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
	DYNAMICSBLOCK *dynPtr;
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    
	
	LOCALASSERT(sbPtr);
	dynPtr=sbPtr->DynPtr;
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(facehuggerStatusPointer);	
	LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr);	


	{
		VECTORCH x,y,z;

		x.vx = -VDBPtr->VDB_Mat.mat11;
		x.vy = -VDBPtr->VDB_Mat.mat21;
		x.vz = -VDBPtr->VDB_Mat.mat31;
		y.vx = -VDBPtr->VDB_Mat.mat13;
		y.vy = -VDBPtr->VDB_Mat.mat23;
		y.vz = -VDBPtr->VDB_Mat.mat33;
		z.vx = -VDBPtr->VDB_Mat.mat12;
		z.vy = -VDBPtr->VDB_Mat.mat22;
		z.vz = -VDBPtr->VDB_Mat.mat32;

		Normalise(&x);
		Normalise(&y);
		Normalise(&z);

		dynPtr->OrientMat.mat11 = x.vx;
		dynPtr->OrientMat.mat12 = x.vy;
		dynPtr->OrientMat.mat13 = x.vz;
		dynPtr->OrientMat.mat21 = y.vx;
		dynPtr->OrientMat.mat22 = y.vy;
		dynPtr->OrientMat.mat23 = y.vz;
		dynPtr->OrientMat.mat31 = z.vx;
		dynPtr->OrientMat.mat32 = z.vy;
		dynPtr->OrientMat.mat33 = z.vz;
	}

	/* set position */
	dynPtr->Position.vx = 0;
	dynPtr->Position.vz = FACEHUGGER_ATTACKZOFFSET/4;
	dynPtr->Position.vy = FACEHUGGER_ATTACKYOFFSET;
	{
		MATRIXCH myMat = VDBPtr->VDB_Mat;
		TransposeMatrixCH(&myMat);
		RotateVector(&(dynPtr->Position), &(myMat));	
	}

	dynPtr->Position.vx += VDBPtr->VDB_World.vx;
	dynPtr->Position.vy += VDBPtr->VDB_World.vy;
	dynPtr->Position.vz += VDBPtr->VDB_World.vz;
	sbPtr->SBdptr->ObFlags&=~ObFlag_NotVis;
	
	sbPtr->SBdptr->ObWorld = dynPtr->Position;
	sbPtr->SBdptr->ObMat = dynPtr->OrientMat;

	ProveHModel(sbPtr->SBdptr->HModelControlBlock,sbPtr->SBdptr);
	RenderThisDisplayblock(sbPtr->SBdptr);

	sbPtr->SBdptr->ObFlags|=ObFlag_NotVis;

}

static void Execute_FHNS_Attack(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr;
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    
	
	LOCALASSERT(sbPtr);
	dynPtr=sbPtr->DynPtr;
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(facehuggerStatusPointer);	
	LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr);	

	/* Play the hugger attack loop sound */
	if (facehuggerStatusPointer->soundHandle == SOUND_NOACTIVEINDEX)
	{
		Sound_Play(SID_FHUG_ATTACKLOOP,"edl",&facehuggerStatusPointer->soundHandle,&dynPtr->Position);
	}

	textprint("face hugger attack \n");

	/* Make not vis */	
	
	sbPtr->SBdptr->ObFlags|=ObFlag_NotVis;

	/* do damage */
	facehuggerStatusPointer->DoomTimer += NormalFrameTime;
	facehuggerStatusPointer->stateTimer -= NormalFrameTime;
	if(facehuggerStatusPointer->stateTimer <= 0)
	{
		facehuggerStatusPointer->stateTimer = FACEHUGGER_NEARATTACKTIME;
		CauseDamageToObject(Player->ObStrategyBlock, &TemplateAmmo[AMMO_FACEHUGGER].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		/* FRI? */
	}

}

static void Execute_FHNS_Wait(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    
	
	LOCALASSERT(sbPtr);
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(facehuggerStatusPointer);	
	
	if(HuggerShouldAttackPlayer())
	{
		NPC_InitMovementData(&(facehuggerStatusPointer->moveData));
   		facehuggerStatusPointer->nearBehaviourState = FHNS_Approach;
		facehuggerStatusPointer->stateTimer = 0;
		facehuggerStatusPointer->CurveTimeOut = 0;
		SetHuggerAnimationSequence(sbPtr,HSS_Run,(ONE_FIXED*2)/3);
	}
}

static void Execute_FHNS_Avoidance(STRATEGYBLOCK *sbPtr)
{
	int terminateState = 0;
	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	fhugStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);
	
	/* Stop any hugger attack sound if playing */
	if (fhugStatusPointer->soundHandle != SOUND_NOACTIVEINDEX &&
			ActiveSounds[fhugStatusPointer->soundHandle].soundIndex != SID_FHUG_MOVE)
	{
		Sound_Stop(fhugStatusPointer->soundHandle);			
	}

	/* Start the hugger movement sound if needed */
	if (fhugStatusPointer->soundHandle == SOUND_NOACTIVEINDEX)
	{
		Sound_Play(SID_FHUG_MOVE,"ed",&fhugStatusPointer->soundHandle,&dynPtr->Position);
	}

	/* set velocity */
	LOCALASSERT((fhugStatusPointer->moveData.avoidanceDirn.vx!=0)||
				(fhugStatusPointer->moveData.avoidanceDirn.vy!=0)||
				(fhugStatusPointer->moveData.avoidanceDirn.vz!=0));
	NPCSetVelocity(sbPtr, &(fhugStatusPointer->moveData.avoidanceDirn), (FACEHUGGER_NEAR_SPEED));
	
	/* next, decrement state timer */
	fhugStatusPointer->stateTimer -= NormalFrameTime;
	if(fhugStatusPointer->stateTimer <= 0) terminateState = 1;

	/* and check for an impeding collision */
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(fhugStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.anySingleObstruction)
		{
			terminateState = 1;
		}
	}
	
	if(terminateState)
	{
		if(HuggerShouldAttackPlayer())
		{
			/* switch to approach */
			NPC_InitMovementData(&(fhugStatusPointer->moveData));
			fhugStatusPointer->nearBehaviourState = FHNS_Approach;  		
			fhugStatusPointer->stateTimer = 0;
			/* no sequence change required */
		}
		else
		{
			/* switch to wait */
			NPC_InitMovementData(&(fhugStatusPointer->moveData));
			fhugStatusPointer->nearBehaviourState = FHNS_Wait;  		
			fhugStatusPointer->stateTimer = 0;
			/* no sequence change required */
		}
	}
}

/* -------------------------------------------------------------------
FaceHugger behaviour support functions
  --------------------------------------------------------------------*/
static void SetHuggerAnimationSequence(STRATEGYBLOCK *sbPtr, HUGGER_SUBSEQUENCES seq, int length)
{

	FACEHUGGER_STATUS_BLOCK *fhugStatus=(FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);

	InitHModelTweening(&fhugStatus->HModelController,(ONE_FIXED>>3),(int)HMSQT_Hugger,(int)seq,length,1);
	
	if (seq==HSS_Jump) fhugStatus->HModelController.LoopAfterTweening=0;

}

static int HuggerShouldAttackPlayer(void)
{
	/* test for player being cloaked */
	{
		PLAYER_STATUS *playerStatusPointer= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPointer);

		if((playerStatusPointer->cloakOn==1)&&(playerStatusPointer->cloakPositionGivenAway==0)) {
		
			return 1;
			/* Was '0'. */
		}

		if (playerStatusPointer->MyFaceHugger!=NULL) return(0);
	}

	/* test for player being an alien */
	if(AvP.PlayerType==I_Alien) return 0;

	return 1;
}

#if 0
static int InContactWithPlayer(DYNAMICSBLOCK *dynPtr)
{
	struct collisionreport *nextReport;

	LOCALASSERT(dynPtr);
	nextReport = dynPtr->CollisionReportPtr;
	
	/* walk the collision report list, looking for collisions against the player */
	while(nextReport)
	{
		if(nextReport->ObstacleSBPtr == Player->ObStrategyBlock) return 1;
		nextReport = nextReport->NextCollisionReportPtr;
	}
	
	return 0;	
}
#endif

static void JumpAtPlayer(STRATEGYBLOCK *sbPtr)
{

	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer=(FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);
	/* Set up jump! */

    if (sbPtr->DynPtr->IsInContactWithFloor==0) {
		/* Jump Not! */
		return;
	}

	/* set animation */
	SetHuggerAnimationSequence(sbPtr, HSS_Jump, (ONE_FIXED));
	fhugStatusPointer->jumping = 1;
	fhugStatusPointer->CurveTimeOut = 0;
	
	fhugStatusPointer->HModelController.Looped=0;
	fhugStatusPointer->HModelController.LoopAfterTweening=0;

	fhugStatusPointer->nearBehaviourState = FHNS_Jumping;
	fhugStatusPointer->stateTimer=0;

}

static void FHugApplyPounceImpulse(STRATEGYBLOCK *sbPtr) {

	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH pounceVector,targetPoint;
	int dist,speed;

	LOCALASSERT(sbPtr);
	fhugStatusPointer=(FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);

	GetTargetingPointOfObject(Player,&targetPoint);
	
	dist = VectorDistance(&(dynPtr->Position),&targetPoint);

	/* Apply a correction based on range. */
	targetPoint.vy-=(dist>>3);

	pounceVector.vx=targetPoint.vx-dynPtr->Position.vx;
	pounceVector.vy=targetPoint.vy-dynPtr->Position.vy;
	pounceVector.vz=targetPoint.vz-dynPtr->Position.vz;

	Normalise(&pounceVector);
	/* Must jump at least a little bit upwards. */
	if (pounceVector.vy>-10000) {
		pounceVector.vy=-10000;
	}	

	speed=FACEHUGGER_JUMP_SPEED;

	pounceVector.vx=MUL_FIXED(speed,pounceVector.vx);
	pounceVector.vy=MUL_FIXED(speed,pounceVector.vy);
	pounceVector.vz=MUL_FIXED(speed,pounceVector.vz);

	sbPtr->DynPtr->LinImpulse=pounceVector;

}

static void Execute_FHNS_Jumping(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	fhugStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);

	/* don't climb on walls, etc */
	dynPtr->UseStandardGravity=1;

	/* Firstly, are we actually pouncing yet? */
	/* NearStateTimer is a status flag. */

	if (fhugStatusPointer->stateTimer!=2) {
		/* Still tweening? */

		if (fhugStatusPointer->HModelController.keyframe_flags) {
			/* We have the flag. */
			fhugStatusPointer->HModelController.Playing=0;
			fhugStatusPointer->stateTimer=1;
		}

		if (fhugStatusPointer->stateTimer==1) {
			/* We've finished!  Are we facing right? */
			VECTORCH orientationDirn;
			int i;

			orientationDirn.vx = Player->ObWorld.vx - dynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = Player->ObWorld.vz - dynPtr->Position.vz;
			i = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE<<2,NULL);
			
			if (i==0) {
				/* Still not right!  Wait for proper facing. */
				fhugStatusPointer->HModelController.Playing=0;
				//CheckPounceIntegrity(sbPtr);
				return;
			} else {
				/* Okay, pounce! */
	
				FHugApplyPounceImpulse(sbPtr);

				fhugStatusPointer->HModelController.Playing=1;
				fhugStatusPointer->stateTimer=2;
			}
		} else {
			/* Yup, still tweening.  Check state validity? */
			//CheckPounceIntegrity(sbPtr);
		}
	} else {
		/* We must be in the jump.  Can't break out of this until we hit something. */
		COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
		
		while (reportPtr)
		{
			STRATEGYBLOCK *hitSbPtr = reportPtr->ObstacleSBPtr;
			/* You know what?  Just did! */
	
			fhugStatusPointer->nearBehaviourState=FHNS_Approach;
			fhugStatusPointer->stateTimer = 0;
			fhugStatusPointer->CurveTimeOut = 0;

			if (hitSbPtr) {
				if ((hitSbPtr->SBdptr==Player)&&(sbPtr->SBDamageBlock.IsOnFire==0)) {
					/* Got him, My Precious, we've Got Him! */
	
					Sound_Play(SID_ED_FACEHUGGERSLAP,"h");
					
					/* Test for attach, or merely bite? */
					fhugStatusPointer->nearBehaviourState = FHNS_Attack;
					fhugStatusPointer->stateTimer = FACEHUGGER_NEARATTACKTIME;
					SetHuggerAnimationSequence(sbPtr,HSS_Attack,ONE_FIXED);		
					dynPtr->DynamicsType = DYN_TYPE_NO_COLLISIONS; 	/* turn off collisons */	
					dynPtr->GravityOn = 0;							/* turn off gravity */
					sbPtr->maintainVisibility = 0;					/* turn off visibility support- be carefull! */
			
					/* Attach to player! */
					{
						PLAYER_STATUS *playerStatusPointer= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
						
						playerStatusPointer->MyFaceHugger=sbPtr;
					}
					return;

				}
			}

			reportPtr = reportPtr->NextCollisionReportPtr;
		}														
	
	}

}

static void Execute_FHNS_Dying(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *huggerStatusPointer;    	
	DISPLAYBLOCK *dispPtr;

	LOCALASSERT(sbPtr);
	huggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(huggerStatusPointer);

	huggerStatusPointer->stateTimer -= NormalFrameTime;

	dispPtr=sbPtr->SBdptr;

	if (dispPtr)
	{
		dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
		dispPtr->ObFlags2 = huggerStatusPointer->stateTimer/2;
	}
}

static void Execute_FHNS_Float(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(facehuggerStatusPointer);
	LOCALASSERT(dynPtr);

	dynPtr->LinVelocity.vx=0;	
	dynPtr->LinVelocity.vy=0;	
	dynPtr->LinVelocity.vz=0;	

	dynPtr->LinImpulse.vx=0;	
	dynPtr->LinImpulse.vy=0;	
	dynPtr->LinImpulse.vz=0;	

	dynPtr->GravityOn=0;

	/* Just float there... */
}

void Wake_Hugger(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(facehuggerStatusPointer);
	LOCALASSERT(dynPtr);

	if (facehuggerStatusPointer->nearBehaviourState==FHNS_Floating) {
		facehuggerStatusPointer->nearBehaviourState = FHNS_Approach;
		dynPtr->GravityOn=1;
	 	SetHuggerAnimationSequence(sbPtr,HSS_Run,(ONE_FIXED*2)/3);
	}
}

static void Execute_FHNS_AboutToJump(STRATEGYBLOCK *sbPtr)
{
	FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;
	DYNAMICSBLOCK *dynPtr;

	/* Should still be playing Walk. */

	LOCALASSERT(sbPtr);
	fhugStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr=sbPtr->DynPtr;
	LOCALASSERT(fhugStatusPointer);
	LOCALASSERT(dynPtr);

	/* don't climb on walls, etc */
	dynPtr->UseStandardGravity=1;

	{
		/* Orientate to target. */
		VECTORCH orientationDirn;
		int i;

		orientationDirn.vx = Player->ObWorld.vx - dynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = Player->ObWorld.vz - dynPtr->Position.vz;
		i = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		if (i==0) {
			/* Still not right!  Wait for proper facing. */
			return;
		} else {
			/* Okay, pounce! */
			int distanceToPlayer = VectorDistance(&(dynPtr->Position),&(Player->ObStrategyBlock->DynPtr->Position));
			if((distanceToPlayer<=FACEHUGGER_JUMPDISTANCE)&&(dynPtr->IsInContactWithFloor)
				&&(sbPtr->SBDamageBlock.IsOnFire==0)) {
				JumpAtPlayer(sbPtr);
				return;
			} else {
				/* Return to approach. */
				NPC_InitMovementData(&(fhugStatusPointer->moveData));
				fhugStatusPointer->nearBehaviourState = FHNS_Approach;  		
				fhugStatusPointer->stateTimer = 0;
				/* no sequence change required */
			}
		}
	}
}






/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct face_hugger_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	signed int health;
	FACEHUGGER_NEAR_BHSTATE nearBehaviourState;	
	int stateTimer;
	int DoomTimer;
	int CurveRadius;
	int CurveLength;
	int CurveTimeOut;
  	BOOL jumping;	
  

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}FACE_HUGGER_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV huggerStatusPointer

void LoadStrategy_FaceHugger(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	FACEHUGGER_STATUS_BLOCK* huggerStatusPointer;
	FACE_HUGGER_SAVE_BLOCK* block = (FACE_HUGGER_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourFaceHugger) return;

	huggerStatusPointer = (FACEHUGGER_STATUS_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_LOAD(health)
	COPYELEMENT_LOAD(nearBehaviourState)
	COPYELEMENT_LOAD(stateTimer)
	COPYELEMENT_LOAD(DoomTimer)
	COPYELEMENT_LOAD(CurveRadius)
	COPYELEMENT_LOAD(CurveLength)
	COPYELEMENT_LOAD(CurveTimeOut)
	COPYELEMENT_LOAD(jumping)

	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	
	//load the hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&huggerStatusPointer->HModelController);
		}
	}
	Load_SoundState(&huggerStatusPointer->soundHandle);
	Load_SoundState(&huggerStatusPointer->soundHandle2);
}

void SaveStrategy_FaceHugger(STRATEGYBLOCK* sbPtr)
{
	FACE_HUGGER_SAVE_BLOCK* block;
	FACEHUGGER_STATUS_BLOCK* huggerStatusPointer;

	huggerStatusPointer = (FACEHUGGER_STATUS_BLOCK*)sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	COPYELEMENT_SAVE(health)
	COPYELEMENT_SAVE(nearBehaviourState)
	COPYELEMENT_SAVE(stateTimer)
	COPYELEMENT_SAVE(DoomTimer)
	COPYELEMENT_SAVE(CurveRadius)
	COPYELEMENT_SAVE(CurveLength)
	COPYELEMENT_SAVE(CurveTimeOut)
	COPYELEMENT_SAVE(jumping)

	//save strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&huggerStatusPointer->HModelController);

	Save_SoundState(&huggerStatusPointer->soundHandle);
	Save_SoundState(&huggerStatusPointer->soundHandle2);

}
