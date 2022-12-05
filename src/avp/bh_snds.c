#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_snds.h"
#include "dynblock.h"
#include "dynamics.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "dxlog.h"

#include "psndplat.h"
#include "jsndsup.h"


void * SoundBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	SOUND_BEHAV_BLOCK * sbb = 0;
	SOUND_TOOLS_TEMPLATE * stt = (SOUND_TOOLS_TEMPLATE *)bhdata;

	GLOBALASSERT (sbptr && stt);
	
	sbb = (SOUND_BEHAV_BLOCK *)AllocateMem (sizeof (SOUND_BEHAV_BLOCK));
	
	GLOBALASSERT (sbb);
	
	sbb->sound_loaded = stt->sound_loaded;

	sbb->position = stt->position;
	sbb->inner_range = stt->inner_range;
	sbb->outer_range = stt->outer_range;
	sbb->max_volume = stt->max_volume;
	sbb->pitch = stt->pitch;
		
	sbb->sound_not_started = 1;
	
	sbb->wav_name = (char *) AllocateMem (strlen (stt->sound_name) + 1);
	strcpy (sbb->wav_name, stt->sound_name);

	sbb->playing=stt->playing;
	sbb->loop=stt->loop;

	sbb->activ_no = SOUND_NOACTIVEINDEX;
	
	return((void*)sbb);
	
}

void SoundBehaveDestroy (STRATEGYBLOCK * sbptr)
{
	SOUND_BEHAV_BLOCK * sbb = (SOUND_BEHAV_BLOCK*)sbptr->SBdataptr;
	
	if (sbb->activ_no != SOUND_NOACTIVEINDEX)
	{
		Sound_Stop (sbb->activ_no);
	}
	if (sbb->wav_name)
	{
		DeallocateMem (sbb->wav_name);
	}
}
void SoundBehaveFun (STRATEGYBLOCK * sbptr)
{
	SOUND_BEHAV_BLOCK * sbb = (SOUND_BEHAV_BLOCK*)sbptr->SBdataptr;
	

	if(!sbb->sound_loaded) return;
	if (sbb->sound_not_started && Player)
	{
		SOUND3DDATA s3d;
	
		if(sbb->playing)
		{
			//start playing the sound , but only if it is close enough
			int dist=VectorDistance(&Player->ObWorld,&sbb->position);
			if(dist<=sbb->outer_range)
			{

				s3d.position = sbb->position;
				s3d.inner_range = sbb->inner_range;
				s3d.outer_range = sbb->outer_range;
				s3d.velocity.vx = 0;
				s3d.velocity.vy = 0;
				s3d.velocity.vz = 0;
		
				if(sbb->loop)
					Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "nelh", &s3d, &sbb->activ_no);
				else
					Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "neh", &s3d, &sbb->activ_no);

				if (sbb->activ_no != SOUND_NOACTIVEINDEX)
				{
					Sound_ChangeVolume (sbb->activ_no, sbb->max_volume);
					Sound_ChangePitch (sbb->activ_no, sbb->pitch);
				}
			}
		}
		sbb->sound_not_started = 0;
	}
	
	/* KJL 14:15:39 12/8/97 - kill any objects that are too far away */
	if(sbb->playing)
	{
		int dist=VectorDistance(&Player->ObWorld,&sbb->position);
		if(dist>sbb->outer_range)
		{
			//stop sound if it is playing
		
			if (sbb->activ_no != SOUND_NOACTIVEINDEX)
			{
				Sound_Stop (sbb->activ_no);
			}
			if(!sbb->loop && sbb->playing)
			{
				//not much point in restarting a non-looping sound, since it won't restart at the right place
				sbb->playing=0;
			}
			return;
		}
	}

	if (sbb->activ_no == SOUND_NOACTIVEINDEX && sbb->sound_loaded && sbb->playing)
	{
		if(sbb->loop)
		{
			SOUND3DDATA s3d;

			s3d.position = sbb->position;
			s3d.inner_range = sbb->inner_range;
			s3d.outer_range = sbb->outer_range;
			s3d.velocity.vx = 0;
			s3d.velocity.vy = 0;
			s3d.velocity.vz = 0;

			if(sbb->loop)
				Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "nelh", &s3d, &sbb->activ_no);
			else
				Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "neh", &s3d, &sbb->activ_no);

			if (sbb->activ_no != SOUND_NOACTIVEINDEX)
			{
				Sound_ChangeVolume (sbb->activ_no, sbb->max_volume);
				Sound_ChangePitch (sbb->activ_no, sbb->pitch);
			}
		}
		else //sound has finished playing
		{
			sbb->playing=0;
		}
	}

