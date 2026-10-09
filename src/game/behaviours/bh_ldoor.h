/******************** LIFT DOORS ********************/

/* 	
	lift doors do not have to look at the environment for
	triggers. they wait for the controlling lift block to
	say open or closed. EXCEPT when the door is open it
	will not close if some other object is in its module
*/


typedef struct lift_door_behaviour_type
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	DOOR_STATES door_state;
	MORPHCTRL *PDmctrl;

	DOOR_STATES request_state;

	int SoundHandle;  

	/*---- Patrick 1/1/97 ----- 
	added for far ai stratgies 
	--------------------------*/
	int door_opening_speed;
	int door_closing_speed;
} LIFT_DOOR_BEHAV_BLOCK;


typedef struct lift_door_tools_template
{
	BOOL state;
	MREF my_module;
	int shape_open;
	int shape_closed;
	char nameID[SB_NAME_LENGTH];

	int door_opening_speed;
	int door_closing_speed;
} LIFT_DOOR_TOOLS_TEMPLATE;


extern void* LiftDoorBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void LiftDoorBehaveFun(STRATEGYBLOCK* sbptr);
