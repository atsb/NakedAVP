#include "3dc.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "comp_shp.h"

#define UseLocalAssert Yes

#include "ourasert.h"

extern SHAPEHEADER ** mainshapelist;

extern MAPBLOCK8 Player_and_Camera_Type8[];
extern MAPBLOCK6 Empty_Object_Type6;
extern MAPBLOCK6 Term_Type6;

extern MODULEMAPBLOCK AvpCompiledMaps[];

MAPHEADER Map[]={
    {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	&Player_and_Camera_Type8[0]
    }
};


MAPSETVDB chnk_playcam_vdb = {

	ViewDB_Flag_AdjustScale
//	| ViewDB_Flag_Hazing
	| ViewDB_Flag_AddSubject
	| ViewDB_Flag_ImageBackdrop
	| ViewDB_Flag_FullSize,

	0,
	0,

	0,
	0,

	0,
	0,
	0,

	0,
	0,
	0,
	0,

	5000,			/* Hazing in */
	30000, /*+ (ONE_FIXED << 0),	hazing end */
	0,

	/*col24(200,200,200),*/			/* Background Colour */
	/*col24(32,128,255),*/			/* Background Colour */
	0,										/* Background Colour */
	/*col24(0,0,0),*/					/* Background Colour */
	/*col24(255,0,0),*/				/* Background Colour */

	65536 >> 2 ,				/* Ambience */

	#if 0
	VState_RelativeYRemote,	/* View State */
	2/*-1*/,					/* View Distance */
	0,0,						/* Pan X,Y */
	#endif

#if 1
	VState_Inside,	/* View State */
	1,					/* View Distance */
	0,0,						/* Pan X,Y */
#endif

};




/****************************************************************************/

MAPBLOCK6 Empty_Landscape_Type6 = {


	MapType_Default,

	-1, /* No shape */

    {0,0,0},		/* Loc */
    {0,0,0},				/* Orient */

	ObFlag_MultLSrc
	/*| ObFlag_BFCRO*/
	| ObFlag_VertexHazing
	/* Hack CONSTANTINE */
	/* | ObFlag_SortFarZ */
	/*| ObFlag_RSP*/
	/*| ObFlag_NotVis*/
	/*| ObFlag_NoColls*/,		/* Flags */
	
	#if StandardStrategyAndCollisions
	StrategyI_Null,	/* Strategy */
	CollType_Landscape,/* e.g. Shape or Landscape */
	GameCollStrat_Default,		/* Game Collision Strategy */
	0,						/* Shape Collision Strategy */
	0,						/* Landscape Collision Strategy */
	#endif

	0,									/* VDB Definition */
	0									/* Interior Type */

};



MAPBLOCK6 Empty_Object_Type6 = {


	MapType_Default,

	-1,	/* No shape */

    {0,0,0},		/* Loc */
    {0,0,0},				/* Orient */

	ObFlag_MultLSrc |
	/*| ObFlag_BFCRO*/
	ObFlag_VertexHazing	|
	/* hack CONSTANTINE */
	/* ObFlag_SortFarZ | */
	/*| ObFlag_RSP*/
	/*| ObFlag_NotVis*/
	/*| ObFlag_NoColls*/
	0,		/* Flags1 (no others in this map type)*/
	
	#if StandardStrategyAndCollisions
	StrategyI_Null,	/* Strategy */
	0,/* e.g. Shape or Landscape */
	GameCollStrat_Default,	/* Game Collision Strategy */
	0,						/* Shape Collision Strategy */
	0,						/* Landscape Collision Strategy */
	#endif
	0,									/* VDB Definition */
	0									/* Interior Type */

};


MAPBLOCK6 Term_Type6 = {

/* map end */

	MapType_Term,
	0,
    {0,0,0},
    {0,0,0},
	0,						/* Flags */
	
	#if StandardStrategyAndCollisions
	0,						/* Strategy */
	0,						/* e.g. Shape or Landscape */
	0,						/* Game Collision Strategy */
	0,						/* Shape Collision Strategy */
	0,						/* Landscape Collision Strategy */
	#endif
	0,						/* VDB Definition */
	0,						/* Interior Type */

};

/****************************************************************************/


