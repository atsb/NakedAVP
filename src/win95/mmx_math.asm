; want 8-byte alignment really!!
_DATA SEGMENT DWORD PUBLIC 'DATA'


	PUBLIC _use_mmx_math
	PUBLIC _mmx_sign_mask
	PUBLIC _mmx_one_fixed_h
	
	align
	_mmx_sign_mask:QWORD 0000800000008000h
	_mmx_one_fixed_h:QWORD 0001000000000000h
	_mmx_one_fixed_hl:QWORD 0001000000010000h
	_mmx_one_hl:QWORD 0000000100000001h
	store1:QWORD ?
	_use_mmx_math:DWORD 1



_DATA ENDS



; want 16-byte alignment really!!
_TEXT SEGMENT DWORD PUBLIC 'CODE'
      ASSUME cs:_TEXT, ds:_DATA

.586

	PUBLIC MMXAsm_VectorDot_
	PUBLIC MMXAsm_VectorDot16_
	PUBLIC MMXAsm_VectorTransformed_
	PUBLIC MMXAsm_VectorTransform_
	PUBLIC MMXAsm_VectorTransformedAndAdd_
	PUBLIC MMXAsm_VectorTransformAndAdd_

	PUBLIC _MMXAsm_VectorDot
	PUBLIC _MMXAsm_VectorDot16
	PUBLIC _MMXAsm_VectorTransformed
	PUBLIC _MMXAsm_VectorTransform
	PUBLIC _MMXAsm_VectorTransformedAndAdd
	PUBLIC _MMXAsm_VectorTransformAndAdd
	
	align 
_MMXAsm_VectorDot:
MMXAsm_VectorDot_:

if 0
	; This is the unoptimized version
	
	; get the data
	movq mm0,[edx]
	movq mm1,[eax]
	movd mm2,[edx+08h]
	movd mm3,[eax+08h]
	
	
	; get it into signed fixed format
	movq mm4,mm0
	movq mm5,mm1
	movq mm6,mm2
	movq mm7,mm3
	
	pand mm4,_mmx_sign_mask
	pand mm5,_mmx_sign_mask
	pand mm6,_mmx_sign_mask
	pand mm7,_mmx_sign_mask
	
	paddd mm4,mm4
	paddd mm5,mm5
	paddd mm6,mm6
	paddd mm7,mm7
	
	paddd mm0,mm4
	paddd mm1,mm5
	paddd mm2,mm6
	paddd mm3,mm7
	
	; at this point we have split all 32 bit values
	; into 16-bit pairs, high and low, both signed
	
	; mm0: y1h y1l x1h x1l
	; mm1: y2h y2l x2h x2l
	; mm2:  0   0  z1h z1l
	; mm3:  0   0  z2h z2l
	
	; swap 1st and 2nd words in mm0,mm1,mm2,mm3 ??
	movq mm4,mm2
	movq mm5,mm3
	punpcklwd mm4,mm0
	; mm4: x1h z1h x1l z1l
	punpcklwd mm5,mm1
	; mm5: x2h z2h x2l z2l
	punpckhwd mm2,mm0
	; mm2: y1h  0  y1l  0
	punpckhwd mm3,mm1
	; mm3: y2h  0  y2l  0
	
	; get the high and low products: x1h*x2h, x1l*x2l, etc
	movq mm0,mm2
	pmaddwd mm0,mm3
	; mm0:     y1h*y2h         y1l*y2l
	movq mm1,mm4
	pmaddwd mm1,mm5
	; mm1: x1h*x2h+z1h*z2h x1l*x2l+z1l*z2l
	
	; exchange dwords in mm3 and mm5
	movq mm6,mm3
	movq mm7,mm5
	psrlq mm3,32
	psrlq mm5,32
	punpckldq mm3,mm6
	punpckldq mm5,mm7
	; mm5: x2l z2l x2h z2h
	; mm3: y2l  0  y2h  0
	
	; compute the products x1h*x2l, x1l*x2h, etc
	pmaddwd mm2,mm3
	; mm2:     y1h*y2l         y1l*y2h
	pmaddwd mm4,mm5
	; mm4: x1h*x2l+z1h*z2l x1l*x2h+z1l*z2h
	
	paddd mm2,mm4
	; mm2: x1h*x2l+y1h*y2l+z1h*z2l x1l*x2h+y1l*y2h+z1l*z2h
	
	; get the low order dwords of mm0,mm1
	movq mm3,mm0
	punpckldq mm0,mm1
	; mm0: x1l*x2l+z1l*z2l     y1l*y2l

	; unfortunately, at this point it is possible to have the
	; wrong value in mm0: if x1l,x2l,x1l,x2l
	; are all -0x8000, the result should
	; be +0x80000000, but of course this becomes
	; -0x80000000
	; in fact the largest +ve value we could have is
	; +0x80000000
	; and the lowest -ve value we could have is
	; -0x7fff0000
	; = 0x80010000
	; so subtracting ONE at this stage gives us a value
	; which is out by ONE, but twos-complement correct
	psubd mm0,_mmx_one_fixed_h
	
	; and the high order dwords
	punpckhdq mm1,mm3
	; mm1: x1h*x2h+z1h*z2h     y1h*y2h
	; in fact it is swapped, but it doesn't matter
	
	; shift the low order dwords down
	psrad mm0,16
	; and the high order dwords up
	pslld mm1,16
	; mm0: x1l*x2l+z1l*z2l>>16 -1  y1l*y2l>>16
	; mm1: x1h*x2h+z1h*z2h<<16     y1h*y2h<<16
	;(mm2) x1h*x2l+y1h*y2l+z1h*z2l x1l*x2h+y1l*y2h+z1l*z2h
	
	; sum up
	paddd mm2,mm0
	paddd mm2,mm1
	movq mm1,mm2
	psrlq mm2,32
	paddd mm1,mm2
	movd eax,mm1
	
	emms
	inc eax
	ret

