#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_binsw.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pldghost.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "plat_shp.h"
#include "psnd.h"

extern int NormalFrameTime;
extern int RealFrameTime;
extern int ReturnPlayerSecurityClearance(STRATEGYBLOCK *sbPtr, unsigned int securityLevel);

/*********************** SWITCH INIT *****************************/

void* BinarySwitchBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
	BIN_SWITCH_TOOLS_TEMPLATE *bs_tt;
	int i;

 	GLOBALASSERT(sbptr);
	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)AllocateMem(sizeof(BINARY_SWITCH_BEHAV_BLOCK));
	if (!bs_bhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

 	bs_bhv->bhvr_type = I_BehaviourBinarySwitch;

	// from loaders
	// 1 rest_state - on or off
	// 2 mode
	// 3 timer switch - time for reset
	// 4 security clerance to operate
	// 5 copy the target name

	bs_tt = (BIN_SWITCH_TOOLS_TEMPLATE*)bhdata;

	sbptr->shapeIndex = bs_tt->shape_num;
	COPY_NAME(sbptr->SBname, bs_tt->nameID);

	bs_bhv->bs_mode = bs_tt->mode;
	bs_bhv->time_for_reset = bs_tt->time_for_reset;
	bs_bhv->security_clerance	= bs_tt->security_clearance;

	bs_bhv->trigger_volume_min=bs_tt->trigger_volume_min;	
	bs_bhv->trigger_volume_max=bs_tt->trigger_volume_max;	
	bs_bhv->switch_flags=bs_tt->switch_flags;	


	bs_bhv->num_targets = bs_tt->num_targets;
	if(bs_tt->num_targets)
	{
		bs_bhv->target_names = (SBNAMEBLOCK *)AllocateMem(sizeof(SBNAMEBLOCK) * bs_tt->num_targets);
		if (!bs_bhv->target_names) 
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}
		bs_bhv->request_messages = (int *)AllocateMem(sizeof(int) * bs_tt->num_targets);
		if (!bs_bhv->request_messages) 
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}

		bs_bhv->bs_targets = (STRATEGYBLOCK **)AllocateMem(sizeof(STRATEGYBLOCK *) * bs_tt->num_targets);
		if (!bs_bhv->bs_targets)
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}
	}
	else
	{
		bs_bhv->target_names=0;
		bs_bhv->request_messages=0;
		bs_bhv->bs_targets=0;
	}
	for (i=0; i<bs_tt->num_targets; i++)
	{
		COPY_NAME(bs_bhv->target_names[i].name, bs_tt->target_names[i].name);
		bs_bhv->request_messages[i]=bs_tt->request_messages[i];
		bs_bhv->bs_targets[i] = 0;
	}
	if(sbptr->DynPtr) //there may be no shape
	{
		sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = bs_tt->position;
		sbptr->DynPtr->OrientEuler = bs_tt->orientation;
		CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
		TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	
	}
	// set up the animation control
	if(sbptr->shapeIndex!=-1)
	{
		int item_num;
		TXACTRLBLK **pptxactrlblk;		
		int shape_num = bs_tt->shape_num;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
 
		SetupPolygonFlagAccessForShape(shptr);

		pptxactrlblk = &bs_bhv->bs_tac;

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
					pnew_txactrlblk->tac_sequence = bs_tt->starts_on;
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
		bs_bhv->bs_tac=0;
	}

	bs_bhv->bs_dtype = binswitch_no_display;



	if (bs_bhv->bs_tac)
	{
		bs_bhv->bs_dtype = binswitch_animate_me;
	}

	bs_bhv->bs_track=bs_tt->track;
	
	if (bs_bhv->bs_track)
	{
		bs_bhv->bs_track->sbptr=sbptr;
		
		if (bs_bhv->bs_dtype == binswitch_animate_me)
		{
			bs_bhv->bs_dtype = binswitch_animate_and_move_me;
		}
		else
		{
			bs_bhv->bs_dtype = binswitch_move_me;
		}
	}

// 	GLOBALASSERT(bs_bhv->bs_dtype != binswitch_no_display);

	// fill in the rest ourselves

	bs_bhv->request = 0;
	bs_bhv->state = bs_tt->starts_on;
	bs_bhv->timer = 0;
	bs_bhv->switch_off_message_same=bs_tt->switch_off_message_same;	
	bs_bhv->switch_off_message_none=bs_tt->switch_off_message_none;	
		
	bs_bhv->soundHandle = SOUND_NOACTIVEINDEX;
	bs_bhv->triggered_last = No;

	if(bs_bhv->bs_mode==I_bswitch_time_delay_autoexec)
	{
		//set request and then treat as normal time delay switch
		bs_bhv->bs_mode=I_bswitch_time_delay;
		bs_bhv->request=I_request_on;
	}

	if(bs_bhv->state)
	{
		bs_bhv->timer=bs_bhv->time_for_reset;	
		if(bs_bhv->bs_track)
		{
			//set the track to the end position
			bs_bhv->bs_track->current_section=(bs_bhv->bs_track->num_sections-1);
			bs_bhv->bs_track->timer=bs_bhv->bs_track->sections[bs_bhv->bs_track->current_section].time_for_section;
			bs_bhv->bs_track->playing=1;
			Update_Track_Position(bs_bhv->bs_track);

		}
	}
	
	bs_bhv->TimeUntilNetSynchAllowed=0;

	return((void*)bs_bhv);
}



