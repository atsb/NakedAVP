/*------------------------------Patrick 12/3/97-----------------------------------
  Source for Switch Operated Doors
  --------------------------------------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "triggers.h"
#include "psnd.h"

#define UseLocalAssert Yes

#include "ourasert.h"
#include "pvisible.h"
#include "bh_swdor.h"
#include "savegame.h"

/* external stuff */

extern int NormalFrameTime;

/*---------------------Patrick 12/3/97-------------------------
  Initialisation of a switch door....
  NB asumes that all switch doors start in a 'closed' state
  -------------------------------------------------------------*/
void InitialiseSwitchDoor(void* bhdata, STRATEGYBLOCK* sbPtr)
{
	SWITCH_DOOR_BEHAV_BLOCK *switchDoorBehaviourPtr;
	SWITCH_DOOR_TOOLS_TEMPLATE *switchDoorToolsData;
	MORPHCTRL *morphCtrl;	
	MORPHHEADER *morphHeader;
	MORPHFRAME *morphFrame;

	/* create a switch door data block */
	switchDoorBehaviourPtr = (SWITCH_DOOR_BEHAV_BLOCK*)AllocateMem(sizeof(SWITCH_DOOR_BEHAV_BLOCK));
	if (!switchDoorBehaviourPtr)
	{
		memoryInitialisationFailure = 1;
		return;
	}

	switchDoorBehaviourPtr->myBehaviourType = I_BehaviourSwitchDoor;
	sbPtr->SBdataptr = (void *)switchDoorBehaviourPtr;	
	/* cast the tools data for access */
	switchDoorToolsData = (SWITCH_DOOR_TOOLS_TEMPLATE *)bhdata;	

	/* Set up a new Morph Control */
 	morphFrame = (MORPHFRAME*)AllocateMem(sizeof(MORPHFRAME));
	if (!morphFrame)
	{
		memoryInitialisationFailure = 1;
		return;
	}
	morphFrame->mf_shape1 = switchDoorToolsData->shapeOpen;
	morphFrame->mf_shape2 = switchDoorToolsData->shapeClosed;
	morphHeader = (MORPHHEADER*)AllocateMem(sizeof(MORPHHEADER));
	if (!morphHeader)
	{
		memoryInitialisationFailure = 1;
		return;
	}
	morphHeader->mph_numframes = 1;
	morphHeader->mph_maxframes = ONE_FIXED;
	morphHeader->mph_frames = morphFrame;
	morphCtrl = (MORPHCTRL*)AllocateMem(sizeof(MORPHCTRL));
	if (!morphCtrl)
	{
		memoryInitialisationFailure = 1;
		return;
	}
	morphCtrl->ObMorphCurrFrame = 0;
	morphCtrl->ObMorphFlags = 0;
	morphCtrl->ObMorphSpeed = 0;
	morphCtrl->ObMorphHeader = morphHeader;
	switchDoorBehaviourPtr->morfControl = sbPtr->SBmorphctrl = morphCtrl;

	/* set up my module, and it's morph controls */
	COPY_NAME(sbPtr->SBname, switchDoorToolsData->nameID);
	{
		MREF mref=switchDoorToolsData->myModule;
		ConvertModuleNameToPointer(&mref, MainSceneArray[0]->sm_marray);
		GLOBALASSERT(mref.mref_ptr);
		GLOBALASSERT(mref.mref_ptr->m_mapptr);
		mref.mref_ptr->m_sbptr = sbPtr;
		sbPtr->SBmoptr = mref.mref_ptr;
	}
	sbPtr->SBmomptr = sbPtr->SBmoptr->m_mapptr;
	sbPtr->SBmomptr->MapMorphHeader = sbPtr->SBmorphctrl->ObMorphHeader;
	sbPtr->SBmoptr->m_flags &= ~m_flag_open;	

	/* set up some other behaviour block stuff */
	COPY_NAME(switchDoorBehaviourPtr->linkedDoorName, switchDoorToolsData->linkedDoorName);
	switchDoorBehaviourPtr->doorState = I_door_closed;
	switchDoorBehaviourPtr->linkedDoorPtr = (STRATEGYBLOCK *)0;
	switchDoorBehaviourPtr->requestOpen = 0;
	switchDoorBehaviourPtr->requestClose = 0;
	switchDoorBehaviourPtr->openTimer = 0;
	switchDoorBehaviourPtr->SoundHandle = SOUND_NOACTIVEINDEX;
	CloseDoor(sbPtr->SBmorphctrl, DOOR_CLOSEFASTSPEED);	

	{
		// Work out the door sound pitch
	
		int maxX,maxY,maxZ,doorSize;

		maxX=mainshapelist[morphFrame->mf_shape2]->shapemaxx;
		maxY=mainshapelist[morphFrame->mf_shape2]->shapemaxy;
		maxZ=mainshapelist[morphFrame->mf_shape2]->shapemaxz;
			
		doorSize = maxX + maxY + maxZ;
		if (doorSize < 3000) doorSize = 3000;
		else if (doorSize > 8000) doorSize = 8000;
		
		doorSize = (3000 - doorSize) >> 4;

		switchDoorBehaviourPtr->doorType = doorSize; 

	}

}



