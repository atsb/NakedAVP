#ifndef _bh_deathvol_h
#define _bh_deathvol_h 1

#ifdef __cplusplus

	extern "C" {

#endif

extern void*  DeathVolumeBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void  DeathVolumeBehaveFun(STRATEGYBLOCK* sbptr);


typedef struct death_volume_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	VECTORCH volume_min;
	VECTORCH volume_max;
	unsigned int damage_per_second; //0 means infinite damage (a proper death volume - bwa ha ha.)
	unsigned int active :1;
	unsigned int collision_required :1;
}DEATH_VOLUME_BEHAV_BLOCK;

typedef struct death_volume_tools_template
{
	char nameID[SB_NAME_LENGTH];
	VECTORCH volume_min;
	VECTORCH volume_max;
	unsigned int damage_per_second;
	unsigned int active :1;
	unsigned int collision_required :1;
}DEATH_VOLUME_TOOLS_TEMPLATE;





#ifdef __cplusplus

	};

#endif


#endif
