/*------------------------------Patrick 14/3/97-----------------------------------
  Source for Platform lift behaviour functions
  --------------------------------------------------------------------------------*/
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "triggers.h"
#include "dynblock.h"
#include "dynamics.h"
#include "showcmds.h"
#include "weapons.h"

#define UseLocalAssert Yes

#include "ourasert.h"
#include "bh_plift.h"

/* for win95 net game support */
#include "pldnet.h"
#include "pldghost.h"

/* prototypes for this file */
static int SquashingSomething(DYNAMICSBLOCK *dynPtr);
static void PushPassengersUpwards(DYNAMICSBLOCK *dynPtr);
static int ActivatedByPlayer(DYNAMICSBLOCK *dynPtr);
static int PlayerIsNearOtherTerminal(STRATEGYBLOCK *sbPtr);
static int NetPlayerAtOtherTerminal(STRATEGYBLOCK *sbPtr);
static int PLiftIsNearerUpThanDown(STRATEGYBLOCK *sbPtr);

/* external globals used in this file */
extern int NormalFrameTime;

/*------------------------------Patrick 15/3/97-----------------------------------
  Behaviour function for platform lift
  --------------------------------------------------------------------------------*/
void PlatformLiftBehaviour(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	switch(platformliftdata->state)
	{
		case(PLBS_AtRest):
		{			
			if(AvP.Network==I_Host)
			{
				/* multiplayer host */
				if(NetPlayerAtOtherTerminal(sbPtr))
				{
					if(platformliftdata->Enabled) ActivatePlatformLift(sbPtr);
				}
				else if(sbPtr->SBdptr)
				{			
					/* check for activation by this player*/
					if(ActivatedByPlayer(dynPtr))
					{
						if (platformliftdata->Enabled) ActivatePlatformLift(sbPtr);
					}
			 	}
			}
			else if(AvP.Network==I_No_Network)
			{
				/* single player */
				if(sbPtr->SBdptr)
				{			
					/* check for activation */
					if(ActivatedByPlayer(dynPtr) || PlayerIsNearOtherTerminal(sbPtr))
					{
						if (platformliftdata->Enabled) ActivatePlatformLift(sbPtr);
					}
			 	}
			}
			else
			{
				LOCALASSERT(AvP.Network==I_Peer);
				if(ActivatedByPlayer(dynPtr)) AddNetMsg_RequestPlatformLiftActivate(sbPtr);
			}						
			//textprint("Platform state: at rest\n");
			break;
		}
		case(PLBS_Activating):
		{
			if(AvP.Network!=I_Peer)
			{
				if(platformliftdata->activationDelayTimer>0)
				{
					platformliftdata->activationDelayTimer -= NormalFrameTime;
				}
								
				
				if(platformliftdata->activationDelayTimer<=0)
				{
					//play start sound if the lift has any
					if(platformliftdata->start_sound)
					{
						Start_Track_Sound(platformliftdata->start_sound,&sbPtr->DynPtr->Position);
					}
					
					
					platformliftdata->activationDelayTimer = 0;
					if(dynPtr->Position.vy > ((platformliftdata->upHeight+platformliftdata->downHeight)/2))
					{
						SendPlatformLiftUp(sbPtr);
					}
					else SendPlatformLiftDown(sbPtr);
				}
			}
			//textprint("Platform state: activating\n");
			//textprint("Platform timer: %d\n",platformliftdata->activationDelayTimer);
			break;	
		}
		case(PLBS_GoingUp):
		{
			if(dynPtr->Position.vy <= platformliftdata->upHeight)
			{
				/* finished */
				StopPlatformLift(sbPtr);
				if (platformliftdata->OneUse)
					platformliftdata->Enabled = No;
			}
			else
			{
				sbPtr->DynPtr->Displacement.vy = -MUL_FIXED(PLATFORMLIFT_SPEED,NormalFrameTime);
				{
					int d = platformliftdata->upHeight - sbPtr->DynPtr->Position.vy;

				 	if (d>sbPtr->DynPtr->Displacement.vy)
				 	{
				 		sbPtr->DynPtr->Displacement.vy=d;
					}
				}
			}
			
			PushPassengersUpwards(dynPtr);
			//textprint("Platform state: going up\n");
			break; 
		}
		case(PLBS_GoingDown):
		{
			if(dynPtr->Position.vy >= platformliftdata->downHeight)
			{
				/* finished */
				StopPlatformLift(sbPtr);
				if (platformliftdata->OneUse)
					platformliftdata->Enabled = No;
			}
			else 
			{
				sbPtr->DynPtr->Displacement.vy = MUL_FIXED(PLATFORMLIFT_SPEED,NormalFrameTime);
				{
					int d = platformliftdata->downHeight - sbPtr->DynPtr->Position.vy;

				 	if (d<sbPtr->DynPtr->Displacement.vy)
				 	{
				 		sbPtr->DynPtr->Displacement.vy=d;
					}
				}
			
				if(AvP.Network!=I_Peer)
				{
					if(SquashingSomething(dynPtr))
					{
//						sbPtr->DynPtr->Displacement.vy = 0;					
						//	SendPlatformLiftUp(sbPtr);
					}
				}
			}

			//textprint("Platform state: going down\n");
			break; 
		}
		default:
		{
			LOCALASSERT(1==0);
		}
	}
	
	//update sound
	if(platformliftdata->state==PLBS_GoingUp || platformliftdata->state==PLBS_GoingDown)
	{
		BOOL mainSoundShouldBePlaying=TRUE;
		
		//the main sound should be started if there is no start sound , or the start sound has
		//almost finished
		if(platformliftdata->start_sound && platformliftdata->start_sound->playing)
		{
			Update_Track_Sound(platformliftdata->start_sound,&dynPtr->Position);

			if(platformliftdata->start_sound->time_left>NormalFrameTime)
			{
				mainSoundShouldBePlaying=FALSE;
			}

		}
		
		if(platformliftdata->sound)
		{
			if(platformliftdata->sound->playing)
			{
				Update_Track_Sound(platformliftdata->sound,&dynPtr->Position);
			}
			else if(mainSoundShouldBePlaying)
			{
				Start_Track_Sound(platformliftdata->sound,&sbPtr->DynPtr->Position);
			}
		}
	}


	//textprint("Platform pos: %d / %d / %d\n",platformliftdata->upHeight,dynPtr->Position.vy,platformliftdata->downHeight);

	/* send state messages in net game */
	if(AvP.Network==I_Host)
	{
		if(platformliftdata->netMsgCount>0)
		{
			/* don't send at rest messages: peers detect end of movement locally. */
			if(platformliftdata->state!=PLBS_AtRest) 
			{
				AddNetMsg_PlatformLiftState(sbPtr);
			}
			platformliftdata->netMsgCount--;	
		}
	}
}
	   
