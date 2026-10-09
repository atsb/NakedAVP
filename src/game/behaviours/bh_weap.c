#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"

#include "bh_types.h"
#include "comp_shp.h"
#include "inventry.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "bh_weap.h"
#include "bh_debri.h"
#include "weapons.h"
#include "dynblock.h"
#include "dynamics.h"
#include "lighting.h"
#include "bh_pred.h"
#include "bh_alien.h"
#include "bh_marin.h"
#include "bh_dummy.h"
#include "bh_rubberduck.h"
#include "pvisible.h"
#include "pheromon.h"
#include "psnd.h"
#include "psndplat.h"
#include "huddefs.h"
#include "ai_sight.h"
#include "targeting.h"
#include "game_statistics.h"

#include "particle.h"
#include "sfx.h"
#include "showcmds.h"
#include "savegame.h"
#include "los.h"
#include "detaillevels.h"

/* for win95 net game support */
#include "pldghost.h"
#include "pldnet.h"

#define FLAMETHROWER_PARTICLES_PER_FRAME (MUL_FIXED(120,NormalFrameTime))
#define PREDPISTOLFLECHETTES_PARTICLES_PER_FRAME (MUL_FIXED(50,NormalFrameTime))
#define TIME_FOR_PREDPISTOLFLECHETTE	(ONE_FIXED/50)
#define TIME_FOR_FLAMETHROWER_PARTICLE	(ONE_FIXED/120)
#define NEAR_WEAPON_FUDGE 1
#define PREDPISTOL_SPREAD	(4)

#define NEW_PREDPISTOL_BOLT	1
#define FRISBEE_SPEED 20000
/* Was 10000. */
#define ENERGY_BOLT_SPEED 65536
		 
/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
static void InitialiseRocketBehaviour(void);
static void InitialiseGrenadeBehaviour(AVP_BEHAVIOUR_TYPE behaviourID);
static STRATEGYBLOCK* InitialisePulseGrenadeBehaviour(void);
static void InitialisePPPlasmaBoltBehaviour(void);
static void InitialiseSpeargunBoltBehaviour(void);
void InitialiseEnergyBoltBehaviour(DAMAGE_PROFILE *damage, int factor);
static void InitialiseFlameThrowerBehaviour(void);
void InitialiseDiscBehaviour(STRATEGYBLOCK *target,SECTION_DATA *disc_section);
static void InitialiseAlienSpitBehaviour(void);
STRATEGYBLOCK* InitialiseEnergyBoltBehaviourKernel(VECTORCH *position,MATRIXCH *orient, int player, DAMAGE_PROFILE *damage, int factor);
static void InitialiseFrisbeeBehaviour(void);
STRATEGYBLOCK* InitialiseFrisbeeBoltBehaviourKernel(VECTORCH *position,MATRIXCH *orient, int player, DAMAGE_PROFILE *damage, int factor);

static void GetGunDirection(VECTORCH *gunDirectionPtr, VECTORCH *positionPtr);
void PredDisc_GetFirstTarget(PC_PRED_DISC_BEHAV_BLOCK *bptr, DISPLAYBLOCK *target, VECTORCH *position);
int PredDisc_TargetFilter(STRATEGYBLOCK *candidate);
void EulerAnglesHoming(VECTORCH *source, VECTORCH *Target, EULER *eulr, int rate);
void SetEulerAngles(VECTORCH *source, VECTORCH *Target, EULER *eulr);
STRATEGYBLOCK *PredDisc_GetNewTarget(PC_PRED_DISC_BEHAV_BLOCK *bptr,VECTORCH *discpos, STRATEGYBLOCK *prevtarg, int mine);
int ObjectIsOnScreen(DISPLAYBLOCK *object);
void Frisbee_Hit_Environment(STRATEGYBLOCK *sbPtr,COLLISIONREPORT *reportPtr);
void Crunch_Position_For_Players_Weapon(VECTORCH *position);
static int SBForcesBounce(STRATEGYBLOCK *sbPtr);
static int Reflect(VECTORCH *Incident, VECTORCH *Normal, EULER *Output);

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
extern int NormalFrameTime;
extern int ProjectilesFired;
extern SECTION_DATA *PWMFSDP;
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern int AccuracyStats_TargetFilter(STRATEGYBLOCK *sbPtr);
extern BOOL CalculateFiringSolution(VECTORCH* firing_pos,VECTORCH* target_pos,VECTORCH* target_vel,int projectile_speed,VECTORCH* solution);

int mx=0;
int my=-2000;
int mz=12000;

extern int NumberOfFlaresActive;
int PredPistolBoltSpeed=32767;
int PredPistolBoltGravity=80000;

int Caster_BlastRadius=5000;

SOUND3DDATA Explosion_SoundData={
	{0,0,0,},
	{0,0,0,},
	15000,
	150000,
};

SOUND3DDATA PredPistolExplosion_SoundData={
	{0,0,0,},
	{0,0,0,},
	15000,
	100000,
};

/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/
										   
void FireProjectileAmmo(enum AMMO_ID AmmoID)
{
	switch (AmmoID)
	{
		case AMMO_GRENADE:
		{
			switch(GrenadeLauncherData.SelectedAmmo)
			{
				case AMMO_GRENADE:
				{
					InitialiseGrenadeBehaviour(I_BehaviourGrenade);
					break;
				}
				case AMMO_FLARE_GRENADE:
				{
		   			InitialiseGrenadeBehaviour(I_BehaviourFlareGrenade);
					break;
				}
				case AMMO_FRAGMENTATION_GRENADE:
				{
			    	InitialiseGrenadeBehaviour(I_BehaviourClusterGrenade);
					break;
				}
				case AMMO_PROXIMITY_GRENADE:
				{
			 	    InitialiseGrenadeBehaviour(I_BehaviourProximityGrenade);
					break;
				}
				default:
				{
					/* KJL 10:36:25 04/21/97 - data error if you got here */
					LOCALASSERT(0);
					break;
				}
			}
			break;
		}
		case AMMO_PULSE_GRENADE:
		{
			InitialisePulseGrenadeBehaviour();
			break;
		}
		case AMMO_SADAR_TOW:
		{
			InitialiseRocketBehaviour();
			break;
		}
		case AMMO_FRISBEE:
		{
			InitialiseFrisbeeBehaviour();
			break;
		}
		
		case AMMO_PRED_RIFLE:
		{
			InitialiseSpeargunBoltBehaviour();
			break;
		}

		case AMMO_PRED_PISTOL:
		{
			InitialisePPPlasmaBoltBehaviour();
			break;
		}

		case AMMO_PRED_ENERGY_BOLT:
		{
			InitialiseEnergyBoltBehaviour(&TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty],65536);
			break;
		}

		case AMMO_PRED_DISC:
		{
			break;
		}
	
	    case AMMO_FLAMETHROWER:
	    {
			InitialiseFlameThrowerBehaviour();
			break;
	    }

		case AMMO_ALIEN_SPIT:
		{
			InitialiseAlienSpitBehaviour();
			break;
		}
		default:
			break;
	}
}

/* CDF 12/7/99 Smart Frisbee for expansion pack? */

int FrisbeeSight_FrustrumReject(STRATEGYBLOCK *sbPtr,VECTORCH *localOffset,STRATEGYBLOCK *target) {

	FRISBEE_BEHAV_BLOCK *frisbeeStatusPointer;
	VECTORCH fixed_offset;

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	frisbeeStatusPointer = (FRISBEE_BEHAV_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(frisbeeStatusPointer);	          		

	#if 0
	PrintDebuggingText("Local Offset: %d %d %d\n",localOffset->vx,localOffset->vy,localOffset->vz);
	#endif

	fixed_offset=*localOffset;
	
	#if 0
	if ((fixed_offset.vx <0) && (
		((fixed_offset.vy) < (-fixed_offset.vx))&&(fixed_offset.vy>=0))
 		||((fixed_offset.vy<0)&&((-fixed_offset.vy) < (-fixed_offset.vx))
 		)&&(
		((fixed_offset.vz) < (-fixed_offset.vx))&&(fixed_offset.vz>=0))
 		||((fixed_offset.vz<0)&&((-fixed_offset.vz) < (-fixed_offset.vx))
 		)) {
		/* 90 horizontal, 90 vertical? */
	#else
	if (((fixed_offset.vx <0) && (
		((fixed_offset.vy) < (-fixed_offset.vx))&&(fixed_offset.vy>=0)))
 		|| (((fixed_offset.vy<0)&&((-fixed_offset.vy) < (-fixed_offset.vx))
 		)&&((fixed_offset.vz>0))
 		)) {
		/* 90 horizontal, 90 vertical? */
	#endif
		return(1);
	} else {
		return(0);
	}
}

int Frisbee_TargetFilter(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourMarinePlayer:
		case I_BehaviourDummy:
			{
				switch (AvP.PlayerType) {
					default:
					case I_Marine:
						return(0);
						break;
					case I_Predator:
					case I_Alien:
						return(1);
						break;
				}
				return(0);
			}
			break;
		case I_BehaviourAlien:
			{
				ALIEN_STATUS_BLOCK *alienStatusPointer;
				LOCALASSERT(candidate);	
				LOCALASSERT(candidate->DynPtr);	
			
				alienStatusPointer=(ALIEN_STATUS_BLOCK *)(candidate->SBdataptr);    
				
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					if ((alienStatusPointer->BehaviourState==ABS_Dormant)||
						(alienStatusPointer->BehaviourState==ABS_Awakening)) {
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
		case I_BehaviourXenoborg:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
			/* Valid. */
			return(1);
			break;
		case I_BehaviourMarine:
			return(0);
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourMarinePlayer:
						switch (netGameData.gameType) {
							case NGT_Individual:
								return(1);
								break;
							case NGT_CoopDeathmatch:
								return(0);
								break;
							case NGT_LastManStanding:
								return(0);
								break;
							case NGT_PredatorTag:
								return(1);
								break;
							case NGT_Coop:
								return(0);
								break;
							case NGT_AlienTag:
								return(1); //However, there shouldn't be more than one alien in alien tag anyway.
								break;
							default:
								return(0);
								break;
						}
						break;
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
					case I_BehaviourAlien:
						return(1);
						break;
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

/*
Function only does minimal frisbee setup , the rest will be done by the load function
*/
static STRATEGYBLOCK* InitialiseFrisbeeBehaviour_ForLoad() {
	VECTORCH zeroVect = {0,0,0};

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
  	FRISBEE_BEHAV_BLOCK *bblk;
		
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourFrisbee,&zeroVect);

	if (dispPtr == 0) return NULL;		 // Failed to allocate display block

	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		
 	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
 	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;


	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr = AllocateMem(sizeof(FRISBEE_BEHAV_BLOCK));
	bblk = dispPtr->ObStrategyBlock->SBdataptr;

	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	memset(dispPtr->ObStrategyBlock->SBdataptr,0,sizeof(FRISBEE_BEHAV_BLOCK));

	bblk->soundHandle = SOUND_NOACTIVEINDEX;

	bblk->Laser.SourcePosition=zeroVect;
	bblk->Laser.SourcePosition=zeroVect;
	bblk->Laser.BeamHasHitPlayer=0;
	bblk->Laser.BeamIsOn=0;

	dispPtr->HModelControlBlock=&bblk->HModelController;
	

	return dispPtr->ObStrategyBlock;
}

STRATEGYBLOCK* CreateFrisbeeKernel(VECTORCH *position, MATRIXCH *orient, int fromplayer) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	FRISBEE_BEHAV_BLOCK *fblk;

	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourFrisbee,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block

	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_ROCKETJET);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);

	if (fromplayer==0) {
		dynPtr->IgnoreThePlayer=0;
	}
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(FRISBEE_BEHAV_BLOCK));
	
	fblk=((FRISBEE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr);
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
		
	fblk->counter = (5*ONE_FIXED);
	fblk->soundHandle = SOUND_NOACTIVEINDEX;
	fblk->Bounced = 0;
	fblk->bounces = 0;

	fblk->Laser.SourcePosition=*position;
	fblk->Laser.SourcePosition=*position;
	fblk->Laser.BeamHasHitPlayer=0;
	fblk->Laser.BeamIsOn=0;
			
	/* Create HModel. */
	{
		SECTION *root_section;

		root_section=GetNamedHierarchyFromLibrary("mdisk","Mdisk");
				
		GLOBALASSERT(root_section);

		Create_HModel(&fblk->HModelController,root_section);
		InitHModelSequence(&fblk->HModelController,HMSQT_MarineStand,MSSS_Minigun_Delta,(ONE_FIXED>>1));
		
		ProveHModel(&fblk->HModelController,dispPtr);
		fblk->HModelController.Looped=1;

		dispPtr->HModelControlBlock=&fblk->HModelController;

		#if 0
		SECTION_DATA *local_disc;
		/* Match disks. */
		local_disc=GetThisSectionData(bblk->HModelController.section_data,"disk");
		local_disc->World_Offset=disc_section->World_Offset;
		local_disc->SecMat=disc_section->SecMat;
		InitHModelTweening(&bblk->HModelController,(ONE_FIXED>>2),HMSQT_MarineStand,MSSS_Minigun_Delta,ONE_FIXED,1);
		#endif
	}

	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;
	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;

	dynPtr->IgnoreThePlayer=1;

	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, FRISBEE_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, FRISBEE_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, FRISBEE_SPEED);

	#if 0
	if (fromplayer==1) {
		/* Add player velocity? */
		dynPtr->LinVelocity.vx+=Player->ObStrategyBlock->DynPtr->LinVelocity.vx;
		dynPtr->LinVelocity.vy+=Player->ObStrategyBlock->DynPtr->LinVelocity.vy;
		dynPtr->LinVelocity.vz+=Player->ObStrategyBlock->DynPtr->LinVelocity.vz;
	}
	#endif

	/* for net game support */
	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	return dispPtr->ObStrategyBlock; 

}

