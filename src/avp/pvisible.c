/*------------------------Patrick 14/1/97-----------------------------
  This source file contains all the functions for the object visibility 
  management system     (which controls visibility for aliens, objects,
  pickups, autoguns, etc....)
  It also contains initialisation and behaviour functions for 
  inanimate objects (like chairs, weapons, etc...).
  --------------------------------------------------------------------*/
#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"
#include "dynblock.h"

#include "bh_alien.h"
#include "pvisible.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "bh_fhug.h"
#include "bh_debri.h"
#include "bh_plachier.h"
#include "plat_shp.h"
#include "psnd.h"
#include "lighting.h"
#include "pldnet.h"
#include "bh_dummy.h"
#include "bh_videoscreen.h"
#include "bh_plift.h"
#include "bh_light.h"
#include "weapons.h"
#include "bh_agun.h"
#include "bh_corpse.h"
#include "chnkload.h"

/* for win95 net game support */
#include "pldghost.h"

#include "pfarlocs.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#define HMODEL_HACK 0

/* prototypes... */
static int WorldPointIsInModule(MODULE* thisModule, VECTORCH* thisPoint);
static int EmergencyRelocateObject(STRATEGYBLOCK *sbPtr);
static void EmergencyPlaceObjectInModule(STRATEGYBLOCK *sbPtr, AIMODULE* target);
static void FragmentInanimateObject(STRATEGYBLOCK *sbptr);
static void ExplodeInanimateObject(STRATEGYBLOCK *sbptr);
static void RespawnInanimateObject(STRATEGYBLOCK *sbPtr);
void KillFragmentalObjectForRespawn(STRATEGYBLOCK *sbPtr);

void IdentifyObject(STRATEGYBLOCK *sbPtr);

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NumActiveBlocks;
extern int NormalFrameTime;
extern int GlobalFrameCounter;

extern void ActivateSelfDestructSequence(int seconds);

extern SOUND3DDATA Explosion_SoundData;

/* this is a default map used in visibility management 
NB If you use this, you MUST set the shapeIndex field appropriately*/
MODULEMAPBLOCK VisibilityDefaultObjectMap =
{
        MapType_Default,
        I_ShapeCube, /* this is a default value */
        {0,0,0},
        {0,0,0},
        #if StandardStrategyAndCollisions
        ObFlag_Dynamic|ObFlag_NewtonMovement|ObFlag_MatMul,
        #else
        0,
        #endif
        0,
        0,
        #if StandardStrategyAndCollisions
        StrategyI_Null,
        0,                                                                      
    GameCollStrat_Default,              
        ShapeCStrat_DoubleExtentEllipsoid,      
        0,                                                      
        #endif
        0,                                                      
        0,                                                      
        0,      
        #if StandardStrategyAndCollisions                                               
        0,                                                      
        0,0,0,                                  
        #endif
        {0,0,0},
        0,                                               
        0,                                               
        #if StandardStrategyAndCollisions
        0,                                               
        0,
        #endif                                           
        0,
        0,
        {0,0,0}
};





/*---------------------Patrick 14/1/97-----------------------------
  
                        SOURCE FOR VISIBILITY MANAGEMENT SYSTEM
  
  -----------------------------------------------------------------*/






/*----------------------Patrick 16/1/97-----------------------------
This function must be called to initialise the 'containingModule' 
field in all strategyblocks for rif-loaded objects.     This cannot 
be done in the first-stage initialisation (via enableBehaviourTypes) 
as the module system is not enabled at this point.
--------------------------------------------------------------------*/
void InitObjectVisibilities(void)
{
        extern int NumActiveStBlocks;
        extern STRATEGYBLOCK *ActiveStBlockList[];      

        int sbIndex = 0;
        STRATEGYBLOCK *sbPtr;

        /* loop thro' the strategy block list, looking for objects that will have
        their visibilities managed ... */
        while(sbIndex < NumActiveStBlocks)
        {       
                sbPtr = ActiveStBlockList[sbIndex++];
                if(     (sbPtr->I_SBtype ==     I_BehaviourAlien)||
                        (sbPtr->I_SBtype ==     I_BehaviourMarine)||
                        (sbPtr->I_SBtype ==     I_BehaviourInanimateObject)||
                        (sbPtr->I_SBtype ==     I_BehaviourVideoScreen)||
                        (sbPtr->I_SBtype ==     I_BehaviourQueenAlien)||
                        (sbPtr->I_SBtype ==     I_BehaviourFaceHugger)||
                        (sbPtr->I_SBtype ==     I_BehaviourPredator)||
                        (sbPtr->I_SBtype ==     I_BehaviourAutoGun)||
                        (sbPtr->I_SBtype ==     I_BehaviourPlatform)||
                        (sbPtr->I_SBtype ==     I_BehaviourBinarySwitch && sbPtr->DynPtr)||/*Allow for switches with no shape*/         
                        (sbPtr->I_SBtype ==     I_BehaviourLinkSwitch && sbPtr->DynPtr)||                       
                        (sbPtr->I_SBtype ==     I_BehaviourTrackObject)||                       
                        (sbPtr->I_SBtype ==     I_BehaviourFan)||                       
                        (sbPtr->I_SBtype ==     I_BehaviourPlacedHierarchy)||                   
                        (sbPtr->I_SBtype ==     I_BehaviourPlacedLight)||                       
                        (sbPtr->I_SBtype ==     I_BehaviourXenoborg)||
                        (sbPtr->I_SBtype ==     I_BehaviourSeal)||
                        (sbPtr->I_SBtype ==     I_BehaviourDatabase)||
                        (sbPtr->I_SBtype ==     I_BehaviourDummy)||
                        (sbPtr->I_SBtype ==     I_BehaviourPredatorAlien)
                        )
                {
                        DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
                        LOCALASSERT(dynPtr);
                        
                        sbPtr->containingModule = ModuleFromPosition(&(dynPtr->Position), (MODULE *)0);
						
                        sbPtr->maintainVisibility = 1;
						
                        
                }
                else
                {
                        /* just in case ... */
                        if(sbPtr->I_SBtype !=   I_BehaviourGenerator &&
						   sbPtr->I_SBtype !=   I_BehaviourMarinePlayer)
                        {
                                sbPtr->containingModule = (MODULE *)0;
                        }
                        sbPtr->maintainVisibility = 0;
                }
                                                        
        }

        /* initialise each object's visibility */
        DoObjectVisibilities();

}


/*----------------------Patrick 13/1/97-----------------------------
This function should be called after the dynamics, and before 
rendering (ie via Cris H's module handler call-back function).

It identifies appropriate strategy blocks in the current block list,
(those which have a non-zero 'containingModule' field), and calls
DoObjectVisibility().
--------------------------------------------------------------------*/

void DoObjectVisibilities(void)
{
        extern int NumActiveStBlocks;
        extern STRATEGYBLOCK *ActiveStBlockList[];      

        int sbIndex = 0;
        STRATEGYBLOCK *sbPtr;

        /* loop thro' the strategy block list, looking for objects that need to have
        their visibilities managed ... */
        while(sbIndex < NumActiveStBlocks)
        {       
                sbPtr = ActiveStBlockList[sbIndex++];
                if(sbPtr->maintainVisibility)
                        DoObjectVisibility(sbPtr);                              
        }
}


