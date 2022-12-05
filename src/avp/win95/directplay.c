/* KJL 15:54:57 03/07/98 - DirectPlay.c */
#include "3dc.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "system.h"
#include "equates.h"
#include "platform.h"
#include "shape.h"
#include "prototyp.h"
#include "inline.h"
#include "dp_sprh.h"
#include "dplayext.h"
#include "equipmnt.h"
#include "pldnet.h"			   
#include "dp_func.h"
#include "dxlog.h"

#include "AvP_Menus.h"
#include "AvP_MP_Config.h"

#define UseLocalAssert Yes
#include "ourasert.h"

GUID SPGuid;
LPGUID lpSPGuid = (LPGUID) &SPGuid;
BOOL GotTCPIP;

/*
Version 0 - Original multiplayer + save patch
Version 100 - Added pistol,skeeter (and new levels)
*/
#define AVP_MULTIPLAYER_VERSION 100

extern void MinimalNetCollectMessages(void);
extern void InitAVPNetGameForHost(int species, int gamestyle, int level);
extern void InitAVPNetGameForJoin(void);
extern int DetermineAvailableCharacterTypes(BOOL ConsiderUsedCharacters);
										 
extern BOOL GetGDISurface();
extern BOOL LeaveGDISurface();
void FindAvPSessions(void);

LPDIRECTPLAYLOBBY3 lpDPlayLobby;

BOOL DirectPlay_GetSessionDesc(LPDPSESSIONDESC2 lpSessionDesc)
{
	HRESULT hr;
	DWORD dwSize=0;
	LPDPSESSIONDESC2 sessionDescBuffer;

	if(!glpDP || !lpSessionDesc) return 0;

	//get size of session desc
	IDirectPlayX_GetSessionDesc(glpDP,NULL,&dwSize);
	sessionDescBuffer =(LPDPSESSIONDESC2) AllocateMem(dwSize);

	//now get the session description
	hr=IDirectPlayX_GetSessionDesc(glpDP,sessionDescBuffer,&dwSize);
	if(hr==DP_OK)
	{
		//copy the contents of the description
		memcpy(lpSessionDesc,sessionDescBuffer,sizeof(DPSESSIONDESC2));
		DeallocateMem(sessionDescBuffer);
		return TRUE;
	}
	
	DeallocateMem(sessionDescBuffer);
	return 0;
}

BOOL DirectPlay_UpdateSessionDescForLobbiedGame(int gamestyle,int level)
{
	DPSESSIONDESC2 sessionDesc;
	HRESULT hr;
	if(!DirectPlay_GetSessionDesc(&sessionDesc)) return 0;

	{
		char* customLevelName = GetCustomMultiplayerLevelName(level,gamestyle);
		if(customLevelName[0])
		{
			//store the gamestyle and a too big level number in dwUser2
			sessionDesc.dwUser2 = (gamestyle<<8)|100;
		}
		else
		{
			//store the gamestyle and level number in dwUser2
			sessionDesc.dwUser2 = (gamestyle<<8)|level;
		}
		
		//store the custom level name in the session name as is.
		//since we never see the session name for lobbied games anyway
		sessionDesc.lpszSessionNameA = customLevelName;
		
		//make sure that dwUser2 is nonzero , so that it can be checked for by the 
		//clients
		sessionDesc.dwUser2|=0x80000000;
		
		sessionDesc.dwUser1 = AVP_MULTIPLAYER_VERSION;
		
		hr = IDirectPlayX_SetSessionDesc(glpDP,&sessionDesc,0);
		if(hr!=DP_OK)
		{
			return FALSE;
		}

	}

   	
	return TRUE;
}