extern void FrisbeeBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    FRISBEE_BEHAV_BLOCK *fbPtr = (FRISBEE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int explodeNow;	

	MODULE *dmod;
	
	dmod=ModuleFromPosition(&sbPtr->DynPtr->Position,playerPherModule);

	//MakeRocketTrailParticles(&(dynPtr->PrevPosition), &(dynPtr->Position));
	explodeNow=0;
	
	if (fbPtr->counter<=0)
	{
		explodeNow=1;
	}
	else
	{        
		if (reportPtr)
		{
			
			/* Cut from disc code.  Don't care about whether it was the player... */
			{

				/* Hit a random strategyblock - what is it? */
				if (SBForcesBounce(reportPtr->ObstacleSBPtr)) {

					MATRIXCH mat;
					/* Bounce. */
					Reflect(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal, &dynPtr->OrientEuler);
					dynPtr->OrientEuler.EulerZ=0;
					dynPtr->IgnoreThePlayer=0;
					MakeImpactSparks(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal,&dynPtr->Position);
					Sound_Play(SID_ED_SKEETERDISC_HITWALL,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
					fbPtr->bounces++;
					/*
					Record that the disc has bounced - for use in network game
					*/
					fbPtr->Bounced=1;

					CreateEulerMatrix(&dynPtr->OrientEuler, &mat);
					TransposeMatrixCH(&mat);

					dynPtr->OrientMat=mat;

					dynPtr->LinVelocity.vx = MUL_FIXED(mat.mat31,FRISBEE_SPEED);
					dynPtr->LinVelocity.vy = MUL_FIXED(mat.mat32,FRISBEE_SPEED);
					dynPtr->LinVelocity.vz = MUL_FIXED(mat.mat33,FRISBEE_SPEED);

					dynPtr->LinImpulse.vx=0;
					dynPtr->LinImpulse.vy=0;
					dynPtr->LinImpulse.vz=0;

			
				} else if (SBIsEnvironment(reportPtr->ObstacleSBPtr)) {
					Frisbee_Hit_Environment(sbPtr,reportPtr);
				} else {
					/* Hit a creature? */
					VECTORCH attack_dir;
				
					/* Accuracy snipped. */
					GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
					CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_FRISBEE].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
					explodeNow=1;
				}
			}
		}
	}

	if (explodeNow) {
		//NewOnScreenMessage("Frisbee Exploded.");

        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_FRISBEE_BLAST].MaxRange,
			&TemplateAmmo[AMMO_FRISBEE_BLAST].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_FRISBEE_BLAST].ExplosionIsFlat
		);
		
		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
			Sound_Play(SID_NICE_EXPLOSION,"n",&Explosion_SoundData);
    	}
 			
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	{
			AddNetMsg_LocalObjectDestroyed(sbPtr);
 			AddNetMsg_SpotOtherSound(SID_NICE_EXPLOSION,&dynPtr->Position,1);
		}

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);

    }
	else
	{
		VECTORCH line;
		SECTION_DATA *disc_section;

		/* We must be flying.  Maintain sound. */
		if(fbPtr->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Update3d(fbPtr->soundHandle,&(sbPtr->DynPtr->Position));
			if (ActiveSounds[fbPtr->soundHandle].soundIndex!=SID_PREDATOR_DISK_FLYING) {
				Sound_Stop(fbPtr->soundHandle);
			 	Sound_Play(SID_ED_SKEETERDISC_SPIN,"del",&(sbPtr->DynPtr->Position),&fbPtr->soundHandle);
			}
		} else {
		 	Sound_Play(SID_ED_SKEETERDISC_SPIN,"del",&(sbPtr->DynPtr->Position),&fbPtr->soundHandle);
		}

		disc_section=GetThisSectionData(fbPtr->HModelController.section_data,"Mdisk");
		
		if (disc_section) {
			//ReleasePrintDebuggingText("Disc section found!\n");
			line.vx=disc_section->SecMat.mat11;
			line.vy=disc_section->SecMat.mat12;
			line.vz=disc_section->SecMat.mat13;

			//MakeParticle(&dynPtr->Position,&line,PARTICLE_PREDATOR_BLOOD);
			/* Make a laser? */
			// KJL 12:32:40 08/02/00 - Fox want the laser beam removed
			fbPtr->Laser.BeamIsOn=0;
			{
				int l;
				for (l=0;l<4;l++)
					MakeFlareParticle(dynPtr);
			}	
			/* Now do the target sweep. */
			if (fbPtr->counter<(4*(ONE_FIXED)))
			{
				int a;
				STRATEGYBLOCK *candidate,*nearest;
				VECTORCH offset;
				/*
				MODULE *dmod;
				dmod=ModuleFromPosition(&sbPtr->DynPtr->Position,playerPherModule);
				LOCALASSERT(dmod);
				*/
				nearest=NULL;

				for (a=0; a<NumActiveStBlocks; a++)
			 	{
					candidate=ActiveStBlockList[a];

					if (candidate!=sbPtr)
					{
						if (candidate->DynPtr)
						{
							if (Frisbee_TargetFilter(candidate))
							{
								/* Check visibility? */
								if ((candidate->SBdptr)&&(sbPtr->SBdptr))
								{
									/* Near case. */
									if ((!NPC_IsDead(candidate))
										||(candidate->I_SBtype==I_BehaviourMarinePlayer)
										||(candidate->I_SBtype==I_BehaviourDummy))
									{
										if (NPCCanSeeTarget(sbPtr, candidate, ONE_FIXED))
										{
											VECTORCH targetPos;
											nearest=candidate;

											/* Shoot it, */
											GetTargetingPointOfObject_Far(nearest,&targetPos);
								
											if (CalculateFiringSolution(&sbPtr->DynPtr->Position,&nearest->DynPtr->Position,
												&nearest->DynPtr->LinVelocity,ENERGY_BOLT_SPEED,&offset)) {
												/* Use this. */
											} else {
												offset.vx=targetPos.vx-sbPtr->DynPtr->Position.vx;
												offset.vy=targetPos.vy-sbPtr->DynPtr->Position.vy;
												offset.vz=targetPos.vz-sbPtr->DynPtr->Position.vz;
												Normalise(&offset);
											}

											{
												MATRIXCH mat;
												MatrixFromZVector(&offset,&mat);
												InitialiseFrisbeeBoltBehaviourKernel(&sbPtr->DynPtr->Position,&mat, 0, 
													&TemplateAmmo[AMMO_FRISBEE].MaxDamage[AvP.Difficulty], 65536);
												
											}
										}	
									}
								}
								else
								{
									if ((!NPC_IsDead(candidate))
										||(candidate->I_SBtype==I_BehaviourMarinePlayer)
										||(candidate->I_SBtype==I_BehaviourDummy))
									{
										
										if (dmod)
										{
											if ((IsModuleVisibleFromModule(dmod,candidate->containingModule)))
											{
												VECTORCH targetPos;
												nearest=candidate;

												/* Shoot it, */
												GetTargetingPointOfObject_Far(nearest,&targetPos);
									
												if (CalculateFiringSolution(&sbPtr->DynPtr->Position,&nearest->DynPtr->Position,
													&nearest->DynPtr->LinVelocity,ENERGY_BOLT_SPEED,&offset)) {
													/* Use this. */
												} else {
													offset.vx=targetPos.vx-sbPtr->DynPtr->Position.vx;
													offset.vy=targetPos.vy-sbPtr->DynPtr->Position.vy;
													offset.vz=targetPos.vz-sbPtr->DynPtr->Position.vz;
													Normalise(&offset);
												}

												{
													MATRIXCH mat;
													MatrixFromZVector(&offset,&mat);
													InitialiseFrisbeeBoltBehaviourKernel(&sbPtr->DynPtr->Position,&mat, 0, 
														&TemplateAmmo[AMMO_FRISBEE].MaxDamage[AvP.Difficulty], 65536);
												}
											}
										}
									}
								}
							}
						}
					}
				}
				if (nearest)
				{
					/* And explode. */

					HandleEffectsOfExplosion
					(
						sbPtr,
						&(dynPtr->Position),
						TemplateAmmo[AMMO_FRISBEE_FIRE].MaxRange,
						&TemplateAmmo[AMMO_FRISBEE_FIRE].MaxDamage[AvP.Difficulty],
						TemplateAmmo[AMMO_FRISBEE_FIRE].ExplosionIsFlat
					);
					
					if (sbPtr->containingModule) {
						Explosion_SoundData.position=dynPtr->Position;
						Sound_Play(SID_ED_SKEETERPLASMAFIRE,"n",&Explosion_SoundData);
			    	}
			 			
					/* for net game support: send a message saying we've blown up... */
					if(AvP.Network != I_No_Network)	{
						AddNetMsg_LocalObjectDestroyed(sbPtr);
						AddNetMsg_SpotOtherSound(SID_ED_SKEETERPLASMAFIRE,&dynPtr->Position,1);
					}

					/* destroy rocket */
			    	DestroyAnyStrategyBlock(sbPtr);
				}
			}
		} else {
			//ReleasePrintDebuggingText("No disc section found!\n");
		}
		fbPtr->counter-=NormalFrameTime;
	}
}


/* KJL 14:08:18 02/21/97 - New rocket & grenade functions */

STRATEGYBLOCK* CreateRocketKernel(VECTORCH *position, MATRIXCH *orient, int fromplayer) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;

	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourRocket,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block

	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_ROCKETJET);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);

	if (fromplayer==0) {
		dynPtr->IgnoreThePlayer=0;
	}
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PREDPISTOL_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
		
	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = fromplayer;
			
	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;
	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    #define MISSILE_SPEED 80000
    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, MISSILE_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, MISSILE_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, MISSILE_SPEED);

	/* for net game support */
	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	return dispPtr->ObStrategyBlock; 

}

static void InitialiseFrisbeeBehaviour(void)
{
	VECTORCH position;
	
	/* calculate the position */
	{
		extern VECTORCH CentreOfMuzzleOffset;
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
 		position = CentreOfMuzzleOffset;
		
		RotateVector(&position,&PlayersWeapon.ObMat);
	
	 	position.vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
	 	position.vx = position.vx/4 + VDBPtr->VDB_World.vx;

	 	position.vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
		position.vy = position.vy/4 + VDBPtr->VDB_World.vy;
		
	 	position.vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
		position.vz = position.vz/4 + VDBPtr->VDB_World.vz;
  	}

	CreateFrisbeeKernel(&position,&PlayersWeapon.ObMat,1);
}

static void InitialiseRocketBehaviour(void)
{
	VECTORCH position;
	
	/* calculate the position */
	{
		extern VECTORCH CentreOfMuzzleOffset;
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
 		position = CentreOfMuzzleOffset;
		
		RotateVector(&position,&PlayersWeapon.ObMat);
	
	 	position.vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
	 	position.vx = position.vx/4 + VDBPtr->VDB_World.vx;

	 	position.vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
		position.vy = position.vy/4 + VDBPtr->VDB_World.vy;
		
	 	position.vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
		position.vz = position.vz/4 + VDBPtr->VDB_World.vz;
  	}

	CreateRocketKernel(&position,&PlayersWeapon.ObMat,1);
}

extern void RocketBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    PREDPISTOL_BEHAV_BLOCK *bbPtr = (PREDPISTOL_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	MakeRocketTrailParticles(&(dynPtr->PrevPosition), &(dynPtr->Position));

	if (reportPtr || (bbPtr->counter<=0) )
    {        

		if (reportPtr) {
			if (reportPtr->ObstacleSBPtr) {
				VECTORCH attack_dir;
			
				if (AccuracyStats_TargetFilter(reportPtr->ObstacleSBPtr)) {
					if (bbPtr->player) {
						int slot;
						/* Log accuracy! */
						slot=SlotForThisWeapon(WEAPON_SADAR);
						if (slot!=-1) {
							CurrentGameStats_WeaponHit(slot,1);
						}
					}
				}

				GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
				CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_SADAR_TOW].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
			}
		}

        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_SADAR_BLAST].MaxRange,
			&TemplateAmmo[AMMO_SADAR_BLAST].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_SADAR_BLAST].ExplosionIsFlat
		);
		
		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
			Sound_Play(SID_NICE_EXPLOSION,"n",&Explosion_SoundData);
    	}
 			
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);

    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
		//dynPtr->IgnoreThePlayer=0;
	}
}

