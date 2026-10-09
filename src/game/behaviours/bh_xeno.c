/* Patrick 4/7/97 ----------------------------------------------------
  Source file for Xenoborg AI behaviour functions....
  --------------------------------------------------------------------*/

/* ChrisF 6/7/98.  Hopeless.  I'll have to take off and nuke the
 entire site from orbit. */
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
#include "bh_pred.h"
#include "bh_xeno.h"
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "bh_debri.h"
#include "plat_shp.h"
#include "particle.h"
#include "ai_sight.h"
#include "sequnces.h"
#include "huddefs.h"
#include "showcmds.h"
#include "sfx.h"
#include "bh_marin.h"
#include "bh_far.h"
#include "pldghost.h"
#include "pheromon.h"
#include "targeting.h"
#include "dxlog.h"
#include "los.h"
#include "bh_alien.h"
#include "bh_corpse.h"
#include "bh_dummy.h"
#include "game_statistics.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "psnd.h"

#define XENO_SENTRYTIME	(20)
#define FAR_XENO_ACTIVITY	0
#define FAR_XENO_FIRING		0

#define XENO_WALKING_ANIM_SPEED	(ONE_FIXED<<1)
#define XENO_TURNING_ANIM_SPEED	(ONE_FIXED<<1)

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern unsigned char Null_Name[8];

VECTORCH null_vec={0,0,0};

extern HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeSetFromLibrary(const char* rif_name,const char* shape_set_name);
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *section_pointer); 

int ShowXenoStats=0;

int RATweak=40;

void EnforceXenoborgShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);
void SetXenoborgShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);
void SetXenoborgShapeAnimSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length);
void XenoborgHandleMovingAnimation(STRATEGYBLOCK *sbPtr);

static void ComputeDeltaValues(STRATEGYBLOCK *sbPtr);
static void KillXeno(STRATEGYBLOCK *sbPtr,int wounds,DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming);
void CreateXenoborg(VECTORCH *Position,int type);
void Xenoborg_ActivateAllDeltas(STRATEGYBLOCK *sbPtr);
void Xenoborg_DeactivateAllDeltas(STRATEGYBLOCK *sbPtr);
STRATEGYBLOCK *Xenoborg_GetNewTarget(VECTORCH *xenopos, STRATEGYBLOCK *me);
void Xenoborg_GetRelativeAngles(STRATEGYBLOCK *sbPtr, int *anglex, int *angley, VECTORCH *pivotPoint);
void Xeno_UpdateTargetTrackPos(STRATEGYBLOCK *sbPtr);
int Xeno_Activation_Test(STRATEGYBLOCK *sbPtr);

void Xeno_HeadMovement_ScanLeftRight(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_HeadMovement_ScanUpDown(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_LeftArmMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_LeftArmMovement_TrackUpDown(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_RightArmMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_RightArmMovement_TrackUpDown(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_TorsoMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_LeftArmMovement_WaveUp(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_RightArmMovement_WaveUp(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_TorsoMovement_TrackToAngle(STRATEGYBLOCK *sbPtr,int rate,int in_anglex);

void Xenoborg_MaintainLeftGun(STRATEGYBLOCK *sbPtr);
void Xenoborg_MaintainRightGun(STRATEGYBLOCK *sbPtr);
void Xeno_MaintainLasers(STRATEGYBLOCK *sbPtr);
void Xeno_SwitchLED(STRATEGYBLOCK *sbPtr,int state);
void Xeno_Stomp(STRATEGYBLOCK *sbPtr);
void Xeno_MaintainSounds(STRATEGYBLOCK *sbPtr);

int Xeno_HeadMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley);
void Xeno_TorsoMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex);
int Xeno_LeftArmMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley);
int Xeno_RightArmMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley);
void Xeno_TorsoMovement_Centre(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_LeftArmMovement_Centre(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_RightArmMovement_Centre(STRATEGYBLOCK *sbPtr,int rate);
void Xeno_Limbs_ShootTheRoof(STRATEGYBLOCK *sbPtr);

void Xeno_Enter_PowerUp_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_PowerDown_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_ActiveWait_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_Dormant_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_TurnToFace_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_Following_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_Returning_State(STRATEGYBLOCK *sbPtr);
void Xeno_Enter_ShootingTheRoof_State(STRATEGYBLOCK *sbPtr);

void Execute_Xeno_Dying(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Inactive(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_PowerUp(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_PowerDown(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_ActiveWait(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_TurnToFace(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Follow(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Return(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Avoidance(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_ShootTheRoof(STRATEGYBLOCK *sbPtr);

void Execute_Xeno_ActiveWait_Far(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_TurnToFace_Far(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Follow_Far(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Return_Far(STRATEGYBLOCK *sbPtr);
void Execute_Xeno_Avoidance_Far(STRATEGYBLOCK *sbPtr);

int XenoActivation_FrustrumReject(VECTORCH *localOffset);

/* Begin Code! */

void CastXenoborg(void) {

	#define BOTRANGE 2000

	VECTORCH position;

	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO XENOBORGS IN MULTIPLAYER MODE");
		return;
	}

	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateXenoborg(&position, 0);

}

void CreateXenoborg(VECTORCH *Position,int type)
{
	STRATEGYBLOCK* sbPtr;
	int i;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) {
		NewOnScreenMessage("FAILED TO CREATE BOT: SB CREATION FAILURE");
		return; /* failure */
	}
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourXenoborg;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = *Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);

		dynPtr->Mass=500; /* As opposed to 160. */
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE BOT: DYNBLOCK CREATION FAILURE");
		return;
	}

	sbPtr->shapeIndex = 0;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* create, initialise and attach an alien data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(XENO_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		XENO_STATUS_BLOCK *xenoStatus = (XENO_STATUS_BLOCK *)sbPtr->SBdataptr;

		NPC_InitMovementData(&(xenoStatus->moveData));
		NPC_InitWanderData(&(xenoStatus->wanderData));     	
		InitWaypointManager(&xenoStatus->waypointManager);

		/* Initialise xenoborg's stats */
		{
			NPC_DATA *NpcData;
	
			NpcData=GetThisNpcData(I_NPC_Xenoborg);
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		
		xenoStatus->behaviourState=XS_Inactive;
		xenoStatus->lastState=XS_Inactive;
		xenoStatus->Target=NULL; 
		COPY_NAME(xenoStatus->Target_SBname,Null_Name);
		xenoStatus->targetTrackPos.vx=0;
		xenoStatus->targetTrackPos.vy=0;
		xenoStatus->targetTrackPos.vz=0;

		xenoStatus->Wounds=0;
		xenoStatus->GibbFactor=0;
		xenoStatus->stateTimer=XENO_POWERDOWN_TIME;
		xenoStatus->my_module=sbPtr->containingModule->m_aimodule;
		xenoStatus->my_spot_therin=sbPtr->DynPtr->Position;
		{
			/* Pull out my_orientdir_therin. */
			xenoStatus->my_orientdir_therin.vx=0;
			xenoStatus->my_orientdir_therin.vx=0;
			xenoStatus->my_orientdir_therin.vz=1000;

			RotateVector(&xenoStatus->my_orientdir_therin,&sbPtr->DynPtr->OrientMat);
		}
		xenoStatus->module_range=7;
		xenoStatus->UpTime=XENO_SENTRYTIME*ONE_FIXED;
		xenoStatus->Head_Pan=0;
		xenoStatus->Head_Tilt=0;
		xenoStatus->Left_Arm_Pan=0;
		xenoStatus->Left_Arm_Tilt=0;
		xenoStatus->Right_Arm_Pan=0;
		xenoStatus->Right_Arm_Tilt=0;
		xenoStatus->Torso_Twist=0;
		
		xenoStatus->headpandir=0;
		xenoStatus->headtiltdir=0;
		xenoStatus->leftarmpandir=0;
		xenoStatus->leftarmtiltdir=0;
		xenoStatus->rightarmpandir=0;
		xenoStatus->rightarmtiltdir=0;
		xenoStatus->torsotwistdir=0;
		
		xenoStatus->Old_Head_Pan=0;
		xenoStatus->Old_Head_Tilt=0;
		xenoStatus->Old_Left_Arm_Pan=0;
		xenoStatus->Old_Left_Arm_Tilt=0;
		xenoStatus->Old_Right_Arm_Pan=0;
		xenoStatus->Old_Right_Arm_Tilt=0;
		xenoStatus->Old_Torso_Twist=0;

		xenoStatus->headLock=0;
		xenoStatus->leftArmLock=0;
		xenoStatus->rightArmLock=0;
		xenoStatus->targetSightTest=0;
		xenoStatus->IAmFar=1;
		xenoStatus->ShotThisFrame=0;
		
		xenoStatus->obstruction.environment=0;
		xenoStatus->obstruction.destructableObject=0;
		xenoStatus->obstruction.otherCharacter=0;
		xenoStatus->obstruction.anySingleObstruction=0;

		/* Init beams. */
		xenoStatus->LeftMainBeam.BeamIsOn = 0;
		xenoStatus->RightMainBeam.BeamIsOn = 0;
		xenoStatus->TargetingLaser[0].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[0].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[0].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[0].BeamIsOn=0;

		xenoStatus->TargetingLaser[1].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[1].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[1].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[1].BeamIsOn=0;

		xenoStatus->TargetingLaser[2].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[2].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[2].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[2].BeamIsOn=0;

		xenoStatus->FiringLeft=0;
		xenoStatus->FiringRight=0;

		xenoStatus->UseHeadLaser=0;
		xenoStatus->UseLALaser=0;
		xenoStatus->UseRALaser=0;

		xenoStatus->head_moving=0;
		xenoStatus->la_moving=0;
		xenoStatus->ra_moving=0;
		xenoStatus->torso_moving=0;

		xenoStatus->HeadLaserOnTarget=0;
		xenoStatus->LALaserOnTarget=0;
		xenoStatus->RALaserOnTarget=0;

		xenoStatus->soundHandle1=SOUND_NOACTIVEINDEX;
		xenoStatus->soundHandle2=SOUND_NOACTIVEINDEX;

		xenoStatus->incidentFlag=0;
		xenoStatus->incidentTimer=0;

		xenoStatus->head_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->left_arm_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->right_arm_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->torso_whirr=SOUND_NOACTIVEINDEX;

		xenoStatus->HModelController.section_data=NULL;
		xenoStatus->HModelController.Deltas=NULL;

		for(i=0;i<SB_NAME_LENGTH;i++) xenoStatus->death_target_ID[i] =0; 
		xenoStatus->death_target_sbptr=0;
		xenoStatus->death_target_request=0;

		root_section=GetNamedHierarchyFromLibrary("hnpc_xenoborg","xenobasic");
				
		if (!root_section) {
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE BOT: NO HMODEL");
			return;
		}
		Create_HModel(&xenoStatus->HModelController,root_section);
		InitHModelSequence(&xenoStatus->HModelController,HMSQT_Xenoborg,XBSS_Powered_Down_Standard,ONE_FIXED);

		{
			DELTA_CONTROLLER *delta;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"HeadTilt",(int)HMSQT_Xenoborg,(int)XBSS_Head_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"HeadPan",(int)HMSQT_Xenoborg,(int)XBSS_Head_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"LeftArmTilt",(int)HMSQT_Xenoborg,(int)XBSS_LeftArm_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"LeftArmPan",(int)HMSQT_Xenoborg,(int)XBSS_LeftArm_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"RightArmTilt",(int)HMSQT_Xenoborg,(int)XBSS_RightArm_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"RightArmPan",(int)HMSQT_Xenoborg,(int)XBSS_RightArm_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"TorsoTwist",(int)HMSQT_Xenoborg,(int)XBSS_Torso_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

		}

		/* Containment test NOW! */
		if(!(sbPtr->containingModule))
		{
			/* no containing module can be found... abort*/
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE BOT: MODULE CONTAINMENT FAILURE");
			return;
		}
		LOCALASSERT(sbPtr->containingModule);
		Xeno_SwitchLED(sbPtr,0);
	
		MakeXenoborgNear(sbPtr);

		NewOnScreenMessage("XENOBORG CREATED");
	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE BOT: MALLOC FAILURE");
		return;
	}
}

static void VerifyDeltaControllers(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);	          		

	/* Nothing has deltas like a xenoborg does. */

	xenoStatusPointer->head_pan=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"HeadPan");
	GLOBALASSERT(xenoStatusPointer->head_pan);

	xenoStatusPointer->head_tilt=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"HeadTilt");
	GLOBALASSERT(xenoStatusPointer->head_tilt);

	xenoStatusPointer->left_arm_pan=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"LeftArmPan");
	GLOBALASSERT(xenoStatusPointer->left_arm_pan);

	xenoStatusPointer->left_arm_tilt=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"LeftArmTilt");
	GLOBALASSERT(xenoStatusPointer->left_arm_tilt);

	xenoStatusPointer->right_arm_pan=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"RightArmPan");
	GLOBALASSERT(xenoStatusPointer->right_arm_pan);

	xenoStatusPointer->right_arm_tilt=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"RightArmTilt");
	GLOBALASSERT(xenoStatusPointer->right_arm_tilt);

	xenoStatusPointer->torso_twist=Get_Delta_Sequence(&xenoStatusPointer->HModelController,"TorsoTwist");
	GLOBALASSERT(xenoStatusPointer->torso_twist);

}

/* Patrick 4/7/97 ----------------------------------------------------
  Xenoborg initialiser, visibility management, behaviour shell, and
  damage functions

 ChrisF 6/7/98.  I don't think so...
  --------------------------------------------------------------------*/
void InitXenoborgBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_XENO *toolsData; 
	int i;

	LOCALASSERT(bhdata);
	toolsData = (TOOLS_DATA_XENO *)bhdata; 
	LOCALASSERT(sbPtr);

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

	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->OrientEuler = toolsData->starteuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);

		dynPtr->Mass=1000; /* As opposed to 160. */
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* create, initialise and attach a xeno data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(XENO_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		XENO_STATUS_BLOCK *xenoStatus = (XENO_STATUS_BLOCK *)sbPtr->SBdataptr;

		NPC_InitMovementData(&(xenoStatus->moveData));
		NPC_InitWanderData(&(xenoStatus->wanderData));     	
		InitWaypointManager(&xenoStatus->waypointManager);

		/* Initialise xenoborg's stats */
		{
			NPC_DATA *NpcData;
	
			NpcData=GetThisNpcData(I_NPC_Xenoborg);
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		
		xenoStatus->behaviourState=XS_Inactive;
		xenoStatus->lastState=XS_Inactive;
		xenoStatus->Target=NULL; 
		COPY_NAME(xenoStatus->Target_SBname,Null_Name);
		xenoStatus->targetTrackPos.vx=0;
		xenoStatus->targetTrackPos.vy=0;
		xenoStatus->targetTrackPos.vz=0;

		xenoStatus->Wounds=0;
		xenoStatus->GibbFactor=0;
		xenoStatus->stateTimer=XENO_POWERDOWN_TIME;
		xenoStatus->my_module=sbPtr->containingModule->m_aimodule;
		xenoStatus->my_spot_therin=sbPtr->DynPtr->Position;
		{
			/* Pull out my_orientdir_therin. */
			xenoStatus->my_orientdir_therin.vx=0;
			xenoStatus->my_orientdir_therin.vx=0;
			xenoStatus->my_orientdir_therin.vz=1000;

			RotateVector(&xenoStatus->my_orientdir_therin,&sbPtr->DynPtr->OrientMat);
		}
		xenoStatus->module_range=toolsData->ModuleRange;
		xenoStatus->UpTime=toolsData->UpTime*ONE_FIXED;
		xenoStatus->Head_Pan=0;
		xenoStatus->Head_Tilt=0;
		xenoStatus->Left_Arm_Pan=0;
		xenoStatus->Left_Arm_Tilt=0;
		xenoStatus->Right_Arm_Pan=0;
		xenoStatus->Right_Arm_Tilt=0;
		xenoStatus->Torso_Twist=0;
		
		xenoStatus->headpandir=0;
		xenoStatus->headtiltdir=0;
		xenoStatus->leftarmpandir=0;
		xenoStatus->leftarmtiltdir=0;
		xenoStatus->rightarmpandir=0;
		xenoStatus->rightarmtiltdir=0;
		xenoStatus->torsotwistdir=0;
		
		xenoStatus->Old_Head_Pan=0;
		xenoStatus->Old_Head_Tilt=0;
		xenoStatus->Old_Left_Arm_Pan=0;
		xenoStatus->Old_Left_Arm_Tilt=0;
		xenoStatus->Old_Right_Arm_Pan=0;
		xenoStatus->Old_Right_Arm_Tilt=0;
		xenoStatus->Old_Torso_Twist=0;

		xenoStatus->headLock=0;
		xenoStatus->leftArmLock=0;
		xenoStatus->rightArmLock=0;
		xenoStatus->targetSightTest=0;
		xenoStatus->IAmFar=1;
		xenoStatus->ShotThisFrame=0;
		
		xenoStatus->obstruction.environment=0;
		xenoStatus->obstruction.destructableObject=0;
		xenoStatus->obstruction.otherCharacter=0;
		xenoStatus->obstruction.anySingleObstruction=0;

		/* Init beams. */
		xenoStatus->LeftMainBeam.BeamIsOn = 0;
		xenoStatus->RightMainBeam.BeamIsOn = 0;
		xenoStatus->TargetingLaser[0].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[0].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[0].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[0].BeamIsOn=0;

		xenoStatus->TargetingLaser[1].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[1].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[1].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[1].BeamIsOn=0;

		xenoStatus->TargetingLaser[2].SourcePosition=null_vec;
		xenoStatus->TargetingLaser[2].TargetPosition=null_vec;
		xenoStatus->TargetingLaser[2].BeamHasHitPlayer=0;
		xenoStatus->TargetingLaser[2].BeamIsOn=0;

		xenoStatus->FiringLeft=0;
		xenoStatus->FiringRight=0;

		xenoStatus->UseHeadLaser=0;
		xenoStatus->UseLALaser=0;
		xenoStatus->UseRALaser=0;

		xenoStatus->head_moving=0;
		xenoStatus->la_moving=0;
		xenoStatus->ra_moving=0;
		xenoStatus->torso_moving=0;

		xenoStatus->HeadLaserOnTarget=0;
		xenoStatus->LALaserOnTarget=0;
		xenoStatus->RALaserOnTarget=0;


		xenoStatus->soundHandle1=SOUND_NOACTIVEINDEX;
		xenoStatus->soundHandle2=SOUND_NOACTIVEINDEX;

		xenoStatus->incidentFlag=0;
		xenoStatus->incidentTimer=0;

		xenoStatus->head_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->left_arm_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->right_arm_whirr=SOUND_NOACTIVEINDEX;
		xenoStatus->torso_whirr=SOUND_NOACTIVEINDEX;

		xenoStatus->HModelController.section_data=NULL;
		xenoStatus->HModelController.Deltas=NULL;

		for(i=0;i<SB_NAME_LENGTH;i++) xenoStatus->death_target_ID[i] = toolsData->death_target_ID[i];
		xenoStatus->death_target_sbptr=0;
		xenoStatus->death_target_request=toolsData->death_target_request;

		root_section=GetNamedHierarchyFromLibrary("hnpc_xenoborg","xenobasic");
				
		GLOBALASSERT(root_section);

		Create_HModel(&xenoStatus->HModelController,root_section);
		InitHModelSequence(&xenoStatus->HModelController,HMSQT_Xenoborg,XBSS_Powered_Down_Standard,ONE_FIXED);

		{
			DELTA_CONTROLLER *delta;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"HeadTilt",(int)HMSQT_Xenoborg,(int)XBSS_Head_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"HeadPan",(int)HMSQT_Xenoborg,(int)XBSS_Head_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"LeftArmTilt",(int)HMSQT_Xenoborg,(int)XBSS_LeftArm_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"LeftArmPan",(int)HMSQT_Xenoborg,(int)XBSS_LeftArm_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"RightArmTilt",(int)HMSQT_Xenoborg,(int)XBSS_RightArm_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"RightArmPan",(int)HMSQT_Xenoborg,(int)XBSS_RightArm_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&xenoStatus->HModelController,"TorsoTwist",(int)HMSQT_Xenoborg,(int)XBSS_Torso_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

		}

		/* Containment test NOW! */
		GLOBALASSERT(sbPtr->containingModule);
		Xeno_SwitchLED(sbPtr,0);
	
	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

}

void XenoborgBehaviour(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	NPC_DATA *NpcData;
	int xenoborgIsNear;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);	          		

	NpcData=GetThisNpcData(I_NPC_Xenoborg);

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	}

	if(sbPtr->SBdptr) {
		xenoborgIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		xenoborgIsNear=0;
	}

	VerifyDeltaControllers(sbPtr);

	#if 0
	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	#endif

	InitWaypointSystem(0);

	if (sbPtr->SBdptr) {
		xenoStatusPointer->IAmFar=0;
	} else {
		xenoStatusPointer->IAmFar=1;
	}

	/* Store angles. */
	xenoStatusPointer->Old_Head_Pan			=xenoStatusPointer->Head_Pan;
	xenoStatusPointer->Old_Head_Tilt		=xenoStatusPointer->Head_Tilt;
	xenoStatusPointer->Old_Left_Arm_Pan		=xenoStatusPointer->Left_Arm_Pan;
	xenoStatusPointer->Old_Left_Arm_Tilt	=xenoStatusPointer->Left_Arm_Tilt;
	xenoStatusPointer->Old_Right_Arm_Pan	=xenoStatusPointer->Right_Arm_Pan;
	xenoStatusPointer->Old_Right_Arm_Tilt	=xenoStatusPointer->Right_Arm_Tilt;
	xenoStatusPointer->Old_Torso_Twist		=xenoStatusPointer->Torso_Twist;

	xenoStatusPointer->head_moving=0;
	xenoStatusPointer->la_moving=0;
	xenoStatusPointer->ra_moving=0;
	xenoStatusPointer->torso_moving=0;

	/* Target handling. */
	if (Validate_Target(xenoStatusPointer->Target,xenoStatusPointer->Target_SBname)==0) {
		xenoStatusPointer->Target=NULL;
	}

	xenoStatusPointer->FiringLeft=0;
	xenoStatusPointer->FiringRight=0;

	xenoStatusPointer->UseHeadLaser=0;
	xenoStatusPointer->UseLALaser=0;
	xenoStatusPointer->UseRALaser=0;

	xenoStatusPointer->LeftMainBeam.BeamIsOn = 0;
	xenoStatusPointer->RightMainBeam.BeamIsOn = 0;
	xenoStatusPointer->TargetingLaser[0].BeamIsOn=0;
	xenoStatusPointer->TargetingLaser[1].BeamIsOn=0;
	xenoStatusPointer->TargetingLaser[2].BeamIsOn=0;

	if (xenoStatusPointer->Target==NULL) {
		if ((xenoborgIsNear)||(xenoStatusPointer->incidentFlag)) {
			/* Get new target. */
			xenoStatusPointer->Target=Xenoborg_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr);
			xenoStatusPointer->targetSightTest=0;
			if (xenoStatusPointer->Target) {
				COPY_NAME(xenoStatusPointer->Target_SBname,xenoStatusPointer->Target->SBname);
				xenoStatusPointer->targetSightTest=1;
			}
			Xeno_UpdateTargetTrackPos(sbPtr);	
			xenoStatusPointer->headLock=0;
			xenoStatusPointer->leftArmLock=0;
			xenoStatusPointer->rightArmLock=0;
		}
	} else if (NPCCanSeeTarget(sbPtr,xenoStatusPointer->Target,XENO_NEAR_VIEW_WIDTH)) {
		Xeno_UpdateTargetTrackPos(sbPtr);	
		xenoStatusPointer->targetSightTest=1;
	} else {
		/* We have a target that we can't see. */
		xenoStatusPointer->headLock=0;
		xenoStatusPointer->leftArmLock=0;
		xenoStatusPointer->rightArmLock=0;
		xenoStatusPointer->targetSightTest=0;
	}
	
	if (xenoStatusPointer->GibbFactor) {
		/* If you're gibbed, you're dead. */
		sbPtr->SBDamageBlock.Health = 0;
	}

	/* Unset incident flag. */
	xenoStatusPointer->incidentFlag=0;

	xenoStatusPointer->incidentTimer-=NormalFrameTime;
	
	if (xenoStatusPointer->incidentTimer<0) {
		xenoStatusPointer->incidentFlag=1;
		xenoStatusPointer->incidentTimer=32767+(FastRandom()&65535);
	}

	if (sbPtr->SBDamageBlock.IsOnFire) {
		
		/* Why not? */
		CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);
	
		if (sbPtr->I_SBtype==I_BehaviourNetCorpse) {
			/* Gettin' out of here... */
			return;
		}

		if (xenoStatusPointer->incidentFlag) {
			if ((FastRandom()&65535)<32767) {
				sbPtr->SBDamageBlock.IsOnFire=0;
			}
		}

	}

	/* Now, switch by state. */

	switch (xenoStatusPointer->behaviourState) {
		case XS_ActiveWait:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_ActiveWait_Far(sbPtr);
			} else {
				Execute_Xeno_ActiveWait(sbPtr);
			}
			break;
		case XS_TurnToFace:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_TurnToFace_Far(sbPtr);
			} else {
				Execute_Xeno_TurnToFace(sbPtr);
			}
			break;
		case XS_Following:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_Follow_Far(sbPtr);
			} else {
				Execute_Xeno_Follow(sbPtr);
			}
			break;
		case XS_Returning:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_Return_Far(sbPtr);
			} else {
				Execute_Xeno_Return(sbPtr);
			}
			break;
		case XS_Avoidance:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_Avoidance_Far(sbPtr);
			} else {
				Execute_Xeno_Avoidance(sbPtr);
			}
			break;
  		case XS_Inactive:
			Execute_Xeno_Inactive(sbPtr);
			break;
		case XS_Activating:
			Execute_Xeno_PowerUp(sbPtr);
			break;
  		case XS_Deactivating:
			Execute_Xeno_PowerDown(sbPtr);
			break;
		case XS_Regenerating:
			break;
		case XS_Dying:
			Execute_Xeno_Dying(sbPtr);
			break;
		case XS_ShootingTheRoof:
			if (xenoStatusPointer->IAmFar) {
				Execute_Xeno_ShootTheRoof(sbPtr);
			} else {
				Execute_Xeno_ShootTheRoof(sbPtr);
			}
			break;
		default:
			/* No action? */
			break;
	}

	/* if we have actually died, we need to remove the strategyblock... so
	do this here */
	if((xenoStatusPointer->behaviourState == XS_Dying)&&(xenoStatusPointer->stateTimer <= 0)) {

		DestroyAnyStrategyBlock(sbPtr);
	}

	if ((xenoStatusPointer->behaviourState!=XS_Dying)
		&&(xenoStatusPointer->behaviourState!=XS_Inactive)
		&&(xenoStatusPointer->behaviourState!=XS_Activating)
		&&(xenoStatusPointer->behaviourState!=XS_Deactivating)) {
		/* Time to regenerate? */
		if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-2)))
			&&(sbPtr->SBDamageBlock.Health>0)) {
			/* 25% health or less. */
			Xeno_Enter_PowerDown_State(sbPtr);
		}
	}

	ComputeDeltaValues(sbPtr);

	ProveHModel_Far(&xenoStatusPointer->HModelController,sbPtr);
	
	#if (FAR_XENO_FIRING==0)
	if (xenoStatusPointer->IAmFar) {
		xenoStatusPointer->FiringLeft=0;
		xenoStatusPointer->FiringRight=0;
	}
	#endif

	/* Now lets deal with the sounds. */
	Xeno_MaintainSounds(sbPtr);

	if (xenoStatusPointer->IAmFar) {
		/* No lasers if far. */
		xenoStatusPointer->UseHeadLaser=0;
		xenoStatusPointer->UseLALaser=0;
		xenoStatusPointer->UseRALaser=0;
	}
		
	/* Now consider the lasers. */
	Xeno_MaintainLasers(sbPtr);
	Xeno_Stomp(sbPtr);

	/* Now, are we firing? */
	Xenoborg_MaintainLeftGun(sbPtr);
	Xenoborg_MaintainRightGun(sbPtr);
	
	/* Unset shot flag. */
	xenoStatusPointer->ShotThisFrame=0;
}

void MakeXenoborgNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	XENO_STATUS_BLOCK *xenoStatusPointer;    

	LOCALASSERT(sbPtr);
	dynPtr = sbPtr->DynPtr;
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(xenoStatusPointer);	          		
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
	if(dPtr==NULL) return; /* if cannot create displayblock, leave far */
			
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

   	/* state and sequence init */
	NPC_InitMovementData(&(xenoStatusPointer->moveData));
	InitWaypointManager(&xenoStatusPointer->waypointManager);
	
	dPtr->ShapeAnimControlBlock = NULL;
	dPtr->ObTxAnimCtrlBlks = NULL;

	dPtr->HModelControlBlock=&xenoStatusPointer->HModelController;

	ProveHModel(dPtr->HModelControlBlock,dPtr);

	xenoStatusPointer->IAmFar=0;

	/* make a sound */
	//Sound_Play(SID_ALIEN_HISS,"d",&sbPtr->DynPtr->Position);
}

void MakeXenoborgFar(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    
	int i;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);	          		
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;

	/* xenoborg data block init */
	if(xenoStatusPointer->behaviourState != XS_Dying) {
   		xenoStatusPointer->stateTimer=0;
	}

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;	

	xenoStatusPointer->IAmFar=1;

}

void Xenoborg_ActivateAllDeltas(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (xenoStatusPointer->head_pan) {
		xenoStatusPointer->head_pan->Active=1;
	}

	if (xenoStatusPointer->head_tilt) {
		xenoStatusPointer->head_tilt->Active=1;
	}

	if (xenoStatusPointer->left_arm_pan) {
		xenoStatusPointer->left_arm_pan->Active=1;
	}

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=1;
	}

	if (xenoStatusPointer->right_arm_pan) {
		xenoStatusPointer->right_arm_pan->Active=1;
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

	if (xenoStatusPointer->torso_twist) {
		xenoStatusPointer->torso_twist->Active=1;
	}

}

void Xenoborg_DeactivateAllDeltas(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (xenoStatusPointer->head_pan) {
		xenoStatusPointer->head_pan->Active=0;
	}
	xenoStatusPointer->Head_Pan=0;
	
	if (xenoStatusPointer->head_tilt) {
		xenoStatusPointer->head_tilt->Active=0;
	}
	xenoStatusPointer->Head_Tilt=0;

	if (xenoStatusPointer->left_arm_pan) {
		xenoStatusPointer->left_arm_pan->Active=0;
	}
	xenoStatusPointer->Left_Arm_Pan=0;

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=0;
	}
	xenoStatusPointer->Left_Arm_Tilt=0;

	if (xenoStatusPointer->right_arm_pan) {
		xenoStatusPointer->right_arm_pan->Active=0;
	}
	xenoStatusPointer->Right_Arm_Pan=0;

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=0;
	}
	xenoStatusPointer->Right_Arm_Tilt=0;

	if (xenoStatusPointer->torso_twist) {
		xenoStatusPointer->torso_twist->Active=0;
	}
	xenoStatusPointer->Torso_Twist=0;

}

void XenoborgIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,VECTORCH *incoming)
{
	
	XENO_STATUS_BLOCK *xenoStatusPointer;    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);	   	                
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);	          		

	xenoStatusPointer->Wounds|=wounds;
	xenoStatusPointer->ShotThisFrame=1;

	if (sbPtr->SBDamageBlock.Health <= 0) {

		/* Oh yes, kill them, too. */
		if (xenoStatusPointer->behaviourState!=XS_Dying) {
			CurrentGameStats_CreatureKilled(sbPtr,NULL);
			KillXeno(sbPtr,wounds,damage,multiple,incoming);
		}
	}	

	if (xenoStatusPointer->behaviourState==XS_Inactive) {
		if (xenoStatusPointer->stateTimer>=XENO_POWERDOWN_TIME) {
			/* Ow, that hurt. */
			Xeno_Enter_PowerUp_State(sbPtr);
		}
	}

	xenoStatusPointer->Target=NULL;
}

