/* CDF 11/6/98 - AI support functions moved out of bh_pred. */

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
#include "bh_agun.h"
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

#define UseLocalAssert Yes
#include "ourasert.h"

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern SECTION_DATA* LOS_HModel_Section;        /* Section of HModel hit */
extern void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *section_pointer); 
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern int GlobalFrameCounter;
extern int RouteFinder_CallsThisFrame;
extern DEATH_DATA Alien_Deaths[];
extern DEATH_DATA Marine_Deaths[];
extern DEATH_DATA Predator_Deaths[];
extern DEATH_DATA Xenoborg_Deaths[];
extern ATTACK_DATA Alien_Attacks[];
extern ATTACK_DATA Wristblade_Attacks[];
extern ATTACK_DATA PredStaff_Attacks[];
extern int SBIsEnvironment(STRATEGYBLOCK *sbPtr);

/* From HModel.c! */
extern void QNormalise(QUAT *q);
extern int IsAIModuleVisibleFromAIModule(AIMODULE *source,AIMODULE *target);

int IsMyPolyRidiculous(void);
int New_GetAvoidanceDirection(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager, VECTORCH *aggregateNormal);
int New_GetSecondAvoidanceDirection(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager, VECTORCH *aggregateNormal);
void InitialiseThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager);
void ClearThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager);
int ExecuteThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager);
int SimpleEdgeDetectionTest(STRATEGYBLOCK *sbPtr, COLLISIONREPORT *vcr);

void AlignVelocityToGravity(STRATEGYBLOCK *sbPtr,VECTORCH *velocity);

int PathArraySize;
PATHHEADER* PathArray;
/* Patrick 4/7/97 --------------------------------------------------

  BEHAVIOUR SUPPORT FUNCTIONS THAT MAY BE USED BY ANY NPC

---------------------------------------------------------------------*/

int CheckAdjacencyValidity(AIMODULE *target,AIMODULE *source,int alien) {

        FARENTRYPOINT *thisEp = GetAIModuleEP(target, source);
        if(thisEp) {
                if ((!alien)&&(thisEp->alien_only)) {
                        return(0);
                } else {
                        return(1);
                }
        }
        return(0);
}

/* Patrick-------------------------------------------------------------
For chris, or anyone else who wants it...
-----------------------------------------------------------------------*/
int NPC_IsDead(STRATEGYBLOCK *sbPtr)
{       
        LOCALASSERT(sbPtr);

        if(sbPtr->SBflags.please_destroy_me==1) return 1;

        switch(sbPtr->I_SBtype)
        {
                case I_BehaviourAlienPlayer:
                case I_BehaviourPredatorPlayer:
                case I_BehaviourMarinePlayer:
                {
                        /* For now. */
                        if (PlayerStatusPtr->IsAlive) {
                                return(0);
                        } else {
                                return(1);
                        }
                        break;
                }
                case(I_BehaviourPredator):
                {
                        PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
                        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);
                        LOCALASSERT(predatorStatusPointer);
                        if(predatorStatusPointer->behaviourState==PBS_Dying) return 1;
                        break;
                }
                case(I_BehaviourMarine):
                case(I_BehaviourSeal):
                {
                        MARINE_STATUS_BLOCK *marineStatusPointer;    
                        marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);
                        LOCALASSERT(marineStatusPointer);
                        if(marineStatusPointer->behaviourState==MBS_Dying) return 1;
                        break;
                }
                case(I_BehaviourAlien):
                {
                        ALIEN_STATUS_BLOCK *alienStatusPointer;
                        alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);
                        LOCALASSERT(alienStatusPointer);
                        if(alienStatusPointer->BehaviourState==ABS_Dying) return 1;
                        break;
                }
                case(I_BehaviourPredatorAlien):
                case(I_BehaviourQueenAlien):
                {
                        PAQ_STATUS_BLOCK *paqStatusPointer;    
                        paqStatusPointer = (PAQ_STATUS_BLOCK *)(sbPtr->SBdataptr);
                        LOCALASSERT(paqStatusPointer);
                        if(paqStatusPointer->NearBehaviourState==PAQNS_Dying) return 1;
                        break;
                }
                case(I_BehaviourNetCorpse):
                        /* Corpses are always dead :-) */
                        return(1);
                        break;
				case I_BehaviourAutoGun:
					{
						AUTOGUN_STATUS_BLOCK *agunStatusPointer;

						LOCALASSERT(sbPtr);
						agunStatusPointer = (AUTOGUN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
						LOCALASSERT(agunStatusPointer);

						if (agunStatusPointer->behaviourState==I_disabled) {
							return(1);
						} else {
							return(0);
						}
					}
					break;
                default:
                {
                        break;
                }
        }

        return 0;
}


/* Patrick-------------------------------------------------------------
This set of 3 functions is used by all npcs in checking for obstructive 
collisions, or targets we cannot reach
-----------------------------------------------------------------------*/
void NPC_InitMovementData(NPC_MOVEMENTDATA *moveData)
{
        LOCALASSERT(moveData);

        moveData->numObstructiveCollisions = 0;

        moveData->avoidanceDirn.vx = 0;
        moveData->avoidanceDirn.vy = 0;
        moveData->avoidanceDirn.vz = 0;

        moveData->lastTarget.vx = 0;
        moveData->lastTarget.vy = 0;
        moveData->lastTarget.vz = 0;

        moveData->lastVelocity.vx = 0;
        moveData->lastVelocity.vy = 0;
        moveData->lastVelocity.vz = 0;

        moveData->numReverses = 0;

        moveData->lastModule=NULL;
}

void NPC_IsObstructed(STRATEGYBLOCK *sbPtr, NPC_MOVEMENTDATA *moveData, NPC_OBSTRUCTIONREPORT *details, STRATEGYBLOCK **destructableObject)
{
        DYNAMICSBLOCK *dynPtr;
        struct collisionreport *nextReport;
        VECTORCH velDirn;
        AVP_BEHAVIOUR_TYPE myType;

        LOCALASSERT(destructableObject);
        LOCALASSERT(details);
        LOCALASSERT(moveData);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);
        nextReport = dynPtr->CollisionReportPtr;
        
        /* init destructable object pointer, etc... */
        *destructableObject = NULL;
        details->environment = 0;
        details->destructableObject = 0;
        details->otherCharacter = 0;
        details->anySingleObstruction = 0;

        /* check our velocity: if we haven't got one, we can't be obstructed, so just return */
        if((sbPtr->DynPtr->LinVelocity.vx==0)&&(sbPtr->DynPtr->LinVelocity.vy==0)&&(sbPtr->DynPtr->LinVelocity.vz==0))
        {
                moveData->numObstructiveCollisions = 0;  
                return;
        }
        
        /* get my velocity and behaviour type */
        velDirn = dynPtr->LinVelocity;
        Normalise(&velDirn);
        myType = sbPtr->I_SBtype;

        /* walk the collision report list, looking for collisions that obstruct our movement...
        excluding objects of our own type and the player */
        while(nextReport)
        {               
                int IsCharacterOrPlayer = 0;
                /* int dotWithGravity; */
                int normalDotWithVelocity;
                
                if(nextReport->ObstacleSBPtr)
                {
                        if(nextReport->ObstacleSBPtr==Player->ObStrategyBlock) IsCharacterOrPlayer = 1;
                        if(nextReport->ObstacleSBPtr->I_SBtype==myType)
                        {
                                IsCharacterOrPlayer = 1;
                                details->otherCharacter = 1;
                                details->anySingleObstruction = 1;
                        } 
                }

                {
                        VECTORCH normVelocity = sbPtr->DynPtr->LinVelocity;
                        Normalise(&normVelocity);
                        normalDotWithVelocity = DotProduct(&(nextReport->ObstacleNormal),&(normVelocity));
                }

//              if((normalDotWithVelocity < -46341)&&(!IsCharacterOrPlayer))
                /* So aliens can break through windows, 19/5/98 CDF */
                if((!IsCharacterOrPlayer)&&((normalDotWithVelocity < -46341)||(nextReport->ObstacleSBPtr)))
                {
                        /* aha... got one....*/
                        moveData->numObstructiveCollisions++;
                        if(moveData->numObstructiveCollisions > NPC_IMPEDING_COL_THRESHOLD)
                        {
                                moveData->numObstructiveCollisions = 0;
                                details->anySingleObstruction = 1;
                                details->environment = 1;

                                if(nextReport->ObstacleSBPtr)
                                {
                                        if(nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourInanimateObject)
                                        {
                                                INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = nextReport->ObstacleSBPtr->SBdataptr;
                                                if((objectstatusptr)&&(objectstatusptr->Indestructable == 0))
                                                {
                                                        /* aha: an object which the npc can destroy... */
                                                        *destructableObject = nextReport->ObstacleSBPtr;
                                                        details->destructableObject = 1;
                                                        details->environment = 0;
                                                }
                                        }
                                }                                        
                        }
                        /* if we have an obstructive collision, then return at this point
                        to avoid resetting numObstructiveCollisions. NB we only want to
                        record one per frame anyway... */
                        return; 
                }
                nextReport = nextReport->NextCollisionReportPtr;
        }

        /* no obstructions this frame, then ... */
        moveData->numObstructiveCollisions = 0;  
}

#if 0
int NPCIsExperiencingObstructiveCollision(STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection)
{
        DYNAMICSBLOCK *dynPtr;
        struct collisionreport *nextReport;

        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);
        nextReport = dynPtr->CollisionReportPtr;
        
        /* check our velocity: if we haven't got one, we can't be obstructed, so just return */
        if((velocityDirection->vx==0)&&(velocityDirection->vy==0)&&(velocityDirection->vz==0)) return 0;
                        
        /* walk the collision report list, looking for collisions against inanimate objects */
        while(nextReport)
        {               
                if(nextReport->ObstacleNormal.vy > -46341) return 1;
                nextReport = nextReport->NextCollisionReportPtr;
        }

        /* no collision, then ... */
        return 0;
}
#endif

int NPC_CannotReachTarget(NPC_MOVEMENTDATA *moveData, VECTORCH* thisTarget, VECTORCH* thisVelocity)
{
        LOCALASSERT(moveData);
        LOCALASSERT(thisTarget);
        LOCALASSERT(thisVelocity);

        /* if movement data has zero velocity, update and return */
        if((moveData->lastVelocity.vx == 0)&&
           (moveData->lastVelocity.vy == 0)&&
           (moveData->lastVelocity.vz == 0))
        {
                moveData->lastVelocity = *thisVelocity;
                moveData->lastTarget = *thisTarget;
                moveData->numReverses = 0;
                return 0;
        }       

        /* if new velocity is zero, update and return */
        if((thisVelocity->vx == 0)&&
           (thisVelocity->vy == 0)&&
           (thisVelocity->vz == 0))
        {
                moveData->lastVelocity = *thisVelocity;
                moveData->lastTarget = *thisTarget;
                moveData->numReverses = 0;
                return 0;
        }       

        /* if move data target and this target are different, update and return*/
        if((thisTarget->vx!=moveData->lastTarget.vx)||
           (thisTarget->vy!=moveData->lastTarget.vy)||
           (thisTarget->vz!=moveData->lastTarget.vz))
        {
                moveData->lastVelocity = *thisVelocity;
                moveData->lastTarget = *thisTarget;
                moveData->numReverses = 0;
                return 0;
        }

        /* at this point we have a previous velocity, a new velocity, 
        and the previous target is the same as the current target... 
        so compare previous and new velocities... */
        if(DotProduct(&(moveData->lastVelocity),thisVelocity)<(-56000)) /* 30 degrees */
        {
                moveData->lastVelocity = *thisVelocity;
                moveData->lastTarget = *thisTarget;
                moveData->numReverses++;
                if(moveData->numReverses>1) 
                {
                        moveData->numReverses = 0;
                        #if 1
                        return 1;
                        #else
                        return 0;
                        #endif
                }
                else return 0;  
        }

        /* just update */
        moveData->lastVelocity = *thisVelocity;
        moveData->lastTarget = *thisTarget;
        moveData->numReverses = 0;
        return 0;
}

/*------------------------------Patrick 14/2/97-----------------------------------
  Returns direction of movement to avoid obstructive collision in current
  direction of movement...
  -------------------------------------------------------------------------------*/

/* in this new version, the direction is taken from the npc's current direction (so
that it will work in 3d, for aliens)... and is returned in velocityDirection */
void NPCGetAvoidanceDirection(STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection, NPC_OBSTRUCTIONREPORT *details)
{
        VECTORCH newDirection1;
        VECTORCH newDirection2;
        int dir1dist = 0;
        int dir2dist = 0;

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        LOCALASSERT(velocityDirection);

        /* init velcity direction */
        velocityDirection->vx = velocityDirection->vy = velocityDirection->vz = 0;
        
        /* just in case */
        if(!sbPtr->containingModule) return; 

        if((details->environment)||(details->destructableObject)||(details->otherCharacter)||(details->anySingleObstruction))
        {
                /* going for a 90 degree turn + back a bit */
                
                /* construct the direction(s)... 
                start with object's local x unit vector (from local coo-ord system in world space) */
                newDirection1.vx = sbPtr->DynPtr->OrientMat.mat11;
                newDirection1.vy = sbPtr->DynPtr->OrientMat.mat12;
                newDirection1.vz = sbPtr->DynPtr->OrientMat.mat13;
                newDirection2.vx = -newDirection1.vx;
                newDirection2.vy = -newDirection1.vy;
                newDirection2.vz = -newDirection1.vz;
                /* ...and add on 1/4 of the -z direction...*/
                newDirection1.vx -= (sbPtr->DynPtr->OrientMat.mat31/4);
                newDirection1.vy -= (sbPtr->DynPtr->OrientMat.mat32/4);
                newDirection1.vz -= (sbPtr->DynPtr->OrientMat.mat33/4);
                newDirection2.vx -= (sbPtr->DynPtr->OrientMat.mat31/4);
                newDirection2.vy -= (sbPtr->DynPtr->OrientMat.mat32/4);
                newDirection2.vz -= (sbPtr->DynPtr->OrientMat.mat33/4);
                Normalise(&newDirection1);
                Normalise(&newDirection2);
                
                /* test how far we could go in each direction... */
                {
                        VECTORCH startingPosition = sbPtr->DynPtr->Position;
                        VECTORCH testDirn = newDirection1;

                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                        if(!LOS_ObjectHitPtr) dir1dist = NPC_MAX_VIEWRANGE;             
                        else dir1dist = LOS_Lambda;

                        startingPosition = sbPtr->DynPtr->Position;
                        testDirn = newDirection2;
                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                        if(!LOS_ObjectHitPtr) dir2dist = NPC_MAX_VIEWRANGE;             
                        else dir2dist = LOS_Lambda;
                }

                if(dir1dist > dir2dist) *velocityDirection = newDirection1;
                else *velocityDirection = newDirection2;
        }
}

/* Project actual shot? */
void ProjectNPCShot(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *muzzlepos, MATRIXCH *muzzleorient, enum AMMO_ID AmmoID, int multiple) {

        VECTORCH shotVector;  /* direction of view-line */
        VECTORCH targetPos;
        int mag;

        GetTargetingPointOfObject_Far(target,&targetPos);

        shotVector.vx = targetPos.vx - muzzlepos->vx;
        shotVector.vy = targetPos.vy - muzzlepos->vy;
        shotVector.vz = targetPos.vz - muzzlepos->vz;

        mag=Approximate3dMagnitude(&shotVector);
        
        shotVector.vx = MUL_FIXED(muzzleorient->mat31,mag);
        shotVector.vy = MUL_FIXED(muzzleorient->mat32,mag);
        shotVector.vz = MUL_FIXED(muzzleorient->mat33,mag);

        
        mag>>=2; /* For now. */

        /* Random tweak. */
        shotVector.vx+=((FastRandom()%mag)-(mag>>1));
        shotVector.vy+=((FastRandom()%mag)-(mag>>1));
        shotVector.vz+=((FastRandom()%mag)-(mag>>1));
        /* Normalise. */
        Normalise(&shotVector);

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
                        VECTORCH alpha = *muzzlepos;
                        VECTORCH beta = shotVector;
                        GLOBALASSERT(objectPtr);

                        if ((objectPtr!=sbPtr->SBdptr)&&(objectPtr!=target->SBdptr)) {
                                /* Can't hit target or self. */
                                CheckForVectorIntersectionWith3dObject(objectPtr, &alpha, &beta,1);
                        }
                }
        }
        #else
        FindPolygonInLineOfSight_TwoIgnores(&shotVector,muzzlepos,0,sbPtr->SBdptr,target->SBdptr);
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
                HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AmmoID,&shotVector, multiple*ONE_FIXED, LOS_HModel_Section);
        }

}

void CastLOSProjectile(STRATEGYBLOCK *sbPtr, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int inaccurate) {
        
        VECTORCH shotVector;
        DISPLAYBLOCK *self;

        shotVector=*in_shotvector;

        if (sbPtr) {
                self=sbPtr->SBdptr;
        } else {
                self=NULL;
        }
        
        /* Normalise. */
        Normalise(&shotVector);

        if (inaccurate) {
                /* Random tweak. */
                shotVector.vx+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                shotVector.vy+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                shotVector.vz+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                /* Normalise. */
                Normalise(&shotVector);
        }

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
                        VECTORCH alpha = *muzzlepos;
                        VECTORCH beta = shotVector;
                        GLOBALASSERT(objectPtr);

                        if (objectPtr!=self) {
                                /* Can't hit self. */
                                CheckForVectorIntersectionWith3dObject(objectPtr, &alpha, &beta,1);
                        }
                }
        }
        #else
        FindPolygonInLineOfSight(&shotVector,muzzlepos,0,self);
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
                HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AmmoID,&shotVector, multiple*ONE_FIXED, LOS_HModel_Section);
        }

}

