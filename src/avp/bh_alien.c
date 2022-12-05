/*------------------------Patrick 6/11/96-----------------------------
  Source file for alien AI behaviour functions....
  --------------------------------------------------------------------*/
#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"
#include "dynamics.h"
#include "comp_shp.h"
#include "load_shp.h"
#include "bh_types.h"
#include "bh_debri.h"
#include "bh_far.h"
#include "bh_near.h"
#include "bh_gener.h"
#include "bh_pred.h"
#include "bh_alien.h"
#include "bh_marin.h"
#include "weapons.h"
#include "pheromon.h"
#include "pfarlocs.h"
#include "pvisible.h"
#include "psnd.h"
#include "psndplat.h"
#include "extents.h"
#include "huddefs.h"
#include "pldghost.h"
#include "bh_corpse.h"
#include "bh_dummy.h"
#include "game_statistics.h"
#include "scream.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "pldnet.h"
#include "avp_userprofile.h"

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern unsigned char Null_Name[8];
extern ACTIVESOUNDSAMPLE ActiveSounds[];

extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern void StartAlienAttackSequence(STRATEGYBLOCK *sbPtr);

extern int NearAliens;
extern int Alt_NearAliens;
extern int FarAliens;
extern int Alt_FarAliens;
extern int ShowHiveState;

/* prototypes for this file */

void KillAlien(STRATEGYBLOCK *sbPtr, int wounds, DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming);
void Execute_Alien_Dying(STRATEGYBLOCK *sbPtr);
STRATEGYBLOCK *Alien_GetNewTarget(VECTORCH *alienpos, STRATEGYBLOCK *me);
void CreateAlienBot(VECTORCH *Position,int type);

/*----------------------Patrick 15/11/96-----------------------------
  Alien's map: used to generate display blocks for aliens when they
  come into view

--------------------------------------------------------------------*/
/* KJL 17:16:34 11/25/96 - I've wrapped the necessary fields in the
mapblock with #if StandardStrategyAndCollisions */

MODULEMAPBLOCK AlienDefaultMap =
{
	MapType_Sprite,
	I_ShapeCube, /* default value */
    {0,0,0},
    {0,0,0},
	ObFlag_NoInfLSrc|ObFlag_MultLSrc,
	0,
	0,
	0,							
	0,							
	0,	
    {0,0,0},
	0,						 
	0,
    0,
    0,						 
    {0,0,0}
};

/* CDF 12/2/98 */

void CastAlienBot(void) {

	#define BOTRANGE 2000

	VECTORCH position;
	#if 0
	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO ALIENBOTS IN MULTIPLAYER MODE");
		return;
	}
	#endif
	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateAlienBot(&position,0);

}

void CastPredAlienBot(void) {

	#define BOTRANGE 2000

	VECTORCH position;
	#if 0
	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO ALIENBOTS IN MULTIPLAYER MODE");
		return;
	}
	#endif
	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateAlienBot(&position,1);

}

void CastPraetorianBot(void) {

	#define BOTRANGE 2000

	VECTORCH position;
	#if 0
	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO ALIENBOTS IN MULTIPLAYER MODE");
		return;
	}
	#endif
	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateAlienBot(&position,2);

}

void CreateAlienBot(VECTORCH *Position,int type)
{
	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) {
		NewOnScreenMessage("FAILED TO CREATE BOT: SB CREATION FAILURE");
		return; /* failure */
	}
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourAlien;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
      	dynPtr->PrevPosition = dynPtr->Position = *Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
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
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(ALIEN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
		int i;

		NPC_InitMovementData(&(alienStatus->moveData));
		NPC_InitWanderData(&(alienStatus->wanderData));     	
		switch (type) {
			case 0:
			default:
				alienStatus->Type = AT_Standard;
				break;
			case 1:
				alienStatus->Type = AT_Predalien;
				break;
			case 2:
				alienStatus->Type = AT_Praetorian;
				break;
		}

		/* Initialise alien's stats */
		{
			NPC_DATA *NpcData;
	
			switch (alienStatus->Type) {
				case AT_Standard:
					NpcData=GetThisNpcData(I_NPC_Alien);
					alienStatus->MaxSpeed=ALIEN_FORWARDVELOCITY;
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = ALIEN_MASS;
					break;
				case AT_Predalien:
					NpcData=GetThisNpcData(I_NPC_PredatorAlien);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,PREDALIEN_SPEED_FACTOR);
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PREDALIEN_MASS;
					break;
				case AT_Praetorian:
					NpcData=GetThisNpcData(I_NPC_PraetorianGuard);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,MUL_FIXED(PRAETORIAN_WALKSPEED_FACTOR,PRAETORIAN_SPEED_FACTOR));
					alienStatus->EnableWaypoints=0;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PRAETORIAN_MASS;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}


		alienStatus->Health = ALIEN_STARTING_HEALTH;
   		alienStatus->GibbFactor=0;
		alienStatus->Wounds=0;
		alienStatus->last_anim_length=ONE_FIXED;
   		sbPtr->integrity = alienStatus->Health;
   		alienStatus->BehaviourState = ABS_Hunt;   		
		alienStatus->current_attack=NULL;
	
		alienStatus->Target=NULL;
		COPY_NAME(alienStatus->Target_SBname,Null_Name);
		alienStatus->Mission=AM_Hunt; /* Was GlobalHunt... */

		alienStatus->my_containing_module=NULL;
		alienStatus->huntingModule=NULL;

		Initialise_AvoidanceManager(sbPtr,&alienStatus->avoidanceManager);
		InitWaypointManager(&alienStatus->waypointManager);

		alienStatus->CurveRadius = 0;
		alienStatus->CurveLength = 0;
		alienStatus->CurveTimeOut = 0;
		alienStatus->FarStateTimer = ALIEN_FAR_MOVE_TIME;
		alienStatus->NearStateTimer = 0;
		alienStatus->IAmCrouched = 0;

		alienStatus->soundHandle = SOUND_NOACTIVEINDEX;
		alienStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		alienStatus->incidentFlag=0;
		alienStatus->incidentTimer=0;
		
		//alien created by generator won't have a death target
		for(i=0;i<SB_NAME_LENGTH;i++) alienStatus->death_target_ID[i] =0; 
		alienStatus->death_target_sbptr=0;

		//this alien wasn't produced by a generator
		alienStatus->generator_sbptr=0;

		
		alienStatus->HModelController.section_data=NULL;
		alienStatus->HModelController.Deltas=NULL;

		switch (alienStatus->Type) {
			case AT_Standard:
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				break;
			case AT_Predalien:
				root_section=GetNamedHierarchyFromLibrary("hnpcpred_alien","TEMPLATE");
				break;
			case AT_Praetorian:
				root_section=GetNamedHierarchyFromLibrary("hnpcpretorian","Template");
				break;
			default:
				GLOBALASSERT(0);
				break;
		}
				
		if (!root_section) {
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE BOT: NO HMODEL");
			return;
		}
		Create_HModel(&alienStatus->HModelController,root_section);
		InitHModelSequence(&alienStatus->HModelController,0,0,ONE_FIXED);

		if (SLUGTRAIL_MODE) {
			SECTION_DATA *leg;
			/* Blow off a leg? */
			leg=GetThisSectionData(alienStatus->HModelController.section_data,"left thigh");
			if (leg) {
				Prune_HModel_Virtual(leg);
				alienStatus->Wounds|=section_flag_left_leg;
				alienStatus->Wounds|=section_flag_left_foot;

				sbPtr->SBDamageBlock.Health-=(20<<ONE_FIXED_SHIFT);
				RecomputeAlienSpeed(sbPtr);
			}
		}


		if (HModelSequence_Exists(&alienStatus->HModelController,(int)HMSQT_AlienStand,(int)ASSS_Hit_Right)) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&alienStatus->HModelController,"HitDelta",(int)HMSQT_AlienStand,(int)ASSS_Hit_Right,-1);
			GLOBALASSERT(delta);
			delta->Playing=0;
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
	
		if (HModelSequence_Exists(&alienStatus->HModelController,HMSQT_AlienRun,ARSS_Jump)) {
			alienStatus->JumpDetected=1;
		} else {
			alienStatus->JumpDetected=0;
		}

		if (GetAlienPounceAttack(&alienStatus->HModelController,0,1)) {
			/* Pounce will be unset eventually. */
			alienStatus->PounceDetected=1;
			alienStatus->EnablePounce=1;
		} else {
			alienStatus->PounceDetected=0;
			alienStatus->EnablePounce=0;
		}
		
		alienStatus->aliensIgniterId=0;

		MakeAlienNear(sbPtr);

		NewOnScreenMessage("ALIENBOT CREATED");

	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE BOT: MALLOC FAILURE");
		return;
	}

}