int DirectPlay_HostGame(char *playerName, char *sessionName,int species,int gamestyle,int level)
{
	int maxPlayers=DetermineAvailableCharacterTypes(FALSE);
	if(maxPlayers<1) maxPlayers=1;
	if(maxPlayers>8) maxPlayers=8;
	
	if(!netGameData.skirmishMode)
	{
		//for lobbied games most of the directplay setup is done when AvP is first started
		if(!LobbiedGame)
		{
			/* This function is intended to handle everything that is required to start a new multiplayer game. */
			CoInitialize(NULL);

			/* Init DP object */
			DPlayCreate(NULL);

			CoCreateInstance(&CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, &IID_IDirectPlayLobby3, (LPVOID*)&lpDPlayLobby);

			
			/* Get TCPIP connection */
			//if (!GetTCPIPConnection()) return 0;
			if (!InitialiseConnection()) return 0;
	
			/* create session */
			{
				char* customLevelName = GetCustomMultiplayerLevelName(level,gamestyle);
				if(customLevelName[0])
				{
					//add the level name to the beginning of the session name
					char name_buffer[100];
					sprintf(name_buffer,"%s:%s",customLevelName,sessionName);
					if ((DPlayCreateSession(name_buffer,maxPlayers,AVP_MULTIPLAYER_VERSION,(gamestyle<<8)|100)) != DP_OK) return 0;
				}
				else
				{
 //					static TCHAR sessionName[] = "AvP test session";
					if ((DPlayCreateSession(sessionName,maxPlayers,AVP_MULTIPLAYER_VERSION,(gamestyle<<8)|level)) != DP_OK) return 0;
				}
			}

			/* Try to create a DP player */
		}
		else
		{
			//for lobbied games we need to fill in the level number into the existing session description
			if(!DirectPlay_UpdateSessionDescForLobbiedGame(gamestyle,level)) return 0;
		}

		if(!DirectPlay_CreatePlayer(playerName,playerName)) return 0;

	}
	else
	{
		//fake multiplayer
		//need to set the id to an non zero value
		AVPDPNetID=100;

		ZeroMemory(&AVPDPplayerName,sizeof(DPNAME));
		AVPDPplayerName.dwSize = sizeof(DPNAME);
		AVPDPplayerName.lpszShortNameA	= playerName;
		AVPDPplayerName.lpszLongNameA = playerName;
		
	}
	InitAVPNetGameForHost(species,gamestyle,level);
	return 1;
}

int DirectPlay_JoinGame(void)
{
	/* This function is intended to handle everything that is required to start a new multiplayer game. */
	CoInitialize(NULL);

	/* Init DP object */
	DPlayCreate(NULL);

	CoCreateInstance(&CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, &IID_IDirectPlayLobby3, (LPVOID*)&lpDPlayLobby);

	/* Get TCPIP connection */
//	if (!GetTCPIPConnection()) return 0;
	if (!InitialiseConnection()) return 0;
	

	/* enum sessions */
	FindAvPSessions();
	return NumberOfSessionsFound;
}

int DirectPlay_ConnectToSession(int sessionNumber, char *playerName)
{
	extern unsigned char DebouncedKeyboardInput[];
	
	if (FAILED(DPlayOpenSession((LPGUID)&SessionData[sessionNumber].Guid))) return 0;
	
	if(!DirectPlay_CreatePlayer(playerName,playerName)) return 0;
	
	InitAVPNetGameForJoin();

	netGameData.levelNumber = SessionData[sessionNumber].levelIndex;

	netGameData.joiningGameStatus = JOINNETGAME_WAITFORDESC;
	
	return 1;
}


int DirectPlay_ConnectingToSession()
{
	extern unsigned char DebouncedKeyboardInput[];
	//see if the player has got bored of waiting
	if(DebouncedKeyboardInput[KEY_ESCAPE])
	{
		//abort attempt to join game
		if(AVPDPNetID)
		{
			IDirectPlayX_DestroyPlayer(glpDP, AVPDPNetID);
			AVPDPNetID = NULL;
		}
		DPlayClose();
		AvP.Network = I_No_Network;	
		return 0;
	}

	MinimalNetCollectMessages();
	if(!netGameData.needGameDescription)
	{
	  	//we now have the game description , so we can go to the configuration menu
	  	return AVPMENU_MULTIPLAYER_CONFIG_JOIN;
	}
	return 1;
}

