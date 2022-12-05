extern void LiftBehaveFun(STRATEGYBLOCK* sbptr);
extern void * LiftBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void CleanUpLiftControl();

extern void TeleportContents(MODULE* new_pos, MODULE* old_pos,BOOL floor_switches_fixed);
extern BOOL BadGuyInModule();





/*********************** Lifts **************************/


typedef struct lift_stations
{
	char lift_call_switch_name[SB_NAME_LENGTH];
	STRATEGYBLOCK* lift_call_switch;
	char lift_door_name[SB_NAME_LENGTH];
	STRATEGYBLOCK* lift_door;
	char lift_floor_switch_name[SB_NAME_LENGTH];
	STRATEGYBLOCK* lift_floor_switch; // the floor switches teleport
	BOOL called; 
	char my_sb_name[SB_NAME_LENGTH];	// only used when envs change
	MODULE* lift_module;
	I_AVP_ENVIRONMENTS env; 	// tells us if we need a cd load
	int num_floor;						// not the floor num but the poition
														// in the array	
	int orient; 				//the facing of the lift (from 0 to 3)
	BOOL starting_station;		
												
	
}LIFT_STATION;	

typedef enum liftmotion{
	
	I_going_up,					/*** numbers go down (0 at surface **/
	I_going_down,				// numbers go up

}LIFT_MOTION;


typedef enum lift_ctrl_states
{
	I_ls_waiting,
	I_ls_closing_door,
	I_ls_moving,
	I_ls_opening_door,
	I_ls_delay_at_floor,

}LIFT_CTRL_STATES;

#define LIFT_FLOOR_DELAY ONE_FIXED*2  // two secs
#define LIFT_MOVE_DELAY ONE_FIXED*5   // three secs


typedef struct lift_control
{
	int num_stations;				// num staions for this lift
	LIFT_STATION** lift_stations;	// array of lift stations for this lift
	int dest_station;							// -1 when there is no floor
	int curr_station;					 		// 	tells us the lift pos
	int prev_station;							// where did we come from
	int delay_at_floor;				 		// tells us how long to stay at floor
	int delay_between_floors;	 		// tells us how long before teleport
	LIFT_MOTION motion;
	LIFT_CTRL_STATES state;
	BOOL floor_switches_fixed;					
	int SoundHandle; 

} LIFT_CONTROL_BLOCK;


typedef struct lift_behaviour
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	char control_sb_name[SB_NAME_LENGTH];
	STRATEGYBLOCK* control_sb;	
	LIFT_CONTROL_BLOCK* lift_control;
	LIFT_STATION lift_station;
	int controller;

} LIFT_BEHAV_BLOCK;
	
#define LiftFlag_Here			0x00000001
#define LiftFlag_Airlock		0x00000002
#define LiftFlag_NoTel			0x00000004 /*switches aren't teleported*/

typedef struct lift_tools_template
{
	// for behaviour block
	char control_sb_name[SB_NAME_LENGTH];
	int controller;
	
	// for control block
	int num_stations;
	
	// for station block	
	char call_switch_name[SB_NAME_LENGTH];
	char lift_door_name[SB_NAME_LENGTH];
	char lift_floor_switch_name[SB_NAME_LENGTH];
	char my_module_name[SB_NAME_LENGTH];
	int environment;
	int num_floor;
	int lift_flags;
	int orient;

	// for strategy block	
	MREF my_module;
	char nameID[SB_NAME_LENGTH];

} LIFT_TOOLS_TEMPLATE;


extern int RequestEnvChangeViaLift;
extern int RequestEnvChangeViaAirlock;
