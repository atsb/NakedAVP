/*KJL*************************
* los.c - Line of sight code *
*************************KJL*/
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
#define UseLocalAssert Yes
#include "ourasert.h"
#include "dxlog.h"
#include "showcmds.h"
#include "targeting.h"
#include "los.h"

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
int IsThisObjectVisibleFromThisPosition(DISPLAYBLOCK *dPtr,VECTORCH *positionPtr,int maxRange);
void CheckForVectorIntersectionWithHierarchicalObject(DISPLAYBLOCK *dPtr, VECTORCH *viewVectorAlphaPtr, VECTORCH *viewVectorBetaPtr);
void CheckForRayIntersectionWithHierarchy(DISPLAYBLOCK *objectPtr, SECTION_DATA *sectionDataPtr);
void CheckForRayIntersectionWithObject(DISPLAYBLOCK *dPtr);
/*KJL****************************************************************************************
* 										D E F I N E S 										*
****************************************************************************************KJL*/
#define POLY_REJECT_RANGE 500

extern MORPHDISPLAY MorphDisplay;
extern VECTORCH MorphedPts[];
extern VECTORCH *ShapePointsPtr;
extern int *ShapeNormalsPtr;
extern int *Shape2NormalsPtr;
extern char ShapeIsMorphed;
extern int **ItemArrayPtr;
extern POLYHEADER *PolyheaderPtr;

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

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
extern DISPLAYBLOCK *Player;

/* unnormalised vector in the direction	which the gun's muzzle is pointing, IN VIEW SPACE */
/* very useful when considering sprites, which lie in a Z-plane in view space */
extern VECTORCH GunMuzzleDirectionInVS;
/* dir gun is pointing, normalised and IN WORLD SPACE */
extern VECTORCH GunMuzzleDirectionInWS;

extern DISPLAYBLOCK PlayersWeapon;


extern int NumOnScreenBlocks;
extern DISPLAYBLOCK *OnScreenBlockList[];
extern int NumActiveBlocks;
extern DISPLAYBLOCK *ActiveBlockList[];


static VECTORCH *ViewpointDirectionPtr;
static VECTORCH *ViewpointPositionPtr;


/*KJL****************************************************************************************
*                                     CODE STARTS HERE!                                     *
****************************************************************************************KJL*/
int CameraCanSeeThisPosition_WithIgnore(DISPLAYBLOCK *ignoredObjectPtr,VECTORCH *positionPtr)
{
	VECTORCH viewVector;  /* direction of view-line */

	GLOBALASSERT(ignoredObjectPtr);

	/* try to look at the centre of the camera */
	viewVector = Global_VDB_Ptr->VDB_World;
   	
   	viewVector.vx -= positionPtr->vx;
	viewVector.vy -= positionPtr->vy;
	viewVector.vz -= positionPtr->vz;
	Normalise(&viewVector);

	FindPolygonInLineOfSight(&viewVector, positionPtr, 0,ignoredObjectPtr);
	
		
	if (LOS_ObjectHitPtr == Player) return 1;
	else return 0;
}

int IsThisObjectVisibleFromThisPosition_WithIgnore(DISPLAYBLOCK *objectPtr,DISPLAYBLOCK *ignoredObjectPtr,VECTORCH *positionPtr,int maxRange)
{
	VECTORCH viewVector;  /* direction of view-line */

	GLOBALASSERT(objectPtr);

	/* try to look at the centre of the object */
	GetTargetingPointOfObject(objectPtr, &viewVector);
   	
   	viewVector.vx -= positionPtr->vx;
	viewVector.vy -= positionPtr->vy;
	viewVector.vz -= positionPtr->vz;
	Normalise(&viewVector);

	FindPolygonInLineOfSight(&viewVector, positionPtr, 0,ignoredObjectPtr);
	
		
	if (LOS_ObjectHitPtr == objectPtr) return 1;
	else return 0;
}

