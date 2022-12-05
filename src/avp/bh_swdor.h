/*------------------------------Patrick 12/3/97-----------------------------------
  Header for Switch Operated Doors
  --------------------------------------------------------------------------------*/

typedef struct switch_door_behaviour_type
{
	AVP_BEHAVIOUR_TYPE myBehaviourType;		/* just for testing system integrity */
	DOOR_STATES doorState;
	MORPHCTRL *morfControl;
	char linkedDoorName[SB_NAME_LENGTH];
	STRATEGYBLOCK* linkedDoorPtr;
	int openTimer;
	unsigned int requestOpen :1;
	unsigned int requestClose :1;
	int SoundHandle;
	int doorType;      // Used to determine door sound type  

} SWITCH_DOOR_BEHAV_BLOCK;

typedef struct switch_door_tools_template
{
	BOOL state;
	MREF myModule;
	int shapeOpen;
	int shapeClosed;
	char linkedDoorName[SB_NAME_LENGTH];
	char nameID[SB_NAME_LENGTH];
} SWITCH_DOOR_TOOLS_TEMPLATE;

#define DOOR_OPENSLOWSPEED		(1<<16)
#define DOOR_OPENFASTSPEED		(1<<20)
#define DOOR_CLOSESLOWSPEED		(1<<17)
#define DOOR_CLOSEFASTSPEED		(1<<20)
#define DOOR_FAROPENTIME		(ONE_FIXED<<2) 	/* 4 seconds: DO NOT CHANGE THIS OR AI MAY NOT WORK*/
#define DOOR_OPENDISTANCE		(5000) 			/* mm */

extern void InitialiseSwitchDoor(void* bhdata, STRATEGYBLOCK* sbptr);
extern void SwitchDoorBehaviour(STRATEGYBLOCK* sbptr);
extern void OpenDoor(MORPHCTRL *mctrl, int speed);
extern void CloseDoor(MORPHCTRL *mctrl, int speed);
