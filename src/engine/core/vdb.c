
#include "3dc.h"
#include "inline.h"



/*

 externs for commonly used global variables and arrays

*/

extern int ScanDrawMode;


/*

 General System Globals

*/

SCENE Global_Scene/* = 0*/;




/*

 Screen Descriptor Block

*/

SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;


/*

 View Descriptor Blocks

*/

	int NumFreeVDBs;
	VIEWDESCRIPTORBLOCK *FreeVDBList[maxvdbs];
	static VIEWDESCRIPTORBLOCK **FreeVDBListPtr = &FreeVDBList[maxvdbs-1];

	int NumActiveVDBs;
	VIEWDESCRIPTORBLOCK *ActiveVDBList[maxvdbs];
	static VIEWDESCRIPTORBLOCK **ActiveVDBListPtr = &ActiveVDBList[0];

	static VIEWDESCRIPTORBLOCK FreeVDBData[maxvdbs];


/* Clip Plane Block */

	static CLIPPLANEPOINTS ClipPlanePoints;



extern int GlobalAmbience;

/*

 Support Functions

*/


/*

 Calculate View Volume Planes from the Clip Boundaries for a VDB

 Before doing this, check that the requested boundaries are within those of
 the physical sceen. If any boundary transgresses, truncate it and set the
 appropriate flag bit.

*/

void VDBClipPlanes(VIEWDESCRIPTORBLOCK *vdb)

