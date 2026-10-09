/*------------------------Patrick 13/12/96-----------------------------
  Source file for FAR AI alien module locations, as used by far alien
  behaviours....
  --------------------------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"

#include "dynblock.h"
#include "dynamics.h"

#include "pheromon.h"
#include "bh_alien.h"
#include "bh_far.h"
#include "pfarlocs.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/* prototypes for this file */
//static void BuildFM_EntryPoints(MODULE *thisModule);
//static void BuildFM_ASingleEP(MODULE *thisModule, MODULE *targetModule);

static void BuildFM_AuxilaryLocs(MODULE *thisModule);
static void GetFarLocHeight(FARVALIDATEDLOCATION *location, MODULE *thisModule);
static void FarLocVolumeTest(FARVALIDATEDLOCATION *location, MODULE *thisModule);
static int IsXZinPoly(VECTORCH* location, struct ColPolyTag *polygonData);
static void InitFarLocDataAreas(MODULE **moduleList, int numModules);

/* external global variables used in this file */
extern int ModuleArraySize;

/* prototypes for external functions */
int SetupPolygonAccessFromShapeIndex(int shapeIndex);
int SetupPointAccessFromShapeIndex(int shapeIndex);
VECTORCH* AccessNextPoint(void);
VECTORCH* AccessPointFromIndex(int index);
int *GetPolygonVertexIndices(void);

/* globals for this file */
FARENTRYPOINTSHEADER *FALLP_EntryPoints=(FARENTRYPOINTSHEADER*)0;
FARLOCATIONSHEADER *FALLP_AuxLocs = (FARLOCATIONSHEADER *)0;

int AIModuleArraySize=0;
AIMODULE *AIModuleArray=0;

/* static globals for this file */
static FARVALIDATEDLOCATION *auxLocsGrid;
static int numInfiniteModules = 0;

static int FL_TotalNumAuxLocs = 0;
static VECTORCH	*FL_AuxData = (VECTORCH *)0;

/* a define for logging location data */
#define logFarLocData	0
#if logFarLocData
static FILE *logfile;
#endif

#define logFarLocPositions	0
#if logFarLocPositions
static FILE *logfile2;
#endif

/*----------------------Patrick 16/12/96------------------------
  This function builds a list of entry points and auxilary
  locations for	each module.

   
  NB this cannot be called until the module system has been 
  initialised for this environment.
----------------------------------------------------------------*/

void BuildFarModuleLocs(void)
{
	MODULE **moduleListPointer;	
	int moduleCounter;

	/* don't do this for a net game */
	//in fact do do it in net game
	#if 0
	if(AvP.Network != I_No_Network)	
	{
		/* Make sure global data is zeroed, then return */
		FALLP_AuxLocs = (FARLOCATIONSHEADER *)0;
		FL_TotalNumAuxLocs = 0;
		FL_AuxData = (VECTORCH *)0;
		return;
	}
	#endif

	LOCALASSERT(ModuleArraySize);

	/* now get a pointer to the list of modules in this environment. */ 
	{
		extern SCENE Global_Scene;
		extern SCENEMODULE **Global_ModulePtr;
		SCENEMODULE *ScenePtr;

		LOCALASSERT(Global_ModulePtr);
		
		ScenePtr = Global_ModulePtr[Global_Scene];
		moduleListPointer = ScenePtr->sm_marray;
	}
		
	#if logFarLocData
	logfile = fopen("E:/3DC/FARLOCS.TXT","w");
	fprintf(logfile, "MODULE FAR LOCATIONS DATA \n");
	fprintf(logfile, "ENV: %s \n", Env_List[AvP.CurrentEnv]->main);
	fprintf(logfile, "************************* \n");
	fprintf(logfile, "number of modules: %d \n", ModuleArraySize);
	fprintf(logfile, "grid size: %d \n", FAR_GRID_SIZE);
	fprintf(logfile, "max locs stored per module: %d \n \n", FAR_MAX_LOCS);
	fprintf(logfile, "alien box height: %d \n", FAR_BB_HEIGHT);
	fprintf(logfile, "alien box width: %d \n", FAR_BB_WIDTH);
	fprintf(logfile, "************************* \n \n");	
	#endif

	/* initialise infinite module counter */
	numInfiniteModules = 0;

	/* allocate some temporary work spaces..*/
	auxLocsGrid = (FARVALIDATEDLOCATION *)AllocateMem(FAR_GRID_SIZE*FAR_GRID_SIZE*sizeof(FARVALIDATEDLOCATION));
	if(!auxLocsGrid) 
	{
		memoryInitialisationFailure = 1;
		return;
	}

	/* Initialise the entry point list and auxilary location list.
	NB entry points are are pre-allocated, since they are evaluated in pairs.*/
	InitFarLocDataAreas(moduleListPointer, ModuleArraySize);
	
	#if logFarLocData
	fprintf(logfile, "********************************* \n");
	fprintf(logfile, "STARTING ENTRY POINTS.... \n");
	fprintf(logfile, "********************************* \n \n");
	#endif

	/* Now go through the module list, and calculate entry points. This step should
	be done before auxilary locations, to be absolutely sure everything works out */
	#if 0
	//set up in projload now
	for(moduleCounter = 0; moduleCounter < ModuleArraySize; moduleCounter++)
	{
		MODULE *thisModule;
		int ThisModuleIndex;	
	 		
	 	/* get a pointer to the next module, and it's index */
		thisModule = moduleListPointer[moduleCounter]; 
		LOCALASSERT(thisModule);
		ThisModuleIndex = thisModule->m_index;
		LOCALASSERT(ThisModuleIndex >= 0);
		LOCALASSERT(ThisModuleIndex < ModuleArraySize);
		
		#if logFarLocData
		fprintf(logfile, "********************************* \n");
		fprintf(logfile, "Module Index: %d %s\n", ThisModuleIndex,thisModule->name);
 		#endif

		/* reject modules that are not physical */
		if(ModuleIsPhysical(thisModule)) 
			BuildFM_EntryPoints(thisModule);
		else
		{
			numInfiniteModules++;
			#if logFarLocData
			fprintf(logfile, "NO EPS COMPUTED: NOT PHYSICAL\n \n");
			#endif
		}
	}
	#endif

	#if logFarLocData
	fprintf(logfile, "********************************* \n");
	fprintf(logfile, "STARTING AUXILARY LOCATIONS.... \n");
	fprintf(logfile, "********************************* \n \n");
	#endif

	/* now go thro' each module calculating auxilary locations */
	for(moduleCounter = 0; moduleCounter < ModuleArraySize; moduleCounter++)
	{
		MODULE *thisModule;
		int ThisModuleIndex;	
	 		
	 	/* get a pointer to the next module, and it's index */
		thisModule = moduleListPointer[moduleCounter]; 
		LOCALASSERT(thisModule);
		ThisModuleIndex = thisModule->m_index;
		LOCALASSERT(ThisModuleIndex >= 0);
		LOCALASSERT(ThisModuleIndex < ModuleArraySize);
		
		#if logFarLocData
		fprintf(logfile, "********************************* \n");
		fprintf(logfile, "Module Index: %d \n", ThisModuleIndex);
		fprintf(logfile, "Module X range: %d %d \n", thisModule->m_minx, thisModule->m_maxx);
		fprintf(logfile, "Module Z range: %d %d \n \n", thisModule->m_minz, thisModule->m_maxz);
		#endif
		
		/* check for entry points into this module if there	aren't any,
		don't bother with auxilary locations */

		if(thisModule->m_aimodule)
			BuildFM_AuxilaryLocs(thisModule);
		/*
		else
		{	
			#if logFarLocData
			fprintf(logfile, "NO AUXILARY LOCS COMPUTED: NO EPS \n \n");
			#endif
		} 
		*/ 
	}

	/* deallocate the temporary work spaces */
	if (auxLocsGrid) DeallocateMem(auxLocsGrid);

	#if logFarLocData
	fprintf(logfile, "************************************* \n");
	fprintf(logfile, "FINISHED ! \n");
	fprintf(logfile, "NUM INFINITE MODULES/TERMINATORS: %d \n", numInfiniteModules);
	fprintf(logfile, "************************************* \n");
	fclose(logfile);
	#endif

	#if logFarLocPositions && 0 //this log will need to updated for the ai modules.
	logfile2 = fopen("D:/PATRICK/FARLOCS2.TXT","w");
	fprintf(logfile2, "MODULE EPs AND AUXILARY LOCATIONS \n");
	fprintf(logfile2, "ENV: %s \n", Env_List[AvP.CurrentEnv]->main);
	fprintf(logfile2, "************************* \n \n");

	for(moduleCounter = 0; moduleCounter < ModuleArraySize; moduleCounter++)
	{
		int i;
		fprintf(logfile2, "MODULE INDEX: %d \n",moduleCounter);
		fprintf(logfile2, "  EPs: \n");
		for(i=0;i<FALLP_EntryPoints[moduleCounter].numEntryPoints;i++)
		{
			VECTORCH posn;
			int index;
			posn = (FALLP_EntryPoints[moduleCounter].entryPointsList)[i].position;
			index = (FALLP_EntryPoints[moduleCounter].entryPointsList)[i].donorIndex; 
			fprintf(logfile2, "  %d %d %d FROM %d \n", posn.vx, posn.vy, posn.vz, index);
		}
		fprintf(logfile2, "  AUX: \n");
		for(i=0;i<FALLP_AuxLocs[moduleCounter].numLocations;i++)
		{
			VECTORCH posn;
			posn = (FALLP_AuxLocs[moduleCounter].locationsList)[i];
			fprintf(logfile2, "  %d %d %d \n", posn.vx, posn.vy, posn.vz);
		}
		fprintf(logfile2, "\n");
	}
	fprintf(logfile2, "************************* \n");
	fprintf(logfile2, "END \n ");
	fclose(logfile2);
	#endif
}

