#ifndef _bh_pargen_h
#define _bh_pargen_h 1

#ifdef __cplusplus

	extern "C" {

#endif
#include "track.h"

extern void*  ParticleGeneratorBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
extern void  ParticleGeneratorBehaveFun(STRATEGYBLOCK* sbptr);
extern void SendRequestToParticleGenerator(STRATEGYBLOCK* sbptr,BOOL state,int extended_data);

typedef enum particle_generator_type
{
	PARGEN_TYPE_SPARK,
	PARGEN_TYPE_STEAM,
	PARGEN_TYPE_BLACKSMOKE,
	PARGEN_TYPE_FLAME,
	PARGEN_TYPE_LAST

}PARTICLE_GENERATOR_TYPE;


typedef struct particle_generator_behav_block
{
	AVP_BEHAVIOUR_TYPE bhvr_type;
	
	VECTORCH position;
	MATRIXCH orientation; 

	VECTORCH relative_position;	 //relative to parent object (if it exists)
	MATRIXCH relative_orientation; //relative to parent object (if it exists)
	
	PARTICLE_GENERATOR_TYPE type;//particle generator type

	TRACK_SOUND* sound; //not actually a track , but it will do

	STRATEGYBLOCK* parent_sbptr; 

	int timer; //time left to next particle burst

	int frequency; //time between bursts
	int probability; //probability of particles (0 - 65536)
	int speed;	//mm/second

	unsigned int active :1;	//is generator currently active

}PARTICLE_GENERATOR_BEHAV_BLOCK;

typedef struct particle_generator_tools_template
{
	char nameID[SB_NAME_LENGTH];
	char parentID[SB_NAME_LENGTH];
	VECTORCH position;
	MATRIXCH orientation; 

	PARTICLE_GENERATOR_TYPE type;//particle generator type
	
	TRACK_SOUND* sound; //not actually a track , but it will do

	int frequency; //time between bursts
	int probability; //probability of particles (0 - 65536)
	int speed;	//mm/second
	
	unsigned int active :1;	
}PARTICLE_GENERATOR_TOOLS_TEMPLATE;





#ifdef __cplusplus

	};

#endif


#endif
