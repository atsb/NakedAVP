#ifndef _avp_user_profile_h_
#define _avp_user_profile_h_ 1

#include "usr_io.h"
#include "avp_envinfo.h"
#include "game_statistics.h"
#include "detaillevels.h"
/* KJL 14:17:41 10/12/98 - User profile

	Structures that contains the information required by the single player game

	e.g. which levels have been played, which difficulty levels etc.

 */
#define MAX_NO_OF_USERS 4

#define MAX_SIZE_OF_USERS_NAME 15

enum AVP_DIFFICULTY_LEVEL_ID
{
	AVP_DIFFICULTY_LEVEL_NONE,
	/* the 'none' difficulty level setting can be used to indicate that
	a level has never been completed */
		
	AVP_DIFFICULTY_LEVEL_EASY,
	AVP_DIFFICULTY_LEVEL_MEDIUM,
	AVP_DIFFICULTY_LEVEL_HARD,
};

enum CHEATMODE_ID
{
	CHEATMODE_PIGSTICKING,
	CHEATMODE_SLUGTRAIL,
	CHEATMODE_SNIPERMUNCH,
	CHEATMODE_TERROR,
	CHEATMODE_SUPERGORE,
	CHEATMODE_GRENADE,
	CHEATMODE_MIRROR,
	CHEATMODE_PIPECLEANER,
	CHEATMODE_DISCOINFERNO,
	CHEATMODE_TRIPTASTIC,
	CHEATMODE_MOTIONBLUR,
	CHEATMODE_UNDERWATER,
	CHEATMODE_JOHNWOO,
	CHEATMODE_WARPSPEED,
	CHEATMODE_LANDOFTHEGIANTS,
	CHEATMODE_IMPOSSIBLEMISSION,
	CHEATMODE_RAINBOWBLOOD,
	CHEATMODE_TICKERTAPE,
	CHEATMODE_NAUSEA,
	CHEATMODE_FREEFALL,
	CHEATMODE_BALLSOFFIRE,
	
	MAX_NUMBER_OF_CHEATMODES,


	CHEATMODE_NONACTIVE// leave me at the end!


};

/* Putting this here to get the definition of the cheat enum. */
typedef struct {
	AvP_GameStats_Stored StatTargets;
	enum CHEATMODE_ID CheatModeToActivate;
} AvP_Level_Target_Desc;

typedef struct 
{
	char Name[MAX_SIZE_OF_USERS_NAME+1];

	// SBF: 32-bit time_t
	uint32_t FileTime;

	// SBF: used to be an incomplete SYSTEMTIME struct, TimeLastUpdated
	int unused[6];

	/* KJL 15:14:12 10/12/98 - array to hold level completion data
	3 species, pad out to 16 levels each */
	char LevelCompleted[3][16];

	unsigned char CheatMode[32];
	unsigned char GammaSetting;
	unsigned char AutoWeaponChangeDisabled : 1;
	unsigned char SpareBits : 7; //not used
	char Padding[74];

	int CDPlayerVolume;

	char MultiplayerCallsign[16];

	int SmackerSoundVolume;
	int EffectsSoundVolume;
	int MoviesAreActive;
	int IntroOutroMoviesAreActive;

	MENU_DETAIL_LEVEL_OPTIONS DetailLevelSettings;
		
	PLAYER_INPUT_CONFIGURATION MarineInputPrimaryConfig;
	PLAYER_INPUT_CONFIGURATION MarineInputSecondaryConfig;
	PLAYER_INPUT_CONFIGURATION AlienInputPrimaryConfig;
	PLAYER_INPUT_CONFIGURATION AlienInputSecondaryConfig;
	PLAYER_INPUT_CONFIGURATION PredatorInputPrimaryConfig;
	PLAYER_INPUT_CONFIGURATION PredatorInputSecondaryConfig;
	CONTROL_METHODS ControlMethods;
	JOYSTICK_CONTROL_METHODS JoystickControlMethods;

	/* This feels a bit bloaty. */
	AvP_GameStats_Stored PersonalBests[I_MaxDifficulties][AVP_ENVIRONMENT_END_OF_LIST];
	/* Yes, it contains impossible!  So sue me! */

} AVP_USER_PROFILE;