void InitialisePlatformLift(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_TOOLS_TEMPLATE *toolsData = (PLATFORMLIFT_TOOLS_TEMPLATE *)bhdata;
	PLATFORMLIFT_BEHAVIOUR_BLOCK* platformliftdata;
	int i;

	LOCALASSERT(sbPtr->I_SBtype == I_BehaviourPlatform);
	LOCALASSERT(toolsData);

	/* create, initialise and attach a data block */
	platformliftdata = (void *)AllocateMem(sizeof(PLATFORMLIFT_BEHAVIOUR_BLOCK));
	if(!platformliftdata)
	{
		memoryInitialisationFailure = 1;
		return;
	}

	sbPtr->SBdataptr = platformliftdata; 
	platformliftdata->homePosition = toolsData->position;
	platformliftdata->activationDelayTimer = 0;
	platformliftdata->state = PLBS_AtRest;
	platformliftdata->netMsgCount = 0;
	
	if(toolsData->travel<0)
	{
		platformliftdata->upHeight = platformliftdata->homePosition.vy + toolsData->travel;
		platformliftdata->downHeight = platformliftdata->homePosition.vy;
	} 		
	else
	{
		platformliftdata->upHeight = platformliftdata->homePosition.vy;
		platformliftdata->downHeight = platformliftdata->homePosition.vy + toolsData->travel;
	}
	
	LOCALASSERT(platformliftdata->upHeight < platformliftdata->downHeight);

	platformliftdata->sound = toolsData->sound;
	platformliftdata->start_sound = toolsData->start_sound;
	platformliftdata->end_sound = toolsData->end_sound;

	platformliftdata->Enabled = toolsData->Enabled;
	platformliftdata->OneUse = toolsData->OneUse;


	/* create and initialise the dynamics block */
	{
		DYNAMICSBLOCK *dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PLATFORM_LIFT);
		GLOBALASSERT(dynPtr);
		
		sbPtr->DynPtr = dynPtr;      	
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->LinVelocity.vx = dynPtr->LinVelocity.vy = dynPtr->LinVelocity.vz = 0;
		dynPtr->OrientEuler = toolsData->orientation;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}

	/* strategy block initialisation */
	sbPtr->shapeIndex = toolsData->shapeIndex;
	for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];
	sbPtr->integrity = 1;
}