int DirectPlay_InitLobbiedGame()
{
	HRESULT hr;
	DWORD dwSize;
	LPDPLCONNECTION lpConnectionSettings;
	extern char MP_PlayerName[];
	
	
	/* This function is intended to handle everything that is required to start a new multiplayer game. */
	CoInitialize(NULL);

	/* Init DP object */
	DPlayCreate(NULL);

	CoCreateInstance(&CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, &IID_IDirectPlayLobby3A, (LPVOID*)&lpDPlayLobby);
	
	//get the size of the connection settings
	hr = IDirectPlayLobby_GetConnectionSettings(lpDPlayLobby,0, NULL, &dwSize);

	//get the connection settings
	lpConnectionSettings = (LPDPLCONNECTION) AllocateMem(dwSize);
    
 	hr= IDirectPlayLobby_GetConnectionSettings(lpDPlayLobby,0,lpConnectionSettings, &dwSize);    
	
	if(hr!=DP_OK) return 0;


	//Make sure the host migrration , and keep alive flags are set for this session
	if(LobbiedGame==LobbiedGame_Server)
	{
		lpConnectionSettings->lpSessionDesc->dwFlags|=(DPSESSION_KEEPALIVE|DPSESSION_MIGRATEHOST);
		lpConnectionSettings->lpSessionDesc->dwUser1 = AVP_MULTIPLAYER_VERSION;
		hr=IDirectPlayLobby_SetConnectionSettings(lpDPlayLobby,0,0,lpConnectionSettings);
		if(hr!=DP_OK)
		{
			LOGDXFMT(("Set connection settings : %x",hr));	
		}
	}
	
	//copy the player's name from the connection settings
	strncpy(MP_PlayerName,lpConnectionSettings->lpPlayerName->lpszShortNameA,NET_PLAYERNAMELENGTH-1);
	MP_PlayerName[NET_PLAYERNAMELENGTH-1]='\0';

	
	//connect to the lobbied game
	hr = IDirectPlayLobby_ConnectEx(lpDPlayLobby,0,&IID_IDirectPlay4A, &glpDP, NULL);

	if(hr!=DP_OK)
	{
		LOGDXFMT(("Connect Ex %x\n",hr));	
		return 0;
	}

	DeallocateMem(lpConnectionSettings);
	return 1;
}


