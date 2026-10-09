#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_light.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pldghost.h"
#include "particle.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "pvisible.h"
#include "plat_shp.h"
#include "bh_debri.h"
#include "equipmnt.h"

extern int NormalFrameTime;
void UpdatePlacedLightState(PLACED_LIGHT_BEHAV_BLOCK* pl_bhv,STRATEGYBLOCK* sbPtr);
void UpdatePlacedLightColour(PLACED_LIGHT_BEHAV_BLOCK* pl_bhv);

void SetTextureAnimationSequence(int shapeindex,TXACTRLBLK* tac,int sequence)
{
	while(tac)
	{
		tac->tac_sequence=sequence;
		tac->tac_txah_s=GetTxAnimHeaderFromShape(tac, shapeindex);
		tac=tac->tac_next;
	}
}


void* InitPlacedLight(void* bhdata,STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_PLACEDLIGHT *toolsData = (TOOLS_DATA_PLACEDLIGHT *)bhdata;
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv;
	int i;

	LOCALASSERT(sbPtr->I_SBtype == I_BehaviourPlacedLight);
	LOCALASSERT(toolsData);

	/* create, initialise and attach a data block */
	pl_bhv = (void *)AllocateMem(sizeof(PLACED_LIGHT_BEHAV_BLOCK));
	if(!pl_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	pl_bhv->bhvr_type=I_BehaviourPlacedLight;

	sbPtr->SBdataptr = pl_bhv;
			
	/* these should be loaded */
	
	/* set default indestructibility */
	pl_bhv->Indestructable = No;

	
	/* Initialise object's stats */
	{
		NPC_DATA *NpcData;
   
		NpcData=GetThisNpcData(I_NPC_DefaultInanimate);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
	}
	
	pl_bhv->destruct_target_request=toolsData->destruct_target_request;
	for(i=0;i<SB_NAME_LENGTH;i++)
	{
		pl_bhv->destruct_target_ID[i]=toolsData->destruct_target_ID[i];
	}
	pl_bhv->destruct_target_sbptr=0;

	

	pl_bhv->sequence=toolsData->sequence;
	pl_bhv->colour_red=toolsData->colour_red;
	pl_bhv->colour_green=toolsData->colour_green;
	pl_bhv->colour_blue=toolsData->colour_blue;
	pl_bhv->colour_diff_red=toolsData->colour_diff_red;
	pl_bhv->colour_diff_green=toolsData->colour_diff_green;
	pl_bhv->colour_diff_blue=toolsData->colour_diff_blue;
	pl_bhv->fade_up_time=toolsData->fade_up_time;
	pl_bhv->fade_down_time=toolsData->fade_down_time;
	pl_bhv->up_time=toolsData->up_time;
	pl_bhv->down_time=toolsData->down_time;
	pl_bhv->timer=toolsData->timer;
	pl_bhv->flicker_timer=0;
	pl_bhv->type=toolsData->type;
	pl_bhv->on_off_type=toolsData->on_off_type;
	pl_bhv->state=toolsData->state;
	pl_bhv->on_off_state=toolsData->on_off_state;
	pl_bhv->swap_colour_and_brightness_alterations=toolsData->swap_colour_and_brightness_alterations;

	pl_bhv->on_off_timer=0;

	
	sbPtr->SBDamageBlock.Health*=toolsData->integrity;

	if(toolsData->static_light)
	{
		sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
	}
	else
	{
		sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_INANIMATE);
	}
	
	if(!sbPtr->DynPtr)
	{
		RemoveBehaviourStrategy(sbPtr);
		return 0;
	}


	sbPtr->DynPtr->Mass = toolsData->mass;
	if (toolsData->integrity > 20)
	{
		pl_bhv->Indestructable = Yes;
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

	pl_bhv->has_broken_sequence=1;
	pl_bhv->has_corona=0;

	/*check to see if object is animated.*/
	/*also check for corona flag at the same time*/
	{
		TXACTRLBLK **pptxactrlblk;		
		int item_num;
		int shape_num = toolsData->shapeIndex;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
		pptxactrlblk = &pl_bhv->inan_tac;
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
						{
							//see how many sequences there are
							int num_seq=0;
							while(pnew_txactrlblk->tac_txarray[num_seq+1])num_seq++;
							if(num_seq<3) pl_bhv->has_broken_sequence=0;
							GLOBALASSERT(num_seq>=2);
						}
					
					}
					else *pptxactrlblk = NULL; 
				}

			if((Request_PolyFlags((void *)poly)) & iflag_light_corona)
			{
				int* vertexptr= &poly->Poly1stPt;
				int num_verts=0;

				pl_bhv->has_corona=1;
				pl_bhv->corona_location.vx=0;
				pl_bhv->corona_location.vy=0;
				pl_bhv->corona_location.vz=0;

				//take the average of all the points in the polygon
				while(*vertexptr!=-1)
				{
					num_verts++;
					AddVector((VECTORCH*)&shptr->points[0][(*vertexptr)*3],&pl_bhv->corona_location);
					vertexptr++;
				}
				pl_bhv->corona_location.vx/=num_verts;
				pl_bhv->corona_location.vy/=num_verts;
				pl_bhv->corona_location.vz/=num_verts;
			}
		}
		*pptxactrlblk=0;

		pl_bhv->light=toolsData->light;
		GLOBALASSERT(pl_bhv->light);

		pl_bhv->light->RedScale=pl_bhv->colour_red;
		pl_bhv->light->GreenScale=pl_bhv->colour_green;
		pl_bhv->light->BlueScale=pl_bhv->colour_blue;
	}
	if(!pl_bhv->inan_tac)
	{
		pl_bhv->has_broken_sequence=0;
	}

	SetTextureAnimationSequence(sbPtr->shapeIndex,pl_bhv->inan_tac,pl_bhv->sequence);

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

	UpdatePlacedLightState(pl_bhv,sbPtr);
	
	/* these must be initialised for respawning objects in multiplayer game */
	pl_bhv->startingHealth = sbPtr->SBDamageBlock.Health;
	pl_bhv->startingArmour = sbPtr->SBDamageBlock.Armour;
	
	return((void*)pl_bhv);

}



