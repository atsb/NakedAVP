/* CDF 11/6/98 - AI support functions moved out of bh_pred. */

#ifndef _bhais_h_
#define _bhais_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------
General AI Support
-------------------------------*/
typedef struct npc_obstructionreport
{
	unsigned int environment :1; /* x consecutive obstructive colls against env or indestructible object */
	unsigned int destructableObject :1;	/* x consecutive obstructive colls against destructible object */
	unsigned int otherCharacter :1;	/* single collison with other npc of same type */
	unsigned int anySingleObstruction :1; /* any single collision of above types */
}NPC_OBSTRUCTIONREPORT;

typedef enum avoidance_substate {
	AvSS_FreeMovement=0,
	AvSS_FirstAvoidance,
	AvSS_SecondAvoidance,
	AvSS_ThirdAvoidance,
} AVOIDANCE_SUBSTATE;

typedef enum avoidance_return_condition {
	AvRC_Clear=0,
	AvRC_Avoidance,
	AvRC_Failure,
} AVOIDANCE_RETURN_CONDITION;

typedef struct npc_avoidancemanager {
	/* New structure for super-avoidance system - expand at will! */
	VECTORCH avoidanceDirection;
	VECTORCH incidenceDirection;
	VECTORCH incidentPoint;

	VECTORCH aggregateNormal;
	
	int recommendedDistance;
	int timer;
	STRATEGYBLOCK *primaryCollision;
	AVOIDANCE_SUBSTATE substate;
	enum AMMO_ID ClearanceDamage;
	/* Third avoidance parameters! */
	int stage;
	VECTORCH baseVector;
	VECTORCH basePoint;
	VECTORCH currentVector;
	MATRIXCH rotationMatrix;
	VECTORCH bestVector;
	int bestDistance;
	int bestStage;

} NPC_AVOIDANCEMANAGER;

#define STANDARD_AVOIDANCE_TIME	(ONE_FIXED*5)
#define THIRD_AVOIDANCE_MINDIST	(2000)

typedef struct npc_movementdata
{
	int numObstructiveCollisions;
	VECTORCH avoidanceDirn;
	VECTORCH lastTarget;
	VECTORCH lastVelocity;
	int numReverses;
	AIMODULE *lastModule;
}NPC_MOVEMENTDATA;

typedef struct npc_wanderdata
{
 	int currentModule;
	VECTORCH worldPosition;
}NPC_WANDERDATA;
	
#define NPC_AVOIDTIME				(ONE_FIXED<<1)
#define NPC_GMD_NOPOLY				(-1)
#define NPC_MIN_MOVEFROMPOLYDIST	(650)
#define NPC_IMPEDING_COL_THRESHOLD	(10) /* Was 10 */
#define NPC_JUMPSPEED				(55) /* mm/s */
#define NPC_JUMPHEIGHT				(1000)
//#define NPC_TURNRATE				(ONE_FIXED) /* thro' 360 degrees */
#define NPC_TURNRATE				(4096)
#define NPC_DEATHTIME				(ONE_FIXED>>1)
#define NPC_TARGETPOINTELEVATION	(-400)
#define NPC_INANIMATEOBJECTDAMAGE	(10000)
#define NPC_NOWANDERMODULE			(-1)
#define NPC_BIMBLINGINMODULE 		(-2)

#define NUM_ATTACKFLAGS				(3)

/* Death Structures */

typedef struct hit_facing {
	int Front:	1;
	int Back:	1;
	int Left:	1;
	int Right:	1;
} HIT_FACING;

typedef struct death_data {
	int Sequence_Type;
	int Sub_Sequence;
	int TweeningTime;
	int Sequence_Length;
	int Multiplayer_Code;
	int Unique_Code; //unique across all deaths - for saving
	int wound_flags; /* HModel wound flags */
	int priority_wounds;
	int Template:	1;
	HIT_FACING Facing;
	int Burning:	1;
	int Electrical:	1;
	int Crouching:	1;
	int MinorBoom:	1;
	int MajorBoom:	1;
} DEATH_DATA;

typedef struct attack_data {
	int Sequence_Type;
	int Sub_Sequence;
	int TweeningTime;
	int Sequence_Length;
	int Multiplayer_Code;
	int Unique_Code; //unique across all attacks - for saving
	int wound_flags;
	enum AMMO_ID flag_damage[NUM_ATTACKFLAGS];
	int Crouching:	1;
	int Pouncing:	1;
} ATTACK_DATA;

typedef enum movement_data_index {
	MDI_Marine_Mooch_Bored=0,
	MDI_Marine_Mooch_Alert,
	MDI_Marine_Combat,
	MDI_Marine_Sprint,
	MDI_Civilian_Mooch_Bored,
	MDI_Civilian_Mooch_Alert,
	MDI_Civilian_Combat,
	MDI_Civilian_Sprint,
	MDI_Predator,
	MDI_Casual_Predator,
	MDI_Xenoborg,
	MDI_End,
} MOVEMENT_DATA_INDEX;

typedef struct movement_data {
	MOVEMENT_DATA_INDEX index;
	unsigned int maxSpeed;
	unsigned int acceleration;
} MOVEMENT_DATA;

/* Function Declarations */

extern int CheckAdjacencyValidity(AIMODULE *target,AIMODULE *source,int alien);
extern int AIModuleIsVisible(AIMODULE *aimodule);
	
