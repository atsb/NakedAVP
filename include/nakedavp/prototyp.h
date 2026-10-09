#ifndef PROTOTYP_INCLUDED
#define PROTOTYP_INCLUDED

/*

 Global Functions, Structures and Variables are prototyped here

*/


#include "shpanim.h"

#ifdef __cplusplus

	extern "C" {

#endif



#include "mem3dc.h"




/*

 Structures, Unions & Enums

*/

extern const int sine[4096];
extern const int cosine[4096];


typedef struct vectorch {

	int vx;
	int vy;
	int vz;

} VECTORCH;

typedef struct quat {

	int quatw;
	int quatx;
	int quaty;
	int quatz;

} QUAT;

typedef struct vectorchf {

	float vx;
	float vy;
	float vz;

} VECTORCHF;

void FNormalise(VECTORCHF *n);

typedef struct vector2d {

	int vx;
	int vy;

} VECTOR2D;

typedef struct vector2df {

	float vx;
	float vy;

} VECTOR2DF;

void FNormalise2d(VECTOR2DF *n);

typedef struct line {

	VECTORCH v0;
	VECTORCH v1;

} LINE;



typedef struct euler {

	int EulerX;
	int EulerY;
	int EulerZ;

} EULER;


typedef struct angularvelocity {

	EULER AngV;

	int PitchCount;
	int YawCount;
	int RollCount;

} ANGULARVELOCITY;


typedef struct matrixch {

	int mat11;
	int mat12;
	int mat13;

	int mat21;
	int mat22;
	int mat23;

	int mat31;
	int mat32;
	int mat33;

} MATRIXCH;

typedef struct matrixchf {

	float mat11;
	float mat12;
	float mat13;

	float mat21;
	float mat22;
	float mat23;

	float mat31;
	float mat32;
	float mat33;

} MATRIXCHF;

/* Sorry about this... */

//#include "hmodel.h"

struct hmodelcontroller;


/*

 This structure is used by map function "MapSetVDB()" to pass parameters
 to the "SetVDB()" function.

*/

typedef struct mapsetvdb {

	int SVDB_Flags;
	int SVDB_ViewType;

	int SVDB_Depth;

	int SVDB_CentreX;
	int SVDB_CentreY;

	int SVDB_ProjX;
	int SVDB_ProjY;
	int SVDB_MaxProj;

	int SVDB_ClipLeft;
	int SVDB_ClipRight;
	int SVDB_ClipUp;
	int SVDB_ClipDown;

	int SVDB_H1;
	int SVDB_H2;
	int SVDB_HInterval;
	int SVDB_HColour;
	int SVDB_Ambience;

	int SVDB_ObViewState;
	int SVDB_ObViewDistance;
	int SVDB_ObPanX;
	int SVDB_ObPanY;


} MAPSETVDB;


/*

 Simplified Shapes

 The array of shapes MUST be terminated by "-1"

*/

typedef struct simshapelist {

	int SSL_Threshold;			/* 1st trans. for proj. r */
	int *SSL_Shapes;				/* ptr to array of shapes */

} SIMSHAPELIST;


/*

 Map Structures

 Each map is made up of a collection of arrays of these different block
 types. Access to them is provided by the header block (see below).

 Map TYPES are defined in the project specific file "equates.h"

*/



typedef struct mapblock1 {

	int MapType;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

} MAPBLOCK1;


typedef struct mapblock2 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

} MAPBLOCK2;


typedef struct mapblock3 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

	VECTORCH MapWorld;

} MAPBLOCK3;


typedef struct mapblock4 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

	VECTORCH MapWorld;

	EULER MapEuler;

} MAPBLOCK4;


typedef struct mapblock5 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

	VECTORCH MapWorld;

	EULER MapEuler;

	int MapFlags;


} MAPBLOCK5;


typedef struct mapblock6 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

	VECTORCH MapWorld;

	EULER MapEuler;

	int MapFlags;


	MAPSETVDB *MapVDBData;

	int MapInteriorType;

} MAPBLOCK6;


typedef struct mapblock7 {

	int MapType;
	int MapShape;

	#if LoadingMapsShapesAndTexturesEtc

		int MapFNameIndex;
		char **MapFNameArray;
		SHAPEHEADER **MapShapeDataArray;

	#endif

	VECTORCH MapWorld;

	EULER MapEuler;

	int MapFlags;
	int MapFlags2;
	int MapFlags3;


	MAPSETVDB *MapVDBData;

	int MapInteriorType;

	int MapLightType;			/* See LIGHTTYPES */


	VECTORCH MapOrigin;			/* Origin of Rotation */

	SIMSHAPELIST *MapSimShapes;

	int MapViewType;			/* See "VDB_ViewType" */


} MAPBLOCK7;


typedef struct mapblock8 {

	int MapType;
	int MapShape;

	VECTORCH MapWorld;

	EULER MapEuler;

	int MapFlags;
	int MapFlags2;
	int MapFlags3;

	MAPSETVDB *MapVDBData;

	int MapInteriorType;

	int MapLightType;			/* See LIGHTTYPES */


	VECTORCH MapOrigin;			/* Origin of Rotation */

	SIMSHAPELIST *MapSimShapes;

	int MapViewType;			/* See "VDB_ViewType" */

	struct displayblock **MapMPtr;	/* Write our dptr here as mother */
	struct displayblock **MapDPtr;	/* Read our dptr here as daughter */

	VECTORCH MapMOffset;					/* Offset from mother */

} MAPBLOCK8;


/*

 Map Header Block

*/

typedef struct mapheader {

	MAPBLOCK1 *MapType1Objects;
	MAPBLOCK2 *MapType2Objects;
	MAPBLOCK3 *MapType3Objects;
	MAPBLOCK4 *MapType4Objects;
	MAPBLOCK5 *MapType5Objects;
	MAPBLOCK6 *MapType6Objects;
	MAPBLOCK7 *MapType7Objects;
	MAPBLOCK8 *MapType8Objects;

} MAPHEADER;



/*

 Physical Screen Descriptor Block

 Only one of these exists and it is filled out by the Video Mode Set
 function.

 Functions use these parameters in conjunction with those in the
 View Descriptor Block to calculate Screen and View scaling factors.

*/

