#define DB_LEVEL 1

#include <stdlib.h>

#include "list_tem.hpp"
#include "chnkload.hpp"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "envchunk.hpp"
#include "obchunk.hpp"
#include "ltchunk.hpp"
#include "avpchunk.hpp"
#include "strachnk.hpp"
#include "sndchunk.hpp"
#include "pvisible.h"
#include "objsetup.hpp"
#include "hierplace.hpp"

#include "bh_gener.h"
#include "bh_swdor.h"
#include "bh_ldoor.h"
#include "bh_plift.h"
#include "bh_pred.h"
#include "bh_fhug.h"
#include "bh_marin.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_alien.h"
#include "bh_xeno.h"
#include "bh_binsw.h"
#include "bh_lnksw.h"
#include "bh_spcl.h"
#include "bh_agun.h"
#include "bh_lift.h"
#include "bh_ltfx.h"
#include "bh_snds.h"
#include "bh_mission.h"
#include "bh_track.h"
#include "bh_fan.h"
#include "bh_light.h"
#include "bh_plachier.h"
#include "bh_cable.h"
#include "bh_deathvol.h"
#include "bh_selfdest.h"
#include "bh_pargen.h"
#include "bh_videoscreen.h"
#include "missions.hpp"	   
#include "track.h"
#include "psndplat.h"

#include "dxlog.h"
#include "mempool.h"
#include "db.h"

#include "pldnet.h"

extern "C" {
#include "3dc.h"
extern MAPSETVDB chnk_playcam_vdb;
extern int GlobalAmbience;
extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
};

static void get_marine_facing_point(VECTORCH& pos,EULER& euler,VECTORCH& facing_point);

//if this number is non-negative , use this value for all random location dierolls
extern "C"
{
int QuantumObjectDieRollOveride=-1;
};

struct BehaviourBlockData
{
	AVP_BEHAVIOUR_TYPE sb_type;
	ObjectID id;
	int shapeindex;
	void *bhdata;
	char* name;

	unsigned int diff_easy :1;
	unsigned int diff_medium :1;
	unsigned int diff_hard :1;
	
	//alternative locations for 'quantum objects'
	int num_locations;
	int location_group;
	VECTORCH* alt_vector;
	EULER* alt_euler;
};

struct LocationGroupDieRoll
{
	int group;
	int roll;
};

static List <BehaviourBlockData*> Behav_List;
static List <LocationGroupDieRoll*> DieRoll_List;

const char * light_set_name = "NORMALLT";

BOOL Xenoborg_Morph_Thingy = 0;
XENO_MORPH_ROOM_TOOLS_TEMPLATE xmrtt;


extern char* Rif_Sound_Directory;

LOADED_SOUND const * GetSoundForMainRif(const char* wav_name)
{
	static char filename[200];
	if(Rif_Sound_Directory)
	{
		sprintf(filename,"%s\\%s",Rif_Sound_Directory,wav_name);
		return GetSound(filename);
	}
	else
	{
		return GetSound(wav_name);
	}

}

void setup_track_sound(Indexed_Sound_Chunk* s_chunk,TRACK_SOUND** ts)
{
	*ts=0;
	if(!s_chunk) return;
	
	
	LOADED_SOUND const * ls=GetSoundForMainRif(s_chunk->wav_name);
	if(ls)
	{
		TRACK_SOUND* sound=(TRACK_SOUND*)PoolAllocateMem(sizeof(TRACK_SOUND));

		sound->sound_loaded=ls;
		sound->inner_range=(unsigned)(s_chunk->inner_range*local_scale);
		sound->outer_range=(unsigned)(s_chunk->outer_range*local_scale);
		sound->pitch=s_chunk->pitch;
		sound->max_volume=s_chunk->max_volume;
		sound->activ_no=SOUND_NOACTIVEINDEX;
		sound->playing=0;

		if(s_chunk->flags & IndexedSoundFlag_Loop)
			sound->loop=1;
		else
			sound->loop=0;

		*ts=sound;
	}
}

void setup_track_sound(Object_Chunk* oc,TRACK_SOUND** start_sound,TRACK_SOUND** mid_sound,TRACK_SOUND** end_sound)
{
	if(start_sound)*start_sound=0;
	if(mid_sound)*mid_sound=0;
	if(end_sound)*end_sound=0;

	db_logf3(("Setting up track sound"));

	List<Chunk*> chlist;
	oc->lookup_child("TRAKSOUN",chlist);
	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Object_Track_Sound_Chunk* otsc=(Object_Track_Sound_Chunk*)chlif();
		TRACK_SOUND** sound_ptr;

		switch(otsc->index)
		{
			case 0:
				sound_ptr=mid_sound;
				break;
			case 1:
				sound_ptr=start_sound;
				break;
			case 2:
				sound_ptr=end_sound;
				break;
		}
		if(!sound_ptr) continue;
		if(*sound_ptr) continue;
		
		
		db_logf3(("Getting %s",otsc->wav_name));
		LOADED_SOUND const * ls=GetSoundForMainRif(otsc->wav_name);
		db_logf3(("Finished getting %s",otsc->wav_name));
		if(ls)
		{
			
			TRACK_SOUND* sound=(TRACK_SOUND*)PoolAllocateMem(sizeof(TRACK_SOUND));

			sound->sound_loaded=ls;
			sound->inner_range=(unsigned)(otsc->inner_range*local_scale);
			sound->outer_range=(unsigned)(otsc->outer_range*local_scale);
			sound->pitch=otsc->pitch;
			sound->max_volume=otsc->max_volume;
			sound->activ_no=SOUND_NOACTIVEINDEX;
			sound->playing=0;

			if(otsc->flags & TrackSoundFlag_Loop)
				sound->loop=1;
			else
				sound->loop=0;

			*sound_ptr=sound;
		}
			
	}	
}

TRACK_CONTROLLER* setup_track_controller(Object_Chunk* oc)
{
	Object_Track_Chunk2* otc=(Object_Track_Chunk2*)oc->lookup_single_child("OBJTRAK2");
	if(!otc) return 0;

	
	
	TRACK_CONTROLLER* tc=(TRACK_CONTROLLER*)PoolAllocateMem(sizeof(TRACK_CONTROLLER));

	tc->sbptr=0;
	tc->playing=0;
	tc->playing_start_sound=0;
	tc->reverse=0;
	tc->loop=0;
	tc->loop_backandforth=0;
	tc->no_rotation=1;
	tc->timer=otc->timer_start;
	tc->current_section=0;
	tc->use_speed_mult=0;
	tc->speed_mult=0;

	GLOBALASSERT(otc->num_sections);

	tc->num_sections=otc->num_sections;

	tc->sections=(TRACK_SECTION_DATA*)PoolAllocateMem(sizeof(TRACK_SECTION_DATA)*otc->num_sections);

	QUAT quat_start;
	quat_start.quatx=(int)-(otc->sections[0].quat_start.x*ONE_FIXED);
	quat_start.quaty=(int)-(otc->sections[0].quat_start.x*ONE_FIXED);
	quat_start.quatz=(int)-(otc->sections[0].quat_start.x*ONE_FIXED);
	quat_start.quatw=(int)(otc->sections[0].quat_start.x*ONE_FIXED);

	for(int i=0;i<tc->num_sections;i++)
	{
		TRACK_SECTION_DATA* tsd=&tc->sections[i];
		ChunkTrackSection* cts=&otc->sections[i];

		tsd->quat_start.quatx=(int)-(cts->quat_start.x*ONE_FIXED);
		tsd->quat_start.quaty=(int)-(cts->quat_start.y*ONE_FIXED);
		tsd->quat_start.quatz=(int)-(cts->quat_start.z*ONE_FIXED);
		tsd->quat_start.quatw=(int)(cts->quat_start.w*ONE_FIXED);

		tsd->quat_end.quatx=(int)-(cts->quat_end.x*ONE_FIXED);
		tsd->quat_end.quaty=(int)-(cts->quat_end.y*ONE_FIXED);
		tsd->quat_end.quatz=(int)-(cts->quat_end.z*ONE_FIXED);
		tsd->quat_end.quatw=(int)(cts->quat_end.w*ONE_FIXED);

		if(tsd->quat_start.quatx!=quat_start.quatx||
		   tsd->quat_start.quaty!=quat_start.quaty||	
		   tsd->quat_start.quatz!=quat_start.quatz||	
		   tsd->quat_start.quatw!=quat_start.quatw)
		{
			tc->no_rotation=0;
		}	
		if(tsd->quat_end.quatx!=quat_start.quatx||
		   tsd->quat_end.quaty!=quat_start.quaty||	
		   tsd->quat_end.quatz!=quat_start.quatz||	
		   tsd->quat_end.quatw!=quat_start.quatw)
		{
			tc->no_rotation=0;
		}	


		
		tsd->pivot_start=cts->pivot_start*local_scale;
		tsd->pivot_travel=(cts->pivot_end-cts->pivot_start)*local_scale;
		tsd->object_offset=cts->object_offset*local_scale;

		tsd->time_for_section=cts->time_for_section;
	}

   	if(otc->flags & TrackFlag_Loop)
   		tc->loop=1;
   	else if(otc->flags & TrackFlag_LoopBackAndForth)
   		tc->loop_backandforth=1;


	if(otc->flags & TrackFlag_PlayingAtStart)
		tc->playing=1;
		
	
	setup_track_sound(oc,&tc->start_sound,&tc->sound,&tc->end_sound);
	
	if(tc->sound)
	{
		tc->sound->playing=tc->playing;
	}

	/* KJL 14:29:09 24/03/98 - addition for smooth tracks */
	if(otc->flags & TrackFlag_UseTrackSmoothing && tc->num_sections>=3)
		tc->use_smoothing = 1;
	else
		tc->use_smoothing = 0;
	
	Preprocess_Track_Controller(tc);

	//set the initial state entries
	tc->initial_state_timer=tc->timer; 
	tc->initial_state_playing=tc->playing; 
	tc->initial_state_reverse=tc->reverse; 

	
	return tc;

}

static int get_location_group_die_roll(int group)
{
	//see if a vaule has already been rolled for this group
	for(LIF<LocationGroupDieRoll*> dlif(&DieRoll_List);!dlif.done();dlif.next())
	{
		if(dlif()->group==group)
		{
			//found the group , so return its roll
			return dlif()->roll;
		}
	}
	//need to roll a new value for this group
	LocationGroupDieRoll* loc_group=new LocationGroupDieRoll;
	loc_group->group=group;
	loc_group->roll=FastRandom() & 0xffff;
	DieRoll_List.add_entry(loc_group);

	return loc_group->roll;
}

static void clear_location_group_die_roll_list()
{
	while(DieRoll_List.size())
	{
		delete DieRoll_List.first_entry();
		DieRoll_List.delete_first_entry();
	}
}



static void select_alternate_location(BehaviourBlockData* bbd)
{
	GLOBALASSERT (bbd);
	if(bbd->num_locations<2) return;

	int dieroll=0;
	if(QuantumObjectDieRollOveride>=0)
	{
		dieroll=QuantumObjectDieRollOveride;
	}
	else
	{
		if(bbd->location_group)
		{
			dieroll=get_location_group_die_roll(bbd->location_group);
		}
		else
		{
			dieroll=FastRandom() & 0xffff;
		}
	}

	dieroll=dieroll%bbd->num_locations;

	VECTORCH* chosen_pos=&bbd->alt_vector[dieroll];
	EULER* chosen_euler=&bbd->alt_euler[dieroll];

	switch(bbd->sb_type)
	{
		case I_BehaviourInanimateObject :
		{
			TOOLS_DATA_INANIMATEOBJECT* tdio=(TOOLS_DATA_INANIMATEOBJECT*) bbd->bhdata;
			tdio->position=*chosen_pos;
			tdio->orientation=*chosen_euler;
			break;
		}

		case I_BehaviourVideoScreen :
		{
			TOOLS_DATA_VIDEO_SCREEN* tdvs=(TOOLS_DATA_VIDEO_SCREEN*) bbd->bhdata;
			tdvs->position=*chosen_pos;
			tdvs->orientation=*chosen_euler;
			break;
		}
		

		case I_BehaviourPlacedSound :
		{
			SOUND_TOOLS_TEMPLATE* stt=(SOUND_TOOLS_TEMPLATE*) bbd->bhdata;
			stt->position = *chosen_pos;
			break;
		}
		
		case I_BehaviourGenerator :
		{
			GENERATOR_BLOCK* tdg =(GENERATOR_BLOCK*) bbd->bhdata;
			tdg->Position=*chosen_pos;
			break;
		}

		case I_BehaviourAlien :
		{
			TOOLS_DATA_ALIEN* tda=(TOOLS_DATA_ALIEN*) bbd->bhdata;
			tda->position=*chosen_pos;
			tda->starteuler=*chosen_euler;
			break;
		}

		case I_BehaviourMarine :
		{
			TOOLS_DATA_MARINE* tdm=(TOOLS_DATA_MARINE*) bbd->bhdata;
			tdm->position=*chosen_pos;
			get_marine_facing_point(*chosen_pos,*chosen_euler,tdm->facing_point);
			break;
		}

		case I_BehaviourDormantPredator :
		case I_BehaviourPredator :
		{
			TOOLS_DATA_PREDATOR* tdp=(TOOLS_DATA_PREDATOR*) bbd->bhdata;
			tdp->position=*chosen_pos;
			break;
		}
	
		case I_BehaviourQueenAlien :
		{
			TOOLS_DATA_QUEEN* tdq=(TOOLS_DATA_QUEEN*) bbd->bhdata;
			tdq->position=*chosen_pos;
			break;
		}
		
		case I_BehaviourFaceHugger :
		{
			TOOLS_DATA_FACEHUGGER* tdf=(TOOLS_DATA_FACEHUGGER*) bbd->bhdata;
			tdf->position=*chosen_pos;
			break;
		}

		case I_BehaviourAutoGun :
		{
			AUTOGUN_TOOLS_TEMPLATE* tda=(AUTOGUN_TOOLS_TEMPLATE*) bbd->bhdata;
			tda->position=*chosen_pos;
			tda->orientation=*chosen_euler;
			break;
		}

		case I_BehaviourXenoborg :
		{
			TOOLS_DATA_XENO* tdx=(TOOLS_DATA_XENO*) bbd->bhdata;
			tdx->position=*chosen_pos;
			tdx->starteuler=*chosen_euler;
			break;
		}

		case I_BehaviourParticleGenerator :
		{
			PARTICLE_GENERATOR_TOOLS_TEMPLATE* part_temp=(PARTICLE_GENERATOR_TOOLS_TEMPLATE*) bbd->bhdata;
			part_temp->position=*chosen_pos;
			MATRIXCH m;
			CreateEulerMatrix(chosen_euler,&m);
			TransposeMatrixCH(&m);
			
			part_temp->orientation=m;
			break;
		}
		
		default:
			break;
	}
}

void create_strategies_from_list ()
{
	for(LIF<BehaviourBlockData*> blif(&Behav_List);!blif.done();blif.next())
	{
		BehaviourBlockData* bbd=blif();

		//is the strategy required by the current difficulty level?
		switch(AvP.Difficulty)
		{
			case I_Easy :
				if(!bbd->diff_easy) continue;
				break;
			case I_Medium :
				if(!bbd->diff_medium) continue;
				break;
			case I_Hard :
			case I_Impossible :
				if(!bbd->diff_hard) continue;
				break;
			default:
				break;
		}

		if(AvP.Network != I_No_Network && !AvP.NetworkAIServer)
		{
			//if this is a network game , and we aren't the	ai host , then we shouldn't set up
			//any aliens
			if(bbd->sb_type==I_BehaviourAlien) continue;
		}

		//if this is a quantum object , need to select location
		select_alternate_location(bbd);
		
		STRATEGYBLOCK * sbPtr;
		sbPtr = CreateActiveStrategyBlock();

		if(!sbPtr)
		{
			GLOBALASSERT(0=="Run out of strategy blocks");
		}

		InitialiseSBValues(sbPtr);
		sbPtr->I_SBtype = bbd->sb_type;
		sbPtr->shapeIndex=bbd->shapeindex;
		sbPtr->SBflags.preserve_until_end_of_level=1;
		if(bbd->name)
		{
			sbPtr->name=bbd->name;
		}
		*(ObjectID*)&sbPtr->SBname[0]=bbd->id;
				
		EnableBehaviourType(sbPtr, bbd->sb_type, bbd->bhdata );
		
	}
	//get rid of stored list of rolls for quantum object groups
	clear_location_group_die_roll_list();
}

