#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "dynamics.h"
#include "bh_plachier.h"
#include "ourasert.h"
#include "jsndsup.h"
#include "psndplat.h"

extern MODULEMAPBLOCK VisibilityDefaultObjectMap;

/*
void PlacedHierarchyRestart(PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv)
{
}

void PlacedHierarchyStop(PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv)
{
}
*/
SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char *);

void* PlacedHierarchyBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	PLACED_HIERARCHY_BEHAV_BLOCK* ph_bhv;
	PLACED_HIERARCHY_TOOLS_TEMPLATE* ph_tt;
  	SECTION *root_section;
	SECTION_DATA* sound_section_data;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	ph_bhv=(PLACED_HIERARCHY_BEHAV_BLOCK*)AllocateMem(sizeof(PLACED_HIERARCHY_BEHAV_BLOCK));
	if(!ph_bhv)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	ph_bhv->bhvr_type=I_BehaviourPlacedHierarchy;

	ph_tt=(PLACED_HIERARCHY_TOOLS_TEMPLATE*)bhdata;

	sbptr->shapeIndex=0;
	COPY_NAME(sbptr->SBname, ph_tt->nameID);
	
	sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = ph_tt->position;
	sbptr->DynPtr->OrientEuler = ph_tt->orientation;
	CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
	TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	

	ph_bhv->num_sequences=ph_tt->num_sequences;
	ph_bhv->sequences=ph_tt->sequences;
	ph_bhv->num_sounds=ph_tt->num_sounds;
	ph_bhv->sounds=ph_tt->sounds;

	ph_bhv->num_special_track_points=ph_tt->num_special_track_points;
	ph_bhv->special_track_points=ph_tt->special_track_points;
	
	root_section=GetNamedHierarchyFromLibrary(ph_tt->file_name,ph_tt->hier_name);
	GLOBALASSERT(root_section);
	Create_HModel(&ph_bhv->HModelController,root_section);
	
	ph_bhv->current_seq=ph_tt->first_sequence;
	InitHModelSequence(&ph_bhv->HModelController,ph_bhv->current_seq->sequence_no,ph_bhv->current_seq->sub_sequence_no,ph_bhv->current_seq->time);
	ph_bhv->HModelController.Playing=ph_tt->playing;
	ph_bhv->HModelController.Looped=ph_bhv->current_seq->loop;

	//find the hierarchy section that sound should be produced from
	sound_section_data=GetThisSectionData(ph_bhv->HModelController.section_data,"SoundSource");
	if(!sound_section_data)
	{
		//if there isn't a SoundSource object , sound can come from the root section
		sound_section_data=ph_bhv->HModelController.section_data;
	}
	ph_bhv->sound_location=&sound_section_data->World_Offset;


	return ((void*)ph_bhv);
}


void MakePlacedHierarchyNear(STRATEGYBLOCK* sbptr)
{
	
	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr = sbptr->DynPtr;
	PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv= (PLACED_HIERARCHY_BEHAV_BLOCK*)(sbptr->SBdataptr);    

    LOCALASSERT(dynPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);

	VisibilityDefaultObjectMap.MapShape = 0;
	tempModule.m_mapptr = &VisibilityDefaultObjectMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr = NULL; /* this is important */
	tempModule.name = NULL; /* this is important */

	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;		
	if(dPtr==NULL) return; /* cannot create displayblock, so leave object "far" */
	
	sbptr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbptr;
	dPtr->ObMyModule = NULL;					
                            
	/* also need to initialise positional information in the new display
	block from the existing dynamics block: this necessary because this 
	function is (usually) called between the dynamics and rendering systems
	so it is not initialised by the dynamics system the first time it is
	drawn. */
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;
	
	

	dPtr->HModelControlBlock=&ph_bhv->HModelController;
	ProveHModel(dPtr->HModelControlBlock,dPtr);
	
}

