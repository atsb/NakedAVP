#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"

#define UseLocalAssert Yes
#include "ourasert.h"


/*
 externs for commonly used global variables and arrays
*/


extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;

/*
 Global Variables
*/

DISPLAYBLOCK *dptr_last;
DISPLAYBLOCK *Player;


/*
		Map Reading Functions
		Read the map data passed and create the objects on the map
		The map header contains an array of pointers to the different map data
		structures
*/


DISPLAYBLOCK* ReadMap(MAPHEADER *mapheader)
{
	MAPBLOCK8 *mapblock8ptr;
	DISPLAYBLOCK *dblockptr = 0;


	/* Set up pointers to the map arrays */
	mapblock8ptr = mapheader->MapType8Objects;


 	/* Map Type #8 Structure */
	if(mapblock8ptr) {

		while(mapblock8ptr->MapType != MapType_Term) {

			dblockptr = CreateActiveObject();

			if(dblockptr)
			{
				dblockptr->ObShape = mapblock8ptr->MapShape;

				CopyLocation(&mapblock8ptr->MapWorld, &dblockptr->ObWorld);
				CopyEuler(&mapblock8ptr->MapEuler, &dblockptr->ObEuler);

				dblockptr->ObFlags  = mapblock8ptr->MapFlags;
				dblockptr->ObFlags2 = mapblock8ptr->MapFlags2;
				dblockptr->ObFlags3 = mapblock8ptr->MapFlags3;

				if (mapblock8ptr->MapType == MapType_Player)
				{
					Player = dblockptr;
				}
				else
				{
					dblockptr->ObLightType = LightType_PerVertex;
					dblockptr->ObFlags |= ObFlag_MultLSrc;
				}

/* KJL 16:55:57 06/05/97 - removing camera stuff */
				if(mapblock8ptr->MapVDBData)
					MapSetVDB(dblockptr, mapblock8ptr->MapVDBData);

				dblockptr->ObLightType = mapblock8ptr->MapLightType;

/* KJL 15:23:52 06/07/97 - removed */
//				CopyVector(&mapblock8ptr->MapOrigin, &dblockptr->ObOrigin);
//				dblockptr->ObSimShapes = mapblock8ptr->MapSimShapes;
//				dblockptr->ObViewType = mapblock8ptr->MapViewType;

				MapBlockInit(dblockptr);

				CreateEulerMatrix(&dblockptr->ObEuler,&dblockptr->ObMat);
				TransposeMatrixCH(&dblockptr->ObMat);

				MapPostProcessing(dblockptr);
			}

			dptr_last = dblockptr;

			mapblock8ptr++;

		}

	}
 	return dblockptr;
}



/*

 Some objects might require a certain amount of general processing after
 all the other map functions have been called

*/

void MapPostProcessing(DISPLAYBLOCK *dptr)
{
	if(dptr)
	{
		/*

		Make sure that objects requesting multiple light sources are at
		least set to "LightType_PerObject"

		*/
		if(dptr->ObFlags & ObFlag_MultLSrc)
		{
			if(dptr->ObLightType == LightType_Infinite)
				dptr->ObLightType = LightType_PerObject;
		}
	}
}


void MapSetVDB(DISPLAYBLOCK *dptr, MAPSETVDB *mapvdbdata)
{

	VIEWDESCRIPTORBLOCK *vdb;

	/* TEST */
	/*LIGHTBLOCK *lptr;*/



	/* Allocate a VDB */

	vdb = CreateActiveVDB();

	if(vdb) {

		dptr->ObVDBPtr = vdb;			/* Object Block ptr to VDB */

		vdb->VDB_ViewObject = dptr;	/* VDB ptr to Object Block */


		/* VDB Setup */

		SetVDB(

			vdb,

			mapvdbdata->SVDB_Flags,
			mapvdbdata->SVDB_ViewType,

			mapvdbdata->SVDB_Depth,

			mapvdbdata->SVDB_CentreX,
			mapvdbdata->SVDB_CentreY,

			mapvdbdata->SVDB_ProjX,
			mapvdbdata->SVDB_ProjY,
			mapvdbdata->SVDB_MaxProj,

			mapvdbdata->SVDB_ClipLeft,
			mapvdbdata->SVDB_ClipRight,
			mapvdbdata->SVDB_ClipUp,
			mapvdbdata->SVDB_ClipDown,

			mapvdbdata->SVDB_H1,
			mapvdbdata->SVDB_H2,
			mapvdbdata->SVDB_HColour,

			mapvdbdata->SVDB_Ambience

		);

		PlatformSpecificVDBInit(vdb);

		#if ProjectSpecificVDBs
		ProjectSpecificVDBInit(vdb);
		#endif


	}

}







/*

 Standard Initialisation for Map Objects

*/

void MapBlockInit(DISPLAYBLOCK *dptr)
{
	SHAPEHEADER *sptr;

	/* Get the shape header ptr */

	sptr = GetShapeData(dptr->ObShape);


	/* Augmented Z */

	if(sptr->shapeflags & ShapeFlag_AugZ) dptr->ObFlags2 |= ObFlag2_AugZ;


	/* Pass address of the shape data header back to the block for others */

	dptr->ObShapeData = sptr;


	/* Does this shape use a BSP tree or a Z Sort ? */

	dptr->ObFlags |= ObFlag_TypeZ;


	/* Copy shape radius to ODB */

	dptr->ObRadius = sptr->shaperadius;

	/* Copy shape xyz extents to ODB */

	dptr->ObMaxX = sptr->shapemaxx;
	dptr->ObMinX = sptr->shapeminx;

	dptr->ObMaxY = sptr->shapemaxy;
	dptr->ObMinY = sptr->shapeminy;

	dptr->ObMaxZ = sptr->shapemaxz;
	dptr->ObMinZ = sptr->shapeminz;


   
}