{


/* Check Clip Boundaries against the Physical Screen */


	/* Check Left Boundary */

	if(vdb->VDB_ClipLeft < ScreenDescriptorBlock.SDB_ClipLeft) {

		vdb->VDB_ClipLeft = ScreenDescriptorBlock.SDB_ClipLeft;
		vdb->VDB_Flags |= ViewDB_Flag_LTrunc;

	}

	/* Check Right Boundary */

	if(vdb->VDB_ClipRight > ScreenDescriptorBlock.SDB_ClipRight) {

		vdb->VDB_ClipRight = ScreenDescriptorBlock.SDB_ClipRight;
		vdb->VDB_Flags |= ViewDB_Flag_RTrunc;

	}

	/* Check Up boundary */

	if(vdb->VDB_ClipUp < ScreenDescriptorBlock.SDB_ClipUp) {

		vdb->VDB_ClipUp = ScreenDescriptorBlock.SDB_ClipUp;
		vdb->VDB_Flags |= ViewDB_Flag_UTrunc;

	}

	/* Check Down boundary */

	if(vdb->VDB_ClipDown > ScreenDescriptorBlock.SDB_ClipDown) {

		vdb->VDB_ClipDown = ScreenDescriptorBlock.SDB_ClipDown;
		vdb->VDB_Flags |= ViewDB_Flag_DTrunc;

	}


/* Calculate Width and Height */

	/* textprint("current wh = %d, %d\n", vdb->VDB_Width, vdb->VDB_Height); */

	/* Width */

	vdb->VDB_Width = vdb->VDB_ClipRight - vdb->VDB_ClipLeft;

	/* Height */

	vdb->VDB_Height = vdb->VDB_ClipDown - vdb->VDB_ClipUp;

	#if 0
	textprint("new wh = %d, %d\n", vdb->VDB_Width, vdb->VDB_Height);
	WaitForReturn();
	#endif


/* Set up the Clip Planes */


	/* Clip Left */

	ClipPlanePoints.cpp1.vx = vdb->VDB_ClipLeft;
	ClipPlanePoints.cpp1.vy = vdb->VDB_ClipUp;
	ClipPlanePoints.cpp1.vz = NearZ;

	ClipPlanePoints.cpp2.vx = vdb->VDB_ClipLeft;
	ClipPlanePoints.cpp2.vy = (vdb->VDB_ClipUp + vdb->VDB_ClipDown) / 2;
	ClipPlanePoints.cpp2.vz = FarZ;

	ClipPlanePoints.cpp3.vx = vdb->VDB_ClipLeft;
	ClipPlanePoints.cpp3.vy = vdb->VDB_ClipDown;
	ClipPlanePoints.cpp3.vz = NearZ;

	MakeClipPlane(vdb, &vdb->VDB_ClipLeftPlane, &ClipPlanePoints);


	/* Clip Right */

	ClipPlanePoints.cpp1.vx = vdb->VDB_ClipRight;
	ClipPlanePoints.cpp1.vy = vdb->VDB_ClipUp;
	ClipPlanePoints.cpp1.vz = NearZ;

	ClipPlanePoints.cpp2.vx = vdb->VDB_ClipRight;
	ClipPlanePoints.cpp2.vy = vdb->VDB_ClipDown;
	ClipPlanePoints.cpp2.vz = NearZ;

	ClipPlanePoints.cpp3.vx = vdb->VDB_ClipRight;
	ClipPlanePoints.cpp3.vy = (vdb->VDB_ClipUp + vdb->VDB_ClipDown) / 2;
	ClipPlanePoints.cpp3.vz = FarZ;

	MakeClipPlane(vdb, &vdb->VDB_ClipRightPlane, &ClipPlanePoints);


	/* Clip Up */

	ClipPlanePoints.cpp1.vx = vdb->VDB_ClipLeft;
	ClipPlanePoints.cpp1.vy = vdb->VDB_ClipUp;
	ClipPlanePoints.cpp1.vz = NearZ;

	ClipPlanePoints.cpp2.vx = vdb->VDB_ClipRight;
	ClipPlanePoints.cpp2.vy = vdb->VDB_ClipUp;
	ClipPlanePoints.cpp2.vz = NearZ;

	ClipPlanePoints.cpp3.vx = (vdb->VDB_ClipLeft + vdb->VDB_ClipRight) / 2;
	ClipPlanePoints.cpp3.vy = vdb->VDB_ClipUp;
	ClipPlanePoints.cpp3.vz = FarZ;

	MakeClipPlane(vdb, &vdb->VDB_ClipUpPlane, &ClipPlanePoints);


	/* Clip Down */

	ClipPlanePoints.cpp1.vx = vdb->VDB_ClipLeft;
	ClipPlanePoints.cpp1.vy = vdb->VDB_ClipDown;
	ClipPlanePoints.cpp1.vz = NearZ;

	ClipPlanePoints.cpp2.vx = (vdb->VDB_ClipLeft + vdb->VDB_ClipRight) / 2;
	ClipPlanePoints.cpp2.vy = vdb->VDB_ClipDown;
	ClipPlanePoints.cpp2.vz = FarZ;

	ClipPlanePoints.cpp3.vx = vdb->VDB_ClipRight;
	ClipPlanePoints.cpp3.vy = vdb->VDB_ClipDown;
	ClipPlanePoints.cpp3.vz = NearZ;

	MakeClipPlane(vdb, &vdb->VDB_ClipDownPlane, &ClipPlanePoints);


/* Clip Z */

	vdb->VDB_ClipZPlane.CPB_Normal.vx = 0;
	vdb->VDB_ClipZPlane.CPB_Normal.vy = 0;
	vdb->VDB_ClipZPlane.CPB_Normal.vz = -ONE_FIXED;

	vdb->VDB_ClipZPlane.CPB_POP.vx = 0;
	vdb->VDB_ClipZPlane.CPB_POP.vy = 0;
	vdb->VDB_ClipZPlane.CPB_POP.vz = vdb->VDB_ClipZ;

}


/*

 Make a Clip Plane.

 Reverse Project the three Clip Points (overwrite their struct).

 Use the first Clip Point as the POP, writing this to the CPB.

 Pass the three Clip Points to MakeNormal() specifying the CPB Normal as
 the destination.

 NOTES

 The vdb is passed for the reverse projection.

*/

void MakeClipPlane(

VIEWDESCRIPTORBLOCK *vdb,
CLIPPLANEBLOCK *cpb,
CLIPPLANEPOINTS *cpp)

