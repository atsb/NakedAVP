
#include "3dc.h"

#include <math.h>

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "pvisible.h"

#include "kzsort.h"
#include "kshape.h"

/*

 Platform Specific Functions

 These functions have been written specifically for a given platform.

 They are not necessarily IO or inline functions; these have their own files.

*/


/*

 externs for commonly used global variables and arrays

*/

	extern VECTORCH RotatedPts[];
	extern unsigned int Outcodes[];
	extern DISPLAYBLOCK *Global_ODB_Ptr;
	extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
	extern SHAPEHEADER *Global_ShapeHeaderPtr;
	extern MATRIXCH LToVMat;
	#if SupportMorphing
	extern VECTORCH MorphedPts[];
	extern MORPHDISPLAY MorphDisplay;
	#endif

	extern DISPLAYBLOCK *OnScreenBlockList[];
	extern int NumOnScreenBlocks;
	extern int NumActiveBlocks;
	extern DISPLAYBLOCK *ActiveBlockList[];
	#if SupportModules
	extern char *ModuleLocalVisArray;
	#endif



/*

 Global Variables

*/

	LONGLONGCH ll_one14 = {one14, 0};
	LONGLONGCH ll_zero = {0, 0};






/*

 Find out which objects are in the View Volume.

 The list of these objects, "OnScreenBlockList", is constructed in a number
 of stages.

 The Range Test is a simple outcode of the View Space Location against the
 Object Radius. The View Volume Test is more involved. The VSL is tested
 against each of the View Volume Planes. If it is more than one Object Radius
 outside any plane, the test fails.

*/

/*

 This is the main view volume test shell

*/








#if StandardShapeLanguage


/*

 Shape Points

 The item array is a table of pointers to item arrays. The points data is
 in the first and only item array.

 Rotate each point and write it out to the Rotated Points Buffer

 NOTE:

 There is a global pointer to the shape header "Global_ShapeHeaderPtr"

*/


#define print_bfcro_stats No

#if SupportMorphing
#define checkmorphpts No
#endif


void ShapePointsInstr(SHAPEINSTR *shapeinstrptr)

