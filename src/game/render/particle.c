/* KJL 14:50:53 10/13/97 - 
 *
 *	Experimental particle system
 *	
 */

#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "dynblock.h"
#include "dynamics.h"
#include "particle.h"
#include "kshape.h"
#include "sfx.h"
#include "d3d_render.h"
#include "psndproj.h"
#include "lighting.h"
#include "bh_light.h"
#include "bh_xeno.h"
#include "pvisible.h"
#include "sphere.h"
#include "bh_rubberduck.h"
#include "bh_weap.h"
#include "weapons.h"
#include "decal.h"
#include "avpview.h"
#include "pldghost.h"
#include "detaillevels.h"
#include "psnd.h"
#include "kzsort.h"
#include "avp_userprofile.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "savegame.h"
#include "los.h"
#include "chnkload.h"
#include "maths.h"

#include <math.h>

#define RGBALIGHT_MAKE(r,g,b,a) RGBA_MAKE(r,g,b,a)

#define MAX_NO_OF_PHEROMONE_TRAILS 500
#define MAX_NO_OF_PARTICLES 5000
#define MAX_NO_OF_EXPLOSIONS 10

#define TRAIL_DECAY_SPEED (51*65536*5)

static PHEROMONE_TRAIL TrailStorage[MAX_NO_OF_PHEROMONE_TRAILS];
static PARTICLE ParticleStorage[MAX_NO_OF_PARTICLES];
static VOLUMETRIC_EXPLOSION ExplosionStorage[MAX_NO_OF_EXPLOSIONS];

static int NumActiveTrails;
static int NumActiveParticles;
static int CurrentExplosionIndex;

extern int NormalFrameTime;
extern int PrevNormalFrameTime;
extern int CloakingPhase;

int NumberOfBloodParticles;
int NumberOfFlaresActive;

extern SOUND3DDATA PredPistolExplosion_SoundData;
extern MODULE *playerPherModule;

static void InitialiseVolumetricExplosions(void);
void DoFlareCorona(DISPLAYBLOCK *objectPtr);
void InitialiseRainDrops(void);
void HandleRipples(void);
void AddRipple(int x,int z,int amplitude);
void MakeMolotovExplosionAt(VECTORCH *positionPtr,int seed);
static void HandleVolumetricExplosion(VOLUMETRIC_EXPLOSION *expPtr);
void DrawXenoborgMainLaserbeam(LASER_BEAM_DESC *laserPtr);
void HandlePheromoneTrails(void);
void RenderTrailSegment(PHEROMONE_TRAIL *trailPtr);
		
