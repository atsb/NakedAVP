#include "3dc.h"
#include "inline.h"
#include "maths.h"

#define UseTimsPinp Yes

/*

 externs for commonly used global variables and arrays

*/

extern const short ArcCosTable[4096];
extern const short ArcSineTable[4096];
extern const short ArcTanTable[256];

extern LONGLONGCH ll_zero;


/*

 Globals

*/

	MATRIXCH IdentityMatrix = {

		ONE_FIXED, 0, 0,
		0, ONE_FIXED, 0,
		0, 0, ONE_FIXED

	};



/*

 Maths functions used by the system

*/



/* One over sin functions - CDF 4/2/98 */

extern int oneoversin[4096];

void ConstructOneOverSinTable(void) {

	int a,sin;

	for (a=0; a<4096; a++) {
		sin=GetSin(a);

		if (sin!=0) {
			oneoversin[a]=DIV_FIXED(ONE_FIXED,sin);
		} else {
			sin=100;
			oneoversin[a]=DIV_FIXED(ONE_FIXED,sin);
		}
	}

}

int GetOneOverSin(int a) {

	int b;

	b=a&wrap360;
	
	return(oneoversin[b]);

}

/*

 Dot Product Function

 It accepts two pointers to vectors and returns an int result

*/

int _DotProduct(VECTORCH *vptr1, VECTORCH *vptr2)

{

	int dp;

	dp =  MUL_FIXED(vptr1->vx, vptr2->vx);
	dp += MUL_FIXED(vptr1->vy, vptr2->vy);
	dp += MUL_FIXED(vptr1->vz, vptr2->vz);

	return(dp);

}


int DotProduct2d(VECTOR2D *vptr1, VECTOR2D *vptr2)

{

	int dp;


	dp  = MUL_FIXED(vptr1->vx, vptr2->vx);
	dp += MUL_FIXED(vptr1->vy, vptr2->vy);

	return dp;

}


/*

 This function returns the distance between two vectors

*/

int VectorDistance(VECTORCH *v1, VECTORCH *v2)

{

	VECTORCH v;


	v.vx = v1->vx - v2->vx;
	v.vy = v1->vy - v2->vy;
	v.vz = v1->vz - v2->vz;

	return Magnitude(&v);

}


/*

 This function compares the distance between two vectors along each of
 the major axes and returns Yes or No if they are within the cube defined
 by the argument passed.

*/

int OutcodeVectorDistance(VECTORCH *v1, VECTORCH *v2, int d)

{

	int i;


	i = v1->vx - v2->vx;
	if(i < 0) i = -i;

	if(i >= d) return No;

	i = v1->vy - v2->vy;
	if(i < 0) i = -i;

	if(i >= d) return No;

	i = v1->vz - v2->vz;
	if(i < 0) i = -i;

	if(i >= d) return No;

	return Yes;

}


/*

 Subtract one VECTORCH from another and return the result as a normal

 v3 = Normal(v1 - v2)

*/

void GetNormalVector(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3)

{

	v3->vx = v1->vx - v2->vx;
	v3->vy = v1->vy - v2->vy;
	v3->vz = v1->vz - v2->vz;

	Normalise(v3);

}


/*

 Normalise a vector close to, but less than, unit length

*/

void Renormalise(VECTORCH *nvector)

{

	int m;
	int xsq, ysq, zsq;


/* Scale x, y and z */

	nvector->vx >>= 2;
	nvector->vy >>= 2;
	nvector->vz >>= 2;


/* Normalise */

	xsq = nvector->vx * nvector->vx;
	ysq = nvector->vy * nvector->vy;
	zsq = nvector->vz * nvector->vz;

	m = SqRoot32(xsq + ysq + zsq);

	if(m == 0) m = 1;			/* Just in case */

	nvector->vx = (nvector->vx * ONE_FIXED) / m;
	nvector->vy = (nvector->vy * ONE_FIXED) / m;
	nvector->vz = (nvector->vz * ONE_FIXED) / m;

}









/*

 Return the shift value required to get one value LTE the other value

*/

int FindShift32(int value, int limit)

{

	int shift = 0;


	/*if(limit == 0) exit(0xfa11fa11);*/


	if(value < 0) value = -value;

	while(value > limit) {

		shift++;

		value >>= 1;

	}

	return shift;

}


/*

 Return the largest value of an int array

*/

int MaxInt(int *iarray, int iarraysize)

{

	int imax = smallint;
	int i;

	for(i = iarraysize; i!=0; i--) {

		if(imax < *iarray) imax = *iarray;

		iarray++;

	}

	return imax;

}


/*

 Return the smallest value of an int array

*/

int MinInt(int *iarray, int iarraysize)

{

	int imin = bigint;
	int i;

	for(i = iarraysize; i!=0; i--) {

		if(imin > *iarray) imin = *iarray;

		iarray++;

	}

	return imin;

}



/*

 Create Matrix from Euler Angles

 It requires a pointer to some euler angles and a pointer to a matrix

 Construct the matrix elements using the following formula

 Formula for ZXY Matrix

 m11 = cy*cz + sx*sy*sz    m12 = -cy*sz + sx*sy*cz   m13 = cx*sy
 m21 = cx*sz               m22 = cx*cz               m23 = -sx
 m31 = -sy*cz + sx*cy*sz   m32 = sy*sz + sx*cy*cz    m33 = cx*cy

*/

void CreateEulerMatrix(EULER *e, MATRIXCH *m1)
{
	int t, sx, sy, sz, cx, cy, cz;


	sx = GetSin(e->EulerX);
	sy = GetSin(e->EulerY);
	sz = GetSin(e->EulerZ);

	cx = GetCos(e->EulerX);
	cy = GetCos(e->EulerY);
	cz = GetCos(e->EulerZ);


	#if 0
	textprint("Euler Matrix Sines & Cosines\n");
	textprint("%d, %d, %d\n", sx, sy, sz);
	textprint("%d, %d, %d\n", cx, cy, cz);
	#endif


/* m11 = cy*cz + sx*sy*sz */

	m1->mat11 = MUL_FIXED(cy, cz);		/* cy*cz	*/
	t = MUL_FIXED(sx, sy);					/* sx*sy */
	t = MUL_FIXED(t, sz);					/* *sz	*/
	m1->mat11 += t;


/* m12 = -cy*sz + sx*sy*cz */

	m1->mat12=MUL_FIXED(-cy,sz);
	t=MUL_FIXED(sx,sy);
	t=MUL_FIXED(t,cz);
	m1->mat12+=t;


/* m13 = cx*sy */

	m1->mat13=MUL_FIXED(cx,sy);


/* m21 = cx*sz */

	m1->mat21=MUL_FIXED(cx,sz);


/* m22 = cx*cz */

	m1->mat22=MUL_FIXED(cx,cz);


/* m23 = -sx */

	m1->mat23=-sx;


/* m31 = -sy*cz + sx*cy*sz */

	m1->mat31=MUL_FIXED(-sy,cz);
	t=MUL_FIXED(sx,cy);
	t=MUL_FIXED(t,sz);
	m1->mat31+=t;


/* m32 = sy*sz + sx*cy*cz */

	m1->mat32=MUL_FIXED(sy,sz);
	t=MUL_FIXED(sx,cy);
	t=MUL_FIXED(t,cz);
	m1->mat32+=t;


/* m33 = cx*cy */

	m1->mat33=MUL_FIXED(cx,cy);
}


