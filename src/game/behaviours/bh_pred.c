/* Patrick 4/7/97 ---------------------------------------------------
Source file for predator AI
---------------------------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pfarlocs.h"
#include "pheromon.h"
#include "bh_types.h"
#include "pvisible.h"
#include "bh_far.h"
#include "bh_debri.h"
#include "bh_pred.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "bh_alien.h"
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "psnd.h"
#include "equipmnt.h"
#include "los.h"
#include "ai_sight.h"
#include "targeting.h"
#include "dxlog.h"
#include "showcmds.h"
#include "huddefs.h"
#include "pldghost.h"
#include "bh_gener.h"
#include "bh_corpse.h"
#include "bh_dummy.h"
#include "bh_agun.h"
#include "scream.h"
#include "game_statistics.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "extents.h"

#define ALL_NEW_AVOIDANCE_PRED  1
#define PREDATOR_HIT_DELTAS     1

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern SECTION_DATA* LOS_HModel_Section;        /* Section of HModel hit */
extern void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *section_pointer); 
extern void HandleSpearImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data);
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern int GlobalFrameCounter;
extern int RouteFinder_CallsThisFrame;
extern int ShowPredoStats;
extern unsigned char Null_Name[8];
extern DEATH_DATA Predator_Special_SelfDestruct_Death;

static DAMAGE_PROFILE Pred_Weapon_Damage;

extern void NPC_GetBimbleTarget(STRATEGYBLOCK *sbPtr,VECTORCH *output);
extern STRATEGYBLOCK* InitialiseEnergyBoltBehaviourKernel(VECTORCH *position,MATRIXCH *orient, int player, DAMAGE_PROFILE *damage, int factor);
extern STRATEGYBLOCK* CreatePPPlasmaBoltKernel(VECTORCH *position,MATRIXCH *orient, int player);
extern DISPLAYBLOCK *HtoHDamageToHModel(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, STRATEGYBLOCK *source, VECTORCH *attack_dir);

/* prototypes for this file */
static void ProcessFarPredatorTargetAIModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule);