void PlacedLightBehaviour(STRATEGYBLOCK *sbPtr)
{		
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
	LOCALASSERT(pl_bhv);

	if(AvP.Network!=I_No_Network)
	{
		//If playing a network game with predestroyed lights , get rid of this light
		if(netGameData.preDestroyLights)
		{
			if(sbPtr->maintainVisibility)
			{
				if (!pl_bhv->Indestructable)
				{
					KillLightForRespawn(sbPtr);
				}
			}
		}
	}	


	if(pl_bhv->inan_tac)
	{
		DISPLAYBLOCK* dptr = sbPtr->SBdptr;

		/*deal with texture animation*/
		if(dptr)
		{
			if(!dptr->ObTxAnimCtrlBlks)
			{ 
				dptr->ObTxAnimCtrlBlks = pl_bhv->inan_tac;
			}
		}
	}
	//if the light is broken , don't do anything else
	if(pl_bhv->state==Light_State_Broken) return;

	//update timer if necessary
	if(pl_bhv->on_off_state!=Light_OnOff_Off && pl_bhv->on_off_state!=Light_OnOff_On)
	{
		pl_bhv->on_off_timer+=NormalFrameTime;
		
		pl_bhv->flicker_timer%=3500;
		
		pl_bhv->flicker_timer+=NormalFrameTime;
	}
	else
	{
		if(pl_bhv->state==Light_State_StrobeUp ||
		   pl_bhv->state==Light_State_StrobeDown ||
		   pl_bhv->state==Light_State_StrobeUpDelay ||
		   pl_bhv->state==Light_State_StrobeDownDelay)
		{
			pl_bhv->timer+=NormalFrameTime;
		}
		else if(pl_bhv->state==Light_State_Flicker)
		{
			pl_bhv->flicker_timer%=1750;
			pl_bhv->flicker_timer+=NormalFrameTime;
		}
	}


	UpdatePlacedLightState(pl_bhv,sbPtr);

	//if light is near update the colour
	if(sbPtr->SBdptr)
	{
		UpdatePlacedLightColour(pl_bhv);
	}


}

