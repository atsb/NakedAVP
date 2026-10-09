#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "bh_lnksw.h"

#include "dynblock.h"
#include "dynamics.h"
#include "pldghost.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "bh_binsw.h"
#include "plat_shp.h"
#include "psnd.h"
#include "inventry.h"

extern int NormalFrameTime;
extern int RealFrameTime;

static BOOL check_link_switch_states (LINK_SWITCH_BEHAV_BLOCK * lsbb)
{
	int i=0;
	LSWITCH_ITEM * lsi = lsbb->lswitch_list;

//	textprint ("Checking link states\n");

	if (!lsbb->state)
		return(No);

//	textprint ("Link switch OK\n");

	while (i < lsbb->num_linked_switches)
	{
		if(lsi[i].bswitch->I_SBtype==I_BehaviourBinarySwitch)
		{
			BINARY_SWITCH_BEHAV_BLOCK * bsbb = ((BINARY_SWITCH_BEHAV_BLOCK *)lsi[i].bswitch->SBdataptr);

			// if it's off return No
			if (!bsbb->state)
				return(No);
//			textprint ("Switch %d OK\n", i);
		
		}
		else if(lsi[i].bswitch->I_SBtype==I_BehaviourLinkSwitch)
		{
			LINK_SWITCH_BEHAV_BLOCK * linked_lsbb = ((LINK_SWITCH_BEHAV_BLOCK *)lsi[i].bswitch->SBdataptr);

			// if the system state is off return No
			if (!linked_lsbb->system_state)
				return(No);
		}
		else
		{
			GLOBALASSERT(0=="Switch should only have links to link switches and binary switches");
		}
		i++;
	}

//	textprint ("Link switchs activated\n");

	return(Yes);
}

#if 0
static void set_link_switch_states_off (LINK_SWITCH_BEHAV_BLOCK * lsbb)
{
	int i=0;
	LSWITCH_ITEM * lsi = lsbb->lswitch_list;

	while (lsi[i].bswitch && i < MAX_SWITCHES_FOR_LINK)
	{
		BINARY_SWITCH_BEHAV_BLOCK * bsbb = ((BINARY_SWITCH_BEHAV_BLOCK *)lsi[i].bswitch->SBdataptr);
		
		// if it's on, tell it to go off
		
		if (! ((bsbb->state && bsbb->rest_state) || (!bsbb->state && !bsbb->rest_state)) )
			RequestState (lsi->bswitch, 0, 0);
		i++;
	}
}
#endif

void* LinkSwitchBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
	LINK_SWITCH_TOOLS_TEMPLATE *ls_tt;
	int i;

 	GLOBALASSERT(sbptr);
	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)AllocateMem(sizeof(LINK_SWITCH_BEHAV_BLOCK));
	if(!ls_bhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	ls_bhv->bhvr_type = I_BehaviourLinkSwitch;

	// from loaders
	// 1 rest_state - on or off
	// 2 mode
	// 3 timer switch - time for reset
	// 4 security clerance to operate
	// 5 copy the target name

	ls_tt = (LINK_SWITCH_TOOLS_TEMPLATE*)bhdata;

	sbptr->shapeIndex = ls_tt->shape_num;
	COPY_NAME(sbptr->SBname, ls_tt->nameID);

	
	if (ls_tt->mode == I_lswitch_SELFDESTRUCT)
	{
		ls_bhv->ls_mode = I_lswitch_timer;
		ls_bhv->IS_SELF_DESTRUCT = Yes;
	}
	else
	{
		ls_bhv->ls_mode = ls_tt->mode;
		ls_bhv->IS_SELF_DESTRUCT = No;
	}
	
	ls_bhv->num_targets=ls_tt->num_targets;
	if(ls_bhv->num_targets)
	{
		ls_bhv->ls_targets = (LINK_SWITCH_TARGET*)AllocateMem(sizeof(LINK_SWITCH_TARGET) * ls_tt->num_targets);
		if (!ls_bhv->ls_targets)
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}
	}
	else
	{
		ls_bhv->ls_targets=0;
	}
	for (i=0; i<ls_tt->num_targets; i++)
	{
		ls_bhv->ls_targets[i]=ls_tt->targets[i];
		ls_bhv->ls_targets[i].sbptr = 0;
	}

	

	ls_bhv->time_for_reset = ls_tt->time_for_reset;
	ls_bhv->security_clerance	= ls_tt->security_clearance;
	
	ls_bhv->switch_flags=ls_tt->switch_flags;
	ls_bhv->trigger_volume_min=ls_tt->trigger_volume_min;	
	ls_bhv->trigger_volume_max=ls_tt->trigger_volume_max;	
	
	ls_bhv->switch_always_on = ls_tt->switch_always_on;
	ls_bhv->switch_off_message_same=ls_tt->switch_off_message_same;	
	ls_bhv->switch_off_message_none=ls_tt->switch_off_message_none;	

	if(sbptr->DynPtr) //there may be no shape
	{
		sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = ls_tt->position;
		sbptr->DynPtr->OrientEuler = ls_tt->orientation;
		CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
		TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	
	}
	// set up the animation control
	if(sbptr->shapeIndex!=-1)
	{
		int item_num;
		TXACTRLBLK **pptxactrlblk;		
		int shape_num = ls_tt->shape_num;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
 
		SetupPolygonFlagAccessForShape(shptr);

		pptxactrlblk = &ls_bhv->ls_tac;

		for(item_num = 0; item_num < shptr->numitems; item_num ++)
		{
			POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
			LOCALASSERT(poly);
				
			if((Request_PolyFlags((void *)poly)) & iflag_txanim)
			{
				TXACTRLBLK *pnew_txactrlblk;
				int num_seq = 0;

				pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
				if (pnew_txactrlblk)
				{
				
					pnew_txactrlblk->tac_flags = 0;
					pnew_txactrlblk->tac_item = item_num;
					pnew_txactrlblk->tac_sequence = ls_tt->rest_state;
					pnew_txactrlblk->tac_node = 0;
					pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);
					pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

					while(pnew_txactrlblk->tac_txarray[num_seq+1])num_seq++;

					// Assert does not work at this point so
					GLOBALASSERT(num_seq==2);

					/* set the flags in the animation header */
					// we only ever have one frame of animation per sequence -
					// nb this can change talk to richard - one sequence with two frames
					// or mutliple sequences???

					//Now two sequences with an arbitrary number of frames - Richard
					
					pnew_txactrlblk->tac_txah.txa_flags |= txa_flag_play;

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
	else
	{
		//no shape - so there won't be any animation
		ls_bhv->ls_tac=0;
	}


	ls_bhv->ls_dtype = linkswitch_no_display;


	if (ls_bhv->ls_tac)
	{
		ls_bhv->ls_dtype = linkswitch_animate_me;
	}
	ls_bhv->ls_track=ls_tt->track;

	if (ls_bhv->ls_track)
	{
		ls_bhv->ls_track->sbptr=sbptr;

		if (ls_bhv->ls_dtype == linkswitch_animate_me)
		{
			ls_bhv->ls_dtype = linkswitch_animate_and_move_me;
		}
		else
		{
			ls_bhv->ls_dtype = linkswitch_move_me;
		}
	}

	// fill in the rest ourselves

	ls_bhv->request = 0;
	ls_bhv->state = ls_tt->rest_state;
	ls_bhv->timer = 0;	
	ls_bhv->system_state = 0;
	
	ls_bhv->soundHandle = SOUND_NOACTIVEINDEX;
	
	ls_bhv->num_linked_switches=ls_tt->num_linked_switches;
	if(ls_tt->num_linked_switches)
		ls_bhv->lswitch_list=(LSWITCH_ITEM*)AllocateMem(sizeof(LSWITCH_ITEM)*ls_bhv->num_linked_switches);
	else
		ls_bhv->lswitch_list=0;

	for (i=0; i<ls_tt->num_linked_switches; i++)
	{
		COPY_NAME (ls_bhv->lswitch_list[i].bs_name, ls_tt->switchIDs[i].name);
	}

	if(ls_bhv->state)
	{
		ls_bhv->timer=ls_bhv->time_for_reset;	
		if(ls_bhv->ls_track)
		{
			//set the track to the end position
			ls_bhv->ls_track->current_section=(ls_bhv->ls_track->num_sections-1);
			ls_bhv->ls_track->timer=ls_bhv->ls_track->sections[ls_bhv->ls_track->current_section].time_for_section;
			ls_bhv->ls_track->playing=1;
			Update_Track_Position(ls_bhv->ls_track);

		}
	}
	
	ls_bhv->TimeUntilNetSynchAllowed=0;
	
	return((void*)ls_bhv);
}

void LinkSwitchBehaveFun(STRATEGYBLOCK* sbptr)
{
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
	DISPLAYBLOCK* dptr;
	int i;
 	
 	GLOBALASSERT(sbptr);
	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((ls_bhv->bhvr_type == I_BehaviourLinkSwitch));
  	dptr = sbptr->SBdptr;

//	if(AvP.Network!=I_No_Network) return; /* disable for network game */

	/****** 
		What I need to do - check to see if we have
		a request - requests have different effects depending on 
		the mode - so we have to switch on the mode
	*****/

	if (ls_bhv->ls_dtype == linkswitch_animate_me || ls_bhv->ls_dtype == linkswitch_animate_and_move_me)
	{
		if(dptr)
			dptr->ObTxAnimCtrlBlks = ls_bhv->ls_tac;
	}

	if (!ReturnPlayerSecurityClearance(0,ls_bhv->security_clerance) && ls_bhv->security_clerance)
	{
		ls_bhv->request = I_no_request;
		return;
	}

	if(ls_bhv->switch_flags && SwitchFlag_UseTriggerVolume)
	{
		/*See if switch has been set off*/
		int i;
		for (i=0; i<NumActiveStBlocks; i++)
		{
			int needToTest = 0;
			STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

			if (sbPtr->DynPtr)
			{
				if (sbPtr->SBdptr == Player)
				{
					needToTest = 1;
				}
				else if (sbPtr->I_SBtype == I_BehaviourNetGhost)
				{
					NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
					if ((ghostData->type == I_BehaviourMarinePlayer)
						||(ghostData->type == I_BehaviourAlienPlayer)
						||(ghostData->type == I_BehaviourPredatorPlayer))
						needToTest = 1;
				}
			}
			
			if(needToTest&&
				sbPtr->DynPtr->Position.vx > ls_bhv->trigger_volume_min.vx &&	
				sbPtr->DynPtr->Position.vx < ls_bhv->trigger_volume_max.vx &&
				sbPtr->DynPtr->Position.vy > ls_bhv->trigger_volume_min.vy &&
				sbPtr->DynPtr->Position.vy < ls_bhv->trigger_volume_max.vy &&
				sbPtr->DynPtr->Position.vz > ls_bhv->trigger_volume_min.vz &&
				sbPtr->DynPtr->Position.vz < ls_bhv->trigger_volume_max.vz)
	    	{
	    		ls_bhv->request=I_request_on;
				break;
	    	}
    	} 
	}
	
	if (ls_bhv->request == I_request_on)
	{
		if (ls_bhv->triggered_last)
		{
			ls_bhv->request = I_no_request;
		}
		else
		{
			ls_bhv->triggered_last = Yes;
		}
	}
	else
	{
		ls_bhv->triggered_last = No;
	}

	if(ls_bhv->switch_always_on)
	{
		ls_bhv->request=I_no_request;
		ls_bhv->state=1;

	}

	if(AvP.Network != I_No_Network)
	{
		/*
		Every time a switch is updated there is a time delay of 5 seconds before the
		switch can next be changed by the host sending synch messages.
		This prevents the host machine from resetting a switch before it learns that
		it has been pressed
		*/
		if(ls_bhv->request == I_no_request)
		{
			ls_bhv->TimeUntilNetSynchAllowed-=RealFrameTime;
			if(ls_bhv->TimeUntilNetSynchAllowed<0)
			{
				ls_bhv->TimeUntilNetSynchAllowed=0;
			}
		}
		else
		{
			ls_bhv->TimeUntilNetSynchAllowed=5*ONE_FIXED;
		}
	}

	switch(ls_bhv->ls_mode)
		{
			case I_lswitch_timer:
				{

					if(ls_bhv->request == I_request_on && !ls_bhv->state) 
					{
						if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
						{
							if(!ls_bhv->ls_track || !ls_bhv->ls_track->sound)
							{
								if (ls_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
					 			{
					 				Sound_Play(SID_SWITCH1,"eh",&ls_bhv->soundHandle);
					 			}
							}
						}

					 	ls_bhv->timer = ls_bhv->time_for_reset;

						if (ls_bhv->ls_dtype == binswitch_move_me || ls_bhv->ls_dtype == binswitch_animate_and_move_me)
						{
							// moving switch
							ls_bhv->new_state = 1;
							ls_bhv->new_request = -1;
							ls_bhv->ls_track->reverse=0;
							Start_Track_Playing(ls_bhv->ls_track);
							ls_bhv->mode_store = ls_bhv->ls_mode;
							ls_bhv->ls_mode = I_lswitch_moving;
						}
						else
						{
						 	ls_bhv->state = 1;
						}


						if(ls_bhv->ls_tac)
						{
							ls_bhv->ls_tac->tac_sequence = 1;
							ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbptr->shapeIndex));
						}
					}
					else if(ls_bhv->timer > 0)
					{
				 		ls_bhv->timer -= NormalFrameTime;
						if(ls_bhv->timer <= 0)
						{

							if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
							{
								if(!ls_bhv->ls_track || !ls_bhv->ls_track->sound)
								{
									if (ls_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
					 				{
					 					Sound_Play(SID_SWITCH2,"eh",&ls_bhv->soundHandle);
					 				}
								}
							}
							
							ls_bhv->state = 0;

							if (ls_bhv->ls_dtype == binswitch_move_me || ls_bhv->ls_dtype == binswitch_animate_and_move_me)
							{
								// moving switch
								ls_bhv->new_state = 0;
								ls_bhv->new_request = -1;
								ls_bhv->ls_track->reverse=1;
								Start_Track_Playing(ls_bhv->ls_track);
								ls_bhv->mode_store = ls_bhv->ls_mode;
								ls_bhv->ls_mode = I_lswitch_moving;
							}


							if(ls_bhv->ls_tac)
							{
								ls_bhv->ls_tac->tac_sequence = 0;
								ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbptr->shapeIndex));
							}
						}						
					}
					break;				
				}
			
			case I_lswitch_toggle:
				{
					// if it's off and no request then we can return
					
					if (!ls_bhv->state)
						if(ls_bhv->request == I_no_request)
							return;

					/* change the state and request the new state in
							the target */

					if(ls_bhv->request != I_no_request)
					{
						if(ls_bhv->ls_dtype == binswitch_move_me || ls_bhv->ls_dtype == binswitch_animate_and_move_me)
						{
							// moving switch
							ls_bhv->new_state = !ls_bhv->state;
							ls_bhv->new_request = -1;
							ls_bhv->mode_store = ls_bhv->ls_mode;
							ls_bhv->ls_mode = I_lswitch_moving;
							ls_bhv->ls_track->reverse=ls_bhv->state;
							Start_Track_Playing(ls_bhv->ls_track);
						}
						else
						{
							ls_bhv->state = !ls_bhv->state;
						}
	
						if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
						{
							if(!ls_bhv->ls_track || !ls_bhv->ls_track->sound)
							{
								if (ls_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
					 			{
					 				Sound_Play(SID_SWITCH1,"eh",&ls_bhv->soundHandle);
					 			}
							}
						}
						
						if(ls_bhv->ls_tac)
						{
							ls_bhv->ls_tac->tac_sequence = ls_bhv->state ? 1 : 0;
							ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbptr->shapeIndex));
						}
						
					}


					break;				
				}
			case I_lswitch_wait:
				{
					// if it's off and no request then we can return
					
					if (!ls_bhv->state)
						if(ls_bhv->request == I_no_request)
							return;

					if(ls_bhv->request == I_request_on)
					{
						if(!ls_bhv->state)//can only be switched on if currently off
						{
							if(!ls_bhv->ls_track || !ls_bhv->ls_track->sound)
							{
								Sound_Play(SID_SWITCH2,"eh",&ls_bhv->soundHandle);
							}
							if(ls_bhv->ls_dtype == binswitch_move_me || ls_bhv->ls_dtype == binswitch_animate_and_move_me)
							{
						
								// moving switch
								ls_bhv->new_state = 1;
								ls_bhv->new_request = -1;
								ls_bhv->ls_track->reverse=0;
								Start_Track_Playing(ls_bhv->ls_track);
								ls_bhv->mode_store = ls_bhv->ls_mode;
								ls_bhv->ls_mode = I_lswitch_moving;
							}
							else
							{
								ls_bhv->state = 1;
							}
						}
					}
					else if	(ls_bhv->request == I_request_off)
					{
						if(ls_bhv->state)//can only be switched off if currently on
						{
							if(!ls_bhv->ls_track || !ls_bhv->ls_track->sound)
							{
								Sound_Play(SID_SWITCH1,"eh",&ls_bhv->soundHandle);
							}
							if(ls_bhv->ls_dtype == binswitch_move_me || ls_bhv->ls_dtype == binswitch_animate_and_move_me)
							{
						
								// moving switch
								ls_bhv->new_state = 0;
								ls_bhv->new_request = -1;
								ls_bhv->ls_track->reverse=1;
								Start_Track_Playing(ls_bhv->ls_track);
								ls_bhv->mode_store = ls_bhv->ls_mode;
								ls_bhv->ls_mode = I_lswitch_moving;
							}
							else
							{
								ls_bhv->state = 0;
							}
						}
						
						
					}										
												
					if(ls_bhv->ls_tac)
					{
						ls_bhv->ls_tac->tac_sequence = ls_bhv->state ? 1 : 0;
						ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbptr->shapeIndex));
					}



					break;
				}
			case I_lswitch_moving:
				{
					textprint ("moving\n");
					Update_Track_Position(ls_bhv->ls_track);
					
					if (!ls_bhv->ls_track->playing)
					{
						ls_bhv->ls_mode = ls_bhv->mode_store;
						ls_bhv->state = ls_bhv->new_state;

						if(ls_bhv->ls_tac)
						{
							ls_bhv->ls_tac->tac_sequence = ls_bhv->state ? 1 : 0;
							ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbptr->shapeIndex));
						}
					}

				}
				break;
			default:
				GLOBALASSERT(2<1);
		}

	ls_bhv->request = I_no_request;
	
	//check to see if the system state has changed
	
	if (ls_bhv->system_state)
	{
		if (!check_link_switch_states(ls_bhv))
		{
			ls_bhv->system_state = No;
			
			//link switch system state is turning off
			if(!ls_bhv->switch_off_message_none)
			{
				for(i=0;i<ls_bhv->num_targets;i++)
				{
					RequestState(ls_bhv->ls_targets[i].sbptr,ls_bhv->ls_targets[i].request_message^(!ls_bhv->switch_off_message_same), sbptr);
				}
			}
		}
	}
	else
	{
		if (check_link_switch_states(ls_bhv))
		{
			ls_bhv->system_state = Yes;
			//link switch system state is turning on
			for(i=0;i<ls_bhv->num_targets;i++)
			{
				RequestState(ls_bhv->ls_targets[i].sbptr,ls_bhv->ls_targets[i].request_message, sbptr);
			}
		}
		
	}
	

}


