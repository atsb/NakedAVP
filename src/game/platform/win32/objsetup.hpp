#ifndef _objsetup_hpp_
#define _objsetup_hpp_

#include "envchunk.hpp"
#include "obchunk.hpp"

extern "C" {
#include "3dc.h"
void SetUpRunTimeLights ();
void create_strategies_from_list();
void deallocate_behaviour_list();
};

void deal_with_module_object(Object_Chunk * ob, int shape1, int AnimationShape, int shape2, MODULE * mod);
void deal_with_placed_object(Object_Chunk * ob, int shape1, int AnimationShape);
void setup_generators (Environment_Data_Chunk * envd);
void setup_particle_generators (Environment_Data_Chunk * envd);
void setup_cables (Environment_Data_Chunk * envd);
void DealWithExternalObjectStategies (Environment_Data_Chunk * envd);
void Create_Xenoborg_Morph_Jobby(Object_Chunk * ob, int AnimationShape, MODULE * mod, MORPHCTRL * mc);
void setup_sounds (Environment_Data_Chunk * envd);
#endif