void DoObjectVisibility(STRATEGYBLOCK *sbPtr)
{       
        if(!(sbPtr->SBdptr))
        {                                                
                /* Note that we don't call modulefromposition() for far objects, as they mostly don't 
                move, and those that do (eg AIs) move to precalculated positions in modules. Thus
                invisible objects are responsible for looking after their own containingModule field.
                This should always be ok: we should always have a correct and valid containing
                module. However, we will do a paranoia check for a null containingModule... */ 
                if(!sbPtr->containingModule)    
                {
                        textprint("Calling Far EmergencyRelocateObject, On object %x, type %d!\n",(int)sbPtr, sbPtr->I_SBtype);
                        IdentifyObject(sbPtr);
                        if(!(EmergencyRelocateObject(sbPtr))) {
                                textprint("Relocate failed!\n");
                                return;
                        }
                }
                if (!sbPtr->containingModule)
                {
                        textprint("Relocate failed and reported success\n");
                        return;
                }

                /* Now do the visibility check: the object has no display block, so check if 
                it's module is visible. If so, make the object visibile too. */


                if ( (sbPtr->I_SBtype == I_BehaviourPlacedLight)
                &&ThisObjectIsInAModuleVisibleFromCurrentlyVisibleModules(sbPtr))
                {
                        MakePlacedLightNear(sbPtr);
                        return;
                }
                else if (sbPtr->I_SBtype == I_BehaviourNetGhost)
                {
                        NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
                        if (ghostDataPtr && ghostDataPtr->type == I_BehaviourFlareGrenade)
                        {
                                if(ThisObjectIsInAModuleVisibleFromCurrentlyVisibleModules(sbPtr))
                                {
                                        MakeGhostNear(sbPtr);
                                        return;
                                }
                        }
                }
				else if (sbPtr->I_SBtype == I_BehaviourPlatform)
				{
					PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
                	//platform lift needs to be made near if its module near or 
					//if it is moving
                	if(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] ||
					   platformliftdata->state==PLBS_GoingUp ||
					   platformliftdata->state==PLBS_GoingDown)
					{
						MakeObjectNear(sbPtr);
						return;
					}
				}
                
                if(ModuleCurrVisArray[(sbPtr->containingModule->m_index)])
                { 
                        /* module is visible, so make object visible too */
                        switch(sbPtr->I_SBtype)
                        {
                                case(I_BehaviourAlien):
                                {
                                        MakeAlienNear(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourVideoScreen):
                                {
                                        MakeObjectNear(sbPtr);
                                        if(sbPtr->SBdptr) AddLightingEffectToObject(sbPtr->SBdptr,LFX_FLARE);
                                        break;
                                }
                                case(I_BehaviourRubberDuck):
                                case(I_BehaviourInanimateObject):
                                {
                                        MakeObjectNear(sbPtr);
                                        break;          
                                }                               
                                case(I_BehaviourAutoGun):
                                {
                                        MakeSentrygunNear(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourPlatform):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourBinarySwitch):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourDatabase):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourLinkSwitch):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourPredator):
                                {
                                        MakePredatorNear(sbPtr);                                        
                                        break;          
                                }
                                case(I_BehaviourXenoborg):
                                {
                                        MakeXenoborgNear(sbPtr);                                        
                                        break;          
                                }
                                case(I_BehaviourQueenAlien):
                                {
                                        MakeQueenNear(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourPredatorAlien):
                                {
                                        GLOBALASSERT(0);
                                        //MakePAQNear(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourFaceHugger):
                                {
                                        MakeFacehuggerNear(sbPtr);                                      
                                        break;          
                                }
                                case(I_BehaviourMarine):
                                {
                                        MakeMarineNear(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourSeal):
                                {
                                        MakeMarineNear(sbPtr); 
                                        break;          
                                }
                                case(I_BehaviourNetGhost):
                                {
                                        NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;

                                        /* KJL 16:42:40 23/01/99 - near behaviour is triggered differently for
                                           lightsources such as flares */
                                        if (ghostDataPtr && ghostDataPtr->type != I_BehaviourFlareGrenade)
                                        {
                                                MakeGhostNear(sbPtr); 
                                        }

                                        break;          
                                }
                                case(I_BehaviourTrackObject):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourFan):
                                {
                                        MakeObjectNear(sbPtr);                                  
                                        break;          
                                }
                                case(I_BehaviourNetCorpse):
                                {
                                        MakeCorpseNear(sbPtr);
                                        break;
                                }
                                case (I_BehaviourPlacedHierarchy):
                                {
                                        MakePlacedHierarchyNear(sbPtr);
                                        break;
                                }
                                case (I_BehaviourPlacedLight):
                                {
                                        /* KJL 16:42:40 23/01/99 - do nothing; near behaviour is triggered 
                                        differently for lightsources */
                                        break;
                                }
                                case (I_BehaviourDummy):
                                {
                                        MakeDummyNear(sbPtr);
                                        break;
                                }
                                default:
                                {
                                        /* only the above object types should get here */
                                        LOCALASSERT(1==0);                                                                              
                                }
                        }
                }
        }
        else
        {
                /* object is currently visible. 
                first need to get it's module, as it may have moved under dynamics */
                MODULE* newModule;
                DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
                LOCALASSERT(dynPtr);                                    

                newModule = ModuleFromPosition(&(dynPtr->Position), (sbPtr->containingModule));
                if(!(newModule))
                {
                        /* attempt to relocate object */
                        textprint("Calling Near EmergencyRelocateObject, On object %x, type %d!\n",(int)sbPtr, sbPtr->I_SBtype);
                        IdentifyObject(sbPtr);
                        if(!(EmergencyRelocateObject(sbPtr))) {
                                textprint("Relocate failed!\n");
                                return;
                        }
                }
                else
                        /* update object's module field */
                        sbPtr->containingModule = newModule;
                        
                /* now check the object's module */
                if (sbPtr->I_SBtype == I_BehaviourPlacedLight)
                {
                        if(!ThisObjectIsInAModuleVisibleFromCurrentlyVisibleModules(sbPtr))
                        {
                                MakeObjectFar(sbPtr);
                        }
                }
                else if(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] == 0)
                { 
                        /* module is invisible, so make object invisible too */
                        switch(sbPtr->I_SBtype)
                        {
                                case(I_BehaviourAlien):
                                {
                                        MakeAlienFar(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourRubberDuck):
                                case(I_BehaviourVideoScreen):
                                case(I_BehaviourInanimateObject):
                                {
                                        MakeObjectFar(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourAutoGun):
                                {
                                        MakeSentrygunFar(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourPlatform):
                                {
					PLATFORMLIFT_BEHAVIOUR_BLOCK *platformliftdata = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
					//don't make platform lift far if it is currently moving
					//(otherwise the lift won't be able to move)
					if(platformliftdata->state!=PLBS_GoingUp &&
					   platformliftdata->state!=PLBS_GoingDown)
					{	
						MakeObjectFar(sbPtr);
					}
                                        break;          
                                }                               
                                case(I_BehaviourBinarySwitch):
                                {
                                        MakeObjectFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourDatabase):
                                {
                                        MakeObjectFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourLinkSwitch):
                                {
                                        MakeObjectFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourPredator):
                                {
                                        MakePredatorFar(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourXenoborg):
                                {
                                        MakeXenoborgFar(sbPtr);
                                        break;          
                                }
                                case(I_BehaviourQueenAlien):
                                {
                                        MakeQueenFar(sbPtr);                                    
                                        break;          
                                }
                                case(I_BehaviourPredatorAlien):
                                {
                                        //MakePAQFar(sbPtr); 
                                        GLOBALASSERT(0);
                                        /* Should be BehaviourAlien! */
                                        break;          
                                }
                                case(I_BehaviourFaceHugger):
                                {
                                        MakeFacehuggerFar(sbPtr);               
                                        break;          
                                }
                                case(I_BehaviourMarine):
                                {
                                        MakeMarineFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourSeal):
                                {
                                        MakeMarineFar(sbPtr); /* not yet supported */                                   
                                        break;          
                                }
                                case(I_BehaviourNetGhost):
                                {
                                        MakeGhostFar(sbPtr); 
                                        break;          
                                }
                                case(I_BehaviourTrackObject):
                                {
                                        MakeObjectFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourFan):
                                {
                                        MakeObjectFar(sbPtr);                                   
                                        break;          
                                }
                                case(I_BehaviourNetCorpse):
                                {
                                        MakeCorpseFar(sbPtr);
                                        break;
                                }
                                case (I_BehaviourPlacedHierarchy):
                                {
                                        MakeObjectFar(sbPtr);
                                        break;
                                }
                                case (I_BehaviourDummy):
                                {
                                        MakeDummyFar(sbPtr);
                                        break;
                                }
                                default:
                                {
                                        /* only the above object types should get here */
                                        LOCALASSERT(1==0);                                                                              
                                }
                        }

                }

        }
                        
}

/*-----------------------Patrick 15/1/97---------------------------
This pair of functions can be used to control the visibility of 
most objects in the environment. Object specific stuff can be done
after the approriate call in doObjectVisibility, or a separate
function can be used (as for aliens)
-------------------------------------------------------------------*/

HMODELCONTROLLER DropShipHModelController;

void MakeObjectNear(STRATEGYBLOCK *sbPtr)
{
        MODULE tempModule;
        DISPLAYBLOCK *dPtr;
        DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

    LOCALASSERT(dynPtr);
        LOCALASSERT(sbPtr->SBdptr == NULL);

        VisibilityDefaultObjectMap.MapShape = sbPtr->shapeIndex;
        tempModule.m_mapptr = &VisibilityDefaultObjectMap;
        tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
        tempModule.m_numlights = 0;
        tempModule.m_lightarray = (struct lightblock *)0;
        tempModule.m_extraitemdata = (struct extraitemdata *)0;
        tempModule.m_dptr = NULL; /* this is important */
        tempModule.name = NULL; /* this is important */

        AllocateModuleObject(&tempModule); 
        dPtr = tempModule.m_dptr;               
        if(dPtr==NULL) return; /* cannot create displayblock, so leave object "far" */
        
        sbPtr->SBdptr = dPtr;
        dPtr->ObStrategyBlock = sbPtr;
        dPtr->ObMyModule = NULL;                                        
                            
        /* also need to initialise positional information in the new display
        block from the existing dynamics block: this necessary because this 
        function is (usually) called between the dynamics and rendering systems
        so it is not initialised by the dynamics system the first time it is
        drawn. */
        dPtr->ObWorld = dynPtr->Position;
        dPtr->ObEuler = dynPtr->OrientEuler;
        dPtr->ObMat = dynPtr->OrientMat;
        
        #if HMODEL_HACK

        if (dPtr->ObShape==GetLoadedShapeMSL("computer")) {

                extern SECTION Chest;
                SECTION *Test_Section;

                extern SECTION *GetHierarchyFromLibrary(const char *rif_name);
                
                Test_Section=GetHierarchyFromLibrary("hnpcalien");

                //Preprocess_HModel(Test_Section);
                Create_HModel(&DropShipHModelController,Test_Section);
                InitHModelSequence(&DropShipHModelController,3,3,ONE_FIXED<<1);
                dPtr->HModelControlBlock=&DropShipHModelController;


                //dPtr->HModelControlBlock->Playing=1;
                dPtr->HModelControlBlock->Looped=1;

        }       
        
        #endif


}

void MakeObjectFar(STRATEGYBLOCK *sbPtr)
{
        int i;

        #if HMODEL_HACK
           
        if (sbPtr->SBdptr->ObShape==GetLoadedShapeMSL("chest")) {
                Dispel_HModel(&DropShipHModelController);
        }

        #endif

        LOCALASSERT(sbPtr->SBdptr != NULL);

        /* get rid of the displayblock */
        i = DestroyActiveObject(sbPtr->SBdptr);
        if(i!=0)
        {
                LOCALASSERT(1==0);
        }
        sbPtr->SBdptr = NULL;

}


/*----------------------Patrick 15/1/97-----------------------------
This function relocates an object back into the environment:
this may be an npc, inanimate object, etc.
It is called if the visibility system fails to find a 
containing module for the object, which may happen for a number
of reasons, eg: an object is blown out of the visible part of the 
environment, or an npc falls out...
NB returns 0 if relocation failed.
--------------------------------------------------------------------*/
static int EmergRelocCalls = 0;
static int EmergencyRelocateObject(STRATEGYBLOCK *sbPtr)
{

        EmergRelocCalls++;

        if(sbPtr->I_SBtype == I_BehaviourNetGhost) return 1;

        /* KJL 14:48:36 09/02/98 - ignore platform lifts */
        if (sbPtr->I_SBtype == I_BehaviourPlatform) return 1;

        /* first, try to reset the object's position: if it has a valid 'containingModule',
        this means that it's last position was in that module, so we can use that as a reset
        position. 
        NB this means that NPC's will be unable to 'run away' into an invisible module. 
        */
        
        if(sbPtr->containingModule) 
        {
        
                /* Nooo, this doesn't work! CDF 3/12/97 Hack. */

                if ((sbPtr->I_SBtype!=I_BehaviourAlien) 
                        &&(sbPtr->I_SBtype!=I_BehaviourMarine)
                ) {
                        textprint("Valid containingModule.\n");
                        sbPtr->DynPtr->Position = sbPtr->DynPtr->PrevPosition;
                        return 1;
                }
        }
                        
        /* so, we don't have a previous module... then search the environment for the 
        nearest invisible module that has entry point locations, and relocate to one of 
        these locations. */
        {       
                //extern SCENE Global_Scene;
                //extern SCENEMODULE **Global_ModulePtr;
                
                AIMODULE *targetModule = 0;
                int targetModuleDistance = 0;
                //SCENEMODULE *ScenePtr;
                AIMODULE *moduleListPointer;    
                int moduleCounter;

                LOCALASSERT(ModuleArraySize);
                LOCALASSERT(Global_ModulePtr);
                LOCALASSERT(FALLP_EntryPoints); /* NB should never get here in a net game, so the codition should be true */

                //ScenePtr = Global_ModulePtr[Global_Scene];
                //moduleListPointer = ScenePtr->sm_marray;
                {
                        extern AIMODULE *AIModuleArray;
                        moduleListPointer = AIModuleArray;
                }

                for(moduleCounter = 0; moduleCounter < AIModuleArraySize; moduleCounter++)
                {
                        AIMODULE *thisModule = &(moduleListPointer[moduleCounter]); 
                        //MODULE* thisModule = moduleListPointer[moduleCounter]; 
        
                        if(     (!(ModuleCurrVisArray[thisModule->m_index]))&&
                                (FALLP_EntryPoints[thisModule->m_index].numEntryPoints != 0)
                          )
                        {
                                /* a candidate */
                                int thisModuleDistance = 
                                        VectorDistance(&(thisModule->m_world), &(sbPtr->DynPtr->Position));                                             
                                                
                                if((!targetModule) || (thisModuleDistance < targetModuleDistance))
                                { 
                                        targetModule = thisModule;
                                        targetModuleDistance = thisModuleDistance;      
                                }
                        }               
                }       

                if(!targetModule)
                {
                        /* this condition shouldn't happen, but since I'm paranoid... */
                        //This can happen on the skirmish levels (where the whole level is visible at once)
                        //Therefore if the object has a previous containing module , use it.
                        //Otherwise get rid of it.

                        //LOCALASSERT(1==0);
                        if(sbPtr->containingModule) 
                        {
        
                                /* Nooo, this doesn't work! CDF 3/12/97 Hack. */

                                textprint("Valid containingModule.\n");
                                sbPtr->DynPtr->Position = sbPtr->DynPtr->PrevPosition;
                                return 1;
                                
                        }
                        DestroyAnyStrategyBlock(sbPtr);
                        return 0;
                }
                
                EmergencyPlaceObjectInModule(sbPtr, targetModule);
                
                #if debug
                if (!sbPtr->containingModule)
                {
                        textprint("WARNING!! EmgcyPlcObInMod failed\n");
                }
                #endif
                
                
                return sbPtr->containingModule ? 1 : 0;
        }
}

static void EmergencyPlaceObjectInModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
        int noOfEntryPoints;
        FARENTRYPOINT *entryPointsList;
        VECTORCH newPosition;
        MODULE *renderModule;

        textprint("Calling EmergencyPlaceObjectInModule!\n");

        /* first off, assert a few pre-conditions */
        GLOBALASSERT(AIModuleIsPhysical(targetModule));
        GLOBALASSERT(sbPtr->maintainVisibility);
        GLOBALASSERT(FALLP_EntryPoints);

        noOfEntryPoints = FALLP_EntryPoints[(targetModule->m_index)].numEntryPoints;
        entryPointsList = FALLP_EntryPoints[(targetModule->m_index)].entryPointsList;  
        
        GLOBALASSERT(noOfEntryPoints);

        /* just use the first entry point */
        newPosition = entryPointsList[0].position;
        
        /* now set the object's new position and current module. 
           NB this is world position + a little extra in y to make sure */
        {
                DYNAMICSBLOCK *dynPtr;
                
                dynPtr = sbPtr->DynPtr;
                dynPtr->Position = newPosition;

                dynPtr->Position.vx += targetModule->m_world.vx;
                dynPtr->Position.vy += targetModule->m_world.vy;
                dynPtr->Position.vz += targetModule->m_world.vz;

                dynPtr->PrevPosition = dynPtr->Position;
           
        }
        /* finally, update the sb's module */
        renderModule=ModuleFromPosition(&sbPtr->DynPtr->Position,NULL);

        sbPtr->containingModule = renderModule;

}




/*------------------------Patrick 14/1/97-----------------------------
  This function returns the module in which a given world-space 
  position is located.  A starting point for the search may also be
  passed, so that the following search pattern is used:
  1. is the location in the indicated starting module.
  2. is the location in any of the starting module's visible-links.
  3. finally, all modules in the environment are searched until
     a containing module is found.
  
  If no module is found, a null module pointer is returned.
  NB only 'physical' modules are returned, ie infinite and
  terminator modules are ignored.

  The function is designed to be used for objects which move over small
  distances, and for which a recently containing module is known (eg 
  avp aliens).... so that the first stage of the search will be successful
  in the majority of cases, and the third stage rarely used.  
  Note that the fn returns the first module which contains the given 
  point, so problems may arise if the point exists in more than one 
  module. This should be ok for environments like avp, where v-linking 
  is not used.
  --------------------------------------------------------------------*/

static int WorldPointIsInModule_WithTolerance(MODULE* thisModule, VECTORCH* thisPoint);
static MODULE* ModuleFromPosition_WithTolerance(VECTORCH *position, MODULE* startingModule);

MODULE* ModuleFromPosition(VECTORCH *position, MODULE* startingModule)
{
        if((startingModule) && (ModuleIsPhysical(startingModule)))
        {
                /* first test for the most trivial, and most likely case */
                if(WorldPointIsInModule(startingModule, position)) 
                        return startingModule;
                
                /* now test visible-linked modules (If there are any) */
                {
                        VMODULE *vlPtr = startingModule->m_vmptr;
                        if(vlPtr)
                        {
                                while(vlPtr->vmod_type != vmtype_term) 
                                {
                                        MODULE *mPtr = vlPtr->vmod_mref.mref_ptr;
                                        if(mPtr) 
                                        {
                                                if(ModuleIsPhysical(mPtr))
                                                        if(WorldPointIsInModule(mPtr, position)) 
                                                                return mPtr;
                                        }                       
                                        vlPtr++;
                                }
                        }
                }
        }

        /* either there is no starting module; the starting module is not physical;
        we are not in the starting module and it has no visibility-linked modules;
        or we haven't found a module yet: so search the entire module list */
        {
                extern SCENE Global_Scene;
                extern SCENEMODULE **Global_ModulePtr;
                extern int ModuleArraySize;
                MODULE **moduleListPointer;     
                int moduleCounter;

                LOCALASSERT(ModuleArraySize);
                LOCALASSERT(Global_ModulePtr);

                moduleListPointer = (Global_ModulePtr[Global_Scene])->sm_marray;
                moduleCounter = ModuleArraySize;
                while(moduleCounter>0)
                {
                        MODULE *thisModule;
                        
                        thisModule = *moduleListPointer++;
                        if(ModuleIsPhysical(thisModule))
                                if(WorldPointIsInModule(thisModule, position)) 
                                        return (thisModule);

                        moduleCounter--;
                }
        }

        /* couldn't find a module */
		/*Try with slightly larger module bounding boxes*/
        return ModuleFromPosition_WithTolerance(position,startingModule);
}


/*-----------------------Patrick 14/1/97---------------------------
Returns 1 if the given WORLD point is in a given module, 
or 0 otherwise.

NB the bizzare structure of this function produces optimum pentium
instructions... apparently.
-------------------------------------------------------------------*/
static int WorldPointIsInModule(MODULE* thisModule, VECTORCH* thisPoint)
{               
        VECTORCH localpoint = *thisPoint;
        localpoint.vx -= thisModule->m_world.vx;
        localpoint.vy -= thisModule->m_world.vy;
        localpoint.vz -= thisModule->m_world.vz;

        if(localpoint.vx <= thisModule->m_maxx)
                if(localpoint.vx >= thisModule->m_minx)
                        if(localpoint.vy <= thisModule->m_maxy)
                                if(localpoint.vy >= thisModule->m_miny)
                                        if(localpoint.vz <= thisModule->m_maxz)
                                                if(localpoint.vz >= thisModule->m_minz)
                                                        return 1;       
        return 0;
}




#define ModuleFromPositionTolerance 50
static MODULE* ModuleFromPosition_WithTolerance(VECTORCH *position, MODULE* startingModule)
{
        if((startingModule) && (ModuleIsPhysical(startingModule)))
        {
                /* first test for the most trivial, and most likely case */
                if(WorldPointIsInModule_WithTolerance(startingModule, position)) 
                        return startingModule;
                
                /* now test visible-linked modules (If there are any) */
                {
                        VMODULE *vlPtr = startingModule->m_vmptr;
                        if(vlPtr)
                        {
                                while(vlPtr->vmod_type != vmtype_term) 
                                {
                                        MODULE *mPtr = vlPtr->vmod_mref.mref_ptr;
                                        if(mPtr) 
                                        {
                                                if(ModuleIsPhysical(mPtr))
                                                        if(WorldPointIsInModule_WithTolerance(mPtr, position)) 
                                                                return mPtr;
                                        }                       
                                        vlPtr++;
                                }
                        }
                }
        }

        /* either there is no starting module; the starting module is not physical;
        we are not in the starting module and it has no visibility-linked modules;
        or we haven't found a module yet: so search the entire module list */
        {
                extern SCENE Global_Scene;
                extern SCENEMODULE **Global_ModulePtr;
                extern int ModuleArraySize;
                MODULE **moduleListPointer;     
                int moduleCounter;

                LOCALASSERT(ModuleArraySize);
                LOCALASSERT(Global_ModulePtr);

                moduleListPointer = (Global_ModulePtr[Global_Scene])->sm_marray;
                moduleCounter = ModuleArraySize;
                while(moduleCounter>0)
                {
                        MODULE *thisModule;
                        
                        thisModule = *moduleListPointer++;
                        if(ModuleIsPhysical(thisModule))
                                if(WorldPointIsInModule_WithTolerance(thisModule, position)) 
                                        return (thisModule);

                        moduleCounter--;
                }
        }

        /* couldn't find a module */
        return (MODULE *)0;
}


static int WorldPointIsInModule_WithTolerance(MODULE* thisModule, VECTORCH* thisPoint)
{               
        VECTORCH localpoint = *thisPoint;
        localpoint.vx -= thisModule->m_world.vx;
        localpoint.vy -= thisModule->m_world.vy;
        localpoint.vz -= thisModule->m_world.vz;

        if(localpoint.vx <= thisModule->m_maxx + ModuleFromPositionTolerance)
                if(localpoint.vx >= thisModule->m_minx - ModuleFromPositionTolerance)
                        if(localpoint.vy <= thisModule->m_maxy + ModuleFromPositionTolerance)
                                if(localpoint.vy >= thisModule->m_miny - ModuleFromPositionTolerance)
                                        if(localpoint.vz <= thisModule->m_maxz + ModuleFromPositionTolerance)
                                                if(localpoint.vz >= thisModule->m_minz - ModuleFromPositionTolerance)
                                                        return 1;       
        return 0;
}



/*---------------------Patrick 14/1/97-----------------------------
  
                SUPPORT FUNCTIONS FOR INANIMATE OBJECTS
  
  -----------------------------------------------------------------*/






/*-----------------------Patrick 16/1/97---------------------------
Inanimate object initialisation and behaviour and functions.
-------------------------------------------------------------------*/
void InitInanimateObject(void* bhdata, STRATEGYBLOCK *sbPtr)
{
        TOOLS_DATA_INANIMATEOBJECT *toolsData = (TOOLS_DATA_INANIMATEOBJECT *)bhdata;
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr;
        enum DYNAMICS_TEMPLATE_ID inanimateDynamicsInitialiser;
        int i;

        LOCALASSERT(sbPtr->I_SBtype == I_BehaviourInanimateObject);
        LOCALASSERT(toolsData);

        /* create, initialise and attach a data block */
        objectstatusptr = (void *)AllocateMem(sizeof(INANIMATEOBJECT_STATUSBLOCK));
        if(!objectstatusptr)
        {
                RemoveBehaviourStrategy(sbPtr);
        }

        sbPtr->SBdataptr = objectstatusptr;
        objectstatusptr->respawnTimer = 0; 
        objectstatusptr->lifespanTimer = 0; 
        objectstatusptr->explosionTimer = 0; 
                        
        /* these should be loaded */
        objectstatusptr->typeId = toolsData->typeId;
        objectstatusptr->subType = toolsData->subType;                                  
        
        /* set default indestructibility */
        objectstatusptr->Indestructable = No;
        objectstatusptr->ghosted_object=0;

        /* set the default inanimate object dynamics template: Inanimate for single player,
        and Static for multiplayer 
        NB some objects are always static, and initialised using
        the static dynamics template directly */
//      if(AvP.Network==I_No_Network) inanimateDynamicsInitialiser = DYNAMICS_TEMPLATE_INANIMATE;
//      else inanimateDynamicsInitialiser = DYNAMICS_TEMPLATE_STATIC;
        inanimateDynamicsInitialiser = DYNAMICS_TEMPLATE_INANIMATE;
        
        /* Initialise object's stats */
        {
                NPC_DATA *NpcData;

                //#warning Change Me!!!

                NpcData=GetThisNpcData(I_NPC_DefaultInanimate);
                LOCALASSERT(NpcData);
                sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
                sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
        }
        
        sbPtr->SBDamageBlock.Health*=toolsData->integrity;

        if(toolsData->triggering_event)
        {
                objectstatusptr->event_target=(OBJECT_EVENT_TARGET*)AllocateMem(sizeof(OBJECT_EVENT_TARGET));

                objectstatusptr->event_target->triggering_event=toolsData->triggering_event;
                objectstatusptr->event_target->request=toolsData->event_request;
                for(i=0;i<SB_NAME_LENGTH;i++) objectstatusptr->event_target->event_target_ID[i] = toolsData->event_target_ID[i];
                objectstatusptr->event_target->event_target_sbptr=0;
        }
        else
        {
                objectstatusptr->event_target=0;
        }

        objectstatusptr->explosionType=toolsData->explosionType;

        /* set inanimate-object-type-specific stuff... 
        1. dynamics block allocation
        2. integrity */
        switch(objectstatusptr->typeId)
        {
                case IOT_HitMeAndIllDestroyBase:
                case IOT_Static:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->DynPtr->Mass = toolsData->mass;
                        if (toolsData->integrity > 20)
                        {
                                objectstatusptr->Indestructable = Yes;
                                sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        }
                        else if (toolsData->integrity < 1)
                        {
                                sbPtr->integrity = 1; // die immediately
                        }
                        else
                        {
                                sbPtr->integrity = (DEFAULT_OBJECT_INTEGRITY)*(toolsData->integrity);
                        }
                        break;
                }                               
                case IOT_Furniture:     
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(inanimateDynamicsInitialiser);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->DynPtr->Mass = toolsData->mass;
                        if (toolsData->integrity > 20)
                        {
                                objectstatusptr->Indestructable = Yes;
                                sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        }
                        else if (toolsData->integrity < 1)
                        {
                                sbPtr->integrity = 1; // die immediately
                        }
                        else
                        {
                                sbPtr->integrity = (DEFAULT_OBJECT_INTEGRITY)*(toolsData->integrity);
                        }
                        break;
                }                               
                case IOT_Weapon:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
						if(objectstatusptr->subType==WEAPON_FLAMETHROWER)
						{
							//flamethrowers explode using a molotov explosion
							objectstatusptr->explosionType=3;							
						}
                        break;
                }                               
                case IOT_Ammo:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;

						if(objectstatusptr->subType==AMMO_FLAMETHROWER)
						{
							//flamethrowers explode using a molotov explosion
							objectstatusptr->explosionType=3;							
						}
                        break;
                }                               
                case IOT_Health:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }

                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_Armour:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }

                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_Key:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }

                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_BoxedSentryGun:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(inanimateDynamicsInitialiser);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_IRGoggles:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(inanimateDynamicsInitialiser);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_DataTape:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }
        case IOT_MTrackerUpgrade:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                case IOT_PheromonePod:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }
                case IOT_SpecialPickupObject:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }
                        if (toolsData->integrity > 20)
                        {
                                objectstatusptr->Indestructable = Yes;
                                sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        }
                        else if (toolsData->integrity < 1)
                        {
                                sbPtr->integrity = 1; // die immediately
                        }
                        else
                        {
                                sbPtr->integrity = (DEFAULT_OBJECT_INTEGRITY)*(toolsData->integrity);
                        }
                        break;
                }
                case IOT_FieldCharge:
                {
                        sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_PICKUPOBJECT);
                        if(!sbPtr->DynPtr)
                        {
                                RemoveBehaviourStrategy(sbPtr);
                                return;
                        }

                        sbPtr->integrity = DEFAULT_OBJECT_INTEGRITY;
                        break;
                }                               
                default:
                {
                        LOCALASSERT(1==0);                              
                        break;
                }
        }
        /*check to see if object is animated.*/
        {
                TXACTRLBLK **pptxactrlblk;              
                int item_num;
                int shape_num = toolsData->shapeIndex;
                SHAPEHEADER *shptr = GetShapeData(shape_num);
                pptxactrlblk = &objectstatusptr->inan_tac;
                for(item_num = 0; item_num < shptr->numitems; item_num ++)
                {
                        POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
                        LOCALASSERT(poly);

                        SetupPolygonFlagAccessForShape(shptr);
                                
                        if((Request_PolyFlags((void *)poly)) & iflag_txanim)
                                {
                                        TXACTRLBLK *pnew_txactrlblk;

                                        pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
                                        if(pnew_txactrlblk)
                                        {
                                                pnew_txactrlblk->tac_flags = 0;                                                                         
                                                pnew_txactrlblk->tac_item = item_num;                                                                           
                                                pnew_txactrlblk->tac_sequence = 0;                                                                              
                                                pnew_txactrlblk->tac_node = 0;                                                                          
                                                pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);                                                                            
                                                pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

                                                *pptxactrlblk = pnew_txactrlblk;
                                                pptxactrlblk = &pnew_txactrlblk->tac_next;
                                        }
                                        else *pptxactrlblk = NULL; 
                                }
                }
                *pptxactrlblk=0;

        }

        /* Initialise the dynamics block */
        {
                DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
                GLOBALASSERT(dynPtr);
        
        dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
                dynPtr->OrientEuler = toolsData->orientation;
                CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
                TransposeMatrixCH(&dynPtr->OrientMat);      
        }

        /* strategy block initialisation */
        sbPtr->shapeIndex = toolsData->shapeIndex;
        for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];

        /* these must be initialised for respawning objects in multiplayer game */
        objectstatusptr->startingHealth = sbPtr->SBDamageBlock.Health;
        objectstatusptr->startingArmour = sbPtr->SBDamageBlock.Armour;

		if(AvP.Network != I_No_Network)
		{
			//if in a network game , see if this is an allowable weapon
			if(objectstatusptr->typeId==IOT_Weapon)
			{
				if((!netGameData.allowSmartgun && objectstatusptr->subType==WEAPON_SMARTGUN)||
				   (!netGameData.allowFlamer && objectstatusptr->subType==WEAPON_FLAMETHROWER)||	
				   (!netGameData.allowSadar && objectstatusptr->subType==WEAPON_SADAR)||	
				   (!netGameData.allowGrenadeLauncher && objectstatusptr->subType==WEAPON_GRENADELAUNCHER)||	
				   (!netGameData.allowPistols && objectstatusptr->subType==WEAPON_MARINE_PISTOL)||	
				   (!netGameData.allowSmartDisc && objectstatusptr->subType==WEAPON_FRISBEE_LAUNCHER)||
				   (!netGameData.allowMinigun && objectstatusptr->subType==WEAPON_MINIGUN))
				{
                	RemoveBehaviourStrategy(sbPtr);
                	return;
				}	
			}
			if(objectstatusptr->typeId==IOT_Ammo)
			{
				if((!netGameData.allowSmartgun && objectstatusptr->subType==AMMO_SMARTGUN)||
				   (!netGameData.allowFlamer && objectstatusptr->subType==AMMO_FLAMETHROWER)||	
				   (!netGameData.allowSadar && objectstatusptr->subType==AMMO_SADAR_TOW)||	
				   (!netGameData.allowGrenadeLauncher && objectstatusptr->subType==AMMO_GRENADE)||	
				   (!netGameData.allowPistols && objectstatusptr->subType==AMMO_MARINE_PISTOL_PC)||	
				   (!netGameData.allowSmartDisc && objectstatusptr->subType==AMMO_FRISBEE)||
				   (!netGameData.allowMinigun && objectstatusptr->subType==AMMO_MINIGUN))
				{
                	RemoveBehaviourStrategy(sbPtr);
                	return;
				}	
			}
		}
}