void PlacedHierarchyBehaveFun(STRATEGYBLOCK* sbptr)
{
	PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv;

	GLOBALASSERT(sbptr);
	ph_bhv =(PLACED_HIERARCHY_BEHAV_BLOCK*) sbptr->SBdataptr;
	GLOBALASSERT((ph_bhv->bhvr_type) == I_BehaviourPlacedHierarchy);

	if(sbptr->SBdptr)
	{
		ProveHModel(&ph_bhv->HModelController,sbptr->SBdptr);
	}
	else
	{
		ProveHModel_Far(&ph_bhv->HModelController,sbptr);
	}

	//update sound
	if(ph_bhv->HModelController.Playing && ph_bhv->current_seq)
	{
		int i,j;
		int timer=ph_bhv->HModelController.sequence_timer;
		int keyframe_flags=ph_bhv->HModelController.keyframe_flags;

		if(keyframe_flags)
		{
			for(i=0;i<ph_bhv->num_special_track_points;i++)
			{
				if(keyframe_flags & ph_bhv->special_track_points[i].track_point_no)
				{
					SPECIAL_TRACK_POINT* stp=&ph_bhv->special_track_points[i];
					for(j=0;j<stp->num_targets;j++)
					{
						TRACK_POINT_TARGET* tpt=&stp->targets[j];
				 		RequestState(tpt->target_sbptr,tpt->request,0);
					}
									
				}	
			}

		}
		

		for(i=0;i<ph_bhv->current_seq->num_sound_times;i++)
		{
			PLACED_HIERARCHY_SOUND_TIMES* s_time=&ph_bhv->current_seq->sound_times[i];
			if(s_time->sound)
			{
				PLACED_HIERARCHY_SOUND* sound=s_time->sound;
				//not much point in continuing if the sound wasn't loaded anyway
				if(sound->sound_loaded)
				{
					if(timer>=s_time->start_time && timer<s_time->end_time)
					{
						//start sound if not already playing
						if(!sound->playing)
						{
							
							int dist=VectorDistance(&Player->ObWorld,ph_bhv->sound_location);
							if(dist<=sound->outer_range) //make sure sound is in range
							{
								SOUND3DDATA s3d;
								s3d.position = *ph_bhv->sound_location;
								s3d.inner_range = sound->inner_range;
								s3d.outer_range = sound->outer_range;
								s3d.velocity.vx = 0;
								s3d.velocity.vy = 0;
								s3d.velocity.vz = 0;
	
								if(s_time->sound->loop)
								{
									Sound_Play ((SOUNDINDEX)sound->sound_loaded->sound_num, "nvpel", &s3d,sound->max_volume,sound->pitch,&sound->activ_no);
								}
								else
								{
									Sound_Play ((SOUNDINDEX)sound->sound_loaded->sound_num, "nvpe", &s3d,sound->max_volume,sound->pitch,&sound->activ_no);
								}
							}
							sound->playing=1;
						}
						//otherwise update its position
						else
						{
							int dist=VectorDistance(&Player->ObWorld,ph_bhv->sound_location);
							if(sound->activ_no!=SOUND_NOACTIVEINDEX)
							{
								if(dist<=sound->outer_range)
								{
									SOUND3DDATA s3d;
									s3d.position = *ph_bhv->sound_location;
									s3d.inner_range = sound->inner_range;
									s3d.outer_range = sound->outer_range;
									s3d.velocity.vx = 0;
									s3d.velocity.vy = 0;
									s3d.velocity.vz = 0;
									Sound_UpdateNew3d (sound->activ_no, &s3d);
								}
								else
								{
									Sound_Stop(sound->activ_no);
								}
							}
							else
							{
								if(dist<=sound->outer_range && sound->loop)
								{
									SOUND3DDATA s3d;
									s3d.position = *ph_bhv->sound_location;
									s3d.inner_range = sound->inner_range;
									s3d.outer_range = sound->outer_range;
									s3d.velocity.vx = 0;
									s3d.velocity.vy = 0;
									s3d.velocity.vz = 0;
									Sound_Play ((SOUNDINDEX)sound->sound_loaded->sound_num, "nvpel", &s3d,sound->max_volume,sound->pitch,&sound->activ_no);
								}
							}
						}
					}
					else
					{
						//stop sound
						if(sound->activ_no!=SOUND_NOACTIVEINDEX)
						{
							Sound_Stop(sound->activ_no);
						}
						sound->playing=0;
					}
				}
			}
		}

	}
}

void DeletePlacedHierarchy(PLACED_HIERARCHY_BEHAV_BLOCK* ph_bhv)
{
	int i;
	if(!ph_bhv) return;
	
	for(i=0;i<ph_bhv->num_sounds;i++)
	{
		if(ph_bhv->sounds[i].activ_no!=SOUND_NOACTIVEINDEX)
		{
			Sound_Stop(ph_bhv->sounds[i].activ_no);
		}
	}

	Dispel_HModel(&ph_bhv->HModelController);
	
}

void PlacedHierarchyStopSequence(PLACED_HIERARCHY_BEHAV_BLOCK* ph_bhv)
{
	int i;
	//stop hierarchy from playing
	ph_bhv->HModelController.Playing=FALSE;
	//and stop all the sounds
	for(i=0;i<ph_bhv->num_sounds;i++)
	{
		if(ph_bhv->sounds[i].activ_no!=SOUND_NOACTIVEINDEX)
		{
			Sound_Stop(ph_bhv->sounds[i].activ_no);
		}
		ph_bhv->sounds[i].playing=0;
	}
}