#define LINKSWITCHSYNCH_ON	 0
#define LINKSWITCHSYNCH_OFF	 1
#define LINKSWITCHSYNCH_IGNORE 2

int LinkSwitchGetSynchData(STRATEGYBLOCK* sbPtr)
{
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
 	GLOBALASSERT(sbPtr);
	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GLOBALASSERT((ls_bhv->bhvr_type == I_BehaviourLinkSwitch));

	//don't try to synch moving switches
	if(ls_bhv->ls_mode==I_lswitch_moving)
	{
		return LINKSWITCHSYNCH_IGNORE;
	}

	if(ls_bhv->state)
		return LINKSWITCHSYNCH_ON;
	else	
		return LINKSWITCHSYNCH_OFF;
}


void LinkSwitchSetSynchData(STRATEGYBLOCK* sbPtr,int status)
{
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
 	GLOBALASSERT(sbPtr);
	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GLOBALASSERT((ls_bhv->bhvr_type == I_BehaviourLinkSwitch));

	if(ls_bhv->TimeUntilNetSynchAllowed>0)
	{
		//ignore this attempt to synch the switch
		return;
	}

	//don't try to synch moving switches
	if(ls_bhv->ls_mode==I_lswitch_moving)
	{
		return;
	}
	
	
	switch(status)
	{
		case LINKSWITCHSYNCH_ON :
			if(!ls_bhv->state)
			{
				//this switch should be on
				RequestState(sbPtr,1,0);
			}
			break;

		case LINKSWITCHSYNCH_OFF :
			if(ls_bhv->state)
			{
				//this switch should be off
				RequestState(sbPtr,0,0);
			}
			break;
	}
}