void MakePlacedLightNear(STRATEGYBLOCK *sbPtr)
{
	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv=(PLACED_LIGHT_BEHAV_BLOCK*) sbPtr->SBdataptr;
	GLOBALASSERT(pl_bhv->bhvr_type==I_BehaviourPlacedLight);

    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	VisibilityDefaultObjectMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &VisibilityDefaultObjectMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 1;
	tempModule.m_lightarray = pl_bhv->light;
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

	dPtr->ObFlags3|=ObFlag3_NoLightDot;//so light will light itself

	//update the colour
	UpdatePlacedLightColour(pl_bhv);
}


void KillLightForRespawn(STRATEGYBLOCK *sbPtr)
{
	LOCALASSERT(sbPtr->SBdataptr);
	LOCALASSERT(AvP.Network!=I_No_Network);

	/* make the light invisible, and remove it from visibility management */
	sbPtr->maintainVisibility = 0;
	if(sbPtr->SBdptr) MakeObjectFar(sbPtr);
}

void RespawnLight(STRATEGYBLOCK *sbPtr)
{
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
	LOCALASSERT(pl_bhv);
	LOCALASSERT(AvP.Network!=I_No_Network);

	if(netGameData.preDestroyLights)
	{
		//don't restore lights for this type of net game
		return;
	}
		
	
	if(pl_bhv->state==Light_State_Broken)
	{
 
		sbPtr->maintainVisibility = 1;
		//MakeObjectNear(sbPtr);
	
		/* must respawn health too... */	
		sbPtr->SBDamageBlock.Health = pl_bhv->startingHealth;
		sbPtr->SBDamageBlock.Armour = pl_bhv->startingArmour;

		//reactivate the light
		pl_bhv->sequence=1;
		pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
	
		pl_bhv->on_off_state=Light_OnOff_On;

		switch(pl_bhv->type)
		{
			case Light_Type_Standard :
				pl_bhv->state=Light_State_Standard;
				break;

			case Light_Type_Strobe :
				pl_bhv->state=Light_State_StrobeUpDelay;
				break;

			case Light_Type_Flicker :
				pl_bhv->state=Light_State_Flicker;
				break;
		}

		//need to use the on animation sequence again
		SetTextureAnimationSequence(sbPtr->shapeIndex,pl_bhv->inan_tac,pl_bhv->sequence);
		
		UpdatePlacedLightState(pl_bhv,sbPtr);
	}

}


/* this global flag is used to distinguish between messages from the host, 
and locally caused damage */
extern int InanimateDamageFromNetHost;

void PlacedLightIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
	LOCALASSERT(pl_bhv);

	if(pl_bhv->state==Light_State_Broken) return;
	#if 1
	
	if((AvP.Network==I_Peer)&&(!InanimateDamageFromNetHost))
	{
		//add light damaged net message
		AddNetMsg_InanimateObjectDamaged(sbPtr,damage,multiple);
		return;
	}
	else if(AvP.Network==I_Host) 
	{
		//add light damaged net message
		if(sbPtr->SBDamageBlock.Health <= 0) AddNetMsg_InanimateObjectDestroyed(sbPtr);
	}
	#endif
		
	if (!pl_bhv->Indestructable)
	{
		
		if(sbPtr->SBDamageBlock.Health <= 0) 
		{
			//change to broken sequence if it exists.
			//otherwise destroy light 
			pl_bhv->state=Light_State_Broken;
			pl_bhv->on_off_state=Light_OnOff_Off;
			pl_bhv->sequence=2;
			pl_bhv->light->LightBright=0;
			
			//notify target of destruction
			if(pl_bhv->destruct_target_sbptr)
			{
				RequestState(pl_bhv->destruct_target_sbptr,pl_bhv->destruct_target_request,0);
			}

			/* KJL 17:08:44 10/07/98 - Make some sparks */
			if (sbPtr->SBdptr)
			{	
				MakeSprayOfSparks(&sbPtr->SBdptr->ObMat,&sbPtr->SBdptr->ObWorld);
   			}
				
			if(pl_bhv->has_broken_sequence)
			{
				SetTextureAnimationSequence(sbPtr->shapeIndex,pl_bhv->inan_tac,pl_bhv->sequence);
			}
			else
			{
				MakeFragments(sbPtr);
				
				if(AvP.Network==I_No_Network) DestroyAnyStrategyBlock(sbPtr);
				else KillLightForRespawn(sbPtr);			
			}

		}
	}
}

