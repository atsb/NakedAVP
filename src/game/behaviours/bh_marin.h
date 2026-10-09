/*--------------Patrick 14/2/97----------------
  Header file for marine support functions
  ---------------------------------------------*/

#ifndef _bhmarin_h_
	#define _bhmarin_h_ 1


	#ifdef __cplusplus

		extern "C" {

	#endif

 
	#include "bh_pred.h"
	#include "psndproj.h"
	#include "sfx.h"
	/*--------------------------------------------
   	enums of marine far and near behaviour states
   	--------------------------------------------*/
	typedef enum marine_bhstate
	{
		MBS_Waiting,
		/* Waiting - stand and do nothing, 
		 until you get a call, see an enemy,
		 or begin to fidget. */
		MBS_Wandering,
		/* Go from module to module around the environment. */
		MBS_Retreating,
		MBS_Sentry,
		MBS_Approaching,
		MBS_Firing,
		MBS_Avoidance,
		MBS_Dying,
		MBS_Responding,
		MBS_Returning,
		MBS_Pathfinding,
		MBS_Taunting,
		MBS_PanicFire,
		MBS_Reloading,
		MBS_PumpAction,
		MBS_GetWeapon,
		MBS_PanicReloading,
		MBS_AcidAvoidance,
	} MARINE_BHSTATE;

	typedef enum marine_movement_style {
		MMS_Stationary=0,
		MMS_Bored,
		MMS_Alert,
		MMS_Combat,
		MMS_Sprint,
	} MARINE_MOVEMENT_STYLE;

	typedef enum state_return_condition {
		SRC_No_Change,
		SRC_Request_Approach,
		SRC_Request_Fire,
		SRC_Request_Wander,
		SRC_Request_Avoidance,
		SRC_Request_Wait,
		SRC_Request_Retreat,
		SRC_Request_Respond,
		SRC_Request_Return,
		SRC_Request_Taunt,
		SRC_Request_PanicFire,
		SRC_Request_Reload,
		SRC_Request_PumpAction,
		SRC_Request_PullPistol,
		SRC_Request_PanicReload,
	} STATE_RETURN_CONDITION;

	/*--------------------------------------------
	 Enum of marine animation sequences
	 --------------------------------------------*/

	typedef enum MarineSequence
	{
		MSQ_Walk,
		MSQ_StandDieFront,
		MSQ_StandDieBack,
		MSQ_StartStandingFire,
		MSQ_StandingFire,
		MSQ_StandDeadFront,
		MSQ_StandDeadBack,
		MSQ_Crawl,
		MSQ_CrouchDie,	
		MSQ_CrouchDead,	
		MSQ_RunningFire,
		MSQ_Crouch,
		MSQ_Stand,
		MSQ_Jump,
		MSQ_Taunt,
		MSQ_Walk_Backwards,
		MSQ_Crawl_Backwards,
		MSQ_RunningFire_Backwards,
		
		MSQ_StandingFireSecondary,
		MSQ_RunningFireSecondary,
		MSQ_RunningFireSecondary_Backwards,

		MSQ_BaseOfCudgelAttacks=40,
	}MARINE_SEQUENCE;

	typedef enum MarineMissions {
		MM_Wait_Then_Wander, // Should do nothing until visible.
		MM_Wander,
		MM_Guard,
		MM_LocalGuard,
		MM_NonCom,
		MM_Pathfinder,
		MM_RunAroundOnFire,
	}MARINE_MISSION;

	/***** Marine squad command state *****/

	typedef struct squadcommand {
		int alertStatus;
		int responseLevel;
		AIMODULE *alertZone;
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

	} SQUAD_COMMAND_STATE;

	/*--------------------------------------------
  	Data for civilian accoutement stuff
  	--------------------------------------------*/
	
	struct hierarchy_shape_replacement;
	typedef struct hierarchy_variant_data
	{
		struct hierarchy_shape_replacement * replacements;
		int voice;
		unsigned int female :1;
	} HIERARCHY_VARIANT_DATA;

	/*--------------------------------------------
  	Marine behaviour data block
  	--------------------------------------------*/
 	typedef struct marineStatusBlock
 	{
		signed int health;
		signed int volleySize;
		signed int primaryWeaponDamage;

		MARINE_BHSTATE behaviourState;
		MARINE_BHSTATE lastState;

		STRATEGYBLOCK *Target;
		MARINE_MISSION Mission;
		char Target_SBname[SB_NAME_LENGTH];

		char death_target_ID[SB_NAME_LENGTH]; //another strategy can be notified of the marine's death
		STRATEGYBLOCK* death_target_sbptr;
		int death_target_request;

		STRATEGYBLOCK* generator_sbptr;//0 unless created by a generator
	
		AIMODULE *lastmodule;
		AIMODULE *destinationmodule;
		AIMODULE *missionmodule;
		AIMODULE *fearmodule;
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
		HMODELCONTROLLER HModelController;
		VECTORCH weaponTarget;		/* position for firing weapon at */
		DISPLAYBLOCK *myGunFlash;
		int soundHandle;
		int soundHandle2;
		NPC_OBSTRUCTIONREPORT obstruction;
		NPC_MOVEMENTDATA moveData;
		NPC_WANDERDATA wanderData;
		int IAmCrouched;
		/* CDF 15/12/97 */
		struct marine_weapon_data *My_Weapon;
		SECTION_DATA *My_Gunflash_Section;
		SECTION_DATA *My_Elevation_Section; /* For elevation computation. */
		int lastroundhit;
		SECTION_DATA *lasthitsection;
		
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

		NPC_AVOIDANCEMANAGER avoidanceManager;
		WAYPOINT_MANAGER waypointManager;

 	}MARINE_STATUS_BLOCK;

	typedef enum marine_npc_weapons {

		MNPCW_PulseRifle,
		MNPCW_Flamethrower,
		MNPCW_Smartgun,
		MNPCW_SADAR,
		MNPCW_GrenadeLauncher,
		MNPCW_Minigun,
		MNPCW_MShotgun,
		MNPCW_MPistol,
		MNPCW_MFlamer,
		MNPCW_MUnarmed,
		MNPCW_MMolotov,
		MNPCW_PistolMarine,
		MNPCW_Android,
		MNPCW_AndroidSpecial,
		MNPCW_Android_Pistol_Special,
		MNPCW_Scientist_A,
		MNPCW_Scientist_B,
		MNPCW_TwoPistols,
		MNPCW_Skeeter,
		MNPCW_End,

	} MARINE_NPC_WEAPONS;
 	
 	/*--------------------------------------------
  	Tools data template
 	--------------------------------------------*/
 	typedef struct tools_data_marine
 	{
		struct vectorch position;
		struct vectorch facing_point;
		int shapeIndex;
		char nameID[SB_NAME_LENGTH];
		MARINE_MISSION Mission;
		char death_target_ID[SB_NAME_LENGTH]; 
		int death_target_request;

		enum marine_npc_weapons marine_type;

		int path;
		int stepnumber;

		int textureID;

 	}TOOLS_DATA_MARINE;

	/*--------------------------------------------
	Weapon Behaviour Type.
   	--------------------------------------------*/


	typedef struct marine_weapon_data {
	
		MARINE_NPC_WEAPONS id;
		enum SFX_ID SfxID;
		STATE_RETURN_CONDITION (*WeaponFireFunction)(STRATEGYBLOCK *);
		void (*WeaponMisfireFunction)(SECTION_DATA *, int *);
		STATE_RETURN_CONDITION (*WeaponPanicFireFunction)(STRATEGYBLOCK *);
		char *Riffname;
		char *HierarchyName;
		char *GunflashName;
		char *ElevationSection;
		char *HitLocationTableName;
		char *TemplateName;
		char *ClipName;
		int MinRange;
		int ForceFireRange;
		int MaxRange;
		int Accuracy;
		int FiringRate;
		int FiringTime;
		int MinimumBurstSize;
		enum AMMO_ID Ammo_Type;
		int clip_size;
		int Reload_Sequence;
		int TargetCallibrationShift;
		SOUNDINDEX StartSound;
		SOUNDINDEX LoopSound;
		SOUNDINDEX EndSound;
		unsigned int EnableGrenades	:1;
		unsigned int UseElevation	:1;
		unsigned int EnableTracker	:1;
		unsigned int ARealMarine 	:1;
		unsigned int Android		:1;

	}MARINE_WEAPON_DATA;

	/*--------------------------------------------
   	Some defines....
   	--------------------------------------------*/
	#define MARINE_STATE_PRINT					0
	#define MARINE_STARTING_HEALTH				30
  	#define NO_OF_FRAGMENTS_FROM_DEAD_MARINE	10
//	#define MARINE_NEAR_SPEED					6000 	/* mm/s */
	/* Experiment.  Can they take the ramps? */
	#define MARINE_NEAR_SPEED					6000 	/* mm/s */
	#define MARINE_NEAR_VIEW_WIDTH				500		/* mm */
	#define MARINE_WEAPON_DAMAGE				5
	#define MARINE_CLOSE_APPROACH_DISTANCE		3000	/* mm */
	#define MARINE_FIRINGPOINT_INFRONT			3000		/* 900 mm */  	
	#define MARINE_FIRINGPOINT_ACROSS			300 	/* 300 mm */
	#define MARINE_FIRINGPOINT_UP				200 	/* 200 mm */
	#define MARINE_FIRINGPOINT_INFRONT_CROUCHED	900		/* mm */  	
	#define MARINE_FIRINGPOINT_ACROSS_CROUCHED	200 	/* mm */
	#define MARINE_FIRINGPOINT_UP_CROUCHED		100 	/* mm */
	#define MARINE_WEAPON_VOLLEYSIZE			(1+(FastRandom()&0x02))	/* 1,2 or 3 1/4 second bursts */
	#define MARINE_CHANCEOFGRENADE				(3)
	#define MARINE_TOO_CLOSE_TO_GRENADE_FOOL	3000
	#define MARINE_PARANOIA_TIME				(ONE_FIXED*10)
	#define MARINE_PANIC_TIME					(ONE_FIXED*120)
	#define SQUAD_PARANOIA_TIME					(ONE_FIXED)

	/* 1.5-2 seconds in 1/16 second. NB DO NOT INCREASE THIS */
	#define MARINE_FAR_MOVE_TIME ((24+(FastRandom()&0x07))*(ONE_FIXED>>4)) 				
	/* 1-2 seconds in 1/8ths of a second */
	#define MARINE_NEAR_TIMEBETWEENFIRING	((8+(FastRandom()&0x07))*(ONE_FIXED>>3))
//	#define MARINE_NEAR_FIRE_TIME			(ONE_FIXED>>2) 	/* 1/4 second */
	#define MARINE_NEAR_FIRE_TIME			(ONE_FIXED) 	/* 1 second */
	/* random time between 1 and 2 seconds,in fixed point,with granularity 1/8th second */
	#define MARINE_NEARWAITTIME					(ONE_FIXED+((FastRandom()&0x7)*(ONE_FIXED>>3)))

	#define SEAL_NEAR_SPEED						7000
	#define SEAL_WEAPON_DAMAGE					15
	#define SEAL_STARTING_HEALTH				150

	//#define MINIGUN_IDLE_SPEED (ONE_FIXED>>2)
	#define MINIGUN_IDLE_SPEED (0)
	#define MINIGUN_MAX_SPEED (ONE_FIXED*20)
	#define MINIGUN_MINIMUM_BURST	(25)

	/*--------------------------------------------
   	Some prototypes...
   	--------------------------------------------*/
	void InitMarineBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
	void InitSealBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
	void SendRequestToMarine(STRATEGYBLOCK* sbPtr,BOOL state,int extended_data);
	void MarineBehaviour(STRATEGYBLOCK *sbPtr);
	void MakeMarineNear(STRATEGYBLOCK *sbPtr);
	void MakeMarineFar(STRATEGYBLOCK *sbPtr);
	void MarineIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section, VECTORCH *incoming);
	void WarnMarineOfAttack(STRATEGYBLOCK *marine,STRATEGYBLOCK *attacker);
	
	DISPLAYBLOCK* AddNPCGunFlashEffect(VECTORCH *position, MATRIXCH* orientation, enum SFX_ID sfxID);
	void RemoveNPCGunFlashEffect(DISPLAYBLOCK* dPtr);
	void MaintainNPCGunFlashEffect(DISPLAYBLOCK* dPtr, VECTORCH *position, MATRIXCH* orientation);

	int Validate_Target(STRATEGYBLOCK *target,char *SBname);
	int Validate_Strategy(STRATEGYBLOCK *target,char *SBname);
	AIMODULE *NearNPC_GetTargetAIModuleForRetreat(STRATEGYBLOCK *sbPtr, NPC_MOVEMENTDATA *moveData);
	extern void InitSquad(void);
	extern void DoSquad(void);
	extern void ZoneAlert(int level,AIMODULE *targetModule);
	extern void Marine_CorpseSightingTest(STRATEGYBLOCK *corpse);
    int MarineSight_FrustrumReject(STRATEGYBLOCK *sbPtr,VECTORCH *localOffset,STRATEGYBLOCK *target);

	#ifdef __cplusplus

	}

	#endif


#endif