int IsThisObjectVisibleFromThisPosition(DISPLAYBLOCK *objectPtr,VECTORCH *positionPtr,int maxRange)
{	 
	VECTORCH viewVector;  /* direction of view-line */

	GLOBALASSERT(objectPtr);

	/* try to look at the centre of the object */
	GetTargetingPointOfObject(objectPtr, &viewVector);
	
   	viewVector.vx -= positionPtr->vx;
	viewVector.vy -= positionPtr->vy;
	viewVector.vz -= positionPtr->vz;
	Normalise(&viewVector);

	FindPolygonInLineOfSight(&viewVector, positionPtr, 0,0);
	
		
	if (LOS_ObjectHitPtr == objectPtr) return 1;
	else return 0;
}


void CheckForVectorIntersectionWith3dObject(DISPLAYBLOCK *objectPtr, VECTORCH *viewVectorAlphaPtr, VECTORCH *viewVectorBetaPtr, int rigorous)
{
	ViewpointPositionPtr = viewVectorAlphaPtr;
	ViewpointDirectionPtr = viewVectorBetaPtr;
	    
	if (objectPtr->HModelControlBlock)
	{
		SECTION_DATA *firstSectionPtr;
	  	firstSectionPtr=objectPtr->HModelControlBlock->section_data;
	  	LOCALASSERT(firstSectionPtr);
		if ( !(
			(objectPtr->ObWorld.vx<1000000 && objectPtr->ObWorld.vx>-1000000)
		 &&	(objectPtr->ObWorld.vy<1000000 && objectPtr->ObWorld.vy>-1000000)
		 &&	(objectPtr->ObWorld.vz<1000000 && objectPtr->ObWorld.vz>-1000000) 
		 ) )
		{
			return;
		}
	  	LOCALASSERT(objectPtr->ObWorld.vx<1000000 && objectPtr->ObWorld.vx>-1000000);
	  	LOCALASSERT(objectPtr->ObWorld.vy<1000000 && objectPtr->ObWorld.vy>-1000000);
	  	LOCALASSERT(objectPtr->ObWorld.vz<1000000 && objectPtr->ObWorld.vz>-1000000);
	  	CheckForRayIntersectionWithHierarchy(objectPtr,firstSectionPtr);
	}  
	else
	{
		CheckForRayIntersectionWithObject(objectPtr);
	}
}









/* KJL 15:26:08 14/05/98 - FindPolygonInLineOfSight

	Using either the Active or OnScreen Block list, find which polygon you would hit first
	if a ray of light was cast out from a given point in world space (viewpointPositionPtr)
	in a given direction (viewpointDirectionPtr).

	The following are set:

	LOS_ObjectHitPtr	- dispPtr of hit object (NULL if none found)

	LOS_Lambda			- distance to hit polygon (or max if no hit)           
	LOS_Point			- world space coords of intersection of ray & hit poly
	LOS_ObjectNormal	- normal of the hit polygon
	LOS_HModel_Section	- hierarchical section that was hit (or NULL if not hmodel)

*/
void FindPolygonInLineOfSight(VECTORCH *viewpointDirectionPtr, VECTORCH *viewpointPositionPtr, int useOnScreenBlockList, DISPLAYBLOCK *objectToIgnorePtr) {
	
	/* Shell function. */
	FindPolygonInLineOfSight_TwoIgnores(viewpointDirectionPtr,viewpointPositionPtr,useOnScreenBlockList,objectToIgnorePtr,NULL);
	/* No, it's not recursive! */
}