//	// hack hack hack fixme fixme fix me
//	#pragma message ("Special code to deal with siren.wav!!");	
	
	if (AvP.DestructTimer != -1)
	{
		if (sbb->sound_loaded && sbb->activ_no != SOUND_NOACTIVEINDEX)
		{
			if (!_stricmp(GameSounds[sbb->sound_loaded->sound_num].wavName, "siren.wav"))
			{
				if (AvP.DestructTimer < ONE_FIXED * 10)
				{
					sbb->max_volume = 127;
				}
				else
				{
					sbb->max_volume = 90;
				}
				Sound_ChangeVolume (sbb->activ_no, sbb->max_volume);
			}
		}
	}
}


void StartPlacedSoundPlaying(STRATEGYBLOCK* sbptr)
{
	SOUND_BEHAV_BLOCK * sbb = 0;
	GLOBALASSERT(sbptr);
	GLOBALASSERT(sbptr->I_SBtype==I_BehaviourPlacedSound);
	sbb = (SOUND_BEHAV_BLOCK*)sbptr->SBdataptr;

	if(!sbb->sound_loaded) return;

	if(!sbb->playing)
	{
		//if sound is within range start it
		int dist=VectorDistance(&Player->ObWorld,&sbb->position);
		if(dist<sbb->outer_range)
		{
			SOUND3DDATA s3d;

			s3d.position = sbb->position;
			s3d.inner_range = sbb->inner_range;
			s3d.outer_range = sbb->outer_range;
			s3d.velocity.vx = 0;
			s3d.velocity.vy = 0;
			s3d.velocity.vz = 0;

			if(sbb->loop)
				Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "nelh", &s3d, &sbb->activ_no);
			else
				Sound_Play ((SOUNDINDEX)sbb->sound_loaded->sound_num, "neh", &s3d, &sbb->activ_no);

			if (sbb->activ_no != SOUND_NOACTIVEINDEX)
			{
				Sound_ChangeVolume (sbb->activ_no, sbb->max_volume);
				Sound_ChangePitch (sbb->activ_no, sbb->pitch);
			}
		}
		sbb->playing=1;
	}
}
void StopPlacedSoundPlaying(STRATEGYBLOCK* sbptr)
{
	SOUND_BEHAV_BLOCK * sbb = 0;
	GLOBALASSERT(sbptr);
	GLOBALASSERT(sbptr->I_SBtype==I_BehaviourPlacedSound);
	sbb = (SOUND_BEHAV_BLOCK*)sbptr->SBdataptr;

	if(!sbb->sound_loaded) return;
	if(sbb->playing)
	{
		sbb->playing=0;
		if (sbb->activ_no != SOUND_NOACTIVEINDEX)
		{
			Sound_Stop (sbb->activ_no);
		}
	}
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct placed_sound_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	BOOL playing;


}PLACED_SOUND_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV sbb 

void LoadStrategy_PlacedSound(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	SOUND_BEHAV_BLOCK * sbb;
	PLACED_SOUND_SAVE_BLOCK* block = (PLACED_SOUND_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourPlacedSound) return;

	sbb = (SOUND_BEHAV_BLOCK *)sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_LOAD(playing)

	sbb->sound_not_started = 0;
	Load_SoundState(&sbb->activ_no);
}

void SaveStrategy_PlacedSound(STRATEGYBLOCK* sbPtr)
{
	PLACED_SOUND_SAVE_BLOCK* block;
	SOUND_BEHAV_BLOCK * sbb;

	sbb = (SOUND_BEHAV_BLOCK *)sbPtr->SBdataptr;


	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	
	//start copying stuff
	COPYELEMENT_SAVE(playing)
	
	Save_SoundState(&sbb->activ_no);

}
