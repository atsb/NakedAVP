/* KJL 12:42:43 29/12/98 - this is effectively Dynamics V4.0. If you think this code looks bad,
you should have seen the previous versions. */

#include "3dc.h"
#include "module.h"					  
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"
#include "dynamics.h"					

#include "bh_types.h"
#include "comp_shp.h"
#include "avpview.h"										
#include "pvisible.h"
#include "extents.h"

#include "bh_marin.h"
#include "bh_pred.h"
#include "bh_alien.h"
#include "showcmds.h"
#include "pldghost.h"
#include "weapons.h"

#include "bh_track.h"
#include "bh_fan.h"
#include "detaillevels.h"
#include "dxlog.h"
#include "avp_userprofile.h"
#include "pfarlocs.h"
#include "particle.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#define TELEPORT_IF_OUTSIDE_ENV 1	   
#define MINIMUM_BOUNDINGBOX_EXTENT 25


#define FLOOR_THRESHOLD 30000
#define NEARLYFLATFLOOR_THRESHOLD 60000

#if 0
	extern int GlobalFrameCounter;
	#define LogInfo LOGDXFMT
#else
	#define LogInfo(args) (void)0
#endif

extern MORPHDISPLAY MorphDisplay;
extern VECTORCH MorphedPts[];
extern VECTORCH *ShapePointsPtr;
extern int *ShapeNormalsPtr;
extern int *Shape2NormalsPtr;
extern char ShapeIsMorphed;
extern int **ItemArrayPtr;
extern POLYHEADER *PolyheaderPtr;
extern DAMAGE_PROFILE FlechetteDamage;
extern DAMAGE_PROFILE FallingDamage;
extern DAMAGE_PROFILE PredPistol_FlechetteDamage;

#define AccessNextPolygon()\
{\
	int *itemPtr = *(ItemArrayPtr++);\
	PolyheaderPtr = (POLYHEADER *) itemPtr;\
}											 

#define GetPolygonVertices(polyPtr)\
{\
	int *vertexNumberPtr = &PolyheaderPtr->Poly1stPt;\
\
  	(polyPtr)->PolyPoint[0] = *(ShapePointsPtr + *vertexNumberPtr++);\
    (polyPtr)->PolyPoint[1] = *(ShapePointsPtr + *vertexNumberPtr++);\
    (polyPtr)->PolyPoint[2] = *(ShapePointsPtr + *vertexNumberPtr++);\
    \
	if (*vertexNumberPtr != Term)\
	{\
	    (polyPtr)->PolyPoint[3] = *(ShapePointsPtr + *vertexNumberPtr);\
	   	(polyPtr)->NumberOfVertices=4; \
	}\
	else\
	{\
	   	(polyPtr)->NumberOfVertices=3; \
	}\
}
#define GetPolygonNormal(polyPtr)\
{	  \
	if (ShapeIsMorphed)\
	{\
		VECTORCH n1Ptr = *(VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex);\
		VECTORCH n2Ptr = *(VECTORCH*)(Shape2NormalsPtr + PolyheaderPtr->PolyNormalIndex);\
        \
		if( ((n1Ptr.vx == n2Ptr.vx)\
		  && (n1Ptr.vy == n2Ptr.vy)\
		  && (n1Ptr.vz == n2Ptr.vz))\
		  || (MorphDisplay.md_lerp == 0) )\
		{\
			(polyPtr)->PolyNormal = n1Ptr;\
		}\
		else if(MorphDisplay.md_lerp == 0xffff)\
		{\
			(polyPtr)->PolyNormal = n2Ptr;\
		}\
		else\
		{\
			VECTORCH *pointPtr[3];\
 			int *vertexNumPtr = &PolyheaderPtr->Poly1stPt;\
\
			pointPtr[0] = (ShapePointsPtr + *vertexNumPtr++);\
			pointPtr[1] = (ShapePointsPtr + *vertexNumPtr++);\
			pointPtr[2] = (ShapePointsPtr + *vertexNumPtr);\
\
			MakeNormal\
			(\
				pointPtr[0],\
				pointPtr[1],\
				pointPtr[2],\
				&(polyPtr)->PolyNormal\
			);\
		}\
	}\
    else /* not morphed */\
    {\
     	(polyPtr)->PolyNormal = *(VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex);\
    }\
}	
#define PolygonFlag (PolyheaderPtr->PolyFlags)

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void ObjectDynamics(void);

static void InitialiseDynamicObjectsList(void);
static void UpdateDisplayBlockData(STRATEGYBLOCK *sbPtr);

static void ApplyGravity(DYNAMICSBLOCK *dynPtr);
static void AlignObjectToGravityDirection(DYNAMICSBLOCK *dynPtr);
static void AlignObjectToStandardGravityDirection(DYNAMICSBLOCK *dynPtr);
static void VectorHomingForSurfaceAlign(VECTORCH *currentPtr, VECTORCH *targetPtr, VECTORCH *perpendicularPtr);

extern void DynamicallyRotateObject(DYNAMICSBLOCK *dynPtr);

static void FindLandscapePolygonsInObjectsPath(STRATEGYBLOCK *sbPtr);
static void FindObjectsToRelocateAgainst(STRATEGYBLOCK *sbPtr);
static void FindObjectPolygonsInObjectsPath(STRATEGYBLOCK *sbPtr);

static void MakeDynamicBoundingBoxForObject(STRATEGYBLOCK *sbPtr, VECTORCH *worldOffsetPtr);
static void TestShapeWithDynamicBoundingBox(DISPLAYBLOCK *objectPtr, DYNAMICSBLOCK *mainDynPtr);
static void TestObjectWithStaticBoundingBox(DISPLAYBLOCK *objectPtr);
static void TestShapeWithParticlesDynamicBoundingBox(DISPLAYBLOCK *objectPtr);


static void CreateSphereBBForObject(const STRATEGYBLOCK *sbPtr);
static signed int DistanceMovedBeforeSphereHitsPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static int SphereProjectOntoPoly(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, VECTORCH *projectedPosition);
static void MakeStaticBoundingBoxForSphere(STRATEGYBLOCK *sbPtr);
static int RelocateSphere(STRATEGYBLOCK *sbPtr);


static void CreateNRBBForObject(const STRATEGYBLOCK *sbPtr);
static signed int DistanceMovedBeforeNRBBHitsPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static int NRBBProjectsOntoPolygon(DYNAMICSBLOCK *dynPtr, int vertexToPlaneDist[], struct ColPolyTag *polyPtr, VECTORCH *projectionDirPtr);
static void MakeStaticBoundingBoxForNRBB(STRATEGYBLOCK *sbPtr);
static int RelocateNRBB(STRATEGYBLOCK *sbPtr);

static void FindLandscapePolygonsInObjectsVicinity(STRATEGYBLOCK *sbPtr);
#if 0
static signed int DistanceMovedBeforeNRBBHitsNegYPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static signed int DistanceMovedBeforeNRBBHitsPosYPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static signed int DistanceMovedBeforeNRBBHitsNegXPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static signed int DistanceMovedBeforeNRBBHitsPosXPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static signed int DistanceMovedBeforeNRBBHitsNegZPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static signed int DistanceMovedBeforeNRBBHitsPosZPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static void TestForValidMovement(STRATEGYBLOCK *sbPtr);
#endif
static int MoveObject(STRATEGYBLOCK *sbPtr);
static void TestForValidPlayerStandUp(STRATEGYBLOCK *sbPtr);
static int SteppingUpIsValid(STRATEGYBLOCK *sbPtr);
static void TestShapeWithStaticBoundingBox(DISPLAYBLOCK *objectPtr);
static int IsPolygonWithinDynamicBoundingBox(const struct ColPolyTag *polyPtr);
static int IsPolygonWithinStaticBoundingBox(const struct ColPolyTag *polyPtr);
static int WhichNRBBVertex(DYNAMICSBLOCK *dynPtr, VECTORCH *normalPtr);
static int DoesPolygonIntersectNRBB(struct ColPolyTag *polyPtr,VECTORCH *objectVertices);


static signed int (*DistanceMovedBeforeObjectHitsPolygon)(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove);
static void (*MakeStaticBoundingBoxForObject)(STRATEGYBLOCK *sbPtr);
static int (*RelocationIsValid)(STRATEGYBLOCK *sbPtr);

static void MovePlatformLift(STRATEGYBLOCK *sbPtr);
static void FindLandscapePolygonsInParticlesPath(PARTICLE *particlePtr, VECTORCH *displacementPtr);

VECTORCH *GetNearestModuleTeleportPoint(MODULE* thisModulePtr, VECTORCH* positionPtr);

/*KJL****************************************************************************************
* 										D E F I N E S 										*
****************************************************************************************KJL*/

#define	AddVectorToVector(v2,v1)\
{				  				\
	v1.vx += v2.vx;				\
	v1.vy += v2.vy;				\
	v1.vz += v2.vz;				\
}
#define	SubVectorFromVector(v2,v1)\
{				  				\
	v1.vx -= v2.vx;				\
	v1.vy -= v2.vy;				\
	v1.vz -= v2.vz;				\
}

#define AddScaledVectorToVector(v2,s,v1)	\
{											\
	v1.vx += MUL_FIXED(v2.vx, s);			\
	v1.vy += MUL_FIXED(v2.vy, s);			\
	v1.vz += MUL_FIXED(v2.vz, s);			\
}
#define SubScaledVectorFromVector(v2,s,v1)	\
{											\
	v1.vx -= MUL_FIXED(v2.vx, s);			\
	v1.vy -= MUL_FIXED(v2.vy, s);			\
 	v1.vz -= MUL_FIXED(v2.vz, s);			\
}



/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
extern int NumActiveStBlocks;
extern STRATEGYBLOCK *ActiveStBlockList[maxstblocks];
extern int NormalFrameTime;


#define COLLISION_GRANULARITY 10 // 40
#define RELOCATION_GRANULARITY 5 // 30
#define GRAVITY_DISPLACEMENT (COLLISION_GRANULARITY+1)
#define MAXIMUM_NUMBER_OF_COLLISIONPOLYS 3000
#define PLAYER_PICKUP_OBJECT_RADIUS 1600


static STRATEGYBLOCK *DynamicObjectsList[MAX_NO_OF_DYNAMICS_BLOCKS];
static int NumberOfDynamicObjects = 1;

static int AccelDueToGravity;
static int DistanceToStepUp;
static VECTORCH DirectionOfTravel;

static struct ColPolyTag CollisionPolysArray[MAXIMUM_NUMBER_OF_COLLISIONPOLYS];
static struct ColPolyTag *CollisionPolysPtr;
static int NumberOfCollisionPolys;

#define MAX_NUMBER_OF_INTERFERENCE_POLYGONS 100
static struct ColPolyTag InterferencePolygons[MAX_NUMBER_OF_INTERFERENCE_POLYGONS];
static int NumberOfInterferencePolygons = 0;

/* global storage of bounding box */
static int DBBMinX,DBBMaxX, DBBMinY,DBBMaxY, DBBMinZ,DBBMaxZ;
static int SBBMinX,SBBMaxX, SBBMinY,SBBMaxY, SBBMinZ,SBBMaxZ;

const static int CuboidVertexList[]={0,1,5,4, 0,4,6,2, 0,2,3,1, 1,3,7,5, 4,5,7,6, 3,2,6,7};
int PlanarGravity=1;

static int PlayersFallingSpeed;
int PlayersMaxHeightWhilstNotInContactWithGround;
/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/

/* Entry point to dynamics system - this function handles the movement of all objects */
extern void ObjectDynamics(void)
{
	int i;
 /*	textprint("player Impulse at %d,%d,%d\n",
	Player->ObStrategyBlock->DynPtr->LinImpulse.vx,
	Player->ObStrategyBlock->DynPtr->LinImpulse.vy,
	Player->ObStrategyBlock->DynPtr->LinImpulse.vz);
 */	
//	if (TICKERTAPE_CHEATMODE)
//		PlayerPheromoneTrail();

	if (FREEFALL_CHEATMODE)
	{
		PlanarGravity = 0;
	}
	else
	{
		PlanarGravity = 1;
	}
	/* clear previous frame's collision reports */
	InitialiseCollisionReports();

	/* create ordered list of dynamic objects */
	InitialiseDynamicObjectsList();

	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		LogInfo
		((
			"Dynamics Logging: frame %d\nDL: player's Position %d,%d,%d\nDL: player's Displacement %d,%d,%d\nDL: NormalFrameTime %d\n",
			GlobalFrameCounter,
			dynPtr->Position.vx,dynPtr->Position.vy,dynPtr->Position.vz,
			dynPtr->Displacement.vx,dynPtr->Displacement.vy,dynPtr->Displacement.vz,
			NormalFrameTime
		));
	
	}
	
	i = NumberOfDynamicObjects;
	/* scan through objects */
	while(i--)
	{
		STRATEGYBLOCK *sbPtr = DynamicObjectsList[i];
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

		GLOBALASSERT(dynPtr->Mass>0);
		      	   
		if (dynPtr->IsNetGhost || (dynPtr->IsPickupObject && !dynPtr->GravityOn))
		{
			#if 0
			dynPtr->Position.vx += MUL_FIXED(dynPtr->LinVelocity.vx, NormalFrameTime);
		    dynPtr->Position.vy += MUL_FIXED(dynPtr->LinVelocity.vy, NormalFrameTime);
		    dynPtr->Position.vz += MUL_FIXED(dynPtr->LinVelocity.vz, NormalFrameTime);
			AlignObjectToStandardGravityDirection(dynPtr);
			#endif
			UpdateDisplayBlockData(sbPtr);
			continue;
		}
		/* setup function pointers */
		switch(dynPtr->DynamicsType)
		{
			case DYN_TYPE_SPHERE_COLLISIONS:
			{
				DistanceMovedBeforeObjectHitsPolygon = DistanceMovedBeforeSphereHitsPolygon;
				MakeStaticBoundingBoxForObject = MakeStaticBoundingBoxForSphere;
				RelocationIsValid = RelocateSphere;
				break;
			}
			case DYN_TYPE_NRBB_COLLISIONS:
			{
				DistanceMovedBeforeObjectHitsPolygon = DistanceMovedBeforeNRBBHitsPolygon;
				MakeStaticBoundingBoxForObject = MakeStaticBoundingBoxForNRBB;
				RelocationIsValid = RelocateNRBB;
				break;
			}
			default:
			{
				/* oh dear, invalid collision shape */
				GLOBALASSERT(0);
				break;
			}
		}
		if ((sbPtr->SBdptr == Player) && dynPtr->RequestsToStandUp)
			TestForValidPlayerStandUp(sbPtr); 
#if 0
		if (dynPtr->OnlyCollideWithObjects) 
		{
			/* initialise near polygons array */	
			CollisionPolysPtr = &CollisionPolysArray[0];
		    NumberOfCollisionPolys=0;
		}
		else
		{
			/* find which landscape polygons occupy the space
		   	through which the object wishes to move */
			FindLandscapePolygonsInObjectsPath(sbPtr);
		}
#endif
		if (dynPtr->OnlyCollideWithObjects)
		{
			/* initialise near polygons array */	
			CollisionPolysPtr = &CollisionPolysArray[0];
			NumberOfCollisionPolys=0;
			MovePlatformLift(sbPtr);
		}
		else if (dynPtr->StopOnCollision)
	  	{
			if (dynPtr->OnlyCollideWithObjects) 
			{
				/* initialise near polygons array */	
				CollisionPolysPtr = &CollisionPolysArray[0];
			    NumberOfCollisionPolys=0;
				FindObjectPolygonsInObjectsPath(sbPtr);
			}
			else
			{
				/* find which landscape polygons occupy the space
			   	through which the object wishes to move */
				FindLandscapePolygonsInObjectsPath(sbPtr);
				if (!dynPtr->OnlyCollideWithEnvironment)
				{
					FindObjectPolygonsInObjectsPath(sbPtr);
				}
			}
			while(dynPtr->DistanceLeftToMove && !MoveObject(sbPtr));
		}
		else
		{
		  	int noOfMoves=4;
			int maxMoveLimit=10;
	   	  	
	   	  	if (dynPtr->OnlyCollideWithObjects) 
			{
				/* initialise near polygons array */	
				CollisionPolysPtr = &CollisionPolysArray[0];
			    NumberOfCollisionPolys=0;
				FindObjectPolygonsInObjectsPath(sbPtr);
			}
			else
			{
				/* find which landscape polygons occupy the space
			   	through which the object wishes to move */
				FindLandscapePolygonsInObjectsPath(sbPtr);
				if (!dynPtr->OnlyCollideWithEnvironment)
				{
					FindObjectPolygonsInObjectsPath(sbPtr);
				}
			}
			
			while (dynPtr->DistanceLeftToMove && noOfMoves && maxMoveLimit)
		 	{	
				int hitSomethingWhileMoving;

				if (ShowDebuggingText.Dynamics) PrintDebuggingText("Displacement:%d,%d,%d\n",
				dynPtr->Displacement.vx,
				dynPtr->Displacement.vy,
				dynPtr->Displacement.vz);

				hitSomethingWhileMoving = MoveObject(sbPtr);

				if(hitSomethingWhileMoving||DistanceToStepUp)
				{
					if (dynPtr->OnlyCollideWithObjects) 
					{
						/* initialise near polygons array */	
						CollisionPolysPtr = &CollisionPolysArray[0];
					    NumberOfCollisionPolys=0;
						FindObjectPolygonsInObjectsPath(sbPtr);
					}
					else
					{
						/* find which landscape polygons occupy the space
					   	through which the object wishes to move */
						FindLandscapePolygonsInObjectsPath(sbPtr);
						if (!dynPtr->OnlyCollideWithEnvironment)
						{
							FindObjectPolygonsInObjectsPath(sbPtr);
						}
					}
				}
				if(hitSomethingWhileMoving&&!DistanceToStepUp)
				{
		     		noOfMoves--;
				}

				maxMoveLimit--;
			}
		}
			
		/* friction */
		#if 0
		if (dynPtr->IsInContactWithFloor)
		{
			int scale = NormalFrameTime<<1;
			if(scale>ONE_FIXED) scale = ONE_FIXED;
			scale = ONE_FIXED;
	   		dynPtr->LinImpulse.vx -= MUL_FIXED(scale,dynPtr->LinImpulse.vx);
	   		dynPtr->LinImpulse.vz -= MUL_FIXED(scale,dynPtr->LinImpulse.vz);
		}
		#else
		if (dynPtr->IsInContactWithFloor)
		{
			int k = NormalFrameTime<<1;
			int dotted = DotProduct(&(dynPtr->LinImpulse),&(dynPtr->GravityDirection));

			VECTORCH linParallel,linPerp;

			linParallel.vx = MUL_FIXED(dotted,dynPtr->GravityDirection.vx);
			linParallel.vy = MUL_FIXED(dotted,dynPtr->GravityDirection.vy);
			linParallel.vz = MUL_FIXED(dotted,dynPtr->GravityDirection.vz);

			linPerp.vx = dynPtr->LinImpulse.vx - linParallel.vx;
			linPerp.vy = dynPtr->LinImpulse.vy - linParallel.vy;
			linPerp.vz = dynPtr->LinImpulse.vz - linParallel.vz;
			
			if (dynPtr->IsInContactWithNearlyFlatFloor)
			{
				if (Approximate3dMagnitude(&linPerp)<3000)
				{
					k*=16;
					if (k>ONE_FIXED) k = ONE_FIXED;
				}
			}

	   		dynPtr->LinImpulse.vx -= MUL_FIXED(k,linPerp.vx);
	   		dynPtr->LinImpulse.vy -= MUL_FIXED(k,linPerp.vy);
	   		dynPtr->LinImpulse.vz -= MUL_FIXED(k,linPerp.vz);
		}
		#endif

		#if 0
		if( (dynPtr->Position.vx != dynPtr->PrevPosition.vx)
		  ||(dynPtr->Position.vy != dynPtr->PrevPosition.vy)
		  ||(dynPtr->Position.vz != dynPtr->PrevPosition.vz))
		#endif
		{				
// 	 		FindObjectsToRelocateAgainst(sbPtr);					
//			TestForValidMovement(sbPtr);
		}
  //		RelocatedDueToFallout(dynPtr);
		UpdateDisplayBlockData(sbPtr);
	}
	#if TELEPORT_IF_OUTSIDE_ENV
	{
		extern MODULE *playerPherModule;
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		MODULE *newModule = (ModuleFromPosition(&(dynPtr->Position), playerPherModule));
		
		if (!newModule)
		{
			/* hmm, player isn't in a module */
			#if 0
			if (playerPherModule)
			{
				dynPtr->Position.vx = playerPherModule->m_world.vx;
				dynPtr->Position.vy = playerPherModule->m_world.vy;
				dynPtr->Position.vz = playerPherModule->m_world.vz;
				PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
				
				dynPtr->PrevPosition = dynPtr->Position;
				dynPtr->LinImpulse.vx = 0;
				dynPtr->LinImpulse.vy = 0;
				dynPtr->LinImpulse.vz = 0;
				UpdateDisplayBlockData(Player->ObStrategyBlock);
			}
			else
			#endif
			{
				if (playerPherModule)
				{
					VECTORCH newPosition = playerPherModule->m_aimodule->m_world;
					VECTORCH *offsetPtr = GetNearestModuleTeleportPoint(playerPherModule, &dynPtr->Position);
					if (offsetPtr)
					{
						newPosition.vx += offsetPtr->vx;
						newPosition.vy += offsetPtr->vy;
						newPosition.vz += offsetPtr->vz;
					}
					
					dynPtr->Position = newPosition;
					dynPtr->PrevPosition = newPosition;
				}
				PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
				dynPtr->LinImpulse.vx = 0;
				dynPtr->LinImpulse.vy = 0;
				dynPtr->LinImpulse.vz = 0;
				UpdateDisplayBlockData(Player->ObStrategyBlock);
	   			//NewOnScreenMessage("Relocated Player");
			}

		}
	}
	#endif
	/* KJL 18:50:17 10/11/98 - Falling Damage */
	if (AvP.PlayerType==I_Marine)
	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		if(dynPtr->IsInContactWithFloor)
		{
			#if 0
			int damage = (PlayersFallingSpeed-15000)*160;
			if (damage>0)
			{
				CauseDamageToObject(Player->ObStrategyBlock,&FallingDamage,damage,NULL);
			
			}
			//falling damage may be turned off in network games
			BOOL fallingDamageDisabled = (!netGameData.fallingDamage && AvP.Network!=I_No_Network);
			int damage = ((dynPtr->Position.vy - PlayersMaxHeightWhilstNotInContactWithGround - 4000))*256
			;
			if (damage>0 && !fallingDamageDisabled)
			{
				CauseDamageToObject(Player->ObStrategyBlock,&FallingDamage,damage,NULL);
			}
			/* CDF 8/4/99 - end of jump sound... */
			{
				int distanceFallen = (dynPtr->Position.vy - PlayersMaxHeightWhilstNotInContactWithGround);

				if ((distanceFallen>500)&&(distanceFallen<4000 || fallingDamageDisabled)) {
					/* Make a sound. */
	   				Sound_Play(SID_MARINE_SMALLLANDING,"h");
					if(AvP.Network!=I_No_Network) netGameData.landingNoise=1;
				}
			}
			#endif
			BOOL fallingDamageDisabled = (!netGameData.fallingDamage && AvP.Network!=I_No_Network);
			int damage = (PlayersFallingSpeed-15000)*256;
			int distanceFallen = (dynPtr->Position.vy - PlayersMaxHeightWhilstNotInContactWithGround);

			if (distanceFallen>5000 && damage>ONE_FIXED && !fallingDamageDisabled)
			{
				CauseDamageToObject(Player->ObStrategyBlock,&FallingDamage,damage,NULL);
			}
			else if (distanceFallen>1000)
			{
	   			Sound_Play(SID_MARINE_SMALLLANDING,"h");
				if(AvP.Network!=I_No_Network) netGameData.landingNoise=1;
			}

			

			PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
		}
		else
		{
			if(dynPtr->LinImpulse.vy < 0)
			{
				PlayersMaxHeightWhilstNotInContactWithGround = 1000000;
			}
			else if(PlayersMaxHeightWhilstNotInContactWithGround>dynPtr->Position.vy)
			{
				PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
			}
		}
	} else if (AvP.PlayerType==I_Predator) {
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;

		if(dynPtr->IsInContactWithFloor)
		{
			/* CDF 8/4/99 - end of jump sound... */
			{
				int distanceFallen = (dynPtr->Position.vy - PlayersMaxHeightWhilstNotInContactWithGround);

				if (distanceFallen>1000) {
					/* Make a sound. */
	   				Sound_Play(SID_PRED_SMALLLANDING,"h");
					if(AvP.Network!=I_No_Network) netGameData.landingNoise=1;
				}
			}
			PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
		}
		else
		{
			if(dynPtr->LinImpulse.vy < 0)
			{
				PlayersMaxHeightWhilstNotInContactWithGround = 1000000;
			}
			else if(PlayersMaxHeightWhilstNotInContactWithGround>dynPtr->Position.vy)
			{
				PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
			}
		}
	}
	/* Check for object pickup */
	{
		int i = NumberOfDynamicObjects;
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
	    
	    while(i--)    
		{
			STRATEGYBLOCK *obstaclePtr = DynamicObjectsList[i];
//		  	if((obstaclePtr->I_SBtype == I_BehaviourHierarchicalFragment)||(obstaclePtr->DynPtr->IsPickupObject))
		  	if(obstaclePtr->DynPtr->IsPickupObject)
			{
				VECTORCH disp;
				disp.vx = dynPtr->Position.vx-obstaclePtr->DynPtr->Position.vx;
				disp.vy = dynPtr->Position.vy-obstaclePtr->DynPtr->Position.vy;
				disp.vz = dynPtr->Position.vz-obstaclePtr->DynPtr->Position.vz;
				if (Approximate3dMagnitude(&disp)<PLAYER_PICKUP_OBJECT_RADIUS)
				{
					/* create a report about the collision */
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = obstaclePtr;
						reportPtr->ObstacleNormal.vx = -dynPtr->GravityDirection.vx;
						reportPtr->ObstacleNormal.vy = -dynPtr->GravityDirection.vy;
						reportPtr->ObstacleNormal.vz = -dynPtr->GravityDirection.vz;
					}
				}
			}
		}
	}
	#if 0
	{
		COLLISIONREPORT *reportPtr = Player->ObStrategyBlock->DynPtr->CollisionReportPtr;

		if (ShowDebuggingText.Dynamics) PrintDebuggingText("Player Impulse:%d,%d,%d\n",
		Player->ObStrategyBlock->DynPtr->LinImpulse.vx,
		Player->ObStrategyBlock->DynPtr->LinImpulse.vy,
		Player->ObStrategyBlock->DynPtr->LinImpulse.vz);

		if (ShowDebuggingText.Dynamics) PrintDebuggingText("Player Position:%d,%d,%d\n",
		Player->ObStrategyBlock->DynPtr->Position.vx,
		Player->ObStrategyBlock->DynPtr->Position.vy,
		Player->ObStrategyBlock->DynPtr->Position.vz);

		if (ShowDebuggingText.Dynamics) PrintDebuggingText("InContactWithFloor %d\n",Player->ObStrategyBlock->DynPtr->IsInContactWithFloor);
		if (ShowDebuggingText.Dynamics) PrintDebuggingText("Player Gravity Direction:%d,%d,%d\n",
			Player->ObStrategyBlock->DynPtr->GravityDirection.vx,
			Player->ObStrategyBlock->DynPtr->GravityDirection.vy,
			Player->ObStrategyBlock->DynPtr->GravityDirection.vz);

		while (reportPtr) /* while there is a valid report */
		{
			if (ShowDebuggingText.Dynamics) PrintDebuggingText("Col Normal %d %d %d\n",reportPtr->ObstacleNormal.vx,reportPtr->ObstacleNormal.vy,reportPtr->ObstacleNormal.vz);
			if (ShowDebuggingText.Dynamics) PrintDebuggingText("strategy ptr %p\n",reportPtr->ObstacleSBPtr);
							 
			/* skip to next report */
			reportPtr = reportPtr->NextCollisionReportPtr;
		}
		PrintDebuggingText("€‚ƒ ©¸ä\n");
		if(!Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)
			NewOnScreenMessage("€‚ƒ word ©¸ä word €‚ƒ word ¸ä word\n");
	}
	#endif
	//NewTrailPoint(Player->ObStrategyBlock->DynPtr);
}

