/* bh_corpse.c 19/8/98 */
#include "3dc.h"

#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "lighting.h"

#include "pfarlocs.h"
#include "pvisible.h"
#include "pheromon.h"
#include "bh_gener.h"
#include "bh_far.h"
#include "bh_pred.h"
#include "bh_marin.h"
#include "bh_weap.h"
#include "bh_debri.h"
#include "bh_alien.h"
#include "bh_xeno.h"
#include "psnd.h"
#include "weapons.h"
#include "load_shp.h"
#include "particle.h"
#include "sfx.h"
#include "huddefs.h"
#include "pldghost.h"
#include "pldnet.h"
#include "psndplat.h"
#include "ai_sight.h"
#include "los.h"
#include "bh_corpse.h"

#include "dxlog.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "sequnces.h"
#include "showcmds.h"
#include "extents.h"

extern int NormalFrameTime;
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern HITLOCATIONTABLE *GetThisHitLocationTable(char *id);
extern MARINE_WEAPON_DATA *GetThisNPCMarineWeapon(MARINE_NPC_WEAPONS this_id);

void SetCorpseAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);

/* these functions are called directly by the visibility management system */
void MakeCorpseNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;
	NETCORPSEDATABLOCK *corpseData = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	LOCALASSERT(corpseData);
    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr = NULL; /* this is important */	
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;
	if(dPtr==NULL) return; /* cannot create displayblock, so leave object "far" */
		
	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					
	dPtr->HModelControlBlock=NULL;

	/* need to initialise positional information in the new display block */ 
	/*Must be done before ProveHModel*/
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;	
	
	 /* set the animation sequence, if we're a player corpse */
	{
		/* Okay, no messing, you MUST be a player corpse. */
		dPtr->HModelControlBlock=&corpseData->HModelController;
		ProveHModel(dPtr->HModelControlBlock,dPtr);
    }
                            

}

void MakeCorpseFar(STRATEGYBLOCK *sbPtr)
{
	int i;
	LOCALASSERT(sbPtr->SBdptr != NULL);

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;	
}

void Convert_Alien_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death,DAMAGE_PROFILE* damage) {

	NETCORPSEDATABLOCK *corpseDataPtr;
	ALIEN_STATUS_BLOCK *alienStatusPointer;
		
	alienStatusPointer=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	/* Convert an alien... to a corpse. */

    GLOBALASSERT(sbPtr);
	GLOBALASSERT(this_death);
	GLOBALASSERT(alienStatusPointer);

	/* Inform the network. */
	if (AvP.Network != I_No_Network)
	{
		AddNetMsg_AlienAIKilled(sbPtr,this_death->Multiplayer_Code,ALIEN_DYINGTIME,alienStatusPointer->GibbFactor,damage);
	}

	corpseDataPtr = (void *)AllocateMem(sizeof(NETCORPSEDATABLOCK));
	GLOBALASSERT(corpseDataPtr);
		
	/* Fill in corpseDataPtr... */
	corpseDataPtr->SoundHandle  = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle2 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle3 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle4 = SOUND_NOACTIVEINDEX;

	corpseDataPtr->Type=I_BehaviourAlien;
	corpseDataPtr->GibbFactor=alienStatusPointer->GibbFactor;
	corpseDataPtr->This_Death=this_death;

	corpseDataPtr->CloakStatus = PCLOAK_Off;
	corpseDataPtr->CloakTimer = 0;
	corpseDataPtr->destructTimer = -1;
	corpseDataPtr->WeaponMisfireFunction=NULL;
	corpseDataPtr->My_Gunflash_Section=NULL;
	corpseDataPtr->My_Weapon=NULL;
	corpseDataPtr->weapon_variable=0;
	corpseDataPtr->Android=0;
	corpseDataPtr->ARealMarine=0;
	corpseDataPtr->TemplateRoot=NULL;
	corpseDataPtr->DeathFiring=0;
	corpseDataPtr->Wounds=0;
	
	switch (alienStatusPointer->Type) {
		case AT_Standard:
		default:
			corpseDataPtr->subtype = 0;
			corpseDataPtr->hltable=GetThisHitLocationTable("alien");
			break;
		case AT_Predalien:
			corpseDataPtr->subtype = 1;
			corpseDataPtr->hltable=GetThisHitLocationTable("predalien");
			break;
		case AT_Praetorian:
			corpseDataPtr->subtype = 2;
			corpseDataPtr->hltable=GetThisHitLocationTable("praetorian");
			break;
	}

	/* Remember wounds, or not? */
	Splice_HModels(&corpseDataPtr->HModelController,alienStatusPointer->HModelController.section_data);
	
	/* Heh... now bin the old data block! */
	if(alienStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(alienStatusPointer->soundHandle);
	}
	if(alienStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(alienStatusPointer->soundHandle2);
	}
	Dispel_HModel(&alienStatusPointer->HModelController);
	DeallocateMem(sbPtr->SBdataptr);
	/* Turn into the corpse. */
	sbPtr->SBdataptr=corpseDataPtr;
	sbPtr->I_SBtype=I_BehaviourNetCorpse;

 	SetCorpseAnimSequence_Core(sbPtr,this_death->Sequence_Type,this_death->Sub_Sequence,
 		this_death->Sequence_Length,this_death->TweeningTime);
	
	if (sbPtr->SBdptr) {
		/* Swap controllers round. */
		sbPtr->SBdptr->HModelControlBlock=&corpseDataPtr->HModelController;
		ProveHModel(&corpseDataPtr->HModelController,sbPtr->SBdptr);
	} else {
		ProveHModel_Far(&corpseDataPtr->HModelController,sbPtr);
	}

	corpseDataPtr->timer=ALIEN_DYINGTIME;
	corpseDataPtr->validityTimer=CORPSE_VALIDITY_TIME;
	corpseDataPtr->HModelController.Looped=0;
	corpseDataPtr->HModelController.LoopAfterTweening=0;

	/* stop motion */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->Friction	= 400000;
	sbPtr->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->LinVelocity.vx;
	sbPtr->DynPtr->LinImpulse.vy+=sbPtr->DynPtr->LinVelocity.vy;
	sbPtr->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->LinVelocity.vz;
	sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
	sbPtr->DynPtr->CanClimbStairs = 0;
	/* Experiment... */
	sbPtr->DynPtr->UseStandardGravity=1;
	sbPtr->DynPtr->Mass	= 160;
	/* Okay... */

	/* KJL 17:19:35 27/08/98 - ignore the player, other body parts, etc */
	sbPtr->DynPtr->OnlyCollideWithEnvironment = 1;

	/* Electric death sound? */
	if (corpseDataPtr->This_Death->Electrical) {
		Sound_Play(SID_ED_ELEC_DEATH,"de",&sbPtr->DynPtr->Position,&corpseDataPtr->SoundHandle4);
	}
}