static PRED_RETURN_CONDITION Execute_PFS_Wander(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Hunt(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Retreat(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Avoidance(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Pathfinder(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Return(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PFS_Engage(STRATEGYBLOCK *sbPtr);

#if 0
static PRED_RETURN_CONDITION Execute_PNS_Approach(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_StandGround(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_NewDischargePistol(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Predator_ThreatAnalysis(STRATEGYBLOCK *sbPtr);
static void CreateNPCPredatorPlasBolt(VECTORCH *startingPosition, VECTORCH *targetDirection);
static void CreateNPCPredatorDisc(VECTORCH *startingPosition, VECTORCH *targetDirection);
#endif
static PRED_RETURN_CONDITION Execute_PNS_Avoidance(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Wander(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Hunt(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Retreat(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_EngageWithPistol(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_DischargePistol(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_EngageWithPlasmaCaster(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_AttackWithPlasmaCaster(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_EngageWithWristblade(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_AttackWithWristblade(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_SwapWeapon(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_EngageWithStaff(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_AttackWithStaff(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Pathfinder(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Return(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Recover(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_Taunting(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_DischargeSpeargun(STRATEGYBLOCK *sbPtr);
static PRED_RETURN_CONDITION Execute_PNS_AttackBrutallyWithPlasmaCaster(STRATEGYBLOCK *sbPtr);

static PRED_RETURN_CONDITION Execute_PNS_SelfDestruct(STRATEGYBLOCK *sbPtr);

static void Execute_Dying(STRATEGYBLOCK *sbPtr);

static void SetPredatorAnimationSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening);
static int PredatorShouldBeCrawling(STRATEGYBLOCK *sbPtr);
static int PredatorShouldAttackPlayer(STRATEGYBLOCK *sbPtr);

static void InitPredatorCloak(PREDATOR_STATUS_BLOCK *predStatus);
static void PredatorCloakOn(PREDATOR_STATUS_BLOCK *predStatus);
static void PredatorCloakOff(PREDATOR_STATUS_BLOCK *predStatus);
static void DoPredatorCloak(PREDATOR_STATUS_BLOCK *predStatus,DYNAMICSBLOCK *dynPtr);

static int PredatorCanSeeTarget(STRATEGYBLOCK *sbPtr);
static int PredatorCanSeeObject(STRATEGYBLOCK *sbPtr,STRATEGYBLOCK *target);
static int PredatorIsAwareOfTarget(STRATEGYBLOCK *sbPtr);

void Predator_Enter_Swapping_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Avoidance_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Attacking_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Engaged_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Withdrawal_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Recover_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Wander_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Hunt_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Return_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Pathfinder_State(STRATEGYBLOCK *sbPtr);
void Predator_Enter_Taunt_State(STRATEGYBLOCK *sbPtr);

void Predator_SwitchState(STRATEGYBLOCK *sbPtr,PRED_RETURN_CONDITION state_result);
STRATEGYBLOCK *Predator_GetNewTarget(STRATEGYBLOCK *me);
int DoPredatorLaserTargeting(STRATEGYBLOCK *sbPtr);
void PredatorHandleMovingAnimation(STRATEGYBLOCK *sbPtr);
void DoPredatorHitSound(STRATEGYBLOCK *sbPtr);
void DoPredatorAcidSound(STRATEGYBLOCK *sbPtr);
void DoPredatorRandomSound(STRATEGYBLOCK *sbPtr);
void DoPredatorTauntSound(STRATEGYBLOCK *sbPtr);
void DoPredatorDeathSound(STRATEGYBLOCK *sbPtr);
void DoPredatorAISwipeSound(STRATEGYBLOCK *sbPtr);

void CreatePredoBot(VECTORCH *position, int weapon);
void Predator_Enter_SelfDestruct_State(STRATEGYBLOCK *sbPtr);
/* Patrick 21/8/97 --------------------------------------------------
Predator personalisation parameters:
format: health,speed,defenceHealth,useShoulderCannon,
timebetweenRangedAttacks,maxShotsPerRangedAttack,
timeBetweenEachShot,closeAttackDamage,chanceOfCloaking(1-8)
---------------------------------------------------------------------*/
static PREDATOR_PERSONALPARAMETERS predatorCV[] = 
{
        {800,8000,200,1,(2*ONE_FIXED),3,(ONE_FIXED),25,200,2},
        {1000,7000,500,0,(ONE_FIXED),1,(ONE_FIXED>>1),20,400,4},
        {600,10000,10,1,(4*ONE_FIXED),2,(ONE_FIXED>>1),50,300,8},
        {600,8000,200,0,(2*ONE_FIXED),2,(ONE_FIXED),25,500,2},
        {1000,9000,100,1,(ONE_FIXED),1,(ONE_FIXED>>1),35,400,6},
};

PREDATOR_WEAPON_DATA NPC_Predator_Weapons[] = {
        {
                PNPCW_Pistol,                                           /* ID */
                Execute_PNS_DischargePistol,         /* Fire Func. */
                Execute_PNS_EngageWithPistol,           /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "pred + pistol",                                        /* HierarchyName */
                "pistol",                                                       /* GunName */
                "R shoulder",                                           /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                1000,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                20000,                                                          /* MaxRange (Don't fire if further) */
                //65536>>3,                                                     /* Firing Rate */
                65536,                                                          /* Firing Rate */
                8,                                                                      /* VolleySize */
                65536>>1,                                                       /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_Wristblade,                                       /* ID */
                Execute_PNS_AttackWithWristblade,       /* Fire Func. */
                Execute_PNS_EngageWithWristblade,       /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "pred with wristblade",                         /* HierarchyName */
                NULL,                                                           /* GunName */
                "R shoulder",                                           /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* MaxRange (Don't fire if further) */
                65536<<1,                                                       /* Firing Rate */
                1,                                                                      /* VolleySize */
                65536>>1,                                                       /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_PlasmaCaster,                             /* ID */
                Execute_PNS_AttackWithPlasmaCaster,     /* Fire Func. */
                Execute_PNS_EngageWithPlasmaCaster,     /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "pred with Plasma Caster",                      /* HierarchyName */
                "Plasma caster",                                        /* GunName */
                "Plasma caster",                                        /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                -1,                                                                     /* MaxRange (Don't fire if further) */
                65536,                                                          /* Firing Rate */
                1,                                                                      /* VolleySize */
                65536,                                                          /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_Staff,                                            /* ID */
                Execute_PNS_AttackWithStaff,            /* Fire Func. */
                Execute_PNS_EngageWithStaff,            /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "pred with staff",                                      /* HierarchyName */
                NULL,                                                           /* GunName */
                "R shoulder",                                           /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* MaxRange (Don't fire if further) */
                65536<<1,                                                       /* Firing Rate */
                1,                                                                      /* VolleySize */
                65536>>1,                                                       /* SwappingTime */
                0,                                                                      /* UseElevation */
        },
        {
                PNPCW_Medicomp,                                         /* ID */
                Execute_PNS_DischargePistol,            /* Fire Func. */
                Execute_PNS_EngageWithPistol,           /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "medicomp",                                                     /* HierarchyName */
                "P caster box",                                         /* GunName */
                "R shoulder",                                           /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                0,                                                                      /* ForceFireRange (Fire if closer) */
                0,                                                                      /* MaxRange (Don't fire if further) */
                65536>>3,                                                       /* Firing Rate */
                8,                                                                      /* VolleySize */
                65536>>1,                                                       /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_Speargun,                                         /* ID */
                Execute_PNS_DischargeSpeargun,          /* Fire Func. */
                Execute_PNS_EngageWithPistol,           /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "Speargun",                                                     /* HierarchyName */
                "staff gun root",                                       /* GunName */
                "R shoulder",                                           /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                -1,                                                                     /* MaxRange (Don't fire if further) */
                65536>>2,                                                       /* Firing Rate */
                1,                                                                      /* VolleySize */
                65536>>1,                                                       /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_SeriousPlasmaCaster,                              /* ID */
                Execute_PNS_AttackBrutallyWithPlasmaCaster,     /* Fire Func. */
                Execute_PNS_EngageWithPlasmaCaster,     /* Engage Func. */
                "hnpcpredator",                                         /* Riffname */
                "pred with Plasma Caster",                      /* HierarchyName */
                "Plasma caster",                                        /* GunName */
                "Plasma caster",                                        /* ElevationName */
                "predator",                                                     /* HitLocationTableName */
                0,                                                                      /* MinRange (Don't fire when closer) */
                PRED_CLOSE_ATTACK_RANGE,                        /* ForceFireRange (Fire if closer) */
                -1,                                                                     /* MaxRange (Don't fire if further) */
                65536>>2,                                                       /* Firing Rate */
                1,                                                                      /* VolleySize */
                65536,                                                          /* SwappingTime */
                1,                                                                      /* UseElevation */
        },
        {
                PNPCW_End,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
        },
};

PREDATOR_WEAPON_DATA *GetThisNPCPredatorWeapon(PREDATOR_NPC_WEAPONS this_id) {

        int a;

        a=0;
        while (NPC_Predator_Weapons[a].id!=PNPCW_End) {
                if (NPC_Predator_Weapons[a].id==this_id) {
                        return(&NPC_Predator_Weapons[a]);
                }
                a++;
        }

        return(NULL);

}

static enum AMMO_ID GetPredatorAttackDamageType(STRATEGYBLOCK *sbPtr,int flagnum) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);

        if (predatorStatusPointer->current_attack==NULL) {
                return(AMMO_NONE);
        }

        /* No different types of predators! */
        return(predatorStatusPointer->current_attack->flag_damage[flagnum]);

}

static void PredatorNearDamageShell(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        int workingflags,flagnum,a;
        int dist,dodamage;

        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);

        /* Damage shell! */
        workingflags=predatorStatusPointer->HModelController.keyframe_flags>>1;
        flagnum=0;

        dist = VectorDistance(&(dynPtr->Position),&(predatorStatusPointer->Target->DynPtr->Position));
        if (dist<PRED_CLOSE_ATTACK_RANGE) {
                dodamage=1;
        } else if ((predatorStatusPointer->Selected_Weapon->id==PNPCW_Staff)&&(dist<PRED_STAFF_ATTACK_RANGE)) {
                dodamage=1;
        } else {
                dodamage=0;
        }
        
        for (a=0; a<NUM_ATTACKFLAGS; a++) {
        
                if (workingflags&1) {
                
                /* Do the alien attack sound */
        
                    #if 0
        	        	unsigned int rand=FastRandom();
                        switch (rand % 4)
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
                                Sound_Play(SID_SWIPE4,"dp",&dynPtr->Position,(rand&255)-128);                                   
                                break;
                        }
                                default:
                                {
                                        break;
                                }
                        }
					#else
					DoPredatorAISwipeSound(sbPtr);
					#endif
                        /* Oops, check range first. */
                        if (dodamage) {
        
                                if (predatorStatusPointer->Target->SBdptr) {
                                        VECTORCH rel_pos,attack_dir;
        
                                        rel_pos.vx=predatorStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
                                        rel_pos.vy=predatorStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
                                        rel_pos.vz=predatorStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;
        
                                        GetDirectionOfAttack(predatorStatusPointer->Target,&rel_pos,&attack_dir);

                                        if (predatorStatusPointer->Target->SBdptr->HModelControlBlock) {
                                                HtoHDamageToHModel(predatorStatusPointer->Target,&TemplateAmmo[GetPredatorAttackDamageType(sbPtr,flagnum)].MaxDamage[AvP.Difficulty], ONE_FIXED, sbPtr, &attack_dir);
                                        } else {
                                                CauseDamageToObject(predatorStatusPointer->Target,&TemplateAmmo[GetPredatorAttackDamageType(sbPtr,flagnum)].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
                                        }               
                                } else {
                                        VECTORCH rel_pos,attack_dir;
        
                                        rel_pos.vx=predatorStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
                                        rel_pos.vy=predatorStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
                                        rel_pos.vz=predatorStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;
        
                                        GetDirectionOfAttack(predatorStatusPointer->Target,&rel_pos,&attack_dir);
        
                                        CauseDamageToObject(predatorStatusPointer->Target,&TemplateAmmo[GetPredatorAttackDamageType(sbPtr,flagnum)].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
                                }
                        }
                }
                /* Prepare next flag. */
                workingflags>>=1;
                flagnum++;
        }

}

static void StartWristbladeAttackSequence(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        ATTACK_DATA *thisAttack;

        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);

        thisAttack=GetWristbladeAttackSequence(&predatorStatusPointer->HModelController,0,
                predatorStatusPointer->IAmCrouched);
        
        GLOBALASSERT(thisAttack);

        SetPredatorAnimationSequence(sbPtr,thisAttack->Sequence_Type,thisAttack->Sub_Sequence,
                thisAttack->Sequence_Length,thisAttack->TweeningTime);

        predatorStatusPointer->current_attack=thisAttack;

}

static void StartPredStaffAttackSequence(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        ATTACK_DATA *thisAttack;

        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);

        thisAttack=GetPredStaffAttackSequence(&predatorStatusPointer->HModelController,0,
                predatorStatusPointer->IAmCrouched);
        
        GLOBALASSERT(thisAttack);

        SetPredatorAnimationSequence(sbPtr,thisAttack->Sequence_Type,thisAttack->Sub_Sequence,
                thisAttack->Sequence_Length,thisAttack->TweeningTime);

        predatorStatusPointer->current_attack=thisAttack;

}

/* ChrisF 11/6/98 - Bot functions. */

void CastPredoBot(int weapon) {

        #define BOTRANGE 2000

        VECTORCH position;

        if (AvP.Network!=I_No_Network) {
                NewOnScreenMessage("NO PREDOBOTS IN MULTIPLAYER MODE");
                return;
        }

        position=Player->ObStrategyBlock->DynPtr->Position;
        position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);              
        position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);              
        position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);              

        CreatePredoBot(&position, weapon);

}

void CreatePredoBot(VECTORCH *position, int weapon)
{

        STRATEGYBLOCK* sbPtr;

        /* create and initialise a strategy block */
        sbPtr = CreateActiveStrategyBlock();
        if(!sbPtr) {
                NewOnScreenMessage("FAILED TO CREATE BOT: SB CREATION FAILURE");
                return; /* failure */
        }
        InitialiseSBValues(sbPtr);

        sbPtr->I_SBtype = I_BehaviourPredator;

        AssignNewSBName(sbPtr);

        /* create, initialise and attach a dynamics block */
        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
        if(sbPtr->DynPtr)
        {
                EULER zeroEuler = {0,0,0};
                DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
        dynPtr->PrevPosition = dynPtr->Position = *position;
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
                /* allocation failure */
                RemoveBehaviourStrategy(sbPtr);
                NewOnScreenMessage("FAILED TO CREATE BOT: DYNBLOCK CREATION FAILURE");
                return;
        }

        sbPtr->shapeIndex = 0;

        sbPtr->maintainVisibility = 1;
        sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

        /* Initialise predator's stats */
        {
                NPC_DATA *NpcData;


                NpcData=GetThisNpcData(I_NPC_Predator);
                LOCALASSERT(NpcData);
                sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
        }

        /* create, initialise and attach a predator data block */
        sbPtr->SBdataptr = (void *)AllocateMem(sizeof(PREDATOR_STATUS_BLOCK));
        if(sbPtr->SBdataptr)
        {
                SECTION *root_section;
                PREDATOR_STATUS_BLOCK *predatorStatus = (PREDATOR_STATUS_BLOCK *)sbPtr->SBdataptr;

				memset(predatorStatus,0,sizeof(*predatorStatus));

                predatorStatus->personalNumber = 0;
                if(predatorStatus->personalNumber<0)
                {
                        predatorStatus->personalNumber=0;
                        LOCALASSERT(1==0);
                }
                if(predatorStatus->personalNumber>PRED_MAXIDENTITY)
                {
                        predatorStatus->personalNumber=PRED_MAXIDENTITY;
                        LOCALASSERT(1==0);
                }

                NPC_InitMovementData(&(predatorStatus->moveData));
                NPC_InitWanderData(&(predatorStatus->wanderData));
                        predatorStatus->health = predatorCV[predatorStatus->personalNumber].startingHealth;
                sbPtr->integrity = predatorStatus->health;
                predatorStatus->behaviourState = PBS_Wandering;
                predatorStatus->stateTimer = PRED_FAR_MOVE_TIME;
                predatorStatus->internalState=0;
                predatorStatus->weaponTarget.vx = predatorStatus->weaponTarget.vy = predatorStatus->weaponTarget.vz = 0;                
                predatorStatus->volleySize = 0;
                predatorStatus->IAmCrouched = 0;                
                predatorStatus->nearSpeed = predatorCV[predatorStatus->personalNumber].speed;
                predatorStatus->GibbFactor=0;
                predatorStatus->current_attack=NULL;

                predatorStatus->incidentFlag=0;
                predatorStatus->incidentTimer=0;
                predatorStatus->patience=PRED_PATIENCE_TIME;
                predatorStatus->enableSwap=0;
                predatorStatus->enableTaunt=0;
                predatorStatus->Explode=0;

                switch( weapon )
                {
                case 0:
                default:
                        predatorStatus->PrimaryWeapon=PNPCW_Wristblade;
                        break;

                case 1:
                        predatorStatus->PrimaryWeapon=PNPCW_PlasmaCaster;
                        break;
                }

                predatorStatus->SecondaryWeapon=PNPCW_Staff;
                predatorStatus->ChangeToWeapon=PNPCW_End;
                predatorStatus->Selected_Weapon=GetThisNPCPredatorWeapon(predatorStatus->PrimaryWeapon);

                predatorStatus->obstruction.environment=0;
                predatorStatus->obstruction.destructableObject=0;
                predatorStatus->obstruction.otherCharacter=0;
                predatorStatus->obstruction.anySingleObstruction=0;

                Initialise_AvoidanceManager(sbPtr,&predatorStatus->avoidanceManager);
                InitWaypointManager(&predatorStatus->waypointManager);

                predatorStatus->Target=NULL; //Player->ObStrategyBlock;
                COPY_NAME(predatorStatus->Target_SBname,Null_Name);

                predatorStatus->soundHandle = SOUND_NOACTIVEINDEX;

                predatorStatus->Pred_Laser_On=0;

                predatorStatus->missionmodule=NULL;
                predatorStatus->fearmodule=NULL;
                predatorStatus->path=-1;
                predatorStatus->stepnumber=-1;

                //a generated predator won't have a death target
                {
                        int i;
                        for(i=0;i<SB_NAME_LENGTH;i++) predatorStatus->death_target_ID[i] =0; 
                        predatorStatus->death_target_request=0;
                        predatorStatus->death_target_sbptr=0;
                }


                root_section=GetNamedHierarchyFromLibrary(predatorStatus->Selected_Weapon->Riffname,predatorStatus->Selected_Weapon->HierarchyName);
                if (!root_section) {
                        RemoveBehaviourStrategy(sbPtr);
                        NewOnScreenMessage("FAILED TO CREATE BOT: NO HMODEL");
                        return;
                }
                Create_HModel(&predatorStatus->HModelController,root_section);
                InitHModelSequence(&predatorStatus->HModelController,0,0,ONE_FIXED);

                if (predatorStatus->Selected_Weapon->UseElevation) {
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatus->HModelController,"Elevation",(int)HMSQT_PredatorStand,(int)PSSS_Elevation,0);
                        GLOBALASSERT(delta);
                        delta->timer=32767;
                }

                predatorStatus->My_Gun_Section=GetThisSectionData(predatorStatus->HModelController.section_data,predatorStatus->Selected_Weapon->GunName);
                predatorStatus->My_Elevation_Section=GetThisSectionData(predatorStatus->HModelController.section_data,predatorStatus->Selected_Weapon->ElevationName);

                #if PREDATOR_HIT_DELTAS
                if (HModelSequence_Exists(&predatorStatus->HModelController,(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront)) {
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatus->HModelController,"HitDelta",(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront,-1);
                        GLOBALASSERT(delta);
                        delta->Playing=0;
                }
                #endif

                ProveHModel_Far(&predatorStatus->HModelController,sbPtr);

                InitPredatorCloak(predatorStatus);      

                if(!(sbPtr->containingModule))
                {
                        /* no containing module can be found... abort*/
                        RemoveBehaviourStrategy(sbPtr);
                        NewOnScreenMessage("FAILED TO CREATE BOT: MODULE CONTAINMENT FAILURE");
                        return;
                }
                LOCALASSERT(sbPtr->containingModule);

                MakePredatorNear(sbPtr);

                NewOnScreenMessage("PREDOBOT CREATED");

        }
        else
        {
                /* allocation failure */
                RemoveBehaviourStrategy(sbPtr);
                NewOnScreenMessage("FAILED TO CREATE BOT: MALLOC FAILURE");
                return;
        }                                          
}

/* Patrick 4/7/97 --------------------------------------------------
Basic NPC functions for predator:
Initialiser, visibility management,behaviour shell, and damage functions
---------------------------------------------------------------------*/
void InitPredatorBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
        TOOLS_DATA_PREDATOR *toolsData; 
        int i;

        LOCALASSERT(sbPtr);
        LOCALASSERT(bhdata);
        toolsData = (TOOLS_DATA_PREDATOR *)bhdata; 

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
                /* allocation failure */
                RemoveBehaviourStrategy(sbPtr);
                return;
        }

        /* Initialise predator's stats */
        {
                NPC_DATA *NpcData;


                NpcData=GetThisNpcData(I_NPC_Predator);
                LOCALASSERT(NpcData);
                sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
        }

        /* create, initialise and attach a predator data block */
        sbPtr->SBdataptr = (void *)AllocateMem(sizeof(PREDATOR_STATUS_BLOCK));
        if(sbPtr->SBdataptr)
        {
                SECTION *root_section;
                PREDATOR_STATUS_BLOCK *predatorStatus = (PREDATOR_STATUS_BLOCK *)sbPtr->SBdataptr;

                predatorStatus->personalNumber = toolsData->predator_number;
                if(predatorStatus->personalNumber<0)
                {
                        predatorStatus->personalNumber=0;
                        LOCALASSERT(1==0);
                }
                if(predatorStatus->personalNumber>PRED_MAXIDENTITY)
                {
                        predatorStatus->personalNumber=PRED_MAXIDENTITY;
                        LOCALASSERT(1==0);
                }

                NPC_InitMovementData(&(predatorStatus->moveData));
                NPC_InitWanderData(&(predatorStatus->wanderData));
        predatorStatus->health = predatorCV[predatorStatus->personalNumber].startingHealth;
                sbPtr->integrity = predatorStatus->health;
                predatorStatus->behaviourState = PBS_Wandering;
                predatorStatus->stateTimer = PRED_FAR_MOVE_TIME;
                predatorStatus->internalState=0;
                predatorStatus->weaponTarget.vx = predatorStatus->weaponTarget.vy = predatorStatus->weaponTarget.vz = 0;                
                predatorStatus->volleySize = 0;
                predatorStatus->IAmCrouched = 0;                
                predatorStatus->nearSpeed = predatorCV[predatorStatus->personalNumber].speed;
                predatorStatus->GibbFactor=0;
                predatorStatus->current_attack=NULL;

                predatorStatus->incidentFlag=0;
                predatorStatus->incidentTimer=0;
                predatorStatus->patience=PRED_PATIENCE_TIME;
                predatorStatus->enableSwap=0;
                predatorStatus->enableTaunt=0;
                predatorStatus->Explode=0;

                #if 0
                predatorStatus->PrimaryWeapon=PNPCW_Pistol;
                predatorStatus->SecondaryWeapon=PNPCW_Wristblade;
                #else
                predatorStatus->PrimaryWeapon=toolsData->primary;
                predatorStatus->SecondaryWeapon=toolsData->secondary;
                #endif
                predatorStatus->ChangeToWeapon=PNPCW_End;
                predatorStatus->Selected_Weapon=GetThisNPCPredatorWeapon(predatorStatus->PrimaryWeapon);

                predatorStatus->obstruction.environment=0;
                predatorStatus->obstruction.destructableObject=0;
                predatorStatus->obstruction.otherCharacter=0;
                predatorStatus->obstruction.anySingleObstruction=0;

                Initialise_AvoidanceManager(sbPtr,&predatorStatus->avoidanceManager);
                InitWaypointManager(&predatorStatus->waypointManager);

                predatorStatus->Target=NULL; //Player->ObStrategyBlock;
                COPY_NAME(predatorStatus->Target_SBname,Null_Name);

                predatorStatus->soundHandle = SOUND_NOACTIVEINDEX;

                predatorStatus->Pred_Laser_On=0;

                predatorStatus->missionmodule=NULL;
                predatorStatus->fearmodule=NULL;
                predatorStatus->path=toolsData->path;
                predatorStatus->stepnumber=toolsData->stepnumber;
                if ((predatorStatus->path!=-1)&&(predatorStatus->stepnumber!=-1)) {
                        predatorStatus->behaviourState = PBS_Pathfinding;
                }

                for(i=0;i<SB_NAME_LENGTH;i++) predatorStatus->death_target_ID[i] = toolsData->death_target_ID[i];
                predatorStatus->death_target_request=toolsData->death_target_request;
                predatorStatus->death_target_sbptr=0;
                
                
                root_section=GetNamedHierarchyFromLibrary(predatorStatus->Selected_Weapon->Riffname,predatorStatus->Selected_Weapon->HierarchyName);
                GLOBALASSERT(root_section);
                Create_HModel(&predatorStatus->HModelController,root_section);
                InitHModelSequence(&predatorStatus->HModelController,0,0,ONE_FIXED);

                if (predatorStatus->Selected_Weapon->UseElevation) {
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatus->HModelController,"Elevation",(int)HMSQT_PredatorStand,(int)PSSS_Elevation,0);
                        GLOBALASSERT(delta);
                        delta->timer=32767;
                }

                predatorStatus->My_Gun_Section=GetThisSectionData(predatorStatus->HModelController.section_data,predatorStatus->Selected_Weapon->GunName);
                predatorStatus->My_Elevation_Section=GetThisSectionData(predatorStatus->HModelController.section_data,predatorStatus->Selected_Weapon->ElevationName);

                #if PREDATOR_HIT_DELTAS
                if (HModelSequence_Exists(&predatorStatus->HModelController,(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront)) {
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatus->HModelController,"HitDelta",(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront,-1);
                        GLOBALASSERT(delta);
                        delta->Playing=0;
                }
                #endif

                ProveHModel_Far(&predatorStatus->HModelController,sbPtr);

                InitPredatorCloak(predatorStatus);      
        }
        else
        {
                /* allocation failure */
                RemoveBehaviourStrategy(sbPtr);
                return;
        }                                          
}

void InitDormantPredatorBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
        DORMANT_PREDATOR_STATUS_BLOCK* pred_bhv;
        GLOBALASSERT(sbPtr);
        pred_bhv=(DORMANT_PREDATOR_STATUS_BLOCK*)AllocateMem(sizeof(DORMANT_PREDATOR_STATUS_BLOCK));

        pred_bhv->bhvr_type=I_BehaviourDormantPredator;
        pred_bhv->toolsData=bhdata;

        sbPtr->SBdataptr=(void*) pred_bhv;

}

void ActivateDormantPredator(STRATEGYBLOCK* sbPtr)
{
        DORMANT_PREDATOR_STATUS_BLOCK* pred_bhv;
        void* toolsData;

        GLOBALASSERT(sbPtr);
        GLOBALASSERT(sbPtr->SBdataptr);
        pred_bhv = (DORMANT_PREDATOR_STATUS_BLOCK*)sbPtr->SBdataptr;
        toolsData=pred_bhv->toolsData;
                
        //convert this strategyblock to a predator      
        DeallocateMem(pred_bhv);
        InitialiseSBValues(sbPtr);
        sbPtr->I_SBtype = I_BehaviourPredator;
        EnableBehaviourType(sbPtr,I_BehaviourPredator ,toolsData );

        //find strategyblock of death target
        {
                PREDATOR_STATUS_BLOCK *predatorStatus = (PREDATOR_STATUS_BLOCK *)sbPtr->SBdataptr;
                predatorStatus->death_target_sbptr = FindSBWithName(predatorStatus->death_target_ID);
        }
        
        sbPtr->maintainVisibility = 1;
        sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

        
}


void SetPredatorElevation(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;
        int offsetx,offsety,offsetz,offseta,angle1;
        DELTA_CONTROLLER *elevation_controller;
        VECTORCH gunpos;

        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    

        if (predatorStatusPointer->Selected_Weapon->UseElevation==0) {
                /* Non elevating weapon. */
                return;
        }

        /* Get gun position? */

        if (predatorStatusPointer->My_Elevation_Section) {
                gunpos=predatorStatusPointer->My_Elevation_Section->World_Offset;
        } else {
                GetTargetingPointOfObject_Far(sbPtr,&gunpos);
        }

        /* Aim at weaponTarget. */
        
        offsetx=(predatorStatusPointer->weaponTarget.vx)-(gunpos.vx);
        offsety=(predatorStatusPointer->weaponTarget.vz)-(gunpos.vz);
        offseta=-((predatorStatusPointer->weaponTarget.vy)-(gunpos.vy));

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
        angle1=ArcTan(offseta,offsetz);

        if (angle1>=3072) angle1-=4096;
        if (angle1>=2048) angle1=angle1-3072;
        if (angle1>1024) angle1=2048-angle1;

        GLOBALASSERT(angle1>=-1024);
        GLOBALASSERT(angle1<=1024);

        elevation_controller=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
        GLOBALASSERT(elevation_controller);
        {
                int fake_timer;

                fake_timer=1024-angle1;
                fake_timer<<=5;
                if (fake_timer==65536) fake_timer=65535;

                GLOBALASSERT(fake_timer>=0);
                GLOBALASSERT(fake_timer<65536);

                elevation_controller->timer=fake_timer;

        }

}

void CentrePredatorElevation(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;
        DELTA_CONTROLLER *elevation_controller;

        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    

        if (predatorStatusPointer->Selected_Weapon->UseElevation==0) {
                /* Non elevating weapon. */
                return;
        }

        elevation_controller=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
		if (elevation_controller) {
	    	/* You can't be too careful with swap weapon stuff...? */
	        elevation_controller->timer=32767;
		}

}

void PredatorBehaviour(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        int predatorIsNear;
        char *descriptor;
        PRED_RETURN_CONDITION state_result;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);                         

        /* test if we've got a containing module: if we haven't, do nothing.
        This is important as the object could have been marked for deletion by the visibility 
        management system...*/
        if(!sbPtr->containingModule)
        {
                DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
                return;
        } 

        if(sbPtr->SBdptr) 
        {
                LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);                                            
                predatorIsNear=1;
        } else {
                predatorIsNear=0;
        }               

        if (ShowSlack) {
                int synthSpeed,setSpeed,slack;
                VECTORCH offset;
                extern int SlackTotal;
                extern int SlackSize;
                                
                offset.vx=(sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx);
                offset.vy=(sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy);
                offset.vz=(sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz);
                
                synthSpeed=Magnitude(&offset);
                synthSpeed=DIV_FIXED(synthSpeed,NormalFrameTime);
                setSpeed=Magnitude(&sbPtr->DynPtr->LinVelocity);
                
                if (setSpeed) {         
                        slack=(ONE_FIXED-(DIV_FIXED(synthSpeed,setSpeed)));
                        SlackTotal+=slack;
                        SlackSize++;
                }
                #if 0
                PrintDebuggingText("MaxSpeed = %d, SynthSpeed = %d, SetSpeed = %d, Slack %d\n",alienStatusPointer->MaxSpeed,synthSpeed,setSpeed,slack);
                #endif
        }

        InitWaypointSystem(0);
        predatorStatusPointer->Pred_Laser_On=0;

        if (Validate_Target(predatorStatusPointer->Target,predatorStatusPointer->Target_SBname)==0) {
                predatorStatusPointer->Target=NULL;
        }

		/* Consider changing target? */
		if (predatorStatusPointer->behaviourState==PBS_Engaging) {
			if (predatorStatusPointer->incidentFlag) {
				if ((FastRandom()&65535)<32767) {
					predatorStatusPointer->Target=NULL;
				}
			}
		}

        if (predatorStatusPointer->Target==NULL) {
        	if ((predatorIsNear)||(predatorStatusPointer->incidentFlag)) {
                /* Get new target. */
                predatorStatusPointer->Target=Predator_GetNewTarget(sbPtr);

                if (predatorStatusPointer->Target) {
                        #if 0
                        textprint("Predator gets new target.\n");
                        #endif
                        COPY_NAME(predatorStatusPointer->Target_SBname,predatorStatusPointer->Target->SBname);
        
                } else {
                        #if 0
                                PrintDebuggingText("Predator found no target!\n");
                        #endif
                }
			}
        }

        /* first of all, look after our cloaking device */
        DoPredatorCloak(predatorStatusPointer,sbPtr->DynPtr);

        /* Unset incident flag. */
        predatorStatusPointer->incidentFlag=0;

        predatorStatusPointer->incidentTimer-=NormalFrameTime;
        
        if (predatorStatusPointer->incidentTimer<0) {
                predatorStatusPointer->incidentFlag=1;
                predatorStatusPointer->incidentTimer=32767+(FastRandom()&65535);
        }

        if (predatorStatusPointer->GibbFactor) {
                /* If you're gibbed, you're dead. */
                sbPtr->SBDamageBlock.Health = 0;
        }

        if (sbPtr->SBDamageBlock.IsOnFire) {

                CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);

                if (sbPtr->I_SBtype==I_BehaviourNetCorpse) {
                        /* Gettin' out of here... */
                        return;
                }

                if (predatorStatusPointer->incidentFlag) {
                        if ((FastRandom()&65535)<32767) {
                                sbPtr->SBDamageBlock.IsOnFire=0;
                        }
                }
                
        }

        if (predatorStatusPointer->behaviourState!=PBS_Dying) {
                HModel_Regen(&predatorStatusPointer->HModelController,PRED_REGEN_TIME);
        }
        
        /* now execute behaviour state */
        switch(predatorStatusPointer->behaviourState)
        {
                case (PBS_Wandering):
                {
                        descriptor="Wandering";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Wander(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Wander(sbPtr);
                        }
                        break;
                }
                case (PBS_Hunting):
                {
                        descriptor="Hunting";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Hunt(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Hunt(sbPtr);
                        }
                        break;
                }
                case (PBS_Avoidance):
                {
                        switch (predatorStatusPointer->avoidanceManager.substate) {
                                default:
                                case AvSS_FreeMovement:
                                        descriptor="Avoidance Level 0";
                                        break;
                                case AvSS_FirstAvoidance:
                                        descriptor="Avoidance Level 1";
                                        break;
                                case AvSS_SecondAvoidance:
                                        descriptor="Avoidance Level 2";
                                        break;
                                case AvSS_ThirdAvoidance:
                                        descriptor="Avoidance Level 3";
                                        break;
                        }       
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Avoidance(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Avoidance(sbPtr);
                        }
                        break;
                }
                case (PBS_Dying):
                {
                        descriptor="Dying";
                        if (predatorIsNear) {
                                Execute_Dying(sbPtr);
                        } else {
                                Execute_Dying(sbPtr);
                        }
                        state_result=PRC_No_Change;
                        break;
                }
                case (PBS_Withdrawing):
                {
                        descriptor="Withdrawing";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Retreat(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Retreat(sbPtr);
                        }
                        break;
                }
                case (PBS_Recovering):
                {
                        descriptor="Recovering";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Recover(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PNS_Recover(sbPtr);
                        }
                        break;
                }
                case (PBS_SwapWeapon):
                {
                        descriptor="Swapping";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_SwapWeapon(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                /* Oooh! */
                                state_result=Execute_PNS_SwapWeapon(sbPtr);
                                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);
                        }
                        break;
                }
                case (PBS_Engaging):
                {
                        descriptor="Engaging";
                        if (predatorIsNear) {
                                state_result=(*predatorStatusPointer->Selected_Weapon->WeaponEngageFunction)(sbPtr);
                        } else {
                                state_result=Execute_PFS_Engage(sbPtr);
                        }
                        break;
                }
                case (PBS_Attacking):
                {
                        descriptor="Attacking";
                        if (predatorIsNear) {
                                state_result=(*predatorStatusPointer->Selected_Weapon->WeaponFireFunction)(sbPtr);
                                SetPredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Engage(sbPtr);
                        }
                        break;
                }
                case (PBS_Returning):
                {
                        descriptor="Returning";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Return(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Return(sbPtr);
                        }
                        break;
                }
                case (PBS_Pathfinding):
                {
                        descriptor="Pathfinding";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Pathfinder(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PFS_Pathfinder(sbPtr);
                        }
                        break;
                }
                case (PBS_Taunting):
                {
                        descriptor="Taunting";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_Taunting(sbPtr);
                                CentrePredatorElevation(sbPtr);
                        } else {
                                state_result=Execute_PNS_Taunting(sbPtr);
                        }
                        break;
                }
                case (PBS_SelfDestruct):
                {
                        descriptor="Self Destructing";
                        if (predatorIsNear) {
                                state_result=Execute_PNS_SelfDestruct(sbPtr);
                                /* No elevation should be present. */
                        } else {
                                state_result=Execute_PNS_SelfDestruct(sbPtr);
                        }
                        break;
                }
                default:
                {
                        LOCALASSERT(1==0);
                        break;
                }
        }

        if (ShowPredoStats) {
                switch (predatorStatusPointer->CloakStatus) {
                        case PCLOAK_Off:
                                PrintDebuggingText("DeCloaked ");
                                break;
                        case PCLOAK_On:
                                PrintDebuggingText("Cloaked ");
                                break;
                        case PCLOAK_Activating:
                                PrintDebuggingText("Cloaking (%d) ",predatorStatusPointer->CloakTimer);
                                break;
                        case PCLOAK_Deactivating:
                                PrintDebuggingText("DeCloaking (%d) ",predatorStatusPointer->CloakTimer);
                                break;
                        default:
                                GLOBALASSERT(0);
                                break;
                }
                PrintDebuggingText("%s Predator in %s: %d,%d\n",descriptor,sbPtr->containingModule->name,
                        (sbPtr->SBDamageBlock.Health>>ONE_FIXED_SHIFT),(sbPtr->SBDamageBlock.Armour>>ONE_FIXED_SHIFT));
        }
        
        Predator_SwitchState(sbPtr,state_result);

        if (!predatorIsNear) {
                
                /* check here to see if predator is in a proximity door - if so, trigger it to open. */
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
                                textprint("FAR PREDATOR MODULE CONTAINMENT FAILURE \n");
                                LOCALASSERT(1==0);
                        }  
                }
                #endif
        }

        /* if we have actually died, we need to remove the strategyblock... so do this here */
        if((predatorStatusPointer->behaviourState == PBS_Dying)&&
           (predatorStatusPointer->stateTimer <= 0)) {
                DestroyAnyStrategyBlock(sbPtr);
        }
        
        #if 0
        /* Now, right at the end, fix animation speed. */
        if ((predatorStatusPointer->HModelController.Sub_Sequence==PRSS_Standard)
                ||(predatorStatusPointer->HModelController.Sub_Sequence==PCSS_Standard)) {

                int speed,animfactor;
                /* ...compute speed factor... */
                speed=Approximate3dMagnitude(&sbPtr->DynPtr->LinVelocity);
                if (speed==0) {
                        animfactor=ONE_FIXED;
                } else {
                        animfactor=DIV_FIXED(625,speed); // Was 512!  Difference to correct for rounding down...
                }
                GLOBALASSERT(animfactor>0);
                if (ShowPredoStats) {
                        PrintDebuggingText("Anim Factor %d, Tweening %d\n",animfactor,predatorStatusPointer->HModelController.Tweening);
                }
                if (predatorStatusPointer->HModelController.Tweening==0) {
                        HModel_SetToolsRelativeSpeed(&predatorStatusPointer->HModelController,animfactor);
                }
        }
        #endif

        /* Update delta playing flag. */
        {
                DELTA_CONTROLLER *hitdelta;
                                
                hitdelta=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta");
        
                if (hitdelta) {
                        if (DeltaAnimation_IsFinished(hitdelta)) {
                                hitdelta->Playing=0;
                        }
                }
        }

	if (predatorStatusPointer->Explode) {
		StartPredatorSelfDestructExplosion(sbPtr);
	}
}

void PredatorHandleMovingAnimation(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH offset;
        int speed,animfactor,walking;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);

        offset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
        offset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
        offset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;
        
        /* ...compute speed factor... */
        speed=Magnitude(&offset);
        if ((speed<(MUL_FIXED(NormalFrameTime,50))) 
                &&(predatorStatusPointer->HModelController.Tweening==0)) {
                /* Not moving much, are we?  Be stationary! */
               if (ShowPredoStats)
               {
                        PrintDebuggingText("Forced stationary animation!\n");
               }
               if(predatorStatusPointer->IAmCrouched)
               {
                        if ((predatorStatusPointer->HModelController.Sequence_Type!=HMSQT_PredatorCrouch)
                                ||(predatorStatusPointer->HModelController.Sub_Sequence!=PCrSS_Standard))
                        {
                                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrouch,PCrSS_Standard,-1,(ONE_FIXED>>3));
                        }
                }
                else
                {
                        if ((predatorStatusPointer->HModelController.Sequence_Type!=HMSQT_PredatorStand)
                                ||(predatorStatusPointer->HModelController.Sub_Sequence!=PSSS_Standard))
                        {
                                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorStand,PSSS_Standard,-1,(ONE_FIXED>>3));
                                 // Alex: this is a bit of a fudge
                                (predatorStatusPointer->avoidanceManager.avoidanceDirection).vx = -(predatorStatusPointer->avoidanceManager.avoidanceDirection).vx;
                                (predatorStatusPointer->avoidanceManager.avoidanceDirection).vz = -(predatorStatusPointer->avoidanceManager.avoidanceDirection).vz;
                        }
                }
                return;
        }
        speed=DIV_FIXED(speed,NormalFrameTime);
        
        if (speed==0) {
                animfactor=ONE_FIXED;
        } else {
                animfactor=DIV_FIXED(625,speed); // Was 512!  Difference to correct for rounding down...
        }
        GLOBALASSERT(animfactor>0);
        if (ShowPredoStats) {
                PrintDebuggingText("Anim Factor %d, Tweening %d, Speed %d\n",animfactor,predatorStatusPointer->HModelController.Tweening,speed);
        }

        walking=0;

        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,HMSQT_PredatorRun,PRSS_Walk)) {
                /* Are we currently walking? */
                if ((predatorStatusPointer->HModelController.Sequence_Type==HMSQT_PredatorRun)
                        &&(predatorStatusPointer->HModelController.Sub_Sequence==PRSS_Walk)) {
                        if (speed<PRED_WALKING_SPEED_MAX) {
                                walking=1;
                        } else {
                                walking=0;
                        }
                } else {
                        if (speed<PRED_WALKING_SPEED_MIN) {
                                walking=1;
                        } else {
                                walking=0;
                        }
                }
        }
        
        /* Start animation? */
        if (predatorStatusPointer->HModelController.Tweening==0) {

                /* If still tweening, probably best to leave it alone... */

                if (PredatorShouldBeCrawling(sbPtr)) {
                
                        predatorStatusPointer->IAmCrouched=1;
                        
                        /* Only one possible sequence here. */
                        if ((predatorStatusPointer->HModelController.Sequence_Type!=HMSQT_PredatorCrawl)
                                ||(predatorStatusPointer->HModelController.Sub_Sequence!=PCSS_Standard)) {
                                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                        }

                } else if (walking==0) {
                
                        predatorStatusPointer->IAmCrouched=0;

                        /* Are we doing the right anim? */
                        if ((predatorStatusPointer->HModelController.Sequence_Type!=HMSQT_PredatorRun)
                                ||(predatorStatusPointer->HModelController.Sub_Sequence!=PRSS_Standard)) {
                                
                                /* If we're currently walking, tween over. */
                                if ((predatorStatusPointer->HModelController.Sequence_Type==HMSQT_PredatorRun)
                                        &&(predatorStatusPointer->HModelController.Sub_Sequence==PRSS_Walk)) {
                                        InitHModelTweening_ToTheMiddle(&predatorStatusPointer->HModelController, (ONE_FIXED>>3),HMSQT_PredatorRun,
                                                PRSS_Standard,(ONE_FIXED<<1),((predatorStatusPointer->HModelController.sequence_timer+(ONE_FIXED>>3))&65535),1);
                                } else {
                                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                                }
                        }

                } else {
                
                        predatorStatusPointer->IAmCrouched=0;

                        /* Are we doing the right anim? */
                        if ((predatorStatusPointer->HModelController.Sequence_Type!=HMSQT_PredatorRun)
                                ||(predatorStatusPointer->HModelController.Sub_Sequence!=PRSS_Walk)) {
                                /* If we're currently running, tween over. */
                                if ((predatorStatusPointer->HModelController.Sequence_Type==HMSQT_PredatorRun)
                                        &&(predatorStatusPointer->HModelController.Sub_Sequence==PRSS_Standard)) {
                                        InitHModelTweening_ToTheMiddle(&predatorStatusPointer->HModelController, (ONE_FIXED>>3),HMSQT_PredatorRun,
                                                PRSS_Walk,(ONE_FIXED<<1),((predatorStatusPointer->HModelController.sequence_timer+(ONE_FIXED>>3))&65535),1);
                                } else {
                                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Walk,ONE_FIXED,(ONE_FIXED>>3));
                                }
                        }
                
                }
        
        }

        if (predatorStatusPointer->HModelController.Tweening==0) {
                HModel_SetToolsRelativeSpeed(&predatorStatusPointer->HModelController,animfactor);
        }

}

void MakePredatorNear(STRATEGYBLOCK *sbPtr)
{
        extern MODULEMAPBLOCK AlienDefaultMap;
        MODULE tempModule;
        DISPLAYBLOCK *dPtr;
        DYNAMICSBLOCK *dynPtr;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);
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
        if(dPtr==NULL) return; /* cannot allocate displayblock, so leave far */
                
        sbPtr->SBdptr = dPtr;
        dPtr->ObStrategyBlock = sbPtr;
        dPtr->ObMyModule = NULL;                                        
                            
        /* need to initialise positional information in the new display block */ 
        dPtr->ObWorld = dynPtr->Position;
        dPtr->ObEuler = dynPtr->OrientEuler;
        dPtr->ObMat = dynPtr->OrientMat;
          
        /* status block init */
        predatorStatusPointer->weaponTarget.vx = predatorStatusPointer->weaponTarget.vy = predatorStatusPointer->weaponTarget.vz = 0;                   
        predatorStatusPointer->volleySize = 0;
        InitPredatorCloak(predatorStatusPointer);       

        /* regenerate health (before deciding whether to attack or defend) */
        if(predatorStatusPointer->health < predatorCV[predatorStatusPointer->personalNumber].startingHealth)
        {
                predatorStatusPointer->health += predatorCV[predatorStatusPointer->personalNumber].regenerationUnit;    

                if(predatorStatusPointer->health > predatorCV[predatorStatusPointer->personalNumber].startingHealth)
                        predatorStatusPointer->health = predatorCV[predatorStatusPointer->personalNumber].startingHealth;
        }

        /* zero linear velocity in dynamics block */
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        /* initialise state and sequence... */
        
        dPtr->HModelControlBlock=&predatorStatusPointer->HModelController;

        CentrePredatorElevation(sbPtr);
        
        ProveHModel(dPtr->HModelControlBlock,dPtr);

        /* LOCALASSERT(predatorStatusPointer->nearBehaviourState != PNS_Dying); */
        if(PredatorShouldBeCrawling(sbPtr)) predatorStatusPointer->IAmCrouched = 1;
        else predatorStatusPointer->IAmCrouched = 0;

        /*Copy extents from the collision extents in extents.c*/
        dPtr->ObMinX=-CollisionExtents[CE_PREDATOR].CollisionRadius;
        dPtr->ObMaxX=CollisionExtents[CE_PREDATOR].CollisionRadius;
        dPtr->ObMinZ=-CollisionExtents[CE_PREDATOR].CollisionRadius;
        dPtr->ObMaxZ=CollisionExtents[CE_PREDATOR].CollisionRadius;
        dPtr->ObMinY=CollisionExtents[CE_PREDATOR].CrouchingTop;
        dPtr->ObMaxY=CollisionExtents[CE_PREDATOR].Bottom;
        dPtr->ObRadius = 1000;
        InitWaypointManager(&predatorStatusPointer->waypointManager);

        if (predatorStatusPointer->behaviourState==PBS_Recovering) {
                if(predatorStatusPointer->IAmCrouched) {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrouch,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                } else {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorStand,PSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                }
        } else if ((predatorStatusPointer->behaviourState!=PBS_SwapWeapon)
        	&&(predatorStatusPointer->behaviourState!=PBS_SelfDestruct)) {
                if(predatorStatusPointer->IAmCrouched) {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                } else {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
                }
        }

}

void MakePredatorFar(STRATEGYBLOCK *sbPtr)
{
        /* get the predator's status block */
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        int i;
        
        LOCALASSERT(sbPtr);     
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);   
        LOCALASSERT(sbPtr->SBdptr != NULL);

        /* get rid of the displayblock */
        i = DestroyActiveObject(sbPtr->SBdptr);
        LOCALASSERT(i==0);
        sbPtr->SBdptr = NULL;

        /* zero linear velocity in dynamics block */
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;      

        /* status block init */
        if ((predatorStatusPointer->behaviourState!=PBS_SwapWeapon)
        	&&(predatorStatusPointer->behaviourState!=PBS_SelfDestruct)) {
                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
        }
        InitPredatorCloak(predatorStatusPointer);       
        
        /* initialise state and sequence... */
        if(predatorStatusPointer->behaviourState == PBS_Dying)
        {
                DestroyAnyStrategyBlock(sbPtr);
                return;
        }
        
        #if 0
        if(PredatorShouldAttackPlayer(sbPtr))
        {
                predatorStatusPointer->behaviourState = PBS_Hunting;
        }
        else
        {
                predatorStatusPointer->behaviourState = PBS_Withdrawing;        
        }
        #endif
}

void PredatorIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming)
{
         
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
                                        
        /* if we're dying, do nothing */
        if(predatorStatusPointer->behaviourState==PBS_Dying)
        {
                /* PFS should be dying, too */
                return;
        }                               
        
        if(!(sbPtr->SBdptr))
        {
                DestroyAnyStrategyBlock(sbPtr);
                return;
        }

        /* Might want to get a new target? */

        predatorStatusPointer->Target=NULL;

        if (predatorStatusPointer->behaviourState!=PBS_SelfDestruct) {  
                NPC_DATA *NpcData;
                /* we are far enough away, so return to approach? */

                NpcData=GetThisNpcData(I_NPC_Predator);
                
                if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-3)))
                        &&(sbPtr->SBDamageBlock.Health>0)) {
                        /* 12.5% health? */
                        Predator_Enter_SelfDestruct_State(sbPtr);
                        return;
                }
        }
		
		#if 0
		{
			int tkd;
			VECTORCH origin,blast;
			/* Make a blood splat? */
			tkd=TotalKineticDamage(damage);
			tkd=MUL_FIXED(tkd,multiple);

			if (tkd) {
				if (Section) {
					origin=Section->World_Offset;
				} else {
					GetTargetingPointOfObject_Far(sbPtr,&origin);
				}
				if (incoming) {
					blast=origin;
					blast.vx+=incoming->vx;
					blast.vy+=incoming->vy;
					blast.vz+=incoming->vz;
				} else {
					blast=origin;
				}
				MakeBloodExplosion(&origin, 100, &blast, (tkd>>2), PARTICLE_PREDATOR_BLOOD);
			}
		}
		#endif

		/* Do we really want to die? */
        if(sbPtr->SBDamageBlock.Health <= 0) {
	        if (predatorStatusPointer->behaviourState!=PBS_SelfDestruct) {
				if (damage->Id==AMMO_ALIEN_TAIL) {
					/* Actually, don't die just yet... */
	                NPC_DATA *NpcData;

	                NpcData=GetThisNpcData(I_NPC_Predator);
                
	                sbPtr->SBDamageBlock.Health=(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-3));
					/* 12.5% health. */
					Predator_Enter_SelfDestruct_State(sbPtr);
					return;
				}
			}
		}

        if(sbPtr->SBDamageBlock.Health <= 0)
        {
                /* KILL PREDATOR! */
                int deathtype=0;
	
				if (AvP.PlayerType!=I_Predator) {
					CurrentGameStats_CreatureKilled(sbPtr,Section);		
				}

                /*notify death target ,if predator has one*/
                if(predatorStatusPointer->death_target_sbptr)
                {
                        RequestState(predatorStatusPointer->death_target_sbptr,predatorStatusPointer->death_target_request, 0);
                } 
        
                #if 0
                /* switch to dying-suicide state? */
                predatorStatusPointer->behaviourState = PBS_Dying;
                predatorStatusPointer->stateTimer = PRED_DIETIME;
                /* No longer need with corpses, and allows us to reference behaviourState later. */
                #endif

                /* Set deathtype */
                
                {
                        int tkd;
                        
                        tkd=TotalKineticDamage(damage);
                        deathtype=0;

                        if (damage->ExplosivePower==1) {
                                if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
                                        /* Okay, you can... splat now. */
                                        predatorStatusPointer->GibbFactor=-(ONE_FIXED>>1);
                                        deathtype=2;
                                }
                        } else if ((tkd<40)&&((multiple>>16)>1)) {
                                int newmult;

                                newmult=DIV_FIXED(multiple,NormalFrameTime);
                                if (MUL_FIXED(tkd,newmult)>(500)) {
                                        predatorStatusPointer->GibbFactor=-(ONE_FIXED>>2);
                                        deathtype=2;
                                }
                        }

                        if ((damage->ExplosivePower==2)||(damage->ExplosivePower==6)) {
                                /* Basically SADARS only. */
                                predatorStatusPointer->GibbFactor=-(ONE_FIXED);
                                deathtype=3;
                        }
                }

                if (damage->ForceBoom) {
                        deathtype+=damage->ForceBoom;
                }

                {
                        SECTION_DATA *chest;
                        
                        chest=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"chest");
                        
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
				if (predatorStatusPointer->GibbFactor>0) {
					/* This probably never happens... */
				} else {
					SECTION_DATA *head;
					/* make a sound... if you have a head. */
					head=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"head");

					/* Is it still attached? */
					if (head) {
						if (head->flags&section_data_notreal) {
							head=NULL;
						}
					}

					if ((predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX)&&(head)) {
						DoPredatorDeathSound(sbPtr);
					}
				}

                {
                        DEATH_DATA *this_death;
                        HIT_FACING facing;
                        SECTION *root;
                        int burning;
                        int wounds;

                        root=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
                        
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
                                &&(damage->Fire>0)
                                &&(damage->Electrical==0)
                                &&(damage->Acid==0)
                                ) {
                                burning=1;
                        } else {
                                burning=0;
                        }
        
                        if (Section) {
                                wounds=(Section->flags&section_flags_wounding);
                        } else {
                                wounds=0;
                        }
                        
                        //if (predatorStatusPointer->behaviourState==PBS_SelfDestruct) {
                        //      this_death=&Predator_Special_SelfDestruct_Death;
                        //} else {
                                this_death=GetPredatorDeathSequence(&predatorStatusPointer->HModelController,root,wounds,
                                        wounds,deathtype,&facing,burning,predatorStatusPointer->IAmCrouched,0);
                        //}
                        
                        GLOBALASSERT(this_death);
                        
                        Remove_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
                        Remove_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta");

                        Convert_Predator_To_Corpse(sbPtr,this_death);

                        return;
                }
        } else {
                /* If not dead, play a hit delta. */
                DELTA_CONTROLLER *hitdelta;
                int frontback;

				if ((damage->Impact==0) 		
					&&(damage->Cutting==0)  	
					&&(damage->Penetrative==0)
					&&(damage->Fire==0)
					&&(damage->Electrical==0)
					&&(damage->Acid>0)
					) {
					DoPredatorAcidSound(sbPtr);
				} else {
					DoPredatorHitSound(sbPtr);
				}

                hitdelta=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta");
        
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

                if (hitdelta) {
                        /* A hierarchy with hit deltas! */
                        if (hitdelta->Playing==0) {
                                
                                int CrouchSubSequence;
                                int StandSubSequence;

                                if (Section==NULL) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitChestBack;
                                                StandSubSequence=PSSS_HitChestBack;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitChestFront;
                                                StandSubSequence=PSSS_HitChestFront;
                                        }
                                } else if (Section->sempai->flags&section_flag_head) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitHeadBack;
                                                StandSubSequence=PSSS_HitHeadBack;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitHeadFront;
                                                StandSubSequence=PSSS_HitHeadFront;
                                        }
                                } else if ((Section->sempai->flags&section_flag_left_arm)
                                        ||(Section->sempai->flags&section_flag_left_hand)) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitRightArm;
                                                StandSubSequence=PSSS_HitRightArm;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitLeftArm;
                                                StandSubSequence=PSSS_HitLeftArm;
                                        }
                                } else if ((Section->sempai->flags&section_flag_right_arm)
                                        ||(Section->sempai->flags&section_flag_right_hand)) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitLeftArm;
                                                StandSubSequence=PSSS_HitLeftArm;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitRightArm;
                                                StandSubSequence=PSSS_HitRightArm;
                                        }
                                } else if ((Section->sempai->flags&section_flag_left_leg)
                                        ||(Section->sempai->flags&section_flag_left_foot)) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitRightLeg;
                                                StandSubSequence=PSSS_HitRightLeg;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitLeftLeg;
                                                StandSubSequence=PSSS_HitLeftLeg;
                                        }
                                } else if ((Section->sempai->flags&section_flag_right_leg)
                                        ||(Section->sempai->flags&section_flag_right_foot)) {
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitLeftLeg;
                                                StandSubSequence=PSSS_HitLeftLeg;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitRightLeg;
                                                StandSubSequence=PSSS_HitRightLeg;
                                        }
                                } else {
                                        /* Chest or misc. hit. */
                                        if (frontback==0) {
                                                CrouchSubSequence=PCrSS_HitChestBack;
                                                StandSubSequence=PSSS_HitChestBack;
                                        } else {
                                                CrouchSubSequence=PCrSS_HitChestFront;
                                                StandSubSequence=PSSS_HitChestFront;
                                        }
                                }
                                

                                if(predatorStatusPointer->IAmCrouched) {
                                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,(int)HMSQT_PredatorCrouch,CrouchSubSequence)) {
                                                Start_Delta_Sequence(hitdelta,(int)HMSQT_PredatorCrouch,CrouchSubSequence,-1);
                                        }
                                } else {
                                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,(int)HMSQT_PredatorStand,StandSubSequence)) {
                                                Start_Delta_Sequence(hitdelta,(int)HMSQT_PredatorStand,StandSubSequence,-1);
                                        }
                                }
                                hitdelta->Playing=1;
                                /* Not looped. */
                        }
                }
        
                /* Break out of recover. */
                if (predatorStatusPointer->behaviourState==PBS_Recovering) {
                        Predator_Enter_Swapping_State(sbPtr);
                }

        }
}

