/** KJL 16:44:26 11/17/97 ***
*                           *
*  D E C A L   S Y S T E M  *
*                           *
************************KJL*/
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"
#include "bh_types.h"
#include "bh_pred.h"
#include "weapons.h"

#include "particle.h"
#include "pldnet.h"
#include "pldghost.h"
#include "kshape.h"
#include "d3d_render.h"
#include "hmodel.h"
#include "paintball.h"
#include "detaillevels.h"
#include "savegame.h"
#include "decal.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#define MAX_NO_OF_DECALS 1024
#define MAX_NO_OF_FIXED_DECALS 1024
#define DECAL_Z_OFFSET 0

THREE_LASER_DOT_DESC PredatorLaserTarget;
/* KJL 11:49:51 20/05/98 - probably not the neatest way of doing it, but
this array stores the information required to render the predator players'
laser sights. */
THREE_LASER_DOT_DESC PredatorLaserSights[NET_MAXPLAYERS];


static DECAL DecalStorage[MAX_NO_OF_DECALS];
static int NumActiveDecals;
static int CurrentDecalIndex;

FIXED_DECAL FixedDecalStorage[MAX_NO_OF_FIXED_DECALS];
int NumFixedDecals;
int CurrentFixedDecalIndex;

DECAL_DESC DecalDescription[MAX_NO_OF_DECAL_IDS] =
{
	/* DECAL_FMV */
	{
		//int StartU;
		0<<16,
		//int StartV;
		0<<16,	 
		//int EndU;
		(127)<<16,
		//int EndV;
	    (95)<<16,

		//int MinSize;
		3000,
		//int MaxSize;
		3000,
		//int GrowthRate;
		0,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_NORMAL,

		//unsigned char Alpha;
		96,
		//unsigned char RedScale;
		{255,		255,	0,		255,	255},
		//unsigned char GreenScale;
		{255,		255,	255,	0,		0},
		//unsigned char BlueScale;
		{255,		255,	0,		255,	0},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 
	},
	/* DECAL_SCORCHED */
	{
		//int StartU;
		0<<16,
		//int StartV;
		64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,

		//int MinSize;
		20,
		//int MaxSize;
		200,
		//int GrowthRate;
		200,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{128,128,128,128,128},
		//unsigned char GreenScale;
		{128,128,128,128,128},
		//unsigned char BlueScale;
		{128,128,128,128,128},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 

	},
	/* DECAL_BULLETHOLE */
	{
		//int StartU;
		224<<16,
		//int StartV;
		224<<16,
		//int EndU;
		255<<16,
		//int EndV;
		255<<16,

		//int MinSize;
		32,
		//int MaxSize;
		32,
		//int GrowthRate;
		0,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,
					 
		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,255,255,255,255},
		//unsigned char GreenScale;
		{255,255,255,255,255},
		//unsigned char BlueScale;
		{255,255,255,255,255},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 
	},
	/* DECAL_PREDATOR_BLOOD */
	{
		//int StartU;
		(0)<<16,
		//int StartV;
		224<<16,
		//int EndU;
		(31)<<16,
		//int EndV;
		255<<16,

		//int MinSize;
		20,
		//int MaxSize;
		400,
		//int GrowthRate;
		100,

		//int MaxSubclassNumber;
		3,
		//int UOffsetForSubclass;
		32<<16,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		128,
		//unsigned char RedScale;
		{0,		0,		192,	0,		255},
		//unsigned char GreenScale;
		{255,	255,	255,	255,	0},
		//unsigned char BlueScale;
		{0,		0,		192,	255,	0},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		1, 
	},
	/* DECAL_ALIEN_BLOOD */
	{
		//int StartU;
		0<<16,
		//int StartV;
		64<<16,
		//int EndU;
		63<<16,
		//int EndV;
		127<<16,

		//int MinSize;
		20,
		//int MaxSize;
		400,
		//int GrowthRate;
		100,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		128,
		//unsigned char RedScale;
		{255,	255,	0,		128,	128},
		//unsigned char GreenScale;
		{255,	255,	255,	128,	128},
		//unsigned char BlueScale;
		{0,		0,		0,		128,	128},

		//unsigned char IsLit:1;
		1,
		//unsigned char CanCombine:1;
		1, 

	},
	/* DECAL_HUMAN_BLOOD */
	{
		//int StartU;
		(0)<<16,
		//int StartV;
		224<<16,
		//int EndU;
		(31)<<16,
		//int EndV;
		255<<16,

		//int MinSize;
		20,
		//int MaxSize;
		400,
		//int GrowthRate;
		100,

		//int MaxSubclassNumber;
		4,
		//int UOffsetForSubclass;
		32<<16,

		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_INVCOLOUR,//GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{64,		64,		255,		0,		  0}, 
		//unsigned char GreenScale;
		{192,		192,	192,		112,	112},
		//unsigned char BlueScale;
		{192,		192,	255,		112,	112},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		1, 

	},
	/* DECAL_ANDROID_BLOOD */
	{
		//int StartU;
		(0)<<16,
		//int StartV;
		224<<16,
		//int EndU;
		(31)<<16,
		//int EndV;
		255<<16,

		//int MinSize;
		20,
		//int MaxSize;
		400,
		//int GrowthRate;
		100,

		//int MaxSubclassNumber;
		3,
		//int UOffsetForSubclass;
		32<<16,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		128,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{255,	255,	255,	255,	255},
		//unsigned char BlueScale;
		{255,	255,	255,	255,	255},

		//unsigned char IsLit:1;
		1,
		//unsigned char CanCombine:1;
		1, 
	},

	/* DECAL_LASERTARGET */
	{
		//int StartU;
		0<<16,
		//int StartV;
		0<<16,
		//int EndU;
		63<<16,
		//int EndV;
		63<<16,

		//int MinSize;
		20,
		//int MaxSize;
		20,
		//int GrowthRate;
		0,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		255,
		//unsigned char RedScale;
		{255,	255,	255,	255,	255},
		//unsigned char GreenScale;
		{0,		0,		255,	255,	255},
		//unsigned char BlueScale;
		{0,		0,		255,	255,	255},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 

	},
	/* DECAL_SHAFTOFLIGHT */
	{
		//int StartU;
		0,
		//int StartV;
		0,
		//int EndU;
		0,
		//int EndV;
		0,

		//int MinSize;
		0,
		//int MaxSize;
		0,
		//int GrowthRate;
		0,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
		
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		64,
		//unsigned char RedScale;
		{255,255,255,255},
		//unsigned char GreenScale;
		{255,255,255,255},
		//unsigned char BlueScale;
		{192,192,192,192},
 
		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 

	},
	/* DECAL_SHAFTOFLIGHT_OUTER */
	{
		//int StartU;
		0,
		//int StartV;
		0,
		//int EndU;
		0,
		//int EndV;
		0,

		//int MinSize;
		0,
		//int MaxSize;
		0,
		//int GrowthRate;
		0,

		//int MaxSubclassNumber;
		0,
		//int UOffsetForSubclass;
		0,
	
		//enum TRANSLUCENCY_TYPE TranslucencyType;
		TRANSLUCENCY_GLOWING,

		//unsigned char Alpha;
		32,
		//unsigned char RedScale;
		{255,255,255,255},
		//unsigned char GreenScale;
		{255,255,255,255},
		//unsigned char BlueScale;
		{192,192,192,192},

		//unsigned char IsLit:1;
		0,
		//unsigned char CanCombine:1;
		0, 

	},

};



