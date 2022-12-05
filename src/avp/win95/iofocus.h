/*
	
	iofocus.h

	Created 21/11/97 by DHM: is input focus set to normal controls,
	or to typing?
*/

#ifndef _iofocus
#define _iofocus 1

	#ifndef _ourbool
	#include "ourbool.h"
	#endif

	#ifndef _gadget
	#include "gadget.h"
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
	#if UseGadgets
		extern OurBool IOFOCUS_AcceptControls(void);
		extern OurBool IOFOCUS_AcceptTyping(void);

		extern void IOFOCUS_Toggle(void);
	#else
		/* Otherwise: the functions collapse to macros: */
		#define IOFOCUS_AcceptControls() (1)
		#define IOFOCUS_AcceptTyping() (0)

		#define IOFOCUS_Toggle() ((void) 0)
	#endif /* UseGadgets */


/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