/* Patrick 4/7/97 --------------------------------------------------
  The various far state behaviour execution functions for predator...
  
  1. Wandering is the initial far state, to which the predator never
  returns: after becoming visible for the first time, it will use only
  hunt and retreat
  2. Hunting is used if the predator feels confident enough to engage
  the player.
  3. Retreating is used if the predator is not confident    
---------------------------------------------------------------------*/
static PRED_RETURN_CONDITION Execute_PFS_Wander(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        AIMODULE *targetModule = 0;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* Decrement the Far state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;   
        /* check if far state timer has timed-out. If so, it is time 
        to do something. Otherwise just return. */
        if(predatorStatusPointer->stateTimer > 0) {
                return(PRC_No_Change);
        }

        /* check for state changes */
        if(PredatorIsAwareOfTarget(sbPtr))
        {
                /* we should be hunting */
                return(PRC_Request_Engage);
        }

        /* Preds NEVER camp.  I mean, get tired of wandering. */
                        
        /* timer has timed-out in roving mode */
        targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,NULL,0);

        /* if there is no target module, it means that the pred is trapped in an
        unlinked module. In this case, reset the timer and return. */                   
        if(!targetModule)
        {
                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                return(PRC_No_Change);
        }
        /* Examine target, and decide what to do */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));         
        ProcessFarPredatorTargetAIModule(sbPtr, targetModule);  
        /* reset timer */
        predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PFS_Hunt(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        AIMODULE *targetModule = 0;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* Decrement the Far state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;   
        /* check if far state timer has timed-out. If so, it is time 
        to do something. Otherwise just return. */
        if(predatorStatusPointer->stateTimer > 0) {
                return(PRC_No_Change);
        }
        
        if ((!PredatorIsAwareOfTarget(sbPtr))
                ||(predatorStatusPointer->Target!=Player->ObStrategyBlock))
        {
                /* I have a bad feeling about this. */
                return(PRC_Request_Wander);
        }
                        
        /* timer has timed-out in hunting mode: */
        targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,0);

        /* if there is no target module, it means that the pred is trapped in an
        unlinked module. In this case, reset the timer and return. */                   
        if(!targetModule)
        {
                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                return(PRC_No_Change);
        }

        /* NB don't need to check for state changes... will regen health on makenear */

        /* Examine target, and decide what to do */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));         
        ProcessFarPredatorTargetAIModule(sbPtr, targetModule);  
        /* reset timer */
        predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;                                 
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PFS_Engage(STRATEGYBLOCK *sbPtr)
{
        /* If we're far, we should be hunting, surely? */       

        return(PRC_Request_Hunt);

}

static PRED_RETURN_CONDITION Execute_PFS_Retreat(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        AIMODULE *targetModule = 0;
        AIMODULE *old_fearmodule;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        old_fearmodule=predatorStatusPointer->fearmodule;

        /* Decrement the state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;   

        if(!(PredatorIsAwareOfTarget(sbPtr))) {
                /* What am I running from? */
                return(PRC_Request_Recover);
        }

        /* check for state changes: randomly decide to switch to recover... */

        if (predatorStatusPointer->incidentFlag) {
                if (!(PredatorCanSeeTarget(sbPtr))) {
                        return(PRC_Request_Recover);
                }
        }

        if(predatorStatusPointer->stateTimer > 0) {
                return(PRC_No_Change);
        }

        /* timer has timed-out in retreat mode: */

        /* Yeah, from where _am_ I running? */
        if(PredatorIsAwareOfTarget(sbPtr)) {
                predatorStatusPointer->fearmodule=predatorStatusPointer->Target->containingModule->m_aimodule;
        } else if (predatorStatusPointer->fearmodule==NULL) {
                predatorStatusPointer->fearmodule=sbPtr->containingModule->m_aimodule;
        }       

        if ((predatorStatusPointer->missionmodule==NULL)||(predatorStatusPointer->fearmodule!=old_fearmodule)) {

                /* Recompute mission module. */
                if (predatorStatusPointer->fearmodule) {
                        predatorStatusPointer->missionmodule = General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->fearmodule,5);
                } else {
                        predatorStatusPointer->missionmodule = General_GetAIModuleForRetreat(sbPtr,Player->ObStrategyBlock->containingModule->m_aimodule,5);
                }
                
        }

        if (predatorStatusPointer->missionmodule==NULL) {
                /* Hey, it'll drop through. */
                return(PRC_Request_Recover);
        }

        targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,predatorStatusPointer->missionmodule,6,0);

        if(!targetModule)
        {
                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                return(PRC_Request_Recover);
        }

        /* Examine target, and decide what to do */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));         
        ProcessFarPredatorTargetAIModule(sbPtr, targetModule);  
        /* reset timer */
        predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;                                 
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PFS_Avoidance(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);

        /* High on the list of Things Not To Be Doing. */

        #if ALL_NEW_AVOIDANCE_PRED
        Initialise_AvoidanceManager(sbPtr,&predatorStatusPointer->avoidanceManager);
        #endif

        switch (predatorStatusPointer->lastState) {
                case PBS_Recovering:
                        return(PRC_Request_Recover);
                        break;
                case PBS_Hunting:
                        return(PRC_Request_Hunt);
                        break;
                case PBS_Engaging:
                        /* Go directly to approach.  Do not pass GO.  Do not collect 200 zorkmids. */
                        return(PRC_Request_Engage);
                        break;
                default:
                        return(PRC_Request_Wander);
                        break;
        }
        /* Still here? */
        return(PRC_Request_Wander);

}

static void ProcessFarPredatorTargetAIModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
        NPC_TARGETMODULESTATUS targetStatus;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        
        LOCALASSERT(sbPtr);
        LOCALASSERT(targetModule);
        LOCALASSERT(sbPtr->I_SBtype == I_BehaviourPredator);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);
        LOCALASSERT(predatorStatusPointer);
            
        targetStatus = GetTargetAIModuleStatus(sbPtr, targetModule,0);
        switch(targetStatus)
        {
                case(NPCTM_NoEntryPoint):
                {
                        /* do nothing: can't get in. reset lastVisitedModule to avoid getting 
                        stuck in a conceptual dead end (?)*/
                        FarNpc_FlipAround(sbPtr);
                        break;
                }
                case(NPCTM_NormalRoom):
                {
                        /* locate to target     */
                        LocateFarNPCInAIModule(sbPtr, targetModule);
                        break;
                }
                case(NPCTM_AirDuct):
                {
                        LocateFarNPCInAIModule(sbPtr, targetModule);
                        break;
                }
                case(NPCTM_LiftTeleport):
                {
                        /* do nothing - predators can't go into lifts   */
                        FarNpc_FlipAround(sbPtr);
                        break;
                }
                case(NPCTM_ProxDoorOpen):
                {
                        /* locate to target: don't need to move thro'quick, as door is constantly retriggered   */
                        LocateFarNPCInAIModule(sbPtr, targetModule);
                        break;
                }               
                case(NPCTM_ProxDoorNotOpen):
                {
                        /* trigger the door, and set timer to quick so we can catch the door when it's open */
                        MODULE *renderModule;
                        renderModule=*(targetModule->m_module_ptrs);
                        /* trigger the door, and set timer to quick so we can catch the door when it's open */
                        ((PROXDOOR_BEHAV_BLOCK *)renderModule->m_sbptr->SBdataptr)->alienTrigger = 1;
                        break;
                }
                case(NPCTM_LiftDoorOpen):
                {
                        /* do nothing - can't use lifts */
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
                        /* locate to target, and move thro' quick as we can't retrigger */
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

static PRED_RETURN_CONDITION Execute_PNS_DischargePistol(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH orientationDirn,relPos,relPos2;
        int correctlyOrientated,range;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* Always turn to face... */
        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);

        /* Fix weapon target! */
        {
                predatorStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
                        200);
                predatorStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
                        200);
                predatorStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
                        200);
        }
		/* Aim up a little? */
		range=VectorDistance((&predatorStatusPointer->weaponTarget),(&sbPtr->DynPtr->Position));
		
		if (range>3000) {
			predatorStatusPointer->weaponTarget.vy-=(range/6);
		} else {
			/* I'm afraid it just won't work. */
			return(PRC_Request_Swap);
		}
        /* Out of range? */
        if(range > predatorStatusPointer->Selected_Weapon->MaxRange)
        {
				/* Return to approach... */
                return(PRC_Request_Engage);
        }

        /* orientate to firing point first */
        if (predatorStatusPointer->My_Elevation_Section) {
                /* Assume range large w.r.t. half shoulder width... */
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - predatorStatusPointer->My_Elevation_Section->World_Offset.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - predatorStatusPointer->My_Elevation_Section->World_Offset.vz;
        } else {
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
        }
        /* Target shift? */
        correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

        /* Decloaking? */
        if (predatorStatusPointer->CloakStatus==PCLOAK_On) {
                PredatorCloakOff(predatorStatusPointer);
        }

        /* If still tweening, pause. */
        if (predatorStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
                return(PRC_No_Change);
        }
        
        if (predatorStatusPointer->stateTimer==predatorStatusPointer->Selected_Weapon->FiringRate) {
                
                predatorStatusPointer->HModelController.Playing=0;

                /* Only terminate if you haven't fired yet... */
                if(!PredatorCanSeeTarget(sbPtr))
                {
                        #if 1
                        /* ... and remove the gunflash */
                        #endif
        
                        return(PRC_Request_Engage);
                }
        
                /* we are not correctly orientated to the target: this could happen because we have
                just entered this state, or the target has moved during firing*/
                if((!correctlyOrientated)||(predatorStatusPointer->CloakStatus!=PCLOAK_Off))
                {
                        #if 1
                        /* stop visual and audio cues: technically, we're not firing at this moment */
                        #endif
                        return(PRC_No_Change);
                }
                
                /* If you are correctly oriented, you can now fire! */

                predatorStatusPointer->HModelController.Playing=1;
                predatorStatusPointer->HModelController.sequence_timer=0;

                relPos.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
                relPos.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
                relPos.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
                          
                relPos2.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vx);
                relPos2.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vy);
                relPos2.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vz);
                
                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
        
                #if 1
                /* look after the gun flash */
                #endif
                
                /* look after the sound */
                Sound_Play(SID_PRED_PISTOL,"d",&(sbPtr->DynPtr->Position));
        
                /* Now fire a bolt. */

                {
                        SECTION_DATA *muzzle;

                        muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");
                        
                        CreatePPPlasmaBoltKernel(&muzzle->World_Offset, &muzzle->SecMat,0);
                        predatorStatusPointer->volleySize++;
                }

                if (predatorStatusPointer->volleySize>3) {
                        predatorStatusPointer->enableSwap=1;
                }
        }       

        predatorStatusPointer->stateTimer -= NormalFrameTime;

        /* You must have fired already. */

        if(predatorStatusPointer->stateTimer > 0)       {
                return(PRC_No_Change);
        }
        
        if(range < predatorStatusPointer->Selected_Weapon->MinRange)
        {
                /* renew firing, as we are still too close to approach */ 
                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                predatorStatusPointer->volleySize = 0;
                return(PRC_No_Change);
        }
        else
        {
                NPC_DATA *NpcData;
                /* we are far enough away, so return to approach? */

                NpcData=GetThisNpcData(I_NPC_Predator);
                
                if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                        &&(sbPtr->SBDamageBlock.Health>0)) {
                        /* 50% health? */
                        if (General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->Target->containingModule->m_aimodule,5)) {
                                return(PRC_Request_Withdraw);
                        } else {
                                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                return(PRC_No_Change);
                        }
                }

                if (predatorStatusPointer->volleySize>=predatorStatusPointer->Selected_Weapon->VolleySize) {
                        if (((FastRandom()&65535)>32767)&&(predatorStatusPointer->enableSwap)) {
                                /* Change weapon! */
                                return(PRC_Request_Swap);
                        } else {
                                return(PRC_Request_Engage);
                        }
                } else {
                        /* And another! */
                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                        return(PRC_No_Change);
                }
        }
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_AttackWithPlasmaCaster(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH orientationDirn,relPos,relPos2;
        int correctlyOrientated,range,onTarget;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        if (predatorStatusPointer->internalState==0) {
                if (predatorStatusPointer->HModelController.Tweening==Controller_NoTweening) {
                        predatorStatusPointer->HModelController.Playing=0;
                }
        }

        /* Always turn to face... */
        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);
        
        /* orientate to firing point first */
        if (predatorStatusPointer->My_Elevation_Section) {
                /* Assume range large w.r.t. half shoulder width... */
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - predatorStatusPointer->My_Elevation_Section->World_Offset.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - predatorStatusPointer->My_Elevation_Section->World_Offset.vz;
        } else {
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
        }
        /* Target shift? */
        correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

        /* Decloaking? */
        if (predatorStatusPointer->CloakStatus==PCLOAK_On) {
                PredatorCloakOff(predatorStatusPointer);
        }

        /* Project three dots? */
        onTarget=DoPredatorLaserTargeting(sbPtr);

		if (predatorStatusPointer->Target->I_SBtype==I_BehaviourAutoGun) {
	        predatorStatusPointer->stateTimer -= (NormalFrameTime<<2);
		} else {
	        predatorStatusPointer->stateTimer -= NormalFrameTime;
		}

        /* Only terminate if you haven't fired yet... */
        if(!PredatorCanSeeTarget(sbPtr))
        {
                #if 1
                /* ... and remove the gunflash */
                #endif
        
                return(PRC_Request_Engage);
        }

        if (predatorStatusPointer->internalState==1) {
                /* Using stateTimer, thanks! */
        } else if(predatorStatusPointer->stateTimer > 0)        {
                return(PRC_No_Change);
        }

        /* State timed out - try to fire! */
        
        /* we are not correctly orientated to the target: this could happen because we have
        just entered this state, or the target has moved during firing*/
        if((!correctlyOrientated)||(predatorStatusPointer->CloakStatus!=PCLOAK_Off))
        {
                #if 1
                /* stop visual and audio cues: technically, we're not firing at this moment */
                #endif
                return(PRC_No_Change);
        }

        if (correctlyOrientated&&(predatorStatusPointer->internalState!=2)) {
                if (predatorStatusPointer->internalState!=1) {
                        predatorStatusPointer->internalState=1;
                        /* Pausing. */
	                    predatorStatusPointer->stateTimer=(ONE_FIXED);
                }
        }

        range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

        if ((predatorStatusPointer->internalState==1)&&(predatorStatusPointer->stateTimer<=0)) {
                if (onTarget) {         
                        
                        /* If you are correctly oriented, you can now fire!  Or taunt instead? */
                        
                        if (((FastRandom()&65535)<32767)&&(predatorStatusPointer->enableTaunt)
                        	&&(predatorStatusPointer->Target->I_SBtype!=I_BehaviourAutoGun)) {
                                /* Suprise! */
                                return(PRC_Request_Taunt);
                        }
                        /* Fire at least once! */
                        predatorStatusPointer->enableTaunt=1;
                
                        predatorStatusPointer->HModelController.Playing=1;
                        predatorStatusPointer->HModelController.Looped=0;
                        predatorStatusPointer->HModelController.sequence_timer=0;
                        predatorStatusPointer->internalState=2;
                
                        relPos.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
                        relPos.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
                        relPos.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
                                  
                        relPos2.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vx);
                        relPos2.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vy);
                        relPos2.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vz);
                        
                        #if 1
                        /* look after the gun flash */
                        #endif
                        
                        /* look after the sound */
                        Sound_Play(SID_PRED_LAUNCHER,"d",&(sbPtr->DynPtr->Position));
                
                        /* Now fire a bolt. */
                
                        {
                                SECTION_DATA *muzzle;
                
                                muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");
                                
                                InitialiseEnergyBoltBehaviourKernel(&muzzle->World_Offset, &muzzle->SecMat,0,&TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty],65536);
                                predatorStatusPointer->volleySize++;
                        }
                        predatorStatusPointer->stateTimer=(ONE_FIXED);
                        predatorStatusPointer->enableSwap=1;
                        return(SRC_No_Change);
                } else {
                        /* You think you're correctly orientated - but you're still not hitting. */
                        if (predatorStatusPointer->stateTimer<=0) {
                                /* Oh, just give up. */
                                predatorStatusPointer->HModelController.Playing=1;
                                predatorStatusPointer->HModelController.Looped=0;
                                predatorStatusPointer->HModelController.sequence_timer=0;
                                predatorStatusPointer->internalState=2;
                        }
                }
        }

        if (predatorStatusPointer->internalState==2) {

                /* After shot. */

                if (predatorStatusPointer->HModelController.sequence_timer<(ONE_FIXED-1)) {
                        /* Still playing. */
                        return(PRC_No_Change);
                }

                if(range < predatorStatusPointer->Selected_Weapon->MinRange)
                {
                        /* renew firing, as we are still too close to approach */ 
                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                        predatorStatusPointer->volleySize = 0;
                        predatorStatusPointer->internalState=0;
                        return(PRC_No_Change);
                }
                else
                {
                        NPC_DATA *NpcData;
                        /* we are far enough away, so return to approach? */
        
                        NpcData=GetThisNpcData(I_NPC_Predator);
                        
                        if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                                &&(sbPtr->SBDamageBlock.Health>0)) {
                                /* 50% health? */
                                if (General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->Target->containingModule->m_aimodule,5)) {
                                        return(PRC_Request_Withdraw);
                                } else {
                                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                        predatorStatusPointer->internalState=0;
                                        return(PRC_No_Change);
                                }
                        }
                
                        if (predatorStatusPointer->volleySize>=predatorStatusPointer->Selected_Weapon->VolleySize) {
                                if ((range<PRED_CLOSE_ATTACK_RANGE)||((FastRandom()&65535)>52000)) {
                                        /* Change weapon! */
                                        return(PRC_Request_Swap);
                                } else {
                                        return(PRC_Request_Engage);
                                }
                        } else {
                                /* And another! */
                                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                predatorStatusPointer->internalState=0;
                                return(PRC_No_Change);
                        }
                }

        }       
        
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Avoidance(STRATEGYBLOCK *sbPtr)
{
        int terminateState = 0;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);            
        LOCALASSERT(predatorStatusPointer);                             

        /* first check for a close attack... */
        if(VectorDistance((&Player->ObStrategyBlock->DynPtr->Position),(&sbPtr->DynPtr->Position)) < PRED_CLOSE_ATTACK_RANGE)
        {       
                return(PRC_Request_Attack);
        }

        /* New avoidance kernel. */

        NPCSetVelocity(sbPtr, &(predatorStatusPointer->avoidanceManager.avoidanceDirection), (predatorStatusPointer->nearSpeed));
        /* Velocity CANNOT be zero, unless deliberately so! */  
        {
                AVOIDANCE_RETURN_CONDITION rc;
                
                rc=AllNewAvoidanceKernel(sbPtr,&predatorStatusPointer->avoidanceManager);

                if (rc!=AvRC_Avoidance) {
                        terminateState=1;
                }
        }

        PredatorHandleMovingAnimation(sbPtr);

        if(terminateState)
        {
                /* Better exit. */
                switch (predatorStatusPointer->lastState) {
                        case PBS_Avoidance:
                        default:
                                /* switch to approach */
                                if(PredatorShouldAttackPlayer(sbPtr))
                                {
                                        /* go to approach */
                                        NPC_InitMovementData(&(predatorStatusPointer->moveData));
                                        return(PRC_Request_Engage);
                                }
                                else
                                {
                                        /* go to retreat */
                                        NPC_InitMovementData(&(predatorStatusPointer->moveData));               
                                        return(PRC_Request_Withdraw);
                                }
                                break;
                        case PBS_Wandering:
                                return(PRC_Request_Wander);
                                break;
                        case PBS_Hunting:
                                return(PRC_Request_Hunt);
                                break;
                        case PBS_Withdrawing:
                                return(PRC_Request_Withdraw);
                                break;
                        case PBS_Engaging:
                        case PBS_Attacking:
                                return(PRC_Request_Engage);
                                break;
                        case PBS_Returning:
                                return(PRC_Request_Return);
                                break;
                        case PBS_Pathfinding:
                                return(PRC_Request_Pathfind);
                                break;
                }
        }
        return(PRC_No_Change);
}

#if 0
static PRED_RETURN_CONDITION Execute_PNS_Approach(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH velocityDirection = {0,0,0};
        VECTORCH targetPosition;
        int targetIsAirduct = 0;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        LOCALASSERT(sbPtr->DynPtr);

        GLOBALASSERT(0);
        /* Function unused! */

        /* Make some noise if not cloaked */
        if (predatorStatusPointer->CloakStatus == PCLOAK_Off)
        {
			#if 0
          unsigned int random=FastRandom() & 127;
          switch (random)
          {
            case 0:
            {
              Sound_Play(SID_PRED_SNARL,"d",&sbPtr->DynPtr->Position);
                break;
            }
            case 1:
            {
              Sound_Play(SID_PRED_SCREAM1,"d",&sbPtr->DynPtr->Position);
              break;
            }
            case 2:
            {
              Sound_Play(SID_PRED_LOUDROAR,"d",&sbPtr->DynPtr->Position);
              break;
            }
                        case 3:
            {
              Sound_Play(SID_PRED_SHORTROAR,"d",&sbPtr->DynPtr->Position);
              break;
                  }
      default:
      {
        break;
      }
        }
			#else
			DoPredatorRandomSound(sbPtr);
			#endif
        }

        PredatorHandleMovingAnimation(sbPtr);
                                
        /* check for state changes...*/
        
        /* now check if we want to close-attack */
        if(VectorDistance((&Player->ObStrategyBlock->DynPtr->Position),(&sbPtr->DynPtr->Position)) < PRED_CLOSE_ATTACK_RANGE)
        {       
                return(PRC_Request_Attack);
        }

        /* now check if we should stand ground */
        if(!(PredatorShouldAttackPlayer(sbPtr)))
        {
                return(PRC_Request_Withdraw);
        }


        /* decrement the state timer if it is > 0. When this timer expires, predator
           will fire at player when it can next see him (or her) */
        if(predatorStatusPointer->stateTimer > 0) predatorStatusPointer->stateTimer -= NormalFrameTime;
        if(predatorStatusPointer->stateTimer <= 0)
        {
                /* if we can see the player, switch to fire */
                if(NPCCanSeeTarget(sbPtr,Player->ObStrategyBlock, PRED_NEAR_VIEW_WIDTH))
                {
                        return(PRC_Request_Attack);
                }
                else 
                {
                        /* reset the timer if we couldn't fire */
                        predatorStatusPointer->stateTimer = predatorCV[predatorStatusPointer->personalNumber].timeBetweenRangedAttacks;
                }

        }

        /* get and set approach velocity */
        NPCGetMovementTarget(sbPtr, Player->ObStrategyBlock, &targetPosition, &targetIsAirduct,0);
        NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        return(PRC_No_Change);
}
#endif

static PRED_RETURN_CONDITION Execute_PNS_EngageWithPistol(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH velocityDirection = {0,0,0};
        VECTORCH targetPosition;
        int targetIsAirduct = 0;
        int range;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        
        PredatorHandleMovingAnimation(sbPtr);

        /* now check for state changes... firstly, if we can no longer attack the target, go
        to wander */
        if(!(PredatorIsAwareOfTarget(sbPtr)))
        {
                return(PRC_Request_Hunt);
                /* Drop out null targets. */
        } else {
        
                /* We have a target that we are aware of. */

                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

                /* Wanna Cloak? */
                if (predatorStatusPointer->CloakStatus==PCLOAK_Off) {
                        PredatorCloakOn(predatorStatusPointer);
                }
                
                /* if we are close... go directly to firing */
                if(range < PRED_CLOSE_ATTACK_RANGE)
                {       
                        /* switch directly to firing, at this distance */
                
                        return(PRC_Request_Attack);
                }
                
                /* if our state timer has run out in approach state, see if we can fire*/
                if(predatorStatusPointer->stateTimer > 0) predatorStatusPointer->stateTimer -= NormalFrameTime;
                if(predatorStatusPointer->stateTimer <= 0)
                {
                        /* it is time to fire, if we can see the target  */
                        if(PredatorCanSeeTarget(sbPtr))
                        {
                                /* we are going to fire then */         
                
                                return(PRC_Request_Attack);
                        }
                        else
                        {
                                /* renew approach state */
                                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                                /* Whatever. */
                        }
                }
        }

        /* See which way we want to go. */
        {
        
                AIMODULE *targetModule;
                MODULE *tcm;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

                GLOBALASSERT(predatorStatusPointer->Target!=NULL);

                if (predatorStatusPointer->Target->containingModule) {
                        tcm=predatorStatusPointer->Target->containingModule;
                } else {
                        tcm=ModuleFromPosition(&predatorStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
                }

                if (tcm) {              
                        targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
                } else {
                        targetModule=NULL;
                }

                if (targetModule==sbPtr->containingModule->m_aimodule) {
                        /* Try going for it, we still can't see them. */
                        NPCGetMovementTarget(sbPtr, predatorStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
                        NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&predatorStatusPointer->waypointManager);
                } else if (!targetModule) {
                        /* Must be inaccessible. */
                        if (ShowPredoStats) {
                                if (predatorStatusPointer->Target->containingModule) {
                                        PrintDebuggingText("I can see you, but I can't get there!\n");
                                } else {
                                        PrintDebuggingText("Hey, you've got no Containing Module!\n");
                                }
                        }
                        return(PRC_No_Change);
                } else {

                        thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                        if (!thisEp) {
                                LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
                                        (*(targetModule->m_module_ptrs))->name,
                                        sbPtr->containingModule->name));
                                GLOBALASSERT(thisEp);
                        }
                        /* If that fired, there's a farped adjacency. */
                
                        predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                        predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                        predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                        predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

                        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
                }
                                
        }

        /* Should have a velocity set now. */

        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;

                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if(predatorStatusPointer->obstruction.environment)
                {
                        /* go to avoidance */
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &targetPosition, &velocityDirection))
        {

                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;
        
                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_EngageWithPlasmaCaster(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH velocityDirection = {0,0,0};
        VECTORCH targetPosition;
        int targetIsAirduct = 0;
        int range;

        /* For the moment, the same as the pistol. */
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        PredatorHandleMovingAnimation(sbPtr);

        predatorStatusPointer->patience-=NormalFrameTime;
        /* Have we become impatient? */
        if (predatorStatusPointer->patience<=0) {
                return(PRC_Request_Swap);
        }
                        
        /* now check for state changes... firstly, if we can no longer attack the target, go
        to wander */
        if(!(PredatorIsAwareOfTarget(sbPtr)))
        {
                return(PRC_Request_Hunt);
                /* Drop out null targets. */
        } else {
        
                /* We have a target that we are aware of. */

                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

                /* if we are close... go directly to firing */
                if(range < PRED_CLOSE_ATTACK_RANGE)
                {       
                        /* switch directly to firing, at this distance */
                
                        return(PRC_Request_Attack);
                } else {

                        /* Wanna Cloak? */
                        if (predatorStatusPointer->CloakStatus==PCLOAK_Off) {
                                PredatorCloakOn(predatorStatusPointer);
                        }
                
                }
                
                /* if our state timer has run out in approach state, see if we can fire*/
                if(predatorStatusPointer->stateTimer > 0) predatorStatusPointer->stateTimer -= NormalFrameTime;
                if(predatorStatusPointer->stateTimer <= 0)
                {
                        /* it is time to fire, if we can see the target  */
                        if(PredatorCanSeeTarget(sbPtr))
                        {
                                /* we are going to fire then */         
                
                                return(PRC_Request_Attack);
                        }
                        else
                        {
                                /* renew approach state */
                                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                                /* Whatever. */
                        }
                }
        }

        /* See which way we want to go. */
        {
        
                AIMODULE *targetModule;
                MODULE *tcm;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

                GLOBALASSERT(predatorStatusPointer->Target!=NULL);

                if (predatorStatusPointer->Target->containingModule) {
                        tcm=predatorStatusPointer->Target->containingModule;
                } else {
                        tcm=ModuleFromPosition(&predatorStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
                }

                if (tcm) {              
                        targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
                } else {
                        targetModule=NULL;
                }

                if (targetModule==sbPtr->containingModule->m_aimodule) {
                        /* Try going for it, we still can't see them. */
                        NPCGetMovementTarget(sbPtr, predatorStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
                        NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&predatorStatusPointer->waypointManager);
                } else if (!targetModule) {
                        /* Must be inaccessible. */
                        if (ShowPredoStats) {
                                if (predatorStatusPointer->Target->containingModule) {
                                        PrintDebuggingText("I can see you, but I can't get there!\n");
                                } else {
                                        PrintDebuggingText("Hey, you've got no Containing Module!\n");
                                }
                        }
                        return(PRC_No_Change);
                } else {

                        thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                        if (!thisEp) {
                                LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
                                        (*(targetModule->m_module_ptrs))->name,
                                        sbPtr->containingModule->name));
                                GLOBALASSERT(thisEp);
                        }
                        /* If that fired, there's a farped adjacency. */
                
                        predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                        predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                        predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                        predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

                        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
                }
                                
        }

        /* Should have a velocity set now. */

        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;

                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if(predatorStatusPointer->obstruction.environment)
                {
                        /* go to avoidance */
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &targetPosition, &velocityDirection))
        {

                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;
        
                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_EngageWithWristblade(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH velocityDirection = {0,0,0};
        VECTORCH targetPosition;
        int targetIsAirduct = 0;
        int range;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        PredatorHandleMovingAnimation(sbPtr);

        predatorStatusPointer->patience-=NormalFrameTime;
        /* Have we become impatient? */
        if (predatorStatusPointer->patience<=0) {
                return(PRC_Request_Swap);
        }
                        
        /* now check for state changes... firstly, if we can no longer attack the target, go
        to wander */
        if(!(PredatorIsAwareOfTarget(sbPtr)))
        {
                return(PRC_Request_Hunt);
                /* Drop out null targets. */
        } else {
        
                /* We have a target that we are aware of. */

                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

                /* Wanna Cloak? */
                if (range<5000) {
                        /* Decloak when close. */
                        PredatorCloakOff(predatorStatusPointer);
                } else if (predatorStatusPointer->CloakStatus==PCLOAK_Off) {
                        PredatorCloakOn(predatorStatusPointer);
                }
                
                /* if we are close... go directly to firing */
                if(range < PRED_CLOSE_ATTACK_RANGE)
                {       
                        /* switch directly to firing, at this distance */
                
                        return(PRC_Request_Attack);
                }
                
                /* if our state timer has run out in approach state, see if we can fire*/
                if(predatorStatusPointer->stateTimer > 0) {
                        predatorStatusPointer->stateTimer -= NormalFrameTime;
                }

                if(predatorStatusPointer->stateTimer <= 0)
                {
                        /* A moment of decision... */
                        /* Might want to withdraw. */
                        if(!PredatorCanSeeTarget(sbPtr))
                        {
                                /* Lost sight of him! */                
                
                                return(PRC_Request_Hunt);
                        }
                        else
                        {
                                /* renew approach state */
                                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                                /* Whatever. */
                        }
                }
        }

        /* See which way we want to go. */
        {
        
                AIMODULE *targetModule;
                MODULE *tcm;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

                GLOBALASSERT(predatorStatusPointer->Target!=NULL);

                if (predatorStatusPointer->Target->containingModule) {
                        tcm=predatorStatusPointer->Target->containingModule;
                } else {
                        tcm=ModuleFromPosition(&predatorStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
                }

                if (tcm) {              
                        targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
                } else {
                        targetModule=NULL;
                }

                if (targetModule==sbPtr->containingModule->m_aimodule) {
                        /* Try going for it, we still can't see them. */
                        NPCGetMovementTarget(sbPtr, predatorStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
                        NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&predatorStatusPointer->waypointManager);
                } else if (!targetModule) {
                        /* Must be inaccessible. */
                        if (ShowPredoStats) {
                                if (predatorStatusPointer->Target->containingModule) {
                                        PrintDebuggingText("I can see you, but I can't get there!\n");
                                } else {
                                        PrintDebuggingText("Hey, you've got no Containing Module!\n");
                                }
                        }
                        return(PRC_No_Change);
                } else {

                        thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                        if (!thisEp) {
                                LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
                                        (*(targetModule->m_module_ptrs))->name,
                                        sbPtr->containingModule->name));
                                GLOBALASSERT(thisEp);
                        }
                        /* If that fired, there's a farped adjacency. */
                
                        predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                        predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                        predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                        predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

                        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
                }
                                
        }

        /* Should have a velocity set now. */

        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;

                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if(predatorStatusPointer->obstruction.environment)
                {
                        /* go to avoidance */
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &targetPosition, &velocityDirection))
        {

                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;

                predatorStatusPointer->patience-=ONE_FIXED;
        
                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_AttackWithWristblade(STRATEGYBLOCK *sbPtr)
{
        VECTORCH orientationDirn;
        int i;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        LOCALASSERT (sbPtr->DynPtr);

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        dynPtr = sbPtr->DynPtr;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* De-cloak. */
        if(predatorStatusPointer->CloakStatus==PCLOAK_On)
        {
                PredatorCloakOff(predatorStatusPointer);
        }

        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);
        
        /* check for state changes: */
        if(VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position)) > PRED_CLOSE_ATTACK_RANGE)
        {
                if (((FastRandom()&65535)>32767)&&(predatorStatusPointer->enableSwap)) {
                        /* Change weapon! */
                        return(PRC_Request_Swap);
                } else {
                        /* switch back to engage. */            
                        GLOBALASSERT(predatorStatusPointer->Target);
                        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);
                        return(PRC_Request_Engage);
                }
        }

        /* Orientate towards player, just to make sure we're facing */
        orientationDirn.vx = predatorStatusPointer->Target->DynPtr->Position.vx - sbPtr->DynPtr->Position.vx;
        orientationDirn.vy = 0;
        orientationDirn.vz = predatorStatusPointer->Target->DynPtr->Position.vz - sbPtr->DynPtr->Position.vz;
        i = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
        
        /* Decrement the near state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;

        PredatorNearDamageShell(sbPtr);

        if (predatorStatusPointer->HModelController.keyframe_flags&1) {

                predatorStatusPointer->enableSwap=1;
                StartWristbladeAttackSequence(sbPtr);
                predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;
                /* Shrug. */
        }
        return(PRC_No_Change);

}

#if 0
static PRED_RETURN_CONDITION Execute_PNS_StandGround(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* basically just stand there and fire... */

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;
        
        /* check if we want to close-attack */
        if(VectorDistance((&Player->ObStrategyBlock->DynPtr->Position),(&sbPtr->DynPtr->Position)) < PRED_CLOSE_ATTACK_RANGE)
        {       
                return(PRC_Request_Attack);
        }

        /* decrement the state timer if it is > 0. When this timer expires, predator
           will fire at player when it can next see him (or her) */
        if(predatorStatusPointer->stateTimer > 0) predatorStatusPointer->stateTimer -= NormalFrameTime;
        if(predatorStatusPointer->stateTimer <= 0)
        {
                /* if we can see the player, switch to fire */
                if(NPCCanSeeTarget(sbPtr,Player->ObStrategyBlock, PRED_NEAR_VIEW_WIDTH))
                {
                        return(PRC_Request_Attack);
                }
                else 
                {
                        /* reset the timer if we couldn't fire */
                        predatorStatusPointer->stateTimer = predatorCV[predatorStatusPointer->personalNumber].timeBetweenRangedAttacks;
                }
        }
        return(PRC_No_Change);
}
#endif