static void InitialiseDynamicObjectsList(void)
{
	STRATEGYBLOCK *unsortedDynamicObjectsList[MAX_NO_OF_DYNAMICS_BLOCKS];
	signed int valueOnWhichToSort[MAX_NO_OF_DYNAMICS_BLOCKS];
	AccelDueToGravity = MUL_FIXED(GRAVITY_STRENGTH,NormalFrameTime);

	if (UNDERWATER_CHEATMODE)
	{
		AccelDueToGravity/=2;
	}
	
	/* scan through list of strategy blocks looking for ones
	   with dynamics blocks and collisions on  */
	{
		int i = NumActiveStBlocks;
		NumberOfDynamicObjects = 0;
		while(i)
		{
			STRATEGYBLOCK *sbPtr = ActiveStBlockList[--i];
			DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;

			if (dynPtr && dispPtr)
			{
				/* skip static objects */
				if(dynPtr->IsStatic)
				{
					UpdateDisplayBlockData(sbPtr);
// 		 			CreateNRBBForObject(sbPtr);
				}
				/* should object simply move? */
  				else if (dynPtr->DynamicsType == DYN_TYPE_NO_COLLISIONS)
				{
					/* Apply gravity */
		   			ApplyGravity(dynPtr);

					dynPtr->Position.vx += 
						MUL_FIXED(dynPtr->LinVelocity.vx+dynPtr->LinImpulse.vx, NormalFrameTime);
				    dynPtr->Position.vy += 
				    	MUL_FIXED(dynPtr->LinVelocity.vy+dynPtr->LinImpulse.vy, NormalFrameTime);
				    dynPtr->Position.vz += 
     			    	MUL_FIXED(dynPtr->LinVelocity.vz+dynPtr->LinImpulse.vz, NormalFrameTime);
					UpdateDisplayBlockData(sbPtr);
				}
				/* is it just static? */
				else /* have to consider it properly */
				{
					/* Apply gravity */
		   	  		ApplyGravity(dynPtr);
					AddEffectsOfForceGenerators(&dynPtr->Position,&dynPtr->LinImpulse,dynPtr->Mass);
					/* create a bb that surrounds the object */
		 //			CreateExtentCuboidForObject(sbPtr); 
		 			switch(dynPtr->DynamicsType)
					{
						case DYN_TYPE_SPHERE_COLLISIONS:
						{
				 			dynPtr->CollisionRadius = 500;
			 			
				 			CreateSphereBBForObject(sbPtr);
							break;
						}
						case DYN_TYPE_NRBB_COLLISIONS:
						{
						#if 0
							textprint
							(
								"%d %d, %d %d, %d %d\n"
								,sbPtr->SBdptr->ObMaxX,sbPtr->SBdptr->ObMinX
								,sbPtr->SBdptr->ObMaxY,sbPtr->SBdptr->ObMinY
								,sbPtr->SBdptr->ObMaxZ,sbPtr->SBdptr->ObMinZ
							);
						#endif
				 			CreateNRBBForObject(sbPtr);
							break;
						}
						default:
						{
							/* oh dear, invalid collision shape */
							GLOBALASSERT(0);
							break;
						}
					}

					if (!dynPtr->IsNetGhost)
					{
						/* set previous position datum */
						dynPtr->PrevPosition = dynPtr->Position;
						//dynPtr->PrevOrientMat = dynPtr->OrientMat;

						/* reset floor contact flag */
						dynPtr->IsInContactWithFloor = 0;
						dynPtr->IsInContactWithNearlyFlatFloor = 0;

						/* calculate object's movement vector */
						if (!dynPtr->UseDisplacement)
						{
							dynPtr->Displacement.vx = 0;
							dynPtr->Displacement.vy = 0;
							dynPtr->Displacement.vz = 0;
						}


						if (dynPtr->OnlyCollideWithObjects)
						{
							dynPtr->Displacement.vx += MUL_FIXED(dynPtr->LinVelocity.vx, NormalFrameTime);
						    dynPtr->Displacement.vy += MUL_FIXED(dynPtr->LinVelocity.vy, NormalFrameTime);
						    dynPtr->Displacement.vz += MUL_FIXED(dynPtr->LinVelocity.vz, NormalFrameTime);
						}    
						else
						{
							dynPtr->Displacement.vx += MUL_FIXED(dynPtr->LinVelocity.vx+dynPtr->LinImpulse.vx, NormalFrameTime);
						    dynPtr->Displacement.vy += MUL_FIXED(dynPtr->LinVelocity.vy+dynPtr->LinImpulse.vy, NormalFrameTime);
						    dynPtr->Displacement.vz += MUL_FIXED(dynPtr->LinVelocity.vz+dynPtr->LinImpulse.vz, NormalFrameTime);

							/* If moving in direction of gravity, add a little bit to make sure you will make contact
							with the ground if your close */
							if (dynPtr->GravityOn)
							{
								if (dynPtr->UseStandardGravity&&PlanarGravity)	/* ie. in direction of +ve Y-axis */
								{
									if (dynPtr->Displacement.vy > 0)
										dynPtr->Displacement.vy += GRAVITY_DISPLACEMENT;
								}
								else 
								{
									if (DotProduct(&(dynPtr->Displacement),&(dynPtr->GravityDirection)) > 0)
										dynPtr->Displacement.vx += MUL_FIXED(dynPtr->GravityDirection.vx, GRAVITY_DISPLACEMENT);
									    dynPtr->Displacement.vy += MUL_FIXED(dynPtr->GravityDirection.vy, GRAVITY_DISPLACEMENT);
									    dynPtr->Displacement.vz += MUL_FIXED(dynPtr->GravityDirection.vz, GRAVITY_DISPLACEMENT);
								}
							}
							
							/* KJL 12:00:29 25/11/98 - resolve against last frames normals */
							#if 0
							{
								COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
								while (reportPtr)
								{
									int magOfPerpVel;
//									if (! ((reportPtr->ObstacleNormal.vx < 100 && reportPtr->ObstacleNormal.vx > -100)
//   										 &&(reportPtr->ObstacleNormal.vz < 100 && reportPtr->ObstacleNormal.vz > -100) ))
									if(reportPtr->ObstaclePoint.vx == 0x7fffffff &&
									   reportPtr->ObstaclePoint.vy == 0x7fffffff &&
									   reportPtr->ObstaclePoint.vz == 0x7fffffff)
									{

//										reportPtr->ObstacleNormal.vy = 0;
//										Normalise(&reportPtr->ObstacleNormal);
										magOfPerpVel = MUL_FIXED(66000,DotProduct(&reportPtr->ObstacleNormal,&(dynPtr->Displacement)));

	//									SubScaledVectorFromVector(reportPtr->ObstacleNormal, magOfPerpVel, (dynPtr->Displacement));
										dynPtr->Displacement.vx -= MUL_FIXED(reportPtr->ObstacleNormal.vx,magOfPerpVel);
										dynPtr->Displacement.vy -= MUL_FIXED(reportPtr->ObstacleNormal.vy,magOfPerpVel);
										dynPtr->Displacement.vz -= MUL_FIXED(reportPtr->ObstacleNormal.vz,magOfPerpVel);
									}
									reportPtr = reportPtr->NextCollisionReportPtr;
								}
							}
							#endif
							
						}    

						dynPtr->DistanceLeftToMove = Magnitude(&dynPtr->Displacement);
						
						if (dynPtr->DistanceLeftToMove>ONE_FIXED/8)
						{
							dynPtr->DistanceLeftToMove = ONE_FIXED/8;
							Normalise(&dynPtr->Displacement);
							dynPtr->Displacement.vx /= 8;
							dynPtr->Displacement.vy /= 8;
							dynPtr->Displacement.vz /= 8;
						}
					}

					if(dynPtr->OnlyCollideWithObjects || dynPtr->IsNetGhost)
					{
						valueOnWhichToSort[NumberOfDynamicObjects] = 0x7fffffff;
					}
					else
					{
						valueOnWhichToSort[NumberOfDynamicObjects] = dynPtr->DistanceLeftToMove;
					}

					unsortedDynamicObjectsList[NumberOfDynamicObjects] = sbPtr;
					NumberOfDynamicObjects++;

					if (dispPtr == Player)
					{
						PlayersFallingSpeed = (dynPtr->LinVelocity.vy+dynPtr->LinImpulse.vy);
					}
				}
				
				/* wipe previous collision records */
				dynPtr->CollisionReportPtr =0;
			}
		}
	}
	/* possibly a good idea to sort objects so that the fastest is moved first */
	{
		/* extremely simple (and inefficient) selection sort */ 
		int outer = NumberOfDynamicObjects;
		while(outer > 0)
		{
			int inner = NumberOfDynamicObjects;
			int highestValueFound = -1;
			int indexOfFastestObject =0;
			while(inner)
			{
				if (valueOnWhichToSort[--inner]>highestValueFound)
				{
					highestValueFound = valueOnWhichToSort[inner];
					indexOfFastestObject = inner;
				}
			}

			DynamicObjectsList[--outer] = unsortedDynamicObjectsList[indexOfFastestObject];
			valueOnWhichToSort[indexOfFastestObject] = -1;
		}
	}
}
static void UpdateDisplayBlockData(STRATEGYBLOCK *sbPtr)
{
	/* If the object associated with this strategyblock has a valid displayblock
	   then update the data which has been changed by the objects movement. */
	DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	if (dispPtr)
	{
		dispPtr->ObWorld = dynPtr->Position;

		if (dynPtr->CollisionRadius)
		{
			int radius = dynPtr->CollisionRadius;
			dispPtr->ObWorld.vy -= radius;
			dispPtr->ObWorld.vx += MUL_FIXED(dynPtr->OrientMat.mat21,radius);
			dispPtr->ObWorld.vy += MUL_FIXED(dynPtr->OrientMat.mat22,radius);
			dispPtr->ObWorld.vz += MUL_FIXED(dynPtr->OrientMat.mat23,radius);
		}
		dispPtr->ObMat = dynPtr->OrientMat;
		dispPtr->ObEuler = dynPtr->OrientEuler;
		
		if (TICKERTAPE_CHEATMODE)
		{
			if (sbPtr)
			{	
				if (sbPtr->I_SBtype == I_BehaviourAlien)
					PlayerPheromoneTrail(dynPtr);
			}
		}
	}
	dynPtr->PrevOrientMat = dynPtr->OrientMat;
}








/* gravity code */
static void ApplyGravity(DYNAMICSBLOCK *dynPtr)
{
	if (dynPtr->GravityOn)
	{
		if (dynPtr->UseStandardGravity)
		{
			if (PlanarGravity)
			{
				/* ie. in direction of +ve Y-axis */
	   			dynPtr->LinImpulse.vy += AccelDueToGravity;	
	
				/* KJL 11:47:04 03/26/97 - aliens need to be aligned */
				if (dynPtr->ToppleForce == TOPPLE_FORCE_ALIEN)
				{
					AlignObjectToStandardGravityDirection(dynPtr);
				}
			}
			/* else if (RadialGravityModel) */
			else
			{
				extern int CloakingPhase;
				dynPtr->GravityDirection.vx = GetSin((CloakingPhase/16)&4095);
				dynPtr->GravityDirection.vy = GetCos((CloakingPhase/16)&4095);
				dynPtr->GravityDirection.vz = GetCos((CloakingPhase/19+400)&4095);
				Normalise(&(dynPtr->GravityDirection));
				AddScaledVectorToVector(dynPtr->GravityDirection,(AccelDueToGravity),dynPtr->LinImpulse);
				AlignObjectToGravityDirection(dynPtr);
			}
		}
		else 
		{
			/* KJL 11:47:04 03/26/97 - aliens need to be aligned */
			if (dynPtr->ToppleForce == TOPPLE_FORCE_ALIEN)
			{
				COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;
				VECTORCH averageNormal= {0,0,0};
				int normalsFound=0;
				
				if(dynPtr->TimeNotInContactWithFloor!=-1)
				{
					while (reportPtr) /* while there is a valid report */
					{
			//			if (reportPtr->ObstacleSBPtr == 0)
						{
							averageNormal.vx -= reportPtr->ObstacleNormal.vx;
							averageNormal.vy -= reportPtr->ObstacleNormal.vy;
							averageNormal.vz -= reportPtr->ObstacleNormal.vz;
							normalsFound++;
						}								 
						/* skip to next report */
						reportPtr = reportPtr->NextCollisionReportPtr;
					}
				}
				if (normalsFound)
				{
		  	  	  //averageNormal.vx /= normalsFound;
		  		  //averageNormal.vy /= normalsFound;
		 		  //averageNormal.vz /= normalsFound;
					if (averageNormal.vx==0 && averageNormal.vy==0 && averageNormal.vz==0)
					{
						// down boy down
						averageNormal.vy = ONE_FIXED;
					}
					else
					{
						Normalise(&averageNormal);
					}
					dynPtr->GravityDirection = averageNormal;
					dynPtr->TimeNotInContactWithFloor = TIME_BEFORE_GRAVITY_KICKS_IN;
		  		}
				else				    
				{
					if (dynPtr->TimeNotInContactWithFloor<=0)
					{
						if (PlanarGravity)
						{
							dynPtr->GravityDirection.vx = 0;
							dynPtr->GravityDirection.vy = 65536;
							dynPtr->GravityDirection.vz = 0;
						}
						/* else if (RadialGravityModel) */
						else
						{
							dynPtr->GravityDirection.vx = -dynPtr->Position.vx;
							dynPtr->GravityDirection.vy = -dynPtr->Position.vy;
							dynPtr->GravityDirection.vz = -dynPtr->Position.vz;
							Normalise(&(dynPtr->GravityDirection));
						}
					}
					else if (dynPtr->TimeNotInContactWithFloor == TIME_BEFORE_GRAVITY_KICKS_IN)
					{
						if (dynPtr->LinVelocity.vx==0 && dynPtr->LinVelocity.vy==0 && dynPtr->LinVelocity.vz==0)
						{
//							dynPtr->GravityDirection.vx = 0;
//							dynPtr->GravityDirection.vy = 65536;
//							dynPtr->GravityDirection.vz = 0;
						}
						else
						{
							/* code to enable going round 270 degree corners */
							#if 1
							Normalise(&dynPtr->LinVelocity);
							dynPtr->GravityDirection.vx -= (dynPtr->LinVelocity.vx*3)/4;
							dynPtr->GravityDirection.vy -= (dynPtr->LinVelocity.vy*3)/4;
							dynPtr->GravityDirection.vz -= (dynPtr->LinVelocity.vz*3)/4;
							Normalise(&dynPtr->GravityDirection);				 
							dynPtr->LinVelocity.vx = 0;
							dynPtr->LinVelocity.vy = 0;
							dynPtr->LinVelocity.vz = 0;
							#endif
						}
					}
					dynPtr->TimeNotInContactWithFloor-=NormalFrameTime;
					if (dynPtr->TimeNotInContactWithFloor<0)
					{
						dynPtr->TimeNotInContactWithFloor = 0;
					}

				}
				AlignObjectToGravityDirection(dynPtr);
			}
			AddScaledVectorToVector(dynPtr->GravityDirection,AccelDueToGravity,dynPtr->LinImpulse);
		}
	}
}
static void AlignObjectToGravityDirection(DYNAMICSBLOCK *dynPtr)
{
	VECTORCH XVector;
	VECTORCH YVector;
	VECTORCH ZVector;
	int staticAxis;
	
	{
		int dotx = Dot((VECTORCH*)&dynPtr->OrientMat.mat11, &dynPtr->GravityDirection);
		int doty = Dot((VECTORCH*)&dynPtr->OrientMat.mat21, &dynPtr->GravityDirection);
		int dotz = Dot((VECTORCH*)&dynPtr->OrientMat.mat31, &dynPtr->GravityDirection);
		
		/* Get their absolute values */
		if(dotx < 0) dotx = -dotx;
		if(doty < 0) doty = -doty;
		if(dotz < 0) dotz = -dotz;

		if (dotx<doty)
		{
			if (dotx<dotz)
			{
				staticAxis = ix;
			}
			else
			{
				staticAxis = iz;
			}
		}
		else
		{
			if (doty<dotz)
			{
				staticAxis = iy;
			}
			else
			{
				staticAxis = iz;
			}
		}
	}
 	if (staticAxis == iz)
	{
		ZVector = *((VECTORCH*)&dynPtr->OrientMat.mat31);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
	  	VectorHomingForSurfaceAlign(&YVector, &dynPtr->GravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat11));
		CrossProduct(&YVector, &ZVector, &XVector);
	}
 	else if (staticAxis == ix)
	{
		XVector = *((VECTORCH*)&dynPtr->OrientMat.mat11);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
		VectorHomingForSurfaceAlign(&YVector, &dynPtr->GravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat31));
		CrossProduct(&XVector, &YVector, &ZVector);
	}
	else if (staticAxis == iy)
	{
		XVector = *((VECTORCH*)&dynPtr->OrientMat.mat11);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
		VectorHomingForSurfaceAlign(&YVector, &dynPtr->GravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat31));
		CrossProduct(&XVector, &YVector, &ZVector);
		CrossProduct(&YVector, &ZVector, &XVector);
	}
	Normalise(&XVector);
	Normalise(&YVector);
	Normalise(&ZVector);

	dynPtr->OrientMat.mat11 = XVector.vx;
   	dynPtr->OrientMat.mat12 = XVector.vy;
	dynPtr->OrientMat.mat13 = XVector.vz;

	dynPtr->OrientMat.mat21 = YVector.vx;
	dynPtr->OrientMat.mat22 = YVector.vy;
	dynPtr->OrientMat.mat23 = YVector.vz;

	dynPtr->OrientMat.mat31 = ZVector.vx;
	dynPtr->OrientMat.mat32 = ZVector.vy;
	dynPtr->OrientMat.mat33 = ZVector.vz;

	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
}
static void AlignObjectToStandardGravityDirection(DYNAMICSBLOCK *dynPtr)
{
	VECTORCH gravityDirection = {0,65536,0};
	VECTORCH XVector,YVector,ZVector;
	int staticAxis;

	dynPtr->GravityDirection = gravityDirection;
	
	{
		int dotx = dynPtr->OrientMat.mat12;
		int doty = dynPtr->OrientMat.mat22;
		int dotz = dynPtr->OrientMat.mat32;
		
		/* Get their absolute values */
		if(dotx < 0) dotx = -dotx;
		if(doty < 0) doty = -doty;
		if(dotz < 0) dotz = -dotz;

		if (dotx<doty)
		{
			if (dotx<dotz)
			{
				staticAxis = ix;
			}
			else
			{
				staticAxis = iz;
			}
		}
		else
		{
			if (doty<dotz)
			{
				staticAxis = iy;
			}
			else
			{
				staticAxis = iz;
			}
		}
	}
 	if (staticAxis == iz)
	{
		ZVector = *((VECTORCH*)&dynPtr->OrientMat.mat31);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
	  	VectorHomingForSurfaceAlign(&YVector, &gravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat11));
		CrossProduct(&YVector, &ZVector, &XVector);
	}
 	else if (staticAxis == ix)
	{
		XVector = *((VECTORCH*)&dynPtr->OrientMat.mat11);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
		VectorHomingForSurfaceAlign(&YVector, &gravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat31));
		CrossProduct(&XVector, &YVector, &ZVector);
	}
	else if (staticAxis == iy)
	{
		XVector = *((VECTORCH*)&dynPtr->OrientMat.mat11);
	 	YVector = *((VECTORCH*)&dynPtr->OrientMat.mat21);
		VectorHomingForSurfaceAlign(&YVector, &gravityDirection,((VECTORCH*)&dynPtr->OrientMat.mat31));
		CrossProduct(&XVector, &YVector, &ZVector);
		CrossProduct(&YVector, &ZVector, &XVector);
	}
	Normalise(&XVector);
	Normalise(&YVector);
	Normalise(&ZVector);

	dynPtr->OrientMat.mat11 = XVector.vx;
   	dynPtr->OrientMat.mat12 = XVector.vy;
	dynPtr->OrientMat.mat13 = XVector.vz;

	dynPtr->OrientMat.mat21 = YVector.vx;
	dynPtr->OrientMat.mat22 = YVector.vy;
	dynPtr->OrientMat.mat23 = YVector.vz;

	dynPtr->OrientMat.mat31 = ZVector.vx;
	dynPtr->OrientMat.mat32 = ZVector.vy;
	dynPtr->OrientMat.mat33 = ZVector.vz;

	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
}
static void VectorHomingForSurfaceAlign(VECTORCH *currentPtr, VECTORCH *targetPtr, VECTORCH *perpendicularPtr)
{
	int cos = Dot(currentPtr, targetPtr);

	if (cos<=-65000)
	{
		int a1 = NormalFrameTime*4;
		if (a1>ONE_FIXED) a1=ONE_FIXED;
		else if (a1<1024) a1=1024; 

		currentPtr->vx = currentPtr->vx+MUL_FIXED(perpendicularPtr->vx-currentPtr->vx, a1);
		currentPtr->vy = currentPtr->vy+MUL_FIXED(perpendicularPtr->vy-currentPtr->vy, a1);
		currentPtr->vz = currentPtr->vz+MUL_FIXED(perpendicularPtr->vz-currentPtr->vz, a1);
		Normalise(currentPtr);
		return;
	}
	else if (cos>=65500)	/* if they're practically parallel just snap currentPtr to targetPtr */
	{
		currentPtr->vx = targetPtr->vx;
		currentPtr->vy = targetPtr->vy;
		currentPtr->vz = targetPtr->vz;
		return;
	}	
	
	if (cos<32768) cos = 32768;
		
	{
  		int a1 = MUL_FIXED(cos*8,NormalFrameTime);
//		int a1 = NormalFrameTime*4;
		if (a1>ONE_FIXED) a1=ONE_FIXED;
		else if (a1<1024) a1=1024; 
		
		currentPtr->vx = currentPtr->vx+MUL_FIXED(targetPtr->vx-currentPtr->vx, a1);
		currentPtr->vy = currentPtr->vy+MUL_FIXED(targetPtr->vy-currentPtr->vy, a1);
		currentPtr->vz = currentPtr->vz+MUL_FIXED(targetPtr->vz-currentPtr->vz, a1);
		Normalise(currentPtr);
	}
	return;
}

extern void DynamicallyRotateObject(DYNAMICSBLOCK *dynPtr)
{
	extern int NormalFrameTime;
	
	MATRIXCH mat;
	EULER euler;
	euler.EulerX = MUL_FIXED(NormalFrameTime,dynPtr->AngVelocity.EulerX);
	euler.EulerX &= wrap360;

	euler.EulerY = MUL_FIXED(NormalFrameTime,dynPtr->AngVelocity.EulerY);
	euler.EulerY &= wrap360;

	euler.EulerZ = MUL_FIXED(NormalFrameTime,dynPtr->AngVelocity.EulerZ);
	euler.EulerZ &= wrap360;

	CreateEulerMatrix(&euler, &mat);
	TransposeMatrixCH(&mat);

  	MatrixMultiply(&dynPtr->OrientMat,&mat,&dynPtr->OrientMat);
 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);
}

static int InterferenceAt(int lambda, DYNAMICSBLOCK *dynPtr);