void Convert_Predator_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death) {

	NETCORPSEDATABLOCK *corpseDataPtr;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;
		
	predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	/* Convert a predator... to a corpse. */

    GLOBALASSERT(sbPtr);
	GLOBALASSERT(this_death);
	GLOBALASSERT(predatorStatusPointer);

	corpseDataPtr = (void *)AllocateMem(sizeof(NETCORPSEDATABLOCK));
	GLOBALASSERT(corpseDataPtr);
		
	/* Fill in corpseDataPtr... */
	corpseDataPtr->SoundHandle  = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle2 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle3 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle4 = SOUND_NOACTIVEINDEX;

	corpseDataPtr->Type=I_BehaviourPredator;
	corpseDataPtr->GibbFactor=predatorStatusPointer->GibbFactor;
	corpseDataPtr->This_Death=this_death;

	corpseDataPtr->CloakStatus = predatorStatusPointer->CloakStatus;
	corpseDataPtr->CloakTimer  = predatorStatusPointer->CloakTimer;

	if ((predatorStatusPointer->behaviourState==PBS_SelfDestruct)
		&&(predatorStatusPointer->internalState==1)) {
		corpseDataPtr->destructTimer = predatorStatusPointer->stateTimer;
	} else {
		corpseDataPtr->destructTimer = -1;
	}

	corpseDataPtr->WeaponMisfireFunction=NULL;
	corpseDataPtr->My_Gunflash_Section=NULL;
	corpseDataPtr->weapon_variable=0;
	corpseDataPtr->Android=0;
	corpseDataPtr->ARealMarine=0;
	corpseDataPtr->TemplateRoot=NULL;
	corpseDataPtr->My_Weapon=NULL;
	corpseDataPtr->DeathFiring=0;
	corpseDataPtr->subtype = 0;
	corpseDataPtr->Wounds=0;

	corpseDataPtr->hltable=GetThisHitLocationTable(predatorStatusPointer->Selected_Weapon->HitLocationTableName);

	/* Remember wounds, or not? */
	Splice_HModels(&corpseDataPtr->HModelController,predatorStatusPointer->HModelController.section_data);
	
	if (this_death->Template) {
		SECTION *root;
		/* Convert to template. */
		root=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
		Transmogrify_HModels(sbPtr,&corpseDataPtr->HModelController,
			root, 1, 0,0);
	}

	/* Heh... now bin the old data block! */
	if(predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(predatorStatusPointer->soundHandle);
	}
	Dispel_HModel(&predatorStatusPointer->HModelController);
	DeallocateMem(sbPtr->SBdataptr);
	/* Turn into the corpse. */
	sbPtr->SBdataptr=corpseDataPtr;
	sbPtr->I_SBtype=I_BehaviourNetCorpse;

 	SetCorpseAnimSequence_Core(sbPtr,this_death->Sequence_Type,this_death->Sub_Sequence,
 		this_death->Sequence_Length,this_death->TweeningTime);
	
	if (sbPtr->SBdptr) {
		/* Swap controllers round. */
		sbPtr->SBdptr->HModelControlBlock=&corpseDataPtr->HModelController;
		ProveHModel(&corpseDataPtr->HModelController,sbPtr->SBdptr);
	} else {
		ProveHModel_Far(&corpseDataPtr->HModelController,sbPtr);
	}

	corpseDataPtr->timer=PRED_DIETIME;
	corpseDataPtr->validityTimer=CORPSE_VALIDITY_TIME;
	corpseDataPtr->HModelController.Looped=0;
	corpseDataPtr->HModelController.LoopAfterTweening=0;

	/* stop motion */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->Friction	= 400000;
	sbPtr->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->LinVelocity.vx;
	sbPtr->DynPtr->LinImpulse.vy+=sbPtr->DynPtr->LinVelocity.vy;
	sbPtr->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->LinVelocity.vz;
	sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
	sbPtr->DynPtr->CanClimbStairs = 0;

	/* KJL 17:19:35 27/08/98 - ignore the player, other body parts, etc */
	sbPtr->DynPtr->OnlyCollideWithEnvironment = 1;

	/* Electric death sound? */
	if (corpseDataPtr->This_Death->Electrical) {
		Sound_Play(SID_ED_ELEC_DEATH,"de",&sbPtr->DynPtr->Position,&corpseDataPtr->SoundHandle4);
	}

}

