#ifndef _bh_videoscreen_h
#define _bh_videoscreen_h

typedef struct video_screen_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	BOOL Indestructable;

	TXACTRLBLK *inan_tac;//for video screens with anims on them

	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* destruct_target_sbptr;

}VIDEO_SCREEN_BEHAV_BLOCK;


typedef struct toolsdata_video_screen
{
	struct vectorch position;
	struct euler orientation;
	int shapeIndex;	
	char nameID[SB_NAME_LENGTH];
	int integrity; // 0-20 (>20 = indestructable)

	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];

	
}TOOLS_DATA_VIDEO_SCREEN;

void* InitVideoScreen(void* bhdata,STRATEGYBLOCK *sbPtr);
void VideoScreenBehaviour(STRATEGYBLOCK *sbPtr);
void VideoScreenIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);

#endif