/*------------------------------Patrick 2/4/97-----------------------------------
  State change functions: these are alos used by network message processing fns
  --------------------------------------------------------------------------------*/
void ActivatePlatformLift(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);

	platformliftdata->state = PLBS_Activating;
	platformliftdata->activationDelayTimer = PLATFORMLIFT_ACTIVATIONTIME;
	platformliftdata->netMsgCount = PLATFORMLIFT_NUMNETMESSAGES;

}

void SendPlatformLiftUp(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	
	platformliftdata->state = PLBS_GoingUp;
	sbPtr->DynPtr->Displacement.vx = sbPtr->DynPtr->Displacement.vz = 0;
	platformliftdata->netMsgCount = PLATFORMLIFT_NUMNETMESSAGES;
	
	sbPtr->DynPtr->Displacement.vy = -MUL_FIXED(PLATFORMLIFT_SPEED,NormalFrameTime);
	{
		int d = platformliftdata->upHeight - sbPtr->DynPtr->Position.vy;

	 	if (d>sbPtr->DynPtr->Displacement.vy)
	 	{
	 		sbPtr->DynPtr->Displacement.vy=d;
		}
	}
}

void SendPlatformLiftDown(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	
	platformliftdata->state = PLBS_GoingDown;
	sbPtr->DynPtr->Displacement.vx = sbPtr->DynPtr->Displacement.vz = 0;
	platformliftdata->netMsgCount = PLATFORMLIFT_NUMNETMESSAGES;

	sbPtr->DynPtr->Displacement.vy = MUL_FIXED(PLATFORMLIFT_SPEED,NormalFrameTime);
	{
		int d = platformliftdata->downHeight - sbPtr->DynPtr->Position.vy;

	 	if (d<sbPtr->DynPtr->Displacement.vy)
	 	{
	 		sbPtr->DynPtr->Displacement.vy=d;
		}
	}
}

void StopPlatformLift(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);

	sbPtr->DynPtr->Displacement.vx = sbPtr->DynPtr->Displacement.vy = sbPtr->DynPtr->Displacement.vz = 0;
	platformliftdata->state = PLBS_AtRest;

	/* just to stop drift... */
	/*
	if(PLiftIsNearerUpThanDown(sbPtr))
		sbPtr->DynPtr->Position.vy = platformliftdata->upHeight;
	else
		sbPtr->DynPtr->Position.vy = platformliftdata->downHeight;
	*/
	platformliftdata->netMsgCount = PLATFORMLIFT_NUMNETMESSAGES;

	//stop sound if the lift has any
	if(platformliftdata->sound)
	{
		Stop_Track_Sound(platformliftdata->sound);
	}
	//start end sound if lift has any
	if(platformliftdata->end_sound)
	{
		Start_Track_Sound(platformliftdata->end_sound,&sbPtr->DynPtr->Position);
	}

}
					

