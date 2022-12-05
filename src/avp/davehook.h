/*
	
	davehook.h


	Created 18/11/97 by DHM:
		Contains all the hooks for my code
*/

#ifndef _davehook
#define _davehook 1

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	extern void DAVEHOOK_Init(void);
	extern void DAVEHOOK_UnInit(void);
	extern void DAVEHOOK_Maintain(void);

	extern void DAVEHOOK_ScreenModeChange_Setup(void);
	extern void DAVEHOOK_ScreenModeChange_Cleanup(void);




/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