STRATEGYBLOCK* CreateGrenadeKernel(AVP_BEHAVIOUR_TYPE behaviourID, VECTORCH *position, MATRIXCH *orient,int fromplayer) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;

	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(behaviourID,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	
	switch(behaviourID)
	{
		case I_BehaviourFlareGrenade:
		{
			dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(FLARE_BEHAV_BLOCK));
			break;
		}
		case I_BehaviourProximityGrenade:
		{
			dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PROX_GRENADE_BEHAV_BLOCK));
			break;
		}
		default:
		{
			dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(GRENADE_BEHAV_BLOCK));
			break;
		}
	}

	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_GRENADE);

	if (fromplayer==0) {
		dynPtr->IgnoreThePlayer=0;
	}

	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
		/* setup dynamics block */
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* align grenade to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* and convert to an euler */
 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    #define GRENADE_SPEED 70000
    dynPtr->LinImpulse.vx = MUL_FIXED(dynPtr->OrientMat.mat31,GRENADE_SPEED);
    dynPtr->LinImpulse.vy = MUL_FIXED(dynPtr->OrientMat.mat32,GRENADE_SPEED);
    dynPtr->LinImpulse.vz = MUL_FIXED(dynPtr->OrientMat.mat33,GRENADE_SPEED);
    dynPtr->AngImpulse.EulerX = ((FastRandom()&2047)-1024)*4;
    dynPtr->AngImpulse.EulerY = ((FastRandom()&2047)-1024)*4;
    dynPtr->AngImpulse.EulerZ = ((FastRandom()&2047)-1024)*8;

	dynPtr->IgnoresNotVisPolys = 1;

	switch(behaviourID)
	{
		case I_BehaviourFlareGrenade:
		{
			dynPtr->StopOnCollision = 1;
			((FLARE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->LifeTimeRemaining = FLARE_LIFETIME*ONE_FIXED;
			((FLARE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->ParticleGenerationTimer = 0;
			AddLightingEffectToObject(dispPtr,LFX_FLARE);
			((FLARE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->SoundHandle = SOUND_NOACTIVEINDEX;

			((FLARE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->becomeStuck = 0;

			NumberOfFlaresActive++;

			break;
		}
		case I_BehaviourProximityGrenade:
		{
			dynPtr->StopOnCollision = 1;
			((PROX_GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->LifeTimeRemaining = 2*ONE_FIXED;
			((PROX_GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->SoundHandle = SOUND_NOACTIVEINDEX;
			((PROX_GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->SoundGenerationTimer = 0;
			break;
		}
		default:
		{
			((GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 2*ONE_FIXED;
			((GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->bouncelastframe = 0;
			break;
		}
	}

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	return dispPtr->ObStrategyBlock;
}

static void InitialiseGrenadeBehaviour(AVP_BEHAVIOUR_TYPE behaviourID)
{
	VECTORCH position;
	
	/* calculate the position */
	{
		extern VECTORCH CentreOfMuzzleOffset;
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
 		position = CentreOfMuzzleOffset;
		
	  	RotateVector(&position,&PlayersWeapon.ObMat);
	
	 	position.vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
	 	position.vx = position.vx/4 + VDBPtr->VDB_World.vx;

	 	position.vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
		position.vy = position.vy/4 + VDBPtr->VDB_World.vy;
		
	 	position.vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
		position.vz = position.vz/4 + VDBPtr->VDB_World.vz;
  	}

	CreateGrenadeKernel(behaviourID,&position,&PlayersWeapon.ObMat,1);

}


extern void GrenadeBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    GRENADE_BEHAV_BLOCK *bbPtr = (GRENADE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int explodeNow = 0;
	int bounce=0;

	/* some sort of trail would look good */
	/* but calling this takes bloody ages */
	MakeGrenadeTrailParticles(&dynPtr->PrevPosition,&dynPtr->Position);
	#if 0
	{
		VECTORCH velocity;
		velocity.vx = (FastRandom()&255) - 128;
		velocity.vy = -1000-(FastRandom()&255);
		velocity.vz = (FastRandom()&255) - 128;
		MakeParticle(&(dynPtr->Position),&(velocity),PARTICLE_BLACKSMOKE);
	}
	#endif
	
	//if (reportPtr==NULL) {
	//	dynPtr->IgnoreThePlayer=0;
	//}
	/* explode if the grenade touches an alien */
	if (reportPtr==NULL) {
		bbPtr->bouncelastframe=0;
	}

	while (reportPtr)
	{
		STRATEGYBLOCK *sbPtr = reportPtr->ObstacleSBPtr;

		bounce=1;

		if(sbPtr)
		{
			if((sbPtr->I_SBtype == I_BehaviourAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourMarinePlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourAlienPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredator)
			 ||(sbPtr->I_SBtype == I_BehaviourXenoborg)
			 ||(sbPtr->I_SBtype == I_BehaviourMarine)
			 ||(sbPtr->I_SBtype == I_BehaviourQueenAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourFaceHugger))
			{
				explodeNow = 1; /* kaboom */
			}
		
			if(sbPtr->I_SBtype == I_BehaviourNetGhost)
			{ 
				NETGHOSTDATABLOCK *ghostData = sbPtr->SBdataptr;
				LOCALASSERT(ghostData);
				LOCALASSERT(AvP.Network!=I_No_Network);

				if((ghostData->type == I_BehaviourMarinePlayer)||
			   		(ghostData->type == I_BehaviourPredatorPlayer)||
			   		(ghostData->type == I_BehaviourAlienPlayer)||
			   		(ghostData->type == I_BehaviourAlien))
				{
			   		explodeNow = 1;
				}				
			}
		} else {
			dynPtr->IgnoreThePlayer=0;
		}	
				
		/* skip to next report */
		reportPtr = reportPtr->NextCollisionReportPtr;
	}														

	if (bounce&&(explodeNow==0)&&(bbPtr->bouncelastframe==0)) {
		Sound_Play(SID_GRENADE_BOUNCE,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
		bbPtr->bouncelastframe=1;
	}

	if ((bbPtr->counter<=0) || explodeNow)
    {        
        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_GRENADE].MaxRange,
	 		&TemplateAmmo[AMMO_GRENADE].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_GRENADE].ExplosionIsFlat
		);
		
	    if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
		    Sound_Play(SID_ED_GRENADE_EXPLOSION,"n",&Explosion_SoundData);
    	}
 		
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);
    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
        DynamicallyRotateObject(dynPtr);
	}
}
extern void ClusterGrenadeBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    GRENADE_BEHAV_BLOCK *bbPtr = (GRENADE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int explodeNow = 0;
	int bounce=0;

	{
		VECTORCH velocity;
		velocity.vx = (FastRandom()&255) - 128;
		velocity.vy = -1000-(FastRandom()&255);
		velocity.vz = (FastRandom()&255) - 128;
		MakeParticle(&(dynPtr->Position),&(velocity),PARTICLE_BLACKSMOKE);
	}
	
	//if (reportPtr==NULL) {
	//	dynPtr->IgnoreThePlayer=0;
	//}
	/* explode if the grenade touches an alien */
	if (reportPtr==NULL) {
		bbPtr->bouncelastframe=0;
	}

	while (reportPtr)
	{
		STRATEGYBLOCK *sbPtr = reportPtr->ObstacleSBPtr;

		bounce=1;

		if (sbPtr)
		{
			if((sbPtr->I_SBtype == I_BehaviourAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourMarinePlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourAlienPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredator)
			 ||(sbPtr->I_SBtype == I_BehaviourXenoborg)
			 ||(sbPtr->I_SBtype == I_BehaviourMarine)
			 ||(sbPtr->I_SBtype == I_BehaviourQueenAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourFaceHugger))
			{
				explodeNow = 1; /* kaboom */
			}

			if(sbPtr->I_SBtype == I_BehaviourNetGhost)
			{ 
				NETGHOSTDATABLOCK *ghostData = sbPtr->SBdataptr;
				LOCALASSERT(ghostData);
				LOCALASSERT(AvP.Network!=I_No_Network);

				if((ghostData->type == I_BehaviourMarinePlayer)||
			   		(ghostData->type == I_BehaviourPredatorPlayer)||
			   		(ghostData->type == I_BehaviourAlienPlayer)||
			   		(ghostData->type == I_BehaviourAlien))
				{
			   		explodeNow = 1;
				}				
			}
		} else {
			dynPtr->IgnoreThePlayer=0;
		}	

		/* skip to next report */
		reportPtr = reportPtr->NextCollisionReportPtr;
	}														
   
	if (bounce&&(explodeNow==0)&&(bbPtr->bouncelastframe==0)) {
		Sound_Play(SID_GRENADE_BOUNCE,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
		bbPtr->bouncelastframe=1;
	}

	if ((bbPtr->counter<=0) || explodeNow)
    {        
		extern void MakeFlechetteExplosionAt(VECTORCH *positionPtr,int seed);
		MakeFlechetteExplosionAt(&dynPtr->Position,0);
		#if 0
		/* make explosion sprite & frag grenades */
		{
			int num;
			for(num=0;num<NO_OF_FRAGS_IN_CLUSTER_BOMB;num++)
			{
				InitialiseFragmentationGrenade(&(dynPtr->Position));
			}
		}
		#endif
		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
		    Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
    	}
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);
    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
        DynamicallyRotateObject(dynPtr);
	}
}

#if 0
static void InitialiseFragmentationGrenade(VECTORCH *originPtr)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH position;
	
	/* calculate the position */
	position = *originPtr;
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourFragmentationGrenade,&position);
	if (dispPtr == 0) return;		 // Failed to allocate display block
	
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	
	/* give grenade a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(GRENADE_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}
			
	((GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = FRAG_LIFETIME;
	((GRENADE_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->bouncelastframe = 0;

	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_GRENADE);

	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	/* setup dynamics block */
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* align grenade to launcher */
	dynPtr->Position=position;
	dynPtr->OrientMat = PlayersWeapon.ObMat;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* and convert to an euler */
 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    dynPtr->LinImpulse.vx = ((FastRandom()&16383)-8192);
    dynPtr->LinImpulse.vy = -(FastRandom()&16383)-8192;
    dynPtr->LinImpulse.vz = ((FastRandom()&16383)-8192);
    dynPtr->AngImpulse.EulerX = ((FastRandom()&2047)-1024)*4;
    dynPtr->AngImpulse.EulerY = ((FastRandom()&2047)-1024)*4;
    dynPtr->AngImpulse.EulerZ = ((FastRandom()&2047)-1024)*8;

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);
}
#endif

extern void ProximityGrenadeBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    PROX_GRENADE_BEHAV_BLOCK *bbPtr = (PROX_GRENADE_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	MakeGrenadeTrailParticles(&dynPtr->PrevPosition,&dynPtr->Position);

	if (bbPtr->LifeTimeRemaining<=0)
    {        
        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_GRENADE].MaxRange,
			&TemplateAmmo[AMMO_GRENADE].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_GRENADE].ExplosionIsFlat
		);
		
		if (bbPtr->SoundHandle!=SOUND_NOACTIVEINDEX)
		{
			Sound_Stop(bbPtr->SoundHandle);
			bbPtr->SoundHandle = SOUND_NOACTIVEINDEX;

		}
	    
		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
		    Sound_Play(SID_ED_GRENADE_PROXEXPLOSION,"n",&Explosion_SoundData);
    	}
		
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy! */
    	DestroyAnyStrategyBlock(sbPtr);
		return;
    }
	else if (dynPtr->IsStatic && bbPtr->LifeTimeRemaining>PROX_GRENADE_TRIGGER_TIME)
	{
		// scan for objects in proximity
		extern int NumActiveStBlocks;
		extern STRATEGYBLOCK *ActiveStBlockList[];	
		int i = NumActiveStBlocks;

		{
			int scale = ONE_FIXED-bbPtr->LifeTimeRemaining/PROX_GRENADE_LIFETIME;
			scale = MUL_FIXED(scale,scale);
			scale = MUL_FIXED(scale,scale)*8;
			bbPtr->SoundGenerationTimer += NormalFrameTime + MUL_FIXED(NormalFrameTime,scale);
   		}
		while (bbPtr->SoundGenerationTimer >= PROX_GRENADE_SOUND_GENERATION_TIME)
		{
			bbPtr->SoundGenerationTimer -= PROX_GRENADE_SOUND_GENERATION_TIME;
			Sound_Play(SID_PROX_GRENADE_ACTIVE,"d",&(dynPtr->Position));
		}


		while (i--)
		{
			STRATEGYBLOCK *obstaclePtr = ActiveStBlockList[i];
			DYNAMICSBLOCK *obstacleDynPtr = obstaclePtr->DynPtr;
			
			if (obstacleDynPtr)
			{
				if(ValidTargetForProxMine(obstaclePtr))
				{
					VECTORCH disp = obstacleDynPtr->Position;
					disp.vx -= dynPtr->Position.vx;
					disp.vy -= dynPtr->Position.vy;
					disp.vz -= dynPtr->Position.vz;
					
					if (Approximate3dMagnitude(&disp)<=PROX_GRENADE_RANGE)
					{	
						bbPtr->LifeTimeRemaining = PROX_GRENADE_TRIGGER_TIME;
					}
				}
			}
		}
	}
	else
	{
		COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;

		if (reportPtr)
		{
			char stickWhereYouAre = 0;

			STRATEGYBLOCK *obstaclePtr = reportPtr->ObstacleSBPtr;

			if (obstaclePtr)
			{
				DISPLAYBLOCK *dispPtr = obstaclePtr->SBdptr;
			
				if(ValidTargetForProxMine(obstaclePtr))
				{
					/* best blow up then */
					bbPtr->LifeTimeRemaining=0;
					return;
				}
				else if (dispPtr)
				{
					if (dispPtr->ObMyModule && (!dispPtr->ObMorphCtrl))
					{
						stickWhereYouAre=1;
					}
				}
			}
			else
			{
				stickWhereYouAre = 1;
			}
						

			if(stickWhereYouAre)
			{
				dynPtr->IsStatic=1;
				dynPtr->PrevPosition=dynPtr->Position;
				MakeMatrixFromDirection(&(reportPtr->ObstacleNormal),&(dynPtr->OrientMat));
				/* KJL 15:27:42 23/01/99 - Euler has to be filled out for network play! */
			 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
				bbPtr->LifeTimeRemaining = PROX_GRENADE_LIFETIME*ONE_FIXED;
			}
		}
	}

   	if (bbPtr->LifeTimeRemaining<=PROX_GRENADE_TRIGGER_TIME && bbPtr->SoundHandle==SOUND_NOACTIVEINDEX)
	{
		Sound_Play(SID_PROX_GRENADE_READYTOBLOW,"de",&(dynPtr->Position),&bbPtr->SoundHandle);
	}
   	bbPtr->LifeTimeRemaining-=NormalFrameTime;

	
}

int ValidTargetForProxMine(STRATEGYBLOCK *obstaclePtr)
{
	if((obstaclePtr->I_SBtype == I_BehaviourAlien)
	 ||(obstaclePtr->I_SBtype == I_BehaviourMarinePlayer)
	 ||(obstaclePtr->I_SBtype == I_BehaviourAlienPlayer)
	 ||(obstaclePtr->I_SBtype == I_BehaviourPredatorPlayer)
	 ||(obstaclePtr->I_SBtype == I_BehaviourPredator)
	 ||(obstaclePtr->I_SBtype == I_BehaviourXenoborg)
	 ||(obstaclePtr->I_SBtype == I_BehaviourMarine)
	 ||(obstaclePtr->I_SBtype == I_BehaviourQueenAlien)
	 ||(obstaclePtr->I_SBtype == I_BehaviourFaceHugger))
	{
		return 1;
	}

	if(obstaclePtr->I_SBtype == I_BehaviourNetGhost)
	{
		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)obstaclePtr->SBdataptr;

		if (ghostDataPtr->type==I_BehaviourAlienPlayer 
		  ||ghostDataPtr->type==I_BehaviourMarinePlayer
		  ||ghostDataPtr->type==I_BehaviourPredatorPlayer
		  ||ghostDataPtr->type==I_BehaviourAlien)
		{
			return 1;
		}
	}
	return 0;
}


extern void FlareGrenadeBehaviour(STRATEGYBLOCK *sbPtr) 
{
    FLARE_BEHAV_BLOCK *bbPtr = (FLARE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;


	if ((bbPtr->LifeTimeRemaining<=0))
    {        
		if (bbPtr->SoundHandle!=SOUND_NOACTIVEINDEX)
		{
			Sound_Stop(bbPtr->SoundHandle);
			bbPtr->SoundHandle = SOUND_NOACTIVEINDEX;

		}
		
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);


		NumberOfFlaresActive--;
		return;
    }

	{
		if (bbPtr->LifeTimeRemaining>ONE_FIXED*4)
		{
			bbPtr->ParticleGenerationTimer += NormalFrameTime;
		}
		else
		{
			bbPtr->ParticleGenerationTimer += MUL_FIXED(NormalFrameTime,bbPtr->LifeTimeRemaining)/4;
		}
   		
		while (bbPtr->ParticleGenerationTimer >= FLARE_PARTICLE_GENERATION_TIME)
		{
			bbPtr->ParticleGenerationTimer -= FLARE_PARTICLE_GENERATION_TIME;
			MakeFlareParticle(dynPtr);
		}

		/* add lighting effect */
		{
			LIGHTBLOCK *lightPtr = sbPtr->SBdptr->ObLights[0];
			LOCALASSERT(sbPtr->SBdptr->ObNumLights==1);
			lightPtr->LightBright = 1+MUL_FIXED
									(
										(ONE_FIXED*4-(FastRandom()&32767)),
										bbPtr->LifeTimeRemaining/FLARE_LIFETIME
									);
		}
   	
   		bbPtr->LifeTimeRemaining-=NormalFrameTime;
	}
	
	if (dynPtr->IsFloating)
	{
		RubberDuckBehaviour(sbPtr);
	}
	else if (!dynPtr->IsStatic)
	{
		COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;

		//if (reportPtr==NULL) {
		//	dynPtr->IgnoreThePlayer=0;
		//}
		if (reportPtr)
		{
			char stickWhereYouAre = 0;

			if (reportPtr->ObstacleSBPtr)
			{
				DISPLAYBLOCK *dispPtr = reportPtr->ObstacleSBPtr->SBdptr;
				if (dispPtr)
				if (dispPtr->ObMyModule && (!dispPtr->ObMorphCtrl))
				{
					stickWhereYouAre=1;
				}
			}
			else
			{
				stickWhereYouAre = 1;
			}
						

			if(stickWhereYouAre)
			{
				dynPtr->IsStatic=1;
				dynPtr->OnlyCollideWithEnvironment = 1;
				dynPtr->PrevPosition=dynPtr->Position;
				MakeMatrixFromDirection(&(reportPtr->ObstacleNormal),&(dynPtr->OrientMat));
				/* KJL 15:27:42 23/01/99 - Euler has to be filled out for network play! */
			 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
				Sound_Play(SID_BURNING_FLARE,"dle",&(dynPtr->Position),&bbPtr->SoundHandle);

				//set flag ,so appropriate net message gets sent
				bbPtr->becomeStuck=1;

			}
		}
//  		DynamicallyRotateObject(dynPtr);
	}
}

static STRATEGYBLOCK* InitialisePulseGrenadeBehaviour(void)
{
	/* more of a rocket than a grenade... */
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH position;
	
	/* calculate the position */
	{
		extern VECTORCH CentreOfMuzzleOffset;
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
 		position = CentreOfMuzzleOffset;
		
	  	RotateVector(&position,&PlayersWeapon.ObMat);
	
	 	position.vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
	 	position.vx = position.vx/4 + VDBPtr->VDB_World.vx;

	 	position.vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
		position.vy = position.vy/4 + VDBPtr->VDB_World.vy;
		
	 	position.vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
		position.vz = position.vz/4 + VDBPtr->VDB_World.vz;
  	}
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPulseGrenade,&position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_ROCKETJET);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PREDPISTOL_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = 1;
			
	/* align rocket to launcher */
	dynPtr->Position=position;
	dynPtr->PrevPosition=position;
	dynPtr->OrientMat = PlayersWeapon.ObMat;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;

	/* align velocity too */	
    #define PULSEGRENADE_SPEED 100000 // Was 30000
	GetGunDirection(&(dynPtr->LinVelocity),&position);
    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, PULSEGRENADE_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, PULSEGRENADE_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, PULSEGRENADE_SPEED);
 
	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	return dispPtr->ObStrategyBlock; 
}


extern void PulseGrenadeBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    PREDPISTOL_BEHAV_BLOCK *bbPtr = (PREDPISTOL_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	MakeRocketTrailParticles(&(dynPtr->PrevPosition), &(dynPtr->Position));

	//Work out the containing module now , since it doesn't seem to get done anywhere else
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), sbPtr->containingModule);

	
	//if (reportPtr==NULL) {
	//	dynPtr->IgnoreThePlayer=0;
	//}
	if (reportPtr || (bbPtr->counter<=0) )
    {        

		if (reportPtr) {
			if (reportPtr->ObstacleSBPtr) {
				VECTORCH attack_dir;
			
				if (AccuracyStats_TargetFilter(reportPtr->ObstacleSBPtr)) {
					if (bbPtr->player) {
						int slot;
						/* Log accuracy! */
						slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
						if (slot!=-1) {
							CurrentGameStats_WeaponHit(slot,1);
						}
					}
				}

				GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
				CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PULSE_GRENADE_STRIKE].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
			}
		}

        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_PULSE_GRENADE].MaxRange,
			&TemplateAmmo[AMMO_PULSE_GRENADE].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_PULSE_GRENADE].ExplosionIsFlat
		);

		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
		    Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
    	}
			
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);
    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
	}
}

STRATEGYBLOCK* InitialiseEnergyBoltBehaviourKernel(VECTORCH *position,MATRIXCH *orient, int player, DAMAGE_PROFILE *damage, int factor) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPredatorEnergyBolt,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	dispPtr->SfxPtr = AllocateSfxBlock();

	if (!dispPtr->SfxPtr)
	{
		// Failed to allocate a special fx block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->SfxPtr->SfxID = SFX_PREDATOR_PLASMA_BOLT;
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	dispPtr->ObShape = 0;
	dispPtr->ObStrategyBlock->shapeIndex = 0;
	dispPtr->ObMinX = -50;
	dispPtr->ObMinY = -50;
	dispPtr->ObMinZ = -50;
	dispPtr->ObMaxX = 50;
	dispPtr->ObMaxY = 50;
	dispPtr->ObMaxZ = 50;
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_PLASMA_BOLT);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(CASTER_BOLT_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->damage = *damage;			
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->blast_radius = MUL_FIXED(factor,Caster_BlastRadius);			
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = player;
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->soundHandle = SOUND_NOACTIVEINDEX;
			
	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;

	//GetGunDirection(&(dynPtr->LinVelocity),&position);
  	//MakeMatrixFromDirection(&(dynPtr->LinVelocity),&(dynPtr->OrientMat));
	//MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
	//dynPtr->PrevOrientMat = dynPtr->OrientMat;

	/* align velocity too */	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, ENERGY_BOLT_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, ENERGY_BOLT_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, ENERGY_BOLT_SPEED);


	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	/* Extra cunning! */
	Sound_Play(SID_PRED_LAUNCHER,"hpd",(FastRandom()&255)-128,&dynPtr->Position);

	if (player==0) {
		dynPtr->IgnoreThePlayer=0;
	}

	return dispPtr->ObStrategyBlock;
}

void InitialiseEnergyBoltBehaviour(DAMAGE_PROFILE *damage, int factor)
{
	VECTORCH position={-300,0,0};
	
	/* calculate the position */
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

		MATRIXCH matrix = VDBPtr->VDB_Mat;
		TransposeMatrixCH(&matrix);
			
	  	RotateVector(&position,&matrix);
	
	 	position.vx += VDBPtr->VDB_World.vx;
		position.vy += VDBPtr->VDB_World.vy;
		position.vz += VDBPtr->VDB_World.vz;
  	}

	#if 1
	{
		VECTORCH targetDirection;
		MATRIXCH orient;

		GetGunDirection(&targetDirection,&position);
		MakeMatrixFromDirection(&targetDirection,&orient);

		InitialiseEnergyBoltBehaviourKernel(&position,&orient,1,damage,factor);
	}
	#else
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPredatorEnergyBolt,&position);
	if (dispPtr == 0) return;		 // Failed to allocate display block
	
	dispPtr->SfxPtr = AllocateSfxBlock();

	if (!dispPtr->SfxPtr)
	{
		// Failed to allocate a special fx block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	dispPtr->SfxPtr->SfxID = SFX_PREDATOR_PLASMA_BOLT;
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	dispPtr->ObShape = 0;
	dispPtr->ObStrategyBlock->shapeIndex = 0;
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_PLASMA_BOLT);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(CASTER_BOLT_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->damage = *damage;			
			
	/* align rocket to launcher */
	dynPtr->Position=position;
	dynPtr->PrevPosition=position;

	/* align velocity too */	
    #define ENERGY_BOLT_SPEED 3000
	GetGunDirection(&(dynPtr->LinVelocity),&position);
	MakeMatrixFromDirection(&(dynPtr->LinVelocity),&(dynPtr->OrientMat));
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
	dynPtr->PrevOrientMat = dynPtr->OrientMat;

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	/* Extra cunning! */
	Sound_Play(SID_PRED_LAUNCHER,"hpd",(FastRandom()&255)-128,&dynPtr->Position);
	#endif
}

/****/

#if NEW_PREDPISTOL_BOLT
STRATEGYBLOCK* CreatePPPlasmaBoltKernel(VECTORCH *position,MATRIXCH *orient, int player)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPPPlasmaBolt,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	dispPtr->SfxPtr = AllocateSfxBlock();

	if (!dispPtr->SfxPtr)
	{
		// Failed to allocate a special fx block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	
	dispPtr->SfxPtr->SfxID = SFX_SMALL_PREDATOR_PLASMA_BOLT;
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	dispPtr->ObShape = 0;
	dispPtr->ObStrategyBlock->shapeIndex = 0;
	
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_PLASMA_BOLT);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_GRENADE);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PREDPISTOL_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = player;
			
	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;

	/* align velocity too */	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	

    dynPtr->LinVelocity.vx = 0;
    dynPtr->LinVelocity.vy = 0;
    dynPtr->LinVelocity.vz = 0;

    dynPtr->LinImpulse.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinImpulse.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinImpulse.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinImpulse.vx = MUL_FIXED(dynPtr->LinImpulse.vx, PredPistolBoltSpeed);
    dynPtr->LinImpulse.vy = MUL_FIXED(dynPtr->LinImpulse.vy, PredPistolBoltSpeed);
    dynPtr->LinImpulse.vz = MUL_FIXED(dynPtr->LinImpulse.vz, PredPistolBoltSpeed);

	dynPtr->UseStandardGravity=0;
	dynPtr->GravityDirection.vx=0;
	dynPtr->GravityDirection.vy=PredPistolBoltGravity; /* Half gravity! */
	dynPtr->GravityDirection.vz=0;

	dynPtr->StopOnCollision=1;
	
	if (player==0) {
		dynPtr->IgnoreThePlayer=0;
	}

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	return dispPtr->ObStrategyBlock;
}
#else
void CreatePPPlasmaBoltKernel(VECTORCH *position,MATRIXCH *orient, int player)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPPPlasmaBolt,position);
	if (dispPtr == 0) return;		 // Failed to allocate display block
	
	dispPtr->SfxPtr = AllocateSfxBlock();

	if (!dispPtr->SfxPtr)
	{
		// Failed to allocate a special fx block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}
	
	dispPtr->SfxPtr->SfxID = SFX_SMALL_PREDATOR_PLASMA_BOLT;
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	dispPtr->ObShape = 0;
	dispPtr->ObStrategyBlock->shapeIndex = 0;
	
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_PLASMA_BOLT);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	((ONE_SHOT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
			
	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;

	/* align velocity too */	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    #define PPBOLT_SPEED 32767
    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, PPBOLT_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, PPBOLT_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, PPBOLT_SPEED);

	if (player==0) {
		dynPtr->IgnoreThePlayer=0;
	}

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);
}
#endif

