#include <math.h>

#include "3dc.h"
#include "mathline.h"

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

int NarrowDivide(LONGLONGCH *a, int b);
int WideMulNarrowDiv(int a, int b, int c);
void RotateVector_ASM(VECTORCH *v, MATRIXCH *m);
void RotateAndCopyVector_ASM(VECTORCH *v1, VECTORCH *v2, MATRIXCH *m);

#undef ASM386

#if !defined(ASM386)
static __int64 ConvertToLongLong(const LONGLONGCH* llch)
{
	__int64 ll;
	
	ll = ((__int64)llch->hi32 << 32) | ((__int64)llch->lo32 << 0);
	
	return ll;
}

static void ConvertFromLongLong(LONGLONGCH* llch, const __int64* ll)
{
	llch->lo32 = (unsigned int)((*ll>> 0) & 0xffffffff);
	llch->hi32 = (  signed int)((*ll>>32) & 0xffffffff);	
}
#endif

void ADD_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c)
{
/*
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
*/
#if defined(ASM386)
int dummy1, dummy2;
__asm__("movl	0(%%esi), %0		\n\t"
	"movl	4(%%esi), %1		\n\t"
	"addl	0(%%edi), %0		\n\t"
	"adcl	4(%%edi), %1		\n\t"
	"movl	%0, 0(%%ebx)		\n\t"
	"movl	%1, 4(%%ebx)		\n\t"
	: "=&r" (dummy1), "=&r" (dummy2)
	: "S" (a), "D" (b), "b" (c)
	: "memory", "cc"
	);

/*
__asm__("movl	0(%%esi), %%eax		\n\t"
	"movl	4(%%esi), %%edx		\n\t"
	"addl	0(%%edi), %%eax		\n\t"
	"adcl	4(%%edi), %%edx		\n\t"
	: "=a" (c->lo32), "=d" (c->hi32)
	: "S" (a), "D" (b)
	);
*/
#else
	__int64 aa = ConvertToLongLong(a);
	__int64 bb = ConvertToLongLong(b);
	
	__int64 cc = aa + bb;
	
	ConvertFromLongLong(c, &cc);
#endif

}

/* ADD ++ */

void ADD_LL_PP(LONGLONGCH *c, LONGLONGCH *a)
{
/*
	_asm
	{
		mov edi,c
		mov esi,a
		mov	eax,[esi]
	 	mov	edx,[esi+4]
		add	[edi],eax
		adc	[edi+4],edx
	}
*/

#if defined(ASM386)
int dummy1, dummy2;
__asm__("movl	0(%%esi), %0		\n\t"
	"movl	4(%%esi), %1		\n\t"
	"addl	%0, 0(%%edi)		\n\t"
	"adcl	%1, 4(%%edi)		\n\t"
	: "=&r" (dummy1), "=&r" (dummy2)
	: "D" (c), "S" (a)
	: "memory", "cc"
	);
#else
	__int64 cc = ConvertToLongLong(c);
	__int64 aa = ConvertToLongLong(a);
	
	cc += aa;
	
	ConvertFromLongLong(c, &cc);
#endif
}

/* SUB */

void SUB_LL(LONGLONGCH *a, LONGLONGCH *b, LONGLONGCH *c)
{
/*
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
*/
#if defined(ASM386)
int dummy1, dummy2;
__asm__("movl	0(%%esi), %0		\n\t"
	"movl	4(%%esi), %1		\n\t"
	"subl	0(%%edi), %0		\n\t"
	"sbbl	4(%%edi), %1		\n\t"
	"movl	%0, 0(%%ebx)		\n\t"
	"movl	%1, 4(%%ebx)		\n\t"
	: "=&r" (dummy1), "=&r" (dummy2)
	: "S" (a), "D" (b), "b" (c)
	: "memory", "cc"
	);
#else
	__int64 aa = ConvertToLongLong(a);
	__int64 bb = ConvertToLongLong(b);
	
	__int64 cc = aa - bb;
	
	ConvertFromLongLong(c, &cc);
#endif
}

/* SUB -- */

void SUB_LL_MM(LONGLONGCH *c, LONGLONGCH *a)
{
/*
	_asm
	{
		mov edi,c
		mov esi,a
		mov	eax,[esi]
		mov	edx,[esi+4]
		sub	[edi],eax
		sbb	[edi+4],edx
	}
*/
#if defined(ASM386)
int dummy1, dummy2;
__asm__("movl	0(%%esi), %0		\n\t"
	"movl	4(%%esi), %1		\n\t"
	"subl	%0, 0(%%edi)		\n\t"
	"sbbl	%1, 4(%%edi)		\n\t"
	: "=&r" (dummy1), "=&r" (dummy2)
	: "D" (c), "S" (a)
	: "memory", "cc"
	);
#else
	__int64 cc = ConvertToLongLong(c);
	__int64 aa = ConvertToLongLong(a);
	
	cc -= aa;
	
	ConvertFromLongLong(c, &cc);
#endif
}

