#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "bh_types.h"
#include "bh_mission.h"
#include "ourasert.h"

extern void MissionObjectiveTriggered(void* mission_objective);
extern void MakeMissionVisible(void* mission_objective);
extern void MakeMissionPossible(void* mission_objective);
extern void	StartTriggerPlotFMV(int number);

void * MissionCompleteBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	MISSION_COMPLETE_BEHAV_BLOCK* m_bhv;
	MISSION_COMPLETE_TOOLS_TEMPLATE* mc_tt;

 	GLOBALASSERT(sbptr);
	
	m_bhv=(MISSION_COMPLETE_BEHAV_BLOCK*)AllocateMem(sizeof(MISSION_COMPLETE_BEHAV_BLOCK));
	if(!m_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	mc_tt= (MISSION_COMPLETE_TOOLS_TEMPLATE*) bhdata;
	
	m_bhv->bhvr_type=I_BehaviourMissionComplete;
	sbptr->shapeIndex = 0;
	COPY_NAME(sbptr->SBname, mc_tt->nameID);


	m_bhv->mission_objective_ptr=mc_tt->mission_objective_ptr;

	return((void*)m_bhv);
}

#define MissionTrigger_MakeVisible 0x00000001
#define MissionTrigger_MakePossible 0x00000002
#define MissionTrigger_DontComplete 0x00000004

void SendRequestToMissionStrategy(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
	MISSION_COMPLETE_BEHAV_BLOCK *mc_bhv;
	GLOBALASSERT(sbptr);
	GLOBALASSERT(sbptr->SBdataptr);
	mc_bhv = (MISSION_COMPLETE_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((mc_bhv->bhvr_type == I_BehaviourMissionComplete));
	
	if(!state) return;
	
	if(extended_data & MissionTrigger_MakeVisible)
	{
		MakeMissionVisible(mc_bhv->mission_objective_ptr);
	}
	if(extended_data & MissionTrigger_MakePossible)
	{
		MakeMissionPossible(mc_bhv->mission_objective_ptr);
	}
	if(!(extended_data & MissionTrigger_DontComplete))
	{
		MissionObjectiveTriggered(mc_bhv->mission_objective_ptr);
	}

}




void * MessageBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	MESSAGE_BEHAV_BLOCK* m_bhv;
	MESSAGE_TOOLS_TEMPLATE* m_tt;

 	GLOBALASSERT(sbptr);
	
	m_bhv=(MESSAGE_BEHAV_BLOCK*)AllocateMem(sizeof(MESSAGE_BEHAV_BLOCK));
	if(!m_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	m_tt= (MESSAGE_TOOLS_TEMPLATE*) bhdata;
	
	m_bhv->bhvr_type=I_BehaviourMessage;
	sbptr->shapeIndex = 0;
	COPY_NAME(sbptr->SBname, m_tt->nameID);


	m_bhv->string_no=m_tt->string_no;
	m_bhv->active=m_tt->active;

	return((void*)m_bhv);
}

#define TextMessageRequest_Activate 1
#define TextMessageRequest_ActivateAndDisplay 2 

void SendRequestToMessageStrategy(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
	MESSAGE_BEHAV_BLOCK *m_bhv;
	GLOBALASSERT(sbptr);
	GLOBALASSERT(sbptr->SBdataptr);
	m_bhv = (MESSAGE_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((m_bhv->bhvr_type == I_BehaviourMessage));
	

	switch(extended_data)
	{
		case TextMessageRequest_Activate :
			m_bhv->active=state;
			break;

		case TextMessageRequest_ActivateAndDisplay :
			m_bhv->active=state;
			
			if(m_bhv->active)
			{
				PrintStringTableEntryInConsole(m_bhv->string_no);
				StartTriggerPlotFMV(m_bhv->string_no);
			}
			break;

		default : 
			if(m_bhv->active && state)
			{
				PrintStringTableEntryInConsole(m_bhv->string_no);
				StartTriggerPlotFMV(m_bhv->string_no);
			}
	}
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct message_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL active;

}MESSAGE_SAVE_BLOCK;


void LoadStrategy_Message(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	MESSAGE_BEHAV_BLOCK* m_bhv;
	MESSAGE_SAVE_BLOCK* block = (MESSAGE_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourMessage) return;

	m_bhv = (MESSAGE_BEHAV_BLOCK *)sbPtr->SBdataptr;

	//start copying stuff

	m_bhv->active = block->active;
}

void SaveStrategy_Message(STRATEGYBLOCK* sbPtr)
{
	MESSAGE_BEHAV_BLOCK* m_bhv;
	MESSAGE_SAVE_BLOCK* block; 
	
	m_bhv = (MESSAGE_BEHAV_BLOCK *)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	block->active = m_bhv->active;
}



/*------------------------------**
** Loading/Saving mission state **
**------------------------------*/
extern int GetMissionStateForSave(void* mission_objective);
extern void SetMissionStateFromLoad(void* mission_objective,int state);


typedef struct mission_complete_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int state;

}MISSION_COMPLETE_SAVE_BLOCK;


void LoadStrategy_MissionComplete(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	MISSION_COMPLETE_BEHAV_BLOCK* m_bhv;
	MISSION_COMPLETE_SAVE_BLOCK* block = (MISSION_COMPLETE_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourMissionComplete) return;

	m_bhv = (MISSION_COMPLETE_BEHAV_BLOCK *)sbPtr->SBdataptr;

	//start copying stuff
   	SetMissionStateFromLoad(m_bhv->mission_objective_ptr , block->state);
}

void SaveStrategy_MissionComplete(STRATEGYBLOCK* sbPtr)
{
	MISSION_COMPLETE_BEHAV_BLOCK* m_bhv;
	MISSION_COMPLETE_SAVE_BLOCK* block; 
	
	m_bhv = (MISSION_COMPLETE_BEHAV_BLOCK *)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	block->state = GetMissionStateForSave(m_bhv->mission_objective_ptr);

}