/*----------------------Patrick 15/11/96-----------------------------
This function is called by the alien generators, to dynamically
create a new alien, during the game.  The alien is initialised
as invisible.

1. create a new strategy block (no diplay block)
2. attach a dynamics block
3. attach an alien data block
4. initialise for far behaviour

NB the strategyblock passed here is a reference to the generator sb
--------------------------------------------------------------------*/
void CreateAlienDynamic(STRATEGYBLOCK *Generator, ALIEN_TYPE type_of_alien)
{
	STRATEGYBLOCK* sbPtr;
	//int i;


	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) return; /* failure */
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourAlien;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
      	dynPtr->PrevPosition = dynPtr->Position = ((GENERATOR_BLOCK* )Generator->SBdataptr)->Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	sbPtr->shapeIndex = Generator->shapeIndex;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);
	LOCALASSERT(sbPtr->containingModule);
	if(!(sbPtr->containingModule))
	{
		/* no containing module can be found... abort*/
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
  	
  	/* assert alien is starting as invisible */
  	LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] == 0);

	/* create, initialise and attach an alien data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(ALIEN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
		int i;

		NPC_InitMovementData(&(alienStatus->moveData));
		NPC_InitWanderData(&(alienStatus->wanderData));     	

		alienStatus->Type = type_of_alien;

		/* Initialise alien's stats */
		{
			NPC_DATA *NpcData;
	
			switch (alienStatus->Type) {
				case AT_Standard:
					NpcData=GetThisNpcData(I_NPC_Alien);
					alienStatus->MaxSpeed=ALIEN_FORWARDVELOCITY;
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = ALIEN_MASS;
					break;
				case AT_Predalien:
					NpcData=GetThisNpcData(I_NPC_PredatorAlien);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,PREDALIEN_SPEED_FACTOR);
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PREDALIEN_MASS;
					break;
				case AT_Praetorian:
					NpcData=GetThisNpcData(I_NPC_PraetorianGuard);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,MUL_FIXED(PRAETORIAN_WALKSPEED_FACTOR,PRAETORIAN_SPEED_FACTOR));
					alienStatus->EnableWaypoints=0;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PRAETORIAN_MASS;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}


		alienStatus->Health = ALIEN_STARTING_HEALTH;
   		alienStatus->GibbFactor=0;
		alienStatus->Wounds=0;
		alienStatus->last_anim_length=ONE_FIXED;
   		sbPtr->integrity = alienStatus->Health;
   		alienStatus->BehaviourState = ABS_Hunt;   		
		alienStatus->current_attack=NULL;
		alienStatus->Wounds=0;

		alienStatus->Target=NULL;
		COPY_NAME(alienStatus->Target_SBname,Null_Name);
		alienStatus->Mission=AM_Hunt;

		alienStatus->my_containing_module=NULL;
		alienStatus->huntingModule=NULL;

		alienStatus->CurveRadius = 0;
		alienStatus->CurveLength = 0;
		alienStatus->CurveTimeOut = 0;
		alienStatus->FarStateTimer = ALIEN_FAR_MOVE_TIME;
		alienStatus->NearStateTimer = 0;
		alienStatus->IAmCrouched = 0;
		
		Initialise_AvoidanceManager(sbPtr,&alienStatus->avoidanceManager);
		InitWaypointManager(&alienStatus->waypointManager);

		alienStatus->soundHandle = SOUND_NOACTIVEINDEX;
		alienStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		alienStatus->incidentFlag=0;
		alienStatus->incidentTimer=0;

		//alien created by generator won't have a death target
		for(i=0;i<SB_NAME_LENGTH;i++) alienStatus->death_target_ID[i] =0; 
		alienStatus->death_target_sbptr=0;

		//note the generator that produced this alien
		alienStatus->generator_sbptr=Generator;
		
		switch (alienStatus->Type) {
			case AT_Standard:
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				break;
			case AT_Predalien:
				root_section=GetNamedHierarchyFromLibrary("hnpcpred_alien","TEMPLATE");
				break;
			case AT_Praetorian:
				root_section=GetNamedHierarchyFromLibrary("hnpcpretorian","Template");
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		Create_HModel(&alienStatus->HModelController,root_section);
		InitHModelSequence(&alienStatus->HModelController,0,0,ONE_FIXED);

		if (SLUGTRAIL_MODE) {
			SECTION_DATA *leg;
			/* Blow off a leg? */
			leg=GetThisSectionData(alienStatus->HModelController.section_data,"left thigh");
			if (leg) {
				Prune_HModel_Virtual(leg);
				alienStatus->Wounds|=section_flag_left_leg;
				alienStatus->Wounds|=section_flag_left_foot;

				sbPtr->SBDamageBlock.Health-=(20<<ONE_FIXED_SHIFT);
				RecomputeAlienSpeed(sbPtr);
			}
		}

		if (HModelSequence_Exists(&alienStatus->HModelController,(int)HMSQT_AlienStand,(int)ASSS_Hit_Right)) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&alienStatus->HModelController,"HitDelta",(int)HMSQT_AlienStand,(int)ASSS_Hit_Right,-1);
			GLOBALASSERT(delta);
			delta->Playing=0;
		}

		ProveHModel_Far(&alienStatus->HModelController,sbPtr);

		if (HModelSequence_Exists(&alienStatus->HModelController,HMSQT_AlienRun,ARSS_Jump)) {
			alienStatus->JumpDetected=1;
		} else {
			alienStatus->JumpDetected=0;
		}

		if (GetAlienPounceAttack(&alienStatus->HModelController,0,1)) {
			/* Pounce will be unset eventually. */
			alienStatus->PounceDetected=1;
			alienStatus->EnablePounce=1;
		} else {
			alienStatus->PounceDetected=0;
			alienStatus->EnablePounce=0;
		}
		alienStatus->aliensIgniterId=0;

	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
}

