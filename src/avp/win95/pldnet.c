/* Patrick 19/3/97 -----------------------------------------------------
  Source for Multi-Player game support 
  ----------------------------------------------------------------------*/

#include <ctype.h>
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "dynblock.h"
#include "dynamics.h"
#include "bh_debri.h"
#include "pvisible.h"
#include "bh_plift.h"
#include "pldnet.h"
#include "pldghost.h"
#include "equipmnt.h"
#include "weapons.h"
#include "multmenu.h"
#include "bh_gener.h"
#include "bh_lnksw.h"
#include "bh_track.h"
#include "psnd.h"
#include "kshape.h"
#include "pfarlocs.h"
#include "avpview.h"
#include "net.h"
#include "los.h"
#include "maths.h"
#include "opengl.h"

/* these required sequence enumerations...*/
#include "bh_pred.h"
#include "bh_alien.h"
#include "bh_marin.h"
#include "bh_binsw.h"
#include "bh_agun.h"
#include "bh_weap.h"

#include "bh_corpse.h"
#include "bh_light.h"

#include "scream.h"

#include "targeting.h"

#include "iofocus.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "showcmds.h"
#define DB_LEVEL 3
#include "db.h"

/* in net.c */
extern DPID AVPDPNetID;
extern int QuickStartMultiplayer;
extern DPNAME AVPDPplayerName;
extern int glpDP;

#define CalculateBytesSentPerSecond 0
							  
/*----------------------------------------------------------------------
  Some globals for use in this file
  ----------------------------------------------------------------------*/
NETGAME_GAMEDATA netGameData=
{
	0,	//NETGAME_STATES myGameState;
	0,	//NETGAME_CHARACTERTYPE myCharacterType;
	0,	//NETGAME_CHARACTERTYPE myNextCharacterType; //if player is currently dead and about to become a new character
	0,	//NETGAME_SPECIALISTCHARACTERTYPE myCharacterSubType;
	0,	//unsigned char myStartFlag;
	{{0},},	//NETGAME_PLAYERDATA playerData[NET_MAXPLAYERS];
	{0,},	//int teamScores[3];
	0,	//NETGAME_TYPE gameType;
	0,	//unsigned char levelNumber;
	1000000,	//unsigned int scoreLimit;
	255,	//unsigned char timeLimit;
	5,	//int invulnerableTime;//in seconds after respawn
	0,	//int GameTimeElapsed;

	//scoring system stuff
	{100,150,75},	//int characterKillValues[3];
	100,	//int baseKillValue;
	TRUE,	//BOOL useDynamicScoring;
	TRUE,	//BOOL useCharacterKillValues;

	{75,100,150},	//int aiKillValues[3];
	
	//for last man standing game
	-1,	//int LMS_AlienIndex;
	0,	//int LMS_RestartTimer;

	0,	//int stateCheckTimeDelay;

	/*following timer used to prevent the game description from being sent every frame*/
	0,	//int gameDescriptionTimeDelay;

	/*sendFrequencey - how often to send messages in fixed point seconds*/
	0,	//int sendFrequency;
	0,	//int sendTimer;

	//player type limits
	8,	//unsigned int maxPredator;
	8,	//unsigned int maxAlien;
	8,	//unsigned int maxMarine;

	8,	//unsigned int maxMarineGeneral;
	8,	//unsigned int maxMarinePulseRifle;
	8,	//unsigned int maxMarineSmartgun;
	8,	//unsigned int maxMarineFlamer;
	8,	//unsigned int maxMarineSadar;
	8,	//unsigned int maxMarineGrenade;
	8,	//unsigned int maxMarineMinigun;
	8,	//unsigned int maxMarineSmartDisc;
	8,	//unsigned int maxMarinePistols;

	//weapons allowed
	TRUE,	//BOOL allowSmartgun;
	TRUE,	//BOOL allowFlamer;
	TRUE,	//BOOL allowSadar;
	TRUE,	//BOOL allowGrenadeLauncher;
	TRUE,	//BOOL allowMinigun;
	TRUE,	//BOOL allowDisc;
	TRUE,	//BOOL allowPistol;
	TRUE,	//BOOL allowPlasmaCaster;
	TRUE,	//BOOL allowSpeargun;
	TRUE,	//BOOL allowMedicomp;
	TRUE,	//BOOL allowSmartDisc;
	TRUE,	//BOOL allowPistols;
	
	0,		//int maxLives;
	FALSE,	//BOOL useSharedLives;
	{0,0,0},//int numDeaths[3];

	0,		//int pointsForResapwn; 
	40,		//int timeForRespawn; //seconds
	
	0,		//int lastPointsBasedRespawn;
	
	TRUE,	//BOOL sendDecals;
	0,	//unsigned int needGameDescription :1;
	FALSE,	//BOOL skirmishMode

	-1, //char myLastScream;

	NETGAMESPEED_100PERCENT, //int gameSpeed;
	0,	//BOOL preDestroyLights;
	0,	//BOOL disableFriendlyFire;
	1,	//BOOL fallingDamage;
	1,	//BOOL pistolInfiniteAmmo;
	1,	//BOOL specialistPistols;
	0,	//int myStrategyCheckSum; 

	0,	//unsigned int tcpip_available :1;
	0,	//unsigned int ipx_available :1;
	0,	//unsigned int modem_available :1;
	0,	//unsigned int serial_available :1;
	0,	//NETGAME_CONNECTIONTYPE connectionType;

	0,	//landingNoise:1;
	0,	//int joiningGameStatus;
	"", //char customLevelName[];
};

void SetDefaultMultiplayerConfig()
{
	netGameData.scoreLimit=1000000;	
	netGameData.timeLimit=255;
	netGameData.invulnerableTime=5;
	netGameData.characterKillValues[0]=100;
	netGameData.characterKillValues[1]=150;
	netGameData.characterKillValues[2]=75;

	netGameData.baseKillValue=100;
	netGameData.useDynamicScoring=1;
	netGameData.useCharacterKillValues=1;

	netGameData.aiKillValues[0]=75;
	netGameData.aiKillValues[1]=100;
	netGameData.aiKillValues[2]=150;

	netGameData.maxMarine=8;
	netGameData.maxAlien=8;
	netGameData.maxPredator=8;
	netGameData.maxMarineGeneral=8;
	netGameData.maxMarinePulseRifle=8;
	netGameData.maxMarineSmartgun=8;
	netGameData.maxMarineFlamer=8;
	netGameData.maxMarineSadar=8;
	netGameData.maxMarineGrenade=8;
	netGameData.maxMarineMinigun=8;
	netGameData.maxMarineSmartDisc=8;
	netGameData.maxMarinePistols=8;

	netGameData.allowSmartgun=1;
	netGameData.allowFlamer=1;
	netGameData.allowSadar=1;
	netGameData.allowGrenadeLauncher=1;
	netGameData.allowMinigun=1;
	netGameData.allowDisc=1;
	netGameData.allowPistol=1;
	netGameData.allowPlasmaCaster=1;
	netGameData.allowSpeargun=1;
	netGameData.allowMedicomp=1;
	netGameData.allowSmartDisc=1;
	netGameData.allowPistols=1;

	netGameData.maxLives=0;
	netGameData.useSharedLives=0;
	netGameData.pointsForRespawn=0;
	netGameData.timeForRespawn=40;

	netGameData.gameSpeed=NETGAMESPEED_100PERCENT;

	netGameData.preDestroyLights=FALSE;
	netGameData.disableFriendlyFire=FALSE;
	netGameData.fallingDamage=TRUE;
	netGameData.pistolInfiniteAmmo=TRUE;
	netGameData.specialistPistols=TRUE;
}

int LobbiedGame=0;




static char sendBuffer[NET_MESSAGEBUFFERSIZE];
static char *endSendBuffer;
static int netNextLocalObjectId = 1;
DPID myNetworkKillerId = 0;
DPID myIgniterId = 0;
int MyHitBodyPartId=-1;
DPID MultiplayerObservedPlayer=0;

/* for testing */
static int numMessagesReceived = 0;
static int numMessagesTransmitted = 0;

static char OnScreenMessageBuffer[300];

static unsigned char FragmentalObjectStatus[NUMBER_OF_FRAGMENTAL_OBJECTS];
static unsigned char StrategySynchArray[NUMBER_OF_STRATEGIES_TO_SYNCH>>2];

int numMarineStartPos=0;
int numAlienStartPos=0;
int numPredatorStartPos=0;

MULTIPLAYER_START* marineStartPositions=0;
MULTIPLAYER_START* alienStartPositions=0;
MULTIPLAYER_START* predatorStartPositions=0;


int ShowMultiplayerScoreTimer=0;
int MultiplayerRestartSeed=0;

static int GameTimeSinceLastSend=0; 

static unsigned int TimeCounterForExtrapolation=0;

/*----------------------------------------------------------------------
  External globals (& protoypes)
  ----------------------------------------------------------------------*/
extern int NormalFrameTime;
extern int RealFrameTime;
extern int TimeScale;
extern int PrevNormalFrameTime;

extern void UpdateAlienAIGhostAnimSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);
extern void RemovePickedUpObject(STRATEGYBLOCK *objectPtr);
extern void PrintStringTableEntryInConsole(enum TEXTSTRING_ID string_id);
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern int DebouncedGotAnyKey;
extern int LastHand;  // For alien claws and two pistols

extern void CreateSpearPossiblyWithFragment(DISPLAYBLOCK *dispPtr, VECTORCH *spearPositionPtr, VECTORCH *spearDirectionPtr);
extern void NewOnScreenMessage(unsigned char *messagePtr);
/*----------------------------------------------------------------------
  Some protoypes for this file
  ----------------------------------------------------------------------*/
static void ProcessSystemMessage(char *msgP,unsigned int msgSize);
static void ProcessGameMessage(DPID senderId, char *msgP,unsigned int msgSize);
static void AddPlayerToGame(DPID id, char*name);
static void AddPlayerAndObjectUpdateMessages(void);
static void UpdateNetworkGameScores(DPID playerKilledId, DPID killerId,NETGAME_CHARACTERTYPE playerKilledType,NETGAME_CHARACTERTYPE killerType);
//static void ConvertNetNameToUpperCase(char *strPtr);

static void ProcessNetMsg_GameDescription(NETMESSAGE_GAMEDESCRIPTION *msgPtr);
static void ProcessNetMsg_PlayerDescription(NETMESSAGE_PLAYERDESCRIPTION *msgPtr, DPID senderId);
static void ProcessNetMsg_StartGame(void);
static void ProcessNetMsg_PlayerState(NETMESSAGE_PLAYERSTATE *msgPtr, DPID senderId);
static void ProcessNetMsg_PlayerState_Minimal(NETMESSAGE_PLAYERSTATE_MINIMAL *msgPtr, DPID senderId,BOOL orientation);
static void ProcessNetMsg_FrameTimer(unsigned short frame_time, DPID senderId);
static void ProcessNetMsg_PlayerKilled(NETMESSAGE_PLAYERKILLED *msgPtr, DPID senderId);
static void ProcessNetMsg_PlayerDeathAnim(NETMESSAGE_CORPSEDEATHANIM* messagePtr,DPID senderId);
static void ProcessNetMsg_PlayerLeaving(DPID senderId);
static void ProcessNetMsg_AllGameScores(NETMESSAGE_ALLGAMESCORES *msgPtr);
static void ProcessNetMsg_PlayerScores(NETMESSAGE_PLAYERSCORES *msgPtr);
static void ProcessNetMsg_SpeciesScores(NETMESSAGE_SPECIESSCORES *msgPtr);
static void ProcessNetMsg_LocalRicochet(NETMESSAGE_LOCALRICOCHET *msgPtr);
static void ProcessNetMsg_LocalObjectState(NETMESSAGE_LOBSTATE *msgPtr, DPID senderId);
static void ProcessNetMsg_LocalObjectDamaged(char *msgPtr, DPID senderId);
static void ProcessNetMsg_LocalObjectDestroyed(NETMESSAGE_LOBDESTROYED *msgPtr, DPID senderId);
static void ProcessNetMsg_ObjectPickedUp(NETMESSAGE_OBJECTPICKEDUP *messagePtr);
static void ProcessNetMsg_EndGame(void);
static void ProcessNetMsg_InanimateObjectDamaged(char *msgPtr);
static void ProcessNetMsg_InanimateObjectDestroyed(NETMESSAGE_INANIMATEDESTROYED *msgPtr);
static void ProcessNetMsg_LOSRequestBinarySwitch(NETMESSAGE_LOSREQUESTBINARYSWITCH *msgPtr);
static void ProcessNetMsg_PlatformLiftState(NETMESSAGE_PLATFORMLIFTSTATE *msgPtr);
static void ProcessNetMsg_RequestPlatformLiftActivate(NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE *msgPtr);
static void ProcessNetMsg_PlayerAutoGunState(NETMESSAGE_AGUNSTATE *msgPtr, DPID senderId);
static char *ProcessNetMsg_ChatBroadcast(char *subMessagePtr, DPID senderId);
static void ProcessNetMsg_MakeExplosion(NETMESSAGE_MAKEEXPLOSION *messagePtr);
static void ProcessNetMsg_MakeFlechetteExplosion(NETMESSAGE_MAKEFLECHETTEEXPLOSION *messagePtr);
static void ProcessNetMsg_MakePlasmaExplosion(NETMESSAGE_MAKEPLASMAEXPLOSION *messagePtr);
static void ProcessNetMsg_PredatorSights(NETMESSAGE_PREDATORSIGHTS *messagePtr, DPID senderId);
static void ProcessNetMsg_FragmentalObjectsStatus(NETMESSAGE_FRAGMENTALOBJECTSSTATUS *messagePtr);
static void ProcessNetMsg_StrategySynch(NETMESSAGE_STRATEGYSYNCH *messagePtr);
static void ProcessNetMsg_LocalObjectOnFire(NETMESSAGE_LOBONFIRE *messagePtr, DPID senderId);
static void ProcessNetMsg_AlienAIState(NETMESSAGE_ALIENAISTATE *messagePtr, DPID senderId);
static void ProcessNetMsg_FarAlienPosition(NETMESSAGE_FARALIENPOSITION *messagePtr, DPID senderId);
static void ProcessNetMsg_SpotAlienSound(NETMESSAGE_SPOTALIENSOUND *messagePtr, DPID senderId);
static void ProcessNetMsg_LocalObjectDestroyed_Request(NETMESSAGE_LOBDESTROYED_REQUEST *messagePtr, DPID senderId);
static void ProcessNetMsg_ScoreChange(NETMESSAGE_SCORECHANGE *messagePtr);
static void ProcessNetMsg_CreateWeapon(NETMESSAGE_CREATEWEAPON *messagePtr);
static void ProcessNetMsg_Gibbing(NETMESSAGE_GIBBING* messagePtr,DPID senderId);
static void ProcessNetMsg_GhostHierarchyDamaged(char *messagePtr, DPID senderId);
static void ProcessNetMsg_MakeDecal(NETMESSAGE_MAKEDECAL *messagePtr);
static void ProcessNetMsg_AlienAISequenceChange(NETMESSAGE_ALIENSEQUENCECHANGE *messagePtr, DPID senderId);
static void ProcessNetMsg_AlienAIKilled(NETMESSAGE_ALIENAIKILLED *messagePtr, DPID senderId);
static void ProcessNetMsg_SpotOtherSound(NETMESSAGE_SPOTOTHERSOUND *messagePtr, DPID senderId);
void StartOfGame_PlayerPlacement(STRATEGYBLOCK *playerSbPtr,int seed);

static void CheckLastManStandingState();
static void Handle_LastManStanding_Restart(DPID alienID,int seed);
static void Handle_LastManStanding_RestartInfo(DPID alienID);
static void Handle_LastManStanding_LastMan(DPID marineID);
static void Handle_LastManStanding_RestartTimer(unsigned char time);

static void CheckSpeciesTagState();
static void Handle_SpeciesTag_NewPersonIt(DPID predatorID);
static int CountPlayersOfType(NETGAME_CHARACTERTYPE species);

static int GetSizeOfLocalObjectDamagedMessage(char* messagePtr);
static int GetSizeOfGhostHierarchyDamagedMessage(char* messagePtr);
static int GetSizeOfInanimateDamagedMessage(char *messagePtr);

static STRATEGYBLOCK *FindObjectFromNetIndex(int obIndex);
static STRATEGYBLOCK *FindEnvironmentObjectFromName(char *name);
static void InitialiseSendMessageBuffer(void);

static MARINE_SEQUENCE GetMyMarineSequence(void);
static ALIEN_SEQUENCE GetMyAlienSequence(void);
static PREDATOR_SEQUENCE GetMyPredatorSequence(void);


static void Inform_PlayerHasDied(DPID killer, DPID victim,NETGAME_CHARACTERTYPE killerType,char weaponIcon);
static void Inform_AiHasDied(DPID killer,ALIEN_TYPE type,char weaponIcon);
static void Inform_PlayerHasLeft(DPID player);
static void Inform_PlayerHasJoined(DPID player);
static void Inform_PlayerHasConnected(DPID player);
static void Inform_NewHost(void);

static void WriteFragmentStatus(int fragmentNumber, int status);
static int ReadFragmentStatus(int fragmentNumber);

static void WriteStrategySynch(int objectNumber, int status);
static int ReadStrategySynch(int objectNumber);

static int GetStrategySynchObjectChecksum();

static int GetDynamicScoreMultiplier(int playerKilledIndex,int killerIndex);
static int GetNetScoreForKill(int playerKilledIndex,int killerIndex);

static void NetworkGameConsoleMessage(enum TEXTSTRING_ID stringID,const char* name1,const char* name2);
static void NetworkGameConsoleMessageWithWeaponIcon(enum TEXTSTRING_ID stringID,const char* name1,const char* name2,const char* weaponSymbol);

static void CheckForPointBasedObjectRespawn();

static int CalculateMyScore();

static void PeriodicScoreUpdate();

void CheckStateOfObservedPlayer();
static int MyPlayerHasAMuzzleFlash(STRATEGYBLOCK *sbPtr);

/*----------------------------------------------------------------------
  Initalisation of net game
  ----------------------------------------------------------------------*/
void InitAVPNetGame(void)
{
	extern int QuickStartMultiplayer;
	/* init garry's dp extended */
	DpExtInit(0,0,0);

	/* init the send message buffer */
	InitialiseSendMessageBuffer();

	/* base initialisation of game description */	
	{
		int i,j;
		for(i=0;i<(NET_MAXPLAYERS);i++)
		{
			netGameData.playerData[i].playerId = 0;
			for(j=0;j<(NET_PLAYERNAMELENGTH);j++) netGameData.playerData[i].name[j] = '\0';
			netGameData.playerData[i].characterType = NGCT_Marine;
			netGameData.playerData[i].characterSubType = NGSCT_General;
			for(j=0;j<(NET_MAXPLAYERS);j++) netGameData.playerData[i].playerFrags[j] = 0;
			netGameData.playerData[i].playerScore=0;
			netGameData.playerData[i].playerScoreAgainst=0;
			netGameData.playerData[i].aliensKilled[0]=0;
			netGameData.playerData[i].aliensKilled[1]=0;
			netGameData.playerData[i].aliensKilled[2]=0;
			netGameData.playerData[i].deathsFromAI=0;
			netGameData.playerData[i].playerAlive=1;
			netGameData.playerData[i].playerHasLives=1;
			netGameData.playerData[i].startFlag = 0;		
		}
		for(j=0;j<3;j++) netGameData.teamScores[j] = 0;
		netGameData.myGameState = NGS_Joining;
		switch (QuickStartMultiplayer)
		{
			default:
			case 1:
				netGameData.myCharacterType = NGCT_Marine;
				netGameData.myNextCharacterType = NGCT_Marine;
				break;

			case 2:
				netGameData.myCharacterType = NGCT_Alien;
				netGameData.myNextCharacterType = NGCT_Alien;
				break;

			case 3:
				netGameData.myCharacterType = NGCT_Predator;
				netGameData.myNextCharacterType = NGCT_Predator;
				break;
		}

		netGameData.myStartFlag = 0;
		netGameData.gameType = NGT_Individual;
		netGameData.levelNumber = 0;
		netGameData.scoreLimit = 0;
		netGameData.timeLimit = 0;
		netGameData.invulnerableTime = 5;
		netGameData.GameTimeElapsed = 0;

		netGameData.LMS_AlienIndex=-1;
		netGameData.stateCheckTimeDelay=0;
		netGameData.LMS_RestartTimer=0;
	}

	myNetworkKillerId = AVPDPNetID;	/* init global id of player who killed me last */
	netNextLocalObjectId = 1;	/* init local object network id */
	numMessagesReceived = 0;	/* these are for testing */
	numMessagesTransmitted = 0;

	/* If I'm the host, add myself to the game data */
	if(AvP.Network==I_Host)
	{
		netGameData.playerData[0].playerId = AVPDPNetID;
		strncpy(netGameData.playerData[0].name,AVPDPplayerName.lpszShortNameA,NET_PLAYERNAMELENGTH-1);
		netGameData.playerData[0].name[NET_PLAYERNAMELENGTH-1] = '\0';
//		ConvertNetNameToUpperCase(netGameData.playerData[0].name);
	}

	InitNetLog();
}

void InitAVPNetGameForHost(int species, int gamestyle, int level)
{
	AvP.GameMode = I_GM_Playing;
	AvP.Network = I_Host;
	AvP.NetworkAIServer = (gamestyle==NGT_Coop);
	/* init garry's dp extended */
	if(!netGameData.skirmishMode)
	{
		DpExtInit(0,0,0);
	}

	/* init the send message buffer */
	InitialiseSendMessageBuffer();

	netGameData.myGameState = NGS_Joining;
	/* base initialisation of game description */	
	{
		int i,j;
		for(i=0;i<(NET_MAXPLAYERS);i++)
		{
			netGameData.playerData[i].playerId = 0;		
			for(j=0;j<(NET_PLAYERNAMELENGTH);j++) netGameData.playerData[i].name[j] = '\0';
			netGameData.playerData[i].characterType = NGCT_Marine;
			netGameData.playerData[i].characterSubType = NGSCT_General;
			for(j=0;j<(NET_MAXPLAYERS);j++) netGameData.playerData[i].playerFrags[j] = 0;
			netGameData.playerData[i].playerScore = 0;
			netGameData.playerData[i].playerScoreAgainst = 0;
			netGameData.playerData[i].aliensKilled[0]=0;
			netGameData.playerData[i].aliensKilled[1]=0;
			netGameData.playerData[i].aliensKilled[2]=0;
			netGameData.playerData[i].deathsFromAI=0;
			netGameData.playerData[i].playerAlive=1;
			netGameData.playerData[i].playerHasLives=1;
			netGameData.playerData[i].startFlag = 0;		
		}
		for(j=0;j<3;j++) netGameData.teamScores[j] = 0;
//		netGameData.myGameState = NGS_Playing;

		
		netGameData.myCharacterSubType=NGSCT_General;
		switch (species)
		{
			default:
			case 0:
				netGameData.myCharacterType = NGCT_Marine;
				netGameData.myNextCharacterType = NGCT_Marine;
				AvP.PlayerType = I_Marine;
				break;
			case 1:
				netGameData.myCharacterType = NGCT_Predator;
				netGameData.myNextCharacterType = NGCT_Predator;
				AvP.PlayerType = I_Predator;
				break;
			case 2:
				netGameData.myCharacterType = NGCT_Alien;
				netGameData.myNextCharacterType = NGCT_Alien;
				AvP.PlayerType = I_Alien;
				break;
			case 3:	//various marine subtypes
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				netGameData.myCharacterType = NGCT_Marine;
				netGameData.myNextCharacterType = NGCT_Marine;
				netGameData.myCharacterSubType =(NETGAME_SPECIALISTCHARACTERTYPE) (species-2);
				AvP.PlayerType = I_Marine;
				break;
		}

		netGameData.myStartFlag = 0;
		netGameData.gameType = gamestyle;
		netGameData.levelNumber = level;
//		netGameData.scoreLimit = 0;
//		netGameData.timeLimit = 0;
//		netGameData.invulnerableTime = 5;
		netGameData.GameTimeElapsed = 0;

//		netGameData.characterKillValues[NGCT_Marine]=100;
//		netGameData.characterKillValues[NGCT_Predator]=150;
//		netGameData.characterKillValues[NGCT_Alien]=75;
//		netGameData.baseKillValue=100;
//		netGameData.useDynamicScoring=1;
//		netGameData.useCharacterKillValues=1;

		netGameData.stateCheckTimeDelay=0;
		netGameData.gameDescriptionTimeDelay=0;

		netGameData.sendTimer=0;
		if(LobbiedGame || netGameData.connectionType==CONN_Modem)
		{
			netGameData.sendFrequency=ONE_FIXED/15;
   	   		netGameData.sendDecals=FALSE;
		}
		else
		{
			netGameData.sendFrequency=0;//ONE_FIXED/15;
   	   		netGameData.sendDecals=TRUE;
		}
	}

	myNetworkKillerId = AVPDPNetID;	/* init global id of player who killed me last */
	netNextLocalObjectId = 1;	/* init local object network id */
	numMessagesReceived = 0;	/* these are for testing */
	numMessagesTransmitted = 0;

	/* If I'm the host, add myself to the game data */
	netGameData.playerData[0].playerId = AVPDPNetID;
	strncpy(netGameData.playerData[0].name,AVPDPplayerName.lpszShortNameA,NET_PLAYERNAMELENGTH-1);
	netGameData.playerData[0].name[NET_PLAYERNAMELENGTH-1] = '\0';
//	ConvertNetNameToUpperCase(netGameData.playerData[0].name);

	{
		int myIndex;
		myIndex = PlayerIdInPlayerList(AVPDPNetID);
		LOCALASSERT(myIndex!=NET_IDNOTINPLAYERLIST);
		netGameData.playerData[myIndex].characterType = netGameData.myCharacterType;
		netGameData.playerData[myIndex].characterSubType = netGameData.myCharacterSubType;
	}
	
	netGameData.LMS_AlienIndex=-1;

	InitNetLog();

	//make sure our time scale is set correctly
	switch(netGameData.gameSpeed)
	{
		case NETGAMESPEED_70PERCENT :
			TimeScale=(ONE_FIXED*70)/100;
			break;
		
		case NETGAMESPEED_80PERCENT :
			TimeScale=(ONE_FIXED*80)/100;
			break;
		
		case NETGAMESPEED_90PERCENT :
			TimeScale=(ONE_FIXED*90)/100;
			break;
		
		case NETGAMESPEED_100PERCENT :
			TimeScale=(ONE_FIXED*100)/100;
			break;

	}

	netGameData.myStrategyCheckSum=0;

	netGameData.numDeaths[0]=0;
	netGameData.numDeaths[1]=0;
	netGameData.numDeaths[2]=0;
	
}
void InitAVPNetGameForJoin(void)
{
	AvP.GameMode = I_GM_Playing;
	AvP.Network = I_Peer;
	
	AvP.NetworkAIServer = 0;

	/* init garry's dp extended */
	DpExtInit(0,0,0);

	/* init the send message buffer */
	InitialiseSendMessageBuffer();

	netGameData.myGameState = NGS_Joining;
	/* base initialisation of game description */	
	{
		int i,j;
		for(i=0;i<(NET_MAXPLAYERS);i++)
		{
			netGameData.playerData[i].playerId = 0;		
			for(j=0;j<(NET_PLAYERNAMELENGTH);j++) netGameData.playerData[i].name[j] = '\0';
			netGameData.playerData[i].characterType = NGCT_Marine;
			netGameData.playerData[i].characterSubType = NGSCT_General;
			for(j=0;j<(NET_MAXPLAYERS);j++) netGameData.playerData[i].playerFrags[j] = 0;
			netGameData.playerData[i].playerScore = 0;
			netGameData.playerData[i].playerScoreAgainst = 0;
			netGameData.playerData[i].aliensKilled[0]=0;
			netGameData.playerData[i].aliensKilled[1]=0;
			netGameData.playerData[i].aliensKilled[2]=0;
			netGameData.playerData[i].deathsFromAI=0;
			netGameData.playerData[i].playerAlive=1;
			netGameData.playerData[i].playerHasLives=1;
			netGameData.playerData[i].startFlag = 0;		
		}
		for(j=0;j<3;j++) netGameData.teamScores[j] = 0;
		netGameData.myStartFlag = 0;		
		netGameData.myGameState = NGS_Joining;

		netGameData.myCharacterType = NGCT_Marine;
		netGameData.myNextCharacterType = NGCT_Marine;
		netGameData.myCharacterSubType=NGSCT_General;
		
		netGameData.stateCheckTimeDelay=0;
		netGameData.gameDescriptionTimeDelay=0;
	
		if(LobbiedGame || netGameData.connectionType==CONN_Modem)
		{
			netGameData.sendFrequency=ONE_FIXED/15;
   	   		netGameData.sendDecals=FALSE;
		}
		else
		{
			netGameData.sendFrequency=0;//ONE_FIXED/15;
   	   		netGameData.sendDecals=TRUE;
		}
		netGameData.sendTimer=0;
		
		netGameData.needGameDescription=1;
	}

	myNetworkKillerId = AVPDPNetID;	/* init global id of player who killed me last */
	netNextLocalObjectId = 1;	/* init local object network id */
	numMessagesReceived = 0;	/* these are for testing */
	numMessagesTransmitted = 0;
	InitNetLog();

	
	netGameData.myStrategyCheckSum=0;

	netGameData.numDeaths[0]=0;
	netGameData.numDeaths[1]=0;
	netGameData.numDeaths[2]=0;

	netGameData.joiningGameStatus = JOINNETGAME_WAITFORSTART;
}

/*----------------------------------------------------------------------
  Core message collection function
  ----------------------------------------------------------------------*/
void MinimalNetCollectMessages(void)
{			
	HRESULT res = DP_OK;
	DPID	dPlayFromId = 0;
	DPID 	dPlayToId = 0;
	unsigned char *msgP = NULL;
	unsigned msgSize = 0;
		
	/* collects messages until something other than DP_OK is returned (eg DP_NoMessages) */
	if(!netGameData.skirmishMode)
	{
		while((res==DP_OK) && glpDP && AVPDPNetID)
		{
			res = DpExtRecv(glpDP,&dPlayFromId,&dPlayToId,DPRECEIVE_ALL,&msgP,(LPDWORD)&msgSize);
			if(res==DP_OK)
			{
				/* process last message, if there is one */
				if(dPlayFromId == DPID_SYSMSG)
				{			
					ProcessSystemMessage(msgP,msgSize);
				}
				else ProcessGameMessage(dPlayFromId,msgP,msgSize);										
			}		
		}
	}
}
void NetCollectMessages(void)
{			
	HRESULT res = DP_OK;
	DPID	dPlayFromId = 0;
	DPID 	dPlayToId = 0;
	unsigned char *msgP = NULL;
	unsigned msgSize = 0;
		
	/* first off, some assertions about our game state */
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Leaving)));	
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameFull)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameStarted)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_HostLost)));

	/* only bother colecting messages under certain game conditions... */
	if((netGameData.myGameState!=NGS_StartUp)&&(netGameData.myGameState!=NGS_Playing)&&(netGameData.myGameState!=NGS_Joining)&&(netGameData.myGameState!=NGS_EndGameScreen)) return;

	InitNetLog();
	LogNetInfo("Collecting Messages... \n");

	/* collects messages until something other than DP_OK is returned (eg DP_NoMessages) */
	if(!netGameData.skirmishMode)
	{
		while((res==DP_OK) && glpDP && AVPDPNetID)
		{
			res = DpExtRecv(glpDP,&dPlayFromId,&dPlayToId,DPRECEIVE_ALL,&msgP,(LPDWORD)&msgSize);				
			if(res==DP_OK)
			{
				numMessagesReceived++;
				/* process last message, if there is one */
				if(dPlayFromId == DPID_SYSMSG)
				{			
					ProcessSystemMessage(msgP,msgSize);
				}
				else ProcessGameMessage(dPlayFromId,msgP,msgSize);										
			}	
		}
	}
	LogNetInfo("... Finished collecting Messages\n");

	/* check ghost integrities */
	MaintainGhosts();
	
	/* time and score limit checks...*/
	{
		//update timer
		if(netGameData.myGameState==NGS_Playing)
		{
			netGameData.GameTimeElapsed += RealFrameTime;
		}
		
		if((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Playing))
		{
			if(netGameData.timeLimit>0)
			{
				if(((netGameData.GameTimeElapsed>>16)/60) >= netGameData.timeLimit)
				{
					/* game has timed-out */
					TransmitEndOfGameNetMsg();
					netGameData.myGameState = NGS_EndGameScreen;
				 //	AvP.MainLoopRunning = 0;
				}
			}
			if(netGameData.scoreLimit>0)
			{
				/* nb this doesn't check team scores */
				int i,j;
				for(i=0;i<NET_MAXPLAYERS;i++)
				{
					if(netGameData.playerData[i].playerId)
					{	
						int score=0;
						for(j=0;j<3;j++)
						{
							score+=netGameData.playerData[i].aliensKilled[j]*netGameData.aiKillValues[j];
						}
						if(netGameData.playerData[i].playerScore >= netGameData.scoreLimit || score>=netGameData.scoreLimit)
						{
							/* someone has reached the score limit */
							TransmitEndOfGameNetMsg();
							netGameData.myGameState = NGS_EndGameScreen;
							break;
						//	AvP.MainLoopRunning = 0;
						}
					}
				}			
			}
			if(netGameData.maxLives>0)
			{
				int i;
				int numPlayers=0;
				int numPlayersWithLifeLeft=0;
				int numAliens=0;
				int numAliensWithLifeLeft=0;
				int numMarines=0;
				int numMarinesWithLifeLeft=0;
				int numPredators=0;
				int numPredatorsWithLifeLeft=0;
				BOOL gameOver=FALSE;
				for(i=0;i<NET_MAXPLAYERS;i++)
				{
					if(netGameData.playerData[i].playerId)
					{	
						numPlayers++;
						numPlayersWithLifeLeft+=netGameData.playerData[i].playerHasLives;

						switch(netGameData.playerData[i].characterType)
						{
							case NGCT_Alien :
								numAliens++;
								numAliensWithLifeLeft+=netGameData.playerData[i].playerHasLives;
								break;

							case NGCT_Marine :
								numMarines++;
								numMarinesWithLifeLeft+=netGameData.playerData[i].playerHasLives;
								break;

							case NGCT_Predator :
								numPredators++;
								numPredatorsWithLifeLeft+=netGameData.playerData[i].playerHasLives;
								break;
							
							default:
								break;
						}
					}
				}
				
				//game is over if no one has life left
				if(numPlayersWithLifeLeft==0)
				{
					gameOver=TRUE;
				}

				if(netGameData.gameType==NGT_Individual)
				{
					if(numPlayersWithLifeLeft<=1 && numPlayers>1)
					{
						//if in a deathmatch game and there is only one player with life , then thats it
						gameOver=TRUE;
					}
				}

				if(netGameData.gameType==NGT_CoopDeathmatch)
				{
					/*
					In a cooperative deathmatch ,the game ends when only one team has life left
					*/
					int numTeams=0;
					int numTeamsWithLifeLeft=0;

					if(numAliens) numTeams++;
					if(numAliensWithLifeLeft) numTeamsWithLifeLeft++;
					if(numPredators) numTeams++;
					if(numPredatorsWithLifeLeft) numTeamsWithLifeLeft++;
					if(numMarines) numTeams++;
					if(numMarinesWithLifeLeft) numTeamsWithLifeLeft++;

					if(numTeams>1 && numTeamsWithLifeLeft<=1)
					{
						gameOver=TRUE;
					}
				}
				
				if(gameOver)
				{
					TransmitEndOfGameNetMsg();
					netGameData.myGameState = NGS_EndGameScreen;
				}

			}
		}
	}


	/* print my score on the screen (temporary) */
	if(netGameData.myGameState==NGS_Playing)
	{
		#if 0
		int myScore;
		int myIndex = PlayerIdInPlayerList(AVPDPNetID);
		myScore = AddUpPlayerFrags(myIndex);
		LOCALASSERT(myIndex!=NET_IDNOTINPLAYERLIST);		
		#if PreBeta
		{
			extern void	jtextprint(const char *t,...);
			jtextprint("NET GAME SCORE: %d \n", myScore);	
		}
		#else
			textprint("NET GAME SCORE: %d \n", myScore);	
		#endif

		#else
		#endif

		#if EXTRAPOLATION_TEST
		{
			extern void PlayerGhostExtrapolation();
			PlayerGhostExtrapolation();
		}
		#endif
	}

	LogNetInfo("Finished message collection post-processing \n");

	if(MultiplayerObservedPlayer)
	{
		CheckStateOfObservedPlayer();
	}

}

/*----------------------------------------------------------------------
  Functions for processing system messages
  ----------------------------------------------------------------------*/
static void ProcessSystemMessage(char *msgP,unsigned int msgSize)
{
	LPDPMSG_GENERIC systemMessage = (LPDPMSG_GENERIC)msgP;	
	
	/* currently, only the host deals with system mesages */
	/* check for invalid parameters */
	if((msgSize==0)||(msgP==NULL)) return;

	switch(systemMessage->dwType)
	{
		case DPSYS_ADDPLAYERTOGROUP:
		{
			/* ignore */
			break;
		}
		case DPSYS_CREATEPLAYERORGROUP:
		{
			/* only useful during startup: during main game, connecting player should
			detect game state and exit immediately */
			if((AvP.Network==I_Host))
			{	
				LPDPMSG_CREATEPLAYERORGROUP createMessage;
				createMessage = (LPDPMSG_CREATEPLAYERORGROUP)systemMessage;
				if(createMessage->dwPlayerType == DPPLAYERTYPE_PLAYER)
				{
					DPID id = createMessage->dpId;
					char *name = &(createMessage->dpnName.lpszShortNameA[0]);
					AddPlayerToGame(id,name);
				}
			}
			LogNetInfo("system message:  DPSYS_CREATEPLAYERORGROUP \n");
			break;
		}
		case DPSYS_DELETEPLAYERFROMGROUP:
		{
//					NewOnScreenMessage("A PLAYER HAS DISCONNECTED");
			/* ignore */
			break;
		}
		case DPSYS_DESTROYPLAYERORGROUP:
		{			
			/* Aha. Either a player has left (should have sent a leaving message)
			or s/he has exited abnormally. In either case, only need to act on
			this during start-up. During the main game, the ghosts will time-out
			anyway */
			if((AvP.Network==I_Host))
			{	
				LPDPMSG_DESTROYPLAYERORGROUP destroyMessage;
				destroyMessage = (LPDPMSG_DESTROYPLAYERORGROUP)systemMessage;
				if(destroyMessage->dwPlayerType == DPPLAYERTYPE_PLAYER)
				{
					DPID id = destroyMessage->dpId;
					RemovePlayerFromGame(id);
//					NewOnScreenMessage("A PLAYER HAS DISCONNECTED");
				}
			}
			LogNetInfo("system message:  DPSYS_DESTROYPLAYERORGROUP \n");
			break;
		}
		case DPSYS_HOST:
		{
			/* Aha... the host has died, then. This is a terminal game state,
			as the host was managing the game. Thefore, temporarily adopt host
			duties for the purpose of ending the game... 
			This is most important during the playing state, but also happens in
			startup. In startup, peers keep a host timeout which should fire before
			this message arrives anyway. */
			LOCALASSERT(AvP.Network==I_Peer);
			if((netGameData.myGameState==NGS_StartUp)||(netGameData.myGameState==NGS_Playing)||(netGameData.myGameState==NGS_Joining)||(netGameData.myGameState==NGS_EndGameScreen))
			{
				AvP.Network=I_Host;
				/* Eek, I guess the old AIs bite the dust? */
				//but the new host can create some more
				AvP.NetworkAIServer = (netGameData.gameType==NGT_Coop);
				Inform_NewHost();
//				TransmitEndOfGameNetMsg();
//				netGameData.myGameState = NGS_EndGame;
//				AvP.MainLoopRunning = 0;

				if(LobbiedGame)
				{
					//no longer a lowly client
					LobbiedGame=LobbiedGame_Server;
				}
			}
			LogNetInfo("system message:  DPSYS_HOST \n");
			break;
		}
		case DPSYS_SESSIONLOST:
		{
			/* Aha. I have lost my connection. Time to exit the game gracefully.*/
			NewOnScreenMessage("Session lost!!");
			/*
			if((netGameData.myGameState==NGS_StartUp)||(netGameData.myGameState==NGS_Joining)||(netGameData.myGameState==NGS_Playing))
			{
				netGameData.myGameState = NGS_Error_HostLost;
			}
			
			LogNetInfo("system message:  DPSYS_SESSIONLOST \n");
			*/
			break;
		}
		case DPSYS_SETPLAYERORGROUPDATA:
		{
			/* ignore */
			break;
		}
		case DPSYS_SETPLAYERORGROUPNAME:
		{
			/* ignore */
			break;
		}
		default:
		{
			/* invalid system message type: ignore */
			break;
		}
	}
}

static void AddPlayerToGame(DPID id, char* name)
{
	int freePlayerIndex;

	LOCALASSERT(AvP.Network==I_Host);			


	/* find a free slot for the player */
	freePlayerIndex = EmptySlotInPlayerList();
	if(freePlayerIndex == NET_NOEMPTYSLOTINPLAYERLIST) return;

	/* initialise the slot */
	netGameData.playerData[freePlayerIndex].playerId = id;		
	
	strncpy(netGameData.playerData[freePlayerIndex].name,name,NET_PLAYERNAMELENGTH-1);
	netGameData.playerData[freePlayerIndex].name[NET_PLAYERNAMELENGTH-1] = '\0';
//	ConvertNetNameToUpperCase(netGameData.playerData[freePlayerIndex].name);
				
	netGameData.playerData[freePlayerIndex].characterType = NGCT_Marine;
	netGameData.playerData[freePlayerIndex].characterSubType = NGSCT_General;
	netGameData.playerData[freePlayerIndex].startFlag = 0;		
	{
		int i;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			netGameData.playerData[freePlayerIndex].playerFrags[i] = 0;
		}
		netGameData.playerData[freePlayerIndex].playerScore = 0;
		netGameData.playerData[freePlayerIndex].playerScoreAgainst = 0;
		netGameData.playerData[freePlayerIndex].aliensKilled[0] = 0;
		netGameData.playerData[freePlayerIndex].aliensKilled[1] = 0;
		netGameData.playerData[freePlayerIndex].aliensKilled[2] = 0;
		netGameData.playerData[freePlayerIndex].deathsFromAI=0;
		netGameData.playerData[freePlayerIndex].playerAlive = 1;
		netGameData.playerData[freePlayerIndex].playerHasLives = 1;

		netGameData.playerData[freePlayerIndex].lastKnownPosition.vx=0;
		netGameData.playerData[freePlayerIndex].lastKnownPosition.vy=100000000;
		netGameData.playerData[freePlayerIndex].lastKnownPosition.vz=0;
	}

	Inform_PlayerHasConnected(id);
}

void RemovePlayerFromGame(DPID id)
{
	int playerIndex,j;
	
	//LOCALASSERT(AvP.Network==I_Host);			

	/* get player index from dpid */
	playerIndex = PlayerIdInPlayerList(id);
	if(playerIndex==NET_IDNOTINPLAYERLIST)
	{
		/* the player is not actually in the game, so do nothing. This might occur, for
		example if a player tries to join when the game is full */
		return;
	}

	/* free the slot */
	netGameData.playerData[playerIndex].playerId = 0;		
	for(j=0;j<NET_PLAYERNAMELENGTH;j++) netGameData.playerData[playerIndex].name[j] = '\0';
	netGameData.playerData[playerIndex].characterType = NGCT_Marine;
	netGameData.playerData[playerIndex].characterSubType = NGSCT_General;
	netGameData.playerData[playerIndex].startFlag = 0;		
	{
		int i;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			netGameData.playerData[playerIndex].playerFrags[i] = 0;
			netGameData.playerData[i].playerFrags[playerIndex] = 0;
		}
		netGameData.playerData[playerIndex].playerScore = 0;
		netGameData.playerData[playerIndex].playerScoreAgainst = 0;
		netGameData.playerData[playerIndex].aliensKilled[0] = 0;
		netGameData.playerData[playerIndex].aliensKilled[1] = 0;
		netGameData.playerData[playerIndex].aliensKilled[2] = 0;
		netGameData.playerData[playerIndex].deathsFromAI=0;
		netGameData.playerData[playerIndex].playerAlive = 1;
		netGameData.playerData[playerIndex].playerHasLives = 1;

		netGameData.playerData[playerIndex].lastKnownPosition.vx=0;
		netGameData.playerData[playerIndex].lastKnownPosition.vy=100000000;
		netGameData.playerData[playerIndex].lastKnownPosition.vz=0;
		
	}
}

/*----------------------------------------------------------------------
  Core function for processing game messages
  ----------------------------------------------------------------------*/
static void ProcessGameMessage(DPID senderId, char *msgP,unsigned int msgSize)
{
	char* subMessagePtr;
	NETMESSAGEHEADER *headerPtr;
	char *endOfMessage;
		
	LogNetInfo("Processing a game message \n");

	/* check the dp message */
	{
		/* check for invalid parameters */
		if((msgSize==0)||(msgP==NULL)) return;
		/* check for an empty message, ie a message that is only the size of
		the dpext header.  We shouldn't get any of these.  */
		if(msgSize <= DPEXT_HEADER_SIZE) return;
	}	
		
	/* some assertions about our game state */
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Leaving)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameFull)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameStarted)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_HostLost)));
		
	/* In leaving or error states, we can ignore game messages */
	if((netGameData.myGameState!=NGS_StartUp)&&(netGameData.myGameState!=NGS_Playing)&&(netGameData.myGameState!=NGS_Joining)&&(netGameData.myGameState!=NGS_EndGameScreen)) return;

	/* validate the sender from our player list, unless we're in startup mode */
//	if((netGameData.myGameState!=NGS_StartUp)&&
//	   (PlayerIdInPlayerList(senderId)==NET_IDNOTINPLAYERLIST)) return;
	
	/* the message includes garry's dp extented header, so skip past this
	and find the end of the message (for checking integrity) */
	subMessagePtr = msgP + DPEXT_HEADER_SIZE;	
	endOfMessage = (char *)(subMessagePtr + (msgSize - DPEXT_HEADER_SIZE));	

	/* Read through to the end of the message... */
	while(subMessagePtr<endOfMessage)
	{		
		headerPtr = (NETMESSAGEHEADER *)subMessagePtr;
		subMessagePtr += sizeof(NETMESSAGEHEADER);		

		switch(headerPtr->type)
		{
			case(NetMT_GameDescription):
			{
				ProcessNetMsg_GameDescription((NETMESSAGE_GAMEDESCRIPTION *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_GAMEDESCRIPTION);
				break;
			}
			case(NetMT_PlayerDescription):
			{
				ProcessNetMsg_PlayerDescription((NETMESSAGE_PLAYERDESCRIPTION *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERDESCRIPTION);
				break;
			}
			case(NetMT_StartGame):
			{
				ProcessNetMsg_StartGame();
				break;
			}
			case(NetMT_PlayerState):
			{
				ProcessNetMsg_PlayerState((NETMESSAGE_PLAYERSTATE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERSTATE);
				break;
			}
			case(NetMT_PlayerState_Minimal):
			{
				ProcessNetMsg_PlayerState_Minimal((NETMESSAGE_PLAYERSTATE_MINIMAL *)subMessagePtr, senderId,FALSE);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERSTATE_MINIMAL);
				break;
			}
			case(NetMT_PlayerState_Medium):
			{
				ProcessNetMsg_PlayerState_Minimal((NETMESSAGE_PLAYERSTATE_MINIMAL *)subMessagePtr, senderId,TRUE);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERSTATE_MEDIUM);
				break;
			}
			case (NetMT_FrameTimer) :
			{
				ProcessNetMsg_FrameTimer(((NETMESSAGE_FRAMETIMER *)subMessagePtr)->frame_time, senderId);
				subMessagePtr += sizeof(NETMESSAGE_FRAMETIMER);
				break;
			}
			case(NetMT_PlayerKilled):
			{
				ProcessNetMsg_PlayerKilled((NETMESSAGE_PLAYERKILLED *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERKILLED);
				break;
			}
			case(NetMT_CorpseDeathAnim):
			{
				ProcessNetMsg_PlayerDeathAnim((NETMESSAGE_CORPSEDEATHANIM *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_CORPSEDEATHANIM);
				break;
			}
			case(NetMT_PlayerLeaving):
			{
				ProcessNetMsg_PlayerLeaving(senderId);
				break;
			}
			case(NetMT_AllGameScores):
			{
				ProcessNetMsg_AllGameScores((NETMESSAGE_ALLGAMESCORES *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_ALLGAMESCORES);
				break;
			}
			case(NetMT_PlayerScores):
			{
				ProcessNetMsg_PlayerScores((NETMESSAGE_PLAYERSCORES *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERSCORES);
				break;
			}
			case(NetMT_LocalRicochet):
			{
				ProcessNetMsg_LocalRicochet((NETMESSAGE_LOCALRICOCHET *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_LOCALRICOCHET);
				break;
			}
			case(NetMT_LocalObjectState):
			{
				ProcessNetMsg_LocalObjectState((NETMESSAGE_LOBSTATE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_LOBSTATE);
				break;
			}
			case(NetMT_LocalObjectDamaged):
			{
				ProcessNetMsg_LocalObjectDamaged((char*)subMessagePtr, senderId);
				subMessagePtr += GetSizeOfLocalObjectDamagedMessage((char*)subMessagePtr);
				break;
			}
			case(NetMT_LocalObjectDestroyed):
			{
				ProcessNetMsg_LocalObjectDestroyed((NETMESSAGE_LOBDESTROYED *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_LOBDESTROYED);
				break;
			}
			case(NetMT_ObjectPickedUp):
			{
				ProcessNetMsg_ObjectPickedUp((NETMESSAGE_OBJECTPICKEDUP *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_OBJECTPICKEDUP);
				break;
			}
			case(NetMT_EndGame):
			{
				ProcessNetMsg_EndGame();
				break;
			}
			case(NetMT_InanimateObjectDamaged):
			{
				ProcessNetMsg_InanimateObjectDamaged((char *)subMessagePtr);
				subMessagePtr += GetSizeOfInanimateDamagedMessage((char *)subMessagePtr);
				break;
			}
			case(NetMT_InanimateObjectDestroyed):
			{
				ProcessNetMsg_InanimateObjectDestroyed((NETMESSAGE_INANIMATEDESTROYED *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_INANIMATEDESTROYED);
				break;
			}
			case(NetMT_LOSRequestBinarySwitch):
			{
				ProcessNetMsg_LOSRequestBinarySwitch((NETMESSAGE_LOSREQUESTBINARYSWITCH *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_LOSREQUESTBINARYSWITCH);
				break;
			}
			case(NetMT_PlatformLiftState):
			{
				ProcessNetMsg_PlatformLiftState((NETMESSAGE_PLATFORMLIFTSTATE *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_PLATFORMLIFTSTATE);
				break;
			}
			case(NetMT_RequestPlatformLiftActivate):
			{
				ProcessNetMsg_RequestPlatformLiftActivate((NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE);
				break;
			}
			case(NetMT_PlayerAutoGunState):
			{
				ProcessNetMsg_PlayerAutoGunState((NETMESSAGE_AGUNSTATE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_AGUNSTATE);
				break;
			}
			case(NetMT_MakeDecal):
			{
				ProcessNetMsg_MakeDecal((NETMESSAGE_MAKEDECAL *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_MAKEDECAL);
				break;
			}
			case(NetMT_ChatBroadcast):
			{
				subMessagePtr = ProcessNetMsg_ChatBroadcast(subMessagePtr, senderId);
				break;
			}
			case(NetMT_MakeExplosion):
			{
				ProcessNetMsg_MakeExplosion((NETMESSAGE_MAKEEXPLOSION *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_MAKEEXPLOSION);
				break;
			}
			case(NetMT_MakeFlechetteExplosion):
			{
				ProcessNetMsg_MakeFlechetteExplosion((NETMESSAGE_MAKEFLECHETTEEXPLOSION *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_MAKEFLECHETTEEXPLOSION);
				break;
			}
			case(NetMT_MakePlasmaExplosion):
			{
				ProcessNetMsg_MakePlasmaExplosion((NETMESSAGE_MAKEPLASMAEXPLOSION *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_MAKEPLASMAEXPLOSION);
				break;
			}
			case(NetMT_PredatorSights):
			{
				ProcessNetMsg_PredatorSights((NETMESSAGE_PREDATORSIGHTS *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_PREDATORSIGHTS);
				break;
			}
			case(NetMT_LocalObjectOnFire):
			{
				ProcessNetMsg_LocalObjectOnFire((NETMESSAGE_LOBONFIRE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_LOBONFIRE);
				break;
			}
			case(NetMT_RestartNetworkGame):
			{
				RestartNetworkGame(((NETMESSAGE_RESTARTGAME*)subMessagePtr)->seed);
				subMessagePtr += sizeof(NETMESSAGE_RESTARTGAME);
				break;
			}
			case(NetMT_FragmentalObjectsStatus):
			{
				ProcessNetMsg_FragmentalObjectsStatus((NETMESSAGE_FRAGMENTALOBJECTSSTATUS *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_FRAGMENTALOBJECTSSTATUS);
				break;
			}
			case(NetMT_StrategySynch):
			{
				ProcessNetMsg_StrategySynch((NETMESSAGE_STRATEGYSYNCH *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_STRATEGYSYNCH);
				break;
			}
			case(NetMT_AlienAIState):
			{
				ProcessNetMsg_AlienAIState((NETMESSAGE_ALIENAISTATE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_ALIENAISTATE);
				break;
			}
			case(NetMT_AlienAISequenceChange):
			{
				ProcessNetMsg_AlienAISequenceChange((NETMESSAGE_ALIENSEQUENCECHANGE *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_ALIENSEQUENCECHANGE);
				break;
			}
			case(NetMT_AlienAIKilled):
			{
				ProcessNetMsg_AlienAIKilled((NETMESSAGE_ALIENAIKILLED *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_ALIENAIKILLED);
				break;
			}
			case(NetMT_FarAlienPosition):
			{
				ProcessNetMsg_FarAlienPosition((NETMESSAGE_FARALIENPOSITION *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_FARALIENPOSITION);
				break;
			}
			case(NetMT_GhostHierarchyDamaged):
			{
				ProcessNetMsg_GhostHierarchyDamaged((char *)subMessagePtr, senderId);
				subMessagePtr += GetSizeOfGhostHierarchyDamagedMessage((char *)subMessagePtr) ;
				break;
			}
			case(NetMT_Gibbing):
			{
				ProcessNetMsg_Gibbing((NETMESSAGE_GIBBING *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_GIBBING);
				break;
			}
			case(NetMT_SpotAlienSound):
			{
				ProcessNetMsg_SpotAlienSound((NETMESSAGE_SPOTALIENSOUND *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_SPOTALIENSOUND);
				break;
			}
			case(NetMT_SpotOtherSound):
			{
				ProcessNetMsg_SpotOtherSound((NETMESSAGE_SPOTOTHERSOUND *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_SPOTOTHERSOUND);
				break;
			}
			case(NetMT_LocalObjectDestroyed_Request):
			{
				ProcessNetMsg_LocalObjectDestroyed_Request((NETMESSAGE_LOBDESTROYED_REQUEST *)subMessagePtr, senderId);
				subMessagePtr += sizeof(NETMESSAGE_LOBDESTROYED_REQUEST);
				break;
			}

			case(NetMT_LastManStanding_Restart):
			{
				Handle_LastManStanding_Restart(((NETMESSAGE_LMS_RESTART*)subMessagePtr)->playerID,((NETMESSAGE_LMS_RESTART*)subMessagePtr)->seed);
				subMessagePtr += sizeof(NETMESSAGE_LMS_RESTART);
				break;
			}
			
			case(NetMT_LastManStanding_RestartInfo):
			{
				Handle_LastManStanding_RestartInfo(((NETMESSAGE_PLAYERID*)subMessagePtr)->playerID);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERID);
				break;
			}

			case(NetMT_LastManStanding_LastMan):
			{
				Handle_LastManStanding_LastMan(((NETMESSAGE_PLAYERID*)subMessagePtr)->playerID);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERID);
				break;
			}
			
			case(NetMT_LastManStanding_RestartCountDown):
			{
				Handle_LastManStanding_RestartTimer(((NETMESSAGE_LMS_RESTARTTIMER*)subMessagePtr)->timer);
				subMessagePtr += sizeof(NETMESSAGE_LMS_RESTARTTIMER);
				break;
			}

			case(NetMT_PredatorTag_NewPredator):
			{
				Handle_SpeciesTag_NewPersonIt(((NETMESSAGE_PLAYERID*)subMessagePtr)->playerID);
				subMessagePtr += sizeof(NETMESSAGE_PLAYERID);
				break;
			}

			case(NetMT_CreateWeapon):
			{
				ProcessNetMsg_CreateWeapon((NETMESSAGE_CREATEWEAPON*)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_CREATEWEAPON);
				break;
			}
			case(NetMT_RespawnPickups):
			{
				if(netGameData.myGameState==NGS_Playing || 
				   netGameData.myGameState==NGS_EndGameScreen)
				{
					RespawnAllPickups();
					PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_WEAPON_RESPAWN);
				}
				break;
			}
			
			case(NetMT_ScoreChange):
			{
				ProcessNetMsg_ScoreChange((NETMESSAGE_SCORECHANGE *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_SCORECHANGE);
				break;
			}
			 
			case(NetMT_SpeciesScores):
			{
				ProcessNetMsg_SpeciesScores((NETMESSAGE_SPECIESSCORES *)subMessagePtr);
				subMessagePtr += sizeof(NETMESSAGE_SPECIESSCORES);
				break;
			}
			
			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}
	}
	/* if we have read the message correctly, the message pointer should be exactly
	at the end of the message buffer*/
	LOCALASSERT(subMessagePtr==endOfMessage);

	LogNetInfo("Finished processing a game message \n");

}

#if CalculateBytesSentPerSecond
int GetBytesPerSecond(int bytesThisFrame)
{
	static int times[100];
	static int bytes[100];
	static int next_index;
	int i;
	int totalBytes=0;

	times[next_index]=0;
	bytes[next_index]=bytesThisFrame;
	next_index=(next_index+1)%100;

	for(i=0;i<100;i++)
	{
		times[i]+=NormalFrameTime;
		totalBytes+=bytes[i];
	}

	if(!times[next_index]) return 0;
	
	return(DIV_FIXED(totalBytes,times[next_index]));

}
#endif

/*----------------------------------------------------------------------
  Core function for sending messages
  ----------------------------------------------------------------------*/
void NetSendMessages(void)
{
	/* some assertions about our game state */
	//LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Leaving)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameFull)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_GameStarted)));
	LOCALASSERT(!((AvP.Network==I_Host)&&(netGameData.myGameState==NGS_Error_HostLost)));

	
	/* only bother sending messages under certain game conditions... */
	if((netGameData.myGameState!=NGS_StartUp)&&(netGameData.myGameState!=NGS_EndGameScreen)&&(netGameData.myGameState!=NGS_Playing)&&(netGameData.myGameState!=NGS_Joining)) return;

	LogNetInfo("Sending net messages... \n");

	/* at this point, add player and other object updates... doing this here
	ensures we are sending our most upto date info */
	if(netGameData.myGameState==NGS_Playing ||
	   netGameData.myGameState==NGS_EndGameScreen)
	{
		netGameData.gameDescriptionTimeDelay-=RealFrameTime;

		GameTimeSinceLastSend+=NormalFrameTime;
		TimeCounterForExtrapolation+=NormalFrameTime;

		#if EXTRAPOLATION_TEST
		//update muzzle flashes here , since this happens after dynamics
		{
			extern void PostDynamicsExtrapolationUpdate();
			PostDynamicsExtrapolationUpdate();
		}
		#endif
		
		if(netGameData.sendFrequency)
		{
			//check to see if messages should be sent this frame
			netGameData.sendTimer-=RealFrameTime;
			if(netGameData.sendTimer>0)
			{
				//don't send messages this frame
				#if CalculateBytesSentPerSecond
				PrintDebuggingText("Bytes/Second: %d\n",GetBytesPerSecond(0));
				#endif
				return;
			}
			netGameData.sendTimer+=netGameData.sendFrequency;
			netGameData.sendTimer=max(0,netGameData.sendTimer);
				
		}
			
		if(netGameData.myGameState==NGS_EndGameScreen)
		{
			if(AvP.Network==I_Host)
			{
				PeriodicScoreUpdate();
				//send game description once per second
				if(netGameData.gameDescriptionTimeDelay<0)
				{
					netGameData.gameDescriptionTimeDelay+=2*ONE_FIXED;
					AddNetMsg_GameDescription();

					AddNetMsg_StrategySynch();
	
					AddNetMsg_FragmentalObjectsStatus();
				}
			}
		}
		else //game state is NGS_Playing
		{
			AddPlayerAndObjectUpdateMessages();
			if(AvP.Network==I_Host)
			{
				//send game description once per second
				if(netGameData.gameDescriptionTimeDelay<0)
				{
					netGameData.gameDescriptionTimeDelay+=ONE_FIXED;
					AddNetMsg_GameDescription();

					AddNetMsg_StrategySynch();
	
					AddNetMsg_FragmentalObjectsStatus();
				}
							
				PeriodicScoreUpdate();

				if(netGameData.gameType==NGT_LastManStanding)
				{
					CheckLastManStandingState();
				}
				else if(netGameData.gameType==NGT_PredatorTag || netGameData.gameType==NGT_AlienTag)
				{
					CheckSpeciesTagState();
				}
				CheckForPointBasedObjectRespawn();
			}
		}
	}
	else if(netGameData.myGameState==NGS_Leaving || 
			netGameData.myGameState==NGS_Joining || 
			netGameData.myGameState==NGS_StartUp)
	{
		if(AvP.Network==I_Host)
		{
			AddNetMsg_GameDescription();
		}
	}
	
	{
		/* send our message buffer...
		NB it should always be non-empty, and always less than the maximum message size */
		HRESULT res;
		int numBytes;
		BOOL clearSendBuffer=TRUE;
		numBytes = (int)(endSendBuffer - &sendBuffer[0]);

		if(netGameData.myGameState==NGS_EndGameScreen || netGameData.myGameState==NGS_Joining)
		{
			//there may not be any messages while showing the end game screen
			if(numBytes==DPEXT_HEADER_SIZE) return;
		}
		
		
		LOCALASSERT(numBytes > DPEXT_HEADER_SIZE);			
		LOCALASSERT(numBytes <= NET_MESSAGEBUFFERSIZE);
		if(!netGameData.skirmishMode)
		{
			if(glpDP && AVPDPNetID)
			{
				res = DpExtSend(glpDP,AVPDPNetID,DPID_ALLPLAYERS,0,&sendBuffer,numBytes);
				if(res!=DP_OK)
				{
					//we have some problem sending...
					switch(res)
					{
						case DPERR_BUSY :
							/*
							failed to send this frame , try preserving the contents of the send buffer ,
							unless it is getting to full.
							*/
							if(numBytes<NET_MESSAGEBUFFERSIZE/2)
							{
								clearSendBuffer=FALSE;
							}
							break; 
						
						case DPERR_CONNECTIONLOST :
							NewOnScreenMessage("Connection lost!!");
							break;
						
						case DPERR_INVALIDPARAMS  :
							LOCALASSERT(0=="Send - Invalid parameters");
							break;
						
						case DPERR_INVALIDPLAYER : 
							LOCALASSERT(0=="Send - Invalid player");
							break;
						
						case DPERR_NOTLOGGEDIN : 
							LOCALASSERT(0=="Send - Not logged in");
							break;
						
						case DPERR_SENDTOOBIG : 
							LOCALASSERT(0=="Send - Send to big");
							break;

						default :
							LOCALASSERT(0=="Send - Unknown error");
							break;

									
					}

				}
				
			}
		}
		numMessagesTransmitted++;
		/* re-initialise the send message buffer */
		/*(unless the send failed because it was to busy)*/
		if(clearSendBuffer)
		{
			InitialiseSendMessageBuffer();
		}

//		PrintDebuggingText("Bytes: %d\n",numBytes);
		#if CalculateBytesSentPerSecond
		PrintDebuggingText("Bytes/Second: %d\n",GetBytesPerSecond(numBytes));
		#endif
		
	}
	
	LogNetInfo("...Finished sending net message \n");
}

static void InitialiseSendMessageBuffer(void)
{
	/* NB the receive buffer is maintained internally to garry's dp-extension */
	/* reset the end of send buffer pointer */
	endSendBuffer = &sendBuffer[0];
	/* add space for a dpext header to the send buffer */
	endSendBuffer += DPEXT_HEADER_SIZE;
}


/*----------------------------------------------------------------------
this function examines the sb-list and adds messages for the player, local
objects, and other such things. NB It would be possible to do this in the
behaviour functions, but would involve a lag
  ----------------------------------------------------------------------*/
static void AddPlayerAndObjectUpdateMessages(void)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;

	/* don't bother adding these if we're finishing 
	(host gets here even if finishing, but doesn't need to send these messages) */
	if(netGameData.myGameState!=NGS_Playing) return;	

	/* NB IF THE LIST OF OBJECT TYPES WHICH ARE UPDATED CHANGES, THERE MUST BE A 
	CORESPONDING CHANGE IN THE LIST OF OBJECTS TESTED BY FindObjectFromNetIndex(),
	ELSE PLAYER WILL NOT BE ABLE TO RECEIVE MESSAGES ABOUT OWN OBJECTS OF THAT TYPE! */	

	#if EXTRAPOLATION_TEST
	AddNetMsg_FrameTimer();
	#endif

	while(sbIndex < NumActiveStBlocks)
	{	
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[sbIndex++];
		switch(sbPtr->I_SBtype)
		{
			case(I_BehaviourMarinePlayer):
			case(I_BehaviourAlienPlayer):
			case(I_BehaviourPredatorPlayer):
			{
				/* the player update message */
				AddNetMsg_PlayerState(sbPtr);
				break;
			}
			case(I_BehaviourRocket):
			case(I_BehaviourGrenade):
			case(I_BehaviourProximityGrenade):
			case(I_BehaviourPulseGrenade):
			case(I_BehaviourFragmentationGrenade):
			case(I_BehaviourPredatorEnergyBolt):
			case(I_BehaviourFrisbeeEnergyBolt):
			case(I_BehaviourClusterGrenade):
			case(I_BehaviourNPCPredatorDisc):
			case(I_BehaviourPredatorDisc_SeekTrack):
			case(I_BehaviourAlienSpit):
			case(I_BehaviourNetCorpse):
			case(I_BehaviourPPPlasmaBolt):
			case(I_BehaviourFrisbee):
//			case(I_BehaviourSpeargunBolt): //spear location is sent once , upon creation
			{
				
 				AddNetMsg_LocalObjectState(sbPtr);
				break;
			}
			case(I_BehaviourFlareGrenade):
			{
			    FLARE_BEHAV_BLOCK *bbPtr = (FLARE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
				DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
				//only send messages for flares while they are moving
				if(!dynPtr->IsStatic || bbPtr->becomeStuck) 
				{ 
					AddNetMsg_LocalObjectState(sbPtr);
				}
				break;
			}
			case(I_BehaviourInanimateObject):
			{
				INANIMATEOBJECT_STATUSBLOCK* objStatPtr = sbPtr->SBdataptr;

				if (objStatPtr->ghosted_object) {
					AddNetMsg_LocalObjectState(sbPtr);
				}
				break;
			}
			case(I_BehaviourAutoGun):
			{
				/* this MUST be a player placed autogun */
				{
				}
				/* the autogun update message */
				AddNetMsg_PlayerAutoGunState(sbPtr);
				break;
			}
			case(I_BehaviourAlien):
			{
				//only add alien details , if it is near (to someone)
				if(sbPtr->SBdptr)
				{
					AddNetMsg_AlienAIState(sbPtr);
				}
				break;
			}

			default:
				break;
		}					
	}
}



/*----------------------------------------------------------------------
  End of network game clean up
  ----------------------------------------------------------------------*/
void EndAVPNetGame(void)
{
#if 0
	HRESULT hres;
#endif

	/* garry's dp extended clean up */
	if(!netGameData.skirmishMode)
	{
		DpExtUnInit();	
	}
	

	//netGameData.myGameState=NGS_Leaving;
	RemovePlayerFromGame(AVPDPNetID);
	TransmitPlayerLeavingNetMsg();
	
	if(!netGameData.skirmishMode)
	{
		DirectPlay_Disconnect();
	}

	#if 0
	/* terminate our player */
	if(AVPDPNetID) 
	{
		hres = IDirectPlay4_DestroyPlayer(glpDP, AVPDPNetID);
		AVPDPNetID = NULL;
	}
	/* terminate the dp object */
	if(glpDP) 
	{
		hres = IDirectPlay4_Close(lpDPlay3AAVP);
		IDirectPlay4_Release(lpDPlay3AAVP);
		lpDPlay3AAVP = NULL;
	}
	#endif
	
	/* reset our game mode here */
	AvP.Network = I_No_Network;	
	AvP.NetworkAIServer = 0;
	netGameData.skirmishMode=FALSE;

	TurnOffMultiplayerObserveMode();

	//reset time scale to default value
	TimeScale=ONE_FIXED;
}

/*----------------------------------------------------------------------
  Functions for adding each message type to the send message buffer
  ----------------------------------------------------------------------*/
void AddNetMsg_GameDescription(void)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_GAMEDESCRIPTION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_GAMEDESCRIPTION);

	/* some conditions */
	LOCALASSERT(AvP.Network==I_Host);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_GAMEDESCRIPTION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_GameDescription;
	
	/*fill out the message */
	{ 	int i;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			messagePtr->players[i].playerId = netGameData.playerData[i].playerId;
			messagePtr->players[i].characterType = (unsigned char)netGameData.playerData[i].characterType;
			messagePtr->players[i].characterSubType = (unsigned char)netGameData.playerData[i].characterSubType;
			messagePtr->players[i].startFlag = netGameData.playerData[i].startFlag;
		}
		messagePtr->gameType = (unsigned char)netGameData.gameType;
		messagePtr->levelNumber = (unsigned char)netGameData.levelNumber;
		messagePtr->scoreLimit = netGameData.scoreLimit;
		messagePtr->timeLimit = (unsigned char)netGameData.timeLimit;
		messagePtr->invulnerableTime = (unsigned char)netGameData.invulnerableTime;

		for(i=0;i<3;i++)
		{
			messagePtr->characterKillValues[i]=(unsigned char)netGameData.characterKillValues[i];
			messagePtr->aiKillValues[i]=(unsigned char)netGameData.aiKillValues[i];
		}
		messagePtr->baseKillValue=(unsigned char)netGameData.baseKillValue;
		messagePtr->useDynamicScoring=(unsigned char)netGameData.useDynamicScoring;
		messagePtr->useCharacterKillValues=(unsigned char)netGameData.useCharacterKillValues;

		
		
		messagePtr->sendDecals=netGameData.sendDecals;

	
		messagePtr->allowSmartgun=netGameData.allowSmartgun;
		messagePtr->allowFlamer=netGameData.allowFlamer;
		messagePtr->allowSadar=netGameData.allowSadar;
		messagePtr->allowGrenadeLauncher=netGameData.allowGrenadeLauncher;
		messagePtr->allowMinigun=netGameData.allowMinigun;
		messagePtr->allowDisc=netGameData.allowDisc;
		messagePtr->allowPistol=netGameData.allowPistol;
		messagePtr->allowPlasmaCaster=netGameData.allowPlasmaCaster;
		messagePtr->allowSpeargun=netGameData.allowSpeargun;
		messagePtr->allowMedicomp=netGameData.allowMedicomp;
		messagePtr->allowSmartDisc=netGameData.allowSmartDisc;
		messagePtr->allowPistols=netGameData.allowPistols;
		
		messagePtr->maxPredator=netGameData.maxPredator;
		messagePtr->maxAlien=netGameData.maxAlien;
		messagePtr->maxMarine=netGameData.maxMarine;

		messagePtr->maxMarineGeneral=netGameData.maxMarineGeneral;
		messagePtr->maxMarinePulseRifle=netGameData.maxMarinePulseRifle;
		messagePtr->maxMarineSmartgun=netGameData.maxMarineSmartgun;
		messagePtr->maxMarineFlamer=netGameData.maxMarineFlamer;
		messagePtr->maxMarineSadar=netGameData.maxMarineSadar;
		messagePtr->maxMarineGrenade=netGameData.maxMarineGrenade;
		messagePtr->maxMarineMinigun=netGameData.maxMarineMinigun;
		messagePtr->maxMarineSmartDisc=netGameData.maxMarineSmartDisc;
		messagePtr->maxMarinePistols=netGameData.maxMarinePistols;
		
	
		messagePtr->useSharedLives=netGameData.useSharedLives;
		messagePtr->maxLives=netGameData.maxLives;
		messagePtr->numDeaths[0]=(unsigned char) min(netGameData.numDeaths[0],255);
		messagePtr->numDeaths[1]=(unsigned char) min(netGameData.numDeaths[1],255);
		messagePtr->numDeaths[2]=(unsigned char) min(netGameData.numDeaths[2],255);

		messagePtr->timeForRespawn=(unsigned char)netGameData.timeForRespawn;
		messagePtr->pointsForRespawn=netGameData.pointsForRespawn;

		messagePtr->GameTimeElapsed=netGameData.GameTimeElapsed>>16;

		messagePtr->gameSpeed=netGameData.gameSpeed;

		messagePtr->preDestroyLights=netGameData.preDestroyLights;
		messagePtr->disableFriendlyFire=netGameData.disableFriendlyFire;
		messagePtr->fallingDamage=netGameData.fallingDamage;
		messagePtr->pistolInfiniteAmmo=netGameData.pistolInfiniteAmmo;
		messagePtr->specialistPistols=netGameData.specialistPistols;

		if(netGameData.myGameState==NGS_EndGameScreen)
			messagePtr->endGame=1;
		else
			messagePtr->endGame=0;
		
	}
}

void AddNetMsg_PlayerDescription(void)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERDESCRIPTION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERDESCRIPTION);

	/* some conditions */
	LOCALASSERT(AvP.Network==I_Peer);
	LOCALASSERT(netGameData.myGameState==NGS_Joining);
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERDESCRIPTION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerDescription;
	
	/*fill out the message */
	{
		messagePtr->characterType = (unsigned char)netGameData.myCharacterType;	
		messagePtr->characterSubType = (unsigned char)netGameData.myCharacterSubType;	
		messagePtr->startFlag = (unsigned char)netGameData.myStartFlag;		
	}
}

void AddNetMsg_StartGame(void)
{
	NETMESSAGEHEADER *headerPtr;
	int headerSize = sizeof(NETMESSAGEHEADER);

	/* some conditions */
	LOCALASSERT(AvP.Network==I_Host);
	LOCALASSERT(netGameData.myGameState==NGS_Joining);
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_StartGame;	
}

extern int UseExtrapolation;
void AddNetMsg_PlayerState(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERSTATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERSTATE);
	int playerIndex;


	if(netGameData.myGameState!=NGS_Playing) return;

	#if EXTRAPOLATION_TEST
	if(UseExtrapolation && netGameData.sendFrequency)
	{
		//see if we can get away with sending reduced information about the player's state
		static VECTORCH previousVelocity;
		static EULER previousOrient;
		static int previousWeapon;
		static unsigned int TimeOfLastPlayerState;
		BOOL sendMinimalState=TRUE;

		if(TimeCounterForExtrapolation<TimeOfLastPlayerState ||
		   TimeCounterForExtrapolation-TimeOfLastPlayerState>ONE_FIXED/4)
		{
			//It has been over 1/4 second since the last full update , so better do a full update now
			sendMinimalState=FALSE;
		}
		
		if(sendMinimalState)
		{
			if(sbPtr->DynPtr->LinImpulse.vx ||
			   sbPtr->DynPtr->LinImpulse.vy ||
			   sbPtr->DynPtr->LinImpulse.vz)
			{
				/*
				The player is probably jumping. This screws up the extrapolation somewhat.
				Therefore better send full player update.
				*/
				sendMinimalState=FALSE;
			}	
				
		}
		
		if(sendMinimalState)
		{
			VECTORCH diff=sbPtr->DynPtr->LinVelocity;
			SubVector(&previousVelocity,&diff);

			if(Approximate3dMagnitude(&diff)>100)
			{
				//the player's velocity has changed , so need a full update.
				sendMinimalState=FALSE;
			}
		}

		
		if(sendMinimalState)
		{
			PLAYER_WEAPON_DATA *weaponPtr;
 			PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
			LOCALASSERT(playerStatusPtr);    	        
    		weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

			if(weaponPtr->WeaponIDNumber!=previousWeapon)
			{
				//the player's weapon has changed ,so need a full update
				previousWeapon=weaponPtr->WeaponIDNumber;
				sendMinimalState=FALSE;
			}
		}

		if(sendMinimalState)
		{
			//don't need to send positional information , but has the player's orientation changed?
			BOOL sendOrient=FALSE;
			if(previousOrient.EulerX!=sbPtr->DynPtr->OrientEuler.EulerX ||
			   previousOrient.EulerY!=sbPtr->DynPtr->OrientEuler.EulerY ||
			   previousOrient.EulerZ!=sbPtr->DynPtr->OrientEuler.EulerZ)
			{
				previousOrient=sbPtr->DynPtr->OrientEuler;
				//we better send the medium sized player state message
				sendOrient=TRUE;
			}
			AddNetMsg_PlayerState_Minimal(sbPtr,sendOrient);
			return;
		}
		else
		{
			previousVelocity=sbPtr->DynPtr->LinVelocity;
			previousOrient=sbPtr->DynPtr->OrientEuler;
			TimeOfLastPlayerState=TimeCounterForExtrapolation;
		
		}
	}
	#endif
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERSTATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerState;

	//check our carrent character type
	{
		switch(AvP.PlayerType)
		{
			case I_Marine :
				netGameData.myCharacterType=NGCT_Marine;
				break;

			case I_Predator :
				netGameData.myCharacterType=NGCT_Predator;
				break;

			case I_Alien :
				netGameData.myCharacterType=NGCT_Alien;
				break;
		}
		
		playerIndex = PlayerIdInPlayerList(AVPDPNetID);
		GLOBALASSERT(playerIndex!=NET_IDNOTINPLAYERLIST);

		messagePtr->characterType=netGameData.myCharacterType;
		messagePtr->characterSubType=netGameData.myCharacterSubType;
		messagePtr->nextCharacterType=netGameData.myNextCharacterType;
		netGameData.playerData[playerIndex].characterType=messagePtr->nextCharacterType;

	}
	
	/* fill out our position and orientation */
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

				
		LOCALASSERT(dynPtr);
		LOCALASSERT((dynPtr->OrientEuler.EulerX >=0 )&&(dynPtr->OrientEuler.EulerX < 4096));	/* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerY >=0 )&&(dynPtr->OrientEuler.EulerY < 4096));	/* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerZ >=0 )&&(dynPtr->OrientEuler.EulerZ < 4096));	/* 9 bits of signed data */

		/* NB we can fit +-4194303 into 23 bits */
		if(dynPtr->Position.vx < -4100000) messagePtr->xPos = -4100000;
		else if(dynPtr->Position.vx > 4100000) messagePtr->xPos = 4100000;
		else messagePtr->xPos = dynPtr->Position.vx;
		messagePtr->xOrient = (dynPtr->OrientEuler.EulerX>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vy < -4100000) messagePtr->yPos = -4100000;
		else if(dynPtr->Position.vy > 4100000) messagePtr->yPos = 4100000;
		else messagePtr->yPos = dynPtr->Position.vy;
		messagePtr->yOrient = (dynPtr->OrientEuler.EulerY>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vz < -4100000) messagePtr->zPos = -4100000;
		else if(dynPtr->Position.vz > 4100000) messagePtr->zPos = 4100000;
		else messagePtr->zPos = dynPtr->Position.vz;
		messagePtr->zOrient = (dynPtr->OrientEuler.EulerZ>>NET_EULERSCALESHIFT);
		 
	
		#if EXTRAPOLATION_TEST
		messagePtr->velocity_x=dynPtr->LinVelocity.vx/100;
		messagePtr->velocity_y=dynPtr->LinVelocity.vy/100;
		messagePtr->velocity_z=dynPtr->LinVelocity.vz/100;
		messagePtr->standard_gravity=dynPtr->UseStandardGravity;
		#endif
	}

	/* KJL 17:04:22 26/01/98 - elevation (for weapon, etc.) */
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		messagePtr->Elevation = playerStatusPtr->ViewPanX;

		switch(AvP.PlayerType)
		{
			case(I_Marine):
			{
				messagePtr->sequence = (unsigned char)GetMyMarineSequence();
				/* Taunt handling. */
				if (playerStatusPtr->tauntTimer==-1) {
					if (messagePtr->sequence!=MSQ_Taunt) {
						/* Taunt overridden. */
						playerStatusPtr->tauntTimer=0;
					} else {
						/* Taunt is go. */
						playerStatusPtr->tauntTimer=TAUNT_LENGTH;
					}
				} else if (playerStatusPtr->tauntTimer!=0) {
					if (messagePtr->sequence!=MSQ_Taunt) {
						/* Taunt cancelled. */
						playerStatusPtr->tauntTimer=0;
					}
				}
				break;
			}
			case(I_Predator):
			{
				messagePtr->sequence = (unsigned char)GetMyPredatorSequence();
				/* Taunt handling. */
				if (playerStatusPtr->tauntTimer==-1) {
					if (messagePtr->sequence!=PredSQ_Taunt) {
						/* Taunt overridden. */
						playerStatusPtr->tauntTimer=0;
					} else {
						/* Taunt is go. */
						playerStatusPtr->tauntTimer=TAUNT_LENGTH;
					}
				} else if (playerStatusPtr->tauntTimer!=0) {
					if (messagePtr->sequence!=PredSQ_Taunt) {
						/* Taunt cancelled. */
						playerStatusPtr->tauntTimer=0;
					}
				}
				break;
			}
			case(I_Alien):
			{
				messagePtr->sequence = (unsigned char)GetMyAlienSequence();
				/* Taunt handling. */
				if (playerStatusPtr->tauntTimer==-1) {
					if (messagePtr->sequence!=ASQ_Taunt) {
						/* Taunt overridden. */
						playerStatusPtr->tauntTimer=0;
					} else {
						/* Taunt is go. */
						playerStatusPtr->tauntTimer=TAUNT_LENGTH;
					}
				} else if (playerStatusPtr->tauntTimer!=0) {
					if (messagePtr->sequence!=ASQ_Taunt) {
						/* Taunt cancelled. */
						playerStatusPtr->tauntTimer=0;
					}
				}
				break;
			}
			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}
		if(PlayerStatusPtr->ShapeState!=PMph_Standing) 
		{
			messagePtr->IAmCrouched = 1;
		}
		else 
		{
			messagePtr->IAmCrouched = 0;
		}
	}

	/* my current weapon id, and whether I am firing it... */
	{
		PLAYER_WEAPON_DATA *weaponPtr;
 		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);    	        
    	weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
		messagePtr->currentWeapon = (signed char)(weaponPtr->WeaponIDNumber);	

		if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
			if(((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY))
				&&(PlayerStatusPtr->IsAlive)) {
				if (LastHand) {
					messagePtr->IAmFiringPrimary = 0;
					messagePtr->IAmFiringSecondary = 1;
				} else {
					messagePtr->IAmFiringPrimary = 1;
					messagePtr->IAmFiringSecondary = 0;
				}				
			} else {
				messagePtr->IAmFiringPrimary = 0;
				messagePtr->IAmFiringSecondary = 0;
			}
			/* Whether in tertiary fire */
			if (AreTwoPistolsInTertiaryFire()) {
				messagePtr->Special=1;
			} else {
				messagePtr->Special=0;
			}
			/* whether or not I'm displaying a gun flash */
			messagePtr->IHaveAMuzzleFlash = MyPlayerHasAMuzzleFlash(sbPtr);
		} else {

			if ((weaponPtr->WeaponIDNumber==WEAPON_MARINE_PISTOL) 
				&&((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY))
				) {
				messagePtr->Special=1;
			} else {
				messagePtr->Special=0;
			}

			if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)&&(PlayerStatusPtr->IsAlive))
				messagePtr->IAmFiringPrimary = 1;
			else messagePtr->IAmFiringPrimary = 0;
			if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY)&&(PlayerStatusPtr->IsAlive))
				messagePtr->IAmFiringSecondary = 1;
			else messagePtr->IAmFiringSecondary = 0;
			/* whether or not I'm displaying a gun flash */
			if(MyPlayerHasAMuzzleFlash(sbPtr)) messagePtr->IHaveAMuzzleFlash = 1;
			else messagePtr->IHaveAMuzzleFlash = 0;
		}
	}
	/* whether or not I'm alive */
	if(PlayerStatusPtr->IsAlive) messagePtr->IAmAlive = 1;
	else messagePtr->IAmAlive = 0;

	netGameData.playerData[playerIndex].playerAlive=messagePtr->IAmAlive;
	
	//Is the player alive or in possesion of extra lives?
	if(messagePtr->IAmAlive)
	{
		messagePtr->IHaveLifeLeft=1;
	}
	else
	{
		messagePtr->IHaveLifeLeft=AreThereAnyLivesLeft();
	}
	
	netGameData.playerData[playerIndex].playerHasLives=messagePtr->IHaveLifeLeft;
	
	/*Am I currently invulnerable?*/
	if(PlayerStatusPtr->invulnerabilityTimer>0)
		messagePtr->IAmInvulnerable=1;
	else
		messagePtr->IAmInvulnerable=0;


	/* whether or not I'm the host */
//	if(AvP.Network==I_Host) messagePtr->IAmHost = 1;
//	else messagePtr->IAmHost = 0;

	/* whether or not I'm a cloaked predator */
	{
 		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);    	        

		if(playerStatusPtr->cloakOn) 
		{
			LOCALASSERT(AvP.PlayerType==I_Predator);
			
			if (playerStatusPtr->CloakingEffectiveness>65535)
			{
				messagePtr->CloakingEffectiveness = 65535;
			}
			else
			{
				messagePtr->CloakingEffectiveness = (unsigned short)(playerStatusPtr->CloakingEffectiveness);
			}
		}
		else messagePtr->CloakingEffectiveness = 0;
	}
	
	/* Am I on fire? */
	if (sbPtr->SBDamageBlock.IsOnFire) {
		messagePtr->IAmOnFire=1;
	} else {
		messagePtr->IAmOnFire=0;
	}
	/* Do I have a disk?  Probably not. */
	messagePtr->IHaveADisk=0;
	if (AvP.PlayerType==I_Predator) {
		if (messagePtr->currentWeapon==WEAPON_PRED_DISC) {
			if (PlayersWeapon.HModelControlBlock) {
				SECTION_DATA *disc_section;
				disc_section=GetThisSectionData(PlayersWeapon.HModelControlBlock->section_data,"disk");
				GLOBALASSERT(disc_section);
				if ((disc_section->flags&section_data_notreal)==0) {
					/* We have a disk! */
					messagePtr->IHaveADisk=1;
				}
			}
		}
	}

	//have we been screaming?
	if(netGameData.myLastScream!=-1)
	{
		messagePtr->scream=netGameData.myLastScream;
	}
	else
	{
		messagePtr->scream=31;
	}
	//reset last scream , so we don't keep sending it
	netGameData.myLastScream=-1;

	/* CDF 21/4/99 Add landing noise? */
	messagePtr->landingNoise=netGameData.landingNoise;
	//reset that too!
	netGameData.landingNoise=0;

}

void AddNetMsg_PlayerState_Minimal(STRATEGYBLOCK *sbPtr,BOOL sendOrient)
{
	PLAYER_WEAPON_DATA *weaponPtr=0;
	int playerIndex;
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERSTATE_MINIMAL *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERSTATE_MINIMAL);
	if(sendOrient)
	{
		messageSize = sizeof(NETMESSAGE_PLAYERSTATE_MEDIUM);
	}



	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERSTATE_MINIMAL *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	if(sendOrient)
		headerPtr->type = (unsigned char)NetMT_PlayerState_Medium;
	else
		headerPtr->type = (unsigned char)NetMT_PlayerState_Minimal;
		

	playerIndex = PlayerIdInPlayerList(AVPDPNetID);
	GLOBALASSERT(playerIndex!=NET_IDNOTINPLAYERLIST);
	
	/* KJL 17:04:22 26/01/98 - elevation (for weapon, etc.) */
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		messagePtr->Elevation = playerStatusPtr->ViewPanX;
	}

	/* my current weapon id, and whether I am firing it... */
	{
 		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);    	        
    	weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

		if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
			if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)&&(PlayerStatusPtr->IsAlive)) {
				if (LastHand) {
					messagePtr->IAmFiringPrimary = 0;
					messagePtr->IAmFiringSecondary = 1;
				} else {
					messagePtr->IAmFiringPrimary = 1;
					messagePtr->IAmFiringSecondary = 0;
				}				
			} else {
				messagePtr->IAmFiringPrimary = 0;
				messagePtr->IAmFiringSecondary = 0;
			}
			/* whether or not I'm displaying a gun flash */
			messagePtr->IHaveAMuzzleFlash = MyPlayerHasAMuzzleFlash(sbPtr);
		} else {
			if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)&&(PlayerStatusPtr->IsAlive))
				messagePtr->IAmFiringPrimary = 1;
			else messagePtr->IAmFiringPrimary = 0;
			if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY)&&(PlayerStatusPtr->IsAlive))
				messagePtr->IAmFiringSecondary = 1;
			else messagePtr->IAmFiringSecondary = 0;
			/* whether or not I'm displaying a gun flash */
			if(MyPlayerHasAMuzzleFlash(sbPtr)) messagePtr->IHaveAMuzzleFlash = 1;
			else messagePtr->IHaveAMuzzleFlash = 0;
		}
	}

	/* whether or not I'm alive */
	if(PlayerStatusPtr->IsAlive) messagePtr->IAmAlive = 1;
	else messagePtr->IAmAlive = 0;

	netGameData.playerData[playerIndex].playerAlive=messagePtr->IAmAlive;
	
	//Is the player alive or in possesion of extra lives?
	if(messagePtr->IAmAlive)
	{
		messagePtr->IHaveLifeLeft=1;
	}
	else
	{
		messagePtr->IHaveLifeLeft=AreThereAnyLivesLeft();
	}
	
	netGameData.playerData[playerIndex].playerHasLives=messagePtr->IHaveLifeLeft;
	


	
	/* whether or not I'm a cloaked predator */
	{
 		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);    	        

		if(playerStatusPtr->cloakOn) 
		{
			LOCALASSERT(AvP.PlayerType==I_Predator);
			
			if (playerStatusPtr->CloakingEffectiveness>65535)
			{
				messagePtr->CloakingEffectiveness = 65535>>8;
			}
			else
			{
				messagePtr->CloakingEffectiveness = (unsigned short)(playerStatusPtr->CloakingEffectiveness>>8);
			}
		}
		else messagePtr->CloakingEffectiveness = 0;
	}
	
	
	/* Am I on fire? */
	if (sbPtr->SBDamageBlock.IsOnFire) {
		messagePtr->IAmOnFire=1;
	} else {
		messagePtr->IAmOnFire=0;
	}
	/* Do I have a disk?  Probably not. */
	messagePtr->IHaveADisk=0;
	if (AvP.PlayerType==I_Predator) {
		if (weaponPtr->WeaponIDNumber==WEAPON_PRED_DISC) {
			if (PlayersWeapon.HModelControlBlock) {
				SECTION_DATA *disc_section;
				disc_section=GetThisSectionData(PlayersWeapon.HModelControlBlock->section_data,"disk");
				GLOBALASSERT(disc_section);
				if ((disc_section->flags&section_data_notreal)==0) {
					/* We have a disk! */
					messagePtr->IHaveADisk=1;
				}
			}
		}
	}

	if(sendOrient)
	{
		NETMESSAGE_PLAYERSTATE_MEDIUM* mediumMessage=(NETMESSAGE_PLAYERSTATE_MEDIUM*) messagePtr;
		mediumMessage->xOrient = (sbPtr->DynPtr->OrientEuler.EulerX>>NET_EULERSCALESHIFT);
		mediumMessage->yOrient = (sbPtr->DynPtr->OrientEuler.EulerY>>NET_EULERSCALESHIFT);
		mediumMessage->zOrient = (sbPtr->DynPtr->OrientEuler.EulerZ>>NET_EULERSCALESHIFT);
	}
}

void AddNetMsg_FrameTimer()
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_FRAMETIMER *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_FRAMETIMER);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_FRAMETIMER *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_FrameTimer;

	/* fill out the message */
	messagePtr->frame_time = (unsigned short)GameTimeSinceLastSend;
	GameTimeSinceLastSend=0;
}

/* support function for addnetmsg_playerstate() */	
static int MyPlayerHasAMuzzleFlash(STRATEGYBLOCK *sbPtr)
{
	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
 	PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
    	        
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
		
	/* first check if we are displaying a muzle flash ourselves */
	if(twPtr->MuzzleFlashShapeName == NULL) return 0;
	if(twPtr->PrimaryIsMeleeWeapon) return 0;
	
	if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
		if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY) {
			if (LastHand) {
				return 2;
			} else {
				return 1;
			}
		} else if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_SECONDARY) {
			if (LastHand) {
				return 2;
			} else {
				return 1;
			}
			return 0;
		}
	}
	
	if (weaponPtr->WeaponIDNumber==WEAPON_MARINE_PISTOL) {
		if ((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY) 
			||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY)) {
			//ReleasePrintDebuggingText("Pistol Muzzle Flash 1\n");
			return 1;
		} else {
			//ReleasePrintDebuggingText("Pistol Muzzle Flash 0\n");
			return 0;
		}
	}
	
	if (weaponPtr->CurrentState != WEAPONSTATE_FIRING_PRIMARY) {
		return 0;
	}

	/* even if we are displaying our own muzzle flash, we don't neccessarily want it to
	be visible to other players (because it looks stupid) */
	if((weaponPtr->WeaponIDNumber==WEAPON_PULSERIFLE)||
	   (weaponPtr->WeaponIDNumber==WEAPON_MARINE_PISTOL)||
	   (weaponPtr->WeaponIDNumber==WEAPON_AUTOSHOTGUN)||
	   (weaponPtr->WeaponIDNumber==WEAPON_SMARTGUN)||
	   (weaponPtr->WeaponIDNumber==WEAPON_MINIGUN)||
	   (weaponPtr->WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER)||
	   (weaponPtr->WeaponIDNumber==WEAPON_PRED_PISTOL)||
	   (weaponPtr->WeaponIDNumber==WEAPON_PRED_RIFLE))
	{
		/* if we get this far, we want to display a muzzle flash */
		return 1;		
	}
	return 0;
}

char GetWeaponIconFromDamage(DAMAGE_PROFILE* damage)
{
	#define ICON_CUDGEL 177
	#define ICON_PULSERIFLE 177
	#define ICON_PULSERIFLE_GRENADE 177
	#define ICON_SMARTGUN 138
	#define ICON_FLAMER 137
	#define ICON_SADAR 140
	#define ICON_GRENADE_STANDARD 139
	#define ICON_GRENADE_PROX 139
	#define ICON_GRENADE_FRAG 139
	#define ICON_MINIGUN 141
	#define ICON_SKEETER 143
	#define ICON_MARINE_PISTOL 142

	#define ICON_CLAW 152
	#define ICON_TAIL 151
	#define ICON_JAWS 176

	#define ICON_WRISTBLADE 154
	#define ICON_PRED_PISTOL 158
	#define ICON_SHOULDERCANNON 156
	#define ICON_SPEARGUN 155
	#define ICON_DISC 157

	
	if(!damage) return 0;
	switch(damage->Id)
	{
		case AMMO_10MM_CULW :
			return ICON_PULSERIFLE;

		case AMMO_PULSE_GRENADE :
		case AMMO_PULSE_GRENADE_STRIKE :
			return ICON_PULSERIFLE_GRENADE;

		case AMMO_SMARTGUN :
			return ICON_SMARTGUN;

		case AMMO_FLAMETHROWER :
		case AMMO_FIREDAMAGE_POSTMAX :
			return ICON_FLAMER;

		case AMMO_SADAR_TOW :
		case AMMO_SADAR_BLAST :
			return ICON_SADAR;
		
		case AMMO_GRENADE :
			return ICON_GRENADE_STANDARD;
			
		case AMMO_FRAGMENTATION_GRENADE :
		case AMMO_FLECHETTE_POSTMAX :
			return ICON_GRENADE_FRAG;

		case AMMO_PROXIMITY_GRENADE :
			return ICON_GRENADE_PROX;

		case AMMO_MINIGUN :
			return ICON_MINIGUN ;

		case AMMO_MARINE_PISTOL :
		case AMMO_MARINE_PISTOL_PC :
			return ICON_MARINE_PISTOL;

		case AMMO_CUDGEL :
			return ICON_CUDGEL;

		case AMMO_FRISBEE :
		case AMMO_FRISBEE_BLAST :
		case AMMO_FRISBEE_FIRE :
			return ICON_SKEETER;

		
		case AMMO_PRED_WRISTBLADE :
		case AMMO_HEAVY_PRED_WRISTBLADE :
		case AMMO_PRED_TROPHY_KILLSECTION :
			return ICON_WRISTBLADE;

		case AMMO_PRED_PISTOL :
			return ICON_PRED_PISTOL;
		
		case AMMO_PRED_RIFLE :
			return ICON_SPEARGUN;

		case AMMO_PRED_ENERGY_BOLT :
		case AMMO_PLASMACASTER_NPCKILL :
		case AMMO_PLASMACASTER_PCKILL :
			return ICON_SHOULDERCANNON;

		case AMMO_PRED_DISC :
			return ICON_DISC;
	
		case AMMO_PREDPISTOL_STRIKE :
			return ICON_PRED_PISTOL;
		

		case AMMO_ALIEN_CLAW :
			return ICON_CLAW;
	
		case AMMO_ALIEN_TAIL :
			return ICON_TAIL;

		case AMMO_ALIEN_BITE_KILLSECTION :
		case AMMO_PC_ALIEN_BITE :
		case AMMO_ALIEN_BITE_KILLSECTION_SUPER :
			return ICON_JAWS;
		default:
			break;
	}

	return 0;
}

void AddNetMsg_PlayerKilled(int objectId,DAMAGE_PROFILE* damage)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERKILLED *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERKILLED);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* definitely should be actually dead */
	LOCALASSERT(PlayerStatusPtr->IsAlive==0);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERKILLED *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerKilled;

	/* fill out the message: myNetworkkillerId should either be NULL indicating that player
	has killed himself, or the DPID of the killer (which may in fact be the player's DPID)*/
	messagePtr->objectId=objectId; /* ID of the new corpse. */
	messagePtr->killerId = myNetworkKillerId;
	messagePtr->myType=netGameData.myCharacterType;

	if(myNetworkKillerId)
	{
		int killer_index=PlayerIdInPlayerList(myNetworkKillerId);
		if(killer_index!=NET_IDNOTINPLAYERLIST)
		{
			messagePtr->killerType=netGameData.playerData[killer_index].characterType;
		}
		else
		{
			//the player doing the damage has either left the game , or never existed.
			//call it suicide then.
			myNetworkKillerId=AVPDPNetID;	
			messagePtr->killerId=0;
		}
	}
	
	if(!myNetworkKillerId || myNetworkKillerId==AVPDPNetID)
	{
		//suicide (or killed by alien , in which case this will be corrected a few lines later)
		messagePtr->killerType=messagePtr->myType;
	}

	//find the icon for the weapon used
	messagePtr->weaponIcon = GetWeaponIconFromDamage(damage);

	/*look at the damage type to see if the damage was done by an ai alien*/
	if(damage)
	{
		switch (damage->Id)
		{
			case AMMO_NPC_ALIEN_CLAW :
			case AMMO_NPC_ALIEN_TAIL :
			case AMMO_NPC_ALIEN_BITE :
				messagePtr->killerType=NGCT_AI_Alien;
				break;

			case AMMO_NPC_PREDALIEN_CLAW :
			case AMMO_NPC_PREDALIEN_BITE :
			case AMMO_NPC_PREDALIEN_TAIL :
				messagePtr->killerType=NGCT_AI_Predalien;
				break;

			case AMMO_NPC_PRAETORIAN_CLAW :
			case AMMO_NPC_PRAETORIAN_BITE :
			case AMMO_NPC_PRAETORIAN_TAIL :
				messagePtr->killerType=NGCT_AI_Praetorian;
				break;
			default:
				break;
		}
	}
	Inform_PlayerHasDied(myNetworkKillerId,AVPDPNetID,messagePtr->killerType,messagePtr->weaponIcon);

	if(AvP.Network==I_Host)
	{
		DoNetScoresForHostDeath(messagePtr->myType,messagePtr->killerType);
	}

}

void AddNetMsg_PlayerDeathAnim(int deathId,int objectId)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_CORPSEDEATHANIM *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_CORPSEDEATHANIM);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_CORPSEDEATHANIM *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_CorpseDeathAnim;

	messagePtr->objectId=objectId; /* ID of the new corpse. */
	messagePtr->deathId = deathId;

}

void AddNetMsg_PlayerLeaving(void)
{
	NETMESSAGEHEADER *headerPtr;
	int headerSize = sizeof(NETMESSAGEHEADER);

	/* some conditions */
	//LOCALASSERT(AvP.Network==I_Peer);
	/* yes: need to send this before changing state, as we need to know our previous state,
	and sendMessage requires one of these two states anyway */
	LOCALASSERT((netGameData.myGameState==NGS_StartUp)||(netGameData.myGameState==NGS_Playing)||(netGameData.myGameState==NGS_Joining)||(netGameData.myGameState==NGS_EndGame)||(netGameData.myGameState==NGS_EndGameScreen));
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerLeaving;	
}

void AddNetMsg_AllGameScores(void)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_ALLGAMESCORES *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_ALLGAMESCORES);

	/* should be sent by host only, whilst in end game */
	LOCALASSERT(AvP.Network==I_Host);
	LOCALASSERT((netGameData.myGameState==NGS_Playing)||(netGameData.myGameState==NGS_EndGameScreen));

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_ALLGAMESCORES *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_AllGameScores;
	
	/*fill out the message */
	{ int i,j;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			for(j=0;j<NET_MAXPLAYERS;j++)
			{
				messagePtr->playerFrags[i][j] = netGameData.playerData[i].playerFrags[j];		
			}
			messagePtr->playerScores[i] = netGameData.playerData[i].playerScore;		
			messagePtr->playerScoresAgainst[i] = netGameData.playerData[i].playerScoreAgainst;		

			for(j=0;j<3;j++)
			{
				messagePtr->aliensKilled[i][j]=netGameData.playerData[i].aliensKilled[j];
			}

			messagePtr->deathsFromAI[i] = netGameData.playerData[i].deathsFromAI;
		}
	}
}

void AddNetMsg_PlayerScores(int playerId)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERSCORES *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERSCORES);

	/* should be sent by host only, whilst playing game */
	LOCALASSERT(AvP.Network==I_Host);
	LOCALASSERT((netGameData.myGameState==NGS_Playing));

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERSCORES *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerScores;
	
	/*fill out the message */
	messagePtr->playerId = (unsigned char)playerId;
	{ int i;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			messagePtr->playerFrags[i] = netGameData.playerData[playerId].playerFrags[i];		
		}
		messagePtr->playerScore=netGameData.playerData[playerId].playerScore;
		messagePtr->playerScoreAgainst=netGameData.playerData[playerId].playerScoreAgainst;

		for(i=0;i<3;i++)
		{
			messagePtr->aliensKilled[i]=netGameData.playerData[playerId].aliensKilled[i];
		}

		messagePtr->deathsFromAI=netGameData.playerData[playerId].deathsFromAI;
	}
}

void AddNetMsg_ScoreChange(int killerIndex,int victimIndex) 
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_SCORECHANGE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_SCORECHANGE);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_SCORECHANGE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_ScoreChange;

	/* Fill in message. */
	messagePtr->killerIndex=(unsigned char) killerIndex;
	messagePtr->victimIndex=(unsigned char) victimIndex;
	if(killerIndex==NET_MAXPLAYERS)
	{
		//killed by ai
		messagePtr->fragCount=netGameData.playerData[victimIndex].deathsFromAI;
	}
	else
	{
		//killed by a player
		messagePtr->fragCount=netGameData.playerData[killerIndex].playerFrags[victimIndex];
		messagePtr->killerScoreFor=netGameData.playerData[killerIndex].playerScore;
	}
	messagePtr->victimScoreAgainst=netGameData.playerData[victimIndex].playerScoreAgainst;

}

void AddNetMsg_SpeciesScores()
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_SPECIESSCORES *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_SPECIESSCORES);
	int i;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_SPECIESSCORES *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_SpeciesScores;

	/* Fill in message. */
	for(i=0;i<3;i++)
	{
		messagePtr->teamScores[i]=netGameData.teamScores[i];
	}
}

/* for sending information about bullet ricochets and plasma impacts */
void AddNetMsg_LocalRicochet(AVP_BEHAVIOUR_TYPE behaviourType, VECTORCH *position, VECTORCH *direction)
{
}

void AddNetMsg_LocalObjectState(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOBSTATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LOBSTATE);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}

	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LOBSTATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LocalObjectState;

	/* fill out the message */
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

		LOCALASSERT(dynPtr);
//		LOCALASSERT((dynPtr->Position.vx < 4194304)&&(dynPtr->Position.vx > -4194304)); /* 23 bits of data */
//		LOCALASSERT((dynPtr->Position.vy < 4194304)&&(dynPtr->Position.vy > -4194304)); /* 23 bits of data */
//		LOCALASSERT((dynPtr->Position.vz < 4194304)&&(dynPtr->Position.vz > -4194304)); /* 23 bits of data */
		LOCALASSERT((dynPtr->OrientEuler.EulerX >=0 )&&(dynPtr->OrientEuler.EulerX < 4096)); /* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerY >=0 )&&(dynPtr->OrientEuler.EulerY < 4096)); /* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerZ >=0 )&&(dynPtr->OrientEuler.EulerZ < 4096));	/* 9 bits of signed data */

		/* NB we can fit +-4194303 into 23 bits */
		if(dynPtr->Position.vx < -4100000) messagePtr->xPos = -4100000;
		else if(dynPtr->Position.vx > 4100000) messagePtr->xPos = 4100000;
		else messagePtr->xPos = dynPtr->Position.vx;
		messagePtr->xOrient = (dynPtr->OrientEuler.EulerX>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vy < -4100000) messagePtr->yPos = -4100000;
		else if(dynPtr->Position.vy > 4100000) messagePtr->yPos = 4100000;
		else messagePtr->yPos = dynPtr->Position.vy;
		messagePtr->yOrient = (dynPtr->OrientEuler.EulerY>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vz < -4100000) messagePtr->zPos = -4100000;
		else if(dynPtr->Position.vz > 4100000) messagePtr->zPos = 4100000;
		else messagePtr->zPos = dynPtr->Position.vz;
		messagePtr->zOrient = (dynPtr->OrientEuler.EulerZ>>NET_EULERSCALESHIFT);		 
	}

	{
		int obId = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((obId >= -NET_MAXOBJECTID)&&(obId <= NET_MAXOBJECTID));
		messagePtr->objectId = obId;
	}

	messagePtr->event_flag=0;

	LOCALASSERT((sbPtr->I_SBtype >= 0)&&(sbPtr->I_SBtype < 256));
	messagePtr->type = (unsigned char)sbPtr->I_SBtype;
	LOCALASSERT((sbPtr->I_SBtype == I_BehaviourRocket)||
				(sbPtr->I_SBtype == I_BehaviourPredatorEnergyBolt)||
				(sbPtr->I_SBtype == I_BehaviourFrisbeeEnergyBolt)||
				(sbPtr->I_SBtype == I_BehaviourPPPlasmaBolt)||
				(sbPtr->I_SBtype == I_BehaviourSpeargunBolt)||
				(sbPtr->I_SBtype == I_BehaviourGrenade)||
				(sbPtr->I_SBtype == I_BehaviourPulseGrenade)||
				(sbPtr->I_SBtype == I_BehaviourFlareGrenade)||
				(sbPtr->I_SBtype == I_BehaviourFragmentationGrenade)||
				(sbPtr->I_SBtype == I_BehaviourClusterGrenade)||
				(sbPtr->I_SBtype == I_BehaviourNPCPredatorDisc)||
				(sbPtr->I_SBtype == I_BehaviourPredatorDisc_SeekTrack)||
				(sbPtr->I_SBtype == I_BehaviourAlienSpit)||				
				(sbPtr->I_SBtype == I_BehaviourProximityGrenade)||
				(sbPtr->I_SBtype == I_BehaviourInanimateObject)||
				(sbPtr->I_SBtype == I_BehaviourFrisbee)||
				(sbPtr->I_SBtype == I_BehaviourNetCorpse));

	#if 1
	if (sbPtr->I_SBtype==I_BehaviourInanimateObject) {
		INANIMATEOBJECT_STATUSBLOCK* objStatPtr = sbPtr->SBdataptr;

		messagePtr->IOType = (unsigned char)objStatPtr->typeId;
		messagePtr->subtype = (unsigned char)objStatPtr->subType;
	} else {
		messagePtr->IOType = (unsigned char)IOT_Non;
		messagePtr->subtype = (unsigned char)0;
	}
	#endif

	if (sbPtr->I_SBtype==I_BehaviourPredatorDisc_SeekTrack) {
	    PC_PRED_DISC_BEHAV_BLOCK *bbPtr = (PC_PRED_DISC_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	
		if (bbPtr->Stuck) {
			/* Signal stuck-ness. */
			messagePtr->event_flag=1;
		}
		else if(bbPtr->Bounced)
		{
			messagePtr->event_flag=2;
			bbPtr->Bounced=0;
		}	
	}
	else if (sbPtr->I_SBtype==I_BehaviourFrisbee) {
	    FRISBEE_BEHAV_BLOCK *fbPtr = (FRISBEE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
	
		if(fbPtr->Bounced)
		{
			messagePtr->event_flag=2;
			fbPtr->Bounced=0;
		}	
	}
	else if(sbPtr->I_SBtype==I_BehaviourFlareGrenade)
	{
	    FLARE_BEHAV_BLOCK *bbPtr = (FLARE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
		if(bbPtr->becomeStuck)
		{
			//set the event flag so that other players will start the flare noise
			messagePtr->event_flag=1;
			bbPtr->becomeStuck=0;
		}
	}
	else if ((sbPtr->I_SBtype==I_BehaviourGrenade)||(sbPtr->I_SBtype==I_BehaviourClusterGrenade))
	{
	    GRENADE_BEHAV_BLOCK *bbPtr = (GRENADE_BEHAV_BLOCK * ) sbPtr->SBdataptr;
		if (bbPtr->bouncelastframe) {
			/* Set event flag to record bounce sound. */
			messagePtr->event_flag=1;
		}	
	}
}


void AddNetMsg_LocalObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int sectionID,int delta_seq,int delta_sub_seq,VECTORCH* incoming)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOBDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;
	NETMESSAGE_DAMAGE_SECTION *messageSection=0;
	NETMESSAGE_DAMAGE_DELTA *messageDelta=0;
	NETMESSAGE_DAMAGE_DIRECTION *messageDirection=0;

	int headerSize = sizeof(NETMESSAGEHEADER);
	int maxMessageSize = sizeof(NETMESSAGE_LOBDAMAGED_HEADER)+
					  sizeof(NETMESSAGE_DAMAGE_PROFILE)+	
					  sizeof(NETMESSAGE_DAMAGE_MULTIPLE)+	
					  sizeof(NETMESSAGE_DAMAGE_SECTION)+	
					  sizeof(NETMESSAGE_DAMAGE_DELTA)+	
					  sizeof(NETMESSAGE_DAMAGE_DIRECTION);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + maxMessageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	
	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LocalObjectDamaged;

	{
		NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		
		LOCALASSERT(ghostData);
		if(sbPtr->I_SBtype != I_BehaviourNetGhost)
		{
			LOCALASSERT(1==0);
		}
	
/*--------------------**
** 	set up the header **
**--------------------*/
		messageHeader = (NETMESSAGE_LOBDAMAGED_HEADER *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_LOBDAMAGED_HEADER);

		messageHeader->playerId=ghostData->playerId;
		messageHeader->objectId = ghostData->playerObjectId;	
		messageHeader->ammo_id=damage->Id;

/*-----------------**
** 	damage profile **
**-----------------*/
		messageHeader->damageProfile=1;
		if(damage->Id>AMMO_NONE && damage->Id<MAX_NO_OF_AMMO_TEMPLATES)
		{
			if(AreDamageProfilesEqual(damage,&TemplateAmmo[damage->Id].MaxDamage[AvP.Difficulty]))
			{
				messageHeader->damageProfile=0;
			}
		}
		else if(damage->Id==AMMO_FLECHETTE_POSTMAX)
		{
			messageHeader->damageProfile=0;
		}
		if(messageHeader->damageProfile)
		{
			messageProfile = (NETMESSAGE_DAMAGE_PROFILE *)endSendBuffer;
			endSendBuffer += sizeof(NETMESSAGE_DAMAGE_PROFILE);

			messageProfile->Impact = damage->Impact;
			messageProfile->Cutting = damage->Cutting;
			messageProfile->Penetrative = damage->Penetrative;
			messageProfile->Fire = damage->Fire;
			messageProfile->Electrical = damage->Electrical;
			messageProfile->Acid = damage->Acid;
		
			messageProfile->ExplosivePower=damage->ExplosivePower;
			messageProfile->Slicing=damage->Slicing;
			messageProfile->ProduceBlood=damage->ProduceBlood;
			messageProfile->ForceBoom=damage->ForceBoom;

			messageProfile->BlowUpSections=damage->BlowUpSections;
			messageProfile->Special=damage->Special;
			messageProfile->MakeExitWounds=damage->MakeExitWounds;
			
		}

/*-----------------**
** damage multiple **
**-----------------*/
		if(multiple!=ONE_FIXED)
		{
			messageHeader->multiple=1;
			messageMultiple = (NETMESSAGE_DAMAGE_MULTIPLE *)endSendBuffer;
			endSendBuffer += sizeof(NETMESSAGE_DAMAGE_MULTIPLE);

			messageMultiple->multiple=multiple;

		}
		else
		{
			messageHeader->multiple=0;
		}
/*------------**
** section id **
**------------*/
		if(sectionID!=-1)
		{
			messageHeader->sectionID=1;
			messageSection = (NETMESSAGE_DAMAGE_SECTION *)endSendBuffer;
			endSendBuffer += sizeof(NETMESSAGE_DAMAGE_SECTION);

			messageSection->SectionID = (short)sectionID;
		}
		else
		{
			messageHeader->sectionID=0;
		}

/*----------------**
** delta sequence **
**----------------*/
		if(delta_seq!=-1)
		{
			messageHeader->delta_seq=1;
			messageDelta = (NETMESSAGE_DAMAGE_DELTA *)endSendBuffer;
			endSendBuffer += sizeof(NETMESSAGE_DAMAGE_DELTA);

			messageDelta->Delta_Sequence=(char)delta_seq;
			messageDelta->Delta_Sub_Sequence=(char)delta_sub_seq;
		}
		else
		{
			messageHeader->delta_seq=0;
		}
		
/*-------------------**
** direction		 **
**-------------------*/

		if(incoming && sbPtr->DynPtr)
		{
			VECTORCH direction=*incoming;

			messageHeader->direction=1;
			messageDirection = (NETMESSAGE_DAMAGE_DIRECTION *)endSendBuffer;
			endSendBuffer += sizeof(NETMESSAGE_DAMAGE_DIRECTION);
		
			//need to rotate the vector into world space
			RotateVector(&direction,&sbPtr->DynPtr->OrientMat);

			//compress vector
			messageDirection->direction_x=direction.vx>>7;
			messageDirection->direction_y=direction.vy>>7;
			messageDirection->direction_z=direction.vz>>7;
		}
		else
		{
			messageHeader->direction=0;
		}

	}
	//that's it
}

void AddNetMsg_LocalObjectDestroyed_Request(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOBDESTROYED_REQUEST *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LOBDESTROYED_REQUEST);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LOBDESTROYED_REQUEST *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LocalObjectDestroyed_Request;

	/* fill out message */
	{
		NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		
		LOCALASSERT(ghostData);
		if(sbPtr->I_SBtype != I_BehaviourNetGhost)
		{
			LOCALASSERT(1==0);
		}
		
		messagePtr->playerId = ghostData->playerId;
/*
		if((ghostData->playerObjectId < -NET_MAXOBJECTID)||(ghostData->playerObjectId > NET_MAXOBJECTID))
		{
			LOCALASSERT(1==0);
		} 
*/
		messagePtr->objectId = ghostData->playerObjectId;	
		/* That's it. */
	}
}

void AddNetMsg_LocalObjectDestroyed(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOBDESTROYED *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LOBDESTROYED);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LOBDESTROYED *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LocalObjectDestroyed;

	/* fill out message */
	{
		int obId = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((obId >= -NET_MAXOBJECTID)&&(obId <= NET_MAXOBJECTID));
		messagePtr->objectId = obId;
	}
}

void AddNetMsg_InanimateObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_INANIMATEDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;

	int headerSize = sizeof(NETMESSAGEHEADER);
	int maxMessageSize = sizeof(NETMESSAGE_INANIMATEDAMAGED_HEADER)+
					  sizeof(NETMESSAGE_DAMAGE_PROFILE)+	
					  sizeof(NETMESSAGE_DAMAGE_MULTIPLE);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;
	/* shouldn't be sending this if we are the host */
	LOCALASSERT(AvP.Network!=I_Host);
	/* only send for inanimate objects*/
	LOCALASSERT(sbPtr->I_SBtype==I_BehaviourInanimateObject ||
				sbPtr->I_SBtype==I_BehaviourPlacedLight);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + maxMessageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_InanimateObjectDamaged;

/*--------------------**
** 	set up the header **
**--------------------*/
	messageHeader = (NETMESSAGE_INANIMATEDAMAGED_HEADER *)endSendBuffer;
	endSendBuffer += sizeof(NETMESSAGE_INANIMATEDAMAGED_HEADER);

	COPY_NAME(&(messageHeader->name),&(sbPtr->SBname));	
	messageHeader->ammo_id=damage->Id;


/*-----------------**
** 	damage profile **
**-----------------*/
	messageHeader->damageProfile=1;
	if(damage->Id>AMMO_NONE && damage->Id<MAX_NO_OF_AMMO_TEMPLATES)
	{
		if(AreDamageProfilesEqual(damage,&TemplateAmmo[damage->Id].MaxDamage[AvP.Difficulty]))
		{
			messageHeader->damageProfile=0;
		}
	}
	if(messageHeader->damageProfile)
	{
		messageProfile = (NETMESSAGE_DAMAGE_PROFILE *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_PROFILE);

		messageProfile->Impact = damage->Impact;
		messageProfile->Cutting = damage->Cutting;
		messageProfile->Penetrative = damage->Penetrative;
		messageProfile->Fire = damage->Fire;
		messageProfile->Electrical = damage->Electrical;
		messageProfile->Acid = damage->Acid;
	
		messageProfile->ExplosivePower=damage->ExplosivePower;
		messageProfile->Slicing=damage->Slicing;
		messageProfile->ProduceBlood=damage->ProduceBlood;
		messageProfile->ForceBoom=damage->ForceBoom;

		messageProfile->BlowUpSections=damage->BlowUpSections;
		messageProfile->Special=damage->Special;
		messageProfile->MakeExitWounds=damage->MakeExitWounds;
		
	}
/*-----------------**
** damage multiple **
**-----------------*/
	if(multiple!=ONE_FIXED)
	{
		messageHeader->multiple=1;
		messageMultiple = (NETMESSAGE_DAMAGE_MULTIPLE *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_MULTIPLE);

		messageMultiple->multiple=multiple;

	}
	else
	{
		messageHeader->multiple=0;
	}

}


void AddNetMsg_InanimateObjectDestroyed(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_INANIMATEDESTROYED *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_INANIMATEDESTROYED);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* should only send if we're the host */
	LOCALASSERT(AvP.Network==I_Host);
	LOCALASSERT(sbPtr->I_SBtype==I_BehaviourInanimateObject ||
	            sbPtr->I_SBtype==I_BehaviourPlacedLight);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_INANIMATEDESTROYED *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_InanimateObjectDestroyed;

	/* fill out message */
	{
		COPY_NAME(&(messagePtr->name),&(sbPtr->SBname));	
	}
}

void AddNetMsg_ObjectPickedUp(char* objectName)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_OBJECTPICKEDUP *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_OBJECTPICKEDUP);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_OBJECTPICKEDUP *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_ObjectPickedUp;

	/* fill out the message */
	COPY_NAME((&messagePtr->name[0]),objectName);
}

void AddNetMsg_EndGame(void)
{
	NETMESSAGEHEADER *headerPtr;
	int headerSize = sizeof(NETMESSAGEHEADER);

	/* should be sent by host only, in endGame state */
	LOCALASSERT(AvP.Network==I_Host);
	/* yes: need to send this before changing state, as we need to know our previous state,
	and sendMessage requires one of these two states anyway */
	LOCALASSERT((netGameData.myGameState==NGS_StartUp)||(netGameData.myGameState==NGS_Playing)||(netGameData.myGameState==NGS_Joining));

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_EndGame;
}

void AddNetMsg_LOSRequestBinarySwitch(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOSREQUESTBINARYSWITCH *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LOSREQUESTBINARYSWITCH);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* also, the object must be a binary switch */
	if(sbPtr->I_SBtype!=I_BehaviourBinarySwitch && sbPtr->I_SBtype!=I_BehaviourLinkSwitch) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LOSREQUESTBINARYSWITCH *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LOSRequestBinarySwitch;

	/* fill out the message */
	COPY_NAME((&messagePtr->name[0]),&(sbPtr->SBname));
}

void AddNetMsg_PlatformLiftState(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLATFORMLIFTSTATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLATFORMLIFTSTATE);
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platLiftData;

	/* only hosts should send this*/
	LOCALASSERT(AvP.Network==I_Host);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	if(sbPtr->I_SBtype!=I_BehaviourPlatform) return;
	platLiftData = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platLiftData);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLATFORMLIFTSTATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlatformLiftState;

	/* fill out the message */
	COPY_NAME((&messagePtr->name[0]),&(sbPtr->SBname));
	messagePtr->state = (char)(platLiftData->state);
}

void AddNetMsg_RequestPlatformLiftActivate(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE);
	PLATFORMLIFT_BEHAVIOUR_BLOCK *platLiftData;

	/* only peers should send this*/
	LOCALASSERT(AvP.Network==I_Peer);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	if(sbPtr->I_SBtype!=I_BehaviourPlatform) return;
	platLiftData = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(platLiftData);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_RequestPlatformLiftActivate;

	/* fill out the message */
	COPY_NAME((&messagePtr->name[0]),&(sbPtr->SBname));
}

void AddNetMsg_PlayerAutoGunState(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_AGUNSTATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_AGUNSTATE);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_AGUNSTATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PlayerAutoGunState;

	/* fill out the message */
	{
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

		LOCALASSERT(dynPtr);
		LOCALASSERT((dynPtr->Position.vx < 4194304)&&(dynPtr->Position.vx > -4194304)); /* 23 bits of data */
		LOCALASSERT((dynPtr->Position.vy < 4194304)&&(dynPtr->Position.vy > -4194304)); /* 23 bits of data */
		LOCALASSERT((dynPtr->Position.vz < 4194304)&&(dynPtr->Position.vz > -4194304)); /* 23 bits of data */
		LOCALASSERT((dynPtr->OrientEuler.EulerX >=0 )&&(dynPtr->OrientEuler.EulerX < 4096)); /* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerY >=0 )&&(dynPtr->OrientEuler.EulerY < 4096)); /* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerZ >=0 )&&(dynPtr->OrientEuler.EulerZ < 4096));	/* 9 bits of signed data */

		/* NB we can fit +-4194303 into 23 bits */
		if(dynPtr->Position.vx < -4100000) messagePtr->xPos = -4100000;
		else if(dynPtr->Position.vx > 4100000) messagePtr->xPos = 4100000;
		else messagePtr->xPos = dynPtr->Position.vx;
		messagePtr->xOrient = (dynPtr->OrientEuler.EulerX>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vy < -4100000) messagePtr->yPos = -4100000;
		else if(dynPtr->Position.vy > 4100000) messagePtr->yPos = 4100000;
		else messagePtr->yPos = dynPtr->Position.vy;
		messagePtr->yOrient = (dynPtr->OrientEuler.EulerY>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vz < -4100000) messagePtr->zPos = -4100000;
		else if(dynPtr->Position.vz > 4100000) messagePtr->zPos = 4100000;
		else messagePtr->zPos = dynPtr->Position.vz;
		messagePtr->zOrient = (dynPtr->OrientEuler.EulerZ>>NET_EULERSCALESHIFT);		 
	}

	/* add the object Id */
	{
		int obId = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((obId >= -NET_MAXOBJECTID)&&(obId <= NET_MAXOBJECTID));
		messagePtr->objectId = obId;
	}

	/* am I firing?? */
	{
		AUTOGUN_STATUS_BLOCK *agData;
		agData = (AUTOGUN_STATUS_BLOCK*)(sbPtr->SBdataptr);
		LOCALASSERT(agData);
		if(agData->Firing) messagePtr->IAmFiring = 1;
		else messagePtr->IAmFiring = 0;
		if(agData->behaviourState==I_disabled) messagePtr->IAmEnabled = 0;
		else messagePtr->IAmEnabled = 1;
	}
}

/* KJL 17:49:31 20/01/98 - transmit make decal info */
void AddNetMsg_MakeDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_MAKEDECAL *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_MAKEDECAL);

	extern int GlobalFrameCounter;
	static int DecalCountThisFrame=0;
	static int FrameStamp;

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	if(FrameStamp!=GlobalFrameCounter)
	{
		FrameStamp=GlobalFrameCounter;
		DecalCountThisFrame=0;
	}

	/*Limit the decal count per frame, even in lan games. Otherwise it is easy to get assertions from having tons
	of alien blood particles*/
	if(DecalCountThisFrame>=10)
	{
		return;
	}
	DecalCountThisFrame++;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > (numBytesLeft-1000))
		{
		//	LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_MAKEDECAL *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_MakeDecal;

	/* fill out the message */
	messagePtr->DecalID = decalID;
	messagePtr->Position = *positionPtr;	
	messagePtr->Direction = *normalPtr;
	messagePtr->ModuleIndex = moduleIndex;
}

/* KJL 14:34:45 08/04/98 - broadcast a message to the other players */
void AddNetMsg_ChatBroadcast(char *string,BOOL same_species_only)
{
	NETMESSAGEHEADER *headerPtr;
	char *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize;
	unsigned char stringLength = 1;

	//remove the console (assuming it is on screen)
	if(IOFOCUS_AcceptTyping())
	{
		IOFOCUS_Toggle();
	}
	
	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;


	{
		char *ptr = string;
		while(*ptr && stringLength<255)
		{
			stringLength++;
			ptr++;
		}
	}
	
	if (stringLength==1 || stringLength>=255) return;

	messageSize = stringLength+1;
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_ChatBroadcast;

	/* fill out the message */
	{
		char *ptr = string;
		//first byte used to distinguish between SAY and SPECIES_SAY
		*messagePtr++=(char)same_species_only;
		

		do
		{
			*messagePtr++ = *ptr;
		}
		while(*ptr++);
	}
	
	
	{
		sprintf(OnScreenMessageBuffer,"%s: %s",netGameData.playerData[PlayerIdInPlayerList(AVPDPNetID)].name,string);
		NewOnScreenMessage(OnScreenMessageBuffer);
	}

}

/* KJL 11:28:44 27/04/98 - make an explosion (just the sfx; damage is handled separately */
void AddNetMsg_MakeExplosion(VECTORCH *positionPtr, enum EXPLOSION_ID explosionID)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_MAKEEXPLOSION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_MAKEEXPLOSION);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_MAKEEXPLOSION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_MakeExplosion;

	/* fill out the message */
	messagePtr->Position = *positionPtr;
	messagePtr->ExplosionID = explosionID;	
}
void AddNetMsg_MakeFlechetteExplosion(VECTORCH *positionPtr, int seed)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_MAKEFLECHETTEEXPLOSION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_MAKEFLECHETTEEXPLOSION);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_MAKEFLECHETTEEXPLOSION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_MakeFlechetteExplosion;

	/* fill out the message */
	messagePtr->Position = *positionPtr;
	messagePtr->Seed = seed;	
}

void AddNetMsg_MakePlasmaExplosion(VECTORCH *positionPtr, VECTORCH *fromPositionPtr, enum EXPLOSION_ID explosionID)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_MAKEPLASMAEXPLOSION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_MAKEPLASMAEXPLOSION);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_MAKEPLASMAEXPLOSION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_MakePlasmaExplosion;

	/* fill out the message */
	messagePtr->Position = *positionPtr;
	messagePtr->FromPosition = *fromPositionPtr;
	messagePtr->ExplosionID = explosionID;	
}

/* KJL 11:27:47 20/05/98 - predator laser sights */
void AddNetMsg_PredatorLaserSights(VECTORCH *positionPtr, VECTORCH *normalPtr, DISPLAYBLOCK *dispPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PREDATORSIGHTS *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PREDATORSIGHTS);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PREDATORSIGHTS *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_PredatorSights;

	/* fill out the message */
	{
		MATRIXCH orientMat;
		EULER orientation;

		MakeMatrixFromDirection(normalPtr,&orientMat);
 		MatrixToEuler(&orientMat, &orientation);
	
		/* NB we can fit +-4194303 into 23 bits */
		if(positionPtr->vx < -4100000) messagePtr->xPos = -4100000;
		else if(positionPtr->vx > 4100000) messagePtr->xPos = 4100000;
		else messagePtr->xPos = positionPtr->vx;
		messagePtr->xOrient = (orientation.EulerX>>NET_EULERSCALESHIFT);
		
		if(positionPtr->vy < -4100000) messagePtr->yPos = -4100000;
		else if(positionPtr->vy > 4100000) messagePtr->yPos = 4100000;
		else messagePtr->yPos = positionPtr->vy;
		messagePtr->yOrient = (orientation.EulerY>>NET_EULERSCALESHIFT);
		
		if(positionPtr->vz < -4100000) messagePtr->zPos = -4100000;
		else if(positionPtr->vz > 4100000) messagePtr->zPos = 4100000;
		else messagePtr->zPos = positionPtr->vz;
		messagePtr->zOrient = (orientation.EulerZ>>NET_EULERSCALESHIFT);		 

		messagePtr->TargetID = 0;
		if (dispPtr)
		{
			if (dispPtr->ObStrategyBlock)
			{
				if (dispPtr->ObStrategyBlock->I_SBtype == I_BehaviourNetGhost)
				{
			   		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)dispPtr->ObStrategyBlock->SBdataptr;
					
					if (ghostDataPtr->playerObjectId==GHOST_PLAYEROBJECTID)
					{
						messagePtr->TargetID = ghostDataPtr->playerId;
					}
				}
			}
		}
	}
}

void AddNetMsg_LocalObjectOnFire(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LOBONFIRE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LOBONFIRE);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LOBONFIRE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LocalObjectOnFire;

	/* fill out message */
	{
		NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		
		LOCALASSERT(ghostData);
		if(sbPtr->I_SBtype != I_BehaviourNetGhost)
		{
			LOCALASSERT(1==0);
		}
		
		messagePtr->playerId = ghostData->playerId;
		/* LOCALASSERT((ghostData->playerObjectId >= -NET_MAXOBJECTID)&&(ghostData->playerObjectId <= NET_MAXOBJECTID)); */
/*
		if((ghostData->playerObjectId < -NET_MAXOBJECTID)||(ghostData->playerObjectId > NET_MAXOBJECTID))
		{
			LOCALASSERT(1==0);
		} 
*/
		messagePtr->objectId = ghostData->playerObjectId;	
		
		/* That's all, folks.  This object is on fire. */
	}
}

void AddNetMsg_RestartNetworkGame(int seed)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_RESTARTGAME *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_RESTARTGAME);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_RESTARTGAME *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = NetMT_RestartNetworkGame;

	/* Fill in message. */
	messagePtr->seed=seed;

}

void AddNetMsg_FragmentalObjectsStatus(void)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_FRAGMENTALOBJECTSSTATUS *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_FRAGMENTALOBJECTSSTATUS);
	static int object_batch=0;

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* only the host should send this */
	LOCALASSERT(AvP.Network==I_Host);			

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_FRAGMENTALOBJECTSSTATUS *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_FragmentalObjectsStatus;
	messagePtr->BatchNumber=object_batch;

	/* fill out the message */
	{
		int i;
		int fragNumber=0;
		int objectsToSkip=object_batch*(NUMBER_OF_FRAGMENTAL_OBJECTS<<3);
		int noPlacedLights=0;

		LOCALASSERT(AvP.Network!=I_No_Network);

		for (i=0; i<NumActiveStBlocks; i++)
		{
			STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

			if(sbPtr->I_SBtype == I_BehaviourInanimateObject)
			{
				INANIMATEOBJECT_STATUSBLOCK* objectStatusPtr = sbPtr->SBdataptr;
				LOCALASSERT(objectStatusPtr);
				
				if((objectStatusPtr->typeId == IOT_Static) && (!objectStatusPtr->Indestructable) && (!objectStatusPtr->ghosted_object) && (!objectStatusPtr->lifespanTimer))
				{
					if(objectsToSkip>0)
					{
						objectsToSkip--;
						continue;
					}
					WriteFragmentStatus(fragNumber++,(objectStatusPtr->respawnTimer==0));
				}
			}
			else if (sbPtr->I_SBtype == I_BehaviourPlacedLight)
			{
				PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
				LOCALASSERT(pl_bhv);

				if(!pl_bhv->Indestructable)
				{
					if(objectsToSkip>0)
					{
						objectsToSkip--;
						continue;
					}
					WriteFragmentStatus(fragNumber++,(pl_bhv->state!=Light_State_Broken));
				}
			}
			if(fragNumber>=(NUMBER_OF_FRAGMENTAL_OBJECTS<<3))break;
		}	
		if(i==NumActiveStBlocks)
		{
			object_batch=0;
		}
		else
		{
			//there are more objects to look at , so increment batch
			object_batch++;
		}


		textprint("noPlacedLights %d\n",noPlacedLights);
		textprint("fragNumber %d\n",fragNumber);

		for (i=0; i<NUMBER_OF_FRAGMENTAL_OBJECTS; i++)
		{
			messagePtr->StatusBitfield[i] = FragmentalObjectStatus[i];
		}
		
			
	}
}

void AddNetMsg_StrategySynch(void)
{
/*
	Scan through objects looking for strategies that may need to be synchronized between players.
	Mainly switches and track objects
	Only do 16 objects at a time , so as not to send too much data
*/
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_STRATEGYSYNCH *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_STRATEGYSYNCH);
	static int object_batch=0;

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* only the host should send this */
	LOCALASSERT(AvP.Network==I_Host);			

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_STRATEGYSYNCH *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_StrategySynch;
	messagePtr->BatchNumber=object_batch;

	/* fill out the message */
	{
		int i;
		int objectNumber=0;
		int objectsToSkip=object_batch*(NUMBER_OF_STRATEGIES_TO_SYNCH);

		LOCALASSERT(AvP.Network!=I_No_Network);

		for (i=0; i<NumActiveStBlocks; i++)
		{
			STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

			if(sbPtr->I_SBtype == I_BehaviourBinarySwitch ||
			   sbPtr->I_SBtype == I_BehaviourLinkSwitch ||
			   sbPtr->I_SBtype == I_BehaviourTrackObject)
			{
				
				if(objectsToSkip>0)
				{
					objectsToSkip--;
					continue;
				}

				switch(sbPtr->I_SBtype)
				{
					case I_BehaviourBinarySwitch :
			 			WriteStrategySynch(objectNumber++,BinarySwitchGetSynchData(sbPtr));
						break;
					case I_BehaviourLinkSwitch :
			 			WriteStrategySynch(objectNumber++,LinkSwitchGetSynchData(sbPtr));
						break;
					case I_BehaviourTrackObject :
			 			WriteStrategySynch(objectNumber++,TrackObjectGetSynchData(sbPtr));
						break;
					default:
						break;
				}
				
			}
			if(objectNumber>=(NUMBER_OF_STRATEGIES_TO_SYNCH))break;
		}	
		if(i==NumActiveStBlocks)
		{
			object_batch=0;
		}
		else
		{
			//there are more objects to look at , so increment batch
			object_batch++;
		}


		for (i=0; i<NUMBER_OF_STRATEGIES_TO_SYNCH>>2; i++)
		{
			messagePtr->StatusBitfield[i] = StrategySynchArray[i];
		}
		
			
	}

	if(!netGameData.myStrategyCheckSum) netGameData.myStrategyCheckSum=GetStrategySynchObjectChecksum();
	messagePtr->strategyCheckSum=netGameData.myStrategyCheckSum;
}

void AddNetMsg_CreateWeapon(char* objectName,int type,VECTORCH* location)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_CREATEWEAPON *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_CREATEWEAPON);

	/* only send this if we are playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_CREATEWEAPON *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_CreateWeapon;

	/* fill out the message */
	COPY_NAME((&messagePtr->name[0]),objectName);
	messagePtr->location=*location;
	messagePtr->type=type;
}
/* KJL 16:32:06 17/06/98 - alien AI network messages */
void AddNetMsg_AlienAIState(STRATEGYBLOCK *sbPtr)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_ALIENAISTATE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_ALIENAISTATE);

	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	ALIEN_STATUS_BLOCK *alienStatusPtr=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);

	LOCALASSERT(dynPtr);
	GLOBALASSERT(alienStatusPtr);

	#if EXTRAPOLATION_TEST
	if(UseExtrapolation && netGameData.sendFrequency)
	{
		BOOL updateRequired=FALSE;
		VECTORCH facing;
		//can we get away with not sending an update this frame

		//has it been a while since the last send
		if(TimeCounterForExtrapolation<alienStatusPtr->timeOfLastSend ||
		   TimeCounterForExtrapolation-alienStatusPtr->timeOfLastSend>ONE_FIXED/4)
		{
			updateRequired=TRUE;
		}
		
		if(dynPtr->LinImpulse.vx || dynPtr->LinImpulse.vy || dynPtr->LinImpulse.vz)
		{
			//alien is probably jumping , extrapolation doesn't work in this case
			updateRequired=TRUE;
		}

		//has the velocity changed
		if(!updateRequired)
		{
			int diff=Magnitude(&dynPtr->LinVelocity)-Magnitude(&alienStatusPtr->lastVelocitySent);
			if(diff>500 || -diff>500)
			{
				updateRequired=TRUE;
			}
		}
		facing.vx=dynPtr->OrientMat.mat31;	
		facing.vy=dynPtr->OrientMat.mat32;	
		facing.vz=dynPtr->OrientMat.mat33;	
				
		//has the facing changed
		if(!updateRequired)
		{
			if(DotProduct(&facing,&alienStatusPtr->lastFacingSent)<64000)
			{
				updateRequired=TRUE;
			}
		}

		if(!updateRequired) return;

		//okay , we do need to send this frame
		alienStatusPtr->timeOfLastSend=TimeCounterForExtrapolation;
		alienStatusPtr->lastVelocitySent=dynPtr->LinVelocity;
		alienStatusPtr->lastFacingSent=facing;
			
	}
	#endif
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_ALIENAISTATE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_AlienAIState;
	
	/* fill out our position and orientation */
	{

		LOCALASSERT((dynPtr->OrientEuler.EulerX >=0 )&&(dynPtr->OrientEuler.EulerX < 4096));	/* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerY >=0 )&&(dynPtr->OrientEuler.EulerY < 4096));	/* 9 bits of signed data */
		LOCALASSERT((dynPtr->OrientEuler.EulerZ >=0 )&&(dynPtr->OrientEuler.EulerZ < 4096));	/* 9 bits of signed data */

		/* NB we can fit +-4194303 into 23 bits */
		if(dynPtr->Position.vx < -4100000) messagePtr->xPos = -4100000;
		else if(dynPtr->Position.vx > 4100000) messagePtr->xPos = 4100000;
		else messagePtr->xPos = dynPtr->Position.vx;
		messagePtr->xOrient = (dynPtr->OrientEuler.EulerX>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vy < -4100000) messagePtr->yPos = -4100000;
		else if(dynPtr->Position.vy > 4100000) messagePtr->yPos = 4100000;
		else messagePtr->yPos = dynPtr->Position.vy;
		messagePtr->yOrient = (dynPtr->OrientEuler.EulerY>>NET_EULERSCALESHIFT);
		
		if(dynPtr->Position.vz < -4100000) messagePtr->zPos = -4100000;
		else if(dynPtr->Position.vz > 4100000) messagePtr->zPos = 4100000;
		else messagePtr->zPos = dynPtr->Position.vz;
		messagePtr->zOrient = (dynPtr->OrientEuler.EulerZ>>NET_EULERSCALESHIFT);

		#if EXTRAPOLATION_TEST
		messagePtr->standard_gravity=dynPtr->UseStandardGravity;
		messagePtr->speed=Magnitude(&dynPtr->LinVelocity);
		#endif
	}

	/* fill out anim sequence */
	{

		
		#if 1
		messagePtr->sequence_type = alienStatusPtr->HModelController.Sequence_Type;
		messagePtr->sub_sequence = alienStatusPtr->HModelController.Sub_Sequence;
		
		//send the sequence length in 256ths of a second (instead of 65536ths)
		GLOBALASSERT(alienStatusPtr->HModelController.Seconds_For_Sequence>=0 && alienStatusPtr->HModelController.Seconds_For_Sequence<32*ONE_FIXED);
		messagePtr->sequence_length = alienStatusPtr->HModelController.Seconds_For_Sequence>>8;
		
		#else
		if (alienStatusPtr->HModelController.Tweening==Controller_NoTweening) {
			messagePtr->sequence_type = alienStatusPtr->HModelController.Sequence_Type;
			messagePtr->sub_sequence = alienStatusPtr->HModelController.Sub_Sequence;
			messagePtr->sequence_length = alienStatusPtr->HModelController.Seconds_For_Sequence;
		} else {
			/* Might be junk. */
			messagePtr->sequence_type  = -1;
			messagePtr->sub_sequence   = -1;
			messagePtr->sequence_length= -1;
		}
		#endif
		messagePtr->AlienType =	alienStatusPtr->Type;
	}

	if (sbPtr->SBDamageBlock.IsOnFire) {
		messagePtr->IAmOnFire=1;
	} else {
		messagePtr->IAmOnFire=0;
	}

	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((guid >= -NET_MAXOBJECTID)&&(guid <= NET_MAXOBJECTID));
		messagePtr->Guid = guid;
	}

}

/* CDF 24/8/98 A better message. */
void AddNetMsg_AlienAISeqChange(STRATEGYBLOCK *sbPtr,int sequence_type,int sub_sequence,int sequence_length,int tweening_time)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_ALIENSEQUENCECHANGE *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_ALIENSEQUENCECHANGE);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_ALIENSEQUENCECHANGE *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_AlienAISequenceChange;
	
	/* fill out anim sequence */

	messagePtr->sequence_type  =sequence_type  ;
	messagePtr->sub_sequence   =sub_sequence   ;

	//convert times into 256ths of a second
	if(sequence_length==-1)
		messagePtr->sequence_length=-1;
	else
		messagePtr->sequence_length=sequence_length>>8;
	if(tweening_time==-1)
		messagePtr->tweening_time  =-1  ;
	else
		messagePtr->tweening_time  =tweening_time>>8  ;

	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((guid >= -NET_MAXOBJECTID)&&(guid <= NET_MAXOBJECTID));
		messagePtr->Guid = guid;
	}

}

void AddNetMsg_AlienAIKilled(STRATEGYBLOCK *sbPtr,int death_code,int death_time, int GibbFactor,DAMAGE_PROFILE* damage)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_ALIENAIKILLED *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_ALIENAIKILLED);

	ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;

	//don't do this if we aren't playing (possibly on end game screen)
	if(netGameData.myGameState != NGS_Playing) return;
	
	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_ALIENAIKILLED *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_AlienAIKilled;
	
	/* fill out anim sequence */

	messagePtr->death_code=death_code;
	messagePtr->death_time=death_time;
	messagePtr->GibbFactor=GibbFactor;

	messagePtr->killerId=myNetworkKillerId;
	
	
	{
		int killerIndex=PlayerIdInPlayerList(messagePtr->killerId);
		if(killerIndex!=NET_IDNOTINPLAYERLIST)
		{
			netGameData.playerData[killerIndex].aliensKilled[alienStatus->Type]++;
			messagePtr->killCount=netGameData.playerData[killerIndex].aliensKilled[alienStatus->Type];

			//record ai's death for purposes of adjusting difficulty.
			//(assuming that optionis being used)
			GeneratorBalance_NoteAIDeath();
		}
		else
		{
			/*
			the player doing the damage has either left the game , or never existed.
			call it suicide then.
			(Could also be 'neutral' damage - flame jets)
			*/
			messagePtr->killerId=0;
			messagePtr->killCount=0;
		}

	}
	
	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((guid >= -NET_MAXOBJECTID)&&(guid <= NET_MAXOBJECTID));
		messagePtr->Guid = guid;
	}

	messagePtr->AlienType=alienStatus->Type;

	//find the icon for the weapon used
	messagePtr->weaponIcon = GetWeaponIconFromDamage(damage);
	

	Inform_AiHasDied(messagePtr->killerId,messagePtr->AlienType,messagePtr->weaponIcon);

}

void AddNetMsg_FarAlienPosition(STRATEGYBLOCK* sbPtr,int targetModuleIndex,int index,BOOL indexIsModuleIndex)
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_FARALIENPOSITION *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_FARALIENPOSITION);

	ALIEN_STATUS_BLOCK *alienStatusPtr=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_FARALIENPOSITION *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_FarAlienPosition;
	
	/* fill out indeces */
    messagePtr->targetModuleIndex = targetModuleIndex;
	messagePtr->index = index;
	messagePtr->indexIsModuleIndex = indexIsModuleIndex;

	messagePtr->alienType =	alienStatusPtr->Type;
	
	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
		messagePtr->Guid = guid;
	}
}

void AddNetMsg_GhostHierarchyDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int sectionID,VECTORCH* incoming) 
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;
	NETMESSAGE_DAMAGE_SECTION *messageSection=0;
	NETMESSAGE_DAMAGE_DIRECTION *messageDirection=0;

	int headerSize = sizeof(NETMESSAGEHEADER);
	int maxMessageSize = sizeof(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER)+
					  sizeof(NETMESSAGE_DAMAGE_PROFILE)+	
					  sizeof(NETMESSAGE_DAMAGE_MULTIPLE)+	
					  sizeof(NETMESSAGE_DAMAGE_SECTION)+	
					  sizeof(NETMESSAGE_DAMAGE_DIRECTION);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + maxMessageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_GhostHierarchyDamaged;


/*--------------------**
** 	set up the header **
**--------------------*/
	messageHeader = (NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER *)endSendBuffer;
	endSendBuffer += sizeof(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER);

	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
		messageHeader->Guid = guid;
	}
	messageHeader->ammo_id=damage->Id;

/*-----------------**
** 	damage profile **
**-----------------*/
	messageHeader->damageProfile=1;
	if(damage->Id>AMMO_NONE && damage->Id<MAX_NO_OF_AMMO_TEMPLATES)
	{
		if(AreDamageProfilesEqual(damage,&TemplateAmmo[damage->Id].MaxDamage[AvP.Difficulty]))
		{
			messageHeader->damageProfile=0;
		}
	}
	if(messageHeader->damageProfile)
	{
		messageProfile = (NETMESSAGE_DAMAGE_PROFILE *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_PROFILE);

		messageProfile->Impact = damage->Impact;
		messageProfile->Cutting = damage->Cutting;
		messageProfile->Penetrative = damage->Penetrative;
		messageProfile->Fire = damage->Fire;
		messageProfile->Electrical = damage->Electrical;
		messageProfile->Acid = damage->Acid;
	
		messageProfile->ExplosivePower=damage->ExplosivePower;
		messageProfile->Slicing=damage->Slicing;
		messageProfile->ProduceBlood=damage->ProduceBlood;
		messageProfile->ForceBoom=damage->ForceBoom;

		messageProfile->BlowUpSections=damage->BlowUpSections;
		messageProfile->Special=damage->Special;
		messageProfile->MakeExitWounds=damage->MakeExitWounds;
		
	}
/*-----------------**
** damage multiple **
**-----------------*/
	if(multiple!=ONE_FIXED)
	{
		messageHeader->multiple=1;
		messageMultiple = (NETMESSAGE_DAMAGE_MULTIPLE *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_MULTIPLE);

		messageMultiple->multiple=multiple;

	}
	else
	{
		messageHeader->multiple=0;
	}

/*------------**
** section id **
**------------*/
	if(sectionID!=-1)
	{
		messageHeader->sectionID=1;
		messageSection = (NETMESSAGE_DAMAGE_SECTION *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_SECTION);

		messageSection->SectionID = (short)sectionID;
	}
	else
	{
		messageHeader->sectionID=0;
	}
	
/*-------------------**
** direction		 **
**-------------------*/

	if(incoming && sbPtr->DynPtr)
	{
		VECTORCH direction=*incoming;

		messageHeader->direction=1;
		messageDirection = (NETMESSAGE_DAMAGE_DIRECTION *)endSendBuffer;
		endSendBuffer += sizeof(NETMESSAGE_DAMAGE_DIRECTION);
	
		//need to rotate the vector into world space
		RotateVector(&direction,&sbPtr->DynPtr->OrientMat);

		//compress vector
		messageDirection->direction_x=direction.vx>>7;
		messageDirection->direction_y=direction.vy>>7;
		messageDirection->direction_z=direction.vz>>7;
	}
	else
	{
		messageHeader->direction=0;
	}
}


void AddNetMsg_Gibbing(STRATEGYBLOCK *sbPtr,int gibbFactor,int seed) {

	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_GIBBING *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_GIBBING);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_GIBBING *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_Gibbing;
	
	messagePtr->gibbFactor=gibbFactor;
	messagePtr->seed=seed;

	/* fill out guid */
	{
		int guid = *((int *)(&(sbPtr->SBname[4])));
//		LOCALASSERT((guid >= -NET_MAXOBJECTID)&&(guid <= NET_MAXOBJECTID));
		messagePtr->Guid = guid;
	}
}


void AddNetMsg_SpotAlienSound(int soundCategory,int alienType,int pitch,VECTORCH *position) {

	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_SPOTALIENSOUND *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_SPOTALIENSOUND);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_SPOTALIENSOUND *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_SpotAlienSound;

	/* Fill in message. */
	messagePtr->soundCategory=(unsigned char)soundCategory;
	messagePtr->pitch=pitch;
	messagePtr->alienType=(unsigned char)alienType;

	messagePtr->vx=position->vx;
	messagePtr->vy=position->vy;
	messagePtr->vz=position->vz;

}
//for messages that just require a player id
void AddNetMsg_PlayerID(DPID playerID,unsigned char message) 
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_PLAYERID *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_PLAYERID);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_PLAYERID *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = message;

	/* Fill in message. */
	messagePtr->playerID=playerID;

}

void AddNetMsg_LastManStanding_RestartTimer(char time) 
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LMS_RESTARTTIMER *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LMS_RESTARTTIMER);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LMS_RESTARTTIMER *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_LastManStanding_RestartCountDown;

	/* Fill in message. */
	messagePtr->timer=time;
}

void AddNetMsg_LastManStanding_Restart(DPID alienID,int seed) 
{
	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_LMS_RESTART *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_LMS_RESTART);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_LMS_RESTART *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = NetMT_LastManStanding_Restart;

	/* Fill in message. */
	messagePtr->playerID=alienID;
	messagePtr->seed=seed;

}


void AddNetMsg_RespawnPickups(void)
{
	NETMESSAGEHEADER *headerPtr;
	int headerSize = sizeof(NETMESSAGEHEADER);

	/* only send this if we are playing or on the end game screen*/
	if(netGameData.myGameState!=NGS_Playing && netGameData.myGameState!=NGS_EndGameScreen) return;

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_RespawnPickups;
}

void AddNetMsg_SpotOtherSound(enum soundindex SoundIndex,VECTORCH *position,int explosion) {

	NETMESSAGEHEADER *headerPtr;
	NETMESSAGE_SPOTOTHERSOUND *messagePtr;
	int headerSize = sizeof(NETMESSAGEHEADER);
	int messageSize = sizeof(NETMESSAGE_SPOTOTHERSOUND);

	/* check there's enough room in the send buffer */
	{
		int numBytesReqd = headerSize + messageSize;
		int numBytesLeft = NET_MESSAGEBUFFERSIZE - ((int)(endSendBuffer - &sendBuffer[0]));
		if(numBytesReqd > numBytesLeft)
		{
			LOCALASSERT(1==0);
			/* don't add it */
			return;
		}
	}
	
	/* set up pointers to header and message structures */
	headerPtr = (NETMESSAGEHEADER *)endSendBuffer;
	endSendBuffer += headerSize;
	messagePtr = (NETMESSAGE_SPOTOTHERSOUND *)endSendBuffer;
	endSendBuffer += messageSize;

	/* fill out the header */
	headerPtr->type = (unsigned char)NetMT_SpotOtherSound;

	/* Fill in message. */
	messagePtr->SoundIndex=SoundIndex;
	if (explosion) {
		messagePtr->explosion=1;
	} else {
		messagePtr->explosion=0;
   }

	messagePtr->vx=position->vx;
	messagePtr->vy=position->vy;
	messagePtr->vz=position->vz;

}



/*----------------------------------------------------------------------
  Functions for processing each message type, as retrieved from the read
  message buffer...
  ----------------------------------------------------------------------*/
static void ProcessNetMsg_GameDescription(NETMESSAGE_GAMEDESCRIPTION *messagePtr)
{	
	/* should only get this if we're not the host, and we're in start-up state */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}

	//if(netGameData.myGameState!=NGS_Joining) return;	

	/* fill out the game description player list with the new player id's */
	{ 
		int i;
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			int playerChanged = 0;

			if ( (netGameData.playerData[i].playerId != messagePtr->players[i].playerId)
			   ||(netGameData.playerData[i].startFlag != messagePtr->players[i].startFlag) )
				playerChanged=1;

			if (netGameData.myGameState==NGS_Playing && playerChanged)
			{
				if (messagePtr->players[i].playerId==0)
				{
					Inform_PlayerHasLeft(netGameData.playerData[i].playerId);
				}
				else if (messagePtr->players[i].startFlag)
				{
					Inform_PlayerHasJoined(messagePtr->players[i].playerId);
				}
				else
				{
					Inform_PlayerHasConnected(messagePtr->players[i].playerId);
				}
			}

			if(netGameData.playerData[i].playerId != messagePtr->players[i].playerId)
			{
				if(messagePtr->players[i].playerId)
				{
					DWORD size=0;
					char* data;
					HRESULT hr;
					//need to find out this player's name
					//first find the size of buffer required
  					hr=IDirectPlayX_GetPlayerName(glpDP,messagePtr->players[i].playerId,0,&size);
					if(hr==DP_OK || hr==DPERR_BUFFERTOOSMALL)
					{
						//allocate buffer to recive the name
						data=AllocateMem(size);
						*data=0;
						hr=IDirectPlayX_GetPlayerName(glpDP,messagePtr->players[i].playerId,data,&size);

						if(hr==DP_OK)
						{
							strncpy(netGameData.playerData[i].name,((DPNAME*)data)->lpszShortNameA,NET_PLAYERNAMELENGTH-1);	
							netGameData.playerData[i].name[NET_PLAYERNAMELENGTH-1]='\0';

						}
						DeallocateMem(data);
					}

				}
				else
				{
					netGameData.playerData[i].name[0]='\0';
				}
			}
		
			netGameData.playerData[i].playerId = messagePtr->players[i].playerId;
			netGameData.playerData[i].characterType = (NETGAME_CHARACTERTYPE)messagePtr->players[i].characterType;
			netGameData.playerData[i].characterSubType = (NETGAME_CHARACTERTYPE)messagePtr->players[i].characterSubType;
			netGameData.playerData[i].startFlag = messagePtr->players[i].startFlag;
		}
		netGameData.gameType = (NETGAME_TYPE)messagePtr->gameType;
		//level number got from the session description instead
		//netGameData.levelNumber = messagePtr->levelNumber;
		netGameData.scoreLimit = messagePtr->scoreLimit;
		netGameData.timeLimit = messagePtr->timeLimit;
		netGameData.invulnerableTime = messagePtr->invulnerableTime;

		for(i=0;i<3;i++)
		{
			netGameData.characterKillValues[i]=messagePtr->characterKillValues[i];
			netGameData.aiKillValues[i]=messagePtr->aiKillValues[i];
		}
		netGameData.baseKillValue=messagePtr->baseKillValue;
		netGameData.useDynamicScoring=messagePtr->useDynamicScoring;
		netGameData.useCharacterKillValues=messagePtr->useCharacterKillValues;

		netGameData.sendDecals=messagePtr->sendDecals;

		netGameData.gameSpeed=messagePtr->gameSpeed;

		netGameData.disableFriendlyFire=messagePtr->disableFriendlyFire;
		netGameData.fallingDamage=messagePtr->fallingDamage;
		netGameData.pistolInfiniteAmmo=messagePtr->pistolInfiniteAmmo;
		netGameData.specialistPistols=messagePtr->specialistPistols;

		if(netGameData.needGameDescription)
		{
			/*We were waiting for the game description , best make sure that our player id appears
			int the player list*/
			if(PlayerIdInPlayerList(AVPDPNetID)!=NET_IDNOTINPLAYERLIST)
			{			
				/*we now have the game description, so we can stop waiting if we were 
				trying to join*/
				netGameData.needGameDescription=0;
				/*
				make sure our time scale is set correctly
				(Only set it the once , so that it can be overridden for debugging purposes)
				*/
				switch(netGameData.gameSpeed)
				{
					case NETGAMESPEED_70PERCENT :
						TimeScale=(ONE_FIXED*70)/100;
						break;
					
					case NETGAMESPEED_80PERCENT :
						TimeScale=(ONE_FIXED*80)/100;
						break;
					
					case NETGAMESPEED_90PERCENT :
						TimeScale=(ONE_FIXED*90)/100;
						break;
					
					case NETGAMESPEED_100PERCENT :
						TimeScale=(ONE_FIXED*100)/100;
						break;

				}
				
			}
		}
		
		netGameData.allowSmartgun=messagePtr->allowSmartgun;
		netGameData.allowFlamer=messagePtr->allowFlamer;
		netGameData.allowSadar=messagePtr->allowSadar;
		netGameData.allowGrenadeLauncher=messagePtr->allowGrenadeLauncher;
		netGameData.allowMinigun=messagePtr->allowMinigun;
		netGameData.allowDisc=messagePtr->allowDisc;
		netGameData.allowPistol=messagePtr->allowPistol;
		netGameData.allowPlasmaCaster=messagePtr->allowPlasmaCaster;
		netGameData.allowSpeargun=messagePtr->allowSpeargun;
		netGameData.allowMedicomp=messagePtr->allowMedicomp;
		netGameData.allowSmartDisc=messagePtr->allowSmartDisc;
		netGameData.allowPistols=messagePtr->allowPistols;
		
		netGameData.maxPredator=messagePtr->maxPredator;
		netGameData.maxAlien=messagePtr->maxAlien;
		netGameData.maxMarine=messagePtr->maxMarine;

		netGameData.maxMarineGeneral=messagePtr->maxMarineGeneral;
		netGameData.maxMarinePulseRifle=messagePtr->maxMarinePulseRifle;
		netGameData.maxMarineSmartgun=messagePtr->maxMarineSmartgun;
		netGameData.maxMarineFlamer=messagePtr->maxMarineFlamer;
		netGameData.maxMarineSadar=messagePtr->maxMarineSadar;
		netGameData.maxMarineGrenade=messagePtr->maxMarineGrenade;
		netGameData.maxMarineMinigun=messagePtr->maxMarineMinigun;
		netGameData.maxMarineSmartDisc=messagePtr->maxMarineSmartDisc;
		netGameData.maxMarinePistols=messagePtr->maxMarinePistols;

		netGameData.useSharedLives=messagePtr->useSharedLives;
		netGameData.maxLives=messagePtr->maxLives;
		netGameData.numDeaths[0]=messagePtr->numDeaths[0];
		netGameData.numDeaths[1]=messagePtr->numDeaths[1];
		netGameData.numDeaths[2]=messagePtr->numDeaths[2];


		netGameData.timeForRespawn=messagePtr->timeForRespawn;
		netGameData.pointsForRespawn=messagePtr->pointsForRespawn;

		{
			//check to if the host's elapsed time is
			//significantly different from our elapsed time value
			int receivedTime=messagePtr->GameTimeElapsed;
			int diff;
			receivedTime<<=16;

			diff=netGameData.GameTimeElapsed-receivedTime;

			if(diff<-ONE_FIXED || diff>ONE_FIXED || netGameData.myGameState==NGS_EndGameScreen)
			{
				//best take the host's value
				netGameData.GameTimeElapsed=receivedTime;
			}
					
		}


	}

	if(messagePtr->endGame)
	{
		if(netGameData.myGameState==NGS_Playing)
		{
			//we must have missed the end game message
			netGameData.myGameState=NGS_EndGameScreen;
		}
	}
	else
	{
		if(netGameData.myGameState==NGS_EndGameScreen)
		{
			//must have missed message to restart game
			//therefore probably better restart now
			RestartNetworkGame(0);

		}
	}
}

static void ProcessNetMsg_PlayerDescription(NETMESSAGE_PLAYERDESCRIPTION *messagePtr, DPID senderId)
{	
	/* only act on this if we're the host and in start-up */
	if(AvP.Network!=I_Host) return;

	/* find the player and fill out their details from the message */
	{ 
		int id = PlayerIdInPlayerList(senderId);
		if(id==NET_IDNOTINPLAYERLIST)
		{
			/* player does not seem to be in the player list, so ignore it */
			return;
		}
		netGameData.playerData[id].characterType = (NETGAME_CHARACTERTYPE)messagePtr->characterType;
		netGameData.playerData[id].characterSubType = (NETGAME_SPECIALISTCHARACTERTYPE)messagePtr->characterSubType;

		if (netGameData.myGameState==NGS_Playing)
		{
			if(messagePtr->startFlag && (netGameData.playerData[id].startFlag != messagePtr->startFlag) )
				Inform_PlayerHasJoined(netGameData.playerData[id].playerId);
		}
		netGameData.playerData[id].startFlag = messagePtr->startFlag;

	}
}

static void ProcessNetMsg_StartGame(void)
{	
	/* only act on this if we're a peer and in start-up */
	return;
	if(AvP.Network!=I_Peer) return;
	if(netGameData.myGameState!=NGS_Joining) return;	

	/* switch our game state... if we are in the player list, switch to start, otherwise
	switch to error */
	if(PlayerIdInPlayerList(AVPDPNetID)!=NET_IDNOTINPLAYERLIST)
		netGameData.myGameState=NGS_Playing;
	else 
	{
		TransmitPlayerLeavingNetMsg();
		netGameData.myGameState=NGS_Error_HostLost;
		AvP.MainLoopRunning = 0;
	}
}

static void ProcessNetMsg_PlayerState(NETMESSAGE_PLAYERSTATE *messagePtr, DPID senderId)
{
	VECTORCH position;
	int playerIndex;
	STRATEGYBLOCK *sbPtr;
#if 0
	/* state check: if we're in startup and we've received this message from the host, we
	should go into an error state: */
	if((netGameData.myGameState==NGS_Joining)&&(messagePtr->IAmHost)) 
	{
		TransmitPlayerLeavingNetMsg();
		netGameData.myGameState=NGS_Error_HostLost;	
		AvP.MainLoopRunning = 0;
	}
#endif


	position.vx = messagePtr->xPos;
	position.vy = messagePtr->yPos;
	position.vz = messagePtr->zPos;
	{
		//recored the players position , even if we aren't currntly playing
		int playerIndex=PlayerIdInPlayerList(senderId);
		if(playerIndex!=NET_IDNOTINPLAYERLIST)
		{
			netGameData.playerData[playerIndex].lastKnownPosition=position;
		}
	}
	
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	playerIndex = PlayerIdInPlayerList(senderId);

	/* KJL 14:47:22 06/04/98 - we don't seem to know about this person yet... ignore them */
	if (playerIndex==NET_IDNOTINPLAYERLIST) return;

	sbPtr = FindGhost(senderId, GHOST_PLAYEROBJECTID);

	//record whether the player is in the land of the living
	netGameData.playerData[playerIndex].playerAlive=messagePtr->IAmAlive;	
	netGameData.playerData[playerIndex].playerHasLives=messagePtr->IHaveLifeLeft;	
	
	//check the player type
	//the value in the netgamedata should be set to next character type
	netGameData.playerData[playerIndex].characterType=messagePtr->nextCharacterType;
	netGameData.playerData[playerIndex].characterSubType=messagePtr->characterSubType;
	if(sbPtr)
	{
		NETGHOSTDATABLOCK *ghostData;
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(ghostData);
		
		//here we need to use the current character type
		switch(messagePtr->characterType)
		{
			case NGCT_Marine :
				if(ghostData->type!=I_BehaviourMarinePlayer)
				{
					sbPtr->SBflags.please_destroy_me=1;
					return;
				}
				break;
			case NGCT_Predator :
				if(ghostData->type!=I_BehaviourPredatorPlayer)
				{
					sbPtr->SBflags.please_destroy_me=1;
					return;
				}
				break;
			case NGCT_Alien :
				if(ghostData->type!=I_BehaviourAlienPlayer)
				{
					sbPtr->SBflags.please_destroy_me=1;
					return;
				}
				break;
		}

	}

	if(!MultiplayerObservedPlayer)
	{
		if(sbPtr && sbPtr->SBdptr)
		{
			//make sure the model is visivle
			sbPtr->SBdptr->ObFlags&=~ObFlag_NotVis;
		}
	}
	
	{
		EULER orientation;
		int sequence;
		int weapon;
		int firingPrimary;
		int firingSecondary;
		
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);			
		sequence = (int)messagePtr->sequence;
		weapon = (int)messagePtr->currentWeapon;
		firingPrimary = (int)messagePtr->IAmFiringPrimary;
		firingSecondary = (int)messagePtr->IAmFiringSecondary;

		//ReleasePrintDebuggingText("Primary %d Secondary %d\n",firingPrimary,firingSecondary);
		
		if(!sbPtr)
		{

			/* If we are not a dead alien then we should have a ghost */		
  //			if(!(((!(messagePtr->IAmAlive)))&&(netGameData.playerData[playerIndex].characterType==NGCT_Alien)))
			if (messagePtr->IAmAlive)
			{
				{
					AVP_BEHAVIOUR_TYPE type;
					if(messagePtr->characterType==NGCT_Marine) type = I_BehaviourMarinePlayer;
					else if(messagePtr->characterType==NGCT_Alien) type = I_BehaviourAlienPlayer;
					else type = I_BehaviourPredatorPlayer;
					sbPtr = CreateNetGhost(senderId,GHOST_PLAYEROBJECTID,&position,&orientation,type,IOT_Non,0);
					if(sbPtr) 
					{
						HandleWeaponElevation(sbPtr,(int)messagePtr->Elevation,weapon);
						//don't draw muzzle flash if observing from this player
						if(MultiplayerObservedPlayer!=senderId) {
							HandleGhostGunFlashEffect(sbPtr, messagePtr->IHaveAMuzzleFlash);
						} else {
							HandleGhostGunFlashEffect(sbPtr, 0);
						}
						HandlePlayerGhostWeaponSound(sbPtr,weapon,firingPrimary,firingSecondary);
						MaintainGhostCloakingStatus(sbPtr,(int)messagePtr->CloakingEffectiveness);
						MaintainGhostFireStatus(sbPtr,(int)messagePtr->IAmOnFire);
					}
				}

			}		
		}
		else
		{

			if(!(((!(messagePtr->IAmAlive)))&&(netGameData.playerData[playerIndex].characterType==NGCT_Alien)))
			{
				/* We are not a dead alien */
				HandleWeaponElevation(sbPtr,(int)messagePtr->Elevation,weapon);
				UpdateGhost(sbPtr,&position,&orientation,sequence,messagePtr->Special);
				//don't draw muzzle flash if observing from this player
				if(MultiplayerObservedPlayer!=senderId) {
					HandleGhostGunFlashEffect(sbPtr, messagePtr->IHaveAMuzzleFlash);
				} else {
					HandleGhostGunFlashEffect(sbPtr, 0);
				}
				HandlePlayerGhostWeaponSound(sbPtr,weapon,firingPrimary,firingSecondary);
				MaintainGhostCloakingStatus(sbPtr,(int)messagePtr->CloakingEffectiveness);
				MaintainGhostFireStatus(sbPtr,(int)messagePtr->IAmOnFire);
				/* Now, more disc. */

				if (messagePtr->IHaveADisk) {
				
					NETGHOSTDATABLOCK *ghostData;
					SECTION_DATA *disc;
					/* Find the thrower's ghost, and add his disc...  */
					
					ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
					GLOBALASSERT(ghostData);
					GLOBALASSERT(ghostData->type==I_BehaviourPredatorPlayer);
					disc=GetThisSectionData(ghostData->HModelController.section_data,"disk");
					if (disc) {
						disc->flags&=~section_data_notreal;
					}
				}
			}
			else
			{
				/* We are a dead alien with a ghost */
				RemoveGhost(sbPtr);
				return;
			}
		}
		
		if(sbPtr && messagePtr->IAmAlive)
		{
			NETGHOSTDATABLOCK *ghostData;
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			
			ghostData->invulnerable=messagePtr->IAmInvulnerable;
			
			#if EXTRAPOLATION_TEST
			{
				VECTORCH velocity,diff;
				int playerTimer=netGameData.playerData[playerIndex].timer;

				velocity.vx=messagePtr->velocity_x*100;
				velocity.vy=messagePtr->velocity_y*100;
				velocity.vz=messagePtr->velocity_z*100;
				
				diff=ghostData->velocity;
				SubVector(&velocity,&diff);
				
				if(Approximate3dMagnitude(&diff)>1000)
				{
					//change in velocity , so reset extrapolation timer
					ghostData->extrapTimer=-ONE_FIXED;
				}

				ghostData->velocity=velocity;
				ghostData->extrapTimerLast=0;
				if(playerTimer>=ghostData->lastTimeRead)
				{
					ghostData->extrapTimer-=(playerTimer-ghostData->lastTimeRead);
				}
				else
				{
					ghostData->extrapTimer=-ONE_FIXED;
				}
				ghostData->lastTimeRead=playerTimer;
				
			}
		

			if(ghostData->type==I_BehaviourAlienPlayer)
			{
				sbPtr->DynPtr->UseStandardGravity=messagePtr->standard_gravity;
				if(!sbPtr->DynPtr->UseStandardGravity)
				{
					if(sbPtr->DynPtr->GravityDirection.vy==ONE_FIXED)
					{
						MATRIXCH mat;
						//alien is crawling , so we need to get an appropriate gravity direction
						CreateEulerMatrix(&orientation,&mat);
						sbPtr->DynPtr->GravityDirection.vx=mat.mat12;
						sbPtr->DynPtr->GravityDirection.vy=mat.mat22;
						sbPtr->DynPtr->GravityDirection.vz=mat.mat32;
					}

					sbPtr->DynPtr->LinImpulse.vx=0;
					sbPtr->DynPtr->LinImpulse.vy=0;
					sbPtr->DynPtr->LinImpulse.vz=0;

					sbPtr->DynPtr->ToppleForce=TOPPLE_FORCE_ALIEN;
				}
			}
			#endif
			
			
			if(messagePtr->scream!=31)
			{
				//this character is screaming
				switch(messagePtr->characterType)
				{
					case NGCT_Marine :
						PlayMarineScream(0,(SOUND_CATERGORY)messagePtr->scream,0,&ghostData->SoundHandle2,&position);
						break;
					
					case NGCT_Alien :
						PlayAlienSound(0,(ALIEN_SOUND_CATEGORY)messagePtr->scream,0,&ghostData->SoundHandle2,&position);
						break;
					
					case NGCT_Predator :
						if ((PREDATOR_SOUND_CATEGORY)messagePtr->scream==PSC_Medicomp_Special) {
							Sound_Play(SID_PRED_NEWROAR,"de",&position,&ghostData->SoundHandle2);
						} else {
							PlayPredatorSound(0,(PREDATOR_SOUND_CATEGORY)messagePtr->scream,0,&ghostData->SoundHandle2,&position);
						}
						break;
				}
			}
			
			/* Landing noise. */
			if (messagePtr->landingNoise) {
				switch(messagePtr->characterType) {
					case NGCT_Marine :
		   				Sound_Play(SID_MARINE_SMALLLANDING,"d",&position);
						break;
					case NGCT_Alien :
						/* No sound for aliens. */
						break;
					case NGCT_Predator :
		   				Sound_Play(SID_PRED_SMALLLANDING,"d",&position);
						break;
				}
			}

			//are we currently following this player's movements
			if(MultiplayerObservedPlayer)
			{
				if(MultiplayerObservedPlayer==senderId)
				{
					PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
					playerStatusPtr->ViewPanX=messagePtr->Elevation;
					Player->ObStrategyBlock->DynPtr->Position=position;
					Player->ObStrategyBlock->DynPtr->PrevPosition=position;
					
					Player->ObStrategyBlock->DynPtr->OrientEuler = orientation;
					CreateEulerMatrix(&Player->ObStrategyBlock->DynPtr->OrientEuler,&Player->ObStrategyBlock->DynPtr->OrientMat);
					TransposeMatrixCH(&Player->ObStrategyBlock->DynPtr->OrientMat);

					if(messagePtr->IAmCrouched)
					{
						PlayerStatusPtr->ShapeState=PMph_Crouching;
					}
					else
					{
						PlayerStatusPtr->ShapeState=PMph_Standing;
					}
					
					//don't draw the player we're observing
					if(sbPtr && sbPtr->SBdptr)
					{
						sbPtr->SBdptr->ObFlags|=ObFlag_NotVis;
					}
				}
			}
		}
	}
}
static void ProcessNetMsg_PlayerState_Minimal(NETMESSAGE_PLAYERSTATE_MINIMAL *messagePtr, DPID senderId,BOOL orientation)
{
	int playerIndex;
	STRATEGYBLOCK *sbPtr;
	
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	playerIndex = PlayerIdInPlayerList(senderId);

	/* KJL 14:47:22 06/04/98 - we don't seem to know about this person yet... ignore them */
	if (playerIndex==NET_IDNOTINPLAYERLIST) return;

	sbPtr = FindGhost(senderId, GHOST_PLAYEROBJECTID);
		
	
	//record whether the player is in the land of the living
	netGameData.playerData[playerIndex].playerAlive=messagePtr->IAmAlive;	
	netGameData.playerData[playerIndex].playerHasLives=messagePtr->IHaveLifeLeft;	
	
	if(!sbPtr)
	{
		//if we don't have a ghost for this player , wait for a full player state message
		return;
	}
	

	if(!MultiplayerObservedPlayer)
	{
		if(sbPtr && sbPtr->SBdptr)
		{
			//make sure the model is visivle
			sbPtr->SBdptr->ObFlags&=~ObFlag_NotVis;
		}
	}
	
	{
		int firingPrimary;
		int firingSecondary;
		
		firingPrimary = (int)messagePtr->IAmFiringPrimary;
		firingSecondary = (int)messagePtr->IAmFiringSecondary;
		
		

		if(!(((!(messagePtr->IAmAlive)))&&(netGameData.playerData[playerIndex].characterType==NGCT_Alien)))
		{
			NETGHOSTDATABLOCK *ghostData;
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			GLOBALASSERT(ghostData);
			
			if(orientation)
			{
				NETMESSAGE_PLAYERSTATE_MEDIUM* mediumMessage=(NETMESSAGE_PLAYERSTATE_MEDIUM*) messagePtr;
				EULER orientation;
				orientation.EulerX = (mediumMessage->xOrient<<NET_EULERSCALESHIFT);
				orientation.EulerY = (mediumMessage->yOrient<<NET_EULERSCALESHIFT);
				orientation.EulerZ = (mediumMessage->zOrient<<NET_EULERSCALESHIFT);			

				UpdateGhost(sbPtr,&sbPtr->DynPtr->Position,&orientation,-1,messagePtr->Special);

			}
			
			/* We are not a dead alien */
			HandleWeaponElevation(sbPtr,(int)messagePtr->Elevation,ghostData->CurrentWeapon);
			//don't draw muzzle flash if observing from this player
			if(MultiplayerObservedPlayer!=senderId)
				HandleGhostGunFlashEffect(sbPtr, messagePtr->IHaveAMuzzleFlash);
			else
				HandleGhostGunFlashEffect(sbPtr, 0);
			MaintainGhostCloakingStatus(sbPtr,(int)messagePtr->CloakingEffectiveness<<8);
			MaintainGhostFireStatus(sbPtr,(int)messagePtr->IAmOnFire);
			/* Now, more disc. */

			if (messagePtr->IHaveADisk) {
			
				SECTION_DATA *disc;
				/* Find the thrower's ghost, and add his disc...  */
				
				GLOBALASSERT(ghostData->type==I_BehaviourPredatorPlayer);
				disc=GetThisSectionData(ghostData->HModelController.section_data,"disk");
				if (disc) {
					disc->flags&=~section_data_notreal;
				}
			}
		}
		else
		{
			/* We are a dead alien with a ghost */
			RemoveGhost(sbPtr);
			return;
		}
		
		
		if(sbPtr && messagePtr->IAmAlive)
		{
			NETGHOSTDATABLOCK *ghostData;
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			
			//are we currently following this player's movements
			if(MultiplayerObservedPlayer)
			{
				if(MultiplayerObservedPlayer==senderId)
				{
					PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
					playerStatusPtr->ViewPanX=messagePtr->Elevation;
				
					//don't draw the player we're observing
					if(sbPtr && sbPtr->SBdptr)
					{
						sbPtr->SBdptr->ObFlags|=ObFlag_NotVis;
					}
				}
			}
		}
	}
}

static void ProcessNetMsg_FrameTimer(unsigned short frame_time,DPID senderId)
{
	int senderPlayerIndex;
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	senderPlayerIndex = PlayerIdInPlayerList(senderId);
	if(senderPlayerIndex==NET_IDNOTINPLAYERLIST) return;

	netGameData.playerData[senderPlayerIndex].timer+=frame_time;
	

}

static void ProcessNetMsg_PlayerKilled(NETMESSAGE_PLAYERKILLED *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int senderPlayerIndex;

	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	/* if this player is not in the play list, messages are probably out of order,
	so just ignore it... */
	senderPlayerIndex = PlayerIdInPlayerList(senderId);
	if(senderPlayerIndex==NET_IDNOTINPLAYERLIST) return;
	
	/* find the ghost for this player */
	sbPtr = FindGhost(senderId, GHOST_PLAYEROBJECTID);
	if(!sbPtr)
	{
		/* we don't have a ghost for this player, which is odd, and implies that messages 
		have got dis-ordered... best just ignore it then */
		return;
	}

	/* we have a ghost for this player: remove it if it's an alien */
//	if(netGameData.playerData[senderPlayerIndex].characterType==NGCT_Alien)
	//RemoveGhost(sbPtr);	
	KillGhost(sbPtr,messagePtr->objectId);

	Inform_PlayerHasDied(messagePtr->killerId, senderId,messagePtr->killerType,messagePtr->weaponIcon);

	/* now attempt to update the scores */
	if(AvP.Network==I_Host)
	{
		UpdateNetworkGameScores(senderId, messagePtr->killerId,messagePtr->myType,messagePtr->killerType);
	}

	

	/* do some sound... */
	LOCALASSERT(sbPtr->DynPtr);
	switch(netGameData.playerData[senderPlayerIndex].characterType)
	{
		case(NGCT_Alien):
		{
			PlayAlienSound(0,ASC_Scream_Dying,0,NULL,&(sbPtr->DynPtr->Position));
			break;
		}
		case(NGCT_Marine):
		{
			PlayMarineScream(0,SC_Death,0,NULL,&(sbPtr->DynPtr->Position));
			break;
		}
		case(NGCT_Predator):
		{
			PlayPredatorSound(0,PSC_Scream_Dying,0,NULL,&(sbPtr->DynPtr->Position));
			break;
		}	
		default:
		{
			LOCALASSERT(1==0);
			break;
		}
	}
}

static void ProcessNetMsg_PlayerDeathAnim(NETMESSAGE_CORPSEDEATHANIM *messagePtr,DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	/* find the ghost for this player */
	sbPtr = FindGhost(senderId, messagePtr->objectId);
	if(!sbPtr)
	{
		/* we don't have a ghost for this player, which is odd, and implies that messages 
		have got dis-ordered... best just ignore it then */
		return;
	}

	//set the death animation for this player
	ApplyGhostCorpseDeathAnim(sbPtr,messagePtr->deathId);

}

static void ProcessNetMsg_AllGameScores(NETMESSAGE_ALLGAMESCORES *messagePtr)
{
	/* should only get this if we're not the host */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* fill in the game scores from the message */
	{	
		int i,j;
		
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			for(j=0;j<NET_MAXPLAYERS;j++)
			{
				netGameData.playerData[i].playerFrags[j] = messagePtr->playerFrags[i][j];
			}
			netGameData.playerData[i].playerScore = messagePtr->playerScores[i];
			netGameData.playerData[i].playerScoreAgainst = messagePtr->playerScoresAgainst[i];

			for(j=0;j<3;j++)
			{
				netGameData.playerData[i].aliensKilled[j]=messagePtr->aliensKilled[i][j];
			}

			netGameData.playerData[i].deathsFromAI = messagePtr->deathsFromAI[i];
		}
	}
	
	netGameData.myGameState=NGS_EndGameScreen;
}

static void ProcessNetMsg_PlayerScores(NETMESSAGE_PLAYERSCORES *messagePtr)
{
	int playerId;
	/* should only get this if we're not the host */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* find the player... */
	playerId = (int)messagePtr->playerId;

	/* fill in the player's scores from the message */
	{	
		int i;
		
		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			netGameData.playerData[playerId].playerFrags[i] = messagePtr->playerFrags[i];
		}
		netGameData.playerData[playerId].playerScore = messagePtr->playerScore;
		netGameData.playerData[playerId].playerScoreAgainst = messagePtr->playerScoreAgainst;
		
		for(i=0;i<3;i++)
		{
			netGameData.playerData[playerId].aliensKilled[i]=messagePtr->aliensKilled[i];
		}
		netGameData.playerData[playerId].deathsFromAI = messagePtr->deathsFromAI;
	}
}

static void ProcessNetMsg_ScoreChange(NETMESSAGE_SCORECHANGE *messagePtr)
{
	/* should only get this if we're not the host */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	if(messagePtr->killerIndex == NET_MAXPLAYERS)
	{
		//killed by ai
		netGameData.playerData[messagePtr->victimIndex].deathsFromAI=messagePtr->fragCount;
	}
	else
	{
		netGameData.playerData[messagePtr->killerIndex].playerFrags[messagePtr->victimIndex]=messagePtr->fragCount;
		netGameData.playerData[messagePtr->killerIndex].playerScore=messagePtr->killerScoreFor;
	}
	netGameData.playerData[messagePtr->victimIndex].playerScoreAgainst=messagePtr->victimScoreAgainst;
}

static void ProcessNetMsg_SpeciesScores(NETMESSAGE_SPECIESSCORES *messagePtr)
{
	int i;
	/* should only get this if we're not the host */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	for(i=0;i<3;i++)
	{
		netGameData.teamScores[i]=messagePtr->teamScores[i]; 
	}

}

static void ProcessNetMsg_LocalRicochet(NETMESSAGE_LOCALRICOCHET *messagePtr)
{
}

static void ProcessNetMsg_LocalObjectState(NETMESSAGE_LOBSTATE *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* get the object id from the message */
	objectId = (int)messagePtr->objectId;
	
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* we don't seem to have a ghost for this object, so create one */
		VECTORCH position;
		EULER orientation;
		AVP_BEHAVIOUR_TYPE type;

		position.vx = messagePtr->xPos;
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		position.vy = messagePtr->yPos;
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		position.vz = messagePtr->zPos;
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);
		type = (AVP_BEHAVIOUR_TYPE)messagePtr->type;

		if (type==I_BehaviourNetCorpse) {
			/* This *should* never happen... */
			textprint("IGNORED A CREATE CORPSE COMMAND.\n");
			return;
		}

		/* NB there is no sequence required for local objects, so just pass zero */
		sbPtr = CreateNetGhost(senderId,objectId,&position,&orientation,type,messagePtr->IOType,messagePtr->subtype);
		if (type==I_BehaviourPredatorDisc_SeekTrack) {
			
			NETGHOSTDATABLOCK *ghostData;
			STRATEGYBLOCK *sbPtr2;
			SECTION_DATA *disc;
			int playerIndex;
			/* Find the thrower's ghost, and rip off his disc :-) */
			
			playerIndex = PlayerIdInPlayerList(senderId);
			if (playerIndex==NET_IDNOTINPLAYERLIST) {
				return;
			}
			sbPtr2 = FindGhost(senderId, GHOST_PLAYEROBJECTID);
			if (!sbPtr2) {
				return;
			}
			/* Got 'em. */
			ghostData = (NETGHOSTDATABLOCK *)sbPtr2->SBdataptr;
			GLOBALASSERT(ghostData);
			disc=GetThisSectionData(ghostData->HModelController.section_data,"disk");
			if (!disc) {
				return;
			}
			disc->flags|=section_data_notreal;
		}
	}
	else
	{
		/* update the ghost... */
		VECTORCH position;
		EULER orientation;

		position.vx = messagePtr->xPos;
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		position.vy = messagePtr->yPos;
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		position.vz = messagePtr->zPos;
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);

		/* NB don't need to update the object type */
		
		{
			NETGHOSTDATABLOCK *ghostData;

			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		
			if(messagePtr->event_flag)
			{
				if (ghostData->type==I_BehaviourPredatorDisc_SeekTrack) 
				{
					if (messagePtr->event_flag==1)
					{ 
						/* Convert! */
						Convert_DiscGhost_To_PickupGhost(sbPtr);
						//return;
					}
					else if(messagePtr->event_flag==2)
					{
						//disc has bounced off a wall , so play appropriate sound
						Sound_Play(SID_PREDATOR_DISK_HITTING_WALL,"dp",&position,((FastRandom()&511)-255));
					}
				}
				else if(ghostData->type==I_BehaviourFrisbee)
				{
					if(messagePtr->event_flag==2)
					{
						//disc has bounced off a wall , so play appropriate sound
						Sound_Play(SID_ED_SKEETERDISC_HITWALL,"dp",&position,((FastRandom()&511)-255));
					}
				}
				else if(ghostData->type==I_BehaviourFlareGrenade)
				{
					//flare has hit wall , so start the flare sound
					if(ghostData->SoundHandle==SOUND_NOACTIVEINDEX)
					{
						Sound_Play(SID_BURNING_FLARE,"dle",&position,&ghostData->SoundHandle);
					}
				}
				else if ((ghostData->type==I_BehaviourGrenade)||(ghostData->type==I_BehaviourClusterGrenade))
				{
					/* Bounce sound. */
					Sound_Play(SID_GRENADE_BOUNCE,"dp",&position,((FastRandom()&511)-255));
				}
			}
			/* NB there is no sequence required for local objects, so just pass zero */
			UpdateGhost(sbPtr,&position,&orientation,0,0);
		}
	}
}

static int GetSizeOfLocalObjectDamagedMessage(char *messagePtr)
{
	int size=sizeof(NETMESSAGE_LOBDAMAGED_HEADER);
	NETMESSAGE_LOBDAMAGED_HEADER *messageHeader = (NETMESSAGE_LOBDAMAGED_HEADER*) messagePtr;

	if(messageHeader->damageProfile) size+=sizeof(NETMESSAGE_DAMAGE_PROFILE);	
	if(messageHeader->multiple) size+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);	
	if(messageHeader->sectionID) size+=sizeof(NETMESSAGE_DAMAGE_SECTION);	
	if(messageHeader->delta_seq) size+=sizeof(NETMESSAGE_DAMAGE_DELTA);	
	if(messageHeader->direction) size+=sizeof(NETMESSAGE_DAMAGE_DIRECTION);	

	return size;
}

static void ProcessNetMsg_LocalObjectDamaged(char *messagePtr, DPID senderId)
{
	NETMESSAGE_LOBDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;
	NETMESSAGE_DAMAGE_SECTION *messageSection=0;
	NETMESSAGE_DAMAGE_DELTA *messageDelta=0;
	NETMESSAGE_DAMAGE_DIRECTION *messageDirection=0;

	if(netGameData.myGameState!=NGS_Playing) return;
	
	
	messageHeader=(NETMESSAGE_LOBDAMAGED_HEADER*) messagePtr;
	messagePtr+=sizeof(NETMESSAGE_LOBDAMAGED_HEADER);

	//find out which elements of the damage message have been sent
	if(messageHeader->damageProfile)
	{
		messageProfile=(NETMESSAGE_DAMAGE_PROFILE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_PROFILE);
	}
	if(messageHeader->multiple)
	{
		messageMultiple=(NETMESSAGE_DAMAGE_MULTIPLE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);
	}
	if(messageHeader->sectionID)
	{
		messageSection=(NETMESSAGE_DAMAGE_SECTION*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_SECTION);
	}
	if(messageHeader->delta_seq)
	{
		messageDelta=(NETMESSAGE_DAMAGE_DELTA*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_DELTA);
	}
	if(messageHeader->direction)
	{
		messageDirection=(NETMESSAGE_DAMAGE_DIRECTION*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_DIRECTION);
	}

	/* This message is for the player who owns the object, so first check 
	if the message is meant for us */
	if(messageHeader->playerId != AVPDPNetID)
	{
		//however we may need to play a delta sequence on the ghost
		if(messageDelta)
		{
			if(messageDelta->Delta_Sequence>0 && messageDelta->Delta_Sub_Sequence>0)
			{
				STRATEGYBLOCK *sbPtr;
				sbPtr=FindGhost(messageHeader->playerId,(int)messageHeader->objectId);
				if(sbPtr)
				{
					PlayHitDeltaOnGhost(sbPtr,messageDelta->Delta_Sequence,messageDelta->Delta_Sub_Sequence);
				}
			}
		}
		return;
	}

	/* next we have to find this object in our strategyblock list */
	{
		int objectId; 
		STRATEGYBLOCK *sbPtr;
		DAMAGE_PROFILE damage;
		SECTION_DATA *section_data;
		HMODELCONTROLLER *controller;
		int multiple;
		VECTORCH incoming;
		VECTORCH direction;
		VECTORCH* incoming_ptr=0;

		objectId = (int)messageHeader->objectId;
		sbPtr = FindObjectFromNetIndex(objectId);
		


		
		/* we set this global to remember the id of the player who sent this damage message,
		so that if the player is killed, we know the id of the killer. this is a bit of
		a nasty hack, but means that we don't have to make any changes to the core damage functions */
		LOCALASSERT(myNetworkKillerId==NULL || myNetworkKillerId ==AVPDPNetID);
		/*
		Don't bother setting the killer id for molotov damage. This is because the only source of this damage
		is explosions from flamethrowers. This damage will always come from the net host , and we don't want to credit the 
		host for all kills caused by this damage.
		
		Similarly all things that cause falling damage should count as suicide (in particular platform lifts)
		*/
		if(messageHeader->ammo_id!=AMMO_MOLOTOV && messageHeader->ammo_id!=AMMO_FALLING_POSTMAX)
		{
			myNetworkKillerId = senderId;		
		}
		/* check if we have found an sb: if not the object has probably been 
		destroyed already, so just ignore it */
		if(sbPtr) 
		{
			//fill out damage profile
			damage.Id=messageHeader->ammo_id;
			if(messageProfile)
			{
				damage.Impact = messageProfile->Impact;
				damage.Cutting = messageProfile->Cutting;
				damage.Penetrative = messageProfile->Penetrative;
				damage.Fire = messageProfile->Fire;
				damage.Electrical = messageProfile->Electrical;
				damage.Acid = messageProfile->Acid;

				damage.ExplosivePower=messageProfile->ExplosivePower;
				damage.Slicing=messageProfile->Slicing;
				damage.ProduceBlood=messageProfile->ProduceBlood;
				damage.ForceBoom=messageProfile->ForceBoom;

				damage.BlowUpSections=messageProfile->BlowUpSections;
				damage.Special=messageProfile->Special;
				damage.MakeExitWounds=messageProfile->MakeExitWounds;
			}
			else
			{
				if(damage.Id==AMMO_FLECHETTE_POSTMAX)
				{
					extern DAMAGE_PROFILE FlechetteDamage;
					damage=FlechetteDamage;
				}
				else
				{
					GLOBALASSERT(damage.Id>AMMO_NONE && damage.Id<MAX_NO_OF_AMMO_TEMPLATES);
					damage=TemplateAmmo[damage.Id].MaxDamage[AvP.Difficulty];
				}
			}

			if(messageMultiple)
			{
				multiple = messageMultiple->multiple;
			}
			else
			{
				multiple = ONE_FIXED;	
			}
		
			section_data=NULL;
			controller=NULL;

			//get the direction vector if there is one
			if(sbPtr->DynPtr && messageDirection)
			{
				if(messageDirection->direction_x ||
				   messageDirection->direction_y ||
				   messageDirection->direction_z)
				{
					MATRIXCH mat=sbPtr->DynPtr->OrientMat;
					TransposeMatrixCH(&mat);

					//extract the direction vector
					incoming.vx=messageDirection->direction_x;
					incoming.vy=messageDirection->direction_y;
					incoming.vz=messageDirection->direction_z;
					//normalise it
					Normalise(&incoming);
					direction=incoming;
					
					//and rotate it from world space to the object's local space
					RotateVector(&incoming,&mat);

					//set the incoming pointer 
					incoming_ptr=&incoming;	
				
				}	

			}
			/*Record the id of the hit body part in a global variable.
			This way , if this blow kill the player , we know which body part has been hit*/
			if(messageSection)
			{
				MyHitBodyPartId=messageSection->SectionID;
			}
			else
			{
				MyHitBodyPartId=-1;
			}

			if (messageSection && messageSection->SectionID!=-1) {
				/* Hmm. */
				if (sbPtr->I_SBtype==I_BehaviourAlien) {
					/* Only allowed for aliens, right now. */
					ALIEN_STATUS_BLOCK *alienStatusPtr=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);

					GLOBALASSERT(alienStatusPtr);
					
					controller=&alienStatusPtr->HModelController;
					section_data=GetThisSectionData_FromID(alienStatusPtr->HModelController.section_data,
						(int)messageSection->SectionID);
				}
				else if (sbPtr->I_SBtype==I_BehaviourNetCorpse)
				{
					NETCORPSEDATABLOCK *corpseData = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;

					GLOBALASSERT(corpseData);
					
					controller=&corpseData->HModelController;
					section_data=GetThisSectionData_FromID(corpseData->HModelController.section_data,
						(int)messageSection->SectionID);
				}
			}
			
			if (section_data) 
			{
				DISPLAYBLOCK *fragged_section=0;
				fragged_section=CauseDamageToHModel(controller,sbPtr,(&damage),
									multiple,section_data,incoming_ptr,NULL,0);
				
				if(fragged_section && damage.Id==AMMO_PRED_RIFLE && incoming_ptr)
				{
					//a speargun has fragged off a body part , so we need to create a spear
					CreateSpearPossiblyWithFragment(fragged_section,&fragged_section->ObWorld,&direction);
				}
				
			} 
			else 
			{
				CauseDamageToObject(sbPtr, (&damage), multiple,incoming_ptr);
			}
		}
		myNetworkKillerId = AVPDPNetID;
		MyHitBodyPartId=-1;
	}
	

	
}


static void ProcessNetMsg_LocalObjectDestroyed_Request(NETMESSAGE_LOBDESTROYED_REQUEST *messagePtr, DPID senderId)
{
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* This message is for the player who owns the object, so first check 
	if the message is meant for us */
	if(messagePtr->playerId != AVPDPNetID) return;

	/* next we have to find this object in our strategyblock list */
	{
		int objectId; 
		STRATEGYBLOCK *sbPtr;

		objectId = (int)messagePtr->objectId;
		sbPtr = FindObjectFromNetIndex(objectId);
		
		/* check if we have found an sb: if not the object has probably been 
		destroyed already, so just ignore it */
		
		if(sbPtr) {

			/* Er... deal with it, okay? */
			if (sbPtr->I_SBtype==I_BehaviourInanimateObject) {
				RemovePickedUpObject(sbPtr);
			} else {
				GLOBALASSERT(0);
			}

		}

	}
}

static void ProcessNetMsg_LocalObjectDestroyed(NETMESSAGE_LOBDESTROYED *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* this message is a cue to destroy a ghost... except for the player which has 
	a seperate 'playerkilled' message... */

	/* start by finding the ghost for this object */
	objectId = (int)messagePtr->objectId;
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* if we haven't got a ghost for this object things have gone wrong: eg, we haven't
		yet received the object creation message. Just ignore it then... */
		return;
	}	
	RemoveGhost(sbPtr);	
}


static int GetSizeOfInanimateDamagedMessage(char *messagePtr)
{
	int size=sizeof(NETMESSAGE_INANIMATEDAMAGED_HEADER);
	NETMESSAGE_INANIMATEDAMAGED_HEADER *messageHeader =(NETMESSAGE_INANIMATEDAMAGED_HEADER*) messagePtr;

	if(messageHeader->damageProfile) size+=sizeof(NETMESSAGE_DAMAGE_PROFILE);	
	if(messageHeader->multiple) size+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);	

	return size;
}

static void ProcessNetMsg_InanimateObjectDamaged(char *messagePtr)
{
	NETMESSAGE_INANIMATEDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;
	
	STRATEGYBLOCK *sbPtr;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* only process this if we're the host*/
	if(AvP.Network!=I_Host) return;

	messageHeader=(NETMESSAGE_INANIMATEDAMAGED_HEADER*) messagePtr;
	messagePtr+=sizeof(NETMESSAGE_INANIMATEDAMAGED_HEADER);

	//find out which elements of the damage message have been sent
	if(messageHeader->damageProfile)
	{
		messageProfile=(NETMESSAGE_DAMAGE_PROFILE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_PROFILE);
	}
	if(messageHeader->multiple)
	{
		messageMultiple=(NETMESSAGE_DAMAGE_MULTIPLE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);
	}
	
	/* start by finding the object */
	sbPtr = FindEnvironmentObjectFromName(messageHeader->name);
	if(!sbPtr)
	{
		/* if we haven't got a ghost for this object things have gone wrong: eg, we haven't
		yet received the object creation message. Just ignore it then... */
		return;
	}	

	/* ok: cause the damage */
	{
		STRATEGYBLOCK *objectPtr = FindEnvironmentObjectFromName(messageHeader->name);
		DAMAGE_PROFILE damage;
		int multiple;

		if(!objectPtr) return; /* couldn't find it */

		//fill out damage profile
		damage.Id=messageHeader->ammo_id;
		if(messageProfile)
		{
			damage.Impact = messageProfile->Impact;
			damage.Cutting = messageProfile->Cutting;
			damage.Penetrative = messageProfile->Penetrative;
			damage.Fire = messageProfile->Fire;
			damage.Electrical = messageProfile->Electrical;
			damage.Acid = messageProfile->Acid;

			damage.ExplosivePower=messageProfile->ExplosivePower;
			damage.Slicing=messageProfile->Slicing;
			damage.ProduceBlood=messageProfile->ProduceBlood;
			damage.ForceBoom=messageProfile->ForceBoom;

			damage.BlowUpSections=messageProfile->BlowUpSections;
			damage.Special=messageProfile->Special;
			damage.MakeExitWounds=messageProfile->MakeExitWounds;
		}
		else
		{
			GLOBALASSERT(damage.Id>AMMO_NONE && damage.Id<MAX_NO_OF_AMMO_TEMPLATES);
			damage=TemplateAmmo[damage.Id].MaxDamage[AvP.Difficulty];
		}

		//get damage multiple
		if(messageMultiple)
		{
			multiple = messageMultiple->multiple;
		}
		else
		{
			multiple = ONE_FIXED;	
		}
		
		CauseDamageToObject(sbPtr, (&damage), multiple,NULL);		
	}	
}

static void ProcessNetMsg_InanimateObjectDestroyed(NETMESSAGE_INANIMATEDESTROYED *messagePtr)
{
	STRATEGYBLOCK *sbPtr;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;
	/* only peers should get this */
	LOCALASSERT(AvP.Network!=I_Host);

	/* start by finding the object */
	sbPtr = FindEnvironmentObjectFromName(messagePtr->name);
	if(!sbPtr)
	{
		/* if we haven't got a ghost for this object things have gone wrong: eg, we haven't
		yet received the object creation message. Just ignore it then... */
		return;
	}	


	/* ok: drop a nuke on it */
	{
		extern int InanimateDamageFromNetHost;

		InanimateDamageFromNetHost = 1;
		CauseDamageToObject(sbPtr, &certainDeath, ONE_FIXED,NULL);
		InanimateDamageFromNetHost = 0;
	}	
}

static void ProcessNetMsg_ObjectPickedUp(NETMESSAGE_OBJECTPICKEDUP *messagePtr)
{
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	{
		STRATEGYBLOCK *objectPtr = FindEnvironmentObjectFromName(messagePtr->name);
		if(!objectPtr) return; /* couldn't find it */
		
		LOCALASSERT(objectPtr->I_SBtype == I_BehaviourInanimateObject);

		/* Play an appropriate sound? */
		{
			INANIMATEOBJECT_STATUSBLOCK* objStatPtr = objectPtr->SBdataptr;

			/* patrick, for e3- add a sound effect to explosions */
			if(objectPtr->DynPtr)
			{
				switch(objStatPtr->typeId)
				{
					case(IOT_Weapon):
						/* Ignore predator case for now! */
						Sound_Play(SID_MARINE_PICKUP_WEAPON,"%d",&objectPtr->DynPtr->Position);
						break;
					case(IOT_Ammo):
						Sound_Play(SID_MARINE_PICKUP_AMMO,"%d",&objectPtr->DynPtr->Position);
						break;
					case(IOT_Armour):
						Sound_Play(SID_MARINE_PICKUP_ARMOUR,"%d",&objectPtr->DynPtr->Position);
						break;
					case(IOT_FieldCharge):
						Sound_Play(SID_PREDATOR_PICKUP_FIELDCHARGE,"%d",&objectPtr->DynPtr->Position);
						break;
					default:
						Sound_Play(SID_PICKUP,"%d",&objectPtr->DynPtr->Position);
						break;
				}
			}
		}

		KillInanimateObjectForRespawn(objectPtr);
	}
}

static void ProcessNetMsg_EndGame(void)
{
	/* should only get this if we're not the host */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
	}

	/* only do this if we're playing or in startup */
			/* check start flags on all players (including ourselves) */
			{
				if(netGameData.playerData[PlayerIdInPlayerList(AVPDPNetID)].startFlag)
				{
					netGameData.myGameState = NGS_Playing;
				}
			}
	if((netGameData.myGameState!=NGS_Playing)&&(netGameData.myGameState!=NGS_StartUp)&&(netGameData.myGameState!=NGS_Joining)) return;
	
	netGameData.myGameState = NGS_EndGame;
	AvP.MainLoopRunning = 0;
}

static void ProcessNetMsg_PlayerLeaving(DPID senderId)
{
	/* only do this if we're playing or in startup */
	if(netGameData.myGameState==NGS_Playing || netGameData.myGameState==NGS_EndGameScreen)
	{
		RemovePlayersGhosts(senderId);
	}

	Inform_PlayerHasLeft(senderId);
	
	RemovePlayerFromGame(senderId);
	
}

static void ProcessNetMsg_LOSRequestBinarySwitch(NETMESSAGE_LOSREQUESTBINARYSWITCH *msgPtr)
{
	STRATEGYBLOCK *objectPtr;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	objectPtr = FindEnvironmentObjectFromName(msgPtr->name);
	if(!objectPtr) return; /* no object */
	if(objectPtr->I_SBtype!=I_BehaviourBinarySwitch && objectPtr->I_SBtype!=I_BehaviourLinkSwitch) return;/* we only do switches */
		
	/* change the state of this object, then, via request state */
	RequestState(objectPtr,1,NULL);
}

static void ProcessNetMsg_PlatformLiftState(NETMESSAGE_PLATFORMLIFTSTATE *msgPtr)
{
	STRATEGYBLOCK *objectPtr;

	/* only peers should get this */
	if(AvP.Network!=I_Peer)
	{
		//Vaguely possible that a host could receive a message that is only intended for peers
		//if this computer has only just become the host.
		LOCALASSERT(AvP.Network==I_Host);
		return;
	}

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	objectPtr = FindEnvironmentObjectFromName(msgPtr->name);
	if(!objectPtr) return; /* no object */
	if(objectPtr->I_SBtype!=I_BehaviourPlatform) return;/* we only do binary switches */
	
	/* update the lift state */
	{
		extern void NetworkPeerChangePlatformLiftState(STRATEGYBLOCK* sbPtr,PLATFORMLIFT_STATES new_state);
		NetworkPeerChangePlatformLiftState(objectPtr,(PLATFORMLIFT_STATES)(msgPtr->state));		
	}	
}

static void ProcessNetMsg_RequestPlatformLiftActivate(NETMESSAGE_REQUESTPLATFORMLIFTACTIVATE *msgPtr)
{
	STRATEGYBLOCK *objectPtr;

	/* only host should process this */
	if(AvP.Network!=I_Host) return;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	objectPtr = FindEnvironmentObjectFromName(msgPtr->name);
	if(!objectPtr) return; /* no object */
	if(objectPtr->I_SBtype!=I_BehaviourPlatform) return;/* we only do platform lifts */
	
	/* update the lift state */
	{
		PLATFORMLIFT_BEHAVIOUR_BLOCK *platLiftData = (PLATFORMLIFT_BEHAVIOUR_BLOCK *)objectPtr->SBdataptr;
		LOCALASSERT(platLiftData);
		if(platLiftData->state == PLBS_AtRest && platLiftData->Enabled) ActivatePlatformLift(objectPtr);
	}	
}

static void ProcessNetMsg_PlayerAutoGunState(NETMESSAGE_AGUNSTATE *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* get the object id from the message */
	objectId = (int)messagePtr->objectId;

	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* we don't seem to have a ghost for this autoGun, so create one */
		VECTORCH position;
		EULER orientation;

		position.vx = messagePtr->xPos;
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		position.vy = messagePtr->yPos;
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		position.vz = messagePtr->zPos;
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);

		/* NB there is no sequence required for autoguns, so just pass zero */
		sbPtr = CreateNetGhost(senderId,objectId,&position,&orientation,I_BehaviourAutoGun,IOT_Non,0);
		if(sbPtr) 
		{
			HandleGhostAutoGunMuzzleFlash(sbPtr, (int)messagePtr->IAmFiring);
			HandleGhostAutoGunSound(sbPtr, (int)messagePtr->IAmFiring);
		}
	
	}
	else
	{
		/* update the autogun ghost... */
		VECTORCH position;
		EULER orientation;

		position.vx = messagePtr->xPos;
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		position.vy = messagePtr->yPos;
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		position.vz = messagePtr->zPos;
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);

		UpdateGhost(sbPtr,&position,&orientation,0,0);
		HandleGhostAutoGunMuzzleFlash(sbPtr, (int)messagePtr->IAmFiring);
		HandleGhostAutoGunSound(sbPtr,(int)messagePtr->IAmFiring);
	}
}


static void ProcessNetMsg_MakeDecal(NETMESSAGE_MAKEDECAL *messagePtr)
{
	if(netGameData.myGameState!=NGS_Playing) return;
	
	AddDecal(messagePtr->DecalID,&(messagePtr->Direction),&(messagePtr->Position),messagePtr->ModuleIndex);
}


static char *ProcessNetMsg_ChatBroadcast(char *subMessagePtr, DPID senderId)
{
	
	BOOL same_species_only;
	/* get player index from dpid */
	char *ptr = subMessagePtr;
	int stringLength=1;

	int playerIndex = PlayerIdInPlayerList(senderId);

	//get same_species flag
	same_species_only=*ptr++;

	while(*ptr++ && stringLength<255)
	{
		stringLength++;
	}
	
	if (stringLength==1 || stringLength>=255)
	{
		LOCALASSERT(0);
		return ptr;
	}
	if(playerIndex==NET_IDNOTINPLAYERLIST)
	{
		return ptr;
	}
		

	if(netGameData.myGameState==NGS_Playing)
	{
		if(same_species_only)
		{
			//was a species say message , check to see if we are the correct species
			if(netGameData.playerData[playerIndex].characterType != netGameData.myCharacterType ) return ptr;
		}
		
		sprintf(OnScreenMessageBuffer,"%s: %s",netGameData.playerData[playerIndex].name,subMessagePtr+1);
		NewOnScreenMessage(OnScreenMessageBuffer);
	
		/* KJL 99/2/5 - play 'incoming message' sound */
		switch(netGameData.playerData[playerIndex].characterType)
		{
			case NGCT_Marine:
			{
				Sound_Play(SID_CONSOLE_MARINEMESSAGE,NULL);
				break;
			}
    	   	case NGCT_Predator:
			{
				Sound_Play(SID_CONSOLE_PREDATORMESSAGE,NULL);
				break;
			}
			case NGCT_Alien:
			{
				Sound_Play(SID_CONSOLE_ALIENMESSAGE,NULL);
				break;
			}
			default:
				break;
		}
	}
	return ptr;
	
}

static void ProcessNetMsg_MakeExplosion(NETMESSAGE_MAKEEXPLOSION *messagePtr)
{
	if(netGameData.myGameState!=NGS_Playing) return;
	MakeVolumetricExplosionAt(&(messagePtr->Position),messagePtr->ExplosionID);
}

static void ProcessNetMsg_MakeFlechetteExplosion(NETMESSAGE_MAKEFLECHETTEEXPLOSION *messagePtr)
{
	extern void MakeFlechetteExplosionAt(VECTORCH *positionPtr,int seed);
	if(netGameData.myGameState!=NGS_Playing) return;
	MakeFlechetteExplosionAt(&(messagePtr->Position),messagePtr->Seed);
}

static void ProcessNetMsg_MakePlasmaExplosion(NETMESSAGE_MAKEPLASMAEXPLOSION *messagePtr)
{
	if(netGameData.myGameState!=NGS_Playing) return;
	MakePlasmaExplosion(&(messagePtr->Position),&(messagePtr->FromPosition),messagePtr->ExplosionID);
}

static void ProcessNetMsg_PredatorSights(NETMESSAGE_PREDATORSIGHTS *messagePtr, DPID senderId)
{
	extern THREE_LASER_DOT_DESC PredatorLaserSights[];
	int playerIndex = PlayerIdInPlayerList(senderId);

	if(netGameData.myGameState!=NGS_Playing) return;

	if(playerIndex==NET_IDNOTINPLAYERLIST)
	{
		return;
	}

	{
		int i=2;

		VECTORCH offset[3] =
		{
			{0,-50,0},
			{43,25,0},
			{-43,25,0},
		};
		MATRIXCH matrix;
		VECTORCH centre;
		EULER orientation;


		centre.vx = messagePtr->xPos;
		orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
		centre.vy = messagePtr->yPos;
		orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
		centre.vz = messagePtr->zPos;
		orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);
	
		CreateEulerMatrix(&orientation,&matrix);
		TransposeMatrixCH(&matrix);
		do
		{
			VECTORCH position = offset[i];

		  	RotateVector(&position,&matrix);

			PredatorLaserSights[playerIndex].Position[i].vx = centre.vx + position.vx;
			PredatorLaserSights[playerIndex].Position[i].vy = centre.vy + position.vy;
			PredatorLaserSights[playerIndex].Position[i].vz = centre.vz + position.vz;
			PredatorLaserSights[playerIndex].Normal[i].vx = matrix.mat31;
			PredatorLaserSights[playerIndex].Normal[i].vy = matrix.mat32;
			PredatorLaserSights[playerIndex].Normal[i].vz = matrix.mat33;
		}
		while(i--);

		PredatorLaserSights[playerIndex].TargetID = messagePtr->TargetID;
  	}	 


}

static void ProcessNetMsg_FragmentalObjectsStatus(NETMESSAGE_FRAGMENTALOBJECTSSTATUS *messagePtr)
{
	int i;
	int fragNumber=0;
	int objectsToSkip=messagePtr->BatchNumber*(NUMBER_OF_FRAGMENTAL_OBJECTS<<3);

	if(netGameData.myGameState!=NGS_Playing) return;
	LOCALASSERT(AvP.Network!=I_No_Network);
	
	for (i=0; i<NUMBER_OF_FRAGMENTAL_OBJECTS; i++)
	{
		FragmentalObjectStatus[i] = messagePtr->StatusBitfield[i];	
	}

	for (i=0; i<NumActiveStBlocks; i++)
	{
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];
		int status;

		if(sbPtr->I_SBtype == I_BehaviourInanimateObject)
		{
			INANIMATEOBJECT_STATUSBLOCK* objectStatusPtr = sbPtr->SBdataptr;
			LOCALASSERT(objectStatusPtr);
			
			if((objectStatusPtr->typeId == IOT_Static) && (!objectStatusPtr->Indestructable) && (!objectStatusPtr->lifespanTimer))
			{
				if(objectsToSkip>0)
				{
					objectsToSkip--;
					continue;
				}
				status = ReadFragmentStatus(fragNumber++);

				if (status) /* should exist */
				{
					#if 0
					/* so if it doesn't exist, respawn it */
					if(objectStatusPtr->respawnTimer!=0)
					{
						RespawnInanimateObject(sbPtr);
						objectStatusPtr->respawnTimer=0;	
					}
					#endif
				}
				else /* shouldn't exist */
				{
					if(objectStatusPtr->respawnTimer==0)
					{
						extern void KillFragmentalObjectForRespawn(STRATEGYBLOCK *sbPtr);
						KillFragmentalObjectForRespawn(sbPtr);	
					}
				}
			}
		}
		else if(sbPtr->I_SBtype == I_BehaviourPlacedLight)
		{
			PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
			LOCALASSERT(pl_bhv);
			
			if(!pl_bhv->Indestructable)
			{
				if(objectsToSkip>0)
				{
					objectsToSkip--;
					continue;
				}
				status = ReadFragmentStatus(fragNumber++);
				
				if (status) /* should exist */
				{
					#if 0
					/* so if it doesn't exist, respawn it */
					if(pl_bhv->state==Light_State_Broken)
					{
						RespawnInanimateObject(sbPtr);
						objectStatusPtr->respawnTimer=0;	
					}
					#endif
				}
				else /* shouldn't exist */
				{
					if(pl_bhv->state!=Light_State_Broken)
					{
						KillLightForRespawn(sbPtr);	
					}
				}
			}
			
		}
		
		if(fragNumber>=(NUMBER_OF_FRAGMENTAL_OBJECTS<<3))break;

	}		
}

static void ProcessNetMsg_StrategySynch(NETMESSAGE_STRATEGYSYNCH *messagePtr)
{
	int i;
	int objectNumber=0;
	int objectsToSkip=messagePtr->BatchNumber*(NUMBER_OF_STRATEGIES_TO_SYNCH);

	if(netGameData.myGameState!=NGS_Playing) return;
	LOCALASSERT(AvP.Network!=I_No_Network);
	
	if(!netGameData.myStrategyCheckSum) netGameData.myStrategyCheckSum=GetStrategySynchObjectChecksum();
	if(messagePtr->strategyCheckSum!=netGameData.myStrategyCheckSum)
	{
		//strategies are obviously different from those on the host's machine
		textprint("Strategy checksums don't match");
		return;
	}
	
	for (i=0; i<NUMBER_OF_STRATEGIES_TO_SYNCH>>2; i++)
	{
		StrategySynchArray[i] = messagePtr->StatusBitfield[i];	
	}

	for (i=0; i<NumActiveStBlocks; i++)
	{
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

		if(sbPtr->I_SBtype == I_BehaviourBinarySwitch ||
		   sbPtr->I_SBtype == I_BehaviourLinkSwitch ||
		   sbPtr->I_SBtype == I_BehaviourTrackObject)
		{
			
			if(objectsToSkip>0)
			{
				objectsToSkip--;
				continue;
			}

			switch(sbPtr->I_SBtype)
			{
				case I_BehaviourBinarySwitch :
		 			BinarySwitchSetSynchData(sbPtr,ReadStrategySynch(objectNumber++));
					break;
				case I_BehaviourLinkSwitch :
		 			LinkSwitchSetSynchData(sbPtr,ReadStrategySynch(objectNumber++));
					break;
				case I_BehaviourTrackObject :
		 			TrackObjectSetSynchData(sbPtr,ReadStrategySynch(objectNumber++));
					break;
				default:
					break;
			}
			
		}
						
		if(objectNumber>=(NUMBER_OF_STRATEGIES_TO_SYNCH))break;

	}		
}

static void ProcessNetMsg_LocalObjectOnFire(NETMESSAGE_LOBONFIRE *messagePtr, DPID senderId)
{
	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* This message is for the player who owns the object, so first check 
	if the message is meant for us */
	if(messagePtr->playerId != AVPDPNetID) return;

	/* next we have to find this object in our strategyblock list */
	{
		int objectId; 
		STRATEGYBLOCK *sbPtr;

		objectId = (int)messagePtr->objectId;
		sbPtr = FindObjectFromNetIndex(objectId);
		
		/* check if we have found an sb: if not the object has probably been 
		destroyed already, so just ignore it */
		if(sbPtr)
		{
			sbPtr->SBDamageBlock.IsOnFire=1;
			if (sbPtr == Player->ObStrategyBlock)
			{
				myIgniterId=senderId;
				PlayerStatusPtr->fireTimer=PLAYER_ON_FIRE_TIME;				
			}
			else if(sbPtr->I_SBtype==I_BehaviourAlien)
			{
				//need to note who burnt this alien
				ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;
				alienStatus->aliensIgniterId=senderId;
			}
		}
	}
}

static void ProcessNetMsg_AlienAIState(NETMESSAGE_ALIENAISTATE *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;
	VECTORCH position;
	EULER orientation;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* get the object id from the message */
	objectId = (int)messagePtr->Guid;
	
	sbPtr = FindGhost(senderId, objectId);
	
	position.vx = messagePtr->xPos;
	orientation.EulerX = (messagePtr->xOrient<<NET_EULERSCALESHIFT);
	position.vy = messagePtr->yPos;
	orientation.EulerY = (messagePtr->yOrient<<NET_EULERSCALESHIFT);
	position.vz = messagePtr->zPos;
	orientation.EulerZ = (messagePtr->zOrient<<NET_EULERSCALESHIFT);
	
	if(!sbPtr)
	{
		/* we don't seem to have a ghost for this object, so create one */
		sbPtr = CreateNetGhost(senderId,objectId,&position,&orientation,I_BehaviourAlien, IOT_Non,messagePtr->AlienType);
	}
	else
	{
		/* update the ghost... */
		MaintainGhostFireStatus(sbPtr,(int)messagePtr->IAmOnFire);
		/* NB don't need to update the object type */
		UpdateAlienAIGhost(sbPtr,&position,&orientation,messagePtr->sequence_type,messagePtr->sub_sequence,messagePtr->sequence_length<<8);
	}

	#if EXTRAPOLATION_TEST
	if(sbPtr)
	{
		NETGHOSTDATABLOCK *ghostData;
		VECTORCH velocity;
		int diff;
		int playerTimer;
		int playerIndex = PlayerIdInPlayerList(senderId);
		if (playerIndex==NET_IDNOTINPLAYERLIST) return;
		
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(ghostData);
				
		playerTimer=netGameData.playerData[playerIndex].timer;

		velocity.vx=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat31,messagePtr->speed);
		velocity.vy=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat32,messagePtr->speed);
		velocity.vz=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat33,messagePtr->speed);

		diff=Approximate3dMagnitude(&ghostData->velocity)-Approximate3dMagnitude(&velocity);
		
		if(diff>1500 || -diff>1500)
		{
			//change in velocity , so reset extrapolation timer
			ghostData->extrapTimer=-ONE_FIXED;
		}

		ghostData->velocity=velocity;
		ghostData->extrapTimerLast=0;
		if(playerTimer>=ghostData->lastTimeRead)
		{
			ghostData->extrapTimer-=(playerTimer-ghostData->lastTimeRead);
		}
		else
		{
			ghostData->extrapTimer=-ONE_FIXED;
		}
		ghostData->lastTimeRead=playerTimer;

		sbPtr->DynPtr->UseStandardGravity=messagePtr->standard_gravity;
		if(!sbPtr->DynPtr->UseStandardGravity)
		{
			if(sbPtr->DynPtr->GravityDirection.vy==ONE_FIXED)
			{
				MATRIXCH mat;
				//alien is crawling , so we need to get an appropriate gravity direction
				CreateEulerMatrix(&orientation,&mat);
				sbPtr->DynPtr->GravityDirection.vx=mat.mat12;
				sbPtr->DynPtr->GravityDirection.vy=mat.mat22;
				sbPtr->DynPtr->GravityDirection.vz=mat.mat32;
			}

			sbPtr->DynPtr->LinImpulse.vx=0;
			sbPtr->DynPtr->LinImpulse.vy=0;
			sbPtr->DynPtr->LinImpulse.vz=0;

			sbPtr->DynPtr->ToppleForce=TOPPLE_FORCE_ALIEN;
		}

	}
	#endif

}

/* CDF 24/8/98 A better message. */
static void ProcessNetMsg_AlienAISequenceChange(NETMESSAGE_ALIENSEQUENCECHANGE *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* get the object id from the message */
	objectId = (int)messagePtr->Guid;
	
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* Gordon Bennet.  Return, then. */
		return;
	}
	else
	{
		int sequence_length,tweening_time;
		/* update the ghost... */

		if(messagePtr->sequence_length==-1)
			sequence_length=-1;
		else
			sequence_length=messagePtr->sequence_length<<8;

		if(messagePtr->tweening_time==-1)
			tweening_time=-1;
		else
			tweening_time=messagePtr->tweening_time<<8;

		UpdateAlienAIGhostAnimSequence(sbPtr,messagePtr->sequence_type,messagePtr->sub_sequence,sequence_length,tweening_time);
	}
}

static void ProcessNetMsg_AlienAIKilled(NETMESSAGE_ALIENAIKILLED *messagePtr, DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	Inform_AiHasDied(messagePtr->killerId,messagePtr->AlienType,messagePtr->weaponIcon);

	{
		int killerIndex=PlayerIdInPlayerList(messagePtr->killerId);
		if(killerIndex!=NET_IDNOTINPLAYERLIST)
		{
			netGameData.playerData[killerIndex].aliensKilled[messagePtr->AlienType]=messagePtr->killCount;
		}
	}
	
	/* get the object id from the message */
	objectId = (int)messagePtr->Guid;
	
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* Gordon Bennet.  Return, then. It's not that important. */
		return;
	}
	else
	{
		/* 'update' the ghost... */
		KillAlienAIGhost(sbPtr,messagePtr->death_code,messagePtr->death_time,messagePtr->GibbFactor);
	}
}

static void ProcessNetMsg_FarAlienPosition(NETMESSAGE_FARALIENPOSITION *messagePtr, DPID senderId)
{
	STRATEGYBLOCK* sbPtr;
	EULER orientation={0,0,0};
	VECTORCH position;
	AIMODULE* targetModule=0;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	//make sure the target module index is in range
	if(messagePtr->targetModuleIndex>=AIModuleArraySize) return;
	targetModule = &AIModuleArray[messagePtr->targetModuleIndex];

	if(messagePtr->indexIsModuleIndex)
	{
		FARENTRYPOINT *targetEntryPoint;
		AIMODULE *startModule=0;
		//The alien is located at an entry point , the second index is the source module index
		//Make sure it is a valid module index
		if(messagePtr->index>=AIModuleArraySize) return;
		startModule = &AIModuleArray[messagePtr->index];
		
		//find the appropriate entry point
		targetEntryPoint = GetAIModuleEP(targetModule,startModule);

		if(!targetEntryPoint) return; //forget it then

		//found the alien's location (relative to module)
		position=targetEntryPoint->position;
		
	}
	else
	{
		//The alien is at one of the modules auxilary locations
		int noOfAuxLocs = FALLP_AuxLocs[messagePtr->targetModuleIndex].numLocations;
		VECTORCH *auxLocsList = FALLP_AuxLocs[messagePtr->targetModuleIndex].locationsList; 

		//make sure we have a valid index
		if(messagePtr->index>=noOfAuxLocs) return;

		//found the alien's location (relative to module)
		position=auxLocsList[messagePtr->index];
	}
	//convert the position into a world position
	position.vx += targetModule->m_world.vx;
	position.vy += targetModule->m_world.vy;
	position.vz += targetModule->m_world.vz;
	

	//find the alien
	sbPtr = FindGhost(senderId, messagePtr->Guid);

	if(!sbPtr)
	{
		//need to create a new ghost then
		sbPtr = CreateNetGhost(senderId,messagePtr->Guid,&position,&orientation,I_BehaviourAlien, IOT_Non,messagePtr->alienType);
	}
	
	if(sbPtr)
	{
		NETGHOSTDATABLOCK *ghostData;
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(ghostData);
		//make sure this is a ghost of an alien
		if(ghostData->type!=I_BehaviourAlien) return;

		//update the position , and mark it as valid for far use only
		ghostData->onlyValidFar=1;

		sbPtr->DynPtr->Position=position;
		sbPtr->DynPtr->PrevPosition=position;

		sbPtr->DynPtr->LinVelocity.vx=0;
		sbPtr->DynPtr->LinVelocity.vy=0;
		sbPtr->DynPtr->LinVelocity.vz=0;

		sbPtr->DynPtr->LinImpulse.vx=0;
		sbPtr->DynPtr->LinImpulse.vy=0;
		sbPtr->DynPtr->LinImpulse.vz=0;

		//make sure the alien is far
		if(sbPtr->SBdptr) MakeGhostFar(sbPtr);
		
	}
}

static int GetSizeOfGhostHierarchyDamagedMessage(char *messagePtr)
{
	int size=sizeof(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER);
	NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER *messageHeader =(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER*) messagePtr;

	if(messageHeader->damageProfile) size+=sizeof(NETMESSAGE_DAMAGE_PROFILE);	
	if(messageHeader->multiple) size+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);	
	if(messageHeader->sectionID) size+=sizeof(NETMESSAGE_DAMAGE_SECTION);	
	if(messageHeader->direction) size+=sizeof(NETMESSAGE_DAMAGE_DIRECTION);	

	return size;
}

static void ProcessNetMsg_GhostHierarchyDamaged(char *messagePtr, DPID senderId)
{
	NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER *messageHeader=0;
	NETMESSAGE_DAMAGE_PROFILE *messageProfile=0;
	NETMESSAGE_DAMAGE_MULTIPLE *messageMultiple=0;
	NETMESSAGE_DAMAGE_SECTION *messageSection=0;
	NETMESSAGE_DAMAGE_DIRECTION *messageDirection=0;
	
	STRATEGYBLOCK *sbPtr;
	int objectId;
	NETGHOSTDATABLOCK *ghostData;
	VECTORCH direction;
	VECTORCH incoming;
	VECTORCH* incoming_ptr=0;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	messageHeader=(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER*) messagePtr;
	messagePtr+=sizeof(NETMESSAGE_GHOSTHIERARCHYDAMAGED_HEADER);

	//find out which elements of the damage message have been sent
	if(messageHeader->damageProfile)
	{
		messageProfile=(NETMESSAGE_DAMAGE_PROFILE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_PROFILE);
	}
	if(messageHeader->multiple)
	{
		messageMultiple=(NETMESSAGE_DAMAGE_MULTIPLE*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_MULTIPLE);
	}
	if(messageHeader->sectionID)
	{
		messageSection=(NETMESSAGE_DAMAGE_SECTION*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_SECTION);
	}
	if(messageHeader->direction)
	{
		messageDirection=(NETMESSAGE_DAMAGE_DIRECTION*) messagePtr;
		messagePtr+=sizeof(NETMESSAGE_DAMAGE_DIRECTION);
	}
	
	/* get the object id from the message */
	objectId = (int)messageHeader->Guid;
	
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* Gordon Bennet.  Return, then. It's not that important. */
		return;
	}
	else
	{
		DAMAGE_PROFILE damage;
		int multiple;
		
		/* damage the ghost... */
		SECTION_DATA *section_data=NULL;
		HMODELCONTROLLER *controller=NULL;
		
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;

		//fill out damage profile
		damage.Id=messageHeader->ammo_id;
		if(messageProfile)
		{
			damage.Impact = messageProfile->Impact;
			damage.Cutting = messageProfile->Cutting;
			damage.Penetrative = messageProfile->Penetrative;
			damage.Fire = messageProfile->Fire;
			damage.Electrical = messageProfile->Electrical;
			damage.Acid = messageProfile->Acid;

			damage.ExplosivePower=messageProfile->ExplosivePower;
			damage.Slicing=messageProfile->Slicing;
			damage.ProduceBlood=messageProfile->ProduceBlood;
			damage.ForceBoom=messageProfile->ForceBoom;

			damage.BlowUpSections=messageProfile->BlowUpSections;
			damage.Special=messageProfile->Special;
			damage.MakeExitWounds=messageProfile->MakeExitWounds;
		}
		else
		{
			GLOBALASSERT(damage.Id>AMMO_NONE && damage.Id<MAX_NO_OF_AMMO_TEMPLATES);
			damage=TemplateAmmo[damage.Id].MaxDamage[AvP.Difficulty];
		}

		if(messageMultiple)
		{
			multiple = messageMultiple->multiple;
		}
		else
		{
			multiple = ONE_FIXED;	
		}
		
		if(sbPtr->DynPtr && messageDirection)
		{
			
			
			if(messageDirection->direction_x ||
			   messageDirection->direction_y ||
			   messageDirection->direction_z)
			{
				MATRIXCH mat=sbPtr->DynPtr->OrientMat;
				TransposeMatrixCH(&mat);

				//extract the direction vector
				incoming.vx=messageDirection->direction_x;
				incoming.vy=messageDirection->direction_y;
				incoming.vz=messageDirection->direction_z;
				//normalise it
				Normalise(&incoming);
				direction=incoming;
				
				//and rotate it from world space to the object's local space
				RotateVector(&incoming,&mat);

				//set the incoming pointer 
				incoming_ptr=&incoming;	
			
			}	

		}
			

		if (messageSection && messageSection->SectionID!=-1) {
			/* Hmm. */
				
			controller=&ghostData->HModelController;
			section_data=GetThisSectionData_FromID(ghostData->HModelController.section_data,
				messageSection->SectionID);
		}
		
		if (section_data) 
		{
			DISPLAYBLOCK *fragged_section=0;
			
			fragged_section=CauseDamageToHModel(controller,sbPtr,(&damage),
								multiple,section_data,incoming_ptr,NULL,1);

			if(fragged_section && damage.Id==AMMO_PRED_RIFLE && incoming_ptr)
			{
				//a speargun has fragged off a body part , so we need to create a spear
				CreateSpearPossiblyWithFragment(fragged_section,&fragged_section->ObWorld,&direction);
			}
		}
	}
}


static void ProcessNetMsg_Gibbing(NETMESSAGE_GIBBING *messagePtr,DPID senderId)
{
	STRATEGYBLOCK *sbPtr;
	int objectId;
	NETGHOSTDATABLOCK *ghostData;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* get the object id from the message */
	objectId = (int)messagePtr->Guid;
	
	sbPtr = FindGhost(senderId, objectId);
	if(!sbPtr)
	{
		/* Gordon Bennet.  Return, then. It's not that important. */
		return;
	}
	else
	{
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;

		//only interested in gibbing corpses
		if(ghostData->type!=I_BehaviourNetCorpse) return;
		
		//use the random number seed
		SetSeededFastRandom(messagePtr->seed);
		//now do the gibbing
		if (messagePtr->gibbFactor>0) 
		{
			Extreme_Gibbing(sbPtr,ghostData->HModelController.section_data,messagePtr->gibbFactor);
		} 
		else if (messagePtr->gibbFactor<0) 
		{
			KillRandomSections(ghostData->HModelController.section_data,-(messagePtr->gibbFactor));
		}
	
	}
}

static void ProcessNetMsg_SpotAlienSound(NETMESSAGE_SPOTALIENSOUND *messagePtr, DPID senderId)
{
	VECTORCH position;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* Just play the thing. */

	

	position.vx=messagePtr->vx;
	position.vy=messagePtr->vy;
	position.vz=messagePtr->vz;

	PlayAlienSound((int)messagePtr->alienType,(int)messagePtr->soundCategory,messagePtr->pitch,NULL,&position);
}

static void ProcessNetMsg_CreateWeapon(NETMESSAGE_CREATEWEAPON * messagePtr)
{
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	//create the weapon
	CreateMultiplayerWeaponPickup(&messagePtr->location,messagePtr->type,&messagePtr->name[0]);
}

static void ProcessNetMsg_SpotOtherSound(NETMESSAGE_SPOTOTHERSOUND *messagePtr, DPID senderId)
{
	VECTORCH position;

	/* only do this if we're playing */
	if(netGameData.myGameState!=NGS_Playing) return;

	/* Just play the thing. */

	position.vx=messagePtr->vx;
	position.vy=messagePtr->vy;
	position.vz=messagePtr->vz;

	PlayOtherSound(messagePtr->SoundIndex,&position,messagePtr->explosion);
}

/*----------------------------------------------------------------------
  These support functions are used to examine the current game state
  ----------------------------------------------------------------------*/

/* returns index if the given DPID is in the player list */
int PlayerIdInPlayerList(DPID Id)
{
	int i;
	/* check first, if we've been passed a null id */
	if(Id==0) return NET_IDNOTINPLAYERLIST;

	/* check player list */
	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId == Id) return i;
	}

	/* failed to find Id */
	return NET_IDNOTINPLAYERLIST;
}

/* returns true if there are any empty slots */
int EmptySlotInPlayerList(void)
{
	int i;

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId == 0) return i;
	}
	return NET_NOEMPTYSLOTINPLAYERLIST;
}

/* Finds the local strategy block who's network object id is that passed */
static STRATEGYBLOCK *FindObjectFromNetIndex(int obIndex)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	/* first of all, check for index "GHOST_PLAYEROBJECTID": that's the player */
	if(obIndex==GHOST_PLAYEROBJECTID)
	{
		return (Player->ObStrategyBlock);
	}

	while(sbIndex < NumActiveStBlocks)
	{	
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[sbIndex++];
		/* need to be careful here: netIndexes maybe the same as pre-assigned names
		for global objects, therefore we must check the type of the object */
		if((sbPtr->I_SBtype == I_BehaviourRocket)||
		   (sbPtr->I_SBtype == I_BehaviourGrenade)||
		   (sbPtr->I_SBtype == I_BehaviourPulseGrenade)||
		   (sbPtr->I_SBtype == I_BehaviourFragmentationGrenade)||
		   (sbPtr->I_SBtype == I_BehaviourFlareGrenade)||
		   (sbPtr->I_SBtype == I_BehaviourPredatorEnergyBolt)||
		   (sbPtr->I_SBtype == I_BehaviourFrisbeeEnergyBolt)||
		   (sbPtr->I_SBtype == I_BehaviourPPPlasmaBolt)||
		   (sbPtr->I_SBtype == I_BehaviourSpeargunBolt)||
		   (sbPtr->I_SBtype == I_BehaviourClusterGrenade)||
		   (sbPtr->I_SBtype == I_BehaviourNPCPredatorDisc)||
		   (sbPtr->I_SBtype == I_BehaviourPredatorDisc_SeekTrack)||
		   (sbPtr->I_SBtype == I_BehaviourAlienSpit)||
		   (sbPtr->I_SBtype == I_BehaviourAutoGun)||
		   (sbPtr->I_SBtype == I_BehaviourAlien)||
		   (sbPtr->I_SBtype == I_BehaviourNetCorpse)||
		   (sbPtr->I_SBtype == I_BehaviourInanimateObject)||
		   (sbPtr->I_SBtype == I_BehaviourProximityGrenade))
		{
			int *sbIdPtr = (int *)(&(sbPtr->SBname[4]));
			if(*sbIdPtr == obIndex) return sbPtr;
		}
	}
	return (STRATEGYBLOCK *)0;
}

/* Finds the inanimate objects strategy block who's name is passed */
/* NB must be careful finding obejcts from their name: local and global objects may end up 
with the same name, so we must make sure we differentiate between them... */
static STRATEGYBLOCK *FindEnvironmentObjectFromName(char *name)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;

	while(sbIndex < NumActiveStBlocks)
	{	
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[sbIndex++];
		
		if((sbPtr->I_SBtype==I_BehaviourInanimateObject)||
			(sbPtr->I_SBtype==I_BehaviourPlatform)||
			(sbPtr->I_SBtype==I_BehaviourPlacedLight)||
			(sbPtr->I_SBtype==I_BehaviourBinarySwitch))
		{
		   if(NAME_ISEQUAL((&(sbPtr->SBname[0])),(name))) return sbPtr;
		}   
	}
	return (STRATEGYBLOCK *)0;
}


/* asigns an object id to a strategyblock, such as a projectile */
#if 0
void AddNetGameObjectID(STRATEGYBLOCK *sbPtr)
{
	int *sbIdPtr = (int *)(&(sbPtr->SBname[0]));

	*sbIdPtr = netNextLocalObjectId++;
}
#endif


/* called by host only: updates the scores for a described kill, and sends a 
game score update message */

static void UpdateNetworkGameScores(DPID playerKilledId, DPID killerId,NETGAME_CHARACTERTYPE playerKilledType,NETGAME_CHARACTERTYPE killerType)
{
	int playerKilledIndex;
	int killerIndex;
	int scoreForKill;
	int i;

	LOCALASSERT(AvP.Network==I_Host);
	if(netGameData.myGameState != NGS_Playing) return;

	
	/* get the index of the player who has been killed. If they're not in
	the player list, can't have been killed ! */	
	playerKilledIndex = PlayerIdInPlayerList(playerKilledId);
	if(playerKilledIndex==NET_IDNOTINPLAYERLIST) return;
					
	if(killerId==0 || killerId == playerKilledId || killerType>=NGCT_AI_Alien)
	{		
		//suicide
		killerIndex=playerKilledIndex;

		//record player's death for purposes of adjusting difficulty.
		//(assuming that optionis being used)
		GeneratorBalance_NotePlayerDeath();
	
	}
	else
	{
		killerIndex = PlayerIdInPlayerList(killerId);
		if(killerIndex==NET_IDNOTINPLAYERLIST) return;
	}
	
	if(killerType>=NGCT_AI_Alien)
	{
		//update deaths from AI;
		netGameData.playerData[playerKilledIndex].deathsFromAI++;		

	}
	else
	{
	//update frag count;
		netGameData.playerData[killerIndex].playerFrags[playerKilledIndex]++;		
	}

	//update overall number of kills
	netGameData.numDeaths[playerKilledType]++;

	
	if(netGameData.gameType==NGT_LastManStanding)
	{
		if(killerType==NGCT_Alien || killerIndex==playerKilledIndex)
		{
			if(playerKilledType!=NGCT_Alien)
			{
				int marineCount=0;
				int marineIndex;

				//give a point to the alien for killing a marine
				if(killerType==NGCT_Alien)
				{
					netGameData.playerData[killerIndex].playerScore+=1;
				}

				//also give a point to every surviving marine
				for(i=0;i<NET_MAXPLAYERS;i++)
				{
					if(i==playerKilledIndex) continue;
					if(netGameData.playerData[i].playerId)
					{
						if(netGameData.playerData[i].characterType!=NGCT_Alien)
						{
							marineCount++;
							marineIndex=i;

							netGameData.playerData[i].playerScore+=1;
							AddNetMsg_ScoreChange(i,i);
						}
					}
				}
				if(marineCount==1)
				{
					//only one marine left , tell everyone
					AddNetMsg_PlayerID(netGameData.playerData[marineIndex].playerId,NetMT_LastManStanding_LastMan);
					Handle_LastManStanding_LastMan(netGameData.playerData[marineIndex].playerId);

				}

			}
		}
		else
		{
			
			
			if(playerKilledType!=NGCT_Alien)
			{
				//lose a point for killing a marine
				netGameData.playerData[killerIndex].playerScore-=1;

			}
			else
			{
				//if we're the last marine gain a point for killing an alien
				for(i=0;i<NET_MAXPLAYERS;i++)
				{
					if(i==killerIndex) continue;
					if(netGameData.playerData[i].playerId)
					{
						if(netGameData.playerData[i].characterType!=NGCT_Alien)
						{
							break;
						}
					}
				}
				if(i==NET_MAXPLAYERS)
				{
					netGameData.playerData[killerIndex].playerScore+=1;
				}

			
			}
		}
	}
	else if(netGameData.gameType==NGT_Coop)
	{
		//do nothing
	}
	else
	{
		NETGAME_CHARACTERTYPE playerKilledCopy=netGameData.playerData[playerKilledIndex].characterType;
		NETGAME_CHARACTERTYPE killerCopy=netGameData.playerData[killerIndex].characterType;

		//temporarily set the character types to what they were at the time of death
		netGameData.playerData[playerKilledIndex].characterType=playerKilledType;
		netGameData.playerData[killerIndex].characterType=killerType;

		//get score for this kill
		scoreForKill=GetNetScoreForKill(playerKilledIndex,killerIndex);
		
		//restore character types
		netGameData.playerData[playerKilledIndex].characterType=playerKilledCopy;
		netGameData.playerData[killerIndex].characterType=killerCopy;
		
		//update score
		netGameData.playerData[killerIndex].playerScore+=scoreForKill;
		if(scoreForKill>0)
		{
			//note score against person being killed
			netGameData.playerData[playerKilledIndex].playerScoreAgainst+=scoreForKill;
		}
		else
		{
			netGameData.playerData[killerIndex].playerScoreAgainst+=scoreForKill;
		}

		if(netGameData.gameType==NGT_CoopDeathmatch)
		{
			//need to adjust the species scores as well
			netGameData.teamScores[killerType]+=scoreForKill;
			AddNetMsg_SpeciesScores();
		}
	}
	
	if(killerType>=NGCT_AI_Alien)
	{
		//killed by alien ai
		AddNetMsg_ScoreChange(NET_MAXPLAYERS,playerKilledIndex);
	}
	else
	{
		AddNetMsg_ScoreChange(killerIndex,playerKilledIndex);
	}

	//AddNetMsg_PlayerScores(killerIndex);			

	if(netGameData.gameType==NGT_PredatorTag || netGameData.gameType==NGT_AlienTag)
	{
		NETGAME_CHARACTERTYPE tagSpecies = NGCT_Predator;
		if(netGameData.gameType==NGT_AlienTag) tagSpecies = NGCT_Alien;

		
		//was the predator killed?	
		if(playerKilledType==tagSpecies)
		{
			if(CountPlayersOfType(tagSpecies)==1)
			{
				//select new predator
				//if it wasn't suicide , choose killer
				if(playerKilledIndex!=killerIndex)
				{
					AddNetMsg_PlayerID(killerId,NetMT_PredatorTag_NewPredator);
					Handle_SpeciesTag_NewPersonIt(killerId);
				}
				else
				{
					//choose next player
					for(i=playerKilledIndex+1;;i++)
					{
						i=i%NET_MAXPLAYERS;
						if(netGameData.playerData[i].playerId)
						{
							/*
							don't choose player if he was the previous predator
							(In fact this should only happen if someone is silly enough to player single player
							predator tag).
							*/
							if(i!=playerKilledIndex)
							{
								AddNetMsg_PlayerID(netGameData.playerData[i].playerId,NetMT_PredatorTag_NewPredator);
								Handle_SpeciesTag_NewPersonIt(netGameData.playerData[i].playerId);
							}
							break;
						}
					}
					
				}
			}
		}
	}
}


/* This function is a hook for the playerdead() function in player.c:
if we're the host, we need to update the scores seperately in the event that we have
been killed during a netgame. For other players, scores are updated (by the host) in 
response to a 'playerKilled' message.  However, the host will not receive it's own 
playerKilled message, and must therefore do it seperately... */
void DoNetScoresForHostDeath(NETGAME_CHARACTERTYPE myType,NETGAME_CHARACTERTYPE killerType)
{
	LOCALASSERT(AvP.Network==I_Host);
	if(myNetworkKillerId)
	{
		int killer_index=PlayerIdInPlayerList(myNetworkKillerId);
		if(killer_index==NET_IDNOTINPLAYERLIST)
		{
			//the player doing the damage has either left the game , or never existed.
			//call it suicide then.
			myNetworkKillerId=AVPDPNetID;	
		}
		
	}
	UpdateNetworkGameScores(AVPDPNetID,myNetworkKillerId,myType,killerType);
}

int AddUpPlayerFrags(int playerId)
{
	int score = 0;
	int j;
	LOCALASSERT(netGameData.playerData[playerId].playerId!=NULL);
	for(j=0;j<(NET_MAXPLAYERS);j++) score+=netGameData.playerData[playerId].playerFrags[j];
	return score;
}

#if 0
static void ConvertNetNameToUpperCase(char *strPtr)
{
	int count = 0;
	while((count<(NET_PLAYERNAMELENGTH-1))&&(strPtr[count]!='\0'))
	{
		strPtr[count] = toupper(strPtr[count]);
		count++;
	}

}
#endif


/* Patrick 11/7/97 ----------------------------------------------
Functions for determining our sequence for player update messages
-----------------------------------------------------------------*/

static MARINE_SEQUENCE GetMyMarineSequence(void)
{
	int playerIsMoving = 0;
	int playerIsFiring = 0;
	int playerIsCrouching = 0;
	int playerIsAlive = 0;
	int playerIsJumping = 0;
	int usingCloseAttackWeapon;
	extern int StaffAttack;		
		
	/* sort out what state we're in */
	if(PlayerStatusPtr->IsAlive) playerIsAlive = 1;
	else playerIsAlive = 0;					

	if (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward) {
		playerIsMoving=-1;
	} else if((PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight)) {
	   playerIsMoving = 1;
	} else {
		playerIsMoving = 0;
	}

	if ( (Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
		&& (Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
		&& (Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz) ) {
		/* Actually not moving - overruled! */
		playerIsMoving=0;
	}

	if(PlayerStatusPtr->ShapeState!=PMph_Standing) 
	{
		playerIsCrouching = 1;
	}
	else 
	{
		playerIsCrouching = 0;		
	}
	
	if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_PRIMARY)||
	   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_PRIMARY)) {
			playerIsFiring = 1;
	} else {
		if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) {
			if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_SECONDARY)||
			   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_SECONDARY)) {
				playerIsFiring = 1;
			} else {
				playerIsFiring = 0;
			}
		} else {
			playerIsFiring = 0;
		}
	}

	if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL)
		usingCloseAttackWeapon = 1;
	else usingCloseAttackWeapon = 0;	

	/* Fix cudgel. */
	if (usingCloseAttackWeapon) {
		if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_SECONDARY)||
		   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_SECONDARY)) {
			playerIsFiring = 1;
		}
	}

	/* KJL 14:27:14 10/29/97 - deal with jumping & falling */
	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		if (!dynPtr->IsInContactWithFloor && (dynPtr->TimeNotInContactWithFloor==0))
			playerIsJumping=1;
	}	

	/* and deduce the sequence */
	if(playerIsAlive==0) 
	{
		if(playerIsCrouching) {
			return MSQ_CrouchDie;
		} else {
			return MSQ_StandDieFront;
		}
	}

	if(playerIsJumping) {
		return MSQ_Jump;
	}

	/* Put this in here... no running cudgel attacks yet. */
	if(playerIsFiring&&usingCloseAttackWeapon) {
		/* Deal with cudgel case! */
		if (StaffAttack>=0) {
			return(MSQ_BaseOfCudgelAttacks+StaffAttack);
		}
	}

	if(playerIsCrouching)
	{
		if(playerIsMoving>0) {
			return MSQ_Crawl;
		} else if (playerIsMoving<0) {
			return MSQ_Crawl_Backwards;
		} else {
			return MSQ_Crouch;
		}
	}

	if(playerIsMoving>0)
	{
		if(playerIsFiring) {
			if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS)
				&&(LastHand)) {
				return MSQ_RunningFireSecondary;
			} else {
				return MSQ_RunningFire;
			}
		} else {
			return MSQ_Walk;
		}
	} else if (playerIsMoving<0) {
		if(playerIsFiring) {
			if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS)
				&&(LastHand)) {
				return MSQ_RunningFireSecondary_Backwards;
			} else {
				return MSQ_RunningFire_Backwards;
			}
		} else {
			return MSQ_Walk_Backwards;
		}
	}

	if(playerIsFiring) {
		if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS)
			&&(LastHand)) {
			return MSQ_StandingFireSecondary;
		} else {
			return MSQ_StandingFire;
		}
	} else {
		if (PlayerStatusPtr->tauntTimer!=0) {
			return MSQ_Taunt;
		} else {
			return MSQ_Stand;
		}
	}
} 

static ALIEN_SEQUENCE GetMyAlienSequence(void)
{
	extern STRATEGYBLOCK *Biting;
	int playerIsMoving = 0;
	int playerIsFiring = 0;
	int playerIsCrouching = 0;
	int playerIsAlive = 0;
	int playerIsJumping = 0;
		
	/* sort out what state we're in */
	if(PlayerStatusPtr->IsAlive) playerIsAlive = 1;
	else playerIsAlive = 0;					

	if (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward) {
		playerIsMoving =-1;
	} else if ((PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight)) {
	   	playerIsMoving = 1;
	} else {
		playerIsMoving = 0;
	}

	if ( (Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
		&& (Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
		&& (Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz) ) {
		/* Actually not moving - overruled! */
		playerIsMoving=0;
	}

	if(PlayerStatusPtr->ShapeState!=PMph_Standing) playerIsCrouching = 1;
	else playerIsCrouching = 0;		

	/* ChrisF 20/4/98: playerIsFiring now specifies alien weapon behaviour. */
	//if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_PRIMARY)||
	//   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_PRIMARY))
	//		playerIsFiring = 1;
	//else playerIsFiring = 0;	
	//
	//if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber!=WEAPON_ALIEN_SPIT) {
	//	usingCloseAttackWeapon = 1;
	//} else {
	//	usingCloseAttackWeapon = 0;
	//}

	switch(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState) {
		case (WEAPONSTATE_FIRING_PRIMARY):
			if(Biting) {
				playerIsFiring=4; //Eat.
			} else {
				playerIsFiring=1; //Claw.
			}
			break;
		case (WEAPONSTATE_FIRING_SECONDARY):
			playerIsFiring=2; //Tail Poise.
			break;
		case (WEAPONSTATE_RECOIL_SECONDARY):
			playerIsFiring=3; //Tail Strike.
			break;
		default:
			playerIsFiring=0; //Nothing.
			break;
	}
			

	/* KJL 14:27:14 10/29/97 - deal with jumping & falling */
	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		if (!dynPtr->IsInContactWithFloor && (dynPtr->TimeNotInContactWithFloor==0))
			playerIsJumping=1;
	}	

	/* and deduce the sequence */
	if(playerIsAlive==0) 
	{
		return ASQ_Stand; /* kind of irrelevant really */
	}

	if(playerIsJumping) /* TODO: consider jump & crouch */
	{
		switch(playerIsFiring) {
			case 1:
				return ASQ_Pounce;
				break;
			case 2:
				return ASQ_JumpingTailPoise;
				break;
			case 3:
				return ASQ_JumpingTailStrike;
				break;
			case 4:
				/* What the hell? */
				return ASQ_Eat;
				break;
			default:
				return ASQ_Jump;
				break;
		}
	}

	if(playerIsCrouching)
	{
		if(playerIsMoving>0)
		{
			switch(playerIsFiring) {
				case 1:
					return ASQ_CrawlingAttack_Claw;
					break;
				case 2:
					return ASQ_CrawlingTailPoise;
					break;
				case 3:
					return ASQ_CrawlingTailStrike;
					break;
				case 4:
					/* What the hell? */
					return ASQ_CrouchEat;
					break;
				default:
					if(Player->ObStrategyBlock->DynPtr->OrientMat.mat22>50000)
						return ASQ_Scamper;
					else
						return ASQ_Crawl;
					break;
			}
		} else if(playerIsMoving<0)	{
			switch(playerIsFiring) {
				case 1:
					return ASQ_CrawlingAttack_Claw_Backwards;
					break;
				case 2:
					return ASQ_CrawlingTailPoise_Backwards;
					break;
				case 3:
					return ASQ_CrawlingTailStrike_Backwards;
					break;
				case 4:
					/* What the hell? */
					return ASQ_CrouchEat;
					break;
				default:
					if(Player->ObStrategyBlock->DynPtr->OrientMat.mat22>50000)
						return ASQ_Scamper_Backwards;
					else
						return ASQ_Crawl_Backwards;
					break;
			}
		} 		
		
		switch(playerIsFiring) {
			case 1:
				return ASQ_CrouchedAttack_Claw;
				break;
			case 2:
				return ASQ_CrouchedTailPoise;
				break;
			case 3:
				return ASQ_CrouchedTailStrike;
				break;
			case 4:
				return ASQ_Eat;
				break;
			default:
				return ASQ_Crouch;
				break;
		}
	}
	
	if(playerIsMoving>0)
	{
		switch(playerIsFiring) {
			case 1:
				return ASQ_RunningAttack_Claw;
				break;
			case 2:
				return ASQ_RunningTailPoise;
				break;
			case 3:
				return ASQ_RunningTailStrike;
				break;
			case 4:
				/* What the hell? */
				return ASQ_Eat;
				break;
			default:
				return ASQ_Run;
				break;
		}
	} else if(playerIsMoving<0) {
		switch(playerIsFiring) {
			case 1:
				return ASQ_RunningAttack_Claw_Backwards;
				break;
			case 2:
				return ASQ_RunningTailPoise_Backwards;
				break;
			case 3:
				return ASQ_RunningTailStrike_Backwards;
				break;
			case 4:
				/* What the hell? */
				return ASQ_Eat;
				break;
			default:
				return ASQ_Run_Backwards;
				break;
		}
	} 		

	switch(playerIsFiring) {
		case 1:
			return ASQ_StandingAttack_Claw;
			break;
		case 2:
			return ASQ_StandingTailPoise;
			break;
		case 3:
			return ASQ_StandingTailStrike;
			break;
		case 4:
			return ASQ_Eat;
			break;
		default:
			if (PlayerStatusPtr->tauntTimer!=0) {
				/* Second lowest priority ever. */
				return ASQ_Taunt;
			} else {
				return ASQ_Stand;
			}
			break;
	}

} 

static PREDATOR_SEQUENCE GetMyPredatorSequence(void)
{
	int playerIsMoving = 0;
	int playerIsFiring = 0;
	int playerIsCrouching = 0;
	int playerIsAlive = 0;
	int playerIsJumping = 0;
	int usingCloseAttackWeapon;
	extern int StaffAttack;		

	/* sort out what state we're in */
	if(PlayerStatusPtr->IsAlive) playerIsAlive = 1;
	else playerIsAlive = 0;					

	if (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward) {
		playerIsMoving=-1;
	} else if((PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)||
	   (PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight)) {
	   	playerIsMoving = 1;
	} else {
		playerIsMoving = 0;
	}

	if ( (Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
		&& (Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
		&& (Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz) ) {
		/* Actually not moving - overruled! */
		playerIsMoving=0;
	}

	if(PlayerStatusPtr->ShapeState!=PMph_Standing) playerIsCrouching = 1;
	else playerIsCrouching = 0;		

	playerIsFiring = 0;	
	
	if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_PRED_SHOULDERCANNON)
	{
		//the shoulder cannon is fired during recoil (I think)
		if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_PRIMARY)
		{
			playerIsFiring = 1;
		}
	}
	else if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_PRED_WRISTBLADE)
	{
		if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_PRIMARY)||
		   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_PRIMARY) ||
		   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_SECONDARY))
		{
			if(StaffAttack!=-1)
			{
				playerIsFiring = 1;
			}
		}
	}
	else
	{
		if((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_PRIMARY)||
		   (PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_RECOIL_PRIMARY))
		{
				playerIsFiring = 1;
		}
	}

	if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_PRED_WRISTBLADE)
		usingCloseAttackWeapon = 3;
	else if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_PRED_DISC)
		usingCloseAttackWeapon = 1;
	else if(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_PRED_STAFF)
		usingCloseAttackWeapon = 2;
	else usingCloseAttackWeapon = 0;	

	/* KJL 14:27:14 10/29/97 - deal with jumping & falling */
	{
		DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
		if (!dynPtr->IsInContactWithFloor && (dynPtr->TimeNotInContactWithFloor==0))
			playerIsJumping=1;
	}	

	/* and deduce the sequence */
	if(playerIsAlive==0) 
	{
		if(playerIsCrouching) {
			return PredSQ_CrouchDie;
		} else {
			return PredSQ_StandDie;
		}
	}

	if(playerIsJumping) {
		return(PredSQ_Jump);
	}

	if(playerIsCrouching)
	{
		if(playerIsMoving>0)
		{
			if(playerIsFiring&&usingCloseAttackWeapon) {
				/* Deal with staff case! */
				if (usingCloseAttackWeapon==2) {
					if (StaffAttack>=0) {
						return(PredSQ_BaseOfStaffAttacks+StaffAttack);
					}
				} else if (usingCloseAttackWeapon==3) {
					if (StaffAttack>=0) {
						return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
					}
				}
				return PredSQ_CrawlingSwipe;
			} else {
				return PredSQ_Crawl;
			}
		} else if (playerIsMoving<0) {
			if(playerIsFiring&&usingCloseAttackWeapon) {
				/* Deal with staff case! */
				if (usingCloseAttackWeapon==2) {
					if (StaffAttack>=0) {
						return(PredSQ_BaseOfStaffAttacks+StaffAttack);
					}
				} else if (usingCloseAttackWeapon==3) {
					if (StaffAttack>=0) {
						return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
					}
				}
				return PredSQ_CrawlingSwipe_Backwards;
			} else {
				return PredSQ_Crawl_Backwards;
			}
		}
		if(playerIsFiring&&usingCloseAttackWeapon) {
			/* Deal with staff case! */
			if (usingCloseAttackWeapon==2) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfStaffAttacks+StaffAttack);
				}
			} else if (usingCloseAttackWeapon==3) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
				}
			}
			return PredSQ_CrouchedSwipe;
		} else {
			return PredSQ_Crouch;
		}
	}
	
	if(playerIsMoving>0)
	{
		if(playerIsFiring&&usingCloseAttackWeapon) {
			/* Deal with staff case! */
			if (usingCloseAttackWeapon==2) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfStaffAttacks+StaffAttack);
				}
			} else if (usingCloseAttackWeapon==3) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
				}
			}
			return PredSQ_RunningSwipe;
		} else {
			return PredSQ_Run;
		}
	} else if (playerIsMoving<0) {
		if(playerIsFiring&&usingCloseAttackWeapon) {
			/* Deal with staff case! */
			if (usingCloseAttackWeapon==2) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfStaffAttacks+StaffAttack);
				}
			} else if (usingCloseAttackWeapon==3) {
				if (StaffAttack>=0) {
					return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
				}
			}
			return PredSQ_RunningSwipe_Backwards;
		} else {
			return PredSQ_Run_Backwards;
		}
	}

	if(playerIsFiring&&usingCloseAttackWeapon) {
		/* Deal with staff case! */
		if (usingCloseAttackWeapon==2) {
			if (StaffAttack>=0) {
				return(PredSQ_BaseOfStaffAttacks+StaffAttack);
			}
		} else if (usingCloseAttackWeapon==3) {
			if (StaffAttack>=0) {
				return(PredSQ_BaseOfWristbladeAttacks+StaffAttack);
			}
		}
		return PredSQ_StandingSwipe;
	} else {
		if (PlayerStatusPtr->tauntTimer!=0) {
			return PredSQ_Taunt;
		} else {
			return PredSQ_Stand;
		}
	}
}

/* Patrick 11/7/97 ----------------------------------------------
Functions for transmitting state changes
-----------------------------------------------------------------*/
void TransmitEndOfGameNetMsg(void)
{
	int i;
	/* first of, initialise the send buffer: this means that we may loose some stacked
	messages, but this should be ok.*/
	InitialiseSendMessageBuffer();

	netGameData.myGameState=NGS_EndGameScreen;
	for(i=0;i<NET_MESSAGEITERATIONS;i++)
	{
		AddNetMsg_AllGameScores();
	 //	AddNetMsg_EndGame();
		NetSendMessages();
	}
	netGameData.stateCheckTimeDelay=3*ONE_FIXED;
}

void TransmitPlayerLeavingNetMsg(void)
{
	int i;
	/* first of, initialise the send buffer: this means that we may loose some stacked
	messages, but this should be ok.*/
	InitialiseSendMessageBuffer();

	for(i=0;i<NET_MESSAGEITERATIONS;i++)
	{
		AddNetMsg_PlayerLeaving();
		NetSendMessages();
	}
}

void TransmitStartGameNetMsg(void)
{
	int i;
	/* first of, initialise the send buffer: this means that we may loose some stacked
	messages, but this should be ok.*/
	InitialiseSendMessageBuffer();

	for(i=0;i<NET_MESSAGEITERATIONS;i++)
	{
		AddNetMsg_StartGame();
		NetSendMessages();
	}
}

/* Patrick 29/7/97 --------------------------------------------------
Stuff for assigning starting positions to network players
---------------------------------------------------------------------*/
#if 0
void TeleportNetPlayerToAStartingPosition(STRATEGYBLOCK *playerSbPtr, int startOfGame)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int numStartPositions;
	VECTORCH* startPositions;

	int sbIndex = 0;
	int numReadThro,numConsidered;
	int start_index=0;
	int found = 0;
	
	PLAYER_STATUS *psPtr=(PLAYER_STATUS*)playerSbPtr->SBdataptr;

	/* some basic checks */
	if(playerSbPtr==NULL) return;	
	if(playerSbPtr->DynPtr==NULL) return;
	if(!ActiveStBlockList) return;
	if(NumActiveStBlocks<=0) return;

	//set the players invulnerability timer
	GLOBALASSERT(psPtr);
	psPtr->invulnerabilityTimer=netGameData.invulnerableTime*ONE_FIXED;
	
		
	//select the start positions for this character type
	switch(AvP.PlayerType)
	{
		case I_Marine :
			numStartPositions=numMarineStartPos;
			startPositions=marineStartPositions;
			break;

		case I_Predator :
			numStartPositions=numPredatorStartPos;
			startPositions=predatorStartPositions;
			break;

		case I_Alien :
			numStartPositions=numAlienStartPos;
			startPositions=alienStartPositions;
			break;
	}
	
	
	if(!numStartPositions) return;

	/* pick a starting point*/
	if(startOfGame)
	{
		numReadThro = (PlayerIdInPlayerList(AVPDPNetID))%8;
		while(numReadThro<0)
		{
			//Probably haven't received the details of all the players yet
			MinimalNetCollectMessages();
			numReadThro = (PlayerIdInPlayerList(AVPDPNetID))%8;
		}
	}
	else numReadThro = FastRandom()%numStartPositions;
	numConsidered = 0;

	/* and go through the list */
	for(start_index=numReadThro;!found;start_index++)
	{	
		start_index%=numStartPositions;
		
		/* we are going to try this one: see if it's clear */
		{
			int sbIndex2 = 0;
			STRATEGYBLOCK *sbPtr2;
			int obstructed = 0;

			while((sbIndex2 < NumActiveStBlocks)&&(!obstructed))
			{	
				sbPtr2 = ActiveStBlockList[sbIndex2++];
				if(sbPtr2->I_SBtype==I_BehaviourNetGhost)
				{
					NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr2->SBdataptr;
					LOCALASSERT(ghostData);			
					if((ghostData->type==I_BehaviourMarinePlayer)||
				   	   (ghostData->type==I_BehaviourPredatorPlayer)||
				   	   (ghostData->type==I_BehaviourAlienPlayer))
					{
						VECTORCH seperationVec;
						LOCALASSERT(sbPtr2->DynPtr);
						seperationVec = sbPtr2->DynPtr->Position;
						seperationVec.vx -= startPositions[start_index].vx;
						seperationVec.vy -= startPositions[start_index].vy;
						seperationVec.vz -= startPositions[start_index].vz;					
						if((Magnitude(&seperationVec)<2000)&&(numConsidered<8)) obstructed = 1;
					}
				}
			}				
		
			numConsidered++;
			if(!obstructed)
			{
				/* found a clear start position */
				playerSbPtr->DynPtr->Position = playerSbPtr->DynPtr->PrevPosition = startPositions[start_index];
				found = 1;
				
			}
		}
	}	
}
#else
void TeleportNetPlayerToAStartingPosition(STRATEGYBLOCK *playerSbPtr, int startOfGame)
{
	int numStartPositions;
	MULTIPLAYER_START* startPositions;
	int start_index;
	int numChecked=0;
	int bestDistance=-1;
	int bestIndex=-1;
	
	PLAYER_STATUS *psPtr=(PLAYER_STATUS*)playerSbPtr->SBdataptr;

	if(MultiplayerRestartSeed)
	{
		StartOfGame_PlayerPlacement(playerSbPtr,MultiplayerRestartSeed);
		MultiplayerRestartSeed=0;	
		return;
	}
	
	/* some basic checks */
	if(playerSbPtr==NULL) return;	
	if(playerSbPtr->DynPtr==NULL) return;

	//set the players invulnerability timer
	GLOBALASSERT(psPtr);
	psPtr->invulnerabilityTimer=netGameData.invulnerableTime*ONE_FIXED;
	
		
	//select the start positions for this character type
	switch(AvP.PlayerType)
	{
		case I_Marine :
			numStartPositions=numMarineStartPos;
			startPositions=marineStartPositions;
			break;

		case I_Predator :
			numStartPositions=numPredatorStartPos;
			startPositions=predatorStartPositions;
			break;

		case I_Alien :
			numStartPositions=numAlienStartPos;
			startPositions=alienStartPositions;
			break;
	}
	
	
	if(!numStartPositions) return;

	for(start_index=FastRandom()%numStartPositions;numChecked<numStartPositions;start_index++,numChecked++)
	{
		int closestDistance=0x7fffffff;
		int playerIndex;
		
		if(start_index>=numStartPositions) start_index=0;
		
		//find the closest player to this start position
		for(playerIndex=0;playerIndex<NET_MAXPLAYERS;playerIndex++)
		{
			if(netGameData.playerData[playerIndex].playerId &&
			   netGameData.playerData[playerIndex].playerId!=AVPDPNetID &&	
			   netGameData.playerData[playerIndex].playerAlive)
			{
				VECTORCH seperationVec;
				int distance;
				
				seperationVec = netGameData.playerData[playerIndex].lastKnownPosition;
				seperationVec.vx -= startPositions[start_index].location.vx;
				seperationVec.vy -= startPositions[start_index].location.vy;
				seperationVec.vz -= startPositions[start_index].location.vz;					

				distance=Approximate3dMagnitude(&seperationVec);
				if(distance<closestDistance)
				{
					closestDistance=distance;
				}
			}	

		}
		
		
		if(closestDistance>20000)				
		{
			//no player near this start position, so it will do
			bestIndex=start_index;
			break;
		}
		else if(closestDistance>bestDistance)
		{
			bestDistance=closestDistance;
			bestIndex=start_index;
		}
	}

	if(bestIndex!=-1)
	{
		extern MODULE * playerPherModule;
		DYNAMICSBLOCK *dynPtr = playerSbPtr->DynPtr;
		//found a start postion
		Player->ObWorld=dynPtr->Position = dynPtr->PrevPosition = startPositions[bestIndex].location;
		dynPtr->OrientEuler = startPositions[bestIndex].orientation;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);

		playerSbPtr->containingModule=ModuleFromPosition(&Player->ObWorld, (MODULE*)0);
		playerPherModule=playerSbPtr->containingModule;

	
		/*
		After teleportation we need to update the visibilities , to ensure that
		dynamics will work properly this frame.
		*/	 
		{
			extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;
			Global_VDB_Ptr->VDB_World = Player->ObWorld;
			AllNewModuleHandler();
			DoObjectVisibilities();
		}
		
	}

}
#endif

/*Works out everyone's starting positions . Use a shared random numer seed
in order to avoid having several players appearing at the same place*/
void StartOfGame_PlayerPlacement(STRATEGYBLOCK *playerSbPtr,int seed)
{
	int numStartPositions;
	MULTIPLAYER_START* startPositions;
	PLAYER_STATUS *psPtr=(PLAYER_STATUS*)playerSbPtr->SBdataptr;
	
	VECTORCH ChosenPositions[NET_MAXPLAYERS];
	int NumChosen=0;
	int i;

	//set the players invulnerability timer
	GLOBALASSERT(psPtr);
	psPtr->invulnerabilityTimer=netGameData.invulnerableTime*ONE_FIXED;
	
	SetSeededFastRandom(seed);

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		int index;
		int numChecked=0;
		if(!netGameData.playerData[i].playerId) continue;

		switch(netGameData.playerData[i].characterType)
		{
			case NGCT_Marine :
				numStartPositions=numMarineStartPos;
				startPositions=marineStartPositions;
				break;
			
			case NGCT_Alien :
				numStartPositions=numAlienStartPos;
				startPositions=alienStartPositions;
				break;
			
			case NGCT_Predator :
				numStartPositions=numPredatorStartPos;
				startPositions=predatorStartPositions;
				break;
			
			default :
				continue;//hmm
				break;
		}

		if(!numStartPositions) continue;

		for(index=SeededFastRandom()%numStartPositions;numChecked<numStartPositions;index++,numChecked++)
		{
			int j;
			if(index>=numStartPositions) index=0;

			//see if anyone is at this position
			for(j=0;j<NumChosen;j++)
			{
				if(ChosenPositions[j].vx==startPositions[index].location.vx &&
				   ChosenPositions[j].vy==startPositions[index].location.vy &&
				   ChosenPositions[j].vz==startPositions[index].location.vz)
				{
					break;
				}	
			}

			if(j==NumChosen)
			{
				//found a clear start position
				break;
			}
		}
		
		if(index>=numStartPositions) index=0;

		//we've either found a clear start position , or gone through the entire list
		//without finding one.Either way take the starting position index that we reached

		ChosenPositions[NumChosen]=startPositions[index].location;
		NumChosen++;
		
		if(netGameData.playerData[i].playerId==AVPDPNetID)
		{
			extern MODULE * playerPherModule;
			//this was our new start position
			DYNAMICSBLOCK *dynPtr = playerSbPtr->DynPtr;
			//found a start postion
			Player->ObWorld=dynPtr->Position = dynPtr->PrevPosition = startPositions[index].location;
			dynPtr->OrientEuler = startPositions[index].orientation;
			CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
			TransposeMatrixCH(&dynPtr->OrientMat);

			playerSbPtr->containingModule=ModuleFromPosition(&Player->ObWorld, (MODULE*)0);
			playerPherModule=playerSbPtr->containingModule;

			/*
			After teleportation we need to update the visibilities , to ensure that
			dynamics will work properly this frame.
			*/
			{
				extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;
				Global_VDB_Ptr->VDB_World = Player->ObWorld;
				AllNewModuleHandler();
				DoObjectVisibilities();
			}
			//don't care about the positioning of anyone further on in the list
			return;

		}

		
	}
}

/* Patrick 4/8/97-------------------------------------
   For testing purposes...
   ---------------------------------------------------*/
#define logNetGameProcesses 0
#if logNetGameProcesses
static FILE *netLogfile;
#endif
void InitNetLog(void)
{
#if logNetGameProcesses
		netLogfile = fopen("NETINFO.TXT","w");
		fprintf(netLogfile, "NETGAME DEBUGGING LOG \n \n");
		fclose(netLogfile);
#endif
}

void LogNetInfo(char *msg)
{
#if logNetGameProcesses
		if(!msg) return;
		netLogfile = fopen("NETINFO.TXT","a");
		fprintf(netLogfile, msg);
		fclose(netLogfile);
#endif
}



DISPLAYBLOCK PlayersMirrorImage;
STRATEGYBLOCK PlayersMirrorImageSB;		  
NETGHOSTDATABLOCK PlayersMirrorGhost;
DYNAMICSBLOCK PlayersMirrorDynBlock;

BOOL Current_Level_Requires_Mirror_Image()
{
	extern char LevelName[];
	if ( (!stricmp(LevelName,"e3demo")) || (!stricmp(LevelName,"e3demosp")) || (!stricmp(LevelName,"derelict")) )
	{
 		return TRUE;
	}
	return FALSE;
}


void CreatePlayersImageInMirror(void)
{
	AVP_BEHAVIOUR_TYPE type;
	STRATEGYBLOCK *sbPtr = &PlayersMirrorImageSB;
	NETGHOSTDATABLOCK *ghostData = &PlayersMirrorGhost; 
	PlayersMirrorImage.ObStrategyBlock = sbPtr;
	
	sbPtr->SBdptr = &PlayersMirrorImage;

	sbPtr->SBdataptr = (void *)ghostData;
	sbPtr->DynPtr = &PlayersMirrorDynBlock;
	
	switch(AvP.PlayerType)
	{
		case(I_Marine):
		{
			type = I_BehaviourMarinePlayer;
			break;
		}
		case(I_Predator):
		{
			type = I_BehaviourPredatorPlayer;
			break;
		}
		case(I_Alien):
		{
			type = I_BehaviourAlienPlayer;
			break;
		}
	}
	ghostData->type = type;
	ghostData->IOType=IOT_Non;
	ghostData->subtype=0;
	ghostData->myGunFlash = NULL;
	ghostData->SoundHandle = SOUND_NOACTIVEINDEX;
	ghostData->currentAnimSequence = 0;
	ghostData->CloakingEffectiveness = 0;
	ghostData->IgnitionHandshaking = 0;
	ghostData->soundStartFlag = 0;
	
	if(AvP.Network == I_No_Network)
	{
		ghostData->playerId=0;
	}
	else
	{
		ghostData->playerId=AVPDPNetID;
	}

	/* set the shape */

	switch(type)
	{
		case(I_BehaviourMarinePlayer):
		{
			CreateMarineHModel(ghostData,WEAPON_PULSERIFLE);
			break;
		}
		case(I_BehaviourAlienPlayer):
		{
			CreateAlienHModel(ghostData,AT_Standard);
			break;
		}
		case(I_BehaviourPredatorPlayer):
		{
			CreatePredatorHModel(ghostData,WEAPON_PRED_WRISTBLADE);
			break;
		}
		default:
			break;
	}
		sbPtr->SBdptr->HModelControlBlock=&ghostData->HModelController;
		ProveHModel(sbPtr->SBdptr->HModelControlBlock,sbPtr->SBdptr);

}

void DeallocatePlayersMirrorImage()
{
	#if MIRRORING_ON
	if(Current_Level_Requires_Mirror_Image())
	{
		Dispel_HModel(&PlayersMirrorGhost.HModelController);
	}
	#endif
}


void RenderPlayersImageInMirror(void)
{
	STRATEGYBLOCK *sbPtr = &PlayersMirrorImageSB;
	NETGHOSTDATABLOCK *ghostData = &PlayersMirrorGhost; 
	
	int sequence;
	int weapon;
	int firingPrimary;
	int firingSecondary;

	switch(AvP.PlayerType)
	{
		case(I_Marine):
		{
			sequence = (unsigned char)GetMyMarineSequence();
			//check for change of charcter type
			if(ghostData->type!=I_BehaviourMarinePlayer)
			{
				ghostData->type=I_BehaviourMarinePlayer;
				//settings currentweapon to -1 will forec the hmodel to be updated
				ghostData->CurrentWeapon=-1;
			}
			
			break;
		}
		case(I_Predator):
		{
			sequence = (unsigned char)GetMyPredatorSequence();
			//check for change of charcter type
			if(ghostData->type!=I_BehaviourPredatorPlayer)
			{
				ghostData->type=I_BehaviourPredatorPlayer;
				//settings currentweapon to -1 will forec the hmodel to be updated
				ghostData->CurrentWeapon=-1;
			}
			break;
		}
		case(I_Alien):
		{
			sequence = (unsigned char)GetMyAlienSequence();
			//check for change of charcter type
			if(ghostData->type!=I_BehaviourAlienPlayer)
			{
				ghostData->type=I_BehaviourAlienPlayer;
				//setting currentweapon to -1 will force the hmodel to be updated
				ghostData->CurrentWeapon=-1;
			}
			break;
		}
		default:
		{
			LOCALASSERT(1==0);
			break;
		}
	}
		/* my current weapon id, and whether I am firing it... */
	{
		PLAYER_WEAPON_DATA *weaponPtr;
 		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);    	        
    	weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
		weapon = (signed char)(weaponPtr->WeaponIDNumber);	

		if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)&&(playerStatusPtr->IsAlive))		
			firingPrimary = 1;
		else firingPrimary = 0;
		if((weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY)&&(playerStatusPtr->IsAlive)) 
			firingSecondary = 1;
		else firingSecondary = 0;
	}

//		if(!(((!(messagePtr->IAmAlive)))&&(netGameData.playerData[playerIndex].characterType==NGCT_Alien)))
	{
		{
			PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
			HandleWeaponElevation(sbPtr,playerStatusPtr->ViewPanX,weapon);
		
			UpdateGhost(sbPtr,&(Player->ObStrategyBlock->DynPtr->Position),&(Player->ObStrategyBlock->DynPtr->OrientEuler),sequence,AreTwoPistolsInTertiaryFire());


			MaintainGhostCloakingStatus(sbPtr,(int)playerStatusPtr->cloakOn);
		}
	}
	{
		extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
		DISPLAYBLOCK *dPtr = &PlayersMirrorImage;
		dPtr->ObWorld = PlayersMirrorDynBlock.Position;
		dPtr->ObMat = PlayersMirrorDynBlock.OrientMat;	
		ReflectObject(dPtr);	 

		PlayersMirrorImage.ObStrategyBlock = 0;
		
		AddShape(dPtr,Global_VDB_Ptr);
		PlayersMirrorImage.ObStrategyBlock = &PlayersMirrorImageSB;
	
	}
	HandleGhostGunFlashEffect(sbPtr,MyPlayerHasAMuzzleFlash(sbPtr));
}


/* KJL 15:32:17 24/05/98 - respawn all game objects that have been destroyed */
void RestartNetworkGame(int seed)
{
	int i,j;
	if(netGameData.myGameState!=NGS_Playing && netGameData.myGameState!=NGS_EndGameScreen) return;

	//reset all the scores

	for(i=0;i<(NET_MAXPLAYERS);i++)
	{
		for(j=0;j<(NET_MAXPLAYERS);j++) netGameData.playerData[i].playerFrags[j] = 0;
		netGameData.playerData[i].playerScore = 0;
		netGameData.playerData[i].playerScoreAgainst = 0;
		netGameData.playerData[i].aliensKilled[0]=0;
		netGameData.playerData[i].aliensKilled[1]=0;
		netGameData.playerData[i].aliensKilled[2]=0;
		netGameData.playerData[i].deathsFromAI=0;
		netGameData.playerData[i].startFlag = 0;		
		netGameData.playerData[i].playerAlive = 1;
		netGameData.playerData[i].playerHasLives = 1;
	}
	for(j=0;j<3;j++) netGameData.teamScores[j] = 0;
	
   	netGameData.stateCheckTimeDelay=0;
	netGameData.GameTimeElapsed = 0;
	netGameData.LMS_AlienIndex=-1;
	netGameData.LMS_RestartTimer=0;
	netGameData.numDeaths[0]=0;
	netGameData.numDeaths[1]=0;
	netGameData.numDeaths[2]=0;


	netGameData.lastPointsBasedRespawn=0;

	AvP.RestartLevel=1;


	
	if(netGameData.gameType==NGT_LastManStanding)
	{
		int i;
		//in a last man standing game , turn everyone in marines to start with
		extern void ChangeToMarine();
		ChangeToMarine();

		for(i=0;i<NET_MAXPLAYERS;i++)
		{
			netGameData.playerData[i].characterType=NGCT_Marine;
		}
	}
	TurnOffMultiplayerObserveMode();
	
	//go to a new start position
//	StartOfGame_PlayerPlacement(Player->ObStrategyBlock, seed);

	netGameData.myGameState=NGS_Playing;

	//record seed for later (used during restart level process)
	MultiplayerRestartSeed=seed;
	if(!MultiplayerRestartSeed) MultiplayerRestartSeed=1;

}




/* KJL 15:46:19 09/04/98 - processing info pertaining to multiplayer games */

static void Inform_PlayerHasDied(DPID killer, DPID victim,NETGAME_CHARACTERTYPE killerType,char weaponIcon)
{
	int victimIndex = PlayerIdInPlayerList(victim);

	/* KJL 15:35:38 09/04/98 - not knowing who the victim is what make things a bit awkward... */
	if(victimIndex==NET_IDNOTINPLAYERLIST) return;

	
	switch(killerType)
	{
		case NGCT_AI_Alien :
		{
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_KILLEDBY_ALIEN,netGameData.playerData[victimIndex].name,0);
			break;
		}
		
		case NGCT_AI_Predalien :
		{
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_KILLEDBY_PREDALIEN,netGameData.playerData[victimIndex].name,0);
			break;
		}
		
		case NGCT_AI_Praetorian :
		{
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_KILLEDBY_PRAETORIAN,netGameData.playerData[victimIndex].name,0);
			break;
		}

		default :
		{
			/* KJL 15:36:03 09/04/98 - killer should be set to null if it's a suicide */
			/*killer==vitim means suicide now ,as well*/
			if (killer && killer!=victim)
			{
				int killerIndex = PlayerIdInPlayerList(killer);
				
				if(killerIndex!=NET_IDNOTINPLAYERLIST)
				{
					char weaponSymbol[5]="";
					if(weaponIcon)
					{
						sprintf(weaponSymbol," %c",weaponIcon);
					}
					NetworkGameConsoleMessageWithWeaponIcon(TEXTSTRING_MULTIPLAYERCONSOLE_KILLEDBY,netGameData.playerData[victimIndex].name,netGameData.playerData[killerIndex].name,weaponSymbol);
				}
			}
			else
			{
				NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_SUICIDE,netGameData.playerData[victimIndex].name,0);
			}
			break;
		}
	}
}

static void Inform_AiHasDied(DPID killer,ALIEN_TYPE type,char weaponIcon)
{
	int killerIndex = PlayerIdInPlayerList(killer);
	
	if(killerIndex!=NET_IDNOTINPLAYERLIST)
	{
		char weaponSymbol[5]="";
		if(weaponIcon)
		{
			sprintf(weaponSymbol," %c",weaponIcon);
		}
		switch(type)
		{
			case AT_Standard :
			{
				NetworkGameConsoleMessageWithWeaponIcon(TEXTSTRING_MULTIPLAYERCONSOLE_ALIEN_KILLED,netGameData.playerData[killerIndex].name,0,weaponSymbol);
	  			break;
			}
			
			case AT_Predalien :
			{
				NetworkGameConsoleMessageWithWeaponIcon(TEXTSTRING_MULTIPLAYERCONSOLE_PREDALIEN_KILLED,netGameData.playerData[killerIndex].name,0,weaponSymbol);
				break;
			}
			
			case AT_Praetorian :
			{
				NetworkGameConsoleMessageWithWeaponIcon(TEXTSTRING_MULTIPLAYERCONSOLE_PRAETORIAN_KILLED,netGameData.playerData[killerIndex].name,0,weaponSymbol);
				break;
			}
		}
	}
}

static void Inform_PlayerHasLeft(DPID player)
{
	int playerIndex = PlayerIdInPlayerList(player);

	/* KJL 15:35:38 09/04/98 - not knowing who the player is what make things a bit awkward... */
	if(playerIndex==NET_IDNOTINPLAYERLIST) return;

	NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_LEAVEGAME,netGameData.playerData[playerIndex].name,0);

}
static void Inform_PlayerHasJoined(DPID player)
{
	int playerIndex = PlayerIdInPlayerList(player);

	/* KJL 15:35:38 09/04/98 - not knowing who the player is what make things a bit awkward... */
	if(playerIndex==NET_IDNOTINPLAYERLIST) return;

	NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_JOINGAME,netGameData.playerData[playerIndex].name,0);
}
static void Inform_PlayerHasConnected(DPID player)
{
	int playerIndex = PlayerIdInPlayerList(player);

	/* KJL 15:35:38 09/04/98 - not knowing who the player is what make things a bit awkward... */
	if(playerIndex==NET_IDNOTINPLAYERLIST) return;

	NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_CONNECTGAME,netGameData.playerData[playerIndex].name,0);
}

static void Inform_NewHost(void)
{
	NewOnScreenMessage(GetTextString(TEXTSTRING_MULTIPLAYERCONSOLE_NEWHOST));
}




static void WriteFragmentStatus(int fragmentNumber, int status)
{
	int n = fragmentNumber>>3;
	int r = fragmentNumber - (n<<3);
	unsigned char *ptr = &FragmentalObjectStatus[n];
	unsigned char mask = 1<<r;

	if (status)
	{
		*ptr |= mask;
	}
	else
	{
		*ptr &= ~mask;
	}
}
static int ReadFragmentStatus(int fragmentNumber)
{
	int n = fragmentNumber>>3;
	int r = fragmentNumber - (n<<3);
	unsigned char *ptr = &FragmentalObjectStatus[n];
	unsigned char mask = 1<<r;

	return((*ptr)&mask);
}

static void WriteStrategySynch(int objectNumber, int status)
{
	int n = objectNumber>>2;
	int shift = (objectNumber - (n<<2))*2;
	unsigned char *ptr = &StrategySynchArray[n];
	unsigned char mask = 3<<shift;

	*ptr &=~ mask;
	*ptr |= (status & 3)<<shift;

}
static int ReadStrategySynch(int objectNumber)
{
	int n = objectNumber>>2;
	int shift = (objectNumber - (n<<2))*2;
	unsigned char *ptr = &StrategySynchArray[n];
	unsigned char mask = 3<<shift;

	return(((*ptr)&mask)>>shift);
}



#if 1
static int GetDynamicScoreMultiplier(int playerKilledIndex,int killerIndex)
{
	int scoreFor;
	int scoreAgainst;
	int mult;
	int playerCount=0;
	int i;


	GLOBALASSERT(playerKilledIndex!=killerIndex);

	scoreFor=netGameData.playerData[playerKilledIndex].playerScore;
	scoreAgainst=netGameData.playerData[playerKilledIndex].playerScoreAgainst;

	scoreFor=max(500,scoreFor+500);
	scoreAgainst=max(500,scoreAgainst+500);
	
	//count players
	for(i=0;i<NET_MAXPLAYERS;i++) 	
	{
		if(netGameData.playerData[i].playerId==0) continue;
		playerCount++;
	}
	//only bother if there are at least 3 players
	if(playerCount<3) return ONE_FIXED;


	//value of player depends on comparing player's score with the number of points scored against that player
	if(scoreFor>scoreAgainst)
	{
		int ratio=DIV_FIXED(scoreFor,scoreAgainst);
		mult=DIV_FIXED(10*ratio,9*ONE_FIXED+ratio);
		if(mult<ONE_FIXED) mult=ONE_FIXED;
	}
	else
	{
		int ratio=DIV_FIXED(scoreAgainst,scoreFor);
		mult=DIV_FIXED(9*ONE_FIXED+ratio,10*ratio);
		if(mult>ONE_FIXED) mult=ONE_FIXED;
	}


	return mult;
	
}
#else
static int GetDynamicScoreMultiplier(int playerKilledIndex,int killerIndex)
{
	int i;
	int scoreTotal=0;
	int playerCount=0;
	int playerKilledScore;
	int mult;

	GLOBALASSERT(playerKilledIndex!=killerIndex);
	
	//add up score of all players
	for(i=0;i<NET_MAXPLAYERS;i++) 	
	{
		if(netGameData.playerData[i].playerId==NULL) continue;
		
		

		playerCount++;
		//give everyone a minimum score of 500 for the purpose of this calculation
		scoreTotal+=max(500,netGameData.playerData[i].playerScore+500);
	}
	if(playerCount<3 || !scoreTotal) return ONE_FIXED;

	playerKilledScore=max(500,netGameData.playerData[playerKilledIndex].playerScore+500);

	//get average score of all players other than killed player
	playerCount--;
	scoreTotal=(scoreTotal-playerKilledScore)/playerCount;

	if(playerKilledScore>scoreTotal)
	{
		int ratio=DIV_FIXED(playerKilledScore,scoreTotal);
		mult=DIV_FIXED(10*ratio,10*ONE_FIXED+ratio);
		if(mult<ONE_FIXED) mult=ONE_FIXED;
	}
	else
	{
		int ratio=DIV_FIXED(scoreTotal,playerKilledScore);
		mult=DIV_FIXED(10*ONE_FIXED+ratio,10*ratio);
		if(mult>ONE_FIXED) mult=ONE_FIXED;
	}


	return mult;
	
}
#endif
static int GetNetScoreForKill(int playerKilledIndex,int killerIndex)
{
	NETGAME_CHARACTERTYPE killerType=netGameData.playerData[killerIndex].characterType;
	NETGAME_CHARACTERTYPE playerKilledType=netGameData.playerData[playerKilledIndex].characterType;

	int score=netGameData.baseKillValue;

	if(playerKilledIndex==killerIndex)	
	{
		//suicide
		return -netGameData.baseKillValue;
	}

	if(netGameData.gameType==NGT_PredatorTag)
	{
		//only the predator can score
		if(killerType!=NGCT_Predator) return 0;
	}
	else if(netGameData.gameType==NGT_AlienTag)
	{
		//only the alien can score
		if(killerType!=NGCT_Alien) return 0;
	}
	else if(netGameData.gameType==NGT_CoopDeathmatch)
	{
		//have we killed someone on the same team
		if(killerType==playerKilledType)
		{
			return -netGameData.baseKillValue;
		}
	}
	
	if(netGameData.useCharacterKillValues)
	{
		score=netGameData.characterKillValues[playerKilledType];
	}

	if(netGameData.useDynamicScoring && score>0)
	{
		int dynamicScoreMult=GetDynamicScoreMultiplier(playerKilledIndex,killerIndex);
		score=MUL_FIXED(score,dynamicScoreMult);
   		//make sure player gets at least one point
		if(score<1) score=1;
	}


	return score;

}


static void CheckLastManStandingState()
{
	//make sure that ther is at least 1 alien and 1 non-alien
	int alienCount=0;
	int nonAlienCount=0;
	int alienIndex;
	int i;

	if(netGameData.stateCheckTimeDelay>0)
	{
		//still too soon after last restart to check again
		netGameData.stateCheckTimeDelay-=RealFrameTime;
		return;
	}

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId)
		{
			if(netGameData.playerData[i].characterType==NGCT_Alien)
			{
				alienCount++;
			}
			else
			{
				nonAlienCount++;
			}
		}
	}

	//if there is only one player forget it
	if((alienCount+nonAlienCount)<2) return;

	if(alienCount && nonAlienCount)
	{
		//the game is still going
		//make sure the restarttimer is 0
		netGameData.LMS_RestartTimer=0;
		return;
	}

	
	
	//find out who the next alien will be
	for(alienIndex=netGameData.LMS_AlienIndex+1;;alienIndex++)
	{
		if(alienIndex>=NET_MAXPLAYERS)
		{
			//everyone has had a turn as an alien 
			TransmitEndOfGameNetMsg();
			netGameData.myGameState = NGS_EndGameScreen;
			return;
		}

		if(netGameData.playerData[alienIndex].playerId)
		{
			//this player will be our new alien
			break;
		}
	}
	
	//game needs to be restarted.
	//has the countdown started?
	if(netGameData.LMS_RestartTimer>0)
	{
		int secondsBefore,secondsAfter;
		secondsBefore=(netGameData.LMS_RestartTimer+ONE_FIXED-1)/ONE_FIXED;
		netGameData.LMS_RestartTimer-=RealFrameTime;
		secondsAfter=(netGameData.LMS_RestartTimer+ONE_FIXED-1)/ONE_FIXED;

		//has the timer reached 0?
		if(netGameData.LMS_RestartTimer<0)
		{
			//get a random number seed for starting position
			int seed=FastRandom();
			netGameData.LMS_RestartTimer=0;
		
			netGameData.LMS_AlienIndex=alienIndex;

			AddNetMsg_LastManStanding_Restart(netGameData.playerData[alienIndex].playerId,seed);
			Handle_LastManStanding_Restart(netGameData.playerData[alienIndex].playerId,seed);

			//set time delay until we next check for need to restart
			//this gives time for the other players to get their messages
			netGameData.stateCheckTimeDelay=5*ONE_FIXED;
		}
		else
		{
			//if the timer has gone down another second , tell everyone
			if(secondsAfter<secondsBefore)
			{
				AddNetMsg_LastManStanding_RestartTimer((char)secondsAfter);
				Handle_LastManStanding_RestartTimer((char)secondsAfter);
			}
		}
	
	}
	else
	{
		static int ReminderTimer=0;

		if(netGameData.LMS_RestartTimer==-1)
		{
			ReminderTimer -= RealFrameTime;
			if(ReminderTimer<0)
			{
				//give a reminder (forced by setting reset timer to 0)
				ReminderTimer=0;
				netGameData.LMS_RestartTimer=0;
			}
		}
		
		if(netGameData.LMS_RestartTimer!=-1)
		{
			
			//tell host to press operate
			PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_LMS_ALLMARINESDEAD);
			PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_LMS_HOSTPRESSOPERATE);
			AddNetMsg_LastManStanding_RestartTimer(255);
			netGameData.LMS_RestartTimer=-1;
			ReminderTimer = 6*ONE_FIXED;
		}
		
		//wait for host to press operate

		{
			PLAYER_STATUS *psPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
			if(psPtr->Mvt_InputRequests.Flags.Rqst_Operate)
			{
				
				//say who the next alien will be
				AddNetMsg_PlayerID(netGameData.playerData[alienIndex].playerId,NetMT_LastManStanding_RestartInfo);
				Handle_LastManStanding_RestartInfo(netGameData.playerData[alienIndex].playerId);
				
				//start the restart timer
				netGameData.LMS_RestartTimer=4*ONE_FIXED;
			}
		}

	}
	
}

static void Handle_LastManStanding_Restart(DPID alienID,int seed)
{
	int i;
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	
	if(AVPDPNetID==alienID)
	{
		//become an alien
		extern void ChangeToAlien();
		ChangeToAlien();
	}
	else
	{
		//become a marine
		extern void ChangeToMarine();
		ChangeToMarine();
	}

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId==alienID)
		{
			netGameData.playerData[i].characterType=NGCT_Alien;
		}
		else
		{
			netGameData.playerData[i].characterType=NGCT_Marine;
		}
	}
	/*
	//go to an new start position
	StartOfGame_PlayerPlacement(Player->ObStrategyBlock,seed);
	
	//restore all the destroyed objects.
	RespawnAllObjects();
	*/

	MultiplayerRestartSeed=seed;
	AvP.RestartLevel=1;
	
	PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_LMS_GO);
}

static void Handle_LastManStanding_RestartInfo(DPID alienID)
{
	int i;
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	

	//find the alien's name
	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId==alienID)
		{
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYER_LMS_WILLBEALIEN,netGameData.playerData[i].name,0);
		}
	}
}

static void Handle_LastManStanding_LastMan(DPID marineID)
{
	int i;
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	
	
	//find the marine's name
	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId==marineID)
		{
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYER_LMS_LASTMANSTANDING,netGameData.playerData[i].name,0);
		}
	}
}

static void Handle_LastManStanding_RestartTimer(unsigned char time)
{
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;	
	if((int)time==255)
	{
		//tell player to wait
		PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_LMS_ALLMARINESDEAD);
		PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_LMS_WAITFORHOST);
	}
	else
	{
		//show countdown
		sprintf(OnScreenMessageBuffer,"%d...",time );
		NewOnScreenMessage(OnScreenMessageBuffer);
	}

}





static int CountPlayersOfType(NETGAME_CHARACTERTYPE species)
{
	int i;
	int numPredators=0;
		
	for(i=0;i<(NET_MAXPLAYERS);i++)
	{
		if(netGameData.playerData[i].playerId)
		{
			if(netGameData.playerData[i].characterType==species)
			{
				numPredators++;
			}
		}
	}

	return numPredators;
}

static void CheckSpeciesTagState()
{
	int i;
	NETGAME_CHARACTERTYPE tagSpecies=NGCT_Predator;
	if(netGameData.gameType==NGT_AlienTag) tagSpecies=NGCT_Alien;

	//make sure that there is at least one predator in the game
	if(netGameData.stateCheckTimeDelay>0)
	{
		netGameData.stateCheckTimeDelay-=RealFrameTime;
		return;
	}
	
	if(CountPlayersOfType(tagSpecies)) return;

	//we need to choose a predator player
	//make it the person with the lowest score
	{
		DPID predID=0;
		int lowScore=1000000000;
		for(i=0;i<(NET_MAXPLAYERS);i++)
		{
			if(netGameData.playerData[i].playerId)
			{
				if(netGameData.playerData[i].playerScore<lowScore)
				{
					lowScore=netGameData.playerData[i].playerScore;
					predID=netGameData.playerData[i].playerId;
				}
			}

		}
		AddNetMsg_PlayerID(predID,NetMT_PredatorTag_NewPredator);
		Handle_SpeciesTag_NewPersonIt(predID);
		

	}
	
}


static void Handle_SpeciesTag_NewPersonIt(DPID predatorID)
{
	/* if we're not playing, ignore it */
	if(netGameData.myGameState!=NGS_Playing) return;
		
	if(AVPDPNetID==predatorID)
	{
		//become aa predator or alien
		if(netGameData.gameType==NGT_PredatorTag)
		{
			extern void ChangeToPredator();
			ChangeToPredator();
		}
		else if(netGameData.gameType==NGT_AlienTag)
		{
			extern void ChangeToAlien();
			ChangeToAlien();
		}
	}

	netGameData.stateCheckTimeDelay=5*ONE_FIXED;
}

void ChangeNetGameType_Individual()
{
	if(AvP.Network==I_No_Network) return;
	netGameData.gameType=NGT_Individual;
}

void ChangeNetGameType_Coop()
{
	if(AvP.Network==I_No_Network) return;
	netGameData.gameType=NGT_CoopDeathmatch;
}

void ChangeNetGameType_LastManStanding()
{
	if(AvP.Network==I_No_Network) return;
	netGameData.gameType=NGT_LastManStanding;
	netGameData.LMS_AlienIndex=0;
	netGameData.stateCheckTimeDelay=0;
}

void ChangeNetGameType_PredatorTag()
{
	if(AvP.Network==I_No_Network) return;
	netGameData.gameType=NGT_PredatorTag;
}


/*
	This function takes a string from the list of language localised strings, and replaces
	instances of %1 and %2 with the appropriate names.
	It then displays the message in the console.
*/
static void NetworkGameConsoleMessage(enum TEXTSTRING_ID stringID,const char* name1,const char* name2)
{
	char* string;
	char* messageptr=&OnScreenMessageBuffer[0];

	string=GetTextString(stringID);
	if(!string) return;

	while(*string)
	{
		if(string[0]=='%')
		{
			if(string[1]>='1' && string[1]<='9')
			{
				if(string[1]=='1' && name1)
				{
					strcpy(messageptr,name1);
					messageptr+=strlen(name1);
				}
				if(string[1]=='2' && name2)
				{
					strcpy(messageptr,name2);
					messageptr+=strlen(name2);
				}

				string+=2;
				continue;
			}
		}
		*messageptr++=*string++;
	}
	*messageptr=0;

	NewOnScreenMessage(OnScreenMessageBuffer);
}

static void NetworkGameConsoleMessageWithWeaponIcon(enum TEXTSTRING_ID stringID,const char* name1,const char* name2,const char* weaponSymbol)
{
	char* string;
	char* messageptr=&OnScreenMessageBuffer[0];

	string=GetTextString(stringID);
	if(!string) return;

	while(*string)
	{
		if(string[0]=='%')
		{
			if(string[1]>='1' && string[1]<='9')
			{
				if(string[1]=='1' && name1)
				{
					strcpy(messageptr,name1);
					messageptr+=strlen(name1);
				}
				if(string[1]=='2' && name2)
				{
					strcpy(messageptr,name2);
					messageptr+=strlen(name2);
				}


				string+=2;
				continue;
			}
		}
		*messageptr++=*string++;
	}
	*messageptr=0;

	if(weaponSymbol)
	{
		strcat(OnScreenMessageBuffer,weaponSymbol);
	}

	NewOnScreenMessage(OnScreenMessageBuffer);
}

static int CharacterTypesAvailable[NUM_PC_TYPES];
static int CharacterSubTypesAvailable[NUM_PC_SUBTYPES];

int DetermineAvailableCharacterTypes(BOOL ConsiderUsedCharacters)
{
	int i;
	int maxMarines=0;
	
	//set limits for disallowed marine types to zero
	if(!netGameData.allowSmartgun) netGameData.maxMarineSmartgun=0;
	if(!netGameData.allowFlamer) netGameData.maxMarineFlamer=0;
	if(!netGameData.allowMinigun) netGameData.maxMarineMinigun=0;
	if(!netGameData.allowSadar) netGameData.maxMarineSadar=0;
	if(!netGameData.allowGrenadeLauncher) netGameData.maxMarineGrenade=0;
	if(!netGameData.allowSmartDisc) netGameData.maxMarineSmartDisc=0;
	if(!netGameData.allowPistols) netGameData.maxMarinePistols=0;

	if(netGameData.gameType==NGT_PredatorTag)
	{
		//force limit of one predator
		netGameData.maxPredator=1;
	}
	else if(netGameData.gameType==NGT_AlienTag)
	{
		//force limit of one alien
		netGameData.maxAlien=1;
	}
	else if(netGameData.gameType==NGT_LastManStanding)
	{
		//no predators are allowed
		netGameData.maxPredator=0;
		//the host will be the first alien (and so is the only person that can select alien on the starting screen)
		netGameData.maxAlien=1;
	}
	else if(netGameData.gameType==NGT_Coop)
	{
		//no pc aliens allowed in coop games
		netGameData.maxAlien=0;
	}

	if(netGameData.skirmishMode)
	{
		//Skirmish mode - player can be anything except an alien
		netGameData.maxAlien = 0;
		netGameData.maxPredator=8;
		netGameData.maxMarine=8;

		netGameData.maxMarineGeneral=8;
		netGameData.maxMarinePulseRifle=8;
		netGameData.maxMarineSmartgun=8;
		netGameData.maxMarineFlamer=8;
		netGameData.maxMarineSadar=8;
		netGameData.maxMarineGrenade=8;
		netGameData.maxMarineMinigun=8;
		netGameData.maxMarineSmartDisc=8;
		netGameData.maxMarinePistols=8;
	}
	
	CharacterTypesAvailable[NGCT_Marine]=netGameData.maxMarine;
	CharacterTypesAvailable[NGCT_Alien]=netGameData.maxAlien;
	CharacterTypesAvailable[NGCT_Predator]=netGameData.maxPredator;

	CharacterSubTypesAvailable[NGSCT_General]=netGameData.maxMarineGeneral;
	CharacterSubTypesAvailable[NGSCT_PulseRifle]=netGameData.maxMarinePulseRifle;
	CharacterSubTypesAvailable[NGSCT_Smartgun]=netGameData.maxMarineSmartgun;
	CharacterSubTypesAvailable[NGSCT_Flamer]=netGameData.maxMarineFlamer;
	CharacterSubTypesAvailable[NGSCT_Sadar]=netGameData.maxMarineSadar;
	CharacterSubTypesAvailable[NGSCT_GrenadeLauncher]=netGameData.maxMarineGrenade;
	CharacterSubTypesAvailable[NGSCT_Minigun]=netGameData.maxMarineMinigun;
	CharacterSubTypesAvailable[NGSCT_Frisbee]=netGameData.maxMarineSmartDisc;
	CharacterSubTypesAvailable[NGSCT_Pistols]=netGameData.maxMarinePistols;


	//go through all the other players to see which character types are being used
	if(ConsiderUsedCharacters)
	{
		if(AvP.Network!=I_No_Network)
		{
			//(it will still be I_No_Network while the host is setting things up)
			for(i=0;i<NET_MAXPLAYERS;i++)
			{
				if(netGameData.playerData[i].playerId && netGameData.playerData[i].playerId!=AVPDPNetID)
				{
					switch(netGameData.playerData[i].characterType)	
					{
						case NGCT_Marine :
							CharacterTypesAvailable[NGCT_Marine]--;
							switch(netGameData.playerData[i].characterSubType)
							{
								case NGSCT_General :
									CharacterSubTypesAvailable[NGSCT_General]--;
									break;

								case NGSCT_PulseRifle :
									CharacterSubTypesAvailable[NGSCT_PulseRifle]--;
									break;
								
								case NGSCT_Smartgun :
									CharacterSubTypesAvailable[NGSCT_Smartgun]--;
									break;
								
								case NGSCT_Flamer :
									CharacterSubTypesAvailable[NGSCT_Flamer]--;
									break;
								
								case NGSCT_Sadar :
									CharacterSubTypesAvailable[NGSCT_Sadar]--;
									break;
								
								case NGSCT_GrenadeLauncher :
									CharacterSubTypesAvailable[NGSCT_GrenadeLauncher]--;
									break;
								
								case NGSCT_Minigun :
									CharacterSubTypesAvailable[NGSCT_Minigun]--;
									break;
								
								case NGSCT_Frisbee :
									CharacterSubTypesAvailable[NGSCT_Frisbee]--;
									break;

								case NGSCT_Pistols :
									CharacterSubTypesAvailable[NGSCT_Pistols]--;
									break;

							}
							break;

						case NGCT_Predator :
							CharacterTypesAvailable[NGCT_Predator]--;
							break;
					
						case NGCT_Alien :
							CharacterTypesAvailable[NGCT_Alien]--;
							break;
							
						default:
							break;
					}
				}
			}
			
		}
	}
	//make sure all the limits are at least 0
	for(i=0;i<NUM_PC_TYPES;i++)
	{
		CharacterTypesAvailable[i]=max(0,CharacterTypesAvailable[i]);
	}
	for(i=0;i<NUM_PC_SUBTYPES;i++)
	{
		CharacterSubTypesAvailable[i]=max(0,CharacterSubTypesAvailable[i]);
	}


	
	//adjust the marine limits to take into account that there must be both sufficent marine slots ,
	// and also sufficient subtype slots
	maxMarines=0;
	for(i=0;i<NUM_PC_SUBTYPES;i++)
	{
		CharacterSubTypesAvailable[i]=min(CharacterSubTypesAvailable[i],CharacterTypesAvailable[NGCT_Marine]);	
		maxMarines+=CharacterSubTypesAvailable[i];
	}
	CharacterTypesAvailable[NGCT_Marine]=min(CharacterTypesAvailable[NGCT_Marine],maxMarines);

	//return the total number of players available
	return CharacterTypesAvailable[NGCT_Marine]+
		   CharacterTypesAvailable[NGCT_Alien]+
		   CharacterTypesAvailable[NGCT_Predator];

}

void GetNextAllowedSpecies(int* species,BOOL search_forwards)
{
	int count =0;

	if(AvP.Network==I_No_Network)
	{
		if(netGameData.gameType==NGT_PredatorTag)
		{
			//this computer is the host setting up a predator tag game
			//therefore must be a predator
			*species=1;
			return;
		}
		else if(netGameData.gameType==NGT_AlienTag || netGameData.gameType==NGT_LastManStanding)
		{
			//this computer is the host setting up an alien tag or last man standing game
			//therefore must be a alien
			*species=2;
			return;
		}
	}
	
	DetermineAvailableCharacterTypes(TRUE);
	do
	{
		switch(*species)
		{
			case 0:	//marine (general)
				if(CharacterSubTypesAvailable[NGSCT_General]>0) return;
				break;
				
			case 1:	//predator
				if(CharacterTypesAvailable[NGCT_Predator]>0) return;
				break;
					
			case 2:	//alien
				if(CharacterTypesAvailable[NGCT_Alien]>0) return;
				break;

			case 3: //marine (pulse rifle)
				if(CharacterSubTypesAvailable[NGSCT_PulseRifle]) return;
				break;

			case 4: //marine (smartgun)
				if(CharacterSubTypesAvailable[NGSCT_Smartgun]) return;
				break;

			case 5: //marine (flamer)
				if(CharacterSubTypesAvailable[NGSCT_Flamer]) return;
				break;
			
			case 6: //marine (sadar)
				if(CharacterSubTypesAvailable[NGSCT_Sadar]) return;
				break;
			
			case 7: //marine (grenade)
				if(CharacterSubTypesAvailable[NGSCT_GrenadeLauncher]) return;
				break;
			
			case 8: //marine (minigun)
				if(CharacterSubTypesAvailable[NGSCT_Minigun]) return;
				break;

			case 9: //marine (frisbee)
				if(CharacterSubTypesAvailable[NGSCT_Frisbee]) return;
				break;

			case 10: //marine (pistols)
				if(CharacterSubTypesAvailable[NGSCT_Pistols]) return;
				break;

		}

		if(search_forwards)
		{
			(*species)++;
			if(*species>10) *species=0;
		}
		else
		{
			(*species)--;
			if(*species<0) *species=10;
		}
		count++;
		
	}while(count<9);

	//oh dear no allowable species
	*species=0;
	
}

void SpeciesTag_DetermineMyNextCharacterType()
{
	NETGAME_CHARACTERTYPE tagSpecies;
	NETGAME_CHARACTERTYPE otherSpecies;
	if(netGameData.gameType==NGT_PredatorTag)
	{
		if(AvP.PlayerType!=I_Predator) return;
		tagSpecies=NGCT_Predator;
		otherSpecies=NGCT_Alien;
	}
	else
	{
		if(AvP.PlayerType!=I_Alien) return;
		tagSpecies=NGCT_Alien;
		otherSpecies=NGCT_Predator;
	}

	

	if(myNetworkKillerId && myNetworkKillerId!=AVPDPNetID && CountPlayersOfType(tagSpecies)==1)
	{
		//become the character that killed me
		int killer_index=PlayerIdInPlayerList(myNetworkKillerId);
		if(killer_index!=NET_IDNOTINPLAYERLIST)
		{
			netGameData.myNextCharacterType=netGameData.playerData[killer_index].characterType;
			netGameData.myCharacterSubType=netGameData.playerData[killer_index].characterSubType;
		}
		else
		{
			//the player doing the damage has either left the game , or never existed.
			//call it suicide then.
			myNetworkKillerId=0;	
		}
		
	}

	if(!(myNetworkKillerId && myNetworkKillerId!=AVPDPNetID && CountPlayersOfType(tagSpecies)==1))
	{
		int total=0;
		//either suicide , or we have too many predators for some reason
		//pick marine/alien at random
		
		//first determine available character types
		DetermineAvailableCharacterTypes(TRUE);
		
		total=CharacterTypesAvailable[NGCT_Marine]+
			  CharacterTypesAvailable[otherSpecies];
			  
		if(total==0)
		{
			//hmm , no available character types
			//in that case look at the character types that are allowed in the first place
			DetermineAvailableCharacterTypes(FALSE);
		
			total=CharacterTypesAvailable[NGCT_Marine]+
				  CharacterTypesAvailable[otherSpecies];
		}
		
		if(total>0)
		{
			int dieroll=(FastRandom() % total);

			if(dieroll<CharacterTypesAvailable[NGCT_Marine])
			{
				int i;
				//become a marine
				netGameData.myNextCharacterType=NGCT_Marine;

				//but what type of marine ?
				total=0;
				for(i=0;i<NUM_PC_SUBTYPES;i++)
				{
					total+=CharacterSubTypesAvailable[i];
				}
				
				//since there were available marine slots , there must be at least some
				//marine subtype slots
				GLOBALASSERT(total>0);

				dieroll=(FastRandom() % total);

				for(i=0;i<NUM_PC_SUBTYPES;i++)
				{
					dieroll-=CharacterSubTypesAvailable[i];
					if(dieroll<0)
					{
						netGameData.myCharacterSubType=i;
						return;
					}
				}

				GLOBALASSERT(0=="Shouldn't be possible to reach this line");
				
			}
			else
			{
				//become an alien
				netGameData.myNextCharacterType=otherSpecies;

			}
		}
		else
		{
			//no marines or aliens allowed?
			//tough, the player can become a standard marine
			netGameData.myNextCharacterType=NGCT_Marine;
			netGameData.myCharacterSubType=NGSCT_General;

		}
	}
}


/*----------------------------------------------------------------**
** Identify the network player closest to the centre of the screen **
**----------------------------------------------------------------*/
void ShowNearestPlayersName()
{
	extern int NumOnScreenBlocks;
	extern DISPLAYBLOCK *OnScreenBlockList[];
	extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
	int numberOfObjects = NumOnScreenBlocks;

	DPID nearestID=0;
	int nearestDist=0x7fffffff;
		
	//search through all the nearby objects for players
	while (numberOfObjects--)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[numberOfObjects];
		STRATEGYBLOCK* sbPtr = objectPtr->ObStrategyBlock;
				
		if (sbPtr && sbPtr->I_SBtype==I_BehaviourNetGhost)
		{
			NETGHOSTDATABLOCK *ghostData;
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;

			//we're only interested in player netghosts
			if(ghostData)
			{
				if(ghostData->type==I_BehaviourMarinePlayer ||
				   ghostData->type==I_BehaviourPredatorPlayer ||
				   ghostData->type==I_BehaviourAlienPlayer)
				{
					VECTORCH targetView;
					SmartTarget_GetCofM(objectPtr,&targetView);
					//get the players screen coordinates
					if(targetView.vz>0)
					{
						int screenX = WideMulNarrowDiv
								(				 			
									targetView.vx,
									Global_VDB_Ptr->VDB_ProjX,
									targetView.vz
								);

						int screenY = WideMulNarrowDiv
								(				 			
									targetView.vy,
									Global_VDB_Ptr->VDB_ProjY,
									targetView.vz
								);

						if(screenX<0) screenX=-screenX;
						if(screenY<0) screenY=-screenY;

						//make sure the player is in fact on screen
						if(screenX<ScreenDescriptorBlock.SDB_Width/2 && screenY<ScreenDescriptorBlock.SDB_Height/2)
						{
							int dist=screenX*screenX + screenY*screenY;
							//is this player closer to the centre of the screen than any before?
							if(dist<nearestDist)
							{
								//the player must be visible
								if (IsThisObjectVisibleFromThisPosition_WithIgnore(objectPtr,Player,&(Global_VDB_Ptr->VDB_World),1000000) ) 
								{
									{
										nearestDist=dist;
										nearestID=ghostData->playerId;
									}
								}
							}
						}
						
					}
				}
			}
		}
	}

	{
		int	nearestIndex=PlayerIdInPlayerList(nearestID);
		
		if(nearestIndex!=NET_IDNOTINPLAYERLIST)
		{
			//we've found the nearest on screen player, so show the name in the console
			NetworkGameConsoleMessage(TEXTSTRING_MULTIPLAYERCONSOLE_PLAYERSEEN,netGameData.playerData[nearestIndex].name,0);
		}
	}

}


static void CheckForPointBasedObjectRespawn()
{
	int score=0;
	int i;
	if(netGameData.pointsForRespawn==0) return;
	
	

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.gameType==NGT_Coop)
		{
			int j;
			for(j=0;j<3;j++)
			{
				score+=netGameData.playerData[i].aliensKilled[j]*netGameData.aiKillValues[j];	
			}
		}
		else
		{
			score+=netGameData.playerData[i].playerScore;	
		}
	}

	if(score>=(netGameData.lastPointsBasedRespawn + netGameData.pointsForRespawn))
	{
		//time for pickups to respawn
		AddNetMsg_RespawnPickups();
		
		netGameData.lastPointsBasedRespawn=score-(score%netGameData.pointsForRespawn);
		RespawnAllPickups();

		PrintStringTableEntryInConsole(TEXTSTRING_MULTIPLAYER_WEAPON_RESPAWN);
		
	}
	
}


static int CountMultiplayerLivesLeft()
{
	int i;
	int livesUsed=0;
	
	
	//count the lives used
	if(netGameData.useSharedLives)
	{
		//lives used is equal to number of deaths + number of currently living players
		if(netGameData.gameType==NGT_CoopDeathmatch)
		{
			//shared lives , are shared between team members
			livesUsed=netGameData.numDeaths[netGameData.myCharacterType];
		}
		else
		{
			livesUsed=netGameData.numDeaths[0]+netGameData.numDeaths[1]+netGameData.numDeaths[2];
		}

		

		for(i=0;i<NET_MAXPLAYERS;i++)	
		{
			if(netGameData.playerData[i].playerId)
			{
				//add an extra life if this player is currently alive
				if(netGameData.gameType==NGT_CoopDeathmatch)
				{
					if(netGameData.playerData[i].characterType!=netGameData.myCharacterType)
					{
						//don't count this player , since he isn't on the same team
						continue;
					}
				}
				livesUsed+=netGameData.playerData[i].playerAlive;
			}
		}
	}
	else
	{
		int index=PlayerIdInPlayerList(AVPDPNetID);
		if(index==NET_IDNOTINPLAYERLIST) return FALSE;		

		for(i=0;i<NET_MAXPLAYERS;i++)	
		{
			livesUsed+=netGameData.playerData[i].playerFrags[index];
		}
		livesUsed+=netGameData.playerData[index].deathsFromAI;
		livesUsed+=netGameData.playerData[index].playerAlive;
	}

	if(livesUsed>netGameData.maxLives)
	{
		return 0;
	}

	return (netGameData.maxLives-livesUsed);
}

BOOL AreThereAnyLivesLeft()
{

	if(netGameData.maxLives==0) return TRUE; //infinite lives
	
	if(netGameData.gameType!=NGT_Individual &&
	   netGameData.gameType!=NGT_Coop &&
	   netGameData.gameType!=NGT_CoopDeathmatch)
		return TRUE; //lives only affect some game types
	
	return (CountMultiplayerLivesLeft()>0);
}

#define FONT_ALIENSYMBOL 176
#define FONT_MARINESYMBOL 177
#define FONT_PREDATORSYMBOL 178
void DoMultiplayerEndGameScreen(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	int i,j;
	int x,y;
	char text[100];

	if(netGameData.myGameState==NGS_EndGameScreen)
	{
		D3D_FadeDownScreen(16384,0);
	}
	else
	{
		ShowMultiplayerScoreTimer-=RealFrameTime;
		if(ShowMultiplayerScoreTimer<=0)ShowMultiplayerScoreTimer=0;
	}
	
//   RenderStringCentred("Test Endgame Screen",ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height/2,0xffffffff);

		
	//draw headings
	y=150;
	x=120;
	
	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId)
		{
			RenderStringVertically(netGameData.playerData[i].name,x,y,0xffffffff);
			x+=20;
		}
	}
	x+=30;
	if(netGameData.gameType==NGT_Coop)
	{
		if(NPCHive.AliensCanBeGenerated)
		{
			RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_ALIENS),x,y,0xffffffff);
			x+=20;
		}
		if(NPCHive.PredAliensCanBeGenerated)
		{
			RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_PREDALIENS),x,y,0xffffffff);
			x+=20;
		}
		if(NPCHive.PraetoriansCanBeGenerated)
		{
			RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_PRAETORIANS),x,y,0xffffffff);
			x+=20;
		}
		x+=20;
		RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_SCORE),x,y,0xffffffff);
		x+=20;
	}
	else
	{
		RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_SCOREFOR),x,y,0xffffffff);
		x+=30;
		RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_SCOREAGAINST),x,y,0xffffffff);
		x+=30;
	}

	


	y+=10;

	for(i=0;i<NET_MAXPLAYERS;i++)
	{
		if(netGameData.playerData[i].playerId)
		{
			RenderStringCentred(netGameData.playerData[i].name,50,y,0xffffffff);
			
			//draw the player's species symbol
			{
				char symbol[2]={0,0};
				switch(netGameData.playerData[i].characterType)
				{
					case NGCT_Marine :
						symbol[0]=FONT_MARINESYMBOL;
						RenderStringCentred(symbol,100,y,0xffffffff);
						break;

					case NGCT_Alien :
						symbol[0]=FONT_ALIENSYMBOL;
						RenderStringCentred(symbol,100,y,0xffffffff);
						break;

					case NGCT_Predator :
						symbol[0]=FONT_PREDATORSYMBOL;
						RenderStringCentred(symbol,100,y,0xffffffff);
						break;
						
					default:
						break;
				}
			}
			
			
			x=120;
			for(j=0;j<NET_MAXPLAYERS;j++)
			{
				if(netGameData.playerData[j].playerId)
				{
					sprintf(text,"%d",netGameData.playerData[i].playerFrags[j]);
					if(i==j)
						RenderStringCentred(text,x,y,0xffff0000);
					else
						RenderStringCentred(text,x,y,0xff00ff00);
							
					x+=20;
				}
			}
			
			
			x+=30;
			if(netGameData.gameType==NGT_Coop)
			{
				int score=0;
				
				if(NPCHive.AliensCanBeGenerated)
				{
					sprintf(text,"%d",netGameData.playerData[i].aliensKilled[0]);
					RenderStringCentred(text,x,y,0xff00ff00);
					x+=20;
				}
				if(NPCHive.PredAliensCanBeGenerated)
				{
					sprintf(text,"%d",netGameData.playerData[i].aliensKilled[1]);
					RenderStringCentred(text,x,y,0xff00ff00);
					x+=20;
				}
				if(NPCHive.PraetoriansCanBeGenerated)
				{
					sprintf(text,"%d",netGameData.playerData[i].aliensKilled[2]);
					RenderStringCentred(text,x,y,0xff00ff00);
					x+=20;
				}
				
				for(j=0;j<3;j++)
				{

					score+=netGameData.playerData[i].aliensKilled[j]*netGameData.aiKillValues[j];
				}
				x+=20;
				sprintf(text,"%d",score);
				RenderStringCentred(text,x,y,0xff00ff00);

			}
			else
			{
				sprintf(text,"%d",netGameData.playerData[i].playerScore);
				RenderStringCentred(text,x,y,0xff00ff00);
				x+=30;

				sprintf(text,"%d",netGameData.playerData[i].playerScoreAgainst);
				RenderStringCentred(text,x,y,0xffff0000);
				x+=30;
			}
			y+=20;
		}
		
	}

	if(netGameData.gameType==NGT_Coop)
	{
		//show kills by ai aliens
		RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_ALIENS),50,y,0xffffffff);

				
		x=120;
		for(j=0;j<NET_MAXPLAYERS;j++)
		{
			if(netGameData.playerData[j].playerId)
			{
				sprintf(text,"%d",netGameData.playerData[j].deathsFromAI);
				RenderStringCentred(text,x,y,0xff00ff00);
				x+=20;
			}
		}
		
		y+=20;
	}

	if(netGameData.maxLives!=0)
	{
		if(netGameData.gameType==NGT_Individual ||
		   netGameData.gameType==NGT_Coop ||
		   netGameData.gameType==NGT_CoopDeathmatch)
		{
			y+=20;

			if(y<240)
			{
				//make sure the text appears far enough down the screen , so we don't
				//overlap with the species score stuff
				y=240;
			}
			//display remaining lives
			sprintf(text,"%s: %d",GetTextString(TEXTSTRING_MULTIPLAYER_LIVES_LEFT),CountMultiplayerLivesLeft());
			RenderStringCentred(text,ScreenDescriptorBlock.SDB_Width/2,y,0xffffffff);
	
		}
	}

	if(netGameData.gameType==NGT_CoopDeathmatch)
	{
		//show species scores
		x+=50;
		//titles
		RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_ALIEN),x,160,0xffffffff);
		RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_MARINE),x,180,0xffffffff);
		RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_PREDATOR),x,200,0xffffffff);

		//scores
		x+=50;
		
		RenderStringVertically(GetTextString(TEXTSTRING_MULTIPLAYER_SPECIESSCORE),x,150,0xffffffff);
		
		sprintf(text,"%d",netGameData.teamScores[NGCT_Alien]);
		RenderStringCentred(text,x,160,0xffffffff);
		sprintf(text,"%d",netGameData.teamScores[NGCT_Marine]);
		RenderStringCentred(text,x,180,0xffffffff);
		sprintf(text,"%d",netGameData.teamScores[NGCT_Predator]);
		RenderStringCentred(text,x,200,0xffffffff);

	}

	
	if(netGameData.myGameState==NGS_EndGameScreen)
	{
		if(AvP.Network==I_Host)
		{
			
			//pause before the host can restart
			if(netGameData.stateCheckTimeDelay>0)
			{
				netGameData.stateCheckTimeDelay-=RealFrameTime;
				return;
			}
			netGameData.stateCheckTimeDelay=0;
			
  			RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_PRESSKEYTORESTARTGAME),ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height-20,0xffffffff);
			if (DebouncedGotAnyKey)
			{
				int seed=FastRandom();
				RestartNetworkGame(seed);
				AddNetMsg_RestartNetworkGame(seed);
			}
		}
		else
		{
 			RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_WAITFORRESTARTGAME),ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height-20,0xffffffff);
		}
	}
	else if(!playerStatusPtr->IsAlive)
	{
		if(AreThereAnyLivesLeft())
		{
 			RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_OPERATETORESPAWN),ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height-20,0xffffffff);
		}
		else
		{
 			RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_OPERATETOOBSERVE),ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height-20,0xffffffff);
		}
	}
}

static BOOL IsItPossibleToScore()
{
	
	if(CalculateMyScore()) return TRUE;
	
	if(netGameData.gameType==NGT_LastManStanding) return TRUE;
	if(netGameData.gameType==NGT_Coop)
	{
		if(netGameData.aiKillValues[0]) return TRUE;
		if(netGameData.aiKillValues[1]) return TRUE;
		if(netGameData.aiKillValues[2]) return TRUE;
		return FALSE;
	}
	else
	{
		if(netGameData.baseKillValue == 0) return FALSE;
		if(netGameData.useCharacterKillValues && !netGameData.useDynamicScoring)
		{
			if(netGameData.characterKillValues[0]) return TRUE;
			if(netGameData.characterKillValues[1]) return TRUE;
			if(netGameData.characterKillValues[2]) return TRUE;
			return FALSE;
		}
	}
	return TRUE;
		
}

void DoMultiplayerSpecificHud()
{

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	char text[200];

	//Show score table if either 
	//1. Player has asked to
	//2. Game is over
	//3. Player is dead and not observing anyone
	if(ShowMultiplayerScoreTimer>0 || 
		netGameData.myGameState==NGS_EndGameScreen ||
		(!playerStatusPtr->IsAlive && !MultiplayerObservedPlayer))
	{
		DoMultiplayerEndGameScreen();
	}
	else 
	{
		
		if(MultiplayerObservedPlayer)
		{
			//show the name of the observed player
			int index=PlayerIdInPlayerList(MultiplayerObservedPlayer);
			if(index!=NET_IDNOTINPLAYERLIST)
			{
		    	RenderStringCentred(netGameData.playerData[index].name,ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height/2,0xff0000ff);
			}
		}
	
		
		/*
		{		
			int i;
			int myIndex = PlayerIdInPlayerList(AVPDPNetID);
			LOCALASSERT(myIndex!=NET_IDNOTINPLAYERLIST);		
			for(i=0;i<NET_MAXPLAYERS;i++)
			{
				if(netGameData.playerData[i].playerId)
				{
					if(netGameData.gameType==NGT_LastManStanding)
					{
						switch(netGameData.playerData[i].characterType)
						{
							case NGCT_Marine :
								PrintDebuggingText("%s : %d   (Marine)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore);
								break;
							case NGCT_Alien :
								PrintDebuggingText("%s : %d   (Alien)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore);
								break;
							case NGCT_Predator :
								PrintDebuggingText("%s : %d   (Predator)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore);
								break;
						}
					}
					else if(netGameData.gameType==NGT_Coop)
					{
						//show kills / deaths
						int totalKills=netGameData.playerData[i].aliensKilled[0]+netGameData.playerData[i].aliensKilled[1]+netGameData.playerData[i].aliensKilled[2];
						PrintDebuggingText("%s : %d/%d\n",netGameData.playerData[i].name,totalKills,netGameData.playerData[i].playerFrags[i]);
					}
					else
					{
						switch(netGameData.playerData[i].characterType)
						{
							case NGCT_Marine :
								PrintDebuggingText("%s : %d   (%d)(Marine)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore,GetNetScoreForKill(i,myIndex));
								break;
							case NGCT_Alien :
								PrintDebuggingText("%s : %d   (%d)(Alien)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore,GetNetScoreForKill(i,myIndex));
								break;
							case NGCT_Predator :
								PrintDebuggingText("%s : %d   (%d)(Predator)\n",netGameData.playerData[i].name,netGameData.playerData[i].playerScore,GetNetScoreForKill(i,myIndex));
								break;
						}
					}
				}
			}
		}
		*/
	}
	
	//show the player's score
	{
		int score = CalculateMyScore();
		if(score || IsItPossibleToScore())
		{
			sprintf(text,"%s : %d",GetTextString(TEXTSTRING_MULTIPLAYER_SCORE),score);
			RenderString(text,5,0,0xffffffff);
		}
	}

	//show time left
	if(netGameData.timeLimit>0)
	{
		int hoursLeft;
		int minutesLeft;
		int secondsLeft=(netGameData.timeLimit*60)-(netGameData.GameTimeElapsed>>16);
		if(secondsLeft<0) secondsLeft=0;

		hoursLeft=secondsLeft/3600;
		minutesLeft=(secondsLeft/60)%60;
		secondsLeft%=60;

		//display time left as hh:mm:ss
		sprintf(text,"%s : %02d:%02d:%02d",GetTextString(TEXTSTRING_MULTIPLAYER_TIME),hoursLeft,minutesLeft,secondsLeft);

		RenderString(text,5,20,0xffffffff);
		
	}
	
}


void GetNextMultiplayerObservedPlayer()
{
	extern int GlobalFrameCounter;
	static int LastFrameTried;

	//Use the frame counter to debounce changing player to observe
	if(LastFrameTried==GlobalFrameCounter || LastFrameTried+1==GlobalFrameCounter)
	{
		//obviously tried last frame , so don't bother trying to find a player to watch
		LastFrameTried=GlobalFrameCounter;
		return;	
	}
	else
	{
		int cur_index=PlayerIdInPlayerList(MultiplayerObservedPlayer);
		int next_index;
		int count=0;

		LastFrameTried=GlobalFrameCounter;
		
		for(next_index=cur_index+1;count<NET_MAXPLAYERS;next_index++,count++)
		{
			if(next_index==NET_MAXPLAYERS) next_index=0;
			if(netGameData.playerData[next_index].playerId)
			{
				if(netGameData.playerData[next_index].playerAlive)
				{
					DYNAMICSBLOCK* dynPtr=Player->ObStrategyBlock->DynPtr;
					MultiplayerObservedPlayer=netGameData.playerData[next_index].playerId;
					//need to disable gravity , and make sure the playher isn't moving
					//of his own accord
					dynPtr->GravityOn=0;

					dynPtr->LinImpulse.vx=0;
					dynPtr->LinImpulse.vy=0;
					dynPtr->LinImpulse.vz=0;

					dynPtr->LinVelocity.vx=0;
					dynPtr->LinVelocity.vy=0;
					dynPtr->LinVelocity.vz=0;
					
					
					return;
				}
			}
		}

		//oh well , no one available to observe
		TurnOffMultiplayerObserveMode();
	}
}

void TurnOffMultiplayerObserveMode()
{
	if(MultiplayerObservedPlayer)
	{
		DYNAMICSBLOCK* dynPtr=Player->ObStrategyBlock->DynPtr;
	
		MultiplayerObservedPlayer=0;

		//need to turn gravity and collisions back on
		dynPtr->GravityOn=1;
	}
}

void CheckStateOfObservedPlayer()
{
	int index=PlayerIdInPlayerList(MultiplayerObservedPlayer);
	if(!MultiplayerObservedPlayer) return;

	//our observed player must exist
	if(index==NET_IDNOTINPLAYERLIST)
	{
		TurnOffMultiplayerObserveMode();
		return;
	}
	
	//he must also be alive
	if(!netGameData.playerData[index].playerAlive)
	{
		TurnOffMultiplayerObserveMode();
		return;
	}
}



static int CalculateMyScore()
{
	int myIndex=PlayerIdInPlayerList(AVPDPNetID);
	if(myIndex==NET_IDNOTINPLAYERLIST) return 0;

	if(netGameData.gameType==NGT_Coop)
	{
		int score=0;
		int i;
		for(i=0;i<3;i++)
		{
			score+=netGameData.playerData[myIndex].aliensKilled[i]*netGameData.aiKillValues[i];
		}
		return score;
		
	}
	else
	{
		return netGameData.playerData[myIndex].playerScore;	
	}
}




static void PeriodicScoreUpdate()
{
	static unsigned int timer=0;
	static int playerIndex=0;
	int count=0;

	//cycle through the players , sending scores every 2 seconds
	timer+=RealFrameTime;
	if(timer<2*ONE_FIXED) return;
	timer=0;
	
	if(netGameData.myGameState!=NGS_Playing) return;
	for(playerIndex++;count<NET_MAXPLAYERS;count++,playerIndex++)
	{
		if(playerIndex>=NET_MAXPLAYERS && netGameData.gameType==NGT_CoopDeathmatch)
		{
			//for species deathmatch games also update team scores occasionly
			AddNetMsg_SpeciesScores();
		}
		
		playerIndex%=NET_MAXPLAYERS;
		if(netGameData.playerData[playerIndex].playerId)
		{
			AddNetMsg_PlayerScores(playerIndex);
			return;
		}
	}
}



static int GetStrategySynchObjectChecksum()
{
	/*
	Generate a number from the sbnames of all the strategies that need to be synched
	*/
	int sum=0;
	int position=0;
	int i;
	for (i=0; i<NumActiveStBlocks; i++)
	{
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[i];

		if(sbPtr->I_SBtype == I_BehaviourBinarySwitch ||
		   sbPtr->I_SBtype == I_BehaviourLinkSwitch ||
		   sbPtr->I_SBtype == I_BehaviourTrackObject)
		{
			position++;
			sum+=(*(int*)&sbPtr->SBname[0])*position;
		}
	}
	if(sum==0) sum=1;
	return sum;
}