#define LightRequest_AdjustIntegrity 0x00000001
#define LightRequest_AdjustType_Standard 0x00000002
#define LightRequest_AdjustType_Strobe 0x00000004
#define LightRequest_AdjustType_Flicker 0x00000008
#define LightRequest_SwapUpAndDown 0x00000010
void SendRequestToPlacedLight(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbptr->SBdataptr;
	LOCALASSERT(pl_bhv);

	if(pl_bhv->state==Light_State_Broken) return;

	if(extended_data)
	{
		if(extended_data & LightRequest_AdjustIntegrity)
		{
			if(state)
			{
				int new_integrity=(extended_data>>7)&0xff;
				sbptr->SBDamageBlock.Health=(10<<ONE_FIXED_SHIFT)*new_integrity;	
				sbptr->integrity = DEFAULT_OBJECT_INTEGRITY*new_integrity;

				if(new_integrity>20)
					pl_bhv->Indestructable = Yes;
				else
					pl_bhv->Indestructable = No;
				
				if(sbptr->integrity==0)
				{
					//destroy the light by applying some damage to it
					//DAMAGE_PROFILE certainDeath = {0,0,10000,0,0,0};
					PlacedLightIsDamaged(sbptr, &certainDeath, ONE_FIXED);
				}
				
			}
		}
		if(state)
		{
			if(extended_data & (LightRequest_AdjustType_Standard|LightRequest_AdjustType_Flicker))
			{
				/* changing state, try to preserve strobe timer */
				switch(pl_bhv->state)
				{
					/* lack of break's intended */
					case Light_State_StrobeUpDelay:
						pl_bhv->timer+=pl_bhv->fade_up_time;
					
					case Light_State_StrobeUp:
						pl_bhv->timer+=pl_bhv->down_time;
					
					case Light_State_StrobeDownDelay:
						pl_bhv->timer+=pl_bhv->fade_down_time;
	
					default: ;
				}
			}
		
			if(extended_data & LightRequest_AdjustType_Standard)
			{
				pl_bhv->type=Light_Type_Standard;
				pl_bhv->state=Light_State_Standard;
				
			}
			if(extended_data & LightRequest_AdjustType_Flicker)
			{
	 			pl_bhv->type=Light_Type_Flicker;
	  			pl_bhv->state=Light_State_Flicker;
				
			}
			if(extended_data & LightRequest_AdjustType_Strobe)
			{
				if(pl_bhv->type!=Light_Type_Strobe)
				{
					pl_bhv->type=Light_Type_Strobe;
					pl_bhv->state=Light_State_StrobeDown;
				}
			}
			if(extended_data & LightRequest_SwapUpAndDown)
			{
				//swap the up and down colours for the light
				int temp_red=pl_bhv->colour_red;	
				int temp_green=pl_bhv->colour_green;	
				int temp_blue=pl_bhv->colour_blue;	
				
				pl_bhv->colour_red+=pl_bhv->colour_diff_red;
				pl_bhv->colour_green+=pl_bhv->colour_diff_green;
				pl_bhv->colour_blue+=pl_bhv->colour_diff_blue;

				pl_bhv->colour_diff_red=temp_red-pl_bhv->colour_red;
				pl_bhv->colour_diff_blue=temp_blue-pl_bhv->colour_blue;
				pl_bhv->colour_diff_green=temp_green-pl_bhv->colour_green;

			}

		}
	}
	else
	{
		if(state) //switch light on
		{
			if(pl_bhv->on_off_state!=Light_OnOff_On)
			{
				switch(pl_bhv->on_off_type)
				{
					case Light_OnOff_Type_Fade :
					{
						switch(pl_bhv->on_off_state)
						{
							case Light_OnOff_FadeOn :
								break;
							
							case Light_OnOff_FadeOff :
								{
									//reverse direction of fade
									int mult=ONE_FIXED-DIV_FIXED(pl_bhv->on_off_timer,pl_bhv->fade_down_time);
									pl_bhv->on_off_timer=MUL_FIXED(mult,pl_bhv->fade_up_time);
									pl_bhv->on_off_state=Light_OnOff_FadeOn;
								}
								break;

							case Light_OnOff_Off :
								{
									//start to fade up
									pl_bhv->on_off_timer=0;
									pl_bhv->on_off_state=Light_OnOff_FadeOn;


								}
								break;

							default :
								LOCALASSERT(1==0);
								break;
						}
					}
					break;

					case Light_OnOff_Type_Standard :
						pl_bhv->on_off_state=Light_OnOff_On;
						break;

	
					case Light_OnOff_Type_Flicker :
						if(pl_bhv->on_off_state!=Light_OnOff_Flicker)
						{
							pl_bhv->on_off_state=Light_OnOff_Flicker;
							pl_bhv->on_off_timer=0;

							//light is flickering on , so play the flickering on sound
							Sound_Play(SID_LIGHT_FLICKER_ON,"d",&sbptr->DynPtr->Position);

						}
						break;		

					default :
						LOCALASSERT(1==0);
						break;
				}

				
			}
		}
		else //switch the light off 
		{
			if(pl_bhv->on_off_state!=Light_OnOff_Off)
			{
				switch(pl_bhv->on_off_type)
				{
					case Light_OnOff_Type_Fade :
					{
						switch(pl_bhv->on_off_state)
						{
							case Light_OnOff_On :
								//start to fade down
								pl_bhv->on_off_timer=0;
								pl_bhv->on_off_state=Light_OnOff_FadeOff;
								break;

							case Light_OnOff_FadeOn :
								//reverse direction of fade
								{
									int mult=ONE_FIXED-DIV_FIXED(pl_bhv->on_off_timer,pl_bhv->fade_up_time);
									pl_bhv->on_off_timer=MUL_FIXED(mult,pl_bhv->fade_down_time);
									pl_bhv->on_off_state=Light_OnOff_FadeOff;
								}
								break;
							
							case Light_OnOff_FadeOff :
								break;


							default :
								LOCALASSERT(1==0);
								break;
						}
					}
					break;

					case Light_OnOff_Type_Standard :
					case Light_OnOff_Type_Flicker :
					{
						pl_bhv->on_off_state=Light_OnOff_Off;
						if(!pl_bhv->swap_colour_and_brightness_alterations)
						{
							pl_bhv->light->LightBright=0;
						}
						else
						{
							pl_bhv->light->RedScale=pl_bhv->colour_red+pl_bhv->colour_diff_red;
							pl_bhv->light->GreenScale=pl_bhv->colour_green+pl_bhv->colour_diff_green;
							pl_bhv->light->BlueScale=pl_bhv->colour_blue+pl_bhv->colour_diff_blue;
						}
					}
					break;

					default :
						LOCALASSERT(1==0);
						break;
				}
			}
		}
	}

	pl_bhv->sequence=(pl_bhv->on_off_state!=Light_OnOff_Off);
	
	SetTextureAnimationSequence(sbptr->shapeIndex,pl_bhv->inan_tac,pl_bhv->sequence);
 
}