typedef struct screendescriptorblock {

	int SDB_Width;
	int SDB_Height;
	int SDB_Depth;
	int SDB_ScreenDepth;
	int SDB_Size;

	int SDB_DiagonalWidth;

	int SDB_CentreX;
	int SDB_CentreY;

	int SDB_ProjX;
	int SDB_ProjY;
	int SDB_MaxProj;

	int SDB_ClipLeft;
	int SDB_ClipRight;
	int SDB_ClipUp;
	int SDB_ClipDown;

	int SDB_Flags;

	int SDB_ViewAngle;
	int SDB_ViewAngleCos;

	unsigned int TLTSize;
	unsigned int TLTShift;

} SCREENDESCRIPTORBLOCK;


/*

 Screen Descriptor Block Flags

 Set these BEFORE setting the video mode

*/

#define SDB_Flag_222                0x00000001		/* 8-bit mode 222 texture palette */
#define SDB_Flag_Raw256             0x00000002		/* 8-bit mode, no texture remap */

#define SDB_Flag_MIP                0x00000004		/* Create MIP maps for images */

#define SDB_Flag_SuperSample2x2     0x00000008		/* 8T and 24 only */

#define SDB_Flag_DrawFrontToBack    0x00000010		/* Useful if Z-Buffering */

#define SDB_Flag_TLTPalette         0x00000020      /* 8Raw only, Images may have ih_flag_tlt and the tlt maps colours from an abstract palette to the screen palette */
#define SDB_Flag_TLTSize            0x00000040      /* The TLTSize member is valid, o/w TLTSize is 256 */
#define SDB_Flag_TLTShift           0x00000080      /* The TLTShift member is valid because the TLTSize is a power of two */



/*

 Clip Plane Block

*/

typedef struct clipplaneblock {

	VECTORCH CPB_Normal;
	VECTORCH CPB_POP;

} CLIPPLANEBLOCK;


/*

 Clip Plane Points

*/

typedef struct clipplanepoints {

	VECTORCH cpp1;
	VECTORCH cpp2;
	VECTORCH cpp3;

} CLIPPLANEPOINTS;

/*

 View Descriptor Block

 View location and orientation will come from the currently designated view
 object. A pointer to an object block will not be essential, which is why I
 have added a location to the viewport block. The matrix exists in the
 viewport block because it is not the same matrix that appears in the object
 block. The object block has a matrix that takes it from local space to world
 space. I need the transpose of this matrix to create a world space to view
 space transformation.

 See "3dc\docs\c*.doc" for more information.

*/

typedef struct viewdescriptorblock {

	struct viewdescriptorblock *VDB_HigherP;
	struct viewdescriptorblock *VDB_LowerP;

	int VDB_ViewType;	/* To match ObViewType, used by image backdrops */

	int VDB_Priority;	/* Determines draw order */

	int VDB_Flags;

	int VDB_ViewAngle;
	int VDB_ViewAngleCos;

	int VDB_Width;
	int VDB_Height;
	int VDB_Depth;
	int VDB_ScreenDepth;

	int VDB_CentreX;
	int VDB_CentreY;

	int VDB_ProjX;
	int VDB_ProjY;
	int VDB_MaxProj;

	int VDB_ClipZ;
	int VDB_ClipLeft;
	int VDB_ClipRight;
	int VDB_ClipUp;
	int VDB_ClipDown;

	CLIPPLANEBLOCK VDB_ClipZPlane;
	CLIPPLANEBLOCK VDB_ClipLeftPlane;
	CLIPPLANEBLOCK VDB_ClipRightPlane;
	CLIPPLANEBLOCK VDB_ClipUpPlane;
	CLIPPLANEBLOCK VDB_ClipDownPlane;

	struct displayblock *VDB_ViewObject;

	VECTORCH VDB_World;

	MATRIXCH VDB_Mat;
	MATRIXCH VDB_HorizonMat;
	MATRIXCH VDB_SpriteMat;

	EULER VDB_MatrixEuler;


	int VDB_H1;
	int VDB_H2;
	int VDB_HInterval;

	int VDB_HColour;						/* "Sky" */
	int VDB_HColour8;						/* For 8-bit colour indirected modes */

	int VDB_HGColour;						/* "Ground" */
	int VDB_HGColour8;					/* For 8-bit colour indirected modes */

	int VDB_Ambience;

	#if pc_backdrops
	BACKDROPTYPE VDB_BackdropType;
	unsigned short VDB_ProjectorXOffsets[MaxScreenWidth];
	#endif

	#if ProjectSpecificVDBs
	void* VDB_ProjectSpecificHook;
	#endif

} VIEWDESCRIPTORBLOCK;


/* Flags */

#define ViewDB_Flag_SingleBuffer	0x00000001
#define ViewDB_Flag_DoubleBuffer	0x00000002
#define ViewDB_Flag_FullSize		0x00000004	/* Use fast screen clear */
#define ViewDB_Flag_NoBackdrop	0x00000008

#define ViewDB_Flag_LTrunc			0x00000010	/* Informs the VDB creator		*/
#define ViewDB_Flag_RTrunc			0x00000020	/* that a physical screen		*/
#define ViewDB_Flag_UTrunc			0x00000040	/* violation has forced a		*/
#define ViewDB_Flag_DTrunc			0x00000080	/* truncation of the viewport	*/

#define ViewDB_Flag_Hazing			0x00000100
#define ViewDB_Flag_DontDraw		0x00000200
#define ViewDB_Flag_AdjustScale	0x00000400	/* Scale 320x200 definition up to equivalent size for the mode */
#define ViewDB_Flag_AddSubject	0x00000800	/* For MapSetVDB, telling it to add dptr_last to the dptr */

#define ViewDB_Flag_NeedToFlushZ	0x00001000	/* Cleared by flush function */


#define ViewDB_Flag_ImageBackdrop 0x00004000	/* This requires a backdrop
																image array, accessed through
																"Global_SceneBackdropPtr" */

#define ViewDB_Flag_Horizon		0x00008000	/* Draw a "traditional"
																Sky/Ground horizon - before
																the backdrop is drawn */

#define ViewDB_Flag_UseBackdropImageColoursForHorizon	0x00010000

#define ViewDB_Flag_NoScreenClear							0x00020000

#define ViewDB_Flag_NoModules									0x00040000


/*

 A VDB global "iflag_drawtx3das2d" that forces all 3d textures to be drawn as 2d
 according to the "iflag_drawtx3das2d" pipeline.

 WARNING!
 
 Only use this when drawing per object or per vdb as it uses the global vdb pointer

*/

#define ViewDB_Flag_drawtx3das2d	0x00080000


/* test the flags with "& VDB_Trunc" to see if any truncation occurred */

