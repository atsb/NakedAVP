#ifndef _bh_spcl_h
#define _bh_spcl_h 1

#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"


void * InitXenoMorphRoom (void * bhdata, STRATEGYBLOCK * sbptr);
void XenoMorphRoomBehaviour (STRATEGYBLOCK * sbptr);


typedef struct xeno_morph_room_tools_template
{
	int MainShape;
	
	int ShutShape;
	int WallsOutShape;
	int ProbesInShape;

	MREF my_module;
	char nameID[SB_NAME_LENGTH];
	
	char doorID[SB_NAME_LENGTH];

} XENO_MORPH_ROOM_TOOLS_TEMPLATE;

typedef enum xeno_morph_room_state
{
	XMRS_Idle,
	XMRS_SafetyChecks,
	XMRS_EnclosingPlayer,
	XMRS_WallsOut,
	XMRS_ProbesIn,
	XMRS_FadeToBlack,
	XMRS_Process,
	XMRS_Return,
	XMRS_ReleasePlayer,
	XMRS_Finished,

} XENO_MORPH_ROOM_STATE;


typedef struct xeno_morph_room_data
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	TXACTRLBLK *tacb;

	MORPHCTRL *XMR_Mctrl;
	
	XENO_MORPH_ROOM_STATE XMR_State;
	
	int MainShape;
	int ShutShape;
	int WallsOutShape;
	int ProbesInShape;
	
	int timer;

	int ** pis_items_str;
	int ** pis_sht_str;
	
	char doorID[SB_NAME_LENGTH];
	STRATEGYBLOCK* DoorToRoom;

} XENO_MORPH_ROOM_DATA;

#endif
