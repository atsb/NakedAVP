// RWH moved out here to provide easier access

/* CDF 25/7/98 - A completely new file. */

#include "3dc.h"		   
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "dynblock.h"
#include "dynamics.h"

#include "weapons.h"
#include "comp_shp.h"
#include "inventry.h"
#include "triggers.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "bh_swdor.h"
#include "bh_plift.h"
#include "load_shp.h"
#include "bh_weap.h"
#include "bh_debri.h"
#include "lighting.h"
#include "bh_lnksw.h"
#include "bh_binsw.h"
#include "pheromon.h"
#include "bh_pred.h"
#include "bh_agun.h"
#include "plat_shp.h"
#include "psnd.h"
#include "ai_sight.h"
#include "sequnces.h"
#include "huddefs.h"
#include "showcmds.h"
#include "sfx.h"
#include "bh_marin.h"
#include "bh_far.h"
#include "targeting.h"
#include "dxlog.h"
#include "los.h"
#include "psndplat.h"
#include "bh_dummy.h"
#include "bh_corpse.h"

/* for win95 net game support */
#include "pldghost.h"
#include "pldnet.h"

#define SENTRYGUN_DRAMA 0

int SentrygunSpread=5;

extern int NormalFrameTime;
extern unsigned char Null_Name[8];
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
void CreateSentrygun(VECTORCH *Position,int type);

void AGunMovement_ScanLeftRight(STRATEGYBLOCK *sbPtr,int rate);
void AGunMovement_Centre(STRATEGYBLOCK *sbPtr,int rate);
void Execute_AGun_Target(STRATEGYBLOCK *sbPtr);
void AGun_MaintainGun(STRATEGYBLOCK *sbPtr);
void Execute_AGun_Dying(STRATEGYBLOCK *sbPtr);

SOUND3DDATA SentryFire_SoundData={
	{0,0,0,},
	{0,0,0,},
	20000,
	40000,
};

SOUND3DDATA SentryWhirr_SoundData={
	{0,0,0,},
	{0,0,0,},
	0,
	32000,
};

/* Begin Code! */

void CreatePlayerAutogun(void) {
	/* Yow, this actually gets called! */
}

void CastSentrygun(void) {

	#define BOTRANGE 2000

	VECTORCH position;

	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO SENTRYGUNS IN MULTIPLAYER MODE");
		return;
	}

	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateSentrygun(&position, 0);

}

void CreateSentrygun(VECTORCH *Position,int type)
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

	sbPtr->I_SBtype = I_BehaviourAutoGun;

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
		dynPtr->UseDisplacement=0;

		dynPtr->Mass=10000; /* As opposed to 160. */
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE GUN: DYNBLOCK CREATION FAILURE");
		return;
	}

	sbPtr->shapeIndex = 0;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* create, initialise and attach an alien data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(AUTOGUN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		AUTOGUN_STATUS_BLOCK *agunStatus = (AUTOGUN_STATUS_BLOCK *)sbPtr->SBdataptr;

		/* Initialise xenoborg's stats */
		{
			NPC_DATA *NpcData;
	
			NpcData=GetThisNpcData(I_NPC_SentryGun);
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		
		agunStatus->behaviourState=I_tracking;
		agunStatus->Target=NULL; 
		COPY_NAME(agunStatus->Target_SBname,Null_Name);
		agunStatus->targetTrackPos.vx=0;
		agunStatus->targetTrackPos.vy=0;
		agunStatus->targetTrackPos.vz=0;

		agunStatus->stateTimer=0;
		agunStatus->Gun_Pan=0;
		agunStatus->Gun_Tilt=0;
		
		agunStatus->gunpandir=0;
		agunStatus->guntiltdir=0;
		
		agunStatus->IAmFar=1;
		agunStatus->Firing=0;
		agunStatus->Drama=0;
		agunStatus->OnTarget=0;
		agunStatus->OnTarget_LastFrame=0;
		agunStatus->WhirrSoundOn=0;
		agunStatus->GunFlash=NULL;
		agunStatus->soundHandle=SOUND_NOACTIVEINDEX;
		agunStatus->soundHandle2=SOUND_NOACTIVEINDEX;

		agunStatus->ammo=500;
		agunStatus->roundsFired=0;
		agunStatus->volleyFired=0;

		agunStatus->incidentFlag=0;
		agunStatus->incidentTimer=0;

		agunStatus->HModelController.section_data=NULL;
		agunStatus->HModelController.Deltas=NULL;

		for(i=0;i<SB_NAME_LENGTH;i++) agunStatus->death_target_ID[i] =0; 
		agunStatus->death_target_sbptr=0;
		agunStatus->death_target_request=0;

		root_section=GetNamedHierarchyFromLibrary("sentry","gun");
				
		if (!root_section) {
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE GUN: NO HMODEL");
			return;
		}
		Create_HModel(&agunStatus->HModelController,root_section);
		InitHModelSequence(&agunStatus->HModelController,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED);

		{
			DELTA_CONTROLLER *delta;

			delta=Add_Delta_Sequence(&agunStatus->HModelController,"GunTilt",(int)HMSQT_Xenoborg,(int)XBSS_Head_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&agunStatus->HModelController,"GunPan",(int)HMSQT_Xenoborg,(int)XBSS_Head_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

		}

		/* Containment test NOW! */
		if(!(sbPtr->containingModule))
		{
			/* no containing module can be found... abort*/
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE GUN: MODULE CONTAINMENT FAILURE");
			return;
		}
		LOCALASSERT(sbPtr->containingModule);
	
		MakeSentrygunNear(sbPtr);

		NewOnScreenMessage("SENTRYGUN CREATED");
	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE GUN: MALLOC FAILURE");
		return;
	}
	if(AvP.Network != I_No_Network) 
	{
		AddNetGameObjectID(sbPtr);
	}
}

void MakeSentrygunNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    

	LOCALASSERT(sbPtr);
	dynPtr = sbPtr->DynPtr;
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(agunStatusPointer);	          		
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
	dPtr->ShapeAnimControlBlock = NULL;
	dPtr->ObTxAnimCtrlBlks = NULL;

	dPtr->HModelControlBlock=&agunStatusPointer->HModelController;

	ProveHModel(dPtr->HModelControlBlock,dPtr);

	agunStatusPointer->IAmFar=0;

}