#define VDB_Trunc (ViewDB_Flag_LTrunc | ViewDB_Flag_RTrunc | ViewDB_Flag_UTrunc | ViewDB_Flag_DTrunc)



/*

 Light Source Data Structures

*/


/*

 Light Types

*/

typedef enum {

	LightType_Infinite,		/* Default */
	LightType_PerObject,
	LightType_PerVertex

} LIGHTTYPES;


typedef struct lightblock {

	int LightFlags;
	int LightType;

	VECTORCH LightWorld;		/* World space light position */

	VECTORCH LocalLP;			/* Light position in object local space */

	int LightBright;			/* 0->1 Fixed Point */
	int LightRange;

	int BrightnessOverRange;

	/* these RGB components take values 0-65536 */
	int RedScale;
	int GreenScale;
	int BlueScale;

	int LightBrightStore;

} LIGHTBLOCK;

/*

 Light Flags

*/

#define LFlag_CosAtten			0x00000001		/* Cosine attenuation */
#define LFlag_CosSpreadAtten	0x00000002		/* Cosine spread attenuation */
#define LFlag_Omni				0x00000004		/* Omnidirectional */
#define LFlag_Deallocate		0x00000008		/* Deallocate at frame end */
#define LFlag_NoShadows			0x00000010		/* Prelighting - No Shadows */
#define LFlag_Off				0x00000020		/* Prelighting - Light OFF */

#define LFlag_PreLitSource		0x00000040		/* WARNING: Not obj. specific */

#define LFlag_WasNotAllocated	0x00000080		/* Supplied by another */

#define LFlag_AbsPos			0x00000100		/* Pos. not rel. to parent */
#define LFlag_AbsOff			0x00000200		/* Offset not rel. to parent */

#define LFlag_AbsLightDir		0x00000400		/* Light dir. not rel. to p. */

#define LFlag_NoSpecular		0x00000800	

#define LFlag_Electrical		0x00001000
#define LFlag_Thermal			0x00002000

/* KJL 16:17:42 01/10/98 - used to specify no specular component to the light;
avoids unnecessary texture wash-out. */

#if SupportMorphing


/*

 Morph Frame

*/

typedef struct morphframe {

	int mf_shape1;
	int mf_shape2;

} MORPHFRAME;


/*

 Morph Header

*/

typedef struct morphheader {

	int mph_numframes;
	int mph_maxframes;
	MORPHFRAME *mph_frames;

} MORPHHEADER;


/*

 Display Pipeline Structure

*/

typedef struct morphdisplay {

	int md_lerp;
	int md_one_minus_lerp;
	int md_shape1;
	int md_shape2;
	SHAPEHEADER *md_sptr1;
	SHAPEHEADER *md_sptr2;

} MORPHDISPLAY;


/*

 Morph Control Structure

*/

typedef struct morphctrl {

	int ObMorphCurrFrame;
	int ObMorphFlags;
	int ObMorphSpeed;
	MORPHHEADER *ObMorphHeader;

} MORPHCTRL;


#endif /* SupportMorphing */



/*

 Object Display Block

*/

typedef struct displayblock
{
	int ObShape;

	struct sfxblock *SfxPtr;

	SHAPEHEADER* ObShapeData;

	char * name;
	
	#if (SupportMorphing && LazyEvaluationForMorphing)
	VECTORCH *ObMorphedPts;
	#endif

 	VECTORCH ObWorld;		/* World Space Location */
	EULER ObEuler;			/* Euler Orientation */
	MATRIXCH ObMat;			/* Local -> World Orientation Matrix */
	
	int ObFlags;
	int ObFlags2;
	int ObFlags3;

	/* Lights */
 	int ObNumLights;
	LIGHTBLOCK *ObLights[MaxObjectLights];
 
	#if SupportModules
	struct module *ObMyModule;	/* This is our module */
	struct module *ObModule;	/* We are in this module */
	#endif

	VECTORCH ObView;			/* View Space Location */
 
	struct viewdescriptorblock *ObVDBPtr;
 
  	/* Lights */
 	int ObLightType;								/* See LIGHTTYPES above */
 
	/* Extent */
 	int ObRadius;		/* max(sqr(x^2+y^2+z^2)) */
 	int ObMaxX;
	int ObMinX;
 	int ObMaxY;
	int ObMinY;
 	int ObMaxZ;
	int ObMinZ;

	struct txactrlblk *ObTxAnimCtrlBlks;

	EXTRAITEMDATA *ObEIDPtr;				/* Overrides shape EID pointer */

	#if SupportMorphing
	MORPHCTRL *ObMorphCtrl;				/* Structure provided by project */
	#endif

	/* The Strategy Block Pointer */
 	struct strategyblock *ObStrategyBlock;	/* Defined in stratdef.h */
 	
	SHAPEANIMATIONCONTROLLER * ShapeAnimControlBlock;

	struct hmodelcontroller * HModelControlBlock;
	
	unsigned int SpecialFXFlags;	
											 
} DISPLAYBLOCK;


/*

 Flags

*/

#define ObFlag_InRange			0x00000001
#define ObFlag_OnScreen			0x00000002
#define ObFlag_NotVis			0x00000004




#define ObFlag_VertexHazing	0x00000020	/* For I_Gouraud, interpolate hue
															across the polygon */


#define ObFlag_TypeZ				0x00000080	/* Shape uses Z Sort */




#define ObFlag_SortFarZ			0x00008000	/* Z + Radius */

#define ObFlag_ArbRot			0x00010000	/* Internal use ONLY */

#define ObFlag_MultLSrc			0x00020000	/* Use Multiple Light Sources */
#define ObFlag_NoInfLSrc		0x00040000	/* Ignore Infinite Light Sources */
#define ObFlag_OnlyInfLSrc		0x00080000	/* Only Infinite Light Sources */






#define ObFlag_ZBuffer			0x08000000	/* Request item z-buffering */

#define ObFlag_BFCRO				0x10000000	/* Back Face Cull Rot. Optimise */
#define ObFlag_RSP				0x20000000	/* Radius Space Partitioning -
															requires RFC data in shape */


#define ObFlag_ParrallelBFC   0x80000000  /* Roxby's scu dsp flag */


/*

 Flags 2

*/

#define ObFlag2_Deallocate		0x00000001	/* Deallocate block at frame end */
#define ObFlag2_ObjLevelHaze	0x00000002	/* Hazing at object level */


#define ObFlag2_AugZ				0x00000008	/* Augmented Z-Sort */




#define ObFlag2_NoBFC			0x00000400	/* Disables Back Face Cull */




