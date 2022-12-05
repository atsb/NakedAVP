#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_track.h"
#include "dynamics.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "plat_shp.h"
#include "pvisible.h"
#include "bh_debri.h"

extern int RealFrameTime;

void NotifyTargetsForTrackPoint(TRACK_OBJECT_BEHAV_BLOCK* to_bhv,int point_num,int reversing);

void* TrackObjectBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	TRACK_OBJECT_BEHAV_BLOCK* to_bhv;
	TRACK_OBJECT_TOOLS_TEMPLATE* to_tt;
	int i;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	to_bhv=(TRACK_OBJECT_BEHAV_BLOCK*)AllocateMem(sizeof(TRACK_OBJECT_BEHAV_BLOCK));
	if(!to_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	to_bhv->bhvr_type=I_BehaviourTrackObject;

	to_tt=(TRACK_OBJECT_TOOLS_TEMPLATE*)bhdata;

	sbptr->shapeIndex = to_tt->shape_num;
	COPY_NAME(sbptr->SBname, to_tt->nameID);

	GLOBALASSERT(to_tt->track);
	
	to_bhv->to_track=to_tt->track;
	to_bhv->to_track->sbptr=sbptr;

	GLOBALASSERT(sbptr->DynPtr);
	
	sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = to_tt->position;
	sbptr->DynPtr->OrientEuler = to_tt->orientation;
	CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
	TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	




	/*check to see if object is animated.*/
	{
		TXACTRLBLK **pptxactrlblk;		
		int item_num;
		SHAPEHEADER *shptr = GetShapeData(to_tt->shape_num);
		pptxactrlblk = &to_bhv->to_tac;
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
						pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(to_tt->shape_num, item_num);										
						pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, to_tt->shape_num);

						*pptxactrlblk = pnew_txactrlblk;
						pptxactrlblk = &pnew_txactrlblk->tac_next;
					}
					else *pptxactrlblk = NULL; 
				}
		}
		*pptxactrlblk=0;
	}

	to_bhv->request=track_no_request;


	to_bhv->num_special_track_points=to_tt->num_special_track_points;
	to_bhv->special_track_points=to_tt->special_track_points;

	to_bhv->destruct_target_request=to_tt->destruct_target_request;
	for(i=0;i<SB_NAME_LENGTH;i++)
	{
		to_bhv->destruct_target_ID[i]=to_tt->destruct_target_ID[i];
	}
	to_bhv->destruct_target_sbptr=0;

	/* Initialise object's stats */
	{
		NPC_DATA *NpcData;
   
		NpcData=GetThisNpcData(I_NPC_DefaultInanimate);
		LOCALASSERT(NpcData);
		sbptr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbptr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbptr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
	}

	sbptr->SBDamageBlock.Health*=to_tt->integrity;
	
	if (to_tt->integrity > 20)
	{
		to_bhv->Indestructable = Yes;
		sbptr->integrity = DEFAULT_OBJECT_INTEGRITY;
	}
	else if (to_tt->integrity < 1)
	{
		to_bhv->Indestructable = No;
		sbptr->integrity = 1; // die immediately
	}	  
	else
	{
		to_bhv->Indestructable = No;
		sbptr->integrity = (DEFAULT_OBJECT_INTEGRITY)*(to_tt->integrity);
	}

	
	Update_Track_Position_Only(to_bhv->to_track);

	to_bhv->TimeUntilNetSynchAllowed=0;
	
	return ((void*)to_bhv);
}

void NotifyTargetsForTrackPoint(TRACK_OBJECT_BEHAV_BLOCK* to_bhv,int point_num,int reversing)
{
	int i;
	GLOBALASSERT(to_bhv);

	for(i=0;i<to_bhv->num_special_track_points;i++)
	{
		if(to_bhv->special_track_points[i].track_point_no==point_num)
		{
			SPECIAL_TRACK_POINT* stp=&to_bhv->special_track_points[i];
			int j;
			
			for(j=0;j<stp->num_targets;j++)
			{
				TRACK_POINT_TARGET* tpt=&stp->targets[j];
				if(reversing)
				{
					if(tpt->flags & TrackRequestFlag_ActiveBackward)
					{
						if(tpt->flags & TrackRequestFlag_OppositeBackward)
						{
							RequestState(tpt->target_sbptr,!tpt->request,0);
						}
						else
						{
							RequestState(tpt->target_sbptr,tpt->request,0);
						}
					}
				 }
				else
				{
					if(tpt->flags & TrackRequestFlag_ActiveForward)
					{
						RequestState(tpt->target_sbptr,tpt->request,0);
					}
				}
			}
			
			break;
		}
	}
}

