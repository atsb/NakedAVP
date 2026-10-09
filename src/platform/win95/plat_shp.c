/*RWH moved to a seperate file*/


#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "gameplat.h"
#include "gamedef.h"


#include "dynblock.h"
#include "dynamics.h"
#define UseLocalAssert No
#include "ourasert.h"


/*						   *
						   *
						   *
						*  *  *
						 * * *
						  ***
                           *
*/
/*KJL***********************************************
* Polygon Access Functions V1.0, 18:12:27 11/07/96 *
***********************************************KJL*/

int SetupPolygonAccess(DISPLAYBLOCK *objectPtr);
void AccessNextPolygon(void);
void GetPolygonVertices(struct ColPolyTag *polyPtr);
void GetPolygonNormal(struct ColPolyTag *polyPtr);
int SetupPolygonAccessFromShapeIndex(int shapeIndex);


/* the following are needed for morphing support */
#if SupportMorphing
extern MORPHDISPLAY MorphDisplay;
extern VECTORCH MorphedPts[];
#endif

VECTORCH *ShapePointsPtr;
int *ShapeNormalsPtr;
int *Shape2NormalsPtr;
char ShapeIsMorphed;
int **ItemArrayPtr;
POLYHEADER *PolyheaderPtr;

int SetupPolygonAccess(DISPLAYBLOCK *objectPtr)
{
	SHAPEHEADER *shape1Ptr;

	#if SupportMorphing
  	if (objectPtr->ObMorphCtrl) /* morphable object? */
	{
		VECTORCH *shape1PointsPtr;
		VECTORCH *shape2PointsPtr;
						
		/* Set up the morph data */
		GetMorphDisplay(&MorphDisplay, objectPtr);

		shape1Ptr = MorphDisplay.md_sptr1;

		if(MorphDisplay.md_lerp == 0x0000)
		{
			
			ShapePointsPtr = (VECTORCH *)*shape1Ptr->points;
			ShapeNormalsPtr = (int *) *(shape1Ptr->sh_normals);
			ShapeIsMorphed=0;
				
		}
		else if(MorphDisplay.md_lerp == 0xffff)
		{
			SHAPEHEADER *shape2Ptr;
			shape2Ptr = MorphDisplay.md_sptr2;
			
			ShapePointsPtr = (VECTORCH *)*shape2Ptr->points;	
			ShapeNormalsPtr = (int *) *(shape2Ptr->sh_normals);
			ShapeIsMorphed=0;
		}
		else
		{
			SHAPEHEADER *shape2Ptr;
			shape2Ptr = MorphDisplay.md_sptr2;
		    
			shape1PointsPtr = (VECTORCH *)(*shape1Ptr->points);
			shape2PointsPtr = (VECTORCH *)(*shape2Ptr->points);

			/* you're going to need all the points so you might as well morph them all at once now */
			{
		    	int numberOfPoints = shape1Ptr->numpoints;
				VECTORCH *morphedPointsPtr = (VECTORCH *) MorphedPts;
		    
				while(numberOfPoints--)
				{
				   	VECTORCH vertex1 = *shape1PointsPtr;
				   	VECTORCH vertex2 = *shape2PointsPtr;
				
					if( (vertex1.vx == vertex2.vx && vertex1.vy == vertex2.vy && vertex1.vz == vertex2.vz) )
					{
						*morphedPointsPtr = vertex1;
					}
					else
					{
						/* KJL 15:27:20 05/22/97 - I've changed this to speed things up, If a vertex
						component has a magnitude greater than 32768 things will go wrong. */
						morphedPointsPtr->vx = vertex1.vx + (((vertex2.vx-vertex1.vx)*MorphDisplay.md_lerp)>>16);
						morphedPointsPtr->vy = vertex1.vy + (((vertex2.vy-vertex1.vy)*MorphDisplay.md_lerp)>>16);
						morphedPointsPtr->vz = vertex1.vz + (((vertex2.vz-vertex1.vz)*MorphDisplay.md_lerp)>>16);
					}

		            shape1PointsPtr++;
		            shape2PointsPtr++;
					morphedPointsPtr++;
				}
			}

		    ShapePointsPtr = (VECTORCH *)MorphedPts;
			ShapeNormalsPtr = (int *) *(shape1Ptr->sh_normals);
			Shape2NormalsPtr = (int *) *(shape2Ptr->sh_normals);
			ShapeIsMorphed=1;
		}
		ItemArrayPtr = (int **)shape1Ptr->items;
	}
  	else /* not a morphing object */
  	#endif
  	{
		shape1Ptr = GetShapeData(objectPtr->ObShape);

		ShapePointsPtr  = (VECTORCH *)(*shape1Ptr->points);
		ShapeNormalsPtr = (int *)(*shape1Ptr->sh_normals);
		ItemArrayPtr = (int **)shape1Ptr->items;
        ShapeIsMorphed=0;
	}	
	
	{
		int *itemPtr = *ItemArrayPtr;
		PolyheaderPtr = (POLYHEADER *) itemPtr;
	}
    
    return shape1Ptr->numitems;
}
void AccessNextPolygon(void)
{
	int *itemPtr = *(ItemArrayPtr++);
	PolyheaderPtr = (POLYHEADER *) itemPtr;
    return;
}											 