#define ObFlag2_NotYetPos		0x00080000	/* Internal, for sub-objects */


#define ObFlag2_NoSubObjectUpdate 0x00200000	/* If you are using your own
																functions to position and
																orient sub-objects, set this
																flag and the sub-object in
																question will be ignored by
																the update function */






#define ObFlag2_SortNearZ	0x20000000				/* Z - Radius */




#define ObFlag2_SortD	0x80000000		/* Use distance rather than Z */



/*

 Flags 3

*/

#define ObFlag3_DynamicModuleObject 0x00000001	/* Allocate module objects
																	around this object as if
																	it were a camera */

#define ObFlag3_ObjectSortedItems	0x00000002	/* Used by the Global Sort */

#define ObFlag3_NoLightDot				0x00000004	/* Surface/Point is lit the
																	same from all angles */


#define ObFlag3_Teleport				0x00000040	/* No motion vector! */

#define ObFlag3_SurfaceAlignDeluxe	0x00000080	/* Better but slower */

#define ObFlag3_PreLit					0x00000100	/* Not source specific */

#define ObFlag3_JustCreated			0x00000200	/* Teleport status for just
																	one frame */

#define ObFlag3_DontDrawIfOurModuleIsNotVis 0x00000400	/* If the module we
																				visible, don't
																				draw us */
#define ObFlag3_AlwaysDynamic			0x00000800	/* Overrides auto-detect */




#if SupportMorphing

/*

 Morphing Flags

*/

#define mph_flag_play					0x00000001
#define mph_flag_reverse				0x00000002
#define mph_flag_noloop	 				0x00000004

/*

 These four are set so that functions can be informed as to the state of
 a sequence. They must be acknowledged and cleared for them to be of any
 lasting use. The "start" and "end" flags are always set when the sequence
 wraps. Depending on whether the sequence is looping or not, the "looped" or
 "finished" flags will be set respectively.

*/

#define mph_flag_start					0x00000008
#define mph_flag_end						0x00000010
#define mph_flag_finished				0x00000020
#define mph_flag_looped					0x00000040

#endif



/*

 Standard Points

*/

typedef struct p2d {

	VECTOR2D point2d;

} P2D;

typedef struct p3d {

	VECTORCH point3d;

} P3D;

/*

 Gouraud Points

*/

typedef struct p2d_gouraud {

	VECTOR2D point2d;
	int i2d;

} P2D_GOURAUD;

typedef struct p3d_gouraud {

	VECTORCH point3d;
	int i3d;

} P3D_GOURAUD;

/*

 Phong Points

*/

typedef struct p2d_phong {

	VECTOR2D point2d;
	VECTORCH phong_normal2d;

} P2D_PHONG;


typedef struct p3d_phong {

	VECTORCH point3d;
	VECTORCH phong_normal3d;

} P3D_PHONG;


/*

 Texture 2d Points

*/

typedef struct p2d_texture2d {

	VECTOR2D point2d;
	int u2d_2d;
	int v2d_2d;

} P2D_TEXTURE2D;


typedef struct p3d_texture2d {

	VECTORCH point3d;
	int u3d_2d;
	int v3d_2d;

} P3D_TEXTURE2D;


/*

 Texture 2d Points - Z-Buffered

*/

#if SupportZBuffering

typedef struct p2d_texture2d_zb {

	VECTOR2D point2d;
	int u2d_2d;
	int v2d_2d;
	float z2d_2d;

} P2D_TEXTURE2D_ZB;


typedef struct p3d_texture2d_zb {

	VECTORCH point3d;
	int u3d_2d;
	int v3d_2d;
	float z3d_2d;

} P3D_TEXTURE2D_ZB;

#endif


/*

 Texture 3d Points

*/

#if support3dtextures

#if int3dtextures

typedef struct p2d_texture3d {

	VECTOR2D point2d;
	int u2d_tx3d;
	int v2d_tx3d;
	int z2d_tx3d;

} P2D_TEXTURE3D;


typedef struct p3d_texture3d {

	VECTORCH point3d;
	int u3d_tx3d;
	int v3d_tx3d;

} P3D_TEXTURE3D;

#else

typedef struct p2d_texture3d {

	VECTOR2D point2d;
	float u2d_tx3d;
	float v2d_tx3d;
	float z2d_tx3d;

} P2D_TEXTURE3D;


typedef struct p3d_texture3d {

	VECTORCH point3d;
	int u3d_tx3d;
	int v3d_tx3d;

} P3D_TEXTURE3D;

#endif

#endif

/*

 Gouraud Texture 2d Points

*/

typedef struct p2d_gouraudtexture2d {

	VECTOR2D point2d;
	int u2d_2d;
	int v2d_2d;
	int i2d_2d;

} P2D_GOURAUDTEXTURE2D;


typedef struct p3d_gouraudtexture2d {

	VECTORCH point3d;
	int u3d_2d;
	int v3d_2d;
	int i3d_2d;

} P3D_GOURAUDTEXTURE2D;


/*

 Gouraud Texture 2d Points - Z-Buffered

*/

#if SupportZBuffering

typedef struct p2d_gouraudtexture2d_zb {

	VECTOR2D point2d;
	int u2d_gtx2d;
	int v2d_gtx2d;
	int i2d_gtx2d;
	float z2d_gtx2d;

} P2D_GOURAUDTEXTURE2D_ZB;


typedef struct p3d_gouraudtexture2d_zb {

	VECTORCH point3d;
	int u3d_gtx2d;
	int v3d_gtx2d;
	int i3d_gtx2d;
	float z3d_gtx2d;

} P3D_GOURAUDTEXTURE2D_ZB;

#endif


#if SupportGouraud3dTextures

/*

 Gouraud Texture 3d Points

*/

typedef struct p2d_gouraudtexture3d {

	VECTOR2D point2d;
	float u2d_gtx3d;
	float v2d_gtx3d;
	float z2d_gtx3d;
	int i2d_gtx3d;

} P2D_GOURAUDTEXTURE3D;


typedef struct p3d_gouraudtexture3d {

	VECTORCH point3d;
	int u3d_gtx3d;
	int v3d_gtx3d;
	int i3d_gtx3d;

} P3D_GOURAUDTEXTURE3D;

#endif	/* SupportGouraud3dTextures */



/*

 Polygon Points.

 These are the points as they appear to a Scan Convertor. Item pointers are
 cast as these structs by the SC to make their life easier.

*/


/*

 I_Polygon

*/

typedef struct i_polygon_pt {

	int i_x;
	int i_y;

} I_POLYGON_PT;