void Convert_Marine_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death) {

	NETCORPSEDATABLOCK *corpseDataPtr;
	MARINE_STATUS_BLOCK *marineStatusPointer;
		
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	/* Convert a predator... to a corpse. */

    GLOBALASSERT(sbPtr);
	GLOBALASSERT(this_death);
	GLOBALASSERT(marineStatusPointer);

	corpseDataPtr = (void *)AllocateMem(sizeof(NETCORPSEDATABLOCK));
	GLOBALASSERT(corpseDataPtr);
		
	/* Fill in corpseDataPtr... */
	corpseDataPtr->SoundHandle  = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle2 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle3 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle4 = SOUND_NOACTIVEINDEX;

	corpseDataPtr->Type=I_BehaviourMarine;
	corpseDataPtr->GibbFactor=marineStatusPointer->GibbFactor;
	corpseDataPtr->This_Death=this_death;

	corpseDataPtr->CloakStatus = PCLOAK_Off;
	corpseDataPtr->CloakTimer = 0;
	corpseDataPtr->destructTimer = -1;

	corpseDataPtr->My_Weapon=marineStatusPointer->My_Weapon;
	corpseDataPtr->WeaponMisfireFunction=marineStatusPointer->My_Weapon->WeaponMisfireFunction;
	corpseDataPtr->My_Gunflash_Section=marineStatusPointer->My_Gunflash_Section;
	corpseDataPtr->TemplateRoot=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->TemplateName);
	corpseDataPtr->Android=marineStatusPointer->Android;
	corpseDataPtr->ARealMarine=marineStatusPointer->My_Weapon->ARealMarine;
	corpseDataPtr->weapon_variable=0;
	corpseDataPtr->Wounds=0;

	corpseDataPtr->subtype = 0;
	corpseDataPtr->hltable=GetThisHitLocationTable(marineStatusPointer->My_Weapon->HitLocationTableName);			
	
	if (corpseDataPtr->WeaponMisfireFunction) {
		if (marineStatusPointer->behaviourState==MBS_Firing) {
			corpseDataPtr->DeathFiring=1;
		} else {
			corpseDataPtr->DeathFiring=0;
		}
	} else {
		corpseDataPtr->DeathFiring=0;
	}

	/* Remember wounds, or not? */
	Splice_HModels(&corpseDataPtr->HModelController,marineStatusPointer->HModelController.section_data);
	
	if (this_death->Template) {
		/* Convert to template. */
		Transmogrify_HModels(sbPtr,&corpseDataPtr->HModelController,
			corpseDataPtr->TemplateRoot, 1, 0,0);
	}
	/* Pass over some sounds? */

	/* Heh... now bin the old data block! */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);
	}
	if(marineStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* soundHandle2 is the voice! */
		corpseDataPtr->SoundHandle2=marineStatusPointer->soundHandle2;
		ActiveSounds[marineStatusPointer->soundHandle2].externalRef=&corpseDataPtr->SoundHandle2;
	}
	Dispel_HModel(&marineStatusPointer->HModelController);
	DeallocateMem(sbPtr->SBdataptr);
	/* Turn into the corpse. */
	sbPtr->SBdataptr=corpseDataPtr;
	sbPtr->I_SBtype=I_BehaviourNetCorpse;

 	SetCorpseAnimSequence_Core(sbPtr,this_death->Sequence_Type,this_death->Sub_Sequence,
 		this_death->Sequence_Length,this_death->TweeningTime);
	
	if (sbPtr->SBdptr) {
		/* Swap controllers round. */
		sbPtr->SBdptr->HModelControlBlock=&corpseDataPtr->HModelController;
		ProveHModel(&corpseDataPtr->HModelController,sbPtr->SBdptr);
	} else {
		ProveHModel_Far(&corpseDataPtr->HModelController,sbPtr);
	}

	corpseDataPtr->timer=MARINE_DYINGTIME;
	corpseDataPtr->validityTimer=CORPSE_VALIDITY_TIME;
	corpseDataPtr->HModelController.Looped=0;
	corpseDataPtr->HModelController.LoopAfterTweening=0;

	/* stop motion */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->Friction	= 400000;
	sbPtr->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->LinVelocity.vx;
	sbPtr->DynPtr->LinImpulse.vy+=sbPtr->DynPtr->LinVelocity.vy;
	sbPtr->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->LinVelocity.vz;
	sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
	sbPtr->DynPtr->CanClimbStairs = 0;

	/* KJL 17:19:35 27/08/98 - ignore the player, other body parts, etc */
	sbPtr->DynPtr->OnlyCollideWithEnvironment = 1;

	/* Electric death sound? */
	if (corpseDataPtr->This_Death->Electrical) {
		Sound_Play(SID_ED_ELEC_DEATH,"de",&sbPtr->DynPtr->Position,&corpseDataPtr->SoundHandle4);
	}

}

