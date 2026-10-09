extern "C"
{
#include "3dc.h"
#include "ourasert.h"
#include "psndplat.h"
#include "dxlog.h"
#include "cd_player.h"
#include "avp_menus.h"
#include "gamedef.h"

#include "avp_envinfo.h"
};

#include "list_tem.hpp"

//lists of tracks for each level
List<int> LevelCDTracks[AVP_ENVIRONMENT_END_OF_LIST];

//lists of tracks for each species in multiplayer games
List<int> MultiplayerCDTracks[3];

static int LastTrackChosen=-1;

extern "C"
{

void EmptyCDTrackList()
{
	for(int i=0;i<AVP_ENVIRONMENT_END_OF_LIST;i++)
	{
		while(LevelCDTracks[i].size()) LevelCDTracks[i].delete_first_entry();
	}

	for(int i=0;i<3;i++)
	{
		while(MultiplayerCDTracks[i].size()) MultiplayerCDTracks[i].delete_first_entry();
	}
}

#define CDTrackFileName "cd tracks.txt"


static void ExtractTracksForLevel(char* & buffer,List<int> & track_list)
{
	//search for a line starting with a #
	while(*buffer)
	{
		if(*buffer=='#') break;
		//search for next line
		while(*buffer)
		{
			if(*buffer=='\n')
			{
				buffer++;
				if(*buffer=='\r') buffer++;
				break;
			}
			buffer++;
		}

	}
	
	while(*buffer)
	{
		//search for a track number or comment
		if(*buffer==';')
		{
			//comment , so no further info on this line
			break;
		}
		else if(*buffer=='\n' || *buffer=='\r')
		{
			//reached end of line
			break;
		}
		else if(*buffer>='0' && *buffer<='9')
		{
			int track=-1;
			//find a number , add it to the list
			sscanf(buffer,"%d",&track);

			if(track>=0)
			{
				track_list.add_entry(track);
			}

			//skip to the next non numerical character
			while(*buffer>='0' && *buffer<='9') buffer++;
		}
		else
		{
			buffer++;
		}
	}

	//go to the next line
	while(*buffer)
	{
		if(*buffer=='\n')
		{
			buffer++;
			if(*buffer=='\r') buffer++;
			break;
		}
		buffer++;
	}
	
}

void LoadCDTrackList()
{
	//clear out the old list first
	EmptyCDTrackList();

	FILE *file = OpenGameFile(CDTrackFileName, FILEMODE_READONLY, FILETYPE_OPTIONAL);
	
	if(file==NULL)
	{
		LOGDXFMT(("Failed to open %s",CDTrackFileName));
		return;
	}

	char* buffer;
	int file_size;

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	//copy the file contents into a buffer
	buffer=new char[file_size+1];
	fread(buffer, 1, file_size, file);
	buffer[file_size]='\0';
	fclose(file);
	
	char* bufferptr=buffer;


	//first extract the multiplayer tracks
	for(int i=0;i<3;i++)
	{
		ExtractTracksForLevel(bufferptr,MultiplayerCDTracks[i]);
	}
	
	//now the level tracks
	for(int i=0 ;i<AVP_ENVIRONMENT_END_OF_LIST;i++)
	{
		ExtractTracksForLevel(bufferptr,LevelCDTracks[i]);
	}
	

	delete [] buffer;
}

static unsigned int TrackSelectCounter=0;

static BOOL PickCDTrack(List<int>& track_list)
{
	//make sure we have some tracks in the list
	if(!track_list.size()) return FALSE;

	//pick the next track in the list
	unsigned int index=TrackSelectCounter % track_list.size();

	TrackSelectCounter++;

	//play it
	CDDA_Stop();
	CDDA_Play(track_list[index]);

	LastTrackChosen = track_list[index];
	return TRUE;	
}


void CheckCDAndChooseTrackIfNeeded()
{
	static enum playertypes lastPlayerType;
	
	//are we bothering with cd tracks
	if(!CDDA_IsOn()) return;
	//is our current track still playing
	if(CDDA_IsPlaying())
	{
		//if in a multiplayer game see if we have changed character type
		if(AvP.Network == I_No_Network || AvP.PlayerType==lastPlayerType) 
			return;
		
		//have changed character type , is the current track in the list for this character type
		if(MultiplayerCDTracks[AvP.PlayerType].contains(LastTrackChosen)) 
			return;

		//Lets choose a new track then
	}

		
	if(AvP.Network == I_No_Network)
	{
		int level=NumberForCurrentLevel();
		if(level>=0 && level<AVP_ENVIRONMENT_END_OF_LIST)
		{
			//pick track based on level
			if(PickCDTrack(LevelCDTracks[level]))
			{
				return;
			}
			
		}
	}
	
	
	//multiplayer (or their weren't ant level specific tracks)
	lastPlayerType=AvP.PlayerType;
	PickCDTrack(MultiplayerCDTracks[AvP.PlayerType]);
	
	 	
}

void ResetCDPlayForLevel()
{
	//check the number of tracks available while we're at it
	CDDA_CheckNumberOfTracks();

	TrackSelectCounter=0;
	CDDA_Stop();
}

};
