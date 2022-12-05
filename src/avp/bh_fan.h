#ifndef _bh_fan_h_
#define _bh_fan_h_ 1


#ifdef __cplusplus

	extern "C" {

#endif

void* FanBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
void  FanBehaveFun(STRATEGYBLOCK* sbptr);



typedef enum fan_states
{
	fan_state_go,
	fan_state_stop,
}FAN_STATE;


typedef struct fan_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;

	FAN_STATE state;

	TRACK_CONTROLLER* track;
	
	int speed_up_mult; //one over time take to get to full speed
	int slow_down_mult;	//one over time take to stop
	int speed_mult;	//0 to one_fixed : current speed relative to full speed
		
	VECTORCH fan_wind_direction; //normalised vector
	int fan_wind_strength; //fixed point multiplier for fan at full speed
	
	int wind_speed;//fixed point multiplier , taking the fan's current speed into account

}FAN_BEHAV_BLOCK;


typedef struct fan_tools_template
{
	char nameID[SB_NAME_LENGTH];
	int shape_num;
	
	VECTORCH position;
	EULER orientation;

	int speed_up_mult; //one over time take to get to full speed
	int slow_down_mult;	//one over time take to stop

	TRACK_CONTROLLER* track;

	VECTORCH fan_wind_direction; //normalised vector
	int fan_wind_strength; //fixed point multiplier
}FAN_TOOLS_TEMPLATE;


#ifdef __cplusplus

	};

#endif


#endif