/*----------------------Patrick 7/11/96-----------------------------
This function is used to initialise a riff-loaded in alien.
-------------------------------------------------------------------*/
void InitAlienBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_ALIEN *toolsData = (TOOLS_DATA_ALIEN *)bhdata;
	int i;

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network && !AvP.NetworkAIServer) 
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* strategy block should be initialised to general default values:
	I_behaviourAlien, shapeIndex, and everything else 0.... 
	
	NB it is necessary that placed aliens are initialised without a
	displayblock, otherwise far aliens will be re-initialised by
	MakeAlienFar() to a hunting state instead of a wait state....*/			
	LOCALASSERT(!(sbPtr->SBdptr));

	/* strategy block initialisation */
	sbPtr->shapeIndex =	toolsData->shapeIndex;

	if(AvP.Network==I_No_Network)
	{
		for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];
	}
	else
	{
		//in a network game , generate a new sb name (so that it gets a reasonably low value)
		AssignNewSBName(sbPtr);
	}

	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_NPC);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->OrientEuler = toolsData->starteuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}
	else
	{
		/* no dynamics block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* create, initialise and attach an alien data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(ALIEN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		ALIEN_STATUS_BLOCK * alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;

		NPC_InitMovementData(&(alienStatus->moveData));
		NPC_InitWanderData(&(alienStatus->wanderData));

		alienStatus->Type = toolsData->type;

		/* Initialise alien's stats */
		{
			NPC_DATA *NpcData;
	
			switch (alienStatus->Type) {
				case AT_Standard:
					NpcData=GetThisNpcData(I_NPC_Alien);
					alienStatus->MaxSpeed=ALIEN_FORWARDVELOCITY;
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = ALIEN_MASS;
					break;
				case AT_Predalien:
					NpcData=GetThisNpcData(I_NPC_PredatorAlien);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,PREDALIEN_SPEED_FACTOR);
					alienStatus->EnableWaypoints=1;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PREDALIEN_MASS;
					break;
				case AT_Praetorian:
					NpcData=GetThisNpcData(I_NPC_PraetorianGuard);
					alienStatus->MaxSpeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,MUL_FIXED(PRAETORIAN_WALKSPEED_FACTOR,PRAETORIAN_SPEED_FACTOR));
					alienStatus->EnableWaypoints=0;
					alienStatus->PreferToCrouch=0;
					sbPtr->DynPtr->Mass = PRAETORIAN_MASS;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}

		alienStatus->Health = ALIEN_STARTING_HEALTH;
   		sbPtr->integrity = alienStatus->Health;
		alienStatus->GibbFactor=0;
		alienStatus->Wounds=0;
		alienStatus->last_anim_length=ONE_FIXED;
		
		if (toolsData->start_inactive) {
	   		alienStatus->BehaviourState = ABS_Dormant;
		} else {
	   		alienStatus->BehaviourState = ABS_Wait;
		}
		
		alienStatus->current_attack=NULL;
		alienStatus->Wounds=0;

		alienStatus->Target=NULL;
		COPY_NAME(alienStatus->Target_SBname,Null_Name);
		alienStatus->Mission=AM_Hunt;

		alienStatus->my_containing_module=NULL;
		alienStatus->huntingModule=NULL;

		alienStatus->CurveRadius = 0;
		alienStatus->CurveLength = 0;
		alienStatus->CurveTimeOut = 0;
		alienStatus->FarStateTimer = ALIEN_FAR_MOVE_TIME;
		alienStatus->NearStateTimer = 0;
		alienStatus->IAmCrouched = 0;

		Initialise_AvoidanceManager(sbPtr,&alienStatus->avoidanceManager);
		InitWaypointManager(&alienStatus->waypointManager);

		alienStatus->soundHandle = SOUND_NOACTIVEINDEX;
		alienStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		alienStatus->incidentFlag=0;
		alienStatus->incidentTimer=0;

		for(i=0;i<SB_NAME_LENGTH;i++) alienStatus->death_target_ID[i] = toolsData->death_target_ID[i];
		alienStatus->death_target_sbptr=0;

		//this alien wasn't produced by a generator
		alienStatus->generator_sbptr=0;

		
		switch (alienStatus->Type) {
			case AT_Standard:
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				break;
			case AT_Predalien:
				root_section=GetNamedHierarchyFromLibrary("hnpcpred_alien","TEMPLATE");
				break;
			case AT_Praetorian:
				root_section=GetNamedHierarchyFromLibrary("hnpcpretorian","Template");
				break;
			default:
				GLOBALASSERT(0);
				break;
		}

		Create_HModel(&alienStatus->HModelController,root_section);

		if (toolsData->start_inactive) {
			if (HModelSequence_Exists(&alienStatus->HModelController,HMSQT_AlienStand,ASSS_Dormant)) {
				InitHModelSequence(&alienStatus->HModelController,HMSQT_AlienStand,ASSS_Dormant,-1);
			} else {
				InitHModelSequence(&alienStatus->HModelController,HMSQT_AlienStand,ASSS_Standard,ONE_FIXED);
			}
		} else {
			InitHModelSequence(&alienStatus->HModelController,0,0,ONE_FIXED);
		}

		if (SLUGTRAIL_MODE) {
			SECTION_DATA *leg;
			/* Blow off a leg? */
			leg=GetThisSectionData(alienStatus->HModelController.section_data,"left thigh");
			if (leg) {
				Prune_HModel_Virtual(leg);
				alienStatus->Wounds|=section_flag_left_leg;
				alienStatus->Wounds|=section_flag_left_foot;

				sbPtr->SBDamageBlock.Health-=(20<<ONE_FIXED_SHIFT);
				RecomputeAlienSpeed(sbPtr);
			}
		}

		if (HModelSequence_Exists(&alienStatus->HModelController,(int)HMSQT_AlienStand,(int)ASSS_Hit_Right)) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&alienStatus->HModelController,"HitDelta",(int)HMSQT_AlienStand,(int)ASSS_Hit_Right,-1);
			GLOBALASSERT(delta);
			delta->Playing=0;
		}

		ProveHModel_Far(&alienStatus->HModelController,sbPtr);

		if (HModelSequence_Exists(&alienStatus->HModelController,HMSQT_AlienRun,ARSS_Jump)) {
			alienStatus->JumpDetected=1;
		} else {
			alienStatus->JumpDetected=0;
		}

		if (GetAlienPounceAttack(&alienStatus->HModelController,0,1)) {
			/* Pounce will be unset eventually. */
			alienStatus->PounceDetected=1;
			alienStatus->EnablePounce=1;
		} else {
			alienStatus->PounceDetected=0;
			alienStatus->EnablePounce=0;
		}
		
		alienStatus->aliensIgniterId=0;

	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}   	   	   	   
}


static BOOL PlayerIsDeadAndNoLivingNetghosts()
{
	if(AvP.Network == I_No_Network)
	{
		//just need to check for the only player we have
		return NPC_IsDead(Player->ObStrategyBlock);

	}
	else
	{
		int sbIndex;
		//first check the host player
		if(!NPC_IsDead(Player->ObStrategyBlock)) return FALSE;
		
		/* go through the strategy blocks looking for players*/
		for(sbIndex=0;sbIndex<NumActiveStBlocks;sbIndex++)
		{
			STRATEGYBLOCK *playerSbPtr = ActiveStBlockList[sbIndex];
			NETGHOSTDATABLOCK *ghostData;
			if(playerSbPtr->I_SBtype!=I_BehaviourNetGhost) continue;
			ghostData = (NETGHOSTDATABLOCK *)playerSbPtr->SBdataptr;

			if(ghostData->type==I_BehaviourMarinePlayer ||
			   ghostData->type==I_BehaviourPredatorPlayer)
			{
				//found a nethost of a player
				return FALSE;
			}
		}
	}
	//Dead, all dead.
	return TRUE;
}

/*----------------------Patrick 7/11/96-----------------------------
AI alien behaviour execution shell:

1. patch to trap aliens who's current module is not set (and set it).
2. call the visibility checking function.
3. select either near or far behaviour functions.

NB the visibility checking function initialises near/far behaviour and
allocates/deallocates displayblock based on changes in visibility
for the alien's module.  This will, in due course, be invoked by a
call back function from the module handler.
--------------------------------------------------------------------*/