PARTICLE_DESC ParticleDescription[MAX_NO_OF_PARTICLE_IDS] =
{
	/* PARTICLE_PREDATOR_BLOOD */
	{
		//int StartU;
		0<<16,
		//int StartV;
		64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		128,
		//unsigned char RedScale;
		{0,		0,		255,	255,	255},
		//unsigned char GreenScale;
		{255,	255,	255,	255,	0},
		//unsigned char BlueScale;
		{0,		0,		255,	255,	0},

		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,

	},
	/* PARTICLE_ALIEN_BLOOD */
	{
#if 0
		//int StartU;
		1<<16,
		//int StartV;
		248<<16,
		//int EndU;
		8<<16,
		//int EndV;
		256<<16,
#else
		//int StartU;
		0<<16,
		//int StartV;
		64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
#endif
		//unsigned int Size;
		50,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		128,
		//unsigned char RedScale;
		{255,	255,	0,		16,		255},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255},
		//unsigned char BlueScale;
		{0,		0,		0,		255,	255},

		//unsigned char IsLit:1;
		1,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,

	},
	/* PARTICLE_HUMAN_BLOOD */
	{
		//int StartU;
		0<<16,
		//int StartV;
		63<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		50,
		   
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{0,			0,		0,		0 ,	0},
		//unsigned char GreenScale;
		{255,		255,	64,		64,	255},
		//unsigned char BlueScale;
		{255,		255,	0,		64,	255},

		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_ANDROID_BLOOD */
	{
		//int StartU;
		0<<16,
		//int StartV;
		63<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		50,
		   
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,		255,  	255,	255,	255},
		//unsigned char GreenScale;
		{255,		255,	255,	255,	255},
		//unsigned char BlueScale;
		{255,		255,	255,	255,	255},

		//unsigned char IsLit:1;
		1,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_MUZZLEFLASH */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{192,	255,	255,	0  ,	255},
		//unsigned char BlueScale;
		{128,	255,	255,	0  ,	0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_SMARTGUNMUZZLEFLASH */
	{
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		127<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		800,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255},
		//unsigned char BlueScale;
		{255,	255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_WATERSPRAY */
	{
		//int StartU;
		0<<16,
		//int StartV;
	   	64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		80,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{200,	200,	0,		0},
		//unsigned char GreenScale;
		{230,	230,	240,	0},
		//unsigned char BlueScale;
		{255,	255,	0,		120},
	
		//unsigned char IsLit:1;
		1,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_WATERFALLSPRAY */
	{
		//int StartU;
		0<<16,
		//int StartV;
	   	64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		1600,//200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		32,
		//unsigned char RedScale;
		{100,	200,	240,	120,	120,	120},
		//unsigned char GreenScale;
		{130,	230,	240,	120,	120,	120},
		//unsigned char BlueScale;
		{255,	255,	240,	120,	120,	120},

		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_BLACKSMOKE */
	{
		//int StartU;
		0<<16,
		//int StartV;
	   	130<<16,
		//int EndU;
		28<<16,
		//int EndV;
		157<<16,
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{64,	64,		64,		64,		64},
		//unsigned char GreenScale;
		{64,	64,		64,		64,		64},
		//unsigned char BlueScale;
		{64,	64,		64,		64,		64},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_FLARESMOKE */
	{
		#if 0
		//int StartU;
		0<<16,
		//int StartV;
	   	130<<16,
		//int EndU;
		28<<16,
		//int EndV;
		157<<16,
		#else
		//int StartU;
		128<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		195<<16,
		//int EndV;
		63<<16,
		#endif
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,		64},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		64},
		//unsigned char BlueScale;
		{255,	255,	255,	64,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_STEAM */
	{
		//int StartU;
		0<<16,
		//int StartV;
	   	130<<16,
		//int EndU;
		28<<16,
		//int EndV;
		157<<16,
		//unsigned int Size;
		100,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{128,	128,	128,	128,		64},
		//unsigned char GreenScale;
		{128,	128,	128,	128,		64},
		//unsigned char BlueScale;
		{128,	128,	128,	128,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_IMPACTSMOKE */
	{
		//int StartU;
		0<<16,
		//int StartV;
	   	130<<16,
		//int EndU;
		28<<16,
		//int EndV;
		157<<16,
		//unsigned int Size;
		100,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{64,	64,		64,		64,		64},
		//unsigned char GreenScale;
		{64,	64,		64,		64,		64},
		//unsigned char BlueScale;
		{64,	64,		64,		64,		64},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_GUNMUZZLE_SMOKE */
	{
		//int StartU;
		128<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		195<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		32,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	64},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		64},
		//unsigned char BlueScale;
		{255,	255,	255,	64,		0},
	
		//unsigned char IsLit:1;
		1,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_FLAME */
	{
		#if 0
		//int StartU;
		0<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		63<<16,
		//int EndV;
		63<<16,
		#else
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		#endif
		//unsigned int Size;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_NONCOLLIDINGFLAME */
	{
		#if 0
		//int StartU;
		0<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		63<<16,
		//int EndV;
		63<<16,
		#else
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		#endif
		//unsigned int Size;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_NONDAMAGINGFLAME */
	{
		#if 0
		//int StartU;
		0<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		63<<16,
		//int EndV;
		63<<16,
		#else
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		#endif
		//unsigned int Size;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_FIRE */
	{
	   	#if 1
	   	//int StartU;
		0<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		63<<16,
		//int EndV;
		63<<16,
		#else
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		#endif

		//unsigned int Size;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_EXPLOSIONFIRE */
	{
		#if 0
		//int StartU;
		128<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		192<<16,
		//int EndV;
		63<<16,
		#else
		//int StartU;
		0<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		48<<16,
		//int EndV;
		63<<16,
		#endif
		//unsigned int Size;
		800,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,  
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_MOLOTOVFLAME */
	{
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		800,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,  
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_MOLOTOVFLAME_NONDAMAGING */
	{
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		800,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,  
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_SPARK */
	{
		//int StartU;
		1<<16,
		//int StartV;
	   	1<<16,
		//int EndU;
		64<<16,
		//int EndV;
		64<<16,
		//unsigned int Size;
		200/8,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_RICOCHET_SPARK */
	{
		//int StartU;
		1<<16,
		//int StartV;
	   	1<<16,
		//int EndU;
		64<<16,
		//int EndV;
		64<<16,
		//unsigned int Size;
		200/8,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		255},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_ORANGE_SPARK */
	{
		//int StartU;
		224<<16,
		//int StartV;
	   	192<<16,
		//int EndU;
		255<<16,
		//int EndV;
		223<<16,
		//unsigned int Size;
		50,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	64,		64,		64,		64},
		//unsigned char GreenScale;
		{255,	64,		64,		64,		64},
		//unsigned char BlueScale;
		{255,	64,		64,		64,		64},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_ORANGE_PLASMA */
	{
		//int StartU;
		224<<16,
		//int StartV;
	   	192<<16,
		//int EndU;
		255<<16,
		//int EndV;
		223<<16,
		//unsigned int Size;
		50,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	64,		64,		64,		64,		0},
		//unsigned char GreenScale;
		{255,	64,		64,		64,		64,		64},
		//unsigned char BlueScale;
		{255,	64,		64,		64,		64,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},

	/* PARTICLE_PLASMATRAIL */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200/4,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_LASERBEAM */
	{
		//int StartU;
		32<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		32<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		50,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_PLASMABEAM */
	{
		//int StartU;
		192<<16,
		//int StartV;
	   	224<<16,
		//int EndU;
		223<<16,
		//int EndV;
		255<<16,
		//unsigned int Size;
		50,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_TRACER */
	{
		//int StartU;
		32<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		32<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		16,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{255,	128,	255,	0,		255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
   
   

	/* PARTICLE_LIGHTFLARE */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{255,	255,	255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		1,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_STAR */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{255,	255,	255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		1,
	},
	/* PARTICLE_FLECHETTE */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		100,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_NORMAL,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{255,	255,	255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_SMOKECLOUD */
	{
		//int StartU;
		128<<16,
		//int StartV;
	   	64<<16,
		//int EndU;
		191<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		1000,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{128,	64,		64,		64,		0},
		//unsigned char GreenScale;
		{128,	64,		64,		64,		64},
		//unsigned char BlueScale;
		{128,	64,		64,		64,		0},
	
		//unsigned char IsLit:1;
		1,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_BLUEPLASMASPHERE */
	{
		//int StartU;
		224<<16,
		//int StartV;
	   	160<<16,
		//int EndU;
		255<<16,
		//int EndV;
		191<<16,
		//unsigned int Size;
		500,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	64,		64,		64,		64,		0},
		//unsigned char GreenScale;
		{255,	64,		64,		64,		64,		255},
		//unsigned char BlueScale;
		{255,	64,		64,		64,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	}, 
	/* PARTICLE_ELECTRICALPLASMASPHERE */
	{
		//int StartU;
		64<<16,
		//int StartV;
	   	128<<16,
		//int EndU;
		127<<16,
		//int EndV;
		191<<16,
		//unsigned int Size;
		2000,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	64,		64,		64,		64,		0},
		//unsigned char GreenScale;
		{255,	64,		64,		64,		64,		255},
		//unsigned char BlueScale;
		{255,	64,		64,		64,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	}, 
	/* PARTICLE_PREDPISTOL_FLECHETTE */
	{
		//int StartU;
		1<<16,
		//int StartV;
	   	1<<16,
		//int EndU;
		64<<16,
		//int EndV;
		64<<16,
		//unsigned int Size;
		200/8,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING */
	{
		//int StartU;
		1<<16,
		//int StartV;
	   	1<<16,
		//int EndU;
		64<<16,
		//int EndV;
		64<<16,
		//unsigned int Size;
		200/8,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_FLECHETTE_NONDAMAGING */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		100,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_NORMAL,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{255,	255,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{255,	255,	255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_DEWLINE */
	{
		//int StartU;
		64<<16,
		//int StartV;
		64<<16,
		//int EndU;
		127<<16,
		//int EndV;
		127<<16,
		//unsigned int Size;
		200/4,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	0,		255,	255},
		//unsigned char BlueScale;
		{64,	64,		255,	0,		0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},
	/* PARTICLE_PARGEN_FLAME */
	{
		//int StartU;
		64<<16,
		//int StartV;
	   	0<<16,
		//int EndU;
		(64+127)<<16,
		//int EndV;
		63<<16,
		//unsigned int Size;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,	255,	255,	128,	128,	0},
		//unsigned char GreenScale;
		{128,	128,	255,	64,		64,		128},
		//unsigned char BlueScale;
		{64,	64,		255,	255,	0,		0},
	
		//unsigned char IsLit:1;
		0,
		//IsDrawnInFront:1;
		0,
		//IsDrawnAtBack:1;
		0,
	},

};


static PARTICLE* AllocateParticle(void);
static void DeallocateParticle(PARTICLE *particlePtr);
static PHEROMONE_TRAIL* AllocatePheromoneTrail(void);
static void DeallocatePheromoneTrail(PHEROMONE_TRAIL *trailPtr);




void InitialiseParticleSystem(void)
{
 	NumActiveParticles = 0;
	NumberOfBloodParticles = 0;

 	NumActiveTrails = 0;

	InitialiseVolumetricExplosions();
	InitialiseRainDrops();
	InitialiseDecalSystem();
	InitForceField();
	Generate_Sphere();

	{
		NumberOfFlaresActive=0;
	}
}

static PARTICLE* AllocateParticle(void)
{
	PARTICLE *particlePtr = 0; /* Default to null ptr */

	if (NumActiveParticles != MAX_NO_OF_PARTICLES) 
	{
		particlePtr = &ParticleStorage[NumActiveParticles];
		NumActiveParticles++;
	}
	else
	{
		/* unable to allocate a particle */
	}

	return particlePtr;
}
static void DeallocateParticle(PARTICLE *particlePtr)
{
	/* is pointer within array? */
	LOCALASSERT(particlePtr>=ParticleStorage);
	LOCALASSERT(particlePtr<=&ParticleStorage[MAX_NO_OF_PARTICLES-1]);
	
	NumActiveParticles--;
	*particlePtr = ParticleStorage[NumActiveParticles];
}

static PHEROMONE_TRAIL* AllocatePheromoneTrail(void)
{
	PHEROMONE_TRAIL *trailPtr = 0; /* Default to null ptr */

	if (NumActiveTrails != MAX_NO_OF_PHEROMONE_TRAILS) 
	{
		trailPtr = &TrailStorage[NumActiveTrails];
		NumActiveTrails++;
	}
	else
	{
		/* unable to allocate a trail */
	}

	return trailPtr;
}
static void DeallocatePheromoneTrail(PHEROMONE_TRAIL *trailPtr)
{
	/* is pointer within array? */
	LOCALASSERT(trailPtr>=TrailStorage);
	LOCALASSERT(trailPtr<=&TrailStorage[MAX_NO_OF_PHEROMONE_TRAILS-1]);
	
	NumActiveTrails--;
	*trailPtr = TrailStorage[NumActiveTrails];
}

static void InitialiseVolumetricExplosions(void)
{
	int i;
	for(i=0; i<MAX_NO_OF_EXPLOSIONS; i++)
	{
		ExplosionStorage[i].LifeTime = 0;
	}
	CurrentExplosionIndex = 0;
}
static VOLUMETRIC_EXPLOSION* AllocateVolumetricExplosion(void)
{
	VOLUMETRIC_EXPLOSION *explosionPtr = 0; /* Default to null ptr */

	explosionPtr = &ExplosionStorage[CurrentExplosionIndex];

	CurrentExplosionIndex++;
	if (CurrentExplosionIndex>=MAX_NO_OF_EXPLOSIONS)
	{
		CurrentExplosionIndex=0;
	}

	LOCALASSERT(explosionPtr);
	return explosionPtr;
}


void MakeParticle(VECTORCH *positionPtr, VECTORCH *velocityPtr, enum PARTICLE_ID particleID)
{
	PARTICLE *particlePtr;

	if( (particleID == PARTICLE_ALIEN_BLOOD) || (particleID == PARTICLE_PREDATOR_BLOOD) || (particleID == PARTICLE_HUMAN_BLOOD)|| (particleID == PARTICLE_ANDROID_BLOOD))
		if (NumberOfBloodParticles>MAX_NO_OF_BLOOD_PARTICLES)
			return;

	particlePtr = AllocateParticle();
	/* were we able to allocate a particle? */
	if (particlePtr)
	{
		PARTICLE_DESC *particleDescPtr = &ParticleDescription[particleID];
		particlePtr->Position = *positionPtr;
		particlePtr->Velocity = *velocityPtr;
		
		particlePtr->ParticleID = particleID;

		particlePtr->Colour = RGBALIGHT_MAKE
							  (
							  	particleDescPtr->RedScale[CurrentVisionMode],
							  	particleDescPtr->GreenScale[CurrentVisionMode],
							  	particleDescPtr->BlueScale[CurrentVisionMode],
							  	particleDescPtr->Alpha
							  );
		particlePtr->Size = particleDescPtr->Size;
	
		switch(particlePtr->ParticleID)
		{
			case PARTICLE_PREDATOR_BLOOD:
			case PARTICLE_ALIEN_BLOOD:
			case PARTICLE_HUMAN_BLOOD:
			case PARTICLE_ANDROID_BLOOD:
			{
				particlePtr->Offset = particlePtr->Position;
				particlePtr->LifeTime = ONE_FIXED;
				NumberOfBloodParticles++;
				break;
			}
			case PARTICLE_BLACKSMOKE:
			{
				particlePtr->LifeTime = ONE_FIXED+(FastRandom()&32767);
				particlePtr->Offset.vx = ((FastRandom()&1023) - 512)*2;
				particlePtr->Offset.vz = ((FastRandom()&1023) - 512)*2;
				break;
			}
			case PARTICLE_WATERSPRAY:
			{
				particlePtr->LifeTime = ONE_FIXED/2;
				break;
			}
			case PARTICLE_WATERFALLSPRAY:
			{
				particlePtr->LifeTime = ONE_FIXED*10;
				break;
			}

			case PARTICLE_FLARESMOKE:
			case PARTICLE_STEAM:
			{
				particlePtr->LifeTime = (ONE_FIXED*1)/2+(FastRandom()&32767);
				particlePtr->Offset.vx = ((FastRandom()&8191) - 4096);
				particlePtr->Offset.vz = ((FastRandom()&8191) - 4096);
				break;
			}
			case PARTICLE_IMPACTSMOKE:
			{
				particlePtr->LifeTime = ONE_FIXED+(FastRandom()&32767);
				particlePtr->Offset.vx = ((FastRandom()&1023) - 512)*2;
				particlePtr->Offset.vz = ((FastRandom()&1023) - 512)*2;
				break;
			}
			case PARTICLE_GUNMUZZLE_SMOKE:
			{
				particlePtr->LifeTime = ONE_FIXED/2+(FastRandom()&32767);
				particlePtr->Offset.vx = ((FastRandom()&1023) - 512);
				particlePtr->Offset.vz = ((FastRandom()&1023) - 512);
				break;
			}

			case PARTICLE_SPARK:	  
			{
				particlePtr->LifeTime = ONE_FIXED;
				break;
			}
			case PARTICLE_RICOCHET_SPARK:
			case PARTICLE_ORANGE_SPARK:
			{
				particlePtr->LifeTime = ONE_FIXED/4;
				break;
			}

			case PARTICLE_ORANGE_PLASMA:
			{
				particlePtr->LifeTime = ONE_FIXED/8;
				break;
			}

			case PARTICLE_PLASMATRAIL:
			{
				particlePtr->LifeTime = ONE_FIXED/2;
				particlePtr->Offset=particlePtr->Position;
				break;
			}
			case PARTICLE_DEWLINE:
			{
				particlePtr->LifeTime = ONE_FIXED; // /2
				particlePtr->Offset=particlePtr->Position;
				break;
			}

			case PARTICLE_FLAME:
			case PARTICLE_NONCOLLIDINGFLAME:
			case PARTICLE_NONDAMAGINGFLAME:
			case PARTICLE_PARGEN_FLAME:
			{
				particlePtr->Offset.vx = (FastRandom()&4095);
				particlePtr->Offset.vy = ((FastRandom()&32767) - 16384);

				particlePtr->LifeTime = ONE_FIXED/2;
				break;
			}
			case PARTICLE_FIRE:
			{
				particlePtr->LifeTime = ONE_FIXED/2+(FastRandom()&16383);
				break;
			}
			case PARTICLE_EXPLOSIONFIRE:
			{
				particlePtr->LifeTime = ONE_FIXED/4;
				particlePtr->Offset = particlePtr->Position;
//	 			particlePtr->Position.vx += particlePtr->Velocity.vx;
//	 			particlePtr->Position.vy += particlePtr->Velocity.vy;
//	 			particlePtr->Position.vz += particlePtr->Velocity.vz;
				break;					 
			}
			case PARTICLE_MOLOTOVFLAME:
			{
				particlePtr->LifeTime = ONE_FIXED*2-(FastRandom()&32767);
				break;
			}	

			case PARTICLE_FLECHETTE:
			case PARTICLE_FLECHETTE_NONDAMAGING:
			{
				particlePtr->LifeTime = ONE_FIXED*8;
				{
					particlePtr->Offset.vy = 1;
				}
				break;
			}

			case PARTICLE_SMOKECLOUD:
			{
				particlePtr->LifeTime = ONE_FIXED*16-1;
				particlePtr->Offset.vx = (FastRandom()&4095);
				particlePtr->Offset.vy = ((FastRandom()&16383) - 8192);
				break;
			}
			case PARTICLE_BLUEPLASMASPHERE:
			{
				particlePtr->LifeTime = 0;
				break;
			}
			case PARTICLE_ELECTRICALPLASMASPHERE:
			{
				particlePtr->LifeTime = 32767;
				break;
			}
			case PARTICLE_PREDPISTOL_FLECHETTE:
			case PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING:
			{
				particlePtr->LifeTime = ONE_FIXED;
				break;
			}
			case PARTICLE_TRACER:
			{
				particlePtr->Position = *positionPtr;
				particlePtr->Offset = *velocityPtr;
				particlePtr->LifeTime = 0;
				break;
			}

			default:
			{
				/* particle initialised wrongly */
				DeallocateParticle(particlePtr);
				LOCALASSERT(0);
				break;
			}
		}

	}
}

void HandleParticleSystem(void)
{
	int i;
	PARTICLE *particlePtr;
	HandleRipples();
//	D3D_DrawWaterTest();
	
	HandleDecalSystem();
	D3D_DecalSystem_End();


	
//	textprint("Particles Active: %d\n",i);
//	D3D_DecalSystem_Setup();
	i = NumActiveParticles;
	particlePtr = ParticleStorage;
	
	while(i--)
	{
		PARTICLE_DESC *particleDescPtr = &ParticleDescription[particlePtr->ParticleID];

		particlePtr->NotYetRendered = 1;
		switch(particlePtr->ParticleID)
		{
			case PARTICLE_ALIEN_BLOOD:
			case PARTICLE_PREDATOR_BLOOD:
			case PARTICLE_HUMAN_BLOOD:
			case PARTICLE_ANDROID_BLOOD:
			{
				particlePtr->Size = 64-(FastRandom()&31);
				particlePtr->Offset	= particlePtr->Position;
				particlePtr->Offset.vx += particlePtr->Velocity.vx>>4;
				particlePtr->Offset.vy += particlePtr->Velocity.vy>>4;
				particlePtr->Offset.vz += particlePtr->Velocity.vz>>4;
				break;
			}
			case PARTICLE_FLARESMOKE:
			{
//				particlePtr->Position.vy -= MUL_FIXED(1000+(FastRandom()&511),NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				
				if (particlePtr->Velocity.vy > -1300)
					particlePtr->Velocity.vy -= MUL_FIXED(4000,NormalFrameTime);

				if (particlePtr->Velocity.vx > 0)
				{
					particlePtr->Velocity.vx -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx < 0) particlePtr->Velocity.vx = 0;
				}
				else if (particlePtr->Velocity.vx < 0)
				{
					particlePtr->Velocity.vx += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx > 0) particlePtr->Velocity.vx = 0;
				}

				if (particlePtr->Velocity.vz > 0)
				{
					particlePtr->Velocity.vz -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz < 0) particlePtr->Velocity.vz = 0;
				}
				else if (particlePtr->Velocity.vz < 0)
				{
					particlePtr->Velocity.vz += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz > 0) particlePtr->Velocity.vz = 0;
				}

				
				particlePtr->Position.vx += MUL_FIXED
											(
												particlePtr->Velocity.vx+
												MUL_FIXED
												(
													-GetSin((particlePtr->Position.vz+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vx
												),
												NormalFrameTime
											
											);

				particlePtr->Position.vz += MUL_FIXED
											(
												particlePtr->Velocity.vz+
												MUL_FIXED
												(
													GetCos((particlePtr->Position.vx+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vz
												),
												NormalFrameTime
											
											);

				particlePtr->Colour = RGBALIGHT_MAKE
									  (
									  	particleDescPtr->RedScale[CurrentVisionMode],
									  	particleDescPtr->GreenScale[CurrentVisionMode],
									  	particleDescPtr->BlueScale[CurrentVisionMode],
									  	(particlePtr->LifeTime>>10)
									  );
				particlePtr->Size = MUL_FIXED(ONE_FIXED-particlePtr->LifeTime,200)+50;
				
				break;
			}
			case PARTICLE_STEAM:
			{
//				particlePtr->Position.vy -= MUL_FIXED(1000+(FastRandom()&511),NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				
				if (particlePtr->Velocity.vy > -1300)
					particlePtr->Velocity.vy -= MUL_FIXED(4000,NormalFrameTime);

				if (particlePtr->Velocity.vx > 0)
				{
					particlePtr->Velocity.vx -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx < 0) particlePtr->Velocity.vx = 0;
				}
				else if (particlePtr->Velocity.vx < 0)
				{
					particlePtr->Velocity.vx += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx > 0) particlePtr->Velocity.vx = 0;
				}

				if (particlePtr->Velocity.vz > 0)
				{
					particlePtr->Velocity.vz -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz < 0) particlePtr->Velocity.vz = 0;
				}
				else if (particlePtr->Velocity.vz < 0)
				{
					particlePtr->Velocity.vz += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz > 0) particlePtr->Velocity.vz = 0;
				}

				
				particlePtr->Position.vx += MUL_FIXED
											(
												particlePtr->Velocity.vx+
												MUL_FIXED
												(
													-GetSin((particlePtr->Position.vz+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vx
												),
												NormalFrameTime
											
											);

				particlePtr->Position.vz += MUL_FIXED
											(
												particlePtr->Velocity.vz+
												MUL_FIXED
												(
													GetCos((particlePtr->Position.vx+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vz
												),
												NormalFrameTime
											
											);
				particlePtr->Colour = RGBALIGHT_MAKE
									  (
									  	particleDescPtr->RedScale[CurrentVisionMode],
									  	particleDescPtr->GreenScale[CurrentVisionMode],
									  	particleDescPtr->BlueScale[CurrentVisionMode],
										(particlePtr->LifeTime>>14)+17
									  );
				break;
			}

			case PARTICLE_BLACKSMOKE:
			case PARTICLE_IMPACTSMOKE:
			{
//				particlePtr->Position.vy -= MUL_FIXED(1000+(FastRandom()&511),NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				
				if (particlePtr->Velocity.vy > -1300)
					particlePtr->Velocity.vy -= MUL_FIXED(3000,NormalFrameTime);

				if (particlePtr->Velocity.vx > 0)
				{
					particlePtr->Velocity.vx -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx < 0) particlePtr->Velocity.vx = 0;
				}
				else if (particlePtr->Velocity.vx < 0)
				{
					particlePtr->Velocity.vx += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vx > 0) particlePtr->Velocity.vx = 0;
				}

				if (particlePtr->Velocity.vz > 0)
				{
					particlePtr->Velocity.vz -= MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz < 0) particlePtr->Velocity.vz = 0;
				}
				else if (particlePtr->Velocity.vz < 0)
				{
					particlePtr->Velocity.vz += MUL_FIXED(2000,NormalFrameTime);
					if (particlePtr->Velocity.vz > 0) particlePtr->Velocity.vz = 0;
				}

				
				particlePtr->Position.vx += MUL_FIXED
											(
												particlePtr->Velocity.vx+
												MUL_FIXED
												(
													-GetSin((particlePtr->Position.vz+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vx
												),
												NormalFrameTime
											
											);

				particlePtr->Position.vz += MUL_FIXED
											(
												particlePtr->Velocity.vz+
												MUL_FIXED
												(
													GetCos((particlePtr->Position.vx+particlePtr->Position.vy)&4095)/4,
													particlePtr->Offset.vz
												),
												NormalFrameTime
											
											);

				{
					int colour = particlePtr->LifeTime>>11;
		  			particlePtr->Colour = RGBALIGHT_MAKE(colour,colour,colour,255);
				}												
				
				break;
			}
			case PARTICLE_GUNMUZZLE_SMOKE:
			{
//				particlePtr->Position.vy -= MUL_FIXED(1000+(FastRandom()&511),NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				#if 1
				if (particlePtr->Velocity.vy > -1300)
					particlePtr->Velocity.vy -= MUL_FIXED(1024,NormalFrameTime);

				if (particlePtr->Velocity.vx > 0)
				{
					particlePtr->Velocity.vx -= MUL_FIXED(1024,NormalFrameTime);
					if (particlePtr->Velocity.vx < 0) particlePtr->Velocity.vx = 0;
				}
				else if (particlePtr->Velocity.vx < 0)
				{
					particlePtr->Velocity.vx += MUL_FIXED(1024,NormalFrameTime);
					if (particlePtr->Velocity.vx > 0) particlePtr->Velocity.vx = 0;
				}

				if (particlePtr->Velocity.vz > 0)
				{
					particlePtr->Velocity.vz -= MUL_FIXED(1024,NormalFrameTime);
					if (particlePtr->Velocity.vz < 0) particlePtr->Velocity.vz = 0;
				}
				else if (particlePtr->Velocity.vz < 0)
				{
					particlePtr->Velocity.vz += MUL_FIXED(1024,NormalFrameTime);
					if (particlePtr->Velocity.vz > 0) particlePtr->Velocity.vz = 0;
				}
				#endif
				
				particlePtr->Position.vx += MUL_FIXED
											(
												particlePtr->Velocity.vx+
												MUL_FIXED
												(
													-GetSin(((particlePtr->Position.vz+particlePtr->Position.vy)*16)&4095)/4,
													particlePtr->Offset.vx
												),
												NormalFrameTime
											
											);

				particlePtr->Position.vz += MUL_FIXED
											(
												particlePtr->Velocity.vz+
												MUL_FIXED
												(
													GetCos(((particlePtr->Position.vx+particlePtr->Position.vy)*16)&4095)/4,
													particlePtr->Offset.vz
												),
												NormalFrameTime
											
											);

				particlePtr->Size = MUL_FIXED(ONE_FIXED-particlePtr->LifeTime,48);
				{
					int colour = particlePtr->LifeTime>>11;
		  			particlePtr->Colour = RGBALIGHT_MAKE(32,32,32,colour);
				}												
				
				break;
			}
			case PARTICLE_FLAME:
			case PARTICLE_NONDAMAGINGFLAME:
			case PARTICLE_PARGEN_FLAME:
			{
				particlePtr->Size = 20+(ONE_FIXED/2-particlePtr->LifeTime)/64;	 
				if (particlePtr->LifeTime==ONE_FIXED/2)
				{
					switch (CurrentVisionMode)
					{
						default:
						case VISION_MODE_NORMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(16,16,128,(particlePtr->LifeTime>>8)|9);
							break;
						}
						case VISION_MODE_IMAGEINTENSIFIER:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,32,0,(particlePtr->LifeTime>>8)|9);
							break;
						}
						case VISION_MODE_PRED_THERMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,0,128,(particlePtr->LifeTime>>8)|9);
						  	break;
						}
						case VISION_MODE_PRED_SEEALIENS:
						case VISION_MODE_PRED_SEEPREDTECH:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(192,128,32,(particlePtr->LifeTime>>8)|9);
						  	break;
						}
					}
				}
				else
				{
					particlePtr->Colour = RGBALIGHT_MAKE
										  (
										  	particleDescPtr->RedScale[CurrentVisionMode],
										  	particleDescPtr->GreenScale[CurrentVisionMode],
										  	particleDescPtr->BlueScale[CurrentVisionMode],
											(particlePtr->LifeTime>>8)|9
										  );
				}

				
				break;
			}
			case PARTICLE_FIRE:
			{
				particlePtr->Size = 300-(FastRandom()&127);
				break;
			}
			case PARTICLE_EXPLOSIONFIRE:
			{
				VECTORCH obstacleNormal;
				int moduleIndex;

				if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex) && !(FastRandom()&15))
				{
					MakeDecal(DECAL_SCORCHED,&obstacleNormal,&(particlePtr->Position),moduleIndex);
					particlePtr->LifeTime=0;
				}
				break;
			}
			case PARTICLE_MOLOTOVFLAME:
			{
				VECTORCH obstacleNormal;
				int moduleIndex;
					
				particlePtr->Colour = RGBALIGHT_MAKE
									  (
									  	particleDescPtr->RedScale[CurrentVisionMode],
									  	particleDescPtr->GreenScale[CurrentVisionMode],
									  	particleDescPtr->BlueScale[CurrentVisionMode],
										(particlePtr->LifeTime>>10)|9
									  );

				particlePtr->Velocity.vy += MUL_FIXED(8000,NormalFrameTime);
				if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))
				{
					#if 0
					int magOfPerpImp = DotProduct(&obstacleNormal,&(particlePtr->Velocity));
					particlePtr->Velocity.vx -= MUL_FIXED(obstacleNormal.vx, magOfPerpImp);
					particlePtr->Velocity.vy -= MUL_FIXED(obstacleNormal.vy, magOfPerpImp);
					particlePtr->Velocity.vz -= MUL_FIXED(obstacleNormal.vz, magOfPerpImp);
					#endif
				}
				particlePtr->Size = 800-(FastRandom()&255);
				break;
			}


			case PARTICLE_NONCOLLIDINGFLAME:
			{				
				particlePtr->Size = 20+(ONE_FIXED/2-particlePtr->LifeTime)/64;	 
				break;
			}
			case PARTICLE_ORANGE_SPARK:
			{
				particlePtr->Offset = particlePtr->Position;

				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Velocity.vx -= MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Velocity.vy -= MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				particlePtr->Velocity.vz -= MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);

				{
					int l = particlePtr->LifeTime*8;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,l,0,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,128,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
						}
					}
				}

				break;
			}
			case PARTICLE_ORANGE_PLASMA:
			{
				particlePtr->Offset = particlePtr->Position;

				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Velocity.vx -= MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Velocity.vy -= MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				particlePtr->Velocity.vz -= MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);

				{
					int l = particlePtr->LifeTime*16;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,l,0,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,128,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
						}
					}
				}

				break;
			}
			case PARTICLE_RICOCHET_SPARK:
			{
				particlePtr->Offset = particlePtr->Position;

				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Velocity.vx -= MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Velocity.vy -= MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				particlePtr->Velocity.vz -= MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);

				{
					int l = particlePtr->LifeTime*8;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,l,0,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,128,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l,l,0,l);
							  	break;
							}

						}
					}
				}

				break;
			}

			case PARTICLE_SPARK:
			{
				particlePtr->Offset = particlePtr->Position;

				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Velocity.vx -= MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Velocity.vy += MUL_FIXED(5000,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				particlePtr->Velocity.vz -= MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);

				{
					int l = particlePtr->LifeTime*2;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,255,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,128,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l,l,0,l);
							  	break;
							}
						}
					}
				}
				break;
			}

			case PARTICLE_DEWLINE:
			{
				int l = (particlePtr->LifeTime)/64/4;
				if (l>255)
				{
					switch (CurrentVisionMode)
					{
						default:
						case VISION_MODE_NORMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,255,255,255);
							break;
						}
						case VISION_MODE_IMAGEINTENSIFIER:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,255,0,255);
							break;
						}
						case VISION_MODE_PRED_THERMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,0,0,255);
						  	break;
						}
						case VISION_MODE_PRED_SEEALIENS:
						case VISION_MODE_PRED_SEEPREDTECH:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,128,64,255);
						  	break;
						}
					}
				}
				else
				{
			 		particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			 		particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			 		particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
					switch (CurrentVisionMode)
					{
						default:
						case VISION_MODE_NORMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,128+(l/2),64+((3*l)/4),l);
							break;
						}
						case VISION_MODE_IMAGEINTENSIFIER:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,l);
							break;
						}
						case VISION_MODE_PRED_THERMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,l/2,l/2,l);
						  	break;
						}
						case VISION_MODE_PRED_SEEALIENS:
						case VISION_MODE_PRED_SEEPREDTECH:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,l/2,l/4,l);
						  	break;
						}
					}
					particlePtr->Size = 32+(FastRandom()&31);
				}

				break;
			}
			case PARTICLE_PLASMATRAIL:
			{
				int l = (particlePtr->LifeTime)/64/4;
				if (l>255)
				{
					switch (CurrentVisionMode)
					{
						default:
						case VISION_MODE_NORMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,0,255,255);
							break;
						}
						case VISION_MODE_IMAGEINTENSIFIER:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,255,0,255);
							break;
						}
						case VISION_MODE_PRED_THERMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(0,0,255,255);
						  	break;
						}
						case VISION_MODE_PRED_SEEALIENS:
						case VISION_MODE_PRED_SEEPREDTECH:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,128,64,255);
						  	break;
						}
					}
				}
				else
				{
			 		particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			 		particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			 		particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
					switch (CurrentVisionMode)
					{
						default:
						case VISION_MODE_NORMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(l/4,l/2,255,l);
							break;
						}
						case VISION_MODE_IMAGEINTENSIFIER:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,l);
							break;
						}
						case VISION_MODE_PRED_THERMAL:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(l/4,l/2,255,l);
						  	break;
						}
						case VISION_MODE_PRED_SEEALIENS:
						case VISION_MODE_PRED_SEEPREDTECH:
						{
							particlePtr->Colour = RGBALIGHT_MAKE(255,l/2,l/4,l);
						  	break;
						}
					}
					particlePtr->Size = 32+(FastRandom()&31);
				}

				break;
			}

			case PARTICLE_WATERSPRAY:
			{
				particlePtr->Colour = RGBALIGHT_MAKE
									  (
									  	particleDescPtr->RedScale[CurrentVisionMode],
									  	particleDescPtr->GreenScale[CurrentVisionMode],
									  	particleDescPtr->BlueScale[CurrentVisionMode],
									  	particleDescPtr->Alpha
									  );
				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				particlePtr->Velocity.vy += MUL_FIXED(10000,NormalFrameTime);

				break;
			}
			case PARTICLE_WATERFALLSPRAY:
			{
				extern int WaterFallBase;
				int y = particlePtr->Position.vy;
				particlePtr->Colour = RGBALIGHT_MAKE
									  (
									  	particleDescPtr->RedScale[CurrentVisionMode],
									  	particleDescPtr->GreenScale[CurrentVisionMode],
									  	particleDescPtr->BlueScale[CurrentVisionMode],
									  	particleDescPtr->Alpha
									  );

				particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
				particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
				particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
				
				particlePtr->Velocity.vy += MUL_FIXED(10000+(FastRandom()&511),NormalFrameTime);
				
				if(particlePtr->Position.vz < 54885 && particlePtr->Position.vx < 179427)
				{
					if (y<4742 && particlePtr->Position.vy>4742)
					{
						particlePtr->Position.vy=4742;
						particlePtr->Velocity.vy=-MUL_FIXED(particlePtr->Velocity.vy,ONE_FIXED/2-(FastRandom()&16384));						
					}
				}
				else if (particlePtr->Position.vz < 58600)
				{
					int l = DIV_FIXED(particlePtr->Position.vz - 54885,58600-54885);

					if (particlePtr->Position.vx < 179427 - MUL_FIXED(l,179427-175545))
					{
						int yThreshold = 4742 + MUL_FIXED(l,8635-4742);
						if (y<yThreshold && particlePtr->Position.vy>yThreshold)
						{
							particlePtr->Position.vy=yThreshold;
							particlePtr->Velocity.vy=-MUL_FIXED(particlePtr->Velocity.vy,ONE_FIXED/2-(FastRandom()&16384));						
						}
					}
				}

				particlePtr->Offset.vx = particlePtr->Position.vx - particlePtr->Velocity.vx/4;
				particlePtr->Offset.vy = particlePtr->Position.vy - particlePtr->Velocity.vy/4;
				particlePtr->Offset.vz = particlePtr->Position.vz - particlePtr->Velocity.vz/4;

				if (particlePtr->Position.vy>WaterFallBase)
				{
					particlePtr->LifeTime = 0;
				}

				break;
			}
			case PARTICLE_FLECHETTE:
			case PARTICLE_FLECHETTE_NONDAMAGING:
			{
				{
					int l = particlePtr->LifeTime/4;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE((l-255)/2+32,0,0,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(32,0,0,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
						}
					}
				}

				if (particlePtr->Offset.vy)//particlePtr->Velocity.vx || particlePtr->Velocity.vy || particlePtr->Velocity.vz)
				{
					VECTORCH obstacleNormal;
					int moduleIndex;
					VECTORCH velocityBackup = particlePtr->Velocity;
								
					if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))
					{
						if(moduleIndex==-1)
						{
							particlePtr->LifeTime=0;
						}
						else
						{
							particlePtr->Offset.vy = 0;
							particlePtr->Velocity = velocityBackup;
							particlePtr->Position.vx += particlePtr->Velocity.vx>>10;
							particlePtr->Position.vy += particlePtr->Velocity.vy>>10;
							particlePtr->Position.vz += particlePtr->Velocity.vz>>10;
						}
					}
				}

				break;
			}
			case PARTICLE_SMOKECLOUD:
			{
				if (particlePtr->LifeTime<ONE_FIXED*8)
				{
					int colour = (particlePtr->LifeTime/(8*256*4));
		  			particlePtr->Colour = RGBALIGHT_MAKE(255,255,255,colour);
				}
				else
				{
		  			particlePtr->Colour = RGBALIGHT_MAKE(255,255,255,64);
				}
				particlePtr->Size = 1000+500-(particlePtr->LifeTime>>10);
				
				AddEffectsOfForceGenerators(&particlePtr->Position,&particlePtr->Velocity,32*64);
				
				break;
			}
			case PARTICLE_BLUEPLASMASPHERE:
			{
				break;
			}
			case PARTICLE_ELECTRICALPLASMASPHERE:
			{
				{
					int colour = (particlePtr->LifeTime/128);
					particlePtr->Size = 200+(ONE_FIXED-particlePtr->LifeTime)/16;
		  			particlePtr->Colour = RGBALIGHT_MAKE(255,255,255,colour);
				}
				break;
			}
			case PARTICLE_PREDPISTOL_FLECHETTE:
			case PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING:
			{
				/* CDF 7/12/98 Placeholder till Kevin gets better. */
				
				/* Based on PARTICLE_SPARK for vision... */				
				particlePtr->Offset = particlePtr->Position;

				{
					int l = particlePtr->LifeTime*2;
					l = MUL_FIXED(255,l);

					if (l>255)
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,255,255);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l/2,255);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l-255,255,255,255);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(255,255,l-255,255);
							  	break;
							}
						}
					}
					else
					{
						switch (CurrentVisionMode)
						{
							default:
							case VISION_MODE_NORMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,l,l);
								break;
							}
							case VISION_MODE_IMAGEINTENSIFIER:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l/2,l/2,128,l);
								break;
							}
							case VISION_MODE_PRED_THERMAL:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(0,l,l,l);
							  	break;
							}
							case VISION_MODE_PRED_SEEALIENS:
							case VISION_MODE_PRED_SEEPREDTECH:
							{
			  					particlePtr->Colour = RGBALIGHT_MAKE(l,l,0,l);
							  	break;
							}
						}
					}
				}

				{
					VECTORCH obstacleNormal;
					int moduleIndex;

					if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))
					{
						if (moduleIndex!=-1)
						{
							MakeDecal(DECAL_SCORCHED,&obstacleNormal,&(particlePtr->Position),moduleIndex);
						}
						particlePtr->LifeTime=0;
					}
				}
				break;
			}
			case PARTICLE_TRACER:
			{
				break;
			}
			default:
			{
				/* particle initialised wrongly */
				LOCALASSERT(0);
				break;
			}
		}
		particlePtr++;
	}
	
	//
	PostLandscapeRendering();
	D3D_DecalSystem_Setup();
	OutputTranslucentPolyList();

	i = NumActiveParticles;
	particlePtr = ParticleStorage;
	while(i--)
	{
		particlePtr->LifeTime -= NormalFrameTime;
		
		if (particlePtr->LifeTime<=0)
		{
			enum PARTICLE_ID particleID = particlePtr->ParticleID;
			if( (particleID == PARTICLE_ALIEN_BLOOD) || (particleID == PARTICLE_PREDATOR_BLOOD) || (particleID==PARTICLE_HUMAN_BLOOD) || (particleID == PARTICLE_ANDROID_BLOOD))
			{
				NumberOfBloodParticles--;
				LOCALASSERT(NumberOfBloodParticles>=0);
			}
			
			if ((particleID == PARTICLE_NONCOLLIDINGFLAME) && (FastRandom()&65535)<4096)
			{
				VECTORCH zero = {0,0,0};
				MakeParticle(&(particlePtr->Position),&zero,PARTICLE_IMPACTSMOKE);
			}
			else if ((particleID == PARTICLE_MOLOTOVFLAME) && (FastRandom()&65535)<4096)
			{
				VECTORCH zero = {0,0,0};
				MakeParticle(&(particlePtr->Position),&zero,PARTICLE_IMPACTSMOKE);
			}


			DeallocateParticle(particlePtr);
		}
		else
		{
			particlePtr++;
		}
	}
	
	{
		extern int NumOnScreenBlocks;
		extern DISPLAYBLOCK *OnScreenBlockList[];
		int numOfObjects = NumOnScreenBlocks;
		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;

			if (!objectPtr->ObShape && objectPtr->SfxPtr)
			{
				DrawSfxObject(objectPtr);
			}
			if (objectPtr->HModelControlBlock)
			{
				SECTION_DATA *firstSectionPtr=objectPtr->HModelControlBlock->section_data;
				ScanHModelForDecals(objectPtr,firstSectionPtr);
			}
			if (sbPtr)
			{
				switch(sbPtr->I_SBtype)
				{
					case I_BehaviourPlacedLight:
					{		  
						PLACED_LIGHT_BEHAV_BLOCK* pl_bhv = sbPtr->SBdataptr;
					   	GLOBALASSERT(pl_bhv);
						GLOBALASSERT(sbPtr->containingModule);

	  					if (LocalDetailLevels.DrawLightCoronas
	  						&& pl_bhv->has_corona
	  						&& pl_bhv->light->LightBright
	  						&& (ModuleCurrVisArray[sbPtr->containingModule->m_index]==2)
	  						&& (pl_bhv->light->RedScale
							 || pl_bhv->light->GreenScale
	  					     || pl_bhv->light->BlueScale)
	  						)
						{
							VECTORCH position=pl_bhv->corona_location;
							RotateVector(&position,&objectPtr->ObMat);
							position.vx += objectPtr->ObWorld.vx;
							position.vy += objectPtr->ObWorld.vy;
							position.vz += objectPtr->ObWorld.vz;
							
							if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&position))
							{
								LIGHTBLOCK *lPtr = pl_bhv->light;
								int colour;
								switch (CurrentVisionMode)
								{
									default:
									case VISION_MODE_NORMAL:
									{
										int r = MUL_FIXED(lPtr->RedScale,lPtr->LightBright)>>8;
										int g = MUL_FIXED(lPtr->GreenScale,lPtr->LightBright)>>8;
										int b = MUL_FIXED(lPtr->BlueScale,lPtr->LightBright)>>8;
										if (r>255) r=255;
										if (g>255) g=255;
										if (b>255) b=255;
										colour = 0xff000000+(r<<16)+(g<<8)+(b);
										break;
									}
									case VISION_MODE_IMAGEINTENSIFIER:
									{
										colour = 0xffffffff;
										break;
									}
									case VISION_MODE_PRED_THERMAL:
									case VISION_MODE_PRED_SEEALIENS:
									case VISION_MODE_PRED_SEEPREDTECH:
									{
										int b = MUL_FIXED
												(
													lPtr->RedScale+lPtr->GreenScale+lPtr->BlueScale,
													lPtr->LightBright
												)>>10;
										if (b>255) b=255;

										colour = 0xff000000+(b<<16)+((b>>1)<<8);
									  	break;
									}
								}
								RenderLightFlare(&position,colour);
							}
						}

						break;
					}
					case I_BehaviourFlareGrenade:
					{		  
						if (LocalDetailLevels.DrawLightCoronas)
						{						
							DoFlareCorona(objectPtr);
						}
						break;
					}
					case I_BehaviourNetGhost:
					{
			   			NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;

						if (ghostDataPtr->type==I_BehaviourFlareGrenade && LocalDetailLevels.DrawLightCoronas)
						{
							DoFlareCorona(objectPtr);
						}
						else if (ghostDataPtr->type==I_BehaviourFrisbee)
						{
							if (sbPtr->DynPtr)
							{
								if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&(sbPtr->DynPtr->Position)))
								{
									int colour;
									switch (CurrentVisionMode)
									{
										default:
										case VISION_MODE_NORMAL:
										{
											colour = 0x40ffffff;
											break;
										}
										case VISION_MODE_IMAGEINTENSIFIER:
										{
											colour = 0x40ffffff;
											break;
										}
										case VISION_MODE_PRED_THERMAL:
										case VISION_MODE_PRED_SEEALIENS:
										case VISION_MODE_PRED_SEEPREDTECH:
										{
											colour = 0x40ff8000;
										  	break;
										}
									}
									RenderLightFlare(&(sbPtr->DynPtr->Position),colour);
								}
							}
						}
						break;
					}
					/* CDF 21/7/99 Frisbee Laser? */
					case I_BehaviourFrisbee:
					{
						if (sbPtr->DynPtr)
						{
							if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&(sbPtr->DynPtr->Position)))
							{
								int colour;
								switch (CurrentVisionMode)
								{
									default:
									case VISION_MODE_NORMAL:
									{
										colour = 0x40ffffff;
										break;
									}
									case VISION_MODE_IMAGEINTENSIFIER:
									{
										colour = 0x40ffffff;
										break;
									}
									case VISION_MODE_PRED_THERMAL:
									case VISION_MODE_PRED_SEEALIENS:
									case VISION_MODE_PRED_SEEPREDTECH:
									{
										colour = 0x40ff8000;
									  	break;
									}
								}
								RenderLightFlare(&(sbPtr->DynPtr->Position),colour);
							}
						}
						break;
					}		 
					/* CDF 21/7/99 Frisbee Laser? */
					case I_BehaviourXenoborg:
					{
						XENO_STATUS_BLOCK *statusPtr = (XENO_STATUS_BLOCK *)sbPtr->SBdataptr;
						LASER_BEAM_DESC *laserPtr = statusPtr->TargetingLaser;
						int i = 2;
						do
						{
							if (laserPtr->BeamIsOn)
							{
								if (laserPtr->BeamHasHitPlayer)
								{			  
									int colour;
									switch (CurrentVisionMode)
									{
										default:
										case VISION_MODE_NORMAL:
										{
											colour = 0xffff0000;
											break;
										}
										case VISION_MODE_IMAGEINTENSIFIER:
										{
											colour = 0xffffffff;
											break;
										}
										case VISION_MODE_PRED_THERMAL:
										case VISION_MODE_PRED_SEEALIENS:
										case VISION_MODE_PRED_SEEPREDTECH:
										{
											colour = 0xffff8000;
										  	break;
										}
									}
									RenderLightFlare(&(laserPtr->SourcePosition),colour);
								}
								else
								{
									PARTICLE particle;
									switch (CurrentVisionMode)
									{
										default:
										case VISION_MODE_NORMAL:
										{
											particle.Colour = RGBALIGHT_MAKE(255,0,0,255);
											break;
										}
										case VISION_MODE_IMAGEINTENSIFIER:
										{
						  					particle.Colour = RGBALIGHT_MAKE(255,255,255,255);
											break;
										}
										case VISION_MODE_PRED_THERMAL:
										case VISION_MODE_PRED_SEEALIENS:
										case VISION_MODE_PRED_SEEPREDTECH:
										{
						  					particle.Colour = RGBALIGHT_MAKE(255,128,0,255);
										  	break;
										}
									}
									particle.ParticleID = PARTICLE_LASERBEAM;
									particle.Position = laserPtr->SourcePosition;
									particle.Offset = laserPtr->TargetPosition;
									particle.Size = 20;
									RenderParticle(&particle);
								}
							}
							laserPtr++;
						}
						while(i--);
						
						if (statusPtr->LeftMainBeam.BeamIsOn)
						{
							DrawXenoborgMainLaserbeam(&statusPtr->LeftMainBeam);
						}
						if (statusPtr->RightMainBeam.BeamIsOn)
						{
							DrawXenoborgMainLaserbeam(&statusPtr->RightMainBeam);
						}
						break;
					}		 
					case I_BehaviourSpeargunBolt:
					{
					    SPEAR_BEHAV_BLOCK *bbPtr = (SPEAR_BEHAV_BLOCK * ) sbPtr->SBdataptr;

						if (bbPtr->SpearThroughFragment)
						{
							DISPLAYBLOCK displayblock;
							displayblock.ObWorld.vx=bbPtr->Position.vx+sbPtr->DynPtr->Position.vx;
							displayblock.ObWorld.vy=bbPtr->Position.vy+sbPtr->DynPtr->Position.vy;
							displayblock.ObWorld.vz=bbPtr->Position.vz+sbPtr->DynPtr->Position.vz;
							displayblock.ObMat=bbPtr->Orient;
							displayblock.ObShape=GetLoadedShapeMSL("spear");
							displayblock.ObShapeData=GetShapeData(displayblock.ObShape);

							displayblock.name=NULL;
							displayblock.ObEuler.EulerX=0;
							displayblock.ObEuler.EulerY=0;
							displayblock.ObEuler.EulerZ=0;
							displayblock.ObFlags=0;
							displayblock.ObFlags2=0;
							displayblock.ObFlags3=0;
							displayblock.ObNumLights=0;
							displayblock.ObRadius=0;
							displayblock.ObMaxX=0;
							displayblock.ObMinX=0;
							displayblock.ObMaxY=0;
							displayblock.ObMinY=0;
							displayblock.ObMaxZ=0;
							displayblock.ObMinZ=0;
							displayblock.ObTxAnimCtrlBlks=NULL;
							displayblock.ObEIDPtr=NULL;
							displayblock.ObMorphCtrl=NULL;
							displayblock.ObStrategyBlock=NULL;
							displayblock.ShapeAnimControlBlock=NULL;
							displayblock.HModelControlBlock=NULL;
							displayblock.ObMyModule=NULL;		
							displayblock.SpecialFXFlags = 0;
							displayblock.SfxPtr=0;

							MakeVector(&displayblock.ObWorld, &Global_VDB_Ptr->VDB_World, &displayblock.ObView);
							RotateVector(&displayblock.ObView, &Global_VDB_Ptr->VDB_Mat);
							RenderThisDisplayblock(&displayblock);
							
						}
						break;
					}
					default:
					{
						/* KJL 19:17:48 31/07/98 - check for hmodels */
						break;
					}
				}
			}
		}
	}
	HandlePheromoneTrails();
	{
		int i;
		for(i=0; i<MAX_NO_OF_EXPLOSIONS; i++)
		{
			if (ExplosionStorage[i].LifeTime)
				HandleVolumetricExplosion(&ExplosionStorage[i]);
		}
	}
	//RenderBoom();
   	//RenderFog();
	D3D_DecalSystem_End();
	
}