else
	;
	; Now the optimized version

	movq mm0,[edx]

	movd mm2,[edx+08h]
	movq mm4,mm0


	pand mm4,_mmx_sign_mask
	movq mm6,mm2

	movq mm1,[eax]
	paddd mm4,mm4

	movd mm3,[eax+08h]
	movq mm5,mm1

	pand mm6,_mmx_sign_mask
	movq mm7,mm3

	pand mm5,_mmx_sign_mask
	paddd mm6,mm6

	pand mm7,_mmx_sign_mask
	paddd mm5,mm5

	paddd mm0,mm4
	paddd mm2,mm6
	
	paddd mm7,mm7
	movq mm4,mm2

	punpcklwd mm4,mm0
	paddd mm1,mm5

	punpckhwd mm2,mm0
	paddd mm3,mm7

	movq mm5,mm3
	punpckhwd mm3,mm1

	punpcklwd mm5,mm1
	movq mm0,mm2

	movq mm1,mm4
	pmaddwd mm0,mm3

	movq mm6,mm3
	psrlq mm3,32

	movq mm7,mm5
	punpckldq mm3,mm6

	pmaddwd mm1,mm5
	psrlq mm5,32

	punpckldq mm5,mm7
	pmaddwd mm2,mm3

	pmaddwd mm4,mm5
	movq mm3,mm0

	; these instructions won't pair and I have no instructions I can pair them with
	punpckldq mm0,mm1

	psubd mm0,_mmx_one_fixed_h
	punpckhdq mm1,mm3

	psrad mm0,16
	paddd mm2,mm4

	pslld mm1,16
	paddd mm2,mm0

	; complete pairing is not possible at this stage - there are too many dependencies
	paddd mm2,mm1
	
	movq mm1,mm2
	psrlq mm2,32
	
	paddd mm1,mm2
	
	movd eax,mm1

	emms

	inc eax
	ret

endif

	; This takes 33 cycles, the orignal C -> nonMMX version takes 80 cycles

	align
_MMXAsm_VectorDot16:
MMXAsm_VectorDot16_:

	movd mm0,[edx+08h]

	packssdw mm0,[edx]

	movd mm1,[eax+08h]

	packssdw mm1,[eax]

	pmaddwd mm0,mm1

	movq mm1,mm0
	psrlq mm0,32

	paddd mm0,mm1

	movd eax,mm0

	emms

	ret
	; taking 14 cycles but assuming 16bit input vector fields


	align
