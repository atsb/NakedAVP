#ifndef HUD_H
#define HUD_H

#include "stratdef.h"  /* for STRATEGYBLOCK */

void InitHUD(void);
void KillHUD(void);
void ReInitHUD(void);
void MaintainHUD(void);
void DoCompletedLevelStatisticsScreen(void);
int ObjectShouldAppearOnMotionTracker(STRATEGYBLOCK *sbPtr);
void RotateVertex(VECTOR2D *vertexPtr, int theta);
void MaintainZoomingLevel(void);

#endif