void AlienBehaviour(STRATEGYBLOCK *sbPtr)
{
	/* get the alien's status block */
	ALIEN_STATUS_BLOCK *alienStatusPointer;
	int alienIsNear;

    LOCALASSERT(sbPtr);
	alienStatusPointer= (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(alienStatusPointer);	          		

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the alien could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	} 

	if(sbPtr->SBdptr) {
		alienIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		alienIsNear=0;
	}

	/* Unset incident flag. */
	alienStatusPointer->incidentFlag=0;

	alienStatusPointer->incidentTimer-=NormalFrameTime;
	
	if (alienStatusPointer->incidentTimer<0) {
		alienStatusPointer->incidentFlag=1;
		alienStatusPointer->incidentTimer=32767+(FastRandom()&65535);
	}

	if(alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(alienStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));

	if (sbPtr->SBDamageBlock.IsOnFire) {

		if (alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[alienStatusPointer->soundHandle].soundIndex!=SID_FIRE) {
				Sound_Stop(alienStatusPointer->soundHandle);
	 		 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&alienStatusPointer->soundHandle,127);
			}
		} else {
	 	 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&alienStatusPointer->soundHandle,127);
		}
		//for multiplayer games it is necessary to say who is doing this damage
		//(that would be the person who set the alien on fire in the first place)
		{
			extern DPID myNetworkKillerId;
			extern DPID AVPDPNetID;

			myNetworkKillerId=alienStatusPointer->aliensIgniterId;
			CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);
			myNetworkKillerId=AVPDPNetID;
		}
		if (sbPtr->I_SBtype==I_BehaviourNetCorpse) {
			/* Gettin' out of here... */
			return;
		}
	} else {
		if (alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[alienStatusPointer->soundHandle].soundIndex==SID_FIRE) {
				Sound_Stop(alienStatusPointer->soundHandle);
			}
		}
	}

	if (alienStatusPointer->GibbFactor) {
		/* If you're gibbed, you're dead. */
		sbPtr->SBDamageBlock.Health = 0;
	}

	/* check if we've been killed */
	if ( (sbPtr->SBDamageBlock.Health <= 0)&&(alienStatusPointer->BehaviourState!=ABS_Dying) )
	{
		textprint("Zombie Alien!!! State is %d.\n",alienStatusPointer->BehaviourState);
		return;
	}

	if ((alienStatusPointer->BehaviourState!=ABS_Dying)
		&&(alienStatusPointer->BehaviourState!=ABS_Dormant)
		&&(alienStatusPointer->BehaviourState!=ABS_Awakening)) {

		/* Target handling. */
		if (Validate_Target(alienStatusPointer->Target,alienStatusPointer->Target_SBname)==0) {
	 
			/* Target must be dead... */
			alienStatusPointer->Target=NULL;
			/* Now try to get a new target. */
		}
	
		if ((alienIsNear)||(alienStatusPointer->incidentFlag)) {
			if (alienStatusPointer->Target==NULL) {
				
				alienStatusPointer->Target=Alien_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr);
		
				if (alienStatusPointer->Target!=NULL) {
					textprint("Alien gets new target.\n");
		
					COPY_NAME(alienStatusPointer->Target_SBname,alienStatusPointer->Target->SBname);
		
					if (alienStatusPointer->BehaviourState!=ABS_Wait) {
						alienStatusPointer->BehaviourState=ABS_Hunt;
					}
					alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
			 		alienStatusPointer->NearStateTimer = 0;
		
				} else {
					textprint("Alien found no target!\n");
					if (alienStatusPointer->BehaviourState!=ABS_Wait) {
						AIMODULE *targetModule;
						/* Hunt or wander? */
						if ((AvP.PlayerType==I_Alien)||(Observer)) {
							/* Eew!  What a nasty hack!  Eeeww! */
							targetModule = FarNPC_GetTargetAIModuleForGlobalHunt(sbPtr);
						} else {
							/* This just makes them hunt a bit more... */
							targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,1);
						}

						if ((!targetModule)||(PlayerIsDeadAndNoLivingNetghosts())) {
							if (alienStatusPointer->BehaviourState!=ABS_Wander) {
								NPC_InitMovementData(&(alienStatusPointer->moveData));
								alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
						 		alienStatusPointer->NearStateTimer = 0;
							}
							alienStatusPointer->BehaviourState=ABS_Wander;
						} else {
							if (alienStatusPointer->BehaviourState!=ABS_Hunt) {
								alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;
						 		alienStatusPointer->NearStateTimer = 0;
							}
							alienStatusPointer->BehaviourState=ABS_Hunt;
						}
					}
				}
			}
		}
	}

	if(sbPtr->SBdptr) 
	{
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);
		#if 0
		textprint("Near Alien in module %s \n",sbPtr->containingModule->name);
		#endif
		NearAlienBehaviour(sbPtr);
		Alt_NearAliens++;
	}
	else
	{
		/* NB if this assert fires, we may just have run out of displayblocks */
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] == 0);
		#if 0
		textprint("Far Alien in module %s \n",sbPtr->containingModule->name);
		#endif
		FarAlienBehaviour(sbPtr);	
		Alt_FarAliens++;
	}

	/* if we have actually died, we need to remove the strategyblock... so
	do this here */
	if((alienStatusPointer->BehaviourState == ABS_Dying)&&(alienStatusPointer->NearStateTimer <= 0)) {

		if(alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(alienStatusPointer->soundHandle);				
		if(alienStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(alienStatusPointer->soundHandle2);
		DestroyAnyStrategyBlock(sbPtr);
	}

	/* Update containing module? */	
	if (sbPtr->containingModule!=alienStatusPointer->my_containing_module) {
		/* This is to slow down new hunting target computation. */
		alienStatusPointer->my_containing_module=sbPtr->containingModule;
	}

	/* Update delta playing flag. */
	{
		DELTA_CONTROLLER *hitdelta;
				
		hitdelta=Get_Delta_Sequence(&alienStatusPointer->HModelController,"HitDelta");
	
		if (hitdelta) {
			if (DeltaAnimation_IsFinished(hitdelta)) {
				hitdelta->Playing=0;
			}
		}
	}
}



/*----------------------Patrick 7/11/96-----------------------------
This pair of functions handle the alien visibility switching....
Note that the ai-pheromone system is maintained by these functions.
--------------------------------------------------------------------*/
void MakeAlienNear(STRATEGYBLOCK *sbPtr)
{
	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	ALIEN_STATUS_BLOCK *alienStatusPointer= (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

    LOCALASSERT(alienStatusPointer);
    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	/* first of all, see how many aliens are currently near: if there are too many,
	destroy this alien, and try to force a generator to make a replacement */
	if(NumGeneratorNPCsVisible() >= MAX_VISIBLEGENERATORNPCS)
	{
		DestroyAnyStrategyBlock(sbPtr);
		ForceAGenerator();
	}

	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;	
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr=NULL;
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;
	if(dPtr==NULL) return; /* cannot create displayblock: leave alien far */
		
	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					
                            
	
	/*Copy extents from the collision extents in extents.c*/
	dPtr->ObMinX=-CollisionExtents[CE_ALIEN].CollisionRadius;
	dPtr->ObMaxX=CollisionExtents[CE_ALIEN].CollisionRadius;
	dPtr->ObMinZ=-CollisionExtents[CE_ALIEN].CollisionRadius;
	dPtr->ObMaxZ=CollisionExtents[CE_ALIEN].CollisionRadius;
	dPtr->ObMinY=CollisionExtents[CE_ALIEN].CrouchingTop;
	dPtr->ObMaxY=CollisionExtents[CE_ALIEN].Bottom;
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
	alienStatusPointer->NearStateTimer = 0;
	alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;	
	alienStatusPointer->CurveRadius = 0;
	alienStatusPointer->CurveLength = 0;
	alienStatusPointer->CurveTimeOut = 0;

	/* zero linear velocity in dynamics block */
	dynPtr->LinVelocity.vx = 0;
	dynPtr->LinVelocity.vy = 0;
	dynPtr->LinVelocity.vz = 0;

	/* initialise our sequence data */
	//dPtr->ShapeAnimControlBlock = &alienStatusPointer->ShpAnimCtrl;
	dPtr->HModelControlBlock=&alienStatusPointer->HModelController;
	
	ProveHModel(dPtr->HModelControlBlock,dPtr);

	if(AlienShouldBeCrawling(sbPtr)) alienStatusPointer->IAmCrouched = 1;
	else alienStatusPointer->IAmCrouched = 0;

	RecomputeAlienSpeed(sbPtr);

	InitWaypointManager(&alienStatusPointer->waypointManager);

    /* initialise our near state and specific sequence */
	if ((alienStatusPointer->BehaviourState == ABS_Dying)
		||(alienStatusPointer->BehaviourState == ABS_Dormant)
		||(alienStatusPointer->BehaviourState == ABS_Awakening)) {
		/* Do nothing. */
	} else if(alienStatusPointer->BehaviourState == ABS_Wait) {
		NPC_InitMovementData(&(alienStatusPointer->moveData));
		alienStatusPointer->BehaviourState = ABS_Wait;
		if(alienStatusPointer->IAmCrouched) SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienCrouch,(int)ACrSS_Standard,ONE_FIXED);
		else SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienStand,(int)ASSS_Standard,ONE_FIXED);
	} else if(AlienIsAwareOfTarget(sbPtr)) {
		NPC_InitMovementData(&(alienStatusPointer->moveData));
		alienStatusPointer->BehaviourState = ABS_Approach;
		if(alienStatusPointer->IAmCrouched) SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienCrawl,(int)ACSS_Standard,ONE_FIXED>>1);
		else SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienRun,(int)ARSS_Standard,ONE_FIXED>>1);	
	} else {
		NPC_InitMovementData(&(alienStatusPointer->moveData));
		/* Was Wander, now Hunt.  Hunt should kick in wander if needed. */
		alienStatusPointer->BehaviourState = ABS_Hunt;
		if(alienStatusPointer->IAmCrouched) SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienCrawl,(int)ACSS_Standard,ONE_FIXED>>1);
		else SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienRun,(int)ARSS_Standard,ONE_FIXED>>1);	
	}
}  

void MakeAlienFar(STRATEGYBLOCK *sbPtr)
{
	/* get the alien's status block */
	ALIEN_STATUS_BLOCK *alienStatusPointer;    
	int i;
	
	LOCALASSERT(sbPtr);
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);   
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	if(sbPtr->SBdptr)
	{
		i = DestroyActiveObject(sbPtr->SBdptr);
		LOCALASSERT(i==0);
		sbPtr->SBdptr = NULL;
	}

	if(alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(alienStatusPointer->soundHandle);				
	if(alienStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(alienStatusPointer->soundHandle2);

    /* initialise our far state */
	if ((alienStatusPointer->BehaviourState!=ABS_Dying) 
		&&(alienStatusPointer->BehaviourState!=ABS_Dormant)
		&&(alienStatusPointer->BehaviourState!=ABS_Awakening)) {
		/* No zombie aliens here! */
		if(AlienIsAwareOfTarget(sbPtr))
		{
			alienStatusPointer->BehaviourState = ABS_Hunt;
		}
		else
		{
			alienStatusPointer->BehaviourState = ABS_Wander;
		}
	}

	NPC_InitWanderData(&(alienStatusPointer->wanderData));
	/* init state timers */
	alienStatusPointer->NearStateTimer = 0;
	alienStatusPointer->FarStateTimer = ALIEN_FAR_MOVE_TIME;	

	/* zero linear velocity in dynamics block */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
}


static void DoAlienAIHiss(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int pitch = (FastRandom() & 255) - 128;
	SOUNDINDEX soundIndex;
	ALIEN_STATUS_BLOCK *alienStatusPointer;

	GLOBALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);
	GLOBALASSERT(alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX);

	/* This one is for ALIEN DAMAGE SCREAM. */

	soundIndex=SID_NOSOUND;

	if (alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayAlienSound((int)alienStatusPointer->Type,ASC_Scream_Hurt,pitch,
			&alienStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);

		if (AvP.Network != I_No_Network)
		{
			AddNetMsg_SpotAlienSound(ASC_Scream_Hurt,(int)alienStatusPointer->Type,pitch,&dynPtr->Position);
		}
	
	}
	

}