/* allocates and initialises primitive data areas */
static void InitFarLocDataAreas(MODULE **moduleList, int numModules)
{
	int moduleCounter;

	/* first, the lists of data headers */	
	LOCALASSERT(numModules>0);
	FALLP_AuxLocs = (FARLOCATIONSHEADER *)AllocateMem(numModules*sizeof(FARLOCATIONSHEADER));
	if(!FALLP_AuxLocs) 
	{
		memoryInitialisationFailure = 1;
		return;
	}


	FL_TotalNumAuxLocs = 0;
	FL_AuxData = (VECTORCH *)0;
		
	/* work out the number of adjacent modules/auxilary locations for each module, 
	and add them up */
	for(moduleCounter=0;moduleCounter<numModules;moduleCounter++)
	{
		MODULE *thisModule; 
		int thisModuleIndex;

		thisModule = moduleList[moduleCounter]; 
		LOCALASSERT(thisModule);
		thisModuleIndex = thisModule->m_index;
		LOCALASSERT(thisModuleIndex >= 0);
		LOCALASSERT(thisModuleIndex < ModuleArraySize);
		
		if(ModuleIsPhysical(thisModule))
		{
			FL_TotalNumAuxLocs += FAR_MAX_LOCS;
		}	
	}

	/* allocate base data areas */
	LOCALASSERT(FL_TotalNumAuxLocs>0);
	FL_AuxData = AllocateMem(FL_TotalNumAuxLocs*sizeof(VECTORCH));;
	if(!FL_AuxData) 
	{
		memoryInitialisationFailure = 1;
		return;
	}

	/* init header lists */
	{
		VECTORCH *auxDataPtr = FL_AuxData;
		
		for(moduleCounter=0;moduleCounter<AIModuleArraySize;moduleCounter++)
		{
			AIMODULE* thisAIModule = &AIModuleArray[moduleCounter]; 
			int thisModuleIndex = thisAIModule->m_index;
			/* NB these pointers and indexes have been validated above */

			FALLP_AuxLocs[thisModuleIndex].numLocations = 0;
			FALLP_AuxLocs[thisModuleIndex].locationsList = (VECTORCH *)0;

			{
				MODULE** modulearray=thisAIModule->m_module_ptrs;
				if(modulearray)
				{
					FALLP_AuxLocs[thisModuleIndex].locationsList = auxDataPtr;
					while(*modulearray)
					{
						modulearray++;
						auxDataPtr += FAR_MAX_LOCS;

					}
				}
			}
		}

		/* we can now validate this process... */
		LOCALASSERT(auxDataPtr==(FL_AuxData+FL_TotalNumAuxLocs));	
	}
}


/*-----------------------Patrick 28/11/96---------------------------
This function deallocates the location lists for each module,
and must be called at some point before the environment re-load
-------------------------------------------------------------------*/
void KillFarModuleLocs(void)
{	
	/* don't do this for a net game */
	//in fact do do it in net game
	//if(AvP.Network != I_No_Network)	return;
	
	LOCALASSERT(ModuleArraySize);
	LOCALASSERT(AIModuleArraySize);
	LOCALASSERT(FALLP_EntryPoints);
	LOCALASSERT(FALLP_AuxLocs);
	LOCALASSERT(FL_TotalNumAuxLocs>0);
	LOCALASSERT(FL_AuxData);

	/* deallocate the base data area in one go, and re-init globals */
	if (FL_AuxData) DeallocateMem(FL_AuxData);
	FL_TotalNumAuxLocs = 0;
	FL_AuxData = (VECTORCH *)0;

	/* deallocate the list headers, and re-init globals */
	if (FALLP_AuxLocs) DeallocateMem(FALLP_AuxLocs);
	FALLP_AuxLocs = (FARLOCATIONSHEADER *)0;
	if (FALLP_EntryPoints) 
	{
		int i;
		for(i=0;i<AIModuleArraySize;i++)
		{
			if(FALLP_EntryPoints[i].entryPointsList)
			{
				DeallocateMem(FALLP_EntryPoints[i].entryPointsList);
			}
		}
		DeallocateMem(FALLP_EntryPoints);
	}
	FALLP_EntryPoints = (FARENTRYPOINTSHEADER *)0;
}



/*-----------------------Patrick 20/12/96---------------------------
THE FOLLOWING ARE SOME GENERIC FUNCTIONS FOR ANALYSING MODULE
STATES. THEY MAY BE USED IN ANY SOURCE FILE.
-------------------------------------------------------------------*/

/*-----------------------Patrick 20/12/96---------------------------
This function takes a module and returns whether it is a door,
and if so, what kind of door it is from an alien perspective.
-------------------------------------------------------------------*/
MODULEDOORTYPE ModuleIsADoor(MODULE* target)
{
	if((target->m_sbptr) && (target->m_sbptr->I_SBtype == I_BehaviourProximityDoor))
		return MDT_ProxDoor;
	if((target->m_sbptr) && (target->m_sbptr->I_SBtype == I_BehaviourLiftDoor))	
		return MDT_LiftDoor;
	if((target->m_sbptr) && (target->m_sbptr->I_SBtype == I_BehaviourSwitchDoor))	
		return MDT_SecurityDoor;

	return MDT_NotADoor;
}

