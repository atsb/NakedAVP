#ifndef _bhtypes_h_
#define _bhtypes_h_ 1

#ifndef _equipmnt_h_
#include "equipmnt.h"
#endif

#include "pmove.h"


#ifdef __cplusplus

	extern "C" {

#endif



/* 
	I think I am going to devide the behaviour of objects into two different forms
	
	In terms of emulating 3d we need to think of multiple cluster of sprite sequences to
	descibe any one motion. In this respect I am going to have a list of capabilities for
	objects. If you supply One animation sequence that allows a capabiltity the animation capability
	language will allow that capabiltiy. If it is a front walk, the object will walk
	but it will always display the one type of sequence even if it is walking away from
	you. When we load up a sprite sequence we include an enum that tells us what capabiltiy_type
	these sequence is part of. The animation capabiltity language and capabiltiy_type can refer
	to morphing equally as to sprites (morphing is some what easier as the object will always look
	right


	Other objects are rather dull and the above capability system is too complex. A module
	playing a TV does not need such a comple behaviour. These objects have much simpler
	strategies and we will have specfifc low level functions to deal with them.

*/


typedef enum actor_capability_types
{
	BHTypeWalk,				/* gen movement */
	BHTypeCrawl,			/* gen movement */
	BHTypeRun,				/* gen movement */
	BHTypeFly,				/* gen movement */
	BHTypeStandUp,			
	BHTypeSitDown,
	BHTypeKneelDown,
	BHTypeCroach,
	BHTypeAttack1,
	BHTypeAttack2,
	BHTypeAttack3,
	BHTypeRangedAttack1,
	BHTypeRangedAttack2,
	BHTypeRangedAttack3,
	BHTypeHit,
	BHTypeDying,
	BHTypeDead,
	BHTypeJump
} ACTOR_CAPABILITY_TYPES;




/* ****************** STRATEGY BLOCK Behaviour DESCRIPTION ************ */


typedef struct CapabilityDescription
{
	int num_animating_items;
	int **item_animations;

} CAPABILITY_DESCRIPTION;


/* the item_animations are specific to each animating
item the have no formal description interms of a type
the theyare listed as.

int item_num
seq_des1
seq_des2
seq_des3
seq_des4
term

the list of void* pointers points to the initial
item_num for the following sequences
*/

typedef struct sequence_descriptor
{
	ACTOR_CAPABILITY_TYPES cap_type;
	TXANIMHEADER* txanim_header;
	int view_angle;
	int view_azimuth;
} SEQUENCE_DESCRIPTOR;	


typedef struct SuicideTimer
{
	int time_left;	/*agrgggghhh*****/

}SUICIDE_TIMER;


	

/**************************** SPECIFIC BEHAVIOUR TYPES ************************/

		
// ENUM now in Stratdef.h


/*-------------Patrick 21/10/96 --------------------
  This structure is used in Player Status to represent
  the various player input requests.  It consists
  of single bit fields, which are used as a bit-mask 
  for recording key-combo sequences for special moves.
  for the purposes of bit-masking, it is implemented as
  a union with an unsigned int...

  NB it is easier to add new fields towards the end,
  as otherwise you may have to change the special
  move input bitmasks
  --------------------------------------------------*/
typedef struct player_input_requests
{
	unsigned int Rqst_Forward :1;					
	unsigned int Rqst_Backward :1;
	unsigned int Rqst_TurnLeft :1;
	unsigned int Rqst_TurnRight :1;
	unsigned int Rqst_LookUp :1;
	unsigned int Rqst_LookDown :1;
	unsigned int Rqst_FirePrimaryWeapon :1;	  
	unsigned int Rqst_Faster :1;	  
	unsigned int Rqst_SideStepLeft :1;	  
	unsigned int Rqst_SideStepRight :1;	  
	unsigned int Rqst_Strafe :1;
	unsigned int Rqst_Crouch :1;	  
	unsigned int Rqst_Jump :1;
	/* NB Lie Down is set by special moves only (ie doesn't require a user input, configuration entry, etc) */
	unsigned int Rqst_Operate :1;
	unsigned int Rqst_CentreView :1;		  
	unsigned int Rqst_NextWeapon :1;
	unsigned int Rqst_PreviousWeapon :1;
	unsigned int Rqst_WeaponNo :4;
	unsigned int Rqst_QuitGame :1;
	unsigned int Rqst_PauseGame :1;

	/* KJL 16:58:37 04/11/98 - Change vision does a variety of things, dependent on the player's
	character. */
	unsigned int Rqst_ChangeVision :1;
	unsigned int Rqst_FireSecondaryWeapon :1;	  

	/* Predator Specific */
	unsigned int Rqst_CycleVisionMode :1;
	unsigned int Rqst_ZoomIn :1;
	unsigned int Rqst_ZoomOut :1;
	unsigned int Rqst_GrapplingHook :1;
	
	/* Alien Specific */
	unsigned int Rqst_Spit :1;

	/* Marine Specific */
	unsigned int Rqst_ThrowFlare :1;
	unsigned int Rqst_Jetpack :1;

	unsigned int :0;

}PLAYER_INPUT_REQUESTS;

/*-------------Patrick 23/10/96 --------------------
  Some defines for key combo bit masks
  these should correspond to the above request flags.
  --------------------------------------------------*/
#define INPUT_BITMASK_FORWARD	0x00000001
#define INPUT_BITMASK_BACKWARD 0x00000002
#define INPUT_BITMASK_LEFT		0x00000004
#define INPUT_BITMASK_RIGHT		0x00000008
#define INPUT_BITMASK_FIRE		0x00000040
#define INPUT_BITMASK_FASTER 	0x00000080
#define INPUT_BITMASK_STRAFE 	0x00000100
#define INPUT_BITMASK_CROUCH 	0x00000200
#define INPUT_BITMASK_JUMP 		0x00000400



/* KJL 14:16:52 09/20/96 - the new player status type 
   modified by patrick */
typedef struct player_status
{	
	AVP_BEHAVIOUR_TYPE	bhvr_type;

    /* player's weapons */
	PLAYER_WEAPON_DATA	WeaponSlot[MAX_NO_OF_WEAPON_SLOTS];
	enum WEAPON_SLOT	SelectedWeaponSlot;
	enum WEAPON_SLOT	SwapToWeaponSlot;
	enum WEAPON_SLOT	PreviouslySelectedWeaponSlot;
    
    int	Health;	 /* in 16.16 */
	int	Energy;	 /* in 16.16 */
	int	Armour;	 /* in 16.16 */
 
    /* general info */
	/* KJL 17:28:20 09/19/96 - not yet used
	
	int	ArmourType;
	int	HealingRate;
	int	CloakingType;
	int	VisionType;
    */
		
	/*-----Patrick 15/10/96--------- 
	Player movement bits...
	------------------------------*/
	enum player_morph_state ShapeState;		/* for controlling morphing */
	
	/* and these are for free (ie normal) movement, 
	and should be set by the (platform dependant) input 
	device reading function */
	unsigned char Mvt_DeviceType;		  
	signed int Mvt_MotionIncrement;	/* 65536 (Forward) to -65536 (Backward) */					
	signed int Mvt_TurnIncrement;		/* 65536 (Right) to -65536 (Left)*/
	signed int Mvt_PitchIncrement;	/* 65536 to -65536 */
	signed int Mvt_SideStepIncrement;	/* 65536 to -65536 */

	/* KJL 10:48:33 03/26/97 - inertia data */
	signed int ForwardInertia;
	signed int StrafeInertia; 
	signed int TurnInertia; 	

	int ViewPanX; /* the looking up/down value that used to be in displayblock */

	union Mvt_InputRequests
	{
		unsigned int Mask;
		unsigned int Mask2;
		PLAYER_INPUT_REQUESTS Flags;		
	}Mvt_InputRequests;

	/* security clearances */
	unsigned int securityClearances;
	/* useful flags */
	unsigned int IsAlive :1;
	unsigned int IsImmortal :1;
	unsigned int Mvt_AnalogueTurning :1;
	unsigned int Mvt_AnaloguePitching :1;
	unsigned int Absolute_Pitching :1;
	unsigned int SwappingIsDebounced :1;
	unsigned int DemoMode :1;
	unsigned int IHaveAPlacedAutogun :1;
	unsigned int IsMovingInWater :1;
	unsigned int JetpackEnabled :1;
	unsigned int GrapplingHookEnabled :1;

	unsigned int MTrackerType;

	/* Patrick: 1/7/97 : for predator-type cloaking stuff */
	unsigned int cloakOn :1;
	unsigned int cloakPositionGivenAway :1;
	int FieldCharge;
	int cloakPositionGivenAwayTimer;
	int PlasmaCasterCharge;
	/* KJL 99/2/3 - Cloaking Effectiveness 
	ranges from 0 (useless) to ONE_FIXED (practically invisible) */
	int CloakingEffectiveness; 
	
	// John 28/7/97 Game Flow stuff
	int UNUSED_Enum_CurrentMission;
	unsigned long UNUSED_StateChangeObjectFlags;

	/* Encumberance */
	ENCUMBERANCE_STATE Encumberance;
	STRATEGYBLOCK *MyFaceHugger;
	STRATEGYBLOCK *MyCorpse;
	int tauntTimer;
	int soundHandle;
	/* Why no 2, you ask? */
	int soundHandle3;
	/* Because '3' is always crackling fire, for *
	 * netghosts and corpses. Really, 2 should be*
	 * the voice and 1 should be weapon use.     */
	int soundHandle4;
	/* For the splash. */
	int soundHandle5;
	/* For the jetpack. */

	int soundHandleForPredatorCloakDamaged;
	/* the above seemed better than soundHandle5 :) */

	HMODELCONTROLLER HModelController;
	int incidentFlag;
	int incidentTimer;
	int fireTimer;
	int invulnerabilityTimer;

} PLAYER_STATUS;	

#define TAUNT_LENGTH (ONE_FIXED<<1)
#define PLAYER_ON_FIRE_TIME	(ONE_FIXED*20)

#define STARTOFGAME_MARINE_HEALTH (100*65536) /* ie. 100 in 16.16 notation */
#define STARTOFGAME_MARINE_ENERGY (100*65536) /* ie. 100 in 16.16 notation */
#define STARTOFGAME_MARINE_ARMOUR (100*65536) /* ie. 100 in 16.16 notation */

/* Patrick 22/8/97------------------------------------------------
Cloaking stuff
------------------------------------------------------------------*/

#define PLAYERCLOAK_MAXENERGY 					(30*ONE_FIXED) /* fixed point seconds */
#define PLAYERCLOAK_RECHARGEFACTOR				(4) /* ... times slower than discharge */
#define PLAYERCLOAK_POSTIONGIVENAWAYTIME		(ONE_FIXED>>2) /*(2*ONE_FIXED) fixed point seconds */
#define PLAYERCLOAK_THRESHOLD					(5*ONE_FIXED)
#define PLAYERCLOAK_POWERON_DRAIN				(2*ONE_FIXED)
#define PLAYERCLOAK_DRAIN_FACTOR				(4)

/* Moved mere from player.c, CDF 23/4/98 */

extern PLAYER_STATUS* PlayerStatusPtr;


/******************** SIMPLE ANIMATIONS ********************/

typedef struct simpleanimbehaviour
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	TXACTRLBLK *tacbSimple;

}SIMPLE_ANIM_BEHAV_BLOCK;

