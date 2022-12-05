#define DB_LEVEL 2

/*------------------------ChrisF 17/3/98-------------------------------
  Source file for Queen AI behaviour functions.  One year 30 days after bh_paq.
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
#include "bh_queen.h"
#include "bh_debri.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "psnd.h"
#include "weapons.h"
#include "extents.h"
#include "sequnces.h"
#include "showcmds.h"
#include "targeting.h"
#include "dxlog.h"
#include <math.h>
#include "db.h"

#include "los.h"
#include "bh_track.h"
#include "scream.h"

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern char LevelName[];
extern unsigned char Null_Name[8];

#define QueenAttackRange 3500

/*minimum time for flamethrower before queen takes notice*/
#define QueenMinimumFireTime ONE_FIXED/8

static BOOL PlayerInTrench=FALSE;
static BOOL PlayerInLocker=FALSE;
static int AirlockTimeOpen=0;



/*special hangar airlock state stuff*/
static BOOL UpperAirlockDoorOpen=FALSE;
static BOOL LowerAirlockDoorOpen=FALSE;
static STRATEGYBLOCK* UpperAirlockDoorSbptr=0;
static VECTORCH UpperAirlockDoorStart;
static STRATEGYBLOCK* LowerAirlockDoorSbptr=0;
static VECTORCH LowerAirlockDoorStart;
static STRATEGYBLOCK* LockerDoorSbptr=0;


#define QUEEN_MAX_OBJECT 10
int NumQueenObjects;
STRATEGYBLOCK* QueenObjectList[QUEEN_MAX_OBJECT];


void SetQueenShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);
void QueenMove_Standby(STRATEGYBLOCK *sbPtr);
void QueenMove_StepForward(STRATEGYBLOCK *sbPtr);
void QueenMove_StepBack(STRATEGYBLOCK *sbPtr);
void QueenMove_TurnLeft(STRATEGYBLOCK *sbPtr);
void QueenMove_TurnRight(STRATEGYBLOCK *sbPtr);
void QueenMove_Walk(STRATEGYBLOCK *sbPtr);
void QueenMove_Taunt(STRATEGYBLOCK *sbPtr);
void QueenMove_Hiss(STRATEGYBLOCK *sbPtr);
void QueenMove_LeftSwipe(STRATEGYBLOCK *sbPtr);
void QueenMove_RightSwipe(STRATEGYBLOCK *sbPtr);
void QueenMove_Charge(STRATEGYBLOCK *sbPtr);
void QueenMove_Close(STRATEGYBLOCK *sbPtr);
void Execute_Queen_Dying(STRATEGYBLOCK *sbPtr);
void KillQueen(STRATEGYBLOCK *sbPtr,DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming);
void QueenForceReconsider(STRATEGYBLOCK *sbPtr);
static BOOL TargetIsFiringFlamethrowerAtQueen(STRATEGYBLOCK *sbPtr);
static void MakeNonFragable(HMODELCONTROLLER *controller);
static void QueenCalculateTargetInfo(STRATEGYBLOCK *sbPtr);
void HandleHangarAirlock();
static BOOL LockerDoorIsClosed();

QUEEN_MANOEUVRE Queen_Next_Command;

int Queen_Next_Waypoint=0;

int Queen_Walk_Rate=100000;
int Queen_Walk_Step_Speed=ONE_FIXED/4.5;

int Queen_Charge_Rate=45000;
int Queen_ButtCharge_Rate=30000;
int Queen_Step_Time=50000;
int Queen_Step_Speed=10000; //16000;
int Queen_Charge_Step_Speed=7500;
int Queen_Step_Mode=1;
int Queen_Turn_Rate=(NPC_TURNRATE>>2);

#define QUEEN_BUTTCHARGE_SPEED 16000
#define QUEEN_CHARGE_SPEED 12000

#define QUEEN_CLOSE_SPEED 3000

#define QUEEN_WALK_SPEED 7000

#define QUEEN_THROWN_OBJECT_SPEED 60000

extern DAMAGE_PROFILE QueenButtDamage;
extern DAMAGE_PROFILE QueenImpactDamage; 
extern DAMAGE_PROFILE VacuumDamage;

VECTORCH Queen_Target_Point={0,0,0};

VECTORCH Queen_Waypoints[] = {
	{37361,2900,-51942},
	{32937,2900,-19959},
	{62809,2900,-28509},
	{64658,2900,-13328},
	{64658,0,-13328},
	{64991,2900,-50362},
	{32239,2900,-53578},
	{-1,-1,-1}
};

void QComm_Stop(void) {
	Queen_Next_Command=QM_Standby;
	Queen_Next_Waypoint=-1;
}

void QComm_StepForward(void) {
	Queen_Next_Command=QM_StepForward;
}

void QComm_StepBack(void) {
	Queen_Next_Command=QM_StepBack;
}

void QComm_TurnLeft(void) {
	Queen_Next_Command=QM_TurnLeft;
}

void QComm_TurnRight(void) {
	Queen_Next_Command=QM_TurnRight;
}

void QComm_Heel(void) {
	Queen_Next_Command=QM_ComeToPoint;
	Queen_Target_Point=Player->ObStrategyBlock->DynPtr->Position;
}

void QComm_Taunt(void) {
	Queen_Next_Command=QM_Taunt;
}

void QComm_Hiss(void) {
	Queen_Next_Command=QM_Hiss;
}

void QComm_LeftSwipe(void) {
	Queen_Next_Command=QM_LeftSwipe;
}

void QComm_RightSwipe(void) {
	Queen_Next_Command=QM_RightSwipe;
}

void QComm_Route(void) {
	Queen_Next_Command=QM_Standby;
	Queen_Next_Waypoint=-2;
}

void QComm_Charge(void) {
	Queen_Next_Command=QM_Charge;
	Queen_Target_Point=Player->ObStrategyBlock->DynPtr->Position;
}

static void QueenSoundHiss(STRATEGYBLOCK* sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	GLOBALASSERT(sbPtr);
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	if(queenStatusPointer->soundHandle==SOUND_NOACTIVEINDEX)
	{
		PlayQueenSound(0,QSC_Hiss,0,&queenStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
		queenStatusPointer->lastSoundCategory=QSC_Hiss;		
	}
}

static void QueenSoundHurt(STRATEGYBLOCK* sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	GLOBALASSERT(sbPtr);
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	/*
	If the queen is currently playing a non-hurt sound then cancel it.
	*/
	if(queenStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX)
	{
		if(queenStatusPointer->lastSoundCategory!=QSC_Scream_Hurt)
		{
			Sound_Stop(queenStatusPointer->soundHandle);
		}
	}
	
	if(queenStatusPointer->soundHandle==SOUND_NOACTIVEINDEX)
	{
		PlayQueenSound(0,QSC_Scream_Hurt,0,&queenStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
		queenStatusPointer->lastSoundCategory=QSC_Scream_Hurt;		
	}
}

static void ThrownObjectBounceNoise(int object_index,VECTORCH* location)
{
	static int QueenObjectSoundHandles[QUEEN_MAX_OBJECT]={SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX,SOUND_NOACTIVEINDEX};
	GLOBALASSERT(location);

	PlayQueenSound(0,QSC_Object_Bounce,0,&QueenObjectSoundHandles[object_index],location);
}

void InitQueenBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_QUEEN *toolsData; 
	QUEEN_STATUS_BLOCK *queenStatus;
	int i;

	LOCALASSERT(sbPtr);
	LOCALASSERT(bhdata);
	toolsData = (TOOLS_DATA_QUEEN *)bhdata; 

	/* Reset command interface. */

	Queen_Next_Command=QM_Standby;

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
	
		dynPtr->UseDisplacement = 1;

		dynPtr->Displacement.vx = 0;
		dynPtr->Displacement.vy = 0;
		dynPtr->Displacement.vz = 0;

		dynPtr->Mass=60000; /* No knockback, please. */
	}
	else
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* Initialise alien's stats */
	{
		NPC_DATA *NpcData;

		NpcData=GetThisNpcData(I_NPC_AlienQueen);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;

		
	}
	/* create, initialise and attach a predator-alien/queen data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(QUEEN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		
		queenStatus = (QUEEN_STATUS_BLOCK *)sbPtr->SBdataptr;
		NPC_InitMovementData(&(queenStatus->moveData));
		NPC_InitWanderData(&(queenStatus->wanderData));

   		sbPtr->integrity = QUEEN_STARTING_HEALTH;
		
		queenStatus->QueenState=QBS_Reconsider;
		queenStatus->current_move=QM_Standby;
		queenStatus->next_move=QM_Standby;
		queenStatus->fixed_foot=RightFoot;
		queenStatus->fixed_foot_section=NULL; //Stupid, but I wouldn't want in uninitialised.
		queenStatus->fixed_foot_oldpos.vx=0;
		queenStatus->fixed_foot_oldpos.vy=0;
		queenStatus->fixed_foot_oldpos.vz=0;

		queenStatus->TargetPos.vx=0;
		queenStatus->TargetPos.vy=0;
		queenStatus->TargetPos.vz=0;


		queenStatus->moveTimer = 0;

		for(i=0;i<SB_NAME_LENGTH;i++) queenStatus->death_target_ID[i] = toolsData->death_target_ID[i];
		queenStatus->death_target_sbptr=0;
		queenStatus->death_target_request=toolsData->death_target_request;

		root_section=GetNamedHierarchyFromLibrary("queen","Template");
		GLOBALASSERT(root_section);
		Create_HModel(&queenStatus->HModelController,root_section);
		InitHModelSequence(&queenStatus->HModelController,
			(int)HMSQT_QueenRightStanceTemplate,
			(int)QRSTSS_Standard,
			ONE_FIXED);
		queenStatus->HModelController.Looped=1;

		queenStatus->attack_delta=Add_Delta_Sequence(&queenStatus->HModelController,"attack",
			(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftSwipe,Queen_Step_Time);
		queenStatus->attack_delta->Playing=0;

		queenStatus->hit_delta=Add_Delta_Sequence(&queenStatus->HModelController,"hit",
			(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftHit,Queen_Step_Time);
		queenStatus->attack_delta->Playing=0;

		queenStatus->TempTarget=FALSE;
		queenStatus->CurrentQueenObject=-1;
		queenStatus->QueenObjectBias=1;
		if(!stricmp(LevelName,"hangar"))
		{
			queenStatus->QueenPlayerBias=1;
		}
		else
		{
			//int he predator version , make it more likely for the queen to go after the player
			queenStatus->QueenPlayerBias=5;
		}
		queenStatus->QueenTargetSB=Player->ObStrategyBlock;
		queenStatus->QueenTauntTimer=0;
		queenStatus->QueenFireTimer=0;
		
		queenStatus->LastVelocity.vx=0;
		queenStatus->LastVelocity.vy=0;
		queenStatus->LastVelocity.vz=0;

		queenStatus->BeenInAirlock=FALSE;
		queenStatus->QueenActivated=FALSE;

		queenStatus->soundHandle=SOUND_NOACTIVEINDEX;

		
		NumQueenObjects=-1;

		if(AvP.PlayerType==I_Marine)
		{
				MakeNonFragable(&queenStatus->HModelController);
		}

		queenStatus->AttackDoneItsDamage = FALSE;
	
	}		   	   	   	   
	else
	{
		GLOBALASSERT(0);
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

}

static BOOL QueenPlayingStunAnimation(HMODELCONTROLLER* HModelController)
{
	GLOBALASSERT(HModelController);

	return(HModelController->Sequence_Type==HMSQT_QueenGeneral &&
		   HModelController->Sub_Sequence==QGSS_Explosion_Stun &&
		   !HModelAnimation_IsFinished(HModelController));
	
}

void SetQueenFoot(STRATEGYBLOCK *sbPtr, QUEEN_FOOT foot) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	DISPLAYBLOCK *dPtr;

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dPtr=sbPtr->SBdptr;
	GLOBALASSERT(dPtr);

	ProveHModel(dPtr->HModelControlBlock,dPtr);

	switch (foot) {
		case (LeftFoot):
			queenStatusPointer->fixed_foot=LeftFoot;
			queenStatusPointer->fixed_foot_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,
				"left foot");
			GLOBALASSERT(queenStatusPointer->fixed_foot_section);
			queenStatusPointer->fixed_foot_oldpos=queenStatusPointer->fixed_foot_section->World_Offset;
			break;
		case (RightFoot):
			queenStatusPointer->fixed_foot=RightFoot;
			queenStatusPointer->fixed_foot_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,
				"right foot");
			GLOBALASSERT(queenStatusPointer->fixed_foot_section);
			queenStatusPointer->fixed_foot_oldpos=queenStatusPointer->fixed_foot_section->World_Offset;
			break;
		default:
			GLOBALASSERT(0);
			break;
	}
	
}


void MakeQueenNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;
	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	QUEEN_STATUS_BLOCK *queenStatusPointer= (QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	/* This should only be called once! */

    LOCALASSERT(queenStatusPointer);
    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;	
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr=NULL;
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;
	if(dPtr==NULL) {
		GLOBALASSERT(0);
	}
		
	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					
                            
	
	/*Copy extents from the collision extents in extents.c*/
	dPtr->ObMinX=-CollisionExtents[CE_QUEEN].CollisionRadius;
	dPtr->ObMaxX=CollisionExtents [CE_QUEEN].CollisionRadius;
	dPtr->ObMinZ=-CollisionExtents[CE_QUEEN].CollisionRadius;
	dPtr->ObMaxZ=CollisionExtents [CE_QUEEN].CollisionRadius;
	dPtr->ObMinY=CollisionExtents [CE_QUEEN].CrouchingTop;
	dPtr->ObMaxY=CollisionExtents [CE_QUEEN].Bottom;
	dPtr->ObRadius = 1000;
	/* also need to initialise positional information in the new display block, 
	from the existing dynamics block.
	NB this necessary because this function is (usually) called between the 
	dynamics and rendering systems so it is not initialised by the dynamics 
	system the first frame it is drawn. */
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;

	/* init state timers */

	/* zero linear velocity in dynamics block */
	dynPtr->LinVelocity.vx = 0;
	dynPtr->LinVelocity.vy = 0;
	dynPtr->LinVelocity.vz = 0;

	/* initialise our sequence data */
	dPtr->HModelControlBlock=&queenStatusPointer->HModelController;

	SetQueenFoot(sbPtr,RightFoot);

	/* Calls ProveHModel. */

}  

void MakeQueenFar(STRATEGYBLOCK *sbPtr) {

	/* get the queen's status block */
	int i;
	QUEEN_STATUS_BLOCK *queenStatusPointer= (QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	LOCALASSERT(sbPtr);
    LOCALASSERT(queenStatusPointer);
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	if(sbPtr->SBdptr)
	{
		i = DestroyActiveObject(sbPtr->SBdptr);
		LOCALASSERT(i==0);
		sbPtr->SBdptr = NULL;
	}

	/* zero linear velocity in dynamics block */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

}


void SetQueenMovement_FromFoot(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	VECTORCH delta_offset,real_pos;
	DISPLAYBLOCK *dPtr;
	
	
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	dPtr=sbPtr->SBdptr;
	GLOBALASSERT(dPtr);

	real_pos=queenStatusPointer->fixed_foot_section->World_Offset;

	ProveHModel(dPtr->HModelControlBlock,dPtr);

	delta_offset.vx=queenStatusPointer->fixed_foot_oldpos.vx-queenStatusPointer->fixed_foot_section->World_Offset.vx;
	delta_offset.vy=0;//queenStatusPointer->fixed_foot_oldpos.vy-queenStatusPointer->fixed_foot_section->World_Offset.vy;
	delta_offset.vz=queenStatusPointer->fixed_foot_oldpos.vz-queenStatusPointer->fixed_foot_section->World_Offset.vz;

	#if 1	
	delta_offset.vx=DIV_FIXED(delta_offset.vx,NormalFrameTime);
	delta_offset.vy=0;//DIV_FIXED(delta_offset.vy,NormalFrameTime);
	delta_offset.vz=DIV_FIXED(delta_offset.vz,NormalFrameTime);

	sbPtr->DynPtr->LinVelocity.vx=delta_offset.vx;
	sbPtr->DynPtr->LinVelocity.vy=delta_offset.vy;
	sbPtr->DynPtr->LinVelocity.vz=delta_offset.vz;

	{
		int facex;
		int facez;
		facex=sbPtr->DynPtr->OrientMat.mat31;
		facez=sbPtr->DynPtr->OrientMat.mat33;

		if((MUL_FIXED(facex,sbPtr->DynPtr->LinVelocity.vx)+MUL_FIXED(facez,sbPtr->DynPtr->LinVelocity.vz))<0)
		{
			//don't want queen to move backwards
			sbPtr->DynPtr->LinVelocity.vx = 0;		
			sbPtr->DynPtr->LinVelocity.vy = 0;
			sbPtr->DynPtr->LinVelocity.vz = 0;
			
			switch (queenStatusPointer->fixed_foot) 
			{
				case (LeftFoot):
					SetQueenFoot(sbPtr,LeftFoot);
					break;
				case (RightFoot):
					SetQueenFoot(sbPtr,RightFoot);
 					break;
				default:
					GLOBALASSERT(0);
					break;
			}
				
		}
		
	}
	#elif 1
	sbPtr->DynPtr->Displacement.vx = delta_offset.vx;
	sbPtr->DynPtr->Displacement.vy = delta_offset.vy;
	sbPtr->DynPtr->Displacement.vz = delta_offset.vz;

	//PrintDebuggingText("Displacement = %d %d %d\n",delta_offset.vx,delta_offset.vy,delta_offset.vz);
	//
	//LOGDXFMT(("New Foot Frame.\nFoot OldPos = %d %d %d.\nFoot NewPos = %d %d %d\nFoot Current Pos = %d %d %d.\nDisplacement = %d %d %d.\n\n",
	//	queenStatusPointer->fixed_foot_oldpos.vx,queenStatusPointer->fixed_foot_oldpos.vy,queenStatusPointer->fixed_foot_oldpos.vz,
	//	real_pos.vx,real_pos.vy,real_pos.vz,
	//	queenStatusPointer->fixed_foot_section->World_Offset.vx,queenStatusPointer->fixed_foot_section->World_Offset.vy,
	//	queenStatusPointer->fixed_foot_section->World_Offset.vz,delta_offset.vx,delta_offset.vy,delta_offset.vz));
	//
	//sbPtr->DynPtr->Displacement.vx = 0;
	//sbPtr->DynPtr->Displacement.vy = 0;
	//sbPtr->DynPtr->Displacement.vz = 200;

	#else
	sbPtr->DynPtr->Position.vx += delta_offset.vx;
	sbPtr->DynPtr->Position.vy += delta_offset.vy;
	sbPtr->DynPtr->Position.vz += delta_offset.vz;
	
	#endif
}

void QueenMove_Standby(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* Verify correct sequence. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if(queenStatusPointer->PlayingHitDelta)
	{
		if(QueenPlayingStunAnimation(&queenStatusPointer->HModelController))
		{
			//queen is in the process of playing stun animation
			return;
		}
	}
	
	if(queenStatusPointer->moveTimer==0)
	{
		//start tweening to standing position
		switch(queenStatusPointer->fixed_foot)
		{
			case (LeftFoot) :
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
					(int)QLSTSS_Standard,-1,ONE_FIXED>>2);
				break;
			case (RightFoot) :
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
					(int)QRSTSS_Standard,-1,ONE_FIXED>>2);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
		queenStatusPointer->moveTimer=1;
	}

}