MODULEDOORTYPE AIModuleIsADoor(AIMODULE* target)
{
	/* A bit rough and ready at this point. */
	int a;
	MODULE **renderModule;

	GLOBALASSERT(target->m_module_ptrs);

	renderModule=target->m_module_ptrs;
	a=0;

	while ((*renderModule)!=NULL) {

		if(((*renderModule)->m_sbptr) && ((*renderModule)->m_sbptr->I_SBtype == I_BehaviourProximityDoor))
			return MDT_ProxDoor;
		if(((*renderModule)->m_sbptr) && ((*renderModule)->m_sbptr->I_SBtype == I_BehaviourLiftDoor))	
			return MDT_LiftDoor;
		if(((*renderModule)->m_sbptr) && ((*renderModule)->m_sbptr->I_SBtype == I_BehaviourSwitchDoor))	
			return MDT_SecurityDoor;

		GLOBALASSERT(a<300);

		renderModule++;
		a++;
	}

	return MDT_NotADoor;
}

/*-----------------------Patrick 20/12/96---------------------------
Returns TRUE if a module is a physical part of the environment,
ie is not infinite or a terminator, etc...
-------------------------------------------------------------------*/
int ModuleIsPhysical(MODULE* target)
{
	if(target->m_flags & m_flag_infinite) return 0;
	if(target->m_type == mtype_term) return 0;
	return 1;
}

int AIModuleIsPhysical(AIMODULE* target)
{
	if (target==NULL) {
		return(0);
	}
	GLOBALASSERT(target);
	if(target->m_module_ptrs==NULL) return 0;
	return 1;
}

/*-----------------------Patrick 20/12/96---------------------------
Takes 2 modules, and returns TRUE if the centre of the target module
is within the bounding box of the source;
-------------------------------------------------------------------*/
int ModuleInModule(MODULE* source, MODULE* target)
{
	
	if(target->m_world.vx < (source->m_world.vx + source->m_minx)) return 0;
	if(target->m_world.vx > (source->m_world.vx + source->m_maxx)) return 0;

	if(target->m_world.vy < (source->m_world.vy + source->m_miny)) return 0;
	if(target->m_world.vy > (source->m_world.vy + source->m_maxy)) return 0;

	if(target->m_world.vz < (source->m_world.vz + source->m_minz)) return 0;
	if(target->m_world.vz > (source->m_world.vz + source->m_maxz)) return 0;

	return 1;
}

/*-----------------------Patrick 20/12/96---------------------------
Returns the number of entries in a given module's adjacency list
If there is no adjacency list, return 0.
-------------------------------------------------------------------*/
int NumAdjacentModules(AIMODULE* target)
{
	AIMODULE **AdjAIModulePtr;
	int counter = 0;
	
	AdjAIModulePtr = (target->m_link_ptrs);
	if(AdjAIModulePtr)	
	{
		while(*AdjAIModulePtr)
		{
			counter++;
			AdjAIModulePtr++;
		}
	}
	else return 0;

	return counter;
}

/*-----------------------Patrick 20/12/96---------------------------
Returns a pointer to an entry point in thisModule from fromModule,
or 0 if there isn't one.
-------------------------------------------------------------------*/
FARENTRYPOINT *GetModuleEP(MODULE* thisModule, MODULE*fromModule)
{
	int numEps;
	FARENTRYPOINT *epList;
	FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
	int tmIndex = fromModule->m_index;

	numEps = FALLP_EntryPoints[thisModule->m_index].numEntryPoints;
	epList = FALLP_EntryPoints[thisModule->m_index].entryPointsList;

	while((numEps>0) && (thisEp == (FARENTRYPOINT *)0))
	{
		if(epList->donorIndex == tmIndex) thisEp = epList;
		epList++;
		numEps--;
	}
	
	return thisEp;
}

FARENTRYPOINT *GetAIModuleEP(AIMODULE* thisModule, AIMODULE*fromModule)
{
	int numEps;
	FARENTRYPOINT *epList;
	FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;
	int tmIndex = fromModule->m_index;

	numEps = FALLP_EntryPoints[thisModule->m_index].numEntryPoints;
	epList = FALLP_EntryPoints[thisModule->m_index].entryPointsList;

	while((numEps>0) && (thisEp == (FARENTRYPOINT *)0))
	{
		if(epList->donorIndex == tmIndex) thisEp = epList;
		epList++;
		numEps--;
	}
	
	return thisEp;
}

/*-----------------------Patrick 20/12/96---------------------------
Returns 1 if the given point is in a given module, or 0 otherwise.
The point is in module LOCAL space.

NB the bizzare structure of this function produces optimum pentium
instructions... apparently.
-------------------------------------------------------------------*/
int PointIsInModule(MODULE* thisModule, VECTORCH* thisPoint)
{		
	/*
	Allow tolerance level on module boundaries equivavlent to that used by
	ModuleFromPosition_WithToleranace.
	THis may be a bad plan , but I think this function only really gets used 
	for the purposes of asserts.
	*/
	if(thisPoint->vx <= thisModule->m_maxx + 50)
		if(thisPoint->vx >= thisModule->m_minx - 50)
			if(thisPoint->vy <= thisModule->m_maxy + 50)
				if(thisPoint->vy >= thisModule->m_miny - 50)
					if(thisPoint->vz <= thisModule->m_maxz + 50)
						if(thisPoint->vz >= thisModule->m_minz - 50)
							return 1;	
	return 0;
}





/*-----------------------Patrick 20/12/96---------------------------
LOCAL FUNCTIONS FOR MODULE ENTRY POINT SUPPORT
-------------------------------------------------------------------*/

/* a structure and globals for the entry point calculations */
typedef struct epbbextents
{
	int	maxX;
	int	minX;
	int	maxY;
	int	minY;
	int	maxZ;
	int	minZ;
} EPBBEXTENTS;

#if 0
static EPBBEXTENTS MI_Volume1;
static EPBBEXTENTS MI_Volume2;
static EPBBEXTENTS MI_Volume3;

static int GetModulesIntersection(MODULE *thisModule, MODULE *targetModule);
static int GetModulePointBox(MODULE *thisModule, EPBBEXTENTS *extents);
static void AddModuleEP(MODULE* thisModule, MODULE*fromModule, VECTORCH *posn);
#endif

/*-----------------------Patrick 16/12/96---------------------------
This Function checks if a module has any adjacent modules, and if
so calls BuildFM_ASingleEntryEp for each one.
-------------------------------------------------------------------*/
#if 0
static void BuildFM_EntryPoints(MODULE *thisModule)
{
	int ThisModuleIndex;
	MREF *AdjModuleRefPtr;
		
	LOCALASSERT(thisModule);
	ThisModuleIndex = thisModule->m_index;
	LOCALASSERT(ThisModuleIndex >= 0);
	LOCALASSERT(ThisModuleIndex < ModuleArraySize);
		
	/* check that there is a list of adjacent modules */
	if(!(NumAdjacentModules(thisModule))) 
	{
		#if logFarLocData
		fprintf(logfile, "No adjacent modules found for this module \n");
		#endif
		return;
	}
	
	AdjModuleRefPtr = thisModule->m_link_ptrs;
	
	/* go through each adjacent module */			
	while(AdjModuleRefPtr->mref_ptr != 0)
	{
		BuildFM_ASingleEP(thisModule, ((MODULE* )AdjModuleRefPtr->mref_ptr));
		AdjModuleRefPtr++;
	}			
}
#endif

