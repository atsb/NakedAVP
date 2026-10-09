#include "3dc.h"
#include "dynblock.h"

#define UseLocalAssert No
#include "ourasert.h"


/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
void InitialiseDynamicsBlocks(void);
DYNAMICSBLOCK* AllocateDynamicsBlock(enum DYNAMICS_TEMPLATE_ID templateID);
void DeallocateDynamicsBlock(DYNAMICSBLOCK *dynPtr);

void InitialiseCollisionReports(void);
COLLISIONREPORT* AllocateCollisionReport(DYNAMICSBLOCK* dynPtr);
																									  
static DYNAMICSBLOCK DynBlockStorage[MAX_NO_OF_DYNAMICS_BLOCKS];
static int NumFreeDynBlocks;
static DYNAMICSBLOCK *FreeDynBlockList[MAX_NO_OF_DYNAMICS_BLOCKS];
static DYNAMICSBLOCK **FreeDynBlockListPtr;

static COLLISIONREPORT CollisionReportStorage[MAX_NO_OF_COLLISION_REPORTS];
int NumFreeCollisionReports;

static DYNAMICSBLOCK DynamicsTemplate[]=
{
	/* DYNAMICS_TEMPLATE_MARINE_PLAYER */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,65536,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
        
       	0,/* int Friction; */
    	0,/* int Elasticity; */
		80*2, /* int Mass */

		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_NONE,

		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		1,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
		

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_ALIEN_NPC */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */
		
		{0,65536,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
       
       	1,/* int Friction; */
    	0,/* int Elasticity;	*/
		100, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_ALIEN,
		
		1,/* GravityOn :1; */
		0,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		1,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */


		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
    /* DYNAMICS_TEMPLATE_GRENADE */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
       
       	0,/* int Friction; */
    	32768/2,/* int Elasticity; */
		20, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_FULL,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		1,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
    /* DYNAMICS_TEMPLATE_ROCKET & predator disc weapon */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */
       
        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
       
       	0,/* int Friction; */
    	65536,/* int Elasticity; */
		40, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_NONE,
		
		0,/* GravityOn :1; */
		0,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		1,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		1,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_DEBRIS */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		30, /* int Mass */
		
		DYN_TYPE_NO_COLLISIONS,
		TOPPLE_FORCE_NONE,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		1,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_STATIC */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		100, /* int Mass */
		
	   	DYN_TYPE_NRBB_COLLISIONS,
	   	TOPPLE_FORCE_NONE,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		1,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_INANIMATE */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	1,/* int Friction; */
    	16384,/* int Elasticity;	*/
		65536, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_FULL,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		1,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_PICKUPOBJECT */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	16384,/* int Elasticity;	*/
		20, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_FULL,
		
		0,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		1,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_SPRITE_NPC */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,65536,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
        
       	1,/* int Friction; */
    	0,/* int Elasticity; */
		80*2, /* int Mass */

		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_NONE,

		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		1,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_STATIC_SPRITE */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{0,0,0,0,0,0,0,0,0},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,65536,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */
        
       	1,/* int Friction; */
    	0,/* int Elasticity; */
		80*2, /* int Mass */

		DYN_TYPE_SPRITE_COLLISIONS,
		TOPPLE_FORCE_NONE,

		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		1,/* CanClimbStairs :1; */
		1,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_PLATFORM_LIFT */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		1, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_NONE,
		
		0,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		1,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		1,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		1,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_ALIEN_DEBRIS */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		1, /* int Mass */
		
		DYN_TYPE_NRBB_COLLISIONS,
		TOPPLE_FORCE_NONE,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		1,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		1,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_ACID_SMOKE */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,-ONE_FIXED,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		1, /* int Mass */
		
		DYN_TYPE_NO_COLLISIONS,
		TOPPLE_FORCE_NONE,
		
		1,/* GravityOn :1; */
		0,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        0,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */
        
		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},
	/* DYNAMICS_TEMPLATE_NET_GHOST */
	{
		{0,0,0},/* EULER OrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	OrientMat - Local -> World Orientation Matrix */
		{0,0,0},/* EULER PrevOrientEuler - Euler Orientation */
		{65536,0,0,0,65536,0,0,0,65536},/* MATRIXCH	PrevOrientMat - Local -> World Orientation Matrix */

		{0,0,0},/* VECTORCH	Position */
		{0,0,0},/* VECTORCH	PrevPosition */

		{0,0,0},/* VECTORCH	LinVelocity */
		{0,0,0},/* VECTORCH LinImpulse */
		
		{0,0,0},/* EULER AngVelocity */
		{0,0,0},/* EULER AngImpulse */

        NULL, /* struct collisionreport *CollisionReportPtr; */

		{0,0,0},/* VECTORCH	GravityDirection */
		0, /* int TimeNotInContactWithFloor */

       	0,/* int Friction; */
    	0,/* int Elasticity;	*/
		100, /* int Mass */
		
	   	DYN_TYPE_NRBB_COLLISIONS,
	   	TOPPLE_FORCE_NONE,
		
		1,/* GravityOn :1; */
		1,/* UseStandardGravity :1 - ie. in direction of increasing Y */
		0,/* StopOnCollision :1; */
		0,/* CanClimbStairs :1; */
		0,/* IsStatic :1; */
		0,/* OnlyCollideWithObjects :1; */
        1,/* IsNetGhost :1; */
		0,/* IgnoreSameObjectsAsYou :1; */
		0,/* IgnoreThePlayer :1; */
		0,/* UseDisplacement :1; */
		0,/* OnlyCollideWithEnvironment :1; */

		0,/* IsInContactWithFloor :1 */
		0,/* IsInContactWithNearlyFlatFloor */
		0,/* RequestsToStandUp :1 */
		0,/* IsFloating :1 */
		0,/* IsPickupObject :1 */
		0,/* IsInanimate :1; */
		0,/* IgnoresNotVisPolys :1; */
	},

	


};
    
    
/*KJL***************************************************************************
* FUNCTIONS TO ALLOCATE AND DEALLOCATE DYNAMICS BLOCKS - KJL 12:02:14 11/13/96 *
***************************************************************************KJL*/
void InitialiseDynamicsBlocks(void)
{
	DYNAMICSBLOCK *freeBlockPtr = DynBlockStorage;
	int blk;

	for(blk=0; blk < MAX_NO_OF_DYNAMICS_BLOCKS; blk++) 
	{								
		FreeDynBlockList[blk] = freeBlockPtr++;
	}

	FreeDynBlockListPtr = &FreeDynBlockList[MAX_NO_OF_DYNAMICS_BLOCKS-1];
	NumFreeDynBlocks = MAX_NO_OF_DYNAMICS_BLOCKS;
}


