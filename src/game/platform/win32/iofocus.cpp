/*******************************************************************
 *
 *    DESCRIPTION: 	iofocus.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 21/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "iofocus.h"
#include "gadget.h"
#include "psnd.h"
extern "C"
{
#include "avp_menus.h"
	
	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/

/* Constants *******************************************************/

/* Macros **********************************************************/

/* Imported function prototypes ************************************/
extern int InGameMenusAreRunning(void);

/* Imported data ***************************************************/


/* Exported globals ************************************************/

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/
	static OurBool iofocus_AcceptTyping = No;

/* Exported function definitions ***********************************/
OurBool IOFOCUS_AcceptControls(void)
{
	return !iofocus_AcceptTyping;
}

OurBool IOFOCUS_AcceptTyping(void)
{
	return iofocus_AcceptTyping;
}

void IOFOCUS_Toggle(void)
{
	#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED||!(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
	if(InGameMenusAreRunning()) return;;

	iofocus_AcceptTyping = !iofocus_AcceptTyping;
	if (iofocus_AcceptTyping)
	{
		Sound_Play(SID_CONSOLE_ACTIVATES,NULL);
	}
	else
	{
		Sound_Play(SID_CONSOLE_DEACTIVATES,NULL);
		RemoveTheConsolePlease();
	}
	#endif
}


/* Internal function definitions ***********************************/

};