{

	int **shapeitemarrayptr;
	int *shapeitemptr;
	VECTORCH *rotptsptr;
	int x, y, z;
	int numitems;
	#if print_bfcro_stats
	int num_rot, num_not_rot;
	#endif



	/*

	Kevin, morphed doors WON'T be using shared points, so I've put your
	patch here, AFTER the intercept for the shared version of the points
	instruction -- Chris.

	*/

	#if KZSORT_ON /* KJL 15:13:46 02/07/97 - used for z-sorting doors correctly! */
	{
		extern int *MorphedObjectPointsPtr;
		MorphedObjectPointsPtr = 0;
	}
	#endif


	/* Set up pointers */

	shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	shapeitemptr      = *shapeitemarrayptr;
	rotptsptr         = &RotatedPts[0];



	#if SupportMorphing

	if(Global_ODB_Ptr->ObMorphCtrl) {


		#if LazyEvaluationForMorphing

		VECTORCH *morphptsptr;

		if(Global_ODB_Ptr->ObMorphedPts == 0) {

			Global_ODB_Ptr->ObMorphedPts = GetMorphedPts(Global_ODB_Ptr,
																		&MorphDisplay);

		}

		morphptsptr = Global_ODB_Ptr->ObMorphedPts;

		GLOBALASSERT(shapeinstrptr->sh_numitems<maxmorphPts);

		for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--) {

			#if SUPPORT_MMX
			if (use_mmx_math)
				MMX_VectorTransformedAndAdd(rotptsptr,morphptsptr,&LToVMat,&Global_ODB_Ptr->ObView);
			else
			#endif
			{
				rotptsptr->vx =  MUL_FIXED(LToVMat.mat11, morphptsptr->vx);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat21, morphptsptr->vy);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat31, morphptsptr->vz);
				rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

				rotptsptr->vy =  MUL_FIXED(LToVMat.mat12, morphptsptr->vx);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat22, morphptsptr->vy);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat32, morphptsptr->vz);
				rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

				rotptsptr->vz =  MUL_FIXED(LToVMat.mat13, morphptsptr->vx);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat23, morphptsptr->vy);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat33, morphptsptr->vz);
				rotptsptr->vz += Global_ODB_Ptr->ObView.vz;
			}

			rotptsptr->vy = MUL_FIXED(rotptsptr->vy,87381);
			morphptsptr++;
			rotptsptr++;

		}


		#else	/* LazyEvaluationForMorphing */


		VECTORCH *morphptsptr = &MorphedPts[0];
		SHAPEHEADER *sptr1;
		SHAPEHEADER *sptr2;
		int *shapeitemptr1;
		int *shapeitemptr2;
		int x1, y1, z1;
		int x2, y2, z2;
		#if checkmorphpts
		int num_old_pts = 0;
		int num_new_pts = 0;
		#endif


		/*textprint("morphing points\n");*/

		sptr1 = MorphDisplay.md_sptr1;
		sptr2 = MorphDisplay.md_sptr2;

		shapeitemptr1 = *(sptr1->points);
		shapeitemptr2 = *(sptr2->points);


		#if KZSORT_ON /* KJL 15:13:46 02/07/97 - used for z-sorting doors correctly! */
		{
			extern int *MorphedObjectPointsPtr;
			MorphedObjectPointsPtr = shapeitemptr2;
		}
		#endif


		for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--) {

			x1 = shapeitemptr1[ix];
			y1 = shapeitemptr1[iy];
			z1 = shapeitemptr1[iz];

			x2 = shapeitemptr2[ix];
			y2 = shapeitemptr2[iy];
			z2 = shapeitemptr2[iz];

			if(x1 == x2 && y1 == y2 && z1 == z2) {

				x = x1;
				y = y1;
				z = z1;

				#if checkmorphpts
				num_old_pts++;
				#endif

			}

			else if(MorphDisplay.md_lerp == 0) {

				x = x1;
				y = y1;
				z = z1;

			}

			else if(MorphDisplay.md_lerp == 0xffff) {

				x = x2;
				y = y2;
				z = z2;

			}

			else
			{
				/* KJL 15:27:20 05/22/97 - I've changed this to speed things up, If a vertex
				component has a magnitude greater than 32768 things will go wrong. */
				x = x1 + (((x2-x1)*MorphDisplay.md_lerp)>>16);
				y = y1 + (((y2-y1)*MorphDisplay.md_lerp)>>16);
				z = z1 + (((z2-z1)*MorphDisplay.md_lerp)>>16);

				#if checkmorphpts
				num_new_pts++;
				#endif

			}

			morphptsptr->vx = x;
			morphptsptr->vy = y;
			morphptsptr->vz = z;

			/* KJL 16:07:15 11/27/97 - I know this test is inside the loop,
			        but all this will go when I change to float everywhere. */
			#if MIRRORING_ON
			if(!Global_ODB_Ptr->ObMyModule || MirroringActive)
			#else
			if (!Global_ODB_Ptr->ObMyModule)
			#endif
			{
				#if SUPPORT_MMX
				if (use_mmx_math)
					MMX_VectorTransformedAndAdd(rotptsptr,morphptsptr,&LToVMat,&Global_ODB_Ptr->ObView);
				else
				#endif
				{
					rotptsptr->vx =  MUL_FIXED(LToVMat.mat11, x);
					rotptsptr->vx += MUL_FIXED(LToVMat.mat21, y);
					rotptsptr->vx += MUL_FIXED(LToVMat.mat31, z);
					rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

					rotptsptr->vy =  MUL_FIXED(LToVMat.mat12, x);
					rotptsptr->vy += MUL_FIXED(LToVMat.mat22, y);
					rotptsptr->vy += MUL_FIXED(LToVMat.mat32, z);
					rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

					rotptsptr->vz =  MUL_FIXED(LToVMat.mat13, x);
					rotptsptr->vz += MUL_FIXED(LToVMat.mat23, y);
					rotptsptr->vz += MUL_FIXED(LToVMat.mat33, z);
					rotptsptr->vz += Global_ODB_Ptr->ObView.vz;
				}
			}
			else /* KJL 14:33:24 11/27/97 - experiment to get rid of tears */
			{
				x += Global_ODB_Ptr->ObWorld.vx - Global_VDB_Ptr->VDB_World.vx;
				y += Global_ODB_Ptr->ObWorld.vy - Global_VDB_Ptr->VDB_World.vy;
				z += Global_ODB_Ptr->ObWorld.vz - Global_VDB_Ptr->VDB_World.vz;

				rotptsptr->vx =  MUL_FIXED(LToVMat.mat11, x);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat21, y);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat31, z);

				rotptsptr->vy =  MUL_FIXED(LToVMat.mat12, x);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat22, y);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat32, z);

				rotptsptr->vz =  MUL_FIXED(LToVMat.mat13, x);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat23, y);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat33, z);
			}
			
			
			shapeitemptr1 += vsize;
			shapeitemptr2 += vsize;
			morphptsptr++;
			
			rotptsptr->vy = MUL_FIXED(rotptsptr->vy,87381);
			rotptsptr++;

		}

		#if checkmorphpts
		textprint("num_old_pts = %d\n", num_old_pts);
		textprint("num_new_pts = %d\n", num_new_pts);
		#endif


		#endif	/* LazyEvaluationForMorphing */


	}

	else {

	#endif

		#if MIRRORING_ON
		int useFirstMethod = 0;


		if(!Global_ODB_Ptr->ObMyModule || MirroringActive)
		{
			useFirstMethod = 1;
		}
		if (Global_ODB_Ptr->ObStrategyBlock)
		{
			#if 0
			if(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourInanimateObject)
			{
				INANIMATEOBJECT_STATUSBLOCK* osPtr = Global_ODB_Ptr->ObStrategyBlock->SBdataptr;
				if(osPtr->typeId==IOT_Static)
				{
					useFirstMethod=0;
				}
			}
			#endif
		}

		if(useFirstMethod)
		#else
		if (!Global_ODB_Ptr->ObMyModule)
		#endif
		{
			for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--)
			{
				#if SUPPORT_MMX
				if (use_mmx_math)
					MMX_VectorTransformedAndAdd(rotptsptr,(VECTORCH *)shapeitemptr,&LToVMat,&Global_ODB_Ptr->ObView);
				else
				#endif
				{
					x = shapeitemptr[ix];
					y = shapeitemptr[iy];
					z = shapeitemptr[iz];

					rotptsptr->vx =  MUL_FIXED(LToVMat.mat11, x);
					rotptsptr->vx += MUL_FIXED(LToVMat.mat21, y);
					rotptsptr->vx += MUL_FIXED(LToVMat.mat31, z);
					rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

					rotptsptr->vy =  MUL_FIXED(LToVMat.mat12, x);
					rotptsptr->vy += MUL_FIXED(LToVMat.mat22, y);
					rotptsptr->vy += MUL_FIXED(LToVMat.mat32, z);
					rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

					rotptsptr->vz =  MUL_FIXED(LToVMat.mat13, x);
					rotptsptr->vz += MUL_FIXED(LToVMat.mat23, y);
					rotptsptr->vz += MUL_FIXED(LToVMat.mat33, z);
					rotptsptr->vz += Global_ODB_Ptr->ObView.vz;
				}
				shapeitemptr += 3;
				rotptsptr->vy = MUL_FIXED(rotptsptr->vy,87381);
				rotptsptr++;
				
			}
		}
		else
		{
			/* KJL 14:33:24 11/27/97 - experiment to get rid of tears */
			for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--)
			{
				x = shapeitemptr[ix];
				y = shapeitemptr[iy];
				z = shapeitemptr[iz];

				x += Global_ODB_Ptr->ObWorld.vx - Global_VDB_Ptr->VDB_World.vx;
				y += Global_ODB_Ptr->ObWorld.vy - Global_VDB_Ptr->VDB_World.vy;
				z += Global_ODB_Ptr->ObWorld.vz - Global_VDB_Ptr->VDB_World.vz;

				rotptsptr->vx =  MUL_FIXED(LToVMat.mat11, x);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat21, y);
				rotptsptr->vx += MUL_FIXED(LToVMat.mat31, z);

				rotptsptr->vy =  MUL_FIXED(LToVMat.mat12, x);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat22, y);
				rotptsptr->vy += MUL_FIXED(LToVMat.mat32, z);

				rotptsptr->vz =  MUL_FIXED(LToVMat.mat13, x);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat23, y);
				rotptsptr->vz += MUL_FIXED(LToVMat.mat33, z);
				shapeitemptr += 3;
				rotptsptr->vy = MUL_FIXED(rotptsptr->vy,87381);
				rotptsptr++;
			}

		}

	#if SupportMorphing
	}
	#endif


}