#if 0
int DirectPlay_ConnectToLobbiedGame(char *playerName)
{
	extern unsigned char DebouncedKeyboardInput[];
	
	InitAVPNetGameForJoin();

	/*
	Wait until there is at least one player in the game (this will be the host).
	This way we can avoid joining until the host is ready
	*/
	while(!DirectPlay_CountPlayersInCurrentSession())
	{
		//see if the player has got bored of waiting
		CheckForWindowsMessages();
		ReadUserInput();
		if(DebouncedKeyboardInput[KEY_ESCAPE])
		{
			//abort attempt to join game
			AvP.Network = I_No_Network;	
			return 0;
		}
	}
	
	
	//create our player
	if(!DirectPlay_CreatePlayer(playerName,playerName))
	{
		LOGDXFMT(("Failed to create player"));	
		
		return 0;
	}
		
	//wait for the game description from the host
	while(netGameData.needGameDescription)
	{
		MinimalNetCollectMessages();
		
		//see if the player has got bored of waiting
		CheckForWindowsMessages();
		ReadUserInput();
		if(DebouncedKeyboardInput[KEY_ESCAPE])
		{
			//abort attempt to join game
			IDirectPlayX_DestroyPlayer(glpDP, AVPDPNetID);
			AVPDPNetID = NULL;
			AvP.Network = I_No_Network;	
			return 0;
		}
	}
	return 1;
}
#else
int DirectPlay_ConnectingToLobbiedGame(char* playerName)
{
	extern unsigned char DebouncedKeyboardInput[];
	DPSESSIONDESC2 sessionDesc;

	//see if the player has got bored of waiting
	if(DebouncedKeyboardInput[KEY_ESCAPE])
	{
		//abort attempt to join game
		if(AVPDPNetID)
		{
			IDirectPlayX_DestroyPlayer(glpDP, AVPDPNetID);
			AVPDPNetID = NULL;
		}
		AvP.Network = I_No_Network;	
		return 0;
	}
	
	//get the session description
	if(!DirectPlay_GetSessionDesc(&sessionDesc))
	{
		return 1;
	}
	
	if(netGameData.joiningGameStatus == JOINNETGAME_WAITFORSTART)
	{
		/*
		Wait until there is at least one player in the game (this will be the host).
		This way we can avoid joining until the host is ready
		*/

		if(sessionDesc.dwCurrentPlayers)
		{
			int gamestyle;
			int level;
			int local_level_index;

			//make sure the version number is correct
			if(sessionDesc.dwUser1 != AVP_MULTIPLAYER_VERSION)
			{
				//argh
				netGameData.joiningGameStatus = JOINNETGAME_WRONGAVPVERSION;
				return 1;
			}

			//need to wait until the level number has been filled into dwUser2
			if(!sessionDesc.dwUser2) return 1;

			gamestyle = (sessionDesc.dwUser2 >> 8) & 0xff;
			level = sessionDesc.dwUser2  & 0xff;
	
			//see if we have the level that the host has chosen
			//(note that the level name is stored in session name for custom levels)
			local_level_index = GetLocalMultiplayerLevelIndex(level,sessionDesc.lpszSessionNameA,gamestyle);
			if(local_level_index<0)
			{
				///no good
				netGameData.joiningGameStatus = JOINNETGAME_DONTHAVELEVEL;
				return 1;
			}
			netGameData.levelNumber = local_level_index;

						
			netGameData.joiningGameStatus = JOINNETGAME_WAITFORDESC;

			//we can now create our player
			if(!DirectPlay_CreatePlayer(playerName,playerName))
			{
				LOGDXFMT(("Failed to create player"));	
				return 0;
			}
		}
		
	}
	if(netGameData.joiningGameStatus == JOINNETGAME_WAITFORDESC)
	{
		MinimalNetCollectMessages();
		if(!netGameData.needGameDescription)
		{
			//we now have the game description , so we can go to the configuration menu
			return AVPMENU_MULTIPLAYER_CONFIG_JOIN;

		}
	}
	return 1;
}

#endif

int DirectPlay_Disconnect(void)
{
	DPlayClose();
	if (glpDP) IDirectPlayX_Release(glpDP);
	glpDP=NULL;
	AvP.Network = I_No_Network;
	return 1;
}



BOOL FAR PASCAL EnumSessionsCallback(
						LPCDPSESSIONDESC2       lpSessionDesc,
						LPDWORD                         lpdwTimeOut,
						DWORD                           dwFlags,
						LPVOID                          lpContext)
{
	char sessionName[100]="";
	char levelName[100]="";
	int gamestyle;
	int level;

	
	// see if last session has been enumerated
    if (dwFlags & DPESC_TIMEDOUT || NumberOfSessionsFound>=MAX_NO_OF_SESSIONS)
		return (FALSE); 
		
	gamestyle = (lpSessionDesc->dwUser2 >> 8) & 0xff;
	level = lpSessionDesc->dwUser2  & 0xff;
		                                        

	//split the session name up into its parts
	if(level>=100)
	{
		char* colon_pos;
		//custom level name may be at the start
		strcpy(levelName,lpSessionDesc->lpszSessionNameA);

		colon_pos = strchr(levelName,':');
		if(colon_pos)
		{
			*colon_pos = 0;
			strcpy(sessionName,colon_pos+1);
		}
		else
		{
			strcpy(sessionName,lpSessionDesc->lpszSessionNameA);
			levelName[0] = 0;
			
		}
		

	}
	else
	{
		strcpy(sessionName,lpSessionDesc->lpszSessionNameA);
	}

	sprintf(SessionData[NumberOfSessionsFound].Name,"%s (%d/%d)",sessionName,lpSessionDesc->dwCurrentPlayers,lpSessionDesc->dwMaxPlayers);

	SessionData[NumberOfSessionsFound].Guid	= lpSessionDesc->guidInstance;

	if(lpSessionDesc->dwCurrentPlayers < lpSessionDesc->dwMaxPlayers)
		SessionData[NumberOfSessionsFound].AllowedToJoin =TRUE;
	else
		SessionData[NumberOfSessionsFound].AllowedToJoin =FALSE;

	//multiplayer version number (possibly)
	if(lpSessionDesc->dwUser1 != AVP_MULTIPLAYER_VERSION)
	{
		float version = 1.0 + lpSessionDesc->dwUser1/100.0;
		SessionData[NumberOfSessionsFound].AllowedToJoin =FALSE;
 		sprintf(SessionData[NumberOfSessionsFound].Name,"%s (V %.2f)",sessionName,version);
	
	}
	else
	{
		//get the level number in our list of levels (assuming we have the level)
		int local_index = GetLocalMultiplayerLevelIndex(level,levelName,gamestyle);

		if(local_index<0)
		{
			//we don't have the level , so ignore this session
			return TRUE;
		}
			
		SessionData[NumberOfSessionsFound].levelIndex = local_index;
		

	}

	NumberOfSessionsFound++;

    return (TRUE);
}