void QueenMove_StepForward(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_StepForward);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
					(int)QLSTSS_Forward_L2R,Queen_Step_Time,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
					(int)QRSTSS_Forward_R2L,Queen_Step_Time,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
		queenStatusPointer->current_move=QM_Standby;
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_StepBack(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_StepBack);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				/* Wrong Foot w.r.t. Forward. */
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Backward_L2R,Queen_Step_Time,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				/* Wrong Foot w.r.t. Forward. */
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Backward_R2L,Queen_Step_Time,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	queenStatusPointer->moveTimer+=NormalFrameTime;

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
	 	queenStatusPointer->current_move=QM_Standby;
		/* Already changed foot. */
	}


}

void QueenMove_TurnLeft(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_TurnLeft);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Forward_L2R,Queen_Step_Time,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Forward_R2L,Queen_Step_Time,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... turn left. */

	{

		VECTORCH left90;
		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		left90.vx=-sbPtr->DynPtr->OrientMat.mat11;
		left90.vy=-sbPtr->DynPtr->OrientMat.mat12;
		left90.vz=-sbPtr->DynPtr->OrientMat.mat13;

		#if 1
		NPCOrientateToVector(sbPtr, &left90,Queen_Turn_Rate,NULL);

		SetQueenMovement_FromFoot(sbPtr);
		#else
		{
			VECTORCH localOffset;
			VECTORCH version1,version2;
			//MATRIXCH WtoL;
	
			ProveHModel(dPtr->HModelControlBlock,dPtr);

			localOffset.vx=queenStatusPointer->fixed_foot_section->World_Offset.vx-sbPtr->DynPtr->Position.vx;
			localOffset.vy=queenStatusPointer->fixed_foot_section->World_Offset.vy-sbPtr->DynPtr->Position.vy;
			localOffset.vz=queenStatusPointer->fixed_foot_section->World_Offset.vz-sbPtr->DynPtr->Position.vz;
		
			//WtoL=sbPtr->DynPtr->OrientMat;
			//TransposeMatrixCH(&WtoL);
			
			//RotateVector(&localOffset,&WtoL);

			PrintDebuggingText("localOffset = %d %d %d\n",localOffset.vx,
				localOffset.vy,localOffset.vz);

			NPCOrientateToVector(sbPtr, &left90,Queen_Turn_Rate,&localOffset);

			//version1=sbPtr->DynPtr->Displacement;
			//SetQueenMovement_FromFoot(sbPtr);
			//version2=sbPtr->DynPtr->Displacement;
			// 
			//LOGDXFMT(("Displacement V1 = %d %d %d\nDisplacement V2 = %d %d %d\n",
			//	version1.vx,version1.vy,version1.vz,
			//	version2.vx,version2.vy,version2.vz));
		}
		#endif

	}

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		queenStatusPointer->current_move=QM_Standby;

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_TurnRight(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_TurnRight);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Forward_L2R,Queen_Step_Time,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Forward_R2L,Queen_Step_Time,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... turn right. */

	{

		VECTORCH right90;
		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		right90.vx=sbPtr->DynPtr->OrientMat.mat11;
		right90.vy=sbPtr->DynPtr->OrientMat.mat12;
		right90.vz=sbPtr->DynPtr->OrientMat.mat13;

		NPCOrientateToVector(sbPtr, &right90,Queen_Turn_Rate,NULL);

		SetQueenMovement_FromFoot(sbPtr);

	}

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		queenStatusPointer->current_move=QM_Standby;

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void SetQueenShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime)
{

	QUEEN_STATUS_BLOCK *queenStatus=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);

	GLOBALASSERT(length!=0);

	if (tweeningtime<=0) {
		InitHModelSequence(&queenStatus->HModelController,(int)type,subtype,length);
		queenStatus->HModelController.Looped=0;
	} else {
		InitHModelTweening(&queenStatus->HModelController, tweeningtime, (int)type,subtype,length, 0);
		queenStatus->HModelController.Looped=0;
	}
}

void QueenMove_ComeToPoint(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	VECTORCH vectotarget;

	/* Complex movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_ComeToPoint);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=0;
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Forward_L2R,Queen_Step_Time,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Forward_R2L,Queen_Step_Time,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}
	
	vectotarget.vx=queenStatusPointer->TargetPos.vx-sbPtr->DynPtr->Position.vx;
	vectotarget.vy=queenStatusPointer->TargetPos.vy-sbPtr->DynPtr->Position.vy;
	vectotarget.vz=queenStatusPointer->TargetPos.vz-sbPtr->DynPtr->Position.vz;

	/* Now... turn to face. */

	{

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		NPCOrientateToVector(sbPtr, &vectotarget,Queen_Turn_Rate,NULL);

		SetQueenMovement_FromFoot(sbPtr);

	}


	queenStatusPointer->moveTimer+=NormalFrameTime;

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		int range;
		/* Comsider next step. */
		
		range=Approximate3dMagnitude(&vectotarget);

		if (range<5000) {
			queenStatusPointer->current_move=QM_Standby;
		} else {
			queenStatusPointer->moveTimer=0;
		}

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}

}

#if 1
void QueenMove_Walk(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	BOOL ChangingFoot=FALSE;

	/* Very complex movement function... */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_ComeToPoint);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(Queen_Step_Time>>2);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
					(int)QGSS_Walk,-1,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=1;
				queenStatusPointer->moveTimer=1; /* It's something of a state flag here. */

				break;
			case (RightFoot):
				/* Argh! Can't start from right foot! */
				queenStatusPointer->current_move=QM_Close;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_ComeToPoint;
				return;
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	

	/* Check for change foot? */

	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		if (queenStatusPointer->HModelController.keyframe_flags) 
		{
			if (queenStatusPointer->HModelController.keyframe_flags & 1) 
			{
				SetQueenFoot(sbPtr,LeftFoot);
				ChangingFoot=TRUE;
			}
			if (queenStatusPointer->HModelController.keyframe_flags & 2) 
			{
				SetQueenFoot(sbPtr,RightFoot);
				ChangingFoot=TRUE;
			}
			if(queenStatusPointer->moveTimer==1) queenStatusPointer->moveTimer=3;
		}
		
		HModel_SetToolsRelativeSpeed(&queenStatusPointer->HModelController,(512*ONE_FIXED)/QUEEN_WALK_SPEED);

	}

	/* Now... turn to face. */

	{

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		//ProveHModel(dPtr->HModelControlBlock,dPtr);

		NPCOrientateToVector(sbPtr, &queenStatusPointer->VectToTarget,Queen_Turn_Rate,NULL);

		//SetQueenMovement_FromFoot(sbPtr);
		
		if (queenStatusPointer->moveTimer!=2) 
		{
			if(queenStatusPointer->moveTimer==1)
			{
				SetQueenMovement_FromFoot(sbPtr);
			}
			else
			{
				VECTORCH velocity;
				//int walkSpeed;
	
				velocity.vx=sbPtr->DynPtr->OrientMat.mat31;
				velocity.vy=0;
				velocity.vz=sbPtr->DynPtr->OrientMat.mat33;

				if ( (velocity.vx==0) && (velocity.vy==0) && (velocity.vz==0) ) {
					sbPtr->DynPtr->LinVelocity.vx = 0;		
					sbPtr->DynPtr->LinVelocity.vy = 0;
					sbPtr->DynPtr->LinVelocity.vz = 0;
					return;			
				}

				Normalise(&velocity);
		
				//walkSpeed=DIV_FIXED(Queen_Walk_Step_Speed,Queen_Walk_Rate);

				sbPtr->DynPtr->LinVelocity.vx = MUL_FIXED(velocity.vx,QUEEN_WALK_SPEED);
				sbPtr->DynPtr->LinVelocity.vy = MUL_FIXED(velocity.vy,QUEEN_WALK_SPEED);
				sbPtr->DynPtr->LinVelocity.vz = MUL_FIXED(velocity.vz,QUEEN_WALK_SPEED);
			}

		}
		

	}

	/* Consider exit state. */

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->moveTimer==2)) {
		
		/* Finished coming to a stop. */

		queenStatusPointer->current_move=QM_Standby;
		queenStatusPointer->next_move=QM_Standby;
		queenStatusPointer->moveTimer=0;

		switch (queenStatusPointer->fixed_foot) {
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		/* Would you believe the end's in the middle? :-) */

	}
	

	//only check for exiting charge , when changing foot
	{
		int range;
		QueenCalculateTargetInfo(sbPtr);
		range=queenStatusPointer->TargetDistance;

		if(range<3000)
		{
			queenStatusPointer->next_move=QM_Close;
		}
		if(ChangingFoot)
		{
			if(queenStatusPointer->PlayingHitDelta)
			{
				queenStatusPointer->HModelController.Playing=0;
				queenStatusPointer->current_move=QM_Standby;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_Standby;
				return;
			}
			
			if(queenStatusPointer->moveTimer!=2)
			{

				if (queenStatusPointer->next_move!=QM_Standby && queenStatusPointer->next_move!=QM_ComeToPoint) 
				{
					//exit without tweening to stopped position
					queenStatusPointer->HModelController.Playing=0;
					queenStatusPointer->current_move=queenStatusPointer->next_move;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->moveTimer=0;

				}
				#if 0
				else if(range<3000)
				{
					//come to a stop
					/* Begin exit. */
					int tweeiningtime=(Queen_Step_Time>>2);
	
					queenStatusPointer->moveTimer=2; /* It's something of a state flag here. */
	
					switch (queenStatusPointer->fixed_foot) {
						case (RightFoot):
							SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
								(int)QRSTSS_Standard,(Queen_Step_Time<<1),tweeiningtime);
							break;
						case (LeftFoot):
							SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
								(int)QLSTSS_Standard,(Queen_Step_Time<<1),tweeiningtime);
							break;
						default:
							GLOBALASSERT(0);
							break;
					}
			
					queenStatusPointer->HModelController.LoopAfterTweening=0;
				}
				#endif
				else if(range>10000 && queenStatusPointer->fixed_foot==LeftFoot && !PlayerInTrench)
				{
					//go into a charge
					queenStatusPointer->current_move=QM_Charge;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->moveTimer=0;
				
				}
				if(range<5000)
				{
					/*if the queen is near but facing the wrong way need to go into close mode
					(turning circle is too large in walk mode)  */


					if (queenStatusPointer->TargetDirection.vz<queenStatusPointer->TargetDirection.vx || 
					    queenStatusPointer->TargetDirection.vz<-queenStatusPointer->TargetDirection.vx) 
					{

						//need to switch to close mode
						queenStatusPointer->HModelController.Playing=0;
						queenStatusPointer->current_move=QM_Close;
						queenStatusPointer->moveTimer=0;
						queenStatusPointer->next_move=QM_Standby;
						return;
					}
				}

			}
		}
	}

}
#else
void QueenMove_Walk(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	VECTORCH vectotarget;

	/* Very complex movement function... */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_ComeToPoint);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(Queen_Step_Time>>2);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
					(int)QGSS_Walk,(Queen_Step_Time<<1),tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=1;
				queenStatusPointer->moveTimer=1; /* It's something of a state flag here. */
				break;
			case (RightFoot):
				/* Argh! Can't start from right foot! */
				queenStatusPointer->current_move=QM_StepForward;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_ComeToPoint;
				return;
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	vectotarget.vx=queenStatusPointer->TargetPos.vx-sbPtr->DynPtr->Position.vx;
	vectotarget.vy=queenStatusPointer->TargetPos.vy-sbPtr->DynPtr->Position.vy;
	vectotarget.vz=queenStatusPointer->TargetPos.vz-sbPtr->DynPtr->Position.vz;

	/* Check for change foot? */

	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		if (queenStatusPointer->HModelController.keyframe_flags) {
			switch (queenStatusPointer->fixed_foot) {
				case (LeftFoot):
					SetQueenFoot(sbPtr,RightFoot);
					break;
				case (RightFoot):
					SetQueenFoot(sbPtr,LeftFoot);
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
		}
	}

	/* Now... turn to face. */

	{

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		NPCOrientateToVector(sbPtr, &vectotarget,Queen_Turn_Rate,NULL);

		SetQueenMovement_FromFoot(sbPtr);

	}

	/* Consider exit state. */

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->moveTimer==2)) {
		
		/* We must have finished. */

		queenStatusPointer->current_move=QM_Standby;

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		/* Would you believe the end's in the middle? :-) */

	}
	

	{
		int range;
		
		range=Approximate3dMagnitude(&vectotarget);

		if ((range<5000)&&(queenStatusPointer->moveTimer!=2)) {
			/* Begin exit. */
			int tweeiningtime=(Queen_Step_Time>>2);
	
			queenStatusPointer->moveTimer=2; /* It's something of a state flag here. */
	
			switch (queenStatusPointer->fixed_foot) {
				case (LeftFoot):
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
						(int)QRSTSS_Standard,(Queen_Step_Time<<1),tweeiningtime);
					break;
				case (RightFoot):
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
						(int)QLSTSS_Standard,(Queen_Step_Time<<1),tweeiningtime);
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
		
			queenStatusPointer->HModelController.LoopAfterTweening=0;
		} else if (queenStatusPointer->moveTimer!=2) {
			queenStatusPointer->moveTimer=1; /* It's something of a state flag here. */
		}
	}

	#if 0
	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		int range;
		/* Comsider next step. */
		
		range=Approximate3dMagnitude(&vectotarget);

		if (range<5000) {
			queenStatusPointer->current_move=QM_Standby;
		} else {
			queenStatusPointer->moveTimer=0;
		}

	}
	#endif

}
#endif

void QueenMove_Taunt(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_Taunt);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Taunt,-1,tweeiningtime);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Taunt,-1,tweeiningtime);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}


	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
		
		queenStatusPointer->current_move=QM_Standby;
		
		/* Same foot. */

	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_Hiss(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */


	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_Hiss);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=1;
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=1;
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	/*
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) 
	{
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}
	*/
	

	
	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	#if 0
	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
		
		queenStatusPointer->current_move=QM_Hiss;
		
		/* Same foot. */

	}
	#endif
	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_LeftSwipe(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_LeftSwipe);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				//SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
				//	(int)QLSTSS_Forward_L2R,Queen_Step_Time,tweeiningtime);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);

				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenLeftStanceTemplate,(int)QLSTSS_LeftSwipe,Queen_Step_Time);
				queenStatusPointer->attack_delta->Playing=1;
				
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				//SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
				//	(int)QRSTSS_Forward_R2L,Queen_Step_Time,tweeiningtime);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);

				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftSwipe,Queen_Step_Time);
				queenStatusPointer->attack_delta->Playing=1;

				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
		
		queenStatusPointer->current_move=QM_Standby;
		
	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_RightSwipe(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_RightSwipe);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceFull,
					(int)QLSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);

				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenLeftStanceTemplate,(int)QLSTSS_RightSwipe,Queen_Step_Time);
				queenStatusPointer->attack_delta->Playing=1;
				
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceFull,
					(int)QRSFSS_Standard_Hiss,Queen_Step_Time,tweeiningtime);

				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_RightSwipe,Queen_Step_Time);
				queenStatusPointer->attack_delta->Playing=1;

				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}

	/* Now... move forward? */

	SetQueenMovement_FromFoot(sbPtr);

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {
		
		queenStatusPointer->current_move=QM_Standby;
		
	}

	queenStatusPointer->moveTimer+=NormalFrameTime;

}