static void InitialisePPPlasmaBoltBehaviour(void)
{
	VECTORCH position;

	GLOBALASSERT(PWMFSDP);
	
	/* calculate the position */
		
	position=PWMFSDP->World_Offset;

	#if NEAR_WEAPON_FUDGE
	{
		VECTORCH fudgeFactor;
		extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;

		fudgeFactor.vx=position.vx-Global_VDB_Ptr->VDB_World.vx;
		fudgeFactor.vy=position.vy-Global_VDB_Ptr->VDB_World.vy;
		fudgeFactor.vz=position.vz-Global_VDB_Ptr->VDB_World.vz;

		Crunch_Position_For_Players_Weapon(&fudgeFactor);

		position=fudgeFactor;
	}
	#endif

	CreatePPPlasmaBoltKernel(&position,&PlayersWeapon.ObMat,1);
	
}

#if NEW_PREDPISTOL_BOLT
extern void PPPlasmaBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	/* Now, kinda explosive. */
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    PREDPISTOL_BEHAV_BLOCK *bbPtr = (PREDPISTOL_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int explodeNow=0;

	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		explodeNow=1;
	}
	else if (reportPtr)
	{
  		#if 1
  		if(reportPtr->ObstacleSBPtr) {
			VECTORCH attack_dir;
			/* A bit more damage if it hits you? */

			if (AccuracyStats_TargetFilter(reportPtr->ObstacleSBPtr)) {
				if (bbPtr->player) {
					int slot;
					/* Log accuracy! */
					slot=SlotForThisWeapon(WEAPON_PRED_PISTOL);
					if (slot!=-1) {
						CurrentGameStats_WeaponHit(slot,1);
					}
				}
			}

			GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
			CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PREDPISTOL_STRIKE].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
		#endif
		explodeNow=1;

	} else {
		bbPtr->counter -= NormalFrameTime;
	}

	if (explodeNow) {

		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_PRED_PISTOL].MaxRange,
	 		&TemplateAmmo[AMMO_PRED_PISTOL].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_PRED_PISTOL].ExplosionIsFlat
		);
		

		#if 0
		MakeBloodExplosion(&dynPtr->Position,50,&dynPtr->Position,100,PARTICLE_PREDPISTOL_FLECHETTE);
		#endif

		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	DestroyAnyStrategyBlock(sbPtr);	
	}
}
#else
extern void PPPlasmaBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    ONE_SHOT_BEHAV_BLOCK *bbPtr = (ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if (reportPtr)
	{
  		if(reportPtr->ObstacleSBPtr) {
			VECTORCH attack_dir;
			GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
			CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_PISTOL].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	DestroyAnyStrategyBlock(sbPtr);	

	} else {
		bbPtr->counter -= NormalFrameTime;
	}
}
#endif

/****/

#define SPEAR_BOLT_SPEED 		(200000)
#define SPEAR_PLAYER_IMPULSE 	(-8000)
#define SPEAR_FUDGE_FACTOR		(500)
/* Was 50000. */

static void InitialiseSpeargunBoltBehaviour(void)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH position;
	
	LOCALASSERT(0); // this routine should not be called

	GLOBALASSERT(PWMFSDP);

	position=PWMFSDP->World_Offset;

	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourSpeargunBolt,&position);
	if (dispPtr == 0) return;		 // Failed to allocate display block
	
	/* KJL 17:53:36 01/08/98 - make the extents teeny-weeny */
	dispPtr->ObMaxX = 10;
	dispPtr->ObMaxY = 10;
	dispPtr->ObMaxZ = 10;
	dispPtr->ObMinX = -10;
	dispPtr->ObMinY = -10;
	dispPtr->ObMinZ = -10;

	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(SPEAR_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}
	memset(dispPtr->ObStrategyBlock->SBdataptr,0,sizeof(SPEAR_BEHAV_BLOCK));

	((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->Stuck = 0;
	/* Is this function still used? */
	((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->Android = 0;

	/* align rocket to launcher */
	dynPtr->Position=position;
	#if NEAR_WEAPON_FUDGE
	{
		VECTORCH fudgeFactor;
		extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;

		fudgeFactor.vx=dynPtr->Position.vx-Global_VDB_Ptr->VDB_World.vx;
		fudgeFactor.vy=dynPtr->Position.vy-Global_VDB_Ptr->VDB_World.vy;
		fudgeFactor.vz=dynPtr->Position.vz-Global_VDB_Ptr->VDB_World.vz;

		Crunch_Position_For_Players_Weapon(&fudgeFactor);

		dynPtr->Position=fudgeFactor;
	}
	#endif
	dynPtr->PrevPosition=position;
	dynPtr->OrientMat = PlayersWeapon.ObMat;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* align velocity too */	
	
	GetGunDirection(&(dynPtr->LinVelocity),&dynPtr->Position);
	dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx,SPEAR_BOLT_SPEED);
	dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy,SPEAR_BOLT_SPEED);
	dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz,SPEAR_BOLT_SPEED);

	Player->ObStrategyBlock->DynPtr->LinImpulse.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat31,SPEAR_PLAYER_IMPULSE);
	Player->ObStrategyBlock->DynPtr->LinImpulse.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat32,SPEAR_PLAYER_IMPULSE);
	Player->ObStrategyBlock->DynPtr->LinImpulse.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat33,SPEAR_PLAYER_IMPULSE);
	
	dynPtr->Mass=1000;

	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);
}

static DISPLAYBLOCK* InitialiseSpeargunBoltBehaviour_ForLoad(void)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH position = {0,0,0};
	

	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourSpeargunBolt,&position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	/* KJL 17:53:36 01/08/98 - make the extents teeny-weeny */
	dispPtr->ObMaxX = 10;
	dispPtr->ObMaxY = 10;
	dispPtr->ObMaxZ = 10;
	dispPtr->ObMinX = -10;
	dispPtr->ObMinY = -10;
	dispPtr->ObMinZ = -10;

	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(SPEAR_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	memset(dispPtr->ObStrategyBlock->SBdataptr,0,sizeof(SPEAR_BEHAV_BLOCK));
	return dispPtr;
}


extern void SpeargunBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    SPEAR_BEHAV_BLOCK *bbPtr = (SPEAR_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	if (!bbPtr->Stuck) {
	 //	MakeRocketTrailParticles(&(dynPtr->PrevPosition), &(dynPtr->Position));
	} else {
		/* Stuck behaviour. */

		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
	
		dynPtr->LinImpulse.vx = 0;
		dynPtr->LinImpulse.vy = 0;
		dynPtr->LinImpulse.vz = 0;

		bbPtr->counter -= NormalFrameTime;
		if (bbPtr->counter <= 0) 
		{
			/* for net game support: send a message saying we've blown up... */
			if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);
			DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
		}
		return;
	}
	
	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);
		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if (reportPtr)
	{
		int normDotBeta = DotProduct(&(dynPtr->LinVelocity),&(reportPtr->ObstacleNormal));
		char stickWhereYouAre = 0;

		if (reportPtr->ObstacleSBPtr)
		{
			DISPLAYBLOCK *dispPtr = reportPtr->ObstacleSBPtr->SBdptr;
			if (dispPtr)
			if (dispPtr->ObMyModule && (!dispPtr->ObMorphCtrl))
			{
				stickWhereYouAre=1;
			}
		}
		else
		{
			stickWhereYouAre = 1;
		}
					
		if(stickWhereYouAre && normDotBeta!=0)
		{
			/* Sink in... */
		    Sound_Play(SID_SPEARGUN_HITTING_WALL,"d",&dynPtr->Position);  
		   //	MakeImpactSparks(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal,&dynPtr->Position);
			bbPtr->Stuck=1;
			/* Counter at 20s. */
			bbPtr->counter=(20*ONE_FIXED);
			dynPtr->GravityOn=0;
			dynPtr->DynamicsType = DYN_TYPE_NO_COLLISIONS;

			{
				int d;
				{
					/* get a pt in the poly */
					VECTORCH pop = reportPtr->ObstaclePoint;								  
					pop.vx -= dynPtr->Position.vx;
					pop.vy -= dynPtr->Position.vy;
					pop.vz -= dynPtr->Position.vz;

					/* hmm, what about double sided polys? */
				  	d = DotProduct(&(reportPtr->ObstacleNormal),&pop);
				}

				{
				  	int lambda = DIV_FIXED(d,normDotBeta);
					
			   		dynPtr->Position.vx	+= MUL_FIXED(lambda,dynPtr->LinVelocity.vx);
	 		   		dynPtr->Position.vy	+= MUL_FIXED(lambda,dynPtr->LinVelocity.vy);
			   		dynPtr->Position.vz	+= MUL_FIXED(lambda,dynPtr->LinVelocity.vz);
				}

			}
			dynPtr->LinVelocity.vx=0;
			dynPtr->LinVelocity.vy=0;
			dynPtr->LinVelocity.vz=0;
			dynPtr->LinImpulse.vx=0;
			dynPtr->LinImpulse.vy=0;
			dynPtr->LinImpulse.vz=0;
			
			MakeFocusedExplosion(&(dynPtr->PrevPosition), &(dynPtr->Position), 20, PARTICLE_SPARK);

		}
		else
		{
			if(reportPtr->ObstacleSBPtr)
	  		{
				VECTORCH attack_dir;
				GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
				CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_RIFLE].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
			}
			if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);
	    	DestroyAnyStrategyBlock(sbPtr);	
		}
	} else {
		/* No collisions. */
		//dynPtr->IgnoreThePlayer=0;

		bbPtr->counter -= NormalFrameTime;
	}
}

/****/

void Crunch_Position_For_Players_Weapon(VECTORCH *position) {

	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

	position->vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
	position->vx = position->vx/4 + VDBPtr->VDB_World.vx;

	position->vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
	position->vy = position->vy/4 + VDBPtr->VDB_World.vy;
	
	position->vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
	position->vz = position->vz/4 + VDBPtr->VDB_World.vz;

}

void FireFlameThrower(VECTORCH *base_position,VECTORCH *base_offset,MATRIXCH *orientmat,int player, int *timer) {

	/* Simple function containing flamethrower function. */

	VECTORCH position;

	(*timer)+=NormalFrameTime;

	while ((*timer)>=TIME_FOR_FLAMETHROWER_PARTICLE) 
	{
		VECTORCH velocity;

		(*timer)-=TIME_FOR_FLAMETHROWER_PARTICLE;
	
		/* calculate the position */

		{
			int offset = MUL_FIXED(FastRandom()&16383,NormalFrameTime);
	
			position=*base_offset;

			position.vz += offset;
			position.vy += (FastRandom()%(offset/8+1)) - offset/16;
			position.vx += (FastRandom()%(offset/8+1)) - offset/16;
		}
	
		RotateVector(&position,orientmat);

		if (player) {
			Crunch_Position_For_Players_Weapon(&position);		
		} else {
			position.vx+=base_position->vx;
			position.vy+=base_position->vy;
			position.vz+=base_position->vz;
		}

		velocity.vx = ((FastRandom()&1023) - 512);//*2;
		velocity.vy = ((FastRandom()&1023) - 512);//*2;
		velocity.vz = ((FastRandom()&511) + 200+512)*16;
		RotateVector(&velocity,orientmat);
		MakeParticle(&position,&(velocity),PARTICLE_FLAME);
	}

}

void FireNetGhostFlameThrower(VECTORCH *positionPtr, MATRIXCH *orientMatPtr)
{
	/* KJL 16:31:42 27/01/98 - these particles aren't colliding, so I'll
	see what happens if I use more... */
	int i = FLAMETHROWER_PARTICLES_PER_FRAME*2;
	
	VECTORCH position;

	while(i--)
	/* calculate the position */
	{
		VECTORCH velocity;

		{
			int offset = MUL_FIXED(FastRandom()&16383,NormalFrameTime);
	
			position.vz = offset;
			position.vy = (FastRandom()%(offset/8+1)) - offset/16;
			position.vx = (FastRandom()%(offset/8+1)) - offset/16;
		}

		velocity.vx = ((FastRandom()&1023) - 512);//*2;
		velocity.vy = ((FastRandom()&1023) - 512);//*2;
		velocity.vz = ((FastRandom()&511) + 200+512)*16;
	
		RotateVector(&position,orientMatPtr);
		RotateVector(&velocity,orientMatPtr);

		position.vx+=positionPtr->vx;
		position.vy+=positionPtr->vy;
		position.vz+=positionPtr->vz;

		if (LocalDetailLevels.GhostFlameThrowerCollisions==0) {
			MakeParticle(&position,&(velocity),PARTICLE_NONCOLLIDINGFLAME);
		} else {
			MakeParticle(&position,&(velocity),PARTICLE_NONDAMAGINGFLAME);
		}
	}

}


static void InitialiseFlameThrowerBehaviour(void)
{
	VECTORCH position;

#if 1
	{
		int i = FLAMETHROWER_PARTICLES_PER_FRAME;
		while(i--)
		/* calculate the position */
		{
			extern VECTORCH CentreOfMuzzleOffset;
			extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
			VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
			VECTORCH velocity;
	 	
	 		position = CentreOfMuzzleOffset;
			
			#if 1
			{
				int offset = MUL_FIXED(FastRandom()&16383,NormalFrameTime);
				position.vz += offset;
				position.vy += (FastRandom()%(offset/8+1)) - offset/16;
				position.vx += (FastRandom()%(offset/8+1)) - offset/16;
			}
			#endif
		  	RotateVector(&position,&PlayersWeapon.ObMat);
		
		 	position.vx+=PlayersWeapon.ObWorld.vx - VDBPtr->VDB_World.vx;
		 	position.vx = position.vx/4 + VDBPtr->VDB_World.vx;

		 	position.vy+=PlayersWeapon.ObWorld.vy - VDBPtr->VDB_World.vy;
			position.vy = position.vy/4 + VDBPtr->VDB_World.vy;
			
		 	position.vz+=PlayersWeapon.ObWorld.vz - VDBPtr->VDB_World.vz;
			position.vz = position.vz/4 + VDBPtr->VDB_World.vz;

	  	
			
			velocity.vx = ((FastRandom()&1023) - 512);//*2;
			velocity.vy = ((FastRandom()&1023) - 512);//*2;
			velocity.vz = ((FastRandom()&1023) + 200)*16;
			RotateVector(&velocity,&(PlayersWeapon.ObMat));
			MakeParticle(&position,&(velocity),PARTICLE_FLAME);
		}
	}
 #endif
}
 
/*----------------------Patrick 4/3/97--------------------------
  Stuff for predator (and xenoborg) projectiles...
  NB will need a creation function for player (AIs have their own)
  --------------------------------------------------------------*/
extern void PredatorEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    CASTER_BOLT_BEHAV_BLOCK *bbPtr = (CASTER_BOLT_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	STRATEGYBLOCK *victim;

	victim=NULL;

	if (bbPtr->damage.Impact) {
		MakePlasmaTrailParticles(dynPtr,bbPtr->damage.Impact);
	} else if(bbPtr->damage.Id==AMMO_SADAR_TOW)	{
		MakePlasmaTrailParticles(dynPtr,100);
	}

	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if (reportPtr)
	{
  		if(reportPtr->ObstacleSBPtr)
  		{
		 	VECTORCH attack_dir;
		 	
			if (AccuracyStats_TargetFilter(reportPtr->ObstacleSBPtr)) {
				if (bbPtr->player) {
					int slot;
					/* Log accuracy! */
					slot=SlotForThisWeapon(WEAPON_PRED_SHOULDERCANNON);
					if (slot!=-1) {
						CurrentGameStats_WeaponHit(slot,1);
					}
				}
			}

		 	GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
			
			switch (reportPtr->ObstacleSBPtr->I_SBtype)
			{
				case I_BehaviourMarine:
				case I_BehaviourAlien:
				{
					SECTION_DATA *chest_section=0;
					DISPLAYBLOCK *objectPtr = reportPtr->ObstacleSBPtr->SBdptr;

					if(objectPtr)
					{
						SECTION_DATA *firstSectionPtr;

					  	firstSectionPtr=objectPtr->HModelControlBlock->section_data;
					  	LOCALASSERT(firstSectionPtr);
						LOCALASSERT(firstSectionPtr->flags&section_data_initialised);

						/* look for the object's torso in preference */
						chest_section =GetThisSectionData(objectPtr->HModelControlBlock->section_data,"chest");
					
						if (chest_section)
						{
							VECTORCH rel_pos;

							rel_pos=dynPtr->Position;
							
							rel_pos.vx-=chest_section->World_Offset.vx;
							rel_pos.vy-=chest_section->World_Offset.vy;
							rel_pos.vz-=chest_section->World_Offset.vz;

							Normalise(&rel_pos);
							
							if (reportPtr->ObstacleSBPtr->I_SBtype==I_BehaviourAlien) {
								/* Spherical BloodExplosion for aliens.  Under protest.  Bleagh. */
								CauseDamageToHModel(objectPtr->HModelControlBlock,reportPtr->ObstacleSBPtr,&bbPtr->damage, ONE_FIXED, chest_section,NULL,&chest_section->World_Offset,0);
							} else {
								CauseDamageToHModel(objectPtr->HModelControlBlock,reportPtr->ObstacleSBPtr,&bbPtr->damage, ONE_FIXED, chest_section,&rel_pos,&chest_section->World_Offset,0);
							}
						}
						else
						{
							CauseDamageToObject(reportPtr->ObstacleSBPtr,&bbPtr->damage, ONE_FIXED,NULL);
						}
						victim=reportPtr->ObstacleSBPtr;
					}

					break;
				}
				default:
				{
					CauseDamageToObject(reportPtr->ObstacleSBPtr,&bbPtr->damage, ONE_FIXED,NULL);
					victim=reportPtr->ObstacleSBPtr;
					break;
				}
			}
		}
		
		{
			char hitEnvironment = 0;

			if (reportPtr->ObstacleSBPtr)
			{
				DISPLAYBLOCK *dispPtr = reportPtr->ObstacleSBPtr->SBdptr;
				if (dispPtr)
				if (dispPtr->ObMyModule)
				{
					hitEnvironment=1;
				}
			}
			else
			{
				hitEnvironment = 1;
			}
						
			#if 0
			MakeParticle(&(dynPtr->Position),&(dynPtr->Position),PARTICLE_BLUEPLASMASPHERE);
			MakeLightElement(&dynPtr->Position,LIGHTELEMENT_PLASMACASTERHIT);
			if(hitEnvironment)
			{
				MakeBloodExplosion(&(dynPtr->PrevPosition), 127, &(dynPtr->Position), 200, PARTICLE_ORANGE_SPARK);
			    Sound_Play(SID_PLASMABOLT_DISSIPATE,"d",&(dynPtr->Position));
			}
			else
			{
				MakeFocusedExplosion(&(dynPtr->PrevPosition), &(dynPtr->Position), 100, PARTICLE_ORANGE_PLASMA);
			    Sound_Play(SID_PLASMABOLT_HIT,"d",&(dynPtr->Position));
// 			    Sound_Play(SID_BLOOD_SPLASH,"d",&(dynPtr->Position));
		   	}
			#endif
			if (hitEnvironment)
			{
				MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_DISSIPATINGPLASMA);
				if (AvP.Network != I_No_Network) AddNetMsg_MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_DISSIPATINGPLASMA);
			}
			else
			{
				MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_FOCUSEDPLASMA);
				if (AvP.Network != I_No_Network) AddNetMsg_MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_FOCUSEDPLASMA);
			}
		}

		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	/* Splash damage? */
		HandleEffectsOfExplosion
		(
			victim,
			&(dynPtr->Position),
			bbPtr->blast_radius,
	 		&bbPtr->damage,
			0
		);
    	DestroyAnyStrategyBlock(sbPtr);	
	} else {
		{
			VECTORCH direction;
			direction.vx = dynPtr->LinVelocity.vx + dynPtr->LinImpulse.vx;
			direction.vy = dynPtr->LinVelocity.vy + dynPtr->LinImpulse.vy;
			direction.vz = dynPtr->LinVelocity.vz + dynPtr->LinImpulse.vz;
			Normalise(&direction);
			MakeMatrixFromDirection(&direction,&dynPtr->OrientMat);
		}
		bbPtr->counter -= NormalFrameTime;
	}



}				 

