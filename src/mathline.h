#ifndef MATHLINE_H
#define MATHLINE_H

#include <math.h>

#define f2i(a, b) a = lrintf(b)

/*

 Fixed Point Multiply.


 16.16 * 16.16 -> 16.16
 or
 16.16 * 0.32 -> 0.32

 A proper version of this function ought to read
 16.16 * 16.16 -> 32.16
 but this would require a __int64 result

 Algorithm:

 Take the mid 32 bits of the 64 bit result

*/

/*
	These functions have been checked for suitability for 
	a Pentium and look as if they would work adequately.
	Might be worth a more detailed look at optimising
	them though.
*/

static __inline int MUL_FIXED(int a, int b)
{
/*
	int retval;
	_asm
	{
		mov eax,a
		imul b
		shrd eax,edx,16
		mov retval,eax
	}
*/

#if defined(ASM386)
	int retval;
__asm__("imull	%2			\n\t"
	"shrdl	$16, %%edx, %%eax	\n\t"
	: "=a" (retval)
	: "0" (a), "m" (b)
	: "%edx", "cc"
	);
	return retval;
#else
	__int64 aa = (__int64) a;
	__int64 bb = (__int64) b;
	
	__int64 cc = aa * bb;
	
	return (int) ((cc >> 16) & 0xffffffff);
#endif
}

#endif
