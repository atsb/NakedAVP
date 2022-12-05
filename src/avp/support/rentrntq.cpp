/*******************************************************************
 *
 *    DESCRIPTION: 	rentrntq.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 21/11/97
 *		(refer to comment in RENTRTQ.H from a description of what
 *		this is for)
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "gadget.h"

#include "fixer.h"

#include "rentrntq.h"

	#if UseGadgets
	#include "iofocus.h"
	#include "hudgadg.hpp"
	#include "textin.hpp"
	#include "consbind.hpp"
	#endif

	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/

/* Constants *******************************************************/
	#define MAX_Q_MESSAGES (256)

/* Macros **********************************************************/

/* Imported function prototypes ************************************/

/* Imported data ***************************************************/

/* Exported globals ************************************************/

/* Internal type definitions ***************************************/
	enum QEntryCategory
	{
		QEntryCat_WM_CHAR,
		QEntryCat_WM_KEYDOWN,

		NUM_Q_ENTRY_CATS
	};

	struct Q_Entry
	{
		enum QEntryCategory QEntryCat;

		union
		{
			char Ch;
				// Valid for: QEntryCat_WM_CHAR

			WPARAM wParam;
				// Valid for: QEntryCat_WM_KEYDOWN
		} CatData; 
	};

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/
	static struct Q_Entry OurQ[MAX_Q_MESSAGES];
	static unsigned int NumQMessages = 0;

extern "C" {
	void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(char Ch);
	void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(WPARAM wParam);
};
	
/* Exported function definitions ***********************************/
/* Functions callable within the Windows procedure */
void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR
(
	char Ch
)
{
	if ( NumQMessages < MAX_Q_MESSAGES )
	{
		OurQ[ NumQMessages ] . QEntryCat = QEntryCat_WM_CHAR;
		OurQ[ NumQMessages ] . CatData . Ch = Ch;
		NumQMessages++;
	}
	// otherwise ignore the message
	// since no decent error handling facilties/memory allocators
	// can safely be called at this stage
}

void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN
(
	WPARAM wParam
)
{
	if ( NumQMessages < MAX_Q_MESSAGES )
	{
		OurQ[ NumQMessages ] . QEntryCat = QEntryCat_WM_KEYDOWN;
		OurQ[ NumQMessages ] . CatData . wParam = wParam;
		NumQMessages++;
	}
	// otherwise ignore the message
	// since no decent error handling facilties/memory allocators
	// can safely be called at this stage
}


/* Functions callable from the WinMain() body of code */
void RE_ENTRANT_QUEUE_WinMain_FlushMessages(void)
{
	// Process the messages:
	{
		#if UseGadgets
		// AVP/Win95-specific code
		if ( HUDGadget :: GetHUD() )
		{
			unsigned int i;

			for (i=0;i<NumQMessages;i++)
			{
				switch ( OurQ[ i ] . QEntryCat )
				{
					case QEntryCat_WM_CHAR:
					{
						char Ch = OurQ[ i ] . CatData . Ch;

						if
						(
							Ch != '`'
								// hack to ignore the grave key
						)
						{
							if ( IOFOCUS_AcceptTyping() )
							{
								HUDGadget :: GetHUD() -> CharTyped
								(
									Ch
								);
							}
						}
						#if 0
						textprint
						(
							"\n\n\nWM_CHAR message flushed; code=0x%x\n\n\n",
							Ch
						);
						#endif
					}
					break;

					case QEntryCat_WM_KEYDOWN:
					{
						if ( IOFOCUS_AcceptTyping() )
						{
							switch ( OurQ[i] . CatData . wParam )
							{
								default:
									// ignore most keys;
									break;

								case VK_BACK:
									HUDGadget :: GetHUD() -> Key_Backspace();
									break;
								case VK_END:
									HUDGadget :: GetHUD() -> Key_End();
									break;
								case VK_HOME:
									HUDGadget :: GetHUD() -> Key_Home();
									break;
								case VK_LEFT:
									HUDGadget :: GetHUD() -> Key_Left();
									break;
								case VK_UP:
									HUDGadget :: GetHUD() -> Key_Up();
									break;
								case VK_RIGHT:
									HUDGadget :: GetHUD() -> Key_Right();
									break;
								case VK_DOWN:
									HUDGadget :: GetHUD() -> Key_Down();
									break;
								case VK_INSERT:
									TextInputState :: ToggleTypingMode();
									break;
								case VK_DELETE:
									HUDGadget :: GetHUD() -> Key_Delete();
									break;
								case VK_TAB:
									HUDGadget :: GetHUD() -> Key_Tab();
									break;							
							}

						}
						else
						{
							#if KeyBindingUses_WM_KEYDOWN
							// 6/4/98:
							// Pass the key to the console-binding code:
							KeyBinding::Process_WM_KEYDOWN
							(
								OurQ[i] . CatData . wParam
							);
							#endif
						}
						#if 0
						textprint
						(
							"\n\n\nWM_KEYDOWN message flushed; code=0x%x\n\n\n",
							OurQ[i] . CatData . wParam
						);
						#endif

					}
					break;
				
					default:
						break;	
				}
			}
		}
		#endif
	}

	// Clear the messages (only place where a semaphore might be needed):
	NumQMessages = 0;
}

void RE_ENTRANT_QUEUE_WinMain_FlushMessagesWithoutProcessing(void)
{
	NumQMessages = 0;
}

/* Internal function definitions ***********************************/