/*-----------------------Patrick 16/12/96---------------------------
This Function builds a single entry point pair between two modules.
-------------------------------------------------------------------*/
#if 0
static void BuildFM_ASingleEP(MODULE *thisModule, MODULE *targetModule)
{
	VECTORCH thisModuleEP; 
	VECTORCH targetModuleEP;
	int numpts1, numpts2;

	/* containment check */
	if(ModuleInModule(thisModule, targetModule))
	{
		#if logFarLocData
		fprintf(logfile, "Against module %d: containment failure \n",targetModule->m_index);
		#endif
		targetModule->m_flags |= m_flag_slipped_inside;
		return;
	}
	if(ModuleInModule(targetModule, thisModule))
	{
		#if logFarLocData
		fprintf(logfile, "Against module %d: containment failure \n",targetModule->m_index);
		#endif
		thisModule->m_flags |= m_flag_slipped_inside;
		return;
	}
	
	/* check if entry point already exists... */
	if(GetModuleEP(thisModule, targetModule) != (FARENTRYPOINT *)0)
	{
		#if logFarLocData
		fprintf(logfile, "Against module %d: ep already exists \n",targetModule->m_index);
		#endif
		return;	
	}

	/* at this stage we know that thisModule is physical....
	but it's a good idea to check the target module */
	if (!(ModuleIsPhysical(targetModule)))
	{
		#if logFarLocData
		fprintf(logfile, "Against module %d: target is not physical \n",targetModule->m_index);
		#endif
		return;	
	}
			
	/* so go ahead and find the entry point pair... */
	#if logFarLocData
	fprintf(logfile, "Calculating ep against module %d %s\n", targetModule->m_index,targetModule->name);
	#endif
				
	/* compute the bounding box intersection between the modules */
	if(GetModulesIntersection(thisModule, targetModule) == 0)
	{	
		#if logFarLocData
		fprintf(logfile, "Against module %d: BBOX failure \n",targetModule->m_index);
		#endif
		return;		
	}

	/* get the bounding boxes defined by each module's points inside the intersection volume */
	
	numpts1 = GetModulePointBox(thisModule, &MI_Volume2);
	numpts2 = GetModulePointBox(targetModule, &MI_Volume3);
	
	if(numpts1<2 || numpts2<2)
	{
		int extentCentre;

		#if logFarLocData
		fprintf(logfile, "Against module %d: not enough points in intersection \n",targetModule->m_index);
		#endif

		/* Create one anyways. */

		extentCentre=MI_Volume1.maxX + MI_Volume1.minX;
		extentCentre>>=1;
		thisModuleEP.vx = extentCentre; 
		targetModuleEP.vx = extentCentre;

		extentCentre=MI_Volume1.maxY + MI_Volume1.minY;
		extentCentre>>=1;
		thisModuleEP.vy = extentCentre; 
		targetModuleEP.vy = extentCentre;

		extentCentre=MI_Volume1.maxZ + MI_Volume1.minZ;
		extentCentre>>=1;
		thisModuleEP.vz = extentCentre; 
		targetModuleEP.vz = extentCentre;

		if( ((MI_Volume1.maxX - MI_Volume1.minX) < (MI_Volume1.maxY - MI_Volume1.minY))&&
			((MI_Volume1.maxX - MI_Volume1.minX) < (MI_Volume1.maxZ - MI_Volume1.minZ)))
		{
			/* x is smallest */
			if(thisModule->m_world.vx > targetModule->m_world.vx)
			{
				thisModuleEP.vx = MI_Volume2.maxX + EP_POSNDISP;
				targetModuleEP.vx = MI_Volume3.minX - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vx = MI_Volume2.minX - EP_POSNDISP;
				targetModuleEP.vx = MI_Volume3.maxX + EP_POSNDISP;
			}
		}
		else if((MI_Volume1.maxZ - MI_Volume1.minZ) < (MI_Volume1.maxY - MI_Volume1.minY))
		{
			/* z is smallest */

			if(thisModule->m_world.vz > targetModule->m_world.vz)
			{
				thisModuleEP.vz = MI_Volume2.maxZ + EP_POSNDISP;
				targetModuleEP.vz = MI_Volume3.minZ - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vz = MI_Volume2.minZ - EP_POSNDISP;
				targetModuleEP.vz = MI_Volume3.maxZ + EP_POSNDISP;
			}
		}
		else
		{
			/* y is smallest */
			if(thisModule->m_world.vy > targetModule->m_world.vy)
			{
				thisModuleEP.vy = MI_Volume2.maxY + EP_POSNDISP;
				targetModuleEP.vy = MI_Volume3.minY - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vy = MI_Volume2.minY - EP_POSNDISP;
				targetModuleEP.vy = MI_Volume3.maxY + EP_POSNDISP;
			}
		}
		#if logFarLocData
		fprintf(logfile, "Made one anyway.\n");
		#endif
	
	} else {

		/* Test the point bounding boxes, just to make sure */
		LOCALASSERT(MI_Volume2.maxX >= MI_Volume2.minX);
		LOCALASSERT(MI_Volume2.maxY >= MI_Volume2.minY);
		LOCALASSERT(MI_Volume2.maxZ >= MI_Volume2.minZ);
		LOCALASSERT(MI_Volume3.maxX >= MI_Volume3.minX);
		LOCALASSERT(MI_Volume3.maxY >= MI_Volume3.minY);
		LOCALASSERT(MI_Volume3.maxZ >= MI_Volume3.minZ);
		
		/* calculate ep's */
		if( ((MI_Volume1.maxX - MI_Volume1.minX) < (MI_Volume1.maxY - MI_Volume1.minY))&&
			((MI_Volume1.maxX - MI_Volume1.minX) < (MI_Volume1.maxZ - MI_Volume1.minZ)))
		{
			/* x is smallest */
			int extentCentre;
		
			if(MI_Volume2.maxY < MI_Volume3.maxY) extentCentre = MI_Volume2.maxY;
			else extentCentre = MI_Volume3.maxY;
			if(MI_Volume2.minY > MI_Volume3.minY) extentCentre += MI_Volume2.minY;
			else extentCentre += MI_Volume3.minY;
			extentCentre /=	2;
			thisModuleEP.vy = extentCentre; 
			targetModuleEP.vy = extentCentre;
			 
			if(MI_Volume2.maxZ < MI_Volume3.maxZ) extentCentre = MI_Volume2.maxZ;
			else extentCentre = MI_Volume3.maxZ;
			if(MI_Volume2.minZ > MI_Volume3.minZ) extentCentre += MI_Volume2.minZ;
			else extentCentre += MI_Volume3.minZ;
			extentCentre /=	2;
			thisModuleEP.vz = extentCentre; 
			targetModuleEP.vz = extentCentre;
		
			if(thisModule->m_world.vx > targetModule->m_world.vx)
			{
				thisModuleEP.vx = MI_Volume2.maxX + EP_POSNDISP;
				targetModuleEP.vx = MI_Volume3.minX - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vx = MI_Volume2.minX - EP_POSNDISP;
				targetModuleEP.vx = MI_Volume3.maxX + EP_POSNDISP;
			}
		}
		else if((MI_Volume1.maxZ - MI_Volume1.minZ) < (MI_Volume1.maxY - MI_Volume1.minY))
		{
			/* z is smallest */
			int extentCentre;
					 
			if(MI_Volume2.maxX < MI_Volume3.maxX) extentCentre = MI_Volume2.maxX;
			else extentCentre = MI_Volume3.maxX;
			if(MI_Volume2.minX > MI_Volume3.minX) extentCentre += MI_Volume2.minX;
			else extentCentre += MI_Volume3.minX;
			extentCentre /=	2;
			thisModuleEP.vx = extentCentre; 
			targetModuleEP.vx = extentCentre;
		
			if(MI_Volume2.maxY < MI_Volume3.maxY) extentCentre = MI_Volume2.maxY;
			else extentCentre = MI_Volume3.maxY;
			if(MI_Volume2.minY > MI_Volume3.minY) extentCentre += MI_Volume2.minY;
			else extentCentre += MI_Volume3.minY;
			extentCentre /=	2;
			thisModuleEP.vy = extentCentre; 
			targetModuleEP.vy = extentCentre;
		
			if(thisModule->m_world.vz > targetModule->m_world.vz)
			{
				thisModuleEP.vz = MI_Volume2.maxZ + EP_POSNDISP;
				targetModuleEP.vz = MI_Volume3.minZ - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vz = MI_Volume2.minZ - EP_POSNDISP;
				targetModuleEP.vz = MI_Volume3.maxZ + EP_POSNDISP;
			}
		}
		else
		{
			/* y is smallest */
			int extentCentre;
		
			if(MI_Volume2.maxX < MI_Volume3.maxX) extentCentre = MI_Volume2.maxX;
			else extentCentre = MI_Volume3.maxX;
			if(MI_Volume2.minX > MI_Volume3.minX) extentCentre += MI_Volume2.minX;
			else extentCentre += MI_Volume3.minX;
			extentCentre /=	2;
			thisModuleEP.vx = extentCentre; 
			targetModuleEP.vx = extentCentre;
			 
			if(MI_Volume2.maxZ < MI_Volume3.maxZ) extentCentre = MI_Volume2.maxZ;
			else extentCentre = MI_Volume3.maxZ;
			if(MI_Volume2.minZ > MI_Volume3.minZ) extentCentre += MI_Volume2.minZ;
			else extentCentre += MI_Volume3.minZ;
			extentCentre /=	2;
			thisModuleEP.vz = extentCentre; 
			targetModuleEP.vz = extentCentre;
		
			if(thisModule->m_world.vy > targetModule->m_world.vy)
			{
				thisModuleEP.vy = MI_Volume2.maxY + EP_POSNDISP;
				targetModuleEP.vy = MI_Volume3.minY - EP_POSNDISP;
			}
			else
			{
				thisModuleEP.vy = MI_Volume2.minY - EP_POSNDISP;
				targetModuleEP.vy = MI_Volume3.maxY + EP_POSNDISP;
			}
		}
	}

	/* convert back into local space */
	thisModuleEP.vx -= thisModule->m_world.vx;
	thisModuleEP.vy -= thisModule->m_world.vy;
	thisModuleEP.vz -= thisModule->m_world.vz;
	targetModuleEP.vx -= targetModule->m_world.vx;
	targetModuleEP.vy -= targetModule->m_world.vy;
	targetModuleEP.vz -= targetModule->m_world.vz;
		
	/* now test to make sure the entry points are inside their respective modules.
	   (If not, don't add either) */
	{
		int inModule;
		inModule = PointIsInModule(thisModule, &thisModuleEP);
		if(!inModule)
		{
			#if logFarLocData
			fprintf(logfile, "....can't add eps: MODULE %d, EP NOT IN MODULE SPACE \n", thisModule->m_index);
			#endif
			return;
		}
		inModule = PointIsInModule(targetModule, &targetModuleEP);
		if(!inModule)
		{
			#if logFarLocData
			fprintf(logfile, "....can't add eps: MODULE %d, EP NOT IN MODULE SPACE \n", targetModule->m_index);
			#endif
			return;
		}
	}
	
	/* Finally, set up the entry points for this and target module */
	AddModuleEP(thisModule, targetModule, &thisModuleEP);
	AddModuleEP(targetModule, thisModule, &targetModuleEP);	
	#if logFarLocData
	fprintf(logfile, "... entry points added \n");
	#endif
	
}
#endif

