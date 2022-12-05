/*------------------------------Patrick 18/3/97------------------------
Header for setting up and handling direct play objects
(This is mostly nicked from DHM's headhunter stuff)
-----------------------------------------------------------------------*/
#ifndef dpfunc_h_included
#define dpfunc_h_included
#ifdef __cplusplus
extern "C" {
#endif

/* globals */
extern LPDIRECTPLAY4 glpDP;
extern LPGUID glpGuid;

extern DPNAME AVPDPplayerName;
extern DPID AVPDPNetID;


/* Constants */ 
#define	MAX_SIZE_FORMAL_NAME	128+1
#define	MAX_SIZE_FRIENDLY_NAME	128+1


HRESULT DPlayClose(void);
HRESULT DPlayCreate(LPVOID lpCon);
HRESULT DPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
						  LPVOID lpData, DWORD dwDataSize);
HRESULT DPlayCreateSession(LPTSTR lptszSessionName,int maxPlayers,int dwUser1,int dwUser2);
HRESULT DPlayDestroyPlayer(DPID pid);
HRESULT DPlayEnumPlayers(LPGUID lpSessionGuid, LPDPENUMPLAYERSCALLBACK2 lpEnumCallback, 
						 LPVOID lpContext, DWORD dwFlags);
HRESULT DPlayEnumSessions(DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumCallback, 
						  LPVOID lpContext, DWORD dwFlags);
HRESULT DPlayGetPlayerData(DPID pid, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags);
HRESULT DPlayClose(void);
HRESULT DPlayCreate(LPVOID lpCon);
HRESULT DPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
						  LPVOID lpData, DWORD dwDataSize);						 
HRESULT DPlayDestroyPlayer(DPID pid);
HRESULT DPlayEnumSessions(DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumCallback, 
						  LPVOID lpContext, DWORD dwFlags);
HRESULT DPlayGetPlayerData(DPID pid, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags);
HRESULT DPlayGetSessionDesc(void);
BOOL IsDPlay(void);
HRESULT DPlayOpenSession(LPGUID lpSessionGuid);
HRESULT DPlayReceive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
HRESULT DPlayRelease(void);
HRESULT DPlaySend(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);
HRESULT DPlaySetPlayerData(DPID pid, LPVOID lpData, DWORD dwSize, DWORD dwFlags);


#ifdef __cplusplus
}
#endif
#endif