int VerifyHitShot(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int maxrange) {
        
        VECTORCH shotVector;
        DISPLAYBLOCK *self,*target_dptr;

        shotVector=*in_shotvector;

        if (sbPtr) {
                self=sbPtr->SBdptr;
        } else {
                self=NULL;
        }
        
        if (target) {
                target_dptr=target->SBdptr;
        } else {
                target_dptr=NULL;
        }

        /* Normalise. */
        Normalise(&shotVector);

        FindPolygonInLineOfSight(&shotVector,muzzlepos,0,self);

        /* Now deal with LOS_ObjectHitPtr. */
        if (LOS_Lambda>maxrange) {
                return(1);
        }

        if (LOS_ObjectHitPtr) {
                if (LOS_HModel_Section) {
                        if (LOS_ObjectHitPtr->ObStrategyBlock) {
                                if (LOS_ObjectHitPtr->ObStrategyBlock->SBdptr) {
                                        GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
                                }
                        }
                }
                
                if ( ((LOS_ObjectHitPtr==target_dptr)&&(target_dptr!=NULL))||(SBIsEnvironment(LOS_ObjectHitPtr->ObStrategyBlock))) {
                        return(1);
                } else {
                        HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AmmoID,&shotVector, multiple*ONE_FIXED, LOS_HModel_Section);
                        return(0);
                }
        }
        return(1);
}

/* this function returns a target point for firing a projectile at the player*/
void NPCGetTargetPosition(VECTORCH *targetPoint, STRATEGYBLOCK *target)
{
        GLOBALASSERT(target);
        GLOBALASSERT(targetPoint);

        if (target->SBdptr) 
        {
                GetTargetingPointOfObject_Far(target, targetPoint);
        }
        else
        {
                *targetPoint = target->DynPtr->Position;
        }
}

/*------------------------Patrick 31/1/97-----------------------------
  Returns 2d approach velocity and time for given NPC, target, and speed
  targetDirn must be a normalised vector direction.
  --------------------------------------------------------------------*/
int NPCSetVelocity(STRATEGYBLOCK *sbPtr, VECTORCH* targetDirn, int in_speed)
{
        int orientated,speed;

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        LOCALASSERT(targetDirn);
        
        /* set targetDirn.vy to 0 just in case: if NPCGetTargetDirn is used get the target
        direction, the y component should always be 0, but other target directions may be
        passed... */
        /* Okay, that was old.  But maybe we need to do that unless UseStandardGravity is unset. */
        
        /* Set up speed as local, so we can tamper with it. */
        speed=in_speed;

        if ((sbPtr->I_SBtype!=I_BehaviourMarine)&&(sbPtr->I_SBtype!=I_BehaviourPredator)
                &&(sbPtr->I_SBtype!=I_BehaviourXenoborg)) {

                if (sbPtr->DynPtr->UseStandardGravity) {
                        targetDirn->vy = 0;
        
                        /* first check for zero direction vector */
                        if((targetDirn->vx==0)&&(targetDirn->vz==0))
                        {
                                sbPtr->DynPtr->LinVelocity.vx = 0;              
                                sbPtr->DynPtr->LinVelocity.vy = 0;
                                sbPtr->DynPtr->LinVelocity.vz = 0;
                                return(1);
                        }       
                }
        
                orientated=NPCOrientateToVector(sbPtr, targetDirn,NPC_TURNRATE,NULL);   
                
                {
                        VECTORCH velocity;
                        VECTORCH yDirection;
                        int dotProduct;
        
                        yDirection.vx = sbPtr->DynPtr->OrientMat.mat21;         
                        yDirection.vy = sbPtr->DynPtr->OrientMat.mat22;         
                        yDirection.vz = sbPtr->DynPtr->OrientMat.mat23;
                        
                        dotProduct = DotProduct(&yDirection,targetDirn);
                        
                        velocity.vx = targetDirn->vx - MUL_FIXED(yDirection.vx,dotProduct);
                        velocity.vy = targetDirn->vy - MUL_FIXED(yDirection.vy,dotProduct);
                        velocity.vz = targetDirn->vz - MUL_FIXED(yDirection.vz,dotProduct);
        
                        if ( (velocity.vx==0) && (velocity.vy==0) && (velocity.vz==0) ) {
                                sbPtr->DynPtr->LinVelocity.vx = 0;              
                                sbPtr->DynPtr->LinVelocity.vy = 0;
                                sbPtr->DynPtr->LinVelocity.vz = 0;
                                return(orientated);                     
                        }
        
                        Normalise(&velocity);
        
                        sbPtr->DynPtr->LinVelocity.vx = MUL_FIXED(velocity.vx,speed);           
                        sbPtr->DynPtr->LinVelocity.vy = MUL_FIXED(velocity.vy,speed);           
                        sbPtr->DynPtr->LinVelocity.vz = MUL_FIXED(velocity.vz,speed);                   
                }
        
                return(orientated);                     
        } else {
                int accelerationThisFrame,deltaVMag,dotProduct;
                VECTORCH deltaV,targetV,yDirection,movementOffset;
                const MOVEMENT_DATA *movementData;

                /* Mode 2, for marines 'n' predators.  And xenoborgs. */
                
                /* I'll still believe in 'Speed'... but look up acceleration. */
                switch (sbPtr->I_SBtype) {
                        case I_BehaviourPredator:
                                /* May need to change this based on state at some point? */
                                {
                                        PREDATOR_STATUS_BLOCK *predatorStatusPointer;
                                        LOCALASSERT(sbPtr);
                                        predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
                                        LOCALASSERT(predatorStatusPointer);
        
                                        if ((predatorStatusPointer->behaviourState==PBS_Wandering)
                                                ||((predatorStatusPointer->behaviourState==PBS_Avoidance)&&(predatorStatusPointer->lastState==PBS_Wandering))) {
                                                movementData=GetThisMovementData(MDI_Casual_Predator);
                                                /* Fix reduced max speed. */
                                                speed=movementData->maxSpeed;
                                        } else {
                                                movementData=GetThisMovementData(MDI_Predator);
                                        }
                                        accelerationThisFrame=movementData->acceleration;
                                }
                                break;
                        case I_BehaviourMarine:
                                {
                                        MARINE_STATUS_BLOCK *marineStatusPointer;
                                        LOCALASSERT(sbPtr);
                                        marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
                                        LOCALASSERT(marineStatusPointer);

                                        accelerationThisFrame=marineStatusPointer->acceleration;
                                }
                                break;
                        case I_BehaviourXenoborg:
                                movementData=GetThisMovementData(MDI_Xenoborg);
                                accelerationThisFrame=movementData->acceleration;
                                break;
                        default:
                                GLOBALASSERT(0);
                                break;
                }
                /* I'll get that from outside in a minute, okay? */
                /* Assume you're ground-based. */
                targetDirn->vy = 0;

                /* first check for zero direction vector */
                if((targetDirn->vx==0)&&(targetDirn->vz==0))
                {       
                        /* For the moment, you can still stop on a dime. */
                        sbPtr->DynPtr->LinVelocity.vx = 0;              
                        sbPtr->DynPtr->LinVelocity.vy = 0;
                        sbPtr->DynPtr->LinVelocity.vz = 0;
                        return(1);
                }       

                accelerationThisFrame=MUL_FIXED(accelerationThisFrame,NormalFrameTime);
                /* u+at, and all that, wot? */

                yDirection.vx = sbPtr->DynPtr->OrientMat.mat21;         
                yDirection.vy = sbPtr->DynPtr->OrientMat.mat22;         
                yDirection.vz = sbPtr->DynPtr->OrientMat.mat23;
                        
                dotProduct = DotProduct(&yDirection,targetDirn);
                
                targetV.vx = targetDirn->vx - MUL_FIXED(yDirection.vx,dotProduct);
                targetV.vy = targetDirn->vy - MUL_FIXED(yDirection.vy,dotProduct);
                targetV.vz = targetDirn->vz - MUL_FIXED(yDirection.vz,dotProduct);

                if ( (targetV.vx==0) && (targetV.vy==0) && (targetV.vz==0) ) {
                        sbPtr->DynPtr->LinVelocity.vx = 0;              
                        sbPtr->DynPtr->LinVelocity.vy = 0;
                        sbPtr->DynPtr->LinVelocity.vz = 0;
                        return(1);
                }
        
                Normalise(&targetV);

                targetV.vx=MUL_FIXED(targetV.vx,speed);
                targetV.vy=MUL_FIXED(targetV.vy,speed);
                targetV.vz=MUL_FIXED(targetV.vz,speed);
                
                deltaV.vx=targetV.vx-sbPtr->DynPtr->LinVelocity.vx;
                deltaV.vy=targetV.vy-sbPtr->DynPtr->LinVelocity.vy;
                deltaV.vz=targetV.vz-sbPtr->DynPtr->LinVelocity.vz;
                /* Now, deltaV is what we need to match. */
                deltaVMag=Approximate3dMagnitude(&deltaV);
                if (deltaVMag<=accelerationThisFrame) {
                        int dotP,magV;
                        VECTORCH normVelocity;

                        sbPtr->DynPtr->LinVelocity=targetV;

                        movementOffset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
                        movementOffset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
                        movementOffset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;
                        
                        normVelocity=sbPtr->DynPtr->LinVelocity;
                        magV=Approximate3dMagnitude(&sbPtr->DynPtr->LinVelocity);

                        Normalise(&movementOffset);
                        Normalise(&normVelocity);
                        dotP=DotProduct(&normVelocity,&movementOffset);
                        
                        /* To get a reasonable value out... */
                        if (magV<(speed>>2)) {
                                orientated=NPCOrientateToVector(sbPtr, &targetV,NPC_TURNRATE,NULL);
                        } else if ((dotP>40000)||((movementOffset.vx==0)&&(movementOffset.vy==0)&&(movementOffset.vz==0))) {
                                orientated=NPCOrientateToVector(sbPtr, &normVelocity,NPC_TURNRATE,NULL);
                        } else {
                                orientated=NPCOrientateToVector(sbPtr, &movementOffset,NPC_TURNRATE,NULL);
                        }
                        return(orientated);
                }
                /* If we're here, we can't make the target velocity yet. */
                Normalise(&deltaV);
                deltaV.vx=MUL_FIXED(deltaV.vx,accelerationThisFrame);
                deltaV.vy=MUL_FIXED(deltaV.vy,accelerationThisFrame);
                deltaV.vz=MUL_FIXED(deltaV.vz,accelerationThisFrame);

                sbPtr->DynPtr->LinVelocity.vx+=deltaV.vx;
                sbPtr->DynPtr->LinVelocity.vy+=deltaV.vy;
                sbPtr->DynPtr->LinVelocity.vz+=deltaV.vz;

                {
                        int dotP,magV;
                        VECTORCH normVelocity;
                                
                        movementOffset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
                        movementOffset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
                        movementOffset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;

                        normVelocity=sbPtr->DynPtr->LinVelocity;
                        magV=Approximate3dMagnitude(&sbPtr->DynPtr->LinVelocity);

                        Normalise(&movementOffset);
                        Normalise(&normVelocity);
                        dotP=DotProduct(&normVelocity,&movementOffset);
                        
                        /* To get a reasonable value out... */
                        if (magV<(speed>>2)) {
                                orientated=NPCOrientateToVector(sbPtr, &targetV,NPC_TURNRATE,NULL);
                        } else if ((dotP>40000)||((movementOffset.vx==0)&&(movementOffset.vy==0)&&(movementOffset.vz==0))) {
                                orientated=NPCOrientateToVector(sbPtr, &normVelocity,NPC_TURNRATE,NULL);
                        } else {
                                orientated=NPCOrientateToVector(sbPtr, &movementOffset,NPC_TURNRATE,NULL);
                        }
                
                }
                return(orientated);
        }
}

/*------------------------Patrick 3/2/97---------------------------------
  Sets an NPC's orientation so that it's z axis aligns to a given vector
  (modified from kevin's alien align to velocity routine)
  -----------------------------------------------------------------------*/
int NPCOrientateToVector(STRATEGYBLOCK *sbPtr, VECTORCH *zAxisVector,int turnspeed, VECTORCH *offset)
{
        int maxTurnThisFrame;
        int turnThisFrame;
        VECTORCH localZAxisVector;
        int localZVecEulerY;
        MATRIXCH toLocal;
        int orientatedOk = 0;

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        LOCALASSERT(zAxisVector);

        localZAxisVector = *zAxisVector;

        /* zero vector: nothing to do */
        if((localZAxisVector.vx==0)&&(localZAxisVector.vy==0)&&(localZAxisVector.vz==0)) return 1;

        /* rotate world zAxisVector into local space */
        toLocal = sbPtr->DynPtr->OrientMat;

        TransposeMatrixCH(&toLocal);
        RotateVector(&localZAxisVector, &toLocal);      
        Normalise(&localZAxisVector);

//      maxTurnThisFrame = WideMulNarrowDiv(NormalFrameTime,4096,NPC_TURNRATE);
        maxTurnThisFrame = MUL_FIXED(NormalFrameTime,turnspeed);
        localZVecEulerY = ArcTan(localZAxisVector.vx, localZAxisVector.vz);
        LOCALASSERT((localZVecEulerY>=0)&&(localZVecEulerY<=4096));

        if(localZVecEulerY==0||localZVecEulerY==4096) 
        {               
                /* if euler-y is 0 we are already aligned: nothing to do */
                return 1; 
        }

        if(localZVecEulerY>2048)
        {
                if(localZVecEulerY>(4096-maxTurnThisFrame))
                {
                        turnThisFrame = localZVecEulerY;
                        orientatedOk = 1;
                }
                else
                {
                        turnThisFrame = (4096-maxTurnThisFrame);
                        orientatedOk = 0;
                }       
        }
        else
        {
                if(localZVecEulerY>maxTurnThisFrame)
                {
                        turnThisFrame = maxTurnThisFrame;
                        orientatedOk = 0;
                }
                else
                {
                        turnThisFrame = localZVecEulerY;
                        orientatedOk = 1;
                }
        }

        /* now convert into a matrix & multiply existing orientation by it ...  */
        {
                MATRIXCH mat;           
                int cos = GetCos(turnThisFrame);
                int sin = GetSin(turnThisFrame);
                mat.mat11 = cos;                 
                mat.mat12 = 0;
                mat.mat13 = -sin;
                mat.mat21 = 0;          
                mat.mat22 = 65536;              
                mat.mat23 = 0;          
                mat.mat31 = sin;                
                mat.mat32 = 0;          
                mat.mat33 = cos;                
                        
                // NOTE : It seems like OrientMat is not being set per frame which
                // leads to inaccuracy build-up from the matrix multiplies.

                if (offset) {
                        VECTORCH new_offset,delta_offset;
                        //MATRIXCH reverse;

                        /* Code to attempt spinning on one foot. */
                        
                        RotateAndCopyVector(offset,&new_offset,&mat);
                        
                        //reverse=mat;
                        //TransposeMatrixCH(&reverse);
                        //
                        //RotateAndCopyVector(offset,&new_offset,&reverse);

                        delta_offset.vx=(offset->vx-new_offset.vx);
                        delta_offset.vy=(offset->vy-new_offset.vy);
                        delta_offset.vz=(offset->vz-new_offset.vz);
                        /* delta_offset is in local space? */

                        //RotateVector(&delta_offset,&sbPtr->DynPtr->OrientMat);

                        /* Now change position.  Bear in mind that many calls overwrite this change. */

                        sbPtr->DynPtr->Displacement.vx=delta_offset.vx;
                        sbPtr->DynPtr->Displacement.vy=delta_offset.vy;
                        sbPtr->DynPtr->Displacement.vz=delta_offset.vz;
                        sbPtr->DynPtr->UseDisplacement=1;

                }               

                MatrixMultiply(&sbPtr->DynPtr->OrientMat,&mat,&sbPtr->DynPtr->OrientMat);
                MatrixToEuler(&sbPtr->DynPtr->OrientMat, &sbPtr->DynPtr->OrientEuler);
        }

        return orientatedOk;
}

/*------------------------Patrick 1/2/97-----------------------------
  Tries to find an ep in an adjacent module which can be used as a 
  movement target for NPC. 
  --------------------------------------------------------------------*/
int NPCFindTargetEP(STRATEGYBLOCK *sbPtr, VECTORCH *targetPosn, AIMODULE **targetModule, int alien)
{
        AIMODULE **AdjModuleRefPtr;
        FARENTRYPOINT *bestEp;
        int bestSmell = 0;
        int aTargetExists = 0;
        AIMODULE *bestModule = (AIMODULE *)0;

        LOCALASSERT(sbPtr);
        LOCALASSERT(targetPosn);
        LOCALASSERT(targetModule);
                                
        if(!(sbPtr->containingModule)) return 0; /* just in case */
        AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;
        
        /* check if there is a module adjacency list */ 
        if(!AdjModuleRefPtr) return 0;

        /* go through each adjacent module */                   
        while(*AdjModuleRefPtr != 0)
        {
                AIMODULE *nextAdjModule = *AdjModuleRefPtr;                             
                if (AIModuleIsVisible(nextAdjModule))
                {
                        /* it is adjacent & visible ... */
                        FARENTRYPOINT *thisEp = GetAIModuleEP(nextAdjModule, sbPtr->containingModule->m_aimodule);
                        if(thisEp)
                        {
                                if (!((!alien)&(thisEp->alien_only))) {
                                        /* ... and has an ep, so test it's pheromone level */
                                        if(PherPl_ReadBuf[(nextAdjModule->m_index)] > bestSmell)
                                        {
                                                bestSmell = PherPl_ReadBuf[(nextAdjModule->m_index)];
                                                bestEp = thisEp;
                                                bestModule = nextAdjModule;
                                                aTargetExists = 1;
                                        }
                                }
                        }
                }
                AdjModuleRefPtr++;
        }

        /* return the result, if there is one */
        if(aTargetExists)
        {
                *targetPosn = bestEp->position;
                *targetModule = bestModule;
                targetPosn->vx += bestModule->m_world.vx;
                targetPosn->vy += bestModule->m_world.vy;
                targetPosn->vz += bestModule->m_world.vz;                       
                LOCALASSERT(bestEp->donorIndex == sbPtr->containingModule->m_aimodule->m_index);                
        }
        return aTargetExists;
}


        
/*------------------------Patrick 11/2/97-----------------------------
  If we're not in the same module as the player, find an ep for a
  suitable target module, and return this.  If cannot find an ep,
  or if the player is in our module, return the player's position.
  --------------------------------------------------------------------*/
