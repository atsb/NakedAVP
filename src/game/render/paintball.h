/* KJL 16:20:56 30/09/98 - Paintball.h */
#ifndef _included_paintball_h_ /* Is this your first time? */
#define _included_paintball_h_ 1

#include "decal.h"

typedef struct
{
	DISPLAYBLOCK	*TargetDispPtr; 
	VECTORCH		TargetPosition;
	VECTORCH		TargetNormal;

	enum DECAL_ID	CurrentDecalID;
	int				CurrentDecalSubclass;
	int				CurrentDecalSize;
	int 			CurrentDecalRotation;

	unsigned int  	IsOn :1;
	unsigned int	DecalIsInverted :1;
	
} PAINTBALLMODE;

extern PAINTBALLMODE PaintBallMode;

#define FIRST_PAINTBALL_DECAL (DECAL_SCORCHED)
#define LAST_PAINTBALL_DECAL (DECAL_HUMAN_BLOOD)

extern void TogglePaintBallMode(void);
extern void PaintBallMode_DrawCurrentDecalAtTarget(void);
extern void PaintBallMode_ChangeSelectedDecalID(int delta);
extern void PaintBallMode_ChangeSize(int delta);
extern void PaintBallMode_AddDecal(void);
extern void PaintBallMode_ChangeSubclass(int delta);
extern void PaintBallMode_Randomise(void);
extern void PaintBallMode_RemoveDecal(void);
extern void PaintBallMode_Rotate(void);

#endif
