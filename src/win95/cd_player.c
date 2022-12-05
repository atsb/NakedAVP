#include "3dc.h"
#include "inline.h"
#include "psndplat.h"
#include "cd_player.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/* KJL 12:40:35 07/05/98 - This is code derived from Patrick's original stuff &
moved into it's own file. */

/* Patrick 10/6/97 -------------------------------------------------------------
  CDDA Support
  ----------------------------------------------------------------------------*/
#define NO_DEVICE -1
int cdDeviceID = NO_DEVICE;
int cdAuxDeviceID = NO_DEVICE;

/* Patrick 9/6/97 -------------------------------------------------------------
   ----------------------------------------------------------------------------
  CDDA Support
  -----------------------------------------------------------------------------
  -----------------------------------------------------------------------------*/
static int CDDASwitchedOn = 0;
static int CDDAIsInitialised = 0;
static int CDDAVolume = CDDA_VOLUME_DEFAULT;
CDOPERATIONSTATES CDDAState; 

static DWORD PreGameCDVolume;//windows cd volume before the game started

static CDTRACKID TrackBeingPlayed;
static enum CDCOMMANDID LastCommandGiven;

extern HWND hWndMain;

int CDPlayerVolume; // volume control from menus

int CDTrackMax=-1; //highest track number on cd

void CDDA_Start(void)
{
	CDDAVolume = CDDA_VOLUME_DEFAULT;
	CDPlayerVolume = CDDAVolume;
	CDDAState = CDOp_Idle;
	CDDAIsInitialised = 0; 
	if(PlatStartCDDA()!=SOUND_PLATFORMERROR)
	{
		CDDAIsInitialised = 1; 
		CDDA_SwitchOn();
		CDDA_ChangeVolume(CDDAVolume); /* init the volume */
		CDDA_CheckNumberOfTracks();
	} 
	LastCommandGiven = CDCOMMANDID_Start;
}

void CDDA_End(void)
{
	if(!CDDAIsInitialised) return;
	
	CDDA_Stop();
	PlatChangeCDDAVolume(CDDA_VOLUME_RESTOREPREGAMEVALUE);
	PlatEndCDDA();
	CDDA_SwitchOff(); 	
	CDDAIsInitialised = 0;

	LastCommandGiven = CDCOMMANDID_End;
}

void CDDA_Management(void)
{
	if(!CDDASwitchedOn) return; /* CDDA is off */
	if(CDDAState==CDOp_Playing) return; /* already playing */
	PlatCDDAManagement();
}

void CDDA_Play(int CDDATrack)
{
	int ok;
	
	if(!CDDASwitchedOn) return; /* CDDA is off */
	if(CDDAState==CDOp_Playing) return; /* already playing */
	if((CDDATrack<=0)||(CDDATrack>=CDTrackMax)) return; /* no such track */

	ok = PlatPlayCDDA((int)CDDATrack);
	if(ok!=SOUND_PLATFORMERROR)
	{
		CDDAState=CDOp_Playing;
		LastCommandGiven = CDCOMMANDID_Play;
		TrackBeingPlayed = CDDATrack;
	}
}
void CDDA_PlayLoop(int CDDATrack)
{
	int ok;
	
	if(!CDDASwitchedOn) return; /* CDDA is off */
	if(CDDAState==CDOp_Playing) return; /* already playing */
	if((CDDATrack<=0)||(CDDATrack>=CDTrackMax)) return; /* no such track */

	ok = PlatPlayCDDA((int)CDDATrack);
	if(ok!=SOUND_PLATFORMERROR)
	{
		CDDAState=CDOp_Playing;
		LastCommandGiven = CDCOMMANDID_PlayLoop;
		TrackBeingPlayed = CDDATrack;
	}
}

extern void CheckCDVolume(void)
{
	if (CDDAVolume != CDPlayerVolume)
	{
		CDDA_ChangeVolume(CDPlayerVolume);
	}
}
void CDDA_ChangeVolume(int volume)
{
	if(!CDDASwitchedOn) return; /* CDDA is off */
	if(volume<CDDA_VOLUME_MIN) return;
	if(volume>CDDA_VOLUME_MAX) return;

	if(CDDA_IsOn()) 
	{
		if(PlatChangeCDDAVolume(volume))
		{
			CDDAVolume=volume;
			CDPlayerVolume = volume;
			LastCommandGiven = CDCOMMANDID_ChangeVolume;
		}
	}
}

int CDDA_GetCurrentVolumeSetting(void)
{
	return CDDAVolume;
}

void CDDA_Stop()
{	
	int ok;
	if(!CDDASwitchedOn) return; /* CDDA is off */
	if(CDDAState!=CDOp_Playing) return; /* nothing playing */
	ok = PlatStopCDDA();
	CDDAState=CDOp_Idle;
	LastCommandGiven = CDCOMMANDID_Stop;
}

