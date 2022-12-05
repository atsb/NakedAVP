#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "weapons.h"
#include "comp_shp.h"
#include "inventry.h"
#include "triggers.h"
#include "huddefs.h"

#include "dynblock.h"
#include "dynamics.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "bh_swdor.h"
#include "load_shp.h"
#include "lighting.h"
#include "bh_lnksw.h"
#include "bh_binsw.h"
#include "bh_lift.h"

#include "psnd.h"

/* for win95 net game support */
#include "pldghost.h"

extern int NormalFrameTime;
// stuff for environment changing 

extern void IntegrateNewEnvironment();
extern int NumActiveBlocks;

// Globals we are going to export

int RequestEnvChangeViaLift	= 0;
int RequestEnvChangeViaAirlock	= 0;

LIFT_CONTROL_BLOCK	EC_Lift_Ctrl;
MODULE Old_Pos_Module;



static void TeleportFloorSwitches(MODULE* dest, MODULE* src, LIFT_CONTROL_BLOCK* liftCtrl);



/*********************** CLOSED TELEPORT LIFTS INIT ***************/

void * LiftBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{

	LIFT_BEHAV_BLOCK *lift_bhv;
 	LIFT_STATION* lift_stn;
	LIFT_TOOLS_TEMPLATE* lift_tt;
	MODULE * my_mod;

 	GLOBALASSERT(sbptr);
	lift_bhv = (LIFT_BEHAV_BLOCK*)AllocateMem(sizeof(LIFT_BEHAV_BLOCK));
	if(!lift_bhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

  lift_bhv->bhvr_type = I_BehaviourLift;

	lift_stn = &lift_bhv->lift_station;
	lift_tt = (LIFT_TOOLS_TEMPLATE*)bhdata;

	// Copy the name over
	COPY_NAME (sbptr->SBname, lift_tt->nameID);


	// Setup module ref if not external
	if (lift_tt->environment == (int)AvP.CurrentEnv)
	{
		{
			MREF mref=lift_tt->my_module;
			ConvertModuleNameToPointer (&mref, MainSceneArray[0]->sm_marray);
		
			my_mod = mref.mref_ptr;
		}
		GLOBALASSERT (my_mod);

		my_mod->m_sbptr = sbptr;
		sbptr->SBmoptr = my_mod;
		sbptr->SBmomptr = my_mod->m_mapptr;
		sbptr->SBflags.no_displayblock = 1;
	}

	// loaded data - first the station

	COPY_NAME(lift_stn->lift_call_switch_name, lift_tt->call_switch_name);
	COPY_NAME(lift_stn->lift_door_name, lift_tt->lift_door_name);
	COPY_NAME(lift_stn->lift_floor_switch_name, lift_tt->lift_floor_switch_name);
	COPY_NAME(lift_stn->my_sb_name, lift_tt->my_module_name);
	
	lift_stn->env	= lift_tt->environment;
	lift_stn->num_floor =	lift_tt->num_floor;
	lift_stn->orient = lift_tt->orient;

	// fill in the rest of the stn data;
	
	lift_stn->lift_call_switch = NULL;	
	lift_stn->lift_door = NULL;	
	lift_stn->lift_floor_switch = NULL;	
	lift_stn->lift_module = sbptr->SBmoptr;
	if(lift_tt->lift_flags & LiftFlag_Here)
		lift_stn->starting_station = 1;
	else
		lift_stn->starting_station = 0;

	lift_stn->called = 0;

	// fill in the behaviour block
		
	lift_bhv->control_sb = NULL;
	lift_bhv->controller = lift_tt->controller;
	COPY_NAME(lift_bhv->control_sb_name, lift_tt->control_sb_name);

	if(lift_bhv->controller)
	{
		LIFT_CONTROL_BLOCK* lcont = AllocateMem(sizeof(LIFT_CONTROL_BLOCK));
		if(!lcont) 
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}

		lift_bhv->lift_control = lcont;

		// fill in the number of floors
		
		lcont->num_stations	= lift_tt->num_stations;
  	lcont->lift_stations = (LIFT_STATION**)AllocateMem(sizeof(LIFT_STATION*)*lcont->num_stations);											

		if(!lcont->lift_stations) 
		{
			memoryInitialisationFailure = 1;
			return ((void *)NULL);
		}

		//and init that array
		{
			int i=0;
			while(i < lcont->num_stations)
			{
				*(lcont->lift_stations + i) = NULL;
				i++;
			}
		}
		// fill in the rest of the data

		*(lcont->lift_stations + lift_stn->num_floor) = lift_stn;
		lcont->dest_station = -1;
		lcont->delay_at_floor = 0;
		lcont->delay_between_floors = 0;
		lcont->motion = I_going_down;
		lcont->state = I_ls_waiting;
		lcont->curr_station = -1;
		lcont->prev_station = -1;
		lcont->SoundHandle = SOUND_NOACTIVEINDEX;

		
		if(lift_tt->lift_flags & LiftFlag_NoTel)
			lcont->floor_switches_fixed=1;
		else
			lcont->floor_switches_fixed=0;

	}


	return((void*)lift_bhv);
}


