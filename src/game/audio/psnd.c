/* Patrick 5/6/97 -------------------------------------------------------------
  AvP sound management source
  ----------------------------------------------------------------------------*/

#define DB_LEVEL 1

#include <stdio.h>

#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"

#include "psndplat.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "db.h"
#include "showcmds.h"
#include "avp_userprofile.h"
#include "cdplayer.h"

/* Patrick 5/6/97 -------------------------------------------------------------
  Internal globals
  ----------------------------------------------------------------------------*/
int SoundSwitchedOn = 0;
static int SoundInitialised = 0;
static int GlobalVolume = VOLUME_DEFAULT;
extern int DebuggingCommandsActive;


int EffectsSoundVolume=VOLUME_DEFAULT;

static int MasterVolumeFadeLevel;
static enum FADE_STATUS_ID {FADE_STATUS_READY, FADE_STATUS_UP, FADE_STATUS_DOWN, FADE_STATUS_DOWN_FAST} MasterVolumeFadeStatus = FADE_STATUS_READY;


static unsigned int Sound_MaxActive_HW = 0;

static void HandleFadingLevel(void);

/* Patrick 5/6/97 -------------------------------------------------------------
  Internal functions
  ----------------------------------------------------------------------------*/
static int FindFreeActiveSound(unsigned int min, unsigned int max);
static int FindLowerPriorityActiveSound(ACTIVESOUNDPRIORITY testPriority, unsigned int min, unsigned int max);

/* Patrick 5/6/97 -------------------------------------------------------------
  External refernces
  ----------------------------------------------------------------------------*/
extern int NormalFrameTime;
extern int GlobalFrameCounter;


/* Patrick 5/6/97 -------------------------------------------------------------
  Sound system functions
  ----------------------------------------------------------------------------*/
void SoundSys_Start(void)
{
	int i;

	/* if we have already switched on, so do nothing */
	if(SoundSwitchedOn) return;
	/* we are not switched on, but we are initialised, so do nothing */	
	if(SoundInitialised) return;

	/* initialise game sounds and active instances*/
	for(i=0;i<SID_MAXIMUM;i++) GameSounds[i] = BlankGameSound;
	for(i=0;i<SOUND_MAXACTIVE;i++) ActiveSounds[i] = BlankActiveSound;
		
	/* try to initialise sound, and turn on */
	SoundInitialised = PlatStartSoundSys();
	Sound_MaxActive_HW = SOUND_MAXACTIVE_SW + PlatMaxHWSounds();
	if(Sound_MaxActive_HW > SOUND_MAXACTIVE)
	{
		Sound_MaxActive_HW = SOUND_MAXACTIVE;
	} 
	SoundSys_SwitchOn();

	/* cancel any fades */
	

	/* set global volume */
	GlobalVolume = VOLUME_DEFAULT;
	PlatChangeGlobalVolume(VOLUME_DEFAULT);
	
	
}
	
void SoundSys_End(void)
{
	/* end only if we are initialised, regardless of whether we are switched on */
	if(!SoundInitialised) return;
		
	/* stop and remove all sounds */
	SoundSys_RemoveAll();
	
	/* switch off, and de-initillise */
	PlatEndSoundSys();
	SoundSys_SwitchOff();
	SoundInitialised = 0; /* forces call to Soundsys_Start to re-start sound system */	
}

void SoundSys_Management(void)
{
	int i;
	int numActive = 0;
	int num3dUpdates = 0;

	if(!SoundSwitchedOn) return;

	/* go through all the active sounds */
	for(i=0;i<SOUND_MAXACTIVE;i++)
	{
		if(ActiveSounds[i].soundIndex==SID_NOSOUND) continue; /* empty slot */
		numActive++;

		if(PlatSoundHasStopped(i) && !ActiveSounds[i].paused)
		{
			Sound_Stop(i);
			continue;			
		}
		if(ActiveSounds[i].threedee) 
		{
			PlatDo3dSound(i);
			num3dUpdates++;
		}		
	}

	HandleFadingLevel();

	{
		int requestedVolume = MUL_FIXED(EffectsSoundVolume,MasterVolumeFadeLevel);
		if (requestedVolume != GlobalVolume)
		{
			SoundSys_ChangeVolume(requestedVolume);
		}

	}
	CheckCDVolume();
	
	PlatUpdatePlayer();
	if (ShowDebuggingText.Sounds)
	{
		int i = Sound_MaxActive_HW;
		PrintDebuggingText("Number of Active Sounds %u \n", SoundNumActiveVoices());
		//display a list of all sounds being played as well
		while(i-- > 0)
		{
			if(ActiveSounds[i].soundIndex != SID_NOSOUND)
			{
				PrintDebuggingText("%s\n",GameSounds[ActiveSounds[i].soundIndex].wavName);
			}
		}
		
	}

   	if(WARPSPEED_CHEATMODE || JOHNWOO_CHEATMODE || DebuggingCommandsActive)
   		UpdateSoundFrequencies();
}

extern void SoundSys_ResetFadeLevel(void)
{
	MasterVolumeFadeLevel = ONE_FIXED;
	MasterVolumeFadeStatus = FADE_STATUS_READY;	
}

extern void SoundSys_FadeIn(void)
{
	/* always fade in from silence ? */
	MasterVolumeFadeLevel = ONE_FIXED/2;
	MasterVolumeFadeStatus = FADE_STATUS_UP;	
}
extern void SoundSys_FadeOut(void)
{
	MasterVolumeFadeStatus = FADE_STATUS_DOWN;	
}
extern void SoundSys_FadeOutFast(void)
{
	MasterVolumeFadeStatus = FADE_STATUS_DOWN_FAST;	
}
static void HandleFadingLevel(void)
{
	switch (MasterVolumeFadeStatus)
	{
		default:
		case FADE_STATUS_READY:
			break;

		case FADE_STATUS_UP:
		{
			MasterVolumeFadeLevel+=NormalFrameTime/2;
			if (MasterVolumeFadeLevel>ONE_FIXED)
			{
				MasterVolumeFadeLevel = ONE_FIXED;
				MasterVolumeFadeStatus = FADE_STATUS_READY;
			}
			break;
		}
		case FADE_STATUS_DOWN:
		{
			MasterVolumeFadeLevel-=NormalFrameTime/8;
			if (MasterVolumeFadeLevel<0)
			{
				MasterVolumeFadeLevel = 0;
				MasterVolumeFadeStatus = FADE_STATUS_READY;
			}
			break;
		}
		case FADE_STATUS_DOWN_FAST:
		{
			MasterVolumeFadeLevel-=NormalFrameTime;
			if (MasterVolumeFadeLevel<0)
			{
				MasterVolumeFadeLevel = 0;
				MasterVolumeFadeStatus = FADE_STATUS_READY;
			}
			break;
		}
	}
}
void SoundSys_PauseOn(void)
{
	int i;

	/* if we're not switched on, should be nothing playing */
	if(!SoundSwitchedOn) return;

	for(i=0;i<SOUND_MAXACTIVE;i++)
	{
		if(ActiveSounds[i].soundIndex!=SID_NOSOUND) ActiveSounds[i].paused = 1;		
	}
}

void SoundSys_PauseOff(void)
{
	int i;

	/* if we're not switched on, should be nothing playing */
	if(!SoundSwitchedOn) return;

	for(i=0;i<SOUND_MAXACTIVE;i++)
	{
		if(ActiveSounds[i].soundIndex!=SID_NOSOUND) ActiveSounds[i].paused = 0;		
	}
}

void SoundSys_StopAll(void)
{
	int i;

	/* if we're not switched on, should be nothing playing */
	if(!SoundSwitchedOn) return;

	for(i=0;i<SOUND_MAXACTIVE;i++)
	{
		if(ActiveSounds[i].soundIndex!=SID_NOSOUND) Sound_Stop(i);		
	}
}