void AutoGunBehaveInit(void *bhdata,STRATEGYBLOCK *sbPtr) {

	AUTOGUN_TOOLS_TEMPLATE *toolsData;
	int i;

	LOCALASSERT(bhdata);
	toolsData = (AUTOGUN_TOOLS_TEMPLATE *)bhdata; 
	LOCALASSERT(sbPtr);

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network) 
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* make the assumption that the loader has initialised the strategy block sensibly... 
	so just set the shapeIndex from the tools data & copy the name id*/
	sbPtr->shapeIndex = toolsData->shapenum;
	for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];

	
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->OrientEuler = toolsData->orientation;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);

		dynPtr->Mass=10000; /* As opposed to 160. */
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* create, initialise and attach an alien data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(AUTOGUN_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		AUTOGUN_STATUS_BLOCK *agunStatus = (AUTOGUN_STATUS_BLOCK *)sbPtr->SBdataptr;

		/* Initialise xenoborg's stats */
		{
			NPC_DATA *NpcData;
	
			NpcData=GetThisNpcData(I_NPC_SentryGun);
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		if (toolsData->startInactive) {
			agunStatus->behaviourState=I_inactive;
		} else {	
			agunStatus->behaviourState=I_tracking;
		}
		agunStatus->Target=NULL; 
		COPY_NAME(agunStatus->Target_SBname,Null_Name);
		agunStatus->targetTrackPos.vx=0;
		agunStatus->targetTrackPos.vy=0;
		agunStatus->targetTrackPos.vz=0;

		agunStatus->stateTimer=0;
		agunStatus->Gun_Pan=0;
		agunStatus->Gun_Tilt=0;
		
		agunStatus->gunpandir=0;
		agunStatus->guntiltdir=0;
		
		agunStatus->IAmFar=1;
		agunStatus->Firing=0;
		agunStatus->Drama=0;
		agunStatus->OnTarget=0;
		agunStatus->OnTarget_LastFrame=0;
		agunStatus->WhirrSoundOn=0;
		agunStatus->GunFlash=NULL;
		agunStatus->soundHandle=SOUND_NOACTIVEINDEX;
		agunStatus->soundHandle2=SOUND_NOACTIVEINDEX;

		agunStatus->ammo=toolsData->ammo;
		agunStatus->roundsFired=0;
		agunStatus->volleyFired=0;

		agunStatus->incidentFlag=0;
		agunStatus->incidentTimer=0;

		agunStatus->HModelController.section_data=NULL;
		agunStatus->HModelController.Deltas=NULL;

		for(i=0;i<SB_NAME_LENGTH;i++) agunStatus->death_target_ID[i] =toolsData->death_target_ID[i]; 
		agunStatus->death_target_sbptr=0;
		agunStatus->death_target_request=toolsData->death_target_request;

		root_section=GetNamedHierarchyFromLibrary("sentry","gun");
				
		if (!root_section) {
			RemoveBehaviourStrategy(sbPtr);
			return;
		}
		Create_HModel(&agunStatus->HModelController,root_section);
		InitHModelSequence(&agunStatus->HModelController,HMSQT_Xenoborg,XBSS_Powered_Up_Standard,ONE_FIXED);

		{
			DELTA_CONTROLLER *delta;

			delta=Add_Delta_Sequence(&agunStatus->HModelController,"GunTilt",(int)HMSQT_Xenoborg,(int)XBSS_Head_Vertical_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

			delta=Add_Delta_Sequence(&agunStatus->HModelController,"GunPan",(int)HMSQT_Xenoborg,(int)XBSS_Head_Horizontal_Delta,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
			delta->Active=0;

		}

		/* Containment test NOW! */
		if(!(sbPtr->containingModule))
		{
			/* no containing module can be found... abort*/
			RemoveBehaviourStrategy(sbPtr);
			return;
		}
		LOCALASSERT(sbPtr->containingModule);
	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

}

void MakeSentrygunFar(STRATEGYBLOCK *sbPtr)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    
	int i;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);	          		
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;

	/* if we have any gun flashes, get rid of them */
	if(agunStatusPointer->GunFlash)
	{
		RemoveNPCGunFlashEffect(agunStatusPointer->GunFlash);
		agunStatusPointer->GunFlash = NULL;
	}

	/* agun data block init */
	if(agunStatusPointer->behaviourState != I_disabled) {
   		agunStatusPointer->stateTimer=0;
	}

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;	

	agunStatusPointer->IAmFar=1;

}

static void Autogun_VerifyDeltaControllers(STRATEGYBLOCK *sbPtr) {

	AUTOGUN_STATUS_BLOCK *agunStatusPointer;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(agunStatusPointer);	          		

	/* Nothing has deltas like a xenoborg does. */
	/* But sentryguns try their best. */

	agunStatusPointer->gun_pan=Get_Delta_Sequence(&agunStatusPointer->HModelController,"GunPan");
	GLOBALASSERT(agunStatusPointer->gun_pan);

	agunStatusPointer->gun_tilt=Get_Delta_Sequence(&agunStatusPointer->HModelController,"GunTilt");
	GLOBALASSERT(agunStatusPointer->gun_tilt);

}

static void AGun_ComputeDeltaValues(STRATEGYBLOCK *sbPtr)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;
	int angle;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	/* Interpret all status block values, and apply to deltas. */
	/* Gun Pan first. */

	angle=agunStatusPointer->Gun_Pan>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-4096;

	GLOBALASSERT(agunStatusPointer->gun_pan);

	/* Now, we have an angle. */

	if (angle>SGUN_PAN_GIMBALL) {
		angle=SGUN_PAN_GIMBALL;
	} else if (angle<-SGUN_PAN_GIMBALL) {
		angle=-SGUN_PAN_GIMBALL;
	}
	
	{
		int fake_timer;

		if (angle>0) {
		
			fake_timer=DIV_FIXED(angle,(SGUN_PAN_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer>=65536) fake_timer=65535;
			if (fake_timer<=0) fake_timer=0;

		} else {
		
			fake_timer=DIV_FIXED(angle,(SGUN_PAN_GIMBALL<<1));
			fake_timer+=32767;
			if (fake_timer>=65536) fake_timer=65535;
			if (fake_timer<=0) fake_timer=0;

		}

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		if (agunStatusPointer->gun_pan->timer!=fake_timer) {
			agunStatusPointer->WhirrSoundOn=1;
		}
		agunStatusPointer->gun_pan->timer=fake_timer;

	}

	/* Gun Tilt... */

	angle=agunStatusPointer->Gun_Tilt>>4;

	if (angle>=3072) angle-=4096;
	if (angle>=2048) angle=angle-3072;
	if (angle> 1024) angle=2048-angle;

	GLOBALASSERT(agunStatusPointer->gun_tilt);

	/* Now, we have an angle. */

	if (angle>SGUN_PITCH_GIMBALL) {
		angle=SGUN_PITCH_GIMBALL;
	} else if (angle<-SGUN_PITCH_GIMBALL) {
		angle=-SGUN_PITCH_GIMBALL;
	}
	
	GLOBALASSERT(angle>=-1024);
	GLOBALASSERT(angle<=1024);

	{
		int fake_timer;

		fake_timer=1024-angle;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;

		fake_timer=65536-fake_timer;

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		if (agunStatusPointer->gun_tilt->timer!=fake_timer) {
			agunStatusPointer->WhirrSoundOn=1;
		}
		agunStatusPointer->gun_tilt->timer=fake_timer;

	}

}

void AGun_UpdateTargetTrackPos(STRATEGYBLOCK *sbPtr) {

	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	if (agunStatusPointer->Target==NULL) {
		agunStatusPointer->targetTrackPos.vx=0;
		agunStatusPointer->targetTrackPos.vy=0;
		agunStatusPointer->targetTrackPos.vz=0;
		return;
	}

	GetTargetingPointOfObject_Far(agunStatusPointer->Target,&agunStatusPointer->targetTrackPos);

}

int Autogun_TargetFilter(STRATEGYBLOCK *candidate) {

	/* Reject NULLs and far targets. */
	if (candidate==NULL) {
		return(0);
	}

	if (candidate->SBdptr==NULL) {
		return(0);
	}

	/* Shoot pretty much anything. */
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
						return(1);
						break;
					case I_Marine:
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
					case I_Alien:
					case I_Predator:
						return(1);
						break;
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
		case I_BehaviourAlien:
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourPredator:
		case I_BehaviourPredatorAlien:
			{
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					return(1);
				}
				break;
			}
		case I_BehaviourSeal:
		case I_BehaviourMarine:
			{
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					return(0);
				}
				break;
			}
		case I_BehaviourXenoborg:
			return(1);
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

