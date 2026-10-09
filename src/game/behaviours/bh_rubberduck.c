#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"

#include "bh_types.h"
#include "dynblock.h"
#include "dynamics.h"

#include "pfarlocs.h"

#include "pvisible.h"
#include "load_shp.h"
#include "particle.h"

#include "bh_rubberduck.h"			   
#include "bh_weap.h"
#include "sfx.h"

#define UseLocalAssert Yes
#include "ourasert.h"

extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
extern int NormalFrameTime;

void CreateRubberDuckBot(void)
{
	CreateRubberDuck(&(Global_VDB_Ptr->VDB_World));
}


void CreateRubberDuck(VECTORCH *positionPtr)
{
	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) return; /* failure */
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourRubberDuck;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_GRENADE);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		EULER zeroEuler = {0,0,0};
		zeroEuler.EulerY = FastRandom()&4095;
      	dynPtr->PrevPosition = dynPtr->Position = *positionPtr;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
		
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;
		dynPtr->LinImpulse.vy = 0;
		dynPtr->GravityOn = 0;
		dynPtr->Elasticity = 0;
		dynPtr->IsFloating = 1;
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	sbPtr->shapeIndex = GetLoadedShapeMSL("ciggies");//Duck");

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), 0);
	LOCALASSERT(sbPtr->containingModule);
	if(!(sbPtr->containingModule))
	{
		/* no containing module can be found... abort*/
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

}

void RubberDuckBehaviour(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	int newLevel;

	if (!sbPtr->SBdptr) return;
	#if 1
	newLevel = EffectOfRipples(&(dynPtr->Position));
	{
		int minusDeltaX,plusDeltaX,minusDeltaZ,plusDeltaZ;
		VECTORCH delta;
		delta.vx = dynPtr->Position.vx;
		delta.vz = dynPtr->Position.vz;
		

		delta.vx -= 50;
		minusDeltaX = EffectOfRipples(&(delta))-newLevel;
		delta.vx += 100;
		plusDeltaX = EffectOfRipples(&(delta))-newLevel;
		delta.vx -= 50;

		delta.vz -= 50;
		minusDeltaZ = EffectOfRipples(&(delta))-newLevel;
		delta.vz += 100;
		plusDeltaZ = EffectOfRipples(&(delta))-newLevel;
		{
			int scale = NormalFrameTime<<1;
			if(scale>ONE_FIXED) scale = ONE_FIXED;
			scale = ONE_FIXED;
	   		dynPtr->LinImpulse.vx -= MUL_FIXED(scale,dynPtr->LinImpulse.vx);
	   		dynPtr->LinImpulse.vz -= MUL_FIXED(scale,dynPtr->LinImpulse.vz);
		}
		if (minusDeltaX > plusDeltaX)
		{
			if (minusDeltaX>0) dynPtr->LinImpulse.vx = -minusDeltaX*256;
		}
		else
		{
			if (plusDeltaX>0) dynPtr->LinImpulse.vx = plusDeltaX*256;
		}
		
		if (minusDeltaZ > plusDeltaZ)
		{
			if (minusDeltaZ>0) dynPtr->LinImpulse.vz = -minusDeltaZ*256;
		}
		else
		{
			if (plusDeltaZ>0) dynPtr->LinImpulse.vz = plusDeltaZ*256;
		}														 

	}
	dynPtr->LinImpulse.vy = 0;
	{
		int level = 0;
		extern char LevelName[];
		{
			if (!strcmp(LevelName,"e3demosp")||!strcmp(LevelName,"e3demo"))
			{
				level = 3300;
			}
			else if (!strcmp(LevelName,"invasion_a"))
			{
				level = -35800;
			}
			else if (!strcmp(LevelName,"genshd1"))
			{
				level = 2656;
			}
			else if (!strcmp(LevelName,"fall")||!strcmp(LevelName,"fall_m"))
			{
				level = 12925;
			}
			else if (!strcmp(LevelName,"derelict"))
			{
				level = 32000;
			}
			dynPtr->Position.vy = newLevel+level;
		}
	}
	
	dynPtr->AngVelocity.EulerY -= MUL_FIXED(dynPtr->AngVelocity.EulerY,NormalFrameTime);
	
	dynPtr->AngVelocity.EulerY += (dynPtr->LinImpulse.vz + dynPtr->LinImpulse.vx+(FastRandom()&255)-128)/16;

	dynPtr->AngVelocity.EulerZ = (dynPtr->LinImpulse.vz/256);
	dynPtr->AngVelocity.EulerX = (dynPtr->LinImpulse.vx/256);
	
	if (dynPtr->AngVelocity.EulerY > 8192) dynPtr->AngVelocity.EulerY = 8192;
	else if (dynPtr->AngVelocity.EulerY < -8192) dynPtr->AngVelocity.EulerY = -8192;
	DynamicallyRotateObject(dynPtr);
	#else
	{
		VECTORCH dir;
		dynPtr->GravityOn = 1;
		dynPtr->IsFloating = 0;
		dynPtr->CanClimbStairs = 0;
		sbPtr->SBdptr->ObFlags3 |= ObFlag3_DynamicModuleObject;

		if (BestDirectionOfTravel(&(Player->ObStrategyBlock->DynPtr->Position), &(dynPtr->Position), &dir))
		{
			dynPtr->LinVelocity.vx = MUL_FIXED(4000,dir.vx);
			dynPtr->LinVelocity.vy = 0;
			dynPtr->LinVelocity.vz = MUL_FIXED(4000,dir.vz);
		}
		else
		{
			dynPtr->LinVelocity.vx = 0;
			dynPtr->LinVelocity.vy = 0;
			dynPtr->LinVelocity.vz = 0;
		}
	}
	#endif

}