void SoundSys_RemoveAll(void)
{
	int i;
	
	/* if we are not initialised, there should be no sounds loaded or playing.
	if we are switched off, we should still unload them all */
	if(!SoundInitialised) return;

	/* make sure there are no sounds playing */
	SoundSys_StopAll();

	/* deallocate all sounds */
	for(i=0;i<SID_MAXIMUM;i++)
	{
		if(GameSounds[i].loaded) 
		{
			LOCALASSERT(GameSounds[i].activeInstances==0);
			PlatEndGameSound(i);
			GameSounds[i] = BlankGameSound;
		}		
	}	
}

void SoundSys_SwitchOn(void)
{
	/* if already switched on, nothing to do*/
	if(SoundSwitchedOn) return; 
	/* if not initialised, can't switch on */
	if(!SoundInitialised) return;
	 
	SoundSwitchedOn = 1;	
	/* initialise global volume controls */
	GlobalVolume = VOLUME_DEFAULT;
	MasterVolumeFadeLevel = ONE_FIXED;
	MasterVolumeFadeStatus = FADE_STATUS_READY;
}

void SoundSys_SwitchOff(void)
{	
	/* if already switched off, nothing to do*/
	if(!SoundSwitchedOn) return; 

	SoundSys_StopAll();
	SoundSwitchedOn = 0;
}

int SoundSys_IsOn(void)
{
	return SoundSwitchedOn;
}

void SoundSys_ChangeVolume(int volume)
{
	int newVolume, ok;
	
	if(!SoundSwitchedOn) return;
	/* validate argument */
	newVolume = volume;
	if(newVolume>VOLUME_MAX) newVolume = VOLUME_MAX;
	if(newVolume<VOLUME_MIN) newVolume = VOLUME_MIN;	

	if (newVolume==GlobalVolume) return;

	/* set global volume */
	GlobalVolume = newVolume;
	/* call the platform function: if we get back an error, ignore it */
	ok = PlatChangeGlobalVolume(newVolume);
}


/* Patrick 5/6/97 -------------------------------------------------------------
  Functions for playing and controlling individual sounds
  ----------------------------------------------------------------------------*/