/*

 MUL

 This is the multiply we use, the 32 x 32 = 64 widening version

*/

void MUL_I_WIDE(int a, int b, LONGLONGCH *c)
{
/*
	_asm
	{
		mov eax,a
		mov ebx,c
		imul b
		mov	[ebx],eax
		mov	[ebx+4],edx
	}
*/
#if defined(ASM386)
unsigned int d1;
__asm__("imull	%3			\n\t"
	"movl	%%eax, 0(%%ebx)		\n\t"
	"movl	%%edx, 4(%%ebx)		\n\t"
	: "=a" (d1)
	: "0" (a), "b" (c), "m" (b)
	: "%edx", "memory", "cc"
	);
#else
	__int64 aa = (__int64) a;
	__int64 bb = (__int64) b;
	
	__int64 cc = aa * bb;
	
	ConvertFromLongLong(c, &cc);
#endif
}

/*

 CMP

 This substitutes for ==, >, <, >=, <=

*/

int CMP_LL(LONGLONGCH *a, LONGLONGCH *b)
{
/*
	int retval;
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
*/
#if defined(ASM386)
	int retval;

__asm__("movl	0(%%ebx), %%eax		\n\t"
	"movl	4(%%ebx), %%edx		\n\t"
	"subl	0(%%ecx), %%eax		\n\t"
	"sbbl	4(%%ecx), %%edx		\n\t"
	"xorl	%%ebx, %%ebx            \n\t"
	"andl	%%edx, %%edx		\n\t"
	"jne	0f			\n\t" /* llnz */
	"andl	%%eax, %%eax		\n\t"
	"je	1f			\n"   /* llgs */
"0:					\n\t" /* llnz */
	"movl	$1, %%ebx		\n\t"
	"andl	%%edx, %%edx		\n\t"
	"jge	1f			\n\t" /* llgs */
	"negl	%%ebx			\n"
"1:					\n\t" /* llgs */
	: "=b" (retval)
	: "b" (a), "c" (b)
	: "%eax", "%edx", "memory", "cc"
	);
	
	return retval;
#else
	if (a->hi32 > b->hi32)
		return 1;
	else if (a->hi32 < b->hi32)
		return -1;
	else if (a->lo32 > b->lo32)
		return 1;
	else if (a->lo32 < b->lo32)
		return -1;
	else
		return 0;
#endif		
}

/* EQUALS */

void EQUALS_LL(LONGLONGCH *a, LONGLONGCH *b)
{
/*
	_asm
	{
		mov edi,a
		mov esi,b
		mov	eax,[esi]
		mov	edx,[esi+4]
		mov	[edi],eax
		mov	[edi+4],edx
	}
*/
#if defined(ASM386)
__asm__("movl	0(%%esi), %%eax		\n\t"
	"movl	4(%%esi), %%edx		\n\t"
	"movl	%%eax, 0(%%edi)		\n\t"
	"movl	%%edx, 4(%%edi)		\n\t"
	:
	: "D" (a), "S" (b)
	: "%eax", "%edx", "memory"
	);
#else
	*a = *b;
#endif
}

/* NEGATE */

void NEG_LL(LONGLONGCH *a)
{
/*
	_asm
	{
		mov esi,a
		not	dword ptr[esi]
		not	dword ptr[esi+4]
		add	dword ptr[esi],1
		adc	dword ptr[esi+4],0
	}
*/
#if defined(ASM386)
__asm__("notl	0(%%esi)		\n\t"
	"notl	4(%%esi)		\n\t"
	"addl	$1, 0(%%esi)		\n\t"
	"adcl	$0, 4(%%esi)		\n\t"
	:
	: "S" (a)
	: "memory", "cc"
	);
#else
	__int64 aa = ConvertToLongLong(a);
	
	aa = -aa;
	
	ConvertFromLongLong(a, &aa);
#endif
}

/* ASR */