void RenderAllParticlesFurtherAwayThan(int zThreshold)
{
	/* now render particles */
	int i = NumActiveParticles;
	PARTICLE *particlePtr = ParticleStorage;
	while(i--)
	{
		if (particlePtr->NotYetRendered)
		{
			VECTORCH position = particlePtr->Position;
			TranslatePointIntoViewspace(&position);

			if (position.vz>zThreshold)
			{
				particlePtr->NotYetRendered = 0;
				switch(particlePtr->ParticleID)
				{
					case PARTICLE_ALIEN_BLOOD:
					{
						RenderParticle(particlePtr);
						{
							VECTORCH obstacleNormal;
							int moduleIndex;
							#if 1
							if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))
							{
								if(moduleIndex!=-1)
								{	
									#if 1
									int i = 1;
									MATRIXCH orientation;
									MakeMatrixFromDirection(&obstacleNormal,&orientation);
									while(i--)
									{
										VECTORCH velocity;
										velocity.vx = ((FastRandom()&1023) - 512);
										velocity.vy = ((FastRandom()&1023) - 512);
										velocity.vz = (255+(FastRandom()&255));
										RotateVector(&velocity,&orientation);
										MakeParticle(&(particlePtr->Position),&(velocity),PARTICLE_IMPACTSMOKE);
									}
									#endif
						 			MakeDecal(DECAL_SCORCHED,&obstacleNormal,&(particlePtr->Position),moduleIndex);
								}
								particlePtr->LifeTime = 0;
							}
							#endif
						}

						particlePtr->Velocity.vy += MUL_FIXED(10000,NormalFrameTime);
						AddEffectsOfForceGenerators(&particlePtr->Position,&particlePtr->Velocity,32*16);

						break;
					}
					case PARTICLE_PREDATOR_BLOOD:
					{
						RenderParticle(particlePtr);
						{
							VECTORCH obstacleNormal;
							int moduleIndex;

							if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))//&& !(FastRandom()&15))
							{
								if(moduleIndex!=-1)
								{
									MakeDecal(DECAL_PREDATOR_BLOOD,&obstacleNormal,&(particlePtr->Position),moduleIndex);
								}
								particlePtr->LifeTime = 0;
							}
						}
						
						particlePtr->Velocity.vy += MUL_FIXED(10000,NormalFrameTime);
						AddEffectsOfForceGenerators(&particlePtr->Position,&particlePtr->Velocity,32*16);
						break;

					}
					case PARTICLE_HUMAN_BLOOD:
					{
						RenderParticle(particlePtr);
						{
							VECTORCH obstacleNormal;
							int moduleIndex;

							if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))//&& !(FastRandom()&15))
							{
								if(moduleIndex!=-1)
								{
									MakeDecal(DECAL_HUMAN_BLOOD,&obstacleNormal,&(particlePtr->Position),moduleIndex);
								}
								particlePtr->LifeTime = 0;
							}
						}
						
						particlePtr->Velocity.vy += MUL_FIXED(10000,NormalFrameTime);
						AddEffectsOfForceGenerators(&particlePtr->Position,&particlePtr->Velocity,32*16);
						break;
					}
					case PARTICLE_ANDROID_BLOOD:
					{
						RenderParticle(particlePtr);
						{
							VECTORCH obstacleNormal;
							int moduleIndex;

							if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex))//&& !(FastRandom()&15))
							{
								if(moduleIndex!=-1)
								{
									MakeDecal(DECAL_ANDROID_BLOOD,&obstacleNormal,&(particlePtr->Position),moduleIndex);
								}
								particlePtr->LifeTime = 0;
							}
						}
						
						particlePtr->Velocity.vy += MUL_FIXED(10000,NormalFrameTime);
						AddEffectsOfForceGenerators(&particlePtr->Position,&particlePtr->Velocity,32*16);
						break;
					}
					case PARTICLE_FLARESMOKE:
					{
					 	RenderParticle(particlePtr);
						{
							VECTORCH impulse={0,0,0};
							int t = MUL_FIXED(NormalFrameTime,NormalFrameTime*4);
							AddEffectsOfForceGenerators(&particlePtr->Position,&impulse,8);
							particlePtr->Position.vx += MUL_FIXED(impulse.vx,t);
							particlePtr->Position.vy += MUL_FIXED(impulse.vy,t);
							particlePtr->Position.vz += MUL_FIXED(impulse.vz,t);

						}
						break;
					}
					case PARTICLE_FLAME:
					case PARTICLE_NONDAMAGINGFLAME:
					case PARTICLE_PARGEN_FLAME:
					{
					   	RenderParticle(particlePtr);
						{
							VECTORCH obstacleNormal;
							int moduleIndex;

							if(ParticleDynamics(particlePtr,&obstacleNormal,&moduleIndex) && !(FastRandom()&15))
							{
								if (particlePtr->ParticleID!=PARTICLE_NONDAMAGINGFLAME) {
									if(moduleIndex!=-1)
									{
										MakeDecal(DECAL_SCORCHED,&obstacleNormal,&(particlePtr->Position),moduleIndex);
									}
								}
							}
						}
						particlePtr->Velocity.vy -= MUL_FIXED(8000,NormalFrameTime);
						break;
					}
					case PARTICLE_FIRE:
					{
						RenderParticle(particlePtr);

						particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
						particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
						particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
						particlePtr->Velocity.vy -= MUL_FIXED(4000,NormalFrameTime);
						
						break;
					}
					case PARTICLE_NONCOLLIDINGFLAME:
					{				
					   	RenderParticle(particlePtr);
						
						particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
						particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
						particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
						particlePtr->Velocity.vy -= MUL_FIXED(8000,NormalFrameTime);
						break;
					}
					case PARTICLE_FLECHETTE:
					case PARTICLE_FLECHETTE_NONDAMAGING:
					{
						RenderFlechetteParticle(particlePtr);
						break;
					}
					case PARTICLE_STEAM:
					case PARTICLE_BLACKSMOKE:
					case PARTICLE_IMPACTSMOKE:
					case PARTICLE_GUNMUZZLE_SMOKE:
					case PARTICLE_EXPLOSIONFIRE:
					case PARTICLE_MOLOTOVFLAME:
					case PARTICLE_ORANGE_SPARK:
					case PARTICLE_ORANGE_PLASMA:
					case PARTICLE_RICOCHET_SPARK:
					case PARTICLE_SPARK:
					case PARTICLE_PLASMATRAIL:
					case PARTICLE_DEWLINE:
					case PARTICLE_WATERSPRAY:
					case PARTICLE_WATERFALLSPRAY:
					case PARTICLE_SMOKECLOUD:
					case PARTICLE_BLUEPLASMASPHERE:
					case PARTICLE_ELECTRICALPLASMASPHERE:
					case PARTICLE_PREDPISTOL_FLECHETTE:
					case PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING:
					case PARTICLE_TRACER:
					{
						RenderParticle(particlePtr);
						break;
					}
					default:
					{
						/* particle initialised wrongly */
						LOCALASSERT(0);
						break;
					}
				}
			}
		}
		particlePtr++;
	}
}
void DoFlareCorona(DISPLAYBLOCK *objectPtr)
{
	VECTORCH position=objectPtr->ObWorld;
	
	if (CameraCanSeeThisPosition_WithIgnore(objectPtr,&position))
	{
		LIGHTBLOCK *lPtr = objectPtr->ObLights[0];
		int a = lPtr->LightBright>>8;
		int colour;

		if (a>255) a=255;
		
		switch (CurrentVisionMode)
		{
			default:
			case VISION_MODE_NORMAL:
			{
				colour = 0xffc8ff + (a<<24);
				break;
			}
			case VISION_MODE_IMAGEINTENSIFIER:
			{
				colour = 0xffffffff;
				break;
			}
			case VISION_MODE_PRED_THERMAL:
			case VISION_MODE_PRED_SEEALIENS:
			case VISION_MODE_PRED_SEEPREDTECH:
			{
				int b = MUL_FIXED
						(
							lPtr->RedScale+lPtr->GreenScale+lPtr->BlueScale,
							lPtr->LightBright
						)>>10;
				if (b>255) b=255;

				colour = 0xff000000+(b<<16)+((b>>1)<<8);
			  	break;
			}
		}
		RenderLightFlare(&position,colour);
	}
}
#define MAX_RAINDROPS 1000
static PARTICLE RainDropStorage[MAX_RAINDROPS];
#define MAX_NO_OF_RIPPLES 100
RIPPLE RippleStorage[MAX_NO_OF_RIPPLES];
int ActiveRippleNumber;
void InitialiseRainDrops(void)
{
	{
		int i = MAX_RAINDROPS;
		PARTICLE *particlePtr = RainDropStorage;
		do
		{
			particlePtr->Position.vy = 0x7fffffff;
			particlePtr->LifeTime = 0;
			particlePtr++;
		}
		while(--i);
	}
	{
		int i = MAX_NO_OF_RIPPLES;
		RIPPLE *ripplePtr = RippleStorage;
		do
		{
			ripplePtr->Active = 0;
			ripplePtr++;
		}
		while(--i);
		ActiveRippleNumber=0;
	}
}