typedef struct simple_anim_tools_template
{
	int shape_num;
	MREF my_module;
	char nameID[SB_NAME_LENGTH];
} SIMPLE_ANIM_TOOLS_TEMPLATE;	


/**********************************************************/
/**********************DOORS*******************************/

typedef enum{					 /* this may be flags*/
	I_door_opening,
	I_door_closing,
	I_door_open,
	I_door_closed,

} DOOR_STATES;




/********************  PROXIMITY DOORS ********************/



typedef struct ProxDoorBehaviourType
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	int door_state;
	MORPHCTRL *PDmctrl;

	/*---- Patrick 1/1/97 ----- 
	added for far ai stratgies 
	--------------------------*/
	int	alienTimer;
	unsigned int alienTrigger :1;
	unsigned int marineTrigger :1;
	unsigned int triggeredByMarine :1;

	/*---- Roxby 1/1/97 ----- 
	Added so that another door can lock
	this door closed
	--------------------------*/

	BOOL lockable_door;
	BOOL door_locked;
	char target_name[SB_NAME_LENGTH];
	STRATEGYBLOCK* door_lock_target;
  int SoundHandle;  
  int doorType;      // Used to determine door sound type  

	int door_opening_speed;
	int door_closing_speed;
} PROXDOOR_BEHAV_BLOCK;