#endif	/* StandardShapeLanguage */







/*

 WideMul2NarrowDiv

 This function takes two pairs of integers, adds their 64-bit products
 together, divides the summed product with another integer and then returns
 the result of that divide, which is also an integer.

*/

int WideMul2NarrowDiv(int a, int b, int c, int d, int e)

{

	LONGLONGCH f;
	LONGLONGCH g;


	MUL_I_WIDE(a, b, &f);
	MUL_I_WIDE(c, d, &g);
	ADD_LL_PP(&f, &g);

	return NarrowDivide(&f, e);

}


/*

 Calculate Plane Normal from three POP's

 The three input vectors are treated as POP's and used to make two vectors.
 These are then crossed to create the normal.

 Make two vectors; (2-1) & (3-1)
 Cross them
 Normalise the vector
  Find the magnitude of the vector
  Divide each component by the magnitude

*/

void MakeNormal(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3, VECTORCH *v4)

{

	VECTORCHF vect0;
	VECTORCHF vect1;
	VECTORCHF n;


	/* vect0 = v2 - v1 */

	vect0.vx = v2->vx - v1->vx;
	vect0.vy = v2->vy - v1->vy;
	vect0.vz = v2->vz - v1->vz;

	/* vect1 = v3 - v1 */

	vect1.vx = v3->vx - v1->vx;
	vect1.vy = v3->vy - v1->vy;
	vect1.vz = v3->vz - v1->vz;


	/* nx = v0y.v1z - v0z.v1y */

	n.vx = (vect0.vy * vect1.vz) - (vect0.vz * vect1.vy);

	/* ny = v0z.v1x - v0x.v1z */

	n.vy = (vect0.vz * vect1.vx) - (vect0.vx * vect1.vz);

	/* nz = v0x.v1y - v0y.v1x */

	n.vz = (vect0.vx * vect1.vy) - (vect0.vy * vect1.vx);


	FNormalise(&n);

	f2i(v4->vx, n.vx * ONE_FIXED);
	f2i(v4->vy, n.vy * ONE_FIXED);
	f2i(v4->vz, n.vz * ONE_FIXED);


	#if 0
	textprint("Magnitude of v4 = %d\n", Magnitude(v4));
	WaitForReturn();
	#endif

}