static void DoAlienDeathScream(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	SOUNDINDEX soundIndex;
	ALIEN_STATUS_BLOCK *alienStatusPointer;

	GLOBALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);
	GLOBALASSERT(alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX);

	soundIndex=SID_NOSOUND;

	/* This one is for ALIEN DEATH SCREAM. */

	soundIndex=SID_NOSOUND;

	if (alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayAlienSound((int)alienStatusPointer->Type,ASC_Scream_Dying,0,
			&alienStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
		
		if (AvP.Network != I_No_Network)
		{
			AddNetMsg_SpotAlienSound((int)ASC_Scream_Dying,(int)alienStatusPointer->Type,0,&dynPtr->Position);
		}
	}
	

}

static void DoAlienDeathSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	SOUNDINDEX soundIndex;
	ALIEN_STATUS_BLOCK *alienStatusPointer;

	GLOBALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	soundIndex=SID_NOSOUND;

	/* This one is for ALIEN DEATH SOUND. */

	soundIndex=SID_NOSOUND;

	PlayAlienSound((int)alienStatusPointer->Type,ASC_Death,0,
		NULL,&sbPtr->DynPtr->Position);

	#if 0	
	if (AvP.Network != I_No_Network)
	{
		soundIndex=ActiveSounds[alienStatusPointer->soundHandle2].soundIndex;
		if (soundIndex!=SID_NOSOUND) {
			AddNetMsg_SpotAlienSound(soundIndex,pitch,&dynPtr->Position);
		}
	}
	#endif

}

extern void DoAlienLimbLossSound(VECTORCH *position) {

	/* This one is for ALIEN LIMBLOSS SOUND. */

	PlayAlienSound(0,ASC_LimbLoss,0,NULL,position);

}

/*----------------------Patrick 7/11/96-----------------------------
Handle weapon impact on an alien
--------------------------------------------------------------------*/
/* KJL 11:46:43 12/17/96 - rewritten */

// JB 16/6/97 rewritten
// Patrick 29/7/97 rewritten again. again.
// ChrisF 16/9/97 rewritten again again, again.
// ChrisF 26/11/97 rewritten again, again, again, again.
// ChrisF 1/4/98 rewritten again again again again again.  Okay, 'modified' then. Added directional parameter.
// ChrisF 20/11/98 rewritten again again again again again again, added hit delta support.
// ChrisF 15/2/99 rewritten again again again again again again again, added fragging noise.

void AlienIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming, DISPLAYBLOCK *frag)
{
	
	int tkd;
	ALIEN_STATUS_BLOCK *alienStatusPointer;
	GLOBALASSERT(sbPtr);

	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
		
	LOCALASSERT(sbPtr);

	alienStatusPointer->Wounds|=wounds;

	if (incoming) {
		textprint("Alien hit from %d %d %d\n",incoming->vx,incoming->vy,incoming->vz);
		/* Knockback effects? */
	}
	
	if (frag) {
		DoAlienLimbLossSound(&sbPtr->DynPtr->Position);
	}

#if 0
	ALIEN_STATUS_BLOCK *alienStatusPointer;
	GLOBALASSERT(sbPtr);
					
	/* reduce alien health */
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPointer);
	if(alienStatusPointer->Health>0) (alienStatusPointer->Health) -= damage; 	
#endif
	/* Perhaps I'd better explain... I now handle damage
	for you.  These functions now are only for special effects,
	and if you need to do anything when damage happens. */
	
	if (alienStatusPointer->BehaviourState==ABS_Dormant) {
		Alien_Awaken(sbPtr);
	}

	/* Speaking of which... */
	{

		/* reduce alien health */
		
		if (sbPtr->SBDamageBlock.Health <= 0) {

			/* Oh yes, kill them, too. */
			if (alienStatusPointer->BehaviourState!=ABS_Dying)
			{
				if (AvP.PlayerType!=I_Alien) {
					CurrentGameStats_CreatureKilled(sbPtr,Section);
				}
				KillAlien(sbPtr,wounds,damage,multiple,incoming);
			}

		} else {
			#if WOUNDING_SPEED_EFFECTS
			/* Alien wounding effects? */
			int factor;
			int changespeed;

			RecomputeAlienSpeed(sbPtr);

			/* Now, get anim speed. */
			factor=GetAlienSpeedFactor(sbPtr);
			changespeed=0;

			switch (alienStatusPointer->Type) {
				case AT_Standard:
					if (factor!=ONE_FIXED) {
						changespeed=1;
					}
					break;
				case AT_Predalien:
					if (factor!=PREDALIEN_SPEED_FACTOR) {
						changespeed=1;
					}
					break;
				case AT_Praetorian:
					if (factor!=PRAETORIAN_SPEED_FACTOR) {
						changespeed=1;
					}
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
	
			if (changespeed) {
				int newspeed;
				/* ONE_FIXED implies we're playing an invariant length anim. */
				newspeed=DIV_FIXED(alienStatusPointer->last_anim_length,factor);

				HModel_ChangeSpeed(&alienStatusPointer->HModelController,newspeed);
			}
			#endif
			
			/* If you got here, you should still be alive. */

			if ((alienStatusPointer->BehaviourState!=ABS_Dying)&&(alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX)) {
				DoAlienAIHiss(sbPtr);
			}

			tkd=TotalKineticDamage(damage);

			if (tkd>=10) {
				/* If not dead, play a hit delta. */
				DELTA_CONTROLLER *hitdelta;
				int frontback;
				
				hitdelta=Get_Delta_Sequence(&alienStatusPointer->HModelController,"HitDelta");
				
				if (incoming) {
					if (incoming->vz>=0) {
						frontback=0;
					} else {
						frontback=1;
					}
				} else {
					/* Default to front. */
					frontback=1;
				}
				
				/* Only do it from the front. */
				if ((hitdelta)&&(frontback)) {
					if (hitdelta->Playing==0) {
						/* A hierarchy with hit deltas! */
						int CrouchSubSequence;
						int StandSubSequence;
					
						if (Section==NULL) {
							if ((FastRandom()&65535)<32767) {
								CrouchSubSequence=ACrSS_Hit_Left;
								StandSubSequence=ASSS_Hit_Left;
							} else {
								CrouchSubSequence=ACrSS_Hit_Right;
								StandSubSequence=ASSS_Hit_Right;
							}
						} else if ((Section->sempai->flags&section_flag_left_arm)
							||(Section->sempai->flags&section_flag_left_hand)) {

							CrouchSubSequence=ACrSS_Hit_Left;
							StandSubSequence=ASSS_Hit_Left;
						} else if ((Section->sempai->flags&section_flag_right_arm)
							||(Section->sempai->flags&section_flag_right_hand)) {

							CrouchSubSequence=ACrSS_Hit_Right;
							StandSubSequence=ASSS_Hit_Right;

						} else {
							/* Chest or misc. hit. */
							if ((FastRandom()&65535)<32767) {
								CrouchSubSequence=ACrSS_Hit_Left;
								StandSubSequence=ASSS_Hit_Left;
							} else {
								CrouchSubSequence=ACrSS_Hit_Right;
								StandSubSequence=ASSS_Hit_Right;
							}
						}
						
					
						if(alienStatusPointer->IAmCrouched) {
							if (HModelSequence_Exists(&alienStatusPointer->HModelController,(int)HMSQT_AlienCrouch,CrouchSubSequence)) {
								Start_Delta_Sequence(hitdelta,(int)HMSQT_AlienCrouch,CrouchSubSequence,-1);
							}
						} else {
							if (HModelSequence_Exists(&alienStatusPointer->HModelController,(int)HMSQT_AlienStand,StandSubSequence)) {
								Start_Delta_Sequence(hitdelta,(int)HMSQT_AlienStand,StandSubSequence,-1);
							}
						}
						hitdelta->Playing=1;
						/* Not looped. */
						if (alienStatusPointer->BehaviourState==ABS_Attack) {
							StartAlienAttackSequence(sbPtr);
							alienStatusPointer->NearStateTimer = ALIEN_ATTACKTIME;
						}
					}
				}
			}
		}
	}
	
}	
	