/* Move an object. At this stage, we have a list of the polygons in the
environment with which the object the may collide. */									   
static int MoveObject(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	int lowestBoundary = 0;
	int highestBoundary = ONE_FIXED;
	int testValue = ONE_FIXED;
    int hitSomething = 0;

    DirectionOfTravel = dynPtr->Displacement;
    Normalise(&DirectionOfTravel);
	
	{
		int maxDistanceAllowed=dynPtr->ObjectVertices[0].vz-dynPtr->ObjectVertices[7].vz;
		if (maxDistanceAllowed>dynPtr->ObjectVertices[0].vx-dynPtr->ObjectVertices[7].vx)
			maxDistanceAllowed=dynPtr->ObjectVertices[0].vx-dynPtr->ObjectVertices[7].vx;
		if (maxDistanceAllowed>dynPtr->ObjectVertices[0].vy-dynPtr->ObjectVertices[7].vy)
			maxDistanceAllowed=dynPtr->ObjectVertices[0].vy-dynPtr->ObjectVertices[7].vy;

		if (maxDistanceAllowed<10)
		{
			LOCALASSERT("Object's bounding box is too small. Suspicious."==0);
		}

		if (dynPtr->DistanceLeftToMove>maxDistanceAllowed)
		{
			testValue = DIV_FIXED(maxDistanceAllowed,dynPtr->DistanceLeftToMove);
			highestBoundary = testValue;
		}
	}

	if (InterferenceAt(testValue,dynPtr))
	{
		testValue /= 2;
		do
		{
			if (InterferenceAt(testValue,dynPtr))
			{
				highestBoundary = testValue;
				testValue = (lowestBoundary+highestBoundary)/2;
			}
			else
			{
				lowestBoundary = testValue;
				testValue = (lowestBoundary+highestBoundary)/2;
			}
			if (MUL_FIXED(highestBoundary-lowestBoundary,dynPtr->DistanceLeftToMove)<=16)
			{
				InterferenceAt(highestBoundary,dynPtr);
				break;
			}
		}
		while(1);
		testValue = lowestBoundary;
		hitSomething = 1;
	}

	{
		VECTORCH displacement;
		displacement.vx = MUL_FIXED(dynPtr->Displacement.vx,testValue);
		displacement.vy = MUL_FIXED(dynPtr->Displacement.vy,testValue);
		displacement.vz = MUL_FIXED(dynPtr->Displacement.vz,testValue);

		AddVectorToVector(displacement, dynPtr->Position);
		SubVectorFromVector(displacement, dynPtr->Displacement);
		{
			VECTORCH *vertexPtr = dynPtr->ObjectVertices;
			int i=8;
			do
			{
			  	vertexPtr->vx += displacement.vx;
				vertexPtr->vy += displacement.vy;
				vertexPtr->vz += displacement.vz;
				vertexPtr++;
			}
			while(--i);
		}

    }	
	
	if (hitSomething)
    {
		int wentUpStep = 0;
		VECTORCH obstacleNormal = {0,0,0};
		int n = NumberOfInterferencePolygons;
		VECTORCH objectCentre = {0,0,0};
		int leastSoFar = 1000000;
		struct ColPolyTag *polygonPtr=0;

		{
			if (DirectionOfTravel.vx>0)
			{
				objectCentre.vx = dynPtr->ObjectVertices[0].vx-COLLISION_GRANULARITY;
			}
			else if (DirectionOfTravel.vx<0)
			{
				objectCentre.vx = dynPtr->ObjectVertices[7].vx+COLLISION_GRANULARITY;
			}
			else
			{
				objectCentre.vx = (dynPtr->ObjectVertices[0].vx+dynPtr->ObjectVertices[7].vx)/2;
			}
			#if 1
			if (DirectionOfTravel.vy>0)
			{
				objectCentre.vy = dynPtr->ObjectVertices[0].vy-COLLISION_GRANULARITY;
			}
			else if (DirectionOfTravel.vy<0)
			{
				objectCentre.vy = dynPtr->ObjectVertices[7].vy+COLLISION_GRANULARITY;
			}
			else
			#endif
			{
				objectCentre.vy = (dynPtr->ObjectVertices[0].vy+dynPtr->ObjectVertices[7].vy)/2;
			}
			if (DirectionOfTravel.vz>0)
			{
				objectCentre.vz = dynPtr->ObjectVertices[0].vz-COLLISION_GRANULARITY;
			}
			else if (DirectionOfTravel.vz<0)
			{
				objectCentre.vz = dynPtr->ObjectVertices[7].vz+COLLISION_GRANULARITY;
			}
			else
			{
				objectCentre.vz = (dynPtr->ObjectVertices[0].vz+dynPtr->ObjectVertices[7].vz)/2;
			}

		}
		#if 0
		PrintDebuggingText("Test point %d,%d,%d\n",objectCentre.vx,objectCentre.vy,objectCentre.vz);
		#endif
		while(n--)
		{	
			#if 1
			VECTORCH r;
			int d;

   			r.vx = objectCentre.vx - InterferencePolygons[n].PolyPoint[0].vx;
   			r.vy = objectCentre.vy - InterferencePolygons[n].PolyPoint[0].vy;
   			r.vz = objectCentre.vz - InterferencePolygons[n].PolyPoint[0].vz;
			d = DotProduct(&r,&InterferencePolygons[n].PolyNormal);

			if (d<0) d+=1000000;
			{
				if (d<leastSoFar)
				{
					obstacleNormal = InterferencePolygons[n].PolyNormal;
					leastSoFar = d;
					polygonPtr = &InterferencePolygons[n];
				}
			}
			#else
			VECTORCH r;
			int d;

			d = DotProduct(&DirectionOfTravel,&InterferencePolygons[n].PolyNormal);

			if (d<0)
			{
				if (d<leastSoFar)
				{
					obstacleNormal = InterferencePolygons[n].PolyNormal;
					leastSoFar = d;
					polygonPtr = &InterferencePolygons[n];
				}
			}
			#endif

		}
		#if 0
		if (obstacleNormal.vx==0 && obstacleNormal.vy==0 && obstacleNormal.vz==0)
		{
			obstacleNormal.vy = -ONE_FIXED;
		}
		else
		{
			Normalise(&obstacleNormal);
		}
		#endif
		if(!polygonPtr)
		{	
			dynPtr->DistanceLeftToMove = 0;
			LOCALASSERT(0);
			return 0;
		}
		else 
		{
			#if 0
			PrintDebuggingText("POLY NORMAL IS %d %d %d\n",(polygonPtr->PolyNormal).vx,(polygonPtr->PolyNormal).vy,(polygonPtr->PolyNormal).vz);
			PrintDebuggingText("POLY NO OF VERTICES %d\n",(polygonPtr->NumberOfVertices));
			PrintDebuggingText("POLY POINT[0] IS %d %d %d\n",(polygonPtr->PolyPoint[0]).vx,(polygonPtr->PolyPoint[0]).vy,(polygonPtr->PolyPoint[0]).vz);
			PrintDebuggingText("POLY POINT[1] IS %d %d %d\n",(polygonPtr->PolyPoint[1]).vx,(polygonPtr->PolyPoint[1]).vy,(polygonPtr->PolyPoint[1]).vz);
			PrintDebuggingText("POLY POINT[2] IS %d %d %d\n",(polygonPtr->PolyPoint[2]).vx,(polygonPtr->PolyPoint[2]).vy,(polygonPtr->PolyPoint[2]).vz);
			#endif
		}
		/* test for a 'step' in front of object */
    	if ( (dynPtr->CanClimbStairs)
		/* check to see that polygon is vertical */
		  &&(polygonPtr->PolyNormal.vy>-250)
		  &&(polygonPtr->PolyNormal.vy<250) )
		{
	        int heightOfStep,topOfStep;
	        {
		    	int vertexNum=polygonPtr->NumberOfVertices-1;
		    	
		    	topOfStep = polygonPtr->PolyPoint[0].vy;
		        do
		        {
					int y = polygonPtr->PolyPoint[vertexNum].vy;

		        	if (y < topOfStep) topOfStep = y;
		        }
	            while(--vertexNum);
			}

	        heightOfStep = dynPtr->ObjectVertices[0].vy - topOfStep;  /* y-axis is +ve downwards, remember */
	 		//textprint("found step %d\n",heightOfStep);
	        if (heightOfStep>0 && heightOfStep < MAXIMUM_STEP_HEIGHT) /* we've hit a 'step' - move player upwards */
	        {
		   		DistanceToStepUp=heightOfStep+COLLISION_GRANULARITY;
	         	wentUpStep = SteppingUpIsValid(sbPtr);
				#if 0
				if (wentUpStep)
				{
					PrintDebuggingText("Found a valid step.\n");
				}
				else
				{
					PrintDebuggingText("Found a step but couldn't go up it.\n");
				}
				#endif

			}
		} 
	    	
		if (!wentUpStep)
		{
			STRATEGYBLOCK *obstacleSBPtr = 0;
			
			if (polygonPtr->ParentObject)
                if (polygonPtr->ParentObject->ObStrategyBlock)
                {
                    obstacleSBPtr = polygonPtr->ParentObject->ObStrategyBlock;
                }
		
			DistanceToStepUp = 0;

        	/* resolve player's movement vector against the collision plane */
			/* awkward problem here to do with non-exact normals */
			{
		   //     int magOfPerpVel = DotProduct(&obstacleNormal,&(dynPtr->Displacement));
		        int magOfPerpVel = MUL_FIXED(66000,DotProduct(&obstacleNormal,&(dynPtr->Displacement)));
		   		SubScaledVectorFromVector(obstacleNormal, magOfPerpVel, (dynPtr->Displacement));
			}
	
    		/* collision - elasticity */
			{
				int magOfPerpImp =	MUL_FIXED
									(
										DotProduct(&obstacleNormal,&dynPtr->LinImpulse),
										65536 + dynPtr->Elasticity
									);
				SubScaledVectorFromVector(obstacleNormal, magOfPerpImp, dynPtr->LinImpulse);
			}
			/* momentum test */
			/* OnlyCollideWithObjects flag indicates a platform lift etc. which should not be involved with momentum transfer */
			if (obstacleSBPtr && (obstacleSBPtr->DynPtr))
			if (!(dynPtr->OnlyCollideWithObjects
				||obstacleSBPtr->DynPtr->OnlyCollideWithObjects
				||obstacleSBPtr->DynPtr->IsStatic
				||dynPtr->IsInanimate
				||obstacleSBPtr->DynPtr->IsInanimate))
			{
				DYNAMICSBLOCK *obsDynPtr = obstacleSBPtr->DynPtr;
				int totalMass = (obsDynPtr->Mass + dynPtr->Mass)*4;
				int diffMass = (dynPtr->Mass-obsDynPtr->Mass)/2;

				obsDynPtr->LinImpulse.vx +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vx + dynPtr->LinVelocity.vx,
						dynPtr->Mass,
						totalMass
					);
				obsDynPtr->LinImpulse.vy +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vy + dynPtr->LinVelocity.vy,
						dynPtr->Mass,
						totalMass
					);
				obsDynPtr->LinImpulse.vz +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vz + dynPtr->LinVelocity.vz,
						dynPtr->Mass,
						totalMass
					);

				dynPtr->LinImpulse.vx +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vx + dynPtr->LinVelocity.vx,
						diffMass,
						totalMass
					);
				dynPtr->LinImpulse.vy +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vy + dynPtr->LinVelocity.vy,
						diffMass,
						totalMass
					);
				dynPtr->LinImpulse.vz +=
					WideMulNarrowDiv
					(
						dynPtr->LinImpulse.vz + dynPtr->LinVelocity.vz,
						diffMass,
						totalMass
					);

			}
			/* see if object has hit the 'floor' */
			if (dynPtr->GravityOn)
			{
				if (dynPtr->UseStandardGravity&&PlanarGravity)
				{
					if (obstacleNormal.vy < -FLOOR_THRESHOLD)
					{
						dynPtr->IsInContactWithFloor = 1;
						if (obstacleNormal.vy < -NEARLYFLATFLOOR_THRESHOLD) dynPtr->IsInContactWithNearlyFlatFloor = 1;
					}
				}
				else 
				{
					int angle = DotProduct(&obstacleNormal,&dynPtr->GravityDirection);
					if (angle < -FLOOR_THRESHOLD)
					{
						/* we've hit something that's against the direction of gravity */
						dynPtr->IsInContactWithFloor = 1;
						if (angle < -NEARLYFLATFLOOR_THRESHOLD) dynPtr->IsInContactWithNearlyFlatFloor = 1;
					}
				}
			}

			/* create a report about the collision */
			{
				{
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = obstacleSBPtr;
						reportPtr->ObstacleNormal = obstacleNormal;
						reportPtr->ObstaclePoint = dynPtr->Position;
					}
				}
				if (obstacleSBPtr&&obstacleSBPtr->DynPtr)
				{
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(obstacleSBPtr->DynPtr);
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = sbPtr;
						reportPtr->ObstacleNormal.vx = -obstacleNormal.vx;
						reportPtr->ObstacleNormal.vy = -obstacleNormal.vy;
						reportPtr->ObstacleNormal.vz = -obstacleNormal.vz;
						reportPtr->ObstaclePoint = dynPtr->Position;
					}
				}
			}
		}
		dynPtr->DistanceLeftToMove = Magnitude(&dynPtr->Displacement);    
		if (dynPtr->DistanceLeftToMove<=16)
		{
			dynPtr->DistanceLeftToMove = 0;
		}
		return 1;
    }    
    else
    {
		dynPtr->DistanceLeftToMove = Magnitude(&dynPtr->Displacement);    
		if (dynPtr->DistanceLeftToMove<=16)
		{
			dynPtr->DistanceLeftToMove = 0;
		}
		return 0;
   	}
	
}

static int InterferenceAt(int lambda, DYNAMICSBLOCK *dynPtr)
{
    VECTORCH objectVertices[8];
    int polysLeft;
    struct ColPolyTag *polyPtr;
    
	{
    	int vertexNum=8;

    	VECTORCH disp;
		disp.vx = MUL_FIXED(dynPtr->Displacement.vx,lambda);
		disp.vy = MUL_FIXED(dynPtr->Displacement.vy,lambda);
		disp.vz = MUL_FIXED(dynPtr->Displacement.vz,lambda);

		do
        {
			vertexNum--;
			objectVertices[vertexNum] = dynPtr->ObjectVertices[vertexNum];
			objectVertices[vertexNum].vx += disp.vx;			
			objectVertices[vertexNum].vy += disp.vy;			
			objectVertices[vertexNum].vz += disp.vz;			
	    }
        while(vertexNum);
	}

	polysLeft = NumberOfCollisionPolys;
    polyPtr = CollisionPolysArray;

	NumberOfInterferencePolygons = 0;

    while(polysLeft)
	{
		{
			if(DotProduct(&DirectionOfTravel,&polyPtr->PolyNormal)<0)
			if (DoesPolygonIntersectNRBB(polyPtr,objectVertices))
			{
				InterferencePolygons[NumberOfInterferencePolygons++] = *polyPtr;
				if (NumberOfInterferencePolygons==MAX_NUMBER_OF_INTERFERENCE_POLYGONS) break;
			}
		}
        polyPtr++;
		polysLeft--;
	}
	return NumberOfInterferencePolygons;
}






static void MovePlatformLift(STRATEGYBLOCK *sbPtr)
{		 
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    int polysLeft;
    struct ColPolyTag *nearPolysPtr;
    int distanceToMove;

	VECTORCH obstacleNormal;
	STRATEGYBLOCK *obstacleSBPtr = 0;
    
    DirectionOfTravel = dynPtr->Displacement;
    Normalise(&DirectionOfTravel);

    /* ugh */				
    distanceToMove = dynPtr->DistanceLeftToMove;
    
	/* make list of platform's polygons */
	{
		VECTORCH zero = {0,0,0};
		MakeDynamicBoundingBoxForObject(sbPtr, &(dynPtr->Position));
		TestShapeWithDynamicBoundingBox(sbPtr->SBdptr,dynPtr);
		MakeDynamicBoundingBoxForObject(sbPtr, &zero);
	}

//	textprint("polys on lift %d\n",NumberOfCollisionPolys);
    DirectionOfTravel.vx = -DirectionOfTravel.vx;
    DirectionOfTravel.vy = -DirectionOfTravel.vy;
    DirectionOfTravel.vz = -DirectionOfTravel.vz;
	{
		int i = NumberOfDynamicObjects;
	    while(i--)    
		{
			
			STRATEGYBLOCK *obstaclePtr = DynamicObjectsList[i];

			/* check whether collision is even possible */
			{
			   	if (sbPtr==obstaclePtr)
					continue;
				{
					VECTORCH *objectVerticesPtr = obstaclePtr->DynPtr->ObjectVertices;
				   	if (!( ( (DBBMaxX >= objectVerticesPtr[7].vx) && (DBBMinX <= objectVerticesPtr[0].vx) )
				   	     &&( (DBBMaxY >= objectVerticesPtr[7].vy) && (DBBMinY <= objectVerticesPtr[0].vy) )
				         &&( (DBBMaxZ >= objectVerticesPtr[7].vz) && (DBBMinZ <= objectVerticesPtr[0].vz) ) ))
						continue;									 
				}
			}

			//textprint("found an object\n");

			polysLeft = NumberOfCollisionPolys;
			nearPolysPtr = CollisionPolysArray;

		    distanceToMove = dynPtr->DistanceLeftToMove;

			/* check against the landscape */
		    while(polysLeft)
			{
				signed int distanceToObstacle;
				
				distanceToObstacle = DistanceMovedBeforeObjectHitsPolygon(obstaclePtr->DynPtr,nearPolysPtr,distanceToMove);

				if (distanceToObstacle>=0)
		        {
				
			       	distanceToMove = distanceToObstacle;
				   	obstacleNormal = nearPolysPtr->PolyNormal;
					obstacleSBPtr =	obstaclePtr;
				}
		        nearPolysPtr++;
				polysLeft--;
			}
			
			if (distanceToMove!=dynPtr->DistanceLeftToMove)
			{
				if (dynPtr->Displacement.vy<0)
				{
					VECTORCH displacement;
//					displacement.vx = -MUL_FIXED(DirectionOfTravel.vx, dynPtr->DistanceLeftToMove-distanceToMove+COLLISION_GRANULARITY);
					displacement.vx = displacement.vz = 0;
					displacement.vy = -(dynPtr->DistanceLeftToMove-distanceToMove+COLLISION_GRANULARITY);
//					displacement.vz = -MUL_FIXED(DirectionOfTravel.vz, dynPtr->DistanceLeftToMove-distanceToMove+COLLISION_GRANULARITY);
					AddVectorToVector(displacement, obstaclePtr->DynPtr->Position);
					obstaclePtr->DynPtr->PrevPosition = obstaclePtr->DynPtr->Position;
					{
						VECTORCH *vertexPtr = obstaclePtr->DynPtr->ObjectVertices;
						int i=8;
						do
						{
							vertexPtr->vy += displacement.vy;
							vertexPtr++;
						}
						while(--i);
					}
					if (obstaclePtr->SBdptr == Player)
					{
						/* look for polygons inside this volume */
						FindLandscapePolygonsInObjectsVicinity(obstaclePtr);
						FindObjectsToRelocateAgainst(obstaclePtr);

						{
							int polysLeft;
						    struct ColPolyTag *polyPtr;
							
						    polysLeft = NumberOfCollisionPolys;
						    polyPtr = CollisionPolysArray;

							while(polysLeft)
							{
						        if(DoesPolygonIntersectNRBB(polyPtr,obstaclePtr->DynPtr->ObjectVertices))
						        {
							   		int greatestDistance;

							    	{
							    		VECTORCH vertex = obstaclePtr->DynPtr->ObjectVertices[WhichNRBBVertex(obstaclePtr->DynPtr,&(polyPtr->PolyNormal))];
										vertex.vx -= polyPtr->PolyPoint[0].vx;
										vertex.vy -= polyPtr->PolyPoint[0].vy;
										vertex.vz -= polyPtr->PolyPoint[0].vz;
										greatestDistance = -DotProduct(&vertex,&(polyPtr->PolyNormal));
									}

									if (greatestDistance>0)
									{
										/* sorry, no room! */
										SubVectorFromVector(displacement, obstaclePtr->DynPtr->Position);
										obstaclePtr->DynPtr->PrevPosition = obstaclePtr->DynPtr->Position;
										{
											VECTORCH *vertexPtr = obstaclePtr->DynPtr->ObjectVertices;
											int i=8;
											do
											{
												vertexPtr->vy -= displacement.vy;
												vertexPtr++;
											}
											while(--i);
										}

										return;
									}
						        }
						        polyPtr++;
								polysLeft--;
							}
							
						}
					}
					if (obstaclePtr->DynPtr->Displacement.vy>0)
					{
						obstaclePtr->DynPtr->Displacement.vy=0;
						obstaclePtr->DynPtr->DistanceLeftToMove = Magnitude(&obstaclePtr->DynPtr->Displacement);
					}
					if (obstaclePtr->DynPtr->LinImpulse.vy>0)
					{
						obstaclePtr->DynPtr->LinImpulse.vy=0;
					}

				}
				{
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
					
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = obstacleSBPtr;
						reportPtr->ObstacleNormal.vx = -obstacleNormal.vx;
						reportPtr->ObstacleNormal.vy = -obstacleNormal.vy;
						reportPtr->ObstacleNormal.vz = -obstacleNormal.vz;
					}
				}
				if (obstacleSBPtr) //&& !(obstacleSBPtr->DynPtr->StopOnCollision))
				 /* give obstacle a report too */
				{
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(obstacleSBPtr->DynPtr);
					
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = sbPtr;
						reportPtr->ObstacleNormal.vx = obstacleNormal.vx;
						reportPtr->ObstacleNormal.vy = obstacleNormal.vy;
						reportPtr->ObstacleNormal.vz = obstacleNormal.vz;
					}
				    /* see if object has hit the 'floor' */
					if (obstacleSBPtr->DynPtr->GravityOn)
					{
						if (obstacleSBPtr->DynPtr->UseStandardGravity&&PlanarGravity)
						{
							if (obstacleNormal.vy < -FLOOR_THRESHOLD)
							{
								obstacleSBPtr->DynPtr->IsInContactWithFloor = 1;
								if (obstacleNormal.vy < -NEARLYFLATFLOOR_THRESHOLD) obstacleSBPtr->DynPtr->IsInContactWithNearlyFlatFloor = 1;
							}
						}
						else 
						{
							int angle = DotProduct(&obstacleNormal,&obstacleSBPtr->DynPtr->GravityDirection);
							if (angle < -FLOOR_THRESHOLD)
							{
								/* we've hit something that's against the direction of gravity */
								obstacleSBPtr->DynPtr->IsInContactWithFloor = 1;
								if (angle < -NEARLYFLATFLOOR_THRESHOLD) obstacleSBPtr->DynPtr->IsInContactWithNearlyFlatFloor = 1;
							}
						}
					}

				}
			}
	   	}
//   	textprint("moving %d out of %d\n",distanceToMove,dynPtr->DistanceLeftToMove);

   	/* move object */

 //		textprint("disp %d %d %d\n",displacement.vx,displacement.vy,displacement.vz);

    }	
	if (dynPtr->Displacement.vy>0 && dynPtr->CollisionReportPtr)
	{
	}
	else
	{
		AddVectorToVector(dynPtr->Displacement, dynPtr->Position);
	}

	#if 0
    if (distanceToMove!=dynPtr->DistanceLeftToMove)
    {
		/* create a report about the collision */
		{
			COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
			
			if (reportPtr)
			{
				reportPtr->ObstacleSBPtr = obstacleSBPtr;
				reportPtr->ObstacleNormal.vx = -obstacleNormal.vx;
				reportPtr->ObstacleNormal.vy = -obstacleNormal.vy;
				reportPtr->ObstacleNormal.vz = -obstacleNormal.vz;
			}
		}
		
		if (obstacleSBPtr) //&& !(obstacleSBPtr->DynPtr->StopOnCollision))
		 /* give obstacle a report too */
		{
			COLLISIONREPORT *reportPtr = AllocateCollisionReport(obstacleSBPtr->DynPtr);
			
			if (reportPtr)
			{
				reportPtr->ObstacleSBPtr = sbPtr;
				reportPtr->ObstacleNormal.vx = obstacleNormal.vx;
				reportPtr->ObstacleNormal.vy = obstacleNormal.vy;
				reportPtr->ObstacleNormal.vz = obstacleNormal.vz;
			}
		}
	}
	#endif
	

}