STRATEGYBLOCK *Autogun_GetNewTarget(STRATEGYBLOCK *sbPtr) {

	int neardist;
	STRATEGYBLOCK *nearest;
	int a;
	STRATEGYBLOCK *candidate;
	MODULE *dmod;
	
	dmod=ModuleFromPosition((&sbPtr->DynPtr->Position),playerPherModule);
	
	LOCALASSERT(dmod);
	
	nearest=NULL;
	neardist=ONE_FIXED;
	
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate!=sbPtr) {
			if (candidate->DynPtr) {
				if (Autogun_TargetFilter(candidate)) {
					VECTORCH offset;
					int dist;
		
					offset.vx=(&sbPtr->DynPtr->Position)->vx-candidate->DynPtr->Position.vx;
					offset.vy=(&sbPtr->DynPtr->Position)->vy-candidate->DynPtr->Position.vy;
					offset.vz=(&sbPtr->DynPtr->Position)->vz-candidate->DynPtr->Position.vz;
			
					dist=Approximate3dMagnitude(&offset);
		
					if (dist<neardist) {
						/* Check visibility? */
						if (NPCCanSeeTarget(sbPtr,candidate,AGUN_NEAR_VIEW_WIDTH)) {
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
	
	return(nearest);

}

int AGunSight_FrustrumReject(VECTORCH *localOffset) {

	VECTORCH fixed_offset;

	#if 0
	PrintDebuggingText("Local Offset: %d %d %d\n",localOffset->vx,localOffset->vy,localOffset->vz);
	#endif

	fixed_offset=*localOffset;
	fixed_offset.vy-=300; /* ish */

	if (((fixed_offset.vz <0) && (
		((fixed_offset.vy) < (-fixed_offset.vz))&&(fixed_offset.vy>=0)))
 		||((fixed_offset.vy<0)&&((-fixed_offset.vy) < (-fixed_offset.vz))
 		)) {
		/* 180 horizontal, 90 vertical. */
		return(1);
	} else {
		return(0);
	}

}

void AutoGunBehaveFun(STRATEGYBLOCK* sbPtr) {

	AUTOGUN_STATUS_BLOCK *agunStatusPointer;
	int agunIsNear;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(agunStatusPointer);	          		

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	}

	if(sbPtr->SBdptr) {
		agunIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		agunIsNear=0;
	}

	Autogun_VerifyDeltaControllers(sbPtr);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	
	sbPtr->DynPtr->LinImpulse.vx = 0;
	sbPtr->DynPtr->LinImpulse.vy = 0;
	sbPtr->DynPtr->LinImpulse.vz = 0;

	/* Target handling. */
	if (Validate_Target(agunStatusPointer->Target,agunStatusPointer->Target_SBname)==0) {
		agunStatusPointer->Target=NULL;
	}

	if (agunStatusPointer->Target) {
		if (!(Autogun_TargetFilter(agunStatusPointer->Target))) {
			agunStatusPointer->Target=NULL;
		}
	}

	if (agunStatusPointer->Firing) {
		agunStatusPointer->Firing-=NormalFrameTime;
	}
	if (agunStatusPointer->Firing<0) {
		agunStatusPointer->Firing=0;
	}

	if (agunStatusPointer->Target==NULL) {
		//if ((agunIsNear)||(agunStatusPointer->incidentFlag)) {
		if (agunIsNear) {
			/* Get new target. */
			agunStatusPointer->Target=Autogun_GetNewTarget(sbPtr);
			if (agunStatusPointer->Target) {
				COPY_NAME(agunStatusPointer->Target_SBname,agunStatusPointer->Target->SBname);
			}
			AGun_UpdateTargetTrackPos(sbPtr);
		}
	} else if (NPCCanSeeTarget(sbPtr,agunStatusPointer->Target,AGUN_NEAR_VIEW_WIDTH)) {
		AGun_UpdateTargetTrackPos(sbPtr);	
	} else {
		/* We have a target that we can't see. */
		agunStatusPointer->Target=NULL;
		AGun_UpdateTargetTrackPos(sbPtr);	
	}
	
	/* Unset incident flag. */
	agunStatusPointer->incidentFlag=0;
	agunStatusPointer->incidentTimer-=NormalFrameTime;
	
	if (agunStatusPointer->incidentTimer<0) {
		agunStatusPointer->incidentFlag=1;
		agunStatusPointer->incidentTimer=32767+(FastRandom()&65535);
	}

	if (sbPtr->SBDamageBlock.IsOnFire) {

		/* Why not? */
		CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);
	
		if (agunStatusPointer->incidentFlag) {
			if ((FastRandom()&65535)<32767) {
				sbPtr->SBDamageBlock.IsOnFire=0;
			}
		}

	}

	/* Now, switch by state. */
	switch (agunStatusPointer->behaviourState) {
		case I_inactive:
			AGunMovement_Centre(sbPtr,2);
			agunStatusPointer->Target=NULL;
			break;
		case I_tracking:
			if (agunStatusPointer->Target==NULL) {
				AGunMovement_ScanLeftRight(sbPtr,2);
			} else {
				Execute_AGun_Target(sbPtr);
			}
			break;
		case I_disabled:
			/* Dying function. */
			Execute_AGun_Dying(sbPtr);
			break;
		default:
			/* No action? */
			break;
	}

	/* if we have actually died, we need to remove the strategyblock... so
	do this here */
	if((agunStatusPointer->behaviourState == I_disabled)&&(agunStatusPointer->stateTimer <= 0)) {

		DestroyAnyStrategyBlock(sbPtr);
	}

	agunStatusPointer->WhirrSoundOn=0;
	AGun_ComputeDeltaValues(sbPtr);
	if ((agunStatusPointer->WhirrSoundOn)&&(agunStatusPointer->behaviourState!=I_disabled)) {
		int dist;

		dist=VectorDistance(&Player->ObWorld,&sbPtr->DynPtr->Position);

		if (dist<SentryWhirr_SoundData.outer_range) {
			/* Play whirr sound.  Start and End sounds removed for artistic reasons. */
			if (agunStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
				SentryWhirr_SoundData.position=sbPtr->DynPtr->Position;
				Sound_Play(SID_ED_SENTRYTURN01,"nel",&SentryWhirr_SoundData,&agunStatusPointer->soundHandle2);
			} else {
				GLOBALASSERT(ActiveSounds[agunStatusPointer->soundHandle2].soundIndex==SID_ED_SENTRYTURN01);
			}
		} else {
			/* Stop whirr sound. */
			if (agunStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(agunStatusPointer->soundHandle2);
			}
		}
	} else {
		/* Stop whirr sound. */
		if (agunStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(agunStatusPointer->soundHandle2);
		}
	}

	/* Verify firing. */
	if (agunStatusPointer->behaviourState == I_disabled) {
		agunStatusPointer->Firing=0;
	}

	ProveHModel_Far(&agunStatusPointer->HModelController,sbPtr);

	/* Now, are we firing? */
	
	if (agunStatusPointer->IAmFar) {
		/* Just to be sure, for now. */
		agunStatusPointer->Firing=0;
	}
	AGun_MaintainGun(sbPtr);

	/* Think sounds. */
	if (agunStatusPointer->Firing==0) {
		if (agunStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[agunStatusPointer->soundHandle].soundIndex==SID_SENTRY_GUN) {
				Sound_Play(SID_SENTRY_END,"d",&sbPtr->DynPtr->Position);
			}
			Sound_Stop(agunStatusPointer->soundHandle);
		}
		/* Sequences! */
		if (agunStatusPointer->HModelController.Sub_Sequence!=XBSS_Powered_Up_Standard) {
			InitHModelTweening(&agunStatusPointer->HModelController,(ONE_FIXED>>3),(int)HMSQT_Xenoborg,XBSS_Powered_Up_Standard,(ONE_FIXED),1);
		}
	} else {
		if (agunStatusPointer->HModelController.Sub_Sequence!=XBSS_Fire_Bolter) {
			InitHModelTweening(&agunStatusPointer->HModelController,(ONE_FIXED>>3),(int)HMSQT_Xenoborg,XBSS_Fire_Bolter,(ONE_FIXED>>2),1);
		}
	}

	if (agunStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Update3d(agunStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void AGunMovement_ScanLeftRight(STRATEGYBLOCK *sbPtr,int rate)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	/* Let's wave the gun around, half full tracking. */
	if (agunStatusPointer->gunpandir) {
		agunStatusPointer->Gun_Pan+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan>(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Pan=(SGUN_PAN_GIMBALL<<3);
			agunStatusPointer->gunpandir=0;
		}
	} else {
		agunStatusPointer->Gun_Pan-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan<-(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Pan=-(SGUN_PAN_GIMBALL<<3);
			agunStatusPointer->gunpandir=1;
		}
	}
	if (agunStatusPointer->gun_pan) {
		agunStatusPointer->gun_pan->Active=1;
	}

	/* And centre tilt. */
	if (agunStatusPointer->Gun_Tilt<0) {
		agunStatusPointer->Gun_Tilt+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt>(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt>0) {
			agunStatusPointer->Gun_Tilt=0;
		}
	} else if (agunStatusPointer->Gun_Tilt>0) {
		agunStatusPointer->Gun_Tilt-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt<-(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=-(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt<0) {
			agunStatusPointer->Gun_Tilt=0;
		}
	}
	if (agunStatusPointer->gun_tilt) {
		agunStatusPointer->gun_tilt->Active=1;
	}
}

void AGunMovement_Centre(STRATEGYBLOCK *sbPtr,int rate)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	/* Let's centre pan... */
	if (agunStatusPointer->Gun_Pan<0) {
		agunStatusPointer->Gun_Pan+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan>(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Pan=(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Pan>0) {
			agunStatusPointer->Gun_Pan=0;
		}
	} else if (agunStatusPointer->Gun_Pan>0) {
		agunStatusPointer->Gun_Pan-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan<-(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Pan=-(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Pan<0) {
			agunStatusPointer->Gun_Pan=0;
		}
	}
	if (agunStatusPointer->gun_pan) {
		agunStatusPointer->gun_pan->Active=1;
	}

	/* And centre tilt. */
	if (agunStatusPointer->Gun_Tilt<0) {
		agunStatusPointer->Gun_Tilt+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt>(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt>0) {
			agunStatusPointer->Gun_Tilt=0;
		}
	} else if (agunStatusPointer->Gun_Tilt>0) {
		agunStatusPointer->Gun_Tilt-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt<-(SGUN_PAN_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=-(SGUN_PAN_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt<0) {
			agunStatusPointer->Gun_Tilt=0;
		}
	}
	if (agunStatusPointer->gun_tilt) {
		agunStatusPointer->gun_tilt->Active=1;
	}
}

void AGun_GetRelativeAngles(STRATEGYBLOCK *sbPtr, int *anglex, int *angley, VECTORCH *pivotPoint) {
	
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	
	MATRIXCH WtoL;
	VECTORCH targetPos;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	/* First, extract relative angle. */

	WtoL=sbPtr->DynPtr->OrientMat;
	TransposeMatrixCH(&WtoL);
	targetPos=agunStatusPointer->targetTrackPos;
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

int AGunMovement_TrackToAngles(STRATEGYBLOCK *sbPtr,int rate,int in_anglex,int in_angley)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	
	int real_anglex,angley,online;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	/* Turn the gun to face a certain way. */

	real_anglex=in_anglex;
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

	if (agunStatusPointer->Gun_Pan<real_anglex) {
		agunStatusPointer->Gun_Pan+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan>(SGUN_PAN_GIMBALL<<4)) {
			agunStatusPointer->Gun_Pan=(SGUN_PAN_GIMBALL<<4);
		} else if (agunStatusPointer->Gun_Pan>real_anglex) {
			agunStatusPointer->Gun_Pan=real_anglex;
			online++;
		}
	} else if (agunStatusPointer->Gun_Pan>real_anglex) {
		agunStatusPointer->Gun_Pan-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Pan<-(SGUN_PAN_GIMBALL<<4)) {
			agunStatusPointer->Gun_Pan=-(SGUN_PAN_GIMBALL<<4);
		} else if (agunStatusPointer->Gun_Pan<real_anglex) {
			agunStatusPointer->Gun_Pan=real_anglex;
			online++;
		}
	} else {
		online++;
	}

	if (agunStatusPointer->gun_pan) {
		agunStatusPointer->gun_pan->Active=1;
	}

	/* Now y. */
	angley=-angley;
	/* Oops. */

	/* Note that Sentryguns now only track vertically HALF their full traverse. */
	/* Those shifts used to be <<4. */

	if (agunStatusPointer->Gun_Tilt<angley) {
		agunStatusPointer->Gun_Tilt+=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt>(SGUN_PITCH_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=(SGUN_PITCH_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt>angley) {
			agunStatusPointer->Gun_Tilt=angley;
			online++;
		}
	} else if (agunStatusPointer->Gun_Tilt>angley) {
		agunStatusPointer->Gun_Tilt-=(NormalFrameTime>>rate);
		if (agunStatusPointer->Gun_Tilt<-(SGUN_PITCH_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=-(SGUN_PITCH_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt<angley) {
			agunStatusPointer->Gun_Tilt=angley;
			online++;
		}
	} else {
		online++;
	}

	if (agunStatusPointer->gun_tilt) {
		agunStatusPointer->gun_tilt->Active=1;
	}

	if (online>1) {
		return(1);
	} else {
		return(0);
	}
}

void Execute_AGun_Target(STRATEGYBLOCK *sbPtr) {

	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	
	int anglex,angley;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);

	{
		SECTION_DATA *gun_section;

		gun_section=GetThisSectionData(agunStatusPointer->HModelController.section_data,"gun");
		GLOBALASSERT(gun_section);
		
		AGun_GetRelativeAngles(sbPtr,&anglex,&angley,&gun_section->World_Offset);

	}

	agunStatusPointer->OnTarget_LastFrame=agunStatusPointer->OnTarget;

	if (AGunMovement_TrackToAngles(sbPtr,2,anglex,angley)) {
		/* Pointing at target! */
		agunStatusPointer->OnTarget=1;

		if (agunStatusPointer->OnTarget_LastFrame==0) {
			/* Newly aquired! */
			//Sound_Play(SID_SENTRYGUN_LOCK,"d",&sbPtr->DynPtr->Position);
			agunStatusPointer->Drama=SENTRYGUN_DRAMA;
		}

		if (agunStatusPointer->Drama<=0) {
			if (agunStatusPointer->Firing==0) {
				/* Renew firing. */
				agunStatusPointer->Firing=(ONE_FIXED>>1);
			}
		} else {
			agunStatusPointer->Firing=0;
			agunStatusPointer->Drama-=NormalFrameTime;
			if (agunStatusPointer->Drama<0) {
				agunStatusPointer->Drama=0;
			}
		}
	} else {
		agunStatusPointer->OnTarget=0;
	};

}

void AGun_MaintainGun(STRATEGYBLOCK *sbPtr)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;
	SECTION_DATA *dum;
	VECTORCH alpha;
	VECTORCH beta;
	int multiple,volley_section,totalrounds;

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);
	
	dum=GetThisSectionData(agunStatusPointer->HModelController.section_data,"flash dummy");

	if ((agunStatusPointer->Firing==0)||(dum==NULL)||(agunStatusPointer->IAmFar)) {
		/* Not firing, go away. */
		if (agunStatusPointer->GunFlash) {
			RemoveNPCGunFlashEffect(agunStatusPointer->GunFlash);
			agunStatusPointer->GunFlash = NULL;
		}
		return;
	}
	
	/* Okay, must be firing.  Did we get anyone? */
	
	volley_section=AGUN_ROF*NormalFrameTime;

	agunStatusPointer->volleyFired+=volley_section;
	if (agunStatusPointer->volleyFired>(AGUN_VOLLEYSIZE<<ONE_FIXED_SHIFT)) {
		agunStatusPointer->volleyFired=(AGUN_VOLLEYSIZE<<ONE_FIXED_SHIFT);
	}
	
	totalrounds=agunStatusPointer->volleyFired>>ONE_FIXED_SHIFT;
	GLOBALASSERT(totalrounds>=agunStatusPointer->roundsFired);
	multiple=totalrounds-agunStatusPointer->roundsFired;

	if ((multiple)&&(agunStatusPointer->ammo==0)) {
		/* Click ineffectually. */
		if (agunStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[agunStatusPointer->soundHandle].soundIndex!=SID_NOAMMO) {
				Sound_Stop(agunStatusPointer->soundHandle);
			}
		}
		if (agunStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
			Sound_Play(SID_NOAMMO,"del",&sbPtr->DynPtr->Position,&agunStatusPointer->soundHandle);
		}
		if(agunStatusPointer->GunFlash)
		{
			RemoveNPCGunFlashEffect(agunStatusPointer->GunFlash);
			agunStatusPointer->GunFlash = NULL;
		}
		return;
	}

	if (multiple>agunStatusPointer->ammo) {
		multiple=agunStatusPointer->ammo;
		agunStatusPointer->ammo=0;
	} else {
		agunStatusPointer->ammo-=multiple;
	}
	agunStatusPointer->roundsFired+=multiple;
	
	/* End of volley? */
	if (agunStatusPointer->volleyFired==(AGUN_VOLLEYSIZE<<ONE_FIXED_SHIFT)) {
		agunStatusPointer->volleyFired=0;
		agunStatusPointer->roundsFired=0;
		agunStatusPointer->Target=NULL;
	}

	while (multiple) {
		/* Set up LOS. */
		//get a random rotation , for error in firing direction
		MATRIXCH rotate;
		
		multiple--;
		
		{
			EULER e;
			//convert angle from degrees to the engine units
			int spread=(SentrygunSpread*4096)/360;
			e.EulerX=(MUL_FIXED(FastRandom()&0xffff,spread*2)-spread)&wrap360;
			e.EulerY=(MUL_FIXED(FastRandom()&0xffff,spread*2)-spread)&wrap360;
			e.EulerZ=0;
			CreateEulerMatrix(&e,&rotate);
		}
		alpha = dum->World_Offset;
		beta.vx=dum->SecMat.mat31;
		beta.vy=dum->SecMat.mat32;
		beta.vz=dum->SecMat.mat33;

		RotateVector(&beta,&rotate);
		
		FindPolygonInLineOfSight(&beta,&alpha,0,sbPtr->SBdptr);
		
		/* Now deal with LOS_ObjectHitPtr. */
		if (LOS_ObjectHitPtr) {
			if (LOS_HModel_Section) {
				if (LOS_ObjectHitPtr->ObStrategyBlock) {
					if (LOS_ObjectHitPtr->ObStrategyBlock->SBdptr) {
						GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
					}
				}
			}
			
			/*Don't allow the sentrygun to destroy 'special' inanimate objects.
			  Doing so ruins some puzzles*/
			if(LOS_ObjectHitPtr->ObStrategyBlock)
			{
				STRATEGYBLOCK* hit_sbptr=LOS_ObjectHitPtr->ObStrategyBlock;
				if(hit_sbptr->I_SBtype==I_BehaviourInanimateObject)
				{
			        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr=(INANIMATEOBJECT_STATUSBLOCK*)hit_sbptr->SBdataptr;
					if(objectstatusptr->event_target)
					{
						/*This object has best be left*/
						continue;
					}
						
				}
			}

			/* this fn needs updating to take amount of damage into account etc. */
			HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AMMO_AUTOGUN,&beta, ONE_FIXED, LOS_HModel_Section);
		}
	}

	/* Do muzzle flash... */
	if (agunStatusPointer->GunFlash==NULL) {
		agunStatusPointer->GunFlash = AddNPCGunFlashEffect(
		  	&dum->World_Offset,&dum->SecMat,
 			SFX_MUZZLE_FLASH_SMARTGUN);
	} else {
		MaintainNPCGunFlashEffect(agunStatusPointer->GunFlash,&dum->World_Offset,&dum->SecMat);
	}
	/* Do the sound. */
	if (agunStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		if (ActiveSounds[agunStatusPointer->soundHandle].soundIndex!=SID_SENTRY_GUN) {
			Sound_Stop(agunStatusPointer->soundHandle);
		}
	}
	if (agunStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {

		SentryFire_SoundData.position=sbPtr->DynPtr->Position;

		Sound_Play(SID_SENTRY_GUN,"nel",&SentryFire_SoundData,&agunStatusPointer->soundHandle);
	}

}

void Execute_AGun_Dying(STRATEGYBLOCK *sbPtr)
{
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);
	
	/* Droop the gun. */
	if (agunStatusPointer->Gun_Tilt<8192) {
		agunStatusPointer->Gun_Tilt+=(NormalFrameTime>>3);
		if (agunStatusPointer->Gun_Tilt>(SGUN_PITCH_GIMBALL<<3)) {
			agunStatusPointer->Gun_Tilt=(SGUN_PITCH_GIMBALL<<3);
		} else if (agunStatusPointer->Gun_Tilt>8192) {
			agunStatusPointer->Gun_Tilt=8192;
		}
	}

	{
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = agunStatusPointer->stateTimer/2;

			if (dispPtr->ObFlags2<ONE_FIXED) {
				agunStatusPointer->HModelController.DisableBleeding=1;
			}
		}
	}
	agunStatusPointer->stateTimer -= NormalFrameTime;
}

static void KillAGun(STRATEGYBLOCK *sbPtr,int wounds,DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming)
{	  
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    	

	LOCALASSERT(sbPtr);
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(agunStatusPointer);
	
	/* make an explosion sound */    
    Sound_Play(SID_SENTRYGUNDEST,"d",&sbPtr->DynPtr->Position);  

	agunStatusPointer->stateTimer=AGUN_DYINGTIME;
	agunStatusPointer->HModelController.Looped=0;
	agunStatusPointer->HModelController.LoopAfterTweening=0;
	/* switch state */
	agunStatusPointer->behaviourState=I_disabled;
	
	/* Fiddle with sequences? */

	/* Ensure sufficient sparking. */
	KillRandomSections(agunStatusPointer->HModelController.section_data,(ONE_FIXED>>1));

 	if(agunStatusPointer->death_target_sbptr)
	{
		RequestState(agunStatusPointer->death_target_sbptr,agunStatusPointer->death_target_request, 0);
	} 

	if (agunStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		/* Well, it shouldn't be! */
		Sound_Stop(agunStatusPointer->soundHandle);
	}

}

void AGunIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,VECTORCH *incoming)
{
	
	AUTOGUN_STATUS_BLOCK *agunStatusPointer;    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);	   	                
	agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(agunStatusPointer);	          		

	if (sbPtr->SBDamageBlock.Health <= 0) {

		/* Oh yes, kill them, too. */
		if (agunStatusPointer->behaviourState!=I_disabled) {
			KillAGun(sbPtr,wounds,damage,multiple,incoming);
		}
	}	

}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct agun_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	AG_STATE behaviourState;
	int stateTimer;

	VECTORCH targetTrackPos;

	int Gun_Pan;
	int Gun_Tilt;

	int incidentFlag;
	int incidentTimer;

	int ammo;
	int roundsFired;
	int volleyFired;

	int Firing;
	int WhirrSoundOn;
	int Drama;

  	unsigned int createdByPlayer:1;
	unsigned int gunpandir	:1;
	unsigned int guntiltdir	:1;
	unsigned int IAmFar	:1;

	unsigned int OnTarget	:1;
	unsigned int OnTarget_LastFrame	:1;