void BinarySwitchBehaveFun(STRATEGYBLOCK* sbptr)
{
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
	DISPLAYBLOCK* dptr;
	int oldState;
 	GLOBALASSERT(sbptr);
	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));
  	dptr = sbptr->SBdptr;

	/*
	if(AvP.Network!=I_No_Network) return;
	*/

	/****** 
		What I need to do - check to see if we have
		a request - requests have different effects depending on 
		the mode - so we have to switch on the mode
	*****/


	if (bs_bhv->bs_dtype == binswitch_animate_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
	{
		if(dptr)
			dptr->ObTxAnimCtrlBlks = bs_bhv->bs_tac;
	}

	if (!ReturnPlayerSecurityClearance(0,bs_bhv->security_clerance) && bs_bhv->security_clerance)
	{
		bs_bhv->request = I_no_request;
		return;
		
	}

	if(AvP.Network != I_No_Network)
	{
		/*
		Every time a switch is updated there is a time delay of 5 seconds before the
		switch can next be changed by the host sending synch messages.
		This prevents the host machine from resetting a switch before it learns that
		it has been pressed
		*/
		if(bs_bhv->request == I_no_request)
		{
			bs_bhv->TimeUntilNetSynchAllowed-=RealFrameTime;
			if(bs_bhv->TimeUntilNetSynchAllowed<0)
			{
				bs_bhv->TimeUntilNetSynchAllowed=0;
			}
		}
		else
		{
			bs_bhv->TimeUntilNetSynchAllowed=5*ONE_FIXED;
		}
	}

	if(bs_bhv->switch_flags && SwitchFlag_UseTriggerVolume)
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
				sbPtr->DynPtr->Position.vx > bs_bhv->trigger_volume_min.vx &&	
				sbPtr->DynPtr->Position.vx < bs_bhv->trigger_volume_max.vx &&
				sbPtr->DynPtr->Position.vy > bs_bhv->trigger_volume_min.vy &&
				sbPtr->DynPtr->Position.vy < bs_bhv->trigger_volume_max.vy &&
				sbPtr->DynPtr->Position.vz > bs_bhv->trigger_volume_min.vz &&
				sbPtr->DynPtr->Position.vz < bs_bhv->trigger_volume_max.vz)
	    	{
	    		bs_bhv->request=I_request_on;
				break;
	    	}
    	} 
	}

	if (bs_bhv->request == I_request_on)
	{
		if (bs_bhv->triggered_last)
		{
			bs_bhv->request = I_no_request;
		}
		else
		{
			bs_bhv->triggered_last = Yes;
		}
	}
	else
	{
		bs_bhv->triggered_last = No;
	}


	switch(bs_bhv->bs_mode)
		{
			case I_bswitch_timer:
				{
					if(bs_bhv->request == I_request_on && !bs_bhv->state) 
						{
							
							bs_bhv->timer = bs_bhv->time_for_reset;

							if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
							{
								if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
								{
									if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
					 				{
										Sound_Play(SID_SWITCH1,"eh",&bs_bhv->soundHandle);
									}
								}
							}

							if (bs_bhv->bs_dtype == binswitch_move_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
							{
								// moving switch
								bs_bhv->new_state = 1;
								bs_bhv->new_request = 1;
								bs_bhv->bs_track->reverse=0;
								Start_Track_Playing(bs_bhv->bs_track);
								bs_bhv->mode_store = bs_bhv->bs_mode;
								bs_bhv->bs_mode = I_bswitch_moving;
							}
							else
							{
								int i;
								for (i=0; i<bs_bhv->num_targets; i++)
								{
									if (bs_bhv->bs_targets[i])
									{
										RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i], sbptr);
										
									}
								}
								bs_bhv->state = 1;
							}
							

							// swap sequence
							if(bs_bhv->bs_tac)
								{
									bs_bhv->bs_tac->tac_sequence = 1;
									bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
								}
							
							
						}															
					 else	if(bs_bhv->timer > 0)
						{
					 		bs_bhv->timer -= NormalFrameTime;
							if(bs_bhv->timer <= 0)
								{
									if(AvP.Network != I_No_Network)
									{
										bs_bhv->TimeUntilNetSynchAllowed=5*ONE_FIXED;
									}
									
									if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
									{
										if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
							 			{
											if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
											{
												Sound_Play(SID_SWITCH2,"eh",&bs_bhv->soundHandle);
											}
										}
									}
									bs_bhv->state = 0;

									if (bs_bhv->bs_dtype == binswitch_move_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
									{
										// moving switch
										bs_bhv->new_state = 0;
										bs_bhv->new_request = -1; //the 'off' message will be sent as soon as the timer finishes
										bs_bhv->bs_track->reverse=1;
										Start_Track_Playing(bs_bhv->bs_track);
										bs_bhv->mode_store = bs_bhv->bs_mode;
										bs_bhv->bs_mode = I_bswitch_moving;
									}
									
									if(!bs_bhv->switch_off_message_none)
									{
										//send the 'off' message
										int i;
										for (i=0; i<bs_bhv->num_targets; i++)
										{
											if (bs_bhv->bs_targets[i])
											{
												RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i]^(!bs_bhv->switch_off_message_same), sbptr);
											}
										}
										
									}

									if(bs_bhv->bs_tac)
										{
											bs_bhv->bs_tac->tac_sequence = 0;
											bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
										}

								}						
						}
					break;				
				}
			
			case I_bswitch_toggle:
				{
					if(bs_bhv->request == I_no_request)
						return;

					/* change the state and request the new state in
							the target */

						if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
						{
							if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
							{
								if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
								{
									Sound_Play(SID_SWITCH1,"eh",&bs_bhv->soundHandle);
								}
							}
						}
						if(bs_bhv->bs_dtype == binswitch_move_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
						{
							// moving switch
							bs_bhv->new_state = !bs_bhv->state;
							if(bs_bhv->new_state)
							{
								bs_bhv->new_request = 1;
							}
							else
							{
								//check to see what the switch off request is
								if(bs_bhv->switch_off_message_same)
									bs_bhv->new_request=1;
								else if(bs_bhv->switch_off_message_none)
									bs_bhv->new_request=-1;
								else
									bs_bhv->new_request=0;

							}
							bs_bhv->mode_store = bs_bhv->bs_mode;
							bs_bhv->bs_mode = I_bswitch_moving;
							bs_bhv->bs_track->reverse=bs_bhv->state;
							Start_Track_Playing(bs_bhv->bs_track);
						}
						else
						{
							bs_bhv->state = !bs_bhv->state;
							{
								int i;
								for (i=0; i<bs_bhv->num_targets; i++)
								{
									if (bs_bhv->bs_targets[i])
									{
										if(bs_bhv->state)
										{
											RequestState(bs_bhv->bs_targets[i], bs_bhv->request_messages[i], sbptr);
										}
										else
										{
											if(!bs_bhv->switch_off_message_none)
											{
												RequestState(bs_bhv->bs_targets[i], bs_bhv->request_messages[i]^(!bs_bhv->switch_off_message_same), sbptr);
											}
										}
									}
								}
							}
						}
						if(bs_bhv->bs_tac)
							{
								bs_bhv->bs_tac->tac_sequence = bs_bhv->state ;
								bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
							}


					break;				
				}
			case I_bswitch_wait:
				{
					if(bs_bhv->request == I_no_request)
						return;

					
					oldState = bs_bhv->state;

					if(bs_bhv->request == I_request_on)
						{
							if(bs_bhv->state)
								break;//switch cannot be activated again until it is reset
					
							if(bs_bhv->bs_dtype == binswitch_move_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
							{
							
								// moving switch
								bs_bhv->new_state = 1;
								bs_bhv->new_request = 1;
								bs_bhv->mode_store = bs_bhv->bs_mode;
								bs_bhv->bs_mode = I_bswitch_moving;
								bs_bhv->bs_track->reverse=0;
								Start_Track_Playing(bs_bhv->bs_track);
							}
							else
							{
								int i;
								bs_bhv->state = 1;
								for (i=0; i<bs_bhv->num_targets; i++)
								{
									if (bs_bhv->bs_targets[i])
									{
										RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i] , sbptr);

									}
								}
							}
						}
					else
						{
							if(!bs_bhv->state) break; //can only be deactivated if currently on
							if(bs_bhv->bs_dtype == binswitch_move_me || bs_bhv->bs_dtype == binswitch_animate_and_move_me)
							{
							
								// moving switch
								bs_bhv->new_state = 0;
								
								//check to see what the switch off request is
								if(bs_bhv->switch_off_message_same)
									bs_bhv->new_request=1;
								else if(bs_bhv->switch_off_message_none)
									bs_bhv->new_request=-1;
								else
									bs_bhv->new_request=0;
								
								bs_bhv->mode_store = bs_bhv->bs_mode;
								bs_bhv->bs_mode = I_bswitch_moving;
								bs_bhv->bs_track->reverse=1;
								Start_Track_Playing(bs_bhv->bs_track);
							}
							else
							{
								int i;
								
								bs_bhv->state = 0;
								if(!bs_bhv->switch_off_message_none)
								{
									for (i=0; i<bs_bhv->num_targets; i++)
									{
										if (bs_bhv->bs_targets[i])
										{
											RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i]^(!bs_bhv->switch_off_message_same), sbptr);

										}
									}
								}
							}
						}											
						
					if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
					{
						if (oldState == bs_bhv->state)
						{
							if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
						 	{
								if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
								{
						 			Sound_Play(SID_SWITCH2,"eh",&bs_bhv->soundHandle);
								}
						 	}
						}
						else
						{
							if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
							{
							   	if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
							   	{
									Sound_Play(SID_SWITCH1,"eh",&bs_bhv->soundHandle);
								}
							}
				 		}
					}
					if(bs_bhv->bs_tac)
						{
							bs_bhv->bs_tac->tac_sequence = bs_bhv->state ? 1 : 0;
							bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
						}

					break;
				}
			case I_bswitch_moving:
				{
					Update_Track_Position(bs_bhv->bs_track);
					if(!bs_bhv->bs_track->playing)
					{
						bs_bhv->bs_mode = bs_bhv->mode_store;
						bs_bhv->state = bs_bhv->new_state;
						if(bs_bhv->bs_tac)
						{
							bs_bhv->bs_tac->tac_sequence = bs_bhv->state ? 1 : 0;
							bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
						}
						if (bs_bhv->new_request != -1)
						{
							int i;
							for (i=0; i<bs_bhv->num_targets; i++)
							{
								if (bs_bhv->bs_targets[i])
								{
									if(bs_bhv->new_request)
										RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i] , sbptr);
									else
										RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i]^1, sbptr);
								}
							}
						}
					}


				}
				break;
			case I_bswitch_time_delay :
			{
				if(bs_bhv->timer>0)
				{
					bs_bhv->timer-=NormalFrameTime;
					if(bs_bhv->timer<=0 || bs_bhv->request == I_request_off)
					{
						//time to send the request
						int i;
						bs_bhv->timer=0;
						
						//only send message if we got here through the time running out
						if(bs_bhv->request != I_request_off)
						{
							for (i=0; i<bs_bhv->num_targets; i++)
							{
								if (bs_bhv->bs_targets[i])
								{
									RequestState(bs_bhv->bs_targets[i],bs_bhv->request_messages[i] , sbptr);
								}
							}
						}
						bs_bhv->state = 0;
					
						 // swap sequence
						if(bs_bhv->bs_tac)
						{
							bs_bhv->bs_tac->tac_sequence = 0;
							bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
						}
						
						if(AvP.Network != I_No_Network)
						{
							bs_bhv->TimeUntilNetSynchAllowed=5*ONE_FIXED;
						}

					}	
				}
				else
				{
					if(bs_bhv->request == I_request_on && bs_bhv->state == 0) 
					{
						bs_bhv->timer = bs_bhv->time_for_reset;
						if(sbptr->shapeIndex!=-1)//don't play a sound if there is no shape
						{
							if (bs_bhv->soundHandle == SOUND_NOACTIVEINDEX) 
					 		{
								if(!bs_bhv->bs_track || !bs_bhv->bs_track->sound)
								{
									Sound_Play(SID_SWITCH1,"eh",&bs_bhv->soundHandle);
								}
							}
						}
						bs_bhv->state = 1;
					
						 // swap sequence
						 if(bs_bhv->bs_tac)
						 	{
						 		bs_bhv->bs_tac->tac_sequence = 1;
						 		bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbptr->shapeIndex));
						 	}
					
					}
				}
			}
			break;
			default:
				GLOBALASSERT(2<1);
		}

	bs_bhv->request = I_no_request;
}


