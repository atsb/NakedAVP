#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "dynblock.h"
#include "dynamics.h"
#include "plat_shp.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "bh_ltfx.h"

extern int NormalFrameTime;


void * LightFXBehaveInit (void * bhdata, STRATEGYBLOCK* sbptr)
{
	LIGHT_FX_BEHAV_BLOCK * lfxbb;
	LIGHT_FX_TOOLS_TEMPLATE * lfxtt;
	MODULE * my_mod;

 	GLOBALASSERT(sbptr);
	
	lfxbb = (LIGHT_FX_BEHAV_BLOCK *)AllocateMem(sizeof(LIGHT_FX_BEHAV_BLOCK));
	if (!lfxbb)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	lfxtt = (LIGHT_FX_TOOLS_TEMPLATE *)bhdata;
	
	COPY_NAME (sbptr->SBname, lfxtt->nameID);
	lfxbb->bhvr_type = I_BehaviourLightFX;

	// Setup module ref
	{
		MREF mref=lfxtt->my_module;
		ConvertModuleNameToPointer (&mref, MainSceneArray[0]->sm_marray);
		my_mod = mref.mref_ptr;
	}
	my_mod->m_sbptr = sbptr;
	sbptr->SBmoptr = my_mod;
	sbptr->SBmomptr = my_mod->m_mapptr;
	sbptr->SBflags.no_displayblock = 1;

	GLOBALASSERT (my_mod);

	lfxbb->type = lfxtt->light_data.type;
	lfxbb->current_state = lfxtt->light_data.init_state;
	
	lfxbb->fade_up_speed = lfxtt->light_data.fade_up_speed;
	lfxbb->fade_down_speed = lfxtt->light_data.fade_down_speed;
	
	lfxbb->post_fade_up_delay = lfxtt->light_data.post_fade_up_delay;
	lfxbb->post_fade_down_delay = lfxtt->light_data.post_fade_down_delay;
	
	if (lfxbb->fade_up_speed == 0)
		lfxbb->fade_up_speed = 10;
	
	if (lfxbb->fade_down_speed == 0)
		lfxbb->fade_down_speed = 10;
	
	if (lfxbb->post_fade_up_delay == 0)
		lfxbb->post_fade_up_delay = 10;
	
	if (lfxbb->post_fade_down_delay == 0)
		lfxbb->post_fade_down_delay = 10;
	
	lfxbb->fade_up_speed_multiplier = DIV_FIXED (ONE_FIXED, lfxbb->fade_up_speed);
	lfxbb->fade_down_speed_multiplier = DIV_FIXED (ONE_FIXED, lfxbb->fade_down_speed);
	
	lfxbb->post_fade_up_delay_multiplier = DIV_FIXED (ONE_FIXED, lfxbb->post_fade_up_delay);
	lfxbb->post_fade_down_delay_multiplier = DIV_FIXED (ONE_FIXED, lfxbb->post_fade_down_delay);
	
	switch (lfxbb->type)
	{
		case LFX_Strobe:
		case LFX_Switch:
		case LFX_FlickySwitch:
		{
			switch (lfxbb->current_state)
			{
				case LFXS_LightOn:
				{
					lfxbb->multiplier = ONE_FIXED;
					lfxbb->timer = 0;
					lfxbb->timer2 = 0;
					break;
				}
				
				case LFXS_LightOff:
				{
					lfxbb->multiplier = 0;
					lfxbb->timer = 0;
					lfxbb->timer2 = 0;
					break;
				}
				
				default:
				{
					lfxbb->multiplier = ONE_FIXED;
					lfxbb->timer = 0;
					lfxbb->timer2 = 0;
					break;
				}
			}
			break;
		}
		
		case LFX_RandomFlicker:
		{
			lfxbb->current_state = LFXS_Flicking;
			lfxbb->multiplier = ONE_FIXED;
			lfxbb->timer = 0;
			lfxbb->timer2 = 0;
			lfxbb->time_to_next_flicker_state = 0;
			break;
		}
		
		default:
		{
			lfxbb->multiplier = ONE_FIXED;
			lfxbb->timer = 0;
			lfxbb->timer2 = 0;
			lfxbb->time_to_next_flicker_state = 0;
			break;
		}
	}

	/* see if this module has a texture animation*/
	{
		TXACTRLBLK **pptxactrlblk;		
		int item_num;
		int shape_num=sbptr->shapeIndex;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
		
		pptxactrlblk = &lfxbb->anim_control;

		for(item_num = 0; item_num < shptr->numitems; item_num ++)
		{
			POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
			LOCALASSERT(poly);
				
			if((Request_PolyFlags((void *)poly)) & iflag_txanim)
				{
					TXACTRLBLK *pnew_txactrlblk;

					pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
					if (pnew_txactrlblk)
					{
 						pnew_txactrlblk->tac_flags = 0;										
						pnew_txactrlblk->tac_item = item_num;										
						pnew_txactrlblk->tac_sequence = 0;										
						pnew_txactrlblk->tac_node = 0;										
						pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);										
						pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

					
						/* change the value held in pptxactrlblk
						 which point to the previous structures "next"
						 pointer*/

						*pptxactrlblk = pnew_txactrlblk;
						pptxactrlblk = &pnew_txactrlblk->tac_next;
					}
					else
					{
						memoryInitialisationFailure = 1;
					}
				}
		}
		*pptxactrlblk=0;
		
	}
	
	return((void*)lfxbb);
}

