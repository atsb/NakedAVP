#ifndef _bh_ltfx_h
#define _bh_ltfx_h 1

#include "ltfx_exp.h"


typedef struct light_fx_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;

	LIGHT_FX_TYPE type;
	LIGHT_FX_STATE current_state;
	
	unsigned long fade_up_speed;
	unsigned long fade_down_speed;
	
	unsigned long post_fade_up_delay;
	unsigned long post_fade_down_delay;
	
	unsigned long fade_up_speed_multiplier;
	unsigned long fade_down_speed_multiplier;
	
	unsigned long post_fade_up_delay_multiplier;
	unsigned long post_fade_down_delay_multiplier;
	
	signed long multiplier;
	unsigned long timer;
	unsigned long timer2;
	
	signed long time_to_next_flicker_state;

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
