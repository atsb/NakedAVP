/*---------------------------Patrick 28/3/97----------------------------
  Header for Multi-Player ghost object support header
  ----------------------------------------------------------------------*/
#ifndef pldghost_h_included
#define pldghost_h_included
#ifdef __cplusplus
extern "C" {
#endif

#include "pvisible.h"
#include "pldnet.h"

/*---------------------------Patrick 28/3/97----------------------------
  Structures for ghosts and ghost data blocks
  ----------------------------------------------------------------------*/

typedef struct netghostdatablock
{
//	DPID playerId;
	int playerId;
	
	signed int playerObjectId; /* -1 == player, all other numbers used for objects */
	AVP_BEHAVIOUR_TYPE type;
	INANIMATEOBJECT_TYPE IOType;
	int subtype;
	SHAPEANIMATIONCONTROLLER ShpAnimCtrl;

	/* KJL 17:33:41 22/01/99 - I've made this a union because I needed a storage space,
	and the currentAnimSequence is only used by specific objects */
	//union
	//{
		int currentAnimSequence;
	//	int EventCounter; // used by grenades
	//};
	
	DISPLAYBLOCK *myGunFlash;
	SECTION_DATA *GunflashSectionPtr;
	int GunFlashFrameStamp;
	int CurrentWeapon;

	int SoundHandle;
	int SoundHandle2;
	int SoundHandle3;
	int SoundHandle4;
	int integrity;
	int timer;

	int FlameHitCount;//number of fire particles that have hit since last frame
	int FlechetteHitCount;//number of flechette particles that have hit since last frame

	HMODELCONTROLLER HModelController;
	HITLOCATIONTABLE *hltable;
	
	int CloakingEffectiveness;

	unsigned int IgnitionHandshaking :1;

	unsigned int invulnerable :1; //for netghost's of players
	unsigned int onlyValidFar:1; //set for alien ai that are far from everyone
	unsigned int soundStartFlag:1;

	#if EXTRAPOLATION_TEST
	VECTORCH velocity;
	int extrapTimerLast;
	int extrapTimer;
	unsigned int lastTimeRead;
	#endif
}NETGHOSTDATABLOCK;

/*---------------------------Patrick 28/3/97----------------------------
  Protoypes
  ----------------------------------------------------------------------*/
extern void UpdateGhost(STRATEGYBLOCK *sbPtr,VECTORCH *position,EULER *orientation,int sequence, int special);
extern void RemoveGhost(STRATEGYBLOCK *sbPtr);
//extern void RemovePlayersGhosts(DPID id);
//extern void RemovePlayerGhost(DPID id);
//extern STRATEGYBLOCK *FindGhost(DPID Id, int obId);
//extern STRATEGYBLOCK *CreateNetGhost(DPID playerId, int objectId, VECTORCH *position, EULER* orientation, AVP_BEHAVIOUR_TYPE type, unsigned char IOType, unsigned char subtype);
extern void RemovePlayersGhosts(int id);
extern void RemovePlayerGhost(int id);
extern STRATEGYBLOCK *FindGhost(int Id, int obId);
extern STRATEGYBLOCK *CreateNetGhost(int playerId, int objectId, VECTORCH *position, EULER* orientation, AVP_BEHAVIOUR_TYPE type, unsigned char IOType, unsigned char subtype);
extern void MakeGhostNear(STRATEGYBLOCK *sbPtr);
extern void MakeGhostFar(STRATEGYBLOCK *sbPtr);
extern void DamageNetworkGhost(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, SECTION_DATA *section,VECTORCH* incoming);
extern void HandleGhostGunFlashEffect(STRATEGYBLOCK *sbPtr, int gunFlashOn);
extern void HandlePlayerGhostWeaponSound(STRATEGYBLOCK *sbPtr, int weapon, int firingPrimary, int firingSecondary);
extern void HandleWeaponElevation(STRATEGYBLOCK *sbPtr, int elevation, int weapon);
extern void MaintainGhosts(void);
extern void HandleGhostAutoGunMuzzleFlash(STRATEGYBLOCK *sbPtr, int firing);
extern void HandleGhostAutoGunSound(STRATEGYBLOCK *sbPtr, int firing);
extern void MaintainGhostCloakingStatus(STRATEGYBLOCK *sbPtr, int IsCloaked);
extern void MaintainGhostFireStatus(STRATEGYBLOCK *sbPtr, int IsOnFire);

extern void NetGhostBehaviour(STRATEGYBLOCK *sbPtr);
extern void KillGhost(STRATEGYBLOCK *sbPtr, int objectId);
extern int Deduce_PlayerDeathSequence(void);
extern STRATEGYBLOCK *MakeNewCorpse();
extern void ApplyGhostCorpseDeathAnim(STRATEGYBLOCK *sbPtr,int deathId);
extern void ApplyCorpseDeathAnim(STRATEGYBLOCK *sbPtr,int deathId);

extern int Deduce_PlayerMarineDeathSequence(STRATEGYBLOCK* sbPtr,DAMAGE_PROFILE* damage,int multiple,VECTORCH* incoming);
extern int Deduce_PlayerAlienDeathSequence(STRATEGYBLOCK* sbPtr,DAMAGE_PROFILE* damage,int multiple,VECTORCH* incoming);
extern int Deduce_PlayerPredatorDeathSequence(STRATEGYBLOCK* sbPtr,DAMAGE_PROFILE* damage,int multiple,VECTORCH* incoming);

extern void UpdateAlienAIGhost(STRATEGYBLOCK *sbPtr,VECTORCH *position,EULER *orientation,int sequence_type,int sub_sequence, int sequence_length);
extern void KillAlienAIGhost(STRATEGYBLOCK *sbPtr,int death_code,int death_time,int GibbFactor);

extern void Convert_DiscGhost_To_PickupGhost(STRATEGYBLOCK *sbPtr);
extern void PlayHitDeltaOnGhost(STRATEGYBLOCK *sbPtr,char delta_seq,char delta_sub_seq);
extern void PlayOtherSound(enum soundindex SoundIndex, VECTORCH *position, int explosion);
void CreateMarineHModel(NETGHOSTDATABLOCK *ghostDataPtr, int weapon);
void CreateAlienHModel(NETGHOSTDATABLOCK *ghostDataPtr,int alienType);
void CreatePredatorHModel(NETGHOSTDATABLOCK *ghostDataPtr, int weapon);

/*---------------------------Patrick 29/3/97----------------------------
  Defines
  ----------------------------------------------------------------------*/
#define GHOST_PLAYEROBJECTID	-1
#define PLAYERGHOST_NUMBEROFFRAGMENTS			10
#define GHOST_INTEGRITY							(ONE_FIXED*8)

#define MPPRED_MUZZLEFLASHOFFSET_INFRONT		800
#define MPPRED_MUZZLEFLASHOFFSET_ACROSS			200
#define MPPRED_MUZZLEFLASHOFFSET_UP				100
#define MPPRED_MUZZLEFLASHOFFSET_INFRONT_CROUCH	1000
#define MPPRED_MUZZLEFLASHOFFSET_ACROSS_CROUCH	200
#define MPPRED_MUZZLEFLASHOFFSET_UP_CROUCH		0

#define MPMARINE_MUZZLEFLASHOFFSET_INFRONT			1100
#define MPMARINE_MUZZLEFLASHOFFSET_ACROSS			200
#define MPMARINE_MUZZLEFLASHOFFSET_UP				0
#define MPMARINE_MUZZLEFLASHOFFSET_INFRONT_CROUCHED	400
#define MPMARINE_MUZZLEFLASHOFFSET_ACROSS_CROUCHED	300
#define MPMARINE_MUZZLEFLASHOFFSET_UP_CROUCHED		(-400)
#define MPMARINE_MUZZLEFLASHOFFSET_INFRONT_RUNNING	1100
#define MPMARINE_MUZZLEFLASHOFFSET_ACROSS_RUNNING	50
#define MPMARINE_MUZZLEFLASHOFFSET_UP_RUNNING		400


/*---------------------------Patrick 28/3/97----------------------------
  Globals
  ----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif

