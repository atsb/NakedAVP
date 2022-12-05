#ifndef __WIN95_CDPLAYER_H__
#define __WIN95_CDPLAYER_H__

/* KJL 12:40:35 07/05/98 - This is code derived from Patrick's original stuff &
moved into it's own file. */


/* Patrick 10/6/97 --------------------------------------------------------------
  SUPPORT FOR CDDA SYSTEM
  -----------------------------------------------------------------------------*/

/* Patrick 10/6/97 --------------------------------------------------------------
  Some Volume defines
  -----------------------------------------------------------------------------*/
#define CDDA_VOLUME_MAX		(127)		
#define CDDA_VOLUME_MIN		(0)
#define CDDA_VOLUME_DEFAULT	(127)
#define CDDA_VOLUME_RESTOREPREGAMEVALUE (-100)

/* Patrick 10/6/97 --------------------------------------------------------------
  Enumeration of CD player states
  -----------------------------------------------------------------------------*/
typedef enum cdoperationstates
{
	CDOp_Idle,
	CDOp_Playing,
}
CDOPERATIONSTATES;

/* Patrick 10/6/97 --------------------------------------------------------------
  CDDA_Start/End are used to initialise and de-initialise the CDDA system.
  No CDDA operations have any effect until the system has been initialised.
  -----------------------------------------------------------------------------*/
extern void CDDA_Start(void);
extern void CDDA_End(void);
/* Patrick 10/6/97 --------------------------------------------------------------
 This is provided to allow platform specific polling/management of the CD device
 whilst playing. It should be called during the main game loop. 
  -----------------------------------------------------------------------------*/
extern void CDDA_Management(void);
/* Patrick 10/6/97 --------------------------------------------------------------
  Play , change volume, and stop are the basic CDDA operations provided. An
  enumeration of tracks should be provided in the platform header.
  -----------------------------------------------------------------------------*/
extern void CDDA_Play(int CDDATrack);
extern void CDDA_PlayLoop(int CDDATrack);
extern void CDDA_ChangeVolume(int volume);
extern void CDDA_Stop(void);
extern int CDDA_CheckNumberOfTracks(void);
/* Patrick 23/6/97 --------------------------------------------------------------
  Returns the current CDDA volume setting.  NB if the cd player has not been 
  initialised the volume setting can still be obtained by calling this function,
  though it may not be changed using CDDA_ChangeVolume(). 
  -----------------------------------------------------------------------------*/
extern int CDDA_GetCurrentVolumeSetting(void);
/* Patrick 10/6/97 --------------------------------------------------------------
  Switch on and switch off may be used to stop and start the CDDA system after
  initialisation. They are provided to allow the user to stop and start CDDA
  during a game.
  -----------------------------------------------------------------------------*/
extern void CDDA_SwitchOn(void);
extern void CDDA_SwitchOff(void);
/* Patrick 10/6/97 --------------------------------------------------------------
  These are provided to interrogate the state of the CDDA system.
  -----------------------------------------------------------------------------*/
extern int CDDA_IsOn(void);
extern int CDDA_IsPlaying(void);



enum CDCOMMANDID
{
	CDCOMMANDID_Start,
	CDCOMMANDID_End,
	CDCOMMANDID_Play,
	CDCOMMANDID_PlayLoop,
	CDCOMMANDID_ChangeVolume,
	CDCOMMANDID_Stop,
};

/* CDDA SUPPORT */

#define VOLUME_CDDA_MAXPLAT			(65535)
#define VOLUME_CDDA_MINPLAT			(0)	

/* Patrick 10/6/97 -------------------------------------------------------------
  Start and end functions provide any platform specific initialisation for
  the CD player. The start function returns SOUND_PLATFORM error if unsucessful,
  or zero otherwise.
  ----------------------------------------------------------------------------*/
extern int PlatStartCDDA(void);
extern void PlatEndCDDA(void);
/* Patrick 10/6/97 -------------------------------------------------------------
  Platform specific play, stop, and change volume functions. NB the volume
  is scaled to the platform limits defined above.  
  These functions return SOUND_PLATFORM error if unsucessful, or 0 otherwise.
  ----------------------------------------------------------------------------*/
extern int PlatPlayCDDA(int track);
extern int PlatStopCDDA(void);
extern int PlatChangeCDDAVolume(int volume);
int PlatGetNumberOfCDTracks(int* numTracks);
/* Patrick 10/6/97 -------------------------------------------------------------
  Management functions are provided for platform specific detection of changes
  in the cd player state (ie finishing a track, or an error).  The basic 
  management function is provided for consoles, who need to poll the device,
  whilst the call back is provided for intercepting WIN95 MCI call backs. 
  ----------------------------------------------------------------------------*/
extern void PlatCDDAManagement(void);
extern void PlatCDDAManagementCallBack(WPARAM flags, LONG deviceId);



extern int CDPlayerVolume;

#endif