#define SUPERGORE_MODE				(CheatMode_Active == CHEATMODE_SUPERGORE)
#define SLUGTRAIL_MODE				(CheatMode_Active == CHEATMODE_SLUGTRAIL)
#define TERROR_MODE					(CheatMode_Active == CHEATMODE_TERROR)
#define GRENADE_MODE				(CheatMode_Active == CHEATMODE_GRENADE)
#define PIGSTICKING_MODE			(CheatMode_Active == CHEATMODE_PIGSTICKING)
#define SNIPERMUNCH_MODE			(CheatMode_Active == CHEATMODE_SNIPERMUNCH)
#define MIRROR_CHEATMODE 			(CheatMode_Active == CHEATMODE_MIRROR)
#define PIPECLEANER_CHEATMODE 		(CheatMode_Active == CHEATMODE_PIPECLEANER)
#define DISCOINFERNO_CHEATMODE 		(CheatMode_Active == CHEATMODE_DISCOINFERNO)
#define TRIPTASTIC_CHEATMODE 		(CheatMode_Active == CHEATMODE_TRIPTASTIC)
#define MOTIONBLUR_CHEATMODE 		(CheatMode_Active == CHEATMODE_MOTIONBLUR)
#define UNDERWATER_CHEATMODE 		(CheatMode_Active == CHEATMODE_UNDERWATER)
#define JOHNWOO_CHEATMODE			(CheatMode_Active == CHEATMODE_JOHNWOO)
#define WARPSPEED_CHEATMODE			(CheatMode_Active == CHEATMODE_WARPSPEED)
#define LANDOFTHEGIANTS_CHEATMODE	(CheatMode_Active == CHEATMODE_LANDOFTHEGIANTS)	
#define IMPOSSIBLEMISSION_CHEATMODE	(CheatMode_Active == CHEATMODE_IMPOSSIBLEMISSION)	
#define RAINBOWBLOOD_CHEATMODE		(CheatMode_Active == CHEATMODE_RAINBOWBLOOD)
#define TICKERTAPE_CHEATMODE		(CheatMode_Active == CHEATMODE_TICKERTAPE)
#define NAUSEA_CHEATMODE			(CheatMode_Active == CHEATMODE_NAUSEA)
#define FREEFALL_CHEATMODE			(CheatMode_Active == CHEATMODE_FREEFALL)
#define BALLSOFFIRE_CHEATMODE		(CheatMode_Active == CHEATMODE_BALLSOFFIRE)


/* e.g. to access a cheat mode

	if (UserProfilePtr->CheatMode[CHEATMODE_PIGSTICKING]&CHEATMODE_IS_ACTIVE)
	{
		...
	}
*/




#define USER_PROFILES_PATH "User_Profiles/"
#define USER_PROFILES_WILDCARD_NAME "*.prf"
#define USER_PROFILES_SUFFIX ".prf"


#ifdef __cplusplus
extern "C"
{
#endif

extern void ExamineSavedUserProfiles(void);
extern int NumberOfUserProfiles(void);
extern AVP_USER_PROFILE *GetFirstUserProfile(void);
extern AVP_USER_PROFILE *GetNextUserProfile(void);
extern int SaveUserProfile(AVP_USER_PROFILE *profilePtr);
extern void DeleteUserProfile(int number);

extern void FixCheatModesInUserProfile(AVP_USER_PROFILE *profilePtr);

extern void GetSettingsFromUserProfile(void);
extern void SaveSettingsToUserProfile(AVP_USER_PROFILE *profilePtr);

extern AVP_USER_PROFILE *UserProfilePtr;

extern int CheatMode_Active;
extern int CheatMode_Species;
extern int CheatMode_Environment;


#ifdef __cplusplus									 
}; // extern "C"
#endif

#endif