/*-----------------------Patrick 28/2/96---------------------------
  This function calculates the bounding box intersection volume
  between 2 adjacent modules (in world space coords).
  
  Returns 1 if the bounding box is valis, 0 if not.
  ------------------------------------------------------------------*/ 
#if 0
static int GetModulesIntersection(MODULE *thisModule, MODULE *targetModule)
{
	int thisExtent, targetExtent;

	/* do x extents */
	thisExtent = (thisModule->m_maxx + thisModule->m_world.vx + EPBB_XTRA);
	targetExtent = (targetModule->m_maxx + targetModule->m_world.vx + EPBB_XTRA);
	if(thisExtent < targetExtent) MI_Volume1.maxX = thisExtent;
	else MI_Volume1.maxX = targetExtent;
	thisExtent = (thisModule->m_minx + thisModule->m_world.vx - EPBB_XTRA);
	targetExtent = (targetModule->m_minx + targetModule->m_world.vx - EPBB_XTRA);
	if(thisExtent > targetExtent) MI_Volume1.minX = thisExtent;
	else MI_Volume1.minX = targetExtent;
	if(!(MI_Volume1.maxX > MI_Volume1.minX)) return 0;

	/* do y extents */
	thisExtent = (thisModule->m_maxy + thisModule->m_world.vy + EPBB_XTRA);
	targetExtent = (targetModule->m_maxy + targetModule->m_world.vy + EPBB_XTRA);
	if(thisExtent < targetExtent) MI_Volume1.maxY = thisExtent;
	else MI_Volume1.maxY = targetExtent;
	thisExtent = (thisModule->m_miny + thisModule->m_world.vy - EPBB_XTRA);
	targetExtent = (targetModule->m_miny + targetModule->m_world.vy - EPBB_XTRA);
	if(thisExtent > targetExtent) MI_Volume1.minY = thisExtent;
	else MI_Volume1.minY = targetExtent;
	if(!(MI_Volume1.maxY > MI_Volume1.minY)) return 0;

	/* do z extents */
	thisExtent = (thisModule->m_maxz + thisModule->m_world.vz + EPBB_XTRA);
	targetExtent = (targetModule->m_maxz+ targetModule->m_world.vz + EPBB_XTRA);
	if(thisExtent < targetExtent) MI_Volume1.maxZ = thisExtent;
	else MI_Volume1.maxZ = targetExtent;
	thisExtent = (thisModule->m_minz + thisModule->m_world.vz - EPBB_XTRA);
	targetExtent = (targetModule->m_minz + targetModule->m_world.vz - EPBB_XTRA);
	if(thisExtent > targetExtent) MI_Volume1.minZ = thisExtent;
	else MI_Volume1.minZ = targetExtent;
	if(!(MI_Volume1.maxZ > MI_Volume1.minZ)) return 0;
	
	return 1;
}


/*-----------------------Patrick 28/2/97---------------------------
Generates a volume defined by the module's points inside the 
mutual intersection volume (in world space)
-------------------------------------------------------------------*/
static int GetModulePointBox(MODULE *thisModule, EPBBEXTENTS *extents)
{
	int numPtsFound = 0;
	int pointCounter;

	/* initialise the extents */
	extents->minX = thisModule->m_maxx + thisModule->m_world.vx;
	extents->maxX = thisModule->m_minx + thisModule->m_world.vx;
	extents->minY = thisModule->m_maxy + thisModule->m_world.vy;
	extents->maxY = thisModule->m_miny + thisModule->m_world.vy;
	extents->minZ = thisModule->m_maxz + thisModule->m_world.vz;
	extents->maxZ = thisModule->m_minz + thisModule->m_world.vz;

	/* go through each point in the shape */
	pointCounter = SetupPointAccessFromShapeIndex(thisModule->m_mapptr->MapShape);
	while(pointCounter>0)
	{
		VECTORCH* thisPt = AccessNextPoint();
		VECTORCH thisWorldPoint;

		thisWorldPoint.vx = thisPt->vx + thisModule->m_world.vx;
		thisWorldPoint.vy = thisPt->vy + thisModule->m_world.vy;
		thisWorldPoint.vz = thisPt->vz + thisModule->m_world.vz;

		if(	(thisWorldPoint.vx >= MI_Volume1.minX)&&
		   	(thisWorldPoint.vx <= MI_Volume1.maxX)&&
		   	(thisWorldPoint.vy >= MI_Volume1.minY)&&
		   	(thisWorldPoint.vy <= MI_Volume1.maxY)&&
		   	(thisWorldPoint.vz >= MI_Volume1.minZ)&&
		   	(thisWorldPoint.vz <= MI_Volume1.maxZ))
		{
			numPtsFound++;

			if(thisWorldPoint.vx > extents->maxX) extents->maxX = thisWorldPoint.vx;
			if(thisWorldPoint.vx < extents->minX) extents->minX = thisWorldPoint.vx;
			if(thisWorldPoint.vy > extents->maxY) extents->maxY = thisWorldPoint.vy;
			if(thisWorldPoint.vy < extents->minY) extents->minY = thisWorldPoint.vy;
			if(thisWorldPoint.vz > extents->maxZ) extents->maxZ = thisWorldPoint.vz;
			if(thisWorldPoint.vz < extents->minZ) extents->minZ = thisWorldPoint.vz;
		}
		pointCounter--;
	}
	return numPtsFound;
}






