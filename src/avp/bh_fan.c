#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "track.h"
#include "bh_fan.h"
#include "dynamics.h"
#include "weapons.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "plat_shp.h"

extern int NormalFrameTime;
extern DAMAGE_PROFILE fan_damage;

void* FanBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	FAN_BEHAV_BLOCK* f_bhv;
	FAN_TOOLS_TEMPLATE* f_tt;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	f_bhv=(FAN_BEHAV_BLOCK*)AllocateMem(sizeof(FAN_BEHAV_BLOCK));
	if(!f_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	f_bhv->bhvr_type=I_BehaviourFan;

	f_tt=(FAN_TOOLS_TEMPLATE*)bhdata;

	sbptr->shapeIndex = f_tt->shape_num;
	COPY_NAME(sbptr->SBname, f_tt->nameID);

	GLOBALASSERT(sbptr->DynPtr);
	
	sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = f_tt->position;
	sbptr->DynPtr->OrientEuler = f_tt->orientation;
	CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
	TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	
	
	f_bhv->track=f_tt->track;
	GLOBALASSERT(f_bhv->track);
	f_bhv->track->sbptr=sbptr;
	f_bhv->track->use_speed_mult=1;
	
	f_bhv->speed_up_mult=f_tt->speed_up_mult;
	f_bhv->slow_down_mult=f_tt->slow_down_mult;
	f_bhv->fan_wind_direction=f_tt->fan_wind_direction;
	f_bhv->fan_wind_strength=f_tt->fan_wind_strength;

	if(f_bhv->track->playing)
	{
		f_bhv->speed_mult=ONE_FIXED;
		f_bhv->track->speed_mult=ONE_FIXED;
 		f_bhv->state=fan_state_go;
	}
	else
	{
		f_bhv->speed_mult=0;
 		f_bhv->state=fan_state_stop;
	}
	//update wind speed
	f_bhv->wind_speed=MUL_FIXED(f_bhv->speed_mult,f_bhv->fan_wind_strength);
 	
 	return ((void*)f_bhv);
}


void FanBehaveFun(STRATEGYBLOCK* sbptr)
{
 	FAN_BEHAV_BLOCK *f_bhv;
 	
 	GLOBALASSERT(sbptr);
	f_bhv = (FAN_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((f_bhv->bhvr_type == I_BehaviourFan));
	GLOBALASSERT(f_bhv->track);

//	textprint("I am a fan\n");
	if(f_bhv->state==fan_state_go)
	{
		if(f_bhv->speed_mult==0)
		{
			//fan just starting
			Start_Track_Playing(f_bhv->track);
		}
		
		if(f_bhv->speed_mult<ONE_FIXED)
		{
			//fan currently accelerating
			f_bhv->speed_mult+=MUL_FIXED(NormalFrameTime,f_bhv->speed_up_mult);
			
			if(f_bhv->speed_mult>=ONE_FIXED)
			{
				f_bhv->speed_mult=ONE_FIXED;
			}
			f_bhv->track->speed_mult=f_bhv->speed_mult;
			//update wind speed
			f_bhv->wind_speed=MUL_FIXED(f_bhv->speed_mult,f_bhv->fan_wind_strength);

		}	
	}
	else
	{
		if(f_bhv->speed_mult>0)
		{
			//fan currently slowing down
			f_bhv->speed_mult-=MUL_FIXED(NormalFrameTime,f_bhv->slow_down_mult);

			if(f_bhv->speed_mult<0)
			{
				//fan has come to a stop
				f_bhv->speed_mult=0;
				Stop_Track_Playing(f_bhv->track);
			}
			f_bhv->track->speed_mult=f_bhv->speed_mult;
			//update wind speed
			f_bhv->wind_speed=MUL_FIXED(f_bhv->speed_mult,f_bhv->fan_wind_strength);
			
		}	
	}

	if(f_bhv->speed_mult)
	{
		Update_Track_Position(f_bhv->track);
	}

	//see if fan has hit anything
	if(sbptr->DynPtr->CollisionReportPtr)
	{
		//don't cause damage iunless fan is going reasonably fast
		if(f_bhv->speed_mult>ONE_FIXED/4)
		{
			COLLISIONREPORT* reportptr=sbptr->DynPtr->CollisionReportPtr;
			
			fan_damage.Cutting=MUL_FIXED(f_bhv->speed_mult,200); 
			//go through all the collision reports and damage any creatures found
			while(reportptr)
			{
				if(reportptr->ObstacleSBPtr)
				{
					STRATEGYBLOCK* hit_sbptr=reportptr->ObstacleSBPtr;
					if(hit_sbptr->I_SBtype==I_BehaviourAlien ||
					   hit_sbptr->I_SBtype==I_BehaviourQueenAlien ||
					   hit_sbptr->I_SBtype==I_BehaviourFaceHugger ||
					   hit_sbptr->I_SBtype==I_BehaviourPredator ||
					   hit_sbptr->I_SBtype==I_BehaviourXenoborg ||
					   hit_sbptr->I_SBtype==I_BehaviourMarine ||
					   hit_sbptr->I_SBtype==I_BehaviourSeal ||
					   hit_sbptr->I_SBtype==I_BehaviourPredatorAlien ||
					   hit_sbptr->I_SBtype==I_BehaviourAlien ||
					   hit_sbptr->I_SBtype==I_BehaviourMarinePlayer ||
					   hit_sbptr->I_SBtype==I_BehaviourPredatorPlayer || 
					   hit_sbptr->I_SBtype==I_BehaviourAlienPlayer) 
					{
						if(f_bhv->speed_mult==ONE_FIXED)
						{
							//ensure death if fan is going at full speed
							CauseDamageToObject(hit_sbptr,&fan_damage,100*ONE_FIXED,&f_bhv->fan_wind_direction);
						}
						else
						{
							CauseDamageToObject(hit_sbptr,&fan_damage,NormalFrameTime,&f_bhv->fan_wind_direction);
						}
					}
				}
				reportptr=reportptr->NextCollisionReportPtr;
			}
		}
	}

}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct fan_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	FAN_STATE state;
	int speed_mult;	//0 to one_fixed : current speed relative to full speed
	int wind_speed;//fixed point multiplier , taking the fan's current speed into account
}FAN_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV f_bhv

void LoadStrategy_Fan(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	FAN_BEHAV_BLOCK *f_bhv;
	FAN_SAVE_BLOCK* block = (FAN_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourFan) return;

	f_bhv = (FAN_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(state)
	COPYELEMENT_LOAD(speed_mult)
	COPYELEMENT_LOAD(wind_speed)

	//load the track position
	if(f_bhv->track)
	{
		SAVE_BLOCK_HEADER* track_header = GetNextBlockIfOfType(SaveBlock_Track);
		if(track_header)
		{
			LoadTrackPosition(track_header,f_bhv->track);
		}
	}
}


void SaveStrategy_Fan(STRATEGYBLOCK* sbPtr)
{
	FAN_SAVE_BLOCK* block; 
	FAN_BEHAV_BLOCK *f_bhv;
	
	f_bhv = (FAN_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(state)
	COPYELEMENT_SAVE(speed_mult)
	COPYELEMENT_SAVE(wind_speed)

	//save the track position
	if(f_bhv->track)
	{
		SaveTrackPosition(f_bhv->track);
	}

}
