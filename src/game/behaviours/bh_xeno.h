/* Patrick 4/7/97------------------------------
  Header file for xenoborg support functions

  ChrisF 6/7/98 Well, sort of.
  ---------------------------------------------*/

#ifndef _bhxeno_h_
	#define _bhxeno_h_ 1


	#ifdef __cplusplus

		extern "C" {

	#endif

/* KJL 16:57:28 16/07/98 - particle.h is needed for LASER_BEAM_DESC */
#include "particle.h"
#include "bh_ais.h" 
/* Patrick 4/7/97------------------------------
  Enumerations of sequences and states
  ---------------------------------------------*/

	typedef enum xeno_bhstate
	{
		XS_ActiveWait,
		XS_TurnToFace,
		XS_Following,
		XS_Returning,
  		XS_Inactive,
		XS_Activating,
  		XS_Deactivating,
		XS_Avoidance,
		XS_Regenerating,
		XS_Dying,
		XS_ShootingTheRoof,

	}XENO_BHSTATE;

/* Patrick 4/7/97------------------------------
  Structures for behaviour data & tools data
  ---------------------------------------------*/
 	typedef struct xenoStatusBlock
 	{
		signed int health;
		XENO_BHSTATE behaviourState;
		XENO_BHSTATE lastState;
		int stateTimer;  		
		NPC_MOVEMENTDATA moveData;
		NPC_WANDERDATA wanderData;
		NPC_OBSTRUCTIONREPORT obstruction;
		AIMODULE *my_module;
		VECTORCH my_spot_therin;
		VECTORCH my_orientdir_therin;
		int module_range;
		int UpTime;
		int GibbFactor;
		int Wounds;

		HMODELCONTROLLER HModelController;
		DELTA_CONTROLLER *head_pan;
		DELTA_CONTROLLER *head_tilt;
		DELTA_CONTROLLER *left_arm_pan;
		DELTA_CONTROLLER *left_arm_tilt;
		DELTA_CONTROLLER *right_arm_pan;
		DELTA_CONTROLLER *right_arm_tilt;
		DELTA_CONTROLLER *torso_twist;

		STRATEGYBLOCK *Target;
		char Target_SBname[SB_NAME_LENGTH];

		VECTORCH targetTrackPos;

		int Head_Pan;
		int Head_Tilt;
		int Left_Arm_Pan;
		int Left_Arm_Tilt;
		int Right_Arm_Pan;
		int Right_Arm_Tilt;
		int Torso_Twist;

		int Old_Head_Pan;
		int Old_Head_Tilt;
		int Old_Left_Arm_Pan;
		int Old_Left_Arm_Tilt;
		int Old_Right_Arm_Pan;
		int Old_Right_Arm_Tilt;
		int Old_Torso_Twist;

		/* KJL 12:23:24 09/12/98 - muzzleflashes replaced by
		beam weapon thingies */
		LASER_BEAM_DESC LeftMainBeam;
		LASER_BEAM_DESC RightMainBeam;
		
		/* KJL 16:56:38 16/07/98 */
		LASER_BEAM_DESC TargetingLaser[3];

		unsigned int headpandir			:1;
		unsigned int headtiltdir		:1;
		unsigned int leftarmpandir		:1;
		unsigned int leftarmtiltdir		:1;
		unsigned int rightarmpandir		:1;
		unsigned int rightarmtiltdir	:1;
		unsigned int torsotwistdir		:1;
	
		unsigned int headLock			:1;
		unsigned int leftArmLock		:1;
		unsigned int rightArmLock		:1;
		unsigned int targetSightTest	:1;
		unsigned int IAmFar				:1;
		unsigned int ShotThisFrame		:1;
	
		unsigned int FiringLeft			:1;
		unsigned int FiringRight		:1;

		unsigned int UseHeadLaser		:1;
		unsigned int UseLALaser			:1;
		unsigned int UseRALaser			:1;

		unsigned int HeadLaserOnTarget	:1;
		unsigned int LALaserOnTarget	:1;
		unsigned int RALaserOnTarget	:1;

		unsigned int head_moving		:1;
		unsigned int la_moving			:1;
		unsigned int ra_moving			:1;
		unsigned int torso_moving		:1;

		int soundHandle1;
		int soundHandle2;

		int incidentFlag;
		int incidentTimer;
			
		int head_whirr;
		int left_arm_whirr;
		int right_arm_whirr;
		int torso_whirr;

		char death_target_ID[SB_NAME_LENGTH];
		STRATEGYBLOCK* death_target_sbptr;
		int death_target_request;
		
		WAYPOINT_MANAGER waypointManager;

 	}XENO_STATUS_BLOCK;

 	typedef struct tools_data_xeno
 	{
		struct vectorch position;
		int shapeIndex;
		char nameID[SB_NAME_LENGTH];
		char death_target_ID[SB_NAME_LENGTH]; 
		int death_target_request;
		struct euler starteuler;
		int UpTime;			/* Default to '20' */
		int ModuleRange;	/* Default to '7' */
 	}TOOLS_DATA_XENO;

/* Patrick 4/7/97------------------------------
  Some defines
  ---------------------------------------------*/
	#define XENO_STARTING_HEALTH				600 	 
	#define XENO_NEAR_SPEED						1000 	/* mm/s */
  	#define XENO_NEAR_ACCURACY					5		/* mm per m max deviation */		
	#define XENO_NEAR_VIEW_WIDTH				500		/* mm */
	#define XENO_CLOSE_APPROACH_DISTANCE		3000	/* mm */
	#define XENO_FIRINGPOINT_INFRONT			1000		/* mm */  	
	#define XENO_FIRINGPOINT_ACROSS				300 	/* mm */
	#define XENO_FIRINGPOINT_UP					100 	/* mm */
    #define XENO_PROJECTILESPEED 				20000	/* mm/s */
	#define XENO_PROJECTILEDAMAGE				10
	#define XENO_SENTRY_SENSITIVITY				1500
	#define XENO_FAR_MOVE_TIME					((24+(FastRandom()&0x07))*(ONE_FIXED>>4))

	/* 1-2 seconds in 1/8ths of a second */
	#define XENO_NEAR_TIMEBETWEENFIRING	((8+(FastRandom()&0x07))*(ONE_FIXED>>3))
	#define XENO_RECOILTIME				(ONE_FIXED>>1) 	/* 1/2 seconds */
	#define XENO_ACTIVATION_TIME		(ONE_FIXED)  /* 1 second */
	#define XENO_DEACTIVATION_TIME		(ONE_FIXED)  /* 1 second */
	#define XENO_REGEN_TIME				(ONE_FIXED*5)
	#define XENO_POWERDOWN_TIME			(ONE_FIXED*5)
		  
	/* 2,3 or 4*/
	#define XENO_VOLLEYSIZE				(2+(FastRandom()%3))

	#define XENO_HEADPAN_GIMBALL		(1024)
	#define XENO_HEADTILT_GIMBALL		(512)
	#define XENO_TORSO_GIMBALL			(1195)

	#define XENO_LEFTARM_CW_GIMBALL		(626)
	#define XENO_LEFTARM_ACW_GIMBALL	(910)
	#define XENO_RIGHTARM_CW_GIMBALL	(910)
	#define XENO_RIGHTARM_ACW_GIMBALL	(626)
	
	#define XENO_ARM_PITCH_GIMBALL		(1024)
	
	#if 0
	/* Original values. */
	#define XENO_HEAD_LOCK_RATE			(2)	/* Was 0 */
	#define XENO_HEAD_SCAN_RATE			(3)
	#define XENO_TORSO_TWIST_RATE		(3)
	#define XENO_ARM_LOCK_RATE			(4)
	#define XENO_FOOT_TURN_RATE			(3)
	
	#else
	/* Let's slow everything down a wee bit. */
	#define XENO_HEAD_LOCK_RATE			(2)	/* Was 0 */
	#define XENO_HEAD_SCAN_RATE			(4)
	#define XENO_TORSO_TWIST_RATE		(5)
	#define XENO_ARM_LOCK_RATE			(5)
	#define XENO_FOOT_TURN_RATE			(5)

	#endif

/* Patrick 4/7/97------------------------------
   Some prototypes
  ---------------------------------------------*/
	void InitXenoborgBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
	void XenoborgBehaviour(STRATEGYBLOCK *sbPtr);
	void MakeXenoborgNear(STRATEGYBLOCK *sbPtr);
	void MakeXenoborgFar(STRATEGYBLOCK *sbPtr);
	void XenoborgIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,VECTORCH *incoming);
	int XenoSight_FrustrumReject(STRATEGYBLOCK *sbtr,VECTORCH *localOffset);
		

	#ifdef __cplusplus

		}

	#endif


#endif