/*-----------------------Patrick 20/12/96---------------------------
Adds an entry point to the list	for this module, 
-------------------------------------------------------------------*/
static void AddModuleEP(MODULE* thisModule, MODULE*fromModule, VECTORCH *posn)
{
	FARENTRYPOINTSHEADER *epHeader = &FALLP_EntryPoints[thisModule->m_index];
	FARENTRYPOINT *epList = epHeader->entryPointsList;

	if(epHeader->numEntryPoints==(NumAdjacentModules((AIMODULE*)thisModule)))
	{
		/* no room for any more eps. This may occur where two modules are not
		mutually linked as adjacent... specifically, the target is missing the
		link. This shouldn't really happen, and if it does it means there's an
		error in the visibility and/or adjacency links which requires attention.
		The effective result is that the linked module will get an ep from the 
		unlinked module, but not the other way round....
		*/
		#if logFarLocData
		fprintf(logfile, "....CAN'T ADD EP TO MODULE %d : NO EP SLOTS \n", thisModule->m_index);
		#endif
		return;
	}

	LOCALASSERT(epHeader->numEntryPoints<(NumAdjacentModules(thisModule)));
	
	epList[epHeader->numEntryPoints].position = *posn;
	epList[epHeader->numEntryPoints].donorIndex = fromModule->m_index;

	(epHeader->numEntryPoints)++;

}
#endif

/*-----------------------Patrick 20/12/96---------------------------
LOCAL FUNCTIONS FOR AUXILARY MODULE LOCATION SUPPORT
-------------------------------------------------------------------*/

/*--------------------- Patrick 26/11/96 --------------------------
  This function attempts to construct a series of valid 
  auxilary locations inside the module (provided it has entry points)    
  -----------------------------------------------------------------*/
static void BuildFM_AuxilaryLocs(MODULE *thisModule)
{
	int gridStartX, gridStartZ, gridExtentX, gridExtentZ, XIndex, ZIndex;
	int NumLocsValid = 0;
	int NumLocsHeightFailed = 0;
	int NumLocsVolFailed = 0;
	int ThisModuleIndex;
	AIMODULE* aimodule=thisModule->m_aimodule;
	 	
	/* get the module index */
	LOCALASSERT(aimodule);
	ThisModuleIndex = aimodule->m_index;
	LOCALASSERT(ThisModuleIndex >= 0);
	LOCALASSERT(ThisModuleIndex < ModuleArraySize);	
	
	gridStartX = thisModule->m_minx + (FAR_BB_WIDTH>>1);
	gridStartZ = thisModule->m_minz + (FAR_BB_WIDTH>>1);
	gridExtentX = thisModule->m_maxx - thisModule->m_minx - FAR_BB_WIDTH;
	gridExtentZ = thisModule->m_maxz - thisModule->m_minz - FAR_BB_WIDTH;

	if(gridExtentX<=0 || gridExtentZ<=0)
	{
		//module is too narrow for auxilary locations
		return;
	}

	LOCALASSERT(gridStartX > thisModule->m_minx);

	/* step through each grid (index) location */
	for(XIndex = FAR_GRID_SIZE; XIndex > 0; XIndex--)
	{
		for(ZIndex = FAR_GRID_SIZE; ZIndex > 0; ZIndex--)
		{
			int locationsIndex = (XIndex-1)*FAR_GRID_SIZE + (ZIndex-1);

			auxLocsGrid[locationsIndex].position.vx = gridStartX + (gridExtentX*(XIndex-1))/(FAR_GRID_SIZE-1);
			auxLocsGrid[locationsIndex].position.vz = gridStartZ + (gridExtentZ*(ZIndex-1))/(FAR_GRID_SIZE-1);
			auxLocsGrid[locationsIndex].position.vy = 0;
			auxLocsGrid[locationsIndex].valid = 1; /* validated by default */

			/* get the floor height for this location.
			If no valid height is found, the location is set to
			minx,minz,miny, ie. outside of the grid, and abandoned */
			
			GetFarLocHeight(&auxLocsGrid[locationsIndex], thisModule);
	
			/* if there's a valid floor height, check the volume around the location
			for impinging polygons.  If volume is impinged, the location is set to 
			minx,minz,miny, ie. outside of the grid, and abandoned */
			if(auxLocsGrid[locationsIndex].valid)
			{
				FarLocVolumeTest(&auxLocsGrid[locationsIndex], thisModule); 
				if(auxLocsGrid[locationsIndex].valid)
				{
					
					AddVector(&thisModule->m_world,&auxLocsGrid[locationsIndex].position);
					SubVector(&aimodule->m_world,&auxLocsGrid[locationsIndex].position);
					NumLocsValid++;
				}
				else NumLocsVolFailed++;	
			}
			else NumLocsHeightFailed++;
		}
	}
					
	
	#if logFarLocData
	fprintf(logfile, "Num valid locs: %d \n", NumLocsValid);
	fprintf(logfile, "Num Height failed: %d \n", NumLocsHeightFailed);
	fprintf(logfile, "Num Vol failed: %d \n \n", NumLocsVolFailed);
	#endif
	
	/* now have a full list of locations.... 
	Those that are zero are invalid: hopefully some have survived */
	LOCALASSERT((NumLocsHeightFailed+NumLocsVolFailed+NumLocsValid) == (FAR_GRID_SIZE*FAR_GRID_SIZE));

	/* If there are any valid locations remaining, store them in the locations list */
	if(NumLocsValid > 0) 
	{
		/* Build a final and definitive list of valid locations for the module:
		look at the number of valid locations. If there are more than the maximum 
		number, take a reasonably distributed sample. Otherwise just put them all in.
	
		NB this wil be a list of Local Space coordinates.
		*/   			
   	
		if(NumLocsValid > FAR_MAX_LOCS) 
		{
			VECTORCH *destinationPtr;
			int locationsIndex = 0;
			
			int numTaken = 0;
			int numFound = 0;
			int nextToTake = 0; 

			/* fill out the header (space is preallocated) */
			destinationPtr = &FALLP_AuxLocs[ThisModuleIndex].locationsList[FALLP_AuxLocs[ThisModuleIndex].numLocations];
			FALLP_AuxLocs[ThisModuleIndex].numLocations += FAR_MAX_LOCS;

  			numTaken=0;
			nextToTake = NumLocsValid / FAR_MAX_LOCS;					
			
			while(numTaken<FAR_MAX_LOCS)
			{
				LOCALASSERT(nextToTake <= NumLocsValid);
				LOCALASSERT(locationsIndex < (FAR_GRID_SIZE*FAR_GRID_SIZE));			
				
				while(auxLocsGrid[locationsIndex].valid == 0) locationsIndex++;
				LOCALASSERT(locationsIndex < (FAR_GRID_SIZE*FAR_GRID_SIZE));			
				numFound++;
				LOCALASSERT(numFound <= NumLocsValid);

				if(numFound == nextToTake)
				{
					*destinationPtr++ = auxLocsGrid[locationsIndex].position;
					numTaken++;
					/* calc index of the next one to take */
					nextToTake = ((numTaken + 1) * NumLocsValid) / FAR_MAX_LOCS;
				}
				/* move to next location */
				locationsIndex++;
			}

		}
		else
		{	
			VECTORCH *destinationPtr;
			int locationsIndex;
			int checkCount = 0; 

			/* fill out the header (space is preallocated) */
			destinationPtr = &FALLP_AuxLocs[ThisModuleIndex].locationsList[FALLP_AuxLocs[ThisModuleIndex].numLocations];
			FALLP_AuxLocs[ThisModuleIndex].numLocations += NumLocsValid;

			/* fill up the list with what we've got */
			locationsIndex = (FAR_GRID_SIZE*FAR_GRID_SIZE) - 1;
			do
			{
				if(auxLocsGrid[locationsIndex].valid)
				{
					/* found a valid location: copy it into the list */
					*(destinationPtr++) = auxLocsGrid[locationsIndex].position;
					checkCount++;
				}			
			}
			while(locationsIndex--);
			
			LOCALASSERT(checkCount == NumLocsValid);
		
		}
		#if logFarLocData
		{
			/* log the final list */
			VECTORCH *tmpPtr;
			int tmpCounter;
			fprintf(logfile, "Locations list: \n");

			tmpCounter = FALLP_AuxLocs[ThisModuleIndex].numLocations;
			tmpPtr = FALLP_AuxLocs[ThisModuleIndex].locationsList;

			while(tmpCounter > 0)
			{
				fprintf(logfile, "%d %d %d \n", tmpPtr->vx, tmpPtr->vz, tmpPtr->vy);
				tmpPtr++;
				tmpCounter--;	
			}

			fprintf(logfile,"\n");
				
		}
		#endif
	}
	else
	{
		/* No valid locations */
		#if logFarLocData
		{
			/* log an error */
			fprintf(logfile,"All auxilary locations FAILED \n");
		}
		#endif	
	}									
}