_MMXAsm_VectorTransformed:
MMXAsm_VectorTransformed_:

if 0
	; eax ptr to result
	; edx ptr to vector xh, xl, yh, yl, zh, zl
	; ecx ptr to matrix a11h, a11l, a12h, etc

	; unoptimized version

	; NOTE: in the Dot Product there was a problem
	; of an internal overflow where -32768*-32768 + -32768*-32768 gave 0x80000000
	; which is -ve in two's complement
	; the additions and subtractions of ONE to resolve this problem
	; are marked '******'
	
	movq mm0,[edx]
	movq mm1,mm0
	pand mm1,_mmx_sign_mask
	paddd mm1,mm1
	paddd mm0,mm1
	; mm0: yh yl xh xl
	
	movq mm2,[ecx]
	movq mm3,mm2
	pand mm3,_mmx_sign_mask
	paddd mm3,mm3
	paddd mm2,mm3
	; mm2: a21h a21l a11h a11l
	
	movd mm4,[edx+08h]
	movq mm5,mm4
	pand mm5,_mmx_sign_mask
	paddd mm5,mm5
	paddd mm4,mm5
	; mm4: 0 0 zh zl
	
	movq mm6,[ecx+18h]
	movq mm7,mm6
	pand mm7,_mmx_sign_mask
	paddd mm7,mm7
	paddd mm6,mm7
	; mm6: a23h a23l a13h a13l
	
	; interleave
	
	movq mm1,mm0
	punpckhwd mm0,mm4
	; mm0: 0 yh 0 yl
	punpcklwd mm1,mm4
	; mm1: zh xh zl xl
	
	movq mm3,mm2
	punpckhwd mm2,mm6
	; mm2: a23h a21h a23l a21l
	punpcklwd mm3,mm6
	; mm3: a13h a11h a13l a11l
	
	; get a13*z, a11*x; a23*z a21*x, high and low products
	movq mm4,mm1
	pmaddwd mm1,mm2
	movq mm6,mm4
	pmaddwd mm4,mm3
	; mm0: 0 yh 0 yl
	; mm6: zh xh zl xl
	; mm2: a23h a21h a23l a21l
	; mm3: a13h a11h a13l a11l
	; mm1: zh*a23h+xh*a21h zl*a23l+xl*a21l
	; mm4: zh*a13h+xh*a11h zl*a13l+xl*a11l
	
	; exchange dwords in mm6
	movq mm7,mm6
	psrlq mm6,32
	punpckldq mm6,mm7
	; mm6: zl xl zh xh
	; mm7: zh xh zl xl
	
	; get the high-low 'cross' products
	pmaddwd mm2,mm6
	pmaddwd mm3,mm6
	; mm2: a23h*zl+a21h*xl a23l*zh+a21l*xh
	; mm3: a13h*zl+a11h*xl a13l*zh+a11l*xh
	
	; interleave mm1,mm4 and mm2,mm3
	movq mm5,mm4
	punpckldq mm4,mm1
	punpckhdq mm5,mm1
	; mm4: zl*a23l+xl*a21l zl*a13l+xl*a11l ******
	; mm5: zh*a23h+xh*a21h zh*a13h+xh*a11h

	; ******
	psubd mm4,_mmx_one_fixed_hl


	movq mm1,mm3
	punpckldq mm3,mm2
	punpckhdq mm1,mm2
	; mm1: zl*a23h+xl*a21h zl*a13h+xl*a11h
	; mm3: zh*a23l+xh*a21l zh*a13l+xh*a11l
	; sum
	paddd mm1,mm3
	; shift the low order dwords down
	psrad mm4,16
	; and the high order dwords up
	pslld mm5,16
	; sum
	paddd mm1,mm4
	paddd mm1,mm5
	; mm1 holding x and y of the result
	; mm0: 0 yh 0 yl
	; mm1: z*a23+x*a21 z*a13+x*a11
	; mm2: 
	; mm3: 
	; mm4: 
	; mm5: 
	; mm6: zl xl zh xh
	; mm7: zh xh zl xl
	
	; grab some more of the matrix
	movq mm2,[ecx+08h]
	movq mm3,mm2
	pand mm3,_mmx_sign_mask
	paddd mm3,mm3
	paddd mm2,mm3 ; mm7 not mm2 in optimized version
	; mm2: a12h a12l a31h a31l
	
	movd mm4,[ecx+20h]
	movq mm5,mm4
	pand mm5,_mmx_sign_mask
	paddd mm5,mm5
	paddd mm4,mm5
	; mm4:   0    0  a33h a33l
	
	; interleave
	movq mm3,mm2
	punpcklwd mm2,mm4
	; mm2: a33h a31h a33l a31l
	psrlq mm3,32
	; mm3:  0     0  a12h a12l
	
	; compute mm2 * mm6/7
	movq mm4,mm2
	pmaddwd mm2,mm7
	pmaddwd mm4,mm6
	; mm2: a33h*zh+a31h*xh a33l*zl+a31l*xl ******
	; mm4: a33h*zl+a31h*xl a33l*zh+a31l*xh
	movq mm7,mm2

	; ******
	psubd mm7,_mmx_one_fixed_hl

	pslld mm2,16
	psrad mm7,16
	paddd mm2,mm4
	paddd mm7,mm4
	psrlq mm2,32
	paddd mm2,mm7
	; mm2:   ?  a33*z+a31*x
	
	
	
	; get the rest of the matrix
	movq mm5,[ecx+010h]
	movq mm6,mm5
	pand mm6,_mmx_sign_mask
	paddd mm6,mm6
	paddd mm5,mm6
	; mm5: a32h a32l a22h a22l
	; mm3:   0   0   a12h a12l
	
	; mm0: 0 yh 0 yl
	movq mm7,mm0
	psrlq mm0,32
	punpcklwd mm0,mm7
	; mm0: 0 0 yl yh
	punpckldq mm0,mm0
	
	; mm0: yl   yh   yl   yh
	movq mm7,mm0
	pmaddwd mm0,mm3
	movq mm6,mm7
	pmaddwd mm7,mm5
	; mm0:       0         yl*a12h+yh*a12l
	; mm7: yl*a32h+yh*a32l yl*a22h+yh*a22l
	; mm6: yl   yh   yl   yh
	punpckldq mm0,mm7
	; mm0: yl*a22h+yh*a22l yl*a12h+yh*a12l
	paddd mm1,mm0
	; mm1: z*a23+x*a21+yl*a22h+yh*a22l z*a13+x*a11+yl*a12h+yh*a12l
	psrlq mm7,32
	paddd mm2,mm7
	; mm2:   ?  a33*z+a31*x+yl*a32h+yh*a32l
	
	
	
	; mm5: a32h a32l a22h a22l
	; mm3:   0   0   a12h a12l
	; mm6:  yl   yh   yl   yh
	
	
	
	; get all h and l separate
	movq mm4,mm3
	punpcklwd mm3,mm5
	; mm3: a22h a12h a22l a12l
	punpckhwd mm5,mm4
	; mm5:   0  a32h  0   a32l
	movq mm4,mm3
	punpckhdq mm3,mm5
	; mm3:   0  a32h a22h a12h
	punpckldq mm4,mm5
	; mm4:   0  a32l a22l a12l
	punpckhwd mm6,mm6
	; mm6:  yl   yl   yh   yh
	movq mm0,mm6
	punpckhdq mm6,mm6
	; mm6:  yl  yl  yl  yl
	punpckldq mm0,mm0
	; mm0:  yh  yh  yh  yh
	pmullw mm3,mm0
	pmulhw mm4,mm6
	; mm3:  0 a32h*yh     a22h*yh     a12h*yh
	; mm4:  0 a32l*yl>>16 a22l*yl>>16 a12l*yl>>16
	pxor mm7,mm7
	pcmpgtw mm7,mm4
	paddw mm3,mm7

	movq mm5,mm4
	punpcklwd mm4,mm3
	punpckhwd mm5,mm3
	paddd mm1,mm4
	paddd mm2,mm5
	
	; ******
	paddd mm1,_mmx_one_hl
	paddd mm2,_mmx_one_hl
	
	movq [eax],mm1
	movd [eax+08h],mm2
	
	emms
	ret