void NPCGetMovementTarget(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *targetPosition,int *targetIsAirduct,int alien)
{       
        LOCALASSERT(sbPtr);
        LOCALASSERT(targetPosition);
        LOCALASSERT(targetIsAirduct);
        LOCALASSERT(target);
        GLOBALASSERT(playerPherModule);

        if (target==Player->ObStrategyBlock) {

                if(sbPtr->containingModule->m_aimodule != playerPherModule->m_aimodule)
                {
                        int epFound;
                        VECTORCH epPosn;
                        AIMODULE *epModule;
                        epFound = NPCFindTargetEP(sbPtr, &epPosn, &epModule,alien);
                        if(epFound)
                        {
                                *targetPosition = epPosn;
                                if((*epModule->m_module_ptrs)->m_flags & MODULEFLAG_AIRDUCT) *targetIsAirduct = 1;
                                else *targetIsAirduct = 0;                      
                                return;
                        }
                }
                /* in same module as player, or can't find an entry point */
                //*targetPosition = Player->ObWorld;
                GetTargetingPointOfObject(Player, targetPosition);
                *targetIsAirduct = 0;
        
        } else {

                /* Improve this presently */
                //*targetPosition = target->DynPtr->Position;
                GetTargetingPointOfObject_Far(target, targetPosition);
                *targetIsAirduct = 0;

        }
}

/*------------------------------Patrick 24/3/97-----------------------------------
        Calculates best direction of movement for NPC, from our main target direction:
        1. If on same poly as player, move towards him/her;
        2. Otherwise, find best adjacent floor polygon to move towards our target.

        NB works in 2d...
  -------------------------------------------------------------------------------*/
static int FindMyFloorPoly(VECTORCH* currentPosition, MODULE* currentModule);
static int CheckMyFloorPoly(VECTORCH* currentPosition, MODULE* currentModule);
static int VectorIntersects2dZVector(VECTORCH *vecStart,VECTORCH *vecEnd, int zExtent);
extern int SetupPolygonAccessFromShapeIndex(int shapeIndex);
extern int SetupPointAccessFromShapeIndex(int shapeIndex);

/* These globals are filled out by FindMyFloorPoly() */
static VECTORCH GMD_myPolyPoints[4];
static int GMD_myPolyNumPoints;
/* these Globals are used by NPCGetMovementDirection() */
VECTORCH myPolyEdgePoints[4];
VECTORCH myPolyEdgeDirections[4];
VECTORCH myPolyEdgeNormals[4];
VECTORCH myPolyMidPoint;
int myPolyEdgeMoveDistances[4];
extern int ShowNearSquad;
extern int ShowSquadState;
extern int ShowPredoStats;

/* a quick prototype */
static void NPCFindCurveToEdgePoint(STRATEGYBLOCK *sbPtr, int index, VECTORCH *velocityDirection);

void NPCGetMovementDirection(STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection, VECTORCH *targetPosition, WAYPOINT_MANAGER *waypointManager)
{
        VECTORCH targetDirection;
        int i;
        int playerPoly = NPC_GMD_NOPOLY;
        int ourPolyThisFrame = NPC_GMD_NOPOLY;
                                
        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        LOCALASSERT(velocityDirection);
        LOCALASSERT(targetPosition);

        if (sbPtr->containingModule==NULL) {
                /* Oops. */
                targetDirection = *targetPosition;
                targetDirection.vx -= sbPtr->DynPtr->Position.vx;
                targetDirection.vz -= sbPtr->DynPtr->Position.vz;
                targetDirection.vy = 0;
                Normalise(&targetDirection);
                *velocityDirection = targetDirection;
                return;
        } else {
                if ((sbPtr->containingModule->m_aimodule->m_waypoints!=NULL)&&(waypointManager)) {
                        if (NPCGetWaypointDirection(sbPtr->containingModule->m_aimodule->m_waypoints,sbPtr,velocityDirection,targetPosition,waypointManager)) {
                                /* Success! */
                                return;
                        }
                }
        }

        /* First get the (2d) direction of the main target */
        targetDirection = *targetPosition;
        targetDirection.vx -= sbPtr->DynPtr->Position.vx;
        targetDirection.vz -= sbPtr->DynPtr->Position.vz;
        /* If we got here, we *should* not be a crawling alien... */
        if (AlienIsCrawling(sbPtr)) {
                targetDirection.vy -= sbPtr->DynPtr->Position.vy;
        } else {
                /* Non-planar adjacency warnings? */
                if ((ShowSquadState)||(ShowPredoStats)||(ShowNearSquad)) {
                        targetDirection.vy -= sbPtr->DynPtr->Position.vy;
                        Normalise(&targetDirection);
                        if (targetDirection.vy<-46000) {
                                PrintDebuggingText("Non-planar adjacency!\n");
                        }
                }
                                
                targetDirection.vy = 0;
        }
        #if 1
        Normalise(&targetDirection);
        #else
        AlignVelocityToGravity(sbPtr,&targetDirection);
        #endif

        /* first- a hack to cope with stairs and al those little polygons:
        if we're in stairs just return the target direction. This is okay aslong as we don't 
        have curved stairs*/
        if(sbPtr->containingModule->m_flags&MODULEFLAG_STAIRS)
        {
                *velocityDirection = targetDirection;
                return;
        }

        /* get the player's poly index, and ours: 
        in the unlikely case that the player or npc doesnt have a current containingg module, 
        just return the target direction */
        if(playerPherModule==NULL)
        {
                *velocityDirection = targetDirection;
                return;
        }
        playerPoly = FindMyFloorPoly(&(Player->ObWorld), playerPherModule);
        ourPolyThisFrame = FindMyFloorPoly(&(sbPtr->DynPtr->Position), sbPtr->containingModule);

        /* Check for not having a current poly: this seems to happen occasionally
        around module boundaries- npc just needs a jolt... */
        if(ourPolyThisFrame == NPC_GMD_NOPOLY)
        {
                *velocityDirection = targetDirection;
                return;
        }

        /* CDF 9/8/98 So we've got a poly, but is it stupid? */
        if (IsMyPolyRidiculous()) {
                *velocityDirection = targetDirection;
                return;
        }

        /* Now check for player on our poly 
        NB only do this is we are not the player: as this function is used in player demo */    

        if((sbPtr != Player->ObStrategyBlock)&&(sbPtr->containingModule==playerPherModule)&&(playerPoly != NPC_GMD_NOPOLY))
        {
                if(playerPoly == ourPolyThisFrame)
                {
                        /* cripes- we're on the same poly */
                        *velocityDirection = targetDirection;
                        return;
                }
        }

        /* test */
        if(sbPtr == Player->ObStrategyBlock)
        {
                textprint("player poly %d \n",ourPolyThisFrame);
        }
                                
        /* Now get all the data we need:
        1. World space coords of poly edge mid points
        2. Directions from npc to those points
        3. Midpoint of our polygon.
        3a Some error checking.
        4. Distances we could move from poly midpoint to edge midpoint before hitting something
        */ 
        for(i=0;i<GMD_myPolyNumPoints;i++)
        {
                int point1 = i; 
                int point2 = i+1;               
                if(point2 >= GMD_myPolyNumPoints) point2 = 0;
                {
                        /* find the edge out normal - NB this won't work for clockwise polygons (2d) */                 
                        VECTORCH upNormal = {0,-65536,0};
                        VECTORCH edgeVector;

                        edgeVector.vx = GMD_myPolyPoints[point2].vx - GMD_myPolyPoints[point1].vx;
                        edgeVector.vy = 0;
                        edgeVector.vz = GMD_myPolyPoints[point2].vz - GMD_myPolyPoints[point1].vz;

                        CrossProduct(&edgeVector,&upNormal,&myPolyEdgeNormals[i]);
                        Normalise(&myPolyEdgeNormals[i]);
                        LOCALASSERT(myPolyEdgeNormals[i].vy == 0);
                }

                /* 1 : Calculate edge midpoint (2d) */
                myPolyEdgePoints[i].vx = ((GMD_myPolyPoints[point1].vx + GMD_myPolyPoints[point2].vx)/2)+MUL_FIXED(myPolyEdgeNormals[i].vx,50);
                myPolyEdgePoints[i].vy = 0;     
                myPolyEdgePoints[i].vz = ((GMD_myPolyPoints[point1].vz + GMD_myPolyPoints[point2].vz)/2)+MUL_FIXED(myPolyEdgeNormals[i].vz,50);         
                /* Into world space*/
                myPolyEdgePoints[i].vx += sbPtr->containingModule->m_world.vx;
                myPolyEdgePoints[i].vz += sbPtr->containingModule->m_world.vz;
                
                /* 2 : Directions to those points */
                myPolyEdgeDirections[i].vx = myPolyEdgePoints[i].vx - sbPtr->DynPtr->Position.vx;
                myPolyEdgeDirections[i].vy = 0;
                myPolyEdgeDirections[i].vz = myPolyEdgePoints[i].vz - sbPtr->DynPtr->Position.vz;               
                Normalise(&myPolyEdgeDirections[i]);
        }

        /* 3 : Poly midpoint- actually just an approximation, but doesn't matter 
           as long as its inside the poly (in world space):
           NB
           Quads are done by finding the midpoint of a diagonal.
           Triangles bisect a side then take the midpoint of that and the third point, else
           they can end up with a midpoint on one of their sides which buggers things up.*/
        if(GMD_myPolyNumPoints==3)
        {
                VECTORCH bisect;
                bisect.vy = ((GMD_myPolyPoints[1].vy + GMD_myPolyPoints[2].vy)/2);
                bisect.vx = ((GMD_myPolyPoints[1].vx + GMD_myPolyPoints[2].vx)/2);
                bisect.vz = ((GMD_myPolyPoints[1].vz + GMD_myPolyPoints[2].vz)/2);
                myPolyMidPoint.vy = ((GMD_myPolyPoints[0].vy + bisect.vy)/2)+sbPtr->containingModule->m_world.vy;
                myPolyMidPoint.vx = ((GMD_myPolyPoints[0].vx + bisect.vx)/2)+sbPtr->containingModule->m_world.vx;
                myPolyMidPoint.vz = ((GMD_myPolyPoints[0].vz + bisect.vz)/2)+sbPtr->containingModule->m_world.vz;
        }
        else
        {
                myPolyMidPoint.vy = ((GMD_myPolyPoints[0].vy + GMD_myPolyPoints[2].vy)/2)+sbPtr->containingModule->m_world.vy;
                myPolyMidPoint.vx = ((GMD_myPolyPoints[0].vx + GMD_myPolyPoints[2].vx)/2)+sbPtr->containingModule->m_world.vx;
                myPolyMidPoint.vz = ((GMD_myPolyPoints[0].vz + GMD_myPolyPoints[2].vz)/2)+sbPtr->containingModule->m_world.vz;
        }

        /* Error trapping:
        1. if midpoint is not in our polygon, just move to target
        2. If main target is in our poly, just move to target
        3. If any edge points are in our poly, just move to the target also
        */
        {
                int edgePoint[2];
                int polyPoints[10];
                int j;

                for(j=0;j<GMD_myPolyNumPoints;j++)
                {
                        polyPoints[(j*2)] = GMD_myPolyPoints[j].vx;
                        polyPoints[(j*2)+1] = GMD_myPolyPoints[j].vz;
                }
                /* Edge points (in poly local space) */
                for(j=0;j<GMD_myPolyNumPoints;j++)
                {
                        edgePoint[0] = myPolyEdgePoints[j].vx - sbPtr->containingModule->m_world.vx;
                        edgePoint[1] = myPolyEdgePoints[j].vz - sbPtr->containingModule->m_world.vz;

                        if(PointInPolygon(&edgePoint[0],&polyPoints[0],GMD_myPolyNumPoints,2))
                        {
                                /* one of the edge points is inside our poly */
                                *velocityDirection = targetDirection;
                                return;
                        }
                }
                /* Mid point (in poly local space) */
                edgePoint[0] = myPolyMidPoint.vx - sbPtr->containingModule->m_world.vx;
                edgePoint[1] = myPolyMidPoint.vz - sbPtr->containingModule->m_world.vz;         
                if(!(PointInPolygon(&edgePoint[0],&polyPoints[0],GMD_myPolyNumPoints,2)))
                {
                        /* the midpoint is inside our poly */
                        *velocityDirection = targetDirection;
                        return;
                }

                /* Target point (in poly local space)*/
                edgePoint[0] = targetPosition->vx - sbPtr->containingModule->m_world.vx;
                edgePoint[1] = targetPosition->vz - sbPtr->containingModule->m_world.vz;                
                if(PointInPolygon(&edgePoint[0],&polyPoints[0],GMD_myPolyNumPoints,2))
                {
                        /* the main target point is inside our poly:- this shouldn't happen,
                        as we have already tested for the player earlier on */
                        *velocityDirection = targetDirection;
                        return;
                }
        }

        /* 4 : Finally, the distances that we can move from the poly midpoint beyond
        each edge midpoint before we hit something... 
        */
        {
                for(i=0;i<GMD_myPolyNumPoints;i++)
                {
                        VECTORCH centreToEdgeVector;
                        int centreToEdgeDistance;

                        centreToEdgeVector.vx = myPolyEdgePoints[i].vx - myPolyMidPoint.vx;
                        centreToEdgeVector.vz = myPolyEdgePoints[i].vz - myPolyMidPoint.vz;
                        centreToEdgeVector.vy = 0;
                        centreToEdgeDistance = Magnitude(&centreToEdgeVector);

                        /* how far we can move along the centre to edge direction */
                        {
                                VECTORCH startingPosition = myPolyMidPoint;
                                VECTORCH testDirn = centreToEdgeVector;

                                /* poly mid point is in 3d world space: we need to do this
                                test at the height of the npc*/
                                startingPosition.vy = myPolyMidPoint.vy - 1500;
                                /*startingPosition.vy = sbPtr->DynPtr->Position.vy;*/
                                /* Normalise the test direction */
                                Normalise(&testDirn);

                                LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                                LOS_Lambda = NPC_MAX_VIEWRANGE;
                                CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                                if(!LOS_ObjectHitPtr) myPolyEdgeMoveDistances[i] = NPC_MAX_VIEWRANGE;           
                                else myPolyEdgeMoveDistances[i] = LOS_Lambda;
                        }

                        /* how far we can move beyond the poly edge */
                        myPolyEdgeMoveDistances[i] -= centreToEdgeDistance;
                        
                        /* quick check to eliminate Saturn type dodgy triangle points:
                        by setting move distance to zero, we should never select this edge as our
                        target edge... */
                        {
                                int point1 = i; 
                                int point2 = i+1;               
                                if(point2 >= GMD_myPolyNumPoints) point2 = 0;           
                                if(     (GMD_myPolyPoints[point1].vx == GMD_myPolyPoints[point2].vx) && 
                                        (GMD_myPolyPoints[point1].vy == GMD_myPolyPoints[point2].vy) && 
                                        (GMD_myPolyPoints[point1].vz == GMD_myPolyPoints[point2].vz))
                                        {
                                                LOCALASSERT(1==0);
                                                myPolyEdgeMoveDistances[i] = 0;
                                        }                       
                        }
                }
        }

        /* Now the crucial bit:- try to find an edge in our polygon that we can move towards
           and which is intersected by the vector from the mid point to the main target. If
           we find one, this is our edge! */ 
        for(i=0;i<GMD_myPolyNumPoints;i++)
        {
                if(myPolyEdgeMoveDistances[i] > NPC_MIN_MOVEFROMPOLYDIST)
                {
                        int vecExtent;
                        int ePoint1, ePoint2;
                        VECTORCH endPoint1, endPoint2;
                        VECTORCH edgeVector;
                        MATRIXCH edgeMatrix;
                        
                        ePoint1 = i;
                        ePoint2 = i+1;
                        if(ePoint2>=GMD_myPolyNumPoints) ePoint2 = 0;
                        edgeVector.vx = GMD_myPolyPoints[ePoint2].vx - GMD_myPolyPoints[ePoint1].vx;
                        edgeVector.vy = 0;
                        edgeVector.vz = GMD_myPolyPoints[ePoint2].vz - GMD_myPolyPoints[ePoint1].vz;
                
                        Normalise(&edgeVector);
                        vecExtent = Magnitude(&edgeVector);
                
                        /* This IS the right way around */
                        edgeMatrix.mat11 = myPolyEdgeNormals[i].vx;
                        edgeMatrix.mat21 = 0;
                        edgeMatrix.mat31 = myPolyEdgeNormals[i].vz;
                        edgeMatrix.mat12 = 0;
                        edgeMatrix.mat22 = 65536;
                        edgeMatrix.mat32 = 0;
                        edgeMatrix.mat13 = edgeVector.vx;
                        edgeMatrix.mat23 = 0;
                        edgeMatrix.mat33 = edgeVector.vz;

                        /* set up the test vector */
                        endPoint1 = myPolyMidPoint;
                        endPoint1.vx = endPoint1.vx - sbPtr->containingModule->m_world.vx - GMD_myPolyPoints[ePoint1].vx;
                        endPoint1.vy = 0;
                        endPoint1.vz = endPoint1.vz - sbPtr->containingModule->m_world.vz - GMD_myPolyPoints[ePoint1].vz;
                        RotateVector(&endPoint1, &edgeMatrix);

                        endPoint2 = *targetPosition;
                        endPoint2.vx = endPoint2.vx - sbPtr->containingModule->m_world.vx - GMD_myPolyPoints[ePoint1].vx;
                        endPoint2.vy = 0;
                        endPoint2.vz = endPoint2.vz - sbPtr->containingModule->m_world.vz - GMD_myPolyPoints[ePoint1].vz;
                        RotateVector(&endPoint2, &edgeMatrix);
                        
                        if(VectorIntersects2dZVector(&endPoint1,&endPoint2,vecExtent))
                        {
                                /* that'll do nicely */
                                
                                /* test */
                                if(sbPtr == Player->ObStrategyBlock)
                                {
                                        textprint("intersection test\n");
                                }
                                
                                #if 0
                                *velocityDirection = myPolyEdgeDirections[i];
                                #else
                                NPCFindCurveToEdgePoint(sbPtr,i,velocityDirection);
                                #endif
                                return;
                        }
                }
        }       

        /* test */
        if(sbPtr == Player->ObStrategyBlock)
        {
                textprint("nearest edge test\n");
        }

        /* Didn't find an intersection edge, so just pick the nearest 
          edge point that we can traverse */
        {
                int directionFound = 0;
                VECTORCH bestDirection;
                int closestDistance = 1000000; /* something very big */

                for(i=0;i<GMD_myPolyNumPoints;i++)
                {
                        if(myPolyEdgeMoveDistances[i] > NPC_MIN_MOVEFROMPOLYDIST)
                        {
                                int myDist = (VectorDistance(&myPolyEdgePoints[i],targetPosition));
                                if(myDist < closestDistance)
                                {
                                        bestDirection = myPolyEdgeDirections[i];
                                        closestDistance = myDist;
                                        directionFound = 1;
                                }
                        }
                }
                /* return best direction, if we have one */
                if(directionFound)
                {
                        LOCALASSERT(bestDirection.vy == 0);
                        *velocityDirection = bestDirection;
                        return;
                }
        }
                        
        /* We have utterly failed to find a suitable direction */
        #if 0
                velocityDirection->vx = velocityDirection->vy = velocityDirection->vz = 0;      
        #else 
                *velocityDirection = targetDirection;
        #endif
}

