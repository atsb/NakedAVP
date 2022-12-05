#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bh_pargen.h"
#include "dynamics.h"
#include "pvisible.h"
#include "particle.h"
#include "lighting.h"

#define UseLocalAssert Yes
#include "ourasert.h"

extern char *ModuleCurrVisArray;
extern int NormalFrameTime;

//particles used by each particle type
static enum PARTICLE_ID ParticleUsed[PARGEN_TYPE_LAST]=
{
	PARTICLE_SPARK,	
	PARTICLE_STEAM,	
	PARTICLE_BLACKSMOKE,
	PARTICLE_PARGEN_FLAME
};

void* ParticleGeneratorBehaveInit(void* bhdata,STRATEGYBLOCK* sbptr)
{
	PARTICLE_GENERATOR_BEHAV_BLOCK* pargen;
	PARTICLE_GENERATOR_TOOLS_TEMPLATE* pg_tt;
	
	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);

	pargen=(PARTICLE_GENERATOR_BEHAV_BLOCK*)AllocateMem(sizeof(PARTICLE_GENERATOR_BEHAV_BLOCK));
	if(!pargen)
	{
		memoryInitialisationFailure = 1;
		return 0;
	}
	pargen->bhvr_type=I_BehaviourParticleGenerator;

	pg_tt=(PARTICLE_GENERATOR_TOOLS_TEMPLATE*)bhdata;

	//copy stuff from tools template
	COPY_NAME(sbptr->SBname, pg_tt->nameID);
	pargen->position = pg_tt->position;
	pargen->orientation = pg_tt->orientation;
	pargen->type = pg_tt->type;
	pargen->active = pg_tt->active;
	pargen->sound = pg_tt->sound;

	//find the generator's module
	sbptr->containingModule=ModuleFromPosition(&(pargen->position),0);
	
	GLOBALASSERT(sbptr->containingModule);

	if(pargen->sound)
	{
		pargen->sound->playing=pargen->active;
	}

	//see if generator has a parent object
	pargen->parent_sbptr=FindSBWithName(pg_tt->parentID);
	if(pargen->parent_sbptr)
	{
		if(pargen->parent_sbptr->DynPtr)
		{
			//work out generator's relative position
			MATRIXCH m=pargen->parent_sbptr->DynPtr->OrientMat;
			TransposeMatrixCH(&m);      
			
			pargen->relative_position=pargen->position;
			pargen->relative_orientation=pargen->orientation;

			SubVector(&pargen->parent_sbptr->DynPtr->Position,&pargen->relative_position);
			RotateVector(&pargen->relative_position,&m);

			MatrixMultiply(&pargen->relative_orientation,&m,&pargen->relative_orientation);
		}
		else
		{
			//parent object has no dynamics block
			//so it can't be used
			pargen->parent_sbptr=0;
		}
	}
		
	pargen->frequency=pg_tt->frequency;
	pargen->timer=pargen->frequency;

	pargen->probability=pg_tt->probability;
	pargen->speed=pg_tt->speed;

	return (void*)pargen;
}

