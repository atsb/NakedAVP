/* CDF 9/10/98 A bold new initiaitive! */

#include "3dc.h"		   
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "dynblock.h"
#include "dynamics.h"

#include "weapons.h"
#include "comp_shp.h"
#include "inventry.h"
#include "triggers.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "bh_swdor.h"
#include "bh_plift.h"
#include "load_shp.h"
#include "bh_weap.h"
#include "bh_debri.h"
#include "lighting.h"
#include "bh_lnksw.h"
#include "bh_binsw.h"
#include "pheromon.h"
#include "bh_pred.h"
#include "bh_agun.h"
#include "plat_shp.h"
#include "psnd.h"
#include "ai_sight.h"
#include "sequnces.h"
#include "huddefs.h"
#include "showcmds.h"
#include "sfx.h"
#include "bh_marin.h"
#include "bh_dummy.h"
#include "bh_far.h"
#include "targeting.h"
#include "dxlog.h"
#include "los.h"
#include "psndplat.h"
#include "extents.h"

/* for win95 net game support */
#include "pldghost.h"
#include "pldnet.h"

extern int NormalFrameTime;
extern unsigned char Null_Name[8];
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
void CreateDummy(VECTORCH *Position);

/* Begin code! */

void CastDummy(void) {

	#define BOTRANGE 2000

	VECTORCH position;

	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO DUMMYS IN MULTIPLAYER MODE");
		return;
	}

	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateDummy(&position);

}

void CreateDummy(VECTORCH *Position) {

	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) {
		NewOnScreenMessage("FAILED TO CREATE DUMMY: SB CREATION FAILURE");
		return; /* failure */
	}
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourDummy;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_MARINE_PLAYER);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = *Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE DUMMY: DYNBLOCK CREATION FAILURE");
		return;
	}

	sbPtr->shapeIndex = 0;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* Initialise dummy's stats */
	{
		sbPtr->SBDamageBlock.Health=30000<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=30000<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags.AcidResistant=1;
		sbPtr->SBDamageBlock.SB_H_flags.FireResistant=1;
		sbPtr->SBDamageBlock.SB_H_flags.ElectricResistant=1;
		sbPtr->SBDamageBlock.SB_H_flags.PerfectArmour=1;
		sbPtr->SBDamageBlock.SB_H_flags.ElectricSensitive=0;
		sbPtr->SBDamageBlock.SB_H_flags.Combustability=0;
		sbPtr->SBDamageBlock.SB_H_flags.Indestructable=1;
	}
	/* create, initialise and attach a dummy data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(DUMMY_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		DUMMY_STATUS_BLOCK *dummyStatus = (DUMMY_STATUS_BLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(dummyStatus);
	
		dummyStatus->incidentFlag=0;
		dummyStatus->incidentTimer=0;

		dummyStatus->HModelController.section_data=NULL;
		dummyStatus->HModelController.Deltas=NULL;
			
		switch (AvP.PlayerType) {
			case I_Marine:
				dummyStatus->PlayerType=I_Marine;
				root_section=GetNamedHierarchyFromLibrary("hnpcmarine","marine with pulse rifle");
				if (!root_section) {
					RemoveBehaviourStrategy(sbPtr);
					NewOnScreenMessage("FAILED TO CREATE DUMMY: NO HMODEL");
					return;
				}
				Create_HModel(&dummyStatus->HModelController,root_section);
				InitHModelSequence(&dummyStatus->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_A,-1);
				break;
			case I_Alien:
				dummyStatus->PlayerType=I_Alien;
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				if (!root_section) {
					RemoveBehaviourStrategy(sbPtr);
					NewOnScreenMessage("FAILED TO CREATE DUMMY: NO HMODEL");
					return;
				}
				Create_HModel(&dummyStatus->HModelController,root_section);
				InitHModelSequence(&dummyStatus->HModelController,(int)HMSQT_AlienStand,(int)ASSS_Standard,-1);
				break;
			case I_Predator:
				dummyStatus->PlayerType=I_Predator;
				root_section=GetNamedHierarchyFromLibrary("hnpcpredator","pred with wristblade");
				if (!root_section) {
					RemoveBehaviourStrategy(sbPtr);
					NewOnScreenMessage("FAILED TO CREATE DUMMY: NO HMODEL");
					return;
				}
				Create_HModel(&dummyStatus->HModelController,root_section);
				InitHModelSequence(&dummyStatus->HModelController,(int)HMSQT_PredatorStand,(int)PSSS_Standard,-1);
				break;
		}
		ProveHModel_Far(&dummyStatus->HModelController,sbPtr);

		if(!(sbPtr->containingModule))
		{
			/* no containing module can be found... abort*/
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE DUMMY: MODULE CONTAINMENT FAILURE");
			return;
		}
		LOCALASSERT(sbPtr->containingModule);

		MakeDummyNear(sbPtr);

		NewOnScreenMessage("DUMMY CREATED");

	} else {
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE DUMMY: MALLOC FAILURE");
		return;
	}
}

void MakeDummyNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	DUMMY_STATUS_BLOCK *dummyStatusPointer;    

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);
	dynPtr = sbPtr->DynPtr;
	dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(dummyStatusPointer);	          		
    LOCALASSERT(dynPtr);


	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr = NULL;
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;
	if(dPtr==NULL) return; /* cannot allocate displayblock, so leave far */
			
	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					

	/* need to initialise positional information in the new display block */ 
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* state and sequence initialisation */
	
	dPtr->HModelControlBlock=&dummyStatusPointer->HModelController;

	ProveHModel(dPtr->HModelControlBlock,dPtr);

	/*Copy extents from the collision extents in extents.c*/
	dPtr->ObMinX=-CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMaxX=CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMinZ=-CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMaxZ=CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMinY=CollisionExtents[CE_MARINE].CrouchingTop;
	dPtr->ObMaxY=CollisionExtents[CE_MARINE].Bottom;
	dPtr->ObRadius = 1000;

}

void MakeDummyFar(STRATEGYBLOCK *sbPtr)
{
	DUMMY_STATUS_BLOCK *dummyStatusPointer;    
	int i;
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->SBdptr != NULL);
	dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(dummyStatusPointer);	          		

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

}

void DummyBehaviour(STRATEGYBLOCK *sbPtr) {

	DUMMY_STATUS_BLOCK *dummyStatusPointer;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(dummyStatusPointer);	          		

	/* Should be the same near as far. */
	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	} else if (dummyStatusPointer->PlayerType!=I_Alien) {
		AddMarinePheromones(sbPtr->containingModule->m_aimodule);
	}

	/* Incident handling. */
	dummyStatusPointer->incidentFlag=0;

	dummyStatusPointer->incidentTimer-=NormalFrameTime;
	
	if (dummyStatusPointer->incidentTimer<0) {
		dummyStatusPointer->incidentFlag=1;
		dummyStatusPointer->incidentTimer=32767+(FastRandom()&65535);
	}

}