/* Patrick 12/6/97: This function is an auxilary function to NPCGetMovementDirection(), and
adds a curve to the edge point direction, weighted towards the centre of the current poly */
static void NPCFindCurveToEdgePoint(STRATEGYBLOCK *sbPtr, int edgeIndex, VECTORCH *velocityDirection)
{
        VECTORCH curvedPath;
        VECTORCH dirnToCentre;
        int weighting;

        LOCALASSERT(sbPtr);
        LOCALASSERT(velocityDirection);

        /* default direction- just in case something goes wrong */
        *velocityDirection = myPolyEdgeDirections[edgeIndex];

        /* find dirn to centre of poly */
        dirnToCentre.vx = myPolyMidPoint.vx - sbPtr->DynPtr->Position.vx;
        dirnToCentre.vz = myPolyMidPoint.vz - sbPtr->DynPtr->Position.vz;
        dirnToCentre.vy = 0;
        if((dirnToCentre.vx==0)&&(dirnToCentre.vz==0)) return;
        Normalise(&dirnToCentre);
        
        /* calculated weighted vector to centre */
        weighting = DotProduct(&myPolyEdgeDirections[edgeIndex],&dirnToCentre);
        if(weighting==0) return;
        
        dirnToCentre.vx = WideMulNarrowDiv(dirnToCentre.vx,weighting,ONE_FIXED);
        dirnToCentre.vz = WideMulNarrowDiv(dirnToCentre.vz,weighting,ONE_FIXED);

        /* add them to find curved direction path */
        curvedPath.vx = myPolyEdgeDirections[edgeIndex].vx + dirnToCentre.vx;
        curvedPath.vz = myPolyEdgeDirections[edgeIndex].vz + dirnToCentre.vz;
        curvedPath.vy = 0;
        Normalise(&curvedPath);
        *velocityDirection = curvedPath;        
}


/*------------------------Patrick 24/3/97-----------------------------
  This function is used to determine an intersection between 2 line
  segments.  The passed segment end points have been transformed into
  a space where the second segment is aligned to the z axis, and extends
  from 0 to the passed extent parameter. This makes the intersection
  test easy...
  --------------------------------------------------------------------*/
static int VectorIntersects2dZVector(VECTORCH *vecStart,VECTORCH *vecEnd, int zExtent)
{
        int vecIntercept;
        LOCALASSERT(vecStart);
        LOCALASSERT(vecEnd);
        
        if((vecStart->vx < 0)&&(vecEnd->vx < 0)) return 0;
        if((vecStart->vx > 0)&&(vecEnd->vx > 0)) return 0;
        if((vecStart->vz < 0)&&(vecEnd->vz < 0)) return 0;
        if((vecStart->vz > zExtent)&&(vecEnd->vz > zExtent)) return 0;
        vecIntercept = vecStart->vz + 
                WideMulNarrowDiv((vecEnd->vz - vecStart->vz),vecStart->vx,(vecEnd->vx - vecStart->vx));
        if((vecIntercept > 0)&&(vecIntercept < zExtent)) return 1; /* (deliberately ignoring endpoints) */      
        return 0;
}

/*------------------------Patrick 11/2/97-----------------------------
  Tries to find a floor polygon for a given world space location in
  a given module
  --------------------------------------------------------------------*/
static int FindMyFloorPoly(VECTORCH* currentPosition, MODULE* currentModule)
{
        struct ColPolyTag polygonData;
        int positionPoints[2];
        VECTORCH localPosition;
        int numPolys;
        int     polyCounter;
        int polyFound = 0;
        int polyFoundIndex = 0;
        
        LOCALASSERT(currentPosition);
        LOCALASSERT(currentModule);

        /* first, get the local position */
        localPosition.vx = currentPosition->vx - currentModule->m_world.vx;
        localPosition.vy = currentPosition->vy - currentModule->m_world.vy;
        localPosition.vz = currentPosition->vz - currentModule->m_world.vz;
        LOCALASSERT(PointIsInModule(currentModule,&localPosition));
        
        /* set up position points for object*/
        positionPoints[0] = localPosition.vx;
        positionPoints[1] = localPosition.vz;

        numPolys = SetupPolygonAccessFromShapeIndex(currentModule->m_mapptr->MapShape);
        polyCounter = numPolys;

        /* loop through the item list, then ... */
        while((polyCounter > 0) && (!polyFound))
        {
                AccessNextPolygon();
                GetPolygonVertices(&polygonData);
                GetPolygonNormal(&polygonData);

                /* first of all, reject any that don't have an up normal */
                if(polygonData.PolyNormal.vy < 0)
                {
                        /* set up poly points for containment test */
                        int polyPoints[10];
                        int numPtsInPoly = polygonData.NumberOfVertices;
                        int i;

                        for(i=0;i<numPtsInPoly;i++)
                        {
                                polyPoints[(i*2)] = polygonData.PolyPoint[i].vx;
                                polyPoints[((i*2)+1)] = polygonData.PolyPoint[i].vz;
                        }

                        if (PointInPolygon(&positionPoints[0],&polyPoints[0],numPtsInPoly,2))
                        {       
                                polyFound = 1;
                                polyFoundIndex = numPolys - polyCounter;
                        }       
                }
                polyCounter--;          
        }
        
        /* Init some globals */         
        {
                int i;
                GMD_myPolyNumPoints = 0;
                for(i=0;i<4;i++)
                {
                        GMD_myPolyPoints[i].vx = GMD_myPolyPoints[i].vy = GMD_myPolyPoints[i].vz = -1;
                }
        }               
        /* if we haven't found a poly, return NPC_GMD_NOPOLY.  Otherwise, return the index and fill
        out some globals... */
        if(!polyFound) return NPC_GMD_NOPOLY;

        LOCALASSERT(polyFoundIndex >= 0);
        LOCALASSERT(polyFound < numPolys);

        GMD_myPolyNumPoints = polygonData.NumberOfVertices;     
        GMD_myPolyPoints[0]     = polygonData.PolyPoint[0];
        GMD_myPolyPoints[1]     = polygonData.PolyPoint[1];
        GMD_myPolyPoints[2]     = polygonData.PolyPoint[2];             
        
        if(GMD_myPolyNumPoints > 3) 
        {
                GMD_myPolyPoints[3] = polygonData.PolyPoint[3];
        }
        
        return(polyFoundIndex);
}


/*------------------------Patrick 28/1/99-----------------------------
  Tries to find a floor polygon for a given world space location in
  a given module. Early exit if the location isn't even in the module...
  --------------------------------------------------------------------*/
int CheckMyFloorPoly(VECTORCH* currentPosition, MODULE* currentModule)
{
        struct ColPolyTag polygonData;
        int positionPoints[2];
        VECTORCH localPosition;
        int numPolys;
        int     polyCounter;
        int polyFound = 0;
        int polyFoundIndex = 0;
        
        LOCALASSERT(currentPosition);
        LOCALASSERT(currentModule);

        /* first, get the local position */
        localPosition.vx = currentPosition->vx - currentModule->m_world.vx;
        localPosition.vy = currentPosition->vy - currentModule->m_world.vy;
        localPosition.vz = currentPosition->vz - currentModule->m_world.vz;
        if( !PointIsInModule(currentModule,&localPosition) )
        {
			// Whoops ! I'm looking in the wrong module. Better just forget it.
        	return NPC_GMD_NOPOLY;
		}
        
        /* set up position points for object*/
        positionPoints[0] = localPosition.vx;
        positionPoints[1] = localPosition.vz;

        numPolys = SetupPolygonAccessFromShapeIndex(currentModule->m_mapptr->MapShape);
        polyCounter = numPolys;

        /* loop through the item list, then ... */
        while((polyCounter > 0) && (!polyFound))
        {
                AccessNextPolygon();
                GetPolygonVertices(&polygonData);
                GetPolygonNormal(&polygonData);

                /* first of all, reject any that don't have an up normal */
                if(polygonData.PolyNormal.vy < 0)
                {
                        /* set up poly points for containment test */
                        int polyPoints[10];
                        int numPtsInPoly = polygonData.NumberOfVertices;
                        int i;

                        for(i=0;i<numPtsInPoly;i++)
                        {
                                polyPoints[(i*2)] = polygonData.PolyPoint[i].vx;
                                polyPoints[((i*2)+1)] = polygonData.PolyPoint[i].vz;
                        }

                        if (PointInPolygon(&positionPoints[0],&polyPoints[0],numPtsInPoly,2))
                        {       
                                polyFound = 1;
                                polyFoundIndex = numPolys - polyCounter;
                        }       
                }
                polyCounter--;          
        }
        
        /* Init some globals */         
        {
                int i;
                GMD_myPolyNumPoints = 0;
                for(i=0;i<4;i++)
                {
                        GMD_myPolyPoints[i].vx = GMD_myPolyPoints[i].vy = GMD_myPolyPoints[i].vz = -1;
                }
        }               
        /* if we haven't found a poly, return NPC_GMD_NOPOLY.  Otherwise, return the index and fill
        out some globals... */
        if(!polyFound) return NPC_GMD_NOPOLY;

        LOCALASSERT(polyFoundIndex >= 0);
        LOCALASSERT(polyFound < numPolys);

        GMD_myPolyNumPoints = polygonData.NumberOfVertices;     
        GMD_myPolyPoints[0]     = polygonData.PolyPoint[0];
        GMD_myPolyPoints[1]     = polygonData.PolyPoint[1];
        GMD_myPolyPoints[2]     = polygonData.PolyPoint[2];             
        
        if(GMD_myPolyNumPoints > 3) 
        {
                GMD_myPolyPoints[3] = polygonData.PolyPoint[3];
        }
        
        return(polyFoundIndex);
}


/* Patrick 23/8/97 -----------------------------------------------------
A couple of functions for wandering
-----------------------------------------------------------------------*/
void NPC_InitWanderData(NPC_WANDERDATA *wanderData)
{
        LOCALASSERT(wanderData);
        wanderData->currentModule = NPC_NOWANDERMODULE;
        wanderData->worldPosition.vx = wanderData->worldPosition.vy = wanderData->worldPosition.vz = 0;
}


/* Patrick: 26/8/97
Finding a suitable target module- look thro' all the visible non-airduct modules 
connected to the npc's current module. pick one, and use it's ep as a 
target.  
We take a random adjacent ep as our target, but reject the most 'backward' one
(compared to our last velocity, as recorded in npc_movedata) as our last choice
*/

void NPC_FindAIWanderTarget(STRATEGYBLOCK *sbPtr, NPC_WANDERDATA *wanderData, NPC_MOVEMENTDATA *moveData, int alien)
{
        AIMODULE* chosenModule = NULL;
        VECTORCH chosenEpWorld;
        int numFound = 0;

        AIMODULE* worstModule = NULL;
        VECTORCH worstEpWorld;
        int worstEpDot;

        AIMODULE **AdjModuleRefPtr;
        VECTORCH lastVelocityDirection;
        int gotLastVelocityDirection = 0;

        LOCALASSERT(sbPtr);
        LOCALASSERT(sbPtr->DynPtr);
        LOCALASSERT(wanderData);
        LOCALASSERT(moveData);

        /* init the wander data block now, and we only have to fill in the correct
        values if we get them... */
        NPC_InitWanderData(wanderData);

        /* do we have a current module? */
        if(!(sbPtr->containingModule->m_aimodule)) return; /* no containing module */
        
        AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;     
        /* check if there is a module adjacency list */ 
        if(!AdjModuleRefPtr) return;

        /* try to get our last velocity direction */
        if((moveData->lastVelocity.vx!=0)||(moveData->lastVelocity.vz!=0)||(moveData->lastVelocity.vy!=0))
        {
                lastVelocityDirection = moveData->lastVelocity;
                Normalise(&lastVelocityDirection);
                gotLastVelocityDirection = 1;
        }
        else gotLastVelocityDirection = 0;

        /* if we've got a previous velocity, go through each adjacent module, 
        and try to find the worst one */                        
        while(*AdjModuleRefPtr != 0)
        {
                AIMODULE *nextAdjModule = *AdjModuleRefPtr;                             
                if ((AIModuleIsVisible(nextAdjModule))&&
                   (((*(nextAdjModule->m_module_ptrs))->m_flags&MODULEFLAG_AIRDUCT)==0)
                   &&(nextAdjModule!=moveData->lastModule))
                {
                        /* it is adjacent & visible & not an airduct:
                        try to find the ep position from this module... */
                        FARENTRYPOINT *thisEp = GetAIModuleEP(nextAdjModule, sbPtr->containingModule->m_aimodule);
                        if(thisEp)
                        {
                                if (!((!alien)&&(thisEp->alien_only))) {
                                        /* aha. an ep!... */ 
                                        VECTORCH thisEpWorld = thisEp->position;
                                        
                                        thisEpWorld.vx += nextAdjModule->m_world.vx;
                                        thisEpWorld.vy += nextAdjModule->m_world.vy;
                                        thisEpWorld.vz += nextAdjModule->m_world.vz;                    
                                        
                                        if(gotLastVelocityDirection)
                                        {
                                                VECTORCH thisEpDirection;
                                                int thisEpDot;
                                        
                                                thisEpDirection = thisEpWorld;
                                                thisEpDirection.vx -= sbPtr->DynPtr->Position.vx;
                                                thisEpDirection.vy -= sbPtr->DynPtr->Position.vy;
                                                thisEpDirection.vz -= sbPtr->DynPtr->Position.vz;
                                                Normalise(&thisEpDirection);
                                                thisEpDot = DotProduct(&thisEpDirection,&lastVelocityDirection);
                                        
                                                if(!worstModule)
                                                {
                                                        worstModule = nextAdjModule;
                                                        worstEpWorld = thisEpWorld;
                                                        worstEpDot = thisEpDot;                                                 
                                                }
                                                else
                                                {
                                                        numFound++;
                                                        if(thisEpDot<worstEpDot)
                                                        {
                                                                /* worse than our worst, so meld current worst with current,
                                                                and set new worst */
                                                                if(FastRandom()%numFound==0)
                                                                {
                                                                        /* take this one */
                                                                        chosenModule = worstModule;
                                                                        chosenEpWorld = worstEpWorld;
                                                                }
                                                                worstModule = nextAdjModule;
                                                                worstEpWorld = thisEpWorld;
                                                                worstEpDot = thisEpDot;                                                 
                                                        }
                                                        else
                                                        {
                                                                /* better than our worst... */
                                                                if(FastRandom()%numFound==0)
                                                                {
                                                                        /* take this one */
                                                                        chosenModule = nextAdjModule;
                                                                        chosenEpWorld = thisEpWorld;
                                                                }
                                                        }                                       
                                                }                               
                                        }
                                        else
                                        {
                                                /* don't bother with worst... */
                                                numFound++;
                                                if(FastRandom()%numFound==0)
                                                {
                                                        /* take this one */
                                                        chosenModule = nextAdjModule;
                                                        chosenEpWorld = thisEpWorld;
                                                }
                                        }
                                }
                        }
                }
                AdjModuleRefPtr++;
        }

        if(chosenModule)
        {
                LOCALASSERT(numFound>=1);
                wanderData->currentModule = sbPtr->containingModule->m_aimodule->m_index;
                wanderData->worldPosition = chosenEpWorld;
        }
        else if(worstModule)
        {
                wanderData->currentModule = sbPtr->containingModule->m_aimodule->m_index;
                wanderData->worldPosition = worstEpWorld;
        }
}

