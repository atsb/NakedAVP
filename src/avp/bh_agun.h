// RWH - the autogun code

/* CDF 25/7/98 - Heaven help us. */

extern void AutoGunBehaveFun(STRATEGYBLOCK* ag_sbptr);
extern void AutoGunBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
//extern void CreatePlayerAutogun(void);

void MakeSentrygunNear(STRATEGYBLOCK *sbPtr);
void MakeSentrygunFar(STRATEGYBLOCK *sbPtr);
int AGunSight_FrustrumReject(VECTORCH *localOffset);
void AGunIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,VECTORCH *incoming);

typedef enum {
	I_disabled,
	I_tracking,
	I_inactive,
}AG_STATE;

typedef struct autogun_behaviour_block
{
	AG_STATE behaviourState;
	int stateTimer;

	HMODELCONTROLLER HModelController;
	DELTA_CONTROLLER *gun_pan;
	DELTA_CONTROLLER *gun_tilt;

	STRATEGYBLOCK *Target;
	char Target_SBname[SB_NAME_LENGTH];
	/* A level of indirection for better control. */
	VECTORCH targetTrackPos;

	int Gun_Pan;
	int Gun_Tilt;
	DISPLAYBLOCK *GunFlash;

	int incidentFlag;
	int incidentTimer;

	int ammo;
	int roundsFired;
	int volleyFired;

  	int soundHandle;
  	int soundHandle2;
	int Firing;
	int WhirrSoundOn;
	int Drama;

  	unsigned int createdByPlayer:1;
	unsigned int gunpandir	:1;
	unsigned int guntiltdir	:1;
	unsigned int IAmFar	:1;

	unsigned int OnTarget	:1;
	unsigned int OnTarget_LastFrame	:1;

	char death_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* death_target_sbptr;
	int death_target_request;

}AUTOGUN_STATUS_BLOCK;


typedef struct autogun_tools_template
{
	VECTORCH position;
	EULER orientation;
	
	int ammo;
	int shapenum;
	int startInactive;

	char nameID[SB_NAME_LENGTH];

	char death_target_ID[SB_NAME_LENGTH]; 
	int death_target_request;

}AUTOGUN_TOOLS_TEMPLATE;

#define SGUN_PITCH_GIMBALL		(1024)
#define SGUN_PAN_GIMBALL		(1024)
#define AGUN_NEAR_VIEW_WIDTH	500		/* mm */

#define AGUN_ROF		(30)
#define AGUN_VOLLEYSIZE	(15)
