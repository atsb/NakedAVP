#ifndef _included_AvP_MP_Config_h_
#define _included_AvP_MP_Config_h_

#ifdef __cplusplus
extern "C" {
#endif

BOOL BuildLoadMPConfigMenu();
void LoadMultiplayerConfigurationByIndex(int index);
void LoadMultiplayerConfiguration(const char* name);
void SaveMultiplayerConfiguration(const char* name);
const char* GetMultiplayerConfigDescription(int index);
void DeleteMultiplayerConfigurationByIndex(int index);


BOOL BuildLoadIPAddressMenu();
void SaveIPAddress(const char* name,const char* address);
void LoadIPAddress(const char* name);

#define LOAD_NEW_MPCONFIG_ENTRIES	(1)
#define SAVE_NEW_MPCONFIG_ENTRIES	(1)



extern int NumCustomLevels;
extern int NumMultiplayerLevels;
extern int NumCoopLevels;

//list of all multiplayer level names as they appear in the menus
extern char** MultiplayerLevelNames;
extern char** CoopLevelNames;

extern void BuildMultiplayerLevelNameArray();

//returns local index of a custom level (if it is a custom level)
int GetCustomMultiplayerLevelIndex(char* name,int gameType);
//returns name of custom level (without stuff tacked on the end)
char* GetCustomMultiplayerLevelName(int index,int gameType);

int GetLocalMultiplayerLevelIndex(int index,char* customLevelName,int gameType);

#ifdef __cplusplus
};
#endif

#endif // _included_AvP_MP_Config_h_
