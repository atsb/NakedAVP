#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "fixer.h"
#include "win95/cd_player.h"
#include "cdplayer.h"

/* cd_player.cpp */
int CDPlayerVolume;

#if SDL_MAJOR_VERSION < 2
static int HaveCDROM = 0;
static SDL_CD *cdrom = NULL;

/* ** */

void CheckCDVolume()
{
/*
	fprintf(stderr, "CheckCDVolume()\n");
*/
}

/* ** */

void CDDA_Start()
{
/*
	fprintf(stderr, "CDDA_Start()\n");
*/

	int numdrives;
	
	if (!HaveCDROM) {
		HaveCDROM = 1;
		SDL_InitSubSystem(SDL_INIT_CDROM);
	}
	
	if (cdrom != NULL)
		CDDA_End();
	
	numdrives = SDL_CDNumDrives();
	
	if (numdrives == 0)
		return;
	
	cdrom = SDL_CDOpen(0);
}

void CDDA_End()
{
/*
	fprintf(stderr, "CDDA_End()\n");
*/

	if (cdrom != NULL) {
		CDDA_Stop();
		
		SDL_CDClose(cdrom);
	}
	
	cdrom = NULL;
}

void CDDA_ChangeVolume(int volume)
{
	fprintf(stderr, "CDDA_ChangeVolume(%d)\n", volume);
}

int CDDA_CheckNumberOfTracks()
{
/*
	fprintf(stderr, "CDDA_CheckNumberOfTracks()\n");
*/

	if (cdrom == NULL)
		return 0;
			
	return cdrom->numtracks;
}

int CDDA_IsOn()
{
/*
	fprintf(stderr, "CDDA_IsOn()\n");
*/	
	return (cdrom != NULL);
}

int CDDA_IsPlaying()
{
/*
	fprintf(stderr, "CDDA_IsPlaying()\n");
*/	
	if (cdrom == NULL)
		return 0;

	return (SDL_CDStatus(cdrom) == CD_PLAYING);
}

void CDDA_Play(int CDDATrack)
{
/*
	fprintf(stderr, "CDDA_Play(%d)\n", CDDATrack);
*/
	if (cdrom == NULL)
		return;
		
	if (CD_INDRIVE(SDL_CDStatus(cdrom))) {
		int track = CDDATrack - 1;
		int i;
		
		if (cdrom->numtracks == 0)
			return;
		
		track %= cdrom->numtracks;
		
		for (i = 0; i < cdrom->numtracks; i++) {
			if (cdrom->track[track].type == SDL_AUDIO_TRACK) {
				SDL_CDPlayTracks(cdrom, track, 0, 1, 0);
				return;
			}
			
			track++;
			track %= cdrom->numtracks;			
		}
	}
}

void CDDA_PlayLoop(int CDDATrack)
{
	fprintf(stderr, "CDDA_PlayLoop(%d)\n", CDDATrack);
	
	/* can't loop with SDL without a thread, so just play the track */
	CDDA_Play(CDDATrack);
}

void CDDA_Stop()
{
/*
	fprintf(stderr, "CDDA_Stop()\n");
*/
	if (cdrom == NULL)
		return;
	
	if (CD_INDRIVE(SDL_CDStatus(cdrom)))
		SDL_CDStop(cdrom);	
}

void CDDA_SwitchOn()
{
/*
	fprintf(stderr, "CDDA_SwitchOn()\n");
*/	
}

#else

// What's a CD?

void CheckCDVolume()
{
}

/* ** */

void CDDA_Start()
{
}

void CDDA_End()
{
}

void CDDA_ChangeVolume(int volume)
{
}

int CDDA_CheckNumberOfTracks()
{
	return 0;
}

int CDDA_IsOn()
{
	return 0;
}

int CDDA_IsPlaying()
{
	return 0;
}

void CDDA_Play(int CDDATrack)
{
}

void CDDA_PlayLoop(int CDDATrack)
{
}

void CDDA_Stop()
{
}

void CDDA_SwitchOn()
{
}

#endif