static void TestForValidPlayerStandUp(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	/* make a collision volume corresponding to a standing character */
	{
		COLLISION_EXTENTS *extentsPtr = 0;
		
		switch(AvP.PlayerType)
		{
			default:
				LOCALASSERT(0);
				/* if no debug then fall through to marine */
			case I_Marine:
				extentsPtr = &CollisionExtents[CE_MARINE];
				break;
				
			case I_Alien:
				extentsPtr = &CollisionExtents[CE_ALIEN];
				break;
			
			case I_Predator:
				extentsPtr = &CollisionExtents[CE_PREDATOR];
				break;
		
		}
	
		/* max X */
		dynPtr->ObjectVertices[0].vx = extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[1].vx = extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[2].vx = extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[3].vx = extentsPtr->CollisionRadius;

		/* max Z */
		dynPtr->ObjectVertices[0].vz = extentsPtr->CollisionRadius;
	    dynPtr->ObjectVertices[2].vz = extentsPtr->CollisionRadius;
	    dynPtr->ObjectVertices[4].vz = extentsPtr->CollisionRadius;
	    dynPtr->ObjectVertices[6].vz = extentsPtr->CollisionRadius;

		/* min X */
		dynPtr->ObjectVertices[4].vx = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[5].vx = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[6].vx = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[7].vx = -extentsPtr->CollisionRadius;
	
		/* min Z */
		dynPtr->ObjectVertices[1].vz = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[3].vz = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[5].vz = -extentsPtr->CollisionRadius;
		dynPtr->ObjectVertices[7].vz = -extentsPtr->CollisionRadius;
	
		/* max Y */
	   	dynPtr->ObjectVertices[0].vy = extentsPtr->Bottom;
	    dynPtr->ObjectVertices[1].vy = extentsPtr->Bottom;
	    dynPtr->ObjectVertices[4].vy = extentsPtr->Bottom;
	    dynPtr->ObjectVertices[5].vy = extentsPtr->Bottom;

		/* min Y */
	   	dynPtr->ObjectVertices[2].vy = extentsPtr->StandingTop;
	    dynPtr->ObjectVertices[3].vy = extentsPtr->StandingTop;
	    dynPtr->ObjectVertices[6].vy = extentsPtr->StandingTop;
	    dynPtr->ObjectVertices[7].vy = extentsPtr->StandingTop;

		/* translate cuboid into world space */
		{						   
			VECTORCH *vertexPtr = dynPtr->ObjectVertices;
	        
	        int vertexNum=8;
	        do
	        {
	        	vertexPtr->vx += dynPtr->Position.vx;
	        	vertexPtr->vy += dynPtr->Position.vy;	
	        	vertexPtr->vz += dynPtr->Position.vz;	
	        	vertexPtr++;
	        }
	        while(--vertexNum);
		}

	}

	/* look for polygons inside this volume */
	FindLandscapePolygonsInObjectsVicinity(sbPtr);
	FindObjectsToRelocateAgainst(sbPtr);
	

	{

		int polysLeft;
	    struct ColPolyTag *polyPtr;
		
	    polysLeft = NumberOfCollisionPolys;
	    polyPtr = CollisionPolysArray;

		while(polysLeft)
		{
	        if(DoesPolygonIntersectNRBB(polyPtr,dynPtr->ObjectVertices))
	        {
		   		int greatestDistance;

		    	{
		    		VECTORCH vertex = dynPtr->ObjectVertices[WhichNRBBVertex(dynPtr,&(polyPtr->PolyNormal))];
					vertex.vx -= polyPtr->PolyPoint[0].vx;
					vertex.vy -= polyPtr->PolyPoint[0].vy;
					vertex.vz -= polyPtr->PolyPoint[0].vz;
					greatestDistance = -DotProduct(&vertex,&(polyPtr->PolyNormal));
				}

				if (greatestDistance>0)
				{
					/* sorry, no standing room */
		 			CreateNRBBForObject(sbPtr);
					return;
				}
	        }
	        polyPtr++;
			polysLeft--;
		}
		
	}

	/* standing up is ok */ 
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

		/* set player state */
		playerStatusPtr->ShapeState = PMph_Standing;

		/* if player is an alien, cancel his ability to walk on walls */
		if (AvP.PlayerType == I_Alien)
		{
			dynPtr->UseStandardGravity=1;
		}
	}											

}   
static int SteppingUpIsValid(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	VECTORCH displacement;

	displacement.vx = MUL_FIXED(DirectionOfTravel.vx, COLLISION_GRANULARITY*2);
	displacement.vy = - DistanceToStepUp;
	displacement.vz = MUL_FIXED(DirectionOfTravel.vz, COLLISION_GRANULARITY*2);


	
	{
		int i=8;
		VECTORCH *vertexPtr = dynPtr->ObjectVertices;
		do
		{
			vertexPtr->vx += displacement.vx;
			vertexPtr->vy += displacement.vy;
			vertexPtr->vz += displacement.vz;
			vertexPtr++;
		}
		while(--i);
		dynPtr->Position.vx += displacement.vx;
		dynPtr->Position.vy += displacement.vy;
		dynPtr->Position.vz += displacement.vz;
	}

	/* look for polygons inside this volume */
	FindLandscapePolygonsInObjectsVicinity(sbPtr);
	FindObjectsToRelocateAgainst(sbPtr);
	

	{

		int polysLeft;
	    struct ColPolyTag *polyPtr;
		
	    polysLeft = NumberOfCollisionPolys;
	    polyPtr = CollisionPolysArray;

		while(polysLeft)
		{
	        if(DoesPolygonIntersectNRBB(polyPtr,dynPtr->ObjectVertices))
	        {
		   		#if 0
		   		int greatestDistance;

		    	{
		    		VECTORCH vertex = dynPtr->ObjectVertices[WhichNRBBVertex(dynPtr,&(polyPtr->PolyNormal))];
					vertex.vx -= polyPtr->PolyPoint[0].vx;
					vertex.vy -= polyPtr->PolyPoint[0].vy;
					vertex.vz -= polyPtr->PolyPoint[0].vz;
					greatestDistance = -DotProduct(&vertex,&(polyPtr->PolyNormal));
				}

				if (greatestDistance>0)
				#endif
				{
					/* sorry, there's a polygon in the way */
					//textprint("no step %d\n",greatestDistance);
					{
						int i=8;
						VECTORCH *vertexPtr = dynPtr->ObjectVertices;
						do
						{
							vertexPtr->vx -= displacement.vx;
							vertexPtr->vy -= displacement.vy;
							vertexPtr->vz -= displacement.vz;
							vertexPtr++;
						}
						while(--i);
						dynPtr->Position.vx -= displacement.vx;
						dynPtr->Position.vy -= displacement.vy;
						dynPtr->Position.vz -= displacement.vz;
					}
					return 0;
				}
	        }
	        polyPtr++;
			polysLeft--;
		}
		
	}

	/* steping up is ok */ 
	{
		//textprint("step ok\n");
		return 1;
	}											

}   

static void FindLandscapePolygonsInObjectsPath(STRATEGYBLOCK *sbPtr)
{
	extern int NumActiveBlocks;
    extern DISPLAYBLOCK *ActiveBlockList[];

	/* initialise near polygons array */	
	CollisionPolysPtr = &CollisionPolysArray[0];
    NumberOfCollisionPolys=0;

   	/* scan through ActiveBlockList for modules */
	{
	   	int numberOfObjects = NumActiveBlocks;
	   	while(numberOfObjects)
	   	{
	   		DISPLAYBLOCK* objectPtr = ActiveBlockList[--numberOfObjects];
	   		char isStaticObject=0;

	   		GLOBALASSERT(objectPtr);
			if(objectPtr->ObStrategyBlock)
				if(objectPtr->ObStrategyBlock->DynPtr)
				{
					if(((objectPtr->ObStrategyBlock->DynPtr->IsStatic)
			  		||(objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithObjects))
			  		&&(!objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithEnvironment))
						isStaticObject=1;
				}

	   		if (objectPtr->ObMyModule) 
	    	{
				MakeDynamicBoundingBoxForObject(sbPtr, &(objectPtr->ObWorld));
			
			    /* if the bounding box intersects with the object, investigate */
			   	if (( (DBBMaxX >= objectPtr->ObMinX) && (DBBMinX <= objectPtr->ObMaxX) )
			      &&( (DBBMaxY >= objectPtr->ObMinY) && (DBBMinY <= objectPtr->ObMaxY) )
 			      &&( (DBBMaxZ >= objectPtr->ObMinZ) && (DBBMinZ <= objectPtr->ObMaxZ) ))
			       	TestShapeWithDynamicBoundingBox(objectPtr,sbPtr->DynPtr);
	        }
	   		else if (isStaticObject)
	    	{
				MakeDynamicBoundingBoxForObject(sbPtr, &(objectPtr->ObWorld));
			
			    /* if the bounding box intersects with the object, investigate */
			   	if (( (DBBMaxX >= -objectPtr->ObRadius) && (DBBMinX <= objectPtr->ObRadius) )
			      &&( (DBBMaxY >= -objectPtr->ObRadius) && (DBBMinY <= objectPtr->ObRadius) )
 			      &&( (DBBMaxZ >= -objectPtr->ObRadius) && (DBBMinZ <= objectPtr->ObRadius) ))
			       	TestShapeWithDynamicBoundingBox(objectPtr,sbPtr->DynPtr);
	        }
	    }
  	}
}   
static void FindObjectPolygonsInObjectsPath(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr=sbPtr->DynPtr;
	/* check against other objects */
	{
		VECTORCH zero = {0,0,0};
		int i = NumberOfDynamicObjects;
		MakeDynamicBoundingBoxForObject(sbPtr, &zero);
	    while(i--)    
		{
			
			STRATEGYBLOCK *obstaclePtr = DynamicObjectsList[i];
			VECTORCH *objectVerticesPtr = obstaclePtr->DynPtr->ObjectVertices;

			/* check whether collision is even possible */
			{
			   	if (sbPtr==obstaclePtr)
					continue;
			
				/* can it be seen? */
				if(obstaclePtr->SBdptr->ObFlags&ObFlag_NotVis) continue;
		
				/* two sprites */
				if((dynPtr->DynamicsType == DYN_TYPE_SPRITE_COLLISIONS)
			     &&(obstaclePtr->DynPtr->DynamicsType == DYN_TYPE_SPRITE_COLLISIONS) ) 
					continue;

				if ( (dynPtr->IgnoreSameObjectsAsYou || obstaclePtr->DynPtr->IgnoreSameObjectsAsYou) 
				 &&(sbPtr->I_SBtype == obstaclePtr->I_SBtype) )
					continue;
		
				if (obstaclePtr->DynPtr->OnlyCollideWithObjects)
					continue;

				if (obstaclePtr->DynPtr->OnlyCollideWithEnvironment)
					continue;

				if ( ((obstaclePtr->SBdptr == Player) && dynPtr->IgnoreThePlayer)
				   ||((sbPtr->SBdptr == Player) && obstaclePtr->DynPtr->IgnoreThePlayer) )
					continue;

				if( (sbPtr->SBdptr==Player)
				  &&( (obstaclePtr->I_SBtype == I_BehaviourHierarchicalFragment)
				    ||(obstaclePtr->DynPtr->IsPickupObject))
				  )
				{
					continue;
				}

			   	if (!( ( (DBBMaxX >= objectVerticesPtr[7].vx) && (DBBMinX <= objectVerticesPtr[0].vx) )
			   	     &&( (DBBMaxY >= objectVerticesPtr[7].vy) && (DBBMinY <= objectVerticesPtr[0].vy) )
			         &&( (DBBMaxZ >= objectVerticesPtr[7].vz) && (DBBMinZ <= objectVerticesPtr[0].vz) ) ))
					continue;									 
			}

			{
				const int *vertexIndexPtr=&CuboidVertexList[0];
				int face=6;
				
				do
				{
					struct ColPolyTag poly;
					poly.NumberOfVertices=4;
					poly.ParentObject = obstaclePtr->SBdptr;
					
					poly.PolyPoint[0]=objectVerticesPtr[*vertexIndexPtr++];
					poly.PolyPoint[1]=objectVerticesPtr[*vertexIndexPtr++];
					poly.PolyPoint[2]=objectVerticesPtr[*vertexIndexPtr++];
					poly.PolyPoint[3]=objectVerticesPtr[*vertexIndexPtr++];

					
					if (IsPolygonWithinDynamicBoundingBox(&poly))
	 				{
						{
							switch(face)
							{
								case 6: /* all points are on max y face */
								{
								    poly.PolyNormal.vx = 0;
								    poly.PolyNormal.vy = ONE_FIXED;
								    poly.PolyNormal.vz = 0;
									break;
								}
								case 5: /* all points are on max z face */
								{
								    poly.PolyNormal.vx = 0;
								    poly.PolyNormal.vy = 0;
								    poly.PolyNormal.vz = ONE_FIXED;
									break;
								}
								case 4: /* all points are on max x face */
								{
								    poly.PolyNormal.vx = ONE_FIXED;
								    poly.PolyNormal.vy = 0;
								    poly.PolyNormal.vz = 0;
									break;
								}
								case 3: /* all points are on min z face */
								{
								    poly.PolyNormal.vx = 0;
								    poly.PolyNormal.vy = 0;
								    poly.PolyNormal.vz = -ONE_FIXED;
									break;
								}
								case 2: /* all points are on min x face */
								{
								    poly.PolyNormal.vx = -ONE_FIXED;
								    poly.PolyNormal.vy = 0;
								    poly.PolyNormal.vz = 0;
									break;
								}
								case 1: /* all points are on min y face */
								{									   
								    poly.PolyNormal.vx = 0;
								    poly.PolyNormal.vy = -ONE_FIXED;
								    poly.PolyNormal.vz = 0;
									break;
								}
							}
						}

						#if 0
						polyDistance = DistanceMovedBeforeObjectHitsPolygon(dynPtr,&poly,distanceToMove);
						if (polyDistance>=0)
				        {
							/* If the player moves into a weapon/ammo/etc, report the collision but
							don't stop the player. (ie. let him walk through it) */
							if( (sbPtr->SBdptr==Player)
							  &&( (obstaclePtr->I_SBtype == I_BehaviourHierarchicalFragment)
							    ||(obstaclePtr->DynPtr->IsPickupObject))
							  )
							{
								/* create a report about the collision */
								COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
								if (reportPtr)
								{
									reportPtr->ObstacleSBPtr = obstaclePtr;
									reportPtr->ObstacleNormal = poly.PolyNormal;
								}
							}
							else
							{
					       	   	distanceToMove = polyDistance;
							   	obstacleNormal = poly.PolyNormal;
								obstaclePoint = poly.PolyPoint[0];
								obstacleSBPtr =	obstaclePtr;
								LOCALASSERT(obstaclePtr);
								topOfStep = -2000000000;
							}
						}
						#endif
						*CollisionPolysPtr = poly;

						CollisionPolysPtr++;
				    	NumberOfCollisionPolys++;

					}
					face--; 
				}
				while(face);
				
			}
		}
	}
}

static void FindLandscapePolygonsInObjectsVicinity(STRATEGYBLOCK *sbPtr)
{
	extern int NumActiveBlocks;
    extern DISPLAYBLOCK *ActiveBlockList[];

	/* intialise near polygons array */	
	CollisionPolysPtr = &CollisionPolysArray[0];
    NumberOfCollisionPolys=0;

   	/* scan through ActiveBlockList for modules */
	{
	   	int numberOfObjects = NumActiveBlocks;
	   	while(numberOfObjects)
	   	{
	   		DISPLAYBLOCK* objectPtr = ActiveBlockList[--numberOfObjects];
	   		char isStaticObject=0;

	   		GLOBALASSERT(objectPtr);
			if(objectPtr->ObStrategyBlock)
				if(objectPtr->ObStrategyBlock->DynPtr)
				{
					if(((objectPtr->ObStrategyBlock->DynPtr->IsStatic)
					||(objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithObjects))
			  		&&(!objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithEnvironment))
						isStaticObject=1;
				}

	   		if (objectPtr->ObMyModule) /* is object a module or static? */
	    	{
				MakeStaticBoundingBoxForObject(sbPtr);
			
				/* translate SBB into space of landscape module */
				SBBMinX -= objectPtr->ObWorld.vx;
				SBBMaxX -= objectPtr->ObWorld.vx;
				
				SBBMinY -= objectPtr->ObWorld.vy;
				SBBMaxY -= objectPtr->ObWorld.vy;
				
				SBBMinZ -= objectPtr->ObWorld.vz;
				SBBMaxZ -= objectPtr->ObWorld.vz;

			    /* if the bounding box intersects with the object, investigate */
			   	if (( (SBBMaxX >= objectPtr->ObMinX) && (SBBMinX <= objectPtr->ObMaxX) )
			      &&( (SBBMaxY >= objectPtr->ObMinY) && (SBBMinY <= objectPtr->ObMaxY) )
 			      &&( (SBBMaxZ >= objectPtr->ObMinZ) && (SBBMinZ <= objectPtr->ObMaxZ) ))
			       	TestShapeWithStaticBoundingBox(objectPtr);
	        }
			else if (isStaticObject)
			{
				MakeStaticBoundingBoxForObject(sbPtr);
			
				/* translate SBB into space of landscape module */
				SBBMinX -= objectPtr->ObWorld.vx;
				SBBMaxX -= objectPtr->ObWorld.vx;
				
				SBBMinY -= objectPtr->ObWorld.vy;
				SBBMaxY -= objectPtr->ObWorld.vy;
				
				SBBMinZ -= objectPtr->ObWorld.vz;
				SBBMaxZ -= objectPtr->ObWorld.vz;

			    /* if the bounding box intersects with the object, investigate */
			   	if (( (SBBMaxX >= objectPtr->ObMinX) && (SBBMinX <= objectPtr->ObMaxX) )
			      &&( (SBBMaxY >= objectPtr->ObMinY) && (SBBMinY <= objectPtr->ObMaxY) )
 			      &&( (SBBMaxZ >= objectPtr->ObMinZ) && (SBBMinZ <= objectPtr->ObMaxZ) ))
			       	TestShapeWithStaticBoundingBox(objectPtr);
			}

	    }
  	}
}   

static void FindObjectsToRelocateAgainst(STRATEGYBLOCK *sbPtr)					
{
	MakeStaticBoundingBoxForObject(sbPtr);
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		int i = NumberOfDynamicObjects;
	    while(i--)
		{
			STRATEGYBLOCK *obstaclePtr = DynamicObjectsList[i];

			/* check whether collision is even possible */
			if (sbPtr==obstaclePtr)
				continue;

			/* ignore platform lifts */
		 	if(obstaclePtr->DynPtr->OnlyCollideWithObjects)
		 		continue;

			/* ignore things that only collide with environment */
			if (obstaclePtr->DynPtr->OnlyCollideWithEnvironment)
				continue;

			/* don't relocate against ammo and stuff */
			if (sbPtr->SBdptr==Player)
			{
				if(obstaclePtr->DynPtr->IsPickupObject)
				{
						continue;
				}
			}
		
			/* two sprites */
			if((dynPtr->DynamicsType == DYN_TYPE_SPRITE_COLLISIONS)
			   &&(obstaclePtr->DynPtr->DynamicsType == DYN_TYPE_SPRITE_COLLISIONS) ) 
				continue;

			if (dynPtr->IgnoreSameObjectsAsYou
			 &&(sbPtr->I_SBtype == obstaclePtr->I_SBtype) )
				continue;
			
			TestObjectWithStaticBoundingBox(obstaclePtr->SBdptr);
		}
	}
}

static void MakeDynamicBoundingBoxForObject(STRATEGYBLOCK *sbPtr, VECTORCH *worldOffsetPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	DBBMaxX = dynPtr->ObjectVertices[0].vx - worldOffsetPtr->vx + COLLISION_GRANULARITY; 
	DBBMinX = dynPtr->ObjectVertices[7].vx - worldOffsetPtr->vx - COLLISION_GRANULARITY;

	DBBMaxY = dynPtr->ObjectVertices[0].vy - worldOffsetPtr->vy + COLLISION_GRANULARITY; 
	DBBMinY = dynPtr->ObjectVertices[7].vy - worldOffsetPtr->vy - COLLISION_GRANULARITY;
																					   
	DBBMaxZ = dynPtr->ObjectVertices[0].vz - worldOffsetPtr->vz + COLLISION_GRANULARITY;
	DBBMinZ = dynPtr->ObjectVertices[7].vz - worldOffsetPtr->vz - COLLISION_GRANULARITY; 

	if (dynPtr->Displacement.vx > 0)
	{
		DBBMaxX += dynPtr->Displacement.vx;
	}    
	else
	{
	    DBBMinX += dynPtr->Displacement.vx;
	}

	if (dynPtr->Displacement.vy > 0)
	{
		DBBMaxY += dynPtr->Displacement.vy;
	}    
	else
	{
	    DBBMinY += dynPtr->Displacement.vy;
	}

	if (dynPtr->Displacement.vz > 0)
	{
		DBBMaxZ += dynPtr->Displacement.vz;
	}    
	else
	{
	    DBBMinZ += dynPtr->Displacement.vz;
	}

}

static void MakeStaticBoundingBoxForSphere(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	int objectSize = dynPtr->CollisionRadius+COLLISION_GRANULARITY;

    SBBMinX = dynPtr->Position.vx - objectSize;
	SBBMaxX = dynPtr->Position.vx + objectSize;
    SBBMinY = dynPtr->Position.vy - objectSize;
	SBBMaxY = dynPtr->Position.vy + objectSize;
    SBBMinZ = dynPtr->Position.vz - objectSize;
	SBBMaxZ = dynPtr->Position.vz + objectSize;
}
static void MakeStaticBoundingBoxForNRBB(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    SBBMinX = dynPtr->ObjectVertices[7].vx - COLLISION_GRANULARITY*16;
	SBBMaxX = dynPtr->ObjectVertices[0].vx + COLLISION_GRANULARITY*16;

    SBBMinY = dynPtr->ObjectVertices[7].vy - COLLISION_GRANULARITY*16;
	SBBMaxY = dynPtr->ObjectVertices[0].vy + COLLISION_GRANULARITY*16;

    SBBMinZ = dynPtr->ObjectVertices[7].vz - COLLISION_GRANULARITY*16;
	SBBMaxZ = dynPtr->ObjectVertices[0].vz + COLLISION_GRANULARITY*16;
}																  

static void TestShapeWithDynamicBoundingBox(DISPLAYBLOCK *objectPtr, DYNAMICSBLOCK *mainDynPtr)
{
	int numberOfItems;
    int needToRotate = 0;

    /* KJL 10:58:22 24/11/98 - If the object is a static object rather than a module,
	we'll need to rotate the polygons into world-space */
   	if (objectPtr->ObStrategyBlock)
	{
		DYNAMICSBLOCK *dynPtr = objectPtr->ObStrategyBlock->DynPtr;

		if (dynPtr)
		{
			if (dynPtr->IsStatic)
			{
				needToRotate = 1;
			}
		}
	}
    
    
    /* okay, let's setup the shape's data and access the first poly */
	numberOfItems = SetupPolygonAccess(objectPtr);
    
    /* go through polys looking for those which intersect with the bounding box */
  	while(numberOfItems--)
	{
		AccessNextPolygon();

		if (mainDynPtr->IgnoresNotVisPolys && (PolygonFlag & iflag_notvis) && !(PolygonFlag & iflag_mirror)) continue;

		GetPolygonVertices(CollisionPolysPtr);

       	if (needToRotate)
		{
			int i = CollisionPolysPtr->NumberOfVertices;	
        	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

			           	
           	do
           	{	
				RotateVector(polyVertexPtr,&objectPtr->ObMat);
				polyVertexPtr++;
        	}
			while(--i);
        }

		{
			VECTORCH *vertices = CollisionPolysPtr->PolyPoint;
			if (CollisionPolysPtr->NumberOfVertices==4)
			{
		    	if (vertices[0].vy < DBBMinY)
		        	if (vertices[1].vy < DBBMinY) 
			      		if (vertices[2].vy < DBBMinY) 
			      			if (vertices[3].vy < DBBMinY) 
		          				continue;
				    
		    	if (vertices[0].vx < DBBMinX)
		    		if (vertices[1].vx < DBBMinX)
		    			if (vertices[2].vx < DBBMinX)
		    				if (vertices[3].vx < DBBMinX) 
		          				continue;
		          
		    	if (vertices[0].vx > DBBMaxX) 
			    	if (vertices[1].vx > DBBMaxX) 
			      		if (vertices[2].vx > DBBMaxX) 
			    			if (vertices[3].vx > DBBMaxX) 
		          				continue;
			    
		    	if (vertices[0].vz < DBBMinZ) 
			    	if (vertices[1].vz < DBBMinZ) 
			      		if (vertices[2].vz < DBBMinZ) 
			      			if (vertices[3].vz < DBBMinZ) 
		          				continue;
				    
		    	if (vertices[0].vz > DBBMaxZ) 
			    	if (vertices[1].vz > DBBMaxZ) 
			    		if (vertices[2].vz > DBBMaxZ) 
			    			if (vertices[3].vz > DBBMaxZ) 
			    				continue;

		    	if (vertices[0].vy > DBBMaxY) 
			    	if (vertices[1].vy > DBBMaxY) 
			    		if (vertices[2].vy > DBBMaxY) 
			    			if (vertices[3].vy > DBBMaxY) 
		        			  	continue;

		    }
		    else
			{
		    	if (vertices[0].vy < DBBMinY)
		        	if (vertices[1].vy < DBBMinY) 
			    		if (vertices[2].vy < DBBMinY) 
		          			continue;
			      
		    	if (vertices[0].vx < DBBMinX) 
			    	if (vertices[1].vx < DBBMinX) 
			    		if (vertices[2].vx < DBBMinX) 
		        		  	continue;
		          
		    	if (vertices[0].vx > DBBMaxX) 
			    	if (vertices[1].vx > DBBMaxX) 
			   			if (vertices[2].vx > DBBMaxX) 
				          	continue;
			    
		    	if (vertices[0].vz < DBBMinZ) 
			  		if (vertices[1].vz < DBBMinZ) 
			   			if (vertices[2].vz < DBBMinZ) 
		          			continue;
				    
		    	if (vertices[0].vz > DBBMaxZ) 
			    	if (vertices[1].vz > DBBMaxZ) 
			   			if (vertices[2].vz > DBBMaxZ) 
			    			continue;

		    	if (vertices[0].vy > DBBMaxY) 
			    	if (vertices[1].vy > DBBMaxY) 
			    		if (vertices[2].vy > DBBMaxY) 
		          			continue;

		    }
		}
    
    	/* add object's world space coords to vertices */
		{
			int i = CollisionPolysPtr->NumberOfVertices;	
        	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

			           	
           	do
           	{	
			    polyVertexPtr->vx += objectPtr->ObWorld.vx;
    	        polyVertexPtr->vy += objectPtr->ObWorld.vy;
        	    polyVertexPtr->vz += objectPtr->ObWorld.vz;
				polyVertexPtr++;
        	}
			while(--i);
        }

        /* get the poly's normal */
    	GetPolygonNormal(CollisionPolysPtr);
       	if (needToRotate)
		{
			RotateVector(&CollisionPolysPtr->PolyNormal,&objectPtr->ObMat);
		}
		CollisionPolysPtr->ParentObject = objectPtr;

    	CollisionPolysPtr++;
    	NumberOfCollisionPolys++;
		/* ran out of space? */
		LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
        
        if (PolygonFlag & iflag_no_bfc)
        {
	    	CollisionPolysPtr->NumberOfVertices = (CollisionPolysPtr-1)->NumberOfVertices;
			if(CollisionPolysPtr->NumberOfVertices==3)
			{
				CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[2];
				CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[1];
				CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[0];
			}
			else
			{
				CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[3];
				CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[2];
				CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[1];
				CollisionPolysPtr->PolyPoint[3] = (CollisionPolysPtr-1)->PolyPoint[0];
			}

			CollisionPolysPtr->PolyNormal.vx = -(CollisionPolysPtr-1)->PolyNormal.vx;
			CollisionPolysPtr->PolyNormal.vy = -(CollisionPolysPtr-1)->PolyNormal.vy;
			CollisionPolysPtr->PolyNormal.vz = -(CollisionPolysPtr-1)->PolyNormal.vz;
			CollisionPolysPtr->ParentObject = objectPtr;

			CollisionPolysPtr++;
	    	NumberOfCollisionPolys++;
			/* ran out of space? */
			LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
        }        
	}
    					       
    return;
}	
static void TestShapeWithParticlesDynamicBoundingBox(DISPLAYBLOCK *objectPtr)
{
	int numberOfItems;
    int needToRotate = 0;

    /* KJL 10:58:22 24/11/98 - If the object is a static object rather than a module,
	we'll need to rotate the polygons into world-space */
   	if (objectPtr->ObStrategyBlock)
	{
		DYNAMICSBLOCK *dynPtr = objectPtr->ObStrategyBlock->DynPtr;

		if (dynPtr)
		{
			if (dynPtr->IsStatic)
			{
				needToRotate = 1;
			}
		}
	}
    
    
    /* okay, let's setup the shape's data and access the first poly */
	numberOfItems = SetupPolygonAccess(objectPtr);
    
    /* go through polys looking for those which intersect with the bounding box */
  	while(numberOfItems--)
	{
		AccessNextPolygon();

        if (PolygonFlag & iflag_notvis) continue;

		GetPolygonVertices(CollisionPolysPtr);
       	if (needToRotate)
		{
			int i = CollisionPolysPtr->NumberOfVertices;	
        	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

			           	
           	do
           	{	
				RotateVector(polyVertexPtr,&objectPtr->ObMat);
				polyVertexPtr++;
        	}
			while(--i);
        }

		{
			VECTORCH *vertices = CollisionPolysPtr->PolyPoint;
			if (CollisionPolysPtr->NumberOfVertices==4)
			{
		    	if (vertices[0].vy < DBBMinY)
		        	if (vertices[1].vy < DBBMinY) 
			      		if (vertices[2].vy < DBBMinY) 
			      			if (vertices[3].vy < DBBMinY) 
		          				continue;
				    
		    	if (vertices[0].vx < DBBMinX)
		    		if (vertices[1].vx < DBBMinX)
		    			if (vertices[2].vx < DBBMinX)
		    				if (vertices[3].vx < DBBMinX) 
		          				continue;
		          
		    	if (vertices[0].vx > DBBMaxX) 
			    	if (vertices[1].vx > DBBMaxX) 
			      		if (vertices[2].vx > DBBMaxX) 
			    			if (vertices[3].vx > DBBMaxX) 
		          				continue;
			    
		    	if (vertices[0].vz < DBBMinZ) 
			    	if (vertices[1].vz < DBBMinZ) 
			      		if (vertices[2].vz < DBBMinZ) 
			      			if (vertices[3].vz < DBBMinZ) 
		          				continue;
				    
		    	if (vertices[0].vz > DBBMaxZ) 
			    	if (vertices[1].vz > DBBMaxZ) 
			    		if (vertices[2].vz > DBBMaxZ) 
			    			if (vertices[3].vz > DBBMaxZ) 
			    				continue;

		    	if (vertices[0].vy > DBBMaxY) 
			    	if (vertices[1].vy > DBBMaxY) 
			    		if (vertices[2].vy > DBBMaxY) 
			    			if (vertices[3].vy > DBBMaxY) 
		        			  	continue;

		    }
		    else
			{
		    	if (vertices[0].vy < DBBMinY)
		        	if (vertices[1].vy < DBBMinY) 
			    		if (vertices[2].vy < DBBMinY) 
		          			continue;
			      
		    	if (vertices[0].vx < DBBMinX) 
			    	if (vertices[1].vx < DBBMinX) 
			    		if (vertices[2].vx < DBBMinX) 
		        		  	continue;
		          
		    	if (vertices[0].vx > DBBMaxX) 
			    	if (vertices[1].vx > DBBMaxX) 
			   			if (vertices[2].vx > DBBMaxX) 
				          	continue;
			    
		    	if (vertices[0].vz < DBBMinZ) 
			  		if (vertices[1].vz < DBBMinZ) 
			   			if (vertices[2].vz < DBBMinZ) 
		          			continue;
				    
		    	if (vertices[0].vz > DBBMaxZ) 
			    	if (vertices[1].vz > DBBMaxZ) 
			   			if (vertices[2].vz > DBBMaxZ) 
			    			continue;

		    	if (vertices[0].vy > DBBMaxY) 
			    	if (vertices[1].vy > DBBMaxY) 
			    		if (vertices[2].vy > DBBMaxY) 
		          			continue;

		    }
		}
    
    	/* add object's world space coords to vertices */
		{
			int i = CollisionPolysPtr->NumberOfVertices;	
        	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

           	do
           	{	
	            polyVertexPtr->vx += objectPtr->ObWorld.vx;
    	        polyVertexPtr->vy += objectPtr->ObWorld.vy;
        	    polyVertexPtr->vz += objectPtr->ObWorld.vz;
				polyVertexPtr++;
        	}
			while(--i);
        }

        /* get the poly's normal */
    	GetPolygonNormal(CollisionPolysPtr);
       	if (needToRotate)
		{
			RotateVector(&CollisionPolysPtr->PolyNormal,&objectPtr->ObMat);
		}
		CollisionPolysPtr->ParentObject = objectPtr;

    	CollisionPolysPtr++;
    	NumberOfCollisionPolys++;
		/* ran out of space? */
		LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
        
        if (PolygonFlag & iflag_no_bfc)
        {
	    	CollisionPolysPtr->NumberOfVertices = (CollisionPolysPtr-1)->NumberOfVertices;
			if(CollisionPolysPtr->NumberOfVertices==3)
			{
				CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[2];
				CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[1];
				CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[0];
			}
			else
			{
				CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[3];
				CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[2];
				CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[1];
				CollisionPolysPtr->PolyPoint[3] = (CollisionPolysPtr-1)->PolyPoint[0];
			}

			CollisionPolysPtr->PolyNormal.vx = -(CollisionPolysPtr-1)->PolyNormal.vx;
			CollisionPolysPtr->PolyNormal.vy = -(CollisionPolysPtr-1)->PolyNormal.vy;
			CollisionPolysPtr->PolyNormal.vz = -(CollisionPolysPtr-1)->PolyNormal.vz;
			CollisionPolysPtr->ParentObject = objectPtr;

			CollisionPolysPtr++;
	    	NumberOfCollisionPolys++;
			/* ran out of space? */
			LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
        }        
	}
    					       
    return;
}	

