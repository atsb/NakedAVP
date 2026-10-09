/* KJL 16:20:30 30/09/98 - Paintball.c */
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "kshape.h"

#include "paintball.h"
#include "showcmds.h"
#define DECAL_Z_OFFSET 5

PAINTBALLMODE PaintBallMode;

extern int NormalFrameTime;

extern void TogglePaintBallMode(void)
{
	PaintBallMode.IsOn = ~PaintBallMode.IsOn;

	if (PaintBallMode.IsOn)
	{
		extern DECAL_DESC DecalDescription[];

		PaintBallMode.CurrentDecalID = FIRST_PAINTBALL_DECAL;
		PaintBallMode.CurrentDecalSubclass = 0;
		PaintBallMode.CurrentDecalSize = DecalDescription[FIRST_PAINTBALL_DECAL].MinSize;
		PaintBallMode.CurrentDecalRotation = 0;
		PaintBallMode.DecalIsInverted = 0;
	}
}

DECAL CurrentDecal;

extern void PaintBallMode_DrawCurrentDecalAtTarget(void)
{
	extern DECAL_DESC DecalDescription[];
	DECAL_DESC *decalDescPtr = &DecalDescription[PaintBallMode.CurrentDecalID];
	extern void MakeMatrixFromDirection(VECTORCH *directionPtr, MATRIXCH *matrixPtr);
	extern MODULE *playerPherModule;
	
	MATRIXCH orientation;
	int sin = MUL_FIXED(PaintBallMode.CurrentDecalSize,GetSin(PaintBallMode.CurrentDecalRotation));
	int cos = MUL_FIXED(PaintBallMode.CurrentDecalSize,GetCos(PaintBallMode.CurrentDecalRotation));
	int z = DECAL_Z_OFFSET;
	if (!PaintBallMode.TargetDispPtr) return;

	if (! (PaintBallMode.TargetDispPtr->ObMyModule&&(!PaintBallMode.TargetDispPtr->ObMorphCtrl)) )
	{
		return;
	}
	
	CurrentDecal.DecalID = PaintBallMode.CurrentDecalID;

	if(PaintBallMode.DecalIsInverted)
	{	
		PaintBallMode.TargetNormal.vx = -PaintBallMode.TargetNormal.vx;
		PaintBallMode.TargetNormal.vy = -PaintBallMode.TargetNormal.vy;
		PaintBallMode.TargetNormal.vz = -PaintBallMode.TargetNormal.vz;
		z = -z;
	}

	MakeMatrixFromDirection(&PaintBallMode.TargetNormal,&orientation);
	
	CurrentDecal.Vertices[0].vx = (-cos) - (-sin);
	CurrentDecal.Vertices[0].vy = (-sin) + (-cos);
	CurrentDecal.Vertices[0].vz = z;
	RotateVector(&(CurrentDecal.Vertices[0]),&orientation);
	CurrentDecal.Vertices[0].vx += PaintBallMode.TargetPosition.vx;
	CurrentDecal.Vertices[0].vy += PaintBallMode.TargetPosition.vy;
	CurrentDecal.Vertices[0].vz += PaintBallMode.TargetPosition.vz;


	CurrentDecal.Vertices[1].vx = (cos) - (-sin);
	CurrentDecal.Vertices[1].vy = (sin) + (-cos);
	CurrentDecal.Vertices[1].vz = z;
	RotateVector(&(CurrentDecal.Vertices[1]),&orientation);
	CurrentDecal.Vertices[1].vx += PaintBallMode.TargetPosition.vx;
	CurrentDecal.Vertices[1].vy += PaintBallMode.TargetPosition.vy;
	CurrentDecal.Vertices[1].vz += PaintBallMode.TargetPosition.vz;

	CurrentDecal.Vertices[2].vx = (cos) - (sin);
	CurrentDecal.Vertices[2].vy = (sin) + (cos);
	CurrentDecal.Vertices[2].vz = z;
	RotateVector(&(CurrentDecal.Vertices[2]),&orientation);
	CurrentDecal.Vertices[2].vx += PaintBallMode.TargetPosition.vx;
	CurrentDecal.Vertices[2].vy += PaintBallMode.TargetPosition.vy;
	CurrentDecal.Vertices[2].vz += PaintBallMode.TargetPosition.vz;

	CurrentDecal.Vertices[3].vx = (-cos) - (sin);
	CurrentDecal.Vertices[3].vy = (-sin) + (cos);
	CurrentDecal.Vertices[3].vz = z;
	RotateVector(&(CurrentDecal.Vertices[3]),&orientation);
	CurrentDecal.Vertices[3].vx += PaintBallMode.TargetPosition.vx;
	CurrentDecal.Vertices[3].vy += PaintBallMode.TargetPosition.vy;
	CurrentDecal.Vertices[3].vz += PaintBallMode.TargetPosition.vz;

	CurrentDecal.ModuleIndex = playerPherModule->m_index;

	CurrentDecal.UOffset = PaintBallMode.CurrentDecalSubclass*decalDescPtr->UOffsetForSubclass;
	RenderDecal(&CurrentDecal);

	{
		PrintDebuggingText("PAINTBALL MODE ACTIVE\n");
		PrintDebuggingText("TOTAL PRE-DECALS: %d OUT OF 1024\n",NumFixedDecals);
	}
	
}