/*

 I_Polygon_ZBuffer

*/

#if ZBufferTest

typedef struct i_polygon_zbuffer_pt {

	int i_x;
	int i_y;
	int i_z;

} I_POLYGON_ZBUFFER_PT;

#endif

#if SupportZBuffering

typedef struct i_polygon_zbuffer_pt {

	int i_x;
	int i_y;
	float i_z;

} I_POLYGON_ZBUFFER_PT;

#endif


/*

 I_GouraudPolygon

*/

typedef struct i_gouraudpolygon_pt {

	int i_x;
	int i_y;
	int i_int;

} I_GOURAUDPOLYGON_PT;


/*

 I_GouraudPolygon_ZBuffer

*/

#if ZBufferTest

typedef struct i_gouraudpolygon_zbuffer_pt {

	int i_x;
	int i_y;
	int i_int;
	int i_gz;

} I_GOURAUDPOLYGON_ZBUFFER_PT;

#endif

#if SupportZBuffering

typedef struct i_gouraudpolygon_zbuffer_pt {

	int i_x;
	int i_y;
	int i_int;
	float i_gz;

} I_GOURAUDPOLYGON_ZBUFFER_PT;

#endif


/*

 I_PhongPolygon

*/

typedef struct i_phongpolygon_pt {

	int i_x;
	int i_y;

	VECTORCH i_n;

} I_PHONGPOLYGON_PT;

#if SupportZBuffering

typedef struct i_phongpolygon_zbuffer_pt {

	int i_x;
	int i_y;

	VECTORCH i_n;

	float i_pz;

} I_PHONGPOLYGON_ZBUFFER_PT;

#endif


/*

 I_2dTexturePolygon

*/

typedef struct i_2dtexturepolygon_pt {

	int i_x;
	int i_y;

	int i_u;
	int i_v;

} I_2DTEXTUREPOLYGON_PT;

#if SupportZBuffering

typedef struct i_2dtexturepolygon_zbuffer_pt {

	int i_x;
	int i_y;

	int i_u;
	int i_v;

	float i_tx2dz;

} I_2DTEXTUREPOLYGON_ZBUFFER_PT;

#endif


/*

 I_3dTexturePolygon

*/

#if support3dtextures

#if int3dtextures

typedef struct i_3dtexturepolygon_pt {

	int i_x;
	int i_y;

	int i_tx3d_u;
	int i_tx3d_v;
	int i_tx3d_z;

} I_3DTEXTUREPOLYGON_PT;

#else

typedef struct i_3dtexturepolygon_pt {

	int i_x;
	int i_y;

	float i_tx3d_u;
	float i_tx3d_v;
	float i_tx3d_z;

} I_3DTEXTUREPOLYGON_PT;

#endif

#if SupportZBuffering

typedef struct i_3dtexturepolygon_zbuffer_pt {

	int i_x;
	int i_y;

	float i_tx3d_u;
	float i_tx3d_v;
	float i_tx3d_z;

} I_3DTEXTUREPOLYGON_ZBUFFER_PT;

#endif

#endif


/*

 I_Gouraud2dTexturePolygon

*/

typedef struct i_gouraud2dtexturepolygon_pt {

	int i_x;
	int i_y;

	int i_u;
	int i_v;

	int i_i;

} I_GOURAUD2DTEXTUREPOLYGON_PT;


#if SupportZBuffering

typedef struct i_gouraud2dtexturepolygon_zbuffer_pt {

	int i_x;
	int i_y;

	int i_u;
	int i_v;

	int i_i;

	float i_gtx2dz;

} I_GOURAUD2DTEXTUREPOLYGON_ZBUFFER_PT;

#endif


#if SupportGouraud3dTextures

/*

 I_Gouraud3dTexturePolygon

*/

typedef struct i_gouraud3dtexturepolygon_pt {

	int i_x;
	int i_y;

	float i_gtx3d_u;
	float i_gtx3d_v;
	float i_gtx3d_z;

	int i_gtx3d_i;

} I_GOURAUD3DTEXTUREPOLYGON_PT;

#endif	/* SupportGouraud3dTextures */


/*

 Clip Point Structure.

 Points P, S, F and I use this structure.

*/

typedef struct clip_point {

	VECTORCH ClipPoint;
	VECTORCH ClipNormal;
	TEXEL ClipTexel;
	int ClipInt;
	int ClipZBuffer;

} CLIP_POINT;


#if support3dtextures

#if int3dtextures

typedef struct clip_point_f {

	VECTORCH ClipPointF;
	VECTORCH ClipNormalF;
	TEXELF ClipTexelF;
	int ClipIntF;
	int ClipZBufferF;

} CLIP_POINT_F;

#else

typedef struct clip_point_f {

	VECTORCH ClipPointF;
	VECTORCH ClipNormalF;
	TEXELF ClipTexelF;
	int ClipIntF;
	float ClipZBufferF;

} CLIP_POINT_F;

#endif

#endif


#if SupportGouraud3dTextures

typedef struct clip_point_gtx3d {

	VECTORCH ClipPointF;
	VECTORCH ClipNormalF;
	TEXELGTX3D ClipTexelF;
	int ClipIntF;
	float ClipZBufferF;

} CLIP_POINT_GTX3D;

#endif




/*

 Scan Data Blocks.

 These are the structures in ScanData[] that the SC outputs. Memory is
 allocated from ScanData[] as int* but is cast to the following structures
 by the SC for ease of use.

*/

typedef struct i_polygon_scan {

	int ips_colour;
	int ips_x1;
	int ips_x2;
	int ips_y;

} I_POLYGON_SCAN;

#if ZBufferTest

typedef struct i_polygon_zbuffer_scan {

	int ips_colour;
	int ips_x1;
	int ips_x2;
	int ips_y;
	int ips_z1;
	int ips_z2;

} I_POLYGON_ZBUFFER_SCAN;

#endif

#if SupportZBuffering

typedef struct i_polygon_zbuffer_scan {

	int ips_colour;
	int ips_x1;
	int ips_x2;
	int ips_y;
	float ips_z1;
	float ips_z2;

} I_POLYGON_ZBUFFER_SCAN;

#endif


typedef struct i_gouraudpolygon_scan {

	int igs_c1;
	int igs_c2;

	int igs_x1;
	int igs_x2;

	int igs_y;

} I_GOURAUDPOLYGON_SCAN;

#if ZBufferTest