void Convert_Xenoborg_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death) {

	NETCORPSEDATABLOCK *corpseDataPtr;
	XENO_STATUS_BLOCK *xenoStatusPointer;
		
	xenoStatusPointer=(XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	/* Convert an xenoborg... to a corpse. */

    GLOBALASSERT(sbPtr);
	GLOBALASSERT(this_death);
	GLOBALASSERT(xenoStatusPointer);

	corpseDataPtr = (void *)AllocateMem(sizeof(NETCORPSEDATABLOCK));
	GLOBALASSERT(corpseDataPtr);
		
	/* Fill in corpseDataPtr... */
	corpseDataPtr->SoundHandle  = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle2 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle3 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle4 = SOUND_NOACTIVEINDEX;

	corpseDataPtr->Type=I_BehaviourXenoborg;
	corpseDataPtr->GibbFactor=xenoStatusPointer->GibbFactor;
	corpseDataPtr->This_Death=this_death;

	corpseDataPtr->CloakStatus = PCLOAK_Off;
	corpseDataPtr->CloakTimer = 0;
	corpseDataPtr->destructTimer = -1;
	corpseDataPtr->WeaponMisfireFunction=NULL;
	corpseDataPtr->My_Gunflash_Section=NULL;
	corpseDataPtr->weapon_variable=0;
	corpseDataPtr->Android=0;
	corpseDataPtr->ARealMarine=0;
	corpseDataPtr->TemplateRoot=NULL;
	corpseDataPtr->My_Weapon=NULL;
	corpseDataPtr->DeathFiring=0;
	corpseDataPtr->subtype = 0;
	corpseDataPtr->Wounds=0;

	corpseDataPtr->hltable=GetThisHitLocationTable("xenoborg");

	/* Remember wounds, or not? */
	Splice_HModels(&corpseDataPtr->HModelController,xenoStatusPointer->HModelController.section_data);
	
	/* Heh... now bin the old data block! */
	Dispel_HModel(&xenoStatusPointer->HModelController);
	DeallocateMem(sbPtr->SBdataptr);
	/* Turn into the corpse. */
	sbPtr->SBdataptr=corpseDataPtr;
	sbPtr->I_SBtype=I_BehaviourNetCorpse;

 	SetCorpseAnimSequence_Core(sbPtr,this_death->Sequence_Type,this_death->Sub_Sequence,
 		this_death->Sequence_Length,this_death->TweeningTime);
	
	if (sbPtr->SBdptr) {
		/* Swap controllers round. */
		sbPtr->SBdptr->HModelControlBlock=&corpseDataPtr->HModelController;
		ProveHModel(&corpseDataPtr->HModelController,sbPtr->SBdptr);
	} else {
		ProveHModel_Far(&corpseDataPtr->HModelController,sbPtr);
	}

	corpseDataPtr->timer=XENO_DYINGTIME;
	corpseDataPtr->validityTimer=CORPSE_VALIDITY_TIME;
	corpseDataPtr->HModelController.Looped=0;
	corpseDataPtr->HModelController.LoopAfterTweening=0;

	/* stop motion */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->Friction	= 400000;
	sbPtr->DynPtr->LinImpulse.vx+=sbPtr->DynPtr->LinVelocity.vx;
	sbPtr->DynPtr->LinImpulse.vy+=sbPtr->DynPtr->LinVelocity.vy;
	sbPtr->DynPtr->LinImpulse.vz+=sbPtr->DynPtr->LinVelocity.vz;
	sbPtr->DynPtr->LinVelocity.vx = sbPtr->DynPtr->LinVelocity.vy = sbPtr->DynPtr->LinVelocity.vz = 0;
	sbPtr->DynPtr->CanClimbStairs = 0;
	/* Experiment... */
	sbPtr->DynPtr->UseStandardGravity=1;
	sbPtr->DynPtr->Mass	= 160;
	/* Okay... */

	/* KJL 17:19:35 27/08/98 - ignore the player, other body parts, etc */
	sbPtr->DynPtr->OnlyCollideWithEnvironment = 1;

	#if 0
	/* Electric death sound? */
	if (corpseDataPtr->This_Death->Electrical) {
		Sound_Play(SID_ED_ELEC_DEATH,"de",&sbPtr->DynPtr->Position,&corpseDataPtr->SoundHandle4);
	}
	#endif

}

void CorpseBehaveFun(STRATEGYBLOCK *sbPtr)
{
	
	/* Just count down. */
	NETCORPSEDATABLOCK *corpseDataPtr;

    LOCALASSERT(sbPtr);
	
	corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(corpseDataPtr);


	if (corpseDataPtr->timer<=0)
	{
		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);

		if (playerStatusPtr->MyCorpse==sbPtr) {
			/* Set players corpse to null. */
			playerStatusPtr->MyCorpse=NULL;
		}

		if(corpseDataPtr->SoundHandle  != SOUND_NOACTIVEINDEX) Sound_Stop(corpseDataPtr->SoundHandle);
		if(corpseDataPtr->SoundHandle2 != SOUND_NOACTIVEINDEX) Sound_Stop(corpseDataPtr->SoundHandle2);
		if(corpseDataPtr->SoundHandle3 != SOUND_NOACTIVEINDEX) Sound_Stop(corpseDataPtr->SoundHandle3);
		if(corpseDataPtr->SoundHandle4 != SOUND_NOACTIVEINDEX) Sound_Stop(corpseDataPtr->SoundHandle4);

		/* Remove corpse. */
		DestroyAnyStrategyBlock(sbPtr);
		AddNetMsg_LocalObjectDestroyed(sbPtr);
		return;
	}
	else
	{
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;

		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = corpseDataPtr->timer/2;

			if (corpseDataPtr->Type==I_BehaviourXenoborg) {
				/* Particularly important for xenoborgs... optional for others? */
				if (dispPtr->ObFlags2<ONE_FIXED) {
					corpseDataPtr->HModelController.DisableBleeding=1;
				}
			}
		}

		/* Does the corpse that falls when not visible make no sound? */
		ProveHModel_Far(&corpseDataPtr->HModelController,sbPtr);
	}
	
	corpseDataPtr->timer-=NormalFrameTime;
	corpseDataPtr->validityTimer-=NormalFrameTime;
	
	/* May get decapitated whilst screaming... */
	if ((corpseDataPtr->Type==I_BehaviourMarine)
		||((corpseDataPtr->Type==I_BehaviourMarinePlayer)&&(AvP.PlayerType==I_Marine))) {

		SECTION_DATA *head;

		head=GetThisSectionData(corpseDataPtr->HModelController.section_data,"head");

		/* Is it still attached? */
		if (head) {
			if (head->flags&section_data_notreal) {
				head=NULL;
			}
		}

		if (head==NULL) {
			if(corpseDataPtr->SoundHandle2 != SOUND_NOACTIVEINDEX) {
				Sound_Stop(corpseDataPtr->SoundHandle2);
			}
		}
	}

	if ((corpseDataPtr->Type==I_BehaviourPredator)||
		((corpseDataPtr->Type==I_BehaviourMarinePlayer)&&(AvP.PlayerType==I_Predator))) {

		/* If we're a partially cloaked predator, continue to decloak. */
		if (corpseDataPtr->CloakStatus==PCLOAK_Off) {
			/* Do nothing. */
		} else if (corpseDataPtr->CloakStatus==PCLOAK_Activating) {
			/* Don't reset timer. */
			corpseDataPtr->CloakStatus = PCLOAK_Deactivating;
		} else if (corpseDataPtr->CloakStatus==PCLOAK_Deactivating) {
			/* Okay, okay! */
		} else {
			/* Cloak must be On. */
			corpseDataPtr->CloakStatus = PCLOAK_Deactivating;		
			corpseDataPtr->CloakTimer = 0; /* Was predStatus->PredShimmer. */
		}
		GLOBALASSERT((corpseDataPtr->CloakStatus==PCLOAK_Deactivating)||(corpseDataPtr->CloakStatus==PCLOAK_Off));
		/* Run the timer. */
		if (corpseDataPtr->CloakStatus==PCLOAK_Deactivating) {
			corpseDataPtr->CloakTimer += NormalFrameTime;
			if(corpseDataPtr->CloakTimer>=(ONE_FIXED))
			{
				corpseDataPtr->CloakTimer=0;
				corpseDataPtr->CloakStatus=PCLOAK_Off;
			}			
		}
	} else {
		GLOBALASSERT(corpseDataPtr->CloakStatus==PCLOAK_Off);
	}

	/* Marine specifics. */
	if (corpseDataPtr->Type==I_BehaviourMarine) {
		/* Did marine die with the trigger held down? */
		if (corpseDataPtr->DeathFiring) {
			/* Is there a gunflash? */
			if(corpseDataPtr->My_Gunflash_Section) {
				/* But is it still attached? */
				if (corpseDataPtr->My_Gunflash_Section->my_controller==&(corpseDataPtr->HModelController)) {
					/* Keep firing! */
					LOCALASSERT(corpseDataPtr->WeaponMisfireFunction);
					/* Shouldn't be doing this without knowing why. */
					(*corpseDataPtr->WeaponMisfireFunction)(corpseDataPtr->My_Gunflash_Section,&corpseDataPtr->weapon_variable);
				}
			}
		}
		
		/* Do we want to trim off the weapons? */
		
		if (corpseDataPtr->HModelController.keyframe_flags) {
		
			GLOBALASSERT(corpseDataPtr->TemplateRoot);

			TrimToTemplate(sbPtr,&corpseDataPtr->HModelController,corpseDataPtr->TemplateRoot, 1);
		}
	}

	/* Fire sound code. */
	if(corpseDataPtr->SoundHandle3!=SOUND_NOACTIVEINDEX) Sound_Update3d(corpseDataPtr->SoundHandle3,&(sbPtr->DynPtr->Position));
	if(corpseDataPtr->SoundHandle4!=SOUND_NOACTIVEINDEX) Sound_Update3d(corpseDataPtr->SoundHandle4,&(sbPtr->DynPtr->Position));
	
	if (sbPtr->SBDamageBlock.IsOnFire) {
		if (corpseDataPtr->SoundHandle3!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[corpseDataPtr->SoundHandle3].soundIndex!=SID_FIRE) {
				Sound_Stop(corpseDataPtr->SoundHandle3);
			 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&corpseDataPtr->SoundHandle3,127);
			}
		} else {
		 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&corpseDataPtr->SoundHandle3,127);
		}
	} else {
		if (corpseDataPtr->SoundHandle3!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(corpseDataPtr->SoundHandle3);
		}
	}
	
	#if CORPSE_SIGHTINGS
	Marine_CorpseSightingTest(sbPtr);
	#endif

	/* Finally consider destructing preds. */
	if (corpseDataPtr->destructTimer>=0) {
		corpseDataPtr->destructTimer-=NormalFrameTime;
		if (corpseDataPtr->destructTimer<=0) {
			StartPredatorSelfDestructExplosion(sbPtr);
			corpseDataPtr->GibbFactor=ONE_FIXED;
			corpseDataPtr->destructTimer=-1;
		}
	}
}

void SetCorpseAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime)
{

	NETCORPSEDATABLOCK *corpseStatus=(NETCORPSEDATABLOCK *)(sbPtr->SBdataptr);

	GLOBALASSERT(length!=0);

	if (tweeningtime<=0) {
		InitHModelSequence(&corpseStatus->HModelController,(int)type,subtype,length);
	} else {
		InitHModelTweening(&corpseStatus->HModelController, tweeningtime, (int)type,subtype,length,0);
	}

	corpseStatus->HModelController.Playing=1;
	/* Might be unset... */
}

void CorpseIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming) {

	NETCORPSEDATABLOCK *corpseDataPtr;
	int tkd,deathtype;

    LOCALASSERT(sbPtr);
	corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(corpseDataPtr);

	/* Set up gibb factor. */

	tkd=TotalKineticDamage(damage);
	deathtype=0;

	corpseDataPtr->Wounds|=wounds;

	switch(corpseDataPtr->Type) {
		case I_BehaviourMarinePlayer:
			{
				if (damage->ExplosivePower==1) {
					if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
						/* Okay, you can gibb now. */
						corpseDataPtr->GibbFactor=ONE_FIXED>>1;
						deathtype=2;
					}
				} else if ((tkd>60)&&((multiple>>16)>1)) {
					int newmult;
				
					newmult=DIV_FIXED(multiple,NormalFrameTime);
					if (MUL_FIXED(tkd,newmult)>(500)) {
						/* Loadsabullets! */
						corpseDataPtr->GibbFactor=-(ONE_FIXED>>2);
						deathtype=2;
					}
				}
				
				if ((damage->ExplosivePower==2)||(damage->ExplosivePower==6)) {
					/* Basically SADARS only. */
					corpseDataPtr->GibbFactor=ONE_FIXED;
					deathtype=3;
				}

				if (damage->ForceBoom) {
					deathtype+=damage->ForceBoom;
				}
			}
			break;
		case I_BehaviourAlien:
		case I_BehaviourAlienPlayer:
			{
				if (damage->ExplosivePower==1) {
				 	/* Explosion case. */
				 	if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
				 		/* Okay, you can gibb now. */
				 		corpseDataPtr->GibbFactor=ONE_FIXED>>1;
						deathtype=2;
				 	}
				} else if ((tkd<40)&&((multiple>>16)>1)) {
				 	int newmult;
				
				 	newmult=DIV_FIXED(multiple,NormalFrameTime);
				 	if (MUL_FIXED(tkd,newmult)>700) {
				 		/* Excessive bullets case 1. */
				 		corpseDataPtr->GibbFactor=ONE_FIXED>>2;
						deathtype=2;
				 	} else if (MUL_FIXED(tkd,newmult)>250) {
				 		/* Excessive bullets case 2. */
				 		corpseDataPtr->GibbFactor=ONE_FIXED>>3;
						deathtype=1;
				 	}
				}
				
				/* Predaliens and preatorians only gibb for sadars. */
				if (corpseDataPtr->subtype!=0) {
					corpseDataPtr->GibbFactor=0;
					/* But retain deathtype. */
				}
				
				if ((damage->ExplosivePower==2)||(damage->ExplosivePower==6)) {
					/* Basically SADARS only. */
					if (corpseDataPtr->Type==AT_Standard) {
						corpseDataPtr->GibbFactor=ONE_FIXED;
					} else {
						corpseDataPtr->GibbFactor=ONE_FIXED>>2;
					}
					deathtype=3;
				}
				
				if (damage->ForceBoom) {
					deathtype+=damage->ForceBoom;
				}
				/* No additional gibbing for flamethrowers. */

				if (damage->Id==AMMO_PREDPISTOL_STRIKE) {
					/* Blow up if hit by the bolt? */
					corpseDataPtr->GibbFactor=ONE_FIXED>>3;
				} else if (damage->Id==AMMO_PRED_PISTOL) {
					/* Unfortunately, that can't happen.  Try this test instead. */
					if (multiple>(43253)) {
						/* Must be pretty close. */
						corpseDataPtr->GibbFactor=ONE_FIXED>>3;
					}
				} else if (damage->Id==AMMO_FLECHETTE_POSTMAX) {
					corpseDataPtr->GibbFactor=ONE_FIXED>>2;
				}
			}
			break;
		case I_BehaviourPredator:
		case I_BehaviourXenoborg:
			/* Predator and Xenoborg 'splatting' currently ommitted... */
			break;
		default:
			break;
	}

}