/* patrick 29/7/97 -----------------------------------
This function to be called only from behaviour
-- ChrisF 1/4/98 What a stupid idea. This function ---
-- to be called only from AlienIsDamaged. ------------
------------------------------------------------------*/
void KillAlien(STRATEGYBLOCK *sbPtr,int wounds,DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming)
{	
	ALIEN_STATUS_BLOCK *alienStatusPointer;
	int tkd,deathtype;
	SECTION_DATA *head;

	LOCALASSERT(sbPtr);	
	LOCALASSERT(sbPtr->DynPtr);	

	
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	/* make a sound.  Just this once without a check. */

	DoAlienDeathSound(sbPtr);

	/*If alien has a death target ,send a request*/
	if(alienStatusPointer->death_target_sbptr)
	{
		RequestState(alienStatusPointer->death_target_sbptr,1, 0);
	} 
	
	/* Set up gibb factor. */

	tkd=TotalKineticDamage(damage);
	deathtype=0;

	if (damage->ExplosivePower==1) {
	 	/* Explosion case. */
	 	if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
	 		/* Okay, you can gibb now. */
	 		alienStatusPointer->GibbFactor=ONE_FIXED>>1;
			deathtype=2;
	 	}
	} else if ((tkd<40)&&((multiple>>16)>1)) {
	 	int newmult;

	 	newmult=DIV_FIXED(multiple,NormalFrameTime);
	 	if (MUL_FIXED(tkd,newmult)>700) {
	 		/* Excessive bullets case 1. */
	 		alienStatusPointer->GibbFactor=ONE_FIXED>>2;
			deathtype=2;
	 	} else if (MUL_FIXED(tkd,newmult)>250) {
	 		/* Excessive bullets case 2. */
	 		alienStatusPointer->GibbFactor=ONE_FIXED>>3;
			deathtype=1;
	 	}
	}

	/* Predaliens and preatorians only gibb for sadars. */
	if (alienStatusPointer->Type!=AT_Standard) {
		alienStatusPointer->GibbFactor=0;
		/* But retain deathtype. */
	}

	if ((damage->ExplosivePower==2)||(damage->ExplosivePower==6)) {
		/* Basically SADARS only. */
		if (alienStatusPointer->Type==AT_Standard) {
			alienStatusPointer->GibbFactor=ONE_FIXED;
		} else {
			alienStatusPointer->GibbFactor=ONE_FIXED>>2;
		}
		deathtype=3;
	}

	if (damage->ForceBoom) {
		deathtype+=damage->ForceBoom;
	}

	if ( (damage->Impact==0)
	   &&(damage->Cutting==0)	
	   &&(damage->Penetrative==0)	
	   &&(damage->Fire!=0)
	   &&(damage->Electrical==0)
	   &&(damage->Acid==0))
	{
		/* that sounds like the flamethrower... */
		alienStatusPointer->GibbFactor=ONE_FIXED>>2;
		/* Gibb just a little for now. */
	}
	
	if (damage->Id==AMMO_PREDPISTOL_STRIKE) {
		/* Blow up if hit by the bolt. */
		alienStatusPointer->GibbFactor=ONE_FIXED>>3;
	} else if (damage->Id==AMMO_FLECHETTE_POSTMAX) {
		alienStatusPointer->GibbFactor=ONE_FIXED>>2;
	}

	{
		SECTION_DATA *chest=GetThisSectionData(alienStatusPointer->HModelController.section_data,"chest");
		
		if (chest==NULL) {
			/* I'm impressed. */
			deathtype+=2;
		} else if ((chest->flags&section_data_notreal)
			&&(chest->flags&section_data_terminate_here)) {
			/* That's gotta hurt. */
			deathtype++;
		}
	}

	/* Gibb noise? */
	if (alienStatusPointer->GibbFactor) {
		DoAlienLimbLossSound(&sbPtr->DynPtr->Position);
	} else {
		/* make a sound... if you have a head. */
		head=GetThisSectionData(alienStatusPointer->HModelController.section_data,"head");

		/* Is it still attached? */
		if (head) {
			if (head->flags&section_data_notreal) {
				head=NULL;
			}
		}

		if ((alienStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX)&&(head)) {
			DoAlienDeathScream(sbPtr);
		}
	}

	/* More restrained death than before... */
	{
	

		DEATH_DATA *this_death;
		HIT_FACING facing;
		int electrical;
	
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

		if ((damage->Impact==0) 		
			&&(damage->Cutting==0)  	
			&&(damage->Penetrative==0)
			&&(damage->Fire==0)
			&&(damage->Electrical>0)
			&&(damage->Acid==0)
			) {
			electrical=1;
		} else {
			electrical=0;
		}

		this_death=GetAlienDeathSequence(&alienStatusPointer->HModelController,NULL,alienStatusPointer->Wounds,alienStatusPointer->Wounds,
			deathtype,&facing,0,alienStatusPointer->IAmCrouched,electrical);

		#if 0
		GLOBALASSERT(this_death);
		
		SetAlienShapeAnimSequence_Core(sbPtr,this_death->Sequence_Type,this_death->Sub_Sequence,
			this_death->Sequence_Length,this_death->TweeningTime);
 
		alienStatusPointer->NearStateTimer=ALIEN_DYINGTIME;
		alienStatusPointer->HModelController.Looped=0;
		alienStatusPointer->HModelController.LoopAfterTweening=0;
		/* switch state */
		alienStatusPointer->BehaviourState=ABS_Dying;

		/* stop sound, if we have one */
		if(alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(alienStatusPointer->soundHandle);				

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
		sbPtr->DynPtr->Mass	= 160;
		/* Okay... */
		#else
		Convert_Alien_To_Corpse(sbPtr,this_death,damage);
		#endif
	}
}

void Execute_Alien_Dying(STRATEGYBLOCK *sbPtr)
{
	ALIEN_STATUS_BLOCK *alienStatusPointer;    	

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);
	
	//sbPtr->DynPtr->LinVelocity.vx = 0;
	//sbPtr->DynPtr->LinVelocity.vy = 0;
	//sbPtr->DynPtr->LinVelocity.vz = 0;

	//sbPtr->DynPtr->LinImpulse.vx = 0;
	//sbPtr->DynPtr->LinImpulse.vy = 0;
	//sbPtr->DynPtr->LinImpulse.vz = 0;
	
	{
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = alienStatusPointer->NearStateTimer/2;

		}
	}
	alienStatusPointer->NearStateTimer -= NormalFrameTime;
}

void RecomputeAlienSpeed(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    	
	int factor,basespeed;

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);

	/* Change max speed. */
	factor=GetAlienSpeedFactor_ForSequence(sbPtr,HMSQT_AlienRun,(int)ARSS_Standard);
	
	switch (alienStatusPointer->Type) {
		case AT_Standard:
		default:
			basespeed=ALIEN_FORWARDVELOCITY;
			break;
		case AT_Predalien:
			basespeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,PREDALIEN_SPEED_FACTOR);
			break;
		case AT_Praetorian:
			/* Could this be dependent on crouching? */
			if (alienStatusPointer->IAmCrouched) {
				basespeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,MUL_FIXED(PRAETORIAN_CRAWLSPEED_FACTOR,PRAETORIAN_SPEED_FACTOR));
			} else {
				basespeed=MUL_FIXED(ALIEN_FORWARDVELOCITY,MUL_FIXED(PRAETORIAN_WALKSPEED_FACTOR,PRAETORIAN_SPEED_FACTOR));
			}
			break;
	}

	alienStatusPointer->MaxSpeed=MUL_FIXED(factor,basespeed);

}

/* Wounding effect function... */

int GetAlienSpeedFactor(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    	
	HMODEL_SEQUENCE_TYPES sequence_type;
	int subsequence;

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);
	
	sequence_type=(HMODEL_SEQUENCE_TYPES)alienStatusPointer->HModelController.Sequence_Type;
	subsequence=alienStatusPointer->HModelController.Sub_Sequence;
	/* That is what the controller thinks the shape is playing. */

	return(GetAlienSpeedFactor_ForSequence(sbPtr,sequence_type,subsequence));

}