void Sound_Play(SOUNDINDEX soundNumber, char *format, ...)
{	
	int newIndex;
	int loop = 0;
	int	*externalRef = NULL;
	ACTIVESOUNDPRIORITY priority = ASP_Minimum;
	SOUND3DDATA * p_3ddata = 0;
	VECTORCH * worldPosn = 0;
	int volume;
	int pitch;
	int marine_ignore;
	int reverb_off = 0;
	int soundStartPosition = 0;

	{
		extern int PlaySounds;
		if (!PlaySounds) return;
	}

	if(!SoundSwitchedOn) return;


	/* check soundIndex for bounds, whether it has been loaded, and number of instances */
	if((soundNumber<0)||(soundNumber>=SID_MAXIMUM)) return;
	if(!(GameSounds[soundNumber].loaded)) return;
	if(!(GameSounds[soundNumber].activeInstances<SOUND_MAXINSTANCES)) return;

	db_logf5(("About to play sound %i", soundNumber));

	/* initialise volume and pitch from game sound data */
	volume = GameSounds[soundNumber].volume;
	pitch = GameSounds[soundNumber].pitch;
	marine_ignore=0;

	/* examine the format string: if it is null, ignore it */
	if(format)
	{
		char *nextChar = format;
		va_list argPtr;

		va_start(argPtr,format);
		while(*nextChar!='\0')
		{
			switch(*nextChar)
			{
				case('d'):
				{					
					worldPosn = va_arg(argPtr,VECTORCH*);
					break;
				}
				case('n'):
				{					
					p_3ddata = va_arg(argPtr,SOUND3DDATA*);
					break;
				}
				case('e'):
				{					
					externalRef = va_arg(argPtr,int*);
					break;
				}
				case('l'):
				{					
					loop = 1;
					priority = ASP_Maximum;
					break;
				}				
				case('h'):
				{					
					priority = ASP_Maximum;
					break;
				}		
				case('v'):
				{					
					volume = va_arg(argPtr,int);
					if(volume>VOLUME_MAX) volume=VOLUME_MAX;
					if(volume<VOLUME_MIN) volume=VOLUME_MIN;
					break;
				}		
				case('p'):
				{					
					pitch = va_arg(argPtr,int);;
					if(pitch>PITCH_MAX) pitch=PITCH_MAX;
					if(pitch<PITCH_MIN) pitch=PITCH_MIN;
					break;
				}
				case('m'):
				{
					marine_ignore=1;
					break;
				}
				case('r'):
				{
					reverb_off = 1;
					break;
				}
				case('P'):
				{					
					soundStartPosition = va_arg(argPtr,int);;
					break;
				}
				default:
				{
					break;
				}		
			}
			nextChar++;
		}
		va_end(argPtr);	
	}

	/* check for invalid parameter combinations */
	if((loop)&&(externalRef==NULL))
	{
		db_log5("SoundPlay bad params.");
		return;
	}

	/* Deal with resource allocation. */
	{
		/* Range of active buffers to search. */
		unsigned int min, max;
		if(GameSounds[soundNumber].flags & SAMPLE_IN_SW)
		{
			min = 0;
			max = SOUND_MAXACTIVE_SW;
		}
		else
		{
			min = SOUND_MAXACTIVE_SW + 1;
			max = Sound_MaxActive_HW;
		}

		/* find free active sound slot */
		newIndex = FindFreeActiveSound(min, max);
		if(newIndex==SOUND_NOACTIVEINDEX && !(GameSounds[soundNumber].flags & SAMPLE_IN_SW))
		{
			//failed to find a free hardware slot , so try software slot instead.
			//mainly to cope with cards that can load sounds into hardware , but can't play them there
			newIndex = FindFreeActiveSound(0, SOUND_MAXACTIVE_SW);

		}
		if(newIndex==SOUND_NOACTIVEINDEX)
		{
			db_log3("Having to stop another sound.");
			/* try to find an existing sound with a lower priority */
			newIndex = FindLowerPriorityActiveSound(priority, min, max);
			if(newIndex==SOUND_NOACTIVEINDEX && !(GameSounds[soundNumber].flags & SAMPLE_IN_SW))
			{
				//failed to find a free hardware slot , so try software slot instead.
				//mainly to cope with cards that can load sounds into hardware , but can't play them there
				newIndex = FindLowerPriorityActiveSound(priority, 0, SOUND_MAXACTIVE_SW);

			}

			if(newIndex==SOUND_NOACTIVEINDEX)
			{
				db_log3("Failed to find a lower priority sound.");
				return; /* give up */
			}

			/* remove it, and use it's slot */
			db_log3("Stopping a lower priority sound.");
			Sound_Stop(newIndex);
		}
		else
		{
			db_log3("Found a free slot.");
		}
	}
	
	/* fill out the active sound */
	ActiveSounds[newIndex].soundIndex = soundNumber;
	ActiveSounds[newIndex].priority = priority;
	ActiveSounds[newIndex].volume = volume;
	ActiveSounds[newIndex].pitch = pitch;
	ActiveSounds[newIndex].externalRef = externalRef;
	ActiveSounds[newIndex].loop = 1;
	ActiveSounds[newIndex].marine_ignore=marine_ignore;
	ActiveSounds[newIndex].reverb_off=reverb_off;
	if(loop) ActiveSounds[newIndex].loop = 1;
	else ActiveSounds[newIndex].loop = 0;

#if 0
fprintf(stderr, "PSND: Play: new = %d. num = %d, p = %d, v = %d, pi = %d, l = %d, mi = %d, rev = %d\n", newIndex, soundNumber, priority, volume, pitch, loop, marine_ignore, reverb_off);
fprintf(stderr, "PSND: Play: %d %d %s l:%d\n", newIndex, soundNumber, GameSounds[soundNumber].wavName, loop);
#endif

	if(worldPosn) 
	{
		VECTORCH zeroPosn = {0,0,0};
		ActiveSounds[newIndex].threedeedata.position = *worldPosn;
		ActiveSounds[newIndex].threedeedata.velocity = zeroPosn;
		ActiveSounds[newIndex].threedeedata.inner_range = 0;
		ActiveSounds[newIndex].threedeedata.outer_range = 32000;
		
		ActiveSounds[newIndex].threedee = 1;
	}
	else if (p_3ddata)
	{
		ActiveSounds[newIndex].threedeedata = *p_3ddata;
		ActiveSounds[newIndex].threedee = 1;
	}
	else 
	{
		VECTORCH zeroPosn = {0,0,0};
		ActiveSounds[newIndex].threedeedata.position = zeroPosn;
		ActiveSounds[newIndex].threedeedata.velocity = zeroPosn;
		ActiveSounds[newIndex].threedee = 0;
	}
	
	/* try and play the sound */
	{
		int ok = PlatPlaySound(newIndex);
		if(ok==SOUND_PLATFORMERROR)
		{
			/* the sound failed to play: any platform cleanups should have been done,
			so just bank the sound slot */
			ActiveSounds[newIndex] = BlankActiveSound;
			db_log5("Error: PlatPlaySound failed.");
			return;
		}		
	}	

	/* finally, update the game sound instances, and external reference */
	GameSounds[soundNumber].activeInstances++;
	if(externalRef) *externalRef = newIndex;

/* only will happen because of savegames */
//	if(soundStartPosition && ActiveSounds[newIndex].dsBufferP)
//	{
//		//sound starts part of the way in
//		IDirectSoundBuffer_SetCurrentPosition(ActiveSounds[newIndex].dsBufferP,soundStartPosition);
//	}
#if 0 /* TODO */
	if (soundStartPosition)
		fprintf(stderr, "Sound_Play: sound starts part of the way in (%d)\n", soundStartPosition);
#endif		
}

