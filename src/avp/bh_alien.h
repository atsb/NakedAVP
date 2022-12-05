/*--------------Patrick 7/11/96----------------
  Header file for Alien AI support functions
  ---------------------------------------------*/

#ifndef _bhalien_h_
	#define _bhalien_h_ 1


	#ifdef __cplusplus

		extern "C" {

	#endif

#include "bh_pred.h"
 
 /*--------------------------------------------
   Enum of alien behaviour states
   --------------------------------------------*/

 typedef enum AlienBhState
 {
   ABS_Wait,
   ABS_Approach,
   ABS_Hunt,
   ABS_Wander,
   ABS_Retreat,
   ABS_Attack,
   ABS_Avoidance,
   ABS_Dying,
   ABS_Pounce,
   ABS_Jump,
   ABS_Dormant,
   ABS_Awakening,
   ABS_Taunting,
 }ALIEN_BHSTATE;

 /*--------------------------------------------
  Enum of alien animation sequences
  --------------------------------------------*/

 typedef enum AlienSequence
 {
	ASQ_Run,
 	ASQ_StandingAttack_Claw,
 	ASQ_Crawl,
	ASQ_Stand,
	ASQ_Crouch,
	ASQ_CrouchedAttack_Claw,
	ASQ_CrawlingAttack_Claw,
	ASQ_RunningAttack_Claw,
	ASQ_Pain,	 
	ASQ_Jump,
	/* Additions by ChrisF, 20/4/98 */
	ASQ_StandingTailPoise,
	ASQ_StandingTailStrike,
	ASQ_CrouchedTailPoise,
	ASQ_CrouchedTailStrike,
	ASQ_RunningTailPoise,
	ASQ_RunningTailStrike,
	ASQ_CrawlingTailPoise,
	ASQ_CrawlingTailStrike,
	ASQ_Eat,
	ASQ_Pounce,
	ASQ_JumpingTailPoise,
	ASQ_JumpingTailStrike,
	ASQ_Taunt,
	ASQ_CrouchEat,
	ASQ_Scamper,
	/* More additions for reversing, 8/5/98 */
	ASQ_Run_Backwards,
 	ASQ_Crawl_Backwards,
	ASQ_RunningAttack_Claw_Backwards,
	ASQ_RunningTailPoise_Backwards,
	ASQ_RunningTailStrike_Backwards,
	ASQ_CrawlingTailPoise_Backwards,
	ASQ_CrawlingTailStrike_Backwards,
	ASQ_CrawlingAttack_Claw_Backwards,
	ASQ_Scamper_Backwards,
 }ALIEN_SEQUENCE;

 #include "sequnces.h"

 typedef enum AlienMissions {
 	AM_UndefinedBehaviour, // Should do nothing.
 	AM_Hunt,
 	AM_GlobalHunt,
 } ALIEN_MISSION;

 typedef enum AlienType {
 	AT_Standard=0,
	AT_Predalien,
	AT_Praetorian,
 } ALIEN_TYPE;

 /*--------------------------------------------
  Alien behaviour data block
  --------------------------------------------*/
 typedef struct alienStatusBlock
 {
	ALIEN_TYPE Type;
	signed char Health;
	int GibbFactor;
	int MaxSpeed;
	int Wounds;
	int last_anim_length;
	ALIEN_BHSTATE BehaviourState;	
	ATTACK_DATA *current_attack;
	int PounceDetected;
	int JumpDetected;
	int EnablePounce;
	int EnableWaypoints;
	int PreferToCrouch;
	
	MODULE *my_containing_module;
	AIMODULE *huntingModule;

	int incidentFlag;
	int incidentTimer;

	STRATEGYBLOCK *Target;
	char Target_SBname[SB_NAME_LENGTH];
	ALIEN_MISSION Mission;

	int CurveRadius;
	int CurveLength;
	int CurveTimeOut;
	int FarStateTimer;
	int NearStateTimer;
	int IAmCrouched;
	NPC_MOVEMENTDATA moveData;
	NPC_WANDERDATA wanderData;
	
	NPC_AVOIDANCEMANAGER avoidanceManager;
	WAYPOINT_MANAGER waypointManager;

	HMODELCONTROLLER HModelController;

	char death_target_ID[SB_NAME_LENGTH]; //another strategy can be notified of the alien's death
	STRATEGYBLOCK* death_target_sbptr;

	STRATEGYBLOCK* generator_sbptr;//0 unless created by a generator

	DPID aliensIgniterId;//Which player in a multiplayer game toasted this poor beast?

	int soundHandle;
	int soundHandle2;

	/*The following three values are used by the extrapolation code for network games.
	It doesn't particularly matter if they aren't initialised*/
	int timeOfLastSend;
	VECTORCH lastFacingSent;
	VECTORCH lastVelocitySent;

 }ALIEN_STATUS_BLOCK;


 /*--------------------------------------------
  Tools data template
  --------------------------------------------*/
 typedef struct tools_data_alien
 {
	struct vectorch position;
	int shapeIndex;
	char nameID[SB_NAME_LENGTH];
	char death_target_ID[SB_NAME_LENGTH]; 
	ALIEN_TYPE type;
	int start_inactive;
	struct euler starteuler;
 }TOOLS_DATA_ALIEN;



/*--------------------------------------------
  Defines
  --------------------------------------------*/

	#define ULTRAVIOLENCE						0
	#define ALIEN_STATE_PRINT					0
	#define WOUNDING_SPEED_EFFECTS				1

  	#define ALIEN_STARTING_HEALTH				(30)
	#define NO_OF_FRAGMENTS_FROM_DEAD_ALIEN (10)

  	/* random time between 1.5 and 2 seconds,in fixed point, with granularity 1/32th second */
	#if ULTRAVIOLENCE
  	#define ALIEN_FAR_MOVE_TIME					((48+(FastRandom()&0xF))*(ONE_FIXED>>7))
	#else
  	#define ALIEN_FAR_MOVE_TIME					((48+(FastRandom()&0xF))*(ONE_FIXED>>5))
	#endif
	/* random time between 1.5 and 2 seconds,in fixed point, with granularity 1/32th second */
  	#define ALIEN_FAR_RETREAT_TIME				((48+(FastRandom()&0xF))*(ONE_FIXED>>5))
	#define ALIEN_JUMPVELOCITY 					(8000)
	#define ALIEN_FORWARDVELOCITY 				(12000)
	#define ALIEN_CURVEDISTANCE 				(8000)
	#define ALIEN_ATTACKDISTANCE_MIN			(2000)
	/* Above (1500) for Ken: reduced from 2000 */
	/* 1/6/98, changed back to 2000.  It was well bust. */
	#define ALIEN_ATTACKDISTANCE_MAX			(4000)
	#define ALIEN_ATTACKRANGE 					(3000)
	/* Range check for damage validity. */
	#define ALIEN_ATTACKTIME 					(ONE_FIXED>>1)
	/* random time between 1 and 2 seconds,in fixed point,with granularity 1/8th second */
	#define ALIEN_NEARWAITTIME					(ONE_FIXED+((FastRandom()&0x7)*(ONE_FIXED>>3)))
	
	#define PREDALIEN_SPEED_FACTOR				((ONE_FIXED*8)/10)
	#define PRAETORIAN_SPEED_FACTOR				((ONE_FIXED*8)/10)
	#define PRAETORIAN_WALKSPEED_FACTOR			((ONE_FIXED*6)/5)
	#define PRAETORIAN_CRAWLSPEED_FACTOR		(ONE_FIXED*2)

	#define ALIEN_POUNCE_MAXRANGE 				(12000)
	#define ALIEN_POUNCE_STARTMAXRANGE 			(8000)
	#define ALIEN_POUNCE_MINRANGE 				(3000)
	#define ALIEN_JUMP_SPEED					(25000)
	#define PREDALIEN_JUMP_SPEED				(25000)
	#define PRAETORIAN_JUMP_SPEED				(25000)

	#define ALIEN_JUMPINESS						(13106)
	#define PREDALIEN_JUMPINESS					(13106)
	#define PRAETORIAN_JUMPINESS				(13106)

	#define ALIEN_MASS							(160)
	#define PREDALIEN_MASS						(200)
	#define PRAETORIAN_MASS						(250)

	#define ALL_NEW_AVOIDANCE_ALIEN				0

/*--------------------------------------------
  Prototypes
  --------------------------------------------*/
 
	void CreateAlienDynamic(STRATEGYBLOCK *Generator , ALIEN_TYPE type_of_alien);
	void InitAlienBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
	void AlienBehaviour(STRATEGYBLOCK *sbPtr);
	void AlienIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming,DISPLAYBLOCK *frag);
 	void MakeAlienNear(STRATEGYBLOCK *sbPtr);
	void MakeAlienFar(STRATEGYBLOCK *sbPtr);
	int AlienShouldBeCrawling(STRATEGYBLOCK *sbPtr);
	void SetAlienShapeAnimSequence(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length);
	void SetAlienShapeAnimSequence_Core(STRATEGYBLOCK *sbPtr,HMODEL_SEQUENCE_TYPES type, int subtype, int length, int tweeningtime);
	int AlienIsAwareOfTarget(STRATEGYBLOCK *sbPtr);
	int AlienHasPathToOfTarget(STRATEGYBLOCK *sbPtr);
	int GetAlienSpeedFactor(STRATEGYBLOCK *sbPtr);
	int GetAlienSpeedFactor_ForSequence(STRATEGYBLOCK *sbPtr, HMODEL_SEQUENCE_TYPES sequence_type,int subsequence);
	void RecomputeAlienSpeed(STRATEGYBLOCK *sbPtr);
	void Alien_GoToApproach(STRATEGYBLOCK *sbPtr);
	void Alien_Awaken(STRATEGYBLOCK *sbPtr);
	int AlienIsCrawling(STRATEGYBLOCK *sbPtr);
    void DoAlienLimbLossSound(VECTORCH *position);

	#ifdef __cplusplus

		}

	#endif


#endif
