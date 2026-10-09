#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_videoscreen.h"
#include "dynblock.h"
#include "dynamics.h"
#include "bh_debri.h"
#include "plat_shp.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "pvisible.h"



void* InitVideoScreen(void* bhdata,STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_VIDEO_SCREEN *toolsData = (TOOLS_DATA_VIDEO_SCREEN *)bhdata;
	VIDEO_SCREEN_BEHAV_BLOCK* videoScreen;
	int i;

	LOCALASSERT(sbPtr->I_SBtype == I_BehaviourVideoScreen);
	LOCALASSERT(toolsData);

	/* create, initialise and attach a data block */
	videoScreen = (void *)AllocateMem(sizeof(VIDEO_SCREEN_BEHAV_BLOCK));
	if(!videoScreen)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	videoScreen->bhvr_type=I_BehaviourVideoScreen;

	sbPtr->SBdataptr = videoScreen;
			
	
	/* set default indestructibility */
	videoScreen->Indestructable = No;

	
	/* Initialise object's stats */
	//set health and armour
	{
		NPC_DATA *NpcData;
   
		NpcData=GetThisNpcData(I_NPC_DefaultInanimate);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
	}
		
	sbPtr->SBDamageBlock.Health*=toolsData->integrity;

	//get a dynamics block
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
	
	if(!sbPtr->DynPtr)
	{
		RemoveBehaviourStrategy(sbPtr);
		return 0;
	}

	
	//is this screen indestructable
	if (toolsData->integrity > 20)
	{
		videoScreen->Indestructable = Yes;
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

	
	/*check to see if object is animated.*/
	{
		TXACTRLBLK **pptxactrlblk;		
		int item_num;
		int shape_num = toolsData->shapeIndex;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
		pptxactrlblk = &videoScreen->inan_tac;
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
	
	//copy destruction target stuff
	videoScreen->destruct_target_request=toolsData->destruct_target_request;
	for(i=0;i<SB_NAME_LENGTH;i++)
	{
		videoScreen->destruct_target_ID[i]=toolsData->destruct_target_ID[i];
	}
	videoScreen->destruct_target_sbptr=0;
	
	return((void*)videoScreen);

}




void VideoScreenBehaviour(STRATEGYBLOCK *sbPtr)
{
	DISPLAYBLOCK* dptr;
	VIDEO_SCREEN_BEHAV_BLOCK* videoScreen;
	GLOBALASSERT(sbPtr);
	videoScreen = sbPtr->SBdataptr;
	GLOBALASSERT(videoScreen);

	dptr = sbPtr->SBdptr;
	if(dptr)
  	{
   		LIGHTBLOCK *lightPtr = dptr->ObLights[0];
		//update light if near
		if (lightPtr)
		{
			if(sbPtr->SBdptr->ObNumLights==1)
			{
				extern int FmvColourRed;
				extern int FmvColourGreen;
				extern int FmvColourBlue;
				
				lightPtr->LightBright = ONE_FIXED;
				lightPtr->LightFlags = LFlag_Omni;
				lightPtr->LightType = LightType_PerVertex;
				lightPtr->LightRange = 5000; 

				lightPtr->RedScale=	  FmvColourRed;
				lightPtr->GreenScale= FmvColourGreen;
				lightPtr->BlueScale=  FmvColourBlue;
			}
		}
		
		//deal with texture animation if near
		if(videoScreen->inan_tac)
		{
			if(!dptr->ObTxAnimCtrlBlks)
			{ 
				dptr->ObTxAnimCtrlBlks = videoScreen->inan_tac;
			}
		}
	}
}




void VideoScreenIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	VIDEO_SCREEN_BEHAV_BLOCK* videoScreen;
	GLOBALASSERT(sbPtr);
	videoScreen = sbPtr->SBdataptr;
	GLOBALASSERT(videoScreen);

	//no netweok code for this yet (if ever)
	if (!videoScreen->Indestructable)
	{
		if(sbPtr->SBDamageBlock.Health <= 0) 
		{
			//notify target of destruction
			if(videoScreen->destruct_target_sbptr)
			{
				RequestState(videoScreen->destruct_target_sbptr,videoScreen->destruct_target_request,0);
			}
			
			
			//the object has been destroyed
			MakeFragments(sbPtr);
			DestroyAnyStrategyBlock(sbPtr);
		}
	}
}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct video_screen_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL Indestructable;
	

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
}VIDEO_SCREEN_SAVE_BLOCK;



void LoadStrategy_VideoScreen(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	VIDEO_SCREEN_BEHAV_BLOCK* videoScreen;
	VIDEO_SCREEN_SAVE_BLOCK* block = (VIDEO_SCREEN_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourVideoScreen) return;

	videoScreen = (VIDEO_SCREEN_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	videoScreen->Indestructable = block->Indestructable;
	
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;
}


void SaveStrategy_VideoScreen(STRATEGYBLOCK* sbPtr)
{
	VIDEO_SCREEN_SAVE_BLOCK *block;
	VIDEO_SCREEN_BEHAV_BLOCK* videoScreen;
	
	videoScreen = (VIDEO_SCREEN_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	block->Indestructable = videoScreen->Indestructable;

	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

}