{

	int x, y;

/* Reverse Project the Clip Points */


/* cpp1 */

/* x */

	x=cpp->cpp1.vx;
	x-=vdb->VDB_CentreX;
	x*=cpp->cpp1.vz;
	x/=vdb->VDB_ProjX;
	cpp->cpp1.vx=x;

/* y */

	y=cpp->cpp1.vy;
	y-=vdb->VDB_CentreY;
	y*=cpp->cpp1.vz;
	y/=vdb->VDB_ProjY;
	cpp->cpp1.vy=y;


/* cpp2 */

/* x */

	x=cpp->cpp2.vx;
	x-=vdb->VDB_CentreX;
	x*=cpp->cpp2.vz;
	x/=vdb->VDB_ProjX;
	cpp->cpp2.vx=x;

/* y */

	y=cpp->cpp2.vy;
	y-=vdb->VDB_CentreY;
	y*=cpp->cpp2.vz;
	y/=vdb->VDB_ProjY;
	cpp->cpp2.vy=y;


/* cpp3 */

/* x */

	x=cpp->cpp3.vx;
	x-=vdb->VDB_CentreX;
	x*=cpp->cpp3.vz;
	x/=vdb->VDB_ProjX;
	cpp->cpp3.vx=x;

/* y */

	y=cpp->cpp3.vy;
	y-=vdb->VDB_CentreY;
	y*=cpp->cpp3.vz;
	y/=vdb->VDB_ProjY;
	cpp->cpp3.vy=y;

/* The 1st Clip Point can be the POP */

	cpb->CPB_POP.vx=cpp->cpp1.vx;
	cpb->CPB_POP.vy=cpp->cpp1.vy;
	cpb->CPB_POP.vz=cpp->cpp1.vz;

/* Make CPB_Normal */

	MakeNormal(&cpp->cpp1, &cpp->cpp2, &cpp->cpp3, &cpb->CPB_Normal);

}


#if pc_backdrops

/*

 Create the projector array used for cylindrically projected backdrops

 The array looks like this

	unsigned short VDB_ProjectorXOffsets[MaxScreenWidth];

 For each centre relative x there is a projector. Using just the x and z
 components of the projector an angular offset from the centre (0) can be
 calculated using ArcTan().

 To calculate a projector one must reverse project each x value at a given
 z value. Rather than go on to create the normalised projector vector we can
 go straight to the ArcTan() to find the angular offset.

*/

static void CreateProjectorArray(VIEWDESCRIPTORBLOCK *vdb)

{

	int sx, x, vx, vz, i, ao;


	vz = ONE_FIXED;

	sx = vdb->VDB_ClipLeft;

	i = 0;

	while(sx < vdb->VDB_ClipRight) {

		x = sx - vdb->VDB_CentreX;

		vx = (x * vz) / vdb->VDB_ProjX;

		ao = ArcTan(vx, vz);

		vdb->VDB_ProjectorXOffsets[i] = ao;

		#if 0
		textprint("Pr. Offset %d = %u\n", i, vdb->VDB_ProjectorXOffsets[i]);
		WaitForReturn();
		#endif

		sx++; i++;

	}

}

#endif






/*

 SetVDB requires a VIEWDESCRIPTORBLOCK

*/

