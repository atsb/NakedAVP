/* KJL 16:14:35 09/09/98 - BonusAbilities.c */
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "weapons.h"
#include "dynblock.h"
#include "avpview.h"

#include "load_shp.h"
#include "kzsort.h"
#include "kshape.h"

#include "pfarlocs.h"
#include "pvisible.h"

#define UseLocalAssert Yes
#include "ourasert.h"


extern int NormalFrameTime;



/* KJL 16:18:20 09/09/98 - Predator Grappling Hook */
struct GrapplingHookData
{
	int IsEmbedded;
	int IsEngaged;
	int Tightness;

	VECTORCH Position;
	MATRIXCH Orientation;
	int ShapeIndex;
	DISPLAYBLOCK *DispPtr;
}; 

static struct GrapplingHookData GrapplingHook;

static DISPLAYBLOCK* CreateGrapplingHook(void);


extern void InitialiseGrapplingHook(void)
{
	GrapplingHook.IsEngaged = 0;
	GrapplingHook.IsEmbedded = 0;
	GrapplingHook.ShapeIndex = GetLoadedShapeMSL("spear");
	GrapplingHook.DispPtr = 0;
	
}

static void FireGrapplingHook(void);
void DisengageGrapplingHook(void);

extern void ActivateGrapplingHook(void)
{	
	if (GrapplingHook.IsEngaged)
	{
		DisengageGrapplingHook();
	}
	else
	{
		FireGrapplingHook();
	}
}
static void FireGrapplingHook(void)
{
	GrapplingHook.DispPtr = CreateGrapplingHook();
	
	if (GrapplingHook.DispPtr)
	{
		GrapplingHook.IsEngaged = 1;
		GrapplingHook.IsEmbedded = 0;
		GrapplingHook.Tightness = ONE_FIXED;

		/* CDF 14/4/99 Make a sound... */
		Sound_Play(SID_GRAPPLE_THROW,"h");

		#if 0
		/* los */
		GrapplingHook.Position = PlayersTarget.Position;
		GrapplingHook.Orientation = Global_VDB_Ptr->VDB_Mat;
		TransposeMatrixCH(&GrapplingHook.Orientation);
		#endif
	}

}

static DISPLAYBLOCK* CreateGrapplingHook(void)
{
	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) return NULL; /* failure */
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourGrapplingHook;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
	if(sbPtr->DynPtr)
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

      	dynPtr->PrevPosition = dynPtr->Position = Global_VDB_Ptr->VDB_World;

		GrapplingHook.Orientation = Global_VDB_Ptr->VDB_Mat;
		TransposeMatrixCH(&GrapplingHook.Orientation);
		dynPtr->OrientMat = GrapplingHook.Orientation;
		
		dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
		dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
		dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		return NULL;
	}

	sbPtr->shapeIndex = GetLoadedShapeMSL("spear");

	sbPtr->maintainVisibility = 0;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), 0);
	LOCALASSERT(sbPtr->containingModule);
	if(!(sbPtr->containingModule))
	{
		/* no containing module can be found... abort*/
		RemoveBehaviourStrategy(sbPtr);
		return NULL;
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
			return NULL;
		}
		
		sbPtr->SBdptr = dPtr;
		dPtr->ObStrategyBlock = sbPtr;
		dPtr->ObMyModule = NULL;					
		dPtr->ObWorld = dynPtr->Position;
		dPtr->ObEuler = dynPtr->OrientEuler;
		dPtr->ObMat = dynPtr->OrientMat;

		dPtr->ObRadius=10;
		dPtr->ObMaxX=10;
		dPtr->ObMinX=-10;
		dPtr->ObMaxY=10;
		dPtr->ObMinY=-10;
		dPtr->ObMaxZ=10;
		dPtr->ObMinZ=-10;
		/* make displayblock a dynamic module object */
		dPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;

//		sbPtr->SBDamageBlock.IsOnFire=1;

		return dPtr;
	}

}