typedef struct i_gouraudpolygon_zbuffer_scan {

	int igs_c1;
	int igs_c2;

	int igs_x1;
	int igs_x2;

	int igs_y;

	int igs_z1;
	int igs_z2;

} I_GOURAUDPOLYGON_ZBUFFER_SCAN;

#endif

#if SupportZBuffering

typedef struct i_gouraudpolygon_zbuffer_scan {

	int igs_c1;
	int igs_c2;

	int igs_x1;
	int igs_x2;

	int igs_y;

	float igs_z1;
	float igs_z2;

} I_GOURAUDPOLYGON_ZBUFFER_SCAN;

#endif


typedef struct i_phongpolygon_scan {

	VECTORCH ips_n1;
	VECTORCH ips_n2;

	int ips_x1;
	int ips_x2;

	int ips_y;

} I_PHONGPOLYGON_SCAN;

#if SupportZBuffering

typedef struct i_phongpolygon_zbuffer_scan {

	VECTORCH ips_n1;
	VECTORCH ips_n2;

	int ips_x1;
	int ips_x2;

	int ips_y;

	float ips_z1;
	float ips_z2;

} I_PHONGPOLYGON_ZBUFFER_SCAN;

#endif


typedef struct i_2dtexturepolygon_scan {

	int i2s_u1;
	int i2s_v1;

	int i2s_u2;
	int i2s_v2;

	int i2s_x1;
	int i2s_x2;

	int i2s_y;

} I_2DTEXTUREPOLYGON_SCAN;

#if SupportZBuffering

typedef struct i_2dtexturepolygon_zbuffer_scan {

	int i2s_u1;
	int i2s_v1;

	int i2s_u2;
	int i2s_v2;

	int i2s_x1;
	int i2s_x2;

	int i2s_y;

	float i2s_z1;
	float i2s_z2;

} I_2DTEXTUREPOLYGON_ZBUFFER_SCAN;

#endif


#if support3dtextures

#if int3dtextures

typedef struct i_3dtexturepolygon_scan {

	int i3s_u1;
	int i3s_v1;
	int i3s_z1;

	int i3s_u2;
	int i3s_v2;
	int i3s_z2;

	int i3s_x1;
	int i3s_x2;

	int i3s_y;

} I_3DTEXTUREPOLYGON_SCAN;

#else

typedef struct i_3dtexturepolygon_scan {

	float i3s_u1;
	float i3s_v1;
	float i3s_z1;

	float i3s_u2;
	float i3s_v2;
	float i3s_z2;

	int i3s_x1;
	int i3s_x2;

	int i3s_y;

} I_3DTEXTUREPOLYGON_SCAN;

#endif

#if SupportZBuffering

typedef struct i_3dtexturepolygon_zbuffer_scan {

	float i3s_u1;
	float i3s_v1;
	float i3s_z1;

	float i3s_u2;
	float i3s_v2;
	float i3s_z2;

	int i3s_x1;
	int i3s_x2;

	int i3s_y;

} I_3DTEXTUREPOLYGON_ZBUFFER_SCAN;

#endif

#endif


typedef struct i_gouraud2dtexturepolygon_scan {

	int ig2s_u1;
	int ig2s_v1;
	int ig2s_c1;

	int ig2s_u2;
	int ig2s_v2;
	int ig2s_c2;

	int ig2s_x1;
	int ig2s_x2;

	int ig2s_y;

} I_GOURAUD2DTEXTUREPOLYGON_SCAN;

#if SupportZBuffering

typedef struct i_gouraud2dtexturepolygon_zbuffer_scan {

	int ig2s_u1;
	int ig2s_v1;
	int ig2s_c1;

	int ig2s_u2;
	int ig2s_v2;
	int ig2s_c2;

	int ig2s_x1;
	int ig2s_x2;

	int ig2s_y;

	float ig2s_z1;
	float ig2s_z2;

} I_GOURAUD2DTEXTUREPOLYGON_ZBUFFER_SCAN;

#endif


#if SupportGouraud3dTextures

typedef struct i_gouraud3dtexturepolygon_scan {

	float ig3s_u1;
	float ig3s_v1;
	float ig3s_z1;
	int ig3s_c1;

	float ig3s_u2;
	float ig3s_v2;
	float ig3s_z2;
	int ig3s_c2;

	int ig3s_x1;
	int ig3s_x2;

	int ig3s_y;

} I_GOURAUD3DTEXTUREPOLYGON_SCAN;

#endif	/* SupportGouraud3dTextures */




/*

 Functions

*/

void ClearScreen(SCREENDESCRIPTORBLOCK *sdb, int Colour);


void PlatformSpecificShowViewEntry(VIEWDESCRIPTORBLOCK *vdb, SCREENDESCRIPTORBLOCK *sdb);
void PlatformSpecificShowViewExit(VIEWDESCRIPTORBLOCK *vdb, SCREENDESCRIPTORBLOCK *sdb);
void AddShape(DISPLAYBLOCK *dblockptr, VIEWDESCRIPTORBLOCK *VDB_Ptr);
void PrepareVDBForShowView(VIEWDESCRIPTORBLOCK *VDB_Ptr);


void SetupLight(
	LIGHTBLOCK *lptr,
	int sl_flags,
	int sl_type,
	VECTORCH *sl_world,
	VECTORCH *sl_dir,
	int sl_panx,
	int sl_pany,
	int sl_bright,
	int sl_spread,
	int sl_range
);

void UpdateObjectLights(DISPLAYBLOCK *dptr);

DISPLAYBLOCK* ReadMap(MAPHEADER *mapptr);

void MapPostProcessing(DISPLAYBLOCK *dptr);
void ObjectQuatAndMat(DISPLAYBLOCK *dblockptr);
void MapBlockInit(DISPLAYBLOCK *dblockptr);
void MapSetVDB(DISPLAYBLOCK *dptr, MAPSETVDB *mapvdbdata);

#if ProjectSpecificVDBs
void ProjectSpecificVDBDestroy(VIEWDESCRIPTORBLOCK *vdb);
void ProjectSpecificVDBInit(VIEWDESCRIPTORBLOCK *vdb);
#endif


void UpdateGame(void);





SHAPEHEADER* GetShapeData(int shapenum);


void InitialiseObjectBlocks(void);

DISPLAYBLOCK* AllocateObjectBlock(void);
void DeallocateObjectBlock(DISPLAYBLOCK *dblockptr);

DISPLAYBLOCK* CreateActiveObject(void);
int DestroyActiveObject(DISPLAYBLOCK *dblockptr);