void SetVDB(VIEWDESCRIPTORBLOCK *vdb, int fl, int ty, int d, int cx, int cy, 
    int prx, int pry, int mxp, int cl, int cr, int cu, int cd, 
    int h1, int h2, int hc, int amb)
{



	/* Initial setup */

	vdb->VDB_Flags			= fl;
	vdb->VDB_ViewType		= ty;


	/* Ambience */

	vdb->VDB_Ambience = amb;
	/* KJL 14:30:57 05/14/97 - set globalAmbience here as well */
	GlobalAmbience = amb;

	/* Width and Height are set by the Clip Boundaries */

	if(vdb->VDB_Flags & ViewDB_Flag_FullSize) {

		vdb->VDB_Depth			= ScreenDescriptorBlock.SDB_Depth;

		vdb->VDB_ScreenDepth    = ScreenDescriptorBlock.SDB_ScreenDepth;

		vdb->VDB_CentreX		= ScreenDescriptorBlock.SDB_CentreX;
		vdb->VDB_CentreY		= ScreenDescriptorBlock.SDB_CentreY;

		vdb->VDB_ProjX			= ScreenDescriptorBlock.SDB_ProjX;
		vdb->VDB_ProjY			= ScreenDescriptorBlock.SDB_ProjY;
		vdb->VDB_MaxProj		= ScreenDescriptorBlock.SDB_MaxProj;

		vdb->VDB_ClipLeft		= ScreenDescriptorBlock.SDB_ClipLeft;
		vdb->VDB_ClipRight	= ScreenDescriptorBlock.SDB_ClipRight;
		vdb->VDB_ClipUp		= ScreenDescriptorBlock.SDB_ClipUp;
		vdb->VDB_ClipDown		= ScreenDescriptorBlock.SDB_ClipDown;

	}

	else {

		 if (ScanDrawMode == ScanDrawDirectDraw)
		   vdb->VDB_Depth			= d;
		 else
		   vdb->VDB_Depth = VideoModeType_24;

		vdb->VDB_ScreenDepth    = ScreenDescriptorBlock.SDB_ScreenDepth;

		vdb->VDB_CentreX		= cx;
		vdb->VDB_CentreY		= cy;

		vdb->VDB_ProjX			= prx;
		vdb->VDB_ProjY			= pry;
		vdb->VDB_MaxProj		= mxp;

		vdb->VDB_ClipLeft		= cl;
		vdb->VDB_ClipRight	= cr;
		vdb->VDB_ClipUp		= cu;
		vdb->VDB_ClipDown		= cd;

		if(vdb->VDB_Flags & ViewDB_Flag_AdjustScale) {

			vdb->VDB_CentreX =
				WideMulNarrowDiv(
					vdb->VDB_CentreX,
					ScreenDescriptorBlock.SDB_Width,
					320);

			vdb->VDB_CentreY =
				WideMulNarrowDiv(
					vdb->VDB_CentreY,
					ScreenDescriptorBlock.SDB_Height,
					200);


			vdb->VDB_ProjX =
				WideMulNarrowDiv(
					vdb->VDB_ProjX,
					ScreenDescriptorBlock.SDB_Width,
					320);

			vdb->VDB_ProjY =
				WideMulNarrowDiv(
					vdb->VDB_ProjY,
					ScreenDescriptorBlock.SDB_Height,
					200);

			vdb->VDB_MaxProj =
				WideMulNarrowDiv(
					vdb->VDB_MaxProj,
					ScreenDescriptorBlock.SDB_Width,
					320);


			vdb->VDB_ClipLeft =
				WideMulNarrowDiv(
					vdb->VDB_ClipLeft,
					ScreenDescriptorBlock.SDB_Width,
					320);

			vdb->VDB_ClipRight =
				WideMulNarrowDiv(
					vdb->VDB_ClipRight,
					ScreenDescriptorBlock.SDB_Width,
					320);

			vdb->VDB_ClipUp =
				WideMulNarrowDiv(
					vdb->VDB_ClipUp,
					ScreenDescriptorBlock.SDB_Height,
					200);

			vdb->VDB_ClipDown =
				WideMulNarrowDiv(
					vdb->VDB_ClipDown,
					ScreenDescriptorBlock.SDB_Height,
					200);

		}

	}


	/* Create the clip planes */

	/* KJL 10:41:13 04/09/97 - set to constant so the same for all screen sizes! */
	vdb->VDB_ClipZ = 64;//vdb->VDB_MaxProj;	/* Safe */

	VDBClipPlanes(vdb);

	#if 0
	/* View Angle */

	if(vdb->VDB_CentreX > vdb->VDB_CentreY) s = vdb->VDB_CentreX;
	else s = vdb->VDB_CentreY;

	z = vdb->VDB_MaxProj * 100;

	x = (s * z) / vdb->VDB_MaxProj;

	vdb->VDB_ViewAngle = ArcTan(x, z);

	vdb->VDB_ViewAngleCos = GetCos(vdb->VDB_ViewAngle);


	#if 0
	textprint("View Angle = %d\n", vdb->VDB_ViewAngle);
	WaitForReturn();
	#endif


	#if pc_backdrops

	/* Projector Array */

	CreateProjectorArray(vdb);

	#endif


	/* Hazing & Background Colour */

	vdb->VDB_H1				= h1;
	vdb->VDB_H2				= h2;
	vdb->VDB_HInterval	= h2 - h1;
	vdb->VDB_HColour		= hc;
	#endif
}