void HandleRainDrops(MODULE *modulePtr,int numberOfRaindrops)
{
	int i = numberOfRaindrops;

	PARTICLE *particlePtr = RainDropStorage;
	LOCALASSERT(i<MAX_RAINDROPS);
	do
	{
		if((particlePtr->Position.vy > modulePtr->m_world.vy+modulePtr->m_maxy-500)
		 ||(particlePtr->Position.vx < modulePtr->m_world.vx+modulePtr->m_minx)
		 ||(particlePtr->Position.vx > modulePtr->m_world.vx+modulePtr->m_maxx)
		 ||(particlePtr->Position.vz < modulePtr->m_world.vz+modulePtr->m_minz)
		 ||(particlePtr->Position.vz > modulePtr->m_world.vz+modulePtr->m_maxz))
		{
			AddRipple(particlePtr->Position.vx,particlePtr->Position.vz,400);
			particlePtr->Position.vy = modulePtr->m_world.vy+modulePtr->m_miny;
			particlePtr->Position.vx = modulePtr->m_world.vx+modulePtr->m_minx+(FastRandom()%(modulePtr->m_maxz-modulePtr->m_minx));
			particlePtr->Position.vz = modulePtr->m_world.vz+modulePtr->m_minz+(FastRandom()%(modulePtr->m_maxz-modulePtr->m_minz));
  //		particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
  //		particlePtr->Velocity.vx = (FastRandom()&255)+5000;
 //			particlePtr->Velocity.vz = (FastRandom()&255)-128;
			particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
			particlePtr->Velocity.vx = (FastRandom()&255)-128;
			particlePtr->Velocity.vz = (FastRandom()&255)-128;
			{
				particlePtr->Offset.vx = -particlePtr->Velocity.vz;
				particlePtr->Offset.vy = 0;
				particlePtr->Offset.vz = particlePtr->Velocity.vx;
				Normalise(&(particlePtr->Offset));
//				particlePtr->Offset.vx = MUL_FIXED(particlePtr->Offset.vx,20);
//				particlePtr->Offset.vz = MUL_FIXED(particlePtr->Offset.vz,20);
				particlePtr->Offset.vx = MUL_FIXED(particlePtr->Offset.vx,50);
				particlePtr->Offset.vz = MUL_FIXED(particlePtr->Offset.vz,50);
			}
		}
		{
			VECTORCH prevPosition = particlePtr->Position;

			#if 0
			ParticleDynamics(particlePtr);
			#else
			particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
			#endif
			if (particlePtr->Position.vy>modulePtr->m_world.vy+modulePtr->m_maxy-500)
				particlePtr->Position.vy=modulePtr->m_world.vy+modulePtr->m_maxy-495;
			D3D_DrawParticle_Rain(particlePtr,&prevPosition);
		}
		particlePtr++;
	}
	while(--i);
	
}
#if 0
void HandleRain(MODULE *modulePtr,int numberOfRaindrops)
{
	int i = numberOfRaindrops;
	/* KJL 15:23:37 12/8/97 - this is written to work with the yard in genshd1 */

	PARTICLE *particlePtr = RainDropStorage;
	LOCALASSERT(i<MAX_RAINDROPS);
	do
	{
		int killDrop=0;
		
		if((particlePtr->Position.vx > -10418)
		 &&(particlePtr->Position.vy > -4030)
		 &&(particlePtr->Position.vz < 35070)
		 &&(particlePtr->Position.vz > -11600))
		{
			killDrop=1;
		}
		
		if((particlePtr->Position.vy > 3000)
		 ||(particlePtr->Position.vx < modulePtr->m_world.vx+modulePtr->m_minx)
		 ||(particlePtr->Position.vx > modulePtr->m_world.vx+modulePtr->m_maxx)
		 ||(particlePtr->Position.vz < -50655)//modulePtr->m_world.vz+modulePtr->m_minz)
		 ||(particlePtr->Position.vz > modulePtr->m_world.vz+modulePtr->m_maxz))
		{
			killDrop=1;
		}
		
		if (killDrop)
		{
			particlePtr->Position.vy = modulePtr->m_world.vy+modulePtr->m_miny;
			particlePtr->Position.vx = modulePtr->m_world.vx+modulePtr->m_minx+(FastRandom()%(modulePtr->m_maxz-modulePtr->m_minx));
			particlePtr->Position.vz = -50655/*+modulePtr->m_minz*/+(FastRandom()%(modulePtr->m_maxz+50655));//-modulePtr->m_minz));
	  		particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
  			particlePtr->Velocity.vx = (FastRandom()&255)+5000;
 			particlePtr->Velocity.vz = (FastRandom()&255)-128;
			{
				particlePtr->Offset.vx = -particlePtr->Velocity.vz;
				particlePtr->Offset.vy = 0;
				particlePtr->Offset.vz = particlePtr->Velocity.vx;
				Normalise(&(particlePtr->Offset));
				particlePtr->Offset.vx = MUL_FIXED(particlePtr->Offset.vx,20);
				particlePtr->Offset.vz = MUL_FIXED(particlePtr->Offset.vz,20);
			}
		}
		{
			VECTORCH prevPosition = particlePtr->Position;

			#if 0
			ParticleDynamics(particlePtr);
			#else
			particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
			#endif
			D3D_DrawParticle_Rain(particlePtr,&prevPosition);
		}
		particlePtr++;
	}
	while(--i);
	
}
#endif
void HandleRain(int numberOfRaindrops)
{
	int i = numberOfRaindrops;
	/* KJL 15:23:37 12/8/97 - this is written to work with the yard in genshd1 */

	PARTICLE *particlePtr = RainDropStorage;
	LOCALASSERT(i<MAX_RAINDROPS);
	do
	{
		int killDrop=0;
		
		if((particlePtr->Position.vx > -10418)
		 &&(particlePtr->Position.vy > -4030)
		 &&(particlePtr->Position.vz < 35070)
		 &&(particlePtr->Position.vz > -11600))
		{
			killDrop=1;
		}
		if((particlePtr->Position.vx > -45486)
		 &&(particlePtr->Position.vx < -29901)
		 &&(particlePtr->Position.vy > -6000)
		 &&(particlePtr->Position.vz < -50656)
		 &&(particlePtr->Position.vz > -70130))
		{
			killDrop=1;
		}
		
		if((particlePtr->Position.vy > 3000)
		 ||(particlePtr->Position.vx < -77000)
		 ||(particlePtr->Position.vx > 134000)
		 ||(particlePtr->Position.vz < -145000)//modulePtr->m_world.vz+modulePtr->m_minz)
		 ||(particlePtr->Position.vz > 48706))
		{
			killDrop=1;
		}
		
		if (killDrop)
		{
			particlePtr->Position.vy = -10000;
			particlePtr->Position.vx = -77000+(FastRandom()%(211000));
			particlePtr->Position.vz = -145000+(FastRandom()%(145000+49000));
	  		particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
  			particlePtr->Velocity.vx = (FastRandom()&255)+5000;
 			particlePtr->Velocity.vz = (FastRandom()&255)-128;
			{
				particlePtr->Offset.vx = -particlePtr->Velocity.vz;
				particlePtr->Offset.vy = 0;
				particlePtr->Offset.vz = particlePtr->Velocity.vx;
				Normalise(&(particlePtr->Offset));
				particlePtr->Offset.vx = MUL_FIXED(particlePtr->Offset.vx,20);
				particlePtr->Offset.vz = MUL_FIXED(particlePtr->Offset.vz,20);
			}
		}
		{
			VECTORCH prevPosition = particlePtr->Position;

			#if 0
			ParticleDynamics(particlePtr);
			#else
			particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
			#endif
			D3D_DrawParticle_Rain(particlePtr,&prevPosition);
		}
		particlePtr++;
	}
	while(--i);
	
}
void HandleRainInTrench(int numberOfRaindrops)
{
	int i = numberOfRaindrops;
	/* KJL 15:23:37 12/8/97 - this is written to work with the yard in genshd1 */

	PARTICLE *particlePtr = RainDropStorage;
	LOCALASSERT(i<MAX_RAINDROPS);
	do
	{
		int killDrop=0;
		/*
		if((particlePtr->Position.vx > -10418)
		 &&(particlePtr->Position.vy > -4030)
		 &&(particlePtr->Position.vz < 35070)
		 &&(particlePtr->Position.vz > -11600))
		{
			killDrop=1;
		}
		*/
		if((particlePtr->Position.vy > 3000)
		 ||(particlePtr->Position.vx < 13500)
		 ||(particlePtr->Position.vx > -50000)
		 ||(particlePtr->Position.vz < -150655)
		 ||(particlePtr->Position.vz > -50655))
		{
			killDrop=1;
		}
		
		if (killDrop)
		{
			particlePtr->Position.vy = -16000;
			particlePtr->Position.vx = -50000+(FastRandom()%(63500));
			particlePtr->Position.vz = -150655+(FastRandom()%(100000));
	  		particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
  			particlePtr->Velocity.vx = (FastRandom()&255)+5000;
 			particlePtr->Velocity.vz = (FastRandom()&255)-128;
			{
				particlePtr->Offset.vx = -particlePtr->Velocity.vz;
				particlePtr->Offset.vy = 0;
				particlePtr->Offset.vz = particlePtr->Velocity.vx;
				Normalise(&(particlePtr->Offset));
				particlePtr->Offset.vx = MUL_FIXED(particlePtr->Offset.vx,20);
				particlePtr->Offset.vz = MUL_FIXED(particlePtr->Offset.vz,20);
			}
		}
		{
			VECTORCH prevPosition = particlePtr->Position;

			#if 0
			ParticleDynamics(particlePtr);
			#else
			particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
			#endif
			D3D_DrawParticle_Rain(particlePtr,&prevPosition);
		}
		particlePtr++;
	}
	while(--i);
	
}

