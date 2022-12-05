#ifndef __AVP_COMP_SHP_H__
#define __AVP_COMP_SHP_H__

extern SHAPEHEADER** mainshapelist;

extern int start_of_loaded_shapes;

extern SHAPEHEADER CUBE_header;

typedef enum marineshapesenum
{
	I_ShapeCube,
    I_ShapePrism,
    I_ShapeMarinePlayer,
    I_ShapePredatorPlayer,
    I_ShapeAlienPlayer,
    I_ShapeAlien,
	I_ShapeBloodSplash,
	I_ShapeBulletRicochet,
    I_ShapeExplosion,
    I_ShapeMarinePlayerCrouch,
	I_ShapeMarinePlayerLieDown,
	I_ShapeFlame1,
	I_ShapeFlame2,
	I_ShapeLGrenade,
    I_ShapePulseRifle,
	I_ShapeAAShotgun,
	I_ShapeCanisterGun,
	I_ShapeFThrower,
	I_ShapeSmartGun,
	I_ShapePig,
	I_ShapeLATW,
	I_ShapeParticle,
	I_ShapeRocket,
	I_ShapeGenerator,

} MARINE_SHAPES_ENUM;

extern int load_precompiled_shapes(void);

#endif