void InitialiseStrategyBlocks(void);
struct strategyblock* AllocateStrategyBlock(void);
void DeallocateStrategyBlock(struct strategyblock *sptr);

struct strategyblock* CreateActiveStrategyBlock(void);
int DestroyActiveStrategyBlock(struct strategyblock*dblockptr);




void InitialiseTxAnimBlocks(void);
TXACTRLBLK* AllocateTxAnimBlock(void);
void DeallocateTxAnimBlock(TXACTRLBLK *TxAnimblockptr);
void AddTxAnimBlock(DISPLAYBLOCK *dptr, TXACTRLBLK *taptr);
TXANIMHEADER* GetTxAnimHeaderFromShape(TXACTRLBLK *taptr, int shape);
void UpdateTxAnim(TXANIMHEADER *txah);
void ChangeSequence(TXANIMHEADER *txah_old, TXANIMHEADER *txah_new);
void ControlTextureAnimation(DISPLAYBLOCK *dptr);





int DisplayAndLightBlockDeallocation(void);


void InitialiseLightBlocks(void);

LIGHTBLOCK* AllocateLightBlock(void);
void DeallocateLightBlock(LIGHTBLOCK *lptr);

LIGHTBLOCK* AddLightBlock(DISPLAYBLOCK *dptr, LIGHTBLOCK *lptr_to_add);
void DeleteLightBlock(LIGHTBLOCK *lptr, DISPLAYBLOCK *dptr);



void VDBClipPlanes(VIEWDESCRIPTORBLOCK *vdb);

void MakeClipPlane(

VIEWDESCRIPTORBLOCK *vdb,
CLIPPLANEBLOCK *cpb,
CLIPPLANEPOINTS *cpp);

void SetVDB(VIEWDESCRIPTORBLOCK *vdb,

				int fl,
				int ty,

				int d,

				int cx,
				int cy,

				int prx,
				int pry,
				int mxp,

				int cl,
				int cr,
				int cu,
				int cd,

				int h1,
				int h2,
				int hcolour,
				int ambience
			);


void InitialiseVDBs(void);

VIEWDESCRIPTORBLOCK* AllocateVDB(void);
void DeallocateVDB(VIEWDESCRIPTORBLOCK *dblockptr);

VIEWDESCRIPTORBLOCK* CreateActiveVDB(void);
int DestroyActiveVDB(VIEWDESCRIPTORBLOCK *dblockptr);

void PlatformSpecificVDBInit(VIEWDESCRIPTORBLOCK *vdb);


/* CDF 4/2/98 */
int GetOneOverSin(int a);
/* CDF 4/2/98 */
int _DotProduct(VECTORCH *v1, VECTORCH *v2);

int DotProduct2d(VECTOR2D *v1, VECTOR2D *v2);


void MakeNormal(

VECTORCH *v1,
VECTORCH *v2,
VECTORCH *v3,
VECTORCH *v4);

void GetNormalVector(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3);

void Normalise(VECTORCH *nvector);
void MNormalise(MATRIXCH *m);

void Normalise2d(VECTOR2D *nvector);

int LineColinearity(LINE *l0, LINE *l1);

void Renormalise(VECTORCH *nvector);

int Magnitude(VECTORCH *v);



int VectorDistance(VECTORCH *v1, VECTORCH *v2);
int OutcodeVectorDistance(VECTORCH *v1, VECTORCH *v2, int d);

void MatrixFromZVector(VECTORCH *v, MATRIXCH *m);


int PointInPolygon(int *point, int *polygon, int c, int ppsize);

void PolyAveragePoint(POLYHEADER *pheader, int *spts, VECTORCH *apt);

int FindShift32(int value, int limit);
int FindShift64(LONGLONGCH *value, LONGLONGCH *limit);

void MaxLONGLONGCH(LONGLONGCH *llarrayptr, int llarraysize, LONGLONGCH *llmax);

int MaxInt(int *iarray, int iarraysize);
int MinInt(int *iarray, int iarraysize);




/*

 Some Maths Functions

*/

void CreateEulerMatrix(EULER *e, MATRIXCH *m1);
void CreateEulerVector(EULER *e, VECTORCH *v);



void MatrixMultiply(

MATRIXCH *m1,
MATRIXCH *m2,
MATRIXCH *m3);

void TransposeMatrixCH(MATRIXCH *m1);

void CopyVector(VECTORCH *v1, VECTORCH *v2);
void CopyLocation(VECTORCH *v1, VECTORCH *v2);


void CopyEuler(EULER *e1, EULER *e2);
void CopyMatrix(MATRIXCH *m1, MATRIXCH *m2);

void MakeVector(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3);
void AddVector(VECTORCH *v1, VECTORCH *v2);
void SubVector(VECTORCH *v1, VECTORCH *v2);
void QuatToMat(QUAT *q,MATRIXCH *m);


void _RotateVector(

	VECTORCH *v,
	MATRIXCH *m);


void _RotateAndCopyVector(

	VECTORCH *v1,
	VECTORCH *v2,
	MATRIXCH *m);


void MakeVectorLocal(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3, MATRIXCH *m);





void MatrixToEuler(MATRIXCH *m, EULER *e);
void MatrixToEuler2(MATRIXCH *m, EULER *e);

int ArcCos(int);
int ArcSin(int);
int ArcTan(int, int);

int FandVD_Distance_2d(VECTOR2D *v0, VECTOR2D *v1);
int FandVD_Distance_3d(VECTORCH *v0, VECTORCH *v1);

int Distance_2d(VECTOR2D *v0, VECTOR2D *v1);
int Distance_3d(VECTORCH *v0, VECTORCH *v1);


/*

 Shape Language Functions

 Each function requires a pointer to SHAPEINSTR

*/

void SetupShapeLanguage(SHAPEHEADER *shapeheaderptr);

void ShapePointsInstr(SHAPEINSTR *shapeinstrptr);


void ShapeSpritePointsInstr(SHAPEINSTR *shapeinstrptr);
void ShapeSpriteRPointsInstr(SHAPEINSTR *shapeinstrptr);


void BackFaceCullPointOutcodeFlagging(void);






/*

 Platform Specific Functions

*/

void InitialiseSystem(void);
void InitialiseRenderer(void);
void ExitSystem(void);
void InitialVideoMode(void);
void RestartLevel();
void ResetFrameCounter(void);
void FrameCounterHandler(void);

void DirectWriteD3DLine(VECTOR2D* LineStart, VECTOR2D* LineEnd, int LineColour);
void* LoadImageIntoDirectDrawSurface(char *fname, IMAGEHEADER *iheader,
      int ImageLoadMode, BOOL Sysmem);
