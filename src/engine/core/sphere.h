#ifndef _included_sphere_h_ /* Is this your first time? */
#define _included_sphere_h_ 1


typedef struct
{
	int v[3];
} TRI_FACE;

#define SPHERE_ORDER 6
#define SPHERE_RADIUS ONE_FIXED	
#define SPHERE_FACES (8*SPHERE_ORDER*SPHERE_ORDER)
#define SPHERE_VERTICES (4*SPHERE_ORDER*SPHERE_ORDER+2)
#define SPHERE_TEXTURE_WRAP 4
extern VECTORCH SphereVertex[];
extern VECTORCH SphereRotatedVertex[];
extern VECTORCH SphereAtmosRotatedVertex[];
extern int SphereAtmosU[];
extern int SphereAtmosV[];
extern TRI_FACE SphereFace[];
extern int SphereVertexHeight[];

typedef struct
{
	VECTORCH Position[SPHERE_VERTICES];
	VECTORCH Velocity[SPHERE_VERTICES];
	int	RipplePhase[SPHERE_VERTICES];
	int BeenStopped[SPHERE_VERTICES];

	int ExplosionPhase;
	int NumberVerticesMoving;
	int LifeTime;
	int UseCollisions;

} VOLUMETRIC_EXPLOSION;

extern void Generate_Sphere(void);


#endif