void LightFXBehaveFun (STRATEGYBLOCK* sbptr)
{
	LIGHT_FX_BEHAV_BLOCK * lfxbb;
	DISPLAYBLOCK* dptr;
	MODULE *mptr;
	
 	GLOBALASSERT(sbptr);

	mptr = sbptr->SBmoptr;
	dptr = sbptr->SBdptr;
	
	lfxbb = (LIGHT_FX_BEHAV_BLOCK *)sbptr->SBdataptr;
	GLOBALASSERT((lfxbb->bhvr_type == I_BehaviourLightFX));
	
	/*deal with any texture animation*/
	if(lfxbb->anim_control)
	{
		if(dptr)
		{
			if(!dptr->ObTxAnimCtrlBlks)
			{ 
				dptr->ObTxAnimCtrlBlks = lfxbb->anim_control;
			}
		}
	}

	/*now update the lighting effects*/
	switch (lfxbb->type)
	{
		case LFX_RandomFlicker:
		{
			if (dptr)
			{
				switch (lfxbb->current_state)
				{
					case LFXS_Flicking:
					{
	
						lfxbb->time_to_next_flicker_state -= NormalFrameTime;
						
						if (lfxbb->time_to_next_flicker_state < 0 && 0)
						{
							int j;
							lfxbb->multiplier = FastRandom() & 65535;

							lfxbb->current_state = LFXS_NotFlicking;

							lfxbb->time_to_next_flicker_state = ((FastRandom() & 65535));

							lfxbb->multiplier &= ~0xf000;

							for (j=0; j<dptr->ObNumLights; j++)
							{
								LIGHTBLOCK * lp = dptr->ObLights[j];
								if (!(lp->LightFlags & LFlag_PreLitSource))
								{
									lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
								}
							}
							lfxbb->timer = 0;					
						}
						else
						{
							lfxbb->timer += NormalFrameTime;
							if (lfxbb->timer > 1750)
							{
								int j;
								lfxbb->multiplier = FastRandom() & 65535;
								if (!((lfxbb->multiplier % 24 )>>3))
								{
									lfxbb->multiplier |= 0xa000;
								}
								else
								{
									lfxbb->multiplier &= ~0xf000;
								}
								
								for (j=0; j<dptr->ObNumLights; j++)
								{
									LIGHTBLOCK * lp = dptr->ObLights[j];
									if (!(lp->LightFlags & LFlag_PreLitSource))
									{
										lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
									}
								}
								lfxbb->timer = 0;					
							}
							
						}
						
						break;
					}
					
					case LFXS_NotFlicking:
					{
						lfxbb->time_to_next_flicker_state -= NormalFrameTime;
						if (lfxbb->time_to_next_flicker_state < 0)
						{
							lfxbb->current_state = LFXS_Flicking;
							lfxbb->time_to_next_flicker_state = FastRandom() & 65535;
						}
						
						break;
					}

					case LFXS_LightOff :
					{
						int j;
						for (j=0; j<dptr->ObNumLights; j++)
						{
							LIGHTBLOCK * lp = dptr->ObLights[j];
							if (!(lp->LightFlags & LFlag_PreLitSource))
							{
								lp->LightBright = 0;
							}
						}
						break;
					}
					
					default:
					{
						break;
					}
				}
			
				
			}
			break;
		}
		
		case LFX_Strobe:
		{
			switch (lfxbb->current_state)
			{
				case LFXS_LightOn:
				{
					lfxbb->timer += MUL_FIXED (NormalFrameTime, lfxbb->post_fade_up_delay_multiplier);
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightFadingDown;
						lfxbb->timer = 0;
					}
					break;
				}
				
				case LFXS_LightOff:
				{
					lfxbb->timer += MUL_FIXED (NormalFrameTime, lfxbb->post_fade_down_delay_multiplier);
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightFadingUp;
						lfxbb->timer = 0;
					}
					break;
				}
				
				case LFXS_LightFadingUp:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_up_speed_multiplier);
					lfxbb->timer += diff;
					lfxbb->multiplier += diff;
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOn;
						lfxbb->timer = 0;
						lfxbb->multiplier = 65536;
					}
					break;
				}
				
				case LFXS_LightFadingDown:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_down_speed_multiplier);
					lfxbb->timer += diff;
					lfxbb->multiplier -= diff;
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOff;
						lfxbb->timer = 0;
						lfxbb->multiplier = 0;
					}
					break;
				}
				
				default:
				{
					GLOBALASSERT (0 == "Light FX state not supported");
					break;
				}
			}

			if (dptr)
			{
				int j;
				for (j=0; j<dptr->ObNumLights; j++)
				{
					LIGHTBLOCK * lp = dptr->ObLights[j];
					if (!(lp->LightFlags & LFlag_PreLitSource))
					{
						lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
					}
				}
				
			}

			break;
		}
		
		case LFX_Switch:
		{
			switch (lfxbb->current_state)
			{
				case LFXS_LightFadingUp:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_up_speed_multiplier);
					lfxbb->timer += diff;
					lfxbb->multiplier += diff;

					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOn;
						lfxbb->timer = 0;
						lfxbb->multiplier = 65536;
					}
					break;
				}
				
				case LFXS_LightFadingDown:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_down_speed_multiplier);
					lfxbb->timer += diff;
					lfxbb->multiplier -= diff;
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOff;
						lfxbb->timer = 0;
						lfxbb->multiplier = 0;
					}
					break;
				}

				case LFXS_LightOn:
				case LFXS_LightOff:
				{
					break;
				}
				
				default:
				{
					GLOBALASSERT (0 == "Light FX state not supported");
					break;
				}
				
			}
		
			if (dptr)
			{
				int j;
				for (j=0; j<dptr->ObNumLights; j++)
				{
					LIGHTBLOCK * lp = dptr->ObLights[j];
					if (!(lp->LightFlags & LFlag_PreLitSource))
					{
						lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
					}
				}
				
			}

			break;
		}
		
		case LFX_FlickySwitch:
		{
			switch (lfxbb->current_state)
			{
				case LFXS_LightFadingUp:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_up_speed_multiplier);

					lfxbb->timer += diff;
					
					
					lfxbb->timer2 += NormalFrameTime;
					if (lfxbb->timer2 > 3553)
					{
						lfxbb->multiplier = FastRandom() & 65535;
						if (!((lfxbb->multiplier % 24 )>>3))
						{
							lfxbb->multiplier |= 0xa000;
						}
						else
						{
							lfxbb->multiplier &= ~0xf000;
						}
						
						lfxbb->timer2 = 0;					
					}

					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOn;
						lfxbb->timer = 0;
						lfxbb->multiplier = 65536;
					}
					break;
				}
				
				case LFXS_LightFadingDown:
				{
					int diff = MUL_FIXED (NormalFrameTime, lfxbb->fade_down_speed_multiplier);
					lfxbb->timer += diff;
					lfxbb->multiplier -= diff;
					if (lfxbb->timer > ONE_FIXED)
					{
						lfxbb->current_state = LFXS_LightOff;
						lfxbb->timer = 0;
						lfxbb->multiplier = 0;
					}
					break;
				}

				case LFXS_LightOn:
				case LFXS_LightOff:
				{
					break;
				}
				
				default:
				{
					GLOBALASSERT (0 == "Light FX state not supported");
					break;
				}
				
			}
		
			if (dptr)
			{
				int j;
				for (j=0; j<dptr->ObNumLights; j++)
				{
					LIGHTBLOCK * lp = dptr->ObLights[j];
					if (!(lp->LightFlags & LFlag_PreLitSource))
					{
						lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
					}
				}
				
			}

			break;
		}

		default:
		{
			GLOBALASSERT (0 == "Light FX type not supported");
			break;
		}
	}
}