/*

 Normalise a vector.

 The returned vector is a fixed point unit vector.

 WARNING!

 The vector must be no larger than 2<<14 because of the square root.
 Because this is an integer function, small components produce errors.

 e.g.

 (100,100,0)

 m=141 (141.42)

 nx = 100 * ONE_FIXED / m = 46,479
 ny = 100 * ONE_FIXED / m = 46,479
 nz = 0

 New m ought to be 65,536 but in fact is 65,731 i.e. 0.29% too large.

*/

void Normalise(VECTORCH *nvector)

{
	VECTORCHF n;
	float m;


	n.vx = nvector->vx;
	n.vy = nvector->vy;
	n.vz = nvector->vz;

	m = 65536.0/sqrt((n.vx * n.vx) + (n.vy * n.vy) + (n.vz * n.vz));

	f2i(nvector->vx, (n.vx * m) );
	f2i(nvector->vy, (n.vy * m) );
	f2i(nvector->vz, (n.vz * m) );
}







void Normalise2d(VECTOR2D *nvector)

{
	VECTOR2DF n;
	float m;


	n.vx = nvector->vx;
	n.vy = nvector->vy;

	m = sqrt((n.vx * n.vx) + (n.vy * n.vy));

	nvector->vx = (n.vx * ONE_FIXED) / m;
	nvector->vy = (n.vy * ONE_FIXED) / m;
}


void FNormalise(VECTORCHF *n)

{

	float m;


	m = sqrt((n->vx * n->vx) + (n->vy * n->vy) + (n->vz * n->vz));

	n->vx /= m;
	n->vy /= m;
	n->vz /= m;

}

void FNormalise2d(VECTOR2DF *n)

{

	float m;


	m = sqrt((n->vx * n->vx) + (n->vy * n->vy));

	n->vx /= m;
	n->vy /= m;

}


