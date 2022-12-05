#ifndef _dynamics_h_ /* KJL 17:23:01 11/05/96 - is this your first time? */
#define _dynamics_h_ 1
#include "particle.h"

/*KJL************************************************************************
* DYNAMICS.H                                                                *
* 			- this file contains prototypes for the functions in dynamics.c *
* 			which can be called	externally.                                 *
************************************************************************KJL*/


/*KJL****************************************************************************************
* 								       S T R U C T U R E S 	 								*
****************************************************************************************KJL*/
struct ColPolyTag
{
	int NumberOfVertices;
	VECTORCH PolyPoint[4];
    VECTORCH PolyNormal;
	DISPLAYBLOCK *ParentObject;
};


/*KJL****************************************************************************************
*                             P H Y S I C A L   C O N S T A N T S                           *
****************************************************************************************KJL*/

#define GRAVITY_STRENGTH 25000
#define TIME_BEFORE_GRAVITY_KICKS_IN 16384

#define MAXIMUM_STEP_HEIGHT 450
#define MAX_ANG_VELOCITY 8192
#define MAX_MOVES 4

//16384

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void ObjectDynamics(void);
extern void DynamicallyRotateObject(DYNAMICSBLOCK *dynPtr);


/* externs to shape access fns (platform specific) */
extern int SetupPolygonAccess(DISPLAYBLOCK *objectPtr);
extern void AccessNextPolygon(void);
extern void GetPolygonVertices(struct ColPolyTag *polyPtr);
extern void GetPolygonNormal(struct ColPolyTag *polyPtr);



/* extra camera movement */
extern EULER HeadOrientation;

extern int ParticleDynamics(PARTICLE *particlePtr, VECTORCH *obstacleNormalPtr, int *moduleIndexPtr);
void AddEffectsOfForceGenerators(VECTORCH *positionPtr, VECTORCH *impulsePtr, int mass);

#endif /* end of preprocessor condition for file wrapping */