void TrackObjectBehaveFun(STRATEGYBLOCK* sbptr)
{
	TRACK_OBJECT_BEHAV_BLOCK *to_bhv;
	TRACK_CONTROLLER* track;
	int current_section;
	int reversing;
 	GLOBALASSERT(sbptr);
	to_bhv = (TRACK_OBJECT_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((to_bhv->bhvr_type == I_BehaviourTrackObject));
	GLOBALASSERT(to_bhv->to_track);
	track=to_bhv->to_track;

	

	//update texture animation if object has one
	if(to_bhv->to_tac)
	{
		DISPLAYBLOCK* dptr = sbptr->SBdptr;

		if(dptr)
		{
			if(!dptr->ObTxAnimCtrlBlks)
				{ 
					dptr->ObTxAnimCtrlBlks = to_bhv->to_tac;
				}
		}
		
	}

	//check to see if any request have come through.

	if(AvP.Network != I_No_Network)
	{
		if(to_bhv->request == track_no_request)
		{
			to_bhv->TimeUntilNetSynchAllowed-=RealFrameTime;
			if(to_bhv->TimeUntilNetSynchAllowed<0)
			{
				to_bhv->TimeUntilNetSynchAllowed=0;
			}
		}
		else
		{
			to_bhv->TimeUntilNetSynchAllowed=5*ONE_FIXED;
		}
	}

	switch(to_bhv->request)
	{
		case track_request_start :
		{
			Start_Track_Playing(track);

			to_bhv->request=track_no_request;
		}
		break;
	
		case track_request_startforward :
		{
			track->reverse=0;
			Start_Track_Playing(track);

			to_bhv->request=track_no_request;
		}
		break;

		case track_request_startbackward :
		{
			track->reverse=1;
			Start_Track_Playing(track);

			to_bhv->request=track_no_request;
		}
		break;
		
		case track_request_stop :
		{
 			Stop_Track_Playing(track);
			
			to_bhv->request=track_no_request;
		}
		break;
		
		default: ;
	}

	if(!track->playing) return;

	current_section=track->current_section;
	reversing=track->reverse;

	Update_Track_Position(track);

	if(to_bhv->special_track_points)
	{
		//see which track points the track has gone through, and notify the
		//targets for those points
		
		/*
		if(reversing)
		{
			while(current_section!=track->current_section)
			{
				NotifyTargetsForTrackPoint(to_bhv,current_section,1);
				current_section--;
				if(current_section<0)
				{
					//looped to end , so passed last point
					current_section=track->num_sections-1;
					NotifyTargetsForTrackPoint(to_bhv,track->num_sections,1);
				}
			}
			if(current_section==0 && !track->playing)
			{
				//stopped at start , so passed point 0
				NotifyTargetsForTrackPoint(to_bhv,0,1);
			}
		}
		else
		{
			while(current_section!=track->current_section)
			{
				NotifyTargetsForTrackPoint(to_bhv,current_section+1,0);
				current_section++;
				if(current_section>=track->num_sections)
				{
					//looped to start , so passed first point
					current_section=0;
					NotifyTargetsForTrackPoint(to_bhv,0,0);
				}
			}
			if(current_section==(track->num_sections-1) && !track->playing)
			{
				//stopped at end , so passed last point
				NotifyTargetsForTrackPoint(to_bhv,track->num_sections,0);
			}
		
		}
		*/
		while(reversing!=track->reverse || current_section!=track->current_section)
		{
			if(reversing)
			{
				NotifyTargetsForTrackPoint(to_bhv,current_section,1);
				current_section--;
				if(current_section<0)
				{
					if(track->loop)
					{
						//looped to end , so passed last point
						current_section=track->num_sections-1;
						NotifyTargetsForTrackPoint(to_bhv,track->num_sections,1);
					}
					else
					{
						current_section=0;
						reversing=0;
					}


				}
				
			}
			else
			{
				NotifyTargetsForTrackPoint(to_bhv,current_section+1,0);
				current_section++;
				if(current_section>=track->num_sections)
				{
					if(track->loop)
					{
						//looped to start , so passed first point
						current_section=0;
						NotifyTargetsForTrackPoint(to_bhv,0,0);
					}
					else
					{
						current_section=track->num_sections-1;
						reversing=1;
					}


				}
			}
		}
		if(!track->playing)
		{
			if(track->reverse)
			{
				//stopped at end , so passed last point
				NotifyTargetsForTrackPoint(to_bhv,track->num_sections,0);
			}
			else
			{
				//stopped at start , so passed point 0
				NotifyTargetsForTrackPoint(to_bhv,0,1);
			}

		}
	}
	

}


void TrackObjectIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	TRACK_OBJECT_BEHAV_BLOCK* to_bhv = sbPtr->SBdataptr;
	LOCALASSERT(to_bhv);

	#if 0
	
	if((AvP.Network==I_Peer)&&(!InanimateDamageFromNetHost))
	{
		//add track damaged net message
		return;
	}
	else if(AvP.Network==I_Host) 
	{
		//add track damaged net message
		//if(sbPtr->SBDamageBlock.Health <= 0) AddNetMsg_InanimateObjectDestroyed(sbPtr);
	}
	#endif

	if(AvP.Network != I_No_Network)
	{
		//I don't want to consider destructable track objects in net games for the moment
		//would screw up the strategy synching
		return;
	}
		
	if (!to_bhv->Indestructable)
	{
		
		if(sbPtr->SBDamageBlock.Health <= 0) 
		{
			
			//notify target of destruction
			if(to_bhv->destruct_target_sbptr)
			{
				RequestState(to_bhv->destruct_target_sbptr,to_bhv->destruct_target_request,0);
			}
				
			MakeFragments(sbPtr);
			DestroyAnyStrategyBlock(sbPtr);
		}
	}
}