extern void XenoborgEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    ONE_SHOT_BEHAV_BLOCK *bbPtr = (ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if (reportPtr)
	{
  		if(reportPtr->ObstacleSBPtr)
			CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_XENOBORG].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
	
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	DestroyAnyStrategyBlock(sbPtr);	
	}
	else bbPtr->counter -= NormalFrameTime;  
}

#define DISC_SPEED 30000 //5000
#define DISC_LIFETIME (10*ONE_FIXED)
#define DISC_FREETIME (1*ONE_FIXED)
#define DISC_MAX_BOUNCES	(10)

void InitialiseDiscBehaviour(STRATEGYBLOCK *target,SECTION_DATA *disc_section) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
  	PC_PRED_DISC_BEHAV_BLOCK *bblk;
	int a;

	GLOBALASSERT(disc_section);
		
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPredatorDisc_SeekTrack,&disc_section->World_Offset);

	if (dispPtr == 0) return;		 // Failed to allocate display block

	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		
 	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}
 	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* Stats! */
	dispPtr->ObStrategyBlock->SBDamageBlock.Health=100<<ONE_FIXED_SHIFT;
	dispPtr->ObStrategyBlock->SBDamageBlock.Armour=100<<ONE_FIXED_SHIFT;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.AcidResistant=0;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.FireResistant=0;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.ElectricResistant=0;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.PerfectArmour=0;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.ElectricSensitive=0;
	dispPtr->ObStrategyBlock->SBDamageBlock.SB_H_flags.Indestructable=1;

	{			
		/* align rocket to launcher */
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;

		if (PlayersTarget.Distance<1400) {
			/* Yuck! */
			dynPtr->Position=Global_VDB_Ptr->VDB_World;
			/* Nudge down a wee tad bit. */
			dynPtr->Position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat21,200);
			dynPtr->Position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat22,200);
			dynPtr->Position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat23,200);

			dynPtr->Position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat11,200);
			dynPtr->Position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat12,200);
			dynPtr->Position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat13,200);

		} else {	 
			dynPtr->Position=disc_section->World_Offset;
		}

	
		dynPtr->PrevPosition = dynPtr->Position;
		TransposeMatrixCH(&matrix);
		/* dynPtr->OrientMat = Player->ObMat; */
		dynPtr->OrientMat = matrix;
		dynPtr->PrevOrientMat = dynPtr->OrientMat;
		/* I added this next line for networking: Patrick */
		MatrixToEuler(&matrix, &PlayersWeapon.ObEuler);

		dynPtr->OrientEuler.EulerX=PlayersWeapon.ObEuler.EulerX;
		dynPtr->OrientEuler.EulerY=PlayersWeapon.ObEuler.EulerY;
		dynPtr->OrientEuler.EulerZ=PlayersWeapon.ObEuler.EulerZ;

		/* align velocity too */	
	    dynPtr->LinVelocity.vx = MUL_FIXED(matrix.mat31,DISC_SPEED);
	    dynPtr->LinVelocity.vy = MUL_FIXED(matrix.mat32,DISC_SPEED);
    	dynPtr->LinVelocity.vz = MUL_FIXED(matrix.mat33,DISC_SPEED);

	}

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PC_PRED_DISC_BEHAV_BLOCK));

	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return;
	}

	bblk=(PC_PRED_DISC_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr;
	if (target) {
		bblk->counter = DISC_LIFETIME;
	} else {
		bblk->counter = DISC_FREETIME;
	}
	bblk->bounces=0;
	bblk->Destruct=0;
	bblk->Stuck=0;
	bblk->Bounced=0;
	bblk->soundHandle = SOUND_NOACTIVEINDEX;
	for (a=0; a<SB_NAME_LENGTH; a++) {
		bblk->Prev_Target_SBname[a]='\0';
		bblk->Prev_Damaged_SBname[a]='\0';
	}
	bblk->Target=target;

	if (bblk->Target) {
		COPY_NAME(bblk->Prev_Target_SBname,bblk->Target_SBname);
		COPY_NAME(bblk->Target_SBname,bblk->Target->SBname);
	} else {
		for (a=0; a<SB_NAME_LENGTH; a++) {
			bblk->Target_SBname[a]='\0';
		}
	}

	/* Create HModel. */
	{
		SECTION *root_section;
		SECTION_DATA *local_disc;

		root_section=GetNamedHierarchyFromLibrary("disk","Disk");
				
		GLOBALASSERT(root_section);

		Create_HModel(&bblk->HModelController,root_section);
		InitHModelSequence(&bblk->HModelController,HMSQT_MarineStand,MSSS_Minigun_Delta,ONE_FIXED);

		ProveHModel(&bblk->HModelController,dispPtr);

		dispPtr->HModelControlBlock=&bblk->HModelController;

		/* Match disks. */
		local_disc=GetThisSectionData(bblk->HModelController.section_data,"disk");
		local_disc->World_Offset=disc_section->World_Offset;
		local_disc->SecMat=disc_section->SecMat;
		InitHModelTweening(&bblk->HModelController,(ONE_FIXED>>2),HMSQT_MarineStand,MSSS_Minigun_Delta,ONE_FIXED,1);
	}

	/* for net game support */
	if(AvP.Network != I_No_Network)	{
		AddNetGameObjectID(dispPtr->ObStrategyBlock);
	}
}

/*
Function only does minimal disc setup , the rest will be done by the load function
*/
static STRATEGYBLOCK* InitialiseDiscBehaviour_ForLoad() {
	VECTORCH zeroVect = {0,0,0};

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
  	PC_PRED_DISC_BEHAV_BLOCK *bblk;
		
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourPredatorDisc_SeekTrack,&zeroVect);

	if (dispPtr == 0) return NULL;		 // Failed to allocate display block

	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		
 	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
 	
	dispPtr->ObStrategyBlock->DynPtr = dynPtr;


	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr = AllocateMem(sizeof(PC_PRED_DISC_BEHAV_BLOCK));
	bblk = dispPtr->ObStrategyBlock->SBdataptr;

	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}
	memset(dispPtr->ObStrategyBlock->SBdataptr,0,sizeof(PC_PRED_DISC_BEHAV_BLOCK));

	bblk->soundHandle = SOUND_NOACTIVEINDEX;

	dispPtr->HModelControlBlock=&bblk->HModelController;
	

	return dispPtr->ObStrategyBlock;
}

extern void NPCDiscBehaviour(STRATEGYBLOCK *sbPtr) 
{
	/* I have change this: mainly because npc preds fire them too... patrick */	
	#if 0
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    ONE_SHOT_BEHAV_BLOCK *bbPtr = (ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int destruct;

	destruct=0;

	while (reportPtr) {

		if (reportPtr->ObstacleSBPtr!=Player->ObStrategyBlock) {

			if (reportPtr->ObstacleSBPtr==NULL)
			{
				destruct=1;
			}
			else
			{
				CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
			}

		}
		reportPtr=reportPtr->NextCollisionReportPtr;		
	}

	if (destruct || (bbPtr->counter<=0) )
    {        
        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_PRED_DISC].MaxRange,
			&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_PRED_DISC].ExplosionIsFlat
		);

    Sound_Play(SID_EXPLOSION,"d",&(dynPtr->Position));
    	
			
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);

    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
	}
	#endif
	
	
	
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    ONE_SHOT_BEHAV_BLOCK *bbPtr = (ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	/* check for a collision with something */
	if(bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if(reportPtr)
	{
  		if(reportPtr->ObstacleSBPtr)
  		{
  			if(!((reportPtr->ObstacleSBPtr==Player->ObStrategyBlock)&&(AvP.PlayerType==I_Predator))) {
				CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
			}
		}

		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	DestroyAnyStrategyBlock(sbPtr);	
	} else {
		bbPtr->counter -= NormalFrameTime;
	}
}


static void InitialiseAlienSpitBehaviour(void)
{
	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	VECTORCH position;
	int numGlobules;

	/* calculate the position */
	{
//	 	position=PlayersWeapon.ObWorld;
		/* KJL 12:31:39 8/11/97 - the spit was being created to far in front
		of the player */
	 	position=Player->ObWorld;
  	}
	
	#define NUM_ALIENSPITGLOBULES 7
	for(numGlobules=0;numGlobules<NUM_ALIENSPITGLOBULES;numGlobules++)
	{
		/* make displayblock with correct shape, etc */
		dispPtr = MakeObject(I_BehaviourAlienSpit,&position);
		if (dispPtr == 0) return;		 // Failed to allocate display block

		/* make displayblock a dynamic module object */
		dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
		
		/* setup dynamics block */
		dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		
		if (dynPtr == 0) 
		{
			// Failed to allocate a dynamics block
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}
		
		dispPtr->ObStrategyBlock->DynPtr = dynPtr;

		/* spit does not collide with other spit */
		dynPtr->IgnoreSameObjectsAsYou = 1;

		/* give missile a maximum lifetime */
		dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK));
	
		if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
		{
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}

		((ONE_SHOT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
				
		/* align rocket to launcher */
		dynPtr->Position=position;
		/* align rocket to launcher */
		{
		    #define SPIT_SPEED 10000
		    #define SPIT_SPREAD 340

			extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
			VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
			MATRIXCH spitDirn;
			EULER spitOffset;			 

			dynPtr->OrientMat = VDBPtr->VDB_Mat;
			TransposeMatrixCH(&dynPtr->OrientMat);

			spitOffset.EulerX = (FastRandom()%(SPIT_SPREAD<<1))-SPIT_SPREAD;
			if(spitOffset.EulerX<0)	spitOffset.EulerX += 4096;
			spitOffset.EulerY = (FastRandom()%(SPIT_SPREAD<<1))-SPIT_SPREAD;
			if(spitOffset.EulerY<0)	spitOffset.EulerY += 4096;
			spitOffset.EulerZ = 0;
			
			CreateEulerMatrix(&spitOffset, &spitDirn);
			TransposeMatrixCH(&spitDirn);			
			MatrixMultiply(&dynPtr->OrientMat,&spitDirn,&dynPtr->OrientMat);

			/* align velocity to z axis */	
		    dynPtr->PrevOrientMat = dynPtr->OrientMat;
		    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->OrientMat.mat31,SPIT_SPEED)+
		    	Player->ObStrategyBlock->DynPtr->LinVelocity.vx+
		    	Player->ObStrategyBlock->DynPtr->LinImpulse.vx;
		    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->OrientMat.mat32,SPIT_SPEED)+
		    	Player->ObStrategyBlock->DynPtr->LinVelocity.vy+
		    	Player->ObStrategyBlock->DynPtr->LinImpulse.vy;
		    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->OrientMat.mat33,SPIT_SPEED)+
		    	Player->ObStrategyBlock->DynPtr->LinVelocity.vz+
		    	Player->ObStrategyBlock->DynPtr->LinImpulse.vz;
		
			/* dynamics blocks flags */
			dynPtr->GravityOn = 1;
		
		}	
		/* I added this next line for networking: Patrick */
		MatrixToEuler(&PlayersWeapon.ObMat, &PlayersWeapon.ObEuler);
		
		/* for net game support */
		if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);
	}
}

extern void AlienSpitBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    ONE_SHOT_BEHAV_BLOCK *bbPtr = (ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr;

	if (bbPtr->counter<=0)
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);
	}
	else if (reportPtr)
    {       
		STRATEGYBLOCK *hitSBPtr = reportPtr->ObstacleSBPtr;

		/* ignore hitting another spit or aliens */
		if(hitSBPtr)
		{
			if(!((hitSBPtr->I_SBtype==I_BehaviourAlienSpit)
			  ||(hitSBPtr->I_SBtype==I_BehaviourAlien)
			  ||(hitSBPtr==Player->ObStrategyBlock) ) )
				CauseDamageToObject(hitSBPtr,&TemplateAmmo[AMMO_ALIEN_SPIT].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);			  	
		}

   		Sound_Play(SID_PRED_NEWROAR,"d",&(dynPtr->Position));

		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
   		DestroyAnyStrategyBlock(sbPtr);
    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
	}
}
			

static void GetGunDirection(VECTORCH *gunDirectionPtr, VECTORCH *positionPtr)
{
 	gunDirectionPtr->vx = PlayersTarget.Position.vx - positionPtr->vx;
 	gunDirectionPtr->vy = PlayersTarget.Position.vy - positionPtr->vy;
 	gunDirectionPtr->vz = PlayersTarget.Position.vz - positionPtr->vz;
	Normalise(gunDirectionPtr);
}

static int Reflect(VECTORCH *Incident, VECTORCH *Normal, EULER *Output) {
	
	int dot,retval;
	VECTORCH outVec,normInc;
	MATRIXCH tempMat;
	/* Ah, the wonders of better math support. */

	GLOBALASSERT(Incident);
	GLOBALASSERT(Normal);
	GLOBALASSERT(Output);

	normInc=*Incident;
	Normalise(&normInc);
	retval = DotProduct(&normInc,Normal);
	/* Hold that thought. */
	dot = retval*(-2);
	/* Yeah, okay, and a better algorithm. */	
	outVec.vx = (normInc.vx + MUL_FIXED(dot,Normal->vx));
	outVec.vy = (normInc.vy + MUL_FIXED(dot,Normal->vy));
	outVec.vz = (normInc.vz + MUL_FIXED(dot,Normal->vz));

	MakeMatrixFromDirection(&outVec,&tempMat);
	MatrixToEuler(&tempMat,Output);
	/* But bear in mind, most of the early one was coping with *
	 * bad functions and junk inputs... that's my excuse.      */
	
	return(retval);
}

