/* ********************************************************************	*
 *																		*
 *	DPLAYEXT.C - DirectPlay multi-player code extensions.				*
 *																		*
 *	By: Garry Lancaster									Version: 0.1	*
 *																		*
 * ******************************************************************** */
 
/* N O T E S ---------------------------------------------------------- */

/* Define USE_DB here to use my db.c debugging file. If you wish to use 
 * another debugging system you will have to redefine the ASSERT macro to 
 * use your debugging system or turn off assertions by defining NDEBUG.
 * - Garry.
 */

/* This code marks an experimental departure for me into Hungarian variable
 * names. Enjoy! (or perhaps not.)
 */
 
/* I N C L U D E S ---------------------------------------------------- */

/* OS includes. */
#include <windows.h>
       
/* Include for this file. */
#include "dplayext.h"

/* Custom includes. */
#define UseLocalAssert Yes
#include "ourasert.h"

/* C O N S T A N T S -------------------------------------------------- */

/* The maximum length of a message that can be received by the system, in
 * bytes. DPEXT_MAX_MSG_SIZE bytes will be required to hold the message, so
 * you can't just set this to a huge number.
 */
#define DPEXT_MAX_MSG_SIZE	3072

/* This msg is not guaranteed. */
#define DPEXT_NOT_GUARANTEED	1
		
/* M A C R O S -------------------------------------------------------- */


/* Advance the guaranteed msg count. Wraps around to 1 when the maximum 
 * +ve value for a signed int is passed.
 */
#define DPEXT_NEXT_GRNTD_MSG_COUNT() \
	if( ++gnCurrGrntdMsgId < 1 ) gnCurrGrntdMsgId = 1

/* Generate the msg stamp for a reply to a msg with a specified
 * guaranteed msg id.
 */	
#define DPEXT_TO_REPLY_STAMP( iSentStamp ) ( -( iSentStamp ) )	

/* Identify the original msg stamp given a reply stamp. */
#define DPEXT_TO_ORIGINAL_STAMP( iReplyStamp ) \
	DPEXT_GET_REPLY_STAMP( iReplyStamp )
	
/* T Y P E S ---------------------------------------------------------- */

struct DpExtGrntdMsgInfo
{
	void **abufPending;
	DWORD cBuffers;
	DWORD iLastUsed;
};

/* G L O B A L S . . . ------------------------------------------------ */

/* ...with external scope. */

/* ...with internal (static) scope. */

/* Buffer used to store incoming messages. */
static unsigned char gbufDpExtRecv[ DPEXT_MAX_MSG_SIZE ];

/* Are we expected to add our own error checking? */
static BOOL gbDpExtDoErrChcks = FALSE;

/* Are we expected to implement guaranteed message sending? */
static BOOL gbDpExtDoGrntdMsgs = FALSE;

/* The current count of guaranteed msgs. Zero is reserved for msgs that
 * aren't guaranteed (using DPEXT_NOT_GUARANTEED) and negative numbers are
 * reserved for replies to guaranteed messages.
 */
static int gnCurrGrntdMsgId = 1;

/* Storage for guaranteed msgs and replies to guaranteed msgs. */
static struct DpExtGrntdMsgInfo	ggmiMsgs 	= { NULL, 0, 0 };
static struct DpExtGrntdMsgInfo ggmiReplies = { NULL, 0, 0 };

/* P R O T O S -------------------------------------------------------- */

/* Should all be static. */

/* Perform any required processing on a received msg. No processing is 
 * required if neither DpExt guaranteed msg system or the DpExt error 
 * checking system are on. Returns TRUE if the message was internal only,
 * FALSE if it should be passed to the user.
 */ 
static BOOL DpExtProcessRecvdMsg(BOOL bIsSystemMsg, LPVOID lpData, 
 	DWORD dwDataSize);

/* Generates and returns a checksum for the supplied buffer. The algorithm
 * used ignores the first 4 bytes of the buffer (this is where the checksum
 * should eventually be stored) and is sensitive to the order of the bytes 
 * and long words stored as well as lost data, added data, duplicated data,
 * and standard corruption.
 */ 
static DWORD DpExtChecksumMsg(LPVOID lpData, DWORD dwDataSize);
	
/* Use a destroy msg to try and free up nodes in the guaranteed msg list 
 * that will never now be freed in the usual manner i.e. by a reply to or 
 * a re-send from the player.
 */
static void DpExtUseDestroyMsg(LPDPMSG_DESTROYPLAYERORGROUP pmsgDestroy);
	
