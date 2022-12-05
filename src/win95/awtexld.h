#ifndef _INCLUDED_AWTEXLD_H_
#define _INCLUDED_AWTEXLD_H_

#ifdef __cplusplus
	extern "C" {
	#define _AWTL_DEFAULTPARM(v) = (v)
#else /* ! __cplusplus */
	#define _AWTL_DEFAULTPARM(v)
#endif /* ? __cplusplus */

#ifdef _MSC_VER
	#define _AWTL_VARARG __cdecl
#else
	#define _AWTL_VARARG
#endif

#include "aw.h"

/******************************/
/* return codes & error codes */
/******************************/

typedef
enum AwTlErc
{
	/* General Errors */
	  AW_TLE_OK /* apparent success */
	, AW_TLE_DXERROR /* unexpected DirectX error - see awTlLastDxErr */
	, AW_TLE_BADPARMS /* parameters passed to function were invalid, or requested unsupported functionality */
	, AW_TLE_NOINIT /* initialization functions have not been successfully called */
	/* File reading errors */
	, AW_TLE_CANTOPENFILE /* file open failed - see awTlLastWinErr for the Windows error code */
	, AW_TLE_CANTREADFILE /* unexpected error reading file - see awTlLastWinErr for the Windows error code */
	, AW_TLE_EOFMET /* unexpected end of file encountered */
	, AW_TLE_BADFILEFORMAT /* file format identifier not recognized */
	, AW_TLE_BADFILEDATA /* file data not consistent */
	/* Conversion errors */
	, AW_TLE_CANTPALETTIZE /* texture format is palettized; file data is not */
	, AW_TLE_IMAGETOOLARGE /* image size is larger in one or both dimensions than maximum texture size */
	, AW_TLE_CANTRELOAD /* loading a new texture into an existing surface failed because the existing surface is an unsuitable size, etc. */
}
	AW_TL_ERC;


/*********/
/* Flags */
/*********/

enum
{
	  AW_TLF_DEFAULT     = 0x00000000U /* no flags set */
	, AW_TLF_TRANSP      = 0x00000001U /* src data has transparency */
	, AW_TLF_PREVSRC     = 0x00000002U /* in AwRestoreTexture, use previously stored source data flags (AW_TLF_TRANSP only) */
	, AW_TLF_COMPRESS    = 0x00000004U /* use ALLOCONLOAD flag */
	, AW_TLF_CHROMAKEY   = 0x00000008U /* use chroma keying for transparency when the texture format has an alpha channel */
	, AW_TLF_VIDMEM      = 0x00000010U /* use Video memory for surfaces which are not textures */
	, AW_TLF_PREVSRCALL  = 0x00000020U /* in AwRestoreTexture, use ALL previously stored flags, except AW_TLF_CHECKLOST and AW_TLF_SKIPNOTLOST */
	, AW_TLF_TEXTURE     = 0x00000040U /* in AwCreateSurface, create a surface in the texture format with the texture flag set */
	, AW_TLF_MINSIZE     = 0x00000080U /* with the 'a' option, ensure all surfaces/textures created are at least as big as the rectangle specified even if the rect is partially off the image */
	, AW_TLF_CHECKLOST   = 0x00000100U /* checks for lost surfaces and calls restore on them */
	, AW_TLF_SKIPNOTLOST = 0x00000200U /* if the above flag also is specified, does not bother trying to restore surfaces which weren't lost */
	
	, _AW_TLF_FORCE32BITENUM = 0x0fffffffU /* probably entirely unnecessary */
};

/* SBF - alt_tab junk */
#define ATIncludeSurfaceDb(p, d, s) fprintf(stderr, "ATIncludeSurfaceDb: %s/%d: %s\n", __FILE__, __LINE__, s)
#define ATIncludeTextureDb(p, d, s) fprintf(stderr, "ATIncludeTextureDb: %s/%d: %s\n", __FILE__, __LINE__, s)
void ATIncludeSurface(DDSurface * pSurface, AW_BACKUPTEXTUREHANDLE hBackup);
void ATRemoveSurface(DDSurface * pSurface);
void ATRemoveTexture(D3DTexture * pTexture);


extern AW_TL_ERC awTlLastErr;

extern D3DTexture * AwCreateTexture(char const * _argFormatS, ...);
extern DDSurface * AwCreateSurface(char const * _argFormatS, ...);

extern AW_TL_ERC AwDestroyBackupTexture(AW_BACKUPTEXTUREHANDLE _bH);


typedef int (* AW_TL_PFN_CALLBACK) (void *);

/* Structure for receiving specific regions of an image in a surface or texture.
 * A pointer to an array of thise structures is passed to the AwCreate...
 * functions if the 'a' format specifier is used. The fields 'left', 'right',
 * 'top' and 'bottom' specify the rectangle to cut out of the image being loaded
 * and must be valid. In AwCreateSurface, the 'pSurface' field is used and is a
 * pointer to the Direct Draw surface created; in AwCreateTexture, the
 * 'pTexture' field is used and is a pointer to the Direct 3D texture created.
 * If an error occurs all the pointers in the array will be set to NULL. The
 * 'width' and 'height' fields will be filled in with the width and height of
 * the surface or texture that is created. If the rectangle specified is
 * completely outsided the main image, the width and height will be set to zero,
 * and the pointer field will be set to NULL, but this does not constitute an
 * error. If the 't' option is used, the pointer fields are assumed to be valid
 * textures or surfaces into which to load the new textures or surfaces. If the
 * pointer is NULL, the structure is ignored. The pointers will remain unchanged
 * even in the event of an error or a rectangle specified outside the main
 * image, though the width and height will still be set to zero.
 */
struct AwCreateGraphicRegion
{
	unsigned left, top, right, bottom; /* rectangle to cut from the original image */
	unsigned width, height; /* width and height of the resulting surface or texture */
#if 0
	union /* DDSurface or D3DTexture pointer depending on the context used */
	{
		DDSurface * pSurface; /* Direct Draw Surface object pointer */
		D3DTexture * pTexture; /* Direct 3D Texture object pointer */
	};
#endif	
};

/* typedef to save typing 'struct' when not using C++ */
typedef struct AwCreateGraphicRegion AW_CREATEGRAPHICREGION;

extern char const * AwTlErrorToString(AW_TL_ERC _AWTL_DEFAULTPARM(awTlLastErr));

#if 0

#include <windows.h>
#include <d3d.h>

/*********************************************/
/* Note:                                     */
/*                                           */
/* This header file contains typedefs for    */
/*  DDObject                                 */
/*  D3DDevice                                */
/*  D3DTexture                               */
/*  D3DSurface                               */
/* and #defines for their corresponding IIDs */
/*  GUID_DD_SURFACE                          */
/*  GUID_D3D_TEXTURE                         */
/*                                           */
/* They are currently typedeffed as follows  */
/*  typedef IDirectDraw2        DDObject;    */
/*  typedef IDirect3DDevice     D3DDevice;   */
/*  typedef IDirect3DTexture    D3DTexture;  */
/*  typedef IDirect3DSurface    DDSurface;   */
/*********************************************/
#include "aw.h"

/***********************************************************************************/
/* awTexLd.h - Author: Jake Hotson                                                 */
/*                                                                                 */
/* Loading of textures into D3D surfaces                                           */
/*	requires DirectX 5 or later                                                    */
/*	requires db.c and db.h                                                         */
/*	also requires                                                                  */
/*		Watcom C++ 11.0 or later                                                   */
/*		or Microsoft Visual C++ 5.0 or later                                       */
/*		or another suitable compiler                                               */
/*                                                                                 */
/* 	Load any known file format                                                     */
/* 		Microsoft bitmap (.BMP)                                                    */
/* 		OS/2 1.x bitmap (.BMP)                                                     */
/* 		OS/2 2.x bitmap (.BMP)                                                     */
/* 		Portable Bitmap (.PBM)                                                     */
/* 		Portable Graymap (.PGM)                                                    */
/* 		Portable Pixelmap (.PPM)                                                   */
/* 	                                                                               */
/* 	Load from various media                                                        */
/* 		File name                                                                  */
/* 		Windows file handle                                                        */
/* 		Raw file data in memory                                                    */
/* 	                                                                               */
/* 	Load into any given texture format provided a conversion is possible           */
/* 		Pad to the required size that the driver will accept                       */
/*                                                                                 */
/* 	Optional handling of transparency                                              */
/* 		Use alpha bits if available                                                */
/* 		Otherwise chroma keying to be used                                         */
/* 		Transparency colour is RGB(0,0,0) for non palettized data                  */
/* 		Or index 0 in palettized data                                              */
/* 		                                                                           */
/* 	Optional storing of processed raw data                                         */
/* 		Use internal format independent of DirectX                                 */
/* 		Function to reload from this data into a possibly different texture format */
/* 		                                                                           */
/* 	Five optional levels of diagnostics possible                                   */
/*		1. Logs when a function returns because of an error                        */
/*		2. Logs the nature of the error                                            */
/*		3. Logs immediately when an error occurs                                   */
/*		4. Logs information as data is parsed, and when main functions are called  */
/*		5. Logs success of every small subsection of code                          */
/* 	                                                                               */
/* 	Maximum error reporting                                                        */
/*                                                                                 */
/* 	Optimal Loading Speed                                                          */
/***********************************************************************************/

/* Version 2.1 */
#define AW_TL_VERSION 210 /* Preprocessor constant can be used to determine the version of this code */

/*
Version History:
----------------
version AW_TL_VERSION
0.9     090           Incomplete & May still contain bugs
0.91    091           AW_TLF_CHROMAKEY flag, and 't' option supported. Fewer bugs
0.92    092           Slightly more debug code
0.93    093           Using typedefs DDObject, D3DDevice, D3DTexture
1.0     100           Assumed to be bug-free
1.1     110           Internal redesign with interface to support any file format
1.2     120           Allow for loading into Direct Draw surfaces which are not textures
1.21    121           Two extra format flags to get width and height of actual image (as opposed to DD/D3D surface)
1.22    122           Split up the initialization function AwSetD3DDevice() into DD part and D3D part
1.3     130           New function: AwGetTextureSize()
1.31    131           New format flag for a callback function for when video mem runs out
1.4     140           Support for loading image split into several surfaces/textures and for loading surfaces as textures
1.5     150           Multiple texture format support

2.0     200           Beginning support for DX 6.0 - new AwSet..Format functions take LPDDPIXELFORMAT not LPDDSURFACEDESC
2.1     210           AW_TLF_CHECKLOST and AW_TLF_SKIPNOTLOST flags added
*/

/**********************************************/
/* Handle C++ linkage and optional parameters */
/**********************************************/

#ifdef __cplusplus
	extern "C" {
	#define _AWTL_DEFAULTPARM(v) = (v)
#else /* ! __cplusplus */
	#define _AWTL_DEFAULTPARM(v)
#endif /* ? __cplusplus */

#ifdef _MSC_VER
	#define _AWTL_VARARG __cdecl
#else
	#define _AWTL_VARARG
#endif

/******************************/
/* return codes & error codes */
/******************************/

typedef
enum AwTlErc
{
	/* General Errors */
	  AW_TLE_OK /* apparent success */
	, AW_TLE_DXERROR /* unexpected DirectX error - see awTlLastDxErr */
	, AW_TLE_BADPARMS /* parameters passed to function were invalid, or requested unsupported functionality */
	, AW_TLE_NOINIT /* initialization functions have not been successfully called */
	/* File reading errors */
	, AW_TLE_CANTOPENFILE /* file open failed - see awTlLastWinErr for the Windows error code */
	, AW_TLE_CANTREADFILE /* unexpected error reading file - see awTlLastWinErr for the Windows error code */
	, AW_TLE_EOFMET /* unexpected end of file encountered */
	, AW_TLE_BADFILEFORMAT /* file format identifier not recognized */
	, AW_TLE_BADFILEDATA /* file data not consistent */
	/* Conversion errors */
	, AW_TLE_CANTPALETTIZE /* texture format is palettized; file data is not */
	, AW_TLE_IMAGETOOLARGE /* image size is larger in one or both dimensions than maximum texture size */
	, AW_TLE_CANTRELOAD /* loading a new texture into an existing surface failed because the existing surface is an unsuitable size, etc. */
}
	AW_TL_ERC;

extern AW_TL_ERC awTlLastErr;
extern HRESULT awTlLastDxErr;
extern DWORD awTlLastWinErr;

#ifdef NDEBUG
	#define AwTlErrorToString ThisIsADebugFunction! /* generate compiler error */
	#define AwDxErrorToString ThisIsADebugFunction! /* generate compiler error */
	#define AwWinErrorToString ThisIsADebugFunction! /* generate compiler error */
#else /* ! NDEBUG */
	extern char const * AwTlErrorToString(AW_TL_ERC _AWTL_DEFAULTPARM(awTlLastErr));
	extern char const * AwDxErrorToString(HRESULT _AWTL_DEFAULTPARM(awTlLastDxErr));
	extern char const * AwWinErrorToString(DWORD _AWTL_DEFAULTPARM(awTlLastWinErr));
#endif /* ? NDEBUG */

/*********/
/* Flags */
/*********/

enum
{
	  AW_TLF_DEFAULT     = 0x00000000U /* no flags set */
	, AW_TLF_TRANSP      = 0x00000001U /* src data has transparency */
	, AW_TLF_PREVSRC     = 0x00000002U /* in AwRestoreTexture, use previously stored source data flags (AW_TLF_TRANSP only) */
	, AW_TLF_COMPRESS    = 0x00000004U /* use ALLOCONLOAD flag */
	, AW_TLF_CHROMAKEY   = 0x00000008U /* use chroma keying for transparency when the texture format has an alpha channel */
	, AW_TLF_VIDMEM      = 0x00000010U /* use Video memory for surfaces which are not textures */
	, AW_TLF_PREVSRCALL  = 0x00000020U /* in AwRestoreTexture, use ALL previously stored flags, except AW_TLF_CHECKLOST and AW_TLF_SKIPNOTLOST */
	, AW_TLF_TEXTURE     = 0x00000040U /* in AwCreateSurface, create a surface in the texture format with the texture flag set */
	, AW_TLF_MINSIZE     = 0x00000080U /* with the 'a' option, ensure all surfaces/textures created are at least as big as the rectangle specified even if the rect is partially off the image */
	, AW_TLF_CHECKLOST   = 0x00000100U /* checks for lost surfaces and calls restore on them */
	, AW_TLF_SKIPNOTLOST = 0x00000200U /* if the above flag also is specified, does not bother trying to restore surfaces which weren't lost */
	
	, _AW_TLF_FORCE32BITENUM = 0x0fffffffU /* probably entirely unnecessary */
};

/*********/
/* Types */
/*********/

/* a callback function */
typedef int (* AW_TL_PFN_CALLBACK) (void *);

/* Structure for receiving specific regions of an image in a surface or texture.
 * A pointer to an array of thise structures is passed to the AwCreate...
 * functions if the 'a' format specifier is used. The fields 'left', 'right',
 * 'top' and 'bottom' specify the rectangle to cut out of the image being loaded
 * and must be valid. In AwCreateSurface, the 'pSurface' field is used and is a
 * pointer to the Direct Draw surface created; in AwCreateTexture, the
 * 'pTexture' field is used and is a pointer to the Direct 3D texture created.
 * If an error occurs all the pointers in the array will be set to NULL. The
 * 'width' and 'height' fields will be filled in with the width and height of
 * the surface or texture that is created. If the rectangle specified is
 * completely outsided the main image, the width and height will be set to zero,
 * and the pointer field will be set to NULL, but this does not constitute an
 * error. If the 't' option is used, the pointer fields are assumed to be valid
 * textures or surfaces into which to load the new textures or surfaces. If the
 * pointer is NULL, the structure is ignored. The pointers will remain unchanged
 * even in the event of an error or a rectangle specified outside the main
 * image, though the width and height will still be set to zero.
 */
struct AwCreateGraphicRegion
{
	unsigned left, top, right, bottom; /* rectangle to cut from the original image */
	unsigned width, height; /* width and height of the resulting surface or texture */
	union /* DDSurface or D3DTexture pointer depending on the context used */
	{
		DDSurface * pSurface; /* Direct Draw Surface object pointer */
		D3DTexture * pTexture; /* Direct 3D Texture object pointer */
	};
};

/* typedef to save typing 'struct' when not using C++ */
typedef struct AwCreateGraphicRegion AW_CREATEGRAPHICREGION;

/********************/
/* Public functions */
/********************/

/* AwSetD3DDevice(DDObject * _ddP, D3DDevice * _d3ddeviceP)
	
	Description:
		Tells the texture loaders about the DirectDraw and
		Direct3D setup. You must call this function before
		loading any textures, and also every time you change
		the DirectDraw or Direct3D device setup.
		As of v1.22 this function is overloaded and only
		available to C++ programmers.
		
	Parameters:
		_ddP
			Pointer to the DirectDraw object obtained from
			a call to DirectDrawCreate().
		_d3ddeviceP
			Pointer to the Direct3DDevice object obtained
			from its GUID which is the lpGuid parameter in
			a call to a D3DENUMDEVICESCALLBACK function for
			your chosen driver.
			
	Returns:
		AW_TLE_OK if the function completed successfully;
		AW_TLE_BADPARMS if the parameters passed to the
			function were incorrect
		AW_TLE_DXERROR if a DirectX SDK call failed
*/
#ifdef __cplusplus
	extern "C++" AW_TL_ERC AwSetD3DDevice(DDObject * _ddP, D3DDevice * _d3ddeviceP);
#endif

/* AwSetDDObject(DDObject * _ddP)
   AwSetD3DDevice(D3DDevice * _d3ddeviceP)

	Description:
		These functions together do what the two parameter
		version of AwSetD3DDevice(), above, does. You
		needn't call AwSetD3DDevice() if you are only
		loading direct draw surfaces. The parameters and
		return values are as described above.
*/

extern AW_TL_ERC AwSetD3DDevice(D3DDevice * _d3ddeviceP);
extern AW_TL_ERC AwSetDDObject(DDObject * _ddP);

/* AwSetTextureFormat2(LPDDPIXELFORMAT _ddpfP)

	Description:
		Informs the texture loaders of the currently slected
		texture format. You must call this function before
		loading any textures, and also every time you change
		the texture format.
		
	Parameters:
		_ddpfP
			Pointer to a DDPIXELFORMAT structure which
			describes the texture format.
			
	Returns:
		AW_TLE_OK if the function completed successfully;
		AW_TLE_BADPARMS if the parameters passed to the
			function were incorrect
*/
extern AW_TL_ERC AwSetTextureFormat2(LPDDPIXELFORMAT _ddpfP);
#define AwSetTextureFormat(_descP) (AwSetTextureFormat2((_descP) ? &(_descP)->ddpfPixelFormat : NULL))

/* AwSetAdditionalTextureFormat2(LPDDPIXELFORMAT _ddpfP, unsigned _maxAlphaBits, int _canDoTransp, unsigned _maxColours)

	Description:
		Informs the texture loaders an additional texture
		format that should be used in certain cases. After
		a call to AwSetTextureFormat, there are no
		additional formats. Each additional format that you
		want to make available should be set with a call to
		this function. When a texture is being loaded, the
		additional formats are considered in the order that
		they were set (ie. the order in which the calls to
		this function were made). For each format
		considered, if it could be used for the image and
		the image's specification passes the tests for this
		format, the format will replace the previously
		chosen format (either the base format set by
		AwSetTextureFormat or an earlier additional format
		whose conditions of use were also met).
		
	Parameters:
		_ddpfP
			Pointer to a DDPIXELFORMAT structure which
			describes the texture format.
		_maxAlphaBits
			Specifies that this format should only be used
			for images that do not have more bits of alpha
			information than this value.
		_canDoTransp
			If zero, specifies that this format should not
			be used for images which have a transparent
			colour.
		_maxColours
			Specifies that this format should only be used
			for images that do not contain more unique
			colours than this value. If this value is zero,
			the format can be used for images with any
			number of colours. If this value is non-zero,
			this format will not be used for images whose
			number of unique colours cannot be determined.
						
	Returns:
		AW_TLE_OK if the function completed successfully;
		AW_TLE_NOINIT if AwSetTextureFormat was not
			previously called.
		AW_TLE_BADPARMS if the parameters passed to the
			function were incorrect
*/
extern AW_TL_ERC AwSetAdditionalTextureFormat2(LPDDPIXELFORMAT _ddpfP, unsigned _maxAlphaBits, int _canDoTransp, unsigned _maxColours);
#define AwSetAdditionalTextureFormat(_descP, _maxAlphaBits, _canDoTransp, _maxColours) (AwSetAdditionalTextureFormat2((_descP) ? &(_descP->ddpfPixelFormat) : NULL,_maxAlphaBits,_canDoTransp,_maxColours))

/* AwSetSurfaceFormat2(LPDDPIXELFORMAT _ddpfP)

	Description:
		As for AwSetTextureFormat but tells AwCreateSurface()
		what format surfaces for blitting should be in
*/
extern AW_TL_ERC AwSetSurfaceFormat2(LPDDPIXELFORMAT _ddpfP);
#define AwSetSurfaceFormat(_descP) (AwSetSurfaceFormat2((_descP) ? &(_descP)->ddpfPixelFormat : NULL))

/* AwGetTextureSize(unsigned * _widthP, unsigned * _heightP, unsigned _width, unsigned _height)

	Description:
		Calculates the size required for a texture on the
		current driver, given a minimum size required to
		hold the proposed image.
		
	Parameters:
		_widthP
			Pointer to a variable which will receive the
			width required for a Direct 3D texture.
		_heightP
			Pointer to a variable which will receive the
			height.
		_width
			Minimum width required for the proposed image.
		_height
			Minimum height required.
			
	Returns:
		AW_TLE_OK if the required Direct 3D texture size was
			calculated;
		AW_TLE_IMAGETOOLARGE if the driver specifies a
			maximum texture size which is less than the size
			that would be required to hold an image of the
			proposed size;
		AW_TLE_NOINIT if driver information was unavailable
			because initialization functions weren't called.
*/
extern AW_TL_ERC AwGetTextureSize(unsigned * _widthP, unsigned * _heightP, unsigned _width, unsigned _height);

/* AwCreateTexture(char const * _argFormatS, ...)

	Description:
		Creates a Direct3D texture for use by the device
		renderer. The exact functionality is determined by
		the first argument which is a format specifier
		string.
		
	Parameters:
		_argFormatS
			Pointer to a null terminated string which
			specifies the order, types and interpretations
			of the other parameters. Each character in the
			string corresponds to a parameter passed to	the
			function, and the parameters are parsed in the
			same order as their format specifiers appear in
			the string. For clarity, upper case letters are
			used to indicate paramters which are pointers to
			data that will be filled in by the function, and
			lower case letters are used for parameters which
			are simply data to be used by the function.
			
			The following specifiers are permitted:
			
		s	The next argument is of type 'LPCTSTR' and is a
			pointer to a ASCII or UNICODE string which is
			the filename of an image file to load.
		h	The next argument is of type 'HANDLE' and is a
			Windows file handle with its file pointer set to
			the start of an image file's data.
		p	The next argument is of type 'void const *' and
			is a pointer to an image file's data in memory.
		r	The next argument is of type
			'AW_BACKUPTEXTUREHANDLE' and is used to restore
			a previously loaded texture, possibly with the
			texture format or Direct3D device having
			changed.
		x	The next argument is of type 'unsigned' and is
			the maximum number of bytes that should be read
			from a file or memory. If parsing the file data
			would cause more bytes to be read than this
			value, the function fails and the error is
			'AW_TLE_EOFMET'. This may be useful in
			conjunction with format specifier 'm' in order
			to prevent a general protection (GP) fault.
		N	The next argument is of type 'unsigned *' and
			points to a variable which will receive the
			number of bytes which are actually read from a
			file or memory. This value will be filled in
			even if the function fails (unless the error is
			'AW_TLE_BADPARMS').
		f	The next argument is of type 'unsigned' and can
			be any combination of 'AW_TLF_...' flags (above)
			which control special functionality.
		W	The next argument is of type 'unsigned *' and
			points to a variable which will receive the
			width of the texture that is created.
		H	The next argument is of type 'unsigned *' and
			points to a variable which will receive the
			height of the texture that is created.
		X	The next argument is of type 'unsigned *' and
			points to a variable which will receive the
			width of the original image.
		Y	The next argument is of type 'unsigned *' and
			points to a variable which will receive the
			height of the original image.
		B	The next argument is of type
			'AW_BACKUPTEXTUREHANDLE *' and points to a
			variable which will receive a handle that can
			later be used with format specifier 'r' in a
			call to this function in order to avoid
			reloading from a file an image that has
			previously been loaded. This value will be
			filled in only if the function succeeds. When
			you no longer need this handle, you should
			deallocate the memory associated with it by
			calling 'AwDestroyBackupTexture()'.
		t	The next argument is of type 'D3DTexture *'
			and is used to load the new texture into an
			existing texture surface. The parameter is NOT
			Release()d; you should do this yourself. This is
			only guaranteed to work if the new texture is of
			the same height and pitch as the previous one
			and compression is not used on either.
		c	The next two arguments indicate a callback
			function which will be called if a texture cannot
			be created due to insufficient video memory. The
			idea is to allow the callee to free some video
			memory and request further attempts at creating
			the texture. The first of the two arguments is of
			type AW_TL_PFN_CALLBACK and is a function pointer
			to the function that would be called. This
			function takes one parameter of type 'void *'
			which is an implementation-specific value, and
			returns type 'int', which is non-zero if another
			attempt at creating the texture should be made.
			If the callback function returns zero, then the
			texture load will be aborted. The second of the
			two arguments is of type 'void *' and is the
			implementation-specific value passed to the
			callback function.
		a	The next two arguments indicate an array of
			rectangles to cut out of the original image and
			create textures for each rectangle. The first of
			these two arguments is of type 'unsigned' and is
			the size of the array. The second argument is of
			type 'AW_CREATEGRAPHICREGION *' and points to an
			array of AwCreateGraphicRegion structs, each
			indicating a region to cut out of the original
			image and create a texture for. In this case, the
			function always returns NULL, and you should test
			for failure by examining awTlLastErr. The 't'
			option, if used, must appear after 'a' in the
			format string, and does not correspond to any
			parameter, since the texture into which to
			reload will be specified for each region in
			the array of structures. The 'W' and 'H' options
			will be ignored. For more information, see the
			definition of this structure above.
			
			There are some restrictions on parameters:
			
			Exactly one of 's', 'h', 'p' or 'r' must be
			used.
			
			Neither 'x' nor 'N' may be used with 'r'.
			
			'r' cannot be used with 'B'.
			
			Neither 'W' or 'H' should be used with 'a'.
			
			Each specifier should be used only once.
			
			't' may not appear before 'a'.
			
			There are no other restriction on the order
			that the parameters occur.
			
	Returns:
		A valid D3DTexture * if the function succeeds;
			awTlLastErr will also be AW_TLE_OK
		NULL if the function fails. awTlLastErr will be
			AW_TLE_BADPARMS if the parameters were
			incorrect, or any other error code if the
			parameters were correct but another error
			occurred.
*/
extern D3DTexture * _AWTL_VARARG AwCreateTexture(char const * _argFormatS, ...);

/* AwCreateSurface(char const * _argFormatS, ...)

	Description:
		As for AwCreateTexture(), but returns a pointer to
		a newly created direct draw surface which is not a
		texture
		
	Parameters:
		_argFormatS
			As AwCreateTexture(), except
			
		t	The next argument is of type 'DDSurface *'
		
*/
extern DDSurface * _AWTL_VARARG AwCreateSurface(char const * _argFormatS, ...);

/* AwDestroyBackupTexture(AW_BACKUPTEXTUREHANDLE _bH)

	Description:
		Deallocates the memory associated with a backup
		texture handle, created by a call to
		'AwCreateTexture()' with a 'B' specifier. The handle
		cannot be used after this.
		
	Parameters:
		_bH
			The handle whose associated memory should be
			deallocated.

	Returns:
		AW_TLE_OK if the function succeeds.
		AW_TLE_BADPARMS if the handle was not valid.
*/
extern AW_TL_ERC AwDestroyBackupTexture(AW_BACKUPTEXTUREHANDLE _bH);

#endif










/* End Wrappers */
#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* ! _INCLUDED_AWTEXLD_H_ */