/* patrick 29/7/97 -----------------------------------
Thess functions to be called only from behaviour
------------------------------------------------------*/
static void KillXeno(STRATEGYBLOCK *sbPtr,int wounds,DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming)
{	  
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	int deathtype,tkd;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	/* make an explosion sound */    
    Sound_Play(SID_SENTRYGUNDEST,"d",&sbPtr->DynPtr->Position);  

	xenoStatusPointer->stateTimer=XENO_DYINGTIME;
	xenoStatusPointer->HModelController.Looped=0;
	xenoStatusPointer->HModelController.LoopAfterTweening=0;
	/* switch state */
	xenoStatusPointer->behaviourState=XS_Dying;

	Xenoborg_DeactivateAllDeltas(sbPtr);
	Xeno_SwitchLED(sbPtr,0);

	if(xenoStatusPointer->death_target_sbptr)
	{
		RequestState(xenoStatusPointer->death_target_sbptr,xenoStatusPointer->death_target_request, 0);
	} 


	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	if (xenoStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle2);
	}
	if (xenoStatusPointer->head_whirr!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->head_whirr);
	}
	if (xenoStatusPointer->left_arm_whirr!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->left_arm_whirr);
	}
	if (xenoStatusPointer->right_arm_whirr!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->right_arm_whirr);
	}
	if (xenoStatusPointer->torso_whirr!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->torso_whirr);
	}

	/* Set up gibb factor. */

	tkd=TotalKineticDamage(damage);
	deathtype=0;

	if (tkd>40) {
	 	/* Explosion case. */
	 	if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
	 		/* Okay, you can gibb now. */
	 		xenoStatusPointer->GibbFactor=-(ONE_FIXED>>3);
			deathtype=2;
	 	}
	} else if ( (multiple>>16)>1 ) {
	 	int newmult;

	 	newmult=DIV_FIXED(multiple,NormalFrameTime);
	 	if (MUL_FIXED(tkd,newmult)>700) {
	 		/* Excessive bullets case 1. */
	 		xenoStatusPointer->GibbFactor=-(ONE_FIXED>>5);
			deathtype=2;
	 	} else if (MUL_FIXED(tkd,newmult)>250) {
	 		/* Excessive bullets case 2. */
	 		//xenoStatusPointer->GibbFactor=ONE_FIXED>>6;
			deathtype=1;
	 	}
	}

	if (tkd>200) {
		/* Basically SADARS only. */
		xenoStatusPointer->GibbFactor=-(ONE_FIXED>>2);

		deathtype=3;
	}
	
	/* No gibbing for flamethrower. */

	{
		SECTION_DATA *chest=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"chest");
		
		if (chest==NULL) {
			/* I'm impressed. */
			deathtype+=2;
		} else if ((chest->flags&section_data_notreal)
			&&(chest->flags&section_data_terminate_here)) {
			/* That's gotta hurt. */
			deathtype++;
		}
	}
	

	{
		DEATH_DATA *this_death;
		HIT_FACING facing;
	
		facing.Front=0;
		facing.Back=0;
		facing.Left=0;
		facing.Right=0;

		if (incoming) {
			if (incoming->vz>0) {
				facing.Back=1;
			} else {
				facing.Front=1;
			}
			if (incoming->vx>0) {
				facing.Right=1;
			} else {
				facing.Left=1;
			}
		}

		this_death=GetXenoborgDeathSequence(&xenoStatusPointer->HModelController,NULL,
			xenoStatusPointer->Wounds,(xenoStatusPointer->Wounds&(
			section_flag_left_leg|section_flag_right_leg|section_flag_left_foot|section_flag_right_foot)),
			deathtype,&facing,0,0,0);

		GLOBALASSERT(this_death);

		Convert_Xenoborg_To_Corpse(sbPtr,this_death);
	}

}

void Execute_Xeno_Dying(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	{
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = xenoStatusPointer->stateTimer/2;
			if (dispPtr->ObFlags2<ONE_FIXED) {
				xenoStatusPointer->HModelController.DisableBleeding=1;
			}
		}
	}
	xenoStatusPointer->stateTimer -= NormalFrameTime;
}

void EnforceXenoborgShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime) {

	XENO_STATUS_BLOCK *xenoStatus;
	
	xenoStatus=(XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);
	
	if ((xenoStatus->HModelController.Sequence_Type==type)
		&&(xenoStatus->HModelController.Sub_Sequence==subtype)) {
		return;
	} else {
		SetXenoborgShapeAnimSequence_Core(sbPtr,type,subtype,length,tweeningtime);
	}
}

void SetXenoborgShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime)
{

	XENO_STATUS_BLOCK *xenoStatus=(XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);

	GLOBALASSERT(length!=0);

	if (tweeningtime<=0) {
		InitHModelSequence(&xenoStatus->HModelController,(int)type,subtype,length);
	} else {
		InitHModelTweening(&xenoStatus->HModelController, tweeningtime, (int)type,subtype,length, 1);
		//xenoStatus->HModelController.ElevationTweening=1;
	}

	xenoStatus->HModelController.Playing=1;
	/* Might be unset... */
}

void SetXenoborgShapeAnimSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length) {

	SetXenoborgShapeAnimSequence_Core(sbPtr,type,subtype,length,(ONE_FIXED>>2));

}

void Xenoborg_GetRelativeAngles(STRATEGYBLOCK *sbPtr, int *anglex, int *angley, VECTORCH *pivotPoint) {
	
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	MATRIXCH WtoL;
	VECTORCH targetPos;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* First, extract relative angle. */

	WtoL=sbPtr->DynPtr->OrientMat;
	TransposeMatrixCH(&WtoL);
	targetPos=xenoStatusPointer->targetTrackPos;
	targetPos.vx-=pivotPoint->vx;
	targetPos.vy-=pivotPoint->vy;
	targetPos.vz-=pivotPoint->vz;
	RotateVector(&targetPos,&WtoL);
	
	/* Now... */
	{
		int offsetx,offsety,offsetz,offseta;

		offsetx=(targetPos.vx);
		offsety=(targetPos.vz);
		offseta=-(targetPos.vy);

		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
	
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;

		}

		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		
		if (angley) {
			(*angley)=ArcTan(offseta,offsetz);

			if ((*angley)>=3072) (*angley)-=4096;
			if ((*angley)>=2048) (*angley)=(*angley)-3072;
			if ((*angley)> 1024) (*angley)=2048-(*angley);

		}
		if (anglex) {
			(*anglex)=ArcTan(offsetx,offsety);
		
			if ((*anglex)>=3072) (*anglex)-=4096;
			if ((*anglex)>=2048) (*anglex)=(*anglex)-4096;

		}
	}

}

void Execute_Xeno_Inactive(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	NPC_DATA *NpcData;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);	          		

	NpcData=GetThisNpcData(I_NPC_Xenoborg);

	if (ShowXenoStats) {
		PrintDebuggingText("In Inactive.\n");
	}

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&xenoStatusPointer->HModelController,sbPtr);
	}

	/* Regenerate a bit? */

	if (sbPtr->SBDamageBlock.Health>0) {
		int health_increment;

		health_increment=DIV_FIXED((NpcData->StartingStats.Health*NormalFrameTime),XENO_REGEN_TIME);
		sbPtr->SBDamageBlock.Health+=health_increment;	
		
		if (sbPtr->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
			sbPtr->SBDamageBlock.Health=(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT);
		}

		HModel_Regen(&xenoStatusPointer->HModelController,XENO_REGEN_TIME);
	}

	if (xenoStatusPointer->stateTimer<XENO_POWERDOWN_TIME) {
		xenoStatusPointer->stateTimer+=NormalFrameTime;
	} else {
		/* Tum te tum te tum. */

		if (Xeno_Activation_Test(sbPtr)) {
			/* Oh well, orange alert. */
			Xeno_Enter_PowerUp_State(sbPtr);
		}
	}
}

void Xeno_Enter_PowerUp_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (xenoStatusPointer->behaviourState!=XS_Inactive) {
		/* Ha! */
		return;
	}

	xenoStatusPointer->Target=NULL;
	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_Activating;

	GLOBALASSERT(xenoStatusPointer->HModelController.Sequence_Type==HMSQT_Xenoborg);
	GLOBALASSERT(xenoStatusPointer->HModelController.Sub_Sequence==XBSS_Powered_Down_Standard);

	SetXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Power_Up,(ONE_FIXED*4),(ONE_FIXED>>2));

	xenoStatusPointer->HModelController.LoopAfterTweening=0;
	
	Xenoborg_ActivateAllDeltas(sbPtr);	
	Xeno_SwitchLED(sbPtr,1);

	/* Now play with a sound. */

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	if (xenoStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle2);
	}
	Sound_Play(SID_POWERUP,"de",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);
}

void Xeno_Enter_PowerDown_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (xenoStatusPointer->behaviourState==XS_Inactive) {
		/* Ha! */
		return;
	}

	xenoStatusPointer->Target=NULL;
	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_Deactivating;

	SetXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Power_Down,ONE_FIXED,(ONE_FIXED>>2));

	xenoStatusPointer->HModelController.LoopAfterTweening=0;

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	Xenoborg_DeactivateAllDeltas(sbPtr);
	Xeno_SwitchLED(sbPtr,0);

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	if (xenoStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* Stop BorgOn. */
		Sound_Stop(xenoStatusPointer->soundHandle2);
	}
	Sound_Play(SID_POWERDN,"de",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);

}

void Xeno_Enter_ActiveWait_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_ActiveWait;

	SetXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED,(ONE_FIXED>>2));

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	
}

void Xeno_Enter_TurnToFace_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_TurnToFace;

	/* Sequence handled in the behaviour. */

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	
}

void Xeno_Enter_Following_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_Following;
	InitWaypointManager(&xenoStatusPointer->waypointManager);

	/* Sequence handled in the behaviour. */

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);
	
}

void Xeno_Enter_Returning_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_Returning;
	InitWaypointManager(&xenoStatusPointer->waypointManager);

	/* Sequence handled in the behaviour. */

	xenoStatusPointer->Target=NULL;
	/* Forget your target, too. */

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);
	
}

void Xeno_Enter_Dormant_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_Inactive;

	SetXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Down_Standard,ONE_FIXED,(ONE_FIXED>>2));

	if (xenoStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle2);
	}
	/* soundHandle1 might be still powering down. */	

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

}

void Xeno_Enter_Avoidance_State(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Make sure obstruction is set! */

	NPC_InitMovementData(&(xenoStatusPointer->moveData));
	NPCGetAvoidanceDirection(sbPtr, &(xenoStatusPointer->moveData.avoidanceDirn),&xenoStatusPointer->obstruction);
	xenoStatusPointer->lastState=xenoStatusPointer->behaviourState;
	xenoStatusPointer->behaviourState = XS_Avoidance;  		
	xenoStatusPointer->stateTimer = NPC_AVOIDTIME;
	InitWaypointManager(&xenoStatusPointer->waypointManager);

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);

}

void Xeno_Enter_ShootingTheRoof_State(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->stateTimer=0;
	xenoStatusPointer->behaviourState=XS_ShootingTheRoof;

	/* Sequence handled in the behaviour. */

	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(xenoStatusPointer->soundHandle1);
	}
	Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	
}

void Xeno_CopeWithLossOfHome(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Ooh, yuck. */

	xenoStatusPointer->my_module=sbPtr->containingModule->m_aimodule;
	xenoStatusPointer->my_spot_therin=sbPtr->DynPtr->Position;

}

void Execute_Xeno_PowerUp(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Wait to finish, I guess... */

	if (ShowXenoStats) {
		PrintDebuggingText("In PowerUp.\n");
	}

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&xenoStatusPointer->HModelController,sbPtr);
	}

	xenoStatusPointer->Target=NULL;
	xenoStatusPointer->stateTimer+=NormalFrameTime;

	if (xenoStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (xenoStatusPointer->stateTimer>((ONE_FIXED*5)/2)) {
			/* Time to start the BorgOn sound. */
			Sound_Play(SID_BORGON,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle2);
		}
	}
	if ((xenoStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(xenoStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		Xeno_Enter_ActiveWait_State(sbPtr);
	}
	

}

void Execute_Xeno_PowerDown(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (ShowXenoStats) {
		PrintDebuggingText("In PowerDown.\n");
	}

	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Power_Down,ONE_FIXED,(ONE_FIXED>>2));
	xenoStatusPointer->HModelController.LoopAfterTweening=0;
	xenoStatusPointer->HModelController.Looped=0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&xenoStatusPointer->HModelController,sbPtr);
	}

	/* Wait to finish, I guess... */

	xenoStatusPointer->Target=NULL;

	if ((xenoStatusPointer->HModelController.Tweening==Controller_NoTweening)
		&&(xenoStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))) {

		Xeno_Enter_Dormant_State(sbPtr);
	}
	

}

void Xeno_TurnAndTarget(STRATEGYBLOCK *sbPtr, int *ref_anglex,int *ref_angley) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	int anglex,angley;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	{
		SECTION_DATA *head_section;

		head_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"neck");
		GLOBALASSERT(head_section);
		
		Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&head_section->World_Offset);

	}
	
	*ref_anglex=anglex;
	*ref_angley=angley;

	/* Start turning / targeting procedure. */
	#if 0
	if ((xenoStatusPointer->headLock)
		||(anglex>((((xenoStatusPointer->Torso_Twist>>4)+XENO_HEADPAN_GIMBALL)*7)/8))
		||(anglex<((((xenoStatusPointer->Torso_Twist>>4)-XENO_HEADPAN_GIMBALL)*7)/8))) {

		Xeno_TorsoMovement_TrackToAngle(sbPtr,XENO_TORSO_TWIST_RATE,anglex);

	}
	#else
	/* Always torso twist. */
	Xeno_TorsoMovement_TrackToAngle(sbPtr,XENO_TORSO_TWIST_RATE,anglex);
	#endif
	xenoStatusPointer->headLock=Xeno_HeadMovement_TrackToAngles(sbPtr,XENO_HEAD_LOCK_RATE,anglex,angley);

	/* Now the arm. */
	
	if ((xenoStatusPointer->headLock)
		&&((anglex<XENO_LEFTARM_ACW_GIMBALL)||(anglex>-XENO_LEFTARM_CW_GIMBALL))) {

		SECTION_DATA *this_section;
		int arm_anglex,arm_angley;

		this_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"left bicep");
		if (this_section) {
			Xenoborg_GetRelativeAngles(sbPtr,&arm_anglex,NULL,&this_section->World_Offset);
		} else {
			arm_anglex=0;
		}

		this_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"left forearm");
		if (this_section) {
			Xenoborg_GetRelativeAngles(sbPtr,NULL,&arm_angley,&this_section->World_Offset);
		} else {
			arm_angley=0;
		}

		xenoStatusPointer->leftArmLock=Xeno_LeftArmMovement_TrackToAngles(sbPtr,XENO_ARM_LOCK_RATE,arm_anglex,arm_angley);
	}

	if ((xenoStatusPointer->headLock)
		&&((anglex<XENO_RIGHTARM_ACW_GIMBALL)||(anglex>-XENO_RIGHTARM_CW_GIMBALL))) {

		SECTION_DATA *this_section;
		int arm_anglex,arm_angley;

		this_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"right bicep");
		if (this_section) {
			Xenoborg_GetRelativeAngles(sbPtr,&arm_anglex,NULL,&this_section->World_Offset);
		} else {
			arm_anglex=0;
		}

		this_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"right forearm");
		if (this_section) {
			Xenoborg_GetRelativeAngles(sbPtr,NULL,&arm_angley,&this_section->World_Offset);
		} else {
			arm_angley=0;
		}
		xenoStatusPointer->rightArmLock=Xeno_RightArmMovement_TrackToAngles(sbPtr,XENO_ARM_LOCK_RATE,arm_anglex,arm_angley);
	}
	xenoStatusPointer->UseHeadLaser=1;
}

void Xeno_Limbs_ShootTheRoof(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	

	Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
	Xeno_LeftArmMovement_WaveUp(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_LeftArmMovement_TrackLeftRight(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_RightArmMovement_WaveUp(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_RightArmMovement_TrackLeftRight(sbPtr,XENO_ARM_LOCK_RATE);
	#if 0
	Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
	Xeno_HeadMovement_ScanUpDown(sbPtr,XENO_HEAD_SCAN_RATE+2);
	#else
	Xeno_HeadMovement_TrackToAngles(sbPtr,XENO_HEAD_SCAN_RATE,0,XENO_HEADTILT_GIMBALL);
	#endif
	xenoStatusPointer->UseHeadLaser=0;
	xenoStatusPointer->UseRALaser=1;
	xenoStatusPointer->UseLALaser=1;
	xenoStatusPointer->FiringLeft=1;
	xenoStatusPointer->FiringRight=1;

}

void Execute_Xeno_ActiveWait(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int anglex,angley,correctlyOrientated;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* What to do?  Do we have a target? */

	if (ShowXenoStats) {
		PrintDebuggingText("In ActiveWait.\n");
	}

	if (xenoStatusPointer->Target==NULL) {
		/* Let's wave the head around. */
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		Xeno_HeadMovement_ScanUpDown(sbPtr,XENO_HEAD_SCAN_RATE+2);
		xenoStatusPointer->UseHeadLaser=1;
		/* Are we at home? */
		if (sbPtr->containingModule->m_aimodule!=xenoStatusPointer->my_module) {
			Xeno_Enter_Returning_State(sbPtr);
			return;
		}
		/* Are we facing the right way? */

		correctlyOrientated = NPCOrientateToVector(sbPtr, &xenoStatusPointer->my_orientdir_therin,(NPC_TURNRATE>>XENO_FOOT_TURN_RATE),NULL);
		
		if (!correctlyOrientated) {

			SECTION_DATA *master_section;
			
			master_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"pelvis presley");
			GLOBALASSERT(master_section);
			
			Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&master_section->World_Offset);
	
			if (anglex>0) {
				EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Right,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
			} else {
				EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Left,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
			}
			if (xenoStatusPointer->soundHandle1==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);
			}

		} else {

			/* Otherwise just wait? */
			if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
				/* Well, it shouldn't be! */
				Sound_Stop(xenoStatusPointer->soundHandle1);
			}
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED,(ONE_FIXED>>2));
			xenoStatusPointer->stateTimer+=NormalFrameTime;
			if (xenoStatusPointer->stateTimer>xenoStatusPointer->UpTime) {
				Xeno_Enter_PowerDown_State(sbPtr);
				/* Voluntary powerdown! */
				xenoStatusPointer->stateTimer=XENO_POWERDOWN_TIME;
			}
		}
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target);
	xenoStatusPointer->stateTimer=0;
	/* Now we have a target.  Can we see it? */
	if (xenoStatusPointer->targetSightTest==0) {
		/* Can't see them.  Are we out of range? */
		if (GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,(xenoStatusPointer->module_range)-1,0)==NULL) {
			Xeno_Enter_Returning_State(sbPtr);
		} else {
			Xeno_Enter_Following_State(sbPtr);
		}
		return;
	}

	Xeno_TurnAndTarget(sbPtr,&anglex,&angley);

	#if 0
	if ((anglex>(((XENO_HEADPAN_GIMBALL)*7)/8))
		||(anglex<-(((XENO_HEADPAN_GIMBALL)*7)/8))) {

		Xeno_Enter_TurnToFace_State(sbPtr);

	}
	#else
	/* Always turn to face too? */
	Xeno_Enter_TurnToFace_State(sbPtr);
	#endif

}