#define NEARLINK_QUEUE_LENGTH 100

typedef struct nl_route_queue {
        int depth;
        AIMODULE *aimodule;
        AIMODULE *first_step;
} NL_ROUTE_QUEUE;

NL_ROUTE_QUEUE NearLink_Route_Queue[NEARLINK_QUEUE_LENGTH];

int NL_Queue_End,NL_Queue_Exec;

AIMODULE *GetNextModuleForLink(AIMODULE *source,AIMODULE *target,int max_depth,int alien) {

        return(GetNextModuleForLink_Core(source,target,max_depth,0,alien));

}

AIMODULE *GetNextModuleForLink_Core(AIMODULE *source,AIMODULE *target,int max_depth,int visibility_check,int alien) {
        
		AIMODULE **AdjModuleRefPtr;

        /* Recursively search AIModule tree, trying to connect source and target. *
         * Return NULL on failure.                                                                                                */

        if (source==target) {
                return(source);
        }

        /* Clear the start. */

        NearLink_Route_Queue[0].depth=0;
        NearLink_Route_Queue[0].aimodule=source;
        NearLink_Route_Queue[0].first_step=NULL;
        NearLink_Route_Queue[1].aimodule=NULL; /* To set a standard. */

        NL_Queue_End=1;
        NL_Queue_Exec=0;
        
        RouteFinder_CallsThisFrame++;

        while (NearLink_Route_Queue[NL_Queue_Exec].aimodule!=NULL) {
                
                AIMODULE *thisModule;

                thisModule=NearLink_Route_Queue[NL_Queue_Exec].aimodule;

                AdjModuleRefPtr = thisModule->m_link_ptrs;
        
                if(AdjModuleRefPtr)     /* check that there is a list of adjacent modules */
                {
                        while(*AdjModuleRefPtr != 0)
                        {
                                /* Probably want some validity test for the link. */
                                if ((AIModuleIsPhysical(*AdjModuleRefPtr))
                                        &&(AIModuleAdmitsPheromones(*AdjModuleRefPtr))
                                        &&(CheckAdjacencyValidity((*AdjModuleRefPtr),thisModule,alien))
                                        &&((visibility_check==0)||(IsAIModuleVisibleFromAIModule(source,
                                        (*AdjModuleRefPtr)))
                                        )) {
                                        /* Is this the target? */
                                        if ( (*AdjModuleRefPtr)==target) {
                                                /* Yes!!! */
                                                if (NearLink_Route_Queue[NL_Queue_Exec].first_step) {
                                                        return(NearLink_Route_Queue[NL_Queue_Exec].first_step);
                                                } else {
                                                        /* Must be the next one. */
                                                        return(target);
                                                }
                                        } else if (
                                                (NearLink_Route_Queue[NL_Queue_Exec].depth<max_depth)
                                                &&( /* Test for 'used this time round' */
                                                        ((*AdjModuleRefPtr)->RouteFinder_FrameStamp!=GlobalFrameCounter)
                                                        ||((*AdjModuleRefPtr)->RouteFinder_IterationNumber!=RouteFinder_CallsThisFrame)
                                                )) {
                                                /* Add to queue. */
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=(*AdjModuleRefPtr);
                                                NearLink_Route_Queue[NL_Queue_End].depth=NearLink_Route_Queue[NL_Queue_Exec].depth+1;
                                                /* Remember first step. */
                                                if (NearLink_Route_Queue[NL_Queue_Exec].first_step==NULL) {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=(*AdjModuleRefPtr);
                                                } else {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=NearLink_Route_Queue[NL_Queue_Exec].first_step;
                                                }
                                                /* Stamp as used. */
                                                (*AdjModuleRefPtr)->RouteFinder_FrameStamp=GlobalFrameCounter;
                                                (*AdjModuleRefPtr)->RouteFinder_IterationNumber=RouteFinder_CallsThisFrame;
                                                NL_Queue_End++;
                                                if (NL_Queue_End>=NEARLINK_QUEUE_LENGTH) {
                                                        NL_Queue_End=0;
                                                        textprint("Wrapping Nearlink Queue!\n");
                                                }
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=NULL;
                                                if (NL_Queue_End==NL_Queue_Exec) {
                                                        LOGDXFMT(("Oh, no.  NearLinkQueue screwed.  NL_Queue_End=%d, depth = %d\n",NL_Queue_End,NearLink_Route_Queue[NL_Queue_Exec].depth));
                                                        LOCALASSERT(NL_Queue_End!=NL_Queue_Exec); //if this happens the queue probably needs to be longer
                                                }
                                        }
                                }
                                /* next adjacent module reference pointer */
                                AdjModuleRefPtr++;
                        }
                }

                /* Done all the links. */

                NL_Queue_Exec++;
                if (NL_Queue_Exec>=NEARLINK_QUEUE_LENGTH) NL_Queue_Exec=0;

        }

        /* Still here?  Must have hit the end, then. */

        return(NULL);
}


int GetNextModuleInPath(int current_module, int path) {

        GLOBALASSERT(path>=0 && path<PathArraySize);
        GLOBALASSERT(PathArray);
        GLOBALASSERT(PathArray[path].path_length);
        
        return (current_module+1)%PathArray[path].path_length;


}

AIMODULE *TranslatePathIndex(int current_module, int path) {

        GLOBALASSERT(path>=0 && path<PathArraySize);
        GLOBALASSERT(PathArray);
        GLOBALASSERT(current_module<PathArray[path].path_length);

        return (PathArray[path].modules_in_path[current_module]);

}

int GetClosestStepInPath(int path,MODULE* current_module)
{
        int i;
        PATHHEADER* path_head;
        GLOBALASSERT(path>=0 && path<PathArraySize);
        GLOBALASSERT(PathArray);
        if(!current_module) return 0;

        path_head=&PathArray[path];
        GLOBALASSERT(path_head->path_length);

        //see if enemy is currently in any of the modules in the path
        {
                AIMODULE* current_aimodule=current_module->m_aimodule;
                for(i=0;i<path_head->path_length;i++)
                {
                        if(current_aimodule==path_head->modules_in_path[i])     
                        {
                                return i;
                        }
                }
        }

        //enemy not on path , so try to find the closest module on the path
        {
                int closest_distance=0x7fffffff;
                int closest_point=0;
                VECTORCH* current_pos=&current_module->m_world;
                VECTORCH diff;

                for(i=0;i<path_head->path_length;i++)
                {
                        int distance;   
                        diff=path_head->modules_in_path[i]->m_world;
                        SubVector(current_pos,&diff);
                        distance=Approximate3dMagnitude(&diff);

                        if(distance<closest_distance)
                        {
                                closest_distance=distance;
                                closest_point=i;
                        }

                }

                return closest_point;
        }
                
}

/* Death Shell */

int CheckDeathValidity(HMODELCONTROLLER *controller,SECTION *TemplateRoot,DEATH_DATA *ThisDeath,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching, int electrical) {

        /* Check against many things.  Harshly. */

        /* Crouching test. */
        if (crouching) {
                if (ThisDeath->Crouching==0) {
                        return(0);
                }
        } else {
                if (ThisDeath->Crouching!=0) {
                        return(0);
                }
        }

        /* Burning test. */
        if (burning) {
                if (ThisDeath->Burning==0) {
                        return(0);
                }
        } else {
                if (ThisDeath->Burning!=0) {
                        return(0);
                }
        }

        /* Electrical test. */
        if (electrical) {
                if (ThisDeath->Electrical==0) {
                        return(0);
                }
        } else {
                if (ThisDeath->Electrical!=0) {
                        return(0);
                }
        }

        /* Hurtiness. */
        switch(hurtiness) {
                case 0:
                default:
                        /* Pain case. */
                        if (ThisDeath->MinorBoom) {
                                return(0);
                        } 
                        if (ThisDeath->MajorBoom) {
                                return(0);
                        }
                        break;
                case 1:
                        /* Minor Boom. */
                        if (!ThisDeath->MinorBoom) {
                                return(0);
                        }
                        break;
                case 2:
                        /* Major Boom. */
                        if (!ThisDeath->MajorBoom) {
                                return(0);
                        }
        }

        /* Facing.  Complex one... */
        /* Input facing must contain at least all the flags in the death. */
        if (facing) {
                if (ThisDeath->Facing.Front) {
                        if (facing->Front==0) {
                                return(0);
                        }
                }
                if (ThisDeath->Facing.Back) {
                        if (facing->Back==0) {
                                return(0);
                        }
                }
                if (ThisDeath->Facing.Left) {
                        if (facing->Left==0) {
                                return(0);
                        }
                }
                if (ThisDeath->Facing.Right) {
                        if (facing->Right==0) {
                                return(0);
                        }
                }
        } else {
                if ( (ThisDeath->Facing.Front)||(ThisDeath->Facing.Back)||(ThisDeath->Facing.Left)||(ThisDeath->Facing.Right) ) {
                        return(0);
                }
        }
        /* Wound flags.  Also quite odd. */
        /* If wound_flags are specified in the death, the input must contain them. */
        if (ThisDeath->wound_flags) {
                if ((ThisDeath->wound_flags&wound_flags)!=ThisDeath->wound_flags) {
                        return(0);
                }
        }
        
        /* Priority wound flags.  As above, but backwards. */
        /* If the input has priority wounds, the death must contain them. */
        if (priority_wounds) {
                if ((ThisDeath->priority_wounds&priority_wounds)!=priority_wounds) {
                        return(0);
                }
        }

        /* Finally, sequence validity. */
        if (ThisDeath->Template) {
                if (!HModelSequence_Exists_FromRoot(TemplateRoot,ThisDeath->Sequence_Type,ThisDeath->Sub_Sequence)) {
                        return(0);
                }
        } else {
                if (!HModelSequence_Exists(controller,ThisDeath->Sequence_Type,ThisDeath->Sub_Sequence)) {
                        return(0);
                }
        }

        /* It got through! */
        return(1);
}

int CountValidDeaths(HMODELCONTROLLER *controller,SECTION *TemplateRoot,DEATH_DATA *FirstDeath,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching, int electrical) {

        DEATH_DATA *this_death;
        int number_of_candidates;

        number_of_candidates=0;
        this_death=FirstDeath;

        while (this_death->Sequence_Type>=0) {
                if (CheckDeathValidity(controller,TemplateRoot,this_death,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical)) {
                        number_of_candidates++;
                }
                this_death++;
        }

        return(number_of_candidates);
}

DEATH_DATA *GetThisDeath(HMODELCONTROLLER *controller,SECTION *TemplateRoot,DEATH_DATA *FirstDeath,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching, int electrical, int index) {

        /* Extract 'index' from the valid deaths. */
        DEATH_DATA *retval;
        DEATH_DATA *this_death;
        int number;

        retval=NULL;

        number=0;
        this_death=FirstDeath;

        while (this_death->Sequence_Type>=0) {
                if (CheckDeathValidity(controller,TemplateRoot,this_death,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical)) {
                        if (number==index) {
                                retval=this_death;
                                break;
                        } else {
                                number++;
                        }
                }
                this_death++;
        }

        GLOBALASSERT(retval);
        return(retval);
}

DEATH_DATA *GetThisDeath_FromCode(HMODELCONTROLLER *controller,DEATH_DATA *FirstDeath,int code) {

        /* Extract 'code' from the valid deaths. */
        DEATH_DATA *retval;
        DEATH_DATA *this_death;

        retval=NULL;

        this_death=FirstDeath;

        while (this_death->Sequence_Type>=0) {
                if (this_death->Multiplayer_Code==code) {
                        retval=this_death;
                        break;
                }
                this_death++;
        }

        GLOBALASSERT(retval);
        return(retval);
}

DEATH_DATA *GetThisDeath_FromUniqueCode(int code) {

	extern DEATH_DATA Alien_Deaths[];
	extern DEATH_DATA Marine_Deaths[];
	extern DEATH_DATA Predator_Special_SelfDestruct_Death;
	extern DEATH_DATA Predator_Deaths[];
	extern DEATH_DATA Xenoborg_Deaths[];
        
    DEATH_DATA* this_death = NULL;
        
    switch (code>>16)
	{
		case 0:
			this_death = &Alien_Deaths[0];
			break;
		case 1:
			this_death = &Marine_Deaths[0];
			break;
		case 2:
			return &Predator_Special_SelfDestruct_Death;

		case 3:
			this_death = &Predator_Deaths[0];
			break;
		case 4:
			this_death = &Xenoborg_Deaths[0];
			break;
		default:
			return 0;
	}
        

       while (this_death->Sequence_Type>=0) {
               if (this_death->Unique_Code==code) {
                       return this_death;
               }
               this_death++;
       }

       return 0;
}


DEATH_DATA *GetDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,DEATH_DATA *FirstDeath,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical) {

        int number_of_candidates;
        int index;
        
        int use_wound_flags;
        int use_priority_wounds;
        int use_hurtiness;
        HIT_FACING *use_facing;
        int use_burning;
        int use_crouching;
		int use_electrical;

        use_priority_wounds=priority_wounds;
        use_wound_flags=wound_flags;
        use_hurtiness=hurtiness;
        use_facing=facing;
        use_burning=burning;
        use_crouching=crouching;
		use_electrical=electrical;

        number_of_candidates=0;

        while (number_of_candidates==0) {
                /* Iterate, making simplifications, until there are valid deaths. */
                number_of_candidates=CountValidDeaths(controller,TemplateRoot,FirstDeath,use_wound_flags,use_priority_wounds,use_hurtiness,use_facing,use_burning,use_crouching,use_electrical);
                if (number_of_candidates==0) {
                        /* Right.  Make a change.  Priority wounds first. */
                        if (use_priority_wounds) {
                                use_priority_wounds=0;
                                continue;
                        }
                        /* Wound flags next. */
                        if (use_wound_flags!=0xffffffff) {
                                use_wound_flags=0xffffffff;
                                continue;
                        }
                        /* Now facing. */
                        if (use_facing) {
                                use_facing=NULL;
                                continue;
                        }
                        /* Now hurtiness. */
                        if (use_hurtiness) {
                                use_hurtiness--;
                                continue;
                        }
                        /* Now electrical. */
                        if (use_electrical) {
                                use_electrical=0;
                                continue;
                        }
                        /* Finally, burning. */
                        if (use_burning) {
                                use_burning=0;
                                continue;
                        }
                        /* Only crouch is left! */
                        //NewOnScreenMessage("DEATH SELECTION FAILURE!\n");
                        if (use_crouching) {
                                use_crouching=0;
                                continue;
                        }
                        //NewOnScreenMessage("I REALLY MEAN IT!\n");
                        /* Here goes nothing. */
                        return(FirstDeath);
                }
        }
        
        /* Right, by now we should have a number of candidates. */
        GLOBALASSERT(number_of_candidates);

        index=FastRandom()%number_of_candidates;

        return(GetThisDeath(controller,TemplateRoot,FirstDeath,use_wound_flags,use_priority_wounds,
                use_hurtiness,use_facing,use_burning,use_crouching,use_electrical,index));
}

DEATH_DATA *GetAlienDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching, int electrical) {

        return(GetDeathSequence(controller,TemplateRoot,Alien_Deaths,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical));

}

DEATH_DATA *GetMarineDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching, int electrical) {

        return(GetDeathSequence(controller,TemplateRoot,Marine_Deaths,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical));

}

DEATH_DATA *GetPredatorDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical) {

        return(GetDeathSequence(controller,TemplateRoot,Predator_Deaths,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical));

}

DEATH_DATA *GetXenoborgDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
        int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical) {

        return(GetDeathSequence(controller,TemplateRoot,Xenoborg_Deaths,wound_flags,priority_wounds,hurtiness,facing,burning,crouching,electrical));

}

/* Attack Shell */

int CheckAttackValidity(HMODELCONTROLLER *controller,ATTACK_DATA *ThisAttack,int wound_flags,
        int crouching,int pouncing) {

        /* Check against many things.  Harshly. */

        /* Crouching test. */
        if (crouching) {
                if (ThisAttack->Crouching==0) {
                        return(0);
                }
        } else {
                if (ThisAttack->Crouching!=0) {
                        return(0);
                }
        }

        /* Pouncing test. */
        if (pouncing) {
                if (ThisAttack->Pouncing==0) {
                        return(0);
                }
        } else {
                if (ThisAttack->Pouncing!=0) {
                        return(0);
                }
        }
        
        /* Wound flags.  Quite odd, and different to deaths. */
        /* If wound_flags are specified in the death, the input must NOT contain them. */
        if (ThisAttack->wound_flags) {
                if (ThisAttack->wound_flags&wound_flags) {
                        return(0);
                }
        }
        
        /* Finally, sequence validity. */
        if (!HModelSequence_Exists(controller,ThisAttack->Sequence_Type,ThisAttack->Sub_Sequence)) {
                return(0);
        }

        /* It got through! */
        return(1);
}

int CountValidAttacks(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int wound_flags,
        int crouching, int pouncing) {

        ATTACK_DATA *this_attack;
        int number_of_candidates;

        number_of_candidates=0;
        this_attack=FirstAttack;

        while (this_attack->Sequence_Type>=0) {
                if (CheckAttackValidity(controller,this_attack,wound_flags,crouching,pouncing)) {
                        number_of_candidates++;
                }
                this_attack++;
        }

        return(number_of_candidates);
}