typedef struct prox_door_tools_template
{
	BOOL has_lock_target;
	char target_name [SB_NAME_LENGTH];
	MREF my_module;
	int shape_open;
	int shape_closed;
	char nameID[SB_NAME_LENGTH];
	BOOL door_is_locked;

	int door_opening_speed;
	int door_closing_speed;
} PROX_DOOR_TOOLS_TEMPLATE;




/* Structures for Stat Initialisation */

typedef enum {
	I_NPC_Civilian=0,
	I_NPC_FaceHugger,
	I_NPC_ChestBurster,
	I_NPC_Alien,
	I_NPC_Xenoborg,
	I_NPC_Marine,
	I_NPC_PredatorAlien,
	I_NPC_SFMarine,
	I_NPC_Predator,
	I_NPC_PraetorianGuard,
	I_NPC_AlienQueen,
	I_NPC_DefaultInanimate,
	I_PC_Alien_Easy,
	I_PC_Marine_Easy,
	I_PC_Predator_Easy,
	I_PC_Alien_Medium,
	I_PC_Marine_Medium,
	I_PC_Predator_Medium,
	I_PC_Alien_Hard,
	I_PC_Marine_Hard,
	I_PC_Predator_Hard,
	I_PC_Alien_Impossible,
	I_PC_Marine_Impossible,
	I_PC_Predator_Impossible,
	I_PC_Alien_MaxStats,
	I_NPC_SentryGun,
	I_NPC_Android,
	I_NPC_End,
} NPC_TYPES;