void HandleRainShaft(MODULE *modulePtr, int bottomY, int topY, int numberOfRaindrops)
{
	int i = numberOfRaindrops;
	PARTICLE_DESC *particleDescPtr = &ParticleDescription[PARTICLE_WATERFALLSPRAY];

	PARTICLE *particlePtr = RainDropStorage;
	LOCALASSERT(i<MAX_RAINDROPS);
	do
	{
		if((particlePtr->Position.vx < modulePtr->m_world.vx+modulePtr->m_minx)
		 ||(particlePtr->Position.vx > modulePtr->m_world.vx+modulePtr->m_maxx)
		 ||(particlePtr->Position.vz < modulePtr->m_world.vz+modulePtr->m_minz)
		 ||(particlePtr->Position.vz > modulePtr->m_world.vz+modulePtr->m_maxz))
		{
			particlePtr->LifeTime=0;
		}
		else if(particlePtr->Position.vy > bottomY)
		{
			particlePtr->Position.vy = bottomY;
	  		particlePtr->Velocity.vy = -particlePtr->Velocity.vy;
  			particlePtr->Velocity.vx = particlePtr->Velocity.vx;
 			particlePtr->Velocity.vz = particlePtr->Velocity.vz;
			particlePtr->LifeTime = 1;
			particlePtr->Size = 100;
			AddRipple(particlePtr->Position.vx,particlePtr->Position.vz,100);
		}
	
		if (particlePtr->LifeTime<=0)
		{
			particlePtr->Position.vy = topY;
			particlePtr->Position.vx = modulePtr->m_world.vx+modulePtr->m_minx+(FastRandom()%(modulePtr->m_maxz-modulePtr->m_minx));
			particlePtr->Position.vz = modulePtr->m_world.vz+modulePtr->m_minz+(FastRandom()%(modulePtr->m_maxz-modulePtr->m_minz));
	  		particlePtr->Velocity.vy = (FastRandom()&8191)+15000;
  			particlePtr->Velocity.vx = (FastRandom()&1023)-512;
 			particlePtr->Velocity.vz = (FastRandom()&1023)-512;
			particlePtr->LifeTime = 100*ONE_FIXED;
			particlePtr->ParticleID = PARTICLE_WATERFALLSPRAY;
			particlePtr->Colour = RGBALIGHT_MAKE
								  (
								  	particleDescPtr->RedScale[CurrentVisionMode],
								  	particleDescPtr->GreenScale[CurrentVisionMode],
								  	particleDescPtr->BlueScale[CurrentVisionMode],
								  	particleDescPtr->Alpha
								  );
			particlePtr->Size = 20;
		}
		else
		{
			particlePtr->LifeTime -= NormalFrameTime;
		}

		{
			particlePtr->Position.vx += MUL_FIXED(particlePtr->Velocity.vx,NormalFrameTime);
			particlePtr->Position.vy += MUL_FIXED(particlePtr->Velocity.vy,NormalFrameTime);
			particlePtr->Position.vz += MUL_FIXED(particlePtr->Velocity.vz,NormalFrameTime);
#if 0
			D3D_DrawParticle_Rain(particlePtr,&prevPosition);
#else
			particlePtr->Offset.vx = particlePtr->Position.vx - particlePtr->Velocity.vx/16;
			particlePtr->Offset.vy = particlePtr->Position.vy - particlePtr->Velocity.vy/16;
			particlePtr->Offset.vz = particlePtr->Position.vz - particlePtr->Velocity.vz/16;

			RenderParticle(particlePtr);
#endif
		}
		particlePtr++;
	}
	while(--i);
	
}

void HandleRipples(void)
{
	int i;

	for(i=0; i<MAX_NO_OF_RIPPLES; i++)
	{
		if (RippleStorage[i].Active)
		{	
			RippleStorage[i].Radius	+= MUL_FIXED(2400,NormalFrameTime);
			RippleStorage[i].InvRadius = DIV_FIXED(4090,RippleStorage[i].Radius);
			RippleStorage[i].Amplitude -= MUL_FIXED(50,NormalFrameTime);
			
			if (RippleStorage[i].Amplitude<0)
			{
				RippleStorage[i].Active = 0;
			}
		}
	}
}

int EffectOfRipples(VECTORCH *point)
{
	int offset;
	int i;
 	offset = GetSin((point->vx+point->vz+CloakingPhase)&4095)>>11;
 	offset += GetSin((point->vx-point->vz*2+CloakingPhase/2)&4095)>>12;

	for(i=0; i<MAX_NO_OF_RIPPLES; i++)
	{
		if (RippleStorage[i].Active)
		{
			int dx=point->vx-RippleStorage[i].X;
			int dz=point->vz-RippleStorage[i].Z;

			if (dx<0) dx = -dx;
			if (dz<0) dz = -dz;
			{
				int a;

				if (dx>dz)
				{
					a = dx+(dz>>1);
				}
				else
				{
					a = dz+(dx>>1);
				}

				if (a<RippleStorage[i].Radius)
				{
					a = MUL_FIXED(a,RippleStorage[i].InvRadius);

					offset+= MUL_FIXED
							 (
							 	RippleStorage[i].Amplitude,
							 	GetSin(a)
							 );
				}
			}
		}
	}
	
	if (offset>256) offset = 256;
	else if (offset<-256) offset = -256;
	return offset;
}


void AddRipple(int x,int z,int amplitude)
{
	RippleStorage[ActiveRippleNumber].Active=1;
	RippleStorage[ActiveRippleNumber].X = x;
	RippleStorage[ActiveRippleNumber].Z = z;
	RippleStorage[ActiveRippleNumber].Radius = 200;
	RippleStorage[ActiveRippleNumber].Amplitude = amplitude;

	ActiveRippleNumber++;
	if (ActiveRippleNumber == MAX_NO_OF_RIPPLES) ActiveRippleNumber=0;

}


void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY)
{
   	extern int NumActiveBlocks;
	extern DISPLAYBLOCK* ActiveBlockList[];
   	int numberOfObjects = NumActiveBlocks;
	
   	while (numberOfObjects--)
	{
		DISPLAYBLOCK* objectPtr = ActiveBlockList[numberOfObjects];

		if(objectPtr->ObStrategyBlock)
		{
			DYNAMICSBLOCK *dynPtr = objectPtr->ObStrategyBlock->DynPtr;


			if (dynPtr)
			{
				int overlapInY=0;
				int overlapInX=0;
				int overlapInZ=0;

				/* floating objects are ignored to avoid positive feedback */
				if(dynPtr->IsFloating) continue;
				#if 1
				if ( (dynPtr->Position.vx==dynPtr->PrevPosition.vx)
				   &&(dynPtr->Position.vy==dynPtr->PrevPosition.vy)
				   &&(dynPtr->Position.vz==dynPtr->PrevPosition.vz) )
				   continue;
				#endif

				if (dynPtr->Position.vy>dynPtr->PrevPosition.vy)
				{
					if ((dynPtr->Position.vy+objectPtr->ObRadius > averageY)
					  &&(dynPtr->PrevPosition.vy-objectPtr->ObRadius < averageY))
					{
						overlapInY=1;
					}
				}
				else
				{
					if ((dynPtr->PrevPosition.vy+objectPtr->ObRadius > averageY)
					  &&(dynPtr->Position.vy-objectPtr->ObRadius < averageY))
					{
						overlapInY=1;
					}
				}

				if (!overlapInY) continue;

				if (dynPtr->Position.vx>dynPtr->PrevPosition.vx)
				{
					if ((dynPtr->Position.vx+objectPtr->ObRadius > minX)
					  &&(dynPtr->PrevPosition.vx-objectPtr->ObRadius < maxX))
					{
						overlapInX=1;
					}
				}
				else
				{
					if ((dynPtr->PrevPosition.vx+objectPtr->ObRadius > minX)
					  &&(dynPtr->Position.vx-objectPtr->ObRadius < maxX))
					{
						overlapInX=1;
					}
				}
				
				if (!overlapInX) continue;
				
				if (dynPtr->Position.vz>dynPtr->PrevPosition.vz)
				{
					if ((dynPtr->Position.vz+objectPtr->ObRadius > minZ)
					  &&(dynPtr->PrevPosition.vz-objectPtr->ObRadius < maxZ))
					{
						overlapInZ=1;
					}
				}
				else
				{
					if ((dynPtr->PrevPosition.vz+objectPtr->ObRadius > minZ)
					  &&(dynPtr->Position.vz-objectPtr->ObRadius < maxZ))
					{
						overlapInZ=1;
					}
				}

				if (!overlapInZ) continue;

				/* we have an overlap */
				
				/* KJL 16:37:29 27/08/98 - if object is on fire its now put out */
				objectPtr->ObStrategyBlock->SBDamageBlock.IsOnFire=0;

				if (objectPtr->ObStrategyBlock->I_SBtype == I_BehaviourFlareGrenade)
				{
					VECTORCH upwards = {0,-65536,0};
					dynPtr->IsFloating = 1;
					dynPtr->GravityOn = 0;
					dynPtr->Elasticity = 0;
					MakeMatrixFromDirection(&upwards,&(dynPtr->OrientMat));
				}
				else if (objectPtr == Player)
				{
					PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(objectPtr->ObStrategyBlock->SBdataptr);    
			    	LOCALASSERT(playerStatusPtr);

					playerStatusPtr->IsMovingInWater = 1;
				}
				{
					
					AddRipple(dynPtr->Position.vx,dynPtr->Position.vz,100);
					#if 0
					{
						int i;
						for (i=0; i<10; i++)
						{
							VECTORCH velocity;
							velocity.vy = (-(FastRandom()%(magnitude)))*8;
							velocity.vx = ((FastRandom()&1023)-512)*8;
							velocity.vz = ((FastRandom()&1023)-512)*8;
							MakeParticle(&(dynPtr->Position), &velocity, PARTICLE_WATERSPRAY);
						}
					}
					#endif
				}
			}
		}
	}
}

void DrawMuzzleFlash(VECTORCH *positionPtr,VECTORCH *directionPtr, enum MUZZLE_FLASH_ID muzzleFlashID)
{
	PARTICLE particle;
	particle.Position = *positionPtr;
	
	D3D_DecalSystem_Setup();
	
	switch (muzzleFlashID)
	{
		case MUZZLE_FLASH_SMARTGUN:
		{
			#if 0
			PARTICLE_DESC *particleDescPtr;
			particle.ParticleID=PARTICLE_SMARTGUNMUZZLEFLASH;
			particleDescPtr = &ParticleDescription[particle.ParticleID];

			particle.Colour = RGBALIGHT_MAKE(particleDescPtr->RedScale[CurrentVisionMode],particleDescPtr->GreenScale[CurrentVisionMode],particleDescPtr->BlueScale[CurrentVisionMode],particleDescPtr->Alpha);
			particle.Size = particleDescPtr->Size;

			particle.Position.vx += MUL_FIXED(100,directionPtr->vx);
			particle.Position.vy += MUL_FIXED(100,directionPtr->vy);
			particle.Position.vz += MUL_FIXED(100,directionPtr->vz);
			RenderParticle(&particle);
			#else
			PARTICLE_DESC *particleDescPtr=&ParticleDescription[PARTICLE_MUZZLEFLASH];
			MATRIXCH muzzleMatrix;
			MATRIXCH rotmat;
			MakeMatrixFromDirection(directionPtr,&muzzleMatrix);
			{
		   		int angle = 4096/12;
		 	  	int cos = GetCos(angle);
		 	  	int sin = GetSin(angle);
		 	  	rotmat.mat11 = cos;		 
		 	  	rotmat.mat12 = sin;
		 	  	rotmat.mat13 = 0;
		 	  	rotmat.mat21 = -sin;	  	
		 	  	rotmat.mat22 = cos;	  	
		 	  	rotmat.mat23 = 0;	  	
		 	  	rotmat.mat31 = 0;	  	
		 	  	rotmat.mat32 = 0;	  	
		 	  	rotmat.mat33 = 65536;	  	
			}
			#if 1
			{
				int i = 16;
				PARTICLE particle;

				particle.Position = *positionPtr;
				particle.Position.vx += MUL_FIXED(100 - (FastRandom()&15),directionPtr->vx);
				particle.Position.vy += MUL_FIXED(100 - (FastRandom()&15),directionPtr->vy);
				particle.Position.vz += MUL_FIXED(100 - (FastRandom()&15),directionPtr->vz);

				particle.ParticleID=PARTICLE_MUZZLEFLASH;

				particle.Colour = RGBALIGHT_MAKE(particleDescPtr->RedScale[CurrentVisionMode],particleDescPtr->GreenScale[CurrentVisionMode],particleDescPtr->BlueScale[CurrentVisionMode],particleDescPtr->Alpha);
				particle.Size = 200;

				while(i--)
				{
					RenderParticle(&particle);
					particle.Position.vx += MUL_FIXED(50 - (FastRandom()&15),directionPtr->vx);
					particle.Position.vy += MUL_FIXED(50 - (FastRandom()&15),directionPtr->vy);
					particle.Position.vz += MUL_FIXED(50 - (FastRandom()&15),directionPtr->vz);
					particle.Size -= (FastRandom()&3)+3;
				}

			}
			#endif
			{
				int a;
				for (a=0; a<12;a++)
				{
					int i=8;
					PARTICLE particle;

					particle.Position = *positionPtr;
					particle.Position.vx += -MUL_FIXED(200,directionPtr->vx) + MUL_FIXED(50,muzzleMatrix.mat21);
					particle.Position.vy += -MUL_FIXED(200,directionPtr->vy) + MUL_FIXED(50,muzzleMatrix.mat22);
					particle.Position.vz += -MUL_FIXED(200,directionPtr->vz) + MUL_FIXED(50,muzzleMatrix.mat23);

					particle.ParticleID=PARTICLE_MUZZLEFLASH;

					particle.Colour = RGBALIGHT_MAKE(particleDescPtr->RedScale[CurrentVisionMode],particleDescPtr->GreenScale[CurrentVisionMode],particleDescPtr->BlueScale[CurrentVisionMode],particleDescPtr->Alpha);
					particle.Size = 50;

					if (!(a&1)) i+=8;

					while(i--)
					{
						RenderParticle(&particle);
						particle.Position.vx += MUL_FIXED((FastRandom()&31)+16,muzzleMatrix.mat21);
						particle.Position.vy += MUL_FIXED((FastRandom()&31)+16,muzzleMatrix.mat22);
						particle.Position.vz += MUL_FIXED((FastRandom()&31)+16,muzzleMatrix.mat23);
						particle.Size += (FastRandom()&15);
					}

					MatrixMultiply(&muzzleMatrix,&rotmat,&muzzleMatrix);
				}

			}
			#endif
			break;
		}
		case MUZZLE_FLASH_AMORPHOUS:
		{
			PARTICLE_DESC *particleDescPtr;
			particle.ParticleID=PARTICLE_MUZZLEFLASH;
			particleDescPtr = &ParticleDescription[particle.ParticleID];

			particle.Colour = RGBALIGHT_MAKE(particleDescPtr->RedScale[CurrentVisionMode],particleDescPtr->GreenScale[CurrentVisionMode],particleDescPtr->BlueScale[CurrentVisionMode],particleDescPtr->Alpha);
			particle.Size = particleDescPtr->Size;
		
			RenderParticle(&particle);
			particle.Position.vx += MUL_FIXED(100,directionPtr->vx);
			particle.Position.vy += MUL_FIXED(100,directionPtr->vy);
			particle.Position.vz += MUL_FIXED(100,directionPtr->vz);
			RenderParticle(&particle);
			particle.Position.vx += MUL_FIXED(100,directionPtr->vx);
			particle.Position.vy += MUL_FIXED(100,directionPtr->vy);
			particle.Position.vz += MUL_FIXED(100,directionPtr->vz);
			RenderParticle(&particle);
			{
				int i = 16;
				particle.Size = 20;

				while(i--)
				{
					RenderParticle(&particle);
					particle.Position.vx = positionPtr->vx + MUL_FIXED(100,directionPtr->vx) + (FastRandom()&127)-64;
					particle.Position.vy = positionPtr->vy + MUL_FIXED(100,directionPtr->vy) + (FastRandom()&127)-64;
					particle.Position.vz = positionPtr->vz + MUL_FIXED(100,directionPtr->vz) + (FastRandom()&127)-64;
				}																								 

			}
			break;
		}
		case MUZZLE_FLASH_SKEETER:
		{
			PARTICLE_DESC *particleDescPtr;
			particle.ParticleID=PARTICLE_MUZZLEFLASH;
			particleDescPtr = &ParticleDescription[particle.ParticleID];

			particle.Colour = RGBALIGHT_MAKE(particleDescPtr->RedScale[CurrentVisionMode],particleDescPtr->GreenScale[CurrentVisionMode],particleDescPtr->BlueScale[CurrentVisionMode],particleDescPtr->Alpha);
			particle.Size = particleDescPtr->Size;
		
			RenderParticle(&particle);
			particle.Position.vx += MUL_FIXED(20,directionPtr->vx);
			particle.Position.vy += MUL_FIXED(20,directionPtr->vy);
			particle.Position.vz += MUL_FIXED(20,directionPtr->vz);
			RenderParticle(&particle);
			particle.Position.vx += MUL_FIXED(20,directionPtr->vx);
			particle.Position.vy += MUL_FIXED(20,directionPtr->vy);
			particle.Position.vz += MUL_FIXED(20,directionPtr->vz);
			RenderParticle(&particle);
			{
				int i = 16;
				particle.Size = 20;

				while(i--)
				{
					RenderParticle(&particle);
					particle.Position.vx = positionPtr->vx + MUL_FIXED(100,directionPtr->vx) + (FastRandom()&127)-64;
					particle.Position.vy = positionPtr->vy + MUL_FIXED(100,directionPtr->vy) + (FastRandom()&127)-64;
					particle.Position.vz = positionPtr->vz + MUL_FIXED(100,directionPtr->vz) + (FastRandom()&127)-64;
				}																								 

			}
			break;
		}
		default:
		{
			LOCALASSERT(0);
			return;
		}
	}
	D3D_DecalSystem_End();

}

void DrawFrisbeePlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr)
{
	int i = 16;
	PARTICLE particle;

	particle.Position = *positionPtr;

	particle.ParticleID=PARTICLE_MUZZLEFLASH;

	particle.Colour = RGBALIGHT_MAKE(255,255,255,255);
	particle.Size = 200;

	while(i--)
	{
		RenderParticle(&particle);
		particle.Position.vx -= MUL_FIXED(50,directionPtr->vx);
		particle.Position.vy -= MUL_FIXED(50,directionPtr->vy);
		particle.Position.vz -= MUL_FIXED(50,directionPtr->vz);
		particle.Size -= 10;
	}

}

void DrawPredatorPlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr)
{
	int i = 16;
	PARTICLE particle;

	particle.Position = *positionPtr;

	particle.ParticleID=PARTICLE_MUZZLEFLASH;

	particle.Colour = RGBALIGHT_MAKE(50,255,255,255);
	particle.Size = 200;

	while(i--)
	{
		RenderParticle(&particle);
		particle.Position.vx -= MUL_FIXED(50,directionPtr->vx);
		particle.Position.vy -= MUL_FIXED(50,directionPtr->vy);
		particle.Position.vz -= MUL_FIXED(50,directionPtr->vz);
		particle.Size -= 10;
	}

}

void DrawSmallPredatorPlasmaBolt(VECTORCH *positionPtr,VECTORCH *directionPtr)
{
	#if 0
	int i = 16;
	PARTICLE particle;

	particle.Position = *positionPtr;

	particle.ParticleID=PARTICLE_MUZZLEFLASH;

	particle.Colour = RGBALIGHT_MAKE(50,255,255,255);
	particle.Size = 100;

	while(i--)
	{
		RenderParticle(&particle);
		particle.Position.vx -= MUL_FIXED(25,directionPtr->vx);
		particle.Position.vy -= MUL_FIXED(25,directionPtr->vy);
		particle.Position.vz -= MUL_FIXED(25,directionPtr->vz);
		particle.Size -= 5;
	}
	#else
	PARTICLE particle;

	particle.Position = *positionPtr;

	particle.ParticleID=PARTICLE_ELECTRICALPLASMASPHERE;

	particle.Colour = RGBALIGHT_MAKE(255,255,255,128);
	particle.Size = 200;

	RenderParticle(&particle);
	particle.ParticleID=PARTICLE_MUZZLEFLASH;
	particle.Colour = RGBALIGHT_MAKE(255,255,255,64);
	particle.Size = 1000;
	RenderParticle(&particle);
	RenderParticle(&particle);

	#endif

}



void MakeFlareParticle(DYNAMICSBLOCK *dynPtr)
{
//	int i = MUL_FIXED(4,density);
//	while(i--)
	{
		VECTORCH velocity;
		velocity.vx = ((FastRandom()&2047) - 1024);
		velocity.vy = ((FastRandom()&2047) - 1024);
		velocity.vz = (1000+(FastRandom()&255))*2;
		RotateVector(&velocity,&(dynPtr->OrientMat));
		MakeParticle(&(dynPtr->Position),&(velocity),PARTICLE_FLARESMOKE);
	}
}
void MakeRocketTrailParticles(VECTORCH *prevPositionPtr, VECTORCH *positionPtr)
{
	VECTORCH disp;
	
	disp.vx = positionPtr->vx - prevPositionPtr->vx;
	disp.vy = positionPtr->vy - prevPositionPtr->vy;
	disp.vz = positionPtr->vz - prevPositionPtr->vz;

	if (disp.vx!=0 || disp.vy!=0 || disp.vz!=0)
	{
		int i=16;
		do
		{
			{
				VECTORCH position;
				VECTORCH velocity;
				velocity.vx = (FastRandom()&1023) - 512;
				velocity.vy = (FastRandom()&1023) - 512;
				velocity.vz = (FastRandom()&1023) - 512;

				position.vx = prevPositionPtr->vx + (disp.vx*i)/16;
				position.vy = prevPositionPtr->vy + (disp.vy*i)/16;
				position.vz = prevPositionPtr->vz + (disp.vz*i)/16;

				MakeParticle(&position,&(velocity),PARTICLE_BLACKSMOKE);
			}
		}
		while(i--);
	}
}
void MakeGrenadeTrailParticles(VECTORCH *prevPositionPtr, VECTORCH *positionPtr)
{
	VECTORCH disp;
	
	disp.vx = positionPtr->vx - prevPositionPtr->vx;
	disp.vy = positionPtr->vy - prevPositionPtr->vy;
	disp.vz = positionPtr->vz - prevPositionPtr->vz;

	if (!(disp.vx==0 && disp.vy==0 && disp.vz==0))
	{
		int i=8;
		do
		{
			{
				VECTORCH position;
				VECTORCH velocity;
				velocity.vx = (FastRandom()&1023) - 512;
				velocity.vy = (FastRandom()&1023) - 512;
				velocity.vz = (FastRandom()&1023) - 512;

				position.vx = prevPositionPtr->vx + (disp.vx*i)/16;
				position.vy = prevPositionPtr->vy + (disp.vy*i)/16;
				position.vz = prevPositionPtr->vz + (disp.vz*i)/16;

				MakeParticle(&position,&(velocity),PARTICLE_FLARESMOKE);
			}
		}
		while(i--);
	}
}

void MakeImpactSmoke(MATRIXCH *orientationPtr, VECTORCH *positionPtr)
{
	int i = 5;
	while(i--)
	{
		VECTORCH velocity;
		velocity.vx = ((FastRandom()&1023) - 512);
		velocity.vy = ((FastRandom()&1023) - 512);
		velocity.vz = (255+(FastRandom()&255));
		RotateVector(&velocity,orientationPtr);
		MakeParticle(positionPtr,&(velocity),PARTICLE_IMPACTSMOKE);
	}
}

void MakeImpactSparks(VECTORCH *incidentPtr, VECTORCH *normalPtr, VECTORCH *positionPtr)
{
	int noOfSparks = 5;
	VECTORCH velocity;
	int d = -2*DotProduct(incidentPtr,normalPtr);
	velocity.vx = (incidentPtr->vx + MUL_FIXED(d,normalPtr->vx))>>3;
	velocity.vy = (incidentPtr->vy + MUL_FIXED(d,normalPtr->vy))>>3;
	velocity.vz = (incidentPtr->vz + MUL_FIXED(d,normalPtr->vz))>>3;

	do
	{
		
//		VECTORCH velocity;
		velocity.vx = velocity.vx + (FastRandom()&2047)-1024;
		velocity.vy = velocity.vy + (FastRandom()&2047)-1024;
		velocity.vz = velocity.vz + (FastRandom()&2047)-1024;
		MakeParticle(positionPtr,&velocity,PARTICLE_RICOCHET_SPARK);	
	}
	while(--noOfSparks);
	MakeRicochetSound(positionPtr);
}

void MakeSprayOfSparks(MATRIXCH *orientationPtr, VECTORCH *positionPtr)
{
	int noOfSparks = 15;
	do
	{
		
		VECTORCH velocity;
		velocity.vx = (FastRandom()&2047)-1024;
		velocity.vy = (FastRandom()&2047);
		velocity.vz = -(FastRandom()&2047)-1024;
		RotateVector(&velocity,orientationPtr);
		MakeParticle(positionPtr,&velocity,PARTICLE_SPARK);	
	}
	while(--noOfSparks);

	MakeLightElement(positionPtr,LIGHTELEMENT_ELECTRICAL_SPARKS);
}

void MakeVolumetricExplosionAt(VECTORCH *positionPtr, enum EXPLOSION_ID explosionID)
{
	switch (explosionID)
	{
		case EXPLOSION_HUGE:
		case EXPLOSION_HUGE_NOCOLLISIONS:
		{
			VOLUMETRIC_EXPLOSION *expPtr;
			int i;
			int r;
			
			/* KJL 11:49:25 19/08/98 - check to see if explosion is inside environment */
			{
				MODULE *module = ModuleFromPosition(positionPtr, (MODULE*)NULL);
				if (!module) return;
			}

			expPtr = AllocateVolumetricExplosion();
			r = (FastRandom()&7)+1;
			
			if (explosionID == EXPLOSION_HUGE_NOCOLLISIONS)
			{
				expPtr->UseCollisions = 0;
			}
			else
			{
				expPtr->UseCollisions = 1;
			}

			for(i=0; i<SPHERE_VERTICES; i++)
			{
				expPtr->Position[i] = *positionPtr;
				expPtr->Velocity[i].vx = SphereVertex[i].vx;
				expPtr->Velocity[i].vy = SphereVertex[i].vy;
				expPtr->Velocity[i].vz = SphereVertex[i].vz;
				expPtr->BeenStopped[i] = 0;
				expPtr->RipplePhase[i] = FastRandom()&4095;
				expPtr->NumberVerticesMoving=SPHERE_VERTICES;
			}
			expPtr->LifeTime = ONE_FIXED-1;
			expPtr->ExplosionPhase = 1;
			
			for(i=0; i<r; i++)
			{
				CreateFlamingDebris(positionPtr,&SphereVertex[FastRandom()%SPHERE_VERTICES]);
			}
			if ((positionPtr->vx!=Player->ObWorld.vx)
		      ||(positionPtr->vy!=Player->ObWorld.vy)
			  ||(positionPtr->vz!=Player->ObWorld.vz))
			{
				MakeLightElement(positionPtr, LIGHTELEMENT_EXPLOSION);
			}

			#if 1
			for (i=0; i<LocalDetailLevels.NumberOfSmokeParticlesFromLargeExplosion; i++)
			{
				VECTORCH position = *positionPtr;
				position.vx += (FastRandom()&2047)-1024;
				position.vy += (FastRandom()&2047)-1024;
				position.vz += (FastRandom()&2047)-1024;
				MakeParticle(&position, positionPtr, PARTICLE_SMOKECLOUD);
			}
			#endif
			break;
		}
		case EXPLOSION_MOLOTOV:
		{
			MakeMolotovExplosionAt(positionPtr,0);
			break;
		}
		case EXPLOSION_PULSEGRENADE:
		case EXPLOSION_SMALL_NOCOLLISIONS:
		{
			VOLUMETRIC_EXPLOSION *expPtr;
			int i;
			int r;
			
			/* KJL 11:49:25 19/08/98 - check to see if explosion is inside environment */
			{
				MODULE *module = ModuleFromPosition(positionPtr, (MODULE*)NULL);
				if (!module) return;
			}
			#if 1
			expPtr = AllocateVolumetricExplosion();

			if (explosionID == EXPLOSION_SMALL_NOCOLLISIONS)
			{
				expPtr->UseCollisions = 0;
			}
			else
			{
				expPtr->UseCollisions = 1;
			}
			r = (FastRandom()&7)+1;
			
			for(i=0; i<SPHERE_VERTICES; i++)
			{
				expPtr->Position[i] = *positionPtr;
				expPtr->Velocity[i].vx = SphereVertex[i].vx;
				expPtr->Velocity[i].vy = SphereVertex[i].vy;
				expPtr->Velocity[i].vz = SphereVertex[i].vz;
				expPtr->BeenStopped[i] = 0;
				expPtr->RipplePhase[i] = FastRandom()&4095;
				expPtr->NumberVerticesMoving=SPHERE_VERTICES;
			}
			expPtr->LifeTime = ONE_FIXED/2;
			expPtr->ExplosionPhase = 1;
			#endif
			#if 0
			for(i=0; i<r; i++)
			{
				CreateFlamingDebris(positionPtr,&SphereVertex[FastRandom()%SPHERE_VERTICES]);
			}
			#endif
			if ((positionPtr->vx!=Player->ObWorld.vx)
		      ||(positionPtr->vy!=Player->ObWorld.vy)
			  ||(positionPtr->vz!=Player->ObWorld.vz))
			{
				MakeLightElement(positionPtr, LIGHTELEMENT_EXPLOSION);
//				MakeLightElement(positionPtr, LIGHTELEMENT_ELECTRICAL_EXPLOSION);
			}

			#if 1
			for (i=0; i<LocalDetailLevels.NumberOfSmokeParticlesFromSmallExplosion; i++)
			{
				VECTORCH position = *positionPtr;
				position.vx += (FastRandom()&1023)-512;
				position.vy += (FastRandom()&1023)-512;
				position.vz += (FastRandom()&1023)-512;
				MakeParticle(&position, positionPtr, PARTICLE_SMOKECLOUD);
			}
			#endif
			break;
		}
		case EXPLOSION_PREDATORPISTOL:
		{																			    
			MakeElectricalExplosion(positionPtr);
			break;
		}
		default:
			break;
	}
}


void MakeFlechetteExplosionAt(VECTORCH *positionPtr,int seed)
{
	extern SOUND3DDATA Explosion_SoundData;
	int i;
	enum PARTICLE_ID particle_to_use;

    Explosion_SoundData.position=*positionPtr;
    Sound_Play(SID_FRAG_RICOCHETS,"n",&Explosion_SoundData);
    
    if(!seed)  
	{
		//need to get a random number seed for this explosion
		while(!seed) seed=FastRandom();

		//explosion originated on this computer , so use damaging type
		particle_to_use=PARTICLE_FLECHETTE;

		//if in a network game , send explosion to other players
		if(AvP.Network!=I_No_Network)
		{
			AddNetMsg_MakeFlechetteExplosion(positionPtr,seed);
		}
	}
	else
	{
		//explosion passed across the network
		//use the nondamaging flechette
		particle_to_use=PARTICLE_FLECHETTE_NONDAMAGING;
	}
	SetSeededFastRandom(seed);
	
	/* KJL 11:49:25 19/08/98 - check to see if explosion is inside environment */
	{
		MODULE *module = ModuleFromPosition(positionPtr, (MODULE*)NULL);
		if (!module) return;
	}
	#if 0
	for(i=0; i<SPHERE_VERTICES; i++)
	{
		if (SphereVertex[i].vy<0) MakeParticle(positionPtr,&SphereVertex[i],PARTICLE_FLECHETTE);
	}
	#else
	for (i=0; i<100; i++)
	{
		VECTORCH velocity;
		int phi = SeededFastRandom()&4095;

		velocity.vy = -(SeededFastRandom()&65535);
		{
			float y = ((float)velocity.vy)/65536.0;
			y = sqrt(1-y*y);

			f2i(velocity.vx,(float)GetCos(phi)*y);
			f2i(velocity.vz,(float)GetSin(phi)*y);
		}

		MakeParticle(positionPtr, &velocity, particle_to_use);
	}
	#endif
	MakeLightElement(positionPtr, LIGHTELEMENT_EXPLOSION);
}
void MakeMolotovExplosionAt(VECTORCH *positionPtr,int seed)
{
	int i;
	enum PARTICLE_ID particle_to_use;

    if(!seed)  
	{
		//need to get a random number seed for this explosion
		while(!seed) seed=FastRandom();

		//explosion originated on this computer , so use damaging type
		particle_to_use=PARTICLE_MOLOTOVFLAME;

		//if in a network game , send explosion to other players
		if(AvP.Network!=I_No_Network)
		{
  //			AddNetMsg_MakeFlechetteExplosion(positionPtr,seed);
		}
	}
	else
	{
		//explosion passed across the network
		//use the nondamaging flechette
 //		particle_to_use=PARTICLE_FLECHETTE_NONDAMAGING;
	}
	SetSeededFastRandom(seed);
	
	/* KJL 11:49:25 19/08/98 - check to see if explosion is inside environment */
	{
		MODULE *module = ModuleFromPosition(positionPtr, (MODULE*)NULL);
		if (!module) return;
	}
	for (i=0; i<100; i++)
	{
		VECTORCH velocity;
		int phi = SeededFastRandom()&4095;

		velocity.vy = -(SeededFastRandom()&65535);
		{
			float y = ((float)velocity.vy)/65536.0;
			y = sqrt(1-y*y);

			f2i(velocity.vx,(float)GetCos(phi)*y);
			f2i(velocity.vz,(float)GetSin(phi)*y);
		}
		velocity.vx /= 8;
		velocity.vy /= 16;
		velocity.vz /= 8;

		MakeParticle(positionPtr, &velocity, particle_to_use);
	}
   //	MakeLightElement(positionPtr, LIGHTELEMENT_EXPLOSION);
}


