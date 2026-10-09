#ifndef _jsndsup_h
#define _jsndsup_h 1

#include "3dc.h"
#include "inline.h"
#include "module.h"

#ifdef __cplusplus

	extern "C" {

#endif

typedef struct loaded_sound
{
	int sound_num;
	
	char * wavname;
	int num_attached;
	
	unsigned long permanent :1;
	
} LOADED_SOUND;


void LoseSound (LOADED_SOUND const * ls);
LOADED_SOUND const * GetSound (char const * fname);

#ifdef __cplusplus

	}; // end of extern "c"

#endif


#endif
