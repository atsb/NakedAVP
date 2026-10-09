/* Patrick 5/6/97 -------------------------------------------------------------
  AvP sound management header
  Provides support for sound samples, and for playing CDDA tracks.
  NB These two systems are entirely seperate.
  ----------------------------------------------------------------------------*/
#ifndef PSND_H
#define PSND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "psndproj.h"

/* Patrick 10/6/97 --------------------------------------------------------------
  SUPPORT FOR SOUND SAMPLE SYSTEM
  -----------------------------------------------------------------------------*/

/* Patrick 5/6/97 --------------------------------------------------------------
  Enumeration of different sound priorities: sounds created with the maximum
  priority may stop and displace sounds with the minimum priority, if the 
  maximum number of sounds is being played.
  -----------------------------------------------------------------------------*/
typedef enum activesoundpriority
{
	ASP_Minimum,
	ASP_Maximum,
}ACTIVESOUNDPRIORITY;

/* this is a structure for new 3d sound support */

typedef struct sound3ddata
{
	VECTORCH position;
	VECTORCH velocity;
	int inner_range;
	int outer_range;
	
} SOUND3DDATA;


/* Patrick 5/6/97 --------------------------------------------------------------
  Some platform independant defines:
  SOUND_NOACTIVEINDEX: used to invalidate sound handles
  MAX\MIN VOLUME: range of values accepted by functions for sound volume settings
  MAX\MIN PITCH: range of values accepted by functions for sound pitch settings,
  covers +- 4 octaves in 128ths of a semi-tone
  -----------------------------------------------------------------------------*/
#define SOUND_NOACTIVEINDEX			(-1)
#define SOUND_PLATFORMERROR			(-1)

#define VOLUME_MAX		(127)		
#define VOLUME_MIN		(0)
#define VOLUME_DEFAULT	(127)
#define VOLUME_FADERATE	(16) /* per second */

#define PITCH_MAX		(6144)	
#define PITCH_MIN		(-6144)
#define PITCH_DEFAULT	(0)

/* Patrick 5/6/97 --------------------------------------------------------------
  SoundSys_Start() & SoundSys_End(): initialise and de-initialise the sound 
  system.
  -----------------------------------------------------------------------------*/
extern void SoundSys_Start(void);
extern void SoundSys_End(void);
/* Patrick 5/6/97 --------------------------------------------------------------
  The management function: cleans up sounds that have finished playing, and 
  updates 3d sounds. This must be called during the game loop, whenever sounds
  are required.
  -----------------------------------------------------------------------------*/
extern void SoundSys_Management(void);
/* Patrick 5/6/97 --------------------------------------------------------------
  StopAll: stops all current sounds. New ones may be started immediately afterwards.
  RemoveAll(): stops all sounds, and then unloads any loaded sounds. The
  sound system remains initialised, but no sounds can be played until they are loaded. 
  -----------------------------------------------------------------------------*/
extern void SoundSys_StopAll(void);
extern void SoundSys_RemoveAll(void);
/* Patrick 5/6/97 --------------------------------------------------------------
  Switch On/Off: can be used to turn on\off sound. If turened off, al sounds
  stop and no further ones will be started until switched on again. Switch on
  only succeeds if the sound system has been initialised succesfully.
  -----------------------------------------------------------------------------*/
extern void SoundSys_SwitchOn(void);
extern void SoundSys_SwitchOff(void);
/* Patrick 5/6/97 --------------------------------------------------------------
  IsOn returns true (1) if the sound system is currently switched on, 
  or false (0) otherwise.  
  NB IsOn always returns false if the sound system has not been initialised, ie
  if SoundSys_Start() has not been called, or if the platform initialisation 
  failed.
  -----------------------------------------------------------------------------*/
extern int SoundSys_IsOn(void);
/* Patrick 5/6/97 --------------------------------------------------------------
  Change volume controls, which effect all current and future sounds.
  NB These do not effect the CDDA player.
  -----------------------------------------------------------------------------*/
extern void SoundSys_ChangeVolume(int volume);


/* New fading functionality KJL 99/4/5 */
extern void SoundSys_ResetFadeLevel(void);
extern void SoundSys_FadeIn(void);
extern void SoundSys_FadeOut(void);
extern void SoundSys_FadeOutFast(void);


/* Patrick 5/6/97 --------------------------------------------------------------
  Sound play function: creates and plays and instance of a loaded game sound.
  Has no effect if the maximum number of sounds are playing, or the maximum
  number of the specified sound are playing (these are defined in psndplat.h)  
  The second paramter is a format string which identifies any further paramters,
  and flags. If the second parameter is NULL, or points to a null string, the
  sound is played in 2D, at default volume and pitch, with lowest priority, 
  will not loop, and will not return a handle to itself.
  The following characters may be used in the format string (all others ignored):
  'd': play the sound as 3d- The next parameter must be the world position, and 
       of type VECTORCH *
  'n': play the sound as new 3d- The next parameter must be the world position, and 
       of type SOUND3DDATA *
  'e': external sound handle refernce- the next parameter must be a pointer
       to the external reference, and of type int*. The external refernce
	   may br used to subsequently stop the sound, change it's volume or
	   pitch, or update it's world space position. If the sound play function
	   fails, or if/when the sound subsequently stops playing, 
	   SOUND_NOACTIVEINDEX is written to this external reference.   
  'v': Volume- the next parameter must be the sounds starting volume
       of type int. Out of range values are forced into range.
  'p': Pitch- the next parameter must be the sounds starting pitch shift (shifted from
  	   the sound's base pitch value), and of type int. Out of range values are forced 
  	   into range.
  'l': play the sound looping (an external reference must be supplied, or the
       function has n effect)
  'h': play sound with maximum priority (minimum is the default)
  'm': flag for marines to ignore.    
  -----------------------------------------------------------------------------*/
extern void Sound_Play(SOUNDINDEX soundNumber, char* format, ...);
/* Patrick 5/6/97 --------------------------------------------------------------
  The remaining functions are used to modify existing playing sounds. All take
  a handle to a sound. If an invalid handle is passed, the functions have no
  effect.  Out or range volume or pitch values are forced into range.
  ChangeVolume works on an absolute scale (where the sounds original volume 
  corresponds to the maximum volume), and ChangePitch works as a pitch-shift
  from the base pitch of the sound. 
  -----------------------------------------------------------------------------*/
extern void Sound_Stop(int activeSoundNumber);
extern void Sound_ChangeVolume(int activeSoundNumber, int volume);
extern void Sound_ChangePitch(int activeSoundNumber, int pitch);
extern void Sound_Update3d(int activeSoundNumber, VECTORCH* posn);
extern void Sound_UpdateNew3d(int activeSoundNumber, SOUND3DDATA * s3d);
extern unsigned int SoundNumActiveVoices();


extern void Load_SoundState(int* soundHandle);
extern void Save_SoundState(int* soundHandle);

#ifdef __cplusplus
}
#endif

#endif