/*

 Create a Unit Vector from three Euler Angles

*/

void CreateEulerVector(EULER *e, VECTORCH *v)

{

	int t, sx, sy, sz, cx, cy, cz;


	sx = GetSin(e->EulerX);
	sy = GetSin(e->EulerY);
	sz = GetSin(e->EulerZ);

	cx = GetCos(e->EulerX);
	cy = GetCos(e->EulerY);
	cz = GetCos(e->EulerZ);


	/* x = -sy*cz + sx*cy*sz */

	v->vx  = MUL_FIXED(-sy, cz);
	t      = MUL_FIXED(sx, cy);
	t      = MUL_FIXED(t, sz);
	v->vx += t;


	/* y = sy*sz + sx*cy*cz */

	v->vy  = MUL_FIXED(sy, sz);
	t      = MUL_FIXED(sx, cy);
	t      = MUL_FIXED(t, cz);
	v->vy += t;


	/* z = cx*cy */

	v->vz = MUL_FIXED(cx,cy);

}



/*

 Matrix Multiply Function

 A 3x3 Matrix is represented here as

 m11 m12 m13
 m21 m22 m23
 m31 m32 m33

 Row #1 (r1) of the matrix is m11 m12 m13
 Column #1 (c1) of the matrix is m11 m32 m31

 Under multiplication

 m'' = m x m'

 where

 m11'' = c1.r1'
 m12'' = c2.r1'
 m13'' = c3.r1'

 m21'' = c1.r2'
 m22'' = c2.r2'
 m23'' = c3.r2'

 m31'' = c1.r3'
 m32'' = c2.r3'
 m33'' = c3.r3'

*/

void MatrixMultiply(struct matrixch *m1, struct matrixch *m2, struct matrixch *m3)

{
	MATRIXCH TmpMat;
	 
/* m11'' = c1.r1' */

	TmpMat.mat11=MUL_FIXED(m1->mat11,m2->mat11);
	TmpMat.mat11+=MUL_FIXED(m1->mat21,m2->mat12);
	TmpMat.mat11+=MUL_FIXED(m1->mat31,m2->mat13);

/* m12'' = c2.r1' */

	TmpMat.mat12=MUL_FIXED(m1->mat12,m2->mat11);
	TmpMat.mat12+=MUL_FIXED(m1->mat22,m2->mat12);
	TmpMat.mat12+=MUL_FIXED(m1->mat32,m2->mat13);

/* m13'' = c3.r1' */

	TmpMat.mat13=MUL_FIXED(m1->mat13,m2->mat11);
	TmpMat.mat13+=MUL_FIXED(m1->mat23,m2->mat12);
	TmpMat.mat13+=MUL_FIXED(m1->mat33,m2->mat13);

/* m21'' = c1.r2' */

	TmpMat.mat21=MUL_FIXED(m1->mat11,m2->mat21);
	TmpMat.mat21+=MUL_FIXED(m1->mat21,m2->mat22);
	TmpMat.mat21+=MUL_FIXED(m1->mat31,m2->mat23);

/* m22'' = c2.r2' */

	TmpMat.mat22=MUL_FIXED(m1->mat12,m2->mat21);
	TmpMat.mat22+=MUL_FIXED(m1->mat22,m2->mat22);
	TmpMat.mat22+=MUL_FIXED(m1->mat32,m2->mat23);

/* m23'' = c3.r2' */

	TmpMat.mat23=MUL_FIXED(m1->mat13,m2->mat21);
	TmpMat.mat23+=MUL_FIXED(m1->mat23,m2->mat22);
	TmpMat.mat23+=MUL_FIXED(m1->mat33,m2->mat23);

/* m31'' = c1.r3' */

	TmpMat.mat31=MUL_FIXED(m1->mat11,m2->mat31);
	TmpMat.mat31+=MUL_FIXED(m1->mat21,m2->mat32);
	TmpMat.mat31+=MUL_FIXED(m1->mat31,m2->mat33);

/* m32'' = c2.r3' */

	TmpMat.mat32=MUL_FIXED(m1->mat12,m2->mat31);
	TmpMat.mat32+=MUL_FIXED(m1->mat22,m2->mat32);
	TmpMat.mat32+=MUL_FIXED(m1->mat32,m2->mat33);

/* m33'' = c3.r3' */

	TmpMat.mat33=MUL_FIXED(m1->mat13,m2->mat31);
	TmpMat.mat33+=MUL_FIXED(m1->mat23,m2->mat32);
	TmpMat.mat33+=MUL_FIXED(m1->mat33,m2->mat33);

/* Finally, copy TmpMat to m3 */

	CopyMatrix(&TmpMat, m3);
}


/*

 Transpose Matrix

*/

void TransposeMatrixCH(MATRIXCH *m1)

{

	int t;

	t=m1->mat12;
	m1->mat12=m1->mat21;
	m1->mat21=t;

	t=m1->mat13;
	m1->mat13=m1->mat31;
	m1->mat31=t;

	t=m1->mat23;
	m1->mat23=m1->mat32;
	m1->mat32=t;

}


/*

 Copy Vector

*/

void CopyVector(VECTORCH *v1, VECTORCH *v2)

{

/* Copy VECTORCH v1 -> VECTORCH v2 */

	v2->vx=v1->vx;
	v2->vy=v1->vy;
	v2->vz=v1->vz;

}


/*

 Copy Location

*/

void CopyLocation(VECTORCH *v1, VECTORCH *v2)

{

/* Copy VECTORCH v1 -> VECTORCH v2 */

	v2->vx=v1->vx;
	v2->vy=v1->vy;
	v2->vz=v1->vz;

}





/*

 Copy Euler

*/

void CopyEuler(EULER *e1, EULER *e2)

{

/* Copy EULER e1 -> EULER e2 */

	e2->EulerX=e1->EulerX;
	e2->EulerY=e1->EulerY;
	e2->EulerZ=e1->EulerZ;

}


/*

 Copy Matrix

*/

void CopyMatrix(MATRIXCH *m1, MATRIXCH *m2)

{

/* Copy MATRIXCH m1 -> MATRIXCH m2 */

	m2->mat11=m1->mat11;
	m2->mat12=m1->mat12;
	m2->mat13=m1->mat13;

	m2->mat21=m1->mat21;
	m2->mat22=m1->mat22;
	m2->mat23=m1->mat23;

	m2->mat31=m1->mat31;
	m2->mat32=m1->mat32;
	m2->mat33=m1->mat33;

}


/*

 Make a Vector.

 v3 = v1 - v2

*/

void MakeVector(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3)

{

	v3->vx = v1->vx - v2->vx;
	v3->vy = v1->vy - v2->vy;
	v3->vz = v1->vz - v2->vz;

}


/*

 Add a Vector.

 v2 = v2 + v1

*/

void AddVector(VECTORCH *v1, VECTORCH *v2)

{

	v2->vx += v1->vx;
	v2->vy += v1->vy;
	v2->vz += v1->vz;

}


/*

 Subtract a Vector.

 v2 = v2 - v1

*/

void SubVector(VECTORCH *v1, VECTORCH *v2)

{

	v2->vx -= v1->vx;
	v2->vy -= v1->vy;
	v2->vz -= v1->vz;

}



