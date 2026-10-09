/* routines which check an object to see if it is in the player's line of sight */
extern int CameraCanSeeThisPosition_WithIgnore(DISPLAYBLOCK *ignoredObjectPtr,VECTORCH *positionPtr);

extern void CheckForViewVectorIntersectionWith3dObject(DISPLAYBLOCK *dPtr);

extern void CheckForVectorIntersectionWith3dObject(DISPLAYBLOCK *objectPtr, VECTORCH *viewVectorAlphaPtr, VECTORCH *viewVectorBetaPtr, int rigorous);

/* General line of sight routine: (written for Roxby!) */
extern int IsThisObjectVisibleFromThisPosition(DISPLAYBLOCK *objectPtr,VECTORCH *positionPtr,int maxRange);
/*KJL****************************************************
* 	dPtr -			DISPLAYBLOCK* target                *
* 	positionPtr - 	VECTORCH* co-ord from which to look *
* 	maxRange -		int maximum range in metres to look *
****************************************************KJL*/
	
/* KJL 18:17:46 16/05/98 - check for an object's visibility, but ignore an object - e.g. ignore the character which is looking */
extern int IsThisObjectVisibleFromThisPosition_WithIgnore(DISPLAYBLOCK *objectPtr,DISPLAYBLOCK *ignoredObjectPtr,VECTORCH *positionPtr,int maxRange);

void FindPolygonInLineOfSight(VECTORCH *viewpointDirectionPtr, VECTORCH *viewpointPositionPtr, int useOnScreenBlockList, DISPLAYBLOCK *objectToIgnorePtr);
void FindPolygonInLineOfSight_TwoIgnores(VECTORCH *viewpointDirectionPtr, VECTORCH *viewpointPositionPtr, int useOnScreenBlockList, DISPLAYBLOCK *objectToIgnorePtr,DISPLAYBLOCK *next_objectToIgnorePtr);

/* Line Of Sight data */
extern VECTORCH 		LOS_Point;	 		/* point in world space which player has hit */
extern int 				LOS_Lambda;			/* distance in mm to point from player */
extern DISPLAYBLOCK*	LOS_ObjectHitPtr;	/* pointer to object that was hit */
extern VECTORCH			LOS_ObjectNormal;	/* normal of the object's face which was hit */
extern SECTION_DATA*	LOS_HModel_Section;	/* Section of HModel hit */

/*KJL**********************************************************************************
* The interface for this function is a bit muddle since it was originally a static fn *
* used by	the dynamics system.                                                      *
*                                                                                     *
* Input:                                                     (in world space)         *
* 		dPtr				- target object                  (in world space)         *
* 		viewVectorAlphaPtr 	- starting point of view-line                             *
* 		viewVectorBetaPtr	- direction of view-line (NORMALISED)                     *
*																					  *
*		WARNING! the contents of the above vectors will be changed!					  *
*                                                                                     *
* 		also set                                                                      *
* 		                                                                              *
* 		LOS_ObjectHitPtr 	- to zero (GLOBAL VARIABLE)                               *
* 		LOS_Lambda 			- to max range in mm (GLOBAL VARIABLE)                    *
*                                                                                     *
* Output:                                                                             *
* 		LOS_ObjectHitPtr	- dPtr of hit object                                      *
* 		LOS_Lambda			- distance to hit polygon (or max if no hit)              *
*		LOS_Point			- world space coords of intersection of line & hit poly   *
*		LOS_ObjectNormal	- normal of the hit polygon								  *
**********************************************************************************KJL*/