/*----------------------Patrick 1/12/96--------------------------
  Given an x/z position for the shape, this function returns a 
  height value, by examining upwards facing polygons....
  To validate the height, there must also be a downwards facing
  polgon above the upwards facing polygon (so that we know that
  the location iiiiis inside the shape): if there is no up &
  down polygon, the location is invalidated, and an error code
  returned (either no up poly, no down poly, no up-or-down poly)
  ----------------------------------------------------------------*/
static void GetFarLocHeight(FARVALIDATEDLOCATION *location, MODULE *thisModule)
{
	int polyCounter;	
	int heightOfUpPoly = thisModule->m_maxy; /* init to lowest module extent */
	int heightOfDownPoly = thisModule->m_miny; /* init to heighest module extent */
	int upPolyFound = 0;
	int downPolyFound = 0;
	int XZcontainment;
			
	struct ColPolyTag polygonData;
	
	polyCounter = SetupPolygonAccessFromShapeIndex(thisModule->m_mapptr->MapShape);
	
	/* loop through the item list, then ... */
	while(polyCounter>0)
	{
		AccessNextPolygon();
		GetPolygonVertices(&polygonData);
		GetPolygonNormal(&polygonData);
				
		/* first test if poly is vertical */
		if((polygonData.PolyNormal.vy > FAR_MIN_INCLINE)||(polygonData.PolyNormal.vy < -FAR_MIN_INCLINE))
		{
			XZcontainment = IsXZinPoly(&(location->position), &polygonData);
			if(XZcontainment)
			{				
				if(polygonData.PolyNormal.vy > 0) /* downwards facing */
				{
					downPolyFound++;
					{
						/* find height of this poly: height of lowest vertex */
						int tmpHeight = polygonData.PolyPoint[0].vy;
						
						if(polygonData.PolyPoint[1].vy > tmpHeight) tmpHeight = polygonData.PolyPoint[1].vy;
						if(polygonData.PolyPoint[2].vy > tmpHeight) tmpHeight = polygonData.PolyPoint[2].vy;
						if(polygonData.NumberOfVertices == 4)
						{
							if(polygonData.PolyPoint[3].vy > tmpHeight) tmpHeight = polygonData.PolyPoint[2].vy;
						}
						
						/* record height of lowest downward facing poly */
						if(tmpHeight > heightOfDownPoly) heightOfDownPoly =	tmpHeight;						
					}
				}
				else /* upwards facing */
				{
					upPolyFound++;
					{
						/* find height of this poly: height of highest vertex */
						int tmpHeight = polygonData.PolyPoint[0].vy;
						
						if(polygonData.PolyPoint[1].vy < tmpHeight) tmpHeight = polygonData.PolyPoint[1].vy;
						if(polygonData.PolyPoint[2].vy < tmpHeight) tmpHeight = polygonData.PolyPoint[2].vy;
						if(polygonData.NumberOfVertices == 4)
						{
							if(polygonData.PolyPoint[3].vy < tmpHeight) tmpHeight = polygonData.PolyPoint[2].vy;
						}
						
						/* record height of heighest upward facing poly */
						if(tmpHeight < heightOfUpPoly) heightOfUpPoly =	tmpHeight;						
					}
				}
			}
		}
		polyCounter--;		
	}			
	/* if up & down polys exist check their heights:
	if there is not enough clearance bewteen the lowest down poly and the heighest up poly, 
	invalidate the location.*/
	if((upPolyFound!=0) && (downPolyFound!=0))
	{
			int minclearance = FAR_BB_HEIGHT;
			if(thisModule->m_flags & MODULEFLAG_AIRDUCT) minclearance>>=1;
			if((heightOfUpPoly-heightOfDownPoly)>=minclearance)
			{
				//position the aux location slightly above the polygon
				location->position.vy = heightOfUpPoly-10;
			}
			else location->valid = 0;		
	}
	else location->valid = 0;
 }


/*------------------ Patrick 3/12/96 ----------------------
  This function determines whether an x/z position lies
  within the x/z projection of a polygon.

  NB is not entirely accurate for concave polygons.
  ---------------------------------------------------------*/
static int IsXZinPoly(VECTORCH* location, struct ColPolyTag *polygonData)
{
	int x = location->vx;
	int z = location->vz;
	int xa,za,xb,zb;
	int intersections = 0;
	int intersectMinz, intersectMaxz;
	int linesToTest;
	int nextLine;
	
	linesToTest = polygonData->NumberOfVertices;

	if(linesToTest == 3)
	{
		xa = polygonData->PolyPoint[2].vx;
		za = polygonData->PolyPoint[2].vz;
	}
	else
	{
		LOCALASSERT(linesToTest == 4);
		xa = polygonData->PolyPoint[3].vx;
		za = polygonData->PolyPoint[3].vz;
	}

	nextLine = 0;
	
	while(nextLine < linesToTest)
	{
		/* copy last first point to next last point (?) */
		xb = xa;
		zb = za;
		
		/* get next first point */
		xa = polygonData->PolyPoint[nextLine].vx;
		za = polygonData->PolyPoint[nextLine].vz;
				
		if(((x>=xb) && (x<=xa)) || ((x<xb) && (x>=xa)))
		{
		  	/* intesection ! */
			intersections++;
			if(!(intersections<4)) return 0;

			{
				int zPosn;

				if(xb==xa)
					zPosn = za;
				else
					zPosn = zb + WideMulNarrowDiv((za-zb),(x-xb),(xa-xb));
				
				if(intersections == 1)
					intersectMinz = intersectMaxz = zPosn;
				else
				{
					if(zPosn < intersectMinz) intersectMinz = zPosn;
					else if(zPosn > intersectMaxz) intersectMaxz = zPosn;
				}
			}
		}	  	
	  	nextLine++;
	}

	if(intersections == 0) return 0;
	if(intersections == 1)
		return (z == intersectMinz);	
	else 
		return ((z >= intersectMinz)&&(z <= intersectMaxz));
	
}