void AddToBehaviourList(const char* name,const ObjectID &ID,AVP_BEHAVIOUR_TYPE sb_type,void* bhdata,int shapeindex=-1,Object_Alternate_Locations_Chunk* loc_chunk=0,int flags=0)
{
	BehaviourBlockData* bbd=(BehaviourBlockData*) PoolAllocateMem(sizeof(BehaviourBlockData));
	bbd->sb_type=sb_type;
	bbd->bhdata=bhdata;
	bbd->id=ID;
	bbd->shapeindex=shapeindex;

	bbd->diff_easy=1;
	bbd->diff_medium=1;
	bbd->diff_hard=1;

	if(name)
	{
		bbd->name=(char*) PoolAllocateMem(strlen(name)+1);
		strcpy(bbd->name,name);	
	}
	else
	{
		bbd->name=0;
	}
	
	if(loc_chunk && loc_chunk->num_locations>=2)
	{
		bbd->num_locations=loc_chunk->num_locations;
		bbd->location_group=loc_chunk->group;
		bbd->alt_vector=(VECTORCH*) PoolAllocateMem(bbd->num_locations*sizeof(VECTORCH));
		bbd->alt_euler=(EULER*) PoolAllocateMem(bbd->num_locations*sizeof(EULER));
		for(int i=0;i<loc_chunk->num_locations;i++)
		{
			bbd->alt_vector[i]=loc_chunk->locations[i].position*local_scale;
			
			QUAT q;

			q.quatx = (int) -(loc_chunk->locations[i].orientation.x*ONE_FIXED);
			q.quaty = (int) -(loc_chunk->locations[i].orientation.y*ONE_FIXED);
			q.quatz = (int) -(loc_chunk->locations[i].orientation.z*ONE_FIXED);
			q.quatw = (int)  (loc_chunk->locations[i].orientation.w*ONE_FIXED);


			MATRIXCH m;

			QuatToMat (&q, &m);

			MatrixToEuler(&m, &bbd->alt_euler[i]);
		}
	}
	else
	{
		bbd->num_locations=0;
		bbd->location_group=0;
		bbd->alt_vector=0;
		bbd->alt_euler=0;
	}
	
	if(flags)
	{
		//see which difficulty levels this object is used for
		bbd->diff_easy=(flags & OBJECT_FLAG_NOTDIFFICULTY1)==0;
		bbd->diff_medium=(flags & OBJECT_FLAG_NOTDIFFICULTY2)==0;
		bbd->diff_hard=(flags & OBJECT_FLAG_NOTDIFFICULTY3)==0;
	}

	Behav_List.add_entry(bbd);
}

void deallocate_behaviour_list()
{
	while(Behav_List.size())
	{
		BehaviourBlockData* bbd=Behav_List.first_entry();

		switch(bbd->sb_type)
		{
			case I_BehaviourMissionComplete :
				{
					MISSION_COMPLETE_TOOLS_TEMPLATE* mctt=(MISSION_COMPLETE_TOOLS_TEMPLATE*)bbd->bhdata;
					if(mctt->mission_objective_ptr)
					{
						delete (MissionObjective *)(mctt->mission_objective_ptr);
					}
					
				}
				break;
			#if !NEW_DEALLOCATION_ORDER
			case I_BehaviourLinkSwitch:
				{
					LINK_SWITCH_TOOLS_TEMPLATE* lstt=(LINK_SWITCH_TOOLS_TEMPLATE*)bbd->bhdata;
					if(lstt->track)
					{
						Deallocate_Track(lstt->track);
					}
					#if !USE_LEVEL_MEMORY_POOL
					DeallocateMem (lstt->targets);
					if(lstt->switchIDs)
					{
						DeallocateMem (lstt->switchIDs);
					}
					#endif
				}
				break;

			case I_BehaviourBinarySwitch :
				{
					BIN_SWITCH_TOOLS_TEMPLATE* bstt=(BIN_SWITCH_TOOLS_TEMPLATE*)bbd->bhdata;
					#if !USE_LEVEL_MEMORY_POOL
					DeallocateMem(bstt->target_names);
					DeallocateMem(bstt->request_messages);	
					#endif
					
					if(bstt->track)
					{
						Deallocate_Track(bstt->track);
					}
				}
				break;

			case I_BehaviourPlacedSound :
				{
					SOUND_TOOLS_TEMPLATE* stt=(SOUND_TOOLS_TEMPLATE*)bbd->bhdata;
					#if !USE_LEVEL_MEMORY_POOL
					DeallocateMem(stt->sound_name);
					#endif
					if(stt->sound_loaded)
					{
						LoseSound(stt->sound_loaded);
					}
				}
				break;

			case I_BehaviourTrackObject :
				{
					TRACK_OBJECT_TOOLS_TEMPLATE* tott=(TRACK_OBJECT_TOOLS_TEMPLATE*)bbd->bhdata;
					if(tott->track)
					{
						Deallocate_Track(tott->track);
					}	
					#if !USE_LEVEL_MEMORY_POOL
					if(tott->special_track_points)
					{
						int i;
						for(i=0;i<tott->num_special_track_points;i++)
						{
							if(tott->special_track_points[i].targets)
								DeallocateMem(tott->special_track_points[i].targets);
						}					
						DeallocateMem(tott->special_track_points);
					}
					#endif
				}
				break;

			case I_BehaviourFan :
				{
					FAN_TOOLS_TEMPLATE* ftt=(FAN_TOOLS_TEMPLATE*)bbd->bhdata;
					if(ftt->track)
					{
						Deallocate_Track(ftt->track);
					}
				}
				break;


			case I_BehaviourPlatform :
				{
					PLATFORMLIFT_TOOLS_TEMPLATE* pltt=(PLATFORMLIFT_TOOLS_TEMPLATE*)bbd->bhdata;
					if(pltt->start_sound)
					{
						Deallocate_Track_Sound(pltt->start_sound);
					}
					if(pltt->sound)
					{
						Deallocate_Track_Sound(pltt->sound);
					}
					if(pltt->end_sound)
					{
						Deallocate_Track_Sound(pltt->end_sound);
					}
				}
				break;

			case I_BehaviourParticleGenerator :
				{
					PARTICLE_GENERATOR_TOOLS_TEMPLATE* pgtt=(PARTICLE_GENERATOR_TOOLS_TEMPLATE*)bbd->bhdata;
					if(pgtt->sound)
					{
						Deallocate_Track_Sound(pgtt->sound);
					}
				}
				break;


			#if !USE_LEVEL_MEMORY_POOL
			case I_BehaviourPlacedLight :
				{
					TOOLS_DATA_PLACEDLIGHT * pltt=(TOOLS_DATA_PLACEDLIGHT*)bbd->bhdata;
					if(pltt->light)
					{
						DeallocateMem(pltt->light);
					}
				}
				break;
			#endif

			case I_BehaviourPlacedHierarchy :
				{
					PLACED_HIERARCHY_TOOLS_TEMPLATE* phtt=(PLACED_HIERARCHY_TOOLS_TEMPLATE*)bbd->bhdata;
					int i;
					#if !USE_LEVEL_MEMORY_POOL
					if(phtt->num_sequences)
					{
						for(i=0;i<phtt->num_sequences;i++)
						{
							if(phtt->sequences[i].sound_times)
							{
								DeallocateMem(phtt->sequences[i].sound_times);
							}
						}
						DeallocateMem(phtt->sequences);
					}
					#endif
					for(i=0;i<phtt->num_sounds;i++)
					{
						if(phtt->sounds[i].sound_loaded)
						{
							LoseSound(phtt->sounds[i].sound_loaded);
						}
					}
					#if !USE_LEVEL_MEMORY_POOL
					if(phtt->sounds)
					{
						DeallocateMem(phtt->sounds);
					}

					if(phtt->special_track_points)
					{
						int i;
						for(i=0;i<phtt->num_special_track_points;i++)
						{
							if(phtt->special_track_points[i].targets)
								DeallocateMem(phtt->special_track_points[i].targets);
						}					
						DeallocateMem(phtt->special_track_points);
					}
					#endif

				}
				#endif //!NEW_DEALLOCATION_ORDER
				
			default:
				break;
		}

		#if !USE_LEVEL_MEMORY_POOL
		if(bbd->alt_vector) DeallocateMem(bbd->alt_vector);
		if(bbd->alt_euler) DeallocateMem(bbd->alt_euler);
		if(bbd->name) DeallocateMem(bbd->name);
		DeallocateMem(bbd->bhdata);
		DeallocateMem(bbd);
		#endif
		Behav_List.delete_first_entry();
	}
}

static void add_simple_animation (Object_Chunk * ob, int list_pos, MODULE * mod)
{
	SIMPLE_ANIM_TOOLS_TEMPLATE * satt =(SIMPLE_ANIM_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(SIMPLE_ANIM_TOOLS_TEMPLATE));
	
	satt->shape_num = list_pos;
	if (mod)
	{
		*((int *)satt->my_module.mref_name) = *((int *)mod->m_name);
	}
	else
	{
		*((int *)satt->my_module.mref_name) = 0;
	}
	*((ObjectID *)satt->nameID) = ob->object_data.ID;

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourSimpleAnimation, (void*)satt,list_pos);
}


static void add_default_object(Object_Chunk * ob, int list_pos)
{

	TOOLS_DATA_INANIMATEOBJECT* tdio =(TOOLS_DATA_INANIMATEOBJECT*) PoolAllocateMem(sizeof(TOOLS_DATA_INANIMATEOBJECT));
	
	tdio->position.vx = (int)(ob->object_data.location.x * local_scale);
	tdio->position.vy = (int)(ob->object_data.location.y * local_scale);
	tdio->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &tdio->orientation);
	
	tdio->triggering_event=0;
	tdio->typeId = IOT_Furniture;
	tdio->subType = 0;
	tdio->shapeIndex = list_pos;
	tdio->mass=5;
	tdio->integrity=2;
	tdio->explosionType=0;
	
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourInanimateObject, (void*)tdio,list_pos,0,ob->get_header()->flags);
}

static void add_trackobject(Object_Chunk* ob, int list_pos,AVP_Strategy_Chunk* asc)
{
	
	TRACK_CONTROLLER* track=setup_track_controller(ob);
	if(!track)
	{
		LOGDXFMT(("%s has no track\n",ob->object_data.o_name));	
	 	GLOBALASSERT(track);
	}
	
	TrackStrategy* ts=(TrackStrategy*) asc->Strategy;
				
	TRACK_OBJECT_TOOLS_TEMPLATE* tott=(TRACK_OBJECT_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(TRACK_OBJECT_TOOLS_TEMPLATE));
	
	*(ObjectID*)&tott->nameID[0]=ob->object_data.ID;

	tott->position.vx = (int)(ob->object_data.location.x * local_scale);
	tott->position.vy = (int)(ob->object_data.location.y * local_scale);
	tott->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &tott->orientation);
	
	tott->shape_num = list_pos;
	 		
	tott->track=track;

	tott->num_special_track_points=ts->num_point_effects;
	if(ts->num_point_effects)
	{
		tott->special_track_points=(SPECIAL_TRACK_POINT*)PoolAllocateMem(sizeof(SPECIAL_TRACK_POINT)*ts->num_point_effects);
		for(int i=0;i<ts->num_point_effects;i++)
		{
			SPECIAL_TRACK_POINT* stp=&tott->special_track_points[i];
			TrackPointEffect* tpe=ts->point_effects[i];

			stp->track_point_no=tpe->point_no;
			stp->num_targets=tpe->num_targets;

			if(stp->num_targets)
			{
				stp->targets=(TRACK_POINT_TARGET*)PoolAllocateMem(sizeof(TRACK_POINT_TARGET)*stp->num_targets);
				for(int j=0;j<stp->num_targets;j++)
				{
					stp->targets[j].request=tpe->targets[j].request;
					stp->targets[j].flags=tpe->targets[j].flags;
					*(ObjectID*)&stp->targets[j].target_name[0]=tpe->targets[j].targetID;
				}
			
			}
			else
				stp->targets=0;
		}
	}
	else
		tott->special_track_points=0;

	if(ts->StrategyType==StratTrackDestruct)
	{
		TrackDestructStrategy* tds=(TrackDestructStrategy*)ts;
		tott->integrity=tds->integrity;
		tott->destruct_target_request=tds->target_request;
		*(ObjectID*)tott->destruct_target_ID=tds->targetID;
	}
	else
	{
		tott->integrity=21;
		tott->destruct_target_request=0;
		ObjectID ID0={0,0};
		*(ObjectID*)tott->destruct_target_ID=ID0;
	}
		
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourTrackObject, (void*)tott,list_pos);

}