static DECAL* AllocateDecal(void);
static int TooManyDecalsOfThisType(enum DECAL_ID decalID, VECTORCH *positionPtr);

void InitialiseDecalSystem(void)
{
// 	VECTORCH normal = {0,0,65536};
// 	VECTORCH position = {-1603,2675,8000};
	NumActiveDecals=0;
	CurrentDecalIndex=0;

 //	MakeDecal(DECAL_FMV, &normal,&position,1);
}

static DECAL* AllocateDecal(void)
{
	DECAL *decalPtr;

	decalPtr = &DecalStorage[CurrentDecalIndex];

	CurrentDecalIndex++;
	if (CurrentDecalIndex>=LocalDetailLevels.MaximumAllowedNumberOfDecals)
	{
		CurrentDecalIndex=0;
	}

	if (NumActiveDecals < LocalDetailLevels.MaximumAllowedNumberOfDecals) 
	{
		NumActiveDecals++;
	}

	LOCALASSERT(decalPtr);
	return decalPtr;
}

extern FIXED_DECAL* AllocateFixedDecal(void)
{
	FIXED_DECAL *decalPtr;


	if (CurrentFixedDecalIndex>=MAX_NO_OF_FIXED_DECALS)
	{
		return 0;
	}

	decalPtr = &FixedDecalStorage[CurrentFixedDecalIndex];
	CurrentFixedDecalIndex++;
	NumFixedDecals++;

	LOCALASSERT(decalPtr);
	return decalPtr;
}
extern void RemoveFixedDecal(void)
{
	if (CurrentFixedDecalIndex)
	{
		CurrentFixedDecalIndex--;
		NumFixedDecals--;
	}
}

