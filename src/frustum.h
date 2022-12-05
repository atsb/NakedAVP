#ifndef _frustrum_h_ /* Is this your first time? */
#define _frustrum_h_ 1

#include "kshape.h"
/*
 * KJL 15:13:43 7/17/97 - frustrum.h
 *
 * function prototypes & pointers for things connected
 * to the view frustrum and clipping
 *
 */

enum FrustrumType 
{
	FRUSTRUM_TYPE_NORMAL,
	FRUSTRUM_TYPE_WIDE
};

extern void SetFrustrumType(enum FrustrumType frustrumType);

/* GOURAUD POLYGON CLIPPING */
extern void GouraudPolygon_ClipWithZ(void);
extern void (*GouraudPolygon_ClipWithNegativeX)(void);
extern void (*GouraudPolygon_ClipWithPositiveY)(void);
extern void (*GouraudPolygon_ClipWithNegativeY)(void);
extern void (*GouraudPolygon_ClipWithPositiveX)(void);

/* TEXTURED POLYGON CLIPPING */
extern void TexturedPolygon_ClipWithZ(void);
extern void (*TexturedPolygon_ClipWithNegativeX)(void);
extern void (*TexturedPolygon_ClipWithPositiveY)(void);
extern void (*TexturedPolygon_ClipWithNegativeY)(void);
extern void (*TexturedPolygon_ClipWithPositiveX)(void);

/* GOURAUD TEXTURED POLYGON CLIPPING */
extern void GouraudTexturedPolygon_ClipWithZ(void);
extern void (*GouraudTexturedPolygon_ClipWithNegativeX)(void);
extern void (*GouraudTexturedPolygon_ClipWithPositiveY)(void);
extern void (*GouraudTexturedPolygon_ClipWithNegativeY)(void);
extern void (*GouraudTexturedPolygon_ClipWithPositiveX)(void);

/* FRUSTRUM TESTS */
extern int PolygonWithinFrustrum(POLYHEADER *polyPtr);
extern int PolygonShouldBeDrawn(POLYHEADER *polyPtr);
extern int (*ObjectWithinFrustrum)(DISPLAYBLOCK *dbPtr);
extern int (*ObjectCompletelyWithinFrustrum)(DISPLAYBLOCK *dbPtr);
extern int (*VertexWithinFrustrum)(RENDERVERTEX *vertexPtr);
extern void (*TestVerticesWithFrustrum)(void);

extern int DecalWithinFrustrum(DECAL *decalPtr);
extern int QuadWithinFrustrum(void);
extern int TriangleWithinFrustrum(void);


/* pass a pointer to a vertex to be tested; results are returned in an int,
using the following defines */
#define INSIDE_FRUSTRUM_Z_PLANE		1
#define INSIDE_FRUSTRUM_PX_PLANE	2	
#define INSIDE_FRUSTRUM_NX_PLANE	4	
#define INSIDE_FRUSTRUM_PY_PLANE	8	
#define INSIDE_FRUSTRUM_NY_PLANE	16	
#define INSIDE_FRUSTRUM				31

extern char FrustrumFlagForVertex[maxrotpts];

#define USE_FOV_53 0

#endif
