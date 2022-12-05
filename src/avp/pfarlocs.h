
/*------------------------Patrick 13/12/96-----------------------------
  Header file for FAR AI alien module locations
  --------------------------------------------------------------------*/

#ifndef _pfarlocs_h_
	#define _pfarlocs_h_ 1

	#ifdef __cplusplus
		extern "C" {
	#endif


/* various structure definitions used for module's auxilary location and
entry point data and headers...*/
typedef struct farlocationsheader
{
	int	numLocations;
	struct vectorch *locationsList;

} FARLOCATIONSHEADER;

typedef struct farvalidatedlocation
{
	int	valid;
	struct vectorch position;

} FARVALIDATEDLOCATION;

typedef struct farentrypoint
{
	struct vectorch position;
	int donorIndex;
	unsigned int alien_only:1; //this entry point can only be used by aliens

} FARENTRYPOINT;

typedef struct farentrypointsheader
{
	int	numEntryPoints;
	struct farentrypoint *entryPointsList;

} FARENTRYPOINTSHEADER;


/* enum of the different door types, as seen by NPC's */
typedef enum moduledoortype
{
	MDT_NotADoor,			
	MDT_ProxDoor,			
	MDT_LiftDoor,
	MDT_SecurityDoor,

} MODULEDOORTYPE;


/* globals */
extern FARLOCATIONSHEADER *FALLP_AuxLocs;
extern FARENTRYPOINTSHEADER *FALLP_EntryPoints;

/* defines for auxilary locations */
#define FAR_BB_HEIGHT	2000 /* should be height of a crouched alien */
#define FAR_BB_WIDTH	1000 /* should be the 'width' of an alien */
#define FAR_POS_HEIGHT	680  /* how high of the floor to put the alien (1/2 alien height + a bit) */
#define FAR_GRID_SIZE	6
#define FAR_MAX_LOCS	5
#define FAR_MIN_INCLINE	50

/* defines for entry points */
#define	EPBB_XTRA 		100
#define	EP_POSNDISP		100
#define EP_MAXPOINTS	200
#define EP_MAXEDGES		200

/* defines for module door types insofar as they relate to alien behaviour 
FADT stands for Far Alien Door Type*/


/* prototypes */
void BuildFarModuleLocs(void);
void KillFarModuleLocs(void);
MODULEDOORTYPE ModuleIsADoor(MODULE* target);
MODULEDOORTYPE AIModuleIsADoor(AIMODULE* target);
int ModuleIsPhysical(MODULE* target);
int AIModuleIsPhysical(AIMODULE* target);
int ModuleInModule(MODULE* target1, MODULE* target2);
int NumAdjacentModules(AIMODULE* target);
FARENTRYPOINT *GetModuleEP(MODULE* thisModule, MODULE*fromModule);
FARENTRYPOINT *GetAIModuleEP(AIMODULE* thisModule, AIMODULE*fromModule);
int PointIsInModule(MODULE* thisModule, VECTORCH* thisPoint);


	#ifdef __cplusplus
	}
	#endif

#endif
