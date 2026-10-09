/* Patrick 5/6/97 -------------------------------------------------------------
  AvP platform specific sound management header:
  Support for sample and CDDA sounds.
  ----------------------------------------------------------------------------*/
#ifndef PSNDPLAT_H
#define PSNDPLAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "psndproj.h"
#include "psnd.h"

/* Patrick 10/6/97 -------------------------------------------------------------
  SAMPLE SUPPORT
  ----------------------------------------------------------------------------*/

/* Patrick 5/6/97 -------------------------------------------------------------
  Data structure for a loaded sound.  The first four fields must be included 
  for all platforms. 
  ----------------------------------------------------------------------------*/
typedef struct soundsampledata
{
  	int loaded;
	int activeInstances;	 
	int volume;		
	int pitch;					

//	LPDIRECTSOUNDBUFFER dsBufferP;
	int dsBufferP;
	void *buffer;
	
	unsigned int flags;
	int dsFrequency;
	char * wavName;
	int length; //time in fixed point seconds
	
}SOUNDSAMPLEDATA;

/* Defines for the flags. */
#define SAMPLE_IN_HW	0x00000001
#define SAMPLE_IN_SW	0x00000002
#define SAMPLE_IN_3D	0x00000004

/* Patrick 5/6/97 -------------------------------------------------------------
  Data structure for a playing (active) sound. The first eight fields must be
  included for all platforms.
  ----------------------------------------------------------------------------*/
typedef struct activesoundsample
{
	SOUNDINDEX soundIndex;
	ACTIVESOUNDPRIORITY priority;	
	int volume;
	int	pitch;
	int *externalRef;			
	unsigned int loop :1;		
	unsigned int threedee :1;
	unsigned int paused :1;
	unsigned int marine_ignore	:1;
	unsigned int reverb_off :1;
	SOUND3DDATA threedeedata;
	
//	LPDIRECTSOUNDBUFFER dsBufferP;
//	LPDIRECTSOUND3DBUFFER ds3DBufferP;
//	LPKSPROPERTYSET	PropSetP;
	int dsBufferP;
	int ds3DBufferP;
	float PropSetP_pos[3];
	float PropSetP_vel[3];
	
	void *buffer;
	void *buffer3d;
	void *propset;	
}ACTIVESOUNDSAMPLE;

/* Patrick 5/6/97 -------------------------------------------------------------
  Data structures for WAV headers and chuncks
  ----------------------------------------------------------------------------*/
typedef struct pwavchunkheader
{
	char chunkName[4];
	int chunkLength;	
} PWAVCHUNKHEADER;

typedef struct pwavriffheader
{
	char type[4];
} PWAVRIFFHEADER;

void InitialiseBaseFrequency(SOUNDINDEX soundNum);
int LoadWavFile(int soundNum, char *);
int LoadWavFromFastFile(int soundNum, char *);

/* Patrick 5/6/97 -------------------------------------------------------------
  Start and EndSoundSys: does any platform operations required to initialise
  and exit the sound system
  ----------------------------------------------------------------------------*/
extern int PlatStartSoundSys(void);
extern void PlatEndSoundSys(void);
/* Patrick 5/6/97 -------------------------------------------------------------
  Sets the global volume, and returns true (1) if successful, or false (0) 
  otherwise. 
  ----------------------------------------------------------------------------*/
extern int PlatChangeGlobalVolume(int volume);
/* Patrick 5/6/97 -------------------------------------------------------------
  Play a sound: does platform operations required to create and play a new
  sound from the active sound data for the given sound. This function must
  also perform any neccessary operations that may be required to initilaise
  a sounds volume, pitch, or 3d attributes.
  PlatStartSoundSys returns true (1) if successful, or false (0) otherwise.
  ----------------------------------------------------------------------------*/
extern int PlatPlaySound(int activeIndex);
/* Patrick 5/6/97 -------------------------------------------------------------
  Stop a sound: Any platform operations required for stopping a sound. returns 
  true (1) if successful, or SOUND_PLATFORMERROR (-1) otherwise.
  ----------------------------------------------------------------------------*/
extern void PlatStopSound(int activeIndex);
/* Patrick 5/6/97 -------------------------------------------------------------
 The following perform the required platform operations for changing volume,
 and pitch for a playing sound. All return true (1) if successful, or 
 SOUND_PLATFORMERROR (-1) otherwise.
  ----------------------------------------------------------------------------*/