static int SBForcesBounce(STRATEGYBLOCK *sbPtr) {

	if (sbPtr==NULL) {
		return(0);
	}

	/* Now switch by type. */
	switch (sbPtr->I_SBtype) {
		case I_BehaviourInanimateObject:
			{
				INANIMATEOBJECT_STATUSBLOCK* objectStatusPtr;
				objectStatusPtr=(INANIMATEOBJECT_STATUSBLOCK *)sbPtr->SBdataptr;
				GLOBALASSERT(objectStatusPtr);
				if (objectStatusPtr->Indestructable) {
					return(1);
				} else {
					return(0);
				}
			}
			break;
		case I_BehaviourProximityDoor:
		case I_BehaviourTrackObject:
		case I_BehaviourLiftDoor:
		case I_BehaviourSwitchDoor:
		case I_BehaviourLinkSwitch:
		case I_BehaviourBinarySwitch:
		case I_BehaviourLift:
		case I_BehaviourPlatform:
		case I_BehaviourPredatorDisc_SeekTrack:
			return(1);
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=sbPtr->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourPredatorDisc_SeekTrack:
						return(1);
						break;
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
	
	return(0);
}

int SBIsEnvironment(STRATEGYBLOCK *sbPtr) {

	if (sbPtr==NULL) {
		return(1);
	}

	if (sbPtr->SBdptr) {
		if (sbPtr->SBdptr->ObMyModule) {
			return(1);
		}
	}

	return(0);
}

void Frisbee_Hit_Environment(STRATEGYBLOCK *sbPtr,COLLISIONREPORT *reportPtr) {
	
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    FRISBEE_BEHAV_BLOCK *fbPtr = (FRISBEE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	MATRIXCH mat;

	/* Hit the environment.  Bounce? */
	int dp;

	dp=Reflect(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal, &dynPtr->OrientEuler);
	dynPtr->OrientEuler.EulerZ=0;
	dynPtr->IgnoreThePlayer=0;

	/* Always bounce.  Reference Disc_Hit_Environment for sticking conditions. */
	MakeImpactSparks(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal,&dynPtr->Position);
	Sound_Play(SID_ED_SKEETERDISC_HITWALL,"dp",&(dynPtr->Position),((FastRandom()&511)-255));

	CreateEulerMatrix(&dynPtr->OrientEuler, &mat);
	TransposeMatrixCH(&mat);

	dynPtr->OrientMat=mat;

	dynPtr->LinVelocity.vx = MUL_FIXED(mat.mat31,FRISBEE_SPEED);
	dynPtr->LinVelocity.vy = MUL_FIXED(mat.mat32,FRISBEE_SPEED);
	dynPtr->LinVelocity.vz = MUL_FIXED(mat.mat33,FRISBEE_SPEED);

	dynPtr->LinImpulse.vx=0;
	dynPtr->LinImpulse.vy=0;
	dynPtr->LinImpulse.vz=0;

	/*
	Record that the disc has bounced - for use in network game
	*/
	fbPtr->Bounced=1;
	fbPtr->bounces++;

}

void Disc_Hit_Environment(STRATEGYBLOCK *sbPtr,COLLISIONREPORT *reportPtr) {
	
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    PC_PRED_DISC_BEHAV_BLOCK *bbPtr = (PC_PRED_DISC_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	MATRIXCH mat;

	/* Hit the environment.  Bounce? */
	int dp;

	dp=Reflect(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal, &dynPtr->OrientEuler);
	dynPtr->OrientEuler.EulerZ=0;
	dynPtr->IgnoreThePlayer=0;

	if ((dp>-46341)&&(bbPtr->counter>0)&&(bbPtr->bounces<=DISC_MAX_BOUNCES)) { /* 65536/Rt2 */
		/* Bounce. */
		MakeImpactSparks(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal,&dynPtr->Position);
		Sound_Play(SID_PREDATOR_DISK_HITTING_WALL,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
		/*
		Record that the disc has bounced - for use in network game
		*/
		bbPtr->Bounced=1;
		bbPtr->bounces++;
	} else {
		CreateEulerMatrix(&dynPtr->OrientEuler, &mat);
		TransposeMatrixCH(&mat);
		/* very steep angle (or very long flight!) - stick. */
		bbPtr->Stuck=1;
		bbPtr->HModelController.Playing=0;
		MakeSprayOfSparks(&mat,&dynPtr->Position);
		Sound_Stop(bbPtr->soundHandle);
		Sound_Play(SID_DISC_STICKSINWALL,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
	}

	if (bbPtr->Target==NULL) {
		/* No target, so come home. */
		bbPtr->Target=Player->ObStrategyBlock;
		COPY_NAME(bbPtr->Target_SBname,bbPtr->Target->SBname);
	}

}

extern void DiscBehaviour_SeekTrack(STRATEGYBLOCK *sbPtr) 
{
	
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    PC_PRED_DISC_BEHAV_BLOCK *bbPtr = (PC_PRED_DISC_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	MATRIXCH mat;
	int getadisc;

	textprint("Disc active!\n");

	ProveHModel_Far(&bbPtr->HModelController,sbPtr);

	if (sbPtr->SBDamageBlock.IsOnFire) {
		sbPtr->SBDamageBlock.IsOnFire=0;
	}

	/* Update target */
	if (bbPtr->Stuck) {
		/* No business doing anything. */
		#if 0
		bbPtr->HModelController.Playing=0;
		if (sbPtr->SBdptr) {
			sbPtr->SBdptr->ObFlags3 &= ~ObFlag3_DynamicModuleObject;
		}
		#else
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
	
		dynPtr->LinImpulse.vx = 0;
		dynPtr->LinImpulse.vy = 0;
		dynPtr->LinImpulse.vz = 0;

		Sound_Stop(bbPtr->soundHandle);
		/* Are we inside the player? */
		{
			extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
			VECTORCH offset;
			int dist;

			offset=Global_VDB_Ptr->VDB_World;
			offset.vx-=dynPtr->Position.vx;
			offset.vy-=dynPtr->Position.vy;
			offset.vz-=dynPtr->Position.vz;

			dist=Approximate3dMagnitude(&offset);

			if (dist<600) {
				/* My, that's close. */
				PLAYER_STATUS *psptr;
				int a;
				psptr=(PLAYER_STATUS *)Player->ObStrategyBlock->SBdataptr;
				for (a=0; a<MAX_NO_OF_WEAPON_SLOTS; a++) {
					if (psptr->WeaponSlot[a].WeaponIDNumber==WEAPON_PRED_DISC) {
						break;
					}
				}
				if (a!=MAX_NO_OF_WEAPON_SLOTS) {
					
					if ((psptr->WeaponSlot[a].PrimaryMagazinesRemaining==0)
						&& (psptr->WeaponSlot[a].PrimaryRoundsRemaining==0)) {
						//psptr->WeaponSlot[a].PrimaryRoundsRemaining+=ONE_FIXED;
						psptr->WeaponSlot[a].PrimaryMagazinesRemaining+=1;
						/* Autoswap to disc here? */
						AutoSwapToDisc();
					} else {
						psptr->WeaponSlot[a].PrimaryMagazinesRemaining+=1;
					}

					#if 0
					NewOnScreenMessage("CAUGHT DISC");
					#endif

					Sound_Stop(bbPtr->soundHandle);
					Sound_Play(SID_PREDATOR_DISK_BEING_CAUGHT,"h");

					if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

			    	DestroyAnyStrategyBlock(sbPtr);	

					return;
				}
			}
		}
		/* Just to make sure... */
		Convert_Disc_To_Pickup(sbPtr);
		return;
		#endif
	} else if (bbPtr->Target==Player->ObStrategyBlock) {
		VECTORCH targetPos;
		/* Home on the player. */
		textprint("Disc homing on player.\n");
		GetTargetingPointOfObject_Far(bbPtr->Target,&targetPos);
		if (bbPtr->counter>0) {
	  		EulerAnglesHoming(&dynPtr->Position,&targetPos,&dynPtr->OrientEuler,4);
		} else {
			textprint("Disc super homing!\n");
	  		EulerAnglesHoming(&dynPtr->Position,&targetPos,&dynPtr->OrientEuler,2);
		}
  	} else if (bbPtr->Target) {
		/* We have a target. */
		textprint("Disc homing on target.\n");
		if (NAME_ISEQUAL(bbPtr->Target_SBname,bbPtr->Target->SBname)) {
			if (!NPC_IsDead(bbPtr->Target)) {
				/* Our target lives! */
				VECTORCH targetPos;
				GLOBALASSERT(bbPtr->Target->DynPtr);
				GetTargetingPointOfObject_Far(bbPtr->Target,&targetPos);
		  		EulerAnglesHoming(&dynPtr->Position,&targetPos,&dynPtr->OrientEuler,4);
			} else {
				/* Target dying - unset. */
				bbPtr->Target=NULL;
			}
		} else {
			/* Target no longer valid - unset. */
			bbPtr->Target=NULL;
		}
	} else {
		/* No target.  Oh well... */
		textprint("Disc in free flight.\n");
	}

	if (bbPtr->Stuck==0) {
		/* We must be flying.  Maintain sound. */
		if(bbPtr->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Update3d(bbPtr->soundHandle,&(sbPtr->DynPtr->Position));
			if (ActiveSounds[bbPtr->soundHandle].soundIndex!=SID_PREDATOR_DISK_FLYING) {
				Sound_Stop(bbPtr->soundHandle);
			 	Sound_Play(SID_PREDATOR_DISK_FLYING,"del",&(sbPtr->DynPtr->Position),&bbPtr->soundHandle);
			}
		} else {
		 	Sound_Play(SID_PREDATOR_DISK_FLYING,"del",&(sbPtr->DynPtr->Position),&bbPtr->soundHandle);
		}
	}

	/* check for a collision with something */
	if(bbPtr->counter <= 0) 
	{
		#if 0
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
		return;
		#else
		/* For now, do nothing... */
		if (bbPtr->counter<-DISC_LIFETIME) {
			if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

			DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
			return;
		}
		#endif
	}
	
	getadisc=0;
	/* To make sure you can't get multiple discs back! */

	while (reportPtr) {
		/* Should be while? */
  		if(reportPtr->ObstacleSBPtr)
  		{
  			/* Hit a strategyblock. */

			if (bbPtr->Stuck) {
				/* Don't hurt anyone. */
				if ((reportPtr->ObstacleSBPtr==Player->ObStrategyBlock)&&(AvP.PlayerType==I_Predator)) {
					/* Hit the owner.  Recover it! */
					PLAYER_STATUS *psptr;
					int a;
					psptr=(PLAYER_STATUS *)Player->ObStrategyBlock->SBdataptr;
					for (a=0; a<MAX_NO_OF_WEAPON_SLOTS; a++) {
						if (psptr->WeaponSlot[a].WeaponIDNumber==WEAPON_PRED_DISC) {
							break;
						}
					}
					if (a!=MAX_NO_OF_WEAPON_SLOTS) {
						
						getadisc=1;
	
						#if 0
						NewOnScreenMessage("RECOVERED DISC");
						#endif
	
						Sound_Stop(bbPtr->soundHandle);
						Sound_Play(SID_PREDATOR_DISK_RECOVERED,"h");

						if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

				    	DestroyAnyStrategyBlock(sbPtr);	
	
					}
				}
  			} else if(!((reportPtr->ObstacleSBPtr==Player->ObStrategyBlock)&&(AvP.PlayerType==I_Predator))) {

				/* Hit a random strategyblock - what is it? */
				if (SBForcesBounce(reportPtr->ObstacleSBPtr)) {

					/* Bounce. */
					Reflect(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal, &dynPtr->OrientEuler);
					dynPtr->OrientEuler.EulerZ=0;
					dynPtr->IgnoreThePlayer=0;
					MakeImpactSparks(&dynPtr->LinVelocity, &reportPtr->ObstacleNormal,&dynPtr->Position);
					Sound_Play(SID_PREDATOR_DISK_HITTING_WALL,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
					bbPtr->bounces++;

					/*
					Record that the disc has bounced - for use in network game
					*/
					bbPtr->Bounced=1;
			
				} else if (SBIsEnvironment(reportPtr->ObstacleSBPtr)) {
					Disc_Hit_Environment(sbPtr,reportPtr);
				} else {
					/* Hit a creature? */
					SECTION_DATA *hit_section;
				 	VECTORCH attack_dir;
					
					dynPtr->IgnoreThePlayer=0;
					/* To make sure. */

					hit_section=NULL;
					
				 	GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
			
					switch (reportPtr->ObstacleSBPtr->I_SBtype)
					{
						case I_BehaviourMarine:
						case I_BehaviourAlien:
						{
							SECTION_DATA *chest_section=0;
							DISPLAYBLOCK *objectPtr = reportPtr->ObstacleSBPtr->SBdptr;
		
							if(objectPtr)
							{
								SECTION_DATA *firstSectionPtr;
		
							  	firstSectionPtr=objectPtr->HModelControlBlock->section_data;
							  	LOCALASSERT(firstSectionPtr);
								LOCALASSERT(firstSectionPtr->flags&section_data_initialised);
		
								/* look for the object's torso in preference */
								chest_section =GetThisSectionData(objectPtr->HModelControlBlock->section_data,"chest");
							
								if (chest_section)
								{
									VECTORCH rel_pos;
		
									rel_pos=dynPtr->Position;
									
									rel_pos.vx-=chest_section->World_Offset.vx;
									rel_pos.vy-=chest_section->World_Offset.vy;
									rel_pos.vz-=chest_section->World_Offset.vz;
		
									Normalise(&rel_pos);
									
									CauseDamageToHModel(objectPtr->HModelControlBlock,reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED, chest_section,&rel_pos,&chest_section->World_Offset,0);
								}
								else
								{
									CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
								}
							}
							break;
						}
					case I_BehaviourAutoGun:
					case I_BehaviourXenoborg:
						{
							/* Spark a bit? */
							CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
							break;
						}
					case I_BehaviourNetGhost:
						{
							NETGHOSTDATABLOCK *dataptr;
							dataptr=reportPtr->ObstacleSBPtr->SBdataptr;
							switch (dataptr->type) {
								case I_BehaviourMarinePlayer:
								case I_BehaviourAlienPlayer:
								case I_BehaviourPredatorPlayer:
									/* Maybe a different damage here? */
									CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
									break;
								default:
									CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
									break;
							}
						}
						break;
					default:
						{
							CauseDamageToObject(reportPtr->ObstacleSBPtr,&TemplateAmmo[AMMO_PRED_DISC].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
							break;
						}
					}

					Sound_Play(SID_PREDATOR_DISK_HITTING_TARGET,"dp",&(dynPtr->Position),((FastRandom()&511)-255));

					if (NAME_ISEQUAL(reportPtr->ObstacleSBPtr->SBname,bbPtr->Target_SBname)) {
						/* Got him!  Seek the player. */
						bbPtr->Target=Player->ObStrategyBlock;
						bbPtr->counter = DISC_LIFETIME;
						COPY_NAME(bbPtr->Target_SBname,bbPtr->Target->SBname);
					}
				}
			} else {
				/* Hit the owner.  Catch it! */
				PLAYER_STATUS *psptr;
				int a;
				psptr=(PLAYER_STATUS *)Player->ObStrategyBlock->SBdataptr;
				for (a=0; a<MAX_NO_OF_WEAPON_SLOTS; a++) {
					if (psptr->WeaponSlot[a].WeaponIDNumber==WEAPON_PRED_DISC) {
						break;
					}
				}
				if (a!=MAX_NO_OF_WEAPON_SLOTS) {

					getadisc=1;

					#if 0
					NewOnScreenMessage("CAUGHT DISC");
					#endif

					Sound_Stop(bbPtr->soundHandle);
					Sound_Play(SID_PREDATOR_DISK_BEING_CAUGHT,"h");

					if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

			    	DestroyAnyStrategyBlock(sbPtr);	

				}
			}
		} else {
			Disc_Hit_Environment(sbPtr,reportPtr);
		}

		/* skip to next report */
		reportPtr = reportPtr->NextCollisionReportPtr;

	} 

	if (getadisc) {
		PLAYER_STATUS *psptr;
		int a;
		psptr=(PLAYER_STATUS *)Player->ObStrategyBlock->SBdataptr;
		for (a=0; a<MAX_NO_OF_WEAPON_SLOTS; a++) {
			if (psptr->WeaponSlot[a].WeaponIDNumber==WEAPON_PRED_DISC) {
				break;
			}
		}
		if (a!=MAX_NO_OF_WEAPON_SLOTS) {
			
			if ((psptr->WeaponSlot[a].PrimaryMagazinesRemaining==0)
				&& (psptr->WeaponSlot[a].PrimaryRoundsRemaining==0)) {
				psptr->WeaponSlot[a].PrimaryRoundsRemaining+=ONE_FIXED;
				/* Autoswap to disc here? */
				AutoSwapToDisc();
			} else {
				psptr->WeaponSlot[a].PrimaryMagazinesRemaining+=1;
			}
		}
	}

	#if 0
	else {
		/* We must be in the clear! */
		#if 0
		dynPtr->IgnoreThePlayer=0;
		#endif
	}
	#endif
	
	if (bbPtr->Stuck) {
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
	
		dynPtr->LinImpulse.vx = 0;
		dynPtr->LinImpulse.vy = 0;
		dynPtr->LinImpulse.vz = 0;
		
	} else {
		/* Decrement Timer.. */
	
		bbPtr->counter -= NormalFrameTime;
	
		if (bbPtr->Target==NULL) {
			if (bbPtr->counter<=0) {
				/* Turn around! */
				bbPtr->Target=Player->ObStrategyBlock;
				bbPtr->counter = DISC_LIFETIME;
				COPY_NAME(bbPtr->Target_SBname,bbPtr->Target->SBname);
				dynPtr->IgnoreThePlayer=0;
			}
		}
	
		/* Move Disc */
	
		CreateEulerMatrix(&dynPtr->OrientEuler, &mat);
	
		TransposeMatrixCH(&mat);
	
		dynPtr->OrientMat=mat;
	
		dynPtr->LinVelocity.vx = MUL_FIXED(mat.mat31,DISC_SPEED);
		dynPtr->LinVelocity.vy = MUL_FIXED(mat.mat32,DISC_SPEED);
		dynPtr->LinVelocity.vz = MUL_FIXED(mat.mat33,DISC_SPEED);
	
		dynPtr->LinImpulse.vx=0;
		dynPtr->LinImpulse.vy=0;
		dynPtr->LinImpulse.vz=0;
		NewTrailPoint(sbPtr->DynPtr);
	}		  
}

void SetEulerAngles(VECTORCH *source, VECTORCH *Target, EULER *eulr) {

	int offsetx,offsety,offsetz;

	offsetx=(Target->vx)-(source->vx);
	offsety=(Target->vz)-(source->vz);
	 
	eulr->EulerY=ArcTan(offsetx,offsety);
	eulr->EulerY&=wrap360;
	
	/* That was for the first plane. Now the second. */

	offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
	offsety=-((Target->vy)-(source->vy));

	eulr->EulerX=ArcTan(offsety,offsetz);
	eulr->EulerX&=wrap360;

	eulr->EulerZ=0;

}

void EulerAnglesHoming(VECTORCH *source, VECTORCH *Target, EULER *eulr, int rate) {

	int offsetx,offsety,offsetz,angle1,angle2,testangle;

	offsetx=(Target->vx)-(source->vx);
	offsety=(Target->vz)-(source->vz);
	 
	angle1=ArcTan(offsetx,offsety);
	 
	angle2=eulr->EulerY;
	 
	if (angle1!=angle2) {
	
		testangle=angle2-angle1;
		if (abs(testangle)<(NormalFrameTime>>rate)) {
			eulr->EulerY=angle1;
			eulr->EulerY&=wrap360;
		} else if ( ((testangle>0) && (testangle<deg180)) || (testangle<-deg180) ) {
			eulr->EulerY-=(NormalFrameTime>>rate);
			eulr->EulerY&=wrap360;
		} else {
			eulr->EulerY+=(NormalFrameTime>>rate);
			eulr->EulerY&=wrap360;
		}

	}

	/* That was for the first plane. Now the second. */

	offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
	offsety=-((Target->vy)-(source->vy));

	angle1=ArcTan(offsety,offsetz);
	angle2=eulr->EulerX;

	if (angle1!=angle2) {
		testangle=angle2-angle1;
		if (abs(testangle)<(NormalFrameTime>>rate)) {
			eulr->EulerX=angle1;
			eulr->EulerX&=wrap360;
		} else if ( ((testangle>0) && (testangle<deg180)) || (testangle<-deg180) ) {
			eulr->EulerX-=(NormalFrameTime>>rate);
			eulr->EulerX&=wrap360;
		} else {
			eulr->EulerX+=(NormalFrameTime>>rate);
			eulr->EulerX&=wrap360;
		}
	}
	
}

int PredDisc_TargetFilter(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourAlien:
			{
				ALIEN_STATUS_BLOCK *alienStatusPointer;
				LOCALASSERT(candidate);	
				LOCALASSERT(candidate->DynPtr);	
			
				alienStatusPointer=(ALIEN_STATUS_BLOCK *)(candidate->SBdataptr);    
				
				if (alienStatusPointer->BehaviourState==ABS_Dying) {
					return(0);
				} else {
					return(1);
				}
				break;
			}
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
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
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
						return(1);
						break;
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

void PredDisc_GetFirstTarget(PC_PRED_DISC_BEHAV_BLOCK *bptr, DISPLAYBLOCK *target, VECTORCH *position) {

	int a;

	if (target!=NULL) {
		if (target->ObStrategyBlock!=NULL) {
			if (PredDisc_TargetFilter(target->ObStrategyBlock)) {
				/* Valid. */
				bptr->Target=target->ObStrategyBlock;
				COPY_NAME(bptr->Target_SBname,target->ObStrategyBlock->SBname);
				return;
			}
		}
	}

	/* Second try. */

	bptr->Target=PredDisc_GetNewTarget(bptr,position,NULL,2);
	if (bptr->Target!=NULL) {
		COPY_NAME(bptr->Target_SBname,bptr->Target->SBname);
		return;
	}

	/* Failed! */

	bptr->Target=NULL;
	{
		for (a=0; a<SB_NAME_LENGTH; a++) {
			bptr->Target_SBname[a]='\0';
		}
	}

}

int ObjectIsOnScreen(DISPLAYBLOCK *object) {

	int a;
	extern DISPLAYBLOCK *OnScreenBlockList[];
	extern int NumOnScreenBlocks;
	
	for (a=0; a<NumOnScreenBlocks; a++) {
		if (OnScreenBlockList[a]==object) break;
	}
	
	if (a==NumOnScreenBlocks) return(0); else return(1);
	
}

#define DISC_PROX_RANGE 90000
	
STRATEGYBLOCK *PredDisc_GetNewTarget(PC_PRED_DISC_BEHAV_BLOCK *bptr,VECTORCH *discpos, STRATEGYBLOCK *prevtarg, int mine) {

	int a,neardist;
	STRATEGYBLOCK *nearest;
	STRATEGYBLOCK *candidate;
	MODULE *dmod;
	
	dmod=ModuleFromPosition(discpos,playerPherModule);
	
	LOCALASSERT(dmod);
	
	nearest=NULL;
	neardist=ONE_FIXED;
	
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if ((candidate!=prevtarg)&&(candidate!=Player->ObStrategyBlock)) {
			if (candidate->DynPtr) {
				if (PredDisc_TargetFilter(candidate)) {
					VECTORCH offset;
					int dist;
		
					offset.vx=discpos->vx-candidate->DynPtr->Position.vx;
					offset.vy=discpos->vy-candidate->DynPtr->Position.vy;
					offset.vz=discpos->vz-candidate->DynPtr->Position.vz;
			
					dist=Approximate3dMagnitude(&offset);
		
					if (! ((mine==1)&&dist>DISC_PROX_RANGE)) {
						if (dist<neardist) {
							/* Check visibility? */
							if (candidate->SBdptr) {
								if (!NPC_IsDead(candidate)) {
									/* Not the last one again! */
									if (!NAME_ISEQUAL(bptr->Prev_Target_SBname,candidate->SBname)) {
										if (mine==1) {
											if (IsThisObjectVisibleFromThisPosition(candidate->SBdptr,discpos,DISC_PROX_RANGE) ) {
												nearest=candidate;
											}
										} else if (mine==2) {
											if (ObjectIsOnScreen(candidate->SBdptr)) {
												nearest=candidate;
											}
										} else if (candidate->containingModule) {
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
			}
		}
	}
	
	return(nearest);

}



void NukeObject(STRATEGYBLOCK *sbPtr)
{
	CauseDamageToObject(sbPtr, &certainDeath, ONE_FIXED,NULL);
}



DISPLAYBLOCK *SpawnMolotovCocktail(SECTION_DATA *root, MATRIXCH *master_orient)
{

	DISPLAYBLOCK *dispPtr;
	STRATEGYBLOCK *sbPtr;
	MODULEMAPBLOCK *mmbptr;
	MODULE m_temp;
	AVP_BEHAVIOUR_TYPE bhvr;
	int woundflags;

	//if( (NumActiveBlocks > maxobjects-5) || (NumActiveStBlocks > maxstblocks-5)) return NULL;

	// 1. Set up shape data BEFORE making the displayblock,
	// since "AllocateModuleObject()" will fill in shapeheader
	// information and extent data

	mmbptr = &TempModuleMap;
               
	/* Doesn't really matter what shape gets generated... */
	//CreateShapeInstance(mmbptr,root->sempai->ShapeName);
	CreateShapeInstance(mmbptr,"Shell");

	bhvr = I_BehaviourMolotov;

	// And allocate the modulemapblock object

	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
	m_temp.m_mapptr = mmbptr;
	m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
	m_temp.m_dptr = NULL;
	AllocateModuleObject(&m_temp);    
	dispPtr = m_temp.m_dptr;
	if(dispPtr==NULL) return (DISPLAYBLOCK *)0; /* patrick: cannot create displayblock, so just return 0 */

	dispPtr->ObMyModule = NULL;     /* Module that created us */

	if(root) //allow case root==NULL for loading
	{
		dispPtr->ObWorld = root->World_Offset;
	}

	sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
  
	if (sbPtr == 0) return (DISPLAYBLOCK *)0; // Failed to allocate a strategy block

	// 2. NOW set up the strategyblock-specific fields for
	// the new displayblock. We won't go through the "AttachNew
	// StrategyBlock" and "AssignRunTimeBehaviours" pair, since
	// the first switches on ObShape and the second on bhvr;
	// but, in this case, there isn't a particular connection
	// between them.

	sbPtr->I_SBtype = bhvr;

	{
		DYNAMICSBLOCK *dynPtr;
	
		sbPtr->SBdataptr = (MOLOTOV_BEHAV_BLOCK *) AllocateMem(sizeof(MOLOTOV_BEHAV_BLOCK));
		if (sbPtr->SBdataptr == 0) {	
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(sbPtr);
			return (DISPLAYBLOCK*)NULL;
		}
			
		((MOLOTOV_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = 65536;//32767;//FastRandom()&32767;
		
		if(root) //allow case root==NULL for loading
		{
			woundflags=Splice_HModels(&(((MOLOTOV_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),root);
			InitHModelSequence( &(((MOLOTOV_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),0,1,ONE_FIXED);
		}

		dispPtr->HModelControlBlock=&(((MOLOTOV_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController);
		
		if(root) //allow case root==NULL for loading
		{
			dispPtr->ObWorld=root->World_Offset;
			dispPtr->ObMat=root->SecMat;
		}

		LOCALASSERT(dispPtr->ObWorld.vx<1000000 && dispPtr->ObWorld.vx>-1000000);
		LOCALASSERT(dispPtr->ObWorld.vy<1000000 && dispPtr->ObWorld.vy>-1000000);
		LOCALASSERT(dispPtr->ObWorld.vz<1000000 && dispPtr->ObWorld.vz>-1000000);

		ProveHModel(dispPtr->HModelControlBlock,dispPtr);

		dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_GRENADE);
			
		if (dynPtr == 0) {
			// Failed to allocate a dynamics block
			RemoveBehaviourStrategy(sbPtr);
			return (DISPLAYBLOCK*)NULL;
		}
		
		if(root) //allow case root==NULL for loading
		{
			dynPtr->Position = root->World_Offset;

			dynPtr->OrientMat=root->SecMat;
		}

		// Give explosion fragments an angular velocity
		dynPtr->AngVelocity.EulerX = (FastRandom()&2047)-1024;
		dynPtr->AngVelocity.EulerY = (FastRandom()&2047)-1024;
		dynPtr->AngVelocity.EulerZ = (FastRandom()&2047)-1024;

		dynPtr->LinVelocity.vx=0;
		dynPtr->LinVelocity.vy=0;
		dynPtr->LinVelocity.vz=0;

		dynPtr->IgnoreThePlayer=0;

		{
			/* Handle velocity... */
			//VECTORCH start={0,2000,12000};
			MATRIXCH tm=*master_orient;
			VECTORCH start;

			start.vx=mx;
			start.vy=my;
			start.vz=mz;
				
			RotateAndCopyVector(&start,&dynPtr->LinImpulse,&tm);

		}

	}

                    
    return dispPtr;
	
}
extern void MolotovBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    MOLOTOV_BEHAV_BLOCK *bbPtr = (MOLOTOV_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	int explodeNow = 0;
	
	//Work out the containing module now , since it doesn't seem to get done anywhere else
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), sbPtr->containingModule);
	
	/* explode if the grenade touches an alien */
	while (reportPtr)
	{
		STRATEGYBLOCK *sbPtr = reportPtr->ObstacleSBPtr;

		if(sbPtr)
		{
			if((sbPtr->I_SBtype == I_BehaviourAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourMarinePlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourAlienPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorPlayer)
			 ||(sbPtr->I_SBtype == I_BehaviourPredator)
			 ||(sbPtr->I_SBtype == I_BehaviourXenoborg)
			 ||(sbPtr->I_SBtype == I_BehaviourMarine)
			 ||(sbPtr->I_SBtype == I_BehaviourQueenAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourPredatorAlien)
			 ||(sbPtr->I_SBtype == I_BehaviourFaceHugger))
			{
				explodeNow = 1; /* kaboom */
				//explodeNow = 0; /* kaboom */
			}
		
			if(sbPtr->I_SBtype == I_BehaviourNetGhost)
			{ 
				NETGHOSTDATABLOCK *ghostData = sbPtr->SBdataptr;
				LOCALASSERT(ghostData);
				LOCALASSERT(AvP.Network!=I_No_Network);

				if((ghostData->type == I_BehaviourMarinePlayer)||
			   		(ghostData->type == I_BehaviourPredatorPlayer)||
			   		(ghostData->type == I_BehaviourAlienPlayer))
				{
			   		explodeNow = 1;
				}				
			}
		} else {	
			/* What the hell! */
			explodeNow=1;
		}
							
		/* skip to next report */
		reportPtr = reportPtr->NextCollisionReportPtr;
	}														
   
	if ((bbPtr->counter<=0) || explodeNow)
    {        
        /* KJL 17:51:56 12/17/96 - make explosion damage other objects */    
		HandleEffectsOfExplosion
		(
			sbPtr,
			&(dynPtr->Position),
			TemplateAmmo[AMMO_MOLOTOV].MaxRange,
	 		&TemplateAmmo[AMMO_MOLOTOV].MaxDamage[AvP.Difficulty],
			TemplateAmmo[AMMO_MOLOTOV].ExplosionIsFlat
		);
		
		if (sbPtr->containingModule) {
			Explosion_SoundData.position=dynPtr->Position;
		    Sound_Play(SID_ED_MOLOTOV_EXPLOSION,"n",&Explosion_SoundData);
    	}
 		
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		/* destroy rocket */
    	DestroyAnyStrategyBlock(sbPtr);
    }
	else
	{
		bbPtr->counter-=NormalFrameTime;
        DynamicallyRotateObject(dynPtr);
	}
}

void FirePredPistolFlechettes(VECTORCH *base_position,VECTORCH *base_offset,MATRIXCH *orientmat,int player, int *timer,BOOL damaging) {

	/* A cheap knock off of the flamethrower function, so as not to rock the boat. */

	VECTORCH position;

	(*timer)+=NormalFrameTime;

	while ((*timer)>=TIME_FOR_PREDPISTOLFLECHETTE) 
	{
		VECTORCH velocity, ratio;

		(*timer)-=TIME_FOR_PREDPISTOLFLECHETTE;
		
		/* Calculate velocity... */
		velocity.vx = ((FastRandom()&1023) - 512)*PREDPISTOL_SPREAD;//*2;
		velocity.vy = ((FastRandom()&1023) - 512)*PREDPISTOL_SPREAD;//*2;
		velocity.vz = ((FastRandom()&1023) + 200+1024)*16; // Was 511, 512.

		/* calculate the position */

		{
			int offset = MUL_FIXED(FastRandom()&32767,NormalFrameTime);	// Was 16383
	
			position=*base_offset;
			
			/* Make sure position corresponds to velocity direction... */
			ratio=velocity;
			Normalise(&ratio);
			position.vx += MUL_FIXED(offset,ratio.vx);
			position.vy += MUL_FIXED(offset,ratio.vy);
			position.vz += MUL_FIXED(offset,ratio.vz);

		}
	
		RotateVector(&position,orientmat);

		if (player) {
			Crunch_Position_For_Players_Weapon(&position);		
		} else {
			position.vx+=base_position->vx;
			position.vy+=base_position->vy;
			position.vz+=base_position->vz;
		}

		RotateVector(&velocity,orientmat);
		MakeParticle(&position,&(velocity),damaging ? PARTICLE_PREDPISTOL_FLECHETTE : PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING);
	}

}


/*-------------------**
** Load/Save Grenade **
**-------------------*/
typedef struct grenade_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int bouncelastframe;

	DYNAMICSBLOCK dynamics;
}GRENADE_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV behav

void LoadStrategy_Grenade(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	GRENADE_BEHAV_BLOCK* behav;
	GRENADE_SAVE_BLOCK* block = (GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grende
	sbPtr = CreateGrenadeKernel(I_BehaviourGrenade,&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter);
	COPYELEMENT_LOAD(bouncelastframe);

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_Grenade(STRATEGYBLOCK* sbPtr)
{
	GRENADE_BEHAV_BLOCK* behav;
	GRENADE_SAVE_BLOCK* block;

	behav = (GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter);
	COPYELEMENT_SAVE(bouncelastframe);

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}


/*-------------------**
** Load/Save Cluster Grenade **
**-------------------*/
typedef struct cluster_grenade_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int bouncelastframe;

	DYNAMICSBLOCK dynamics;
}CLUSTER_GRENADE_SAVE_BLOCK;


void LoadStrategy_ClusterGrenade(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	GRENADE_BEHAV_BLOCK* behav;
	CLUSTER_GRENADE_SAVE_BLOCK* block = (CLUSTER_GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grenade
	sbPtr = CreateGrenadeKernel(I_BehaviourClusterGrenade,&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter);
	COPYELEMENT_LOAD(bouncelastframe);

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_ClusterGrenade(STRATEGYBLOCK* sbPtr)
{
	GRENADE_BEHAV_BLOCK* behav;
	CLUSTER_GRENADE_SAVE_BLOCK* block;

	behav = (GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter);
	COPYELEMENT_SAVE(bouncelastframe);

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}


/*-------------------**
** Load/Save Flare Grenade **
**-------------------*/
typedef struct flare_grenade_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int LifeTimeRemaining;
	int ParticleGenerationTimer;

	DYNAMICSBLOCK dynamics;
}FLARE_GRENADE_SAVE_BLOCK;

void LoadStrategy_FlareGrenade(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	FLARE_BEHAV_BLOCK* behav;
	FLARE_GRENADE_SAVE_BLOCK* block = (FLARE_GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grende
	sbPtr = CreateGrenadeKernel(I_BehaviourFlareGrenade,&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (FLARE_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(LifeTimeRemaining)
	COPYELEMENT_LOAD(ParticleGenerationTimer)

	*sbPtr->DynPtr = block->dynamics;
	
	Load_SoundState(&behav->SoundHandle);
}

void SaveStrategy_FlareGrenade(STRATEGYBLOCK* sbPtr)
{
	FLARE_BEHAV_BLOCK* behav;
	FLARE_GRENADE_SAVE_BLOCK* block;

	behav = (FLARE_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(LifeTimeRemaining)
	COPYELEMENT_SAVE(ParticleGenerationTimer)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

	Save_SoundState(&behav->SoundHandle);

}

/*-------------------**
** Load/Save Prox Grenade **
**-------------------*/
typedef struct prox_grenade_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int LifeTimeRemaining;
	int SoundGenerationTimer;

	DYNAMICSBLOCK dynamics;
}PROX_GRENADE_SAVE_BLOCK;

void LoadStrategy_ProxGrenade(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PROX_GRENADE_BEHAV_BLOCK* behav;
	PROX_GRENADE_SAVE_BLOCK* block = (PROX_GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grende
	sbPtr = CreateGrenadeKernel(I_BehaviourProximityGrenade,&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (PROX_GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(LifeTimeRemaining)
	COPYELEMENT_LOAD(SoundGenerationTimer)

	*sbPtr->DynPtr = block->dynamics;

	Load_SoundState(&behav->SoundHandle);
}

void SaveStrategy_ProxGrenade(STRATEGYBLOCK* sbPtr)
{
	PROX_GRENADE_BEHAV_BLOCK* behav;
	PROX_GRENADE_SAVE_BLOCK* block;

	behav = (PROX_GRENADE_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(LifeTimeRemaining)
	COPYELEMENT_SAVE(SoundGenerationTimer)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

	Save_SoundState(&behav->SoundHandle);

}

/*-------------------**
** Load/Save rocket **
**-------------------*/
typedef struct rocket_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int player;

	DYNAMICSBLOCK dynamics;
}ROCKET_SAVE_BLOCK;

void LoadStrategy_Rocket(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PREDPISTOL_BEHAV_BLOCK* behav;
	ROCKET_SAVE_BLOCK* block = (ROCKET_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grende
	sbPtr = CreateRocketKernel(&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(player)

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_Rocket(STRATEGYBLOCK* sbPtr)
{
	PREDPISTOL_BEHAV_BLOCK* behav;
	ROCKET_SAVE_BLOCK* block;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(player)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}


/*-------------------**
** Load/Save PP plasma bolt **
**-------------------*/
typedef struct ppplasma_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int player;

	DYNAMICSBLOCK dynamics;
}PPPLASMA_SAVE_BLOCK;

void LoadStrategy_PPPlasmaBolt(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PREDPISTOL_BEHAV_BLOCK* behav;
	PPPLASMA_SAVE_BLOCK* block = (PPPLASMA_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default bolt
	sbPtr = CreatePPPlasmaBoltKernel(&block->dynamics.Position,&block->dynamics.OrientMat,0);
	if(!sbPtr) return;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(player)

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_PPPlasmaBolt(STRATEGYBLOCK* sbPtr)
{
	PREDPISTOL_BEHAV_BLOCK* behav;
	PPPLASMA_SAVE_BLOCK* block;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(player)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}




/*-------------------**
** Load/Save energy bolt **
**-------------------*/
typedef struct pred_energy_bolt_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int player;
	DAMAGE_PROFILE damage;
	int blast_radius;

	DYNAMICSBLOCK dynamics;
}PREDATOR_ENERGY_BOLT_SAVE_BLOCK;

void LoadStrategy_PredatorEnergyBolt(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	CASTER_BOLT_BEHAV_BLOCK* behav;
	PREDATOR_ENERGY_BOLT_SAVE_BLOCK* block = (PREDATOR_ENERGY_BOLT_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default bolt
	sbPtr = InitialiseEnergyBoltBehaviourKernel(&block->dynamics.Position,&block->dynamics.OrientMat,0,&block->damage,ONE_FIXED);
	if(!sbPtr) return;

	behav = (CASTER_BOLT_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(player)
	COPYELEMENT_LOAD(damage)
	COPYELEMENT_LOAD(blast_radius)

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_PredatorEnergyBolt(STRATEGYBLOCK* sbPtr)
{
	CASTER_BOLT_BEHAV_BLOCK* behav;
	PREDATOR_ENERGY_BOLT_SAVE_BLOCK* block;

	behav = (CASTER_BOLT_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(player)
	COPYELEMENT_SAVE(damage)
	COPYELEMENT_SAVE(blast_radius)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}




/*-------------------**
** Load/Save pulse grenade **
**-------------------*/
typedef struct pulse_grenade_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;
	int player;

	DYNAMICSBLOCK dynamics;
}PULSE_GRENADE_SAVE_BLOCK;

void LoadStrategy_PulseGrenade(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PREDPISTOL_BEHAV_BLOCK* behav;
	PULSE_GRENADE_SAVE_BLOCK* block = (PULSE_GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default grenade
	sbPtr = InitialisePulseGrenadeBehaviour();
	if(!sbPtr) return;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(player)

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_PulseGrenade(STRATEGYBLOCK* sbPtr)
{
	PREDPISTOL_BEHAV_BLOCK* behav;
	PULSE_GRENADE_SAVE_BLOCK* block;

	behav = (PREDPISTOL_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(player)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}

/*-------------------**
** Load/Save molotov **
**-------------------*/
typedef struct molotov_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int counter;

	DYNAMICSBLOCK dynamics;
}MOLOTOV_SAVE_BLOCK;

void LoadStrategy_Molotov(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	DISPLAYBLOCK* dPtr;
	MOLOTOV_BEHAV_BLOCK* behav;
	PULSE_GRENADE_SAVE_BLOCK* block = (PULSE_GRENADE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default molotov
	dPtr = SpawnMolotovCocktail(0,&block->dynamics.OrientMat);
	if(!dPtr) return;

	sbPtr = dPtr->ObStrategyBlock;

	behav = (MOLOTOV_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)

	*sbPtr->DynPtr = block->dynamics;


	//load the molotov hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&behav->HModelController);
		}
	}

}

void SaveStrategy_Molotov(STRATEGYBLOCK* sbPtr)
{
	MOLOTOV_BEHAV_BLOCK* behav;
	PULSE_GRENADE_SAVE_BLOCK* block;

	behav = (MOLOTOV_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

	//save the molotov hierarchy
	SaveHierarchy(&behav->HModelController);
}



/*-------------------------**
** Load/Save Predator Disc **
**-------------------------*/


typedef struct predator_disc_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	int counter;
	int Destruct:1;
	int Stuck	:1;
	int Bounced :1; 
	int bounces;

	char Prev_Target_SBname[SB_NAME_LENGTH];
	char Prev_Damaged_SBname[SB_NAME_LENGTH];
//pointer things
	char Target_SBname[SB_NAME_LENGTH];

//strategy block stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}PREDATOR_DISC_SAVE_BLOCK;

void LoadStrategy_PredatorDisc(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PC_PRED_DISC_BEHAV_BLOCK* behav;
	PREDATOR_DISC_SAVE_BLOCK* block = (PREDATOR_DISC_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default disc
	sbPtr = InitialiseDiscBehaviour_ForLoad();
	if(!sbPtr) return;

	behav = (PC_PRED_DISC_BEHAV_BLOCK*) sbPtr->SBdataptr;

	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(Destruct)
	COPYELEMENT_LOAD(Stuck)
	COPYELEMENT_LOAD(Bounced)  
	COPYELEMENT_LOAD(bounces)

	COPY_NAME(behav->Target_SBname,block->Target_SBname);
	COPY_NAME(behav->Prev_Target_SBname,block->Prev_Target_SBname);
	COPY_NAME(behav->Prev_Damaged_SBname,block->Prev_Damaged_SBname);
	behav->Target = FindSBWithName(behav->Target_SBname);

//strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load the hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&behav->HModelController);
		}
	}
	Load_SoundState(&behav->soundHandle);
}

void SaveStrategy_PredatorDisc(STRATEGYBLOCK* sbPtr)
{
	PREDATOR_DISC_SAVE_BLOCK *block;
	PC_PRED_DISC_BEHAV_BLOCK *behav;
	
	
	behav = (PC_PRED_DISC_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

//start copying stuff
	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(Destruct)
	COPYELEMENT_SAVE(Stuck)
	COPYELEMENT_SAVE(Bounced)  
	COPYELEMENT_SAVE(bounces)

	COPY_NAME(block->Target_SBname,behav->Target_SBname);
	COPY_NAME(block->Prev_Target_SBname,behav->Prev_Target_SBname);
	COPY_NAME(block->Prev_Damaged_SBname,behav->Prev_Damaged_SBname);

//strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;
//save hierarchy 
	SaveHierarchy(&behav->HModelController);

	Save_SoundState(&behav->soundHandle);
}



/*-------------------------**
** Load/Save speargun bolt **
**-------------------------*/

typedef struct spear_bolt_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	int counter;
	MATRIXCH Orient;
	VECTORCH Position;
//	HMODELCONTROLLER HierarchicalFragment;
	int Android;
	AVP_BEHAVIOUR_TYPE Type;
	int SubType;
	unsigned int SpearThroughFragment;
	unsigned int Stuck :1;

//strategy block stuff
	DYNAMICSBLOCK dynamics;
}SPEAR_BOLT_SAVE_BLOCK;


void LoadStrategy_SpearBolt(SAVE_BLOCK_HEADER* header)
{
	DISPLAYBLOCK* dPtr;
	STRATEGYBLOCK* sbPtr;
	SPEAR_BEHAV_BLOCK* behav;
	SPEAR_BOLT_SAVE_BLOCK* block = (SPEAR_BOLT_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default spear bolt
	dPtr = InitialiseSpeargunBoltBehaviour_ForLoad();
	if(!dPtr) return;
	
	sbPtr = dPtr->ObStrategyBlock;
	if(!sbPtr) return;

	behav = (SPEAR_BEHAV_BLOCK*)sbPtr->SBdataptr;
	
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(Orient)
	COPYELEMENT_LOAD(Position)
	COPYELEMENT_LOAD(Android)
	COPYELEMENT_LOAD(Type)
	COPYELEMENT_LOAD(SubType)
	COPYELEMENT_LOAD(SpearThroughFragment)
	COPYELEMENT_LOAD(Stuck)

	*sbPtr->DynPtr = block->dynamics;

	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&behav->HierarchicalFragment);
		}
	}
	//if there is a hierarchy  fill it in the displayblock
	if(behav->HierarchicalFragment.Root_Section)
	{
		dPtr->HModelControlBlock = &behav->HierarchicalFragment;	
	}
}

void SaveStrategy_SpearBolt(STRATEGYBLOCK* sbPtr)
{
	SPEAR_BOLT_SAVE_BLOCK *block;
	SPEAR_BEHAV_BLOCK *behav;
	
	behav = (SPEAR_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);



	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(Orient)
	COPYELEMENT_SAVE(Position)
	COPYELEMENT_SAVE(Android)
	COPYELEMENT_SAVE(Type)
	COPYELEMENT_SAVE(SubType)
	COPYELEMENT_SAVE(SpearThroughFragment)
	COPYELEMENT_SAVE(Stuck)
	
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	//save the hierarchy
	SaveHierarchy(&behav->HierarchicalFragment);
}

typedef struct frisbee_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	int counter;
	int Bounced :1; 
	int bounces;

//strategy block stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
} FRISBEE_SAVE_BLOCK;

void LoadStrategy_Frisbee(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	FRISBEE_BEHAV_BLOCK* behav;
	FRISBEE_SAVE_BLOCK* block = (FRISBEE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default disc
	sbPtr = InitialiseFrisbeeBehaviour_ForLoad();
	if(!sbPtr) return;

	behav = (FRISBEE_BEHAV_BLOCK*) sbPtr->SBdataptr;

	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(Bounced)  
	COPYELEMENT_LOAD(bounces)

//strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load the hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&behav->HModelController);
		}
	}
	Load_SoundState(&behav->soundHandle);
}

void SaveStrategy_Frisbee(STRATEGYBLOCK* sbPtr)
{
	FRISBEE_SAVE_BLOCK *block;
	FRISBEE_BEHAV_BLOCK *behav;
	
	
	behav = (FRISBEE_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

//start copying stuff
	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(Bounced)  
	COPYELEMENT_SAVE(bounces)

//strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;
//save hierarchy 
	SaveHierarchy(&behav->HModelController);

	Save_SoundState(&behav->soundHandle);
}

STRATEGYBLOCK* InitialiseFrisbeeBoltBehaviourKernel(VECTORCH *position,MATRIXCH *orient, int player, DAMAGE_PROFILE *damage, int factor) {

	DISPLAYBLOCK *dispPtr;
	DYNAMICSBLOCK *dynPtr;
	
	/* make displayblock with correct shape, etc */
	dispPtr = MakeObject(I_BehaviourFrisbeeEnergyBolt,position);
	if (dispPtr == 0) return NULL;		 // Failed to allocate display block
	
	dispPtr->SfxPtr = AllocateSfxBlock();

	if (!dispPtr->SfxPtr)
	{
		// Failed to allocate a special fx block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->SfxPtr->SfxID = SFX_FRISBEE_PLASMA_BOLT;
	/* make displayblock a dynamic module object */
	dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	dispPtr->ObShape = 0;
	dispPtr->ObStrategyBlock->shapeIndex = 0;
	dispPtr->ObMinX = -50;
	dispPtr->ObMinY = -50;
	dispPtr->ObMinZ = -50;
	dispPtr->ObMaxX = 50;
	dispPtr->ObMaxY = 50;
	dispPtr->ObMaxZ = 50;
	/* add lighting effect */
	AddLightingEffectToObject(dispPtr,LFX_PLASMA_BOLT);
	
	/* setup dynamics block */
	dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	
	if (dynPtr == 0) 
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	dispPtr->ObStrategyBlock->DynPtr = dynPtr;

	/* give missile a maximum lifetime */
	dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(CASTER_BOLT_BEHAV_BLOCK));
	
	if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
	{
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
		return NULL;
	}

	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->damage = *damage;			
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->blast_radius = MUL_FIXED(factor,Caster_BlastRadius);			
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = player;
	((CASTER_BOLT_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->soundHandle = SOUND_NOACTIVEINDEX;
			
	/* align rocket to launcher */
	dynPtr->Position=*position;
	dynPtr->PrevPosition=*position;

	dynPtr->IgnoreSameObjectsAsYou = 1;

	//GetGunDirection(&(dynPtr->LinVelocity),&position);
  	//MakeMatrixFromDirection(&(dynPtr->LinVelocity),&(dynPtr->OrientMat));
	//MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
	//dynPtr->PrevOrientMat = dynPtr->OrientMat;

	/* align velocity too */	
	dynPtr->OrientMat = *orient;
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
	/* I added this next line for networking: Patrick */
	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	/* align velocity too */	
    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, ENERGY_BOLT_SPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, ENERGY_BOLT_SPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, ENERGY_BOLT_SPEED);


	if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);

	/* Extra cunning! */
	Sound_Play(SID_PRED_LAUNCHER,"hpd",(FastRandom()&255)-128,&dynPtr->Position);

	if (player==0) {
		dynPtr->IgnoreThePlayer=0;
	}

	return dispPtr->ObStrategyBlock;
}

extern void FrisbeeEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr) 
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
    CASTER_BOLT_BEHAV_BLOCK *bbPtr = (CASTER_BOLT_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	STRATEGYBLOCK *victim;

	victim=NULL;

	MakeDewlineTrailParticles(dynPtr,32);

	/* check for a collision with something */
	if (bbPtr->counter <= 0) 
	{
		/* for net game support: send a message saying we've blown up... */
		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

		DestroyAnyStrategyBlock(sbPtr); /* timed-out */			
	}
	else if (reportPtr)
	{
  		if(reportPtr->ObstacleSBPtr)
  		{
		 	VECTORCH attack_dir;
		 	
			/* Accuracy snipped again! */

		 	GetDirectionOfAttack(reportPtr->ObstacleSBPtr,&dynPtr->LinVelocity,&attack_dir);
			
			switch (reportPtr->ObstacleSBPtr->I_SBtype)
			{
				/* No specific location damage. */
				default:
				{
					CauseDamageToObject(reportPtr->ObstacleSBPtr,&bbPtr->damage, ONE_FIXED,NULL);
					victim=reportPtr->ObstacleSBPtr;
					break;
				}
			}
		}
		
		#if 0
		{
			char hitEnvironment = 0;

			if (reportPtr->ObstacleSBPtr)
			{
				DISPLAYBLOCK *dispPtr = reportPtr->ObstacleSBPtr->SBdptr;
				if (dispPtr)
				if (dispPtr->ObMyModule)
				{
					hitEnvironment=1;
				}
			}
			else
			{
				hitEnvironment = 1;
			}
						
			if (hitEnvironment)
			{
				MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_DISSIPATINGPLASMA);
				if (AvP.Network != I_No_Network) AddNetMsg_MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_DISSIPATINGPLASMA);
			}
			else
			{
				MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_FOCUSEDPLASMA);
				if (AvP.Network != I_No_Network) AddNetMsg_MakePlasmaExplosion(&(dynPtr->Position),&(dynPtr->PrevPosition),EXPLOSION_FOCUSEDPLASMA);
			}
		}
		#endif

		if(AvP.Network != I_No_Network)	AddNetMsg_LocalObjectDestroyed(sbPtr);

    	/* Splash damage? */
		HandleEffectsOfExplosion
		(
			victim,
			&(dynPtr->Position),
			bbPtr->blast_radius,
	 		&bbPtr->damage,
			0
		);
    	DestroyAnyStrategyBlock(sbPtr);	
	} else {

		{
			VECTORCH direction;
			direction.vx = dynPtr->LinVelocity.vx + dynPtr->LinImpulse.vx;
			direction.vy = dynPtr->LinVelocity.vy + dynPtr->LinImpulse.vy;
			direction.vz = dynPtr->LinVelocity.vz + dynPtr->LinImpulse.vz;
			Normalise(&direction);
			MakeMatrixFromDirection(&direction,&dynPtr->OrientMat);
		}
		bbPtr->counter -= NormalFrameTime;
	}

}				 

void LoadStrategy_FrisbeeEnergyBolt(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	CASTER_BOLT_BEHAV_BLOCK* behav;
	PREDATOR_ENERGY_BOLT_SAVE_BLOCK* block = (PREDATOR_ENERGY_BOLT_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create default bolt
	sbPtr = InitialiseFrisbeeBoltBehaviourKernel(&block->dynamics.Position,&block->dynamics.OrientMat,0,&block->damage,ONE_FIXED);
	if(!sbPtr) return;

	behav = (CASTER_BOLT_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//copy stuff over
	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(player)
	COPYELEMENT_LOAD(damage)
	COPYELEMENT_LOAD(blast_radius)

	*sbPtr->DynPtr = block->dynamics;
}

void SaveStrategy_FrisbeeEnergyBolt(STRATEGYBLOCK* sbPtr)
{
	CASTER_BOLT_BEHAV_BLOCK* behav;
	PREDATOR_ENERGY_BOLT_SAVE_BLOCK* block;

	behav = (CASTER_BOLT_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(player)
	COPYELEMENT_SAVE(damage)
	COPYELEMENT_SAVE(blast_radius)

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

}