typedef struct {
	NPC_TYPES Type;
	//int StartingHealth;
	//int StartingArmour;
	//SBHEALTHFLAGS SB_H_flags;
	DAMAGEBLOCK StartingStats;
} NPC_DATA;

/* Interface function! */

extern NPC_DATA *GetThisNpcData(NPC_TYPES NpcType);

/********************************************************/
/*******************  Database behaviour************/

/* 
	we will need to include an enum into another menu
	graphic which contains the text to overlay onto
	the menugraphics
*/

typedef struct database
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	int num;
}DATABASE_BLOCK;

typedef struct database_template
{
	int num;
	VECTORCH position;
	EULER orientation;
	int shape_num;
} DATABASE_TOOLS_TEMPLATE;

extern void DatabaseMenus(DATABASE_BLOCK* db);

/***********************************************************/
/****************** externs for bh_types.c ******************/
/* functions*/

extern void AssignAllSBNames();
extern void AssignRunTimeBehaviours(STRATEGYBLOCK* sbptr); 
extern void EnableBehaviourType(STRATEGYBLOCK* sbptr, AVP_BEHAVIOUR_TYPE sb_type, void *bhdata);
extern void ExecuteBehaviour(STRATEGYBLOCK* sbptr);
extern void ObjectBehaviours(void);
extern void RequestState(STRATEGYBLOCK* sb, int message, STRATEGYBLOCK * SBRequester);
extern BOOL GetState(STRATEGYBLOCK* sb);
extern void RemoveBehaviourStrategy(STRATEGYBLOCK* sbptr);

extern void UnlockThisProxdoor(STRATEGYBLOCK* sbptr);

extern void FindMaxZXandYAverages(VECTORCH* vect, SHAPEHEADER* shapeptr);

extern DISPLAYBLOCK *MakeObject(AVP_BEHAVIOUR_TYPE bhvr, VECTORCH *positionPtr);

extern void SetupPlayerAutoGun();

#ifdef __cplusplus

	};

#endif


#endif