/*

 Matrix Rotatation of a Vector

 Overwrite the Source Vector with the Rotated Vector

 x' = v.c1
 y' = v.c2
 z' = v.c3

*/

void _RotateVector(VECTORCH *v, MATRIXCH*  m)
{

	int x, y, z;
	

	x =  MUL_FIXED(m->mat11, v->vx);
	x += MUL_FIXED(m->mat21, v->vy);
	x += MUL_FIXED(m->mat31, v->vz);

	y  = MUL_FIXED(m->mat12, v->vx);
	y += MUL_FIXED(m->mat22, v->vy);
	y += MUL_FIXED(m->mat32, v->vz);

	z  = MUL_FIXED(m->mat13, v->vx);
	z += MUL_FIXED(m->mat23, v->vy);
	z += MUL_FIXED(m->mat33, v->vz);

	v->vx = x;
	v->vy = y;
	v->vz = z;
}


/*

 Matrix Rotation of a Source Vector using a Matrix
 Copying to a Destination Vector

 x' = v.c1
 y' = v.c2
 z' = v.c3

*/

void _RotateAndCopyVector(VECTORCH *v1, VECTORCH *v2, MATRIXCH *m)

{

	v2->vx=MUL_FIXED(m->mat11,v1->vx);
	v2->vx+=MUL_FIXED(m->mat21,v1->vy);
	v2->vx+=MUL_FIXED(m->mat31,v1->vz);

	v2->vy=MUL_FIXED(m->mat12,v1->vx);
	v2->vy+=MUL_FIXED(m->mat22,v1->vy);
	v2->vy+=MUL_FIXED(m->mat32,v1->vz);

	v2->vz=MUL_FIXED(m->mat13,v1->vx);
	v2->vz+=MUL_FIXED(m->mat23,v1->vy);
	v2->vz+=MUL_FIXED(m->mat33,v1->vz);

}



/*

 Matrix to Euler Angles

 Maths overflow is a real problem for this function. To prevent overflows
 the matrix Sines and Cosines are calculated using values scaled down by 4.


 sinx	=	-M23

 cosx	=	sqr ( 1 - sinx^2 )


 siny	=	M13 / cosx

 cosy	=	M33 / cosx


 sinz	=	M21 / cosx

 cosz	=	M22 / cosx


*/


#define m2e_scale 2
#define ONE_FIXED_S ((ONE_FIXED >> m2e_scale) - 1)
#define m2e_shift 14

#define j_and_r_change Yes


void MatrixToEuler(MATRIXCH *m, EULER *e)