static void TestShapeWithStaticBoundingBox(DISPLAYBLOCK *objectPtr)
{
	int numberOfItems;
    int needToRotate = 0;

    /* KJL 10:58:22 24/11/98 - If the object is a static object rather than a module,
	we'll need to rotate the polygons into world-space */
   	if (objectPtr->ObStrategyBlock)
	{
		DYNAMICSBLOCK *dynPtr = objectPtr->ObStrategyBlock->DynPtr;

		if (dynPtr)
		{
			if (dynPtr->IsStatic)
			{
				needToRotate = 1;
			}
		}
	}
    
    /* okay, let's setup the shape's data and access the first poly */
	numberOfItems = SetupPolygonAccess(objectPtr);
    
    /* go through polys looking for those which intersect with the bounding box */
  	while(numberOfItems--)
	{
		AccessNextPolygon();
        
  //      if (PolygonFlag & iflag_notvis) continue;
		
		GetPolygonVertices(CollisionPolysPtr);
    	if (needToRotate)
		{
			int i = CollisionPolysPtr->NumberOfVertices;	
        	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

			           	
           	do
           	{	
				RotateVector(polyVertexPtr,&objectPtr->ObMat);
				polyVertexPtr++;
        	}
			while(--i);
        }

        if(IsPolygonWithinStaticBoundingBox(CollisionPolysPtr))
        {
        	/* add object's world space coords to vertices */
			{
				int i = CollisionPolysPtr->NumberOfVertices;	
            	VECTORCH *polyVertexPtr = CollisionPolysPtr->PolyPoint;

				do
	           	{	
		            polyVertexPtr->vx += objectPtr->ObWorld.vx;
	    	        polyVertexPtr->vy += objectPtr->ObWorld.vy;
	        	    polyVertexPtr->vz += objectPtr->ObWorld.vz;
					polyVertexPtr++;
            	}
				while(--i);		 
            }

            /* get the poly's normal */
        	GetPolygonNormal(CollisionPolysPtr);
	       	if (needToRotate)
			{
				RotateVector(&CollisionPolysPtr->PolyNormal,&objectPtr->ObMat);
			}

        	CollisionPolysPtr++;
        	NumberOfCollisionPolys++;

			/* ran out of space? */
			LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
	        if (PolygonFlag & iflag_no_bfc)
	        {
		    	CollisionPolysPtr->NumberOfVertices = (CollisionPolysPtr-1)->NumberOfVertices;
				if(CollisionPolysPtr->NumberOfVertices==3)
				{
					CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[2];
					CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[1];
					CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[0];
				}
				else
				{
					CollisionPolysPtr->PolyPoint[0] = (CollisionPolysPtr-1)->PolyPoint[3];
					CollisionPolysPtr->PolyPoint[1] = (CollisionPolysPtr-1)->PolyPoint[2];
					CollisionPolysPtr->PolyPoint[2] = (CollisionPolysPtr-1)->PolyPoint[1];
					CollisionPolysPtr->PolyPoint[3] = (CollisionPolysPtr-1)->PolyPoint[0];
				}

				CollisionPolysPtr->PolyNormal.vx = -(CollisionPolysPtr-1)->PolyNormal.vx;
				CollisionPolysPtr->PolyNormal.vy = -(CollisionPolysPtr-1)->PolyNormal.vy;
				CollisionPolysPtr->PolyNormal.vz = -(CollisionPolysPtr-1)->PolyNormal.vz;

				CollisionPolysPtr++;
		    	NumberOfCollisionPolys++;
				/* ran out of space? */
				LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
	        }        
        }
        
	}
    					       
    return;
}
static void TestObjectWithStaticBoundingBox(DISPLAYBLOCK *objectPtr)
{
    
	DYNAMICSBLOCK *dynPtr = objectPtr->ObStrategyBlock->DynPtr;
//	VECTORCH *objectPositionPtr = &(dynPtr->Position);
    /* if the bounding box does not intersect with the object at all just return */
	VECTORCH *objectVerticesPtr = dynPtr->ObjectVertices;

   	if (!( ( (SBBMaxX >= objectVerticesPtr[7].vx) && (SBBMinX <= objectVerticesPtr[0].vx) )
   	     &&( (SBBMaxY >= objectVerticesPtr[7].vy) && (SBBMinY <= objectVerticesPtr[0].vy) )
         &&( (SBBMaxZ >= objectVerticesPtr[7].vz) && (SBBMinZ <= objectVerticesPtr[0].vz) ) ))
       	return;
    
    /* okay, let's access the objects polys */
	{
		const int *vertexIndexPtr=&CuboidVertexList[0];
		int face=6;
		
		do
		{
			CollisionPolysPtr->NumberOfVertices = 4;
			CollisionPolysPtr->PolyPoint[0]=objectVerticesPtr[*vertexIndexPtr++];
			CollisionPolysPtr->PolyPoint[1]=objectVerticesPtr[*vertexIndexPtr++];
			CollisionPolysPtr->PolyPoint[2]=objectVerticesPtr[*vertexIndexPtr++];
			CollisionPolysPtr->PolyPoint[3]=objectVerticesPtr[*vertexIndexPtr++];

	        if(IsPolygonWithinStaticBoundingBox(CollisionPolysPtr))
			{
				if(dynPtr->DynamicsType == DYN_TYPE_CUBOID_COLLISIONS)
				{
					switch(face)
					{
						case 6: /* all points are on max y face */
						{
						    CollisionPolysPtr->PolyNormal.vx = dynPtr->OrientMat.mat21;
						    CollisionPolysPtr->PolyNormal.vy = dynPtr->OrientMat.mat22;
						    CollisionPolysPtr->PolyNormal.vz = dynPtr->OrientMat.mat23;
							break;
						}
						case 5: /* all points are on max z face */
						{
						    CollisionPolysPtr->PolyNormal.vx = dynPtr->OrientMat.mat31;
						    CollisionPolysPtr->PolyNormal.vy = dynPtr->OrientMat.mat32;
						    CollisionPolysPtr->PolyNormal.vz = dynPtr->OrientMat.mat33;
							break;
						}
						case 4: /* all points are on max x face */
						{
						    CollisionPolysPtr->PolyNormal.vx = dynPtr->OrientMat.mat11;
						    CollisionPolysPtr->PolyNormal.vy = dynPtr->OrientMat.mat12;
						    CollisionPolysPtr->PolyNormal.vz = dynPtr->OrientMat.mat13;
							break;
						}
						case 3: /* all points are on min z face */
						{
						    CollisionPolysPtr->PolyNormal.vx = -dynPtr->OrientMat.mat31;
						    CollisionPolysPtr->PolyNormal.vy = -dynPtr->OrientMat.mat32;
						    CollisionPolysPtr->PolyNormal.vz = -dynPtr->OrientMat.mat33;
							break;
						}
						case 2: /* all points are on min x face */
						{
						    CollisionPolysPtr->PolyNormal.vx = -dynPtr->OrientMat.mat11;
						    CollisionPolysPtr->PolyNormal.vy = -dynPtr->OrientMat.mat12;
						    CollisionPolysPtr->PolyNormal.vz = -dynPtr->OrientMat.mat13;
							break;
						}
						case 1: /* all points are on min y face */
						{
						    CollisionPolysPtr->PolyNormal.vx = -dynPtr->OrientMat.mat21;
						    CollisionPolysPtr->PolyNormal.vy = -dynPtr->OrientMat.mat22;
						    CollisionPolysPtr->PolyNormal.vz = -dynPtr->OrientMat.mat23;
							break;
						}
					}	
				}
				else
				{
					switch(face)
					{
						case 6: /* all points are on max y face */
						{
						    CollisionPolysPtr->PolyNormal.vx = 0;
						    CollisionPolysPtr->PolyNormal.vy = ONE_FIXED;
						    CollisionPolysPtr->PolyNormal.vz = 0;
							break;
						}
						case 5: /* all points are on max z face */
						{
						    CollisionPolysPtr->PolyNormal.vx = 0;
						    CollisionPolysPtr->PolyNormal.vy = 0;
						    CollisionPolysPtr->PolyNormal.vz = ONE_FIXED;
							break;
						}
						case 4: /* all points are on max x face */
						{
						    CollisionPolysPtr->PolyNormal.vx = ONE_FIXED;
						    CollisionPolysPtr->PolyNormal.vy = 0;
						    CollisionPolysPtr->PolyNormal.vz = 0;
							break;
						}
						case 3: /* all points are on min z face */
						{
						    CollisionPolysPtr->PolyNormal.vx = 0;
						    CollisionPolysPtr->PolyNormal.vy = 0;
						    CollisionPolysPtr->PolyNormal.vz = -ONE_FIXED;
							break;
						}
						case 2: /* all points are on min x face */
						{
						    CollisionPolysPtr->PolyNormal.vx = -ONE_FIXED;
						    CollisionPolysPtr->PolyNormal.vy = 0;
						    CollisionPolysPtr->PolyNormal.vz = 0;
							break;
						}
						case 1: /* all points are on min y face */
						{									   
						    CollisionPolysPtr->PolyNormal.vx = 0;
						    CollisionPolysPtr->PolyNormal.vy = -ONE_FIXED;
						    CollisionPolysPtr->PolyNormal.vz = 0;
							break;
						}
					}
				}
	        	CollisionPolysPtr++;
	        	NumberOfCollisionPolys++;

				/* ran out of space? */
				LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
			}
		}
		while(--face);
		
	}
    return;
}	
	

static int IsPolygonWithinDynamicBoundingBox(const struct ColPolyTag *polyPtr)
{
	VECTORCH *vertices = polyPtr->PolyPoint;

	if (polyPtr->NumberOfVertices==4)
	{
    	if (vertices[0].vy < DBBMinY)
        	if (vertices[1].vy < DBBMinY) 
	      		if (vertices[2].vy < DBBMinY) 
	      			if (vertices[3].vy < DBBMinY) 
          				return 0;
		    
    	if (vertices[0].vx < DBBMinX)
    		if (vertices[1].vx < DBBMinX)
    			if (vertices[2].vx < DBBMinX)
    				if (vertices[3].vx < DBBMinX) 
          				return 0;
          
    	if (vertices[0].vx > DBBMaxX) 
	    	if (vertices[1].vx > DBBMaxX) 
	      		if (vertices[2].vx > DBBMaxX) 
	    			if (vertices[3].vx > DBBMaxX) 
          				return 0;
	    
    	if (vertices[0].vz < DBBMinZ) 
	    	if (vertices[1].vz < DBBMinZ) 
	      		if (vertices[2].vz < DBBMinZ) 
	      			if (vertices[3].vz < DBBMinZ) 
          				return 0;
		    
    	if (vertices[0].vz > DBBMaxZ) 
	    	if (vertices[1].vz > DBBMaxZ) 
	    		if (vertices[2].vz > DBBMaxZ) 
	    			if (vertices[3].vz > DBBMaxZ) 
	    				return 0;

    	if (vertices[0].vy > DBBMaxY) 
	    	if (vertices[1].vy > DBBMaxY) 
	    		if (vertices[2].vy > DBBMaxY) 
	    			if (vertices[3].vy > DBBMaxY) 
        			  	return 0;

    }
    else
	{
    	if (vertices[0].vy < DBBMinY)
        	if (vertices[1].vy < DBBMinY) 
	    		if (vertices[2].vy < DBBMinY) 
          			return 0;
	      
    	if (vertices[0].vx < DBBMinX) 
	    	if (vertices[1].vx < DBBMinX) 
	    		if (vertices[2].vx < DBBMinX) 
        		  	return 0;
          
    	if (vertices[0].vx > DBBMaxX) 
	    	if (vertices[1].vx > DBBMaxX) 
	   			if (vertices[2].vx > DBBMaxX) 
		          	return 0;
	    
    	if (vertices[0].vz < DBBMinZ) 
	  		if (vertices[1].vz < DBBMinZ) 
	   			if (vertices[2].vz < DBBMinZ) 
          			return 0;
		    
    	if (vertices[0].vz > DBBMaxZ) 
	    	if (vertices[1].vz > DBBMaxZ) 
	   			if (vertices[2].vz > DBBMaxZ) 
	    			return 0;

    	if (vertices[0].vy > DBBMaxY) 
	    	if (vertices[1].vy > DBBMaxY) 
	    		if (vertices[2].vy > DBBMaxY) 
          			return 0;

    }
    
    
    return 1;
} 

static int IsPolygonWithinStaticBoundingBox(const struct ColPolyTag *polyPtr)
{
	VECTORCH *vertices = polyPtr->PolyPoint;

	if (polyPtr->NumberOfVertices==4)
	{
    	if (vertices[0].vy < SBBMinY)
        	if (vertices[1].vy < SBBMinY) 
	      		if (vertices[2].vy < SBBMinY) 
	      			if (vertices[3].vy < SBBMinY) 
          				return 0;
		    
    	if (vertices[0].vx < SBBMinX)
    		if (vertices[1].vx < SBBMinX)
    			if (vertices[2].vx < SBBMinX)
    				if (vertices[3].vx < SBBMinX) 
          				return 0;
          
    	if (vertices[0].vx > SBBMaxX) 
	    	if (vertices[1].vx > SBBMaxX) 
	      		if (vertices[2].vx > SBBMaxX) 
	    			if (vertices[3].vx > SBBMaxX) 
          				return 0;
	    
    	if (vertices[0].vz < SBBMinZ) 
	    	if (vertices[1].vz < SBBMinZ) 
	      		if (vertices[2].vz < SBBMinZ) 
	      			if (vertices[3].vz < SBBMinZ) 
          				return 0;
		    
    	if (vertices[0].vz > SBBMaxZ) 
	    	if (vertices[1].vz > SBBMaxZ) 
	    		if (vertices[2].vz > SBBMaxZ) 
	    			if (vertices[3].vz > SBBMaxZ) 
	    				return 0;

    	if (vertices[0].vy > SBBMaxY) 
	    	if (vertices[1].vy > SBBMaxY) 
	    		if (vertices[2].vy > SBBMaxY) 
	    			if (vertices[3].vy > SBBMaxY) 
        			  	return 0;

    }
    else
	{
    	if (vertices[0].vy < SBBMinY)
        	if (vertices[1].vy < SBBMinY) 
	    		if (vertices[2].vy < SBBMinY) 
          			return 0;
	      
    	if (vertices[0].vx < SBBMinX) 
	    	if (vertices[1].vx < SBBMinX) 
	    		if (vertices[2].vx < SBBMinX) 
        		  	return 0;
          
    	if (vertices[0].vx > SBBMaxX) 
	    	if (vertices[1].vx > SBBMaxX) 
	   			if (vertices[2].vx > SBBMaxX) 
		          	return 0;
	    
    	if (vertices[0].vz < SBBMinZ) 
	  		if (vertices[1].vz < SBBMinZ) 
	   			if (vertices[2].vz < SBBMinZ) 
          			return 0;
		    
    	if (vertices[0].vz > SBBMaxZ) 
	    	if (vertices[1].vz > SBBMaxZ) 
	   			if (vertices[2].vz > SBBMaxZ) 
	    			return 0;

    	if (vertices[0].vy > SBBMaxY) 
	    	if (vertices[1].vy > SBBMaxY) 
	    		if (vertices[2].vy > SBBMaxY) 
          			return 0;

    }
    
    
    return 1;
} 










/*KJL****************************************************************************
* A function which takes a plane's normal and decides which axis to ignore when *
* projecting points on the plane into a 2D space.                               *
****************************************************************************KJL*/
static int AxisToIgnore(VECTORCH *normal)
{							  
	VECTORCH absNormal = *normal;
	if (absNormal.vx<0) absNormal.vx=-absNormal.vx;
	if (absNormal.vy<0) absNormal.vy=-absNormal.vy;
	if (absNormal.vz<0) absNormal.vz=-absNormal.vz;

	if (absNormal.vx > absNormal.vy)
	{
		if (absNormal.vx > absNormal.vz)
		{
			return ix;
		}
		else
		{
			return iz;
		}
	}
	else
	{
		if (absNormal.vy > absNormal.vz)
		{
			return iy;
		}
		else
		{
			return iz;
		}
	}
}

#if 0
static void TestForValidMovement(STRATEGYBLOCK *sbPtr)
{
	#if 1
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	/* I'm a platform lift - leave me alone */
	if(dynPtr->OnlyCollideWithObjects)
		return;

	if(RelocationIsValid(sbPtr))
	{
		/* movement ok */
	}
	else
	{
		/* cancel movement */
		//PrintDebuggingText("Relocate!");
		dynPtr->Position=dynPtr->PrevPosition;
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
	}
	#endif
}   
#endif

static int RelocateSphere(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    VECTORCH objectPosition = dynPtr->Position;
    VECTORCH objectVertices[8];

    int polysLeft;
    struct ColPolyTag *polyPtr;
	
    polysLeft = NumberOfCollisionPolys;
   	polyPtr = CollisionPolysArray;
	
	{
    	int vertexNum=8;
		do
        {
			vertexNum--;
			objectVertices[vertexNum] = dynPtr->ObjectVertices[vertexNum];			
	    }
        while(vertexNum);
	}
	    
	/* first pass relocate */
    while(polysLeft)
	{
	   	VECTORCH planeNormal = polyPtr->PolyNormal;
        VECTORCH pointOnPlane = polyPtr->PolyPoint[0];
        int distance;

        {
        	VECTORCH planeToObject;

			planeToObject.vx = objectPosition.vx - pointOnPlane.vx;
			planeToObject.vy = objectPosition.vy - pointOnPlane.vy;
			planeToObject.vz = objectPosition.vz - pointOnPlane.vz;
			  
    	    distance = dynPtr->CollisionRadius - Dot(&planeToObject,&planeNormal);
        }

		if (distance>0 && DoesPolygonIntersectNRBB(polyPtr,objectVertices))
		{
			VECTORCH displacement;

			displacement.vx = MUL_FIXED(planeNormal.vx,distance+RELOCATION_GRANULARITY);
			displacement.vy = MUL_FIXED(planeNormal.vy,distance+RELOCATION_GRANULARITY);
			displacement.vz = MUL_FIXED(planeNormal.vz,distance+RELOCATION_GRANULARITY);
			
			AddVectorToVector(displacement,objectPosition);
			{
		    	int vertexNum=8;
				VECTORCH *vertexPtr = objectVertices;

		        do
		        {
					vertexPtr->vx += displacement.vx;
					vertexPtr->vy += displacement.vy;
					vertexPtr->vz += displacement.vz;
					vertexPtr++;
			    }
		        while(--vertexNum);
			}

		}

        polyPtr++;
		polysLeft--;
	}

	/* did we move at all? */
	if ( (objectPosition.vx == dynPtr->Position.vx)
	   &&(objectPosition.vy == dynPtr->Position.vy)
	   &&(objectPosition.vz == dynPtr->Position.vz) )
	   return 1;
 
 	/* pass test if okay */
	polysLeft = NumberOfCollisionPolys;
	polyPtr = CollisionPolysArray;
    
	{
		int stillIntersectingSomething = 0;
		while(polysLeft)
		{
			{
			   	VECTORCH planeNormal = polyPtr->PolyNormal;
	    	    VECTORCH pointOnPlane = polyPtr->PolyPoint[0];
	        	int distance;

		        {
		        	VECTORCH planeToObject;

					planeToObject.vx = objectPosition.vx - pointOnPlane.vx;
					planeToObject.vy = objectPosition.vy - pointOnPlane.vy;
					planeToObject.vz = objectPosition.vz - pointOnPlane.vz;
					  
		    	    distance = dynPtr->CollisionRadius - Dot(&planeToObject,&planeNormal);
		        }

				if (distance>0 && DoesPolygonIntersectNRBB(polyPtr,objectVertices))
				{
					stillIntersectingSomething = 1;
				}
	        }
	        polyPtr++;
			polysLeft--;
		}
		
		if (stillIntersectingSomething)
		{
			return 0;
		}
		else
		{
	    	int vertexNum=8;
			do
	        {
				vertexNum--;
				dynPtr->ObjectVertices[vertexNum] = objectVertices[vertexNum];			
		    }
	        while(vertexNum);
			dynPtr->Position = objectPosition;
			return 1;
		}
	}
}

