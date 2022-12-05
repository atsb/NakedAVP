/* ********************************************************************	*
 *																		*
 *	DPLAYEXT.H - Header for	DirectPlay extensions.						*
 *																		*
 *	By: Garry Lancaster									Version: 0.1	*
 *																		*
 * ******************************************************************** */

/* N O T E S ---------------------------------------------------------- */

/* S T A R T   W R A P P E R ------------------------------------------ */

/* Avoid multiple inclusions of this file in a single source file. */
#ifndef DPLAYEXT_H_INCLUDED
#define DPLAYEXT_H_INCLUDED

/* Permit use in a C++ source file. */
#ifdef __cplusplus
extern "C" {
#endif

/* I N C L U D E S ---------------------------------------------------- */

/* DirectPlay include. */
#include "dplay.h"

/* C O N S T A N T S -------------------------------------------------- */

/* All messages sent and all non-system messages received will have the
 * first DPEXT_HEADER_SIZE bytes used for header information. Therefore,
 * you must make all buffers at least this big and not use these bytes 
 * yourself.
 */
#define DPEXT_HEADER_SIZE	( sizeof( struct DpExtHeader) )

/* This flag alters the DpExtRecv() function behaviour to allow the user
 * to provide the buffer that the message will be copied to. This is more
 * like the original DirectPlay Receive() fn behaviour than the default.
 */
#define DPEXT_USER_BUFFER	0x10000000

/* M A C R O S -------------------------------------------------------- */

/* T Y P E S ---------------------------------------------------------- */

/* This structure is supplied so that you can treat it as a base class
 * for your messages in C++ if you wish.
 */
struct DpExtHeader
{
	DWORD dwChecksum;	/* Error checking information. 	 */
	DWORD dwMsgStamp;	/* Contains guaranteed msg info. */
};

/* G L O B A L S ------------------------------------------------------	*/

/* All declarations. NO definitions. */

/* P R O T O S -------------------------------------------------------- */

/* Call this fn to initialise the DpExt module. cGrntdBufs sets the number
 * of guaranteed message buffers to use and cBytesPerBuf is the number of
 * bytes to allocate to each buffer, and thus the maximum length 
 * (including DpExt header) for a guaranteed msg. cGrntdBufs should be 0 
 * if you don't want DpExt's guaranteed msg sending system to be turned on.
 * bErrChcks should be TRUE if you require DpExt's error checking system,
 * otherwise FALSE.
 */
extern BOOL DpExtInit(DWORD cGrntdBufs, DWORD cBytesPerBuf, BOOL bErrChcks);

/* Un-initialises the DpExt module. */ 
extern void DpExtUnInit(void);

/* This fn has the same parameters as the standard DirectPlay Send() fn. 
 * However, you *must* leave DPEXT_HEADER_SIZE bytes free at the start of
 * your data buffer. The dwDataSize byte count must includes these bytes. 
 */ 
extern HRESULT DpExtSend
(
	LPDIRECTPLAY4 lpDP2A,	/* IN: Ptr to IDirectPlay2A (DBCS) interface. */	 
	DPID idFrom, 			/* IN: ID of sending player (you)	*/
	DPID idTo,				/* IN: ID of destination player.	*/ 
	DWORD dwFlags, 			/* IN: DirectPlay Flags.			*/
	LPVOID lpData,			/* IN: Ptr to start of message. 	*/
	DWORD dwDataSize		/* IN: Byte count of message. 		*/
);

/* This fn has similar parameters to the standard DirectPlay Receive()
 * fn. Be aware that the 2 id parameters, the lplpData parameter and the
 * lpdwDataSize parameters are only valid as inputs if the
 * appropriate flags are used in the dwFlags parameter. They are always 
 * valid as outputs. 
 *
 * The main difference from the standard Receive() is that the LPVOID 
 * lpData field has changed to LPVOID *lplpData. Now instead of supplying 
 * the buffer for the message and having the fn fail if the	buffer isn't 
 * big enough, the buffer is allocated for you and you get a pointer to it. 
 * I re-use the same buffer each time DpExtRecv() is called, so the pointer 
 * is only valid until the next time you call this fn. 
 *
 * If you want to maintain the message longer than that you can (a) copy the
 * message data to your own buffer or (b) more efficiently, pass a ptr to
 * a ptr to your own allocated data buffer in lplpData, a ptr to its size 
 * in lpdwDataSize and set the flag DPEXT_USER_BUFFER in dwFlags - this is 
 * more like the original DirectPlay Receive() fn behaviour. In the case of
 * (b), you must leave DPEXT_HEADER_SIZE bytes free at the start of your 
 * user buffer and include these bytes in *lpdwDataSize.
 *
 * All non-system messages received will use their first DPEXT_HEADER_SIZE
 * bytes for header information. Your message proper begins after this 
 * header.
 */
extern HRESULT DpExtRecv
(
	LPDIRECTPLAY4 lpDP2A,	/* IN: Ptr to IDirectPlay2A (DBCS) interface. */
	LPDPID lpidFrom, 		/* IN/OUT: Ptr to from player id.  	*/
	LPDPID lpidTo, 			/* IN/OUT: Ptr to to player id.		*/
	DWORD dwFlags, 			/* IN: DirectPlay flags.			*/
	LPVOID *lplpData, 		/* IN/OUT: Ptr to ptr to message data.	*/
	LPDWORD lpdwDataSize	/* IN/OUT: Ptr to byte count of message.	*/
);

/* E N D   W R A P P E R ---------------------------------------------- */

/* Permit use in a C++ source file. */
#ifdef __cplusplus
}
#endif

/* Avoid multiple inclusions of this file in a single source file. */
#endif
