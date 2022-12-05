/* KJL 10:19:41 30/03/98 - ConsoleLog.cpp

	This file handles the mirroring of the console text 
	to a log file.
	
*/
#include <stdlib.h>
#include <string.h>
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "debuglog.hpp"
#include "consolelog.hpp"

#include "bh_types.h"
#include "inventry.h"
#include "bh_alien.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_fhug.h"
#include "bh_marin.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "bh_agun.h"
#include "weapons.h"

#if 0
static LogFile ConsoleLogFile("ConsoleLog.txt");
#endif

extern "C"
{
	int LogConsoleTextToFile;
extern void OutputBugReportToConsoleLogfile(char *messagePtr)
{
#if 0
	extern MODULE *playerPherModule;
	extern struct Target PlayersTarget;

	ConsoleLogFile.lprintf("\n*** AvP Automated Bug Report ****\n\n");
	ConsoleLogFile.lprintf("Comment: %s\n\n", (char const*)messagePtr);

	ConsoleLogFile.lprintf("Environment: %s\n", (char const*)Env_List[AvP.CurrentEnv]->main );
	ConsoleLogFile.lprintf("Game type: ");

	if (AvP.Network != I_No_Network)
	{
	 	ConsoleLogFile.lprintf("Multiplayer\n");
	}
	else
	{
	 	ConsoleLogFile.lprintf("Single player\n");
	}


	ConsoleLogFile.lprintf("Player's Species: ");
	switch(AvP.PlayerType)
	{
		case I_Marine:
			ConsoleLogFile.lprintf("Marine\n");
			break;
			
		case I_Alien:
			ConsoleLogFile.lprintf("Alien\n");
			break;
		
		case I_Predator:
			ConsoleLogFile.lprintf("Predator\n");
			break;
	}

	ConsoleLogFile.lprintf("\nPlayer's Coords: %d,%d,%d\n",Player->ObWorld.vx,Player->ObWorld.vy,Player->ObWorld.vz);
 	ConsoleLogFile.lprintf("Player's Module: %d '%s'\n", playerPherModule->m_index,playerPherModule->name);
 	ConsoleLogFile.lprintf("Player's Module Coords: %d %d %d\n",playerPherModule->m_world.vx,playerPherModule->m_world.vy,playerPherModule->m_world.vz);
	ConsoleLogFile.lprintf("Player's Target: %d %d %d\n",PlayersTarget.Position.vx,PlayersTarget.Position.vy,PlayersTarget.Position.vz);
	ConsoleLogFile.lprintf("\n");
#endif
}

extern void OutputToConsoleLogfile(char *messagePtr)
{
#if 0
	if(LogConsoleTextToFile)
	{
		ConsoleLogFile.lprintf("%s\n", (char const*)messagePtr);
	}
#endif	
}

};