void GrapplingHookBehaviour(STRATEGYBLOCK *sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	if (!GrapplingHook.IsEmbedded)
	{
		COLLISIONREPORT *reportPtr = dynPtr->CollisionReportPtr;

		if(reportPtr)
		{
			char stickWhereYouAre = 0;

			if (reportPtr->ObstacleSBPtr)
			{
				DISPLAYBLOCK *dispPtr = reportPtr->ObstacleSBPtr->SBdptr;
				if (dispPtr)
				if (dispPtr->ObMyModule && (!dispPtr->ObMorphCtrl))
				{
					stickWhereYouAre=1;
				}
			}
			else
			{
				stickWhereYouAre = 1;
			}
						

			if(stickWhereYouAre)
			{
				dynPtr->IsStatic=1;
				dynPtr->PrevPosition=dynPtr->Position;
				GrapplingHook.Position=dynPtr->Position;
				GrapplingHook.IsEmbedded=1;
				/* CDF 14/4/99 Make a sound. */
				Sound_Play(SID_GRAPPLE_HIT_WALL,"d",&dynPtr->Position);
				return;
			}
			else
			{
				DisengageGrapplingHook();
				return;
			}
		}
	}
	else
	{
		GrapplingHook.Tightness -= NormalFrameTime*4;
		if (GrapplingHook.Tightness<0) GrapplingHook.Tightness=0;
	}
}

extern void DisengageGrapplingHook(void)
{
	GrapplingHook.IsEngaged = 0;
	GrapplingHook.IsEmbedded = 0;
	if (GrapplingHook.DispPtr)
	{
    	RemoveBehaviourStrategy(GrapplingHook.DispPtr->ObStrategyBlock);
		GrapplingHook.DispPtr=NULL;
	}
}

extern void HandleGrapplingHookForces(void)
{
	if (GrapplingHook.IsEmbedded)
	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		VECTORCH direction = GrapplingHook.Position;
		int distance;

		direction.vx -= dynPtr->Position.vx;
		direction.vy -= dynPtr->Position.vy-1000;
		direction.vz -= dynPtr->Position.vz;

		distance = Approximate3dMagnitude(&direction);
		if (distance>4096+1024)
		{
			Normalise(&direction);
			dynPtr->LinImpulse.vx += MUL_FIXED(direction.vx,NormalFrameTime);
			dynPtr->LinImpulse.vy += MUL_FIXED(direction.vy,NormalFrameTime);
			dynPtr->LinImpulse.vz += MUL_FIXED(direction.vz,NormalFrameTime);
		}
		else if (distance>1024)
		{
			int s = MUL_FIXED((distance-1024)*16,NormalFrameTime);
			Normalise(&direction);
			dynPtr->LinImpulse.vx += MUL_FIXED(direction.vx,s);
			dynPtr->LinImpulse.vy += MUL_FIXED(direction.vy,s);
			dynPtr->LinImpulse.vz += MUL_FIXED(direction.vz,s);

			dynPtr->LinImpulse.vx -= MUL_FIXED(dynPtr->LinImpulse.vx,NormalFrameTime/2);
			dynPtr->LinImpulse.vy -= MUL_FIXED(dynPtr->LinImpulse.vy,NormalFrameTime/2);
			dynPtr->LinImpulse.vz -= MUL_FIXED(dynPtr->LinImpulse.vz,NormalFrameTime/2);
		}

		if (Approximate3dMagnitude(&dynPtr->LinImpulse)>ONE_FIXED)
		{
			Normalise(&dynPtr->LinImpulse);
		}
	}
}