ATTACK_DATA *GetThisAttack(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int wound_flags,
        int crouching, int pouncing, int index) {

        /* Extract 'index' from the valid attacks. */
        ATTACK_DATA *retval;
        ATTACK_DATA *this_attack;
        int number;

        retval=NULL;

        number=0;
        this_attack=FirstAttack;

        while (this_attack->Sequence_Type>=0) {
                if (CheckAttackValidity(controller,this_attack,wound_flags,crouching,pouncing)) {
                        if (number==index) {
                                retval=this_attack;
                                break;
                        } else {
                                number++;
                        }
                }
                this_attack++;
        }

        GLOBALASSERT(retval);
        return(retval);
}

ATTACK_DATA *GetThisAttack_FromUniqueCode(int code)
{
	extern ATTACK_DATA Alien_Special_Gripping_Attack;
	//search for an attack using a code that should be unique across all attacks
	//(used for loading)

    ATTACK_DATA *this_attack;
	if(code<0) return NULL;


	///try the alien attacks
	this_attack = &Alien_Attacks[0];
	while (this_attack->Sequence_Type>=0) {
    	if (this_attack->Unique_Code==code) {
        	return this_attack;
        	break;
        }
    	this_attack++;
    }
	
	//try the wristblade attacks
	this_attack = &Wristblade_Attacks[0];
	while (this_attack->Sequence_Type>=0) {
    	if (this_attack->Unique_Code==code) {
        	return this_attack;
        	break;
        }
    	this_attack++;
    }
	
	//try the staff attacks
	this_attack = &PredStaff_Attacks[0];
	while (this_attack->Sequence_Type>=0) {
    	if (this_attack->Unique_Code==code) {
        	return this_attack;
        	break;
        }
    	this_attack++;
    }

	//try gripping attack
	if(Alien_Special_Gripping_Attack.Unique_Code==code)
	{
		return &Alien_Special_Gripping_Attack;
	}

	//no such attack
	return NULL;
}


ATTACK_DATA *GetThisAttack_FromCode(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int code) {

        /* Extract 'code' from the valid attacks. */
        ATTACK_DATA *retval;
        ATTACK_DATA *this_attack;

        retval=NULL;

        this_attack=FirstAttack;

        while (this_attack->Sequence_Type>=0) {
                if (this_attack->Multiplayer_Code==code) {
                        retval=this_attack;
                        break;
                }
                this_attack++;
        }

        GLOBALASSERT(retval);
        return(retval);
}

ATTACK_DATA *GetAttackSequence(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int wound_flags,int crouching, int pouncing) {

        int number_of_candidates;
        int index;
        
        int use_wound_flags;
        int use_crouching;

        use_wound_flags=wound_flags;
        use_crouching=crouching;

        number_of_candidates=0;

        while (number_of_candidates==0) {
                /* Iterate, making simplifications, until there are valid deaths. */
                number_of_candidates=CountValidAttacks(controller,FirstAttack,use_wound_flags,use_crouching,pouncing);
                if (number_of_candidates==0) {
                        /* Wound flags first. */
                        if (use_wound_flags!=0) {
                                use_wound_flags=0;
                                continue;
                        }
                        /* Only crouch is left! */
                        if (use_crouching) {
                                use_crouching=0;
                                continue;
                        }
                        /* Now, pounce is absolutely inviolate. */
                        if (pouncing) {
                                /* If we're looking for a pounce, and there is none, return NULL. */
                                return(NULL);
                        }
                        //NewOnScreenMessage("ATTACK SELECTION FAILURE!\n");
                        /* Here goes nothing. */
                        return(FirstAttack);
                }
        }
        
        /* Right, by now we should have a number of candidates. */
        GLOBALASSERT(number_of_candidates);

        index=FastRandom()%number_of_candidates;

        return(GetThisAttack(controller,FirstAttack,use_wound_flags,
                use_crouching,pouncing,index));
}

ATTACK_DATA *GetAlienAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching) {

        return(GetAttackSequence(controller,Alien_Attacks,wound_flags,crouching,0));

}

ATTACK_DATA *GetAlienPounceAttack(HMODELCONTROLLER *controller,int wound_flags,int crouching) {

        return(GetAttackSequence(controller,Alien_Attacks,wound_flags,crouching,1));

}

ATTACK_DATA *GetWristbladeAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching) {

        return(GetAttackSequence(controller,Wristblade_Attacks,wound_flags,crouching,0));

}

ATTACK_DATA *GetPredStaffAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching) {

        return(GetAttackSequence(controller,PredStaff_Attacks,wound_flags,crouching,0));

}

AIMODULE *NearNPC_GetTargetAIModuleForRetreat(STRATEGYBLOCK *sbPtr, NPC_MOVEMENTDATA *moveData)
{
        extern unsigned int PlayerSmell;
        
        AIMODULE **AdjModuleRefPtr;
        AIMODULE* targetModule = (AIMODULE *)0;
        unsigned int targetSmell = PlayerSmell + 1;     /* should be higher than any smell anywhere this frame */
        unsigned int targetNumAdj = 0;
        int targetEpDot=-ONE_FIXED;
        VECTORCH lastVelocityDirection;
        int gotLastVelocityDirection = 0;

        LOCALASSERT(sbPtr);     
        if(sbPtr->containingModule==NULL) return targetModule;  
        AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;

        /* try to get our last velocity direction */
        if((moveData->lastVelocity.vx!=0)||(moveData->lastVelocity.vz!=0)||(moveData->lastVelocity.vy!=0))
        {
                lastVelocityDirection = moveData->lastVelocity;
                Normalise(&lastVelocityDirection);
                gotLastVelocityDirection = 1;
        }
        else gotLastVelocityDirection = 0;

        /* check that there is a list of adjacent modules, and that it is not
        empty (ie points to zero) */
        if(AdjModuleRefPtr)     
        {
                while(*AdjModuleRefPtr != 0)
                {

                        /* get the index */
                        int AdjModuleIndex = (*AdjModuleRefPtr)->m_index;
                        int AdjModuleSmell = PherPl_ReadBuf[AdjModuleIndex];
                        FARENTRYPOINT *thisEp = GetAIModuleEP((*AdjModuleRefPtr), sbPtr->containingModule->m_aimodule);
                        int thisEpDot = -ONE_FIXED;
                        int chooseThisOne = 0;

                        if(thisEp)
                        {
                                /* aha. an ep!... */ 
                                VECTORCH thisEpWorld = thisEp->position;

                                thisEpWorld.vx += (*AdjModuleRefPtr)->m_world.vx;
                                thisEpWorld.vy += (*AdjModuleRefPtr)->m_world.vy;
                                thisEpWorld.vz += (*AdjModuleRefPtr)->m_world.vz;                       

                                if(gotLastVelocityDirection)
                                {
                                        VECTORCH thisEpDirection;

                                        thisEpDirection = thisEpWorld;
                                        thisEpDirection.vx -= sbPtr->DynPtr->Position.vx;
                                        thisEpDirection.vy -= sbPtr->DynPtr->Position.vy;
                                        thisEpDirection.vz -= sbPtr->DynPtr->Position.vz;
                                        Normalise(&thisEpDirection);
                                        thisEpDot = DotProduct(&thisEpDirection,&lastVelocityDirection);
                                }
                        }

                        /* if this adjacent module's smell value is lower than
                        the current 'highest smell' record the new module as the
                        target.  
                        Tie break on best direction. */

                        if (!targetModule) {
                                chooseThisOne=1;
                        } else {
                                if (AdjModuleSmell < targetSmell) {
                                        chooseThisOne=1;
                                } else if (AdjModuleSmell == targetSmell) {
                                        if (thisEpDot>targetEpDot) {
                                                chooseThisOne=1;
                                        }
                                }
                        }

                        if (chooseThisOne)
                        {                                               
                                targetSmell = PherPl_ReadBuf[AdjModuleIndex];
                                targetModule = *AdjModuleRefPtr;
                                targetNumAdj = NumAdjacentModules(*AdjModuleRefPtr);                                                    
                                targetEpDot = thisEpDot;
                        }
                        /* next adjacent module reference pointer */
                        AdjModuleRefPtr++;
                }
        }
        return targetModule;
}

AIMODULE *General_GetRetreatModule_Core(STRATEGYBLOCK *sbPtr,AIMODULE *source,int max_depth) {
        
        AIMODULE **AdjModuleRefPtr;
        AIMODULE *deepest_target;
        NL_ROUTE_QUEUE deepest_route;
        
        /* Note this DOES NOT set the CallsThisFrame variable.  That MUST be set before the call. */

        /* Clear the start. */

        deepest_route.depth=0;
        deepest_route.aimodule=NULL;
        deepest_route.first_step=NULL;
        deepest_target=NULL;

        NearLink_Route_Queue[0].depth=0;
        NearLink_Route_Queue[0].aimodule=source;
        NearLink_Route_Queue[0].first_step=NULL;
        NearLink_Route_Queue[1].aimodule=NULL; /* To set a standard. */

        NL_Queue_End=1;
        NL_Queue_Exec=0;

        while (NearLink_Route_Queue[NL_Queue_Exec].aimodule!=NULL) {
                
                AIMODULE *thisModule;

                thisModule=NearLink_Route_Queue[NL_Queue_Exec].aimodule;

                AdjModuleRefPtr = thisModule->m_link_ptrs;
        
                if(AdjModuleRefPtr)     /* check that there is a list of adjacent modules */
                {
                        while(*AdjModuleRefPtr != 0)
                        {
                                /* Probably want some validity test for the link. */
                                if ((AIModuleIsPhysical(*AdjModuleRefPtr))
                                        &&(AIModuleAdmitsPheromones(*AdjModuleRefPtr))
                                        /* No visibility check? */
                                        ) {
                                        
                                        /* Consider depth? */
                                        if (NearLink_Route_Queue[NL_Queue_Exec].depth<deepest_route.depth) {
                                                deepest_route=NearLink_Route_Queue[NL_Queue_Exec];
                                                deepest_target=(*AdjModuleRefPtr);
                                        }

                                        /* Process link. */
                                        if ((NearLink_Route_Queue[NL_Queue_Exec].depth<max_depth)
                                                &&( /* Test for 'used this time round' */
                                                        ((*AdjModuleRefPtr)->RouteFinder_FrameStamp!=GlobalFrameCounter)
                                                        ||((*AdjModuleRefPtr)->RouteFinder_IterationNumber!=RouteFinder_CallsThisFrame)
                                                )) {
                                                /* Add to queue. */
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=(*AdjModuleRefPtr);
                                                NearLink_Route_Queue[NL_Queue_End].depth=NearLink_Route_Queue[NL_Queue_Exec].depth+1;
                                                /* Remember first step. */
                                                if (NearLink_Route_Queue[NL_Queue_Exec].first_step==NULL) {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=(*AdjModuleRefPtr);
                                                } else {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=NearLink_Route_Queue[NL_Queue_Exec].first_step;
                                                }
                                                /* Stamp as used. */
                                                (*AdjModuleRefPtr)->RouteFinder_FrameStamp=GlobalFrameCounter;
                                                (*AdjModuleRefPtr)->RouteFinder_IterationNumber=RouteFinder_CallsThisFrame;
                                                NL_Queue_End++;
                                                if (NL_Queue_End>=NEARLINK_QUEUE_LENGTH) {
                                                        NL_Queue_End=0;
                                                        textprint("Wrapping Nearlink Queue!\n");
                                                }
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=NULL;
                                                if (NL_Queue_End==NL_Queue_Exec) {
                                                        LOGDXFMT(("Oh, no.  NearLinkQueue screwed.  NL_Queue_End=%d, depth = %d\n",NL_Queue_End,NearLink_Route_Queue[NL_Queue_Exec].depth));
                                                        LOCALASSERT(NL_Queue_End!=NL_Queue_Exec); //if this happens the queue probably needs to be longer
                                                }
                                        } else if (NearLink_Route_Queue[NL_Queue_Exec].depth>=max_depth) {
                                                /* That's well deep.  Let's return. */

                                                return(*AdjModuleRefPtr);

                                        }
                                }
                                /* next adjacent module reference pointer */
                                AdjModuleRefPtr++;
                        }
                }

                /* Done all the links. */
                NL_Queue_Exec++;
                if (NL_Queue_Exec>=NEARLINK_QUEUE_LENGTH) NL_Queue_Exec=0;

        }

        if (deepest_target) {
                /* Split up for easier debugging... */
                return(deepest_target);
        } else {
                /* There's nowhere to retreat to! */
                return(NULL);
        }

}

AIMODULE *General_GetAIModuleForRetreat(STRATEGYBLOCK *sbPtr,AIMODULE *fearModule,int max_depth) {
        
        AIMODULE **AdjModuleRefPtr;
        AIMODULE *my_module;
        int success;
        
        GLOBALASSERT(sbPtr->containingModule);
        my_module=sbPtr->containingModule->m_aimodule;
        GLOBALASSERT(my_module);

        if (fearModule==NULL) {
                return(NULL);
        }

        /* Hmm... maybe we need to consider being in sight at some point. */
        #if 0
        /* Firstly, are we in sight of the fear module? */
        if (IsModuleVisibleFromModule((*fearModule->m_module_ptrs),sbPtr->containingModule)) {
                /* Hmm, still in sight.  Try to get out of sight. */
        } else {
        }
        #endif

        /* Step one: search down till we get to my_module, checking off modules. */
        
        if (my_module==fearModule) {
                /* Cripes! */
                AIMODULE *targetModule;

                RouteFinder_CallsThisFrame++;
                targetModule=General_GetRetreatModule_Core(sbPtr,my_module,max_depth);
                return(targetModule);

        }

        /* Clear the start. */

        NearLink_Route_Queue[0].depth=0;
        NearLink_Route_Queue[0].aimodule=fearModule;
        NearLink_Route_Queue[0].first_step=NULL;
        NearLink_Route_Queue[1].aimodule=NULL; /* To set a standard. */

        NL_Queue_End=1;
        NL_Queue_Exec=0;
        success=0;
        
        /* Hijack RouteFinder. */       
        RouteFinder_CallsThisFrame++;

        while (NearLink_Route_Queue[NL_Queue_Exec].aimodule!=NULL) {
                
                AIMODULE *thisModule;

                thisModule=NearLink_Route_Queue[NL_Queue_Exec].aimodule;

                AdjModuleRefPtr = thisModule->m_link_ptrs;
        
                if(AdjModuleRefPtr)     /* check that there is a list of adjacent modules */
                {
                        while(*AdjModuleRefPtr != 0)
                        {
                                /* Probably want some validity test for the link. */
                                if ((AIModuleIsPhysical(*AdjModuleRefPtr))
                                        &&(AIModuleAdmitsPheromones(*AdjModuleRefPtr))
                                        /* No visibility check. */
                                        ) {
                                        /* Is this my_module? */
                                        if ( (*AdjModuleRefPtr)==my_module) {
                                                /* Yes!!! Break out. */
                                                success=1;
                                                break;
                                        } else if (
                                                (NearLink_Route_Queue[NL_Queue_Exec].depth<max_depth)
                                                &&( /* Test for 'used this time round' */
                                                        ((*AdjModuleRefPtr)->RouteFinder_FrameStamp!=GlobalFrameCounter)
                                                        ||((*AdjModuleRefPtr)->RouteFinder_IterationNumber!=RouteFinder_CallsThisFrame)
                                                )) {

                                                success=0;
                                                /* Add to queue. */
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=(*AdjModuleRefPtr);
                                                NearLink_Route_Queue[NL_Queue_End].depth=NearLink_Route_Queue[NL_Queue_Exec].depth+1;
                                                /* Remember first step. */
                                                if (NearLink_Route_Queue[NL_Queue_Exec].first_step==NULL) {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=(*AdjModuleRefPtr);
                                                } else {
                                                        NearLink_Route_Queue[NL_Queue_End].first_step=NearLink_Route_Queue[NL_Queue_Exec].first_step;
                                                }
                                                /* Stamp as used. */
                                                (*AdjModuleRefPtr)->RouteFinder_FrameStamp=GlobalFrameCounter;
                                                (*AdjModuleRefPtr)->RouteFinder_IterationNumber=RouteFinder_CallsThisFrame;
                                                NL_Queue_End++;
                                                if (NL_Queue_End>=NEARLINK_QUEUE_LENGTH) {
                                                        NL_Queue_End=0;
                                                        textprint("Wrapping Nearlink Queue!\n");
                                                }
                                                NearLink_Route_Queue[NL_Queue_End].aimodule=NULL;
                                                if (NL_Queue_End==NL_Queue_Exec) {
                                                        LOGDXFMT(("Oh, no.  NearLinkQueue screwed.  NL_Queue_End=%d, depth = %d\n",NL_Queue_End,NearLink_Route_Queue[NL_Queue_Exec].depth));
                                                        LOCALASSERT(NL_Queue_End!=NL_Queue_Exec); //if this happens the queue probably needs to be longer
                                                }
                                        }
                                }
                                /* next adjacent module reference pointer */
                                AdjModuleRefPtr++;
                        }
                }

                /* Done all the links. */

                if (success) {
                        /* Continue break out. */
                        break;
                }

                NL_Queue_Exec++;
                if (NL_Queue_Exec>=NEARLINK_QUEUE_LENGTH) NL_Queue_Exec=0;

        }
        
        /* By now, we should have broken out... or maxed out the range. */
        if (success) {
                AIMODULE *targetModule;
                targetModule=General_GetRetreatModule_Core(sbPtr,my_module,max_depth);
                return(targetModule);
        } else {
                return(NULL);
        }
}