/*------------------------------Patrick 15/3/97-----------------------------------
  Support functions
  --------------------------------------------------------------------------------*/
static int SquashingSomething(DYNAMICSBLOCK *dynPtr)
{
	struct collisionreport *nextReport;
	int squashingSomething = 0;
	LOCALASSERT(dynPtr);
	nextReport = dynPtr->CollisionReportPtr;
				
	/* walk the collision report list, looking for collisions against objects */
	while(nextReport)
	{
		STRATEGYBLOCK *sbPtr = nextReport->ObstacleSBPtr;
		if(sbPtr)
		{
			if(nextReport->ObstacleNormal.vy < -46340)
			{
				extern DAMAGE_PROFILE FallingDamage;
				squashingSomething=1;
				
				/* squish! */
				CauseDamageToObject(sbPtr, &FallingDamage, (100*NormalFrameTime), NULL);
			}
			else if (nextReport->ObstacleNormal.vy > 46340)
			{
				DYNAMICSBLOCK *objDynPtr = sbPtr->DynPtr;
				if (objDynPtr)
				{
					if (objDynPtr->LinImpulse.vy < PLATFORMLIFT_SPEED)
					{
						objDynPtr->LinImpulse.vy = PLATFORMLIFT_SPEED;
					}
				}
			}
		}		
		nextReport = nextReport->NextCollisionReportPtr;
	}
	return squashingSomething;
}

static void PushPassengersUpwards(DYNAMICSBLOCK *dynPtr)
{
	#if 0
	struct collisionreport *nextReport;

	LOCALASSERT(dynPtr);
	nextReport = dynPtr->CollisionReportPtr;
				
	/* walk the collision report list, looking for collisions against objects */
	while(nextReport)
	{
		STRATEGYBLOCK *sbPtr = nextReport->ObstacleSBPtr;
		if(sbPtr)
		{
			DYNAMICSBLOCK *objDynPtr = sbPtr->DynPtr;
			if (objDynPtr)
			{
				int upwardsImpulse = -MUL_FIXED(GRAVITY_STRENGTH,NormalFrameTime) - PLATFORMLIFT_SPEED;

				if (objDynPtr->LinImpulse.vy > upwardsImpulse)
				{
					objDynPtr->LinImpulse.vy = upwardsImpulse;
					objDynPtr->IsInContactWithFloor =1;
					PrintDebuggingText("I'M PUSHING UP AN OBJECT!");
				}
			}
		}		
		nextReport = nextReport->NextCollisionReportPtr;
	}
	#endif
}

static int ActivatedByPlayer(DYNAMICSBLOCK *dynPtr)
{
	struct collisionreport *nextReport;

	LOCALASSERT(dynPtr);
	nextReport = dynPtr->CollisionReportPtr;
				
	/* walk the collision report list, looking for collisions against objects */
	while(nextReport)
	{
///		textprint("collision with %p\n",nextReport->ObstacleSBPtr);
		if(nextReport->ObstacleSBPtr == Player->ObStrategyBlock) return 1;		
	
		nextReport = nextReport->NextCollisionReportPtr;
	}
	return 0;
}

static int PlayerIsNearOtherTerminal(STRATEGYBLOCK *sbPtr)
{
	int averageHeight;
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	LOCALASSERT(platformliftdata->state == PLBS_AtRest);
 	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);
	
	averageHeight = (platformliftdata->upHeight+platformliftdata->downHeight)/2;

	if(dynPtr->Position.vy > averageHeight)
	{
		if (Player->ObStrategyBlock->DynPtr->Position.vy < averageHeight)
			return 1;
	}
	else
	{
		if (Player->ObStrategyBlock->DynPtr->Position.vy > averageHeight)
			return 1;
	}
	return 0;
}

