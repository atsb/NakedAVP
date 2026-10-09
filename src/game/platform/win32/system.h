#ifndef SYSTEM_INCLUDED
#define SYSTEM_INCLUDED

/*   AVP - WIN95

 Project Specific System Equates etc.

*/


#ifdef __cplusplus

extern "C" {

#endif



/********************* SYSTEM, PLATFORM AND GAME************/

#define Yes 1
#define No 0

#ifdef _DEBUG /* standard compiler command line debugging-ON switch */
	#define debug Yes
#elif defined(NDEBUG) /* standard compiler command line debugging-OFF switch */
	#define debug No
#else /* default switch */
	#define debug Yes
#endif

#define SuppressWarnings 	Yes
							
#define Term -1


/********************  General *****************************/

#define GlobalScale 1



#define one14 16383

#define ONE_FIXED 65536
#define ONE_FIXED_SHIFT 16

#define Digital	No


/* Offsets from *** int pointer *** for vectors and vertices */

typedef enum {

	ix,
	iy,
	iz

} PTARRAYINDICES;

#define StopCompilationOnMultipleInclusions No
#define UseProjPlatAssert                   Yes /* assert fired functions are in dxlog.c */


/***************  CAMERA AND VIEW VOL********************/
#define NearZ 	1024
#define FarZ 	ONE_FIXED

#define SupportMultiCamModules	Yes


/************* Timer and Frame Rate Independence *************/

#define TimerFrame		1000	
#define NormalFrame ONE_FIXED
#define NormalFrameShift ONE_FIXED_SHIFT


/***************** Angles  and VALUES ******************/

#define deg10 114
#define deg22pt5 256
#define deg45 512
#define deg90 1024
#define deg180 2048
#define deg270 3072
#define deg315 3584
#define deg337pt5 3840
#define deg350 3980
#define deg360 4096
#define wrap360 4095

#define Cosine45 		46341		/* 46340.95001 cosine(45deg)*/

#define bigint 1<<30		/*  max int size*/
#define smallint -(bigint)	/* smallest int size*/


/****************** BUFFER SIZES **********************/

#define maxvdbs 1
#define maxobjects 750
extern int maxshapes;
#define maxstblocks 1000

#define maxrotpts 10000

//tuning for morph sizes
#define maxmorphPts 1024

#define maxpolys 4000
#define maxpolyptrs maxpolys
#define maxpolypts 9			/* Translates as number of vectors */

#define vsize 3					/* Scale for polygon vertex indices */

#define MaxImages 400
#define MaxImageGroups 1


/************** Some Shell and Loading Platform Compiler Options ******************/

#undef RIFF_SYSTEM
#define RIFF_SYSTEM
#define TestRiffLoaders Yes
#define LoadingMapsShapesAndTexturesEtc		No

#define pc_backdrops						No


/***************** DRAW SORT *******************/

#define SupportTrackOptimisation			No


#define SupportBSP						 	No

#define SupportZBuffering				Yes
#define ZBufferTest							No


/***************** SHAPE DATA DEFINES************/

#define StandardShapeLanguage						Yes

#define SupportModules 									Yes
#define IncludeModuleFunctionPrototypes	Yes

#define SupportMorphing									Yes
#define LazyEvaluationForMorphing				No


/***************** COLLISION DEFINES*************/
#define StandardStrategyAndCollisions		No
#define IntermediateSSACM	No		/* User preference */



/************** TEXTURE DEFINES*******************/

#define maxTxAnimblocks	100

/* Texture usage of the colour int */

#define TxDefn 16				/* Shift up for texture definition index */
#define TxLocal 0x8000			/* Set bit 15 to signify a local index */
#define ClrTxIndex 0xffff0000	/* AND with this to clear the low 16-bits */
#define ClrTxDefn 0x0000ffff	/* AND with this to clear the high 16-bits */

/*
 3d textures
 This defines the amount by which we can scale up U/Z, V/Z & 1/Z
 It is defined in terms of the maximum UV size and the size of an int
 A value of 10 gives a scale of 31 - 10 = 21
 1/Z, the critical value, reaches 0 at 2^21 = 2km
 Since we know that z STARTS at no less than 2^8, we can increase this value
 by 8, giving 2^29
 1/Z now reaches 0 at 2^29 = 537km
*/

#define support3dtextures					Yes
#define int3dtextures						No /* there is no D3D Zbuffer support for int 3d textures */
#define SupportGouraud3dTextures  			Yes


/*************************** WINDOWS 95 *********************/

#define SUPPORT_MMX 0

#define MaxD3DInstructions 1000 // includes state change instructions!!!
#define MaxD3DVertices     256

#define optimiseflip No /* unstable at present */
#define optimiseblit Yes /* unstable at present */


#ifdef __cplusplus
	
	};

#endif


#endif