void InanimateObjectBehaviour(STRATEGYBLOCK *sbPtr)
{               
        /* handle respawn timer here */
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbPtr->SBdataptr;
        LOCALASSERT(objectstatusptr);

        LOCALASSERT(!(objectstatusptr->respawnTimer<0)); /* this should never happen */
        
		/* Lock disc pickups in place. */
		if ((objectstatusptr->typeId == IOT_Ammo)
			&&(objectstatusptr->subType == (int)AMMO_PRED_DISC)) {
		
			sbPtr->DynPtr->LinVelocity.vx=0;
			sbPtr->DynPtr->LinVelocity.vy=0;
			sbPtr->DynPtr->LinVelocity.vz=0;

			sbPtr->DynPtr->LinImpulse.vx=0;
			sbPtr->DynPtr->LinImpulse.vy=0;
			sbPtr->DynPtr->LinImpulse.vz=0;
		}
		
        if(objectstatusptr->inan_tac)
        {
                DISPLAYBLOCK* dptr = sbPtr->SBdptr;

                /*deal with texture animation*/
                if(dptr)
                {
                        if(!dptr->ObTxAnimCtrlBlks)
                                { 
                                        dptr->ObTxAnimCtrlBlks = objectstatusptr->inan_tac;
                                }
                }
                
        }

        if(objectstatusptr->explosionTimer)
        {
                //the object is about to explode
                if(objectstatusptr->explosionStartFrame==GlobalFrameCounter)
                {
                        //the explosion was triggered earlier this frame
                        //therefore don't bother altering the timer until next frame
                        return;
                }

                objectstatusptr->explosionTimer-=NormalFrameTime;
                if(objectstatusptr->explosionTimer<=0)
                {
                        objectstatusptr->explosionTimer=-1;
                        InanimateObjectIsDamaged(sbPtr,0,0);
                        return;
                }

        }

        if(AvP.Network!=I_No_Network)
        {
                //see if dropped weapons have timed out
                if(objectstatusptr->lifespanTimer>0)
                {
                        objectstatusptr->lifespanTimer-=NormalFrameTime;
                        if(objectstatusptr->lifespanTimer<=0)
                        {
                                FragmentInanimateObject(sbPtr);
                                DestroyAnyStrategyBlock(sbPtr);
                        }
                        return;
                }
        }

        /* do nothing if the timer is zero */
        if(objectstatusptr->respawnTimer==0 || objectstatusptr->respawnTimer==OBJECT_RESPAWN_NO_RESPAWN) return;

        /* textprint("RESPAWN TIMER %d \n", objectstatusptr->respawnTimer); */          
        /* If we get here, the object is in a respawn state:
        this should only happen in a net-game */
        LOCALASSERT(AvP.Network!=I_No_Network);
        LOCALASSERT(!sbPtr->SBdptr);
        LOCALASSERT(!sbPtr->maintainVisibility);

        objectstatusptr->respawnTimer -= NormalFrameTime;
        if(objectstatusptr->respawnTimer<=0)
        {
                /* time to respawn, then */
                RespawnInanimateObject(sbPtr);
                objectstatusptr->respawnTimer=0;        
        }               
}