void QueenMove_Charge(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	BOOL ChangingFoot=FALSE;

	/* Very different movement function... */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_Charge);

	/* Charge at the player. */
	if(!queenStatusPointer->TempTarget)
	{
	//	queenStatusPointer->TargetPos=Player->ObStrategyBlock->DynPtr->Position;
		queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;
	}
	


	if (queenStatusPointer->moveTimer==0) 
	{
		/* Do setup. */
		int tweeiningtime=(Queen_Step_Time>>2);

		sbPtr->DynPtr->LinVelocity.vx = 0;
		sbPtr->DynPtr->LinVelocity.vy = 0;
		sbPtr->DynPtr->LinVelocity.vz = 0;
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				
				{
					//which version of sprint should we use
					if(DeltaAnimation_IsFinished(queenStatusPointer->attack_delta))
						SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
							(int)QGSS_Sprint_Full,-1,tweeiningtime);
					else
						SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
							(int)QGSS_Sprint,-1,tweeiningtime);

					queenStatusPointer->HModelController.LoopAfterTweening=1;
					queenStatusPointer->moveTimer=1; /* It's something of a state flag here. */
				}

				break;
			case (RightFoot):
				/* Argh! Can't start from right foot! */
				queenStatusPointer->current_move=QM_Close;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_Charge;
				return;
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		queenStatusPointer->SwerveTimer=(ONE_FIXED/2)+(FastRandom() & 0xffff)*2;
		return;
	}

	/* Check for change foot? */

	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		if (queenStatusPointer->HModelController.keyframe_flags) 
		{
			if (queenStatusPointer->HModelController.keyframe_flags & 1) 
			{
				SetQueenFoot(sbPtr,LeftFoot);
				ChangingFoot=TRUE;
			}
			if (queenStatusPointer->HModelController.keyframe_flags & 2) 
			{
				SetQueenFoot(sbPtr,RightFoot);
				ChangingFoot=TRUE;
			}
		
			if(queenStatusPointer->moveTimer==1) queenStatusPointer->moveTimer=3;
		}	
		HModel_SetToolsRelativeSpeed(&queenStatusPointer->HModelController,(512*ONE_FIXED)/QUEEN_CHARGE_SPEED);

	}

	/* Now... turn to face. */

	if (queenStatusPointer->moveTimer!=2) {

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		#if 1
		{
			VECTORCH v;
			if(queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock && !queenStatusPointer->TempTarget && queenStatusPointer->TargetDistance>5000)
			{
				//if charging at player don't go in a straight line
				#define QUEEN_COS 63302
				#define QUEEN_SIN 16961
				if(queenStatusPointer->SwerveDirection)
				{
					v.vx=MUL_FIXED(queenStatusPointer->VectToTarget.vx,QUEEN_COS)+MUL_FIXED(queenStatusPointer->VectToTarget.vz,QUEEN_SIN);
					v.vy=0;
					v.vz=MUL_FIXED(queenStatusPointer->VectToTarget.vz,QUEEN_COS)-MUL_FIXED(queenStatusPointer->VectToTarget.vx,QUEEN_SIN);
				}
				else
				{
					v.vx=MUL_FIXED(queenStatusPointer->VectToTarget.vx,QUEEN_COS)-MUL_FIXED(queenStatusPointer->VectToTarget.vz,QUEEN_SIN);
					v.vy=0;
					v.vz=MUL_FIXED(queenStatusPointer->VectToTarget.vz,QUEEN_COS)+MUL_FIXED(queenStatusPointer->VectToTarget.vx,QUEEN_SIN);
				}
			
				queenStatusPointer->SwerveTimer-=NormalFrameTime;
				if(queenStatusPointer->SwerveTimer<=0)
				{
					//alter swerve direction , and keep it for the next .5 to 2.5 seconds
					queenStatusPointer->SwerveTimer=(ONE_FIXED/2)+(FastRandom() & 0xffff)*2;
					queenStatusPointer->SwerveDirection=!queenStatusPointer->SwerveDirection;
				}
			}
			else
			{
				v=queenStatusPointer->VectToTarget;
			}
			NPCOrientateToVector(sbPtr, &v,Queen_Turn_Rate,NULL);

			//NPCOrientateToVector(sbPtr, &queenStatusPointer->VectToTarget,Queen_Turn_Rate,NULL);
		}
		/* Now, just a normal lin velocity. */
		{
			if(queenStatusPointer->moveTimer==1)
			{
				SetQueenMovement_FromFoot(sbPtr);
			}
			else
			{
				VECTORCH velocity;
		
				velocity.vx=sbPtr->DynPtr->OrientMat.mat31;
				velocity.vy=0;
				velocity.vz=sbPtr->DynPtr->OrientMat.mat33;

				if ( (velocity.vx==0) && (velocity.vy==0) && (velocity.vz==0) ) {
					sbPtr->DynPtr->LinVelocity.vx = 0;		
					sbPtr->DynPtr->LinVelocity.vy = 0;
					sbPtr->DynPtr->LinVelocity.vz = 0;
					return;			
				}
			
		
				Normalise(&velocity);
		

				sbPtr->DynPtr->LinVelocity.vx = MUL_FIXED(velocity.vx,QUEEN_CHARGE_SPEED);
				sbPtr->DynPtr->LinVelocity.vy = MUL_FIXED(velocity.vy,QUEEN_CHARGE_SPEED);
				sbPtr->DynPtr->LinVelocity.vz = MUL_FIXED(velocity.vz,QUEEN_CHARGE_SPEED);
			}
		}

		#else
		NPCOrientateToVector(sbPtr, &vectotarget,Queen_Turn_Rate,NULL);
		SetQueenMovement_FromFoot(sbPtr);
		#endif
	}

	/* Consider exit state. */

	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->moveTimer==2)) {
		
		/* We must have finished. */

		queenStatusPointer->current_move=QM_Standby;

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		/* Would you believe the end's in the middle? :-) */

	}
	

	if(ChangingFoot)
	{
		
		QueenCalculateTargetInfo(sbPtr);
		
		if(queenStatusPointer->PlayingHitDelta)
		{
			queenStatusPointer->HModelController.Playing=0;
			queenStatusPointer->current_move=QM_Standby;
			queenStatusPointer->moveTimer=0;
			queenStatusPointer->next_move=QM_Standby;
			return;
		}
		//only check for exiting charge , when changing foot
		{
			/* Only charge if we're facing the right way. */


			if (queenStatusPointer->TargetDirection.vz<queenStatusPointer->TargetDirection.vx || 
				queenStatusPointer->TargetDirection.vz<-queenStatusPointer->TargetDirection.vx)
			{
				/* Spin round a bit more. */
				queenStatusPointer->HModelController.Playing=0;
				queenStatusPointer->current_move=QM_Close;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_Charge;
				return;
			}
		}
		
		if (((queenStatusPointer->TargetDistance<8000 && queenStatusPointer->fixed_foot==LeftFoot && queenStatusPointer->TargetRelSpeed<QUEEN_CLOSE_SPEED) ||queenStatusPointer->next_move!=QM_Standby)&&(queenStatusPointer->moveTimer!=2)) 
		{
			queenStatusPointer->HModelController.Playing=0;
			queenStatusPointer->current_move=queenStatusPointer->next_move;
			queenStatusPointer->moveTimer=0;
			if(queenStatusPointer->current_move==QM_Standby)
			{
				if(queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock && !queenStatusPointer->TempTarget)
					queenStatusPointer->current_move=QM_Close;
				else
					queenStatusPointer->current_move=QM_ComeToPoint;
			}
			queenStatusPointer->next_move=QM_Standby;
			return;
			#if 0
			/* Begin exit. */
			int tweeiningtime=(Queen_Step_Time>>2);
	
			queenStatusPointer->moveTimer=2; /* It's something of a state flag here. */
	
			switch (queenStatusPointer->fixed_foot) {
				case (LeftFoot):
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
						(int)QGSS_Stop_To_Right,(Queen_Step_Time),tweeiningtime);
					break;
				case (RightFoot):
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
						(int)QGSS_Stop_To_Left,(Queen_Step_Time),tweeiningtime);
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
		
			queenStatusPointer->HModelController.LoopAfterTweening=0;
			#endif
		}

		//check to see if queen should change between sprints
		if(queenStatusPointer->fixed_foot==LeftFoot)
		{
			if(DeltaAnimation_IsFinished(queenStatusPointer->attack_delta))
			{
				//switch to full sprint if not already doing it
				if(queenStatusPointer->HModelController.Sub_Sequence!=QGSS_Sprint_Full)
				{
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
						(int)QGSS_Sprint_Full,-1,ONE_FIXED>>3);
					queenStatusPointer->HModelController.LoopAfterTweening=1;
					
				}
			}
			else
			{
				//switch to template sprint if not already doing it
				if(queenStatusPointer->HModelController.Sub_Sequence!=QGSS_Sprint)
				{
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
						(int)QGSS_Sprint,-1,ONE_FIXED>>3);
					queenStatusPointer->HModelController.LoopAfterTweening=1;
					
				}
			}
		}
	}

}

void QueenMove_ButtAttack(STRATEGYBLOCK* sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_ButtAttack);	

	if (queenStatusPointer->moveTimer==0) {
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
				(int)QGSS_ButtConnect,-1,tweeiningtime);
		
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
	}


	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) 
	{
		
		queenStatusPointer->current_move=QM_Standby;
		//finished attack switch back to reconsider
		queenStatusPointer->QueenState=QBS_Reconsider;

		//queen ends this sequence with right foot forward
		SetQueenFoot(sbPtr,RightFoot);
		
	}

	sbPtr->DynPtr->LinVelocity.vx = 0;		
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	

	queenStatusPointer->moveTimer+=NormalFrameTime;
}

void QueenMove_ButtCharge(STRATEGYBLOCK* sbPtr)
{

	QUEEN_STATUS_BLOCK *queenStatusPointer;
	BOOL ChangingFoot=FALSE;

	/* Very different movement function... */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_ButtCharge);

	/* Charge at the player. */
	//queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;

	if (queenStatusPointer->moveTimer==0) 
	{
		/* Do setup. */
		int tweeiningtime=(Queen_Step_Time>>2);

		sbPtr->DynPtr->LinVelocity.vx = 0;
		sbPtr->DynPtr->LinVelocity.vy = 0;
		sbPtr->DynPtr->LinVelocity.vz = 0;
				
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
					(int)QGSS_RunButtAttack,-1,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=1;
				queenStatusPointer->moveTimer=1; /* It's something of a state flag here. */
				
				break;
			case (RightFoot):
				/* Argh! Can't start from right foot! */
				queenStatusPointer->current_move=QM_Close;
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->next_move=QM_Charge;
				return;
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
		return;
	}

	/* Check for change foot? */

	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) {
		if (queenStatusPointer->HModelController.keyframe_flags) 
		{
			if (queenStatusPointer->HModelController.keyframe_flags & 1) 
			{
				SetQueenFoot(sbPtr,LeftFoot);
				ChangingFoot=TRUE;
			}
			if (queenStatusPointer->HModelController.keyframe_flags & 2) 
			{
				SetQueenFoot(sbPtr,RightFoot);
				ChangingFoot=TRUE;
			}
			if(queenStatusPointer->moveTimer==1) queenStatusPointer->moveTimer=3;
		}
		HModel_SetToolsRelativeSpeed(&queenStatusPointer->HModelController,(512*ONE_FIXED)/QUEEN_BUTTCHARGE_SPEED);
	}

	/* Now... turn to face. */

	if (queenStatusPointer->moveTimer!=2) {

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		ProveHModel(dPtr->HModelControlBlock,dPtr);

		NPCOrientateToVector(sbPtr, &queenStatusPointer->VectToTarget,Queen_Turn_Rate,NULL);
		
		
		/* Now, just a normal lin velocity. */
		if(queenStatusPointer->moveTimer==1)
		{
			SetQueenMovement_FromFoot(sbPtr);
		}
		else
		{
			VECTORCH velocity;
		
			velocity.vx=sbPtr->DynPtr->OrientMat.mat31;
			velocity.vy=0;
			velocity.vz=sbPtr->DynPtr->OrientMat.mat33;

			if ( (velocity.vx==0) && (velocity.vy==0) && (velocity.vz==0) ) {
				sbPtr->DynPtr->LinVelocity.vx = 0;		
				sbPtr->DynPtr->LinVelocity.vy = 0;
				sbPtr->DynPtr->LinVelocity.vz = 0;
				return;			
			}
			
		
			Normalise(&velocity);
		
			//runSpeed=DIV_FIXED(Queen_Charge_Step_Speed,Queen_ButtCharge_Rate);

			sbPtr->DynPtr->LinVelocity.vx = MUL_FIXED(velocity.vx,QUEEN_BUTTCHARGE_SPEED);
			sbPtr->DynPtr->LinVelocity.vy = MUL_FIXED(velocity.vy,QUEEN_BUTTCHARGE_SPEED);
			sbPtr->DynPtr->LinVelocity.vz = MUL_FIXED(velocity.vz,QUEEN_BUTTCHARGE_SPEED);
		}

	}

	

	if(ChangingFoot)
	{
		
		int range;
		QueenCalculateTargetInfo(sbPtr);
		range=queenStatusPointer->TargetDistance;
		//only check for exiting charge , when changing foot
		
		if(queenStatusPointer->PlayingHitDelta)
		{
			queenStatusPointer->HModelController.Playing=0;
			queenStatusPointer->current_move=QM_Standby;
			queenStatusPointer->moveTimer=0;
			queenStatusPointer->next_move=QM_Standby;
			return;
		}

		if(PlayerInLocker)
		{
			//stop using butt charge , since the queen can't actually get to the player
			queenStatusPointer->moveTimer=0;
			queenStatusPointer->current_move=QM_Charge;
			queenStatusPointer->next_move=QM_Standby;
			return;

		}
		
		
		if (queenStatusPointer->TargetDirection.vz<queenStatusPointer->TargetDirection.vx || 
			queenStatusPointer->TargetDirection.vz<-queenStatusPointer->TargetDirection.vx)
		{
			/* Spin round a bit more. */
			queenStatusPointer->HModelController.Playing=0;
			queenStatusPointer->current_move=QM_Close;
			queenStatusPointer->moveTimer=0;
			queenStatusPointer->next_move=QM_Standby;
			return;
		}
		

		if (queenStatusPointer->moveTimer!=2 && queenStatusPointer->next_move!=QM_Standby) 
		{
			queenStatusPointer->HModelController.Playing=0;
			queenStatusPointer->current_move=queenStatusPointer->next_move;
			queenStatusPointer->next_move=QM_Standby;
			queenStatusPointer->moveTimer=0;
			return;
		} 
	}


}

void QueenMove_Climb(STRATEGYBLOCK* sbPtr)
{
	
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	GLOBALASSERT(queenStatusPointer->current_move==QM_Climbing);

	if (queenStatusPointer->moveTimer==0) 
	{
		//just starting to climb out , so start the animation sequence
		int tweeiningtime=(Queen_Step_Time>>2);
		SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,
			(int)QGSS_ClimbOut,-1,tweeiningtime);

		queenStatusPointer->HModelController.Playing=1;
		
		//set the start position of this maneuver
		queenStatusPointer->ClimbStartPosition=sbPtr->DynPtr->Position;

		//while climbing out queen needs to ignore gravity and collisions
		sbPtr->DynPtr->GravityOn=0;
		sbPtr->DynPtr->OnlyCollideWithObjects=1;	
	
		/*moveTimer is being used as a state flag again*/
		queenStatusPointer->moveTimer=1;
	}

	/*Adjust the queen's facing*/
	{
		VECTORCH direction={-ONE_FIXED,0,0};
		NPCOrientateToVector(sbPtr, &direction,Queen_Turn_Rate,NULL);
	}

	
	/*If the queen has stopped tweening , then we need to deal with her movement*/
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
	{
		#define QueenClimbTime1 7598
		#define QueenClimbTime2 16146
		#define QueenClimbTime3 42740
		
		ProveHModel(sbPtr->SBdptr->HModelControlBlock,sbPtr->SBdptr);
		{
			VECTORCH StageOneMovement={0,-6771,0};		
			VECTORCH StageTwoMovement={-3266,0,-372};		

			//work out where the queen should be
			VECTORCH newPosition=queenStatusPointer->ClimbStartPosition;
			int timer=queenStatusPointer->HModelController.sequence_timer;

			if(timer<QueenClimbTime1)
			{
				//no movement
			}
			else if(timer<QueenClimbTime2)
			{
				int scale=DIV_FIXED(timer-QueenClimbTime1,QueenClimbTime2-QueenClimbTime1);

				newPosition.vx+=MUL_FIXED(StageOneMovement.vx,scale);
				newPosition.vy+=MUL_FIXED(StageOneMovement.vy,scale);
				newPosition.vz+=MUL_FIXED(StageOneMovement.vz,scale);
			}
			else if(timer<QueenClimbTime3)
			{
				int scale=DIV_FIXED(timer-QueenClimbTime2,QueenClimbTime3-QueenClimbTime2);
				AddVector(&StageOneMovement,&newPosition);
				
				newPosition.vx+=MUL_FIXED(StageTwoMovement.vx,scale);
				newPosition.vy+=MUL_FIXED(StageTwoMovement.vy,scale);
				newPosition.vz+=MUL_FIXED(StageTwoMovement.vz,scale);

			}							 
			else
			{
				AddVector(&StageOneMovement,&newPosition);
				AddVector(&StageTwoMovement,&newPosition);
			}

			sbPtr->DynPtr->Displacement=newPosition;
			SubVector(&sbPtr->DynPtr->Position,&sbPtr->DynPtr->Displacement);
		}
	
		if(queenStatusPointer->HModelController.sequence_timer>62000)
		{
			//the queen has finished getting out
			queenStatusPointer->current_move=QM_Standby;
			queenStatusPointer->next_move=QM_Standby;
			queenStatusPointer->moveTimer=0;

			QueenForceReconsider(sbPtr);

			sbPtr->DynPtr->GravityOn=1;
			sbPtr->DynPtr->OnlyCollideWithObjects=0;	

			//the queen ends in right stance
			queenStatusPointer->fixed_foot=RightFoot;
			
		}
		
	}

	
}


#define Queen_Swipe_Left 0
#define Queen_Swipe_Right 1
#define Queen_Swipe_Left_Low 2
#define Queen_Swipe_Right_Low 3