extern void CreateFlamingDebris(VECTORCH *positionPtr, VECTORCH *dirPtr)
{
	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) return; /* failure */
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourFragment;

	AssignNewSBName(sbPtr);

	sbPtr->SBdataptr = (void*)AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK));

	if (sbPtr->SBdataptr == 0) 
	{	
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
	((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ((FastRandom()&32768)<<2) + 65535*2;

	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_DEBRIS);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
      	dynPtr->PrevPosition = dynPtr->Position = *positionPtr;
		dynPtr->OrientEuler.EulerX = 0;
		dynPtr->OrientEuler.EulerY = FastRandom()&4095;
		dynPtr->OrientEuler.EulerZ = 0;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
		
		dynPtr->LinImpulse.vx = dirPtr->vx/4;
		dynPtr->LinImpulse.vy = dirPtr->vy/4;
		if (dynPtr->LinImpulse.vy>0) dynPtr->LinImpulse.vy=-dynPtr->LinImpulse.vy;
		dynPtr->LinImpulse.vy += -4000;
		dynPtr->LinImpulse.vz = dirPtr->vz/4;
		dynPtr->AngVelocity.EulerX = (((FastRandom()&2047)-1023))<<2;
		dynPtr->AngVelocity.EulerY = (((FastRandom()&2047)-1023))<<2;
		dynPtr->AngVelocity.EulerZ = (((FastRandom()&2047)-1023))<<2;

		dynPtr->Elasticity = ONE_FIXED/4;
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	sbPtr->shapeIndex = GetLoadedShapeMSL("Shell");

	sbPtr->maintainVisibility = 0;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), 0);
	LOCALASSERT(sbPtr->containingModule);
	if(!(sbPtr->containingModule))
	{
		/* no containing module can be found... abort*/
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
	
	{
		MODULE tempModule;
		DISPLAYBLOCK *dPtr;
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
  
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
		if(dPtr==NULL)
		{
			RemoveBehaviourStrategy(sbPtr);
			return;
		}
		
		sbPtr->SBdptr = dPtr;
		dPtr->ObStrategyBlock = sbPtr;
		dPtr->ObMyModule = NULL;					
		dPtr->ObWorld = dynPtr->Position;
		dPtr->ObEuler = dynPtr->OrientEuler;
		dPtr->ObMat = dynPtr->OrientMat;

		/* make displayblock a dynamic module object */
		dPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

		sbPtr->SBDamageBlock.IsOnFire=1;

	}
}

void CreateRubberDucks(void)
{
	extern char LevelName[];
	#if 0
	if ( (!stricmp(LevelName,"e3demo")) || (!stricmp(LevelName,"e3demosp")) )
	{
		int i = 6;

		do
		{
			VECTORCH pos = {1023,3400,27536};
			pos.vx += (FastRandom()&4095)-2048;
			pos.vz += (FastRandom()&4095)-2048;
			CreateRubberDuck(&pos);
		}
		while(--i);
	}
	else 
	#endif
	if ( (!stricmp(LevelName,"invasion_a")) )
	{
		int i = 6;

		do
		{
			VECTORCH pos = {21803,-35491,40607};
			pos.vx += FastRandom()&8191;
			pos.vz -= FastRandom()&8191;
			CreateRubberDuck(&pos);
		}
		while(--i);
	}
}