#define TRACKSYNCH_ON_FORWARD 0
#define TRACKSYNCH_OFF_FORWARD 1
#define TRACKSYNCH_ON_REVERSE 2
#define TRACKSYNCH_OFF_REVERSE 3

int TrackObjectGetSynchData(STRATEGYBLOCK* sbPtr)
{
	TRACK_OBJECT_BEHAV_BLOCK* to_bhv = sbPtr->SBdataptr;
	LOCALASSERT(to_bhv);

	if(to_bhv->to_track->playing)	
	{
		if(to_bhv->to_track->reverse)
			return TRACKSYNCH_ON_REVERSE;
		else
			return TRACKSYNCH_ON_FORWARD;
	}
	else
	{
		if(to_bhv->to_track->reverse)
			return TRACKSYNCH_OFF_REVERSE;
		else
			return TRACKSYNCH_OFF_FORWARD;
	}
}


void TrackObjectSetSynchData(STRATEGYBLOCK* sbPtr,int status)
{
	TRACK_OBJECT_BEHAV_BLOCK* to_bhv = sbPtr->SBdataptr;
	LOCALASSERT(to_bhv);
	
	if(to_bhv->TimeUntilNetSynchAllowed>0)
	{
		//ignore this attempt to synch the switch
		return;
	}

	if(to_bhv->to_track->playing)
	{
		//don't bother stopping moving tracks;
		return;
	}

	switch(status)
	{
		case TRACKSYNCH_ON_FORWARD :
			to_bhv->request=track_request_startforward;
			break;

		case TRACKSYNCH_OFF_FORWARD :
			if(to_bhv->to_track->reverse && !to_bhv->to_track->loop)
			{
				//track is at the end , but should be at the start
				to_bhv->request=track_request_startbackward;
			}
			break;

		case TRACKSYNCH_ON_REVERSE :
			to_bhv->request=track_request_startbackward;
			break;

		case TRACKSYNCH_OFF_REVERSE :
			if(!to_bhv->to_track->reverse && !to_bhv->to_track->loop)
			{
				//track is at the start , but should be at the end
				to_bhv->request=track_request_startforward;
			}
			break;

	}
}



/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct track_object_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL Indestructable;
	TRACK_OBJECT_REQUEST_STATE request;

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;

}TRACK_OBJECT_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV to_bhv

void LoadStrategy_TrackObject(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	TRACK_OBJECT_BEHAV_BLOCK *to_bhv;
	TRACK_OBJECT_SAVE_BLOCK* block = (TRACK_OBJECT_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourTrackObject) return;

	to_bhv = (TRACK_OBJECT_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	
	COPYELEMENT_LOAD(Indestructable)
	COPYELEMENT_LOAD(request)

	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load the track position
	if(to_bhv->to_track)
	{
		SAVE_BLOCK_HEADER* track_header = GetNextBlockIfOfType(SaveBlock_Track);
		if(track_header)
		{
			LoadTrackPosition(track_header,to_bhv->to_track);
		}
	}
	
}

void SaveStrategy_TrackObject(STRATEGYBLOCK* sbPtr)
{
	TRACK_OBJECT_SAVE_BLOCK* block; 
	TRACK_OBJECT_BEHAV_BLOCK *to_bhv;
	
	to_bhv = (TRACK_OBJECT_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	
	COPYELEMENT_SAVE(Indestructable)
	COPYELEMENT_SAVE(request)
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the track position
	if(to_bhv->to_track)
	{
		SaveTrackPosition(to_bhv->to_track);
	}

}