void Queen_Do_Swipe(STRATEGYBLOCK *sbPtr,int side)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	VECTORCH vectohand,targetpos;
	int range_to_player;
	SECTION_DATA *hand_section;


	
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	/*Is the queen already doing a swipe?*/
	if ((queenStatusPointer->attack_delta->timer==(ONE_FIXED-1))
		||(queenStatusPointer->attack_delta->timer==0)) 
	{
		/* She isn't , so start it */
		switch (side)
		{
			case Queen_Swipe_Left :
				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftSwipe,ONE_FIXED);
				break;

			case Queen_Swipe_Right :
				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_RightSwipe,ONE_FIXED);
				break;

			case Queen_Swipe_Left_Low :
				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftSwipe_Low,ONE_FIXED);
				break;

			case Queen_Swipe_Right_Low :
				Start_Delta_Sequence(queenStatusPointer->attack_delta,
					(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_RightSwipe_Low,ONE_FIXED);
				break;
			
			
			default :
				LOCALASSERT(1==0);
			
		}
		queenStatusPointer->attack_delta->Playing=1;
		queenStatusPointer->attack_delta->Looped=0;

		queenStatusPointer->AttackDoneItsDamage=FALSE;
	
	}

	if(queenStatusPointer->attack_delta->timer>37000 && !queenStatusPointer->AttackDoneItsDamage)
	{
		queenStatusPointer->AttackDoneItsDamage=TRUE;
		
		//get the hand corresponding to the currently playing swipe
		switch(queenStatusPointer->attack_delta->sub_sequence)
		{
			case QRSTSS_RightSwipe:
			case QRSTSS_RightSwipe_Low:
				hand_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,"right hand");
				break;
			
			case QRSTSS_LeftSwipe:
			case QRSTSS_LeftSwipe_Low:
				hand_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,"left hand");
				break;

			default :
				LOCALASSERT(1==0);
		}
			

		GetTargetingPointOfObject(Player,&targetpos);
		vectohand.vx=targetpos.vx-hand_section->World_Offset.vx;
		vectohand.vy=0;//targetpos.vy-hand_section->World_Offset.vy;
		vectohand.vz=targetpos.vz-hand_section->World_Offset.vz;

		range_to_player=Approximate3dMagnitude(&vectohand);
	
		//see if queen hit an intervening object
		{
			VECTORCH direction;

			direction.vx=targetpos.vx-hand_section->World_Offset.vx;
			direction.vy=targetpos.vy-hand_section->World_Offset.vy;
			direction.vz=targetpos.vz-hand_section->World_Offset.vz;

			Normalise(&direction);
			
			LOS_ObjectHitPtr=0;
			FindPolygonInLineOfSight(&direction,&hand_section->World_Offset,1,sbPtr->SBdptr);
	
			if(LOS_ObjectHitPtr)
			{
				if(LOS_ObjectHitPtr->ObStrategyBlock)
				{
					if(LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourInanimateObject || 
					   LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourTrackObject)
					{
						//damage the object instead of the player
						//if(LOS_Lambda<QueenAttackRange)
						{
							CauseDamageToObject(LOS_ObjectHitPtr->ObStrategyBlock,&TemplateAmmo[AMMO_NPC_PAQ_CLAW].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
						}
						return;
					}
				}
			}
		}
		
		/* ATM, target is always player. */

		if (range_to_player<QueenAttackRange) 
		{
			//do from .75 to 1.25 times base damage
			CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_NPC_PAQ_CLAW].MaxDamage[AvP.Difficulty],(ONE_FIXED*.75)+(FastRandom()&0x7fff),NULL);
			//set the taunt timer
			queenStatusPointer->QueenTauntTimer=ONE_FIXED/2;						
		}
	}

}


void QueenMove_Close(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	/* First movement function. */

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	GLOBALASSERT(queenStatusPointer->current_move==QM_Close);

	if (queenStatusPointer->moveTimer==0) 
	{
		
		/* Do setup. */
		int tweeiningtime=(ONE_FIXED>>3);
				
		sbPtr->DynPtr->LinVelocity.vx = 0;
		sbPtr->DynPtr->LinVelocity.vy = 0;
		sbPtr->DynPtr->LinVelocity.vz = 0;
		
		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenLeftStanceTemplate,
					(int)QLSTSS_Forward_L2R,-1,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=0;

				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,RightFoot);
				SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenRightStanceTemplate,
					(int)QRSTSS_Forward_R2L,-1,tweeiningtime);
				queenStatusPointer->HModelController.LoopAfterTweening=0;
				
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
		/* Go! */
		queenStatusPointer->moveTimer++;
		GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
		return;
	}
	
	if (queenStatusPointer->HModelController.Tweening==Controller_NoTweening) 
	{
		BOOL moveHalfSpeed=FALSE;
		if (queenStatusPointer->HModelController.Looped!=0) {
			/* Quirkafleeg */
			GLOBALASSERT(queenStatusPointer->HModelController.Looped==0);
		}

		QueenCalculateTargetInfo(sbPtr);
		//if the queen is close to her target , and the target is not in front of her
		//then she needs to slow down to reduce her turning circle
		if(queenStatusPointer->TargetDistance<3000)
		{
			if(queenStatusPointer->TargetDirection.vz<queenStatusPointer->TargetDirection.vx ||
			   queenStatusPointer->TargetDirection.vz<-queenStatusPointer->TargetDirection.vx)
			{
				moveHalfSpeed=TRUE;
			}
		}
		if(moveHalfSpeed)
			HModel_SetToolsRelativeSpeed(&queenStatusPointer->HModelController,(512*ONE_FIXED)/(QUEEN_CLOSE_SPEED/2));
		else
			HModel_SetToolsRelativeSpeed(&queenStatusPointer->HModelController,(512*ONE_FIXED)/QUEEN_CLOSE_SPEED);

	}


	/* Now... turn to face. */

	{

		DISPLAYBLOCK *dPtr;

		dPtr=sbPtr->SBdptr;
		GLOBALASSERT(dPtr);

		NPCOrientateToVector(sbPtr,&queenStatusPointer->VectToTarget,Queen_Turn_Rate,NULL);

		#if 1
		SetQueenMovement_FromFoot(sbPtr);

		#else
		{
			VECTORCH velocity;
			int dotProduct;
			int runSpeed;
		
			velocity.vx=sbPtr->DynPtr->OrientMat.mat31;
			velocity.vy=0;
			velocity.vz=sbPtr->DynPtr->OrientMat.mat33;

			if ( (velocity.vx==0) && (velocity.vy==0) && (velocity.vz==0) ) {
				sbPtr->DynPtr->LinVelocity.vx = 0;		
				sbPtr->DynPtr->LinVelocity.vy = 0;
				sbPtr->DynPtr->LinVelocity.vz = 0;
				return;			
			}
			
		
			Normalise(&velocity);
		

			sbPtr->DynPtr->LinVelocity.vx = MUL_FIXED(velocity.vx,QUEEN_CLOSE_SPEED);
			sbPtr->DynPtr->LinVelocity.vy = MUL_FIXED(velocity.vy,QUEEN_CLOSE_SPEED);
			sbPtr->DynPtr->LinVelocity.vz = MUL_FIXED(velocity.vz,QUEEN_CLOSE_SPEED);
		}
		#endif

	}

	queenStatusPointer->moveTimer+=NormalFrameTime;
	
	if ((queenStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(queenStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		queenStatusPointer->current_move=QM_Standby;

		switch (queenStatusPointer->fixed_foot) {
			case (LeftFoot):
				SetQueenFoot(sbPtr,RightFoot);
				break;
			case (RightFoot):
				SetQueenFoot(sbPtr,LeftFoot);
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
		queenStatusPointer->moveTimer=0;
		queenStatusPointer->current_move=queenStatusPointer->next_move;
		queenStatusPointer->next_move=QM_Standby;

	}


}

void QueenIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming, VECTORCH *point) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	GLOBALASSERT(sbPtr);
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	GLOBALASSERT(queenStatusPointer);

	/* Ouch. */
	if (sbPtr->SBDamageBlock.Health <= 0) {
		if (queenStatusPointer->QueenState!=QBS_Dead) {
			KillQueen(sbPtr,damage,multiple,Section,incoming);
		}
		return;
	}

	//scream a bit
	QueenSoundHurt(sbPtr);

	GLOBALASSERT(damage);
	if(damage->ExplosivePower && incoming)
	{
		/*
		Don't allow the queen to be stunned if she is climbing out of the airlock.
		This would screw up the sequence to much.
		*/
		if(queenStatusPointer->current_move!=QM_Climbing)
		{
			if(!QueenPlayingStunAnimation(&queenStatusPointer->HModelController))
			{
				if((damage->ExplosivePower==2)||(damage->ExplosivePower==6))
				{
					//big explosion, play fall over anim
					SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,(int)QGSS_Explosion_Stun,-1,ONE_FIXED>>3);
					queenStatusPointer->HModelController.LoopAfterTweening=0;

					queenStatusPointer->current_move=QM_Stun;

					queenStatusPointer->PlayingHitDelta=TRUE;
					QueenForceReconsider(sbPtr);

					//the queen ends in right stance
					queenStatusPointer->fixed_foot=RightFoot;
					
				}
				else
				{
					//play a hit delta if not already doing so
					if ((queenStatusPointer->hit_delta->timer==(ONE_FIXED-1))
						||(queenStatusPointer->hit_delta->timer==0)) 
					{
						VECTORCH dir=*incoming;
						MATRIXCH WtoL=sbPtr->DynPtr->OrientMat;
						TransposeMatrixCH(&WtoL);
						RotateVector(&dir,&WtoL);

						if(dir.vx>0)
						{
							Start_Delta_Sequence(queenStatusPointer->hit_delta,
								(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_RightHit,-1);
							queenStatusPointer->hit_delta->Playing=1;
						}
						else
						{
							Start_Delta_Sequence(queenStatusPointer->hit_delta,
								(int)HMSQT_QueenRightStanceTemplate,(int)QRSTSS_LeftHit,-1);
							queenStatusPointer->hit_delta->Playing=1;
						}

						queenStatusPointer->PlayingHitDelta=TRUE;
						QueenForceReconsider(sbPtr);
						
	
					}
				}
			}
		}
	}

	if(AvP.PlayerType==I_Marine && Section)
	{
		//don't allow the marine to completely destroy a section
		if(Section->current_damage.Health<=0)
		{
			Section->current_damage.Health=1;
		}
	}

}

void Execute_Queen_Dying(STRATEGYBLOCK *sbPtr) {

	QUEEN_STATUS_BLOCK *queenStatusPointer;

	GLOBALASSERT(sbPtr);
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	GLOBALASSERT(queenStatusPointer);

	/* Here for completeness.  Queens never melt away. */

}

void KillQueen(STRATEGYBLOCK *sbPtr,DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming)
{	
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	int tkd;

	/* Oh my God!  They've killed Queenie! */

	GLOBALASSERT(sbPtr);
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	GLOBALASSERT(queenStatusPointer);

	/* make a sound.  Just this once without a check. */
	Sound_Play(SID_ALIEN_KILL,"d",&sbPtr->DynPtr->Position);

	/*If queen has a death target ,send a request*/
	if(queenStatusPointer->death_target_sbptr)
	{
		RequestState(queenStatusPointer->death_target_sbptr,queenStatusPointer->death_target_request, 0);
	}
	/* Queens never gibb, they're that hard. */
	
	tkd=TotalKineticDamage(damage);

	/* Deal with sequence. */
	RemoveAllDeltas(&queenStatusPointer->HModelController);

	if (tkd>200) {
		//SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,QGSS_Explode_Death,(ONE_FIXED),(ONE_FIXED>>2));
		/* That death doesn't work. */
		SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,QGSS_FaceDeath,(ONE_FIXED),(ONE_FIXED>>2));
	} else {
		SetQueenShapeAnimSequence_Core(sbPtr,HMSQT_QueenGeneral,QGSS_FaceDeath,(ONE_FIXED),(ONE_FIXED>>2));
	}

	//if the queen is carrying an object , release it and reset gravity on it
	if(queenStatusPointer->QueenState==QBS_CarryingObject)
	{
		if(!QueenObjectList[queenStatusPointer->CurrentQueenObject]->SBflags.destroyed_but_preserved)
		{
			QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->GravityOn=1;
			QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OnlyCollideWithEnvironment=0;	
			queenStatusPointer->CurrentQueenObject=-1;
		}
	}
	
	
	/* More restrained death than before... */
	{

		queenStatusPointer->QueenState=QBS_Dead;

		/* stop motion */
		LOCALASSERT(sbPtr->DynPtr);
		sbPtr->DynPtr->Friction	= 400000;
		sbPtr->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->LinVelocity.vx;
		sbPtr->DynPtr->LinImpulse.vy+=sbPtr->DynPtr->LinVelocity.vy;
		sbPtr->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->LinVelocity.vz;
		sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
		sbPtr->DynPtr->IgnoreSameObjectsAsYou = 1;
		/* Experiment... */
		sbPtr->DynPtr->UseStandardGravity=1;

	}


}


void FindQueenObjects()
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	
	
	NumQueenObjects=0;
		
	//search through all the strategies for the throwable objects
	//also look for the special airlock doors
	UpperAirlockDoorSbptr=0;
	LowerAirlockDoorSbptr=0;
	LockerDoorSbptr=0;
	UpperAirlockDoorOpen=FALSE;
	LowerAirlockDoorOpen=FALSE;

	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		if(sbPtr->name)
		{	
			if(strstr(sbPtr->name,"QueenAmmo"))
			{
				if(NumQueenObjects<QUEEN_MAX_OBJECT)
				{
					QueenObjectList[NumQueenObjects++]=sbPtr;	
					GLOBALASSERT(sbPtr->DynPtr);
					sbPtr->SBflags.preserve_until_end_of_level=1;
				}
			}
			else if(!strcmp(sbPtr->name,"a1door"))
			{
				UpperAirlockDoorSbptr=sbPtr;	
				UpperAirlockDoorStart=sbPtr->DynPtr->Position;
			}
			else if(!strcmp(sbPtr->name,"a3door"))
			{
				LowerAirlockDoorSbptr=sbPtr;	
				LowerAirlockDoorStart=sbPtr->DynPtr->Position;
			}
			else if(!strcmp(sbPtr->name,"locker door"))
			{
				LockerDoorSbptr=sbPtr;	
				sbPtr->SBflags.preserve_until_end_of_level=1;
			}
		}
	}
}


BOOL CalculateTrajectory(VECTORCH* start,VECTORCH* dest,VECTORCH* velocity,int obj_speed,VECTORCH* result)
{
	VECTORCH normal;
	VECTORCH rotated_vel;
	VECTORCH rotated_result;
	int closing_speed;
	int distance;
	int vertical_distance;
	int time_to_target;
	
	//get a normalised vector from start to destination
	normal=*dest;
	SubVector(start,&normal);

	vertical_distance=normal.vy-1000;
	normal.vy=0;	
	
	if(!normal.vx && !normal.vz)
		return FALSE;

	distance=Magnitude(&normal);
		
	Normalise(&normal);

	//apply rotation to velocity that would rotate the normal to (ONE_FIXED,0,0)
	rotated_vel.vx=MUL_FIXED(velocity->vx,normal.vx)+MUL_FIXED(velocity->vz,normal.vz);
	rotated_vel.vy=0;
	rotated_vel.vz=MUL_FIXED(velocity->vx,-normal.vz)+MUL_FIXED(velocity->vz,normal.vx);

	if(rotated_vel.vz>=obj_speed || -rotated_vel.vz>=obj_speed)
	{
		//no hope of hitting
		return FALSE;
	}

	rotated_result.vz=rotated_vel.vz;
	
	//calculate x component using floats
	{
		float z=(float)rotated_result.vz;
		float speed=(float)obj_speed;

		float x=sqrt(speed*speed-z*z);

		rotated_result.vx=x;
	}

	closing_speed=rotated_result.vx-rotated_vel.vx;
	if(closing_speed<=0)
	{
		//can't hit
		return FALSE;
	}

	time_to_target=DIV_FIXED(distance,closing_speed);
	
	if(time_to_target>(3*ONE_FIXED))
	{
		//take to long to hit target
		return FALSE;
	}

	//rotate result back
	result->vx=MUL_FIXED(rotated_result.vx,normal.vx)+MUL_FIXED(rotated_result.vz,-normal.vz);
	result->vy=0;
	result->vz=MUL_FIXED(rotated_result.vx,normal.vz)+MUL_FIXED(rotated_result.vz,normal.vx);

	//calculate required up component
	//u=s/t - a*t/2
	result->vy=DIV_FIXED(vertical_distance,time_to_target)-MUL_FIXED(time_to_target,GRAVITY_STRENGTH/2);

	//we have a targeting solution.
	return TRUE;
}


//calculate target's relative position
static void QueenCalculateTargetInfo(STRATEGYBLOCK *sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	GLOBALASSERT(sbPtr);

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	GLOBALASSERT(queenStatusPointer);

	if(queenStatusPointer->TargetInfoValid) return;

	{
		MATRIXCH WtoL;

		//get vector from queen to target
		queenStatusPointer->VectToTarget=queenStatusPointer->TargetPos;
		SubVector(&sbPtr->DynPtr->Position,&queenStatusPointer->VectToTarget);

		queenStatusPointer->VectToTarget.vy=0;

		//get length of vector, and then normalise it
		queenStatusPointer->TargetDistance=Approximate3dMagnitude(&queenStatusPointer->VectToTarget);
		Normalise(&queenStatusPointer->VectToTarget);
		
		//rotate vector to queen's local space to get relative direction
		queenStatusPointer->TargetDirection=queenStatusPointer->VectToTarget;
		WtoL=sbPtr->DynPtr->OrientMat;
		TransposeMatrixCH(&WtoL);
		RotateVector(&queenStatusPointer->TargetDirection,&WtoL);



		queenStatusPointer->TargetRelSpeed=0;
		if(!queenStatusPointer->TempTarget && queenStatusPointer->QueenTargetSB)
		{
			if(queenStatusPointer->QueenTargetSB->DynPtr)
			{
				VECTORCH facing;
				facing.vx=sbPtr->DynPtr->OrientMat.mat31;
				facing.vy=sbPtr->DynPtr->OrientMat.mat32;
				facing.vz=sbPtr->DynPtr->OrientMat.mat33;


				queenStatusPointer->TargetRelSpeed=DotProduct(&facing,&queenStatusPointer->QueenTargetSB->DynPtr->LinVelocity);		
			}
		}
	}

	queenStatusPointer->TargetInfoValid=TRUE;
}