/* this global flag is used to distinguish between messages from the host, 
and locally caused damage */
int InanimateDamageFromNetHost = 0;


void InanimateObjectIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbPtr->SBdataptr;
        LOCALASSERT(objectstatusptr);

        if((AvP.Network==I_Peer)&&(!InanimateDamageFromNetHost))
        {
                /* this means that the damage was generated locally in a net-game:
                in this case, just send a damage message to the host */
                AddNetMsg_InanimateObjectDamaged(sbPtr,damage,multiple);
                return;
        }
        else if(AvP.Network==I_Host) 
        {
                /* if we're the host, inform everyone that the object is dead */
                if(sbPtr->SBDamageBlock.Health <= 0) AddNetMsg_InanimateObjectDestroyed(sbPtr);
        }

		if(sbPtr->SBflags.please_destroy_me)
		{
			//object has already been destroyed , so ignore any further damage
			return;
		}
        
        //if object has been destroyed , see if it has a target that it needs to notify
        if(objectstatusptr->event_target)
        {
                if(sbPtr->SBDamageBlock.Health <= 0 && !objectstatusptr->Indestructable) 
                {
                        if(objectstatusptr->event_target->triggering_event & ObjectEventFlag_Destroyed)
                        {
                                RequestState(objectstatusptr->event_target->event_target_sbptr,objectstatusptr->event_target->request,0);
                        }
                }
        }
                
        if (!objectstatusptr->Indestructable && objectstatusptr->explosionType)
        {		

        
        	if(InanimateDamageFromNetHost)
			{
        		//do explosion effect , without the damage
        		switch(objectstatusptr->explosionType)
				{
					case 1:
					{
        		        //small explosion
						MakeVolumetricExplosionAt(&(sbPtr->DynPtr->Position),EXPLOSION_SMALL_NOCOLLISIONS);

						Explosion_SoundData.position=sbPtr->DynPtr->Position;
	   		
						Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
						
						break;
					}
					case 2:
					{
        		        //big explosion
						MakeVolumetricExplosionAt(&(sbPtr->DynPtr->Position),EXPLOSION_HUGE_NOCOLLISIONS);

						Explosion_SoundData.position=sbPtr->DynPtr->Position;
						
						Sound_Play(SID_NICE_EXPLOSION,"n",&Explosion_SoundData);
						
						break;
					}
					case 3:
        		    {
        		        //molotov explosion
						MakeVolumetricExplosionAt(&(sbPtr->DynPtr->Position),EXPLOSION_MOLOTOV);

						Explosion_SoundData.position=sbPtr->DynPtr->Position;
						
						Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
						
						break;
					}
				}
        	}
			else
			{                
        		if(objectstatusptr->explosionTimer==0)  
        		{
        		        if(sbPtr->SBDamageBlock.Health <= 0)
        		        {
        		                //start explosion timer
        		                //this gives a slight time delay for object destruction, to allow for chain reactions
        		                //(rather than all explosive objects going up in one frame)
        		                objectstatusptr->explosionTimer=ONE_FIXED/10;
        		                objectstatusptr->explosionStartFrame=GlobalFrameCounter;
        		                return;
        		        }
        		}
        		else if(objectstatusptr->explosionTimer<0)      
        		{
        		        //time for the explosion
        		        objectstatusptr->explosionTimer=0;
        		        switch(objectstatusptr->explosionType)
						{
							case 1:
							{
        		                //small explosion
        		                HandleEffectsOfExplosion
        		                (
        		                        sbPtr,
        		                        &(sbPtr->DynPtr->Position),
        		                        5000,
        		                        &SmallExplosionDamage,
        		                        0
        		                );
								Explosion_SoundData.position=sbPtr->DynPtr->Position;
								
								Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
								
								break;
							}
							case 2:
							{
        		                //big explosion
        		                HandleEffectsOfExplosion
        		                (
        		                        sbPtr,
        		                        &(sbPtr->DynPtr->Position),
        		                        10000,
        		                        &BigExplosionDamage,
        		                        0
        		                );
								Explosion_SoundData.position=sbPtr->DynPtr->Position;

								Sound_Play(SID_NICE_EXPLOSION,"n",&Explosion_SoundData);
								
								break;
							}
							case 3:
        		            {
        		                //molotov explosion
        		                HandleEffectsOfExplosion
        		                (
        		                        sbPtr,
        		                        &(sbPtr->DynPtr->Position),
										TemplateAmmo[AMMO_MOLOTOV].MaxRange,
								 		&TemplateAmmo[AMMO_MOLOTOV].MaxDamage[AvP.Difficulty],
										TemplateAmmo[AMMO_MOLOTOV].ExplosionIsFlat
        		                );
								Explosion_SoundData.position=sbPtr->DynPtr->Position;

								Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
								
								break;
							}
						}

        		        //set health to zero (in case object has recovered in the delay)
        		        sbPtr->SBDamageBlock.Health=0;
        		}
        		else
        		{
        		        //currently in explosion count down , so wait
        		        return;
        		}
			}
        }
		

        switch(objectstatusptr->typeId)
        {
                case IOT_Static:
                {
                        if (!objectstatusptr->Indestructable)
                        {
                                if(sbPtr->SBDamageBlock.Health <= 0) 
                                {
                                        FragmentInanimateObject(sbPtr);
                                        if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                        else KillFragmentalObjectForRespawn(sbPtr);                     
                                }
                        }
                        break;
                }                               
                case IOT_Furniture:     
                {
                        if (!objectstatusptr->Indestructable)
                        {
                                if(sbPtr->SBDamageBlock.Health <= 0)
                                {
                                        FragmentInanimateObject(sbPtr);
                                        DestroyAnyStrategyBlock(sbPtr);
                                }
                        }
                        break;
                }                               
                case IOT_Weapon:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                FragmentInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_Ammo:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0)
                        {
                                FragmentInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_Health:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                FragmentInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_Armour:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                FragmentInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_Key:
                {
                        /* do nothing */
                        break;
                }                               
                case IOT_BoxedSentryGun:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                ExplodeInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_IRGoggles:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                ExplodeInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_DataTape:
                {
                        /* do nothing */
                        break;
                }
        case IOT_MTrackerUpgrade:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                ExplodeInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_PheromonePod:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                ExplodeInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                case IOT_SpecialPickupObject:
                {
                        if (!objectstatusptr->Indestructable)
						{
                        	if(sbPtr->SBDamageBlock.Health <= 0) 
                        	{
                        	        FragmentInanimateObject(sbPtr);
                        	        if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                        	        else KillInanimateObjectForRespawn(sbPtr);                      
                        	}
                        }
                        break;
                }
                case IOT_HitMeAndIllDestroyBase:
                {
                        if (damage->Penetrative>0) {
                                /* Destroy base. */
                                if (AvP.DestructTimer==-1) {
                                        textprint("Boom!  You've blown up the base!\n");
                                        ActivateSelfDestructSequence(60);
                                        /* Dummy switch hack. */
                                        {
                                                STRATEGYBLOCK *evil_switch_SBPtr;
                                                char Evil_Switch_SBname[] = {0x46,0xb9,0x01,0xf0,0x63,0xe4,0x56,0x0,};
                                        
                                                evil_switch_SBPtr=FindSBWithName(Evil_Switch_SBname);
                                        
                                                GLOBALASSERT(evil_switch_SBPtr);

                                                RequestState(evil_switch_SBPtr,1,Player->ObStrategyBlock);
                                        }
                                }
                        }
                        break;
                }
                case IOT_FieldCharge:
                {
                        if(sbPtr->SBDamageBlock.Health <= 0) 
                        {
                                FragmentInanimateObject(sbPtr);
                                if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
                                else KillInanimateObjectForRespawn(sbPtr);                      
                        }
                        break;
                }                               
                default:
                {
                        LOCALASSERT(1==0);                              
                        break;
                }
        }
}