extern void RemoveAllFixedDecals(void)
{
	if (PaintBallMode.IsOn)
	{
		/* pretty drastic */
		CurrentFixedDecalIndex = 0;
		NumFixedDecals = 0;
	}
}
void MakeDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex)
{
	if (TooManyDecalsOfThisType(decalID,positionPtr)) return;
	AddDecal(decalID,normalPtr,positionPtr,moduleIndex);

	/* no blood across network? */
 	if(AvP.Network!=I_No_Network) 
	{
		if(netGameData.sendDecals)
		{
			switch (decalID)
			{
				case DECAL_BULLETHOLE:
				case DECAL_SCORCHED:
				{
					AddNetMsg_MakeDecal(decalID,normalPtr,positionPtr,moduleIndex);
					break;
				}
				default:
					break;
			}
		}
	}
}

#define MAX_NO_OF_SIMILAR_DECALS_IN_ONE_PLACE 4
#define INCREMENT_IN_DECAL_SIZE (25*4)
static int TooManyDecalsOfThisType(enum DECAL_ID decalID, VECTORCH *positionPtr)
{
	int i = NumActiveDecals;
	DECAL *decalPtr = DecalStorage;
	DECAL *similarDecalsPtr[MAX_NO_OF_SIMILAR_DECALS_IN_ONE_PLACE];

	int minX,maxX, minY,maxY, minZ,maxZ;
	int decalsOfThisType = 0;
	{
		int decalSize = DecalDescription[decalID].MaxSize;
		minX = positionPtr->vx - decalSize;
		maxX = positionPtr->vx + decalSize;
		minY = positionPtr->vy - decalSize;
		maxY = positionPtr->vy + decalSize;
		minZ = positionPtr->vz - decalSize;
		maxZ = positionPtr->vz + decalSize;
	}

	while(i--)
	{
		if (decalPtr->DecalID == decalID)
		{
			if (decalPtr->Centre.vx > minX && decalPtr->Centre.vx < maxX)
			if (decalPtr->Centre.vy > minY && decalPtr->Centre.vy < maxY)
			if (decalPtr->Centre.vz > minZ && decalPtr->Centre.vz < maxZ)
			{
				similarDecalsPtr[decalsOfThisType] = decalPtr;
				decalsOfThisType++;
			}
			
		}
		if (decalsOfThisType>=MAX_NO_OF_SIMILAR_DECALS_IN_ONE_PLACE)
		{
			if (DecalDescription[decalID].CanCombine) 
			{
				int j;//= FastRandom()%MAX_NO_OF_SIMILAR_DECALS_IN_ONE_PLACE;
		  		for (j=0; j<MAX_NO_OF_SIMILAR_DECALS_IN_ONE_PLACE; j++)
				{
					similarDecalsPtr[j]->TargetSize += INCREMENT_IN_DECAL_SIZE;
					if (similarDecalsPtr[j]->TargetSize > DecalDescription[decalID].MaxSize)
					{
						similarDecalsPtr[j]->TargetSize = DecalDescription[decalID].MaxSize;
					}
					else
					{
						break;
					}
				}
			}
			return 1;
		}
		decalPtr++;
	}
	
	return 0;
}
	
void AddDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex)
{
	DECAL *decalPtr;
	MATRIXCH orientation;
	int decalSize; 
	int theta = FastRandom()&4095;
	int sin = GetSin(theta);
	int cos = GetCos(theta);


	MakeMatrixFromDirection(normalPtr,&orientation);
	
	if (decalID == DECAL_BULLETHOLE)
	{
		MakeImpactSmoke(&orientation,positionPtr);
	}

	decalPtr = AllocateDecal();
	
	decalPtr->DecalID = decalID;

	decalPtr->Centre = *positionPtr;

	if(DecalDescription[decalID].GrowthRate)
	{
		decalSize = ONE_FIXED;
		decalPtr->Direction[0].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(-decalSize,sin);
		decalPtr->Direction[0].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(-decalSize,cos);
		decalPtr->Direction[0].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Direction[0]),&orientation);
		Normalise(&(decalPtr->Direction[0]));

		decalPtr->Direction[1].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(-decalSize,sin);
		decalPtr->Direction[1].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(-decalSize,cos);
		decalPtr->Direction[1].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Direction[1]),&orientation);
		Normalise(&(decalPtr->Direction[1]));

		decalPtr->Direction[2].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(decalSize,sin);
		decalPtr->Direction[2].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(decalSize,cos);
		decalPtr->Direction[2].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Direction[2]),&orientation);
		Normalise(&(decalPtr->Direction[2]));

		decalPtr->Direction[3].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(decalSize,sin);
		decalPtr->Direction[3].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(decalSize,cos);
		decalPtr->Direction[3].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Direction[3]),&orientation);
		Normalise(&(decalPtr->Direction[3]));
	   	decalPtr->CurrentSize = DecalDescription[decalID].MinSize;
		decalPtr->TargetSize = DecalDescription[decalID].MaxSize;
		if (DecalDescription[decalID].CanCombine) decalPtr->TargetSize/=4;
	}
	else
	{
		decalSize = DecalDescription[decalID].MinSize;
		decalPtr->Vertices[0].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(-decalSize,sin);
		decalPtr->Vertices[0].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(-decalSize,cos);
		decalPtr->Vertices[0].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Vertices[0]),&orientation);
		decalPtr->Vertices[0].vx += positionPtr->vx;
		decalPtr->Vertices[0].vy += positionPtr->vy;
		decalPtr->Vertices[0].vz += positionPtr->vz;


		decalPtr->Vertices[1].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(-decalSize,sin);
		decalPtr->Vertices[1].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(-decalSize,cos);
		decalPtr->Vertices[1].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Vertices[1]),&orientation);
		decalPtr->Vertices[1].vx += positionPtr->vx;
		decalPtr->Vertices[1].vy += positionPtr->vy;
		decalPtr->Vertices[1].vz += positionPtr->vz;

		decalPtr->Vertices[2].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(decalSize,sin);
		decalPtr->Vertices[2].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(decalSize,cos);
		decalPtr->Vertices[2].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Vertices[2]),&orientation);
		decalPtr->Vertices[2].vx += positionPtr->vx;
		decalPtr->Vertices[2].vy += positionPtr->vy;
		decalPtr->Vertices[2].vz += positionPtr->vz;

		decalPtr->Vertices[3].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(decalSize,sin);
		decalPtr->Vertices[3].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(decalSize,cos);
		decalPtr->Vertices[3].vz = DECAL_Z_OFFSET;
		RotateVector(&(decalPtr->Vertices[3]),&orientation);
		decalPtr->Vertices[3].vx += positionPtr->vx;
		decalPtr->Vertices[3].vy += positionPtr->vy;
		decalPtr->Vertices[3].vz += positionPtr->vz;

	}

	decalPtr->ModuleIndex = moduleIndex;

	switch (decalID)
	{
		case DECAL_HUMAN_BLOOD:
		case DECAL_PREDATOR_BLOOD:
		case DECAL_ANDROID_BLOOD:
		{	
			decalPtr->UOffset = (FastRandom()&1)*(32<<16);
			if (normalPtr->vy<-32768)
			{
				decalPtr->UOffset+=64<<16;
			}
			else
			{
				decalPtr->TargetSize = DecalDescription[decalID].MaxSize;
				decalPtr->CurrentSize = decalPtr->TargetSize-1;
			}
			break;
		}
		default:
		{
			decalPtr->UOffset = 0;
			break;
		}

	}

}

