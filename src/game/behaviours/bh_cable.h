#ifndef _bh_cable_h
#define _bh_cable_h 1

#ifdef __cplusplus

	extern "C" {

#endif

extern void*  PowerCableBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void  PowerCableBehaveFun(STRATEGYBLOCK* sbptr);


#define CABLE_HEALTH_DISTANCE 2000

typedef struct power_cable_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	
	char nameID[SB_NAME_LENGTH];
	
	VECTORCH position;

	int max_charge;	 //health value in fixed point
	int current_charge;	//ditto
	int recharge_rate;	//recharge rate per second
}POWER_CABLE_BEHAV_BLOCK;

typedef struct power_cable_tools_template
{
	char nameID[SB_NAME_LENGTH];
	VECTORCH position;
	int max_charge;
	int current_charge;
	int recharge_rate; 
}POWER_CABLE_TOOLS_TEMPLATE;





#ifdef __cplusplus

	};

#endif


#endif