void add_placed_hierarchy(Placed_Hierarchy_Chunk* phc,const char* fname,const char* hname)
{
	int i;
	PLACED_HIERARCHY_TOOLS_TEMPLATE* phtt=(PLACED_HIERARCHY_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(PLACED_HIERARCHY_TOOLS_TEMPLATE));
	
	//first setup the sounds
	phtt->sounds=0;
	
	List<Chunk*> chlist;
	phc->lookup_child("INDSOUND",chlist);
	phtt->num_sounds=0;
	
	//find the highest index
	LIF<Chunk*> chlif(&chlist);
	for(; !chlif.done(); chlif.next())
	{
		Indexed_Sound_Chunk* isc=(Indexed_Sound_Chunk*)chlif();
		phtt->num_sounds=max(phtt->num_sounds,isc->index+1);
	}

	if(phtt->num_sounds)
	{
		phtt->sounds=(PLACED_HIERARCHY_SOUND*)PoolAllocateMem(sizeof(PLACED_HIERARCHY_SOUND)*phtt->num_sounds);
		for(i=0;i<phtt->num_sounds;i++)
		{
			phtt->sounds[i].sound_loaded=0;
			phtt->sounds[i].activ_no=SOUND_NOACTIVEINDEX;				
		}
		
		for(chlif.restart();!chlif.done();chlif.next())
		{
			Indexed_Sound_Chunk* isc=(Indexed_Sound_Chunk*)chlif();
			PLACED_HIERARCHY_SOUND* phs=&phtt->sounds[isc->index];

			phs->inner_range=(unsigned)(isc->inner_range*local_scale);
			phs->outer_range=(unsigned)(isc->outer_range*local_scale);
			phs->pitch=isc->pitch;
			phs->max_volume=isc->max_volume;
			phs->playing=0;
			phs->loop=((isc->flags & IndexedSoundFlag_Loop)!=0);
			phs->sound_loaded=GetSoundForMainRif(isc->wav_name);
		}
	}

	//now setup the sequences
	phc->lookup_child("PLHISEQU",chlist);
	phtt->num_sequences=0;
	//find the highest sequence index
	for(chlif.restart();!chlif.done();chlif.next())
	{
		Placed_Hierarchy_Sequence_Chunk* phsc=(Placed_Hierarchy_Sequence_Chunk*)chlif();
		phtt->num_sequences=max(phtt->num_sequences,phsc->index+1);
	}

	GLOBALASSERT(phtt->num_sequences);

	phtt->sequences=(PLACED_HIERARCHY_SEQUENCE*)PoolAllocateMem(sizeof(PLACED_HIERARCHY_SEQUENCE)*phtt->num_sequences);
	for(i=0;i<phtt->num_sequences;i++)
	{
		phtt->sequences[i].sequence_no=-1;
	}
	phtt->first_sequence=&phtt->sequences[0];
	phtt->playing=0;

	for(chlif.restart();!chlif.done();chlif.next())
	{
		Placed_Hierarchy_Sequence_Chunk* phsc=(Placed_Hierarchy_Sequence_Chunk*)chlif();
		PLACED_HIERARCHY_SEQUENCE* ph_seq=&phtt->sequences[phsc->index];

		ph_seq->sequence_no=phsc->sequence;
		ph_seq->sub_sequence_no=phsc->sub_sequence;
		ph_seq->time=(int)(((float)phsc->time*(float)ONE_FIXED)/1000.0);
		ph_seq->loop=((phsc->flags & HierarchySequenceFlag_Loop)!=0);

		if(phsc->flags & HierarchySequenceFlag_InitialSequence)
		{
			phtt->first_sequence=ph_seq;
			if(phsc->flags & HierarchySequenceFlag_Playing)
			{
				phtt->playing=0;	
			}
		}

		ph_seq->num_sound_times=phsc->sound_list_size;
		if(ph_seq->num_sound_times)
		{
			ph_seq->sound_times=(PLACED_HIERARCHY_SOUND_TIMES*)PoolAllocateMem(sizeof(PLACED_HIERARCHY_SOUND_TIMES)*ph_seq->num_sound_times);
			for(i=0;i<ph_seq->num_sound_times;i++)
			{
				ph_seq->sound_times[i].start_time=phsc->sound_list[i].start_time;	
				ph_seq->sound_times[i].end_time=phsc->sound_list[i].end_time;	
				
				int sound_index=phsc->sound_list[i].sound_index;
				if(sound_index>=0 && sound_index<phtt->num_sounds)
				{
					ph_seq->sound_times[i].sound=&phtt->sounds[sound_index];
				}
				else
				{
					ph_seq->sound_times[i].sound=0;
				}

			}
		}
		else
		{
			ph_seq->sound_times=0;
		}
		
	}

	phtt->num_special_track_points=0;
	phtt->special_track_points=0;
	AVP_Strategy_Chunk* asc=(AVP_Strategy_Chunk*) phc->lookup_single_child("AVPSTRAT");
	if(asc && asc->Strategy)
	{
		GLOBALASSERT(asc->Strategy->StrategyType==StratHierarchy);
		HierarchyStrategy* hs=(HierarchyStrategy*) asc->Strategy;
	
		phtt->num_special_track_points=hs->num_point_effects;
		if(hs->num_point_effects)
		{
			phtt->special_track_points=(SPECIAL_TRACK_POINT*)PoolAllocateMem(sizeof(SPECIAL_TRACK_POINT)*hs->num_point_effects);
			for(int i=0;i<hs->num_point_effects;i++)
			{
				SPECIAL_TRACK_POINT* stp=&phtt->special_track_points[i];
				TrackPointEffect* tpe=hs->point_effects[i];

				stp->track_point_no=1<<tpe->point_no;
				stp->num_targets=tpe->num_targets;

				if(stp->num_targets)
				{
					stp->targets=(TRACK_POINT_TARGET*)PoolAllocateMem(sizeof(TRACK_POINT_TARGET)*stp->num_targets);
					for(int j=0;j<stp->num_targets;j++)
					{
						stp->targets[j].request=tpe->targets[j].request;
						stp->targets[j].flags=tpe->targets[j].flags;
						*(ObjectID*)&stp->targets[j].target_name[0]=tpe->targets[j].targetID;
					}
				
				}
				else
					stp->targets=0;
			}
		}
		else
			phtt->special_track_points=0;

	}
	
	Placed_Hierarchy_Data_Chunk* data=phc->get_data_chunk();
	

	*(ObjectID*) &phtt->nameID[0]=data->id;

	phtt->position.vx = (int)(data->location.x * local_scale);
	phtt->position.vy = (int)(data->location.y * local_scale);
	phtt->position.vz = (int)(data->location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(data->orientation.x*ONE_FIXED);
	q.quaty = (int) -(data->orientation.y*ONE_FIXED);
	q.quatz = (int) -(data->orientation.z*ONE_FIXED);
	q.quatw = (int) (data->orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &phtt->orientation);

	phtt->hier_name=hname;
	phtt->file_name=fname;

	AddToBehaviourList(0,data->id, I_BehaviourPlacedHierarchy, (void *) phtt,0);
}


static void GetFanWindDirection(QUAT *q1, QUAT *q2, VECTORCH* dir)

{

	//calulate q1 inverse * q2 , and then take axis of rotation
	int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
	int S, T;

	temp1 = MUL_FIXED((q1->quatz - q1->quaty), (q2->quaty - q2->quatz));
	temp2 = MUL_FIXED((-q1->quatw + q1->quatx), (q2->quatw + q2->quatx));
	temp3 = MUL_FIXED((-q1->quatw - q1->quatx), (q2->quaty + q2->quatz));
	temp4 = MUL_FIXED((q1->quatz + q1->quaty), (q2->quatw - q2->quatx));
	temp5 = MUL_FIXED((q1->quatz - q1->quatx), (q2->quatx - q2->quaty));
	temp6 = MUL_FIXED((q1->quatz + q1->quatx), (q2->quatx + q2->quaty));
	temp7 = MUL_FIXED((-q1->quatw + q1->quaty), (q2->quatw - q2->quatz));
	temp8 = MUL_FIXED((-q1->quatw - q1->quaty), (q2->quatw + q2->quatz));

	S = temp6 + temp7 + temp8;

	T = (temp5 + S) / 2;

	dir->vx = temp2 + T - S;
	dir->vy = temp3 + T - temp8;
	dir->vz = temp4 + T - temp7;

	if(temp1 + T - temp6 > 0)
	{
		dir->vx=-dir->vx;
		dir->vy=-dir->vy;
		dir->vz=-dir->vz;
	}

	Normalise(dir);

}

static void add_fan(Object_Chunk* ob, int list_pos,AVP_Strategy_Chunk* asc)
{
	
	TRACK_CONTROLLER* track=setup_track_controller(ob);
	GLOBALASSERT(track);
	
	FanStrategy* fs=(FanStrategy*) asc->Strategy;
				
	FAN_TOOLS_TEMPLATE* ftt=(FAN_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(FAN_TOOLS_TEMPLATE));
	
	*(ObjectID*)&ftt->nameID[0]=ob->object_data.ID;

	ftt->position.vx = (int)(ob->object_data.location.x * local_scale);
	ftt->position.vy = (int)(ob->object_data.location.y * local_scale);
	ftt->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &ftt->orientation);
	
	ftt->shape_num = list_pos;
	ftt->track=track;
	ftt->speed_up_mult=DIV_FIXED(ONE_FIXED,fs->speed_up_time); 		
	ftt->slow_down_mult=DIV_FIXED(ONE_FIXED,fs->slow_down_time); 	
	
	GetFanWindDirection(&track->sections[0].quat_start,&track->sections[0].quat_end,&ftt->fan_wind_direction);

	if(fs->fan_wind_strength>=-1000 && fs->fan_wind_strength<=1000)
		ftt->fan_wind_strength=(fs->fan_wind_strength*ONE_FIXED)/100;
	else
		ftt->fan_wind_strength=ONE_FIXED;
	//if the wind strength is negative , then the wind goes in the opposite direction
	if(ftt->fan_wind_strength<0)
	{
		ftt->fan_wind_strength=-ftt->fan_wind_strength;
		ftt->fan_wind_direction.vx=-ftt->fan_wind_direction.vx;
		ftt->fan_wind_direction.vy=-ftt->fan_wind_direction.vy;
		ftt->fan_wind_direction.vz=-ftt->fan_wind_direction.vz;
	}
		
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourFan, (void*)ftt,list_pos,0,ob->get_header()->flags);
}


static void add_linkswitch(const char* name,AVP_Strategy_Chunk* asc,const ObjectID& ID,Object_Chunk* oc=0,int list_pos=-1)
{

	LinkSwitchStrategy * lss = (LinkSwitchStrategy *)asc->Strategy;

	LINK_SWITCH_TOOLS_TEMPLATE* lstt=(LINK_SWITCH_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(LINK_SWITCH_TOOLS_TEMPLATE));
	
	lstt->trigger_volume_min.vx=lstt->trigger_volume_max.vx=0;
	lstt->trigger_volume_min.vy=lstt->trigger_volume_max.vy=0;
	lstt->trigger_volume_min.vz=lstt->trigger_volume_max.vz=0;
	lstt->switch_flags=0;
	
	if(oc) //switch has a shape
	{
		lstt->position.vx = (int)(oc->object_data.location.x * local_scale);
		lstt->position.vy = (int)(oc->object_data.location.y * local_scale);
		lstt->position.vz = (int)(oc->object_data.location.z * local_scale);
	

		QUAT q;

		q.quatx = (int) -(oc->object_data.orientation.x*ONE_FIXED);
		q.quaty = (int) -(oc->object_data.orientation.y*ONE_FIXED);
		q.quatz = (int) -(oc->object_data.orientation.z*ONE_FIXED);
		q.quatw = (int) (oc->object_data.orientation.w*ONE_FIXED);


		MATRIXCH m;

		QuatToMat (&q, &m);

		MatrixToEuler(&m, &lstt->orientation);

		lstt->track=setup_track_controller(oc);
		if(lstt->track)
		{
			lstt->track->loop=FALSE;
			lstt->track->loop_backandforth=FALSE;
			lstt->track->playing=FALSE;
			lstt->track->initial_state_playing=FALSE;
			lstt->track->timer=0;
			lstt->track->initial_state_timer=0;
		}
		lstt->switch_always_on=0;	
	}
	else //switch has no shape
	{
		lstt->position.vx = 0;
		lstt->position.vy = 0;
		lstt->position.vz = 0;
	
		lstt->track=0;

		lstt->orientation.EulerX=lstt->orientation.EulerY=lstt->orientation.EulerZ=0;
		lstt->switch_always_on=1;	
	}
	
	
	lstt->rest_state = ((lss->flags & BinSwitchFlag_StartsOn)!=0);
	lstt->mode = lss->Mode;
	lstt->time_for_reset = lss->Time;
	lstt->security_clearance = lss->Security;
	lstt->shape_num = list_pos;
	*((ObjectID *)lstt->nameID) =ID;

	
	lstt->switch_off_message_same = ((lss->flags & BinSwitchFlag_OffMessageSame)!=0);
	lstt->switch_off_message_none = ((lss->flags & BinSwitchFlag_OffMessageNone)!=0);
	
	LINK_SWITCH_TARGET* ls_target=(LINK_SWITCH_TARGET*) PoolAllocateMem (sizeof(LINK_SWITCH_TARGET));

	*(ObjectID*)ls_target->name=lss->Target.ID;
	ls_target->request_message=1|lss->Target.request;
	ls_target->sbptr=0;

	lstt->num_targets=1;
	lstt->targets=ls_target;

	if(lss->NumLinks)
	  		lstt->switchIDs=(SBNAMEBLOCK*) PoolAllocateMem(lss->NumLinks*sizeof(SBNAMEBLOCK));
	else
		lstt->switchIDs=0;
	
	lstt->num_linked_switches=lss->NumLinks;
	for (int i=0; i<lss->NumLinks; i++)
	{
		*((ObjectID *)lstt->switchIDs[i].name) = lss->LinkedSwitches[i];
	}
	
	AddToBehaviourList(name,ID, I_BehaviourLinkSwitch, (void*)lstt,list_pos);
	
}
static void add_multitarget_linkswitch(const char* name,AVP_Strategy_Chunk* asc,const ObjectID& ID,Object_Chunk* oc=0,int list_pos=-1)
{
	MultiSwitchStrategy * mss = (MultiSwitchStrategy *)asc->Strategy;

	LINK_SWITCH_TOOLS_TEMPLATE* lstt=(LINK_SWITCH_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(LINK_SWITCH_TOOLS_TEMPLATE));
	
	if(oc) //switch has a shape
	{
		lstt->position.vx = (int)(oc->object_data.location.x * local_scale);
		lstt->position.vy = (int)(oc->object_data.location.y * local_scale);
		lstt->position.vz = (int)(oc->object_data.location.z * local_scale);
	

		QUAT q;

		q.quatx = (int) -(oc->object_data.orientation.x*ONE_FIXED);
		q.quaty = (int) -(oc->object_data.orientation.y*ONE_FIXED);
		q.quatz = (int) -(oc->object_data.orientation.z*ONE_FIXED);
		q.quatw = (int) (oc->object_data.orientation.w*ONE_FIXED);


		MATRIXCH m;

		QuatToMat (&q, &m);

		MatrixToEuler(&m, &lstt->orientation);

		lstt->track=setup_track_controller(oc);
		if(lstt->track)
		{
			lstt->track->loop=FALSE;
			lstt->track->loop_backandforth=FALSE;
			lstt->track->playing=FALSE;
			lstt->track->initial_state_playing=FALSE;
			lstt->track->timer=0;
			lstt->track->initial_state_timer=0;
		}
	}
	else //switch has no shape
	{
		lstt->position.vx = 0;
		lstt->position.vy = 0;
		lstt->position.vz = 0;
	
		lstt->track=0;

		lstt->orientation.EulerX=lstt->orientation.EulerY=lstt->orientation.EulerZ=0;
	}
	
	if(mss->StrategyType==StratAreaSwitch)
	{
		//switch can be triggered by walking into a ceartain area
		AreaSwitchStrategy* ass=(AreaSwitchStrategy*)mss;
		lstt->trigger_volume_min=ass->trigger_min*local_scale;
		lstt->trigger_volume_max=ass->trigger_max*local_scale;
		lstt->switch_flags=SwitchFlag_UseTriggerVolume;
	}
	else
	{
		lstt->trigger_volume_min.vx=lstt->trigger_volume_max.vx=0;
		lstt->trigger_volume_min.vy=lstt->trigger_volume_max.vy=0;
		lstt->trigger_volume_min.vz=lstt->trigger_volume_max.vz=0;
		lstt->switch_flags=0;
	}

	lstt->switch_off_message_same = ((mss->flags & BinSwitchFlag_OffMessageSame)!=0);
	lstt->switch_off_message_none = ((mss->flags & BinSwitchFlag_OffMessageNone)!=0);
	lstt->rest_state = mss->RestState;
	
	lstt->mode = mss->Mode;
	if(lstt->mode>I_lswitch_toggle)
	{
		lstt->mode=I_lswitch_timer;
	}
	lstt->time_for_reset = mss->Time;
	lstt->security_clearance = mss->Security;
	lstt->shape_num = list_pos;
	*((ObjectID *)lstt->nameID) =ID;

	int i;
	
	lstt->num_targets=mss->NumTargets;
	if(mss->NumTargets)
	{
		lstt->targets=(LINK_SWITCH_TARGET*) PoolAllocateMem(mss->NumTargets * sizeof(LINK_SWITCH_TARGET));
		for(i=0;i<mss->NumTargets;i++)
		{
			*(ObjectID*)lstt->targets[i].name=mss->Targets[i].ID;
			lstt->targets[i].request_message=mss->Targets[i].request;
			lstt->targets[i].sbptr=0;
		}
	}
	else
	{
		lstt->targets=0;
	}
	
	

	if(mss->NumLinks)
		lstt->switchIDs=(SBNAMEBLOCK*) PoolAllocateMem(mss->NumLinks * sizeof(SBNAMEBLOCK));
	else
		lstt->switchIDs=0;
	
	lstt->num_linked_switches=mss->NumLinks;
	for (i=0; i<mss->NumLinks; i++)
	{
		*((ObjectID *)lstt->switchIDs[i].name) = mss->LinkedSwitches[i];
	}
	lstt->switch_always_on=0;	
	
	
	AddToBehaviourList(name,ID, I_BehaviourLinkSwitch, (void*)lstt,list_pos);
	
}