void Sound_Stop(int activeSoundNumber)
{
	SOUNDINDEX soundNo;
	int buf;
	
	if(!SoundSwitchedOn) return;
	/* validate argument */
	if(activeSoundNumber<0) return;
	if(activeSoundNumber>=SOUND_MAXACTIVE) return;

	/* Check there's a sound in this slot */
	if(ActiveSounds[activeSoundNumber].soundIndex == SID_NOSOUND) return;

	/* update game sound instances, and external reference */
	soundNo = ActiveSounds[activeSoundNumber].soundIndex;
	GameSounds[soundNo].activeInstances--;
	db_assert1((GameSounds[soundNo].activeInstances>=0)&&
				(GameSounds[soundNo].activeInstances<SOUND_MAXINSTANCES));
	if(ActiveSounds[activeSoundNumber].externalRef)
		*(ActiveSounds[activeSoundNumber].externalRef) = SOUND_NOACTIVEINDEX;      
			
	/* stop the sound: it may have already stopped, of course, but never mind */
	
	PlatStopSound(activeSoundNumber);

#if 0	
fprintf(stderr, "PSND: Stop: %d %d %s\n", activeSoundNumber, soundNo, GameSounds[soundNo].wavName);
#endif
	
	/* release the active sound slot */
	buf = ActiveSounds[activeSoundNumber].ds3DBufferP;
	ActiveSounds[activeSoundNumber] = BlankActiveSound;
	ActiveSounds[activeSoundNumber].ds3DBufferP = buf;
}