void ASR_LL(LONGLONGCH *a, int shift)
{
/*
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
*/
#if defined(ASM386)
unsigned int d1;
__asm__ volatile
	("andl	%0, %0			\n\t"
	"jle	0			\n" /* asrdn */
"1:					\n\t" /* asrlp */
	"sarl	$1, 4(%%esi)		\n\t"
	"rcrl	$1, 0(%%esi)		\n\t"
	"decl	%0			\n\t"
	"jne	1			\n"
"0:					\n\t"
	: "=&r" (d1)
	: "S" (a), "a" (shift)
	: "memory", "cc"
	);
#else
	__int64 aa = ConvertToLongLong(a);
	
	aa >>= shift;
	
	ConvertFromLongLong(a, &aa);
#endif	
}

/* Convert int to LONGLONGCH */

void IntToLL(LONGLONGCH *a, int *b)
{
/*
	_asm
	{
		mov esi,b
		mov edi,a
		mov	eax,[esi]
		cdq
		mov	[edi],eax
		mov	[edi+4],edx
	}
*/
#if defined(ASM386)
__asm__("movl	0(%%esi), %%eax		\n\t"
	"cdq				\n\t"
	"movl	%%eax, 0(%%edi)		\n\t"
	"movl	%%edx, 4(%%edi)		\n\t"
	: 
	: "S" (b), "D" (a)
	: "%eax", "%edx", "memory", "cc"
	);
#else
	__int64 aa = (__int64) *b;
	
	ConvertFromLongLong(a, &aa);
#endif
}

//
// Fixed Point Multiply - MUL_FIXED
// See mathline.h
//

/*

 Fixed Point Divide - returns a / b

*/

int DIV_FIXED(int a, int b)
{
	if (b == 0) printf("DEBUG THIS: a = %d, b = %d\n", a, b);	
	
	if (b == 0) return 0; /* TODO: debug this! (start with alien on ferarco) */
/*
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
*/
#if defined(ASM386)
	int retval;
__asm__("cdq				\n\t"
	"roll	$16, %%eax		\n\t"
	"mov	%%ax, %%dx		\n\t"
	"xor	%%ax, %%ax		\n\t"
	"idivl	%2			\n\t"
	: "=a" (retval)
	: "0" (a), "m" (b)
	: "%edx", "cc"
	);
	return retval;
#else
	{
	__int64 aa = (__int64) a;
	__int64 bb = (__int64) b;
	__int64 cc = (aa << 16) / bb;
	
	return (int) (cc & 0xffffffff);
	}
#endif
}

/*

 Multiply and Divide Functions.

*/

/*

 A Narrowing 64/32 Division

*/

int NarrowDivide(LONGLONGCH *a, int b)
{
/*
	int retval;
	_asm
	{
		mov esi,a
		mov	eax,[esi]
		mov	edx,[esi+4]
		idiv	b
		mov retval,eax
	}
*/
#if defined(ASM386)
	int retval;
__asm__("movl	0(%%esi), %%eax		\n\t"
	"movl	4(%%esi), %%edx		\n\t"
	"idivl	%2			\n\t"
	: "=a" (retval)
	: "S" (a), "m" (b)
	: "%edx", "cc"
	);
	return retval;
#else
	__int64 aa = ConvertToLongLong(a);
	__int64 bb = (__int64) b;
	
	__int64 cc = aa / bb;
	
	return (int) (cc & 0xffffffff);
#endif
}

/*

 This function performs a Widening Multiply followed by a Narrowing Divide.

 a = (a * b) / c

*/

int WideMulNarrowDiv(int a, int b, int c)
{
/*
	int retval;
	_asm
	{
		mov eax,a
		imul b
		idiv c
		mov retval,eax
	}
*/
#if defined(ASM386)
	int retval;
__asm__("imull	%2			\n\t"
	"idivl	%3			\n\t"
	: "=a" (retval)
	: "0" (a), "m" (b), "m" (c)
	: "%edx", "cc"
	);	
	return retval;
#else
	__int64 aa = (__int64) a;
	__int64 bb = (__int64) b;
	__int64 cc = (__int64) c;
	
	__int64 dd = (aa * bb) / cc;
	
	return (int) (dd & 0xffffffff);
#endif
}

/*

 Square Root

 Returns the Square Root of a 32-bit number

*/

int SqRoot32(int A)
{
/*
	_asm
	{
		finit
		fild A
		fsqrt
		fistp temp2
		fwait
	}
*/

#if defined(ASM386)
	static volatile int sqrt_temp;
__asm__ volatile
	("finit				\n\t"
	"fildl	%0			\n\t"
	"fsqrt				\n\t"
	"fistpl	sqrt_temp		\n\t"
	"fwait				\n\t"
	:
	: "m" (A)
	: "memory", "cc"
	);
	
	return sqrt_temp;
#else
    float fA = A;
	return lrintf(sqrtf(fA));
#endif
}