/* returns 1 if the player or a remote player is within 5 m of the other end of
the lift track...*/
static int NetPlayerAtOtherTerminal(STRATEGYBLOCK *sbPtr)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	
	int sbIndex = 0;
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH otherTerminal;

	LOCALASSERT(sbPtr);
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	LOCALASSERT(platformliftdata->state == PLBS_AtRest);
 	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if(PLiftIsNearerUpThanDown(sbPtr))
	{
		otherTerminal = dynPtr->Position;
		otherTerminal.vy = platformliftdata->downHeight;
	}
	else
	{
		otherTerminal = dynPtr->Position;
		otherTerminal.vy = platformliftdata->upHeight;	
	}

	while(sbIndex < NumActiveStBlocks)
	{	
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[sbIndex++];

		if(sbPtr->I_SBtype==I_BehaviourNetGhost)
		{
			NETGHOSTDATABLOCK *ghostData = 	ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(ghostData);

			if((ghostData->type==I_BehaviourMarinePlayer)||
	   		   (ghostData->type==I_BehaviourAlienPlayer)||
	   		   (ghostData->type==I_BehaviourPredatorPlayer))
			{
				/* remote player */
				LOCALASSERT(sbPtr->DynPtr);
				if(VectorDistance(&(sbPtr->DynPtr->Position),&(otherTerminal))<5000) return 1;
			}
		}
		else if(sbPtr==Player->ObStrategyBlock)
		{
			/* local player */
			LOCALASSERT(sbPtr->DynPtr);
			if(VectorDistance(&(sbPtr->DynPtr->Position),&(otherTerminal))<5000) return 1;
		}
	}		
	return 0;
}


static int PLiftIsNearerUpThanDown(STRATEGYBLOCK *sbPtr)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	int upDist, downDist;

	LOCALASSERT(sbPtr);
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	LOCALASSERT(sbPtr->DynPtr);

	upDist = sbPtr->DynPtr->Position.vy - platformliftdata->upHeight;
	downDist = platformliftdata->downHeight - sbPtr->DynPtr->Position.vy;

	if(upDist<downDist) return 1;
	else return 0;
}


void NetworkPeerChangePlatformLiftState(STRATEGYBLOCK* sbPtr,PLATFORMLIFT_STATES new_state)
{
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	LOCALASSERT(sbPtr);
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platformliftdata);
	LOCALASSERT(sbPtr->DynPtr);

	//check to see if this is a new state , and change any sound playing as necessary

	if(new_state==PLBS_Activating)
	{
		/*
		ignore this state change , since only the host does anything while the lift is
		activating , and this will prevent the lift from stopping at the ends correctly
		*/
		return;

	}
	
	if((new_state==PLBS_GoingUp || new_state==PLBS_GoingUp) &&
	   !(platformliftdata->state==PLBS_GoingUp || platformliftdata->state==PLBS_GoingDown))
	{
		//platform lift has started moving
		if(platformliftdata->start_sound)
		{
			Start_Track_Sound(platformliftdata->start_sound,&sbPtr->DynPtr->Position);
		}
	}
	
	platformliftdata->state=new_state;
	

}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct platform_lift_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	DYNAMICSBLOCK dynamics;

	int activationDelayTimer;
	PLATFORMLIFT_STATES	state;
	
	BOOL Enabled; 
	BOOL OneUse; //if set ,lift becomes disabled after changing position once
	

}PLATFORM_LIFT_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV platformliftdata

void LoadStrategy_PlatformLift(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	PLATFORM_LIFT_SAVE_BLOCK* block = (PLATFORM_LIFT_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourPlatform) return;

	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(activationDelayTimer)
	COPYELEMENT_LOAD(state)
	COPYELEMENT_LOAD(Enabled)
	COPYELEMENT_LOAD(OneUse)

	*sbPtr->DynPtr = block->dynamics;

}


void SaveStrategy_PlatformLift(STRATEGYBLOCK* sbPtr)
{
	PLATFORM_LIFT_SAVE_BLOCK *block;
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata;
	
	platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	
	COPYELEMENT_SAVE(activationDelayTimer)
	COPYELEMENT_SAVE(state)
	COPYELEMENT_SAVE(Enabled)
	COPYELEMENT_SAVE(OneUse)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
}