/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct corpse_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int timer;
	int validityTimer;
	
	AVP_BEHAVIOUR_TYPE Type;
	int hltable_index;
	int GibbFactor;
	
	/* If you're a predator... */
	PRED_CLOAKSTATE CloakStatus;
	int CloakTimer;
	int destructTimer;
	/* If you're a marine... */
	int weapon_id;
	int weapon_variable;
	int Android;
	int ARealMarine;
	/* If you're an alien... */
	int subtype;

	int Wounds;

	int DeathFiring	:1;

	
	int deathCode;

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}CORPSE_SAVE_BLOCK;



//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV corpseDataPtr


void LoadStrategy_Corpse(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	extern HITLOCATIONTABLE Global_Hitlocation_Tables[];

	STRATEGYBLOCK* sbPtr;
	NETCORPSEDATABLOCK* corpseDataPtr;
	CORPSE_SAVE_BLOCK* block = (CORPSE_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//see if there is a living version of this corpse
	sbPtr = FindSBWithName(header->SBname);

	if(sbPtr)
	{
		//make sure the strategy found is some type of creature
		if(sbPtr->I_SBtype != I_BehaviourAlien &&
		   sbPtr->I_SBtype != I_BehaviourMarine &&
		   sbPtr->I_SBtype != I_BehaviourPredator &&
		   sbPtr->I_SBtype != I_BehaviourDormantPredator &&
		   sbPtr->I_SBtype != I_BehaviourXenoborg &&
		   sbPtr->I_SBtype != I_BehaviourFaceHugger)
		{
			return;
		}
		//get rid of it then
		DestroyAnyStrategyBlock(sbPtr);

		sbPtr = NULL;
	}

	//now we need to create a corpse from scratch
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr)
	{
		GLOBALASSERT(0=="Run out of strategy blocks");
		return;
	}
	InitialiseSBValues(sbPtr);
	corpseDataPtr = (void *)AllocateMem(sizeof(NETCORPSEDATABLOCK));

	//fill in some default values
	memset(corpseDataPtr,0,sizeof(*corpseDataPtr));

	corpseDataPtr->SoundHandle  = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle2 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle3 = SOUND_NOACTIVEINDEX;
	corpseDataPtr->SoundHandle4 = SOUND_NOACTIVEINDEX;
	
	sbPtr->SBdataptr=corpseDataPtr;
	sbPtr->I_SBtype=I_BehaviourNetCorpse;
	COPY_NAME(sbPtr->SBname,block->header.SBname);
	sbPtr->shapeIndex = 0;
	sbPtr->maintainVisibility = 1;

	//get a dynamics block
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);


	//start copying stuff
	
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(validityTimer)
	COPYELEMENT_LOAD(Type)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(CloakStatus)
	COPYELEMENT_LOAD(CloakTimer)
	COPYELEMENT_LOAD(destructTimer)
	COPYELEMENT_LOAD(weapon_variable)
	COPYELEMENT_LOAD(Android)
	COPYELEMENT_LOAD(ARealMarine)
	COPYELEMENT_LOAD(subtype)
	COPYELEMENT_LOAD(Wounds)
	COPYELEMENT_LOAD(DeathFiring)

					 
	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//hit location table
	if(block->hltable_index>=0)
	{
		corpseDataPtr->hltable = &Global_Hitlocation_Tables[block->hltable_index];
	}
	else
	{
		corpseDataPtr->hltable = NULL;
	}

	// get death_data
	corpseDataPtr->This_Death = GetThisDeath_FromUniqueCode(block->deathCode);

	//get marine's weapon (if a marine)
	corpseDataPtr->My_Weapon = GetThisNPCMarineWeapon(block->weapon_id);
	if(corpseDataPtr->My_Weapon)
	{
		corpseDataPtr->WeaponMisfireFunction=corpseDataPtr->My_Weapon->WeaponMisfireFunction;
		corpseDataPtr->My_Gunflash_Section=corpseDataPtr->My_Gunflash_Section;
		corpseDataPtr->TemplateRoot=GetNamedHierarchyFromLibrary(corpseDataPtr->My_Weapon->Riffname,corpseDataPtr->My_Weapon->TemplateName);
	}
	

	//load the corpse's hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&corpseDataPtr->HModelController);
		}
	}
	
	Load_SoundState(&corpseDataPtr->SoundHandle);
	Load_SoundState(&corpseDataPtr->SoundHandle2);
	Load_SoundState(&corpseDataPtr->SoundHandle3);
	Load_SoundState(&corpseDataPtr->SoundHandle4);

}


