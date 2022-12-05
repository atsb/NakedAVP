#ifndef _included_d3d_render_h_ /* Is this your first time? */
#define _included_d3d_render_h_ 1
#include "particle.h"

extern void D3D_SetupSceneDefaults(void);
extern void D3D_DrawBackdrop(void);
extern void PostLandscapeRendering(void);
extern void D3D_DrawWaterTest(MODULE *testModulePtr);


extern void D3D_ZBufferedCloakedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_ZBufferedGouraudTexturedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_ZBufferedGouraudPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_ZBufferedTexturedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_PredatorThermalVisionPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);

extern void D3D_BackdropPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_Decal_Output(DECAL *decalPtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_Particle_Output(PARTICLE *particlePtr,RENDERVERTEX *renderVerticesPtr);
extern void D3D_DrawParticle_Rain(PARTICLE *particlePtr,VECTORCH *prevPositionPtr);

extern void D3D_DecalSystem_Setup(void);
extern void D3D_DecalSystem_End(void);

extern void D3D_FadeDownScreen(int brightness, int colour);
extern void D3D_PlayerOnFireOverlay(void);


extern void CheckWireFrameMode(int shouldBeOn);

extern void InitForceField(void);
#endif