static void HandleVolumetricExplosion(VOLUMETRIC_EXPLOSION *expPtr)
{
	int i;
	PARTICLE particle;
	int velocityModifier;
	particle.ParticleID = PARTICLE_EXPLOSIONFIRE;

	if (!expPtr->LifeTime) return;
	RenderExplosionSurface(expPtr);
	
	if (expPtr->ExplosionPhase)
	{
		{
			int v = (DIV_FIXED(SPHERE_VERTICES,expPtr->NumberVerticesMoving+1)+ONE_FIXED);
			velocityModifier = MUL_FIXED(GetSin(expPtr->LifeTime/64)/16,v);
		}
		
		for(i=0; i<SPHERE_VERTICES; i++)
		{
			int v;
			VECTORCH obstacleNormal = {0,0,0};
			int moduleIndex;
			if ((expPtr->Velocity[i].vx==0)
			   &&(expPtr->Velocity[i].vy==0)
			   &&(expPtr->Velocity[i].vz==0))
			 continue;
			particle.Velocity = expPtr->Velocity[i];
			#if 1
			{
				v = GetSin((CloakingPhase*4+expPtr->RipplePhase[i])&4095)/4;
				v = velocityModifier+MUL_FIXED(v,velocityModifier);
			}
			#endif
			particle.Velocity.vx = MUL_FIXED(particle.Velocity.vx,v);
			particle.Velocity.vy = MUL_FIXED(particle.Velocity.vy,v);
			particle.Velocity.vz = MUL_FIXED(particle.Velocity.vz,v);
		

			particle.Position = expPtr->Position[i];
		
			if(LocalDetailLevels.ExplosionsDeformToEnvironment && expPtr->UseCollisions)
			{
				if(ParticleDynamics(&particle,&obstacleNormal,&moduleIndex))
				{
					int magOfPerpImp = DotProduct(&obstacleNormal,&(expPtr->Velocity[i]));
					expPtr->Velocity[i].vx -= MUL_FIXED(obstacleNormal.vx, magOfPerpImp);
					expPtr->Velocity[i].vy -= MUL_FIXED(obstacleNormal.vy, magOfPerpImp);
					expPtr->Velocity[i].vz -= MUL_FIXED(obstacleNormal.vz, magOfPerpImp);

					if(!expPtr->BeenStopped[i])
					{
						expPtr->BeenStopped[i] = 1;
						expPtr->NumberVerticesMoving--;
					}
				}
			}
			else
			{
				particle.Position.vx += MUL_FIXED(particle.Velocity.vx,NormalFrameTime);
				particle.Position.vy += MUL_FIXED(particle.Velocity.vy,NormalFrameTime);
				particle.Position.vz += MUL_FIXED(particle.Velocity.vz,NormalFrameTime);
			}
			expPtr->Position[i] = particle.Position;
		}


		expPtr->LifeTime -= NormalFrameTime;

		if (expPtr->LifeTime<=0)
		{
			expPtr->LifeTime=0;
		}
	}
}

void MakeOldVolumetricExplosionAt(VECTORCH *positionPtr)
{
	int noRequired = 32;//MUL_FIXED(2500,NormalFrameTime);
	int i;
//	VECTORCH zero={0,0,0};
	for (i=0; i<noRequired; i++)
	{
		VECTORCH velocity;
		int phi = FastRandom()&4095;
		int speed = 6000*4+(FastRandom()&4095);

		velocity.vz = (FastRandom()&131071) - ONE_FIXED;
		{
			float z = ((float)velocity.vz)/65536.0;
			z = sqrt(1-z*z);

			f2i(velocity.vx,(float)GetCos(phi)*z);
			f2i(velocity.vy,(float)GetSin(phi)*z);
		}
		
		velocity.vx = MUL_FIXED(velocity.vx,speed);
		velocity.vy = MUL_FIXED(velocity.vy,speed);
		velocity.vz = MUL_FIXED(velocity.vz,speed);

		MakeParticle(positionPtr, &velocity, PARTICLE_EXPLOSIONFIRE);
	}

	MakeLightElement(positionPtr, LIGHTELEMENT_EXPLOSION);
}

extern void MakePlasmaExplosion(VECTORCH *positionPtr, VECTORCH *fromPositionPtr, enum EXPLOSION_ID explosionID)
{
	switch (explosionID)
	{
		case EXPLOSION_DISSIPATINGPLASMA:
		{
			MakeParticle(positionPtr,positionPtr,PARTICLE_BLUEPLASMASPHERE);
			MakeLightElement(positionPtr,LIGHTELEMENT_PLASMACASTERHIT);
			MakeBloodExplosion(fromPositionPtr, 127, positionPtr, 200, PARTICLE_ORANGE_SPARK);
			Sound_Play(SID_PLASMABOLT_DISSIPATE,"d",positionPtr);
			break;
		}
		case EXPLOSION_FOCUSEDPLASMA:
		{
			MakeParticle(positionPtr,positionPtr,PARTICLE_BLUEPLASMASPHERE);
			MakeLightElement(positionPtr,LIGHTELEMENT_PLASMACASTERHIT);
			MakeFocusedExplosion(fromPositionPtr, positionPtr, 100, PARTICLE_ORANGE_PLASMA);
			Sound_Play(SID_PLASMABOLT_HIT,"d",positionPtr);
			break;
		}
		default:
			break;
	}
}
extern void MakeElectricalExplosion(VECTORCH *positionPtr)
{
	{
		PredPistolExplosion_SoundData.position=*positionPtr;
		Sound_Play(SID_WIL_PRED_PISTOL_EXPLOSION,"n",&PredPistolExplosion_SoundData);
   	}
	//Sound_Play(SID_WIL_PRED_PISTOL_EXPLOSION,"d",positionPtr);
	MakeParticle(positionPtr,positionPtr,PARTICLE_ELECTRICALPLASMASPHERE);
	MakeLightElement(positionPtr, LIGHTELEMENT_ELECTRICAL_EXPLOSION);
}

void MakeBloodExplosion(VECTORCH *originPtr, int creationRadius, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID)
{
	VECTORCH blastDir;
	int i;
	
	/* Dividing by zero can be bad for your health */
	LOCALASSERT(creationRadius!=0);

	blastDir.vx = originPtr->vx - blastPositionPtr->vx;
	blastDir.vy = originPtr->vy - blastPositionPtr->vy;
	blastDir.vz = originPtr->vz - blastPositionPtr->vz;

	for (i=0; i<noOfParticles; i++)
	{
		VECTORCH velocity;
		VECTORCH position;
		int phi = FastRandom()&4095;
		int speed = 6000+(FastRandom()&4095);
		int r;
		//speed = FastRandom()&2047;

		velocity.vz = (FastRandom()&131071) - ONE_FIXED;
		{
			float z = ((float)velocity.vz)/65536.0;
			z = sqrt(1-z*z);

			f2i(velocity.vx,(float)GetCos(phi)*z);
			f2i(velocity.vy,(float)GetSin(phi)*z);
		}
		
		if (DotProduct(&velocity,&blastDir)<0)
		{
			velocity.vx = -velocity.vx; 
			velocity.vy = -velocity.vy;
			velocity.vz = -velocity.vz; 
		}

		r = FastRandom()%creationRadius;
		position.vx = originPtr->vx + MUL_FIXED(velocity.vx,r);
		position.vy = originPtr->vy + MUL_FIXED(velocity.vy,r);
		position.vz = originPtr->vz + MUL_FIXED(velocity.vz,r);
		
		velocity.vx = MUL_FIXED(velocity.vx,speed);
		velocity.vy = MUL_FIXED(velocity.vy,speed);
		velocity.vz = MUL_FIXED(velocity.vz,speed);


		MakeParticle(&position, &velocity, particleID);
	}
}

	
void MakeFocusedExplosion(VECTORCH *originPtr, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID)
{
	VECTORCH blastDir;
	int i;
	
	blastDir.vx = originPtr->vx - blastPositionPtr->vx;
	blastDir.vy = originPtr->vy - blastPositionPtr->vy;
	blastDir.vz = originPtr->vz - blastPositionPtr->vz;

	for (i=0; i<noOfParticles; i++)
	{
		VECTORCH velocity,position;
		int phi = FastRandom()&4095;
		int speed = 6000+(FastRandom()&4095);
		int r;
		speed = FastRandom()&2047;

		velocity.vz = (FastRandom()&131071) - ONE_FIXED;
		{
			float z = ((float)velocity.vz)/65536.0;
			z = sqrt(1-z*z);

			f2i(velocity.vx,(float)GetCos(phi)*z);
			f2i(velocity.vy,(float)GetSin(phi)*z);
		}
		
		if (DotProduct(&velocity,&blastDir)<0)
		{
			velocity.vx = -velocity.vx; 
			velocity.vy = -velocity.vy;
			velocity.vz = -velocity.vz; 
		}
		r = FastRandom()%127;
		position.vx = originPtr->vx + MUL_FIXED(velocity.vx,r);
		position.vy = originPtr->vy + MUL_FIXED(velocity.vy,r);
		position.vz = originPtr->vz + MUL_FIXED(velocity.vz,r);
		
		velocity.vx = MUL_FIXED(velocity.vx,speed);
		velocity.vy = MUL_FIXED(velocity.vy,speed);
		velocity.vz = MUL_FIXED(velocity.vz,speed);


		MakeParticle(&position, &velocity, particleID);
	}
}




void MakePlasmaTrailParticles(DYNAMICSBLOCK *dynPtr, int number)
{
	
	VECTORCH disp;
	
	disp.vx = dynPtr->Position.vx - dynPtr->PrevPosition.vx;
	disp.vy = dynPtr->Position.vy - dynPtr->PrevPosition.vy;
	disp.vz = dynPtr->Position.vz - dynPtr->PrevPosition.vz;

  //	if (disp.vx!=0 || disp.vy!=0 || disp.vz!=0)
	{
		int i=number;
		do
		{
			VECTORCH velocity;
			VECTORCH position;
			#if 1

			int phi = FastRandom()&4095;
			int speed = 512;

			velocity.vz = (FastRandom()&131071) - ONE_FIXED;
			{
				float z = ((float)velocity.vz)/65536.0;
				z = sqrt(1-z*z);

				f2i(velocity.vx,(float)GetCos(phi)*z);
				f2i(velocity.vy,(float)GetSin(phi)*z);
			}
			
			velocity.vx = MUL_FIXED(velocity.vx,speed);//+dynPtr->LinVelocity.vx/32;
			velocity.vy = MUL_FIXED(velocity.vy,speed);//+dynPtr->LinVelocity.vy/32;
			velocity.vz = MUL_FIXED(velocity.vz,speed);//+dynPtr->LinVelocity.vz/32;
			#else
			velocity.vx = dynPtr->LinVelocity.vx;
			velocity.vy = dynPtr->LinVelocity.vy;
			velocity.vz = dynPtr->LinVelocity.vz;
			#endif
			position.vx = dynPtr->PrevPosition.vx + (disp.vx*i)/number;
			position.vy = dynPtr->PrevPosition.vy + (disp.vy*i)/number;
			position.vz = dynPtr->PrevPosition.vz + (disp.vz*i)/number;

			MakeParticle(&position, &velocity, PARTICLE_PLASMATRAIL);
		}
		while(i--);
	}
}

void MakeDewlineTrailParticles(DYNAMICSBLOCK *dynPtr, int number)
{
	
	VECTORCH disp;
	
	disp.vx = dynPtr->Position.vx - dynPtr->PrevPosition.vx;
	disp.vy = dynPtr->Position.vy - dynPtr->PrevPosition.vy;
	disp.vz = dynPtr->Position.vz - dynPtr->PrevPosition.vz;

  //	if (disp.vx!=0 || disp.vy!=0 || disp.vz!=0)
	{
		int i=number;
		do
		{
			VECTORCH velocity;
			VECTORCH position;
			#if 1

			int phi = FastRandom()&4095;
			int speed = 256; //512

			velocity.vz = (FastRandom()&131071) - ONE_FIXED;
			{
				float z = ((float)velocity.vz)/65536.0;
				z = sqrt(1-z*z);

				f2i(velocity.vx,(float)GetCos(phi)*z);
				f2i(velocity.vy,(float)GetSin(phi)*z);
			}
			
			velocity.vx = MUL_FIXED(velocity.vx,speed);//+dynPtr->LinVelocity.vx/32;
			velocity.vy = MUL_FIXED(velocity.vy,speed);//+dynPtr->LinVelocity.vy/32;
			velocity.vz = MUL_FIXED(velocity.vz,speed);//+dynPtr->LinVelocity.vz/32;
			#else
			velocity.vx = dynPtr->LinVelocity.vx;
			velocity.vy = dynPtr->LinVelocity.vy;
			velocity.vz = dynPtr->LinVelocity.vz;
			#endif
			position.vx = dynPtr->PrevPosition.vx + (disp.vx*i)/number;
			position.vy = dynPtr->PrevPosition.vy + (disp.vy*i)/number;
			position.vz = dynPtr->PrevPosition.vz + (disp.vz*i)/number;

			MakeParticle(&position, &velocity, PARTICLE_DEWLINE);
		}
		while(i--);
	}
}

void DrawXenoborgMainLaserbeam(LASER_BEAM_DESC *laserPtr)
{
	if (laserPtr->BeamHasHitPlayer)
	{			  
		int colour;
		switch (CurrentVisionMode)
		{
			default:
			case VISION_MODE_NORMAL:
			{
				colour = 0xff0000ff;
				break;
			}
			case VISION_MODE_IMAGEINTENSIFIER:
			{
				colour = 0xffffffff;
				break;
			}
			case VISION_MODE_PRED_THERMAL:
			case VISION_MODE_PRED_SEEALIENS:
			case VISION_MODE_PRED_SEEPREDTECH:
			{
				colour = 0xffff8000; 
			  	break;
			}
		}
		RenderLightFlare(&(laserPtr->SourcePosition),colour);
		RenderLightFlare(&(laserPtr->SourcePosition),0xffffffff);
	}
	else
	{
		PARTICLE particle;
		switch (CurrentVisionMode)
		{
			default:
			case VISION_MODE_NORMAL:
			{
				particle.Colour = RGBALIGHT_MAKE(0,0,255,255);
				break;
			}
			case VISION_MODE_IMAGEINTENSIFIER:
			{
				particle.Colour = RGBALIGHT_MAKE(255,255,255,255);
				break;
			}
			case VISION_MODE_PRED_THERMAL:
			case VISION_MODE_PRED_SEEALIENS:
			case VISION_MODE_PRED_SEEPREDTECH:
			{
				particle.Colour = RGBALIGHT_MAKE(255,128,0,255);
			  	break;
			}
		}
	 	particle.Colour = RGBALIGHT_MAKE(255,255,255,255);
		particle.ParticleID = PARTICLE_PLASMABEAM;
		particle.Position = laserPtr->SourcePosition;
		particle.Offset = laserPtr->TargetPosition;
		particle.Size = 32;
		RenderParticle(&particle);
	 //	particle.Colour = RGBALIGHT_MAKE(255,255,255,255);
	 //	particle.Size = 20;
	 //	RenderParticle(&particle);

	}
}
/* KJL 16:37:03 20/06/98 - stupid but quick implementation */
#define NO_OF_VERTICES_IN_TRAIL 300
PHEROMONE_TRAIL Trail[NO_OF_VERTICES_IN_TRAIL];

void NewTrailPoint(DYNAMICSBLOCK *dynPtr)
{
 	PHEROMONE_TRAIL *trailPtr = AllocatePheromoneTrail();
	if (!trailPtr) return;

 	GLOBALASSERT(dynPtr);
	trailPtr->Vertex[0] = dynPtr->Position;
   //	trailPtr->Vertex[0].vy -=1000;
	
	trailPtr->Perp[0] = *((VECTORCH*)&(dynPtr->OrientMat.mat11));
	trailPtr->Size[0] = 127*65536;

	trailPtr->Vertex[1] = dynPtr->PrevPosition;
   //	trailPtr->Vertex[1].vy -=1000;
	trailPtr->Perp[1] = *((VECTORCH*)&(dynPtr->PrevOrientMat.mat11));
	trailPtr->Size[1] = 127*65536-MUL_FIXED(TRAIL_DECAY_SPEED,PrevNormalFrameTime);
}

VECTORCH PlayerPheromoneTrailPerp = {0,0,0};
void PlayerPheromoneTrail(DYNAMICSBLOCK *dynPtr)
{
 	PHEROMONE_TRAIL *trailPtr = AllocatePheromoneTrail();
	VECTORCH disp;
	if (!trailPtr) return;

	disp.vx = dynPtr->Position.vx-dynPtr->PrevPosition.vx;
	disp.vy = dynPtr->Position.vy-dynPtr->PrevPosition.vy;
	disp.vz = dynPtr->Position.vz-dynPtr->PrevPosition.vz;

	trailPtr->Vertex[0] = dynPtr->Position;
	#if 1
	trailPtr->Vertex[0].vx -= dynPtr->OrientMat.mat21>>6;
	trailPtr->Vertex[0].vy -= dynPtr->OrientMat.mat22>>6;
	trailPtr->Vertex[0].vz -= dynPtr->OrientMat.mat23>>6;
	#endif
 //	trailPtr->Vertex[0].vy -= 500;
	
	trailPtr->Perp[0] = *((VECTORCH*)&(dynPtr->OrientMat.mat11));
#if 0
	trailPtr->Perp[0].vx = disp.vz - disp.vy;
	trailPtr->Perp[0].vy = disp.vx - disp.vz;
	trailPtr->Perp[0].vz = disp.vy - disp.vx;
	if (trailPtr->Perp[0].vx!=0 || trailPtr->Perp[0].vy !=0 || trailPtr->Perp[0].vz!=0)
	{
		Normalise(&trailPtr->Perp[0]);
	}
	else
	{
		trailPtr->Perp[0] = PlayerPheromoneTrailPerp;
	}
#endif
	trailPtr->Size[0] = 127*65536*4;

	trailPtr->Vertex[1] = dynPtr->PrevPosition;
	#if 1
	trailPtr->Vertex[1].vx -= dynPtr->PrevOrientMat.mat21>>6;
	trailPtr->Vertex[1].vy -= dynPtr->PrevOrientMat.mat22>>6;
	trailPtr->Vertex[1].vz -= dynPtr->PrevOrientMat.mat23>>6;
	#endif
 //	trailPtr->Vertex[1].vy -= 500;


  trailPtr->Perp[1] = *((VECTORCH*)&(dynPtr->PrevOrientMat.mat11));
//	trailPtr->Perp[1] = PlayerPheromoneTrailPerp;
//	PlayerPheromoneTrailPerp = trailPtr->Perp[0];

	trailPtr->Size[1] = 127*65536*4-MUL_FIXED(TRAIL_DECAY_SPEED,PrevNormalFrameTime);
}

