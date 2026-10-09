#ifndef _ltfx_exp_h
#define _ltfx_exp_h 1

#ifdef __cplusplus

extern "C"
{

#endif

typedef enum 
{

	LFX_RandomFlicker,
	LFX_Strobe,
	LFX_Switch,
	LFX_FlickySwitch,
	
} LIGHT_FX_TYPE;

typedef enum
{
	
	LFXS_LightOn,
	LFXS_LightOff,
	LFXS_LightFadingUp,
	LFXS_LightFadingDown,
	LFXS_Flicking,
	LFXS_NotFlicking,
	
} LIGHT_FX_STATE;

typedef struct
{
	
	unsigned long type;
	unsigned long init_state;
	
	unsigned long fade_up_speed;
	unsigned long fade_down_speed;
	
	unsigned long post_fade_up_delay;
	unsigned long post_fade_down_delay;

} LightFXData;

#ifdef __cplusplus

};

#endif

#endif