void Execute_Xeno_TurnToFace(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int correctlyOrientated;
	int anglex,angley;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Do we have a target? */

	if (ShowXenoStats) {
		PrintDebuggingText("In Turn To Face.\n");
	}

	if (xenoStatusPointer->Target==NULL) {
		Xeno_Enter_ActiveWait_State(sbPtr);
		/* Otherwise just wait? */
		return;
	}

	/* Now we have a target. */
	GLOBALASSERT(xenoStatusPointer->Target);
	
	/* Set up animation... Which Way? */
	{
		SECTION_DATA *master_section;
		VECTORCH orientationDirn;

		master_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"pelvis presley");
		GLOBALASSERT(master_section);
		
		Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&master_section->World_Offset);
	
		if (anglex>0) {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Right,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		} else {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Left,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		}
	
		/* Then turn to face it, of course. */

		orientationDirn.vx = xenoStatusPointer->targetTrackPos.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = xenoStatusPointer->targetTrackPos.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,(NPC_TURNRATE>>XENO_FOOT_TURN_RATE),NULL);
		
	}

	Xeno_TurnAndTarget(sbPtr,&anglex,&angley);

	if (angley>=XENO_HEADTILT_GIMBALL) {
		Xeno_Enter_ShootingTheRoof_State(sbPtr);
		return;
	}

	if (correctlyOrientated) {
		Xeno_Enter_ActiveWait_State(sbPtr);
	}
	
}

void Execute_Xeno_ShootTheRoof(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int correctlyOrientated;
	int anglex,angley;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Do we have a target? */

	if (ShowXenoStats) {
		PrintDebuggingText("In Shoot The Roof.\n");
	}

	if ((xenoStatusPointer->Target==NULL)||(xenoStatusPointer->IAmFar)) {
		Xeno_Enter_ActiveWait_State(sbPtr);
		/* Otherwise just wait? */
		return;
	}

	/* Now we have a target. */
	GLOBALASSERT(xenoStatusPointer->Target);
	
	/* Set up animation... Which Way?  Keep TurnToFace functionality? */
	{
		SECTION_DATA *master_section;
		VECTORCH orientationDirn;

		master_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"pelvis presley");
		GLOBALASSERT(master_section);
		
		Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&master_section->World_Offset);
	
		anglex=512;

		if (anglex>0) {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Right,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		} else {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Left,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		}
	
		#if 0
		/* Then turn to face it, of course. */

		orientationDirn.vx = xenoStatusPointer->targetTrackPos.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = xenoStatusPointer->targetTrackPos.vz - sbPtr->DynPtr->Position.vz;
		#else
		/* Synthesize a new orientationDirn. */
		orientationDirn.vx=sbPtr->DynPtr->OrientMat.mat11;
		orientationDirn.vy=sbPtr->DynPtr->OrientMat.mat12;
		orientationDirn.vz=sbPtr->DynPtr->OrientMat.mat13;
		#endif
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,(NPC_TURNRATE>>XENO_FOOT_TURN_RATE),NULL);
		
	}

	Xeno_Limbs_ShootTheRoof(sbPtr);

	if (angley<XENO_HEADTILT_GIMBALL) {
		Xeno_Enter_TurnToFace_State(sbPtr);
		return;
	}
	
}

void Execute_Xeno_Follow(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	VECTORCH velocityDirection = {0,0,0};
	VECTORCH targetPosition;
	int targetIsAirduct = 0;
	int anglex,angley;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	/* In theory, we're following the target, and can't see it. */

	if (ShowXenoStats) {
		PrintDebuggingText("In Follow.\n");
	}

	if (xenoStatusPointer->Target==NULL) {
		/* Let's wave the head around. */
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		/* Do we want to do this? */
		Xeno_HeadMovement_ScanUpDown(sbPtr,XENO_HEAD_SCAN_RATE+2);
		xenoStatusPointer->UseHeadLaser=1;
		/* And return to my module. */
		Xeno_Enter_Returning_State(sbPtr);
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target);
	/* Now we know have a target.  Can we see it yet? */

	if (xenoStatusPointer->targetSightTest==0) {
		
		AIMODULE *targetModule;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
		
		/* Can't see them.  Never mind.  Go to the next module? */
		if (xenoStatusPointer->Target->containingModule==NULL) {
			/* Fall through for now. */
			targetModule=NULL;
		} else if (GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,(xenoStatusPointer->module_range)-1,0)==NULL) {
			/* Too Far! */
			Xeno_Enter_ActiveWait_State(sbPtr);
			return;
		} else {
			/* Still in range: keep going. */
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
				xenoStatusPointer->Target->containingModule->m_aimodule,xenoStatusPointer->module_range,0);

		}

		if (targetModule==NULL) {
			/* They're way away. */
			Xeno_Enter_Returning_State(sbPtr);
			return;
		}

		if (targetModule==sbPtr->containingModule->m_aimodule) {
			/* Good Grief, Penfold!  He's right there, but I can't see him! */
			NPCGetMovementDirection(sbPtr, &velocityDirection, &xenoStatusPointer->targetTrackPos,&xenoStatusPointer->waypointManager);
			NPCSetVelocity(sbPtr, &velocityDirection, XENO_NEAR_SPEED);
			targetPosition=xenoStatusPointer->targetTrackPos;
			if (ShowXenoStats) {
				PrintDebuggingText("Direct movement - no LOS.\n");
			}
			/* Oh, well. Go to last known position? */
		} else {
			thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
			if (!thisEp) {
				LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
					(*(targetModule->m_module_ptrs))->name,
					sbPtr->containingModule->name));
				GLOBALASSERT(thisEp);
			}
			/* If that fired, there's a farped adjacency. */
			
			xenoStatusPointer->wanderData.worldPosition=thisEp->position;
			xenoStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
			xenoStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
			xenoStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
	
			NPCGetMovementDirection(sbPtr, &velocityDirection, &(xenoStatusPointer->wanderData.worldPosition),&xenoStatusPointer->waypointManager);
			NPCSetVelocity(sbPtr, &velocityDirection, XENO_NEAR_SPEED);
			targetPosition=xenoStatusPointer->wanderData.worldPosition;
		}		
	} else {
		/* Re-aquired!  Get a bit closer? */
		int range;

		if (GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,(xenoStatusPointer->module_range)-1,0)==NULL) {
			/* Too Far! */
			Xeno_Enter_ActiveWait_State(sbPtr);
			return;
		}

		range=VectorDistance((&xenoStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

		if (range>XENO_CLOSE_APPROACH_DISTANCE) {
			NPCGetMovementTarget(sbPtr, xenoStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
			NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&xenoStatusPointer->waypointManager);
			NPCSetVelocity(sbPtr, &velocityDirection, XENO_NEAR_SPEED);
			if (ShowXenoStats) {
				PrintDebuggingText("Direct movement - LOS Okay.\n");
			}
		} else {
			/* Return to ActiveWait. */
			Xeno_Enter_ActiveWait_State(sbPtr);
			return;
		}

	}
	
	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif

	if (xenoStatusPointer->targetSightTest==0) {
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		xenoStatusPointer->UseHeadLaser=1;
	} else {
		Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
	}

	/* test here for impeding collisions, and not being able to reach target... */
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(xenoStatusPointer->moveData),&xenoStatusPointer->obstruction,&destructableObject);
		#if 1
		if(xenoStatusPointer->obstruction.environment)
		{
			/* go to avoidance */
			Xeno_Enter_Avoidance_State(sbPtr);
			return;
		}
		#endif
		if(xenoStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(xenoStatusPointer->moveData), &targetPosition, &velocityDirection))
	{

		xenoStatusPointer->obstruction.environment=1;
		xenoStatusPointer->obstruction.destructableObject=0;
		xenoStatusPointer->obstruction.otherCharacter=0;
		xenoStatusPointer->obstruction.anySingleObstruction=0;
	
		Xeno_Enter_Avoidance_State(sbPtr);
		return;
	}

}

void Execute_Xeno_Return(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	VECTORCH velocityDirection = {0,0,0};
	VECTORCH *targetPosition=NULL;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	/* In theory, we're following the target, and can't see it. */

	if (ShowXenoStats) {
		PrintDebuggingText("In Return.\n");
	}

	if (xenoStatusPointer->Target!=NULL) {
		/* Saw something! */
		/* Go to active wait. */
		Xeno_Enter_ActiveWait_State(sbPtr);
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target==NULL);
	/* Find our way home. */

	{
		
		AIMODULE *targetModule;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
		
		/* Go to the next module. */
		targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,xenoStatusPointer->module_range+2,0);
		/* Just to be on the safe side. */
		
		if (targetModule==NULL) {
			/* Emergency! */
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
				xenoStatusPointer->my_module,xenoStatusPointer->module_range+5,0);
			if (targetModule==NULL) {
				/* Totally broken.  Stay here. */
				Xeno_CopeWithLossOfHome(sbPtr);
				return;			
			}
		}

		if (targetModule!=sbPtr->containingModule->m_aimodule) {

			thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
			if (!thisEp) {
				LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
					(*(targetModule->m_module_ptrs))->name,
					sbPtr->containingModule->name));
				GLOBALASSERT(thisEp);
			}
			/* If that fired, there's a farped adjacency. */
			
			xenoStatusPointer->wanderData.worldPosition=thisEp->position;
			xenoStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
			xenoStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
			xenoStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

			NPCGetMovementDirection(sbPtr, &velocityDirection, &(xenoStatusPointer->wanderData.worldPosition),&xenoStatusPointer->waypointManager);
			NPCSetVelocity(sbPtr, &velocityDirection, XENO_NEAR_SPEED);
			targetPosition=&(xenoStatusPointer->wanderData.worldPosition);
		} else {
			VECTORCH offset;
			int dist;
			/* In our own home module! */

			offset.vx=sbPtr->DynPtr->Position.vx-xenoStatusPointer->my_spot_therin.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-xenoStatusPointer->my_spot_therin.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-xenoStatusPointer->my_spot_therin.vz;
			/* Fix for midair start points, grrrr. */
			offset.vy>>=2;
	
			/* Find distance off spot. */
			dist=Approximate3dMagnitude(&offset);

			if (dist<XENO_SENTRY_SENSITIVITY) {
				Xeno_Enter_ActiveWait_State(sbPtr);
				return;			
			} else {
				NPCGetMovementDirection(sbPtr, &velocityDirection, &(xenoStatusPointer->my_spot_therin),&xenoStatusPointer->waypointManager);
				NPCSetVelocity(sbPtr, &velocityDirection, XENO_NEAR_SPEED);
				targetPosition=&(xenoStatusPointer->my_spot_therin);
			}
		}		
	}

	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif
	
	Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
	Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
	xenoStatusPointer->UseHeadLaser=1;

	/* test here for impeding collisions, and not being able to reach target... */
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(xenoStatusPointer->moveData),&xenoStatusPointer->obstruction,&destructableObject);
		#if 1
		if(xenoStatusPointer->obstruction.environment)
		{
			/* go to avoidance */
			Xeno_Enter_Avoidance_State(sbPtr);
			return;
		}
		#endif
		if(xenoStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(xenoStatusPointer->moveData), targetPosition, &velocityDirection))
	{

		xenoStatusPointer->obstruction.environment=1;
		xenoStatusPointer->obstruction.destructableObject=0;
		xenoStatusPointer->obstruction.otherCharacter=0;
		xenoStatusPointer->obstruction.anySingleObstruction=0;
	
		Xeno_Enter_Avoidance_State(sbPtr);
		return;
	}

}

void Execute_Xeno_Avoidance(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    
	int terminateState = 0;
	int anglex,angley;
	
	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);

	if (ShowXenoStats) {
		PrintDebuggingText("In Avoidance.\n");
	}

	/* Sequences... */
	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif
	
	if (xenoStatusPointer->targetSightTest==0) {
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		xenoStatusPointer->UseHeadLaser=1;
	} else {
		Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
	}
	
	/* set velocity */
	LOCALASSERT((xenoStatusPointer->moveData.avoidanceDirn.vx!=0)||
				(xenoStatusPointer->moveData.avoidanceDirn.vy!=0)||
				(xenoStatusPointer->moveData.avoidanceDirn.vz!=0));
	NPCSetVelocity(sbPtr, &(xenoStatusPointer->moveData.avoidanceDirn), (XENO_NEAR_SPEED));

	/* decrement state timer */
	xenoStatusPointer->stateTimer -= NormalFrameTime;
	if(xenoStatusPointer->stateTimer <= 0) terminateState = 1;

	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(xenoStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.anySingleObstruction)
		{
			terminateState = 1;
		}
	}

	if(terminateState)
	{
		
		/* go to an appropriate state */
		switch (xenoStatusPointer->lastState) {
			case XS_Returning:
				Xeno_Enter_Returning_State(sbPtr);
				return;
				break;
			case XS_Following:
				Xeno_Enter_Following_State(sbPtr);
				return;
				break;
			default:
				Xeno_Enter_ActiveWait_State(sbPtr);
				return;
				break;
		}
		/* Still here? */
		return;
		
	}
	return;
}

int Xenoborg_TargetFilter(STRATEGYBLOCK *candidate) {

	/* Let's face it, Xenos shoot everything but other xenos. */
	switch (candidate->I_SBtype) {
		case I_BehaviourMarinePlayer:
		case I_BehaviourAlienPlayer:
		case I_BehaviourPredatorPlayer:
			{
				if (Observer) {
					return(0);
				}

				switch(AvP.PlayerType)
				{
					case I_Alien:
					case I_Predator:
					case I_Marine:
						return(1);
						break;
					default:
						GLOBALASSERT(0);
						return(0);
						break;
				}
				break;
			}
		case I_BehaviourDummy:
			{
				DUMMY_STATUS_BLOCK *dummyStatusPointer;    
				dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(candidate->SBdataptr);    
			    LOCALASSERT(dummyStatusPointer);	          		
				switch (dummyStatusPointer->PlayerType) {
					case I_Marine:
					case I_Predator:
					case I_Alien:
						return(1);
						break;
					default:
						GLOBALASSERT(0);
						return(0);
						break;
				}
				break;
			}
		case I_BehaviourAlien:
			{
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					/* Are we inactive? */
					ALIEN_STATUS_BLOCK *asb=(ALIEN_STATUS_BLOCK *)candidate->SBdataptr;
					GLOBALASSERT(asb);
					if (asb->BehaviourState==ABS_Dormant) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
			}
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourPredator:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
		case I_BehaviourMarine:
			{
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					return(1);
				}
				break;
			}
		case I_BehaviourXenoborg:
			return(0);
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
						return(1);
						break;
					default:
						return(1);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

}

STRATEGYBLOCK *Xenoborg_GetNewTarget(VECTORCH *xenopos, STRATEGYBLOCK *me) {

	int neardist;
	STRATEGYBLOCK *nearest;
	int a;
	STRATEGYBLOCK *candidate;
	MODULE *dmod;
	
	dmod=ModuleFromPosition(xenopos,playerPherModule);
	
	LOCALASSERT(dmod);
	
	nearest=NULL;
	neardist=ONE_FIXED;
	
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate!=me) {
			if (candidate->DynPtr) {
				if (Xenoborg_TargetFilter(candidate)) {
					VECTORCH offset;
					int dist;
		
					offset.vx=xenopos->vx-candidate->DynPtr->Position.vx;
					offset.vy=xenopos->vy-candidate->DynPtr->Position.vy;
					offset.vz=xenopos->vz-candidate->DynPtr->Position.vz;
			
					dist=Approximate3dMagnitude(&offset);
		
					if (dist<neardist) {
						/* Check visibility? */
						if (NPCCanSeeTarget(me,candidate,XENO_NEAR_VIEW_WIDTH)) {
							if (!NPC_IsDead(candidate)) {
								if ((IsModuleVisibleFromModule(dmod,candidate->containingModule))) {		
									nearest=candidate;
								}	
							}
						}
					}
				}
			}
		}
	}

	#if 0
	if (nearest==NULL) {
		if (Xenoborg_TargetFilter(Player->ObStrategyBlock)) {
			nearest=Player->ObStrategyBlock;
		} else {
			nearest=NULL; /* Erk! */
		}
	}
	#endif
	
	return(nearest);

}