static PRED_RETURN_CONDITION Execute_PNS_SwapWeapon(STRATEGYBLOCK *sbPtr) {
        
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->ChangeToWeapon==PNPCW_End) {
                /* Rubbish! */
                return(PRC_Request_Hunt);
        }

		Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
			"Predator Swap Weapon Start");

        if (predatorStatusPointer->stateTimer==0) {
                /* Haven't started yet. */
                if (predatorStatusPointer->ChangeToWeapon==predatorStatusPointer->Selected_Weapon->id) {
                        /* Stop wasting my time! */
                        return(PRC_Request_Hunt);
                }

                if (ShowPredoStats) {
                        PrintDebuggingText("Part one ");
                }

                /* Decloak. */
                if (predatorStatusPointer->CloakStatus!=PCLOAK_Off) {
                        /* Sorry, I want to see this. */
                        return(PRC_No_Change);
                }

                /* Right.  Is there a 'deselect' anim? */
                if (predatorStatusPointer->IAmCrouched) {
                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,
                                (int)HMSQT_PredatorCrouch,(int)PCrSS_Get_Weapon)) {
                                /* It's there! */
                                if (predatorStatusPointer->Selected_Weapon->SwappingTime!=0) {
                                        /* Valid swap time, too. */
                                        InitHModelTweening_Backwards(&predatorStatusPointer->HModelController,
                                                (ONE_FIXED>>2),(int)HMSQT_PredatorCrouch,(int)PCrSS_Get_Weapon,
                                                predatorStatusPointer->Selected_Weapon->SwappingTime,0);
                                        predatorStatusPointer->HModelController.Looped=0;
                                        predatorStatusPointer->stateTimer=1; /* Swapping Out. */
                                        return(PRC_No_Change);
                                }
                        }
                } else {
                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,
                                (int)HMSQT_PredatorStand,(int)PSSS_Get_Weapon)) {
                                /* It's there! */
                                if (predatorStatusPointer->Selected_Weapon->SwappingTime!=0) {
                                        /* Valid swap time, too. */
                                        InitHModelTweening_Backwards(&predatorStatusPointer->HModelController,
                                                (ONE_FIXED>>2),(int)HMSQT_PredatorStand,(int)PSSS_Get_Weapon,
                                                predatorStatusPointer->Selected_Weapon->SwappingTime,0);
                                        predatorStatusPointer->HModelController.Looped=0;
                                        predatorStatusPointer->stateTimer=1; /* Swapping Out. */
                                        return(PRC_No_Change);
                                }
                        }
                }
                /* If you're still here, there must be no swapping out sequence. */
                predatorStatusPointer->stateTimer=2;
                /* Ah well, go directly to the middle. */
                return(PRC_No_Change);
        } else if (predatorStatusPointer->stateTimer==1) {
                /* You are in the process of swapping out. */

                if (ShowPredoStats) {
                        PrintDebuggingText("Part two ");
                }

                if (HModelAnimation_IsFinished(&predatorStatusPointer->HModelController)) {
                        /* Right, you've finished! */
                        predatorStatusPointer->stateTimer=2;
                        return(PRC_No_Change);
                } else {
                        GLOBALASSERT(predatorStatusPointer->HModelController.Looped==0);
                        return(PRC_No_Change);
                }
        } else if (predatorStatusPointer->stateTimer==2) {
                /* In the middle! */
                PREDATOR_WEAPON_DATA *targetWeapon;
                SECTION *root_section;

                if (ShowPredoStats) {
                        PrintDebuggingText("Part three ");
                }

                targetWeapon=GetThisNPCPredatorWeapon(predatorStatusPointer->ChangeToWeapon);
                if (!targetWeapon) {
                        /* We're screwed.  Stay with the old one! */
                        predatorStatusPointer->ChangeToWeapon=PNPCW_End;
                        return(PRC_Request_Hunt);
                }
                predatorStatusPointer->Selected_Weapon=targetWeapon;
                root_section=GetNamedHierarchyFromLibrary(predatorStatusPointer->Selected_Weapon->Riffname,predatorStatusPointer->Selected_Weapon->HierarchyName);
                GLOBALASSERT(root_section);
                
                /* Strip out HitDelta for now, if any... */
                {
                        DELTA_CONTROLLER *delta;
                        delta=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta");
                        if (delta) {
                                Remove_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta");
                        }
                }

                /* Strip off elevation too. */
                {
                        DELTA_CONTROLLER *delta;
                        delta=Get_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
                        if (delta) {
                                Remove_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
                        }
                }

				Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
					"Predator Swap Weapon Two");

				/* In the interests of getting the new sections right... */
                predatorStatusPointer->HModelController.Sequence_Type=(int)HMSQT_PredatorStand;
				predatorStatusPointer->HModelController.Sub_Sequence=(int)PSSS_Get_Weapon;

                Transmogrify_HModels(sbPtr,&predatorStatusPointer->HModelController,root_section,0,1,0);
                
                #if PREDATOR_HIT_DELTAS
                if (HModelSequence_Exists(&predatorStatusPointer->HModelController,(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront)) {
                        /* This ritual in case _one_ of the hierarchies doesn't have hitdeltas. */
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatusPointer->HModelController,"HitDelta",(int)HMSQT_PredatorStand,(int)PSSS_HitChestFront,-1);
                        GLOBALASSERT(delta);
                        delta->Playing=0;
                }
                #endif

                /* Replace elevation. */
                if (predatorStatusPointer->Selected_Weapon->UseElevation) {
					if (Get_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation")==NULL) {
                        DELTA_CONTROLLER *delta;
                        delta=Add_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation",(int)HMSQT_PredatorStand,(int)PSSS_Elevation,0);
                        GLOBALASSERT(delta);
                    	delta->timer=32767;
                    }
                } else {
					/* Better make sure it's gone... */
                    Remove_Delta_Sequence(&predatorStatusPointer->HModelController,"Elevation");
                }

				Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
					"Predator Swap Weapon Two A");

				DeInitialise_HModel(&predatorStatusPointer->HModelController);
                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);
        
				Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
					"Predator Swap Weapon Three");

                predatorStatusPointer->My_Gun_Section=GetThisSectionData(predatorStatusPointer->HModelController.section_data,predatorStatusPointer->Selected_Weapon->GunName);
                predatorStatusPointer->My_Elevation_Section=GetThisSectionData(predatorStatusPointer->HModelController.section_data,predatorStatusPointer->Selected_Weapon->ElevationName);

                /* Now go for the Get_Weapon sequence. */
                if (predatorStatusPointer->IAmCrouched) {
                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,
                                (int)HMSQT_PredatorCrouch,(int)PCrSS_Get_Weapon)) {
                                /* It's there! */
                                if (predatorStatusPointer->Selected_Weapon->SwappingTime!=0) {
                                        /* Valid swap time, too. */
                                        predatorStatusPointer->HModelController.Sequence_Type=(int)HMSQT_PredatorCrouch;
                                        predatorStatusPointer->HModelController.Sub_Sequence=(int)PCrSS_Get_Weapon;
                                        predatorStatusPointer->HModelController.Seconds_For_Sequence=predatorStatusPointer->Selected_Weapon->SwappingTime;
                                        /* That to get the new sections right. */
                                        InitHModelTweening(&predatorStatusPointer->HModelController,
                                                (ONE_FIXED>>2),(int)HMSQT_PredatorCrouch,(int)PCrSS_Get_Weapon,
                                                predatorStatusPointer->Selected_Weapon->SwappingTime,0);
                                        predatorStatusPointer->HModelController.Looped=0;
                                        predatorStatusPointer->stateTimer=3; /* Swapping In. */

										Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
											"Predator Swap Weapon Three A");

						                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);

										Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
											"Predator Swap Weapon Four");

                                        return(PRC_No_Change);
                                }
                        }
                } else {
                        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,
                                (int)HMSQT_PredatorStand,(int)PSSS_Get_Weapon)) {
                                /* It's there! */
                                if (predatorStatusPointer->Selected_Weapon->SwappingTime!=0) {
                                        /* Valid swap time, too. */
                                        predatorStatusPointer->HModelController.Sequence_Type=(int)HMSQT_PredatorStand;
                                        predatorStatusPointer->HModelController.Sub_Sequence=(int)PSSS_Get_Weapon;
                                        predatorStatusPointer->HModelController.Seconds_For_Sequence=predatorStatusPointer->Selected_Weapon->SwappingTime;
                                        /* That to get the new sections right. */
                                        InitHModelTweening(&predatorStatusPointer->HModelController,
                                                (ONE_FIXED>>2),(int)HMSQT_PredatorStand,(int)PSSS_Get_Weapon,
                                                predatorStatusPointer->Selected_Weapon->SwappingTime,0);
                                        predatorStatusPointer->HModelController.Looped=0;
                                        predatorStatusPointer->stateTimer=3; /* Swapping In. */

										Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
											"Predator Swap Weapon Four A");

						                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);

										Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
											"Predator Swap Weapon Five");

                                        return(PRC_No_Change);
                                }
                        }
                }

                /* If you're still here, there must be no swapping in sequence. */
                predatorStatusPointer->stateTimer=4;
                /* Ah well, go directly to the end. */

				Verify_Positions_In_HModel(sbPtr,&predatorStatusPointer->HModelController,
					"Predator Swap Weapon Six");

                return(PRC_No_Change);
                
        } else if (predatorStatusPointer->stateTimer==3) {
                /* You are in the process of swapping in. */
                
                if (ShowPredoStats) {
                        PrintDebuggingText("Part four ");
                }

                if (HModelAnimation_IsFinished(&predatorStatusPointer->HModelController)) {
                        /* Right, you've finished! */
                        predatorStatusPointer->stateTimer=4;
                        return(PRC_No_Change);
                } else {
                        GLOBALASSERT(predatorStatusPointer->HModelController.Looped==0);
                        return(PRC_No_Change);
                }
        } else if (predatorStatusPointer->stateTimer==4) {
                /* All (valid) conclusions arrive here. */
                
                if (ShowPredoStats) {
                        PrintDebuggingText("Part five ");
                }

                predatorStatusPointer->ChangeToWeapon=PNPCW_End;
                if (predatorStatusPointer->Selected_Weapon->id==PNPCW_Medicomp) {
                        return(PRC_Request_Recover);
                } else {
                        return(PRC_Request_Attack);
                }
        }

        if (ShowPredoStats) {
                PrintDebuggingText("No Part %d ",predatorStatusPointer->stateTimer);
        }

        return(PRC_No_Change);

}

static PRED_RETURN_CONDITION Execute_PNS_Wander(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;
        DYNAMICSBLOCK *dynPtr;
        VECTORCH velocityDirection = {0,0,0};
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);
                
        PredatorHandleMovingAnimation(sbPtr);

        /* should we change to approach state? */
        if (PredatorIsAwareOfTarget(sbPtr))
        {
                /* doesn't require a sequence change */
                return(PRC_Request_Engage);
        }

        /* Are we using bimble rules? */

        if (predatorStatusPointer->wanderData.currentModule==NPC_BIMBLINGINMODULE) {
        
                int range;

                /* Range to target... */

                range=VectorDistance((&predatorStatusPointer->wanderData.worldPosition),(&sbPtr->DynPtr->Position));

                if (range<2000) {

                        /* Reset system, try again. */
                        predatorStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;

                }
                
        }
        else
        {
                /* wander target aquisition: if no target, or moved module */
                LOCALASSERT(sbPtr->containingModule);
                if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
                {
                        NPC_InitMovementData(&(predatorStatusPointer->moveData));
                        NPC_FindAIWanderTarget(sbPtr,&(predatorStatusPointer->wanderData),&(predatorStatusPointer->moveData),0);
                }
                else if(predatorStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)
                {
                        NPC_FindAIWanderTarget(sbPtr,&(predatorStatusPointer->wanderData),&(predatorStatusPointer->moveData),0);
                }
                
                /* if we still haven't got one, bimble about in this one for a bit. */
                if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
                {
                        NPC_GetBimbleTarget(sbPtr,&predatorStatusPointer->wanderData.worldPosition);
                        predatorStatusPointer->wanderData.currentModule=NPC_BIMBLINGINMODULE;
                }
                
        }
                
        /* ok: should have a current target at this stage... */
        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager))
        {
                        /* Go to all new avoidance. */
            return(PRC_Request_Avoidance);
        }

        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Hunt(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        VECTORCH velocityDirection = {0,0,0};

        /* Your mission: to advance into the players module, even if near. */
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);
                
        PredatorHandleMovingAnimation(sbPtr);

        /* should we change to approach state? */
        if(PredatorCanSeeTarget(sbPtr))
        {
                return(PRC_Request_Engage);
        } else if(!(PredatorIsAwareOfTarget(sbPtr))) {
                /* I have a bad feeling about this, too. */
                //return(PRC_No_Change);
				/* Might just be in a non-vis module now... */
        }

        {
                AIMODULE *targetModule;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

                targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,0);

                if (targetModule) {
                        if (ShowPredoStats) {
                                PrintDebuggingText("Target Module %s.\n",(*(targetModule->m_module_ptrs))->name);
                        }
                } else {
                        if (ShowPredoStats) {
                                PrintDebuggingText("Target Module NULL.\n");
                        }
                }

                if (targetModule==sbPtr->containingModule->m_aimodule) {
                        /* Hey, it'll drop through. */
						if (predatorStatusPointer->Target==NULL) {
	                        return(PRC_Request_Wander);
						} else {
	                        return(PRC_Request_Engage);
						}
                }

                if (!targetModule) {
                        #if 1
                        /* Must be sealed off. */
                        return(PRC_Request_Recover);
                        #else
                        extern MODULE *playerPherModule;
                        
                        LOGDXFMT(("Jules's bug: predator is in %s, player is in %s",sbPtr->containingModule->name,playerPherModule->name));
                        GLOBALASSERT(targetModule);
                        #endif
                }                               

                thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                if (!thisEp) {
                        LOGDXFMT(("This assert is a busted adjacency!"));
                        GLOBALASSERT(thisEp);
                }
                /* If that fired, there's a farped adjacency. */
        
                predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
                
        }

        /* ok: should have a current target at this stage... */
        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;

                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if((predatorStatusPointer->obstruction.environment)||(predatorStatusPointer->obstruction.otherCharacter))
                {
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &(predatorStatusPointer->wanderData.worldPosition), &velocityDirection))
        {
                /* go to avoidance */
                /* no sequence change required */
                
                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;

                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Retreat(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        VECTORCH velocityDirection = {0,0,0};
        AIMODULE *old_fearmodule;

        /* Your mission: to advance out of trouble, even if near. */
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);
                
        PredatorHandleMovingAnimation(sbPtr);

        old_fearmodule=predatorStatusPointer->fearmodule;

        if(!(PredatorIsAwareOfTarget(sbPtr))) {
                /* What am I running from? */
                return(PRC_Request_Recover);
        }

        #if 0
        if (predatorStatusPointer->incidentFlag) {
                /* Need a better test here. */
                if (!(PredatorCanSeeTarget(sbPtr))) {
                        return(PRC_Request_Recover);
                }
        }
        #endif

        /* Yeah, from where _am_ I running? */
        if(PredatorIsAwareOfTarget(sbPtr)) {
                predatorStatusPointer->fearmodule=predatorStatusPointer->Target->containingModule->m_aimodule;
        } else if (predatorStatusPointer->fearmodule==NULL) {
                predatorStatusPointer->fearmodule=sbPtr->containingModule->m_aimodule;
        }       

        if ((predatorStatusPointer->missionmodule==NULL)||(predatorStatusPointer->fearmodule!=old_fearmodule)) {

                /* Recompute mission module. */
                if (predatorStatusPointer->fearmodule) {
                        predatorStatusPointer->missionmodule = General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->fearmodule,5);
                } else {
                        predatorStatusPointer->missionmodule = General_GetAIModuleForRetreat(sbPtr,Player->ObStrategyBlock->containingModule->m_aimodule,5);
                }
                
        }

        {
                AIMODULE *targetModule;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
  
                if (predatorStatusPointer->missionmodule==NULL) {
                        /* Hey, it'll drop through. */
                        return(PRC_Request_Recover);
                }

                if (ShowPredoStats) {
                        PrintDebuggingText("Target Module %s.\n",(*(predatorStatusPointer->missionmodule->m_module_ptrs))->name);
                }

                targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,predatorStatusPointer->missionmodule,6,0);
                
                if (targetModule) {
                        if (ShowPredoStats) {
                                PrintDebuggingText("Next Module is %s.\n",(*(targetModule->m_module_ptrs))->name);
                        }
                } else {
                        if (ShowPredoStats) {
                                PrintDebuggingText("Next Module is NULL!\n");
                        }
                }

                if ((targetModule==sbPtr->containingModule->m_aimodule)
                        || (targetModule==NULL)) {
                        /* Hey, it'll drop through. */
                        return(PRC_Request_Recover);
                }
                
                GLOBALASSERT(targetModule);
                
                thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                if (!thisEp) {
                        LOGDXFMT(("This assert is a busted adjacency!"));
                        GLOBALASSERT(thisEp);
                }
                /* If that fired, there's a farped adjacency. */
        
                predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
                
        }

        /* ok: should have a current target at this stage... */
        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;

                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if((predatorStatusPointer->obstruction.environment)||(predatorStatusPointer->obstruction.otherCharacter))
                {
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        #if 1
        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &(predatorStatusPointer->wanderData.worldPosition), &velocityDirection))
        {
                /* go to avoidance */
                /* no sequence change required */
                
                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;

                return(PRC_Request_Avoidance);
        }
        #endif
        #endif
        return(PRC_No_Change);
}

static void Execute_Dying(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predStatusPointer;       

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        predStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);        
        LOCALASSERT(predStatusPointer);
        
        /* make sure we're not cloaked... if activating, ignore until fully on */
        if(predStatusPointer->CloakStatus==PCLOAK_On)
        {
                PredatorCloakOff(predStatusPointer);
                Sound_Play(SID_VISION_ON,"d",&sbPtr->DynPtr->Position);
        }
        
        sbPtr->DynPtr->LinImpulse.vx = 0;
        sbPtr->DynPtr->LinImpulse.vy = 0;
        sbPtr->DynPtr->LinImpulse.vz = 0;
        
        predStatusPointer->stateTimer -= NormalFrameTime;
}

#if 0
/* Patrick 4/7/97 --------------------------------------------------
Behaviour support functions 
---------------------------------------------------------------------*/
#define PRED_PLASMASPEED 30000
#define PRED_DISCSPEED 30000
static void CreateNPCPredatorPlasBolt(VECTORCH *startingPosition, VECTORCH *targetDirection)
{
        DISPLAYBLOCK *dPtr;
        DYNAMICSBLOCK *dynPtr;
        
        /* make displayblock with correct shape, etc */
        dPtr = MakeObject(I_BehaviourPredatorEnergyBolt,startingPosition);
        if(!dPtr) return;       
        LOCALASSERT(dPtr->ObStrategyBlock);

        /* add lighting effect */
        AddLightingEffectToObject(dPtr,LFX_ROCKETJET);  

        /* setup dynamics block */
        dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
        if(!dynPtr)
        {
                RemoveBehaviourStrategy(dPtr->ObStrategyBlock);
                return;
        }

        dPtr->ObStrategyBlock->DynPtr = dynPtr;

        /* give projectile a maximum lifetime */
        dPtr->ObStrategyBlock->SBdataptr = AllocateMem(sizeof(CASTER_BOLT_BEHAV_BLOCK));
        if(!dPtr->ObStrategyBlock->SBdataptr)
        {
                RemoveBehaviourStrategy(dPtr->ObStrategyBlock);
                return;
        }

        ((CASTER_BOLT_BEHAV_BLOCK *)dPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;                   
        ((CASTER_BOLT_BEHAV_BLOCK *)dPtr->ObStrategyBlock->SBdataptr)->damage = TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty];
        ((CASTER_BOLT_BEHAV_BLOCK *)dPtr->ObStrategyBlock->SBdataptr)->blast_radius = 2000;
        /* align projectile to launcher */
        dynPtr->Position = *startingPosition;
        dynPtr->PrevPosition=dynPtr->Position;
        MakeMatrixFromDirection(targetDirection, &(dynPtr->OrientMat));
        dynPtr->PrevOrientMat = dynPtr->OrientMat; /* stops mis-alignment if dynamics problem */            

        /* set velocity */      
    dynPtr->LinVelocity.vx = MUL_FIXED(targetDirection->vx,PRED_PLASMASPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(targetDirection->vy,PRED_PLASMASPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(targetDirection->vz,PRED_PLASMASPEED);
}

static void CreateNPCPredatorDisc(VECTORCH *startingPosition, VECTORCH *targetDirection)
{
        DISPLAYBLOCK *dPtr;
        DYNAMICSBLOCK *dynPtr;
        
        /* make displayblock with correct shape, etc */
        dPtr = MakeObject(I_BehaviourNPCPredatorDisc,startingPosition);
        if(!dPtr) return;       
        LOCALASSERT(dPtr->ObStrategyBlock);

        /* add lighting effect */
        dPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

        /* setup dynamics block */
        dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
        if(!dynPtr)
        {
                RemoveBehaviourStrategy(dPtr->ObStrategyBlock);
                return;
        }

        dPtr->ObStrategyBlock->DynPtr = dynPtr;

        /* give projectile a maximum lifetime */
        dPtr->ObStrategyBlock->SBdataptr = AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK));
        if(!dPtr->ObStrategyBlock->SBdataptr)
        {
                RemoveBehaviourStrategy(dPtr->ObStrategyBlock);
                return;
        }

        ((ONE_SHOT_BEHAV_BLOCK *)dPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;                      
        /* align projectile to launcher */
        dynPtr->Position = *startingPosition;
        MakeMatrixFromDirection(targetDirection, &(dynPtr->OrientMat));
        dynPtr->PrevOrientMat = dynPtr->OrientMat; /* stops mis-alignment if dynamics problem */            

        /* set velocity */      
    dynPtr->LinVelocity.vx = MUL_FIXED(targetDirection->vx,PRED_DISCSPEED);
    dynPtr->LinVelocity.vy = MUL_FIXED(targetDirection->vy,PRED_DISCSPEED);
    dynPtr->LinVelocity.vz = MUL_FIXED(targetDirection->vz,PRED_DISCSPEED);
}
#endif

static void SetPredatorAnimationSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening)
{

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        GLOBALASSERT(length!=0);

        if (tweening<=0) {
                InitHModelSequence(&predatorStatusPointer->HModelController,(int)type,subtype,length);
        } else {        
                InitHModelTweening(&predatorStatusPointer->HModelController, tweening, (int)type,subtype,length, 1);
        }
}

static int PredatorShouldAttackPlayer(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predStatusPointer;       

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        predStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);        
        LOCALASSERT(predStatusPointer);

        if(predStatusPointer->health > predatorCV[predStatusPointer->personalNumber].defenceHealth)
                return 1;
        else
                return 0;
}

static int PredatorShouldBeCrawling(STRATEGYBLOCK *sbPtr)
{
        if(sbPtr->containingModule->m_flags & MODULEFLAG_AIRDUCT) return 1;
        return 0;
}

/* Patrick 21/8/97 ----------------------------------------------------
Predator cloak interface functions...
-----------------------------------------------------------------------*/
static void InitPredatorCloak(PREDATOR_STATUS_BLOCK *predStatus)
{
        predStatus->CloakStatus = PCLOAK_Off;
        predStatus->CloakingEffectiveness = 0;            
        predStatus->CloakTimer = 0;
}

static void PredatorCloakOn(PREDATOR_STATUS_BLOCK *predStatus)
{
        LOCALASSERT(predStatus);
        
        #if 0
        /* Temp! */
        return;
        #else
        if (predStatus->CloakStatus==PCLOAK_On) {
                return;
        } else if (predStatus->CloakStatus==PCLOAK_Deactivating) {
                /* Don't reset timer. */
                predStatus->CloakStatus = PCLOAK_Activating;
                return;
        } else if (predStatus->CloakStatus==PCLOAK_Activating) {
                /* Okay, okay! */
                return;
        } else {
                /* Cloak ust be Off. */
                predStatus->CloakStatus = PCLOAK_Activating;            
                predStatus->CloakTimer = ONE_FIXED-1;
        }
        #endif
}

