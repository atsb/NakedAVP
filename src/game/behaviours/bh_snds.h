#ifndef _bh_snds_h
#define _bh_snds_h

#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "jsndsup.h"

#ifdef __cplusplus

	extern "C" {

#endif

typedef struct sound_tools_template
{
	VECTORCH position;

	unsigned long inner_range;
	unsigned long outer_range;
	
	unsigned long max_volume;
	unsigned long pitch;
	
	unsigned int playing :1;
	unsigned int loop :1;

	char * sound_name;
	LOADED_SOUND const * sound_loaded;

} SOUND_TOOLS_TEMPLATE;


typedef struct sound_behav_block
{
	VECTORCH position;
	
	unsigned long inner_range;
	unsigned long outer_range;
	int max_volume;
	int	pitch;
	
	int activ_no;
	
	char * wav_name;
	
// sound management stuff
	
	LOADED_SOUND const * sound_loaded;
	
	BOOL sound_not_started;

	unsigned int playing :1;
	unsigned int loop :1;
	
} SOUND_BEHAV_BLOCK;

void * SoundBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
void SoundBehaveFun (STRATEGYBLOCK * );
void SoundBehaveDestroy (STRATEGYBLOCK * sbptr);

void StartPlacedSoundPlaying(STRATEGYBLOCK* sbptr);
void StopPlacedSoundPlaying(STRATEGYBLOCK* sbptr);


#ifdef __cplusplus

	}; // end of extern "c"

#endif

#endif
