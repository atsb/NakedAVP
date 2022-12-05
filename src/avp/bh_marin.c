/*------------------------Patrick 2/7/97-----------------------------
  Source file for Marine and Seal AI behaviour functions....
  --------------------------------------------------------------------*/
#include "3dc.h"

#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "lighting.h"

#include "pfarlocs.h"
#include "pvisible.h"
#include "pheromon.h"
#include "bh_gener.h"
#include "bh_far.h"
#include "bh_pred.h"
#include "bh_marin.h"
#include "bh_weap.h"
#include "bh_debri.h"
#include "bh_alien.h"
#include "psnd.h"
#include "weapons.h"
#include "load_shp.h"
#include "particle.h"
#include "sfx.h"
#include "huddefs.h"
#include "pldghost.h"
#include "pldnet.h"
#include "psndplat.h"
#include "ai_sight.h"
#include "los.h"
#include "bh_corpse.h"
#include "bh_dummy.h"
#include "scream.h"
#include "targeting.h"

#include "dxlog.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "sequnces.h"
#include "showcmds.h"
#include "extents.h"
#include "avp_userprofile.h"
#include "hud.h"

#define ALL_PULSERIFLES 0
#define MOTIONTRACKERS 0
#define ANARCHY 0
#define PISTOL_CLIP_SIZE 8
#define SENTRY_SENSITIVITY 1500
#define MARINE_AUTODETECT_RANGE	2500

#define SUSPECT_SENSITIVITY 2100
/* Ten centimetres.  It can make a lot of difference. */

#define ALL_NEW_AVOIDANCE	(1)
#define TWO_PISTOL_GUY		(1)

/* external global variables used in this file */
extern int ModuleArraySize;
extern char *ModuleCurrVisArray;
extern int NormalFrameTime;
extern unsigned char Null_Name[8];
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern int CurrentLightAtPlayer;

extern int AIModuleArraySize;
extern AIMODULE *AIModuleArray;
extern DAMAGE_PROFILE FallingDamage;

extern enum PARTICLE_ID GetBloodType(STRATEGYBLOCK *sbPtr);
extern HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeSetFromLibrary(const char* rif_name,const char* shape_set_name);
extern HIERARCHY_VARIANT_DATA* GetHierarchyAlternateShapeSetCollectionFromLibrary(const char* rif_name,int collection_index);
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern STRATEGYBLOCK* CreateRocketKernel(VECTORCH *position, MATRIXCH *orient,int fromplayer);
extern STRATEGYBLOCK* CreateFrisbeeKernel(VECTORCH *position, MATRIXCH *orient, int fromplayer);
extern int AlienPCIsCurrentlyVisible(int checktime,STRATEGYBLOCK *sbPtr);
extern int SBIsEnvironment(STRATEGYBLOCK *sbPtr);
void Marine_SwitchExpression(STRATEGYBLOCK *sbPtr,int state);

extern void PrintSpottedNumber(void);

/* prototypes for this file */
static STATE_RETURN_CONDITION Execute_MFS_Wait(STRATEGYBLOCK *sbPtr);
#if 0
static STATE_RETURN_CONDITION Execute_MFS_Hunt(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeGL(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Hunt(STRATEGYBLOCK *sbPtr);
#endif
static STATE_RETURN_CONDITION Execute_MFS_Wander(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Approach(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Firing(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Return(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Pathfinder(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Avoidance(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_SentryMode(STRATEGYBLOCK *sbPtr);

static STATE_RETURN_CONDITION Execute_MNS_Approach(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Avoidance(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Wander(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Wait(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeLOSWeapon(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeShotgun(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargePistol(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeSADAR(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeFlamethrower(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_ThrowMolotov(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeMinigun(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_SentryMode(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Respond(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Retreat(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Return(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Pathfinder(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Taunting(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_Reloading(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_GetWeapon(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_NewDischargeGL(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeSmartgun(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicReloading(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeTwoPistols(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_DischargeSkeeter(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_AcidAvoidance(STRATEGYBLOCK *sbPtr);

static void MarineMisfireFlameThrower(SECTION_DATA *muzzle, int *timer);
static STATE_RETURN_CONDITION Execute_MNS_NullPanicFire(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireLOSWeapon(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireFlamethrower(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireGL(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireMinigun(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFirePistol(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireShotgun(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PanicFireUnarmed(STRATEGYBLOCK *sbPtr);

void NPC_Maintain_Minigun(STRATEGYBLOCK *sbPtr, DELTA_CONTROLLER *mgd);
void Marine_AssumePanicExpression(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Respond(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MNS_PumpAction(STRATEGYBLOCK *sbPtr);
static STATE_RETURN_CONDITION Execute_MFS_Retreat(STRATEGYBLOCK *sbPtr);

static void Execute_Dying(STRATEGYBLOCK *sbPtr); /* used for near and far */

static void ProcessFarMarineTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule);
static void LobAGrenade(STRATEGYBLOCK *sbPtr);
static void CreateMarineGunFlash(STRATEGYBLOCK *sbPtr);

static void MarineFireFlameThrower(STRATEGYBLOCK *sbPtr);

static void	Marine_ConsiderFallingDamage(STRATEGYBLOCK *sbPtr);
static void	Marine_MirrorSuspectPoint(STRATEGYBLOCK *sbPtr);
static int MarineCanSeeTarget(STRATEGYBLOCK *sbPtr);
static int MarineCanSeeObject(STRATEGYBLOCK *sbPtr,STRATEGYBLOCK *target);
static int MarineIsAwareOfTarget(STRATEGYBLOCK *sbPtr);
static int MarineShouldBeCrawling(STRATEGYBLOCK *sbPtr);
static int MarineRetreatsInTheFaceOfDanger(STRATEGYBLOCK *sbPtr);
static void SetMarineAnimationSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening);
static void SetMarineAnimationSequence_Null(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening);
void Marine_Enter_Respond_State(STRATEGYBLOCK *sbPtr);

int SpeedRangeMods(VECTORCH *range,VECTORCH *speed);
STRATEGYBLOCK *Marine_GetNewTarget(VECTORCH *marinepos, STRATEGYBLOCK *me);
void FakeTrackerWheepGenerator(VECTORCH *marinepos, STRATEGYBLOCK *me);

void InitMission(STRATEGYBLOCK *sbPtr,MARINE_MISSION mission);

void WanderMission_Control(STRATEGYBLOCK *sbPtr);
void PathfinderMission_Control(STRATEGYBLOCK *sbPtr);
void GuardMission_Control(STRATEGYBLOCK *sbPtr);
void LocalGuardMission_Control(STRATEGYBLOCK *sbPtr);
void LoiterMission_Control(STRATEGYBLOCK *sbPtr);
void RunAroundOnFireMission_Control(STRATEGYBLOCK *sbPtr);

void WanderMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result);
void PathfinderMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result);
void GuardMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result);
void LocalGuardMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result);
void LoiterMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result);

void Marine_Enter_SentryMode_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Wait_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Firing_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Avoidance_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Wander_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Approach_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Hunt_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Retreat_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Return_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Pathfinder_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Taunt_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_PanicFire_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_Reload_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_PumpAction_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_PullPistol_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_OneArmShotgun_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_OneArmPistol_State(STRATEGYBLOCK *sbPtr);
void Marine_Enter_PanicReload_State(STRATEGYBLOCK *sbPtr);

void Marine_Activate_AcidAvoidance_State(STRATEGYBLOCK *sbPtr, VECTORCH *incidence);

void Marine_QueueNeutralExpression(STRATEGYBLOCK *sbPtr);
void Marine_QueueGrimaceExpression(STRATEGYBLOCK *sbPtr);
void Marine_QueuePanicExpression(STRATEGYBLOCK *sbPtr);
void Marine_QueueWink1Expression(STRATEGYBLOCK *sbPtr);
void Marine_QueueWink2Expression(STRATEGYBLOCK *sbPtr);
int Marine_HasHisMouthOpen(STRATEGYBLOCK *sbPtr);
void Marine_UpdateFace(STRATEGYBLOCK *sbPtr);

void Marine_MuteVoice(STRATEGYBLOCK *sbPtr);
void Marine_WoundedScream(STRATEGYBLOCK *sbPtr);
void Marine_AcidScream(STRATEGYBLOCK *sbPtr);
void Marine_BurningScream(STRATEGYBLOCK *sbPtr);
void Marine_DeathScream(STRATEGYBLOCK *sbPtr);
void Marine_ElectrocutionScream(STRATEGYBLOCK *sbPtr);
void Marine_BurningDeathScream(STRATEGYBLOCK *sbPtr);
void Marine_AngryScream(STRATEGYBLOCK *sbPtr);
void Marine_PanicScream(STRATEGYBLOCK *sbPtr);
void Marine_OoophSound(STRATEGYBLOCK *sbPtr);
void Marine_SurpriseSound(STRATEGYBLOCK *sbPtr);
void Marine_Sobbing(STRATEGYBLOCK *sbPtr);
void Marine_TauntShout(STRATEGYBLOCK *sbPtr);

void NPC_GetBimbleTarget(STRATEGYBLOCK *sbPtr,VECTORCH *output);
void CreateMarineBot(VECTORCH *Position,int weapon);
void Convert_To_RunningOnFire(STRATEGYBLOCK *sbPtr);
void GetPointToFaceMarineTowards(STRATEGYBLOCK *sbPtr,VECTORCH *output);
void DoMarineHearing(STRATEGYBLOCK *sbPtr);

static void HandleWaitingAnimations(STRATEGYBLOCK *sbPtr);
static void HandleMovingAnimations(STRATEGYBLOCK *sbPtr);
int MakeModifiedTargetNum(int targetnum,int dist);


/* Console Variables */

int Marine_Skill=20000;
int Marine_Terminal_Velocity=20000;

/* Console Variables */

MARINE_WEAPON_DATA NPC_Marine_Weapons[] = {
	{
		MNPCW_PulseRifle,					/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeLOSWeapon,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireLOSWeapon,		/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"marine with pulse rifle", 			/* HierarchyName */
		"dum flash",						/* GunflashName */
		"pulse rifle",						/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"pulse mag",						/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1000*65536/60,						/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		10,									/* MinimumBurstSize */
		AMMO_10MM_CULW_NPC,						/* Ammo profile */
		99,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		300,								/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_PULSE_LOOP,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		1,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Flamethrower,					/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */
		Execute_MNS_DischargeFlamethrower,	/* Func. */
		MarineMisfireFlameThrower,			/* Misfire func. */
		Execute_MNS_PanicFireFlamethrower,	/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"marine with flame thrower",		/* HierarchyName */
		"dum flash",						/* GunflashName */
		"flamer",							/* ElevationSection */
		"marine with flame thrower",		/* HitLocationTableName */
		"Template",							/* TemplateName */
		"flamer mag",						/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		5000,								/* ForceFireRange (Fire if closer) */
		10000,								/* MaxRange (Don't fire if further) */
		ONE_FIXED,							/* Accuracy */
		200,								/* Firing Rate */
		MARINE_NEAR_FIRE_TIME<<2,			/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_FLAMETHROWER,					/* Ammo profile */
		(ONE_FIXED*3),						/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_INCIN_START,					/* StartSound */
		SID_INCIN_LOOP,						/* LoopSound */
		SID_INCIN_END,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Smartgun,						/* ID */
		SFX_MUZZLE_FLASH_SMARTGUN,			/* enum SFX_ID SfxID; */
		Execute_MNS_DischargeSmartgun,		/* Func.  Changed from DischargeLOSWeapon! */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireLOSWeapon,		/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"marine with smart gun",			/* HierarchyName */
		"dum flash",						/* GunflashName */
		"smart gun",						/* ElevationSection */
		"marine with smart gun", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"smart mag",						/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		36000,								/* MaxRange (Don't fire if further) */
		ONE_FIXED,							/* Accuracy */
		50*65536,							/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		50,									/* MinimumBurstSize */
		AMMO_SMARTGUN_NPC,						/* Ammo profile */
		300,								/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_SMART1, 						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_SADAR,						/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeSADAR,			/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_NullPanicFire,			/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"marine with SADAR",				/* HierarchyName */
		"dum flame",						/* GunflashName */
		"SADAR Tube",						/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"SADAR",							/* ClipName */
		5000,								/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1000*65536/60,						/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		1,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		300,								/* TargetCallibrationShift */
		SID_ROCKFIRE,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		1,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_GrenadeLauncher,				/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_NewDischargeGL,			/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireGL,			/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"marine + grenade launcher",		/* HierarchyName */
		"dum flash",						/* GunflashName */
		"gren stock",						/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"gren mag",							/* ClipName */
		9000,								/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1000*65536/60,						/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		6,									/* clip_size */
		MSSS_Reload,  						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_ROCKFIRE,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		1,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Minigun,						/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeMinigun,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireMinigun,		/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"Marine with Mini Gun", 			/* HierarchyName */
		"dum flash",						/* GunflashName */
		"mini gun",							/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"mini mag",							/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		100*65536,							/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		MINIGUN_MINIMUM_BURST,				/* MinimumBurstSize */
		AMMO_MINIGUN_NPC,						/* Ammo profile */
		500,								/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_MINIGUN_LOOP,					/* LoopSound */
		SID_MINIGUN_END, 					/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_PistolMarine,					/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargePistol,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFirePistol,		/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"PISTOL",			 				/* HierarchyName */
		"dum flash",						/* GunflashName */
		"Rbicep",							/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		30000,	  							/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_MARINE_PISTOL,					/* Ammo profile */
		12,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		250,								/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_MShotgun,						/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeShotgun,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireShotgun,		/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"male_shotgun",		 				/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"shot gun",							/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"TEMPLATE",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1,									/* Firing Rate */
		-1,									/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_SHOTGUN,						/* Ammo profile */
		8,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_MPistol,						/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargePistol,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFirePistol,		/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"male_pistol",		 				/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"male right bicep",					/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"TEMPLATE",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		10000,	  							/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW_NPC,						/* Ammo profile */
		PISTOL_CLIP_SIZE,					/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_MFlamer,						/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */
		Execute_MNS_DischargeFlamethrower,	/* Func. */
		MarineMisfireFlameThrower,			/* Misfire func. */
		Execute_MNS_PanicFireFlamethrower,	/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"male_flamer",		 				/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"male right bicep",					/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"TEMPLATE",							/* TemplateName */
		"flame canaster",					/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		5000,								/* ForceFireRange (Fire if closer) */
		10000,								/* MaxRange (Don't fire if further) */
		ONE_FIXED,							/* Accuracy */
		200,								/* Firing Rate */
		MARINE_NEAR_FIRE_TIME<<2,			/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_FLAMETHROWER,					/* Ammo profile */
		(ONE_FIXED*2),						/* clip_size */
		MSSS_Reload, 						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_INCIN_START,					/* StartSound */
		SID_INCIN_LOOP,						/* LoopSound */
		SID_INCIN_END,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_MUnarmed,						/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */

		NULL,								/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireUnarmed,		/* WeaponPanicFireFunction */
		"hnpc_civvie", 						/* Riffname */
		"male_unarmed",			 			/* HierarchyName */
		NULL,								/* GunflashName */
		NULL,								/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"TEMPLATE",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_NONE,							/* Ammo profile */
		-1,									/* clip_size */
		MSSS_Standard,							/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		0,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_MMolotov,						/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */

		Execute_MNS_ThrowMolotov,			/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_NullPanicFire,			/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"male_bottle", 		 				/* HierarchyName */
		"bottle",							/* GunflashName */
		NULL,								/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"TEMPLATE",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		10000,								/* MaxRange (Don't fire if further) */
		-10000,								/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		-1,									/* clip_size */
		MSSS_Standard,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		0,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Android,						/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeShotgun,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireShotgun,		/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"Android shotgun",	 				/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"shot gun",							/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"Android template",					/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1,									/* Firing Rate */
		-1,									/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_SHOTGUN,						/* Ammo profile */
		8,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		1,									/* Android */
	},
	{
		MNPCW_AndroidSpecial,				/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeShotgun,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFireShotgun,		/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"Android Shotgun Special",			/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"male right bicep",					/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"Android template",					/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		1,									/* Firing Rate */
		-1,									/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_SHOTGUN,						/* Ammo profile */
		8,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		1,									/* Android */
	},
	{
		MNPCW_Android_Pistol_Special,		/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargePistol,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFirePistol,		/* WeaponPanicFireFunction */
		"hnpc_civvie",						/* Riffname */
		"Android Pistol Special",			/* HierarchyName */
		"flash dummy",						/* GunflashName */
		"male right bicep",					/* ElevationSection */
		"male civvie",		 				/* HitLocationTableName */
		"Android template",					/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		10000,	  							/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW_NPC,						/* Ammo profile */
		PISTOL_CLIP_SIZE,					/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		1,									/* Android */
	},
	{
		MNPCW_TwoPistols,					/* ID */
		SFX_MUZZLE_FLASH_AMORPHOUS,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeTwoPistols,	/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_PanicFirePistol,		/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"Two Pistol",		 				/* HierarchyName */
		"dum flash",						/* GunflashName */
		"Rbicep",							/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		30000,	  							/* Accuracy */
		12,									/* Firing Rate */
		(MARINE_NEAR_FIRE_TIME>>2),			/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_MARINE_PISTOL,					/* Ammo profile */
		24,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		250,								/* TargetCallibrationShift */
		SID_SHOTGUN,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		1,									/* UseElevation */
		0,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Skeeter,						/* ID */
		SFX_MUZZLE_FLASH_SKEETER,			/* enum SFX_ID SfxID; */

		Execute_MNS_DischargeSkeeter,		/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_NullPanicFire,			/* WeaponPanicFireFunction */
		"hnpcmarine",						/* Riffname */
		"skeeter",							/* HierarchyName */
		"dum flash",						/* GunflashName */
		"Skeeter Tube",						/* ElevationSection */
		"marine with pulse rifle", 			/* HitLocationTableName */
		"Template",							/* TemplateName */
		"Skeeter",							/* ClipName */
		5000,								/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		-1,									/* MaxRange (Don't fire if further) */
		0,									/* Accuracy */
		ONE_FIXED,							/* Firing Rate */
		1625*65536/2000,					/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		1,									/* clip_size */
		MSSS_Reload,						/* Reload_Sequence */
		300,								/* TargetCallibrationShift */
		SID_ED_SKEETERCHARGE,				/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_ED_SKEETERLAUNCH,				/* EndSound */
		1,									/* Enable Grenades */
		1,									/* UseElevation */
		1,									/* EnableTracker */
		1,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Scientist_A,					/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */

		NULL,								/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_NullPanicFire,			/* WeaponPanicFireFunction */
		"scientist",						/* Riffname */
		"clip",		 		 				/* HierarchyName */
		"clip board",						/* GunflashName */
		NULL,								/* ElevationSection */
		"bub with molotov", 				/* HitLocationTableName */
		"Template",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		10000,								/* MaxRange (Don't fire if further) */
		-10000,								/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		-1,									/* clip_size */
		MSSS_Standard,							/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		0,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_Scientist_B,					/* ID */
		SFX_NONE,							/* enum SFX_ID SfxID; */

		NULL,								/* Func. */
		NULL,								/* Misfire func. */
		Execute_MNS_NullPanicFire,			/* WeaponPanicFireFunction */
		"scientist",						/* Riffname */
		"testtube",	  		 				/* HierarchyName */
		"test tube",						/* GunflashName */
		NULL,								/* ElevationSection */
		"bub with molotov", 				/* HitLocationTableName */
		"Template",							/* TemplateName */
		NULL,								/* ClipName */
		0,									/* MinRange (Don't fire when closer) */
		MARINE_CLOSE_APPROACH_DISTANCE,		/* ForceFireRange (Fire if closer) */
		10000,								/* MaxRange (Don't fire if further) */
		-10000,								/* Accuracy */
		1,									/* Firing Rate */
		MARINE_NEAR_FIRE_TIME,				/* Firing Time */
		0,									/* MinimumBurstSize */
		AMMO_10MM_CULW,						/* Ammo profile */
		-1,									/* clip_size */
		MSSS_Standard,							/* Reload_Sequence */
		0,									/* TargetCallibrationShift */
		SID_NOSOUND,						/* StartSound */
		SID_NOSOUND,						/* LoopSound */
		SID_NOSOUND,						/* EndSound */
		0,									/* Enable Grenades */
		0,									/* UseElevation */
		0,									/* EnableTracker */
		0,									/* ARealMarine */
		0,									/* Android */
	},
	{
		MNPCW_End,
		SFX_NONE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		MAX_NO_OF_AMMO_TEMPLATES,
		-1,
		MSSS_Standard,
		0,
		SID_NOSOUND,
		SID_NOSOUND,
		SID_NOSOUND,
		0,
		0,
		0,
		0,
		0,
	},
};

VECTORCH ShotgunBlast[] = {
	{0,0,400,},
	{100,0,400,},
	{-100,0,400,},
	{50,0,400,},
	{-50,0,400,},
	{-1,-1,-1,},
};

SQUAD_COMMAND_STATE NpcSquad;
int ShowSquadState=0;
int ShowNearSquad=0;
extern SCENE Global_Scene;
static MODULE **Global_ModuleArrayPtr;
extern int ModuleArraySize;
static char tempstring[256];
static int tracker_noise;

void ForceCaps(char *input) {
	char *p;
	p=input;
	while (*p!=0) {
		if ( (*p>=97) && (*p<=122) ) {
			/* Force Caps */
			*p-=32;
		}
		p++;
	}
}

/* Squad functions, CDF 27/5/98 */

void InitSquad(void) {

	/* Maybe level specific later? */
	
	NpcSquad.alertStatus=0;
	NpcSquad.responseLevel=0;
	NpcSquad.alertZone=NULL;
	NpcSquad.alertPriority=0;

	NpcSquad.Squad_Suspicion=0;
	NpcSquad.squad_suspect_point.vx=0;
	NpcSquad.squad_suspect_point.vy=0;
	NpcSquad.squad_suspect_point.vz=0;
	
	NpcSquad.RespondingMarines=0;
	NpcSquad.Alt_RespondingMarines=0;

	NpcSquad.NearUnpanickedMarines=0;
	NpcSquad.Alt_NearUnpanickedMarines=0;

	NpcSquad.NearPanickedMarines=0;
	NpcSquad.Alt_NearPanickedMarines=0;

	NpcSquad.NearBurningMarines=0;
	NpcSquad.Alt_NearBurningMarines=0;

	NpcSquad.Squad_Delta_Morale=0;
	NpcSquad.Nextframe_Squad_Delta_Morale=0;
}

void DoSquad(void) {

	/* Maintain squad level stuff. */

	if (NpcSquad.alertZone!=NULL) {
		MaintainMarineTargetZone(NpcSquad.alertZone);
	}
	
	/* Maintain squad suspicion. */
	if (NpcSquad.Squad_Suspicion>0) {
		NpcSquad.Squad_Suspicion-=NormalFrameTime;
		if (NpcSquad.Squad_Suspicion<0) {
			NpcSquad.Squad_Suspicion=0;
		}
	}

	/* Maintain stats. */
	NpcSquad.RespondingMarines=NpcSquad.Alt_RespondingMarines;
	NpcSquad.Alt_RespondingMarines=0;

	NpcSquad.NearUnpanickedMarines=NpcSquad.Alt_NearUnpanickedMarines;
	NpcSquad.Alt_NearUnpanickedMarines=0;

	NpcSquad.NearPanickedMarines=NpcSquad.Alt_NearPanickedMarines;
	NpcSquad.Alt_NearPanickedMarines=0;

	NpcSquad.NearBurningMarines=NpcSquad.Alt_NearBurningMarines;
	NpcSquad.Alt_NearBurningMarines=0;

	NpcSquad.Squad_Delta_Morale=NpcSquad.Nextframe_Squad_Delta_Morale;
	NpcSquad.Nextframe_Squad_Delta_Morale=0;

	/* Update morale. */
	NpcSquad.Nextframe_Squad_Delta_Morale+=MUL_FIXED(NormalFrameTime,(NpcSquad.NearUnpanickedMarines*50));
	NpcSquad.Nextframe_Squad_Delta_Morale-=MUL_FIXED(NormalFrameTime,(NpcSquad.NearPanickedMarines*1000));
	NpcSquad.Nextframe_Squad_Delta_Morale-=MUL_FIXED(NormalFrameTime,(NpcSquad.NearBurningMarines*3000));

	if (TERROR_MODE) {
		NpcSquad.Nextframe_Squad_Delta_Morale=-100000000;
	}
	
	/* Status display. */
	if (ShowSquadState) {
		PrintDebuggingText("Marine Alert Status = %d\n",NpcSquad.alertStatus);
		PrintDebuggingText("Marine Alert Priority = %d\n",NpcSquad.alertPriority);
		PrintDebuggingText("Responding Marines = %d\n",NpcSquad.RespondingMarines);
		PrintDebuggingText("NearPanicked Marines = %d\n",NpcSquad.NearPanickedMarines);
		PrintDebuggingText("NearUnpanicked Marines = %d\n",NpcSquad.NearUnpanickedMarines);
		PrintDebuggingText("NearBurning Marines = %d\n",NpcSquad.NearBurningMarines);
		PrintDebuggingText("Marine Outstanding Response Level = %d\n",NpcSquad.responseLevel);
		if (NpcSquad.alertZone==NULL) {
			PrintDebuggingText("Marine Alert Zone = NULL\n");
		} else {
			MODULE *sampleModule;

			sampleModule=*(NpcSquad.alertZone->m_module_ptrs);
			if (sampleModule==NULL) {
				PrintDebuggingText("Marine Alert Zone = Totally Farped! %d\n",NpcSquad.alertZone->m_index);
			} else {
				PrintDebuggingText("Marine Alert Zone = %d, '%s'\n",sampleModule->m_index,sampleModule->name);
			}
		}
		PrintDebuggingText("Squad Suspicion = %d\n",NpcSquad.Squad_Suspicion);
		PrintDebuggingText("Squad Suspect Point = %d %d %d\n",NpcSquad.squad_suspect_point.vx,
			NpcSquad.squad_suspect_point.vy,NpcSquad.squad_suspect_point.vz);
	}

	/* And now just for me... :-) */
	PrintSpottedNumber();

}

void ZoneAlert(int level,AIMODULE *targetModule) {

	int idealResponse;	
	/* Bad stuff is going down. */

	/* Switch to this one if it has a higher level than the current priority. */

	if (level<NpcSquad.alertPriority) {
		/* Don't bother me with trifles! */
		return;
	}

	if (level>=NpcSquad.alertStatus) {
		NpcSquad.alertStatus=level;
	}

	NpcSquad.alertPriority=level;
	NpcSquad.alertZone=targetModule;
	switch (NpcSquad.alertStatus) {
		case 0:
			/* Can this ever happen? */
			idealResponse=1;
			break;
		case 1:
			idealResponse=1;
			break;
		case 2:
			idealResponse=3;
			break;
		case 3:
			idealResponse=5;
			break;
		default:
			idealResponse=1;
			break;
	}

	if (NpcSquad.RespondingMarines<idealResponse) {
		NpcSquad.responseLevel=(idealResponse-NpcSquad.RespondingMarines);
	}

}

void PointAlert(int level, VECTORCH *point) {
	
	MODULE *alertModule;

	alertModule=ModuleFromPosition(point,playerPherModule);

	if (NpcSquad.Squad_Suspicion!=SQUAD_PARANOIA_TIME) {
		NpcSquad.Squad_Suspicion=SQUAD_PARANOIA_TIME;
		NpcSquad.squad_suspect_point=*point;
	}

	if (alertModule==NULL) {
		return;
	}

	ZoneAlert(level,alertModule->m_aimodule);

}

void DeprioritiseAlert(AIMODULE *aimodule) {
	
	/* Parameterised, to make sure we're doing it right. */

	if (aimodule==NpcSquad.alertZone) {
		NpcSquad.alertPriority=0;
	}
}

void Console_ZoneAlert(int input) {

	MODULE *target;
	SCENEMODULE *smptr;

	smptr = Global_ModulePtr[Global_Scene];
	Global_ModuleArrayPtr = smptr->sm_marray;
	
	if ((input==0)||(input>=ModuleArraySize)) {
		target=playerPherModule;
	} else {
		target=Global_ModuleArrayPtr[input];	
	}

	sprintf(tempstring,"NEW ZONE ALERT IN %d, '%s'\n",target->m_index,target->name);
	ForceCaps(tempstring);
	NewOnScreenMessage(tempstring);

	ZoneAlert(3,target->m_aimodule);
}

/* Interface function - 15/12/97 */

MARINE_WEAPON_DATA *GetThisNPCMarineWeapon(MARINE_NPC_WEAPONS this_id) {

	int a;

	a=0;
	while (NPC_Marine_Weapons[a].id!=MNPCW_End) {
		if (NPC_Marine_Weapons[a].id==this_id) {
			return(&NPC_Marine_Weapons[a]);
		}
		a++;
	}

	return(NULL);

}

MARINE_NPC_WEAPONS GetThisManAWeapon(void) {

	int a;
	MARINE_NPC_WEAPONS thisweap;

	a=FastRandom()&65535;

	if (a<(ONE_FIXED>>3)) {
		/* 1/8: Smartgun. */
		thisweap=(MNPCW_Smartgun);
	} else if (a<(ONE_FIXED/3)) {
		/* 5/24: Flamethrower. */
		thisweap=(MNPCW_Flamethrower);
	} else if (a<((2*ONE_FIXED)/3)) {
		thisweap=(MNPCW_PulseRifle);
		//thisweap=(MNPCW_SADAR);
	} else {
		thisweap=(MNPCW_MShotgun);
	}

	/* Check... */
	{
		MARINE_WEAPON_DATA *tempweap;
		SECTION *root_section;
		
		tempweap=GetThisNPCMarineWeapon(thisweap);
		root_section=GetNamedHierarchyFromLibrary(tempweap->Riffname,tempweap->HierarchyName);
		if (!root_section) {
			thisweap=MNPCW_PulseRifle;
		}
	}

	return(thisweap);
}

void ChangeToAlternateAccoutrementSet(STRATEGYBLOCK *sbPtr, int index) {

	HIERARCHY_VARIANT_DATA* variant_data;
	HIERARCHY_SHAPE_REPLACEMENT* replacement_array;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	int a;
	    
	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		
	
	variant_data=GetHierarchyAlternateShapeSetCollectionFromLibrary(marineStatusPointer->My_Weapon->Riffname,index);

	if (variant_data==NULL) {
		return;
	}
	
	marineStatusPointer->Female=variant_data->female;
	marineStatusPointer->Voice=variant_data->voice;

	replacement_array=(HIERARCHY_SHAPE_REPLACEMENT*)variant_data->replacements;

	if (replacement_array==NULL) {
		return;
	}
	
	
	a=0;

	while (replacement_array[a].replaced_section_name!=NULL) {
		SECTION_DATA *target_section;

		target_section=GetThisSectionData(marineStatusPointer->HModelController.section_data,
			replacement_array[a].replaced_section_name);
		if (target_section) {
			target_section->Shape=replacement_array[a].replacement_shape;
			#if 1
			target_section->ShapeNum=replacement_array[a].replacement_shape_index;
			#endif
			target_section->replacement_id = replacement_array[a].replacement_id;
			
			Setup_Texture_Animation_For_Section(target_section);
			
		}
		a++;
	}

}

void ChangeToAlternateShapeSet(STRATEGYBLOCK *sbPtr, char *setname) {

	HIERARCHY_SHAPE_REPLACEMENT* replacement_array;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	int a;
	    
	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		
	
	replacement_array=GetHierarchyAlternateShapeSetFromLibrary(marineStatusPointer->My_Weapon->Riffname,setname);

	if (replacement_array==NULL) {
		return;
	}

	a=0;

	while (replacement_array[a].replaced_section_name!=NULL) {
		SECTION_DATA *target_section;

		target_section=GetThisSectionData(marineStatusPointer->HModelController.section_data,
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

/* CDF 3/4/98 */

void CastMarineBot(int weapon) {

	#define BOTRANGE 2000

	VECTORCH position;

	if (AvP.Network!=I_No_Network) {
		NewOnScreenMessage("NO MARINEBOTS IN MULTIPLAYER MODE");
		return;
	}

	position=Player->ObStrategyBlock->DynPtr->Position;
	position.vx+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat31,BOTRANGE);		
	position.vy+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat32,BOTRANGE);		
	position.vz+=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat33,BOTRANGE);		

	CreateMarineBot(&position, weapon);

}

void CreateMarineBot(VECTORCH *Position, int weapon)
{
	STRATEGYBLOCK* sbPtr;

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr) {
		NewOnScreenMessage("FAILED TO CREATE BOT: SB CREATION FAILURE");
		return; /* failure */
	}
	InitialiseSBValues(sbPtr);

	sbPtr->I_SBtype = I_BehaviourMarine;

	AssignNewSBName(sbPtr);
			
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = *Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}
	else
	{
		/* dynamics block allocation failed... */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE BOT: DYNBLOCK CREATION FAILURE");
		return;
	}

	sbPtr->shapeIndex = 0;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);

	/* create, initialise and attach a marine data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(MARINE_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		MARINE_STATUS_BLOCK *marineStatus = (MARINE_STATUS_BLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(marineStatus);

		NPC_InitMovementData(&(marineStatus->moveData));
		NPC_InitWanderData(&(marineStatus->wanderData));
     	marineStatus->health = MARINE_STARTING_HEALTH;
   		sbPtr->integrity = marineStatus->health;
		marineStatus->volleySize = 0;
		
		marineStatus->primaryWeaponDamage = MARINE_WEAPON_DAMAGE;
		marineStatus->stateTimer = MARINE_FAR_MOVE_TIME;
		marineStatus->weaponTarget.vx = marineStatus->weaponTarget.vy = marineStatus->weaponTarget.vz = 0;
		marineStatus->myGunFlash = (DISPLAYBLOCK *)0;  							
		marineStatus->soundHandle = SOUND_NOACTIVEINDEX;
		marineStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		marineStatus->obstruction.environment=0;
		marineStatus->obstruction.destructableObject=0;
		marineStatus->obstruction.otherCharacter=0;
		marineStatus->obstruction.anySingleObstruction=0;

		Initialise_AvoidanceManager(sbPtr,&marineStatus->avoidanceManager);
		InitWaypointManager(&marineStatus->waypointManager);

		marineStatus->IAmCrouched = 0;
		marineStatus->lastroundhit=0;
		marineStatus->lasthitsection=NULL;

		marineStatus->weapon_variable=0;
		marineStatus->weapon_variable2=0;

		marineStatus->Skill=Marine_Skill;
		marineStatus->Courage=ONE_FIXED;
			
		marineStatus->FiringAnim=0;
		marineStatus->SpotFlag=0;

		//a generated marine won't have a death target
		{
			int i;
			for(i=0;i<SB_NAME_LENGTH;i++) marineStatus->death_target_ID[i] =0; 
			marineStatus->death_target_request=0;
			marineStatus->death_target_sbptr=0;
		}
		
		//this marine wasn't produced by a generator
		marineStatus->generator_sbptr=0;


		marineStatus->Target=NULL; //Player->ObStrategyBlock;
		if ( (weapon<=0)||(weapon>(int)MNPCW_End)) {
			marineStatus->My_Weapon=GetThisNPCMarineWeapon(GetThisManAWeapon());
		} else {
			#if (TWO_PISTOL_GUY==0)
			if (weapon>=MNPCW_TwoPistols) {
				marineStatus->My_Weapon=GetThisNPCMarineWeapon(GetThisManAWeapon());
			} else {
				marineStatus->My_Weapon=GetThisNPCMarineWeapon(weapon-1);
			}
			#else
			marineStatus->My_Weapon=GetThisNPCMarineWeapon(weapon-1);
			#endif
		}
	
		/* Initialise marine's stats */
		{
			NPC_DATA *NpcData;
			
			if (marineStatus->My_Weapon->Android) {
				NpcData=GetThisNpcData(I_NPC_Android);
			} else if (marineStatus->My_Weapon->ARealMarine) {
				NpcData=GetThisNpcData(I_NPC_Marine);
			} else {
				NpcData=GetThisNpcData(I_NPC_Civilian);
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		
		{
			const MOVEMENT_DATA *movementData;

			marineStatus->speedConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			marineStatus->accelerationConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			
			if (marineStatus->My_Weapon->ARealMarine) {	
				movementData=GetThisMovementData(MDI_Marine_Combat);
			} else {
				movementData=GetThisMovementData(MDI_Civilian_Combat);
			}
			GLOBALASSERT(movementData);
			marineStatus->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatus->speedConstant);
			marineStatus->acceleration = MUL_FIXED(movementData->acceleration,marineStatus->accelerationConstant);
		}
		COPY_NAME(marineStatus->Target_SBname,Null_Name);
		marineStatus->lastmodule=NULL;
		marineStatus->destinationmodule=NULL;
		marineStatus->missionmodule=NULL;
		marineStatus->fearmodule=NULL;
		marineStatus->my_facing_point=sbPtr->DynPtr->Position;
		marineStatus->path=-1;
		marineStatus->stepnumber=-1;
		marineStatus->sawlastframe=0;
		marineStatus->gotapoint=0;
		marineStatus->lastframe_fallingspeed=0;
		marineStatus->suspicious=0;
		marineStatus->previous_suspicion=0;
		marineStatus->using_squad_suspicion=0;
		marineStatus->suspect_point.vx=0;
		marineStatus->suspect_point.vy=0;
		marineStatus->suspect_point.vz=0;
		marineStatus->internalState=0;
		#if MOTIONTRACKERS
		if (marineStatus->My_Weapon->EnableTracker) {
			if ((FastRandom()&65535)<(ONE_FIXED>>2)) {
				marineStatus->mtracker_timer=FastRandom()&65535;
			} else {
				marineStatus->mtracker_timer=-1;
			}
		} else {
			marineStatus->mtracker_timer=-1;
		}
		#else
		marineStatus->mtracker_timer=-1;
		#endif
		marineStatus->GibbFactor=0;
		marineStatus->Wounds=0;

		marineStatus->incidentFlag=0;
		marineStatus->incidentTimer=0;

		if (marineStatus->My_Weapon->id==MNPCW_MPistol) {
			/* Special case for pistols? */
			marineStatus->lastroundhit=PISTOL_CLIP_SIZE;
		}
		marineStatus->clipammo=marineStatus->My_Weapon->clip_size;
		marineStatus->roundsForThisTarget=0;

		marineStatus->HModelController.section_data=NULL;
		marineStatus->HModelController.Deltas=NULL;
		/* In case we need to deallocate it. */
		root_section=GetNamedHierarchyFromLibrary(marineStatus->My_Weapon->Riffname,marineStatus->My_Weapon->HierarchyName);
		if (!root_section) {
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE BOT: NO HMODEL");
			return;
		}
		Create_HModel(&marineStatus->HModelController,root_section);
		InitHModelSequence(&marineStatus->HModelController,(int)HMSQT_MarineRun,(int)MRSS_Standard,ONE_FIXED);
		
		if (marineStatus->My_Weapon->UseElevation) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&marineStatus->HModelController,"Elevation",(int)HMSQT_MarineStand,(int)MSSS_Elevation,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
		}

		if (marineStatus->My_Weapon->id==MNPCW_Minigun) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&marineStatus->HModelController,"Minigun",(int)HMSQT_MarineStand,(int)MSSS_Minigun_Delta,(ONE_FIXED>>3));
			GLOBALASSERT(delta);
			delta->Playing=0;
			delta->Looped=1;
		}
		
		/* Create blank hit delta sequence. */
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0)
		{
			if (HModelSequence_Exists(&marineStatus->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
				DELTA_CONTROLLER *delta;
				delta=Add_Delta_Sequence(&marineStatus->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
				GLOBALASSERT(delta);
				delta->Playing=0;
			}
		}

		marineStatus->My_Gunflash_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->GunflashName);
		marineStatus->My_Elevation_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->ElevationSection);
		
		//initialise female to 0..
		//may be changed by ChangeToAlternateAccoutrementSet
		marineStatus->Female=0;
		marineStatus->Voice=0;
		marineStatus->Android=marineStatus->My_Weapon->Android;
		
		//use accoutement set for both marine and civilian
		#if 0
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0) {
			int dice=FastRandom()&65535;
			if (dice<16384) {
				ChangeToAlternateShapeSet(sbPtr, "Face Four");
			} else if (dice<32768) {
				ChangeToAlternateShapeSet(sbPtr, "Face Three");
			}
		} else if (strcmp("hnpc_civvie",marineStatus->My_Weapon->Riffname)==0) {
			#if 0
			int dice=FastRandom()&65535;
			if (dice<32767) {
				ChangeToAlternateShapeSet(sbPtr, "MedicalGuy");
			} else if (dice<49152) {
				ChangeToAlternateShapeSet(sbPtr, "CompanyGuy");
			}
			#else
			ChangeToAlternateAccoutrementSet(sbPtr, 0);
			#endif
		}
		#else
		//pick a random texture id rather than using 0 , so that we can get 
		//texture ids not normally allowed by the level
		ChangeToAlternateAccoutrementSet(sbPtr, FastRandom());
		#endif
		marineStatus->VoicePitch = (FastRandom() & 255) - 128;

		Marine_SwitchExpression(sbPtr,0);
		marineStatus->Target_Expression=0;
		marineStatus->Blink=-1;

		ProveHModel_Far(&marineStatus->HModelController,sbPtr);

		if(!(sbPtr->containingModule))
		{
			/* no containing module can be found... abort*/
			RemoveBehaviourStrategy(sbPtr);
			NewOnScreenMessage("FAILED TO CREATE BOT: MODULE CONTAINMENT FAILURE");
			return;
		}
		LOCALASSERT(sbPtr->containingModule);

		if (marineStatus->My_Weapon->WeaponFireFunction==NULL) {
			InitMission(sbPtr,MM_NonCom);
		} else {
			InitMission(sbPtr,MM_Wander);
		}

		MakeMarineNear(sbPtr);

		NewOnScreenMessage("MARINEBOT CREATED");
	}
	else
	{
		/* no data block can be allocated */
		RemoveBehaviourStrategy(sbPtr);
		NewOnScreenMessage("FAILED TO CREATE BOT: MALLOC FAILURE");
		return;
	}
}

/*------------------------Patrick 24/2/97-----------------------------
  Marine/Seal behaviour shell functions
  --------------------------------------------------------------------*/

void InitMarineBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	TOOLS_DATA_MARINE *toolsData = (TOOLS_DATA_MARINE *)bhdata; 
	int i;

	LOCALASSERT(sbPtr);

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network) 
	{
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* make the assumption that the loader has initialised the strategy block sensibly... 
	so just set the shapeIndex from the tools data & copy the name id*/
	sbPtr->shapeIndex = toolsData->shapeIndex;
	for(i=0;i<SB_NAME_LENGTH;i++) sbPtr->SBname[i] = toolsData->nameID[i];

	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = toolsData->position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
		
		/* zero linear velocity in dynamics block */
		dynPtr->LinVelocity.vx = 0;
		dynPtr->LinVelocity.vy = 0;
		dynPtr->LinVelocity.vz = 0;

		sbPtr->containingModule=ModuleFromPosition(&dynPtr->Position,NULL);

		GLOBALASSERT(sbPtr->containingModule);
	}
	else 
	{
		/* allocation failed */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* create, initialise and attach a marine data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(MARINE_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		MARINE_STATUS_BLOCK *marineStatus = (MARINE_STATUS_BLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(marineStatus);

		NPC_InitMovementData(&(marineStatus->moveData));
		NPC_InitWanderData(&(marineStatus->wanderData));
     	marineStatus->health = MARINE_STARTING_HEALTH;
   		sbPtr->integrity = marineStatus->health;
		marineStatus->volleySize = 0;
		marineStatus->primaryWeaponDamage = MARINE_WEAPON_DAMAGE;
		marineStatus->stateTimer = MARINE_FAR_MOVE_TIME;
		marineStatus->weaponTarget.vx = marineStatus->weaponTarget.vy = marineStatus->weaponTarget.vz = 0;
		marineStatus->myGunFlash = (DISPLAYBLOCK *)0;  							
		marineStatus->soundHandle = SOUND_NOACTIVEINDEX;
		marineStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		marineStatus->obstruction.environment=0;
		marineStatus->obstruction.destructableObject=0;
		marineStatus->obstruction.otherCharacter=0;
		marineStatus->obstruction.anySingleObstruction=0;

		Initialise_AvoidanceManager(sbPtr,&marineStatus->avoidanceManager);
		InitWaypointManager(&marineStatus->waypointManager);

		marineStatus->IAmCrouched = 0;
		marineStatus->lastroundhit=0;
		marineStatus->lasthitsection=NULL;

		marineStatus->weapon_variable=0;
		marineStatus->weapon_variable2=0;

		marineStatus->Skill=Marine_Skill;
		marineStatus->Courage=ONE_FIXED;

		marineStatus->FiringAnim=0;
		marineStatus->SpotFlag=0;

		marineStatus->Target=NULL; //Player->ObStrategyBlock;

		marineStatus->my_spot=sbPtr->DynPtr->Position;
		
		#if ALL_PULSERIFLES
		marineStatus->My_Weapon=GetThisNPCMarineWeapon(MNPCW_PulseRifle);
		#else
		marineStatus->My_Weapon=GetThisNPCMarineWeapon(toolsData->marine_type);
		#endif

		/* Initialise marine's stats */
		{
			NPC_DATA *NpcData;

			if (marineStatus->My_Weapon->Android) {
				NpcData=GetThisNpcData(I_NPC_Android);
			} else if (marineStatus->My_Weapon->ARealMarine) {
				NpcData=GetThisNpcData(I_NPC_Marine);
			} else {
				NpcData=GetThisNpcData(I_NPC_Civilian);
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}

		if (toolsData->Mission==MM_NonCom) {
			marineStatus->My_Weapon=GetThisNPCMarineWeapon(MNPCW_MUnarmed);
		}

		if (marineStatus->My_Weapon->id==MNPCW_MPistol) {
			/* Special case for pistols? */
			marineStatus->lastroundhit=PISTOL_CLIP_SIZE;
		}
		marineStatus->clipammo=marineStatus->My_Weapon->clip_size;
		marineStatus->roundsForThisTarget=0;

		{
			const MOVEMENT_DATA *movementData;

			marineStatus->speedConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			marineStatus->accelerationConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			
			if (marineStatus->My_Weapon->ARealMarine) {	
				movementData=GetThisMovementData(MDI_Marine_Combat);
			} else {
				movementData=GetThisMovementData(MDI_Civilian_Combat);
			}
			GLOBALASSERT(movementData);
			marineStatus->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatus->speedConstant);
			marineStatus->acceleration = MUL_FIXED(movementData->acceleration,marineStatus->accelerationConstant);
		}
		COPY_NAME(marineStatus->Target_SBname,Null_Name);
		marineStatus->lastmodule=NULL;
		marineStatus->destinationmodule=NULL;
		marineStatus->missionmodule=NULL;
		marineStatus->fearmodule=NULL;
		marineStatus->my_facing_point=toolsData->facing_point;
		marineStatus->path=toolsData->path;
		marineStatus->stepnumber=toolsData->stepnumber;
		marineStatus->sawlastframe=0;
		marineStatus->gotapoint=0;
		marineStatus->lastframe_fallingspeed=0;
		marineStatus->suspicious=0;
		marineStatus->previous_suspicion=0;
		marineStatus->using_squad_suspicion=0;
		marineStatus->suspect_point.vx=0;
		marineStatus->suspect_point.vy=0;
		marineStatus->suspect_point.vz=0;
		marineStatus->internalState=0;
		#if MOTIONTRACKERS
		if (marineStatus->My_Weapon->EnableTracker) {
			if ((FastRandom()&65535)<(ONE_FIXED>>2)) {
				marineStatus->mtracker_timer=FastRandom()&65535;
			} else {
				marineStatus->mtracker_timer=-1;
			}
		} else {
			marineStatus->mtracker_timer=-1;
		}
		#else
		marineStatus->mtracker_timer=-1;
		#endif
		marineStatus->GibbFactor=0;
		marineStatus->Wounds=0;

		marineStatus->incidentFlag=0;
		marineStatus->incidentTimer=0;

		//InitShapeAnimationController(&marineStatus->ShpAnimCtrl, GetShapeData(sbPtr->shapeIndex));
		
		GLOBALASSERT(marineStatus->My_Weapon->HierarchyName);

		root_section=GetNamedHierarchyFromLibrary(marineStatus->My_Weapon->Riffname,marineStatus->My_Weapon->HierarchyName);

		Create_HModel(&marineStatus->HModelController,root_section);
		InitHModelSequence(&marineStatus->HModelController,(int)HMSQT_MarineRun,(int)MRSS_Standard,ONE_FIXED);

		if (marineStatus->My_Weapon->UseElevation) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&marineStatus->HModelController,"Elevation",(int)HMSQT_MarineStand,(int)MSSS_Elevation,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
		}
		
		/* Create blank hit delta sequence. */
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0)
		{
			if (HModelSequence_Exists(&marineStatus->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
				DELTA_CONTROLLER *delta;
				delta=Add_Delta_Sequence(&marineStatus->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
				GLOBALASSERT(delta);
				delta->Playing=0;
			}
		}

		marineStatus->My_Gunflash_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->GunflashName);
		marineStatus->My_Elevation_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->ElevationSection);
		
		for(i=0;i<SB_NAME_LENGTH;i++) marineStatus->death_target_ID[i] = toolsData->death_target_ID[i];
		marineStatus->death_target_request=toolsData->death_target_request;
		marineStatus->death_target_sbptr=0;

		//this marine wasn't produced by a generator
		marineStatus->generator_sbptr=0;
		
		//initialise female to 0..
		//may be changed by ChangeToAlternateAccoutrementSet
		marineStatus->Female=0;
		marineStatus->Voice=0;
		marineStatus->Android=marineStatus->My_Weapon->Android;
		
		//use accoutement set for both marine and civilian
		#if 0
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0) {
			/* Normal marine. */
			if ((toolsData->textureID==0)||(toolsData->textureID>4)) {
				/* Random. */
				int dice=FastRandom()&65535;
				if (dice<16384) {
					ChangeToAlternateShapeSet(sbPtr, "Face Four");
				} else if (dice<32768) {
					ChangeToAlternateShapeSet(sbPtr, "Face Three");
				}
			} else {
				switch (toolsData->textureID) {
					case 1:
					default:
						/* No change. */
						break;
					case 2:
						ChangeToAlternateShapeSet(sbPtr, "Face Four");
						break;
					case 3:
						ChangeToAlternateShapeSet(sbPtr, "Face Three");
						break;
				}
			}
		} else if (strcmp("hnpc_civvie",marineStatus->My_Weapon->Riffname)==0) {
			/* Civvies. */
			#if 0
			if ((toolsData->textureID==0)||(toolsData->textureID>3)) {
				int dice=FastRandom()&65535;
				if (dice<32767) {
					ChangeToAlternateShapeSet(sbPtr, "MedicalGuy");
				} else if (dice<49152) {
					ChangeToAlternateShapeSet(sbPtr, "CompanyGuy");
				}
			} else {
				switch (toolsData->textureID) {
					case 1:
					default:
						/* No change. */
						break;
					case 2:
						ChangeToAlternateShapeSet(sbPtr, "MedicalGuy");
						break;
					case 3:
						ChangeToAlternateShapeSet(sbPtr, "CompanyGuy");
						break;
				}
			}
			#else
			/* Gonna need a different handler for this? */
			ChangeToAlternateAccoutrementSet(sbPtr, toolsData->textureID);
			#endif
		}
		#else
		ChangeToAlternateAccoutrementSet(sbPtr, toolsData->textureID);
		#endif
		marineStatus->VoicePitch = (FastRandom() & 255) - 128;

		Marine_SwitchExpression(sbPtr,0);
		marineStatus->Target_Expression=0;
		marineStatus->Blink=-1;

		ProveHModel_Far(&marineStatus->HModelController,sbPtr);

		if (marineStatus->My_Weapon->WeaponFireFunction==NULL) {
			InitMission(sbPtr,MM_NonCom);
		} else {
			InitMission(sbPtr,toolsData->Mission);
		}
	}
	else 
	{
		/* allocation failed */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}			   	   	   	   
}

void InitSealBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr)
{
	/* Die, superfluous function!!! */	
	InitMarineBehaviour(bhdata,sbPtr);

}

void CreateMarineDynamic(STRATEGYBLOCK *Generator,MARINE_NPC_WEAPONS weapon_for_marine)
{
	STRATEGYBLOCK* sbPtr;
	GENERATOR_BLOCK *generatorBlock;

	generatorBlock=Generator->SBdataptr;
	GLOBALASSERT(generatorBlock);

	/* check we're not in a net game */
	if(AvP.Network != I_No_Network) 
	{
		return;
	}

	/* create and initialise a strategy block */
	sbPtr = CreateActiveStrategyBlock();
	if(!sbPtr)
	{
		/* allocation failed */
		return;
	}

	InitialiseSBValues(sbPtr);
	sbPtr->I_SBtype = I_BehaviourMarine;	

	/* Old way. *
	for(i = 0; i < SB_NAME_LENGTH; i++) {
		sbPtr->SBname[i] = '\0';
	}
	* CDF - 9/9/97 */
	AssignNewSBName(sbPtr);
	/* New way. */
				
	/* create, initialise and attach a dynamics block */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_SPRITE_NPC);
	if(sbPtr->DynPtr)
	{
		EULER zeroEuler = {0,0,0};
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dynPtr);
      	dynPtr->PrevPosition = dynPtr->Position = ((GENERATOR_BLOCK* )Generator->SBdataptr)->Position;
		dynPtr->OrientEuler = zeroEuler;
		CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
		TransposeMatrixCH(&dynPtr->OrientMat);      
	}
	else 
	{
		/* allocation failed */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}

	/* set the shape */
	sbPtr->shapeIndex = Generator->shapeIndex;

	sbPtr->maintainVisibility = 1;
	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);
	LOCALASSERT(sbPtr->containingModule);
	if(!(sbPtr->containingModule))
	{
		/* no containing module can be found... abort*/
		DestroyAnyStrategyBlock(sbPtr);
		return;
	}
  	
  	/* assert marine is starting as invisible */
  	LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)] == 0);

	/* create, initialise and attach a marine data block */
	sbPtr->SBdataptr = (void *)AllocateMem(sizeof(MARINE_STATUS_BLOCK));
	if(sbPtr->SBdataptr)
	{
		SECTION *root_section;
		MARINE_STATUS_BLOCK *marineStatus = (MARINE_STATUS_BLOCK *)sbPtr->SBdataptr;
		GLOBALASSERT(marineStatus);

		NPC_InitMovementData(&(marineStatus->moveData));
		NPC_InitWanderData(&(marineStatus->wanderData));
     	marineStatus->health = MARINE_STARTING_HEALTH;
   		sbPtr->integrity = marineStatus->health;
		marineStatus->volleySize = 0;
		marineStatus->nearSpeed = MARINE_NEAR_SPEED;
		marineStatus->primaryWeaponDamage = MARINE_WEAPON_DAMAGE;
		marineStatus->stateTimer = MARINE_FAR_MOVE_TIME;
		marineStatus->weaponTarget.vx = marineStatus->weaponTarget.vy = marineStatus->weaponTarget.vz = 0;
		marineStatus->myGunFlash = (DISPLAYBLOCK *)0;  							
		marineStatus->soundHandle = SOUND_NOACTIVEINDEX;
		marineStatus->soundHandle2 = SOUND_NOACTIVEINDEX;

		marineStatus->obstruction.environment=0;
		marineStatus->obstruction.destructableObject=0;
		marineStatus->obstruction.otherCharacter=0;
		marineStatus->obstruction.anySingleObstruction=0;

		Initialise_AvoidanceManager(sbPtr,&marineStatus->avoidanceManager);
		InitWaypointManager(&marineStatus->waypointManager);

		marineStatus->IAmCrouched = 0;
		marineStatus->lastroundhit=0;
		marineStatus->lasthitsection=NULL;

		marineStatus->weapon_variable=0;
		marineStatus->weapon_variable2=0;

		marineStatus->Skill=Marine_Skill;
		marineStatus->Courage=ONE_FIXED;

		marineStatus->FiringAnim=0;
		marineStatus->SpotFlag=0;

		//a generated marine won't have a death target
		{
			int i;
			for(i=0;i<SB_NAME_LENGTH;i++) marineStatus->death_target_ID[i] =0; 
			marineStatus->death_target_request=0;
			marineStatus->death_target_sbptr=0;
		}
		
		//note the generator that produced this marine
		marineStatus->generator_sbptr=Generator;

		marineStatus->Target=NULL; //Player->ObStrategyBlock;

		marineStatus->My_Weapon=GetThisNPCMarineWeapon(weapon_for_marine);
	
		/* Initialise marine's stats */
		{
			NPC_DATA *NpcData;

			if (marineStatus->My_Weapon->Android) {
				NpcData=GetThisNpcData(I_NPC_Android);
			} else if (marineStatus->My_Weapon->ARealMarine) {
				NpcData=GetThisNpcData(I_NPC_Marine);
			} else {
				NpcData=GetThisNpcData(I_NPC_Civilian);
			}
			LOCALASSERT(NpcData);
			sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
			sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		}
		
		{
			const MOVEMENT_DATA *movementData;

			marineStatus->speedConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			marineStatus->accelerationConstant=(ONE_FIXED-8192)+(FastRandom()&16383);
			
			if (marineStatus->My_Weapon->ARealMarine) {	
				movementData=GetThisMovementData(MDI_Marine_Combat);
			} else {
				movementData=GetThisMovementData(MDI_Civilian_Combat);
			}
			GLOBALASSERT(movementData);
			marineStatus->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatus->speedConstant);
			marineStatus->acceleration = MUL_FIXED(movementData->acceleration,marineStatus->accelerationConstant);
		}

		COPY_NAME(marineStatus->Target_SBname,Null_Name);
		marineStatus->lastmodule=NULL;
		marineStatus->destinationmodule=NULL;
		marineStatus->missionmodule=NULL;
		marineStatus->fearmodule=NULL;
		marineStatus->my_facing_point=sbPtr->DynPtr->Position;

		marineStatus->path=generatorBlock->path;
		marineStatus->stepnumber=generatorBlock->stepnumber;

		marineStatus->sawlastframe=0;
		marineStatus->gotapoint=0;
		marineStatus->lastframe_fallingspeed=0;
		marineStatus->suspicious=0;
		marineStatus->previous_suspicion=0;
		marineStatus->using_squad_suspicion=0;
		marineStatus->suspect_point.vx=0;
		marineStatus->suspect_point.vy=0;
		marineStatus->suspect_point.vz=0;
		marineStatus->internalState=0;
		#if MOTIONTRACKERS
		if (marineStatus->My_Weapon->EnableTracker) {
			if ((FastRandom()&65535)<(ONE_FIXED>>2)) {
				marineStatus->mtracker_timer=FastRandom()&65535;
			} else {
				marineStatus->mtracker_timer=-1;
			}
		} else {
			marineStatus->mtracker_timer=-1;
		}
		#else
		marineStatus->mtracker_timer=-1;
		#endif
		marineStatus->GibbFactor=0;
		marineStatus->Wounds=0;

		marineStatus->incidentFlag=0;
		marineStatus->incidentTimer=0;

		if (marineStatus->My_Weapon->id==MNPCW_MPistol) {
			/* Special case for pistols? */
			marineStatus->lastroundhit=PISTOL_CLIP_SIZE;
		}
		marineStatus->clipammo=marineStatus->My_Weapon->clip_size;
		marineStatus->roundsForThisTarget=0;

		//InitShapeAnimationController(&marineStatus->ShpAnimCtrl, GetShapeData(sbPtr->shapeIndex));
		
		root_section=GetNamedHierarchyFromLibrary(marineStatus->My_Weapon->Riffname,marineStatus->My_Weapon->HierarchyName);
		Create_HModel(&marineStatus->HModelController,root_section);
		InitHModelSequence(&marineStatus->HModelController,(int)HMSQT_MarineRun,(int)MRSS_Standard,ONE_FIXED);
		
		if (marineStatus->My_Weapon->UseElevation) {
			DELTA_CONTROLLER *delta;
			delta=Add_Delta_Sequence(&marineStatus->HModelController,"Elevation",(int)HMSQT_MarineStand,(int)MSSS_Elevation,0);
			GLOBALASSERT(delta);
			delta->timer=32767;
		}
		
		/* Create blank hit delta sequence. */
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0)
		{
			if (HModelSequence_Exists(&marineStatus->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
				DELTA_CONTROLLER *delta;
				delta=Add_Delta_Sequence(&marineStatus->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
				GLOBALASSERT(delta);
				delta->Playing=0;
			}
		}

		marineStatus->My_Gunflash_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->GunflashName);
		marineStatus->My_Elevation_Section=GetThisSectionData(marineStatus->HModelController.section_data,marineStatus->My_Weapon->ElevationSection);
		
		//initialise female to 0..
		//may be changed by ChangeToAlternateAccoutrementSet
		marineStatus->Female=0;
		marineStatus->Voice=0;
		marineStatus->Android=marineStatus->My_Weapon->Android;
		
		//use accoutement set for both marine and civilian
		#if 0
		if (strcmp("hnpcmarine",marineStatus->My_Weapon->Riffname)==0) {
			int dice=FastRandom()&65535;
			if (dice<16384) {
				ChangeToAlternateShapeSet(sbPtr, "Face Four");
			} else if (dice<32768) {
				ChangeToAlternateShapeSet(sbPtr, "Face Three");
			}
		} else if (strcmp("hnpc_civvie",marineStatus->My_Weapon->Riffname)==0) {
			#if 0
			int dice=FastRandom()&65535;
			if (dice<32767) {
				ChangeToAlternateShapeSet(sbPtr, "MedicalGuy");
			} else if (dice<49152) {
				ChangeToAlternateShapeSet(sbPtr, "CompanyGuy");
			}
			#else
			ChangeToAlternateAccoutrementSet(sbPtr, 0);
			#endif
		}
		#else
		//texture id 0 picks random one out of those allowed by the level
		ChangeToAlternateAccoutrementSet(sbPtr, 0);
		#endif
		marineStatus->VoicePitch = (FastRandom() & 255) - 128;

		Marine_SwitchExpression(sbPtr,0);
		marineStatus->Target_Expression=0;
		marineStatus->Blink=-1;

		ProveHModel_Far(&marineStatus->HModelController,sbPtr);
		
		if (marineStatus->My_Weapon->WeaponFireFunction==NULL) {
			InitMission(sbPtr,MM_NonCom);
		} else if ( (marineStatus->path!=-1)&&(marineStatus->stepnumber!=-1)) {
			InitMission(sbPtr,MM_Pathfinder);
		} else {
			InitMission(sbPtr,MM_Wander);
		}

	}
	else 
	{
		/* allocation failed */
		RemoveBehaviourStrategy(sbPtr);
		return;
	}
}

void SetMarineElevation(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	int offsetx,offsety,offsetz,offseta,angle1;
	DELTA_CONTROLLER *elevation_controller;
	VECTORCH *gunpos;

	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	if (marineStatusPointer->My_Weapon->UseElevation==0) {
		/* Non elevating weapon. */
		return;
	}

	if (marineStatusPointer->My_Elevation_Section) {
		gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
	} else {
		gunpos=&sbPtr->DynPtr->Position;
	}
	/* Aim at weaponTarget. */
	
	offsetx=(marineStatusPointer->weaponTarget.vx)-(gunpos->vx);
	offsety=(marineStatusPointer->weaponTarget.vz)-(gunpos->vz);
	offseta=-((marineStatusPointer->weaponTarget.vy)-(gunpos->vy));

	while( (offsetx>(ONE_FIXED>>2))
		||(offsety>(ONE_FIXED>>2))
		||(offseta>(ONE_FIXED>>2))
		||(offsetx<-(ONE_FIXED>>2))
		||(offsety<-(ONE_FIXED>>2))
		||(offseta<-(ONE_FIXED>>2))) {
	
		offsetx>>=1;
		offsety>>=1;
		offseta>>=1;

	}

	offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
	angle1=ArcTan(offseta,offsetz);

	if (angle1>=3072) angle1-=4096;
	if (angle1>=2048) angle1=angle1-3072;
	if (angle1>1024) angle1=2048-angle1;

	GLOBALASSERT(angle1>=-1024);
	GLOBALASSERT(angle1<=1024);


	elevation_controller=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Elevation");
	GLOBALASSERT(elevation_controller);
	
	if (marineStatusPointer->IAmCrouched) {
		elevation_controller->sequence_type=HMSQT_MarineCrouch;
		elevation_controller->sub_sequence=MCrSS_Elevation;
	} else {
		if (marineStatusPointer->FiringAnim==1) {
			elevation_controller->sequence_type=HMSQT_MarineStand;
			elevation_controller->sub_sequence=MSSS_Hip_Fire_Elevation;
		} else {
			elevation_controller->sequence_type=HMSQT_MarineStand;
			elevation_controller->sub_sequence=MSSS_Elevation;
		}
	}

	{
		int fake_timer;

		fake_timer=1024-angle1;
		fake_timer<<=5;
		if (fake_timer==65536) fake_timer=65535;

		GLOBALASSERT(fake_timer>=0);
		GLOBALASSERT(fake_timer<65536);

		elevation_controller->timer=fake_timer;

	}

	/* Unless you're a reloading pistol. */
	if (marineStatusPointer->My_Weapon->id==MNPCW_MPistol) {
		if (marineStatusPointer->lastroundhit==-1) {
			elevation_controller->timer=32767;
		}
	}
	/* Or a firing grenade launcher or shotgun in state 1. */
	if ((marineStatusPointer->My_Weapon->id==MNPCW_GrenadeLauncher)||(marineStatusPointer->My_Weapon->id==MNPCW_MShotgun)
		||(marineStatusPointer->My_Weapon->id==MNPCW_Android)
		||(marineStatusPointer->My_Weapon->id==MNPCW_AndroidSpecial)) {
		if ((marineStatusPointer->behaviourState==MBS_Firing)&&(marineStatusPointer->internalState)) {
			elevation_controller->timer=32767;
		}
	}
}

void CentreMarineElevation(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	DELTA_CONTROLLER *elevation_controller;

	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    

	if (marineStatusPointer->My_Weapon->UseElevation==0) {
		/* Non elevating weapon. */
		return;
	}

	elevation_controller=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Elevation");
	GLOBALASSERT(elevation_controller);

	if (marineStatusPointer->IAmCrouched) {
		elevation_controller->sequence_type=HMSQT_MarineCrouch;
		elevation_controller->sub_sequence=MCrSS_Elevation;
	} else {
		if (marineStatusPointer->FiringAnim==1) {
			elevation_controller->sequence_type=HMSQT_MarineStand;
			elevation_controller->sub_sequence=MSSS_Hip_Fire_Elevation;
		} else {
			elevation_controller->sequence_type=HMSQT_MarineStand;
			elevation_controller->sub_sequence=MSSS_Elevation;
		}
	}

	elevation_controller->timer=32767;

}

void MarineBehaviour(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* test if we've got a containing module: if we haven't, do nothing.
	This is important as the object could have been marked for deletion by the visibility 
	management system...*/
	if(!sbPtr->containingModule)
	{
		DestroyAnyStrategyBlock(sbPtr); /* just to make sure */
		return;
	} else if (marineStatusPointer->behaviourState!=MBS_Dying) {
		#if SUPER_PHEROMONE_SYSTEM
		AddMarinePheromones(sbPtr->containingModule->m_aimodule);
		#endif
	}

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}

	if (ShowSlack) {
		int synthSpeed,setSpeed,slack;
		VECTORCH offset;
		extern int SlackTotal;
		extern int SlackSize;
				
		offset.vx=(sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx);
		offset.vy=(sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy);
		offset.vz=(sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz);
		
		synthSpeed=Magnitude(&offset);
		synthSpeed=DIV_FIXED(synthSpeed,NormalFrameTime);
		setSpeed=Magnitude(&sbPtr->DynPtr->LinVelocity);
		
		if (setSpeed) {		
			slack=(ONE_FIXED-(DIV_FIXED(synthSpeed,setSpeed)));
			SlackTotal+=slack;
			SlackSize++;
		}
		#if 0
		PrintDebuggingText("MaxSpeed = %d, SynthSpeed = %d, SetSpeed = %d, Slack %d\n",alienStatusPointer->MaxSpeed,synthSpeed,setSpeed,slack);
		#endif
	}

	GLOBALASSERT(marineStatusPointer->My_Weapon->WeaponPanicFireFunction);
	
	InitWaypointSystem(0);

	if ((Validate_Target(marineStatusPointer->Target,marineStatusPointer->Target_SBname)==0)
		&&((marineStatusPointer->Target!=Player->ObStrategyBlock)||(Observer))) {
		marineStatusPointer->Target=NULL;
		/* Were you suspicious of something before? */
		if (marineStatusPointer->previous_suspicion) {
			marineStatusPointer->suspicious=marineStatusPointer->previous_suspicion;
			marineStatusPointer->previous_suspicion=0;
			marineStatusPointer->using_squad_suspicion=0;
		}
	}

	/* unset suspicion? */
	
	if (marineStatusPointer->suspicious!=0) {
		if ((marineStatusPointer->behaviourState!=MBS_Approaching)
			&&(marineStatusPointer->behaviourState!=MBS_Responding)) {
			marineStatusPointer->suspicious-=NormalFrameTime;
			/* To fix the next trap... */
			if (marineStatusPointer->suspicious==0) {
				marineStatusPointer->suspicious=-1;
			}
		}
		if (marineStatusPointer->suspicious<0) {
			marineStatusPointer->suspicious=0;
			/* Set to zero on natural timeout, too. */
			marineStatusPointer->previous_suspicion=0;
			marineStatusPointer->using_squad_suspicion=0;
			if ((marineStatusPointer->behaviourState==MBS_Waiting)
				||(marineStatusPointer->behaviourState==MBS_Sentry)) {
				/* We might concievably want to do this for all states. */
				marineStatusPointer->gotapoint=0;
			}
		}
		/* Approaching marines remain suspicious. */
	} else {
		/* Not suspicious. */
		if (marineStatusPointer->Target) {
			if (!MarineCanSeeTarget(sbPtr)) {
				/* Oh well, forget it, then. */
				marineStatusPointer->Target=NULL;
			}
		}
	}
	/* Squad level suspicion? */
	if (((marineStatusPointer->suspicious==0)||(marineStatusPointer->using_squad_suspicion))
		&&(NpcSquad.Squad_Suspicion!=0)
		&&(sbPtr->SBdptr)
		&&(marineStatusPointer->behaviourState!=MBS_Approaching)
		&&(marineStatusPointer->behaviourState!=MBS_Firing)
		&&(marineStatusPointer->behaviourState!=MBS_Dying)) {
		/* Use squad suspicion. */
		marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
		marineStatusPointer->suspect_point=NpcSquad.squad_suspect_point;
		/* Set this to zero when you get a *new* suspicion. */
		marineStatusPointer->previous_suspicion=0;
		marineStatusPointer->using_squad_suspicion=1;
	}
	
	if (marineStatusPointer->sawlastframe==2) {
		marineStatusPointer->sawlastframe=0;
	}

	/* Unset incident flag. */
	marineStatusPointer->incidentFlag=0;

	marineStatusPointer->incidentTimer-=NormalFrameTime;
	
	if (marineStatusPointer->incidentTimer<0) {
		marineStatusPointer->incidentFlag=1;
		marineStatusPointer->incidentTimer=32767+(FastRandom()&65535);
	}

	/* Run Motion Tracker. */

	tracker_noise=0;
	
	if (marineStatusPointer->mtracker_timer>=0) {
		marineStatusPointer->mtracker_timer+=NormalFrameTime;
	}
	/* Negative timer means no tracker. */
	if (marineStatusPointer->mtracker_timer>ONE_FIXED) {
		marineStatusPointer->mtracker_timer=0;
		tracker_noise=1;
	}

	if ((marineStatusPointer->Target==NULL) 
		#if ANARCHY
		|| (marineStatusPointer->lastmodule!=sbPtr->containingModule->m_aimodule)
		|| (marineStatusPointer->Target==Player->ObStrategyBlock)
		#endif
		) {
		
		if ((marineIsNear)||(marineStatusPointer->incidentFlag)) {
			/* Get new target. */
			marineStatusPointer->roundsForThisTarget=0;
			marineStatusPointer->sawlastframe=0;		
			marineStatusPointer->Target=Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr);

			if (marineStatusPointer->Target) {
				#if MARINE_STATE_PRINT
				textprint("Marine gets new target.\n");
				#endif
				COPY_NAME(marineStatusPointer->Target_SBname,marineStatusPointer->Target->SBname);

				/* Remember your suspicion... */
				marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
				marineStatusPointer->suspicious=0;
		
				/* Do stats. */
				if (marineStatusPointer->Target==Player->ObStrategyBlock) {
					if (marineStatusPointer->SpotFlag==0) {
						CurrentGameStats_Spotted();
						marineStatusPointer->SpotFlag=1;
					}
				}

				/* New Enemy! */
		
				if (NpcSquad.alertStatus==0) {
					NpcSquad.alertStatus=1;
				}
				if (marineStatusPointer->Target->containingModule) {
					ZoneAlert(2,marineStatusPointer->Target->containingModule->m_aimodule);
				} else {
					ZoneAlert(2,sbPtr->containingModule->m_aimodule);
				}

			} else {
				#if 0
					PrintDebuggingText("Marine found no target!\n");
				#endif
			}
		}
	} else if (marineStatusPointer->mtracker_timer==0) {
		/* Fake sweep for wheeps. */
		FakeTrackerWheepGenerator(&sbPtr->DynPtr->Position,sbPtr);
		
		if (marineStatusPointer->Target->SBdptr) {
			if (NpcSquad.Squad_Suspicion!=SQUAD_PARANOIA_TIME) {
				if (MarineCanSeeTarget(sbPtr)) {
					/* Hey, guys! */
					GLOBALASSERT(marineStatusPointer->Target->DynPtr);
					NpcSquad.Squad_Suspicion=SQUAD_PARANOIA_TIME;
					NpcSquad.squad_suspect_point=marineStatusPointer->Target->DynPtr->Position;
				}
			}
		}
	}

	/* Brushing Test. */

	{
		struct collisionreport *nextReport;
		nextReport = sbPtr->DynPtr->CollisionReportPtr;

		while(nextReport)
		{		
 			if(nextReport->ObstacleSBPtr)
 			{	
				if((nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourAlien)||
				   ((nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourMarinePlayer)
				   &&(AvP.PlayerType!=I_Marine))||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourAlienPlayer)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourPredatorPlayer)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourNetGhost)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourXenoborg)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourPredatorAlien)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourQueenAlien)||
				   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourFaceHugger))
					{
					marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
					marineStatusPointer->suspect_point=nextReport->ObstacleSBPtr->DynPtr->Position;
					/* Set this to zero when you get a *new* suspicion. */
					marineStatusPointer->previous_suspicion=0;
					marineStatusPointer->using_squad_suspicion=0;
					if (marineStatusPointer->Android==0) {
						marineStatusPointer->Courage-=(NormalFrameTime>>1);
					}
				}
			} 		
 			nextReport = nextReport->NextCollisionReportPtr;
		}
	}

	/* Blinking! */
	if (marineStatusPointer->behaviourState!=MBS_Dying) {
		if (marineStatusPointer->Expression<3) {
			if (marineStatusPointer->incidentFlag) {
				if ((FastRandom()&65535)<24000) {
					switch (marineStatusPointer->Expression) {
						default:
							GLOBALASSERT(0);
							break;
						case 0:
							Marine_SwitchExpression(sbPtr,3);
							break;
						case 1:
							Marine_SwitchExpression(sbPtr,4);
							break;
						case 2:
							Marine_SwitchExpression(sbPtr,5);
							break;
					}
					marineStatusPointer->Blink=0;
				}
			}
		} else if (marineStatusPointer->Expression>=3) {
			if (marineStatusPointer->Blink>=0) {
				marineStatusPointer->Blink+=NormalFrameTime;
				if (marineStatusPointer->Blink>(ONE_FIXED/7)) {
					switch (marineStatusPointer->Expression) {
						default:
							GLOBALASSERT(0);
							break;
						case 3:
							Marine_SwitchExpression(sbPtr,0);
							break;
						case 4:
							Marine_SwitchExpression(sbPtr,1);
							break;
						case 5:
							Marine_SwitchExpression(sbPtr,2);
							break;
					}
				}
			}
		}
	}

	/* Now hearing. */
	
	DoMarineHearing(sbPtr);

	/* That was senses. Now courage. */
	if (marineStatusPointer->Android==0) {

		if ((marineStatusPointer->Target==NULL)&&(marineStatusPointer->suspicious==0)) {
			marineStatusPointer->Courage+=MUL_FIXED(NormalFrameTime,300);
		}

		marineStatusPointer->Courage+=NpcSquad.Squad_Delta_Morale;

		if (marineStatusPointer->Courage>=(ONE_FIXED<<1)) {
			marineStatusPointer->Courage=(ONE_FIXED<<1);
		} else if (marineStatusPointer->Courage<0) {
			marineStatusPointer->Courage=0;
		}
	} else {
		marineStatusPointer->Courage=ONE_FIXED;
	}

	/* Dead yet? */
	
	if (marineStatusPointer->GibbFactor) {
		/* If you're gibbed, you're dead. */
		sbPtr->SBDamageBlock.Health = 0;
	}

	if (sbPtr->SBDamageBlock.IsOnFire) {

		CauseDamageToObject(sbPtr,&firedamage,NormalFrameTime,NULL);

		if (sbPtr->I_SBtype==I_BehaviourNetCorpse) {
			/* Gettin' out of here... */
			return;
		}

		if (marineStatusPointer->Mission!=MM_RunAroundOnFire) {
			if (marineStatusPointer->Android==0) {
				Convert_To_RunningOnFire(sbPtr);
			} else {
				/* Handle sound for an android. */
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
				} else {
					if (ActiveSounds[marineStatusPointer->soundHandle].soundIndex!=SID_FIRE) {
						Sound_Stop(marineStatusPointer->soundHandle);
					 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&marineStatusPointer->soundHandle,127);
					}
				}
			}
		}
		
	}

	/* Mission Control! */

	if ((ShowSquadState)||((sbPtr->SBdptr)&&(ShowNearSquad))) {
		if (sbPtr->name) {
			PrintDebuggingText("%s: ",sbPtr->name);
		} else {
			PrintDebuggingText("Unnamed: ");
		}
		if (marineStatusPointer->suspicious) {
			PrintDebuggingText("Suspicious %d ",marineStatusPointer->suspicious);
		}
	}

	if ((marineStatusPointer->Mission!=MM_NonCom)&&(marineStatusPointer->Mission!=MM_RunAroundOnFire)) {
		if (marineStatusPointer->My_Weapon->WeaponFireFunction==NULL) {
			LOGDXFMT(("Marine Weapon ID = %d\n",marineStatusPointer->My_Weapon->id));
			LOGDXFMT(("Mission %d\n",marineStatusPointer->Mission));
		}
		GLOBALASSERT(marineStatusPointer->My_Weapon->WeaponFireFunction);
	}

	switch (marineStatusPointer->Mission) {
		case (MM_Wait_Then_Wander):
		case (MM_Wander):
		{
			WanderMission_Control(sbPtr);
			break;
		}
		case (MM_Guard):
		{
			GuardMission_Control(sbPtr);
			break;
		}
		case (MM_LocalGuard):
		{
			LocalGuardMission_Control(sbPtr);
			break;
		}
		case (MM_NonCom):
		{
			LoiterMission_Control(sbPtr);
			break;
		}
		case (MM_Pathfinder):
		{
			PathfinderMission_Control(sbPtr);
			break;
		}
		case (MM_RunAroundOnFire):
		{
			RunAroundOnFireMission_Control(sbPtr);
			break;
		}
		default:
		{
			GLOBALASSERT(0);
			break;
		}
	}

	if (TERROR_MODE) {
		if ((marineStatusPointer->behaviourState!=MBS_PanicFire)
			&&(marineStatusPointer->behaviourState!=MBS_PanicReloading)
			&&(marineStatusPointer->behaviourState!=MBS_Retreating)
			&&(marineStatusPointer->behaviourState!=MBS_Approaching)
			&&(marineStatusPointer->behaviourState!=MBS_GetWeapon)
			&&(marineStatusPointer->behaviourState!=MBS_Taunting)
			&&(marineStatusPointer->behaviourState!=MBS_Avoidance)
			&&(marineStatusPointer->behaviourState!=MBS_Dying)
			) {
			Marine_Enter_Retreat_State(sbPtr);
		}
	}

	if (marineStatusPointer->My_Weapon->id==MNPCW_Minigun) {
		DELTA_CONTROLLER *mgd=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		if (mgd) {
			NPC_Maintain_Minigun(sbPtr,mgd);
		}
	}

	/* if we have actually died, we need to remove the strategyblock... so
	do this here */
	if((marineStatusPointer->behaviourState == MBS_Dying)&&(marineStatusPointer->stateTimer <= 0)) {
	
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle);				
		if(marineStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle2);

		DestroyAnyStrategyBlock(sbPtr);
	}

	marineStatusPointer->lastmodule=sbPtr->containingModule->m_aimodule;

	if (sbPtr->SBdptr) {
		/* I reckon only near trackers wheep. */
		if (tracker_noise==1) {
			Sound_Play(SID_TRACKER_CLICK,"d",&sbPtr->DynPtr->Position);
		} else if (tracker_noise==2) {
			Sound_Play(SID_TRACKER_WHEEP,"d",&sbPtr->DynPtr->Position);
		}
	}

	/* Update squad stats. */
	if ((marineStatusPointer->behaviourState==MBS_Responding)
		||((marineStatusPointer->behaviourState==MBS_Avoidance)
			&&(marineStatusPointer->lastState==MBS_Responding))) {

		NpcSquad.Alt_RespondingMarines++;
	}

	if ((marineStatusPointer->Mission==MM_RunAroundOnFire)
		&&(sbPtr->SBdptr)) {

		NpcSquad.Alt_NearBurningMarines++;
	}

	if ((sbPtr->SBdptr)&&(
		(marineStatusPointer->behaviourState==MBS_Retreating)
		||(marineStatusPointer->Mission==MM_RunAroundOnFire)
		)) {
		/* Burning marines are considered panicked, too. */
			
		NpcSquad.Alt_NearPanickedMarines++;
	}
	
	if ((sbPtr->SBdptr)&&(marineStatusPointer->My_Weapon->ARealMarine)&&(
		(marineStatusPointer->behaviourState!=MBS_Retreating)
		&&(marineStatusPointer->behaviourState!=MBS_Dying)
		)) {
	
		NpcSquad.Alt_NearUnpanickedMarines++;
	}

	/* Make marines heal, really slowly? */
	HModel_Regen(&marineStatusPointer->HModelController,(120*ONE_FIXED));
	/* Two minutes.  Only heals sections, too. */

	/* Change the face if we can... */
	Marine_UpdateFace(sbPtr);

	/* And finally, update lastframe flag. */
	Marine_ConsiderFallingDamage(sbPtr);

}

void EndMarineMuzzleFlash(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;

	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);

	if(marineStatusPointer->myGunFlash) 
	{
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;				
	}

	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

}

void InitMission(STRATEGYBLOCK *sbPtr,MARINE_MISSION mission) {

	MARINE_STATUS_BLOCK *marineStatus;

	marineStatus = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatus);

	switch (mission) {
		case MM_RunAroundOnFire:
			marineStatus->Mission=MM_RunAroundOnFire;
   			marineStatus->behaviourState = MBS_Wandering;
			marineStatus->lastState = MBS_Waiting;
			/* Shouldn't need a sequence force. */
			break;
		case MM_Guard:
			marineStatus->Mission=MM_Guard;
   			marineStatus->behaviourState = MBS_Sentry;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_SentryMode_State(sbPtr);
			break;
		case MM_LocalGuard:
			marineStatus->Mission=MM_LocalGuard;
   			marineStatus->behaviourState = MBS_Waiting;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_Wait_State(sbPtr);
			break;
		case MM_NonCom:
			marineStatus->Mission=MM_NonCom;
   			marineStatus->behaviourState = MBS_Waiting;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_Wait_State(sbPtr);
			break;
		case MM_Wait_Then_Wander:
			marineStatus->Mission=MM_Wait_Then_Wander;
   			marineStatus->behaviourState = MBS_Waiting;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_Wait_State(sbPtr);
			break;
		case MM_Pathfinder:
			marineStatus->Mission=MM_Pathfinder;
   			marineStatus->behaviourState = MBS_Pathfinding;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_Pathfinder_State(sbPtr);
			break;
		case MM_Wander:
		default:
			marineStatus->Mission=MM_Wander;
   			marineStatus->behaviourState = MBS_Approaching;
			marineStatus->lastState = MBS_Waiting;
			Marine_Enter_Approach_State(sbPtr);
			break;
	}

}

void WanderMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		
	

	/* Current Behaviour. */

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}
	
	{
	
		if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
			if (marineStatusPointer->Mission==MM_Wait_Then_Wander) {
				/* A bit of a cheat. */
				PrintDebuggingText("Wait Then ");
			}
		}

		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Approaching):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine approach in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Approach(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Approach(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Firing):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponFireFunction)(sbPtr);
					SetMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PumpAction):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine pump action in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_PumpAction(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PumpAction(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicFire):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine panic firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponPanicFireFunction)(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Avoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine ");
					switch (marineStatusPointer->avoidanceManager.substate) {
						default:
						case AvSS_FreeMovement:
							PrintDebuggingText("Avoidance Level 0");
							break;
						case AvSS_FirstAvoidance:
							PrintDebuggingText("Avoidance Level 1");
							break;
						case AvSS_SecondAvoidance:
							PrintDebuggingText("Avoidance Level 2");
							break;
						case AvSS_ThirdAvoidance:
							PrintDebuggingText("Avoidance Level 3");
							break;
					}	
					PrintDebuggingText(" in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
				if(marineIsNear) {
					state_result=Execute_MNS_Avoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Avoidance(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Wandering):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine wander in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wander(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wander(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Responding):
			{
				/* Closest to hunt. */
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine responding in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Respond(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Respond(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Retreating):
			{
				/* Real men never retreat! */
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine Retreating in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
			
				if (marineIsNear) {
					state_result=Execute_MNS_Retreat(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Retreat(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Waiting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine wait in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wait(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Sentry):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine sentry in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
				GLOBALASSERT(0);
				if(marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Dying):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine dying in %s\n",sbPtr->containingModule->name);
				}
				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			case(MBS_Taunting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine taunt in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Taunting(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Taunting(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Reloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Reloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Reloading(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicReloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine panic reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_PanicReloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PanicReloading(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_GetWeapon):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine get weapon in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_GetWeapon(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_GetWeapon(sbPtr);
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case MBS_Returning:
			case MBS_Pathfinding:
			{
				/* How the hell did you get here?!? */
				Marine_Enter_Wander_State(sbPtr);
				break;
			}
			case(MBS_AcidAvoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Wander marine acid avoidance in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_AcidAvoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					Marine_Enter_Wait_State(sbPtr);
					break;
				}
				WanderMission_SwitchState(sbPtr,state_result);
				break;
			}
			default:
			{
				LOGDXFMT(("Marine in unsupported state %d!\n",marineStatusPointer->behaviourState));
				LOCALASSERT(1==0);
			}
		}	
	}
				
	if (!marineIsNear) {

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void PathfinderMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		
	

	/* Current Behaviour. */

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}
	
	{

		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Approaching):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine approach in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Approach(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Approach(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Firing):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponFireFunction)(sbPtr);
					SetMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PumpAction):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine pump action in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_PumpAction(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PumpAction(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicFire):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine panic firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponPanicFireFunction)(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Avoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine ");
					switch (marineStatusPointer->avoidanceManager.substate) {
						default:
						case AvSS_FreeMovement:
							PrintDebuggingText("Avoidance Level 0");
							break;
						case AvSS_FirstAvoidance:
							PrintDebuggingText("Avoidance Level 1");
							break;
						case AvSS_SecondAvoidance:
							PrintDebuggingText("Avoidance Level 2");
							break;
						case AvSS_ThirdAvoidance:
							PrintDebuggingText("Avoidance Level 3");
							break;
					}	
					PrintDebuggingText(" in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
				if(marineIsNear) {
					state_result=Execute_MNS_Avoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Avoidance(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Wandering):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine wander in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wander(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wander(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Responding):
			{
				/* Closest to hunt. */
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine responding in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Respond(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Respond(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Retreating):
			{
				/* Real men never retreat! */
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine retreating in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
				
				if (marineIsNear) {
					state_result=Execute_MNS_Retreat(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Retreat(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Waiting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine wait in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wait(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Sentry):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine sentry in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
				GLOBALASSERT(0);
				if(marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Dying):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine dying in %s\n",sbPtr->containingModule->name);
				}
				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			case(MBS_Pathfinding):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine pathfinding in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Pathfinder(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Pathfinder(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Taunting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine taunt in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Taunting(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Taunting(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Returning):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine returning in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Return(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Return(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Reloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Reloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Reloading(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicReloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine panic reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_PanicReloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PanicReloading(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_GetWeapon):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Pathfinder marine get weapon in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_GetWeapon(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_GetWeapon(sbPtr);
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_AcidAvoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Prahfinder marine acid avoidance in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_AcidAvoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					Marine_Enter_Wait_State(sbPtr);
					break;
				}
				PathfinderMission_SwitchState(sbPtr,state_result);
				break;
			}
			default:
			{
				LOGDXFMT(("Marine in unsupported state %d!\n",marineStatusPointer->behaviourState));
				LOCALASSERT(1==0);
			}
		}	
	}
				
	if (!marineIsNear) {

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void GuardMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Firstly, fix missionmodule. */

	if (marineStatusPointer->missionmodule==NULL) {
		marineStatusPointer->missionmodule=sbPtr->containingModule->m_aimodule;
		marineStatusPointer->my_spot=sbPtr->DynPtr->Position;
	}

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}
	
	{
	
		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Approaching):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine approaching in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				GLOBALASSERT(0);
				if (marineIsNear) {
					state_result=Execute_MNS_Approach(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wander(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Firing):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponFireFunction)(sbPtr);
					SetMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PumpAction):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine pump action in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_PumpAction(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PumpAction(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicFire):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine panic firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponPanicFireFunction)(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Avoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine ");
					switch (marineStatusPointer->avoidanceManager.substate) {
						default:
						case AvSS_FreeMovement:
							PrintDebuggingText("Avoidance Level 0");
							break;
						case AvSS_FirstAvoidance:
							PrintDebuggingText("Avoidance Level 1");
							break;
						case AvSS_SecondAvoidance:
							PrintDebuggingText("Avoidance Level 2");
							break;
						case AvSS_ThirdAvoidance:
							PrintDebuggingText("Avoidance Level 3");
							break;
					}	
					PrintDebuggingText(" in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Avoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Avoidance(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Wandering):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine wandering in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wander(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Retreating):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine retreating in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				/* Real men never retreat! */
				if (marineIsNear) {
					state_result=Execute_MNS_Retreat(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Retreat(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Waiting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine waiting in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_SentryMode(sbPtr);
				}
				state_result=SRC_Request_Wait; /* Go back to sentry. */
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Responding):
			case(MBS_Sentry):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine sentry in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_SentryMode(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Dying):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine dying in %s\n",sbPtr->containingModule->name);
				}

				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			case(MBS_Taunting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine taunt in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Taunting(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Taunting(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Reloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Reloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Reloading(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicReloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine panic reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_PanicReloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PanicReloading(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_GetWeapon):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine get weapon in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_GetWeapon(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_GetWeapon(sbPtr);
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case MBS_Returning:
			case MBS_Pathfinding:
			{
				/* How the hell did you get here?!? */
				Marine_Enter_Wander_State(sbPtr);
				break;
			}
			case(MBS_AcidAvoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Guard marine acid avoidance in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_AcidAvoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					Marine_Enter_Wait_State(sbPtr);
					break;
				}
				GuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			default:
			{
				LOGDXFMT(("Guard marine in unsupported state %d!\n",marineStatusPointer->behaviourState));
				LOCALASSERT(1==0);
			}
		}	
	}

	if (!marineIsNear) {	

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void LocalGuardMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Firstly, fix missionmodule. */

	if (marineStatusPointer->missionmodule==NULL) {
		marineStatusPointer->missionmodule=sbPtr->containingModule->m_aimodule;
	}

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}
	
	{
	
		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Approaching):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine approaching in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Approach(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wander(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Firing):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponFireFunction)(sbPtr);
					SetMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PumpAction):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine pump action in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_PumpAction(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PumpAction(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicFire):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine panic firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponPanicFireFunction)(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Avoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine ");
					switch (marineStatusPointer->avoidanceManager.substate) {
						default:
						case AvSS_FreeMovement:
							PrintDebuggingText("Avoidance Level 0");
							break;
						case AvSS_FirstAvoidance:
							PrintDebuggingText("Avoidance Level 1");
							break;
						case AvSS_SecondAvoidance:
							PrintDebuggingText("Avoidance Level 2");
							break;
						case AvSS_ThirdAvoidance:
							PrintDebuggingText("Avoidance Level 3");
							break;
					}	
					PrintDebuggingText(" in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Avoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Avoidance(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Wandering):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine wandering in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wander(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Retreating):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine retreating in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				/* Real men never retreat! */
				if (marineIsNear) {
					state_result=Execute_MNS_Retreat(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Retreat(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Waiting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine waiting in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wait(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Responding):
			{
				/* Well, it *might* happen... */
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine responding in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Respond(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Respond(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Returning):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine returning in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Return(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Return(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Sentry):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine sentry in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Taunting):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine taunt in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Taunting(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Taunting(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Reloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Reloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Reloading(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicReloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine panic reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_PanicReloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PanicReloading(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Dying):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine dying in %s\n",sbPtr->containingModule->name);
				}

				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			case(MBS_GetWeapon):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine get weapon in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_GetWeapon(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_GetWeapon(sbPtr);
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			case MBS_Pathfinding:
			{
				/* How the hell did you get here?!? */
				Marine_Enter_Return_State(sbPtr);
				break;
			}
			case(MBS_AcidAvoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Local Guard marine acid avoidance in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_AcidAvoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					Marine_Enter_Wait_State(sbPtr);
					break;
				}
				LocalGuardMission_SwitchState(sbPtr,state_result);
				break;
			}
			default:
			{
				LOGDXFMT(("Local Guard marine in unsupported state %d!\n",marineStatusPointer->behaviourState));
				LOCALASSERT(1==0);
			}
		}	
	}

	if (!marineIsNear) {	

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void LoiterMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Fleeing Behaviour. */

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}

	{
	
		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Approaching):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant approaching in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Approach(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Firing):
			{
			
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					GLOBALASSERT(0);
					state_result=(*marineStatusPointer->My_Weapon->WeaponFireFunction)(sbPtr);
					SetMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PumpAction):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant marine pump action in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_PumpAction(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PumpAction(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicFire):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant panic firing in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=(*marineStatusPointer->My_Weapon->WeaponPanicFireFunction)(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Firing(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Avoidance):
			{
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant ");
					switch (marineStatusPointer->avoidanceManager.substate) {
						default:
						case AvSS_FreeMovement:
							PrintDebuggingText("Avoidance Level 0");
							break;
						case AvSS_FirstAvoidance:
							PrintDebuggingText("Avoidance Level 1");
							break;
						case AvSS_SecondAvoidance:
							PrintDebuggingText("Avoidance Level 2");
							break;
						case AvSS_ThirdAvoidance:
							PrintDebuggingText("Avoidance Level 3");
							break;
					}	
					PrintDebuggingText(" in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Avoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Avoidance(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Responding):
			case(MBS_Wandering):
			{
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant wandering in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wander(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wander(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Retreating):
			{
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant retreating in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				/* Wusses, now: they retreat. */
				if (marineIsNear) {
					state_result=Execute_MNS_Retreat(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Retreat(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Waiting):
			{
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant waiting in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_Wait(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Sentry):
			{
				GLOBALASSERT(0);
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant sentry in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_SentryMode(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MFS_Wait(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Reloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_Reloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_Reloading(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_PanicReloading):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant panic reloading in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}
	
				if(marineIsNear) {
					state_result=Execute_MNS_PanicReloading(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					state_result=Execute_MNS_PanicReloading(sbPtr);
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			case(MBS_Dying):
			{
				
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant dying in %s\n",sbPtr->containingModule->name);
				}

				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			case MBS_Returning:
			case MBS_Pathfinding:
			{
				/* How the hell did you get here?!? */
				Marine_Enter_Wander_State(sbPtr);
				break;
			}
			case(MBS_AcidAvoidance):
			{
				if ((ShowSquadState)||((marineIsNear)&&(ShowNearSquad))) {
					PrintDebuggingText("Noncombatant acid avoidance in %s: %d\n",sbPtr->containingModule->name,marineStatusPointer->Courage);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_AcidAvoidance(sbPtr);
					CentreMarineElevation(sbPtr);
				} else {
					Marine_Enter_Wait_State(sbPtr);
					break;
				}
				LoiterMission_SwitchState(sbPtr,state_result);
				break;
			}
			default:
			{
				/* NonComs can't taunt. */
				LOGDXFMT(("Marine in unsupported state %d!\n",marineStatusPointer->behaviourState));
				LOCALASSERT(1==0);
			}
		}	
	}

	if (!marineIsNear) {	

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void WanderMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result) {

	STATE_RETURN_CONDITION real_state_result;
	/* Experiment: override result? */
	switch (state_result) {
		case (SRC_Request_Fire):
		case (SRC_Request_Approach):
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
				real_state_result=SRC_Request_Retreat;
				EndMarineMuzzleFlash(sbPtr);
			} else {
				real_state_result=state_result;
			}
			break;
		default:
			real_state_result=state_result;
			break;
	}

	switch (real_state_result) {
		case (SRC_No_Change):
		{
			/* No action. */
			break;
		}
		case (SRC_Request_Taunt):
		{
			Marine_Enter_Taunt_State(sbPtr);
			break;
		}
		case (SRC_Request_Wait):
		{
			Marine_Enter_Wait_State(sbPtr);
			break;
		}
		case (SRC_Request_Fire):
		{
			Marine_Enter_Firing_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicFire):
		{
			Marine_Enter_PanicFire_State(sbPtr);
			break;
		}
		case (SRC_Request_Avoidance):
		{
			Marine_Enter_Avoidance_State(sbPtr);
			break;
		}
		case (SRC_Request_Approach):
		{
			Marine_Enter_Approach_State(sbPtr);
			break;
		}
		case (SRC_Request_Wander):
		{
			Marine_Enter_Wander_State(sbPtr);
			break;
		}
		case (SRC_Request_Retreat):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Respond):
		{
			Marine_Enter_Respond_State(sbPtr);
			break;
		}
		case (SRC_Request_Reload):
		{
			Marine_Enter_Reload_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicReload):
		{
			Marine_Enter_PanicReload_State(sbPtr);
			break;
		}
		case (SRC_Request_PumpAction):
		{
			Marine_Enter_PumpAction_State(sbPtr);
			break;
		}
		case (SRC_Request_PullPistol):
		{
			Marine_Enter_PullPistol_State(sbPtr);
			break;
		}
		default:
		{
			/* How did we end up here? */
			GLOBALASSERT(0);
			break;
		}

}

}

void PathfinderMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result) {

	STATE_RETURN_CONDITION real_state_result;
	/* Experiment: override result? */
	switch (state_result) {
		case (SRC_Request_Fire):
		case (SRC_Request_Approach):
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
				real_state_result=SRC_Request_Retreat;
				EndMarineMuzzleFlash(sbPtr);
			} else {
				real_state_result=state_result;
			}
			break;
		default:
			real_state_result=state_result;
			break;
	}

	switch (real_state_result) {
		case (SRC_No_Change):
		{
			/* No action. */
			break;
		}
		case (SRC_Request_Taunt):
		{
			Marine_Enter_Taunt_State(sbPtr);
			break;
		}
		case (SRC_Request_Wait):
		{
			Marine_Enter_Pathfinder_State(sbPtr);
			break;
		}
		case (SRC_Request_Fire):
		{
			Marine_Enter_Firing_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicFire):
		{
			Marine_Enter_PanicFire_State(sbPtr);
			break;
		}
		case (SRC_Request_Avoidance):
		{
			Marine_Enter_Avoidance_State(sbPtr);
			break;
		}
		case (SRC_Request_Approach):
		{
			Marine_Enter_Approach_State(sbPtr);
			break;
		}
		case (SRC_Request_Wander):
		{
			Marine_Enter_Wander_State(sbPtr);
			break;
		}
		case (SRC_Request_Retreat):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Respond):
		{
			Marine_Enter_Respond_State(sbPtr);
			break;
		}
		case (SRC_Request_Return):
		{
			Marine_Enter_Return_State(sbPtr);
			break;
		}
		case (SRC_Request_Reload):
		{
			Marine_Enter_Reload_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicReload):
		{
			Marine_Enter_PanicReload_State(sbPtr);
			break;
		}
		case (SRC_Request_PumpAction):
		{
			Marine_Enter_PumpAction_State(sbPtr);
			break;
		}
		case (SRC_Request_PullPistol):
		{
			Marine_Enter_PullPistol_State(sbPtr);
			break;
		}
		default:
		{
			/* How did we end up here? */
			GLOBALASSERT(0);
			break;
		}

}

}

void GuardMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result) {

	STATE_RETURN_CONDITION real_state_result;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Experiment: override result? */
	switch (state_result) {
		case (SRC_Request_Fire):
		case (SRC_Request_Approach):
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
				real_state_result=SRC_Request_PanicFire;
				EndMarineMuzzleFlash(sbPtr);
			} else {
				real_state_result=state_result;
			}
			break;
		default:
			real_state_result=state_result;
			break;
	}

	switch (real_state_result) {
		case (SRC_No_Change):
		{
			/* No action. */
			break;
		}
		case (SRC_Request_Taunt):
		{
			Marine_Enter_Taunt_State(sbPtr);
			break;
		}
		case (SRC_Request_Wait):
		{
			Marine_Enter_SentryMode_State(sbPtr);
			break;
		}
		case (SRC_Request_Fire):
		{
			Marine_Enter_Firing_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicFire):
		{
			Marine_Enter_PanicFire_State(sbPtr);
			break;
		}
		case (SRC_Request_Avoidance):
		{
			Marine_Enter_SentryMode_State(sbPtr);
			break;
		}
		case (SRC_Request_Approach):
		{
			Marine_Enter_SentryMode_State(sbPtr);
			break;
		}
		case (SRC_Request_Wander):
		{
			Marine_Enter_SentryMode_State(sbPtr);
			break;
		}
		case (SRC_Request_Retreat):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Reload):
		{
			Marine_Enter_Reload_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicReload):
		{
			Marine_Enter_PanicReload_State(sbPtr);
			break;
		}
		case (SRC_Request_PumpAction):
		{
			Marine_Enter_PumpAction_State(sbPtr);
			break;
		}
		case (SRC_Request_PullPistol):
		{
			Marine_Enter_PullPistol_State(sbPtr);
			break;
		}
		default:
		{
			/* How did we end up here? */
			GLOBALASSERT(0);
			break;
		}

}

}

void LocalGuardMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result) {

	STATE_RETURN_CONDITION real_state_result;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Experiment: override result? */
	switch (state_result) {
		case (SRC_Request_Fire):
		case (SRC_Request_Approach):
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
				real_state_result=SRC_Request_Retreat;
				EndMarineMuzzleFlash(sbPtr);
			} else {
				real_state_result=state_result;
			}
			break;
		default:
			real_state_result=state_result;
			break;
	}

	switch (real_state_result) {
		case (SRC_No_Change):
		{
			/* No action. */
			break;
		}
		case (SRC_Request_Taunt):
		{
			Marine_Enter_Taunt_State(sbPtr);
			break;
		}
		case (SRC_Request_Wait):
		{
			if (marineStatusPointer->missionmodule==sbPtr->containingModule->m_aimodule) {
				Marine_Enter_Wait_State(sbPtr);
			} else {
				Marine_Enter_Return_State(sbPtr);
			}
			break;
		}
		case (SRC_Request_Fire):
		{
			Marine_Enter_Firing_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicFire):
		{
			Marine_Enter_PanicFire_State(sbPtr);
			break;
		}
		case (SRC_Request_Avoidance):
		{
			Marine_Enter_Avoidance_State(sbPtr);
			break;
		}
		case (SRC_Request_Approach):
		{
			Marine_Enter_Approach_State(sbPtr);
			break;
		}
		case (SRC_Request_Wander):
		{
			Marine_Enter_Wander_State(sbPtr);
			break;
		}
		case (SRC_Request_Retreat):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Return):
		{
			Marine_Enter_Return_State(sbPtr);
			break;
		}
		case (SRC_Request_Reload):
		{
			Marine_Enter_Reload_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicReload):
		{
			Marine_Enter_PanicReload_State(sbPtr);
			break;
		}
		case (SRC_Request_PumpAction):
		{
			Marine_Enter_PumpAction_State(sbPtr);
			break;
		}
		case (SRC_Request_PullPistol):
		{
			Marine_Enter_PullPistol_State(sbPtr);
			break;
		}
		default:
		{
			/* How did we end up here? */
			GLOBALASSERT(0);
			break;
		}

}

}

void LoiterMission_SwitchState(STRATEGYBLOCK *sbPtr,STATE_RETURN_CONDITION state_result) {

	STATE_RETURN_CONDITION real_state_result;

	MARINE_STATUS_BLOCK *marineStatusPointer;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Experiment: override result? */
	switch (state_result) {
		case (SRC_Request_Fire):
		case (SRC_Request_Approach):
			if ((MarineRetreatsInTheFaceOfDanger(sbPtr))&&(marineStatusPointer->Target)) {
				real_state_result=SRC_Request_Retreat;
				EndMarineMuzzleFlash(sbPtr);
			} else {
				real_state_result=state_result;
			}
			break;
		default:
			real_state_result=state_result;
			break;
	}

	switch (real_state_result) {
		case (SRC_No_Change):
		{
			/* No action. */
			break;
		}
		case (SRC_Request_Taunt):
		{
			Marine_Enter_Taunt_State(sbPtr);
			break;
		}
		case (SRC_Request_Wait):
		{
			Marine_Enter_Wait_State(sbPtr);
			break;
		}
		case (SRC_Request_Fire):
		case (SRC_Request_Reload):
		case (SRC_Request_PanicReload):
		case (SRC_Request_PumpAction):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Avoidance):
		{
			Marine_Enter_Avoidance_State(sbPtr);
			break;
		}
		case (SRC_Request_Approach):
		{
			if (marineStatusPointer->Target) {
				/* Approach?  You must be mad! */
				Marine_Enter_Retreat_State(sbPtr);
			} else {
				/* We must be just suspicious. */
				Marine_Enter_Approach_State(sbPtr);
			}
			break;
		}
		case (SRC_Request_Retreat):
		{
			Marine_Enter_Retreat_State(sbPtr);
			break;
		}
		case (SRC_Request_Wander):
		{
			Marine_Enter_Wander_State(sbPtr);
			break;
		}
		case (SRC_Request_PanicFire):
		{
			Marine_Enter_PanicFire_State(sbPtr);
			break;
		}
		default:
		{
			/* How did we end up here? */
			GLOBALASSERT(0);
			break;
		}

}

}

void MakeMarineNear(STRATEGYBLOCK *sbPtr)
{
	extern MODULEMAPBLOCK AlienDefaultMap;

	MODULE tempModule;
	DISPLAYBLOCK *dPtr;
	DYNAMICSBLOCK *dynPtr;
	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->SBdptr == NULL);
	dynPtr = sbPtr->DynPtr;
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		
    LOCALASSERT(dynPtr);

	/* first of all, see how many marines are currently near: if there are too many,
	destroy this one, and try to force a generator to make a replacement */
	if(sbPtr->I_SBtype==I_BehaviourMarine)
	{
		if(NumGeneratorNPCsVisible() >= MAX_VISIBLEGENERATORNPCS)
		{
			DestroyAnyStrategyBlock(sbPtr);
			ForceAGenerator();
		}
	}

	AlienDefaultMap.MapShape = sbPtr->shapeIndex;
	tempModule.m_mapptr = &AlienDefaultMap;
	tempModule.m_sbptr = (STRATEGYBLOCK*)NULL;
	tempModule.m_numlights = 0;
	tempModule.m_lightarray = (struct lightblock *)0;
	tempModule.m_extraitemdata = (struct extraitemdata *)0;
	tempModule.m_dptr = NULL;
	AllocateModuleObject(&tempModule); 
	dPtr = tempModule.m_dptr;
	if(dPtr==NULL) return; /* cannot allocate displayblock, so leave far */
			
	sbPtr->SBdptr = dPtr;
	dPtr->ObStrategyBlock = sbPtr;
	dPtr->ObMyModule = NULL;					

	/* need to initialise positional information in the new display block */ 
	dPtr->ObWorld = dynPtr->Position;
	dPtr->ObEuler = dynPtr->OrientEuler;
	dPtr->ObMat = dynPtr->OrientMat;

	/* marine data block init */
	marineStatusPointer->weaponTarget.vx = marineStatusPointer->weaponTarget.vy = marineStatusPointer->weaponTarget.vz = 0;			

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* state and sequence initialisation */
	//dPtr->ShapeAnimControlBlock = &marineStatusPointer->ShpAnimCtrl;
	
	dPtr->HModelControlBlock=&marineStatusPointer->HModelController;

	/* Just in case. */
	CentreMarineElevation(sbPtr);
	InitWaypointManager(&marineStatusPointer->waypointManager);
	
	ProveHModel(dPtr->HModelControlBlock,dPtr);

	if(MarineShouldBeCrawling(sbPtr)) marineStatusPointer->IAmCrouched = 1;
	else marineStatusPointer->IAmCrouched = 0;

	if (marineStatusPointer->behaviourState==MBS_Firing) {
		Marine_Enter_Firing_State(sbPtr);
		/* To avoid negative volleys */
	}

	/*Copy extents from the collision extents in extents.c*/
	dPtr->ObMinX=-CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMaxX=CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMinZ=-CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMaxZ=CollisionExtents[CE_MARINE].CollisionRadius;
	dPtr->ObMinY=CollisionExtents[CE_MARINE].CrouchingTop;
	dPtr->ObMaxY=CollisionExtents[CE_MARINE].Bottom;
	dPtr->ObRadius = 1000;

	marineStatusPointer->gotapoint=0;

	/* Once they become near, Wait_Then_Hunt marines become Wander marines. */

	if (marineStatusPointer->Mission==MM_Wait_Then_Wander) {
		marineStatusPointer->Mission=MM_Wander;
	}

	/* And force pathfinders to get a new module? */
	if (marineStatusPointer->Mission==MM_Pathfinder) {
		if (marineStatusPointer->behaviourState==MBS_Pathfinding) {
			marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
		}
	}

	marineStatusPointer->lastframe_fallingspeed=-1;
}

void MakeMarineFar(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int i;
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->SBdptr != NULL);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* get rid of the displayblock */
	i = DestroyActiveObject(sbPtr->SBdptr);
	LOCALASSERT(i==0);
	sbPtr->SBdptr = NULL;

	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;

	/* zero linear velocity in dynamics block */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	
	/* if we have a gun flash, get rid of it */
	if(marineStatusPointer->myGunFlash)
	{
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* stop sound, if we have one */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle);	
	if(marineStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle2);	

	/* set the correct state(s) or remove, if we're dying */
	if(marineStatusPointer->behaviourState == MBS_Dying)
	{
	   /* if we're dying, we can be removed at this point */
	   DestroyAnyStrategyBlock(sbPtr);
	   return;
	}

}

void KillMarine(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming) {

	int deathtype,gibbFactor;
	int a;
	STRATEGYBLOCK *candidate;

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	SECTION_DATA *head;

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* Morale. */
	if (marineStatusPointer->My_Weapon->ARealMarine) {
		/* Only marine deaths get 'spotted' everywhere, by the APC guy! */
		NpcSquad.Nextframe_Squad_Delta_Morale-=10000;
		/* So, warn the squad? */
		ZoneAlert(3,sbPtr->containingModule->m_aimodule);
	}

	Marine_MuteVoice(sbPtr);

	/*notify death target ,if marine has one*/
	if(marineStatusPointer->death_target_sbptr)
	{
		RequestState(marineStatusPointer->death_target_sbptr,marineStatusPointer->death_target_request, 0);
	} 

	/* get rid of the gun flash, if we've got it */
	if(marineStatusPointer->myGunFlash)
	{
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}

	/* stop sound, if we have one, and it's not the fire */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		if (ActiveSounds[marineStatusPointer->soundHandle].soundIndex!=SID_FIRE) {
			Sound_Stop(marineStatusPointer->soundHandle);
		}
	}

	/* Set GibbFactor */
	gibbFactor=0;
	{
		int tkd;
		
		tkd=TotalKineticDamage(damage);
		deathtype=0;

		if (damage->ExplosivePower==1) {
			if (MUL_FIXED(tkd,(multiple&((ONE_FIXED<<1)-1)))>20) {
				/* Okay, you can gibb now. */
				marineStatusPointer->GibbFactor=ONE_FIXED>>1;
				marineStatusPointer->mtracker_timer=-1;
				deathtype=2;
			}
		} else if ((tkd>60)&&((multiple>>16)>1)) {
			int newmult;

			newmult=DIV_FIXED(multiple,NormalFrameTime);
			if (MUL_FIXED(tkd,newmult)>(500)) {
				/* Loadsabullets! */
				marineStatusPointer->GibbFactor=-(ONE_FIXED>>2);
				marineStatusPointer->mtracker_timer=-1;
				deathtype=2;
			}
		}

		if ((damage->ExplosivePower==2)||(damage->ExplosivePower==6)) {
			/* Basically SADARS only. */
			marineStatusPointer->GibbFactor=ONE_FIXED;
			marineStatusPointer->mtracker_timer=-1;
			deathtype=3;
		}
	}
	gibbFactor=marineStatusPointer->GibbFactor;

	if (damage->ForceBoom) {
		deathtype+=damage->ForceBoom;
	}

	{
		SECTION_DATA *chest;
		
		chest=GetThisSectionData(marineStatusPointer->HModelController.section_data,"chest");
		
		if (chest==NULL) {
			/* I'm impressed. */
			deathtype+=2;
		} else if ((chest->flags&section_data_notreal)
			&&(chest->flags&section_data_terminate_here)) {
			/* That's gotta hurt. */
			deathtype++;
		}
	}

	/* make a sound... if you have a head. */
	head=GetThisSectionData(marineStatusPointer->HModelController.section_data,"head");

	/* Is it still attached? */
	if (head) {
		if (head->flags&section_data_notreal) {
			head=NULL;
		}
	}
	
	if (marineStatusPointer->GibbFactor) {
		/* Probably want to make some sort of splatting noise... */
	} else if (head) {
		if (marineStatusPointer->Expression!=3) {
			/* Expression 3 just looks too peaceful. */
			if (marineStatusPointer->Mission==MM_RunAroundOnFire) {
				/* More burning screams. */
				Marine_BurningDeathScream(sbPtr);
			} else if ((damage->Impact==0) 		
				&&(damage->Cutting==0)  	
				&&(damage->Penetrative==0)
				&&(damage->Fire==0)
				&&(damage->Electrical>0)
				&&(damage->Acid==0)
				) {
				Marine_ElectrocutionScream(sbPtr);
			} else if ((Section)&&(damage->Id==AMMO_PRED_RIFLE)) {
				/* Hit in the chest or pelvis by a speargun? */
				if( (strcmp(Section->sempai->Section_Name,"pelvis")==0)
					||(strcmp(Section->sempai->Section_Name,"pelvis presley")==0)
					||(strcmp(Section->sempai->Section_Name,"chest")==0) ){
					Marine_OoophSound(sbPtr);
				} else {
					Marine_DeathScream(sbPtr);
				}
			} else {
				Marine_DeathScream(sbPtr);
			}
		}
	}
	
	/* Now final stage. */
	{
		DEATH_DATA *this_death;
		HIT_FACING facing;
		SECTION *root;
		int burning,electrical;

		root=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->TemplateName);
		
		facing.Front=0;
		facing.Back=0;
		facing.Left=0;
		facing.Right=0;
		
		if (incoming) {
			if (incoming->vz>0) {
				facing.Back=1;
			} else {
				facing.Front=1;
			}
			if (incoming->vx>0) {
				facing.Right=1;
			} else {
				facing.Left=1;
			}
		}
		
		if ((marineStatusPointer->Mission==MM_RunAroundOnFire)
			&&(damage->Impact==0) 		
			&&(damage->Cutting==0)  	
			&&(damage->Penetrative==0)
			&&(damage->Fire>0)
			&&(damage->Electrical==0)
			&&(damage->Acid==0)
			) {
			burning=1;
		} else {
			burning=0;
		}

		if ((damage->Impact==0) 		
			&&(damage->Cutting==0)  	
			&&(damage->Penetrative==0)
			&&(damage->Fire==0)
			&&(damage->Electrical>0)
			&&(damage->Acid==0)
			) {
			electrical=1;
		} else {
			electrical=0;
		}

		this_death=GetMarineDeathSequence(&marineStatusPointer->HModelController,root,marineStatusPointer->Wounds,marineStatusPointer->Wounds,
			deathtype,&facing,burning,marineStatusPointer->IAmCrouched,electrical);
		
		GLOBALASSERT(this_death);

		Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Elevation");
		Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		
		Convert_Marine_To_Corpse(sbPtr,this_death);
	}

	/* See if anyone saw that? */
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		GLOBALASSERT(candidate);

		if (candidate->I_SBtype==I_BehaviourMarine) {
			/* Are you already suspicious? */
			marineStatusPointer = (MARINE_STATUS_BLOCK *)(candidate->SBdataptr);

			/* Did you see that? */
			if (NPCCanSeeTarget(candidate,sbPtr,MARINE_NEAR_VIEW_WIDTH)) {

				if (marineStatusPointer->Android==0) {
					if (gibbFactor) {
						marineStatusPointer->Courage-=20000;
					} else if (head==NULL) {
						marineStatusPointer->Courage-=15000;
					} else {
						marineStatusPointer->Courage-=10000;
					}
				}

				if ((marineStatusPointer->suspicious==0)||(marineStatusPointer->using_squad_suspicion)) {
					/* Okay, react. */
					marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
					marineStatusPointer->suspect_point=sbPtr->DynPtr->Position;
					/* Set this to zero when you get a *new* suspicion. */
					marineStatusPointer->previous_suspicion=0;
					marineStatusPointer->using_squad_suspicion=0;
				}
			}
		}
	}
}

void MarineIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* if we're dying, do nothing */
	if(marineStatusPointer->behaviourState==MBS_Dying)
	{
		/* MFS should be dying, too */
		return;
	}				
	
	if(!(sbPtr->SBdptr))
	{
		DestroyAnyStrategyBlock(sbPtr);
		return;
	}

	marineStatusPointer->Wounds|=wounds;

	if (sbPtr->SBDamageBlock.Health > 0) {
	
		if (marineStatusPointer->Mission==MM_RunAroundOnFire) {
			Marine_BurningScream(sbPtr);
		} else if ((damage->Impact==0) 		
			&&(damage->Cutting==0)  	
			&&(damage->Penetrative==0)
			&&(damage->Fire==0)
			&&(damage->Electrical==0)
			&&(damage->Acid>0)
			) {
			Marine_AcidScream(sbPtr);
			if ((marineStatusPointer->behaviourState!=MBS_AcidAvoidance)
				&&(marineStatusPointer->behaviourState!=MBS_Firing)
				&&(marineStatusPointer->behaviourState!=MBS_Avoidance)
				&&(marineStatusPointer->behaviourState!=MBS_Dying)
				&&(marineStatusPointer->behaviourState!=MBS_PanicFire)
				&&(marineStatusPointer->behaviourState!=MBS_Reloading)
				&&(marineStatusPointer->behaviourState!=MBS_PumpAction)
				&&(marineStatusPointer->behaviourState!=MBS_GetWeapon)
				&&(marineStatusPointer->behaviourState!=MBS_PanicReloading))
			{
				Marine_Activate_AcidAvoidance_State(sbPtr,incoming);
			}
		} else {
			Marine_WoundedScream(sbPtr);
		}
		/* Open the mouth? */
		Marine_AssumePanicExpression(sbPtr);
	}
		
	/* Might want to get a new target? */

	marineStatusPointer->Target=NULL;

	if (marineStatusPointer->Android) {
		/* Kill non-functional androids. */
		if ((marineStatusPointer->Wounds&section_flag_left_hand)
			&&(marineStatusPointer->Wounds&section_flag_right_hand)) {
			sbPtr->SBDamageBlock.Health=0;
		}
	}

	/* reduce marine health */
	if(sbPtr->SBDamageBlock.Health <= 0) {
		/* marine experiences death */
		int dice=FastRandom()&65535;
		#if 0
		if ((dice<16384)||(marineStatusPointer->Android)) {
		#else
		if (marineStatusPointer->Android) {
		#endif
			Marine_SwitchExpression(sbPtr,3);
		} else if (dice<32768) {
			Marine_SwitchExpression(sbPtr,5);
		} else {
			Marine_SwitchExpression(sbPtr,4);
		}
		if (AvP.PlayerType!=I_Marine) {
			CurrentGameStats_CreatureKilled(sbPtr,Section);
		}
		KillMarine(sbPtr,damage,multiple,wounds,Section,incoming);
	} else {
		/* If not dead, play a hit delta. */
		DELTA_CONTROLLER *hitdelta;
		int frontback;

		hitdelta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
	
		if (incoming) {
			if (incoming->vz>=0) {
				frontback=0;
			} else {
				frontback=1;
			}
		} else {
			/* Default to front. */
			frontback=1;
		}

		if (hitdelta) {
			/* A hierarchy with hit deltas! */
			int CrouchSubSequence;
			int StandSubSequence;

			if (Section==NULL) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitChestBack;
					StandSubSequence=MSSS_HitChestBack;
				} else {
					CrouchSubSequence=MCrSS_HitChestFront;
					StandSubSequence=MSSS_HitChestFront;
				}
			} else if (Section->sempai->flags&section_flag_head) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitHeadBack;
					StandSubSequence=MSSS_HitHeadBack;
				} else {
					CrouchSubSequence=MCrSS_HitHeadFront;
					StandSubSequence=MSSS_HitHeadFront;
				}
			} else if ((Section->sempai->flags&section_flag_left_arm)
				||(Section->sempai->flags&section_flag_left_hand)) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitRightArm;
					StandSubSequence=MSSS_HitRightArm;
				} else {
					CrouchSubSequence=MCrSS_HitLeftArm;
					StandSubSequence=MSSS_HitLeftArm;
				}
			} else if ((Section->sempai->flags&section_flag_right_arm)
				||(Section->sempai->flags&section_flag_right_hand)) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitLeftArm;
					StandSubSequence=MSSS_HitLeftArm;
				} else {
					CrouchSubSequence=MCrSS_HitRightArm;
					StandSubSequence=MSSS_HitRightArm;
				}
			} else if ((Section->sempai->flags&section_flag_left_leg)
				||(Section->sempai->flags&section_flag_left_foot)) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitRightLeg;
					StandSubSequence=MSSS_HitRightLeg;
				} else {
					CrouchSubSequence=MCrSS_HitLeftLeg;
					StandSubSequence=MSSS_HitLeftLeg;
				}
			} else if ((Section->sempai->flags&section_flag_right_leg)
				||(Section->sempai->flags&section_flag_right_foot)) {
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitLeftLeg;
					StandSubSequence=MSSS_HitLeftLeg;
				} else {
					CrouchSubSequence=MCrSS_HitRightLeg;
					StandSubSequence=MSSS_HitRightLeg;
				}
			} else {
				/* Chest or misc. hit. */
				if (frontback==0) {
					CrouchSubSequence=MCrSS_HitChestBack;
					StandSubSequence=MSSS_HitChestBack;
				} else {
					CrouchSubSequence=MCrSS_HitChestFront;
					StandSubSequence=MSSS_HitChestFront;
				}
			}
			

			if(marineStatusPointer->IAmCrouched) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineCrouch,CrouchSubSequence)) {
					Start_Delta_Sequence(hitdelta,(int)HMSQT_MarineCrouch,CrouchSubSequence,-1); /* Was (ONE_FIXED>>2) */
				}
			} else {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,StandSubSequence)) {
					Start_Delta_Sequence(hitdelta,(int)HMSQT_MarineStand,StandSubSequence,-1); /* Was (ONE_FIXED>>2) */
				}
			}
			hitdelta->Playing=1;
			/* Not looped. */

		}

		/* Finally, warn the squad. */
		ZoneAlert(3,sbPtr->containingModule->m_aimodule);
		/* And become suspicious. */
		marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
		/* Set this to zero when you get a *new* suspicion. */
		marineStatusPointer->previous_suspicion=0;
		marineStatusPointer->using_squad_suspicion=0;
		if (incoming) {
			marineStatusPointer->suspect_point=*incoming;
			/* Flip it round! */
			marineStatusPointer->suspect_point.vx=-marineStatusPointer->suspect_point.vx;
			marineStatusPointer->suspect_point.vy=-marineStatusPointer->suspect_point.vy;
			marineStatusPointer->suspect_point.vz=-marineStatusPointer->suspect_point.vz;
			Normalise(&marineStatusPointer->suspect_point);
			marineStatusPointer->suspect_point.vx>>=5;
			marineStatusPointer->suspect_point.vy>>=5;
			marineStatusPointer->suspect_point.vz>>=5;
		} else {
			marineStatusPointer->suspect_point.vx=0;
			marineStatusPointer->suspect_point.vy=0;
			marineStatusPointer->suspect_point.vz=-2000;
		}
		RotateVector(&marineStatusPointer->suspect_point,&sbPtr->DynPtr->OrientMat);

		marineStatusPointer->suspect_point.vx+=sbPtr->DynPtr->Position.vx;
		marineStatusPointer->suspect_point.vy+=sbPtr->DynPtr->Position.vy;
		marineStatusPointer->suspect_point.vz+=sbPtr->DynPtr->Position.vz;

		/* Switch wounded androids into an appropriate state. */
		if (marineStatusPointer->Android) {
			if (marineStatusPointer->Wounds&section_flag_left_hand) {
				if (marineStatusPointer->My_Weapon->id!=MNPCW_AndroidSpecial) {
					Marine_Enter_OneArmShotgun_State(sbPtr);
				}
			} else if (marineStatusPointer->Wounds&section_flag_right_hand) {
				if (marineStatusPointer->My_Weapon->id!=MNPCW_Android_Pistol_Special) {
					Marine_Enter_OneArmPistol_State(sbPtr);
				}
			}

		}

	}

}



/*------------------------Patrick 24/2/97-----------------------------
  Marine far state behaviour functions
  --------------------------------------------------------------------*/

static STATE_RETURN_CONDITION Execute_MFS_Firing(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* I can't deal with this right now.  Better wait instead. */

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	return(SRC_Request_Wait);

}

static STATE_RETURN_CONDITION Execute_MFS_Avoidance(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* High on the list of Things Not To Be Doing. */

	#if ALL_NEW_AVOIDANCE
	Initialise_AvoidanceManager(sbPtr,&marineStatusPointer->avoidanceManager);
	#endif

	switch (marineStatusPointer->lastState) {
		case MBS_Retreating:
			return(SRC_Request_Retreat);
			break;
		case MBS_Returning:
			return(SRC_Request_Return);
			break;
		case MBS_Responding:
			return(SRC_Request_Respond);
			break;
		case MBS_Approaching:
			/* Go directly to approach.  Do not pass GO.  Do not collect 200 zorkmids. */
			return(SRC_Request_Approach);
			break;
		default:
			return(SRC_Request_Wander);
			break;
	}
	/* Still here? */
	return(SRC_Request_Wander);

}

static STATE_RETURN_CONDITION Execute_MFS_Wait(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule=0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->suspicious) {
		int correctlyOrientated;
		VECTORCH orientationDirn;
		
		orientationDirn.vx =  marineStatusPointer->suspect_point.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->suspect_point.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		marineStatusPointer->gotapoint=0;
	}

	/* Might want to spin on the spot. */
	if (marineStatusPointer->gotapoint==1) {

		VECTORCH orientationDirn;
		int correctlyOrientated;

		orientationDirn.vx = marineStatusPointer->wanderData.worldPosition.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->wanderData.worldPosition.vz - sbPtr->DynPtr->Position.vz;

		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,ONE_FIXED,NULL);
	
		if (correctlyOrientated) {
			marineStatusPointer->gotapoint=2;
			/* Done. */
		}

	} else if (marineStatusPointer->gotapoint==0) {
		GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);
	}


	/* See if you're allowed to respond. */

	if (
		(marineStatusPointer->Mission==MM_Wait_Then_Wander)
		||(marineStatusPointer->Mission==MM_Guard)
		) {
		return(SRC_No_Change);
	}

	/* Possible response?  LocalGuarders are allowed to wander. */

	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					NpcSquad.responseLevel--;
					return(SRC_Request_Respond);
				}
			}
		}
	}

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) return(SRC_No_Change);

	/* Might want to wander. */

	if ((FastRandom()&65535)<2048)
	{
		/* we should be wandering... we're bored of waiting. */
		return(SRC_Request_Wander);
	}

	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MFS_SentryMode(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Okay, so you're a Sentry who's been pushed off his spot. */

	if ((sbPtr->containingModule->m_aimodule==marineStatusPointer->missionmodule)
		||(marineStatusPointer->missionmodule==NULL)) {
	
		if (marineStatusPointer->missionmodule!=NULL) {
			int dist;
			VECTORCH offset;
			/* Relocate? */
				
			offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->my_spot.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->my_spot.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->my_spot.vz;
			/* Fix for midair start points, grrrr. */
			offset.vy>>=2;

			dist=Approximate3dMagnitude(&offset);

			if (dist>SENTRY_SENSITIVITY) {
				sbPtr->DynPtr->Position=marineStatusPointer->my_spot;
				sbPtr->containingModule = (ModuleFromPosition(&(sbPtr->DynPtr->Position), sbPtr->containingModule));

			}
		}

		if (marineStatusPointer->suspicious) {
			int correctlyOrientated;
			VECTORCH orientationDirn;
			
			orientationDirn.vx = marineStatusPointer->suspect_point.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->suspect_point.vz - sbPtr->DynPtr->Position.vz;
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
			marineStatusPointer->gotapoint=0;
		}
		
		/* Might want to spin on the spot. */
		if (marineStatusPointer->gotapoint==1) {
		
			VECTORCH orientationDirn;
			int correctlyOrientated;
		
			orientationDirn.vx = marineStatusPointer->wanderData.worldPosition.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->wanderData.worldPosition.vz - sbPtr->DynPtr->Position.vz;
		
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,ONE_FIXED,NULL);
		
			if (correctlyOrientated) {
				marineStatusPointer->gotapoint=2;
				/* Done. */
			}
		
		} else if (marineStatusPointer->gotapoint==0) {
			GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);
		}

	} else {

		/* Decrement the Far state timer */
		marineStatusPointer->stateTimer -= NormalFrameTime;	
		/* check if far state timer has timed-out. If so, it is time 
		to do something. Otherwise just return. */
		if(marineStatusPointer->stateTimer > 0) return(SRC_No_Change);

		/* Never engage, and ignore alerts. */				
		
		if (sbPtr->containingModule->m_aimodule==marineStatusPointer->missionmodule) {
			/* Same state next frame. */
			return(SRC_No_Change);
		}
		
		/* get the target module... */
		
		targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->missionmodule,7,0);
		
		/* If there is no target module, we're way out there.  Better wander a bit more. */
		if(!targetModule)
		{
			targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,marineStatusPointer->lastmodule,0);
		}
		/* Examine target, and decide what to do */
		GLOBALASSERT(AIModuleIsPhysical(targetModule));		
		ProcessFarMarineTargetModule(sbPtr, targetModule);	
		/* reset timer */
		marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	}
	return(SRC_No_Change);
}
#if 0
static STATE_RETURN_CONDITION Execute_MFS_Hunt(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) return(SRC_No_Change);
			
	/* check for state changes */
	
	if ((!MarineIsAwareOfTarget(sbPtr))
		||(marineStatusPointer->Target!=Player->ObStrategyBlock))
	{
		/* we should be wandering... can't hunt other NPCs */
		return(SRC_Request_Wander);
	}

	/* get the target module... */
	targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,0);

	/* if there is no target module, it means that the pred is trapped in an
	unlinked module. In this case, reset the timer and return. */			
	if(!targetModule)
	{
		marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;
		return(SRC_Request_Wait);
	}
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	return(SRC_No_Change);
}
#endif

static STATE_RETURN_CONDITION Execute_MFS_Approach(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;
	MODULE *tcm;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	if (!MarineIsAwareOfTarget(sbPtr))
	{
		/* we should be wandering... can't hunt other NPCs */
		return(SRC_Request_Wander);
	}

	/* See if we can fire? */

	if (marineStatusPointer->Target) {
		if (marineStatusPointer->Target->containingModule) {
			if (IsModuleVisibleFromModule(marineStatusPointer->Target->containingModule,sbPtr->containingModule)) {
				/* Take the shot? */
				return(SRC_Request_Fire);
			}
		}
	}

	/* Can't fire.  We want to get closer, then. */
	
	if (marineStatusPointer->Target->containingModule) {
		tcm=marineStatusPointer->Target->containingModule;
	} else {
		tcm=ModuleFromPosition(&marineStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
	}
	
	if (tcm) {		
		targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
	}

	if (targetModule) {
		/* We have somewhere to go. */
		GLOBALASSERT(AIModuleIsPhysical(targetModule));		
		ProcessFarMarineTargetModule(sbPtr, targetModule);	
		/* reset timer */
		marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;
		marineStatusPointer->destinationmodule=targetModule;
		return(SRC_No_Change);
	}

	/* Can't do nothin.  Better wait, then.  Everything else will see to itself. */
	if (marineStatusPointer->Mission==MM_Pathfinder) {
		return(SRC_Request_Return);
	} else {
		return(SRC_Request_Wait);
	}
}


static STATE_RETURN_CONDITION Execute_MFS_Respond(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	
	if (ShowSquadState) {
		if (marineStatusPointer->destinationmodule==NULL) {
			PrintDebuggingText("Target module is NULL\n");
		} else {
			PrintDebuggingText("Target module is %s\n",(*(marineStatusPointer->destinationmodule->m_module_ptrs))->name);
		}
	}

	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) {
		return(SRC_No_Change);
	}
			
	/* check for state changes */
	
	if (MarineIsAwareOfTarget(sbPtr)) {
		/* Picked up a target. */
		return(SRC_Request_Approach);
	}

	/* get the target module... */
	targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);

	if (targetModule==sbPtr->containingModule->m_aimodule) {
		/* We've arrived. */
		DeprioritiseAlert(sbPtr->containingModule->m_aimodule);
		/* Hey, if it's real, there'll be a new one soon enough. */
		return(SRC_Request_Wait);
	}

	/* if there is no target module, it means that the pred is trapped in an
	unlinked module. In this case, reset the timer and return. */			
	if(!targetModule)
	{
		/* We can't do it. */
		return(SRC_Request_Wait);
	}
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	marineStatusPointer->destinationmodule=targetModule;
	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MFS_Wander(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) {
		return(SRC_No_Change);
	}
			
	/* check for state changes */
	if(MarineIsAwareOfTarget(sbPtr))
	/* Hack! */
	{
		/* we should be hunting */
		return(SRC_Request_Approach);
	}

	/* New alert? */
	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					NpcSquad.responseLevel--;
					return(SRC_Request_Respond);
				}
			}
		}
	}

	/* Bored of wandering?  How about we wait a while? */
	if ((FastRandom()&65535)<2048)
	{
		/* we should be wandering... we're bored of waiting. */
		return(SRC_Request_Wait);
	}

	/* get the target module... */
	targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,marineStatusPointer->lastmodule,0);

	/* if there is no target module, it means that the pred is trapped in an
	unlinked module. In this case, reset the timer and return. */			
	if(!targetModule)
	{
		marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;
		return(SRC_No_Change);		
	}
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MFS_Return(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Okay, so you're a LocalGuard or Pathfinder who's gotten lost. */

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) return(SRC_No_Change);
			
	/* check for state changes */
	if(MarineIsAwareOfTarget(sbPtr))
	/* Hack! */
	{
		/* we should be engaging */
		return(SRC_Request_Approach);
	}

	/* Ignore alerts? */
	#if 0
	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					NpcSquad.responseLevel--;
					return(SRC_Request_Respond);
				}
			}
		}
	}
	#endif

	/* Never break out of return unless your life is in danger! */

	/* Or unless we're back. */

	if (sbPtr->containingModule->m_aimodule==marineStatusPointer->missionmodule) {
		return(SRC_Request_Wait);
	}

	/* get the target module... */
	
	targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->missionmodule,7,0);

	/* If there is no target module, we're way out there.  Better wander a bit more. */
	if(!targetModule)
	{
		targetModule = FarNPC_GetTargetAIModuleForWander(sbPtr,marineStatusPointer->lastmodule,0);
	}
	/* Examine target, and decide what to do */
	if (AIModuleIsPhysical(targetModule)==0) {
		#if 0
		/* No longer a straight assert: not it's breakpointable. */
		GLOBALASSERT(0);
		#else
		/* We're probably fubared.  Change to Wander. */
		if (marineStatusPointer->Mission!=MM_NonCom) {
			/* Dunno what a NonCom is doing here, but Code Defensively! */
			marineStatusPointer->Mission=MM_Wander;
		}
		return(SRC_Request_Wait);
		#endif
	}
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MFS_Pathfinder(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;
	int nextModuleIndex;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Okay, so you're a LocalGuard or Pathfinder who's gotten lost. */

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;	
	/* check if far state timer has timed-out. If so, it is time 
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) return(SRC_No_Change);
			
	/* check for state changes */
	if(MarineIsAwareOfTarget(sbPtr))
	/* Hack! */
	{
		/* we should be engaging */
		return(SRC_Request_Approach);
	}

	/* Ignore alerts. */

	/* Never break out of pathfinder unless your life is in danger! */

	/* Okay, so where are we exactly? */

	if ((marineStatusPointer->stepnumber<0)||(marineStatusPointer->path<0)) {
		/* Get OUT! */
		return(SRC_Request_Wander);
	}

	targetModule = TranslatePathIndex(marineStatusPointer->stepnumber,marineStatusPointer->path);

	if (targetModule==NULL) {
		/* Oh, to heck with this.  Try to wander. */
		return(SRC_Request_Wander);
	}
	
	/* Right, so there is a somewhere to get to. */

	if (targetModule!=sbPtr->containingModule->m_aimodule) {
		/* But we're nowhere near it.  Geeze... */
		marineStatusPointer->missionmodule=targetModule;
		return(SRC_Request_Return);
	}
	
	/* Okay, so now we need to know where to go now. */

	nextModuleIndex=GetNextModuleInPath(marineStatusPointer->stepnumber,marineStatusPointer->path);
	GLOBALASSERT(nextModuleIndex>=0);
	/* If that fires, it's Richard's fault. */
	targetModule=TranslatePathIndex(nextModuleIndex,marineStatusPointer->path);
	GLOBALASSERT(targetModule);
	/* Ditto. */
	marineStatusPointer->stepnumber=nextModuleIndex;

	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MFS_Retreat(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	AIMODULE *targetModule = 0;
	AIMODULE *old_fearmod;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Decrement the Far state timer */
	marineStatusPointer->stateTimer -= (NormalFrameTime<<1);
	/* Double speed, remember? */
	
	/* check if far state timer has timed-out. If so, it is time
	to do something. Otherwise just return. */
	if(marineStatusPointer->stateTimer > 0) {
		return(SRC_No_Change);
	}
			
	old_fearmod=marineStatusPointer->fearmodule;
	
	/* From where am I running? */
	if(MarineIsAwareOfTarget(sbPtr)) {
		marineStatusPointer->fearmodule=marineStatusPointer->Target->containingModule->m_aimodule;
	} else if (marineStatusPointer->fearmodule==NULL) {
		marineStatusPointer->fearmodule=sbPtr->containingModule->m_aimodule;
	}	
	
	if (marineStatusPointer->fearmodule!=old_fearmod) {
		marineStatusPointer->destinationmodule = General_GetAIModuleForRetreat(sbPtr,marineStatusPointer->fearmodule,5);
	}

	targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->destinationmodule,6,0);

	if(!targetModule)
	{
		/* We can't do it. */
		return(SRC_Request_Wait);
	}
	/* Examine target, and decide what to do */
	GLOBALASSERT(AIModuleIsPhysical(targetModule));		
	ProcessFarMarineTargetModule(sbPtr, targetModule);	
	/* reset timer */
	marineStatusPointer->stateTimer = MARINE_FAR_MOVE_TIME;					
	marineStatusPointer->destinationmodule=targetModule;
	return(SRC_No_Change);

}

static void ProcessFarMarineTargetModule(STRATEGYBLOCK *sbPtr, AIMODULE* targetModule)
{
	NPC_TARGETMODULESTATUS targetStatus;
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	
	LOCALASSERT(sbPtr);
	LOCALASSERT(targetModule);
	LOCALASSERT(sbPtr->I_SBtype == I_BehaviourMarine);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);	    
	LOCALASSERT(marineStatusPointer);

	targetStatus = GetTargetAIModuleStatus(sbPtr, targetModule,0);
	switch(targetStatus)
	{
		case(NPCTM_NoEntryPoint):
		{
			/* do nothing */
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_NormalRoom):
		{
			/* locate to target	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_AirDuct):
		{
			/* loacate to target */
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_LiftTeleport):
		{
			/* do nothing */
			FarNpc_FlipAround(sbPtr);
			break;
		}
		case(NPCTM_ProxDoorOpen):
		{
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}		
		case(NPCTM_ProxDoorNotOpen):
		{
			MODULE *renderModule;
			renderModule=*(targetModule->m_module_ptrs);
			/* trigger the door, and set timer to quick so we can catch the door when it's open */
			((PROXDOOR_BEHAV_BLOCK *)renderModule->m_sbptr->SBdataptr)->marineTrigger = 1;
			break;
		}
		case(NPCTM_LiftDoorOpen):
		{
			///* do nothing - can't use lifts	*/
			//FarNpc_FlipAround(sbPtr);
			/* What the hell!!! */
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_LiftDoorNotOpen):
		{
		   	/*  do nothing - can't open lift doors */
			FarNpc_FlipAround(sbPtr);
		   	break;
		}
		case(NPCTM_SecurityDoorOpen):
		{
			/* locate to target, and move thro' quick as we can't retrigger	*/
			LocateFarNPCInAIModule(sbPtr, targetModule);
			break;
		}
		case(NPCTM_SecurityDoorNotOpen):
		{
			MODULE *renderModule;
			renderModule=*(targetModule->m_module_ptrs);
			/* do some door opening stuff here. Door should stay open for long enough
			for us to catch it open next time */
			RequestState((renderModule->m_sbptr),1,0);
		   	break;
		}
		default:
		{
			LOCALASSERT(1==0);
		}
	}		
}


/*------------------------Patrick 24/2/97-----------------------------
  Marine near state behaviour functions
  --------------------------------------------------------------------*/

void Marine_Enter_SentryMode_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Sentry;
	marineStatusPointer->stateTimer = ONE_FIXED; /* Ignored anyway... */

	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		if (marineStatusPointer->suspicious) {
			if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_Wait_Alert)) {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Wait_Alert,(ONE_FIXED<<2),(ONE_FIXED>>3));
			} else {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
			}
		} else {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleWaitingAnimations(sbPtr);
	#endif

	GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);

}

void Marine_Enter_Wait_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Waiting;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Stand_To_Fidget)) {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Stand_To_Fidget,((ONE_FIXED*3)/2),(ONE_FIXED>>3));
			marineStatusPointer->HModelController.LoopAfterTweening=0;
		} else {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleWaitingAnimations(sbPtr);
	#endif

	GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);

}

void Marine_Enter_Firing_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;
	range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Firing;
	marineStatusPointer->volleySize = 0;
	
	if (marineStatusPointer->My_Weapon->id!=MNPCW_MPistol) {
		marineStatusPointer->lastroundhit=0;
	}
	if ((marineStatusPointer->My_Weapon->id==MNPCW_Flamethrower)||
		(marineStatusPointer->My_Weapon->id==MNPCW_MFlamer)) {
		marineStatusPointer->weapon_variable=0;
	}

	marineStatusPointer->lasthitsection=NULL;
	marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
	GLOBALASSERT(marineStatusPointer->Target);
	NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	/* Arbitrarily decide to crouch? */
	if (marineStatusPointer->Android) {
		marineStatusPointer->IAmCrouched=0;
	} else {
		int prob;

		prob=20000;

		if (range<6000) {
			prob+=10000;
		}

		if ((marineStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy)>3000) {
			prob+=20000;
		}
		
		if ((FastRandom()&65535)<prob) {
			marineStatusPointer->IAmCrouched=1;
		}
	}

	if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_FireFromHips)) {
		int target;

		target=(marineStatusPointer->Courage>>1);
		if (marineStatusPointer->Mission==MM_Guard) {
			target+=32767;
		}

		if ((FastRandom()&((ONE_FIXED<<1)-1))>target) {
			marineStatusPointer->FiringAnim=1;
		} else {
			marineStatusPointer->FiringAnim=0;
		}
	} else {
		marineStatusPointer->FiringAnim=0;
	}

	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCrSS_Attack_Primary,-1,(ONE_FIXED>>3));
	} else {
		if (marineStatusPointer->FiringAnim==1) {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_FireFromHips,-1,(ONE_FIXED>>3));
		} else {
			if (marineStatusPointer->My_Weapon->id!=MNPCW_TwoPistols) {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Attack_Primary,-1,(ONE_FIXED>>3));
			} else {
				/* Two Pistols uses Stand Standard. */
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,-1,(ONE_FIXED>>3));
			}
		}
	}

	if (marineStatusPointer->My_Weapon->id==MNPCW_PulseRifle) {
		if ((marineStatusPointer->FiringAnim==0)&&(marineStatusPointer->IAmCrouched==0)) {
			/* Wink 2. */
			Marine_QueueWink2Expression(sbPtr);
		} else {
			Marine_QueueGrimaceExpression(sbPtr);
		}
	} else {
		Marine_QueueGrimaceExpression(sbPtr);
	}

	/* This next for firing only! */
	marineStatusPointer->HModelController.StopAfterTweening=1;
	
	if (marineStatusPointer->My_Weapon->id==MNPCW_GrenadeLauncher) {
		/* Why do we need internalState 1 here again? */
		marineStatusPointer->internalState=1;
		marineStatusPointer->HModelController.Looped=0;
		marineStatusPointer->HModelController.LoopAfterTweening=0;
		
		/* Put loft in now? */
		{
			int range;
			range=VectorDistance((&marineStatusPointer->weaponTarget),(&sbPtr->DynPtr->Position));

			marineStatusPointer->weaponTarget.vy-=(range/8);
		}
	}

	if ((marineStatusPointer->My_Weapon->id==MNPCW_MShotgun)
		||(marineStatusPointer->My_Weapon->id==MNPCW_Android)
		||(marineStatusPointer->My_Weapon->id==MNPCW_AndroidSpecial)) {
		marineStatusPointer->internalState=1;
		marineStatusPointer->HModelController.Looped=0;
		marineStatusPointer->HModelController.LoopAfterTweening=0;
	}

}

void Marine_Enter_Avoidance_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* Make sure obstruction is set! */

	marineStatusPointer->gotapoint=0;

	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	NPCGetAvoidanceDirection(sbPtr, &(marineStatusPointer->moveData.avoidanceDirn),&marineStatusPointer->obstruction);						
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Avoidance;  		
	marineStatusPointer->stateTimer = NPC_AVOIDTIME;

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

	/* Don't interfere with expression... */

}

void Marine_Enter_Wander_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	NPC_InitMovementData(&(marineStatusPointer->moveData));
	NPC_InitWanderData(&(marineStatusPointer->wanderData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Wandering;
	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 					

	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_Mooch_Bored)) {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Mooch_Bored,ONE_FIXED,(ONE_FIXED>>3));
		} else {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

}

void Marine_Enter_Approach_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Approaching;

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	/* Neutral??? */
	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		if (marineStatusPointer->Target==NULL) {
			if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_Mooch_Alert)) {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Mooch_Alert,ONE_FIXED,(ONE_FIXED>>3));
			} else {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
			}
		} else {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

	marineStatusPointer->destinationmodule=NULL;

}

void Marine_Enter_Hunt_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Wandering; /* CHANGE ME!!! */

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	/* This really shouldn't be called, should it? */
	Marine_QueueNeutralExpression(sbPtr);

	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}

}

void Marine_Enter_Respond_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Responding;
	
	if (NpcSquad.responseLevel) {
		NpcSquad.responseLevel--;
	}

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	/* Determined! */
	Marine_QueueNeutralExpression(sbPtr);

	/* We must now be suspicious. */
	marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
	marineStatusPointer->suspect_point=NpcSquad.squad_suspect_point;
	/* That'll do as a default. */
	if (NpcSquad.alertZone) {
		/* Gotta have a point. */
		marineStatusPointer->suspect_point=NpcSquad.alertZone->m_world;
	}
	/* Set this to zero when you get a *new* suspicion. */
	marineStatusPointer->previous_suspicion=0;
	marineStatusPointer->using_squad_suspicion=1;

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

}

void Marine_Enter_Return_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Returning;
	
	marineStatusPointer->destinationmodule=NULL;

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

}

void Marine_Enter_Pathfinder_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Pathfinding;
	
	marineStatusPointer->destinationmodule=NULL;

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	Marine_QueueNeutralExpression(sbPtr);

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

}

void Marine_Enter_Retreat_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	marineStatusPointer->volleySize = 0;
	NPC_InitMovementData(&(marineStatusPointer->moveData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Retreating;

	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;			 		

	marineStatusPointer->fearmodule=NULL;
	/* It'll get set on state execution. */

	if ((FastRandom()&65535)<32767) {
		Marine_QueuePanicExpression(sbPtr);
		Marine_PanicScream(sbPtr);
	} else {
		Marine_QueueGrimaceExpression(sbPtr);
	}

	#if 0
	if(marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));		
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
	}
	#endif
	/* Actually, sequence change is done in the function now. */

}

void Marine_Enter_Taunt_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Taunt_One)) {

		marineStatusPointer->gotapoint=0;
		marineStatusPointer->internalState=0;
		marineStatusPointer->volleySize = 0;
		marineStatusPointer->lastState=marineStatusPointer->behaviourState;
		marineStatusPointer->behaviourState = MBS_Taunting;
		marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Taunt_One,-1,(ONE_FIXED>>3));
		marineStatusPointer->HModelController.LoopAfterTweening=0;
	} else {
		/* Aw, forget it. */
		Marine_Enter_Wait_State(sbPtr);
		return;
	}

	if ((FastRandom()&65535)<32767) {
		Marine_QueueWink1Expression(sbPtr);
	}

	Marine_TauntShout(sbPtr);

}

void Marine_Enter_Reload_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,marineStatusPointer->My_Weapon->Reload_Sequence));
	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Reloading;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	/* Dunno if this is right. */
	Marine_QueueGrimaceExpression(sbPtr);

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,marineStatusPointer->My_Weapon->Reload_Sequence,-1,(ONE_FIXED>>3));
	marineStatusPointer->HModelController.LoopAfterTweening=0;
	
}

void Marine_Enter_PanicReload_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,marineStatusPointer->My_Weapon->Reload_Sequence));
	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_PanicReloading;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	if ((FastRandom()&65535)<32767) {
		Marine_QueuePanicExpression(sbPtr);
		if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
			Marine_PanicScream(sbPtr);
		} else {
			Marine_AngryScream(sbPtr);
		}
	} else {
		Marine_QueueGrimaceExpression(sbPtr);
	}

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,MSSS_Panic_Reload)) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Panic_Reload,-1,(ONE_FIXED>>3));
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,marineStatusPointer->My_Weapon->Reload_Sequence,-1,(ONE_FIXED>>3));
	}
	marineStatusPointer->HModelController.LoopAfterTweening=0;
	
}

void Marine_Enter_PumpAction_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,MSSS_PumpAction));

	/* Maintain many things from fire. */	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_PumpAction;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	/* Dunno if this is right. */
	Marine_QueueGrimaceExpression(sbPtr);

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	if (marineStatusPointer->IAmCrouched) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCrSS_PumpAction,-1,(ONE_FIXED>>5));
		marineStatusPointer->HModelController.LoopAfterTweening=0;
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_PumpAction,-1,(ONE_FIXED>>5));
		marineStatusPointer->HModelController.LoopAfterTweening=0;
	}
	
}

void Marine_Enter_PanicFire_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	if (marineStatusPointer->My_Weapon->id==MNPCW_SADAR) {
		#if 0
		if (marineStatusPointer->Target) {
			Marine_Enter_Firing_State(sbPtr);
			return;
		} else {
			Marine_Enter_Wait_State(sbPtr);
			return;
		}
		#else
		Marine_Enter_PullPistol_State(sbPtr);
		return;
		#endif
	} else if (marineStatusPointer->My_Weapon->id==MNPCW_Skeeter) {
		Marine_Enter_PullPistol_State(sbPtr);
		return;
	} else if (marineStatusPointer->My_Weapon->id==MNPCW_MMolotov) {
		if (marineStatusPointer->Target) {
			Marine_Enter_Firing_State(sbPtr);
			return;
		} else {
			Marine_Enter_Wait_State(sbPtr);
			return;
		}
	}

	marineStatusPointer->internalState=0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_PanicFire;
	marineStatusPointer->volleySize = 0;

	if (marineStatusPointer->My_Weapon->id!=MNPCW_MPistol) {
		marineStatusPointer->lastroundhit=0;
	}
	if ((marineStatusPointer->My_Weapon->id==MNPCW_Flamethrower)||
		(marineStatusPointer->My_Weapon->id==MNPCW_MFlamer)) {
		marineStatusPointer->weapon_variable=0;
	}

	marineStatusPointer->lasthitsection=NULL;
	marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
	if (marineStatusPointer->Target) {
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
	}

	if ((FastRandom()&65535)<32767) {
		Marine_QueuePanicExpression(sbPtr);
		if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
			Marine_PanicScream(sbPtr);
		} else {
			Marine_AngryScream(sbPtr);
		}
	} else {
		Marine_QueueGrimaceExpression(sbPtr);
	}

	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_WildFire_0)) {
		/* *can* enter wild fire... */
		#if 0
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		marineStatusPointer->HModelController.LoopAfterTweening=1;
		#endif
		/* Sequence will be set in the function. */
	} else {
		/* Aw, forget it. */
		Marine_Enter_Retreat_State(sbPtr);
		marineStatusPointer->suspicious=MARINE_PANIC_TIME;
	}

}

void Marine_Enter_PullPistol_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	SECTION *root;
	MARINE_WEAPON_DATA *pistol_data;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(marineStatusPointer->My_Weapon->ARealMarine);
	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_GetWeapon;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
		/* ...and NO Minigun delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		}
		/* ...and strip out HitDelta for now, too. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		}
	}

	/* Now, try to be clever... */
	/* Turn into a pistol guy! */
	pistol_data=GetThisNPCMarineWeapon(MNPCW_PistolMarine);
	GLOBALASSERT(pistol_data);

	root=GetNamedHierarchyFromLibrary(pistol_data->Riffname,pistol_data->HierarchyName);
	GLOBALASSERT(root);

	marineStatusPointer->HModelController.Sequence_Type=HMSQT_MarineStand;
	marineStatusPointer->HModelController.Sub_Sequence=MSSS_Get_Weapon;
	/* That's to put the pistol in the right place... */
	Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root, 1, 1,0);
	marineStatusPointer->My_Weapon=pistol_data;
	marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
	marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
	/* Start loaded! */
	marineStatusPointer->clipammo=marineStatusPointer->My_Weapon->clip_size;

	/* Dunno if this is right. */
	Marine_QueueGrimaceExpression(sbPtr);

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Get_Weapon,-1,(ONE_FIXED>>3));
	marineStatusPointer->HModelController.LoopAfterTweening=0;

	/* Attempt to put the hitdelta back? */
	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
		DELTA_CONTROLLER *delta;
		delta=Add_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
		GLOBALASSERT(delta);
		delta->Playing=0;
	}

	DeInitialise_HModel(&marineStatusPointer->HModelController);
	ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	
}

void Marine_Enter_OneArmShotgun_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	SECTION *root;
	MARINE_WEAPON_DATA *pistol_data;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(marineStatusPointer->Android);
	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_Waiting;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
		/* ...and NO Minigun delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		}
		/* ...and strip out HitDelta for now, too. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		}
	}

	/* Now, try to be clever... */
	/* Turn into an Android Special!  */
	pistol_data=GetThisNPCMarineWeapon(MNPCW_AndroidSpecial);
	GLOBALASSERT(pistol_data);

	root=GetNamedHierarchyFromLibrary(pistol_data->Riffname,pistol_data->HierarchyName);
	GLOBALASSERT(root);

	marineStatusPointer->HModelController.Sequence_Type=HMSQT_MarineStand;
	marineStatusPointer->HModelController.Sub_Sequence=MSSS_Standard;
	/* That's to put the pistol in the right place... */
	Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root, 1, 1,0);
	marineStatusPointer->My_Weapon=pistol_data;
	marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
	marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
	
	/* Retain clipammo from the old shotgun. */

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,-1,(ONE_FIXED>>3));
	marineStatusPointer->HModelController.LoopAfterTweening=0;

	/* Attempt to put the hitdelta back? */
	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
		DELTA_CONTROLLER *delta;
		delta=Add_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
		GLOBALASSERT(delta);
		delta->Playing=0;
	}

	DeInitialise_HModel(&marineStatusPointer->HModelController);
	ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	
}

void Marine_Enter_OneArmPistol_State(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	SECTION *root;
	MARINE_WEAPON_DATA *pistol_data;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(marineStatusPointer->Android);
	
	marineStatusPointer->gotapoint=0;
	marineStatusPointer->internalState=0;
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_GetWeapon;
	marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;

	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
		/* ...and NO Minigun delta... */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
		}
		/* ...and strip out HitDelta for now, too. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		}
	}

	/* Now, try to be clever... */
	/* Turn into an Android Special!  */
	pistol_data=GetThisNPCMarineWeapon(MNPCW_Android_Pistol_Special);
	GLOBALASSERT(pistol_data);

	root=GetNamedHierarchyFromLibrary(pistol_data->Riffname,pistol_data->HierarchyName);
	GLOBALASSERT(root);

	marineStatusPointer->HModelController.Sequence_Type=HMSQT_MarineStand;
	marineStatusPointer->HModelController.Sub_Sequence=MSSS_Get_Weapon;
	/* That's to put the pistol in the right place... */
	Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root, 1, 1,0);
	marineStatusPointer->My_Weapon=pistol_data;
	marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
	marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
	/* Start loaded! */
	marineStatusPointer->clipammo=marineStatusPointer->My_Weapon->clip_size;

	/* Remove the gunflash */
	if(marineStatusPointer->myGunFlash) 
	{	
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}
	/* .... and stop the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);		
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Get_Weapon,-1,(ONE_FIXED>>3));
	marineStatusPointer->HModelController.LoopAfterTweening=0;

	/* Attempt to put the hitdelta back? */
	if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
		DELTA_CONTROLLER *delta;
		delta=Add_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
		GLOBALASSERT(delta);
		delta->Playing=0;
	}

	DeInitialise_HModel(&marineStatusPointer->HModelController);
	ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	
}

static STATE_RETURN_CONDITION Execute_MNS_Approach(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH velocityDirection = {0,0,0};
	VECTORCH targetPosition;
	int targetIsAirduct = 0;
	int range;
	
	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	HandleMovingAnimations(sbPtr);

	/* now check for state changes... firstly, if we can no longer attack the target, go
	to wander */
	if(!(MarineIsAwareOfTarget(sbPtr)))
	{
		if (marineStatusPointer->suspicious==0) {
			/* Return to wait.  Nothing to worry about. */
			return(SRC_Request_Wait);
		}
	
	} else {
		
		/* We have a target that we are aware of. */

		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
		
		/* if we are close... go directly to firing */
		if(range < marineStatusPointer->My_Weapon->ForceFireRange)
		{	
			/* switch directly to firing, at this distance */
		
			return(SRC_Request_Fire);
		}
		
		/* if our state timer has run out in approach state, see if we can fire*/
		if(marineStatusPointer->stateTimer > 0) marineStatusPointer->stateTimer -= NormalFrameTime;
		if(marineStatusPointer->stateTimer <= 0)
		{
			/* it is time to fire, if we can see the target  */
   			if((MarineCanSeeTarget(sbPtr))
			   &&((marineStatusPointer->My_Weapon->MaxRange==-1) 
			   ||(range<marineStatusPointer->My_Weapon->MaxRange))) {

				/* we are going to fire then */		
		
				return(SRC_Request_Fire);  		
			}
			else
			{
			   	/* renew approach state */
			   	marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;
		
			}
		}
	}

	/* Kick them out of the stupid state? */

	/* See which way we want to go. */
	if ((marineStatusPointer->destinationmodule==sbPtr->containingModule->m_aimodule)
		||(marineStatusPointer->destinationmodule==NULL)) {

		AIMODULE *targetModule;
		MODULE *tcm;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

		if (marineStatusPointer->Target==NULL) {
			/* Must be approaching a suspect point. */
			GLOBALASSERT(marineStatusPointer->suspicious);
			tcm=ModuleFromPosition(&marineStatusPointer->suspect_point,sbPtr->containingModule);
		} else {
			if (marineStatusPointer->Target->containingModule) {
				tcm=marineStatusPointer->Target->containingModule;
			} else {
				tcm=ModuleFromPosition(&marineStatusPointer->Target->DynPtr->Position,sbPtr->containingModule);
			}
		}

		if (tcm) {		
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,tcm->m_aimodule,7,0);
		} else {
			targetModule=NULL;
		}

		if (targetModule==sbPtr->containingModule->m_aimodule) {
			/* Try going for it, we still can't see them. */
			if (marineStatusPointer->Target) {
				NPCGetMovementTarget(sbPtr, marineStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
			} else {
				int range2;
				VECTORCH relTargetPosition;
				/* Just use the target point? */
				targetPosition=marineStatusPointer->suspect_point;
	
				#if 0
				range2=VectorDistance(&targetPosition,(&sbPtr->DynPtr->Position));
				#else

				relTargetPosition=targetPosition;

				relTargetPosition.vx-=sbPtr->DynPtr->Position.vx;
				relTargetPosition.vy-=sbPtr->DynPtr->Position.vy;
				relTargetPosition.vz-=sbPtr->DynPtr->Position.vz;

				/* Let's try doing this. */
				relTargetPosition.vy>>=2;
				
				range2=Approximate3dMagnitude(&relTargetPosition);
				#endif

				if (range2<SUSPECT_SENSITIVITY) {
					/* That's probably close enough. */
					marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
					/* Used to unset suspicion at that point. */
					if (marineStatusPointer->Mission==MM_Pathfinder) {
						/* Get back to business! */
						return(SRC_Request_Return);
					} else {
						return(SRC_Request_Wait);
					}
				}
							
			}
			NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&marineStatusPointer->waypointManager);
		} else if (!targetModule) {
			/* Must be inaccessible.  Time out suspicion. */
			marineStatusPointer->suspicious-=NormalFrameTime;
			/* To fix the next trap... */
			if (marineStatusPointer->suspicious==0) {
				marineStatusPointer->suspicious=-1;
			}
			if (marineStatusPointer->suspicious<0) {
				marineStatusPointer->suspicious=0;
				/* Set to zero on natural timeout, too. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
				if ((marineStatusPointer->behaviourState==MBS_Waiting)
					||(marineStatusPointer->behaviourState==MBS_Sentry)) {
					/* We might concievably want to do this for all states. */
					marineStatusPointer->gotapoint=0;
				}
			}

			if (ShowSquadState) {
				if (marineStatusPointer->Target) {
					if (marineStatusPointer->Target->containingModule) {
						PrintDebuggingText("I can see you, but I can't get there!\n");
					} else {
						PrintDebuggingText("Hey, you've got no Containing Module!\n");
					}
				} else {
					PrintDebuggingText("Can't get to the suspect point!\n");
					/* Yuck.  Stop it. */
					#if 0
					marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
					/* Used to unset suspicion at that point. */
					#else
					marineStatusPointer->suspicious=1;
					/* Just this once, let's unset it... We can't do anything about it. */
					#endif
					return(SRC_Request_Wait);
				}
			}
			return(SRC_No_Change);
		} else {

			thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
			if (!thisEp) {
				LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",
					(*(targetModule->m_module_ptrs))->name,
					sbPtr->containingModule->name));
				//LOGDXFMT(("This assert is a busted adjacency!"));
				GLOBALASSERT(thisEp);
			}
			/* If that fired, there's a farped adjacency. */
			GLOBALASSERT(thisEp->alien_only==0);
			/* If that fired, GetNextModuleForLink went wrong. */
		
			marineStatusPointer->wanderData.worldPosition=thisEp->position;
			marineStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
			marineStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
			marineStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;

			NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
		
		}
				
	} else if (marineStatusPointer->destinationmodule!=NULL) {
		/* Go towards next module. */
		NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	} else {
		/*  we are still in approach state: target the target, and move */
		if (marineStatusPointer->Target) {
			NPCGetMovementTarget(sbPtr, marineStatusPointer->Target, &targetPosition, &targetIsAirduct,0);
		} else {
			/* How did we get here, anyway? */
			targetPosition=marineStatusPointer->suspect_point;
		}
		NPCGetMovementDirection(sbPtr, &velocityDirection, &targetPosition,&marineStatusPointer->waypointManager);
	}

	/* Should have a velocity set now. */

	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if(marineStatusPointer->obstruction.environment)
		{
			/* go to avoidance */
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &targetPosition, &velocityDirection))
	{

		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;
	
		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}


/* this function is specifically for marines, and creates a gun flash
pointing in the direction of the target point */
static void MaintainMarineGunFlash(STRATEGYBLOCK *sbPtr)
{
	VECTORCH firingDirn,firingPoint;
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int firingOffsetUp;
	int firingOffsetInfront;
	int firingOffsetAcross;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);

	GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);

	/* get the firing point offsets:*/
	if(marineStatusPointer->IAmCrouched)
	{
		firingOffsetUp = MARINE_FIRINGPOINT_UP;
		firingOffsetInfront = MARINE_FIRINGPOINT_INFRONT;
		firingOffsetAcross = MARINE_FIRINGPOINT_ACROSS;
	}
	else
	{
		firingOffsetUp = MARINE_FIRINGPOINT_UP_CROUCHED;
		firingOffsetInfront = MARINE_FIRINGPOINT_INFRONT_CROUCHED;
		firingOffsetAcross = MARINE_FIRINGPOINT_ACROSS_CROUCHED;
	}
	
	/* find the firing direction */
	firingDirn.vx = marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	firingDirn.vy = marineStatusPointer->weaponTarget.vy - sbPtr->DynPtr->Position.vy;
	firingDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	Normalise(&firingDirn);
	{
		VECTORCH yNormal = {0,-65536,0};
		VECTORCH tempDirn;
		VECTORCH acrossDirn;

		/* now find firing point (conceptually, the end of the weapon muzzle)... */
		firingPoint = sbPtr->DynPtr->Position;		
		firingPoint.vx += MUL_FIXED(firingDirn.vx,firingOffsetInfront);
		firingPoint.vz += MUL_FIXED(firingDirn.vz,firingOffsetInfront);		

		tempDirn = firingDirn;
		tempDirn.vy = 0;
		Normalise(&tempDirn);
		CrossProduct(&tempDirn,&yNormal,&acrossDirn);		
		Normalise(&acrossDirn);
		firingPoint.vx += MUL_FIXED(tempDirn.vx,firingOffsetAcross);
		firingPoint.vz += MUL_FIXED(tempDirn.vz,firingOffsetAcross);
		firingPoint.vy += MUL_FIXED(yNormal.vy,firingOffsetUp);
	}
	LOCALASSERT(marineStatusPointer->myGunFlash!=NULL);

	MaintainNPCGunFlashEffect(marineStatusPointer->myGunFlash,&marineStatusPointer->My_Gunflash_Section->World_Offset,
		 &marineStatusPointer->My_Gunflash_Section->SecMat);
}

/* NB this shouldn't be called if :
1. we are close to the target
2. we are crouched */
static void LobAGrenade(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    	
	//VECTORCH firingPoint;
	//VECTORCH firingDirn;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(marineStatusPointer);	
	
	/* first, find the firing direction */
	//firingDirn.vx = marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	//firingDirn.vy = marineStatusPointer->weaponTarget.vy - sbPtr->DynPtr->Position.vz;
	//firingDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	//Normalise(&firingDirn);
	
	if((sbPtr->DynPtr->Position.vy - marineStatusPointer->weaponTarget.vy) > 2500)
	{
		/* too high */
		return;
	}
	
	#if 0
	{
		VECTORCH yNormal = {0,-65536,0};
		VECTORCH tempDirn;
		VECTORCH acrossDirn;

		/* now find firing point (conceptually, the end of the weapon muzzle + a bit)... */
		firingPoint = sbPtr->DynPtr->Position;		
		firingPoint.vx += MUL_FIXED(firingDirn.vx,(MARINE_FIRINGPOINT_INFRONT+400));
		firingPoint.vz += MUL_FIXED(firingDirn.vz,(MARINE_FIRINGPOINT_INFRONT+400));		

		tempDirn = firingDirn;
		tempDirn.vy = 0;
		Normalise(&tempDirn);
		CrossProduct(&tempDirn,&yNormal,&acrossDirn);		
		Normalise(&acrossDirn);
		firingPoint.vx += MUL_FIXED(tempDirn.vx,MARINE_FIRINGPOINT_ACROSS);
		firingPoint.vz += MUL_FIXED(tempDirn.vz,MARINE_FIRINGPOINT_ACROSS);
		firingPoint.vy += MUL_FIXED(yNormal.vy,MARINE_FIRINGPOINT_UP);
	}
	#endif

	/* this bit nicked from player grenade creation	fn. in bh_weap.c :
	actually, it is now a pulse rifle grenade */
	{
		DISPLAYBLOCK *dispPtr;
		DYNAMICSBLOCK *dynPtr;

		dispPtr = MakeObject(I_BehaviourPulseGrenade,&marineStatusPointer->My_Gunflash_Section->World_Offset);
		if(!dispPtr) return;
		LOCALASSERT(dispPtr->ObStrategyBlock);	
		AddLightingEffectToObject(dispPtr,LFX_ROCKETJET);
		dynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ROCKET);
		if(!dynPtr)
		{
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}
		dispPtr->ObStrategyBlock->DynPtr = dynPtr;
		dispPtr->ObStrategyBlock->SBdataptr=AllocateMem(sizeof(PREDPISTOL_BEHAV_BLOCK));
		if(!dispPtr->ObStrategyBlock->SBdataptr)
		{
			RemoveBehaviourStrategy(dispPtr->ObStrategyBlock);
			return;
		}
		((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->counter = 5*ONE_FIXED;
		((PREDPISTOL_BEHAV_BLOCK *)dispPtr->ObStrategyBlock->SBdataptr)->player = 0;

		GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
			
	    #define PULSEGRENADE_SPEED 100000 // Was 30000
		dynPtr->Position = marineStatusPointer->My_Gunflash_Section->World_Offset;
		dynPtr->OrientMat = marineStatusPointer->My_Gunflash_Section->SecMat;
		dynPtr->PrevOrientMat = dynPtr->OrientMat; /* stops mis-alignment if dynamics problem */	    

	    dynPtr->LinVelocity.vx = dynPtr->OrientMat.mat31;
	    dynPtr->LinVelocity.vy = dynPtr->OrientMat.mat32;
	    dynPtr->LinVelocity.vz = dynPtr->OrientMat.mat33;

	    dynPtr->LinVelocity.vx = MUL_FIXED(dynPtr->LinVelocity.vx, PULSEGRENADE_SPEED);
	    dynPtr->LinVelocity.vy = MUL_FIXED(dynPtr->LinVelocity.vy, PULSEGRENADE_SPEED);
	    dynPtr->LinVelocity.vz = MUL_FIXED(dynPtr->LinVelocity.vz, PULSEGRENADE_SPEED);

	    //dynPtr->LinImpulse.vx = MUL_FIXED(firingDirn.vx,PULSEGRENADE_SPEED);
	    //dynPtr->LinImpulse.vy = MUL_FIXED(firingDirn.vy,PULSEGRENADE_SPEED);
	    //dynPtr->LinImpulse.vz = MUL_FIXED(firingDirn.vz,PULSEGRENADE_SPEED);		
	}
}

/*NB avoidance state behaviour function probabaly doesn't need to check
for crouching, or for target becoming cloaked.  Only when we exit the state
do we need to check these behaviour parameters to pick a new state and sequence*/
static STATE_RETURN_CONDITION Execute_MNS_Avoidance(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int terminateState = 0;
	
	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);

	#if 1
	HandleMovingAnimations(sbPtr);
	#endif
	
	#if ALL_NEW_AVOIDANCE
	/* New avoidance kernel. */

	NPCSetVelocity(sbPtr, &(marineStatusPointer->avoidanceManager.avoidanceDirection), (marineStatusPointer->nearSpeed));
	/* Velocity CANNOT be zero, unless deliberately so! */	
	{
		AVOIDANCE_RETURN_CONDITION rc;
		
		rc=AllNewAvoidanceKernel(sbPtr,&marineStatusPointer->avoidanceManager);

		if (rc!=AvRC_Avoidance) {
			terminateState=1;
		}
		/* Should put in a test for AvRC_Failure here. */
	}

	#if 0	
	{
		VECTORCH point;
		/* Wacky test. */
		GetTargetingPointOfObject_Far(sbPtr,&point);
		MakeParticle(&point,&sbPtr->DynPtr->LinVelocity,PARTICLE_NONCOLLIDINGFLAME);
	
		MakeParticle(&point,&(marineStatusPointer->avoidanceManager.avoidanceDirection),PARTICLE_PREDATOR_BLOOD);
		MakeParticle(&point,&(marineStatusPointer->avoidanceManager.aggregateNormal),PARTICLE_ALIEN_BLOOD);

	}
	#endif

	#else
	/* set velocity */
	LOCALASSERT((marineStatusPointer->moveData.avoidanceDirn.vx!=0)||
				(marineStatusPointer->moveData.avoidanceDirn.vy!=0)||
				(marineStatusPointer->moveData.avoidanceDirn.vz!=0));
	NPCSetVelocity(sbPtr, &(marineStatusPointer->moveData.avoidanceDirn), (marineStatusPointer->nearSpeed));

	/* decrement state timer */
	marineStatusPointer->stateTimer -= NormalFrameTime;
	if(marineStatusPointer->stateTimer <= 0) terminateState = 1;

	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&obstruction,&destructableObject);
		if(obstruction.anySingleObstruction)
		{
			terminateState = 1;
		}
	}
	#endif

	if(terminateState)
	{
		/* switch to approach */
		if(MarineIsAwareOfTarget(sbPtr))
		{
			/* go to approach */
			
			return(SRC_Request_Approach);
			/* Was fire! */
		}
		else
		{
			/* go to an appropriate state */
			switch (marineStatusPointer->lastState) {
				case MBS_Retreating:
					return(SRC_Request_Retreat);
					break;
				case MBS_Returning:
					return(SRC_Request_Return);
					break;
				case MBS_Responding:
					return(SRC_Request_Respond);
					break;
				case MBS_Approaching:
					/* Go directly to approach.  Do not pass GO.  Do not collect 200 zorkmids. */
					return(SRC_Request_Approach);
					break;
				case MBS_Sentry:
					/* Go back to sentry. */
					return(SRC_Request_Wait);
					break;
				default:
					return(SRC_Request_Wander);
					break;
			}
			/* Still here? */
			return(SRC_Request_Wander);
		}
	}
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_Wander(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH velocityDirection = {0,0,0};
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);
	
	HandleMovingAnimations(sbPtr);

	/* should we change to approach state? */
	if ((MarineIsAwareOfTarget(sbPtr))||(marineStatusPointer->suspicious))
	{
		/* doesn't require a sequence change */
		return(SRC_Request_Approach);
	}

	/* Is there a new alert? */
	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		/* Are we already there? */
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				AIMODULE *targetModule=0;
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					return(SRC_Request_Respond);
				}
			}
		}
	}

	/* Are we bored of wandering yet? */

	marineStatusPointer->stateTimer-=NormalFrameTime;

	if (marineStatusPointer->stateTimer<=0) {
		/* Time to make a decision. */
		marineStatusPointer->stateTimer = MARINE_NEARWAITTIME;
		if ((FastRandom()&65535)<8192)
		{
			/* This is too tiring... let's just wait around a while. */
			return(SRC_Request_Wait);
		}
	}

	/* Are we using bimble rules? */

	if (marineStatusPointer->wanderData.currentModule==NPC_BIMBLINGINMODULE) {
	
		int range;

		/* Range to target... */

		range=VectorDistance((&marineStatusPointer->wanderData.worldPosition),(&sbPtr->DynPtr->Position));

		if (range<2000) {

			/* Reset system, try again. */
			marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
			marineStatusPointer->moveData.lastModule=NULL;

		}
		
	} else {

		/* wander target aquisition: if no target, or moved module */
		LOCALASSERT(sbPtr->containingModule);
		if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		{
			NPC_InitMovementData(&(marineStatusPointer->moveData));
			marineStatusPointer->moveData.lastModule=marineStatusPointer->lastmodule;
			NPC_FindAIWanderTarget(sbPtr,&(marineStatusPointer->wanderData),&(marineStatusPointer->moveData),0);
			
		}
		else if(marineStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)
		{
			NPC_FindAIWanderTarget(sbPtr,&(marineStatusPointer->wanderData),&(marineStatusPointer->moveData),0);

		}
		
		/* if we still haven't got one, bimble about in this one for a bit. */
		if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		{
			NPC_GetBimbleTarget(sbPtr,&marineStatusPointer->wanderData.worldPosition);
			marineStatusPointer->wanderData.currentModule=NPC_BIMBLINGINMODULE;

		}
		
	}
		
	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			/* go to avoidance */
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_Return(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	AIMODULE *targetModule;
	VECTORCH velocityDirection = {0,0,0};
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);
	
	HandleMovingAnimations(sbPtr);

	/* should we change to approach state? */
	if(MarineIsAwareOfTarget(sbPtr))
	{
		/* doesn't require a sequence change */
		return(SRC_Request_Approach);
	}

	#if 0
	/* Ignore alerts. */
	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		/* Are we already there? */
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				AIMODULE *targetModule=0;
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					return(SRC_Request_Respond);
				}
			}
		}
	}
	#endif

	marineStatusPointer->stateTimer-=NormalFrameTime;

	/* Are we there yet? */
	if (sbPtr->containingModule->m_aimodule==marineStatusPointer->missionmodule) {
		return(SRC_Request_Wait);
	}
	
	/* Target module aquisition. */

	LOCALASSERT(sbPtr->containingModule);

	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		NPC_InitMovementData(&(marineStatusPointer->moveData));
	}
	if ((marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		||(marineStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)) {

		targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->missionmodule,7,0);
	
		if (targetModule) {
			FARENTRYPOINT *thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
			if(thisEp) {
				/* aha. an ep!... */ 
				VECTORCH thisEpWorld = thisEp->position;

				thisEpWorld.vx += targetModule->m_world.vx;
				thisEpWorld.vy += targetModule->m_world.vy;
				thisEpWorld.vz += targetModule->m_world.vz;			
				
				marineStatusPointer->wanderData.currentModule = sbPtr->containingModule->m_aimodule->m_index;
		 		marineStatusPointer->wanderData.worldPosition = thisEpWorld;
			
				GLOBALASSERT(thisEp->alien_only==0);
				/* If that fired, GetNextModuleForLink went wrong. */
			} else {
				/* Failure case. */
				marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
			}

		} else {
			/* Another failure case. */
			marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
		}
	}
	
	/* if we still haven't got one, bimble about in this one for a bit. */
	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		NPC_InitMovementData(&(marineStatusPointer->moveData));
		marineStatusPointer->moveData.lastModule=marineStatusPointer->lastmodule;
		NPC_FindAIWanderTarget(sbPtr,&(marineStatusPointer->wanderData),&(marineStatusPointer->moveData),0);
	}

	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE) {
		/* STILL broken!  Okay, just... wander, then. */
		return(SRC_Request_Wander);
	}
		
	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			/* go to avoidance */
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_Pathfinder(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	AIMODULE *targetModule;
	VECTORCH velocityDirection = {0,0,0};
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);
	
	#if 0	
	/* check if we should be crouched or standing up */
	if(marineStatusPointer->IAmCrouched)
	{
		/* curently crouched */
		if(!(MarineShouldBeCrawling(sbPtr)))
		{
			/* should be running*/
			marineStatusPointer->IAmCrouched = 0;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	else
	{
		/* currently standing */
		if(MarineShouldBeCrawling(sbPtr))
		{
			/* should be crawling */
			marineStatusPointer->IAmCrouched = 1;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif

	/* should we change to approach state? */
	if(MarineIsAwareOfTarget(sbPtr))
	{
		/* doesn't require a sequence change */
		return(SRC_Request_Approach);
	}

	if (marineStatusPointer->suspicious) {
		MODULE *targetModule;
		/* Are we allowed to check it out? */
		targetModule=ModuleFromPosition(&marineStatusPointer->suspect_point,sbPtr->containingModule);
		if (targetModule) {
			if (IsModuleVisibleFromModule(targetModule,sbPtr->containingModule)) {
				/* I suppose we'd better go, then. */
				if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
					return(SRC_Request_Wait);
				} else {
					return(SRC_Request_Approach);
				}
			}
		}
	}

	/* Ignore alerts. */

	marineStatusPointer->stateTimer-=NormalFrameTime;

	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		NPC_InitMovementData(&(marineStatusPointer->moveData));
	}
	if ((marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		||(marineStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)) {

		FARENTRYPOINT *thisEp;
		int nextModuleIndex;
		/* Okay, so where are we exactly? */

		if ((marineStatusPointer->stepnumber<0)||(marineStatusPointer->path<0)) {
			/* Get OUT! */
			return(SRC_Request_Wander);
		}
	
		targetModule = TranslatePathIndex(marineStatusPointer->stepnumber,marineStatusPointer->path);
	
		if (targetModule==NULL) {
			/* Oh, to heck with this.  Try to wander. */
			return(SRC_Request_Wander);
		}
		
		/* Right, so there is a somewhere to get to. */
	
		if (targetModule!=sbPtr->containingModule->m_aimodule) {
			/* But we're nowhere near it.  Geeze... */
			marineStatusPointer->missionmodule=targetModule;
			return(SRC_Request_Return);
		}
		
		/* Okay, so now we need to know where to go now. */
	
		nextModuleIndex=GetNextModuleInPath(marineStatusPointer->stepnumber,marineStatusPointer->path);
		GLOBALASSERT(nextModuleIndex>=0);
		/* If that fires, it's Richard's fault. */
		targetModule=TranslatePathIndex(nextModuleIndex,marineStatusPointer->path);
		GLOBALASSERT(targetModule);
		/* Ditto. */
		marineStatusPointer->stepnumber=nextModuleIndex;
		
		thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
		if(thisEp) {
			/* aha. an ep!... */ 
			VECTORCH thisEpWorld = thisEp->position;

			thisEpWorld.vx += targetModule->m_world.vx;
			thisEpWorld.vy += targetModule->m_world.vy;
			thisEpWorld.vz += targetModule->m_world.vz;			
			
			marineStatusPointer->wanderData.currentModule = sbPtr->containingModule->m_aimodule->m_index;
			marineStatusPointer->wanderData.worldPosition = thisEpWorld;
			GLOBALASSERT(thisEp->alien_only==0);
			/* If that fired, the path goes through an alien-only link. */
		
		} else {
			/* Failure case. */
			marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
		}

	}
	
	/* if we still haven't got one, wander for a bit. */
	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
	{
		NPC_InitMovementData(&(marineStatusPointer->moveData));
		marineStatusPointer->moveData.lastModule=marineStatusPointer->lastmodule;
		NPC_FindAIWanderTarget(sbPtr,&(marineStatusPointer->wanderData),&(marineStatusPointer->moveData),0);
	}

	if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE) {
		/* STILL broken!  Okay, just... wander forever, then. */
		return(SRC_Request_Wander);
	}
		
	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			/* go to avoidance */
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}

void NPC_GetBimbleTarget(STRATEGYBLOCK *sbPtr,VECTORCH *output) {

	MODULE *my_module;
	VECTORCH alternate;
	int success;

	/* Get a random position in the same module. */

	my_module=sbPtr->containingModule;
	success=0;

	while (success==0) {

		do {
			output->vx=FastRandom()&65535;
			output->vx+=my_module->m_minx;
		} while (!( (output->vx>my_module->m_minx)&&(output->vx<my_module->m_maxx) ));
		
		do {
			output->vz=FastRandom()&65535;
			output->vz+=my_module->m_minz;
		} while (!( (output->vz>my_module->m_minz)&&(output->vz<my_module->m_maxz) ));
		
		output->vy=sbPtr->DynPtr->Position.vy;
		output->vy-=my_module->m_world.vy;
		
		if (!( (output->vy>my_module->m_miny)&&(output->vy<my_module->m_maxy) )) {
			do {
				output->vy=FastRandom()&65535;
				output->vy+=my_module->m_miny;
			} while (!( (output->vy>my_module->m_miny)&&(output->vy<my_module->m_maxy) ));
		}
		
		GLOBALASSERT(PointIsInModule(my_module,output));
		
		output->vx+=my_module->m_world.vx;
		output->vy+=my_module->m_world.vy;
		output->vz+=my_module->m_world.vz;
		
		/* Check for waypoints? */
		
		if (my_module->m_waypoints) {
			if (GetPositionValidity(my_module,output,&alternate)==NULL) {
				/* Failure! */
				success=0;
			} else {
				if ( (alternate.vx!=output->vx)
					|| (alternate.vy!=output->vy)
					|| (alternate.vz!=output->vz) ) {
					*output=alternate;
					/* Success! */
					success=1;
				} else {
					/* Success! */
					success=1;
				}
			}	
		} else {
			/* Success! */
			success=1;
		}

	}

}

static void HandleFidgetAnimations(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* A sub function for simplicity. */

	if(marineStatusPointer->IAmCrouched) {
		/* No crouch fidget yet. */
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrouch)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MCrSS_Standard)) {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	} else {
		/* Are we in some sort of fidget anim? */
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)||(
			(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Stand_To_Fidget)
			&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Fidget_A)
			&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Fidget_B)
			&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Fidget_C)
			)) {

			if ((marineStatusPointer->HModelController.Sequence_Type==HMSQT_MarineRun)
				&&(marineStatusPointer->HModelController.Sub_Sequence==MRSS_Mooch_Bored)) {
				/* Mooch bored does not require Stand_To_Fidget - go directly to Fidget_A. */
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_A)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_A,ONE_FIXED<<3,(ONE_FIXED>>3));
					marineStatusPointer->internalState=0;
				} else {
					/* No fidgets at all! */
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
				}
			} else if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Stand_To_Fidget)) {
				/* Not in any kind of fidget anim: run Stand_To_Fidget if poss. */
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Stand_To_Fidget,((ONE_FIXED*3)/2),(ONE_FIXED>>3));
				marineStatusPointer->HModelController.LoopAfterTweening=0;
			} else if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_A)) {
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_A,ONE_FIXED<<3,(ONE_FIXED>>3));
			} else {
				/* No fidgets at all! */
				SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
			}

		} else {
			if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Stand_To_Fidget)) {
				/* We must have gone through StandToFidget, or at least be in it. */
				if (((marineStatusPointer->HModelController.Tweening==Controller_NoTweening)
					&&(marineStatusPointer->HModelController.sequence_timer==(ONE_FIXED-1))
					&&(marineStatusPointer->HModelController.Looped==0))
				||((marineStatusPointer->HModelController.keyframe_flags)&&(marineStatusPointer->internalState))) {
					/* End of old sequence. */
					/* Go back to normal. */
					if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_A)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_A,ONE_FIXED<<3,(ONE_FIXED>>3));
						marineStatusPointer->internalState=0;
					}
				} else if (marineStatusPointer->HModelController.keyframe_flags) {
					if ((FastRandom()&65535)<21846) {
						if ((FastRandom()&65535)<32767) {
							if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_B)) {
								SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_B,-1,(ONE_FIXED>>3));
								marineStatusPointer->HModelController.LoopAfterTweening=0;
								marineStatusPointer->internalState=1;
							} else if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_C)) {
								SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_C,-1,(ONE_FIXED>>3));
								marineStatusPointer->HModelController.LoopAfterTweening=0;
								marineStatusPointer->internalState=2;
							}
						} else {
							if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_C)) {
								SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_C,-1,(ONE_FIXED>>3));
								marineStatusPointer->HModelController.LoopAfterTweening=0;
								marineStatusPointer->internalState=2;
							} else if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_Fidget_B)) {
								SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Fidget_B,-1,(ONE_FIXED>>3));
								marineStatusPointer->HModelController.LoopAfterTweening=0;
								marineStatusPointer->internalState=1;
							}
						}
					}
					/* Else do nothing, I guess. */
				}
			} else {
				/* No stand to fidget. */
			}
		}
	}

}

static void HandleWaitingAnimations(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int tweeningtime;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	#if 0
	/* If you get here, you look stationary, so BE stationary! */
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;
	#endif
	
	{
		DELTA_CONTROLLER *delta;
		/* There should be NO head turn delta. */
		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		if (delta) {
			Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
		}
	}

	/* First test to see if we're in midair. */
	if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_Jump)) {
		if (!sbPtr->DynPtr->IsInContactWithFloor) {
			VECTORCH offset;

			offset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;

			if ((offset.vx!=0)||(offset.vy!=0)||(offset.vz!=0)) {
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Jump)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Jump,-1,(ONE_FIXED>>4));
				}
			}
		}
	}

	/* Get a random tweening time... */

	tweeningtime=((FastRandom()&65535)+32767)>>3;

	/* Figure out what we should be playing, and do it. */

	if (MarineShouldBeCrawling(sbPtr)) {
		marineStatusPointer->IAmCrouched=1;
	} else {
		marineStatusPointer->IAmCrouched=0;
	}

	if(marineStatusPointer->IAmCrouched) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrouch)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MCSS_Standard)) {
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrouch,MCSS_Standard,ONE_FIXED,tweeningtime);
		}
	} else {
		if (marineStatusPointer->suspicious) {
			/* Go directly to wait alert, if you can. */
			if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_Wait_Alert)) {
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Wait_Alert)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Wait_Alert,(MUL_FIXED((ONE_FIXED<<2),((FastRandom()&32767)+65536))),tweeningtime);
				}
			} else {
				HandleFidgetAnimations(sbPtr);
			}
		} else {
			HandleFidgetAnimations(sbPtr);
		}
	}
}

static void HandleMovingAnimations(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	MARINE_MOVEMENT_STYLE style;
	MARINE_BHSTATE baseState;
	const MOVEMENT_DATA *movementData;
	VECTORCH offset;
	int can_mooch_bored;
	int can_mooch_alert;
	int can_sprint;
	int speed,animfactor;
	
	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	style=-1;

	/* First test to see if we're in midair. */
	if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_Jump)) {
		if (!sbPtr->DynPtr->IsInContactWithFloor) {
			VECTORCH offset;

			offset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;

			if ((offset.vx!=0)||(offset.vy!=0)||(offset.vz!=0)) {
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Jump)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Jump,-1,(ONE_FIXED>>4));
				}
			}
		}
	}

	can_mooch_bored=HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_Mooch_Bored);
	can_mooch_alert=HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_Mooch_Alert);
	can_sprint=HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_Sprint);

	/* Figure out what we should be playing... */
	
	if (marineStatusPointer->behaviourState==MBS_Avoidance) {
		baseState=marineStatusPointer->lastState;
	} else {
		baseState=marineStatusPointer->behaviourState;
	}

	switch (baseState) {
		case MBS_Waiting:
			/* Why am I here? */
			style=MMS_Alert;
			break;
		case MBS_Wandering:
			style=MMS_Bored;
			break;
		case MBS_Retreating:
			style=MMS_Sprint;
			break;
		case MBS_Sentry:
		case MBS_Returning:
		case MBS_Pathfinding:
			if (marineStatusPointer->Target==NULL) {
				if (marineStatusPointer->suspicious) {
					style=MMS_Alert;
				} else {
					style=MMS_Bored;
				}
			} else {
				style=MMS_Combat;
			}
		case MBS_Approaching:
		case MBS_Responding:
			if (marineStatusPointer->Target==NULL) {
				style=MMS_Alert;
			} else {
				style=MMS_Combat;
			}
			break;
		case MBS_Firing:
			/* Shouldn't really be here. */
			style=MMS_Combat;
			break;
		case MBS_Avoidance:
		case MBS_Dying:
			/* Definitely shouldn't be here! */
			GLOBALASSERT(0);
			break;
		default:
			/* Shouldn't really be here. */
			style=MMS_Combat;
			break;
	}	

	/* Finally... */
	offset.vx=sbPtr->DynPtr->Position.vx-sbPtr->DynPtr->PrevPosition.vx;
	offset.vy=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;
	offset.vz=sbPtr->DynPtr->Position.vz-sbPtr->DynPtr->PrevPosition.vz;
	{
		#if 0
		int dist;
		/* ...compute speed factor... */
		speed=Approximate3dMagnitude(&sbPtr->DynPtr->LinVelocity);
		dist=Approximate3dMagnitude(&offset);
		if (dist<(MUL_FIXED(NormalFrameTime,100))) {
			/* Not moving much, are we? */
			style=MMS_Stationary;
		}
		#else
		/* ...compute speed factor... */
		speed=Magnitude(&offset);
		if (speed<(MUL_FIXED(NormalFrameTime,50))) {
			/* Not moving much, are we? */
			style=MMS_Stationary;
		}
		speed=DIV_FIXED(speed,NormalFrameTime);
		#endif
	}

	/* Fix crouching. */
	if(marineStatusPointer->IAmCrouched)
	{
		/* currently crouched */
		if(!(MarineShouldBeCrawling(sbPtr))) {
			marineStatusPointer->IAmCrouched=0;
		}
	} else {
		/* currently standing */
		if(MarineShouldBeCrawling(sbPtr)) {
			marineStatusPointer->IAmCrouched=1;
		}
	}

	/* Now, pre-emptive reject of unavailable cases. */
	if (style==MMS_Bored) {
		if (!can_mooch_bored) {
			style=MMS_Combat;
		}
	}

	if (style==MMS_Alert) {
		if (!can_mooch_alert) {
			style=MMS_Combat;
		}
	}

	if (style==MMS_Sprint) {
		#if 1
		if (!can_sprint) {
		#else
		if (1) {
		#endif
			style=MMS_Combat;
		}
	}

	if (speed==0) {
		style=MMS_Stationary;
		animfactor=ONE_FIXED;
	} else {
		animfactor=DIV_FIXED(625,speed); // Was 512!  Difference to correct for rounding down...
	}
	GLOBALASSERT(animfactor>0);

	/* ...and do it. */
	switch (style) {
		
		case -1:
			/* Whoops! */
			GLOBALASSERT(0);
			break;
		case MMS_Stationary:
			{
				/* Ouch. */
				HandleWaitingAnimations(sbPtr);
			}
			break;
		case MMS_Bored:
			{
				if(marineStatusPointer->IAmCrouched) {
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrawl)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MCSS_Standard)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>4));
					}

					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Combat);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Combat);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);

				} else {
					/* If we're here, we must be able to mooch bored. */
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineRun)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MRSS_Mooch_Bored)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Mooch_Bored,((ONE_FIXED*7)/5),(ONE_FIXED>>4));
					}
	
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Mooch_Bored);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Mooch_Bored);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);

					//marineStatusPointer->nearSpeed=MARINE_NEAR_SPEED>>2;
				}
			}
			break;
		case MMS_Alert:
			{
				if(marineStatusPointer->IAmCrouched) {
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrawl)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MCSS_Standard)) {
							SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>4));
					}
					
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Combat);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Combat);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				} else {
					/* If we're here, we must be able to mooch alert. */
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineRun)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MRSS_Mooch_Alert)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Mooch_Alert,((ONE_FIXED*23)/10),(ONE_FIXED>>4));
					}
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Mooch_Alert);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Mooch_Alert);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				}
			}
			break;
		case MMS_Sprint:
			{
				if(marineStatusPointer->IAmCrouched) {
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrawl)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MCSS_Standard)) {
							SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>4));
					}
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Combat);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Combat);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				} else {
					/* If we're here, we must be able to sprint. */
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineRun)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MRSS_Sprint)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Sprint,((ONE_FIXED*7)/10),(ONE_FIXED>>4));
					}
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Sprint);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Sprint);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				}
			}
			break;
		case MMS_Combat:
		default:
			{
				if(marineStatusPointer->IAmCrouched) {
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineCrawl)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MCSS_Standard)) {
							SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCSS_Standard,ONE_FIXED,(ONE_FIXED>>4));
					}
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Combat);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Combat);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				} else {
					if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineRun)
						||(marineStatusPointer->HModelController.Sub_Sequence!=MRSS_Standard)) {
						SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>4));
					}
					if (marineStatusPointer->My_Weapon->ARealMarine) {	
						movementData=GetThisMovementData(MDI_Marine_Combat);
					} else {
						movementData=GetThisMovementData(MDI_Civilian_Combat);
					}
					GLOBALASSERT(movementData);
					marineStatusPointer->nearSpeed = MUL_FIXED(movementData->maxSpeed,marineStatusPointer->speedConstant);
					marineStatusPointer->acceleration = MUL_FIXED(movementData->acceleration,marineStatusPointer->accelerationConstant);
				}
			}
			break;
	
	}
	if (marineStatusPointer->HModelController.Tweening==0) {
		HModel_SetToolsRelativeSpeed(&marineStatusPointer->HModelController,animfactor);
	}

	/* Finally, civilians sprinting away look over their shoulders... */
	{
		int crs;
		DELTA_CONTROLLER *delta;

		crs=0;

		if (marineStatusPointer->My_Weapon->ARealMarine==0) {
			if (style==MMS_Sprint) {
				if (marineStatusPointer->IAmCrouched==0) {
					if (marineStatusPointer->behaviourState==MBS_Retreating) {
						if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineRun,MRSS_SprintHeadDelta)) {
							crs=1;
						}
					}
				}
			}
		}

		if (crs) {
			delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
			if (!delta) {
				/* Add it. */
				delta=Add_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta",(int)HMSQT_MarineRun,(int)MRSS_SprintHeadDelta,-1);
				GLOBALASSERT(delta);
				delta->Playing=0;
				delta->Looped=0;
			}
			/* Now we must have it... */
			if (delta->Playing==0) {
				if (marineStatusPointer->incidentFlag) {
					/* Start it. */
					Start_Delta_Sequence(delta,(int)HMSQT_MarineRun,(int)MRSS_SprintHeadDelta,-1);
					delta->Playing=1;
				}
			} else {
				if (DeltaAnimation_IsFinished(delta)) {
					delta->Playing=0;
				}
			}
		} else {
			/* There should be NO such delta. */
			delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
			if (delta) {
				Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");
			}
		}
	}
}

static STATE_RETURN_CONDITION Execute_MNS_SentryMode(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,offset,velocityDirection;
	int correctlyOrientated,range;
	int dist;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->my_spot.vx;
	offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->my_spot.vy;
	offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->my_spot.vz;
	/* Fix for midair start points, grrrr. */
	offset.vy>>=2;
	
	/* Find distance off spot. */
	dist=Approximate3dMagnitude(&offset);

	if ((dist<SENTRY_SENSITIVITY)&&(sbPtr->containingModule->m_aimodule==marineStatusPointer->missionmodule)) {
	
		/* On the spot. */

		/* zero velocity */
		LOCALASSERT(sbPtr->DynPtr);
		sbPtr->DynPtr->LinVelocity.vx = 0;
		sbPtr->DynPtr->LinVelocity.vy = 0;
		sbPtr->DynPtr->LinVelocity.vz = 0;

		HandleWaitingAnimations(sbPtr);
		
		correctlyOrientated=0;

		if(MarineIsAwareOfTarget(sbPtr)) {
		
			GLOBALASSERT(marineStatusPointer->Target);
			NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		
			/* orientate to firing point first */
			orientationDirn.vx = marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		} else if (marineStatusPointer->suspicious) {
			/* Orientate to suspect point? */
			orientationDirn.vx = marineStatusPointer->suspect_point.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->suspect_point.vz - sbPtr->DynPtr->Position.vz;
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
			if (correctlyOrientated) {
				/* Please don't be staring at a wall... */
    			SECTION_DATA *head_sec;
				VECTORCH sight_vec;

				head_sec=GetThisSectionData(marineStatusPointer->HModelController.section_data,"head");
				GLOBALASSERT(head_sec);

				sight_vec.vx=sbPtr->DynPtr->OrientMat.mat31;
				sight_vec.vy=sbPtr->DynPtr->OrientMat.mat32;
				sight_vec.vz=sbPtr->DynPtr->OrientMat.mat33;
				
				FindPolygonInLineOfSight(&sight_vec,&head_sec->World_Offset,0,sbPtr->SBdptr);
				if (LOS_ObjectHitPtr) {
					if (SBIsEnvironment(LOS_ObjectHitPtr->ObStrategyBlock)) {
						if (LOS_Lambda<2000) {
							Marine_MirrorSuspectPoint(sbPtr);
						}
					}
				}
			}
			marineStatusPointer->gotapoint=0;
		} else {
			if (marineStatusPointer->gotapoint) {
				VECTORCH orientationDirn;
				int correctlyOrientated;

				orientationDirn.vx = marineStatusPointer->wanderData.worldPosition.vx - sbPtr->DynPtr->Position.vx;
				orientationDirn.vy = 0;
				orientationDirn.vz = marineStatusPointer->wanderData.worldPosition.vz - sbPtr->DynPtr->Position.vz;
				correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
			} else {
				GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);
			}
		}
		
		if(!correctlyOrientated) {
		
			return(SRC_No_Change);
		
		}
		
		if (marineStatusPointer->Target==NULL) {
			/* Must be suspicious? */
			if (marineStatusPointer->suspicious) {
				return(SRC_No_Change);
			}
			/* Else drop through? */
		} else {
			/* We have a target, and should be correctly orientated. */
		
			if (MarineCanSeeTarget(sbPtr)) {
				/* I can see! */
				range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
			
				if ((marineStatusPointer->My_Weapon->MaxRange==-1) ||
					(range<marineStatusPointer->My_Weapon->MaxRange)) {
			
				   return(SRC_Request_Fire);	
			
				}
			} else {
				/* Eh? */
				if (marineStatusPointer->suspicious) {
					return(SRC_No_Change);
				}
			}
		}
		
		/* Well, we're stuck with sentrymode. */
		if (marineStatusPointer->gotapoint) {
		
			VECTORCH orientationDirn;
			int correctlyOrientated;
		
			orientationDirn.vx = marineStatusPointer->wanderData.worldPosition.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->wanderData.worldPosition.vz - sbPtr->DynPtr->Position.vz;
		
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		} else {
			GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);
		}
		
		return(SRC_No_Change);
		
	}

	/* If you got here, you're lost. */

	if (marineStatusPointer->missionmodule==NULL) {
		/* Fused! */
		return(SRC_No_Change);
	}

	HandleMovingAnimations(sbPtr);

	if (sbPtr->containingModule->m_aimodule!=marineStatusPointer->missionmodule) {

		AIMODULE *targetModule;
		/* Not even in the same module! */

		if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		{
			NPC_InitMovementData(&(marineStatusPointer->moveData));
		}
		if ((marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
			||(marineStatusPointer->wanderData.currentModule!=sbPtr->containingModule->m_aimodule->m_index)) {
		
			targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->missionmodule,7,0);
		
			if (targetModule) {
				FARENTRYPOINT *thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
				if(thisEp) {
					/* aha. an ep!... */ 
					VECTORCH thisEpWorld = thisEp->position;
		
					thisEpWorld.vx += targetModule->m_world.vx;
					thisEpWorld.vy += targetModule->m_world.vy;
					thisEpWorld.vz += targetModule->m_world.vz;			
					
					marineStatusPointer->wanderData.currentModule = sbPtr->containingModule->m_aimodule->m_index;
			 		marineStatusPointer->wanderData.worldPosition = thisEpWorld;

					GLOBALASSERT(thisEp->alien_only==0);
					/* If that fired, GetNextModuleForLink went wrong. */
				
				} else {
					/* Failure case. */
					marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
				}
		
			} else {
				/* Another failure case. */
				marineStatusPointer->wanderData.currentModule=NPC_NOWANDERMODULE;
			}
		}
		
		/* if we still haven't got one, bimble about in this one for a bit. */
		if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE)
		{
			NPC_InitMovementData(&(marineStatusPointer->moveData));
			marineStatusPointer->moveData.lastModule=marineStatusPointer->lastmodule;
			NPC_FindAIWanderTarget(sbPtr,&(marineStatusPointer->wanderData),&(marineStatusPointer->moveData),0);
		}
		
		if(marineStatusPointer->wanderData.currentModule==NPC_NOWANDERMODULE) {
			/* STILL broken!  We're in a lot of trouble. */
			return(SRC_No_Change);
		}
		
		/* Should have a legal target. */
		NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);

	} else {
		/* Same module, wrong place.  Just go for it.  */
		NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->my_spot),&marineStatusPointer->waypointManager);

	}

	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;
		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			/* go to avoidance */
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
	

}

static STATE_RETURN_CONDITION Execute_MNS_Wait(STRATEGYBLOCK *sbPtr)
{
	/* wait until near state timer runs out, then wander:
	alternatively, if we can attack the target, go straight to approach */
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* test for attack */
	if (MarineIsAwareOfTarget(sbPtr))
	{
		if ((FastRandom()&65535)<32767) {
			return(SRC_Request_Approach);
		} else {
			return(SRC_Request_Fire);
		}
	}

	if (marineStatusPointer->suspicious) {
		int range;

		#if 0
		range=VectorDistance(&marineStatusPointer->suspect_point,(&sbPtr->DynPtr->Position));
		#else
		VECTORCH targetPosition;
		
		targetPosition=marineStatusPointer->suspect_point;

		targetPosition.vx-=sbPtr->DynPtr->Position.vx;
		targetPosition.vy-=sbPtr->DynPtr->Position.vy;
		targetPosition.vz-=sbPtr->DynPtr->Position.vz;

		/* Let's try doing this. */
		targetPosition.vy>>=2;
		
		range=Approximate3dMagnitude(&targetPosition);
		#endif
		
		if (range>SUSPECT_SENSITIVITY) {
			/* Too far away! */
			marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
			/* Used to unset suspicion at that point. */
			return(SRC_Request_Approach);
		} else {
			/* We could at least turn to face it. */
			VECTORCH orientationDirn;
			int correctlyOrientated;

			orientationDirn.vx = marineStatusPointer->suspect_point.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->suspect_point.vz - sbPtr->DynPtr->Position.vz;

			if (sbPtr->SBDamageBlock.IsOnFire) {
				/* Can't handle 'suspect points' stuck to them! */
				correctlyOrientated = 1;
			} else {
				correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
			}

			/* For when suspicion times out. */
			marineStatusPointer->gotapoint=0;

			if (correctlyOrientated) {
				/* Please don't be staring at a wall... */
    			SECTION_DATA *head_sec;
				VECTORCH sight_vec;

				head_sec=GetThisSectionData(marineStatusPointer->HModelController.section_data,"head");
				GLOBALASSERT(head_sec);

				sight_vec.vx=sbPtr->DynPtr->OrientMat.mat31;
				sight_vec.vy=sbPtr->DynPtr->OrientMat.mat32;
				sight_vec.vz=sbPtr->DynPtr->OrientMat.mat33;
				
				FindPolygonInLineOfSight(&sight_vec,&head_sec->World_Offset,0,sbPtr->SBdptr);
				if (LOS_ObjectHitPtr) {
					if (SBIsEnvironment(LOS_ObjectHitPtr->ObStrategyBlock)) {
						if (LOS_Lambda<2000) {
							Marine_MirrorSuspectPoint(sbPtr);
						}
					}
				}
			}

		}

	}

	/* Think Sequences. */

	HandleWaitingAnimations(sbPtr);

	if ((NpcSquad.alertZone)&&(marineStatusPointer->Mission!=MM_LocalGuard)
		&&(marineStatusPointer->Mission!=MM_NonCom)) {
		/* Are we already there? */
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			if (NpcSquad.responseLevel>0) {
				/* Picked up a target.  Can we move to respond? */
				AIMODULE *targetModule=0;
				targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
				if (targetModule) {
					return(SRC_Request_Respond);
				}
			}
		}
	}

	/* still waiting: decrement timer */
	marineStatusPointer->stateTimer-=NormalFrameTime;
	
	if(marineStatusPointer->stateTimer<=0)
	{

		/* Might want to wander. */

		if ((FastRandom()&65535)<2048)
		{
			/* we should be wandering... we're bored of waiting. */
			return(SRC_Request_Wander);
		}

		/* No, I'm happy waiting. */
		marineStatusPointer->stateTimer=MARINE_NEARWAITTIME;
		return(SRC_No_Change);
	}

	/* Well, we're stuck with waiting. */
	
	if (marineStatusPointer->suspicious) {
		/* Stay facing the suspect point. */
		return(SRC_No_Change);
	}

	if (marineStatusPointer->gotapoint) {

		VECTORCH orientationDirn;
		int correctlyOrientated;

		orientationDirn.vx = marineStatusPointer->wanderData.worldPosition.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->wanderData.worldPosition.vz - sbPtr->DynPtr->Position.vz;

		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

	} else {
		GetPointToFaceMarineTowards(sbPtr,&marineStatusPointer->wanderData.worldPosition);
	}

	return(SRC_No_Change);
}

/*----------------------- Patrick 16/6/97 ------------------------
  Special state for dying:
  NB once we have entered this state, we are locked into it until
  we are dead.
  ----------------------------------------------------------------*/
static void Execute_Dying(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    	

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(marineStatusPointer);

	#if 0	
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	sbPtr->DynPtr->LinImpulse.vx = 0;
	sbPtr->DynPtr->LinImpulse.vy = 0;
	sbPtr->DynPtr->LinImpulse.vz = 0;
	#endif

	{
		DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = marineStatusPointer->stateTimer/2;

		}
	}
	
	marineStatusPointer->stateTimer -= NormalFrameTime;

	#if 0
	if ( (sbPtr->DynPtr->Position.vx=sbPtr->DynPtr->PrevPosition.vx)
		|| (sbPtr->DynPtr->Position.vx=sbPtr->DynPtr->PrevPosition.vy)
		|| (sbPtr->DynPtr->Position.vx=sbPtr->DynPtr->PrevPosition.vz)) {

		/* Now, turn off collisions? */
		if(sbPtr->DynPtr)
		{
			sbPtr->DynPtr->IsStatic	= 1;
			sbPtr->DynPtr->DynamicsType	= DYN_TYPE_NO_COLLISIONS;
			sbPtr->DynPtr->GravityOn = 0;
		}

	}
	#endif


	/* Did marine die with the trigger held down? */
	if (marineStatusPointer->lastroundhit==-2) {
		/* Is there a gunflash? */
		if(marineStatusPointer->My_Gunflash_Section) {
			/* But is it still attached? */
			if (marineStatusPointer->My_Gunflash_Section->my_controller==&(marineStatusPointer->HModelController)) {
				/* Keep firing! */
				LOCALASSERT(marineStatusPointer->My_Weapon->WeaponMisfireFunction);
				/* Shouldn't be doing this without knowing why. */
				(*marineStatusPointer->My_Weapon->WeaponMisfireFunction)(marineStatusPointer->My_Gunflash_Section,&marineStatusPointer->weapon_variable);
			}
		}
	}

	/* Do we want to trim off the weapons? */

	if (marineStatusPointer->HModelController.keyframe_flags) {
		SECTION *root;

		root=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->TemplateName);

		TrimToTemplate(sbPtr,&marineStatusPointer->HModelController,
			root, 1);
	}

}

static void MarineFireFlameThrower(STRATEGYBLOCK *sbPtr) {

	VECTORCH null_vec;
	MARINE_STATUS_BLOCK *marineStatusPointer;    	

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(marineStatusPointer);

	null_vec.vx=0;
	null_vec.vy=0;
	null_vec.vz=0;

	GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);

	FireFlameThrower(&marineStatusPointer->My_Gunflash_Section->World_Offset,&null_vec,
		&marineStatusPointer->My_Gunflash_Section->SecMat,0,&marineStatusPointer->weapon_variable);

}

void MarineMisfireFlameThrower(SECTION_DATA *muzzle, int *timer) {

	VECTORCH null_vec;

	null_vec.vx=0;
	null_vec.vy=0;
	null_vec.vz=0;

	FireFlameThrower(&muzzle->World_Offset,&null_vec,&muzzle->SecMat,0,timer);

}

/*----------------------- Patrick 18/4/97 ------------------------
  Support for gunflash objects. These are for use by AI marines
  and also network ghosted players
  ----------------------------------------------------------------*/

/* this function is specifically for marines, and creates a gun flash
pointing in the direction of the target point */
static void CreateMarineGunFlash(STRATEGYBLOCK *sbPtr)
{
	VECTORCH firingDirn,firingPoint;
	MARINE_STATUS_BLOCK *marineStatusPointer;    	
	int	firingOffsetUp,firingOffsetInfront,firingOffsetAcross;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(marineStatusPointer);

	/* get the firing point offsets:*/
	if(marineStatusPointer->IAmCrouched)
	{
		firingOffsetUp = MARINE_FIRINGPOINT_UP;
		firingOffsetInfront = MARINE_FIRINGPOINT_INFRONT;
		firingOffsetAcross = MARINE_FIRINGPOINT_ACROSS;
	}
	else
	{
		firingOffsetUp = MARINE_FIRINGPOINT_UP_CROUCHED;
		firingOffsetInfront = MARINE_FIRINGPOINT_INFRONT_CROUCHED;
		firingOffsetAcross = MARINE_FIRINGPOINT_ACROSS_CROUCHED;
	}

	/* find the firing direction */
	firingDirn.vx = marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	firingDirn.vy = marineStatusPointer->weaponTarget.vy - sbPtr->DynPtr->Position.vy;
	firingDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	Normalise(&firingDirn);
	{
		VECTORCH yNormal = {0,-65536,0};
		VECTORCH tempDirn;
		VECTORCH acrossDirn;

		/* now find firing point (conceptually, the end of the weapon muzzle)... */
		firingPoint = sbPtr->DynPtr->Position;		
		firingPoint.vx += MUL_FIXED(firingDirn.vx,firingOffsetInfront);
		firingPoint.vz += MUL_FIXED(firingDirn.vz,firingOffsetInfront);		

		tempDirn = firingDirn;
		tempDirn.vy = 0;
		Normalise(&tempDirn);
		CrossProduct(&tempDirn,&yNormal,&acrossDirn);		
		Normalise(&acrossDirn);
		firingPoint.vx += MUL_FIXED(tempDirn.vx,firingOffsetAcross);
		firingPoint.vz += MUL_FIXED(tempDirn.vz,firingOffsetAcross);
		firingPoint.vy += MUL_FIXED(yNormal.vy,firingOffsetUp);
	}


	LOCALASSERT(marineStatusPointer->myGunFlash==NULL);
	GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
	marineStatusPointer->myGunFlash = AddNPCGunFlashEffect
 						  (
 						  	&marineStatusPointer->My_Gunflash_Section->World_Offset,
 							&marineStatusPointer->My_Gunflash_Section->SecMat,
 							marineStatusPointer->My_Weapon->SfxID
 						  );
}

/* Add a gun flash at the given position & orientation */
DISPLAYBLOCK* AddNPCGunFlashEffect(VECTORCH *position, MATRIXCH* orientation, enum SFX_ID sfxID)
{
	DISPLAYBLOCK *dPtr;

	dPtr = CreateSFXObject(sfxID);

	if(dPtr)												
	{
		dPtr->ObMyModule = NULL;					                    
		dPtr->ObWorld = *position;
		dPtr->ObMat = *orientation;
		//CreateEulerMatrix(orientation, &dPtr->ObMat);
		//TransposeMatrixCH(&dPtr->ObMat);	
		AddLightingEffectToObject(dPtr,LFX_MUZZLEFLASH);
		GLOBALASSERT(dPtr->SfxPtr);
		dPtr->SfxPtr->EffectDrawnLastFrame=0;
	}
	return dPtr;
}

/* Remove the gunflash */
void RemoveNPCGunFlashEffect(DISPLAYBLOCK* dPtr)
{
	DestroyActiveObject(dPtr);
}

/* Update the gunflash effect given a new position and orientation */
void MaintainNPCGunFlashEffect(DISPLAYBLOCK* dPtr, VECTORCH *position, MATRIXCH* orientation)
{
	dPtr->ObWorld = *position;
	dPtr->ObMat = *orientation;

	/* oh, oh, and re-add the lighting effect  */	
	AddLightingEffectToObject(dPtr,LFX_MUZZLEFLASH);
}



/* Patrick: 2/7/97 --------------------------------------------------------
   Some marine support functions
   -------------------------------------------------------------------------*/
static void SetMarineAnimationSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(length!=0);

	if (tweening<=0) {
		InitHModelSequence(&marineStatusPointer->HModelController,(int)type,subtype,length);
	} else {	
		InitHModelTweening(&marineStatusPointer->HModelController, tweening, (int)type,subtype,length, 1);
	}

	ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
}

static void SetMarineAnimationSequence_Null(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweening) {
	/* Ha! */
	#if (NEW_ANIM_SYSTEM==0) 
	SetMarineAnimationSequence(sbPtr,type,subtype,length,tweening);
	#endif

}

static int MarineCanSeeTarget(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int targetIsCloaked,targetTaunted;
	VECTORCH offset;
	int dist;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Target==NULL) {
		/* You can't see nothin. */
		return(0);
	}

	targetIsCloaked=0;
	targetTaunted=0;

	if (marineStatusPointer->Target==Player->ObStrategyBlock) {
		/* test for player being cloaked */
		LOCALASSERT(playerStatusPtr);
		if (AvP.PlayerType==I_Predator) {
		
			if((playerStatusPtr->cloakOn==1)&&(playerStatusPtr->cloakPositionGivenAway==0)) {
				targetIsCloaked=1;
			}
		}
		if (playerStatusPtr->tauntTimer) {
			/* Idiot. */
			targetIsCloaked=0;
			/* On the other hand, */
			targetTaunted=1;
		}
	
	} else {
		/* Test for NPC predators being cloaked, or aliens hiding? */
		if (marineStatusPointer->Target->I_SBtype==I_BehaviourPredator) {
			PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)marineStatusPointer->Target->SBdataptr;
			GLOBALASSERT(predStatus);
			if (predStatus->CloakStatus==PCLOAK_On) {
				targetIsCloaked=1;
			}
		}
	}

	offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->Target->DynPtr->Position.vx;
	offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->Target->DynPtr->Position.vy;
	offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->Target->DynPtr->Position.vz;
	dist=Approximate3dMagnitude(&offset);

	/* If a marine is suspicious, and the target is within 2m of the suspect_point... */
	if (marineStatusPointer->suspicious) {
		/* Detect on the incidentFlag? */
		if (marineStatusPointer->incidentFlag) {
			int dice,targetnum;

			dice=(FastRandom()&65535);
			targetnum=(marineStatusPointer->Skill);

			targetnum=MakeModifiedTargetNum(targetnum,dist);

			if (marineStatusPointer->Target==Player->ObStrategyBlock) {
				if (AvP.PlayerType==I_Predator) {
					if (playerStatusPtr->cloakOn==1) {
						dice=MUL_FIXED(dice,playerStatusPtr->CloakingEffectiveness);
					}		
				}
			} else if (marineStatusPointer->Target->I_SBtype==I_BehaviourPredator) {
				PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)marineStatusPointer->Target->SBdataptr;
				GLOBALASSERT(predStatus);
				if (predStatus->CloakStatus==PCLOAK_On) {
					dice=MUL_FIXED(dice,predStatus->CloakingEffectiveness);
				}
			}
			
			if (dice<targetnum) {
				targetIsCloaked=0;
			}
		}
		if (dist<MARINE_AUTODETECT_RANGE) {
			/* Too close to fool anyone. */
			targetIsCloaked=0;
		}
	}

	if (marineStatusPointer->Target==Player->ObStrategyBlock) {
		/* Alien test is now here, since it uses probability differently. */
		if (AvP.PlayerType==I_Alien) {
			if (!AlienPCIsCurrentlyVisible(marineStatusPointer->incidentFlag,sbPtr)) {
				if (playerStatusPtr->tauntTimer) {
					/* Idiot. */
					targetIsCloaked=0;
					/* Taunted flag should already be set. */
				} else {
					targetIsCloaked=1;
				}
			}
		}
	}

	if (marineStatusPointer->sawlastframe) {
		if (marineStatusPointer->incidentFlag) {
			/* Chance of losing target. */
			VECTORCH offset;
			int speed;
	
			offset.vx=marineStatusPointer->Target->DynPtr->Position.vx-marineStatusPointer->Target->DynPtr->PrevPosition.vx;
			offset.vy=marineStatusPointer->Target->DynPtr->Position.vy-marineStatusPointer->Target->DynPtr->PrevPosition.vy;
			offset.vz=marineStatusPointer->Target->DynPtr->Position.vz-marineStatusPointer->Target->DynPtr->PrevPosition.vz;

			/* ...compute speed factor... */
			speed=Magnitude(&offset);
			speed=DIV_FIXED(speed,NormalFrameTime);

			if (speed>50) {
				/* The faster you move, the more likely to be lost. */
				speed<<=1;
				if ((FastRandom()&65535)>speed) {
					/* Retain target! */
					targetIsCloaked=0;
				}
			} else {
				/* Can't lose them if they're still. */
				targetIsCloaked=0;
			}
		} else {
			targetIsCloaked=0;
		}
	}

	if (marineStatusPointer->Target->SBDamageBlock.IsOnFire) {
		/* Oh come ON. */
		targetIsCloaked=0;
	}

	if(!(NPCCanSeeTarget(sbPtr,marineStatusPointer->Target, MARINE_NEAR_VIEW_WIDTH))) {
		if (marineStatusPointer->sawlastframe) {
			marineStatusPointer->sawlastframe=2;
			/* I'm suspicious now. */
			marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
			marineStatusPointer->suspect_point=marineStatusPointer->Target->DynPtr->Position;
			/* And unset previous_suspicion. */
			marineStatusPointer->previous_suspicion=0;
			marineStatusPointer->using_squad_suspicion=0;
		}
		return(0);
	}

	if (targetIsCloaked) {
		return(0);
	}
	marineStatusPointer->sawlastframe=1;

	if ((targetTaunted)&&(marineStatusPointer->Target)&&(marineStatusPointer->Android==0)) {

		if (dist<16834) {
			dist=16384-dist;
			dist<<=2;
			marineStatusPointer->Courage-=MUL_FIXED((NormalFrameTime<<1),dist);
		}
	}
	return(1);
}

static int MarineCanSeeObject(STRATEGYBLOCK *sbPtr,STRATEGYBLOCK *target)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int targetIsCloaked, targetTaunted, targetWasCloaked;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	VECTORCH offset;
	int dist;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (target==NULL) {
		/* You can't see nothin. */
		return(0);
	}

	targetIsCloaked=0;
	targetTaunted=0;
	targetWasCloaked=0;

	if (target==Player->ObStrategyBlock) {
		/* test for player being cloaked */
		LOCALASSERT(playerStatusPtr);
		if (AvP.PlayerType==I_Predator) {

			if((playerStatusPtr->cloakOn==1)&&(playerStatusPtr->cloakPositionGivenAway==0)) {
				targetIsCloaked=1;
				targetWasCloaked=1;
			}
		}
		if (playerStatusPtr->tauntTimer) {
			/* Idiot. */
			targetIsCloaked=0;
			/* On the other hand, */
			targetTaunted=1;
		}
	
	} else {
		/* Test for NPC predators being cloaked, or aliens hiding? */
		if (target->I_SBtype==I_BehaviourPredator) {
			PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)target->SBdataptr;
			GLOBALASSERT(predStatus);
			if (predStatus->CloakStatus==PCLOAK_On) {
				targetIsCloaked=1;
				targetWasCloaked=1;
			}
		}
	}

	offset.vx=sbPtr->DynPtr->Position.vx-target->DynPtr->Position.vx;
	offset.vy=sbPtr->DynPtr->Position.vy-target->DynPtr->Position.vy;
	offset.vz=sbPtr->DynPtr->Position.vz-target->DynPtr->Position.vz;
	dist=Approximate3dMagnitude(&offset);

	/* If a marine is suspicious, and the target is within 2m of the suspect_point... */
	if (marineStatusPointer->suspicious) {
		/* Detect on the incidentFlag? */
		if (marineStatusPointer->incidentFlag) {
			int dice,targetnum;

			dice=(FastRandom()&65535);
			targetnum=(marineStatusPointer->Skill);

			targetnum=MakeModifiedTargetNum(targetnum,dist);

			if (target==Player->ObStrategyBlock) {
				if (AvP.PlayerType==I_Predator) {
					if (playerStatusPtr->cloakOn==1) {
						dice=MUL_FIXED(dice,playerStatusPtr->CloakingEffectiveness);
					}		
				}
			} else if (target->I_SBtype==I_BehaviourPredator) {
				PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)target->SBdataptr;
				GLOBALASSERT(predStatus);
				if (predStatus->CloakStatus==PCLOAK_On) {
					dice=MUL_FIXED(dice,predStatus->CloakingEffectiveness);
				}
			}

			if (dice<targetnum) {
				targetIsCloaked=0;
			}
		}
		if (dist<MARINE_AUTODETECT_RANGE) {
			/* Too close to fool anyone. */
			targetIsCloaked=0;
		}
	}

	if (target==Player->ObStrategyBlock) {
		/* Alien test is now here, since it uses probability differently. */
		if (AvP.PlayerType==I_Alien) {
			if (!AlienPCIsCurrentlyVisible(marineStatusPointer->incidentFlag,sbPtr)) {
				if (playerStatusPtr->tauntTimer) {
					/* Idiot. */
					targetIsCloaked=0;
					/* Taunted flag should already be set. */
				} else {
					targetIsCloaked=1;
					targetWasCloaked=1;
				}
			}
		}
	}

	if (marineStatusPointer->sawlastframe) {
		if (marineStatusPointer->incidentFlag) {
			/* Chance of losing target. */
			VECTORCH offset;
			int speed;
	
			offset.vx=target->DynPtr->Position.vx-target->DynPtr->PrevPosition.vx;
			offset.vy=target->DynPtr->Position.vy-target->DynPtr->PrevPosition.vy;
			offset.vz=target->DynPtr->Position.vz-target->DynPtr->PrevPosition.vz;

			/* ...compute speed factor... */
			speed=Magnitude(&offset);
			speed=DIV_FIXED(speed,NormalFrameTime);

			if (speed>50) {
				/* The faster you move, the more likely to be lost. */
				speed<<=1;
				if ((FastRandom()&65535)>speed) {
					/* Retain target! */
					targetIsCloaked=0;
				}
			} else {
				/* Can't lose them if they're still. */
				targetIsCloaked=0;
			}
		} else {
			targetIsCloaked=0;
		}
	}

	if (target->SBDamageBlock.IsOnFire) {
		/* Oh come ON. */
		targetIsCloaked=0;
	}

	/* NO saw last frame usage! */

	if (targetIsCloaked) {
		return(0);
	}

	if(!(NPCCanSeeTarget(sbPtr,target, MARINE_NEAR_VIEW_WIDTH))) {
		return(0);
	}
	
	if ((targetWasCloaked)&&(marineStatusPointer->Target==NULL)) {
		/* Exhibit suprise? */
		if ((FastRandom()&65535)<16384) {
			Marine_SurpriseSound(sbPtr);
		}
	}

	if ((targetTaunted)&&(marineStatusPointer->Target)&&(marineStatusPointer->Android==0)) {
		/* It must be the player. */
		if (dist<16834) {
			dist=16384-dist;
			dist<<=2;
			marineStatusPointer->Courage-=MUL_FIXED((NormalFrameTime<<1),dist);
		}
	}
	return(1);
}

void WarnMarineOfAttack(STRATEGYBLOCK *marine,STRATEGYBLOCK *attacker) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	GLOBALASSERT(marine);
	
	GLOBALASSERT(marine->I_SBtype==I_BehaviourMarine);
	
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(marine->SBdataptr);    
	GLOBALASSERT(marineStatusPointer);	          		

	GLOBALASSERT(attacker);
	
	/* Test? */	

	if(MarineCanSeeObject(marine,attacker)) {
		
		/* Remember your suspicion! */
		marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
		marineStatusPointer->suspicious=1; /* It's right there! */
		marineStatusPointer->Target=attacker;
		
		COPY_NAME(marineStatusPointer->Target_SBname,marineStatusPointer->Target->SBname);

		PointAlert(2,&attacker->DynPtr->Position);
	}

}

static int MarineIsAwareOfTarget(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int targetIsCloaked;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	VECTORCH offset;
	int dist;

	/* Like MarineCanSeeTarget, but the 'Far' version:  should it hunt? */

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Target!=NULL) {

		/* Okay, let's try this sense malarky. */
		
		/* Motion tracker, only when scanning. */
	
		if (marineStatusPointer->mtracker_timer==0) {
			if (marineStatusPointer->Target->DynPtr) {
				DYNAMICSBLOCK *tDynPtr;
				MATRIXCH WtoL;
				VECTORCH offset;
				/* Arc reject. */
	
				tDynPtr=marineStatusPointer->Target->DynPtr;
	
				offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->Target->DynPtr->Position.vx;
				offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->Target->DynPtr->Position.vy;
				offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->Target->DynPtr->Position.vz;
				
				WtoL=sbPtr->DynPtr->OrientMat;
				TransposeMatrixCH(&WtoL);
				RotateVector(&offset,&WtoL);
	
				if (offset.vz<=0) {
					#if 0
					if (
						(tDynPtr->Position.vx!=tDynPtr->PrevPosition.vx)
						||(tDynPtr->Position.vx!=tDynPtr->PrevPosition.vx)
						||(tDynPtr->Position.vx!=tDynPtr->PrevPosition.vx)
						) {
					#else
					if (ObjectShouldAppearOnMotionTracker(marineStatusPointer->Target)) {
					#endif
						int range;
			
						range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
			
						if (range<=MOTIONTRACKER_RANGE) {
							marineStatusPointer->suspicious=MARINE_PARANOIA_TIME; /* It might be there. */
							marineStatusPointer->suspect_point=marineStatusPointer->Target->DynPtr->Position;
							/* Set this to zero when you get a *new* suspicion. */
							marineStatusPointer->previous_suspicion=0;
							marineStatusPointer->using_squad_suspicion=0;
							tracker_noise=2; /* Wheep! */
							return(1);
						}
					}
				}
			}
		}
	
		/* So, there's nothing on the scanner. */
	
		targetIsCloaked=0;
	
		/* Far visibility. */
	
		if (marineStatusPointer->Target==Player->ObStrategyBlock) {
			/* test for player being cloaked */
			LOCALASSERT(playerStatusPtr);
			if (AvP.PlayerType==I_Predator) {
			
				if((playerStatusPtr->cloakOn==1)&&(playerStatusPtr->cloakPositionGivenAway==0)) {
					targetIsCloaked=1;
				}
			}
			if (playerStatusPtr->tauntTimer) {
				/* Idiot. */
				targetIsCloaked=0;
			}
	
		} else {
			/* Test for NPC predators being cloaked, or aliens hiding? */
			if (marineStatusPointer->Target->I_SBtype==I_BehaviourPredator) {
				PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)marineStatusPointer->Target->SBdataptr;
				GLOBALASSERT(predStatus);
				if (predStatus->CloakStatus==PCLOAK_On) {
					targetIsCloaked=1;
				}
			}
		}

		offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->Target->DynPtr->Position.vx;
		offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->Target->DynPtr->Position.vy;
		offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->Target->DynPtr->Position.vz;
		dist=Approximate3dMagnitude(&offset);
	
		/* If a marine is suspicious, and the target is within 2m of the suspect_point... */
		if (marineStatusPointer->suspicious) {
			/* Detect on the incidentFlag? */
			if (marineStatusPointer->incidentFlag) {
				int dice,targetnum;

				dice=(FastRandom()&65535);
				targetnum=(marineStatusPointer->Skill);

				targetnum=MakeModifiedTargetNum(targetnum,dist);

				if (marineStatusPointer->Target==Player->ObStrategyBlock) {
					if (AvP.PlayerType==I_Predator) {
						if (playerStatusPtr->cloakOn==1) {
							dice=MUL_FIXED(dice,playerStatusPtr->CloakingEffectiveness);
						}		
					}
				} else if (marineStatusPointer->Target->I_SBtype==I_BehaviourPredator) {
					PREDATOR_STATUS_BLOCK *predStatus=(PREDATOR_STATUS_BLOCK *)marineStatusPointer->Target->SBdataptr;
					GLOBALASSERT(predStatus);
					if (predStatus->CloakStatus==PCLOAK_On) {
						dice=MUL_FIXED(dice,predStatus->CloakingEffectiveness);
					}
				}
				
				if (dice<targetnum) {
					targetIsCloaked=0;
				}
			}
			if (dist<MARINE_AUTODETECT_RANGE) {
				/* Too close to fool anyone. */
				targetIsCloaked=0;
			}
		}
	
		if (marineStatusPointer->Target==Player->ObStrategyBlock) {
			/* Alien test is now here, since it uses probability differently. */
			if (AvP.PlayerType==I_Alien) {
				if (!AlienPCIsCurrentlyVisible(marineStatusPointer->incidentFlag,sbPtr)) {
					if (playerStatusPtr->tauntTimer) {
						/* Idiot. */
						targetIsCloaked=0;
					} else {
						targetIsCloaked=1;
					}
				}
			}
		}

		if (marineStatusPointer->sawlastframe) {
			if (marineStatusPointer->incidentFlag) {
				/* Chance of losing target. */
				VECTORCH offset;
				int speed;
		
				offset.vx=marineStatusPointer->Target->DynPtr->Position.vx-marineStatusPointer->Target->DynPtr->PrevPosition.vx;
				offset.vy=marineStatusPointer->Target->DynPtr->Position.vy-marineStatusPointer->Target->DynPtr->PrevPosition.vy;
				offset.vz=marineStatusPointer->Target->DynPtr->Position.vz-marineStatusPointer->Target->DynPtr->PrevPosition.vz;
	
				/* ...compute speed factor... */
				speed=Magnitude(&offset);
				speed=DIV_FIXED(speed,NormalFrameTime);
	
				if (speed>50) {
					/* The faster you move, the more likely to be lost. */
					speed<<=1;
					if ((FastRandom()&65535)>speed) {
						/* Retain target! */
						targetIsCloaked=0;
					}
				} else {
					/* Can't lose them if they're still. */
					targetIsCloaked=0;
				}
			} else {
				targetIsCloaked=0;
			}
		}
	
		if (marineStatusPointer->Target->SBDamageBlock.IsOnFire) {
			/* Oh come ON. */
			targetIsCloaked=0;
		}
	
		if (targetIsCloaked==0) {
			if (!sbPtr->SBdptr) {
				if (marineStatusPointer->Target->containingModule) {
					if (IsModuleVisibleFromModule(marineStatusPointer->Target->containingModule,sbPtr->containingModule)) {
						/* Remember your suspicion! */
						marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
						/* Seen, unset suspicion. */
						marineStatusPointer->suspicious=1;
						return(1);
					}
				}
			} else {
				if (MarineCanSeeTarget(sbPtr)) {
					/* Remember your suspicion! */
					marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
					/* Near visibility - seen target, unset suspicion. */
					marineStatusPointer->suspicious=1;
					return(1);
				}
			}
		}
	}	

	/* Lastly, is there something we want to investigate? */
							
	if (marineStatusPointer->suspicious) {
		if (!sbPtr->SBdptr) {
			MODULE *targetModule;
			/* Far case. */
			targetModule=ModuleFromPosition(&marineStatusPointer->suspect_point,sbPtr->containingModule);
			/* Target isn't guaranteed, is it? */
			if (targetModule) {
				if (IsModuleVisibleFromModule(targetModule,sbPtr->containingModule)) {
					/* Seen the point, unset suspicion?  Maybe not. */
					#if 0
					/* Remember your suspicion! */
					marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
					marineStatusPointer->suspicious=1;
					/* Just let it time out instead. */
					#endif
					/* Shouldn't have a target now. */
					marineStatusPointer->Target=NULL;
					return(0);
					/* If there was something there, we should have picked it up by now. */
				}
			}
			/* Still suspicious. */
		} else {
			/* Near case. */
			VECTORCH offset;
			MATRIXCH WtoL;
			/* Arc reject. */

			offset.vx=sbPtr->DynPtr->Position.vx-marineStatusPointer->suspect_point.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-marineStatusPointer->suspect_point.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-marineStatusPointer->suspect_point.vz;
			
			WtoL=sbPtr->DynPtr->OrientMat;
			TransposeMatrixCH(&WtoL);
			RotateVector(&offset,&WtoL);
			/* Do reject. */
			if (MarineSight_FrustrumReject(sbPtr,&offset,NULL)) {
				if (IsThisObjectVisibleFromThisPosition(sbPtr->SBdptr,&marineStatusPointer->suspect_point,NPC_MAX_VIEWRANGE)) {
					/* I know what you're going to say.  That's backwards. */
					#if 0
					/* Remember your suspicion! */
					marineStatusPointer->previous_suspicion=marineStatusPointer->suspicious;
					marineStatusPointer->suspicious=1; /* Flag for unsetting nicely. */
					/* Now, let's try just timing it out. */
					#endif
					/* By this point I would have thought there's no valid target. */
					marineStatusPointer->Target=NULL;
					return(0);
					/* Well, show me 'IsThisPositionVisibleFromThisObject' and I'll be happy to change it. */
				}
			}
		}
	}

	return(0);
}

static int MarineShouldBeCrawling(STRATEGYBLOCK *sbPtr)
{
	if(sbPtr->containingModule->m_flags & MODULEFLAG_AIRDUCT) return 1;
	return 0;
}

static STATE_RETURN_CONDITION Execute_MNS_DischargeFlamethrower(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* look after the gun flash */
	if(marineStatusPointer->myGunFlash) {
		/* No gunflash, neither. */
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
	}

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state*/
	if(!MarineCanSeeTarget(sbPtr))
	{

		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	GLOBALASSERT(marineStatusPointer->Target);
	NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
	/* Fix weapon target! */
	if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
		marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
			marineStatusPointer->My_Weapon->TargetCallibrationShift);
		marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
			marineStatusPointer->My_Weapon->TargetCallibrationShift);
		marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
			marineStatusPointer->My_Weapon->TargetCallibrationShift);
	}
	
	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}

	/* Are we out of range? */
	{
		int range;

		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

		if ((marineStatusPointer->My_Weapon->MaxRange!=-1) &&
			(range>=marineStatusPointer->My_Weapon->MaxRange)) {
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			return(SRC_Request_Approach);

		}
	}

	/* orientate to firing point first */
	orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	orientationDirn.vy = 0;
	orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

	/* I have a cunning plan... */
	{
		DELTA_CONTROLLER *delta;

		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			if (!(DeltaAnimation_IsFinished(delta))) {
				correctlyOrientated=0;
			}
		}
	}

	/* we are not correctly orientated to the target: this could happen because we have
	just entered this state, or the target has moved during firing*/
	
	if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
		}
		return(SRC_No_Change);
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_Reload);
	}

	/* at this point we are correctly orientated: if we have no gunflash yet,
	and our state timer is set to marine_near_firetime then we have either
	just started firing, or have become dis-orienated between bursts. This is a good
	time to consider firing a grenade... */

	/* No grenades with FT. */
		
	/* look after the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
	else 
	{ 
		/* SID_INCIN_LOOP? */
		Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
		Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	MarineFireFlameThrower(sbPtr);

	/* Lighting? */
	if (sbPtr->SBdptr) {
		AddLightingEffectToObject(sbPtr->SBdptr,LFX_MUZZLEFLASH);
	}

	if (marineStatusPointer->clipammo>0) {
		marineStatusPointer->clipammo-=NormalFrameTime;
		if (marineStatusPointer->clipammo<0) {
			marineStatusPointer->clipammo=0;
		}
	}

	if(marineStatusPointer->stateTimer > 0)	return(SRC_No_Change);
	
	
	{
		/* we are far enough away, so return to approach */
	
		/* ... and remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
	
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Approach);
	}
}

static void DischargeLOSWeapon_Core(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH relPos,relPos2;
	int mod,hitroll,range,volleytime,volleyrounds;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* Snipped from DischargeLOSWeapon. */

	if (marineStatusPointer->Target) {
		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
	
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
	} else {
		/* Just a number. */
		range=10000;
	}

	/* look after the gun flash */
	if(marineStatusPointer->myGunFlash) {
		MaintainMarineGunFlash(sbPtr);
	} else {
		CreateMarineGunFlash(sbPtr);
	}

	/* look after the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
	else 
	{ 
		Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
		Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;
		
	/* Volleysize is now rounds fired this state. */

	volleytime=marineStatusPointer->My_Weapon->FiringTime-marineStatusPointer->stateTimer;
	/* It was that or reverse the state timer for this state. */
	volleyrounds=MUL_FIXED(volleytime,marineStatusPointer->My_Weapon->FiringRate);
	volleyrounds>>=ONE_FIXED_SHIFT;

	volleyrounds-=marineStatusPointer->volleySize;
	marineStatusPointer->volleySize+=volleyrounds;

	LOCALASSERT(volleyrounds>=0);

	if (marineStatusPointer->clipammo!=-1) {
		/* We're counting ammo. */
		if (volleyrounds>marineStatusPointer->clipammo) {
			volleyrounds=marineStatusPointer->clipammo;
		}
		marineStatusPointer->clipammo-=volleyrounds;
		LOCALASSERT(marineStatusPointer->clipammo>=0);
	}

	marineStatusPointer->roundsForThisTarget+=volleyrounds;

	/* Now hit the target with volleyrounds bullets. */

	mod=SpeedRangeMods(&relPos,&relPos2);

	hitroll=marineStatusPointer->Skill; /* Marine skill... */
	if (marineStatusPointer->Target==Player->ObStrategyBlock) {
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		LOCALASSERT(playerStatusPtr);
		if ( (AvP.PlayerType==I_Alien)
			||((AvP.PlayerType==I_Predator)&&(playerStatusPtr->cloakOn==1))) {
			/* Vs the player, lighting effects on aliens and cloaked preds. */
			hitroll=MUL_FIXED(hitroll,((CurrentLightAtPlayer>>1)+32767));
		}
	}
	hitroll-=mod;
	hitroll+=marineStatusPointer->My_Weapon->Accuracy;

	/* Here we go... */
	{
		#define SUSTAINMOD (ONE_FIXED>>3)
		int a,hits;

		hits=0;

		for (a=0; a<volleyrounds; a++) {
			
			int realhitroll;

			if (marineStatusPointer->lastroundhit) {
				realhitroll=hitroll+SUSTAINMOD;
			} else {
				realhitroll=hitroll;
			}

			if (marineStatusPointer->Target) {
				if (marineStatusPointer->lasthitsection) {
					/* Verify this section is valid? */
					HMODELCONTROLLER *tctrl=NULL;
	
					if (marineStatusPointer->Target->SBdptr) {
						tctrl=marineStatusPointer->Target->SBdptr->HModelControlBlock;
					}

					if ((marineStatusPointer->lasthitsection->flags&section_data_notreal)
						|| (marineStatusPointer->lasthitsection->flags&section_data_terminate_here)
						|| ( (tctrl!=NULL)&&(tctrl!=marineStatusPointer->lasthitsection->my_controller)) ) {
						/* Invalid. */
						marineStatusPointer->lasthitsection=NULL;
					}
				}

				if ( (marineStatusPointer->lastroundhit==0)||(marineStatusPointer->lasthitsection==NULL)) {
					marineStatusPointer->lasthitsection=HitLocationRoll(marineStatusPointer->Target,sbPtr);
				}
			}

			if ((FastRandom()&65535)<realhitroll) {
				hits++;
				marineStatusPointer->lastroundhit=1;
			} else {
				marineStatusPointer->lastroundhit=0;
			}

			if (marineStatusPointer->Target==NULL) {
				hits=0;
			}
		}

		/* Handle Damage. */
		{
			VECTORCH attack_dir,rel_pos;
			VECTORCH shotvector;

			shotvector.vx=0;
			shotvector.vy=0;
			shotvector.vz=65535;
			RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);
			
			/* DO DAMAGE TO TARGET HERE */
			#if MARINE_STATE_PRINT
			textprint("Hits = %d\n",hits);
			#endif
		
			if (hits!=0) {
				
				int range2;

				GLOBALASSERT(marineStatusPointer->Target);

				rel_pos.vx=marineStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
				rel_pos.vy=marineStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
				rel_pos.vz=marineStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;
				range2=Approximate3dMagnitude(&rel_pos);

				if (VerifyHitShot(sbPtr,marineStatusPointer->Target,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, hits,range2)) {
					/* If 0, hits have been dealt with. */					
					GetDirectionOfAttack(marineStatusPointer->Target,&rel_pos,&attack_dir);

					if (marineStatusPointer->lasthitsection) {
						CauseDamageToHModel(marineStatusPointer->lasthitsection->my_controller, marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED*hits, marineStatusPointer->lasthitsection,&attack_dir,NULL,0);
					} else {
						CauseDamageToObject(marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED*hits,&attack_dir);
					}
				}

			}

			if ((volleyrounds-hits)>0) {
				GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
			
				if (marineStatusPointer->Target) {
					ProjectNPCShot(sbPtr, marineStatusPointer->Target, &marineStatusPointer->My_Gunflash_Section->World_Offset,&marineStatusPointer->My_Gunflash_Section->SecMat, marineStatusPointer->My_Weapon->Ammo_Type, (volleyrounds-hits));
				} else {
					/* Like a miss, so it's inaccurate. */
					CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, volleyrounds,1);
				}
			}

		}

	}

}

static STATE_RETURN_CONDITION Execute_MNS_DischargeLOSWeapon(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state*/
	if (marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize) {
		/* Keep firing! */
	} else {
		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			EndMarineMuzzleFlash(sbPtr);
			#else
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}

			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#endif

			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			marineStatusPointer->lastroundhit=0;
			marineStatusPointer->lasthitsection=NULL;
			return(SRC_Request_Wait);
		}
	}

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		EndMarineMuzzleFlash(sbPtr);
		return(SRC_Request_Reload);
	}		


	if (marineStatusPointer->Target) {
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* Here we must have a target.  Renew suspicion for new arrivals. */
		if (NpcSquad.Squad_Suspicion==0) {
			PointAlert(2,&marineStatusPointer->weaponTarget);
		}
	}
	/* Otherwise, stay facing the same way. */

	/* orientate to firing point first */
	orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	orientationDirn.vy = 0;
	orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

	/* I have a cunning plan... */
	{
		DELTA_CONTROLLER *delta;

		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			if (!(DeltaAnimation_IsFinished(delta))) {
				correctlyOrientated=0;
			}
		}
	}
	/* I have another cunning plan... */
	if ((marineStatusPointer->volleySize>0)&&
		(marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		correctlyOrientated=1;
	}


	/* we are not correctly orientated to the target: this could happen because we have
	just entered this state, or the target has moved during firing*/
	if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

		/* stop visual and audio cues: technically, we're not firing at this moment */
		#if 1
		EndMarineMuzzleFlash(sbPtr);
		#else
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#endif

		#if MARINE_STATE_PRINT
		textprint("Turning to face.\n");
		#endif
		marineStatusPointer->lastroundhit=0;
		marineStatusPointer->lasthitsection=NULL;
		return(SRC_No_Change);
	}

	if (marineStatusPointer->Target) {
		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
	  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
	
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
	}

	/* Test for grenading! */
	if ((marineStatusPointer->myGunFlash==NULL)&&(marineStatusPointer->stateTimer == MARINE_NEAR_FIRE_TIME)
		&&(marineStatusPointer->Target))
	{

		/* NB don't fire grenades in air ducts */
		if ((FastRandom()&MARINE_CHANCEOFGRENADE) == 0) {
			if (!(marineStatusPointer->IAmCrouched)) {
				if (range > MARINE_TOO_CLOSE_TO_GRENADE_FOOL) {
					if (marineStatusPointer->My_Weapon->EnableGrenades) {

						GLOBALASSERT(marineStatusPointer->Target);
						NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
						LobAGrenade(sbPtr);
						marineStatusPointer->stateTimer = MARINE_NEAR_TIMEBETWEENFIRING;
						marineStatusPointer->volleySize = 0;

					
						#if MARINE_STATE_PRINT
						textprint("fired a grenade.\n");
						#endif
					
						return(SRC_Request_Approach);
					}
				}
			}
		}
	}
	
	DischargeLOSWeapon_Core(sbPtr);

	if (marineStatusPointer->Target==NULL) {
		/* Getting out of here! */
		return(SRC_No_Change);
	}

	/* Did we get him? */
	if ((NPC_IsDead(marineStatusPointer->Target))
		&&(marineStatusPointer->volleySize>=marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			if (Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr)==NULL) {
				/* Huzzah! */
				#if 1
				EndMarineMuzzleFlash(sbPtr);
				#else
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				#endif
				marineStatusPointer->lastroundhit=0;
				marineStatusPointer->lasthitsection=NULL;
				return(SRC_Request_Taunt);
			}
		}
	}
	
	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		#if 1
		EndMarineMuzzleFlash(sbPtr);
		#else
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#endif
		#endif

		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_Request_Fire);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if 1
		/* ... and remove the gunflash */
		#if 1
		EndMarineMuzzleFlash(sbPtr);
		#else
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#endif
		#endif

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

int SpeedRangeMods(VECTORCH *range,VECTORCH *speed) {

	int dot,theta,sinthet,rmod;
	int magrange,magspeed;

	/* Should return between 0 and ONE_FIXED... */

	magrange=Approximate3dMagnitude(range);
	rmod=magrange>>2;
	
	magspeed=Approximate3dMagnitude(speed);
	dot=DotProduct(range,speed);
	if (dot<0) dot=-dot;
	{
		int ab=MUL_FIXED(magrange,magspeed);
		ab<<=1;
		
		if (ab==0) dot=0; else dot=DIV_FIXED(dot,ab);

		if (dot>ONE_FIXED) {
			dot=ONE_FIXED;
			/* Well, I suppose it could happen. */
			/* Accuracy errors... */
			LOCALASSERT(dot<=ONE_FIXED);	
		}

		theta=ArcCos(dot);
		sinthet=GetSin(theta);

		dot=WideMulNarrowDiv(sinthet,magspeed,magrange);

	}
	dot<<=3;

	return(dot+rmod);	

}

int Validate_Target(STRATEGYBLOCK *target,char *SBname) {
	/* General purpose function. */

	if (target==NULL) return(0);

	if (target==Player->ObStrategyBlock) {
		if (Observer) {
			return(0);
		}
	}

	if (!NAME_ISEQUAL(target->SBname,SBname)) {
		return(0);
	} else {
		if (NPC_IsDead(target)) {
			return(0);
		} else {
			return(1);
		}
	}
}

int Validate_Strategy(STRATEGYBLOCK *target,char *SBname) {
	/* General purpose function Too. */

	if (target==NULL) return(0);

	if (target==Player->ObStrategyBlock) {
		if (Observer) {
			return(0);
		}
	}

	if (!NAME_ISEQUAL(target->SBname,SBname)) {
		return(0);
	} else {
		return(1);
	}
}

int Marine_TargetFilter(STRATEGYBLOCK *candidate) {

	switch (candidate->I_SBtype) {
		case I_BehaviourMarinePlayer:
		case I_BehaviourAlienPlayer:
		case I_BehaviourPredatorPlayer:
			{
				if (Observer) {
					return(0);
				}

				switch(AvP.PlayerType)
				{
					case I_Alien:
					case I_Predator:
						return(1);
						break;
					case I_Marine:
						return(0);
						break;
					default:
						GLOBALASSERT(0);
						return(0);
						break;
				}
				break;
			}
		case I_BehaviourDummy:
			{
				DUMMY_STATUS_BLOCK *dummyStatusPointer;    
				dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(candidate->SBdataptr);    
			    LOCALASSERT(dummyStatusPointer);	          		
				switch (dummyStatusPointer->PlayerType) {
					case I_Alien:
					case I_Predator:
						return(1);
						break;
					case I_Marine:
						return(0);
						break;
					default:
						GLOBALASSERT(0);
						return(0);
						break;
				}
				break;
			}
		case I_BehaviourAlien:
			{
				ALIEN_STATUS_BLOCK *alienStatusPointer;
				LOCALASSERT(candidate);	
				LOCALASSERT(candidate->DynPtr);	
			
				alienStatusPointer=(ALIEN_STATUS_BLOCK *)(candidate->SBdataptr);    
				
				if (NPC_IsDead(candidate)) {
					return(0);
				} else {
					if ((alienStatusPointer->BehaviourState==ABS_Dormant)||
						(alienStatusPointer->BehaviourState==ABS_Awakening)) {
						return(0);
					} else {
						return(1);
					}
				}
				break;
			}
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourPredator:
		case I_BehaviourXenoborg:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
			/* Valid. */
			return(1);
			break;
		case I_BehaviourMarine:
			#if ANARCHY
			return(1);
			#else
			return(0);
			#endif
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
						//return(1);
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


STRATEGYBLOCK *Marine_GetNewTarget(VECTORCH *marinepos, STRATEGYBLOCK *me) {

	int neardist, newblip;
	STRATEGYBLOCK *nearest;
	#if 1
	int a;
	STRATEGYBLOCK *candidate;
	#endif
	MODULE *dmod;
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int dist;
	VECTORCH offset;

	LOCALASSERT(me);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(me->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		
	
	dmod=ModuleFromPosition(marinepos,playerPherModule);
	
	LOCALASSERT(dmod);
	
	nearest=NULL;
	neardist=ONE_FIXED;
	newblip=0;
	
	//#if ANARCHY
	#if 1
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];

		if (candidate!=me) {
			if (candidate->DynPtr) {
				/* Arc reject. */
				MATRIXCH WtoL;

				offset.vx=marinepos->vx-candidate->DynPtr->Position.vx;
				offset.vy=marinepos->vy-candidate->DynPtr->Position.vy;
				offset.vz=marinepos->vz-candidate->DynPtr->Position.vz;
			
				WtoL=me->DynPtr->OrientMat;
				TransposeMatrixCH(&WtoL);
				RotateVector(&offset,&WtoL);

				if (offset.vz<=0) {

					/* It'll wheep, anyway. */
					if (marineStatusPointer->mtracker_timer==0) {
						#if 0
						if (
							(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							) {
						#else
						if (ObjectShouldAppearOnMotionTracker(candidate)) {
						#endif
					
							dist=Approximate3dMagnitude(&offset);
							if (dist<MOTIONTRACKER_RANGE) {
								tracker_noise=2;
							}							
						}
					}
					
					if (Marine_TargetFilter(candidate)) {
					
						dist=Approximate3dMagnitude(&offset);
					
						if (dist<neardist) {
							/* Check visibility? */
							if ((candidate->SBdptr)&&(me->SBdptr)) {
								/* Near case. */
								if ((!NPC_IsDead(candidate))
									||(candidate->I_SBtype==I_BehaviourMarinePlayer)
									||(candidate->I_SBtype==I_BehaviourDummy)) {
									if ((MarineCanSeeObject(me,candidate))) {
										nearest=candidate;
										neardist=dist;
									}	
								}
							} else {
								if ((!NPC_IsDead(candidate))
									||(candidate->I_SBtype==I_BehaviourMarinePlayer)
									||(candidate->I_SBtype==I_BehaviourDummy)) {
									if ((IsModuleVisibleFromModule(dmod,candidate->containingModule))) {
										nearest=candidate;
										neardist=dist;
									}	
								}
							}
							
							if (marineStatusPointer->mtracker_timer==0) {
								/* Hey, the tracker's on. */
								if (dist<MOTIONTRACKER_RANGE) {
									#if 0
									if (
										(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
										||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
										||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
										) {
									#else
									if (ObjectShouldAppearOnMotionTracker(candidate)) {
									#endif
										newblip=1;
										marineStatusPointer->suspect_point=candidate->DynPtr->Position;
										/* Set this to zero when you get a *new* suspicion. */
										marineStatusPointer->previous_suspicion=0;
										marineStatusPointer->using_squad_suspicion=0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	#endif

	if (nearest==NULL) {
		if (newblip) {
			marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
			tracker_noise=2;
			PointAlert(1,&marineStatusPointer->suspect_point);
		}
	} else {
		PointAlert(2,&nearest->DynPtr->Position);
	}

	if (nearest) {
		/* Must have seen them. */
		marineStatusPointer->roundsForThisTarget=0;
		marineStatusPointer->sawlastframe=1;

		if (((marineStatusPointer->suspicious==0)&&((FastRandom()&65535)<16384))
			||(neardist<((FastRandom()&4095)+1000))) {
			/* Exhibit suprise? */
			Marine_SurpriseSound(me);
		}
	}

	return(nearest);

}

void FakeTrackerWheepGenerator(VECTORCH *marinepos, STRATEGYBLOCK *me) {

	int a,dist;
	STRATEGYBLOCK *candidate;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	VECTORCH offset;

	LOCALASSERT(me);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(me->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->mtracker_timer==0) {
		for (a=0; a<NumActiveStBlocks; a++) {
			candidate=ActiveStBlockList[a];
			if (candidate!=me) {
				if (candidate->DynPtr) {
					/* Arc reject. */
					MATRIXCH WtoL;

					offset.vx=marinepos->vx-candidate->DynPtr->Position.vx;
					offset.vy=marinepos->vy-candidate->DynPtr->Position.vy;
					offset.vz=marinepos->vz-candidate->DynPtr->Position.vz;
			
					WtoL=me->DynPtr->OrientMat;
					TransposeMatrixCH(&WtoL);
					RotateVector(&offset,&WtoL);

					if (offset.vz<=0) {
						/* It'll wheep, anyway. */
						#if 0
						if (
							(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							||(candidate->DynPtr->Position.vx!=candidate->DynPtr->PrevPosition.vx)
							) {
						#else
						if (ObjectShouldAppearOnMotionTracker(candidate)) {
						#endif
							dist=Approximate3dMagnitude(&offset);
							if (dist<MOTIONTRACKER_RANGE) {
								tracker_noise=2;
							}							
						}
					}
				}
			}
		}
	}
}

#if 0
static STATE_RETURN_CONDITION Execute_MNS_Hunt(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH velocityDirection = {0,0,0};

	/* Your mission: to advance into the players module, even if near. */
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);
		
	/* check if we should be crouched or standing up */
	if(marineStatusPointer->IAmCrouched)
	{
		/* curently crouched */
		if(!(MarineShouldBeCrawling(sbPtr)))
		{
			/* should be running*/
			marineStatusPointer->IAmCrouched = 0;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	else
	{
		/* currently standing */
		if(MarineShouldBeCrawling(sbPtr))
		{
			/* should be crawling */
			marineStatusPointer->IAmCrouched = 1;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}

	/* should we change to approach state? */
	if(MarineCanSeeTarget(sbPtr))
	{
		return(SRC_Request_Approach);
	} else if(!(MarineIsAwareOfTarget(sbPtr))) {
		return(SRC_Request_Wait);
	}

	{
		AIMODULE *targetModule;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

		targetModule = FarNPC_GetTargetAIModuleForHunt(sbPtr,0);

		if (targetModule) {
		//	textprint("Target module is %s\n",targetModule->name);
			#if MARINE_STATE_PRINT
			textprint("Target module is... an AI module...\n");
			#endif
		} else {
			#if MARINE_STATE_PRINT
			textprint("Target module is NULL!\n");
			#endif
		}

		if (targetModule==sbPtr->containingModule->m_aimodule) {
			/* Hey, it'll drop through. */
			return(SRC_Request_Approach);
		}
		
		if (!targetModule) {
			#if 1
			/* Must be sealed off. */
			return(SRC_Request_Wait);
			#else
			extern MODULE *playerPherModule;
			
			LOGDXFMT(("Jules's bug: marine is in %s, player is in %s",sbPtr->containingModule->name,playerPherModule->name));
			GLOBALASSERT(targetModule);
			#endif
		}				

		thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
		if (!thisEp) {
			//LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",targetModule->name,sbPtr->containingModule->name));
			LOGDXFMT(("This assert is a busted adjacency!"));
			GLOBALASSERT(thisEp);
		}
		/* If that fired, there's a farped adjacency. */
		GLOBALASSERT(thisEp->alien_only==0);
		/* If that fired, Get...ModuleForHunt went wrong. */
	
		marineStatusPointer->wanderData.worldPosition=thisEp->position;
		marineStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
		marineStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
		marineStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
		
	}

	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}
#endif

static STATE_RETURN_CONDITION Execute_MNS_Respond(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH velocityDirection = {0,0,0};

	/* Your mission: to advance into the alert zone, even if near. */
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);
	
	#if 0		
	/* check if we should be crouched or standing up */
	if(marineStatusPointer->IAmCrouched)
	{
		/* curently crouched */
		if(!(MarineShouldBeCrawling(sbPtr)))
		{
			/* should be running*/
			marineStatusPointer->IAmCrouched = 0;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineRun,MRSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	else
	{
		/* currently standing */
		if(MarineShouldBeCrawling(sbPtr))
		{
			/* should be crawling */
			marineStatusPointer->IAmCrouched = 1;
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	#else
	HandleMovingAnimations(sbPtr);
	#endif
	
	/* should we change to approach state? */
	if((MarineIsAwareOfTarget(sbPtr))) {
		return(SRC_Request_Approach);
	}
	/* If we've picked up a new target, go for it. */

	if (sbPtr->containingModule->m_aimodule==NpcSquad.alertZone) {
		/* We're here! */
		DeprioritiseAlert(sbPtr->containingModule->m_aimodule);
		/* Hey, if it's real, there'll be a new one soon enough. */
		return(SRC_Request_Approach);
	}

	{
		AIMODULE *targetModule;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

		targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);

		if (targetModule) {
		//	textprint("Target module is %s\n",targetModule->name);
			#if MARINE_STATE_PRINT
			textprint("Target module is... an AI module...\n");
			#endif
		} else {
			#if MARINE_STATE_PRINT
			textprint("Target module is NULL!\n");
			#endif
		}

		if (targetModule==sbPtr->containingModule->m_aimodule) {
			/* Looks like we've arrived. */
			/* Hey, it'll drop through. */
			return(SRC_Request_Approach);
		}
		
		if (!targetModule) {
			#if 1
			/* Must be sealed off. */
			return(SRC_Request_Wait);
			#else
			extern MODULE *playerPherModule;
			
			LOGDXFMT(("Jules's bug: marine is in %s, player is in %s",sbPtr->containingModule->name,playerPherModule->name));
			GLOBALASSERT(targetModule);
			#endif
		}				

		thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
		if (!thisEp) {
			//LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",targetModule->name,sbPtr->containingModule->name));
			LOGDXFMT(("This assert is a busted adjacency!"));
			GLOBALASSERT(thisEp);
		}
		/* If that fired, there's a farped adjacency. */
		GLOBALASSERT(thisEp->alien_only==0);
		/* If that fired, GetNextModuleForLink went wrong. */
	
		marineStatusPointer->wanderData.worldPosition=thisEp->position;
		marineStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
		marineStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
		marineStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
		
	}

	/* ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif

	return(SRC_No_Change);
}

static int MarineRetreatsInTheFaceOfDanger(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* This depends on mission, armament, and whether he's cornered. */

	if (marineStatusPointer->Android) {
		return(0);
	}

	switch (marineStatusPointer->Mission) {
		case MM_NonCom:
			#if 0
			/* Err... runferrit! */
			return(1);
			break;
			#endif
		case MM_Guard:
		case MM_Wander:
		case MM_Wait_Then_Wander:
		case MM_LocalGuard:
		case MM_Pathfinder:
			{
				if ((FastRandom()&65535)>marineStatusPointer->Courage) {
					return(1);
				} else {
					return(0);
				}
				break;
			}
		default:
			return(0);
			break;
	}

	return(0);
}

static STATE_RETURN_CONDITION Execute_MNS_Retreat(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH velocityDirection = {0,0,0};
	AIMODULE *old_fearmod;

	/* Your mission: to advance out of trouble, even if near. */
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);

	old_fearmod=marineStatusPointer->fearmodule;
	
	/* From where am I running? */
	if(MarineIsAwareOfTarget(sbPtr)) {
		marineStatusPointer->fearmodule=marineStatusPointer->Target->containingModule->m_aimodule;
	} else if (marineStatusPointer->fearmodule==NULL) {
		marineStatusPointer->fearmodule=sbPtr->containingModule->m_aimodule;
		
		if (TERROR_MODE) {
			/* Better correct... */
			marineStatusPointer->fearmodule=Player->ObStrategyBlock->containingModule->m_aimodule;
		}
	}	
	
	if (marineStatusPointer->fearmodule!=old_fearmod) {
		marineStatusPointer->destinationmodule = General_GetAIModuleForRetreat(sbPtr,marineStatusPointer->fearmodule,5);
	}

	{
		AIMODULE *targetModule;
		FARENTRYPOINT *thisEp = (FARENTRYPOINT *)0;

		targetModule = GetNextModuleForLink(sbPtr->containingModule->m_aimodule,marineStatusPointer->destinationmodule,6,0);

		#if 0
		if (targetModule) {
			//textprint("Target module is %s\n",targetModule->name);
			#if MARINE_STATE_PRINT
			textprint("Target AI module found.\n");
			#endif
		} else {
			#if MARINE_STATE_PRINT
			textprint("Target module is NULL!\n");
			#endif
			return(SRC_Request_Wait);
		}
		#endif

		if ((targetModule==sbPtr->containingModule->m_aimodule)
			|| (targetModule==NULL)) {
			/* There's no-where to run! */
			if (marineStatusPointer->Target) {
				return(SRC_Request_PanicFire);
			} else {
				return(SRC_Request_Wait);
			}
		}
		
		GLOBALASSERT(targetModule);
		
		thisEp=GetAIModuleEP(targetModule,sbPtr->containingModule->m_aimodule);
		if (!thisEp) {
			//LOGDXFMT(("This assert is a busted adjacency!\nNo EP between %s and %s.",targetModule->name,sbPtr->containingModule->name));
			LOGDXFMT(("This assert is a busted adjacency!"));
			GLOBALASSERT(thisEp);
		}
		/* If that fired, there's a farped adjacency. */
		GLOBALASSERT(thisEp->alien_only==0);
		/* If that fired, GetNextModuleForLink went wrong. */
	
		marineStatusPointer->wanderData.worldPosition=thisEp->position;
		marineStatusPointer->wanderData.worldPosition.vx+=targetModule->m_world.vx;
		marineStatusPointer->wanderData.worldPosition.vy+=targetModule->m_world.vy;
		marineStatusPointer->wanderData.worldPosition.vz+=targetModule->m_world.vz;
		
	}

	/* Ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	if (marineStatusPointer->Target) {
		int dp;
		VECTORCH vectotarget;
		/* Are we running in a stupid direction? */
		vectotarget.vx=marineStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
		vectotarget.vy=marineStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
		vectotarget.vz=marineStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;
		Normalise(&vectotarget);
		
		dp=DotProduct(&vectotarget,&velocityDirection);

		if (dp>55000) {
			/* Argh!  He's in the way! */
			return(SRC_Request_PanicFire);
		}
	}
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	

	HandleMovingAnimations(sbPtr);
	/* ...so we must be here for the duration. */

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_PanicScream(sbPtr);
				}
			}
		}
	}

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}

	#if 1
	if(NPC_CannotReachTarget(&(marineStatusPointer->moveData), &(marineStatusPointer->wanderData.worldPosition), &velocityDirection))
	{
		/* go to avoidance */
		/* no sequence change required */
		
		marineStatusPointer->obstruction.environment=1;
		marineStatusPointer->obstruction.destructableObject=0;
		marineStatusPointer->obstruction.otherCharacter=0;
		marineStatusPointer->obstruction.anySingleObstruction=0;

		return(SRC_Request_Avoidance);
	}
	#endif
	#endif

	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_DischargeShotgun(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;
	int hitroll;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	#if MARINE_STATE_PRINT
	textprint("Firing shotgun... ");
	#endif

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if(!MarineCanSeeTarget(sbPtr))
	{

		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_Reload);
	}

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}
	
	/* Deal with tweening part. */
	if (marineStatusPointer->internalState) {
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			marineStatusPointer->HModelController.Playing=0;
			marineStatusPointer->HModelController.sequence_timer=0;
			marineStatusPointer->internalState=0;
		}
		return(SRC_No_Change);
	}

	#if 0
	if ((marineStatusPointer->HModelController.keyframe_flags)
		||(marineStatusPointer->HModelController.Playing==0)) {
	
		marineStatusPointer->HModelController.Playing=0;
	#else
	if (marineStatusPointer->HModelController.Playing==0) {
	#endif

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{

			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}
		
		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}

			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;
		marineStatusPointer->HModelController.sequence_timer=0;

		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
		
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
		
		/* at this point we are correctly orientated: if we have no gunflash yet,
		and our state timer is set to marine_near_firetime then we have either
		just started firing, or have become dis-orienated between bursts. This is a good
		time to consider firing a grenade... */
		
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) {
			MaintainMarineGunFlash(sbPtr);
		} else {
			CreateMarineGunFlash(sbPtr);
		}
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now hit the target with a shotgun blast. */
	
		hitroll=0;
		
		while (ShotgunBlast[hitroll].vz>0) {
			VECTORCH world_vec;
			
			RotateAndCopyVector(&ShotgunBlast[hitroll],&world_vec,&marineStatusPointer->My_Gunflash_Section->SecMat);
			CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&world_vec, marineStatusPointer->My_Weapon->Ammo_Type, 1,0);
	
			hitroll++;
		}
		if (marineStatusPointer->clipammo>0) {
			marineStatusPointer->clipammo--;
		}

	} else {

		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* You must have fired already. */
	#if 1
	GLOBALASSERT(marineStatusPointer->HModelController.Looped==0);
	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		return(SRC_Request_PumpAction);
	}
	return(SRC_No_Change);
	#else
	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_No_Change);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
	#endif
}

#define PISTOL_RELOAD_TIME 65536

static STATE_RETURN_CONDITION Execute_MNS_DischargePistol(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;
	int mod,hitroll;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	LOCALASSERT((marineStatusPointer->My_Weapon->id==MNPCW_MPistol)
		||(marineStatusPointer->My_Weapon->id==MNPCW_PistolMarine)
		||(marineStatusPointer->My_Weapon->id==MNPCW_Android_Pistol_Special));

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if(!MarineCanSeeTarget(sbPtr))
	{
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		return(SRC_Request_Reload);
	}
	
	#if MARINE_STATE_PRINT
	textprint("Firing pistol... ");
	#endif

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}

	relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
	relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
	relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
		  
	relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
	relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
	relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
	
	range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if ((marineStatusPointer->HModelController.keyframe_flags)
		||(marineStatusPointer->HModelController.Playing==0)) {
		
		marineStatusPointer->HModelController.Playing=0;
		marineStatusPointer->HModelController.sequence_timer=0;

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
			#endif
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}
		
		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening==Controller_Tweening)) {

			#if 1
			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}
			#endif
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;
		if (marineStatusPointer->clipammo>0) {
			marineStatusPointer->clipammo--;
		}
		marineStatusPointer->roundsForThisTarget++;
		
		/* at this point we are correctly orientated: if we have no gunflash yet,
		and our state timer is set to marine_near_firetime then we have either
		just started firing, or have become dis-orienated between bursts. This is a good
		time to consider firing a grenade... */
		
		#if 1
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
		else CreateMarineGunFlash(sbPtr);
		#else
		/* KJL 15:50:48 05/01/98 - draw muzzleflash */
		GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
		{
			VECTORCH direction;
		
			direction.vx = marineStatusPointer->My_Gunflash_Section->SecMat.mat31;
			direction.vy = marineStatusPointer->My_Gunflash_Section->SecMat.mat32;
			direction.vz = marineStatusPointer->My_Gunflash_Section->SecMat.mat33;
		
			DrawMuzzleFlash(&marineStatusPointer->My_Gunflash_Section->World_Offset,&direction,MUZZLE_FLASH_AMORPHOUS);
		}
		#endif
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now hit the target with one bullet. */
	
		mod=SpeedRangeMods(&relPos,&relPos2);
	
		hitroll=marineStatusPointer->Skill; /* Marine skill... */
		if (marineStatusPointer->Target==Player->ObStrategyBlock) {
			PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
			LOCALASSERT(playerStatusPtr);
			if ( (AvP.PlayerType==I_Alien)
				||((AvP.PlayerType==I_Predator)&&(playerStatusPtr->cloakOn==1))) {
				/* Vs the player, lighting effects on aliens and cloaked preds. */
				hitroll=MUL_FIXED(hitroll,((CurrentLightAtPlayer>>1)+32767));
			}
		}
		hitroll-=mod;
		hitroll+=marineStatusPointer->My_Weapon->Accuracy;
	
		{
							
			/* Handle Damage. */
			if ((FastRandom()&65535)<hitroll) {
				/* DO DAMAGE TO TARGET HERE */
				
				VECTORCH rel_pos,attack_dir;
				int dist;
				VECTORCH shotvector;

				shotvector.vx=0;
				shotvector.vy=0;
				shotvector.vz=65535;
				RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);

				rel_pos.vx=marineStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
				rel_pos.vy=marineStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
				rel_pos.vz=marineStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;

				dist=Approximate3dMagnitude(&rel_pos);

				if (VerifyHitShot(sbPtr,marineStatusPointer->Target,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, 1,dist)) {

					GetDirectionOfAttack(marineStatusPointer->Target,&rel_pos,&attack_dir);
					/* Get hit location? */
		
					marineStatusPointer->lasthitsection=HitLocationRoll(marineStatusPointer->Target,sbPtr);
				
					if (marineStatusPointer->lasthitsection) {
						CauseDamageToHModel(marineStatusPointer->lasthitsection->my_controller, marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED, marineStatusPointer->lasthitsection,&attack_dir,NULL,0);
					} else {
						CauseDamageToObject(marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
					}
				}
			} else {
				GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
				ProjectNPCShot(sbPtr, marineStatusPointer->Target, &marineStatusPointer->My_Gunflash_Section->World_Offset,&marineStatusPointer->My_Gunflash_Section->SecMat, marineStatusPointer->My_Weapon->Ammo_Type, 1);
			}

			/* Did we get him? */
			if ((NPC_IsDead(marineStatusPointer->Target))&&(marineStatusPointer->My_Weapon->ARealMarine)) {
				/* Only real marines taunt. */
				if ((marineStatusPointer->roundsForThisTarget==1)||(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
					if (Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr)==NULL) {
						/* Huzzah! */
						if(marineStatusPointer->myGunFlash) 
						{
							RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
							marineStatusPointer->myGunFlash = NULL;				
						}
						if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
							Sound_Stop(marineStatusPointer->soundHandle);
							Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
						}
						marineStatusPointer->lastroundhit=0;
						marineStatusPointer->lasthitsection=NULL;
						return(SRC_Request_Taunt);
					}
				}
			}
		}
	} else {	
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* You must have fired already. */

	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if ((range < MARINE_CLOSE_APPROACH_DISTANCE)||(marineStatusPointer->Android))
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_No_Change);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_ThrowMolotov(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if (marineStatusPointer->stateTimer==marineStatusPointer->My_Weapon->FiringTime) {
		marineStatusPointer->HModelController.Playing=0;
		marineStatusPointer->HModelController.sequence_timer=0;
		marineStatusPointer->stateTimer--;
	}

	{
		if (marineStatusPointer->Target) {
			/* Only terminate if you haven't fired yet... */
			if(!MarineCanSeeTarget(sbPtr))
			{
			
				/* .... and stop the sound */
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);		
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				
				#if MARINE_STATE_PRINT
				textprint("Returning no target.\n");
				#endif
				return(SRC_Request_Wait);
			}
			
			GLOBALASSERT(marineStatusPointer->Target);
			NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
			/* Fix weapon target! */
			if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
				marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
				marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
				marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
			}
			
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			if (NpcSquad.Squad_Suspicion==0) {
				PointAlert(2,&marineStatusPointer->weaponTarget);
			}
			
		}

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if(!correctlyOrientated)
		{
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;

		/* at this point we are correctly orientated: if we have no gunflash yet,
		and our state timer is set to marine_near_firetime then we have either
		just started firing, or have become dis-orienated between bursts. This is a good
		time to consider firing a grenade... */
		
		/* No muzzle flash. */
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		if(marineStatusPointer->My_Gunflash_Section) {

			if (marineStatusPointer->HModelController.keyframe_flags) {
				
				SECTION *root;
				MARINE_WEAPON_DATA *noncom;

				/* Throw! */			
				SpawnMolotovCocktail(marineStatusPointer->My_Gunflash_Section, &sbPtr->DynPtr->OrientMat);
				marineStatusPointer->My_Gunflash_Section=NULL;
				marineStatusPointer->Mission=MM_NonCom;
				/* Turn into a noncom! */
								
				{
					/* Remove hitdelta, if there is one. */
					DELTA_CONTROLLER *delta;
					delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
					if (delta) {
						Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
					}
				}

				noncom=GetThisNPCMarineWeapon(MNPCW_MUnarmed);

				root=GetNamedHierarchyFromLibrary(noncom->Riffname,noncom->HierarchyName);
				Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root, 1, 0,0);
				marineStatusPointer->My_Weapon=noncom;
				
				/* Attempt to put the hitdelta back? */
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,(int)HMSQT_MarineStand,(int)MSSS_HitChestFront)) {
					DELTA_CONTROLLER *delta;
					delta=Add_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta",(int)HMSQT_MarineStand,(int)MSSS_HitChestFront,(ONE_FIXED>>2));
					GLOBALASSERT(delta);
					delta->Playing=0;
				}

				return(SRC_Request_Retreat);
			}
		}
	}	

	return(SRC_No_Change);
}

#if 0
static STATE_RETURN_CONDITION Execute_MNS_DischargeGL(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	#if MARINE_STATE_PRINT
	textprint("Firing grenade launcher... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if (marineStatusPointer->HModelController.keyframe_flags&2) {
		marineStatusPointer->internalState=1;
	}
	if (marineStatusPointer->HModelController.keyframe_flags&1) {
		marineStatusPointer->internalState=0;
	} 

	if(!MarineCanSeeTarget(sbPtr))
	{
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		return(SRC_Request_Reload);
	}		

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}

	if (marineStatusPointer->stateTimer==marineStatusPointer->My_Weapon->FiringTime) {
		
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			marineStatusPointer->HModelController.Playing=0;
		}

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
			#endif
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}
	
		/* Aim up a little? */
		range=VectorDistance((&marineStatusPointer->weaponTarget),(&sbPtr->DynPtr->Position));

		marineStatusPointer->weaponTarget.vy-=(range/6);
			
		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

			#if 1
			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}
			#endif
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;
		marineStatusPointer->HModelController.sequence_timer=0;

		#if 0
		/* Can't afford to have the jam! */
		if (marineStatusPointer->internalState!=0) {
			/* Just in case. */
			return(SRC_No_Change);
		}
		#endif

		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
		
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
		
		/* Are they, by some chance, really close? */
		if (range<marineStatusPointer->My_Weapon->MinRange) {
			if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
				/* Stay cool. */
				return(SRC_Request_PullPistol);
			} else {
				if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
					/* Fail twice, then flee... else continue. */
					return(SRC_Request_Retreat);
				}
			}
		}

		#if 1
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
		else CreateMarineGunFlash(sbPtr);
		#else
		/* KJL 15:50:48 05/01/98 - draw muzzleflash */
		GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
		{
			VECTORCH direction;
		
			direction.vx = marineStatusPointer->My_Gunflash_Section->SecMat.mat31;
			direction.vy = marineStatusPointer->My_Gunflash_Section->SecMat.mat32;
			direction.vz = marineStatusPointer->My_Gunflash_Section->SecMat.mat33;
		
			DrawMuzzleFlash(&marineStatusPointer->My_Gunflash_Section->World_Offset,&direction,MUZZLE_FLASH_AMORPHOUS);
		}
		#endif
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now fire a grenade. */
	
	
		{
			LOCALASSERT(marineStatusPointer->My_Gunflash_Section);
			//LOCALASSERT(marineStatusPointer->internalState==0);
			
			CreateGrenadeKernel(I_BehaviourGrenade, &marineStatusPointer->My_Gunflash_Section->World_Offset, &marineStatusPointer->My_Gunflash_Section->SecMat,0);
		
			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}
	}	

	if (marineStatusPointer->stateTimer<marineStatusPointer->My_Weapon->FiringTime) {
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* You must have fired already. */

	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}
#endif

static STATE_RETURN_CONDITION Execute_MNS_NewDischargeGL(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if(!MarineCanSeeTarget(sbPtr))
	{

		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_Reload);
	}

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}
	
	/* Deal with tweening part. */
	if (marineStatusPointer->internalState) {
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			marineStatusPointer->HModelController.Playing=0;
			marineStatusPointer->HModelController.sequence_timer=0;
			marineStatusPointer->internalState=0;
		}
		return(SRC_No_Change);
	}

	if (marineStatusPointer->HModelController.Playing==0) {

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{

			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* Aim up a little? */
		range=VectorDistance((&marineStatusPointer->weaponTarget),(&sbPtr->DynPtr->Position));

		marineStatusPointer->weaponTarget.vy-=(range/8);
		
		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}

			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;
		marineStatusPointer->HModelController.sequence_timer=0;

		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
		
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
		
		/* Are they, by some chance, really close? */
		if (range<marineStatusPointer->My_Weapon->MinRange) {
			if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
				/* Stay cool. */
				return(SRC_Request_PullPistol);
			} else {
				if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
					/* Fail twice, then flee... else continue. */
					return(SRC_Request_Retreat);
				}
			}
		}

		/* at this point we are correctly orientated: if we have no gunflash yet,
		and our state timer is set to marine_near_firetime then we have either
		just started firing, or have become dis-orienated between bursts. This is a good
		time to consider firing a grenade... */
		
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) {
			MaintainMarineGunFlash(sbPtr);
		} else {
			CreateMarineGunFlash(sbPtr);
		}
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now fire a grenade. */
		{
			LOCALASSERT(marineStatusPointer->My_Gunflash_Section);
			
			CreateGrenadeKernel(I_BehaviourGrenade, &marineStatusPointer->My_Gunflash_Section->World_Offset, &marineStatusPointer->My_Gunflash_Section->SecMat,0);
		
			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}

	} else {

		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* You must have fired already. */

	GLOBALASSERT(marineStatusPointer->HModelController.Looped==0);
	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		return(SRC_Request_PumpAction);
	}
	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MNS_DischargeSADAR(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	#if MARINE_STATE_PRINT
	textprint("Firing SADAR... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	if(!MarineCanSeeTarget(sbPtr))
	{

		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		return(SRC_Request_Reload);
	}		

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}

	if (marineStatusPointer->stateTimer==marineStatusPointer->My_Weapon->FiringTime) {
		
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			marineStatusPointer->HModelController.Playing=0;
		}

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
			#endif
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

			#if 1
			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}
			#endif
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->HModelController.Playing=1;
		marineStatusPointer->HModelController.sequence_timer=0;

		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
		
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

		/* Are they, by some chance, really close? */
		if (range<marineStatusPointer->My_Weapon->MinRange) {
			if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
				/* Stay cool. */
				return(SRC_Request_PullPistol);
			} else {
				if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
					/* Fail twice, then flee... else continue. */
					return(SRC_Request_Retreat);
				}
			}
		}
		
		/* at this point we are correctly orientated: if we have no gunflash yet,
		and our state timer is set to marine_near_firetime then we have either
		just started firing, or have become dis-orienated between bursts. This is a good
		time to consider firing a grenade... */
		
		#if 1
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
		else CreateMarineGunFlash(sbPtr);
		#else
		/* KJL 15:50:48 05/01/98 - draw muzzleflash */
		GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
		{
			VECTORCH direction;
		
			direction.vx = marineStatusPointer->My_Gunflash_Section->SecMat.mat31;
			direction.vy = marineStatusPointer->My_Gunflash_Section->SecMat.mat32;
			direction.vz = marineStatusPointer->My_Gunflash_Section->SecMat.mat33;
		
			DrawMuzzleFlash(&marineStatusPointer->My_Gunflash_Section->World_Offset,&direction,MUZZLE_FLASH_AMORPHOUS);
		}
		#endif
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now fire a rocket. */
	  	
		{
			SECTION_DATA *rocket_section;

			rocket_section=GetThisSectionData(marineStatusPointer->HModelController.section_data,"dum flash");

			LOCALASSERT(rocket_section);
			
			CreateRocketKernel(&rocket_section->World_Offset, &rocket_section->SecMat,0);

			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}
	}	

	if (marineStatusPointer->stateTimer<marineStatusPointer->My_Weapon->FiringTime) {
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* You must have fired already. */

	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

void GetNewRandomDirection(MARINE_STATUS_BLOCK *marineStatusPointer) {
	
	int rnum;

	rnum=FastRandom()&65535;

	marineStatusPointer->lastroundhit&=65535;
	marineStatusPointer->lastroundhit|=(rnum<<16);
	
}

void GetFirstRandomDirection(MARINE_STATUS_BLOCK *marineStatusPointer,VECTORCH *output) {

	int rnum;
	MATRIXCH tempmat;
	EULER tempeul;
	VECTORCH tempvec;

	rnum=FastRandom()&65535;

	rnum>>=4;

	tempeul.EulerX=0;
	tempeul.EulerY=rnum;
	tempeul.EulerZ=0;

	CreateEulerMatrix(&tempeul,&tempmat);

	tempvec.vx=0;
	tempvec.vy=0;
	tempvec.vz=65535;

	RotateAndCopyVector(&tempvec,output,&tempmat);

	marineStatusPointer->lastroundhit=(rnum<<20)+(rnum<<4);

}

void TurnToFaceRandomDirection(MARINE_STATUS_BLOCK *marineStatusPointer,VECTORCH *output) {

	int currentangle,targetangle,deltaangle,deltaangle2;
	MATRIXCH tempmat;
	EULER tempeul;
	VECTORCH tempvec;

	currentangle=(marineStatusPointer->lastroundhit)&(65535);
	targetangle=(marineStatusPointer->lastroundhit>>16)&(65535);

	deltaangle=targetangle-currentangle;
	deltaangle2=deltaangle;
	if (deltaangle<0) deltaangle+=65536;

	if (deltaangle>32767) {
		currentangle-=(NormalFrameTime>>1);
		if (currentangle<0) currentangle+=65536;
	} else {
		currentangle+=(NormalFrameTime>>1);
		if (currentangle>65535) currentangle-=65536;
	}
	/* Overshoot test. */
	deltaangle=targetangle-currentangle;
	if ((deltaangle*deltaangle2)<=0) {
		currentangle=targetangle;
	}

	LOCALASSERT(currentangle<65536);
	marineStatusPointer->lastroundhit&=~65535;
	marineStatusPointer->lastroundhit|=currentangle;

	currentangle>>=4;

	tempeul.EulerX=0;
	tempeul.EulerY=currentangle;
	tempeul.EulerZ=0;

	CreateEulerMatrix(&tempeul,&tempmat);

	tempvec.vx=0;
	tempvec.vy=0;
	tempvec.vz=65535;

	RotateAndCopyVector(&tempvec,output,&tempmat);

}

void Convert_To_RunningOnFire(STRATEGYBLOCK *sbPtr) {

	SECTION *root;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Burn off tracker. */
	marineStatusPointer->mtracker_timer=-1;

	/* Grimace. */
	Marine_SwitchExpression(sbPtr,5);

	if (marineStatusPointer->behaviourState == MBS_Dying) {
		/* Just die. */
		return;
	}														

	/* get rid of the gun flash, if we've got it */
	if(marineStatusPointer->myGunFlash)
	{
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;
	}

	/* Switch to template. */
	root=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->TemplateName);

	/* Remove all deltas. */
	Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Elevation");
	Remove_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
	Remove_Delta_Sequence(&marineStatusPointer->HModelController,"Minigun");
	Remove_Delta_Sequence(&marineStatusPointer->HModelController,"sprintheaddelta");

	Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,
		root, 1, 0,0);

	SetMarineAnimationSequence(sbPtr,HMSQT_MarineRun,MRSS_Tem_Run_On_Fire,
		(ONE_FIXED*3),(ONE_FIXED>>2));

	GetFirstRandomDirection(marineStatusPointer,&(marineStatusPointer->wanderData.worldPosition));
	/* Hey, it's a spare vector. */
	marineStatusPointer->nearSpeed=MARINE_NEAR_SPEED>>1;
	NPCSetVelocity(sbPtr, &(marineStatusPointer->wanderData.worldPosition), marineStatusPointer->nearSpeed);	

	InitMission(sbPtr,MM_RunAroundOnFire);

	marineStatusPointer->stateTimer=(ONE_FIXED);

	/* Add crackling sound. */

	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle);

 	Sound_Play(SID_FIRE,"dlev",&(sbPtr->DynPtr->Position),&marineStatusPointer->soundHandle,127);

}

static STATE_RETURN_CONDITION Execute_MNS_RunAroundOnFire(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;

	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);

	if (sbPtr->SBDamageBlock.IsOnFire==0) {
		/* Stay on fire! */
		sbPtr->SBDamageBlock.IsOnFire=1;
	}

	/* check if we should be crouched or standing up */
	if(marineStatusPointer->IAmCrouched)
	{
		/* curently crouched */
		if(!(MarineShouldBeCrawling(sbPtr)))
		{
			/* should be running*/
			marineStatusPointer->IAmCrouched = 0;
			SetMarineAnimationSequence(sbPtr,HMSQT_MarineRun,MRSS_Tem_Run_On_Fire,ONE_FIXED,(ONE_FIXED>>3));
		}
	}
	else
	{
		/* currently standing */
		if(MarineShouldBeCrawling(sbPtr))
		{
			/* should be crawling, I guess... */
			marineStatusPointer->IAmCrouched = 1;
			SetMarineAnimationSequence(sbPtr,HMSQT_MarineCrawl,MCrSS_Standard,ONE_FIXED,(ONE_FIXED>>3));
		}
	}

	/* Set velocity for this frame. */
	marineStatusPointer->stateTimer-=NormalFrameTime;
	if (marineStatusPointer->stateTimer<0) {
		marineStatusPointer->stateTimer=0;
		//if (FastRandom()&15==0) 
		{
			GetNewRandomDirection(marineStatusPointer);
			marineStatusPointer->stateTimer=(ONE_FIXED);
		}
	}
	TurnToFaceRandomDirection(marineStatusPointer,&(marineStatusPointer->wanderData.worldPosition));
	NPCSetVelocity(sbPtr, &(marineStatusPointer->wanderData.worldPosition), marineStatusPointer->nearSpeed);	

	/* Handle sound... */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));

	/* test here for impeding collisions, and not being able to reach target... */
	#if ALL_NEW_AVOIDANCE
	{
		if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
			/* Go to all new avoidance. */
			return(SRC_Request_Avoidance);
		}
	}
	#else
	{
		STRATEGYBLOCK *destructableObject = NULL;

		NPC_IsObstructed(sbPtr,&(marineStatusPointer->moveData),&marineStatusPointer->obstruction,&destructableObject);
		if((marineStatusPointer->obstruction.environment)||(marineStatusPointer->obstruction.otherCharacter))
		{
			/* go to avoidance */
			GetNewRandomDirection(marineStatusPointer);
			TurnToFaceRandomDirection(marineStatusPointer,&(marineStatusPointer->wanderData.worldPosition));
			NPCSetVelocity(sbPtr, &(marineStatusPointer->wanderData.worldPosition), marineStatusPointer->nearSpeed);	
			return(SRC_Request_Avoidance);
		}
		if(marineStatusPointer->obstruction.destructableObject)
		{
			LOCALASSERT(destructableObject);
			CauseDamageToObject(destructableObject,&TemplateAmmo[AMMO_NPC_OBSTACLE_CLEAR].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
	}
	#endif

	return(SRC_No_Change);
}

void RunAroundOnFireMission_Control(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	STATE_RETURN_CONDITION state_result;
	int marineIsNear;
	    
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* Current Behaviour. */

	if(sbPtr->SBdptr) {
		marineIsNear=1;
		LOCALASSERT(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]);						
	} else {
		marineIsNear=0;
	}

	{
	
		switch(marineStatusPointer->behaviourState)
		{
			case(MBS_Dying):
			{
				if ((ShowSquadState)||((sbPtr->SBdptr)&&(ShowNearSquad))) {
					PrintDebuggingText("RAOF marine dying in %s\n",sbPtr->containingModule->name);
				}

				if (marineIsNear) {
					Execute_Dying(sbPtr);
				} else {
					Execute_Dying(sbPtr);
				}
				break;
			}
			default:
			{
				if ((ShowSquadState)||((sbPtr->SBdptr)&&(ShowNearSquad))) {
					PrintDebuggingText("RAOF marine running in %s\n",sbPtr->containingModule->name);
				}

				if (marineIsNear) {
					state_result=Execute_MNS_RunAroundOnFire(sbPtr);
				} else {
					Execute_MFS_Wait(sbPtr);
				}
				break;
			}
		}	
	}

	if (!marineIsNear) {	

		/* check here to see if marine is in a proximity door - if so, trigger it to open. */
		{
			MODULEDOORTYPE doorType = ModuleIsADoor(sbPtr->containingModule);
	
			if(doorType == MDT_ProxDoor)	
				((PROXDOOR_BEHAV_BLOCK *)sbPtr->containingModule->m_sbptr->SBdataptr)->marineTrigger = 1;
		}
	
		/* lastly, do a containment test: to make sure that we are inside a module. */
		#if UseLocalAssert   
		{
			VECTORCH localCoords;
			MODULE *thisModule = sbPtr->containingModule;
			
			LOCALASSERT(thisModule);
	
			localCoords = sbPtr->DynPtr->Position;
			localCoords.vx -= thisModule->m_world.vx;
			localCoords.vy -= thisModule->m_world.vy;
			localCoords.vz -= thisModule->m_world.vz;
			
			if(PointIsInModule(thisModule, &localCoords)==0)
			{
				textprint("FAR MARINE MODULE CONTAINMENT FAILURE \n");
				LOCALASSERT(1==0);
			}  
		}
		#endif
	}

}

void GetPointToFaceMarineTowards(STRATEGYBLOCK *sbPtr,VECTORCH *output) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	AIMODULE *targetModule;
	AIMODULE **AdjModuleRefPtr;
	AIMODULE* chosenModule = NULL;
	VECTORCH chosenEpWorld;
	int numFound = 0;

	LOCALASSERT(sbPtr);
	if (sbPtr->containingModule==NULL) {
		return;
	}
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	/* First trap... are you suspicious? */
	if (marineStatusPointer->suspicious) {
		MODULE *suspect_module;

		suspect_module=ModuleFromPosition(&marineStatusPointer->suspect_point,sbPtr->containingModule);
		
		if (suspect_module==NULL) {
			/* Gordon Bennet. */
		 	*output = marineStatusPointer->suspect_point;
			marineStatusPointer->gotapoint=1;
			/* Congratulations. */
			return;
		}

		/* In the same module? */
		if (sbPtr->containingModule->m_aimodule==suspect_module->m_aimodule) {
		 	*output = marineStatusPointer->suspect_point;
			marineStatusPointer->gotapoint=1;
			/* Congratulations. */
			return;
		}

		/* Can you see the suspect point? */
		if (sbPtr->SBdptr!=NULL) {
			if (IsThisObjectVisibleFromThisPosition(sbPtr->SBdptr,&marineStatusPointer->suspect_point,NPC_MAX_VIEWRANGE)) {
			 	*output = marineStatusPointer->suspect_point;
				marineStatusPointer->gotapoint=1;
				/* Congratulations. */
				return;
			}
		}
		
		{
			/* Try to face towards the most appropriate EP. */
			targetModule=GetNextModuleForLink(sbPtr->containingModule->m_aimodule,suspect_module->m_aimodule,7,0);
			if ((targetModule==NULL)||(targetModule==sbPtr->containingModule->m_aimodule)) {
			 	*output = marineStatusPointer->suspect_point;
				marineStatusPointer->gotapoint=1;
				/* Congratulations. */
				return;
			} else {
				/* Get the EP. */
				FARENTRYPOINT *thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
				if(thisEp) {
					VECTORCH thisEpWorld = thisEp->position;

					thisEpWorld.vx += targetModule->m_world.vx;
					thisEpWorld.vy += targetModule->m_world.vy;
					thisEpWorld.vz += targetModule->m_world.vz;			

			 		*output = thisEpWorld;
					marineStatusPointer->gotapoint=1;
					/* Congratulations. */
					return;
			
				}
				/* Still here?  Can't you get anything right? */
			 	*output = marineStatusPointer->suspect_point;
				marineStatusPointer->gotapoint=1;
				/* Congratulations. */
				return;
			}
		}
	}

	/* We know your module, don't we? */

	marineStatusPointer->gotapoint=0;
	targetModule=NULL;

	/* Okay.  If you're a guard, do your job. */
	if (marineStatusPointer->Mission==MM_Guard) {

 		*output = marineStatusPointer->my_facing_point;
		marineStatusPointer->gotapoint=1;
		/* Congratulations. */
		return;
	}

	// SBF - 20080518 - commented out - this block of code is a NO-OP
	// due to the aliased targetModule variable.  This code might have
	// been disabled intentionally? In any case, disabling this code
	// works around a crash in FarNPC_GetTargetAIModuleForMarineRespond
	// during level reloads.
#if 0 // SBF - 20080518 - commented out
	if (NpcSquad.alertZone) {
		/* Might want to face towards trouble. */
		if (sbPtr->containingModule->m_aimodule!=NpcSquad.alertZone) {
			AIMODULE *targetModule=0;
			targetModule = FarNPC_GetTargetAIModuleForMarineRespond(sbPtr);
		}
	}

	/* Did that work? */

	if (targetModule) {
		FARENTRYPOINT *thisEp = GetAIModuleEP(targetModule, sbPtr->containingModule->m_aimodule);
		if(thisEp) {
			VECTORCH thisEpWorld = thisEp->position;

			thisEpWorld.vx += targetModule->m_world.vx;
			thisEpWorld.vy += targetModule->m_world.vy;
			thisEpWorld.vz += targetModule->m_world.vz;			

	 		*output = thisEpWorld;
			marineStatusPointer->gotapoint=1;
			/* Congratulations. */
			return;
			
		}
	}
#endif // SBF - 20080518 - commented out

	AdjModuleRefPtr = sbPtr->containingModule->m_aimodule->m_link_ptrs;	
	/* check if there is a module adjacency list */ 
	if(!AdjModuleRefPtr) {
		/* Just be random, then. */
		return;
	}

	if(!(*AdjModuleRefPtr)) {
		/* Just be random, then. */
		return;
	}

	while(*AdjModuleRefPtr != 0)
	{
		AIMODULE *nextAdjModule = *AdjModuleRefPtr;				
		if (((*(nextAdjModule->m_module_ptrs))->m_flags&MODULEFLAG_AIRDUCT)==0)
		{
			/* Overlook airducts :-) */
			FARENTRYPOINT *thisEp = GetAIModuleEP(nextAdjModule, sbPtr->containingModule->m_aimodule);
			if(thisEp)
			{
				if (thisEp->alien_only==0) {
					/* aha. an ep!... */ 
					VECTORCH thisEpWorld = thisEp->position;
	
					thisEpWorld.vx += nextAdjModule->m_world.vx;
					thisEpWorld.vy += nextAdjModule->m_world.vy;
					thisEpWorld.vz += nextAdjModule->m_world.vz;			
	
					numFound++;
					if(FastRandom()%numFound==0)
					{
						/* take this one */
						chosenModule = nextAdjModule;
						chosenEpWorld = thisEpWorld;
					}
				}
			}
		}
		AdjModuleRefPtr++;
	}
		
	if(chosenModule)
	{
		LOCALASSERT(numFound>=1);
 		*output = chosenEpWorld;
		marineStatusPointer->gotapoint=1;
		/* Congratulations. */
		return;
	}

	/* else... return? */

	return;
}

int MarineSight_FrustrumReject(STRATEGYBLOCK *sbPtr,VECTORCH *localOffset,STRATEGYBLOCK *target) {

	MARINE_STATUS_BLOCK *marineStatusPointer;

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->containingModule); 
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Target==NULL) {
		/* Chop off the top 45 degrees. */
		if ( (localOffset->vz <0) && (
			((localOffset->vy-100)>=0)||(((localOffset->vy-100)<0)&&((-(localOffset->vy-100)) < (-localOffset->vz)))
			)) {
			/* 180 horizontal, 180 vertical. */
			return(1);
		} else {
			return(0);
		}
	} else if (marineStatusPointer->Target!=target) {
	
		if ( (localOffset->vz <0) 
	//		&& (localOffset->vz <  (localOffset->vy+500)) 
	// 		&& (localOffset->vz > -(localOffset->vy+500)) 
			) {
			/* 180 horizontal, 180 vertical. */
			return(1);
		} else {
			return(0);
		}
	} else {
		/* Slightly different now, for error correction. */
		if (localOffset->vz <0) {
			return(1);
		} else {
			/* 270 horizontal, 180 vertical. */
			if ( ((localOffset->vx>0)&&(localOffset->vx<localOffset->vz))
				||((localOffset->vx<0)&&((-localOffset->vx)<localOffset->vz)) ) {
				return(0);
			} else {
				return(1);
			}
		}
	}
}

int Marine_SoundInterest(SOUNDINDEX soundIndex) {

	/* Returns 0->ONE_FIXED scale.  0 is ignored.  Otherwise, larger value means
	lower priority. */
	
	switch (soundIndex) {
		case SID_ALIEN_HISS:
		case SID_ALIEN_HISS1:
		case SID_ALIEN_SCREAM:
		case SID_SWIPE:
		case SID_SWISH:
		case SID_TAIL:
		case SID_PRED_LAUNCHER:
		case SID_PRED_FRISBEE:
		case SID_PRED_PISTOL:
		case SID_PRED_SNARL:
		case SID_PRED_SCREAM1:
		case SID_PRED_LASER:
		case SID_SWIPE2:	
		case SID_SWIPE3:		
		case SID_SWIPE4:		
		case SID_PRED_HISS:		
		case SID_HIT_FLESH:		
		case SID_ALIEN_HIT:			
		case SID_ALIEN_KILL:
		case SID_FHUG_ATTACKLOOP:
		case SID_FHUG_MOVE:
		case SID_BUGDIE1:
		case SID_BUGDIE2:
		case SID_BUGDIE3:
		case SID_MARINE_DEATH1:
		case SID_MARINE_DEATH2:
		case SID_PRED_LOUDROAR:
		case SID_MARINE_HIT:
		case SID_ALIEN_HIT2:
		case SID_PRED_SHORTROAR:
		case SID_PRED_SLASH:
		case SID_RIP:
		case SID_PRED_NEWROAR:
		case SID_PLASMABOLT_DISSIPATE:
		case SID_PLASMABOLT_HIT:
		case SID_ALIEN_JAW_ATTACK:
		case SID_BODY_BEING_HACKED_UP_0:
		case SID_BODY_BEING_HACKED_UP_1:
		case SID_BODY_BEING_HACKED_UP_2:
		case SID_BODY_BEING_HACKED_UP_3:
		case SID_BODY_BEING_HACKED_UP_4:
		case SID_PREDATOR_PICKUP_FIELDCHARGE:
		case SID_PREDATOR_PICKUP_WEAPON:
		case SID_PREDATOR_CLOAKING_ACTIVE:
		case SID_PREDATOR_CLOAKING_DAMAGED:
		case SID_PREDATOR_SPEARGUN_EMPTY:
		case SID_PREDATOR_PLASMACASTER_TARGET_FOUND:
		case SID_PREDATOR_PLASMACASTER_TARGET_LOCKED:
		case SID_PREDATOR_PLASMACASTER_TARGET_LOST:
		case SID_PREDATOR_PLASMACASTER_CHARGING:
		case SID_PREDATOR_PLASMACASTER_EMPTY:
		case SID_PREDATOR_DISK_TARGET_LOCKED:
		case SID_PREDATOR_DISK_FLYING:
		case SID_PREDATOR_DISK_HITTING_TARGET:
		case SID_PREDATOR_DISK_HITTING_WALL:
		case SID_PREDATOR_DISK_BEING_CAUGHT:
		case SID_PREDATOR_DISK_RECOVERED:
		case SID_PREDATOR_VOCAL_SNARL_1:
		case SID_PREDATOR_VOCAL_SNARL_2:
		case SID_ALIEN_TAILUNFURL:
		case SID_ALIEN_TAUNT_1:
		case SID_ALIEN_TAUNT_2:
		case SID_PRED_JUMP_START_1:
		case SID_PRED_JUMP_START_2:
		case SID_PRED_JUMP_START_3:
		case SID_PRED_CLOAKON:
		case SID_PRED_CLOAKOFF:
		case SID_PRED_SMALLLANDING:
		case SID_ED_FACEHUGGERSLAP:
		case SID_GRAPPLE_HIT_WALL:
		case SID_GRAPPLE_THROW:
		case SID_ED_ELEC_DEATH:
			/* Enemy sounds. */
			return(ONE_FIXED>>2);
			break;
		case SID_SWITCH1:
		case SID_SWITCH2:
		case SID_PULSE_START:
		case SID_PULSE_LOOP:
		case SID_PULSE_END:
		case SID_LIFT_START:
		case SID_LIFT_LOOP:
		case SID_LIFT_END:
		case SID_VISION_ON:
		case SID_VISION_LOOP:
		case SID_FIRE:
		case SID_PICKUP:
		case SID_SPLASH1:
		case SID_SPLASH2:
		case SID_SPLASH3:
		case SID_SPLASH4:
		case SID_POWERUP:
		case SID_POWERDN:
		case SID_ACID_SPRAY:
		case SID_DOORSTART:
		case SID_DOORMID:
		case SID_DOOREND:
		case SID_BORGON:
		case SID_SPARKS:
		case SID_STOMP:
		case SID_LOADMOVE:
		case SID_NOAMMO:
		case SID_LONGLOAD:
		case SID_NADELOAD:
		case SID_NADEFIRE:
		case SID_NADEEXPLODE:
		case SID_SHRTLOAD:
		case SID_INCIN_START:
		case SID_INCIN_LOOP:
		case SID_INCIN_END:
		case SID_ROCKFIRE:
		case SID_SHOTGUN:
		case SID_SMART1:
		case SID_SMART2:
		case SID_SMART3:
		case SID_SENTRY_GUN:
		case SID_SENTRY_END:
		case SID_NICE_EXPLOSION:
		case SID_EXPLOSION:
		case SID_MINIGUN_END:
		case SID_MINIGUN_LOOP:
		case SID_FRAG_RICOCHETS:
		case SID_SPEARGUN_HITTING_WALL:
		case SID_DISC_STICKSINWALL:
		case SID_PREDATOR_PLASMACASTER_REDTRIANGLES:
		case SID_WIL_PRED_PISTOL_EXPLOSION:
		case SID_PROX_GRENADE_READYTOBLOW:
		case SID_PROX_GRENADE_ACTIVE:
		case SID_ED_GRENADE_EXPLOSION:
		case SID_ED_GRENADE_PROXEXPLOSION:
		case SID_ED_MOLOTOV_EXPLOSION:
		case SID_SENTRYGUNDEST:
			return(ONE_FIXED);
			break;
		case SID_ARMSTART:
		case SID_ARMMID:
		case SID_ARMEND:
		case SID_TRACKER_CLICK:
		case SID_TRACKER_WHEEP:
		case SID_RICOCH1:
		case SID_RICOCH2:
		case SID_RICOCH3:
		case SID_RICOCH4:
		case SID_TELETEXT:
		case SID_TRACKER_WHEEP_HIGH:
		case SID_TRACKER_WHEEP_LOW:
		case SID_PULSE_RIFLE_FIRING_EMPTY:
		case SID_THROW_FLARE:
		case SID_CONSOLE_ACTIVATES:
		case SID_CONSOLE_DEACTIVATES:
		case SID_CONSOLE_MARINEMESSAGE:
		case SID_CONSOLE_ALIENMESSAGE:
		case SID_CONSOLE_PREDATORMESSAGE:
		case SID_MINIGUN_READY:
		case SID_MINIGUN_EMPTY:
		case SID_SMART_MODESWITCH:
		case SID_GRENADE_BOUNCE:
		case SID_BURNING_FLARE:
		case SID_FLAMETHROWER_PILOT_LIGHT:
		case SID_MARINE_JUMP_START:
		case SID_MARINE_JUMP_END:
		case SID_MARINE_PICKUP_WEAPON:
		case SID_MARINE_PICKUP_AMMO:
		case SID_MARINE_PICKUP_ARMOUR:
		case SID_SENTRYGUN_LOCK:
		case SID_SENTRYGUN_SHUTDOWN:
		case SID_WIL_MINIGUN_READY:
		case SID_SADAR_FIRE:
		case SID_MARINE_JUMP_START_2:
		case SID_MARINE_JUMP_START_3:
		case SID_MARINE_JUMP_START_4:
		case SID_ED_LARGEWEAPONDROP:
		case SID_MENUS_SELECT_ITEM:
		case SID_MENUS_CHANGE_ITEM:
		case SID_PRED_ZOOM_IN:
		case SID_PRED_ZOOM_OUT:
		case SID_MARINE_SMALLLANDING:
		case SID_LIGHT_FLICKER_ON:
		case SID_ED_SENTRYTURN01:
		case SID_PULSE_SWIPE01:
		case SID_PULSE_SWIPE02:
		case SID_PULSE_SWIPE03:
		case SID_PULSE_SWIPE04:
		case SID_ED_JETPACK_START:
		case SID_ED_JETPACK_MID:
		case SID_ED_JETPACK_END:
		case SID_IMAGE:
		case SID_IMAGE_OFF:
			/* Basic marine sounds, and the sentrygun. */
			return(0);
			break;
		case SID_STARTOF_LOADSLOTS:
		case SID_UNUSED_125:
		case SID_UNUSED_126:
		case SID_UNUSED_127:
		case SID_UNUSED_128:
		case SID_UNUSED_129:
		case SID_UNUSED_130:
		case SID_UNUSED_131:
		case SID_UNUSED_132:
		case SID_UNUSED_133:
		case SID_UNUSED_134:
		case SID_UNUSED_135:
		case SID_UNUSED_136:
		case SID_UNUSED_137:
		case SID_UNUSED_138:
		case SID_UNUSED_139:
		case SID_UNUSED_140:
		case SID_UNUSED_141:
		case SID_UNUSED_142:
		case SID_UNUSED_143:
		case SID_UNUSED_144:
		case SID_UNUSED_145:
		case SID_UNUSED_146:
		case SID_UNUSED_147:
		case SID_UNUSED_148:
		case SID_UNUSED_149:
		case SID_ENDOF_LOADSLOTS:
			/* Environment sounds. */
			return(ONE_FIXED<<1);
			break;
		default:
			/* Eh?  Could be a scream... */
			return(ONE_FIXED<<1);
			break;
	}

}

int Marine_SoundCourageBonus(SOUNDINDEX soundIndex) {

	/* Returns a FRI value. + is GOOD! */
	
	switch (soundIndex) {
		case SID_ALIEN_HISS:
		case SID_ALIEN_HISS1:
		case SID_ALIEN_SCREAM:
		case SID_SWIPE:
		case SID_SWISH:
		case SID_TAIL:
		case SID_PRED_LAUNCHER:
		case SID_PRED_FRISBEE:
		case SID_PRED_PISTOL:
		case SID_PRED_SNARL:
		case SID_PRED_SCREAM1:
		case SID_PRED_LASER:
		case SID_SWIPE2:	
		case SID_SWIPE3:		
		case SID_SWIPE4:		
		case SID_PRED_HISS:		
		case SID_HIT_FLESH:		
		case SID_ALIEN_HIT:			
		case SID_ALIEN_KILL:
		case SID_FHUG_ATTACKLOOP:
		case SID_FHUG_MOVE:
		case SID_BUGDIE1:
		case SID_BUGDIE2:
		case SID_BUGDIE3:
		case SID_MARINE_DEATH1:
		case SID_MARINE_DEATH2:
		case SID_PRED_LOUDROAR:
		case SID_MARINE_HIT:
		case SID_ALIEN_HIT2:
		case SID_PRED_SHORTROAR:
		case SID_PRED_SLASH:
		case SID_RIP:
		case SID_PRED_NEWROAR:
		case SID_PLASMABOLT_DISSIPATE:
		case SID_PLASMABOLT_HIT:
		case SID_ALIEN_JAW_ATTACK:
		case SID_BODY_BEING_HACKED_UP_0:
		case SID_BODY_BEING_HACKED_UP_1:
		case SID_BODY_BEING_HACKED_UP_2:
		case SID_BODY_BEING_HACKED_UP_3:
		case SID_BODY_BEING_HACKED_UP_4:
		case SID_PREDATOR_PICKUP_FIELDCHARGE:
		case SID_PREDATOR_PICKUP_WEAPON:
		case SID_PREDATOR_CLOAKING_ACTIVE:
		case SID_PREDATOR_CLOAKING_DAMAGED:
		case SID_PREDATOR_SPEARGUN_EMPTY:
		case SID_PREDATOR_PLASMACASTER_TARGET_FOUND:
		case SID_PREDATOR_PLASMACASTER_TARGET_LOCKED:
		case SID_PREDATOR_PLASMACASTER_TARGET_LOST:
		case SID_PREDATOR_PLASMACASTER_CHARGING:
		case SID_PREDATOR_PLASMACASTER_EMPTY:
		case SID_PREDATOR_DISK_TARGET_LOCKED:
		case SID_PREDATOR_DISK_FLYING:
		case SID_PREDATOR_DISK_HITTING_TARGET:
		case SID_PREDATOR_DISK_HITTING_WALL:
		case SID_PREDATOR_DISK_BEING_CAUGHT:
		case SID_PREDATOR_DISK_RECOVERED:
		case SID_PREDATOR_VOCAL_SNARL_1:
		case SID_PREDATOR_VOCAL_SNARL_2:
		case SID_ALIEN_TAILUNFURL:
		case SID_ALIEN_TAUNT_1:
		case SID_ALIEN_TAUNT_2:
		case SID_PRED_JUMP_START_1:
		case SID_PRED_JUMP_START_2:
		case SID_PRED_JUMP_START_3:
		case SID_PRED_CLOAKON:
		case SID_PRED_CLOAKOFF:
		case SID_PRED_SMALLLANDING:
		case SID_ED_FACEHUGGERSLAP:
		case SID_GRAPPLE_HIT_WALL:
		case SID_GRAPPLE_THROW:
		case SID_ED_ELEC_DEATH:
			/* Enemy sounds! */
			return(MUL_FIXED(NormalFrameTime,-3000));
			break;
		case SID_MINIGUN_END:
		case SID_MINIGUN_LOOP:
		case SID_INCIN_START:
		case SID_INCIN_LOOP:
		case SID_INCIN_END:
		case SID_ROCKFIRE:
		case SID_PULSE_START:
		case SID_PULSE_LOOP:
		case SID_PULSE_END:
		case SID_SHOTGUN:
		case SID_SMART1:
		case SID_SMART2:
		case SID_SMART3:
		case SID_TRACKER_WHEEP_HIGH:
		case SID_TRACKER_WHEEP_LOW:
		case SID_PULSE_RIFLE_FIRING_EMPTY:
		case SID_THROW_FLARE:
		case SID_CONSOLE_ACTIVATES:
		case SID_CONSOLE_DEACTIVATES:
		case SID_CONSOLE_MARINEMESSAGE:
		case SID_CONSOLE_ALIENMESSAGE:
		case SID_CONSOLE_PREDATORMESSAGE:
		case SID_MINIGUN_READY:
		case SID_MINIGUN_EMPTY:
		case SID_SMART_MODESWITCH:
		case SID_GRENADE_BOUNCE:
		case SID_BURNING_FLARE:
		case SID_FLAMETHROWER_PILOT_LIGHT:
		case SID_MARINE_JUMP_START:
		case SID_MARINE_JUMP_END:
		case SID_MARINE_PICKUP_WEAPON:
		case SID_MARINE_PICKUP_AMMO:
		case SID_MARINE_PICKUP_ARMOUR:
		case SID_SENTRYGUN_LOCK:
		case SID_SENTRYGUN_SHUTDOWN:
		case SID_WIL_MINIGUN_READY:
		case SID_SADAR_FIRE:
		case SID_MARINE_JUMP_START_2:
		case SID_MARINE_JUMP_START_3:
		case SID_MARINE_JUMP_START_4:
		case SID_ED_GRENADE_EXPLOSION:
		case SID_ED_GRENADE_PROXEXPLOSION:
		case SID_ED_MOLOTOV_EXPLOSION:
		case SID_ED_LARGEWEAPONDROP:
		case SID_MENUS_SELECT_ITEM:
		case SID_MENUS_CHANGE_ITEM:
		case SID_IMAGE:
		case SID_IMAGE_OFF:
			/* Marine type sounds. */
			return(MUL_FIXED(NormalFrameTime,300));
			break;
		case SID_SWITCH1:
		case SID_SWITCH2:
		case SID_LIFT_START:
		case SID_LIFT_LOOP:
		case SID_LIFT_END:
		case SID_VISION_ON:
		case SID_VISION_LOOP:
		case SID_FIRE:
		case SID_PICKUP:
		case SID_SPLASH1:
		case SID_SPLASH2:
		case SID_SPLASH3:
		case SID_SPLASH4:
		case SID_POWERUP:
		case SID_POWERDN:
		case SID_ACID_SPRAY:
		case SID_DOORSTART:
		case SID_DOORMID:
		case SID_DOOREND:
		case SID_BORGON:
		case SID_SPARKS:
		case SID_STOMP:
		case SID_LOADMOVE:
		case SID_NOAMMO:
		case SID_LONGLOAD:
		case SID_NADELOAD:
		case SID_NADEFIRE:
		case SID_NADEEXPLODE:
		case SID_SHRTLOAD:
		case SID_SENTRY_GUN:
		case SID_SENTRY_END:
		case SID_NICE_EXPLOSION:
		case SID_EXPLOSION:
		case SID_FRAG_RICOCHETS:
		case SID_SPEARGUN_HITTING_WALL:
		case SID_DISC_STICKSINWALL:
		case SID_PREDATOR_PLASMACASTER_REDTRIANGLES:
		case SID_WIL_PRED_PISTOL_EXPLOSION:
		case SID_PROX_GRENADE_READYTOBLOW:
		case SID_PROX_GRENADE_ACTIVE:
		case SID_SENTRYGUNDEST:
			/* Weird sounds. */
			return(ONE_FIXED);
			break;
		case SID_ARMSTART:
		case SID_ARMMID:
		case SID_ARMEND:
		case SID_TRACKER_CLICK:
		case SID_TRACKER_WHEEP:
		case SID_TELETEXT:
		case SID_RICOCH1:
		case SID_RICOCH2:
		case SID_RICOCH3:
		case SID_RICOCH4:
		case SID_NOSOUND:
		case SID_PRED_ZOOM_IN:
		case SID_PRED_ZOOM_OUT:
		case SID_MARINE_SMALLLANDING:
		case SID_LIGHT_FLICKER_ON:
		case SID_ED_SENTRYTURN01:
		case SID_PULSE_SWIPE01:
		case SID_PULSE_SWIPE02:
		case SID_PULSE_SWIPE03:
		case SID_PULSE_SWIPE04:
		case SID_ED_JETPACK_START:
		case SID_ED_JETPACK_MID:
		case SID_ED_JETPACK_END:
			/* Basic marine sounds, and the sentrygun. */
			return(0);
			break;
		case SID_STARTOF_LOADSLOTS:
		case SID_UNUSED_125:
		case SID_UNUSED_126:
		case SID_UNUSED_127:
		case SID_UNUSED_128:
		case SID_UNUSED_129:
		case SID_UNUSED_130:
		case SID_UNUSED_131:
		case SID_UNUSED_132:
		case SID_UNUSED_133:
		case SID_UNUSED_134:
		case SID_UNUSED_135:
		case SID_UNUSED_136:
		case SID_UNUSED_137:
		case SID_UNUSED_138:
		case SID_UNUSED_139:
		case SID_UNUSED_140:
		case SID_UNUSED_141:
		case SID_UNUSED_142:
		case SID_UNUSED_143:
		case SID_UNUSED_144:
		case SID_UNUSED_145:
		case SID_UNUSED_146:
		case SID_UNUSED_147:
		case SID_UNUSED_148:
		case SID_UNUSED_149:
		case SID_ENDOF_LOADSLOTS:
			/* Environment sounds. */
			return(MUL_FIXED(NormalFrameTime,-300));
			break;
		default:
			/* Eh?  Could be a scream... */
			return(MUL_FIXED(NormalFrameTime,-1500));
			break;
	}

}

void DoMarineHearing(STRATEGYBLOCK *sbPtr) {

	int a,nearest,neardist,dist,priority;
	MARINE_STATUS_BLOCK *marineStatusPointer;
	VECTORCH offset;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);	          		

	#if 0
	/* Hearing is low priority. */
	if (marineStatusPointer->suspicious) {
		return;
	}
	#endif

	nearest=-1;
	neardist=10000000;
	/* Hence, if you're still interested in something else, return. */

	for(a=0;a<SOUND_MAXACTIVE;a++) 
	{
		/* Ignore sounds with no position. */
		if ((ActiveSounds[a].threedee)&&(ActiveSounds[a].loop==0)&&(ActiveSounds[a].marine_ignore==0)
			&&(ActiveSounds[a].soundIndex!=SID_NOSOUND)) {
			priority=Marine_SoundInterest(ActiveSounds[a].soundIndex);
			if (marineStatusPointer->Android==0) {
				marineStatusPointer->Courage+=Marine_SoundCourageBonus(ActiveSounds[a].soundIndex);
			}
			if (priority) {
				int test_dist;
				/* Interesting sound... */
				offset.vx=sbPtr->DynPtr->Position.vx-ActiveSounds[a].threedeedata.position.vx;
				offset.vy=sbPtr->DynPtr->Position.vy-ActiveSounds[a].threedeedata.position.vy;
				offset.vz=sbPtr->DynPtr->Position.vz-ActiveSounds[a].threedeedata.position.vz;

				dist=Approximate3dMagnitude(&offset);

				if (ActiveSounds[a].threedeedata.outer_range==-1) {
					test_dist=100000;
					/* 100 m. */
				} else {
					test_dist=ActiveSounds[a].threedeedata.outer_range;
				}

				if (dist<=test_dist) {
					/* In range.  Modify by priority. */
					dist=MUL_FIXED(dist,priority);

					if (dist<neardist) {
						/* Got one! */
						nearest=a;
						neardist=dist;
					}
				}
			}
		}
	}

	/* Finally, test the player. */
	{
		extern int playerNoise;

		if (playerNoise) {
		
			offset.vx=sbPtr->DynPtr->Position.vx-Player->ObStrategyBlock->DynPtr->Position.vx;
			offset.vy=sbPtr->DynPtr->Position.vy-Player->ObStrategyBlock->DynPtr->Position.vy;
			offset.vz=sbPtr->DynPtr->Position.vz-Player->ObStrategyBlock->DynPtr->Position.vz;
		
			dist=Approximate3dMagnitude(&offset);

			if (dist<=100000) {
				/* In range.  Assume priority is ONE_FIXED. */
				if (dist<neardist) {
					/* Got one! */
					nearest=-2;
					neardist=dist;
				}
			}
		
		}
	}

	if (nearest!=-1) {
		int level,priority;

		if (nearest==-2) {
			/* Heard the player! */

			marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
			marineStatusPointer->suspect_point=Player->ObStrategyBlock->DynPtr->Position;
			/* Set this to zero when you get a *new* suspicion. */
			marineStatusPointer->previous_suspicion=0;
			marineStatusPointer->using_squad_suspicion=0;

			level=2;

		} else {

			marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
			marineStatusPointer->suspect_point=ActiveSounds[nearest].threedeedata.position;
			/* Set this to zero when you get a *new* suspicion. */
			marineStatusPointer->previous_suspicion=0;
			marineStatusPointer->using_squad_suspicion=0;

			priority=Marine_SoundInterest(ActiveSounds[nearest].soundIndex);
			if (priority>ONE_FIXED) {
				level=1;
			} else if (priority<ONE_FIXED) {
				level=3;
			} else {
				level=2;
			}
		
		}

		PointAlert(level,&marineStatusPointer->suspect_point);
	}

}

static STATE_RETURN_CONDITION Execute_MNS_Taunting(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	}

	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		return(SRC_Request_Wait);
	} else {
		return(SRC_No_Change);
	}

}

static STATE_RETURN_CONDITION Execute_MNS_Reloading(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	}

	/* Clip management. */
	if (marineStatusPointer->My_Weapon->ClipName) {
		SECTION_DATA *clip;
		clip=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ClipName);
		if (clip) {
			/* Now, check keyframe flags. */
			if (marineStatusPointer->HModelController.keyframe_flags&4) {
				/* Vanish it. */
				clip->flags|=(section_data_terminate_here|section_data_notreal);
			}

			if (marineStatusPointer->HModelController.keyframe_flags&1) {
				/* Trim it off. */
				MakeHierarchicalDebris(NULL,clip, &clip->World_Offset, &clip->SecMat,NULL,0);
			}

			if (marineStatusPointer->HModelController.keyframe_flags&2) {
				/* Put it back. */
				if ((marineStatusPointer->My_Weapon->id==MNPCW_SADAR)
					||(marineStatusPointer->My_Weapon->id==MNPCW_Skeeter)) {
					SECTION *root_section;
					/* Heaven help us. */
					clip->flags&=(~(section_data_terminate_here|section_data_notreal));

					root_section=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->HierarchyName);
					GLOBALASSERT(root_section);

					Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root_section,0,1,1);
					ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
					marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
					marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
				} else {
					clip->flags&=(~(section_data_terminate_here|section_data_notreal));
				}
			}
		}
	}
	
	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		marineStatusPointer->clipammo=marineStatusPointer->My_Weapon->clip_size;
		return(SRC_Request_Approach);
	} else {
		return(SRC_No_Change);
	}
	
}

static STATE_RETURN_CONDITION Execute_MNS_PanicReloading(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_PanicScream(sbPtr);
				} else {
					Marine_AngryScream(sbPtr);
				}
			} else {
				/* Scream anyway? */
				Marine_PanicScream(sbPtr);
				/* Open the mouth? */
				Marine_AssumePanicExpression(sbPtr);
			}
		}
	}

	/* Clip management. */
	if (marineStatusPointer->My_Weapon->ClipName) {
		SECTION_DATA *clip;
		clip=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ClipName);
		if (clip) {
			/* Now, check keyframe flags. */
			if (marineStatusPointer->HModelController.keyframe_flags&4) {
				/* Vanish it. */
				clip->flags|=(section_data_terminate_here|section_data_notreal);
			}

			if (marineStatusPointer->HModelController.keyframe_flags&1) {
				/* Trim it off. */
				MakeHierarchicalDebris(NULL,clip, &clip->World_Offset, &clip->SecMat,NULL,0);
			}

			if (marineStatusPointer->HModelController.keyframe_flags&2) {
				/* Put it back. */
				if (marineStatusPointer->My_Weapon->id==MNPCW_SADAR) {
					SECTION *root_section;
					/* Heaven help us. */
					clip->flags&=(~(section_data_terminate_here|section_data_notreal));

					root_section=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->HierarchyName);
					GLOBALASSERT(root_section);

					Transmogrify_HModels(sbPtr,&marineStatusPointer->HModelController,root_section,0,1,1);
					ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
					marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
					marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
				} else {
					clip->flags&=(~(section_data_terminate_here|section_data_notreal));
				}
			}
		}
	}
	
	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		marineStatusPointer->clipammo=marineStatusPointer->My_Weapon->clip_size;
		return(SRC_Request_Approach);
	} else {
		return(SRC_No_Change);
	}
	
}

static STATE_RETURN_CONDITION Execute_MNS_GetWeapon(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	}

	if (HModelAnimation_IsFinished(&marineStatusPointer->HModelController)) {
		return(SRC_Request_Wait);
	} else {
		return(SRC_No_Change);
	}

}

int NPCFireMinigun_Core(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	marineStatusPointer->weapon_variable+=(NormalFrameTime<<7);

	if (marineStatusPointer->weapon_variable>=MINIGUN_MAX_SPEED) {
	
		marineStatusPointer->weapon_variable=MINIGUN_MAX_SPEED;
		
		/* Fire! */
		DischargeLOSWeapon_Core(sbPtr);
		
		return(1);
	}		

	/* Maintain_Minigun called anyway. */

	return(0);

}


void NPC_Maintain_Minigun(STRATEGYBLOCK *sbPtr, DELTA_CONTROLLER *mgd) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);
	
	if (mgd==NULL) {
		return;
	}

   	if ((marineStatusPointer->behaviourState!=MBS_Firing)
   		||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)
   		||(marineStatusPointer->HModelController.Playing==0)) {
		/* Not firing. */
		marineStatusPointer->weapon_variable-=(NormalFrameTime<<3);
		if (marineStatusPointer->weapon_variable<MINIGUN_IDLE_SPEED) {
			marineStatusPointer->weapon_variable=MINIGUN_IDLE_SPEED;
		}
	}

	if (marineStatusPointer->weapon_variable!=marineStatusPointer->weapon_variable2) {
		int hmspinrate;

		if (marineStatusPointer->weapon_variable) {
						
			hmspinrate=DIV_FIXED(ONE_FIXED,marineStatusPointer->weapon_variable);

			Delta_Sequence_ChangeSpeed(mgd,hmspinrate);
			mgd->Playing=1;
		} else {
			mgd->Playing=0;
		}
	}

	marineStatusPointer->weapon_variable2=marineStatusPointer->weapon_variable;

}

static STATE_RETURN_CONDITION Execute_MNS_DischargeMinigun(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine firing... ");
	#endif

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state*/

	if (marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize) {
		/* Keep firing! */
	} else {

		if(!MarineCanSeeTarget(sbPtr))
		{
	
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
	
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			marineStatusPointer->lastroundhit=0;
			marineStatusPointer->lasthitsection=NULL;
			return(SRC_Request_Wait);
		}
	}

	if (marineStatusPointer->Target) {
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* Here we must have a target.  Renew suspicion for new arrivals. */
		if (NpcSquad.Squad_Suspicion==0) {
			PointAlert(2,&marineStatusPointer->weaponTarget);
		}
	}
	/* Otherwise, stay facing the same way. */

	/* orientate to firing point first */
	orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	orientationDirn.vy = 0;
	orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

	/* I have a cunning plan... */
	{
		DELTA_CONTROLLER *delta;

		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			if (!(DeltaAnimation_IsFinished(delta))) {
				correctlyOrientated=0;
			}
		}
	}
	/* I have another cunning plan... */
	if ((marineStatusPointer->volleySize>0)&&
		(marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		correctlyOrientated=1;
	}

	/* we are not correctly orientated to the target: this could happen because we have
	just entered this state, or the target has moved during firing */
	if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#if MARINE_STATE_PRINT
		textprint("Turning to face.\n");
		#endif
		marineStatusPointer->lastroundhit=0;
		marineStatusPointer->lasthitsection=NULL;
		return(SRC_No_Change);
	}

	NPCFireMinigun_Core(sbPtr);

	if (marineStatusPointer->Target==NULL) {
		/* Getting out of here! */
		return(SRC_No_Change);
	}

	relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
	relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
	relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
	  
	relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
	relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
	relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);

	range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

	/* Did we get him? */
	if ((NPC_IsDead(marineStatusPointer->Target))
		&&(marineStatusPointer->volleySize>=marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			if (Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr)==NULL) {
				/* Huzzah! */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				marineStatusPointer->lastroundhit=0;
				marineStatusPointer->lasthitsection=NULL;
				return(SRC_Request_Taunt);
			}
		}
	}
	
	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif

		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_Request_Fire);
	}
	else
	{
		/* we are far enough away, so return to approach */

		/* ... and remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}

		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_NullPanicFire(STRATEGYBLOCK *sbPtr) {

	/* This MUST exist to redirect weapons with no panic fire. */

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	return(SRC_Request_Wait);

}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireLOSWeapon(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;
	int volleytime,volleyrounds;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int threshold22,threshold45,threshold67,threshold90,ideal_sequence;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				threshold22=70;
				threshold45=220;
				threshold67=490;
				threshold90=730;
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_67) {
				threshold22=70;
				threshold45=220;
				threshold67=490;
				threshold90=768;
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				threshold22=70;
				threshold45=220;
				threshold67=512;
				threshold90=768;
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_22) {
				threshold22=70;
				threshold45=256;
				threshold67=512;
				threshold90=768;
			} else {
				threshold22=90;
				threshold45=256;
				threshold67=512;
				threshold90=768;
			}

			if (angle1>threshold90) {
				ideal_sequence=4;
			} else if (angle1>threshold67) {
				ideal_sequence=3;
			} else if (angle1>threshold45) {
				ideal_sequence=2;
			} else if (angle1>threshold22) {
				ideal_sequence=1;
			} else {
				ideal_sequence=0;
			}

			sequence=-1;
			while (sequence==-1) {
				switch (ideal_sequence) {
					case 0:
					default:
						sequence=0;
						break;
					case 1:
						if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_22)) {
							sequence=1;
						} else {
							ideal_sequence=0;
						}
						break;
					case 2:
						if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
							sequence=2;
						} else {
							ideal_sequence=1;
						}
						break;
					case 3:
						if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_67)) {
							sequence=3;
						} else {
							ideal_sequence=2;
						}
						break;
					case 4:
						if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
							sequence=4;
						} else {
							ideal_sequence=3;
						}
						break;
				}
			}
		}

		switch (sequence) {
			case 4:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 3:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_67)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_67,-1,(ONE_FIXED>>3));
				}
				break;
			case 2:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_22)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_22,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_PanicReload);
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if (marineStatusPointer->Target==NULL) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);
		
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		if (correctlyOrientated) {
			if (MarineCanSeeTarget(sbPtr)) {
				/* Lost target... */
				marineStatusPointer->Target=NULL;
			}
		}	
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	/* look after the gun flash */
	if(marineStatusPointer->myGunFlash) {
		MaintainMarineGunFlash(sbPtr);
	} else {
		CreateMarineGunFlash(sbPtr);
	}

	/* look after the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
	else 
	{ 
		Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
		Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;
		
	/* Volleysize is now rounds fired this state. */

	volleytime=marineStatusPointer->My_Weapon->FiringTime-marineStatusPointer->stateTimer;
	/* It was that or reverse the state timer for this state. */
	volleyrounds=MUL_FIXED(volleytime,marineStatusPointer->My_Weapon->FiringRate);
	volleyrounds>>=ONE_FIXED_SHIFT;

	volleyrounds-=marineStatusPointer->volleySize;
	marineStatusPointer->volleySize+=volleyrounds;

	LOCALASSERT(volleyrounds>=0);

	if (marineStatusPointer->clipammo!=-1) {
		/* We're counting ammo. */
		if (volleyrounds>marineStatusPointer->clipammo) {
			volleyrounds=marineStatusPointer->clipammo;
		}
		marineStatusPointer->clipammo-=volleyrounds;
		LOCALASSERT(marineStatusPointer->clipammo>=0);
	}

	/* Now fire volleyrounds bullets. */
	if (volleyrounds>0) {
		VECTORCH shotvector;

		shotvector.vx=0;
		shotvector.vy=0;
		shotvector.vz=65535;
		RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);

		CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, volleyrounds,1);

	}

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireFlamethrower(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int highthreshold,topthreshold;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				highthreshold=90;
				topthreshold=(1024-300);
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				highthreshold=90;
				topthreshold=(1024-256);
			} else {
				highthreshold=128;
				topthreshold=(1024-256);
			}

			if (angle1>topthreshold) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
					sequence=2;
				} else {
					sequence=0;
				}
			} else if (angle1>highthreshold) { //Was 256!
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
					sequence=1;
				} else {
					sequence=0;
				}
			} else {
				sequence=0;
			}
		}

		switch (sequence) {
			case 2:
				/* Escape for civvie flamethrower here? */
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->Target==NULL) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);

		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

		if (correctlyOrientated) {
			if (MarineCanSeeTarget(sbPtr)) {
				/* Lost target... */
				marineStatusPointer->Target=NULL;
			}
		}	
	}

	if (marineStatusPointer->clipammo==0) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			if (marineStatusPointer->My_Weapon->ARealMarine) {
				return(SRC_Request_PullPistol);
			} else {
				return(SRC_Request_PanicReload);
			}
		} else {
			return(SRC_Request_PanicReload);
		}
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	/* look after the gun flash */
	if(marineStatusPointer->myGunFlash) {
		/* No gunflash, neither. */
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
	}

	/* look after the sound */
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
	else 
	{ 
		Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
		Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
	}

	marineStatusPointer->stateTimer -= NormalFrameTime;
		
	MarineFireFlameThrower(sbPtr);

	/* Lighting? */
	if (sbPtr->SBdptr) {
		AddLightingEffectToObject(sbPtr->SBdptr,LFX_MUZZLEFLASH);
	}

	if (marineStatusPointer->clipammo>0) {
		marineStatusPointer->clipammo-=NormalFrameTime;
		if (marineStatusPointer->clipammo<0) {
			marineStatusPointer->clipammo=0;
		}
	}

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

void Marine_SwitchExpression(STRATEGYBLOCK *sbPtr,int state) {

	MARINE_STATUS_BLOCK *marineStatusPointer;
	SECTION_DATA *head;
	TXACTRLBLK *tacb;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    	
	LOCALASSERT(marineStatusPointer);

	head=GetThisSectionData(marineStatusPointer->HModelController.section_data,"head");
	
	marineStatusPointer->Expression=state;
	marineStatusPointer->Blink=-1;
	
	if (head) {
		if ((head->flags&section_data_notreal)==0) {

			tacb=head->tac_ptr;

			while (tacb) {
				tacb->tac_sequence = state ;
				tacb->tac_txah_s = GetTxAnimHeaderFromShape(tacb, head->ShapeNum);
			
				tacb=tacb->tac_next;
			}
		}
	}
}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireGL(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;
	int keyframeflags,a;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int highthreshold,topthreshold;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				highthreshold=90;
				topthreshold=(1024-300);
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				highthreshold=90;
				topthreshold=(1024-256);
			} else {
				highthreshold=128;
				topthreshold=(1024-256);
			}

			if (angle1>topthreshold) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
					sequence=2;
				} else {
					sequence=0;
				}
			} else if (angle1>highthreshold) { //Was 256!
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
					sequence=1;
				} else {
					sequence=0;
				}
			} else {
				sequence=0;
			}
		}

		switch (sequence) {
			case 2:
				/* Escape for civvie flamethrower here? */
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->Target==NULL) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);

		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

		if (correctlyOrientated) {
			if (MarineCanSeeTarget(sbPtr)) {
				/* Lost target... */
				marineStatusPointer->Target=NULL;
			}
		}	
	}

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		return(SRC_Request_PanicReload);
	}		

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Stop cues anyway? */
	if(marineStatusPointer->myGunFlash) {
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;				
	}
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	if (marineStatusPointer->Target) {
		int range;

		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

		/* Are they, by some chance, really close? */
		if (range<marineStatusPointer->My_Weapon->MinRange) {
			if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
				/* Stay cool. */
				return(SRC_Request_PullPistol);
			}
			/* Probably can't flee... :-) */
		}
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	keyframeflags=marineStatusPointer->HModelController.keyframe_flags>>1;

	/* I know there are only four firing points... */
	for (a=0; a<4; a++) {
		if (keyframeflags&1) {
			/* Fire a grenade! */

			/* look after the gun flash */
			if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
			else CreateMarineGunFlash(sbPtr);

			/* look after the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
			else { 
				Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
				Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
			}

			LOCALASSERT(marineStatusPointer->My_Gunflash_Section);
			CreateGrenadeKernel(I_BehaviourGrenade, &marineStatusPointer->My_Gunflash_Section->World_Offset, &marineStatusPointer->My_Gunflash_Section->SecMat,0);
			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}
		keyframeflags>>=1;
	}
	/* ...and they are all identical. */

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireMinigun(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int highthreshold,topthreshold;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				highthreshold=90;
				topthreshold=(1024-300);
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				highthreshold=90;
				topthreshold=(1024-256);
			} else {
				highthreshold=128;
				topthreshold=(1024-256);
			}

			if (angle1>topthreshold) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
					sequence=2;
				} else {
					sequence=0;
				}
			} else if (angle1>highthreshold) { //Was 256!
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
					sequence=1;
				} else {
					sequence=0;
				}
			} else {
				sequence=0;
			}
		}

		switch (sequence) {
			case 2:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->clipammo==0) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			return(SRC_Request_PullPistol);
		} else {
			return(SRC_Request_PanicReload);
		}
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if ((marineStatusPointer->Target==NULL)&&(marineStatusPointer->volleySize>=marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		if (marineStatusPointer->Target) {

			NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
	
			/* orientate to firing point first */
			orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

			if (correctlyOrientated) {
				if (MarineCanSeeTarget(sbPtr)) {
					/* Lost target... */
					marineStatusPointer->Target=NULL;
				}
			}	
		} else {
			/* Who cares! */
			correctlyOrientated=1;
		}
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	NPCFireMinigun_Core(sbPtr);

	/* Now, consider termination. */

	if (marineStatusPointer->volleySize<MINIGUN_MINIMUM_BURST) {
		/* Can't terminate yet. */
		return(SRC_No_Change);
	}

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MNS_PanicFirePistol(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;
	int a,keyframeflags;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int highthreshold,topthreshold;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				highthreshold=90;
				topthreshold=(1024-300);
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				highthreshold=90;
				topthreshold=(1024-256);
			} else {
				highthreshold=128;
				topthreshold=(1024-256);
			}

			if (angle1>topthreshold) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
					sequence=2;
				} else {
					sequence=0;
				}
			} else if (angle1>highthreshold) { //Was 256!
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
					sequence=1;
				} else {
					sequence=0;
				}
			} else {
				sequence=0;
			}
		}

		switch (sequence) {
			case 2:
				/* Escape for civvie flamethrower here? */
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_PanicReload);
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if (marineStatusPointer->Target==NULL) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);
		
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		if (correctlyOrientated) {
			if (MarineCanSeeTarget(sbPtr)) {
				/* Lost target... */
				marineStatusPointer->Target=NULL;
			}
		}	
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Stop cues anyway? */
	if(marineStatusPointer->myGunFlash) {
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;				
	}
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	keyframeflags=marineStatusPointer->HModelController.keyframe_flags>>1;

	/* I know there are only six possible firing points... */
	for (a=0; a<6; a++) {
		if ((keyframeflags&1)||((a==0)&&(marineStatusPointer->HModelController.Tweening==Controller_EndTweening))) {
			/* Fire a shot. */

			/* look after the gun flash */
			if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
			else CreateMarineGunFlash(sbPtr);

			/* look after the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
			else { 
				Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
				Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
			}

			{
				VECTORCH shotvector;

				shotvector.vx=0;
				shotvector.vy=0;
				shotvector.vz=65535;
				RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);

				CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, 1,1);
				if (marineStatusPointer->clipammo>0) {
					marineStatusPointer->clipammo--;
				}
			}
		}
		keyframeflags>>=1;
	}
	/* ...and they are all identical. */

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireShotgun(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;
	int correctlyOrientated;
	int keyframeflags,a,b;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* Stabilise sequence. */
	if (marineStatusPointer->Target==NULL) {
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
			
			SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
		}
	} else {
		int offsetx,offsety,offsetz,offseta,angle1;
		VECTORCH *gunpos;
		int sequence=0;
		/* Pick a sequence based on angle. */

		if (marineStatusPointer->My_Elevation_Section) {
			gunpos=&marineStatusPointer->My_Elevation_Section->World_Offset;
		} else {
			gunpos=&sbPtr->DynPtr->Position;
		}
		/* Aim at Target. */
		
		offsetx=(marineStatusPointer->Target->DynPtr->Position.vx)-(gunpos->vx);
		offsety=(marineStatusPointer->Target->DynPtr->Position.vz)-(gunpos->vz);
		offseta=-((marineStatusPointer->Target->DynPtr->Position.vy)-(gunpos->vy));
	
		while( (offsetx>(ONE_FIXED>>2))
			||(offsety>(ONE_FIXED>>2))
			||(offseta>(ONE_FIXED>>2))
			||(offsetx<-(ONE_FIXED>>2))
			||(offsety<-(ONE_FIXED>>2))
			||(offseta<-(ONE_FIXED>>2))) {
		
			offsetx>>=1;
			offsety>>=1;
			offseta>>=1;
	
		}
	
		offsetz=SqRoot32((offsetx*offsetx)+(offsety*offsety));
		angle1=ArcTan(offseta,offsetz);
	
		if (angle1>=3072) angle1-=4096;
		if (angle1>=2048) angle1=angle1-3072;
		if (angle1>1024) angle1=2048-angle1;
	
		GLOBALASSERT(angle1>=-1024);
		GLOBALASSERT(angle1<=1024);

		sequence=0;
		/* Now correct for hysteresis... */
		{
			int highthreshold,topthreshold;

			if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_90) {
				highthreshold=90;
				topthreshold=(1024-300);
			} else if (marineStatusPointer->HModelController.Sub_Sequence==MSSS_WildFire_45) {
				highthreshold=90;
				topthreshold=(1024-256);
			} else {
				highthreshold=128;
				topthreshold=(1024-256);
			}

			if (angle1>topthreshold) {
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_90)) {
					sequence=2;
				} else {
					sequence=0;
				}
			} else if (angle1>highthreshold) { //Was 256!
				if (HModelSequence_Exists(&marineStatusPointer->HModelController,HMSQT_MarineStand,MSSS_WildFire_45)) {
					sequence=1;
				} else {
					sequence=0;
				}
			} else {
				sequence=0;
			}
		}

		switch (sequence) {
			case 2:
				/* Escape for civvie flamethrower here? */
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>3));
				}
				break;
			case 1:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>3));
				}
				break;
			case 0:
			default:
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)) {
					SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>3));
				}
				break;
		}
	}

	if (marineStatusPointer->clipammo==0) {
		return(SRC_Request_PanicReload);
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				if (marineStatusPointer->My_Weapon->id==MNPCW_MUnarmed) {
					Marine_AngryScream(sbPtr);
				}
			}
		}
	}

	if (marineStatusPointer->Target==NULL) {
		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				return(SRC_Request_Wait);
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);

		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

		if (correctlyOrientated) {
			if (MarineCanSeeTarget(sbPtr)) {
				/* Lost target... */
				marineStatusPointer->Target=NULL;
			}
		}	
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* Stop cues anyway? */
	if(marineStatusPointer->myGunFlash) {
		RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
		marineStatusPointer->myGunFlash = NULL;				
	}
	if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(marineStatusPointer->soundHandle);
		Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
	}

	/* Right.  Correctly orientated or not, we blaze away. */

	keyframeflags=marineStatusPointer->HModelController.keyframe_flags>>1;

	/* I know there are only three firing points... */
	for (a=0; a<3; a++) {
		if (keyframeflags&1) {

			/* look after the gun flash */
			if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
			else CreateMarineGunFlash(sbPtr);

			/* look after the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
			else { 
				Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
				Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
			}

			/* Now hit the target with a shotgun blast. */
	
			b=0;

			while (ShotgunBlast[b].vz>0) {
				VECTORCH world_vec;

				RotateAndCopyVector(&ShotgunBlast[b],&world_vec,&marineStatusPointer->My_Gunflash_Section->SecMat);
				CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&world_vec, marineStatusPointer->My_Weapon->Ammo_Type, 1,0);
	
				b++;
			}
			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}
		keyframeflags>>=1;
	}
	/* ...and they are all identical. */

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			/* Two consecutive tests... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				return(SRC_Request_Retreat);
			}
		}
	}

	return(SRC_No_Change);

}

static void Marine_EnterExtremePanicAnimation(STRATEGYBLOCK *sbPtr) {

	if ((FastRandom()&65535)<21845) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_90,-1,(ONE_FIXED>>1));
	} else if ((FastRandom()&65535)<21845) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_45,-1,(ONE_FIXED>>1));
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_WildFire_0,-1,(ONE_FIXED>>1));
	}
	
}

static void Marine_EnterLesserPanicAnimation(STRATEGYBLOCK *sbPtr) {

	if ((FastRandom()&65535)<32767) {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Panic_One,-1,(ONE_FIXED>>1));
	} else {
		SetMarineAnimationSequence_Null(sbPtr,HMSQT_MarineStand,MSSS_Panic_Two,-1,(ONE_FIXED>>1));
	}
		
}

static STATE_RETURN_CONDITION Execute_MNS_PanicFireUnarmed(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn;

	/* Also known as... gibber in terror. */

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine panic firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* Firstly, are we trying to retreat? */
	if (marineStatusPointer->internalState==1) {
		/* Wait for tweening to finish... */
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			/* Then the first incidentFlag we get, we're outta here. */
			if (marineStatusPointer->incidentFlag) {
				return(SRC_Request_Retreat);
			}
		}
		return(SRC_No_Change);
	}

	/* Stabilise sequence. */
	if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
		||((marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_0)
			&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_45)
			&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_WildFire_90))) {

		/* If we're not in one of the 'extreme panic' animations, are we in one of the others? */
		if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
			||((marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_One)
				&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_Two))) {
			/* No, we're not.  See which level of panic to go into... */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
				/* It's PaNiC tImE! */
				Marine_EnterExtremePanicAnimation(sbPtr);
			} else {
				/* Just look worried. */
				Marine_EnterLesserPanicAnimation(sbPtr);
			}
		} else {
			/* Yes, we are.  Is it getting worse? */
			int range;
			STRATEGYBLOCK *threat;
	
			if (marineStatusPointer->incidentFlag) {

				threat=Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr);
				if (threat) {
					range=VectorDistance(&sbPtr->DynPtr->Position,&threat->DynPtr->Position);
				} else {
					range=100000;
				}

				if ((MarineRetreatsInTheFaceOfDanger(sbPtr))||(range<3000)) {
					if (range<3000) {
						if (marineStatusPointer->Android==0) {
							marineStatusPointer->Courage-=5000;
						}
					}
					Marine_EnterExtremePanicAnimation(sbPtr);
				} else {
					/* Feeling really brave? */
					if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
						Marine_QueueGrimaceExpression(sbPtr);
					}
				}
			}
			/* Else remain as you were. */
		}
	} else {
		/* We are in extreme panic.  Is it getting better? */
		if (marineStatusPointer->incidentFlag) {
			if ((FastRandom()&65535)<13107) {
				if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
					if (!(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
						/* Two consecutive tests. */
						Marine_EnterLesserPanicAnimation(sbPtr);
					}
				}
			}
		}
		/* Enforce panic expression, too. */
		Marine_QueuePanicExpression(sbPtr);
	}

	if (marineStatusPointer->Target==NULL) {
		/* Civvies with courage 0 get a bonus to shut up... */
		if (marineStatusPointer->Courage==0) {
			/* No reason to block this for Androids. */
			marineStatusPointer->Courage=5000;
		}

		if (marineStatusPointer->incidentFlag) {
			/* Courage roll to stop. */
			if (MarineRetreatsInTheFaceOfDanger(sbPtr)==0) {
				/* Okay, calm down a bit. */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
	
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				/* Try to be suspicious? */
				marineStatusPointer->suspicious=MARINE_PANIC_TIME;
				marineStatusPointer->suspect_point=marineStatusPointer->weaponTarget;
				/* Set this to zero when you get a *new* suspicion. */
				marineStatusPointer->previous_suspicion=0;
				marineStatusPointer->using_squad_suspicion=0;
	
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||((marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_One)
						&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_Two))) {
					Marine_EnterLesserPanicAnimation(sbPtr);
					return(SRC_No_Change);
				} else {
					return(SRC_Request_Wait);
				}
			}
		}
	} else {
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);

		/* May as well keep an eye out. */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		#if 0
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		#endif	
	}

	/* Scream handling. */
	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		if (marineStatusPointer->incidentFlag) {
			if (Marine_HasHisMouthOpen(sbPtr)) {
				Marine_Sobbing(sbPtr);
			} else {
				/* Start yelling again? */
				if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
					Marine_QueuePanicExpression(sbPtr);
				}
			}
		}
	}

	if (NpcSquad.Squad_Suspicion==0) {
		if (marineStatusPointer->Target) {
			/* Here we must have a target.  Renew suspicion for new arrivals. */
			PointAlert(2,&marineStatusPointer->weaponTarget);
		} else {
			PointAlert(1,&sbPtr->DynPtr->Position);
		}
	}

	/* Wait for end of tweening before we proceed. */
	if (marineStatusPointer->HModelController.Tweening!=Controller_NoTweening) {
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_No_Change);
	}

	/* No functionality in here... */

	marineStatusPointer->stateTimer -= NormalFrameTime;

	/* Now, consider termination. */

	if (marineStatusPointer->incidentFlag) {
		if ((FastRandom()&65535)<13107) {
			if ((!(MarineRetreatsInTheFaceOfDanger(sbPtr)))
				||(!(MarineRetreatsInTheFaceOfDanger(sbPtr)))) {
				/* Two chances... */

				/* stop visual and audio cues: technically, we're not firing at this moment */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
		
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				if ((marineStatusPointer->HModelController.Sequence_Type!=HMSQT_MarineStand)
					||((marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_One)
						&&(marineStatusPointer->HModelController.Sub_Sequence!=MSSS_Panic_Two))) {
					Marine_EnterLesserPanicAnimation(sbPtr);
					marineStatusPointer->internalState=1;
					/* This is a bit pre-emptive, but never mind. */
					if (marineStatusPointer->Android==0) {
						marineStatusPointer->Courage-=5000;
					}
					return(SRC_No_Change);
				} else {
					return(SRC_Request_Retreat);
				}
			}
		}
	}

	return(SRC_No_Change);

}

void Marine_AssumeNeutralExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if ((marineStatusPointer->Expression>2)&&(marineStatusPointer->Expression<6)) {
		/* In a blink. */
		Marine_SwitchExpression(sbPtr,3);
		if (marineStatusPointer->Blink<0) {
			/* Dunno why, but you never know. */
			marineStatusPointer->Blink=0;
		}
	} else {
		/* Not in a blink. */
		Marine_SwitchExpression(sbPtr,0);
	}

}

void Marine_AssumeGrimaceExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if ((marineStatusPointer->Expression>2)&&(marineStatusPointer->Expression<6)) {
		/* In a blink. */
		Marine_SwitchExpression(sbPtr,4);
		if (marineStatusPointer->Blink<0) {
			/* Dunno why, but you never know. */
			marineStatusPointer->Blink=0;
		}
	} else {
		/* Not in a blink. */
		Marine_SwitchExpression(sbPtr,1);
	}

}

void Marine_AssumePanicExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if ((marineStatusPointer->Expression>2)&&(marineStatusPointer->Expression<6)) {
		/* In a blink. */
		Marine_SwitchExpression(sbPtr,5);
		if (marineStatusPointer->Blink<0) {
			/* Dunno why, but you never know. */
			marineStatusPointer->Blink=0;
		}
	} else {
		/* Not in a blink. */
		Marine_SwitchExpression(sbPtr,2);
	}

}

void Marine_AssumeWink1Expression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* Who cares about blinking? */
	Marine_SwitchExpression(sbPtr,6);
	marineStatusPointer->Blink=-1;

}

void Marine_AssumeWink2Expression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	/* Who cares about blinking? */
	Marine_SwitchExpression(sbPtr,7);
	marineStatusPointer->Blink=-1;

}

static STATE_RETURN_CONDITION Execute_MNS_PumpAction(STRATEGYBLOCK *sbPtr)
{

	MARINE_STATUS_BLOCK *marineStatusPointer;    
	int range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
    LOCALASSERT(marineStatusPointer);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if (!sbPtr->SBdptr) {
		/* We're far... do the timer! */
		ProveHModel_Far(&marineStatusPointer->HModelController,sbPtr);
	}

	if (!(HModelAnimation_IsFinished(&marineStatusPointer->HModelController))) {
		return(SRC_No_Change);
	}

	/* Exit procedure... same as firing was. */

	if (marineStatusPointer->Target==NULL) {
		return(SRC_Request_Wait);
	}

	range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
	
	/* State timer should be continuous from fire state. */
	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning firing at range %d.\n",range);
		#endif
		return(SRC_Request_Fire);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close firing at range %d.\n",range);
		#endif
		return(SRC_Request_Fire);
	}
	else
	{
		/* we are far enough away, so return to approach */
		
		if (marineStatusPointer->Android) {
			return(SRC_Request_Fire);
		} else {
			#if MARINE_STATE_PRINT
			textprint("Returning too far termination at range %d.\n",range);
			#endif
			return(SRC_Request_Approach);
		}
	}
	return(SRC_Request_Fire);

}

void Marine_CorpseSightingTest(STRATEGYBLOCK *corpse) {
	
	int a;
	STRATEGYBLOCK *candidate;
	MARINE_STATUS_BLOCK *marineStatusPointer;	

	/* This is called from CORPSE behaviour. */
	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		GLOBALASSERT(candidate);

		if (candidate->I_SBtype==I_BehaviourMarine) {
			/* Are you already suspicious? */
			marineStatusPointer = (MARINE_STATUS_BLOCK *)(candidate->SBdataptr);

			if (((marineStatusPointer->suspicious==0)||(marineStatusPointer->using_squad_suspicion))
				/* As if we'd care... */
				&&(marineStatusPointer->Target==NULL)
				/* To make the tests a bit rarer. */
				&&(marineStatusPointer->incidentFlag)) {
				/* Did you see that? */
				if (NPCCanSeeTarget(candidate,corpse,MARINE_NEAR_VIEW_WIDTH)) {
					/* Okay, react. */
					marineStatusPointer->suspicious=MARINE_PARANOIA_TIME;
					marineStatusPointer->suspect_point=corpse->DynPtr->Position;
					/* Set this to zero when you get a *new* suspicion. */
					marineStatusPointer->previous_suspicion=0;
					marineStatusPointer->using_squad_suspicion=0;
				}
			}
		}
	}
}

void Marine_MuteVoice(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if(marineStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
		/* Cut them off! */
		Sound_Stop(marineStatusPointer->soundHandle2);
	}

}

void Marine_OoophSound(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Oooph,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_SurpriseSound(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Surprise,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=5000;
		}
	}
	
	/* Open the mouth? */
	Marine_AssumePanicExpression(sbPtr);

}

void Marine_WoundedScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Pain,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_AcidScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Acid,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_BurningScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_OnFire,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
	/* Urgh, that was really grim. */

}

void Marine_DeathScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		/* I figure if you're screaming already, forget it. */
		PlayMarineScream(marineStatusPointer->Voice,SC_Death,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}

}

void Marine_ElectrocutionScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		/* I figure if you're screaming already, forget it. */
		PlayMarineScream(marineStatusPointer->Voice,SC_Electrocution,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}

}

void Marine_BurningDeathScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		/* I figure if you're screaming already, forget it. */
		PlayMarineScream(marineStatusPointer->Voice,SC_OnFire,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
	/* That too was really quite unpleasant. */

}

void Marine_AngryScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Angry,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_PanicScream(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Panic,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_Sobbing(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}
	
	/* This is getting quite upsetting... */

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Sobbing,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

void Marine_TauntShout(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Android) {
		return;
	}

	if (marineStatusPointer->soundHandle2==SOUND_NOACTIVEINDEX) {
		PlayMarineScream(marineStatusPointer->Voice,SC_Taunt,marineStatusPointer->VoicePitch,
			&marineStatusPointer->soundHandle2,&sbPtr->DynPtr->Position);
	}
}

int Marine_HasHisMouthOpen(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if ((marineStatusPointer->Expression==2)||(marineStatusPointer->Expression==5)) {
		/* yeah, okay, it's hard coded. */
		return(1);
	} else {
		return(0);
	}

}

void Marine_QueueNeutralExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->Target_Expression=0;

}

void Marine_QueueGrimaceExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->Target_Expression=1;

}

void Marine_QueuePanicExpression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->Target_Expression=2;

}

void Marine_QueueWink1Expression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->Target_Expression=6;

}

void Marine_QueueWink2Expression(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->Target_Expression=7;

}

void Marine_UpdateFace(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (marineStatusPointer->Expression!=marineStatusPointer->Target_Expression) {
		/* Right, consider this. */
		if (Marine_HasHisMouthOpen(sbPtr)) {
			/* Does the target expression also have it's mouth open? */
			if (!((marineStatusPointer->Target_Expression==2)||(marineStatusPointer->Target_Expression==5))) {
				/* Are we screaming? */
				if (marineStatusPointer->soundHandle2!=SOUND_NOACTIVEINDEX) {
					/* Can't close our mouth yet. */
					return;
				}
			}
		}
	}
	/* Also ignore if you're in the wink counterpart of the target. */
	switch (marineStatusPointer->Target_Expression) {
		case 0:
			if (marineStatusPointer->Expression==3) {
				return;
			}
			break;
		case 1:
			if (marineStatusPointer->Expression==4) {
				return;
			}
			break;
		case 2:
			if (marineStatusPointer->Expression==5) {
				return;
			}
			break;
		default:
			break;
	}
	/* Exit if current==target? */	
	if (marineStatusPointer->Expression==marineStatusPointer->Target_Expression) {
		return;
	}

	/* If we got here, it must be okay... but do it properly! */
	switch (marineStatusPointer->Target_Expression) {
		case 0:
		case 3:
			Marine_AssumeNeutralExpression(sbPtr);
			break;
		case 1:
		case 4:
			Marine_AssumeGrimaceExpression(sbPtr);
			break;
		case 2:
		case 5:
			Marine_AssumePanicExpression(sbPtr);
			break;
		case 6:
			Marine_AssumeWink1Expression(sbPtr);
			break;
		case 7:
			Marine_AssumeWink2Expression(sbPtr);
			break;
		default:
			GLOBALASSERT(0);
			break;
	}
}

static void	Marine_MirrorSuspectPoint(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	{
		VECTORCH offset;

		offset.vx=marineStatusPointer->suspect_point.vx-sbPtr->DynPtr->Position.vx;
		offset.vy=marineStatusPointer->suspect_point.vy-sbPtr->DynPtr->Position.vy;
		offset.vz=marineStatusPointer->suspect_point.vz-sbPtr->DynPtr->Position.vz;

		marineStatusPointer->suspect_point.vx=sbPtr->DynPtr->Position.vx-offset.vx;
		marineStatusPointer->suspect_point.vy=sbPtr->DynPtr->Position.vy-offset.vy;
		marineStatusPointer->suspect_point.vz=sbPtr->DynPtr->Position.vz-offset.vz;
	}

}



void SendRequestToMarine(STRATEGYBLOCK* sbPtr,BOOL state,int extended_data)
{
	//handle RequestState (from switches etc.)
	
	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if(extended_data & 1)
	{
		//change mission type 
		MARINE_MISSION new_mission=MM_Wait_Then_Wander;
		int new_path=-1;
		int new_stepnumber=-1;
		//state must be 1
		if(!state) return;

		//can't change mission type if non-combatant or if on fire
		if(marineStatusPointer->Mission==MM_NonCom ||
		   marineStatusPointer->Mission==MM_RunAroundOnFire)
			return;

		switch((extended_data>>7)&0xff)
		{
			case 0:
				new_mission=MM_Wait_Then_Wander;
				break;
			case 1:
				new_mission=MM_Wander;
				break;
			case 2:
				new_mission=MM_Guard;
				//need to cacluculate mission module from 'myspot' guard location
				{
					MODULE* module=ModuleFromPosition(&marineStatusPointer->my_spot,0);
					GLOBALASSERT(module);
					marineStatusPointer->missionmodule=module->m_aimodule;
				}
				break;
			case 3:
				new_mission=MM_LocalGuard;
				break;
			case 4:
				new_mission=MM_Pathfinder;
				break;
			
		}
		if(new_mission==MM_Pathfinder)
		{
			//find the path and stepnumber
			new_path=(extended_data>>15)&0xff;
			new_stepnumber=GetClosestStepInPath(new_path,sbPtr->containingModule);
		}
				
		marineStatusPointer->path=new_path;
		marineStatusPointer->stepnumber=new_stepnumber;
		InitMission(sbPtr,new_mission);

	}

}

static void	Marine_ConsiderFallingDamage(STRATEGYBLOCK *sbPtr) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	if (sbPtr->DynPtr->IsInContactWithFloor) {
		if (marineStatusPointer->lastframe_fallingspeed>0) {

			if (marineStatusPointer->lastframe_fallingspeed>Marine_Terminal_Velocity) {
				VECTORCH point,point2;
				enum PARTICLE_ID blood_type;

				/* Kill marine... */
				CauseDamageToObject(sbPtr,&FallingDamage, (50*ONE_FIXED), NULL);
				/* Experiment with blood. */
				GetTargetingPointOfObject_Far(sbPtr,&point);
				point2=sbPtr->DynPtr->Position;
				point2.vy-=200;
				blood_type=GetBloodType(sbPtr);
				MakeBloodExplosion(&point2,127,&point,50,blood_type);
				/* Crunching sound optional. */
			}
		}
		marineStatusPointer->lastframe_fallingspeed=0;
	} else {
		/* Compute falling speed. */
		if (marineStatusPointer->lastframe_fallingspeed>=0) {
			int offset;

			offset=sbPtr->DynPtr->Position.vy-sbPtr->DynPtr->PrevPosition.vy;

			marineStatusPointer->lastframe_fallingspeed=DIV_FIXED(offset,NormalFrameTime);
			if (marineStatusPointer->lastframe_fallingspeed<0) {
				marineStatusPointer->lastframe_fallingspeed=0;
			}
		} else {
			marineStatusPointer->lastframe_fallingspeed=0;
		}
	}
}

static STATE_RETURN_CONDITION Execute_MNS_DischargeSmartgun(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	/* Alternative smartgun function? */

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->HModelController.Playing=1;

	#if MARINE_STATE_PRINT
	textprint("Marine firing... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state*/
	if (marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize) {
		/* Keep firing! */
	} else {
		if(!MarineCanSeeTarget(sbPtr))
		{

			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}

			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			marineStatusPointer->lastroundhit=0;
			marineStatusPointer->lasthitsection=NULL;
			return(SRC_Request_Wait);
		}
	}

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		return(SRC_Request_Reload);
	}		


	if (marineStatusPointer->Target) {
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* Here we must have a target.  Renew suspicion for new arrivals. */
		if (NpcSquad.Squad_Suspicion==0) {
			PointAlert(2,&marineStatusPointer->weaponTarget);
		}
	}
	/* Otherwise, stay facing the same way. */

	/* orientate to firing point first */
	orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
	orientationDirn.vy = 0;
	orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
	correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

	/* I have a cunning plan... */
	{
		DELTA_CONTROLLER *delta;

		delta=Get_Delta_Sequence(&marineStatusPointer->HModelController,"HitDelta");
		if (delta) {
			if (!(DeltaAnimation_IsFinished(delta))) {
				correctlyOrientated=0;
			}
		}
	}
	/* I have another cunning plan... */
	if ((marineStatusPointer->volleySize>0)&&
		(marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		correctlyOrientated=1;
	}


	/* we are not correctly orientated to the target: this could happen because we have
	just entered this state, or the target has moved during firing*/
	if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening!=Controller_NoTweening)) {

		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}

		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#if MARINE_STATE_PRINT
		textprint("Turning to face.\n");
		#endif
		marineStatusPointer->lastroundhit=0;
		marineStatusPointer->lasthitsection=NULL;
		return(SRC_No_Change);
	}

	if (marineStatusPointer->Target) {
		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
	  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
	
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));
	}
	
	#if 0
	DischargeLOSWeapon_Core(sbPtr);
	#else
	{
		int volleytime,volleyrounds;

		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) {
			MaintainMarineGunFlash(sbPtr);
		} else {
			CreateMarineGunFlash(sbPtr);
		}

		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}

		marineStatusPointer->stateTimer -= NormalFrameTime;
			
		/* Volleysize is now rounds fired this state. */

		volleytime=marineStatusPointer->My_Weapon->FiringTime-marineStatusPointer->stateTimer;
		/* It was that or reverse the state timer for this state. */
		volleyrounds=MUL_FIXED(volleytime,marineStatusPointer->My_Weapon->FiringRate);
		volleyrounds>>=ONE_FIXED_SHIFT;

		volleyrounds-=marineStatusPointer->volleySize;
		marineStatusPointer->volleySize+=volleyrounds;

		LOCALASSERT(volleyrounds>=0);

		if (marineStatusPointer->clipammo!=-1) {
			/* We're counting ammo. */
			if (volleyrounds>marineStatusPointer->clipammo) {
				volleyrounds=marineStatusPointer->clipammo;
			}
			marineStatusPointer->clipammo-=volleyrounds;
			LOCALASSERT(marineStatusPointer->clipammo>=0);
		}

		marineStatusPointer->roundsForThisTarget+=volleyrounds;

		/* Now hit the target with volleyrounds bullets. */

		{
			VECTORCH shotvector;

			shotvector.vx=0;
			shotvector.vy=0;
			shotvector.vz=65535;
			RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);

			CastLOSProjectile(sbPtr,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, volleyrounds,1);
		}
	}
	#endif

	if (marineStatusPointer->Target==NULL) {
		/* Getting out of here! */
		return(SRC_No_Change);
	}

	/* Did we get him? */
	if ((NPC_IsDead(marineStatusPointer->Target))
		&&(marineStatusPointer->volleySize>=marineStatusPointer->My_Weapon->MinimumBurstSize)) {
		if (MarineRetreatsInTheFaceOfDanger(sbPtr)) {
			if (Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr)==NULL) {
				/* Huzzah! */
				if(marineStatusPointer->myGunFlash) 
				{
					RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
					marineStatusPointer->myGunFlash = NULL;				
				}
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(marineStatusPointer->soundHandle);
					Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
				}
				marineStatusPointer->lastroundhit=0;
				marineStatusPointer->lasthitsection=NULL;
				return(SRC_Request_Taunt);
			}
		}
	}
	
	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if(range < MARINE_CLOSE_APPROACH_DISTANCE)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		#if 1
		/* stop visual and audio cues: technically, we're not firing at this moment */
		if(marineStatusPointer->myGunFlash) 
		{
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;				
		}
		#endif
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		if (marineStatusPointer->Android==0) {
			marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_Request_Fire);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if 1
		/* ... and remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		#endif
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

int MakeModifiedTargetNum(int targetnum,int dist) {

	/* Adjust targetnum by range? */
	if (dist>10000) {
		int factor,tdist;

		tdist=dist-10000;
		if (tdist>100000) {
			tdist=100000;
		}
		factor=DIV_FIXED(tdist,100000);
		factor=ONE_FIXED-factor;
		if (factor<(ONE_FIXED>>3)) {
			factor=(ONE_FIXED>>3);
		}
		
		return(MUL_FIXED(factor,targetnum));
	}
	return(targetnum);
}





/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct marine_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

 //from marine status block
 	signed int health;
 	signed int volleySize;
 	signed int primaryWeaponDamage;
 
 	MARINE_BHSTATE behaviourState;
 	MARINE_BHSTATE lastState;

	MARINE_MISSION Mission;

	VECTORCH my_spot;
	VECTORCH my_facing_point;

	/* Movement data. */
	signed int nearSpeed;
	int acceleration;
	int speedConstant;
	int accelerationConstant;
	/* Sense data */
	int mtracker_timer;
	VECTORCH suspect_point;
	int suspicious;
	int previous_suspicion;
	int using_squad_suspicion;
	int sawlastframe;
	int gotapoint;
	int lastframe_fallingspeed;
	/* Pathfinder parameters */
	int path;
	int stepnumber;
	/* Pathfinder parameters */
	int stateTimer;
	int internalState;

	VECTORCH weaponTarget;		/* position for firing weapon at */

	NPC_OBSTRUCTIONREPORT obstruction;
	NPC_WANDERDATA wanderData;

	int IAmCrouched;

	int lastroundhit;
		
	int GibbFactor;
	int Wounds;

	int incidentFlag;
	int incidentTimer;

	int weapon_variable;
	int weapon_variable2;
	int clipammo;
	int roundsForThisTarget;

	int Female;
	int Android;
	int Skill;
	int Courage;
	int Voice;
	int VoicePitch;
		
	int FiringAnim;
	int Expression;
	int Target_Expression;
	int Blink;
	int SpotFlag;


//annoying pointer related things
	int lastmodule_index;
	int destinationmodule_index;
	int missionmodule_index;
	int fearmodule_index;


	char Target_SBname[SB_NAME_LENGTH];
	char Generator_SBname[SB_NAME_LENGTH];

	int weapon_id;

	//strategyblock stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}MARINE_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV marineStatusPointer


void LoadStrategy_Marine(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	MARINE_STATUS_BLOCK* marineStatusPointer;
	MARINE_SAVE_BLOCK* block = (MARINE_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);

	if(sbPtr)
	{
		//make sure the strategy found is of the right type
		if(sbPtr->I_SBtype != I_BehaviourMarine) return;
	}
	else
	{
		//we will have to generate an alien then
		TOOLS_DATA_MARINE tdm;

		//make sure the marine is in a module 
		if(!ModuleFromPosition(&block->dynamics.Position,NULL)) return;

		sbPtr = CreateActiveStrategyBlock();
		if(!sbPtr) return;

		sbPtr->I_SBtype = I_BehaviourMarine;
		sbPtr->shapeIndex = 0;
		sbPtr->maintainVisibility = 1;
		COPY_NAME(sbPtr->SBname,block->header.SBname);

		//create using a fake tools data
		memset(&tdm,0,sizeof(TOOLS_DATA_MARINE));

		tdm.position = block->dynamics.Position;
		COPY_NAME(tdm.nameID,block->header.SBname);
		tdm. marine_type = block->weapon_id;
 
 		EnableBehaviourType(sbPtr,I_BehaviourMarine , &tdm );

 	}


	marineStatusPointer = (MARINE_STATUS_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff

 	COPYELEMENT_LOAD(health)
 	COPYELEMENT_LOAD(volleySize)
 	COPYELEMENT_LOAD(primaryWeaponDamage)
 	COPYELEMENT_LOAD(behaviourState)
 	COPYELEMENT_LOAD(lastState)
	COPYELEMENT_LOAD(Mission)
	COPYELEMENT_LOAD(my_spot)
	COPYELEMENT_LOAD(my_facing_point)
	COPYELEMENT_LOAD(nearSpeed)
	COPYELEMENT_LOAD(acceleration)
	COPYELEMENT_LOAD(speedConstant)
	COPYELEMENT_LOAD(accelerationConstant)
	COPYELEMENT_LOAD(mtracker_timer)
	COPYELEMENT_LOAD(suspect_point)
	COPYELEMENT_LOAD(suspicious)
	COPYELEMENT_LOAD(previous_suspicion)
	COPYELEMENT_LOAD(using_squad_suspicion)
	COPYELEMENT_LOAD(sawlastframe)
	COPYELEMENT_LOAD(gotapoint)
	COPYELEMENT_LOAD(lastframe_fallingspeed)
	COPYELEMENT_LOAD(path)
	COPYELEMENT_LOAD(stepnumber)
	COPYELEMENT_LOAD(stateTimer)
	COPYELEMENT_LOAD(internalState)
	COPYELEMENT_LOAD(weaponTarget)		/* position for firing weapon at */
	COPYELEMENT_LOAD(obstruction)
	COPYELEMENT_LOAD(wanderData)
	COPYELEMENT_LOAD(IAmCrouched)
	COPYELEMENT_LOAD(lastroundhit)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(Wounds)

	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)

	COPYELEMENT_LOAD(weapon_variable)
	COPYELEMENT_LOAD(weapon_variable2)
	COPYELEMENT_LOAD(clipammo)
	COPYELEMENT_LOAD(roundsForThisTarget)

	COPYELEMENT_LOAD(Female)
	COPYELEMENT_LOAD(Android)
	COPYELEMENT_LOAD(Skill)
	COPYELEMENT_LOAD(Courage)
	COPYELEMENT_LOAD(Voice)
	COPYELEMENT_LOAD(VoicePitch)
	
	COPYELEMENT_LOAD(FiringAnim)
	COPYELEMENT_LOAD(Expression)
	COPYELEMENT_LOAD(Target_Expression)
	COPYELEMENT_LOAD(Blink)
	COPYELEMENT_LOAD(SpotFlag)


	//load ai module pointers
	marineStatusPointer->lastmodule = GetPointerFromAIModuleIndex(block->lastmodule_index);
	marineStatusPointer->destinationmodule = GetPointerFromAIModuleIndex(block->destinationmodule_index);
	marineStatusPointer->missionmodule = GetPointerFromAIModuleIndex(block->missionmodule_index);
	marineStatusPointer->fearmodule = GetPointerFromAIModuleIndex(block->fearmodule_index);

	//load target
	COPY_NAME(marineStatusPointer->Target_SBname,block->Target_SBname);
	marineStatusPointer->Target = FindSBWithName(marineStatusPointer->Target_SBname);


	//find the marine's generator
	marineStatusPointer->generator_sbptr = FindSBWithName(block->Generator_SBname);

	//get marine's weapon
	marineStatusPointer->My_Weapon = GetThisNPCMarineWeapon(block->weapon_id);

	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&marineStatusPointer->HModelController);
		}
	}

	//get section data pointers
	if(marineStatusPointer->My_Weapon)
	{
		marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->GunflashName);
		marineStatusPointer->My_Elevation_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,marineStatusPointer->My_Weapon->ElevationSection);
	}
	
	Load_SoundState(&marineStatusPointer->soundHandle);
	Load_SoundState(&marineStatusPointer->soundHandle2);
}

void SaveStrategy_Marine(STRATEGYBLOCK* sbPtr)
{
	MARINE_SAVE_BLOCK *block;
	MARINE_STATUS_BLOCK* marineStatusPointer;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff

 	COPYELEMENT_SAVE(health)
 	COPYELEMENT_SAVE(volleySize)
 	COPYELEMENT_SAVE(primaryWeaponDamage)
 	COPYELEMENT_SAVE(behaviourState)
 	COPYELEMENT_SAVE(lastState)
	COPYELEMENT_SAVE(Mission)
	COPYELEMENT_SAVE(my_spot)
	COPYELEMENT_SAVE(my_facing_point)
	COPYELEMENT_SAVE(nearSpeed)
	COPYELEMENT_SAVE(acceleration)
	COPYELEMENT_SAVE(speedConstant)
	COPYELEMENT_SAVE(accelerationConstant)
	COPYELEMENT_SAVE(mtracker_timer)
	COPYELEMENT_SAVE(suspect_point)
	COPYELEMENT_SAVE(suspicious)
	COPYELEMENT_SAVE(previous_suspicion)
	COPYELEMENT_SAVE(using_squad_suspicion)
	COPYELEMENT_SAVE(sawlastframe)
	COPYELEMENT_SAVE(gotapoint)
	COPYELEMENT_SAVE(lastframe_fallingspeed)
	COPYELEMENT_SAVE(path)
	COPYELEMENT_SAVE(stepnumber)
	COPYELEMENT_SAVE(stateTimer)
	COPYELEMENT_SAVE(internalState)
	COPYELEMENT_SAVE(weaponTarget)		/* position for firing weapon at */
	COPYELEMENT_SAVE(obstruction)
	COPYELEMENT_SAVE(wanderData)
	COPYELEMENT_SAVE(IAmCrouched)
	COPYELEMENT_SAVE(lastroundhit)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(Wounds)

	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)

	COPYELEMENT_SAVE(weapon_variable)
	COPYELEMENT_SAVE(weapon_variable2)
	COPYELEMENT_SAVE(clipammo)
	COPYELEMENT_SAVE(roundsForThisTarget)

	COPYELEMENT_SAVE(Female)
	COPYELEMENT_SAVE(Android)
	COPYELEMENT_SAVE(Skill)
	COPYELEMENT_SAVE(Courage)
	COPYELEMENT_SAVE(Voice)
	COPYELEMENT_SAVE(VoicePitch)
	
	COPYELEMENT_SAVE(FiringAnim)
	COPYELEMENT_SAVE(Expression)
	COPYELEMENT_SAVE(Target_Expression)
	COPYELEMENT_SAVE(Blink)
	COPYELEMENT_SAVE(SpotFlag)

	//save ai module pointers
	block->lastmodule_index = GetIndexFromAIModulePointer(marineStatusPointer->lastmodule);
	block->destinationmodule_index = GetIndexFromAIModulePointer(marineStatusPointer->destinationmodule);
	block->missionmodule_index = GetIndexFromAIModulePointer(marineStatusPointer->missionmodule);
	block->fearmodule_index = GetIndexFromAIModulePointer(marineStatusPointer->fearmodule);

	//save target
	COPY_NAME(block->Target_SBname,marineStatusPointer->Target_SBname);
	
	//save the marine's generator name
	if(marineStatusPointer->generator_sbptr)
	{
		COPY_NAME(block->Generator_SBname,marineStatusPointer->generator_sbptr->SBname);
	}
	else
	{
		COPY_NAME(block->Generator_SBname,Null_Name);
	}

	//save marine's weapon
	if(marineStatusPointer->My_Weapon) 
		block->weapon_id = marineStatusPointer->My_Weapon->id;
	else 
		block->weapon_id = -1;
	
	//copy strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&marineStatusPointer->HModelController);

	Save_SoundState(&marineStatusPointer->soundHandle);
	Save_SoundState(&marineStatusPointer->soundHandle2);

}



/*----------------------------**
** And now the squad state... **
**----------------------------*/


typedef struct marine_squad_save_block
{
	SAVE_BLOCK_HEADER header;

	int alertStatus;
	int responseLevel;
	int alertPriority;

	int Squad_Suspicion;
	VECTORCH squad_suspect_point;
	
	/* Now some stats. */
	int RespondingMarines;
	int Alt_RespondingMarines;

	int NearUnpanickedMarines;
	int Alt_NearUnpanickedMarines;

	int NearPanickedMarines;
	int Alt_NearPanickedMarines;

	int NearBurningMarines;
	int Alt_NearBurningMarines;
	
	int Squad_Delta_Morale;
	int Nextframe_Squad_Delta_Morale;

	int alertZone_index;
	
}MARINE_SQUAD_SAVE_BLOCK;

#undef SAVELOAD_BEHAV
//defines for load/save macros
#define SAVELOAD_BEHAV (&NpcSquad)


void LoadMarineSquadState(SAVE_BLOCK_HEADER* header)
{
	MARINE_SQUAD_SAVE_BLOCK *block = (MARINE_SQUAD_SAVE_BLOCK*) header;
	
	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//copy stuff

	COPYELEMENT_LOAD(alertStatus)
	COPYELEMENT_LOAD(responseLevel)
	COPYELEMENT_LOAD(alertPriority)
	COPYELEMENT_LOAD(Squad_Suspicion)
	COPYELEMENT_LOAD(squad_suspect_point)
	COPYELEMENT_LOAD(RespondingMarines)
	COPYELEMENT_LOAD(Alt_RespondingMarines)
	COPYELEMENT_LOAD(NearUnpanickedMarines)
	COPYELEMENT_LOAD(Alt_NearUnpanickedMarines)
	COPYELEMENT_LOAD(NearPanickedMarines)
	COPYELEMENT_LOAD(Alt_NearPanickedMarines)
	COPYELEMENT_LOAD(NearBurningMarines)
	COPYELEMENT_LOAD(Alt_NearBurningMarines)
	COPYELEMENT_LOAD(Squad_Delta_Morale)
	COPYELEMENT_LOAD(Nextframe_Squad_Delta_Morale)

	//and an aimodule
	NpcSquad.alertZone = GetPointerFromAIModuleIndex(block->alertZone_index);

}

void SaveMarineSquadState()
{
	MARINE_SQUAD_SAVE_BLOCK * block;

	GET_SAVE_BLOCK_POINTER(block);

	//fill in the header
	block->header.type = SaveBlock_MarineSquad;
	block->header.size = sizeof(*block);

	//copy stuff

	COPYELEMENT_SAVE(alertStatus)
	COPYELEMENT_SAVE(responseLevel)
	COPYELEMENT_SAVE(alertPriority)
	COPYELEMENT_SAVE(Squad_Suspicion)
	COPYELEMENT_SAVE(squad_suspect_point)
	COPYELEMENT_SAVE(RespondingMarines)
	COPYELEMENT_SAVE(Alt_RespondingMarines)
	COPYELEMENT_SAVE(NearUnpanickedMarines)
	COPYELEMENT_SAVE(Alt_NearUnpanickedMarines)
	COPYELEMENT_SAVE(NearPanickedMarines)
	COPYELEMENT_SAVE(Alt_NearPanickedMarines)
	COPYELEMENT_SAVE(NearBurningMarines)
	COPYELEMENT_SAVE(Alt_NearBurningMarines)
	COPYELEMENT_SAVE(Squad_Delta_Morale)
	COPYELEMENT_SAVE(Nextframe_Squad_Delta_Morale)

	//and an aimodule
	block->alertZone_index = GetIndexFromAIModulePointer(NpcSquad.alertZone);
}

static STATE_RETURN_CONDITION Execute_MNS_DischargeTwoPistols(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;
	int mod,hitroll;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	LOCALASSERT(marineStatusPointer->My_Weapon->id==MNPCW_TwoPistols);

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	if(!MarineCanSeeTarget(sbPtr))
	{
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
		return(SRC_Request_Wait);
	}

	if (marineStatusPointer->clipammo==0) {
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		return(SRC_Request_Reload);
	}
	
	#if MARINE_STATE_PRINT
	textprint("Firing pistol... ");
	#endif

	/* Here we must have a target.  Renew suspicion for new arrivals. */
	if (NpcSquad.Squad_Suspicion==0) {
		PointAlert(2,&marineStatusPointer->weaponTarget);
	}

	/* first of all, validate this state: if the target suddenly becomes cloaked, then
	we should switch immediately to wait state.*/

	//if ((marineStatusPointer->HModelController.keyframe_flags)
	//	||(marineStatusPointer->HModelController.Playing==0)) {

	relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
	relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
	relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
		  
	relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
	relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
	relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
	
	range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

	if (marineStatusPointer->volleySize==0) {
		
		//marineStatusPointer->HModelController.Playing=0;
		//marineStatusPointer->HModelController.sequence_timer=0;

		/* Only terminate if you haven't fired yet... */
		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
			#endif
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}
		
		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		
		/* we are not correctly orientated to the target: this could happen because we have
		just entered this state, or the target has moved during firing*/
		//if((!correctlyOrientated)||(marineStatusPointer->HModelController.Tweening==Controller_Tweening)) {
		if (!correctlyOrientated) {

			#if 1
			/* stop visual and audio cues: technically, we're not firing at this moment */
			if(marineStatusPointer->myGunFlash) 
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;				
			}
			#endif
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			#if MARINE_STATE_PRINT
			textprint("Turning to face.\n");
			#endif
			return(SRC_No_Change);
		}
		
		/* If you are correctly oriented, you can now fire! */

		marineStatusPointer->volleySize++;
		marineStatusPointer->HModelController.Playing=1;
		if (marineStatusPointer->clipammo>0) {
			marineStatusPointer->clipammo--;
		}
		marineStatusPointer->roundsForThisTarget++;

		/* Alternate gun flash. */
		if (strcmp(marineStatusPointer->My_Gunflash_Section->sempai->Section_Name,"dum flash")==0) {
			marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,"dum L flash");
		} else {
			marineStatusPointer->My_Gunflash_Section=GetThisSectionData(marineStatusPointer->HModelController.section_data,"dum flash");
		}
		
		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) MaintainMarineGunFlash(sbPtr);
		else CreateMarineGunFlash(sbPtr);
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		else 
		{ 
			Sound_Play(marineStatusPointer->My_Weapon->StartSound,"d",&(sbPtr->DynPtr->Position));
			Sound_Play(marineStatusPointer->My_Weapon->LoopSound,"del",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
		}
	
		/* Now hit the target with one bullet. */
	
		mod=SpeedRangeMods(&relPos,&relPos2);
	
		hitroll=marineStatusPointer->Skill; /* Marine skill... */
		if (marineStatusPointer->Target==Player->ObStrategyBlock) {
			PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
			LOCALASSERT(playerStatusPtr);
			if ( (AvP.PlayerType==I_Alien)
				||((AvP.PlayerType==I_Predator)&&(playerStatusPtr->cloakOn==1))) {
				/* Vs the player, lighting effects on aliens and cloaked preds. */
				hitroll=MUL_FIXED(hitroll,((CurrentLightAtPlayer>>1)+32767));
			}
		}
		hitroll-=mod;
		hitroll+=marineStatusPointer->My_Weapon->Accuracy;
	
		{
							
			/* Handle Damage. */
			if ((FastRandom()&65535)<hitroll) {
				/* DO DAMAGE TO TARGET HERE */
				
				VECTORCH rel_pos,attack_dir;
				int dist;
				VECTORCH shotvector;

				shotvector.vx=0;
				shotvector.vy=0;
				shotvector.vz=65535;
				RotateVector(&shotvector,&marineStatusPointer->My_Gunflash_Section->SecMat);

				rel_pos.vx=marineStatusPointer->Target->DynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
				rel_pos.vy=marineStatusPointer->Target->DynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
				rel_pos.vz=marineStatusPointer->Target->DynPtr->Position.vz-sbPtr->DynPtr->Position.vz;

				dist=Approximate3dMagnitude(&rel_pos);

				if (VerifyHitShot(sbPtr,marineStatusPointer->Target,&marineStatusPointer->My_Gunflash_Section->World_Offset,&shotvector, marineStatusPointer->My_Weapon->Ammo_Type, 1,dist)) {

					GetDirectionOfAttack(marineStatusPointer->Target,&rel_pos,&attack_dir);
					/* Get hit location? */
		
					marineStatusPointer->lasthitsection=HitLocationRoll(marineStatusPointer->Target,sbPtr);
				
					if (marineStatusPointer->lasthitsection) {
						CauseDamageToHModel(marineStatusPointer->lasthitsection->my_controller, marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED, marineStatusPointer->lasthitsection,&attack_dir,NULL,0);
					} else {
						CauseDamageToObject(marineStatusPointer->Target,&TemplateAmmo[marineStatusPointer->My_Weapon->Ammo_Type].MaxDamage[AvP.Difficulty], ONE_FIXED,&attack_dir);
					}
				}
			} else {
				GLOBALASSERT(marineStatusPointer->My_Gunflash_Section);
				ProjectNPCShot(sbPtr, marineStatusPointer->Target, &marineStatusPointer->My_Gunflash_Section->World_Offset,&marineStatusPointer->My_Gunflash_Section->SecMat, marineStatusPointer->My_Weapon->Ammo_Type, 1);
			}

			/* Did we get him? */
			if ((NPC_IsDead(marineStatusPointer->Target))&&(marineStatusPointer->My_Weapon->ARealMarine)) {
				/* Only real marines taunt. */
				if ((marineStatusPointer->roundsForThisTarget==1)||(MarineRetreatsInTheFaceOfDanger(sbPtr))) {
					if (Marine_GetNewTarget(&sbPtr->DynPtr->Position,sbPtr)==NULL) {
						/* Huzzah! */
						if(marineStatusPointer->myGunFlash) 
						{
							RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
							marineStatusPointer->myGunFlash = NULL;				
						}
						if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
							Sound_Stop(marineStatusPointer->soundHandle);
							Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
						}
						marineStatusPointer->lastroundhit=0;
						marineStatusPointer->lasthitsection=NULL;
						return(SRC_Request_Taunt);
					}
				}
			}
		}
	} else {	
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}
	}

	marineStatusPointer->stateTimer -= (NormalFrameTime);

	/* You must have fired already. */

	if(marineStatusPointer->stateTimer > 0)	{
		#if MARINE_STATE_PRINT
		textprint("Returning continue at range %d.\n",range);
		#endif
		return(SRC_No_Change);
	}
	
	if ((range < MARINE_CLOSE_APPROACH_DISTANCE)||(marineStatusPointer->Android)
		//||(marineStatusPointer->volleySize<marineStatusPointer->My_Weapon->MinimumBurstSize))
		)
	{
		/* renew firing, as we are still too close to approach */ 
		marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
		marineStatusPointer->volleySize = 0;
		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		#if MARINE_STATE_PRINT
		textprint("Returning too close renewal at range %d.\n",range);
		#endif
		if (marineStatusPointer->Android==0) {
		//	marineStatusPointer->Courage-=(ONE_FIXED>>3);
		}
		return(SRC_No_Change);
	}
	else
	{
		/* we are far enough away, so return to approach */

		#if MARINE_STATE_PRINT
		textprint("Returning too far termination at range %d.\n",range);
		#endif
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		marineStatusPointer->volleySize = 0;
		return(SRC_Request_Approach);
	}
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_DischargeSkeeter(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	VECTORCH orientationDirn,relPos,relPos2;
	int correctlyOrientated,range;

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	#if MARINE_STATE_PRINT
	textprint("Firing Skeeter... ");
	#endif

	/* zero velocity */
	LOCALASSERT(sbPtr->DynPtr);
	sbPtr->DynPtr->LinVelocity.vx = 0;
	sbPtr->DynPtr->LinVelocity.vy = 0;
	sbPtr->DynPtr->LinVelocity.vz = 0;

	/* If a skeeter launcher has started firing, he can't stop! */

	if (marineStatusPointer->clipammo==0) {
		/* Reload here. */
		return(SRC_Request_Reload);
	}		

	if (marineStatusPointer->Target) {
		/* Here we must have a target.  Renew suspicion for new arrivals. */
		if (NpcSquad.Squad_Suspicion==0) {
			PointAlert(2,&marineStatusPointer->weaponTarget);
		}
	}

	if (marineStatusPointer->stateTimer==marineStatusPointer->My_Weapon->FiringTime) {
	
		/* First cycle. */	
		if (marineStatusPointer->HModelController.Tweening==Controller_NoTweening) {
			marineStatusPointer->HModelController.Playing=0;
		}

		/* Only terminate if you haven't begun firing yet... */
		
		if (marineStatusPointer->Target==NULL) {
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		if(!MarineCanSeeTarget(sbPtr))
		{
			#if 1
			/* ... and remove the gunflash */
			if(marineStatusPointer->myGunFlash)
			{
				RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
				marineStatusPointer->myGunFlash = NULL;		
			}
			#endif
		
			/* .... and stop the sound */
			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);		
				Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
			}
			
			#if MARINE_STATE_PRINT
			textprint("Returning no target.\n");
			#endif
			return(SRC_Request_Wait);
		}

		GLOBALASSERT(marineStatusPointer->Target);
		NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
		/* Fix weapon target! */
		if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
			marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
			marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
				marineStatusPointer->My_Weapon->TargetCallibrationShift);
		}

		/* orientate to firing point first */
		orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
		orientationDirn.vy = 0;
		orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
		correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);

		/* Consider range. */

		relPos.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(sbPtr->DynPtr->Position.vx);
		relPos.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(sbPtr->DynPtr->Position.vy);
		relPos.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(sbPtr->DynPtr->Position.vz);
			  
		relPos2.vx=(marineStatusPointer->Target->DynPtr->Position.vx)-(marineStatusPointer->Target->DynPtr->PrevPosition.vx);
		relPos2.vy=(marineStatusPointer->Target->DynPtr->Position.vy)-(marineStatusPointer->Target->DynPtr->PrevPosition.vy);
		relPos2.vz=(marineStatusPointer->Target->DynPtr->Position.vz)-(marineStatusPointer->Target->DynPtr->PrevPosition.vz);
		
		range=VectorDistance((&marineStatusPointer->Target->DynPtr->Position),(&sbPtr->DynPtr->Position));

		/* Are they, by some chance, really close? */
		if (range<marineStatusPointer->My_Weapon->MinRange) {
			/* Even a complete idiot wouldn't fire this thing by reflex. */
			return(SRC_Request_PullPistol);
		}
	
		if (correctlyOrientated) {
			/* If you are correctly oriented, you can now fire! */
			marineStatusPointer->HModelController.Playing=1;
			marineStatusPointer->HModelController.sequence_timer=0;
			marineStatusPointer->stateTimer-=NormalFrameTime;

			if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
				Sound_Stop(marineStatusPointer->soundHandle);
			}
			if(marineStatusPointer->soundHandle==SOUND_NOACTIVEINDEX) {
				Sound_Play(marineStatusPointer->My_Weapon->StartSound,"de",&(sbPtr->DynPtr->Position),&(marineStatusPointer->soundHandle));
			}
		}
		return(SRC_No_Change);
	} else if (marineStatusPointer->stateTimer>0) {
		
		/* We must be in the build up. */
		if (marineStatusPointer->Target) {
			NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
			/* Fix weapon target! */
			if (marineStatusPointer->My_Weapon->TargetCallibrationShift) {
				marineStatusPointer->weaponTarget.vx-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat11,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
				marineStatusPointer->weaponTarget.vy-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat12,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
				marineStatusPointer->weaponTarget.vz-=MUL_FIXED(sbPtr->DynPtr->OrientMat.mat13,
					marineStatusPointer->My_Weapon->TargetCallibrationShift);
			}

			/* orientate to firing point first */
			orientationDirn.vx =  marineStatusPointer->weaponTarget.vx - sbPtr->DynPtr->Position.vx;
			orientationDirn.vy = 0;
			orientationDirn.vz = marineStatusPointer->weaponTarget.vz - sbPtr->DynPtr->Position.vz;
			correctlyOrientated = NPCOrientateToVector(sbPtr, &orientationDirn,NPC_TURNRATE,NULL);
		} else {
			/* Tough! */
			correctlyOrientated = 1;
		}

		/* look after the gun flash */
		if(marineStatusPointer->myGunFlash) {
			MaintainMarineGunFlash(sbPtr);
		} else {
			CreateMarineGunFlash(sbPtr);
		}
		
		/* look after the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Update3d(marineStatusPointer->soundHandle,&(sbPtr->DynPtr->Position));
		}

		marineStatusPointer->stateTimer -= NormalFrameTime;

		#if MARINE_STATE_PRINT
		textprint("Turning to face.\n");
		#endif
		return(SRC_No_Change);

	} else {
		/* Now fire a skeeter. */
	  	
		/* Remove the gunflash */
		if(marineStatusPointer->myGunFlash) 
		{	
			RemoveNPCGunFlashEffect(marineStatusPointer->myGunFlash);
			marineStatusPointer->myGunFlash = NULL;
		}
		/* .... and stop the sound */
		if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(marineStatusPointer->soundHandle);		
			Sound_Play(marineStatusPointer->My_Weapon->EndSound,"d",&(sbPtr->DynPtr->Position));
		}

		{
			SECTION_DATA *rocket_section;

			rocket_section=GetThisSectionData(marineStatusPointer->HModelController.section_data,"dum flash");

			LOCALASSERT(rocket_section);
			
			CreateFrisbeeKernel(&rocket_section->World_Offset, &rocket_section->SecMat,0);

			if (marineStatusPointer->clipammo>0) {
				marineStatusPointer->clipammo--;
			}
		}

		/* You must have fired already. */

		if(range < MARINE_CLOSE_APPROACH_DISTANCE)
		{
			/* renew firing, as we are still too close to approach */ 
			marineStatusPointer->stateTimer = marineStatusPointer->My_Weapon->FiringTime;			 		
			marineStatusPointer->volleySize = 0;
			GLOBALASSERT(marineStatusPointer->Target);
			NPCGetTargetPosition(&(marineStatusPointer->weaponTarget),marineStatusPointer->Target);
			#if MARINE_STATE_PRINT
			textprint("Returning too close renewal at range %d.\n",range);
			#endif
			return(SRC_No_Change);
		}
		else
		{
			/* we are far enough away, so return to approach */

			#if MARINE_STATE_PRINT
			textprint("Returning too far termination at range %d.\n",range);
			#endif
			return(SRC_Request_Approach);
		}

	}	
	
	return(SRC_No_Change);
}

static STATE_RETURN_CONDITION Execute_MNS_AcidAvoidance(STRATEGYBLOCK *sbPtr)
{
	MARINE_STATUS_BLOCK *marineStatusPointer;    
	DYNAMICSBLOCK *dynPtr;
	VECTORCH velocityDirection = {0,0,0};

	/* Your mission: to advance out of the acid. */
	
	LOCALASSERT(sbPtr);
	marineStatusPointer=(MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	dynPtr = sbPtr->DynPtr;
	LOCALASSERT(marineStatusPointer);
	LOCALASSERT(dynPtr);

	/* Where are we going? */

	marineStatusPointer->stateTimer-=NormalFrameTime;
	if (marineStatusPointer->stateTimer<0) {
		return(SRC_Request_Approach);
	}	

	/* Ok: should have a current target at this stage... */
	NPCGetMovementDirection(sbPtr, &velocityDirection, &(marineStatusPointer->wanderData.worldPosition),&marineStatusPointer->waypointManager);
	NPCSetVelocity(sbPtr, &velocityDirection, marineStatusPointer->nearSpeed);	
	HandleMovingAnimations(sbPtr);

	if (New_NPC_IsObstructed(sbPtr,&marineStatusPointer->avoidanceManager)) {
		/* Go to all new avoidance. */
		return(SRC_Request_Avoidance);
	}
	return(SRC_No_Change);

}

void Marine_Activate_AcidAvoidance_State(STRATEGYBLOCK *sbPtr, VECTORCH *incidence) {

	MARINE_STATUS_BLOCK *marineStatusPointer;    

	LOCALASSERT(sbPtr);
	marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(marineStatusPointer);	          		

	marineStatusPointer->gotapoint=0;

	NPC_InitMovementData(&(marineStatusPointer->moveData));
	NPC_InitWanderData(&(marineStatusPointer->wanderData));
	InitWaypointManager(&marineStatusPointer->waypointManager);
	marineStatusPointer->volleySize = 0;
	marineStatusPointer->lastState=marineStatusPointer->behaviourState;
	marineStatusPointer->behaviourState = MBS_AcidAvoidance;
	marineStatusPointer->stateTimer = ONE_FIXED;

	/* Get a destination. */
	{
		VECTORCH dest;
		if (incidence) {
			dest=*incidence;
		} else {
			/* Boo. */
			dest.vx=sbPtr->DynPtr->OrientMat.mat11;
			dest.vy=sbPtr->DynPtr->OrientMat.mat12;
			dest.vy=sbPtr->DynPtr->OrientMat.mat13;
		}
		AlignVelocityToGravity(sbPtr,&dest);
		if (Approximate3dMagnitude(&dest)==0) {
			/* Boo. */
			dest.vx=sbPtr->DynPtr->OrientMat.mat11;
			dest.vy=sbPtr->DynPtr->OrientMat.mat12;
			dest.vy=sbPtr->DynPtr->OrientMat.mat13;
		}
		marineStatusPointer->wanderData.worldPosition=sbPtr->DynPtr->Position;

		marineStatusPointer->wanderData.worldPosition.vx+=(dest.vx>>6);
		marineStatusPointer->wanderData.worldPosition.vy+=(dest.vy>>6);
		marineStatusPointer->wanderData.worldPosition.vz+=(dest.vz>>6);

	}

	/* Not sure we need an expression for this one. */
	#if 0
	Marine_QueueNeutralExpression(sbPtr);
	#endif

	HandleMovingAnimations(sbPtr);

}
