#ifndef _bh_plachier_h_
#define _bh_plachier_h_ 1

#include "bh_track.h" //for the special_track_point structure

#ifdef __cplusplus

	extern "C" {

#endif


typedef struct placed_hierarchy_sound
{
	unsigned long inner_range;
	unsigned long outer_range;
	int max_volume;
	int	pitch;
	
	struct loaded_sound const * sound_loaded;
	int activ_no;

	unsigned int loop:1;
	unsigned int playing:1; //sound should be playing (if in range)
}PLACED_HIERARCHY_SOUND;

typedef struct placed_hierarchy_sound_times
{
	int start_time;
	int end_time;
	PLACED_HIERARCHY_SOUND* sound;
}PLACED_HIERARCHY_SOUND_TIMES;

typedef struct placed_hierarchy_sequence
{
	int sequence_no;
	int sub_sequence_no;
	int time;
	unsigned int loop:1;

	int num_sound_times;
	PLACED_HIERARCHY_SOUND_TIMES* sound_times;
}PLACED_HIERARCHY_SEQUENCE;


typedef struct placed_hierarchy_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;

	HMODELCONTROLLER HModelController;
	
	PLACED_HIERARCHY_SEQUENCE* current_seq;
	int num_sequences;
	PLACED_HIERARCHY_SEQUENCE* sequences;

	VECTORCH* sound_location;

	int num_sounds;
	PLACED_HIERARCHY_SOUND* sounds;


	int num_special_track_points;
	SPECIAL_TRACK_POINT* special_track_points;

}PLACED_HIERARCHY_BEHAV_BLOCK;


typedef struct placed_hierarchy_tools_template
{
	char nameID[SB_NAME_LENGTH];

	VECTORCH position;
	EULER orientation;

	int num_sequences;
	PLACED_HIERARCHY_SEQUENCE* sequences;
	PLACED_HIERARCHY_SEQUENCE* first_sequence;
	BOOL playing;

	int num_sounds;
	PLACED_HIERARCHY_SOUND* sounds;

	int num_special_track_points;
	SPECIAL_TRACK_POINT* special_track_points;

	const char* file_name;
	const char* hier_name;
}PLACED_HIERARCHY_TOOLS_TEMPLATE;

extern void* PlacedHierarchyBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr);
void PlacedHierarchyBehaveFun(STRATEGYBLOCK* sbptr);
void MakePlacedHierarchyNear(STRATEGYBLOCK* sbptr);
void DeletePlacedHierarchy(PLACED_HIERARCHY_BEHAV_BLOCK*);
void SendRequestToPlacedHierarchy(STRATEGYBLOCK* sbptr,BOOL state,int extended_data);


#ifdef __cplusplus

	};

#endif


#endif