/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct link_switch_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BINARY_SWITCH_REQUEST_STATE request;
	BOOL system_state;
	BOOL state;

	LSWITCH_MODE ls_mode;
	int timer;
	
	BOOL new_state;
	int new_request;
	
	LSWITCH_MODE mode_store;

	BOOL triggered_last;
	
	int txanim_sequence;

}LINK_SWITCH_SAVE_BLOCK;


//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV ls_bhv

void LoadStrategy_LinkSwitch(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
	LINK_SWITCH_SAVE_BLOCK* block = (LINK_SWITCH_SAVE_BLOCK*) header; 
	
	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourLinkSwitch) return;

	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(request)
	COPYELEMENT_LOAD(system_state)
	COPYELEMENT_LOAD(state)
	COPYELEMENT_LOAD(ls_mode)
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(new_state)
	COPYELEMENT_LOAD(new_request)
	COPYELEMENT_LOAD(mode_store)
	COPYELEMENT_LOAD(triggered_last)

	//set the texture animation sequence
	if(ls_bhv->ls_tac)
	{
		ls_bhv->ls_tac->tac_sequence = block->txanim_sequence;
		ls_bhv->ls_tac->tac_txah_s = GetTxAnimHeaderFromShape(ls_bhv->ls_tac, (sbPtr->shapeIndex));
	}

	//load the track position , if the switch has one
	if(ls_bhv->ls_track)
	{
		SAVE_BLOCK_HEADER* track_header = GetNextBlockIfOfType(SaveBlock_Track);
		if(track_header)
		{
			LoadTrackPosition(track_header,ls_bhv->ls_track);
		}
	}

}

void SaveStrategy_LinkSwitch(STRATEGYBLOCK* sbPtr)
{
	LINK_SWITCH_SAVE_BLOCK *block;
	LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
	ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	COPYELEMENT_SAVE(request)
	COPYELEMENT_SAVE(system_state)
	COPYELEMENT_SAVE(state)
	COPYELEMENT_SAVE(ls_mode)
	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(new_state)
	COPYELEMENT_SAVE(new_request)
	COPYELEMENT_SAVE(mode_store)
	COPYELEMENT_SAVE(triggered_last)
	

	//get the animation sequence
	if(ls_bhv->ls_tac)
	{
		block->txanim_sequence = ls_bhv->ls_tac->tac_sequence;
	}
	else
	{
		block->txanim_sequence = 0;
	}

	//save the track position , if the switch has one
	if(ls_bhv->ls_track)
	{
		SaveTrackPosition(ls_bhv->ls_track);
	}
	
}