void FindAvPSessions(void)
{
	DPSESSIONDESC2 sessionDesc;

	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.guidApplication = *glpGuid;


	NumberOfSessionsFound=0;

	IDirectPlayX_EnumSessions(glpDP, &sessionDesc, 0,
							EnumSessionsCallback, 0,
//							DPENUMSESSIONS_AVAILABLE|
							DPENUMSESSIONS_ALL|
							DPENUMSESSIONS_ASYNC);
//							DPENUMSESSIONS_RETURNSTATUS);
}




// ---------------------------------------------------------------------------
BOOL FAR PASCAL EnumSPCallback(
						LPCGUID			lpguidSP,
						LPVOID			lpConnection,
						DWORD			dwSize,
						LPCDPNAME		lpName,
						DWORD			dwFlags,
						LPVOID			lpContext)
{
	HRESULT hr = DPlayCreate(lpConnection);
	//make sure we can actually use this service provider
	if(hr==DP_OK)
	{
		if (IsEqualGUID(lpguidSP, &DPSPGUID_TCPIP))
		{
			netGameData.tcpip_available=1;	
		}
		else if(IsEqualGUID(lpguidSP, &DPSPGUID_IPX))
		{
			netGameData.ipx_available=1;
		}
		else if(IsEqualGUID(lpguidSP, &DPSPGUID_SERIAL))
		{
			netGameData.serial_available=1;
		}
	
		else if(IsEqualGUID(lpguidSP, &DPSPGUID_MODEM))
		{
			netGameData.modem_available=1;
		}
	}
	return(TRUE);
}


static BOOL ConnectionOk;

BOOL FAR PASCAL EnumSPAndConnectCallback(
						LPCGUID			lpguidSP,
						LPVOID			lpConnection,
						DWORD			dwSize,
						LPCDPNAME		lpName,
						DWORD			dwFlags,
						LPVOID			lpContext)
{
	if (IsEqualGUID(lpguidSP, lpContext))
	{
		HRESULT hr;
		//check for windows messages in order to clear any key presses from the windows messages.
		CheckForWindowsMessages();
		//switch to gdi surface in case any dialog boxes are brought up
		GetGDISurface();
		hr=DPlayCreate(lpConnection);
		if(hr==DP_OK)
		{
			ConnectionOk=TRUE;
		}
		LeaveGDISurface();
		return FALSE;
	}
	return(TRUE);
}

void DirectPlay_EnumConnections()
{
	netGameData.tcpip_available=0;
	netGameData.ipx_available=0;
	netGameData.modem_available=0;
	netGameData.serial_available=0;

	/* This function is intended to handle everything that is required to start a new multiplayer game. */
	CoInitialize(NULL);

	/* Init DP object */
	DPlayCreate(NULL);
	
	IDirectPlayX_EnumConnections( glpDP, glpGuid, EnumSPCallback, 0, 0);
}