void RenderLaserTarget(THREE_LASER_DOT_DESC *laserTargetPtr)
{
	extern MODULE * playerPherModule;
	int i;

	if(!playerPherModule) return;

	i=2;
	do
	{
		DECAL decal;
		MATRIXCH orientation;
		int decalSize; 
		VECTORCH *positionPtr = &(laserTargetPtr->Position[i]);

		MakeMatrixFromDirection(&(laserTargetPtr->Normal[i]),&orientation);
		
		decal.DecalID = DECAL_LASERTARGET;
		decalSize = DecalDescription[DECAL_LASERTARGET].MaxSize;

		decal.Vertices[0].vx = -decalSize;
		decal.Vertices[0].vy = -decalSize;
		decal.Vertices[0].vz = DECAL_Z_OFFSET;
		RotateVector(&(decal.Vertices[0]),&orientation);
		decal.Vertices[0].vx += positionPtr->vx;
		decal.Vertices[0].vy += positionPtr->vy;
		decal.Vertices[0].vz += positionPtr->vz;

		decal.Vertices[1].vx = decalSize;
		decal.Vertices[1].vy = -decalSize;
		decal.Vertices[1].vz = DECAL_Z_OFFSET;
		RotateVector(&(decal.Vertices[1]),&orientation);
		decal.Vertices[1].vx += positionPtr->vx;
		decal.Vertices[1].vy += positionPtr->vy;
		decal.Vertices[1].vz += positionPtr->vz;

		decal.Vertices[2].vx = decalSize;
		decal.Vertices[2].vy = decalSize;
		decal.Vertices[2].vz = DECAL_Z_OFFSET;
		RotateVector(&(decal.Vertices[2]),&orientation);
		decal.Vertices[2].vx += positionPtr->vx;
		decal.Vertices[2].vy += positionPtr->vy;
		decal.Vertices[2].vz += positionPtr->vz;

		decal.Vertices[3].vx = -decalSize;
		decal.Vertices[3].vy = decalSize;
		decal.Vertices[3].vz = DECAL_Z_OFFSET;
		RotateVector(&(decal.Vertices[3]),&orientation);
		decal.Vertices[3].vx += positionPtr->vx;
		decal.Vertices[3].vy += positionPtr->vy;
		decal.Vertices[3].vz += positionPtr->vz;

		decal.ModuleIndex = playerPherModule->m_index;
		decal.UOffset = 0;
		RenderDecal(&decal);
	}
	while(i--);
}