/* -------------------------------------------------------------------- *
 *																		*
 * 						F U N C T I O N S 								*
 *																		*
 * -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- *
 *	I N T E R F A C E   F N S - with external scope.					*
 * -------------------------------------------------------------------- */


/* Call this fn to initialise the DpExt module. cGrntdBufs sets the number
 * of guaranteed message buffers to use and cBytesPerBuf is the number of
 * bytes to allocate to each buffer, and thus the maximum length 
 * (including DpExt header) for a guaranteed msg. cGrntdBufs should be 0 
 * if you don't want DpExt's guaranteed msg sending system to be turned on.
 * bErrChcks should be TRUE if you require DpExt's error checking system,
 * otherwise FALSE.
 */
BOOL DpExtInit(DWORD cGrntdBufs, DWORD cBytesPerBuf, 
	BOOL bErrChcks)
{
	return TRUE;
}


/* Un-initialises the DpExt module. */ 
void DpExtUnInit(void)
{
}

 
/* This fn has the same parameters as the standard DirectPlay Send() fn. 
 * However, you *must* leave DPEXT_HEADER_SIZE bytes free at the start of
 * your data buffer. The dwDataSize byte count must includes these bytes. 
 */ 
HRESULT DpExtSend
(
	LPDIRECTPLAY4 lpDP2A,	/* IN: Ptr to IDirectPlay3A (DBCS) interface. */	 
	DPID idFrom, 			/* IN: ID of sending player (you)	*/
	DPID idTo,				/* IN: ID of destination player.	*/ 
	DWORD dwFlags, 			/* IN: DirectPlay Flags.			*/
	LPVOID lpData,			/* IN: Ptr to start of message. 	*/
	DWORD dwDataSize		/* IN: Byte count of message. 		*/
)
{
	HRESULT hrSend;
	
	/* Assert input conditions that the DirectPlay call may not check. */
	LOCALASSERT( lpDP2A );
	LOCALASSERT( lpData );
	LOCALASSERT( dwDataSize >= DPEXT_HEADER_SIZE );
	
	/* Add header information. */
	{
		struct DpExtHeader *pmsghdr	= (struct DpExtHeader *) lpData;
			
		if( gbDpExtDoGrntdMsgs && ( DPSEND_GUARANTEED & dwFlags ) )
		{
			/* Add guaranteed msg stamp. */
			pmsghdr->dwMsgStamp = gnCurrGrntdMsgId;
			DPEXT_NEXT_GRNTD_MSG_COUNT();
		}
		else
		{
			pmsghdr->dwMsgStamp = DPEXT_NOT_GUARANTEED;
		}
		if( gbDpExtDoErrChcks ) 
		{
			pmsghdr->dwChecksum = DpExtChecksumMsg( lpData, dwDataSize );
		}
	}
	
	hrSend = IDirectPlay3_Send( lpDP2A, idFrom, idTo, dwFlags, lpData,
		dwDataSize );
		
	return hrSend;	
}


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
HRESULT DpExtRecv
(
	LPDIRECTPLAY4 lpDP2A,	/* IN: Ptr to IDirectPlay3A (DBCS) interface. */
	LPDPID lpidFrom, 		/* IN/OUT: Ptr to from player id.  	*/
	LPDPID lpidTo, 			/* IN/OUT: Ptr to to player id.		*/
	DWORD dwFlags, 			/* IN: DirectPlay flags.			*/
	LPVOID *lplpData, 		/* IN/OUT: Ptr to ptr to message data.	*/
	LPDWORD lpdwDataSize	/* IN/OUT: Ptr to byte count of message.	*/
)
{
	HRESULT hrRecv;
	BOOL	bInternalOnly;
	BOOL	bIsSysMsg;
	
	/* Assert input conditions that the DirectPlay call may not check. */
	LOCALASSERT( lpDP2A );
	LOCALASSERT( lpidFrom );
	LOCALASSERT( lpidTo );
	LOCALASSERT( lpdwDataSize );
	
	/* Did the user want to use their own data buffer? 
	 * N.B. Does this need to go in the loop?
	 */
	if( !( DPEXT_USER_BUFFER & dwFlags ) )
	{
		/* No. Set parameters to write to internal buffer. */
		*lplpData = gbufDpExtRecv;
		*lpdwDataSize = DPEXT_MAX_MSG_SIZE;
	}
	
	do
	{
		bInternalOnly = FALSE;	/* Default. */
		
		hrRecv = IDirectPlay3_Receive( lpDP2A, lpidFrom, lpidTo, dwFlags, 
			*lplpData, lpdwDataSize );
		
		/* DirectPlay bug work-around. *lpdwDataSize does not get filled in
		 * automatically for some system messages.
		 */
		if( ( DPID_SYSMSG == *lpidFrom ) && 
			( DP_OK == hrRecv ) )
		{
		   	DPMSG_GENERIC msgGenSys	= *( (LPDPMSG_GENERIC) *lplpData );
			
			bIsSysMsg = TRUE;
			switch( msgGenSys.dwType )
			{
				case DPSYS_ADDPLAYERTOGROUP:
				case DPSYS_DELETEPLAYERFROMGROUP:
					/* Didn't test for bug - possibly okay. */
					*lpdwDataSize = sizeof( DPMSG_ADDPLAYERTOGROUP );
					break;
				case DPSYS_CREATEPLAYERORGROUP:	
					/* Not necessary - bug doesn't affect this message. */
					break;
				case DPSYS_DESTROYPLAYERORGROUP:
					*lpdwDataSize = sizeof( DPMSG_DESTROYPLAYERORGROUP );
					break;	
				case DPSYS_SETPLAYERORGROUPDATA:
					/* Didn't test for bug - possibly okay. */
					*lpdwDataSize = sizeof( DPMSG_SETPLAYERORGROUPDATA );
					break;	
				case DPSYS_SETPLAYERORGROUPNAME:
					/* Didn't test for bug - possibly okay. */
					*lpdwDataSize = sizeof( DPMSG_SETPLAYERORGROUPNAME );	
					break;
				case DPSYS_HOST:
				case DPSYS_SESSIONLOST:
					*lpdwDataSize = sizeof( DPMSG_GENERIC );
					break;	
			}
		}
		else bIsSysMsg = FALSE;
		
		/* Might we need to process this message? */
		if( ( DP_OK == hrRecv ) && 
			( gbDpExtDoGrntdMsgs || gbDpExtDoErrChcks ) )
		{
			bInternalOnly = DpExtProcessRecvdMsg
			( 
				bIsSysMsg,
				*lplpData,
				*lpdwDataSize
			);	
		}		
	}
	while( bInternalOnly );
		
	return hrRecv;		
}

	 	 
/* -------------------------------------------------------------------- *
 *	S T A T I C   F N S - with internal scope.							*
 * -------------------------------------------------------------------- */