BOOL GetTCPIPConnection(void)
{
	extern char IPAddressString[]; 
	HRESULT hr;
	DPCOMPOUNDADDRESSELEMENT addressElements[3];
	LPVOID lpAddress=NULL;
	DWORD dwElementCount;
	DWORD dwAddressSize;

	GotTCPIP = FALSE;
	/* Enumerate Service Providers */
	IDirectPlayX_EnumConnections( glpDP, glpGuid, EnumSPCallback, 0, 0);

	if (!GotTCPIP) return FALSE;

	DPlayRelease();

	addressElements[0].guidDataType = DPAID_ServiceProvider;
	addressElements[0].dwDataSize = sizeof(GUID);
	addressElements[0].lpData = (LPVOID) &DPSPGUID_TCPIP;

	// IP address string
	addressElements[1].guidDataType = DPAID_INet;
	addressElements[1].dwDataSize = lstrlen(IPAddressString) + 1;
	addressElements[1].lpData = IPAddressString;
	dwElementCount = 2;

	// see how much room is needed to store this address
	hr = lpDPlayLobby->lpVtbl->CreateCompoundAddress(lpDPlayLobby,addressElements, dwElementCount, NULL, &dwAddressSize);

	if (hr != DPERR_BUFFERTOOSMALL)	goto FAILURE;

	// allocate space
	lpAddress = AllocMem(dwAddressSize);
	if (lpAddress == NULL)
	{
		hr = DPERR_NOMEMORY;
		goto FAILURE;
	}

	// create the address
	hr = lpDPlayLobby->lpVtbl->CreateCompoundAddress(lpDPlayLobby,addressElements, dwElementCount, lpAddress, &dwAddressSize);
	if FAILED(hr) goto FAILURE;

	DPlayCreate(lpAddress);
	return TRUE;

	FAILURE:

	if (lpAddress) DeallocateMem(lpAddress);
	return FALSE;
}

BOOL InitialiseConnection()
{
	HRESULT hr;
	ConnectionOk=FALSE;

	switch(netGameData.connectionType)
	{
		case CONN_TCPIP :
		{
			extern char IPAddressString[]; 
			DPCOMPOUNDADDRESSELEMENT addressElements[3];
			LPVOID lpAddress=NULL;
			DWORD dwElementCount;
			DWORD dwAddressSize;
			
			DPlayRelease();

			addressElements[0].guidDataType = DPAID_ServiceProvider;
			addressElements[0].dwDataSize = sizeof(GUID);
			addressElements[0].lpData = (LPVOID) &DPSPGUID_TCPIP;

			// IP address string
			addressElements[1].guidDataType = DPAID_INet;
			addressElements[1].dwDataSize = lstrlen(IPAddressString) + 1;
			addressElements[1].lpData = IPAddressString;
			dwElementCount = 2;

			// see how much room is needed to store this address
			hr = lpDPlayLobby->lpVtbl->CreateCompoundAddress(lpDPlayLobby,addressElements, dwElementCount, NULL, &dwAddressSize);

			if (hr != DPERR_BUFFERTOOSMALL)	goto FAILURE;

			// allocate space
			lpAddress = AllocMem(dwAddressSize);
			if (lpAddress == NULL)
			{
				hr = DPERR_NOMEMORY;
				goto FAILURE;
			}

			// create the address
			hr = lpDPlayLobby->lpVtbl->CreateCompoundAddress(lpDPlayLobby,addressElements, dwElementCount, lpAddress, &dwAddressSize);
			if FAILED(hr) goto FAILURE;

			hr=DPlayCreate(lpAddress);
			
			if (lpAddress) DeallocateMem(lpAddress);

			return hr==DP_OK;

			FAILURE:

			if (lpAddress) DeallocateMem(lpAddress);
			return FALSE;
			break;
		}
		case CONN_IPX :
			IDirectPlayX_EnumConnections( glpDP, glpGuid, EnumSPAndConnectCallback,(void*)&DPSPGUID_IPX,0);
			break;
		
		case CONN_Modem :
			IDirectPlayX_EnumConnections( glpDP, glpGuid, EnumSPAndConnectCallback,(void*)&DPSPGUID_MODEM,0);
			break;

		case CONN_Serial :
			IDirectPlayX_EnumConnections( glpDP, glpGuid, EnumSPAndConnectCallback, (void*)&DPSPGUID_SERIAL,0);
			break;
	}

	return ConnectionOk;
}

