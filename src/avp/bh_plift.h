#include "track.h"
/*------------------------------Patrick 14/3/97-----------------------------------
  Header for platform lift stuff
  --------------------------------------------------------------------------------*/

typedef enum platformlift_states
{
	PLBS_AtRest,
	PLBS_Activating,
	PLBS_GoingUp,
	PLBS_GoingDown,
} PLATFORMLIFT_STATES;

typedef struct platformlift_behaviour_type
{
	VECTORCH homePosition;
	int upHeight;
	int downHeight;
	int activationDelayTimer;
	PLATFORMLIFT_STATES	state;

	TRACK_SOUND* sound;
	TRACK_SOUND* start_sound;
	TRACK_SOUND* end_sound;
	
	// A switch will set these flags on AssignSBNames
	
	BOOL Enabled; 
	BOOL OneUse; //if set ,lift becomes disabled after changing position once

	int netMsgCount;

} PLATFORMLIFT_BEHAVIOUR_BLOCK;

typedef struct platformlift_tools_template
{
	struct vectorch position;
	struct euler orientation;
	int shapeIndex;
	int travel;	/* vertical distance from start position to end position (down = +ve) */
	BOOL Enabled;
	BOOL OneUse;
	char nameID[SB_NAME_LENGTH];

	TRACK_SOUND* sound;
	TRACK_SOUND* start_sound;
	TRACK_SOUND* end_sound;

} PLATFORMLIFT_TOOLS_TEMPLATE;

#define PLATFORMLIFT_SPEED				5000 		/* mm/s */
#define PLATFORMLIFT_ACTIVATIONTIME		((ONE_FIXED*3)>>1)	/* fixed point seconds */
#define PLATFORMLIFT_NUMNETMESSAGES			5

void InitialisePlatformLift(void* bhdata, STRATEGYBLOCK *sbPtr);
void PlatformLiftBehaviour(STRATEGYBLOCK *sbPtr);

void ActivatePlatformLift(STRATEGYBLOCK *sbPtr);
void SendPlatformLiftUp(STRATEGYBLOCK *sbPtr);
void SendPlatformLiftDown(STRATEGYBLOCK *sbPtr);
void StopPlatformLift(STRATEGYBLOCK *sbPtr);
