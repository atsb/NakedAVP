#ifndef _bh_track_h_
#define _bh_track_h_ 1

#include "track.h"

#ifdef __cplusplus

	extern "C" {

#endif

extern void* TrackObjectBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void  TrackObjectBehaveFun(STRATEGYBLOCK* sbptr);
extern void  TrackObjectIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);

extern int TrackObjectGetSynchData(STRATEGYBLOCK* sbptr);
extern void TrackObjectSetSynchData(STRATEGYBLOCK* sbptr,int status);

typedef enum track_object_req_states
{
	track_no_request,
	track_request_start,
	track_request_stop,
	track_request_startforward,
	track_request_startbackward,
}TRACK_OBJECT_REQUEST_STATE;



#ifndef TrackRequestFlag_ActiveForward
	#define TrackRequestFlag_ActiveForward 0x00000001
	#define TrackRequestFlag_ActiveBackward 0x00000002
	#define TrackRequestFlag_OppositeBackward 0x00000004
#endif
typedef struct track_point_target 
{

	char target_name [SB_NAME_LENGTH];
	STRATEGYBLOCK * target_sbptr;
	int request;
	int flags;

}TRACK_POINT_TARGET;

typedef struct special_track_point
{
	int track_point_no;
	int num_targets;
	
	TRACK_POINT_TARGET* targets;

}SPECIAL_TRACK_POINT;

typedef struct track_object_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	BOOL Indestructable;

	TRACK_CONTROLLER* to_track;

	TRACK_OBJECT_REQUEST_STATE request;

	TXACTRLBLK *to_tac;//for objects with anims on them

	int num_special_track_points;
	SPECIAL_TRACK_POINT* special_track_points;

	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* destruct_target_sbptr;

	int TimeUntilNetSynchAllowed; 
}TRACK_OBJECT_BEHAV_BLOCK;


typedef struct track_object_tools_template
{
	char nameID[SB_NAME_LENGTH];
	int shape_num;
	
	TRACK_CONTROLLER* track;

	VECTORCH position;
	EULER orientation;

	int num_special_track_points;
	SPECIAL_TRACK_POINT* special_track_points;


	int integrity; // 0-20 (>20 = indestructable)
	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];

}TRACK_OBJECT_TOOLS_TEMPLATE;


#ifdef __cplusplus

	};

#endif


#endif