int GetAlienSpeedFactor_ForSequence(STRATEGYBLOCK *sbPtr, HMODEL_SEQUENCE_TYPES sequence_type,int subsequence) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    	
	int factor,factortype;

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);

	factortype=-1;

	switch (sequence_type) {

		case HMSQT_AlienRun:
			switch ((ALIENRUN_SUBSEQUENCES)subsequence)	{
				case ARSS_Standard:
					factortype=2;
					break;
				case ARSS_Attack_Swipe:
				case ARSS_Jump:
					factortype=1;
					break;
				case ARSS_Dies:
					factortype=0;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			break;
		case HMSQT_AlienCrawl:
			switch ((ALIENCRAWL_SUBSEQUENCES)subsequence) {
				case ACSS_Standard:
				case ACSS_Crawl_Hurt:
				case ACSS_Scamper:
					factortype=2;
					break;
				case ACSS_Attack_Bite:
				case ACSS_Attack_Tail:
					factortype=1;
					break;
				case ACSS_Dies:
				case ACSS_Pain_Fall_Fwd:
				case ACSS_Pain_Fall_Back:
				case ACSS_Pain_Fall_Left:
				case ACSS_Pain_Fall_Right:
				case ACSS_Boom_Fall_Fwd:
				case ACSS_Boom_Fall_Back:
				case ACSS_Boom_Fall_Left:
				case ACSS_Boom_Fall_Right:
					factortype=0;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			break;
		case HMSQT_AlienStand:
			switch ((ALIENSTAND_SUBSEQUENCES)subsequence) {
				case ASSS_Taunt:
				case ASSS_Taunt2:
				case ASSS_Taunt3:
				case ASSS_Fear:
				case ASSS_Standard:
				case ASSS_FidgetA:
				case ASSS_FidgetB:
				case ASSS_Attack_Right_Swipe_In:
				case ASSS_Attack_Left_Swipe_In:
				case ASSS_Attack_Both_In:
				case ASSS_Attack_Both_Down:
				case ASSS_Attack_Bite:
				case ASSS_Attack_Tail:
				case ASSS_Attack_Low_Left_Swipe:
				case ASSS_Attack_Low_Right_Swipe:
				case ASSS_Feed:
				case ASSS_Unfurl:
				case ASSS_Dormant:
					factortype=1;
					break;
				case ASSS_Dies:
				case ASSS_Pain_Fall_Fwd:
				case ASSS_Pain_Fall_Back:
				case ASSS_Pain_Fall_Left:
				case ASSS_Pain_Fall_Right:
				case ASSS_Boom_Fall_Fwd:
				case ASSS_Boom_Fall_Back:
				case ASSS_Boom_Fall_Left:
				case ASSS_Boom_Fall_Right:
				case ASSS_Spin_Clockwise:
				case ASSS_Spin_Anticlockwise:
				case ASSS_BurningDeath:
					factortype=0;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			break;
		case HMSQT_AlienCrouch:
			switch ((ALIENCROUCH_SUBSEQUENCES)subsequence) {
				case ACrSS_Standard:
					factortype=2;
					break;
				case ACrSS_Attack_Bite:
				case ACrSS_Attack_Tail:
				case ACrSS_Attack_Swipe:
				case ACrSS_Pounce:
				case ACrSS_Taunt:
					factortype=1;
					break;
				case ACrSS_Dies:
				case ACrSS_Dies_Thrash:
					factortype=0;
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
			break;
		case HMSQT_Hugger:
		default:
			/* Nooooo! */
			GLOBALASSERT(0);
			break;
	}

	LOCALASSERT(factortype!=-1);

	switch (factortype) {
		case 0:
			return(ONE_FIXED);
			break;
		case 1:
			{
				/* Affected by alien type. */
				int prefactor;

				switch (alienStatusPointer->Type) {
					case AT_Standard:
						prefactor=ONE_FIXED;
						break;
					case AT_Predalien:
						prefactor=PREDALIEN_SPEED_FACTOR;
						break;
					case AT_Praetorian:
						prefactor=PRAETORIAN_SPEED_FACTOR;
						break;
					default:
						GLOBALASSERT(0);
						break;
				}

				return(prefactor);

			}
			break;
		case 2:
			{
				/* More complex.  Affected by alien type and health. */
				NPC_DATA *NpcData;
				int prefactor;
				
				switch (alienStatusPointer->Type) {
					case AT_Standard:
						NpcData=GetThisNpcData(I_NPC_Alien);
						prefactor=ONE_FIXED;
						break;
					case AT_Predalien:
						NpcData=GetThisNpcData(I_NPC_PredatorAlien);
						prefactor=PREDALIEN_SPEED_FACTOR;
						break;
					case AT_Praetorian:	
						NpcData=GetThisNpcData(I_NPC_PraetorianGuard);
						prefactor=PRAETORIAN_SPEED_FACTOR;
						break;
					default:
						GLOBALASSERT(0);
						break;
				}

				LOCALASSERT(NpcData);
				
				factor=sbPtr->SBDamageBlock.Health/NpcData->StartingStats.Health;
				/* ONE_FIXED shift already included. */
				LOCALASSERT(factor<=ONE_FIXED);

				factor+=ONE_FIXED;
				factor>>=1;
				
				/* Wounding effects. */
				if (alienStatusPointer->Wounds&(section_flag_left_leg|section_flag_right_leg)) {
					/* Missing a leg. */
					factor-=(ONE_FIXED/3);
				} else if ((alienStatusPointer->Wounds&section_flag_left_foot)
					&&(alienStatusPointer->Wounds&section_flag_right_foot)) {
					/* Missing both feet. */
					factor-=(ONE_FIXED>>2);
				} else if ((alienStatusPointer->Wounds&section_flag_left_foot)
					||(alienStatusPointer->Wounds&section_flag_right_foot)) {
					/* Missing one foot. */
					factor-=(ONE_FIXED>>3);
				}

				if (factor<(ONE_FIXED>>3)) {
					factor=(ONE_FIXED>>3);
				}

				if (prefactor!=ONE_FIXED) {
					factor=MUL_FIXED(prefactor,factor);
				}
	
				return(factor);
			}
			break;
		default:
			GLOBALASSERT(0);
			break;
	}

	return(0);

}

int Alien_TargetFilter(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourAlienPlayer:
		case I_BehaviourMarinePlayer:
		case I_BehaviourPredatorPlayer:
			{
				if (Observer) {
					return(0);
				}
				
				if(AvP.Network != I_No_Network)
				{
					//In multiplayer games we don't want the aliens to be going after
					//the host once he's dead. 
					PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (candidate->SBdataptr);
					if(!playerStatusPtr->IsAlive)
					{
						return(0);
					}
				}

				switch(AvP.PlayerType)
				{
					case I_Marine:
					case I_Predator:
						return(1);
						break;
					case I_Alien:
						return(0);
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
						return(1);
						break;
					case I_Alien:
						return(0);
						break;
					default:
						GLOBALASSERT(0);
						return(0);
						break;
				}
				break;
			}
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourAlien:
			{
				return(0);
				break;
			}
		case I_BehaviourPredator:
		case I_BehaviourXenoborg:
		case I_BehaviourMarine:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
			/* Valid. */
			return(1);
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourMarinePlayer:
					case I_BehaviourPredatorPlayer:
						return(1);
						//return(0);
						break;
					case I_BehaviourAlienPlayer:
					default:
						return(0);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

}

STRATEGYBLOCK *Alien_GetNewTarget(VECTORCH *alienpos, STRATEGYBLOCK *me) {

	int neardist;
	STRATEGYBLOCK *nearest;
	int a;
	STRATEGYBLOCK *candidate;
	MODULE *dmod;
	
	dmod=ModuleFromPosition(alienpos,playerPherModule);
	
	LOCALASSERT(dmod);
	
	nearest=NULL;
	neardist=ONE_FIXED;
	
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate!=me) {
			if (candidate->DynPtr) {
				if (Alien_TargetFilter(candidate)) {
					VECTORCH offset;
					int dist;
		
					offset.vx=alienpos->vx-candidate->DynPtr->Position.vx;
					offset.vy=alienpos->vy-candidate->DynPtr->Position.vy;
					offset.vz=alienpos->vz-candidate->DynPtr->Position.vz;
			
					dist=Approximate3dMagnitude(&offset);
					/* Preferentially ignore predators? */
					if (candidate->I_SBtype==I_BehaviourPredator) {
						dist<<=2;
					}
		
					if (dist<neardist) {
						/* Check visibility? */
						//if (candidate->SBdptr) {
							if (!NPC_IsDead(candidate)) {
								if ((IsModuleVisibleFromModule(dmod,candidate->containingModule))) {
									nearest=candidate;
									neardist=dist;
								}	
							}
						//}
					}
				}
			}
		}
	}

	#if 0
	if (nearest==NULL) {
		if (Alien_TargetFilter(Player->ObStrategyBlock)) {
			nearest=Player->ObStrategyBlock;
		} else {
			nearest=NULL; /* Erk! */
		}
	}
	#endif
	
	return(nearest);

}

void Alien_Awaken(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    	

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);

	alienStatusPointer->BehaviourState = ABS_Awakening;

	if(HModelSequence_Exists(&alienStatusPointer->HModelController,HMSQT_AlienStand,ASSS_Unfurl)) {
		SetAlienShapeAnimSequence_Core(sbPtr,HMSQT_AlienStand,ASSS_Unfurl,-1,(ONE_FIXED>>2));
	} else {
		SetAlienShapeAnimSequence_Core(sbPtr,HMSQT_AlienStand,ASSS_Standard,(ONE_FIXED),(ONE_FIXED>>2));
	}
	alienStatusPointer->HModelController.LoopAfterTweening=0;

}