else
	;
	; optimized version

	movq mm0,[edx]

	movd mm4,[edx+08h]
	movq mm1,mm0

	movq mm2,[ecx]
	movq mm5,mm4

	pand mm1,_mmx_sign_mask
	movq mm3,mm2

	pand mm5,_mmx_sign_mask
	paddd mm1,mm1

	movq mm6,[ecx+18h]
	paddd mm5,mm5

	pand mm3,_mmx_sign_mask
	movq mm7,mm6

	paddd mm0,mm1
	paddd mm3,mm3

	pand mm7,_mmx_sign_mask
	paddd mm2,mm3

	movq mm1,mm0
	punpckhwd mm0,mm4

	paddd mm4,mm5
	paddd mm7,mm7

	paddd mm6,mm7
	punpcklwd mm1,mm4

	movq mm3,mm2
	punpckhwd mm2,mm6

	punpcklwd mm3,mm6
	movq mm4,mm1

	movq mm6,mm1
	pmaddwd mm4,mm3

	movq mm7,mm6
	psrlq mm6,32

	pmaddwd mm1,mm2
	punpckldq mm6,mm7

	movq store1,mm7
	pmaddwd mm3,mm6

	movq mm7,[ecx+08h]
	pmaddwd mm2,mm6

	movq mm5,mm4
	punpckldq mm4,mm1

	psubd mm4,_mmx_one_fixed_hl
	punpckhdq mm5,mm1

	movq mm1,mm7
	psrad mm4,16

	pand mm1,_mmx_sign_mask
	pslld mm5,16

	paddd mm1,mm1
	paddd mm5,mm4

	paddd mm7,mm1
	movq mm1,mm3

	movd mm4,[ecx+20h]
	punpckldq mm3,mm2

	paddd mm3,mm5
	movq mm5,mm4

	pand mm5,_mmx_sign_mask
	punpckhdq mm1,mm2

	paddd mm1,mm3
	paddd mm5,mm5

	movq mm2,[ecx+010h]
	movq mm3,mm7

	paddd mm4,mm5
	movq mm5,mm2

	pand mm2,_mmx_sign_mask
	punpcklwd mm7,mm4

	movq mm4,mm7
	psrlq mm3,32

	pmaddwd mm7,store1
	paddd mm2,mm2

	pmaddwd mm4,mm6
	movq mm6,mm0

	psrlq mm0,32
	paddd mm5,mm2

	punpcklwd mm0,mm6
	movq mm2,mm7
	
	psubd mm7,_mmx_one_fixed_hl
	pslld mm2,16
	
	psrad mm7,16
	paddd mm2,mm4

	paddd mm7,mm4
	punpckldq mm0,mm0

	movq mm6,mm0
	psrlq mm2,32

	paddd mm2,mm7
	movq mm7,mm6

	pmaddwd mm0,mm3
	punpckhwd mm7,mm7

	pmaddwd mm6,mm5
	movq mm4,mm3

	punpcklwd mm3,mm5

	punpckhwd mm5,mm4
	movq mm4,mm7

	punpckldq mm0,mm6

	paddd mm1,mm0
	punpckhdq mm7,mm7

	movq mm0,mm3
	punpckldq mm3,mm5

	pmulhw mm3,mm7
	punpckhdq mm0,mm5

	punpckldq mm4,mm4

	pmullw mm0,mm4
	psrlq mm6,32

	paddd mm2,mm6
	pxor mm6,mm6

	pcmpgtw mm6,mm3
	movq mm5,mm3

	paddd mm1,_mmx_one_hl
	paddw mm0,mm6

	paddd mm2,_mmx_one_hl
	punpcklwd mm3,mm0

	paddd mm1,mm3
	punpckhwd mm5,mm0

	paddd mm2,mm5

	movq [eax],mm1

	movd [eax+08h],mm2
	
	emms
	ret
	; 63 cycles compared with 204 for the C-nonMMX version