/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct light_fx_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	LIGHT_FX_STATE current_state;
	signed long multiplier;
	unsigned long timer;
	unsigned long timer2;
	signed long time_to_next_flicker_state;

}LIGHT_FX_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV lfxbb

void LoadStrategy_LightFx(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	LIGHT_FX_BEHAV_BLOCK * lfxbb;
	LIGHT_FX_SAVE_BLOCK* block = (LIGHT_FX_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourLightFX) return;

	lfxbb = (LIGHT_FX_BEHAV_BLOCK *)sbPtr->SBdataptr;

	//start copying stuff

	COPYELEMENT_LOAD(current_state)
	COPYELEMENT_LOAD(multiplier)
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(timer2)
	COPYELEMENT_LOAD(time_to_next_flicker_state)


	//update the brightness of the lights
	{
		DISPLAYBLOCK* dptr = sbPtr->SBdptr;;
		if(dptr)
		{
			//I'm not sure that we will ever have a displayblock at this point, anyway. hmm.
			int j;
			for (j=0; j<dptr->ObNumLights; j++)
			{
				LIGHTBLOCK * lp = dptr->ObLights[j];
				if (!(lp->LightFlags & LFlag_PreLitSource))
				{
					lp->LightBright = MUL_FIXED (lp->LightBrightStore, lfxbb->multiplier);
				}
			}
		}
	}

	
}

void SaveStrategy_LightFx(STRATEGYBLOCK* sbPtr)
{
	LIGHT_FX_SAVE_BLOCK *block;
	LIGHT_FX_BEHAV_BLOCK * lfxbb;
	
	lfxbb = (LIGHT_FX_BEHAV_BLOCK *)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(current_state)
	COPYELEMENT_SAVE(multiplier)
	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(timer2)
	COPYELEMENT_SAVE(time_to_next_flicker_state)
}