#define ObjectRequest_AdjustIntegrity 0x00000001
void SendRequestToInanimateObject(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbptr->SBdataptr;
        GLOBALASSERT(objectstatusptr);
        GLOBALASSERT((sbptr->I_SBtype == I_BehaviourInanimateObject));


        if(state)
        {
                if(extended_data & ObjectRequest_AdjustIntegrity)
                {
                        int new_integrity=(extended_data>>7)&0xff;
                        sbptr->SBDamageBlock.Health=(10<<ONE_FIXED_SHIFT)*new_integrity;        
                        sbptr->integrity = DEFAULT_OBJECT_INTEGRITY*new_integrity;

                        if(new_integrity>20)
                                objectstatusptr->Indestructable = Yes;
                        else
                                objectstatusptr->Indestructable = No;
                        
                        if(sbptr->integrity==0)
                        {
                                //destroy the object by applying some damage to it
                                InanimateObjectIsDamaged(sbptr, &certainDeath, ONE_FIXED);
                        }
                }
        }

}

static void FragmentInanimateObject(STRATEGYBLOCK *sbPtr)
{
        MakeFragments(sbPtr);
}

static void ExplodeInanimateObject(STRATEGYBLOCK *sbPtr)
{
        Sound_Play(SID_EXPLOSION,"d",&(sbPtr->DynPtr->Position));
}