{

	int x, sinx, cosx, siny, cosy, sinz, cosz;
	int abs_cosx, abs_cosy, abs_cosz;
	int SineMatrixPitch, SineMatrixYaw, SineMatrixRoll;
	int CosMatrixPitch, CosMatrixYaw, CosMatrixRoll;




	#if 0
	textprint("CosMatrixPitch = %d\n", CosMatrixPitch);
	/* WaitForReturn(); */
	#endif


	if(m->mat32 >-65500 && m->mat32<65500)
	{
			/* Yaw */

		/* Pitch */

		#if j_and_r_change
		SineMatrixPitch = -m->mat32;
		#else
		SineMatrixPitch = -m->mat23;
		#endif

		SineMatrixPitch >>= m2e_scale;

		#if 0
		textprint("SineMatrixPitch = %d\n", SineMatrixPitch);
		/* WaitForReturn(); */
		#endif

		CosMatrixPitch = SineMatrixPitch * SineMatrixPitch;
		CosMatrixPitch >>= m2e_shift;

		CosMatrixPitch = -CosMatrixPitch;
		CosMatrixPitch += ONE_FIXED_S;
		CosMatrixPitch *= ONE_FIXED_S;
		CosMatrixPitch = SqRoot32(CosMatrixPitch);

		if(CosMatrixPitch) {

			if(CosMatrixPitch > ONE_FIXED_S) CosMatrixPitch = ONE_FIXED_S;
			else if(CosMatrixPitch < -ONE_FIXED_S) CosMatrixPitch = -ONE_FIXED_S;

		}

		else CosMatrixPitch = 1;

		SineMatrixYaw = WideMulNarrowDiv(
			#if j_and_r_change
			m->mat31 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
			#else
			m->mat13 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
			#endif

		#if 0
		textprint("SineMatrixYaw = %d\n", SineMatrixYaw);
		/* WaitForReturn(); */
		#endif

		CosMatrixYaw = WideMulNarrowDiv(
			m->mat33 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);

		#if 0
		textprint("CosMatrixYaw = %d\n", CosMatrixYaw);
		/* WaitForReturn(); */
		#endif


		/* Roll */

		SineMatrixRoll = WideMulNarrowDiv(
			#if j_and_r_change
			m->mat12 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
			#else
			m->mat21 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
			#endif

		#if 0
		textprint("SineMatrixRoll = %d\n", SineMatrixRoll);
		/* WaitForReturn(); */
		#endif

		CosMatrixRoll = WideMulNarrowDiv(
			m->mat22 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);

		#if 0
		textprint("CosMatrixRoll = %d\n", CosMatrixRoll);
		/* WaitForReturn(); */
		#endif
	
		/* Tables are for values +- 2^16 */

		sinx = SineMatrixPitch << m2e_scale;
		siny = SineMatrixYaw   << m2e_scale;
		sinz = SineMatrixRoll  << m2e_scale;

		cosx = CosMatrixPitch << m2e_scale;
		cosy = CosMatrixYaw   << m2e_scale;
		cosz = CosMatrixRoll  << m2e_scale;

		#if 0
		textprint("sines = %d, %d, %d\n", sinx, siny, sinz);
		textprint("cos's = %d, %d, %d\n", cosx, cosy, cosz);
		/* WaitForReturn(); */
		#endif

		/* Absolute Cosines */

		abs_cosx = cosx;
		if(abs_cosx < 0) abs_cosx = -abs_cosx;

		abs_cosy = cosy;
		if(abs_cosy < 0) abs_cosy = -abs_cosy;

		abs_cosz = cosz;
		if(abs_cosz < 0) abs_cosz = -abs_cosz;


		/* Euler X */

		if(abs_cosx > Cosine45) {

			x = ArcSin(sinx);

			if(cosx < 0) {
				x =  -x;
				x += deg180;
				x &= wrap360;
			}
		}

		else {

			x = ArcCos(cosx);

			if(sinx < 0) {
				x =  -x;
				x &= wrap360;			
			}
		}

		#if (j_and_r_change == No)
		x = -x;
		x &= wrap360;
		#endif

		e->EulerX = x;


		/* Euler Y */

		if(abs_cosy > Cosine45) {

			x = ArcSin(siny);

			if(cosy < 0) {
				x =  -x;
				x += deg180;
				x &= wrap360;
			}

		}

		else {

			x = ArcCos(cosy);

			if(siny < 0) {
				x =  -x;
				x &= wrap360;			
			}

		}

		#if (j_and_r_change == No)
		x = -x;
		x &= wrap360;
		#endif

		e->EulerY = x;


		/* Euler Z */

		if(abs_cosz > Cosine45) {

			x = ArcSin(sinz);

			if(cosz < 0) {
				x =  -x;
				x += deg180;
				x &= wrap360;
			}
		}

		else {

			x = ArcCos(cosz);

			if(sinz < 0) {
				x =  -x;
				x &= wrap360;			
			}
		}

		#if (j_and_r_change == No)
		x =  -x;
		x &= wrap360;
		#endif

		e->EulerZ = x;
	}
	else //singularity case
	{

		if(m->mat32>0)
			e->EulerX = 3072;
		else
			e->EulerX = 1024;
		
		e->EulerZ=0;


		
		/* Yaw */
		
		siny = -m->mat13 ;

		cosy = 	m->mat11 ;

		abs_cosy = cosy;
		if(abs_cosy < 0) abs_cosy = -abs_cosy;


		if(abs_cosy > Cosine45) {

			x = ArcSin(siny);

			if(cosy < 0) {
				x =  -x;
				x += deg180;
				x &= wrap360;
			}

		}

		else {

			x = ArcCos(cosy);

			if(siny < 0) {
				x =  -x;
				x &= wrap360;			
			}

		}

		#if (j_and_r_change == No)
		x = -x;
		x &= wrap360;
		#endif

		e->EulerY = x;

	}




	#if 0
	textprint("\nEuler from VDB Matrix is:\n%d\n%d\n%d\n",
	e->EulerX,
	e->EulerY,
	e->EulerZ
	);
	/* WaitForReturn(); */
	#endif

}


#define j_and_r_change_2 Yes

void MatrixToEuler2(MATRIXCH *m, EULER *e)

{

	int x, sinx, cosx, siny, cosy, sinz, cosz;
	int abs_cosx, abs_cosy, abs_cosz;
	int SineMatrixPitch, SineMatrixYaw, SineMatrixRoll;
	int CosMatrixPitch, CosMatrixYaw, CosMatrixRoll;


	/* Pitch */

	#if j_and_r_change_2
	SineMatrixPitch = -m->mat32;
	#else
	SineMatrixPitch = -m->mat23;
	#endif

	SineMatrixPitch >>= m2e_scale;

	#if 0
	textprint("SineMatrixPitch = %d\n", SineMatrixPitch);
	/* WaitForReturn(); */
	#endif

	CosMatrixPitch = SineMatrixPitch * SineMatrixPitch;
	CosMatrixPitch >>= m2e_shift;

	CosMatrixPitch = -CosMatrixPitch;
	CosMatrixPitch += ONE_FIXED_S;
	CosMatrixPitch *= ONE_FIXED_S;
	CosMatrixPitch = SqRoot32(CosMatrixPitch);

	if(CosMatrixPitch) {

		if(CosMatrixPitch > ONE_FIXED_S) CosMatrixPitch = ONE_FIXED_S;
		else if(CosMatrixPitch < -ONE_FIXED_S) CosMatrixPitch = -ONE_FIXED_S;

	}

	else CosMatrixPitch = 1;


	#if 0
	textprint("CosMatrixPitch = %d\n", CosMatrixPitch);
	/* WaitForReturn(); */
	#endif


	/* Yaw */

	SineMatrixYaw = WideMulNarrowDiv(
		#if j_and_r_change_2
		m->mat31 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
		#else
		m->mat13 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
		#endif

	#if 0
	textprint("SineMatrixYaw = %d\n", SineMatrixYaw);
	/* WaitForReturn(); */
	#endif

	CosMatrixYaw = WideMulNarrowDiv(
		m->mat33 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);

	#if 0
	textprint("CosMatrixYaw = %d\n", CosMatrixYaw);
	/* WaitForReturn(); */
	#endif


	/* Roll */

	SineMatrixRoll = WideMulNarrowDiv(
		#if j_and_r_change_2
		m->mat12 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
		#else
		m->mat21 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);
		#endif

	#if 0
	textprint("SineMatrixRoll = %d\n", SineMatrixRoll);
	/* WaitForReturn(); */
	#endif

	CosMatrixRoll = WideMulNarrowDiv(
		m->mat22 >> m2e_scale, ONE_FIXED_S, CosMatrixPitch);

	#if 0
	textprint("CosMatrixRoll = %d\n", CosMatrixRoll);
	/* WaitForReturn(); */
	#endif


	/* Tables are for values +- 2^16 */

	sinx = SineMatrixPitch << m2e_scale;
	siny = SineMatrixYaw   << m2e_scale;
	sinz = SineMatrixRoll  << m2e_scale;

	cosx = CosMatrixPitch << m2e_scale;
	cosy = CosMatrixYaw   << m2e_scale;
	cosz = CosMatrixRoll  << m2e_scale;

	#if 0
	textprint("sines = %d, %d, %d\n", sinx, siny, sinz);
	textprint("cos's = %d, %d, %d\n", cosx, cosy, cosz);
	/* WaitForReturn(); */
	#endif

	/* Absolute Cosines */

	abs_cosx = cosx;
	if(abs_cosx < 0) abs_cosx = -abs_cosx;

	abs_cosy = cosy;
	if(abs_cosy < 0) abs_cosy = -abs_cosy;

	abs_cosz = cosz;
	if(abs_cosz < 0) abs_cosz = -abs_cosz;


	/* Euler X */

	if(abs_cosx > Cosine45) {

		x = ArcSin(sinx);

		if(cosx < 0) {
			x =  -x;
			x += deg180;
			x &= wrap360;
		}
	}

	else {

		x = ArcCos(cosx);

		if(sinx < 0) {
			x =  -x;
			x &= wrap360;			
		}
	}

	#if (j_and_r_change_2 == No)
	x = -x;
	x &= wrap360;
	#endif

	e->EulerX = x;


	/* Euler Y */

	if(abs_cosy > Cosine45) {

		x = ArcSin(siny);

		if(cosy < 0) {
			x =  -x;
			x += deg180;
			x &= wrap360;
		}

	}

	else {

		x = ArcCos(cosy);

		if(siny < 0) {
			x =  -x;
			x &= wrap360;			
		}

	}

	#if (j_and_r_change_2 == No)
	x = -x;
	x &= wrap360;
	#endif

	e->EulerY = x;


	/* Euler Z */

	if(abs_cosz > Cosine45) {

		x = ArcSin(sinz);

		if(cosz < 0) {
			x =  -x;
			x += deg180;
			x &= wrap360;
		}
	}

	else {

		x = ArcCos(cosz);

		if(sinz < 0) {
			x =  -x;
			x &= wrap360;			
		}
	}

	#if (j_and_r_change_2 == No)
	x =  -x;
	x &= wrap360;
	#endif

	e->EulerZ = x;


	#if 0
	textprint("\nEuler from VDB Matrix is:\n%d\n%d\n%d\n",
	e->EulerX,
	e->EulerY,
	e->EulerZ
	);
	/* WaitForReturn(); */
	#endif

}



/*

 Normalise a Matrix

 Dot the three vectors together (XY, XZ, YZ) and take the two nearest to
 90ø from each other. Cross them to create a new third vector, then cross
 the first and third to create a new second.

*/

void MNormalise(MATRIXCH *m)

{

	VECTORCH *x = (VECTORCH *) &m->mat11;
	VECTORCH *y = (VECTORCH *) &m->mat21;
	VECTORCH *z = (VECTORCH *) &m->mat31;
	int dotxy = Dot(x, y);
	int dotxz = Dot(x, z);
	int dotyz = Dot(y, z);
	VECTORCH *s;
	VECTORCH *t;
	VECTORCH u;
	VECTORCH v;
	VECTORCH zero = {0, 0, 0};


	#if 0
	textprint("dotxy = %d\n", dotxy);
	textprint("dotxz = %d\n", dotxz);
	textprint("dotyz = %d\n", dotyz);
	#endif

	#if 0
	/* TEST */
	dotxy = 0;
	dotxz = 0;
	dotyz = 1;
	#endif


	#if 0
	textprint("%d	%d	%d\n",
		x->vx,
		x->vy,
		x->vz
	);

	textprint("%d	%d	%d\n",
		y->vx,
		y->vy,
		y->vz
	);

	textprint("%d	%d	%d\n",
		z->vx,
		z->vy,
		z->vz
	);
	#endif


	/* Find the two vectors nearest 90ø */

	if(dotxy > dotxz && dotxy > dotyz) {

		/* xy are the closest to 90ø */

		/*textprint("xy\n");*/

		s = x;
		t = y;

		MakeNormal(&zero, s, t, &u);		/* Cross them for a new 3rd vector */

		MakeNormal(&zero, s, &u, &v);		/* Cross 1st & 3rd for a new 2nd */
		v.vx = -v.vx;
		v.vy = -v.vy;
		v.vz = -v.vz;

		CopyVector(&u, z);
		CopyVector(&v, y);

	}

	else if(dotxz > dotxy && dotxz > dotyz) {

		/* xz are the closest to 90ø */

		/*textprint("xz\n");*/

		s = x;
		t = z;

		MakeNormal(&zero, s, t, &u);		/* Cross them for a new 3rd vector */
		u.vx = -u.vx;
		u.vy = -u.vy;
		u.vz = -u.vz;

		MakeNormal(&zero, s, &u, &v);		/* Cross 1st & 3rd for a new 2nd */

		CopyVector(&u, y);
		CopyVector(&v, z);

	}

	else {

		/* yz are the closest to 90ø */

		/*textprint("yz\n");*/

		s = y;
		t = z;

		MakeNormal(&zero, s, t, &u);		/* Cross them for a new 3rd vector */

		MakeNormal(&zero, s, &u, &v);		/* Cross 1st & 3rd for a new 2nd */
		v.vx = -v.vx;
		v.vy = -v.vy;
		v.vz = -v.vz;

		CopyVector(&u, x);
		CopyVector(&v, z);

	}


	#if 0
	textprint("%d	%d	%d\n",
		x->vx,
		x->vy,
		x->vz
	);

	textprint("%d	%d	%d\n",
		y->vx,
		y->vy,
		y->vz
	);

	textprint("%d	%d	%d\n",
		z->vx,
		z->vy,
		z->vz
	);
	#endif

	#if 0
	textprint("mag. x = %d\n", Magnitude(x));
	textprint("mag. y = %d\n", Magnitude(y));
	textprint("mag. z = %d\n", Magnitude(z));
	#endif

	/*WaitForReturn();*/


}




/*

 ArcCos

 In:  COS value as -65,536 -> +65,536.
 Out: Angle in 0 -> 4095 form.

 Notes:

 The angle returned is in the range 0 -> 2,047 since the sign of SIN
 is not known.

 ArcSin(x) = ArcTan ( x, sqr ( 1-x*x ) )
 ArcCos(x) = ArcTan ( sqr ( 1-x*x ), x)

 -65,536 = 180 Degrees
 0	  = 90 Degrees
 +65,536 = 0 Degrees

 The table has 4,096 entries.

*/

int ArcCos(int c)

{

	short acos;

	if(c < (-(ONE_FIXED - 1))) c = -(ONE_FIXED - 1);
	else if(c > (ONE_FIXED - 1)) c = ONE_FIXED - 1;

	#if 0
	c =  c >> 5;		/* -64k -> +64k becomes -2k -> +2k */
	c += 2048;			/* -2k -> +2k becomes 0 -> 4k */
	#endif

	acos = ArcCosTable[(c >> 5) + 2048];

	return (int) (acos & wrap360);

}


/*

 ArcSin

 In:  SIN value in ax as -65,536 -> +65,536.
 Out: Angle in 0 -> 4095 form in ax.

 Notes:

 The angle returned is in the range -1,024 -> 1,023 since the sign of COS
 is not known.

 ArcSin(x) = ArcTan ( x, sqr ( 1-x*x ) )
 ArcCos(x) = ArcTan ( sqr ( 1-x*x ), x)

 -65,536 = 270 Degrees
 0	  = 0 Degrees
 +65,536 = 90 Degrees

 The table has 4,096 entries.

*/

int ArcSin(int s)

{

	short asin;


	if(s < (-(ONE_FIXED - 1))) s = -(ONE_FIXED - 1);
	else if(s > (ONE_FIXED - 1)) s = ONE_FIXED - 1;

	#if 0
	s =  s >> 5;		/* -64k -> +64k becomes -2k -> +2k */
	s += 2048;			/* -2k -> +2k becomes 0 -> 4k */
	#endif

	asin = ArcSineTable[(s >> 5) + 2048];

	return (int) (asin & wrap360);

}


/*

 ArcTan

 Pass (x,z)

 And ATN(x/z) is returned such that:

 000ø is Map North
 090ø is Map East
 180ø is Map South
 270ø is Map West

*/

int ArcTan(int height_x, int width_z)

{

	int abs_height_x, abs_width_z, angle, sign, signsame, temp;

	sign=0;

	if((height_x<0 && width_z<0) || (height_x>=0 && width_z>=0))
		signsame=Yes;
	else
		signsame=No;

	abs_height_x=height_x;
	if(abs_height_x<0) abs_height_x=-abs_height_x;

	abs_width_z=width_z;
	if(abs_width_z<0) abs_width_z=-abs_width_z;

/*

 Find ATN

*/

	if(width_z==0) angle=-deg90;

	else if(abs_width_z==abs_height_x)
		angle=deg45;

	else {

		if(abs_width_z>abs_height_x) {
			temp=abs_width_z;
			abs_width_z=abs_height_x;
			abs_height_x=temp;
			sign=-1;
		}

		if(abs_height_x!=0)

			/* angle = (abs_width_z << 8) / abs_height_x; */



			angle = DIV_INT((abs_width_z << 8), abs_height_x);





		else
			angle=deg22pt5;

		angle=ArcTanTable[angle];

		if(sign>=0) {
			angle=-angle;
			angle+=deg90;
		}

	}

	if(signsame==No) angle=-angle;

	if(width_z<=0) angle+=deg180;

	angle&=wrap360;

	return(angle);

}


/*

 Matrix from Z-Vector

*/

void MatrixFromZVector(VECTORCH *v, MATRIXCH *m)

{

	VECTORCH XVector;
	VECTORCH YVector;

	VECTORCH zero = {0, 0, 0};


	XVector.vx = v->vz;
	XVector.vy = 0;
	XVector.vz = -v->vx;

	Normalise(&XVector);

	MakeNormal(&zero, &XVector, v, &YVector);

	m->mat11 = XVector.vx;
	m->mat12 = XVector.vy;
	m->mat13 = XVector.vz;

	m->mat21 = -YVector.vx;
	m->mat22 = -YVector.vy;
	m->mat23 = -YVector.vz;

	m->mat31 = v->vx;
	m->mat32 = v->vy;
	m->mat33 = v->vz;

}










/*

 Distance Functions

*/


/*

 Foley and Van Dam 2d distance function

 WARNING! Returns distance x 3

 Here is the F & VD distance function:

 x + z + (max(x,z) * 2)
 ----------------------
          3

*/

int FandVD_Distance_2d(VECTOR2D *v0, VECTOR2D *v1)

{

	int max;
	int d;


	int dx = v1->vx - v0->vx;
	int dy = v1->vy - v0->vy;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;

	if(dx > dy) max = dx;
	else max = dy;

	d = (dx + dy + (max * 2));

	return d;

}


/*

 Foley and Van Dam 3d distance function

 WARNING! Returns distance x 9

 For a 3d version, calculate (f(f(x,y), y*3))/9

*/

int FandVD_Distance_3d(VECTORCH *v0, VECTORCH *v1)

{

	int dxy, max;

	int dz = v1->vz - v0->vz;

	if(dz < 0) dz = -dz;

	dz *= 3;

	dxy = FandVD_Distance_2d((VECTOR2D *) v0, (VECTOR2D *) v1);

	if(dxy > dz) max = dxy;
	else max = dz;

	return (dxy + dz + (max * 2));

}


/*

 NextLowPower2() returns the next lowest power of 2 of the passed value.

 e.g. 18 is returned as 16.

*/

int NextLowPower2(int i)

{

	int n = 1;


	while(n <= i)
		n <<= 1;

	return n >> 1;

}


/*

 Transform a world location into the local space of the passed matrix and
 location.

 Vector v1 is transformed to v2
 It is made relative to vector v3 and rotated using matrix m transposed

 A possible use is the transformation of world points into the local space
 of a display block

 e.g.

	MakeVectorLocal(&v1, &v2, &dptr->ObWorld, &dptr->ObMat);

 This would place vector v2 into the local space of display block dptr

*/

void MakeVectorLocal(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3, MATRIXCH *m)

{

	MATRIXCH transmat;


	CopyMatrix(m, &transmat);
	TransposeMatrixCH(&transmat);

	v2->vx = v1->vx - v3->vx;
	v2->vy = v1->vy - v3->vy;
	v2->vz = v1->vz - v3->vz;

	RotateVector(v2, &transmat);

}





/*

 Returns "Yes" if "point" is inside "polygon"


 **************************************************

 WARNING!! Point and Polygon Data are OVERWRITTEN!!

 **************************************************


 The function requires point to be an integer array containing a single
 XY pair. The number of points must be passed too.

 Pass the size of the polygon point e.g. A Gouraud polygon has points X,Y,I
 so its point size would be 3.


 Item								Polygon Point Size
 ----								------------------

 I_Polygon							2
 I_GouraudPolygon					3
 I_2dTexturedPolygon				4
 I_3dTexturedPolygon,			5
 I_Gouraud2dTexturedPolygon	5
 I_Polygon_ZBuffer				3
 I_GouraudPolygon_ZBuffer		4


 PASS ONLY POSITIVE COORDINATES!

*/

int PointInPolygon(int *point, int *polygon, int c, int ppsize)

{


	#if UseTimsPinp


  /* Tim's New Point In Polygon test-- hopefully much faster, */
  /* certainly much smaller. */
  /* Uses Half-Line test for point-in-2D-polygon test */
  /* Tests the half-line going from the point in the direction of positive z */

  int  x, z;        /* point */
  int  sx, sz;      /* vertex 1 */
  int *polyp;       /* vertex 2 pointer */
  int  t;
  int  dx, dz;      /* ABS(vertex 2 - vertex 1) */
  int  sgnx;        /* going left or going right */
  int  intersects;  /* number of intersections so far discovered */
  LONGLONGCH a_ll, b_ll;

  /* reject lines and points */
  if (c < 3) return(No);

  intersects = 0;

  x = point[ix];
  z = point[iy];  /* ! */

  /* get last point */
  polyp = polygon + ((c - 1) * ppsize);
  sx = polyp[0];
  sz = polyp[1];

  /* go back to first point */
  polyp = polygon;

dx = 0; /* TODO: uninitialized?? */

  /* for each point */
  while (0 != c)
  {
    
    /* is this line straddling the x co-ordinate of the point? */
    /* if not it is not worth testing for intersection with the half-line */
    /* we must be careful to get the strict and non-stict inequalities */
    /* correct, or we may count intersections with vertices the wrong number */
    /* of times. */
    sgnx = 0;
    if (sx < x && x <= polyp[0])
    {
      /* going right */
      sgnx = 1;
      dx   = polyp[0] - sx;
    }
    if (polyp[0] < x && x <= sx)
    {
      /* going left */
      sgnx = -1;
      dx   = sx - polyp[0];
    }

    /* if sgnx is zero then neither of the above conditions are true, */
    /* hence the line does not straddle the point in x */
    if (0 != sgnx)
    {
      /* next do trivial cases of line totally above or below point */
      if (z < sz && z < polyp[1])
      {
        /* line totally above point -- intersection */
        intersects++;
      }
      else if (z <= sz || z <= polyp[1])
      {
        /* line straddles point in both x and z -- we must do interpolation */

        /* get absolute differences between line end z co-ordinates */
        dz = (sz < polyp[1])?(polyp[1] - sz):(sz - polyp[1]);

        /* B504 is the square root of 7FFFFFFF */
        if (0xB504L < dx || 0xB504L < dz)
        {
          /* LARGE line -- use 64-bit values */
          /* interpolate z */
          MUL_I_WIDE(polyp[1] - sz, x - sx, &a_ll);
          MUL_I_WIDE(polyp[0] - sx, z - sz, &b_ll);
          if(CMP_LL(&a_ll, &b_ll) == sgnx)
          {
            /* we have an intersection */
            intersects++;
          }
        }
        else
        {
          /* small line -- use 32-bit values */
          /* interpolate z */
          t = (polyp[1] - sz) * (x - sx) - (polyp[0] - sx) * (z - sz);
          if ((t < 0 && sgnx < 0) || (0 < t && 0 < sgnx))
          {
            /* we have an intersection */
            intersects++;
          }
        }
      } /* (if line straddles point in z) */
    } /* (if line straddles point in x) */

    /* get next line : */
    /* new vertex 1 is old vertex 2 */
    sx = polyp[0];
    sz = polyp[1];

    /* new vertex 2 is next point */
    polyp += ppsize;

    /* next vertex */
    c--;
  }

  if (intersects & 1)
  {
    /* Odd number of intersections -- point is inside polygon */
    return(Yes);
  }
  else
  {
    /* even number of intersections -- point is outside polygon */
    return(No);
  }



#else


	int i;
	int si, ti;
	int s0, t0;
	int s1, t1;
	int *v0;
	int *v1;
	int ivdot, ivdotcnt, sgn_currivdot, sgn_ivdot, ivstate;
	int ns, nt;
	int x_scale, y_scale;
	int DotNudge;

	int x, z;
	LONGLONGCH xx;
	LONGLONGCH zz;
	LONGLONGCH xx_tmp;
	LONGLONGCH zz_tmp;
	VECTORCH PolyAvgPt;


	/* Reject points and lines */

	if(c < 3) return No;


	/* Find the average point */

	v0 = polygon;

	EQUALS_LL(&xx, &ll_zero);
	EQUALS_LL(&zz, &ll_zero);

	for(i = c; i!=0; i--) {

		x = v0[0];
		z = v0[1];

		IntToLL(&xx_tmp, &x);		/* xx_tmp = (long long)x */
		IntToLL(&zz_tmp, &z);		/* zz_tmp = (long long)z */

		ADD_LL_PP(&xx, &xx_tmp);	/* xx += xx_tmp */
		ADD_LL_PP(&zz, &zz_tmp);	/* zz += zz_tmp */

		v0 += ppsize;

	}

	PolyAvgPt.vx = NarrowDivide(&xx, c);
	PolyAvgPt.vz = NarrowDivide(&zz, c);


	/* Centre the polygon */

	v0 = polygon;

	for(i = c; i!=0; i--) {

		v0[0] -= PolyAvgPt.vx;
		v0[1] -= PolyAvgPt.vz;

		v0 += ppsize;

	}


	/* Centre the test point */

	point[0] -= PolyAvgPt.vx;
	point[1] -= PolyAvgPt.vz;


	/* Scale to avoid maths overflow */

	v0 = polygon;

	s0 = 0;
	t0 = 0;

	for(i = c; i!=0; i--) {

		si = v0[0]; if(si < 0) si = -si;
		if(si > s0) s0 = si;

		ti = v0[1]; if(ti < 0) ti = -ti;
		if(ti > t0) t0 = ti;

		v0 += ppsize;

	}

	si = point[ix]; if(si < 0) si = -si;
	if(si > s0) s0 = si;

	ti = point[iy]; if(ti < 0) ti = -ti;
	if(ti > t0) t0 = ti;


	#if 0
	textprint("\nmax x = %d\n", s0);
	textprint("max y = %d\n", t0);
	#endif


	x_scale = FindShift32(s0, 16383);
	y_scale = FindShift32(t0, 16383);


	#if 0
	textprint("scales = %d, %d\n", x_scale, y_scale);
	#endif


	v0 = polygon;

	for(i = c; i!=0; i--) {

		v0[0] >>= x_scale;
		v0[1] >>= y_scale;

		/*textprint("(%d, %d)\n", v0[0], v0[1]);*/

		v0 += ppsize;

	}

	point[ix] >>= x_scale;
	point[iy] >>= y_scale;




#if 1

	/* Clockwise or Anti-Clockwise? */

	ns = -(polygon[iy + ppsize] - polygon[iy]);
	nt =  (polygon[ix + ppsize] - polygon[ix]);

	si = polygon[(ppsize*2) + ix] - polygon[ix];
	ti = polygon[(ppsize*2) + iy] - polygon[iy];

	ivdot = (ns * si) + (nt * ti);

	if(ivdot < 0) DotNudge = -1;
	else DotNudge = 1;

#endif



	#if 0
	if(ivdot < 0) textprint("Clockwise\n");
	WaitForReturn();
	#endif


	/* Point to test */

	si = point[ix];
	ti = point[iy];


	#if 0
	textprint("p_test %d, %d\n", si, ti);
	#endif


	/* Polygon Vector pointers */

	v0 = polygon;
	v1 = v0 + ppsize;


	/* Dot result monitor */

	ivdotcnt = 0;
	ivstate  = Yes;			/* assume inside */


	/* Test v(s, t) against the vectors */

	for(i = c; i!=0 && ivstate == Yes; i--) {


		/* second vector pointer wraps once */

		if(i == 1) v1 = polygon;


		/* get the vector */

		s0 = v0[ix];
		t0 = v0[iy];

		s1 = v1[ix];
		t1 = v1[iy];


		#if 0
		textprint("%d,%d; %d,%d\n", s0, t0, s1, t1);
		#endif


		/* get the vector normal */

		ns = -(t1 - t0);		/* s -> -t */
		nt = s1 - s0;			/* t -> s  */


		/* Dot with intersection point */

		ivdot = (ns * (si - s0)) + (nt * (ti - t0));


		/* TEST */
		ivdot += DotNudge;


		sgn_ivdot = 1;
		if(ivdot < 0) sgn_ivdot = -1;


		/* only continue if current dot is same as last, else quit */

		if(ivdotcnt == 0) sgn_currivdot = sgn_ivdot;

		else {

			if(sgn_ivdot != sgn_currivdot) ivstate = No;
			sgn_currivdot = sgn_ivdot;

		}

		v0 += ppsize;
		v1 += ppsize;

		ivdotcnt++;

	}

	if(ivstate) return Yes;
	else return No;


#endif


}







/*

 #defines and statics required for Jamie's Most Excellent 
 random number generator

*/

#define DEG_3	31
#define SEP_3	3

static int32_t table [DEG_3] =
{
  -851904987, -43806228, -2029755270, 1390239686, -1912102820,
  -485608943, 1969813258, -1590463333, -1944053249, 455935928,
  508023712, -1714531963, 1800685987, -2015299881, 654595283,
  -1149023258, -1470005550, -1143256056, -1325577603, -1568001885,
  1275120390, -607508183, -205999574, -1696891592, 1492211999,
  -1528267240, -952028296, -189082757, 362343714, 1424981831,
  2039449641
};

#define TABLE_END (table + sizeof (table) / sizeof (table [0]))

static int32_t * front_ptr = table + SEP_3;
static int32_t * rear_ptr = table;


void SetSeededFastRandom(int seed);
void SetFastRandom(void)

{

	int i;
	long number = GetTickCount();


	for(i = 0; i < DEG_3; ++i) {

      number   = 1103515145 * number + 12345;
      table[i] = number;

	}

	front_ptr = table + SEP_3;
	rear_ptr  = table;

	for(i = 0; i < 10 * DEG_3; ++i)
		(void) FastRandom ();

	SetSeededFastRandom(FastRandom());

}


int FastRandom(void)

{

	int32_t i;

	/*

	Discard least random bit.
	Shift as unsigned to avoid replicating sign bit.
	Faster than masking.

	*/

	*front_ptr += *rear_ptr;
	i = (int32_t) ((uint32_t) *front_ptr >> 1);

	/* `front_ptr' and `rear_ptr' can't wrap at the same time. */

	++front_ptr;

	if(front_ptr < TABLE_END) {

      ++rear_ptr;

      if (rear_ptr < TABLE_END) return i;

      rear_ptr = table;

	}

	else {				/* front_ptr >= TABLE_END */

		front_ptr = table;
		++rear_ptr;

	}

	return (int) i;

}

/*a second copy of the random number generator for getting random numbers from a single seed*/

#define SEEDED_DEG_3	13
#define SEEDED_SEP_3	3

static int32_t seeded_table [SEEDED_DEG_3];

#define SEEDED_TABLE_END (seeded_table + sizeof (seeded_table) / sizeof (seeded_table [0]))

static int32_t * seeded_front_ptr = seeded_table + SEEDED_SEP_3;
static int32_t * seeded_rear_ptr = seeded_table;



int SeededFastRandom(void)

{

	int32_t i;

	/*

	Discard least random bit.
	Shift as unsigned to avoid replicating sign bit.
	Faster than masking.

	*/

	*seeded_front_ptr += *seeded_rear_ptr;
	i = (int32_t) ((uint32_t) *seeded_front_ptr >> 1);

	/* `front_ptr' and `rear_ptr' can't wrap at the same time. */

	++seeded_front_ptr;

	if(seeded_front_ptr < SEEDED_TABLE_END) {

      ++seeded_rear_ptr;

      if (seeded_rear_ptr < SEEDED_TABLE_END) return i;

      seeded_rear_ptr = seeded_table;

	}

	else {				/* front_ptr >= TABLE_END */

		seeded_front_ptr = seeded_table;
		++seeded_rear_ptr;

	}

	return (int) i;

}

void SetSeededFastRandom(int seed)

{

	int i;
	int32_t number = seed;


	for(i = 0; i < SEEDED_DEG_3; ++i) {

      number   = 1103515145 * number + 12345;
      seeded_table[i] = number;

	}

	seeded_front_ptr = seeded_table + SEEDED_SEP_3;
	seeded_rear_ptr  = seeded_table;

	for(i = 0; i < 2 * SEEDED_DEG_3; ++i)
		(void) SeededFastRandom ();

}

#if StandardShapeLanguage

/*

 Calculate the average point on this polygon

*/

void PolyAveragePoint(POLYHEADER *pheader, int *spts, VECTORCH *apt)

{

	int x, y, z;
	LONGLONGCH xx;
	LONGLONGCH yy;
	LONGLONGCH zz;
	LONGLONGCH xx_tmp;
	LONGLONGCH yy_tmp;
	LONGLONGCH zz_tmp;
	int *mypolystart = &pheader->Poly1stPt;
	int numpolypts;


	/* Find the average point */

	EQUALS_LL(&xx, &ll_zero);
	EQUALS_LL(&yy, &ll_zero);
	EQUALS_LL(&zz, &ll_zero);

	numpolypts = 0;

	while(*mypolystart != Term) {

		x = *(spts + *mypolystart + ix);
		y = *(spts + *mypolystart + iy);
		z = *(spts + *mypolystart + iz);

		IntToLL(&xx_tmp, &x);		/* xx_tmp = (long long)x */
		IntToLL(&yy_tmp, &y);		/* yy_tmp = (long long)y */
		IntToLL(&zz_tmp, &z);		/* zz_tmp = (long long)z */

		ADD_LL_PP(&xx, &xx_tmp);	/* xx += xx_tmp */
		ADD_LL_PP(&yy, &yy_tmp);	/* yy += yy_tmp */
		ADD_LL_PP(&zz, &zz_tmp);	/* zz += zz_tmp */

		numpolypts++;
		mypolystart++;

	}

	apt->vx = NarrowDivide(&xx, numpolypts);
	apt->vy = NarrowDivide(&yy, numpolypts);
	apt->vz = NarrowDivide(&zz, numpolypts);

}

#endif	/* StandardShapeLanguage */






/* KJL 15:07:39 01/08/97 - Returns the magnitude of the 
   cross product of two vectors a and b. */
int MagnitudeOfCrossProduct(VECTORCH *a, VECTORCH *b)

{
	VECTORCH c;				 
    
	c.vx = MUL_FIXED(a->vy,b->vz) - MUL_FIXED(a->vz,b->vy);
	c.vy = MUL_FIXED(a->vz,b->vx) - MUL_FIXED(a->vx,b->vz);
	c.vz = MUL_FIXED(a->vx,b->vy) - MUL_FIXED(a->vy,b->vx);
    
	return Magnitude(&c);
}

/* KJL 15:08:01 01/08/97 - sets the vector c to be the
   cross product of the vectors a and b. */
void CrossProduct(VECTORCH *a, VECTORCH *b, VECTORCH *c)

{
	c->vx = MUL_FIXED(a->vy,b->vz) - MUL_FIXED(a->vz,b->vy);
	c->vy = MUL_FIXED(a->vz,b->vx) - MUL_FIXED(a->vx,b->vz);
	c->vz = MUL_FIXED(a->vx,b->vy) - MUL_FIXED(a->vy,b->vx);
}



/* KJL 12:01:08 7/16/97 - returns the magnitude of a vector - max error about 13%, though average error
less than half this. Very fast compared to other approaches. */
int Approximate3dMagnitude(VECTORCH *v)
{
	int dx,dy,dz;

	dx = v->vx;
	if (dx<0) dx = -dx;
	
	dy = v->vy;
	if (dy<0) dy = -dy;
	
	dz = v->vz;	 	 
	if (dz<0) dz = -dz;
						 
	
	if (dx>dy)
	{
		if (dx>dz)
		{
			return dx + ((dy+dz)>>2);
		}
		else
		{
			return dz + ((dy+dx)>>2);
		}
	}
	else
	{
		if (dy>dz)
		{
			return dy + ((dx+dz)>>2);
		}
		else
		{
			return dz + ((dx+dy)>>2);
		}
	}
}


/*

 Quaternion to Matrix

 This is the column(row) matrix that is produced. Our matrices are
 row(column) and so are a transpose of this.

   1 - 2yy - 2zz           2xy + 2wz             2xz - 2wy

   2xy - 2wz               1 - 2xx - 2zz         2yz + 2wx

   2xz + 2wy               2yz - 2wx             1 - 2xx - 2yy

*/

void QuatToMat(QUAT *q,MATRIXCH *m)
{

	int q_w, q_x, q_y, q_z;

	int q_2x, q_2y, q_2z;

	int q_2xw;
	int q_2xx;
	int q_2xy;
	int q_2xz;
	int q_2yw;
	int q_2yy;
	int q_2yz;
	int q_2zw;
	int q_2zz;

/*

 The most efficient way to create the matrix is as follows

 1/ Double x, y & z

*/

	q_w=q->quatw;
	q_x=q->quatx;
	q_y=q->quaty;
	q_z=q->quatz;

	q_2x=q_x*2;
	q_2y=q_y*2;
	q_2z=q_z*2;

/*

 2/ Form their products with w, x, y & z
    These are

    (2x)w   (2y)w  (2z)w
    (2x)x
    (2x)y   (2y)y
    (2x)z   (2y)z  (2z)z

*/

	q_2xw=MUL_FIXED(q_2x,q_w);
	q_2yw=MUL_FIXED(q_2y,q_w);
	q_2zw=MUL_FIXED(q_2z,q_w);

	q_2xx=MUL_FIXED(q_2x,q_x);

	q_2xy=MUL_FIXED(q_2x,q_y);
	q_2yy=MUL_FIXED(q_2y,q_y);

	q_2xz=MUL_FIXED(q_2x,q_z);
	q_2yz=MUL_FIXED(q_2y,q_z);
	q_2zz=MUL_FIXED(q_2z,q_z);


/* mat11 = 1 - 2y^2 - 2z^2	 */

	m->mat11=ONE_FIXED-q_2yy-q_2zz;

/* mat12 = 2xy - 2wz */

	m->mat12=q_2xy-q_2zw;

/* mat13 = 2xz + 2wy */

	m->mat13=q_2xz+q_2yw;

/* mat21 = 2xy + 2wz */

	m->mat21=q_2xy+q_2zw;

/* mat22 = 1 - 2x^2 - 2z^2 */

	m->mat22=ONE_FIXED-q_2xx-q_2zz;

/* mat23 = 2yz - 2wx	 */

	m->mat23=q_2yz-q_2xw;

/* mat31 = 2xz - 2wy */

	m->mat31=q_2xz-q_2yw;

/* mat32 = 2yz + 2wx */

	m->mat32=q_2yz+q_2xw;

/* mat33 = 1 - 2x^2 - 2y^2 */

	m->mat33=ONE_FIXED-q_2xx-q_2yy;

}