void CDDA_SwitchOn()
{
	LOCALASSERT(!CDDA_IsPlaying());
	if(CDDAIsInitialised) CDDASwitchedOn = 1;	
}

void CDDA_SwitchOff()
{
	if(!CDDASwitchedOn) return; /* CDDA is off already */
	if(CDDA_IsPlaying()) CDDA_Stop();
	CDDASwitchedOn = 0;	
}

int CDDA_IsOn()
{
	return CDDASwitchedOn;
}

int CDDA_IsPlaying()
{
	if(CDDAState==CDOp_Playing) 
	{
		LOCALASSERT(CDDASwitchedOn);
		return 1;
	}
	return 0;
}

int CDDA_CheckNumberOfTracks()
{
	int numTracks=0;

	if(CDDA_IsOn()) 
	{
		PlatGetNumberOfCDTracks(&numTracks);

		//if there is only one track , then it probably can't be used anyway
		if(numTracks==1) numTracks=0;

		//store the maximum allowed track number
		CDTrackMax=numTracks;
	}
	return numTracks;
}



/* win95 specific */

int PlatStartCDDA(void)
{
	static void PlatGetCDDAVolumeControl(void);
	DWORD dwReturn;
	MCI_OPEN_PARMS mciOpenParms;

	/* Initialise device handles */
	cdDeviceID = NO_DEVICE;
	cdAuxDeviceID = NO_DEVICE;

	/* try to open mci cd-audio device */
	mciOpenParms.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;
	dwReturn = mciSendCommand(NULL,MCI_OPEN,MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID,(DWORD)(LPVOID)&mciOpenParms);
	if(dwReturn) 
	{
		/* error */
		cdDeviceID = NO_DEVICE;
		return SOUND_PLATFORMERROR;
	}
	cdDeviceID = mciOpenParms.wDeviceID;  
	
	/* now try to get the cd volume control, by obtaining the auxiliary device id for
	the cd-audio player*/
	PlatGetCDDAVolumeControl();
	return 0;
}

/* this is a support function for PlatStartCDDA() */
#if 0
static void PlatGetCDDAVolumeControl(void)
{
	MMRESULT mmres;
	unsigned int numAuxDevs,i;
	
	numAuxDevs = auxGetNumDevs();
	/* search the auxilary device list for the cd player */
	for(i=0;i<numAuxDevs;i++)
	{
		AUXCAPS devCaps;
		mmres = auxGetDevCaps(i,&devCaps,sizeof(AUXCAPS));
		if(mmres==MMSYSERR_NOERROR)
		{
			if((devCaps.wTechnology&AUXCAPS_CDAUDIO)&&(devCaps.dwSupport&AUXCAPS_VOLUME))
			{								
						cdAuxDeviceID = i;					
			}
		}
	}
}
#else
static void PlatGetCDDAVolumeControl(void)
{
	int i;
	int numDev = mixerGetNumDevs();


	//go through the mixer devices searching for one that can deal with the cd volume
	for(i=0;i<numDev;i++)
	{
		HMIXER handle;
		if(mixerOpen(&handle,i,0,0,MIXER_OBJECTF_MIXER ) == MMSYSERR_NOERROR )
		{
			
			//try to get the compact disc mixer line
			MIXERLINE line;
			line.cbStruct=sizeof(MIXERLINE);
			line.dwComponentType=MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

			if(mixerGetLineInfo(handle,&line,MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
			{
				MIXERLINECONTROLS lineControls;
				MIXERCONTROL control;


				lineControls.cbStruct=sizeof(MIXERLINECONTROLS);
				lineControls.dwLineID=line.dwLineID;
				lineControls.pamxctrl=&control;
				lineControls.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME ;
				lineControls.cControls=1;
				lineControls.cbmxctrl=sizeof(MIXERCONTROL);
				
				 control.cbStruct=sizeof(MIXERCONTROL);


				//try to get the volume control
				if(mixerGetLineControls(handle,&lineControls,MIXER_GETLINECONTROLSF_ONEBYTYPE)==MMSYSERR_NOERROR)
				{

					MIXERCONTROLDETAILS details;
					MIXERCONTROLDETAILS_UNSIGNED detailValue;

					details.cbStruct=sizeof(MIXERCONTROLDETAILS);
					details.dwControlID=control.dwControlID;
					details.cChannels=1;
					details.cMultipleItems=0;
					details.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					details.paDetails=&detailValue;
					
					//get the current volume so that we can restore it later
					if(mixerGetControlDetails(handle,&details,MIXER_GETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR)
					{
						PreGameCDVolume = detailValue.dwValue;
						mixerClose(handle);
						
						return; //success
					}
				}
			}
			 
			
			mixerClose(handle);
		}

	}

	return;
}
#endif


void PlatEndCDDA(void)
{	
	DWORD dwReturn;

    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return;	

	dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_CLOSE,MCI_WAIT,NULL);	
	cdDeviceID=NO_DEVICE;

	/* reset the auxilary device handle */
	cdAuxDeviceID = NO_DEVICE;
}

int PlatPlayCDDA(int track)
{
	DWORD dwReturn;
    MCI_SET_PARMS mciSetParms = {0,0,0};
    MCI_PLAY_PARMS mciPlayParms = {0,0,0};
	MCI_STATUS_PARMS mciStatusParms = {0,0,0,0};

    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return SOUND_PLATFORMERROR;
    
    /* set the time format */
	mciSetParms.dwTimeFormat = MCI_FORMAT_MSF;
	dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_SET,MCI_SET_TIME_FORMAT,(DWORD)(LPVOID) &mciSetParms);
	if(dwReturn)
	{
//    	NewOnScreenMessage("CD ERROR - TIME FORMAT");
    	/* error */
    	return SOUND_PLATFORMERROR;
	}  
	
	/* find the length of the track... */
	mciStatusParms.dwItem = MCI_STATUS_LENGTH;
	mciStatusParms.dwTrack = track;
    dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_TRACK,(DWORD)(LPVOID)&mciStatusParms);
	if(dwReturn)
	{
    	/* error */
//    	NewOnScreenMessage("CD ERROR - GET LENGTH");
    	return SOUND_PLATFORMERROR;
	}  

    /* set the time format */
	mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
	dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_SET,MCI_SET_TIME_FORMAT,(DWORD)(LPVOID) &mciSetParms);
	if(dwReturn)
	{
    	/* error */
//    	NewOnScreenMessage("CD ERROR - TIME FORMAT");
    	return SOUND_PLATFORMERROR;
	}  

	/* play the track: set up for notification when track finishes, or an error occurs */ 
    mciPlayParms.dwFrom=MCI_MAKE_TMSF(track,0,0,0);
    mciPlayParms.dwTo=MCI_MAKE_TMSF(track,	MCI_MSF_MINUTE(mciStatusParms.dwReturn),
    										MCI_MSF_SECOND(mciStatusParms.dwReturn),
    										MCI_MSF_FRAME(mciStatusParms.dwReturn));
    mciPlayParms.dwCallback=(DWORD)hWndMain;
    dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_PLAY,MCI_FROM|MCI_TO|MCI_NOTIFY,(DWORD)(LPVOID)&mciPlayParms);
	if(dwReturn)
    {
    	/* error */
//    	NewOnScreenMessage("CD ERROR - PLAY");
    	return SOUND_PLATFORMERROR;
    }
    return 0;
}

