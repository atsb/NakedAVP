/*KJL******************************************************************
* weapon.c - this is home to the weapon state machine and related fns *
******************************************************************KJL*/
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"
#include "inventry.h"
#include "comp_shp.h"
#include "load_shp.h"
#include "huddefs.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "dynblock.h"
#include "dynamics.h"
#include "lighting.h"
#include "pvisible.h"
#include "bh_alien.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_fhug.h"
#include "bh_marin.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "bh_agun.h"
#include "bh_light.h"
#include "bh_corpse.h"
#include "bh_ais.h"
#include "bh_videoscreen.h"
#include "bh_track.h"
#include "weapons.h"
#include "avpview.h"

#include "psnd.h"
#include "vision.h"
#include "plat_shp.h"

#include "particle.h"
#include "psndproj.h"
#include "psndplat.h"
#include "showcmds.h"
#include "projload.hpp"

/* for win 95 net support */
#include "pldghost.h"
#include "pldnet.h"

#include "los.h"
#include "kshape.h"
#include "targeting.h"
#include "extents.h"
#include "scream.h"
#include "avp_userprofile.h"

#define BITE_HEALTH_RECOVERY	(50)
#define BITE_ARMOUR_RECOVERY	(30)
#define PULSERIFLE_MINIMUM_BURST	(4)
#define SMARTGUN_MINIMUM_BURST	(8)

#define MEDICOMP_USE_THRESHOLD	(10*ONE_FIXED)
#define MEDICOMP_DRAIN_BLOCK	((10*ONE_FIXED)-1)
#define EXTINGUISHER_USE_THRESHOLD	(3*ONE_FIXED)
#define EXTINGUISHER_DRAIN_BLOCK	(3*ONE_FIXED)
#define GREENFLASH_INTENSITY	(5*ONE_FIXED)

#define CASTER_CHARGERATIO (2)
/* Was 3... */

#define PRED_PISTOL_SECONDARY_FIRE_CHARGE	(ONE_FIXED<<1)

#define QUIRKAFLEEG	0
/* That turns on medicomp ammo.  Private joke.  :-) */

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/

#if 0
static char tempstring[256];
#endif

static int WBStrikeTime=(ONE_FIXED>>1);
static int ACStrikeTime=(ONE_FIXED/6);
int AutoSwap=-1;

int PredPistol_ShotCost=120000;  /* Changed from 60000 (Fox value), 1/3/99 */
int Caster_Jumpstart=10000;
int Caster_Chargetime=150000;
int Caster_ChargeRatio=400000;
int Caster_TrickleRate=0;
int Caster_TrickleLevel=16384;
int Caster_MinCharge=0;

int Caster_NPCKill=16384;
int Caster_PCKill=49151;

static int AC_Speed_Factor=ONE_FIXED;
int Alien_Visible_Weapon;
static int Alien_Tail_Clock;
static int Wristblade_StrikeType;
STRATEGYBLOCK *Alien_Tail_Target;
char Alien_Tail_Target_SBname[SB_NAME_LENGTH];

static DAMAGE_PROFILE Player_Weapon_Damage;

extern SOUND3DDATA Explosion_SoundData;

extern DISPLAYBLOCK* Player;
extern int NormalFrameTime;
extern unsigned char Null_Name[8];
extern int ShowPredoStats;
extern int playerNoise;
extern int PlayerDamagedOverlayIntensity;

extern int NumOnScreenBlocks;
extern int NumActiveBlocks;
extern DISPLAYBLOCK *OnScreenBlockList[];
extern DISPLAYBLOCK *ActiveBlockList[];
extern DISPLAYBLOCK *SmartTarget_Object;
extern int HtoHStrikes;
extern int weaponHandle;
extern int predHUDSoundHandle;
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern NETGAME_GAMEDATA netGameData;

int WeaponFidgetPlaying;
int Old_Minigun_SpinSpeed;
int Minigun_SpinSpeed;
int Weapon_ThisBurst;
EULER Minigun_MaxHeadJolt;
EULER Minigun_HeadJolt;
int Flamethrower_Timer;

SECTION_DATA *PlayerStaff1=NULL;
SECTION_DATA *PlayerStaff2=NULL;
SECTION_DATA *PlayerStaff3=NULL;
int StaffAttack=-1;
STRATEGYBLOCK *Biting;
char Biting_SBname[SB_NAME_LENGTH];
int Bit=0;

extern int Validate_Target(STRATEGYBLOCK *target,char *SBname);

extern MODULEMAPBLOCK * MakeDefaultModuleMapblock();
extern void InitialiseEnergyBoltBehaviour(DAMAGE_PROFILE *damage,int factor);
extern void PredDisc_GetFirstTarget(PC_PRED_DISC_BEHAV_BLOCK *bptr, DISPLAYBLOCK *target, VECTORCH *position);
extern void InitialiseDiscBehaviour(STRATEGYBLOCK *target,SECTION_DATA *disc_section);
extern int Validate_Target(STRATEGYBLOCK *target,char *SBname);
extern int playerNoise;
extern int SlotForThisWeapon(enum WEAPON_ID weaponID);
extern void MakeBloodExplosion(VECTORCH *originPtr, int creationRadius, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID);
extern HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeSetFromLibrary(const char* rif_name,const char* shape_set_name);
extern void Crunch_Position_For_Players_Weapon(VECTORCH *position);
extern DISPLAYBLOCK *MakePistolCasing(VECTORCH *position,MATRIXCH *orient);

int FriendlyFireDamageFilter(DAMAGE_PROFILE *damage);
static void MarineZeroAmmoFunctionality(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr);
static void PredatorZeroAmmoFunctionality(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr);

/* Line Of Sight information used by FireLineOfSightWeapon() */
VECTORCH 		LOS_Point;	 		/* point in world space which player has hit */
int 			LOS_Lambda;			/* distance in mm to point from player */
DISPLAYBLOCK*	LOS_ObjectHitPtr;	/* pointer to object that was hit */
VECTORCH		LOS_ObjectNormal;	/* normal of the object's face which was hit */
SECTION_DATA*	LOS_HModel_Section;	/* Section of HModel hit */


/* unnormalised vector in the direction	which the gun's muzzle is pointing, IN VIEW SPACE */
/* very useful when considering sprites, which lie in a z-plane in view space */
VECTORCH GunMuzzleDirectionInVS;

VECTORCH PlayerGunBarrelOffset;

/* dir gun is pointing, normalised and in world space */
VECTORCH GunMuzzleDirectionInWS;

DISPLAYBLOCK PlayersWeapon;
DISPLAYBLOCK PlayersWeaponMuzzleFlash;
HMODELCONTROLLER PlayersWeaponHModelController;
SECTION_DATA *PWMFSDP; /* PlayersWeaponMuzzleFlashSectionDataPointer */
VECTORCH PlayersWeaponCameraOffset;

struct Target PlayersTarget;

int GrenadeLauncherSelectedAmmo;
int LastHand;  // For alien claws and two pistols

int AllowGoldWeapons = 0; // flag to indicate the Gold version weapons should be allowed

char *GrenadeLauncherBulletNames[6] = {
	"bulletF",	//05_
	"bulletA",	//_
	"bulletB",	//01_
	"bulletC",	//02_
	"bulletD",	//03_
	"bulletE",	//04_
};

enum WEAPON_ID MarineWeaponHierarchy[] = {
	
	WEAPON_PULSERIFLE,
	WEAPON_SMARTGUN,
	WEAPON_TWO_PISTOLS,
	WEAPON_MARINE_PISTOL,
	WEAPON_MINIGUN,
	WEAPON_FLAMETHROWER,
	WEAPON_GRENADELAUNCHER,
	WEAPON_SADAR,
	WEAPON_FRISBEE_LAUNCHER,
	WEAPON_CUDGEL,
	NULL_WEAPON

};

enum WEAPON_ID PredatorWeaponHierarchy[] = {
	
	WEAPON_PRED_RIFLE,
	WEAPON_PRED_WRISTBLADE,
	NULL_WEAPON

};

extern void CastLOSSpear(STRATEGYBLOCK *sbPtr, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int inaccurate);

VECTORCH SpreadfireSpears[] = {
	{0,0,	600,},
	{100,0,	600,},
	{-100,0,600,},
	{50,0,	600,},
	{-50,0,	600,},
	{25,50,	600,},
	{-25,50,600,},
	{75,50,	600,},
	{-75,50,600,},
	{25,-50,600,},
	{-25,-50,600,},
	{75,-50,600,},
	{-75,-50,600,},
	{-1,-1,-1,},
};

SECTION_DATA *GrenadeLauncherSectionPointers[6];
                      
/* Used to calculate the damage of one projectile due to FRI
   Only used by the flamethrower at the moment */
int ProjectilesFired; 

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void DoShapeAnimation (DISPLAYBLOCK * dptr);
void FindHitArea(DISPLAYBLOCK *dptr);
DISPLAYBLOCK *HtoHDamageToHModel(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, STRATEGYBLOCK *source, VECTORCH *attack_dir);
DISPLAYBLOCK *AlienTail_TargetSelect(void);
STRATEGYBLOCK *GetBitingTarget(void);
SECTION_DATA *CheckBiteIntegrity(void);
/* Yes, whatever, but this was LESS TEDIOUS! */
STRATEGYBLOCK *GetTrophyTarget(SECTION_DATA **head_section_data);

extern void PlayerIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiplier,VECTORCH* incoming);

void UpdateWeaponStateMachine(void);
void HandleSpearImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data);
static void WeaponStateIdle(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr,TEMPLATE_WEAPON_DATA *twPtr, int justfiredp,int justfireds, int ps);
static int RequestChangeOfWeapon(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr);
void ChangeHUDToAlternateShapeSet(char *riffname,char *setname);



static void StateDependentMovement(PLAYER_STATUS *playerStatusPtr, PLAYER_WEAPON_DATA *weaponPtr);

int FireAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr);
int FireNonAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr);
int FireNonAutomaticSecondaryAmmo(PLAYER_WEAPON_DATA *weaponPtr);
int GrenadeLauncherChangeAmmo(PLAYER_WEAPON_DATA *weaponPtr);
int PredDiscChangeMode(PLAYER_WEAPON_DATA *weaponPtr);
int SmartgunSecondaryFire(PLAYER_WEAPON_DATA *weaponPtr);
int DamageObjectInLineOfSight(PLAYER_WEAPON_DATA *weaponPtr);
int MeleeWeapon_180Degree_Front(PLAYER_WEAPON_DATA *weaponPtr);
int MeleeWeapon_90Degree_Front(PLAYER_WEAPON_DATA *weaponPtr);
int FireEmptyMinigun(PLAYER_WEAPON_DATA *weaponPtr);

int Staff_Manager(DAMAGE_PROFILE *damage,SECTION_DATA *section1,SECTION_DATA *section2,SECTION_DATA *section3,
	STRATEGYBLOCK *wielder);

static void PlayerFireLineOfSightAmmo(enum AMMO_ID AmmoID, int multiple);
extern void FireProjectileAmmo(enum AMMO_ID AmmoID);

void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *section_pointer); 


void FireAutoGun(STRATEGYBLOCK *sbPtr);
void FindEndOfShape(VECTORCH* endPositionPtr, int shapeIndex);
static void CalculateTorque(EULER *rotationPtr, VECTORCH *directionPtr, STRATEGYBLOCK *sbPtr);
static void CalculateTorqueAtPoint(EULER *rotationPtr, VECTORCH *pointPtr, STRATEGYBLOCK *sbPtr);

void PositionPlayersWeaponMuzzleFlash(void);

void MeleeWeaponNullTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

void AlienClawTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
void AlienClawEndTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
void AlienTailTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

void PredWristbladeTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
void PredDiscThrowTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

void ParticleBeamSwapping(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
void ParticleBeamReadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
void ParticleBeamUnreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

void InitThisWeapon(PLAYER_WEAPON_DATA *pwPtr);

static int RequestChangeOfWeaponWhilstSwapping(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr);

static void DamageDamageBlock(DAMAGEBLOCK *DBPtr, DAMAGE_PROFILE *damage, int multiple);
DISPLAYBLOCK *CauseDamageToHModel(HMODELCONTROLLER *HMC_Ptr, STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, SECTION_DATA *this_section_data,VECTORCH *incoming, VECTORCH *position, int FromHost);
#if 0
void WeaponCreateStartFrame(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
#endif
int PC_Alien_Eat_Attack(int hits);
int FirePredatorDisc(PLAYER_WEAPON_DATA *weaponPtr,SECTION_DATA *disc_section);

void BiteAttack_AwardHealth(STRATEGYBLOCK *sbPtr,AVP_BEHAVIOUR_TYPE pre_bite_type);
void LimbRip_AwardHealth(void);
void GrenadeLauncher_EmergencyChangeAmmo(PLAYER_WEAPON_DATA *weaponPtr);
int AccuracyStats_TargetFilter(STRATEGYBLOCK *sbPtr);

#define Random16BitNumber (FastRandom()&65535)
/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/

int Predator_WantToChangeWeapon(PLAYER_STATUS *playerStatusPtr, PLAYER_WEAPON_DATA *weaponPtr) {

	if ((
		(weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_WRISTBLADE)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_STAFF)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_MEDICOMP)
		)||( 
		(AvP.PlayerType==I_Predator)&&(weaponPtr->WeaponIDNumber==WEAPON_PRED_SHOULDERCANNON)
		&&(playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart)
		&&(playerStatusPtr->FieldCharge<MUL_FIXED((Caster_Jumpstart-playerStatusPtr->PlasmaCasterCharge),Caster_ChargeRatio))
		)||( 
		(AvP.PlayerType==I_Predator)&&(weaponPtr->WeaponIDNumber==WEAPON_PRED_PISTOL)
		&&(playerStatusPtr->FieldCharge<PredPistol_ShotCost)
		)) {
		
		return(1);

	} else {

		return(0);
	
	}
}

int Predator_WeaponHasAmmo(PLAYER_STATUS *playerStatusPtr, int slot) {

	if (slot!=-1) {
		PLAYER_WEAPON_DATA *this_weaponPtr;

	    this_weaponPtr = &(PlayerStatusPtr->WeaponSlot[slot]);
	
		switch (this_weaponPtr->WeaponIDNumber) {
			default:
				{
					return(0);
				}
				break;
			case WEAPON_PRED_WRISTBLADE:
			case WEAPON_PRED_STAFF:
				{
					return(1);
				}
				break;
			case WEAPON_PRED_MEDICOMP:
				{
					if ((playerStatusPtr->FieldCharge<EXTINGUISHER_USE_THRESHOLD)
						&&(playerStatusPtr->FieldCharge<MEDICOMP_USE_THRESHOLD)) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
			case WEAPON_PRED_RIFLE:
			case WEAPON_PRED_DISC:
				{
					if (this_weaponPtr->PrimaryRoundsRemaining==0 && this_weaponPtr->PrimaryMagazinesRemaining==0) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
			case WEAPON_PRED_PISTOL:
				{
					if (playerStatusPtr->FieldCharge<PredPistol_ShotCost) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
			case WEAPON_PRED_SHOULDERCANNON:
				{
					if ((playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart)
						&&(playerStatusPtr->FieldCharge<MUL_FIXED((Caster_Jumpstart-playerStatusPtr->PlasmaCasterCharge),Caster_ChargeRatio))) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
		}
	}

	return(0);
}

int Marine_WantToChangeWeapon(PLAYER_WEAPON_DATA *weaponPtr) {

	if (((weaponPtr->SecondaryRoundsRemaining==0 && weaponPtr->SecondaryMagazinesRemaining==0)
		&& (weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0))
		|| (weaponPtr->WeaponIDNumber==WEAPON_CUDGEL)) {
		return(1);		
	}
	return(0);
}

int WeaponHasAmmo(int slot) {
	
	if (slot!=-1) {
		PLAYER_WEAPON_DATA *this_weaponPtr;

	    this_weaponPtr = &(PlayerStatusPtr->WeaponSlot[slot]);

		if ((this_weaponPtr->SecondaryRoundsRemaining!=0 || this_weaponPtr->SecondaryMagazinesRemaining!=0)
			|| (this_weaponPtr->PrimaryRoundsRemaining!=0 || this_weaponPtr->PrimaryMagazinesRemaining!=0)
			||	(this_weaponPtr->WeaponIDNumber==WEAPON_CUDGEL)
			|| (
				(this_weaponPtr->WeaponIDNumber==WEAPON_GRENADELAUNCHER)
				&& ( 
				(GrenadeLauncherData.ProximityRoundsRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationRoundsRemaining!=0)
			 	||(GrenadeLauncherData.StandardRoundsRemaining!=0)
				||(GrenadeLauncherData.ProximityMagazinesRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationMagazinesRemaining!=0)
			 	||(GrenadeLauncherData.StandardMagazinesRemaining!=0)
				)
			)
			) {
			return(1);
		}
	}
	return(0);
}

int SimilarPredWeapons(PLAYER_WEAPON_DATA *nwp,PLAYER_WEAPON_DATA *owp) {

	if ((nwp->WeaponIDNumber==WEAPON_PRED_WRISTBLADE)
		||(nwp->WeaponIDNumber==WEAPON_PRED_SHOULDERCANNON)
		||(nwp->WeaponIDNumber==WEAPON_PRED_MEDICOMP)) {

		if ((owp->WeaponIDNumber==WEAPON_PRED_WRISTBLADE)
			||(owp->WeaponIDNumber==WEAPON_PRED_SHOULDERCANNON)
			||(owp->WeaponIDNumber==WEAPON_PRED_MEDICOMP)) {
				
			if (nwp->WeaponIDNumber!=owp->WeaponIDNumber) {
				return(1);
			}
		}
	}
	
	return(0);

}

enum PARTICLE_ID GetBloodType(STRATEGYBLOCK *sbPtr) {

	enum PARTICLE_ID blood_type=PARTICLE_NULL;

	if (sbPtr==NULL) {
		return(PARTICLE_NULL);
	}

	switch (sbPtr->I_SBtype) {
		default:
			blood_type=PARTICLE_NULL;
			break;
		case I_BehaviourHierarchicalFragment:
			{
				HDEBRIS_BEHAV_BLOCK *debrisStatusPointer;

				debrisStatusPointer=(HDEBRIS_BEHAV_BLOCK *)(sbPtr->SBdataptr);    
				GLOBALASSERT(debrisStatusPointer);

				if (debrisStatusPointer->Android) {
					blood_type=PARTICLE_ANDROID_BLOOD;
				} else {
					blood_type=PARTICLE_HUMAN_BLOOD;
				}
			}
			break;
		case I_BehaviourSpeargunBolt:
			{
				SPEAR_BEHAV_BLOCK *debrisStatusPointer;

				debrisStatusPointer=(SPEAR_BEHAV_BLOCK *)(sbPtr->SBdataptr);    
				GLOBALASSERT(debrisStatusPointer);

				if (debrisStatusPointer->Android) {
					blood_type=PARTICLE_ANDROID_BLOOD;
				} else {
					blood_type=PARTICLE_HUMAN_BLOOD;
				}
			}
			break;
		case I_BehaviourMarine:
		case I_BehaviourSeal:
			{
				MARINE_STATUS_BLOCK *marineStatusPointer;
			
				marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				GLOBALASSERT(marineStatusPointer);
				
				if (marineStatusPointer->Android) {
					blood_type=PARTICLE_ANDROID_BLOOD;
				} else {
					blood_type=PARTICLE_HUMAN_BLOOD;
				}
			}
			break;
		case I_BehaviourAlien:
		case I_BehaviourQueenAlien:
			blood_type=PARTICLE_ALIEN_BLOOD;
			break;
		case I_BehaviourPredator:
			blood_type=PARTICLE_PREDATOR_BLOOD;
			break;
		case I_BehaviourXenoborg:
		case I_BehaviourAutoGun:
			blood_type=PARTICLE_SPARK;
			break;
		case I_BehaviourNetCorpse:
			{
				NETCORPSEDATABLOCK *corpseDataPtr;

				corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
				LOCALASSERT(corpseDataPtr);

				switch (corpseDataPtr->Type) {
					default:
						blood_type=PARTICLE_NULL;
						break;
					case I_BehaviourMarinePlayer :
					case I_BehaviourMarine:
					case I_BehaviourSeal:
						if (corpseDataPtr->Android) {
							blood_type=PARTICLE_ANDROID_BLOOD;
						} else {
							blood_type=PARTICLE_HUMAN_BLOOD;
						}
						break;
					case I_BehaviourAlienPlayer :
					case I_BehaviourAlien:
					case I_BehaviourQueenAlien:
						blood_type=PARTICLE_ALIEN_BLOOD;
						break;
					case I_BehaviourPredatorPlayer:
					case I_BehaviourPredator:
						blood_type=PARTICLE_PREDATOR_BLOOD;
						break;
					case I_BehaviourXenoborg:
					case I_BehaviourAutoGun:
						blood_type=PARTICLE_SPARK;
						break;
				}
				
			}
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *corpseDataPtr;
				corpseDataPtr=sbPtr->SBdataptr;
				switch (corpseDataPtr->type) {
					default:
						blood_type=PARTICLE_NULL;
						break;
					case I_BehaviourMarinePlayer :
					case I_BehaviourMarine:
					case I_BehaviourSeal:
						/* Add an Android test here? */
						blood_type=PARTICLE_HUMAN_BLOOD;
						break;
					case I_BehaviourAlienPlayer :
					case I_BehaviourAlien:
					case I_BehaviourQueenAlien:
						blood_type=PARTICLE_ALIEN_BLOOD;
						break;
					case I_BehaviourPredatorPlayer:
					case I_BehaviourPredator:
						blood_type=PARTICLE_PREDATOR_BLOOD;
						break;
					case I_BehaviourXenoborg:
					case I_BehaviourAutoGun:
						blood_type=PARTICLE_SPARK;
						break;
					case I_BehaviourNetCorpse:
						switch (corpseDataPtr->subtype) {
							default:
								blood_type=PARTICLE_NULL;
								break;
							case I_BehaviourMarinePlayer :
							case I_BehaviourMarine:
							case I_BehaviourSeal:
								/* Add an Android test here? */
								blood_type=PARTICLE_HUMAN_BLOOD;
								break;
							case I_BehaviourAlienPlayer :
							case I_BehaviourAlien:
							case I_BehaviourQueenAlien:
								blood_type=PARTICLE_ALIEN_BLOOD;
								break;
							case I_BehaviourPredatorPlayer:
							case I_BehaviourPredator:
								blood_type=PARTICLE_PREDATOR_BLOOD;
								break;
							case I_BehaviourXenoborg:
							case I_BehaviourAutoGun:
								blood_type=PARTICLE_SPARK;
								break;
						}
						break;
				}		
			}
			break;
	}

	return(blood_type);
}

static int FirePrimaryLate,FireSecondaryLate;

void UpdateWeaponStateMachine(void)
{
	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
    int justfiredp,justfireds,ps;

    /* access the extra data hanging off the strategy block */
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    	
	/* player's current weapon */
    GLOBALASSERT(playerStatusPtr->SelectedWeaponSlot<MAX_NO_OF_WEAPON_SLOTS);
    
	if (playerStatusPtr->MyFaceHugger) {
		return;
	}

	CurrentGameStats_UsingWeapon(playerStatusPtr->SelectedWeaponSlot);

    /* init a pointer to the weapon's data */
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

	justfiredp=0; // Has the player just fired...
	justfireds=0; // Has the player just fired...
	ps=0; // If so, primary or secondary?  For rapid fire recoil handling, in WeaponStateIdle.

	FirePrimaryLate=0;
	FireSecondaryLate=0;

	/* Hack for autoswap commmands. */
	if (AutoSwap!=-1) {
        PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo=(AutoSwap+1);
		AutoSwap=-1;
	}

	/* Player doesn't have a weapon! This will eventually be changed into an assertion 
	that the player *does* have a weapon */
	if (weaponPtr->WeaponIDNumber == NULL_WEAPON)
		return;

	/* Player is dead. Weapon goes idle */
    if (!playerStatusPtr->IsAlive)
	{
		#if 0
		WeaponCreateStartFrame((void *)playerStatusPtr, weaponPtr);
		#endif

		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
    	weaponPtr->StateTimeOutCounter=0;
		return;
	}

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
    
 	CalculatePlayersTarget(twPtr,weaponPtr);

	playerStatusPtr->Encumberance=twPtr->Encum_Idle; //Default state

	textprint("Weapon State %d\n",weaponPtr->CurrentState);
	textprint("Weapon_ThisBurst %d\n",Weapon_ThisBurst);
	textprint("Primary Mags %d\n",weaponPtr->PrimaryMagazinesRemaining);
	textprint("Primary Rounds %d\n",weaponPtr->PrimaryRoundsRemaining);
	textprint("Players Target Distance %d\n",PlayersTarget.Distance);
	#if 0
	textprint("Minigun_HeadJolt %d %d %d\n",Minigun_HeadJolt.EulerX,Minigun_HeadJolt.EulerY,Minigun_HeadJolt.EulerZ);
	textprint("HeadOrientation %d %d %d\n",HeadOrientation.EulerX,HeadOrientation.EulerY,HeadOrientation.EulerZ);
	textprint("ViewPanX %d\n",playerStatusPtr->ViewPanX);
	#endif

    /* if weapon is not idle, evaluate any state changes required */
	if (WEAPONSTATE_IDLE!=weaponPtr->CurrentState)
	{
    	/* Time out current weapon state */
    	{
        	int timeOutRate = twPtr->TimeOutRateForState[weaponPtr->CurrentState];
            
			if (WEAPONSTATE_INSTANTTIMEOUT==timeOutRate)
            {
            	weaponPtr->StateTimeOutCounter=0;
            }
            else
           	{
            	weaponPtr->StateTimeOutCounter-=MUL_FIXED(timeOutRate,NormalFrameTime);
            }
        
        }
        if(weaponPtr->StateTimeOutCounter<=0)
        {
			WeaponFidgetPlaying=0;
			/* It's the only way to be sure. */
        	switch(weaponPtr->CurrentState)
			{
                case WEAPONSTATE_FIRING_PRIMARY:
                {
				   	if (twPtr->PrimaryIsMeleeWeapon) {
						/* Melee weapons fire at the end of the 'firing' phase, in	 *
						 * addition to other special properties, like infinite ammo. */
						if (twPtr->FirePrimaryFunction!=NULL) {
							if ((*twPtr->FirePrimaryFunction)(weaponPtr)) {
								/* Er... well done. */
								/* It might be important to put something here, okay? */
								/* Till then, this will be compiled out. */
							}
						}
					}

					if (twPtr->PrimaryIsRapidFire)
					{
						/* after shooting weapon goes straight to idle state */
						weaponPtr->CurrentState = WEAPONSTATE_IDLE;
						justfiredp=weaponPtr->StateTimeOutCounter+1;
						ps=1;
				    	weaponPtr->StateTimeOutCounter=0;
					}
					else
					{
						/* there is a 'recovery' time after shooting */
						weaponPtr->CurrentState = WEAPONSTATE_RECOIL_PRIMARY;
						weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
					}
				           
					playerStatusPtr->Encumberance=twPtr->Encum_FirePrime;
				                	
                	break;
				}
                case WEAPONSTATE_FIRING_SECONDARY:
                {
					
				   	if (twPtr->SecondaryIsMeleeWeapon) {
						/* Melee weapons fire at the end of the 'firing' phase, in	 *
						 * addition to other special properties, like infinite ammo. */
						if (twPtr->FireSecondaryFunction!=NULL) {
							if ((*twPtr->FireSecondaryFunction)(weaponPtr)) {
								/* Er... well done. */
								/* It might be important to put something here, okay? */
								/* Till then, this will be compiled out. */
							}
						}
					}

					if (twPtr->SecondaryIsRapidFire)
					{
						/* after shooting weapon goes straight to idle state */
						weaponPtr->CurrentState = WEAPONSTATE_IDLE;
						justfireds=weaponPtr->StateTimeOutCounter+1;
						ps=2;
				    	weaponPtr->StateTimeOutCounter=0;
					}
					else
					{
						/* there is a 'recovery' time after shooting */
						weaponPtr->CurrentState = WEAPONSTATE_RECOIL_SECONDARY;
						weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
					}
				            
					playerStatusPtr->Encumberance=twPtr->Encum_FireSec;
				                	
                	break;
				}
            
			    case WEAPONSTATE_RELOAD_PRIMARY:
			    {
			    	/* load a new magazine */
                    TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];
					if (weaponPtr->PrimaryRoundsRemaining==0) {
						if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
							/* Two pistols reloads BOTH primary and secondary. */
							if (weaponPtr->PrimaryMagazinesRemaining) {
			                	weaponPtr->PrimaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
			                	weaponPtr->PrimaryMagazinesRemaining--;
							}
							if (weaponPtr->SecondaryMagazinesRemaining) {
			                	weaponPtr->SecondaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
			                	weaponPtr->SecondaryMagazinesRemaining--;
							}
						} else {
							/* Grenade launcher has already reloaded at this point. */
		                	weaponPtr->PrimaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
		                	weaponPtr->PrimaryMagazinesRemaining--;
						}
					}
               		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
			    	weaponPtr->StateTimeOutCounter=0;
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;

		        	break;
		        }
			    case WEAPONSTATE_RELOAD_SECONDARY:
			    {
			    	/* load a new magazine */
                    TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->SecondaryAmmoID];
                	weaponPtr->SecondaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
                	weaponPtr->SecondaryMagazinesRemaining--;
               		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
			    	weaponPtr->StateTimeOutCounter=0;
					
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;

		        	break;
		        }
                
                case WEAPONSTATE_SWAPPING_IN:
                case WEAPONSTATE_SWAPPING_OUT:
                {
                  	if (playerStatusPtr->SwapToWeaponSlot!=WEAPON_FINISHED_SWAPPING)
                    {
           		       	PLAYER_WEAPON_DATA *newWeaponPtr;
           				PLAYER_WEAPON_DATA *nwp,*owp;

						nwp=&(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
						owp=&(playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot]);
	
						playerStatusPtr->PreviouslySelectedWeaponSlot = playerStatusPtr->SelectedWeaponSlot;
                    	playerStatusPtr->SelectedWeaponSlot = playerStatusPtr->SwapToWeaponSlot;
                    	newWeaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot]);
                        
                    	playerStatusPtr->SwapToWeaponSlot = WEAPON_FINISHED_SWAPPING;
            		    
						if (SimilarPredWeapons(nwp,owp)	&& (weaponPtr->CurrentState==WEAPONSTATE_SWAPPING_IN)) {

							/* Special case. */
	                    	newWeaponPtr->CurrentState = WEAPONSTATE_READYING;
							newWeaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
                    		weaponPtr = newWeaponPtr;
    						twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
						
						} else {

            		    	newWeaponPtr->CurrentState = WEAPONSTATE_SWAPPING_IN;
                			newWeaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
                    		
                    		weaponPtr = newWeaponPtr;
    						twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
							
							GrabWeaponShape(weaponPtr);
							
							if (!(twPtr->PrimaryIsMeleeWeapon)) {
								GrabMuzzleFlashShape(twPtr);
							}
							PlayersWeapon.ObTxAnimCtrlBlks=weaponPtr->TxAnimCtrl;
							if (twPtr->HasShapeAnimation) {
								PlayersWeapon.ShapeAnimControlBlock=&weaponPtr->ShpAnimCtrl;
							} else {
								PlayersWeapon.ShapeAnimControlBlock=NULL;
							}
						}
                    }
					else
                   	{
                    	weaponPtr->CurrentState = WEAPONSTATE_READYING;
						weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
                    }
                	
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;
                	
                	break;
                }
                	        
		        case WEAPONSTATE_RECOIL_PRIMARY:
				{
					if (!twPtr->PrimaryIsAutomatic && playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon)
					{
		        		weaponPtr->CurrentState = WEAPONSTATE_WAITING;
					}
					else
					{
		        		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
				    	weaponPtr->StateTimeOutCounter=0;
					}
					
					playerStatusPtr->Encumberance=twPtr->Encum_FirePrime;
					
					break;
				}
		        case WEAPONSTATE_RECOIL_SECONDARY:
				{
					if (!twPtr->SecondaryIsAutomatic && playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon)
					{
		        		weaponPtr->CurrentState = WEAPONSTATE_WAITING;
					}
					else
					{
		        		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
				    	weaponPtr->StateTimeOutCounter=0;
					}
					playerStatusPtr->Encumberance=twPtr->Encum_FireSec;
					break;
				}


				case WEAPONSTATE_WAITING:
				{
					/* wait for player to take his finger of fire */
					if( (!(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon
					     ||playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon))
					|| (/* Alien Claw! */ 
						(weaponPtr->WeaponIDNumber == WEAPON_ALIEN_CLAW)
						&&(twPtr->PrimaryIsAutomatic)
						&&(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon)
						) )
					{
		        		weaponPtr->CurrentState = WEAPONSTATE_IDLE;
				    	weaponPtr->StateTimeOutCounter=0;
					}
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;
					break;
				}
		        
				case WEAPONSTATE_UNREADYING:
		        {
					if (SimilarPredWeapons(weaponPtr,&playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot])) {
           		       	
           		       	PLAYER_WEAPON_DATA *newWeaponPtr;
						/* Special case. */
						playerStatusPtr->PreviouslySelectedWeaponSlot = playerStatusPtr->SelectedWeaponSlot;
                    	playerStatusPtr->SelectedWeaponSlot = playerStatusPtr->SwapToWeaponSlot;
                    	newWeaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot]);
                    	playerStatusPtr->SwapToWeaponSlot = WEAPON_FINISHED_SWAPPING;

						newWeaponPtr->CurrentState = WEAPONSTATE_READYING;
						newWeaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
                    	
                    	weaponPtr = newWeaponPtr;
    					twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
						//GrabWeaponShape(weaponPtr);
						
						if (!(twPtr->PrimaryIsMeleeWeapon)) GrabMuzzleFlashShape(twPtr);
						PlayersWeapon.ObTxAnimCtrlBlks=weaponPtr->TxAnimCtrl;
						if (twPtr->HasShapeAnimation) {
							PlayersWeapon.ShapeAnimControlBlock=&weaponPtr->ShpAnimCtrl;
						} else {
							PlayersWeapon.ShapeAnimControlBlock=NULL;
						}

					} else {
						/* Change Me! */
						weaponPtr->CurrentState = WEAPONSTATE_SWAPPING_OUT;
						weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
						playerStatusPtr->Encumberance=twPtr->Encum_Idle;
					}
		        	break;
		        }
				case WEAPONSTATE_READYING:
		        {
					//NewOnScreenMessage("WEAPON READY");
					/* That's TEMPORARY!  For TESTING! */
		        	weaponPtr->CurrentState = WEAPONSTATE_IDLE;
			    	weaponPtr->StateTimeOutCounter=0;
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;
		        	break;
		        }

		        default: /* applicable to many states, eg. WEAPONSTATE_JAMMED */
		        {
                	/* if timed-out return to idle state */
		        	weaponPtr->CurrentState = WEAPONSTATE_IDLE;
			    	weaponPtr->StateTimeOutCounter=0;
					playerStatusPtr->Encumberance=twPtr->Encum_Idle;
		        	break;
		        }
		    }		    
        }
	}
    else {
       	int timeOutRate = twPtr->TimeOutRateForState[weaponPtr->CurrentState];

		/* WEAPONSTATE_IDLE counts UP. */

        weaponPtr->StateTimeOutCounter+=MUL_FIXED(timeOutRate,NormalFrameTime);
		textprint("WeaponstateIdle Time Counter = %d\n",weaponPtr->StateTimeOutCounter);
					
    	//weaponPtr->StateTimeOutCounter=0;
	}
    
	switch(weaponPtr->CurrentState)
	{
	    case WEAPONSTATE_JAMMED:
        {
    		if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon)
    		{
            	// MakeClickingNoise function goes here!
				PlayWeaponClickingNoise(weaponPtr->WeaponIDNumber);
            }
    		break;	
        }
        case WEAPONSTATE_IDLE:
        {
			WeaponStateIdle(playerStatusPtr,weaponPtr,twPtr,justfiredp,justfireds,ps);
			break;	
        }
		case WEAPONSTATE_UNREADYING:
		{
			enum WEAPON_SLOT oldSlot=playerStatusPtr->SwapToWeaponSlot;
		   	if(RequestChangeOfWeaponWhilstSwapping(playerStatusPtr,weaponPtr))
			{
				if (playerStatusPtr->SwappingIsDebounced)
				{
					//if (oldSlot==WEAPON_FINISHED_SWAPPING)
					//	weaponPtr->StateTimeOutCounter = 65536-weaponPtr->StateTimeOutCounter;
					//?

					NewOnScreenMessage
					(
						GetTextString
						(
							TemplateWeapon[playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot].WeaponIDNumber].Name
						)
					);
					playerStatusPtr->SwappingIsDebounced = 0;
				}
				else
				{
					playerStatusPtr->SwapToWeaponSlot = oldSlot;
				}
			}
			else playerStatusPtr->SwappingIsDebounced = 1;
			break;
		}
		case WEAPONSTATE_READYING:
       	case WEAPONSTATE_SWAPPING_IN:
        case WEAPONSTATE_SWAPPING_OUT:
		{
			enum WEAPON_SLOT oldSlot=playerStatusPtr->SwapToWeaponSlot;

		   	if(RequestChangeOfWeaponWhilstSwapping(playerStatusPtr,weaponPtr))
			{
				if (playerStatusPtr->SwappingIsDebounced)
				{
					
					if (weaponPtr->CurrentState==WEAPONSTATE_READYING) {
						/* Back outta here? */
						weaponPtr->CurrentState=WEAPONSTATE_UNREADYING;
						weaponPtr->StateTimeOutCounter = 65536-weaponPtr->StateTimeOutCounter;
						if (twPtr->UseStateMovement==0) {
							/* I guess it's hierarchical... */
							PlayersWeaponHModelController.Reversed=1;
						}
					} else if (SimilarPredWeapons(&playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot],&playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot])) {
					
						/* Very special case.. */

					} else if (oldSlot==WEAPON_FINISHED_SWAPPING) {
						weaponPtr->StateTimeOutCounter = 65536-weaponPtr->StateTimeOutCounter;
						if (twPtr->UseStateMovement==0) {
							/* I guess it's hierarchical... */
							PlayersWeaponHModelController.Reversed=1;
						}
					}

					NewOnScreenMessage
					(
						GetTextString
						(
							TemplateWeapon[playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot].WeaponIDNumber].Name
						)
					);
					playerStatusPtr->SwappingIsDebounced = 0;
				}
				else
				{
					playerStatusPtr->SwapToWeaponSlot = oldSlot;
				}
			}
			else playerStatusPtr->SwappingIsDebounced = 1;
			
			break;
		}
        default:
        {
        	break;
        }
        
	}
	StateDependentMovement(playerStatusPtr,weaponPtr);
	PositionPlayersWeapon();
    
}

static void WeaponStateIdle(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr,TEMPLATE_WEAPON_DATA *twPtr, int justfiredp, int justfireds, int ps)
{
	int CanFirePrimary, CanFireSecondary;
	int WishToFirePrimary,PrimaryFired;

	CanFirePrimary=1;
	CanFireSecondary=1;
	PrimaryFired=0;

	if ( (twPtr->FireInChangeVision==0)&&(IsVisionChanging()) ) {
		CanFirePrimary=0;
		CanFireSecondary=0;
	}

	#if 0
	if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
		CanFirePrimary=0;
		CanFireSecondary=0;
	}
	#endif

	WishToFirePrimary=playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon;

	if (weaponPtr->WeaponIDNumber == WEAPON_MINIGUN) {
		if ((Weapon_ThisBurst<MINIGUN_MINIMUM_BURST)&&(Weapon_ThisBurst>=0)
			&&(weaponPtr->PrimaryRoundsRemaining)) {
			WishToFirePrimary=1;
			/* Retain CanFirePrimary. */
		}
		if (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor==0) {
			CanFirePrimary=0;
			Weapon_ThisBurst = -1;
		}
	} else if (weaponPtr->WeaponIDNumber == WEAPON_PULSERIFLE) {
		if ((Weapon_ThisBurst<PULSERIFLE_MINIMUM_BURST)&&(Weapon_ThisBurst>=0)
			&&(weaponPtr->PrimaryRoundsRemaining)) {
			WishToFirePrimary=1;
			/* Retain CanFirePrimary. */
		}
	} else if (weaponPtr->WeaponIDNumber == WEAPON_SMARTGUN) {
		if ((Weapon_ThisBurst<SMARTGUN_MINIMUM_BURST)&&(Weapon_ThisBurst>=0)
			&&(weaponPtr->PrimaryRoundsRemaining)) {
			WishToFirePrimary=1;
			/* Retain CanFirePrimary. */
		}
	}

	/* does the player wish to fire primary ammo*/
   	if ((WishToFirePrimary)&&(CanFirePrimary)) {
    	if (twPtr->PrimaryAmmoID != AMMO_NONE) {
       		if (twPtr->PrimaryIsMeleeWeapon) {
   	    	
				if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
					/* Force Decloak. */
					playerStatusPtr->cloakOn=0;
					Sound_Play(SID_PRED_CLOAKOFF,"h");
					//playerNoise=1;
				}

   	    		weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
				if (justfiredp) {
					weaponPtr->StateTimeOutCounter = justfiredp-1;
				} else {
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
				}
				/* KJL 11:46:42 03/04/97 - sound effects? */
			} else if ((weaponPtr->PrimaryRoundsRemaining)
				||( (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS)&&(weaponPtr->SecondaryRoundsRemaining) )) {
			
       			/* consider probability of jamming */
   	  		   	if (twPtr->ProbabilityOfJamming > Random16BitNumber) {
					weaponPtr->CurrentState = WEAPONSTATE_JAMMED;
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
       		  	} else { /* okay, the weapon is able to fire */
        	    	
					if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
						/* Force Decloak. */
						playerStatusPtr->cloakOn=0;
						Sound_Play(SID_PRED_CLOAKOFF,"h");
						//playerNoise=1;
					}

					if (twPtr->FirePrimaryFunction!=NULL) {
						/* FIRE!!! */
						if (twPtr->FirePrimaryLate) {
							if (weaponPtr->WeaponIDNumber == WEAPON_PRED_PISTOL) {
								/* A little hackette, since I'm tired. */
								if (playerStatusPtr->FieldCharge>0) {
									FirePrimaryLate=1;
		        	      			weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
									weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
								}
							} else {
								FirePrimaryLate=1;
	        	      			weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
								weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
							}
						} else if ((*twPtr->FirePrimaryFunction)(weaponPtr)) {
							if (twPtr->PrimaryMuzzleFlash) {
								if (twPtr->PrimaryAmmoID==AMMO_PARTICLE_BEAM) {
									AddLightingEffectToObject(Player,LFX_PARTICLECANNON);
								} else {
									AddLightingEffectToObject(Player,LFX_MUZZLEFLASH);
								}
							}
        	      			weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
							weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
						}
					}
					PrimaryFired=1;
				}
			}
			else
			{
				/* KJL 16:07:23 23/11/98 - trying to fire a weapon with no ammo */
				PlayWeaponClickingNoise(weaponPtr->WeaponIDNumber);
				if (weaponPtr->WeaponIDNumber == WEAPON_MINIGUN) {
					FireEmptyMinigun(weaponPtr);
				}
        	}
		} else if (twPtr->FirePrimaryFunction!=NULL) {

        	int timeOutRate = twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY];

			if (timeOutRate!=WEAPONSTATE_INSTANTTIMEOUT) {
				if ((*twPtr->FirePrimaryFunction)(weaponPtr)) {
	        		weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
				}
			} else {
				/* Do something special anyway... */
				(*twPtr->FirePrimaryFunction)(weaponPtr);
        		weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
				weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
    } else if((playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon)&&(CanFireSecondary)) {
		/* or the secondary ammo */
    	if (twPtr->SecondaryAmmoID != AMMO_NONE) {
           	if (twPtr->SecondaryIsMeleeWeapon) {
				if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
					/* Force Decloak. */
					playerStatusPtr->cloakOn=0;
					Sound_Play(SID_PRED_CLOAKOFF,"h");
					//playerNoise=1;
				}
       	    	weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
				if (justfireds) {
					weaponPtr->StateTimeOutCounter = justfireds-1;
				} else {
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
				}
				/* KJL 11:46:42 03/04/97 - sound effects? */
			} else if ( (weaponPtr->SecondaryRoundsRemaining)
				||( (weaponPtr->WeaponIDNumber==WEAPON_MARINE_PISTOL)&&(weaponPtr->PrimaryRoundsRemaining) )
				||( (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS)&&(weaponPtr->PrimaryRoundsRemaining) )) {
           		/* consider probability of jamming */
       	  	   	if (twPtr->ProbabilityOfJamming > Random16BitNumber) {
					weaponPtr->CurrentState = WEAPONSTATE_JAMMED;
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
       	  	   	} else {
					/* okay, the weapon is able to fire */
					if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
						/* Force Decloak. */
						playerStatusPtr->cloakOn=0;
						Sound_Play(SID_PRED_CLOAKOFF,"h");
						//playerNoise=1;
					}
					if (twPtr->FireSecondaryFunction!=NULL) {
						/* FIRE!!! */
						if (twPtr->FireSecondaryLate) {
							FireSecondaryLate=1;
	              	    	weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
							weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
						} else if ((*twPtr->FireSecondaryFunction)(weaponPtr)) {
							if (twPtr->SecondaryMuzzleFlash) {					
								AddLightingEffectToObject(Player,LFX_MUZZLEFLASH);
		              	    	weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
								weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
							}
						}
					}
				}
			} else {
            	// MakeClickingNoise function goes here!
				PlayWeaponClickingNoise(weaponPtr->WeaponIDNumber);
            }
		} else if (twPtr->FireSecondaryFunction!=NULL) {

        	int timeOutRate = twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_SECONDARY];

			if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
				/* Force Decloak. */
				playerStatusPtr->cloakOn=0;
				Sound_Play(SID_PRED_CLOAKOFF,"h");
				//playerNoise=1;
			}

			if (timeOutRate!=WEAPONSTATE_INSTANTTIMEOUT) {
				if ((*twPtr->FireSecondaryFunction)(weaponPtr)) {
	        		weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
				}
			} else {
				/* Do something special anyway... */
				(*twPtr->FireSecondaryFunction)(weaponPtr);
        		weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
				weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
    } else if (ps) {
		/* Not firing now, for some reason.  But you were. *
		 * Not only that, but your're also rapid fire.     *
		 * For example, Doom Plasma Gun has rapid fire and *
		 * recoil: also anything that 'charges', like tail.*/
		if (ps==1) {
			if (twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_PRIMARY]!=WEAPONSTATE_INSTANTTIMEOUT) {
				/* there is a 'recovery' time after shooting */
				weaponPtr->CurrentState = WEAPONSTATE_RECOIL_PRIMARY;
				weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		} else {
			if (twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_SECONDARY]!=WEAPONSTATE_INSTANTTIMEOUT) {
				/* there is a 'recovery' time after shooting */
				weaponPtr->CurrentState = WEAPONSTATE_RECOIL_SECONDARY;
				weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
    } else if ((RequestChangeOfWeapon(playerStatusPtr,weaponPtr))
    	||((playerStatusPtr->SelectedWeaponSlot!=playerStatusPtr->SwapToWeaponSlot)
    		&&(playerStatusPtr->SwapToWeaponSlot!=WEAPON_FINISHED_SWAPPING))) {
		weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
	    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
		NewOnScreenMessage
		(
			GetTextString
			(
				TemplateWeapon[playerStatusPtr->WeaponSlot[playerStatusPtr->SwapToWeaponSlot].WeaponIDNumber].Name
			)
		);
	} else if (weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining!=0)	{
    	/* if you've ran out of ammo, but you've got some magazines, reload. */
    	if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
			/* Two pistols ammo handling! */
			if (weaponPtr->SecondaryRoundsRemaining==0) {
				if (weaponPtr->SecondaryMagazinesRemaining!=0) {
			       	weaponPtr->CurrentState = WEAPONSTATE_RELOAD_PRIMARY;
			   		weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
				} else {
					/* Change to one pistol if you can! */
					int pistol_slot;

					pistol_slot=SlotForThisWeapon(WEAPON_MARINE_PISTOL);
					if (pistol_slot!=-1) {
						playerStatusPtr->SwapToWeaponSlot = pistol_slot;
						weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
						weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
					} else {
						/* Utterly fubared. */
						weaponPtr->SecondaryMagazinesRemaining=0;
						return;
					}
				}
			}
	    } else {
	       	weaponPtr->CurrentState = WEAPONSTATE_RELOAD_PRIMARY;
	   		weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
		}
	#if 0
	} else if ((weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0)
		&& (AvP.PlayerType==I_Marine)
		&& (weaponPtr->WeaponIDNumber==WEAPON_GRENADELAUNCHER)
		&& ( 
				(GrenadeLauncherData.ProximityRoundsRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationRoundsRemaining!=0)
			 	||(GrenadeLauncherData.StandardRoundsRemaining!=0)
				||(GrenadeLauncherData.ProximityMagazinesRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationMagazinesRemaining!=0)
			 	||(GrenadeLauncherData.StandardMagazinesRemaining!=0)
			)
		) {
		/* Deal with Al's 'grenade launcher change ammo' case... */
		GrenadeLauncher_EmergencyChangeAmmo(weaponPtr);
	} else if ((weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0)
		&& (AvP.PlayerType==I_Marine)
		&& ( (weaponPtr->WeaponIDNumber!=WEAPON_GRENADELAUNCHER)
			|| ( 
				(GrenadeLauncherData.ProximityRoundsRemaining==0)
			  	&&(GrenadeLauncherData.FragmentationRoundsRemaining==0)
			 	&&(GrenadeLauncherData.StandardRoundsRemaining==0)
				&&(GrenadeLauncherData.ProximityMagazinesRemaining==0)
			  	&&(GrenadeLauncherData.FragmentationMagazinesRemaining==0)
			 	&&(GrenadeLauncherData.StandardMagazinesRemaining==0)
			))
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PULSERIFLE)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_CUDGEL)
		) {
		/* Auto change to pulserifle. */
		int slot;
				
		/*Don't swap weapons if the marine is a specialist marine*/
		if(AvP.Network == I_No_Network || netGameData.myCharacterSubType==NGSCT_General)
		{
			slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
			if (slot==-1) {
				/* Argh! Whadda ya mean, you've got no pulse rifle? */
			} else {
	    		playerStatusPtr->SwapToWeaponSlot = slot;
				weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
			    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
	} else if ((weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0)
		&& (AvP.PlayerType==I_Marine)
		&& (weaponPtr->WeaponIDNumber==WEAPON_PULSERIFLE)
		&& (weaponPtr->SecondaryRoundsRemaining==0 && weaponPtr->SecondaryMagazinesRemaining==0)
		) {
		/* If you _are_ the pulserifle, and you have no rounds left, try to get the cudgel. */
		if(AvP.Network == I_No_Network || netGameData.myCharacterSubType==NGSCT_General)
		{
			int slot;
			/* Might want to check for any other weapons with ammo here? */
			slot=SlotForThisWeapon(WEAPON_CUDGEL);
			if (slot!=-1) {
	    		playerStatusPtr->SwapToWeaponSlot = slot;
				weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
			    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
	} else if ((AvP.PlayerType==I_Marine)
		&& (weaponPtr->WeaponIDNumber==WEAPON_CUDGEL)
		) {
		/* If cudgel is selected, and the pulserifle has ammo, change to _it_. */
		int pulserifle_slot;
		
		pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
		if (pulserifle_slot!=-1) {
			if (WeaponHasAmmo(pulserifle_slot)) {
	    		playerStatusPtr->SwapToWeaponSlot = pulserifle_slot;
				weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
			    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
	#else
	} else if ((AvP.PlayerType==I_Marine)
		&&(Marine_WantToChangeWeapon(weaponPtr))) {
		MarineZeroAmmoFunctionality(playerStatusPtr,weaponPtr);
	#endif

	#if 0
	} else if ((
		(weaponPtr->PrimaryRoundsRemaining==0 && weaponPtr->PrimaryMagazinesRemaining==0)
		&& (AvP.PlayerType==I_Predator)
		#if 0
		/* Why is this line here? */
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_RIFLE)
		/* I find this VERY scary.  The Phantom Code Changer is at work. */
		#endif
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_WRISTBLADE)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_STAFF)
		&& (weaponPtr->WeaponIDNumber!=WEAPON_PRED_MEDICOMP)
		)||( 
		(AvP.PlayerType==I_Predator)&&(weaponPtr->WeaponIDNumber==WEAPON_PRED_SHOULDERCANNON)
		&&(playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart)
		&&(playerStatusPtr->FieldCharge<MUL_FIXED((Caster_Jumpstart-playerStatusPtr->PlasmaCasterCharge),Caster_ChargeRatio))
		)||( 
		(AvP.PlayerType==I_Predator)&&(weaponPtr->WeaponIDNumber==WEAPON_PRED_PISTOL)
		&&(playerStatusPtr->FieldCharge<PredPistol_ShotCost)
		)) {
		/* Auto change to wristblade. */
		int slot;
				
		slot=SlotForThisWeapon(WEAPON_PRED_WRISTBLADE);
		if (slot==-1) {
			/* Argh! Whadda ya mean, you've got no wristblade? */
			GLOBALASSERT(0);
		} else {
	    	playerStatusPtr->SwapToWeaponSlot = slot;
			weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
		    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
		}
	#else
	} else if ((AvP.PlayerType==I_Predator)
		&& (Predator_WantToChangeWeapon(playerStatusPtr,weaponPtr))) {
		PredatorZeroAmmoFunctionality(playerStatusPtr,weaponPtr);
	#endif
    } else if (twPtr->SecondaryAmmoID != AMMO_NONE) {
    	/* if you've ran out of ammo, but you've got some magazines, reload. */
		if (weaponPtr->SecondaryRoundsRemaining==0 && weaponPtr->SecondaryMagazinesRemaining!=0) {
	    	if (weaponPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
				/* Two pistols ammo handling! */
				if (weaponPtr->PrimaryRoundsRemaining==0) {
					if (weaponPtr->PrimaryMagazinesRemaining!=0) {
				       	weaponPtr->CurrentState = WEAPONSTATE_RELOAD_PRIMARY;
				   		weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
					} else {
						/* Change to one pistol if you can! */
						int pistol_slot;

						pistol_slot=SlotForThisWeapon(WEAPON_MARINE_PISTOL);
						if (pistol_slot!=-1) {
							playerStatusPtr->SwapToWeaponSlot = pistol_slot;
							weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
							weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
						} else {
							/* Utterly fubared. */
							weaponPtr->PrimaryMagazinesRemaining=0;
							return;
						}
					}
				}
		    } else {
	          	weaponPtr->CurrentState = WEAPONSTATE_RELOAD_SECONDARY;
		   		weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
    	}
	} else if ((AvP.PlayerType==I_Predator)&&(weaponPtr->WeaponIDNumber==WEAPON_PRED_DISC)) {
		SECTION_DATA *disc_section;

		/* You should have a disc. */
		disc_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"disk");
		GLOBALASSERT(disc_section);
		/* Force Appear Disc. */
		disc_section->flags&=~section_data_notreal;

	} else {
		/* Wow, genuinely idle! */
	}

	if ((weaponPtr->WeaponIDNumber == WEAPON_PULSERIFLE)
		||(weaponPtr->WeaponIDNumber == WEAPON_SMARTGUN)) {
		if (!PrimaryFired) {
			/* Maybe Reset? */
			Weapon_ThisBurst=-1;
		}
	}
}

static int RequestChangeOfWeapon(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr)
{
	playerStatusPtr->SwappingIsDebounced = 0;

	if (playerStatusPtr->MyFaceHugger) {
		return(0);
	}

    /* else if you wish to change to the previous weapon */
    if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PreviousWeapon)
   	{
    	enum WEAPON_SLOT newSlot = playerStatusPtr->SelectedWeaponSlot;
		int slotValidity;
       	
		slotValidity=0;

        do {
	   		if(newSlot-- == WEAPON_SLOT_1) {
     			newSlot=MAX_NO_OF_WEAPON_SLOTS-1;
			}
			
			if (playerStatusPtr->WeaponSlot[newSlot].Possessed==1) {
				slotValidity=1;
			}
			/* But not if you're a disk launcher with no disks. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[newSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[newSlot].PrimaryMagazinesRemaining==0) {
					slotValidity=0;
				}
			}
			/* And not if you're the cudgel. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->SelectedWeaponSlot!=newSlot) {
					slotValidity=0;
				}
			}
			/* But, if you are the cudgel, ignore the pulserifle unless it has ammo. */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(newSlot)) {
						slotValidity=0;
					}
				}
			} else {
				/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
				int pulserifle_slot;
				int cudgel_slot;

				cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);

				if (cudgel_slot!=-1) {
					if (playerStatusPtr->WeaponSlot[cudgel_slot].Possessed!=1) {
						cudgel_slot=-1;
					}
				}

				if (cudgel_slot!=-1) {
					pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
					
					if (pulserifle_slot!=-1) {					
						if (!WeaponHasAmmo(pulserifle_slot)) {
							newSlot=cudgel_slot;
							slotValidity=1;		
						}
					}
				}
			}
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                slotValidity=0;
            }
		} while(slotValidity==0);

		if(newSlot != playerStatusPtr->SelectedWeaponSlot)
		{
		    playerStatusPtr->SwapToWeaponSlot = newSlot;
		  	return 1;
		}
    }
    /* else if you wish to change to the next weapon */
    else if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_NextWeapon)
    {
		enum WEAPON_SLOT newSlot = playerStatusPtr->SelectedWeaponSlot;
		int slotValidity;
       	
		slotValidity=0;

        do {
	   		if(++newSlot == MAX_NO_OF_WEAPON_SLOTS) {
     			newSlot=WEAPON_SLOT_1;
			}
			if (playerStatusPtr->WeaponSlot[newSlot].Possessed==1) {
				slotValidity=1;
			}
			/* But not if you're a disk launcher with no disks. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[newSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[newSlot].PrimaryMagazinesRemaining==0) {
					slotValidity=0;
				}
			}
			/* And not if you're the cudgel. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->SelectedWeaponSlot!=newSlot) {
					slotValidity=0;
				}
			}
			/* But, if you are the cudgel, ignore the pulserifle unless it has ammo. */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(newSlot)) {
						slotValidity=0;
					}
				}
			} else {
				/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
				int pulserifle_slot;
				int cudgel_slot;

				cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);

				if (cudgel_slot!=-1) {
					if (playerStatusPtr->WeaponSlot[cudgel_slot].Possessed!=1) {
						cudgel_slot=-1;
					}
				}

				if (cudgel_slot!=-1) {
					pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
					
					if (pulserifle_slot!=-1) {					
						if (!WeaponHasAmmo(pulserifle_slot)) {
							newSlot=cudgel_slot;
							slotValidity=1;		
						}
					}
				}
			}
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                slotValidity=0;
            }
		} while(slotValidity==0);
        
		if(newSlot != playerStatusPtr->SelectedWeaponSlot)
		{
		    playerStatusPtr->SwapToWeaponSlot = newSlot;
		  	return 1;
		}
    }
	/* else pick a weapon, any weapon */
	else if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo)
    {
        enum WEAPON_SLOT requestedSlot = (enum WEAPON_SLOT)(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo - 1);

        LOCALASSERT(requestedSlot < MAX_NO_OF_WEAPON_SLOTS);
        LOCALASSERT(requestedSlot >= 0);
        
        if( (requestedSlot != playerStatusPtr->SelectedWeaponSlot)
          &&(playerStatusPtr->WeaponSlot[requestedSlot].Possessed == 1) )
        { 
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                return 0;
            }
			if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[requestedSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[requestedSlot].PrimaryMagazinesRemaining==0) {
					#if 0
					sprintf(tempstring,"NO AMMO FOR %s\n",GetTextString(
						TemplateWeapon[playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber].Name)
					);			
					NewOnScreenMessage(tempstring);
					#else
					//NewOnScreenMessage(GetTextString(TEXTSTRING_NOAMMOFORWEAPON));
					#endif
					return 0;
				}
			}
			/* Look, I said you can't select the cudgel! */
			if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				return 0;
			}
	    	/* Of course, if you are the cudgel and the the pulserifle has no ammo... */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(requestedSlot)) {
						return(0);
					}
				}
			} else {
				if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
					int pulserifle_slot;
					int cudgel_slot;

					cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);
					if (cudgel_slot!=-1) {
						pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
						
						if (pulserifle_slot!=-1) {					
							if (!WeaponHasAmmo(pulserifle_slot)) {
								requestedSlot=cudgel_slot;
							}
						}
					}
				}
			}

	    	playerStatusPtr->SwapToWeaponSlot = requestedSlot;
		  	return 1;
    	} else if( (requestedSlot != playerStatusPtr->SelectedWeaponSlot)
          &&(playerStatusPtr->WeaponSlot[requestedSlot].Possessed == -1) )
		{
			#if 0
			sprintf(tempstring,"%s NOT AVAILABLE IN DEMO\n",GetTextString(
				TemplateWeapon[playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber].Name)
			);			
			NewOnScreenMessage(tempstring);
			#endif
		}
    }

   	return 0;
}
static int RequestChangeOfWeaponWhilstSwapping(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr)
{
    enum WEAPON_SLOT currentSlot;

	if (playerStatusPtr->MyFaceHugger) {
		return(0);
	}

	if (playerStatusPtr->SwapToWeaponSlot == WEAPON_FINISHED_SWAPPING)
	{
		currentSlot = playerStatusPtr->SelectedWeaponSlot;
	}
	else 
	{
		currentSlot = playerStatusPtr->SwapToWeaponSlot;
	}
    
    
    /* else if you wish to change to the previous weapon */
    if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PreviousWeapon)
   	{
    	enum WEAPON_SLOT newSlot = currentSlot;
       	int slotValidity=0;

        do
		{
	   		if(newSlot-- == WEAPON_SLOT_1) {
     			newSlot=MAX_NO_OF_WEAPON_SLOTS-1;
			}
			if (playerStatusPtr->WeaponSlot[newSlot].Possessed==1) {
				slotValidity=1;
			}
			/* But not if you're a disk launcher with no disks. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[newSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[newSlot].PrimaryMagazinesRemaining==0) {
					slotValidity=0;
				}
			}
			/* And not if you're the cudgel. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->SelectedWeaponSlot!=newSlot) {
					slotValidity=0;
				}
			}
			/* But, if you are the cudgel, ignore the pulserifle unless it has ammo. */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(newSlot)) {
						slotValidity=0;
					}
				}
			} else {
				/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
				int pulserifle_slot;
				int cudgel_slot;

				cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);

				if (cudgel_slot!=-1) {
					if (playerStatusPtr->WeaponSlot[cudgel_slot].Possessed!=1) {
						cudgel_slot=-1;
					}
				}

				if (cudgel_slot!=-1) {
					pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
					
					if (pulserifle_slot!=-1) {					
						if (!WeaponHasAmmo(pulserifle_slot)) {
							newSlot=cudgel_slot;
							slotValidity=1;		
						}
					}
				}
			}
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                slotValidity=0;
            }
		}
		while(slotValidity==0);

		playerStatusPtr->SwapToWeaponSlot = newSlot;
		return 1;
	}
    /* else if you wish to change to the next weapon */
    else if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_NextWeapon)
    {
		enum WEAPON_SLOT newSlot = currentSlot;
		int slotValidity=0;
	
        do
		{
	   		if(++newSlot == MAX_NO_OF_WEAPON_SLOTS) {
     			newSlot=WEAPON_SLOT_1;
			}
			if (playerStatusPtr->WeaponSlot[newSlot].Possessed==1) {
				slotValidity=1;
			}
			/* But not if you're a disk launcher with no disks. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[newSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[newSlot].PrimaryMagazinesRemaining==0) {
					slotValidity=0;
				}
			}
			/* And not if you're the cudgel. */
			if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->SelectedWeaponSlot!=newSlot) {
					slotValidity=0;
				}
			}
			/* But, if you are the cudgel, ignore the pulserifle unless it has ammo. */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(newSlot)) {
						slotValidity=0;
					}
				}
			} else {
				/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
				int pulserifle_slot;
				int cudgel_slot;

				cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);

				if (cudgel_slot!=-1) {
					if (playerStatusPtr->WeaponSlot[cudgel_slot].Possessed!=1) {
						cudgel_slot=-1;
					}
				}

				if (cudgel_slot!=-1) {
					pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
					
					if (pulserifle_slot!=-1) {					
						if (!WeaponHasAmmo(pulserifle_slot)) {
							newSlot=cudgel_slot;
							slotValidity=1;		
						}
					}
				}
			}
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[newSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                slotValidity=0;
            }
		}
		while(slotValidity==0);
        
	    playerStatusPtr->SwapToWeaponSlot = newSlot;
		return 1;
    }
	/* else pick a weapon, any weapon */
	else if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo)
    {
        enum WEAPON_SLOT requestedSlot = (enum WEAPON_SLOT)(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo - 1);

        LOCALASSERT(requestedSlot < MAX_NO_OF_WEAPON_SLOTS);
        LOCALASSERT(requestedSlot >= 0);
        
        if( (requestedSlot != currentSlot)
          &&(playerStatusPtr->WeaponSlot[requestedSlot].Possessed == 1) )
        { 
            // Disallow Gold version weapons with regular version
			if (!AllowGoldWeapons && ((playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_FRISBEE_LAUNCHER) || 
			   (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_MARINE_PISTOL) || 
			   (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_TWO_PISTOLS))) {
                return 0;
            }
			if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PRED_DISC) {
				if (playerStatusPtr->WeaponSlot[requestedSlot].PrimaryRoundsRemaining==0 
					&& playerStatusPtr->WeaponSlot[requestedSlot].PrimaryMagazinesRemaining==0) {
					#if 0
					sprintf(tempstring,"NO AMMO FOR %s\n",GetTextString(
						TemplateWeapon[playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber].Name)
					);			
					NewOnScreenMessage(tempstring);
					#else
					//NewOnScreenMessage(GetTextString(TEXTSTRING_NOAMMOFORWEAPON));
					#endif
					return 0;
				}
			}
			/* Look, I said you can't select the cudgel! */
			if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				return 0;
			}
	    	/* Of course, if you are the cudgel and the the pulserifle has no ammo... */
			if (playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot].WeaponIDNumber==WEAPON_CUDGEL) {
				if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					if (!WeaponHasAmmo(requestedSlot)) {
						return(0);
					}
				}
			} else {
				if (playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber==WEAPON_PULSERIFLE) {
					/* If you're not the cudgel... if it exists, and the PR has no ammo, select cudgel. */
					int pulserifle_slot;
					int cudgel_slot;

					cudgel_slot=SlotForThisWeapon(WEAPON_CUDGEL);
					if (cudgel_slot!=-1) {
						pulserifle_slot=SlotForThisWeapon(WEAPON_PULSERIFLE);
						
						if (pulserifle_slot!=-1) {					
							if (!WeaponHasAmmo(pulserifle_slot)) {
								requestedSlot=cudgel_slot;
							}
						}
					}
				}
			}

	    	playerStatusPtr->SwapToWeaponSlot = requestedSlot;
			return 1;
    	} else if( (requestedSlot != currentSlot)
          &&(playerStatusPtr->WeaponSlot[requestedSlot].Possessed == -1) )
        {
			#if 0
			sprintf(tempstring,"%s NOT AVAILABLE IN DEMO\n",GetTextString(
				TemplateWeapon[playerStatusPtr->WeaponSlot[requestedSlot].WeaponIDNumber].Name)
			);			
			NewOnScreenMessage(tempstring);
			#endif
		}
    }

   	return 0;
}

/*KJL********************************************************************************************
* Function to handle weapons whose firing rates are >= the frame rate, and therefore need to be *
* handled in a FRI manner.                                                                      *
********************************************************************************************KJL*/
int FireBurstWeapon(PLAYER_WEAPON_DATA *weaponPtr) {

	if (Weapon_ThisBurst==-1) {
		Weapon_ThisBurst=0;
	}

	Weapon_ThisBurst+=FireAutomaticWeapon(weaponPtr);

	return(1);
}

int FireAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
  	TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];
   	int oldAmmoCount;
    
    {
    	oldAmmoCount=weaponPtr->PrimaryRoundsRemaining>>16;
		/* ammo is in 16.16. we want the integer part, rounded up */
		if ( (weaponPtr->PrimaryRoundsRemaining&0xffff)!=0 ) oldAmmoCount+=1;
	}
		
	   	
   	{
   	   	/* theoretical number of bullets fired each frame, as a 16.16 number */
   	   	int bulletsToFire=MUL_FIXED(twPtr->FiringRate,NormalFrameTime);

    	if (bulletsToFire<weaponPtr->PrimaryRoundsRemaining)
    	{
    		weaponPtr->PrimaryRoundsRemaining -= bulletsToFire;	
       	}
        else /* end of magazine */
        {
           	weaponPtr->PrimaryRoundsRemaining=0;	
        }
    }

    {
    	int bulletsFired;
    	int newAmmoCount=weaponPtr->PrimaryRoundsRemaining>>16;
			/* ammo is in 16.16. we want the integer part, rounded up */
			if ( (weaponPtr->PrimaryRoundsRemaining&0xffff)!=0 ) newAmmoCount+=1;
        
        bulletsFired = oldAmmoCount-newAmmoCount;

	    /* Changed to create projectiles for the flamethrower */

	    if (templateAmmoPtr->CreatesProjectile)
	    {
	      if (bulletsFired)
	      {
	        ProjectilesFired=bulletsFired;
	        FireProjectileAmmo(twPtr->PrimaryAmmoID);
	      }
	    }
	    else /* instantaneous line of sight */
	    {
	      if (bulletsFired)
			  {
				  PlayerFireLineOfSightAmmo(twPtr->PrimaryAmmoID,bulletsFired);
				CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,bulletsFired);
			  }
	    }
		return(bulletsFired);
	}
}

int FireNonAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
   	TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];
  	weaponPtr->PrimaryRoundsRemaining -= 65536;

    /* does ammo create an actual object? */
    if (templateAmmoPtr->CreatesProjectile)
    {
		FireProjectileAmmo(twPtr->PrimaryAmmoID);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
    }
    else /* instantaneous line of sight */
    {
		PlayerFireLineOfSightAmmo(twPtr->PrimaryAmmoID,1);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
	}	
	return(1);	
}	

int FireNonAutomaticSecondaryAmmo(PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
   	TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->SecondaryAmmoID];
  	weaponPtr->SecondaryRoundsRemaining -= 65536;

    /* does ammo create an actual object? */
    if (templateAmmoPtr->CreatesProjectile)
    {
		FireProjectileAmmo(twPtr->SecondaryAmmoID);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
    }
    else /* instantaneous line of sight */
    {
		PlayerFireLineOfSightAmmo(twPtr->SecondaryAmmoID,1);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
	}	
	return(1);	
}	

 
#define WEAPON_RANGE 1000000  /* link this to weapon/ammo type perhaps */

/* instantaneous line of sight firing */
static void PlayerFireLineOfSightAmmo(enum AMMO_ID AmmoID, int multiple)
{

	if (PlayersTarget.DispPtr)
	{
		if (PlayersTarget.HModelSection!=NULL) {
			textprint("Hitting a hierarchical section.\n");
			GLOBALASSERT(PlayersTarget.DispPtr->HModelControlBlock==PlayersTarget.HModelSection->my_controller);
		}
		
		HandleWeaponImpact(&(PlayersTarget.Position),PlayersTarget.DispPtr->ObStrategyBlock,AmmoID,&GunMuzzleDirectionInWS, multiple*ONE_FIXED, PlayersTarget.HModelSection);

		/* Put in a target filter here? */
		if (AccuracyStats_TargetFilter(PlayersTarget.DispPtr->ObStrategyBlock)) {
			CurrentGameStats_WeaponHit(PlayerStatusPtr->SelectedWeaponSlot,multiple);
		}
	}
	 	 
}

void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data) 
{
	VECTORCH incoming,*invec;

	/* 'multiple' is now in FIXED POINT!  */

	#if 0
	if ((GRENADE_MODE)&&(positionPtr)) {
		switch (AmmoID) {
			case AMMO_10MM_CULW:
			case AMMO_SMARTGUN:
			case AMMO_MINIGUN:
				{		
					HandleEffectsOfExplosion
					(
						sbPtr,
						positionPtr,
						TemplateAmmo[AMMO_PULSE_GRENADE].MaxRange,
						&TemplateAmmo[AMMO_PULSE_GRENADE].MaxDamage[AvP.Difficulty],
						TemplateAmmo[AMMO_PULSE_GRENADE].ExplosionIsFlat
					);
					{
						Explosion_SoundData.position=*positionPtr;
					    Sound_Play(SID_NADEEXPLODE,"n",&Explosion_SoundData);
			    	}
				}
				break;
			default:
				break;
		}
	}
	#endif

	if(sbPtr)  
	{
		if (sbPtr->DynPtr) {
			MATRIXCH WtoLMat;
			/* Consider incoming hit direction. */
			WtoLMat=sbPtr->DynPtr->OrientMat;
			TransposeMatrixCH(&WtoLMat);
			RotateAndCopyVector(directionPtr,&incoming,&WtoLMat);
			invec=&incoming;
		} else {
			invec=NULL;
		}

		if (this_section_data)
		{
			if (sbPtr->SBdptr)
			{
				//if (sbPtr->SBdptr->HModelControlBlock && (sbPtr->I_SBtype != I_BehaviourNetGhost))
				if (sbPtr->SBdptr->HModelControlBlock)
				{
					/* Netghost case now handled properly. */
					AddDecalToHModel(&LOS_ObjectNormal, &LOS_Point,this_section_data);

					GLOBALASSERT(sbPtr->SBdptr->HModelControlBlock==this_section_data->my_controller);
					CauseDamageToHModel(sbPtr->SBdptr->HModelControlBlock, sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple, this_section_data,invec,positionPtr,0);
					/* No longer return: do knockback. */
					//return;
				} 
				else
				{
		  			CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple,invec);
				}
			}
			else
			{
		  		CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple,invec);
			}
		}
		else
		{
		  	CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple,invec);
		}

		if (sbPtr->DynPtr)
		{
			DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			EULER rotation;
			#if 0
			int magnitudeOfForce = (5000*TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty].Impact) / dynPtr->Mass;
			#else
			int magnitudeOfForce = (5000*TotalKineticDamage(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty])) / dynPtr->Mass;
			#endif
			/* okay, not too sure yet what this should do */
	  	  	dynPtr->LinImpulse.vx += MUL_FIXED(directionPtr->vx,magnitudeOfForce);
			dynPtr->LinImpulse.vy += MUL_FIXED(directionPtr->vy,magnitudeOfForce);
	  	  	dynPtr->LinImpulse.vz += MUL_FIXED(directionPtr->vz,magnitudeOfForce);

		  //CalculateTorque(&rotation, directionPtr, sbPtr);
			CalculateTorqueAtPoint(&rotation,positionPtr,sbPtr);
			/* okay, not too sure yet what this should do */
		    // magnitudeOfForce /= 100;
	  		
	  		dynPtr->AngImpulse.EulerX += MUL_FIXED(rotation.EulerX,magnitudeOfForce);
	  		dynPtr->AngImpulse.EulerY += MUL_FIXED(rotation.EulerY,magnitudeOfForce);
	  		dynPtr->AngImpulse.EulerZ += MUL_FIXED(rotation.EulerZ,magnitudeOfForce);

		}


	}

	/* KJL 10:44:49 02/03/98 - add bullet hole to world */
	if (LOS_ObjectHitPtr) {
		/* only add to the landscape, ie. modules */
		if (LOS_ObjectHitPtr->ObMyModule)
		{
			/* bad idea to put bullethole on a morphing object */
			if(!LOS_ObjectHitPtr->ObMorphCtrl)
			{
				MakeDecal
				(
					DECAL_BULLETHOLE,
					&LOS_ObjectNormal,
					positionPtr,
					LOS_ObjectHitPtr->ObMyModule->m_index
				);
			}
			/* cause a ricochet 1 in 8 times */
			if (((FastRandom()&65535) < 8192)&&(AmmoID!=AMMO_XENOBORG))
			{
				MakeImpactSparks(directionPtr, &LOS_ObjectNormal, positionPtr);
			}
		}
		
		/*also do ricochets on the queen*/
		if(sbPtr && sbPtr->I_SBtype==I_BehaviourQueenAlien)
		{
			/* cause a ricochet 1 in 8 times */
			if (((FastRandom()&65535) < 8192)&&(AmmoID!=AMMO_XENOBORG))
			{
				MakeImpactSparks(directionPtr, &LOS_ObjectNormal, positionPtr);
			}
		}
	}

		
	
	#if 0
	switch (AmmoID)
	{
		case AMMO_PLASMA:
		{
			DISPLAYBLOCK *dispPtr = MakeDebris(I_BehaviourPlasmaImpact,positionPtr);
			if (dispPtr) 
			{
 				if(AvP.Network!=I_No_Network) 
  					AddNetMsg_LocalRicochet(I_BehaviourPlasmaImpact,positionPtr,&LOS_ObjectNormal);

				MakeMatrixFromDirection(&LOS_ObjectNormal,&dispPtr->ObMat);
			}
			break;
		}
		case AMMO_PARTICLE_BEAM:
		{
			{
				VECTORCH velocity={0,0,0};
				MakeParticle(positionPtr,&(velocity),PARTICLE_BLACKSMOKE);
			}
			break;
		}
		default:
		{
			DISPLAYBLOCK *dispPtr = MakeDebris(I_BehaviourBulletRicochet,positionPtr);
			if (dispPtr) 
			{
 				if(AvP.Network!=I_No_Network) 
 					AddNetMsg_LocalRicochet(I_BehaviourBulletRicochet,positionPtr,&LOS_ObjectNormal);

				MakeMatrixFromDirection(&LOS_ObjectNormal,&dispPtr->ObMat);
			}
			break;
		}
	}
	#endif
	

}

int TotalKineticDamage(DAMAGE_PROFILE *damage) {
	
	int tkd;
	

	tkd=(damage->Impact+damage->Penetrative+damage->Cutting);


	return(tkd);
}

void GetDirectionOfAttack(STRATEGYBLOCK *sbPtr,VECTORCH *WorldVector,VECTORCH *Output) {

	MATRIXCH WtoLMat;

	GLOBALASSERT(sbPtr);
	
	if (sbPtr->DynPtr==NULL) {
		/* Handle as best we can. */
		Output->vx=0;
		Output->vy=0;
		Output->vz=0;
		return;
	}
	
	/* Consider incoming hit direction. */
	WtoLMat=sbPtr->DynPtr->OrientMat;
	TransposeMatrixCH(&WtoLMat);
	RotateAndCopyVector(WorldVector,Output,&WtoLMat);
	Normalise(Output);

}

void CauseDamageToObject(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming)
{
	int use_multiple;

	GLOBALASSERT(sbPtr);
	GLOBALASSERT(damage);

	use_multiple=multiple;

	if (sbPtr->I_SBtype==I_BehaviourNetGhost) {
		DamageNetworkGhost(sbPtr, damage, use_multiple,NULL,incoming);
		return;
	}
	
	if (sbPtr->I_SBtype==I_BehaviourDummy) {
		/* Dummys are INDESTRUCTIBLE. */
		return;
	}

	if (sbPtr->I_SBtype==I_BehaviourPredator) {
		if (damage->Id==AMMO_PRED_ENERGY_BOLT) {
			/* Make NPC Preds immune to splash damage? */
			return;
		}
	}

	if (damage->Id==AMMO_NPC_OBSTACLE_CLEAR) {
		if (sbPtr->I_SBtype==I_BehaviourInanimateObject) {
			INANIMATEOBJECT_STATUSBLOCK* objStatPtr = sbPtr->SBdataptr;
			
			if (objStatPtr->explosionType!=0) {
				/* Try not to blunder through fatal things... */
				return;
			}
		}
	}

	if(sbPtr->I_SBtype==I_BehaviourMarinePlayer ||
	   sbPtr->I_SBtype==I_BehaviourAlienPlayer ||
	   sbPtr->I_SBtype==I_BehaviourPredatorPlayer)
	{
		//check for player invulnerability
		PLAYER_STATUS *psPtr=(PLAYER_STATUS*)sbPtr->SBdataptr;
		GLOBALASSERT(psPtr);
		if(psPtr->invulnerabilityTimer>0)
		{
			//player is still invulnerable
			return;
		}

		#if 0
		/* And check for warpspeed cheatmode. */
		if (WARPSPEED_CHEATMODE) {
			use_multiple>>=1;
		}
		#endif
	}

	/* Friendly Fire? */
	if(AvP.Network != I_No_Network)
	{
		if (netGameData.disableFriendlyFire && (netGameData.gameType==NGT_CoopDeathmatch || netGameData.gameType==NGT_Coop)) {
			if (sbPtr->I_SBtype==I_BehaviourMarinePlayer) {
				if ((FriendlyFireDamageFilter(damage))==0) {
					return;
				}
			}
		}
	}
   			 
	DamageDamageBlock(&sbPtr->SBDamageBlock,damage,use_multiple);

	/* This bit will still need to exist: but differently. */

	/* Andy 13/10/97 -
		 Sound calls now placed in object specific damage functions ...IsDamaged */

 	switch(sbPtr->I_SBtype)
	{
		case I_BehaviourAlien:
		{
   	 	/* reduce alien health */
			AlienIsDamaged(sbPtr, damage, use_multiple, 0,NULL,incoming,NULL);
			break;
		}
		case I_BehaviourMarinePlayer:
		case I_BehaviourAlienPlayer:
		case I_BehaviourPredatorPlayer:
		{
			PlayerIsDamaged(sbPtr,damage,use_multiple,incoming);
			break;
		}
		case I_BehaviourInanimateObject:
		{
			InanimateObjectIsDamaged(sbPtr,damage,use_multiple);
			break;
		}
		case I_BehaviourVideoScreen:
		{
			VideoScreenIsDamaged(sbPtr,damage,use_multiple);
			break;
		}
		case I_BehaviourPlacedLight:
		{
			PlacedLightIsDamaged(sbPtr,damage,use_multiple);
			break;
		}
		case I_BehaviourTrackObject:
		{
			TrackObjectIsDamaged(sbPtr,damage,use_multiple);
			break;
		}
		case I_BehaviourPredator:
		{
			PredatorIsDamaged(sbPtr,damage,use_multiple,NULL,incoming);
			break;
		}
		case I_BehaviourXenoborg:
		{
			XenoborgIsDamaged(sbPtr, damage, use_multiple, 0,incoming);
			break;
		}
		case I_BehaviourSeal:
		case I_BehaviourMarine:
		{
			MarineIsDamaged(sbPtr,damage,use_multiple,0,NULL,incoming);
			break;
		}
		case I_BehaviourQueenAlien:
		{
			QueenIsDamaged(sbPtr,damage,use_multiple,NULL,incoming,NULL);
			break;
		}
		case I_BehaviourPredatorAlien:
		{
			GLOBALASSERT(0);
			//PAQIsDamaged(sbPtr,damage,use_multiple);
			break;
		}	    
		case I_BehaviourFaceHugger:
		{
			FacehuggerIsDamaged(sbPtr,damage,use_multiple);
			break;
		}	    
	  case I_BehaviourGrenade:
		case I_BehaviourFlareGrenade:
		case I_BehaviourFragmentationGrenade:
		case I_BehaviourProximityGrenade:
	  case I_BehaviourRocket:
	  case I_BehaviourPulseGrenade:
	  case I_BehaviourPredatorEnergyBolt:
	  case I_BehaviourXenoborgEnergyBolt:
		{
			/* make it explode! */
			((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = 0;
			break;
		}
		case I_BehaviourFrisbee:
		{
			/* make it explode! */
			((FRISBEE_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = 0;
			break;
		}
		case I_BehaviourNetGhost:
		{
			DamageNetworkGhost(sbPtr, damage, use_multiple,NULL,incoming);
			break;
		}
		case I_BehaviourAutoGun:
		{
			AGunIsDamaged(sbPtr, damage, use_multiple, 0,incoming);
			break;
		}
		case I_BehaviourNetCorpse:
		{
			CorpseIsDamaged(sbPtr,damage,use_multiple,0,NULL,incoming);
			break;
		}
		default:
			break;
	}
}


void HandleEffectsOfExplosion(STRATEGYBLOCK *objectToIgnorePtr, VECTORCH *centrePtr, int maxRange, DAMAGE_PROFILE *maxDamage, int flat)
{
	DISPLAYBLOCK *ignoreDispPtr;
	int i = NumActiveStBlocks;

	if (objectToIgnorePtr)
	{
		ignoreDispPtr = objectToIgnorePtr->SBdptr;
	}
	else
	{
		ignoreDispPtr = 0;
	}

	/* The damage done by an explosions varies linearly with range from the explosion's
	centre so that no damage is done to an object which is 'maxRange' away. */

	while(i)
	{
		STRATEGYBLOCK *sbPtr = ActiveStBlockList[--i];
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;

		if (sbPtr == objectToIgnorePtr) continue;
		  
		if (dynPtr && dispPtr)
		{
			int range;
			VECTORCH displacement;
			
			displacement.vx = dynPtr->Position.vx - centrePtr->vx;
			displacement.vy = dynPtr->Position.vy - centrePtr->vy;
			displacement.vz = dynPtr->Position.vz - centrePtr->vz;
			range = Approximate3dMagnitude(&displacement);
			
			/* find if object is in explosion radius */
			if (range && range < maxRange)
			{
				/* now check line of sight */
				BOOL visible=IsThisObjectVisibleFromThisPosition_WithIgnore(dispPtr,ignoreDispPtr,centrePtr,maxRange);
				if(LOS_Lambda>range)
				{
					//seen past the object , so it probably is visible , but has a hole in it
					visible=TRUE;
				}
				
				if(visible)
				{
					int mult,damage;
					enum PARTICLE_ID blood_type;

					if (flat) mult=ONE_FIXED;
					else mult=DIV_FIXED((maxRange-range),maxRange);

					damage=MUL_FIXED(TotalKineticDamage(maxDamage),mult);

					/* Identify blood type now! */
					blood_type=GetBloodType(sbPtr);
					{
						VECTORCH attack_dir;
						GetDirectionOfAttack(sbPtr,&displacement,&attack_dir);

						CauseDamageToObject(sbPtr,maxDamage,mult,&attack_dir);
				 	}
					if (NPC_IsDead(sbPtr)) {
						/* Blood! */
						VECTORCH position;

						GetTargetingPointOfObject_Far(sbPtr,&position);
						
						if ((damage)&&(blood_type!=PARTICLE_NULL)) {
							/* Let's just use ObMaxX for now... */
							MakeBloodExplosion(&position, dispPtr->ObRadius, centrePtr, damage, blood_type);
						}
					}
				 	/* effect of explosion on object's dynamics */
					{
						EULER rotation;
	 					int magnitudeOfForce = 5000*damage/dynPtr->Mass;
						
						Normalise(&displacement);

						/* okay, not too sure yet what this should do */
				  		dynPtr->LinImpulse.vx += MUL_FIXED(displacement.vx,magnitudeOfForce);
						dynPtr->LinImpulse.vy += MUL_FIXED(displacement.vy,magnitudeOfForce);
					  	dynPtr->LinImpulse.vz += MUL_FIXED(displacement.vz,magnitudeOfForce);
		
						CalculateTorque(&rotation, &displacement, sbPtr);
				
						/* okay, not too sure yet what this should do */
						 //	magnitudeOfForce /= 100;
				  		dynPtr->AngImpulse.EulerX += MUL_FIXED(rotation.EulerX,magnitudeOfForce);
				  		dynPtr->AngImpulse.EulerY += MUL_FIXED(rotation.EulerY,magnitudeOfForce);
				  		dynPtr->AngImpulse.EulerZ += MUL_FIXED(rotation.EulerZ,magnitudeOfForce);
					}
				}
			}
		}
   	}
	
	switch (maxDamage->ExplosivePower)
	{
		case 6:
		{
			
			MakeVolumetricExplosionAt(centrePtr,EXPLOSION_SMALL_NOCOLLISIONS);
		
			if(AvP.Network!=I_No_Network) 
			{
				AddNetMsg_MakeExplosion(centrePtr,EXPLOSION_SMALL_NOCOLLISIONS);
			}
			break;
		}
		case 5:
		{
			MakeVolumetricExplosionAt(centrePtr,EXPLOSION_MOLOTOV);
			break;
		}
		case 4:
		{
			/* Must be the plasmacaster.  SFX? */
			break;
		}
		case 3:
		{
			MakeElectricalExplosion(centrePtr);
			
			if(AvP.Network!=I_No_Network) 
			{
				AddNetMsg_MakeExplosion(centrePtr,EXPLOSION_PREDATORPISTOL);
			}
			break;
		}
		case 2:
		{
			
			MakeVolumetricExplosionAt(centrePtr,EXPLOSION_HUGE);
		
			if(AvP.Network!=I_No_Network) 
			{
				AddNetMsg_MakeExplosion(centrePtr,EXPLOSION_HUGE);
			}
			break;
		}
		case 1:
		{
			
			MakeVolumetricExplosionAt(centrePtr,EXPLOSION_PULSEGRENADE);
		
			if(AvP.Network!=I_No_Network) 
			{
				AddNetMsg_MakeExplosion(centrePtr,EXPLOSION_PULSEGRENADE);
			}
			break;
		}
		default:
			break;
	}

}


void FireAutoGun(STRATEGYBLOCK *sbPtr)
{
	#if 0
	AUTOGUN_BEHAV_BLOCK *ag_bhv;
	ag_bhv = (AUTOGUN_BEHAV_BLOCK*)(sbPtr->SBdataptr);

	FireLineOfSightAmmo(AMMO_AUTOGUN, &(ag_bhv->global_muzz_pos), &(ag_bhv->tgt_direction),1);
	#endif
}


void MakeMatrixFromDirection(VECTORCH *directionPtr, MATRIXCH *matrixPtr)
{
	VECTORCH XVector;
	VECTORCH YVector;
    
   	if (directionPtr->vx==0)
	{
		XVector.vx = 65536;
		XVector.vy = 0;
		XVector.vz = 0;
	}
	else if (directionPtr->vy==0)
	{
		XVector.vx = 0;
		XVector.vy = 65536;
		XVector.vz = 0;
	}
	else if (directionPtr->vz==0)
	{
		XVector.vx = 0;
		XVector.vy = 0;
		XVector.vz = 65536;
	}
	else
	{
		XVector.vx = directionPtr->vz;
		XVector.vy = 0;
		XVector.vz = -directionPtr->vx;
		Normalise(&XVector);
	}

	CrossProduct(directionPtr, &XVector, &YVector);
	
	matrixPtr->mat11 = XVector.vx;
	matrixPtr->mat12 = XVector.vy;
	matrixPtr->mat13 = XVector.vz;

	matrixPtr->mat21 = YVector.vx;
	matrixPtr->mat22 = YVector.vy;
	matrixPtr->mat23 = YVector.vz;

	matrixPtr->mat31 = directionPtr->vx;
	matrixPtr->mat32 = directionPtr->vy;
	matrixPtr->mat33 = directionPtr->vz;
}



/*KJL*************
* 3D Weapon Code *
*************KJL*/
/* values which are evaluated at runtime */
VECTORCH CentreOfMuzzleOffset;
int MuzzleFlashLength;

void PositionPlayersWeapon(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	VECTORCH gunDirection;
	//VECTORCH gunOffset = twPtr->RestPosition;
	VECTORCH gunOffset = PlayersWeaponCameraOffset;
	gunOffset.vx += twPtr->RestPosition.vx;
	gunOffset.vy += twPtr->RestPosition.vy;
	gunOffset.vz += twPtr->RestPosition.vz;

	gunOffset.vx += weaponPtr->PositionOffset.vx;
	gunOffset.vy += weaponPtr->PositionOffset.vy;
	gunOffset.vz += weaponPtr->PositionOffset.vz;

   	if ( (!(twPtr->PrimaryIsMeleeWeapon || (weaponPtr->WeaponIDNumber == WEAPON_PRED_DISC)
		||(weaponPtr->WeaponIDNumber == WEAPON_PRED_SHOULDERCANNON) 
		||(weaponPtr->WeaponIDNumber == WEAPON_PRED_MEDICOMP) 
		)) ) {
		{
			gunDirection = PlayersTarget.Position;
		
			{
				VECTORCH offset;
				offset.vx = gunOffset.vx/4;
				offset.vy = gunOffset.vy/4;
				offset.vz = gunOffset.vz/4;

				{
					MATRIXCH mat = Global_VDB_Ptr->VDB_Mat;
					TransposeMatrixCH(&mat);
					RotateVector(&offset, &mat);
		 		}
			 	gunDirection.vx -= Global_VDB_Ptr->VDB_World.vx-offset.vx;
			  	gunDirection.vy -= Global_VDB_Ptr->VDB_World.vy-offset.vy;
			  	gunDirection.vz -= Global_VDB_Ptr->VDB_World.vz-offset.vz;
			}
			Normalise(&gunDirection);
		}
		{
			MATRIXCH mat;
			EULER dir = weaponPtr->DirectionOffset;
			VECTORCH XVector;
			VECTORCH YVector;
		    VECTORCH ZVector;

			ZVector = gunDirection;
			#if 0
			/* KJL 13:13:34 04/05/97 - allow weapon to rotate about z axis */
			XVector.vx = ZVector.vz;
			XVector.vy = 0;
			XVector.vz = -ZVector.vx;

			Normalise(&XVector);

			CrossProduct(&ZVector,&XVector, &YVector);
			#else
			/* keep weapon stable about z axis - definitely needed for alien */
			YVector.vx = Global_VDB_Ptr->VDB_Mat.mat12;
			YVector.vy = Global_VDB_Ptr->VDB_Mat.mat22;
			YVector.vz = Global_VDB_Ptr->VDB_Mat.mat32;
			CrossProduct(&YVector,&ZVector, &XVector);
			Normalise(&XVector);
			#endif

			PlayersWeapon.ObMat.mat11 = XVector.vx;
			PlayersWeapon.ObMat.mat12 = XVector.vy;
			PlayersWeapon.ObMat.mat13 = XVector.vz;

			PlayersWeapon.ObMat.mat21 = YVector.vx;
			PlayersWeapon.ObMat.mat22 = YVector.vy;
			PlayersWeapon.ObMat.mat23 = YVector.vz;
			
			PlayersWeapon.ObMat.mat31 = ZVector.vx; 
			PlayersWeapon.ObMat.mat32 = ZVector.vy; 
			PlayersWeapon.ObMat.mat33 = ZVector.vz; 

			if (dir.EulerX != 0 || dir.EulerY !=0 || dir.EulerZ != 0)
		 	{
				if (dir.EulerX<0) dir.EulerX = 4096 + dir.EulerX;
				if (dir.EulerY<0) dir.EulerY = 4096 + dir.EulerY;
				if (dir.EulerZ<0) dir.EulerZ = 4096 + dir.EulerZ;

				CreateEulerMatrix(&dir, &mat);
				TransposeMatrixCH(&mat);
				MatrixMultiply(&PlayersWeapon.ObMat,&mat,&PlayersWeapon.ObMat);
	 			 			 
		 	}
		}
	} else {
		/* Pred Weapons. */
		PlayersWeapon.ObMat=Global_VDB_Ptr->VDB_Mat;
		TransposeMatrixCH(&PlayersWeapon.ObMat);
	}

	PlayersWeapon.ObView = gunOffset;

	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
	
		VECTORCH offset = PlayersWeapon.ObView;
		MATRIXCH matrix	= VDBPtr->VDB_Mat;

		TransposeMatrixCH(&matrix);
		 
	   	RotateVector(&offset, &matrix);
	 	 
		PlayersWeapon.ObWorld.vx = (VDBPtr->VDB_World.vx + offset.vx);
		PlayersWeapon.ObWorld.vy = (VDBPtr->VDB_World.vy + offset.vy);
		PlayersWeapon.ObWorld.vz = (VDBPtr->VDB_World.vz + offset.vz);
	}

	/* Late firing.  Note that this is a horrible hack. */
	if (FirePrimaryLate) {
		if ((*twPtr->FirePrimaryFunction)(weaponPtr)) {
			if (twPtr->PrimaryMuzzleFlash) {
				if (twPtr->PrimaryAmmoID==AMMO_PARTICLE_BEAM) {
					AddLightingEffectToObject(Player,LFX_PARTICLECANNON);
				} else {
					AddLightingEffectToObject(Player,LFX_MUZZLEFLASH);
				}
			}
		}
	} else if (FireSecondaryLate) {
		if (twPtr->SecondaryMuzzleFlash) {
			if ((*twPtr->FireSecondaryFunction)(weaponPtr)) {
				AddLightingEffectToObject(Player,LFX_MUZZLEFLASH);
			}
		}
	}
	/* It ignores all the things that are pipeline specific, like the return *
	 * condition of the fire function, and won't work for melee weapons. */

}

void PositionPlayersWeaponMuzzleFlash(void)
{
	int size = 65536*3/4 - (FastRandom()&16383);
	int length = size;

	if (PWMFSDP) {
//		textprint("Hierarchical Muzzle Flash.\n");
	
		PlayersWeaponMuzzleFlash.ObWorld=PWMFSDP->World_Offset;
		PlayersWeaponMuzzleFlash.ObMat=PWMFSDP->SecMat;
		return;

	}

	{

		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
		if (weaponPtr->WeaponIDNumber==WEAPON_SMARTGUN) length*=2;
	
	}

	{
		VECTORCH v = CentreOfMuzzleOffset;
 //		int disp = MUL_FIXED(MuzzleFlashLength,length);
		
		RotateVector(&v,&PlayersWeapon.ObMat);
	
		v.vx+=PlayersWeapon.ObWorld.vx;
		v.vy+=PlayersWeapon.ObWorld.vy;
		v.vz+=PlayersWeapon.ObWorld.vz;
		/* position the muzzle flash in world space */
		PlayersWeaponMuzzleFlash.ObWorld.vx = v.vx+MUL_FIXED(200,PlayersWeapon.ObMat.mat31);
		PlayersWeaponMuzzleFlash.ObWorld.vy = v.vy+MUL_FIXED(200,PlayersWeapon.ObMat.mat32);
		PlayersWeaponMuzzleFlash.ObWorld.vz = v.vz+MUL_FIXED(200,PlayersWeapon.ObMat.mat33);
 //		PlayersWeaponMuzzleFlash.ObWorld.vx = v.vx+MUL_FIXED(disp,PlayersWeapon.ObMat.mat31);
 //		PlayersWeaponMuzzleFlash.ObWorld.vy = v.vy+MUL_FIXED(disp,PlayersWeapon.ObMat.mat32);
 //		PlayersWeaponMuzzleFlash.ObWorld.vz = v.vz+MUL_FIXED(disp,PlayersWeapon.ObMat.mat33);
	}
	
	/* calculate it's view space position */
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;

		PlayersWeaponMuzzleFlash.ObView.vx = PlayersWeaponMuzzleFlash.ObWorld.vx - VDBPtr->VDB_World.vx;
		PlayersWeaponMuzzleFlash.ObView.vy = PlayersWeaponMuzzleFlash.ObWorld.vy - VDBPtr->VDB_World.vy;
		PlayersWeaponMuzzleFlash.ObView.vz = PlayersWeaponMuzzleFlash.ObWorld.vz - VDBPtr->VDB_World.vz;
		RotateVector(&PlayersWeaponMuzzleFlash.ObView, &matrix);
	}
	
	/* align the muzzle flash with the gun */
	PlayersWeaponMuzzleFlash.ObMat.mat11 = MUL_FIXED(size,PlayersWeapon.ObMat.mat11);
	PlayersWeaponMuzzleFlash.ObMat.mat12 = MUL_FIXED(size,PlayersWeapon.ObMat.mat12);
	PlayersWeaponMuzzleFlash.ObMat.mat13 = MUL_FIXED(size,PlayersWeapon.ObMat.mat13);

	PlayersWeaponMuzzleFlash.ObMat.mat21 = MUL_FIXED(size,PlayersWeapon.ObMat.mat21);
	PlayersWeaponMuzzleFlash.ObMat.mat22 = MUL_FIXED(size,PlayersWeapon.ObMat.mat22);
	PlayersWeaponMuzzleFlash.ObMat.mat23 = MUL_FIXED(size,PlayersWeapon.ObMat.mat23);

	PlayersWeaponMuzzleFlash.ObMat.mat31 = MUL_FIXED(length,PlayersWeapon.ObMat.mat31);
	PlayersWeaponMuzzleFlash.ObMat.mat32 = MUL_FIXED(length,PlayersWeapon.ObMat.mat32);
	PlayersWeaponMuzzleFlash.ObMat.mat33 = MUL_FIXED(length,PlayersWeapon.ObMat.mat33);
	
	/* rotate flash around in random multiples of 60 degrees */
	{
		MATRIXCH mat;
   		int angle = (FastRandom()%6)*683;
 	  	int cos = GetCos(angle);
 	  	int sin = GetSin(angle);
 	  	mat.mat11 = cos;		 
 	  	mat.mat12 = sin;
 	  	mat.mat13 = 0;
 	  	mat.mat21 = -sin;	  	
 	  	mat.mat22 = cos;	  	
 	  	mat.mat23 = 0;	  	
 	  	mat.mat31 = 0;	  	
 	  	mat.mat32 = 0;	  	
 	  	mat.mat33 = 65536;	  	
		 	MatrixMultiply(&PlayersWeaponMuzzleFlash.ObMat,&mat,&PlayersWeaponMuzzleFlash.ObMat);
	}

}	



static void StateDependentMovement(PLAYER_STATUS *playerStatusPtr, PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (twPtr->WeaponStateFunction[weaponPtr->CurrentState]!=NULL) {
		(*twPtr->WeaponStateFunction[weaponPtr->CurrentState])((void *)playerStatusPtr,weaponPtr);
	}

 	if (twPtr->UseStateMovement==0) {
		
		/* Judder is now in targeting.c! */
		return;
	}

 	/* Handle on-screen movement of gun */
    switch(weaponPtr->CurrentState)
	{
		case WEAPONSTATE_SWAPPING_IN:
        case WEAPONSTATE_SWAPPING_OUT:
		{
			//VECTORCH gunOffset = twPtr->RestPosition;
        	VECTORCH gunOffset = PlayersWeaponCameraOffset;
        	int offset;

        	/* scroll old weapon down screen & new weapon up */
        	if (playerStatusPtr->SwapToWeaponSlot != WEAPON_FINISHED_SWAPPING)
            {
				offset = 65536-weaponPtr->StateTimeOutCounter;	
           	}
            else
            {
				offset = weaponPtr->StateTimeOutCounter;
            }
          	
          	/* slide a bit to the right when swapping */            
			weaponPtr->DirectionOffset.EulerX =(offset>>7);
			weaponPtr->PositionOffset.vx = MUL_FIXED(offset,gunOffset.vx);
            weaponPtr->PositionOffset.vy = MUL_FIXED(offset,gunOffset.vy);
			weaponPtr->PositionOffset.vz = MUL_FIXED(-offset,gunOffset.vz);
			
			break;

		}
        case WEAPONSTATE_FIRING_PRIMARY:
        {
	        if (twPtr->PrimaryIsRapidFire)
        	{
	        	/* jiggle the weapon around when you shoot */
			   	weaponPtr->PositionOffset.vz = (FastRandom()&twPtr->RecoilMaxRandomZ) - twPtr->RecoilMaxZ;
  	 			weaponPtr->DirectionOffset.EulerX =	(FastRandom()&twPtr->RecoilMaxXTilt)-twPtr->RecoilMaxXTilt/2;
  	 			weaponPtr->DirectionOffset.EulerY =	(FastRandom()&twPtr->RecoilMaxYTilt)-twPtr->RecoilMaxYTilt/2;
				
				HeadOrientation.EulerX = ((FastRandom()&31)-16)&4095;
			}
			else
			{
				HeadOrientation.EulerX = 4095-32;
			}
			break;	
        }
        case WEAPONSTATE_RECOIL_PRIMARY:
        {
			int offset = 65536-weaponPtr->StateTimeOutCounter;
		
		   	weaponPtr->PositionOffset.vz = -MUL_FIXED(offset,twPtr->RecoilMaxZ);
 			weaponPtr->DirectionOffset.EulerX =-MUL_FIXED(offset,twPtr->RecoilMaxXTilt);
            break;
        }
        case WEAPONSTATE_RECOIL_SECONDARY:
        {
			int offset = 65536-weaponPtr->StateTimeOutCounter;
			
		   	weaponPtr->PositionOffset.vz = -MUL_FIXED(offset,twPtr->RecoilMaxZ);
 			weaponPtr->DirectionOffset.EulerX =-MUL_FIXED(offset,twPtr->RecoilMaxXTilt);
            break;
        }
        
        
		default:
        {
			/* recentre offsets */		
			int linearCenteringSpeed = MUL_FIXED(300,NormalFrameTime);				  
			int rotationalCenteringSpeed = MUL_FIXED(1500,NormalFrameTime);				  
    		if (weaponPtr->PositionOffset.vx > 0 )
			{
				weaponPtr->PositionOffset.vx -= linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vx < 0) weaponPtr->PositionOffset.vx = 0;	
			}
			else if (weaponPtr->PositionOffset.vx < 0 )
			{
				weaponPtr->PositionOffset.vx += linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vx > 0) weaponPtr->PositionOffset.vx = 0;	
			}
			
			if (weaponPtr->PositionOffset.vy > 0 )
			{
				weaponPtr->PositionOffset.vy -= linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vy < 0) weaponPtr->PositionOffset.vy = 0;	
			}
			else if (weaponPtr->PositionOffset.vy < 0 )
			{
				weaponPtr->PositionOffset.vy += linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vy > 0) weaponPtr->PositionOffset.vy = 0;	
			}
			
			if (weaponPtr->PositionOffset.vz > 0 )
			{
				weaponPtr->PositionOffset.vz -= linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vz < 0) weaponPtr->PositionOffset.vz = 0;	
			}
			else if (weaponPtr->PositionOffset.vz < 0 )
			{
				weaponPtr->PositionOffset.vz += linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vz > 0) weaponPtr->PositionOffset.vz = 0;	
			}

			if (weaponPtr->DirectionOffset.EulerX > 0 )
			{
				weaponPtr->DirectionOffset.EulerX -= rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerX < 0) weaponPtr->DirectionOffset.EulerX = 0;	
			}
			else if (weaponPtr->DirectionOffset.EulerX < 0 )
			{
				weaponPtr->DirectionOffset.EulerX += rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerX > 0) weaponPtr->DirectionOffset.EulerX = 0;	
			}
			
			if (weaponPtr->DirectionOffset.EulerY > 0 )
			{
				weaponPtr->DirectionOffset.EulerY -= rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerY < 0) weaponPtr->DirectionOffset.EulerY = 0;	
			}
			else if (weaponPtr->DirectionOffset.EulerY < 0 )
			{
				weaponPtr->DirectionOffset.EulerY += rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerY > 0) weaponPtr->DirectionOffset.EulerY = 0;	
			}

			if (weaponPtr->DirectionOffset.EulerZ > 0 )
			{
				weaponPtr->DirectionOffset.EulerZ -= rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerZ < 0) weaponPtr->DirectionOffset.EulerZ = 0;	
			}
			else if (weaponPtr->DirectionOffset.EulerZ < 0 )
			{
				weaponPtr->DirectionOffset.EulerZ += rotationalCenteringSpeed;
				if (weaponPtr->DirectionOffset.EulerZ > 0) weaponPtr->DirectionOffset.EulerZ = 0;	
			}
        	
        	break;
        }
     
        
	}

}



extern void UpdateWeaponShape(void)
{
	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
    
    /* access the extra data hanging off the strategy block */
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    	
	/* player's current weapon */
    GLOBALASSERT(playerStatusPtr->SelectedWeaponSlot<MAX_NO_OF_WEAPON_SLOTS);
        
    /* init a pointer to the weapon's data */
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

	/* Player doesn't have a weapon! This will eventually be changed into an assertion 
	that the player *does* have a weapon */
	if (weaponPtr->WeaponIDNumber == NULL_WEAPON)
		return;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	GrabWeaponShape(weaponPtr);

	if (!(twPtr->PrimaryIsMeleeWeapon)) GrabMuzzleFlashShape(twPtr);

	PlayersWeapon.ObTxAnimCtrlBlks=weaponPtr->TxAnimCtrl;
	if (twPtr->HasShapeAnimation) {
		PlayersWeapon.ShapeAnimControlBlock=&weaponPtr->ShpAnimCtrl;
	} else {
		PlayersWeapon.ShapeAnimControlBlock=NULL;
	}

}

void GetHierarchicalWeapon(char *riffname, char *hierarchyname, int sequence_type, int sub_sequence) {

	extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
	SECTION *root_section;
	SECTION_DATA *camera_section;

	root_section=GetNamedHierarchyFromLibrary(riffname,hierarchyname);

	GLOBALASSERT(root_section);

	Dispel_HModel(&PlayersWeaponHModelController);
	Create_HModel(&PlayersWeaponHModelController,root_section);
	InitHModelSequence(&PlayersWeaponHModelController,sequence_type,sub_sequence,ONE_FIXED); // Was >>3
	
	/* Causes that 'one frame' flicker? */
	//ProveHModel(&PlayersWeaponHModelController,&PlayersWeapon);
	
	PlayersWeapon.HModelControlBlock=&PlayersWeaponHModelController;
	PlayersWeapon.HModelControlBlock->Playing=1;
	PlayersWeapon.HModelControlBlock->Looped=1;

	PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"dum flash");
	if (PWMFSDP==NULL) {
		PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum flash");
		/* ?&$(*"*&^ pred pistol!!! */
		if (PWMFSDP==NULL) {
			PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum Flash");
		}
	}
	/* Could be NULL though, I don't care at this stage. */
	
	camera_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"Camera Root");

	if (camera_section) {
		//PlayersWeaponCameraOffset=camera_section->sempai->sequence_array->first_frame->Offset;
		GetKeyFrameOffset(camera_section->sempai->sequence_array->first_frame,&PlayersWeaponCameraOffset);
		PlayersWeaponCameraOffset.vx=-PlayersWeaponCameraOffset.vx;
		PlayersWeaponCameraOffset.vy=-PlayersWeaponCameraOffset.vy;
		PlayersWeaponCameraOffset.vz=-PlayersWeaponCameraOffset.vz;
	} else {
		GLOBALASSERT(0);
		/* If you really want, you could do something like... *
		PlayersWeaponCameraOffset=twPtr->RestPosition;
		 * But that would cause a compiler error.             */
	}


}

void GrabWeaponShape(PLAYER_WEAPON_DATA *weaponPtr)
{
	
	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	/* if there is no shape name then return */
	if (twPtr->WeaponShapeName == NULL) return;

	PlayersWeapon.ObShape = GetLoadedShapeMSL(twPtr->WeaponShapeName);
	LOCALASSERT(PlayersWeapon.ObShape>0);

   	PlayersWeapon.ObFlags  = ObFlag_VertexHazing|ObFlag_MultLSrc;
	PlayersWeapon.ObFlags2 = 0;
	PlayersWeapon.ObFlags3 = 0;

  	PlayersWeapon.ObLightType = LightType_PerVertex;

	PlayersWeapon.HModelControlBlock=NULL;

	//WeaponSetStartFrame(NULL,NULL);
	FindEndOfShape(&CentreOfMuzzleOffset,PlayersWeapon.ObShape);

	if (twPtr->HierarchyName) {

		/* Bit of a cheap test, I know. */
		GetHierarchicalWeapon(twPtr->RiffName,twPtr->HierarchyName,twPtr->InitialSequenceType,twPtr->InitialSubSequence);

	} else {
		Dispel_HModel(&PlayersWeaponHModelController);
		PWMFSDP=NULL;
		PlayersWeaponCameraOffset=twPtr->RestPosition;
	}

	if (twPtr->WeaponInitFunction!=NULL) {
		(*twPtr->WeaponInitFunction)(weaponPtr);
	}

}

void GrabMuzzleFlashShape(TEMPLATE_WEAPON_DATA *twPtr)
{
	#if 0
	/* if there is no shape name then return */
	if (twPtr->MuzzleFlashShapeName == NULL) return;

	/* otherwise setup displayblock */
	PlayersWeaponMuzzleFlash.ObShape = GetLoadedShapeMSL(twPtr->MuzzleFlashShapeName);
	LOCALASSERT(PlayersWeaponMuzzleFlash.ObShape>0);

   	PlayersWeaponMuzzleFlash.ObFlags  = ObFlag_NoInfLSrc|ObFlag_MultLSrc;
	PlayersWeaponMuzzleFlash.ObFlags2 = 0;
	PlayersWeaponMuzzleFlash.ObFlags3 = ObFlag3_NoLightDot;

  	PlayersWeaponMuzzleFlash.ObLightType = LightType_PerVertex;

  	{
  		SHAPEHEADER *shapePtr = GetShapeData(PlayersWeaponMuzzleFlash.ObShape);
	   	MuzzleFlashLength = -shapePtr->shapeminz;
	}
	#endif
}


void FindEndOfShape(VECTORCH* endPositionPtr, int shapeIndex)
{
	extern int SetupPolygonAccessFromShapeIndex(int shapeIndex);
	int max_z = 0;
   	int pointsFound = 0;
	int numberOfItems;
	
	{
		SHAPEHEADER *shapePtr = GetShapeData(shapeIndex);
		LOCALASSERT(shapePtr);
		max_z = shapePtr->shapemaxz;
	}
	endPositionPtr->vx = 0;
	endPositionPtr->vy = 0;
	endPositionPtr->vz = max_z;

	numberOfItems = SetupPolygonAccessFromShapeIndex(shapeIndex);
	
	/* go through polys and get the average position of all the
	points whose z value is near the max z of the shape */
	while(numberOfItems)
	{
		AccessNextPolygon();
		{
			struct ColPolyTag polyData;
	        GetPolygonVertices(&polyData);
	        
		   	{
		 		VECTORCH *vertexPtr = &polyData.PolyPoint[0];
			 	int noOfVertices = polyData.NumberOfVertices;
				do            
		 		{
					int z = vertexPtr->vz;

					if (z >= max_z-5)
					{
						endPositionPtr->vx += vertexPtr->vx;
						endPositionPtr->vy += vertexPtr->vy;
						pointsFound++;
					}

					vertexPtr++;
		 			noOfVertices--;
		 		}
				while(noOfVertices);
		 	}
		}
		numberOfItems--;
	}
	LOCALASSERT(pointsFound!=0);
	endPositionPtr->vx /= pointsFound;
	endPositionPtr->vy /= pointsFound;

    return;
}


#if 0
static void FireLineOfSightAmmo(enum AMMO_ID AmmoID, VECTORCH* sourcePtr, VECTORCH* directionPtr, int multiple)
{
	#if 0
	LOS_Lambda = 10000000;
	LOS_ObjectHitPtr = 0;
	LOS_HModel_Section=NULL;
	{
	   	extern int NumActiveBlocks;
		extern DISPLAYBLOCK* ActiveBlockList[];
	   	int numberOfObjects = NumActiveBlocks;
		
	   	while (numberOfObjects--)
		{
			DISPLAYBLOCK* objectPtr = ActiveBlockList[numberOfObjects];
			VECTORCH alpha = *sourcePtr;
			VECTORCH beta = *directionPtr;
			GLOBALASSERT(objectPtr);

			CheckForVectorIntersectionWith3dObject(objectPtr, &alpha, &beta,1);
		}
	}
  	#else
	FindPolygonInLineOfSight(directionPtr,sourcePtr,0,NULL);
	#endif

	if (LOS_ObjectHitPtr)
	{
		/* this fn needs updating to take amount of damage into account etc. */
		if (LOS_ObjectHitPtr->ObStrategyBlock) {
			if ((LOS_ObjectHitPtr->ObStrategyBlock->SBdptr)&&(LOS_HModel_Section)) {
				GLOBALASSERT(LOS_ObjectHitPtr->ObStrategyBlock->SBdptr->HModelControlBlock==LOS_HModel_Section->my_controller);
			}
		}

		HandleWeaponImpact(&LOS_Point,LOS_ObjectHitPtr->ObStrategyBlock,AmmoID,directionPtr, multiple*ONE_FIXED, LOS_HModel_Section);
	}
}
#endif


static void CalculateTorque(EULER *rotationPtr, VECTORCH *directionPtr, STRATEGYBLOCK *sbPtr)
{
	VECTORCH point,absPoint;
	int intersectionPlane;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	{
		MATRIXCH mat=dynPtr->OrientMat;
		TransposeMatrixCH(&mat);
		
		point.vx = directionPtr->vx - dynPtr->Position.vx;
		point.vy = directionPtr->vy - dynPtr->Position.vy;
		point.vz = directionPtr->vz - dynPtr->Position.vz;
		
		RotateVector(&point,&mat);
	}

	absPoint=point;
	if (absPoint.vx<0) absPoint.vx = -absPoint.vx;
	if (absPoint.vy<0) absPoint.vy = -absPoint.vy;
	if (absPoint.vz<0) absPoint.vz = -absPoint.vz;

	/* KJL 16:59:03 03/06/97 - paranoia check */
	LOCALASSERT(absPoint.vx>=0);
	LOCALASSERT(absPoint.vy>=0);
	LOCALASSERT(absPoint.vz>=0);
	
	if (absPoint.vx > absPoint.vz)
	{
		if (absPoint.vx > absPoint.vy)
		{
			intersectionPlane = ix;
		}
		else
		{
			intersectionPlane = iy;
		}
	}
	else
	{
		if (absPoint.vz > absPoint.vy)
		{
			intersectionPlane = iz;
		}
		else
		{
			intersectionPlane = iy;
		}
	}

	switch (intersectionPlane)
	{
		case ix:
		{
			
			rotationPtr->EulerX = 0;
			rotationPtr->EulerY = -DIV_FIXED(point.vz,point.vx);
			rotationPtr->EulerZ = DIV_FIXED(point.vy,point.vx);
			
			break;
		}	
		case iy:
		{
			rotationPtr->EulerX = DIV_FIXED(point.vz,point.vy);
			rotationPtr->EulerY = 0;
			rotationPtr->EulerZ = -DIV_FIXED(point.vx,point.vy);

			break;
		}	
		case iz:
		{
			rotationPtr->EulerX = -DIV_FIXED(point.vy,point.vz);
			rotationPtr->EulerY = DIV_FIXED(point.vx,point.vz);
			rotationPtr->EulerZ = 0;

			break;
		}	
		default:
			break;
	}

}


static void CalculateTorqueAtPoint(EULER *rotationPtr, VECTORCH *pointPtr, STRATEGYBLOCK *sbPtr)
{
	#if 0
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
	VECTORCH point;
	VECTORCH rotationDirection;
	int rotationMagnitude;

	point.vx = pointPtr->vx - dynPtr->Position.vx;
	point.vy = pointPtr->vy - dynPtr->Position.vy;
	point.vz = pointPtr->vz - dynPtr->Position.vz;
	
	CrossProduct(pointPtr, directionPtr, &rotationDirection);

	rotationMagnitude = Magnitude(&rotationDirection);
	
	/* normalise direction */
	rotationDirection.vx = DIV_FIXED(rotationDirection.vx,rotationMagnitude);
	rotationDirection.vy = DIV_FIXED(rotationDirection.vy,rotationMagnitude);
	rotationDirection.vz = DIV_FIXED(rotationDirection.vz,rotationMagnitude);
	
	{
	//	MATRIXCH matrix,mat;
	//	MakeMatrixFromDirection(directionPtr, &matrix);
	
	//	MatrixMultiply(&mat,&matrix,&mat);
	//	MatrixToEuler(&mat,rotationPtr);
	}
	
	#else
	VECTORCH normal,point;
	int intersectionPlane;
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	/* if the strategy block doesn't have a dynamics or display block, skip it */
	if (!(dynPtr) || !(sbPtr->SBdptr)) return;
	{
		MATRIXCH mat=dynPtr->OrientMat;
		TransposeMatrixCH(&mat);
		
		point.vx = pointPtr->vx - dynPtr->Position.vx;
		point.vy = pointPtr->vy - dynPtr->Position.vy;
		point.vz = pointPtr->vz - dynPtr->Position.vz;
		
		RotateVector(&point,&mat);
	
 //		normal = *normalPtr;
 //		RotateVector(&normal,&mat);
		normal = point;
	}
	
	{							  
		VECTORCH absNormal = normal;
		if (absNormal.vx<0) absNormal.vx=-absNormal.vx;
		if (absNormal.vy<0) absNormal.vy=-absNormal.vy;
		if (absNormal.vz<0) absNormal.vz=-absNormal.vz;

		if (absNormal.vx > absNormal.vy)
		{
			if (absNormal.vx > absNormal.vz)
			{
				intersectionPlane = ix;
			}
			else
			{
				intersectionPlane = iz;
	 		}
		}
		else
		{
			if (absNormal.vy > absNormal.vz)
			{
				intersectionPlane = iy;
			}
			else
			{
				intersectionPlane = iz;
			}
		}
	}
	


	switch (intersectionPlane)
	{
		case ix:
		{
			
			rotationPtr->EulerX = 0;
			if (point.vx>0)
			{
				rotationPtr->EulerY = -point.vz;
	   			rotationPtr->EulerZ = point.vy;
			}
			else
			{
		 		rotationPtr->EulerY = point.vz;
		   		rotationPtr->EulerZ = -point.vy;
			}
			break;
		}	
		case iy:
		{
			rotationPtr->EulerY = 0;
			
			if (point.vy>0)
			{
	   			rotationPtr->EulerX = point.vz;
		 		rotationPtr->EulerZ = -point.vx;
			}
			else
			{
			 	rotationPtr->EulerX = -point.vz;
		   		rotationPtr->EulerZ = point.vx;
			}
			break;
		}	
		case iz:
		{
			rotationPtr->EulerZ = 0;
			if (point.vz>0)
			{
		  		rotationPtr->EulerX = -point.vy;
				rotationPtr->EulerY = point.vx;
			}
			else
			{
				rotationPtr->EulerX = point.vy;
				rotationPtr->EulerY = -point.vx;
			}
			break;
		}	
		default:
			break;
	}
	{
		SHAPEHEADER	*shapePtr = GetShapeData(sbPtr->SBdptr->ObShape);
		rotationPtr->EulerX=DIV_FIXED(rotationPtr->EulerX, shapePtr->shaperadius);
		rotationPtr->EulerY=DIV_FIXED(rotationPtr->EulerY, shapePtr->shaperadius);
		rotationPtr->EulerZ=DIV_FIXED(rotationPtr->EulerZ, shapePtr->shaperadius);
	}
	#if 0
	{
		MATRIXCH mat;
		CreateEulerMatrix(rotationPtr, &mat);
   		TransposeMatrixCH(&mat);

		MatrixMultiply(&mat,&(dynPtr->OrientMat),&mat);
		MatrixToEuler(&mat,rotationPtr);
	  	
	}
	#else
	{
//		RotateVector((VECTORCH *)(rotationPtr),&(dynPtr->OrientMat));
 	}
	#endif
	#endif
}



void AlienTailTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
//    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	VECTORCH startDir = {0,-65536,0};
	VECTORCH endDir = {37837,37837,37837};
	VECTORCH direction;
	int timeLeft = (weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)? weaponPtr->StateTimeOutCounter:65536-weaponPtr->StateTimeOutCounter;
	int timeDone = 65536-timeLeft;

	weaponPtr->PositionOffset.vx = MUL_FIXED(timeDone,300);
	weaponPtr->PositionOffset.vy = MUL_FIXED(timeDone,0);
	weaponPtr->PositionOffset.vz = MUL_FIXED(timeDone,500);
	
	direction.vx = WideMul2NarrowDiv(startDir.vx, timeLeft, endDir.vx, timeDone, ONE_FIXED);
	direction.vy = WideMul2NarrowDiv(startDir.vy, timeLeft, endDir.vy, timeDone, ONE_FIXED);
	direction.vz = WideMul2NarrowDiv(startDir.vz, timeLeft, endDir.vz, timeDone, ONE_FIXED);
	Normalise(&direction);	
	
	MakeMatrixFromDirection(&direction,&(PlayersWeapon.ObMat));
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;
		 
		TransposeMatrixCH(&matrix);
		MatrixMultiply(&matrix,&PlayersWeapon.ObMat,&PlayersWeapon.ObMat);
	}
}

void MeleeWeaponNullTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
	weaponPtr->PositionOffset.vz = -100000;
}

void AlienClawTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
//    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	VECTORCH startDir = {-65536,0,0};
	VECTORCH endDir = {-2,0,65535};
	VECTORCH direction;
	int timeLeft = weaponPtr->StateTimeOutCounter;
	int timeDone = 65536-timeLeft;
	
	weaponPtr->PositionOffset.vx = MUL_FIXED(timeDone,600);
	weaponPtr->PositionOffset.vy = MUL_FIXED(timeDone,-200);
	weaponPtr->PositionOffset.vz = MUL_FIXED(timeDone,400);
	
	direction.vx = WideMul2NarrowDiv(startDir.vx, timeLeft, endDir.vx, timeDone, ONE_FIXED);
	direction.vy = WideMul2NarrowDiv(startDir.vy, timeLeft, endDir.vy, timeDone, ONE_FIXED);
	direction.vz = WideMul2NarrowDiv(startDir.vz, timeLeft, endDir.vz, timeDone, ONE_FIXED);
	Normalise(&direction);	
//	direction = endDir;
	
	MakeMatrixFromDirection(&direction,&(PlayersWeapon.ObMat));
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;
		 
		TransposeMatrixCH(&matrix);
		MatrixMultiply(&matrix,&PlayersWeapon.ObMat,&PlayersWeapon.ObMat);
	}
	PlayersWeapon.ObMat.mat21 = -PlayersWeapon.ObMat.mat21;
	PlayersWeapon.ObMat.mat22 = -PlayersWeapon.ObMat.mat22;
	PlayersWeapon.ObMat.mat23 = -PlayersWeapon.ObMat.mat23;
	PlayersWeapon.ObMat.mat11 = -PlayersWeapon.ObMat.mat11;
	PlayersWeapon.ObMat.mat12 = -PlayersWeapon.ObMat.mat12;
	PlayersWeapon.ObMat.mat13 = -PlayersWeapon.ObMat.mat13;
}
void AlienClawEndTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
//    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	VECTORCH startDir = {-2,0,65535};
	VECTORCH endDir = {-2,0,65535};
	VECTORCH direction;
	int timeLeft = weaponPtr->StateTimeOutCounter;
	int timeDone = 65536-timeLeft;

	weaponPtr->PositionOffset.vx = 600+MUL_FIXED(timeDone,200);
	weaponPtr->PositionOffset.vy = -200+MUL_FIXED(timeDone,300);
	weaponPtr->PositionOffset.vz = 400+MUL_FIXED(timeDone,-400);
	
	direction.vx = WideMul2NarrowDiv(startDir.vx, timeLeft, endDir.vx, timeDone, ONE_FIXED);
	direction.vy = WideMul2NarrowDiv(startDir.vy, timeLeft, endDir.vy, timeDone, ONE_FIXED);
	direction.vz = WideMul2NarrowDiv(startDir.vz, timeLeft, endDir.vz, timeDone, ONE_FIXED);
	Normalise(&direction);	

	MakeMatrixFromDirection(&direction,&(PlayersWeapon.ObMat));
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;
		 
		TransposeMatrixCH(&matrix);
		MatrixMultiply(&matrix,&PlayersWeapon.ObMat,&PlayersWeapon.ObMat);
	}
	PlayersWeapon.ObMat.mat21 = -PlayersWeapon.ObMat.mat21;
	PlayersWeapon.ObMat.mat22 = -PlayersWeapon.ObMat.mat22;
	PlayersWeapon.ObMat.mat23 = -PlayersWeapon.ObMat.mat23;
	PlayersWeapon.ObMat.mat11 = -PlayersWeapon.ObMat.mat11;
	PlayersWeapon.ObMat.mat12 = -PlayersWeapon.ObMat.mat12;
	PlayersWeapon.ObMat.mat13 = -PlayersWeapon.ObMat.mat13;
}

void PredWristbladeTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
//    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	VECTORCH startDir = {0,0,65536};
	VECTORCH endDir = {-37837,-37837,37837};
	VECTORCH direction;
	int timeLeft = ((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY))? weaponPtr->StateTimeOutCounter:65536-weaponPtr->StateTimeOutCounter;
	int timeDone = 65536-timeLeft;

	weaponPtr->PositionOffset.vx = MUL_FIXED(timeDone,-200);
	weaponPtr->PositionOffset.vy = MUL_FIXED(timeDone,-50);
	weaponPtr->PositionOffset.vz = MUL_FIXED(timeDone,900);
	
	direction.vx = WideMul2NarrowDiv(startDir.vx, timeLeft, endDir.vx, timeDone, ONE_FIXED);
	direction.vy = WideMul2NarrowDiv(startDir.vy, timeLeft, endDir.vy, timeDone, ONE_FIXED);
	direction.vz = WideMul2NarrowDiv(startDir.vz, timeLeft, endDir.vz, timeDone, ONE_FIXED);
	Normalise(&direction);	
	
	MakeMatrixFromDirection(&direction,&(PlayersWeapon.ObMat));
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;
		 
		TransposeMatrixCH(&matrix);
		MatrixMultiply(&matrix,&PlayersWeapon.ObMat,&PlayersWeapon.ObMat);
	}
}





/* in mm */
//#define ACTIVATION_Z_RANGE 3000
//#define ACTIVATION_X_RANGE 500
//#define ACTIVATION_Y_RANGE 500

#define ACTIVATION_Z_RANGE 4000
#define ACTIVATION_X_RANGE 2000
#define ACTIVATION_Y_RANGE 2000

int DamageObjectInLineOfSight(PLAYER_WEAPON_DATA *weaponPtr)
{
	int numberOfObjects = NumOnScreenBlocks;
	enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
	
	while (numberOfObjects)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];
		STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;
		GLOBALASSERT(objectPtr);
		
		/* does object have a strategy block? */
		if (sbPtr)
		{		
			/* is it in range? */
			if (objectPtr->ObView.vz > 0 && objectPtr->ObView.vz < ACTIVATION_Z_RANGE)
			{
				int screenX = objectPtr->ObView.vx;
			   	int screenY = objectPtr->ObView.vy;

				if (screenX<0) screenX=-screenX;
				if (screenY<0) screenY=-screenY;
				screenX-=objectPtr->ObMaxX;
				screenY-=objectPtr->ObMaxY;

				if (screenX < ACTIVATION_X_RANGE && screenY < ACTIVATION_Y_RANGE)
				{
					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			  	  	if (dynPtr)
					{
						int magnitudeOfForce = (5000*TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty].Cutting) / dynPtr->Mass;
				  	  	dynPtr->LinImpulse.vx += MUL_FIXED(Player->ObMat.mat31,magnitudeOfForce);
						dynPtr->LinImpulse.vy += MUL_FIXED(Player->ObMat.mat32,magnitudeOfForce);
				  	  	dynPtr->LinImpulse.vz += MUL_FIXED(Player->ObMat.mat33,magnitudeOfForce);
			  			CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
					}
			  	}
			}
		}
	}

 
	return(1);
}


void PredDiscThrowTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr)
{
//  TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	EULER startorient = {0,0,1024};
	EULER endorient = {-1024,0,1024};
	EULER orient;
	VECTORCH startpoint= {400,-50,900};
	VECTORCH endpoint= {-800,-50,200};
	int timeLeft = weaponPtr->StateTimeOutCounter;
	int timeDone = 65536-timeLeft;

	weaponPtr->PositionOffset.vx = WideMul2NarrowDiv(startpoint.vx, timeLeft, endpoint.vx, timeDone, ONE_FIXED);
	weaponPtr->PositionOffset.vy = WideMul2NarrowDiv(startpoint.vy, timeLeft, endpoint.vy, timeDone, ONE_FIXED);
	weaponPtr->PositionOffset.vz = WideMul2NarrowDiv(startpoint.vz, timeLeft, endpoint.vz, timeDone, ONE_FIXED);
	
	orient.EulerX = WideMul2NarrowDiv(startorient.EulerX, timeLeft, endorient.EulerX, timeDone, ONE_FIXED);
	orient.EulerY = WideMul2NarrowDiv(startorient.EulerY, timeLeft, endorient.EulerY, timeDone, ONE_FIXED);
	orient.EulerZ = WideMul2NarrowDiv(startorient.EulerZ, timeLeft, endorient.EulerZ, timeDone, ONE_FIXED);

	CreateEulerMatrix(&orient,&(PlayersWeapon.ObMat));

	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
		MATRIXCH matrix	= VDBPtr->VDB_Mat;
		 
		TransposeMatrixCH(&matrix);
		MatrixMultiply(&matrix,&PlayersWeapon.ObMat,&PlayersWeapon.ObMat);
	}
}

static void DamageDamageBlock(DAMAGEBLOCK *DBPtr, DAMAGE_PROFILE *damage, int multiple) {

	/* Separate function to ease writing clarity. */

	int ArmourDamage;
	int HealthDamage;
	int Fraction;
	int Integer;
	
	Fraction=multiple&(ONE_FIXED-1);
	Integer=multiple>>ONE_FIXED_SHIFT;
	
	/* We must apply damage sequentially. Fraction first, then each 'multiple'. 
	Otherwise, an armour damaging attack hitting multiple times in one frame
	would behave differently depending on the framerate.  Sure, that case might
	never arise, but that's not the point!
	*/

	if (DBPtr->SB_H_flags.Indestructable) {
		return;
	}
	
	if (Fraction) {
	
		ArmourDamage=0;
		HealthDamage=0;
	
		if (damage->Impact) {
			int unblocked,tempdamage;
			/* Impact damage. */
			tempdamage=damage->Impact*Fraction;
			HealthDamage+=tempdamage/10;
			
			//unblocked=tempdamage-DBPtr->Armour;
			unblocked=MUL_FIXED(((damage->Impact*ONE_FIXED)-DBPtr->Armour),Fraction);
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
			tempdamage>>=4;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if (damage->Cutting) {
			int unblocked,tempdamage;
			/* Cutting damage. */
			tempdamage=damage->Cutting*Fraction;
			HealthDamage+=tempdamage>>2;
			
			//unblocked=tempdamage-DBPtr->Armour;
			unblocked=MUL_FIXED(((damage->Cutting*ONE_FIXED)-DBPtr->Armour),Fraction);
			
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
			tempdamage>>=3;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if (damage->Penetrative) {
			int unblocked,tempdamage;
			/* Penetrative damage. */
			tempdamage=damage->Penetrative*Fraction;
			HealthDamage+=tempdamage/10;
			
			//unblocked=tempdamage-(DBPtr->Armour>>6);
			unblocked=MUL_FIXED(((damage->Penetrative*ONE_FIXED)-(DBPtr->Armour>>6)),Fraction);
			
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
			tempdamage>>=4;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if ((damage->Fire)&&(DBPtr->SB_H_flags.FireResistant==0)) {
			int tempdamage;
			/* Fire damage. */
			tempdamage=damage->Fire*Fraction;
			if (DBPtr->SB_H_flags.Combustability==2) {
				tempdamage<<=1;
			} else if (DBPtr->SB_H_flags.Combustability==3) {
				tempdamage>>=2;
			}
			HealthDamage+=tempdamage;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage>>3;
			}
		}
	
		if ((damage->Electrical)&&(DBPtr->SB_H_flags.ElectricResistant==0)) {
			int tempdamage;
			/* Electrical damage. */
			tempdamage=damage->Electrical*Fraction;
			if (DBPtr->SB_H_flags.ElectricSensitive) {
				tempdamage<<=2;
			}
			HealthDamage+=tempdamage;
		}
	
		if ((damage->Acid)&&(DBPtr->SB_H_flags.AcidResistant==0)) {
			int unblocked,tempdamage;
			/* Acid damage. */
			tempdamage=damage->Acid*Fraction;
			unblocked=0;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
					ArmourDamage+=tempdamage;
					if (ArmourDamage>DBPtr->Armour) {
						/* Now we're through. */
						unblocked=tempdamage-DBPtr->Armour;
					}
				} else {
					ArmourDamage+=(tempdamage>>1);
					if (ArmourDamage>DBPtr->Armour) {
						/* Now we're through. */
						unblocked=tempdamage-(DBPtr->Armour<<1);
					}
				}
			}
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
		}
	
		/* Apply Damage. */
	
		if (ArmourDamage>0) {
			if (DBPtr->Armour<ArmourDamage) DBPtr->Armour=0;
			else DBPtr->Armour-=ArmourDamage;
		}
	
		if (HealthDamage>0) {
			if (DBPtr->Health<HealthDamage) DBPtr->Health=0;
			else DBPtr->Health-=HealthDamage;
		}
	
	}
	
	/* Now, the integer. */
	
	while (Integer) {
	
		ArmourDamage=0;
		HealthDamage=0;
	
		if (damage->Impact) {
			int unblocked,tempdamage;
			/* Impact damage. */
			tempdamage=damage->Impact*ONE_FIXED;
			HealthDamage+=tempdamage/10;
			unblocked=tempdamage-DBPtr->Armour;
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
			tempdamage>>=4;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if (damage->Cutting) {
			int unblocked,tempdamage;
			/* Cutting damage. */
			tempdamage=damage->Cutting*ONE_FIXED;
			HealthDamage+=tempdamage>>2;
			unblocked=tempdamage-DBPtr->Armour;
			if (unblocked>0) HealthDamage+=unblocked;
			tempdamage>>=3;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if (damage->Penetrative) {
			int unblocked,tempdamage;
			/* Penetrative damage. */
			tempdamage=damage->Penetrative*ONE_FIXED;
			HealthDamage+=tempdamage/10;
			unblocked=tempdamage-(DBPtr->Armour>>6);
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
			tempdamage>>=4;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage;
			}
		}
	
		if ((damage->Fire)&&(DBPtr->SB_H_flags.FireResistant==0)) {
			int tempdamage;
			/* Fire damage. */
			tempdamage=damage->Fire*ONE_FIXED;
			if (DBPtr->SB_H_flags.Combustability==2) {
				tempdamage<<=1;
			} else if (DBPtr->SB_H_flags.Combustability==3) {
				tempdamage>>=2;
			}
			HealthDamage+=tempdamage;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				ArmourDamage+=tempdamage>>3;
			}
		}
	
		if ((damage->Electrical)&&(DBPtr->SB_H_flags.ElectricResistant==0)) {
			int tempdamage;
			/* Electrical damage. */
			tempdamage=damage->Electrical*ONE_FIXED;
			if (DBPtr->SB_H_flags.ElectricSensitive) {
				tempdamage<<=2;
			}
			HealthDamage+=tempdamage;
		}
	
		if ((damage->Acid)&&(DBPtr->SB_H_flags.AcidResistant==0)) {
			int unblocked,tempdamage;
			/* Acid damage. */
			tempdamage=damage->Acid*ONE_FIXED;
			unblocked=0;
			if (DBPtr->SB_H_flags.PerfectArmour==0) {
				if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
					ArmourDamage+=tempdamage;
					if (ArmourDamage>DBPtr->Armour) {
						/* Now we're through. */
						unblocked=tempdamage-DBPtr->Armour;
					}
				} else {
					ArmourDamage+=(tempdamage>>1);
					if (ArmourDamage>DBPtr->Armour) {
						/* Now we're through. */
						unblocked=tempdamage-(DBPtr->Armour<<1);
					}
				}
			}
			if (unblocked>0) {
				HealthDamage+=unblocked;
			}
		}
	
		/* Apply Damage. */
	
		if (ArmourDamage>0) {
			if (DBPtr->Armour<ArmourDamage) DBPtr->Armour=0;
			else DBPtr->Armour-=ArmourDamage;
		}
	
		if (HealthDamage>0) {
			if (DBPtr->Health<HealthDamage) DBPtr->Health=0;
			else DBPtr->Health-=HealthDamage;
		}
	
		Integer--;
	
	}
	
}

void InitThisWeapon(PLAYER_WEAPON_DATA *pwPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	int shapenum;
	SHAPEHEADER *shptr;

	if (pwPtr->WeaponIDNumber==NULL_WEAPON) {
		/* D'oh! */
		pwPtr->TxAnimCtrl=NULL;
		pwPtr->ShpAnimCtrl.current.seconds_per_frame=0;
		pwPtr->ShpAnimCtrl.current.sequence_no=0;
		pwPtr->ShpAnimCtrl.current.start_frame=0;
		pwPtr->ShpAnimCtrl.current.end_frame=0;
		pwPtr->ShpAnimCtrl.current.default_start_and_end_frames=0;
		pwPtr->ShpAnimCtrl.current.reversed=0;
		pwPtr->ShpAnimCtrl.current.stop_at_end=0;
		pwPtr->ShpAnimCtrl.current.empty=0;
		pwPtr->ShpAnimCtrl.current.stop_now=0;
		pwPtr->ShpAnimCtrl.current.pause_at_end=0;
		pwPtr->ShpAnimCtrl.current.done_a_frame=0;
		pwPtr->ShpAnimCtrl.current.sequence=NULL;
		pwPtr->ShpAnimCtrl.current.current_frame=0;
		pwPtr->ShpAnimCtrl.current.time_to_next_frame=0;

		pwPtr->ShpAnimCtrl.next.seconds_per_frame=0;
		pwPtr->ShpAnimCtrl.next.sequence_no=0;
		pwPtr->ShpAnimCtrl.next.start_frame=0;
		pwPtr->ShpAnimCtrl.next.end_frame=0;
		pwPtr->ShpAnimCtrl.next.default_start_and_end_frames=0;
		pwPtr->ShpAnimCtrl.next.reversed=0;
		pwPtr->ShpAnimCtrl.next.stop_at_end=0;
		pwPtr->ShpAnimCtrl.next.empty=0;
		pwPtr->ShpAnimCtrl.next.stop_now=0;
		pwPtr->ShpAnimCtrl.next.pause_at_end=0;
		pwPtr->ShpAnimCtrl.next.done_a_frame=0;
		pwPtr->ShpAnimCtrl.next.sequence=NULL;
		pwPtr->ShpAnimCtrl.next.current_frame=0;
		pwPtr->ShpAnimCtrl.next.time_to_next_frame=0;

		pwPtr->ShpAnimCtrl.finished=0;
		pwPtr->ShpAnimCtrl.playing=0;
		pwPtr->ShpAnimCtrl.anim_header=NULL;
		return;
	}

	twPtr=&TemplateWeapon[pwPtr->WeaponIDNumber];

	if (twPtr->WeaponShapeName==NULL) return; 
	/* Got to allow for GetLoadedShapeMSL not being bomb-proof. */

	shapenum = GetLoadedShapeMSL(twPtr->WeaponShapeName);
	shptr=GetShapeData(shapenum);

	if (twPtr->HasTextureAnimation) {
		int item_num;
		TXACTRLBLK **pptxactrlblk;		

		pptxactrlblk = &pwPtr->TxAnimCtrl;

		SetupPolygonFlagAccessForShape(shptr);

		for(item_num = 0; item_num < shptr->numitems; item_num ++)
		{
			POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
			LOCALASSERT(poly);
				
			if((Request_PolyFlags((void *)poly)) & iflag_txanim)
			{
				TXACTRLBLK *pnew_txactrlblk;
	
				pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
				
				if (pnew_txactrlblk) 
				{
					// We have allocated the new tx anim control block so initialise it
	
					pnew_txactrlblk->tac_flags = 0;										
					pnew_txactrlblk->tac_item = item_num;										
					pnew_txactrlblk->tac_sequence = 0;										
					pnew_txactrlblk->tac_node = 0;										
					pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shapenum, item_num);										
					pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shapenum);
				
					pnew_txactrlblk->tac_txah.txa_currentframe = 0;
					pnew_txactrlblk->tac_txah.txa_flags |= txa_flag_play;
			
					/* change the value held in pptxactrlblk
					 which point to the previous structures "next"
				 	pointer*/
	
					*pptxactrlblk = pnew_txactrlblk;
					pptxactrlblk = &pnew_txactrlblk->tac_next;
				}
		
			}
		}
		*pptxactrlblk=0;

	} else {
		pwPtr->TxAnimCtrl=NULL;
	}

	/* Now the shape animation... */

	if (twPtr->HasShapeAnimation) {
		InitShapeAnimationController(&pwPtr->ShpAnimCtrl,shptr);
	} else {
		pwPtr->ShpAnimCtrl.current.seconds_per_frame=0;
		pwPtr->ShpAnimCtrl.current.sequence_no=0;
		pwPtr->ShpAnimCtrl.current.start_frame=0;
		pwPtr->ShpAnimCtrl.current.end_frame=0;
		pwPtr->ShpAnimCtrl.current.default_start_and_end_frames=0;
		pwPtr->ShpAnimCtrl.current.reversed=0;
		pwPtr->ShpAnimCtrl.current.stop_at_end=0;
		pwPtr->ShpAnimCtrl.current.empty=0;
		pwPtr->ShpAnimCtrl.current.stop_now=0;
		pwPtr->ShpAnimCtrl.current.pause_at_end=0;
		pwPtr->ShpAnimCtrl.current.done_a_frame=0;
		pwPtr->ShpAnimCtrl.current.sequence=NULL;
		pwPtr->ShpAnimCtrl.current.current_frame=0;
		pwPtr->ShpAnimCtrl.current.time_to_next_frame=0;

		pwPtr->ShpAnimCtrl.next.seconds_per_frame=0;
		pwPtr->ShpAnimCtrl.next.sequence_no=0;
		pwPtr->ShpAnimCtrl.next.start_frame=0;
		pwPtr->ShpAnimCtrl.next.end_frame=0;
		pwPtr->ShpAnimCtrl.next.default_start_and_end_frames=0;
		pwPtr->ShpAnimCtrl.next.reversed=0;
		pwPtr->ShpAnimCtrl.next.stop_at_end=0;
		pwPtr->ShpAnimCtrl.next.empty=0;
		pwPtr->ShpAnimCtrl.next.stop_now=0;
		pwPtr->ShpAnimCtrl.next.pause_at_end=0;
		pwPtr->ShpAnimCtrl.next.done_a_frame=0;
		pwPtr->ShpAnimCtrl.next.sequence=NULL;
		pwPtr->ShpAnimCtrl.next.current_frame=0;
		pwPtr->ShpAnimCtrl.next.time_to_next_frame=0;

		pwPtr->ShpAnimCtrl.finished=0;
		pwPtr->ShpAnimCtrl.playing=0;
		pwPtr->ShpAnimCtrl.anim_header=NULL;
	}
}

void ParticleBeamSwapping(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		/* Setup animation. */
		SHAPEANIMATIONCONTROLDATA sacd;
		InitShapeAnimationControlData(&sacd);
		sacd.seconds_per_frame = ONE_FIXED/36;
		sacd.sequence_no = 0; 
		sacd.default_start_and_end_frames = 1;
		sacd.reversed = 0;
		sacd.stop_at_end = 1;
		SetShapeAnimationSequence(&PlayersWeapon, &sacd);
	}
}

void ParticleBeamReadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	DoShapeAnimation(&PlayersWeapon);

}

void ParticleBeamUnreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		/* Start animation. */
		SHAPEANIMATIONCONTROLDATA sacd;
		InitShapeAnimationControlData(&sacd);
		sacd.seconds_per_frame = ONE_FIXED/36;
		sacd.sequence_no = 0; 
		sacd.default_start_and_end_frames = 1;
		sacd.reversed = 1;
		sacd.stop_at_end = 1;
		SetShapeAnimationSequence(&PlayersWeapon, &sacd);

	} else {
		DoShapeAnimation(&PlayersWeapon);
	}

}

void GrenadeLauncher_UpdateBullets(PLAYER_WEAPON_DATA *weaponPtr) {

	int a;	
	/* Flags the correct number of bullets as visible... */

	for (a=0; a<6; a++) {
		if (a>(weaponPtr->PrimaryRoundsRemaining>>ONE_FIXED_SHIFT)) {
			/* Not vis. */
			GrenadeLauncherSectionPointers[a]->flags|=section_data_notreal;
		} else {
			/* Vis. */
			GrenadeLauncherSectionPointers[a]->flags&=~section_data_notreal;
		}
	}

	switch (GrenadeLauncherData.SelectedAmmo) {
		case AMMO_FLARE_GRENADE:
		{
			GrenadeLauncherData.FlareRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FlareMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			ChangeHUDToAlternateShapeSet("MarineWeapons","Flare");
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{
			GrenadeLauncherData.FragmentationRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FragmentationMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			ChangeHUDToAlternateShapeSet("MarineWeapons","Frag");
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			GrenadeLauncherData.ProximityRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.ProximityMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			ChangeHUDToAlternateShapeSet("MarineWeapons","Proxmine");
			break;
		}
		case AMMO_GRENADE:
		{
			GrenadeLauncherData.StandardRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.StandardMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			ChangeHUDToAlternateShapeSet("MarineWeapons","Grenade");
			break;
		}
		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}


}

int GrenadeLauncherFire(PLAYER_WEAPON_DATA *weaponPtr) {

	int a;

	a=FireNonAutomaticWeapon(weaponPtr);

	//GrenadeLauncher_UpdateBullets(weaponPtr);

	return(a);

}

void GrenadeLauncher_EmergencyChangeAmmo(PLAYER_WEAPON_DATA *weaponPtr) {

	enum AMMO_ID change_to_ammo,original_ammo;

	/* CDF 11/3/99: different priorities here.  And only three ammo types. */
	
	original_ammo=GrenadeLauncherData.SelectedAmmo;
	change_to_ammo=AMMO_NONE;
	
	
	switch(original_ammo) {
		case AMMO_GRENADE:
		{
			if ((GrenadeLauncherData.FragmentationRoundsRemaining>0)
				||(GrenadeLauncherData.FragmentationMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_FRAGMENTATION_GRENADE;
			} else if ((GrenadeLauncherData.ProximityRoundsRemaining>0)
				||(GrenadeLauncherData.ProximityMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_PROXIMITY_GRENADE;
			}
			break;
		}
		case AMMO_FLARE_GRENADE:
		{
			GLOBALASSERT(0);
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{
			if ((GrenadeLauncherData.StandardRoundsRemaining>0)
				||(GrenadeLauncherData.StandardMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_GRENADE;
			} else if ((GrenadeLauncherData.ProximityRoundsRemaining>0)
				||(GrenadeLauncherData.ProximityMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_PROXIMITY_GRENADE;
			}
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			if ((GrenadeLauncherData.StandardRoundsRemaining>0)
				||(GrenadeLauncherData.StandardMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_GRENADE;
			} else if ((GrenadeLauncherData.FragmentationRoundsRemaining>0)
				||(GrenadeLauncherData.FragmentationMagazinesRemaining>0)) {
				/* Valid. */
				change_to_ammo=AMMO_FRAGMENTATION_GRENADE;
			}
			break;
		}
		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}
	

	if ((change_to_ammo==original_ammo)||(change_to_ammo==AMMO_NONE)) {
		/* No other ammo types! */
		return;
	}

	switch (original_ammo) {
		case AMMO_FLARE_GRENADE:
		{
			GrenadeLauncherData.FlareRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FlareMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{
			GrenadeLauncherData.FragmentationRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FragmentationMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			GrenadeLauncherData.ProximityRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.ProximityMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_GRENADE:
		{
			GrenadeLauncherData.StandardRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.StandardMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}

	/* KJL 11:31:44 04/09/97 - when you press secondary fire, you change ammo type */	
	switch(change_to_ammo)
	{
		case AMMO_FLARE_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_FLARE_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.FlareRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.FlareMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_FLARE_GRENADE));
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{

			GrenadeLauncherData.SelectedAmmo = AMMO_FRAGMENTATION_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.FragmentationRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.FragmentationMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_FRAGMENTATION_GRENADE));
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_PROXIMITY_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.ProximityRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.ProximityMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_PROXIMITY_GRENADE));
			break;
		}
		case AMMO_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.StandardRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.StandardMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_GRENADE));
			break;
		}
		default:
			break;
	}

	weaponPtr->CurrentState = WEAPONSTATE_RECOIL_SECONDARY;
	weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;

	return;	
	
}

int GrenadeLauncherChangeAmmo(PLAYER_WEAPON_DATA *weaponPtr) {

	enum AMMO_ID change_to_ammo,original_ammo,temp_ammo;

	/* CDF 1/5/98: only change to ammo that you have! */
	
	original_ammo=GrenadeLauncherData.SelectedAmmo;
	change_to_ammo=AMMO_NONE;
	temp_ammo=original_ammo;
	
	while (change_to_ammo==AMMO_NONE) {
		switch(temp_ammo) {
			case AMMO_GRENADE:
			{
//				temp_ammo=AMMO_FLARE_GRENADE;
				temp_ammo=AMMO_FRAGMENTATION_GRENADE;
				if (temp_ammo==original_ammo) {
					/* Gone all the way round.  Doh! */
					change_to_ammo=temp_ammo;
					break;
				}
				//if ((GrenadeLauncherData.FlareRoundsRemaining>0)
				//	||(GrenadeLauncherData.FlareMagazinesRemaining>0)) {
				if ((GrenadeLauncherData.FragmentationRoundsRemaining>0)
					||(GrenadeLauncherData.FragmentationMagazinesRemaining>0)) {
					/* Valid. */
					change_to_ammo=temp_ammo;
				}
				break;
			}
			case AMMO_FLARE_GRENADE:
			{
				temp_ammo=AMMO_FRAGMENTATION_GRENADE;
				if (temp_ammo==original_ammo) {
					/* Gone all the way round.  Doh! */
					change_to_ammo=temp_ammo;
					break;
				}
				if ((GrenadeLauncherData.FragmentationRoundsRemaining>0)
					||(GrenadeLauncherData.FragmentationMagazinesRemaining>0)) {
					/* Valid. */
					change_to_ammo=temp_ammo;
				}
				break;
			}
			case AMMO_FRAGMENTATION_GRENADE:
			{
				temp_ammo=AMMO_PROXIMITY_GRENADE;
//				temp_ammo=AMMO_GRENADE;
				if (temp_ammo==original_ammo) {
					/* Gone all the way round.  Doh! */
					change_to_ammo=temp_ammo;
					break;
				}
				if ((GrenadeLauncherData.ProximityRoundsRemaining>0)
					||(GrenadeLauncherData.ProximityMagazinesRemaining>0)) {
				//if ((GrenadeLauncherData.StandardRoundsRemaining>0)
				//	||(GrenadeLauncherData.StandardMagazinesRemaining>0)) {
					/* Valid. */
					change_to_ammo=temp_ammo;
				}
				break;
			}
			case AMMO_PROXIMITY_GRENADE:
			{
				temp_ammo=AMMO_GRENADE;
				if (temp_ammo==original_ammo) {
					/* Gone all the way round.  Doh! */
					change_to_ammo=temp_ammo;
					break;
				}
				if ((GrenadeLauncherData.StandardRoundsRemaining>0)
					||(GrenadeLauncherData.StandardMagazinesRemaining>0)) {
					/* Valid. */
					change_to_ammo=temp_ammo;
				}
				break;
			}
			default:
			{
				GLOBALASSERT(0);
				break;
			}
		}
	}

	if (change_to_ammo==original_ammo) {
		/* No other ammo types! */
		return(0);
	}

	switch (original_ammo) {
		case AMMO_FLARE_GRENADE:
		{
			GrenadeLauncherData.FlareRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FlareMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{
			GrenadeLauncherData.FragmentationRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.FragmentationMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			GrenadeLauncherData.ProximityRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.ProximityMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		case AMMO_GRENADE:
		{
			GrenadeLauncherData.StandardRoundsRemaining    = weaponPtr->PrimaryRoundsRemaining;
			GrenadeLauncherData.StandardMagazinesRemaining = weaponPtr->PrimaryMagazinesRemaining;
			break;
		}
		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}

	/* KJL 11:31:44 04/09/97 - when you press secondary fire, you change ammo type */	
	switch(change_to_ammo)
	{
		case AMMO_FLARE_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_FLARE_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.FlareRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.FlareMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_FLARE_GRENADE));
			break;
		}
		case AMMO_FRAGMENTATION_GRENADE:
		{

			GrenadeLauncherData.SelectedAmmo = AMMO_FRAGMENTATION_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.FragmentationRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.FragmentationMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_FRAGMENTATION_GRENADE));
			break;
		}
		case AMMO_PROXIMITY_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_PROXIMITY_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.ProximityRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.ProximityMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_PROXIMITY_GRENADE));
			break;
		}
		case AMMO_GRENADE:
		{
			GrenadeLauncherData.SelectedAmmo = AMMO_GRENADE;
			weaponPtr->PrimaryRoundsRemaining =	GrenadeLauncherData.StandardRoundsRemaining;
			weaponPtr->PrimaryMagazinesRemaining = GrenadeLauncherData.StandardMagazinesRemaining;
			NewOnScreenMessage(GetTextString(TEXTSTRING_AMMO_SHORTNAME_GRENADE));
			break;
		}
		default:
			break;
	}

	//GrenadeLauncher_UpdateBullets(weaponPtr);
    //weaponPtr->CurrentState = WEAPONSTATE_WAITING;

	weaponPtr->CurrentState = WEAPONSTATE_RECOIL_SECONDARY;
	weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;

	return(1);	
	
}

int SmartgunSecondaryFire(PLAYER_WEAPON_DATA *weaponPtr) {

	switch (SmartgunMode) {
		case I_Track:
			/* Language Localise */
			NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_FREEMODE));
			SmartgunMode=I_Free;
			break;
		case I_Free:
			/* Language Localise */
			NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_TRACKMODE));
			SmartgunMode=I_Track;
			break;		   
		default:
			GLOBALASSERT(0);
			break;
	}
	Sound_Play(SID_SMART_MODESWITCH,NULL);

    weaponPtr->CurrentState = WEAPONSTATE_WAITING;

	return(0);	

}

int PredDiscChangeMode(PLAYER_WEAPON_DATA *weaponPtr) {

	#if 0
	switch (ThisDiscMode) {
		case I_Seek_Track:
			ThisDiscMode=I_Search_Destroy;
			/* Language Localise */
			NewOnScreenMessage("SEARCH AND DESTROY");
			break;
		case I_Search_Destroy:
			ThisDiscMode=I_Proximity_Mine;
			/* Language Localise */
			NewOnScreenMessage("PROXIMITY MINE");
			break;
		case I_Proximity_Mine:
			ThisDiscMode=I_Seek_Track;
			/* Language Localise */
			NewOnScreenMessage("SEEK AND TRACK");
			break;
	}
    weaponPtr->CurrentState = WEAPONSTATE_WAITING;
	#endif

	return(0);	
}


int MeleeWeapon_180Degree_Front_Core(DAMAGE_PROFILE *damage,int multiple,int range)
{
	int numberOfObjects = NumOnScreenBlocks;
	int numhits=0;
	STRATEGYBLOCK *objectToHit=0;

	int hurt_people;
	
	hurt_people=1;

	while (numberOfObjects)
	{
		VECTORCH targetpos,targetposW;
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];
		STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;
		GLOBALASSERT(objectPtr);
		
		/* does object have a strategy block? */
		if (sbPtr)
		{		

			GetTargetingPointOfObject(objectPtr,&targetpos);
			targetposW=targetpos;
			targetpos.vx-=Global_VDB_Ptr->VDB_World.vx;
			targetpos.vy-=Global_VDB_Ptr->VDB_World.vy;
			targetpos.vz-=Global_VDB_Ptr->VDB_World.vz;
			RotateVector(&targetpos,&Global_VDB_Ptr->VDB_Mat);

			/* is it in range? */
			if (targetpos.vz > 0) {

				int dist=Approximate3dMagnitude(&targetpos);

				if (dist<range)	{

					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			  	  	if (dynPtr)
					{
						#if 1
						/* KJL 19:10:49 25/01/99 - I'm going to try changing this to
						see if I can have an Alien that doesn't kill multiple things in one frame */

						/* CDF 30/3/99 - I'm going to change it back with modifications,
						since people evidently can't make up their minds. */

						//if (IsThisObjectVisibleFromThisPosition_WithIgnore(Player,objectPtr,&targetposW,range))
						if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&targetposW))
						{

							int magnitudeOfForce = (5000*damage->Cutting) / dynPtr->Mass;
					  	  	dynPtr->LinImpulse.vx += MUL_FIXED(Player->ObMat.mat31,magnitudeOfForce);
							dynPtr->LinImpulse.vy += MUL_FIXED(Player->ObMat.mat32,magnitudeOfForce);
					  	  	dynPtr->LinImpulse.vz += MUL_FIXED(Player->ObMat.mat33,magnitudeOfForce);
							/* Consider target aspect. */
							{
								VECTORCH attack_dir,displacement;
								int real_multiple;
								int do_attack;

								displacement.vx = dynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->Position.vx;
								displacement.vy = dynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->Position.vy;
								displacement.vz = dynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->Position.vz;

								GetDirectionOfAttack(sbPtr,&displacement,&attack_dir);
		
								if (attack_dir.vz>0) {
									real_multiple=multiple<<1;
								} else {
									real_multiple=multiple;
								}
							
					  			/* Consider player's target. */

								/* Here's the mod. */
								switch (sbPtr->I_SBtype) {
									case I_BehaviourMarine:
									case I_BehaviourMarinePlayer:
									case I_BehaviourPredator:
									case I_BehaviourAutoGun:
									case I_BehaviourAlien:
										if (hurt_people) {
											do_attack=1;
											hurt_people=0;
										} else {
											do_attack=0;
										}
										break;
									default:
										do_attack=1;
										break;
								}

								if (do_attack) {
									if (sbPtr->SBdptr->HModelControlBlock) {
										DISPLAYBLOCK *frag;

										frag=HtoHDamageToHModel(sbPtr, damage, real_multiple, NULL, &attack_dir);
										/* If you're an alien, consider limb rip damage. */
										if (AvP.PlayerType==I_Alien) {
											if (sbPtr->I_SBtype==I_BehaviourMarine) {
												MARINE_STATUS_BLOCK *marineStatusPointer;
											
												marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
											    LOCALASSERT(marineStatusPointer);	          		
							
												if (marineStatusPointer->Android==0) {					
													if (frag) {
														/* Took off a limb! */
														LimbRip_AwardHealth();
													}
												}
											} else if (sbPtr->I_SBtype==I_BehaviourNetCorpse) {
												NETCORPSEDATABLOCK *corpseDataPtr;
											
												corpseDataPtr = (NETCORPSEDATABLOCK *)(sbPtr->SBdataptr);
												LOCALASSERT(corpseDataPtr);
							
												if (corpseDataPtr->Android==0) {					
													if (frag) {
														/* Took off a limb! */
														LimbRip_AwardHealth();
													}
												}
											}
										}
									} else {
						  				CauseDamageToObject(sbPtr, damage, real_multiple,&attack_dir);
									}
								}
							}
							numhits++;
						}
						#else
						//if (IsThisObjectVisibleFromThisPosition_WithIgnore(Player,objectPtr,&targetposW,range))
						if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&targetposW)) {
							objectToHit = sbPtr;
							range = dist;
						}
						#endif
					}
			  	}
			}
		}
	}

	/* Here's a new modification... */
	if (numhits==0) {
		/* Let's see if we've missed something obvious. */
		if (PlayersTarget.DispPtr) {
			if (PlayersTarget.Distance<range) {
				if (PlayersTarget.DispPtr->ObStrategyBlock) {
					/* May as well hit this, then. */
					objectToHit=PlayersTarget.DispPtr->ObStrategyBlock;
					if (objectToHit->DynPtr) {
						DYNAMICSBLOCK *dynPtr = objectToHit->DynPtr;
						int magnitudeOfForce = (5000*damage->Cutting) / dynPtr->Mass;
				  	  	dynPtr->LinImpulse.vx += MUL_FIXED(Player->ObMat.mat31,magnitudeOfForce);
						dynPtr->LinImpulse.vy += MUL_FIXED(Player->ObMat.mat32,magnitudeOfForce);
				  	  	dynPtr->LinImpulse.vz += MUL_FIXED(Player->ObMat.mat33,magnitudeOfForce);
						/* Consider target aspect. */
						{
							VECTORCH attack_dir,displacement;
							int real_multiple;

							displacement.vx = dynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->Position.vx;
							displacement.vy = dynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->Position.vy;
							displacement.vz = dynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->Position.vz;

							GetDirectionOfAttack(objectToHit,&displacement,&attack_dir);

							if (attack_dir.vz>0) {
								real_multiple=multiple<<1;
							} else {
								real_multiple=multiple;
							}
						
				  			/* Consider player's target. */

							if (objectToHit->SBdptr->HModelControlBlock) {
								DISPLAYBLOCK *frag;

								frag=HtoHDamageToHModel(objectToHit, damage, real_multiple, NULL, &attack_dir);
								/* If you're an alien, consider limb rip damage. */
								if (AvP.PlayerType==I_Alien) {
									if (objectToHit->I_SBtype==I_BehaviourMarine) {
										MARINE_STATUS_BLOCK *marineStatusPointer;
									
										marineStatusPointer = (MARINE_STATUS_BLOCK *)(objectToHit->SBdataptr);    
									    LOCALASSERT(marineStatusPointer);	          		
					
										if (marineStatusPointer->Android==0) {					
											if (frag) {
												/* Took off a limb! */
												LimbRip_AwardHealth();
											}
										}
									} else if (objectToHit->I_SBtype==I_BehaviourNetCorpse) {
										NETCORPSEDATABLOCK *corpseDataPtr;
									
										corpseDataPtr = (NETCORPSEDATABLOCK *)(objectToHit->SBdataptr);
										LOCALASSERT(corpseDataPtr);
					
										if (corpseDataPtr->Android==0) {					
											if (frag) {
												/* Took off a limb! */
												LimbRip_AwardHealth();
											}
										}
									}
								}
							} else {
				  				CauseDamageToObject(objectToHit, damage, real_multiple,&attack_dir);
							}
						}
						numhits++;
					}
				}
			}
		}
	}

	return(numhits);
}

int MeleeWeapon_180Degree_Front(PLAYER_WEAPON_DATA *weaponPtr)
{
	enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
	int hits;

	hits=MeleeWeapon_180Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);

	return(hits);
}

int MeleeWeapon_90Degree_Front_Core(DAMAGE_PROFILE *damage,int multiple,int range)
{
	int numberOfObjects = NumOnScreenBlocks;
	int numhits=0;
	STRATEGYBLOCK *objectToHit=NULL;

	int hurt_people;
	
	hurt_people=1;

	while (numberOfObjects)
	{
		STRATEGYBLOCK *sbPtr;
		VECTORCH targetpos,targetposW;
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];

		GLOBALASSERT(objectPtr);
		sbPtr = objectPtr->ObStrategyBlock;
		
		/* does object have a strategy block? */
		if (sbPtr)
		{		
			GetTargetingPointOfObject(objectPtr,&targetpos);
			targetposW=targetpos;
			targetpos.vx-=Global_VDB_Ptr->VDB_World.vx;
			targetpos.vy-=Global_VDB_Ptr->VDB_World.vy;
			targetpos.vz-=Global_VDB_Ptr->VDB_World.vz;
			RotateVector(&targetpos,&Global_VDB_Ptr->VDB_Mat);
		
			/* is it in the frustrum? */
			if ( (targetpos.vz >0) 
				&& (targetpos.vz >  targetpos.vx) 
				&& (targetpos.vz > -targetpos.vx) 
				&& (targetpos.vz >  targetpos.vy) 
				&& (targetpos.vz > -targetpos.vy) ) {

				int dist=Approximate3dMagnitude(&targetpos);

				/* HACKAHACKAHACKA */

				if (objectPtr->HModelControlBlock==NULL) {
					if (objectPtr->ObShapeData) {
						dist-=(objectPtr->ObShapeData->shaperadius)>>1;
					}
				}

				if (dist<range)	{

					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			  	  	if (dynPtr)
					{

						if (IsThisObjectVisibleFromThisPosition_WithIgnore(Player,objectPtr,&targetposW,range)) {

							int magnitudeOfForce = (5000*damage->Cutting) / dynPtr->Mass;
				  		  	dynPtr->LinImpulse.vx += MUL_FIXED(Player->ObMat.mat31,magnitudeOfForce);
							dynPtr->LinImpulse.vy += MUL_FIXED(Player->ObMat.mat32,magnitudeOfForce);
				  		  	dynPtr->LinImpulse.vz += MUL_FIXED(Player->ObMat.mat33,magnitudeOfForce);
			  				/* Consider player's target. */
			  				
							/* Consider target aspect. */
							{
								VECTORCH attack_dir,displacement;
								int real_multiple;
								int do_attack;

								displacement.vx = dynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->Position.vx;
								displacement.vy = dynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->Position.vy;
								displacement.vz = dynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->Position.vz;

								GetDirectionOfAttack(sbPtr,&displacement,&attack_dir);
		
								if (attack_dir.vz>0) {
									real_multiple=multiple<<1;
								} else {
									real_multiple=multiple;
								}
							
					  			/* Consider player's target. */
								
								switch (sbPtr->I_SBtype) {
									case I_BehaviourMarine:
									case I_BehaviourMarinePlayer:
									case I_BehaviourPredator:
									case I_BehaviourAutoGun:
									case I_BehaviourAlien:
										if (hurt_people) {
											do_attack=1;
											hurt_people=0;
										} else {
											do_attack=0;
										}
										break;
									default:
										do_attack=1;
										break;
								}

								if (do_attack) {
									if (sbPtr->SBdptr->HModelControlBlock) {
										if (damage->Special) {
											/* Target an alien's chest. */
											if (sbPtr->I_SBtype==I_BehaviourAlien) {
												SECTION_DATA *chest;
												chest=GetThisSectionData(sbPtr->SBdptr->HModelControlBlock->section_data,"chest");
												GLOBALASSERT(chest);
												CauseDamageToHModel(sbPtr->SBdptr->HModelControlBlock,sbPtr,damage,real_multiple,chest,&attack_dir,NULL,0);
											} else {
												HtoHDamageToHModel(sbPtr, damage, real_multiple, NULL, &attack_dir);
											}
										} else {
											HtoHDamageToHModel(sbPtr, damage, real_multiple, NULL, &attack_dir);
										}
									} else {
					  					CauseDamageToObject(sbPtr, damage, real_multiple,&attack_dir);
									}
								}
							}
							numhits++;
						}
					}
			  	}
			}
		}
	}
	
	/* Here's a new modification... */
	if (numhits==0) {
		/* Let's see if we've missed something obvious. */
		if (PlayersTarget.DispPtr) {
			if (PlayersTarget.Distance<range) {
				if (PlayersTarget.DispPtr->ObStrategyBlock) {
					/* May as well hit this, then. */
					objectToHit=PlayersTarget.DispPtr->ObStrategyBlock;
					if (objectToHit->DynPtr) {
						DYNAMICSBLOCK *dynPtr = objectToHit->DynPtr;
						int magnitudeOfForce = (5000*damage->Cutting) / dynPtr->Mass;
				  	  	dynPtr->LinImpulse.vx += MUL_FIXED(Player->ObMat.mat31,magnitudeOfForce);
						dynPtr->LinImpulse.vy += MUL_FIXED(Player->ObMat.mat32,magnitudeOfForce);
				  	  	dynPtr->LinImpulse.vz += MUL_FIXED(Player->ObMat.mat33,magnitudeOfForce);
						/* Consider target aspect. */
						{
							VECTORCH attack_dir,displacement;
							int real_multiple;

							displacement.vx = dynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->Position.vx;
							displacement.vy = dynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->Position.vy;
							displacement.vz = dynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->Position.vz;

							GetDirectionOfAttack(objectToHit,&displacement,&attack_dir);

							if (attack_dir.vz>0) {
								real_multiple=multiple<<1;
							} else {
								real_multiple=multiple;
							}
						
				  			/* Consider player's target. */

							if (objectToHit->SBdptr->HModelControlBlock) {
								DISPLAYBLOCK *frag;

								frag=HtoHDamageToHModel(objectToHit, damage, real_multiple, NULL, &attack_dir);
								/* If you're an alien, consider limb rip damage. */
								if (AvP.PlayerType==I_Alien) {
									if (objectToHit->I_SBtype==I_BehaviourMarine) {
										MARINE_STATUS_BLOCK *marineStatusPointer;
									
										marineStatusPointer = (MARINE_STATUS_BLOCK *)(objectToHit->SBdataptr);    
									    LOCALASSERT(marineStatusPointer);	          		
					
										if (marineStatusPointer->Android==0) {					
											if (frag) {
												/* Took off a limb! */
												LimbRip_AwardHealth();
											}
										}
									} else if (objectToHit->I_SBtype==I_BehaviourNetCorpse) {
										NETCORPSEDATABLOCK *corpseDataPtr;
									
										corpseDataPtr = (NETCORPSEDATABLOCK *)(objectToHit->SBdataptr);
										LOCALASSERT(corpseDataPtr);
					
										if (corpseDataPtr->Android==0) {					
											if (frag) {
												/* Took off a limb! */
												LimbRip_AwardHealth();
											}
										}
									}
								}
							} else {
				  				CauseDamageToObject(objectToHit, damage, real_multiple,&attack_dir);
							}
						}
						numhits++;
					}
				}
			}
		}
	}


	return(numhits);
}

int MeleeWeapon_90Degree_Front(PLAYER_WEAPON_DATA *weaponPtr)
{
	enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
	int hits;

	hits=MeleeWeapon_90Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);

	return(hits);
}
#if 0
void WeaponCreateStartFrame(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	extern void CopyAnimationFrameToShape (SHAPEANIMATIONCONTROLDATA *sacd, DISPLAYBLOCK * dptr);

	/* Setup animation. */
	SHAPEANIMATIONCONTROLDATA sacd;
	InitShapeAnimationControlData(&sacd);
	sacd.seconds_per_frame = ONE_FIXED/16;
	sacd.sequence_no = 0; 
	sacd.default_start_and_end_frames = 1;
	sacd.reversed = 0;
	sacd.stop_at_end = 1;
	SetShapeAnimationSequence(&PlayersWeapon, &sacd);
	sacd.sequence = &(PlayersWeapon.ShapeAnimControlBlock)->anim_header->anim_sequences[sacd.sequence_no];
	sacd.current_frame=0;
	CopyAnimationFrameToShape(&sacd, &PlayersWeapon);
	WeaponFidgetPlaying=0;

}
#endif
void WeaponSetStartFrame(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	#if 0 //old shape animation stuff
	extern void CopyAnimationFrameToShape (SHAPEANIMATIONCONTROLDATA *sacd, DISPLAYBLOCK * dptr);

	/* Setup animation. */
	SHAPEANIMATIONCONTROLDATA sacd;
	InitShapeAnimationControlData(&sacd);
	sacd.seconds_per_frame = ONE_FIXED/16;
	sacd.sequence_no = 0; 
	sacd.default_start_and_end_frames = 1;
	sacd.reversed = 0;
	sacd.stop_at_end = 1;
	SetShapeAnimationSequence(&PlayersWeapon, &sacd);
	WeaponFidgetPlaying=0;
	#endif

}

void PulseRifleSwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,ONE_FIXED>>3);
		PlayersWeaponHModelController.Looped=0;
	}

	Weapon_ThisBurst=-1;
}

void PulseRifleSwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Go,ONE_FIXED>>3);
		PlayersWeaponHModelController.Looped=0;
	}

}

void PulseRifleGrenadeRecoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Secondary_Fire,ONE_FIXED);
		PlayersWeaponHModelController.Looped=0;
	}

}

void PulseRifleReloadClip(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Reload,ONE_FIXED);
		PlayersWeaponHModelController.Looped=0;
	}

}

void MinigunStartSpin(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	Old_Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
	Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
	Weapon_ThisBurst=-1;
	Minigun_HeadJolt.EulerX=0;
	Minigun_HeadJolt.EulerY=0;
	Minigun_HeadJolt.EulerZ=0;
	Minigun_MaxHeadJolt.EulerX=0;
	Minigun_MaxHeadJolt.EulerY=0;
	Minigun_MaxHeadJolt.EulerZ=0;

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Standard_Fire) {
		#if (MINIGUN_IDLE_SPEED) 
			InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire,DIV_FIXED(ONE_FIXED,MINIGUN_IDLE_SPEED));
			PlayersWeaponHModelController.Playing=1;
		#else 
			InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire,ONE_FIXED);
			PlayersWeaponHModelController.Playing=0;
		#endif
	}

}

void MinigunStopSpin(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	/* Spin down first. */

	Minigun_SpinSpeed-=(NormalFrameTime<<3);
	if (Minigun_SpinSpeed<MINIGUN_IDLE_SPEED) {

		weaponPtr->StateTimeOutCounter = 0;
		Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
		
		/* Stop Spin. */
		PlayersWeaponHModelController.Playing=0;
	} else {
		/* Can't stop yet. */
		weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;

	}

	if (Minigun_SpinSpeed!=Old_Minigun_SpinSpeed) {
		int hmspinrate;
		if (Minigun_SpinSpeed) {
			hmspinrate=DIV_FIXED(ONE_FIXED,Minigun_SpinSpeed);
			HModel_ChangeSpeed(&PlayersWeaponHModelController,hmspinrate);
			PlayersWeaponHModelController.Playing=1;
		} else {
			PlayersWeaponHModelController.Playing=0;
		}
	}

	Old_Minigun_SpinSpeed=Minigun_SpinSpeed;
	
	textprint("Minigun Spin Speed = %d\n",Minigun_SpinSpeed);

	/* Think sounds. */
	if (Minigun_SpinSpeed==0) {
		/* No sound at all, ideally! */
  		if(weaponHandle != SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_END) {
				/* Allow SID_MINIGUN_END to stop if it's going. */
	       		Sound_Stop(weaponHandle);
			}
		}
	} else {
		/* Winding down - should be playing SID_MINIGUN_END. */
  		if(weaponHandle != SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_END) {
				/* Stop other sounds... */
	       		Sound_Stop(weaponHandle);
			}
		}
  		if(weaponHandle == SOUND_NOACTIVEINDEX) {
  			Sound_Play(SID_MINIGUN_END,"eh",&weaponHandle);
			playerNoise=1;
		}
	}
}

#define MINIGUN_MOVING_IMPULSE	(-36000)
#define MINIGUN_JOLTTIME_SHIFT	(2)

int FireMinigun(PLAYER_WEAPON_DATA *weaponPtr) {

	#if FORCE_MINIGUN_STOP
	if (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor) {
		Minigun_SpinSpeed+=(NormalFrameTime<<7);
	}
	#else
	if ((Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
		&&(Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
		&&(Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz)
		&&(Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)
		){

		Minigun_SpinSpeed+=(NormalFrameTime<<7);
	} else {
		Minigun_SpinSpeed-=(NormalFrameTime<<3);
		if (Minigun_SpinSpeed<MINIGUN_IDLE_SPEED) {
			Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
		}
		return(0);
	}
	#endif
	if (Minigun_SpinSpeed>=MINIGUN_MAX_SPEED) {
	
		Minigun_SpinSpeed=MINIGUN_MAX_SPEED;

		Weapon_ThisBurst+=FireAutomaticWeapon(weaponPtr);

		/* Give the player an impulse? */
		if ((Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
			||(Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
			||(Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0))
		{
			int impulse;

			impulse=MUL_FIXED(MINIGUN_MOVING_IMPULSE,NormalFrameTime);

			Player->ObStrategyBlock->DynPtr->LinImpulse.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat31,impulse);
			Player->ObStrategyBlock->DynPtr->LinImpulse.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat32,impulse);
			Player->ObStrategyBlock->DynPtr->LinImpulse.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat33,impulse);

			Minigun_MaxHeadJolt.EulerX=-(78+(FastRandom()&63));
			Minigun_MaxHeadJolt.EulerY=-(78+(FastRandom()&63));
			Minigun_MaxHeadJolt.EulerZ=300;

			Minigun_HeadJolt.EulerX	= Minigun_MaxHeadJolt.EulerX;
			Minigun_HeadJolt.EulerY	= Minigun_MaxHeadJolt.EulerY;
			Minigun_HeadJolt.EulerZ	= Minigun_MaxHeadJolt.EulerZ;

		}
		
		return(1);
	} else {
		Weapon_ThisBurst=0;
		Minigun_HeadJolt.EulerX=0;
		Minigun_HeadJolt.EulerY=0;
		Minigun_HeadJolt.EulerZ=0;

		Minigun_MaxHeadJolt.EulerX=0;
		Minigun_MaxHeadJolt.EulerY=0;
		Minigun_MaxHeadJolt.EulerZ=0;
	}

	/* Maintain_Minigun called anyway. */

	return(0);

}

int FireEmptyMinigun(PLAYER_WEAPON_DATA *weaponPtr) {

	#if FORCE_MINIGUN_STOP
	if (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor) {
		Minigun_SpinSpeed+=(NormalFrameTime<<7);
	}
	#else
	if ((Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
		&&(Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
		&&(Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz)
		&&(Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)
		){

		Minigun_SpinSpeed+=(NormalFrameTime<<7);
	} else {
		Minigun_SpinSpeed-=(NormalFrameTime<<3);
		if (Minigun_SpinSpeed<MINIGUN_IDLE_SPEED) {
			Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
		}
		return(0);
	}
	#endif
	if (Minigun_SpinSpeed>=MINIGUN_MAX_SPEED) {
	
		Minigun_SpinSpeed=MINIGUN_MAX_SPEED;

		/* No bullets, no impulse. */
				
		return(1);
	} else {
		Weapon_ThisBurst=0;
		Minigun_HeadJolt.EulerX=0;
		Minigun_HeadJolt.EulerY=0;
		Minigun_HeadJolt.EulerZ=0;

		Minigun_MaxHeadJolt.EulerX=0;
		Minigun_MaxHeadJolt.EulerY=0;
		Minigun_MaxHeadJolt.EulerZ=0;
	}

	/* Maintain_Minigun called anyway. */

	return(0);

}


void Maintain_Minigun(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

   	if (weaponPtr->CurrentState!=WEAPONSTATE_FIRING_PRIMARY) {
		Minigun_SpinSpeed-=(NormalFrameTime<<3);
		if (Minigun_SpinSpeed<MINIGUN_IDLE_SPEED) {
			Minigun_SpinSpeed=MINIGUN_IDLE_SPEED;
		}
		if (Weapon_ThisBurst!=0) {
			Weapon_ThisBurst=-1;
			Minigun_HeadJolt.EulerX=0;
			Minigun_HeadJolt.EulerY=0;
			Minigun_HeadJolt.EulerZ=0;

			Minigun_MaxHeadJolt.EulerX=0;
			Minigun_MaxHeadJolt.EulerY=0;
			Minigun_MaxHeadJolt.EulerZ=0;
		}
		/* Not firing - play empty or wind down sound.  Which one? */
		if (Minigun_SpinSpeed==0) {
			/* No sound at all, ideally! */
	   		if(weaponHandle != SOUND_NOACTIVEINDEX) {
				if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_END) {
					/* Allow SID_MINIGUN_END to stop if it's going. */
		       		Sound_Stop(weaponHandle);
				}
			}
		} else if (Minigun_SpinSpeed<Old_Minigun_SpinSpeed) {
			/* Winding down - should be playing SID_MINIGUN_END. */
	   		if(weaponHandle != SOUND_NOACTIVEINDEX) {
				if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_END) {
					/* Should be playing SID_MINIGUN_LOOP. */
		       		Sound_Stop(weaponHandle);
		   			Sound_Play(SID_MINIGUN_END,"eh",&weaponHandle);
					playerNoise=1;
				}
			}
			#if 0
			/* Removed to stop multiple playings! */
	   		if(weaponHandle == SOUND_NOACTIVEINDEX) {
	   			Sound_Play(SID_MINIGUN_END,"eh",&weaponHandle);
				playerNoise=1;
			}
			#endif
		} else {
			/* Winding up or steady - play SID_MINIGUN_EMPTY. */
	   		if(weaponHandle != SOUND_NOACTIVEINDEX) {
				if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_EMPTY) {
					/* Stop other sounds... */
		       		Sound_Stop(weaponHandle);
				}
			}
	   		if(weaponHandle == SOUND_NOACTIVEINDEX) {
	   			Sound_Play(SID_MINIGUN_EMPTY,"elh",&weaponHandle);
				playerNoise=1;
			}
		}
	} else {
		if (Weapon_ThisBurst<MINIGUN_MINIMUM_BURST) {
			/* Forge a new fire request somehow? */
		} else {
			Weapon_ThisBurst=MINIGUN_MINIMUM_BURST;
		}
		/* Firing - play loop sound! */
   		if(weaponHandle != SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[weaponHandle].soundIndex!=SID_MINIGUN_LOOP) {
	       		Sound_Stop(weaponHandle);
			}
		}
   		if(weaponHandle == SOUND_NOACTIVEINDEX) {
   			Sound_Play(SID_MINIGUN_LOOP,"elh",&weaponHandle);
			playerNoise=1;
		}
	}

	if (Minigun_SpinSpeed!=Old_Minigun_SpinSpeed) {
		int hmspinrate;
		if (Minigun_SpinSpeed) {
			hmspinrate=DIV_FIXED(ONE_FIXED,Minigun_SpinSpeed);
			HModel_ChangeSpeed(&PlayersWeaponHModelController,hmspinrate);
			PlayersWeaponHModelController.Playing=1;
		} else {
			PlayersWeaponHModelController.Playing=0;
		}
	}

	if ((Minigun_HeadJolt.EulerX)
		||(Minigun_HeadJolt.EulerY)
		||(Minigun_HeadJolt.EulerZ)) {
	
		int joltratex,joltratey,joltratez;

		joltratex=MUL_FIXED((NormalFrameTime<<MINIGUN_JOLTTIME_SHIFT),Minigun_MaxHeadJolt.EulerX);
		joltratey=MUL_FIXED((NormalFrameTime<<MINIGUN_JOLTTIME_SHIFT),Minigun_MaxHeadJolt.EulerY);
		joltratez=MUL_FIXED((NormalFrameTime<<MINIGUN_JOLTTIME_SHIFT),Minigun_MaxHeadJolt.EulerZ);
		
		if (Minigun_HeadJolt.EulerX>0) {
			if (joltratex>Minigun_HeadJolt.EulerX) {
				joltratex=Minigun_HeadJolt.EulerX;
			}
			Minigun_HeadJolt.EulerX-=joltratex;
			if (Minigun_HeadJolt.EulerX<0) {
				Minigun_HeadJolt.EulerX=0;
			}
		} else if (Minigun_HeadJolt.EulerX<0) {
			if (joltratex<Minigun_HeadJolt.EulerX) {
				joltratex=Minigun_HeadJolt.EulerX;
			}
			Minigun_HeadJolt.EulerX-=joltratex;
			if (Minigun_HeadJolt.EulerX>0) {
				Minigun_HeadJolt.EulerX=0;
			}
		}

        ((PLAYER_STATUS *)playerStatus)->ViewPanX += joltratex;
		if ((((PLAYER_STATUS *)playerStatus)->ViewPanX>1024)
			&&(((PLAYER_STATUS *)playerStatus)->ViewPanX<3200)) {
			((PLAYER_STATUS *)playerStatus)->ViewPanX=3200;
		}
		/* Okay, that 3200 comes from 3072 + '128'.  '128' is hardcoded into pmove.c. */
		((PLAYER_STATUS *)playerStatus)->ViewPanX &= wrap360;

		if (Minigun_HeadJolt.EulerY>0) {
			if (joltratey>Minigun_HeadJolt.EulerY) {
				joltratey=Minigun_HeadJolt.EulerY;
			}
			Minigun_HeadJolt.EulerY-=joltratey;
			if (Minigun_HeadJolt.EulerY<0) {
				Minigun_HeadJolt.EulerY=0;
			}
		} else if (Minigun_HeadJolt.EulerY<0) {
			if (joltratey<Minigun_HeadJolt.EulerY) {
				joltratey=Minigun_HeadJolt.EulerY;
			}
			Minigun_HeadJolt.EulerY-=joltratey;
			if (Minigun_HeadJolt.EulerY>0) {
				Minigun_HeadJolt.EulerY=0;
			}
		}
		joltratey&=wrap360;
		/* Forcibly turn the player! */		
		{
			MATRIXCH mat;   	
	 	  	int cos = GetCos(joltratey);
	 	  	int sin = GetSin(joltratey);
	 	  	mat.mat11 = cos;		 
	 	  	mat.mat12 = 0;
	 	  	mat.mat13 = -sin;
	 	  	mat.mat21 = 0;	  	
	 	  	mat.mat22 = 65536;	  	
	 	  	mat.mat23 = 0;	  	
	 	  	mat.mat31 = sin;	  	
	 	  	mat.mat32 = 0;	  	
	 	  	mat.mat33 = cos;	  	
			
			MatrixMultiply(&Player->ObStrategyBlock->DynPtr->OrientMat,&mat,&Player->ObStrategyBlock->DynPtr->OrientMat);

		 	MatrixToEuler(&Player->ObStrategyBlock->DynPtr->OrientMat, &Player->ObStrategyBlock->DynPtr->OrientEuler);
		}

		if (Minigun_HeadJolt.EulerZ>0) {
			if (joltratex>Minigun_HeadJolt.EulerZ) {
				joltratex=Minigun_HeadJolt.EulerZ;
			}
			Minigun_HeadJolt.EulerZ-=joltratez;
			if (Minigun_HeadJolt.EulerZ<0) {
				Minigun_HeadJolt.EulerZ=0;
			}
		} else if (Minigun_HeadJolt.EulerZ<0) {
			if (joltratex<Minigun_HeadJolt.EulerZ) {
				joltratex=Minigun_HeadJolt.EulerZ;
			}
			Minigun_HeadJolt.EulerZ-=joltratez;
			if (Minigun_HeadJolt.EulerZ>0) {
				Minigun_HeadJolt.EulerZ=0;
			}
		}
		HeadOrientation.EulerZ+=joltratez;

	}

	Old_Minigun_SpinSpeed=Minigun_SpinSpeed;
	
	textprint("Minigun Spin Speed = %d\n",Minigun_SpinSpeed);

}

void GrenadeLauncherRecoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire,(ONE_FIXED*4)/3);
		PlayersWeaponHModelController.Looped=0;
	}
}

void GrenadeLauncherReload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Standard_Reload) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Reload,(ONE_FIXED*4)/3);
		PlayersWeaponHModelController.Looped=0;
		GrenadeLauncher_UpdateBullets(weaponPtr);
	}
	
	if (PlayersWeaponHModelController.keyframe_flags) {
    	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
		TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];

		/* Reload grenade launcher ahead of schedule. */
		weaponPtr->PrimaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
		weaponPtr->PrimaryMagazinesRemaining--;
		GrenadeLauncher_UpdateBullets(weaponPtr);

	}
}

void GrenadeLauncherReload_Change(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Standard_Reload) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Reload,(ONE_FIXED*4)/3);
		PlayersWeaponHModelController.Looped=0;
		/* No update bullets here. */
	}
	
	if (PlayersWeaponHModelController.keyframe_flags) {
    	TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
		TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];

		/* Reload grenade launcher if unloaded. */
		if (weaponPtr->PrimaryRoundsRemaining==0) {
			weaponPtr->PrimaryRoundsRemaining = templateAmmoPtr->AmmoPerMagazine;
			weaponPtr->PrimaryMagazinesRemaining--;
		}
		GrenadeLauncher_UpdateBullets(weaponPtr);
		/* A little cheat... */
		if (weaponPtr->PrimaryRoundsRemaining<templateAmmoPtr->AmmoPerMagazine) {
			GrenadeLauncherSectionPointers[0]->flags|=section_data_notreal;
		}
	}
}

void GrenadeLauncherNull(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* A bit of a hack - for resetting position. */

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	/* Pop the round in the breach... */
	GrenadeLauncherSectionPointers[0]->flags|=section_data_notreal;
	/* It'll be put back at the next update. */

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
	}
}

void GrenadeLauncherIdle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

//	textprint("GL Rounds = %d\n",(weaponPtr->PrimaryRoundsRemaining>>ONE_FIXED_SHIFT));
	/* Don't update on secondary fire! */

	GrenadeLauncher_UpdateBullets(weaponPtr);

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
	}

}

void GrenadeLauncherFidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

//	textprint("GL Rounds = %d\n",(weaponPtr->PrimaryRoundsRemaining>>ONE_FIXED_SHIFT));
	GrenadeLauncher_UpdateBullets(weaponPtr);

	if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
	}

	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,ONE_FIXED,0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	}
	
}

void GrenadeLauncher_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,ONE_FIXED/3);
		PlayersWeaponHModelController.Looped=0;
	}
}

void GrenadeLauncher_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Go,ONE_FIXED/3);
		PlayersWeaponHModelController.Looped=0;
	}
}

void GrenadeLauncherInit(PLAYER_WEAPON_DATA *weaponPtr) {

	int a;

	/* Setup grenades. */

	for (a=0; a<6; a++) {
		GrenadeLauncherSectionPointers[a]=GetThisSectionData(
			PlayersWeaponHModelController.section_data,GrenadeLauncherBulletNames[a]);
	}

	GrenadeLauncher_UpdateBullets(weaponPtr);

}

void PulseRifleFidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
	}

	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,ONE_FIXED,0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	
	}

}

void WristBlade_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
	}

	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

}

void TemplateHands_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		SECTION *root_section;

		root_section=GetNamedHierarchyFromLibrary("pred_HUD","Template");
		GLOBALASSERT(root_section);

		PlayersWeaponHModelController.Sequence_Type=(int)HMSQT_PredatorHUD;
		PlayersWeaponHModelController.Sub_Sequence=(int)PHSS_Go;
		PlayersWeaponHModelController.Seconds_For_Sequence=(ONE_FIXED/3);
		/* That to get the new sections right. */
		Transmogrify_HModels(NULL,&PlayersWeaponHModelController,root_section, 0, 1,0);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Go,(ONE_FIXED/3),0);

	}
}

void WristBlade_Readying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		SECTION *root_section;

		root_section=GetNamedHierarchyFromLibrary("pred_HUD","wrist blade");
		GLOBALASSERT(root_section);

		PlayersWeaponHModelController.Sequence_Type=(int)HMSQT_PredatorHUD;
		PlayersWeaponHModelController.Sub_Sequence=(int)PHSS_Come;
		PlayersWeaponHModelController.Seconds_For_Sequence=(ONE_FIXED>>1);
		/* That to get the new sections right. */
		Transmogrify_HModels(NULL,&PlayersWeaponHModelController,root_section, 0, 1,0);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Come,(ONE_FIXED>>1),0);

	}
}

void WristBlade_Unreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Go,ONE_FIXED>>1);
		PlayersWeaponHModelController.Looped=0;
	}
}

void StrikeTime(int time) {

	char mbuf[128];

	if (time<1) {
		sprintf(mbuf,"STRIKETIME IS %d\n",WBStrikeTime);
		NewOnScreenMessage(mbuf);	
		return;
	}
	
	sprintf(mbuf,"STRIKETIME %d -> %d\n",WBStrikeTime,time);
	NewOnScreenMessage(mbuf);	

	TemplateWeapon[WEAPON_PRED_WRISTBLADE].TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]=
		(DIV_FIXED(WEAPONSTATE_INITIALTIMEOUTCOUNT,time));
	WBStrikeTime=time;

}

void GoGoGadgetCudgelPrimaryAttackAnimation(void) {

	/* Standard_Fire is the default. */

	if ((FastRandom()&65535)<21645) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=0;
			} else {
				StaffAttack=1;
			}	
			return;
		}
	}

	if ((FastRandom()&65535)<32767) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Secondary_Fire)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Secondary_Fire,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=2;
			} else {
				StaffAttack=3;
			}	
			return;
		}
	}

	/* Still here? Use default. */
	
	InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,-1,0);
	if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
		StaffAttack=0;
	} else {
		StaffAttack=1;
	}	

}

void Cudgel_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Let's cut'n'paster the wristblade code. */
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		GoGoGadgetCudgelPrimaryAttackAnimation();

	} else {
		if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Standard_Fire)
			&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Secondary_Fire)) {
			/* Something's changing state without resetting the timer... */
			GoGoGadgetCudgelPrimaryAttackAnimation();
		}

		if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
			/* End state. */
			weaponPtr->StateTimeOutCounter = 0;
			StaffAttack=-1;
		} else {
			/* Maintain attack. */
			weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

		}
		
		{
			/*  In the middle. */
			/* Execute attack. */
			int hits;
		
			hits=0;
		
			if (PlayersWeaponHModelController.keyframe_flags&1) {
				hits++;
			}
		
			if (hits) {
		
				enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
		
				MeleeWeapon_90Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);
				PlayCudgelSound();
				HtoHStrikes++;
			}
		}	
	}

}

void GoGoGadgetWristbladePrimaryAttackAnimation(void) {

	/* Attack_Jab is the default. */

	if ((FastRandom()&65535)<21645) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Attack_Primary)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=6;
			} else {
				StaffAttack=7;
			}	
			return;
		}
	}

	if ((FastRandom()&65535)<32767) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=8;
			} else {
				StaffAttack=9;
			}	
			return;
		}
	}

	/* Still here? Use default. */
	
	InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Jab,-1,0);
	if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
		StaffAttack=0;
	} else {
		StaffAttack=1;
	}	

}

void WristBlade_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	#if 0
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		if ((FastRandom()&65536)<32767) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,WBStrikeTime,0);
		} else {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,WBStrikeTime,0);
			//InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary,WBStrikeTime,0);
		}
	}

	{
		/* Execute attack. */
		int hits;

		hits=0;

		if (PlayersWeaponHModelController.keyframe_flags&1) {
			hits++;
		}

		if (hits) {

			enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;

			MeleeWeapon_90Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);
			PlayPredSlashSound();

		}

	}
	#else
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		GoGoGadgetWristbladePrimaryAttackAnimation();

	} else {
		if ((PlayersWeaponHModelController.Sub_Sequence!=PHSS_Attack_Jab)
			&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Attack_Primary)
			&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Attack_Secondary)) {
			/* Something's changing state without resetting the timer... */
			GoGoGadgetWristbladePrimaryAttackAnimation();
		}

		if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
			/* End state. */
			weaponPtr->StateTimeOutCounter = 0;
			StaffAttack=-1;
		} else {
			/* Maintain attack. */
			weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

		}
		
		{
			//enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
			/*  In the middle. */
			/* Execute attack. */
			int hits;
		
			hits=0;
		
			if (PlayersWeaponHModelController.keyframe_flags&1) {
				hits++;
			}
		
			if (hits) {
		
				enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
		
				MeleeWeapon_90Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);
				PlayPredSlashSound();
				HtoHStrikes++;
			}
		}	
	}
	#endif

}

void WristBlade_Strike_Secondary(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		if ((FastRandom()&65536)<32767) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=2;
			} else {
				StaffAttack=3;
			}	
		} else {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary,-1,0);
			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=4;
			} else {
				StaffAttack=5;
			}	
		}

	} else {
		if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
			/* End state. */
			weaponPtr->StateTimeOutCounter = 0;
			StaffAttack=-1;
		} else {
			/* Maintain attack. */
			weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

		}
		
		{
			//enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
			/*  In the middle. */
			/* Execute attack. */
			int hits;
		
			hits=0;
		
			if (PlayersWeaponHModelController.keyframe_flags&1) {
				hits++;
			}
		
			if (hits) {
		
				enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].SecondaryAmmoID;
		
				MeleeWeapon_90Degree_Front_Core(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);
				PlayPredSlashSound();
				HtoHStrikes++;
		
		}
	
	}
	}

}

void PredPistol_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Come,-1);
		PlayersWeaponHModelController.Looped=0;

	} else if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		/* End state. */
		weaponPtr->StateTimeOutCounter = 0;

	} else {

		/*  In the middle. */
		weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

	}

}

void PredPistol_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Go,-1,0);
		PlayersWeaponHModelController.Looped=0;
	
	} else if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		/* End state. */
		weaponPtr->StateTimeOutCounter = 0;

	} else {

		/*  In the middle. */
		weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

	}

}

void PredPistol_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
	}

	/* Are we running? */

	#if 1
	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
	#else
	{
	#endif
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

	Flamethrower_Timer=0;

}

void PredPistol_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,ONE_FIXED,1);
	}

}

int PlayerFireFlameThrower(PLAYER_WEAPON_DATA *weaponPtr) {

	VECTORCH *firingpos;

	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
   	int oldAmmoCount;
    
	ProveHModel(&PlayersWeaponHModelController,&PlayersWeapon);
    {
    	oldAmmoCount=weaponPtr->PrimaryRoundsRemaining>>16;
		/* ammo is in 16.16. we want the integer part, rounded up */
		if ( (weaponPtr->PrimaryRoundsRemaining&0xffff)!=0 ) oldAmmoCount+=1;
	}
		
	   	
   	{
   	   	/* theoretical number of bullets fired each frame, as a 16.16 number */
   	   	int bulletsToFire=MUL_FIXED(twPtr->FiringRate,NormalFrameTime);

    	if (bulletsToFire<weaponPtr->PrimaryRoundsRemaining)
    	{
    		weaponPtr->PrimaryRoundsRemaining -= bulletsToFire;	
       	}
        else /* end of magazine */
        {
           	weaponPtr->PrimaryRoundsRemaining=0;	
        }
    }

    {
    	int bulletsFired;
    	int newAmmoCount=weaponPtr->PrimaryRoundsRemaining>>16;
			/* ammo is in 16.16. we want the integer part, rounded up */
			if ( (weaponPtr->PrimaryRoundsRemaining&0xffff)!=0 ) newAmmoCount+=1;
        
        bulletsFired = oldAmmoCount-newAmmoCount;

	}

	CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,NormalFrameTime);

	if ((PWMFSDP)&&(PlayersTarget.Distance>2500)) {	//Was 1700
		VECTORCH null_vec={0,0,0};
	
		textprint("Hierarchical Flamethrower Fire!\n");
	
		firingpos=&PWMFSDP->World_Offset;
		
		FireFlameThrower(firingpos,&null_vec,&PlayersWeapon.ObMat,0, &Flamethrower_Timer);

	} else {
		#if 0
		firingpos=&CentreOfMuzzleOffset;
		FireFlameThrower(&PlayersWeapon.ObWorld,firingpos,&PlayersWeapon.ObMat,1, &Flamethrower_Timer);
		#else
		VECTORCH Firing_Position;
		VECTORCH null_vec={0,0,0};
		int lerp;

		/* Find a better place to fire from. */
		Firing_Position=PlayersWeapon.ObWorld;

		Firing_Position.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat31,200); //z: 300?
		Firing_Position.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat32,200); //z: 300?
		Firing_Position.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat33,200); //z: 300?

		Firing_Position.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat21,50); //y: 150?
		Firing_Position.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat22,50); //y: 150?
		Firing_Position.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat23,50); //y: 150?

		Firing_Position.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat11,200); //x: 250? 
		Firing_Position.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat12,200); //x: 250?
		Firing_Position.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat13,200); //x: 250?
		/* Interpolation... */
		if (PWMFSDP) {
			lerp=PlayersTarget.Distance-1600;
			if (lerp<0) {
				lerp=0;
			}
			lerp=MUL_FIXED(lerp,ONE_FIXED);
			lerp=DIV_FIXED(lerp,(2500-1600));

			textprint("lerp %d\n",lerp);

			Firing_Position.vx=MUL_FIXED((ONE_FIXED-lerp),Firing_Position.vx);
			Firing_Position.vy=MUL_FIXED((ONE_FIXED-lerp),Firing_Position.vy);
			Firing_Position.vz=MUL_FIXED((ONE_FIXED-lerp),Firing_Position.vz);

			Firing_Position.vx+=MUL_FIXED(lerp,PWMFSDP->World_Offset.vx);
			Firing_Position.vy+=MUL_FIXED(lerp,PWMFSDP->World_Offset.vy);
			Firing_Position.vz+=MUL_FIXED(lerp,PWMFSDP->World_Offset.vz);
		}

		FireFlameThrower(&Firing_Position,&null_vec,&PlayersWeapon.ObMat,0, &Flamethrower_Timer);
		#endif
	}


	return(1);

}

#define ALWAYS_EXIT_WOUNDS 1

DISPLAYBLOCK *CauseDamageToHModel(HMODELCONTROLLER *HMC_Ptr, STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, SECTION_DATA *this_section_data,VECTORCH *incoming, VECTORCH *position, int FromHost) {
	
	DAMAGEBLOCK tempdamage;
	int healthdamage,armourdamage;
	int wounds;
	DISPLAYBLOCK *frag;

	LOCALASSERT(HMC_Ptr==this_section_data->my_controller);

	frag=NULL;

	if ((FromHost==0)&&(sbPtr->I_SBtype==I_BehaviourNetGhost)) {
		DamageNetworkGhost(sbPtr, damage, multiple,this_section_data,incoming);
		return(NULL);
	}

	if (sbPtr->I_SBtype==I_BehaviourDummy) {
		/* Dummys are INDESTRUCTIBLE. */
		return(NULL);
	}

	if(sbPtr->I_SBtype==I_BehaviourMarinePlayer ||
	   sbPtr->I_SBtype==I_BehaviourAlienPlayer ||
	   sbPtr->I_SBtype==I_BehaviourPredatorPlayer)
	{
		//check for player invulnerability
		PLAYER_STATUS *psPtr=(PLAYER_STATUS*)sbPtr->SBdataptr;
		GLOBALASSERT(psPtr);
		if(psPtr->invulnerabilityTimer>0)
		{
			//player is still invulnerable
			return(NULL);
		}
	}

	wounds=0;

	/* Pass damage up test? */
	if ( (this_section_data->sempai->flags&section_flag_passdamagetoparent)
		&&(this_section_data->My_Parent)) {
		/* Try again, a level up. */
		return(CauseDamageToHModel(HMC_Ptr,sbPtr,damage,multiple,this_section_data->My_Parent,incoming,position,FromHost));
	}

	/* The section takes damage... and the SB takes the health and armour damage. */
	tempdamage.Health=this_section_data->current_damage.Health;
	tempdamage.Armour=this_section_data->current_damage.Armour;
	DamageDamageBlock(&this_section_data->current_damage,damage,multiple);
	if (this_section_data->sempai->flags&section_flag_doesnthurtsb) {
		healthdamage=0;
		armourdamage=0;
	} else {
		healthdamage=tempdamage.Health-this_section_data->current_damage.Health;
		armourdamage=tempdamage.Armour-this_section_data->current_damage.Armour;
	}

	if (armourdamage>0) {
		if (sbPtr->SBDamageBlock.Armour<armourdamage) sbPtr->SBDamageBlock.Armour=0;
		else sbPtr->SBDamageBlock.Armour-=armourdamage;
	}

	if (healthdamage>0) {
		if (sbPtr->SBDamageBlock.Health<healthdamage) sbPtr->SBDamageBlock.Health=0;
		else sbPtr->SBDamageBlock.Health-=healthdamage;
	}
	
	/* Consider networking. */
	if (AvP.Network != I_No_Network)
	{
		if (sbPtr->I_SBtype!=I_BehaviourNetGhost) {
			AddNetMsg_GhostHierarchyDamaged(sbPtr,damage,multiple,this_section_data->sempai->IDnumber,incoming);
		}
	}

	#if ALWAYS_EXIT_WOUNDS
	/* otherwise, exit wounds! */
	if ((incoming)&&(this_section_data->sempai->flags&section_sprays_anything)
		&&(damage->MakeExitWounds)) {
		enum PARTICLE_ID blood_type;
		int a;
		VECTORCH *startpos;
		VECTORCH final_spray_direction;
	
		if (this_section_data->sempai->flags&section_sprays_blood) {
			blood_type=GetBloodType(sbPtr);
			/* Er... default? */
		} else if (this_section_data->sempai->flags&section_sprays_acid) {
			blood_type=PARTICLE_ALIEN_BLOOD;
		} else if (this_section_data->sempai->flags&section_sprays_predoblood) {
			blood_type=PARTICLE_PREDATOR_BLOOD;
		} else if (this_section_data->sempai->flags&section_sprays_sparks) {
			blood_type=PARTICLE_SPARK;
		} else {
			blood_type=PARTICLE_FLAME;
			/* Distinctive. */
		}

		if (position) {
			startpos=position;
		} else {
			startpos=&this_section_data->World_Offset;
		}
		/* 'incoming' SHOULD be normalised. */
		for (a=0; a<(multiple>>ONE_FIXED_SHIFT); a++) {
			
			RotateAndCopyVector(incoming,&final_spray_direction,&sbPtr->DynPtr->OrientMat);
	
			/* Scale down. */
			
			//final_spray_direction.vx>>=1;
			//final_spray_direction.vy>>=1;
			//final_spray_direction.vz>>=1;
			
			/* Add random element. */
			
			final_spray_direction.vx+=( (FastRandom()&511)-256);
			final_spray_direction.vy+=( (FastRandom()&511)-256);
			final_spray_direction.vz+=( (FastRandom()&511)-256);

			MakeParticle(startpos, &final_spray_direction, blood_type);
		}
	}
	#endif

	if (this_section_data->current_damage.Health<=0) {
		/* Might want to create a frag here? */
		if (damage->BlowUpSections) {

			VECTORCH blastCentre;
			/* Deduce blastCentre. */
			blastCentre=this_section_data->World_Offset;
			
			if (incoming) {
				blastCentre.vx+=incoming->vx;
				blastCentre.vy+=incoming->vy;
				blastCentre.vz+=incoming->vz;
			}
			
			/* For the moment, this ALWAYS has priority. */
			Pop_Section(sbPtr,this_section_data,&blastCentre,&wounds);

			if (wounds&section_has_sparkoflife) {
				/* Kill SB off! */
				sbPtr->SBDamageBlock.Health=0;
				/* That's the way... */
			}

		} else if ( ((this_section_data->sempai->flags&section_is_master_root)==0) 
			&&((this_section_data->sempai->flags&section_flag_never_frag)==0)
			&&(((this_section_data->sempai->flags&section_sprays_acid)&&((this_section_data->sempai->flags&section_flag_fragonlyfordisks)==0))
				||((this_section_data->sempai->StartingStats.Health<TotalKineticDamage(damage))&&((this_section_data->sempai->flags&section_flag_fragonlyfordisks)==0))
				||((damage->Slicing>2)&&(this_section_data->sempai->flags&section_flag_fragonlyfordisks))
				||((damage->Slicing>0)&&((this_section_data->sempai->flags&section_flag_fragonlyfordisks)==0))
			)
			){

			MATRIXCH *orientptr;
			{
				/* Work out which orientation to use. */
				if (this_section_data->My_Parent==NULL) {
					/* The root (Gah!), so use sbPtr. */
					orientptr=&sbPtr->DynPtr->OrientMat;
				} else {
					/* Use parent. */
					orientptr=&this_section_data->My_Parent->SecMat;
				}
			}
			
			/* Never frag off the root, it wouldn't make sense. */
			
			frag=MakeHierarchicalDebris(sbPtr,this_section_data, &this_section_data->World_Offset, orientptr,&wounds,3);
			
			/* Oh Dear!  Every section below and including this one becomes... unreal. 
			And if any of them contain the spark of life, we need to know. */
			
			if (wounds&section_has_sparkoflife) {
				/* Kill SB off! */
				sbPtr->SBDamageBlock.Health=0;
				/* That's the way... */
			}
		} else {
			/* Don't frag the master root, and take care with non-aliens. */
			if (this_section_data->sempai->flags&section_has_sparkoflife) {
				/* However, if the master root is destroyed and is a critical section, */
				sbPtr->SBDamageBlock.Health=0;
				/* ...still kill them off. */
			}
			#if (ALWAYS_EXIT_WOUNDS==0)
			/* otherwise, exit wounds! */
			if ((incoming)&&(this_section_data->sempai->flags&section_sprays_anything)
				&&(damage->MakeExitWounds)) {
				enum PARTICLE_ID blood_type;
				int a;
				VECTORCH *startpos;
				VECTORCH final_spray_direction;
			
				if (this_section_data->sempai->flags&section_sprays_blood) {
					blood_type=GetBloodType(sbPtr);
					/* Er... default? */
				} else if (this_section_data->sempai->flags&section_sprays_acid) {
					blood_type=PARTICLE_ALIEN_BLOOD;
				} else if (this_section_data->sempai->flags&section_sprays_predoblood) {
					blood_type=PARTICLE_PREDATOR_BLOOD;
				} else if (this_section_data->sempai->flags&section_sprays_sparks) {
					blood_type=PARTICLE_SPARK;
				} else {
					blood_type=PARTICLE_FLAME;
					/* Distinctive. */
				}

				if (position) {
					startpos=position;
				} else {
					startpos=&this_section_data->World_Offset;
				}
				/* 'incoming' SHOULD be normalised. */
				for (a=0; a<(multiple>>ONE_FIXED_SHIFT); a++) {
					
					RotateAndCopyVector(incoming,&final_spray_direction,&sbPtr->DynPtr->OrientMat);
		
					/* Scale down. */
					
					//final_spray_direction.vx>>=1;
					//final_spray_direction.vy>>=1;
					//final_spray_direction.vz>>=1;
					
					/* Add random element. */
					
					final_spray_direction.vx+=( (FastRandom()&511)-256);
					final_spray_direction.vy+=( (FastRandom()&511)-256);
					final_spray_direction.vz+=( (FastRandom()&511)-256);

					MakeParticle(startpos, &final_spray_direction, blood_type);
				}
			}
			#endif
		}
	}

	/* There... */

 	switch(sbPtr->I_SBtype)
	{
		case I_BehaviourAlien:
		{
   	 	/* reduce alien health */
			AlienIsDamaged(sbPtr, damage, multiple, wounds,this_section_data,incoming,frag);
			break;
		}
		case I_BehaviourMarinePlayer:
		case I_BehaviourAlienPlayer:
		case I_BehaviourPredatorPlayer:
		{
			PlayerIsDamaged(sbPtr,damage,multiple,incoming);
			break;
		}
		case I_BehaviourInanimateObject:
		{
			InanimateObjectIsDamaged(sbPtr,damage,multiple);
			break;
		}
		case I_BehaviourPredator:
		{
			PredatorIsDamaged(sbPtr,damage,multiple,this_section_data,incoming);
			break;
		}
		case I_BehaviourXenoborg:
		{
			XenoborgIsDamaged(sbPtr, damage, multiple, wounds,incoming);
			break;
		}
		case I_BehaviourSeal:
		case I_BehaviourMarine:
		{
			MarineIsDamaged(sbPtr,damage,multiple,wounds,this_section_data,incoming);
			break;
		}
		case I_BehaviourQueenAlien:
		{
			QueenIsDamaged(sbPtr,damage,multiple,this_section_data,incoming,position);
			break;
		}
		case I_BehaviourPredatorAlien:
		{
			GLOBALASSERT(0);
			//PAQIsDamaged(sbPtr,damage,multiple);
			break;
		}	    
		case I_BehaviourFaceHugger:
		{
			FacehuggerIsDamaged(sbPtr,damage,multiple);
			break;
		}	    
		#if 0
		/* Whoa, positive feedback! */
		case I_BehaviourNetGhost:
		{
			DamageNetworkGhost(sbPtr, damage, multiple,this_section_data,incoming);
			break;
		}
		#endif
		case I_BehaviourAutoGun:
		{
			AGunIsDamaged(sbPtr, damage, multiple, wounds,incoming);
			break;
		}
		case I_BehaviourNetCorpse:
		{
			CorpseIsDamaged(sbPtr,damage,multiple,wounds,this_section_data,incoming);
			break;
		}
		default:
			break;
	}

	return(frag);
}

VECTORCH HitAreaArray[HAM_end] = {
	{0,0,0},
	{0,0,1000},
	{0,0,-1000},
	{0,-1000,0},
	{0,1000,0},
	{-1000,0,0},
	{1000,0,0},
	{-1000,-1000,0},
	{1000,-1000,0},
	{-1000,1000,0},
	{1000,1000,0},	
};

VECTORCH Local_HitAreaArray[HAM_end];
HITAREAMATRIX HitZone,HitAspect;

void FindHitArea(DISPLAYBLOCK *dptr) {

	int a;
	MATRIXCH LtoV; 
	int nearest,neardist,dist;
	int fbnearest,fbneardist;

	MatrixMultiply(&Global_VDB_Ptr->VDB_Mat,&dptr->ObMat,&LtoV);

	nearest=-1;
	fbnearest=-1;
	neardist=1000000;
	fbneardist=1000000;

	for (a=0; a<HAM_end; a++) {
		RotateAndCopyVector(&HitAreaArray[a],&Local_HitAreaArray[a],&LtoV);

		Local_HitAreaArray[a].vx+=dptr->ObView.vx;
		Local_HitAreaArray[a].vy+=dptr->ObView.vy;
		Local_HitAreaArray[a].vz+=dptr->ObView.vz;


		dist=Approximate3dMagnitude(&Local_HitAreaArray[a]);

		if ( (a!=HAM_Front) && (a!=HAM_Back) ) {
			if (dist<neardist) {
				nearest=a;
				neardist=dist;
			}
		} 
		if ( (a==HAM_Front) || (a==HAM_Back) || (a==HAM_Centre) ) {
			if (dist<fbneardist) {
				fbnearest=a;
				fbneardist=dist;
			}
		}
	}

	LOCALASSERT(nearest!=-1);
	LOCALASSERT( (fbnearest==HAM_Front) || (fbnearest==HAM_Back) || (fbnearest==HAM_Centre) );

	HitZone=nearest;
	HitAspect=fbnearest;

	switch(nearest){
		case HAM_Centre:
			textprint("Nearest = Centre\n");
			break;
		case HAM_Top:
			textprint("Nearest = Top\n");
			break;
		case HAM_Base:
			textprint("Nearest = Base\n");
			break;
		case HAM_Left:
			textprint("Nearest = Left\n");
			break;
		case HAM_Right:
			textprint("Nearest = Right\n");
			break;
		case HAM_TopLeft:
			textprint("Nearest = TopLeft\n");
			break;
		case HAM_TopRight:
			textprint("Nearest = TopRight\n");
			break;
		case HAM_BaseLeft:
			textprint("Nearest = BaseLeft\n");
			break;
		case HAM_BaseRight:
			textprint("Nearest = BaseRight\n");
			break;
		default:
			GLOBALASSERT(0);
			break;
	}
	switch (fbnearest) {
		case HAM_Centre:
			textprint("Aspect Centre\n");
			break;
		case HAM_Front:
			textprint("Aspect Front\n");
			break;
		case HAM_Back:
			textprint("Aspect Back\n");
			break;
	}

}

HITLOCATIONTABLE *GetThisHitLocationTable(char *id) {
	
	int a;
	extern HITLOCATIONTABLE Global_Hitlocation_Tables[];

	a=0;
	while (Global_Hitlocation_Tables[a].id!=NULL) {
		if (strcmp(id,Global_Hitlocation_Tables[a].id)==0) {
			return(&Global_Hitlocation_Tables[a]);
		}
		a++;
	}
	return(NULL);
}

HITLOCATIONTABLEENTRY *Get_Sublocation(STRATEGYBLOCK *sbPtr) {

	HITLOCATIONTABLE *hltable;
	HITLOCATIONTABLEENTRY *subtable,*entry;
	int dice;

	/* Identify table... */
	switch (sbPtr->I_SBtype) {
		case I_BehaviourMarine:
		case I_BehaviourSeal:
			{
				/* Get hierarchy name... */
				MARINE_STATUS_BLOCK *marineStatusPointer;    
				
				LOCALASSERT(sbPtr);
				marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				LOCALASSERT(marineStatusPointer);	          		
				
				hltable=GetThisHitLocationTable(marineStatusPointer->My_Weapon->HitLocationTableName);			
			}
			break;
		case I_BehaviourAlien:
			{
				ALIEN_STATUS_BLOCK *alienStatusPointer;    
				
				LOCALASSERT(sbPtr);
				alienStatusPointer = (ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				LOCALASSERT(alienStatusPointer);	          		
				
				switch (alienStatusPointer->Type) {
					case AT_Standard:
					default:
						hltable=GetThisHitLocationTable("alien");
						break;
					case AT_Predalien:
						hltable=GetThisHitLocationTable("predalien");
						break;
					case AT_Praetorian:
						hltable=GetThisHitLocationTable("praetorian");
						break;
				}
			}
			break;
		case I_BehaviourPredator:
			{
				/* Get hierarchy name... */
				PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
				
				LOCALASSERT(sbPtr);
				predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				LOCALASSERT(predatorStatusPointer);	          		
				
				hltable=GetThisHitLocationTable(predatorStatusPointer->Selected_Weapon->HitLocationTableName);
			}
			break;
		case I_BehaviourXenoborg:
			hltable=GetThisHitLocationTable("xenoborg");
			break;
		case I_BehaviourAutoGun:
			hltable=GetThisHitLocationTable("sentrygun");
			break;
		case I_BehaviourNetCorpse:
			{
				NETCORPSEDATABLOCK *corpseDataPtr;    
				
				LOCALASSERT(sbPtr);
				corpseDataPtr = (NETCORPSEDATABLOCK *)(sbPtr->SBdataptr);    
				LOCALASSERT(corpseDataPtr);

				hltable=corpseDataPtr->hltable;
				/* Special corpse case. */
				HitZone=HAM_Centre;
				HitAspect=HAM_Centre;
			}
			break;
		case I_BehaviourNetGhost :
			{
				NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
				hltable=ghostData->hltable;

				if(ghostData->type==I_BehaviourNetCorpse)	
				{
					//special corpse case here as well , I should imagine
					HitZone=HAM_Centre;
					HitAspect=HAM_Centre;
				}
				
			}
			break;
		default:
			textprint("No hit table!\n");
			hltable=NULL;
			/* See ChrisF */
			break;
	}

	if (hltable==NULL) {
		/* Hey ho... */
		return(NULL);
	}

	/* Now the fun bit. */

	dice=FastRandom()&65535;

	switch (HitZone) {
		case HAM_Centre:
			subtable=hltable->CentreLocs;
			break;
		case HAM_Top:
			subtable=hltable->TopLocs;
			break;
		case HAM_Base:
			subtable=hltable->BaseLocs;
			break;
		case HAM_Left:
			subtable=hltable->LeftLocs;
			break;
		case HAM_Right:
			subtable=hltable->RightLocs;
			break;
		case HAM_TopLeft:
			subtable=hltable->TopLeftLocs;
			break;
		case HAM_TopRight:
			subtable=hltable->TopRightLocs;
			break;
		case HAM_BaseLeft:
			subtable=hltable->BaseLeftLocs;
			break;
		case HAM_BaseRight:
			subtable=hltable->BaseRightLocs;
			break;
		default:
			GLOBALASSERT(0);
			break;
	}

	/* Now, get location. */

	entry=subtable;

	while (entry->section_name!=NULL) {
		if (dice<entry->cprob) {
			if (!( ( (HitAspect==HAM_Front)&&(entry->aspect&back_aspect) )
				||( (HitAspect==HAM_Back)&&(entry->aspect&front_aspect) ) )) {
				/* Okay! */
				break;
			}
		}
		dice-=entry->cprob;
		entry++;
	}

	return(entry);
}

SECTION_DATA *HitLocationRoll(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *source) {

	HITLOCATIONTABLEENTRY *entry;

	if (sbPtr->SBdptr==NULL) {
		/* Far case?  Hey ho... */
		return(NULL);
	}
	
	if (sbPtr->SBdptr->HModelControlBlock==NULL) {
		/* Not a hierarchy. */
		return(NULL);
	}

	if ((source==NULL) || (source==Player->ObStrategyBlock)) {
		/* Why not?  This shouldn't get called, really... */
		FindHitArea(sbPtr->SBdptr);
	} else {
		HitZone=HAM_Top;
		HitAspect=HAM_Centre;
	}

	entry=Get_Sublocation(sbPtr);

	if (entry==NULL) {
		/* Failure! */
		return(NULL);
	}

	if (entry->section_name) {
		/* Valid hit. */
		SECTION_DATA *target;
		target=GetThisSectionData(sbPtr->SBdptr->HModelControlBlock->section_data,entry->section_name);

		if (target) {
			/* Success! */
			return(target);
		}
	}

	/* Failure! */
	return(NULL);
}

DISPLAYBLOCK *HtoHDamageToHModel(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, STRATEGYBLOCK *source, VECTORCH *attack_dir) {

	HITLOCATIONTABLEENTRY *entry;

	if (sbPtr->SBdptr==NULL) {
		/* Far case?  Hey ho... */
		CauseDamageToObject(sbPtr, damage, multiple,attack_dir);
		return(NULL);
	}
	
	LOCALASSERT(sbPtr->SBdptr->HModelControlBlock);
	
	if ((source==NULL) || (source==Player->ObStrategyBlock)) {
		FindHitArea(sbPtr->SBdptr);
	} else {
		HitZone=HAM_Top;
		HitAspect=HAM_Centre;
	}
	
	entry=Get_Sublocation(sbPtr);

	if (entry==NULL) {
		/* Failure! */
		CauseDamageToObject(sbPtr, damage, multiple,attack_dir);
		return(NULL);
	}

	if (entry->section_name) {
		/* Valid hit. */
		SECTION_DATA *target;
		target=GetThisSectionData(sbPtr->SBdptr->HModelControlBlock->section_data,entry->section_name);

		if (target) {
			/* Success! */
			DISPLAYBLOCK *frag;
			textprint("Damaged %s!\n",entry->section_name);
			frag=CauseDamageToHModel(sbPtr->SBdptr->HModelControlBlock, sbPtr, damage, multiple, target,attack_dir,NULL,0);
			return(frag);
		}
	}

	/* Failure!  Never mind. */
	
	CauseDamageToObject(sbPtr, damage, multiple,attack_dir);
	return(NULL);
}

void AlienClaw_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_AlienHUD,(int)AHSS_LeftSwipeDown,ONE_FIXED/3);
		PlayersWeaponHModelController.Looped=0;
		PlayersWeaponHModelController.Playing=0;
	}

	LastHand=0;
	Alien_Visible_Weapon=0; //Claws
}

void AlienClaw_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* ...Nothing? */

}

void AlienClaw_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* ...Nothing? */

	if ( (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor==0) 
		&& (Player->ObStrategyBlock->DynPtr->TimeNotInContactWithFloor<=0) ) {
		/* Jumping or falling... */
		TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=0;
	} else {
		TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=1;
	}

	if (Alien_Visible_Weapon==1) {
		/* Correct tail idle position. */
		DELTA_CONTROLLER *XDelta,*YDelta;

		GLOBALASSERT(Alien_Visible_Weapon==1);

		XDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"XDelta");
		YDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"YDelta");

		XDelta->timer=32767;
		YDelta->timer=32767;
	}

}

void AlienClaw_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Look for surplus claw keyframe flags. */
	{
		/* Execute attack. */
		int hits;

		hits=0;

		if (PlayersWeaponHModelController.keyframe_flags&1) {
			hits++;
		}

		if (PlayersWeaponHModelController.keyframe_flags&2) {
			hits++;
		}

		if (hits) {

			HtoHStrikes+=hits;

			MeleeWeapon_180Degree_Front_Core(&Player_Weapon_Damage,ONE_FIXED*hits,4000);
			PlayAlienSwipeSound();

		}
	}

	if ( (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor==0) 
		&& (Player->ObStrategyBlock->DynPtr->TimeNotInContactWithFloor<=0) ) {
		/* Jumping or falling... */
		TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=0;
	} else {
		TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=1;
	}

	if (Alien_Visible_Weapon==1) {
		/* Correct tail idle position. */
		DELTA_CONTROLLER *XDelta,*YDelta;

		GLOBALASSERT(Alien_Visible_Weapon==1);

		XDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"XDelta");
		YDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"YDelta");

		XDelta->timer=32767;
		YDelta->timer=32767;
	}

}

void FixAlienStrikeSpeed(int time) {

	ACStrikeTime=MUL_FIXED(time,AC_Speed_Factor);

	TemplateWeapon[WEAPON_ALIEN_CLAW].TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]=
		(DIV_FIXED(WEAPONSTATE_INITIALTIMEOUTCOUNT,ACStrikeTime));

}

void AlienStrikeTime(int time) {

	if (time<1) return;
	
	AC_Speed_Factor=time;

}

void WristBlade_WindUp(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
			(int)PHSS_PullBack,-1,0);

		/* Setup base damage. */
		Player_Weapon_Damage=TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty];
		Player_Weapon_Damage.Special=0;

		Alien_Tail_Clock=0;
		Wristblade_StrikeType=0;

		TemplateWeapon[WEAPON_PRED_WRISTBLADE].SecondaryIsAutomatic=1;

	} else if ((PlayersWeaponHModelController.Tweening==Controller_NoTweening)
		&&(PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1))) {

		/* Finished poise move. */
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
			(int)PHSS_Hold,-1,1);

		Alien_Tail_Clock=-1;

	} else if (Alien_Tail_Clock>=0) {

		int flags,a;

		Alien_Tail_Clock+=NormalFrameTime;

		/* In the move... */
		flags=PlayersWeaponHModelController.keyframe_flags;
		
		for (a=0; a<6; a++) {
			/* Gotta be less than six! */
			if (flags&1) {
				Wristblade_StrikeType++;
			}
			flags>>=1;
		}
		
	} else {
		/* Windup finished and holding... timeout? */
		Wristblade_StrikeType=-1;
		GLOBALASSERT(Alien_Tail_Clock<0);
		Alien_Tail_Clock-=NormalFrameTime;
		if (Alien_Tail_Clock<-(ONE_FIXED<<2)) {
			/* Time out? */
			if (PlayersWeaponHModelController.Sub_Sequence!=PHSS_PullBack) {
				InitHModelTweening_Backwards(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
					(int)PHSS_PullBack,-1,0);
			} else {
				/* In the timeout... */
				if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
					/* End fire, somehow. */
					weaponPtr->CurrentState = WEAPONSTATE_RECOIL_SECONDARY;
					weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
					return;
				}
			}
		}
	}

}

void ThrowSecondaryStrongStrike(void) {

	int attack=-1;	

	if ((FastRandom()&65535)<32767) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
			(int)PHSS_Attack_Secondary_Strong_One)) {
			attack=0;
		} else {
			attack=1;
		}
	} else {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
			(int)PHSS_Attack_Secondary_Strong_Two)) {
			attack=1;
		} else {
			attack=0;
		}
	}

	switch (attack) {
		default:
		case 0:
			GLOBALASSERT(HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
				 (int)PHSS_Attack_Secondary_Strong_One));
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
				(int)PHSS_Attack_Secondary_Strong_One,-1,0);

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=2;
			} else {
				StaffAttack=3;
			}
			break;
		case 1:
			GLOBALASSERT(HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
				 (int)PHSS_Attack_Secondary_Strong_Two));
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
				(int)PHSS_Attack_Secondary_Strong_Two,-1,0);

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=2;
			} else {
				StaffAttack=3;
			}
			break;
	}
}

void ThrowSecondaryWeakStrike(void) {

	int attack=-1;	

	if ((FastRandom()&65535)<32767) {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
			(int)PHSS_Attack_Secondary_Weak_One)) {
			attack=0;
		} else {
			attack=1;
		}
	} else {
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
			(int)PHSS_Attack_Secondary_Weak_Two)) {
			attack=1;
		} else {
			attack=0;
		}
	}

	switch (attack) {
		default:
		case 0:
			GLOBALASSERT(HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
				 (int)PHSS_Attack_Secondary_Weak_One));
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
				(int)PHSS_Attack_Secondary_Weak_One,-1,0);

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=4;
			} else {
				StaffAttack=5;
			}
			break;
		case 1:
			GLOBALASSERT(HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,
				 (int)PHSS_Attack_Secondary_Weak_Two));
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,
				(int)PHSS_Attack_Secondary_Weak_Two,-1,0);

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				StaffAttack=4;
			} else {
				StaffAttack=5;
			}
			break;
	}
	
}

void WristBlade_WindUpStrike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Eek!  Well, fire must have been released at this stage. */

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_PredatorHUD);

	if (PlayersWeaponHModelController.Tweening!=Controller_NoTweening) {
		/* I don't wanna know right now... */
		GLOBALASSERT(PlayersWeaponHModelController.Playing);
		return;

	}
	
	/* Maintain this state... */
	weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);
	
	if (PlayersWeaponHModelController.Sub_Sequence==PHSS_PullBack) {
		int flag,multiplyer;

		if (PlayersWeaponHModelController.Reversed) {
			/* In the timeout. */
			/* Stop quick refire... */
			TemplateWeapon[WEAPON_PRED_WRISTBLADE].SecondaryIsAutomatic=0;
			/* Wait, then end. */
			if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
				/* End state. */
				weaponPtr->StateTimeOutCounter = 0;
				StaffAttack=-1;
			}
			return;
		}

		/* Still in the pull back... wait for the right flag. */
		flag=(1<<Wristblade_StrikeType);

		if (PlayersWeaponHModelController.keyframe_flags&flag) {
			/* Got it.  Fire that attack. */
			switch (Wristblade_StrikeType) {
				case -1:
					multiplyer=ONE_FIXED;
					ThrowSecondaryStrongStrike();
					Player_Weapon_Damage.Special=1;
					break;
				default:
					multiplyer=(ONE_FIXED>>1);
					ThrowSecondaryWeakStrike();
					Player_Weapon_Damage.Special=0;
					break;
			}
			Player_Weapon_Damage.Impact 	=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Impact 	,multiplyer);
			Player_Weapon_Damage.Cutting	=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Cutting	,multiplyer);
			Player_Weapon_Damage.Penetrative=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Penetrative,multiplyer);
			Player_Weapon_Damage.Fire		=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Fire		,multiplyer);
			Player_Weapon_Damage.Electrical	=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Electrical	,multiplyer);
			Player_Weapon_Damage.Acid		=MUL_FIXED(TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty].Acid		,multiplyer);

		} else if ((PlayersWeaponHModelController.Tweening==Controller_NoTweening)
			&&(PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1))) {
			/* Hit the end!  Whoops.  Fire big attack. */
			ThrowSecondaryStrongStrike();
			Player_Weapon_Damage=TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty];
			Player_Weapon_Damage.Special=1;
		}

	} else if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Hold) {
		/* In 'Held Back' stance: fire the biggest attack. */
		ThrowSecondaryStrongStrike();
		Player_Weapon_Damage=TemplateAmmo[AMMO_HEAVY_PRED_WRISTBLADE].MaxDamage[AvP.Difficulty];
		Player_Weapon_Damage.Special=1;
	} else {
		/* In the attack.  Look for flags and do the damage stuff. */
		if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
			/* End state. */
			weaponPtr->StateTimeOutCounter = 0;
			StaffAttack=-1;
		} else {
			/* Maintain attack. */
			weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);
		}

		{
			/*  In the middle. */
			/* Execute attack. */
			int hits;
		
			hits=0;
		
			if (PlayersWeaponHModelController.keyframe_flags&1) {
				hits++;
			}
		
			if (hits) {
				
				STRATEGYBLOCK *Trophy;
				SECTION_DATA *head_sec;
				enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].SecondaryAmmoID;
		

				/* Intercept 'trophy' attack. */
				if (Player_Weapon_Damage.Special) {
					Trophy=GetTrophyTarget(&head_sec);
				} else {
					Trophy=NULL;
				}
				if (Trophy) {
					/* Apply damage. */
					CauseDamageToHModel(Trophy->SBdptr->HModelControlBlock, Trophy, &TemplateAmmo[AMMO_PRED_TROPHY_KILLSECTION].MaxDamage[AvP.Difficulty],
						ONE_FIXED,head_sec,NULL,NULL,0);
					if (PlayerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
						PlayPredatorSound(0,PSC_Taunt,0,&PlayerStatusPtr->soundHandle,NULL);
						playerNoise=1;
					}
					CurrentGameStats_TrophyCollected(Trophy);
				} else {
					MeleeWeapon_90Degree_Front_Core(&Player_Weapon_Damage,ONE_FIXED,TemplateAmmo[AmmoID].MaxRange);
				}
				PlayPredSlashSound();
				HtoHStrikes++;
			}
		}	

	}

}

void AlienTail_Poise(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	GLOBALASSERT(TemplateWeapon[WEAPON_ALIEN_CLAW].SecondaryIsAutomatic);

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		DELTA_CONTROLLER *XDelta,*YDelta;

		/* Check we've got the tail. */

		if (Alien_Visible_Weapon!=1) {
					
			GetHierarchicalWeapon("alien_HUD","tail",(int)HMSQT_AlienHUD,(int)AHSS_TailCome);
			ProveHModel(&PlayersWeaponHModelController,&PlayersWeapon);

			Alien_Visible_Weapon=1;
			/* Create delta sequences. */
			XDelta=Add_Delta_Sequence(&PlayersWeaponHModelController,
				"XDelta",(int)HMSQT_AlienHUD,(int)AHSS_Hor_Delta,0);
			YDelta=Add_Delta_Sequence(&PlayersWeaponHModelController,
				"YDelta",(int)HMSQT_AlienHUD,(int)AHSS_Ver_Delta,0);
		} else {

			XDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"XDelta");
			YDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"YDelta");

		}

		XDelta->timer=32767;
		YDelta->timer=32767;

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
			(int)AHSS_TailCome,ONE_FIXED,0);

		/* Setup base damage. */
		Player_Weapon_Damage=TemplateAmmo[AMMO_ALIEN_TAIL].MaxDamage[AvP.Difficulty];
		
		Player_Weapon_Damage.Impact=0;
		Player_Weapon_Damage.Cutting=0;
		Player_Weapon_Damage.Penetrative=30;
		Player_Weapon_Damage.Fire=0;
		Player_Weapon_Damage.Electrical=0;
		Player_Weapon_Damage.Acid=0;

		Player_Weapon_Damage.BlowUpSections=0;
		Player_Weapon_Damage.Special=0;

		Alien_Tail_Target=NULL;
		COPY_NAME(Alien_Tail_Target_SBname,Null_Name);
		
		Alien_Tail_Clock=0;

	} else if ((PlayersWeaponHModelController.Tweening==Controller_NoTweening)
		&&(PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1))) {

		/* Finished poise move. */
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
			(int)AHSS_TailHold,ONE_FIXED,1);

		Alien_Tail_Clock=-1;

		textprint("Tail Max. Pen. Damage = %d\n",Player_Weapon_Damage.Penetrative);

	} else if (Alien_Tail_Clock!=-1) {

		Alien_Tail_Clock+=NormalFrameTime;

		while (Alien_Tail_Clock>=1000) {
			Player_Weapon_Damage.Penetrative+=1;
			Alien_Tail_Clock-=1000;
		}

		textprint("Tail Pen. Damage = %d\n",Player_Weapon_Damage.Penetrative);
	
	} else {
		textprint("Tail Max. Pen. Damage = %d - Poised.\n",Player_Weapon_Damage.Penetrative);
	}

}

int tail_xcal=120;
int tail_ycal=220;

void ComputeTailDeltaValues(DELTA_CONTROLLER *XDelta,DELTA_CONTROLLER *YDelta) {
	
	int temp_timer,screenX,screenY;
	VECTORCH target_pos;

	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

	GetTargetingPointOfObject(Alien_Tail_Target->SBdptr,&target_pos);
	TranslatePointIntoViewspace(&target_pos);

	screenX = WideMulNarrowDiv
				(				 			
					target_pos.vx,
					VDBPtr->VDB_ProjX,
					target_pos.vz
				);
	screenY = WideMulNarrowDiv
				(
					target_pos.vy,
					VDBPtr->VDB_ProjY,	    	  
					target_pos.vz
				);


	if (MIRROR_CHEATMODE) {
		screenX=-screenX;
	}

	temp_timer=screenX*(-tail_xcal);
	temp_timer+=32767;

	if (temp_timer<0) temp_timer=0;
	if (temp_timer>65535) temp_timer=65535;
	XDelta->timer=temp_timer;

	temp_timer=screenY*(tail_ycal);
	temp_timer+=32767;

	if (temp_timer<0) temp_timer=0;
	if (temp_timer>65535) temp_timer=65535;
	YDelta->timer=temp_timer;
	
	textprint("Target Screen X,Y %d %d\n",screenX,screenY);

}

void AlienTail_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	DELTA_CONTROLLER *XDelta,*YDelta;

	GLOBALASSERT(Alien_Visible_Weapon==1);

	XDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"XDelta");
	YDelta=Get_Delta_Sequence(&PlayersWeaponHModelController,"YDelta");

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		DISPLAYBLOCK *target;

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
			(int)AHSS_TailStrike,(ONE_FIXED/6),0);

		/* Find target. */
		target=AlienTail_TargetSelect();

		if (target) {
			Alien_Tail_Target=target->ObStrategyBlock;
			COPY_NAME(Alien_Tail_Target_SBname,Alien_Tail_Target->SBname);
			/* Set XDelta and YDelta. */
			ComputeTailDeltaValues(XDelta,YDelta);
		} else {
			XDelta->timer=32767;
			YDelta->timer=32767;
		}
		
		/* Don't bother with the next bit. */
		return;
	}

	/* Check target validity. */

	if (Validate_Target(Alien_Tail_Target,Alien_Tail_Target_SBname)==0) {
		/* Lost it somehow. */
		Alien_Tail_Target=NULL;
		COPY_NAME(Alien_Tail_Target_SBname,Null_Name);
	}

	if (Alien_Tail_Target) {
		if (Alien_Tail_Target->SBdptr==NULL) {
			/* Likewise, moved off screen. */
			Alien_Tail_Target=NULL;
			COPY_NAME(Alien_Tail_Target_SBname,Null_Name);
		} else {
			/* Do we still have a target?  Correct for aiming. */
			ComputeTailDeltaValues(XDelta,YDelta);
		}
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {

		/* Just for now, do damage anyway... */
		if (Alien_Tail_Target) {

			int multiple;
			
			multiple=ONE_FIXED;		
			/* Consider target aspect. */
			{
				VECTORCH attack_dir,displacement;

				displacement.vx = Alien_Tail_Target->DynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->Position.vx;
				displacement.vy = Alien_Tail_Target->DynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->Position.vy;
				displacement.vz = Alien_Tail_Target->DynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->Position.vz;

				GetDirectionOfAttack(Alien_Tail_Target,&displacement,&attack_dir);
		
				if (attack_dir.vz>0) {
					multiple<<=1;
				}
			
				if (Alien_Tail_Target->SBdptr->HModelControlBlock) {
					HtoHDamageToHModel(Alien_Tail_Target, &Player_Weapon_Damage,multiple, NULL, &attack_dir);
				} else {
	  				CauseDamageToObject(Alien_Tail_Target, &Player_Weapon_Damage,multiple, &attack_dir);
				}
			}
		}
		PlayAlienTailSound();
		/* Slower recoil... */
		HModel_ChangeSpeed(&PlayersWeaponHModelController,(ONE_FIXED));
		HtoHStrikes++;
	
	}

}

void AlienClaw_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int alien_speed;

		/* Consider possibility of bite first... */
		Biting=GetBitingTarget();
		if (Biting) {
			/* Fix the name. */
			COPY_NAME(Biting_SBname,Biting->SBname);
			/* Fix the speed. */
			FixAlienStrikeSpeed(ONE_FIXED/3);
			/* Init attack. */
			Bit=0;
			/* Then leave. */
			return;
		}

		alien_speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);
	
		/* Speed: < 5000 = standing.  > 22000 = jumping.
			In practice, walk/fall = 18000 ish, jump = 27000 ish. */

		/* Check we've got the claws. */

		if (Alien_Visible_Weapon!=0) {
			
			GetHierarchicalWeapon("alien_HUD","claws",(int)HMSQT_AlienHUD,(int)AHSS_LeftSwipeDown);
			ProveHModel(&PlayersWeaponHModelController,&PlayersWeapon);

			Alien_Visible_Weapon=0;
	
		}

		/* Setup base damage. */

		Player_Weapon_Damage=TemplateAmmo[AMMO_ALIEN_CLAW].MaxDamage[AvP.Difficulty];

		/* Choose attack type and speed. */

		if ((alien_speed<5000)&&(PlayerStatusPtr->ShapeState==PMph_Standing)
			&&(Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)) {
			/* Standing on solid ground. */

			TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=1;

			FixAlienStrikeSpeed(ONE_FIXED/3);

			textprint("Standing Claw, Speed %d\n",alien_speed);

			if ((FastRandom()&65536)>32768) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
					(int)AHSS_Both_In,ACStrikeTime,0);
			} else {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
					(int)AHSS_Both_Down,ACStrikeTime,0);
			}

		} else {
			
			/* Crouching, falling, or running? */

			if ( (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor==0) 
				&& (Player->ObStrategyBlock->DynPtr->TimeNotInContactWithFloor<=0) ) {
				/* Jumping or falling... */
				TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=0;
			} else {
				TemplateWeapon[WEAPON_ALIEN_CLAW].PrimaryIsAutomatic=1;
			}

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				/* If crouching, falling or no, slower strike. */
				if (  /* Gravity points vaguely down... */
					(Player->ObStrategyBlock->DynPtr->GravityDirection.vx<46341)
					&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vx>-46341)
					&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vy>46341)
					&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vz<46341)
					&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vz>-46341)
				   ) {
					FixAlienStrikeSpeed(ONE_FIXED/4);
				} else {
					FixAlienStrikeSpeed(ONE_FIXED/3);
				}
			} else {
				FixAlienStrikeSpeed(ONE_FIXED/5);
			}
			
			/* Speed test here, for jumping? */

			if ( (Player->ObStrategyBlock->DynPtr->IsInContactWithFloor==0) 
				&& (Player->ObStrategyBlock->DynPtr->TimeNotInContactWithFloor<=0)
				&& (alien_speed>22000) ) {

				/* Must be jumping. */
				FixAlienStrikeSpeed(ONE_FIXED/3);
				/* Extra damage for pounce. */
				Player_Weapon_Damage.Impact+=10;
				Player_Weapon_Damage.Cutting+=10;
				if ((FastRandom()&65536)>32768) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_PounceIn,ACStrikeTime,0);
				} else {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_PounceDown,ACStrikeTime,0);
				}

			} else if (LastHand) {
				if ((FastRandom()&65536)>32768) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_LeftSwipeIn,(ONE_FIXED/6),0);
				} else {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_LeftSwipeDown,(ONE_FIXED/6),0);
				}
				LastHand=0;
			} else {
				if ((FastRandom()&65536)>32768) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_RightSwipeIn,(ONE_FIXED/6),0);
				} else {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
						(int)AHSS_RightSwipeDown,(ONE_FIXED/6),0);
				}
				LastHand=1;
			}
		}

		PlayersWeaponHModelController.Playing=1;
	}
	
	/* Now we're in the attack. */
	if (Biting) {
		/* Kinda placeholder. */
		if (weaponPtr->StateTimeOutCounter<(WEAPONSTATE_INITIALTIMEOUTCOUNT>>1)) {
			if (Bit==0) {
				SECTION_DATA *head_sec;
				Bit=1;
				/* Try the bite. */
				head_sec=CheckBiteIntegrity();
				if (head_sec)
				{
					AVP_BEHAVIOUR_TYPE pre_bite_type=Biting->I_SBtype;

					CurrentGameStats_HeadBitten(Biting);

					/* Munch! */
					if (SUPERGORE_MODE) {
						CauseDamageToHModel(Biting->SBdptr->HModelControlBlock, Biting, &TemplateAmmo[AMMO_ALIEN_BITE_KILLSECTION_SUPER].MaxDamage[AvP.Difficulty],
							ONE_FIXED,head_sec,NULL,NULL,0);
					} else {
						CauseDamageToHModel(Biting->SBdptr->HModelControlBlock, Biting, &TemplateAmmo[AMMO_ALIEN_BITE_KILLSECTION].MaxDamage[AvP.Difficulty],
							ONE_FIXED,head_sec,NULL,NULL,0);
					}

					/* Side effects, anyone? */
					{
						BiteAttack_AwardHealth(Biting,pre_bite_type);
					}
					/* Play a sound? */
					if (PlayerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
						Sound_Stop(PlayerStatusPtr->soundHandle);
					}
					Sound_Play(SID_ALIEN_JAW_ATTACK,"de",&(Player->ObStrategyBlock->DynPtr->Position),&PlayerStatusPtr->soundHandle);
					HtoHStrikes++;

					/* KJL 11:55:27 30/07/98 - Cue Special FX! */
					{
						extern void AlienBiteAttackHasHappened(void);
						AlienBiteAttackHasHappened();
					}

				}
				else
				{
					/* Play a different sound? */
				}
			}
		}
	} else {
		/* Execute attack. */
		int hits;

		hits=0;

		if (PlayersWeaponHModelController.keyframe_flags&1) {
			hits++;
		}

		if (PlayersWeaponHModelController.keyframe_flags&2) {
			hits++;
		}

		if (hits) {

			HtoHStrikes+=hits;

			MeleeWeapon_180Degree_Front_Core(&Player_Weapon_Damage,ONE_FIXED*hits,4000);
			PlayAlienSwipeSound();

		}

	}

}

void AlienGrab_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Same as for claws, ATM */

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_AlienHUD,(int)AHSS_LeftSwipeDown,ONE_FIXED/3);
		PlayersWeaponHModelController.Looped=0;
		PlayersWeaponHModelController.Playing=0;
	}

	LastHand=0;
	Alien_Visible_Weapon=0; //Claws
}

void AlienGrab_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* ...Nothing? */

}

void AlienGrab_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* ...Nothing? */

}

void AlienGrab_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Eating function. */

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int alien_speed;

		alien_speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);
	
		/* Speed: < 5000 = standing.  > 22000 = jumping.
			In practice, walk/fall = 18000 ish, jump = 27000 ish. */

		/* Check we've got the claws. */

		if (Alien_Visible_Weapon!=2) {
			
			GetHierarchicalWeapon("alien_HUD","eat",(int)HMSQT_AlienHUD,(int)AHSS_Eat);

			Alien_Visible_Weapon=2;
	
		}

		/* Setup base damage. */

		Player_Weapon_Damage=TemplateAmmo[AMMO_ALIEN_CLAW].MaxDamage[AvP.Difficulty];

		/* Choose attack type and speed. */

		if ((alien_speed<5000)&&(Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)
			&&( /* Gravity check. */
			(Player->ObStrategyBlock->DynPtr->GravityDirection.vx<46341)
			&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vx>-46341)
			&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vy>46341)
			&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vz<46341)
			&&(Player->ObStrategyBlock->DynPtr->GravityDirection.vz>-46341)
			)) {
			/* Standing on solid ground. */

			TemplateWeapon[WEAPON_ALIEN_GRAB].PrimaryIsAutomatic=1;

			TemplateWeapon[WEAPON_ALIEN_GRAB].TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]=
				(DIV_FIXED(WEAPONSTATE_INITIALTIMEOUTCOUNT,(ONE_FIXED/3)));

			if ((FastRandom()&65536)>32768) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
					(int)AHSS_Eat,(ONE_FIXED/3),0);
			} else {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_AlienHUD,
					(int)AHSS_Eat,(ONE_FIXED/3),0);
			}
			PlayersWeaponHModelController.Playing=1;

		} else {
			
			/* No joy. */

		}

	}
	
	{
		/* Execute attack. */
		int hits;

		hits=0;

		if (PlayersWeaponHModelController.keyframe_flags&1) {
			hits++;
		}

		if (PlayersWeaponHModelController.keyframe_flags&2) {
			hits++;
		}

		if (hits) {

			PC_Alien_Eat_Attack(hits);
			PlayAlienSwipeSound();

		}

	}

}

int Target_IsEdible(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourAlien:
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourXenoborg:
		case I_BehaviourPredatorAlien:
			{
				return(0);
				break;
			}
		case I_BehaviourPredator:
		case I_BehaviourMarine:
		case I_BehaviourSeal:
			{
				if (NPC_IsDead(candidate)) {
					/* Must be dead to be eaten. */
					return(1);
				} else {
					return(0);
				}
				break;
			}
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
						/* Put a test in, once we have dead ghosts. */
						return(0);
						break;
					default:
						return(0);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

}

void Placeholder_Eating_Effect(STRATEGYBLOCK *sbPtr) {

	VECTORCH final_spray_direction;
	enum PARTICLE_ID blood_type;
	/* Spray is go! */
	
	/* Add random element. */
	
	final_spray_direction.vx=( (FastRandom()&2047)-1024);
	final_spray_direction.vy=( -(FastRandom()&511)); /* Should be upwards. */
	final_spray_direction.vz=( (FastRandom()&2047)-1024);
	
	/* Identify spray type. */
	
	blood_type=GetBloodType(sbPtr);
	
	/* Call spray function. */
						
	MakeParticle(&sbPtr->DynPtr->Position, &final_spray_direction, blood_type);
	
}

#define EAT_ATTACK_RANGE 1500

int PC_Alien_Eat_Attack(int hits)
{
	int numberOfObjects = NumOnScreenBlocks;
	int numhits=0;
	
	while (numberOfObjects)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];
		STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;
		GLOBALASSERT(objectPtr);
		
		/* does object have a strategy block? */
		if (sbPtr)
		{		
			/* is it in the frustrum? */
			if ( (objectPtr->ObView.vz >0) 
				&& (objectPtr->ObView.vz >  objectPtr->ObView.vx) 
				&& (objectPtr->ObView.vz > -objectPtr->ObView.vx) 
				&& (objectPtr->ObView.vz >  objectPtr->ObView.vy) 
				&& (objectPtr->ObView.vz > -objectPtr->ObView.vy) ) {

				int dist=Approximate3dMagnitude(&objectPtr->ObView);

				if (dist<EAT_ATTACK_RANGE)	{
										
					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			  	  	if (dynPtr)
					{

						if (Target_IsEdible(sbPtr)) {
							/* Go go gadget eat? */
							textprint("Eating! Yum Yum...\n");
							Placeholder_Eating_Effect(sbPtr);
						}

						numhits++;
					}
			  	}
			}
		}
	}
 
	return(numhits);
}

void PlasmaCaster_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	/* Note, not the same as WristConsole_Idle! */

	/* Do charge, then sequences. */

	/* Jumpstart plasmacaster. */
	if (playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart) {
		int jumpgap,jumpcharge;
		
		jumpgap=Caster_Jumpstart-playerStatusPtr->PlasmaCasterCharge;

		if (playerStatusPtr->FieldCharge>=MUL_FIXED(jumpgap,Caster_ChargeRatio)) {
			/* We have enough. */
			jumpcharge=jumpgap;
		} else {
			//jumpcharge=DIV_FIXED(playerStatusPtr->FieldCharge,Caster_ChargeRatio);
			/* Don't drain insufficient charge. */
			jumpcharge=0;
		}

		playerStatusPtr->PlasmaCasterCharge+=jumpcharge;
		playerStatusPtr->FieldCharge-=MUL_FIXED(jumpcharge,Caster_ChargeRatio);
		CurrentGameStats_ChargeUsed(MUL_FIXED(jumpcharge,Caster_ChargeRatio));
		LOCALASSERT(playerStatusPtr->FieldCharge>=0);

	}

	if ((playerStatusPtr->PlasmaCasterCharge<Caster_TrickleLevel)&&(Caster_TrickleRate)) {
		if (playerStatusPtr->FieldCharge>0) {
			int chargerate;
			int optimumchargerate;
			
			optimumchargerate=MUL_FIXED(Caster_TrickleRate,NormalFrameTime);
			/* optimumchargerate is for the CASTER. */

			if (playerStatusPtr->FieldCharge>=MUL_FIXED(optimumchargerate,Caster_ChargeRatio)) {
				chargerate=optimumchargerate;
			} else {
				GLOBALASSERT(Caster_ChargeRatio);
				chargerate=DIV_FIXED(playerStatusPtr->FieldCharge,Caster_ChargeRatio);
			}
			
			if (playerStatusPtr->PlasmaCasterCharge+chargerate>Caster_TrickleLevel) {
				chargerate=Caster_TrickleLevel-playerStatusPtr->PlasmaCasterCharge;
			}

			playerStatusPtr->FieldCharge-=MUL_FIXED(chargerate,Caster_ChargeRatio);
			CurrentGameStats_ChargeUsed(MUL_FIXED(chargerate,Caster_ChargeRatio));
			LOCALASSERT(playerStatusPtr->FieldCharge>=0);
			playerStatusPtr->PlasmaCasterCharge+=chargerate;
			LOCALASSERT(playerStatusPtr->PlasmaCasterCharge<=Caster_TrickleLevel);
		}
	}

	if (ShowPredoStats) {
		PrintDebuggingText("Plasma Caster Charge = %d\n",playerStatusPtr->PlasmaCasterCharge);
	}

	/* Now anim control. */

	if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Attack_Primary) {
		if (PlayersWeaponHModelController.Tweening!=0) {
			PlayersWeaponHModelController.Playing=1;
			return;
		}
		GLOBALASSERT(PlayersWeaponHModelController.Looped==0);
		if (!(HModelAnimation_IsFinished(&PlayersWeaponHModelController))) {
			return;
		}		
	}

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
	}

	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

	/* Be very quiet... we're hunting wabbits! */
	if(weaponHandle != SOUND_NOACTIVEINDEX) {
   		Sound_Stop(weaponHandle);
	}

}

int SecondaryFirePCPlasmaCaster(PLAYER_WEAPON_DATA *weaponPtr) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	if (PlayersWeaponHModelController.Sub_Sequence!=PHSS_Attack_Primary) {
		/* Start Animation. */
		GLOBALASSERT(PlayersWeaponHModelController.section_data);
		GLOBALASSERT(PlayersWeaponHModelController.Playing==1);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
	} else {
		if (PlayersWeaponHModelController.keyframe_flags) {
			PlayersWeaponHModelController.Playing=0;
		}
	}

	/* Handle field charge. */	
	if (playerStatusPtr->PlasmaCasterCharge<ONE_FIXED) {
		if (playerStatusPtr->FieldCharge>0) {
			int chargerate;
			int optimumchargerate;
			
			//optimumchargerate=NormalFrameTime>>CASTER_CHARGETIME; /* 8 sec recharge? */
			
			if (Caster_Chargetime==0) {
				optimumchargerate=65536;
			} else {
				optimumchargerate=DIV_FIXED(NormalFrameTime,Caster_Chargetime);
			}
			/* optimumchargerate is for the CASTER. */

			if (playerStatusPtr->FieldCharge>=MUL_FIXED(optimumchargerate,Caster_ChargeRatio)) {
				chargerate=optimumchargerate;
			} else {
				GLOBALASSERT(Caster_ChargeRatio);
				chargerate=DIV_FIXED(playerStatusPtr->FieldCharge,Caster_ChargeRatio);
			}
			
			if (playerStatusPtr->PlasmaCasterCharge+chargerate>ONE_FIXED) {
				chargerate=ONE_FIXED-playerStatusPtr->PlasmaCasterCharge;
			}

			playerStatusPtr->FieldCharge-=MUL_FIXED(chargerate,Caster_ChargeRatio);
			CurrentGameStats_ChargeUsed(MUL_FIXED(chargerate,Caster_ChargeRatio));
			LOCALASSERT(playerStatusPtr->FieldCharge>=0);
			playerStatusPtr->PlasmaCasterCharge+=chargerate;
			LOCALASSERT(playerStatusPtr->PlasmaCasterCharge<=ONE_FIXED);

			if (chargerate) {
				/* Play a charging sound! */
		  		if(weaponHandle != SOUND_NOACTIVEINDEX) {
					if (ActiveSounds[weaponHandle].soundIndex!=SID_PREDATOR_PLASMACASTER_CHARGING) {
						/* Stop other sounds... */
			       		Sound_Stop(weaponHandle);
					}
				}
		  		if(weaponHandle == SOUND_NOACTIVEINDEX) {
		  			Sound_Play(SID_PREDATOR_PLASMACASTER_CHARGING,"ehl",&weaponHandle);
				}
			} else {
				/* Be very quiet... we're hunting wabbits! */
				if(weaponHandle != SOUND_NOACTIVEINDEX) {
			   		Sound_Stop(weaponHandle);
				}
			}
		}
	} else {
		/* Be very quiet... we're hunting wabbits! */
		if(weaponHandle != SOUND_NOACTIVEINDEX) {
	   		Sound_Stop(weaponHandle);
		}
	}

	return(1);
}

int FirePCPlasmaCaster(PLAYER_WEAPON_DATA *weaponPtr) {

	int jumpcharge;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
	
	if (playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart) {
		return(0);
	}

	/* Fix plasmacaster damage. */

	#if 0
	Player_Weapon_Damage=TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty];
		
	Player_Weapon_Damage.Impact 	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Impact 	,playerStatusPtr->PlasmaCasterCharge);
	Player_Weapon_Damage.Cutting	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Cutting	,playerStatusPtr->PlasmaCasterCharge);
	Player_Weapon_Damage.Penetrative=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Penetrative,playerStatusPtr->PlasmaCasterCharge);
	Player_Weapon_Damage.Fire		=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Fire		,playerStatusPtr->PlasmaCasterCharge);
	Player_Weapon_Damage.Electrical	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Electrical	,playerStatusPtr->PlasmaCasterCharge);
	Player_Weapon_Damage.Acid		=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Acid		,playerStatusPtr->PlasmaCasterCharge);

	Player_Weapon_Damage.BlowUpSections=1;
	Player_Weapon_Damage.Special=0;
	#else
	Player_Weapon_Damage=TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty];

	if (Caster_PCKill>0) {
		
		/* At least we can theoretically work it. */

		if ((playerStatusPtr->PlasmaCasterCharge>=Caster_NPCKill)
			&&(Caster_PCKill>Caster_NPCKill)) {
			/* In the upper graph. */
			int factor;

			factor=playerStatusPtr->PlasmaCasterCharge-Caster_NPCKill;
			factor=DIV_FIXED(factor,(Caster_PCKill-Caster_NPCKill));

			Player_Weapon_Damage.Impact 	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Impact 		+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Impact 		,factor);
			Player_Weapon_Damage.Cutting	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Cutting		+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Cutting		,factor);
			Player_Weapon_Damage.Penetrative=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Penetrative	+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Penetrative	,factor);
			Player_Weapon_Damage.Fire		=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Fire			+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Fire		,factor);
			Player_Weapon_Damage.Electrical	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Electrical	+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Electrical	,factor);
			Player_Weapon_Damage.Acid		=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Acid			+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Acid		,factor);

		} else {
			/* In the lower graph. */
			int factor;
			
			factor=playerStatusPtr->PlasmaCasterCharge;
			factor=DIV_FIXED(factor,Caster_NPCKill);

			Player_Weapon_Damage.Impact 	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Impact 	,factor);
			Player_Weapon_Damage.Cutting	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Cutting	,factor);
			Player_Weapon_Damage.Penetrative=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Penetrative,factor);
			Player_Weapon_Damage.Fire		=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Fire		,factor);
			Player_Weapon_Damage.Electrical	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Electrical	,factor);
			Player_Weapon_Damage.Acid		=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Acid		,factor);

		}

	}

	Player_Weapon_Damage.BlowUpSections=1;
	Player_Weapon_Damage.Special=0;
	#endif
	
	InitialiseEnergyBoltBehaviour(&Player_Weapon_Damage,playerStatusPtr->PlasmaCasterCharge);

	/* Be very quiet... we're hunting wabbits! */
	if(weaponHandle != SOUND_NOACTIVEINDEX) {
   		Sound_Stop(weaponHandle);
	}

	/* Jumpstart plasmacaster. */

	if (playerStatusPtr->FieldCharge>=MUL_FIXED(Caster_Jumpstart,Caster_ChargeRatio)) {
		jumpcharge=Caster_Jumpstart;
	} else {
		//jumpcharge=DIV_FIXED(playerStatusPtr->FieldCharge,Caster_ChargeRatio);
		/* Not enough - drain nothing! */
		jumpcharge=0;
	}

	playerStatusPtr->PlasmaCasterCharge=jumpcharge;
	playerStatusPtr->FieldCharge-=MUL_FIXED(jumpcharge,Caster_ChargeRatio);
	CurrentGameStats_ChargeUsed(MUL_FIXED(jumpcharge,Caster_ChargeRatio));
	LOCALASSERT(playerStatusPtr->FieldCharge>=0);

	return(1);
}

void Secondary_PlasmaCaster_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {
	/* Restart anim. */
	PlayersWeaponHModelController.Playing=1;

	/* Be very quiet... we're hunting wabbits! */
	if(weaponHandle != SOUND_NOACTIVEINDEX) {
   		Sound_Stop(weaponHandle);
	}
}

void PlasmaCaster_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	int jumpcharge;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
	
	PlayersWeaponHModelController.Playing=1;

	/* Be very quiet... we're hunting wabbits! */
	if(weaponHandle != SOUND_NOACTIVEINDEX) {
   		Sound_Stop(weaponHandle);
	}

	#if 0
	if (playerStatusPtr->PlasmaCasterCharge<Caster_Jumpstart) {
		return;
	}
	#endif

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		if (playerStatusPtr->PlasmaCasterCharge<Caster_MinCharge) {
			/* Don't fire at all! */
			Sound_Play(SID_PREDATOR_PLASMACASTER_EMPTY,"h");
			return;
		}

		/* Fix plasmacaster damage. */
		#if 0
		int multiplyer,a;
		a=playerStatusPtr->PlasmaCasterCharge;
		
		/* These values computed by hand! */
		multiplyer=MUL_FIXED(a,a);
		multiplyer=MUL_FIXED(multiplyer,26653);
		multiplyer+=MUL_FIXED(a,38883);
		/* Should fit for JUMPCHARGE == 0.1*max. */
		LOCALASSERT(multiplyer>=0);
		
		Player_Weapon_Damage=TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty];

		Player_Weapon_Damage.Impact 	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Impact 	,multiplyer);
		Player_Weapon_Damage.Cutting	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Cutting	,multiplyer);
		Player_Weapon_Damage.Penetrative=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Penetrative,multiplyer);
		Player_Weapon_Damage.Fire		=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Fire		,multiplyer);
		Player_Weapon_Damage.Electrical	=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Electrical	,multiplyer);
		Player_Weapon_Damage.Acid		=MUL_FIXED(TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty].Acid		,multiplyer);
		
		Player_Weapon_Damage.BlowUpSections=1;
		Player_Weapon_Damage.Special=0;
		#else
		Player_Weapon_Damage=TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty];

		if (Caster_PCKill>0) {
			
			/* At least we can theoretically work it. */

			if ((playerStatusPtr->PlasmaCasterCharge>=Caster_NPCKill)
				&&(Caster_PCKill>Caster_NPCKill)) {
				/* In the upper graph. */
				int factor;

				factor=playerStatusPtr->PlasmaCasterCharge-Caster_NPCKill;
				factor=DIV_FIXED(factor,(Caster_PCKill-Caster_NPCKill));

				Player_Weapon_Damage.Impact 	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Impact 		+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Impact 		,factor);
				Player_Weapon_Damage.Cutting	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Cutting		+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Cutting		,factor);
				Player_Weapon_Damage.Penetrative=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Penetrative	+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Penetrative	,factor);
				Player_Weapon_Damage.Fire		=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Fire			+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Fire		,factor);
				Player_Weapon_Damage.Electrical	=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Electrical	+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Electrical	,factor);
				Player_Weapon_Damage.Acid		=TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Acid			+MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_PCKILL].MaxDamage[AvP.Difficulty].Acid		,factor);

			} else {
				/* In the lower graph. */
				int factor;
				
				factor=playerStatusPtr->PlasmaCasterCharge;
				factor=DIV_FIXED(factor,Caster_NPCKill);

				Player_Weapon_Damage.Impact 	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Impact 	,factor);
				Player_Weapon_Damage.Cutting	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Cutting	,factor);
				Player_Weapon_Damage.Penetrative=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Penetrative,factor);
				Player_Weapon_Damage.Fire		=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Fire		,factor);
				Player_Weapon_Damage.Electrical	=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Electrical	,factor);
				Player_Weapon_Damage.Acid		=MUL_FIXED(TemplateAmmo[AMMO_PLASMACASTER_NPCKILL].MaxDamage[AvP.Difficulty].Acid		,factor);

			}

		}

		Player_Weapon_Damage.BlowUpSections=1;
		Player_Weapon_Damage.Special=0;
		#endif

		InitialiseEnergyBoltBehaviour(&Player_Weapon_Damage,playerStatusPtr->PlasmaCasterCharge);

		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
		
		/* Jumpstart plasmacaster. */
		
		if (playerStatusPtr->FieldCharge>=MUL_FIXED(Caster_Jumpstart,Caster_ChargeRatio)) {
			jumpcharge=Caster_Jumpstart;
		} else {
			GLOBALASSERT(Caster_ChargeRatio);
			jumpcharge=DIV_FIXED(playerStatusPtr->FieldCharge,Caster_ChargeRatio);
		}
		
		playerStatusPtr->PlasmaCasterCharge=jumpcharge;
		playerStatusPtr->FieldCharge-=MUL_FIXED(jumpcharge,Caster_ChargeRatio);
		CurrentGameStats_ChargeUsed(MUL_FIXED(jumpcharge,Caster_ChargeRatio));
		LOCALASSERT(playerStatusPtr->FieldCharge>=0);

	}
}

#define FIREPREDPISTOL_FIELDCHARGE (ONE_FIXED>>2)

int FirePredPistol(PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

    if (playerStatusPtr->FieldCharge>=FIREPREDPISTOL_FIELDCHARGE)
    {
		FireProjectileAmmo(twPtr->PrimaryAmmoID);
		playerStatusPtr->FieldCharge-=FIREPREDPISTOL_FIELDCHARGE;
		CurrentGameStats_ChargeUsed(FIREPREDPISTOL_FIELDCHARGE);
		return(1);
    }
    else /* instantaneous line of sight */
    {
		return(0);
	}	
}	

#define FIRESPEARGUN_FIELDCHARGE (0)
#define SPEAR_PLAYER_IMPULSE 	(-8000)

int FireSpeargun(PLAYER_WEAPON_DATA *weaponPtr)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

    if (playerStatusPtr->FieldCharge>=FIRESPEARGUN_FIELDCHARGE)
    {
		/* Ammo check already happened. */
		if (!(PIGSTICKING_MODE)) {
		  	weaponPtr->PrimaryRoundsRemaining -= 65536;
		}

		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
		
		//FireProjectileAmmo(twPtr->PrimaryAmmoID);
		if (PlayersTarget.DispPtr)
		{

			if (AccuracyStats_TargetFilter(PlayersTarget.DispPtr->ObStrategyBlock)) {
				CurrentGameStats_WeaponHit(PlayerStatusPtr->SelectedWeaponSlot,1);
			}

			if (PlayersTarget.HModelSection!=NULL) {
				textprint("Hitting a hierarchical section.\n");
				GLOBALASSERT(PlayersTarget.DispPtr->HModelControlBlock==PlayersTarget.HModelSection->my_controller);
			}
			
			if (PIGSTICKING_MODE) {
				/* Cheat mode goes here! */
				int hitroll=0;
		
				while (SpreadfireSpears[hitroll].vz>0) {
					VECTORCH world_vec;
					MATRIXCH transpose;
			
					transpose=Global_VDB_Ptr->VDB_Mat;
					TransposeMatrixCH(&transpose);
					RotateAndCopyVector(&SpreadfireSpears[hitroll],&world_vec,&transpose);
					CastLOSSpear(Player->ObStrategyBlock,&Global_VDB_Ptr->VDB_World,&world_vec, AMMO_PRED_RIFLE, 1,0);
	
					hitroll++;
				}
			} else {
				HandleSpearImpact(&(PlayersTarget.Position),PlayersTarget.DispPtr->ObStrategyBlock,twPtr->PrimaryAmmoID,&GunMuzzleDirectionInWS, 1, PlayersTarget.HModelSection);
			}
		}

		if ((Player->ObStrategyBlock->DynPtr->Position.vx==Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
			&&(Player->ObStrategyBlock->DynPtr->Position.vy==Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
			&&(Player->ObStrategyBlock->DynPtr->Position.vz==Player->ObStrategyBlock->DynPtr->PrevPosition.vz)
			&&(Player->ObStrategyBlock->DynPtr->IsInContactWithFloor)
			){
			/* Behave normally! */
		} else {
			/* Kickback! */
			Player->ObStrategyBlock->DynPtr->LinImpulse.vx+=MUL_FIXED(PlayersWeapon.ObMat.mat31,SPEAR_PLAYER_IMPULSE);
			Player->ObStrategyBlock->DynPtr->LinImpulse.vy+=MUL_FIXED(PlayersWeapon.ObMat.mat32,SPEAR_PLAYER_IMPULSE);
			Player->ObStrategyBlock->DynPtr->LinImpulse.vz+=MUL_FIXED(PlayersWeapon.ObMat.mat33,SPEAR_PLAYER_IMPULSE);
		}

		if (!(PIGSTICKING_MODE)) {
			playerStatusPtr->FieldCharge-=FIRESPEARGUN_FIELDCHARGE;
			CurrentGameStats_ChargeUsed(FIRESPEARGUN_FIELDCHARGE);
		}
		return(1);
    }
    else /* instantaneous line of sight */
    {
		return(0);
	}	
}	

int Tail_TargetFilter(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourPredator:
		case I_BehaviourXenoborg:
		case I_BehaviourMarine:
		case I_BehaviourSeal:
		case I_BehaviourAutoGun:
		case I_BehaviourPredatorDisc_SeekTrack:
		case I_BehaviourInanimateObject:
		case I_BehaviourRubberDuck:
		case I_BehaviourPlacedLight:
		case I_BehaviourDormantPredator:
		case I_BehaviourTrackObject:
			return(1);
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourPredator:
					case I_BehaviourXenoborg:
					case I_BehaviourMarine:
					case I_BehaviourSeal:
					case I_BehaviourAutoGun:
					case I_BehaviourPredatorDisc_SeekTrack:
					case I_BehaviourInanimateObject:
					case I_BehaviourRubberDuck:
					case I_BehaviourPlacedLight:
					case I_BehaviourDormantPredator:
					case I_BehaviourMarinePlayer:
					case I_BehaviourPredatorPlayer:
						return(1);
						break;
					case I_BehaviourAlienPlayer:
					{
						switch (netGameData.gameType) {
							case NGT_Individual:
								return(1);
								break;
							case NGT_CoopDeathmatch:
								return(0);
								break;
							case NGT_LastManStanding:
								return(0);
								break;
							case NGT_PredatorTag:
								return(1);
								break;
							case NGT_Coop:
								return(0);
								break;
							case NGT_AlienTag:
								return(1); //However, there shouldn't be more than one alien in alien tag anyway.
								break;
							default:
								return(0);
								break;
						}
						break;
					}
					default:
						return(0);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

}

#define ALIEN_TAIL_RANGE (4000)

DISPLAYBLOCK *AlienTail_TargetSelect(void)
{
	int numberOfObjects = NumOnScreenBlocks;
	DISPLAYBLOCK *nearest;
	STRATEGYBLOCK *sbPtr;
	int neardist;

	nearest=NULL;
	neardist=100000;

	while (numberOfObjects)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];
		sbPtr = objectPtr->ObStrategyBlock;
		GLOBALASSERT(objectPtr);
		
		/* does object have a strategy block? */
		if (sbPtr)
		{		
			if (Tail_TargetFilter(sbPtr)) {
				/* is it in the frustrum? */
				if ( (objectPtr->ObView.vz >0) 
					&& (objectPtr->ObView.vz >  (objectPtr->ObView.vx>>1)) 
					&& (objectPtr->ObView.vz > -(objectPtr->ObView.vx>>1)) 
					&& (objectPtr->ObView.vz >  (objectPtr->ObView.vy>>1)) 
					&& (objectPtr->ObView.vz > -(objectPtr->ObView.vy>>1)) ) {
	
					int dist=Approximate3dMagnitude(&objectPtr->ObView);
	
					if (dist<ALIEN_TAIL_RANGE)	{
	
						DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
				  	  	if (dynPtr)
						{
							//if (IsThisObjectVisibleFromThisPosition_WithIgnore(Player,objectPtr,&dynPtr->Position,ALIEN_TAIL_RANGE)) {
							if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&dynPtr->Position)) {
					  			/* Consider target validity here? */
					  			
								if (dist<neardist) {
									nearest=objectPtr;
									neardist=dist;
								}
							}
						}
				  	}
				}
			}
		}
	}

	if (nearest) {
		return(nearest);
	}

	/* Let's see if we've missed something obvious. */
	if (PlayersTarget.DispPtr) {
		if (PlayersTarget.Distance<ALIEN_TAIL_RANGE) {
			if (PlayersTarget.DispPtr->ObStrategyBlock) {
				sbPtr=PlayersTarget.DispPtr->ObStrategyBlock;
				/* It must be in the frustrum... */
				if (sbPtr) {
					if (sbPtr->DynPtr) {
						if (Tail_TargetFilter(sbPtr)) {
							/* May as well hit this, then. */
							return(PlayersTarget.DispPtr);
						}
					}
				}
			}
		}
	}

	return(NULL);
}

/* Plasmacaster function set. */

void WristConsole_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
	}

	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

}

void TemplateHands_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		GLOBALASSERT(PlayersWeaponHModelController.section_data);
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Come,ONE_FIXED/3);
		PlayersWeaponHModelController.Looped=0;

		/* Stop pred hud sound. */
		if(predHUDSoundHandle != SOUND_NOACTIVEINDEX) {
			Sound_Stop(predHUDSoundHandle);
		}
	}

	/* Init damage for plasmacaster. */

	Player_Weapon_Damage=TemplateAmmo[AMMO_PRED_ENERGY_BOLT].MaxDamage[AvP.Difficulty];

	Player_Weapon_Damage.Impact=0;
	Player_Weapon_Damage.Cutting=0;
	Player_Weapon_Damage.Penetrative=0;
	Player_Weapon_Damage.Fire=0;
	Player_Weapon_Damage.Electrical=0;
	Player_Weapon_Damage.Acid=0;
	Player_Weapon_Damage.Special=0;

}

void WristConsole_Readying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		SECTION *root_section;

		root_section=GetNamedHierarchyFromLibrary("pred_HUD","wrist display");
		GLOBALASSERT(root_section);
		
		#if 0
		Dispel_HModel(&PlayersWeaponHModelController);
		Create_HModel(&PlayersWeaponHModelController,root_section);
		GLOBALASSERT(PlayersWeaponHModelController.section_data);
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Come,131625);	/* 2.1s */
		PlayersWeaponHModelController.Looped=0;
		#else
		PlayersWeaponHModelController.Sequence_Type=HMSQT_PredatorHUD;
		PlayersWeaponHModelController.Sub_Sequence=PHSS_Come;
		Transmogrify_HModels(NULL,&PlayersWeaponHModelController,root_section,0,1,1);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Come,129434,0);	/* 2.1s */
		#endif

		/* Stop pred hud sound. */
		if(predHUDSoundHandle != SOUND_NOACTIVEINDEX) {
			Sound_Stop(predHUDSoundHandle);
		}

	}
	
}

void WristConsole_Unreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		GLOBALASSERT(PlayersWeaponHModelController.section_data);
		#if 0
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Go,104857); /* 1.6s */
		PlayersWeaponHModelController.Looped=0;
		#else
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Go,96666,0); /* 1.6s */
		#endif
	}
}

void WristConsole_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		GLOBALASSERT(PlayersWeaponHModelController.section_data);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
	}
}

void SADAR_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,ONE_FIXED);
		PlayersWeaponHModelController.Looped=0;
	}

}

void SADAR_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Go,ONE_FIXED,1);
		PlayersWeaponHModelController.Looped=0;
	}

}

void SADAR_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
	}
	
	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,ONE_FIXED,0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	
	}

}

void SADAR_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
	}

}

void SADAR_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		//InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,(ONE_FIXED/6)-(ONE_FIXED>>4),0);
		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire,(ONE_FIXED/6));
		PlayersWeaponHModelController.Looped=0;
	}

}

void SADAR_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Standard_Reload,((ONE_FIXED*3)/2),0);
	}

}

void Minigun_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	Weapon_ThisBurst=-1;

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		/* Just play the readying sound. */
		Sound_Play(SID_WIL_MINIGUN_READY,"h");					
	}

	Flamethrower_Timer=0;
}

void GenericMarineWeapon_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	Weapon_ThisBurst=-1;

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_IN]);

		GLOBALASSERT(time!=0);

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,time);
		PlayersWeaponHModelController.Looped=0;
	}

	Flamethrower_Timer=0;
	StaffAttack=-1;
	LastHand=1;
}

void GenericMarineWeapon_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_OUT]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;
	}
}

void GenericMarineWeapon_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	
	#if 0
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
	}
	#endif

	if ((PlayersWeaponHModelController.Sub_Sequence!=(int)twPtr->InitialSubSequence)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
	}
	
	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,ONE_FIXED,0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	
	}

	Flamethrower_Timer=0;
	StaffAttack=-1;

}

void GenericMarineWeapon_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,ONE_FIXED,1);
	}

}

void GenericMarineWeapon_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Reload,time,0);
		PlayersWeaponHModelController.Looped=0;
	}
	StaffAttack=-1;
}

void GenericPredatorWeapon_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_IN]);

		GLOBALASSERT(time!=0);

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Come,time);
		PlayersWeaponHModelController.Looped=0;
	}

	Flamethrower_Timer=0;
}

void GenericPredatorWeapon_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_OUT]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;
	}
}

void GenericPredatorWeapon_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	
	#if 0
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
	}
	#endif

	if ((PlayersWeaponHModelController.Sub_Sequence!=(int)twPtr->InitialSubSequence)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)
		&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Run)) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
	}
	
	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

}

void GenericPredatorWeapon_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,time,1);
	}

}

void SpearGun_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,time,0);
	}

}

void GenericPredatorWeapon_Firing_Secondary(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_SECONDARY]);
		time-=(ONE_FIXED>>4);												   

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary,time,1);
	}

}

void GenericPredatorWeapon_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Plays 'Program', actually... */
	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Program,time,0);
		PlayersWeaponHModelController.Looped=0;
	}
}

#if 0
void PredatorDisc_Throwing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,time,0);
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {
		SECTION_DATA *disc_section;
		disc_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"disk");
		GLOBALASSERT(disc_section);
		/* Throw Disc. */
		if (FirePredatorDisc(weaponPtr,disc_section)) {
			/* Mask disc. */
			disc_section->flags|=section_data_notreal;
		}
	}
}
#else
void PredatorDisc_Throwing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_FIRING_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
	} else if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		weaponPtr->StateTimeOutCounter=0;
	} else {
		weaponPtr->StateTimeOutCounter=(ONE_FIXED>>1);
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {
		SECTION_DATA *disc_section;
		disc_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"disk");
		GLOBALASSERT(disc_section);
		/* Throw Disc. */
		if (FirePredatorDisc(weaponPtr,disc_section)) {
			/* Mask disc. */
			disc_section->flags|=section_data_notreal;
		}
	}
}
#endif

void PredatorDisc_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	
	/* A bit like the Alien Claw one. */

	if ((PlayersWeaponHModelController.Sub_Sequence!=(int)twPtr->InitialSubSequence)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)
		&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Run)) {
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)twPtr->InitialSubSequence,ONE_FIXED,1);
	}
	
	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */
		if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Run)) {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
			}
		} else {
			if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),PHSS_Stand,(int)PHSS_Stand,ONE_FIXED,1);
			}
		}
	} else {
		TEMPLATE_WEAPON_DATA *twPtr;
    
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=PHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
		}
	
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {

			if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Fidget) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),twPtr->InitialSequenceType,(int)PHSS_Stand,ONE_FIXED,1);
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
				}
			}
		}
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {
		SECTION_DATA *disc_section;
		disc_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"disk");
		if (disc_section) {
			/* Throw Disc. */
			if (FirePredatorDisc(weaponPtr,disc_section)) {
				/* Mask disc. */
				disc_section->flags|=section_data_notreal;
			}
		}
	}

}

int PredatorDisc_Prefiring(PLAYER_WEAPON_DATA *weaponPtr) {

	/* Hey ho. */

	return(1);
}

int FirePredatorDisc(PLAYER_WEAPON_DATA *weaponPtr,SECTION_DATA *disc_section) {

  	#if 0
  	PC_PRED_DISC_BEHAV_BLOCK bblk;

	PredDisc_GetFirstTarget(&bblk, PlayersTarget.DispPtr, &disc_section->World_Offset);

	if ((bblk.Target==NULL)&&(ThisDiscMode==I_Search_Destroy)) {
		NewOnScreenMessage("NO TARGET");
        weaponPtr->CurrentState = WEAPONSTATE_WAITING;
		return(0);
	} else {
	  	weaponPtr->PrimaryRoundsRemaining -= 65536;
		InitialiseDiscBehaviour(bblk.Target,disc_section);
		return(1);	
	}
	#else
	if (SmartTarget_Object) {
		InitialiseDiscBehaviour(SmartTarget_Object->ObStrategyBlock,disc_section);
	} else {
		InitialiseDiscBehaviour(NULL,disc_section);
	}
	weaponPtr->PrimaryRoundsRemaining -= 65536;
	return(1);
	#endif
}

void PredatorDisc_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Right.  Go, recreate disc, then come. */
	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,(twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]<<1));
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;
		PlayersWeaponHModelController.LoopAfterTweening=0;
	}

	if ((PlayersWeaponHModelController.Tweening==Controller_NoTweening)
		&&(PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1))) {
	
		SECTION_DATA *disc_section;
		int time;

		if (PlayersWeaponHModelController.Sub_Sequence==PHSS_Come) {
			/* If this is the second time, end. */
			weaponPtr->StateTimeOutCounter = 0;
		} else {
			disc_section=GetThisSectionData(PlayersWeaponHModelController.section_data,"disk");
			GLOBALASSERT(disc_section);
			/* Appear Disc. */
			disc_section->flags&=~section_data_notreal;
	
			time=DIV_FIXED(ONE_FIXED,(twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]<<1));
			time-=(ONE_FIXED>>4);
	
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Come,time,0);
			PlayersWeaponHModelController.Looped=0;
			PlayersWeaponHModelController.LoopAfterTweening=0;
		}
	} else {
		/* Stay in this state. */
		weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);
	}
}

void SpikeyThing_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	/* We must be in the animation. */

	/* Maintain this state... */
	weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);
	GLOBALASSERT(PlayersWeaponHModelController.Sub_Sequence==PHSS_Attack_Secondary);

	/* Force Decloak. */
	if (playerStatusPtr->cloakOn) {
		playerStatusPtr->cloakOn=0;
		Sound_Play(SID_PRED_CLOAKOFF,"h");
		//playerNoise=1;
	}
	
	/* ...Unless we're at the end. */
	if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		/* End state. */
		weaponPtr->StateTimeOutCounter = 0;
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {
		if (Player->ObStrategyBlock->SBDamageBlock.IsOnFire) {
			Player->ObStrategyBlock->SBDamageBlock.IsOnFire=0;
		}
	}

	if (PlayersWeaponHModelController.keyframe_flags&2) {
		NPC_DATA *NpcData;
	
		switch (AvP.Difficulty) {
			case I_Easy:
				NpcData=GetThisNpcData(I_PC_Predator_Easy);
				break;
			default:
			case I_Medium:
				NpcData=GetThisNpcData(I_PC_Predator_Medium);
				break;
			case I_Hard:
				NpcData=GetThisNpcData(I_PC_Predator_Hard);
				break;
			case I_Impossible:
				NpcData=GetThisNpcData(I_PC_Predator_Impossible);
				break;
		}
		LOCALASSERT(NpcData);

		/* Ouch. */
		Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		playerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;

		/* Now yell a bit? */
		if (playerStatusPtr->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(playerStatusPtr->soundHandle);
		}
		Sound_Play(SID_PRED_NEWROAR,"hev",&playerStatusPtr->soundHandle,VOLUME_MAX);
		if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_Medicomp_Special;
		playerNoise=1;

		if (GREENFLASH_INTENSITY>PlayerDamagedOverlayIntensity)
		{
			PlayerDamagedOverlayIntensity=GREENFLASH_INTENSITY;
		}

		{
			/* Auto change to wristblade? */
			int slot;

			/* Try flashback weapon... */
			slot=playerStatusPtr->PreviouslySelectedWeaponSlot;
			if (slot==-1) {
				slot=SlotForThisWeapon(WEAPON_PRED_WRISTBLADE);
			} 
			if (slot==-1) {
				/* Argh! Whadda ya mean, you've got no wristblade? */
				GLOBALASSERT(0);
			}

			{
		    	playerStatusPtr->SwapToWeaponSlot = slot;
				weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
			    weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
			}
		}
	}
}

void Extinguisher_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	/* We must be in the animation. */

	/* Maintain this state... */
	weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);
	GLOBALASSERT(PlayersWeaponHModelController.Sub_Sequence==PHSS_Attack_Primary);

	/* Force Decloak. */
	if (playerStatusPtr->cloakOn) {
		playerStatusPtr->cloakOn=0;
		Sound_Play(SID_PRED_CLOAKOFF,"h");
		//playerNoise=1;
	}
	
	/* ...Unless we're at the end. */
	if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		/* End state. */
		weaponPtr->StateTimeOutCounter = 0;
	}

	if (PlayersWeaponHModelController.keyframe_flags&1) {
		if (Player->ObStrategyBlock->SBDamageBlock.IsOnFire) {
			Player->ObStrategyBlock->SBDamageBlock.IsOnFire=0;
		}
	}
	
	/* That's all, folks. */
}

int FireSpikeyThing(PLAYER_WEAPON_DATA *weaponPtr) {

    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	NPC_DATA *NpcData;
	
	switch (AvP.Difficulty) {
		case I_Easy:
			NpcData=GetThisNpcData(I_PC_Predator_Easy);
			break;
		default:
		case I_Medium:
			NpcData=GetThisNpcData(I_PC_Predator_Medium);
			break;
		case I_Hard:
			NpcData=GetThisNpcData(I_PC_Predator_Hard);
			break;
		case I_Impossible:
			NpcData=GetThisNpcData(I_PC_Predator_Impossible);
			break;
	}
	LOCALASSERT(NpcData);
	LOCALASSERT(playerStatusPtr);
	
	#if QUIRKAFLEEG
	if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
    	if (weaponPtr->PrimaryRoundsRemaining<ONE_FIXED) {
			return(FireExtinguisher(weaponPtr));
		}
	}
	#endif

	if (playerStatusPtr->FieldCharge<=MEDICOMP_USE_THRESHOLD) {
		return(0);
	}

	if ((Player->ObStrategyBlock->SBDamageBlock.Health==NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)
		&&(Player->ObStrategyBlock->SBDamageBlock.IsOnFire==0)) {
		return(0);	
	}

	#if QUIRKAFLEEG
	if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
    	if (weaponPtr->PrimaryRoundsRemaining<ONE_FIXED) {
			return(0);
		} else {
	    	weaponPtr->PrimaryRoundsRemaining-=ONE_FIXED;
		}
	}
	#endif

	if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
		/* Force Decloak. */
		playerStatusPtr->cloakOn=0;
		Sound_Play(SID_PRED_CLOAKOFF,"h");
		//playerNoise=1;
	}

	GLOBALASSERT(AvP.PlayerType==I_Predator);

	/* No longer will we do that wussey instant healing thing! */
	playerStatusPtr->FieldCharge-=MEDICOMP_DRAIN_BLOCK;
	CurrentGameStats_ChargeUsed(MEDICOMP_DRAIN_BLOCK);
	/* But we do do the instant charging thing. */
	LOCALASSERT(playerStatusPtr->FieldCharge>=0);

	GLOBALASSERT(PlayersWeaponHModelController.section_data);
	InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Secondary,-1,0);

	return(1);

}

int FireExtinguisher(PLAYER_WEAPON_DATA *weaponPtr) {

    TEMPLATE_WEAPON_DATA *twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	
	/* Ha ha ha.  I made a funny! */

	LOCALASSERT(playerStatusPtr);
	
	if (playerStatusPtr->FieldCharge<=EXTINGUISHER_USE_THRESHOLD) {
		return(0);
	}

	if (Player->ObStrategyBlock->SBDamageBlock.IsOnFire==0) {
		return(0);	
	}
	

	if ( (twPtr->FireWhenCloaked==0)&&(playerStatusPtr->cloakOn) ) {
		/* Force Decloak. */
		playerStatusPtr->cloakOn=0;
		Sound_Play(SID_PRED_CLOAKOFF,"h");
		//playerNoise=1;
	}

	GLOBALASSERT(AvP.PlayerType==I_Predator);

	/* No longer will we do that wussey instant healing thing! */
	playerStatusPtr->FieldCharge-=EXTINGUISHER_DRAIN_BLOCK;
	CurrentGameStats_ChargeUsed(EXTINGUISHER_DRAIN_BLOCK);
	/* But we do do the instant charging thing. */
	LOCALASSERT(playerStatusPtr->FieldCharge>=0);

	GLOBALASSERT(PlayersWeaponHModelController.section_data);
	InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);

	return(1);

}

void Staff_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_IN]);

		GLOBALASSERT(time!=0);

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Come,time);
		PlayersWeaponHModelController.Looped=0;
	}
	/* Handle staff sections. */
	PlayerStaff1=GetThisSectionData(PlayersWeaponHModelController.section_data,"Staff R blade");
	PlayerStaff2=GetThisSectionData(PlayersWeaponHModelController.section_data,"Staff ROOT");
	PlayerStaff3=GetThisSectionData(PlayersWeaponHModelController.section_data,"Staff L blade");
	StaffAttack=-1;

}

void Staff_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_OUT]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_PredatorHUD,(int)PHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;
	}
	/* Handle staff sections. */
	PlayerStaff1=NULL;
	PlayerStaff2=NULL;
	PlayerStaff3=NULL;
	StaffAttack=-1;
}

void Staff_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	/* Are we running? */

	if ( (Player->ObStrategyBlock->DynPtr->LinVelocity.vx!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vy!=0)
		|| (Player->ObStrategyBlock->DynPtr->LinVelocity.vz!=0) ) {

		/* Were we running? */

		if (PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Run) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Run,ONE_FIXED,1);
		}
	} else {
	
		if ((PlayersWeaponHModelController.Sub_Sequence!=(int)PHSS_Stand)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
		}
		
		if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
		
			if (WeaponFidgetPlaying) {
				if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Stand,ONE_FIXED,1);
					WeaponFidgetPlaying=0;
				}
			} else if ((FastRandom()&255)==0) {
				/* Start animation. */
				#if 1
				if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_PredatorHUD,(int)PHSS_Fidget)) {
					InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_PredatorHUD,(int)PHSS_Fidget,-1,0);
					weaponPtr->StateTimeOutCounter=0;
					WeaponFidgetPlaying=1;
				}
				#endif
			}
		
		}
	}
}

void StaffAttack_Basic(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_PredatorHUD,(int)PHSS_Attack_Primary,-1,0);
		if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
			StaffAttack=1;
		} else {
			StaffAttack=0;
		}	
	} else if (HModelAnimation_IsFinished(&PlayersWeaponHModelController)) {
		/* End state. */
		weaponPtr->StateTimeOutCounter = 0;
		StaffAttack=-1;
	} else {
		enum AMMO_ID AmmoID=TemplateWeapon[weaponPtr->WeaponIDNumber].PrimaryAmmoID;
		/*  In the middle. */
		weaponPtr->StateTimeOutCounter = (WEAPONSTATE_INITIALTIMEOUTCOUNT>>1);

		Staff_Manager(&TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty],PlayerStaff1,PlayerStaff2,PlayerStaff3,
			Player->ObStrategyBlock);

	}

}

int PointIsInPlayer(VECTORCH *point) {
	/* Special case. */
	VECTORCH max,min;

	max.vx=Player->ObShapeData->shapemaxx;
	min.vx=Player->ObShapeData->shapeminx;
	max.vy=Player->ObShapeData->shapemaxy;
	min.vy=Player->ObShapeData->shapeminy;
	max.vz=Player->ObShapeData->shapemaxz;
	min.vz=Player->ObShapeData->shapeminz;

	RotateVector(&max,&Player->ObMat);
	RotateVector(&min,&Player->ObMat);

	max.vx+=Player->ObStrategyBlock->DynPtr->Position.vx;
	max.vy+=Player->ObStrategyBlock->DynPtr->Position.vy;
	max.vz+=Player->ObStrategyBlock->DynPtr->Position.vz;

	min.vx+=Player->ObStrategyBlock->DynPtr->Position.vx;
	min.vy+=Player->ObStrategyBlock->DynPtr->Position.vy;
	min.vz+=Player->ObStrategyBlock->DynPtr->Position.vz;
	/* Now test. */
	if ((point->vx>min.vx)&&(point->vx<max.vx)&&
		(point->vy>min.vy)&&(point->vy<max.vy)&&
		(point->vz>min.vz)&&(point->vz<max.vz)) {
		/* Success. */
		return(1);
	}

	return(0);
}

int IsPointInsideObject(VECTORCH *point, STRATEGYBLOCK *sbPtr, SECTION_DATA **hit_section) {
	
	DISPLAYBLOCK *dPtr;

	dPtr=sbPtr->SBdptr;

	if (dPtr==Player) {
		GLOBALASSERT(dPtr);
	}

	if (dPtr==NULL) {
		*hit_section=NULL;
		return(0);
	}
	if (dPtr->SfxPtr) {
		*hit_section=NULL;
		return(0);
	}
	if (dPtr==Player) {
		/* Special case for player. */
		if (PointIsInPlayer(point)) {
			/* Hit! */
			*hit_section=NULL;
			return(1);
		}
	} else if (dPtr->HModelControlBlock) {
		SECTION_DATA *hs;
		/* Hierarchy case. */
		hs=PointInHModel(dPtr->HModelControlBlock,point);
		if (hs) {
			*hit_section=hs;
			return(1);
		} else {
			*hit_section=NULL;
			return(0);
		}
	} else {
		VECTORCH offset;
		int dist;
		/* Use shape data. */
		if (dPtr->ObShapeData==NULL) {
			*hit_section=NULL;
			return(0);
		}
		offset.vx=dPtr->ObWorld.vx-point->vx;
		offset.vy=dPtr->ObWorld.vy-point->vy;
		offset.vz=dPtr->ObWorld.vz-point->vz;
		dist=Approximate3dMagnitude(&offset);
		if (dist<dPtr->ObShapeData->shaperadius) {
			/* Hit! */
			*hit_section=NULL;
			return(1);
		}
	}
	*hit_section=NULL;
	return(0);
}

int Staff_Manager(DAMAGE_PROFILE *damage,SECTION_DATA *section1,SECTION_DATA *section2,SECTION_DATA *section3,
	STRATEGYBLOCK *wielder)	{
	
	SECTION_DATA *hit_section;
	int numberOfObjects = NumActiveBlocks;
	int numhits;
	int hitatall=0;

	GLOBALASSERT(wielder->SBdptr);
	
	while (numberOfObjects)
	{
		DISPLAYBLOCK* objectPtr = ActiveBlockList[--numberOfObjects];
		STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;
		GLOBALASSERT(objectPtr);
		
		numhits=0;

		if (objectPtr==Player) {
			GLOBALASSERT(objectPtr);
			/* So I can breakpoint it. */
		}

		/* does object have a strategy block? */
		if ((sbPtr)&&(sbPtr!=wielder))
		{		

			DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			
			if (dynPtr) {

				if (IsThisObjectVisibleFromThisPosition_WithIgnore(objectPtr,wielder->SBdptr,&wielder->DynPtr->Position,10000)) {

					/* Deduce if each section has hit. */
			
					if (IsPointInsideObject(&section1->World_Offset,sbPtr,&hit_section)) {
						numhits++;
					} else if (IsPointInsideObject(&section2->World_Offset,sbPtr,&hit_section)) {
						numhits++;
					} else if (IsPointInsideObject(&section3->World_Offset,sbPtr,&hit_section)) {
						numhits++;
					}

				}
			}
		}

		if (numhits) {
			hitatall++;
			if (sbPtr->SBdptr->HModelControlBlock) {
				#if 0
				if (hit_section==NULL) {
					HtoHDamageToHModel(sbPtr, damage, NormalFrameTime, NULL, NULL);
				} else {
					CauseDamageToHModel(sbPtr->SBdptr->HModelControlBlock, sbPtr, damage, NormalFrameTime, hit_section,NULL,NULL,0);
				}
				#else
				HtoHDamageToHModel(sbPtr, damage, NormalFrameTime, NULL, NULL);
				#endif
			} else {
				CauseDamageToObject(sbPtr, damage, NormalFrameTime,NULL);
			}
		}

	}
	return(hitatall);
}

/* Cheat mode in here! */
#define BITE_RANGE 	((SNIPERMUNCH_MODE)? 100000:3000)
#define BITE_RADIUS	((SNIPERMUNCH_MODE)? 1500:200)

#define TROPHY_RANGE 	(3000)
#define TROPHY_RADIUS	(200)

SECTION_DATA *CheckBiteIntegrity(void) {

	VECTORCH targetpos;
	DISPLAYBLOCK *objectPtr;

	if (Biting==NULL) {
		return(NULL);
	}
	if (!(Validate_Strategy(Biting,Biting_SBname))) {
		return(NULL);
	}
	if (!(Biting->SBdptr)) {
		/* Gone far? */
		return(NULL);
	}
	objectPtr=Biting->SBdptr;

	GetTargetingPointOfObject(objectPtr,&targetpos);
	targetpos.vx-=Global_VDB_Ptr->VDB_World.vx;
	targetpos.vy-=Global_VDB_Ptr->VDB_World.vy;
	targetpos.vz-=Global_VDB_Ptr->VDB_World.vz;
	RotateVector(&targetpos,&Global_VDB_Ptr->VDB_Mat);
	
	/* is it in the range band? */
	if ((targetpos.vz >0) 
		&& (targetpos.vz <  (BITE_RANGE<<1))) {
	
		DYNAMICSBLOCK *dynPtr = Biting->DynPtr;
		GLOBALASSERT(dynPtr);
		
		if (IsThisObjectVisibleFromThisPosition_WithIgnore(objectPtr,Player,&Global_VDB_Ptr->VDB_World,(BITE_RANGE<<1))) {
	
			SECTION_DATA *head;
			/* The minute his *head* is in view... */
			head=GetThisSectionData(objectPtr->HModelControlBlock->section_data,"head");
			if (head!=NULL) {
				if (head->flags&section_data_notreal) {
					/* Is it still attached? */
					head=NULL;
				}
			}
			/* We'll have the head now if at all. */
			if (head) {
				VECTORCH temp_view;
				int dist;
				/* I assume if we're alive, we have a head. */
				temp_view=head->View_Offset;
				temp_view.vz=0;
				dist=Approximate3dMagnitude(&temp_view);
				/* Scale radius with FOV here? */
				if (dist<(BITE_RADIUS<<1)) {
					/* Aw, okay. */
					return(head);
				}
			}
		}
	}
	/* If you got here, you failed. */
	return(NULL);
}

int AlienBite_TargetFilter(STRATEGYBLOCK *sbPtr) {

	/* Is it tasty, mm?  Is it... crunchable? */

	if ((sbPtr->I_SBtype==I_BehaviourMarine)
		||(sbPtr->I_SBtype==I_BehaviourSeal))
	{
		MARINE_STATUS_BLOCK *marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
		GLOBALASSERT(marineStatusPointer);
					
		if (!marineStatusPointer->Android && !NPC_IsDead(sbPtr))
		{
			return(1);
		}
	}
	else if (sbPtr->I_SBtype==I_BehaviourNetCorpse)
	{
		NETCORPSEDATABLOCK *corpseDataPtr;

		corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(corpseDataPtr);
	
		if (((corpseDataPtr->timer/2)<ONE_FIXED)||corpseDataPtr->Android)
		{
			/* Already fading. */
			return(0);
		}
		else if ((corpseDataPtr->Type==I_BehaviourMarine)
			||(corpseDataPtr->Type==I_BehaviourSeal)
			||(corpseDataPtr->Type==I_BehaviourPredator)
			)
		{
			/* Of course it's dead! */
			return(1);
		}
	} else if (sbPtr->I_SBtype==I_BehaviourNetGhost) {
		NETGHOSTDATABLOCK* ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(ghostData);
		
		//check for opponent in network game
		if(ghostData->type==I_BehaviourNetCorpse)
		{
			if(ghostData->subtype==I_BehaviourMarinePlayer ||
			   ghostData->subtype==I_BehaviourPredatorPlayer)
			{
				return 1;
			}
		}
		else if (ghostData->type==I_BehaviourMarinePlayer || 
				 ghostData->type==I_BehaviourPredatorPlayer)
		{
			// We don't want to allow bite attacks against players
			// that have respawn invulnerability
			if(!ghostData->invulnerable)
			{
				return 1;
			}
		}
	} else if (sbPtr->I_SBtype==I_BehaviourPredator) {

		/* There's always got to be a difficult one! */
		SECTION_DATA *head;

		if (!sbPtr->SBdptr) {
			return(0);
		}
		/* The minute his *head* is in view... */
		head=GetThisSectionData(sbPtr->SBdptr->HModelControlBlock->section_data,"head");
		/* Is it still attached? */
		if (head) {
			if (head->flags&section_data_notreal) {
				head=NULL;
			}
		}
		/* We'll have the head now if at all. */
		if (head) {
			DAMAGEBLOCK temp_damage;
			
			temp_damage=head->current_damage;
			
			DamageDamageBlock(&temp_damage,&TemplateAmmo[AMMO_PC_ALIEN_BITE].MaxDamage[AvP.Difficulty],ONE_FIXED);

			if (temp_damage.Health<=0) {
				/* Killed the head, that'll do. */
				return(1);
			}
			/* So the head lived, huh?  Let's try the body. */
			temp_damage=sbPtr->SBDamageBlock;
			DamageDamageBlock(&temp_damage,&TemplateAmmo[AMMO_PC_ALIEN_BITE].MaxDamage[AvP.Difficulty],ONE_FIXED);

			if (temp_damage.Health<=0) {
				/* Killed the body, that'll do too. */
				return(1);
			}

		}
		return(0);
	}
	
	return(0);

}

STRATEGYBLOCK *GetBitingTarget(void) {

	/* Sweep OnScreenBlockList. */

	int numberOfObjects = NumOnScreenBlocks;
	
	while (numberOfObjects)
	{
		STRATEGYBLOCK *sbPtr;
		VECTORCH targetpos;
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];

		GLOBALASSERT(objectPtr);
		sbPtr = objectPtr->ObStrategyBlock;
		
		/* does object have a strategy block? */
		if (sbPtr) {
	
			if (AlienBite_TargetFilter(sbPtr)) {		
					
				GLOBALASSERT(objectPtr->HModelControlBlock);

				GetTargetingPointOfObject(objectPtr,&targetpos);
				targetpos.vx-=Global_VDB_Ptr->VDB_World.vx;
				targetpos.vy-=Global_VDB_Ptr->VDB_World.vy;
				targetpos.vz-=Global_VDB_Ptr->VDB_World.vz;
				RotateVector(&targetpos,&Global_VDB_Ptr->VDB_Mat);
				
				/* is it in the range band? */
				if ((targetpos.vz >0) 
					&& (targetpos.vz <  BITE_RANGE)) {
				
					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
				  	if (dynPtr) {
				
						if (IsThisObjectVisibleFromThisPosition_WithIgnore(objectPtr,Player,&Global_VDB_Ptr->VDB_World,(BITE_RANGE<<1))) {
				
							SECTION_DATA *head;
							/* The minute his *head* is in view... */
							head=GetThisSectionData(objectPtr->HModelControlBlock->section_data,"head");
							/* Is it still attached? */
							if (head) {
								if (head->flags&section_data_notreal) {
									head=NULL;
								}
							}
							/* We'll have the head now if at all. */
							if (head) {
								VECTORCH temp_view;
								int dist;
								/* I assume if we're alive, we have a head. */
								temp_view=head->View_Offset;
								temp_view.vz=0;
								dist=Approximate3dMagnitude(&temp_view);
								if (dist<BITE_RADIUS) {
									/* Aw, okay. */
									return(sbPtr);
								}
							}
						}
					}
				}
			}
		}
	}

	return(NULL);

}

int Trophy_TargetFilter(STRATEGYBLOCK *sbPtr) {

	/* Only works on dead'uns. */

	if (sbPtr->I_SBtype==I_BehaviourNetCorpse)
	{
		NETCORPSEDATABLOCK *corpseDataPtr;

		corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(corpseDataPtr);
	
		if ((corpseDataPtr->timer/2)<ONE_FIXED)
		{
			/* Already fading. */
			return(0);
		}
		else if ((corpseDataPtr->Type==I_BehaviourMarine)
			||(corpseDataPtr->Type==I_BehaviourSeal)
			||(corpseDataPtr->Type==I_BehaviourAlien)
			||(corpseDataPtr->Type==I_BehaviourPredator)
			)
		{
			/* Of course it's dead! */
			return(1);
		}
	} else if (sbPtr->I_SBtype==I_BehaviourNetGhost) {
		NETGHOSTDATABLOCK* ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(ghostData);
		
		//check for opponent in network game
		if(ghostData->type==I_BehaviourNetCorpse)
		{
			if(ghostData->subtype==I_BehaviourMarinePlayer ||
			   ghostData->subtype==I_BehaviourPredatorPlayer ||
			   ghostData->subtype==I_BehaviourAlienPlayer)
			{
				return 1;
			}
		}
	} else if (sbPtr->I_SBtype==I_BehaviourHierarchicalFragment) {

		DISPLAYBLOCK *objectPtr;
		HDEBRIS_BEHAV_BLOCK *debrisDataPtr;

		debrisDataPtr = (HDEBRIS_BEHAV_BLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(debrisDataPtr);

		/* Check type. */
		
		if ((debrisDataPtr->Type==I_BehaviourMarine)
			||(debrisDataPtr->Type==I_BehaviourSeal)
			||(debrisDataPtr->Type==I_BehaviourAlien)
			||(debrisDataPtr->Type==I_BehaviourPredator)
			) {
			
			objectPtr=sbPtr->SBdptr;
			if (objectPtr) {
				if (objectPtr->HModelControlBlock) {
					SECTION_DATA *head;
					/* Test for the head... */
					head=GetThisSectionData(objectPtr->HModelControlBlock->section_data,"head");
					/* Is it still attached? */
					if (head) {
						if (head->flags&section_data_notreal) {
							head=NULL;
						}
					}
					if (head) {
						if ((HModel_DepthTest(objectPtr->HModelControlBlock,head,1))==0) {
							/* Fragment is big enough. */
							return(1);
						}
					}
				}
			}
		}
	} else if (sbPtr->I_SBtype==I_BehaviourSpeargunBolt) {

		DISPLAYBLOCK *objectPtr;
		SPEAR_BEHAV_BLOCK *spearDataPtr;

		spearDataPtr = (SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(spearDataPtr);

		/* Check type. */
		
		if ((spearDataPtr->Type==I_BehaviourMarine)
			||(spearDataPtr->Type==I_BehaviourSeal)
			||(spearDataPtr->Type==I_BehaviourAlien)
			||(spearDataPtr->Type==I_BehaviourPredator)
			) {
			
			objectPtr=sbPtr->SBdptr;
			if (objectPtr) {
				if (objectPtr->HModelControlBlock) {
					SECTION_DATA *head;
					/* Test for the head... */
					head=GetThisSectionData(objectPtr->HModelControlBlock->section_data,"head");
					/* Is it still attached? */
					if (head) {
						if (head->flags&section_data_notreal) {
							head=NULL;
						}
					}
					if (head) {
						if ((HModel_DepthTest(objectPtr->HModelControlBlock,head,1))==0) {
							/* Fragment is big enough. */
							return(1);
						}
					}
				}
			}
		}
	}
	
	return(0);

}

STRATEGYBLOCK *GetTrophyTarget(SECTION_DATA **head_section_data) {

	/* Sweep OnScreenBlockList. */

	int numberOfObjects = NumOnScreenBlocks;

	*head_section_data=NULL;
	
	while (numberOfObjects)
	{
		STRATEGYBLOCK *sbPtr;
		VECTORCH targetpos;
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];

		GLOBALASSERT(objectPtr);
		sbPtr = objectPtr->ObStrategyBlock;
		
		/* does object have a strategy block? */
		if (sbPtr) {
	
			if (Trophy_TargetFilter(sbPtr)) {		
					
				GLOBALASSERT(objectPtr->HModelControlBlock);

				GetTargetingPointOfObject(objectPtr,&targetpos);
				targetpos.vx-=Global_VDB_Ptr->VDB_World.vx;
				targetpos.vy-=Global_VDB_Ptr->VDB_World.vy;
				targetpos.vz-=Global_VDB_Ptr->VDB_World.vz;
				RotateVector(&targetpos,&Global_VDB_Ptr->VDB_Mat);
				
				/* is it in the range band? */
				if ((targetpos.vz >0) 
					&& (targetpos.vz <  TROPHY_RANGE)) {
				
					DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
				  	if (dynPtr) {
				
						//if (IsThisObjectVisibleFromThisPosition_WithIgnore(objectPtr,Player,&Global_VDB_Ptr->VDB_World,(TROPHY_RANGE<<1))) {
						if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&dynPtr->Position)) {
				
							SECTION_DATA *head;
							/* The minute his *head* is in view... */
							head=GetThisSectionData(objectPtr->HModelControlBlock->section_data,"head");
							/* Is it still attached? */
							if (head) {
								if (head->flags&section_data_notreal) {
									head=NULL;
								}
							}
							/* We'll have the head now if at all. */
							if (head) {
								VECTORCH temp_view;
								int dist;
								/* I assume if we're alive, we have a head. */
								temp_view=head->View_Offset;
								temp_view.vz=0;
								dist=Approximate3dMagnitude(&temp_view);
								if (dist<TROPHY_RADIUS) {
									/* Aw, okay. */
									*head_section_data=head;
									return(sbPtr);
								}
							}
						}
					}
				}
			}
		}
	}

	return(NULL);

}

extern void AutoSwapToDisc(void) {

	int slot;
	PLAYER_WEAPON_DATA *weaponPtr;
 
    weaponPtr = &(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot]);
			
	slot=SlotForThisWeapon(WEAPON_PRED_DISC);
	if (slot==-1) {
		/* Maybe you're not a predator... */
		return;
   	#if 0
	} else if (weaponPtr->CurrentState==WEAPONSTATE_IDLE) {
        PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo=(slot+1);
   	#else
	} else { 
		AutoSwap=slot;
   	#endif
	}

}

extern void AutoSwapToDisc_OutOfSequence(void) {

	int slot;
	PLAYER_WEAPON_DATA *weaponPtr;
 
    weaponPtr = &(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot]);
			
	slot=SlotForThisWeapon(WEAPON_PRED_DISC);

	if (slot!=-1) {
		AutoSwap=slot;
	}

}

#define SPEAR_NPC_IMPULSE	(20000)

void HandleSpearImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data) 
{
	VECTORCH incoming,*invec;
	DISPLAYBLOCK *fragged_section;

	fragged_section=NULL;

	if(sbPtr)  
	{
		if (sbPtr->DynPtr) {
			MATRIXCH WtoLMat;
			/* Consider incoming hit direction. */
			WtoLMat=sbPtr->DynPtr->OrientMat;
			TransposeMatrixCH(&WtoLMat);
			RotateAndCopyVector(directionPtr,&incoming,&WtoLMat);
			invec=&incoming;
		} else {
			invec=NULL;
		}

		if (this_section_data)
		{
			if (sbPtr->SBdptr)
			{
				//if (sbPtr->SBdptr->HModelControlBlock && (sbPtr->I_SBtype != I_BehaviourNetGhost))
				if (sbPtr->SBdptr->HModelControlBlock)
				{
					/* Netghost case now handled properly. */
					AddDecalToHModel(&LOS_ObjectNormal, &LOS_Point,this_section_data);

					GLOBALASSERT(sbPtr->SBdptr->HModelControlBlock==this_section_data->my_controller);
					fragged_section=CauseDamageToHModel(sbPtr->SBdptr->HModelControlBlock, sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple*ONE_FIXED, this_section_data,invec,positionPtr,0);
					/* No longer return: do knockback. */
					//return;
				} 
				else
				{
		  			CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple*ONE_FIXED,invec);
				}
			}
			else
			{
		  		CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple*ONE_FIXED,invec);
			}
		}
		else
		{
		  	CauseDamageToObject(sbPtr, &TemplateAmmo[AmmoID].MaxDamage[AvP.Difficulty], multiple*ONE_FIXED,invec);
		}

		if (sbPtr->DynPtr)
		{
			DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
			/* Forget torque for the moment. */

			/*
			Knock the character back - unless it is the alien queen.
			*/
			if(sbPtr->I_SBtype!=I_BehaviourQueenAlien)
			{
				dynPtr->LinImpulse.vx+=MUL_FIXED(directionPtr->vx,(SPEAR_NPC_IMPULSE));
				dynPtr->LinImpulse.vy+=MUL_FIXED(directionPtr->vy,(SPEAR_NPC_IMPULSE));
				dynPtr->LinImpulse.vz+=MUL_FIXED(directionPtr->vz,(SPEAR_NPC_IMPULSE));
			}

			if (fragged_section) {

				fragged_section->ObStrategyBlock->DynPtr->LinImpulse.vx+=MUL_FIXED(directionPtr->vx,SPEAR_NPC_IMPULSE);
				fragged_section->ObStrategyBlock->DynPtr->LinImpulse.vy+=MUL_FIXED(directionPtr->vy,SPEAR_NPC_IMPULSE);
				fragged_section->ObStrategyBlock->DynPtr->LinImpulse.vz+=MUL_FIXED(directionPtr->vz,SPEAR_NPC_IMPULSE);
			} 
		}


	}


	/* check to see if we've shot off a body part, or we're stuck into the environment */
	{
		char needToAddSpear = 0;

		if (fragged_section)
		{
			needToAddSpear = 1;
		}
		else if (sbPtr)
		{
			DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;

			if (dispPtr)
			if (dispPtr->ObMyModule && (!dispPtr->ObMorphCtrl))
			{
				needToAddSpear=1;
			}
		}
		else
		{
			needToAddSpear = 1;
		}
		if (needToAddSpear) CreateSpearPossiblyWithFragment(fragged_section,&LOS_Point,directionPtr);
	}
}


void CreateSpearPossiblyWithFragment(DISPLAYBLOCK *dispPtr, VECTORCH *spearPositionPtr, VECTORCH *spearDirectionPtr)
{

	if (dispPtr)
	{
		STRATEGYBLOCK *sbPtr;
		DYNAMICSBLOCK *dynPtr;
		SPEAR_BEHAV_BLOCK *newBehaviourDataBlock;
		int android;

		AVP_BEHAVIOUR_TYPE Type;
		int SubType;

		sbPtr = dispPtr->ObStrategyBlock;
		LOCALASSERT(sbPtr);
		
		dynPtr = sbPtr->DynPtr;
		LOCALASSERT(dynPtr);

		LOCALASSERT(sbPtr->I_SBtype == I_BehaviourHierarchicalFragment);
	
		newBehaviourDataBlock=(SPEAR_BEHAV_BLOCK*)AllocateMem(sizeof(SPEAR_BEHAV_BLOCK));
	
		android=((HDEBRIS_BEHAV_BLOCK *) sbPtr->SBdataptr)->Android;

		Splice_HModels(&(newBehaviourDataBlock->HierarchicalFragment),(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController).section_data);

		Type=((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type;
		SubType=((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType;

		/* we don't need the old extra sb data */
		Dispel_HModel(&(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController));
		DeallocateMem(sbPtr->SBdataptr);

		dispPtr->HModelControlBlock = &(newBehaviourDataBlock->HierarchicalFragment);
		
		sbPtr->SBdataptr =(void*) newBehaviourDataBlock;

		if (!sbPtr->SBdataptr) 
		{
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(sbPtr);
			return;
		}
		sbPtr->I_SBtype = I_BehaviourSpeargunBolt;
		
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->counter = 5*ONE_FIXED;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Stuck = 0;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->SpearThroughFragment = 1;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Android = android;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Type = Type;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->SubType = SubType;


		dynPtr->LinVelocity.vx = spearDirectionPtr->vx/4;
		dynPtr->LinVelocity.vy = spearDirectionPtr->vy/4;
		dynPtr->LinVelocity.vz = spearDirectionPtr->vz/4;
		dynPtr->LinImpulse.vx = 0;
		dynPtr->LinImpulse.vy = 0;
		dynPtr->LinImpulse.vz = 0;
		dynPtr->AngVelocity.EulerX = 0;
		dynPtr->AngVelocity.EulerY = 0;
		dynPtr->AngVelocity.EulerZ = 0;
		dynPtr->GravityOn = 0;
		dynPtr->StopOnCollision = 1;

		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Position.vx = spearPositionPtr->vx - dynPtr->Position.vx;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Position.vy = spearPositionPtr->vy - dynPtr->Position.vy;
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Position.vz = spearPositionPtr->vz - dynPtr->Position.vz;
		{
			MATRIXCH mat;
			MakeMatrixFromDirection(spearDirectionPtr,&mat);
			((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->Orient = mat;
		}		

		/* Ooooh!  Not initialised at all! CDF 23/9/98, hassled about an assert */
		/* KJL 10:37:56 05/10/98 - Just keeping you on your toes :) */
		
		InitHModelTweening(&(((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->HierarchicalFragment),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
		ProveHModel(dispPtr->HModelControlBlock,dispPtr);
		/* And just for now... */
		((SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr)->HierarchicalFragment.Playing=0;

		/* Baaaad Kevin!  And why couldn't you have called the HModelController 'HModelController' like God intended? */
		/* KJL 10:38:17 05/10/98 - Hey, I was trying to be descriptive. */

	    Sound_Play(SID_PLASMABOLT_HIT,"d",&dynPtr->Position);  
		
	}
	else /* there is no hierarchical fragment; we've stuck into the environment */
	{
		DYNAMICSBLOCK *dynPtr;

		dispPtr = MakeObject(I_BehaviourSpeargunBolt,spearPositionPtr);
		if (dispPtr == 0) return;		 // Failed to allocate display block
		
		/* KJL 17:53:36 01/08/98 - make the extents teeny-weeny */
		dispPtr->ObMaxX = 10;
		dispPtr->ObMaxY = 10;
		dispPtr->ObMaxZ = 10;
		dispPtr->ObMinX = -10;
		dispPtr->ObMinY = -10;
		dispPtr->ObMinZ = -10;

		/* setup dynamics block */
		dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		dynPtr->DynamicsType = DYN_TYPE_NO_COLLISIONS;
		
		if (!dynPtr) 
		{
			// Failed to allocate a dynamics block
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}

		dispPtr->ObStrategyBlock->DynPtr = dynPtr;

		/* give missile a maximum lifetime */
		dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(SPEAR_BEHAV_BLOCK));
		
		if (dispPtr->ObStrategyBlock->SBdataptr == 0) 
		{
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}
		memset(dispPtr->ObStrategyBlock->SBdataptr,0,sizeof(SPEAR_BEHAV_BLOCK));

		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 20*ONE_FIXED;
		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->Stuck = 1;
		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->SpearThroughFragment = 0;
		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->Android = 0;
		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->Type = I_BehaviourNull;
		((SPEAR_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->SubType = 0;

		dynPtr->Position= *spearPositionPtr;
		dynPtr->PrevPosition= *spearPositionPtr;
		{
			MATRIXCH mat;
			MakeMatrixFromDirection(spearDirectionPtr,&mat);
			dynPtr->OrientMat = mat;
			MatrixToEuler(&mat,&dynPtr->OrientEuler);
		}		
		dynPtr->PrevOrientMat = dynPtr->OrientMat;
		{
			VECTORCH pos = dynPtr->Position;
			pos.vx += spearDirectionPtr->vx;
			pos.vy += spearDirectionPtr->vy;
			pos.vz += spearDirectionPtr->vz;
			MakeFocusedExplosion(&(dynPtr->Position), &pos, 20, PARTICLE_SPARK);
			MakeLightElement(&(dynPtr->Position),LIGHTELEMENT_ELECTRICAL_SPARKS);
		}

		if(AvP.Network != I_No_Network)
		{
			//send location here , so as to avoid having to update it every frame
			AddNetMsg_LocalObjectState(dispPtr->ObStrategyBlock);
		}
		
		#if 0 // no network yet!
		if(AvP.Network != I_No_Network)	AddNetGameObjectID(dispPtr->ObStrategyBlock);
		#endif
	    Sound_Play(SID_SPEARGUN_HITTING_WALL,"d",&dynPtr->Position);  
		return;
	}	
}

void LimbRip_AwardHealth(void) {

	NPC_DATA *NpcData;
	int health_bonus;

	NpcData=GetThisNpcData(I_PC_Alien_MaxStats);
	LOCALASSERT(NpcData);

	switch (AvP.Difficulty) {
		case I_Easy:
			health_bonus=5;
			break;
		case I_Medium:
		default:
			health_bonus=2;
			break;
		case I_Hard:
			health_bonus=1;
			break;
		case I_Impossible:
			health_bonus=0;
			break;
	}

	/* Now add some health. */
	Player->ObStrategyBlock->SBDamageBlock.Health+=(health_bonus<<ONE_FIXED_SHIFT);
	if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
		Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
	}
	PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;

}

void BiteAttack_AwardHealth(STRATEGYBLOCK *sbPtr,AVP_BEHAVIOUR_TYPE pre_bite_type) {

	NPC_DATA *NpcData;

	/* Have all your armour back? */
	NpcData=GetThisNpcData(I_PC_Alien_MaxStats);
	LOCALASSERT(NpcData);

	if ((pre_bite_type==I_BehaviourMarine)
		||(pre_bite_type==I_BehaviourSeal)
		||(pre_bite_type==I_BehaviourPredator)) {

		/* Add some armour... */
		Player->ObStrategyBlock->SBDamageBlock.Armour+=(BITE_ARMOUR_RECOVERY<<ONE_FIXED_SHIFT);
		if (Player->ObStrategyBlock->SBDamageBlock.Armour>(NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT)) {
			Player->ObStrategyBlock->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		}
		PlayerStatusPtr->Armour=Player->ObStrategyBlock->SBDamageBlock.Armour;

		/* And some health, too. */
		Player->ObStrategyBlock->SBDamageBlock.Health+=(BITE_HEALTH_RECOVERY<<ONE_FIXED_SHIFT);
		if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
			Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		}
		PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
		
		return;
	}

	if (pre_bite_type==I_BehaviourNetCorpse) {
		/* It probably is still a corpse now... */
		NETCORPSEDATABLOCK *corpseDataPtr;

		corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(corpseDataPtr);

		if ((corpseDataPtr->Type==I_BehaviourMarine)
			||(corpseDataPtr->Type==I_BehaviourSeal)
			||(corpseDataPtr->Type==I_BehaviourPredator)
			) {
			/* No armour, just a bit of health. */
			Player->ObStrategyBlock->SBDamageBlock.Health+=(20<<ONE_FIXED_SHIFT);
			if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
				Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			}
			PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
		}
	}		

	if(pre_bite_type==I_BehaviourNetGhost)	
	{
		//rather higher health awards for getting network opponents
		NETGHOSTDATABLOCK *ghostData;
		NPC_DATA* StartingHealth;
		
			switch (AvP.Difficulty) {
				case I_Easy:
					StartingHealth=GetThisNpcData(I_PC_Alien_Easy);
					break;
				default:
				case I_Medium:
					StartingHealth=GetThisNpcData(I_PC_Alien_Medium);
					break;
				case I_Hard:
					StartingHealth=GetThisNpcData(I_PC_Alien_Hard);
					break;
				case I_Impossible:
					StartingHealth=GetThisNpcData(I_PC_Alien_Impossible);
					break;
			}
		
		ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
		LOCALASSERT(ghostData);

		if(ghostData->type==I_BehaviourNetCorpse)
		{
			if(ghostData->subtype==I_BehaviourMarinePlayer ||
			   ghostData->subtype==I_BehaviourPredatorPlayer)
			{
				/* Add some armour... */
				Player->ObStrategyBlock->SBDamageBlock.Armour+=(BITE_ARMOUR_RECOVERY<<ONE_FIXED_SHIFT);
				if (Player->ObStrategyBlock->SBDamageBlock.Armour>(NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT)) {
					Player->ObStrategyBlock->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
				}
				PlayerStatusPtr->Armour=Player->ObStrategyBlock->SBDamageBlock.Armour;

				/* And some health, too. */
				Player->ObStrategyBlock->SBDamageBlock.Health+=(20<<ONE_FIXED_SHIFT);
				if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
					Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
				}

				//make sure player has at least starting health
				if(Player->ObStrategyBlock->SBDamageBlock.Health<(StartingHealth->StartingStats.Health<<ONE_FIXED_SHIFT))
				{
					Player->ObStrategyBlock->SBDamageBlock.Health=StartingHealth->StartingStats.Health<<ONE_FIXED_SHIFT;
				}

				PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
			}
		}
		else if(ghostData->type==I_BehaviourMarinePlayer ||
				ghostData->type==I_BehaviourPredatorPlayer)
		{
			//killed  player character with a bite attack

			/* Add some armour... */
			Player->ObStrategyBlock->SBDamageBlock.Armour+=(StartingHealth->StartingStats.Armour<<ONE_FIXED_SHIFT);
			if (Player->ObStrategyBlock->SBDamageBlock.Armour>(NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT)) {
				Player->ObStrategyBlock->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			}
			PlayerStatusPtr->Armour=Player->ObStrategyBlock->SBDamageBlock.Armour;

			/* And some health, too. */
			Player->ObStrategyBlock->SBDamageBlock.Health+=(StartingHealth->StartingStats.Health<<ONE_FIXED_SHIFT);
			if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
				Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			}

			PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
		}
		
	}

}

int PlayerFirePredPistolFlechettes(PLAYER_WEAPON_DATA *weaponPtr) {

	extern VECTORCH CentreOfMuzzleOffset;
	VECTORCH *firingpos;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
	
	/* Another cheap copy of the flamethrower function. */
    
	ProveHModel(&PlayersWeaponHModelController,&PlayersWeapon);

	if (playerStatusPtr->FieldCharge>0) {
		playerStatusPtr->FieldCharge-=(NormalFrameTime);
		CurrentGameStats_ChargeUsed(NormalFrameTime);
		if (playerStatusPtr->FieldCharge<0) {
			playerStatusPtr->FieldCharge=0;
		}
	} else {
		return(0);
	}

	if (PWMFSDP) {
		VECTORCH null_vec={0,0,0};
	
		textprint("Hierarchical Flechettes Fire!\n");
	
		firingpos=&PWMFSDP->World_Offset;
		
		FirePredPistolFlechettes(firingpos,&null_vec,&PlayersWeapon.ObMat,0,&Flamethrower_Timer,TRUE);
	
	} else {
		firingpos=&CentreOfMuzzleOffset;
		FirePredPistolFlechettes(&PlayersWeapon.ObWorld,firingpos,&PlayersWeapon.ObMat,1,&Flamethrower_Timer,TRUE);
	}


	return(1);

}

int PredPistolSecondaryFire(PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

	LOCALASSERT(playerStatusPtr);

	if (playerStatusPtr->FieldCharge>=PredPistol_ShotCost) {

		playerStatusPtr->FieldCharge-=PredPistol_ShotCost;
		CurrentGameStats_ChargeUsed(PredPistol_ShotCost);
	    
		FireProjectileAmmo(twPtr->PrimaryAmmoID);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);

		return(1);
	}

	return(0);
}

void ChangeHUDToAlternateShapeSet(char *riffname,char *setname) {

	HIERARCHY_SHAPE_REPLACEMENT* replacement_array;
	int a;
	
	replacement_array=GetHierarchyAlternateShapeSetFromLibrary(riffname,setname);

	if (replacement_array==NULL) {
		return;
	}

	a=0;

	while (replacement_array[a].replaced_section_name!=NULL) {
		SECTION_DATA *target_section;

		target_section=GetThisSectionData(PlayersWeaponHModelController.section_data,
			replacement_array[a].replaced_section_name);
		if (target_section) {
			target_section->Shape=replacement_array[a].replacement_shape;
			#if 1
			target_section->ShapeNum=replacement_array[a].replacement_shape_index;
			#endif
			
			Setup_Texture_Animation_For_Section(target_section);
		}
		a++;
	}
}

int AccuracyStats_TargetFilter(STRATEGYBLOCK *sbPtr) {

	if (sbPtr==NULL) {
		return(0);
	}

	switch (sbPtr->I_SBtype) {
		case I_BehaviourPredator:
			/* Valid. */
			if (NPC_IsDead(sbPtr)) {
				return(1);
			} else {
				return(1);
			}
			break;
		case I_BehaviourAlien:
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourXenoborg:
		case I_BehaviourMarine:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
			/* Valid. */
			if (NPC_IsDead(sbPtr)) {
				return(1);
			} else {
				return(1);
			}
			break;
		case I_BehaviourNetCorpse:
			{
				NETCORPSEDATABLOCK *corpseDataPtr;
			    LOCALASSERT(sbPtr);
				corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
				LOCALASSERT(corpseDataPtr);

				if (corpseDataPtr->validityTimer>0) {
					return(1);
				} else {
					return(0);
				}
			}
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=sbPtr->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourPredatorPlayer:
						return(1);
						break;
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourRocket:
					case I_BehaviourGrenade:
					case I_BehaviourFlareGrenade:
					case I_BehaviourFragmentationGrenade:
					case I_BehaviourClusterGrenade:
					case I_BehaviourNPCPredatorDisc:
					case I_BehaviourPredatorDisc_SeekTrack:
					case I_BehaviourAlien:
					case I_BehaviourProximityGrenade:
						return(1);
						break;
					default:
						return(0);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

	return(0);

}

int FriendlyFireDamageFilter(DAMAGE_PROFILE *damage) {

	BOOL VulnerableToAlienDamage = TRUE;
	BOOL VulnerableToMarineDamage = TRUE;
	BOOL VulnerableToPredatorDamage = TRUE;

	if(AvP.PlayerType == I_Alien)
	{
		VulnerableToAlienDamage = FALSE;
	}
	else if(AvP.PlayerType == I_Marine)
	{
		VulnerableToMarineDamage = FALSE;
		if(netGameData.gameType==NGT_Coop)
		{
			VulnerableToPredatorDamage = FALSE;
		}
	}
	else if(AvP.PlayerType == I_Predator)
	{
		VulnerableToPredatorDamage = FALSE;
		if(netGameData.gameType==NGT_Coop)
		{
			VulnerableToMarineDamage = FALSE;
		}
	}
	
	/* Does this damage type hurt you? */

	switch (damage->Id) {
		case AMMO_ALIEN_CLAW:
		case AMMO_ALIEN_TAIL:
		case AMMO_ALIEN_BITE_KILLSECTION:
		case AMMO_ALIEN_BITE_KILLSECTION_SUPER:
		case AMMO_PC_ALIEN_BITE:
			return(VulnerableToAlienDamage);
			break;
		
		case AMMO_10MM_CULW:
		case AMMO_SMARTGUN:
		case AMMO_FLAMETHROWER:
		case AMMO_SADAR_TOW:
		case AMMO_GRENADE:
		case AMMO_MINIGUN:
		case AMMO_PULSE_GRENADE:
		case AMMO_FRAGMENTATION_GRENADE:
		case AMMO_PROXIMITY_GRENADE:
		case AMMO_SADAR_BLAST:
		case AMMO_PULSE_GRENADE_STRIKE:
		case AMMO_CUDGEL:
		case AMMO_FLECHETTE_POSTMAX:
		case AMMO_FRISBEE:
		case AMMO_FRISBEE_BLAST:
		case AMMO_FRISBEE_FIRE:
		case AMMO_MARINE_PISTOL_PC:
			return(VulnerableToMarineDamage);
			break;

		case AMMO_PRED_WRISTBLADE:
		case AMMO_PRED_PISTOL:
		case AMMO_PRED_RIFLE:
		case AMMO_PRED_ENERGY_BOLT:
		case AMMO_PRED_DISC:
		case AMMO_PRED_STAFF:
		case AMMO_HEAVY_PRED_WRISTBLADE:
		case AMMO_PREDPISTOL_STRIKE:
		case AMMO_PLASMACASTER_NPCKILL:
		case AMMO_PLASMACASTER_PCKILL:
		case AMMO_PRED_TROPHY_KILLSECTION:
			return(VulnerableToPredatorDamage);
			break;
			
		default: ;
	}

	return TRUE;
}


/*-------------------**
** Load/Save Globals **
**-------------------*/
#include "savegame.h"

typedef struct weapons_c_save_block
{
	SAVE_BLOCK_HEADER header;

	int Alien_Visible_Weapon;
	int Alien_Tail_Clock;
	int Wristblade_StrikeType;
	DAMAGE_PROFILE Player_Weapon_Damage;
	int playerNoise;
	int PlayerDamagedOverlayIntensity;
	int HtoHStrikes;
	int WeaponFidgetPlaying;
	int Old_Minigun_SpinSpeed;
	int Minigun_SpinSpeed;
	int Weapon_ThisBurst;
	EULER Minigun_MaxHeadJolt;
	EULER Minigun_HeadJolt;
	int Flamethrower_Timer;
	int StaffAttack;
	int GrenadeLauncherSelectedAmmo;
	int LastHand;  // For alien claws
	VECTORCH PlayersWeaponCameraOffset;
	VECTORCH PlayerGunBarrelOffset;
}WEAPONS_C_SAVE_BLOCK;

void Load_WeaponsCGlobals(SAVE_BLOCK_HEADER* header)
{
	WEAPONS_C_SAVE_BLOCK* block = (WEAPONS_C_SAVE_BLOCK*) header;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	Alien_Visible_Weapon = block->Alien_Visible_Weapon;
	Alien_Tail_Clock = block->Alien_Tail_Clock;
	Wristblade_StrikeType = block->Wristblade_StrikeType;
	Player_Weapon_Damage = block->Player_Weapon_Damage;
	playerNoise = block->playerNoise;
	PlayerDamagedOverlayIntensity = block->PlayerDamagedOverlayIntensity;
	HtoHStrikes = block->HtoHStrikes;
	WeaponFidgetPlaying = block->WeaponFidgetPlaying;
	Old_Minigun_SpinSpeed = block->Old_Minigun_SpinSpeed;
	Minigun_SpinSpeed = block->Minigun_SpinSpeed;
	Weapon_ThisBurst = block->Weapon_ThisBurst;
	Minigun_MaxHeadJolt = block->Minigun_MaxHeadJolt;
	Minigun_HeadJolt = block->Minigun_HeadJolt;
	Flamethrower_Timer = block->Flamethrower_Timer;
	StaffAttack = block->StaffAttack;
	GrenadeLauncherSelectedAmmo = block->GrenadeLauncherSelectedAmmo;
	LastHand = block->LastHand;  // For alien claws
	
	PlayersWeaponCameraOffset = block->PlayersWeaponCameraOffset;
	PlayerGunBarrelOffset = block->PlayerGunBarrelOffset;
	
	Load_SoundState(&weaponHandle);
}

void Save_WeaponsCGlobals()
{
	WEAPONS_C_SAVE_BLOCK* block;

	GET_SAVE_BLOCK_POINTER(block);

	//fill in the header
	block->header.size = sizeof(*block);
	block->header.type = SaveBlock_WeaponsCGlobals;

	block->Alien_Visible_Weapon = Alien_Visible_Weapon;
	block->Alien_Tail_Clock = Alien_Tail_Clock;
	block->Wristblade_StrikeType = Wristblade_StrikeType;
	block->Player_Weapon_Damage = Player_Weapon_Damage;
	block->playerNoise = playerNoise;
	block->PlayerDamagedOverlayIntensity = PlayerDamagedOverlayIntensity;
	block->HtoHStrikes = HtoHStrikes;
	block->WeaponFidgetPlaying = WeaponFidgetPlaying;
	block->Old_Minigun_SpinSpeed = Old_Minigun_SpinSpeed;
	block->Minigun_SpinSpeed = Minigun_SpinSpeed;
	block->Weapon_ThisBurst = Weapon_ThisBurst;
	block->Minigun_MaxHeadJolt = Minigun_MaxHeadJolt;
	block->Minigun_HeadJolt = Minigun_HeadJolt;
	block->Flamethrower_Timer = Flamethrower_Timer;
	block->StaffAttack = StaffAttack;
	block->GrenadeLauncherSelectedAmmo = GrenadeLauncherSelectedAmmo;
	block->LastHand = LastHand;  // For alien claws

	block->PlayersWeaponCameraOffset = PlayersWeaponCameraOffset;
	block->PlayerGunBarrelOffset = PlayerGunBarrelOffset;

	

	Save_SoundState(&weaponHandle);
}

void MarinePistol_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *StockBack;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	StockBack=Get_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	if (StockBack) {
		/* Get rid of it. */
		Remove_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	}

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		
		if (weaponPtr->PrimaryRoundsRemaining>=65536) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>6),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,-1,0);
		} else {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>6),HMSQT_MarineHUD,(int)MHSS_Secondary_Fire,-1,0);
		}
		{
			SECTION_DATA *casing;
			/* Make a little casing. */
			
			casing=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum R Pistol round");
			
			if (casing) {
				MakePistolCasing(&casing->World_Offset,&casing->SecMat);
			}
		}
	}

}

void MarinePistol_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	DELTA_CONTROLLER *StockBack;

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	StockBack=Get_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	if (StockBack) {
		/* Get rid of it. */
		Remove_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	}

	if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		if (weaponPtr->PrimaryRoundsRemaining) {
			InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
		}
	}

	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,(ONE_FIXED<<1),0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	
	}

}

void MarinePistol_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *StockBack;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	StockBack=Get_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	if (StockBack) {
		/* Get rid of it. */
		Remove_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
	}

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Reload,time,0);
		PlayersWeaponHModelController.Looped=0;

		if ((AvP.Network != I_No_Network)&&(PISTOL_INFINITE_AMMO)) {
			/* Pistol infinite ammo hack? */
			weaponPtr->PrimaryMagazinesRemaining++;
		}
	}
	StaffAttack=-1;
}

DISPLAYBLOCK *MakePistolCasing(VECTORCH *position,MATRIXCH *orient) {

	DISPLAYBLOCK *dispPtr;
	STRATEGYBLOCK *sbPtr;
	MODULEMAPBLOCK *mmbptr;
	MODULE m_temp;

	if( (NumActiveBlocks > maxobjects-5) || (NumActiveStBlocks > maxstblocks-5)) return NULL;

	mmbptr = &TempModuleMap;

	CreateShapeInstance(mmbptr,"Pistol case");

	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
	m_temp.m_mapptr = mmbptr;
	m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
	m_temp.m_dptr = NULL;
	AllocateModuleObject(&m_temp);    
	dispPtr = m_temp.m_dptr;
	if(dispPtr==NULL) return (DISPLAYBLOCK *)0; /* patrick: cannot create displayblock, so just return 0 */

	dispPtr->ObMyModule = NULL;     /* Module that created us */
	dispPtr->ObWorld = *position;

	sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
  
	if (sbPtr == 0) return (DISPLAYBLOCK *)0; // Failed to allocate a strategy block

	sbPtr->I_SBtype = I_BehaviourFragment;

	{
		DYNAMICSBLOCK *dynPtr;
		VECTORCH impulse;

		sbPtr->SBdataptr = (ONE_SHOT_BEHAV_BLOCK *) AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK ));
		if (sbPtr->SBdataptr == 0) 
		{	
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(sbPtr);
			return(DISPLAYBLOCK*)NULL;
		}


		((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ALIEN_DYINGTIME;

		dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_DEBRIS);

		if (dynPtr == 0)
		{
			// Failed to allocate a dynamics block
			RemoveBehaviourStrategy(sbPtr);
			return(DISPLAYBLOCK*)NULL;
		}

		dynPtr->Position = *position;
		dynPtr->OrientMat= *orient;

		dynPtr->AngVelocity.EulerX = 0;
		dynPtr->AngVelocity.EulerY = ((FastRandom()&4095)<<2);
		dynPtr->AngVelocity.EulerZ = 0;

		impulse.vx=0;
		impulse.vy=0;
		impulse.vz=6000;
	
		RotateVector(&impulse,orient);

		dynPtr->LinImpulse = impulse;

		sbPtr->SBflags.not_on_motiontracker=1;
	}
    
    return dispPtr;

}

void MarineTwoPistols_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		
		if (LastHand==0) {
			if (weaponPtr->PrimaryRoundsRemaining>=65536) {
				Start_Delta_Sequence(FireRight,HMSQT_MarineHUD,MHSS_Standard_Fire,-1);
				FireRight->Playing=1;
				FireRight->Active=1;
			} else {
				Start_Delta_Sequence(FireRight,HMSQT_MarineHUD,MHSS_Right_Out,-1);
				FireRight->Playing=1;
				FireRight->Active=1;
			}
			{
				SECTION_DATA *casing;
				/* Make a little casing. */
				
				casing=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum R Pistol round");
				
				if (casing) {
					MakePistolCasing(&casing->World_Offset,&casing->SecMat);
				}
			}
		} else {
			if (weaponPtr->SecondaryRoundsRemaining>=65536) {
				Start_Delta_Sequence(FireLeft,HMSQT_MarineHUD,MHSS_Secondary_Fire,-1);
				FireLeft->Playing=1;
				FireLeft->Active=1;
			} else {
				Start_Delta_Sequence(FireLeft,HMSQT_MarineHUD,MHSS_Left_Out,-1);
				FireLeft->Playing=1;
				FireLeft->Active=1;
			}
			{
				SECTION_DATA *casing;
				/* Make a little casing. */
				
				casing=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum L Pistol round");
				
				if (casing) {
					MakePistolCasing(&casing->World_Offset,&casing->SecMat);
				}
			}
		}
		if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>1),HMSQT_MarineHUD,(int)MHSS_Stationary,-1,0);
		}
	}
}

void MarineTwoPistols_SecondaryFiring(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;

	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		
		if (LastHand==0) {
			if (weaponPtr->PrimaryRoundsRemaining>=65536) {
				Start_Delta_Sequence(FireRight,HMSQT_MarineHUD,MHSS_Standard_Fire,-1);
				FireRight->Playing=1;
				FireRight->Active=1;
			} else {
				Start_Delta_Sequence(FireRight,HMSQT_MarineHUD,MHSS_Right_Out,-1);
				FireRight->Playing=1;
				FireRight->Active=1;
			}
			{
				SECTION_DATA *casing;
				/* Make a little casing. */
				
				casing=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum R Pistol round");
				
				if (casing) {
					MakePistolCasing(&casing->World_Offset,&casing->SecMat);
				}
			}
		} else {
			if (weaponPtr->SecondaryRoundsRemaining>=65536) {
				Start_Delta_Sequence(FireLeft,HMSQT_MarineHUD,MHSS_Secondary_Fire,-1);
				FireLeft->Playing=1;
				FireLeft->Active=1;
			} else {
				Start_Delta_Sequence(FireLeft,HMSQT_MarineHUD,MHSS_Left_Out,-1);
				FireLeft->Playing=1;
				FireLeft->Active=1;
			}
			{
				SECTION_DATA *casing;
				/* Make a little casing. */
				
				casing=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum L Pistol round");
				
				if (casing) {
					MakePistolCasing(&casing->World_Offset,&casing->SecMat);
				}
			}
		}

		/* Start Crossover sequence too. */

		if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Tertiary_Fire) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Tertiary_Fire,-1,0);
		}
	}
}

int AreTwoPistolsInTertiaryFire(void) {

	PLAYER_WEAPON_DATA *weaponPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

	if (PlayersWeaponHModelController.Sub_Sequence!=MHSS_Tertiary_Fire) {
		if ((weaponPtr->WeaponIDNumber==WEAPON_MARINE_PISTOL) 
			&&((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY))
			) {
			/* This to trap single player's reflection. */
			return(1);
		} else {
			return(0);
		}
	} else {
		return(1);
	}

}

int FireMarineTwoPistols(PLAYER_WEAPON_DATA *weaponPtr, int secondary)
{
	TEMPLATE_WEAPON_DATA *twPtr=&TemplateWeapon[weaponPtr->WeaponIDNumber];
   	TEMPLATE_AMMO_DATA *templateAmmoPtr = &TemplateAmmo[twPtr->PrimaryAmmoID];

	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	/* Deduce which pistol can fire, if either? */

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}

	if (LastHand==0) {
		/* Look to the left. */
		if (!DeltaAnimation_IsFinished(FireLeft)) {
			/* Left should not be able to fire. */
			return(0);
		}
		if (weaponPtr->SecondaryRoundsRemaining) {
			/* Fire left. */
			LastHand=1;
			PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum Flash L");
		  	weaponPtr->SecondaryRoundsRemaining -= 65536;
		} else {
			/* Try the other hand... */
			if (!DeltaAnimation_IsFinished(FireRight)) {
				/* Right should not be able to fire. */
				return(0);
			}
			if (weaponPtr->PrimaryRoundsRemaining) {
				/* Fire right. */
				LastHand=0;
				PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum Flash");
			  	weaponPtr->PrimaryRoundsRemaining -= 65536;
			}
		}
	} else {
		if (!DeltaAnimation_IsFinished(FireRight)) {
			/* Right should not be able to fire. */
			return(0);
		}
		if (weaponPtr->PrimaryRoundsRemaining) {
			/* Fire right. */
			LastHand=0;
			PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum Flash");
		  	weaponPtr->PrimaryRoundsRemaining -= 65536;
		} else {
			/* Try the other hand... */
			if (!DeltaAnimation_IsFinished(FireLeft)) {
				/* Left should not be able to fire. */
				return(0);
			}
			if (weaponPtr->SecondaryRoundsRemaining) {
				/* Fire left. */
				LastHand=1;
				PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum Flash L");
			  	weaponPtr->SecondaryRoundsRemaining -= 65536;
			}
		}
	}

	{
		/* Force gun judder */
		if (secondary) {
			weaponPtr->CurrentState = WEAPONSTATE_FIRING_SECONDARY;
			weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
		} else {
			weaponPtr->CurrentState = WEAPONSTATE_FIRING_PRIMARY;
			weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;
		}
		CalculatePlayersTarget(twPtr,weaponPtr);
	}

    /* does ammo create an actual object? */
    if (templateAmmoPtr->CreatesProjectile)
    {
		FireProjectileAmmo(twPtr->PrimaryAmmoID);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
    }
    else /* instantaneous line of sight */
    {
		PlayerFireLineOfSightAmmo(twPtr->PrimaryAmmoID,1);
		CurrentGameStats_WeaponFired(PlayerStatusPtr->SelectedWeaponSlot,1);
	}	
	return(1);	
}	

int FireMarineTwoPistolsPrimary(PLAYER_WEAPON_DATA *weaponPtr) {
	
	return(FireMarineTwoPistols(weaponPtr,0));

}

int FireMarineTwoPistolsSecondary(PLAYER_WEAPON_DATA *weaponPtr) {
	
	return(FireMarineTwoPistols(weaponPtr,1));

}

void MarineTwoPistols_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}

	if (!DeltaAnimation_IsFinished(FireRight)) {
		/* Just leave it alone... */
		return;
	}

	/* If we're in Tertiary Fire, leave it alone for a moment. */
	if (PlayersWeaponHModelController.Sub_Sequence==MHSS_Tertiary_Fire) {
		if (PlayersWeaponHModelController.Tweening!=Controller_NoTweening) {
			return;
		}
		if (weaponPtr->StateTimeOutCounter < (ONE_FIXED>>2)) {
			return;
		}
	}

	if ((PlayersWeaponHModelController.Sub_Sequence!=MHSS_Stationary)&&(PlayersWeaponHModelController.Sub_Sequence!=MHSS_Fidget)) {
		if (PlayersWeaponHModelController.Sub_Sequence==MHSS_Tertiary_Fire) {
			InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>1),HMSQT_MarineHUD,(int)MHSS_Stationary,-1,0);
		} else {
			InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED);
		}
	}

	if (weaponPtr->StateTimeOutCounter > ONE_FIXED) {
	
		if (WeaponFidgetPlaying) {
			if (PlayersWeaponHModelController.sequence_timer==(ONE_FIXED-1)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
				WeaponFidgetPlaying=0;
			}
		} else if ((FastRandom()&255)==0) {
			/* Start animation. */
			if (HModelSequence_Exists(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Fidget)) {
				InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>2),HMSQT_MarineHUD,(int)MHSS_Fidget,(ONE_FIXED<<1),0);
				weaponPtr->StateTimeOutCounter=0;
				WeaponFidgetPlaying=1;
			}
		}
	
	}

}

void MarineTwoPistols_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	GLOBALASSERT(PlayersWeaponHModelController.Sequence_Type==HMSQT_MarineHUD);

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RELOAD_PRIMARY]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Standard_Reload,time,0);
		PlayersWeaponHModelController.Looped=0;
		FireRight->Active=0;
		FireLeft->Active=0;


		/* Pistol infinite ammo hack? */
		if ((AvP.Network != I_No_Network)&&(PISTOL_INFINITE_AMMO)) {
			weaponPtr->PrimaryMagazinesRemaining++;
			weaponPtr->SecondaryMagazinesRemaining++;
		}

	}
	StaffAttack=-1;
}

static void MarineZeroAmmoFunctionality(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr) {

	int weaponNum;
	int slot;

	/* Let's cut this into a function. */

	if (AvP.PlayerType!=I_Marine) {
		/* For now. */
		return;
	}

	if ((weaponPtr->WeaponIDNumber==WEAPON_GRENADELAUNCHER)
		&& ( 
				(GrenadeLauncherData.ProximityRoundsRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationRoundsRemaining!=0)
			 	||(GrenadeLauncherData.StandardRoundsRemaining!=0)
				||(GrenadeLauncherData.ProximityMagazinesRemaining!=0)
			  	||(GrenadeLauncherData.FragmentationMagazinesRemaining!=0)
			 	||(GrenadeLauncherData.StandardMagazinesRemaining!=0)
			)
		) {
		/* Deal with Al's 'grenade launcher change ammo' case... */
		GrenadeLauncher_EmergencyChangeAmmo(weaponPtr);
		return;
	}

	/* Now the serious bit. */

	weaponNum=0;

	/* Now, step up until you get to NULL or find a weapon with ammo. */
	//weaponNum--;
	while (MarineWeaponHierarchy[weaponNum]!=NULL_WEAPON) {

		slot=SlotForThisWeapon(MarineWeaponHierarchy[weaponNum]);
		if (slot!=-1) {
			if (playerStatusPtr->WeaponSlot[slot].Possessed==1) {
				if (WeaponHasAmmo(slot)) {
					/* Okay! */
					break;
				}
			}
		}
		weaponNum++;
	}

	if (MarineWeaponHierarchy[weaponNum]==NULL_WEAPON) {
		/* No possible action. */
		return;
	}
	if (MarineWeaponHierarchy[weaponNum]==weaponPtr->WeaponIDNumber) {
		/* If you found the current weapon, do nothing. */
		return;
	}
	
	/* So, change to this weapon. */
	playerStatusPtr->SwapToWeaponSlot = slot;
	weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
	weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;

}

void MarinePistol_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *StockBack;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_OUT]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;

		if (weaponPtr->PrimaryRoundsRemaining==0) {
			StockBack=Get_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
			if (StockBack==NULL) {
				StockBack=Add_Delta_Sequence(&PlayersWeaponHModelController,"StockBack",HMSQT_MarineHUD,MHSS_Right_Out,ONE_FIXED);
			}
			if (StockBack) {
				StockBack->Playing=0;
				StockBack->timer=65535;
				StockBack->lastframe_timer=65535;
			}
		}

		/* Alter Two Pistols ammo? */
		{
			int slot;

			slot=SlotForThisWeapon(WEAPON_TWO_PISTOLS);
			if (slot!=-1) {
       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryRoundsRemaining=weaponPtr->PrimaryRoundsRemaining;
				/* Now split the clips between the pistols? */
				
       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining=(weaponPtr->PrimaryMagazinesRemaining)>>1;
       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].SecondaryMagazinesRemaining=(weaponPtr->PrimaryMagazinesRemaining)>>1;
				if (weaponPtr->PrimaryMagazinesRemaining&1) {
	       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining++;
				}

			}
		}
	}
}

void MarineTwoPistols_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_OUT]);
		time-=(ONE_FIXED>>4);

		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>4),HMSQT_MarineHUD,(int)MHSS_Go,time,0);
		PlayersWeaponHModelController.Looped=0;
		/* Alter Pistol ammo? */
		{
			int slot;

			slot=SlotForThisWeapon(WEAPON_MARINE_PISTOL);
			if (slot!=-1) {
				if (weaponPtr->PrimaryRoundsRemaining==0) {
	       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining=weaponPtr->PrimaryMagazinesRemaining+weaponPtr->SecondaryMagazinesRemaining;
	       			if (((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining>99) {
						((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining=99;
	       			};
	       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryRoundsRemaining=weaponPtr->SecondaryRoundsRemaining;
					weaponPtr->SecondaryRoundsRemaining=0;
	       			weaponPtr->PrimaryRoundsRemaining=((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryRoundsRemaining;
				} else {
	       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryMagazinesRemaining=weaponPtr->PrimaryMagazinesRemaining+weaponPtr->SecondaryMagazinesRemaining;
	       			((PLAYER_STATUS *)playerStatus)->WeaponSlot[slot].PrimaryRoundsRemaining=weaponPtr->PrimaryRoundsRemaining;
				}
			}
		}
	}
}

void MarinePistol_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *StockBack;
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	Weapon_ThisBurst=-1;

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_IN]);

		GLOBALASSERT(time!=0);

		if (weaponPtr->PrimaryRoundsRemaining==0) {
			StockBack=Get_Delta_Sequence(&PlayersWeaponHModelController,"StockBack");
			if (StockBack==NULL) {
				StockBack=Add_Delta_Sequence(&PlayersWeaponHModelController,"StockBack",HMSQT_MarineHUD,MHSS_Right_Out,ONE_FIXED);
			}
			if (StockBack) {
				StockBack->Playing=0;
				StockBack->timer=65535;
				StockBack->lastframe_timer=65535;
				StockBack->Active=1;
			}
		}

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,time);
		PlayersWeaponHModelController.Looped=0;
	}

	Flamethrower_Timer=0;
	StaffAttack=-1;
	LastHand=1;
}

void MarineTwoPistols_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	TEMPLATE_WEAPON_DATA *twPtr;
	DELTA_CONTROLLER *FireRight;
	DELTA_CONTROLLER *FireLeft;

	FireRight=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireRight");
	if (FireRight==NULL) {
		FireRight=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireRight",HMSQT_MarineHUD,MHSS_Standard_Fire,ONE_FIXED);
		FireRight->Playing=0;
		FireRight->Active=0;
	}

	FireLeft=Get_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft");
	if (FireLeft==NULL) {
		FireLeft=Add_Delta_Sequence(&PlayersWeaponHModelController,"FireLeft",HMSQT_MarineHUD,MHSS_Secondary_Fire,ONE_FIXED);
		FireLeft->Playing=0;
		FireLeft->Active=0;
	}
    
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	Weapon_ThisBurst=-1;

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		int time;

		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_SWAPPING_IN]);

		GLOBALASSERT(time!=0);

		InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Come,time);
		PlayersWeaponHModelController.Looped=0;

		if (weaponPtr->PrimaryRoundsRemaining==0) {
			Start_Delta_Sequence(FireRight,HMSQT_MarineHUD,MHSS_Right_Out,-1);
			FireRight->Playing=0;
			FireRight->timer=65535;
			FireRight->lastframe_timer=65535;
			FireRight->Active=1;
		}

		if (weaponPtr->SecondaryRoundsRemaining==0) {
			Start_Delta_Sequence(FireLeft,HMSQT_MarineHUD,MHSS_Left_Out,-1);
			FireLeft->Playing=0;
			FireLeft->timer=65535;
			FireLeft->lastframe_timer=65535;
			FireLeft->Active=1;
		}

	}

	Flamethrower_Timer=0;
	StaffAttack=-1;
	LastHand=1;
}

void Frisbee_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {

		Sound_Stop(weaponHandle);
		Sound_Play(SID_ED_SKEETERLAUNCH,"h");

		//InitHModelSequence(&PlayersWeaponHModelController,HMSQT_MarineHUD,(int)MHSS_Standard_Fire,(ONE_FIXED/6));
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>6),HMSQT_MarineHUD,(int)MHSS_Standard_Fire,-1,0);
		PlayersWeaponHModelController.Looped=0;
		FireNonAutomaticWeapon(weaponPtr);
	}

}

void Frisbee_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr) {

	/* Clumsy, but here we go. */

	AddLightingEffectToObject(Player,LFX_MUZZLEFLASH);
	if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
		//InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Stationary,ONE_FIXED,1);
		InitHModelTweening(&PlayersWeaponHModelController,(ONE_FIXED>>3),HMSQT_MarineHUD,(int)MHSS_Secondary_Fire,-1,0);
	}

}

static void PredatorZeroAmmoFunctionality(PLAYER_STATUS *playerStatusPtr,PLAYER_WEAPON_DATA *weaponPtr) {

	int weaponNum;
	int slot;

	/* Let's cut this into a function. */

	if (AvP.PlayerType!=I_Predator) {
		/* For now. */
		return;
	}

	/* Now the serious bit. */

	weaponNum=0;

	/* Now, step up until you get to NULL or find a weapon with ammo. */
	//weaponNum--;
	while (PredatorWeaponHierarchy[weaponNum]!=NULL_WEAPON) {

		slot=SlotForThisWeapon(PredatorWeaponHierarchy[weaponNum]);
		if (slot!=-1) {
			if (playerStatusPtr->WeaponSlot[slot].Possessed==1) {
				if (Predator_WeaponHasAmmo(playerStatusPtr,slot)) {
					/* Okay! */
					break;
				}
			}
		}
		weaponNum++;
	}

	if (PredatorWeaponHierarchy[weaponNum]==NULL_WEAPON) {
		/* No possible action. */
		return;
	}
	if (PredatorWeaponHierarchy[weaponNum]==weaponPtr->WeaponIDNumber) {
		/* If you found the current weapon, do nothing. */
		return;
	}
	
	/* So, change to this weapon. */
	playerStatusPtr->SwapToWeaponSlot = slot;
	weaponPtr->CurrentState = WEAPONSTATE_UNREADYING;
	weaponPtr->StateTimeOutCounter = WEAPONSTATE_INITIALTIMEOUTCOUNT;

}