void SendRequestToPlacedHierarchy(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
	PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv;
	int seq_num;
	GLOBALASSERT(sbptr);
	GLOBALASSERT(sbptr->SBdataptr);
	ph_bhv = (PLACED_HIERARCHY_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((ph_bhv->bhvr_type == I_BehaviourPlacedHierarchy));
	
	seq_num=(extended_data>>7)&0xff;
	if(state)
	{
		PLACED_HIERARCHY_SEQUENCE* new_sequence;
		GLOBALASSERT(seq_num<ph_bhv->num_sequences);

		new_sequence=&ph_bhv->sequences[seq_num];
		GLOBALASSERT(new_sequence->sequence_no!=-1);

		if(new_sequence==ph_bhv->current_seq)
		{
			//restart the current sequence
			ph_bhv->HModelController.Playing=1;
		}
		else
		{
			//stop the current sequence
			PlacedHierarchyStopSequence(ph_bhv);
			//start the new sequence		
			ph_bhv->current_seq=new_sequence;
			InitHModelSequence(&ph_bhv->HModelController,ph_bhv->current_seq->sequence_no,ph_bhv->current_seq->sub_sequence_no,ph_bhv->current_seq->time);
			ph_bhv->HModelController.Playing=1;
			ph_bhv->HModelController.Looped=ph_bhv->current_seq->loop;
		}

	}
	else
	{
		PlacedHierarchyStopSequence(ph_bhv);
	}
	
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"


void LoadStrategy_PlacedHierarchy(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	int i;
	STRATEGYBLOCK* sbPtr;
	PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv;
	char * buffer =(char*) header;

	buffer+=sizeof(*header);

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourPlacedHierarchy) return;
	
	ph_bhv = (PLACED_HIERARCHY_BEHAV_BLOCK*)sbPtr->SBdataptr;
	
	{
		int sequence_index;
		sequence_index = *(int*) buffer;
		buffer+=sizeof(int);
		if(sequence_index>=0 && sequence_index<ph_bhv->num_sequences)
		{
			ph_bhv->current_seq = &ph_bhv->sequences[sequence_index];
		}
	}

	{
		int loaded_num_sounds;

		loaded_num_sounds = *(int*) buffer;
		buffer +=sizeof(int);

		for(i=0;i<loaded_num_sounds && i<ph_bhv->num_sounds;i++)
		{
			int playing;
			playing = *(int*)buffer;
			buffer += sizeof(int);

			ph_bhv->sounds[i].playing = playing;
		}

	}
	
	//load the hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&ph_bhv->HModelController);
		}
	}

	for(i=0;i<ph_bhv->num_sounds;i++)
	{
		Load_SoundState(&ph_bhv->sounds[i].activ_no);
	}

}

void SaveStrategy_PlacedHierarchy(STRATEGYBLOCK* sbPtr)
{
	PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv;
	SAVE_BLOCK_STRATEGY_HEADER* header;
	char* buffer;
	unsigned int size;
	int i;

	ph_bhv = (PLACED_HIERARCHY_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//determine memeory required
	size = sizeof(SAVE_BLOCK_STRATEGY_HEADER)+(2*sizeof(int))+(sizeof(int)*ph_bhv->num_sounds);

	header = (SAVE_BLOCK_STRATEGY_HEADER*) GetPointerForSaveBlock(size);
	buffer = (char*) header;
	buffer += sizeof(*header);

	//fill in the header
	header->type = SaveBlock_Strategy;
	header->size = size;
	header->bhvr_type = I_BehaviourPlacedHierarchy;
	COPY_NAME(header->SBname,sbPtr->SBname);

	{
		int sequence_index = ph_bhv->current_seq-ph_bhv->sequences;
		*(int*)buffer = sequence_index;
		buffer+=sizeof(int);
	}

	*(int*)buffer = ph_bhv->num_sounds;
	buffer+=sizeof(int);
		
	for(i=0;i<ph_bhv->num_sounds;i++)
	{
		int playing = ph_bhv->sounds[i].playing;
		*(int*)buffer = playing;
		buffer+=sizeof(int);
	}

	//save the hierarchy
	SaveHierarchy(&ph_bhv->HModelController);

	for(i=0;i<ph_bhv->num_sounds;i++)
	{
		Save_SoundState(&ph_bhv->sounds[i].activ_no);
	}
	
}