void Sound_ChangeVolume(int activeSoundNumber, int volume)
{
	int newVolume;
	if(!SoundSwitchedOn) return;

	/* validate argument */
	if(activeSoundNumber<0) return;
	if(activeSoundNumber>=SOUND_MAXACTIVE) return;
	/* Check there's a sound in this slot */
	if(ActiveSounds[activeSoundNumber].soundIndex == SID_NOSOUND) return;

	/* validate other argument */
	newVolume = volume;
	if(newVolume>VOLUME_MAX) newVolume = VOLUME_MAX;
	if(newVolume<VOLUME_MIN) newVolume = VOLUME_MAX;

	/* check the new volume is different from the old one */
	if(volume==(ActiveSounds[activeSoundNumber].volume)) return;

	/* set volume field in active sound */
	ActiveSounds[activeSoundNumber].volume = newVolume;
	
	/* if we're a 2d sound, just change the volume, but if we're 3d
	then call 3d update instead */
	if(ActiveSounds[activeSoundNumber].threedee) PlatDo3dSound(activeSoundNumber);
	else PlatChangeSoundVolume(activeSoundNumber,ActiveSounds[activeSoundNumber].volume);	
}

void Sound_ChangePitch(int activeSoundNumber, int pitch)
{
	int newPitch;
	if(!SoundSwitchedOn) return;

	/* validate argument */
	if(activeSoundNumber<0) return;
	if(activeSoundNumber>=SOUND_MAXACTIVE) return;
	/* Check there's a sound in this slot */
	if(ActiveSounds[activeSoundNumber].soundIndex == SID_NOSOUND) return;

	/* validate other argument */
	newPitch = pitch;
	if(newPitch>PITCH_MAX) newPitch = PITCH_MAX;
	if(newPitch<PITCH_MIN) newPitch = PITCH_MAX;

	/* check the new pitch is different from the old one */
	if(pitch==(ActiveSounds[activeSoundNumber].pitch)) return;

	/* set pitch field in active sound, and change it... */
	ActiveSounds[activeSoundNumber].pitch = newPitch;
	PlatChangeSoundPitch(activeSoundNumber,ActiveSounds[activeSoundNumber].pitch);	
}

void Sound_Update3d(int activeSoundNumber, VECTORCH* posn)
{
	if(!SoundSwitchedOn) return;

	/* validate argument */
	if(activeSoundNumber<0) return;
	if(activeSoundNumber>=SOUND_MAXACTIVE) return;

	/* Check there's a sound in this slot */
	if(ActiveSounds[activeSoundNumber].soundIndex == SID_NOSOUND) return;

	ActiveSounds[activeSoundNumber].threedeedata.position = *posn;
}

void Sound_UpdateNew3d(int activeSoundNumber, SOUND3DDATA * s3d)
{
	if(!SoundSwitchedOn) return;

	/* validate argument */
	if(activeSoundNumber<0) return;
	if(activeSoundNumber>=SOUND_MAXACTIVE) return;

	/* Check there's a sound in this slot */
	if(ActiveSounds[activeSoundNumber].soundIndex == SID_NOSOUND) return;

	ActiveSounds[activeSoundNumber].threedeedata = *s3d;
	
}

unsigned int SoundNumActiveVoices()
{
	int i = Sound_MaxActive_HW;
	int num_active = 0;

	while(i-- > 0)
	{
		if(ActiveSounds[i].soundIndex != SID_NOSOUND)
		{
			num_active++;
		}
	}
	
	return num_active;
}

/* Patrick 5/6/97 -------------------------------------------------------------
  Internal support functions
  ----------------------------------------------------------------------------*/
static int FindFreeActiveSound(unsigned int min, unsigned int max)
{
	int i;
	for(i = min; (i < max); i++) 
	{
		if(ActiveSounds[i].soundIndex == SID_NOSOUND) return i;
	}
	return SOUND_NOACTIVEINDEX;
}

static int FindLowerPriorityActiveSound(ACTIVESOUNDPRIORITY testPriority, unsigned int min, unsigned int max)
{
	int i;
	for(i = min; (i < max); i++) 
	{
		if((ActiveSounds[i].soundIndex != SID_NOSOUND)&&
		   (ActiveSounds[i].priority < testPriority)) return i;
	}
	return SOUND_NOACTIVEINDEX;
}