static int RelocateNRBB(STRATEGYBLOCK *sbPtr)
{
	int noProblems = 1;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
    VECTORCH objectPosition = dynPtr->Position;
    VECTORCH objectVertices[8];
    int polysLeft;
    struct ColPolyTag *polyPtr;

	polysLeft = NumberOfCollisionPolys;
    polyPtr = CollisionPolysArray;
    
	{
    	int vertexNum=8;
		do
        {
			vertexNum--;
			objectVertices[vertexNum] = dynPtr->ObjectVertices[vertexNum];			
	    }
        while(vertexNum);
	}

    while(polysLeft)
	{
	   	VECTORCH planeNormal = polyPtr->PolyNormal;
        VECTORCH pointOnPlane = polyPtr->PolyPoint[0];
        
        {
        	VECTORCH planeToObject;

			planeToObject.vx = objectPosition.vx - pointOnPlane.vx;
			planeToObject.vy = objectPosition.vy - pointOnPlane.vy;
			planeToObject.vz = objectPosition.vz - pointOnPlane.vz;
			  
    	    if (DotProduct(&planeToObject,&planeNormal) < 0)
    	    {
		        polyPtr++;
				polysLeft--;
    	    	continue;
			} 	
        }
        {
			int greatestDistance;

	    	{
	    		VECTORCH vertex = objectVertices[WhichNRBBVertex(dynPtr,&planeNormal)];
				vertex.vx -= pointOnPlane.vx;
				vertex.vy -= pointOnPlane.vy;
				vertex.vz -= pointOnPlane.vz;
				greatestDistance = -DotProduct(&vertex,&planeNormal);
			}

			if ((greatestDistance>0) && DoesPolygonIntersectNRBB(polyPtr,objectVertices))
			{
				#if 0
				VECTORCH displacement;

				displacement.vx = MUL_FIXED(planeNormal.vx,greatestDistance+RELOCATION_GRANULARITY);
				displacement.vy = MUL_FIXED(planeNormal.vy,greatestDistance+RELOCATION_GRANULARITY);
				displacement.vz = MUL_FIXED(planeNormal.vz,greatestDistance+RELOCATION_GRANULARITY);
				
				AddVectorToVector(displacement,objectPosition);
		    	{
		        	int vertexNum=8;
		    		VECTORCH *vertexPtr = objectVertices;

		            do
		            {
						vertexPtr->vx += displacement.vx;
						vertexPtr->vy += displacement.vy;
						vertexPtr->vz += displacement.vz;
						vertexPtr++;
				    }
		            while(--vertexNum);
				}
				#endif
				/* create a report about the collision */
				{
					COLLISIONREPORT *reportPtr = AllocateCollisionReport(dynPtr);
					
					if (reportPtr)
					{
						reportPtr->ObstacleSBPtr = 0;//obstacleSBPtr;
						reportPtr->ObstacleNormal = planeNormal;

						reportPtr->ObstaclePoint = pointOnPlane;
					   //	reportPtr->ObstaclePoint.vx = 0x7fffffff;
					   //	reportPtr->ObstaclePoint.vy = 0x7fffffff;
					   //	reportPtr->ObstaclePoint.vz = 0x7fffffff;
					}
				}
	   			noProblems = 0;
			
			}
		}
        polyPtr++;
		polysLeft--;
	}
	#if 0
	/* did we move at all? */
	if ( (objectPosition.vx == dynPtr->Position.vx)
	   &&(objectPosition.vy == dynPtr->Position.vy)
	   &&(objectPosition.vz == dynPtr->Position.vz) )
	   return noProblems;


 	/* pass test if okay */
    polysLeft = NumberOfCollisionPolys;
    polyPtr = CollisionPolysArray;

	{
		while(polysLeft)
		{
	        if(DoesPolygonIntersectNRBB(polyPtr,objectVertices))
	        {
			   	VECTORCH planeNormal = polyPtr->PolyNormal;
		        VECTORCH pointOnPlane = polyPtr->PolyPoint[0];
				int greatestDistance;

		    	{
		    		VECTORCH vertex = objectVertices[WhichNRBBVertex(dynPtr,&planeNormal)];
					vertex.vx -= pointOnPlane.vx;
					vertex.vy -= pointOnPlane.vy;
					vertex.vz -= pointOnPlane.vz;
					greatestDistance = -DotProduct(&vertex,&planeNormal);
				}

				if (greatestDistance>0)
				{
					/* still intersecting something */
					return noProblems;
				}
	        }
	        polyPtr++;
			polysLeft--;
		}
		
		{
	    	int vertexNum=8;
			do
	        {
				vertexNum--;
				dynPtr->ObjectVertices[vertexNum] = objectVertices[vertexNum];			
		    }
	        while(vertexNum);
			dynPtr->Position = objectPosition;
			return 1;
		}
	}
	#endif
	if (!noProblems) PrintDebuggingText("RECOMMEND RELOCATE\n");
	return noProblems;
}


static int DoesPolygonIntersectNRBB(struct ColPolyTag *polyPtr,VECTORCH *objectVertices)
{
	VECTORCH *minVertexPtr = &objectVertices[7];
	VECTORCH *vertices = polyPtr->PolyPoint;

  	/* trivial rejection tests */
	if (polyPtr->NumberOfVertices==4)
	{
    	if (vertices[0].vx < minVertexPtr->vx)
    		if (vertices[1].vx < minVertexPtr->vx)
    			if (vertices[2].vx < minVertexPtr->vx)
    				if (vertices[3].vx < minVertexPtr->vx) 
          				return 0;
          
    	if (vertices[0].vz < minVertexPtr->vz) 
	    	if (vertices[1].vz < minVertexPtr->vz) 
	      		if (vertices[2].vz < minVertexPtr->vz) 
	      			if (vertices[3].vz < minVertexPtr->vz) 
          				return 0;
	    
    	if (vertices[0].vy < minVertexPtr->vy)
        	if (vertices[1].vy < minVertexPtr->vy) 
	      		if (vertices[2].vy < minVertexPtr->vy) 
	      			if (vertices[3].vy < minVertexPtr->vy) 
          				return 0;
		    
		
    	if (vertices[0].vx > objectVertices->vx) 
	    	if (vertices[1].vx > objectVertices->vx) 
	      		if (vertices[2].vx > objectVertices->vx) 
	    			if (vertices[3].vx > objectVertices->vx) 
          				return 0;
    	
    	if (vertices[0].vz > objectVertices->vz) 
	    	if (vertices[1].vz > objectVertices->vz) 
	    		if (vertices[2].vz > objectVertices->vz) 
	    			if (vertices[3].vz > objectVertices->vz) 
	    				return 0;

    	if (vertices[0].vy > objectVertices->vy) 
	    	if (vertices[1].vy > objectVertices->vy) 
	    		if (vertices[2].vy > objectVertices->vy) 
	    			if (vertices[3].vy > objectVertices->vy) 
        			  	return 0;

    }
    else
	{
    	if (vertices[0].vx < minVertexPtr->vx)
    		if (vertices[1].vx < minVertexPtr->vx)
    			if (vertices[2].vx < minVertexPtr->vx)
          				return 0;
          
    	if (vertices[0].vz < minVertexPtr->vz) 
	    	if (vertices[1].vz < minVertexPtr->vz) 
	      		if (vertices[2].vz < minVertexPtr->vz) 
          				return 0;
	    
    	if (vertices[0].vy < minVertexPtr->vy)
        	if (vertices[1].vy < minVertexPtr->vy) 
	      		if (vertices[2].vy < minVertexPtr->vy) 
          				return 0;
		    
		
    	if (vertices[0].vx > objectVertices->vx) 
	    	if (vertices[1].vx > objectVertices->vx) 
	      		if (vertices[2].vx > objectVertices->vx) 
          				return 0;
    	
    	if (vertices[0].vz > objectVertices->vz) 
	    	if (vertices[1].vz > objectVertices->vz) 
	    		if (vertices[2].vz > objectVertices->vz) 
	    				return 0;

    	if (vertices[0].vy > objectVertices->vy) 
	    	if (vertices[1].vy > objectVertices->vy) 
	    		if (vertices[2].vy > objectVertices->vy) 
        			  	return 0;
    }
    
   	/* are any of the poly's vertices inside the object's bounding box? */
	{
    	if (vertices[0].vy >= minVertexPtr->vy)
			if (vertices[0].vy <= objectVertices->vy) 
    			if (vertices[0].vx >= minVertexPtr->vx)
		  			if (vertices[0].vx <= objectVertices->vx) 
	    				if (vertices[0].vz >= minVertexPtr->vz) 
				    		if (vertices[0].vz <= objectVertices->vz) 
       								return 1;
    	if (vertices[1].vy >= minVertexPtr->vy)
			if (vertices[1].vy <= objectVertices->vy) 
    			if (vertices[1].vx >= minVertexPtr->vx)
		  			if (vertices[1].vx <= objectVertices->vx) 
	    				if (vertices[1].vz >= minVertexPtr->vz) 
				    		if (vertices[1].vz <= objectVertices->vz) 
       								return 1;
    	if (vertices[2].vy >= minVertexPtr->vy)
			if (vertices[2].vy <= objectVertices->vy) 
    			if (vertices[2].vx >= minVertexPtr->vx)
		  			if (vertices[2].vx <= objectVertices->vx) 
	    				if (vertices[2].vz >= minVertexPtr->vz) 
				    		if (vertices[2].vz <= objectVertices->vz) 
       								return 1;
    }
	if (polyPtr->NumberOfVertices==4)
	{
    	if (vertices[3].vy >= minVertexPtr->vy)
			if (vertices[3].vy <= objectVertices->vy) 
    			if (vertices[3].vx >= minVertexPtr->vx)
		  			if (vertices[3].vx <= objectVertices->vx) 
	    				if (vertices[3].vz >= minVertexPtr->vz) 
				    		if (vertices[3].vz <= objectVertices->vz) 
       								return 1;
    }

	/* okay, it's not that simple then. Let's see if any of the poly's edges
	   intersect the objects bounding box */
	{
	    int vertexA = polyPtr->NumberOfVertices;
		do
		{
			VECTORCH alpha,beta;
			int vertexB;
			vertexA--;
			vertexB = (vertexA+1)%(polyPtr->NumberOfVertices);
		
			alpha = vertices[vertexA];
			beta.vx = vertices[vertexB].vx - alpha.vx;
			beta.vy = vertices[vertexB].vy - alpha.vy;
			beta.vz = vertices[vertexB].vz - alpha.vz;

			/* edge is the line segment 'alpha + lambda*beta' where lambda is between 0 and 1 */
			if (beta.vy!=0)
			{
				{
					/* box edge 1:  y=minVertexPtr->vy; normal is (0,-1,0) */
				   	int lambda;
									
					f2i(lambda,(float)(minVertexPtr->vy - alpha.vy)/(float)beta.vy*65536.0f);

					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionX = alpha.vx + MUL_FIXED(lambda,beta.vx);
						if (intersectionX >= minVertexPtr->vx && intersectionX <= objectVertices->vx)
						{
							int intersectionZ = alpha.vz + MUL_FIXED(lambda,beta.vz);
							if (intersectionZ >= minVertexPtr->vz && intersectionZ <= objectVertices->vz)
								return 1;
						}
					}
				}
				{
					/* box edge 2:  y=objectVertices->vy; normal is (0,1,0) */
					int lambda;// = DIV_FIXED(objectVertices->vy - (alpha.vy),(beta.vy) );
					f2i(lambda,(float)(objectVertices->vy - alpha.vy)/(float)beta.vy*65536.0f);
					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionX = alpha.vx + MUL_FIXED(lambda,beta.vx);
						if (intersectionX >= minVertexPtr->vx && intersectionX <= objectVertices->vx)
						{
							int intersectionZ = alpha.vz + MUL_FIXED(lambda,beta.vz);
							if (intersectionZ >= minVertexPtr->vz && intersectionZ <= objectVertices->vz)
								return 1;
						}
					}
				}
			}
			if (beta.vx!=0)
			{
				{
					/* box edge 3:  x=minVertexPtr->vx; normal is (-1,0,0) */
					int lambda;// = DIV_FIXED(minVertexPtr->vx - (alpha.vx),(beta.vx) );
					f2i(lambda,(float)(minVertexPtr->vx - alpha.vx)/(float)beta.vx*65536.0f);
					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionY = alpha.vy + MUL_FIXED(lambda,beta.vy);
						if (intersectionY >= minVertexPtr->vy && intersectionY <= objectVertices->vy)
						{
							int intersectionZ = alpha.vz + MUL_FIXED(lambda,beta.vz);
							if (intersectionZ >= minVertexPtr->vz && intersectionZ <= objectVertices->vz)
								return 1;
						}
					}
				}
				{
					/* box edge 4:  x=objectVertices->vx; normal is (1,0,0) */
					int lambda;// = DIV_FIXED(objectVertices->vx - (alpha.vx),(beta.vx) );
					f2i(lambda,(float)(objectVertices->vx - alpha.vx)/(float)beta.vx*65536.0f);
					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionY = alpha.vy + MUL_FIXED(lambda,beta.vy);
						if (intersectionY >= minVertexPtr->vy && intersectionY <= objectVertices->vy)
						{
							int intersectionZ = alpha.vz + MUL_FIXED(lambda,beta.vz);
							if (intersectionZ >= minVertexPtr->vz && intersectionZ <= objectVertices->vz)
								return 1;
						}
					}
				}
			}
			if (beta.vz!=0)
			{
				{
					/* box edge 5:  z=minVertexPtr->vz; normal is (0,0,-1) */
					int lambda;// = DIV_FIXED(minVertexPtr->vz - (alpha.vz),(beta.vz));
					f2i(lambda,(float)(minVertexPtr->vz - alpha.vz)/(float)beta.vz*65536.0f);
					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionY = alpha.vy + MUL_FIXED(lambda,beta.vy);
						if (intersectionY >= minVertexPtr->vy && intersectionY <= objectVertices->vy)
						{
							int intersectionX = alpha.vx + MUL_FIXED(lambda,beta.vx);
							if (intersectionX >= minVertexPtr->vx && intersectionX <= objectVertices->vx)
								return 1;
						}
					}
				}
				{
					/* box edge 6:  z=objectVertices->vz; normal is (0,0,1) */
					int lambda;// = DIV_FIXED(objectVertices->vz - (alpha.vz),(beta.vz));
					f2i(lambda,(float)(objectVertices->vz - alpha.vz)/(float)beta.vz*65536.0f);
					/* eliminate the divides? */
					if (lambda>=0 && lambda <= 65536)
					{
						int intersectionY = alpha.vy + MUL_FIXED(lambda,beta.vy);
						if (intersectionY >= minVertexPtr->vy && intersectionY <= objectVertices->vy)
						{
							int intersectionX = alpha.vx + MUL_FIXED(lambda,beta.vx);
							if (intersectionX >= minVertexPtr->vx && intersectionX <= objectVertices->vx)
								return 1;
						}
					}
				}
			}

		}
		while(vertexA);
	}
	/* Still here? Damn. Okay, we'll have to check to see if a cuboid's diagonal intersect the polygon */
	{
		VECTORCH alpha,beta;
		int dottedNormals;
		int lambda;

		if ((polyPtr->PolyNormal).vx > 0)
		{
			alpha.vx = minVertexPtr->vx;
			beta.vx = objectVertices->vx - alpha.vx;
		}
		else
		{
			alpha.vx = objectVertices->vx;
			beta.vx = minVertexPtr->vx - alpha.vx;
		}

		if ((polyPtr->PolyNormal).vy > 0)
		{
			alpha.vy = minVertexPtr->vy;
			beta.vy = objectVertices->vy - alpha.vy;
		}
		else
		{
			alpha.vy = objectVertices->vy;
			beta.vy = minVertexPtr->vy - alpha.vy;
		}

		if ((polyPtr->PolyNormal).vz > 0)
		{
			alpha.vz = minVertexPtr->vz;
			beta.vz = objectVertices->vz - alpha.vz;
		}
		else
		{
			alpha.vz = objectVertices->vz;
			beta.vz = minVertexPtr->vz - alpha.vz;
		}

		dottedNormals = DotProduct(&(polyPtr->PolyNormal),&beta);
		#if 1//debug
		if (!dottedNormals)
		{
			#if 0
			char buffer[200];
			sprintf(buffer,"POLY NORMAL IS %d %d %d\n",(polyPtr->PolyNormal).vx,(polyPtr->PolyNormal).vy,(polyPtr->PolyNormal).vz);
			NewOnScreenMessage(buffer);
			sprintf(buffer,"POLY NO OF VERTICES %d\n",(polyPtr->NumberOfVertices));
			NewOnScreenMessage(buffer);
			sprintf(buffer,"POLY POINT IS %d %d %d\n",(polyPtr->PolyPoint[0]).vx,(polyPtr->PolyPoint[0]).vy,(polyPtr->PolyPoint[0]).vz);
			NewOnScreenMessage(buffer);
			#endif
			LOGDXFMT(( "POLY NORMAL IS %d %d %d\n",(polyPtr->PolyNormal).vx,(polyPtr->PolyNormal).vy,(polyPtr->PolyNormal).vz));
			LOGDXFMT(( "POLY NO OF VERTICES %d\n",polyPtr->NumberOfVertices));
			LOGDXFMT(( "POLY POINT IS %d %d %d\n",(polyPtr->PolyPoint[0]).vx,(polyPtr->PolyPoint[0]).vy,(polyPtr->PolyPoint[0]).vz ));
			LOCALASSERT("Found normal which may be incorrect"==0);
			return 0;
		}
		#endif
		{
			VECTORCH cornerToPlane;
			cornerToPlane.vx = vertices[0].vx - alpha.vx;
			cornerToPlane.vy = vertices[0].vy - alpha.vy;
			cornerToPlane.vz = vertices[0].vz - alpha.vz;
			lambda = DIV_FIXED
			(
				DotProduct(&(polyPtr->PolyNormal),&cornerToPlane),
				dottedNormals
			);
	  	}

		if (lambda >= 0 && lambda <= 65536)
		{
			int axis1,axis2;
			/* decide which 2d plane to project onto */
			{
				VECTORCH absNormal = (polyPtr->PolyNormal);
				if (absNormal.vx<0) absNormal.vx=-absNormal.vx;
				if (absNormal.vy<0) absNormal.vy=-absNormal.vy;
				if (absNormal.vz<0) absNormal.vz=-absNormal.vz;

				if (absNormal.vx > absNormal.vy)
				{
					if (absNormal.vx > absNormal.vz)
					{
						axis1=iy;
						axis2=iz;
					}
					else
					{
						axis1=ix;
						axis2=iy;
					}
				}
				else
				{
					if (absNormal.vy > absNormal.vz)
					{
						axis1=ix;
						axis2=iz;
					}
					else
					{
						axis1=ix;
						axis2=iy;
					}
				}
			}


			{
				int projectedPolyVertex[20];
				int projectedPointOnPlane[2];

				projectedPointOnPlane[0]=*((int*)&alpha+axis1) + MUL_FIXED(lambda,*((int*)&beta+axis1));
			 	projectedPointOnPlane[1]=*((int*)&alpha+axis2) + MUL_FIXED(lambda,*((int*)&beta+axis2));

			 	{
			 		VECTORCH *vertexPtr = &vertices[0];
			 		int *projectedVertexPtr= &projectedPolyVertex[0];
					int noOfVertices = polyPtr->NumberOfVertices;

					do
					{
			 			*projectedVertexPtr++ = *((int*)vertexPtr + axis1);
			 			*projectedVertexPtr++ = *((int*)vertexPtr + axis2);

		 	 		   	vertexPtr++;
			 		}
	                while(--noOfVertices);

			 	}


				if (PointInPolygon(&projectedPointOnPlane[0],&projectedPolyVertex[0],polyPtr->NumberOfVertices,2))
					return 1;
			}
		}
	}
	/* after all that we can be sure that the polygon isn't intersecting the object */
    return 0;
} 


static int WhichNRBBVertex(DYNAMICSBLOCK *dynPtr, VECTORCH *normalPtr)
{
	VECTORCH dir;
	
	dir.vx = -normalPtr->vx;
	dir.vy = -normalPtr->vy;
	dir.vz = -normalPtr->vz;
	
	if (dir.vx>=0) 
	{
		if (dir.vy>=0)
		{
			if (dir.vz>=0)
			{
				/* +ve x +ve y +ve z */
				return 0;
			}
			else
			{
				/* +ve x +ve y -ve z */
				return 1;
			} 
		}
		else
		{
			if (dir.vz>=0)
			{
				/* +ve x -ve y +ve z */
				return 2;
			}
			else
			{
				/* +ve x -ve y -ve z */
				return 3;
			} 
		} 
	}
	else
	{
		if (dir.vy>=0)
		{
			if (dir.vz>=0)
			{
				/* -ve x +ve y +ve z */
				return 4;
			}
			else
			{
				/* -ve x +ve y -ve z */
				return 5;
			} 
		}
		else
		{
			if (dir.vz>=0)
			{
				/* -ve x -ve y +ve z */
				return 6;
			}
			else
			{
				/* -ve x -ve y -ve z */
				return 7;
			} 
		} 
	}
}







static signed int DistanceMovedBeforeSphereHitsPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int sphereToPolyDist;

	LOCALASSERT(polyPtr);

    dottedNormals = -DotProduct(&DirectionOfTravel,&(polyPtr->PolyNormal));

	/* reject if polygon does not face against direction of sphere's travel */
	if (dottedNormals<=0)
	{
		return -1; 
	}

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
		VECTORCH sphereToPoly;

		sphereToPoly.vx = dynPtr->Position.vx - polyPtr->PolyPoint[0].vx;
		sphereToPoly.vy = dynPtr->Position.vy - polyPtr->PolyPoint[0].vy;
		sphereToPoly.vz = dynPtr->Position.vz - polyPtr->PolyPoint[0].vz;

	    sphereToPolyDist = DotProduct(&sphereToPoly,&(polyPtr->PolyNormal))-dynPtr->CollisionRadius;
	}
	/* reject if polygon is 'behind' sphere */
	if (sphereToPolyDist<0)
	{
		#if 0
		if (sphereToPolyDist>-dynPtr->CollisionRadius)
		{
			VECTORCH projectedPosition;
			
		   	projectedPosition.vx = dynPtr->Position.vx + MUL_FIXED(sphereToPolyDist,polyPtr->PolyNormal.vx);
		   	projectedPosition.vy = dynPtr->Position.vy + MUL_FIXED(sphereToPolyDist,polyPtr->PolyNormal.vy);
		   	projectedPosition.vz = dynPtr->Position.vz + MUL_FIXED(sphereToPolyDist,polyPtr->PolyNormal.vz);

		 	if (DoesSphereProjectOntoPoly(dynPtr, polyPtr, &projectedPosition))
			{
		  		textprint("RELOCATION CASE\n");
			}
		}
		#endif
		return -2;
	}
	
  	/* calculate distance along direction of travel */
					  
	sphereToPolyDist = DIV_FIXED(sphereToPolyDist,dottedNormals);

	if (sphereToPolyDist>=distanceToMove)
	{
		return -4;
	}	

	/* test if sphere's projected path intersect polygon */
	{
		VECTORCH projectedPosition;
	   	projectedPosition.vx = dynPtr->Position.vx + MUL_FIXED(DirectionOfTravel.vx, sphereToPolyDist) - MUL_FIXED(dynPtr->CollisionRadius,polyPtr->PolyNormal.vx);
	   	projectedPosition.vy = dynPtr->Position.vy + MUL_FIXED(DirectionOfTravel.vy, sphereToPolyDist) - MUL_FIXED(dynPtr->CollisionRadius,polyPtr->PolyNormal.vy);
	   	projectedPosition.vz = dynPtr->Position.vz + MUL_FIXED(DirectionOfTravel.vz, sphereToPolyDist) - MUL_FIXED(dynPtr->CollisionRadius,polyPtr->PolyNormal.vz);


		if (SphereProjectOntoPoly(dynPtr, polyPtr, &projectedPosition))
		{
			return sphereToPolyDist;
		}
	}
	/* polygon not in way of sphere */
	return -5;
}


static int SphereProjectOntoPoly(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, VECTORCH *projectedPosition)
{
	/* decide which 2d plane to project onto */
	int axisToIgnore = AxisToIgnore(&polyPtr->PolyNormal);
    
    if (axisToIgnore!=ix)
    {
    	int polyMax,polyMin;
        
    	/* search x-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vx;
			do
			{
	        	int x;
				vertexNum--;
	        	x = polyPtr->PolyPoint[vertexNum].vx;
	        	if (x > polyMax) polyMax = x;
	        	else if (x < polyMin) polyMin = x;
	        }
            while(vertexNum);
		}        
        
        /* test to see if object & polygon overlap */
        if (projectedPosition->vx+dynPtr->CollisionRadius < polyMin || projectedPosition->vx-dynPtr->CollisionRadius > polyMax)
        	return 0;
    }
    if (axisToIgnore!=iz)
    {
    	int polyMax,polyMin;
        
    	/* search z-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vz;
	        do
	        {
	        	int z;
				vertexNum--;
	        	z = polyPtr->PolyPoint[vertexNum].vz;
	        	if (z > polyMax) polyMax = z;
	        	else if (z < polyMin) polyMin = z;
	        }
            while(vertexNum);
		}        
                
        /* test to see if object & polygon overlap */
        if (projectedPosition->vz+dynPtr->CollisionRadius < polyMin || projectedPosition->vz-dynPtr->CollisionRadius > polyMax)
        	return 0;
    }
    if (axisToIgnore!=iy)
    {
    	int polyMax,polyMin;
        
    	/* search y-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vy;
	        do
	        {
				int y;
				vertexNum--;

				y = polyPtr->PolyPoint[vertexNum].vy;
	        	if (y > polyMax) polyMax = y;
	        	else if (y < polyMin) polyMin = y;
	        }
            while(vertexNum);
		}        
        
        /* test to see if object & polygon overlap */
        if (projectedPosition->vy+dynPtr->CollisionRadius < polyMin || projectedPosition->vy-dynPtr->CollisionRadius > polyMax)
        	return 0;

		#if 1
    	/* test for a 'step' in front of object */
        {
	        int heightOfStep = projectedPosition->vy+dynPtr->CollisionRadius - polyMin;  /* y-axis is +ve downwards, remember */
	        if (heightOfStep < MAXIMUM_STEP_HEIGHT) /* we've hit a 'step' - move player upwards */
	        {
	   			DistanceToStepUp=heightOfStep+COLLISION_GRANULARITY;
		        LOCALASSERT(heightOfStep>0);
	        }
		}
		#endif   
    }
    
    
    return 1;
}