extern int PlatChangeSoundVolume(int activeIndex, int volume);
extern int PlatChangeSoundPitch(int activeIndex, int pitch);
/* Patrick 5/6/97 -------------------------------------------------------------
 The following function performs the neccessary operations required for a 3d 
 sound (eg volume, pan). Return true (1) if successful, or SOUND_PLATFORMERROR 
 (-1) otherwise. 
  ----------------------------------------------------------------------------*/
extern int PlatDo3dSound(int activeIndex);
/* Patrick 5/6/97 -------------------------------------------------------------
	Return true (1) if the sound has stopped, false (0) if it is still playing
	or SOUND_PLATFORMERROR (-1) otherwise.   
  ----------------------------------------------------------------------------*/
extern int PlatSoundHasStopped(int activeIndex);
/* Patrick 5/6/97 -------------------------------------------------------------
  Performs any platform specific operations required to unload a game sound,
  wile the sound system is exiting
  ----------------------------------------------------------------------------*/
extern void PlatEndGameSound(SOUNDINDEX index);

/* Davew 23/7/98 --------------------------------------------------------------
	Update the player in the 3D sound system.
  ----------------------------------------------------------------------------*/
extern void PlatUpdatePlayer();

/* Davew 27/7/98 ---------------------------------------------------------------
	This sets the enviroment for the Listener. The first parameter is the index
	of the enviroment, the second is the reverb ratio to apply to sound (0 - 1)
	float. Negative means let the card deal with reverb.
	---------------------------------------------------------------------------*/
extern void PlatSetEnviroment(unsigned int env_index, float reverb_mix);
extern unsigned int PlatMaxHWSounds();

/* Davew 11/11/98 --------------------------------------------------------------
	Controls the use of 3DHW, by default it is used if present. Return codes
	2 means it was already in that state, 1 means to changed state sucessfully
	-1 means it failed to change state. Note these functions may well be slow.
	---------------------------------------------------------------------------*/
extern int PlatUse3DSoundHW();
extern int PlatDontUse3DSoundHW();

/* Patrick 5/6/97 -------------------------------------------------------------
  Defines for max number of sounds, and instances of sounds, allowed;
  also maximum and minimum volume, pitch, and pan values for platform.
  ----------------------------------------------------------------------------*/
#define SOUND_MAXACTIVE			(120)
#define SOUND_MAXACTIVE_SW		(20)
#define SOUND_MAXINSTANCES		(20)
#define SOUND_MAXSIZE 			(250000)	/* biggest sample we will allow to be loaded */

#define SOUND_DEACTIVATERANGE (10000 * GlobalScale)

#define VOLUME_MAXPLAT			(0)			/* attenuation values, in db's */
#define VOLUME_MINPLAT			(-10000)	
#define VOLUME_PLAT2DSCALE		(96)		/* in 128ths of the original volume */

/* frequency values are those accepted by ds. Pitch is measured in semi-tones,
and is applied relative to the loaded frequency of the sound */
#define FREQUENCY_MAXPLAT			(100000)
#define FREQUENCY_MINPLAT			(100)
#define PITCH_MAXPLAT				(6144)
#define PITCH_MINPLAT				(-6144)
#define PITCH_DEFAULTPLAT			(0)

#define PAN_MAXPLAT					(1200) 	   
#define PAN_MINPLAT					(-1200)
#define PAN_3DDAMPDISTANCE			(1000)

/* NB ds supports pan +- 10000: */

/* Patrick 5/6/97 -------------------------------------------------------------
  Defines for 3D attenuation:
  Volume attenuation is per metre, in global volume units, as defined by 
  VOLUME_MAX and VOLUME_MIN in psnd.h
  Pan attenuation is per 180/2048 degrees, in platform units as defined above
  by PAN_MAXPLAT and PAN_MINPLAT (ie panattenuation*(1024)<=PAN_MAXPLAT), 
  assuming symetrical range.
  ----------------------------------------------------------------------------*/
#define VOLUME_3DATTENUATION	(4)

/* Patrick 5/6/97 -------------------------------------------------------------
  Global references to sound management data areas, and blank instances of data
  structures storted in those areas    
  ----------------------------------------------------------------------------*/
extern SOUNDSAMPLEDATA GameSounds[];		   
extern ACTIVESOUNDSAMPLE ActiveSounds[]; 
extern SOUNDSAMPLEDATA BlankGameSound;
extern ACTIVESOUNDSAMPLE BlankActiveSound;

void UpdateSoundFrequencies(void);

#ifdef __cplusplus
}
#endif

#endif
