/* Patrick 27/2/97 --------------------------------------------
  Header file for facehugger support functions
  -------------------------------------------------------------*/

#ifndef _bhfhug_h_
#define _bhfhug_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "bh_pred.h"

/* ------------------------------------------------------------
  Some enums
  -------------------------------------------------------------*/
typedef enum FhugSequence
{
	FhSQ_Run,
	FhSQ_Attack,
	FhSQ_Stand,
	FhSQ_Jump,
}FHUG_SEQUENCE;

typedef enum FhugHModelSequence {
	FhSSQ_Stand	= 0,
	FhSSQ_Run,
	FhSSQ_Dies,   
	FhSSQ_Jump,   
	FhSSQ_Attack, 
} FHUG_HMODEL_SEQUENCE;

#include "sequnces.h"
 
typedef enum facehugger_near_bhstate
{
	FHNS_Approach,
	FHNS_Attack,
	FHNS_Wait,
	FHNS_Avoidance,
	FHNS_Dying,
	FHNS_Floating,
	FHNS_Jumping,
	FHNS_AboutToJump,
}FACEHUGGER_NEAR_BHSTATE;

/* ------------------------------------------------------------
  Some structures
  -------------------------------------------------------------*/
typedef struct facehuggerStatusBlock
{
	signed int health;
	FACEHUGGER_NEAR_BHSTATE nearBehaviourState;	
	int stateTimer;
	int DoomTimer;
	int CurveRadius;
	int CurveLength;
	int CurveTimeOut;
  	unsigned int jumping :1;	
	NPC_MOVEMENTDATA moveData;
	HMODELCONTROLLER HModelController;
	int soundHandle;
	int soundHandle2;
  
	char death_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* death_target_sbptr;
	int death_target_request;
}FACEHUGGER_STATUS_BLOCK;

typedef struct tools_data_facehugger
{
	struct vectorch position;
	int shapeIndex;
	char nameID[SB_NAME_LENGTH];
	int startInactive;

	char death_target_ID[SB_NAME_LENGTH]; 
	int death_target_request;
}TOOLS_DATA_FACEHUGGER;

/* ------------------------------------------------------------
  Some structures
  -------------------------------------------------------------*/
#define FACEHUGGER_STARTING_HEALTH			5 	 
#define NO_OF_FRAGMENTS_FROM_DEAD_FHUGA	 	10
#define FACEHUGGER_NEAR_SPEED				4000/* 8000 */
#define FACEHUGGER_JUMPSPEED				6000/* 10000 */
#define FACEHUGGER_JUMPDISTANCE				3000
#define FACEHUGGER_ATTACKYOFFSET			(-300)/*300*/
#define FACEHUGGER_ATTACKZOFFSET			(325)/*2000*/
#define	FACEHUGGER_NEARATTACKTIME			(ONE_FIXED>>2)
#define	FACEHUGGER_NEARATTACKDAMAGE			10
#define FACEHUGGER_DYINGTIME				(ONE_FIXED<<2)
#define FACEHUGGER_EXPIRY_TIME				5
#define FACEHUGGER_JUMP_SPEED				(15000)

/* ------------------------------------------------------------
  Some prototypes
  -------------------------------------------------------------*/
void InitFacehuggerBehaviour(void* bhdata, STRATEGYBLOCK *sbPtr);
void FacehuggerBehaviour(STRATEGYBLOCK *sbPtr);
void MakeFacehuggerNear(STRATEGYBLOCK *sbPtr);
void MakeFacehuggerFar(STRATEGYBLOCK *sbPtr);
void FacehuggerIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);
void Wake_Hugger(STRATEGYBLOCK *sbPtr);
		

#ifdef __cplusplus
}
#endif

#endif