static void ComputeDeltaValues(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int angle;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Interpret all status block values, and apply to deltas. */
	/* Head Pan first. */

	angle=xenoStatusPointer->Head_Pan>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-4096;

	/* Now, we have an angle. */

	if (angle>XENO_HEADPAN_GIMBALL) {
		angle=XENO_HEADPAN_GIMBALL;
	} else if (angle<-XENO_HEADPAN_GIMBALL) {
		angle=-XENO_HEADPAN_GIMBALL;
	}

	#if 0
	angle=angle*2;
	
	GLOBALASSERT(xenoStatusPointer->head_pan);

	{
		int fake_timer;

		fake_timer=1024-angle;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;
	#else
	GLOBALASSERT(xenoStatusPointer->head_pan);

	{
		int fake_timer;

		fake_timer=DIV_FIXED(angle,(XENO_HEADPAN_GIMBALL<<1));
		fake_timer+=32767;
		fake_timer=65536-fake_timer;
		if (fake_timer>=65536) fake_timer=65535;
		if (fake_timer<=0) fake_timer=0;

	#endif

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->head_pan->timer=fake_timer;

	}

	/* Head tilt next. */
	angle=xenoStatusPointer->Head_Tilt>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-3072;
	if (angle> 1024) angle=2048-angle;

	/* Now, we have an angle. */

	if (angle>XENO_HEADTILT_GIMBALL) {
		angle=XENO_HEADTILT_GIMBALL;
	} else if (angle<-XENO_HEADTILT_GIMBALL) {
		angle=-XENO_HEADTILT_GIMBALL;
	}

	angle=angle*2;
	
	GLOBALASSERT(angle>=-1024);
	GLOBALASSERT(angle<=1024);

	GLOBALASSERT(xenoStatusPointer->head_tilt);
	
	{
		int fake_timer;

		fake_timer=1024-angle;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->head_tilt->timer=fake_timer;

	}

	/* Left Arm Pan now. */

	angle=xenoStatusPointer->Left_Arm_Pan>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-4096;

	GLOBALASSERT(xenoStatusPointer->left_arm_pan);

	/* Now, we have an angle. */

	if (angle>XENO_LEFTARM_ACW_GIMBALL) {
		angle=XENO_LEFTARM_ACW_GIMBALL;
	} else if (angle<-XENO_LEFTARM_CW_GIMBALL) {
		angle=-XENO_LEFTARM_CW_GIMBALL;
	}
	
	{
		int fake_timer;

		if (angle>0) {
		
			fake_timer=DIV_FIXED(angle,(XENO_LEFTARM_ACW_GIMBALL<<1));
			fake_timer+=32767;
			fake_timer=65536-fake_timer;
			if (fake_timer>=65536) fake_timer=65535;
			if (fake_timer<=0) fake_timer=0;

		} else {
		
			fake_timer=DIV_FIXED(angle,(XENO_LEFTARM_CW_GIMBALL<<1));
			fake_timer+=32767;
			fake_timer=65536-fake_timer;
			if (fake_timer>=65536) fake_timer=65535;
			if (fake_timer<=0) fake_timer=0;

		}

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->left_arm_pan->timer=fake_timer;

	}

	/* Left Arm Tilt... */

	angle=xenoStatusPointer->Left_Arm_Tilt>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-3072;
	if (angle> 1024) angle=2048-angle;

	GLOBALASSERT(xenoStatusPointer->left_arm_tilt);

	/* Now, we have an angle. */

	if (angle>XENO_ARM_PITCH_GIMBALL) {
		angle=XENO_ARM_PITCH_GIMBALL;
	} else if (angle<-XENO_ARM_PITCH_GIMBALL) {
		angle=-XENO_ARM_PITCH_GIMBALL;
	}
	
	GLOBALASSERT(angle>=-1024);
	GLOBALASSERT(angle<=1024);

	{
		int fake_timer;

		fake_timer=1024-angle;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->left_arm_tilt->timer=fake_timer;

	}

	/* Right Arm Pan now. */

	angle=xenoStatusPointer->Right_Arm_Pan>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-4096;

	GLOBALASSERT(xenoStatusPointer->right_arm_pan);

	/* Now, we have an angle. */

	if (angle>XENO_RIGHTARM_ACW_GIMBALL) {
		angle=XENO_RIGHTARM_ACW_GIMBALL;
	} else if (angle<-XENO_RIGHTARM_CW_GIMBALL) {
		angle=-XENO_RIGHTARM_CW_GIMBALL;
	}
	
	{
		int fake_timer;

		if (angle>0) {
		
			fake_timer=DIV_FIXED(angle,(XENO_RIGHTARM_ACW_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer>=65536) fake_timer=65535;

		} else {
		
			fake_timer=DIV_FIXED(angle,(XENO_RIGHTARM_CW_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer<=0) fake_timer=0;

		}

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->right_arm_pan->timer=fake_timer;

	}

	/* Right Arm Tilt... */

	angle=xenoStatusPointer->Right_Arm_Tilt>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-3072;
	if (angle> 1024) angle=2048-angle;

	GLOBALASSERT(xenoStatusPointer->right_arm_pan);

	/* Now, we have an angle. */

	if (angle>XENO_ARM_PITCH_GIMBALL) {
		angle=XENO_ARM_PITCH_GIMBALL;
	} else if (angle<-XENO_ARM_PITCH_GIMBALL) {
		angle=-XENO_ARM_PITCH_GIMBALL;
	}
	
	GLOBALASSERT(angle>=-1024);
	GLOBALASSERT(angle<=1024);

	{
		int fake_timer;

		fake_timer=1024-angle;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->right_arm_tilt->timer=fake_timer;

	}

	/* ... and Torso Twist. */

	angle=xenoStatusPointer->Torso_Twist>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-4096;

	GLOBALASSERT(xenoStatusPointer->torso_twist);

	/* Now, we have an angle. */

	if (angle>XENO_TORSO_GIMBALL) {
		angle=XENO_TORSO_GIMBALL;
	} else if (angle<-XENO_TORSO_GIMBALL) {
		angle=-XENO_TORSO_GIMBALL;
	}
	
	{
		int fake_timer;

		if (angle>0) {
		
			fake_timer=DIV_FIXED(angle,(XENO_TORSO_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer>=65536) fake_timer=65535;

		} else {
		
			fake_timer=DIV_FIXED(angle,(XENO_TORSO_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer<=0) fake_timer=0;

		}

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		xenoStatusPointer->torso_twist->timer=fake_timer;

	}

}

void Xeno_HeadMovement_ScanLeftRight(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the head around. */
	if (xenoStatusPointer->headpandir) {
		xenoStatusPointer->Head_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Pan>(XENO_HEADPAN_GIMBALL<<4)) {
			xenoStatusPointer->Head_Pan=(XENO_HEADPAN_GIMBALL<<4);
			xenoStatusPointer->headpandir=0;
		} else {
			xenoStatusPointer->head_moving=1;
		}
	} else {
		xenoStatusPointer->Head_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Pan<-(XENO_HEADPAN_GIMBALL<<4)) {
			xenoStatusPointer->Head_Pan=-(XENO_HEADPAN_GIMBALL<<4);
			xenoStatusPointer->headpandir=1;
		} else {
			xenoStatusPointer->head_moving=1;
		}
	}
	if (xenoStatusPointer->head_pan) {
		xenoStatusPointer->head_pan->Active=1;
	}

}

void Xeno_HeadMovement_ScanUpDown(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the head around. */
	if (xenoStatusPointer->headtiltdir) {
		xenoStatusPointer->Head_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Tilt>(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Head_Tilt=(XENO_HEADTILT_GIMBALL<<4);
			xenoStatusPointer->headtiltdir=0;
		} else {
			xenoStatusPointer->head_moving=1;
		}
	} else {
		xenoStatusPointer->Head_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Tilt<-(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Head_Tilt=-(XENO_HEADTILT_GIMBALL<<4);
			xenoStatusPointer->headtiltdir=1;
		} else {
			xenoStatusPointer->head_moving=1;
		}
	}
	if (xenoStatusPointer->head_tilt) {
		xenoStatusPointer->head_tilt->Active=1;
	}

}

void Xeno_LeftArmMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the left arm around. */
	if (xenoStatusPointer->leftarmpandir) {
		xenoStatusPointer->Left_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan>(XENO_LEFTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=(XENO_LEFTARM_ACW_GIMBALL<<4);
			xenoStatusPointer->leftarmpandir=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	} else {
		xenoStatusPointer->Left_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan<-(XENO_LEFTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=-(XENO_LEFTARM_CW_GIMBALL<<4);
			xenoStatusPointer->leftarmpandir=1;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	}

	if (xenoStatusPointer->left_arm_pan) {
		xenoStatusPointer->left_arm_pan->Active=1;
	}

}

void Xeno_LeftArmMovement_TrackUpDown(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the left arm around. */
	if (xenoStatusPointer->leftarmtiltdir) {
		xenoStatusPointer->Left_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->leftarmtiltdir=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	} else {
		xenoStatusPointer->Left_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->leftarmtiltdir=1;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	}

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=1;
	}

}

void Xeno_LeftArmMovement_WaveUp(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the left arm around. */
	if (xenoStatusPointer->leftarmtiltdir) {
		xenoStatusPointer->Left_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt>-(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=-(XENO_HEADTILT_GIMBALL<<4);
			xenoStatusPointer->leftarmtiltdir=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	} else {
		xenoStatusPointer->Left_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->leftarmtiltdir=1;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	}

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=1;
	}

}

void Xeno_RightArmMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the Right arm around. */
	if (xenoStatusPointer->rightarmpandir) {
		xenoStatusPointer->Right_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan>(XENO_RIGHTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=(XENO_RIGHTARM_ACW_GIMBALL<<4);
			xenoStatusPointer->rightarmpandir=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	} else {
		xenoStatusPointer->Right_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan<-(XENO_RIGHTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=-(XENO_RIGHTARM_CW_GIMBALL<<4);
			xenoStatusPointer->rightarmpandir=1;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_pan) {
		xenoStatusPointer->right_arm_pan->Active=1;
	}

}

void Xeno_RightArmMovement_TrackUpDown(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the Right arm around. */
	if (xenoStatusPointer->rightarmtiltdir) {
		xenoStatusPointer->Right_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->rightarmtiltdir=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	} else {
		xenoStatusPointer->Right_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->rightarmtiltdir=1;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

}

void Xeno_RightArmMovement_WaveUp(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's wave the Right arm around. */
	if (xenoStatusPointer->rightarmtiltdir) {
		xenoStatusPointer->Right_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt>-(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=-(XENO_HEADTILT_GIMBALL<<4);
			xenoStatusPointer->rightarmtiltdir=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	} else {
		xenoStatusPointer->Right_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
			xenoStatusPointer->rightarmtiltdir=1;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

}

void Xeno_TorsoMovement_TrackLeftRight(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Let's twist the torso. */
	if (xenoStatusPointer->torsotwistdir) {
		xenoStatusPointer->Torso_Twist+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist>(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=(XENO_TORSO_GIMBALL<<4);
			xenoStatusPointer->torsotwistdir=0;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	} else {
		xenoStatusPointer->Torso_Twist-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist<-(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=-(XENO_TORSO_GIMBALL<<4);
			xenoStatusPointer->torsotwistdir=1;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

}

int Xeno_HeadMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	int real_anglex,angley,online;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Turn the head to face a certain way. */

	real_anglex=in_anglex-(xenoStatusPointer->Torso_Twist>>4);
	angley=in_angley;
	online=0;

	/* Now fix multiples. */
	while ((real_anglex>4095)||(real_anglex<0)) {
		if (real_anglex<0) {
			real_anglex+=4096;
		} else if (real_anglex>4095) {
			real_anglex-=4096;
		}
	}
	
	if (real_anglex>=3072) real_anglex-=4096;
	if (real_anglex>=2048) real_anglex=real_anglex-3072;
	if (real_anglex> 1024) real_anglex=2048-real_anglex;

	if (angley>=3072) angley-=4096;
	if (angley>=2048) angley=angley-3072;
	if (angley> 1024) angley=2048-angley;

	GLOBALASSERT((real_anglex<=1024)&&(real_anglex>=-1024));
	GLOBALASSERT((angley<=1024)&&(angley>=-1024));

	if (ShowXenoStats) {
		PrintDebuggingText("Target head angles: %d %d\n",real_anglex,angley);
	}

	real_anglex<<=4;
	angley<<=4;

	if (xenoStatusPointer->Head_Pan<real_anglex) {
		xenoStatusPointer->Head_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Pan>(XENO_HEADPAN_GIMBALL<<4)) {
			xenoStatusPointer->Head_Pan=(XENO_HEADPAN_GIMBALL<<4);
		} else if (xenoStatusPointer->Head_Pan>real_anglex) {
			xenoStatusPointer->Head_Pan=real_anglex;
			online++;
		}
	} else if (xenoStatusPointer->Head_Pan>real_anglex) {
		xenoStatusPointer->Head_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Pan<-(XENO_HEADPAN_GIMBALL<<4)) {
			xenoStatusPointer->Head_Pan=-(XENO_HEADPAN_GIMBALL<<4);
		} else if (xenoStatusPointer->Head_Pan<real_anglex) {
			xenoStatusPointer->Head_Pan=real_anglex;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->head_pan) {
		xenoStatusPointer->head_pan->Active=1;
	}

	/* Now y. */
	angley=-angley;
	/* Oops. */

	if (xenoStatusPointer->Head_Tilt<angley) {
		xenoStatusPointer->Head_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Tilt>(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Head_Tilt=(XENO_HEADTILT_GIMBALL<<4);
		} else if (xenoStatusPointer->Head_Tilt>angley) {
			xenoStatusPointer->Head_Tilt=angley;
			online++;
		}
	} else if (xenoStatusPointer->Head_Tilt>angley) {
		xenoStatusPointer->Head_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Head_Tilt<-(XENO_HEADTILT_GIMBALL<<4)) {
			xenoStatusPointer->Head_Tilt=-(XENO_HEADTILT_GIMBALL<<4);
		} else if (xenoStatusPointer->Head_Tilt<angley) {
			xenoStatusPointer->Head_Tilt=angley;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->head_tilt) {
		xenoStatusPointer->head_tilt->Active=1;
	}

	if (online<=1) {
		/* Still moving! */
		xenoStatusPointer->head_moving=1;
	}
		
	if (xenoStatusPointer->HeadLaserOnTarget) {
		online=2;
	}

	if (online>1) {
		return(1);
	} else {
		return(0);
	}
}

void Xeno_TorsoMovement_TrackToAngle(STRATEGYBLOCK *sbPtr,int rate,int in_anglex)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	int anglex;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Turn the torso to face a certain way. No angley here. */

	anglex=in_anglex;

	if (anglex>=3072) anglex-=4096;
	if (anglex>=2048) anglex=anglex-3072;
	if (anglex> 1024) anglex=2048-anglex;

	anglex<<=4;

	if (xenoStatusPointer->Torso_Twist<anglex) {
		xenoStatusPointer->Torso_Twist+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist>(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=(XENO_TORSO_GIMBALL<<4);
		} else if (xenoStatusPointer->Torso_Twist>anglex) {
			xenoStatusPointer->Torso_Twist=anglex;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	} else if (xenoStatusPointer->Torso_Twist>anglex) {
		xenoStatusPointer->Torso_Twist-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist<-(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=-(XENO_TORSO_GIMBALL<<4);
		} else if (xenoStatusPointer->Torso_Twist<anglex) {
			xenoStatusPointer->Torso_Twist=anglex;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	}

	if (xenoStatusPointer->torso_twist) {
		xenoStatusPointer->torso_twist->Active=1;
	}

}

void Xeno_TorsoMovement_Centre(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Turn the torso to face a certain way. No angley here. */

	if (xenoStatusPointer->Torso_Twist<0) {
		xenoStatusPointer->Torso_Twist+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist>(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=(XENO_TORSO_GIMBALL<<4);
		} else if (xenoStatusPointer->Torso_Twist>0) {
			xenoStatusPointer->Torso_Twist=0;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	} else if (xenoStatusPointer->Torso_Twist>0) {
		xenoStatusPointer->Torso_Twist-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Torso_Twist<-(XENO_TORSO_GIMBALL<<4)) {
			xenoStatusPointer->Torso_Twist=-(XENO_TORSO_GIMBALL<<4);
		} else if (xenoStatusPointer->Torso_Twist<0) {
			xenoStatusPointer->Torso_Twist=0;
		} else {
			xenoStatusPointer->torso_moving=1;
		}
	}

	if (xenoStatusPointer->torso_twist) {
		xenoStatusPointer->torso_twist->Active=1;
	}

}

int Xeno_LeftArmMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	int real_anglex,angley,online;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Aim the Left Arm at a point. */

	real_anglex=in_anglex-(xenoStatusPointer->Torso_Twist>>4);
	angley=in_angley;
	online=0;

	/* Now fix multiples. */
	while ((real_anglex>4095)||(real_anglex<0)) {
		if (real_anglex<0) {
			real_anglex+=4096;
		} else if (real_anglex>4095) {
			real_anglex-=4096;
		}
	}
	
	if (real_anglex>=3072) real_anglex-=4096;
	if (real_anglex>=2048) real_anglex=real_anglex-3072;
	if (real_anglex> 1024) real_anglex=2048-real_anglex;

	if (angley>=3072) angley-=4096;
	if (angley>=2048) angley=angley-3072;
	if (angley> 1024) angley=2048-angley;

	real_anglex<<=4;
	angley<<=4;

	if (xenoStatusPointer->Left_Arm_Pan<real_anglex) {
		xenoStatusPointer->Left_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan>(XENO_LEFTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=(XENO_LEFTARM_ACW_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Pan>real_anglex) {
			xenoStatusPointer->Left_Arm_Pan=real_anglex;
			online++;
		}
	} else if (xenoStatusPointer->Left_Arm_Pan>real_anglex) {
		xenoStatusPointer->Left_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan<-(XENO_LEFTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=-(XENO_LEFTARM_CW_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Pan<real_anglex) {
			xenoStatusPointer->Left_Arm_Pan=real_anglex;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->left_arm_pan) {
		xenoStatusPointer->left_arm_pan->Active=1;
	}

	/* Now y. */
	angley=-angley;
	/* Oops. */

	if (xenoStatusPointer->Left_Arm_Tilt<angley) {
		xenoStatusPointer->Left_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Tilt>angley) {
			xenoStatusPointer->Left_Arm_Tilt=angley;
			online++;
		}
	} else if (xenoStatusPointer->Left_Arm_Tilt>angley) {
		xenoStatusPointer->Left_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Tilt<angley) {
			xenoStatusPointer->Left_Arm_Tilt=angley;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=1;
	}

	if (online<=1) {
		/* Still going! */
		xenoStatusPointer->la_moving=1;
	}

	if (xenoStatusPointer->HeadLaserOnTarget) {
		xenoStatusPointer->UseLALaser=1;
	}

	if (xenoStatusPointer->LALaserOnTarget) {
		online=2;
	}

	if (online>1) {
		if (xenoStatusPointer->Target) {
			/* What the heck! */
			xenoStatusPointer->FiringLeft=1;
		}
		return(1);
	} else {
		return(0);
	}

}

void Xeno_LeftArmMovement_Centre(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Centre the Left Arm. */

	if (xenoStatusPointer->Left_Arm_Pan<0) {
		xenoStatusPointer->Left_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan>(XENO_LEFTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=(XENO_LEFTARM_ACW_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Pan>0) {
			xenoStatusPointer->Left_Arm_Pan=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	} else if (xenoStatusPointer->Left_Arm_Pan>0) {
		xenoStatusPointer->Left_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Pan<-(XENO_LEFTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Pan=-(XENO_LEFTARM_CW_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Pan<0) {
			xenoStatusPointer->Left_Arm_Pan=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	}

	if (xenoStatusPointer->left_arm_pan) {
		xenoStatusPointer->left_arm_pan->Active=1;
	}

	/* Now y. */

	if (xenoStatusPointer->Left_Arm_Tilt<0) {
		xenoStatusPointer->Left_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Tilt>0) {
			xenoStatusPointer->Left_Arm_Tilt=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	} else if (xenoStatusPointer->Left_Arm_Tilt>0) {
		xenoStatusPointer->Left_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Left_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Left_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Left_Arm_Tilt<0) {
			xenoStatusPointer->Left_Arm_Tilt=0;
		} else {
			xenoStatusPointer->la_moving=1;
		}
	}

	if (xenoStatusPointer->left_arm_tilt) {
		xenoStatusPointer->left_arm_tilt->Active=1;
	}

	return;

}

int Xeno_RightArmMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	
	int real_anglex,angley,online;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Aim the Right Arm at a point. */

	real_anglex=in_anglex-(xenoStatusPointer->Torso_Twist>>4)+RATweak;
	angley=in_angley;
	online=0;

	/* Now fix multiples. */
	while ((real_anglex>4095)||(real_anglex<0)) {
		if (real_anglex<0) {
			real_anglex+=4096;
		} else if (real_anglex>4095) {
			real_anglex-=4096;
		}
	}
	
	if (real_anglex>=3072) real_anglex-=4096;
	if (real_anglex>=2048) real_anglex=real_anglex-3072;
	if (real_anglex> 1024) real_anglex=2048-real_anglex;

	if (angley>=3072) angley-=4096;
	if (angley>=2048) angley=angley-3072;
	if (angley> 1024) angley=2048-angley;

	real_anglex<<=4;
	angley<<=4;

	if (xenoStatusPointer->Right_Arm_Pan<real_anglex) {
		xenoStatusPointer->Right_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan>(XENO_RIGHTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=(XENO_RIGHTARM_ACW_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Pan>real_anglex) {
			xenoStatusPointer->Right_Arm_Pan=real_anglex;
			online++;
		}
	} else if (xenoStatusPointer->Right_Arm_Pan>real_anglex) {
		xenoStatusPointer->Right_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan<-(XENO_RIGHTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=-(XENO_RIGHTARM_CW_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Pan<real_anglex) {
			xenoStatusPointer->Right_Arm_Pan=real_anglex;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->right_arm_pan) {
		xenoStatusPointer->right_arm_pan->Active=1;
	}

	/* Now y. */
	angley=-angley;
	/* Oops. */

	if (xenoStatusPointer->Right_Arm_Tilt<angley) {
		xenoStatusPointer->Right_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Tilt>angley) {
			xenoStatusPointer->Right_Arm_Tilt=angley;
			online++;
		}
	} else if (xenoStatusPointer->Right_Arm_Tilt>angley) {
		xenoStatusPointer->Right_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Tilt<angley) {
			xenoStatusPointer->Right_Arm_Tilt=angley;
			online++;
		}
	} else {
		online++;
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

	if (online<=1) {
		/* Still moving! */
		xenoStatusPointer->ra_moving=1;
	}

	if (xenoStatusPointer->HeadLaserOnTarget) {
		xenoStatusPointer->UseRALaser=1;
	}

	if (xenoStatusPointer->RALaserOnTarget) {
		online=2;
	}

	if (online>1) {
		if (xenoStatusPointer->Target) {
			/* What the heck! */
			xenoStatusPointer->FiringRight=1;
		}
		return(1);
	} else {
		return(0);
	}

}

void Xeno_RightArmMovement_Centre(STRATEGYBLOCK *sbPtr,int rate)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Centre the Right Arm. */

	if (xenoStatusPointer->Right_Arm_Pan<0) {
		xenoStatusPointer->Right_Arm_Pan+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan>(XENO_RIGHTARM_ACW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=(XENO_RIGHTARM_ACW_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Pan>0) {
			xenoStatusPointer->Right_Arm_Pan=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	} else if (xenoStatusPointer->Right_Arm_Pan>0) {
		xenoStatusPointer->Right_Arm_Pan-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Pan<-(XENO_RIGHTARM_CW_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Pan=-(XENO_RIGHTARM_CW_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Pan<0) {
			xenoStatusPointer->Right_Arm_Pan=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_pan) {
		xenoStatusPointer->right_arm_pan->Active=1;
	}

	/* Now y. */

	if (xenoStatusPointer->Right_Arm_Tilt<0) {
		xenoStatusPointer->Right_Arm_Tilt+=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt>(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Tilt>0) {
			xenoStatusPointer->Right_Arm_Tilt=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	} else if (xenoStatusPointer->Right_Arm_Tilt>0) {
		xenoStatusPointer->Right_Arm_Tilt-=(NormalFrameTime>>rate);
		if (xenoStatusPointer->Right_Arm_Tilt<-(XENO_ARM_PITCH_GIMBALL<<4)) {
			xenoStatusPointer->Right_Arm_Tilt=-(XENO_ARM_PITCH_GIMBALL<<4);
		} else if (xenoStatusPointer->Right_Arm_Tilt<0) {
			xenoStatusPointer->Right_Arm_Tilt=0;
		} else {
			xenoStatusPointer->ra_moving=1;
		}
	}

	if (xenoStatusPointer->right_arm_tilt) {
		xenoStatusPointer->right_arm_tilt->Active=1;
	}

	return;

}

int XenoActivation_FrustrumReject(VECTORCH *localOffset) {

	if ( (localOffset->vz <0) 
		&& (localOffset->vz <  localOffset->vx) 
		&& (localOffset->vz < -localOffset->vx) 
		&& (localOffset->vz <  localOffset->vy) 
		&& (localOffset->vz < -localOffset->vy) ) {
	
		/* 90 horizontal, 90 vertical. */
		return(1);
	} else {
		return(0);
	}

}

int XenoSight_FrustrumReject(STRATEGYBLOCK *sbPtr,VECTORCH *localOffset) {

	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	if ( (localOffset->vx>0) ) {
		/* 180 horizontal, 180 vertical. */
		return(1);
	} else {
		if ((xenoStatusPointer->Target)||(xenoStatusPointer->ShotThisFrame)) {
			return(1);
		} else {
			return(0);
		}
	}

}

void Xeno_UpdateTargetTrackPos(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;    	

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	if (xenoStatusPointer->Target==NULL) {
		xenoStatusPointer->targetTrackPos.vx=0;
		xenoStatusPointer->targetTrackPos.vy=0;
		xenoStatusPointer->targetTrackPos.vz=0;
		return;
	}

	GetTargetingPointOfObject_Far(xenoStatusPointer->Target,&xenoStatusPointer->targetTrackPos);

}

static void ProcessFarXenoborgTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
	NPC_TARGETMODULESTATUS targetStatus;
	XENO_STATUS_BLOCK *xenoStatusPointer;    
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(targetModule);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);	    
	LOCALASSERT(xenoStatusPointer);

	targetStatus = GetTargetAIModuleStatus(sbPtr, targetModule,0);
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
			/* loacate to target */
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_LiftTeleport):
		{
			/* do nothing */
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_ProxDoorOpen):
		{
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
			//FarNpc_FlipAround(sbPtr);
			/* What the hell!!! */
			LocateFarNPCInAIModule(sbPtr, targetModule);
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
		}
	}		
}

void Execute_Xeno_ActiveWait_Far(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int anglex,angley,correctlyOrientated;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* What to do?  Do we have a target? */

	if (ShowXenoStats) {
		PrintDebuggingText("In ActiveWait Far.\n");
	}

	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED,(ONE_FIXED>>2));

	if (xenoStatusPointer->Target==NULL) {
		#if FAR_XENO_ACTIVITY
		/* Let's wave the head around. */
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		Xeno_HeadMovement_ScanUpDown(sbPtr,XENO_HEAD_SCAN_RATE+2);
		#endif

		if (sbPtr->containingModule->m_aimodule!=xenoStatusPointer->my_module) {
			Xeno_Enter_Returning_State(sbPtr);
			return;
		}

		xenoStatusPointer->stateTimer+=NormalFrameTime;
		if (xenoStatusPointer->stateTimer>xenoStatusPointer->UpTime) {
			Xeno_Enter_PowerDown_State(sbPtr);
		}

		/* Are we at home? */
		if (sbPtr->containingModule->m_aimodule!=xenoStatusPointer->my_module) {
			Xeno_Enter_Returning_State(sbPtr);
			return;
		}
		/* Are we facing the right way? */

		correctlyOrientated = NPCOrientateToVector(sbPtr, &xenoStatusPointer->my_orientdir_therin,ONE_FIXED,NULL);
		
		if (!correctlyOrientated) {

			SECTION_DATA *master_section;
			
			master_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"pelvis presley");
			GLOBALASSERT(master_section);
			
			Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&master_section->World_Offset);
	
			if (anglex>0) {
				EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Right,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
			} else {
				EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Left,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
			}
			#if FAR_XENO_ACTIVITY
			if (xenoStatusPointer->soundHandle1==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_LOADMOVE,"del",&sbPtr->DynPtr->Position,&xenoStatusPointer->soundHandle1);
			}
			#endif
		} else {

			/* Otherwise just wait? */
			if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
				/* Well, it shouldn't be! */
				Sound_Stop(xenoStatusPointer->soundHandle1);
			}
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED,(ONE_FIXED>>2));
			xenoStatusPointer->stateTimer+=NormalFrameTime;
			if (xenoStatusPointer->stateTimer>xenoStatusPointer->UpTime) {
				Xeno_Enter_PowerDown_State(sbPtr);
			}
		}
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target);
	/* Now we have a target.  Can we see it? */
	if (xenoStatusPointer->targetSightTest==0) {
		/* Can't see them.  Are we out of range? */	
		if (GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,(xenoStatusPointer->module_range)-1,0)==NULL) {
			Xeno_Enter_Returning_State(sbPtr);
		} else {
			Xeno_Enter_Following_State(sbPtr);
		}
		return;
	}

	#if FAR_XENO_ACTIVITY
	Xeno_TurnAndTarget(sbPtr,&anglex,&angley);

	if ((anglex>(((XENO_HEADPAN_GIMBALL)*7)/8))
		||(anglex<-(((XENO_HEADPAN_GIMBALL)*7)/8))) {

		Xeno_Enter_TurnToFace_State(sbPtr);

	}
	#endif

	if (ShowXenoStats) {
		PrintDebuggingText("Targets in module....\n");
	}

}

void Execute_Xeno_TurnToFace_Far(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	int correctlyOrientated;
	int anglex,angley;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* Do we have a target? */

	if (ShowXenoStats) {
		PrintDebuggingText("In Turn To Face Far.\n");
	}

	if (xenoStatusPointer->Target==NULL) {
		Xeno_Enter_ActiveWait_State(sbPtr);
		/* Otherwise just wait? */
		return;
	}

	/* Now we have a target. */
	GLOBALASSERT(xenoStatusPointer->Target);
	
	/* Set up animation... Which Way? */
	{
		SECTION_DATA *master_section;
		VECTORCH orientationDirn;

		master_section=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"pelvis presley");
		GLOBALASSERT(master_section);
		
		Xenoborg_GetRelativeAngles(sbPtr,&anglex,&angley,&master_section->World_Offset);
	
		if (anglex<2048) {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Right,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		} else {
			EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Turn_Left,XENO_TURNING_ANIM_SPEED,(ONE_FIXED>>2));
		}
	
		/* Then turn to face it, of course. */

		orientationDirn.vx = xenoStatusPointer->targetTrackPos.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = xenoStatusPointer->targetTrackPos.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,ONE_FIXED,NULL);
		/* Spin FAST. */		
	}

	#if FAR_XENO_ACTIVITY
	Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
	#endif

	if (correctlyOrientated) {
		Xeno_Enter_ActiveWait_State(sbPtr);
	}
	
}

void Execute_Xeno_Follow_Far(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
#if FAR_XENO_ACTIVITY
	int anglex,angley;
#endif

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	/* In theory, we're following the target, and can't see it. */

	if (ShowXenoStats) {
		PrintDebuggingText("In Follow Far.\n");
	}

	if (xenoStatusPointer->Target==NULL) {
		/* Let's wave the head around. */
		#if FAR_XENO_ACTIVITY
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		Xeno_HeadMovement_ScanUpDown(sbPtr,XENO_HEAD_SCAN_RATE+2);
		#endif
		/* And return to my module. */
		Xeno_Enter_Returning_State(sbPtr);
		return;
	}

	/* Increment the Far state timer */
	xenoStatusPointer->stateTimer += NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(xenoStatusPointer->stateTimer < XENO_FAR_MOVE_TIME) {
		#if FAR_XENO_ACTIVITY
		Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
		#endif
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target);
	/* Now we know have a target.  Can we see it yet? */

	if (xenoStatusPointer->targetSightTest==0) {
		
		AIMODULE *targetModule;
		
		/* Can't see them.  Never mind.  Go to the next module? */
		if (xenoStatusPointer->Target->containingModule==NULL) {
			/* Fall through for now. */
			targetModule=NULL;
		} else if (GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,(xenoStatusPointer->module_range)-1,0)==NULL) {
			/* Too Far! */
			Xeno_Enter_ActiveWait_State(sbPtr);
			return;
		} else {
			/* Still in range: keep going. */
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
				xenoStatusPointer->Target->containingModule->m_aimodule,xenoStatusPointer->module_range,0);

		}

		if (targetModule==NULL) {
			/* They're way away. */
			Xeno_Enter_Returning_State(sbPtr);
			return;
		}

		if (targetModule!=xenoStatusPointer->Target->containingModule->m_aimodule) {

			GLOBALASSERT(targetModule);
			ProcessFarXenoborgTargetModule(sbPtr,targetModule);

		} else {
			/* In our target's module! */

			Xeno_Enter_ActiveWait_State(sbPtr);

		}		
		
	} else {
		/* Re-aquired! */
		Xeno_Enter_ActiveWait_State(sbPtr);
		return;
	}

	xenoStatusPointer->stateTimer=0;

	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif
	
	#if FAR_XENO_ACTIVITY
	if (xenoStatusPointer->targetSightTest==0) {
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
	} else {
		Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
	}
	#endif

}

void Execute_Xeno_Return_Far(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	/* In theory, we're following the target, and can't see it. */

	if (ShowXenoStats) {
		PrintDebuggingText("In Return Far.\n");
	}

	if (xenoStatusPointer->Target!=NULL) {
		/* Saw something! */
		/* Go to active wait. */
		Xeno_Enter_ActiveWait_State(sbPtr);
		return;
	}

	/* Increment the Far state timer */
	xenoStatusPointer->stateTimer += NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(xenoStatusPointer->stateTimer < XENO_FAR_MOVE_TIME) {
		#if FAR_XENO_ACTIVITY
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		#endif
		return;
	}

	GLOBALASSERT(xenoStatusPointer->Target==NULL);
	/* Find our way home. */

	{
		
		AIMODULE *targetModule;
		
		/* Go to the next module. */
		targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
			xenoStatusPointer->my_module,xenoStatusPointer->module_range+2,0);
		/* Just to be on the safe side. */

		if (targetModule==NULL) {
			/* Emergency! */
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,
				xenoStatusPointer->my_module,xenoStatusPointer->module_range+5,0);
			if (targetModule==NULL) {
				/* Totally broken.  Stay here. */
				Xeno_CopeWithLossOfHome(sbPtr);
				return;			
			}
		}

		if (targetModule!=sbPtr->containingModule->m_aimodule) {

			GLOBALASSERT(targetModule);
			ProcessFarXenoborgTargetModule(sbPtr,targetModule);

		} else {
			/* In our own home module! */

			sbPtr->DynPtr->Position=xenoStatusPointer->my_spot_therin;

			Xeno_Enter_ActiveWait_State(sbPtr);

		}		
	}

	xenoStatusPointer->stateTimer=0;

	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif
	
	#if FAR_XENO_ACTIVITY
	Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
	Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
	Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
	#endif

}

