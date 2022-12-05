/* ChrisF 17/3/98 ------------------------------------------------
  Header file for alien queen support functions
  -----------------------------------------------------------------*/

#ifndef _bhqueen_h_
#define _bhqueen_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "bh_pred.h"
#include "scream.h"

typedef enum queen_behaviour_state {

	QBS_Command,
	QBS_Waypoints,
	QBS_Engagement,
	QBS_GoingForObject,
	QBS_CarryingObject,
	QBS_Reconsider,
	QBS_ClimbingOutOfAirlock,
	QBS_Dead,

} QUEEN_BEHAVIOUR_STATE;

typedef enum queen_manoeuvre {
	QM_Standby,
	QM_StepForward,
	QM_StepBack,
	QM_TurnLeft,
	QM_TurnRight,
	QM_ComeToPoint,
	QM_Taunt,
	QM_Hiss,
	QM_LeftSwipe,
	QM_RightSwipe,
	QM_Charge,
	QM_Close,
	QM_Jump,
	QM_ButtCharge,
	QM_ButtAttack,
	QM_Stun,
	QM_Climbing,
	QM_ChangeFacing,
	QM_ApproachWall,
} QUEEN_MANOEUVRE;

typedef enum queen_foot {
	LeftFoot,
	RightFoot,
} QUEEN_FOOT;

typedef struct tools_data_queen
{
	struct vectorch position;
	int shapeIndex;
	char nameID[SB_NAME_LENGTH];
	char death_target_ID[SB_NAME_LENGTH]; 
	int death_target_request;
}TOOLS_DATA_QUEEN;

typedef struct queenStatusBlock
{
	QUEEN_BEHAVIOUR_STATE QueenState;
	QUEEN_MANOEUVRE current_move;
	QUEEN_MANOEUVRE next_move;

	QUEEN_FOOT fixed_foot;
	SECTION_DATA *fixed_foot_section;
	VECTORCH fixed_foot_oldpos;

	DELTA_CONTROLLER *attack_delta;
	DELTA_CONTROLLER *hit_delta;
	VECTORCH TargetPos;

	int moveTimer;

	NPC_MOVEMENTDATA moveData;
	NPC_WANDERDATA wanderData;
	HMODELCONTROLLER HModelController;

	char death_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* death_target_sbptr;
	int death_target_request;

	BOOL TempTarget;//going for an intermediate point
	int TempTargetTimer;//time before queen gives up going for intermediate point

	int CurrentQueenObject;
	int QueenStateTimer;
	int QueenObjectBias;
	int QueenPlayerBias;
	int QueenTauntTimer;
	int QueenFireTimer;
	VECTORCH LastVelocity;
	SECTION_DATA* QueenRightHand;
	STRATEGYBLOCK* QueenTargetSB;

	BOOL BeenInAirlock;
	BOOL QueenActivated; //queen is inactive until seen

	BOOL TargetInfoValid; //have the next three items been set
	int TargetDistance; //distance of current target from queen
	int TargetRelSpeed; //targets speed in queen's direction
	VECTORCH TargetDirection; //targets direction relative to queen
	VECTORCH VectToTarget;

	unsigned int PlayingHitDelta :1;

	int SwerveTimer;
	BOOL SwerveDirection;

	VECTORCH ClimbStartPosition; //used when climing out of the airlock
	BOOL AttackDoneItsDamage;

	/*special hangar airlock state stuff*/
	STRATEGYBLOCK* upper_airlockdoor_sbptr;
	VECTORCH upper_airlockdoor_start;
	STRATEGYBLOCK* lower_airlockdoor_sbptr;
	VECTORCH lower_airlockdoor_start;

	int soundHandle;
	QUEEN_SOUND_CATEGORY lastSoundCategory;


} QUEEN_STATUS_BLOCK;

 
void InitQueenBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
void QueenBehaviour(STRATEGYBLOCK *sbPtr);
void MakeQueenNear(STRATEGYBLOCK *sbPtr);
void MakeQueenFar(STRATEGYBLOCK *sbPtr);
void QueenIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,SECTION_DATA *Section, VECTORCH *incoming, VECTORCH *point);

#ifdef __cplusplus
}
#endif

#endif
