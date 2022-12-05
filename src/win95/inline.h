#ifndef INLINE_INCLUDED
#define INLINE_INCLUDED

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif


#if SUPPORT_MMX
#include "mmx_math.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* 
	Standard macros.  Note that FIXED_TO_INT
	and INT_TO_FIXED are very suboptimal in 
	this version!!!
	Also, MUL_INT and ISR are ONLY intended 
	to be used in Win95 so that Saturn versions
	of the same code can be compiled using calls
	to hand optimised assembler functions, i.e.
	for code that is never intended to be run on
	a Saturn they are unnecessary.
*/

#define OUR_ABS(x)                (((x) < 0) ? -(x) : (x))
#define OUR_SIGN(x)	             (((x) < 0) ? -1 : +1)
#define OUR_INT_TO_FIXED(x)	 	 (int) ((x) * (65536))
#define OUR_FIXED_TO_INT(x)		 (int) ((x) / (65536))
#define OUR_MUL_INT(a, b)	       ((a) * (b))
#define OUR_ISR(a, shift)		    ((a) >> (shift))

/*

 Platform Specific 64-Bit Operator Functions

 Not all compilers support 64-bit operations, and some platforms may not
 even support 64-bit numbers. Support for 64-bit operations is therefore
 provided in the platform specific fucntions below.

 For C++ a mew class could be defined. However the current system is not
 compiled as C++ and the Cygnus GNU C++ is not currently working.

*/


/*
	These functions have been checked for suitability for 
	a Pentium and look as if they would pair up okay.
	Might be worth a more detailed look at optimising
	them though.
	Obviously there is a problem with values not being
	loaded into registers for these functions, but this
	may be unavoidable for 64 bit values on a Watcom
	platform.
*/


#if defined(_MSC_VER) && 0 /* inline assember for the Microsoft compiler */

/* ADD */

static void ADD_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c)
{
	_asm
	{
		mov esi,a
		mov edi,b
		mov ebx,c
		mov	eax,[esi]
		mov	edx,[esi+4]
		add	eax,[edi]
		adc	edx,[edi+4]
		mov	[ebx],eax
		mov	[ebx+4],edx
	}
}

/* ADD ++ */

static void ADD_LL_PP(LONGLONGCH *c, LONGLONGCH *a)
{
	_asm
	{
		mov edi,c
		mov esi,a
		mov	eax,[esi]
		mov	edx,[esi+4]
		add	[edi],eax
		adc	[edi+4],edx
	}
}

/* SUB */

static void SUB_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c)
{
	_asm
	{
		mov esi,a
		mov edi,b
		mov ebx,c
		mov	eax,[esi]
		mov	edx,[esi+4]
		sub	eax,[edi]
		sbb	edx,[edi+4]
		mov	[ebx],eax
		mov	[ebx+4],edx
	}
}

/* SUB -- */

static void SUB_LL_MM(LONGLONGCH *c, LONGLONGCH *a)
{
	_asm
	{
		mov edi,c
		mov esi,a
		mov	eax,[esi]
		mov	edx,[esi+4]
		sub	[edi],eax
		sbb	[edi+4],edx
	}
}

/*

 MUL

 This is the multiply we use, the 32 x 32 = 64 widening version

*/

static void MUL_I_WIDE(int a, int b, LONGLONGCH *c)
{
	_asm
	{
		mov eax,a
		mov ebx,c
		imul b
		mov	[ebx],eax
		mov	[ebx+4],edx
	}
}

/*

 CMP

 This substitutes for ==, >, <, >=, <=

*/

static int CMP_LL(LONGLONGCH *a, LONGLONGCH *b)
{
	int retval = 0;
	_asm
	{
		mov ebx,a
		mov ecx,b
		mov	eax,[ebx]
		mov	edx,[ebx+4]
		sub	eax,[ecx]
		sbb	edx,[ecx+4]
		and	edx,edx
		jne	llnz
		and	eax,eax
		je	llgs
		llnz:
		mov	retval,1
		and	edx,edx
		jge	llgs
		neg	retval
		llgs:
	}
	return retval;
}

/* EQUALS */

static void EQUALS_LL(LONGLONGCH *a, LONGLONGCH *b)
{
	_asm
	{
		mov edi,a
		mov esi,b
		mov	eax,[esi]
		mov	edx,[esi+4]
		mov	[edi],eax
		mov	[edi+4],edx
	}
}