/*---------------------Patrick 13/3/97-------------------------
  Switch door behaviour function.
  -------------------------------------------------------------*/
void SwitchDoorBehaviour(STRATEGYBLOCK* sbPtr)
{
	SWITCH_DOOR_BEHAV_BLOCK *doorBehaviour;
	MORPHCTRL *mCtrl;
	MODULE *mPtr;
	int linkedDoorIsClosed = 1;

 	GLOBALASSERT(sbPtr);
	doorBehaviour = (SWITCH_DOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;
	GLOBALASSERT(doorBehaviour);
	mCtrl = doorBehaviour->morfControl;
	GLOBALASSERT(mCtrl);
	mPtr = sbPtr->SBmoptr;
	GLOBALASSERT(mPtr);
	
	/* update morphing.... */
	UpdateMorphing(mCtrl);

 	/* get state of linked door: if there isn't a linked door, 'linkeddoorisclosed' 
 	remains true so that there is no obstruction to operation of this door.
 	
 	NB can't use 'GetState' here, as it returns true only if the door is fully open
 	(used by the NPC's).  Here, we need to determine if the door is closed (which is 
 	not the same as !open) */
	if(doorBehaviour->linkedDoorPtr)
	{
		if(((SWITCH_DOOR_BEHAV_BLOCK *)doorBehaviour->linkedDoorPtr->SBdataptr)->doorState != I_door_closed) linkedDoorIsClosed = 0;
 	}
 	
 	switch(doorBehaviour->doorState)
	{
		case I_door_opening:
		{	
			/* LOCALASSERT(linkedDoorIsClosed);	*/
			/* check if we've got a close request */
			if(doorBehaviour->requestClose && !AnythingInMyModule(sbPtr->SBmoptr)) 
			{
				if(sbPtr->SBdptr) CloseDoor(mCtrl, DOOR_CLOSESLOWSPEED);
				else CloseDoor(mCtrl, DOOR_CLOSEFASTSPEED);
				doorBehaviour->doorState = I_door_closing;
			}
			/* already opening, so just allow the door to continue... */
			else if(mCtrl->ObMorphFlags & mph_flag_finished) 
			{
				//door has finished opening
				doorBehaviour->doorState = I_door_open;
				doorBehaviour->openTimer = DOOR_FAROPENTIME;
		        
		        if (doorBehaviour->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
			    	Sound_Play(SID_DOOREND,"dp",&mPtr->m_world,doorBehaviour->doorType);
				  	Sound_Stop(doorBehaviour->SoundHandle);
		        }
				
			}
			break;
		}		
		case I_door_closing:
		{
			/* LOCALASSERT(linkedDoorIsClosed);	*/
			/* check if we've got an open request, or anything has jumped in */
			if((doorBehaviour->requestOpen)||(AnythingInMyModule(sbPtr->SBmoptr)))
			{
				//have to start opening again
				if(sbPtr->SBdptr) OpenDoor(mCtrl, DOOR_OPENSLOWSPEED);
				else OpenDoor(mCtrl, DOOR_OPENFASTSPEED);
				doorBehaviour->doorState = I_door_opening;

				Sound_Play(SID_DOORSTART,"dp",&mPtr->m_world,doorBehaviour->doorType);
		 		Sound_Play(SID_DOORMID,"delp",&mPtr->m_world,&doorBehaviour->SoundHandle,doorBehaviour->doorType);

			}
			/* check if we've finished closing */
			else if(mCtrl->ObMorphFlags & mph_flag_finished)
			{
				doorBehaviour->doorState = I_door_closed;
				mPtr->m_flags &= ~m_flag_open;
		        
		        if (doorBehaviour->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
			    	Sound_Play(SID_DOOREND,"dp",&mPtr->m_world,doorBehaviour->doorType);
				  	Sound_Stop(doorBehaviour->SoundHandle);
		        }

			}
			break;
		}
		case I_door_open:
		{
			/* LOCALASSERT(linkedDoorIsClosed);	*/		
			/*if we've got a close request , set the open timer to 0 
				so the door will start closing*/
			if(doorBehaviour->requestClose)
			{
				doorBehaviour->openTimer=0;
			}
			/* check our timer to see if it's time to close*/
			if(doorBehaviour->openTimer <= 0)
			{
				/* make sure there's nothing inside the door module before closing */
				if(AnythingInMyModule(sbPtr->SBmoptr)==0)
				{
					if(sbPtr->SBdptr) CloseDoor(mCtrl, DOOR_CLOSESLOWSPEED);
					else CloseDoor(mCtrl, DOOR_CLOSEFASTSPEED);
					doorBehaviour->doorState = I_door_closing;
					doorBehaviour->openTimer = 0;
				
					Sound_Play(SID_DOORSTART,"dp",&mPtr->m_world,doorBehaviour->doorType);
			 		Sound_Play(SID_DOORMID,"delp",&mPtr->m_world,&doorBehaviour->SoundHandle,doorBehaviour->doorType);
				}
			}
			else doorBehaviour->openTimer -= NormalFrameTime;
			break;
		}
		case I_door_closed:
		{
			if((doorBehaviour->requestOpen)&&(linkedDoorIsClosed))
			{
				/* just open the door */
				if(sbPtr->SBdptr) OpenDoor(mCtrl, DOOR_OPENSLOWSPEED);
				else OpenDoor(mCtrl, DOOR_OPENFASTSPEED);
				doorBehaviour->doorState = I_door_opening;
				mPtr->m_flags |= m_flag_open;

				Sound_Play(SID_DOORSTART,"dp",&mPtr->m_world,doorBehaviour->doorType);
		 		Sound_Play(SID_DOORMID,"delp",&mPtr->m_world,&doorBehaviour->SoundHandle,doorBehaviour->doorType);
	
			}
		}
	}

	/* must reset this every frame */
	doorBehaviour->requestOpen = 0;
	doorBehaviour->requestClose = 0;
}





/*--------------------**
** Loading and Saving **
**--------------------*/
typedef struct switch_door_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	DOOR_STATES doorState;
	int openTimer;
	unsigned int requestOpen :1;
	unsigned int requestClose :1;
		
	//from the morph control
	int ObMorphCurrFrame;
	int ObMorphFlags;
	int ObMorphSpeed;

}SWITCH_DOOR_SAVE_BLOCK;