int PlatGetNumberOfCDTracks(int* numTracks)
{
	DWORD dwReturn;
	MCI_STATUS_PARMS mciStatusParms = {0,0,0,0};
  
    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return SOUND_PLATFORMERROR;
	if(!numTracks) return SOUND_PLATFORMERROR;	


	/* find the number tracks... */
	mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS ;
    dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_STATUS,MCI_STATUS_ITEM ,(DWORD)(LPVOID)&mciStatusParms);
	if(dwReturn)
	{
    	/* error */
    	return SOUND_PLATFORMERROR;
	}  

	//number of tracks is in the dwReturn member
	*numTracks=mciStatusParms.dwReturn;
	

	return 0;
	
}

int PlatStopCDDA(void)
{
	DWORD dwReturn;

    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) 
    {
    	return SOUND_PLATFORMERROR;
	}

	/* stop the cd player */
	dwReturn = mciSendCommand((UINT)cdDeviceID,MCI_STOP,MCI_WAIT,NULL);
	if(dwReturn)
    {
    	/* error */
    	return SOUND_PLATFORMERROR;
    }
    return 0;
}
#if 0
int PlatChangeCDDAVolume(int volume)
{
    MMRESULT mmres;
	unsigned int newVolume;

    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return SOUND_PLATFORMERROR;
    /* check the mixer device id */
	if(cdAuxDeviceID==NO_DEVICE) return SOUND_PLATFORMERROR;

	/* scale and set the new volume */
	{
		int channelVolume;
		channelVolume = VOLUME_CDDA_MINPLAT +  WideMulNarrowDiv(volume,
		(VOLUME_CDDA_MAXPLAT-VOLUME_CDDA_MINPLAT), (CDDA_VOLUME_MAX-CDDA_VOLUME_MIN));
		if(channelVolume < VOLUME_CDDA_MINPLAT) channelVolume = VOLUME_CDDA_MINPLAT;
		if(channelVolume > VOLUME_CDDA_MAXPLAT) channelVolume = VOLUME_CDDA_MAXPLAT;

		/* set left and right channels (if there is only one channel,
		should still work ok)*/
		newVolume = channelVolume|(channelVolume<<16);
	}
	PlatGetCDDAVolumeControl();

	mmres = auxSetVolume((UINT)cdAuxDeviceID,(DWORD)newVolume);
	if(mmres==MMSYSERR_NOERROR) return 1;
	else return SOUND_PLATFORMERROR;	
}
#else
int PlatChangeCDDAVolume(int volume)
{
    MMRESULT mmres;
	unsigned int newVolume;
	int i;
	int numDev = mixerGetNumDevs();

    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return SOUND_PLATFORMERROR;

	//go through the mixer devices searching for one that can deal with the cd volume
	for(i=0;i<numDev;i++)
	{
		HMIXER handle;
		if(mixerOpen(&handle,i,0,0,MIXER_OBJECTF_MIXER ) == MMSYSERR_NOERROR )
		{
			
			//try to get the compact disc mixer line
			MIXERLINE line;
			line.cbStruct=sizeof(MIXERLINE);
			line.dwComponentType=MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

			if(mixerGetLineInfo(handle,&line,MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
			{
				MIXERLINECONTROLS lineControls;
				MIXERCONTROL control;


				lineControls.cbStruct=sizeof(MIXERLINECONTROLS);
				lineControls.dwLineID=line.dwLineID;
				lineControls.pamxctrl=&control;
				lineControls.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME ;
				lineControls.cControls=1;
				lineControls.cbmxctrl=sizeof(MIXERCONTROL);
				
				control.cbStruct=sizeof(MIXERCONTROL);


				//try to get the volume control
				if(mixerGetLineControls(handle,&lineControls,MIXER_GETLINECONTROLSF_ONEBYTYPE)==MMSYSERR_NOERROR)
				{

					MIXERCONTROLDETAILS details;
					MIXERCONTROLDETAILS_UNSIGNED detailValue;

					details.cbStruct=sizeof(MIXERCONTROLDETAILS);
					details.dwControlID=control.dwControlID;
					details.cChannels=1;
					details.cMultipleItems=0;
					details.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					details.paDetails=&detailValue;
										
					
					if(volume==CDDA_VOLUME_RESTOREPREGAMEVALUE)
					{
						//set the volume to what it was before the game started
						newVolume=PreGameCDVolume;
					}
					else
					{
						//scale the volume
						newVolume = control.Bounds.dwMinimum +  WideMulNarrowDiv(volume,
							(control.Bounds.dwMaximum-control.Bounds.dwMinimum), (CDDA_VOLUME_MAX-CDDA_VOLUME_MIN));

						if(newVolume<control.Bounds.dwMinimum) newVolume=control.Bounds.dwMinimum;
						if(newVolume>control.Bounds.dwMaximum) newVolume=control.Bounds.dwMaximum;
					}
					//fill in the volume in the control details structure
					detailValue.dwValue=newVolume;
	
	
					mmres = mixerSetControlDetails(handle,&details,MIXER_SETCONTROLDETAILSF_VALUE);
					mixerClose(handle);

					if(mmres==MMSYSERR_NOERROR) return 1;
					else return SOUND_PLATFORMERROR;	
					
				}
			}
			 
			
			mixerClose(handle);
		}

	}

	return SOUND_PLATFORMERROR;
}

#endif




void PlatCDDAManagement(void)
{
	/* does nothing for Win95: use call back instead */ 
}

void PlatCDDAManagementCallBack(WPARAM flags, LONG deviceId)
{
    extern CDOPERATIONSTATES CDDAState;
    
    /* check the cdDeviceId */
    if(cdDeviceID==NO_DEVICE) return;
	/* compare with the passed device id */
	if((UINT)deviceId!=(UINT)cdDeviceID) return;

	if(flags&MCI_NOTIFY_SUCCESSFUL)
	{
		CDDAState = CDOp_Idle;
		//NewOnScreenMessage("CD COMMAND RETURNED WITH SUCCESSFUL");
		/* Play it again, sam */
		if (LastCommandGiven == CDCOMMANDID_PlayLoop)
		{
			CDDA_PlayLoop(TrackBeingPlayed);
		}
	}
	else if(flags&MCI_NOTIFY_FAILURE)
	{
		/* error while playing: abnormal termination */
		//NewOnScreenMessage("CD COMMAND FAILED");
		CDDAState = CDOp_Idle;
	}
	else if(flags&MCI_NOTIFY_SUPERSEDED)
	{
		//NewOnScreenMessage("CD COMMAND SUPERSEDED");
	}
	else if(flags&MCI_NOTIFY_ABORTED)
	{
		/* aborted or superceeded: try and stop the device */
		//NewOnScreenMessage("CD COMMAND ABORTED(?)");
	  //	CDDA_Stop();
	}
	else
	{
		//NewOnScreenMessage("CD COMMAND RETURNED WITH UNKNOWN MESSAGE");
	}
}