/*

 Return the magnitude of a vector

*/

int Magnitude(VECTORCH *v)

{
	VECTORCHF n;
	int m;


	n.vx = v->vx;
	n.vy = v->vy;
	n.vz = v->vz;

	f2i(m, sqrt((n.vx * n.vx) + (n.vy * n.vy) + (n.vz * n.vz)));

	return m;
}

/*

 Shift the 64-bit value until is LTE the limit

 Return the shift value

*/

int FindShift64(LONGLONGCH *value, LONGLONGCH *limit)

{

	int shift = 0;
	int s;
	LONGLONGCH value_tmp;


	EQUALS_LL(&value_tmp, value);

	s = CMP_LL(&value_tmp, &ll_zero);
	if(s < 0) NEG_LL(&value_tmp);


	while(GT_LL(&value_tmp, limit)) {

		shift++;

		ASR_LL(&value_tmp, 1);

	}

	return shift;

}


/*

 MaxLONGLONGCH

 Return a pointer to the largest value of a long long array

*/

void MaxLONGLONGCH(LONGLONGCH *llarrayptr, int llarraysize, LONGLONGCH *llmax)

{

	int i;


	EQUALS_LL(llmax, &ll_zero);

	for(i = llarraysize; i!=0; i--) {

		if(LT_LL(llmax, llarrayptr)) {

			EQUALS_LL(llmax, llarrayptr);

		}

		llarrayptr++;

	}

}












/*

 Some operators derived from the 64-bit CMP function.

*/


/*

 GT_LL

 To express if(a > b)

 use

 if(GT_LL(a, b))

*/

int GT_LL(LONGLONGCH *a, LONGLONGCH *b)

{

	int s = CMP_LL(a, b);		/* a-b */


	if(s > 0) return (Yes);

	else return (No);

}


/*

 LT_LL

 To express if(a < b)

 use

 if(LT_LL(a, b))

*/

int LT_LL(LONGLONGCH *a, LONGLONGCH *b)

{

	int s = CMP_LL(a, b);		/* a-b */


	if(s < 0) return (Yes);

	else return (No);

}




/*

 Copy Clip Point Function

*/

void CopyClipPoint(CLIP_POINT *cp1, CLIP_POINT *cp2)

{

	cp2->ClipPoint.vx = cp1->ClipPoint.vx;
	cp2->ClipPoint.vy = cp1->ClipPoint.vy;
	cp2->ClipPoint.vz = cp1->ClipPoint.vz;

	cp2->ClipNormal.vx = cp1->ClipNormal.vx;
	cp2->ClipNormal.vy = cp1->ClipNormal.vy;
	cp2->ClipNormal.vz = cp1->ClipNormal.vz;

	cp2->ClipTexel.uuu = cp1->ClipTexel.uuu;
	cp2->ClipTexel.vee = cp1->ClipTexel.vee;

	cp2->ClipInt     = cp1->ClipInt;
	cp2->ClipZBuffer = cp1->ClipZBuffer;

}


/*

 Matrix Rotatation of a Vector - Inline Version

 Overwrite the Source Vector with the Rotated Vector

 x' = v.c1
 y' = v.c2
 z' = v.c3

*/

void RotVect(VECTORCH *v, MATRIXCH *m)

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

 Dot Product Function - Inline Version

 It accepts two pointers to vectors and returns an int result

*/

int _Dot(VECTORCH *vptr1, VECTORCH *vptr2)

{

	int dp;

	dp  = MUL_FIXED(vptr1->vx, vptr2->vx);
	dp += MUL_FIXED(vptr1->vy, vptr2->vy);
	dp += MUL_FIXED(vptr1->vz, vptr2->vz);

	return(dp);

}


/*

 Make a Vector - Inline Version

 v3 = v1 - v2

*/

void MakeV(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3)

{

	v3->vx = v1->vx - v2->vx;
	v3->vy = v1->vy - v2->vy;
	v3->vz = v1->vz - v2->vz;

}

/*

 Add a Vector.

 v2 = v2 + v1

*/

void AddV(VECTORCH *v1, VECTORCH *v2)

{

	v2->vx += v1->vx;
	v2->vy += v1->vy;
	v2->vz += v1->vz;

}