MAPBLOCK8 Player_and_Camera_Type8[] = {
#if SupportModules
	/*  Ship */
  {					 
	MapType_Player,

	I_ShapeMarinePlayer, 	/*mainshapelist position 0*/


	/* MapType_Camera, */

    {0, 0, 0},							/* Loc */
    {0, 3062, 0},						/* Orient */

	ObFlag_MultLSrc
	|0
	|0,
															/* Flags 2 */
	0
	/*	| ObFlag2_DirectedMerge	 */			
	/*	| ObFlag2_SurfaceAlign*/
	/*	| ObFlag2_Augmented_LFR*/
	/*	| ObFlag2_LFR_GuessReloc*/
	/*	| ObFlag2_LFR_UseFRM*/
	/*	| ObFlag2_FRM_VariableR */
	/*	| ObFlag2_NoMotionLerp*/
	/*| ObFlag2_NoMerge*/
	/*| ObFlag2_NoMergeDIfNear*/
	/*| ObFlag2_NoMergeD*/
	|0,
	
	0
	| 0,											/* Flags 3 */
	
	#if StandardStrategyAndCollisions
	StrategyI_Player,						/* Strategy */
	0,											/* e.g. Shape or Landscape */
	GameCollStrat_Default,				/* Game Collision Strategy */
	ShapeCStrat_DoubleExtentEllipsoid,	/* Shape Collision Strategy */
	0,											/* Landscape Collision Strategy */
	#endif
	
	&chnk_playcam_vdb,					/* VDB Definition */
	IType_Body,											/* Interior Type */
									 
	0,											/* MapLightType */
	#if StandardStrategyAndCollisions
	0,											/* MapMass */
	0,0,0,									/* MapNewtonV */
	#endif
    {0,0,0},									/* MapOrigin */
					   
	0,											/* MapSimShapes */
	0,											/* MapViewType */

	0,					/* MapMPtr */

	0,											/* MapDPtr */
    {0,0,0}									/* MapMOffset */
  },

/****************************************************************************/

	#if 0
	/* Player Ship Camera - MUST be next map object */

	MapType_PlayerShipCamera,

	I_ShapeCube, /* mainshapelist position 1 */

	0,0,0,							/* Loc */
	0,0,0,							/* Orient */
	0
	|0					/* Flags 1 */
	| 0,

	0,									/* Flags 2 */
	0,									/* Flags 3 */
	
	#if StandardStrategyAndCollisions
	StrategyI_Camera,				/* Strategy */
	0,									/* e.g. Shape or Landscape */
	GameCollStrat_Default,		/* Game Collision Strategy */
	0,									/* Shape Collision Strategy */
	0,									/* Landscape Collision Strategy */
	#endif
	
	&chnk_playcam_vdb,					/* VDB Definition */

	#if 0

		IType_Default,					/* Interior Type */

	#else

		IType_Body,						/* Interior Type */

	#endif

	0,											/* MapLightType */
	
	#if StandardStrategyAndCollisions
	0,											/* MapMass */
	0,0,0,									/* MapNewtonV */
	#endif
	0,0,0,									/* MapOrigin */
	0,											/* MapSimShapes */
	0,											/* MapViewType */

	0,											/* MapMPtr */
	0,											/* MapDPtr */
	0,0,0,									/* MapMOffset */
	#endif

/****************************************************************************/
/****************************************************************************/
	/* Map End */
#endif /*SupportModules*/
  {
	MapType_Term,		/* Map Type Function */
	0,						/* Shape */
    {0,0,0},				/* Loc */
    {0,0,0},				/* Orient */
	0,						/* Flags 1 */
	0,						/* Flags 2 */
	0,						/* Flags 3 */
	#if StandardStrategyAndCollisions
	0,						/* Strategy */
	0,						/* e.g. Shape or Landscape */
	0,						/* Game Collision Strategy */
	0,						/* Shape Collision Strategy */
	0,						/* Landscape Collision Strategy */
	#endif
	0,						/* VDB Definition */
	0,						/* Interior Type */
	0,						/* MapLightType */
	#if StandardStrategyAndCollisions
	0,						/* MapMass */
	0,0,0,				/* MapNewtonV */
	#endif
    {0,0,0},				/* MapOrigin */
	0,						/* MapSimShapes */
	0,						/* MapViewType */
	0,						/* MapMPtr */
	0,						/* MapDPtr */
    {0,0,0}				/* MapMOffset */
  }
};



/* ******************************************************************************** */


/*--------------**
** Module stuff **
**--------------*/

SCENEMODULE MainScene;

SCENEMODULE * MainSceneArray[] = 
{
	&MainScene,
	0
};

/* these are effectively mapblock8*/

extern MODULEMAPBLOCK AvpCompiledMaps[];