static SOUNDINDEX GetSoundIndexFromNameAndIndex(const char* name,SOUNDINDEX index);

#include "savegame.h"

typedef struct sound_save_block
{
	SAVE_BLOCK_HEADER header;

	int fileNameLength;

	SOUNDINDEX soundIndex;
	ACTIVESOUNDPRIORITY priority;	
	int volume;
	int	pitch;
	unsigned int loop :1;		
	unsigned int threedee :1;
	unsigned int paused :1;
	unsigned int marine_ignore	:1;
	unsigned int reverb_off :1;
	unsigned int externalRef :1;
	SOUND3DDATA threedeedata;
	int position;
}SOUND_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV sound

void Load_SoundState(int* soundHandle)
{
	SOUND_SAVE_BLOCK* block;
	const char* name;
	SOUNDINDEX soundIndex;

	if(!soundHandle) return;
	block = (SOUND_SAVE_BLOCK*)GetNextBlockIfOfType(SaveBlock_SoundState);
	if(!block) return ;

	name = (const char*)(block+1);
	//stop the current sound
	if(*soundHandle == SOUND_NOACTIVEINDEX)
	{
		Sound_Stop(*soundHandle);
	}

	//check the size
	if(block->header.size != sizeof(*block) + block->fileNameLength) return;

	soundIndex = GetSoundIndexFromNameAndIndex(name,block->soundIndex);
	if(soundIndex==SID_NOSOUND) return;

	{
		char playOptions[20]="evpP";

		if(block->loop) strcat(playOptions,"l");
		if(block->marine_ignore) strcat(playOptions,"m");
		if(block->reverb_off) strcat(playOptions,"r");
		if(block->priority == ASP_Maximum) strcat(playOptions,"h");

		if(block->threedee)
		{
			strcat(playOptions,"n");
			Sound_Play(soundIndex,playOptions,soundHandle,block->volume,block->pitch,block->position,&block->threedeedata);
		}
		else
		{
			Sound_Play(soundIndex,playOptions,soundHandle,block->volume,block->pitch,block->position);
		}

	}


}


void Save_SoundState(int* soundHandle)
{
	
	if(!soundHandle) return;
	
	
	if(*soundHandle == SOUND_NOACTIVEINDEX)
	{
		SAVE_BLOCK_HEADER* header;
		GET_SAVE_BLOCK_POINTER(header);
	
		//fill in the header
		header->size = sizeof(*header);
		header->type = SaveBlock_SoundState;
	}
	else
	{
		ACTIVESOUNDSAMPLE* sound = &ActiveSounds[*soundHandle];
		
		SOUND_SAVE_BLOCK* block;
		const char* name = GameSounds[sound->soundIndex].wavName;
		int name_length = strlen(name) + 1;
			
	   	block = GetPointerForSaveBlock(sizeof(*block)+name_length);
	
		//fill in the header
		block->header.size = sizeof(*block) + name_length;
		block->header.type = SaveBlock_SoundState;
		
		

		COPYELEMENT_SAVE(soundIndex)
		COPYELEMENT_SAVE(priority)	
		COPYELEMENT_SAVE(volume)
		COPYELEMENT_SAVE(pitch)
		COPYELEMENT_SAVE(loop)		
		COPYELEMENT_SAVE(threedee)
		COPYELEMENT_SAVE(paused)
		COPYELEMENT_SAVE(marine_ignore)
		COPYELEMENT_SAVE(reverb_off)
		COPYELEMENT_SAVE(threedeedata)
		block->externalRef = 1;

		block->position = 0;
	   	block->fileNameLength = name_length;

		//the volume in the active sound list is scaled differently from the volume used
		//by Sound_Play
		block->volume<<=7;
		block->volume/=VOLUME_PLAT2DSCALE;

/* only for savegames */		
//		if(sound->dsBufferP)
//			IDirectSoundBuffer_GetCurrentPosition(sound->dsBufferP,(LPDWORD)&block->position,NULL);
//		else
			block->position = 0;
#if 0 /* TODO */
fprintf(stderr, "Save_SoundState: GetCurrentPosition!\n");
#endif		
		strcpy((char*)(block+1),name);
		
	}	

}

