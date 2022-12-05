#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fixer.h"

#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "equipmnt.h"

#include "pldnet.h"
#include "net.h"


DPID AVPDPNetID;
int QuickStartMultiplayer=1;
DPNAME AVPDPplayerName;
int glpDP; /* directplay object */

BOOL DpExtInit(DWORD cGrntdBufs, DWORD cBytesPerBuf, BOOL bErrChcks)
{
	fprintf(stderr, "DpExtInit(%d, %d, %d)\n", cGrntdBufs, cBytesPerBuf, bErrChcks);
	
	return TRUE;
}

void DpExtUnInit()
{
	fprintf(stderr, "DpExtUnInit()\n");
}

HRESULT DpExtRecv(int lpDP2A, void *lpidFrom, void *lpidTo, DWORD dwFlags, void *lplpData, LPDWORD lpdwDataSize)
{
/*
	fprintf(stderr, "DpExtRecv(%d, %p, %p, %d, %p, %p)\n", lpDP2A, lpidFrom, lpidTo, dwFlags, lplpData, lpdwDataSize);
*/
	return 1;
}

HRESULT DpExtSend(int lpDP2A, DPID idFrom, DPID idTo, DWORD dwFlags, void *lpData, DWORD dwDataSize)
{
/*
	fprintf(stderr, "DpExtSend(%d, %d, %d, %d, %p, %d)\n", lpDP2A, idFrom, idTo, dwFlags, lpData, dwDataSize);
*/
#if 0
	FILE *fp = fopen("net.log", "ab");
	fprintf(fp, "\nDpExtSend(%d, %d, %d, %d, %p, %d) ", lpDP2A, idFrom, idTo, dwFlags, lpData, dwDataSize);
	fprintf(fp, "time = %d\n", timeGetTime());
	fwrite(lpData, dwDataSize, 1, fp);
	fclose(fp);
#endif

	return 1;
}

/* directplay.c */
int DirectPlay_ConnectingToLobbiedGame(char* playerName)
{
	fprintf(stderr, "DirectPlay_ConnectingToLobbiedGame(%s)\n", playerName);
	
	return 0;
}

int DirectPlay_ConnectingToSession()
{
	fprintf(stderr, "DirectPlay_ConnectingToSession()\n");
	
	return 0;
}

BOOL DirectPlay_UpdateSessionList(int *SelectedItem)
{
	fprintf(stderr, "DirectPlay_UpdateSessionList(%p)\n", SelectedItem);
	
	return 0;
}

int DirectPlay_JoinGame()
{
	fprintf(stderr, "DirectPlay_JoinGame()\n");
	
	return 0;
}

void DirectPlay_EnumConnections()
{
	fprintf(stderr, "DirectPlay_EnumConnections()\n");
	
	netGameData.tcpip_available = 1;
	netGameData.ipx_available = 0;
	netGameData.modem_available = 0;
	netGameData.serial_available = 0;                        
}

int DirectPlay_HostGame(char *playerName, char *sessionName,int species,int gamestyle,int level)
{
	extern int DetermineAvailableCharacterTypes(int);
	
	int maxPlayers=DetermineAvailableCharacterTypes(FALSE);
	if(maxPlayers<1) maxPlayers=1;
	if(maxPlayers>8) maxPlayers=8;
	
	if(!netGameData.skirmishMode) {
		fprintf(stderr, "DirectPlay_HostGame(%s, %s, %d, %d, %d)\n", playerName, sessionName, species, gamestyle, level);
		
		//fake multiplayer
		//need to set the id to an non zero value
		AVPDPNetID=100;
		
		memset(&AVPDPplayerName, 0, sizeof(AVPDPplayerName));
		AVPDPplayerName.dwSize = sizeof(DPNAME);
		AVPDPplayerName.lpszShortNameA  = playerName;
		AVPDPplayerName.lpszLongNameA = playerName;
		
		glpDP = 1;
	} else {
		//fake multiplayer
		//need to set the id to an non zero value
		AVPDPNetID=100;
		
		memset(&AVPDPplayerName, 0, sizeof(AVPDPplayerName));
		AVPDPplayerName.dwSize = sizeof(DPNAME);
		AVPDPplayerName.lpszShortNameA  = playerName;
		AVPDPplayerName.lpszLongNameA = playerName;
	}
	
	InitAVPNetGameForHost(species,gamestyle,level);
	
	return 1;
}

int DirectPlay_ConnectToSession(int sessionNumber, char *playerName)
{
	fprintf(stderr, "DirectPlay_ConnectToSession(%d, %s)\n", sessionNumber, playerName);
	
	return 0;
}

int DirectPlay_Disconnect()
{
	fprintf(stderr, "DirectPlay_Disconnect()\n");
	
	return 1;
}

HRESULT IDirectPlayX_GetPlayerName(int glpDP, DPID id, void *data, void *size)
{
	fprintf(stderr, "IDirectPlayX_GetPlayerName(%d, %d, %p, %p)\n", glpDP, id, data, size);

	return 1;
}

/* End of Linux-related junk */
