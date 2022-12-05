#ifndef _included_decal_h_ /* Is this your first time? */
#define _included_decal_h_ 1

#include "d3_func.h"
#include "vision.h"

enum DECAL_ID
{
	DECAL_FMV=0,
	DECAL_SCORCHED,
	DECAL_BULLETHOLE,
	DECAL_PREDATOR_BLOOD,
	DECAL_ALIEN_BLOOD,
	DECAL_HUMAN_BLOOD,
	DECAL_ANDROID_BLOOD,

	DECAL_LASERTARGET,
	DECAL_SHAFTOFLIGHT,
	DECAL_SHAFTOFLIGHT_OUTER,

	MAX_NO_OF_DECAL_IDS
};

typedef struct
{
	enum DECAL_ID DecalID;
	VECTORCH Vertices[4];
	VECTORCH Direction[4];
	VECTORCH Centre;
	int ModuleIndex;

	int CurrentSize;
	int TargetSize;
	int UOffset;

} DECAL;

typedef struct
{
	enum DECAL_ID DecalID;
	VECTORCH Vertices[4];
	int ModuleIndex;
	int UOffset;

} FIXED_DECAL;


typedef struct
{
	enum DECAL_ID DecalID;
	VECTORCH Vertices[4];
	VECTORCH Centre;
} OBJECT_DECAL;

typedef struct
{
	int StartU;
	int StartV;
	int EndU;
	int EndV;
	
	int MinSize;
	int MaxSize;
	int GrowthRate;

	int MaxSubclassNumber;
	int UOffsetForSubclass;

	enum TRANSLUCENCY_TYPE TranslucencyType;

	unsigned char Alpha;
	unsigned char RedScale[NUMBER_OF_VISION_MODES];
	unsigned char GreenScale[NUMBER_OF_VISION_MODES];
	unsigned char BlueScale[NUMBER_OF_VISION_MODES];

	unsigned char IsLit:1;
	unsigned char CanCombine:1;

} DECAL_DESC;


typedef struct 
{
	VECTORCH Position[3];
	VECTORCH Normal[3];
	VECTORCH LightSource;
	char DotIsOnPlayer;
//	DPID TargetID;
	int TargetID;
	
	int ShouldBeDrawn;

} THREE_LASER_DOT_DESC;

extern void InitialiseDecalSystem(void);
extern void MakeDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex);
extern void AddDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex);
extern void HandleDecalSystem(void);

struct section_data; // hmodel.h
extern void AddDecalToHModel(VECTORCH *normalPtr, VECTORCH *positionPtr, struct section_data *sectionPtr);
void ScanHModelForDecals(DISPLAYBLOCK *objectPtr, struct section_data *sectionDataPtr);

extern FIXED_DECAL* AllocateFixedDecal(void);
extern void RemoveFixedDecal(void);
extern void RemoveAllFixedDecals(void);


extern DECAL_DESC DecalDescription[];
extern THREE_LASER_DOT_DESC PredatorLaserTarget;


extern FIXED_DECAL FixedDecalStorage[];
extern int NumFixedDecals;
extern int CurrentFixedDecalIndex;

#define MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION 16
#endif
