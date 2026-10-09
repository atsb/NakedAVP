#ifndef _included_particle_h_ /* Is this your first time? */
#define _included_particle_h_ 1

#include "decal.h"
#include "dynblock.h"
enum PARTICLE_ID
{
	PARTICLE_PREDATOR_BLOOD,
	PARTICLE_ALIEN_BLOOD,
	PARTICLE_HUMAN_BLOOD,
	PARTICLE_ANDROID_BLOOD,

	PARTICLE_MUZZLEFLASH,
	PARTICLE_SMARTGUNMUZZLEFLASH,

	PARTICLE_WATERSPRAY,
	PARTICLE_WATERFALLSPRAY,
	
	PARTICLE_BLACKSMOKE,
	PARTICLE_FLARESMOKE,
	PARTICLE_STEAM,
	PARTICLE_IMPACTSMOKE,
	PARTICLE_GUNMUZZLE_SMOKE,

	PARTICLE_FLAME,
	PARTICLE_NONCOLLIDINGFLAME,
	PARTICLE_NONDAMAGINGFLAME,
	PARTICLE_FIRE,
	PARTICLE_EXPLOSIONFIRE,
	PARTICLE_MOLOTOVFLAME,
	PARTICLE_MOLOTOVFLAME_NONDAMAGING,

	PARTICLE_SPARK,	
	PARTICLE_RICOCHET_SPARK,	
	PARTICLE_ORANGE_SPARK,
	PARTICLE_ORANGE_PLASMA,

	PARTICLE_PLASMATRAIL,	
	PARTICLE_LASERBEAM,
	PARTICLE_PLASMABEAM,
	PARTICLE_TRACER,

	PARTICLE_LIGHTFLARE,
	PARTICLE_STAR,
	PARTICLE_FLECHETTE,


	PARTICLE_SMOKECLOUD,
	PARTICLE_BLUEPLASMASPHERE,
	PARTICLE_ELECTRICALPLASMASPHERE,

	PARTICLE_PREDPISTOL_FLECHETTE,
	PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING,

	PARTICLE_FLECHETTE_NONDAMAGING,

	PARTICLE_DEWLINE,
	
	PARTICLE_PARGEN_FLAME, //identical to PARTICLE_FLAME , except doesn't damage netghosts

	MAX_NO_OF_PARTICLE_IDS,

	PARTICLE_NULL,
};

enum EXPLOSION_ID
{
	EXPLOSION_PULSEGRENADE,
	EXPLOSION_HUGE,
	EXPLOSION_MOLOTOV,

	EXPLOSION_DISSIPATINGPLASMA,
	EXPLOSION_FOCUSEDPLASMA,

	EXPLOSION_PREDATORPISTOL,

	EXPLOSION_SMALL_NOCOLLISIONS,
	EXPLOSION_HUGE_NOCOLLISIONS,
};

struct ColourComponents
{
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;
};

typedef struct
{
	unsigned int NotYetRendered;

	enum PARTICLE_ID ParticleID;
	signed int LifeTime;
	
	VECTORCH Position;
	
	VECTORCH Velocity;
	VECTORCH Offset;

//	union
//	{
//		unsigned int Colour;
//		struct ColourComponents ColourComponents;
//	};
	unsigned int Colour;

	unsigned int Size;

} PARTICLE;

typedef struct
{
	int StartU;
	int StartV;
	int EndU;
	int EndV;
	unsigned int Size;

	enum TRANSLUCENCY_TYPE TranslucencyType;

	unsigned char Alpha;
	unsigned char RedScale[NUMBER_OF_VISION_MODES];
	unsigned char GreenScale[NUMBER_OF_VISION_MODES];
	unsigned char BlueScale[NUMBER_OF_VISION_MODES];

	unsigned char IsLit:1;
	unsigned char IsDrawnInFront:1;
	unsigned char IsDrawnAtBack:1;
	
} PARTICLE_DESC;

typedef struct
{
	int Active;
	int X;
	int Z;
	int Amplitude;
	int Radius;
	int InvRadius;
} RIPPLE;

typedef struct
{
	VECTORCH Vertex[2];
	VECTORCH Perp[2];
	int Size[2];
} PHEROMONE_TRAIL;


typedef struct 
{
	VECTORCH	SourcePosition;
	VECTORCH	TargetPosition;
	char		BeamHasHitPlayer;
	char		BeamIsOn;
} LASER_BEAM_DESC;



extern PARTICLE_DESC ParticleDescription[];



extern void InitialiseParticleSystem(void);
extern void MakeParticle(VECTORCH *positionPtr, VECTORCH *velocityPtr, enum PARTICLE_ID particleID);
extern void HandleParticleSystem(void);
void RenderAllParticlesFurtherAwayThan(int zThreshold);

extern void HandleRainDrops(MODULE *modulePtr,int numberOfRaindrops);
extern int EffectOfRipples(VECTORCH *point);




enum MUZZLE_FLASH_ID
{
	MUZZLE_FLASH_SMARTGUN,
	MUZZLE_FLASH_AMORPHOUS,
	MUZZLE_FLASH_SKEETER,
};

extern void DrawMuzzleFlash(VECTORCH *positionPtr,VECTORCH *directionPtr, enum MUZZLE_FLASH_ID muzzleFlashID);

extern void MakeFlareParticle(DYNAMICSBLOCK *dynPtr);
extern void MakeRocketTrailParticles(VECTORCH *prevPositionPtr, VECTORCH *positionPtr);
extern void MakeImpactSmoke(MATRIXCH *orientationPtr, VECTORCH *positionPtr);
extern void MakeImpactSparks(VECTORCH *incidentPtr, VECTORCH *normalPtr, VECTORCH *positionPtr);
extern void MakeVolumetricExplosionAt(VECTORCH *positionPtr, enum EXPLOSION_ID explosionID);
extern void MakePlasmaExplosion(VECTORCH *positionPtr, VECTORCH *fromPositionPtr, enum EXPLOSION_ID explosionID);
extern void MakeDewlineTrailParticles(DYNAMICSBLOCK *dynPtr, int number);


extern void MakeBloodExplosion(VECTORCH *originPtr, int creationRadius, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID);
extern void MakeFocusedExplosion(VECTORCH *originPtr, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID);
extern void MakeElectricalExplosion(VECTORCH *positionPtr);
void MakeSprayOfSparks(MATRIXCH *orientationPtr, VECTORCH *positionPtr);
void PlayerPheromoneTrail(DYNAMICSBLOCK *dynPtr);
void MakeGrenadeTrailParticles(VECTORCH *prevPositionPtr, VECTORCH *positionPtr);
void MakePlasmaTrailParticles(DYNAMICSBLOCK *dynPtr, int number);
void NewTrailPoint(DYNAMICSBLOCK *dynPtr);
void TimeScaleThingy();
void DrawFrisbeePlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr);
void DrawPredatorPlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr);
void DrawSmallPredatorPlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr);


#define MAX_NO_OF_BLOOD_PARTICLES 500
extern int NumberOfBloodParticles;

#endif
