#ifndef _bhlnksw_h_
#define _bhlnksw_h_ 1

#include "track.h"

#ifdef __cplusplus

	extern "C" {

#endif


void* LinkSwitchBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
void LinkSwitchBehaveFun(STRATEGYBLOCK* sbptr);

extern int LinkSwitchGetSynchData(STRATEGYBLOCK* sbptr);
extern void LinkSwitchSetSynchData(STRATEGYBLOCK* sbptr,int status);

typedef enum link_switch_mode
{
	I_lswitch_timer,
	I_lswitch_wait,
	I_lswitch_toggle,
	I_lswitch_moving,
	I_lswitch_SELFDESTRUCT,

} LSWITCH_MODE;

typedef enum link_switch_req_states
{
	linkswitch_no_request,
	linkswitch_request_on,
	linkswitch_request_off,
}LINK_SWITCH_REQUEST_STATE;

typedef enum lswitch_display_types
{
	linkswitch_no_display,
	linkswitch_animate_me,
	linkswitch_move_me,
	linkswitch_animate_and_move_me,
} LSWITCH_DISPLAY_TYPES;

typedef enum ls_move_dir
{
	ls_start_to_end,
	ls_end_to_start,
} LS_MOVE_DIR;


typedef struct lswitch_item
{
	STRATEGYBLOCK * bswitch;
	char bs_name [SB_NAME_LENGTH];
	
} LSWITCH_ITEM;

typedef struct link_switch_target
{
	char name[SB_NAME_LENGTH];
	int request_message;
	STRATEGYBLOCK* sbptr;
}LINK_SWITCH_TARGET;

typedef struct link_switch
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	LINK_SWITCH_REQUEST_STATE request;

	BOOL system_state;
	
	BOOL state;
	LSWITCH_MODE ls_mode;
	
	int num_targets;
	
	LINK_SWITCH_TARGET* ls_targets;
	
	int time_for_reset;	// constant
	int timer;
	
	int security_clerance; // what the plyer has to be to use this switch
	
	int num_linked_switches;
	LSWITCH_ITEM* lswitch_list ;
	
	// stuff for showing how the switch displays its state
	
	LSWITCH_DISPLAY_TYPES ls_dtype;
	
	TXACTRLBLK *ls_tac; // animations

	// or track
	TRACK_CONTROLLER* ls_track;
	
	
	BOOL new_state;
	int new_request;
	
	LSWITCH_MODE mode_store;
	
	// SELF DESTRUCT SEQUENCE STUFF
	
	BOOL IS_SELF_DESTRUCT;
  
  	int soundHandle;	

	BOOL triggered_last;

	int switch_flags;
	VECTORCH trigger_volume_min;//for switches that can be set off by walking
	VECTORCH trigger_volume_max;//into a given area
	
	unsigned int switch_always_on:1;
	unsigned int switch_off_message_same:1;
	unsigned int switch_off_message_none:1;

	int TimeUntilNetSynchAllowed; 

}LINK_SWITCH_BEHAV_BLOCK;

typedef struct link_switch_tools_template
{
	VECTORCH position;
	EULER orientation;
	

	BOOL rest_state;
	int mode;
	int time_for_reset;
	int security_clearance;
	
	int num_targets;
	LINK_SWITCH_TARGET * targets;
	
	int shape_num;
	
	TRACK_CONTROLLER* track;
	
	
	char nameID[SB_NAME_LENGTH];
	
	int num_linked_switches;
	SBNAMEBLOCK* switchIDs; 

	int switch_flags;
	VECTORCH trigger_volume_min;//for switches that can be set off by walking
	VECTORCH trigger_volume_max;//into a given area

	unsigned int switch_always_on:1;
	unsigned int switch_off_message_same:1;
	unsigned int switch_off_message_none:1;
} LINK_SWITCH_TOOLS_TEMPLATE;


#ifdef __cplusplus

	};

#endif


#endif