void HandlePheromoneTrails(void)
{	
	int i;
	PHEROMONE_TRAIL *trailPtr;
	int decayDelta;

	i = NumActiveTrails;
	trailPtr = TrailStorage;
	//textprint("Pheromone Segments active:%d\n",NumActiveTrails);

	decayDelta = MUL_FIXED(TRAIL_DECAY_SPEED,NormalFrameTime);

#if 0
	if (TICKERTAPE_CHEATMODE)
	{
		decayDelta/=4;
	}
#endif
	while(i--)
	{

		trailPtr->Size[0] -= decayDelta;
		trailPtr->Size[1] -= decayDelta;
		if (trailPtr->Size[1]<0) trailPtr->Size[1]=0;
		if (trailPtr->Size[0]<0)
		{
			DeallocatePheromoneTrail(trailPtr);
		}
		else
		{
			RenderTrailSegment(trailPtr);	
			trailPtr++;
		}

	}					 
}
#include "frustum.h"
void RenderTrailSegment(PHEROMONE_TRAIL *trailPtr)
{
 	POLYHEADER fakeHeader;
	VECTORCH temp;
	int Uoffset = GetCos(CloakingPhase&4095)*128;
	int Voffset	= GetSin(CloakingPhase&4095)*128;

	temp.vx = trailPtr->Vertex[0].vx - MUL_FIXED(trailPtr->Perp[0].vx,trailPtr->Size[0]/65536);
	temp.vy = trailPtr->Vertex[0].vy - MUL_FIXED(trailPtr->Perp[0].vy,trailPtr->Size[0]/65536);
	temp.vz	= trailPtr->Vertex[0].vz - MUL_FIXED(trailPtr->Perp[0].vz,trailPtr->Size[0]/65536);
	VerticesBuffer[0].U = (temp.vx+temp.vz)*8192+Uoffset;
	VerticesBuffer[0].V = (temp.vy+temp.vz)*8192+Voffset;
	TranslatePointIntoViewspace(&temp);
	VerticesBuffer[0].X = temp.vx;
	VerticesBuffer[0].Y = temp.vy;
	VerticesBuffer[0].Z	= temp.vz;
//	VerticesBuffer[0].U = ParticleDescription[PARTICLE_PLASMABEAM].StartU/2;
//	VerticesBuffer[0].V = ParticleDescription[PARTICLE_PLASMABEAM].StartV/2;
	
	temp.vx = trailPtr->Vertex[0].vx + MUL_FIXED(trailPtr->Perp[0].vx,trailPtr->Size[0]/65536);
	temp.vy = trailPtr->Vertex[0].vy + MUL_FIXED(trailPtr->Perp[0].vy,trailPtr->Size[0]/65536);
	temp.vz	= trailPtr->Vertex[0].vz + MUL_FIXED(trailPtr->Perp[0].vz,trailPtr->Size[0]/65536);
	VerticesBuffer[1].U = (temp.vx+temp.vz)*8192+Uoffset;
	VerticesBuffer[1].V = (temp.vy+temp.vz)*8192+Voffset;
	TranslatePointIntoViewspace(&temp);
	VerticesBuffer[1].X = temp.vx;
	VerticesBuffer[1].Y = temp.vy;
	VerticesBuffer[1].Z	= temp.vz;
//	VerticesBuffer[1].U = ParticleDescription[PARTICLE_PLASMABEAM].StartU/2;
//	VerticesBuffer[1].V = ParticleDescription[PARTICLE_PLASMABEAM].EndV/2;


	temp.vx = trailPtr->Vertex[1].vx + MUL_FIXED(trailPtr->Perp[1].vx,trailPtr->Size[1]/65536);
	temp.vy = trailPtr->Vertex[1].vy + MUL_FIXED(trailPtr->Perp[1].vy,trailPtr->Size[1]/65536);
	temp.vz	= trailPtr->Vertex[1].vz + MUL_FIXED(trailPtr->Perp[1].vz,trailPtr->Size[1]/65536);
	VerticesBuffer[2].U = (temp.vx+temp.vz)*8192+Uoffset;
	VerticesBuffer[2].V = (temp.vy+temp.vz)*8192+Voffset;
	TranslatePointIntoViewspace(&temp);
	VerticesBuffer[2].X = temp.vx;
	VerticesBuffer[2].Y = temp.vy;
	VerticesBuffer[2].Z	= temp.vz;
//	VerticesBuffer[2].U = ParticleDescription[PARTICLE_PLASMABEAM].EndU/2;
//	VerticesBuffer[2].V = ParticleDescription[PARTICLE_PLASMABEAM].EndV/2;

	temp.vx = trailPtr->Vertex[1].vx - MUL_FIXED(trailPtr->Perp[1].vx,trailPtr->Size[1]/65536);
	temp.vy = trailPtr->Vertex[1].vy - MUL_FIXED(trailPtr->Perp[1].vy,trailPtr->Size[1]/65536);
	temp.vz	= trailPtr->Vertex[1].vz - MUL_FIXED(trailPtr->Perp[1].vz,trailPtr->Size[1]/65536);
	VerticesBuffer[3].U = (temp.vx+temp.vz)*8192+Uoffset;
	VerticesBuffer[3].V = (temp.vy+temp.vz)*8192+Voffset;
	TranslatePointIntoViewspace(&temp);
	VerticesBuffer[3].X = temp.vx;
	VerticesBuffer[3].Y = temp.vy;
	VerticesBuffer[3].Z	= temp.vz;
//	VerticesBuffer[3].U = ParticleDescription[PARTICLE_PLASMABEAM].EndU/2;
//	VerticesBuffer[3].V = ParticleDescription[PARTICLE_PLASMABEAM].StartV/2;

	{
		extern int SpecialFXImageNumber;
		extern int CloudyImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = SpecialFXImageNumber;
		fakeHeader.PolyColour = CloudyImageNumber;
	}
	RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;
	

 	{
		int i;
		for (i=0; i<4; i++) 
		{
			VerticesBuffer[i].A = trailPtr->Size[i/2]/65536;


			VerticesBuffer[i].R = 255;
			VerticesBuffer[i].G	= 255;
			VerticesBuffer[i].B = 255;
			VerticesBuffer[i].SpecularR = 0;
			VerticesBuffer[i].SpecularG = 0;
			VerticesBuffer[i].SpecularB = 0;

		}
		RenderPolygon.NumberOfVertices=4;
	}
			
	GouraudTexturedPolygon_ClipWithZ();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeX();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveX();
	if(RenderPolygon.NumberOfVertices<3) return;
	D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
}

extern void RenderParticlesInMirror(void)
{
	/* now render particles */
	int i = NumActiveParticles;
	PARTICLE *particlePtr = ParticleStorage;

//  	RenderPlayersImageInMirror();

	D3D_DecalSystem_Setup();
	while(i--)
	{
		switch(particlePtr->ParticleID)
		{
			case PARTICLE_FLARESMOKE:
			case PARTICLE_STEAM:
			case PARTICLE_BLACKSMOKE:
			case PARTICLE_IMPACTSMOKE:
			case PARTICLE_SMOKECLOUD:
			{
				particlePtr->Position.vx = MirroringAxis - particlePtr->Position.vx;
				RenderParticle(particlePtr);
				particlePtr->Position.vx = MirroringAxis - particlePtr->Position.vx;
				break;
			}
			default:
				break;
		}
		particlePtr++;
	}
//			DrawingAReflection=1;
//			DrawingAReflection=0;

	{
		extern int NumOnScreenBlocks;
		extern DISPLAYBLOCK *OnScreenBlockList[];
		int numOfObjects = NumOnScreenBlocks;
		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];

			if (!objectPtr->ObShape && objectPtr->SfxPtr)
			{
				DrawSfxObject(objectPtr);
			}
		}
	}
	D3D_DecalSystem_End();

}



#if 0

int FogNotSetupYet=1;
int Fog_Phi[256];
int Fog_Theta[256];
int Fog_Radius[256];
int	Fog_PhiDelta[256];
int Fog_ThetaDelta[256];
int Fog_RadiusDelta[256];

void RenderFog(void)
{
	int i;
	if (FogNotSetupYet)
	{
		FogNotSetupYet=0;
		for (i=0; i<256; i++)
		{
			Fog_Phi[i] = FastRandom()&2047;
			Fog_Theta[i] = FastRandom()&4095;
			Fog_Radius[i] = (FastRandom()&8191)-4096;
			Fog_PhiDelta[i] = (FastRandom()&1023)-512;
			Fog_ThetaDelta[i] = (FastRandom()&1023)-512;
			Fog_RadiusDelta[i] = (FastRandom()&255)+255;
		}

	}
	for (i=0; i<255; i++)
	{
		int phi = Fog_Phi[i];
		int theta = Fog_Theta[i];
		int radius = Fog_Radius[i];

		{
			PARTICLE particle;
			particle.ParticleID = PARTICLE_NONCOLLIDINGFLAME;
			particle.Position.vx = MUL_FIXED(MUL_FIXED(GetCos(phi),GetSin(theta)),radius);
			particle.Position.vy = MUL_FIXED(MUL_FIXED(GetSin(phi),GetSin(theta)),radius);
			if (particle.Position.vy<0)particle.Position.vy=-particle.Position.vy;
			particle.Position.vz = MUL_FIXED(GetCos(theta),radius);
			particle.Colour = RGBA_MAKE(255,255,255,255);
			particle.Size = 100;
			RenderParticle(&particle);
	  	}
		Fog_Phi[i]+=	MUL_FIXED(Fog_PhiDelta[i],NormalFrameTime);
		Fog_Theta[i]+=	MUL_FIXED(Fog_ThetaDelta[i],NormalFrameTime);
		Fog_Phi[i]&=2047;
		Fog_Theta[i]&=4095;
		if (Fog_Radius[i]>4096)
		{	
			Fog_Radius[i]-=	MUL_FIXED(Fog_RadiusDelta[i],NormalFrameTime);
		}
		else
		{	
			Fog_Radius[i]+=	MUL_FIXED(Fog_RadiusDelta[i],NormalFrameTime);
		}

	}

}
#endif








#if 1
void TimeScaleThingy()
{
	extern int NumActiveBlocks;
	extern DISPLAYBLOCK *ActiveBlockList[];
	extern int TimeScale;
	int DesiredTimeScale=ONE_FIXED;
	extern int RealFrameTime;
	
	int i;

	for(i=0;i<NumActiveBlocks;i++)
	{
		STRATEGYBLOCK* sbPtr = ActiveBlockList[i]->ObStrategyBlock;
		if(sbPtr)
		{
			switch(sbPtr->I_SBtype)
			{
				case I_BehaviourAlien:
				case I_BehaviourQueenAlien :
				case I_BehaviourFaceHugger :
				case I_BehaviourPredator :
				case I_BehaviourXenoborg :
				case I_BehaviourMarine :
				case I_BehaviourSeal :
				case I_BehaviourPredatorAlien :
				case I_BehaviourAutoGun :
					DesiredTimeScale=MUL_FIXED(DesiredTimeScale,ONE_FIXED*.8);
					break;

				case I_BehaviourGrenade :
				case I_BehaviourRocket :
				case I_BehaviourFrisbee:
				case I_BehaviourPulseGrenade :
				case I_BehaviourMolotov :
					DesiredTimeScale=MUL_FIXED(DesiredTimeScale,ONE_FIXED*.7);
					break;
					
				default: ;
			}
		}
	}
	
	for(i=0; i<MAX_NO_OF_EXPLOSIONS; i++)
	{
		if (ExplosionStorage[i].LifeTime)
		{
			DesiredTimeScale=MUL_FIXED(DesiredTimeScale,ONE_FIXED*.5);
		}
	}

	if(DesiredTimeScale<ONE_FIXED) DesiredTimeScale=MUL_FIXED(DesiredTimeScale,ONE_FIXED*.8);

	if(DesiredTimeScale<ONE_FIXED/10) DesiredTimeScale=ONE_FIXED/10;


	if(TimeScale<DesiredTimeScale)
	{
		TimeScale+=RealFrameTime/8;
		if(TimeScale>DesiredTimeScale) TimeScale=DesiredTimeScale;
	}
	else if (TimeScale>DesiredTimeScale)
	{
		TimeScale-=RealFrameTime;
		if(TimeScale<DesiredTimeScale) TimeScale=DesiredTimeScale;
	}
}
#else
void TimeScaleThingy()
{
	static int time=0;
	extern int TimeScale;
	extern int RealFrameTime;
	int ts;

	time += RealFrameTime;
	
	ts = GetSin((time/64)&4095);
	ts = MUL_FIXED(ts,ts)/2;

	TimeScale = 32678 + ts;
}
#endif




/*----------------------**
**  Load/Save particles **
**----------------------*/

typedef struct particle_save_block_header
{
	SAVE_BLOCK_HEADER header;
	
	int NumActiveParticles;
	int NumberOfBloodParticles;

	//followed by paricle array after this block

}PARTICLE_SAVE_BLOCK_HEADER;

void Load_Particles(SAVE_BLOCK_HEADER* header)
{
	PARTICLE_SAVE_BLOCK_HEADER* block = (PARTICLE_SAVE_BLOCK_HEADER*) header;
	PARTICLE* saved_particle = (PARTICLE*)(block+1);
	int expected_size;
	int i;

	//make sure the block is the correct size
	expected_size = sizeof(*block);
	expected_size += sizeof(PARTICLE) * block->NumActiveParticles;
	if(header->size != expected_size) return;

	//right copy the stuff then
	NumActiveParticles = block->NumActiveParticles;
	NumberOfBloodParticles = block->NumberOfBloodParticles;

	for(i=0;i<NumActiveParticles;i++)
	{
		ParticleStorage[i] = *saved_particle++;
	}	
}

void Save_Particles()
{
	PARTICLE_SAVE_BLOCK_HEADER* block;
	int i;
	
	if(!NumActiveParticles) return;

	//get memory for header
	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_Particles;
	block->header.size = sizeof(*block) + NumActiveParticles * sizeof(PARTICLE);

	block->NumActiveParticles = NumActiveParticles;
	block->NumberOfBloodParticles = NumberOfBloodParticles;
	
	
	//now save the particles
	for(i=0;i<NumActiveParticles;i++)
	{
		PARTICLE* particle = GetPointerForSaveBlock(sizeof(PARTICLE));
		*particle = ParticleStorage[i];
	}	
}

/*----------------------**
** Load/Save Explosions **
**----------------------*/

typedef struct explosion_save_block_header
{
	SAVE_BLOCK_HEADER header;

	int NumActiveExplosions;
	//followed by explosion array after this block

}EXPLOSION_SAVE_BLOCK_HEADER;



void Load_VolumetricExplosions(SAVE_BLOCK_HEADER* header)
{
	int i;
	EXPLOSION_SAVE_BLOCK_HEADER* block = (EXPLOSION_SAVE_BLOCK_HEADER*) header;
	VOLUMETRIC_EXPLOSION* saved_explosion = (VOLUMETRIC_EXPLOSION*) (block+1);
	int expected_size;

	//make sure the block is the correct size
	expected_size = sizeof(*block);
	expected_size += sizeof(VOLUMETRIC_EXPLOSION) * block->NumActiveExplosions;
	if(header->size != expected_size) return;

	for(i=0;i<block->NumActiveExplosions;i++)
	{
		VOLUMETRIC_EXPLOSION* explosion = AllocateVolumetricExplosion();
		if(explosion)
		{
			*explosion = *saved_explosion++;
		}
	}
	
}

void Save_VolumetricExplosions()
{
	int i;
	EXPLOSION_SAVE_BLOCK_HEADER* block;
	int NumActiveExplosions = 0;

	//first find the number of explosions currently active
	for(i=0;i<MAX_NO_OF_EXPLOSIONS;i++)
	{
		if(ExplosionStorage[i].LifeTime) NumActiveExplosions++;
	}

	//don's save if there aren't any
	if(!NumActiveExplosions) return;

	//get memory for header
	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_VolumetricExplosions;
	block->header.size = sizeof(*block) + NumActiveExplosions * sizeof(VOLUMETRIC_EXPLOSION);

	block->NumActiveExplosions = NumActiveExplosions;

	//now save the explosions
	for(i=0;i<MAX_NO_OF_EXPLOSIONS;i++)
	{
		if(ExplosionStorage[i].LifeTime)
		{
			VOLUMETRIC_EXPLOSION* explosion = GetPointerForSaveBlock(sizeof(VOLUMETRIC_EXPLOSION));
			*explosion = ExplosionStorage[i];
		}
	}
}


/*----------------------------**
** Load/Save pheromone trails **
**----------------------------*/

typedef struct pheromone_save_block_header
{
	SAVE_BLOCK_HEADER header;

	int NumActiveTrails;
	//followed by pheromone array after this block

}PHEROMONE_SAVE_BLOCK_HEADER;



void Load_PheromoneTrails(SAVE_BLOCK_HEADER* header)
{
	int i;
	PHEROMONE_SAVE_BLOCK_HEADER* block = (PHEROMONE_SAVE_BLOCK_HEADER*) header;
	PHEROMONE_TRAIL* saved_trail = (PHEROMONE_TRAIL*) (block+1);
	int expected_size;

	//make sure the block is the correct size
	expected_size = sizeof(*block);
	expected_size += sizeof(PHEROMONE_TRAIL) * block->NumActiveTrails;
	if(header->size != expected_size) return;

	for(i=0;i<block->NumActiveTrails;i++)
	{
		PHEROMONE_TRAIL* trail = AllocatePheromoneTrail();
		if(trail) 
		{
			*trail = *saved_trail++;	
		}
	}

}

void Save_PheromoneTrails()
{
	PHEROMONE_SAVE_BLOCK_HEADER* block;
	int i;

	if(!NumActiveTrails) return;

	//get memory for header
	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_PheromoneTrail;
	block->header.size = sizeof(*block) + NumActiveTrails * sizeof(PHEROMONE_TRAIL);

	block->NumActiveTrails = NumActiveTrails;


	//now save the trails
	for(i=0;i<NumActiveTrails;i++)
	{
		PHEROMONE_TRAIL* trail = GET_SAVE_BLOCK_POINTER(trail);
		*trail = TrailStorage[i];
	}
	
}