static void add_binswitch (const char* name,AVP_Strategy_Chunk* asc,const ObjectID& ID,Object_Chunk* oc=0,int list_pos=-1)
{
	BinSwitchStrategy * bss = (BinSwitchStrategy *)asc->Strategy;

	BIN_SWITCH_TOOLS_TEMPLATE* bstt=(BIN_SWITCH_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(BIN_SWITCH_TOOLS_TEMPLATE));
	
	if(oc)//switch has a shape
	{
		bstt->position.vx = (int)(oc->object_data.location.x * local_scale);
		bstt->position.vy = (int)(oc->object_data.location.y * local_scale);
		bstt->position.vz = (int)(oc->object_data.location.z * local_scale);

	
		bstt->trigger_volume_min.vx=bstt->trigger_volume_max.vx=0;
		bstt->trigger_volume_min.vy=bstt->trigger_volume_max.vy=0;
		bstt->trigger_volume_min.vz=bstt->trigger_volume_max.vz=0;
		bstt->switch_flags=0;
	
	
		QUAT q;

		q.quatx = (int) -(oc->object_data.orientation.x*ONE_FIXED);
		q.quaty = (int) -(oc->object_data.orientation.y*ONE_FIXED);
		q.quatz = (int) -(oc->object_data.orientation.z*ONE_FIXED);
		q.quatw = (int) (oc->object_data.orientation.w*ONE_FIXED);


		MATRIXCH m;

		QuatToMat (&q, &m);

		MatrixToEuler(&m, &bstt->orientation);
	
		bstt->track=setup_track_controller(oc);
		if(bstt->track)
		{
			bstt->track->loop=FALSE;
			bstt->track->loop_backandforth=FALSE;
			bstt->track->playing=FALSE;
			bstt->track->initial_state_playing=FALSE;
			bstt->track->timer=0;
			bstt->track->initial_state_timer=0;
		}
	}	
	else	//switch has no shape
	{
		bstt->position.vx = 0;
		bstt->position.vy = 0;
		bstt->position.vz = 0;

		bstt->track=0;

		bstt->trigger_volume_min.vx=bstt->trigger_volume_max.vx=0;
		bstt->trigger_volume_min.vy=bstt->trigger_volume_max.vy=0;
		bstt->trigger_volume_min.vz=bstt->trigger_volume_max.vz=0;
		bstt->switch_flags=0;
	
		bstt->orientation.EulerX=bstt->orientation.EulerY=bstt->orientation.EulerZ=0;
	
	}
	
	bstt->starts_on = ((bss->flags & BinSwitchFlag_StartsOn)!=0);
	bstt->switch_off_message_same = ((bss->flags & BinSwitchFlag_OffMessageSame)!=0);
	bstt->switch_off_message_none = ((bss->flags & BinSwitchFlag_OffMessageNone)!=0);

	bstt->mode = bss->Mode;
	bstt->time_for_reset = bss->Time;
	bstt->security_clearance = bss->Security;
	
	bstt->num_targets = 1;

	int* request_message=(int*) PoolAllocateMem(sizeof(int));
	request_message[0]=1|bss->Target.request;
	bstt->request_messages=request_message;

	SBNAMEBLOCK* snb =(SBNAMEBLOCK*) PoolAllocateMem(sizeof(SBNAMEBLOCK));
	*((ObjectID *)snb->name) = bss->Target.ID;
	bstt->target_names = snb;
	
	bstt->shape_num = list_pos;
	*((ObjectID *)bstt->nameID) = ID;

	AddToBehaviourList(name,ID, I_BehaviourBinarySwitch, (void*)bstt,list_pos);

}

static void add_multiswitch (const char* name,AVP_Strategy_Chunk* asc,const ObjectID& ID,Object_Chunk* oc=0,int list_pos=-1)
{
	MultiSwitchStrategy * mss = (MultiSwitchStrategy *)asc->Strategy;
	if(mss->NumLinks)
	{
		add_multitarget_linkswitch(name,asc,ID,oc,list_pos);
		return;
	}
	
	BIN_SWITCH_TOOLS_TEMPLATE* bstt=(BIN_SWITCH_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(BIN_SWITCH_TOOLS_TEMPLATE));
	
	if(oc) //switch has a shape
	{
		bstt->position.vx = (int)(oc->object_data.location.x * local_scale);
		bstt->position.vy = (int)(oc->object_data.location.y * local_scale);
		bstt->position.vz = (int)(oc->object_data.location.z * local_scale);

			
		QUAT q;

		q.quatx = (int) -(oc->object_data.orientation.x*ONE_FIXED);
		q.quaty = (int) -(oc->object_data.orientation.y*ONE_FIXED);
		q.quatz = (int) -(oc->object_data.orientation.z*ONE_FIXED);
		q.quatw = (int) (oc->object_data.orientation.w*ONE_FIXED);


		MATRIXCH m;

		QuatToMat (&q, &m);

		MatrixToEuler(&m, &bstt->orientation);

		bstt->track=setup_track_controller(oc);
		if(bstt->track)
		{
			bstt->track->loop=FALSE;
			bstt->track->loop_backandforth=FALSE;
			bstt->track->playing=FALSE;
			bstt->track->initial_state_playing=FALSE;
			bstt->track->timer=0;
			bstt->track->initial_state_timer=0;
		}
	}
	else //switch has no shapes
	{
		bstt->position.vx = 0;
		bstt->position.vy = 0;
		bstt->position.vz = 0;

		bstt->track=0;
		bstt->orientation.EulerX=bstt->orientation.EulerY=bstt->orientation.EulerZ=0;
	}
		
	if(mss->StrategyType==StratAreaSwitch)
	{
		//switch can be triggered by walking into a ceartain area
		AreaSwitchStrategy* ass=(AreaSwitchStrategy*)mss;
		bstt->trigger_volume_min=ass->trigger_min*local_scale;
		bstt->trigger_volume_max=ass->trigger_max*local_scale;
		bstt->switch_flags=SwitchFlag_UseTriggerVolume;
	}
	else
	{
		bstt->trigger_volume_min.vx=bstt->trigger_volume_max.vx=0;
		bstt->trigger_volume_min.vy=bstt->trigger_volume_max.vy=0;
		bstt->trigger_volume_min.vz=bstt->trigger_volume_max.vz=0;
		bstt->switch_flags=0;
	}
	
	
	bstt->starts_on = mss->RestState;
	bstt->switch_off_message_same = ((mss->flags & MultiSwitchFlag_OffMessageSame)!=0);
	bstt->switch_off_message_none = ((mss->flags & MultiSwitchFlag_OffMessageNone)!=0);
	
	bstt->mode = mss->Mode;
	bstt->time_for_reset = mss->Time;
	bstt->security_clearance = mss->Security;
	
	bstt->num_targets = mss->NumTargets;
	
	
	SBNAMEBLOCK * snb = (SBNAMEBLOCK*) PoolAllocateMem(sizeof(SBNAMEBLOCK)*mss->NumTargets);
	
	int* request_messages=(int*) PoolAllocateMem(sizeof(int) * mss->NumTargets);
	for (int i=0; i<mss->NumTargets; i++)
	{
		*((ObjectID *)snb[i].name) = mss->Targets[i].ID;
		request_messages[i]=mss->Targets[i].request;
	}

	bstt->request_messages=request_messages;
	
	bstt->target_names = snb;
	
	bstt->shape_num = list_pos;
	*((ObjectID *)bstt->nameID) = ID;
	

	AddToBehaviourList(name,ID, I_BehaviourBinarySwitch, (void*)bstt,list_pos);
	
}


static void add_platlift (Object_Chunk * ob, int list_pos, AVP_Strategy_Chunk * asc)
{
	PlatLiftStrategy* pls=(PlatLiftStrategy*)asc->Strategy;

	PLATFORMLIFT_TOOLS_TEMPLATE* ptt=(PLATFORMLIFT_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(PLATFORMLIFT_TOOLS_TEMPLATE));
	
	ptt->position.vx = (int)(ob->object_data.location.x * local_scale);
	ptt->position.vy = (int)(ob->object_data.location.y * local_scale);
	ptt->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &ptt->orientation);

	ptt->shapeIndex = list_pos;
	*((ObjectID *)ptt->nameID) = ob->object_data.ID;
	ptt->travel=0;
	
	Object_Track_Chunk2* otc=(Object_Track_Chunk2*)ob->lookup_single_child("OBJTRAK2");
	if(otc)
	{
		if(otc->num_sections==1)
		{
			ptt->travel=(int)((otc->sections[0].pivot_end.y-otc->sections[0].pivot_start.y)*local_scale);
		}
	}
	else
	{
		GLOBALASSERT(0=="Platform lift must have a track");
	}

	setup_track_sound(ob,&ptt->start_sound,&ptt->sound,&ptt->end_sound);
	
	if(pls->flags & PlatformLiftFlags_Disabled)
		ptt->Enabled=FALSE;
	else
		ptt->Enabled=TRUE;

	if(pls->flags & PlatformLiftFlags_OneUse)
		ptt->OneUse=TRUE;
	else
		ptt->OneUse=FALSE;

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourPlatform, (void*)ptt,list_pos,0,ob->get_header()->flags);
	
}

static void add_deathvolume(const char* name,AVP_Strategy_Chunk* asc,ObjectID ID)
{
	DeathVolumeStrategy* dvs=(DeathVolumeStrategy*)asc->Strategy;
	
	DEATH_VOLUME_TOOLS_TEMPLATE* dvtt=(DEATH_VOLUME_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(DEATH_VOLUME_TOOLS_TEMPLATE));

	dvtt->volume_min=dvs->volume_min*local_scale;
	dvtt->volume_max=dvs->volume_max*local_scale;
	if(dvs->flags & DeathVolumeFlag_StartsOn)
		dvtt->active=1;
	else
		dvtt->active=0;

	if(dvs->flags & DeathVolumeFlag_CollisionNotRequired)
		dvtt->collision_required=0;
	else
		dvtt->collision_required=1;

	dvtt->damage_per_second = dvs->damage;

	*(ObjectID*)&dvtt->nameID[0]=ID;
	

	AddToBehaviourList(name,ID, I_BehaviourDeathVolume, (void*)dvtt);
}

static void add_selfdestruct(const char* name,AVP_Strategy_Chunk* asc,ObjectID ID)
{
	SelfDestructStrategy* sds=(SelfDestructStrategy*)asc->Strategy;
	
	SELF_DESTRUCT_TOOLS_TEMPLATE* sdtt=(SELF_DESTRUCT_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(SELF_DESTRUCT_TOOLS_TEMPLATE));

	sdtt->timer=sds->timer<<16;

	*(ObjectID*)&sdtt->nameID[0]=ID;
	

	AddToBehaviourList(name,ID, I_BehaviourSelfDestruct, (void*)sdtt);
}

static void add_message_strategy(const char* name,AVP_Strategy_Chunk* asc,ObjectID ID)
{
	TextMessageStrategy* tms=(TextMessageStrategy*)asc->Strategy;
	assert (tms->message_string>0 && tms->message_string<450);

	MESSAGE_TOOLS_TEMPLATE* mtt=(MESSAGE_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(MESSAGE_TOOLS_TEMPLATE));

	mtt->string_no=(TEXTSTRING_ID)((tms->message_string-1)+TEXTSTRING_LEVELMSG_001);
	*(ObjectID*)&mtt->nameID[0]=ID;

	if(tms->flags & TextMessageFlag_NotActiveAtStart)
		mtt->active=FALSE;
	else
		mtt->active=TRUE;



	AddToBehaviourList(name,ID, I_BehaviourMessage, (void*)mtt);
	
}

void setup_placed_light_data (LIGHTBLOCK * lPtr, Placed_Object_Light_Chunk * lc);

static void add_placed_light(Object_Chunk* ob,int list_pos,AVP_Strategy_Chunk* asc)
{
	SimpleStrategy * ss = (SimpleStrategy *)asc->Strategy;
	


	TOOLS_DATA_PLACEDLIGHT* pltd=(TOOLS_DATA_PLACEDLIGHT*) PoolAllocateMem(sizeof(TOOLS_DATA_PLACEDLIGHT));
	
	pltd->position.vx = (int)(ob->object_data.location.x * local_scale);
	pltd->position.vy = (int)(ob->object_data.location.y * local_scale);
	pltd->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &pltd->orientation);
	
	pltd->shapeIndex = list_pos;
	pltd->mass = ss->mass/10;
	if (!pltd->mass) pltd->mass = 1;
	pltd->integrity = ss->integrity & 0xff;
	*((ObjectID *)pltd->nameID) = ob->object_data.ID;

	Placed_Object_Light_Chunk* lchunk=(Placed_Object_Light_Chunk*)ob->lookup_single_child("PLOBJLIT");
	GLOBALASSERT(lchunk);

	pltd->light=(LIGHTBLOCK*) PoolAllocateMem(sizeof(LIGHTBLOCK));
	
	setup_placed_light_data(pltd->light,lchunk);

	pltd->colour_red=((lchunk->light.up_colour>>16)&0xff)*257;
	pltd->colour_green=((lchunk->light.up_colour>>8)&0xff)*257;
	pltd->colour_blue=((lchunk->light.up_colour)&0xff)*257;
	pltd->colour_diff_red=((lchunk->light.down_colour>>16)&0xff)*257;
	pltd->colour_diff_green=((lchunk->light.down_colour>>8)&0xff)*257;
	pltd->colour_diff_blue=((lchunk->light.down_colour)&0xff)*257;

	pltd->colour_diff_red-=pltd->colour_red;
	pltd->colour_diff_green-=pltd->colour_green;
	pltd->colour_diff_blue-=pltd->colour_blue;

	pltd->fade_up_time=(max(lchunk->light.fade_up_time,1)*ONE_FIXED)/1000;
	pltd->fade_down_time=(max(lchunk->light.fade_down_time,1)*ONE_FIXED)/1000;
	pltd->up_time=(max(lchunk->light.up_time,1)*ONE_FIXED)/1000;
	pltd->down_time=(max(lchunk->light.down_time,1)*ONE_FIXED)/1000;
	pltd->timer=(max(lchunk->light.start_time,1)*ONE_FIXED)/1000;

	pltd->type=(LIGHT_TYPE)lchunk->light.light_type;
	pltd->on_off_type=(LIGHT_ON_OFF_TYPE)lchunk->light.on_off_type;
	
	if(lchunk->light.flags & PlacedLightFlag_On)
	{
		pltd->sequence=1;
		pltd->on_off_state=Light_OnOff_On;

	}
	else
	{
		pltd->sequence=0;
		pltd->on_off_state=Light_OnOff_Off;
		//pltd->light->LightBright=0;
	}
	
	if(lchunk->light.flags & PlacedLightFlag_SwapColourBright)
		pltd->swap_colour_and_brightness_alterations=1;
	else
		pltd->swap_colour_and_brightness_alterations=0;


	if(pltd->type==Light_Type_Strobe)
		pltd->state=Light_State_StrobeUp;
	else if(pltd->type==Light_Type_Flicker)
		pltd->state=Light_State_Flicker;
	else
		pltd->state=Light_State_Standard;

   	pltd->destruct_target_request=ss->target_request;
   	*(ObjectID*)pltd->destruct_target_ID=ss->targetID;

	pltd->static_light=1;

	
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourPlacedLight, (void *) pltd,list_pos,0,ob->get_header()->flags);
}

