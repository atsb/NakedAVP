#ifndef _bh_ltfx_h
#define _bh_ltfx_h 1

#include "ltfx_exp.h"


typedef struct light_fx_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;

	LIGHT_FX_TYPE type;
	LIGHT_FX_STATE current_state;
	
    uint32_t fade_up_speed;
    uint32_t fade_down_speed;
	
    uint32_t post_fade_up_delay;
    uint32_t post_fade_down_delay;
	
    uint32_t fade_up_speed_multiplier;
    uint32_t fade_down_speed_multiplier;
	
    uint32_t post_fade_up_delay_multiplier;
    uint32_t post_fade_down_delay_multiplier;
	
    int32_t multiplier;
    uint32_t timer;
    uint32_t timer2;
	
    int32_t time_to_next_flicker_state;

	TXACTRLBLK *anim_control;
} LIGHT_FX_BEHAV_BLOCK;

typedef struct light_fx_tools_template
{

	LightFXData light_data;
		
	char nameID[SB_NAME_LENGTH];
	MREF my_module;

} LIGHT_FX_TOOLS_TEMPLATE;

void * LightFXBehaveInit (void * bhdata, STRATEGYBLOCK* sbptr);
void LightFXBehaveFun (STRATEGYBLOCK* sbptr);


#endif