//annoying pointer related things

	char Target_SBname[SB_NAME_LENGTH];


//strategy block stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;

}AGUN_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV agunStatusPointer

void LoadStrategy_Autogun(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	AUTOGUN_STATUS_BLOCK* agunStatusPointer;
	AGUN_SAVE_BLOCK* block = (AGUN_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourAutoGun) return;

	agunStatusPointer =(AUTOGUN_STATUS_BLOCK*) sbPtr->SBdataptr;

	
	//start copying stuff
	COPYELEMENT_LOAD(behaviourState)
	COPYELEMENT_LOAD(stateTimer)
	COPYELEMENT_LOAD(targetTrackPos)
	COPYELEMENT_LOAD(Gun_Pan)
	COPYELEMENT_LOAD(Gun_Tilt)
	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)
	COPYELEMENT_LOAD(ammo)
	COPYELEMENT_LOAD(roundsFired)
	COPYELEMENT_LOAD(volleyFired)
	COPYELEMENT_LOAD(Firing)
	COPYELEMENT_LOAD(WhirrSoundOn)
	COPYELEMENT_LOAD(Drama)
	COPYELEMENT_LOAD(createdByPlayer)
	COPYELEMENT_LOAD(gunpandir)
	COPYELEMENT_LOAD(guntiltdir)
	COPYELEMENT_LOAD(IAmFar)
	COPYELEMENT_LOAD(OnTarget)
	COPYELEMENT_LOAD(OnTarget_LastFrame)

	//load target
	COPY_NAME(agunStatusPointer->Target_SBname,block->Target_SBname);
	agunStatusPointer->Target = FindSBWithName(agunStatusPointer->Target_SBname);


	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&agunStatusPointer->HModelController);
		}
	}

	//get delta controller pointers
	Autogun_VerifyDeltaControllers(sbPtr);

	Load_SoundState(&agunStatusPointer->soundHandle);
	Load_SoundState(&agunStatusPointer->soundHandle2);
	
}

