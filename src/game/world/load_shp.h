
typedef enum 
{
	I_Shape_BadGuy1 = 0,		
	I_Shape_BadGuy2,		
	I_Shape_BadGuy3,		
	I_Shape_BadGuy4,		
	I_Shape_BadGuy5,
	I_Shape_BadGuy6,		
	I_Shape_BadGuy7,		
	
	I_Shape_Grenade,
	I_Shape_Missile,
	I_Shape_Disc,
	I_Shape_Plasma,	
	I_Shape_AutoGun,
		
	I_Shape_Explosion1,
	I_Shape_Explosion2,
	I_Shape_PlasmaGunExp,
	I_Shape_Ricochet,

	I_Shape_BloodSplash1,
	I_Shape_BloodSplash2,
	I_Shape_BloodSplash3,
	I_Shape_BloodSplash4,
	I_Shape_BloodSplash5,

	I_Num_Character_Shapes,

}CHARACTER_SHAPES;
	

extern void InitCharacterMSLReferences();
extern int GetMSLPosFromEnum(CHARACTER_SHAPES shape_enum);
extern int GetLoadedShapeMSL(const char *);