extern int NPC_IsDead(STRATEGYBLOCK *sbPtr);
extern void NPC_InitMovementData(NPC_MOVEMENTDATA *moveData);
extern void NPC_IsObstructed(STRATEGYBLOCK *sbPtr, NPC_MOVEMENTDATA *moveData, NPC_OBSTRUCTIONREPORT *details, STRATEGYBLOCK **destructableObject);
extern int NPC_CannotReachTarget(NPC_MOVEMENTDATA *moveData, VECTORCH* thisTarget, VECTORCH* thisVelocity);
extern void NPCGetAvoidanceDirection(STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection, NPC_OBSTRUCTIONREPORT *details);
extern void NPCGetTargetPosition(VECTORCH *targetPoint,STRATEGYBLOCK *target);
extern int NPCSetVelocity(STRATEGYBLOCK *sbPtr, VECTORCH* targetDirn, int in_speed);
extern int NPCOrientateToVector(STRATEGYBLOCK *sbPtr, VECTORCH *zAxisVector, int turnspeed, VECTORCH *offset);
extern void NPCGetMovementTarget(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *targetPosition, int* targetIsAirduct,int alien);
extern void NPCGetMovementDirection(STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection, VECTORCH* target, WAYPOINT_MANAGER *waypointManager);
extern void NPC_InitWanderData(NPC_WANDERDATA *wanderData);
extern void NPC_FindWanderTarget(STRATEGYBLOCK *sbPtr, NPC_WANDERDATA *wanderData, NPC_MOVEMENTDATA *moveData);
extern void NPC_FindAIWanderTarget(STRATEGYBLOCK *sbPtr, NPC_WANDERDATA *wanderData, NPC_MOVEMENTDATA *moveData, int alien);
extern void ProjectNPCShot(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *muzzlepos, MATRIXCH *muzzleorient,enum AMMO_ID AmmoID, int multiple);
extern void CastLOSProjectile(STRATEGYBLOCK *sbPtr, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int inaccurate);
extern int VerifyHitShot(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, VECTORCH *muzzlepos, VECTORCH *in_shotvector, enum AMMO_ID AmmoID, int multiple, int maxrange);
extern AIMODULE *GetNextModuleForLink(AIMODULE *source,AIMODULE *target,int max_depth,int alien);
extern AIMODULE *GetNextModuleForLink_Core(AIMODULE *source,AIMODULE *target,int max_depth,int visibility_check,int alien);
extern int GetNextModuleInPath(int current_module, int path);
extern AIMODULE *TranslatePathIndex(int current_module, int path);
extern int GetClosestStepInPath(int path,MODULE* current_module);

extern DEATH_DATA *GetDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,DEATH_DATA *FirstDeath,int wound_flags,int priority_wounds,
	int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical);
extern DEATH_DATA *GetAlienDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
	int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical);
extern DEATH_DATA *GetMarineDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
	int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical);
extern DEATH_DATA *GetPredatorDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
	int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical);
extern DEATH_DATA *GetXenoborgDeathSequence(HMODELCONTROLLER *controller,SECTION *TemplateRoot,int wound_flags,int priority_wounds,
	int hurtiness,HIT_FACING *facing,int burning,int crouching,int electrical);

extern DEATH_DATA *GetThisDeath_FromCode(HMODELCONTROLLER *controller,DEATH_DATA *FirstDeath,int code);
extern DEATH_DATA *GetThisDeath_FromUniqueCode(int code);

extern ATTACK_DATA *GetThisAttack_FromCode(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int code);
extern ATTACK_DATA *GetAttackSequence(HMODELCONTROLLER *controller,ATTACK_DATA *FirstAttack,int wound_flags,int crouching, int pouncing);
extern ATTACK_DATA *GetAlienAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching);
extern ATTACK_DATA *GetAlienPounceAttack(HMODELCONTROLLER *controller,int wound_flags,int crouching);
extern ATTACK_DATA *GetWristbladeAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching);
extern ATTACK_DATA *GetPredStaffAttackSequence(HMODELCONTROLLER *controller,int wound_flags,int crouching);

extern ATTACK_DATA *GetThisAttack_FromUniqueCode(int code);//for loading

extern AIMODULE *NearNPC_GetTargetAIModuleForRetreat(STRATEGYBLOCK *sbPtr, NPC_MOVEMENTDATA *moveData);
extern AIMODULE *General_GetAIModuleForRetreat(STRATEGYBLOCK *sbPtr,AIMODULE *fearModule,int max_depth);
/* All New Avoidance Code! */
extern int New_NPC_IsObstructed(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager);
extern void Initialise_AvoidanceManager(STRATEGYBLOCK *sbPtr, NPC_AVOIDANCEMANAGER *manager);
extern AVOIDANCE_RETURN_CONDITION AllNewAvoidanceKernel(STRATEGYBLOCK *sbPtr,NPC_AVOIDANCEMANAGER *manager);
/* All New Avoidance Code! */
extern const MOVEMENT_DATA *GetThisMovementData(MOVEMENT_DATA_INDEX index);
extern void AlignVelocityToGravity(STRATEGYBLOCK *sbPtr,VECTORCH *velocity);

extern int NPC_targetIsPlayer; 
extern VECTORCH NPC_movementTarget;
extern int *NPC_myPoly;
extern int *NPC_myLastPoly;
extern int *NPC_lastFrameModule;

extern int Observer;	

typedef struct pathheader
{
	int path_length;
	AIMODULE** modules_in_path;
}PATHHEADER;

extern int PathArraySize;
extern PATHHEADER* PathArray;

#ifdef __cplusplus
}
#endif


#endif