static BOOL DirectPlay_CreatePlayer(char* FormalName,char* FriendlyName)
{
	HRESULT hr;

	// Initialise static DP name structure to refer to the names:
	ZeroMemory(&AVPDPplayerName,sizeof(DPNAME));
	AVPDPplayerName.dwSize = sizeof(DPNAME);
	AVPDPplayerName.lpszShortNameA	= FriendlyName;
	AVPDPplayerName.lpszLongNameA = FormalName;

	hr = IDirectPlayX_CreatePlayer
	(
		glpDP,			   /* our dp object*/
		&AVPDPNetID,	   /* our ID */	
		&AVPDPplayerName,  /* our name */	
		NULL, 			   /* event */	
		NULL,			   /* player data */
		0,				   /* size of player data */
		0				   /* flags */
	);

	if (hr == DP_OK) return 1;
	else return 0;	
}




#if 0
void DirectPlay_ExitLobbiedGame()
{
	HRESULT hres = IDirectPlayX_DestroyPlayer(glpDP, AVPDPNetID);

	if(AvP.Network == I_Host)
	{
		int i;
		int numPlayersLeft=0;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			if(netGameData.playerData[i].playerId)
			{
				extern LPDIRECTPLAYLOBBY3 lpDPlayLobby;
				HRESULT hr;

				DPlayClose();

				CoCreateInstance(&CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, &IID_IDirectPlayLobby3A, (LPVOID*)&lpDPlayLobby);
			    hr = IDirectPlayLobby_ConnectEx(lpDPlayLobby,0,&IID_IDirectPlay4A, &glpDP, NULL);
				if(hr!=DP_OK)
				{
					LOGDXFMT(("Connect Ex : %x",hr));	
				}
								
				//now been demoted to a client
				LobbiedGame=LobbiedGame_Client;

//				DirectPlay_Disconnect();
//				DirectPlay_InitLobbiedGame();
			
//				DPlayCreate(NULL);
				
			
				break;
			}
		}

	}
	AVPDPNetID = NULL;
}
#endif


BOOL DirectPlay_UpdateSessionList(int * SelectedItem)
{
	int i;
	GUID OldSessionGuids[MAX_NO_OF_SESSIONS];
	int OldNumberOfSessions=NumberOfSessionsFound;
	BOOL changed=FALSE;

	//take a list of the old session guids
	for(i=0;i<NumberOfSessionsFound;i++)
	{
		OldSessionGuids[i]=SessionData[i].Guid;
	}

	//do the session enumeration thing
	FindAvPSessions();

	//Have the available sessions changed?
	//first check number of sessions
	if(NumberOfSessionsFound!=OldNumberOfSessions)
	{
		changed = TRUE;
	}
	else
	{
		//now check the guids of the new sessions against our previous list
		for(i=0;i<NumberOfSessionsFound;i++)
		{
			if(!IsEqualGUID(&OldSessionGuids[i],&SessionData[i].Guid))
			{
				changed = TRUE;
			}
		}
	}

	if(changed && OldNumberOfSessions>0)
	{
		//See if our previously selected session is in the new session list
		int OldSelection = *SelectedItem;
		*SelectedItem=0;

		for(i=0;i<NumberOfSessionsFound;i++)
		{
			if(IsEqualGUID(&OldSessionGuids[OldSelection],&SessionData[i].Guid))
			{
				//found the session
				*SelectedItem = i;
				break;
			}
		}
	}

	return changed;
}