void FindPolygonInLineOfSight_TwoIgnores(VECTORCH *viewpointDirectionPtr, VECTORCH *viewpointPositionPtr, int useOnScreenBlockList, DISPLAYBLOCK *objectToIgnorePtr,DISPLAYBLOCK *next_objectToIgnorePtr)
{
	DISPLAYBLOCK **displayBlockList;
   	int numberOfObjects;
   	
   	if (useOnScreenBlockList)
   	{
		numberOfObjects = NumOnScreenBlocks;
		displayBlockList = OnScreenBlockList;
	}
	else
   	{
   		numberOfObjects = NumActiveBlocks;
		displayBlockList = ActiveBlockList;
	}

	/* initialise the LoS data */

  	LOS_Lambda=10000000;
	LOS_ObjectHitPtr = 0;
	LOS_HModel_Section= 0;
	ViewpointPositionPtr = viewpointPositionPtr;
	ViewpointDirectionPtr = viewpointDirectionPtr;

	    
   	/* scan throught each object */
   	while (numberOfObjects--)
	{
		DISPLAYBLOCK* objectPtr = displayBlockList[numberOfObjects];
		
		if ((objectPtr == objectToIgnorePtr)||(objectPtr == next_objectToIgnorePtr)) continue;

		/* if hierarchical model, consider each object in the model separately */
		if (objectPtr->HModelControlBlock)
		{
			SECTION_DATA *firstSectionPtr;
		  	firstSectionPtr=objectPtr->HModelControlBlock->section_data;

		  	LOCALASSERT(firstSectionPtr);
			if ( !(
				(objectPtr->ObWorld.vx<1000000 && objectPtr->ObWorld.vx>-1000000)
			 &&	(objectPtr->ObWorld.vy<1000000 && objectPtr->ObWorld.vy>-1000000)
			 &&	(objectPtr->ObWorld.vz<1000000 && objectPtr->ObWorld.vz>-1000000) 
			 ) )
			 {
				continue;
				
				LOGDXFMT(("Pre assertions check for having processed the section...\n"));
				LOGDXFMT(("State of initialised flag: %d\n",(firstSectionPtr->flags&section_data_initialised)));
				LOGDXFMT(("Name of section: %s\n",firstSectionPtr->sempai->Section_Name));
				LOGDXFMT(("It was playing sequence: %d,%d\n",firstSectionPtr->my_controller->Sequence_Type,
					firstSectionPtr->my_controller->Sub_Sequence));
				LOGDXFMT(("ObWorld %d,%d,%d\n",objectPtr->ObWorld.vx,objectPtr->ObWorld.vy,objectPtr->ObWorld.vz));

				{
					DYNAMICSBLOCK *dynPtr=objectPtr->ObStrategyBlock->DynPtr;
					LOCALASSERT(dynPtr);
					LOGDXFMT(("DynPtr->Position %d,%d,%d\n",dynPtr->Position.vx,dynPtr->Position.vy,dynPtr->Position.vz));
				}
				
				LOCALASSERT(firstSectionPtr->flags&section_data_initialised);
				LOCALASSERT(objectPtr->ObWorld.vx<1000000 && objectPtr->ObWorld.vx>-1000000);
				LOCALASSERT(objectPtr->ObWorld.vy<1000000 && objectPtr->ObWorld.vy>-1000000);
				LOCALASSERT(objectPtr->ObWorld.vz<1000000 && objectPtr->ObWorld.vz>-1000000);
				
			}
		  	CheckForRayIntersectionWithHierarchy(objectPtr,firstSectionPtr);
		}  
		else
		{
			CheckForRayIntersectionWithObject(objectPtr);
		}
	}
}

