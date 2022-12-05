#include "3dc.h"
#include "inline.h"
#include "module.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "psndplat.h"

#include "list_tem.hpp"
#include "jsndsup.h"

extern "C"
{
	extern int SoundSwitchedOn;
	// Pat sets this up
};

List <LOADED_SOUND *> loaded_sounds;

void LoseSound (LOADED_SOUND const * ls)
{
	LOADED_SOUND ** ucls_p = (LOADED_SOUND **) &ls;
	LOADED_SOUND * ucls = *ucls_p;

	if (loaded_sounds.contains (ucls))
	{
		LOADED_SOUND * ls_in_list = loaded_sounds.similar_entry (ucls);
		ls_in_list->num_attached --;
		
		if (ls_in_list->permanent == 1)
		{
			if (ls_in_list->num_attached <= 0)
			{
				loaded_sounds.delete_entry (ls_in_list);
				DeallocateMem (ls_in_list->wavname);
				DeallocateMem (ls_in_list);
			}
		
			return;
		}
		
		if (ls_in_list->num_attached <= 0)
		{
			PlatEndGameSound ((SOUNDINDEX)ls_in_list->sound_num);
			loaded_sounds.delete_entry (ls_in_list);
			DeallocateMem (ls_in_list->wavname);
			DeallocateMem (ls_in_list);
		}
	}
	else
	{
		GLOBALASSERT (0);
	}
}

void LoseAllNonCommonSounds()
{
	LOADED_SOUND* ls;
	while(loaded_sounds.size())
	{
		ls=loaded_sounds.first_entry();
		loaded_sounds.delete_first_entry();		

		if(!ls->permanent)
		{
			PlatEndGameSound ((SOUNDINDEX)ls->sound_num);
		}

		DeallocateMem (ls->wavname);
		DeallocateMem (ls);
	}
}

static int find_empty_game_sound()
{
	if(!SoundSwitchedOn) return (-1);

	for (int i=SID_STARTOF_LOADSLOTS; i<=SID_ENDOF_LOADSLOTS; i++)
	{
		if (GameSounds[i].loaded == 0)
		{
			return(i);
		}
	}
	return(-1);
	
}

static int find_permanent_game_sound(const char * wavname)
{
	if(!SoundSwitchedOn) return (-1);

	for (int i=0; i<SID_MAXIMUM; i++)
	{
		if (GameSounds[i].wavName)
		{
			if (!(_stricmp( GameSounds[i].wavName, wavname)))
			{
				return(i);
			}
		}
	}
	return(-1);
	
}


LOADED_SOUND const * GetSound (char const * fname)
{
	if(!SoundSwitchedOn) return (0);

	const char * wavname = strrchr (fname, '\\');
	
	if (wavname)
	{
		wavname ++;
	}
	else
	{
		wavname = fname;
	}
	
	// check if wavname already loaded
	
	for (LIF<LOADED_SOUND *> lsi(&loaded_sounds); !lsi.done(); lsi.next())
	{
		if (!_stricmp (lsi()->wavname, wavname))
		{
			lsi()->num_attached ++;
			return(lsi());
		}
	}
	
	LOADED_SOUND * ls = 0;

	int perm_snum = find_permanent_game_sound (wavname);
	
	if (perm_snum != -1)
	{
		ls = (LOADED_SOUND *) AllocateMem(sizeof (LOADED_SOUND));
		
		ls->sound_num = perm_snum;
		ls->wavname = (char *) AllocateMem(sizeof (char) * (strlen (wavname)+1));
		strcpy (ls->wavname, wavname);
		ls->num_attached = 1;
		ls->permanent = 1;
		loaded_sounds.add_entry (ls);
		return (ls);
	}
	
	// not loaded, so try and load it
	
	int soundNum = find_empty_game_sound();
	if (soundNum == -1)
	{
		GLOBALASSERT(0=="Run out of sound slots");
		return(0);
	}
	
	int ok = FindAndLoadWavFile (soundNum, (char *)fname);


	if (ok)
	{
		GameSounds[soundNum].loaded = 1;
		GameSounds[soundNum].activeInstances = 0;;	 
		GameSounds[soundNum].volume = VOLUME_DEFAULT;		
		GameSounds[soundNum].pitch = 0;
		InitialiseBaseFrequency((SOUNDINDEX)soundNum);

		ls = (LOADED_SOUND *) AllocateMem(sizeof (LOADED_SOUND));
		
		ls->sound_num = soundNum;
		ls->wavname = (char *)AllocateMem(sizeof (char) * (strlen (wavname)+1));
		strcpy (ls->wavname, wavname);
		ls->num_attached = 1;
		ls->permanent = 0;
		loaded_sounds.add_entry (ls);
	}
	
	return(ls);
	
}