void GetPolygonVertices(struct ColPolyTag *polyPtr)
{
	int *vertexNumberPtr = &PolyheaderPtr->Poly1stPt;

  	polyPtr->PolyPoint[0] = *(ShapePointsPtr + *vertexNumberPtr++);
    polyPtr->PolyPoint[1] = *(ShapePointsPtr + *vertexNumberPtr++);
    polyPtr->PolyPoint[2] = *(ShapePointsPtr + *vertexNumberPtr++);
    
	if (*vertexNumberPtr != Term)
	{
	    polyPtr->PolyPoint[3] = *(ShapePointsPtr + *vertexNumberPtr);
	   	polyPtr->NumberOfVertices=4; 
	}
	else
	{
	   	polyPtr->NumberOfVertices=3; 
	}

    return;
}
void GetPolygonNormal(struct ColPolyTag *polyPtr)
{	  
	if (ShapeIsMorphed)
	{
		VECTORCH n1Ptr = *(VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex);
		VECTORCH n2Ptr = *(VECTORCH*)(Shape2NormalsPtr + PolyheaderPtr->PolyNormalIndex);
        
		if( ((n1Ptr.vx == n2Ptr.vx)
		  && (n1Ptr.vy == n2Ptr.vy)
		  && (n1Ptr.vz == n2Ptr.vz))
		  || (MorphDisplay.md_lerp == 0) )
		{
			polyPtr->PolyNormal = n1Ptr;
		}
		else if(MorphDisplay.md_lerp == 0xffff)
		{
			polyPtr->PolyNormal = n2Ptr;
		}
		else
		{
			VECTORCH *pointPtr[3];
 			int *vertexNumPtr = &PolyheaderPtr->Poly1stPt;

			pointPtr[0] = (ShapePointsPtr + *vertexNumPtr++);
			pointPtr[1] = (ShapePointsPtr + *vertexNumPtr++);
			pointPtr[2] = (ShapePointsPtr + *vertexNumPtr);

			MakeNormal
			(
				pointPtr[0],
				pointPtr[1],
				pointPtr[2],
				&polyPtr->PolyNormal
			);
		}
	}
    else /* not morphed */
    {
     	polyPtr->PolyNormal = *(VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex);
    	
    	/* KJL 20:55:36 05/14/97 - turned off for alpha */
    	#if 0
    	if(	(polyPtr->PolyNormal.vx==0)
		  &&(polyPtr->PolyNormal.vy==0)
		  &&(polyPtr->PolyNormal.vz==0) )
		{
			textprint("shape data has zero normal\n");
		}
		#endif

    }
    return;
}	

/*-----------------------Patrick 1/12/96--------------------------
  I have added this function to initialise polygon access for a
  module based on the shape index specified in its mapblock....
  Specifically, this is for setting up location data for modules
  during game initialisation, and so doesn't support morphing.
  ----------------------------------------------------------------*/