#define BINARYSWITCHSYNCH_ON	 0
#define BINARYSWITCHSYNCH_OFF	 1
#define BINARYSWITCHSYNCH_IGNORE 2

int BinarySwitchGetSynchData(STRATEGYBLOCK* sbPtr)
{
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
 	GLOBALASSERT(sbPtr);
	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));

	//don't try to synch moving switches
	if(bs_bhv->bs_mode==I_bswitch_moving)
	{
		return BINARYSWITCHSYNCH_IGNORE;
	}

	if(bs_bhv->state)
		return BINARYSWITCHSYNCH_ON;
	else	
		return BINARYSWITCHSYNCH_OFF;
}


void BinarySwitchSetSynchData(STRATEGYBLOCK* sbPtr,int status)
{
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
 	GLOBALASSERT(sbPtr);
	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));

	if(bs_bhv->TimeUntilNetSynchAllowed>0)
	{
		//ignore this attempt to synch the switch
		return;
	}

	//don't try to synch moving switches
	if(bs_bhv->bs_mode==I_bswitch_moving)
	{
		return;
	}
	
	
	switch(status)
	{
		case BINARYSWITCHSYNCH_ON :
			if(!bs_bhv->state)
			{
				//this switch should be on
				RequestState(sbPtr,1,0);
			}
			break;

		case BINARYSWITCHSYNCH_OFF :
			if(bs_bhv->state)
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
typedef struct binary_switch_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BINARY_SWITCH_REQUEST_STATE request;
	BOOL state;
	BSWITCH_MODE bs_mode;
	int timer;
	
	BSWITCH_MODE mode_store;
	
	BOOL new_state;
	int new_request;
	
	BOOL triggered_last;

	int txanim_sequence;

}BINARY_SWITCH_SAVE_BLOCK;