/* NEGATE */

static void NEG_LL(LONGLONGCH *a)
{
	_asm
	{
		mov esi,a
		not	dword ptr[esi]
		not	dword ptr[esi+4]
		add	dword ptr[esi],1
		adc	dword ptr[esi+4],0
	}
}

/* ASR */

static void ASR_LL(LONGLONGCH *a, int shift)
{
	_asm
	{
		mov esi,a
		mov eax,shift
		and	eax,eax
		jle	asrdn
		asrlp:
		sar	dword ptr[esi+4],1
		rcr	dword ptr[esi],1
		dec	eax
		jne	asrlp
		asrdn:
	}
}

/* Convert int to LONGLONGCH */

static void IntToLL(LONGLONGCH *a, int *b)
{
	_asm
	{
		mov esi,b
		mov edi,a
		mov	eax,[esi]
		cdq
		mov	[edi],eax
		mov	[edi+4],edx
	}
}

/*

 Fixed Point Multiply.


 16.16 * 16.16 -> 16.16
 or
 16.16 * 0.32 -> 0.32

 A proper version of this function ought to read
 16.16 * 16.16 -> 32.16
 but this would require a long long result

 Algorithm:

 Take the mid 32 bits of the 64 bit result

*/

/*
	These functions have been checked for suitability for 
	a Pentium and look as if they would work adequately.
	Might be worth a more detailed look at optimising
	them though.
*/

static int MUL_FIXED(int a, int b)
{
	int retval;
	_asm
	{
		mov eax,a
		imul b
		shrd eax,edx,16
		mov retval,eax
	}
	return retval;
}

/*

 Fixed Point Divide - returns a / b

*/

static int DIV_FIXED(int a, int b)
{
	int retval;
	_asm
	{
		mov eax,a
		cdq
		rol eax,16
		mov dx,ax
		xor ax,ax
		idiv b
		mov retval,eax
	}
	return retval;
}

/*

 Multiply and Divide Functions.

*/


/*

 32/32 division

 This macro is a function on some other platforms

*/

#define DIV_INT(a, b) ((a) / (b))

/*

 A Narrowing 64/32 Division

*/

static int NarrowDivide(LONGLONGCH *a, int b)
{
	int retval;
	_asm
	{
		mov esi,a
		mov	eax,[esi]
		mov	edx,[esi+4]
		idiv	b
		mov retval,eax
	}
	return retval;
}

/*

 This function performs a Widening Multiply followed by a Narrowing Divide.

 a = (a * b) / c

*/

static int WideMulNarrowDiv(int a, int b, int c)
{
	int retval;
	_asm
	{
		mov eax,a
		imul b
		idiv c
		mov retval,eax
	}
	return retval;
}

/*

 Function to rotate a VECTORCH using a MATRIXCH

 This is the C function

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

 This is the MUL_FIXED inline assembler function

	imul edx
	shrd eax,edx,16


typedef struct matrixch {

	int mat11;	0
	int mat12;	4
	int mat13;	8

	int mat21;	12
	int mat22;	16
	int mat23;	20

	int mat31;	24
	int mat32;	28
	int mat33;	32

} MATRIXCH;

*/

static void RotateVector_ASM(VECTORCH *v, MATRIXCH *m)
{
	_asm
	{
		mov esi,v
		mov edi,m

		mov	eax,[edi + 0]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ecx,eax
		mov	eax,[edi + 12]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ecx,eax
		mov	eax,[edi + 24]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ecx,eax

		mov	eax,[edi + 4]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ebx,eax
		mov	eax,[edi + 16]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ebx,eax
		mov	eax,[edi + 28]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ebx,eax

		mov	eax,[edi + 8]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ebp,eax
		mov	eax,[edi + 20]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ebp,eax
		mov	eax,[edi + 32]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ebp,eax

		mov	[esi + 0],ecx
		mov	[esi + 4],ebx
		mov	[esi + 8],ebp
	}
}

/*

 Here is the same function, this time copying the result to a second vector

*/