void CheckForRayIntersectionWithHierarchy(DISPLAYBLOCK *objectPtr, SECTION_DATA *sectionDataPtr)
{
	SECTION *sectionPtr;

	/* LOS check. */

	sectionPtr=sectionDataPtr->sempai;
		
	/* Unreal things can't be hit... */

	if (!(sectionDataPtr->flags&section_data_notreal) && (sectionPtr->Shape!=NULL))
	{
		DISPLAYBLOCK dummy_displayblock;
		
		dummy_displayblock.ObShape=sectionPtr->ShapeNum;
		dummy_displayblock.ObShapeData=sectionPtr->Shape;
		dummy_displayblock.ObWorld=sectionDataPtr->World_Offset;
		dummy_displayblock.ObMat=sectionDataPtr->SecMat;

		dummy_displayblock.ObRadius=0;
		dummy_displayblock.ObMaxX=0;
		dummy_displayblock.ObMinX=0;
		dummy_displayblock.ObMaxY=0;
		dummy_displayblock.ObMinY=0;
		dummy_displayblock.ObMaxZ=0;
		dummy_displayblock.ObMinZ=0;

		dummy_displayblock.ObTxAnimCtrlBlks=NULL;
		dummy_displayblock.ObEIDPtr=NULL;
		dummy_displayblock.ObMorphCtrl=NULL;
		dummy_displayblock.ObStrategyBlock=objectPtr->ObStrategyBlock;
		dummy_displayblock.ShapeAnimControlBlock=sectionDataPtr->sac_ptr;
		dummy_displayblock.HModelControlBlock=NULL; /* Don't even want to think about that. */
		dummy_displayblock.ObMyModule=NULL;

		/* KJL 21:12:11 12/11/98 - arg! ObFlags wasn't set */
		dummy_displayblock.ObFlags = 0;

		if ( !(
			(dummy_displayblock.ObWorld.vx<1000000 && dummy_displayblock.ObWorld.vx>-1000000)
		 &&	(dummy_displayblock.ObWorld.vy<1000000 && dummy_displayblock.ObWorld.vy>-1000000)
		 &&	(dummy_displayblock.ObWorld.vz<1000000 && dummy_displayblock.ObWorld.vz>-1000000) 
		 ) ) {
	
			LOGDXFMT(("Pre assertions check for having processed the section...\n"));
			LOGDXFMT(("State of initialised flag: %x\n",(sectionDataPtr->flags&section_data_initialised)));
			LOGDXFMT(("Name of section: %s\n",sectionDataPtr->sempai->Section_Name));
			LOGDXFMT(("It was playing sequence: %d,%d\n",sectionDataPtr->my_controller->Sequence_Type,
				sectionDataPtr->my_controller->Sub_Sequence));
			LOGDXFMT(("ObWorld %d,%d,%d\n",dummy_displayblock.ObWorld.vx,dummy_displayblock.ObWorld.vy,dummy_displayblock.ObWorld.vz));

			{
				DYNAMICSBLOCK *dynPtr=objectPtr->ObStrategyBlock->DynPtr;
				LOCALASSERT(dynPtr);
				LOGDXFMT(("DynPtr->Position %d,%d,%d\n",dynPtr->Position.vx,dynPtr->Position.vy,dynPtr->Position.vz));
			}

			LOCALASSERT(sectionDataPtr->flags&section_data_initialised);
			LOCALASSERT(dummy_displayblock.ObWorld.vx<1000000 && dummy_displayblock.ObWorld.vx>-1000000);
			LOCALASSERT(dummy_displayblock.ObWorld.vy<1000000 && dummy_displayblock.ObWorld.vy>-1000000);
			LOCALASSERT(dummy_displayblock.ObWorld.vz<1000000 && dummy_displayblock.ObWorld.vz>-1000000);
			
		}
		
		CheckForRayIntersectionWithObject(&dummy_displayblock);
		
		if (LOS_ObjectHitPtr == &dummy_displayblock)
		{
			/* ah, we've hit this object */
			LOS_ObjectHitPtr = objectPtr;
			LOS_HModel_Section = sectionDataPtr;
		}
	}

	/* Now call recursion... */
	if (sectionDataPtr->First_Child!=NULL)
	{
		SECTION_DATA *childrenListPtr = sectionDataPtr->First_Child;

		while (childrenListPtr!=NULL)
		{
			CheckForRayIntersectionWithHierarchy(objectPtr,childrenListPtr);
			childrenListPtr=childrenListPtr->Next_Sibling;
		}
	}

}