//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV bs_bhv

void LoadStrategy_BinarySwitch(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
	BINARY_SWITCH_SAVE_BLOCK* block = (BINARY_SWITCH_SAVE_BLOCK*) header; 
	
	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourBinarySwitch) return;

	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(request)
	COPYELEMENT_LOAD(state)
	COPYELEMENT_LOAD(bs_mode)
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(mode_store)
	COPYELEMENT_LOAD(new_state)
	COPYELEMENT_LOAD(new_request)
	COPYELEMENT_LOAD(triggered_last)

	//set the texture animation sequence
	if(bs_bhv->bs_tac)
	{
		bs_bhv->bs_tac->tac_sequence = block->txanim_sequence;
		bs_bhv->bs_tac->tac_txah_s = GetTxAnimHeaderFromShape(bs_bhv->bs_tac, (sbPtr->shapeIndex));
	}

	//load the track position , if the switch has one
	if(bs_bhv->bs_track)
	{
		SAVE_BLOCK_HEADER* track_header = GetNextBlockIfOfType(SaveBlock_Track);
		if(track_header)
		{
			LoadTrackPosition(track_header,bs_bhv->bs_track);
		}
	}

}

void SaveStrategy_BinarySwitch(STRATEGYBLOCK* sbPtr)
{
	BINARY_SWITCH_SAVE_BLOCK *block;
	BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
	bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbPtr->SBdataptr;
	

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(request)
	COPYELEMENT_SAVE(state)
	COPYELEMENT_SAVE(bs_mode)
	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(mode_store)
	COPYELEMENT_SAVE(new_state)
	COPYELEMENT_SAVE(new_request)
	COPYELEMENT_SAVE(triggered_last)

	//get the animation sequence
	if(bs_bhv->bs_tac)
	{
		block->txanim_sequence = bs_bhv->bs_tac->tac_sequence;
	}
	else
	{
		block->txanim_sequence = 0;
	}

	//save the track position , if the switch has one
	if(bs_bhv->bs_track)
	{
		SaveTrackPosition(bs_bhv->bs_track);
	}
	
}