void HandleDecalSystem(void)
{
	D3D_DecalSystem_Setup();
	
	if (NumActiveDecals > LocalDetailLevels.MaximumAllowedNumberOfDecals)
	{
		NumActiveDecals = LocalDetailLevels.MaximumAllowedNumberOfDecals;
	}

	{
		int i = NumActiveDecals;
		DECAL *decalPtr = DecalStorage;
	//	textprint("Decals Active: %d\n",i);
		while(i--)
		{
			DECAL_DESC *decalDescPtr = &DecalDescription[decalPtr->DecalID];

			if (decalDescPtr->GrowthRate && decalPtr->CurrentSize < decalPtr->TargetSize)
			{
				int i;
				extern int NormalFrameTime;
				decalPtr->CurrentSize += MUL_FIXED(decalDescPtr->GrowthRate,NormalFrameTime);
				if (decalPtr->CurrentSize > decalPtr->TargetSize)
				{
					decalPtr->CurrentSize = decalPtr->TargetSize;
				}
				for (i=0; i<4; i++)
				{
					decalPtr->Vertices[i].vx = MUL_FIXED(decalPtr->Direction[i].vx,decalPtr->CurrentSize);
					decalPtr->Vertices[i].vy = MUL_FIXED(decalPtr->Direction[i].vy,decalPtr->CurrentSize);
					decalPtr->Vertices[i].vz = MUL_FIXED(decalPtr->Direction[i].vz,decalPtr->CurrentSize);
					decalPtr->Vertices[i].vx += decalPtr->Centre.vx;
					decalPtr->Vertices[i].vy += decalPtr->Centre.vy;
					decalPtr->Vertices[i].vz += decalPtr->Centre.vz;
				}

			}
			RenderDecal(decalPtr);
			decalPtr++;
		}
	}
	{
		int i = NumFixedDecals;
		DECAL dummyDecal;
		FIXED_DECAL *decalPtr = FixedDecalStorage;
	//	textprint("Decals Active: %d\n",i);
		while(i--)
		{
			dummyDecal.DecalID = decalPtr->DecalID;
			dummyDecal.Vertices[0] = decalPtr->Vertices[0];
			dummyDecal.Vertices[1] = decalPtr->Vertices[1];
			dummyDecal.Vertices[2] = decalPtr->Vertices[2];
			dummyDecal.Vertices[3] = decalPtr->Vertices[3];
			dummyDecal.ModuleIndex = decalPtr->ModuleIndex;
			dummyDecal.UOffset = decalPtr->UOffset;

			RenderDecal(&dummyDecal);
			decalPtr++;;
		}
	}
//	CubeOMatic();
	if (AvP.PlayerType == I_Predator && PredatorLaserTarget.ShouldBeDrawn)
	{
		RenderLaserTarget(&PredatorLaserTarget);
	}

 	
	{
		extern int NumActiveBlocks;
		extern DISPLAYBLOCK *ActiveBlockList[];
		int numOfObjects = NumActiveBlocks;
		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = ActiveBlockList[--numOfObjects];
			STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;

			if (sbPtr)
			{
				switch(sbPtr->I_SBtype)
				{
					case (I_BehaviourNetGhost):
				   	{
					 	NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
						GLOBALASSERT(AvP.Network!=I_No_Network);
						if (ghostData->type == I_BehaviourPredatorPlayer)
						{
							extern DPID AVPDPNetID;
							int playerIndex = PlayerIdInPlayerList(ghostData->playerId);
							if(playerIndex==NET_IDNOTINPLAYERLIST)
							{
								continue;
							}
							if ((ghostData->CurrentWeapon != WEAPON_PRED_RIFLE)
							  &&(ghostData->CurrentWeapon != WEAPON_PRED_SHOULDERCANNON))
							{
								continue;
							}

							if (AVPDPNetID==PredatorLaserSights[playerIndex].TargetID)
							{
								SECTION_DATA *plasma_muzzle;
								plasma_muzzle=GetThisSectionData(ghostData->HModelController.section_data,"dum flash");
								if (plasma_muzzle!=NULL)
								{
									extern void RenderLightFlare(VECTORCH *positionPtr, unsigned int colour);
									RenderLightFlare(&plasma_muzzle->World_Offset,0xffff0000);
								}
							}
							else
							{
								RenderLaserTarget(&PredatorLaserSights[playerIndex]);
							}
						}
						break;
					}
					case (I_BehaviourPredator):
					{
						extern void RenderLightFlare(VECTORCH *positionPtr, unsigned int colour);
						PREDATOR_STATUS_BLOCK *statusPtr = (PREDATOR_STATUS_BLOCK *)sbPtr->SBdataptr;
						LOCALASSERT(statusPtr);
//						RenderLightFlare(&(sbPtr->DynPtr->Position),0x7fffffff);
//						textprint("predator?\n");
						if(statusPtr->Pred_Laser_On)
						{
							if (statusPtr->Pred_Laser_Sight.DotIsOnPlayer)
							{
//								textprint("Render flare\n");
								RenderLightFlare(&(statusPtr->Pred_Laser_Sight.LightSource),0xffff0000);
							}
							else
							{
//								textprint("Render dots\n");
								RenderLaserTarget(&(statusPtr->Pred_Laser_Sight));
							}
						}
						break;
						
					}
					default:
						break;
				}
				
			}
		}
	}

	/* KJL 11:03:53 01/10/98 - check for paintball mode */
	{
		if (PaintBallMode.IsOn)
		{
			PaintBallMode_DrawCurrentDecalAtTarget();
		}
	}
}

