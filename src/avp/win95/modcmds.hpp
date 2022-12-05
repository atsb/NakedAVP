/*
	
	modcmds.hpp

*/

#ifndef _modcmds
#define _modcmds 1

	#ifndef MODULE_INCLUDED
	#include "module.h"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	namespace ModuleCommands
	{
		void ListModules(void);
		
		void TryToTeleport(char* UpperCasePotentialModuleName);

		MODULE* FindModule(char* UpperCasePotentialModuleName);
			// allowed to return NULL if no match

		void TeleportPlayerToModule(MODULE* pModule_Dst);
	};
	


/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