/*-------------------Patrick 30/4/96-----------------------
  A couple of functions for     supporting respawning objects 
  in network games...
  ---------------------------------------------------------*/
static void RespawnInanimateObject(STRATEGYBLOCK *sbPtr)
{
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbPtr->SBdataptr;
        LOCALASSERT(objectstatusptr);
        
        sbPtr->maintainVisibility = 1;
        MakeObjectNear(sbPtr);
        
        /* must respawn health too... */        
        sbPtr->SBDamageBlock.Health = objectstatusptr->startingHealth;
        sbPtr->SBDamageBlock.Armour = objectstatusptr->startingArmour;

}

void KillInanimateObjectForRespawn(STRATEGYBLOCK *sbPtr)
{
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbPtr->SBdataptr;
        LOCALASSERT(objectstatusptr);
        LOCALASSERT(AvP.Network!=I_No_Network);
        
        /* make the object invisible, and remove it from visibility management */
        if(!objectstatusptr->lifespanTimer)
        {
                sbPtr->maintainVisibility = 0;
                if(sbPtr->SBdptr) MakeObjectFar(sbPtr);

				if(netGameData.timeForRespawn>0)
                	objectstatusptr->respawnTimer = netGameData.timeForRespawn*ONE_FIXED;
				else
                	objectstatusptr->respawnTimer = OBJECT_RESPAWN_NO_RESPAWN;
        }
        else
        {
                //get rid of this object permanently
                DestroyAnyStrategyBlock(sbPtr);

        }

}