void Alien_GoToApproach(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPointer;    	

	LOCALASSERT(sbPtr);
	alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(alienStatusPointer);

	NPC_InitMovementData(&(alienStatusPointer->moveData));
	alienStatusPointer->BehaviourState = ABS_Approach;
	InitWaypointManager(&alienStatusPointer->waypointManager);
	if(alienStatusPointer->IAmCrouched) {
		SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienCrawl,(int)ACSS_Standard,ONE_FIXED>>1);
	} else {
		SetAlienShapeAnimSequence(sbPtr,HMSQT_AlienRun,(int)ARSS_Standard,ONE_FIXED>>1);			
	}
	RecomputeAlienSpeed(sbPtr);
	alienStatusPointer->CurveTimeOut = 0;
	alienStatusPointer->NearStateTimer = 0;	

}

int AlienIsCrawling(STRATEGYBLOCK *sbPtr) {
	
	/* For external calls. */

	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    LOCALASSERT(dynPtr);

	if (sbPtr->I_SBtype!=I_BehaviourAlien) {
		return(0);
	}

	if (dynPtr->UseStandardGravity!=0) {
		return(0);
	} else {
		return(1);
	}
}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct alien_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	ALIEN_TYPE Type;
	signed char Health;
	int GibbFactor;
	int Wounds;
	int last_anim_length;
	ALIEN_BHSTATE BehaviourState;	
	int PounceDetected;
	int JumpDetected;
	int EnablePounce;
	

	int incidentFlag;
	int incidentTimer;

	ALIEN_MISSION Mission;

	int CurveRadius;
	int CurveLength;
	int CurveTimeOut;
	int FarStateTimer;
	int NearStateTimer;
	int IAmCrouched;


	int huntingModuleIndex;
	int currentAttackCode;
//	NPC_MOVEMENTDATA moveData;
	NPC_WANDERDATA wanderData;

//	HMODELCONTROLLER HModelController;

//	STRATEGYBLOCK* generator_sbptr;//0 unless created by a generator

	char Target_SBname[SB_NAME_LENGTH];

	char Generator_SBname[SB_NAME_LENGTH];

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}ALIEN_SAVE_BLOCK;



//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV alienStatusPointer


void LoadStrategy_Alien(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	ALIEN_STATUS_BLOCK* alienStatusPointer;
	ALIEN_SAVE_BLOCK* block = (ALIEN_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);

	if(sbPtr)
	{
		//make sure the strategy found is of the right type
		if(sbPtr->I_SBtype != I_BehaviourAlien) return;
	}
	else
	{
		//we will have to generate an alien then
		TOOLS_DATA_ALIEN tda;

		//make sure the alien is in a module 
		if(!ModuleFromPosition(&block->dynamics.Position,NULL)) return;
		
		sbPtr = CreateActiveStrategyBlock();
		if(!sbPtr) return;

		sbPtr->I_SBtype = I_BehaviourAlien;
		sbPtr->shapeIndex = 0;
		sbPtr->maintainVisibility = 1;
		COPY_NAME(sbPtr->SBname,block->header.SBname);

		//create using a fake tools data
		tda.position = block->dynamics.Position;
		tda.shapeIndex = 0;
		COPY_NAME(tda.nameID,block->header.SBname);
		COPY_NAME(tda.death_target_ID,Null_Name);
		tda.type = block->Type;
		tda.start_inactive = 0;

		tda.starteuler.EulerX = 0;
		tda.starteuler.EulerY = 0;
		tda.starteuler.EulerZ = 0;
 
 		EnableBehaviourType(sbPtr,I_BehaviourAlien , &tda );

 	}


	alienStatusPointer = (ALIEN_STATUS_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(Type)
	COPYELEMENT_LOAD(Health)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(Wounds)
	COPYELEMENT_LOAD(last_anim_length)
	COPYELEMENT_LOAD(BehaviourState)
	COPYELEMENT_LOAD(PounceDetected)
	COPYELEMENT_LOAD(JumpDetected)
	COPYELEMENT_LOAD(EnablePounce)
	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)
	COPYELEMENT_LOAD(Mission)
	COPYELEMENT_LOAD(CurveRadius)
	COPYELEMENT_LOAD(CurveLength)
	COPYELEMENT_LOAD(CurveTimeOut)
	COPYELEMENT_LOAD(FarStateTimer)
	COPYELEMENT_LOAD(NearStateTimer)
	COPYELEMENT_LOAD(IAmCrouched)
	COPYELEMENT_LOAD(wanderData)

	//load target
	COPY_NAME(alienStatusPointer->Target_SBname,block->Target_SBname);
	alienStatusPointer->Target = FindSBWithName(alienStatusPointer->Target_SBname);

	//load hunting module
	if(block->huntingModuleIndex>=0 && block->huntingModuleIndex< AIModuleArraySize)
	{
		alienStatusPointer->huntingModule = &AIModuleArray[block->huntingModuleIndex];
	}
	else
	{
		alienStatusPointer->huntingModule = NULL;
	}

	//get the alien's attack from the attack code
	alienStatusPointer->current_attack = GetThisAttack_FromUniqueCode(block->currentAttackCode);
					 
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//find the alien's generator
	alienStatusPointer->generator_sbptr = FindSBWithName(block->Generator_SBname);

	//load the aliens hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&alienStatusPointer->HModelController);
		}
	}

	Load_SoundState(&alienStatusPointer->soundHandle);
	Load_SoundState(&alienStatusPointer->soundHandle2);
}


void SaveStrategy_Alien(STRATEGYBLOCK* sbPtr)
{
	ALIEN_SAVE_BLOCK *block;
	ALIEN_STATUS_BLOCK* alienStatusPointer;
	
	
	alienStatusPointer = (ALIEN_STATUS_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(Type)
	COPYELEMENT_SAVE(Health)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(Wounds)
	COPYELEMENT_SAVE(last_anim_length)
	COPYELEMENT_SAVE(BehaviourState)
	COPYELEMENT_SAVE(PounceDetected)
	COPYELEMENT_SAVE(JumpDetected)
	COPYELEMENT_SAVE(EnablePounce)
	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)
	COPYELEMENT_SAVE(Mission)
	COPYELEMENT_SAVE(CurveRadius)
	COPYELEMENT_SAVE(CurveLength)
	COPYELEMENT_SAVE(CurveTimeOut)
	COPYELEMENT_SAVE(FarStateTimer)
	COPYELEMENT_SAVE(NearStateTimer)
	COPYELEMENT_SAVE(IAmCrouched)
	COPYELEMENT_SAVE(wanderData)

	
	//save target
	COPY_NAME(block->Target_SBname,alienStatusPointer->Target_SBname);

	//save hunting module
	if(alienStatusPointer->huntingModule)
	{
		block->huntingModuleIndex = alienStatusPointer->huntingModule->m_index;
	}
	else
	{
		block->huntingModuleIndex = -1;
	}

	//save attack code
	if(alienStatusPointer->current_attack)
	{
		block->currentAttackCode = alienStatusPointer->current_attack->Unique_Code;
	}
	else
	{
		block->currentAttackCode = -1;
	}

	//save the alien's generator name
	if(alienStatusPointer->generator_sbptr)
	{
		COPY_NAME(block->Generator_SBname,alienStatusPointer->generator_sbptr->SBname);
	}
	else
	{
		COPY_NAME(block->Generator_SBname,Null_Name);
	}
	

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the aliens hierarchy
	SaveHierarchy(&alienStatusPointer->HModelController);

	Save_SoundState(&alienStatusPointer->soundHandle);
	Save_SoundState(&alienStatusPointer->soundHandle2);
}