/*

 Initialise the Free VDB List

*/

void InitialiseVDBs(void)

{

	VIEWDESCRIPTORBLOCK *FreeVDBPtr = &FreeVDBData[0];

	NumActiveVDBs = 0;

	for(NumFreeVDBs = 0; NumFreeVDBs < maxvdbs; NumFreeVDBs++) {

		FreeVDBList[NumFreeVDBs] = FreeVDBPtr;

		FreeVDBPtr++;

	}

	FreeVDBListPtr   = &FreeVDBList[maxvdbs-1];
	ActiveVDBListPtr = &ActiveVDBList[0];

}


/*

 Get a VDB from the Free VDB list

*/

VIEWDESCRIPTORBLOCK* AllocateVDB(void)

{

	VIEWDESCRIPTORBLOCK *FreeVDBPtr = 0;	/* Default to null ptr */
	int *i_src;
	int i;


	if(NumFreeVDBs) {

		FreeVDBPtr = *FreeVDBListPtr--;

		NumFreeVDBs -= 1;							/* One less free block */

		/* Clear the block */

		i_src = (int *) FreeVDBPtr;
		for(i = sizeof(VIEWDESCRIPTORBLOCK)/4; i!=0; i--)
			*i_src++ = 0;

	}

	return(FreeVDBPtr);

}


/*

 Return a VDB to the Free VDB list

*/

void DeallocateVDB(VIEWDESCRIPTORBLOCK *vdb)

{

	FreeVDBListPtr++;
	*FreeVDBListPtr = vdb;

	NumFreeVDBs++;							/* One more free block */

}


/*

 Allocate a VDB and make it active

*/

VIEWDESCRIPTORBLOCK* CreateActiveVDB(void)

{

	VIEWDESCRIPTORBLOCK *vdb;
	VIEWDESCRIPTORBLOCK *vdb_tmp;
	VIEWDESCRIPTORBLOCK **v_src;

	int v;
	int p = -1;


	vdb = AllocateVDB();

	if(vdb) {

		/* Find the next "VDB_Priority" */

		if(NumActiveVDBs) {

			v_src = &ActiveVDBList[0];

			for(v = NumActiveVDBs; v!=0; v--) {

				vdb_tmp = *v_src++;

				if(vdb_tmp->VDB_Priority > p) p = vdb_tmp->VDB_Priority;

			}

		}

		/* Set the VDB priority */

		vdb->VDB_Priority = (p + 1);

		/* Update the active VDB list */

		*ActiveVDBListPtr++ = vdb;

		NumActiveVDBs++;

	}

	return vdb;

}


/*

 Deallocate an active VDB

*/

int DestroyActiveVDB(VIEWDESCRIPTORBLOCK *dblockptr)
{

	int j = -1;
	int i;


	/* If the VDB ptr is OK, search the Active VDB List */

	if(dblockptr) {

		for(i = 0; i < NumActiveVDBs && j!=0; i++) {

			if(ActiveVDBList[i] == dblockptr) {

				#if ProjectSpecificVDBs
				ProjectSpecificVDBDestroy(dblockptr);
				#endif

				ActiveVDBList[i] = ActiveVDBList[NumActiveVDBs - 1];
				NumActiveVDBs--;
				ActiveVDBListPtr--;
				DeallocateVDB(dblockptr);		/* Return VDB to Free List */
				j = 0;								/* Flag OK */

			}
		}
	}

	return(j);

}