void KillFragmentalObjectForRespawn(STRATEGYBLOCK *sbPtr)
{
        INANIMATEOBJECT_STATUSBLOCK* objectstatusptr = sbPtr->SBdataptr;
        LOCALASSERT(objectstatusptr);
        LOCALASSERT(AvP.Network!=I_No_Network);
        
        /* make the object invisible, and remove it from visibility management */
        sbPtr->maintainVisibility = 0;
        if(sbPtr->SBdptr) MakeObjectFar(sbPtr);

        /* KJL 12:44:23 24/05/98 -
        
                Set the respawn counter to be the max allowable, so that if all the
        respawn counters are set to zero, it forces all the objects that have been
        destroyed (eg. glass) to respawn simultaneously.

        Not a perfect solution: the object will respawn in 9.1 hours on its own. */
        
        objectstatusptr->respawnTimer = OBJECT_RESPAWN_NO_RESPAWN;

}

void RespawnAllObjects(void)
{
        int i;

        LOCALASSERT(AvP.Network!=I_No_Network);

        for (i=0; i<NumActiveStBlocks; i++)
        {
                STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

                if(sbPtr->I_SBtype == I_BehaviourInanimateObject)
                {
                        INANIMATEOBJECT_STATUSBLOCK* objectStatusPtr = sbPtr->SBdataptr;
                        LOCALASSERT(objectStatusPtr);
                        
                        if(objectStatusPtr->respawnTimer!=0)
                        {
                                RespawnInanimateObject(sbPtr);
                                objectStatusPtr->respawnTimer=0;        
                        }
                }
                else if(sbPtr->I_SBtype == I_BehaviourPlacedLight)
                {
                        RespawnLight(sbPtr);
                }
        }               
}

void RespawnAllPickups(void)
{
	int i;

	LOCALASSERT(AvP.Network!=I_No_Network);

	for (i=0; i<NumActiveStBlocks; i++)
	{
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

		if(sbPtr->I_SBtype == I_BehaviourInanimateObject)
		{
			INANIMATEOBJECT_STATUSBLOCK* objectStatusPtr = sbPtr->SBdataptr;
			LOCALASSERT(objectStatusPtr);

			if(objectStatusPtr->typeId==IOT_Weapon ||
			   objectStatusPtr->typeId==IOT_Ammo ||
			   objectStatusPtr->typeId==IOT_Health ||
			   objectStatusPtr->typeId==IOT_Armour ||
			   objectStatusPtr->typeId==IOT_FieldCharge)
			{
				if(objectStatusPtr->respawnTimer!=0)
				{
					RespawnInanimateObject(sbPtr);
					objectStatusPtr->respawnTimer=0;        
				}
			}
		}
	}               
}

void IdentifyObject(STRATEGYBLOCK *sbPtr) {
        
        if (sbPtr==NULL) {
                textprint("Object is NULL!\n");
                return;
        }

        if (sbPtr->I_SBtype == I_BehaviourInanimateObject) {
                INANIMATEOBJECT_STATUSBLOCK* objStatPtr = sbPtr->SBdataptr;
        
                switch(objStatPtr->typeId)
                {
                        case(IOT_Weapon):
                        {
                                switch(objStatPtr->subType)
                                {
                                        case WEAPON_PULSERIFLE:
                                        {
                                                textprint("Object is a Pulse Rifle.\n");
                                                return;
                                        }
                                        case WEAPON_AUTOSHOTGUN:
                                        {
                                                textprint("Object is an Autoshotgun.\n");
                                                return;
                                        }
                                        case WEAPON_SMARTGUN:
                                        {
                                                textprint("Object is a Smartgun.\n");
                                                return;
                                        }
                                        case WEAPON_FLAMETHROWER:
                                        {
                                                textprint("Object is a Flamethrower.\n");
                                                return;
                                        }
                                        case WEAPON_PLASMAGUN:
                                        {
                                                textprint("Object is a Plasmagun!\n");
                                                return;
                                        }
                                        case WEAPON_SADAR:
                                        {
                                                textprint("Object is a Sadar.\n");
                                                return;
                                        }
                                        case WEAPON_GRENADELAUNCHER:
                                        {
                                                textprint("Object is a Grenade Launcher.\n");
                                                return;
                                        }
                                        case WEAPON_MINIGUN:
                                        {
                                                textprint("Object is a Minigun.\n");
                                                return;
                                        }
        
                                        default:
                                                textprint("Object is unknown weapon (subtype %d).\n",(int)objStatPtr->subType);
                                                break;
                                }
                                break;
                        }
                        case(IOT_Ammo):
                        {
                                switch(objStatPtr->subType)
                                {
                                        case AMMO_10MM_CULW:
                                        {
                                                textprint("Object is Pulse Rifle ammo.\n");
                                                break;
                                        }
                                        case AMMO_SHOTGUN:
                                        {
                                                textprint("Object is Shotgun ammo.\n");
                                                break;
                                        }
                                        case AMMO_SMARTGUN:
                                        {
                                                textprint("Object is Smartgun ammo.\n");
                                                break;
                                        }
                                        case AMMO_FLAMETHROWER:
                                        {
                                                textprint("Object is Flamethrower ammo.\n");
                                                break;
                                        }
                                        case AMMO_PLASMA:
                                        {
                                                textprint("Object is Plasmagun ammo!\n");
                                                break;
                                        }
                                        case AMMO_SADAR_TOW:
                                        {
                                                textprint("Object is Sadar ammo.\n");
                                                break;
                                        }
                                        case AMMO_GRENADE: 
                                        {
                                                textprint("Object is Grenade Launcher ammo.\n");
                                                break;
                                        }
                                        case AMMO_MINIGUN:
                                        {
                                                textprint("Object is Minigun ammo.\n");
                                                break;
                                        }
                                        default:
                                                textprint("Object is unlisted ammo (subtype %d).\n",(int)objStatPtr->subType);
                                                break;
                                }
                                
                                break;
                        }
                        case(IOT_Health):
                        {
                                textprint("Object is health.\n");
                                break;
                        }
                        case(IOT_Armour):
                        {
                                textprint("Object is armour.\n");
                                break;
                        }
                        case(IOT_FieldCharge):
                        {
                                textprint("Object is field charge powerup.\n");
                                break;
                        }
                        default:
                        {
                                textprint("Object is unlisted subtype (%d).\n",(int)objStatPtr->subType);
                                break;
                        }
                }
        } else {
                switch(sbPtr->I_SBtype) {
                        case I_BehaviourMarine:
                        {
                                textprint("Object is a marine.\n");
                                textprint("Marine is in %s\n",sbPtr->containingModule->name);
                                break;
                        }
                        case I_BehaviourAlien:
                        {
                                textprint("Object is an alien.\n");
                                break;
                        }
                        case I_BehaviourPredator:
                        {
                                textprint("Object is a predator.\n");
                                break;
                        }
                        default:
                        {
                                textprint("Object is not supported - change PVisible.c for expanded diagnostics!\n");
                                break;
                        }
                }
        }

        if (!sbPtr->DynPtr) {
                textprint("Object has no dynamics block!\n");
        } else {
                textprint("Object world co-ords: %d %d %d\n",sbPtr->DynPtr->Position.vx,sbPtr->DynPtr->Position.vy,sbPtr->DynPtr->Position.vz);
        }

}