static void PredatorCloakOff(PREDATOR_STATUS_BLOCK *predStatus)
{
        LOCALASSERT(predStatus);

        if (predStatus->CloakStatus==PCLOAK_Off) {
                return;
        } else if (predStatus->CloakStatus==PCLOAK_Activating) {
                /* Don't reset timer. */
                predStatus->CloakStatus = PCLOAK_Deactivating;
                return;
        } else if (predStatus->CloakStatus==PCLOAK_Deactivating) {
                /* Okay, okay! */
                return;
        } else {
                /* Cloak must be On. */
                predStatus->CloakStatus = PCLOAK_Deactivating;          
                predStatus->CloakTimer = 0; /* Was predStatus->PredShimmer. */
        }
}


static void DoPredatorCloak(PREDATOR_STATUS_BLOCK *predStatus,DYNAMICSBLOCK *dynPtr)
{
        LOCALASSERT(predStatus);
        switch(predStatus->CloakStatus)
        {
                case(PCLOAK_Off):
                {
                        /* do nothing */
                        break;
                }
                case(PCLOAK_Activating):
                {
                        predStatus->CloakTimer -= (NormalFrameTime);
                        if(predStatus->CloakTimer<=0)
                        {
                                predStatus->CloakTimer=0;
                                predStatus->CloakStatus=PCLOAK_On;
                        }
                        break;
                }
                case(PCLOAK_On):
                {
                        break;
                }
                case(PCLOAK_Deactivating):
                {
                        predStatus->CloakTimer += (NormalFrameTime);
                        if(predStatus->CloakTimer>=(ONE_FIXED))
                        {
                                predStatus->CloakTimer=0;
                                predStatus->CloakStatus=PCLOAK_Off;
                        }                       
                        break;
                }
                default:
                {
                        LOCALASSERT(1==0);
                        break;
                }
        }

	/* KJL - cloaking effectiveness */
	if(predStatus->CloakStatus!=PCLOAK_Off)
	{
		int maxPossibleEffectiveness;
		VECTORCH velocity;

		velocity.vx = dynPtr->Position.vx - dynPtr->PrevPosition.vx;
		velocity.vy = dynPtr->Position.vy - dynPtr->PrevPosition.vy;
		velocity.vz = dynPtr->Position.vz - dynPtr->PrevPosition.vz;
		
		maxPossibleEffectiveness = ONE_FIXED - DIV_FIXED(Magnitude(&velocity)*4,NormalFrameTime);

		if (maxPossibleEffectiveness<0) maxPossibleEffectiveness = 0;

		predStatus->CloakingEffectiveness += NormalFrameTime;
		if (predStatus->CloakingEffectiveness>maxPossibleEffectiveness)
		{
			predStatus->CloakingEffectiveness = maxPossibleEffectiveness;
		}
	}
	else
	{
		predStatus->CloakingEffectiveness -= NormalFrameTime;
		if (predStatus->CloakingEffectiveness<0)
		{
			predStatus->CloakingEffectiveness=0;
		}
	}
//	PrintDebuggingText("cloaking effectiveness %d\n",predStatus->CloakingEffectiveness);

}

int NPCPredatorIsCloaked(STRATEGYBLOCK *sbPtr) {
        
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* For external calls. */

        switch(predatorStatusPointer->CloakStatus)
        {
                case(PCLOAK_Off):
                case(PCLOAK_Activating):
                case(PCLOAK_Deactivating):
                {
                        return(0);
                        break;
                }
                case(PCLOAK_On):
                {
                        return(1);
                        break;
                }
                default:
                {
                        LOCALASSERT(1==0);
                        break;
                }
        }

        return(0);
}

static int PredatorCanSeeTarget(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        int targetIsCloaked;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        if (predatorStatusPointer->Target==NULL) {
                /* You can't see nothin. */
                return(0);
        }

        /* Predators see... well, anything. */

        targetIsCloaked=0;

        if (predatorStatusPointer->Target==Player->ObStrategyBlock) {
                /* test for player hiding? */
        
        } else {
                /* Test for NPC aliens hiding? */
        }

        if(!(NPCCanSeeTarget(sbPtr,predatorStatusPointer->Target, 1000000))) {
                return(0);
        }

        if (targetIsCloaked) {
                return(0);
        }

        return(1);
}

static int PredatorCanSeeObject(STRATEGYBLOCK *sbPtr,STRATEGYBLOCK *target)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        int targetIsCloaked;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        if (target==NULL) {
                /* You can't see nothin. */
                return(0);
        }

        /* Predators see... well, anything. */

        targetIsCloaked=0;

        if (target==Player->ObStrategyBlock) {
                /* test for player hiding? */
        
        } else {
                /* Test for NPC aliens hiding? */
        }

        if(!(NPCCanSeeTarget(sbPtr,target, 1000000))) {
                return(0);
        }

        if (targetIsCloaked) {
                return(0);
        }

        return(1);
}

static int PredatorIsAwareOfTarget(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        if (predatorStatusPointer->Target==NULL) {
                /* You can't see nothin. */
                return(0);
        }

        /* Okay, let's cut to the chase. */
		
		/* Slightly more restrictive now... */
		if ((IsModuleVisibleFromModule(sbPtr->containingModule,
			predatorStatusPointer->Target->containingModule))) {
			return(1);
		} else {
			return(0);
		}

        return(1);
}

void Predator_Enter_Wander_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Wandering;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Hunt_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Hunting;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Recover_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        if (predatorStatusPointer->Selected_Weapon->id!=PNPCW_Medicomp) {
                /* Change to it. */
                predatorStatusPointer->ChangeToWeapon=PNPCW_Medicomp;
                NPC_InitMovementData(&(predatorStatusPointer->moveData));
                NPC_InitWanderData(&(predatorStatusPointer->wanderData));
                InitWaypointManager(&predatorStatusPointer->waypointManager);
                predatorStatusPointer->volleySize = 0;
                predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
                predatorStatusPointer->behaviourState = PBS_SwapWeapon;
                predatorStatusPointer->stateTimer = 0; /* Just starting. */
                predatorStatusPointer->Pred_Laser_On=0;

                /* Deal with sequence in a bit. */
                PredatorCloakOff(predatorStatusPointer);
                return;
        }

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Recovering;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrouch,PCrSS_Attack_Primary,ONE_FIXED,(ONE_FIXED>>3));         
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorStand,PSSS_Attack_Primary,ONE_FIXED,(ONE_FIXED>>3));
        }

}

void Predator_Enter_Withdrawal_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Withdrawing;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;
        predatorStatusPointer->Pred_Laser_On=0;
        
        /* Force re-evaluation of priorities. */
        predatorStatusPointer->missionmodule=NULL;
        predatorStatusPointer->fearmodule=NULL;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Engaged_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Engaging;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Attacking_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Attacking;
        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                                         
        predatorStatusPointer->Pred_Laser_On=0;
        predatorStatusPointer->internalState=0;
        /* Become patient again. */
        predatorStatusPointer->patience=PRED_PATIENCE_TIME;

        if (predatorStatusPointer->Selected_Weapon->id==PNPCW_Wristblade) {
                StartWristbladeAttackSequence(sbPtr);
        } else if (predatorStatusPointer->Selected_Weapon->id==PNPCW_Staff) {
                StartPredStaffAttackSequence(sbPtr);
        } else {
                if(predatorStatusPointer->IAmCrouched) {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrouch,PCrSS_Attack_Primary,-1,(ONE_FIXED>>3));        
                } else {
                        SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorStand,PSSS_Attack_Primary,-1,(ONE_FIXED>>3));
                }
        }
}

void Predator_Enter_Avoidance_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPCGetAvoidanceDirection(sbPtr, &(predatorStatusPointer->moveData.avoidanceDirn),&predatorStatusPointer->obstruction);
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Avoidance;
        predatorStatusPointer->stateTimer = NPC_AVOIDTIME;                                                              
        predatorStatusPointer->Pred_Laser_On=0;

        /* zero linear velocity in dynamics block */
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Swapping_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* What are we swapping to? */
        if (predatorStatusPointer->ChangeToWeapon==PNPCW_End) {
                /* Use default. */
                if (predatorStatusPointer->Selected_Weapon->id==predatorStatusPointer->PrimaryWeapon) {
                        /* Using primary - change to secondary. */
                        predatorStatusPointer->ChangeToWeapon=predatorStatusPointer->SecondaryWeapon;
                } else {
                        /* Using secondary or other - change to primary. */
                        predatorStatusPointer->ChangeToWeapon=predatorStatusPointer->PrimaryWeapon;
                }
        }

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_SwapWeapon;
        predatorStatusPointer->stateTimer = 0; /* Just starting. */
        predatorStatusPointer->Pred_Laser_On=0;
        predatorStatusPointer->patience=PRED_PATIENCE_TIME;

        predatorStatusPointer->enableSwap=0;

        /* Deal with sequence in a bit. */
        PredatorCloakOff(predatorStatusPointer);
}

void Predator_Enter_Return_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        GLOBALASSERT(predatorStatusPointer->path!=-1);
        GLOBALASSERT(predatorStatusPointer->stepnumber!=-1);

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Returning;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Pathfinder_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        
        GLOBALASSERT(predatorStatusPointer->path!=-1);
        GLOBALASSERT(predatorStatusPointer->stepnumber!=-1);

        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_Pathfinding;
        predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;                                                         
        predatorStatusPointer->Pred_Laser_On=0;

        #if 0
        if(predatorStatusPointer->IAmCrouched) {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorCrawl,PCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));                
        } else {
                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorRun,PRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
        }
        #endif

}

void Predator_Enter_Taunt_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        if (HModelSequence_Exists(&predatorStatusPointer->HModelController,(int)HMSQT_PredatorStand,(int)PSSS_Taunt_One)) {

                NPC_InitMovementData(&(predatorStatusPointer->moveData));
                NPCGetAvoidanceDirection(sbPtr, &(predatorStatusPointer->moveData.avoidanceDirn),&predatorStatusPointer->obstruction);
                InitWaypointManager(&predatorStatusPointer->waypointManager);
                predatorStatusPointer->volleySize = 0;
                predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
                predatorStatusPointer->behaviourState = PBS_Taunting;
                predatorStatusPointer->stateTimer = NPC_AVOIDTIME;                                                              
                predatorStatusPointer->Pred_Laser_On=0;
                /* Become patient again. */
                predatorStatusPointer->patience=PRED_PATIENCE_TIME;
                predatorStatusPointer->enableTaunt=0;

                SetPredatorAnimationSequence(sbPtr,HMSQT_PredatorStand,PSSS_Taunt_One,-1,(ONE_FIXED>>3));               
                predatorStatusPointer->HModelController.LoopAfterTweening=0;
				
				#if 0
                Sound_Play(SID_PRED_NEWROAR,"d",&sbPtr->DynPtr->Position);
				#else
				DoPredatorTauntSound(sbPtr);
				#endif

        } else {
                /* Yuck. */
                Predator_Enter_Engaged_State(sbPtr);
        }

}

void Predator_Enter_SelfDestruct_State(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        SECTION *root;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        
        /* Switch to template. */       
        root=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
        GLOBALASSERT(root);
        RemoveAllDeltas(&predatorStatusPointer->HModelController);
        /* Set 'new' sequence. */
        predatorStatusPointer->HModelController.Sequence_Type=HMSQT_PredatorCrouch;
        predatorStatusPointer->HModelController.Sub_Sequence=PCrSS_Det_Prog;
        Transmogrify_HModels(sbPtr,&predatorStatusPointer->HModelController,root, 1, 0,0);
        ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);

        InitHModelTweening(&predatorStatusPointer->HModelController, (ONE_FIXED>>3), 
                HMSQT_PredatorCrouch,PCrSS_Det_Prog,-1,0);

        /* Init stats. */
        NPC_InitMovementData(&(predatorStatusPointer->moveData));
        NPC_InitWanderData(&(predatorStatusPointer->wanderData));
        InitWaypointManager(&predatorStatusPointer->waypointManager);
        predatorStatusPointer->volleySize = 0;
        predatorStatusPointer->lastState=predatorStatusPointer->behaviourState;
        predatorStatusPointer->behaviourState = PBS_SelfDestruct;
        predatorStatusPointer->stateTimer = 0;
        predatorStatusPointer->Pred_Laser_On=0;
        predatorStatusPointer->internalState=0; /* Not yet primed. */
        predatorStatusPointer->IAmCrouched = 1;

}

void Predator_SwitchState(STRATEGYBLOCK *sbPtr,PRED_RETURN_CONDITION state_result) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        
        switch (state_result) {
                case (PRC_No_Change):
                {
                        /* No action. */
                        break;
                }
                case (PRC_Request_Engage):
                {
                        Predator_Enter_Engaged_State(sbPtr);
                        break;
                }
                case (PRC_Request_Attack):
                {
                        Predator_Enter_Attacking_State(sbPtr);
                        break;
                }
                case (PRC_Request_Wander):
                {
                        if ((predatorStatusPointer->path!=-1)&&
                                (predatorStatusPointer->stepnumber!=-1)) {
                                /* Pathfinding mission. */
                                if ((predatorStatusPointer->behaviourState==PBS_Returning)
                                        ||(predatorStatusPointer->behaviourState==PBS_Pathfinding)) {
                                        /* A real order. */
                                        Predator_Enter_Wander_State(sbPtr);
                                } else {
                                        /* Pathfind instead. */
                                        Predator_Enter_Pathfinder_State(sbPtr);
                                }
                        } else {
                                Predator_Enter_Wander_State(sbPtr);
                        }
                        break;
                }
                case (PRC_Request_Avoidance):
                {
                        Predator_Enter_Avoidance_State(sbPtr);
                        break;
                }
                case (PRC_Request_Hunt):
                {
                        Predator_Enter_Hunt_State(sbPtr);
                        break;
                }
                case (PRC_Request_Withdraw):
                {
                        Predator_Enter_Withdrawal_State(sbPtr);
                        break;
                }
                case (PRC_Request_Recover):
                {
                        Predator_Enter_Recover_State(sbPtr);
                        break;
                }
                case (PRC_Request_Swap):
                {
                        Predator_Enter_Swapping_State(sbPtr);
                        break;
                }
                case (PRC_Request_Pathfind):
                {
                        Predator_Enter_Pathfinder_State(sbPtr);
                        break;
                }
                case (PRC_Request_Return):
                {
                        Predator_Enter_Return_State(sbPtr);
                        break;
                }
                case (PRC_Request_Taunt):
                {
                        Predator_Enter_Taunt_State(sbPtr);
                        break;
                }
                default:
                {
                        /* No action? */
                        break;
                }

}

}

int Predator_TargetFilter(STRATEGYBLOCK *candidate) {

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
                                        case I_Marine:
                                                return(1);
                                                break;
                                        case I_Predator:
                                                /* Just this once. */
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
                                        case I_Alien:
                                                return(1);
                                                break;
                                        case I_Predator:
                                                return(0);
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
                                ALIEN_STATUS_BLOCK *alienStatusPointer;
                                LOCALASSERT(candidate); 
                                LOCALASSERT(candidate->DynPtr); 
                        
                                alienStatusPointer=(ALIEN_STATUS_BLOCK *)(candidate->SBdataptr);    
                                
                                if (NPC_IsDead(candidate)) {
                                        return(0);
                                } else {
                                        return(1);
                                }
                                break;
                        }
				case I_BehaviourAutoGun:
                    if (NPC_IsDead(candidate)) {
                            return(0);
                    } else {
                            AUTOGUN_STATUS_BLOCK *autogunStatusPointer;    
                            autogunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(candidate->SBdataptr);    
                            LOCALASSERT(autogunStatusPointer);
							
							if (autogunStatusPointer->behaviourState==I_inactive) {
	                            return(0);
							} else {
	                            return(1);
							}
                    }
					break;
                case I_BehaviourQueenAlien:
                case I_BehaviourFaceHugger:
                case I_BehaviourMarine:
                case I_BehaviourXenoborg:
                case I_BehaviourSeal:
                case I_BehaviourPredatorAlien:
                        /* Valid. */
                        return(1);
                        break;
                case I_BehaviourPredator:
                        #if ANARCHY
                        return(1);
                        #else
                        return(0);
                        #endif
                        break;
                case I_BehaviourNetGhost:
                        {
                                NETGHOSTDATABLOCK *dataptr;
                                dataptr=candidate->SBdataptr;
                                switch (dataptr->type) {
                                        case I_BehaviourMarinePlayer:
                                        case I_BehaviourAlienPlayer:
                                        case I_BehaviourPredatorPlayer:
                                                //return(1);
                                                return(0);
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


STRATEGYBLOCK *Predator_GetNewTarget(STRATEGYBLOCK *sbPtr) {

        #if 0
        /* Here's the simple one, for testing purposes. */
        return(Player->ObStrategyBlock);
        #else

        int neardist, newblip;
        STRATEGYBLOCK *nearest;

        int a;
        STRATEGYBLOCK *candidate;
        MODULE *dmod;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        int dist;
        VECTORCH offset;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);
        LOCALASSERT(predatorStatusPointer);                             
        
        dmod=ModuleFromPosition(&sbPtr->DynPtr->Position,playerPherModule);
        
        LOCALASSERT(dmod);
        
        nearest=NULL;
        neardist=ONE_FIXED;
        newblip=0;
        
        for (a=0; a<NumActiveStBlocks; a++) {
                candidate=ActiveStBlockList[a];
                if (candidate!=sbPtr) {
                        if (candidate->DynPtr) {
                                /* Arc reject. */
                                MATRIXCH WtoL;

                                offset.vx=sbPtr->DynPtr->Position.vx-candidate->DynPtr->Position.vx;
                                offset.vy=sbPtr->DynPtr->Position.vy-candidate->DynPtr->Position.vy;
                                offset.vz=sbPtr->DynPtr->Position.vz-candidate->DynPtr->Position.vz;
                        
                                WtoL=sbPtr->DynPtr->OrientMat;
                                TransposeMatrixCH(&WtoL);
                                RotateVector(&offset,&WtoL);

                                if (offset.vz<=0) {
                                        
                                        if (Predator_TargetFilter(candidate)) {
                                        
                                                dist=Approximate3dMagnitude(&offset);
                                        
                                                if (dist<neardist) {
                                                        /* Check visibility? */
                                                        if (candidate->SBdptr) {
                                                                /* Near case. */
                                                                if (!NPC_IsDead(candidate)) {
                                                                        if ((PredatorCanSeeObject(sbPtr,candidate))) {
                                                                                nearest=candidate;
                                                                                neardist=dist;
                                                                        }       
                                                                }
                                                        } else {
                                                                if (!NPC_IsDead(candidate)) {
                                                                        if ((IsModuleVisibleFromModule(dmod,candidate->containingModule))) {
                                                                                nearest=candidate;
                                                                                neardist=dist;
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
        #endif
        
}

int DoPredatorLaserTargeting(STRATEGYBLOCK *sbPtr) {

        int i,hits;
        MATRIXCH matrix;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        SECTION_DATA *plasma_muzzle;
        VECTORCH offset[3] =
        {
                {0,-50,0},
                {43,25,0},
                {-43,25,0},
        };
        STRATEGYBLOCK *laserhits[3];
        STRATEGYBLOCK *consensus;
        VECTORCH z_vec;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);
        LOCALASSERT(predatorStatusPointer);                             

        predatorStatusPointer->Pred_Laser_On=0;
        /* Reset it at the end! */

        plasma_muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");
        if (plasma_muzzle==NULL) {
                return(0);
        }
        
        matrix = plasma_muzzle->SecMat;

        z_vec.vx=matrix.mat31;
        z_vec.vy=matrix.mat32;
        z_vec.vz=matrix.mat33;

        TransposeMatrixCH(&matrix);

        predatorStatusPointer->Pred_Laser_Sight.LightSource=plasma_muzzle->World_Offset;
        predatorStatusPointer->Pred_Laser_Sight.DotIsOnPlayer=0;

        i=2;
        hits=0;
                        
        do {
                VECTORCH position = offset[i];
        
                RotateVector(&position,&matrix);
                position.vx += plasma_muzzle->World_Offset.vx;
                position.vy += plasma_muzzle->World_Offset.vy;
                position.vz += plasma_muzzle->World_Offset.vz;
                FindPolygonInLineOfSight(&z_vec, &position, 0,sbPtr->SBdptr);

                predatorStatusPointer->Pred_Laser_Sight.Normal[i] = LOS_ObjectNormal;
                predatorStatusPointer->Pred_Laser_Sight.Position[i] = LOS_Point;

                if (LOS_ObjectHitPtr==Player) {
                        predatorStatusPointer->Pred_Laser_Sight.DotIsOnPlayer=1;
                }

                if (LOS_ObjectHitPtr) {
                        laserhits[i]=LOS_ObjectHitPtr->ObStrategyBlock;
                } else {
                        laserhits[i]=NULL;
                }
                
                if (laserhits[i]) {
                        hits++;
                }
        } while(i--);

        /* Now, what have we hit? */

        consensus=NULL; 

        switch (hits) {
                case 0:
                default:
                        consensus=NULL;
                        break;
                case 1:
                        i=2;
                        do {
                                if (laserhits[i]!=NULL) {
                                        consensus=laserhits[i];
                                }
                        } while (i--);
                        break;
                case 2:
                        i=2;
                        do {
                                if (laserhits[i]!=NULL) {
                                        if (consensus==NULL) {
                                                consensus=laserhits[i];
                                        } else {
                                                if (laserhits[i]!=consensus) {
                                                        consensus=NULL;
                                                }
                                        }
                                }
                        } while (i--);
                        break;
                case 3:
                        if ((laserhits[0]==laserhits[1])||(laserhits[0]==laserhits[2])) {
                                consensus=laserhits[0];
                        } else if (laserhits[1]==laserhits[2]) {
                                consensus=laserhits[1];
                        } else {
                                consensus=NULL;
                        }
                        break;
        }

        predatorStatusPointer->Pred_Laser_On=1;
        
        if (predatorStatusPointer->Target==NULL) {
                return(0);
        }

        if (consensus==predatorStatusPointer->Target) {
                return(1);
        } else {
                return(0);
        }

}

static PRED_RETURN_CONDITION Execute_PNS_EngageWithStaff(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH velocityDirection = {0,0,0};
        VECTORCH targetPosition;
        int targetIsAirduct = 0;
        int range;

        /* Same as wristblade version. */       
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        PredatorHandleMovingAnimation(sbPtr);

        predatorStatusPointer->patience-=NormalFrameTime;
        /* Have we become impatient? */
        if (predatorStatusPointer->patience<=0) {
                return(PRC_Request_Swap);
        }
                        
        /* now check for state changes... firstly, if we can no longer attack the target, go
        to wander */
        if(!(PredatorIsAwareOfTarget(sbPtr)))
        {
                return(PRC_Request_Hunt);
                /* Drop out null targets. */
        } else {
        
                /* We have a target that we are aware of. */

                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

                /* Wanna Cloak? */
                if (range<10000) {
                        /* Decloak when close. */
                        PredatorCloakOff(predatorStatusPointer);
                } else if (predatorStatusPointer->CloakStatus==PCLOAK_Off) {
                        PredatorCloakOn(predatorStatusPointer);
                }
                
                /* if we are close... go directly to firing */
                if(range < PRED_STAFF_ATTACK_RANGE)
                {       
                        /* switch directly to firing, at this distance */
                
                        return(PRC_Request_Attack);
                }
                
                /* if our state timer has run out in approach state, see if we can fire*/
                if(predatorStatusPointer->stateTimer > 0) {
                        predatorStatusPointer->stateTimer -= NormalFrameTime;
                }

                if(predatorStatusPointer->stateTimer <= 0)
                {
                        /* A moment of decision... */
                        /* Might want to withdraw. */
                        if(!PredatorCanSeeTarget(sbPtr))
                        {
                                /* Lost sight of him! */                
                
                                return(PRC_Request_Hunt);
                        }
                        else
                        {
                                /* renew approach state */
                                predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
                                /* Whatever. */
                        }
                }
        }

        /* See which way we want to go. */
        {
        
                AIMODULE *targetModule;
                MODULE *tcm;
                FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

                GLOBALASSERT(predatorStatusPointer->Target!=NULL);

                if (predatorStatusPointer->Target->containingModule) {
                        tcm=predatorStatusPointer->Target->containingModule;
                } else {
                        tcm=ModuleFromPosition(&predatorStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
                }

                if (tcm) {              
                        targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
                } else {
                        targetModule=NULL;
                }

                if (targetModule==sbPtr->containingModule->m_aimodule) {
                        /* Try going for it, we still can't see them. */
                        NPCGetMovementTarget(sbPtr, predatorStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
                        NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&predatorStatusPointer->waypointManager);
                } else if (!targetModule) {
                        /* Must be inaccessible. */
                        if (ShowPredoStats) {
                                if (predatorStatusPointer->Target->containingModule) {
                                        PrintDebuggingText("I can see you, but I can't get there!\n");
                                } else {
                                        PrintDebuggingText("Hey, you've got no Containing Module!\n");
                                }
                        }
                        return(PRC_No_Change);
                } else {

                        thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
                        if (!thisEp) {
                                LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
                                        (*(targetModule->m_module_ptrs))->name,
                                        sbPtr->containingModule->name));
                                GLOBALASSERT(thisEp);
                        }

                        /* If that fired, there's a farped adjacency. */
                        predatorStatusPointer->wanderData.worldPosition=thisEp->position;
                        predatorStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
                        predatorStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
                        predatorStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

                        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
                }
                                
        }

        /* Should have a velocity set now. */
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager))
        {
                /* Go to all new avoidance. */
            return(PRC_Request_Avoidance);
        }

        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_AttackWithStaff(STRATEGYBLOCK *sbPtr)
{
        VECTORCH orientationDirn;
        int i;
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        LOCALASSERT (sbPtr->DynPtr);

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        dynPtr = sbPtr->DynPtr;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* De-cloak. */
        if(predatorStatusPointer->CloakStatus==PCLOAK_On)
        {
                PredatorCloakOff(predatorStatusPointer);
        }

        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);

        /* check for state changes: */
        if(VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position)) > PRED_STAFF_ATTACK_RANGE)
        {
                if (((FastRandom()&65535)>40000)&&(predatorStatusPointer->enableSwap)) {
                        /* Quite like the staff. */
                        return(PRC_Request_Swap);
                } else {
                        /* switch back to engage. */            
                        GLOBALASSERT(predatorStatusPointer->Target);
                        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);
                        return(PRC_Request_Engage);
                }
        }

        /* Orientate towards player, just to make sure we're facing */
        orientationDirn.vx = predatorStatusPointer->Target->DynPtr->Position.vx - sbPtr->DynPtr->Position.vx;
        orientationDirn.vy = 0;
        orientationDirn.vz = predatorStatusPointer->Target->DynPtr->Position.vz - sbPtr->DynPtr->Position.vz;
        /* Fudge? */
        orientationDirn.vx+=(predatorStatusPointer->Target->DynPtr->OrientMat.mat11>>8);
        orientationDirn.vz+=(predatorStatusPointer->Target->DynPtr->OrientMat.mat13>>8);
        i = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
        
        /* Decrement the near state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;

        PredatorNearDamageShell(sbPtr);

        #if 0
        /* Staff damage. */
        {
                SECTION_DATA *tip1,*mid,*tip2;

                tip1=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"staff a blade");
                mid=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"staff center");
                tip2=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"staff b blade");
        
                GLOBALASSERT(tip1);
                GLOBALASSERT(mid);
                GLOBALASSERT(tip2);

                Staff_Manager(&TemplateAmmo[AMMO_NPC_PRED_STAFF].MaxDamage[AvP.Difficulty],tip1,mid,tip2,sbPtr);

        }
        #endif

        if (predatorStatusPointer->HModelController.keyframe_flags&1) {

                //predatorStatusPointer->enableSwap=1;
                StartPredStaffAttackSequence(sbPtr);
                predatorStatusPointer->stateTimer = PRED_NEAR_CLOSEATTACK_TIME;
                /* Shrug. */
        }

        return(PRC_No_Change);

}