static void RotateAndCopyVector_ASM(VECTORCH *v1, VECTORCH *v2, MATRIXCH *m)
{
	_asm
	{
		mov esi,v1
		mov edi,m

		mov	eax,[edi + 0]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ecx,eax
		mov	eax,[edi + 12]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ecx,eax
		mov	eax,[edi + 24]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ecx,eax

		mov	eax,[edi + 4]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ebx,eax
		mov	eax,[edi + 16]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ebx,eax
		mov	eax,[edi + 28]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ebx,eax

		mov	eax,[edi + 8]
		imul	DWORD PTR [esi + 0]
		shrd	eax,edx,16
		mov	ebp,eax
		mov	eax,[edi + 20]
		imul	DWORD PTR [esi + 4]
		shrd	eax,edx,16
		add	ebp,eax
		mov	eax,[edi + 32]
		imul	DWORD PTR [esi + 8]
		shrd	eax,edx,16
		add	ebp,eax

		mov edx,v2
		mov	[edx + 0],ecx
		mov	[edx + 4],ebx
		mov	[edx + 8],ebp
	}
}

/*

 Square Root

 Returns the Square Root of a 32-bit number

*/

static long temp;
static long temp2;

static int SqRoot32(int A)
{
	_asm
	{
		finit
		fild A
		fsqrt
		fistp temp2
		fwait
	}
	return (int)temp2;
}


/*

 This may look ugly (it is) but it is a MUCH faster way to convert "float" into "int" than
 the function call "CHP" used by the WATCOM compiler.

*/

static float fptmp;
static int itmp;

static void FloatToInt(void)
{
	_asm
	{
		fld fptmp
		fistp itmp
	}
}

/*

 This macro makes usage of the above function easier and more elegant

*/

#define f2i(a, b) { \
fptmp = (b); \
FloatToInt(); \
a = itmp;}

#else

// parts of mathline.c that have been re-inlined.
// MUL_FIXED, f2i
#include "mathline.h"

/* inline assembly has been moved to mathline.c */
void ADD_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c);
void ADD_LL_PP(LONGLONGCH *c, LONGLONGCH *a);
void SUB_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c);
void SUB_LL_MM(LONGLONGCH *c, LONGLONGCH *a);
void MUL_I_WIDE(int a, int b, LONGLONGCH *c);
int CMP_LL(LONGLONGCH *a, LONGLONGCH *b);
void EQUALS_LL(LONGLONGCH *a, LONGLONGCH *b);
void NEG_LL(LONGLONGCH *a);
void ASR_LL(LONGLONGCH *a, int shift);
void IntToLL(LONGLONGCH *a, int *b);
int DIV_FIXED(int a, int b);

#define DIV_INT(a, b) ((a) / (b))

int NarrowDivide(LONGLONGCH *a, int b);
int WideMulNarrowDiv(int a, int b, int c);
void RotateVector_ASM(VECTORCH *v, MATRIXCH *m);
void RotateAndCopyVector_ASM(VECTORCH *v1, VECTORCH *v2, MATRIXCH *m);

int SqRoot32(int A);

#endif

int WideMul2NarrowDiv(int a, int b, int c, int d, int e);
int _Dot(VECTORCH *vptr1, VECTORCH *vptr2);
void MakeV(VECTORCH *v1, VECTORCH *v2, VECTORCH *v3);
void AddV(VECTORCH *v1, VECTORCH *v2);
void RotVect(VECTORCH *v, MATRIXCH *m);

#if SUPPORT_MMX

#define RotateVector(v,m) (use_mmx_math ? MMX_VectorTransform((v),(m)) : _RotateVector((v),(m)))
#define RotateAndCopyVector(v_in,v_out,m) (use_mmx_math ? MMX_VectorTransformed((v_out),(v_in),(m)) : _RotateAndCopyVector((v_in),(v_out),(m)))
#define Dot(v1,v2) (use_mmx_math ? MMXInline_VectorDot((v1),(v2)) : _Dot((v1),(v2)))
#define DotProduct(v1,v2) (use_mmx_math ? MMX_VectorDot((v1),(v2)) : _DotProduct((v1),(v2)))

#else /* ! SUPPORT_MMX */

#define RotateVector(v,m) (_RotateVector((v),(m)))
#define RotateAndCopyVector(v_in,v_out,m) (_RotateAndCopyVector((v_in),(v_out),(m)))
#define Dot(v1,v2) (_Dot((v1),(v2)))
#define DotProduct(v1,v2) (_DotProduct((v1),(v2)))

#endif /* ? SUPPORT_MMX */

#ifdef __cplusplus
}
#endif


#endif
