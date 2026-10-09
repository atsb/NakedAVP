typedef enum light_state
{
	Light_State_Standard,
	Light_State_Broken,	
	Light_State_StrobeUp,
	Light_State_StrobeDown,
	Light_State_StrobeUpDelay,
	Light_State_StrobeDownDelay,
	Light_State_Flicker,

}LIGHT_STATE;

typedef enum light_on_off_state
{
	Light_OnOff_Off,
	Light_OnOff_On,
	Light_OnOff_FadeOff,
	Light_OnOff_FadeOn,
	Light_OnOff_Flicker,

}LIGHT_ON_OFF_STATE;

typedef enum light_type
{
	Light_Type_Standard,
	Light_Type_Strobe,
	Light_Type_Flicker,
}LIGHT_TYPE;

typedef enum light_on_off_type
{
	Light_OnOff_Type_Standard,
	Light_OnOff_Type_Fade,
	Light_OnOff_Type_Flicker,
}LIGHT_ON_OFF_TYPE;


typedef struct placed_light_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	BOOL Indestructable;

	TXACTRLBLK *inan_tac;//for lights with anims on them

	LIGHTBLOCK* light;

	LIGHT_STATE state;
	LIGHT_ON_OFF_STATE on_off_state;
	int sequence;	//texture animation sequence

	int colour_red;  //colour for fade up state
	int colour_green;
	int colour_blue;
	int colour_diff_red;  //difference from up colour to down colour
	int colour_diff_green;
	int colour_diff_blue;
	
	int fade_up_time;
	int fade_down_time;
	int up_time;
	int down_time;
	
	int timer;
	int on_off_timer;
	int flicker_timer;

	LIGHT_TYPE type;
	LIGHT_ON_OFF_TYPE on_off_type;

	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];
	STRATEGYBLOCK* destruct_target_sbptr;

	VECTORCH corona_location;

	int startingHealth; //for network games
	int startingArmour;
	
	int has_broken_sequence:1; //is there a third texture sequence 
	int has_corona:1;
	int swap_colour_and_brightness_alterations:1;

}PLACED_LIGHT_BEHAV_BLOCK;

typedef struct toolsdata_placed_light
{
	struct vectorch position;
	struct euler orientation;
	int shapeIndex;	/* for john */
	char nameID[SB_NAME_LENGTH];

	int mass; // Kilos??
	int integrity; // 0-20 (>20 = indestructable)

	LIGHTBLOCK* light;
	
	unsigned int static_light:1;

	int sequence;

	int colour_red;
	int colour_green;
	int colour_blue;
	int colour_diff_red;
	int colour_diff_green;
	int colour_diff_blue;
	
	int fade_up_time;
	int fade_down_time;
	int up_time;
	int down_time;
	
	int timer;


	LIGHT_STATE state;
	LIGHT_ON_OFF_STATE on_off_state;
	LIGHT_TYPE type;
	LIGHT_ON_OFF_TYPE on_off_type;

	int destruct_target_request;
	char destruct_target_ID[SB_NAME_LENGTH];
	int swap_colour_and_brightness_alterations:1;
	
}TOOLS_DATA_PLACEDLIGHT;





void* InitPlacedLight(void* bhdata,STRATEGYBLOCK *sbPtr);
void PlacedLightBehaviour(STRATEGYBLOCK *sbPtr);

void MakePlacedLightNear(STRATEGYBLOCK *sbPtr);

void PlacedLightIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);

void SendRequestToPlacedLight(STRATEGYBLOCK* sbptr,BOOL state,int extended_data);

void RespawnLight(STRATEGYBLOCK *sbPtr);
void KillLightForRespawn(STRATEGYBLOCK *sbPtr);