DYNAMICSBLOCK* AllocateDynamicsBlock(enum DYNAMICS_TEMPLATE_ID templateID)
{
	DYNAMICSBLOCK *dynPtr = 0; /* Default to null ptr */

	if (NumFreeDynBlocks) 
	{
		dynPtr = *FreeDynBlockListPtr--;
		NumFreeDynBlocks--;
		GLOBALASSERT(templateID>=0);
		GLOBALASSERT(templateID<MAX_NO_OF_DYNAMICS_TEMPLATES);
		*dynPtr = DynamicsTemplate[templateID];	
	}
	else
	{
		/* unable to allocate a dynamics block I'm afraid; 
		   MAX_NO_OF_DYNAMICS_BLOCKS is too low */
   		LOCALASSERT(NumFreeDynBlocks);
	}

	return dynPtr;
}


void DeallocateDynamicsBlock(DYNAMICSBLOCK *dynPtr)
{
	GLOBALASSERT(dynPtr);
	*(++FreeDynBlockListPtr) = dynPtr;
	NumFreeDynBlocks++;
}

/*KJL***************************************************************************
* FUNCTIONS TO INITIALISE & ALLOCATE COLLISION REPORTS - KJL 12:17:13 11/19/96 *
***************************************************************************KJL*/
void InitialiseCollisionReports(void)
{
	NumFreeCollisionReports = MAX_NO_OF_COLLISION_REPORTS;
}


COLLISIONREPORT* AllocateCollisionReport(DYNAMICSBLOCK* dynPtr)
{
	COLLISIONREPORT *newReportPtr = 0; /* Default to null ptr */
	GLOBALASSERT(dynPtr);

	if (NumFreeCollisionReports) 
	{
		NumFreeCollisionReports--;
		newReportPtr = &CollisionReportStorage[NumFreeCollisionReports];
	    
	    if (dynPtr->CollisionReportPtr) /* already some reports */
	    {
			COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;

	        /* search for the last report */
	        while(reportPtr->NextCollisionReportPtr)
	        	reportPtr = reportPtr->NextCollisionReportPtr;
	    	reportPtr->NextCollisionReportPtr = newReportPtr;
	    } 
	    else /* object's first report */
	   	{
	    	dynPtr->CollisionReportPtr = newReportPtr;
	    }

	    /* make report the end of the list */
	    newReportPtr->NextCollisionReportPtr=0;
	}
	else
	{
		/* unable to allocate a collision block I'm afraid */
		LOCALASSERT(1==0);
	}

	return newReportPtr;
}

