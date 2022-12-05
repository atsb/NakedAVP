#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"

#include "particle.h"
#include "sfx.h"
#include "detaillevels.h"
#include "bh_types.h"
#include "bh_ais.h"
#include "bh_pred.h"
#include "bh_corpse.h"
#include "lighting.h"
#define UseLocalAssert Yes
#include "ourasert.h"

static SFXBLOCK SfxBlockStorage[MAX_NO_OF_SFX_BLOCKS];
static int NumFreeSfxBlocks;
static SFXBLOCK *FreeSfxBlockList[MAX_NO_OF_SFX_BLOCKS];
static SFXBLOCK **FreeSfxBlockListPtr;



/*KJL***************************************************************************
* FUNCTIONS TO ALLOCATE AND DEALLOCATE SFX BLOCKS - KJL 12:02:14 11/13/96 *
***************************************************************************KJL*/
void InitialiseSfxBlocks(void)
{
	SFXBLOCK *freeBlockPtr = SfxBlockStorage;
	int blk;

	for(blk=0; blk < MAX_NO_OF_SFX_BLOCKS; blk++) 
	{								
		FreeSfxBlockList[blk] = freeBlockPtr++;
	}

	FreeSfxBlockListPtr = &FreeSfxBlockList[MAX_NO_OF_SFX_BLOCKS-1];
	NumFreeSfxBlocks = MAX_NO_OF_SFX_BLOCKS;
}


SFXBLOCK* AllocateSfxBlock(void)
{
	SFXBLOCK *sfxPtr = 0; /* Default to null ptr */

	if (NumFreeSfxBlocks) 
	{
		sfxPtr = *FreeSfxBlockListPtr--;
		NumFreeSfxBlocks--;
	}
	else
	{
		/* unable to allocate a sfxamics block I'm afraid; 
		   MAX_NO_OF_SFX_BLOCKS is too low */
   	  //LOCALASSERT(NumFreeSfxBlocks);
		textprint("No Free SFX blocks!\n");
	}

	return sfxPtr;
}


void DeallocateSfxBlock(SFXBLOCK *sfxPtr)
{
	GLOBALASSERT(sfxPtr);
	*(++FreeSfxBlockListPtr) = sfxPtr;
	NumFreeSfxBlocks++;
}



DISPLAYBLOCK *CreateSFXObject(enum SFX_ID sfxID)
{
	DISPLAYBLOCK *dispPtr = CreateActiveObject();
	
	if (dispPtr)
	{
		SFXBLOCK *sfxPtr = AllocateSfxBlock();

		if (sfxPtr)
		{
			dispPtr->SfxPtr = sfxPtr;
			sfxPtr->SfxID = sfxID;
		}
		else
		{
			/* damn, we've got a DISPLAYBLOCK, but were unable to get a SFXBLOCK;
			   this means we must dealloc the DISPLAYBLOCK and return NULL to indicate 
			   failure.
			*/
			DestroyActiveObject(dispPtr);
			dispPtr = 0;
		}
	}

	return dispPtr;
}


void DrawSfxObject(DISPLAYBLOCK *dispPtr)
{
	SFXBLOCK *sfxPtr;
	
	GLOBALASSERT(dispPtr);
	
	sfxPtr = dispPtr->SfxPtr;
	GLOBALASSERT(sfxPtr);
	

	switch(sfxPtr->SfxID)
	{
		case SFX_MUZZLE_FLASH_AMORPHOUS:
		{
			if (!sfxPtr->EffectDrawnLastFrame)
			{
				VECTORCH direction;

				direction.vx = dispPtr->ObMat.mat31;
				direction.vy = dispPtr->ObMat.mat32;
				direction.vz = dispPtr->ObMat.mat33;
				DrawMuzzleFlash(&dispPtr->ObWorld,&direction,MUZZLE_FLASH_AMORPHOUS);
			}
			sfxPtr->EffectDrawnLastFrame=!sfxPtr->EffectDrawnLastFrame;

			break;
		}
		case SFX_MUZZLE_FLASH_SMARTGUN:
		{
			VECTORCH direction;

			direction.vx = dispPtr->ObMat.mat31;
			direction.vy = dispPtr->ObMat.mat32;
			direction.vz = dispPtr->ObMat.mat33;
			DrawMuzzleFlash(&dispPtr->ObWorld,&direction,MUZZLE_FLASH_SMARTGUN);
			break;
		}
		case SFX_MUZZLE_FLASH_SKEETER:
		{
			if (!sfxPtr->EffectDrawnLastFrame)
			{
				VECTORCH direction;

				direction.vx = dispPtr->ObMat.mat31;
				direction.vy = dispPtr->ObMat.mat32;
				direction.vz = dispPtr->ObMat.mat33;
				DrawMuzzleFlash(&dispPtr->ObWorld,&direction,MUZZLE_FLASH_SKEETER);
			}
			sfxPtr->EffectDrawnLastFrame=!sfxPtr->EffectDrawnLastFrame;

			break;
		}
		case SFX_FRISBEE_PLASMA_BOLT:
		{
			VECTORCH direction;
			direction.vx = dispPtr->ObMat.mat31;
			direction.vy = dispPtr->ObMat.mat32;
			direction.vz = dispPtr->ObMat.mat33;
			DrawFrisbeePlasmaBolt(&dispPtr->ObWorld,&direction);

			break;
		}
		case SFX_PREDATOR_PLASMA_BOLT:
		{
			VECTORCH direction;
			direction.vx = dispPtr->ObMat.mat31;
			direction.vy = dispPtr->ObMat.mat32;
			direction.vz = dispPtr->ObMat.mat33;
			DrawPredatorPlasmaBolt(&dispPtr->ObWorld,&direction);

			break;
		}
		case SFX_SMALL_PREDATOR_PLASMA_BOLT:
		{
			VECTORCH direction;
			direction.vx = dispPtr->ObMat.mat31;
			direction.vy = dispPtr->ObMat.mat32;
			direction.vz = dispPtr->ObMat.mat33;
			DrawSmallPredatorPlasmaBolt(&dispPtr->ObWorld,&direction);

			break;
		}

		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}
}