void UpdatePlacedLightState(PLACED_LIGHT_BEHAV_BLOCK* pl_bhv,STRATEGYBLOCK* sbPtr)
{
	BOOL done=FALSE;
	GLOBALASSERT(pl_bhv);

	while(!done)
	{
		switch(pl_bhv->state)
		{
			case Light_State_Standard :
			case Light_State_Flicker :
			case Light_State_Broken :
				done=TRUE;
				break;


			case Light_State_StrobeUp :
				if(pl_bhv->timer>=pl_bhv->fade_up_time)
				{
					pl_bhv->timer-=pl_bhv->fade_up_time;
					pl_bhv->state=Light_State_StrobeUpDelay;
				}
				else
				{
					done=TRUE;
				}
				break;

			case Light_State_StrobeUpDelay :
				if(pl_bhv->timer>=pl_bhv->up_time)
				{
					pl_bhv->timer-=pl_bhv->up_time;
					pl_bhv->state=Light_State_StrobeDown;
				}
				else
				{
					done=TRUE;
				}
				break;

			case Light_State_StrobeDown :
				if(pl_bhv->timer>=pl_bhv->fade_down_time)
				{
					pl_bhv->timer-=pl_bhv->fade_down_time;
					pl_bhv->state=Light_State_StrobeDownDelay;
				}
				else
				{
					done=TRUE;
				}
				break;
		
			case Light_State_StrobeDownDelay :
				if(pl_bhv->timer>=pl_bhv->down_time)
				{
					pl_bhv->timer-=pl_bhv->down_time;
					pl_bhv->state=Light_State_StrobeUp;
				}
				else
				{
					done=TRUE;
				}
				break;

	 		default :
				LOCALASSERT(1==0);
				done=TRUE;
	 	}
	}
	
	switch(pl_bhv->on_off_state)
	{
		case Light_OnOff_On :
		case Light_OnOff_Off :
			break;
		
		case Light_OnOff_FadeOn :
			if(pl_bhv->on_off_timer>= pl_bhv->fade_up_time)
			{
				pl_bhv->on_off_timer=0;
				if(!pl_bhv->swap_colour_and_brightness_alterations)
				{
					pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
				}
				else
				{
					pl_bhv->light->RedScale=pl_bhv->colour_red;
					pl_bhv->light->GreenScale=pl_bhv->colour_green;
					pl_bhv->light->BlueScale=pl_bhv->colour_blue;
				}
				pl_bhv->on_off_state=Light_OnOff_On;
			}
			break;

		case Light_OnOff_FadeOff :
			if(pl_bhv->on_off_timer>= pl_bhv->fade_down_time)
			{
				pl_bhv->on_off_timer=0;
				if(!pl_bhv->swap_colour_and_brightness_alterations)
				{
					pl_bhv->light->LightBright=0;
				}
				else
				{
					pl_bhv->light->RedScale=pl_bhv->colour_red+pl_bhv->colour_diff_red;
					pl_bhv->light->GreenScale=pl_bhv->colour_green+pl_bhv->colour_diff_green;
					pl_bhv->light->BlueScale=pl_bhv->colour_blue+pl_bhv->colour_diff_blue;
				}
				pl_bhv->on_off_state=Light_OnOff_Off;
			}
			break;
			
		case Light_OnOff_Flicker :
			if(pl_bhv->on_off_timer>2*ONE_FIXED)
			{
				pl_bhv->on_off_timer=0;
				pl_bhv->on_off_state=Light_OnOff_On;
				pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
			}
			break;
		default :
			LOCALASSERT(1==0);

	}
	
}