endif

	align
_MMXAsm_VectorTransform:
MMXAsm_VectorTransform_:

	movq mm0,[eax]

	movd mm4,[eax+08h]
	movq mm1,mm0

	movq mm2,[edx]
	movq mm5,mm4

	pand mm1,_mmx_sign_mask
	movq mm3,mm2

	pand mm5,_mmx_sign_mask
	paddd mm1,mm1

	movq mm6,[edx+18h]
	paddd mm5,mm5

	pand mm3,_mmx_sign_mask
	movq mm7,mm6

	paddd mm0,mm1
	paddd mm3,mm3

	pand mm7,_mmx_sign_mask
	paddd mm2,mm3
	
	movq mm1,mm0
	punpckhwd mm0,mm4
	
	paddd mm4,mm5
	paddd mm7,mm7

	paddd mm6,mm7
	punpcklwd mm1,mm4
	
	movq mm3,mm2
	punpckhwd mm2,mm6

	punpcklwd mm3,mm6
	movq mm4,mm1

	movq mm6,mm1
	pmaddwd mm4,mm3

	movq mm7,mm6
	psrlq mm6,32

	pmaddwd mm1,mm2
	punpckldq mm6,mm7
	
	movq store1,mm7
	pmaddwd mm3,mm6

	movq mm7,[edx+08h]
	pmaddwd mm2,mm6

	movq mm5,mm4
	punpckldq mm4,mm1

	psubd mm4,_mmx_one_fixed_hl
	punpckhdq mm5,mm1

	movq mm1,mm7
	psrad mm4,16

	pand mm1,_mmx_sign_mask
	pslld mm5,16

	paddd mm1,mm1
	paddd mm5,mm4

	paddd mm7,mm1
	movq mm1,mm3

	movd mm4,[edx+20h]
	punpckldq mm3,mm2

	paddd mm3,mm5
	movq mm5,mm4

	pand mm5,_mmx_sign_mask
	punpckhdq mm1,mm2

	paddd mm1,mm3
	paddd mm5,mm5

	movq mm2,[edx+010h]
	movq mm3,mm7

	paddd mm4,mm5
	movq mm5,mm2

	pand mm2,_mmx_sign_mask
	punpcklwd mm7,mm4

	movq mm4,mm7
	psrlq mm3,32

	pmaddwd mm7,store1
	paddd mm2,mm2

	pmaddwd mm4,mm6
	movq mm6,mm0

	psrlq mm0,32
	paddd mm5,mm2

	punpcklwd mm0,mm6
	movq mm2,mm7

	psubd mm7,_mmx_one_fixed_hl
	pslld mm2,16

	psrad mm7,16
	paddd mm2,mm4
	
	paddd mm7,mm4
	punpckldq mm0,mm0
	
	movq mm6,mm0
	psrlq mm2,32

	paddd mm2,mm7
	movq mm7,mm6

	pmaddwd mm0,mm3
	punpckhwd mm7,mm7
	
	pmaddwd mm6,mm5
	movq mm4,mm3

	punpcklwd mm3,mm5

	punpckhwd mm5,mm4
	movq mm4,mm7

	punpckldq mm0,mm6

	paddd mm1,mm0
	punpckhdq mm7,mm7

	movq mm0,mm3
	punpckldq mm3,mm5

	pmulhw mm3,mm7
	punpckhdq mm0,mm5
	
	punpckldq mm4,mm4

	pmullw mm0,mm4
	psrlq mm6,32

	paddd mm2,mm6
	pxor mm6,mm6
	
	pcmpgtw mm6,mm3
	movq mm5,mm3

	paddd mm1,_mmx_one_hl
	paddw mm0,mm6

	paddd mm2,_mmx_one_hl
	punpcklwd mm3,mm0

	paddd mm1,mm3
	punpckhwd mm5,mm0

	paddd mm2,mm5

	movq [eax],mm1

	movd [eax+08h],mm2
	
	emms
	ret
	; 63 cycles compared with 204 for the C-nonMMX version


	align