void SaveStrategy_Autogun(STRATEGYBLOCK* sbPtr)
{
	AUTOGUN_STATUS_BLOCK* agunStatusPointer;
	AGUN_SAVE_BLOCK* block;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	agunStatusPointer =(AUTOGUN_STATUS_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_SAVE(behaviourState)
	COPYELEMENT_SAVE(stateTimer)
	COPYELEMENT_SAVE(targetTrackPos)
	COPYELEMENT_SAVE(Gun_Pan)
	COPYELEMENT_SAVE(Gun_Tilt)
	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)
	COPYELEMENT_SAVE(ammo)
	COPYELEMENT_SAVE(roundsFired)
	COPYELEMENT_SAVE(volleyFired)
	COPYELEMENT_SAVE(Firing)
	COPYELEMENT_SAVE(WhirrSoundOn)
	COPYELEMENT_SAVE(Drama)
	COPYELEMENT_SAVE(createdByPlayer)
	COPYELEMENT_SAVE(gunpandir)
	COPYELEMENT_SAVE(guntiltdir)
	COPYELEMENT_SAVE(IAmFar)
	COPYELEMENT_SAVE(OnTarget)
	COPYELEMENT_SAVE(OnTarget_LastFrame)

	//save target
	COPY_NAME(block->Target_SBname,agunStatusPointer->Target_SBname);
	
	//save strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&agunStatusPointer->HModelController);

	Save_SoundState(&agunStatusPointer->soundHandle);
	Save_SoundState(&agunStatusPointer->soundHandle2);
	
}