void QueenForceReconsider(STRATEGYBLOCK* sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	if(queenStatusPointer->QueenState==QBS_Reconsider) return;

	if(queenStatusPointer->QueenState==QBS_CarryingObject)
	{
		//need to drop the object
		if(queenStatusPointer->CurrentQueenObject!=-1)
		{
			//if the object has been destroyed anywa, don't need to worry about dropping it
			if(!QueenObjectList[queenStatusPointer->CurrentQueenObject]->SBflags.destroyed_but_preserved)
			{
				//put gravity back on
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->GravityOn=1;
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OnlyCollideWithEnvironment=0;	
				
				{
					MATRIXCH id_mat={ONE_FIXED,0,0,0,ONE_FIXED,0,0,0,ONE_FIXED};
					QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OrientMat=id_mat;

				}
				
				//give the object a slight impulse away from the queen
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->OrientMat.mat31/10;
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->OrientMat.mat33/10;
			}
		}
	}

	queenStatusPointer->CurrentQueenObject=-1;
	queenStatusPointer->QueenState=QBS_Reconsider;
	queenStatusPointer->QueenTargetSB=Player->ObStrategyBlock;
	queenStatusPointer->TempTarget=FALSE;
	queenStatusPointer->TargetInfoValid=FALSE;
	queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;					
	
}


#define AirlockMinX 33882
#define AirlockMaxX 45542
#define AirlockMinZ -13936
#define AirlockMaxZ -6266
#define AirlockCentreX ((AirlockMinX+AirlockMaxX)/2)
#define AirlockCentreZ ((AirlockMinZ+AirlockMaxZ)/2)
#define AirlockY   12000
#define HangarFloorLevel 3950

#define AirlockOpeningDistance 2500

#define AirlockOffset 1000
void QueenCheckForAvoidAirlock(STRATEGYBLOCK *sbPtr)
{
	//only need to look out for the airlock in hangar
	if(!stricmp(LevelName,"hangar"))
	{
		QUEEN_STATUS_BLOCK *queenStatusPointer =(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);
		VECTORCH* qpos=&sbPtr->DynPtr->Position;
		VECTORCH* tpos=&queenStatusPointer->TargetPos;

		if(!UpperAirlockDoorOpen) return;
		if(PlayerInTrench) return;

		if(queenStatusPointer->QueenState==QBS_ClimbingOutOfAirlock)
		{
			//not much point in trying to avoid airlock now!
			return;
		}
		if(!queenStatusPointer->BeenInAirlock)
		{
			//if the queen hasn't fallen in yet , then she isn't careful while
			//charging at the player
			if(queenStatusPointer->current_move==QM_ButtCharge)
			{
				return;
			}
		}
		
		if(qpos->vx>AirlockMaxX-1000)
		{
			if(tpos->vx<AirlockMaxX-1000)
			{
				//queen and target are on opposite sides of the max X side
				//find out where the queen's path will intersect the side
				int scale=DIV_FIXED((AirlockMaxX-1000)-qpos->vx,tpos->vx-qpos->vx);
				int zintercept=MUL_FIXED(tpos->vz-qpos->vz,scale)+qpos->vz;

				if(zintercept>=AirlockMinZ && zintercept<=AirlockMaxZ)
				{
								
//					textprint("Airlock Max X\n");
//					return;
					//head for the corner that is closest to the current target
					queenStatusPointer->TargetPos.vx=AirlockMaxX;
					if(abs(AirlockMinZ-tpos->vz)<abs(AirlockMaxZ-tpos->vz))
					{
						queenStatusPointer->TargetPos.vz=AirlockMinZ;
					}
					else
					{
						queenStatusPointer->TargetPos.vz=AirlockMaxZ;
					}
					queenStatusPointer->TempTarget=TRUE;
					queenStatusPointer->TempTargetTimer=ONE_FIXED;
					queenStatusPointer->TargetInfoValid=FALSE;
					return;
				}
			}
		}
		else if(qpos->vx<AirlockMinX+1000)
		{
			if(tpos->vx>AirlockMinX+1000)
			{
				//queen and target are on opposite sides of the min X side
				//find out where the queen's path will intersect the side
				int scale=DIV_FIXED(AirlockMinX+1000-qpos->vx,tpos->vx-qpos->vx);
				int zintercept=MUL_FIXED(tpos->vz-qpos->vz,scale)+qpos->vz;

				if(zintercept>=AirlockMinZ && zintercept<=AirlockMaxZ)
				{
//					textprint("Airlock Min X\n");
//					return;
					//head for the corner that is closest to the current target
					queenStatusPointer->TargetPos.vx=AirlockMinX;
					if(abs(AirlockMinZ-tpos->vz)<abs(AirlockMaxZ-tpos->vz))
					{
						queenStatusPointer->TargetPos.vz=AirlockMinZ;
					}
					else
					{
						queenStatusPointer->TargetPos.vz=AirlockMaxZ;
					}
					queenStatusPointer->TempTarget=TRUE;
					queenStatusPointer->TempTargetTimer=ONE_FIXED;
					queenStatusPointer->TargetInfoValid=FALSE;
					return;
				}
			}
		}
	
		if(qpos->vz>AirlockMaxZ-1000)
		{
			if(tpos->vz<AirlockMaxZ-1000)
			{
				//queen and target are on opposite sides of the max Z side
				//find out where the queen's path will intersect the side
				int scale=DIV_FIXED((AirlockMaxZ-1000)-qpos->vz,tpos->vz-qpos->vz);
				int xintercept=MUL_FIXED(tpos->vx-qpos->vx,scale)+qpos->vx;

				if(xintercept>=AirlockMinX && xintercept<=AirlockMaxX)
				{
//					textprint("Airlock Max Z\n");
//					return;
					//head for the corner that is closest to the current target
					queenStatusPointer->TargetPos.vz=AirlockMaxZ;
					if(abs(AirlockMinX-tpos->vx)<abs(AirlockMaxX-tpos->vx))
					{
						queenStatusPointer->TargetPos.vx=AirlockMinX;
					}
					else
					{
						queenStatusPointer->TargetPos.vx=AirlockMaxX;
					}
					queenStatusPointer->TempTarget=TRUE;
					queenStatusPointer->TempTargetTimer=ONE_FIXED;
					queenStatusPointer->TargetInfoValid=FALSE;
					return;
				}
			}
		}
		else if(qpos->vz<AirlockMinZ+1000)
		{
			if(tpos->vz>AirlockMinZ+1000)
			{
				//queen and target are on opposite sides of the min Z side
				//find out where the queen's path will intersect the side
				int scale=DIV_FIXED(AirlockMinZ+1000-qpos->vz,tpos->vz-qpos->vz);
				int xintercept=MUL_FIXED(tpos->vx-qpos->vx,scale)+qpos->vx;

				if(xintercept>=AirlockMinX && xintercept<=AirlockMaxX)
				{
//					textprint("Airlock Min Z\n");
//					return;
					//head for the corner that is closest to the current target
					queenStatusPointer->TargetPos.vz=AirlockMinZ;
					if(abs(AirlockMinX-tpos->vx)<abs(AirlockMaxX-tpos->vx))
					{
						queenStatusPointer->TargetPos.vx=AirlockMinX;
					}
					else
					{
						queenStatusPointer->TargetPos.vx=AirlockMaxX;
					}
					queenStatusPointer->TempTarget=TRUE;
					queenStatusPointer->TempTargetTimer=ONE_FIXED;
					queenStatusPointer->TargetInfoValid=FALSE;
					return;
				}
			}
		}
  		textprint("Airlock not in the way\n");
	}
}

#define HangarLockerMinX 17104
#define HangarLockerMaxX 19680
#define HangarLockerMinZ -32700
#define HangarLockerMaxZ -29661
#define HangarLockerCentreZ ((HangarLockerMinZ+HangarLockerMaxZ)/2)


BOOL ObjectIsInAirlock(STRATEGYBLOCK* sbPtr)
{
	GLOBALASSERT(sbPtr);
	GLOBALASSERT(sbPtr->DynPtr);
	
	if(sbPtr->DynPtr->Position.vx>AirlockMinX && sbPtr->DynPtr->Position.vx<AirlockMaxX &&
	   sbPtr->DynPtr->Position.vz>AirlockMinZ && sbPtr->DynPtr->Position.vz<AirlockMaxZ &&
	   sbPtr->DynPtr->Position.vy>(HangarFloorLevel+1000))
	{
		return TRUE;
	}
	return FALSE;

}

void QueenPickupTargetObject(STRATEGYBLOCK *sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	//queen must be going for an object
	if(queenStatusPointer->QueenState!=QBS_GoingForObject) return;

	//disable gravity for object , while it is being carried
	queenStatusPointer->QueenTargetSB->DynPtr->GravityOn=0;
	//also stop it colliding with the queen
	queenStatusPointer->QueenTargetSB->DynPtr->OnlyCollideWithEnvironment=1;

	//change queen's state
	queenStatusPointer->QueenState=QBS_CarryingObject;
	queenStatusPointer->QueenStateTimer=0;
									
	//now heading for the player
	queenStatusPointer->QueenTargetSB=Player->ObStrategyBlock;
	queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;					
	queenStatusPointer->TargetInfoValid=FALSE;
	queenStatusPointer->next_move=QM_Close;
	
}