static PRED_RETURN_CONDITION Execute_PFS_Pathfinder(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        AIMODULE *targetModule = 0;
        int nextModuleIndex;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);                         

        /* Okay, so you're a LocalGuard or Pathfinder who's gotten lost. */

        /* Decrement the Far state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;   
        /* check if far state timer has timed-out. If so, it is time 
        to do something. Otherwise just return. */
        if(predatorStatusPointer->stateTimer > 0) return(PRC_No_Change);
                        
        /* check for state changes */
        if(PredatorIsAwareOfTarget(sbPtr))
        /* Hack! */
        {
                /* we should be engaging */
                return(PRC_Request_Engage);
        }

        /* Ignore alerts. */

        /* Never break out of pathfinder unless your life is in danger! */

        /* Okay, so where are we exactly? */

        if ((predatorStatusPointer->stepnumber<0)||(predatorStatusPointer->path<0)) {
                /* Get OUT! */
                return(PRC_Request_Wander);
        }

        targetModule = TranslatePathIndex(predatorStatusPointer->stepnumber,predatorStatusPointer->path);

        if (targetModule==NULL) {
                /* Oh, to heck with this.  Try to wander. */
                return(PRC_Request_Wander);
        }
        
        /* Right, so there is a somewhere to get to. */

        if (targetModule!=sbPtr->containingModule->m_aimodule) {
                /* But we're nowhere near it.  Geeze... */
                predatorStatusPointer->missionmodule=targetModule;
                return(PRC_Request_Return);
        }
        
        /* Okay, so now we need to know where to go now. */

        nextModuleIndex=GetNextModuleInPath(predatorStatusPointer->stepnumber,predatorStatusPointer->path);
        GLOBALASSERT(nextModuleIndex>=0);
        /* If that fires, it's Richard's fault. */
        targetModule=TranslatePathIndex(nextModuleIndex,predatorStatusPointer->path);
        GLOBALASSERT(targetModule);
        /* Ditto. */
        predatorStatusPointer->stepnumber=nextModuleIndex;

        /* Examine target, and decide what to do */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));         
        ProcessFarPredatorTargetAIModule(sbPtr, targetModule);  
        /* reset timer */
        predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Pathfinder(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        AIMODULE *targetModule;
        VECTORCH velocityDirection = {0,0,0};
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);
        
        PredatorHandleMovingAnimation(sbPtr);

        /* should we change to approach state? */
        if(PredatorIsAwareOfTarget(sbPtr))
        {
                /* doesn't require a sequence change */
                return(PRC_Request_Engage);
        }

        predatorStatusPointer->stateTimer-=NormalFrameTime;

        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
        {
                NPC_InitMovementData(&(predatorStatusPointer->moveData));
        }
        if ((predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
                ||(predatorStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)) {

                FARENTRYPOINT *thisEp;
                int nextModuleIndex;
                /* Okay, so where are we exactly? */

                if ((predatorStatusPointer->stepnumber<0)||(predatorStatusPointer->path<0)) {
                        /* Get OUT! */
                        return(PRC_Request_Wander);
                }
        
                targetModule = TranslatePathIndex(predatorStatusPointer->stepnumber,predatorStatusPointer->path);
        
                if (targetModule==NULL) {
                        /* Oh, to heck with this.  Try to wander. */
                        return(PRC_Request_Wander);
                }
                
                /* Right, so there is a somewhere to get to. */
        
                if (targetModule!=sbPtr->containingModule->m_aimodule) {
                        /* But we're nowhere near it.  Geeze... */
                        predatorStatusPointer->missionmodule=targetModule;
                        return(PRC_Request_Return);
                }
                
                /* Okay, so now we need to know where to go now. */
        
                nextModuleIndex=GetNextModuleInPath(predatorStatusPointer->stepnumber,predatorStatusPointer->path);
                GLOBALASSERT(nextModuleIndex>=0);
                /* If that fires, it's Richard's fault. */
                targetModule=TranslatePathIndex(nextModuleIndex,predatorStatusPointer->path);
                GLOBALASSERT(targetModule);
                /* Ditto. */
                predatorStatusPointer->stepnumber=nextModuleIndex;
                
                thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
                if(thisEp) {
                        /* aha. an ep!... */ 
                        VECTORCH thisEpWorld = thisEp->position;

                        thisEpWorld.vx += targetModule->m_world.vx;
                        thisEpWorld.vy += targetModule->m_world.vy;
                        thisEpWorld.vz += targetModule->m_world.vz;                     
                        
                        predatorStatusPointer->wanderData.currentModule = sbPtr->containingModule->m_aimodule->m_index;
                        predatorStatusPointer->wanderData.worldPosition = thisEpWorld;
                
                } else {
                        /* Failure case. */
                        predatorStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
                }

        }
        
        /* if we still haven't got one, wander for a bit. */
        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
        {
                NPC_InitMovementData(&(predatorStatusPointer->moveData));
                NPC_FindAIWanderTarget(sbPtr,&(predatorStatusPointer->wanderData),&(predatorStatusPointer->moveData),0);
        }

        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE) {
                /* STILL broken!  Okay, just... wander forever, then. */
                return(PRC_Request_Wander);
        }
                
        /* ok: should have a current target at this stage... */
        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;
                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if((predatorStatusPointer->obstruction.environment)||(predatorStatusPointer->obstruction.otherCharacter))
                {
                        /* go to avoidance */
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &(predatorStatusPointer->wanderData.worldPosition), &velocityDirection))
        {
                /* go to avoidance */
                /* no sequence change required */
                
                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;

                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PFS_Return(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        AIMODULE *targetModule = 0;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);                         

        /* Decrement the Far state timer */
        predatorStatusPointer->stateTimer -= NormalFrameTime;   
        /* check if far state timer has timed-out. If so, it is time 
        to do something. Otherwise just return. */
        if(predatorStatusPointer->stateTimer > 0) return(SRC_No_Change);
                        
        /* check for state changes */
        if(PredatorIsAwareOfTarget(sbPtr))
        /* Hack! */
        {
                /* we should be engaging */
                return(PRC_Request_Engage);
        }

        if (sbPtr->containingModule->m_aimodule==predatorStatusPointer->missionmodule) {
                return(PRC_Request_Pathfind);
        }

        /* get the target module... */
        
        targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,predatorStatusPointer->missionmodule,7,0);

        /* If there is no target module, we're way out there.  Better wander a bit more. */
        if(!targetModule)
        {
                targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,NULL,0);
        }
        /* Examine target, and decide what to do */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));         
        ProcessFarPredatorTargetAIModule(sbPtr, targetModule);  
        /* reset timer */
        predatorStatusPointer->stateTimer = PRED_FAR_MOVE_TIME;                                 
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Return(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        DYNAMICSBLOCK *dynPtr;
        AIMODULE *targetModule;
        VECTORCH velocityDirection = {0,0,0};
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer=(PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(predatorStatusPointer);
        LOCALASSERT(dynPtr);
        
        PredatorHandleMovingAnimation(sbPtr);

        /* should we change to approach state? */
        if(PredatorIsAwareOfTarget(sbPtr))
        {
                /* doesn't require a sequence change */
                return(PRC_Request_Engage);
        }

        predatorStatusPointer->stateTimer-=NormalFrameTime;

        /* Are we there yet? */
        if (sbPtr->containingModule->m_aimodule==predatorStatusPointer->missionmodule) {
                return(PRC_Request_Pathfind);
        }
        
        /* Target module aquisition. */

        LOCALASSERT(sbPtr->containingModule);

        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
        {
                NPC_InitMovementData(&(predatorStatusPointer->moveData));
        }
        if ((predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
                ||(predatorStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)) {

                targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,predatorStatusPointer->missionmodule,7,0);
        
                if (targetModule) {
                        FARENTRYPOINT *thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
                        if(thisEp) {
                                /* aha. an ep!... */ 
                                VECTORCH thisEpWorld = thisEp->position;

                                thisEpWorld.vx += targetModule->m_world.vx;
                                thisEpWorld.vy += targetModule->m_world.vy;
                                thisEpWorld.vz += targetModule->m_world.vz;                     
                                
                                predatorStatusPointer->wanderData.currentModule = sbPtr->containingModule->m_aimodule->m_index;
                                predatorStatusPointer->wanderData.worldPosition = thisEpWorld;
                        
                        } else {
                                /* Failure case. */
                                predatorStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
                        }

                } else {
                        /* Another failure case. */
                        predatorStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
                }
        }
        
        /* if we still haven't got one, bimble about in this one for a bit. */
        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
        {
                NPC_InitMovementData(&(predatorStatusPointer->moveData));
                NPC_FindAIWanderTarget(sbPtr,&(predatorStatusPointer->wanderData),&(predatorStatusPointer->moveData),0);
        }

        if(predatorStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE) {
                /* STILL broken!  Okay, just... wander, then. */
                return(PRC_Request_Wander);
        }
                
        /* ok: should have a current target at this stage... */
        NPCGetMovementDirection(sbPtr, &velocityDirection, &(predatorStatusPointer->wanderData.worldPosition),&predatorStatusPointer->waypointManager);
        NPCSetVelocity(sbPtr, &velocityDirection, predatorStatusPointer->nearSpeed);    

        /* test here for impeding collisions, and not being able to reach target... */
        #if ALL_NEW_AVOIDANCE_PRED
        {
                if (New_NPC_IsObstructed(sbPtr,&predatorStatusPointer->avoidanceManager)) {
                        /* Go to all new avoidance. */
                        return(PRC_Request_Avoidance);
                }
        }
        #else
        {
                STRATEGYBLOCK *destructableObject = NULL;
                NPC_IsObstructed(sbPtr,&(predatorStatusPointer->moveData),&predatorStatusPointer->obstruction,&destructableObject);
                if((predatorStatusPointer->obstruction.environment)||(predatorStatusPointer->obstruction.otherCharacter))
                {
                        /* go to avoidance */
                        return(PRC_Request_Avoidance);
                }
                if(predatorStatusPointer->obstruction.destructableObject)
                {
                        LOCALASSERT(destructableObject);
                        CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
                }
        }

        if(NPC_CannotReachTarget(&(predatorStatusPointer->moveData), &(predatorStatusPointer->wanderData.worldPosition), &velocityDirection))
        {
                /* go to avoidance */
                /* no sequence change required */
                
                predatorStatusPointer->obstruction.environment=1;
                predatorStatusPointer->obstruction.destructableObject=0;
                predatorStatusPointer->obstruction.otherCharacter=0;
                predatorStatusPointer->obstruction.anySingleObstruction=0;

                return(PRC_Request_Avoidance);
        }
        #endif
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_Recover(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        NPC_DATA *NpcData;
        
        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (!sbPtr->SBdptr) {
                /* We're far... do the timer! */
                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);
        }

        /* Stay where you are and regain health. */

        NpcData=GetThisNpcData(I_NPC_Predator);

        if (sbPtr->SBDamageBlock.Health>0) {
                int health_increment;

                health_increment=DIV_FIXED((NpcData->StartingStats.Health*NormalFrameTime),PRED_REGEN_TIME);
                sbPtr->SBDamageBlock.Health+=health_increment;  
                
                if (sbPtr->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
                        sbPtr->SBDamageBlock.Health=(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT);
                }

                HModel_Regen(&predatorStatusPointer->HModelController,PRED_REGEN_TIME);
        }
        
        /* Are we fully healed? */
        if (sbPtr->SBDamageBlock.Health==(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
                return(PRC_Request_Swap);
        }

        if (PredatorCanSeeTarget(sbPtr)) {
                return(PRC_Request_Swap);
        }

        /* Sequence should be set? */
        
        return(PRC_No_Change);

}

static PRED_RETURN_CONDITION Execute_PNS_Taunting(STRATEGYBLOCK *sbPtr)
{

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        #if 0
        if (predatorStatusPointer->HModelController.keyframe_flags) {
			#if 0
      		Sound_Play(SID_PRED_NEWROAR,"d",&sbPtr->DynPtr->Position);
			#else
			DoPredatorTauntSound(sbPtr);
			#endif
        }
        #endif

        if (!sbPtr->SBdptr) {
                /* We're far... do the timer! */
                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);
        }

        if (HModelAnimation_IsFinished(&predatorStatusPointer->HModelController)) {
                return(PRC_Request_Engage);
        } else {
                return(PRC_No_Change);
        }

}

static PRED_RETURN_CONDITION Execute_PNS_DischargeSpeargun(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH orientationDirn,relPos,relPos2;
        int correctlyOrientated,range;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* Always turn to face... */
        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);

        /* Fix weapon target! */
        {
                predatorStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat21,
                        300);
                predatorStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat22,
                        300);
                predatorStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat23,
                        300);
        }
        
        /* orientate to firing point first */
        if (predatorStatusPointer->My_Elevation_Section) {
                /* Assume range large w.r.t. half shoulder width... */
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - predatorStatusPointer->My_Elevation_Section->World_Offset.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - predatorStatusPointer->My_Elevation_Section->World_Offset.vz;
        } else {
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
        }
        /* Target shift? */
        correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

        /* Decloaking? */
        if (predatorStatusPointer->CloakStatus==PCLOAK_On) {
                PredatorCloakOff(predatorStatusPointer);
        }

        /* If still tweening, pause. */
        if (predatorStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
                return(PRC_No_Change);
        }
        
        if (predatorStatusPointer->stateTimer==predatorStatusPointer->Selected_Weapon->FiringRate) {
                
                predatorStatusPointer->HModelController.Playing=0;

                /* Only terminate if you haven't fired yet... */
                if(!PredatorCanSeeTarget(sbPtr))
                {
                        #if 1
                        /* ... and remove the gunflash */
                        #endif
        
                        return(PRC_Request_Engage);
                }
        
                /* we are not correctly orientated to the target: this could happen because we have
                just entered this state, or the target has moved during firing*/
                if((!correctlyOrientated)||(predatorStatusPointer->CloakStatus!=PCLOAK_Off))
                {
                        #if 1
                        /* stop visual and audio cues: technically, we're not firing at this moment */
                        #endif
                        return(PRC_No_Change);
                }
                
                /* If you are correctly oriented, you can now fire! */

                predatorStatusPointer->HModelController.Playing=1;
                predatorStatusPointer->HModelController.sequence_timer=0;

                relPos.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
                relPos.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
                relPos.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
                          
                relPos2.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vx);
                relPos2.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vy);
                relPos2.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vz);
                
                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
        
                #if 1
                /* look after the gun flash */
                #endif
                
                /* look after the sound */
                Sound_Play(SID_PRED_LASER,"d",&(sbPtr->DynPtr->Position));
        
                /* Now fire a spear... */

                {
                        SECTION_DATA *muzzle;
                        VECTORCH shotVector;

                        muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");

                        shotVector.vx=muzzle->SecMat.mat31;
                        shotVector.vy=muzzle->SecMat.mat32;
                        shotVector.vz=muzzle->SecMat.mat33;
                        
                        FindPolygonInLineOfSight(&shotVector,&muzzle->World_Offset,0,sbPtr->SBdptr);

                        /* Now deal with LOS_ObjectHitPtr. */
                        if (LOS_ObjectHitPtr) {
                                if (LOS_HModel_Section) {
                                        if (LOS_ObjectHitPtr->ObStrategyBlock) {
                                                if (LOS_ObjectHitPtr->ObStrategyBlock->SBdptr) {
                                                        GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
                                                }
                                        }
                                }
                                HandleSpearImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AMMO_PRED_RIFLE,&shotVector, 1, LOS_HModel_Section);
                        }

                        predatorStatusPointer->volleySize++;
                }

                if (predatorStatusPointer->volleySize>0) { /* Was 3 for pistol. */
                        predatorStatusPointer->enableSwap=1;
                }
        }       

        predatorStatusPointer->stateTimer -= NormalFrameTime;

        /* You must have fired already. */

        if(predatorStatusPointer->stateTimer > 0)       {
                return(PRC_No_Change);
        }
        
        if(range < predatorStatusPointer->Selected_Weapon->MinRange)
        {
                /* renew firing, as we are still too close to approach */ 
                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                predatorStatusPointer->volleySize = 0;
                return(PRC_No_Change);
        }
        else
        {
                NPC_DATA *NpcData;
                /* we are far enough away, so return to approach? */

                NpcData=GetThisNpcData(I_NPC_Predator);
                
                if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                        &&(sbPtr->SBDamageBlock.Health>0)) {
                        /* 50% health? */
                        if (General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->Target->containingModule->m_aimodule,5)) {
                                return(PRC_Request_Withdraw);
                        } else {
                                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                return(PRC_No_Change);
                        }
                }

                if (predatorStatusPointer->volleySize>=predatorStatusPointer->Selected_Weapon->VolleySize) {
                        if (((FastRandom()&65535)>32767)&&(predatorStatusPointer->enableSwap)) {
                                /* Change weapon! */
                                return(PRC_Request_Swap);
                        } else {
                                return(PRC_Request_Engage);
                        }
                } else {
                        /* And another! */
                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                        return(PRC_No_Change);
                }
        }
        return(PRC_No_Change);
}

