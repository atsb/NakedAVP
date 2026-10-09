/*--------------Patrick 21/1/97-----------------------
  Header file for predator AI & NPC support functions
  ---------------------------------------------------*/

#ifndef _bhpred_h_
#define _bhpred_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "bh_ais.h"
#include "decal.h"

/*-----------------------------
Predator Specific Support
-------------------------------*/	
/* predator animation sequences */	
typedef enum predatorsequence
{
	PredSQ_Run=0,
	PredSQ_Crawl,
	PredSQ_Stand,
	PredSQ_Crouch,
	PredSQ_StandDie,
	PredSQ_StandDead,
	PredSQ_CrouchDie,	
	PredSQ_CrouchDead,	
	PredSQ_StandingSwipe,
	PredSQ_CrouchedSwipe,
	PredSQ_RunningSwipe,
	PredSQ_CrawlingSwipe,
	PredSQ_Jump,
	PredSQ_Taunt,
	PredSQ_Run_Backwards,
	PredSQ_Crawl_Backwards,
	PredSQ_RunningSwipe_Backwards,
	PredSQ_CrawlingSwipe_Backwards,
	PredSQ_BaseOfStaffAttacks=20,
	PredSQ_BaseOfWristbladeAttacks=40,
}PREDATOR_SEQUENCE;

typedef enum pred_bhstate {
	PBS_Wandering,
	PBS_Hunting,
	PBS_Avoidance,
	PBS_Withdrawing,
	PBS_Dying,
	PBS_Recovering,
	PBS_SwapWeapon,
	PBS_Engaging,
	PBS_Attacking,
	PBS_Pathfinding,
	PBS_Returning,
	PBS_Taunting,
	PBS_SelfDestruct,
} PRED_BHSTATE;

typedef enum pred_return_condition {
	PRC_No_Change,
	PRC_Request_Engage,
	PRC_Request_Attack,
	PRC_Request_Wander,
	PRC_Request_Avoidance,
	PRC_Request_Hunt,
	PRC_Request_Withdraw,
	PRC_Request_Recover,
	PRC_Request_Swap,
	PRC_Request_Return,
	PRC_Request_Pathfind,
	PRC_Request_Taunt,
	PRC_Request_SelfDestruct,
} PRED_RETURN_CONDITION;

/* cloaking states */
typedef enum pred_cloakstate
{
	PCLOAK_Off,
	PCLOAK_Activating,
	PCLOAK_On,
	PCLOAK_Deactivating,
}PRED_CLOAKSTATE;

typedef struct predatorPersonalParameters
{
	int startingHealth;				/* initial health */
	int speed;						/* running speed */
	int defenceHealth;			 	/* health level below which defensive behaviour is used */
	int useShoulderCannon;		 	/* 1 = shouldercannon, 0 = disc */
	int timeBetweenRangedAttacks;	
	int maxShotsPerRangedAttack;
	int timeBetweenShots;
	int closeAttackDamage;
	int regenerationUnit;			/* health recovered when made visible */
	int chanceOfCloaking;			/* 0-8, chance in 8 */
} PREDATOR_PERSONALPARAMETERS;

typedef enum predator_npc_weapons {

	PNPCW_Pistol,
	PNPCW_Wristblade,
	PNPCW_PlasmaCaster,
	PNPCW_Staff,
	PNPCW_Medicomp,
	PNPCW_Speargun,
	PNPCW_SeriousPlasmaCaster,
	PNPCW_End,

} PREDATOR_NPC_WEAPONS;

typedef struct predator_weapon_data {

	PREDATOR_NPC_WEAPONS id;
	PRED_RETURN_CONDITION (*WeaponFireFunction)(STRATEGYBLOCK *);
	PRED_RETURN_CONDITION (*WeaponEngageFunction)(STRATEGYBLOCK *);
	char *Riffname;
	char *HierarchyName;
	char *GunName;
	char *ElevationName;
	char *HitLocationTableName;
	int MinRange;
	int ForceFireRange;
	int MaxRange;
	int FiringRate;
	int VolleySize;
	int SwappingTime;
	int UseElevation	:1;

}PREDATOR_WEAPON_DATA;