static void add_prox_door (Object_Chunk * ob, int shp1, int shp2, MODULE * mod,AVP_Strategy_Chunk* asc)
{

	PROX_DOOR_TOOLS_TEMPLATE * pdtt =(PROX_DOOR_TOOLS_TEMPLATE *) PoolAllocateMem(sizeof(PROX_DOOR_TOOLS_TEMPLATE));

	pdtt->has_lock_target = No;
	pdtt->door_opening_speed=1<<16;
	pdtt->door_closing_speed=1<<17;
	if(asc)
	{
		DoorStrategy * ds = (DoorStrategy *)asc->Strategy;
		if(ds->DoorFlags & DoorFlag_Locked)
			pdtt->door_is_locked=Yes;
		else
			pdtt->door_is_locked=No;

		if(ds->DoorFlags & DoorFlag_Horizontal)
		{
			mod->m_flags|=MODULEFLAG_HORIZONTALDOOR;
		}
		if(ds->TimeToOpen)
		{
			pdtt->door_opening_speed=655360/ds->TimeToOpen;
		}
		if(ds->TimeToClose)
		{
			pdtt->door_closing_speed=655360/ds->TimeToClose;
		}
		
	}
	else pdtt->door_is_locked=No;
	pdtt->shape_open = shp1;
	pdtt->shape_closed = shp2;
	*((int *)pdtt->my_module.mref_name) = *((int *) mod->m_name);
	*((ObjectID *)pdtt->nameID) = ob->object_data.ID;

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourProximityDoor, (void*)pdtt,shp1);


	
}


static void add_lift_door (Object_Chunk * ob, int shape1, int shape2, MODULE * mod, AVP_Strategy_Chunk * asc)
{

	DoorStrategy * ds = (DoorStrategy *)asc->Strategy;

	LIFT_DOOR_TOOLS_TEMPLATE * ldtt =(LIFT_DOOR_TOOLS_TEMPLATE *) PoolAllocateMem(sizeof( LIFT_DOOR_TOOLS_TEMPLATE));
	
	ldtt->state = (ds->DoorFlags & DoorFlag_Open) ? I_door_open : I_door_closed;
	*((int *)ldtt->my_module.mref_name) = *((int *)mod->m_name);
	*((ObjectID *)ldtt->nameID) = ob->object_data.ID;
	ldtt->shape_open = shape1;
	ldtt->shape_closed = shape2;

	if(ds->TimeToOpen)
		ldtt->door_opening_speed=655360/ds->TimeToOpen;
	else
		ldtt->door_opening_speed=1<<16;

	if(ds->TimeToClose)
	 	ldtt->door_closing_speed=655360/ds->TimeToClose;
	else
		ldtt->door_closing_speed=1<<17;
	
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourLiftDoor, (void*)ldtt,shape1);

}



static void add_switch_door (Object_Chunk * ob, int shape1, int shape2, MODULE * mod, AVP_Strategy_Chunk * asc)
{

	SwitchDoorStrategy * sds = (SwitchDoorStrategy*)asc->Strategy;

	SWITCH_DOOR_TOOLS_TEMPLATE * sdtt =(SWITCH_DOOR_TOOLS_TEMPLATE *) PoolAllocateMem(sizeof(SWITCH_DOOR_TOOLS_TEMPLATE));
	
	sdtt->state =I_door_closed;
	*((int *)sdtt->myModule.mref_name) = *((int *)mod->m_name);
	*((ObjectID *)sdtt->nameID) = ob->object_data.ID;
	sdtt->shapeOpen = shape1;
	sdtt->shapeClosed = shape2;
	*(ObjectID*)sdtt->linkedDoorName=sds->AssocDoor;

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourSwitchDoor, (void*)sdtt,shape1);

}

static void add_door (Object_Chunk * ob, int shape1, int shape2, MODULE * mod, AVP_Strategy_Chunk * asc)
{

	DoorStrategy * ds = (DoorStrategy *)asc->Strategy;
	
	if (ds->DoorFlags & DoorFlag_Lift)
	{
		add_lift_door (ob, shape1, shape2, mod, asc);
	}
	else if (ds->DoorFlags & DoorFlag_Proximity)
	{
		add_prox_door (ob, shape1, shape2, mod, asc);
	}
}

static void add_lift_object (Object_Chunk * ob, int list_pos, MODULE * mod, AVP_Strategy_Chunk * asc)
{
	LiftStrategy * ls = (LiftStrategy *)asc->Strategy;
	
	ObjectID ControlID=ob->object_data.ID;
	for(int i=0;i<ls->NumAssocLifts;i++)
	{
		ControlID=Minimum(ControlID,ls->AssocLifts[i]);
	}
	LIFT_TOOLS_TEMPLATE * ltt =(LIFT_TOOLS_TEMPLATE *) PoolAllocateMem(sizeof(LIFT_TOOLS_TEMPLATE));
	
	*((ObjectID *)ltt->my_module_name) = ob->object_data.ID;
	*((ObjectID *)ltt->call_switch_name) = ls->AssocCallSwitch;
	*((ObjectID *)ltt->lift_door_name) = ls->AssocDoor;
	*((ObjectID *)ltt->lift_floor_switch_name) = ls->AssocFloorSwitch;

	ltt->environment = AvP.CurrentEnv;
	
	ltt->num_floor = ls->Floor;

	*((ObjectID *)ltt->control_sb_name)=ControlID;
	
	ltt->controller = (ControlID==ob->object_data.ID);
	
	ltt->num_stations = ls->NumAssocLifts +ls->NumExternalLifts +1;

	ltt->lift_flags=ls->LiftFlags;
	
	*((int *)ltt->my_module.mref_name) = *((int *)mod->m_name);
	*((ObjectID *)ltt->nameID) = ob->object_data.ID;
	
	ltt->orient=ls->Facing;

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourLift, (void*)ltt,list_pos);
	
}


static void add_light_fx (Object_Chunk * ob,int list_pos, MODULE * mod, AVP_Strategy_Chunk * asc)
{
	LightingStrategy * ls = (LightingStrategy *)asc->Strategy;
	
	LIGHT_FX_TOOLS_TEMPLATE * lfxtt =(LIGHT_FX_TOOLS_TEMPLATE *) PoolAllocateMem(sizeof(LIGHT_FX_TOOLS_TEMPLATE));
	
	lfxtt->light_data = ls->LightData;
	
	*((ObjectID *)lfxtt->nameID) = ob->object_data.ID;
	*((int *)lfxtt->my_module.mref_name) = *((int *)mod->m_name);
	
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourLightFX, (void*)lfxtt,list_pos);
	
}

void deal_with_module_object(Object_Chunk * ob, int shape1, int AnimationShape, int shape2, MODULE * mod)
{
	Chunk * pChunk = ob->lookup_single_child("OBJPRJDT");
	
	AVP_Strategy_Chunk * asc = 0;

	Object_Project_Data_Chunk * opdc = 0;
	
	if (pChunk)
	{
		opdc = (Object_Project_Data_Chunk *)pChunk;
		
		pChunk = opdc->lookup_single_child("AVPSTRAT");
		
		if (pChunk)
		{
			asc = (AVP_Strategy_Chunk *)pChunk;
		}
	}
	
	if (asc)
	{
		assert (asc->Strategy != 0);

		switch (asc->Strategy->StrategyType)
		{
			case StratLift:
				add_lift_object(ob, shape1, mod, asc);
				break;
				
			case StratDoor:
				if (shape2 != -1)
				{
					add_door (ob, shape1, shape2, mod, asc);
				}
				break;
			case StratSwitchDoor:
				if (shape2 != -1)
				{
					add_switch_door (ob, shape1, shape2, mod, asc);
				}
				break;
				
			case StratLighting:
			{
				add_light_fx (ob,shape1, mod, asc);
				break;
			}	
			
			default:
				assert (0); // Strategy not recognised
				break;
		}
	}
	else
	{

		if (shape2 != -1)
		{
			add_prox_door (ob, shape1, shape2, mod,0);
		}
		else if(AnimationShape!=-1)
		{
			add_simple_animation (ob, AnimationShape, mod);
		}
		
	}
}


static void add_videoscreen(Object_Chunk * ob, int list_pos, AVP_Strategy_Chunk * asc)
{
	SimpleStrategy * ss = (SimpleStrategy *)asc->Strategy;
	
	TOOLS_DATA_VIDEO_SCREEN* tdvs=(TOOLS_DATA_VIDEO_SCREEN*) PoolAllocateMem(sizeof(TOOLS_DATA_VIDEO_SCREEN));
	
	tdvs->position.vx = (int)(ob->object_data.location.x * local_scale);
	tdvs->position.vy = (int)(ob->object_data.location.y * local_scale);
	tdvs->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &tdvs->orientation);
	
	tdvs->shapeIndex = list_pos;
	tdvs->integrity = ss->integrity & 0xff;
	*((ObjectID *)tdvs->nameID) = ob->object_data.ID;

   	//copy destruction target info
   	tdvs->destruct_target_request=ss->target_request;
   	*(ObjectID*)tdvs->destruct_target_ID=ss->targetID;
	
	
	Object_Alternate_Locations_Chunk* loc_chunk = (Object_Alternate_Locations_Chunk*) ob->lookup_single_child("ALTLOCAT");

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourVideoScreen, (void *) tdvs,list_pos,loc_chunk,ob->get_header()->flags);
}

static void add_newsimpleobject(Object_Chunk * ob, int list_pos, AVP_Strategy_Chunk * asc)
{
// test for video screen which is type ReservedForJohn

	SimpleStrategy * ss = (SimpleStrategy *)asc->Strategy;

	if (ss->Type == IOT_ReservedForJohn)
	{
		add_videoscreen (ob, list_pos, asc);
		return;
	}

	if(ob->lookup_single_child("PLOBJLIT"))
	{
		add_placed_light(ob,list_pos,asc);
		return;
	}

	
	TOOLS_DATA_INANIMATEOBJECT* tdio=(TOOLS_DATA_INANIMATEOBJECT*) PoolAllocateMem(sizeof(TOOLS_DATA_INANIMATEOBJECT));
	
	tdio->position.vx = (int)(ob->object_data.location.x * local_scale);
	tdio->position.vy = (int)(ob->object_data.location.y * local_scale);
	tdio->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &tdio->orientation);
	
	tdio->typeId = (INANIMATEOBJECT_TYPE)ss->Type;
	tdio->subType = ss->ExtraData;
	tdio->shapeIndex = list_pos;
	tdio->mass = ss->mass/10;
	if (!tdio->mass) tdio->mass = 1;
	tdio->integrity = ss->integrity & 0xff;
	*((ObjectID *)tdio->nameID) = ob->object_data.ID;

	
	
	tdio->explosionType=(ss->flags & SimStratFlag_ExplosionMask)>>SimStratFlag_ExplosionShift;
	
	
	if(ss->targetID.id1 || ss->targetID.id2)
	{
   		tdio->triggering_event=ss->flags;
   		tdio->event_request=ss->target_request;
   		*(ObjectID*)tdio->event_target_ID=ss->targetID;
	}
	else
	{
		tdio->triggering_event=0;
	}
	
	if(tdio->typeId==IOT_Ammo)
	{
		//need to convert the subtype number
		switch(tdio->subType)
		{
			case 0:
				tdio->subType=AMMO_10MM_CULW;
				break;
			case 2:
				tdio->subType=AMMO_SMARTGUN;
				break;
			case 3:
				tdio->subType=AMMO_FLAMETHROWER;
				break;
			case 5:
				tdio->subType=AMMO_SADAR_TOW;
				break;
			case 6:
				tdio->subType=AMMO_GRENADE;
				break;
			case 7:
				tdio->subType=AMMO_MINIGUN;
				break;
			case 8:
				tdio->subType=AMMO_PULSE_GRENADE;
				break;
			case 9:
				tdio->subType=AMMO_PRED_RIFLE;
				break;
		}
	}

	Object_Alternate_Locations_Chunk* loc_chunk = (Object_Alternate_Locations_Chunk*) ob->lookup_single_child("ALTLOCAT");

	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourInanimateObject, (void *) tdio,list_pos,loc_chunk,ob->get_header()->flags);
	
}

static void add_simpleobject(Object_Chunk * ob, int list_pos, AVP_Strategy_Chunk * asc)
{
// test for sentry gun which is type ReservedForJohn

	if (asc->Strategy->Type == IOT_ReservedForJohn)
	{
		//add_autogun (ob, list_pos, asc->Strategy->ExtraData);
		return;
	}

	
	TOOLS_DATA_INANIMATEOBJECT* tdio=(TOOLS_DATA_INANIMATEOBJECT*) PoolAllocateMem(sizeof(TOOLS_DATA_INANIMATEOBJECT));
	
	tdio->position.vx = (int)(ob->object_data.location.x * local_scale);
	tdio->position.vy = (int)(ob->object_data.location.y * local_scale);
	tdio->position.vz = (int)(ob->object_data.location.z * local_scale);

	QUAT q;

	q.quatx = (int) -(ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) -(ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) -(ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	MatrixToEuler(&m, &tdio->orientation);
	
	tdio->typeId = (INANIMATEOBJECT_TYPE)asc->Strategy->Type;
	tdio->subType = asc->Strategy->ExtraData;
	tdio->shapeIndex = list_pos;
	tdio->mass = 5;
	tdio->integrity = DEFAULT_OBJECT_INTEGRITY;
	tdio->explosionType=0;
	*((ObjectID *)tdio->nameID) = ob->object_data.ID;
	
	Object_Alternate_Locations_Chunk* loc_chunk = (Object_Alternate_Locations_Chunk*) ob->lookup_single_child("ALTLOCAT");
	
	AddToBehaviourList(ob->object_data.o_name,ob->object_data.ID, I_BehaviourInanimateObject, (void *) tdio,list_pos,loc_chunk,ob->get_header()->flags);
	
}

void deal_with_placed_object(Object_Chunk * ob, int shape1, int /*AnimationShape*/)
{
	db_logf3(("Dealing with object %s , shape %d",ob->object_data.o_name,shape1));
	Chunk * pChunk = ob->lookup_single_child("OBJPRJDT");
	
	AVP_Strategy_Chunk * asc = 0;

	Object_Project_Data_Chunk * opdc = 0;
	
	if (pChunk)
	{
		opdc = (Object_Project_Data_Chunk *)pChunk;
		
		pChunk = opdc->lookup_single_child("AVPSTRAT");
		
		if (pChunk)
		{
			asc = (AVP_Strategy_Chunk *)pChunk;
		}
	}
	
	int obflags=ob->get_header()->flags;
	if (asc)
	{
		assert (asc->Strategy != 0);

		switch (asc->Strategy->StrategyType)
		{
			case StratBinSwitch:
				add_binswitch (ob->object_data.o_name,asc,ob->object_data.ID,ob,shape1);
				break;

			case StratAreaSwitch: //(Area Switch derived from Multi Switch)
			case StratMultiSwitch:
				add_multiswitch (ob->object_data.o_name,asc,ob->object_data.ID,ob,shape1);
				break;

			case StratLinkSwitch:
				add_linkswitch (ob->object_data.o_name,asc,ob->object_data.ID,ob,shape1);
				break;
				
			case StratPlatLift :
				add_platlift(ob,shape1,asc);
				break;

			case StratSimpleObject:
		 		if(obflags & OBJECT_FLAG_PCLOAD)
				{
					if((AvP.PlayerType==I_Marine && (obflags & OBJECT_FLAG_AVPGAMEMODEMARINE))||
					   (AvP.PlayerType==I_Predator && (obflags & OBJECT_FLAG_AVPGAMEMODEPREDATOR))||
					   (AvP.PlayerType==I_Alien && (obflags & OBJECT_FLAG_AVPGAMEMODEALIEN)))
					{
						add_simpleobject (ob, shape1,asc);
					}
		 		}
				break;

			case StratNewSimpleObject:
		 		if(obflags & OBJECT_FLAG_PCLOAD)
				{
					if((AvP.PlayerType==I_Marine && (obflags & OBJECT_FLAG_AVPGAMEMODEMARINE))||
					   (AvP.PlayerType==I_Predator && (obflags & OBJECT_FLAG_AVPGAMEMODEPREDATOR))||
					   (AvP.PlayerType==I_Alien && (obflags & OBJECT_FLAG_AVPGAMEMODEALIEN)))
					{
						add_newsimpleobject (ob, shape1,asc);
					}
		 		}
				break;
			

			case StratTrack :
			case StratTrackDestruct :
				add_trackobject(ob,shape1,asc);
				break;
			
			case StratFan :
				add_fan(ob,shape1,asc);
				break;
			
			default:
				assert (0); // Strategy not recognised
				break;
		}
	}
	else
	{
#if 0
		if(AnimationShape!=-1)
		{
			add_simple_animation (ob, AnimationShape, 0);
		}
		else
#endif		
		{
		 	if(obflags & OBJECT_FLAG_PCLOAD)
			{
				if((AvP.PlayerType==I_Marine && (obflags & OBJECT_FLAG_AVPGAMEMODEMARINE))||
				   (AvP.PlayerType==I_Predator && (obflags & OBJECT_FLAG_AVPGAMEMODEPREDATOR))||
				   (AvP.PlayerType==I_Alien && (obflags & OBJECT_FLAG_AVPGAMEMODEALIEN)))
				{
					add_default_object(ob,shape1);
				}
		 	}
		}
	}
	db_logf3(("Finished with object %s , shape %d",ob->object_data.o_name,shape1));
}

static void add_alien(AVP_Generator_Chunk * agc)
{

	TOOLS_DATA_ALIEN* tda=(TOOLS_DATA_ALIEN*) PoolAllocateMem(sizeof(TOOLS_DATA_ALIEN));
	tda->position.vx = (int)(agc->location.x * local_scale);
	tda->position.vy = (int)(agc->location.y * local_scale);
	tda->position.vz = (int)(agc->location.z * local_scale);
	
	//tda->shapeIndex = GetLoadedShapeMSL("Alien");
	tda->shapeIndex = 0;

	tda->start_inactive=(agc->flags & AVPGENFLAG_GENERATORINACTIVE)!=0;

	tda->starteuler.EulerX=0;
	tda->starteuler.EulerY=agc->orientation & 4095;
	tda->starteuler.EulerZ=0;
	

	switch (agc->type)
	{
		case 1:
			tda->type=AT_Standard;
			break;
		
		case 5:
			tda->type=AT_Predalien;
			break;

		case 8:
			tda->type=AT_Praetorian;
			break;
			
		default :
			tda->type=AT_Standard;
	}

	//look for extra strategy stuff
	for(int i=0;i<SB_NAME_LENGTH;i++) tda->death_target_ID[i]=0;
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;
						
							*(ObjectID*)&tda->death_target_ID[0]=es->DeathTarget;
						}
						break;
						
				}
			}
		}
	}


	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tda->nameID[0]=ID;
	
	AddToBehaviourList(agc->name,ID, I_BehaviourAlien, (void *) tda,0,agc->get_alternate_locations_chunk(),agc->flags);
}

