#ifndef MODULE_INCLUDED


/*

 Modules

*/


#ifdef __cplusplus

	extern "C" {

#endif


	#if SupportModules


#include "bh_waypt.h"

typedef struct moduletableheader {

	int mth_xsize;		/* Extents in world space */
	int mth_ysize;
	int mth_zsize;

	int mth_numx;		/* Number of modules along each axis */
	int mth_numy;
	int mth_numz;

	/* Pointer to an array of pointers to modules */

	struct module **mth_moduletable;

} MODULETABLEHEADER;


/*

 NOTES

 There are no pointers to strategy and/or animation data structures yet.
 These will be added as and when they are needed.

*/

typedef enum {

	mtype_module,
	mtype_term

} MODULETYPE;


typedef union mref {

	char mref_name[4];					/* Module name */
	struct module *mref_ptr;			/* Module pointer */

} MREF;


typedef enum {

	vmtype_vmodule,
	vmtype_term

} VMODULETYPE;


typedef enum {

	vmodi_null,			/* Null instruction */
	vmodi_bra_vc		/* Branch if viewport closed */

} VMODI;

typedef union _vmodidata {

	char vmodidata_label[4];
	struct vmodule *vmodidata_ptr;
	int vmodidata;

} VMODIDATA;


typedef struct vmodule {

	VMODULETYPE vmod_type;
	char vmod_name[4];				/* Unique name for this VMODULE */
	VMODI vmod_instr;
	VMODIDATA vmod_data;
	MREF vmod_mref;
	VECTORCH vmod_dir;
	int vmod_angle;
	int vmod_flags;

} VMODULE;


#define vm_flag_gotptrs				0x00000001	/* VMODULE references have
																been converted from
																names to pointers */


#if 0

typedef enum {

	vptype_viewport,
	vptype_term

} VIEWPORTTYPE;


typedef struct viewport {

	VIEWPORTTYPE vp_type;

	int vp_flags;

	VECTORCH vp0;
	VECTORCH vp1;
	VECTORCH vp2;
	VECTORCH vp3;

} VIEWPORT;

#endif


/*

 This is the map block for module objects. It was originally based on the
 MAPBLOCK8 structure.

*/

typedef struct modulemapblock {

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


	struct displayblock **MapMPtr;	/* Write our dptr here as mother */
	struct displayblock **MapDPtr;	/* Read our dptr here as daughter */

	VECTORCH MapMOffset;					/* Offset from mother */

	#if SupportMorphing
	MORPHHEADER *MapMorphHeader;
	#endif
	
} MODULEMAPBLOCK;


/*

 Module functions called either when the module is visible or when the view
 is inside the module.

*/

typedef enum {

	mfun_null,

} MFUNCTION;


/*

 This is the MODULE structure

*/

struct aimodule;

typedef struct module {

	MODULETYPE m_type;

	char m_name[4];						/* Unique name for this MODULE */

	int m_index;							/* Unique module index */

	int m_flags;

	VECTORCH m_world;						/* World location */

	MREF m_ext;								/* Get module extents from the shape
													found through this other module */

	int m_ext_scale;						/* Scale extents by this value (fp) */

	int m_maxx;								/* Module extents */
	int m_minx;
	int m_maxy;
	int m_miny;
	int m_maxz;
	int m_minz;

	MODULEMAPBLOCK *m_mapptr;			/* Map data for the module object */
	struct displayblock *m_dptr;		/* Display block (not constant) */

	MREF m_vptr;							/* Vertical pointer to module array */

	VMODULE *m_vmptr;						/* Pointer to an array of VMODULE, or
													"visible module" structures */
	
	MREF *m_link_ptrs;					/* Pointer to an arbitrary sized array
													of module references - the array is
													zero terminated */
										/*should be got rid of  soon*/



	MODULETABLEHEADER *m_table;		/* A hash table whose creation is
													triggered by a threshold value set by
													"system.h". This is to speed up module
													list traversal */

	MFUNCTION m_ifvisible;				/* Function called if module visible */
	MFUNCTION m_ifvinside;				/* Function called if view inside */
	MREF m_funref;							/* Function access to another module */

	struct strategyblock *m_sbptr;	/* Project supplies structure */

	int m_numlights;						/* # light blocks in array */
	struct lightblock *m_lightarray;	/* Ptr. to array of light blocks */

	struct extraitemdata *m_extraitemdata;

	MATRIXCH m_mat;						/* Internal use only */
	
	char * name;

	WAYPOINT_HEADER *m_waypoints;

	struct aimodule *m_aimodule;  /* the aimodule that this module is a part of*/

	float m_sound_reverb; /*settings for the way sound should */
	int m_sound_env_index;/*be played in this module*/
	
} MODULE;


/* Flags */

#define m_flag_infinite						0x00000001	/* No extent test, the
																		view is always in this
																		module */

#define m_flag_gotptrs						0x00000002	/* Module references have
																		been converted from
																		names to pointers */

#define m_flag_open							0x00000004	/* The viewport/Door is
																		open. This state is
																		read from the "dptr"
																		morphing frame if it is
																		present and if it is
																		appropriate to do so */

#define m_flag_dormant						0x00000008	/* The module is not active */


#define m_flag_gotmat						0x00000010	/* Internal use only */

#define m_flag_visible_on_map		0x00000020	/* Flag for Kevin's map stuff */

#define m_flag_slipped_inside		0x00000040  /* Another flag 4 Kevin */

#define MODULEFLAG_AIRDUCT			0x80000000
#define MODULEFLAG_STAIRS			0x40000000
#define MODULEFLAG_SKY				0x20000000
#define MODULEFLAG_FOG				0x10000000
#define MODULEFLAG_HORIZONTALDOOR	0x08000000


typedef struct aimodule
{
	int m_index; //the index in AIModuleArray
	
	VECTORCH m_world;						/* World location */
	
	//adjacent aimodules - null terminated array
	struct aimodule **m_link_ptrs;					
	
	//the render modules that make up this ai module - null terminated array
	MODULE **m_module_ptrs;					
	
	WAYPOINT_HEADER *m_waypoints;

	/* CDF 1/6/98 - Routefinder Globals */
	int RouteFinder_FrameStamp;
	int RouteFinder_IterationNumber;

}AIMODULE;


/*
 Module Scene Structure

*/

typedef struct scenemodule {

	MODULE *sm_module;	/* Pointer to module structure for this scene */
	MODULE **sm_marray;	/* Pointer to array of pointers to all modules */

} SCENEMODULE;


/*

 "The View"

 The "View Finder" accesses the view location and orientation through this
 global structure. This is so that views can be passed to other functions as
 a single pointer if required.

*/

typedef struct aview {

	VECTORCH vloc;
	MATRIXCH vmat;
	struct viewdescriptorblock *vvdb;

} AVIEW;




/*

 Module Function Prototypes

*/

#if IncludeModuleFunctionPrototypes

void ModuleHandler(VIEWDESCRIPTORBLOCK *vdb);
void ProcessModules(VIEWDESCRIPTORBLOCK *vdb, MODULE *mptr);
void ViewFinder(MODULE *mptr);
void ReadVMODULEArrays(VMODULE *vptr);


void UpdateModules(void);
void ModuleFunctions(MODULE *mptr, MFUNCTION mf);
void AllocateModuleObject(MODULE *mptr);
void DeallocateModuleObject(MODULE *mptr);
void AllNewModuleHandler(void);


/*

 A project supplied function. The display block has been successfuly
 allocated and has been fully initialised.

*/

void ModuleObjectJustAllocated(MODULE *mptr);


/*

 A project supplied function. The display block is about to be deallocated.

*/

void ModuleObjectAboutToBeDeallocated(MODULE *mptr);


/*

 A project supplied function. These are the new and old modules this ????

*/

void NewAndOldModules(int num_new, MODULE **m_new,
								int num_old, MODULE **m_old, char *m_currvis);



#if SupportMultiCamModules
void InitGlobalVMA(void);
void DeallocateGlobalVMA(void);
#if SupportMultiCamModules
void UpdateDynamicModuleObjects(void);
#endif
#endif

void PreprocessAllModules(void);
void PreprocessModuleArray(MODULE **m_array_ptr);
void PreprocessVMODIDATA(VMODULE *v_ptr);

void DeallocateModuleVisArrays(void);
int GetModuleVisArrays(void);
int InsideModule(MODULE *mptr);

void ConvertModuleNameToPointer(MREF *mref_ptr, MODULE **m_array_ptr);
void ConvertVModuleNameToPointer(VMODIDATA *vmodidata_ptr, VMODULE *v_array_ptr);

int CompareName(char *name1, char *name2);
void PrintName(char *name);

int SaveModuleArray(MODULE *mptr, char *filename);
MODULE* LoadModuleArray(MODULE *mptr, int size, char *filename);

int IsModuleVisibleFromModule(MODULE *source, MODULE *target);
int ThisObjectIsInAModuleVisibleFromCurrentlyVisibleModules(struct strategyblock *sbPtr);

#endif	/* IncludeModuleFunctionPrototypes */

extern SCENEMODULE **Global_ModulePtr;
extern SCENEMODULE *MainSceneArray[];
extern AVIEW ModuleView;
extern MODULE *Global_MotherModule;
extern char *ModuleCurrVisArray;
extern char *ModulePrevVisArray;
extern char *ModuleTempArray;
extern char *ModuleLocalVisArray;
extern int ModuleArraySize;

extern int AIModuleArraySize;
extern AIMODULE *AIModuleArray;


#endif	/* SupportModules */


#ifdef __cplusplus

	};

#endif

#define MODULE_INCLUDED

#endif
