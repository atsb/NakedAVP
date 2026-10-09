/*****************************************************************************************************//*KJL*****************************************************************************
* Equipmnt.c - contains the data for all equipment that can be used in the game. *
*                                                                                *
*****************************************************************************KJL*/
#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "equipmnt.h"
#include "hmodel.h"
#include "sequnces.h"
#include "weapons.h"

#define USE_ENCUMBERANCE 0

/* Weapon Functions */

extern void AlienClawTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClawEndTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MeleeWeaponNullTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienTailTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredDiscThrowTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredWristbladeTrajectory(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void ParticleBeamSwapping(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PulseRifleSwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PulseRifleSwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PulseRifleGrenadeRecoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PulseRifleReloadClip(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PulseRifleFidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void ParticleBeamReadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void ParticleBeamUnreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MinigunStartSpin(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MinigunStopSpin(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Maintain_Minigun(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherRecoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherIdle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherFidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherNull(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherReload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncherReload_Change(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncher_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GrenadeLauncher_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_Strike_Secondary(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_Readying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_Unreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredPistol_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredPistol_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredPistol_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredPistol_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClaw_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClaw_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClaw_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClaw_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienClaw_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienTail_Poise(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienTail_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienGrab_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienGrab_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienGrab_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void AlienGrab_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PlasmaCaster_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristConsole_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristConsole_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void TemplateHands_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void TemplateHands_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristConsole_Readying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristConsole_Unreadying(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SADAR_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericMarineWeapon_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericMarineWeapon_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericMarineWeapon_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericMarineWeapon_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericMarineWeapon_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_Firing_Secondary(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void GenericPredatorWeapon_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredatorDisc_Throwing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredatorDisc_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PredatorDisc_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void StaffAttack_Basic(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Staff_Idle(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Staff_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Staff_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void PlasmaCaster_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_WindUp(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void WristBlade_WindUpStrike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SpikeyThing_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Extinguisher_Use(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Secondary_PlasmaCaster_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Minigun_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void SpearGun_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarinePistol_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarinePistol_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarinePistol_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarinePistol_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarinePistol_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Cudgel_Strike(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_SecondaryFiring(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_Fidget(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_Reload(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_SwapOut(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void MarineTwoPistols_SwapIn(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Frisbee_Recoil(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);
extern void Frisbee_Firing(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

extern void GrenadeLauncherInit(PLAYER_WEAPON_DATA *weaponPtr);
extern int GrenadeLauncherFire(PLAYER_WEAPON_DATA *weaponPtr);

extern void WeaponSetStartFrame(void *playerStatus, PLAYER_WEAPON_DATA *weaponPtr);

extern int FireBurstWeapon(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireMinigun(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireNonAutomaticWeapon(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireNonAutomaticSecondaryAmmo(PLAYER_WEAPON_DATA *weaponPtr);
extern int GrenadeLauncherChangeAmmo(PLAYER_WEAPON_DATA *weaponPtr);
extern int PredDiscChangeMode(PLAYER_WEAPON_DATA *weaponPtr);
extern int PredatorDisc_Prefiring(PLAYER_WEAPON_DATA *weaponPtr);
extern int SmartgunSecondaryFire(PLAYER_WEAPON_DATA *weaponPtr);
extern int DamageObjectInLineOfSight(PLAYER_WEAPON_DATA *weaponPtr);
extern int MeleeWeapon_180Degree_Front(PLAYER_WEAPON_DATA *weaponPtr);
extern int MeleeWeapon_90Degree_Front(PLAYER_WEAPON_DATA *weaponPtr);
extern int PlayerFireFlameThrower(PLAYER_WEAPON_DATA *weaponPtr);
extern int FirePCPlasmaCaster(PLAYER_WEAPON_DATA *weaponPtr);
extern int SecondaryFirePCPlasmaCaster(PLAYER_WEAPON_DATA *weaponPtr);
extern int FirePredPistol(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireSpeargun(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireSpikeyThing(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireExtinguisher(PLAYER_WEAPON_DATA *weaponPtr);
extern int PlayerFirePredPistolFlechettes(PLAYER_WEAPON_DATA *weaponPtr);
extern int PredPistolSecondaryFire(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireMarineTwoPistolsPrimary(PLAYER_WEAPON_DATA *weaponPtr);
extern int FireMarineTwoPistolsSecondary(PLAYER_WEAPON_DATA *weaponPtr);

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
/* CDF 2/10/97 Key for weapons vs. slots... */
enum WEAPON_ID MarineWeaponKey[MAX_NO_OF_WEAPON_SLOTS] = {
	WEAPON_PULSERIFLE,
	/* AUTOSHOTGUN removed, 4/3/98, CDF, By order of Al */
	WEAPON_SMARTGUN,
    WEAPON_FLAMETHROWER,
    WEAPON_SADAR,
    WEAPON_GRENADELAUNCHER,
    WEAPON_MINIGUN,	 
	WEAPON_FRISBEE_LAUNCHER,
	WEAPON_MARINE_PISTOL,
	WEAPON_TWO_PISTOLS,
	NULL_WEAPON,
	#if 1
	WEAPON_CUDGEL
	#else
	NULL_WEAPON
	#endif
};

enum WEAPON_ID PredatorWeaponKey[MAX_NO_OF_WEAPON_SLOTS] = {
    WEAPON_PRED_WRISTBLADE,
	WEAPON_PRED_RIFLE,
	WEAPON_PRED_SHOULDERCANNON,
	WEAPON_PRED_MEDICOMP,
	WEAPON_PRED_PISTOL,
	WEAPON_PRED_DISC,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON
};

enum WEAPON_ID AlienWeaponKey[MAX_NO_OF_WEAPON_SLOTS] = {
    WEAPON_ALIEN_CLAW,
	WEAPON_ALIEN_GRAB,
    WEAPON_ALIEN_SPIT,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON,
	NULL_WEAPON
};

/* KJL 10:45:56 09/20/96 - contains all the generic weapon info */
TEMPLATE_WEAPON_DATA	TemplateWeapon[MAX_NO_OF_WEAPON_TEMPLATES] =
{

	/*KJL**************
	* 	PULSE RIFLE   *
	**************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_10MM_CULW,
		/* SecondaryAmmoID; */
		AMMO_PULSE_GRENADE,

		FireBurstWeapon, /* FirePrimaryFunction */
		FireNonAutomaticSecondaryAmmo, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */
		   
	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,							/* WEAPONSTATE_IDLE	*/
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536,						/* WEAPONSTATE_RELOAD_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_SECONDARY	*/
			65536,					/* WEAPONSTATE_RECOIL_SECONDARY	*/
			65536,						/* WEAPONSTATE_RELOAD_SECONDARY	*/
			65536*8,  					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*8,  					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			PulseRifleFidget,  /* WEAPONSTATE_IDLE	*/
			WeaponSetStartFrame,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			PulseRifleReloadClip,  /* WEAPONSTATE_RELOAD_PRIMARY */
			WeaponSetStartFrame,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			PulseRifleGrenadeRecoil,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			PulseRifleSwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			PulseRifleSwapOut,  /* WEAPONSTATE_SWAPPING_OUT */
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1000*65536/60,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius */
	    0,
		/* RestPosition; */
		//{300,400,800},
		{0,0,0},
		
		/* RecoilMaxZ; */
		90, //0, //60,
		/* RecoilMaxRandomZ; */
		60, //0, //31,
		/* RecoilMaxXTilt; */
		30, //0, //31,
		/* RecoilMaxYTilt; */
		30, //0, //15,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_INGAME_PULSERIFLE,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Pulse Rifle",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		1,
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	
	/*KJL*********************
	* 	WEAPON_AUTOSHOTGUN   *
	*********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_SHOTGUN,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireNonAutomaticWeapon, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	65536*8,					/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*4,					/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*8,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*8,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    5500,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		80,
		/* RecoilMaxRandomZ; */
		31,
		/* RecoilMaxXTilt; */
		-31,
		/* RecoilMaxYTilt; */
		15,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL******************
	* 	WEAPON_SMARTGUN   *
	******************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_SMARTGUN,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireBurstWeapon, /* FirePrimaryFunction */
		SmartgunSecondaryFire, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536,						/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*4,					/* WEAPONSTATE_SWAPPING_IN	 was 2/3? */
			65536*4,					/* WEAPONSTATE_SWAPPING_OUT	 was 2/3? */
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_IDLE	*/
			GenericMarineWeapon_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			GenericMarineWeapon_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericMarineWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericMarineWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_JAMMED */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_WAITING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_READYING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    50*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    55000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		60, //60,
		/* RecoilMaxRandomZ; */
		31, //31,
		/* RecoilMaxXTilt; */
		15, //31,
		/* RecoilMaxYTilt; */
		15, //15,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_SMARTGUN,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Smart Gun",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			3*ONE_FIXED/4, /* MovementMultiple	*/
			3*ONE_FIXED/4, /* TurningMultiple */
			3*ONE_FIXED/4, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED/2, /* MovementMultiple	*/
			ONE_FIXED/2, /* TurningMultiple */
			ONE_FIXED/2, /* JumpingMultiple */
			1, /* CanCrouch */
			0, /* CanRun */
		},
		{ /* Encum_FireSec */
			3*ONE_FIXED/4, /* MovementMultiple	*/
			3*ONE_FIXED/4, /* TurningMultiple */
			3*ONE_FIXED/4, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		1,
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,	
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*KJL**********************
	*   WEAPON_FLAMETHROWER   *
	**********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_FLAMETHROWER,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		PlayerFireFlameThrower, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536,						/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_IDLE	*/
			GenericMarineWeapon_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			GenericMarineWeapon_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericMarineWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericMarineWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_JAMMED */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_WAITING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_READYING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    15*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{-350,0,0},
		
		/* RecoilMaxZ; */
		0, //60,
		/* RecoilMaxRandomZ; */
		0, //31,
		/* RecoilMaxXTilt; */
		0, //31,
		/* RecoilMaxYTilt; */
		0, //15,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_FLAMETHROWER,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    NULL,/* ie. no muzzle flash */
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Flamethrower",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		1,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		1,
		/* FireSecondaryLate */
		1,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL*******************
	*   WEAPON_PLASMAGUN   *
	*******************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PLASMA,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536,						/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    5500,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0,
		/* RecoilMaxRandomZ; */
		0,
		/* RecoilMaxXTilt; */
		0,
		/* RecoilMaxYTilt; */
		0,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJ****************
	*   WEAPON_SADAR   *
	****************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_SADAR_TOW,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireNonAutomaticWeapon, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	65536*6, 					/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2/3,					/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			SADAR_Fidget,  /* WEAPONSTATE_IDLE	*/
			SADAR_Idle,  /* WEAPONSTATE_FIRING_PRIMARY */
			SADAR_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			SADAR_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			SADAR_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			SADAR_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			SADAR_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			SADAR_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			SADAR_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			SADAR_Idle,  /* WEAPONSTATE_JAMMED */
			SADAR_Idle,  /* WEAPONSTATE_WAITING */
			SADAR_Idle,  /* WEAPONSTATE_READYING */
			SADAR_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //200,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //24,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_SADAR,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Rocket Launcher",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			0, /* MovementMultiple	*/
			0, /* TurningMultiple */
			0, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif
		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*KJL*************************
	*   WEAPON_GRENADELAUNCHER   *
	*************************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_GRENADE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		GrenadeLauncherFire, /* FirePrimaryFunction */
		GrenadeLauncherChangeAmmo, /* FireSecondaryFunction */
		GrenadeLauncherInit,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	(65536*3)/4, // Was *6...		/* WEAPONSTATE_RECOIL_PRIMARY */
			(65536*3)/4,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			(65536*3)/4,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*4,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*4,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			GrenadeLauncherFidget,  /* WEAPONSTATE_IDLE	*/
			GrenadeLauncherNull,  /* WEAPONSTATE_FIRING_PRIMARY */
			GrenadeLauncherRecoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			GrenadeLauncherReload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			GrenadeLauncherReload_Change,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GrenadeLauncherIdle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GrenadeLauncher_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GrenadeLauncher_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GrenadeLauncherIdle,  /* WEAPONSTATE_JAMMED */
			GrenadeLauncherIdle,  /* WEAPONSTATE_WAITING */
			GrenadeLauncherIdle,  /* WEAPONSTATE_READYING */
			GrenadeLauncherIdle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //60,
		/* RecoilMaxRandomZ; */
		0, //31,
		/* RecoilMaxXTilt; */
		0, //31,
		/* RecoilMaxYTilt; */
		0, //15,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_GRENADELAUNCHER,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Grenade Launcher",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		1,
	},
	/*KJL*****************
	*   WEAPON_MINIGUN   *
	*****************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_MINIGUN,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireMinigun, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*4,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*4,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			65536, /* WEAPONSTATE_UNREADYING */

		},
		{
			Maintain_Minigun,  /* WEAPONSTATE_IDLE	*/
			Maintain_Minigun,  /* WEAPONSTATE_FIRING_PRIMARY */
			Maintain_Minigun,  /* WEAPONSTATE_RECOIL_PRIMARY */
			Maintain_Minigun,  /* WEAPONSTATE_RELOAD_PRIMARY */
			Maintain_Minigun,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			Maintain_Minigun,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			Maintain_Minigun,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			Minigun_SwapIn,		/* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			Maintain_Minigun,  /* WEAPONSTATE_JAMMED */
			Maintain_Minigun,  /* WEAPONSTATE_WAITING */
			MinigunStartSpin,  /* WEAPONSTATE_READYING */
			MinigunStopSpin,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    //60*65536,
	    100*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		60, //60, //0,
		/* RecoilMaxRandomZ; */
		31, //31, //0,
		/* RecoilMaxXTilt; */
		31, //31, //31, //0,
		/* RecoilMaxYTilt; */
		31, //15, //15, //0,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_MINIGUN,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Minigun",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,
		
		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			0, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			0, /* JumpingMultiple */
			1, /* CanCrouch */
			0, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			0, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			#if FORCE_MINIGUN_STOP
			0, 			   /* MovementMultiple	*/
			#else
			2*ONE_FIXED/3, /* MovementMultiple	*/
			#endif
			7*ONE_FIXED/8, /* TurningMultiple */
			#if FORCE_MINIGUN_STOP
			0, 			   /* JumpingMultiple */
			#else
			2*ONE_FIXED/3, /* JumpingMultiple */
			#endif
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		1,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*KJL*********************
	*   WEAPON_SONICCANNON   *
	*********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_SONIC_PULSE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	65536*8,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    5500,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //60,
		/* RecoilMaxRandomZ; */
		0, //31,
		/* RecoilMaxXTilt; */
		0, //31,
		/* RecoilMaxYTilt; */
		0, //15,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
	    "Hsonicg",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,
		
		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL********************
	*   WEAPON_BEAMCANNON   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PARTICLE_BEAM,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireAutomaticWeapon, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			65536, /* WEAPONSTATE_READYING */
			65536, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			ParticleBeamSwapping,  /* WEAPONSTATE_SWAPPING_IN	*/
			ParticleBeamSwapping,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			ParticleBeamReadying,  /* WEAPONSTATE_READYING */
			ParticleBeamUnreadying,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1000*65536/60,
//	    40*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    5500,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		60,
		/* RecoilMaxRandomZ; */
		31,
		/* RecoilMaxXTilt; */
		5,
		/* RecoilMaxYTilt; */
		5,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
	    //"Cpbhud",
	    "Shell",/*"CPBhudf",*/
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,
		
		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			2*ONE_FIXED/3, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		1,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,/*1,*/
		/* HasTextureAnimation */
		0,/*1,*/
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL********************
	*   WEAPON_MYSTERYGUN   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_SMARTGUN,
		/* SecondaryAmmoID; */
		AMMO_PULSE_GRENADE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			65536,						/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_SECONDARY	*/
			65536*6,					/* WEAPONSTATE_RECOIL_SECONDARY	*/
			65536,						/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*2,   					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*2,   					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	   	2000*65536/60,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius */
	    55000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		60,
		/* RecoilMaxRandomZ; */
		31,
		/* RecoilMaxXTilt; */
		31,
		/* RecoilMaxYTilt; */
		15,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
	    "Hmystry",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,
		
		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif

		/* UseStateMovement :1; */
		1,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		1,
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	

	/*KJL**********************************
	*   PPP	RRR	EEE	DD	 A	TTT	OOO	RRR   *
	*   P P	R R	E	D D	A A	 T	O O	R R   *
	*   PPP	RRR	EEE	D D	AAA	 T	O O	RRR   *
	*   P	RR	E	D D	A A	 T	O O	RR	  *
	*   P	R R	EEE	DD	A A	 T	OOO	R R   *
	**********************************KJL*/

	/*KJL*************************
	*   WEAPON_PRED_WRISTBLADE   *
	*************************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_WRISTBLADE,
		/* SecondaryAmmoID; */
		AMMO_HEAVY_PRED_WRISTBLADE,
		   
		//MeleeWeapon_90Degree_Front, /* FirePrimaryFunction */
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			65536>>2, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			65536>>2,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*3,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536*3,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			65536*2, 						/* WEAPONSTATE_READYING */
			65536*2, 						/* WEAPONSTATE_UNREADYING */

		},
		{
			WristBlade_Idle,  /* WEAPONSTATE_IDLE	*/
			WristBlade_Strike,  /* WEAPONSTATE_FIRING_PRIMARY */
			WristBlade_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			WristBlade_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			WristBlade_WindUp,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			WristBlade_WindUpStrike,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			WristBlade_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			TemplateHands_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			TemplateHands_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			WristBlade_Idle,  /* WEAPONSTATE_JAMMED */
			WristBlade_Idle,  /* WEAPONSTATE_WAITING */
			WristBlade_Readying,  /* WEAPONSTATE_READYING */
			WristBlade_Unreadying,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,350,0},

		/* Name; */
		TEXTSTRING_INGAME_WRISTBLADE,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    0,
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"Template",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Come,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		1,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		1,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},

	/*KJL*********************
	*   WEAPON_PRED_PISTOL   *
	*********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_PISTOL,
		/* SecondaryAmmoID; */
		AMMO_PRED_PISTOL,
		   
		//FirePredPistol, /* FirePrimaryFunction */
		PredPistolSecondaryFire, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		//PlayerFirePredPistolFlechettes, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_PRIMARY */
			(65536*2), 					/* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */
										
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY */
			65536,						/* WEAPONSTATE_RELOAD_SECONDARY */

			((65536*5)/6),				/* WEAPONSTATE_SWAPPING_IN	*/ /* Was >>2 */
			((65536*5)/6), 				/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			PredPistol_Idle,  /* WEAPONSTATE_IDLE	*/
			PredPistol_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			PredPistol_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			PredPistol_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			PredPistol_Firing,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			PredPistol_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			PredPistol_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericPredatorWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericPredatorWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			PredPistol_Idle,  /* WEAPONSTATE_JAMMED */
			PredPistol_Idle,  /* WEAPONSTATE_WAITING */
			PredPistol_Idle,  /* WEAPONSTATE_READYING */
			PredPistol_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    16*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0, //55000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //31,
		/* RecoilMaxRandomZ; */
		0, //15,
		/* RecoilMaxXTilt; */
		0, //15,
		/* RecoilMaxYTilt; */
		0, //7,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_INGAME_PISTOL,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"pistol",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		//-1,
		/* InitialSubSequence */
		(int)PHSS_Stand,
		//-1,

		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		0,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		1,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},

	/*KJL********************
	*   WEAPON_PRED_RIFLE   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_RIFLE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireSpeargun, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			#if 0
			65536*2,					/* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			#else
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_PRIMARY */
			65536*2,					/* WEAPONSTATE_RECOIL_PRIMARY */
			#endif
			65536*4,					/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_IDLE	*/
			#if 0
			GenericPredatorWeapon_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			#else
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_FIRING_PRIMARY */
			SpearGun_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			#endif
			GenericPredatorWeapon_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericPredatorWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericPredatorWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_JAMMED */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_WAITING */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_READYING */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    640,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //80,
		/* RecoilMaxRandomZ; */
		0, //31,
		/* RecoilMaxXTilt; */
		0, //31,	//-31?
		/* RecoilMaxYTilt; */
		0, //15,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_INGAME_RIFLE,

		/* WeaponShapeName; */
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"Speargun",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Stand,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		0,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*KJL*****************************
	*   WEAPON_PRED_SHOULDERCANNON   *
	*****************************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_ENERGY_BOLT,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		//FirePCPlasmaCaster, /* FirePrimaryFunction */
		SecondaryFirePCPlasmaCaster, /* FirePrimaryFunction */
		SecondaryFirePCPlasmaCaster, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	65536*8,					/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*4,					/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			65536*8,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*3,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*3,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			31208, /* (2.1s) */ /* WEAPONSTATE_READYING */
			40960, /* (1.6s) */ /* WEAPONSTATE_UNREADYING */

		},
		{
			PlasmaCaster_Idle,  /* WEAPONSTATE_IDLE	*/
			//WristConsole_Use,  /* WEAPONSTATE_FIRING_PRIMARY */
			PlasmaCaster_Idle,  /* WEAPONSTATE_FIRING_PRIMARY */
			PlasmaCaster_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			PlasmaCaster_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			PlasmaCaster_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			Secondary_PlasmaCaster_Recoil,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			PlasmaCaster_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			TemplateHands_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			TemplateHands_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			PlasmaCaster_Idle,  /* WEAPONSTATE_JAMMED */
			PlasmaCaster_Idle,  /* WEAPONSTATE_WAITING */
			WristConsole_Readying,  /* WEAPONSTATE_READYING */
			WristConsole_Unreadying,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		0,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    0,
	    /* SmartTargetRadius in pixels */
	    65000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,350,0},
		/* Name; */
		TEXTSTRING_INGAME_SHOULDERCANNON,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    NULL,/* ie. no muzzle flash */
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"Template",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Come,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		1, //0
		/* PrimaryIsAutomatic	:1; */
		1, //0
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		1,
		/* FireSecondaryLate */
		1,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},

	/*CDF*******************
	*   WEAPON_PRED_DISC   *
	*******************CDF*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_DISC,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		PredatorDisc_Prefiring, /* FirePrimaryFunction */
		NULL, 	/* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			65536*2, 					/* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2,					/* WEAPONSTATE_RELOAD_PRIMARY */

			65536*2, 					/* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*8,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*8,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_IDLE	*/
			PredatorDisc_Throwing,  /* WEAPONSTATE_FIRING_PRIMARY */
			PredatorDisc_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			PredatorDisc_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericPredatorWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericPredatorWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_JAMMED */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_WAITING */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_READYING */
			GenericPredatorWeapon_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    20000,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,350,0},

		/* Name; */
		TEXTSTRING_INGAME_DISC,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    0,
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"disk",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Stand,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		0,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL*****************************
	*   WEAPON_PRED_MEDICOMP		 *
	*****************************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_NONE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		FireSpikeyThing, /* FirePrimaryFunction */
		FireExtinguisher, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			65536>>2, /* WEAPONSTATE_FIRING_PRIMARY */
			//65536*2, 					/* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			65536>>2, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*3,					/* WEAPONSTATE_SWAPPING_IN	*/
			65536*3,					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			31208, /* (2.1s) */ /* WEAPONSTATE_READYING */
			40960, /* (1.6s) */ /* WEAPONSTATE_UNREADYING */

		},
		{
			WristConsole_Idle,  /* WEAPONSTATE_IDLE	*/
			SpikeyThing_Use,  /* WEAPONSTATE_FIRING_PRIMARY */
			WristConsole_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			WristConsole_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			Extinguisher_Use,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			WristConsole_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			WristConsole_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			TemplateHands_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			TemplateHands_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			WristConsole_Idle,  /* WEAPONSTATE_JAMMED */
			WristConsole_Idle,  /* WEAPONSTATE_WAITING */
			WristConsole_Readying,  /* WEAPONSTATE_READYING */
			WristConsole_Unreadying,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		0,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    0,
	    /* SmartTargetRadius in pixels */
	    65000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,350,0},
		/* Name; */
		TEXTSTRING_INGAME_MEDICOMP,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    NULL,/* ie. no muzzle flash */
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"Template",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Come,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		0,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		1,
		/* FireSecondaryLate */
		1,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL*****************************
	*   WEAPON_PRED_STAFF			 *
	*****************************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_PRED_STAFF,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			65536>>2, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			65536*2, 						/* WEAPONSTATE_READYING */
			65536*2, 						/* WEAPONSTATE_UNREADYING */

		},
		{
			Staff_Idle,  /* WEAPONSTATE_IDLE	*/
			StaffAttack_Basic,  /* WEAPONSTATE_FIRING_PRIMARY */
			Staff_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			Staff_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			Staff_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			Staff_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			Staff_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			Staff_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			Staff_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			Staff_Idle,  /* WEAPONSTATE_JAMMED */
			Staff_Idle,  /* WEAPONSTATE_WAITING */
			Staff_Idle,  /* WEAPONSTATE_READYING */
			Staff_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,350,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    0,
		/* RiffName */
		"pred_HUD",
		/* HierarchyName */
		"staff",
		/* InitialSequenceType */
		(int)HMSQT_PredatorHUD,
		/* InitialSubSequence */
		(int)PHSS_Come,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		1,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		1,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		1,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},

	/*KJL********************
	*   WEAPON_ALIEN_CLAW   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_ALIEN_CLAW,
		/* SecondaryAmmoID; */
		AMMO_ALIEN_TAIL,
		   
		//MeleeWeapon_180Degree_Front, /* FirePrimaryFunction */
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			//65536*4, /* WEAPONSTATE_FIRING_PRIMARY */
			65536*6, /* WEAPONSTATE_FIRING_PRIMARY */
		   	//65536*2, /* WEAPONSTATE_RECOIL_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			65536*2,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_SWAPPING_IN	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_SWAPPING_OUT	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			AlienClaw_Idle,  /* WEAPONSTATE_IDLE	*/
			AlienClaw_Strike,  /* WEAPONSTATE_FIRING_PRIMARY */
			AlienClaw_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			AlienClaw_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			AlienTail_Poise,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			AlienTail_Strike,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			AlienClaw_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			AlienClaw_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			AlienClaw_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			AlienClaw_Idle,  /* WEAPONSTATE_JAMMED */
			AlienClaw_Idle,  /* WEAPONSTATE_WAITING */
			AlienClaw_Idle,  /* WEAPONSTATE_READYING */
			AlienClaw_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		///* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		//4,
	    ///* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    //160,
	    ///* SmartTargetRadius in pixels */
	    //0,
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		0,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    1500,
	    /* SmartTargetRadius in pixels */
	    55000,
		/* RestPosition; */
		{0,0,0},
		   
		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{600,-100,400},

		/* Name; */
		TEXTSTRING_INGAME_CLAW,

		/* WeaponShapeName; */
	    "Shell",
		/* MuzzleFlashShapeName; */
	    NULL,
		/* RiffName */
		"alien_HUD",
		/* HierarchyName */
		"claws",
		/* InitialSequenceType */
		(int)HMSQT_AlienHUD,
		/* InitialSubSequence */
		(int)AHSS_LeftSwipeDown,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		1,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		1,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL********************
	*   WEAPON_ALIEN_GRAB   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_ALIEN_TAIL,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		//MeleeWeapon_90Degree_Front, /* FirePrimaryFunction */
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			65536*6, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			65536*2,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_SWAPPING_IN	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_SWAPPING_OUT	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			AlienGrab_Idle,  /* WEAPONSTATE_IDLE	*/
			AlienGrab_Strike,  /* WEAPONSTATE_FIRING_PRIMARY */
			AlienGrab_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			AlienGrab_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			AlienGrab_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			AlienGrab_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			AlienGrab_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			AlienGrab_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			AlienGrab_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			AlienGrab_Idle,  /* WEAPONSTATE_JAMMED */
			AlienGrab_Idle,  /* WEAPONSTATE_WAITING */
			AlienGrab_Idle,  /* WEAPONSTATE_READYING */
			AlienGrab_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //0,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{300,0,500},

		/* Name; */
		TEXTSTRING_INGAME_TAIL,

		/* WeaponShapeName; */
	    "claw",
		/* MuzzleFlashShapeName; */
	    NULL,
		/* RiffName */
		"alien_HUD",
		/* HierarchyName */
		"eat",
		/* InitialSequenceType */
		(int)HMSQT_AlienHUD,
		/* InitialSubSequence */
		(int)AHSS_Eat,
		
		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		1,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		1,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJ*********************
	*   WEAPON_ALIEN_SPIT   *
	********************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_ALIEN_SPIT,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	65536*6, 					/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2/3,						/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    55000,
		/* RestPosition; */
		{0,0,0},
		
		/* RecoilMaxZ; */
		0, //200,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //24,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_BLANK,

		/* WeaponShapeName; */
	    NULL,
		/* MuzzleFlashShapeName; */
	    NULL,
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,

		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		1,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*CDF*************************
	*   WEAPON_CUDGEL            *
	*************************CDF*/
	{
		/* PrimaryAmmoID; */
		AMMO_CUDGEL,
		/* SecondaryAmmoID; */
		AMMO_CUDGEL,
		   
		//MeleeWeapon_90Degree_Front, /* FirePrimaryFunction */
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			65536>>2, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			65536>>2, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536*3,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536*3,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			65536*2, 						/* WEAPONSTATE_READYING */
			65536*2, 						/* WEAPONSTATE_UNREADYING */

		},
		{
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_IDLE	*/
			Cudgel_Strike, /* WEAPONSTATE_FIRING_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_PRIMARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RELOAD_PRIMARY */
			Cudgel_Strike, /* WEAPONSTATE_FIRING_SECONDARY */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			GenericMarineWeapon_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			GenericMarineWeapon_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_JAMMED */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_WAITING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_READYING */
			GenericMarineWeapon_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    0,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{0,0,0},

		/* RecoilMaxZ; */
		0, //-1024,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //0,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{0,0,0}, //{300,350,0}, ???

		/* Name; */
		TEXTSTRING_INGAME_PULSERIFLE,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    0,
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Cudgel",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,
		
		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif		
		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		1,
		/* PrimaryIsMeleeWeapon :1; */
		1,  
		/* SecondaryIsRapidFire   :1; */   
		1,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		1,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		0,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	/*KJL**************
	*  MARINE PISTOL  *
	**************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_MARINE_PISTOL_PC,
		/* SecondaryAmmoID; */
		AMMO_MARINE_PISTOL_PC,

		FireNonAutomaticWeapon, /* FirePrimaryFunction */
		FireNonAutomaticWeapon, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */
		   
	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_PRIMARY */
			(65536*5),					/* WEAPONSTATE_RECOIL_PRIMARY */
			((65536*2)/3),						/* WEAPONSTATE_RELOAD_PRIMARY */
										
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY */
		   	(65536*5),					/* WEAPONSTATE_RECOIL_SECONDARY */
			65536,						/* WEAPONSTATE_RELOAD_SECONDARY */

			(65536),					/* WEAPONSTATE_SWAPPING_IN	*/ /* Was >>2 */
			(65536), 					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			MarinePistol_Fidget,  /* WEAPONSTATE_IDLE	*/
			MarinePistol_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			MarinePistol_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			MarinePistol_Firing,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			MarinePistol_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			MarinePistol_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT */
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    //1000*65536/60,
		12*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius */
	    0,
		/* RestPosition; */
		//{300,400,800},
		{0,0,0},
		
		/* RecoilMaxZ; */
		90, //0, //60,
		/* RecoilMaxRandomZ; */
		60, //0, //31,
		/* RecoilMaxXTilt; */
		30, //0, //31,
		/* RecoilMaxYTilt; */
		30, //0, //15,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_INGAME_MARINE_PISTOL,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Pistol",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*CDF**************************
	*   WEAPON_FRISBEE_LAUNCHER   *
	**************************CDF*/
	{
		/* PrimaryAmmoID; */
		AMMO_FRISBEE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		PredatorDisc_Prefiring, /* FirePrimaryFunction.  It's empty. */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			//WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
			//((65536*1000)/1625), /* WEAPONSTATE_FIRING_PRIMARY */
			((65536*2000)/1625), /* WEAPONSTATE_FIRING_PRIMARY */
		   	(65000), 						/* WEAPONSTATE_RECOIL_PRIMARY */
			65536*2/3,					/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			SADAR_Fidget,  /* WEAPONSTATE_IDLE	*/
			Frisbee_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			Frisbee_Recoil,  /* WEAPONSTATE_RECOIL_PRIMARY */
			SADAR_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			SADAR_Idle,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			SADAR_Idle,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			SADAR_Idle,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			SADAR_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			SADAR_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT	*/
			SADAR_Idle,  /* WEAPONSTATE_JAMMED */
			SADAR_Idle,  /* WEAPONSTATE_WAITING */
			SADAR_Idle,  /* WEAPONSTATE_READYING */
			SADAR_Idle,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    0,
		/* RestPosition; */
		{200,0,0},
		
		/* RecoilMaxZ; */
		0, //200,
		/* RecoilMaxRandomZ; */
		0, //0,
		/* RecoilMaxXTilt; */
		0, //24,
		/* RecoilMaxYTilt; */
		0, //0,

		/* StrikePosition */
		{0,0,0},

		/* Name; */
		TEXTSTRING_INGAME_SKEETER,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"SD",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			0, /* MovementMultiple	*/
			0, /* TurningMultiple */
			0, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif
		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},
	/*KJL************
	*  TWO PISTOLS  *
	************KJL*/
	{
		/* PrimaryAmmoID; */
		AMMO_MARINE_PISTOL_PC,
		/* SecondaryAmmoID; */
		AMMO_MARINE_PISTOL_PC,

		FireMarineTwoPistolsPrimary, /* FirePrimaryFunction */
		FireMarineTwoPistolsSecondary, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */
		   
	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			65536,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_FIRING_PRIMARY */
			//(65536*5),					/* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			((65536)/3),						/* WEAPONSTATE_RELOAD_PRIMARY */
										
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_RECOIL_SECONDARY */
		   	//(65536*5),					/* WEAPONSTATE_RECOIL_SECONDARY */
			65536,						/* WEAPONSTATE_RELOAD_SECONDARY */

			(65536),					/* WEAPONSTATE_SWAPPING_IN	*/ /* Was >>2 */
			(65536), 					/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			MarineTwoPistols_Fidget,  /* WEAPONSTATE_IDLE	*/
			MarineTwoPistols_Firing,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			MarineTwoPistols_Reload,  /* WEAPONSTATE_RELOAD_PRIMARY */
			MarineTwoPistols_SecondaryFiring,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			MarineTwoPistols_SwapIn,  /* WEAPONSTATE_SWAPPING_IN	*/
			MarineTwoPistols_SwapOut,  /* WEAPONSTATE_SWAPPING_OUT */
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    //1000*65536/60,
		12*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius */
	    0,
		/* RestPosition; */
		//{300,400,800},
		{0,0,0},
		
		/* RecoilMaxZ; */
		90, //0, //60,
		/* RecoilMaxRandomZ; */
		60, //0, //31,
		/* RecoilMaxXTilt; */
		30, //0, //31,
		/* RecoilMaxYTilt; */
		30, //0, //15,

		/* StrikePosition */
		{0,0,0},
		
		/* Name; */
		TEXTSTRING_INGAME_TWOPISTOLS,

		/* WeaponShapeName; */
		/* dummy shape*/
		"Shell",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		"MarineWeapons",
		/* HierarchyName */
		"Two pistol",
		/* InitialSequenceType */
		(int)HMSQT_MarineHUD,
		/* InitialSubSequence */
		(int)MHSS_Stationary,

		#if USE_ENCUMBERANCE
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#else
		{ /* Encum_Idle */
			7*ONE_FIXED/8, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			7*ONE_FIXED/8, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			2*ONE_FIXED/3, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED/2, /* MovementMultiple	*/
			7*ONE_FIXED/8, /* TurningMultiple */
			2*ONE_FIXED/3, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		#endif		
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,  
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		1,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		1,
		/* SecondaryMuzzleFlash */
		1,
		/* LogAccuracy */
		1,
		/* LogShots */
		1,
	},

	#if 0
	/*KJL***********
	* 	TEMPLATE   *
	***********KJL*/
	{
		/* PrimaryAmmoID; */
		/* SecondaryAmmoID; */
		AMMO_NONE,
		/* SecondaryAmmoID; */
		AMMO_NONE,
		   
		NULL, /* FirePrimaryFunction */
		NULL, /* FireSecondaryFunction */
		NULL,	/* WeaponInitFunction */

	    /* TimeOutRateForState[MAX_NO_OF_WEAPON_STATES]; in 16.16 */
		{
			0,/* WEAPONSTATE_IDLE	*/
			
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_PRIMARY */
		   	WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_PRIMARY */
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_PRIMARY */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_FIRING_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RECOIL_SECONDARY	*/
			WEAPONSTATE_INSTANTTIMEOUT,	/* WEAPONSTATE_RELOAD_SECONDARY	*/
										
			65536,						/* WEAPONSTATE_SWAPPING_IN	*/
			65536,						/* WEAPONSTATE_SWAPPING_OUT	*/
			65536,						/* WEAPONSTATE_JAMMED */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_WAITING */

			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_READYING */
			WEAPONSTATE_INSTANTTIMEOUT, /* WEAPONSTATE_UNREADYING */

		},
		{
			NULL,  /* WEAPONSTATE_IDLE	*/
			NULL,  /* WEAPONSTATE_FIRING_PRIMARY */
			NULL,  /* WEAPONSTATE_RECOIL_PRIMARY */
			NULL,  /* WEAPONSTATE_RELOAD_PRIMARY */
			NULL,  /* WEAPONSTATE_FIRING_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RECOIL_SECONDARY	*/
			NULL,  /* WEAPONSTATE_RELOAD_SECONDARY	*/
			NULL,  /* WEAPONSTATE_SWAPPING_IN	*/
			NULL,  /* WEAPONSTATE_SWAPPING_OUT	*/
			NULL,  /* WEAPONSTATE_JAMMED */
			NULL,  /* WEAPONSTATE_WAITING */
			NULL,  /* WEAPONSTATE_READYING */
			NULL,  /* WEAPONSTATE_UNREADYING */
		},
		/* ProbabilityOfJamming; */
	    32,
	    /* FiringRate;	*/
	    1*65536,
	    
		/* SmartTargetSpeed; signed int, how fast the crosshair moves. */
		4,
	    /* GunCrosshairSpeed;  integer, how fast the gun moves. */
	    160,
	    /* SmartTargetRadius in pixels */
	    50,
		/* RestPosition; */
		{300,400,800},
		
		/* RecoilMaxZ; */
		60,
		/* RecoilMaxRandomZ; */
		31,
		/* RecoilMaxXTilt; */
		31,
		/* RecoilMaxYTilt; */
		15,

		/* Name; */
		TEXTSTRING_INGAME_BLANK,

		/* WeaponShapeName; */
	    "Plasma",
		/* MuzzleFlashShapeName; */
	    "Sntrymuz",
		/* RiffName */
		NULL,
		/* HierarchyName */
		NULL,
		/* InitialSequenceType */
		-1,
		/* InitialSubSequence */
		-1,

		{ /* Encum_Idle */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FirePrime */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		{ /* Encum_FireSec */
			ONE_FIXED, /* MovementMultiple	*/
			ONE_FIXED, /* TurningMultiple */
			ONE_FIXED, /* JumpingMultiple */
			1, /* CanCrouch */
			1, /* CanRun */
		},
		/* UseStateMovement :1; */
		0,
		/* IsSmartTarget :1; */
		0,
		/* PrimaryIsRapidFire   :1; */   
		0,  
		/* PrimaryIsAutomatic	:1; */
		0,
		/* PrimaryIsMeleeWeapon :1; */
		0,
		/* SecondaryIsRapidFire   :1; */   
		0,  
		/* SecondaryIsAutomatic	:1; */
		0,
		/* SecondaryIsMeleeWeapon :1; */
		0,  
		/* HasShapeAnimation */
		0,
		/* HasTextureAnimation */
		0,
		/* FireWhenCloaked */
		1,
		/* FireInChangeVision */
		1,
		/* FirePrimaryLate */
		0,
		/* FireSecondaryLate */
		0,
		/* PrimaryMuzzleFlash */
		0,
		/* SecondaryMuzzleFlash */
		0,
		/* LogAccuracy */
		0,
		/* LogShots */
		0,
	},
	#endif
};

TEMPLATE_AMMO_DATA		TemplateAmmo[MAX_NO_OF_AMMO_TEMPLATES] =
{
	/* AMMO_10MM_CULW */
	{
		99*65536,		/* AmmoPerMagazine */
		{
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW,
			},
		},				
		0,				/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_10MM_CULW, /* ShortName */
		0,				/* CreatesProjectile */
		0,
	},
	/* AMMO_SHOTGUN */
	{
		20*65536,
		{
			{
				20,10,0,0,0,0,	/* Impact point damage */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SHOTGUN,
			},
			{
				20,10,0,0,0,0,	/* Impact point damage */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SHOTGUN,
			},
			{
				20,10,0,0,0,0,	/* Impact point damage */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SHOTGUN,
			},
			{
				20,10,0,0,0,0,	/* Impact point damage */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SHOTGUN,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_SHOTGUN, /* ShortName */
		0,
		0,
	},
	/* AMMO_SMARTGUN */
	{
		500*65536,
		{
			//6,0,6,0,0,0,
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_SMARTGUN, /* ShortName */
		0,
		0,
	},
	/* AMMO_FLAMETHROWER */
	{
		100*65536,
		{
			{
				0,0,0,25,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLAMETHROWER,
			},
			{
				0,0,0,25,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLAMETHROWER,
			},
			{
				0,0,0,25,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLAMETHROWER,
			},
			{
				0,0,0,25,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLAMETHROWER,
			},
		},
		500,
		TEXTSTRING_AMMO_SHORTNAME_FLAMETHROWER, /* ShortName */
		1,
		0,
	},
	/* AMMO_PLASMA */
	{
		10*65536,		 /* AmmoPerMagazine */
		{
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMA,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMA,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMA,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMA,
			},
		},  /* MaxDamage */
		5000,			 /* MaxRange */
		TEXTSTRING_BLANK, /* ShortName */
		1,				 /* CreatesProjectile */
		0,
	},
	/* AMMO_SADAR_TOW */
	{
		1*65536,
		{
			{
				0,0,500,0,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_TOW,
			},
			{
				0,0,500,0,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_TOW,
			},
			{
				0,0,500,0,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_TOW,
			},
			{
				0,0,500,0,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_TOW,
			},
		},
		14000, //Was 7500,
		TEXTSTRING_AMMO_SHORTNAME_SADAR_TOW, /* ShortName */
		1,
		0
	},
	/* AMMO_GRENADE */ 
	{
		6*65536,
		{
			{
				110,0,1,5,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_GRENADE,
			},
			{
				110,0,1,5,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_GRENADE,
			},
			{
				110,0,1,5,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_GRENADE,
			},
			{
				110,0,1,5,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_GRENADE,
			},
		},
		10000,
		TEXTSTRING_AMMO_SHORTNAME_GRENADE, /* ShortName */
		1,
		1
	},
	/* AMMO_MINIGUN */
	{
		800*65536,
		{
			//11,0,1,0,0,0,
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_MINIGUN, /* ShortName */
		0,
		0
	},
	/* AMMO_PULSE_GRENADE */
	{
		5*65536,
		{
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_PULSE_GRENADE, /* ShortName */
		1,
		1
	},
	/* AMMO_FLARE_GRENADE */
	{
		6*65536,
		{
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLARE_GRENADE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLARE_GRENADE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLARE_GRENADE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FLARE_GRENADE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_FLARE_GRENADE, /* ShortName */
		1,
		0
	},
	/* AMMO_FRAGMENTATION_GRENADE */
	{
		6*65536,
		{	
			{
				40,10,1,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRAGMENTATION_GRENADE,
			},
			{
				40,10,1,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRAGMENTATION_GRENADE,
			},
			{
				40,10,1,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRAGMENTATION_GRENADE,
			},
			{
				40,10,1,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRAGMENTATION_GRENADE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_FRAGMENTATION_GRENADE, /* ShortName */
		1,
		1
	},
	/* AMMO_PROXIMITY_GRENADE */
	{
		6*65536,
		{
			{
				40,0,1,5,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PROXIMITY_GRENADE,
			},
			{
				40,0,1,5,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PROXIMITY_GRENADE,
			},
			{
				40,0,1,5,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PROXIMITY_GRENADE,
			},
			{
				40,0,1,5,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PROXIMITY_GRENADE,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_PROXIMITY_GRENADE, /* ShortName */
		1,
		1
	},
	/* AMMO_PARTICLE_BEAM */
	{
		100*65536,
		{
			{
				0,0,0,0,15,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PARTICLE_BEAM,
			},
			{
				0,0,0,0,15,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PARTICLE_BEAM,
			},
			{
				0,0,0,0,15,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PARTICLE_BEAM,
			},
			{
				0,0,0,0,15,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PARTICLE_BEAM,
			},
		},
		0,
		TEXTSTRING_BLANK, /* ShortName */
		0,
		0
	},
	/* AMMO_SONIC_PULSE */
	{
		100*65536,	 /* AmmoPerMagazine */
		{
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SONIC_PULSE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SONIC_PULSE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SONIC_PULSE,
			},
			{
				0,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SONIC_PULSE,
			},
		},			 /* MaxDamage */
		0,			 /* MaxRange */
		TEXTSTRING_BLANK, /* ShortName */
		1,			 /* CreatesProjectile */
		0
	},
	
	/* PREDATOR */

	/* AMMO_PRED_WRISTBLADE */
	{
		0,
		{
			{
				0,10,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_WRISTBLADE,
			},
			{
				0,10,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_WRISTBLADE,
			},
			{
				0,10,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_WRISTBLADE,
			},
			{
				0,10,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_WRISTBLADE,
			},
		},
		2500,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	#if 0
	/* AMMO_PRED_PISTOL */
	{
		100*65536,		/* AmmoPerMagazine */
		{
			{
				20,0,0,5,2,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				20,0,0,5,2,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				20,0,0,5,2,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				20,0,0,5,2,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
		},				/* MaxDamage */
		5000,			/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,				/* CreatesProjectile */
		0				
	},
	#else
	/* AMMO_PRED_PISTOL */
	{
		100*65536,		/* AmmoPerMagazine */
		{
			{
				0,0,0,0,20,0,
				3,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				0,0,0,0,20,0,
				3,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				0,0,0,0,20,0,
				3,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
			{
				0,0,0,0,20,0,
				3,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_PISTOL,
			},
		},				/* MaxDamage */
		5000,			/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,				/* CreatesProjectile */
		0				/* ExplosionIsFlat */
	},
	#endif
	/* AMMO_PRED_RIFLE */
	{
		20*65536,		/* AmmoPerMagazine */
		{
			{
				//0,0,40,0,10,0,  //That's just wuss!
				0,0,200,0,20,0,
				0,	/* ExplosivePower */
				2,	/* Slicing */
				1,	/* ProduceBlood */
				1,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_PRED_RIFLE,
			},
			{
				0,0,200,0,20,0,
				0,	/* ExplosivePower */
				2,	/* Slicing */
				1,	/* ProduceBlood */
				1,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_PRED_RIFLE,
			},
			{
				0,0,200,0,20,0,
				0,	/* ExplosivePower */
				2,	/* Slicing */
				1,	/* ProduceBlood */
				1,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_PRED_RIFLE,
			},
			{
				0,0,200,0,20,0,
				0,	/* ExplosivePower */
				2,	/* Slicing */
				1,	/* ProduceBlood */
				1,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_PRED_RIFLE,
			},
		},				/* MaxDamage */
		0,				/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,				/* CreatesProjectile */
		0
	},
	/* AMMO_PRED_ENERGY_BOLT */
	{
		99*65536,
		{
			{
				50,0,300,50,100,0,
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_ENERGY_BOLT,
			},
			{
				50,0,300,50,100,0,
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_ENERGY_BOLT,
			},
			{
				50,0,300,50,100,0,
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_ENERGY_BOLT,
			},
			{
				50,0,300,50,100,0,
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_ENERGY_BOLT,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_PRED_DISC */
	{
		1*65536,
		{
			{
				0,300,0,0,0,0,
				0,	/* ExplosivePower */
				3,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC,
			},
			{
				0,300,0,0,0,0,
				0,	/* ExplosivePower */
				3,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC,
			},
			{
				0,300,0,0,0,0,
				0,	/* ExplosivePower */
				3,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC,
			},
			{
				0,300,0,0,0,0,
				0,	/* ExplosivePower */
				3,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC,
			},
		},
		1000,
		TEXTSTRING_INGAME_DISC,
//		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		1
	},


	/* ALIEN */

	/* AMMO_ALIEN_CLAW */
	{
		0,
		{
			{
				0,21,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_CLAW,
			},
			{
				0,21,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_CLAW,
			},
			{
				0,21,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_CLAW,
			},
			{
				0,21,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_CLAW,
			},
		},
		4000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_ALIEN_TAIL */
	{
		0,
		{
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_TAIL,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_TAIL,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_TAIL,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_TAIL,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_ALIEN_SPIT */
	{
		10,
		{
			{
				0,30,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_SPIT,
			},
			{
				0,30,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_SPIT,
			},
			{
				0,30,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_SPIT,
			},
			{
				0,30,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_SPIT,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},

	/* MISC AND OUT OF SEQUENCE THINGS */

	/* AMMO_AUTOGUN */
	{
		0,
		{
			{
				2,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_AUTOGUN,
			},
			{
				2,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_AUTOGUN,
			},
			{
				2,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_AUTOGUN,
			},
			{
				2,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_AUTOGUN,
			},
		},
		0,
		TEXTSTRING_BLANK, /* ShortName */
		0,
		0
	},
	/* AMMO_XENOBORG */
	{
		0,
		{
			{
				0,10,0,0,10,0, // A bit wuss. Placeholder.
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_XENOBORG,
			},
			{
				0,10,0,0,10,0, // A bit wuss. Placeholder.
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_XENOBORG,
			},
			{
				0,10,0,0,10,0, // A bit wuss. Placeholder.
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_XENOBORG,
			},
			{
				0,10,0,0,10,0, // A bit wuss. Placeholder.
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_XENOBORG,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_FACEHUGGER */
	{
		0,
		{
			{
				0,40,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FACEHUGGER,
			},
			{
				0,40,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FACEHUGGER,
			},
			{
				0,40,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FACEHUGGER,
			},
			{
				0,40,0,0,0,20,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FACEHUGGER,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_OBSTACLE_CLEAR */
	{
		0,
		{
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_OBSTACLE_CLEAR,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_ALIEN_FRAG */
	{
		0,
		{
			{
				0,0,0,0,0,10, // Change me!
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_FRAG,
			},
			{
				0,0,0,0,0,10, // Change me!
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_FRAG,
			},
			{
				0,0,0,0,0,10, // Change me!
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_FRAG,
			},
			{
				0,0,0,0,0,10, // Change me!
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_FRAG,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_ALIEN_DEATH */
	{
		0,
		{
			{
				0,0,0,0,0,10,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_DEATH,
			},
			{
				0,0,0,0,0,10,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_DEATH,
			},
			{
				0,0,0,0,0,10,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_DEATH,
			},
			{
				0,0,0,0,0,10,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_DEATH,
			},
		},
		3000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_SHOTGUN_BLAST */
	{
		20*65536,
		{
			{
				10,0,0,5,0,0,	/* Blast damage */
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SHOTGUN_BLAST,
			},
			{
				10,0,0,5,0,0,	/* Blast damage */
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SHOTGUN_BLAST,
			},
			{
				10,0,0,5,0,0,	/* Blast damage */
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SHOTGUN_BLAST,
			},
			{
				10,0,0,5,0,0,	/* Blast damage */
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SHOTGUN_BLAST,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_SADAR_BLAST */
	{
		1*65536,
		{
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_SADAR_BLAST,
			},
		},
		10000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_ALIEN_BITE_KILLSECTION */
	{
		1*65536,
		{
			/*Make the damage 501 instead of 500 , so it can be identified as different from all other damage types*/
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},

	/* AMMO_PRED_DISC_PM */
	{
		1*65536,
		{
			{
				100,0,0,20,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC_PM,
			},
			{
				100,0,0,20,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC_PM,
			},
			{
				100,0,0,20,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC_PM,
			},
			{
				100,0,0,20,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_DISC_PM,
			},
		},
		12000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		1
	},
	/* AMMO_NPC_ALIEN_CLAW */
	{
		0,
		{
			{
				0,10,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_CLAW,
			},
			{
				0,10,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_CLAW,
			},
			{
				0,10,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_CLAW,
			},
			{
				0,10,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_CLAW,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PAQ_CLAW */
	{
		0,
		{
			{
				0,70,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PAQ_CLAW,
			},
			{
				0,70,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PAQ_CLAW,
			},
			{
				0,70,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PAQ_CLAW,
			},
			{
				0,70,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PAQ_CLAW,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_PULSE_GRENADE_STRIKE */
	{
		0,
		{
			{
				50,0,30,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE_STRIKE,
			},
			{
				50,0,30,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE_STRIKE,
			},
			{
				50,0,30,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE_STRIKE,
			},
			{
				50,0,30,10,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PULSE_GRENADE_STRIKE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_ALIEN_TAIL */
	{
		0,
		{
			{
				0,10,30,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_TAIL,
			},
			{
				0,10,30,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_TAIL,
			},
			{
				0,10,30,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_TAIL,
			},
			{
				0,10,30,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_TAIL,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_ALIEN_BITE */
	{
		0,
		{	
			{
				0,20,10,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_BITE,
			},
			{
				0,20,10,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_BITE,
			},
			{
				0,20,10,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_BITE,
			},
			{
				0,20,10,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_ALIEN_BITE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PREDALIEN_CLAW */
	{
		0,
		{
			{
				20,20,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_CLAW,
			},
			{
				20,20,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_CLAW,
			},
			{
				20,20,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_CLAW,
			},
			{
				20,20,0,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_CLAW,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PREDALIEN_BITE */
	{
		0,
		{
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_BITE,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_BITE,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_BITE,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_BITE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PREDALIEN_TAIL */
	{
		0,
		{
			{
				10,10,40,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_TAIL,
			},
			{
				10,10,40,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_TAIL,
			},
			{
				10,10,40,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_TAIL,
			},
			{
				10,10,40,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PREDALIEN_TAIL,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PRAETORIAN_CLAW */
	{
		0,
		{
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_CLAW,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_CLAW,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_CLAW,
			},
			{
				0,20,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_CLAW,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PRAETORIAN_BITE */
	{
		0,
		{
			{
				0,30,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_BITE,
			},
			{
				0,30,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_BITE,
			},
			{
				0,30,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_BITE,
			},
			{
				0,30,20,0,0,2,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_BITE,
			},
		},	 		
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PRAETORIAN_TAIL */
	{
		0,
		{
			{
				0,10,60,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_TAIL,
			},
			{
				0,10,60,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_TAIL,
			},
			{
				0,10,60,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_TAIL,
			},
			{
				0,10,60,0,0,2,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRAETORIAN_TAIL,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_PRED_STAFF */
	{
		0,
		{
			{
				0,120,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_STAFF,
			},
			{
				0,120,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_STAFF,
			},
			{
				0,120,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_STAFF,
			},
			{
				0,120,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_STAFF,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_NPC_PRED_STAFF */
	{
		0,
		{
			{
				0,80,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRED_STAFF,
			},
			{
				0,80,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRED_STAFF,
			},
			{
				0,80,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRED_STAFF,
			},
			{
				0,80,0,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_NPC_PRED_STAFF,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_PC_ALIEN_BITE */
	{
		0,
		{
			{
				0,0,45,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PC_ALIEN_BITE,
			},
			{
				0,0,45,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PC_ALIEN_BITE,
			},
			{
				0,0,45,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PC_ALIEN_BITE,
			},
			{
				0,0,45,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PC_ALIEN_BITE,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_HEAVY_PRED_WRISTBLADE */
	{
		0,
		{
			{
				0,80,0,0,0,60,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_HEAVY_PRED_WRISTBLADE,
			},
			{
				0,80,0,0,0,60,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_HEAVY_PRED_WRISTBLADE,
			},
			{
				0,80,0,0,0,60,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_HEAVY_PRED_WRISTBLADE,
			},
			{
				0,80,0,0,0,60,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_HEAVY_PRED_WRISTBLADE,
			},
		},
		2500,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_MARINE_PISTOL */
	{
		12*65536,		/* AmmoPerMagazine */
		{
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL,
			},
		},				
		0,				/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,				/* CreatesProjectile */
		0
	},
	/* AMMO_PREDPISTOL_STRIKE */
	{
		100*65536,		/* AmmoPerMagazine */
		{
			{
				0,0,0,0,30,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PREDPISTOL_STRIKE,
			},
			{
				0,0,0,0,30,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PREDPISTOL_STRIKE,
			},
			{
				0,0,0,0,30,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PREDPISTOL_STRIKE,
			},
			{
				0,0,0,0,30,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PREDPISTOL_STRIKE,
			},
		},				/* MaxDamage */
		5000,			/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,				/* CreatesProjectile */
		0				/* ExplosionIsFlat */
	},
	/* AMMO_PLASMACASTER_NPCKILL */
	{
		99*65536,
		{
			{
				0,0,12,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_NPCKILL,
			},
			{
				0,0,12,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_NPCKILL,
			},
			{
				0,0,12,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_NPCKILL,
			},
			{
				0,0,12,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_NPCKILL,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_PLASMACASTER_PCKILL */
	{
		99*65536,
		{
			{
				15,0,65,15,25,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_PCKILL,
			},
			{
				15,0,65,15,25,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_PCKILL,
			},
			{
				15,0,65,15,25,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_PCKILL,
			},
			{
				15,0,65,15,25,0,	/* MaxDamage - I,C,P,F,E,A */
				4,	/* ExplosivePower */
				0,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PLASMACASTER_PCKILL,
			},
		},
		2000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_10MM_CULW_NPC */
	{
		99*65536,		/* AmmoPerMagazine */
		{
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW_NPC,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW_NPC,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW_NPC,
			},
			{
				2,0,8,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_10MM_CULW_NPC,
			},
		},				
		0,				/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_10MM_CULW, /* ShortName */
		0,				/* CreatesProjectile */
		0,
	},
	/* AMMO_SMARTGUN_NPC */
	{
		500*65536,
		{
			//6,0,6,0,0,0,
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN_NPC,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN_NPC,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN_NPC,
			},
			{
				8,0,2,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_SMARTGUN_NPC,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_SMARTGUN, /* ShortName */
		0,
		0,
	},
	/* AMMO_MINIGUN_NPC */
	{
		800*65536,
		{
			//11,0,1,0,0,0,
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN_NPC,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN_NPC,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN_NPC,
			},
			{
				20,0,8,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MINIGUN_NPC,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_MINIGUN, /* ShortName */
		0,
		0
	},
	/* AMMO_MOLOTOV */ 
	{
		6*65536,
		{
			{
				1,0,1,5,0,0,
				5,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_MOLOTOV,
			},
			{
				1,0,1,5,0,0,
				5,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_MOLOTOV,
			},
			{
				1,0,1,5,0,0,
				5,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_MOLOTOV,
			},
			{
				1,0,1,5,0,0,
				5,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_MOLOTOV,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_GRENADE, /* ShortName */
		1,
		1
	},
	/* AMMO_ALIEN_OBSTACLE_CLEAR */
	{
		0,
		{
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_OBSTACLE_CLEAR,
			},
			{
				0,30,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_OBSTACLE_CLEAR,
			},
		},
		0,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_PRED_TROPHY_KILLSECTION */
	{
		1*65536,
		{
			/*Make the damage 501 instead of 500 , so it can be identified as different from all other damage types*/
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_TROPHY_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_TROPHY_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_TROPHY_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_PRED_TROPHY_KILLSECTION,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_CUDGEL */
	{
		0,
		{
			{
				10,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_CUDGEL,
			},
			{
				10,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_CUDGEL,
			},
			{
				10,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_CUDGEL,
			},
			{
				10,0,0,0,0,0,
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_CUDGEL,
			},
		},
		2500,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		0,
		0
	},
	/* AMMO_ALIEN_BITE_KILLSECTION_SUPER */
	{
		1*65536,
		{
			/*Make the damage 501 instead of 500 , so it can be identified as different from all other damage types*/
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
			{
				0,0,501,0,0,0,
				0,	/* ExplosivePower */
				1,	/* Slicing */
				1,	/* ProduceBlood */
				0,	/* ForceBoom */
				1,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_ALIEN_BITE_KILLSECTION,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_MARINE_PISTOL_PC */
	{
		12*65536,		/* AmmoPerMagazine */
		{
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL_PC,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL_PC,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL_PC,
			},
			{
				4,0,16,0,0,0,	/* MaxDamage - I,C,P,F,E,A */
				0,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				1,	/* MakeExitWounds */
				AMMO_MARINE_PISTOL_PC,
			},
		},				
		0,				/* MaxRange */
		TEXTSTRING_AMMO_SHORTNAME_MARINE_PISTOL, /* ShortName */
		0,				/* CreatesProjectile */
		0
	},
	/* AMMO_FRISBEE */
	{
		1*65536,
		{
			{
				0,0,500,0,0,0,
				6,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE,
			},
			{
				0,0,500,0,0,0,
				6,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE,
			},
			{
				0,0,500,0,0,0,
				6,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE,
			},
			{
				0,0,500,0,0,0,
				6,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE,
			},
		},
		14000, //Was 7500,
		TEXTSTRING_AMMO_SHORTNAME_SKEETER, /* ShortName */
		1,
		0
	},
	/* AMMO_FRISBEE_BLAST */
	{
		1*65536,
		{
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_BLAST,
			},
			{
				60,0,0,10,0,0,
				2,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_BLAST,
			},
		},
		10000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		0
	},
	/* AMMO_FRISBEE_FIRE */
	{
		5*65536,
		{
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_FIRE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_FIRE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_FIRE,
			},
			{
				50,0,1,0,0,0,
				1,	/* ExplosivePower */
				0,	/* Slicing */
				0,	/* ProduceBlood */
				0,	/* ForceBoom */
				0,	/* BlowUpSections */
				0,	/* Special */
				0,	/* MakeExitWounds */
				AMMO_FRISBEE_FIRE,
			},
		},
		5000,
		TEXTSTRING_AMMO_SHORTNAME_UNKNOWN, /* ShortName */
		1,
		1
	},

};

/* CDF 4/8/98 - placing these here to centralise all DAMAGE_PROFILEs */

DAMAGE_PROFILE certainDeath = {0,0,10000,0,0,0, 0,0,0,0,0,0,0,AMMO_NONE};
DAMAGE_PROFILE console_nuke = {0,0,0,0,1000,0, 0,0,0,0,0,0,0,AMMO_NONE}; 
DAMAGE_PROFILE firedamage 	= {0,0,0,5,0,0, 0,0,0,0,0,0,0,AMMO_FIREDAMAGE_POSTMAX}; 

//Deamage caused by placed objects that explode when destroyed
DAMAGE_PROFILE SmallExplosionDamage = {50,0,1,0,0,0, 1,0,0,0,0,0,AMMO_NONE};
DAMAGE_PROFILE BigExplosionDamage = {60,0,10,0,0,0, 2,0,0,0,0,0,AMMO_NONE};

/* KJL 17:05:19 27/08/98 - Flechette damage */
DAMAGE_PROFILE FlechetteDamage={0,10,0,0,0,0,0,0,0,0,0,0,1,AMMO_FLECHETTE_POSTMAX};

/* CDF 16:45 9/11/98 - Fan damage, from bh_fan.c */
DAMAGE_PROFILE fan_damage={0,100,0,0,0,0,2,1,1,0,0,0,AMMO_NONE};

/* KJL 18:29:27 10/11/98 - Falling damage */
/* CDF 17:52:00 22/2/99 Changed to Pen from Electrical, to fix NPC death selection */
DAMAGE_PROFILE FallingDamage={0,0,1,0,0,0,0,0,0,0,0,0,0,AMMO_FALLING_POSTMAX};

/* CDF 7/12/98 Pred Pistol Flechette Damage */
DAMAGE_PROFILE PredPistol_FlechetteDamage={0,0,0,0,1,0,0,0,0,0,0,0,1,AMMO_NONE};

/*Damage profiles related to queen level*/
DAMAGE_PROFILE QueenButtDamage={40,0,0,0,0,0,0,0,0,0,0,0,AMMO_NONE};
//the impact damage entry is filled in when the damage is done
DAMAGE_PROFILE QueenImpactDamage={0,0,0,0,0,0,0,0,0,0,0,0,AMMO_NONE}; 
DAMAGE_PROFILE VacuumDamage={0,0,0,0,20,0,0,0,0,0,0,0,AMMO_NONE};


//Damage for death volumes that do damage per second
DAMAGE_PROFILE DeathVolumeDamage={0,0,1,0,0,0,0,0,0,0,0,0,0,AMMO_NONE};

/* KJL 11:23:25 04/07/97 - hackette for the grenade launcher which has 4 ammo types */
GRENADE_LAUNCHER_DATA GrenadeLauncherData;
PRED_DISC_MODES ThisDiscMode;
SMARTGUN_MODES SmartgunMode;
/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/

/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/

/*KJL**********************************************************
* Initialise the generic data used for weapon templates, etc. *
**********************************************************KJL*/
void InitialiseEquipment(void)
{
	#if 0
    int i = MAX_NO_OF_WEAPON_TEMPLATES;

	while(i--)
	{
		TemplateWeapon[i].RestPosition.vx = 0;	
		TemplateWeapon[i].RestPosition.vy = 0;	
		TemplateWeapon[i].RestPosition.vz = 0;	
	}
	#endif
	/* KJL 15:47:30 03/19/97 - not much happening here */	
}





/*
	10 mm culw rounds   (Ceramic - Ultra Light Weight) 0.05 Kg each. (1.5 oz!!)
	20 mm culw rounds   (Ceramic - Ultra Light Weight) 0.08 Kg each. (1.5 oz!!)
	standard grenades 0.2 Kg each .
	Heavy grenades 0.4 Kg each.
	shotgun rounds 0.1 each	

	0	standard grenade					1  Kg
	1  	99x10 mm magizines of culw rounds.  5  Kg 
 	2  	500x10 mm magazines of culw rounds.	25 Kg
	3	5xstandard grenades					1  Kg
	4   5 Heavy grenades HE		            2  Kg
	5	5 Heavy grenades Napalm				2  Kg
	6 5 HEavy grenades Canister			2  Kg
	7	5 Heavy Grenades Cluster			2  Kg
	8	5 Heavy Grenades WP					2  Kg						
	9	20x shotgun rounds					2  Kg
	10   20 litres of fuel					18 Kg
	11	500x20 mm magizines of culw rounds	40 Kg
	12	Sonic Gun Power Packs				5  Kg


*/
#if 0
/* Prototype HModel for minigun */

KEYFRAME_DATA Handle_First_Frame = {
	{0,0,0},	/* Offset */
	{0,0,0}, /* Deltas */
	{ONE_FIXED,0,0,0}, /* Quat? */
	{0,0,0,0,}, /* Next quat */
	0,	/* Omega */
	0,	/* oneoversinomega */
	0,  /* oneoversequencelength */
	65535, /* Time to next frame */
	NULL, /* Pointer to next frame */
};

KEYFRAME_DATA Barrel_Second_Frame = {
	{129,50,674},	/* Offset */
	{0,0,0}, /* Deltas */
	{ONE_FIXED>>1,0,0,56756}, /* Quat? */
	{0,0,0,0,}, /* Next quat */
	0,	/* Omega */
	0,	/* oneoversinomega */
	0,  /* oneoversequencelength */
	ONE_FIXED, /* Time to next frame */
	NULL, /* Pointer to next frame */
};

KEYFRAME_DATA Barrel_First_Frame = {
	{129,50,674},	/* Offset */
	{0,0,0}, /* Deltas */
	{ONE_FIXED,0,0,0}, /* Quat? */
	{0,0,0,0,}, /* Next quat */
	0,	/* Omega */
	0,	/* oneoversinomega */
	0,  /* oneoversequencelength */
	ONE_FIXED, /* Time to next frame */
	&Barrel_Second_Frame, /* Pointer to next frame */
};

SEQUENCE Handle_Sequence = {
	0,
	&Handle_First_Frame	
};

SEQUENCE Barrel_Sequence = {
	0,
	&Barrel_First_Frame
};

SECTION H_Minigun_Barrel = {
	0,
	NULL,
	"Hminibar",
	"MinigunBarrel",
	NULL,
	1,
	&Barrel_Sequence,
	{
		100,	/* Health */
		50,		/* Armour */
		0, /* IsOnFire */
		{
			0,	/* Acid Resistant */
			1,	/* Fire Resistant */
			1,	/* Electric Resistant */
			1,	/* Perfect Armour */
			0,	/* Electric Sensitive */
			0,	/* Combustability */
		},
	},
	{0,0,0,},
	0
};

SECTION *Handle_Branches[] = {
	&H_Minigun_Barrel,
	NULL
};

SECTION H_Minigun_Handle = {
	0,
	NULL,
	"Hminihan",
	"MinigunHandle",
	Handle_Branches,
	1,
	&Handle_Sequence,
	{
		100,	/* Health */
		50,		/* Armour */
		0, /* IsOnFire */
		{
			0,	/* Acid Resistant */
			1,	/* Fire Resistant */
			1,	/* Electric Resistant */
			1,	/* Perfect Armour */
			0,	/* Electric Sensitive */
			0,	/* Combustability */
		},
	},
	{0,0,0,},
	section_is_master_root
};

#endif




BOOL AreDamageProfilesEqual(DAMAGE_PROFILE* profile1,DAMAGE_PROFILE* profile2)
{
	if(!profile1) return FALSE;
	if(!profile2) return FALSE;

	if(profile1->Impact==profile2->Impact &&	
	   profile1->Cutting==profile2->Cutting &&
	   profile1->Penetrative==profile2->Penetrative &&
	   profile1->Fire==profile2->Fire &&
	   profile1->Electrical==profile2->Electrical &&
	   profile1->Acid==profile2->Acid &&
	   profile1->ExplosivePower==profile2->ExplosivePower &&
	   profile1->Slicing==profile2->Slicing &&
	   profile1->ProduceBlood==profile2->ProduceBlood &&
	   profile1->ForceBoom==profile2->ForceBoom &&
	   profile1->BlowUpSections==profile2->BlowUpSections &&
	   profile1->Special==profile2->Special &&
	   profile1->MakeExitWounds==profile2->MakeExitWounds &&
	   profile1->Id==profile2->Id)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
