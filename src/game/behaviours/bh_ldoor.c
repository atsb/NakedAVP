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

#include "dynblock.h"
#include "dynamics.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "pvisible.h"
#include "bh_pred.h"
#include "bh_swdor.h"
#include "bh_ldoor.h"
#include "bh_plift.h"
#include "load_shp.h"
#include "lighting.h"
#include "bh_lnksw.h"
#include "bh_binsw.h"
#include "bh_lift.h"

#include "psnd.h"
#include "savegame.h"

/* for win95 net game support */
#include "pldghost.h"


extern int NormalFrameTime;


void* LiftDoorBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	LIFT_DOOR_BEHAV_BLOCK *doorbhv;
	LIFT_DOOR_TOOLS_TEMPLATE *doortt;
	MORPHCTRL* morphctrl;	
	MORPHHEADER* morphheader;
	MORPHFRAME* morphframe;
	MODULE * my_mod;

	doorbhv = (LIFT_DOOR_BEHAV_BLOCK*)AllocateMem(sizeof(LIFT_DOOR_BEHAV_BLOCK));
	if (!doorbhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	doorbhv->bhvr_type = I_BehaviourLiftDoor;

	// from loaders
	
	doortt = (LIFT_DOOR_TOOLS_TEMPLATE*)bhdata;	

	// Set up a new Morph Control
	morphctrl = (MORPHCTRL*)AllocateMem(sizeof(MORPHCTRL));
	if (!morphctrl)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphheader = (MORPHHEADER*)AllocateMem(sizeof(MORPHHEADER));
	if (!morphheader)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphframe = (MORPHFRAME*)AllocateMem(sizeof(MORPHFRAME));
	if (!morphframe)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	morphframe->mf_shape1 = doortt->shape_open;
	morphframe->mf_shape2 = doortt->shape_closed;

	morphheader->mph_numframes = 1;
	morphheader->mph_maxframes = ONE_FIXED;
	morphheader->mph_frames = morphframe;

	morphctrl->ObMorphCurrFrame = 0;
	morphctrl->ObMorphFlags = 0;
	morphctrl->ObMorphSpeed = 0;
	morphctrl->ObMorphHeader = morphheader;

	// Copy the name over
	COPY_NAME (sbptr->SBname, doortt->nameID);

	// Setup module ref
	{
		MREF mref=doortt->my_module;
		ConvertModuleNameToPointer (&mref, MainSceneArray[0]->sm_marray);
		my_mod = mref.mref_ptr;
	}
	GLOBALASSERT (my_mod);

	my_mod->m_sbptr = sbptr;
	sbptr->SBmoptr = my_mod;
	sbptr->SBmomptr = my_mod->m_mapptr;
	sbptr->SBflags.no_displayblock = 1;

	doorbhv->door_state	= doortt->state;
	doorbhv->PDmctrl = morphctrl;
	doorbhv->door_closing_speed=doortt->door_closing_speed;
	doorbhv->door_opening_speed=doortt->door_opening_speed;

	// all lift doors have a closed starting state except the
	// one where the lift is - fill in other data

	sbptr->SBmorphctrl = doorbhv->PDmctrl;

	if(doorbhv->door_state == I_door_open)
		{
			sbptr->SBmorphctrl->ObMorphCurrFrame = 0; 
			OpenDoor(sbptr->SBmorphctrl, DOOR_OPENFASTSPEED);	
		}
	else
		{
			GLOBALASSERT(doorbhv->door_state == I_door_closed);
			sbptr->SBmorphctrl->ObMorphCurrFrame = 1; 
			CloseDoor(sbptr->SBmorphctrl, DOOR_CLOSEFASTSPEED);	
		}
	doorbhv->request_state = doorbhv->door_state;

	// copy data into relevant structures

	sbptr->SBmorphctrl = doorbhv->PDmctrl;

	if(sbptr->SBmoptr)
		{
			sbptr->SBmoptr->m_flags |= m_flag_open;
		}
	if(sbptr->SBmomptr)
		{
			sbptr->SBmomptr->MapMorphHeader = sbptr->SBmorphctrl->ObMorphHeader;
		}
			
			
  	doorbhv->SoundHandle=SOUND_NOACTIVEINDEX;
			
	return((void*)doorbhv);
}


void LiftDoorBehaveFun(STRATEGYBLOCK* sbptr)
{
	LIFT_DOOR_BEHAV_BLOCK *doorbhv;
	MORPHCTRL *mctrl;
	DISPLAYBLOCK* dptr;
	MODULE *mptr;

 	GLOBALASSERT(sbptr);
	doorbhv = (LIFT_DOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((doorbhv->bhvr_type == I_BehaviourLiftDoor));
	mctrl = doorbhv->PDmctrl;
	GLOBALASSERT(mctrl);
	mptr = sbptr->SBmoptr;
	GLOBALASSERT(mptr);
	dptr = sbptr->SBdptr;
	
	/* update morphing.... */
	UpdateMorphing(mctrl);

 	switch(doorbhv->door_state)
	{
		case I_door_opening:
		{	
			mptr->m_flags |= m_flag_open;
			if(mctrl->ObMorphFlags & mph_flag_finished)		
			{
		        if (doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
		          Sound_Play(SID_DOOREND,"d",&mptr->m_world);
		          Sound_Stop(doorbhv->SoundHandle);
		        }
				doorbhv->door_state = I_door_open;
			}
			break;
		}
		case I_door_closing:
		{
			if(mctrl->ObMorphFlags & mph_flag_finished)
			{
		        if (doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
					Sound_Play(SID_DOOREND,"d",&mptr->m_world);
					Sound_Stop(doorbhv->SoundHandle);
		        }
				doorbhv->door_state = I_door_closed;
				mptr->m_flags &= ~m_flag_open;
			}
			else if(AnythingInMyModule(sbptr->SBmoptr))
			{
				if (doorbhv->SoundHandle==SOUND_NOACTIVEINDEX)
				{
					Sound_Play(SID_DOORSTART,"d",&mptr->m_world);
					Sound_Play(SID_DOORMID,"del",&mptr->m_world,&doorbhv->SoundHandle);
				}
				OpenDoor(mctrl, doorbhv->door_opening_speed);
				doorbhv->door_state = I_door_opening;
				mptr->m_flags |= m_flag_open;
			}							
			break;
		}
		case I_door_open:
		{
			mptr->m_flags |= m_flag_open;
			if(doorbhv->request_state == I_door_closed)
			{
		 	    					
				if (doorbhv->SoundHandle==SOUND_NOACTIVEINDEX)
				{
 					Sound_Play(SID_DOORSTART,"d",&mptr->m_world);
 					Sound_Play(SID_DOORMID,"del",&mptr->m_world,&doorbhv->SoundHandle);
				}
							
				CloseDoor(mctrl, doorbhv->door_closing_speed);
				doorbhv->door_state = I_door_closing;
			}
			break;
		}
		case I_door_closed:
		{
			mptr->m_flags &= ~m_flag_open;
			if(doorbhv->request_state == I_door_open)
			{
						
				if (doorbhv->SoundHandle==SOUND_NOACTIVEINDEX)
				{
					Sound_Play(SID_DOORSTART,"d",&mptr->m_world);
					Sound_Play(SID_DOORMID,"del",&mptr->m_world,&doorbhv->SoundHandle);
				}
		
				OpenDoor(mctrl, doorbhv->door_opening_speed);
				doorbhv->door_state = I_door_opening;
				mptr->m_flags |= m_flag_open;
			}
			break;
		}
		default:
			LOCALASSERT(1==0);
	}
}



/*--------------------**
** Loading and Saving **
**--------------------*/
typedef struct lift_door_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	DOOR_STATES door_state;
	DOOR_STATES request_state;

	//from the morph control
	int ObMorphCurrFrame;
	int ObMorphFlags;
	int ObMorphSpeed;

}LIFT_DOOR_SAVE_BLOCK;

void LoadStrategy_LiftDoor(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	LIFT_DOOR_BEHAV_BLOCK *doorbhv;
	LIFT_DOOR_SAVE_BLOCK* block = (LIFT_DOOR_SAVE_BLOCK*) header; 

	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(block->header.SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourLiftDoor) return;

	doorbhv = (LIFT_DOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	doorbhv->request_state = block->request_state;
	doorbhv->door_state = block->door_state;
	
	
	doorbhv->PDmctrl->ObMorphCurrFrame = block->ObMorphCurrFrame;
	doorbhv->PDmctrl->ObMorphFlags = block->ObMorphFlags;
	doorbhv->PDmctrl->ObMorphSpeed = block->ObMorphSpeed;

	Load_SoundState(&doorbhv->SoundHandle);
	
}

void SaveStrategy_LiftDoor(STRATEGYBLOCK* sbPtr)
{
	LIFT_DOOR_SAVE_BLOCK *block;
	LIFT_DOOR_BEHAV_BLOCK *doorbhv ;
	
	doorbhv = (LIFT_DOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	block->request_state = doorbhv->request_state;
	block->door_state = doorbhv->door_state;
	block->ObMorphCurrFrame = doorbhv->PDmctrl->ObMorphCurrFrame;
	block->ObMorphFlags = doorbhv->PDmctrl->ObMorphFlags;
	block->ObMorphSpeed = doorbhv->PDmctrl->ObMorphSpeed;

	Save_SoundState(&doorbhv->SoundHandle);
}

/*---------------------------**
** End of loading and saving **
**---------------------------*/