/*--------------------Patrick 4/12/96--------------------
  This function checks a potential location for impinging
  downward or sideways facing polygons (ie ceiling or floor).  
  This is done using a bounding box containment test.  If 
  the test fails, it invalidates the location.
  -------------------------------------------------------*/

static int farbbox_maxx;
static int farbbox_minx;
static int farbbox_maxy;
static int farbbox_miny;
static int farbbox_maxz;
static int farbbox_minz;
static struct ColPolyTag farbbox_polygonData;

static int FarBoxContainsPolygon();

static void FarLocVolumeTest(FARVALIDATEDLOCATION *location, MODULE *thisModule)
{
	int polyCounter;
	int containmentFailure = 0;

	LOCALASSERT(location->valid);
	
	/* the location is provided as an x,y,z:
	the x and z indicate the centre, and the y indicates the bottom.
	translate these into bounding box extents.... */
	
	/* 10/7/97: this test has been modified: the bbox is moved up slightly, and any
	impinging ploygon invalidates the location */

	farbbox_maxx = location->position.vx + (FAR_BB_WIDTH>>1);
	farbbox_minx = location->position.vx - (FAR_BB_WIDTH>>1);
	farbbox_maxz = location->position.vz + (FAR_BB_WIDTH>>1);
	farbbox_minz = location->position.vz - (FAR_BB_WIDTH>>1);
	farbbox_maxy = location->position.vy - 10;

	/* patrick 4/7/97: a little adittion for airducts: npc should be crouched in them */
	if(thisModule->m_flags & MODULEFLAG_AIRDUCT)
		farbbox_miny = location->position.vy - (FAR_BB_HEIGHT>>1) - 10;	
	else
		farbbox_miny = location->position.vy - FAR_BB_HEIGHT - 10;

	/* now just run through the polygons in the shape. If a polygon is 
	inside (actually, not definitely outside) the bbox, invalidate the location */		
 	polyCounter = SetupPolygonAccessFromShapeIndex(thisModule->m_mapptr->MapShape);
	while((polyCounter>0)&&(!containmentFailure))
	{
		AccessNextPolygon();
		GetPolygonVertices(&farbbox_polygonData);
		containmentFailure = FarBoxContainsPolygon();					
		polyCounter--;
	}
	/* so, if there	has been a containmentFailure, invalidate the location */
	if(containmentFailure) location->valid = 0;
}

/*--------------------Patrick 5/12/96----------------------------------
 This does a bounding box extent test for a polygon.
 NB returns false if the poly definitely isn't in the bbox, and returns 
 true if the poly is in the bbox, and for unresolved cases.
 ----------------------------------------------------------------------*/
static int FarBoxContainsPolygon()
{
	if(farbbox_polygonData.NumberOfVertices == 3)
	{
		if(	(farbbox_polygonData.PolyPoint[0].vy<=farbbox_miny)&&
			(farbbox_polygonData.PolyPoint[1].vy<=farbbox_miny)&&
			(farbbox_polygonData.PolyPoint[2].vy<=farbbox_miny))	return 0;

		if(	(farbbox_polygonData.PolyPoint[0].vx<=farbbox_minx)&&
			(farbbox_polygonData.PolyPoint[1].vx<=farbbox_minx)&&
			(farbbox_polygonData.PolyPoint[2].vx<=farbbox_minx))	return 0;	

		if( (farbbox_polygonData.PolyPoint[0].vz<=farbbox_minz)&&
			(farbbox_polygonData.PolyPoint[1].vz<=farbbox_minz)&&
			(farbbox_polygonData.PolyPoint[2].vz<=farbbox_minz))	return 0;

		if( (farbbox_polygonData.PolyPoint[0].vz>=farbbox_maxz)&&
			(farbbox_polygonData.PolyPoint[1].vz>=farbbox_maxz)&&
			(farbbox_polygonData.PolyPoint[2].vz>=farbbox_maxz))	return 0;
						
		if( (farbbox_polygonData.PolyPoint[0].vx>=farbbox_maxx)&&
			(farbbox_polygonData.PolyPoint[1].vx>=farbbox_maxx)&&
			(farbbox_polygonData.PolyPoint[2].vx>=farbbox_maxx))	return 0;

		if( (farbbox_polygonData.PolyPoint[0].vy>=farbbox_maxy)&&
			(farbbox_polygonData.PolyPoint[1].vy>=farbbox_maxy)&&
			(farbbox_polygonData.PolyPoint[2].vy>=farbbox_maxy))	return 0;
	}
	else
	{
		if(	(farbbox_polygonData.PolyPoint[0].vy<=farbbox_miny)&&
			(farbbox_polygonData.PolyPoint[1].vy<=farbbox_miny)&&
			(farbbox_polygonData.PolyPoint[2].vy<=farbbox_miny)&&
			(farbbox_polygonData.PolyPoint[3].vy<=farbbox_miny))	return 0;

		if(	(farbbox_polygonData.PolyPoint[0].vx<=farbbox_minx)&&
			(farbbox_polygonData.PolyPoint[1].vx<=farbbox_minx)&&
			(farbbox_polygonData.PolyPoint[2].vx<=farbbox_minx)&&
			(farbbox_polygonData.PolyPoint[3].vx<=farbbox_minx))	return 0;	

		if( (farbbox_polygonData.PolyPoint[0].vz<=farbbox_minz)&&
			(farbbox_polygonData.PolyPoint[1].vz<=farbbox_minz)&&
			(farbbox_polygonData.PolyPoint[2].vz<=farbbox_minz)&&
			(farbbox_polygonData.PolyPoint[3].vz<=farbbox_minz))	return 0;

		if( (farbbox_polygonData.PolyPoint[0].vz>=farbbox_maxz)&&
			(farbbox_polygonData.PolyPoint[1].vz>=farbbox_maxz)&&
			(farbbox_polygonData.PolyPoint[2].vz>=farbbox_maxz)&&
			(farbbox_polygonData.PolyPoint[3].vz>=farbbox_maxz))	return 0;

		if( (farbbox_polygonData.PolyPoint[0].vx>=farbbox_maxx)&&
			(farbbox_polygonData.PolyPoint[1].vx>=farbbox_maxx)&&
			(farbbox_polygonData.PolyPoint[2].vx>=farbbox_maxx)&&
			(farbbox_polygonData.PolyPoint[3].vx>=farbbox_maxx))	return 0;
						
		if( (farbbox_polygonData.PolyPoint[0].vy>=farbbox_maxy)&&
			(farbbox_polygonData.PolyPoint[1].vy>=farbbox_maxy)&&
			(farbbox_polygonData.PolyPoint[2].vy>=farbbox_maxy)&&
			(farbbox_polygonData.PolyPoint[3].vy>=farbbox_maxy))	return 0;
	}	
	return 1;
}