extern void PaintBallMode_ChangeSelectedDecalID(int delta)
{
	extern DECAL_DESC DecalDescription[];

	PaintBallMode.CurrentDecalID += delta;
	
	if (PaintBallMode.CurrentDecalID>LAST_PAINTBALL_DECAL)
	{
		PaintBallMode.CurrentDecalID=FIRST_PAINTBALL_DECAL;
	}
	else if (PaintBallMode.CurrentDecalID<FIRST_PAINTBALL_DECAL)
	{
		PaintBallMode.CurrentDecalID=LAST_PAINTBALL_DECAL;
	}
	
	PaintBallMode.CurrentDecalSubclass = 0;
	PaintBallMode.CurrentDecalSize = DecalDescription[PaintBallMode.CurrentDecalID].MaxSize;

}

extern void PaintBallMode_ChangeSize(int delta)
{
	extern DECAL_DESC DecalDescription[];
	DECAL_DESC *decalDescPtr = &DecalDescription[PaintBallMode.CurrentDecalID];

	if (delta>0)
	{
		PaintBallMode.CurrentDecalSize+=NormalFrameTime>>6;
		if (PaintBallMode.CurrentDecalSize>decalDescPtr->MaxSize*2)
			PaintBallMode.CurrentDecalSize=decalDescPtr->MaxSize*2;
	}
	else
	{
		PaintBallMode.CurrentDecalSize-=NormalFrameTime>>6;
		if (PaintBallMode.CurrentDecalSize<decalDescPtr->MinSize)
			PaintBallMode.CurrentDecalSize=decalDescPtr->MinSize;
	}
}

extern void PaintBallMode_Rotate(void)
{
	PaintBallMode.CurrentDecalRotation += NormalFrameTime>>5;
	PaintBallMode.CurrentDecalRotation &= 4095;
}

extern void PaintBallMode_ChangeSubclass(int delta)
{
	extern DECAL_DESC DecalDescription[];
	DECAL_DESC *decalDescPtr = &DecalDescription[PaintBallMode.CurrentDecalID];

	PaintBallMode.CurrentDecalSubclass += delta;
	
	if (PaintBallMode.CurrentDecalSubclass>decalDescPtr->MaxSubclassNumber)
	{
		PaintBallMode.CurrentDecalSubclass=0;
	}
	else if (PaintBallMode.CurrentDecalSubclass<0)
	{
		PaintBallMode.CurrentDecalSubclass=decalDescPtr->MaxSubclassNumber;
	}

}


extern void PaintBallMode_AddDecal(void)
{
	FIXED_DECAL *newDecalPtr = AllocateFixedDecal();

	if (!newDecalPtr) return;

	newDecalPtr->DecalID = CurrentDecal.DecalID;
	newDecalPtr->Vertices[0] = CurrentDecal.Vertices[0];
	newDecalPtr->Vertices[1] = CurrentDecal.Vertices[1];
	newDecalPtr->Vertices[2] = CurrentDecal.Vertices[2];
	newDecalPtr->Vertices[3] = CurrentDecal.Vertices[3];
	newDecalPtr->ModuleIndex = CurrentDecal.ModuleIndex;
	newDecalPtr->UOffset = CurrentDecal.UOffset;
}
extern void PaintBallMode_RemoveDecal(void)
{
	RemoveFixedDecal();
}

extern void PaintBallMode_Randomise(void)
{
	extern DECAL_DESC DecalDescription[];
	DECAL_DESC *decalDescPtr = &DecalDescription[PaintBallMode.CurrentDecalID];

	int randomScale = ONE_FIXED+4096 - (FastRandom()&8191);
	
	PaintBallMode.CurrentDecalSize = MUL_FIXED(PaintBallMode.CurrentDecalSize,randomScale);
											   
	if (PaintBallMode.CurrentDecalSize>decalDescPtr->MaxSize)
	{
		PaintBallMode.CurrentDecalSize=decalDescPtr->MaxSize;
	}
	if (PaintBallMode.CurrentDecalSize<decalDescPtr->MinSize)
	{
	 	PaintBallMode.CurrentDecalSize=decalDescPtr->MinSize;
	}

	PaintBallMode.CurrentDecalRotation = FastRandom()&4095;
	
}