void UpdatePlacedLightColour(PLACED_LIGHT_BEHAV_BLOCK* pl_bhv)
{
	int mult;
	GLOBALASSERT(pl_bhv);

	//first alter colour for stobing lights
	switch(pl_bhv->state)	
	{
		case Light_State_Broken :
			break;
		case Light_State_Standard :
			if(pl_bhv->on_off_state!=Light_OnOff_Off)
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red;
				pl_bhv->light->GreenScale=pl_bhv->colour_green;
				pl_bhv->light->BlueScale=pl_bhv->colour_blue;
			}
			break;
	
		case Light_State_StrobeUpDelay :
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red;
				pl_bhv->light->GreenScale=pl_bhv->colour_green;
				pl_bhv->light->BlueScale=pl_bhv->colour_blue;
			}
			else
			{
				pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
			}
			break;
		
		case Light_State_StrobeDownDelay :
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red+pl_bhv->colour_diff_red;
				pl_bhv->light->GreenScale=pl_bhv->colour_green+pl_bhv->colour_diff_green;
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+pl_bhv->colour_diff_blue;
			}
			else
			{
				pl_bhv->light->LightBright=0;
			}
			break;
		
		case Light_State_StrobeUp :
			mult=DIV_FIXED(pl_bhv->fade_up_time-pl_bhv->timer,pl_bhv->fade_up_time);

			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red+MUL_FIXED(pl_bhv->colour_diff_red,mult);
				pl_bhv->light->GreenScale=pl_bhv->colour_green+MUL_FIXED(pl_bhv->colour_diff_green,mult);
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+MUL_FIXED(pl_bhv->colour_diff_blue,mult);
			}
			else
			{
				mult=ONE_FIXED-mult;
				pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);
			}
			break;
		
		case Light_State_StrobeDown :
			mult=DIV_FIXED(pl_bhv->timer,pl_bhv->fade_down_time);
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red+MUL_FIXED(pl_bhv->colour_diff_red,mult);
				pl_bhv->light->GreenScale=pl_bhv->colour_green+MUL_FIXED(pl_bhv->colour_diff_green,mult);
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+MUL_FIXED(pl_bhv->colour_diff_blue,mult);
			}
			else
			{
				mult=ONE_FIXED-mult;
				pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);
			}
			break;

		case Light_State_Flicker :
			if(pl_bhv->on_off_state==Light_OnOff_On)
			{
				if(pl_bhv->flicker_timer>=1750)
				{
					mult= FastRandom() & 0xffff;
					if (!((mult % 24 )>>3))
					{
						mult |= 0xa000;
					}
					else
					{
						mult &=~ 0xf000;
					}
					
					pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);
					

				}
			}
			break;
		
		default :
			LOCALASSERT(1==0);
	}

	//now alter brightness for switching on/off
	switch(pl_bhv->on_off_state)
	{
		case Light_OnOff_Off :
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->LightBright=0;
			}
			else
			{
				pl_bhv->light->RedScale=pl_bhv->colour_red+pl_bhv->colour_diff_red;
				pl_bhv->light->GreenScale=pl_bhv->colour_green+pl_bhv->colour_diff_green;
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+pl_bhv->colour_diff_blue;
			}
			break;

		case Light_OnOff_On :
			if(pl_bhv->state!=Light_State_Flicker)
			{
				if(!pl_bhv->swap_colour_and_brightness_alterations)
				{
					pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
				}
				else
				{
					pl_bhv->light->RedScale=pl_bhv->colour_red;
					pl_bhv->light->GreenScale=pl_bhv->colour_green;
					pl_bhv->light->BlueScale=pl_bhv->colour_blue;
				}
			}
			break;

		case Light_OnOff_FadeOn :
			mult=DIV_FIXED(pl_bhv->on_off_timer,pl_bhv->fade_up_time);
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);
			}
			else
			{
				mult=ONE_FIXED-mult;
				pl_bhv->light->RedScale=pl_bhv->colour_red+MUL_FIXED(pl_bhv->colour_diff_red,mult);
				pl_bhv->light->GreenScale=pl_bhv->colour_green+MUL_FIXED(pl_bhv->colour_diff_green,mult);
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+MUL_FIXED(pl_bhv->colour_diff_blue,mult);
			}
			break;

		case Light_OnOff_FadeOff :
			mult=DIV_FIXED(pl_bhv->fade_down_time-pl_bhv->on_off_timer,pl_bhv->fade_down_time);
			if(!pl_bhv->swap_colour_and_brightness_alterations)
			{
				pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);
			}
			else
			{
				mult=ONE_FIXED-mult;
				pl_bhv->light->RedScale=pl_bhv->colour_red+MUL_FIXED(pl_bhv->colour_diff_red,mult);
				pl_bhv->light->GreenScale=pl_bhv->colour_green+MUL_FIXED(pl_bhv->colour_diff_green,mult);
				pl_bhv->light->BlueScale=pl_bhv->colour_blue+MUL_FIXED(pl_bhv->colour_diff_blue,mult);
			}
			break;

		case Light_OnOff_Flicker :
			if(pl_bhv->flicker_timer>=3500)
			{
				mult= FastRandom() & 0xffff;
				if (!((mult % 24 )>>3))
				{
					mult |= 0xa000;
				}
				else
				{
					mult &=~ 0xf000;
				}
				pl_bhv->light->LightBright=MUL_FIXED(pl_bhv->light->LightBrightStore,mult);

			}
			break;
	
		default :
			LOCALASSERT(1==0);
		
	}
}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct placed_light_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL Indestructable;
	
	LIGHT_TYPE type;

	LIGHT_STATE state;
	LIGHT_ON_OFF_STATE on_off_state;
	int sequence;	//texture animation sequence
	
	int colour_red;  //colour for fade up state
	int colour_green;
	int colour_blue;
	int colour_diff_red;  //difference from up colour to down colour
	int colour_diff_green;
	int colour_diff_blue;
	
	int timer;
	int on_off_timer;
	int flicker_timer;
	

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
}PLACED_LIGHT_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV pl_bhv