void* LoadImageIntoD3DImmediateSurface(char *fname, IMAGEHEADER *iheader,
      int TextureFileType);
void* LoadImageIntoD3DTexture(char *fname, IMAGEHEADER *iheader,
      int TextureFileType);
void ReloadImageIntoD3DImmediateSurface(IMAGEHEADER* iheader);
void* ReloadImageIntoD3DTexture(IMAGEHEADER* iheader);
int GetTextureHandle(IMAGEHEADER *imageHeaderPtr);

void* LoadFontIntoDirectDrawSurface(char *fname, IMAGEHEADER *iheader);
void ClearScreen(SCREENDESCRIPTORBLOCK* sdb, int Colour);

void InitGame(void);
void StartGame(void);
void ExitGame(void);

void InitialiseParallelStrategy(void);
void UpdateParallelStrategy(void);

unsigned char* AllocateScreenBuffer(int sbuffersize);

void SetPalette(unsigned char *palette);

void CreatePaletteRemapTable(unsigned char *palette);
unsigned char GetRemappedPaletteColour(int r, int g, int b, int bitsize);

int NearestColour(int rs, int gs, int bs, unsigned char *palette);

#if LoadingMapsShapesAndTexturesEtc

void InitialiseImageHeaders(void);
int LoadImagesForShapes(SHAPEHEADER **shapelist);

#else

int InitialiseTextures(void);

#endif

void MakeShapeTexturesGlobal(SHAPEHEADER *shptr, int TxIndex, int LTxIndex);
void MakeTxAnimFrameTexturesGlobal(SHAPEHEADER *sptr,
												POLYHEADER *pheader,
												int LTxIndex, int TxIndex);

void SpriteResizing(SHAPEHEADER *sptr);

void FindImageExtents(IMAGEHEADER *ihdr, int numuvs, int *uvdata, IMAGEEXTENTS *e, IMAGEEXTENTS *e_curr);


int GetMVSIndex(TXANIMHEADER *txah, EULER *e);


IMAGEHEADER* GetImageHeader(void);

void* GetTexture(int texindex);

TEXTURE* GetTextureMemory(int txsize);
void ReturnTextureMemory(TEXTURE *txptr);


/* Backdrops */

#if pc_backdrops
int UpdateBackdrops(SCENE Scene);
int DeallocateBackdrops(SCENE Scene);
int LoadBackdrop(char *image, IMAGEHEADER *ihdr);
#endif


void GetProjectFilename(char *fname, char *image);


void GetDOSFilename(char *fnameptr);
int CompareFilenameCH(char *string1, char *string2);

TEXTURE* LoadImageCH(char *fname, IMAGEHEADER *iheader);
TEXTURE* LoadBMP(char *fname, IMAGEHEADER *iheader);
TEXTURE* LoadPGM(char *fname, IMAGEHEADER *iheader);
int LoadPGMPalette(char *fname, unsigned char *palette);
int LoadPGMPaletteLightingTable(char *filename, unsigned char *palette);

void Create_MIP_Map(IMAGEHEADER *iheader);


int NextLowPower2(int i);


/* User Input */

void ReadUserInput(void);

void InitMouse(void);
void ReadMouse(void);



typedef struct mousedata {

	short MouseDataX;
	short MouseDataY;
	unsigned short MouseDataButton;
	short MouseDataVelX;
	short MouseDataVelY;

} MOUSEDATA;


void ReadKeyboard(void);


void WaitForReturn(void);

void CursorHome(void);

void InitialiseItemLists(void);

void InitialiseItemPointers(void);
void InitialiseItemData(void);

void* AllocateItemData(int itemsize);


int GetZForZBuffer(int z);
void FlushZBuffer(VIEWDESCRIPTORBLOCK *vdb);


/* Draw Item */

void Draw_Item_GouraudPolygon(int *itemptr);
void Draw_Item_2dTexturePolygon(int *itemptr);
void Draw_Item_Gouraud2dTexturePolygon(int *itemptr);
void Draw_Item_Gouraud3dTexturePolygon(int *itemptr);

void Draw_Item_ZB_GouraudPolygon(int *itemptr);
void Draw_Item_ZB_2dTexturePolygon(int *itemptr);
void Draw_Item_ZB_Gouraud2dTexturePolygon(int *itemptr);
void Draw_Item_ZB_Gouraud3dTexturePolygon(int *itemptr);


/*

 Texture Animation

*/

int* GetTxAnimArrayZ(int shape, int item);
TXANIMHEADER* GetTxAnimDataZ(int shape, int item, int sequence);

#if SupportBSP
TXANIMHEADER* GetTxAnimDataBSP(int shape, int node, int item, int sequence);
#endif


int GT_LL(LONGLONGCH *a, LONGLONGCH *b);
int LT_LL(LONGLONGCH *a, LONGLONGCH *b);


void SetFastRandom(void);
int FastRandom(void);


void DrawPalette(int x0, int y0, int x1, int y1);


/*

 Equates and Enums

*/

typedef enum {

	Boundary_Left,
	Boundary_Right,
	Boundary_Up,
	Boundary_Down,
	Boundary_Z

} CLIP2DBOUNDARIES;



#if SupportMorphing

void UpdateMorphing(MORPHCTRL *mcptr);
void UpdateMorphingDptr(DISPLAYBLOCK *dptr);
void GetMorphDisplay(MORPHDISPLAY *md, DISPLAYBLOCK *dptr);
void CopyMorphCtrl(MORPHCTRL *src, MORPHCTRL *dst);

VECTORCH* GetMorphedPts(DISPLAYBLOCK *dptr, MORPHDISPLAY *md);

#if LazyEvaluationForMorphing
void FreeMorphArrays(void);
#endif

#endif








/* KJL 15:07:39 01/08/97 - Returns the magnitude of the 
   cross product of two vectors a and b. */
int MagnitudeOfCrossProduct(VECTORCH *a, VECTORCH *b);

/* KJL 15:08:01 01/08/97 - sets the vector c to be the
   cross product of the vectors a and b. */
void CrossProduct(VECTORCH *a, VECTORCH *b, VECTORCH *c);

/* KJL 12:01:08 7/16/97 - returns the magnitude of a vector - max error about 13%, though average error
   less than half this. Very fast compared to other approaches. */
int Approximate3dMagnitude(VECTORCH *v);








#ifdef __cplusplus

	};

#endif




#endif
