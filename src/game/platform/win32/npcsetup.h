#ifndef _included_npcsetup_h_
#define _included_npcsetup_h_

#include "chnkload.h" /* for RIFFHANDLE type */

#ifdef __cplusplus
extern "C" {
#endif

/* pass handle to environment rif */
void InitNPCs(RIFFHANDLE);

/* unload them all after intance of game */
void EndNPCs();

#ifdef __cplusplus
}
#endif

#endif /* ! _included_npcsetup_h_ */