_MMXAsm_VectorTransformedAndAdd:
MMXAsm_VectorTransformedAndAdd_:

	movq mm0,[edx]

	movd mm4,[edx+08h]
	movq mm1,mm0

	movq mm2,[ecx]
	movq mm5,mm4

	pand mm1,_mmx_sign_mask
	movq mm3,mm2

	pand mm5,_mmx_sign_mask
	paddd mm1,mm1

	movq mm6,[ecx+18h]
	paddd mm5,mm5

	pand mm3,_mmx_sign_mask
	movq mm7,mm6

	paddd mm0,mm1
	paddd mm3,mm3

	pand mm7,_mmx_sign_mask
	paddd mm2,mm3
	
	movq mm1,mm0
	punpckhwd mm0,mm4
	
	paddd mm4,mm5
	paddd mm7,mm7

	paddd mm6,mm7
	punpcklwd mm1,mm4
	
	movq mm3,mm2
	punpckhwd mm2,mm6

	punpcklwd mm3,mm6
	movq mm4,mm1

	movq mm6,mm1
	pmaddwd mm4,mm3

	movq mm7,mm6
	psrlq mm6,32

	pmaddwd mm1,mm2
	punpckldq mm6,mm7
	
	movq store1,mm7
	pmaddwd mm3,mm6

	movq mm7,[ecx+08h]
	pmaddwd mm2,mm6

	movq mm5,mm4
	punpckldq mm4,mm1

	psubd mm4,_mmx_one_fixed_hl
	punpckhdq mm5,mm1
	
	movq mm1,mm7
	psrad mm4,16

	pand mm1,_mmx_sign_mask
	pslld mm5,16

	paddd mm1,mm1
	paddd mm5,mm4

	paddd mm7,mm1
	movq mm1,mm3

	movd mm4,[ecx+20h]
	punpckldq mm3,mm2

	paddd mm3,mm5
	movq mm5,mm4

	pand mm5,_mmx_sign_mask
	punpckhdq mm1,mm2

	paddd mm1,mm3
	paddd mm5,mm5

	movq mm2,[ecx+010h]
	movq mm3,mm7

	paddd mm4,mm5
	movq mm5,mm2

	pand mm2,_mmx_sign_mask
	punpcklwd mm7,mm4

	movq mm4,mm7
	psrlq mm3,32

	pmaddwd mm7,store1
	paddd mm2,mm2

	pmaddwd mm4,mm6
	movq mm6,mm0

	psrlq mm0,32
	paddd mm5,mm2

	punpcklwd mm0,mm6
	movq mm2,mm7

	psubd mm7,_mmx_one_fixed_hl
	pslld mm2,16
	
	psrad mm7,16
	paddd mm2,mm4
	
	paddd mm7,mm4
	punpckldq mm0,mm0
	
	movq mm6,mm0
	psrlq mm2,32

	paddd mm2,mm7
	movq mm7,mm6

	pmaddwd mm0,mm3
	punpckhwd mm7,mm7
	
	pmaddwd mm6,mm5
	movq mm4,mm3

	paddd mm1,_mmx_one_hl
	punpcklwd mm3,mm5

	punpckhwd mm5,mm4
	movq mm4,mm7

	paddd mm2,_mmx_one_hl
	punpckldq mm0,mm6

	paddd mm1,mm0
	punpckhdq mm7,mm7

	movq mm0,mm3
	punpckldq mm3,mm5

	pmulhw mm3,mm7
	punpckhdq mm0,mm5
	
	paddd mm1,[ebx]
	punpckldq mm4,mm4

	pmullw mm0,mm4
	psrlq mm6,32

	paddd mm2,mm6
	pxor mm6,mm6
	
	pcmpgtw mm6,mm3
	movq mm5,mm3

	movd mm4,[ebx+08h]
	paddw mm0,mm6

	paddd mm2,mm4
	punpcklwd mm3,mm0

	paddd mm1,mm3
	punpckhwd mm5,mm0

	paddd mm2,mm5

	movq [eax],mm1

	movd [eax+08h],mm2
	
	emms
	ret
	; 63 cycles compared with 204 for the C-nonMMX version


	align