void AddDecalToHModel(VECTORCH *normalPtr, VECTORCH *positionPtr, SECTION_DATA *sectionDataPtr)
{
	enum DECAL_ID decalID;
	OBJECT_DECAL *decalPtr;
	MATRIXCH orientation;
	VECTORCH v;

	int decalSize; 
	int theta,sin,cos;

	if (!LocalDetailLevels.DrawHierarchicalDecals) return;

	theta = FastRandom()&4095;
	sin = GetSin(theta);
	cos = GetCos(theta);

	
	LOCALASSERT(sectionDataPtr->NumberOfDecals <= MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION);
	{
		MATRIXCH mat = sectionDataPtr->SecMat;
		VECTORCH n = *normalPtr;

		TransposeMatrixCH(&mat);

		RotateVector(&n,&mat);
		MakeMatrixFromDirection(&n,&orientation);

		v = *positionPtr;
		v.vx -= sectionDataPtr->World_Offset.vx;
		v.vy -= sectionDataPtr->World_Offset.vy;
		v.vz -= sectionDataPtr->World_Offset.vz;
		RotateVector(&v,&mat);
	}
	
	
	{
		SECTION	*sectionPtr = sectionDataPtr->sempai;
	
		if(sectionPtr->flags&section_sprays_blood)
		{
			decalID = DECAL_HUMAN_BLOOD;
		}
		else if(sectionPtr->flags&section_sprays_predoblood)
		{
			decalID = DECAL_PREDATOR_BLOOD;
		}
		else if(sectionPtr->flags&section_sprays_acid)
		{
			decalID = DECAL_ALIEN_BLOOD;
		}
		else
		{
			decalID = DECAL_BULLETHOLE;
		}

	}

	decalPtr = &sectionDataPtr->Decals[sectionDataPtr->NextDecalToUse];
	
	if (sectionDataPtr->NextDecalToUse >= MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION-1)
	{
		sectionDataPtr->NextDecalToUse = 0;
	}
	else
	{
		sectionDataPtr->NextDecalToUse++;
	}

	if (sectionDataPtr->NumberOfDecals < MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION)
	{
		sectionDataPtr->NumberOfDecals++;
	}

	
	decalPtr->DecalID = decalID;
	decalSize = 40;//DecalDescription[decalID].MaxSize;

	decalPtr->Centre = v;

	decalPtr->Vertices[0].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(-decalSize,sin);
	decalPtr->Vertices[0].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(-decalSize,cos);
	decalPtr->Vertices[0].vz = DECAL_Z_OFFSET;
	RotateVector(&(decalPtr->Vertices[0]),&orientation);
	decalPtr->Vertices[0].vx += v.vx;
	decalPtr->Vertices[0].vy += v.vy;
	decalPtr->Vertices[0].vz += v.vz;

	decalPtr->Vertices[1].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(-decalSize,sin);
	decalPtr->Vertices[1].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(-decalSize,cos);
	decalPtr->Vertices[1].vz = DECAL_Z_OFFSET;
	RotateVector(&(decalPtr->Vertices[1]),&orientation);
	decalPtr->Vertices[1].vx += v.vx;
	decalPtr->Vertices[1].vy += v.vy;
	decalPtr->Vertices[1].vz += v.vz;

	decalPtr->Vertices[2].vx = MUL_FIXED(decalSize,cos) - MUL_FIXED(decalSize,sin);
	decalPtr->Vertices[2].vy = MUL_FIXED(decalSize,sin) + MUL_FIXED(decalSize,cos);
	decalPtr->Vertices[2].vz = DECAL_Z_OFFSET;
	RotateVector(&(decalPtr->Vertices[2]),&orientation);
	decalPtr->Vertices[2].vx += v.vx;
	decalPtr->Vertices[2].vy += v.vy;
	decalPtr->Vertices[2].vz += v.vz;

	decalPtr->Vertices[3].vx = MUL_FIXED(-decalSize,cos) - MUL_FIXED(decalSize,sin);
	decalPtr->Vertices[3].vy = MUL_FIXED(-decalSize,sin) + MUL_FIXED(decalSize,cos);
	decalPtr->Vertices[3].vz = DECAL_Z_OFFSET;
	RotateVector(&(decalPtr->Vertices[3]),&orientation);
	decalPtr->Vertices[3].vx += v.vx;
	decalPtr->Vertices[3].vy += v.vy;
	decalPtr->Vertices[3].vz += v.vz;
}