void HandleSfxForObject(DISPLAYBLOCK *dispPtr)
{
	STRATEGYBLOCK *sbPtr = dispPtr->ObStrategyBlock;

	if (dispPtr->SpecialFXFlags & SFXFLAG_ONFIRE)
	{
		HandleObjectOnFire(dispPtr);
	}

	if (sbPtr)
	{
		if(sbPtr->I_SBtype == I_BehaviourNetCorpse)
		{
			NETCORPSEDATABLOCK *corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
		
			if(!( (dispPtr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)&&(dispPtr->ObFlags2 < ONE_FIXED) )
				&& corpseDataPtr->This_Death->Electrical && ((FastRandom()&255)==0))	
			{
				VECTORCH velocity;
				velocity.vx = (FastRandom()&2047)-1024;
				velocity.vy = (FastRandom()&2047)-1024;
				velocity.vz = (FastRandom()&2047)-1024;
				MakeParticle(&dispPtr->ObWorld,&velocity,PARTICLE_SPARK);	
				velocity.vx = (FastRandom()&2047)-1024;
				velocity.vy = (FastRandom()&2047)-1024;
				velocity.vz = (FastRandom()&2047)-1024;
				MakeParticle(&dispPtr->ObWorld,&velocity,PARTICLE_SPARK);	
				MakeLightElement(&dispPtr->ObWorld,LIGHTELEMENT_ELECTRICAL_SPARKS);
				
			}
		}
		
	}
}


void HandleObjectOnFire(DISPLAYBLOCK *dispPtr)
{
	int objectIsDisappearing;
	extern int NormalFrameTime;
	int noRequired = 1;
	int i;
	VECTORCH velocity;
	
	if (!dispPtr->ObShape) return;

	if (dispPtr->ObShapeData->shaperadius<=LocalDetailLevels.AlienEnergyViewThreshold) return;

	#if 1
	{
		DYNAMICSBLOCK *dynPtr;
		STRATEGYBLOCK *sbPtr;
		
	   	sbPtr = dispPtr->ObStrategyBlock;
		LOCALASSERT(sbPtr);
		dynPtr = sbPtr->DynPtr;
		LOCALASSERT(sbPtr);

		
		velocity.vx = DIV_FIXED((dynPtr->Position.vx-dynPtr->PrevPosition.vx)*3,NormalFrameTime*4);
		velocity.vy = DIV_FIXED((dynPtr->Position.vy-dynPtr->PrevPosition.vy)*3,NormalFrameTime*4);
		velocity.vz = DIV_FIXED((dynPtr->Position.vz-dynPtr->PrevPosition.vz)*3,NormalFrameTime*4);

		if (dispPtr==sbPtr->SBdptr)	noRequired = 5;

	}
	#else
	velocity.vx = 0;
	velocity.vy = 0;
	velocity.vz = 0;
	#endif
	
	objectIsDisappearing = ( (dispPtr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND) &&(dispPtr->ObFlags2 <= ONE_FIXED) )	;

	for (i=0; i<noRequired; i++)
	{
		VECTORCH position;
		position.vx = dispPtr->ObWorld.vx+(FastRandom()&255)-128;
		position.vy = dispPtr->ObWorld.vy+(FastRandom()&255)-128;
		position.vz = dispPtr->ObWorld.vz+(FastRandom()&255)-128;
		#if 0
		MakeParticle(&(position), &velocity, PARTICLE_NONCOLLIDINGFLAME);
		#else
		if (objectIsDisappearing)
		{
			if ((FastRandom()&65535) < dispPtr->ObFlags2)
				MakeParticle(&(position), &velocity, PARTICLE_FIRE);
		}
		else
		{
			MakeParticle(&(position), &velocity, PARTICLE_FIRE);
		}

		if ((FastRandom()&65535) > 32768)
		{
			MakeParticle(&(position), &velocity, PARTICLE_IMPACTSMOKE);
		}
		#endif
	}

}
