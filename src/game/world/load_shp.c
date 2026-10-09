
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"

#include "load_shp.h"

#define UseLocalAssert No

#include "ourasert.h"

char *Marine_Loaded_Shape_Names[I_Num_Character_Shapes] =
{
	"ALIEN_STANDING",
	"ALIEN_CROUCHING",
	"XENOBORG",
	"PREDATOR",
	"PREDATOR_ALIEN",
	"FACE_HUGGER",
	"QUEEN",
	
	"GRENADE",
	"MISSILE",
	"DISC",
	"PRED_PALSMA",
	"AUTOGUN",
	
	"EXPLOSION",
	"EXPLOSION",
	"PLASMA_EXP",
	"RICOCHET",

	"ALIEN_BLOOD_SPLASH",
	"ALIEN_BLOOD_SPLASH",
	"PREDATOR_BLOOD_SPLASH",
	"ALIEN_BLOOD_SPLASH",
	"FACEHUGGER_BLOOD_SPLASH",
};

	
char *Alien_Loaded_Shape_Names[I_Num_Character_Shapes] =
{
	"MARINE",
	"NONE",
	"XENOBORG",
	"PREDATOR",
	"PREDATOR_ALIEN",
	"NONE",
	"QUEEN",
	
	"GRENADE",
	"MISSILE",
	"DISC",
	"PRED_PALSMA",
	"NONE",
	
	"EXPLOSION",
	"EXPLOSION",
	"PLASMA_EXP",
	"RICOCHET",

	"MARINE_BLOOD_SPLASH",
	"NONE",
	"PREDATOR_BLOOD_SPLASH",
	"NONE",
	"NONE",
};


char *Predator_Loaded_Shape_Names[I_Num_Character_Shapes] =
{
	"MARINE",
	"NONE",
	"XENOBORG",
	"SPECIAL_MARINE",
	"PREDATOR_ALIEN",
	"FACE_HUGGER",
	"QUEEN",
	
	"GRENADE",
	"MISSILE",
	"DISC",
	"PRED_PALSMA",
	"NONE",
	
	"EXPLOSION",
	"EXPLOSION",
	"PLASMA_EXP",
	"RICOCHET",

	"ALIEN_BLOOD_SPLASH",
	"ALIEN_BLOOD_SPLASH",
	"MARINE_BLOOD_SPLASH",
	"ALIEN_BLOOD_SPLASH",
	"FACEHUGGER_BLOOD_SPLASH",
};

static int LoadedShapesInMSL[I_Num_Character_Shapes];

void InitCharacterMSLReferences()
{
	int shape_num = I_Num_Character_Shapes;

	while(--shape_num >= 0)
		{
			switch(AvP.PlayerType)
				{
					case I_Marine:
						{
//							LoadedShapesInMSL[shape_num] =
//								 GetLoadedShapeMSL(Marine_Loaded_Shape_Names[shape_num]);					
							break;
						}
					case I_Predator:
						{
//							LoadedShapesInMSL[shape_num] =
//								 GetLoadedShapeMSL(Predator_Loaded_Shape_Names[shape_num]);					
							break;
						}
					case I_Alien:
						{
//							LoadedShapesInMSL[shape_num] =
//								 GetLoadedShapeMSL(Alien_Loaded_Shape_Names[shape_num]);					
							break;
						}

					default:
						GLOBALASSERT(2<1);
				}
		}
}



int GetMSLPosFromEnum(CHARACTER_SHAPES shape_enum)
{
	return(LoadedShapesInMSL[shape_enum]);
}