void QueenBehaviour(STRATEGYBLOCK *sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	int DistanceToPlayer;
	BOOL JumpDesired=FALSE;
	BOOL ConsiderJumping=FALSE;
	
	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	
	if(NumQueenObjects==-1)
	{
		//first frame...
		//find all objects that the queen can chuck
		FindQueenObjects();
		queenStatusPointer->CurrentQueenObject=-1;
		//get the queens hand section
		queenStatusPointer->QueenRightHand=GetThisSectionData(queenStatusPointer->HModelController.section_data,"rrrr fing");
		GLOBALASSERT(queenStatusPointer->QueenRightHand);
	}

	if(!queenStatusPointer->QueenActivated)
	{
		//is the queen in a visible module?
		GLOBALASSERT(sbPtr->containingModule);
		if(ModuleCurrVisArray[sbPtr->containingModule->m_index]==2)
		{
			queenStatusPointer->QueenActivated=TRUE;
		}
		else
		{
			return;
		}
	}


	HandleHangarAirlock();	
	

	if(queenStatusPointer->QueenState==QBS_Dead)
	{
		//check for victory on marine level.
		if(!stricmp(LevelName,"hangar"))
		{
			//is the player in the locker ('hangar')
			if(Player->ObWorld.vx> HangarLockerMinX && Player->ObWorld.vx < HangarLockerMaxX &&
			   Player->ObWorld.vz> HangarLockerMinZ && Player->ObWorld.vz < HangarLockerMaxZ)	
				PlayerInLocker=TRUE;
			else
				PlayerInLocker=FALSE;

			//In the marine level the player can only win if he is inside the locker room area ,and 
			//the locker door is shut
			if((PlayerInLocker && LockerDoorIsClosed()) ||
			   !(LowerAirlockDoorOpen && UpperAirlockDoorOpen))	
			{
				AvP.LevelCompleted=1;
			}
		} 

		
		Execute_Queen_Dying(sbPtr);
		return;
	}

			/*----------------------------------**
			** 	Check state of delta animations **
			**----------------------------------*/
	if(queenStatusPointer->PlayingHitDelta)
	{
		//check to see if the queen is still stunned from an explosion
		if (!QueenPlayingStunAnimation(&queenStatusPointer->HModelController) && DeltaAnimation_IsFinished(queenStatusPointer->hit_delta)) 
		{
			queenStatusPointer->hit_delta->Playing=0;
			queenStatusPointer->hit_delta->timer=0;
			
			queenStatusPointer->PlayingHitDelta=FALSE;
			queenStatusPointer->current_move=QM_Close;
			queenStatusPointer->next_move=QM_Standby;
			queenStatusPointer->moveTimer=0;
		}
	}

	if(DeltaAnimation_IsFinished(queenStatusPointer->attack_delta))
	{
		queenStatusPointer->attack_delta->Playing=0;
		queenStatusPointer->attack_delta->timer=0;
	}
	
	queenStatusPointer->TargetInfoValid=FALSE;

	if(queenStatusPointer->CurrentQueenObject!=-1)
	{
		if(QueenObjectList[queenStatusPointer->CurrentQueenObject]->SBflags.destroyed_but_preserved)
		{
			//the object that the queen was going for has been destroyed
			queenStatusPointer->CurrentQueenObject=-1;
			queenStatusPointer->QueenState=QBS_Reconsider;
		}
	}
	
	
	if (sbPtr->SBdptr==NULL) {
		/* No far behaviour. */
		return;
	}

	if(!dynPtr->IsInContactWithFloor && queenStatusPointer->current_move!=QM_Jump && queenStatusPointer->current_move!=QM_Climbing)
	{
		//if queen is not on the ground , and not currently jumping 
		//switch to standby
		queenStatusPointer->current_move=QM_Standby;
	}

	if(dynPtr->IsInContactWithFloor)
	{
		
		if(!stricmp(LevelName,"hangar"))
		{
			//is the player in the trench
			if((dynPtr->Position.vy+1500)<Player->ObWorld.vy)
			{
				PlayerInTrench=TRUE;
				if(queenStatusPointer->QueenState!=QBS_Engagement)
				{
					QueenForceReconsider(sbPtr);
				}
			}
			else
			{
				PlayerInTrench=FALSE;
			}
			//is the player in the locker ('hangar')
			if(Player->ObWorld.vx> HangarLockerMinX && Player->ObWorld.vx < HangarLockerMaxX &&
			   Player->ObWorld.vz> HangarLockerMinZ && Player->ObWorld.vz < HangarLockerMaxZ)	
				PlayerInLocker=TRUE;
			else
				PlayerInLocker=FALSE;


			//is the queen in the airlock?
			if(UpperAirlockDoorOpen && !LowerAirlockDoorOpen)
			{
				//has the queen fallen into the airlock
				if(ObjectIsInAirlock(sbPtr) && !ObjectIsInAirlock(Player->ObStrategyBlock) && queenStatusPointer->QueenState!=QBS_ClimbingOutOfAirlock)
				{
					QueenForceReconsider(sbPtr);
				}
			}

				
		}
	}

	queenStatusPointer->QueenStateTimer+=NormalFrameTime;

	{
		VECTORCH pos=Player->ObWorld;
		SubVector(&dynPtr->Position,&pos);
		DistanceToPlayer=Approximate3dMagnitude(&pos);
	}
	
					/*-------------------------------------------------------**
					** 	Check collision reports , and check for need to jump **
					**-------------------------------------------------------*/
	JumpDesired=FALSE;
	
	{
	
		BOOL ignore_obstacles=FALSE;
		if(queenStatusPointer->LastVelocity.vx || queenStatusPointer->LastVelocity.vz ||
		   queenStatusPointer->current_move==QM_Climbing)
		{
		/* Now, smash stuff. */
	
			COLLISIONREPORT *nextReport;

			nextReport = dynPtr->CollisionReportPtr;

			while(nextReport) 
			{
				int normalDotWithVelocity;
				BOOL ConsiderJumpingForThisObject=FALSE;

				{
					VECTORCH normVelocity = sbPtr->DynPtr->LinVelocity;
					normVelocity.vy=0;
					Normalise(&normVelocity);
					normalDotWithVelocity = DotProduct(&(nextReport->ObstacleNormal),&(normVelocity));
				}

				if(normalDotWithVelocity<-20000)//is the object in the way
				{
					if(nextReport->ObstacleNormal.vy<20000 && nextReport->ObstacleNormal.vy>-20000)
					{
						//obstacle is reasonably vertical , may need to jump
						ConsiderJumpingForThisObject=TRUE;
					}
					
					if (nextReport->ObstacleSBPtr) 
					{

						if(nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourInanimateObject)
						{
							INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = nextReport->ObstacleSBPtr->SBdataptr;
							if((objectstatusptr)&&(objectstatusptr->Indestructable == 0))
							{
								int i;
								BOOL AllowedToDestroy=TRUE;
								//is this one of the queen's objects?
								for(i=0;i<NumQueenObjects;i++)
								{
									if(QueenObjectList[i]==nextReport->ObstacleSBPtr)
									{
										AllowedToDestroy=FALSE;
									}
								}

								if(AllowedToDestroy)
								{
									/* aha: an object which the queen can destroy... */
									CauseDamageToObject(nextReport->ObstacleSBPtr,&TemplateAmmo[AMMO_NPC_PAQ_CLAW].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
									//no need to jump over this
									ConsiderJumpingForThisObject=FALSE;
								}
							}
						}
						else if(nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourTrackObject)
						{
							/* aha: an object which the queen can destroy... */
							CauseDamageToObject(nextReport->ObstacleSBPtr,&TemplateAmmo[AMMO_NPC_PAQ_CLAW].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
							//no need to jump over this
							ConsiderJumpingForThisObject=FALSE;
						}
						else
						{
							ConsiderJumpingForThisObject=FALSE;
						}
						
						
						if(queenStatusPointer->QueenState==QBS_GoingForObject)
						{
							//is this the object that the queen was heading for?
							if(nextReport->ObstacleSBPtr==queenStatusPointer->QueenTargetSB)
							{
								QueenPickupTargetObject(sbPtr);

								//no need to jump over this
								ConsiderJumpingForThisObject=FALSE;
								
							}
						}
						else if(queenStatusPointer->QueenState==QBS_CarryingObject)
						{
							//if the queen is colliding with the object she is carrying , ignore it
							if(QueenObjectList[queenStatusPointer->CurrentQueenObject]==nextReport->ObstacleSBPtr)
							{
								//no need to jump over this
								ConsiderJumpingForThisObject=FALSE;
								
							}
						}

						if(nextReport->ObstacleSBPtr==Player->ObStrategyBlock)
						{
							ConsiderJumpingForThisObject=FALSE;

							//have we connected with a butt attack
							if(queenStatusPointer->current_move==QM_ButtCharge)
							{
								queenStatusPointer->current_move=QM_ButtAttack;
								queenStatusPointer->next_move=QM_Standby;
								queenStatusPointer->moveTimer=0;

								{
									//knock the player back
									VECTORCH* player_impulse;
									player_impulse=&Player->ObStrategyBlock->DynPtr->LinImpulse;
									player_impulse->vx+=dynPtr->OrientMat.mat31/4;
									player_impulse->vy-=6000;
									player_impulse->vz+=dynPtr->OrientMat.mat33/4;
								}
								{
									//do some damage
									CauseDamageToObject(Player->ObStrategyBlock,&QueenButtDamage, ONE_FIXED,NULL);

								}
							}
							else if(queenStatusPointer->current_move==QM_Climbing)
							{
								//need to push he player out of the way
								Player->ObStrategyBlock->DynPtr->LinImpulse.vx=min(Player->ObStrategyBlock->DynPtr->LinImpulse.vx,-3000);
							}
							else if(queenStatusPointer->QueenState!=QBS_Engagement &&
							   queenStatusPointer->QueenState!=QBS_CarryingObject)
							{
								QueenForceReconsider(sbPtr);
							}

						}
					}
					
					/*Queen climbing out of the airlock?*/
					if(queenStatusPointer->QueenState==QBS_ClimbingOutOfAirlock &&
					   queenStatusPointer->current_move!=QM_Climbing)	

					{
						QueenCalculateTargetInfo(sbPtr);
						if((dynPtr->Position.vx-AirlockMinX)<2800)		
						{
							//queen has hit the end wall , so start climbing
							queenStatusPointer->current_move=QM_Climbing;
							queenStatusPointer->moveTimer=0;
						}
					}


				}
				if(ConsiderJumpingForThisObject) ConsiderJumping=TRUE;
				nextReport = nextReport->NextCollisionReportPtr;
			}
	
		}
		
		//if the queen is approaching the player in the locker
		//ignore obstacles to allow the quuen to get to the player
		if(PlayerInLocker && !queenStatusPointer->TempTarget &&
		   queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock)
		{
			QueenCalculateTargetInfo(sbPtr);
			if(queenStatusPointer->TargetDistance<12000)	
			{
				ignore_obstacles=TRUE;
			}
		}
	
		/*If the queen is trying to get out of the airlock , then we don't want to deviate because 
		 of walls getting in the way*/
		if(queenStatusPointer->QueenState==QBS_ClimbingOutOfAirlock)
		{
			ignore_obstacles=TRUE;
		}

		//check for nearby obstacles , and decide whether to jump.
		if(!ignore_obstacles)
		{
			VECTORCH direction;
			VECTORCH position=sbPtr->SBdptr->ObWorld;
			
			QueenCalculateTargetInfo(sbPtr);
			
			direction.vx=dynPtr->OrientMat.mat31;
			direction.vy=0;
			direction.vz=dynPtr->OrientMat.mat33;
			Normalise(&direction);

			
			//only bother to think of jumping if the queen is facing her target

			if(queenStatusPointer->TargetDirection.vz>(queenStatusPointer->TargetDirection.vx*2) && 
			   queenStatusPointer->TargetDirection.vz>-(queenStatusPointer->TargetDirection.vx*2))
			{
				int left_dist,centre_dist,right_dist;
				VECTORCH left_point,centre_point,right_point;
				int distance;

				//do several line of sight tests one metre above ground , in front of queen.
				//if there is nothing near by , then queen should be able to jump over
			
				position.vy-=1000;
				

				LOS_ObjectHitPtr=0;
				FindPolygonInLineOfSight(&queenStatusPointer->VectToTarget,&position,1,sbPtr->SBdptr);
				centre_dist=LOS_Lambda;
				centre_point=LOS_Point;
				if(LOS_ObjectHitPtr && LOS_ObjectHitPtr->ObStrategyBlock)
				{
					if(LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourInanimateObject ||
					   LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourTrackObject ||
					   LOS_ObjectHitPtr==Player)
					{
						//not an obstacle
						centre_dist=1000000;
					}
				}

				//check on the right
				position.vx+=dynPtr->OrientMat.mat11/32;
				position.vy+=dynPtr->OrientMat.mat12/32;
				position.vz+=dynPtr->OrientMat.mat13/32;
				
				LOS_ObjectHitPtr=0;
				FindPolygonInLineOfSight(&queenStatusPointer->VectToTarget,&position,1,sbPtr->SBdptr);
				right_dist=LOS_Lambda;
				right_point=LOS_Point;
				if(LOS_ObjectHitPtr && LOS_ObjectHitPtr->ObStrategyBlock)
				{
					if(LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourInanimateObject ||
					   LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourTrackObject ||
					   LOS_ObjectHitPtr==Player)
					{
						//not an obstacle
						right_dist=1000000;
					}
				}
	

				//check on the left
				position.vx-=dynPtr->OrientMat.mat11/16;
				position.vy-=dynPtr->OrientMat.mat12/16;
				position.vz-=dynPtr->OrientMat.mat13/16;
				
				LOS_ObjectHitPtr=0;
				FindPolygonInLineOfSight(&queenStatusPointer->VectToTarget,&position,1,sbPtr->SBdptr);
				left_dist=LOS_Lambda;
				left_point=LOS_Point;
				if(LOS_ObjectHitPtr && LOS_ObjectHitPtr->ObStrategyBlock)
				{
					if(LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourInanimateObject ||
					   LOS_ObjectHitPtr->ObStrategyBlock->I_SBtype==I_BehaviourTrackObject ||
					   LOS_ObjectHitPtr==Player)
					{
						//not an obstacle
						left_dist=1000000;
					}
				}

				distance=queenStatusPointer->TargetDistance;
				
				if(distance>10000) distance=10000;
				
				if(right_dist<distance || centre_dist<distance || left_dist<distance)
				{
					if(centre_dist<left_dist && centre_dist<right_dist)
					{
						queenStatusPointer->TargetPos=centre_point;		
					}
					else if(right_dist<left_dist)
					{
						queenStatusPointer->TargetPos=right_point;		
					}
					else
					{
						queenStatusPointer->TargetPos=left_point;		
					}
					/*
					if(right_dist<left_dist)
					{
						queenStatusPointer->TargetPos.vx-=dynPtr->OrientMat.mat11/8;
						queenStatusPointer->TargetPos.vz-=dynPtr->OrientMat.mat13/8;
					}
					else
					{
						queenStatusPointer->TargetPos.vx+=dynPtr->OrientMat.mat11/8;
						queenStatusPointer->TargetPos.vz+=dynPtr->OrientMat.mat13/8;
					}
					queenStatusPointer->TempTarget=TRUE;
					queenStatusPointer->TempTargetTimer=6*ONE_FIXED;
					queenStatusPointer->TargetInfoValid=FALSE;
					*/
				}
				else if(ConsiderJumping)
				{
					JumpDesired=TRUE;
				}
			
			}
		}
		
	}
	

	if(queenStatusPointer->QueenState==QBS_GoingForObject)
	{
		//if the queen is close enought to the object she is going for
		//pick it up even without a collision
		QueenCalculateTargetInfo(sbPtr);

		if(queenStatusPointer->TargetDistance<1000)
		{
			QueenPickupTargetObject(sbPtr);
		}
		
	}
	
	
						/*---------------------------------------------------**
						** 	consider if queen should change her current plan **
						**---------------------------------------------------*/
	{
		
		switch(queenStatusPointer->QueenState)
		{
			case QBS_Reconsider :
			{
				//find closest object that is on the ground
				int closest=-1;
				int DistanceToObject=1000000000;
				int i;

				textprint("Queen Reconsider\n");
				if(queenStatusPointer->PlayingHitDelta)
				{
					break;
				}
				
				if(UpperAirlockDoorOpen && !LowerAirlockDoorOpen)
				{
					
					//has the queen fallen into the airlock
					if(ObjectIsInAirlock(sbPtr))
					{
						//the queen is in the airlock
						if(ObjectIsInAirlock(Player->ObStrategyBlock))
						{
							//player is in the airlock with the queen
							//splat him
							queenStatusPointer->QueenState=QBS_Engagement;
							queenStatusPointer->QueenTargetSB=Player->ObStrategyBlock;
							queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;					
						}
						else
						{
							//go into get out of airlock mode in that case
							queenStatusPointer->QueenState=QBS_ClimbingOutOfAirlock;
							queenStatusPointer->TempTarget=TRUE;
							queenStatusPointer->TempTargetTimer=50*ONE_FIXED;

							queenStatusPointer->TargetPos.vx=AirlockMinX-6000;
							queenStatusPointer->TargetPos.vz=AirlockCentreZ;

							queenStatusPointer->BeenInAirlock=TRUE;
						}
						queenStatusPointer->QueenStateTimer=0;
						queenStatusPointer->TargetInfoValid=FALSE;
						break;
					}
				}


				for(i=0;i<NumQueenObjects;i++)
				{
					if(!QueenObjectList[i]->SBflags.destroyed_but_preserved)
					{
						if(QueenObjectList[i]->DynPtr->IsInContactWithFloor)
						{
							VECTORCH pos=QueenObjectList[i]->DynPtr->Position;
							int dist;
							SubVector(&dynPtr->Position,&pos);

							if(!stricmp(LevelName,"battle"))
							{
								//In battle , the objects near the egg sack are hard for the queen to get to.
								//So best ignore them.
								if(QueenObjectList[i]->DynPtr->Position.vz>0) continue;
							}
							
							
							//if the y distance is more than 1500 , the object is probably inaccesible
							if(pos.vy<=1500)
							{
								dist=Approximate3dMagnitude(&pos);

								if(dist<DistanceToObject)
								{
									DistanceToObject=dist;
									closest=i;
								}
							}
						}
					}
				}

				{
					int player_value=MUL_FIXED(FastRandom()& 0xffff,DistanceToPlayer/queenStatusPointer->QueenPlayerBias);
					int object_value=MUL_FIXED(FastRandom()& 0xffff,DistanceToObject/queenStatusPointer->QueenObjectBias);

					if(object_value<player_value && closest!=-1 && !PlayerInTrench)
					{
						//go for the object
						queenStatusPointer->QueenState=QBS_GoingForObject;
						queenStatusPointer->CurrentQueenObject=closest;
						queenStatusPointer->QueenTargetSB=QueenObjectList[queenStatusPointer->CurrentQueenObject];

						if(!stricmp(LevelName,"hangar"))
						{
							queenStatusPointer->QueenPlayerBias++;
						}
						else
						{
							//int he predator version , make it more likely for the queen to go after the player
							queenStatusPointer->QueenPlayerBias+=3;
						}
						queenStatusPointer->QueenObjectBias--;
						if(queenStatusPointer->QueenObjectBias==0) queenStatusPointer->QueenObjectBias=1;

					}
					else
					{
						//go for the player
						queenStatusPointer->QueenState=QBS_Engagement;
						queenStatusPointer->CurrentQueenObject=-1;
						queenStatusPointer->QueenTargetSB=Player->ObStrategyBlock;
						queenStatusPointer->QueenObjectBias++;
						queenStatusPointer->QueenPlayerBias--;
						if(queenStatusPointer->QueenPlayerBias==0)queenStatusPointer->QueenPlayerBias=1;
					}

					queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;					
					queenStatusPointer->QueenStateTimer=0;
					queenStatusPointer->TargetInfoValid=FALSE;
					
				}
				
			
			}
			
			break;

			case QBS_Engagement :
			{
				if(queenStatusPointer->QueenStateTimer>10*ONE_FIXED && DistanceToPlayer>4000)
				{
					queenStatusPointer->QueenState=QBS_Reconsider;
					queenStatusPointer->QueenStateTimer=0;
				}
			}
			textprint("Queen Engagement\n");
			break;

			case QBS_GoingForObject :
			{
				if(queenStatusPointer->QueenStateTimer>15*ONE_FIXED)
				{
					QueenForceReconsider(sbPtr);
				}
			}
			textprint("Queen Going for object\n");
			break;

			case QBS_CarryingObject :
			{
				
				if(queenStatusPointer->QueenStateTimer>15*ONE_FIXED)
				{
					QueenForceReconsider(sbPtr);
				}
				
			}
			textprint("Queen Carrying object\n");
			break;

			case QBS_ClimbingOutOfAirlock :
				textprint("Queen climbing out of airlock\n");
			break;
			
			default: ;
		}	
	}
	textprint("Queen Bias - Object %d  Player %d\n",queenStatusPointer->QueenObjectBias,queenStatusPointer->QueenPlayerBias);
	textprint("Queen Health %d\n",sbPtr->SBDamageBlock.Health>>16);	
	
						/*--------------------**
						** 	Can queen attack? **
						**--------------------*/

	//queen must be in engagement mode , and not avoiding stuff
	if(!queenStatusPointer->TempTarget && queenStatusPointer->QueenState==QBS_Engagement)
	{
		//player must be close enough and in front 90 degree arc
		QueenCalculateTargetInfo(sbPtr);
		if (queenStatusPointer->TargetDistance<4000 ||
		    (queenStatusPointer->TargetDistance<8000 && queenStatusPointer->TargetRelSpeed<1000))  
		{
			if(queenStatusPointer->current_move==QM_Standby ||
			   queenStatusPointer->current_move==QM_Close ||
			   queenStatusPointer->current_move==QM_Charge)
			{

				if(queenStatusPointer->TargetDirection.vz>queenStatusPointer->TargetDirection.vx &&
				   queenStatusPointer->TargetDirection.vz>-queenStatusPointer->TargetDirection.vx)
				{
					int x=queenStatusPointer->TargetDirection.vx;
					x+=(FastRandom() % 80000);
					x-=40000;
					if (x>0) 
					{
						if(PlayerInTrench)
							Queen_Do_Swipe(sbPtr,Queen_Swipe_Right_Low);
						else
							Queen_Do_Swipe(sbPtr,Queen_Swipe_Right);

					} 
					else 
					{
						if(PlayerInTrench)
							Queen_Do_Swipe(sbPtr,Queen_Swipe_Left_Low);
						else
							Queen_Do_Swipe(sbPtr,Queen_Swipe_Left);
					}
				}
			}
		}
	}
	
	 

			/*------------------------------------------**
			** Decide if new movement state is required **
			**------------------------------------------*/


	if(queenStatusPointer->QueenState==QBS_CarryingObject)
	{
		//Queen is lining up to throw object
		QueenCalculateTargetInfo(sbPtr);
		if(queenStatusPointer->TargetDirection.vz>(queenStatusPointer->TargetDirection.vx*2) && 
		   queenStatusPointer->TargetDirection.vz>-(queenStatusPointer->TargetDirection.vx*2) && !queenStatusPointer->TempTarget)
		{
			//Time to throw object at player.
			VECTORCH impulse;
			if(CalculateTrajectory(&QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->Position,
								   &Player->ObStrategyBlock->DynPtr->Position,
								   &Player->ObStrategyBlock->DynPtr->LinVelocity,
								   QUEEN_THROWN_OBJECT_SPEED,
								   &impulse))
			{
				//give the object an impulse ,and reinstate gravity
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->LinImpulse=impulse;
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->GravityOn=1;
				QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OnlyCollideWithEnvironment=0;	
				{
					MATRIXCH id_mat={ONE_FIXED,0,0,0,ONE_FIXED,0,0,0,ONE_FIXED};
					QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OrientMat=id_mat;

				}
				//queen will need to choose a new objective
				queenStatusPointer->QueenState=QBS_Reconsider;
				queenStatusPointer->QueenTargetSB=Player->ObStrategyBlock;
				queenStatusPointer->TargetInfoValid=FALSE;
				
			}
			else
			{
				//continue heading for target
				if(queenStatusPointer->current_move==QM_Standby)
				{
					queenStatusPointer->next_move=QM_Close;
				}
			}
  		

		}
		else
		{
			//Queen needs to turn some more
			if(queenStatusPointer->current_move==QM_Standby)
			{
				queenStatusPointer->next_move=QM_Close;
			}
		}
	}
	//if in standby , need to choose a move
	else if(queenStatusPointer->current_move==QM_Standby && queenStatusPointer->QueenState!=QBS_Reconsider)
	{
		QueenCalculateTargetInfo(sbPtr);
		//queen is heading towards object , or player
		if(((queenStatusPointer->TargetDistance>8000)||(queenStatusPointer->TargetDistance>2000 && queenStatusPointer->TargetRelSpeed>QUEEN_CLOSE_SPEED))  &&
		   queenStatusPointer->TargetDirection.vz>queenStatusPointer->TargetDirection.vx &&
		   queenStatusPointer->TargetDirection.vz>-queenStatusPointer->TargetDirection.vx)
		{
			queenStatusPointer->next_move=QM_Charge;
			if(queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock)
			{
				//going for player. possibility of butting player
				//Nb. queen can't actually colide with player , if the player is in the locker area
				if((FastRandom()&0xffff)<(ONE_FIXED/2) && !PlayerInLocker)
				{
					queenStatusPointer->next_move=QM_ButtCharge;
				}
			}
			if(PlayerInTrench)
			{
				queenStatusPointer->next_move=QM_ComeToPoint;
			}
		} 
		else if (queenStatusPointer->TargetDistance>3000 || queenStatusPointer->TargetRelSpeed>0) 
		{
			if(queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock && !queenStatusPointer->TempTarget)
				queenStatusPointer->next_move=QM_Close;
			else
				queenStatusPointer->next_move=QM_ComeToPoint;
		} 
		else 
		{
			if(queenStatusPointer->QueenState!=QBS_ClimbingOutOfAirlock)
			{
				if(queenStatusPointer->TempTarget)
				{
					queenStatusPointer->TempTarget=FALSE;
					queenStatusPointer->TargetInfoValid=FALSE;
				}
			}			
			if(queenStatusPointer->QueenState==QBS_GoingForObject)
			{
				//have to keep going until queen hits object
				queenStatusPointer->next_move=QM_Close;
			}
			else
			{
				//queen right next to target now , so wait
				queenStatusPointer->next_move=QM_Standby;
				//assuming queen is facing her target , that is. 
				if(queenStatusPointer->TargetDirection.vz<queenStatusPointer->TargetDirection.vx*2 ||
				   queenStatusPointer->TargetDirection.vz<-queenStatusPointer->TargetDirection.vx*2)
				{
					//queen isn't facing target , so she needs to continue closing after all
					queenStatusPointer->next_move=QM_Close;

				}

			}
		}

	}
	textprint("Queen target distance %d\n",queenStatusPointer->TargetDistance);
	textprint("Queen target position %d : %d : %d\n",queenStatusPointer->TargetPos.vx,queenStatusPointer->TargetPos.vy,queenStatusPointer->TargetPos.vz);
///////////////////////////////////////////////////////////////////////	
				/*--------------------**
				** 	Airlock avoidance **
				**--------------------*/

	QueenCheckForAvoidAirlock(sbPtr);

	if(queenStatusPointer->TempTarget)
	{
		QueenCalculateTargetInfo(sbPtr);
		if(queenStatusPointer->TargetDistance<4000)
		{
			queenStatusPointer->TempTarget=FALSE;
			queenStatusPointer->TargetInfoValid=FALSE;
			
		}
		queenStatusPointer->TempTargetTimer-=NormalFrameTime;
		if(queenStatusPointer->TempTargetTimer<0)
		{
			queenStatusPointer->TempTarget=FALSE;
			queenStatusPointer->TargetInfoValid=FALSE;
		}
	}

	

		/*-------------------------------------------------------------------**
		** 	check for redirecting the queen , if the player is in the locker **
		**-------------------------------------------------------------------*/

	if(!queenStatusPointer->TempTarget && 
		queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock &&
		PlayerInLocker)
	{
		//is the queen in line with the locker area?
		if((dynPtr->Position.vz-HangarLockerMaxZ)*2>dynPtr->Position.vx-HangarLockerMaxX ||
		   (HangarLockerMinZ-dynPtr->Position.vz)*2>dynPtr->Position.vx-HangarLockerMaxX)
		{
			//nope
			queenStatusPointer->TempTarget=TRUE;
			queenStatusPointer->TempTargetTimer=2*ONE_FIXED;
			queenStatusPointer->TargetInfoValid=FALSE;
			
			//move the queen so that she is lined up with the locker
			queenStatusPointer->TargetPos.vx=dynPtr->Position.vx;
			queenStatusPointer->TargetPos.vy=dynPtr->Position.vy;
			queenStatusPointer->TargetPos.vz=HangarLockerCentreZ;
		}	
	}
	
					/*---------------------**
					** 	check for taunting **
					**---------------------*/

	if(queenStatusPointer->QueenTauntTimer>0 )
	{
		queenStatusPointer->QueenTauntTimer-=NormalFrameTime;
		if(queenStatusPointer->QueenTargetSB==Player->ObStrategyBlock && DistanceToPlayer>10000)
		{
			QueenCalculateTargetInfo(sbPtr);
	   		if(queenStatusPointer->TargetDirection.vz>queenStatusPointer->TargetDirection.vx &&
			   queenStatusPointer->TargetDirection.vz>-queenStatusPointer->TargetDirection.vx)
			{
				queenStatusPointer->QueenTauntTimer=0;
				//Queen has recently hit player , and is facing player
				//time to taunt
				queenStatusPointer->next_move=QM_Taunt;
			}
		}
	}
	
				/*-------------------------------**
				** 	Flamethrower avoidance stuff **
				**-------------------------------*/

	//only if queen is trying to attack the player in close combat
	if(queenStatusPointer->QueenState==QBS_Engagement)
	{
		
		
		if(queenStatusPointer->QueenFireTimer>QueenMinimumFireTime)
		{
			/*stop and hiss for a bit*/
			if(queenStatusPointer->current_move==QM_Standby ||
			   queenStatusPointer->current_move==QM_ComeToPoint ||	
			   queenStatusPointer->current_move==QM_ButtCharge ||	
			   queenStatusPointer->current_move==QM_Charge ||	
			   queenStatusPointer->current_move==QM_Close)
			{
				queenStatusPointer->current_move=QM_Hiss;
				queenStatusPointer->moveTimer=0;
			}	
			
		}
		else if(queenStatusPointer->QueenFireTimer==0)
		{
			if(queenStatusPointer->next_move==QM_Hiss)
			{
				queenStatusPointer->next_move=QM_Standby;
			}
			
			if(queenStatusPointer->current_move==QM_Hiss)
			{
				queenStatusPointer->moveTimer=0;
				queenStatusPointer->current_move=queenStatusPointer->next_move;
				queenStatusPointer->next_move=QM_Standby;
					
			}

		}
		//update the timer
		if(TargetIsFiringFlamethrowerAtQueen(sbPtr))
		{
			queenStatusPointer->QueenFireTimer+=NormalFrameTime;
			if(queenStatusPointer->QueenFireTimer>2*ONE_FIXED)
			{
				queenStatusPointer->QueenFireTimer=2*ONE_FIXED;
			}
		}
		else
		{
			queenStatusPointer->QueenFireTimer-=NormalFrameTime;
			if(queenStatusPointer->QueenFireTimer<0)
			{
				queenStatusPointer->QueenFireTimer=0;
			}
		}
	}
	else
	{
		//make sure queen isn't hissing
		if(queenStatusPointer->next_move==QM_Hiss)
		{
			queenStatusPointer->next_move=QM_Standby;
		}
		
		if(queenStatusPointer->current_move==QM_Hiss)
		{
			queenStatusPointer->moveTimer=0;
			queenStatusPointer->current_move=queenStatusPointer->next_move;
			queenStatusPointer->next_move=QM_Standby;
				
		}
		queenStatusPointer->QueenFireTimer=0;
	}
	


	if(!queenStatusPointer->TempTarget)
	{
		//update target location
		queenStatusPointer->TargetPos=queenStatusPointer->QueenTargetSB->DynPtr->Position;					
	}
	
	
	
						/*---------------------------**
						** 	Handle Queeen's movement **
						**---------------------------*/
	
	
	dynPtr->Displacement.vx = 0;
	dynPtr->Displacement.vy = 0;
	dynPtr->Displacement.vz = 0;

	
	if (queenStatusPointer->QueenState!=QBS_Dead) 
	{
		switch (queenStatusPointer->current_move) 
		{
			case (QM_Standby):
			{
				
				textprint("Queen State: Standby\n");
				db_log3(("Queen State: Standby\n"));
				
				if (queenStatusPointer->next_move==QM_Standby || queenStatusPointer->PlayingHitDelta) 
				{
					/* Do not much. */
					QueenMove_Standby(sbPtr);
				}
				else
				{
					queenStatusPointer->current_move=queenStatusPointer->next_move;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->moveTimer=0;
					//queenStatusPointer->TargetPos=Queen_Target_Point;
				}
				
				break;
			}
			case (QM_Stun) :
			{
				textprint("Queen State: Stunned\n");
				//don't do anything
				sbPtr->DynPtr->LinVelocity.vx = 0;
				sbPtr->DynPtr->LinVelocity.vy = 0;
				sbPtr->DynPtr->LinVelocity.vz = 0;
				break;
			}

			case (QM_StepForward):
			{
				textprint("Queen State: Step Forward\n");
				/* Move function. */
		   //		QueenMove_StepForward(sbPtr);
				break;
			}
			case (QM_StepBack):
			{
				textprint("Queen State: Step Back\n");
				/* Move function. */
		   		QueenMove_StepBack(sbPtr);
				break;
			}
			case (QM_TurnLeft):
			{
				textprint("Queen State: Turn Left\n");
				/* Move function. */
			//	QueenMove_TurnLeft(sbPtr);
				break;
			}
			case (QM_TurnRight):
			{
				textprint("Queen State: Turn Right\n");
				/* Move function. */
			//	QueenMove_TurnRight(sbPtr);
				break;
			}
			case (QM_ComeToPoint):
			{
				textprint("Queen State: Come To Point\n");
				db_log3(("Queen State: Come To Point\n"));
				/* Move function. */
			
				if(JumpDesired)
				{
					textprint("Jumping\n");
					sbPtr->DynPtr->LinImpulse.vy-=10000;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->current_move=QM_Jump;
				}
				else
				{
					QueenMove_Walk(sbPtr);
				}
			
				break;
			}
			case (QM_Taunt):
			{
				textprint("Queen State: Taunt\n");
				db_log3(("Queen State: Taunt\n"));
				/* Move function. */
				QueenMove_Taunt(sbPtr);
				break;
			}
			case (QM_Hiss):
			{
				textprint("Queen State: Hiss\n");
				/* Move function. */
				QueenMove_Hiss(sbPtr);
				break;
			}
			case (QM_LeftSwipe):
			{
				textprint("Queen State: Left Swipe\n");
			//	QueenMove_LeftSwipe(sbPtr);
				break;
			}
			case (QM_RightSwipe):
			{
				textprint("Queen State: Right Swipe\n");
			//	QueenMove_RightSwipe(sbPtr);
				break;
			}
			
			case (QM_ButtAttack):
			{
				textprint("Queen State: Butt attack\n");
				db_log3(("Queen State: Butt attack\n"));
				QueenMove_ButtAttack(sbPtr);
				break;
			}
			
			case (QM_Charge):
			{
				textprint("Queen State: Charging\n");
				db_log3(("Queen State: Charging\n"));
				/* Move function. */
				
				if(JumpDesired)
				{
					textprint("Jumping\n");
					sbPtr->DynPtr->LinImpulse.vy-=10000;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->current_move=QM_Jump;
				}
				else
				{
				   	QueenMove_Charge(sbPtr);
				}
				
				break;
			}
						
			case (QM_Close):
			{
				textprint("Queen State: Closing\n");
				db_log3(("Queen State: Closing\n"));
				/* Move function. */


				if(JumpDesired)
				{
					textprint("Jumping\n");
					sbPtr->DynPtr->LinImpulse.vy-=10000;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->current_move=QM_Jump;
				}
				else
				{
					QueenMove_Close(sbPtr);
				}
				
				break;
			}

			case (QM_ButtCharge) :
			{
				textprint("Queen State: Butt charging\n");
				db_log3(("Queen State: Butt charging\n"));
				/* Move function. */


				if(JumpDesired)
				{
					textprint("Jumping\n");
					sbPtr->DynPtr->LinImpulse.vy-=10000;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->current_move=QM_Jump;
				}
				else
				{
					QueenMove_ButtCharge(sbPtr);
				}
				
				break;
			}

			case (QM_Jump):
			{
				textprint("Queen State: Jumping\n");
				db_log3(("Queen State: Jumping\n"));
				//stay in jump mode until queen hits the floor again
				if(!dynPtr->IsInContactWithFloor)
				{
					sbPtr->DynPtr->LinVelocity.vx=sbPtr->DynPtr->OrientMat.mat31/10;
					sbPtr->DynPtr->LinVelocity.vy=0;
					sbPtr->DynPtr->LinVelocity.vz=sbPtr->DynPtr->OrientMat.mat33/10;
				}
				else
				{
					queenStatusPointer->current_move=queenStatusPointer->next_move;
					queenStatusPointer->next_move=QM_Standby;
					queenStatusPointer->moveTimer=0;
				}
				
				break;
			}

			case (QM_Climbing):
			{
				textprint("Queen State: Climbing\n");
				sbPtr->DynPtr->LinVelocity.vx = 0;
				sbPtr->DynPtr->LinVelocity.vy = 0;
				sbPtr->DynPtr->LinVelocity.vz = 0;
				QueenMove_Climb(sbPtr);
			}
			break;
			
			default:
			{
				GLOBALASSERT(0);
				return;
			}
		}
		HModel_Regen(&queenStatusPointer->HModelController,(20*ONE_FIXED));

	}

	queenStatusPointer->LastVelocity=dynPtr->LinVelocity;
	

				/*----------------------------------**
				** 	Update object queen is carrying **
				**----------------------------------*/
	
	if(queenStatusPointer->QueenState==QBS_CarryingObject)
	{
		ProveHModel(sbPtr->SBdptr->HModelControlBlock,sbPtr->SBdptr);
				
		QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->Position=queenStatusPointer->QueenRightHand->World_Offset;
		QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->PrevPosition=queenStatusPointer->QueenRightHand->World_Offset;
		QueenObjectList[queenStatusPointer->CurrentQueenObject]->DynPtr->OrientMat=queenStatusPointer->QueenRightHand->SecMat;
	}

	sbPtr->SBDamageBlock.IsOnFire=0;
	/* That would be silly. */

/*----------------------------------------------------------------------------------------**
** 	Monitor the queen's objects , and check for any high speed collisions with the player **
**----------------------------------------------------------------------------------------*/
	{
		int i;
		for(i=0;i<NumQueenObjects;i++)
		{
			if(!QueenObjectList[i]->SBflags.destroyed_but_preserved)
			{
				BOOL doneBounceNoise=FALSE;
				COLLISIONREPORT *nextReport;

				nextReport = QueenObjectList[i]->DynPtr->CollisionReportPtr;

				while(nextReport) 
				{
					int impulse=Approximate3dMagnitude(&QueenObjectList[i]->DynPtr->LinImpulse);
					if(impulse>10000)
					{
						//object hit something while moving quickly , so play a bounce sound
						if(!doneBounceNoise)
						{
							ThrownObjectBounceNoise(i,&nextReport->ObstaclePoint);
							doneBounceNoise=TRUE;
						}
																	
						if(nextReport->ObstacleSBPtr)
						{
							
							if(nextReport->ObstacleSBPtr==Player->ObStrategyBlock ||
							   nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourInanimateObject ||
							   (nextReport->ObstacleSBPtr->name && !strcmp(nextReport->ObstacleSBPtr->name,"locker door")))
							{
								
								//object has hit the player at speed.

								DAMAGE_PROFILE impact_damage=QueenImpactDamage;
								VECTORCH direction;
								VECTORCH hit_object_direction;
								int dotproduct;
								

								//adjust the object's position so it is ear the player 
								//back in the direction that it came from
								direction=QueenObjectList[i]->DynPtr->Position;
								SubVector(&QueenObjectList[i]->DynPtr->PrevPosition,&direction);
								if(direction.vx || direction.vy || direction.vz)
								{
									Normalise(&direction);
								}

								hit_object_direction=nextReport->ObstacleSBPtr->DynPtr->Position;
								SubVector(&nextReport->ObstacleSBPtr->DynPtr->PrevPosition,&hit_object_direction);
								if(hit_object_direction.vx || hit_object_direction.vy || hit_object_direction.vz)
								{
									Normalise(&hit_object_direction);
								}

								dotproduct=Dot(&direction,&hit_object_direction);

								//damage the player
								impact_damage.Impact=40+MUL_FIXED(15,dotproduct);
								CauseDamageToObject(nextReport->ObstacleSBPtr,&impact_damage, ONE_FIXED,NULL);

								//damage the thrown object as well (almost certainly destroying it)
								impact_damage.Impact*=10;
								CauseDamageToObject(QueenObjectList[i],&impact_damage, ONE_FIXED,NULL);
								
								
								{
									//knock the object back
									VECTORCH* impulse;
									impulse=&nextReport->ObstacleSBPtr->DynPtr->LinImpulse;
									impulse->vx+=direction.vx/4;
									impulse->vz+=direction.vz/4;
								}
														
								
								QueenObjectList[i]->DynPtr->Position.vx=nextReport->ObstacleSBPtr->DynPtr->Position.vx+direction.vx/100;
								QueenObjectList[i]->DynPtr->Position.vz=nextReport->ObstacleSBPtr->DynPtr->Position.vz+direction.vz/100;
								QueenObjectList[i]->DynPtr->PrevPosition.vx=QueenObjectList[i]->DynPtr->Position.vx;
								QueenObjectList[i]->DynPtr->PrevPosition.vz=QueenObjectList[i]->DynPtr->Position.vz;

								//set the taunt timer
								queenStatusPointer->QueenTauntTimer=ONE_FIXED/2;						

								break;
							}
						}
					}
					nextReport = nextReport->NextCollisionReportPtr;
				}
	
			}
		}
	}

	QueenSoundHiss(sbPtr);

}


static BOOL LockerDoorIsClosed()
{
	TRACK_OBJECT_BEHAV_BLOCK* door;
	GLOBALASSERT(LockerDoorSbptr);
	if(LockerDoorSbptr->SBflags.destroyed_but_preserved) return FALSE;
	GLOBALASSERT(LockerDoorSbptr->SBdataptr);
	
	door=(TRACK_OBJECT_BEHAV_BLOCK*)LockerDoorSbptr->SBdataptr;

	GLOBALASSERT(door->to_track);

	if(door->to_track->reverse && !door->to_track->playing) return TRUE;

	return FALSE;

}

void HandleHangarAirlock()
{
	//only need to look out for the airlock in hangar
	int wind_multiplier=0;

	if(!stricmp(LevelName,"hangar"))
	{
		GLOBALASSERT(UpperAirlockDoorSbptr);
		GLOBALASSERT(UpperAirlockDoorSbptr->DynPtr);
		GLOBALASSERT(LowerAirlockDoorSbptr);
		GLOBALASSERT(LowerAirlockDoorSbptr->DynPtr);
		

		//check to see which of the airlock doors are open
		{
			VECTORCH* door_pos=&UpperAirlockDoorSbptr->DynPtr->Position;
			int upper_open_amount=0;
			int lower_open_amount=0;
			
			if(door_pos->vz==UpperAirlockDoorStart.vz)
			{
				UpperAirlockDoorOpen=FALSE;
			}
			else
			{
				UpperAirlockDoorOpen=TRUE;
				upper_open_amount=DIV_FIXED(door_pos->vz-UpperAirlockDoorStart.vz,AirlockOpeningDistance);
			}
			
			door_pos=&LowerAirlockDoorSbptr->DynPtr->Position;
			if(door_pos->vz==LowerAirlockDoorStart.vz)
			{
				LowerAirlockDoorOpen=FALSE;
			}
			else
			{
				LowerAirlockDoorOpen=TRUE;
				lower_open_amount=DIV_FIXED(door_pos->vz-LowerAirlockDoorStart.vz,AirlockOpeningDistance);
			}
			
			wind_multiplier=MUL_FIXED(upper_open_amount,lower_open_amount);

			if(wind_multiplier<0) wind_multiplier=-wind_multiplier;
			//get a multiplier for the wind strength , based on how far the two pairs of doors are open
			if(wind_multiplier>ONE_FIXED) wind_multiplier=ONE_FIXED;
			
			if(wind_multiplier>=ONE_FIXED)
			{
				AirlockTimeOpen+=NormalFrameTime;
			}
			else
			{
				AirlockTimeOpen=0;
			}
			
		}
		
		if(LowerAirlockDoorOpen && UpperAirlockDoorOpen)
		{
			extern int NumActiveBlocks;

			int i = NumActiveBlocks;
			extern DISPLAYBLOCK *ActiveBlockList[];
			
			textprint("Wind strength %d\n",wind_multiplier);
			
			
			for(i=0;i<NumActiveBlocks;i++)
			{
				STRATEGYBLOCK *sbPtr = ActiveBlockList[i]->ObStrategyBlock;
				if(sbPtr && sbPtr->DynPtr && !sbPtr->DynPtr->IsStatic)
				{
					VECTORCH* pos=&sbPtr->DynPtr->Position;	
					VECTORCH* cur_impulse=&sbPtr->DynPtr->LinImpulse;
					VECTORCH impulse;
					BOOL above_airlock=FALSE;

					if(pos->vx>HangarLockerMinX && pos->vx<HangarLockerMaxX && pos->vz>HangarLockerMinZ && pos->vz<HangarLockerMaxZ)
					{
						if(LockerDoorIsClosed())
						{
							continue;
						}
					}

					if(pos->vy>20000)
					{
						//outside , so damage this object
						if(sbPtr->I_SBtype==I_BehaviourQueenAlien) 
							//the queen has loads of health , so need to damage her quicker
							CauseDamageToObject(sbPtr,&VacuumDamage,NormalFrameTime*30,NULL);
						else
							CauseDamageToObject(sbPtr,&VacuumDamage,NormalFrameTime,NULL);

						if(pos->vy>21000)
						{
							//turn off gravity
							sbPtr->DynPtr->GravityOn=0;
						}
						continue;
					}
					else if(AirlockTimeOpen> 10*ONE_FIXED)
					{
						/*
						After 10 seconds start doing damage to objects even if they aren't outside yet.
						Scale damage so it increases linearly until 30 seconds have passed
						*/
						
						int multiplier;
						AirlockTimeOpen=max(AirlockTimeOpen,30*ONE_FIXED);
						multiplier=MUL_FIXED(NormalFrameTime,(AirlockTimeOpen-10*ONE_FIXED)/20);
												
						CauseDamageToObject(sbPtr,&VacuumDamage,multiplier,NULL);
					}
					
					
					if(pos->vx>AirlockMinX && pos->vx<AirlockMaxX && pos->vz>AirlockMinZ && pos->vz<AirlockMaxZ) above_airlock=TRUE;
					
					
					impulse.vx=AirlockCentreX-pos->vx;
					impulse.vy=0;
					impulse.vz=AirlockCentreZ-pos->vz;
					
					if(!above_airlock && impulse.vx<0)
					{
						if(impulse.vz*2>impulse.vx && impulse.vz*2<-impulse.vx)
						{
							if(impulse.vz>0)
							{
								impulse.vz-=12000;

							}
							else
							{
								impulse.vz+=12000;
							}
						}

					}
					
					Normalise(&impulse);

					impulse.vx/=2;
					if(above_airlock)
						impulse.vy=30000;
					else
						impulse.vy=0;
					impulse.vz/=2;

					if(wind_multiplier<ONE_FIXED)
					{
						impulse.vx=MUL_FIXED(impulse.vx,wind_multiplier);
						impulse.vy=MUL_FIXED(impulse.vy,wind_multiplier);
						impulse.vz=MUL_FIXED(impulse.vz,wind_multiplier);
					}


					if(impulse.vx>0)
					{
						if(impulse.vx>cur_impulse->vx)
						{
							cur_impulse->vx+=MUL_FIXED(impulse.vx,NormalFrameTime);
							cur_impulse->vx=min(cur_impulse->vx,impulse.vx);
						}
					}
					else
					{
						if(impulse.vx<cur_impulse->vx)
						{
							cur_impulse->vx+=MUL_FIXED(impulse.vx,NormalFrameTime);
							cur_impulse->vx=max(cur_impulse->vx,impulse.vx);
						}
					}

					if(impulse.vy>0)
					{
						if(impulse.vy>cur_impulse->vy)
						{
							cur_impulse->vy+=MUL_FIXED(impulse.vy,NormalFrameTime);
							cur_impulse->vy=min(cur_impulse->vy,impulse.vy);
						}
					}
					else
					{
						if(impulse.vy<cur_impulse->vy)
						{
							cur_impulse->vy+=MUL_FIXED(impulse.vy,NormalFrameTime);
							cur_impulse->vy=max(cur_impulse->vy,impulse.vy);
						}
					}
 

					if(impulse.vz>0)
					{
						if(impulse.vz>cur_impulse->vz)
						{
							cur_impulse->vz+=MUL_FIXED(impulse.vz,NormalFrameTime);
							cur_impulse->vz=min(cur_impulse->vz,impulse.vz);
						}
					}
					else
					{
						if(impulse.vz<cur_impulse->vz)
						{
							cur_impulse->vz+=MUL_FIXED(impulse.vz,NormalFrameTime);
							cur_impulse->vz=max(cur_impulse->vz,impulse.vz);
						}
					}


					if((pos->vy+ActiveBlockList[i]->ObMaxY)>3950 && !above_airlock)
					{
						if(sbPtr->DynPtr->UseStandardGravity)
							cur_impulse->vy-=MUL_FIXED(MUL_FIXED(GRAVITY_STRENGTH+5000,NormalFrameTime),wind_multiplier);
						else
							cur_impulse->vy-=MUL_FIXED(MUL_FIXED(5000,NormalFrameTime),wind_multiplier);

					}

					
				}
			}
		}
	}
}


static BOOL TargetIsFiringFlamethrowerAtQueen(STRATEGYBLOCK *sbPtr)
{
	QUEEN_STATUS_BLOCK *queenStatusPointer;
	GLOBALASSERT(sbPtr);

	queenStatusPointer=(QUEEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	
	//only marine has flamethrower
	if(AvP.PlayerType!=I_Marine) return FALSE;

	//queen must be heading for the player
	if(queenStatusPointer->QueenTargetSB!=Player->ObStrategyBlock) return FALSE;
	if(queenStatusPointer->TempTarget) return FALSE;

	QueenCalculateTargetInfo(sbPtr);

	if(queenStatusPointer->TargetDistance>30000) return FALSE; //queen is too far away to be harmed
	//if(queenStatusPointer->TargetDistance<3000) return FALSE; //queen close enough to attack , so she may as well go ahead
	/*
	if (queenStatusPointer->attack_delta->timer!=(ONE_FIXED-1) &&
		queenStatusPointer->attack_delta->timer!=0)
	{
		//currently attacking
		return FALSE;
	} 
	*/
	{
		/* Is the player firing a flamethrower? */
		PLAYER_WEAPON_DATA *weaponPtr;
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	    GLOBALASSERT(playerStatusPtr);

	    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	
		if (weaponPtr->WeaponIDNumber != WEAPON_FLAMETHROWER) 
		{
			return(FALSE);
		}
		if (weaponPtr->CurrentState != WEAPONSTATE_FIRING_PRIMARY) 
		{
			return(FALSE);
		}
		
	}

	//The player is firing the flamethrower.
	//are the queen and player facing each other

	{

		VECTORCH offset;
		MATRIXCH WtoL;
		
		WtoL=sbPtr->DynPtr->OrientMat;
 	
		offset=sbPtr->DynPtr->Position;
		SubVector(&Player->ObWorld,&offset);
			
		TransposeMatrixCH(&WtoL);
		RotateVector(&offset,&WtoL);

		if ( (offset.vz <0) 
			&& (offset.vz <  offset.vx) 
			&& (offset.vz < -offset.vx) 
			&& (offset.vz <  offset.vy) 
			&& (offset.vz < -offset.vy) ) {
	
			/* 90 horizontal, 90 vertical... continue. */
		} else {
			return(FALSE);
		}
		
		/* Now test it for the other way round. */

		WtoL=Player->ObMat;
		
		offset=Player->ObWorld;
		SubVector(&sbPtr->DynPtr->Position,&offset);
	
		TransposeMatrixCH(&WtoL);
		RotateVector(&offset,&WtoL);

		if ( (offset.vz <0) 
			&& (offset.vz <  offset.vx) 
			&& (offset.vz < -offset.vx) 
			&& (offset.vz <  offset.vy) 
			&& (offset.vz < -offset.vy) ) {
	
			/* 90 horizontal, 90 vertical... continue. */
		} else {
			return(FALSE);
		}

	}
	
	/* If here, then it must be true! */
	return(TRUE);
	

	
}


static void MakeNonFragable_Recursion(SECTION_DATA *this_section_data)
{
	SECTION_DATA *sdptr;

	sdptr=NULL;

	/* flag this section. */
	this_section_data->sempai->flags|=section_flag_never_frag;

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			MakeNonFragable_Recursion(child_list_ptr);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return;
}

static void MakeNonFragable(HMODELCONTROLLER *controller)
{
	MakeNonFragable_Recursion(controller->section_data);
}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct queen_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff

	QUEEN_BEHAVIOUR_STATE QueenState;
	QUEEN_MANOEUVRE current_move;
	QUEEN_MANOEUVRE next_move;

	QUEEN_FOOT fixed_foot;
	VECTORCH fixed_foot_oldpos;

	VECTORCH TargetPos;

	int moveTimer;

	NPC_WANDERDATA wanderData;

	BOOL TempTarget;//going for an intermediate point
	int TempTargetTimer;//time before queen gives up going for intermediate point

	int CurrentQueenObject;
	int QueenStateTimer;
	int QueenObjectBias;
	int QueenPlayerBias;
	int QueenTauntTimer;
	int QueenFireTimer;
	VECTORCH LastVelocity;

	BOOL BeenInAirlock;
	BOOL QueenActivated; //queen is inactive until seen

	BOOL TargetInfoValid; //have the next three items been set
	int TargetDistance; //distance of current target from queen
	int TargetRelSpeed; //targets speed in queen's direction
	VECTORCH TargetDirection; //targets direction relative to queen
	VECTORCH VectToTarget;

	unsigned int PlayingHitDelta :1;

	int SwerveTimer;
	BOOL SwerveDirection;

	QUEEN_SOUND_CATEGORY lastSoundCategory;

	VECTORCH ClimbStartPosition; //used when climing out of the airlock
	BOOL AttackDoneItsDamage;

//and now those evil globals...
	BOOL PlayerInTrench;
	BOOL PlayerInLocker;
	int AirlockTimeOpen;
	BOOL UpperAirlockDoorOpen;
	BOOL LowerAirlockDoorOpen;
	VECTORCH UpperAirlockDoorStart;
	VECTORCH LowerAirlockDoorStart;


//annoying pointer related things
	char Target_SBname[SB_NAME_LENGTH];

	SECTION_DATA *fixed_foot_section;
	SECTION_DATA* QueenRightHand;

//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}QUEEN_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV queenStatusPointer

void LoadStrategy_Queen(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	QUEEN_STATUS_BLOCK* queenStatusPointer;
	QUEEN_SAVE_BLOCK* block = (QUEEN_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourQueenAlien) return;

	queenStatusPointer =(QUEEN_STATUS_BLOCK*) sbPtr->SBdataptr;

	//normally done on first frame , but need to do this here when loading
	FindQueenObjects();
	
	
	//start copying stuff
	COPYELEMENT_LOAD(QueenState)
	COPYELEMENT_LOAD(current_move)
	COPYELEMENT_LOAD(next_move)

	COPYELEMENT_LOAD(fixed_foot)
	COPYELEMENT_LOAD(fixed_foot_oldpos)

	COPYELEMENT_LOAD(TargetPos)

	COPYELEMENT_LOAD(moveTimer)

	COPYELEMENT_LOAD(wanderData)

	COPYELEMENT_LOAD(TempTarget)//going for an intermediate point
	COPYELEMENT_LOAD(TempTargetTimer)//time before queen gives up going for intermediate point

	COPYELEMENT_LOAD(CurrentQueenObject)
	COPYELEMENT_LOAD(QueenStateTimer)
	COPYELEMENT_LOAD(QueenObjectBias)
	COPYELEMENT_LOAD(QueenPlayerBias)
	COPYELEMENT_LOAD(QueenTauntTimer)
	COPYELEMENT_LOAD(QueenFireTimer)
	COPYELEMENT_LOAD(LastVelocity)

	COPYELEMENT_LOAD(BeenInAirlock)
	COPYELEMENT_LOAD(QueenActivated) //queen is inactive until seen

	COPYELEMENT_LOAD(TargetInfoValid) //have the next three items been set
	COPYELEMENT_LOAD(TargetDistance) //distance of current target from queen
	COPYELEMENT_LOAD(TargetRelSpeed) //targets speed in queen's direction
	COPYELEMENT_LOAD(TargetDirection) //targets direction relative to queen
	COPYELEMENT_LOAD(VectToTarget)

	COPYELEMENT_LOAD(PlayingHitDelta)
	COPYELEMENT_LOAD(SwerveTimer)
	COPYELEMENT_LOAD(SwerveDirection)
	COPYELEMENT_LOAD(lastSoundCategory)

	COPYELEMENT_LOAD(ClimbStartPosition) //used when climing out of the airlock
	COPYELEMENT_LOAD(AttackDoneItsDamage)

	//load globals
	PlayerInTrench = block->PlayerInTrench;
	PlayerInLocker = block->PlayerInLocker;
	AirlockTimeOpen = block->AirlockTimeOpen;
	UpperAirlockDoorOpen = block->UpperAirlockDoorOpen;
	LowerAirlockDoorOpen = block->LowerAirlockDoorOpen;
	UpperAirlockDoorStart = block->UpperAirlockDoorStart;
	LowerAirlockDoorStart =	block->LowerAirlockDoorStart;
	

	//load target
	queenStatusPointer->QueenTargetSB = FindSBWithName(block->Target_SBname);


	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&queenStatusPointer->HModelController);
		}
	}

	//get delta controller pointers
	queenStatusPointer->attack_delta=Get_Delta_Sequence(&queenStatusPointer->HModelController,"attack");
 	queenStatusPointer->hit_delta=Get_Delta_Sequence(&queenStatusPointer->HModelController,"hit");

	

	//get section data pointers

	if(queenStatusPointer->fixed_foot == LeftFoot)
	{
		queenStatusPointer->fixed_foot_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,
				"left foot");
	}
	else
	{
		queenStatusPointer->fixed_foot_section=GetThisSectionData(queenStatusPointer->HModelController.section_data,
				"right foot");

	}

	queenStatusPointer->QueenRightHand=GetThisSectionData(queenStatusPointer->HModelController.section_data,"rrrr fing");

	Load_SoundState(&queenStatusPointer->soundHandle);
	

}

