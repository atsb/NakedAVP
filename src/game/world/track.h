#ifndef track_h
#define track_h 1

#ifdef __cplusplus
extern "C" {
#endif

struct loaded_sound;

typedef struct track_sound
{
	unsigned long inner_range;
	unsigned long outer_range;
	int max_volume;
	int	pitch;
	
	struct loaded_sound const * sound_loaded;
	int activ_no;
	int time_left;

	unsigned int loop:1;
	unsigned int playing:1; //sound should be playing (if in range)

} TRACK_SOUND;

typedef struct track_section_data
{
	QUAT quat_start;	//orientations of object relative to pivot
	QUAT quat_end;
	
	VECTORCH pivot_start;		//pivot start pos in world space
	VECTORCH pivot_travel;		

	VECTORCH object_offset; //from pivot
	
	int time_for_section; //time take for this section in fixed point seconds
	
	//preprocessed stuff
	int omega;
	int oneoversinomega;
	int oneovertime;	
	


	/* KJL 12:25:51 24/03/98 - quaternions added to handle spining of quaternions over track */
	QUAT quat_prev;
	QUAT quat_start_control;
	QUAT quat_end_control;
	
	/* KJL 12:25:37 24/03/98 - data points used to create a smooth curve from the 3d keyframes */
	VECTORCH pivot_0;
	VECTORCH pivot_1;
	VECTORCH pivot_2;
	VECTORCH pivot_3;

} TRACK_SECTION_DATA;

typedef struct track_controller
{
	STRATEGYBLOCK* sbptr;
	
	int num_sections;
	TRACK_SECTION_DATA* sections;

	int timer;
	int speed_mult;
	int current_section;

	TRACK_SOUND* sound;
	TRACK_SOUND* start_sound;
	TRACK_SOUND* end_sound;

	int initial_state_timer; //used by Reset_Track()

	unsigned int playing:1;
	unsigned int reverse:1;
	unsigned int no_rotation:1;
	unsigned int loop:1;
	unsigned int loop_backandforth:1;
	unsigned int use_speed_mult:1;
	unsigned int use_smoothing:1;
	unsigned int playing_start_sound:1;
	unsigned int initial_state_playing:1; //used by Reset_Track()
	unsigned int initial_state_reverse:1; //used by Reset_Track()



}TRACK_CONTROLLER;

void Update_Track_Position_Only(TRACK_CONTROLLER* tc); //just moves track to correspond with current timer setting
void Update_Track_Position(TRACK_CONTROLLER* tc);
void Preprocess_Track_Controller(TRACK_CONTROLLER* tc);

void Start_Track_Playing(TRACK_CONTROLLER* tc);
void Stop_Track_Playing(TRACK_CONTROLLER* tc);

void Reset_Track(TRACK_CONTROLLER* tc); //called when restarting a level

void Deallocate_Track(TRACK_CONTROLLER* tc);

/*the track_sound stuff doesn't actually refer to the track directly , so
it can be used by other things*/
void Start_Track_Sound(TRACK_SOUND* ts,VECTORCH * location); 
void Stop_Track_Sound(TRACK_SOUND* ts);
void Update_Track_Sound(TRACK_SOUND* ts,VECTORCH * location);
void Deallocate_Track_Sound(TRACK_SOUND* ts);

struct save_block_header;

void LoadTrackPosition(struct save_block_header*,TRACK_CONTROLLER*);
void SaveTrackPosition(TRACK_CONTROLLER*);

#ifdef __cplusplus
}
#endif

#endif