void LoadStrategy_PlacedLight(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv;
	PLACED_LIGHT_SAVE_BLOCK* block = (PLACED_LIGHT_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourPlacedLight) return;

	pl_bhv = sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(Indestructable);
	COPYELEMENT_LOAD(type);
	COPYELEMENT_LOAD(state);
   	COPYELEMENT_LOAD(on_off_state);
	COPYELEMENT_LOAD(sequence);	//texture animation sequence
	COPYELEMENT_LOAD(colour_red);  //colour for fade up state
	COPYELEMENT_LOAD(colour_green);
	COPYELEMENT_LOAD(colour_blue);
	COPYELEMENT_LOAD(colour_diff_red);  //difference from up colour to down colour
	COPYELEMENT_LOAD(colour_diff_green);
	COPYELEMENT_LOAD(colour_diff_blue);
	COPYELEMENT_LOAD(timer);
	COPYELEMENT_LOAD(on_off_timer);
	COPYELEMENT_LOAD(flicker_timer);
	
	
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	SetTextureAnimationSequence(sbPtr->shapeIndex,pl_bhv->inan_tac,pl_bhv->sequence);
	UpdatePlacedLightColour(pl_bhv);

}

void SaveStrategy_PlacedLight(STRATEGYBLOCK* sbPtr)
{
	PLACED_LIGHT_SAVE_BLOCK *block;
	PLACED_LIGHT_BEHAV_BLOCK* pl_bhv;
	
	pl_bhv = sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	COPYELEMENT_SAVE(Indestructable);
	COPYELEMENT_SAVE(type);
	COPYELEMENT_SAVE(state);
   	COPYELEMENT_SAVE(on_off_state);
	COPYELEMENT_SAVE(sequence);	//texture animation sequence
	COPYELEMENT_SAVE(colour_red);  //colour for fade up state
	COPYELEMENT_SAVE(colour_green);
	COPYELEMENT_SAVE(colour_blue);
	COPYELEMENT_SAVE(colour_diff_red);  //difference from up colour to down colour
	COPYELEMENT_SAVE(colour_diff_green);
	COPYELEMENT_SAVE(colour_diff_blue);
	COPYELEMENT_SAVE(timer);
	COPYELEMENT_SAVE(on_off_timer);
	COPYELEMENT_SAVE(flicker_timer);

	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

}