static signed int DistanceMovedBeforeNRBBHitsPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
  	VECTORCH polyNormal;
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);
	
	polyNormal = polyPtr->PolyNormal;	  

	#if 0
	if (polyNormal.vy<=-65500)
		return DistanceMovedBeforeNRBBHitsNegYPolygon(dynPtr, polyPtr, distanceToMove);
	else if (polyNormal.vy>=65500)
		return DistanceMovedBeforeNRBBHitsPosYPolygon(dynPtr, polyPtr, distanceToMove);
	else if (polyNormal.vx>=65500)
		return DistanceMovedBeforeNRBBHitsPosXPolygon(dynPtr, polyPtr, distanceToMove);
	else if (polyNormal.vx<=-65500)
		return DistanceMovedBeforeNRBBHitsNegXPolygon(dynPtr, polyPtr, distanceToMove);
	else if (polyNormal.vz>=65500)
		return DistanceMovedBeforeNRBBHitsPosZPolygon(dynPtr, polyPtr, distanceToMove);
	else if (polyNormal.vz<=-65500)
		return DistanceMovedBeforeNRBBHitsNegZPolygon(dynPtr, polyPtr, distanceToMove);
	#endif

    dottedNormals = -DotProduct(&DirectionOfTravel,&polyNormal);
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
		VECTORCH polyPoint = polyPtr->PolyPoint[0];
	    int originToPlaneDist = DotProduct(&polyPoint,&polyNormal);
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] =
	        		DotProduct(&dynPtr->ObjectVertices[vertexNum],&polyNormal) - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0) return -1;
	        }
			while(vertexNum);
	    }    

		
		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		      	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
#if 0
static signed int DistanceMovedBeforeNRBBHitsNegYPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = DirectionOfTravel.vy;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = -polyPtr->PolyPoint[0].vy;

    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = -dynPtr->ObjectVertices[vertexNum].vy - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0)
				{
					return -1;
				}
	        }
			while(vertexNum);
	    }    

		
		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}

static signed int DistanceMovedBeforeNRBBHitsPosYPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = -DirectionOfTravel.vy;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = polyPtr->PolyPoint[0].vy;
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = dynPtr->ObjectVertices[vertexNum].vy - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0)
				{
					return -1;
				}
	        }
			while(vertexNum);
	    }    

		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
static signed int DistanceMovedBeforeNRBBHitsPosXPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = -DirectionOfTravel.vx;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = polyPtr->PolyPoint[0].vx;
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = dynPtr->ObjectVertices[vertexNum].vx - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0) return -1;
	        }
			while(vertexNum);
	    }    

		
		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
static signed int DistanceMovedBeforeNRBBHitsNegXPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = DirectionOfTravel.vx;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = -polyPtr->PolyPoint[0].vx;
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = -dynPtr->ObjectVertices[vertexNum].vx - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0) return -1;
	        }
			while(vertexNum);
	    }    

		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
static signed int DistanceMovedBeforeNRBBHitsPosZPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = -DirectionOfTravel.vz;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = polyPtr->PolyPoint[0].vz;
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = dynPtr->ObjectVertices[vertexNum].vz - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0) return -1;
	        }
			while(vertexNum);
	    }    

		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