MODULEMAPBLOCK AvpCompiledMaps[] = {
/****************************************************************************/
/****************************************************************************/
	/* Map End */
	{
	MapType_Term,		/* Map Type Function */
	0,						/* Shape */
    {0,0,0},				/* Loc */
    {0,0,0},				/* Orient */
	0,						/* Flags 1 */
	0,						/* Flags 2 */
	0,						/* Flags 3 */
	#if StandardStrategyAndCollisions
	0,						/* Strategy */
	0,						/* e.g. Shape or Landscape */
	0,						/* Game Collision Strategy */
	0,						/* Shape Collision Strategy */
	0,						/* Landscape Collision Strategy */
	#endif
	0,						/* VDB Definition */
	0,						/* Interior Type */
	0,						/* MapLightType */
	#if StandardStrategyAndCollisions
	0,						/* MapMass */
	0,0,0,				/* MapNewtonV */
	#endif
    {0,0,0},				/* MapOrigin */
	0,						/* MapSimShapes */
	0,						/* MapViewType */
	0,						/* MapMPtr */
	0,						/* MapDPtr */
    {0,0,0},				/* MapMOffset */
	}
};



MODULE Empty_Module = {

	mtype_module,							/* MODULETYPE m_type */
	"null",									/* char m_name[] */
	0,											/* int m_index */
	0,											/* int m_flags */
    {0,0,0},									/* VECTOR m_world */
    {"null"},									/* MREF m_ext */
	0,											/* int m_ext_scale */
	0,											/* int m_maxx */
	0,											/* int m_minx */
	0,											/* int m_maxy */
	0,											/* int m_miny */
	0,											/* int m_maxz */
	0,											/* int m_minz */
	0,											/* MODULEMAPBLOCK *m_mapptr */
	0,											/* struct displayblock *m_dptr */
    {"null"},									/* MREF m_vptr */
	0,											/* VMODULE *m_v_ptrs */
	0,											/* struct module **m_link_ptrs */
//	0,											/* VIEWPORT *m_viewports */
	0,											/* MODULETABLEHEADER *m_table */
	mfun_null,								/* MFUNCTION m_ifvisible */
	mfun_null,								/* MFUNCTION m_ifvinside */
    {"null"},									/* MREF m_funref */
	0,											/* Strategy block * */
	0,											/* num lights*/
	NULL,										/* pointer to light blocks*/
	NULL,										/* pointer to extraitemdata */
	
    {0,0,0,
	0,0,0,
	0,0,0},
	
	0,
};


MODULE Term_Module = {

	mtype_term								/* MODULETYPE m_type */

};


// this is the one used for the loaded modules

MODULEMAPBLOCK Empty_Module_Map = 
{

	MapType_Default,                   /* MapType */
	-1,                                /* MapShape */
    {0, 0, 0},                           /* MapWorld */
    {0, 0, 0},                           /* MapEuler */                                             /* Orient */

	0
	| ObFlag_MultLSrc                       /* MapFlags */
	| ObFlag_NoInfLSrc
	//| ObFlag_BFCRO
	//| ObFlag_VertexHazing
	| 0,

	0
	//| ObFlag2_SortD
	| 0,                               /* Flags 2 */

	0
	| ObFlag3_ObjectSortedItems
	| ObFlag3_NoLightDot
	| ObFlag3_PreLit
	| 0,                               /* Flags 3 */

	0,                                 /* VDB Definition */
	0,                                 /* Interior Type */

	LightType_PerVertex,               /* MapLightType */
    {0,0,0},                           /* MapOrigin */
	0,                                 /* MapSimShapes */
	0,                                 /* MapViewType */

	0,                                 /* MapMPtr */
	0,                                 /* MapDPtr */
    {0,0,0},                           /* MapMOffset */

};

// !!!!!!!!!!!!

// Default ModuleMapBlock defined below; this will be used for
// run-time creation of both temporary, local, and global objects;
// for the most part, these objects will have no collisions, comprising
// sprite-based explosions, vertex-based explosion fragments, missiles,
// grenades, etc. They might also include "placeable" objects, like
// deposited inventory items, ammo, medpacks, etc.

// !!!!!!!!!!!!

MODULEMAPBLOCK TempModuleMap = \
{

	MapType_Default,                   /* MapType */
	-1,                                /* MapShape */

    {0, 0, 0},                         /* MapWorld */
    {0, 0, 0},                         /* MapEuler */                                             /* Orient */

	0																	/* flags 1*/
	| ObFlag_NoInfLSrc
	| ObFlag_MultLSrc 			
	|0,
																		/* Flags 2 */
	0,
	                             	
	0
//	| ObFlag3_NoLightDot
	| 0,															/* Flags 3 */

	0,                                /* VDB Definition */
	0,                                /* Interior Type */

	LightType_PerVertex,               /* MapLightType */

    {0,0,0},                           /* MapOrigin */
	0,                                 /* MapSimShapes */
	0,                                 /* MapViewType */
	0,                                 /* MapMPtr */
	0,                                 /* MapDPtr */
    {0,0,0}                            /* MapMOffset */

};