extern int ConvertObjectIndexToPathIndex(int path_index,int object_index);


static void add_marine(AVP_Generator_Chunk * agc)
{
	TOOLS_DATA_MARINE* tdm=(TOOLS_DATA_MARINE*) PoolAllocateMem(sizeof(TOOLS_DATA_MARINE));

	tdm->position.vx = (int)(agc->location.x * local_scale);
	tdm->position.vy = (int)(agc->location.y * local_scale);
	tdm->position.vz = (int)(agc->location.z * local_scale);
	
	EULER euler;
	euler.EulerX=0;
	euler.EulerY=agc->orientation & 4095;
	euler.EulerZ=0;

	get_marine_facing_point(tdm->position,euler,tdm->facing_point);

	
	tdm->shapeIndex = 0;

	tdm->textureID=agc->textureID;
	

	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tdm->nameID[0]=ID;

	tdm->Mission=MM_Wait_Then_Wander;
	tdm->path=-1;
 	tdm->stepnumber=-1;

	//look for extra strategy stuff
	for(int i=0;i<SB_NAME_LENGTH;i++) tdm->death_target_ID[i]=0;
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;

							switch (es->MissionType)
							{
								case 1:
									tdm->Mission=MM_Wander;
									break;
								case 2:
									tdm->Mission=MM_Guard;
									break;
								case 3:
									tdm->Mission=MM_NonCom;
									break;
								case 4:
									tdm->Mission=MM_LocalGuard;
									break;
								case 5:
									tdm->Mission=MM_Pathfinder;
									tdm->path=es->ExtraMissionData &0xffff;
									tdm->stepnumber=ConvertObjectIndexToPathIndex(tdm->path,es->ExtraMissionData>>16);
									if(tdm->stepnumber==-1)
									{
										LOGDXFMT(("Error setting up path for marine %s\n",agc->name));	
										GLOBALASSERT(0);
									}
									break;
							}
							
							*(ObjectID*)&tdm->death_target_ID[0]=es->DeathTarget;
							
							tdm->death_target_request=es->target_request;
								
						}
						break;
				}
			}
		}
	}

	switch (agc->sub_type)
	{
		case 0:	//pulse rifle
			tdm->marine_type=MNPCW_PulseRifle;	
			break;

		case 5:	//pistol marine
			tdm->marine_type=MNPCW_PistolMarine;	
			break;
	
		case 10: //smartgun
			tdm->marine_type=MNPCW_Smartgun;	
			break;
	
		case 20://grenade launcher
			tdm->marine_type=MNPCW_GrenadeLauncher;	
			break;
	
		case 30://shotgun 1
		case 31://shotgun 2
		case 32://shotgun 3
			tdm->marine_type=MNPCW_MShotgun;
			break;

		case 80://Molotov
			tdm->marine_type=MNPCW_MMolotov;
			break;
	
		case 40://pistol 1
		case 41://pistol 2
		case 90://Scientist1
		case 91://Scientist2
			tdm->marine_type=MNPCW_MPistol;
			break;
		
		case 50://flamethrower
			tdm->marine_type=MNPCW_Flamethrower;	
			break;

		case 60://sadar
			tdm->marine_type=MNPCW_SADAR;
			break;

		case 70://Minigun
			tdm->marine_type=MNPCW_Minigun;
	   		break;
		
		case 100://Civilian flamer
			tdm->marine_type=MNPCW_MFlamer;
			break;

		case 110://Civilian unarmed
			tdm->marine_type=MNPCW_MUnarmed;
			break;

		case 120://android
			tdm->marine_type=MNPCW_Android;
			break;
		
		default:
			tdm->marine_type=MNPCW_PulseRifle;	
			break;
	}
	
	
	AddToBehaviourList(agc->name,ID, I_BehaviourMarine, (void *) tdm,0,agc->get_alternate_locations_chunk(),agc->flags);
}	


static void add_predator(AVP_Generator_Chunk * agc)
{

	TOOLS_DATA_PREDATOR* tdp=(TOOLS_DATA_PREDATOR*) PoolAllocateMem(sizeof(TOOLS_DATA_PREDATOR));
	tdp->position.vx = (int)(agc->location.x * local_scale);
	tdp->position.vy = (int)(agc->location.y * local_scale);
	tdp->position.vz = (int)(agc->location.z * local_scale);
	
	#if 0
	switch (agc->textureID)
	{
		case 1:
			tdp->shapeIndex = GetLoadedShapeMSL("PHead1");	
			break;	
		case 2:
			tdp->shapeIndex = GetLoadedShapeMSL("PHead2");	
			break;	
		case 3:
			tdp->shapeIndex = GetLoadedShapeMSL("PHead3");	
			break;	
		case 4:
			tdp->shapeIndex = GetLoadedShapeMSL("PHead4");	
			break;
		default :
			tdp->shapeIndex = GetLoadedShapeMSL("fred");	/* patrick 8/7/96 */
			break;	
	}
	#endif
	tdp->predator_number=agc->textureID;
	tdp->shapeIndex = 0;

	switch((agc->sub_type)&7)
	{
		case 0:
			tdp->primary=PNPCW_Pistol;
			break;
		case 1:
			tdp->primary=PNPCW_Wristblade;
			break;
		case 2:
			tdp->primary=PNPCW_PlasmaCaster;
			break;
		case 3:
			tdp->primary=PNPCW_Staff;
			break;
		default :
			tdp->primary=PNPCW_Pistol;
			break;

	}
	switch((agc->sub_type>>3)&7)
	{
		case 0:
			tdp->secondary=PNPCW_Pistol;
			break;
		case 1:
			tdp->secondary=PNPCW_Wristblade;
			break;
		case 2:
			tdp->secondary=PNPCW_PlasmaCaster;
			break;
		case 3:
			tdp->secondary=PNPCW_Staff;
			break;
		default :
			tdp->secondary=PNPCW_Pistol;
			break;

	}
	//make sure primary and secondary weapons are different
	if(tdp->primary==tdp->secondary)
	{
		if(tdp->primary==PNPCW_Wristblade)	
			tdp->secondary=PNPCW_Pistol;
		else
			tdp->secondary=PNPCW_Wristblade;
	}

	tdp->path=-1;
	tdp->stepnumber=-1;
	
	for(int i=0;i<SB_NAME_LENGTH;i++) tdp->death_target_ID[i]=0;
	//look for extra strategy stuff
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;

							*(ObjectID*)&tdp->death_target_ID[0]=es->DeathTarget;
							tdp->death_target_request=es->target_request;
							switch (es->MissionType)
							{
								case 1:
									tdp->path=es->ExtraMissionData &0xffff;
									tdp->stepnumber=ConvertObjectIndexToPathIndex(tdp->path,es->ExtraMissionData>>16);
									if(tdp->stepnumber==-1)
									{
										LOGDXFMT(("Error setting up path for predator %s\n",agc->name));	
										GLOBALASSERT(0);
									}
									
									break;
							}
							
								
						}
						break;
				}
			}
		}
	}
	

	 	

	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tdp->nameID[0] = ID;

	if(agc->flags & AVPGENFLAG_GENERATORINACTIVE)
		AddToBehaviourList(agc->name,ID, I_BehaviourDormantPredator, (void *) tdp,0,agc->get_alternate_locations_chunk(),agc->flags);
	else
		AddToBehaviourList(agc->name,ID, I_BehaviourPredator, (void *) tdp,0,agc->get_alternate_locations_chunk(),agc->flags);
}	

static void add_queen(AVP_Generator_Chunk * agc)
{
	
	TOOLS_DATA_QUEEN* tdq=(TOOLS_DATA_QUEEN*) PoolAllocateMem(sizeof(TOOLS_DATA_QUEEN));
	tdq->position.vx = (int)(agc->location.x * local_scale);
	tdq->position.vy = (int)(agc->location.y * local_scale);
	tdq->position.vz = (int)(agc->location.z * local_scale);

	tdq->shapeIndex = 0;

	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tdq->nameID[0]=ID;


	//look for extra strategy stuff
	for(int i=0;i<SB_NAME_LENGTH;i++) tdq->death_target_ID[i]=0;
	tdq->death_target_request=0;

	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;
						
							*(ObjectID*)&tdq->death_target_ID[0]=es->DeathTarget;
							tdq->death_target_request=es->target_request;
						}
						break;
						
				}
			}
		}
	}									


	AddToBehaviourList(agc->name,ID, I_BehaviourQueenAlien, (void *) tdq,0,agc->get_alternate_locations_chunk(),agc->flags);
	
}	

static void add_hugger(AVP_Generator_Chunk * agc)
{

	TOOLS_DATA_FACEHUGGER* tdfh =(TOOLS_DATA_FACEHUGGER*) PoolAllocateMem(sizeof(TOOLS_DATA_FACEHUGGER));

	tdfh->position.vx = (int)(agc->location.x * local_scale);
	tdfh->position.vy = (int)(agc->location.y * local_scale);
	tdfh->position.vz = (int)(agc->location.z * local_scale);

	//tdfh->shapeIndex = GetLoadedShapeMSL("Facehug");
	tdfh->shapeIndex = 0;

	tdfh->startInactive=(agc->flags & AVPGENFLAG_GENERATORINACTIVE)!=0;

	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tdfh->nameID[0]=ID;


	for(int i=0;i<SB_NAME_LENGTH;i++) tdfh->death_target_ID[i]=0;
	
	//look for extra strategy stuff
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;

							*(ObjectID*)&tdfh->death_target_ID[0]=es->DeathTarget;
							tdfh->death_target_request=es->target_request;
													
						}
						break;
				}
			}
		}
	}

	
	AddToBehaviourList(agc->name,ID, I_BehaviourFaceHugger, (void *) tdfh,0,agc->get_alternate_locations_chunk(),agc->flags);
}

static void add_autogun(AVP_Generator_Chunk * agc)
{
	AUTOGUN_TOOLS_TEMPLATE* att=(AUTOGUN_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(AUTOGUN_TOOLS_TEMPLATE));
	att->position.vx = (int)(agc->location.x * local_scale);
	att->position.vy = (int)(agc->location.y * local_scale);
	att->position.vz = (int)(agc->location.z * local_scale);
	
	att->shapenum = 0;

	att->startInactive=(agc->flags & AVPGENFLAG_GENERATORINACTIVE)!=0;

	att->orientation.EulerX=0;
	att->orientation.EulerY=agc->orientation & 4095;
	att->orientation.EulerZ=0;

	att->ammo=(agc->extra1<<8)+agc->extra2;
	if(att->ammo>=30000) att->ammo=0x7fffffff;
	
	//look for extra strategy stuff
	for(int i=0;i<SB_NAME_LENGTH;i++) att->death_target_ID[i]=0;
	att->death_target_request=0;

	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;
						
							*(ObjectID*)&att->death_target_ID[0]=es->DeathTarget;
							att->death_target_request=es->target_request;
						}
						break;
						
				}
			}
		}
	}


	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&att->nameID[0]=ID;
	
	AddToBehaviourList(agc->name,ID,I_BehaviourAutoGun, (void *) att,0,agc->get_alternate_locations_chunk(),agc->flags);
}	


static void add_xenoborg(AVP_Generator_Chunk * agc)
{
	
	TOOLS_DATA_XENO* tdx=(TOOLS_DATA_XENO*) PoolAllocateMem(sizeof(TOOLS_DATA_XENO));
	
	tdx->position.vx = (int)(agc->location.x * local_scale);
	tdx->position.vy = (int)(agc->location.y * local_scale);
	tdx->position.vz = (int)(agc->location.z * local_scale);

	tdx->shapeIndex = 0;

	tdx->starteuler.EulerX=0;
	tdx->starteuler.EulerY=agc->orientation & 4095;
	tdx->starteuler.EulerZ=0;

	if(agc->extra1)
		tdx->UpTime=(int)agc->extra1*5;
	else
		tdx->UpTime=20;//default value
	if(agc->extra2)
		tdx->ModuleRange=agc->extra2;
	else
		tdx->ModuleRange=7;//default value
	
	//look for extra strategy stuff
	for(int i=0;i<SB_NAME_LENGTH;i++) tdx->death_target_ID[i]=0;
	tdx->death_target_request=0;
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	AVP_Strategy_Chunk* asc=0;
	if(agedc)
	{
		Chunk * pChunk = agedc->lookup_single_child("AVPSTRAT");
		if(pChunk)
		{
			asc=(AVP_Strategy_Chunk*) pChunk;
		
			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratEnemy :
						{
							EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;
						
							*(ObjectID*)&tdx->death_target_ID[0]=es->DeathTarget;
							tdx->death_target_request=es->target_request;
						}
						break;
						
				}
			}
		}
	}


	ObjectID ID=agc->CalculateID();
	*(ObjectID*)&tdx->nameID[0]=ID;


	AddToBehaviourList(agc->name,ID, I_BehaviourXenoborg, (void *) tdx,0,agc->get_alternate_locations_chunk(),agc->flags);
	
}	