static signed int DistanceMovedBeforeNRBBHitsNegZPolygon(DYNAMICSBLOCK *dynPtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
    int dottedNormals;
	int obstacleDistance= 0x7fffffff;
    int vertexToPlaneDist[8];

	LOCALASSERT(polyPtr);

    dottedNormals = DirectionOfTravel.vz;
	if (dottedNormals<=0) return -1; /* reject poly */

    /* calculate distance each vertex is from poly's plane along direction of motion */
    {
	    int originToPlaneDist = -polyPtr->PolyPoint[0].vz;
    	
    	{
        	int vertexNum=8;
            do
            {
            	vertexNum--;

	        	vertexToPlaneDist[vertexNum] = -dynPtr->ObjectVertices[vertexNum].vz - originToPlaneDist;

				if (vertexToPlaneDist[vertexNum] < 0) return -1;
	        }
			while(vertexNum);
	    }    

		
		{
        	int vertexNum=8;
			int *distancePtr = vertexToPlaneDist;
            do
            {
				*distancePtr = DIV_FIXED(*distancePtr,dottedNormals);
	
				if(*distancePtr < obstacleDistance)
					obstacleDistance = *distancePtr;
		    	
				distancePtr++;
		    }
            while(--vertexNum);

		}
	}
	if (obstacleDistance>=distanceToMove)
		return -2;

	/* test if any vertices projected paths intersect polygons */
	if (NRBBProjectsOntoPolygon(dynPtr,vertexToPlaneDist,polyPtr,&DirectionOfTravel))
	{
		return obstacleDistance;
	}

	return -3;
}
#endif
static int NRBBProjectsOntoPolygon(DYNAMICSBLOCK *dynPtr, int vertexToPlaneDist[], struct ColPolyTag *polyPtr, VECTORCH *projectionDirPtr)
{
	/* decide which 2d plane to project onto */
	int axisToIgnore = AxisToIgnore(&polyPtr->PolyNormal);
    int objMaxX,objMinX,objMaxY,objMinY,objMaxZ,objMinZ;

    if (axisToIgnore!=ix)
    {
    	int polyMax,polyMin;
        
    	/* search x-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vx;
			do
			{
   	        	int x;
				vertexNum--;
	        	x = polyPtr->PolyPoint[vertexNum].vx;
	        	if (x > polyMax) polyMax = x;
	        	else if (x < polyMin) polyMin = x;
	        }
            while(vertexNum);
		}        
        /* search x-coords of object vertices for min and max */
        {
			int i=7; /* 8 vertices in a cuboid */
        	
        	objMaxX = objMinX = dynPtr->ObjectVertices[0].vx + MUL_FIXED(vertexToPlaneDist[0],projectionDirPtr->vx);
			do
	        {
	        	int x;
	        	x = dynPtr->ObjectVertices[i].vx + MUL_FIXED(vertexToPlaneDist[i],projectionDirPtr->vx);
	        	if (x > objMaxX) objMaxX = x;
	        	else if (x < objMinX) objMinX = x;
	        	i--;																
	        }
			while(i);
		}
        
        /* test to see if object & polygon overlap */
        if (objMaxX < polyMin || objMinX > polyMax)
        	return 0;
    }
    if (axisToIgnore!=iz)
    {
    	int polyMax,polyMin;
        
    	/* search z-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vz;
	        do
	        {
	        	int z;
				vertexNum--;
	        	z = polyPtr->PolyPoint[vertexNum].vz;
	        	if (z > polyMax) polyMax = z;
	        	else if (z < polyMin) polyMin = z;
	        }
            while(vertexNum);
		}        
        /* search z-coords of object vertices for min and max */
        {
			int i=7; /* 8 vertices in a cuboid */
        	
        	objMaxZ = objMinZ = dynPtr->ObjectVertices[0].vz + MUL_FIXED(vertexToPlaneDist[0],projectionDirPtr->vz);
			do
	        {
	        	int z;
	        	z = dynPtr->ObjectVertices[i].vz + MUL_FIXED(vertexToPlaneDist[i],projectionDirPtr->vz);
	        	if (z > objMaxZ) objMaxZ = z;
	        	else if (z < objMinZ) objMinZ = z;
	        	i--;																
	        }
			while(i);		 
		}
        
        /* test to see if object & polygon overlap */
        if (objMaxZ < polyMin || objMinZ > polyMax)
        	return 0;
    }
    if (axisToIgnore!=iy)
    {
    	int polyMax,polyMin;
        
    	/* search y-coords of poly vertices for min and max */
        {
	    	int vertexNum=polyPtr->NumberOfVertices;
	    	
	    	polyMax = polyMin = polyPtr->PolyPoint[0].vy;
	        do
	        {
				int y;
				vertexNum--;

				y = polyPtr->PolyPoint[vertexNum].vy;
	        	if (y > polyMax) polyMax = y;
	        	else if (y < polyMin) polyMin = y;
	        }
            while(vertexNum);
		}        
        /* search y-coords of object vertices for min and max */
        {
			int i=7; /* 8 vertices in a cuboid */
        	
        	objMaxY = objMinY = dynPtr->ObjectVertices[0].vy + MUL_FIXED(vertexToPlaneDist[0],projectionDirPtr->vy);
			do
	        {
	        	int y;
	        	
	        	y = dynPtr->ObjectVertices[i].vy + MUL_FIXED(vertexToPlaneDist[i],projectionDirPtr->vy);
	        	if (y > objMaxY) objMaxY = y;
	        	else if (y < objMinY) objMinY = y;
	        	i--;																
	        }
			while(i);
		}
        
        /* test to see if object & polygon overlap */
        if (objMaxY < polyMin || objMinY > polyMax)
        	return 0;

    }
    
	/* test for triangle/rectangle overlap */
	/* traingle vertices are stored anticlockwise! */
	if (polyPtr->NumberOfVertices == 3)
	{
		if(axisToIgnore==ix)
		{
			VECTORCH n;

			n.vy = (polyPtr->PolyPoint[1].vz-polyPtr->PolyPoint[0].vz);
			n.vz = -(polyPtr->PolyPoint[1].vy-polyPtr->PolyPoint[0].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[0].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[0].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vy = (polyPtr->PolyPoint[2].vz-polyPtr->PolyPoint[1].vz);
			n.vz = -(polyPtr->PolyPoint[2].vy-polyPtr->PolyPoint[1].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[1].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[1].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vy = (polyPtr->PolyPoint[0].vz-polyPtr->PolyPoint[2].vz);
			n.vz = -(polyPtr->PolyPoint[0].vy-polyPtr->PolyPoint[2].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[2].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[2].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

		}
	    else if(axisToIgnore==iy)
		{
			VECTORCH n;

			n.vx = (polyPtr->PolyPoint[1].vz-polyPtr->PolyPoint[0].vz);
			n.vz = -(polyPtr->PolyPoint[1].vx-polyPtr->PolyPoint[0].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[2].vz-polyPtr->PolyPoint[1].vz);
			n.vz = -(polyPtr->PolyPoint[2].vx-polyPtr->PolyPoint[1].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[0].vz-polyPtr->PolyPoint[2].vz);
			n.vz = -(polyPtr->PolyPoint[0].vx-polyPtr->PolyPoint[2].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}
		}
		else if(axisToIgnore==iz)
		{
			VECTORCH n;

			n.vx = (polyPtr->PolyPoint[1].vy-polyPtr->PolyPoint[0].vy);
			n.vy = -(polyPtr->PolyPoint[1].vx-polyPtr->PolyPoint[0].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinY-polyPtr->PolyPoint[0].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinY-polyPtr->PolyPoint[0].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[2].vy-polyPtr->PolyPoint[1].vy);
			n.vy = -(polyPtr->PolyPoint[2].vx-polyPtr->PolyPoint[1].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinY-polyPtr->PolyPoint[1].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinY-polyPtr->PolyPoint[1].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[0].vy-polyPtr->PolyPoint[2].vy);
			n.vy = -(polyPtr->PolyPoint[0].vx-polyPtr->PolyPoint[2].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinY-polyPtr->PolyPoint[2].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinY-polyPtr->PolyPoint[2].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}
		}
	}
	else
	{
		if(axisToIgnore==ix)
		{
			VECTORCH n;

			n.vy = (polyPtr->PolyPoint[1].vz-polyPtr->PolyPoint[0].vz);
			n.vz = -(polyPtr->PolyPoint[1].vy-polyPtr->PolyPoint[0].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[0].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[0].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vy = (polyPtr->PolyPoint[2].vz-polyPtr->PolyPoint[1].vz);
			n.vz = -(polyPtr->PolyPoint[2].vy-polyPtr->PolyPoint[1].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[1].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[1].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vy = (polyPtr->PolyPoint[3].vz-polyPtr->PolyPoint[2].vz);
			n.vz = -(polyPtr->PolyPoint[3].vy-polyPtr->PolyPoint[2].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[2].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[2].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vy = (polyPtr->PolyPoint[0].vz-polyPtr->PolyPoint[3].vz);
			n.vz = -(polyPtr->PolyPoint[0].vy-polyPtr->PolyPoint[3].vy);
			if(polyPtr->PolyNormal.vx<0)
			{
				n.vy = -n.vy;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxY-polyPtr->PolyPoint[3].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d2 = (objMaxY-polyPtr->PolyPoint[3].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d3 = (objMinY-polyPtr->PolyPoint[3].vy)*n.vy + (objMaxZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d4 = (objMinY-polyPtr->PolyPoint[3].vy)*n.vy + (objMinZ-polyPtr->PolyPoint[3].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}
		}
	    else if(axisToIgnore==iy)
		{
			VECTORCH n;

			n.vx = (polyPtr->PolyPoint[1].vz-polyPtr->PolyPoint[0].vz);
			n.vz = -(polyPtr->PolyPoint[1].vx-polyPtr->PolyPoint[0].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[0].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[0].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[2].vz-polyPtr->PolyPoint[1].vz);
			n.vz = -(polyPtr->PolyPoint[2].vx-polyPtr->PolyPoint[1].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[1].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[1].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[3].vz-polyPtr->PolyPoint[2].vz);
			n.vz = -(polyPtr->PolyPoint[3].vx-polyPtr->PolyPoint[2].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[2].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[2].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[0].vz-polyPtr->PolyPoint[3].vz);
			n.vz = -(polyPtr->PolyPoint[0].vx-polyPtr->PolyPoint[3].vx);
			if(polyPtr->PolyNormal.vy>0)
			{
				n.vx = -n.vx;
				n.vz = -n.vz;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[3].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d2 = (objMaxX-polyPtr->PolyPoint[3].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d3 = (objMinX-polyPtr->PolyPoint[3].vx)*n.vx + (objMaxZ-polyPtr->PolyPoint[3].vz)*n.vz;
				d4 = (objMinX-polyPtr->PolyPoint[3].vx)*n.vx + (objMinZ-polyPtr->PolyPoint[3].vz)*n.vz;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

		}
		else if(axisToIgnore==iz)
		{
			VECTORCH n;

			n.vx = (polyPtr->PolyPoint[1].vy-polyPtr->PolyPoint[0].vy);
			n.vy = -(polyPtr->PolyPoint[1].vx-polyPtr->PolyPoint[0].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinY-polyPtr->PolyPoint[0].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[0].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[0].vx)*n.vx + (objMinY-polyPtr->PolyPoint[0].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[2].vy-polyPtr->PolyPoint[1].vy);
			n.vy = -(polyPtr->PolyPoint[2].vx-polyPtr->PolyPoint[1].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinY-polyPtr->PolyPoint[1].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[1].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[1].vx)*n.vx + (objMinY-polyPtr->PolyPoint[1].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[3].vy-polyPtr->PolyPoint[2].vy);
			n.vy = -(polyPtr->PolyPoint[3].vx-polyPtr->PolyPoint[2].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinY-polyPtr->PolyPoint[2].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[2].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[2].vx)*n.vx + (objMinY-polyPtr->PolyPoint[2].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}

			n.vx = (polyPtr->PolyPoint[0].vy-polyPtr->PolyPoint[3].vy);
			n.vy = -(polyPtr->PolyPoint[0].vx-polyPtr->PolyPoint[3].vx);
			if(polyPtr->PolyNormal.vz<0)
			{
				n.vx = -n.vx;
				n.vy = -n.vy;
			}
			{
				int d1,d2,d3,d4;
				d1 = (objMaxX-polyPtr->PolyPoint[3].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[3].vy)*n.vy;
				d2 = (objMaxX-polyPtr->PolyPoint[3].vx)*n.vx + (objMinY-polyPtr->PolyPoint[3].vy)*n.vy;
				d3 = (objMinX-polyPtr->PolyPoint[3].vx)*n.vx + (objMaxY-polyPtr->PolyPoint[3].vy)*n.vy;
				d4 = (objMinX-polyPtr->PolyPoint[3].vx)*n.vx + (objMinY-polyPtr->PolyPoint[3].vy)*n.vy;

				if (d1>0 && d2>0 && d3>0 && d4>0) return 0;
			}
		}
	}
    return 1;
}

static void CreateNRBBForObject(const STRATEGYBLOCK *sbPtr)
{
	VECTORCH *objectVertices = sbPtr->DynPtr->ObjectVertices;
	COLLISION_EXTENTS *extentsPtr = 0;
	int objectIsCrouching = 0;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	
	dynPtr->CollisionRadius = 0;


	switch (sbPtr->I_SBtype)
	{
   		case I_BehaviourMarinePlayer:
		{
			PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

			/* set player state */
			objectIsCrouching = (playerStatusPtr->ShapeState != PMph_Standing);
			
			switch(AvP.PlayerType)
			{
				case I_Marine:
					extentsPtr = &CollisionExtents[CE_MARINE];
					dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
					break;
					
				case I_Alien:
					extentsPtr = &CollisionExtents[CE_ALIEN];
					dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
					break;
				
				case I_Predator:
					extentsPtr = &CollisionExtents[CE_PREDATOR];
					dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
					break;
			}
			break;	
		}

   		case I_BehaviourAlien:
		{
			ALIEN_STATUS_BLOCK *alienStatusPtr = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
			objectIsCrouching = alienStatusPtr->IAmCrouched;

			extentsPtr = &CollisionExtents[CE_ALIEN];
			dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
			sbPtr->SBdptr->ObRadius = 2000;
			break;
		}
		case I_BehaviourPredator:
		{
			PREDATOR_STATUS_BLOCK *predatorStatusPtr = (PREDATOR_STATUS_BLOCK *)sbPtr->SBdataptr;
			objectIsCrouching = predatorStatusPtr->IAmCrouched;

			sbPtr->SBdptr->ObRadius = 2000;
			extentsPtr = &CollisionExtents[CE_PREDATOR];
//			dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
			break;
		}
		case I_BehaviourMarine:
		{
			MARINE_STATUS_BLOCK *marineStatusPtr = (MARINE_STATUS_BLOCK *)sbPtr->SBdataptr;
			objectIsCrouching = marineStatusPtr->IAmCrouched;
			
			sbPtr->SBdptr->ObRadius = 2000;
			extentsPtr = &CollisionExtents[CE_MARINE];
//			dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
			break;
		}
		case I_BehaviourFaceHugger:
		{
			extentsPtr = &CollisionExtents[CE_FACEHUGGER];
			dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
			break;
		}
		case I_BehaviourXenoborg:
		{
			extentsPtr = &CollisionExtents[CE_XENOBORG];
			break;
		}
		case I_BehaviourPredatorAlien:
		{
			extentsPtr = &CollisionExtents[CE_PREDATORALIEN];
			break;
		}
		
		case I_BehaviourQueenAlien:
			extentsPtr = &CollisionExtents[CE_QUEEN];
			break;
		case I_BehaviourSeal:
			extentsPtr = &CollisionExtents[CE_MARINE];
			break;
		
		case I_BehaviourNetCorpse:
			extentsPtr = &CollisionExtents[CE_CORPSE];
			sbPtr->SBdptr->ObRadius = 700;
			break;

		case I_BehaviourNetGhost:
		{
			NETGHOSTDATABLOCK *ghostData;

			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(ghostData);
			
			switch(ghostData->type)
			{
		   		case I_BehaviourAlienPlayer:
				{
					extentsPtr = &CollisionExtents[CE_ALIEN];
					dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
					sbPtr->SBdptr->ObRadius = 700;
					break;
				}
				case I_BehaviourPredatorPlayer:
				{
					extentsPtr = &CollisionExtents[CE_PREDATOR];
					sbPtr->SBdptr->ObRadius = 700;
					break;
				}
				case I_BehaviourMarinePlayer:
				{
					extentsPtr = &CollisionExtents[CE_MARINE];
					sbPtr->SBdptr->ObRadius = 700;
					break;
				}

		   		case I_BehaviourAlien:
				{
					#if 0
					ALIEN_STATUS_BLOCK *alienStatusPtr = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
					objectIsCrouching = alienStatusPtr->IAmCrouched;
					#endif
					extentsPtr = &CollisionExtents[CE_ALIEN];
					dynPtr->CollisionRadius = extentsPtr->CollisionRadius;
					sbPtr->SBdptr->ObRadius = 2000;
					break;
				}

				case I_BehaviourNetCorpse:
					extentsPtr = &CollisionExtents[CE_MARINE];
					sbPtr->SBdptr->ObRadius = 700;
					break;
				default:
					break;
			}
		}

		default:
			break;
	}

	if (extentsPtr)
	{
		/* max X */
		objectVertices[0].vx = extentsPtr->CollisionRadius;
		objectVertices[1].vx = extentsPtr->CollisionRadius;
		objectVertices[2].vx = extentsPtr->CollisionRadius;
		objectVertices[3].vx = extentsPtr->CollisionRadius;

		/* max Z */
		objectVertices[0].vz = extentsPtr->CollisionRadius;
	    objectVertices[2].vz = extentsPtr->CollisionRadius;
	    objectVertices[4].vz = extentsPtr->CollisionRadius;
	    objectVertices[6].vz = extentsPtr->CollisionRadius;

		/* min X */
		objectVertices[4].vx = -extentsPtr->CollisionRadius;
		objectVertices[5].vx = -extentsPtr->CollisionRadius;
		objectVertices[6].vx = -extentsPtr->CollisionRadius;
		objectVertices[7].vx = -extentsPtr->CollisionRadius;
	
		/* min Z */
		objectVertices[1].vz = -extentsPtr->CollisionRadius;
		objectVertices[3].vz = -extentsPtr->CollisionRadius;
		objectVertices[5].vz = -extentsPtr->CollisionRadius;
		objectVertices[7].vz = -extentsPtr->CollisionRadius;
	
		/* max Y */
	   	objectVertices[0].vy = extentsPtr->Bottom;
	    objectVertices[1].vy = extentsPtr->Bottom;
	    objectVertices[4].vy = extentsPtr->Bottom;
	    objectVertices[5].vy = extentsPtr->Bottom;
		
		/* min Y */
		if(objectIsCrouching)
		{
		   	objectVertices[2].vy = extentsPtr->CrouchingTop;
		    objectVertices[3].vy = extentsPtr->CrouchingTop;
		    objectVertices[6].vy = extentsPtr->CrouchingTop;
		    objectVertices[7].vy = extentsPtr->CrouchingTop;
		}
		else
		{
		   	objectVertices[2].vy = extentsPtr->StandingTop;
		    objectVertices[3].vy = extentsPtr->StandingTop;
		    objectVertices[6].vy = extentsPtr->StandingTop;
		    objectVertices[7].vy = extentsPtr->StandingTop;
	    }
	}
	else
	/* make a cuboid from the shape's extent data */
	{
	    {
	   		int shapeMaxX = sbPtr->SBdptr->ObMaxX;
			if (shapeMaxX < MINIMUM_BOUNDINGBOX_EXTENT) shapeMaxX = MINIMUM_BOUNDINGBOX_EXTENT;
	        objectVertices[0].vx = shapeMaxX;
	        objectVertices[1].vx = shapeMaxX;
	        objectVertices[2].vx = shapeMaxX;
	        objectVertices[3].vx = shapeMaxX;
	    }
	    {
	   		int shapeMinX = sbPtr->SBdptr->ObMinX;
			if (shapeMinX > -MINIMUM_BOUNDINGBOX_EXTENT) shapeMinX = -MINIMUM_BOUNDINGBOX_EXTENT;
	        objectVertices[4].vx = shapeMinX;
	        objectVertices[5].vx = shapeMinX;
	        objectVertices[6].vx = shapeMinX;
	        objectVertices[7].vx = shapeMinX;
	    }
	    {
	   		int shapeMaxY = sbPtr->SBdptr->ObMaxY;
			if (shapeMaxY < MINIMUM_BOUNDINGBOX_EXTENT) shapeMaxY = MINIMUM_BOUNDINGBOX_EXTENT;
	        objectVertices[0].vy = shapeMaxY;
	        objectVertices[1].vy = shapeMaxY;
	        objectVertices[4].vy = shapeMaxY;
	        objectVertices[5].vy = shapeMaxY;
	    }
	    {
	   		int shapeMinY = sbPtr->SBdptr->ObMinY;
			if (shapeMinY > -MINIMUM_BOUNDINGBOX_EXTENT) shapeMinY = -MINIMUM_BOUNDINGBOX_EXTENT;
	        objectVertices[2].vy = shapeMinY;
	        objectVertices[3].vy = shapeMinY;
	        objectVertices[6].vy = shapeMinY;
	        objectVertices[7].vy = shapeMinY;
	    }
	    {
	   		int shapeMaxZ = sbPtr->SBdptr->ObMaxZ;
			if (shapeMaxZ < MINIMUM_BOUNDINGBOX_EXTENT) shapeMaxZ = MINIMUM_BOUNDINGBOX_EXTENT;
			objectVertices[0].vz = shapeMaxZ;
	        objectVertices[2].vz = shapeMaxZ;
	        objectVertices[4].vz = shapeMaxZ;
	        objectVertices[6].vz = shapeMaxZ;
		}
		{
	   		int shapeMinZ = sbPtr->SBdptr->ObMinZ;
			if (shapeMinZ > -MINIMUM_BOUNDINGBOX_EXTENT) shapeMinZ = -MINIMUM_BOUNDINGBOX_EXTENT;
		    objectVertices[1].vz = shapeMinZ;
	        objectVertices[3].vz = shapeMinZ;
	        objectVertices[5].vz = shapeMinZ;
	        objectVertices[7].vz = shapeMinZ;
	    }
	}

	/* translate cuboid into world space */
	{						   
		VECTORCH *vertexPtr = objectVertices;
        VECTORCH objectPosition = sbPtr->DynPtr->Position;
        
        int vertexNum=8;
        do
        {
        	vertexPtr->vx += objectPosition.vx;
        	vertexPtr->vy += objectPosition.vy;	
        	vertexPtr->vz += objectPosition.vz;	
        	vertexPtr++;
        }
        while(--vertexNum);
        
	}
}
static void CreateSphereBBForObject(const STRATEGYBLOCK *sbPtr)
{
	VECTORCH *objectVertices = sbPtr->DynPtr->ObjectVertices;
	/* make a cuboid from the sphere's radius */
	{
		int radius = sbPtr->DynPtr->CollisionRadius;	

        objectVertices[0].vx = radius;
        objectVertices[1].vx = radius;
        objectVertices[2].vx = radius;
        objectVertices[3].vx = radius;
        objectVertices[0].vy = radius;
        objectVertices[1].vy = radius;
        objectVertices[4].vy = radius;
        objectVertices[5].vy = radius;
		objectVertices[0].vz = radius;
        objectVertices[2].vz = radius;
        objectVertices[4].vz = radius;
        objectVertices[6].vz = radius;

		radius = -radius;

        objectVertices[4].vx = radius;
        objectVertices[5].vx = radius;
        objectVertices[6].vx = radius;
        objectVertices[7].vx = radius;
        objectVertices[2].vy = radius;
        objectVertices[3].vy = radius;
        objectVertices[6].vy = radius;
        objectVertices[7].vy = radius;
	    objectVertices[1].vz = radius;
        objectVertices[3].vz = radius;
        objectVertices[5].vz = radius;
        objectVertices[7].vz = radius;
	}

	/* translate cuboid into world space */
	{						   
		VECTORCH *vertexPtr = objectVertices;
        VECTORCH objectPosition = sbPtr->DynPtr->Position;
        
        int vertexNum=8;
        do
        {
        	vertexPtr->vx += objectPosition.vx;
        	vertexPtr->vy += objectPosition.vy;	
        	vertexPtr->vz += objectPosition.vz;	
        	vertexPtr++;
        }
        while(--vertexNum);
        
	}
}




#if 0
static int RelocatedDueToFallout(DYNAMICSBLOCK *dynPtr)
{
	/* If the object is outside ALL of the modules in the
	ActiveBlockList, move it back to its previous position. */	
	
	{
		extern int NumActiveBlocks;
	    extern DISPLAYBLOCK *ActiveBlockList[];

	   	/* scan through modules and stop if object is inside any of them */
		{
			int objectInsideSomething = 0;
		   	
		   	int numberOfObjects = NumActiveBlocks;
		   	while(numberOfObjects)
		   	{
		   		DISPLAYBLOCK* objectPtr = ActiveBlockList[--numberOfObjects];
		   		GLOBALASSERT(objectPtr);
		   		if (objectPtr->ObMyModule) /* is object a module ? */
		    	{
					VECTORCH position = dynPtr->Position;
					position.vx -= objectPtr->ObWorld.vx;
					position.vy -= objectPtr->ObWorld.vy;
					position.vz -= objectPtr->ObWorld.vz;

					if (position.vx >= objectPtr->ObMinX) 
				    	if (position.vx <= objectPtr->ObMaxX) 
						    if (position.vz >= objectPtr->ObMinZ) 
							    if (position.vz <= objectPtr->ObMaxZ) 
								    if (position.vy >= objectPtr->ObMinY) 
									    if (position.vy <= objectPtr->ObMaxY)
										{
											objectInsideSomething = 1;
											break;
										}
		        }
		    }

			if (!objectInsideSomething)
			{
				dynPtr->Position=dynPtr->PrevPosition;
//				NewOnScreenMessage("DEBUG: WENT OUTSIDE ENV");
			}

			return !objectInsideSomething;
	  	}
	}   

}
#endif






/* KJL 10:43:28 8/20/97 - stuff to add to bh_near.c */
#if 0

	textprint("alien vel %d %d %d\n",sbPtr->DynPtr->LinVelocity.vx,sbPtr->DynPtr->LinVelocity.vy,sbPtr->DynPtr->LinVelocity.vz);
	#if 1
	if(alienStatusPtr->IAmCrouched)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);

		dynPtr->DynamicsType = DYN_TYPE_SPHERE_COLLISIONS;
	}
	else
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);

		dynPtr->DynamicsType = DYN_TYPE_NRBB_COLLISIONS;
	}
	#endif
#endif

static signed int DistanceMovedBeforeParticleHitsPolygon(PARTICLE *particlePtr, struct ColPolyTag *polyPtr, int distanceToMove);

#if 1

/*KJL****************
* PARTICLE DYNAMICS *
****************KJL*/

int ParticleDynamics(PARTICLE *particlePtr, VECTORCH *obstacleNormalPtr, int *moduleIndexPtr)
{
	VECTORCH prevPosition = particlePtr->Position;
	DISPLAYBLOCK *hitModule = 0;
    int polysLeft;
    struct ColPolyTag *nearPolysPtr;
	int distanceToMove;
    VECTORCH displacement;
	int hitObstacle=0;

	displacement.vx = MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
	displacement.vy = MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
	displacement.vz = MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
	distanceToMove = Magnitude(&displacement);

	if (distanceToMove<COLLISION_GRANULARITY) return 0;

	DirectionOfTravel = particlePtr->Velocity;
	Normalise(&DirectionOfTravel);

	if (!LocalDetailLevels.BloodCollidesWithEnvironment
		&&(particlePtr->ParticleID==PARTICLE_ALIEN_BLOOD
		 ||particlePtr->ParticleID==PARTICLE_HUMAN_BLOOD
		 ||particlePtr->ParticleID==PARTICLE_PREDATOR_BLOOD))
	{	
		CollisionPolysPtr = &CollisionPolysArray[0];
    	NumberOfCollisionPolys=0;
	}
	else
	{
	   	FindLandscapePolygonsInParticlesPath(particlePtr, &displacement);
	}
		
    polysLeft = NumberOfCollisionPolys;
    nearPolysPtr = CollisionPolysArray;
    
	/* check against selected polys */
    while(polysLeft)
	{
		signed int distanceToObstacle;
		
		distanceToObstacle = DistanceMovedBeforeParticleHitsPolygon(particlePtr,nearPolysPtr,distanceToMove);

		if (distanceToObstacle>=0)
        {
	       	hitObstacle=1;
	       	distanceToMove = distanceToObstacle;
		   	*obstacleNormalPtr = nearPolysPtr->PolyNormal;
			if ( (nearPolysPtr->ParentObject)
			   &&(nearPolysPtr->ParentObject->ObMyModule) )
			{
				*moduleIndexPtr = nearPolysPtr->ParentObject->ObMyModule->m_index;
				hitModule = nearPolysPtr->ParentObject;
			}
			else
			{
				*moduleIndexPtr = -1;
				hitModule=0;
				if (particlePtr->ParticleID==PARTICLE_FLECHETTE && nearPolysPtr->ParentObject)
				{
					if (nearPolysPtr->ParentObject->ObStrategyBlock)
					{
						CauseDamageToObject(nearPolysPtr->ParentObject->ObStrategyBlock,&FlechetteDamage,ONE_FIXED,&(particlePtr->Velocity));
					}
				} else if (particlePtr->ParticleID==PARTICLE_PREDPISTOL_FLECHETTE && nearPolysPtr->ParentObject)
				{
					if (nearPolysPtr->ParentObject->ObStrategyBlock)
					{
						CauseDamageToObject(nearPolysPtr->ParentObject->ObStrategyBlock,&PredPistol_FlechetteDamage,ONE_FIXED,&(particlePtr->Velocity));
					}
				}
	
			}
		}
        nearPolysPtr++;
		polysLeft--;
	}
	

	if (distanceToMove>COLLISION_GRANULARITY)
	{
		distanceToMove-=COLLISION_GRANULARITY;
		particlePtr->Position.vx += MUL_FIXED(DirectionOfTravel.vx,distanceToMove);
		particlePtr->Position.vy += MUL_FIXED(DirectionOfTravel.vy,distanceToMove);
		particlePtr->Position.vz += MUL_FIXED(DirectionOfTravel.vz,distanceToMove);
	}
	
	if (hitObstacle)
	{
		int magOfPerpImp = DotProduct(obstacleNormalPtr,&(particlePtr->Velocity));
		int magnitude = Magnitude(&particlePtr->Velocity);
		particlePtr->Velocity.vx -= MUL_FIXED(obstacleNormalPtr->vx, magOfPerpImp);
		particlePtr->Velocity.vy -= MUL_FIXED(obstacleNormalPtr->vy, magOfPerpImp);
		particlePtr->Velocity.vz -= MUL_FIXED(obstacleNormalPtr->vz, magOfPerpImp);
		if(particlePtr->Velocity.vx || particlePtr->Velocity.vy || particlePtr->Velocity.vz)
		{
			Normalise(&particlePtr->Velocity);
			particlePtr->Velocity.vx = MUL_FIXED(particlePtr->Velocity.vx,magnitude/2);
			particlePtr->Velocity.vy = MUL_FIXED(particlePtr->Velocity.vy,magnitude/2);
			particlePtr->Velocity.vz = MUL_FIXED(particlePtr->Velocity.vz,magnitude/2);
		}
	}

	/* test to see if you've hit any objects */
	{
		int i = NumberOfDynamicObjects;
	   	{
			if (prevPosition.vx > particlePtr->Position.vx)
			{
			    DBBMinX = particlePtr->Position.vx;
				DBBMaxX = prevPosition.vx;
			}    
			else
			{
			    DBBMinX = prevPosition.vx;
				DBBMaxX = particlePtr->Position.vx;
			}

			if (prevPosition.vy > particlePtr->Position.vy)
			{
			    DBBMinY = particlePtr->Position.vy;
				DBBMaxY = prevPosition.vy;
			}    
			else
			{
			    DBBMinY = prevPosition.vy;
				DBBMaxY = particlePtr->Position.vy;
			}

			if (prevPosition.vz > particlePtr->Position.vz)
			{
			    DBBMinZ = particlePtr->Position.vz;
				DBBMaxZ = prevPosition.vz;
			}    
			else
			{
			    DBBMinZ = prevPosition.vz;
				DBBMaxZ = particlePtr->Position.vz;
			}

		}
		/* scan through objects */
		while(i--)
		{
			STRATEGYBLOCK *sbPtr = DynamicObjectsList[i];
			DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;

			if (dispPtr)
			if (DBBMinX<dynPtr->ObjectVertices[0].vx && DBBMaxX>dynPtr->ObjectVertices[7].vx)		
			if (DBBMinY<dynPtr->ObjectVertices[0].vy && DBBMaxY>dynPtr->ObjectVertices[7].vy)		
			if (DBBMinZ<dynPtr->ObjectVertices[0].vz && DBBMaxZ>dynPtr->ObjectVertices[7].vz)		
			{
				/* blam! particle hit something */
				if (particlePtr->ParticleID==PARTICLE_ALIEN_BLOOD)
				{
					if ((dispPtr==Player)||(sbPtr->I_SBtype==I_BehaviourMarine)) {
						CauseDamageToObject(sbPtr,&TemplateAmmo[AMMO_ALIEN_FRAG].MaxDamage[AvP.Difficulty], NormalFrameTime,NULL);
					}
				}
				else if (particlePtr->ParticleID==PARTICLE_FLAME 
				       ||particlePtr->ParticleID==PARTICLE_PARGEN_FLAME
				       ||particlePtr->ParticleID==PARTICLE_MOLOTOVFLAME)
				{
					BOOL ignoreDamage = FALSE;
					if ((dispPtr==Player)||(dispPtr->HModelControlBlock))
					{
				
						if(AvP.Network != I_No_Network)
						{
							//If friendly fire has been disabled , we may need to ignore this option
							if(particlePtr->ParticleID==PARTICLE_FLAME)
							{
								if (netGameData.disableFriendlyFire && (netGameData.gameType==NGT_CoopDeathmatch || netGameData.gameType==NGT_Coop)) 
								{
									//okay , friendly fire is off , so flamethrower particles , can't harm
									//marines.
									//in coop games it shouldn't harm predators either
									if(sbPtr->I_SBtype==I_BehaviourNetGhost)
									{
										NETGHOSTDATABLOCK *ghostData=(NETGHOSTDATABLOCK *) sbPtr->SBdataptr;
										GLOBALASSERT(ghostData);
										if (ghostData->type==I_BehaviourMarinePlayer) 
										{
											ignoreDamage = TRUE;
										}
										else if (ghostData->type==I_BehaviourPredatorPlayer && netGameData.gameType==NGT_Coop) 
										{
											ignoreDamage = TRUE;
										}
									}
									if(dispPtr==Player)
									{
										if(AvP.PlayerType == I_Marine)
										{
											ignoreDamage = TRUE;
										}
										else if (AvP.PlayerType == I_Predator && netGameData.gameType==NGT_Coop)
										{
											ignoreDamage = TRUE;
										}
									}
									
								}
							}
														
							if(!ignoreDamage)
							{
								if(dispPtr==Player)
								{
									extern DPID myIgniterId;
									myIgniterId=0; //the player hasn't been set alight by a network opponent
								}
								else if(sbPtr->I_SBtype==I_BehaviourAlien)
								{
									//note that the alien has been set on fire locally
									ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
									if(particlePtr->ParticleID==PARTICLE_FLAME)
									{
										extern DPID AVPDPNetID;
										alienStatus->aliensIgniterId=AVPDPNetID;
									}
									else
									{
										//no credit for aliens killed by molotov particles , or
										//particle generator particles.
										alienStatus->aliensIgniterId=0;
									}
								}
							}
						}
						if(!ignoreDamage)
						{
							if(sbPtr->I_SBtype==I_BehaviourNetGhost)
							{
															
								/*
								Don't let molotov particles damage netghosts. Each machine in a net game will be generating
								its own molotov particles.
								Similary for PARTICLE_PARGEN_FLAME
								*/
								if(particlePtr->ParticleID==PARTICLE_FLAME)
								{
									NETGHOSTDATABLOCK *ghostData=(NETGHOSTDATABLOCK *) sbPtr->SBdataptr;
									GLOBALASSERT(ghostData);

									
									//just add up the number of particles that have hit the ghost 
									//so the damage message can be sent in one block
									ghostData->FlameHitCount++;
									sbPtr->SBDamageBlock.IsOnFire = 1;
								}
							}
							else
							{
								CauseDamageToObject(sbPtr,&TemplateAmmo[AMMO_FLAMETHROWER].MaxDamage[AvP.Difficulty], ONE_FIXED/400,NULL);
								sbPtr->SBDamageBlock.IsOnFire = 1;
							}
						}
					}
					if (dispPtr==Player && !ignoreDamage)
					{
						PlayerStatusPtr->fireTimer=PLAYER_ON_FIRE_TIME;
					}
		 		}
				else if (particlePtr->ParticleID==PARTICLE_FLECHETTE)
				{
					if(sbPtr->I_SBtype==I_BehaviourNetGhost)
					{
						//for net ghosts add up all the flechettes that hit it this frame , and send as one
						//damage message
						NETGHOSTDATABLOCK *ghostData=(NETGHOSTDATABLOCK *) sbPtr->SBdataptr;
						GLOBALASSERT(ghostData);
						ghostData->FlechetteHitCount++;

					}
					else
					{
						CauseDamageToObject(sbPtr,&FlechetteDamage,ONE_FIXED,&(particlePtr->Velocity));
					}
				}
				else if (particlePtr->ParticleID==PARTICLE_PREDPISTOL_FLECHETTE)
				{
					CauseDamageToObject(sbPtr,&PredPistol_FlechetteDamage,particlePtr->LifeTime,&(particlePtr->Velocity));
				}
			}
		}
	}

	if (*moduleIndexPtr != -1)
	{
		if(hitModule)
		{
			char stickWhereYouAre = 0;

			if (hitModule->ObStrategyBlock)
			{
				if (hitModule->ObMyModule && (!hitModule->ObMorphCtrl))
				{
					stickWhereYouAre=1;
				}
			}
			else
			{
				stickWhereYouAre = 1;
			}
		
			if (!stickWhereYouAre) *moduleIndexPtr = -1;
			
			return hitObstacle;
		}				
		return hitObstacle;
	}
	else return 0;
}

static void FindLandscapePolygonsInParticlesPath(PARTICLE *particlePtr, VECTORCH *displacementPtr)
{
	extern int NumActiveBlocks;
    extern DISPLAYBLOCK *ActiveBlockList[];

	/* initialise near polygons array */	
	CollisionPolysPtr = &CollisionPolysArray[0];
    NumberOfCollisionPolys=0;

   	/* scan through ActiveBlockList for modules */
	{
	   	int numberOfObjects = NumActiveBlocks;
	   	while(numberOfObjects)
	   	{
	   		DISPLAYBLOCK* objectPtr = ActiveBlockList[--numberOfObjects];
	   		char isStaticObject=0;

	   		GLOBALASSERT(objectPtr);
			if(objectPtr->ObStrategyBlock)
				if(objectPtr->ObStrategyBlock->DynPtr)
				{
					if(((objectPtr->ObStrategyBlock->DynPtr->IsStatic)
					||(objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithObjects))
			  		&&(!objectPtr->ObStrategyBlock->DynPtr->OnlyCollideWithEnvironment))
						isStaticObject=1;
				}

	   		if (objectPtr->ObMyModule) /* is object a module or static? */
	    	{
				{
					DBBMaxX = particlePtr->Position.vx - objectPtr->ObWorld.vx + COLLISION_GRANULARITY; 
					DBBMinX = particlePtr->Position.vx - objectPtr->ObWorld.vx - COLLISION_GRANULARITY;

					DBBMaxY = particlePtr->Position.vy - objectPtr->ObWorld.vy + COLLISION_GRANULARITY; 
					DBBMinY = particlePtr->Position.vy - objectPtr->ObWorld.vy - COLLISION_GRANULARITY;
					
					DBBMaxZ = particlePtr->Position.vz - objectPtr->ObWorld.vz + COLLISION_GRANULARITY;
					DBBMinZ = particlePtr->Position.vz - objectPtr->ObWorld.vz - COLLISION_GRANULARITY; 

					if (displacementPtr->vx > 0)
					{
						DBBMaxX += displacementPtr->vx;
					}    
					else
					{
					    DBBMinX += displacementPtr->vx;
					}

					if (displacementPtr->vy > 0)
					{
						DBBMaxY += displacementPtr->vy;
					}    
					else
					{
					    DBBMinY += displacementPtr->vy;
					}

					if (displacementPtr->vz > 0)
					{
						DBBMaxZ += displacementPtr->vz;
					}    
					else
					{
					    DBBMinZ += displacementPtr->vz;
					}

				}
				LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
				
				/* if the bounding box intersects with the object, investigate */
			   	if (( (DBBMaxX >= objectPtr->ObMinX) && (DBBMinX <= objectPtr->ObMaxX) )
			      &&( (DBBMaxY >= objectPtr->ObMinY) && (DBBMinY <= objectPtr->ObMaxY) )
 			      &&( (DBBMaxZ >= objectPtr->ObMinZ) && (DBBMinZ <= objectPtr->ObMaxZ) ))
			       	TestShapeWithParticlesDynamicBoundingBox(objectPtr);
	        }
			else if (isStaticObject)
			{
				{
					DBBMaxX = particlePtr->Position.vx - objectPtr->ObWorld.vx + COLLISION_GRANULARITY; 
					DBBMinX = particlePtr->Position.vx - objectPtr->ObWorld.vx - COLLISION_GRANULARITY;

					DBBMaxY = particlePtr->Position.vy - objectPtr->ObWorld.vy + COLLISION_GRANULARITY; 
					DBBMinY = particlePtr->Position.vy - objectPtr->ObWorld.vy - COLLISION_GRANULARITY;
					
					DBBMaxZ = particlePtr->Position.vz - objectPtr->ObWorld.vz + COLLISION_GRANULARITY;
					DBBMinZ = particlePtr->Position.vz - objectPtr->ObWorld.vz - COLLISION_GRANULARITY; 

					if (displacementPtr->vx > 0)
					{
						DBBMaxX += displacementPtr->vx;
					}    
					else
					{
					    DBBMinX += displacementPtr->vx;
					}

					if (displacementPtr->vy > 0)
					{
						DBBMaxY += displacementPtr->vy;
					}    
					else
					{
					    DBBMinY += displacementPtr->vy;
					}

					if (displacementPtr->vz > 0)
					{
						DBBMaxZ += displacementPtr->vz;
					}    
					else
					{
					    DBBMinZ += displacementPtr->vz;
					}

				}
				LOCALASSERT(NumberOfCollisionPolys < MAXIMUM_NUMBER_OF_COLLISIONPOLYS);
				
				/* if the bounding box intersects with the object, investigate */
			   	if (( (DBBMaxX >= -objectPtr->ObRadius) && (DBBMinX <= objectPtr->ObRadius) )
			      &&( (DBBMaxY >= -objectPtr->ObRadius) && (DBBMinY <= objectPtr->ObRadius) )
 			      &&( (DBBMaxZ >= -objectPtr->ObRadius) && (DBBMinZ <= objectPtr->ObRadius) ))
			       	TestShapeWithParticlesDynamicBoundingBox(objectPtr);
				
			}
	    }
  	}
}   

static signed int DistanceMovedBeforeParticleHitsPolygon(PARTICLE *particlePtr, struct ColPolyTag *polyPtr, int distanceToMove)
{
	VECTORCH pointOnPlane;
	int lambda;
	int axis1;
	int axis2;

	{
		int normDotBeta = DotProduct(&(polyPtr->PolyNormal),&DirectionOfTravel);

		/* trivial rejection of poly if it is not facing LOS */
   		if (normDotBeta>-500)
   		{
   			return -1;
		}
	
		/* calculate coords of plane-line intersection */
		{
			int d;
			{
				/* get a pt in the poly */
				VECTORCH pop=polyPtr->PolyPoint[0];								  
				pop.vx -= particlePtr->Position.vx;
				pop.vy -= particlePtr->Position.vy;
				pop.vz -= particlePtr->Position.vz;

			  	d = DotProduct(&(polyPtr->PolyNormal),&pop);
			}
			if (d>=0)
			{
				return -1;
			}		  
		  	lambda = DIV_FIXED(d,normDotBeta);
			if (lambda>=distanceToMove) 
			{
				return -1;
			}															                  
	   		pointOnPlane.vx	= particlePtr->Position.vx + MUL_FIXED(lambda,DirectionOfTravel.vx);
		   	pointOnPlane.vy	= particlePtr->Position.vy + MUL_FIXED(lambda,DirectionOfTravel.vy);
	   		pointOnPlane.vz	= particlePtr->Position.vz + MUL_FIXED(lambda,DirectionOfTravel.vz);

  		}

		/* decide which 2d plane to project onto */

		{
			VECTORCH absNormal = (polyPtr->PolyNormal);
			if (absNormal.vx<0) absNormal.vx=-absNormal.vx;
			if (absNormal.vy<0) absNormal.vy=-absNormal.vy;
			if (absNormal.vz<0) absNormal.vz=-absNormal.vz;

			if (absNormal.vx > absNormal.vy)
			{
				if (absNormal.vx > absNormal.vz)
				{
					axis1=iy;
					axis2=iz;
				}
				else
				{
					axis1=ix;
					axis2=iy;
				}
			}
			else
			{
				if (absNormal.vy > absNormal.vz)
				{
					axis1=ix;
					axis2=iz;
				}
				else
				{
					axis1=ix;
					axis2=iy;
				}
			}
		}

	}
	{
		int projectedPolyVertex[20];
		int projectedPointOnPlane[2];
		int *popPtr = &pointOnPlane.vx;

		projectedPointOnPlane[0]=*(popPtr+axis1);
	 	projectedPointOnPlane[1]=*(popPtr+axis2);

	 	{
	 		VECTORCH *vertexPtr = polyPtr->PolyPoint;
	 		int *projectedVertexPtr= projectedPolyVertex;
			int noOfVertices = polyPtr->NumberOfVertices;

			do
			{
	 			*projectedVertexPtr++ = *((int*)vertexPtr + axis1);
	 			*projectedVertexPtr++ = *((int*)vertexPtr + axis2);

 	 		   	vertexPtr++;
	 			noOfVertices--;
	 		}
            while(noOfVertices);

	 	}


		if (PointInPolygon(&projectedPointOnPlane[0],&projectedPolyVertex[0],polyPtr->NumberOfVertices,2))
		{
		    return lambda;
		}
	}
	
	return -1;
}

void AddEffectsOfForceGenerators(VECTORCH *positionPtr, VECTORCH *impulsePtr, int mass)
{
	extern int NumActiveBlocks;
	extern DISPLAYBLOCK *ActiveBlockList[];
	int numOfObjects = NumActiveBlocks;
	while(numOfObjects)
	{
		DISPLAYBLOCK *objectPtr = ActiveBlockList[--numOfObjects];
		STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;

		if (sbPtr && sbPtr->I_SBtype==I_BehaviourFan)
		{
		 	FAN_BEHAV_BLOCK *fanPtr;
			fanPtr = (FAN_BEHAV_BLOCK*)sbPtr->SBdataptr;

			if (fanPtr->wind_speed)
			{
				int mag;
				VECTORCH disp = sbPtr->DynPtr->Position;
				disp.vx -= positionPtr->vx;
				disp.vy -= positionPtr->vy;
				disp.vz -= positionPtr->vz;

				mag = 16384 - Approximate3dMagnitude(&disp);

				if (mag<0) continue;
				if(MagnitudeOfCrossProduct(&disp,&fanPtr->fan_wind_direction)>objectPtr->ObRadius+200) continue;
				mag=MUL_FIXED(MUL_FIXED(mag*32*4*32,fanPtr->wind_speed),NormalFrameTime)/mass;
	
				impulsePtr->vx += MUL_FIXED(fanPtr->fan_wind_direction.vx,mag);
				impulsePtr->vy += MUL_FIXED(fanPtr->fan_wind_direction.vy,mag);
				impulsePtr->vz += MUL_FIXED(fanPtr->fan_wind_direction.vz,mag);
			}
//			PrintDebuggingText("Fan acting upon object %d\n",fanPtr->wind_speed);
		}
	}
}

#endif


 
VECTORCH *GetNearestModuleTeleportPoint(MODULE* thisModulePtr, VECTORCH* positionPtr)
{
	extern FARENTRYPOINTSHEADER *FALLP_EntryPoints;
	int numEps;
	FARENTRYPOINT *epList;
	FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
	int tmIndex = thisModulePtr->m_aimodule->m_index;
	int distance = 0x7fffffff;
	
	numEps = FALLP_EntryPoints[tmIndex].numEntryPoints;
	epList = FALLP_EntryPoints[tmIndex].entryPointsList;

	while((numEps>0))
	{
		VECTORCH p = *positionPtr;
		int d;		

		p.vx -= thisModulePtr->m_aimodule->m_world.vx + epList->position.vx;
		p.vy -= thisModulePtr->m_aimodule->m_world.vy + epList->position.vy;
		p.vz -= thisModulePtr->m_aimodule->m_world.vz + epList->position.vz;

		d = Approximate3dMagnitude(&p);

		if (d<distance)
		{
			distance = d;
			thisEp = epList;
		}

		epList++;
		numEps--;
	}
	
	if(!thisEp)
	{
		return 0;
	}
	else
	{
		return &(thisEp->position);
	}
}