extern void RenderGrapplingHook(void)
{
	if (GrapplingHook.IsEngaged && GrapplingHook.DispPtr)
	{
		extern void D3D_DrawCable(VECTORCH *centrePtr, MATRIXCH *orientationPtr);
		extern int CloakingPhase;
		VECTORCH cable[46];
		int i;
		{
			MATRIXCH mat = Global_VDB_Ptr->VDB_Mat;
			TransposeMatrixCH(&mat);

			cable[0].vx = Global_VDB_Ptr->VDB_World.vx-mat.mat31/128;
			cable[0].vy = Global_VDB_Ptr->VDB_World.vy-mat.mat32/128+500;
			cable[0].vz = Global_VDB_Ptr->VDB_World.vz-mat.mat33/128;
		}
//		cable[0].vx = Global_VDB_Ptr->VDB_World.vx;
//		cable[0].vy = Global_VDB_Ptr->VDB_World.vy+500;
//		cable[0].vz = Global_VDB_Ptr->VDB_World.vz;

		for (i=1; i<46; i++)
		{	
			cable[i].vx = ((45-i)*cable[0].vx + (i)*GrapplingHook.DispPtr->ObStrategyBlock->DynPtr->Position.vx)/45;
			cable[i].vy = ((45-i)*cable[0].vy + (i)*GrapplingHook.DispPtr->ObStrategyBlock->DynPtr->Position.vy)/45;
			cable[i].vz = ((45-i)*cable[0].vz + (i)*GrapplingHook.DispPtr->ObStrategyBlock->DynPtr->Position.vz)/45;

			if (GrapplingHook.Tightness!=0)
			{
				int x = GetSin((302*i+CloakingPhase)&4095)/256;
				int y = GetSin((502*i+200+CloakingPhase)&4095)/256;
				int z = GetCos((302*i+100+CloakingPhase)&4095)/256;
				int u = GetSin( ((4096*i)/45)&4095 );
				u = MUL_FIXED(MUL_FIXED(u,u),GrapplingHook.Tightness);
				cable[i].vx += MUL_FIXED(u,x);
				cable[i].vy += MUL_FIXED(u,y);
				cable[i].vz += MUL_FIXED(u,z);
			}
		}
		
		{
			MATRIXCH mat;
			VECTORCH dir = cable[45];
			dir.vx -= cable[0].vx;
			dir.vy -= cable[0].vy;
			dir.vz -= cable[0].vz;
			Normalise(&dir);
			MakeMatrixFromDirection(&dir,&mat);
			D3D_DrawCable(cable, &mat);
		}
		#if 0
		DISPLAYBLOCK displayblock;
		displayblock.ObWorld=GrapplingHook.Position;
		displayblock.ObMat=GrapplingHook.Orientation;
		displayblock.ObShape=GrapplingHook.ShapeIndex;
		displayblock.ObShapeData=GetShapeData(GrapplingHook.ShapeIndex);

		displayblock.name=NULL;
		displayblock.ObEuler.EulerX=0;
		displayblock.ObEuler.EulerY=0;
		displayblock.ObEuler.EulerZ=0;
		displayblock.ObFlags=0;
		displayblock.ObFlags2=0;
		displayblock.ObFlags3=0;
		displayblock.ObNumLights=0;
		displayblock.ObRadius=0;
		displayblock.ObMaxX=0;
		displayblock.ObMinX=0;
		displayblock.ObMaxY=0;
		displayblock.ObMinY=0;
		displayblock.ObMaxZ=0;
		displayblock.ObMinZ=0;
		displayblock.ObTxAnimCtrlBlks=NULL;
		displayblock.ObEIDPtr=NULL;
		displayblock.ObMorphCtrl=NULL;
		displayblock.ObStrategyBlock=NULL;
		displayblock.ShapeAnimControlBlock=NULL;
		displayblock.HModelControlBlock=NULL;
		displayblock.ObMyModule=NULL;		
		displayblock.SpecialFXFlags = 0;
		displayblock.SfxPtr=0;

		MakeVector(&displayblock.ObWorld, &Global_VDB_Ptr->VDB_World, &displayblock.ObView);
		RotateVector(&displayblock.ObView, &Global_VDB_Ptr->VDB_Mat);
		RenderThisDisplayblock(&displayblock);
		{
			PARTICLE particle;
			particle.Colour = 0xffffffff;
			particle.ParticleID = PARTICLE_LASERBEAM;
			particle.Position = Player->ObStrategyBlock->DynPtr->Position;
			particle.Position.vy-=1000;
			particle.Offset = GrapplingHook.DispPtr->ObStrategyBlock->DynPtr->Position;
			particle.Size = 20;
			RenderParticle(&particle);
		}
		#endif

	}
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct grapple_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int IsEmbedded;
	int IsEngaged;
	int Tightness;

	VECTORCH Position;
	MATRIXCH Orientation;
//strategy block stuff
	DYNAMICSBLOCK dynamics;
}GRAPPLE_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV (&GrapplingHook)

void LoadStrategy_Grapple(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	GRAPPLE_SAVE_BLOCK* block = (GRAPPLE_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//create the grappling hook
	GrapplingHook.DispPtr = CreateGrapplingHook();
	if(!GrapplingHook.DispPtr) return;

	//copy suff from the save block
	COPYELEMENT_LOAD(IsEmbedded)
	COPYELEMENT_LOAD(IsEngaged)
	COPYELEMENT_LOAD(Tightness)
	COPYELEMENT_LOAD(Position)
	COPYELEMENT_LOAD(Orientation)
	
	*GrapplingHook.DispPtr->ObStrategyBlock->DynPtr = block->dynamics;
}

void SaveStrategy_Grapple(STRATEGYBLOCK* sbPtr)
{
	GRAPPLE_SAVE_BLOCK* block;
	
	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//copy stuff to the save block
	COPYELEMENT_SAVE(IsEmbedded)
	COPYELEMENT_SAVE(IsEngaged)
	COPYELEMENT_SAVE(Tightness)
	COPYELEMENT_SAVE(Position)
	COPYELEMENT_SAVE(Orientation)
	
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
}