extern "C"{
extern void SetHiveParamaters(int enemytype,int max,int genpermin,int deltagenpermin,int time);
};

void setup_generators (Environment_Data_Chunk * envd)
{
	//first setup the global generator paramaters
	Chunk * pChunk = envd->lookup_single_child("GLOGENDC");
	
	int generator_enemy;
	
	if(pChunk)
	{
		Global_Generator_Data_Chunk* ggdc=(Global_Generator_Data_Chunk*)pChunk;
		switch(ggdc->EnemyGenerated)
		{
			case Generate_Aliens :
				generator_enemy=I_BehaviourAlien;
				break;
			case Generate_Marines :
				generator_enemy=I_BehaviourMarine;
				break;					
			default :
				GLOBALASSERT("Invalid enemy type"==0);
				
		}
		SetHiveParamaters(generator_enemy,ggdc->MaxNPCSOnLevel,ggdc->NPCSPerMinute,ggdc->NPCAcceleration,ggdc->HiveStateChangeTime*ONE_FIXED);

	}
	else
	{
		generator_enemy=I_BehaviourAlien;
		SetHiveParamaters(generator_enemy,25,4,2,60*ONE_FIXED);
	}
	
	
	
	Special_Objects_Chunk * soc = 0;

 	pChunk = envd->lookup_single_child ("SPECLOBJ");
	if (pChunk)
	{
		soc = (Special_Objects_Chunk *)pChunk;
	}
		// and alien generator objects
	
	if (soc)
	{
		{
			List<Chunk *> cl;
			soc->lookup_child("AVPGENER",cl);
			for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
			{
				AVP_Generator_Chunk * agc = (AVP_Generator_Chunk *)cli();
		
				if (agc->type)
				{
					
					#if 0
					if(AvP.PlayerType==I_Alien && (agc->flags & AVPGENFLAG_AVPGAMEMODEALIEN)||
					   AvP.PlayerType==I_Marine && (agc->flags & AVPGENFLAG_AVPGAMEMODEMARINE)||
					   AvP.PlayerType==I_Predator && (agc->flags & AVPGENFLAG_AVPGAMEMODEPREDATOR))
					#endif
					{
						//note : only aliens can appear in a network game
						switch (agc->type)
						{
							case 1:
								add_alien(agc);
								break;
							case 2:
								if(AvP.Network == I_No_Network)
									add_predator(agc);
								break;
							
							case 3:
								if(AvP.Network == I_No_Network)
									add_hugger(agc);
								break;
							case 4:
								if(AvP.Network == I_No_Network)
									add_xenoborg(agc);
								break;
							case 5:
								add_alien(agc);//add_alien also does predaliens
								break;
							case 6:
								if(AvP.Network == I_No_Network)
									add_queen(agc);
								break;
							case 7:
								if(AvP.Network == I_No_Network)
									add_marine(agc);
								break;
							case 8:
								add_alien(agc); //add_alien will also do pretorian guards
								break;

							case 9:
								if(AvP.Network == I_No_Network)
									add_autogun(agc); 
								break;

							default:
								
								break;
						}
						
					
					}
				}
				else
				{
					#if 0
					//check to see if generator is flagged for this game mode

					if(AvP.PlayerType==I_Alien && (agc->flags & AVPGENFLAG_AVPGAMEMODEALIEN)||
					   AvP.PlayerType==I_Marine && (agc->flags & AVPGENFLAG_AVPGAMEMODEMARINE)||
					   AvP.PlayerType==I_Predator && (agc->flags & AVPGENFLAG_AVPGAMEMODEPREDATOR))
					#endif
					//see if generator is a multiplayer start position
					if(agc->flags & AVPGENFLAG_MULTIPLAYERSTART)
					{
						continue;
					}
					

					{
						ObjectID ID=agc->CalculateID();
					
						
						GENERATOR_BLOCK* tdg = (GENERATOR_BLOCK*) PoolAllocateMem(sizeof(GENERATOR_BLOCK));
						memset(tdg,0,sizeof(GENERATOR_BLOCK));
						
						tdg->Position.vx = (int)(agc->location.x * local_scale);
						tdg->Position.vy = (int)(agc->location.y * local_scale);
						tdg->Position.vz = (int)(agc->location.z * local_scale);
						tdg->Active=!(agc->flags & AVPGENFLAG_GENERATORINACTIVE);

						tdg->GenerationRate=1;
						tdg->GenerationRateIncrease=0;
						tdg->use_own_rate_values=0;

						tdg->MaxGenNPCs=0;
						tdg->use_own_max_npc=0;

						tdg->path=-1;
						tdg->stepnumber=-1;
						
						AVP_Generator_Extended_Settings_Chunk* setting=agc->get_extended_settings();
						if(setting)
						{
							/*The weightings are in the same order in generator_block and avp_generator_weighting,
							and I can't be bothered to copy them individually*/
							int* copyfrom = (int*) &setting->weights->PulseMarine_Wt;
							int* copyto =  (int*) &tdg->PulseMarine_Wt;

							for(int i=0;i<15;i++)
							{
								copyto[i]=copyfrom[i];
								tdg->WeightingTotal+=copyto[i];
							}

							if(agc->flags & AVPGENFLAG_USEOWNRATE)
							{
								tdg->GenerationRate=setting->GenerationRate;
								tdg->GenerationRateIncrease=setting->GenRateIncrease;
								tdg->use_own_rate_values=1;
							}
							if(agc->flags & AVPGENFLAG_USEOWNLIMIT)
							{
								tdg->MaxGenNPCs=setting->GenLimit;
								tdg->use_own_max_npc=1;
							}
										
						}

						if(tdg->WeightingTotal==0)
						{
							//set genrator to generate level's default creature
							if(generator_enemy==I_BehaviourAlien)
							{
								tdg->Alien_Wt=1;
								tdg->WeightingTotal=1;
							}
							else
							{
								tdg->PulseMarine_Wt=1;
								tdg->WeightingTotal=1;
							}

						}


						/*See if generator has extra strategy data, currently just used for producing path following creatures*/

						AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	
						if(agedc)
						{
							AVP_Strategy_Chunk* asc = (AVP_Strategy_Chunk*) agedc->lookup_single_child("AVPSTRAT");
							if(asc)
							{
							
								if(asc->Strategy)
								{
									switch(asc->Strategy->StrategyType)
									{
										case StratGenerator :
											{
											    GeneratorStrategy* gs=(GeneratorStrategy*)asc->Strategy;

												if(gs->MissionType==1)
												{
													tdg->path=gs->ExtraMissionData &0xffff;
													tdg->stepnumber=ConvertObjectIndexToPathIndex(tdg->path,gs->ExtraMissionData>>16);
													if(tdg->stepnumber==-1)
													{
														LOGDXFMT(("Error setting up path for generator %s\n",agc->name));	
														GLOBALASSERT(0);
													}
												}
												
													
											}
											break;
									}
								}
							}
						}
						
					
						AddToBehaviourList(agc->name,ID, I_BehaviourGenerator, (void *) tdg,0,agc->get_alternate_locations_chunk(),agc->flags);
					}
				}
				
			}
		}
		
		if(AvP.Network != I_No_Network)
		{
			//setup multiplayer start positions
			numMarineStartPos=0;
			numAlienStartPos=0;
			numPredatorStartPos=0;

			marineStartPositions=0;
			alienStartPositions=0;
			predatorStartPositions=0;

			//go through the list to find the number of each type of start location
			List<Chunk *> cl;
			soc->lookup_child("AVPGENER",cl);
			
			LIF<Chunk *> cli(&cl);
			for (; !cli.done(); cli.next())
			{
				AVP_Generator_Chunk * agc = (AVP_Generator_Chunk *)cli();
				if(agc->type) continue;
				if(!(agc->flags & AVPGENFLAG_MULTIPLAYERSTART))
				{
					continue;
				}


				if(agc->flags & AVPGENFLAG_AVPGAMEMODEALIEN) numAlienStartPos++;
				if(agc->flags & AVPGENFLAG_AVPGAMEMODEMARINE) numMarineStartPos++;
				if(agc->flags & AVPGENFLAG_AVPGAMEMODEPREDATOR) numPredatorStartPos++;
			}

			if(numMarineStartPos)
			{
				marineStartPositions=(MULTIPLAYER_START*) PoolAllocateMem(sizeof(MULTIPLAYER_START)*numMarineStartPos);
			}
			if(numPredatorStartPos)
			{
				predatorStartPositions=(MULTIPLAYER_START*) PoolAllocateMem(sizeof(MULTIPLAYER_START)*numPredatorStartPos);
			}
			if(numAlienStartPos)
			{
				alienStartPositions=(MULTIPLAYER_START*) PoolAllocateMem(sizeof(MULTIPLAYER_START)*numAlienStartPos);
			}
			
			int mpos=0;
			int apos=0;
			int ppos=0;

			//go through the list a second time setting up the positions
			for(cli.restart();!cli.done();cli.next())
			{
				AVP_Generator_Chunk * agc = (AVP_Generator_Chunk *)cli();
				if(agc->type) continue;
				if(!(agc->flags & AVPGENFLAG_MULTIPLAYERSTART))
				{
					continue;
				}

				MULTIPLAYER_START start_pos;
				start_pos.location.vx = (int)(agc->location.x * local_scale);
				start_pos.location.vy = (int)(agc->location.y * local_scale);
				start_pos.location.vz = (int)(agc->location.z * local_scale);

				start_pos.orientation.EulerX=0;
				start_pos.orientation.EulerY=agc->orientation & 4095;
				start_pos.orientation.EulerZ=0;
				
				if(agc->flags & AVPGENFLAG_AVPGAMEMODEALIEN) alienStartPositions[apos++]=start_pos;
				if(agc->flags & AVPGENFLAG_AVPGAMEMODEMARINE) marineStartPositions[mpos++]=start_pos;
				if(agc->flags & AVPGENFLAG_AVPGAMEMODEPREDATOR) predatorStartPositions[ppos++]=start_pos;
			}
		}
	}
	
}

typedef struct mission_setup 
{
	AVP_Strategy_Chunk* asc;
	int order_number;
	ObjectID ID;
	MissionObjective* mission;
}MISSION_SETUP;

static List<MISSION_SETUP *> mos_list;

static void add_mission_to_list(AVP_Strategy_Chunk * asc,ObjectID id)
{
	MISSION_SETUP* ms=new MISSION_SETUP;

	ms->asc=asc;
	ms->ID=id;
	ms->mission=0;
	
	MissionObjectiveStrategy * mos = (MissionObjectiveStrategy *)asc->Strategy;
	ms->order_number=mos->mission_number;

	for(LIF<MISSION_SETUP*> mlif(&mos_list);!mlif.done();mlif.next())
	{
		if(mlif()->order_number>ms->order_number)
		{
			mos_list.add_entry_before(ms,mlif());
			return;
		}
	}
	mos_list.add_entry(ms);
}

void SetupMissionObjectives()
{
	LIF<MISSION_SETUP*> mlif(&mos_list);
	
	for(; !mlif.done(); mlif.next())
	{
		MissionObjectiveStrategy * mos=(MissionObjectiveStrategy*) mlif()->asc->Strategy;
		assert (mos->mission_description_string>=0 && mos->mission_description_string<450);
		assert (mos->mission_complete_string>=0 && mos->mission_complete_string<450);
	
		TEXTSTRING_ID desc_string;
		if(mos->mission_description_string==0)
			desc_string=TEXTSTRING_BLANK;
		else
			desc_string=(TEXTSTRING_ID)((mos->mission_description_string-1)+TEXTSTRING_LEVELMSG_001);
		
		TEXTSTRING_ID complete_string;
		if(mos->mission_complete_string==0)
			complete_string=TEXTSTRING_BLANK;	
		else
			complete_string=(TEXTSTRING_ID)((mos->mission_complete_string-1)+TEXTSTRING_LEVELMSG_001);

		MissionObjectiveState m_state;
		
		if(mos->flags & MissionFlag_Visible)
		{
			if (mos->flags & MissionFlag_CurrentlyPossible)
		 		m_state=MOS_VisibleUnachieved;
			else
		 		m_state=MOS_VisibleUnachievedNotPossible;
		}
		else if (mos->flags & MissionFlag_CurrentlyPossible)
		{
			m_state=MOS_HiddenUnachieved;
		}
		else
		  	m_state=MOS_HiddenUnachievedNotPossible;
		
		MissionObjective* mission;
		
		/* KJL 22:50:21 02/08/98 - set any additional effect of completing the mission */
		switch(mos->mission_completion_effect)
		{
			default: // fall through to 'no effect' if effect not yet catered for
			case MCE_None:
				mission = new MissionObjective(desc_string,m_state,complete_string,MissionFX_None);
				break;
			case MCE_CompleteLevel:
				mission = new MissionObjective(desc_string,m_state,complete_string,MissionFX_CompletesLevel);
				break;
		}
		
		mlif()->mission=mission;

		MISSION_COMPLETE_TOOLS_TEMPLATE* mctt=(MISSION_COMPLETE_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(MISSION_COMPLETE_TOOLS_TEMPLATE));
		mctt->mission_objective_ptr=mission;
		*(ObjectID*)&mctt->nameID[0]=mlif()->ID;

		AddToBehaviourList(0,mlif()->ID, I_BehaviourMissionComplete, (void*)mctt,-1);
	}

	//now set up the mission alterations
	for(mlif.restart();!mlif.done();mlif.next())
	{
		MissionObjectiveStrategy * mos=(MissionObjectiveStrategy*) mlif()->asc->Strategy;
		
		for(int i=0;i<mos->num_mission_targets;i++)
		{
			//find the mission referred to
			for(LIF<MISSION_SETUP*> mlif2(&mos_list);!mlif2.done();mlif2.next())
			{
				if(mlif2()->ID==mos->mission_targets->target_mission)
				{
					mlif()->mission->AddMissionAlteration(mlif2()->mission,mos->mission_targets->effect_on_target);
					break;
				}
			}
						
		}
		
	}

	while(mos_list.size())
	{
		delete mos_list.first_entry();
		mos_list.delete_first_entry();
	}
}



void setup_light_data (LIGHTBLOCK * lPtr, Light_Chunk * lc)
{
	/* KJL 14:42:42 20/02/98 - this function has been updated;
	many things have been removed from the lightblock structure */
	
	lPtr->LightFlags = lc->light.engine_light_flags | LFlag_PreLitSource
										 | LFlag_AbsPos | LFlag_WasNotAllocated 
										 | LFlag_AbsLightDir | LFlag_CosAtten; // Forcing attenuation

	lPtr->LightType = LightType_PerVertex;

	lPtr->LightWorld.vx = (int)(lc->light.location.x * local_scale);
	lPtr->LightWorld.vy = (int)(lc->light.location.y * local_scale);
	lPtr->LightWorld.vz = (int)(lc->light.location.z * local_scale);
	
	lPtr->LightBright = (int)(lc->light.brightness * 1.0);
	lPtr->LightBrightStore = (int)(lc->light.brightness * 1.0);

	/* KJL 10:57:57 9/24/97 - colour scales - these take the values 0 to 65536 */
	lPtr->RedScale = ((lc->light.colour>>16)&255)*257;
	lPtr->GreenScale = ((lc->light.colour>>8)&255)*257;
	lPtr->BlueScale = ((lc->light.colour)&255)*257;

	lPtr->LightRange = (int)((lc->light.range * 1.0) * local_scale);

	if (lc->light.local_light_flags & LOFlag_NoPreLight)
	{
		lPtr->LightFlags &= ~LFlag_PreLitSource;
	}
	
}