static PRED_RETURN_CONDITION Execute_PNS_AttackBrutallyWithPlasmaCaster(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH orientationDirn,relPos,relPos2;
        int correctlyOrientated,range,onTarget;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* No messing, no taunting, no decloaking. */

        if (predatorStatusPointer->internalState==0) {
                if (predatorStatusPointer->HModelController.Tweening==Controller_NoTweening) {
                        predatorStatusPointer->HModelController.Playing=0;
                }
        }

        /* Always turn to face... */
        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);
        
        /* orientate to firing point first */
        if (predatorStatusPointer->My_Elevation_Section) {
                /* Assume range large w.r.t. half shoulder width... */
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - predatorStatusPointer->My_Elevation_Section->World_Offset.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - predatorStatusPointer->My_Elevation_Section->World_Offset.vz;
        } else {
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
        }
        /* Target shift? */
        correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

        /* Cloaking? */
        if (predatorStatusPointer->CloakStatus==PCLOAK_Off) {
                /* Stay unfair. */
                PredatorCloakOn(predatorStatusPointer);
        }

        /* Project three dots? */
        onTarget=DoPredatorLaserTargeting(sbPtr);

        predatorStatusPointer->stateTimer -= NormalFrameTime;

        /* Only terminate if you haven't fired yet... */
        if(!PredatorCanSeeTarget(sbPtr))
        {
                #if 1
                /* ... and remove the gunflash */
                #endif
        
                return(PRC_Request_Engage);
        }

        if (predatorStatusPointer->internalState==1) {
                /* Using stateTimer, thanks! */
        } else if(predatorStatusPointer->stateTimer > 0) {
                return(PRC_No_Change);
        }

        /* State timed out - try to fire! */
        
        /* we are not correctly orientated to the target: this could happen because we have
        just entered this state, or the target has moved during firing*/
        if (!correctlyOrientated)
        {
                #if 1
                /* stop visual and audio cues: technically, we're not firing at this moment */
                #endif
                return(PRC_No_Change);
        }

        /* Ready to fire... */

        if (correctlyOrientated&&(predatorStatusPointer->internalState!=2)) {
                if (predatorStatusPointer->internalState!=1) {
                        predatorStatusPointer->internalState=1;
                        /* Disable pausing... */
                        predatorStatusPointer->stateTimer=0;
                }
        }

        range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

        if ((predatorStatusPointer->internalState==1)&&(predatorStatusPointer->stateTimer<=0)) {
                if (onTarget) {         

                        /* If you are correctly oriented, you can now fire! */
                        
                        predatorStatusPointer->enableTaunt=0;
                        /* No taunting here, we're busy. */
                
                        predatorStatusPointer->HModelController.Playing=1;
                        predatorStatusPointer->HModelController.Looped=0;
                        predatorStatusPointer->HModelController.sequence_timer=0;
                        predatorStatusPointer->internalState=2;
                
                        relPos.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
                        relPos.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
                        relPos.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
                                  
                        relPos2.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vx);
                        relPos2.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vy);
                        relPos2.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vz);
                        
                        #if 1
                        /* look after the gun flash */
                        #endif
                        
                        /* look after the sound */
                        Sound_Play(SID_PRED_LAUNCHER,"d",&(sbPtr->DynPtr->Position));
                
                        /* Now fire a bolt. */
                
                        {
                                SECTION_DATA *muzzle;
                
                                muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");
                                
                                Pred_Weapon_Damage=TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty];

                                Pred_Weapon_Damage.Impact               =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Impact         ,10000);
                                Pred_Weapon_Damage.Cutting              =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Cutting        ,10000);
                                Pred_Weapon_Damage.Penetrative  =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Penetrative,10000);
                                Pred_Weapon_Damage.Fire                 =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Fire           ,10000);
                                Pred_Weapon_Damage.Electrical   =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Electrical     ,10000);
                                Pred_Weapon_Damage.Acid                 =MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Acid           ,10000);

                                Pred_Weapon_Damage.BlowUpSections=1;
                                Pred_Weapon_Damage.Special=0;

                                InitialiseEnergyBoltBehaviourKernel(&muzzle->World_Offset, &muzzle->SecMat,0,&Pred_Weapon_Damage,65536);
                                predatorStatusPointer->volleySize++;
                        }
                        predatorStatusPointer->enableSwap=1;
                        return(SRC_No_Change);
                } else {

                        /* You think you're correctly orientated - but you're still not hitting. */
                        if (predatorStatusPointer->stateTimer<=0) {
                                /* Oh, just give up. */
                                predatorStatusPointer->HModelController.Playing=1;
                                predatorStatusPointer->HModelController.Looped=0;
                                predatorStatusPointer->HModelController.sequence_timer=0;
                                predatorStatusPointer->internalState=2;
                        }
                }
        }

        if (predatorStatusPointer->internalState==2) {

                /* After shot. */
                
                #if 0
                if (!(HModelAnimation_IsFinished(&predatorStatusPointer->HModelController))) {
                        /* Still playing. */
                        return(PRC_No_Change);
                }
                #endif

                if(range < predatorStatusPointer->Selected_Weapon->MinRange)
                {
                        /* renew firing, as we are still too close to approach */ 
                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                        predatorStatusPointer->volleySize = 0;
                        predatorStatusPointer->internalState=0;
                        return(PRC_No_Change);
                }
                else
                {
                        NPC_DATA *NpcData;
                        /* we are far enough away, so return to approach? */
        
                        NpcData=GetThisNpcData(I_NPC_Predator);
                        
                        if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                                &&(sbPtr->SBDamageBlock.Health>0)) {
                                /* 50% health? */
                                if (General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->Target->containingModule->m_aimodule,5)) {
                                        return(PRC_Request_Withdraw);
                                } else {
                                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                        predatorStatusPointer->internalState=0;
                                        return(PRC_No_Change);
                                }
                        }
                
                        if (predatorStatusPointer->volleySize>=predatorStatusPointer->Selected_Weapon->VolleySize) {
                                if (range<PRED_CLOSE_ATTACK_RANGE) {
                                        /* Change weapon?  No random chance... */
                                        return(PRC_Request_Swap);
                                } else {
                                        #if 0
                                        return(PRC_Request_Engage);
                                        #else
                                        /* Heck, if we can still see them... */
                                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;
                                        predatorStatusPointer->internalState=0;
                                        return(PRC_No_Change);
                                        #endif
                                }
                        } else {
                                /* And another! */
                                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;
                                predatorStatusPointer->internalState=0;
                                return(PRC_No_Change);
                        }
                }

        }       
        
        return(PRC_No_Change);
}

#if 0
static PRED_RETURN_CONDITION Predator_ThreatAnalysis(STRATEGYBLOCK *sbPtr) {

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             
        
        /* A bit of an experimental function. */
        if (predatorStatusPointer->Target==NULL) {
                /* There ain't no threat! */
                NPC_DATA *NpcData;

                NpcData=GetThisNpcData(I_NPC_Predator);

                if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                        &&(sbPtr->SBDamageBlock.Health>0)) {
                        /* 50% health? */
                        return(PRC_Request_Withdraw);
                } else {
                        return(PRC_Request_Wander);
                }
        }

        GLOBALASSERT(predatorStatusPointer->Target);
        /* Now, what manner of threat do we face? */

        /* Frankly, this needs discussing before I do anything major. */
        
        return(PRC_Request_Swap);

}

static PRED_RETURN_CONDITION Execute_PNS_NewDischargePistol(STRATEGYBLOCK *sbPtr)
{
        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
        VECTORCH orientationDirn,relPos,relPos2;
        int correctlyOrientated,range;

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
        LOCALASSERT(predatorStatusPointer);                             

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (predatorStatusPointer->Target==NULL) {
                /* Bomb out. */
                return(PRC_Request_Wander);
        }

        /* Always turn to face... */
        GLOBALASSERT(predatorStatusPointer->Target);
        NPCGetTargetPosition(&(predatorStatusPointer->weaponTarget),predatorStatusPointer->Target);

        /* Fix weapon target! */
        {
                predatorStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
                        200);
                predatorStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
                        200);
                predatorStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
                        200);
        }
        
        /* orientate to firing point first */
        if (predatorStatusPointer->My_Elevation_Section) {
                /* Assume range large w.r.t. half shoulder width... */
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - predatorStatusPointer->My_Elevation_Section->World_Offset.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - predatorStatusPointer->My_Elevation_Section->World_Offset.vz;
        } else {
                orientationDirn.vx = predatorStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
                orientationDirn.vy = 0;
                orientationDirn.vz = predatorStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
        }
        /* Target shift? */
        correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

        /* Decloaking? */
        if (predatorStatusPointer->CloakStatus==PCLOAK_On) {
                PredatorCloakOff(predatorStatusPointer);
        }

        /* If still tweening, pause. */
        if (predatorStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
                return(PRC_No_Change);
        }
        
        //if (predatorStatusPointer->stateTimer==predatorStatusPointer->Selected_Weapon->FiringRate) 
        {
                
                //predatorStatusPointer->HModelController.Playing=0;

                /* Only terminate if you haven't fired yet... */
                if(!PredatorCanSeeTarget(sbPtr))
                {
                        #if 1
                        /* ... and remove the gunflash */
                        #endif
        
                        return(PRC_Request_Engage);
                }
        
                /* we are not correctly orientated to the target: this could happen because we have
                just entered this state, or the target has moved during firing*/
                if((!correctlyOrientated)||(predatorStatusPointer->CloakStatus!=PCLOAK_Off))
                {
                        #if 1
                        /* stop visual and audio cues: technically, we're not firing at this moment */
                        #endif
                        return(PRC_No_Change);
                }
                
                /* If you are correctly oriented, you can now fire! */

                //predatorStatusPointer->HModelController.Playing=1;
                //predatorStatusPointer->HModelController.sequence_timer=0;

                relPos.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
                relPos.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
                relPos.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
                          
                relPos2.vx=(predatorStatusPointer->Target->DynPtr->Position.vx)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vx);
                relPos2.vy=(predatorStatusPointer->Target->DynPtr->Position.vy)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vy);
                relPos2.vz=(predatorStatusPointer->Target->DynPtr->Position.vz)-(predatorStatusPointer->Target->DynPtr->PrevPosition.vz);
                
                range=VectorDistance((&predatorStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
        
                #if 1
                /* look after the gun flash */
                #endif
                
                /* look after the sound */
                Sound_Play(SID_PRED_PISTOL,"d",&(sbPtr->DynPtr->Position));
        
                /* Now fire a volley... */

                {
                        SECTION_DATA *muzzle;
                        VECTORCH null_vec;

                        muzzle=GetThisSectionData(predatorStatusPointer->HModelController.section_data,"dum flash");
                        
                        null_vec.vx=0;
                        null_vec.vy=0;
                        null_vec.vz=0;

                        GLOBALASSERT(muzzle);

                        FirePredPistolFlechettes(&muzzle->World_Offset,&null_vec,&muzzle->SecMat,0,&predatorStatusPointer->internalState,TRUE);

                }

                //if (predatorStatusPointer->volleySize>3) {
                //      predatorStatusPointer->enableSwap=1;
                //}
        }       

        predatorStatusPointer->stateTimer -= NormalFrameTime;

                /* Alex: I changed this 28/01/99 - it makes the Predator a bit easier, but also
                a bit more interesting - it hopefully makes the end of Earthbound a bit more fun. */
            if(range < predatorStatusPointer->Selected_Weapon->MinRange)
                {
                        predatorStatusPointer->enableSwap=1;
                        return(PRC_Request_Swap);
                }

        /* You must have fired already. */

        if(predatorStatusPointer->stateTimer > 0)       {
                return(PRC_No_Change);
        }
        
        /* Enable swap after a full burst. */
//        predatorStatusPointer->enableSwap=1;

        #if 0
        {
                /* renew firing, as we are still too close to approach */ 
                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                predatorStatusPointer->volleySize = 0;
                return(PRC_No_Change);
        }
        else
        {
                NPC_DATA *NpcData;
                /* we are far enough away, so return to approach? */

                NpcData=GetThisNpcData(I_NPC_Predator);
                
                if ((sbPtr->SBDamageBlock.Health<(NpcData->StartingStats.Health<<(ONE_FIXED_SHIFT-1)))
                        &&(sbPtr->SBDamageBlock.Health>0)) {
                        /* 50% health? */
                        if (General_GetAIModuleForRetreat(sbPtr,predatorStatusPointer->Target->containingModule->m_aimodule,5)) {
                                return(PRC_Request_Withdraw);
                        } else {
                                predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                                return(PRC_No_Change);
                        }
                }

                if (predatorStatusPointer->volleySize>=predatorStatusPointer->Selected_Weapon->VolleySize) {
                        if (((FastRandom()&65535)>32767)&&(predatorStatusPointer->enableSwap)) {
                                /* Change weapon! */
                                return(PRC_Request_Swap);
                        } else {
                                return(PRC_Request_Engage);
                        }
                } else {
                        /* And another! */
                        predatorStatusPointer->stateTimer = predatorStatusPointer->Selected_Weapon->FiringRate;                                 
                        return(PRC_No_Change);
                }
        }
        #endif
        return(PRC_No_Change);
}
#endif

static PRED_RETURN_CONDITION Execute_PNS_SelfDestruct(STRATEGYBLOCK *sbPtr)
{

        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    

        LOCALASSERT(sbPtr);
        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(predatorStatusPointer);

        /* zero linear velocity in dynamics block */
        LOCALASSERT(sbPtr->DynPtr);
        sbPtr->DynPtr->LinVelocity.vx = 0;
        sbPtr->DynPtr->LinVelocity.vy = 0;
        sbPtr->DynPtr->LinVelocity.vz = 0;

        if (!sbPtr->SBdptr) {
                /* We're far... do the timer! */
                ProveHModel_Far(&predatorStatusPointer->HModelController,sbPtr);
        }

        if (predatorStatusPointer->HModelController.Sub_Sequence==PCrSS_Det_Prog) {
                if (HModelAnimation_IsFinished(&predatorStatusPointer->HModelController)) {
                        InitHModelTweening(&predatorStatusPointer->HModelController,(ONE_FIXED>>2),
                                HMSQT_PredatorCrouch,PCrSS_Det_Laugh,-1,1);
                }
        }

        /* And if we're playing Laugh, we'd better laugh... */

        if (predatorStatusPointer->internalState==0) {
                /* Not yet activated. */
                if (predatorStatusPointer->HModelController.keyframe_flags&1) {
                        predatorStatusPointer->stateTimer=PRED_SELF_DESTRUCT_TIMER;
                        predatorStatusPointer->internalState=1;
                }
        } else if (predatorStatusPointer->internalState==1) {
                predatorStatusPointer->stateTimer-=NormalFrameTime;
                if (predatorStatusPointer->stateTimer<=0) {
                        /* Kaboom. */
                        predatorStatusPointer->internalState=2;
                        /* And kill the pred, for good measure? */
                        #if 0
                        predatorStatusPointer->GibbFactor=ONE_FIXED;
                        CauseDamageToObject(sbPtr,&certainDeath,ONE_FIXED,NULL);
                        StartPredatorSelfDestructExplosion(sbPtr);
                        /* So as not to confuse the corpse. */
						#else
                        predatorStatusPointer->GibbFactor=ONE_FIXED;
						predatorStatusPointer->Explode=1;
						#endif
                }
        }
                
        /* There's no escape from this other than death! */
        return(PRC_No_Change);

}

void StartPredatorSelfDestructExplosion(STRATEGYBLOCK *sbPtr) {

        /* So it can be easily altered.  Also referenced by bh_corpse.c. */
	#if 0
        HandleEffectsOfExplosion
        (
                sbPtr,
                &(sbPtr->DynPtr->Position),
                TemplateAmmo[AMMO_SADAR_BLAST].MaxRange,
                &TemplateAmmo[AMMO_SADAR_BLAST].MaxDamage[AvP.Difficulty],
                TemplateAmmo[AMMO_SADAR_BLAST].ExplosionIsFlat
        );
        Sound_Play(SID_NICE_EXPLOSION,"d",&(sbPtr->DynPtr->Position));
	#else
	/* Copied from SelfDestructBehavFun. */
	int i;

	/* KJL 16:20:57 27/08/98 - let's do some pyrotechnics */
	Sound_Play(SID_NICE_EXPLOSION,"d",&(Player->ObWorld));
	MakeVolumetricExplosionAt(&Player->ObWorld,EXPLOSION_HUGE);
	Player->ObStrategyBlock->SBDamageBlock.IsOnFire=1;
	
	//blow up everone
	for (i=0; i<NumActiveStBlocks; i++)
	{
		STRATEGYBLOCK *SbPtr = ActiveStBlockList[i];
		if(SbPtr->I_SBtype==I_BehaviourAlien ||
		   SbPtr->I_SBtype==I_BehaviourQueenAlien ||
		   SbPtr->I_SBtype==I_BehaviourFaceHugger ||
		   SbPtr->I_SBtype==I_BehaviourPredator ||
		   SbPtr->I_SBtype==I_BehaviourXenoborg ||
		   SbPtr->I_SBtype==I_BehaviourMarine ||
		   SbPtr->I_SBtype==I_BehaviourSeal ||
		   SbPtr->I_SBtype==I_BehaviourPredatorAlien ||
		   SbPtr->I_SBtype==I_BehaviourAlien ||
		   SbPtr->I_SBtype==I_BehaviourMarinePlayer ||
		   SbPtr->I_SBtype==I_BehaviourPredatorPlayer || 
		   SbPtr->I_SBtype==I_BehaviourAlienPlayer) 
		{
	 		{
	 			//kill the creature/player
	 			VECTORCH direction={0,-ONE_FIXED,0};
	 			CauseDamageToObject(SbPtr,&certainDeath,ONE_FIXED,&direction);
	 		
			}
		}
	}
	#endif
}

void DoPredatorHitSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
 
	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if (predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		return;
	}

	/* This one is for PREDATOR DAMAGE SCREAM. */

	if (predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
		PlayPredatorSound(0,PSC_Scream_Hurt,pitch,
			&predatorStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void DoPredatorAcidSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
 
	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if (predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		return;
	}

	/* This one is for PREDATOR ACID DAMAGE SCREAM. */

	if (predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
		PlayPredatorSound(0,PSC_Acid,pitch,
			&predatorStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void DoPredatorDeathSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
 
	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if (predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		return;
	}

	/* This one is for PREDATOR DEATH SCREAM. */

	if (predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
		PlayPredatorSound(0,PSC_Scream_Dying,pitch,
			&predatorStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void DoPredatorRandomSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
 
	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if (predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		return;
	}

	/* This one is for PREDATOR GENERAL SCREAM. */

	if (predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
		PlayPredatorSound(0,PSC_Scream_General,pitch,
			&predatorStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void DoPredatorTauntSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
 
	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	if (predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		return;
	}

	/* This one is for PREDATOR TAUNT. */

	if (predatorStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
		PlayPredatorSound(0,PSC_Taunt,pitch,
			&predatorStatusPointer->soundHandle,&sbPtr->DynPtr->Position);
	}

}

void DoPredatorAISwipeSound(STRATEGYBLOCK *sbPtr) {

	DYNAMICSBLOCK *dynPtr;
	PREDATOR_STATUS_BLOCK *predatorStatusPointer;

	GLOBALASSERT(sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(predatorStatusPointer);
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(dynPtr);

	/* This one is for PREDATOR SWIPE SOUND. */

	PlayPredatorSound(0,PSC_Swipe,0,NULL,&sbPtr->DynPtr->Position);

}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct predator_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	signed int health;
	PRED_BHSTATE behaviourState;
	PRED_BHSTATE lastState;

	int stateTimer;
	int internalState;
	int patience;
	int enableSwap;
	int enableTaunt;
	VECTORCH weaponTarget;			/* target position for firing weapon at */
	int volleySize;					/* used for weapon control */
	NPC_OBSTRUCTIONREPORT obstruction;
	NPC_WANDERDATA wanderData;

	int IAmCrouched;
	int personalNumber;				/* for predator personalisation */
	int nearSpeed;
	int GibbFactor;

	int incidentFlag;
	int incidentTimer;

	PREDATOR_NPC_WEAPONS PrimaryWeapon;
	PREDATOR_NPC_WEAPONS SecondaryWeapon;
	PREDATOR_NPC_WEAPONS ChangeToWeapon;

	/* these are for cloaking... */
	PRED_CLOAKSTATE CloakStatus;
	int CloakingEffectiveness;
	int CloakTimer;

	/* And these for the laser dots. */
	THREE_LASER_DOT_DESC Pred_Laser_Sight;
	int Pred_Laser_On	:1;
	int Explode			:1;

	int path;
	int stepnumber;


//annoying pointer related things

	int weapon_id;
	int currentAttackCode;
	char Target_SBname[SB_NAME_LENGTH];

	int missionmodule_index;
	int fearmodule_index;

//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;

}PREDATOR_SAVE_BLOCK;


//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV predatorStatusPointer

void LoadStrategy_Predator(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PREDATOR_STATUS_BLOCK* predatorStatusPointer;
	PREDATOR_SAVE_BLOCK* block = (PREDATOR_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	/*
	Check the strategy type.
	Slight complication here , since it might be a dormant predator
	(assuming that they are actually used in any of the levels)
	*/

	if(sbPtr->I_SBtype == I_BehaviourDormantPredator)
	{
		//best make it un-dormant then
		ActivateDormantPredator(sbPtr);	
	}
	else if(sbPtr->I_SBtype != I_BehaviourPredator)
	{
		return;
	}

	predatorStatusPointer = (PREDATOR_STATUS_BLOCK*) sbPtr->SBdataptr;
	

	//start copying stuff
	
	COPYELEMENT_LOAD(health)
	COPYELEMENT_LOAD(behaviourState)
	COPYELEMENT_LOAD(lastState)
	COPYELEMENT_LOAD(stateTimer)
	COPYELEMENT_LOAD(internalState)
	COPYELEMENT_LOAD(patience)
	COPYELEMENT_LOAD(enableSwap)
	COPYELEMENT_LOAD(enableTaunt)
	COPYELEMENT_LOAD(weaponTarget)			/* target position for firing weapon at */
	COPYELEMENT_LOAD(volleySize)					/* used for weapon control */
	COPYELEMENT_LOAD(obstruction)
	COPYELEMENT_LOAD(wanderData)
	COPYELEMENT_LOAD(IAmCrouched)
	COPYELEMENT_LOAD(personalNumber)				/* for predator personalisation */
	COPYELEMENT_LOAD(nearSpeed)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)
	COPYELEMENT_LOAD(PrimaryWeapon)
	COPYELEMENT_LOAD(SecondaryWeapon)
	COPYELEMENT_LOAD(ChangeToWeapon)
	COPYELEMENT_LOAD(CloakStatus)
	COPYELEMENT_LOAD(CloakingEffectiveness)
	COPYELEMENT_LOAD(CloakTimer)
	COPYELEMENT_LOAD(Pred_Laser_Sight)
	COPYELEMENT_LOAD(Pred_Laser_On)
	COPYELEMENT_LOAD(Explode)
	COPYELEMENT_LOAD(path)
	COPYELEMENT_LOAD(stepnumber)

	//load ai module pointers
	predatorStatusPointer->missionmodule = GetPointerFromAIModuleIndex(block->missionmodule_index);
	predatorStatusPointer->fearmodule = GetPointerFromAIModuleIndex(block->fearmodule_index);

	//load target
	COPY_NAME(predatorStatusPointer->Target_SBname,block->Target_SBname);
	predatorStatusPointer->Target = FindSBWithName(predatorStatusPointer->Target_SBname);

	//get the predator's attack from the attack code
	predatorStatusPointer->current_attack = GetThisAttack_FromUniqueCode(block->currentAttackCode);
	
	//get marine's weapon
	predatorStatusPointer->Selected_Weapon = GetThisNPCPredatorWeapon(block->weapon_id);

	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&predatorStatusPointer->HModelController);
		}
	}


    //sort out section pointers
    predatorStatusPointer->My_Gun_Section=GetThisSectionData(predatorStatusPointer->HModelController.section_data,predatorStatusPointer->Selected_Weapon->GunName);
    predatorStatusPointer->My_Elevation_Section=GetThisSectionData(predatorStatusPointer->HModelController.section_data,predatorStatusPointer->Selected_Weapon->ElevationName);


	Load_SoundState(&predatorStatusPointer->soundHandle);

}

void SaveStrategy_Predator(STRATEGYBLOCK* sbPtr)
{
	PREDATOR_SAVE_BLOCK *block;
	PREDATOR_STATUS_BLOCK* predatorStatusPointer;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	predatorStatusPointer = (PREDATOR_STATUS_BLOCK*) sbPtr->SBdataptr;


	//start copying stuff
	
	COPYELEMENT_SAVE(health)
	COPYELEMENT_SAVE(behaviourState)
	COPYELEMENT_SAVE(lastState)
	COPYELEMENT_SAVE(stateTimer)
	COPYELEMENT_SAVE(internalState)
	COPYELEMENT_SAVE(patience)
	COPYELEMENT_SAVE(enableSwap)
	COPYELEMENT_SAVE(enableTaunt)
	COPYELEMENT_SAVE(weaponTarget)			/* target position for firing weapon at */
	COPYELEMENT_SAVE(volleySize)					/* used for weapon control */
	COPYELEMENT_SAVE(obstruction)
	COPYELEMENT_SAVE(wanderData)
	COPYELEMENT_SAVE(IAmCrouched)
	COPYELEMENT_SAVE(personalNumber)				/* for predator personalisation */
	COPYELEMENT_SAVE(nearSpeed)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)
	COPYELEMENT_SAVE(PrimaryWeapon)
	COPYELEMENT_SAVE(SecondaryWeapon)
	COPYELEMENT_SAVE(ChangeToWeapon)
	COPYELEMENT_SAVE(CloakStatus)
	COPYELEMENT_SAVE(CloakingEffectiveness)
	COPYELEMENT_SAVE(CloakTimer)
	COPYELEMENT_SAVE(Pred_Laser_Sight)
	COPYELEMENT_SAVE(Pred_Laser_On)
	COPYELEMENT_SAVE(Explode)
	COPYELEMENT_SAVE(path)
	COPYELEMENT_SAVE(stepnumber)

	//save ai module pointers
	block->missionmodule_index = GetIndexFromAIModulePointer(predatorStatusPointer->missionmodule);
	block->fearmodule_index = GetIndexFromAIModulePointer(predatorStatusPointer->fearmodule);

	//save target
	COPY_NAME(block->Target_SBname,predatorStatusPointer->Target_SBname);

	//save attack code
	if(predatorStatusPointer->current_attack)
	{
		block->currentAttackCode = predatorStatusPointer->current_attack->Unique_Code;
	}
	else
	{
		block->currentAttackCode = -1;
	}
	
	//save predator's weapon
	if(predatorStatusPointer->Selected_Weapon) 
		block->weapon_id = predatorStatusPointer->Selected_Weapon->id;
	else 
		block->weapon_id = -1;
	
	//copy strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&predatorStatusPointer->HModelController);

	Save_SoundState(&predatorStatusPointer->soundHandle);
}