int AIModuleIsVisible(AIMODULE *aimodule) {
        
        MODULE **module_list;
        /* D'oh! */
        module_list=aimodule->m_module_ptrs;

        while (*module_list) {
                if (ModuleCurrVisArray[(*module_list)->m_index]) {
                        return(1);
                }
                module_list++;
        }

        return(0);
}

int IsMyPolyRidiculous(void) {

        int a,sideend,distance;
        VECTORCH side;

        /* Please make sure the globals are sensible! */

        a=0;

        for (a=0; a<GMD_myPolyNumPoints; a++) {
        
                if (a>=(GMD_myPolyNumPoints-1)) {
                        sideend=0;
                } else {
                        sideend=a+1;
                }

                side.vx=GMD_myPolyPoints[a].vx-GMD_myPolyPoints[sideend].vx;
                side.vy=GMD_myPolyPoints[a].vy-GMD_myPolyPoints[sideend].vy;
                side.vz=GMD_myPolyPoints[a].vz-GMD_myPolyPoints[sideend].vz;

                distance=Approximate3dMagnitude(&side);

                if (distance<1000) {
                        /* Stoopid! */
                        return(1);
                }
        }       
        
        return(0);
}

void Initialise_AvoidanceManager(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager) {

        DYNAMICSBLOCK *dynPtr;

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        ClearThirdAvoidance(sbPtr,manager);

        manager->avoidanceDirection.vx=0;
        manager->avoidanceDirection.vy=0;
        manager->avoidanceDirection.vz=0;

        manager->incidenceDirection.vx=0;
        manager->incidenceDirection.vy=0;
        manager->incidenceDirection.vz=0;

        manager->incidentPoint.vx=0;
        manager->incidentPoint.vy=0;
        manager->incidentPoint.vz=0;

        manager->aggregateNormal.vx=0;
        manager->aggregateNormal.vy=0;
        manager->aggregateNormal.vz=0;

        manager->recommendedDistance=0;
        manager->timer=0;
        manager->primaryCollision=NULL; 
        manager->substate=AvSS_FreeMovement;

		if ((sbPtr->I_SBtype==I_BehaviourAlien)||(sbPtr->I_SBtype==I_BehaviourFaceHugger)) {
			/* Allows destruction of explosive objects. */
			manager->ClearanceDamage=AMMO_ALIEN_OBSTACLE_CLEAR;
		} else {
			manager->ClearanceDamage=AMMO_NPC_OBSTACLE_CLEAR;
		}
}

int New_NPC_IsObstructed(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager)
{
    DYNAMICSBLOCK *dynPtr;
    struct collisionreport *nextReport;
    VECTORCH myVelocityDirection,aggregateNormal;
    int numObstructiveCollisions;
    STRATEGYBLOCK *highestPriorityCollision;
	COLLISIONREPORT vcr;

    LOCALASSERT(manager);
    LOCALASSERT(sbPtr);
    dynPtr = sbPtr->DynPtr;
    LOCALASSERT(dynPtr);
    nextReport = dynPtr->CollisionReportPtr;
    
    numObstructiveCollisions=0;

    /* check our velocity: if we haven't got one, we can't be obstructed, so just return */
    if((sbPtr->DynPtr->LinVelocity.vx==0)&&(sbPtr->DynPtr->LinVelocity.vy==0)&&(sbPtr->DynPtr->LinVelocity.vz==0))
    {
    	return(0);
    }
    
	if (sbPtr->I_SBtype==I_BehaviourMarine) {
		if (SimpleEdgeDetectionTest(sbPtr,&vcr)) {
			vcr.NextCollisionReportPtr=nextReport;
			nextReport=&vcr;
		}
	}

    if (nextReport==NULL) {
        /* Trivial reject to save time. */
        return(0);
    }

    /* get my velocity direction, normalised... */
    myVelocityDirection = dynPtr->LinVelocity;
    Normalise(&myVelocityDirection);

    highestPriorityCollision=(STRATEGYBLOCK *)-1;
    aggregateNormal.vx=0;
    aggregateNormal.vy=0;
    aggregateNormal.vz=0;
    
    if (manager->substate==AvSS_FreeMovement) {
        Initialise_AvoidanceManager(sbPtr,manager);
    }

    /* Walk the collision report list. */
    while(nextReport)
    {               
        int normalDotWithVelocity;
        
        if(nextReport->ObstacleSBPtr)
        {
            /* Possible testing for type here.  Personally, I don't care, apart from destructibles. */
        }

        normalDotWithVelocity = DotProduct(&(nextReport->ObstacleNormal),&myVelocityDirection);
	
		#if 0
        if(((normalDotWithVelocity < -46341)||
            ((nextReport->ObstacleSBPtr)&&(!SBIsEnvironment(nextReport->ObstacleSBPtr))&&(normalDotWithVelocity < -32768))))
            /* 45 degs vs environment, 60 degs vs objects. */
		#else
        if (normalDotWithVelocity < -32768)
		#endif
        {
            /* If we're in FirstAvoidance already, might want to disregard the same collision again. */
            if (manager->substate==AvSS_FirstAvoidance) {
                if (!SBIsEnvironment(manager->primaryCollision)) {
                    if (manager->primaryCollision==nextReport->ObstacleSBPtr) {
                        /* Advance and continue. */
                        nextReport = nextReport->NextCollisionReportPtr;
                        continue;
                    }
                }
            }
            
            /* We have detected a collision with a strategy, or an obstructive environment bit. */
            numObstructiveCollisions++;
            aggregateNormal.vx+=nextReport->ObstacleNormal.vx;
            aggregateNormal.vy+=nextReport->ObstacleNormal.vy;
            aggregateNormal.vz+=nextReport->ObstacleNormal.vz;

            /* Sort out highest priority collision. */
            if (highestPriorityCollision==(STRATEGYBLOCK *)-1) {
                highestPriorityCollision=nextReport->ObstacleSBPtr;
            } else {
                /* If this collision is with the environment, and the older one was not, replace it. */
                if (SBIsEnvironment(nextReport->ObstacleSBPtr)&&(!SBIsEnvironment(highestPriorityCollision))) {
                    highestPriorityCollision=nextReport->ObstacleSBPtr;
                }
            }

            {
                if(nextReport->ObstacleSBPtr)
                {
                    if(nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourInanimateObject)
                    {
                        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = nextReport->ObstacleSBPtr->SBdataptr;
                        if (objectstatusptr) {
                        	if (objectstatusptr->Indestructable == 0) {
								/* Consider explosive objects as obstructions to most things. */
								if ((objectstatusptr->explosionType==0)||(manager->ClearanceDamage!=AMMO_NPC_OBSTACLE_CLEAR)) {
		                            /* aha: an object which the npc can destroy... damage it, and return zero. */
		                            CauseDamageToObject(nextReport->ObstacleSBPtr,TemplateAmmo[manager->ClearanceDamage].MaxDamage, ONE_FIXED,NULL);
		                            return(0);
		                            /* After a few frames of that, there'll just be real obstructions. */
								}
							}
                        }
                    }
                }                                        
            }
        }
        nextReport = nextReport->NextCollisionReportPtr;
    }

    if (numObstructiveCollisions==0) {
        /* No collisions!  Woohoo!  But don't reset the substate... */
        return(0);
    }
    
    switch (manager->substate) {
    
        case AvSS_FreeMovement:
        default:
        {
            /* Right, we've run into something all right. */
            GLOBALASSERT(highestPriorityCollision!=(STRATEGYBLOCK *)-1);
        
            manager->primaryCollision=highestPriorityCollision;
            manager->incidenceDirection=myVelocityDirection;
            manager->incidentPoint=dynPtr->Position;
            /* Decide on a distance... */
        
            if (SBIsEnvironment(manager->primaryCollision)) {
                manager->recommendedDistance=3000;
            } else {
                manager->recommendedDistance=2000;
            }
            manager->timer=STANDARD_AVOIDANCE_TIME;
        
            /* ...and a direction. */
            Normalise(&aggregateNormal);
            manager->aggregateNormal=aggregateNormal;
        
            if (New_GetAvoidanceDirection(sbPtr,manager,&aggregateNormal)==0) {
                /* No valid directions in pass 1 - not dealt with yet! */
                GLOBALASSERT(0);
            }
        
            manager->substate=AvSS_FirstAvoidance;
        
            return(1);
        }
        break;
        case AvSS_FirstAvoidance:
        {
            /* Right, we've run into something again. */
            
            /* Retain point, direction and distance. */
        
            /* ...but get a new direction. */
            aggregateNormal.vx+=manager->aggregateNormal.vx;
            aggregateNormal.vy+=manager->aggregateNormal.vy;
            aggregateNormal.vz+=manager->aggregateNormal.vz;
            /* Add the new aggregatenormal to the old one, and normalise... */
            Normalise(&aggregateNormal);
            /* Then pass that number into the second direction system. */
            if (New_GetSecondAvoidanceDirection(sbPtr,manager,&aggregateNormal)==0) {
                /* No valid directions in pass 2 - not dealt with yet! */
                GLOBALASSERT(0);
            }
        
            manager->substate=AvSS_SecondAvoidance;
            manager->timer=STANDARD_AVOIDANCE_TIME;
        
            return(1);
        }
        case AvSS_SecondAvoidance:
        case AvSS_ThirdAvoidance:
        {
            /* Right, we've run into something again again. (Again.) */
            
            /* Retain point, direction and distance, and go directly to third avoidance. */
        
            manager->timer=STANDARD_AVOIDANCE_TIME;
            InitialiseThirdAvoidance(sbPtr,manager);
                            
            return(1);
        }
        break;
    }
}

AVOIDANCE_RETURN_CONDITION AllNewAvoidanceKernel(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager) {

        DYNAMICSBLOCK *dynPtr;

        /* Velocity must be set deliberately... even if it's null. */

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);
        
        /* Want to split here based on substate. */

        switch (manager->substate) {
                case AvSS_FreeMovement:
                {
                        /* Really shouldn't be here.  Go'way. */
                        Initialise_AvoidanceManager(sbPtr,manager);
                        return(AvRC_Clear);
                        break;
                }
                case AvSS_FirstAvoidance:
                {
                        if (New_NPC_IsObstructed(sbPtr,manager)==1) {
                                /* We're obstructed AGAIN! */
                                return(AvRC_Avoidance);
                        }

                        /* Are we far enough away? */
                        {
                                int distance;
                                VECTORCH offset;
                
                                offset.vx=dynPtr->Position.vx-manager->incidentPoint.vx;
                                offset.vy=dynPtr->Position.vy-manager->incidentPoint.vy;
                                offset.vz=dynPtr->Position.vz-manager->incidentPoint.vz;
        
                                distance=Approximate3dMagnitude(&offset);

                                if (distance>manager->recommendedDistance) {
                                        /* Exit! */
                                        Initialise_AvoidanceManager(sbPtr,manager);
                                        return(AvRC_Clear);
                                }
                        }

                        manager->timer-=NormalFrameTime;
                        if (manager->timer<=0) {
                                /* Ooh, we're in a fix here... */
                                /* Probably need Avoidance 3 for this. */
                                InitialiseThirdAvoidance(sbPtr,manager);
                                return(AvRC_Avoidance);
                        }
                        break;
                }
                case AvSS_SecondAvoidance:
                {
                        if (New_NPC_IsObstructed(sbPtr,manager)==1) {
                                /* Should be in Third Avoidance here. */
                                return(AvRC_Avoidance);
                        }

                        /* Are we far enough away? */
                        {
                                int distance;
                                VECTORCH offset;
                
                                offset.vx=dynPtr->Position.vx-manager->incidentPoint.vx;
                                offset.vy=dynPtr->Position.vy-manager->incidentPoint.vy;
                                offset.vz=dynPtr->Position.vz-manager->incidentPoint.vz;
        
                                distance=Approximate3dMagnitude(&offset);

                                if (distance>manager->recommendedDistance) {
                                        /* Exit! */
                                        Initialise_AvoidanceManager(sbPtr,manager);
                                        return(AvRC_Clear);
                                }
                        }

                        manager->timer-=NormalFrameTime;
                        if (manager->timer<=0) {
                                /* Ooh, we're in a fix here... */
                                /* Probably need Avoidance 3 for this. */
                                InitialiseThirdAvoidance(sbPtr,manager);
                                return(AvRC_Avoidance);
                        }
                        break;
                }
                case AvSS_ThirdAvoidance:
                {
                        int result;
                        /* Let's have a whole function here! */
                        result=ExecuteThirdAvoidance(sbPtr,manager);
                        if (result==-1) {
                                /* Totally fubared.  Return failure. */
                                Initialise_AvoidanceManager(sbPtr,manager);
                                return(AvRC_Failure);
                        } else if (result==1) {
                                /* Success!  Return clear. */
                                Initialise_AvoidanceManager(sbPtr,manager);
                                return(AvRC_Clear);
                        } else {
                                /* Still going, return avoidance. */
                                return(AvRC_Avoidance);
                        }
                        break;
                }
                default:
                        GLOBALASSERT(0);
                        break;
        }

        return(AvRC_Avoidance);

}

int New_GetAvoidanceDirection(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager, VECTORCH *aggregateNormal) {

        DYNAMICSBLOCK *dynPtr;
        int dot;
        VECTORCH spaceNormal,transverse;
        VECTORCH direction[4];
        /* Yeesh. */

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        /* aggregateNormal should be normalised, and should point away from the collisions. */
        /* First dot it with gravity... */
        
        if (dynPtr->UseStandardGravity) {
                dynPtr->GravityDirection.vx=0;
                dynPtr->GravityDirection.vy=65536;
                dynPtr->GravityDirection.vz=0;
        }

        dot = -(DotProduct(&dynPtr->GravityDirection,aggregateNormal));
        /* Hold that thought. */
        spaceNormal.vx = (aggregateNormal->vx + MUL_FIXED(dot,dynPtr->GravityDirection.vx));
        spaceNormal.vy = (aggregateNormal->vy + MUL_FIXED(dot,dynPtr->GravityDirection.vy));
        spaceNormal.vz = (aggregateNormal->vz + MUL_FIXED(dot,dynPtr->GravityDirection.vz));
        
        Normalise(&spaceNormal);
        /* Now, spaceNormal should be in the plane we want to consider. */
        CrossProduct(&spaceNormal,&dynPtr->GravityDirection,&transverse);
        Normalise(&transverse);
        /* ...And 'transverse' should be at 90degs to it. */

        /* For now, emulate the old avoidance code... */

        direction[0]=transverse;
        direction[1].vx=-transverse.vx;
        direction[1].vy=-transverse.vy;
        direction[1].vz=-transverse.vz;

        // Added by Alex - see if we can get a better direction this way.
        direction[2] = spaceNormal;
        direction[3].vx = -direction[2].vx;
        direction[3].vy = -direction[2].vy;
        direction[3].vz = -direction[2].vz;

        direction[0].vx += (spaceNormal.vx/4);
        direction[0].vy += (spaceNormal.vy/4);
        direction[0].vz += (spaceNormal.vz/4);
        direction[1].vx += (spaceNormal.vx/4);
        direction[1].vy += (spaceNormal.vy/4);
        direction[1].vz += (spaceNormal.vz/4);

        direction[2].vx -= (transverse.vx/4);
        direction[2].vy -= (transverse.vy/4);
        direction[2].vz -= (transverse.vz/4);
        direction[3].vx -= (transverse.vx/4);
        direction[3].vy -= (transverse.vy/4);
        direction[3].vz -= (transverse.vz/4);

        Normalise(&direction[0]);
        Normalise(&direction[1]);
        Normalise(&direction[2]);
        Normalise(&direction[3]);
        
        {
                int this_distance, i;
                int best_distance_so_far = 0;
                int best_direction_so_far = 0;

                /* test how far we could go in each direction... */
                for( i=0; i<4; i++ )
                {
                        VECTORCH startingPosition = sbPtr->DynPtr->Position;
                        VECTORCH testDirn = direction[i];

                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                        if(!LOS_ObjectHitPtr) this_distance = NPC_MAX_VIEWRANGE;          
                        else this_distance = LOS_Lambda;

                        if( this_distance > best_distance_so_far )
                        {
                                // What follows is an attempt to make sure we don't jump off any cliffs...
                                VECTORCH test_location;
                                testDirn.vx *= this_distance;
                                testDirn.vy *= this_distance;
                                testDirn.vz *= this_distance;
                                test_location.vx = startingPosition.vx + testDirn.vx;
                                test_location.vy = startingPosition.vy + testDirn.vy;
                                test_location.vz = startingPosition.vz + testDirn.vz;
                                if( CheckMyFloorPoly(&test_location, sbPtr->containingModule) != NPC_GMD_NOPOLY)
                                {
                                        best_direction_so_far = i;
                                        best_distance_so_far = this_distance;
                                }
                        }

                }

                manager->avoidanceDirection = direction[best_direction_so_far];
        }
                        
        return(1);
}