void setup_placed_light_data (LIGHTBLOCK * lPtr, Placed_Object_Light_Chunk * lc)
{
	/* KJL 14:42:42 20/02/98 - this function has been updated;
	many things have been removed from the lightblock structure */
	
	lPtr->LightFlags = lc->light.engine_light_flags | LFlag_WasNotAllocated 
										 | LFlag_AbsLightDir | LFlag_CosAtten; // Forcing attenuation

	if(lc->light.flags & PlacedLightFlag_NoSpecular)
	{
		lPtr->LightFlags|=LFlag_NoSpecular;
	}

	lPtr->LightType = LightType_PerVertex;

	lPtr->LightWorld.vx = 0;
	lPtr->LightWorld.vy = 0;
	lPtr->LightWorld.vz = 0;
	
	lPtr->LightBright = (int)(lc->light.brightness * 1.0);
	lPtr->LightBrightStore = (int)(lc->light.brightness * 1.0);

	/* KJL 10:57:57 9/24/97 - colour scales - these take the values 0 to 65536 */
	lPtr->RedScale = ((lc->light.up_colour>>16)&255)*257;
	lPtr->GreenScale = ((lc->light.up_colour>>8)&255)*257;
	lPtr->BlueScale = ((lc->light.up_colour)&255)*257;

	lPtr->LightRange = (int)((lc->light.range * 1.0) * local_scale);
	
}

void SetUpRunTimeLights ()
{
	// go through the list of modules adding lights
	
	Environment_Data_Chunk * edc = 0;
	Light_Set_Chunk * lsc = 0;

	Chunk * pChunk = Env_Chunk->lookup_single_child("REBENVDT");
	
	if (pChunk)
	{
		edc = (Environment_Data_Chunk *)pChunk;
		
		List<Chunk *> cl;
		edc->lookup_child("LIGHTSET",cl);
		
		while (cl.size())
		{
			Light_Set_Chunk * ls = (Light_Set_Chunk *)cl.first_entry();
			
			Chunk * pChunk2 = ls->lookup_single_child("LTSETHDR");
			if (pChunk2)
			{
				if (!strncmp(light_set_name, ((Light_Set_Header_Chunk *)pChunk2)->light_set_name, 8))
				{
					lsc = ls;
					break;
				}
			}
			
			cl.delete_first_entry();
		}
		
	}
		
	if (!lsc)
	{
		return;
	}

	pChunk = lsc->lookup_single_child("AMBIENCE");
	
	if (pChunk)
	{
		GlobalAmbience = (chnk_playcam_vdb.SVDB_Ambience = ((Lighting_Ambience_Chunk *)pChunk)->ambience);

 		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

		if (VDBPtr)
		{
			VDBPtr->VDB_Ambience = GlobalAmbience;
		}
	}

	List<Chunk *> cl;
	lsc->lookup_child ("STDLIGHT",cl);
	LIF<Chunk *> cli(&cl);

	
	MODULE ** m_arrayPtr = MainScene.sm_marray;
	
	while (*m_arrayPtr)
	{
		List<Light_Chunk *> lights_for_this_module;
		
		MODULE * this_mod = *m_arrayPtr++;
		
		if (this_mod->m_flags & m_flag_infinite)
		{
			continue;
		}
		
		for (cli.restart(); !cli.done(); cli.next())
		{
			Light_Chunk * lc = (Light_Chunk *)cli();

			// try to throw away light
			if (!(lc->light.local_light_flags & LOFlag_Runtime))
			{
				continue;
			}
			
			if (lc->light_added_to_module)
			{
				continue;
			}
			
			if ((lc->light.location.x * local_scale) > (this_mod->m_world.vx + this_mod->m_maxx))
				continue;
			
			if ((lc->light.location.x * local_scale) < (this_mod->m_world.vx + this_mod->m_minx))
				continue;
			
			if ((lc->light.location.y * local_scale) > (this_mod->m_world.vy + this_mod->m_maxy))
				continue;
			
			if ((lc->light.location.y * local_scale) < (this_mod->m_world.vy + this_mod->m_miny))
				continue;
			
			if ((lc->light.location.z * local_scale) > (this_mod->m_world.vz + this_mod->m_maxz))
				continue;
			
			if ((lc->light.location.z * local_scale) < (this_mod->m_world.vz + this_mod->m_minz))
				continue;

			// light in module
				
			lc->light_added_to_module = TRUE;
			
			lights_for_this_module.add_entry(lc);
			
		}
		
		this_mod->m_numlights = lights_for_this_module.size();

		assert (lights_for_this_module.size() <= MaxObjectLights);
		
		if (lights_for_this_module.size())
		{
			this_mod->m_lightarray = (LIGHTBLOCK *) PoolAllocateMem (sizeof(LIGHTBLOCK)*(lights_for_this_module.size()));
			if (!this_mod->m_lightarray)
			{
				memoryInitialisationFailure = 1;
				return;
			}

			LIGHTBLOCK * lPtr = this_mod->m_lightarray;
			
			for (LIF<Light_Chunk *> lci(&lights_for_this_module); !lci.done(); lci.next(), lPtr++)
			{
				setup_light_data (lPtr, lci());
			}
		}
		else
		{
			this_mod->m_lightarray = 0;
		}
	}
}
#if 0
void SetupExternalLift(AVP_External_Strategy_Chunk* aesc)
{
	LiftStrategy* ls=(LiftStrategy*)aesc->Strategy;

	ObjectID ControlID={0x7fffffff,0x7fffffff};
	for(int i=0;i<ls->NumExternalLifts;i++)
	{
		if(ls->ExternalLifts[i].EnvNum==aesc->ThisEnvNum)
		{
	  		ControlID=Minimum(ControlID,ls->ExternalLifts[i].LiftID);
		}
	}
	
	
	LIFT_TOOLS_TEMPLATE * ltt = new LIFT_TOOLS_TEMPLATE;
	
	*((ObjectID *)ltt->my_module_name) = aesc->ObjID;
	*((ObjectID *)ltt->call_switch_name) = ls->AssocCallSwitch;
	*((ObjectID *)ltt->lift_door_name) = ls->AssocDoor;
	*((ObjectID *)ltt->lift_floor_switch_name) = ls->AssocFloorSwitch;

	ltt->environment = aesc->ExtEnvNum-1;
	
	ltt->num_floor = ls->Floor;

	*((ObjectID *)ltt->control_sb_name)=ControlID;
	
	ltt->controller = 0;
	
	ltt->num_stations = ls->NumAssocLifts + ls->NumExternalLifts+1;

	ltt->lift_flags=ls->LiftFlags;
	
	*((ObjectID *)ltt->nameID) = aesc->ObjID;

	ltt->orient=ls->Facing;
	if((ls->LiftFlags & LiftFlag_Airlock) || (ls->LiftFlags & LiftFlag_ExitOtherSide))
		ltt->orient=(ltt->orient+2) % 4;

	ltt_list.add_entry(ltt);	
}
#endif

void DealWithExternalObjectStategies (Environment_Data_Chunk * envd)
{
	List<Chunk*> chlist;
	#if 0
	envd->lookup_child("AVPEXSTR",chlist);
	for(LIF<Chunk*> slif(&chlist);!slif.done();slif.next())
	{
		AVP_External_Strategy_Chunk* aesc=(AVP_External_Strategy_Chunk*) slif();
		if(aesc->Strategy)
		{
			switch(aesc->Strategy->StrategyType)
			{
				case StratLift :
					SetupExternalLift(aesc);
					break;
			}
		}
	}
	#endif

	Chunk * pChunk = envd->lookup_single_child("SPECLOBJ");
	if(pChunk)
	{
		List<Chunk *> chlist;
		((Chunk_With_Children*)pChunk)->lookup_child("VIOBJECT",chlist);
		for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
		{
			Virtual_Object_Chunk* voc=(Virtual_Object_Chunk*)chlif();
			Chunk * pChunk2=voc->lookup_single_child("AVPSTRAT");
			if(!pChunk2)continue;
			AVP_Strategy_Chunk* asc=(AVP_Strategy_Chunk*)pChunk2;
			
			pChunk2=voc->lookup_single_child("VOBJPROP");
			if(!pChunk2)continue;
			Virtual_Object_Properties_Chunk* vopc=(Virtual_Object_Properties_Chunk*)pChunk2;

			if(asc->Strategy)
			{
				switch(asc->Strategy->StrategyType)
				{
					case StratAreaSwitch: //(Area Switch derived from Multi Switch)
					case StratMultiSwitch:
						add_multiswitch (vopc->name,asc,vopc->ID);
						break;
					case StratBinSwitch:
						add_binswitch (vopc->name,asc,vopc->ID);
						break;
					case StratLinkSwitch:
						add_linkswitch (vopc->name,asc,vopc->ID);
						break;

					case StratMessage:
						add_message_strategy (vopc->name,asc,vopc->ID);
						break;

					case StratMissionObjective :
						add_mission_to_list(asc,vopc->ID);
						break;

					case StratDeathVolume :
						add_deathvolume(vopc->name,asc,vopc->ID);
						break;

					case StratSelfDestruct :
						add_selfdestruct(vopc->name,asc,vopc->ID);
						break;
				}
			}
		}
	}
	SetupMissionObjectives();
}


void setup_sounds (Environment_Data_Chunk * envd)
{
	Special_Objects_Chunk * soc = 0;

 	Chunk * pChunk = envd->lookup_single_child ("SPECLOBJ");
	if (pChunk)
	{
		soc = (Special_Objects_Chunk *)pChunk;
	}
	
	if (soc)
	{
		List<Chunk *> cl;
		soc->lookup_child("SOUNDOB2",cl);
		for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
		{
			Sound_Object_Chunk * snd = (Sound_Object_Chunk *) cli();
			SOUND_TOOLS_TEMPLATE* stt =(SOUND_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(SOUND_TOOLS_TEMPLATE));
			
			stt->position.vx = (int)(snd->position.x * local_scale);
			stt->position.vy = (int)(snd->position.y * local_scale);
			stt->position.vz = (int)(snd->position.z * local_scale);
			
			stt->inner_range = (unsigned)(snd->inner_range * local_scale);
			stt->outer_range = (unsigned)(snd->outer_range * local_scale);
			stt->max_volume = snd->max_volume;
			stt->pitch = snd->pitch;
			
			stt->playing = 0;
			stt->loop = 0;

			if(snd->flags & SoundObjectFlag_NotPlayingAtStart)
				stt->playing=0;
			else
				stt->playing=1;
			
			if(snd->flags & SoundObjectFlag_NoLoop)
				stt->loop=0;
			else
				stt->loop=1;

			stt->sound_name =(char*) PoolAllocateMem(strlen (snd->wav_name) + 1);
			strcpy(stt->sound_name,snd->wav_name);
			stt->sound_loaded=GetSoundForMainRif(snd->wav_name);
					
			AddToBehaviourList(snd->snd_name,snd->CalculateID(), I_BehaviourPlacedSound, (void *) stt);
		}
	}
}


void setup_cables(Environment_Data_Chunk * envd)
{
	if(!envd) return;
	Special_Objects_Chunk* soc = (Special_Objects_Chunk*) envd->lookup_single_child("SPECLOBJ");
	if(!soc) return;
	List<Chunk*> chlist;
	soc->lookup_child("AVPCABLE",chlist);

	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		AVP_Power_Cable_Chunk* appc=(AVP_Power_Cable_Chunk*)chlif();

		POWER_CABLE_TOOLS_TEMPLATE* tdpc=(POWER_CABLE_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(POWER_CABLE_TOOLS_TEMPLATE));

		tdpc->position.vx = (int)(appc->location.x * local_scale);
		tdpc->position.vy = (int)(appc->location.y * local_scale);
		tdpc->position.vz = (int)(appc->location.z * local_scale);

		if(appc->flags & PowerCableFlag_UseDefaultSettings)
		{
			tdpc->max_charge=90*ONE_FIXED;
			tdpc->current_charge=90*ONE_FIXED;
			tdpc->recharge_rate=ONE_FIXED;
		}
		else
		{
			tdpc->max_charge=appc->max_charge;
			tdpc->current_charge=appc->initial_charge;
			tdpc->recharge_rate=appc->recharge_rate;
		}
		*(ObjectID*)&tdpc->nameID[0]=appc->id;
		
		AddToBehaviourList(0,appc->id, I_BehaviourPowerCable, (void *) tdpc,0);
		
	}
}

void setup_particle_generators(Environment_Data_Chunk * envd)
{
	if(!envd) return;
	Special_Objects_Chunk* soc = (Special_Objects_Chunk*) envd->lookup_single_child("SPECLOBJ");
	if(!soc) return;
	
	List<Chunk*> chlist;
	soc->lookup_child("PARGENER",chlist);

	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		
		AVP_Particle_Generator_Chunk* part_chunk=(AVP_Particle_Generator_Chunk*)chlif();
		AVP_Particle_Generator_Data_Chunk* data_chunk=(AVP_Particle_Generator_Data_Chunk*)part_chunk->get_data_chunk();
		GLOBALASSERT(data_chunk);

		PARTICLE_GENERATOR_TOOLS_TEMPLATE* part_temp=(PARTICLE_GENERATOR_TOOLS_TEMPLATE*) PoolAllocateMem(sizeof(PARTICLE_GENERATOR_TOOLS_TEMPLATE));

		part_temp->position.vx = (int)(data_chunk->location.x * local_scale);
		part_temp->position.vy = (int)(data_chunk->location.y * local_scale);
		part_temp->position.vz = (int)(data_chunk->location.z * local_scale);

		*(ObjectID*)&part_temp->nameID[0]=data_chunk->id;
		*(ObjectID*)&part_temp->parentID[0]=data_chunk->parent_id;

		part_temp->type=(enum particle_generator_type)data_chunk->type;

		part_temp->probability=(data_chunk->probability*ONE_FIXED)/100;
		part_temp->speed=data_chunk->speed*10;
		
		if(data_chunk->type==PARGEN_TYPE_SPARK)
			part_temp->frequency=(data_chunk->time*ONE_FIXED)/10;
		else
			part_temp->frequency=ONE_FIXED/max(data_chunk->quantity,1);

		part_temp->active=!(data_chunk->flags & ParticleGeneratorFlag_Inactive);

		{
			//get direction vector from orientation
			QUAT q;
			q.quatx = (int) -(data_chunk->orientation.x*ONE_FIXED);
			q.quaty = (int) -(data_chunk->orientation.y*ONE_FIXED);
			q.quatz = (int) -(data_chunk->orientation.z*ONE_FIXED);
			q.quatw = (int)  (data_chunk->orientation.w*ONE_FIXED);

			MATRIXCH m;
			QuatToMat (&q, &m);

			part_temp->orientation=m;
		
		}

		setup_track_sound(part_chunk->get_sound_chunk(),&part_temp->sound);
			
		AddToBehaviourList(data_chunk->name,data_chunk->id, I_BehaviourParticleGenerator, (void *) part_temp,0,part_chunk->get_alternate_locations_chunk());
		
	}
}

static void get_marine_facing_point(VECTORCH& pos,EULER& euler,VECTORCH& facing_point)
{
	//select point 2 metres(ish) in front of marine
	MATRIXCH mat;
	CreateEulerMatrix(&euler,&mat);
	facing_point=pos;
	facing_point.vx+=mat.mat13/30;
	facing_point.vy+=mat.mat23/30;
	facing_point.vz+=mat.mat33/30;
}