void Execute_Xeno_Avoidance_Far(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;    
#if FAR_XENO_ACTIVITY
	int anglex,angley;
#endif
	
	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(xenoStatusPointer);

	if (ShowXenoStats) {
		PrintDebuggingText("In Avoidance Far.\n");
	}

	/* Sequences... */
	#if 0
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));
	#else
	XenoborgHandleMovingAnimation(sbPtr);
	#endif
	
	#if FAR_XENO_ACTIVITY
	if (xenoStatusPointer->targetSightTest==0) {
		Xeno_TorsoMovement_Centre(sbPtr,XENO_TORSO_TWIST_RATE);
		Xeno_LeftArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_RightArmMovement_Centre(sbPtr,XENO_ARM_LOCK_RATE);
		Xeno_HeadMovement_ScanLeftRight(sbPtr,XENO_HEAD_SCAN_RATE);
	} else {
		Xeno_TurnAndTarget(sbPtr,&anglex,&angley);
	}
	#endif

	{
		
		/* go to an appropriate state */
		switch (xenoStatusPointer->lastState) {
			case XS_Returning:
				Xeno_Enter_Returning_State(sbPtr);
				return;
				break;
			case XS_Following:
				Xeno_Enter_Following_State(sbPtr);
				return;
				break;
			default:
				Xeno_Enter_ActiveWait_State(sbPtr);
				return;
				break;
		}
		/* Still here? */
		return;
		
	}
	return;
}