typedef struct predatorStatusBlock
{
	signed int health;
	PRED_BHSTATE behaviourState;
	PRED_BHSTATE lastState;

	int stateTimer;
	int internalState;
	int patience;
	int enableSwap;
	int enableTaunt;
	VECTORCH weaponTarget;			/* target position for firing weapon at */
	int volleySize;					/* used for weapon control */
	NPC_OBSTRUCTIONREPORT obstruction;
	NPC_MOVEMENTDATA moveData;
	NPC_WANDERDATA wanderData;
	
	NPC_AVOIDANCEMANAGER avoidanceManager;
	WAYPOINT_MANAGER waypointManager;

	int IAmCrouched;
	int personalNumber;				/* for predator personalisation */
	int nearSpeed;
	int GibbFactor;

	int incidentFlag;
	int incidentTimer;

	PREDATOR_WEAPON_DATA *Selected_Weapon;
	PREDATOR_NPC_WEAPONS PrimaryWeapon;
	PREDATOR_NPC_WEAPONS SecondaryWeapon;
	PREDATOR_NPC_WEAPONS ChangeToWeapon;
	ATTACK_DATA *current_attack;

	STRATEGYBLOCK *Target;
	char Target_SBname[SB_NAME_LENGTH];
	int soundHandle;
	
	HMODELCONTROLLER HModelController;
	SECTION_DATA *My_Gun_Section; /* For template computation. */
	SECTION_DATA *My_Elevation_Section; /* For elevation computation. */

	/* these are for cloaking... */
	PRED_CLOAKSTATE CloakStatus;
	int CloakingEffectiveness;
	int CloakTimer;

	/* And these for the laser dots. */
	THREE_LASER_DOT_DESC Pred_Laser_Sight;
	int Pred_Laser_On	:1;
	int Explode			:1;
	/* Pathfinder parameters */
	int path;
	int stepnumber;
	AIMODULE *missionmodule;
	AIMODULE *fearmodule;
	/* Pathfinder parameters */

	char death_target_ID[SB_NAME_LENGTH]; //another strategy can be notified of the marine's death
	STRATEGYBLOCK* death_target_sbptr;
	int death_target_request;

}PREDATOR_STATUS_BLOCK;

typedef struct dormantPredatorStatusBlock
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	void* toolsData; 
}DORMANT_PREDATOR_STATUS_BLOCK;

/* Tools data template */
typedef struct tools_data_predator
{
	struct vectorch position;
	int shapeIndex;
	int predator_number;
	char nameID[SB_NAME_LENGTH];
	PREDATOR_NPC_WEAPONS primary;
	PREDATOR_NPC_WEAPONS secondary;
	int path;
	int stepnumber;

	char death_target_ID[SB_NAME_LENGTH]; 
	int death_target_request;

}TOOLS_DATA_PREDATOR;

#define PRED_SELF_DESTRUCT_TIMER			(ONE_FIXED*3)
#define PRED_WALKING_SPEED_MAX				(5000)
#define PRED_WALKING_SPEED_MIN				(3000)
#define PRED_PATIENCE_TIME					(6*ONE_FIXED)
#define PRED_REGEN_TIME						(10*ONE_FIXED)
#define PRED_MAXIDENTITY					(4) 	 	
#define NO_OF_FRAGMENTS_FROM_DEAD_PREDATOR 	(10)
#define PRED_CLOSE_ATTACK_RANGE				(1500) 	/* mm */
#define PRED_STAFF_ATTACK_RANGE				(2000) 	/* mm */
#define PRED_NEAR_VIEW_WIDTH				(500)	/* mm */  	
#define PRED_FPPLASMA_INFRONT				(600) 	/* mm */
#define PRED_FPPLASMA_ACROSS				(-500) 	/* mm */
#define PRED_FPPLASMA_UP					(900) 	/* mm */
#define PRED_FPPLASMA_INFRONTCROUCH			(600) 	/* mm */
#define PRED_FPPLASMA_ACROSSCROUCH			(-500) 	/* mm */
#define PRED_FPPLASMA_UPCROUCH				(400) 	/* mm */
#define PRED_FPDISC_INFRONT					(600) 	/* mm */
#define PRED_FPDISC_ACROSS					(500) 	/* mm */
#define PRED_FPDISC_UP						(500) 	/* mm */
#define PRED_FPDISC_INFRONTCROUCH			(600) 	/* mm */
#define PRED_FPDISC_ACROSSCROUCH			(500) 	/* mm */
#define PRED_FPDISC_UPCROUCH				(200) 	/* mm */
#define PRED_PLASBOLTSPEED 					(22000)	/* mm/s */
#define PRED_PLASBOLTDAMAGE					(50)
/* 1.5-2 seconds in 1/16 second. NB DO NOT INCREASE THIS */
#define PRED_FAR_MOVE_TIME			((24+(FastRandom()&0x07))*(ONE_FIXED>>4))  	
#define PRED_NEAR_CLOSEATTACK_TIME	ONE_FIXED	   /* 1 second */

extern void InitPredatorBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
extern void InitDormantPredatorBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
extern void PredatorBehaviour(STRATEGYBLOCK *sbPtr);
extern void MakePredatorNear(STRATEGYBLOCK *sbPtr);
extern void MakePredatorFar(STRATEGYBLOCK *sbPtr);
extern void PredatorIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming);
extern void ActivateDormantPredator(STRATEGYBLOCK *sbPtr);
extern int NPCPredatorIsCloaked(STRATEGYBLOCK *sbPtr);
extern void StartPredatorSelfDestructExplosion(STRATEGYBLOCK *sbPtr);

#ifdef __cplusplus
}
#endif


#endif
