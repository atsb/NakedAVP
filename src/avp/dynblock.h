#ifndef _dynblock_h_ /* Is this your first time? */
#define _dynblock_h_ 1

/*KJL****************************************************************************************
* 								       S T R U C T U R E S 	 								*
****************************************************************************************KJL*/
enum DYN_TYPE
{
	DYN_TYPE_NO_COLLISIONS,
	DYN_TYPE_SPRITE_COLLISIONS,
	DYN_TYPE_SPHERE_COLLISIONS,	
	DYN_TYPE_NRBB_COLLISIONS,
	DYN_TYPE_CUBOID_COLLISIONS,
};

enum TOPPLE_FORCE
{
	TOPPLE_FORCE_NONE,
	TOPPLE_FORCE_ALIEN,
	TOPPLE_FORCE_FULL
};

/* KJL 17:19:14 11/05/96 - my 'dynamicsblock'. I'll use the enginesque lower/upper case naming convention */
typedef struct dynamicsblock
{
	/* representations of orientation of object */
	EULER		OrientEuler;	/* Euler Orientation */
	MATRIXCH	OrientMat;		/* Local -> World Orientation Matrix */
	EULER		PrevOrientEuler;	/* Euler Orientation */
	MATRIXCH	PrevOrientMat;		/* Local -> World Orientation Matrix */

	/* position in World Space (units mm) */
	VECTORCH	Position;
	VECTORCH	PrevPosition;

	/* component of velocity (in World Space, units mm per sec) due to internal forces
	   eg. a player walking forward, a rocket's thrust - set by strategies */
	VECTORCH	LinVelocity;
    /* component of velocity (in World Space, units mm per sec) due to external forces
	   eg. gravity, explosions, jumping - set by dynamics system & strategies */
	VECTORCH	LinImpulse;
    
	/* angular velocity in World Space */
	EULER		AngVelocity;
	/* rotational effects due to external forces */
	EULER		AngImpulse;
	   
    /* pointer to report(s) on last frame's collisions (singly-linked list) */
    struct collisionreport *CollisionReportPtr;

	/* object's normalised gravity vector - used if UseStandardGravity is set to false */
	VECTORCH	GravityDirection; 
	int			TimeNotInContactWithFloor;

    /* physical constants */
    int			Friction;	/* difficult to set a scale as yet */
    int			Elasticity;	/* 0 = perfectly inelastic, 65536 = perfectly elastic */
    int			Mass;		/* integer in kg */

	/* collision flags */
	/* eg. can go up steps, cuboid model etc */
	
	enum DYN_TYPE DynamicsType;
	enum TOPPLE_FORCE ToppleForce;

	unsigned int GravityOn :1;
	unsigned int UseStandardGravity :1;	/* ie. in direction of increasing Y */
	unsigned int StopOnCollision :1;  /* eg. missiles stop as soon as bthey hit something; players don't */
    unsigned int CanClimbStairs :1; 
    unsigned int IsStatic :1;
	unsigned int OnlyCollideWithObjects :1;
	unsigned int IsNetGhost :1;
	unsigned int IgnoreSameObjectsAsYou :1; /* don't collide with objects which have the same behaviour type */
	unsigned int IgnoreThePlayer :1;
	unsigned int UseDisplacement :1;
	unsigned int OnlyCollideWithEnvironment :1;

	unsigned int IsInContactWithFloor :1;
	unsigned int IsInContactWithNearlyFlatFloor :1;
	unsigned int RequestsToStandUp :1;
	unsigned int IsFloating :1;
	unsigned int IsPickupObject :1;
	unsigned int IsInanimate :1;
	unsigned int IgnoresNotVisPolys :1;
	

	/* FOR INTERNAL USE ONLY */
	int			CollisionRadius;
	int			DistanceLeftToMove;
	VECTORCH	Displacement;
	VECTORCH	ObjectVertices[8]; /* vertices of the cuboid which describes the object */

} DYNAMICSBLOCK;


enum DYNAMICS_TEMPLATE_ID
{
	DYNAMICS_TEMPLATE_MARINE_PLAYER,
	DYNAMICS_TEMPLATE_ALIEN_NPC,
    DYNAMICS_TEMPLATE_GRENADE,
    DYNAMICS_TEMPLATE_ROCKET,
	DYNAMICS_TEMPLATE_DEBRIS,
	DYNAMICS_TEMPLATE_STATIC,
	DYNAMICS_TEMPLATE_INANIMATE,
	DYNAMICS_TEMPLATE_PICKUPOBJECT,
	DYNAMICS_TEMPLATE_SPRITE_NPC,
	DYNAMICS_TEMPLATE_STATIC_SPRITE,
	DYNAMICS_TEMPLATE_PLATFORM_LIFT,
	DYNAMICS_TEMPLATE_ALIEN_DEBRIS,
	DYNAMICS_TEMPLATE_ACID_SMOKE,
	DYNAMICS_TEMPLATE_NET_GHOST,
	
	MAX_NO_OF_DYNAMICS_TEMPLATES
};

typedef struct collisionreport
{
	/* strategy block of whatever you've hit - this is null if you've hit the landscape */
	struct strategyblock *ObstacleSBPtr;

    /* the normal of the obstacle's face which you've hit */
    VECTORCH ObstacleNormal;
	VECTORCH ObstaclePoint;

	/* ptr to the next report, null if there isn't one */
	struct collisionreport *NextCollisionReportPtr;

} COLLISIONREPORT;

#define MAX_NO_OF_DYNAMICS_BLOCKS maxstblocks
#define MAX_NO_OF_COLLISION_REPORTS 400
/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void InitialiseDynamicsBlocks(void);
extern DYNAMICSBLOCK* AllocateDynamicsBlock(enum DYNAMICS_TEMPLATE_ID templateID);
extern void DeallocateDynamicsBlock(DYNAMICSBLOCK *dynPtr);

extern void InitialiseCollisionReports(void);
extern COLLISIONREPORT* AllocateCollisionReport(DYNAMICSBLOCK* dynPtr);

#endif /* end of preprocessor condition for file wrapping */