#define FIRING_RATE_LEFT	25

void Xenoborg_MaintainLeftGun(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *left_dum;
	VECTORCH alpha;
	VECTORCH beta;
	int multiple;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	left_dum=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"flash dummy A");

	if (xenoStatusPointer->Wounds&section_flag_left_hand) {
		xenoStatusPointer->FiringLeft=0;
	}

	if ((xenoStatusPointer->FiringLeft==0)||(left_dum==NULL)||(xenoStatusPointer->IAmFar)) {
		/* Not firing, go away. */
		xenoStatusPointer->LeftMainBeam.BeamIsOn = 0;
		return;
	}
	
	/* Okay, must be firing.  Did we get anyone? */

	multiple=FIRING_RATE_LEFT*NormalFrameTime;
	
	#if 0
	LOS_Lambda = NPC_MAX_VIEWRANGE;
	LOS_ObjectHitPtr = 0;
	LOS_HModel_Section=NULL;
	{
	   	extern int NumActiveBlocks;
		extern DISPLAYBLOCK* ActiveBlockList[];
	   	int numberOfObjects = NumActiveBlocks;
		
	   	while (numberOfObjects--)
		{
			DISPLAYBLOCK* objectPtr = ActiveBlockList[numberOfObjects];
			
			alpha = left_dum->World_Offset;

			beta.vx=left_dum->SecMat.mat31;
			beta.vy=left_dum->SecMat.mat32;
			beta.vz=left_dum->SecMat.mat33;

			GLOBALASSERT(objectPtr);

			if (objectPtr!=sbPtr->SBdptr) {
				/* Can't hit self. */
				CheckForVectorIntersectionWith3dObject(objectPtr, &alpha, &beta,1);
			}
		}
	}
	#else
	{
		alpha = left_dum->World_Offset;

		beta.vx=left_dum->SecMat.mat31;
		beta.vy=left_dum->SecMat.mat32;
		beta.vz=left_dum->SecMat.mat33;
		FindPolygonInLineOfSight(&beta,&alpha,0,sbPtr->SBdptr);
	}
	#endif
	
	/* Now deal with LOS_ObjectHitPtr. */
	if (LOS_ObjectHitPtr) {
		if (LOS_HModel_Section) {
			if (LOS_ObjectHitPtr->ObStrategyBlock) {
				if (LOS_ObjectHitPtr->ObStrategyBlock->SBdptr) {
					GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
				}
			}
		}
		/* this fn needs updating to take amount of damage into account etc. */
		HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AMMO_XENOBORG,&beta, multiple, LOS_HModel_Section);
	}

	/* Cheat for killing far targets? */
	if (xenoStatusPointer->Target) {
		if (xenoStatusPointer->Target->SBdptr==NULL) {
			HandleWeaponImpact(&xenoStatusPointer->Target->DynPtr->Position,xenoStatusPointer->Target,AMMO_XENOBORG,&beta, multiple, NULL);
		}
	}
	
	/* update beam SFX data */
	xenoStatusPointer->LeftMainBeam.BeamIsOn = 1;
	xenoStatusPointer->LeftMainBeam.SourcePosition = left_dum->World_Offset;

	if (LOS_ObjectHitPtr) {
		xenoStatusPointer->LeftMainBeam.TargetPosition = LOS_Point;
	} else {
		/* Must be hitting nothing... */
		xenoStatusPointer->LeftMainBeam.TargetPosition=alpha;
		xenoStatusPointer->LeftMainBeam.TargetPosition.vx+=(beta.vx>>3);
		xenoStatusPointer->LeftMainBeam.TargetPosition.vy+=(beta.vy>>3);
		xenoStatusPointer->LeftMainBeam.TargetPosition.vz+=(beta.vz>>3);
	}

	if (LOS_ObjectHitPtr==Player)
	{
		xenoStatusPointer->LeftMainBeam.BeamHasHitPlayer = 1;
	}
	else
	{
		xenoStatusPointer->LeftMainBeam.BeamHasHitPlayer = 0;
	}
}

#define FIRING_RATE_RIGHT	25

void Xenoborg_MaintainRightGun(STRATEGYBLOCK *sbPtr)
{
	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *right_dum;
	VECTORCH alpha;
	VECTORCH beta;
	int multiple;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);
	
	right_dum=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"flash dummy B");

	if (xenoStatusPointer->Wounds&section_flag_right_hand) {
		xenoStatusPointer->FiringRight=0;
	}

	if ((xenoStatusPointer->FiringRight==0)||(right_dum==NULL)||(xenoStatusPointer->IAmFar)) {
		/* Not firing, go away. */
		xenoStatusPointer->RightMainBeam.BeamIsOn = 0;
		return;
	}
	
	/* Okay, must be firing.  Did we get anyone? */

	multiple=FIRING_RATE_RIGHT*NormalFrameTime;

	#if 0
	LOS_Lambda = NPC_MAX_VIEWRANGE;
	LOS_ObjectHitPtr = 0;
	LOS_HModel_Section=NULL;
	{
	   	extern int NumActiveBlocks;
		extern DISPLAYBLOCK* ActiveBlockList[];
	   	int numberOfObjects = NumActiveBlocks;
		
	   	while (numberOfObjects--)
		{
			DISPLAYBLOCK* objectPtr = ActiveBlockList[numberOfObjects];
			
			alpha = right_dum->World_Offset;

			beta.vx=right_dum->SecMat.mat31;
			beta.vy=right_dum->SecMat.mat32;
			beta.vz=right_dum->SecMat.mat33;

			GLOBALASSERT(objectPtr);

			if (objectPtr!=sbPtr->SBdptr) {
				/* Can't hit self. */
				CheckForVectorIntersectionWith3dObject(objectPtr, &alpha, &beta,1);
			}
		}
	}
	#else
	{
		alpha = right_dum->World_Offset;

		beta.vx=right_dum->SecMat.mat31;
		beta.vy=right_dum->SecMat.mat32;
		beta.vz=right_dum->SecMat.mat33;
		FindPolygonInLineOfSight(&beta,&alpha,0,sbPtr->SBdptr);
	}
	#endif
	/* Now deal with LOS_ObjectHitPtr. */
	if (LOS_ObjectHitPtr) {
		if (LOS_HModel_Section) {
			if (LOS_ObjectHitPtr->ObStrategyBlock) {
				if (LOS_ObjectHitPtr->ObStrategyBlock->SBdptr) {
					GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
				}
			}
		}
		/* this fn needs updating to take amount of damage into account etc. */
		HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AMMO_XENOBORG,&beta, multiple, LOS_HModel_Section);
	}

	/* Cheat for killing far targets? */
	if (xenoStatusPointer->Target) {
		if (xenoStatusPointer->Target->SBdptr==NULL) {
			HandleWeaponImpact(&xenoStatusPointer->Target->DynPtr->Position,xenoStatusPointer->Target,AMMO_XENOBORG,&beta, multiple, NULL);
		}
	}

	/* update beam SFX data */
	xenoStatusPointer->RightMainBeam.BeamIsOn = 1;
	xenoStatusPointer->RightMainBeam.SourcePosition = right_dum->World_Offset;

	if (LOS_ObjectHitPtr) {
		xenoStatusPointer->RightMainBeam.TargetPosition = LOS_Point;
	} else {
		/* Must be hitting nothing... */
		xenoStatusPointer->RightMainBeam.TargetPosition=alpha;
		xenoStatusPointer->RightMainBeam.TargetPosition.vx+=(beta.vx>>3);
		xenoStatusPointer->RightMainBeam.TargetPosition.vy+=(beta.vy>>3);
		xenoStatusPointer->RightMainBeam.TargetPosition.vz+=(beta.vz>>3);
	}

	if (LOS_ObjectHitPtr==Player)
	{
		xenoStatusPointer->RightMainBeam.BeamHasHitPlayer = 1;
	}
	else
	{
		xenoStatusPointer->RightMainBeam.BeamHasHitPlayer = 0;
	}

}

int Xeno_Activation_Test(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	STRATEGYBLOCK *candidate;
	MATRIXCH WtoL;
	int a;
	MODULE *dmod;
	
	dmod=ModuleFromPosition(&sbPtr->DynPtr->Position,playerPherModule);
	
	LOCALASSERT(dmod);

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	WtoL=sbPtr->DynPtr->OrientMat;
	TransposeMatrixCH(&WtoL);

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate!=sbPtr) {
			if (candidate->DynPtr) {
				if (Xenoborg_TargetFilter(candidate)) {
					VECTORCH offset;
		
					offset.vx=sbPtr->DynPtr->Position.vx-candidate->DynPtr->Position.vx;
					offset.vy=sbPtr->DynPtr->Position.vy-candidate->DynPtr->Position.vy;
					offset.vz=sbPtr->DynPtr->Position.vz-candidate->DynPtr->Position.vz;
			
					RotateVector(&offset,&WtoL);
					
					if (XenoActivation_FrustrumReject(&offset)) {
						/* Check visibility? */
						if (NPCCanSeeTarget(sbPtr,candidate,XENO_NEAR_VIEW_WIDTH)) {
							if (!NPC_IsDead(candidate)) {
								if ((IsModuleVisibleFromModule(dmod,candidate->containingModule))) {		
									return(1);
								}	
							}
						}
					}
				}
			}
		}
	}
	return(0);
}

void Xeno_MaintainLasers(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *dum;
	VECTORCH alpha;
	VECTORCH beta;
	int a,uselaser;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	xenoStatusPointer->HeadLaserOnTarget=0;
	xenoStatusPointer->LALaserOnTarget=0;
	xenoStatusPointer->RALaserOnTarget=0;

	#if (FAR_XENO_ACTIVITY==0)
	if (xenoStatusPointer->IAmFar) {
		xenoStatusPointer->TargetingLaser[0].BeamIsOn=0;
		xenoStatusPointer->TargetingLaser[1].BeamIsOn=0;
		xenoStatusPointer->TargetingLaser[2].BeamIsOn=0;
		return;
	}
	#endif

	for (a=0; a<3; a++) {

		xenoStatusPointer->TargetingLaser[a].BeamIsOn=0;
		switch (a) {
			case 0:
				dum=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"flash dummyZ");
				if (xenoStatusPointer->UseHeadLaser) {
					uselaser=1;
				} else {
					uselaser=0;
				}
				if (dum==NULL) {
					uselaser=0;
				}
				break;
			case 1:
				dum=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"flash dummy C");
				if (xenoStatusPointer->UseLALaser) {
					uselaser=1;
				} else {
					uselaser=0;
				}
				if (dum==NULL) {
					uselaser=0;
				}
				break;
			case 2:
				dum=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"flash dummy D");
				if (xenoStatusPointer->UseRALaser) {
					uselaser=1;
				} else {
					uselaser=0;
				}
				if (dum==NULL) {
					uselaser=0;
				}
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
	
		if (uselaser) {
		
			alpha = dum->World_Offset;
			
			beta.vx=dum->SecMat.mat31;
			beta.vy=dum->SecMat.mat32;
			beta.vz=dum->SecMat.mat33;
			
			FindPolygonInLineOfSight(&beta,&alpha,0,sbPtr->SBdptr);

			/* Now deal with LOS_ObjectHitPtr. */
			if (LOS_ObjectHitPtr==Player) {
				xenoStatusPointer->TargetingLaser[a].BeamHasHitPlayer=1;
			} else {
				xenoStatusPointer->TargetingLaser[a].BeamHasHitPlayer=0;
			}
			
			xenoStatusPointer->TargetingLaser[a].SourcePosition=alpha;
			if (LOS_ObjectHitPtr) {
				xenoStatusPointer->TargetingLaser[a].TargetPosition=LOS_Point;
			} else {
				/* Must be hitting nothing... */
				xenoStatusPointer->TargetingLaser[a].TargetPosition=alpha;
				xenoStatusPointer->TargetingLaser[a].TargetPosition.vx+=(beta.vx>>3);
				xenoStatusPointer->TargetingLaser[a].TargetPosition.vy+=(beta.vy>>3);
				xenoStatusPointer->TargetingLaser[a].TargetPosition.vz+=(beta.vz>>3);

			}
			xenoStatusPointer->TargetingLaser[a].BeamIsOn=1;
			
			if (LOS_ObjectHitPtr) {	
				if (LOS_ObjectHitPtr->ObStrategyBlock==xenoStatusPointer->Target) {
					switch (a) {
						case 0:
							xenoStatusPointer->HeadLaserOnTarget=1;
							break;
						case 1:
							xenoStatusPointer->LALaserOnTarget=1;
							break;
						case 2:
							xenoStatusPointer->RALaserOnTarget=1;
							break;
						default:
							GLOBALASSERT(0);
							break;
					}
				}
			}
		}
	}
	
}

void Xeno_SwitchLED(STRATEGYBLOCK *sbPtr,int state) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *led;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	led=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"led");

	if (led) {
		if (led->tac_ptr) {
			led->tac_ptr->tac_sequence = state ;
			led->tac_ptr->tac_txah_s = GetTxAnimHeaderFromShape(led->tac_ptr, led->ShapeNum);
		}
	}
}

void Xeno_Stomp(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *foot;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	foot=NULL;

	if (xenoStatusPointer->HModelController.keyframe_flags&4) {
		foot=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"right foot");
	} else if (xenoStatusPointer->HModelController.keyframe_flags&8) {
		foot=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"left foot");
	}

	if (foot) {
		Sound_Play(SID_STOMP,"d",&foot->World_Offset);
	}

}