int GetAvoidanceDirection(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager)
{
        DYNAMICSBLOCK *dynPtr;
		VECTORCH newDirection1, newDirection2;
		int dir1dist, dir2dist;
		MATRIXCH matrix;
		EULER euler;

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        /* just in case */
        if(!sbPtr->containingModule) return(0); 

        /* going for a 90 degree turn + back a bit */
		newDirection1 = manager->incidenceDirection;
		newDirection2 = newDirection1;
		euler.EulerX = 0;
		euler.EulerY = 1024;
		euler.EulerZ = 0;
		CreateEulerMatrix( &euler, &matrix );
		RotateVector( &newDirection1, &matrix );
		euler.EulerY = 3072;
		CreateEulerMatrix( &euler, &matrix );
		RotateVector( &newDirection2, &matrix );

#if 0                
        /* construct the direction(s)... 
        start with object's local x unit vector (from local coo-ord system in world space) */
        newDirection1.vx = sbPtr->DynPtr->OrientMat.mat11;
        newDirection1.vy = sbPtr->DynPtr->OrientMat.mat12;
        newDirection1.vz = sbPtr->DynPtr->OrientMat.mat13;
        newDirection2.vx = -newDirection1.vx;
        newDirection2.vy = -newDirection1.vy;
        newDirection2.vz = -newDirection1.vz;
        /* ...and add on 1/4 of the -z direction...*/
        newDirection1.vx -= (sbPtr->DynPtr->OrientMat.mat31/4);
        newDirection1.vy -= (sbPtr->DynPtr->OrientMat.mat32/4);
        newDirection1.vz -= (sbPtr->DynPtr->OrientMat.mat33/4);
        newDirection2.vx -= (sbPtr->DynPtr->OrientMat.mat31/4);
        newDirection2.vy -= (sbPtr->DynPtr->OrientMat.mat32/4);
        newDirection2.vz -= (sbPtr->DynPtr->OrientMat.mat33/4);
#endif

        Normalise(&newDirection1);
        Normalise(&newDirection2);
                
        /* test how far we could go in each direction... */
        {
               VECTORCH startingPosition = sbPtr->DynPtr->Position;
               VECTORCH testDirn = newDirection1;

               LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
               LOS_Lambda = NPC_MAX_VIEWRANGE;
               CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
               if(!LOS_ObjectHitPtr) dir1dist = NPC_MAX_VIEWRANGE;             
               else dir1dist = LOS_Lambda;

               startingPosition = sbPtr->DynPtr->Position;
               testDirn = newDirection2;
               LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
               LOS_Lambda = NPC_MAX_VIEWRANGE;
               CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
               if(!LOS_ObjectHitPtr) dir2dist = NPC_MAX_VIEWRANGE;             
               else dir2dist = LOS_Lambda;
        }

        if(dir1dist > dir2dist) manager->avoidanceDirection = newDirection1;
        else manager->avoidanceDirection = newDirection2;

        return(1);
}

int New_GetSecondAvoidanceDirection(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager, VECTORCH *aggregateNormal) {

        DYNAMICSBLOCK *dynPtr;
        int dot;
        VECTORCH spaceNormal,transverse;
        VECTORCH direction1,direction2;
        /* Yeesh. */

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        /* aggregateNormal should be normalised, and should point away from the collisions. */
        /* First dot it with gravity... */

        dot = -(DotProduct(&dynPtr->GravityDirection,aggregateNormal));
        /* Hold that thought. */
        spaceNormal.vx = (aggregateNormal->vx + MUL_FIXED(dot,dynPtr->GravityDirection.vx));
        spaceNormal.vy = (aggregateNormal->vy + MUL_FIXED(dot,dynPtr->GravityDirection.vy));
        spaceNormal.vz = (aggregateNormal->vz + MUL_FIXED(dot,dynPtr->GravityDirection.vz));
        
        Normalise(&spaceNormal);
        /* Now, spaceNormal should be in the plane we want to consider. */
        CrossProduct(&spaceNormal,&dynPtr->GravityDirection,&transverse);
        Normalise(&transverse);
        /* ...And 'transverse' should be at 90degs to it. */

        /* For now, emulate the old avoidance code... */

        direction1=transverse;
        direction2.vx=-transverse.vx;
        direction2.vy=-transverse.vy;
        direction2.vz=-transverse.vz;

        if ((FastRandom()&65535)<32767) {
                direction1.vx += (spaceNormal.vx/2);
                direction1.vy += (spaceNormal.vy/2);
                direction1.vz += (spaceNormal.vz/2);
                direction2.vx += (spaceNormal.vx/2);
                direction2.vy += (spaceNormal.vy/2);
                direction2.vz += (spaceNormal.vz/2);
        } else {
                direction1.vx += (spaceNormal.vx);
                direction1.vy += (spaceNormal.vy);
                direction1.vz += (spaceNormal.vz);
                direction2.vx += (spaceNormal.vx);
                direction2.vy += (spaceNormal.vy);
                direction2.vz += (spaceNormal.vz);
        }

        Normalise(&direction1);
        Normalise(&direction2);
        
        {
                int dir1dist,dir2dist;
                /* test how far we could go in each direction... */
                {
                        VECTORCH startingPosition = sbPtr->DynPtr->Position;
                        VECTORCH testDirn = direction1;

                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                        if(!LOS_ObjectHitPtr) dir1dist = NPC_MAX_VIEWRANGE;             
                        else dir1dist = LOS_Lambda;

                        startingPosition = sbPtr->DynPtr->Position;
                        testDirn = direction2;
                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&startingPosition,&testDirn,0);
                        if(!LOS_ObjectHitPtr) dir2dist = NPC_MAX_VIEWRANGE;             
                        else dir2dist = LOS_Lambda;
                }

                if(dir1dist > dir2dist) manager->avoidanceDirection = direction1;
                else manager->avoidanceDirection = direction2;
        }
                        
        return(1);
}

void AlignVelocityToGravity(STRATEGYBLOCK *sbPtr,VECTORCH *velocity) {

        DYNAMICSBLOCK *dynPtr;
        int dot;

        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        if (dynPtr->UseStandardGravity) {
                dynPtr->GravityDirection.vx=0;
                dynPtr->GravityDirection.vy=65536;
                dynPtr->GravityDirection.vz=0;
        }

        dot = -(DotProduct(&dynPtr->GravityDirection,velocity));
        /* Hold that thought. */
        velocity->vx = (velocity->vx + MUL_FIXED(dot,dynPtr->GravityDirection.vx));
        velocity->vy = (velocity->vy + MUL_FIXED(dot,dynPtr->GravityDirection.vy));
        velocity->vz = (velocity->vz + MUL_FIXED(dot,dynPtr->GravityDirection.vz));

		if ((velocity->vx==0)&&(velocity->vy==0)&&(velocity->vz==0)) {
			/* That can't be good.  Can it? */
			//velocity->vx=ONE_FIXED;
			return;
		}
        
        Normalise(velocity);

}

void ClearThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager) {

        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);

        manager->baseVector.vx=0;
        manager->baseVector.vy=0;
        manager->baseVector.vz=0;
        manager->currentVector=manager->baseVector;
        manager->avoidanceDirection.vx=0;
        manager->avoidanceDirection.vy=0;
        manager->avoidanceDirection.vz=0;
        manager->bestVector.vx=0;
        manager->bestVector.vy=0;
        manager->bestVector.vz=0;
        manager->basePoint=manager->baseVector;
        manager->stage=0;
        manager->bestDistance=0;
        manager->bestStage=0;
        /* Er... ignore the rotmat for now... */
}

void InitialiseThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager) {

        DYNAMICSBLOCK *dynPtr;
        
        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);

        if (dynPtr->UseStandardGravity) {
                dynPtr->GravityDirection.vx=0;
                dynPtr->GravityDirection.vy=65536;
                dynPtr->GravityDirection.vz=0;
        }
        /* Setup base vector. */
        {
                int dot;
                /* Try using positive... z. */
                manager->baseVector.vx=sbPtr->DynPtr->OrientMat.mat31;
                manager->baseVector.vy=sbPtr->DynPtr->OrientMat.mat32;
                manager->baseVector.vz=sbPtr->DynPtr->OrientMat.mat33;
                
                dot = (DotProduct(&dynPtr->GravityDirection,&manager->baseVector));
                
                if (!((dot<65000)&&(dot>-65000))) {
                        /* Too close.  Let's use x. */
                        manager->baseVector.vx=sbPtr->DynPtr->OrientMat.mat11;
                        manager->baseVector.vy=sbPtr->DynPtr->OrientMat.mat12;
                        manager->baseVector.vz=sbPtr->DynPtr->OrientMat.mat13;
                }
                AlignVelocityToGravity(sbPtr,&manager->baseVector);
        }
        manager->currentVector=manager->baseVector;

        /* Keep still! */
        manager->avoidanceDirection.vx=0;
        manager->avoidanceDirection.vy=0;
        manager->avoidanceDirection.vz=0;
        
        manager->bestVector.vx=0;
        manager->bestVector.vy=0;
        manager->bestVector.vz=0;

        manager->basePoint=dynPtr->Position;

        manager->stage=0;
        manager->bestDistance=0;
        manager->bestStage=0;
        
        /* Finally, generate a matrix to rotate 22.5degs about GravityDirection. */

        {
                QUAT deltaRotQ;
                /* Angle is 256, halfangle is 128. */   
                int cosHalfAngle = GetCos(128);
                int sinHalfAngle = GetSin(128);

                deltaRotQ.quatw=cosHalfAngle;
                deltaRotQ.quatx=MUL_FIXED(sinHalfAngle,dynPtr->GravityDirection.vx);
                deltaRotQ.quaty=MUL_FIXED(sinHalfAngle,dynPtr->GravityDirection.vy);
                deltaRotQ.quatz=MUL_FIXED(sinHalfAngle,dynPtr->GravityDirection.vz);

                QNormalise(&deltaRotQ);
                QuatToMat(&deltaRotQ,&manager->rotationMatrix);

        }
        /* Say we're in Third Avoidance. */
        manager->substate=AvSS_ThirdAvoidance;
}

int ExecuteThirdAvoidance(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager) {
        
        DYNAMICSBLOCK *dynPtr;
        
        LOCALASSERT(manager);
        LOCALASSERT(sbPtr);
        dynPtr = sbPtr->DynPtr;
        LOCALASSERT(dynPtr);
        
        if (manager->stage<9) {
                /* Still in the spin.  Increment stage NOW! */
                manager->stage++;
                /* Now raycast. */
                {
                        VECTORCH testDirn = manager->currentVector;

                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&manager->basePoint,&testDirn,0);
                        
                        #if 0
                        MakeParticle(&manager->basePoint,&testDirn,PARTICLE_PREDATOR_BLOOD);
                        #endif

                        if (LOS_ObjectHitPtr) {
                                /* Hit environment! */
                                if (LOS_Lambda>manager->bestDistance) {
                                        /* Register this as best. */
                                        manager->bestDistance=LOS_Lambda;
                                        manager->bestStage=manager->stage;
                                        manager->bestVector=testDirn;
                                }
                        }

                        testDirn.vx = -manager->currentVector.vx;
                        testDirn.vy = -manager->currentVector.vy;
                        testDirn.vz = -manager->currentVector.vz;
                        LOS_ObjectHitPtr = (DISPLAYBLOCK *)0;
                        LOS_Lambda = NPC_MAX_VIEWRANGE;
                        CheckForVectorIntersectionWith3dObject(sbPtr->containingModule->m_dptr,&manager->basePoint,&testDirn,0);

                        #if 0
                        MakeParticle(&manager->basePoint,&testDirn,PARTICLE_PREDATOR_BLOOD);
                        #endif

                        if (LOS_ObjectHitPtr) {
                                /* Hit environment! */
                                if (LOS_Lambda>manager->bestDistance) {
                                        /* Register this as best. */
                                        manager->bestDistance=LOS_Lambda;
                                        manager->bestStage=-manager->stage;
                                        manager->bestVector=testDirn;
                                }
                        }
                }
                /* Now, rotate vector round. */
                RotateVector(&manager->currentVector,&manager->rotationMatrix);
        } else if (manager->stage==9) {
                /* Now, we must have checked all 16 directions. */
                if (manager->bestStage==0) {
                        /* That's a bit of a mystery.  Return fubared. */
                        return(-1);
                }
                /* Now, my beautiful assistant, open the envelope! */
                if (manager->bestDistance<THIRD_AVOIDANCE_MINDIST) {
                        /* Whatta loada junk. */
                        return(-1);
                } else {
                        /* Go, my child, in the direction of destiny! */
                        manager->avoidanceDirection=manager->bestVector;
                        Normalise(&manager->avoidanceDirection);
                        manager->stage++;
                        return(0);
                }
        } else {
                /* In stage 10, we're going that way and trying to escape. */
                if (New_NPC_IsObstructed(sbPtr,manager)==1) {
                        int distance;
                        VECTORCH offset;
                        /* We're obstructed AGAIN!  Gordon Bennet... restart? */
                        offset.vx=dynPtr->Position.vx-manager->basePoint.vx;
                        offset.vy=dynPtr->Position.vy-manager->basePoint.vy;
                        offset.vz=dynPtr->Position.vz-manager->basePoint.vz;
                        
                        distance=Approximate3dMagnitude(&offset);
                        if (distance<300) {
                                /* I suspect restarting will get us nowhere, *
                                 * something's in the primary path.  Fail!   */
                                return(-1);
                        }
                        InitialiseThirdAvoidance(sbPtr,manager);
                        return(0);
                }

                /* Are we far enough away? */
                {
                        int distance;
                        VECTORCH offset;
                
                        offset.vx=dynPtr->Position.vx-manager->incidentPoint.vx;
                        offset.vy=dynPtr->Position.vy-manager->incidentPoint.vy;
                        offset.vz=dynPtr->Position.vz-manager->incidentPoint.vz;
        
                        distance=Approximate3dMagnitude(&offset);
                        if (distance>manager->recommendedDistance) {
                                /* Now let's check against third avoidance. */
                                offset.vx=dynPtr->Position.vx-manager->basePoint.vx;
                                offset.vy=dynPtr->Position.vy-manager->basePoint.vy;
                                offset.vz=dynPtr->Position.vz-manager->basePoint.vz;
        
                                distance=Approximate3dMagnitude(&offset);
                                if (distance>(THIRD_AVOIDANCE_MINDIST>>1)) {
                                        /* Exit with success!  Glory be! */
                                        return(1);
                                }
                        } else {
                                /* Still check, to stop repeated third avoidance bouncing. */
                                offset.vx=dynPtr->Position.vx-manager->basePoint.vx;
                                offset.vy=dynPtr->Position.vy-manager->basePoint.vy;
                                offset.vz=dynPtr->Position.vz-manager->basePoint.vz;
        
                                distance=Approximate3dMagnitude(&offset);
                                if (distance>((manager->bestDistance)>>1)) {
                                        /* Exit with success, before we look stupid! */
                                        return(1);
                                }
                        }
                }
                /* Still here, hmm? */
                return(0);
        }

        return(0);

}

void CastLOSSpear(STRATEGYBLOCK *sbPtr, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int inaccurate) {
        
        VECTORCH shotVector;
        DISPLAYBLOCK *self;

        shotVector=*in_shotvector;

        if (sbPtr) {
                self=sbPtr->SBdptr;
        } else {
                self=NULL;
        }
        
        /* Normalise. */
        Normalise(&shotVector);

        if (inaccurate) {
                /* Random tweak. */
                shotVector.vx+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                shotVector.vy+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                shotVector.vz+=((FastRandom()%(ONE_FIXED>>2))-(ONE_FIXED>>3));
                /* Normalise. */
                Normalise(&shotVector);
        }


        FindPolygonInLineOfSight(&shotVector,muzzlepos,0,self);

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
                HandleSpearImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AmmoID,&shotVector, multiple, LOS_HModel_Section);
        }

}

int SimpleEdgeDetectionTest(STRATEGYBLOCK *sbPtr, COLLISIONREPORT *vcr) {

	VECTORCH alpha,beta,tvec;

	GetTargetingPointOfObject_Far(sbPtr,&alpha);
	/* Now add half a metre in positive z. */
	
	tvec.vx=sbPtr->DynPtr->OrientMat.mat31;
	tvec.vy=sbPtr->DynPtr->OrientMat.mat32;
	tvec.vz=sbPtr->DynPtr->OrientMat.mat33;

	tvec.vx=MUL_FIXED(tvec.vx,1000);
	tvec.vy=MUL_FIXED(tvec.vy,1000);
	tvec.vz=MUL_FIXED(tvec.vz,1000);

	alpha.vx+=tvec.vx;
	alpha.vy+=tvec.vy;
	alpha.vz+=tvec.vz;

	beta.vx=sbPtr->DynPtr->OrientMat.mat21;
	beta.vy=sbPtr->DynPtr->OrientMat.mat22;
	beta.vz=sbPtr->DynPtr->OrientMat.mat23;
	Normalise(&beta);

	/* Now do an LOS test. */
	FindPolygonInLineOfSight(&beta,&alpha,0,sbPtr->SBdptr);

	/* Pass the test if the test hit something within a metre (y) of dynPtr->Position. */

	if (LOS_ObjectHitPtr) {
		VECTORCH offset;
		int dot;
		/* Examine LOS_Point. */
		offset.vx=LOS_Point.vx-sbPtr->DynPtr->Position.vx;
		offset.vy=LOS_Point.vy-sbPtr->DynPtr->Position.vy;
		offset.vz=LOS_Point.vz-sbPtr->DynPtr->Position.vz;

		dot=DotProduct(&offset,&beta);

		//ReleasePrintDebuggingText("Dot %d\n",dot);

		if ((dot>-1000)&&(dot<1000)) {
			return(0);
		}
	}

	/* Must be about to hit a rail or an edge? */
	vcr->ObstacleSBPtr=NULL;
	vcr->ObstacleNormal.vx=-sbPtr->DynPtr->OrientMat.mat31;
	vcr->ObstacleNormal.vy=-sbPtr->DynPtr->OrientMat.mat32;
	vcr->ObstacleNormal.vz=-sbPtr->DynPtr->OrientMat.mat33;
	vcr->ObstaclePoint=LOS_Point;
	vcr->NextCollisionReportPtr=NULL;

	return(1);

}
