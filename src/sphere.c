#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "sphere.h"

#define UseLocalAssert Yes
#include "ourasert.h"


#define MakeVertex(o,x,y,z)	\
{ \
	o->vx=(x); \
	o->vy=(y); \
	o->vz=(z); \
	o++; \
}

#define MakeFace(f,a,b,c) \
{ \
	f->v[0] = (a); \
	f->v[1] = (b); \
	f->v[2] = (c); \
	f++; \
}

VECTORCH OctantVertex[(SPHERE_ORDER+1)*(SPHERE_ORDER+2)/2];
VECTORCH SphereVertex[SPHERE_VERTICES];

VECTORCH SphereRotatedVertex[SPHERE_VERTICES];
VECTORCH SphereAtmosRotatedVertex[SPHERE_VERTICES];
int SphereAtmosU[SPHERE_VERTICES];
int SphereAtmosV[SPHERE_VERTICES];


TRI_FACE SphereFace[SPHERE_FACES];
int SphereVertexHeight[SPHERE_VERTICES];

static void Generate_SphereOctant(void)
{
	int i,j;

	VECTORCH *o = OctantVertex;

	/* i=0, j=0 */
	MakeVertex(o,0,0,SPHERE_RADIUS);

	for (i=1; i<SPHERE_ORDER; i++)
	{
		int cosPhi, sinPhi;
		{
			int phi = 1024*i/SPHERE_ORDER;
			cosPhi = GetCos(phi);
			sinPhi = GetSin(phi);
		}

		/* 0<i<n, j=0 */
		/* => cosTheta = 1, sinTheta = 0 */
		MakeVertex(o,sinPhi,0,cosPhi);

		for (j=1; j<i; j++)
		{
			int cosTheta, sinTheta;
			{
				int theta = 1024*j/i;
				cosTheta = GetCos(theta);
				sinTheta = GetSin(theta);
			}

			/* 0<i<n, 0<j<i */
			MakeVertex(o,MUL_FIXED(cosTheta,sinPhi),MUL_FIXED(sinTheta,sinPhi),cosPhi);
		}

		/* 0<i<n, j=i */
		MakeVertex(o,0,sinPhi,cosPhi);
	}

	/* i=n, j=0 */
	MakeVertex(o,SPHERE_RADIUS,0,0);
			
	for (j=1; j<SPHERE_ORDER; j++)
	{
		int cosTheta, sinTheta;
		{
			int theta = 1024*j/SPHERE_ORDER;
			cosTheta = GetCos(theta);
			sinTheta = GetSin(theta);
		}
		/* i=n, 0<j<i */
		MakeVertex(o,cosTheta,sinTheta,0);
	}

	/* i=n, j=i */
	MakeVertex(o,0,SPHERE_RADIUS,0);
}

void Generate_Sphere(void)
{
	/* first generate vertices */
	{
		int i,j;
		VECTORCH *v = SphereVertex;
		VECTORCH *o = OctantVertex;
				
		Generate_SphereOctant();

		/* north pole */
		*v++ = *o;
		for (i=0; ++i<=SPHERE_ORDER;)
		{
			o += i;
			/* 1st Quadrant */
			for (j=i; --j>=0; o++, v++)
			{
				*v = *o;
			}
			/* 2nd Quadrant */
			for (j=i; --j>=0; o--, v++)
			{
				v->vx = -o->vx;
				v->vy = o->vy;
				v->vz = o->vz;
			}
			/* 3rd Quadrant */
			for (j=i; --j>=0; o++, v++)
			{
				v->vx = -o->vx;
				v->vy = -o->vy;
				v->vz = o->vz;
			}
			/* 4th Quadrant */
			for (j=i; --j>=0; o--, v++)
			{
				v->vx = o->vx;
				v->vy = -o->vy;
				v->vz = o->vz;
			}
		}
		for (; --i>1;)
		{
			o -= i;
			/* 5th Quadrant */
			for (j=i; --j>0; o++, v++)
			{
				v->vx = o->vx;
				v->vy = o->vy;
				v->vz = -o->vz;
			}
			/* 6th Quadrant */
			for (j=i; --j>0; o--, v++)
			{
				v->vx = -o->vx;
				v->vy = o->vy;
				v->vz = -o->vz;
			}
			/* 7th Quadrant */
			for (j=i; --j>0; o++, v++)
			{
				v->vx = -o->vx;
				v->vy = -o->vy;
				v->vz = -o->vz;
			}
			/* 8th Quadrant */
			for (j=i; --j>0; o--, v++)
			{
				v->vx = o->vx;
				v->vy = -o->vy;
				v->vz = -o->vz;
			}
		}
		o--;
		/* south pole */
		v->vx = -o->vx;
		v->vy = -o->vy;
		v->vz = -o->vz;
	}
	
	/* now generate face data */
	{
		TRI_FACE *f = SphereFace;
		int kv,kw,ko,kv0,kw0,i,j;

		kv = 0, kw = 1;

		for(i=0; i<SPHERE_ORDER; i++)
		{
			kv0 = kv, kw0 = kw;
			for (ko=1; ko<=3; ko++)
			{
				for (j=i;; j--)
				{
					MakeFace(f,kv,kw,++kw);
					if (j==0) break;
					MakeFace(f,kv,kw,++kv);
				}
			}
			for (j=i;;j--)
			{
				if (j==0)
				{
					MakeFace(f,kv0,kw,kw0);
					kv++;
					kw++;
					break;
				}
				MakeFace(f,kv,kw,++kw);
				if (j==1)
				{
					MakeFace(f,kv,kw,kv0);
				}
				else MakeFace(f,kv,kw,++kv);
			}
		}
		for(; --i>=0;)
		{
			kv0=kv,kw0=kw;
			for(ko=5;ko<=7;ko++)
			{
				for (j=i;; j--)
				{
					MakeFace(f,kv,kw,++kv);
					if (j==0) break;	 
					MakeFace(f,kv,kw,++kw);
				}
			}
			for (j=i;;j--)
			{
				if (j==0)
				{
					MakeFace(f,kv,kw0,kv0);
					kv++;
					kw++;
					break;
				}
				MakeFace(f,kv,kw,++kv);
				if (j==1)
				{
					MakeFace(f,kv,kw,kw0);
				}
				else MakeFace(f,kv,kw,++kw);
			}
		}
	}
	{
		int i;
		VECTORCH *vSphere = SphereVertex;
		for(i=0;i<SPHERE_VERTICES;i++,vSphere++)
		{
//			int radius = vSphere->vx*vSphere->vx+vSphere->vz*vSphere->vz;
//			if (radius<16384) radius = 16384;

//			SphereAtmosU[i] = DIV_FIXED(ArcCos(vSphere->vy)*32*128*8,radius);
			SphereAtmosV[i] = ArcCos(vSphere->vy)*32*128*SPHERE_TEXTURE_WRAP;//*8;
			SphereAtmosU[i] = ArcTan(vSphere->vz,vSphere->vx)*16*128*SPHERE_TEXTURE_WRAP;//*8;
		}
	}
}