void Load_SoundState_NoRef(SAVE_BLOCK_HEADER* header)
{
	SOUND_SAVE_BLOCK* block = (SOUND_SAVE_BLOCK*) header;
	const char* name = (const char*)(block+1);
	SOUNDINDEX soundIndex;

	//check the size
	if(block->header.size != sizeof(*block) + block->fileNameLength) return;

	//only load if the sound doesn't require an external reference
	if(block->externalRef) return;

	
	soundIndex = GetSoundIndexFromNameAndIndex(name,block->soundIndex);
	if(soundIndex==SID_NOSOUND) return;

	{
		char playOptions[20]="vpP";

		if(block->marine_ignore) strcat(playOptions,"m");
		if(block->reverb_off) strcat(playOptions,"r");
		if(block->priority == ASP_Maximum) strcat(playOptions,"h");

		if(block->threedee)
		{
			strcat(playOptions,"n");
			Sound_Play(soundIndex,playOptions,block->volume,block->pitch,block->position,&block->threedeedata);
		}
		else
		{
			Sound_Play(soundIndex,playOptions,block->volume,block->pitch,block->position);
		}
	}
}

void Save_SoundsWithNoReference()
{
	int i;

	for(i=0;i<SOUND_MAXACTIVE;i++)
	{
		if(ActiveSounds[i].soundIndex != SID_NOSOUND)
		{
			if(!ActiveSounds[i].externalRef)
			{
				ACTIVESOUNDSAMPLE* sound = &ActiveSounds[i];
				SOUND_SAVE_BLOCK* block;

				const char* name = GameSounds[sound->soundIndex].wavName;
				int name_length = strlen(name) + 1;
					
				block = GetPointerForSaveBlock(sizeof(*block)+name_length);
	
				//fill in the header
				block->header.size = sizeof(*block) + name_length;
				block->header.type = SaveBlock_SoundState;
				

				COPYELEMENT_SAVE(soundIndex)
				COPYELEMENT_SAVE(priority)	
				COPYELEMENT_SAVE(volume)
				COPYELEMENT_SAVE(pitch)
				COPYELEMENT_SAVE(loop)		
				COPYELEMENT_SAVE(threedee)
				COPYELEMENT_SAVE(paused)
				COPYELEMENT_SAVE(marine_ignore)
				COPYELEMENT_SAVE(reverb_off)
				COPYELEMENT_SAVE(threedeedata)
				block->externalRef = 0;

				block->position = 0;
				block->fileNameLength = name_length;

				//the volume in the active sound list is scaled differently from the volume used
				//by Sound_Play
				block->volume<<=7;
				block->volume/=VOLUME_PLAT2DSCALE;

/* savegames */
//				if(sound->dsBufferP)
//					IDirectSoundBuffer_GetCurrentPosition(sound->dsBufferP,(LPDWORD)&block->position,NULL);
//				else
					block->position = 0;
#if 0 /* TODO */
				fprintf(stderr, "Save_SoundsWithNoReference: GetCurrentPosition!\n");
#endif
				
				strcpy((char*)(block+1),name);
			}
		}
	}
}


static SOUNDINDEX GetSoundIndexFromNameAndIndex(const char* name,SOUNDINDEX index)
{
	int i;
	if(index>=0 && index<SID_MAXIMUM)
	{
		if(GameSounds[index].loaded)
		{
			if(!strcmp(GameSounds[index].wavName,name)) return index;
		}
	
	}

	for(i=0;i<SID_MAXIMUM;i++)
	{
		if(GameSounds[i].loaded)
		{
			if(!strcmp(GameSounds[i].wavName,name)) return (SOUNDINDEX) i;
		}
	}
	return SID_NOSOUND;
}
