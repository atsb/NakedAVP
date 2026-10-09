#ifndef _bh_binsw_h_
#define _bh_binsw_h_ 1

#include "3dc.h"
#include "track.h"

#ifdef __cplusplus

	extern "C" {

#endif


extern void* BinarySwitchBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void BinarySwitchBehaveFun(STRATEGYBLOCK* sbptr);

extern int BinarySwitchGetSynchData(STRATEGYBLOCK* sbptr);
extern void BinarySwitchSetSynchData(STRATEGYBLOCK* sbptr,int status);

/******************** BinarySwitch ***********************/

// enum to decxribe how the switch works
// action is always dependent on the SB block of the target

typedef enum binary_switch_mode
{
	I_bswitch_timer,
	I_bswitch_wait,
	I_bswitch_toggle,
	I_bswitch_moving,
	I_bswitch_time_delay,
	I_bswitch_time_delay_autoexec,//timer starts as soon as game starts

} BSWITCH_MODE;

typedef enum binary_switch_req_states
{
	I_no_request,
	I_request_on,
	I_request_off,
}BINARY_SWITCH_REQUEST_STATE;

typedef enum bswitch_display_types
{
	binswitch_no_display,
	binswitch_animate_me,
	binswitch_move_me,
	binswitch_animate_and_move_me,
} BSWITCH_DISPLAY_TYPES;

typedef enum bs_move_dir
{
	bs_start_to_end,
	bs_end_to_start,
} BS_MOVE_DIR;

// require three states for rest fso we can ignore 


#define SwitchFlag_UseTriggerVolume 0x00000001 //switch triggered by walking into trigger volume

typedef struct binary_switch
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	BINARY_SWITCH_REQUEST_STATE request;
	BOOL state;
	BSWITCH_MODE bs_mode;
	
	int num_targets;
	SBNAMEBLOCK * target_names;
	int * request_messages;
	
	STRATEGYBLOCK ** bs_targets;
	
	
	int time_for_reset;	// constant
	int timer;

	// stuff for showing how the switch displays its state

	BSWITCH_DISPLAY_TYPES bs_dtype;
	
	TXACTRLBLK *bs_tac; // animations

	
	TRACK_CONTROLLER* bs_track;
	// or positions
	
	
	BSWITCH_MODE mode_store;

	BOOL new_state;
	int new_request;
	
	int security_clerance; // what the plyer has to be to use this switch

	int switch_flags;
	VECTORCH trigger_volume_min;//for switches that can be set off by walking
	VECTORCH trigger_volume_max;//into a given area

  int soundHandle;
	
	BOOL triggered_last;
	
	unsigned int switch_off_message_same:1;
	unsigned int switch_off_message_none:1;

	int TimeUntilNetSynchAllowed; 
	
		 
}BINARY_SWITCH_BEHAV_BLOCK;

typedef struct bin_switch_tools_template
{
	VECTORCH position;
	EULER orientation;

	int mode;
	int time_for_reset;
	int security_clearance;
	
	int num_targets;
	SBNAMEBLOCK * target_names;
	int* request_messages;

	int shape_num;
	
	char nameID[SB_NAME_LENGTH];

	TRACK_CONTROLLER* track;
	
	int switch_flags;
	VECTORCH trigger_volume_min;//for switches that can be set off by walking
	VECTORCH trigger_volume_max;//into a given area

	unsigned int starts_on:1;
	unsigned int switch_off_message_same:1;
	unsigned int switch_off_message_none:1;


} BIN_SWITCH_TOOLS_TEMPLATE;



#ifdef __cplusplus

	};

#endif


#endif