int SetupPolygonAccessFromShapeIndex(int shapeIndex)
{
	SHAPEHEADER *shape1Ptr;
	
	shape1Ptr = GetShapeData(shapeIndex);
	ShapePointsPtr  = (VECTORCH *)(*shape1Ptr->points);
	ShapeNormalsPtr = (int *)(*shape1Ptr->sh_normals);
	ItemArrayPtr = (int **)shape1Ptr->items;
    ShapeIsMorphed=0;
	
	{
		int *itemPtr = *ItemArrayPtr;
		PolyheaderPtr = (POLYHEADER *) itemPtr;
	}
    
    return shape1Ptr->numitems;
}

/*--------------------Patrick 17/12/96----------------------------
  I have added some more shape data access functions......
  ----------------------------------------------------------------*/

static VECTORCH patPointData;
static int patPolyVertexIndices[4];
static VECTORCH *patShapePointsPtr;

int SetupPointAccessFromShapeIndex(int shapeIndex)
{
	SHAPEHEADER *shapePtr;

	shapePtr = GetShapeData(shapeIndex);
	patShapePointsPtr  = (VECTORCH *)(*shapePtr->points);
	    
    return shapePtr->numpoints;
}


VECTORCH* AccessNextPoint(void)
{
	patPointData = *patShapePointsPtr++;
	return &patPointData;
}

VECTORCH* AccessPointFromIndex(int index)
{
	patPointData = patShapePointsPtr[index];
	return &patPointData;
}		

/* KJL 18:51:08 21/11/98 - similiar function for polys */									 
POLYHEADER *AccessPolyFromIndex(int index)
{
	int *itemPtr = *(ItemArrayPtr+index);
	PolyheaderPtr = (POLYHEADER *) itemPtr;
	return PolyheaderPtr;
}

void DestroyPolygon(int shapeIndex,int polyIndex)
{
	SHAPEHEADER *shapePtr = GetShapeData(shapeIndex);
	shapePtr->numitems--;
	*(ItemArrayPtr+polyIndex) = *(ItemArrayPtr+shapePtr->numitems);
}

void ReplaceVertexInPolygon(int polyIndex, int oldVertex, int newVertex)
{
	int *vertexNumberPtr;
	int *itemPtr = *(ItemArrayPtr+polyIndex);
	PolyheaderPtr = (POLYHEADER *) itemPtr;
	
	vertexNumberPtr = &PolyheaderPtr->Poly1stPt;

    while(*vertexNumberPtr != Term)
	{
    	if (*vertexNumberPtr == oldVertex)
		{
			*vertexNumberPtr = newVertex;
		}
		vertexNumberPtr++; 
	}

	{
		VECTORCH newNormal;
		VECTORCH *pointPtr[3];
		int *vertexNumPtr = &PolyheaderPtr->Poly1stPt;
		pointPtr[0] = (ShapePointsPtr + *vertexNumPtr++);
		pointPtr[1] = (ShapePointsPtr + *vertexNumPtr++);
		pointPtr[2] = (ShapePointsPtr + *vertexNumPtr);
		MakeNormal
		(
			pointPtr[0],
			pointPtr[1],
			pointPtr[2],
			&newNormal
		);
	   	*(VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex)=newNormal;
	}
    	

}
VECTORCH *GetPolygonNormalFromIndex(void)
{
	return (VECTORCH*)(ShapeNormalsPtr + PolyheaderPtr->PolyNormalIndex);
}

int *GetPolygonVertexIndices(void)
{
	int *vertexNumberPtr = &PolyheaderPtr->Poly1stPt;
    int numberOfVertices=0;

    patPolyVertexIndices[3]	= -1;

    while(*vertexNumberPtr != Term)
	{
    	patPolyVertexIndices[numberOfVertices++] = (*vertexNumberPtr);
		vertexNumberPtr++; 
	}

    return &patPolyVertexIndices[0];
}


/*--------------------Roxby 3/7/97----------------------------
  I have added some more shape data access functions......
  ----------------------------------------------------------------*/


void SetupPolygonFlagAccessForShape(SHAPEHEADER *shape) 
{
}
		
		
int Request_PolyFlags(void *polygon) 
{
	POLYHEADER *poly = (POLYHEADER*)polygon;
	return poly->PolyFlags;
}
