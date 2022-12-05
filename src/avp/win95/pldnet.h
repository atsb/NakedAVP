/* Patrick 10/7/97------------------------------------------------------
  Header for Multi-Player game support header
  ----------------------------------------------------------------------*/
#ifndef pldnet_h_included
#define pldnet_h_included
#ifdef __cplusplus
extern "C" {
#endif

#define EXTRAPOLATION_TEST 1

/* Oh, for heaven's sake... */
#include "psnd.h"
#include "particle.h"
/* ---------------------------------------------------------------------
  Some defines for multiplayer games
  ----------------------------------------------------------------------*/
#define NET_MAXPLAYERS				(8)
#define NET_MAXPLAYEROBJECTS		(15)
#define NET_MAXPLAYERSCORE			(500)
#define NET_MAXGAMETIME				(255) /* minutes */
#define NET_MESSAGEBUFFERSIZE		(3072) /* an adequate number of bytes */
#define NET_PLAYERNAMELENGTH		(13)
#define NET_EULERSCALESHIFT			(4)
#define NET_MAXOBJECTID				(32767) 
#define NET_MESSAGEITERATIONS		(5)
#define NET_MAXTEAMS				(4)

#define NET_IDNOTINPLAYERLIST		(-1)
#define NET_NOEMPTYSLOTINPLAYERLIST	(-1)

#define NET_STARTUPINTEGRITY 		(ONE_FIXED*2)

#define NETGAMESPEED_70PERCENT 0
#define NETGAMESPEED_80PERCENT 1
#define NETGAMESPEED_90PERCENT 2
#define NETGAMESPEED_100PERCENT 3

#define JOINNETGAME_WAITFORSTART 0
#define JOINNETGAME_WAITFORDESC 1
#define JOINNETGAME_WRONGAVPVERSION 2
#define JOINNETGAME_DONTHAVELEVEL 3

/* ---------------------------------------------------------------------
  Enum of message types
  ----------------------------------------------------------------------*/
typedef enum netmessagetype
{
	NetMT_GameDescription,
	NetMT_PlayerDescription,
	NetMT_StartGame,
	NetMT_PlayerState,	 
	NetMT_PlayerState_Minimal,	 
	NetMT_PlayerState_Medium,	 
	NetMT_PlayerKilled,	 
	NetMT_PlayerLeaving,		 
	NetMT_AllGameScores,	 
	NetMT_PlayerScores,	 
	NetMT_LocalRicochet,	  
	NetMT_LocalObjectState,	   
	NetMT_LocalObjectDamaged,  
	NetMT_LocalObjectDestroyed,
	NetMT_ObjectPickedUp,							
	NetMT_InanimateObjectDamaged,  
	NetMT_InanimateObjectDestroyed,
	NetMT_LOSRequestBinarySwitch,  
	NetMT_PlatformLiftState,  
	NetMT_RequestPlatformLiftActivate,
	NetMT_PlayerAutoGunState,
	NetMT_MakeDecal,
	NetMT_ChatBroadcast,
	NetMT_MakeExplosion,
	NetMT_MakeFlechetteExplosion,
	NetMT_MakePlasmaExplosion,
	NetMT_PredatorSights,
	NetMT_LocalObjectOnFire,
	NetMT_RestartNetworkGame,
	NetMT_FragmentalObjectsStatus,
	NetMT_AlienAIState,
	NetMT_AlienAISequenceChange,
	NetMT_AlienAIKilled,
	NetMT_GhostHierarchyDamaged,
	NetMT_SpotAlienSound,
	NetMT_LocalObjectDestroyed_Request,
	NetMT_LastManStanding_Restart,
	NetMT_LastManStanding_RestartInfo,
	NetMT_LastManStanding_RestartCountDown,
	NetMT_LastManStanding_LastMan,
	NetMT_PredatorTag_NewPredator,  //same message also used for alien tag
	NetMT_EndGame,		 
	NetMT_CreateWeapon,
	NetMT_RespawnPickups,
	NetMT_ScoreChange,
	NetMT_Gibbing,
	NetMT_CorpseDeathAnim,
	NetMT_StrategySynch,
	NetMT_FrameTimer,
	NetMT_SpeciesScores,
	NetMT_FarAlienPosition,
	NetMT_SpotOtherSound,
}NETMESSAGETYPE;

/* ---------------------------------------------------------------------
  Enums of game types, and local game states
  ----------------------------------------------------------------------*/
typedef enum netgame_type
{
	NGT_Individual,
	NGT_CoopDeathmatch,
	NGT_LastManStanding,
	NGT_PredatorTag,
	NGT_Coop,
	NGT_AlienTag
}NETGAME_TYPE;

typedef enum netgame_states
{
	NGS_StartUp,
	NGS_Joining,
	NGS_Playing,	
	NGS_Leaving,
	NGS_EndGame,
	NGS_Error_GameFull,
	NGS_Error_GameStarted,
	NGS_Error_HostLost,
	NGS_EndGameScreen,
}NETGAME_STATES;


#define NUM_PC_TYPES 3
typedef enum netgame_charactertype
{
	NGCT_Marine,
	NGCT_Predator,
	NGCT_Alien,
	NGCT_AI_Alien,
	NGCT_AI_Predalien,
	NGCT_AI_Praetorian,
}NETGAME_CHARACTERTYPE;

#define NUM_PC_SUBTYPES 9
typedef enum netgame_specialistcharactertype
{
	NGSCT_General,
	NGSCT_PulseRifle,
	NGSCT_Smartgun,
	NGSCT_Flamer,
	NGSCT_Sadar,
	NGSCT_GrenadeLauncher,
	NGSCT_Minigun,
	NGSCT_Frisbee,
	NGSCT_Pistols,
}NETGAME_SPECIALISTCHARACTERTYPE;

typedef enum netgame_connectiontype
{
	CONN_TCPIP,
	CONN_IPX,
	CONN_Serial,
	CONN_Modem,
	CONN_Mplayer,
}NETGAME_CONNECTIONTYPE;

/* ---------------------------------------------------------------------
  Player data structure, and game description data structure
  ----------------------------------------------------------------------*/
typedef struct netgame_playerdata
{
	DPID playerId;
	
	char name[NET_PLAYERNAMELENGTH];
	NETGAME_CHARACTERTYPE characterType;
	NETGAME_SPECIALISTCHARACTERTYPE characterSubType;
	int playerFrags[NET_MAXPLAYERS];
	int aliensKilled[3];
	int deathsFromAI;
	int playerScore;
	int playerScoreAgainst; //points scored by killing this player

	VECTORCH lastKnownPosition;

	unsigned int timer; //used by extrapolation stuff (doesn't matter that it isn't initialised)

	unsigned char startFlag;
	unsigned char playerAlive:1;
	unsigned char playerHasLives:1;


}NETGAME_PLAYERDATA;

typedef struct netgame_gamedata
{
	NETGAME_STATES myGameState;
	NETGAME_CHARACTERTYPE myCharacterType;
	NETGAME_CHARACTERTYPE myNextCharacterType; //if player is currently dead and about to become a new character
	NETGAME_SPECIALISTCHARACTERTYPE myCharacterSubType;
	unsigned char myStartFlag;
	NETGAME_PLAYERDATA playerData[NET_MAXPLAYERS];
	int teamScores[3];
	NETGAME_TYPE gameType;
	int levelNumber;
	int scoreLimit;
	int timeLimit;
	int invulnerableTime;//in seconds after respawn
	int GameTimeElapsed;

	//scoring system stuff
	int characterKillValues[3];
	int baseKillValue;
	BOOL useDynamicScoring;
	BOOL useCharacterKillValues;

	int aiKillValues[3];

	//for last man standing game
	int LMS_AlienIndex;
	int LMS_RestartTimer;

	int stateCheckTimeDelay;

	/*following timer used to prevent the game description from being sent every frame*/
	int gameDescriptionTimeDelay;

	/*sendFrequencey - how often to send messages in fixed point seconds*/
	int sendFrequency;
	int sendTimer;

	//player type limits
	int maxPredator;
	int maxAlien;
	int maxMarine;

	int maxMarineGeneral;
	int maxMarinePulseRifle;
	int maxMarineSmartgun;
	int maxMarineFlamer;
	int maxMarineSadar;
	int maxMarineGrenade;
	int maxMarineMinigun;
	int maxMarineSmartDisc;
	int maxMarinePistols;

	//weapons allowed
	BOOL allowSmartgun;
	BOOL allowFlamer;
	BOOL allowSadar;
	BOOL allowGrenadeLauncher;
	BOOL allowMinigun;
	BOOL allowDisc;
	BOOL allowPistol;
	BOOL allowPlasmaCaster;
	BOOL allowSpeargun;
	BOOL allowMedicomp;
	BOOL allowSmartDisc;
	BOOL allowPistols;

	//lives
	int maxLives;
	BOOL useSharedLives;
	int numDeaths[3];	//deaths for each species

	//object respawn
	int pointsForRespawn; 
	int timeForRespawn; //seconds

	int lastPointsBasedRespawn;
		
	
	
	BOOL sendDecals;
	unsigned int needGameDescription :1;

	BOOL skirmishMode; //for single player multiplayer games

	int myLastScream;

	int gameSpeed; //0 to 3 , for 70,80,90 or 100 percent speed

	BOOL preDestroyLights;
	BOOL disableFriendlyFire; //stops people on the same team from hurting each other
	BOOL fallingDamage;
	BOOL pistolInfiniteAmmo;
	BOOL specialistPistols;

	//don't bother tring to synch strategies if the checksum values are different
	int myStrategyCheckSum; 

	unsigned int tcpip_available :1;
	unsigned int ipx_available :1;
	unsigned int modem_available :1;
	unsigned int serial_available :1;

	NETGAME_CONNECTIONTYPE connectionType;
	
	unsigned int landingNoise:1;

	int joiningGameStatus;

	char customLevelName[40];

}NETGAME_GAMEDATA;

/* ---------------------------------------------------------------------
  Individual message structures
   ----------------------------------------------------------------------*/

PACKED_PUSH
typedef struct netmessageheader
{
	unsigned char type;
} PACKED NETMESSAGEHEADER;
PACKED_POP

PACKED_PUSH
typedef struct gamedescription_playerdata
{
	DPID playerId;
	
	unsigned char characterType:2;
	unsigned char characterSubType:6;
	unsigned char startFlag;
} PACKED GAMEDESCRIPTION_PLAYERDATA;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_gamedescription
{
	GAMEDESCRIPTION_PLAYERDATA players[NET_MAXPLAYERS];	
	unsigned char gameType;
	unsigned char levelNumber; 
	int scoreLimit;
	unsigned char timeLimit;
	unsigned char invulnerableTime;

	unsigned char maxLives;
	unsigned char numDeaths[3];
	unsigned char timeForRespawn;
	int pointsForRespawn;

	short GameTimeElapsed;//in seconds

	//scoring system stuff
	unsigned char characterKillValues[3];
	unsigned char baseKillValue;
	unsigned int useDynamicScoring:1;
	unsigned int useCharacterKillValues:1;
	
	unsigned char aiKillValues[3];

	unsigned int sendDecals :1;
	//weapons allowed
	unsigned int allowSmartgun:1;
	unsigned int allowFlamer:1;
	unsigned int allowSadar:1;
	unsigned int allowGrenadeLauncher:1;
	unsigned int allowMinigun:1;
	unsigned int allowDisc:1;
	unsigned int allowPistol:1;
	unsigned int allowPlasmaCaster:1;
	unsigned int allowSpeargun:1;
	unsigned int allowMedicomp:1;
	unsigned int allowSmartDisc:1;
	unsigned int allowPistols:1;

	//player type limits
	unsigned int maxPredator:4;
	unsigned int maxAlien:4;
	unsigned int maxMarine:4;

	unsigned int maxMarineGeneral:4;
	unsigned int maxMarinePulseRifle:4;
	unsigned int maxMarineSmartgun:4;
	unsigned int maxMarineFlamer:4;
	unsigned int maxMarineSadar:4;
	unsigned int maxMarineGrenade:4;
	unsigned int maxMarineMinigun:4;
	unsigned int maxMarineSmartDisc:4;
	unsigned int maxMarinePistols:4;

	unsigned int useSharedLives:1;

	unsigned int gameSpeed:2;

	unsigned int endGame:1;

	unsigned int preDestroyLights:1;
	unsigned int disableFriendlyFire:1;
	unsigned int fallingDamage:1;
	unsigned int pistolInfiniteAmmo:1;
	unsigned int specialistPistols:1;

} PACKED NETMESSAGE_GAMEDESCRIPTION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerdescription
{
	unsigned char characterType: 3;
	unsigned char characterSubType: 4;
	unsigned char startFlag: 1;	
} PACKED NETMESSAGE_PLAYERDESCRIPTION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerstate
{
	unsigned char characterType :2; //send character type each frame (in case it changes)
	unsigned char nextCharacterType :2; //if player is currently dead and about to become a new character
	unsigned char characterSubType :4;
	signed int xPos: 23;
	signed int xOrient: 9;
	signed int yPos: 23;
	signed int yOrient: 9;
	signed int zPos: 23;
	signed int zOrient: 9;
	unsigned char sequence;
	unsigned char currentWeapon;
	unsigned short CloakingEffectiveness;

	unsigned int Elevation : 12;
	unsigned int IHaveAMuzzleFlash: 2;
	unsigned int IAmFiringPrimary: 1;								
	unsigned int IAmFiringSecondary: 1;								
	unsigned int IAmAlive: 1;
//	unsigned int IAmHost: 1;
	unsigned int IAmOnFire: 1;
	unsigned int IHaveADisk:	1;
	unsigned int IHaveLifeLeft:1;
	unsigned int IAmCrouched:1;
	unsigned int Special:1;

	unsigned int scream:5;

	unsigned int IAmInvulnerable:1;

#if EXTRAPOLATION_TEST
	//this lot will need to be sent more efficiently
	int standard_gravity:1;
	int velocity_x :10; //in 10's of cm/second
	int velocity_y :10;	//in 10's of cm/second
	int velocity_z :10;	//in 10's of cm/second
#endif
	unsigned int landingNoise:1;

} PACKED NETMESSAGE_PLAYERSTATE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerstate_minimal
{
	unsigned short Elevation : 12;
	unsigned short IHaveAMuzzleFlash: 2;
	unsigned short IAmFiringPrimary: 1;								
	unsigned short IAmFiringSecondary: 1;								
	unsigned short IAmAlive: 1;
	unsigned char IAmOnFire: 1;
	unsigned char IHaveADisk:	1;
	unsigned char IHaveLifeLeft:1;
	unsigned char Special:1;

	unsigned char CloakingEffectiveness;
} PACKED NETMESSAGE_PLAYERSTATE_MINIMAL;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerstate_medium
{
	NETMESSAGE_PLAYERSTATE_MINIMAL minimalMessage;

	signed int xOrient: 9;
	signed int yOrient: 9;
	signed int zOrient: 9;

} PACKED NETMESSAGE_PLAYERSTATE_MEDIUM;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_frametimer
{
	unsigned short frame_time;
} PACKED NETMESSAGE_FRAMETIMER;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerkilled
{
	int objectId;
	DPID killerId;
	
	NETGAME_CHARACTERTYPE myType;  //take character types at time of death , in case they change
	NETGAME_CHARACTERTYPE killerType;
	char weaponIcon;
} PACKED NETMESSAGE_PLAYERKILLED;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_corpsedeathanim
{
	int objectId;
	int deathId;
} PACKED NETMESSAGE_CORPSEDEATHANIM;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_allgamescores
{
	int playerFrags[NET_MAXPLAYERS][NET_MAXPLAYERS];
	int playerScores[NET_MAXPLAYERS];
	int playerScoresAgainst[NET_MAXPLAYERS];
	int aliensKilled[NET_MAXPLAYERS][3];
	int deathsFromAI[NET_MAXPLAYERS];
	
} PACKED NETMESSAGE_ALLGAMESCORES;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_speciesscores
{
	int teamScores[3];
} PACKED NETMESSAGE_SPECIESSCORES;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_playerscores
{
	unsigned char playerId;
	int playerFrags[NET_MAXPLAYERS];
	int playerScore;
	int playerScoreAgainst;
	int aliensKilled[3];
	int deathsFromAI;
} PACKED NETMESSAGE_PLAYERSCORES;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_scorechange
{
	unsigned char killerIndex;
	unsigned char victimIndex;
	int fragCount;
	int killerScoreFor;
	int victimScoreAgainst;
} PACKED NETMESSAGE_SCORECHANGE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_localRicochet
{
	signed int xPos;
	signed int yPos;
	signed int zPos;
	signed int xDirn;
	signed int yDirn;
	signed int zDirn;
	unsigned char type;			
} PACKED NETMESSAGE_LOCALRICOCHET;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_lobstate
{
	signed int xPos: 23;
	signed int xOrient: 9;
	signed int yPos: 23;
	signed int yOrient: 9;
	signed int zPos: 23;
	signed int zOrient: 9;
	signed int objectId;
	unsigned char type;
	unsigned char IOType;
	unsigned char subtype;
	unsigned char event_flag;
} PACKED NETMESSAGE_LOBSTATE;
PACKED_POP


//damage message is now split into multiple parts , to avoid sending
//stuff that isn't required
PACKED_PUSH
typedef struct netmessage_lobdamaged_header
{
	DPID playerId;
	
	signed int objectId;
	short ammo_id:11;
	
	short damageProfile:1; 
	short multiple:1; 
	short sectionID:1; 
	short delta_seq:1; 
	short direction:1; 
} PACKED NETMESSAGE_LOBDAMAGED_HEADER;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_ghosthierarchydamaged_header
{
	signed int Guid;
	short ammo_id:11;
	
	short damageProfile:1; 
	short multiple:1; 
	short sectionID:1; 
	short direction:1; 
} PACKED NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_inanimatedamaged_header
{
	char name[8];
	short ammo_id:11;
	
	short damageProfile:1; 
	short multiple:1; 
} PACKED NETMESSAGE_INANIMATEDAMAGED_HEADER;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_damage_profile
{
	short Impact;		 /* nb I have copied these, as I don't think*/
	short Cutting;		 /* the structure will pack if I insert it directly */
	short Penetrative;
	short Fire;
	short Electrical;
	short Acid;

	unsigned int ExplosivePower	:3;
	unsigned int Slicing		:2;
	unsigned int ProduceBlood	:1;
	unsigned int ForceBoom		:2;
	unsigned int BlowUpSections	:1;
	unsigned int Special		:1;
	unsigned int MakeExitWounds	:1;
} PACKED NETMESSAGE_DAMAGE_PROFILE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_damage_multiple
{
	int multiple;
} PACKED NETMESSAGE_DAMAGE_MULTIPLE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_damage_section
{
	short SectionID;
} PACKED NETMESSAGE_DAMAGE_SECTION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_damage_delta
{
	char Delta_Sequence;
	char Delta_Sub_Sequence;
} PACKED NETMESSAGE_DAMAGE_DELTA;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_damage_direction
{
	int direction_x:10;
	int direction_y:10;
	int direction_z:10;
} PACKED NETMESSAGE_DAMAGE_DIRECTION;
PACKED_POP
//that was the last part of the local object damage stuff

PACKED_PUSH
typedef struct netmessage_lobdestroyed_request
{
	DPID playerId;
	
	signed int objectId;
} PACKED NETMESSAGE_LOBDESTROYED_REQUEST;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_lobdestroyed
{
	signed int objectId;
} PACKED NETMESSAGE_LOBDESTROYED;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_objectpickedup
{
	char name[8];
} PACKED NETMESSAGE_OBJECTPICKEDUP;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_inanimatedamaged
{
	char name[8];
	short Impact;		 /* nb I have copied these, as I don't think*/
	short Cutting;		 /* the structure will pack if I insert it directly */
	short Penetrative;
	short Fire;
	short Electrical;
	short Acid;

	unsigned int ExplosivePower	:3;
	unsigned int Slicing		:2;
	unsigned int ProduceBlood	:1;
	unsigned int ForceBoom		:2;
	unsigned int BlowUpSections	:1;
	unsigned int Special		:1;
	unsigned int MakeExitWounds	:1;

	enum AMMO_ID ammo_id;

	int multiple;				

} PACKED NETMESSAGE_INANIMATEDAMAGED;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_inanimatedestroyed
{
	char name[8];
} PACKED NETMESSAGE_INANIMATEDESTROYED;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_losrequestbinaryswitch
{
	char name[8];
} PACKED NETMESSAGE_LOSREQUESTBINARYSWITCH;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_platformliftstate
{
	char name[8];
	char state;
} PACKED NETMESSAGE_PLATFORMLIFTSTATE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_requestplatformliftactivate
{
	char name[8];
} PACKED NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_agunstate
{
	signed int xPos: 23;
	signed int xOrient: 9;
	signed int yPos: 23;
	signed int yOrient: 9;
	signed int zPos: 23;
	signed int zOrient: 9;
	signed int objectId;
	unsigned char IAmFiring: 1;
	unsigned char IAmEnabled: 1;
} PACKED NETMESSAGE_AGUNSTATE;
PACKED_POP

/* KJL 17:45:21 20/01/98 - make decal message */
/* currently not optimised for space! */
#include "decal.h"
PACKED_PUSH
typedef struct netmessage_makedecal
{
	enum DECAL_ID DecalID;
	VECTORCH Position;
	VECTORCH Direction;
	int ModuleIndex;
} PACKED NETMESSAGE_MAKEDECAL;
PACKED_POP

/* KJL 11:32:52 27/04/98 - explosions */
PACKED_PUSH
typedef struct netmessage_makeexplosion
{
	enum EXPLOSION_ID ExplosionID;
	VECTORCH Position;
} PACKED NETMESSAGE_MAKEEXPLOSION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_makeflechetteexplosion
{
	VECTORCH Position;
	int Seed;
} PACKED NETMESSAGE_MAKEFLECHETTEEXPLOSION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_makeplasmaexplosion
{
	enum EXPLOSION_ID ExplosionID;
	VECTORCH Position;
	VECTORCH FromPosition;
} PACKED NETMESSAGE_MAKEPLASMAEXPLOSION;
PACKED_POP

/* KJL 11:13:59 20/05/98 - pred laser sights */
PACKED_PUSH
typedef struct netmessage_predatorsights
{
//	THREE_LASER_DOT_DESC Dots;
// was 85 bytes
	
	signed int xPos: 23;
	signed int xOrient: 9;
	signed int yPos: 23;
	signed int yOrient: 9;
	signed int zPos: 23;
	signed int zOrient: 9;

	DPID TargetID;

	// 16 bytes (DPID 4 bytes?)
} PACKED NETMESSAGE_PREDATORSIGHTS;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_lobonfire
{
	DPID playerId;
	
	signed int objectId;
} PACKED NETMESSAGE_LOBONFIRE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_alienaistate
{
	signed int Guid;

	signed int xPos: 23;
	signed int xOrient: 9;
	signed int yPos: 23;
	signed int yOrient: 9;
	signed int zPos: 23;
	signed int zOrient: 9;

	unsigned char sequence_type;
	unsigned char sub_sequence;
	unsigned short sequence_length:13; //in 256ths of a second , up to ~32 seconds

	#if 0
	unsigned int Elevation : 12;
	unsigned char IAmFiringPrimary: 1;								
	unsigned char IAmFiringSecondary: 1;								
	#endif
	unsigned short IAmOnFire: 1;
	unsigned short AlienType: 2;//alien/predalien/praetorian

	#if EXTRAPOLATION_TEST
	unsigned short speed:15;
	unsigned short standard_gravity:1;
	#endif

} PACKED NETMESSAGE_ALIENAISTATE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_aliensequencechange
{
	signed int Guid;

	unsigned char sequence_type;
	unsigned char sub_sequence;
	short sequence_length; //in 256ths of a second
	short tweening_time;

} PACKED NETMESSAGE_ALIENSEQUENCECHANGE;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_alienaikilled
{
	signed int Guid;
	
	int death_code;
	int death_time;
	int GibbFactor;

	DPID killerId;
	
	int killCount;
	unsigned char AlienType: 2;//alien/predalien/praetorian

	char weaponIcon;
} PACKED NETMESSAGE_ALIENAIKILLED;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_faralienposition
{
	signed int Guid;

	unsigned int targetModuleIndex:14;
	unsigned int index:15; //index is either module index , or an index in the aux locations list
	unsigned int indexIsModuleIndex:1;
	unsigned int alienType:2;

} PACKED NETMESSAGE_FARALIENPOSITION;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_gibbing
{
	signed int Guid;
	int gibbFactor;
	int seed;
} PACKED NETMESSAGE_GIBBING;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_spotaliensound
{
	unsigned char soundCategory:6;
	unsigned char alienType:2;
	int pitch;
	int vx;
	int vy;
	int vz;

} PACKED NETMESSAGE_SPOTALIENSOUND;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_createweapon
{
	char name[8];
	VECTORCH location;
	int type;

} PACKED NETMESSAGE_CREATEWEAPON;
PACKED_POP

#define NUMBER_OF_FRAGMENTAL_OBJECTS (64>>3)
PACKED_PUSH
typedef struct netmessage_fragmentalobjectsstatus
{
	unsigned char BatchNumber; //send object states over several frames
	unsigned char StatusBitfield[NUMBER_OF_FRAGMENTAL_OBJECTS];

} PACKED NETMESSAGE_FRAGMENTALOBJECTSSTATUS;
PACKED_POP

#define NUMBER_OF_STRATEGIES_TO_SYNCH 16
PACKED_PUSH
typedef struct netmessage_strategysynch
{
	unsigned char BatchNumber; //send object states over several frames
	int strategyCheckSum;
	unsigned char StatusBitfield[NUMBER_OF_STRATEGIES_TO_SYNCH>>2]; //2bits per strategy

} PACKED NETMESSAGE_STRATEGYSYNCH;
PACKED_POP


//for messages that just require a player id
PACKED_PUSH
typedef struct netmessage_playerid
{
	DPID playerID; 	
} PACKED NETMESSAGE_PLAYERID;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_lms_restart
{
	DPID playerID; 
	
	int seed;
} PACKED NETMESSAGE_LMS_RESTART;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_restartgame
{
	int seed;
} PACKED NETMESSAGE_RESTARTGAME;
PACKED_POP

//countdown to restart
PACKED_PUSH
typedef struct netmessage_lms_restarttimer
{
	unsigned char timer;
} PACKED NETMESSAGE_LMS_RESTARTTIMER;
PACKED_POP

PACKED_PUSH
typedef struct netmessage_spotothersound
{
	enum soundindex SoundIndex;
	int vx;
	int vy;
	int vz;
	int explosion:1;

} PACKED NETMESSAGE_SPOTOTHERSOUND;
PACKED_POP

PACKED_PUSH
typedef struct multiplayer_start
{
	VECTORCH location;
	EULER orientation;
} PACKED MULTIPLAYER_START;
PACKED_POP


/* ---------------------------------------------------------------------
   Some prototypes
  ----------------------------------------------------------------------*/
extern void InitAVPNetGame(void);
extern void NetCollectMessages(void);
extern void NetSendMessages(void);
extern void EndAVPNetGame(void);
extern int PlayerIdInPlayerList(DPID Id);
//use assignnewsbname instead of addnetgameobjectid
#define AddNetGameObjectID AssignNewSBName
extern void AddNetGameObjectID(STRATEGYBLOCK *sbPtr);
extern void RecordFinalNetGameScores(void);
extern void DoNetScoresForHostDeath(NETGAME_CHARACTERTYPE myType,NETGAME_CHARACTERTYPE killerType);
extern void RemovePlayerFromGame(DPID id);
extern int EmptySlotInPlayerList(void);
extern void TeleportNetPlayerToAStartingPosition(STRATEGYBLOCK *playerSbPtr, int startOfGame);
extern int AddUpPlayerFrags(int playerId);

extern void AddNetMsg_GameDescription(void);
extern void AddNetMsg_PlayerDescription(void);
extern void AddNetMsg_StartGame(void);
extern void AddNetMsg_PlayerState(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_PlayerState_Minimal(STRATEGYBLOCK *sbPtr,BOOL sendOrient);
extern void AddNetMsg_FrameTimer();
extern void AddNetMsg_PlayerKilled(int objectId,DAMAGE_PROFILE* damage);
extern void AddNetMsg_PlayerDeathAnim(int deathId, int objectId);
extern void AddNetMsg_PlayerLeaving(void);
extern void AddNetMsg_AllGameScores(void);
extern void AddNetMsg_PlayerScores(int playerId);
extern void AddNetMsg_SpeciesScores();
extern void AddNetMsg_LocalRicochet(AVP_BEHAVIOUR_TYPE behaviourType, VECTORCH *position, VECTORCH *direction);
extern void AddNetMsg_LocalObjectState(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_LocalObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int sectionID,int delta_seq,int delta_sub_seq,VECTORCH* incoming);
extern void AddNetMsg_LocalObjectDestroyed(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_ObjectPickedUp(char *objectName);
extern void AddNetMsg_InanimateObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);
extern void AddNetMsg_InanimateObjectDestroyed(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_LOSRequestBinarySwitch(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_PlatformLiftState(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_RequestPlatformLiftActivate(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_RequestPlatformLiftReverse(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_PlayerAutoGunState(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_EndGame(void);
extern void AddNetMsg_MakeDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex);
extern void AddNetMsg_ChatBroadcast(char *string,BOOL same_species_only);
extern void AddNetMsg_MakeExplosion(VECTORCH *positionPtr, enum EXPLOSION_ID explosionID);
extern void AddNetMsg_MakeFlechetteExplosion(VECTORCH *positionPtr, int seed);
extern void AddNetMsg_MakePlasmaExplosion(VECTORCH *positionPtr, VECTORCH *fromPositionPtr, enum EXPLOSION_ID explosionID);
extern void AddNetMsg_PredatorLaserSights(VECTORCH *positionPtr, VECTORCH *normalPtr, DISPLAYBLOCK *dispPtr);
extern void AddNetMsg_LocalObjectOnFire(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_RestartNetworkGame(int seed);
extern void AddNetMsg_FragmentalObjectsStatus(void);
extern void AddNetMsg_StrategySynch(void);
extern void AddNetMsg_AlienAIState(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_AlienAISeqChange(STRATEGYBLOCK *sbPtr,int sequence_type,int sub_sequence,int sequence_length,int tweening_time);
extern void AddNetMsg_AlienAIKilled(STRATEGYBLOCK *sbPtr,int death_code,int death_time, int GibbFactor,DAMAGE_PROFILE* damage);
extern void AddNetMsg_FarAlienPosition(STRATEGYBLOCK* sbPtr,int targetModuleIndex,int index,BOOL indexIsModuleIndex);
extern void AddNetMsg_GhostHierarchyDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int sectionID,VECTORCH* incoming);
extern void AddNetMsg_SpotAlienSound(int soundCategory,int alienType,int pitch,VECTORCH *position);
extern void AddNetMsg_LocalObjectDestroyed_Request(STRATEGYBLOCK *sbPtr);
extern void AddNetMsg_ScoreChange(int killerIndex,int victimIndex);

extern void AddNetMsg_PlayerID(DPID playerID,unsigned char message);
extern void AddNetMsg_LastManStanding_RestartTimer(char time);
extern void AddNetMsg_LastManStanding_Restart(DPID alienID,int seed);

extern void AddNetMsg_CreateWeapon(char* objectName,int type,VECTORCH* location);

extern void AddNetMsg_RespawnPickups();

extern void AddNetMsg_Gibbing(STRATEGYBLOCK* sbPtr,int gibbFactor,int seed);
extern void AddNetMsg_SpotOtherSound(enum soundindex SoundIndex,VECTORCH *position,int explosion);

extern void TransmitEndOfGameNetMsg(void);
extern void TransmitPlayerLeavingNetMsg(void);
extern void TransmitStartGameNetMsg(void);

extern void RestartNetworkGame(int seed);

extern void DeallocatePlayersMirrorImage();

void InitNetLog(void);
void LogNetInfo(char *msg);

extern BOOL AreThereAnyLivesLeft();

extern void DoMultiplayerSpecificHud();

extern void GetNextMultiplayerObservedPlayer();
extern void TurnOffMultiplayerObserveMode();
extern void CheckStateOfObservedPlayer();
void MinimalNetCollectMessages(void);
void InitAVPNetGameForHost(int species, int gamestyle, int level);
void InitAVPNetGameForJoin(void);
void CreatePlayersImageInMirror(void);
void RenderPlayersImageInMirror(void);


/* ---------------------------------------------------------------------
   Some global references
  ----------------------------------------------------------------------*/
extern NETGAME_GAMEDATA netGameData;
extern int peerStartUpIntegrities[];

extern int numMarineStartPos;
extern int numAlienStartPos;
extern int numPredatorStartPos;

extern MULTIPLAYER_START* marineStartPositions;
extern MULTIPLAYER_START* alienStartPositions;
extern MULTIPLAYER_START* predatorStartPositions;

#define LobbiedGame_NotLobbied 0
#define LobbiedGame_Server 1
#define LobbiedGame_Client 2
extern int LobbiedGame;

#ifdef __cplusplus
}

#endif
#endif