void Xeno_MaintainSounds(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	SECTION_DATA *sec;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	/* First, the two big system sounds. */
	if (xenoStatusPointer->soundHandle1!=SOUND_NOACTIVEINDEX) {
		Sound_Update3d(xenoStatusPointer->soundHandle1,&sbPtr->DynPtr->Position);
	}
	if (xenoStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		Sound_Update3d(xenoStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
	
	/* Now, all the lesser sounds: */
	/* Head. */
	sec=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"head");
	if (sec==NULL) {
		/* No sound. */
		if (xenoStatusPointer->head_whirr!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(xenoStatusPointer->head_whirr);
		}
	} else {
		if (xenoStatusPointer->head_moving==0) {
			/* Stationary. */
			if (xenoStatusPointer->head_whirr!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(xenoStatusPointer->head_whirr);
				Sound_Play(SID_ARMEND,"d",&sec->World_Offset);
			}
		} else {
			/* Moving! */
			if (xenoStatusPointer->head_whirr==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ARMSTART,"d",&sec->World_Offset);
				Sound_Play(SID_ARMMID,"del",&sec->World_Offset,&xenoStatusPointer->head_whirr);
			} else {
				Sound_Update3d(xenoStatusPointer->head_whirr,&sec->World_Offset);
			}
		}
	}
	/* Left Arm. */
	sec=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"left bicep");
	if (sec==NULL) {
		/* No sound. */
		if (xenoStatusPointer->left_arm_whirr!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(xenoStatusPointer->left_arm_whirr);
		}
	} else {
		if (xenoStatusPointer->la_moving==0) {
			/* Stationary. */
			if (xenoStatusPointer->left_arm_whirr!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(xenoStatusPointer->left_arm_whirr);
				Sound_Play(SID_ARMEND,"d",&sec->World_Offset);
			}
		} else {
			/* Moving! */
			if (xenoStatusPointer->left_arm_whirr==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ARMSTART,"d",&sec->World_Offset);
				Sound_Play(SID_ARMMID,"del",&sec->World_Offset,&xenoStatusPointer->left_arm_whirr);
			} else {
				Sound_Update3d(xenoStatusPointer->left_arm_whirr,&sec->World_Offset);
			}
		}
	}
	/* Right Arm. */
	sec=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"right bicep");
	if (sec==NULL) {
		/* No sound. */
		if (xenoStatusPointer->right_arm_whirr!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(xenoStatusPointer->right_arm_whirr);
		}
	} else {
		if (xenoStatusPointer->ra_moving==0) {
			/* Stationary. */
			if (xenoStatusPointer->right_arm_whirr!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(xenoStatusPointer->right_arm_whirr);
				Sound_Play(SID_ARMEND,"d",&sec->World_Offset);
			}
		} else {
			/* Moving! */
			if (xenoStatusPointer->right_arm_whirr==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ARMSTART,"d",&sec->World_Offset);
				Sound_Play(SID_ARMMID,"del",&sec->World_Offset,&xenoStatusPointer->right_arm_whirr);
			} else {
				Sound_Update3d(xenoStatusPointer->right_arm_whirr,&sec->World_Offset);
			}
		}
	}
	/* Torso Twist. */
	sec=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"chest");
	if (sec==NULL) {
		/* No sound. */
		if (xenoStatusPointer->torso_whirr!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(xenoStatusPointer->torso_whirr);
		}
	} else {
		if (xenoStatusPointer->torso_moving==0) {
			/* Stationary. */
			if (xenoStatusPointer->torso_whirr!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(xenoStatusPointer->torso_whirr);
				Sound_Play(SID_ARMEND,"d",&sec->World_Offset);
			}
		} else {
			/* Moving! */
			if (xenoStatusPointer->torso_whirr==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ARMSTART,"d",&sec->World_Offset);
				Sound_Play(SID_ARMMID,"del",&sec->World_Offset,&xenoStatusPointer->torso_whirr);
			} else {
				Sound_Update3d(xenoStatusPointer->torso_whirr,&sec->World_Offset);
			}
		}
	}
}

void XenoborgHandleMovingAnimation(STRATEGYBLOCK *sbPtr) {

	XENO_STATUS_BLOCK *xenoStatusPointer;
	VECTORCH offset;
	int speed,animfactor;

	LOCALASSERT(sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(xenoStatusPointer);

	offset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
	offset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
	offset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;
	
	/* ...compute speed factor... */
	speed=Magnitude(&offset);
	if (speed<(MUL_FIXED(NormalFrameTime,50))) {
		/* Not moving much, are we?  Be stationary! */
		if (ShowXenoStats) {
			PrintDebuggingText("Forced stationary animation!\n");
		}
		EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED,(ONE_FIXED>>2));
		return;
	}
	speed=DIV_FIXED(speed,NormalFrameTime);
	
	if (speed==0) {
		animfactor=ONE_FIXED;
	} else {
		animfactor=DIV_FIXED(625,speed); // Was 512!  Difference to correct for rounding down...
	}
	GLOBALASSERT(animfactor>0);
	if (ShowXenoStats) {
		PrintDebuggingText("Anim Factor %d, Tweening %d\n",animfactor,xenoStatusPointer->HModelController.Tweening);
	}

	/* Start animation. */
	EnforceXenoborgShapeAnimSequence_Core(sbPtr,HMSQT_Xenoborg,XBSS_Walking,XENO_WALKING_ANIM_SPEED,(ONE_FIXED>>2));

	if (xenoStatusPointer->HModelController.Tweening==0) {
		HModel_SetToolsRelativeSpeed(&xenoStatusPointer->HModelController,animfactor);
	}

}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct xenoborg_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	signed int health;
	XENO_BHSTATE behaviourState;
	XENO_BHSTATE lastState;
	int stateTimer;  		
	NPC_WANDERDATA wanderData;
	NPC_OBSTRUCTIONREPORT obstruction;
	VECTORCH my_spot_therin;
	VECTORCH my_orientdir_therin;
	int module_range;
	int UpTime;
	int GibbFactor;
	int Wounds;



	VECTORCH targetTrackPos;

	int Head_Pan;
	int Head_Tilt;
	int Left_Arm_Pan;
	int Left_Arm_Tilt;
	int Right_Arm_Pan;
	int Right_Arm_Tilt;
	int Torso_Twist;

	int Old_Head_Pan;
	int Old_Head_Tilt;
	int Old_Left_Arm_Pan;
	int Old_Left_Arm_Tilt;
	int Old_Right_Arm_Pan;
	int Old_Right_Arm_Tilt;
	int Old_Torso_Twist;

 	LASER_BEAM_DESC LeftMainBeam;
 	LASER_BEAM_DESC RightMainBeam;
 	LASER_BEAM_DESC TargetingLaser[3];

	unsigned int headpandir			:1;
	unsigned int headtiltdir		:1;
	unsigned int leftarmpandir		:1;
	unsigned int leftarmtiltdir		:1;
	unsigned int rightarmpandir		:1;
	unsigned int rightarmtiltdir	:1;
	unsigned int torsotwistdir		:1;
	
	unsigned int headLock			:1;
	unsigned int leftArmLock		:1;
	unsigned int rightArmLock		:1;
	unsigned int targetSightTest	:1;
	unsigned int IAmFar				:1;
	unsigned int ShotThisFrame		:1;
	
	unsigned int FiringLeft			:1;
	unsigned int FiringRight		:1;

	unsigned int UseHeadLaser		:1;
	unsigned int UseLALaser			:1;
	unsigned int UseRALaser			:1;

	unsigned int HeadLaserOnTarget	:1;
	unsigned int LALaserOnTarget	:1;
	unsigned int RALaserOnTarget	:1;

	unsigned int head_moving		:1;
	unsigned int la_moving			:1;
	unsigned int ra_moving			:1;
	unsigned int torso_moving		:1;
   
	int incidentFlag;
	int incidentTimer;
		
	int head_whirr;
	int left_arm_whirr;
	int right_arm_whirr;
	int torso_whirr;
		

//annoying pointer related things
	int my_module_index;

	char Target_SBname[SB_NAME_LENGTH];

//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}XENOBORG_SAVE_BLOCK;




//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV xenoStatusPointer


void LoadStrategy_Xenoborg(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	int i;
	STRATEGYBLOCK* sbPtr;
	XENO_STATUS_BLOCK* xenoStatusPointer;
	XENOBORG_SAVE_BLOCK* block = (XENOBORG_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourXenoborg) return;

	xenoStatusPointer =(XENO_STATUS_BLOCK*) sbPtr->SBdataptr;

	
	//start copying stuff
	COPYELEMENT_LOAD(health)
	COPYELEMENT_LOAD(behaviourState)
	COPYELEMENT_LOAD(lastState)
	COPYELEMENT_LOAD(stateTimer)  		
	COPYELEMENT_LOAD(wanderData)
	COPYELEMENT_LOAD(obstruction)
	COPYELEMENT_LOAD(my_spot_therin)
	COPYELEMENT_LOAD(my_orientdir_therin)
	COPYELEMENT_LOAD(module_range)
	COPYELEMENT_LOAD(UpTime)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(Wounds)
	COPYELEMENT_LOAD(targetTrackPos)
	COPYELEMENT_LOAD(Head_Pan)
	COPYELEMENT_LOAD(Head_Tilt)
	COPYELEMENT_LOAD(Left_Arm_Pan)
	COPYELEMENT_LOAD(Left_Arm_Tilt)
	COPYELEMENT_LOAD(Right_Arm_Pan)
	COPYELEMENT_LOAD(Right_Arm_Tilt)
	COPYELEMENT_LOAD(Torso_Twist)
	COPYELEMENT_LOAD(Old_Head_Pan)
	COPYELEMENT_LOAD(Old_Head_Tilt)
	COPYELEMENT_LOAD(Old_Left_Arm_Pan)
	COPYELEMENT_LOAD(Old_Left_Arm_Tilt)
	COPYELEMENT_LOAD(Old_Right_Arm_Pan)
	COPYELEMENT_LOAD(Old_Right_Arm_Tilt)
	COPYELEMENT_LOAD(Old_Torso_Twist)
 	COPYELEMENT_LOAD(LeftMainBeam)
 	COPYELEMENT_LOAD(RightMainBeam)
 	

	COPYELEMENT_LOAD(headpandir)
	COPYELEMENT_LOAD(headtiltdir)
	COPYELEMENT_LOAD(leftarmpandir)
	COPYELEMENT_LOAD(leftarmtiltdir)
	COPYELEMENT_LOAD(rightarmpandir)
	COPYELEMENT_LOAD(rightarmtiltdir)
	COPYELEMENT_LOAD(torsotwistdir)
	
	COPYELEMENT_LOAD(headLock)
	COPYELEMENT_LOAD(leftArmLock)
	COPYELEMENT_LOAD(rightArmLock)
	COPYELEMENT_LOAD(targetSightTest)
	COPYELEMENT_LOAD(IAmFar)
	COPYELEMENT_LOAD(ShotThisFrame)
	
	COPYELEMENT_LOAD(FiringLeft)
	COPYELEMENT_LOAD(FiringRight)

	COPYELEMENT_LOAD(UseHeadLaser)
	COPYELEMENT_LOAD(UseLALaser)
	COPYELEMENT_LOAD(UseRALaser)

	COPYELEMENT_LOAD(HeadLaserOnTarget)
	COPYELEMENT_LOAD(LALaserOnTarget)
	COPYELEMENT_LOAD(RALaserOnTarget)

	COPYELEMENT_LOAD(head_moving)
	COPYELEMENT_LOAD(la_moving)
	COPYELEMENT_LOAD(ra_moving)
	COPYELEMENT_LOAD(torso_moving)
   
	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)
	
	COPYELEMENT_LOAD(head_whirr)
	COPYELEMENT_LOAD(left_arm_whirr)
	COPYELEMENT_LOAD(right_arm_whirr)
	COPYELEMENT_LOAD(torso_whirr)

 	for(i=0;i<3;i++)
	{
 		COPYELEMENT_LOAD(TargetingLaser[i])
	}
	
	//load ai module pointers
	xenoStatusPointer->my_module = GetPointerFromAIModuleIndex(block->my_module_index);

	//load target
	COPY_NAME(xenoStatusPointer->Target_SBname,block->Target_SBname);
	xenoStatusPointer->Target = FindSBWithName(xenoStatusPointer->Target_SBname);

	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&xenoStatusPointer->HModelController);
		}
	}
	

	//finally get the delta controller pointers
	VerifyDeltaControllers(sbPtr);
	
	Load_SoundState(&xenoStatusPointer->soundHandle1);
	Load_SoundState(&xenoStatusPointer->soundHandle2);
}


void SaveStrategy_Xenoborg(STRATEGYBLOCK* sbPtr)
{
	int i;
	XENO_STATUS_BLOCK* xenoStatusPointer;
	XENOBORG_SAVE_BLOCK* block;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	xenoStatusPointer = (XENO_STATUS_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_SAVE(health)
	COPYELEMENT_SAVE(behaviourState)
	COPYELEMENT_SAVE(lastState)
	COPYELEMENT_SAVE(stateTimer)  		
	COPYELEMENT_SAVE(wanderData)
	COPYELEMENT_SAVE(obstruction)
	COPYELEMENT_SAVE(my_spot_therin)
	COPYELEMENT_SAVE(my_orientdir_therin)
	COPYELEMENT_SAVE(module_range)
	COPYELEMENT_SAVE(UpTime)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(Wounds)
	COPYELEMENT_SAVE(targetTrackPos)
	COPYELEMENT_SAVE(Head_Pan)
	COPYELEMENT_SAVE(Head_Tilt)
	COPYELEMENT_SAVE(Left_Arm_Pan)
	COPYELEMENT_SAVE(Left_Arm_Tilt)
	COPYELEMENT_SAVE(Right_Arm_Pan)
	COPYELEMENT_SAVE(Right_Arm_Tilt)
	COPYELEMENT_SAVE(Torso_Twist)
	COPYELEMENT_SAVE(Old_Head_Pan)
	COPYELEMENT_SAVE(Old_Head_Tilt)
	COPYELEMENT_SAVE(Old_Left_Arm_Pan)
	COPYELEMENT_SAVE(Old_Left_Arm_Tilt)
	COPYELEMENT_SAVE(Old_Right_Arm_Pan)
	COPYELEMENT_SAVE(Old_Right_Arm_Tilt)
	COPYELEMENT_SAVE(Old_Torso_Twist)
 	COPYELEMENT_SAVE(LeftMainBeam)
 	COPYELEMENT_SAVE(RightMainBeam)
 	

	COPYELEMENT_SAVE(headpandir)
	COPYELEMENT_SAVE(headtiltdir)
	COPYELEMENT_SAVE(leftarmpandir)
	COPYELEMENT_SAVE(leftarmtiltdir)
	COPYELEMENT_SAVE(rightarmpandir)
	COPYELEMENT_SAVE(rightarmtiltdir)
	COPYELEMENT_SAVE(torsotwistdir)
	
	COPYELEMENT_SAVE(headLock)
	COPYELEMENT_SAVE(leftArmLock)
	COPYELEMENT_SAVE(rightArmLock)
	COPYELEMENT_SAVE(targetSightTest)
	COPYELEMENT_SAVE(IAmFar)
	COPYELEMENT_SAVE(ShotThisFrame)
	
	COPYELEMENT_SAVE(FiringLeft)
	COPYELEMENT_SAVE(FiringRight)

	COPYELEMENT_SAVE(UseHeadLaser)
	COPYELEMENT_SAVE(UseLALaser)
	COPYELEMENT_SAVE(UseRALaser)

	COPYELEMENT_SAVE(HeadLaserOnTarget)
	COPYELEMENT_SAVE(LALaserOnTarget)
	COPYELEMENT_SAVE(RALaserOnTarget)

	COPYELEMENT_SAVE(head_moving)
	COPYELEMENT_SAVE(la_moving)
	COPYELEMENT_SAVE(ra_moving)
	COPYELEMENT_SAVE(torso_moving)
   
	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)
	
	COPYELEMENT_SAVE(head_whirr)
	COPYELEMENT_SAVE(left_arm_whirr)
	COPYELEMENT_SAVE(right_arm_whirr)
	COPYELEMENT_SAVE(torso_whirr)

 	for(i=0;i<3;i++)
	{
 		COPYELEMENT_SAVE(TargetingLaser[i])
	}

	//load ai module pointers
	block->my_module_index = GetIndexFromAIModulePointer(xenoStatusPointer->my_module);

	//save target
	COPY_NAME(block->Target_SBname,xenoStatusPointer->Target_SBname);

	//save strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&xenoStatusPointer->HModelController);

	Save_SoundState(&xenoStatusPointer->soundHandle1);
	Save_SoundState(&xenoStatusPointer->soundHandle2);
}
