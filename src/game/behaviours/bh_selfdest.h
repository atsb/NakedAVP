#ifndef _bh_selfdest_h
#define _bh_selfdest_h 1

#ifdef __cplusplus

	extern "C" {

#endif

extern void*  SelfDestructBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void  SelfDestructBehaveFun(STRATEGYBLOCK* sbptr);


typedef struct self_destruct_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	int timer; //in fixed point seconds
	BOOL active;
}SELF_DESTRUCT_BEHAV_BLOCK;

typedef struct self_destruct_tools_template
{
	char nameID[SB_NAME_LENGTH];
	int timer;
}SELF_DESTRUCT_TOOLS_TEMPLATE;





#ifdef __cplusplus

	};

#endif


#endif