//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV doorbhv

void LoadStrategy_SwitchDoor(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	SWITCH_DOOR_BEHAV_BLOCK *doorbhv;
	SWITCH_DOOR_SAVE_BLOCK* block = (SWITCH_DOOR_SAVE_BLOCK*) header; 

	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourSwitchDoor) return;

	doorbhv = (SWITCH_DOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	COPYELEMENT_LOAD(doorState)
	COPYELEMENT_LOAD(openTimer)
	COPYELEMENT_LOAD(requestOpen)
	COPYELEMENT_LOAD(requestClose)

	COPYELEMENT_LOAD_EXT(block->ObMorphCurrFrame,doorbhv->morfControl->ObMorphCurrFrame)
	COPYELEMENT_LOAD_EXT(block->ObMorphFlags , doorbhv->morfControl->ObMorphFlags)
	COPYELEMENT_LOAD_EXT(block->ObMorphSpeed , doorbhv->morfControl->ObMorphSpeed)

	Load_SoundState(&doorbhv->SoundHandle);
}

void SaveStrategy_SwitchDoor(STRATEGYBLOCK* sbPtr)
{
	SWITCH_DOOR_SAVE_BLOCK *block;
	SWITCH_DOOR_BEHAV_BLOCK *doorbhv ;
	
	doorbhv = (SWITCH_DOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	COPYELEMENT_SAVE(doorState)
	COPYELEMENT_SAVE(openTimer)
	COPYELEMENT_SAVE(requestOpen)
	COPYELEMENT_SAVE(requestClose)
	
	COPYELEMENT_SAVE_EXT(block->ObMorphCurrFrame,doorbhv->morfControl->ObMorphCurrFrame)
	COPYELEMENT_SAVE_EXT(block->ObMorphFlags , doorbhv->morfControl->ObMorphFlags)
	COPYELEMENT_SAVE_EXT(block->ObMorphSpeed , doorbhv->morfControl->ObMorphSpeed)

	Save_SoundState(&doorbhv->SoundHandle);
	
}