void ScanHModelForDecals(DISPLAYBLOCK *objectPtr, SECTION_DATA *sectionDataPtr)
{
	SECTION *sectionPtr = sectionDataPtr->sempai;
		
	/* Unreal things aren't drawn... */
	if (!(sectionDataPtr->flags&section_data_notreal) && (sectionPtr->Shape!=NULL))
	{
		/* does the object have decals? */
		extern MODULE *playerPherModule;
		if(sectionDataPtr->NumberOfDecals && playerPherModule)
		{
			int d;
			for(d=0; d<sectionDataPtr->NumberOfDecals; d++)
			{
				int i;
				DECAL decal;
				decal.DecalID = sectionDataPtr->Decals[d].DecalID;
				decal.ModuleIndex = playerPherModule->m_index;
				decal.UOffset = 0;

				for(i=0; i<5; i++)
				{
					decal.Vertices[i] = sectionDataPtr->Decals[d].Vertices[i];
					RotateVector(&(decal.Vertices[i]),&(sectionDataPtr->SecMat));
					decal.Vertices[i].vx += sectionDataPtr->World_Offset.vx;
					decal.Vertices[i].vy += sectionDataPtr->World_Offset.vy;
					decal.Vertices[i].vz += sectionDataPtr->World_Offset.vz;
				}
				RenderDecal(&decal);
			}
		}
	}

	/* Now call recursion... */
	if (sectionDataPtr->First_Child!=NULL)
	{
		SECTION_DATA *childrenListPtr = sectionDataPtr->First_Child;

		while (childrenListPtr!=NULL)
		{
			ScanHModelForDecals(objectPtr,childrenListPtr);
			childrenListPtr=childrenListPtr->Next_Sibling;
		}
	}

}



/*------------------**
** Load/Save Decals **
**------------------*/

typedef struct decal_save_block_header
{
	SAVE_BLOCK_HEADER header;

	int NumActiveDecals;

	//followed by array of decals
}DECAL_SAVE_BLOCK_HEADER;

void Load_Decals(SAVE_BLOCK_HEADER* header)
{
	int i;
	DECAL_SAVE_BLOCK_HEADER* block = (DECAL_SAVE_BLOCK_HEADER*) header;
	DECAL* saved_decal = (DECAL*) (block+1);
	int expected_size;

	//make sure the block is the correct size
	expected_size = sizeof(*block);
	expected_size += sizeof(DECAL) * block->NumActiveDecals;
	if(header->size != expected_size) return;


	for(i=0;i<block->NumActiveDecals;i++)
	{
		DECAL* decal = AllocateDecal();
		if(decal) 
		{
			*decal = *saved_decal++;	
		}
	}

}

void Save_Decals()
{
	DECAL_SAVE_BLOCK_HEADER* block;
	int i;

	if(!NumActiveDecals) return;

	//get memory for header
	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_Decals;
	block->header.size = sizeof(*block) + NumActiveDecals * sizeof(DECAL);

	block->NumActiveDecals = NumActiveDecals;


	//now save the decals
	for(i=0;i<NumActiveDecals;i++)
	{
		DECAL* decal = GET_SAVE_BLOCK_POINTER(decal);
		*decal = DecalStorage[i];
	}
	
}