void SaveStrategy_Queen(STRATEGYBLOCK* sbPtr)
{
	QUEEN_STATUS_BLOCK* queenStatusPointer;
	QUEEN_SAVE_BLOCK* block;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	queenStatusPointer = (QUEEN_STATUS_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_SAVE(QueenState)
	COPYELEMENT_SAVE(current_move)
	COPYELEMENT_SAVE(next_move)

	COPYELEMENT_SAVE(fixed_foot)
	COPYELEMENT_SAVE(fixed_foot_oldpos)

	COPYELEMENT_SAVE(TargetPos)

	COPYELEMENT_SAVE(moveTimer)

	COPYELEMENT_SAVE(wanderData)

	COPYELEMENT_SAVE(TempTarget)//going for an intermediate point
	COPYELEMENT_SAVE(TempTargetTimer)//time before queen gives up going for intermediate point

	COPYELEMENT_SAVE(CurrentQueenObject)
	COPYELEMENT_SAVE(QueenStateTimer)
	COPYELEMENT_SAVE(QueenObjectBias)
	COPYELEMENT_SAVE(QueenPlayerBias)
	COPYELEMENT_SAVE(QueenTauntTimer)
	COPYELEMENT_SAVE(QueenFireTimer)
	COPYELEMENT_SAVE(LastVelocity)

	COPYELEMENT_SAVE(BeenInAirlock)
	COPYELEMENT_SAVE(QueenActivated) //queen is inactive until seen

	COPYELEMENT_SAVE(TargetInfoValid) //have the next three items been set
	COPYELEMENT_SAVE(TargetDistance) //distance of current target from queen
	COPYELEMENT_SAVE(TargetRelSpeed) //targets speed in queen's direction
	COPYELEMENT_SAVE(TargetDirection) //targets direction relative to queen
	COPYELEMENT_SAVE(VectToTarget)

	COPYELEMENT_SAVE(PlayingHitDelta)
	COPYELEMENT_SAVE(SwerveTimer)
	COPYELEMENT_SAVE(SwerveDirection)
	COPYELEMENT_SAVE(lastSoundCategory)

	COPYELEMENT_SAVE(ClimbStartPosition) //used when climing out of the airlock
	COPYELEMENT_SAVE(AttackDoneItsDamage)

	//save globals
	block->PlayerInTrench = PlayerInTrench;
	block->PlayerInLocker = PlayerInLocker;
	block->AirlockTimeOpen = AirlockTimeOpen;
	block->UpperAirlockDoorOpen = UpperAirlockDoorOpen;
	block->LowerAirlockDoorOpen = LowerAirlockDoorOpen;
	block->UpperAirlockDoorStart = UpperAirlockDoorStart;
	block->LowerAirlockDoorStart =	LowerAirlockDoorStart;

	//save target
	if(queenStatusPointer->QueenTargetSB)
	{
		COPY_NAME(block->Target_SBname,queenStatusPointer->QueenTargetSB->SBname);
	}
	else
	{
		COPY_NAME(block->Target_SBname,Null_Name);
	}

	//save strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&queenStatusPointer->HModelController);

	Save_SoundState(&queenStatusPointer->soundHandle);
}