/* Perform any required processing on a received msg. No processing is 
 * required if neither DpExt guaranteed msg system or the DpExt error 
 * checking system are on. Returns TRUE if the message was internal only,
 * FALSE if it should be passed to the user.
 */ 
static BOOL DpExtProcessRecvdMsg(BOOL bIsSystemMsg, LPVOID lpData, 
 	DWORD dwDataSize)
{
	/* Is this a system message? */
	if( bIsSystemMsg )
	{
		/* Yes. Do we need to intercept any system messages for our own
		 * nefarious purposes?
		 */
		if( gbDpExtDoGrntdMsgs )
		{
			/* Yes, we need to intercept messages about deleted players or
			 * groups. Is this one?
			 */
			DPMSG_GENERIC *pmsgGeneric = (DPMSG_GENERIC *) lpData; 
			
			if( DPSYS_DESTROYPLAYERORGROUP == pmsgGeneric->dwType )
			{
				DpExtUseDestroyMsg( (LPDPMSG_DESTROYPLAYERORGROUP) 
					pmsgGeneric );
			} 
		} 
	}
	else
	{
		/* No, this isn't a system message. Use DpExt header... */
		struct DpExtHeader *pmsghdr = (struct DpExtHeader *) lpData;
		
		LOCALASSERT( dwDataSize >= DPEXT_HEADER_SIZE );
			
		/* ...Should we check this message for errors? */
		if( gbDpExtDoErrChcks )
		{
			
		}
		
		/* ...Was this message sent by the DpExt guaranteed message system? */
	}

	return FALSE;
}	


/* Generates and returns a checksum for the supplied buffer. The algorithm
 * used ignores the first 4 bytes of the buffer (this is where the checksum
 * should eventually be stored) and is sensitive to the order of the bytes 
 * and long words stored as well as lost data, added data, duplicated data,
 * and standard corruption.
 */ 
static DWORD DpExtChecksumMsg(LPVOID lpData, DWORD dwDataSize)
{
	return 0;	/* No implementation yet. */
}


/* Use a destroy msg to try and free up nodes in the guaranteed msg list 
 * that will never now be freed in the usual manner i.e. by a reply to or 
 * a re-send from the player.
 */
static void DpExtUseDestroyMsg(LPDPMSG_DESTROYPLAYERORGROUP pmsgDestroy)
{
	/* No implementation yet. */
}