void CheckForRayIntersectionWithObject(DISPLAYBLOCK *dPtr)
{
	int numberOfItems;
	VECTORCH viewVectorAlpha = *ViewpointPositionPtr;
	VECTORCH viewVectorBeta = *ViewpointDirectionPtr;
	VECTORCH position;
	int needToRotate;
	
	/* check for a valid object */
	{
		STRATEGYBLOCK* sbPtr;				
		GLOBALASSERT(dPtr);
		
		/* can it be seen? */
		if((dPtr->ObFlags&ObFlag_NotVis)&&(dPtr!=Player)) return;
		/* KJL 21:18:44 23/05/98 - ugh. Currently the player's bounding box is notvis; I need a rethink */

		/* any hierarchical models should have been split up by now */
		LOCALASSERT(dPtr->HModelControlBlock==NULL);

		/* no shape? */
		if (!dPtr->ObShape && dPtr->SfxPtr) return;

		sbPtr = dPtr->ObStrategyBlock;

		/* test for objects we're not interested in */
   		if (sbPtr)
		{
			if (sbPtr->DynPtr)
			{
				/* ignore it if it's a non-collideable object, eg. debris */
				if(sbPtr->DynPtr->DynamicsType == DYN_TYPE_NO_COLLISIONS)
					return;
			}
		}
	}
	
	/* check objects position is sensible */
	#if 0
	LOCALASSERT(dPtr->ObWorld.vx<1000000 && dPtr->ObWorld.vx>-1000000);
	LOCALASSERT(dPtr->ObWorld.vy<1000000 && dPtr->ObWorld.vy>-1000000);
	LOCALASSERT(dPtr->ObWorld.vz<1000000 && dPtr->ObWorld.vz>-1000000);
	#else
	if(dPtr->ObWorld.vx>1000000 || dPtr->ObWorld.vx<-1000000) return;
	if(dPtr->ObWorld.vy>1000000 || dPtr->ObWorld.vy<-1000000) return;
	if(dPtr->ObWorld.vz>1000000 || dPtr->ObWorld.vz<-1000000) return;
	#endif
	if (dPtr==Player)
	{
		position = dPtr->ObStrategyBlock->DynPtr->Position;
	}
	else
	{
		position = dPtr->ObWorld;
	}
	/* transform view line into shape space */
	viewVectorAlpha.vx -= position.vx;
	viewVectorAlpha.vy -= position.vy;
	viewVectorAlpha.vz -= position.vz;
	
	#if 1
	if (dPtr!=Player)
	{
		if (MagnitudeOfCrossProduct(&viewVectorAlpha,&viewVectorBeta)>dPtr->ObShapeData->shaperadius)
			return;
	}
	#endif

	/* if we're not dealing with a module, it's probably rotated */
	if(!dPtr->ObMyModule&&dPtr!=Player)
	{
		needToRotate = 1;
	}
	else
	{
		needToRotate = 0;
	}

	if (needToRotate)
	{
		MATRIXCH matrix = dPtr->ObMat;
		TransposeMatrixCH(&matrix);
		RotateVector(&viewVectorBeta,&matrix);
		RotateVector(&viewVectorAlpha,&matrix);
	}

	numberOfItems = SetupPolygonAccess(dPtr);
	if (!dPtr->ObShape)
	{
//		PrintDebuggingText("polys %d\n",numberOfItems);
	}

  	while(numberOfItems--)
	{
		extern POLYHEADER *PolyheaderPtr;
		VECTORCH polyNormal;
		struct ColPolyTag polyData;
		VECTORCH pointOnPlane;
		int lambda;
		int axis1;
		int axis2;
        
		/* scanning through polys */
		AccessNextPolygon();
		
		if( (PolyheaderPtr->PolyFlags & iflag_notvis) && !(PolyheaderPtr->PolyFlags & iflag_mirror)) continue;


		{
			int normDotBeta;

			GetPolygonNormal(&polyData);
			polyNormal = polyData.PolyNormal;
            
			normDotBeta	= DotProduct(&(polyNormal),&viewVectorBeta);
			{
				/* if the polygon is flagged as double-sided, and it's pointing
				the wrong way, consider it to be flipped round */
				//if((PolyheaderPtr->PolyFlags) & iflag_no_bfc)
				/* KJL 16:06:11 10/07/98 - treat all polys as no bfc */
				{
					if (normDotBeta>0)
					{
						normDotBeta=-normDotBeta;
						polyNormal.vx = -polyNormal.vx;
						polyNormal.vy = -polyNormal.vy;
						polyNormal.vz = -polyNormal.vz;
					}
				}
			}
			
			/* trivial rejection of poly if it is not facing LOS */
	   		if (normDotBeta>-POLY_REJECT_RANGE)
	   		{
	   			continue;
			}
            
            GetPolygonVertices(&polyData);
			/* calculate coords of plane-line intersection */
			{
				int d;
				{
					/* get a pt in the poly */
					VECTORCH pop=polyData.PolyPoint[0];								  
					pop.vx -= viewVectorAlpha.vx;
					pop.vy -= viewVectorAlpha.vy;
					pop.vz -= viewVectorAlpha.vz;

				  	d = DotProduct(&(polyNormal),&pop);
				}

				if (d>0) continue;

			  	lambda = DIV_FIXED(d,normDotBeta);
				
				if (lambda>=LOS_Lambda) 
				{
					continue;
				}															                  
		   		pointOnPlane.vx	= viewVectorAlpha.vx + MUL_FIXED(lambda,viewVectorBeta.vx);
 		   		pointOnPlane.vy	= viewVectorAlpha.vy + MUL_FIXED(lambda,viewVectorBeta.vy);
		   		pointOnPlane.vz	= viewVectorAlpha.vz + MUL_FIXED(lambda,viewVectorBeta.vz);

	  		}

			/* decide which 2d plane to project onto */

			{
				VECTORCH absNormal = (polyNormal);
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
		 		VECTORCH *vertexPtr = &polyData.PolyPoint[0];
		 		int *projectedVertexPtr= &projectedPolyVertex[0];
				int noOfVertices = polyData.NumberOfVertices;

				do
				{
		 			*projectedVertexPtr++ = *((int*)vertexPtr + axis1);
		 			*projectedVertexPtr++ = *((int*)vertexPtr + axis2);

	 	 		   	vertexPtr++;
		 			noOfVertices--;
		 		}
                while(noOfVertices);

		 	}


			if (PointInPolygon(&projectedPointOnPlane[0],&projectedPolyVertex[0],polyData.NumberOfVertices,2))
			{
				/* rotate vector back into World Space if it's not a module */
				LOS_ObjectNormal = polyNormal;
				if (needToRotate)
				{
					MATRIXCH matrix = dPtr->ObMat;
					RotateVector(&pointOnPlane,&matrix);
				
					RotateVector(&LOS_ObjectNormal,&matrix);
				}

				/* and translate origin */
				pointOnPlane.vx += position.vx;
				pointOnPlane.vy += position.vy;
				pointOnPlane.vz += position.vz;

				LOS_Point=pointOnPlane;
				LOS_ObjectHitPtr = dPtr;
				LOS_Lambda=lambda;
				LOS_HModel_Section = 0;
			}
		}
	}
    return;
}

/* KJL 15:35:58 14/05/98 - IsObjectVisibleFromThisPoint

	Returns a non-zero value if an object can be seen from a given point in world space
	(viewpointPositionPtr), with a given field of view (fieldOfView) about a given
	direction (viewpointDirectionPtr).
*/
int IsObjectVisibleFromThisPoint(DISPLAYBLOCK *dispPtr, VECTORCH *viewpointDirectionPtr, VECTORCH *viewpointPositionPtr, int fieldOfView)
{

	return 0;
}