void SaveStrategy_Corpse(STRATEGYBLOCK* sbPtr)
{
	CORPSE_SAVE_BLOCK *block;
	NETCORPSEDATABLOCK* corpseDataPtr;
	
	corpseDataPtr = (NETCORPSEDATABLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(validityTimer)
	COPYELEMENT_SAVE(Type)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(CloakStatus)
	COPYELEMENT_SAVE(CloakTimer)
	COPYELEMENT_SAVE(destructTimer)
	COPYELEMENT_SAVE(weapon_variable)
	COPYELEMENT_SAVE(Android)
	COPYELEMENT_SAVE(ARealMarine)
	COPYELEMENT_SAVE(subtype)
	COPYELEMENT_SAVE(Wounds)
	COPYELEMENT_SAVE(DeathFiring)

 
	//hit location table
	if(corpseDataPtr->hltable)
	{
		block->hltable_index = corpseDataPtr->hltable->index;
	}
	else
	{
		block->hltable_index = -1;
	}

	//save death code
	if(corpseDataPtr->This_Death)
	{
		block->deathCode = corpseDataPtr->This_Death->Unique_Code;
	}
	else
	{
		block->deathCode = -1;
	}

	//save marine's weapon
	if(corpseDataPtr->My_Weapon) 
		block->weapon_id = corpseDataPtr->My_Weapon->id;
	else 
		block->weapon_id = -1;
	

	//save strategy block stuff

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the  hierarchy
	SaveHierarchy(&corpseDataPtr->HModelController);

	Save_SoundState(&corpseDataPtr->SoundHandle);
	Save_SoundState(&corpseDataPtr->SoundHandle2);
	Save_SoundState(&corpseDataPtr->SoundHandle3);
	Save_SoundState(&corpseDataPtr->SoundHandle4);
}
