#ifndef _kshape_h_ /* Is this your first time? */
#define _kshape_h_ 1

#include "particle.h"
#include "sphere.h"


typedef struct 
{
	/* base coords */
	int X;
	int Y;
	int Z;

	/* texels and intensity */
	int U;
	int V;

	/* coloured components */
	unsigned char R;
	unsigned char G;
	unsigned char B;

	/* alpha component */
	unsigned char A;

	/* specular colour */
	unsigned char SpecularR;
	unsigned char SpecularG;
	unsigned char SpecularB;

	/* fog component */
	unsigned char Fog;

} RENDERVERTEX;

typedef struct
{
	/* stamp used for lazy evaluation */
	int Stamp;

	/* colour scalings */
	unsigned char R;
	unsigned char G;
	unsigned char B;

	/* specular colour */
	unsigned char SpecularR;
	unsigned char SpecularG;
	unsigned char SpecularB;

	/* fog component */
	unsigned char Fog;

} COLOURINTENSITIES;


typedef struct 
{
	RENDERVERTEX Vertices[maxpolypts];

	unsigned int NumberOfVertices;

	unsigned int MinZ;
	unsigned int MaxZ;

	int ImageIndex;

	unsigned char IsTextured :1;
	unsigned char IsLit :1;
	unsigned char IsSpecularLit :1;
	enum TRANSLUCENCY_TYPE TranslucencyMode;

} RENDERPOLYGON;

extern RENDERVERTEX VerticesBuffer[9];
extern RENDERPOLYGON RenderPolygon;




enum LIGHTING_MODEL_ID
{
	LIGHTING_MODEL_STANDARD,
	LIGHTING_MODEL_HIERARCHICAL,
	LIGHTING_MODEL_PRELIT,
};


extern void InitialiseLightIntensityStamps(void);

extern int FindHeatSourcesInHModel(DISPLAYBLOCK *dispPtr);


extern void TranslationSetup(void);
extern void TranslatePointIntoViewspace(VECTORCH *pointPtr);

extern void CheckRenderStatesForModule(MODULE *modulePtr);


extern void RenderDecal(DECAL *decalPtr);
extern void RenderParticle(PARTICLE *particlePtr);
void RenderInsideAlienTongue(int offset);
void RenderPredatorTargetingSegment(int theta, int scale, int drawInRed);
void RenderPredatorPlasmaCasterCharge(int value, VECTORCH *worldOffsetPtr, MATRIXCH *orientationPtr);
void CreateStarArray(void);
void OutputTranslucentPolyList(void);
void RenderLightFlare(VECTORCH *positionPtr, unsigned int colour);
extern void RenderFlechetteParticle(PARTICLE *particlePtr);
void RenderExplosionSurface(VOLUMETRIC_EXPLOSION *explosionPtr);
void ClearTranslucentPolyList(void);
void AddHierarchicalShape(DISPLAYBLOCK *dptr, VIEWDESCRIPTORBLOCK *VDB_Ptr);

/* KJL 10:25:44 7/23/97 - this offset is used to push back the normal game gfx,
so that the HUD can be drawn over the top without sinking into walls, etc. */
extern int HeadUpDisplayZOffset;

/* KJL 16:17:13 11/02/98 - heat source containment */
typedef struct
{
	VECTORCH Position;
} HEATSOURCE;

#define MAX_NUMBER_OF_HEAT_SOURCES 10
extern HEATSOURCE HeatSourceList[];
extern int NumberOfHeatSources;

#define MIRRORING_ON 1

#if MIRRORING_ON
extern int MirroringActive;
extern int MirroringAxis;
#endif

#endif