// this function reposts anything that would prevent 
// The palyer most be in the module and nothing
// else can be


BOOL BadGuyInModuleOrNoPlayer()
{
	return(0);
}	

/*************************** LIFT CONTROL *****************************/

void LiftBehaveFun(STRATEGYBLOCK* sbptr)
{
	LIFT_BEHAV_BLOCK *lift_bhv;	
	LIFT_CONTROL_BLOCK *lift_ctrl;
	LIFT_STATION *curr_stn;
	I_AVP_ENVIRONMENTS dest_env;

 	GLOBALASSERT(sbptr);
	lift_bhv = (LIFT_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((lift_bhv->bhvr_type == I_BehaviourLift));

	curr_stn = &lift_bhv->lift_station;
	GLOBALASSERT(curr_stn);

	lift_ctrl = lift_bhv->lift_control;
	

	/* ALL the lift modules have a Strategy block that contains inforamtion
		 about that particular location. Only one points to the LIFT CONTROL
		 BLOCK
	*/

	/* the lifts will work between envs as well as on the same env 
		 so the control of the doors in the other envs sin't actually set,
		 but a record of the postion is maintained
	*/


		
	if(lift_bhv->controller)
   	{

   		// HACK	- RWH - if this is true the data is broken and this fix won't always work

   		if(lift_ctrl->curr_station == -1)
   			lift_ctrl->curr_station = 0;

   		switch(lift_ctrl->state)
		{
			case I_ls_waiting:
			 {
			 	/*** find dest and set movement direction this is always the
			 	same even if the lift is on a different floor ***/
			 	int lower_station = -1;
			 	int upper_station = -1;
			 	int i;
			 	LIFT_STATION *lift_stn;

			 	/**** find the nearest floors selected ****/
				
				// search up and down
			 	for(i = lift_ctrl->curr_station; (i >= 0) && (lower_station == -1); i --)
		 		{
		 			lift_stn = lift_ctrl->lift_stations[i];
		 
		 			if(lift_stn->called)
		 					upper_station = i;    // higher floors have lower nums
		 		}
			 	for(i = lift_ctrl->curr_station; (i < lift_ctrl->num_stations) && (upper_station == -1); i ++)
		 		{
		 			lift_stn = lift_ctrl->lift_stations[i];

		 			if(lift_stn->called)
		 				lower_station = i;      // lower floors have higher nums
		 		}

			 	/****** set the	destination and the motion direction ****/

			 	if(lift_ctrl->motion == I_going_down)// higher nums
			 	 {
			 	 	if(lower_station != -1)
					{
						lift_ctrl->dest_station = lower_station;
					}
			 	 	else
		 	 		{
		 	 			lift_ctrl->dest_station = upper_station;
		 	 			lift_ctrl->motion =I_going_up;
		 	 		}
			 	 }															
				else if(lift_ctrl->motion == I_going_up) // lower nums
				{
					if(upper_station != -1)
					{
						lift_ctrl->dest_station = upper_station;
					}
					else
					{
						lift_ctrl->dest_station = lower_station;
						lift_ctrl->motion =I_going_down;
					}
				}															

			 	// now we have a station to go to, we start the lift in motion
			 	// unless we have just pressed the current floor switch again
			 	
			 	if(lift_ctrl->dest_station == lift_ctrl->curr_station)
		 		{
		 			// we have just called the lift to the same place
		 			lift_ctrl->dest_station = -1;
		 			lift_stn = lift_ctrl->lift_stations[lift_ctrl->curr_station];
		 			lift_stn->called = 0;
		 		}
			 	else if(lift_ctrl->dest_station != -1)
		 		{
		 			// if there is a dest, set the lift to go
		 			// close current stations 
		 			lift_stn = lift_ctrl->lift_stations[lift_ctrl->curr_station];

		 			lift_ctrl->prev_station = lift_ctrl->curr_station;
		 			if(lift_stn->lift_door)
 					{
 						// no door - must be on another env
 						RequestState(lift_stn->lift_door, 0, 0);
 					}
		 			lift_ctrl->state = I_ls_closing_door;
		 		}
			 		
			 	break;
			 }
			case I_ls_closing_door:
				{
					// lift station for the current lift position has to 
					// have a closed door

					BOOL door_state;
					LIFT_STATION *pos_stn = lift_ctrl->lift_stations[lift_ctrl->curr_station];
					MODULE* mptr;
					
					mptr = pos_stn->lift_module;
					
					// deal with the fact that the lift is in a different env

					// same env - normal code
					// called from us to another env -  normal code
					// called from another env to us - jump the close door

					if(pos_stn->env != AvP.CurrentEnv)
						{
	 						lift_ctrl->state = I_ls_moving;
							lift_ctrl->delay_between_floors = LIFT_MOVE_DELAY;
							break;
						}							
					
					// otherwise run normal code
					// close the door before we change state to moving						 
			
					door_state = GetState(pos_stn->lift_door);
					
					if(!door_state)
						{
							// turn off the current lift position station call switches
							// run only when pos_stn is in our own env POSSIBLE BUG -
							// lift wont be called to our floor if there is an Alien in it

							if(BadGuyInModuleOrNoPlayer())
							{
								lift_ctrl->state = I_ls_opening_door;
								lift_ctrl->dest_station = -1;
								lift_ctrl->delay_between_floors = -1;
								RequestState(pos_stn->lift_door, 1, 0);
							}
							else
							{
							
		 						lift_ctrl->state = I_ls_moving;
								lift_ctrl->delay_between_floors = LIFT_MOVE_DELAY;

								RequestState(pos_stn->lift_call_switch, 0, 0);
								if(pos_stn->lift_floor_switch)
									RequestState(pos_stn->lift_floor_switch, 0, 0);

								//what if there is a bad guy in the lift - if there is open the door
								//and reset all the switches and stations
								
								if (lift_ctrl->SoundHandle==SOUND_NOACTIVEINDEX)
								{
									GLOBALASSERT(mptr);
									Sound_Play(SID_LIFT_START,"dh",&mptr->m_world);
									Sound_Play(SID_LIFT_LOOP,"delh",&mptr->m_world, &lift_ctrl->SoundHandle);
								}
							}	
							
						}
						break;
				 }

			case I_ls_moving:
				{
					int curr_station_num = lift_ctrl->curr_station;
					LIFT_STATION *pos_stn = lift_ctrl->lift_stations[lift_ctrl->curr_station];
					MODULE* mptr;
					mptr = pos_stn->lift_module;

					

					dest_env = lift_ctrl->lift_stations[lift_ctrl->dest_station]->env;

						lift_ctrl->delay_between_floors -= NormalFrameTime;
					
					// TRAP CHANGE OF ENVIRONMENT
					// we need to trap it here so we don't have the delay
					// between floors. However. The trap should make sure it is the nex array pos

					if(AvP.CurrentEnv != dest_env)
						{
							// have to load an env - can we do it next - is the
							// new env the next floor in the array??

							if(lift_ctrl->motion == I_going_up)
								{
									curr_station_num --;												
								}
							else
								{
									curr_station_num ++;												
								}
							lift_ctrl->curr_station=curr_station_num;
							if(curr_station_num == lift_ctrl->dest_station)
								{
									// OKAY chnage Environment VIA LIFT
									// we need a copy of our current module pos
									// (for the teleport), a copy of the
									// SBs in our module and a copy of the lift
									// control block

									LIFT_STATION *lift_stn_old;
									EC_Lift_Ctrl = *lift_ctrl;

									lift_stn_old = EC_Lift_Ctrl.lift_stations[EC_Lift_Ctrl.prev_station];
									Old_Pos_Module = *lift_stn_old->lift_module;				
				
									InitPreservedSBs();
									PreserveStBlocksInModule(&Old_Pos_Module);
													
									EC_Lift_Ctrl.curr_station = curr_station_num;
									RequestEnvChangeViaLift = 1;

									// we now do the rest of the SBs. before we
									// can chack the env - note that NOTHING can move
									// into the lift now.

								}

						}

					// else do normal code

					else if((lift_ctrl->delay_between_floors < 0))
						{
							if(lift_ctrl->motion == I_going_up)
								{
									lift_ctrl->curr_station --;												
								}
							else
								{
									lift_ctrl->curr_station ++;												
								}
							

								
							LOCALASSERT(lift_ctrl->curr_station >= 0);
						
						
											
							if(lift_ctrl->curr_station == lift_ctrl->dest_station)
							{
								// at destination
									LIFT_STATION *lift_stn_new = lift_ctrl->lift_stations[lift_ctrl->curr_station];
								LIFT_STATION *lift_stn_old = lift_ctrl->lift_stations[lift_ctrl->prev_station];
								MODULE *new_pos, *old_pos;
							
								new_pos = lift_stn_new->lift_module;
								old_pos = lift_stn_old->lift_module;
			
								lift_ctrl->dest_station = -1;
								lift_ctrl->state = I_ls_opening_door;					
								
								/* roxby: i have taken this out - patrick */
								/* GLOBALASSERT(mptr); */
								Sound_Play(SID_LIFT_END,"h");
								Sound_Stop(lift_ctrl->SoundHandle);
															
								//  door open

								RequestState(lift_stn_new->lift_door, 1, 0);

								lift_stn_new->called = 0;

								if(old_pos)
								{	
									// if we don't have an old pos, we must
									// be coming from another env
									TeleportContents(new_pos, old_pos,lift_ctrl->floor_switches_fixed);
								}
							}

							else 
							{
							 	// interrupt monment to stop at new floor
							 	//this should never happen in a seperate env
							 	LIFT_STATION *lift_stn_new = lift_ctrl->lift_stations[lift_ctrl->curr_station];
							 	LIFT_STATION *lift_stn_old = lift_ctrl->lift_stations[lift_ctrl->prev_station];
							 	MODULE* old_pos, *new_pos;
							 	// to trap button presses when moving between floors -
							 	// not really ness and it complecates things
							 	old_pos = lift_stn_old->lift_module;
							 	new_pos = lift_stn_new->lift_module;
#if 0						 				
							
							 	if(lift_stn_new->called)	
						 		{
						 			lift_ctrl->state = I_ls_opening_door;	
						 			RequestState(lift_stn_new->lift_call_switch, 0, 0);
						 			if(lift_stn_new->lift_floor_switch)
						 				RequestState(lift_stn_new->lift_floor_switch, 0, 0);
						 			RequestState(lift_stn_new->lift_door, 1, 0);

						 			if(old_pos)
						 				{	
						 					// if we don't have an old pos, we must
						 					// be coming from another env
						 					TeleportContents(new_pos, old_pos,lift_ctrl->floor_switches_fixed);
						 				}

						 			lift_stn_new->called = 0;

						 		}
							 	else
#endif
						 		{												
						 			// futher to go - move along now
						 			lift_ctrl->delay_between_floors += LIFT_MOVE_DELAY;
						 		}
							 }
						}
						
					break;
				}								  																		
			case I_ls_opening_door:
				{
					LIFT_STATION *lift_stn_new = lift_ctrl->lift_stations[lift_ctrl->curr_station];
					BOOL door_state = GetState(lift_stn_new->lift_door);
					
					if(door_state)
						{
							lift_ctrl->state = I_ls_delay_at_floor;
							lift_ctrl->delay_at_floor = LIFT_FLOOR_DELAY;
						}
					break;
				}
			case I_ls_delay_at_floor:
				{
					lift_ctrl->delay_at_floor -= NormalFrameTime;

					if(lift_ctrl->delay_at_floor < 0)
						{
						 	LIFT_STATION *lift_stn;
			
							// turn on the floor lights
							lift_stn = lift_ctrl->lift_stations[lift_ctrl->curr_station];
							
							RequestState(lift_stn->lift_call_switch, 1, 0);
							if(lift_stn->lift_floor_switch_name[0] ||	lift_stn->lift_floor_switch_name[4])
								RequestState(lift_stn->lift_floor_switch, 1, 0);
							
							
							lift_ctrl->state = I_ls_waiting;
							lift_ctrl->dest_station = -1;
								
						}
					break;					
				}

			default:

				GLOBALASSERT(2<1);
		}
   	}							

}


void TeleportContents(MODULE* new_pos, MODULE* old_pos,BOOL floor_switches_fixed)
{
	//	this used within level - find objects in module
	// all will have sbs

	int i;
	int max_x, min_x, max_y, min_y, max_z, min_z;	
	int dest_max_x, dest_min_x, dest_max_y, dest_min_y, dest_max_z, dest_min_z;	
	VECTORCH mod_offset;

	mod_offset.vx = new_pos->m_world.vx - old_pos->m_world.vx;
	mod_offset.vy = new_pos->m_world.vy - old_pos->m_world.vy;
	mod_offset.vz = new_pos->m_world.vz - old_pos->m_world.vz;

	max_x = old_pos->m_maxx + old_pos->m_world.vx;
	min_x = old_pos->m_minx + old_pos->m_world.vx;
	max_y = old_pos->m_maxy + old_pos->m_world.vy;
	min_y = old_pos->m_miny + old_pos->m_world.vy;
	max_z = old_pos->m_maxz + old_pos->m_world.vz;
	min_z = old_pos->m_minz + old_pos->m_world.vz;

	dest_max_x = new_pos->m_maxx + new_pos->m_world.vx -200;
	dest_min_x = new_pos->m_minx + new_pos->m_world.vx +200;
	dest_max_y = new_pos->m_maxy + new_pos->m_world.vy -200;
	dest_min_y = new_pos->m_miny + new_pos->m_world.vy +200;
	dest_max_z = new_pos->m_maxz + new_pos->m_world.vz -200;
	dest_min_z = new_pos->m_minz + new_pos->m_world.vz +200;

	for(i = 0; i < NumActiveStBlocks; i++)
		{
			VECTORCH obj_world;
			STRATEGYBLOCK	*sbptr;
			DYNAMICSBLOCK	*dynptr;			

			sbptr = ActiveStBlockList[i];

			if(!(dynptr = sbptr->DynPtr))
				continue;
			
			if(floor_switches_fixed)
			{
			 	if(sbptr->I_SBtype==I_BehaviourBinarySwitch || sbptr->I_SBtype==I_BehaviourLinkSwitch)
					continue; 
				else if(sbptr->I_SBtype==I_BehaviourInanimateObject && ((INANIMATEOBJECT_STATUSBLOCK*)sbptr->SBdataptr)->typeId==0)	
					continue;
			}
			obj_world = dynptr->Position;

			if(obj_world.vx < max_x)
				if(obj_world.vx > min_x)
					if(obj_world.vz < max_z)
						if(obj_world.vz > min_z)
							if(obj_world.vy < max_y)
								if(obj_world.vy > min_y)
									{
										
										dynptr->Position.vx += mod_offset.vx; 
										dynptr->Position.vy += mod_offset.vy; 
										dynptr->Position.vz += mod_offset.vz; 

										dynptr->PrevPosition.vx += mod_offset.vx; 
										dynptr->PrevPosition.vy += mod_offset.vy; 
										dynptr->PrevPosition.vz += mod_offset.vz; 

										//make sure new location is inside destination module
										if(!dynptr->IsStatic)
										{
											if(dynptr->Position.vx<dest_min_x) dynptr->Position.vx=dest_min_x;
											if(dynptr->Position.vy<dest_min_y) dynptr->Position.vy=dest_min_y;
											if(dynptr->Position.vz<dest_min_z) dynptr->Position.vz=dest_min_z;
											if(dynptr->Position.vx>dest_max_x) dynptr->Position.vx=dest_max_x;
											if(dynptr->Position.vy>dest_max_y) dynptr->Position.vy=dest_max_y;
											if(dynptr->Position.vz>dest_max_z) dynptr->Position.vz=dest_max_z;

											if(dynptr->PrevPosition.vx<dest_min_x) dynptr->PrevPosition.vx=dest_min_x;
											if(dynptr->PrevPosition.vy<dest_min_y) dynptr->PrevPosition.vy=dest_min_y;
											if(dynptr->PrevPosition.vz<dest_min_z) dynptr->PrevPosition.vz=dest_min_z;
											if(dynptr->PrevPosition.vx>dest_max_x) dynptr->PrevPosition.vx=dest_max_x;
											if(dynptr->PrevPosition.vy>dest_max_y) dynptr->PrevPosition.vy=dest_max_y;
											if(dynptr->PrevPosition.vz>dest_max_z) dynptr->PrevPosition.vz=dest_max_z;
										}
											
										if(sbptr->maintainVisibility)
											{
												sbptr->containingModule = new_pos;
											}
									}
		}
}


void CleanUpLiftControl()
{
	// okay - we have selected a new env AND it is the
	// next env in the list - lets go with the load
	// we need a copu list of strat blocks that WE DO NOT
	// DEALLOCATE the behaviour / dyn blocks for. We then
	// reinitialise AFTER these have been inserted into the


	// ActiveStBlockList 
	LIFT_STATION *lift_stn = EC_Lift_Ctrl.lift_stations[EC_Lift_Ctrl.dest_station];

	char dest_mod_name[8];
	MODULE *new_pos_module;
	int orient_diff;

	STRATEGYBLOCK* new_loc_sbptr;

	COPY_NAME(dest_mod_name, lift_stn->my_sb_name);

	orient_diff =  EC_Lift_Ctrl.lift_stations[EC_Lift_Ctrl.dest_station]->orient
					- EC_Lift_Ctrl.lift_stations[EC_Lift_Ctrl.prev_station]->orient;


	// DESTROYS ALL OUR OLD DATA - ONLY OUR
	// COPIES REMAIN

	ChangeEnvironmentToEnv(lift_stn->env);

	IntegrateNewEnvironment();

	// find where we have teleported to

	new_loc_sbptr = FindSBWithName(dest_mod_name);
	GLOBALASSERT(new_loc_sbptr);
	new_pos_module = new_loc_sbptr->SBmoptr;

	// teleport the bastrads
	TeleportPreservedSBsToNewEnvModule(new_pos_module, &Old_Pos_Module,orient_diff);
	AddPreservedSBsToActiveList();

	// find the new control block anc copy our preserved one in
	// need to recalc lift_station_new

  	{
  		LIFT_BEHAV_BLOCK *lift_bhv;	
  		LIFT_CONTROL_BLOCK *lift_ctrl;
  		LIFT_STATION *lift_stn_new;

  		lift_bhv = (LIFT_BEHAV_BLOCK*)new_loc_sbptr->SBdataptr;
  		GLOBALASSERT((lift_bhv->bhvr_type == I_BehaviourLift));
  
  		lift_ctrl = lift_bhv->lift_control;

  		GLOBALASSERT(lift_ctrl->num_stations == EC_Lift_Ctrl.num_stations);

  		// copy over the vital bits

  		lift_ctrl->curr_station = EC_Lift_Ctrl.curr_station;
  		lift_ctrl->prev_station = EC_Lift_Ctrl.prev_station;
  		lift_ctrl->motion = EC_Lift_Ctrl.motion;
  
  		// and get into the right state

  		lift_ctrl->state = I_ls_opening_door;					
  		lift_ctrl->dest_station = -1;

  		// and get the periphs into their right states
		// get the new lift station
  		lift_stn_new = lift_ctrl->lift_stations[lift_ctrl->curr_station];

		//set all the other station switches to off to overdide
		// the starting_station init in AssignAllSBNames
		
		{
			int i;
			LIFT_STATION *liftStnPtr;
			
			for(i=0; i < lift_ctrl->num_stations; i++)
			{
				liftStnPtr = lift_ctrl->lift_stations[i];
				if(liftStnPtr->env == AvP.CurrentEnv)
				{
					RequestState(liftStnPtr->lift_call_switch, 0, 0);
					RequestState(liftStnPtr->lift_floor_switch, 0, 0);
					RequestState(liftStnPtr->lift_door, 0, 0);
				}
			}
		}

		// set our current stations periphs into the correct
		// state


		RequestState(lift_stn_new->lift_call_switch, 1, 0);
		RequestState(lift_stn_new->lift_floor_switch, 1, 0);
   		RequestState(lift_stn_new->lift_door, 1, 0);

		TeleportFloorSwitches
		(
			lift_stn_new->lift_module,
			lift_stn_new->lift_floor_switch->containingModule,
			lift_ctrl			
		);
		
  		InitPreservedSBs();

  		UpdateWeaponShape(); // so we get the correct shape in the MSL
  	}
}





static void TeleportFloorSwitches
(
	MODULE* dest,
	MODULE* src,
	LIFT_CONTROL_BLOCK * liftCtrl
)
{
	if(dest != src)
	{
		TeleportContents(dest, src, 0);
	}
}	