_MMXAsm_VectorTransformAndAdd:
MMXAsm_VectorTransformAndAdd_:

	movq mm0,[eax]

	movd mm4,[eax+08h]
	movq mm1,mm0

	movq mm2,[edx]
	movq mm5,mm4

	pand mm1,_mmx_sign_mask
	movq mm3,mm2

	pand mm5,_mmx_sign_mask
	paddd mm1,mm1

	movq mm6,[edx+18h]
	paddd mm5,mm5

	pand mm3,_mmx_sign_mask
	movq mm7,mm6

	paddd mm0,mm1
	paddd mm3,mm3

	pand mm7,_mmx_sign_mask
	paddd mm2,mm3
	
	movq mm1,mm0
	punpckhwd mm0,mm4
	
	paddd mm4,mm5
	paddd mm7,mm7

	paddd mm6,mm7
	punpcklwd mm1,mm4
	
	movq mm3,mm2
	punpckhwd mm2,mm6

	punpcklwd mm3,mm6
	movq mm4,mm1

	movq mm6,mm1
	pmaddwd mm4,mm3

	movq mm7,mm6
	psrlq mm6,32

	pmaddwd mm1,mm2
	punpckldq mm6,mm7
	
	movq store1,mm7
	pmaddwd mm3,mm6

	movq mm7,[edx+08h]
	pmaddwd mm2,mm6

	movq mm5,mm4
	punpckldq mm4,mm1

	psubd mm4,_mmx_one_fixed_hl
	punpckhdq mm5,mm1
	
	movq mm1,mm7
	psrad mm4,16

	pand mm1,_mmx_sign_mask
	pslld mm5,16

	paddd mm1,mm1
	paddd mm5,mm4

	paddd mm7,mm1
	movq mm1,mm3

	movd mm4,[edx+20h]
	punpckldq mm3,mm2

	paddd mm3,mm5
	movq mm5,mm4

	pand mm5,_mmx_sign_mask
	punpckhdq mm1,mm2

	paddd mm1,mm3
	paddd mm5,mm5

	movq mm2,[edx+010h]
	movq mm3,mm7

	paddd mm4,mm5
	movq mm5,mm2

	pand mm2,_mmx_sign_mask
	punpcklwd mm7,mm4

	movq mm4,mm7
	psrlq mm3,32

	pmaddwd mm7,store1
	paddd mm2,mm2

	pmaddwd mm4,mm6
	movq mm6,mm0

	psrlq mm0,32
	paddd mm5,mm2

	punpcklwd mm0,mm6
	movq mm2,mm7

	psubd mm7,_mmx_one_fixed_hl
	pslld mm2,16
	
	psrad mm7,16
	paddd mm2,mm4
	
	paddd mm7,mm4
	punpckldq mm0,mm0
	
	movq mm6,mm0
	psrlq mm2,32

	paddd mm2,mm7
	movq mm7,mm6

	pmaddwd mm0,mm3
	punpckhwd mm7,mm7
	
	pmaddwd mm6,mm5
	movq mm4,mm3

	paddd mm1,_mmx_one_hl
	punpcklwd mm3,mm5

	punpckhwd mm5,mm4
	movq mm4,mm7

	paddd mm2,_mmx_one_hl
	punpckldq mm0,mm6

	paddd mm1,mm0
	punpckhdq mm7,mm7

	movq mm0,mm3
	punpckldq mm3,mm5

	pmulhw mm3,mm7
	punpckhdq mm0,mm5
	
	paddd mm1,[ecx]
	punpckldq mm4,mm4

	pmullw mm0,mm4
	psrlq mm6,32

	paddd mm2,mm6
	pxor mm6,mm6
	
	pcmpgtw mm6,mm3
	movq mm5,mm3

	movd mm4,[ecx+08h]
	paddw mm0,mm6

	paddd mm2,mm4
	punpcklwd mm3,mm0

	paddd mm1,mm3
	punpckhwd mm5,mm0

	paddd mm2,mm5

	movq [eax],mm1

	movd [eax+08h],mm2
	
	emms
	ret
	; 63 cycles compared with 204 for the C-nonMMX version


_TEXT	ENDS

END

