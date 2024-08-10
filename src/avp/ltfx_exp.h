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
	
    uint32_t type;
    uint32_t init_state;
	
    uint32_t fade_up_speed;
    uint32_t fade_down_speed;
	
    uint32_t post_fade_up_delay;
    uint32_t post_fade_down_delay;

} LightFXData;

#ifdef __cplusplus

};

#endif

#endif