STRATEGYBLOCK* CreateMultiplayerWeaponPickup(VECTORCH* location,int type,char* name)
{
	TOOLS_DATA_INANIMATEOBJECT toolsdata;
	STRATEGYBLOCK * sbPtr;
	INANIMATEOBJECT_STATUSBLOCK* objectstatusptr;
	int trueType;
	
	if ((type==WEAPON_TWO_PISTOLS)||(type==WEAPON_MARINE_PISTOL)) {
		if (!netGameData.allowPistols) {
			return(0);
		}
	}

	if (type==WEAPON_TWO_PISTOLS) {
		trueType=WEAPON_MARINE_PISTOL;
	} else {
		trueType=type;
	}

	//fill out a tools template , and use the normal inanimate object creation function
	toolsdata.position=*location;
	toolsdata.orientation.EulerX=0;
	toolsdata.orientation.EulerY=0;
	toolsdata.orientation.EulerZ=0;

	toolsdata.typeId=IOT_Weapon;
	toolsdata.subType=trueType;
	toolsdata.mass=5;
	toolsdata.integrity=2;
	toolsdata.triggering_event=0;
	toolsdata.explosionType=0;

	toolsdata.shapeIndex=-1;
	
	//find the weapon's shape
	switch(trueType)
	{
		case WEAPON_MARINE_PISTOL:
			toolsdata.shapeIndex=GetLoadedShapeMSL("Pistol");
			break;
		case WEAPON_PULSERIFLE:
			toolsdata.shapeIndex=GetLoadedShapeMSL("pulse");
			break;
		case WEAPON_SMARTGUN:
			toolsdata.shapeIndex=GetLoadedShapeMSL("smart");
			break;
		case WEAPON_FLAMETHROWER:
			toolsdata.shapeIndex=GetLoadedShapeMSL("flame");
			break;
		case WEAPON_FRISBEE_LAUNCHER:
			toolsdata.shapeIndex=GetLoadedShapeMSL("Skeeter");
			toolsdata.orientation.EulerZ=1024;
			break;
		case WEAPON_SADAR:
			toolsdata.shapeIndex=GetLoadedShapeMSL("sadar");
			break;
		case WEAPON_GRENADELAUNCHER:
			toolsdata.shapeIndex=GetLoadedShapeMSL("grenade");
			break;
		case WEAPON_MINIGUN:
			toolsdata.shapeIndex=GetLoadedShapeMSL("minigun");
			break;
		
	}

	if(toolsdata.shapeIndex==-1)
	{
		//failed to find shape
		//forget about it
		return 0;
	}
	//adjust the weapon , so it isn't stuck through the floor
	toolsdata.position.vy-=(mainshapelist[toolsdata.shapeIndex]->shapemaxy);
	


	sbPtr = CreateActiveStrategyBlock();

	InitialiseSBValues(sbPtr);
	if(!sbPtr) return 0;

	sbPtr->I_SBtype = I_BehaviourInanimateObject;
	sbPtr->shapeIndex=toolsdata.shapeIndex;
	
	EnableBehaviourType(sbPtr,I_BehaviourInanimateObject, &toolsdata );
	if(!sbPtr->SBdataptr) return 0;
	
	if(!name)
	{
		AssignNewSBName(sbPtr);
		AddNetMsg_CreateWeapon(&sbPtr->SBname[0],trueType,location);
	}
	else
	{
		COPY_NAME(sbPtr->SBname,name);  
	}

	objectstatusptr=(INANIMATEOBJECT_STATUSBLOCK*)sbPtr->SBdataptr;
	
	objectstatusptr->lifespanTimer=20*ONE_FIXED;
	
	//sort out object visibility
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);
		
		sbPtr->containingModule = ModuleFromPosition(&(dynPtr->Position), (MODULE *)0);
		sbPtr->maintainVisibility = 1;
		
	
		dynPtr->GravityOn = 1;

	}

	return sbPtr;
}

void MakePlayersWeaponPickupVisible()
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	STRATEGYBLOCK* sbPtr;
	int i;
	/*
	Search through the strategy block list for weapons.
	Any weapons that have a lifespan timer should be made visible.
	There should only be one such object , so we can stop looking once we find it
	*/

	for(i=NumActiveStBlocks-1;i>=0;i--)
	{
		sbPtr=ActiveStBlockList[i];
		if(sbPtr->I_SBtype==I_BehaviourInanimateObject && !sbPtr->maintainVisibility)
		{
			INANIMATEOBJECT_STATUSBLOCK* objectstatusptr;
			objectstatusptr=(INANIMATEOBJECT_STATUSBLOCK*)sbPtr->SBdataptr;

			if(objectstatusptr->typeId==IOT_Weapon)
			{
				if(objectstatusptr->lifespanTimer>0)
				{
					//okay we've found the object , so allow it to be visible
					sbPtr->maintainVisibility = 1;
					return;
				}	
			}
		}
	}
	
}


STRATEGYBLOCK* Create_Pred_Disc_Pickup_For_Load()
{
	STRATEGYBLOCK* sbPtr;
	TOOLS_DATA_INANIMATEOBJECT toolsData;
	int discShapeIndex;

	{
		extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
		SECTION* section;
		/*
		We need to find the shape index of the disc object. To do this , we need to
		get the disc head-up-display hierarchy , and search for the disc section
		*/
		section = GetNamedHierarchyFromLibrary("pred_HUD","disk");
		if(!section) return NULL;

		section = GetThisSection(section,"disk");
		if(!section) return NULL;

		//found the disc section
		discShapeIndex = section->ShapeNum;

	}
	
	//fill in a tools data for the disc
	memset(&toolsData,0,sizeof(toolsData));
	toolsData.typeId = IOT_Ammo;
	toolsData.subType = (int)AMMO_PRED_DISC;
	toolsData.shapeIndex = discShapeIndex;

	
	sbPtr = CreateActiveStrategyBlock();

	if(!sbPtr)
	{
		return NULL;
	}

	InitialiseSBValues(sbPtr);
	sbPtr->I_SBtype = I_BehaviourInanimateObject;
	sbPtr->shapeIndex = discShapeIndex;
			
	EnableBehaviourType(sbPtr,I_BehaviourInanimateObject, &toolsData );
    sbPtr->maintainVisibility = 1;

	return sbPtr;

}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct inanimate_object_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	INANIMATEOBJECT_TYPE typeId;
	int subType; /* weapon id, security level or other relevant enumerated type... */

	BOOL Indestructable;
	int explosionTimer; //slight time delay after destruction for explosion

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}INANIMATE_OBJECT_SAVE_BLOCK;


//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV objectstatusptr

void LoadStrategy_InanimateObject(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	INANIMATEOBJECT_STATUSBLOCK* objectstatusptr;
	INANIMATE_OBJECT_SAVE_BLOCK* block = (INANIMATE_OBJECT_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr)
	{
		//Didn't find the object already existing.
		//Might be a predator disc pickup however.
		if(block->typeId == IOT_Ammo && block->subType == (int)AMMO_PRED_DISC)
		{
			//okay we need to create a disc then
			sbPtr = Create_Pred_Disc_Pickup_For_Load();
		}
		if(!sbPtr) return;

	}

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourInanimateObject) return;

	objectstatusptr = (INANIMATEOBJECT_STATUSBLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(Indestructable)
	COPYELEMENT_LOAD(explosionTimer)
	COPYELEMENT_LOAD(typeId)
	COPYELEMENT_LOAD(subType)
	
	

	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

}


void SaveStrategy_InanimateObject(STRATEGYBLOCK* sbPtr)
{
	INANIMATE_OBJECT_SAVE_BLOCK *block;
	INANIMATEOBJECT_STATUSBLOCK* objectstatusptr;
	
	objectstatusptr = (INANIMATEOBJECT_STATUSBLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(Indestructable)
	COPYELEMENT_SAVE(explosionTimer)
	COPYELEMENT_SAVE(typeId)
	COPYELEMENT_SAVE(subType)
	

	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;
}