void ParticleGeneratorBehaveFun(STRATEGYBLOCK* sbptr)
{
	BOOL play_sound=FALSE;
	PARTICLE_GENERATOR_BEHAV_BLOCK* pargen;	
 	GLOBALASSERT(sbptr);
	pargen = (PARTICLE_GENERATOR_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((pargen->bhvr_type == I_BehaviourParticleGenerator));

	if(pargen->parent_sbptr)
	{
		//generator has parent object , update position 
		if(pargen->parent_sbptr->SBflags.destroyed_but_preserved)
		{
			//parent object destroyed
			//switch off generator if active and return
			if(pargen->active)
			{
				pargen->active=0;
				if(pargen->sound)
				{
					//stop sound
					Stop_Track_Sound(pargen->sound);
				}
			}
			return;
		}
		else
		{
			DYNAMICSBLOCK *dynPtr = pargen->parent_sbptr->DynPtr;
			GLOBALASSERT(dynPtr);
		
			//update orientation
			MatrixMultiply(&pargen->relative_orientation,&dynPtr->OrientMat,&pargen->orientation);

			//update position
			pargen->position=pargen->relative_position;
			RotateVector(&pargen->position,&dynPtr->OrientMat);	
			AddVector(&dynPtr->Position,&pargen->position);

			//update containing module from parent
			sbptr->containingModule=pargen->parent_sbptr->containingModule;
		}
	}
	
	
	if(pargen->active)
	{
		//update timer
		pargen->timer-=NormalFrameTime;
		
		if(pargen->sound)
		{
			//update sound
			Update_Track_Sound(pargen->sound,&pargen->position);
		}
		
		if(!sbptr->containingModule)
		{
			sbptr->containingModule=ModuleFromPosition(&(pargen->position),0);
		}
		GLOBALASSERT(sbptr->containingModule);
		
		if(ModuleCurrVisArray[(sbptr->containingModule->m_index)])
		{
			//if containing module is near create particles.
			
			switch(pargen->type)
			{
				case PARGEN_TYPE_SPARK :
				{
					if(pargen->timer<0)	
					{

						if((FastRandom() & 0xffff)<=pargen->probability)
						{
							//create sparks
							//like MakeSprayOfSparks , but in z direction
							int noOfSparks = 15;
							do
							{
								
								VECTORCH velocity;
								velocity.vx = (FastRandom()&2047)-1024;
								velocity.vy = (FastRandom()&2047)-1024;
								velocity.vz = (FastRandom()&2047);
								RotateVector(&velocity,&pargen->orientation);
								MakeParticle(&pargen->position,&velocity,ParticleUsed[pargen->type]);	
							}
							while(--noOfSparks);
							
							MakeLightElement(&pargen->position,LIGHTELEMENT_ELECTRICAL_SPARKS);
							play_sound=TRUE;
						}

					}
				}
				break;

				case PARGEN_TYPE_FLAME :
					//add light effect
					MakeLightElement(&pargen->position,LIGHTELEMENT_PARGEN_FLAME);
					//and fall throgh to next section
				case PARGEN_TYPE_STEAM :
				case PARGEN_TYPE_BLACKSMOKE :
				{
					while(pargen->timer<0)
					{
						VECTORCH velocity;
						VECTORCH position=pargen->position;
						int offset;
	
						pargen->timer+=pargen->frequency;

						//get a velocity of magnitude in the region of speed
						velocity.vx = ((FastRandom()&1023) - 512);
						velocity.vy = ((FastRandom()&1023) - 512);
						velocity.vz = MUL_FIXED(pargen->speed,(ONE_FIXED+(FastRandom()&0x3fff)));
						
						RotateVector(&velocity,&pargen->orientation);

						//allow particle to have had part of a frames movement already
						offset=FastRandom()%NormalFrameTime;
						position.vx+=MUL_FIXED(offset,velocity.vx);
						position.vy+=MUL_FIXED(offset,velocity.vy);
						position.vz+=MUL_FIXED(offset,velocity.vz);

						MakeParticle(&position,&velocity,ParticleUsed[pargen->type]);	
					}
				}
				break;
				
				default: ;
			}
		}

		if(pargen->timer<0)	
		{
			pargen->timer=pargen->frequency+(pargen->timer % pargen->frequency);
		}
	}

	if(play_sound)
	{
		//if the generator has a non-looping sound , we should play it now
		if(pargen->sound && !pargen->sound->loop)
		{
			Start_Track_Sound(pargen->sound,&pargen->position);
		}
	}
}


void SendRequestToParticleGenerator(STRATEGYBLOCK* sbptr,BOOL state,int extended_data)
{
	PARTICLE_GENERATOR_BEHAV_BLOCK* pargen = sbptr->SBdataptr;
	LOCALASSERT(pargen);

	if(state)
	{
		//if generator has a parent object that has been destroyed , then it can't be switched on
		if(pargen->parent_sbptr && pargen->parent_sbptr->SBflags.destroyed_but_preserved)
		{
			return;
		}
		
		pargen->active=1;
		if(pargen->sound && pargen->sound->loop)
		{
			//start sound
			Start_Track_Sound(pargen->sound,&pargen->position);
		}
	}
	else
	{
		pargen->active=0;
		if(pargen->sound)
		{
			//stop sound
			Stop_Track_Sound(pargen->sound);
		}
	}

}





/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

typedef struct particle_generator_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	
	VECTORCH position;
	MATRIXCH orientation; 

	VECTORCH relative_position;	 //relative to parent object (if it exists)
	MATRIXCH relative_orientation; //relative to parent object (if it exists)

	int timer; //time left to next particle burst

	unsigned int active :1;	//is generator currently active
	unsigned int sound_playing :1;

}PARTICLE_GENERATOR_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV pargen

void LoadStrategy_ParticleGenerator(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PARTICLE_GENERATOR_BEHAV_BLOCK* pargen;	
	PARTICLE_GENERATOR_SAVE_BLOCK* block = (PARTICLE_GENERATOR_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourParticleGenerator) return;

	pargen = (PARTICLE_GENERATOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff
	COPYELEMENT_LOAD(position)
	COPYELEMENT_LOAD(orientation)
	COPYELEMENT_LOAD(relative_position)
	COPYELEMENT_LOAD(relative_orientation)
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(active)

	if(pargen->sound)
	{
		pargen->sound->playing = block->sound_playing;
		Load_SoundState(&pargen->sound->activ_no);
	}
}

void SaveStrategy_ParticleGenerator(STRATEGYBLOCK* sbPtr)
{
	PARTICLE_GENERATOR_SAVE_BLOCK *block;
	PARTICLE_GENERATOR_BEHAV_BLOCK* pargen;
	
	pargen = (PARTICLE_GENERATOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff
	COPYELEMENT_SAVE(position)
	COPYELEMENT_SAVE(orientation)
	COPYELEMENT_SAVE(relative_position)
	COPYELEMENT_SAVE(relative_orientation)
	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(active)

	if(pargen->sound)
	{
		block->sound_playing = pargen->sound->playing;
		Save_SoundState(&pargen->sound->activ_no);
	}
}
