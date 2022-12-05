#include "fixer.h"

#ifndef DB_LEVEL
#define DB_LEVEL 4
#endif
#include "db.h"

#ifndef NDEBUG
	#define HT_FAIL db_log1
	#include "hash_tem.hpp" // for the backup surfaces memory leak checking
#endif

#include "iff.hpp"

#include "list_tem.hpp"

#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "media.hpp"
#include "awtexld.h"
#include "awtexld.hpp"

#ifdef _CPPRTTI
	#include <typeinfo.h>
#endif

/* awTexLd.cpp - Author: Jake Hotson */

/*****************************************/
/* Preprocessor switches for experiments */
/*****************************************/

#define MIPMAPTEST 0 // experiment to create mip map surfaces for textures, but doesn't bother putting any data into them

/*****************************/
/* DB_LEVEL dependent macros */
/*****************************/

#if DB_LEVEL >= 5
#define inline // prevent function inlining at level 5 debugging
#endif

/*****************************************************/
/* ZEROFILL and SETDWSIZE macros ensure that I won't */
/* accidentally get the parameters wrong             */
/*****************************************************/

#if 1 // which do you prefer?

// zero mem
template <class X>
static inline void ZEROFILL(X & x)
{
	memset(&x,0,sizeof(X));
}

// set dwSize
template <class X>
static inline void SETDWSIZE(X & x)
{
	x.dwSize = sizeof(X);
}

template <class X>
static inline void INITDXSTRUCT(X & x)
{
	ZEROFILL(x);
	SETDWSIZE(x);
}

#else

#define ZEROFILL(x) (memset(&x,0,sizeof x))
#define SETDWSIZE(x) (x.dwSize = sizeof x)
#define INITDXSTRUCT(x) (ZEROFILL(x),SETDWSIZE(x))

#endif

/*****************************************************************/
/* Put everything I can in a namespace to avoid naming conflicts */
/*****************************************************************/

namespace AwTl
{
	/**************************************************/
	/* Allow breakpoints to be potentially hard coded */
	/* into macros and template functions             */
	/**************************************************/
	
	db_code5(void BrkPt(){})
	#define BREAKPOINT db_code5(::AwTl::BrkPt();)
	
	#if DB_LEVEL > 4
	static unsigned GetRefCount(IUnknown * pUnknown)
	{
#if 0
		if (!pUnknown) return 0;
		pUnknown->AddRef();
		return static_cast<unsigned>(pUnknown->Release());
#endif
		return 0;
	}
	#endif
	
	/*********************************/
	/* Pixel format global structure */
	/*********************************/

	PixelFormat pixelFormat;
	
	PixelFormat pfTextureFormat;
	PixelFormat pfSurfaceFormat;
	
	static inline void SetBitShifts(unsigned * leftShift,unsigned * rightShift,unsigned mask)
	{
		if (!mask)
			*leftShift = 0;
		else
			for (*leftShift = 0; !(mask & 1); ++*leftShift, mask>>=1)
				;
		for (*rightShift = 8; mask; --*rightShift, mask>>=1)
			;
	}
		
	/************************************/
	/* D3D Driver info global structure */
	/************************************/

	static
	struct DriverDesc
	{
		DriverDesc() : validB(false), ddP(NULL) {}
		
		bool validB : 1;
		bool needSquareB : 1;
		bool needPow2B : 1;
		
		unsigned minWidth;
		unsigned minHeight;
		unsigned maxWidth;
		unsigned maxHeight;
		
		DWORD memFlag;
		
		void * ddP;
	} driverDesc;

	/*************************************************************************/
	/* Class used to hold all the parameters for the CreateTexture functions */
	/*************************************************************************/
	
	class CreateTextureParms
	{
		public:
			inline CreateTextureParms()
				: loadTextureB(false)
				, fileNameS(NULL)
				, fileH(NULL)
				, dataP(NULL)
				, restoreH(NULL)
				, maxReadBytes(UINT_MAX)
				, bytesReadP(NULL)
				, flags(AW_TLF_DEFAULT)
				, widthP(NULL)
				, heightP(NULL)
				, originalWidthP(NULL)
				, originalHeightP(NULL)	
				, prevTexP(static_cast<D3DTexture *>(NULL))
				, prevTexB(false)
				, callbackF(NULL)
				, rectA(NULL)
			{
			}
			
			SurfUnion DoCreate() const;
			
			bool loadTextureB;
			
			char *fileNameS;
			FILE *fileH;
			PtrUnionConst dataP;
			AW_BACKUPTEXTUREHANDLE restoreH;
			
			unsigned maxReadBytes;
			unsigned * bytesReadP;
			
			unsigned flags;
			
			unsigned * widthP;
			unsigned * heightP;
			
			unsigned * originalWidthP;
			unsigned * originalHeightP;
			
			SurfUnion prevTexP;
			bool prevTexB; // used when rectA is non-NULL, otherwise prevTexP is used
			
			AW_TL_PFN_CALLBACK callbackF;
			void * callbackParam;
			
			unsigned numRects;
			AwCreateGraphicRegion * rectA;
	};
	

	/****************************************/
	/* Reference Count Object Debug Support */
	/****************************************/
	
	#ifndef NDEBUG
	
		static bool g_bAllocListActive = false;
		
		class AllocList : public ::HashTable<RefCntObj *>
		{
			public:
				AllocList()
				{
					g_bAllocListActive = true;
				}
				~AllocList()
				{
					if (Size())
					{
						db_log1(("AW: Potential Memory Leaks Detected!!!"));
					}
					#ifdef _CPPRTTI
						//#warning "Run-Time Type Identification (RTTI) is enabled"
						for (Iterator itLeak(*this) ; !itLeak.Done() ; itLeak.Next())
						{
							db_logf1(("\tAW Object not deallocated: Type: %s RefCnt: %u",typeid(*itLeak.Get()).name(),itLeak.Get()->m_nRefCnt));
						}
						if (Size())
						{
							db_log1(("AW: Object dump complete"));
						}
					#else // ! _CPPRTTI
						//#warning "Run-Time Type Identification (RTTI) is not enabled - memory leak checking will not report types"
						unsigned nRefs(0);
						for (Iterator itLeak(*this) ; !itLeak.Done() ; itLeak.Next())
						{
							nRefs += itLeak.Get()->m_nRefCnt;
						}
						if (Size())
						{
							db_logf1(("AW: Objects not deallocated: Number of Objects: %u Number of References: %u",Size(),nRefs));
						}
					#endif // ! _CPPRTTI
					g_bAllocListActive = false;
				}
		};
		
		static AllocList g_listAllocated;

		void DbRemember(RefCntObj * pObj)
		{
			g_listAllocated.AddAsserted(pObj);
		}
		
		void DbForget(RefCntObj * pObj)
		{
			if (g_bAllocListActive)
				g_listAllocated.RemoveAsserted(pObj);
		}
		
	#endif // ! NDEBUG
	
	/********************************************/
	/* structure to contain loading information */
	/********************************************/
	
	struct LoadInfo
	{
		DDSurface * surfaceP;
		bool surface_lockedB;
		DDSurface * dst_surfaceP;
		D3DTexture * textureP;
		D3DTexture * dst_textureP;
		
		unsigned surface_width;
		unsigned surface_height;
		PtrUnion surface_dataP;
		LONG surface_pitch;
		DWORD dwCapsCaps;
		
		unsigned * widthP;
		unsigned * heightP;
		SurfUnion prevTexP;
		SurfUnion resultP;
		unsigned top,left,bottom,right;
		unsigned width,height; // set to right-left and bottom-top
		
		AwCreateGraphicRegion * rectP;
		
		bool skipB; // used to indicate that a surface/texture was not lost and .`. does not need restoring
		
		LoadInfo()
			: surfaceP(NULL)
			, surface_lockedB(false)
			, dst_surfaceP(NULL)
			, textureP(NULL)
			, dst_textureP(NULL)
			, skipB(false)
		{
		}
	};
	
	/*******************************/
	/* additiional texture formats */
	/*******************************/
	
	struct AdditionalPixelFormat : PixelFormat
	{
		bool canDoTranspB;
		unsigned maxColours;
		
		// for List
		bool operator == (AdditionalPixelFormat const &) const { return false; }
		bool operator != (AdditionalPixelFormat const &) const { return true; }
	};
	
	static List<AdditionalPixelFormat> listTextureFormats;

} // namespace AwTl

/*******************/
/* Generic Loaders */
/*******************/
	
#define HANDLE_DXERROR(s) \
	if (DD_OK != awTlLastDxErr)	{ \
		awTlLastErr = AW_TLE_DXERROR; \
		db_logf3(("AwCreateGraphic() failed whilst %s",s)); \
		db_log1("AwCreateGraphic(): ERROR: DirectX SDK call failed"); \
		goto EXIT_WITH_ERROR; \
	} else { \
		db_logf5(("\tsuccessfully completed %s",s)); \
	}
	
#define ON_ERROR_RETURN_NULL(s) \
	if (awTlLastErr != AW_TLE_OK) { \
		db_logf3(("AwCreateGraphic() failed whilst %s",s)); \
		db_logf1(("AwCreateGraphic(): ERROR: %s",AwTlErrorToString())); \
		return static_cast<D3DTexture *>(NULL); \
	} else { \
		db_logf5(("\tsuccessfully completed %s",s)); \
	}
	
#define CHECK_MEDIA_ERRORS(s) \
	if (pMedium->m_fError) { \
		db_logf3(("AwCreateGraphic(): The following media errors occurred whilst %s",s)); \
		if (pMedium->m_fError & MediaMedium::MME_VEOFMET) { \
			db_log3("\tA virtual end of file was met"); \
			if (awTlLastErr == AW_TLE_OK) awTlLastErr = AW_TLE_EOFMET; \
		} \
		if (pMedium->m_fError & MediaMedium::MME_EOFMET) { \
			db_log3("\tAn actual end of file was met"); \
			if (awTlLastErr == AW_TLE_OK) awTlLastErr = AW_TLE_EOFMET; \
		} \
		if (pMedium->m_fError & MediaMedium::MME_OPENFAIL) { \
			db_log3("\tThe file could not be opened"); \
			if (awTlLastErr == AW_TLE_OK) { awTlLastErr = AW_TLE_CANTOPENFILE; awTlLastWinErr = GetLastError(); } \
		} \
		if (pMedium->m_fError & MediaMedium::MME_CLOSEFAIL) { \
			db_log3("\tThe file could not be closed"); \
			if (awTlLastErr == AW_TLE_OK) { awTlLastErr = AW_TLE_CANTOPENFILE; awTlLastWinErr = GetLastError(); } \
		} \
		if (pMedium->m_fError & MediaMedium::MME_UNAVAIL) { \
			db_log3("\tA requested operation was not available"); \
			if (awTlLastErr == AW_TLE_OK) { awTlLastErr = AW_TLE_CANTREADFILE; awTlLastWinErr = GetLastError(); } \
		} \
		if (pMedium->m_fError & MediaMedium::MME_IOERROR) { \
			db_log3("\tA read error occurred"); \
			if (awTlLastErr == AW_TLE_OK) { awTlLastErr = AW_TLE_CANTREADFILE; awTlLastWinErr = GetLastError(); } \
		} \
	}
	
AwTl::SurfUnion AwBackupTexture::Restore(AwTl::CreateTextureParms const & rParams)
{
	using namespace AwTl;

	ChoosePixelFormat(rParams);
	
	if (!pixelFormat.validB)
		db_log3("AwCreateGraphic(): ERROR: pixel format not valid");
	if (!driverDesc.ddP || (!driverDesc.validB && rParams.loadTextureB))
		db_log3("AwCreateGraphic(): ERROR: driver description not valid");
	
	awTlLastErr = pixelFormat.validB && driverDesc.ddP && (driverDesc.validB || !rParams.loadTextureB) ? AW_TLE_OK : AW_TLE_NOINIT;
	
	ON_ERROR_RETURN_NULL("initializing restore")
	
	OnBeginRestoring(pixelFormat.palettizedB ? 1<<pixelFormat.bitsPerPixel : 0);
	
	ON_ERROR_RETURN_NULL("initializing restore")
	
	SurfUnion pTex = CreateTexture(rParams);
	
	OnFinishRestoring(AW_TLE_OK == awTlLastErr ? true : false);
	
	return pTex;
}

bool AwBackupTexture::HasTransparentMask(bool bDefault)
{
	return bDefault;
}

DWORD AwBackupTexture::GetTransparentColour()
{
	return 0;
}

void AwBackupTexture::ChoosePixelFormat(AwTl::CreateTextureParms const & _parmsR)
{
//	fprintf(stderr, "AwBackupTexture::ChoosePixelFormat(...)\n");
	
	using namespace AwTl;
	
	pixelFormat.validB = false; // set invalid first
	
	// which flags to use?
	unsigned fMyFlags =
		_parmsR.flags & AW_TLF_PREVSRCALL ? db_assert1(_parmsR.restoreH), m_fFlags
		: _parmsR.flags & AW_TLF_PREVSRC ? db_assert1(_parmsR.restoreH),
			((_parmsR.flags & ~AW_TLF_TRANSP) | (m_fFlags & AW_TLF_TRANSP))
		: _parmsR.flags;
		
	// transparency?
	m_bTranspMask = HasTransparentMask(fMyFlags & AW_TLF_TRANSP ? true : false);

	if (_parmsR.loadTextureB || (fMyFlags & AW_TLF_TEXTURE))
	{
#if 0		
		// use a texture format
		unsigned nColours = GetNumColours();
		unsigned nMinPalSize = GetMinPaletteSize();
		
		PixelFormat * pFormat = &pfTextureFormat;
		
		for (LIF<AdditionalPixelFormat> itFormat(&listTextureFormats); !itFormat.done(); itFormat.next())
		{
			AdditionalPixelFormat * pThisFormat = &itFormat();
			// is this format suitable?
			// ignoring alpha for now
			if
			(
				   (nMinPalSize <= 1U<<pThisFormat->bitsPerPixel && nMinPalSize || !pThisFormat->palettizedB) // few enough colours for palettized format
				&& (nColours <= pThisFormat->maxColours && nColours || !pThisFormat->maxColours) // pass the max colours test
				&& (pThisFormat->canDoTranspB || !m_bTranspMask) // pass the transparency test
			)
			{
				pFormat = pThisFormat;
			}
		}

		pixelFormat = *pFormat;

		#if DB_LEVEL >= 4
		if (pixelFormat.palettizedB)
		{
			db_logf4(("\tchosen %u-bit palettized texture format",pixelFormat.bitsPerPixel));
		}
		else
		{
			if (pixelFormat.alphaB)
			{
				unsigned alpha_l_shft,alpha_r_shft;
				SetBitShifts(&alpha_l_shft,&alpha_r_shft,pixelFormat.dwRGBAlphaBitMask);
			
				db_logf4(("\tchosen %u-bit %u%u%u%u texture format",
					pixelFormat.bitsPerPixel,
					8U-pixelFormat.redRightShift,
					8U-pixelFormat.greenRightShift,
					8U-pixelFormat.blueRightShift,
					8U-alpha_r_shft));
			}
			else
			{
				db_logf4(("\tchosen %u-bit %u%u%u texture format",
					pixelFormat.bitsPerPixel,
					8U-pixelFormat.redRightShift,
					8U-pixelFormat.greenRightShift,
					8U-pixelFormat.blueRightShift));
			}
		}
		#endif
	}
	else
	{
		// use display surface format
		pixelFormat = pfSurfaceFormat;

#endif		
		/* Just convert the texture to 32bpp */
		// may want to support paletted textures
		// at some point; at which point, should
		// push texture conversion into the opengl layer
		pixelFormat.palettizedB = 0;
		
		pixelFormat.alphaB = 1;
		pixelFormat.validB = 1;
		pixelFormat.texB = 1;
		pixelFormat.bitsPerPixel = 32;
		pixelFormat.redLeftShift = 0;
		pixelFormat.greenLeftShift = 8;
		pixelFormat.blueLeftShift = 16;
		pixelFormat.redRightShift = 0;
		pixelFormat.greenRightShift = 0;
		pixelFormat.blueRightShift = 0;
		pixelFormat.dwRGBAlphaBitMask = 0xFF000000;
	}
}

extern "C" {
extern int CreateOGLTexture(D3DTexture *, unsigned char *);
extern int CreateIMGSurface(D3DTexture *, unsigned char *);
};

AwTl::SurfUnion AwBackupTexture::CreateTexture(AwTl::CreateTextureParms const & _parmsR)
{
	using namespace AwTl;
	
	// which flags to use?
	unsigned fMyFlags =
	(_parmsR.flags & AW_TLF_PREVSRCALL) ? db_assert1(_parmsR.restoreH),
	(_parmsR.flags & (AW_TLF_CHECKLOST|AW_TLF_SKIPNOTLOST)) | (m_fFlags & ~(AW_TLF_CHECKLOST|AW_TLF_SKIPNOTLOST))
	: (_parmsR.flags & AW_TLF_PREVSRC) ? db_assert1(_parmsR.restoreH),
	((_parmsR.flags & ~AW_TLF_TRANSP) | (m_fFlags & AW_TLF_TRANSP))
	: _parmsR.flags;

	if (_parmsR.originalWidthP) *_parmsR.originalWidthP = m_nWidth;
	if (_parmsR.originalHeightP) *_parmsR.originalHeightP = m_nHeight;	
	
	if (_parmsR.rectA != NULL) {
		fprintf(stderr, "AwBackupTexture::CreateTexture - rectangle cutouts?\n");
	}
	
	if (pixelFormat.texB && (m_bTranspMask && (!pixelFormat.alphaB || fMyFlags & AW_TLF_CHROMAKEY))) {
		fprintf(stderr, "AwBackupTexture::CreateTexture - chroma\n");
	}
	
	if (pixelFormat.texB && m_bTranspMask) {
		//fprintf(stderr, "AwBackupTexture::CreateTexture - transparency\n");
	}

	// convert asset to 32-bit rgba
	// may want to support paletted textures
	// at some point; at which point, should
	// push texture conversion into the opengl layer

			unsigned char *buf = (unsigned char *)malloc(m_nWidth * m_nHeight * 4);
			
			Colour * paletteP = m_nPaletteSize ? GetPalette() : NULL;
			
			unsigned y = 0;
			bool reversed_rowsB = AreRowsReversed();
			if (reversed_rowsB)
			{
				y = m_nHeight-1;
			}
			
			for (int i = 0, rowcount = m_nHeight; rowcount; --rowcount, i++)
			{
				PtrUnion src_rowP = GetRowPtr(y);
				db_assert1(src_rowP.voidP);
				
				// allow loading of the next row from the file
				LoadNextRow(src_rowP);
				
				// loop for copying data to surfaces
				{
					
					{
						// are we in the vertical range of this surface?
						{
							
							// convert and copy the section of the row to the direct draw surface
//							ConvertRow(pLoadInfo->surface_dataP,pLoadInfo->surface_width,src_rowP,pLoadInfo->left,pLoadInfo->width,paletteP db_code1(DB_COMMA m_nPaletteSize));

							PtrUnion my_data = &buf[y*m_nWidth*4];
							
							ConvertRow(my_data,m_nWidth,src_rowP,0,m_nWidth,paletteP db_code1(DB_COMMA m_nPaletteSize));
							
						}
					}
				}
				
				// next row
				if (reversed_rowsB)
					--y;
				else
					++y;
				
			}
				
	// convert to texture
	D3DTexture *Tex = (D3DTexture *)calloc(1, sizeof(D3DTexture));

	Tex->w = m_nWidth;
	Tex->h = m_nHeight;
	Tex->hasAlpha = m_bTranspMask;
	Tex->hasChroma = m_fFlags & AW_TLF_CHROMAKEY;

	if (pixelFormat.texB) {
		CreateOGLTexture(Tex, buf);
	} else {
		CreateIMGSurface(Tex, buf); 
	}

	return static_cast<SurfUnion>(Tex);

#if 0
	
	// which flags to use?
	unsigned fMyFlags =
		_parmsR.flags & AW_TLF_PREVSRCALL ? db_assert1(_parmsR.restoreH),
			_parmsR.flags & (AW_TLF_CHECKLOST|AW_TLF_SKIPNOTLOST) | m_fFlags & ~(AW_TLF_CHECKLOST|AW_TLF_SKIPNOTLOST)
		: _parmsR.flags & AW_TLF_PREVSRC ? db_assert1(_parmsR.restoreH),
			_parmsR.flags & ~AW_TLF_TRANSP | m_fFlags & AW_TLF_TRANSP
		: _parmsR.flags;
		
	db_code1(ULONG refcnt;)
	
	DDPalette * dd_paletteP = NULL;
	LoadInfo * arrLoadInfo = NULL;
	unsigned nLoadInfos = 0;
	{
		// quick error check
		if (pixelFormat.palettizedB && (!m_nPaletteSize || 1U<<pixelFormat.bitsPerPixel < m_nPaletteSize))
			awTlLastErr = AW_TLE_CANTPALETTIZE;
		if (!m_nHeight || !m_nWidth)
			awTlLastErr = AW_TLE_BADFILEDATA;
		if (AW_TLE_OK != awTlLastErr)
		{
			db_log1("AwCreateGraphic() failed whilst interpreting the header data or palette");
			goto EXIT_WITH_ERROR;
		}
		
		if (_parmsR.originalWidthP) *_parmsR.originalWidthP = m_nWidth;
		if (_parmsR.originalHeightP) *_parmsR.originalHeightP = m_nHeight;
		
		if (_parmsR.rectA)
		{
			nLoadInfos = 0;
			arrLoadInfo = _parmsR.numRects ? new LoadInfo[_parmsR.numRects] : NULL; 
			for (unsigned i=0; i<_parmsR.numRects; ++i)
			{
				_parmsR.rectA[i].width = 0;
				_parmsR.rectA[i].height = 0;
				if
				(
					   _parmsR.rectA[i].top < m_nHeight
					&& _parmsR.rectA[i].left < m_nWidth
					&& (!_parmsR.prevTexB || (_parmsR.loadTextureB ? (_parmsR.rectA[i].pTexture != NULL) : (_parmsR.rectA[i].pSurface != NULL)))
					&& _parmsR.rectA[i].right > _parmsR.rectA[i].left
					&& _parmsR.rectA[i].bottom > _parmsR.rectA[i].top
				)
				{
					// rectangle covers at least some of the image and non-null previous texture
					arrLoadInfo[nLoadInfos].widthP = &_parmsR.rectA[i].width;
					arrLoadInfo[nLoadInfos].heightP = &_parmsR.rectA[i].height;
					if (_parmsR.prevTexB)
					{
						if (_parmsR.loadTextureB)
							arrLoadInfo[nLoadInfos].prevTexP = _parmsR.rectA[i].pTexture;
						else
							arrLoadInfo[nLoadInfos].prevTexP = _parmsR.rectA[i].pSurface;
					}
					else
					{
						arrLoadInfo[nLoadInfos].prevTexP = static_cast<D3DTexture *>(NULL);
						if (_parmsR.loadTextureB)
							_parmsR.rectA[i].pTexture = NULL;
						else
							_parmsR.rectA[i].pSurface = NULL;
					}
					
					arrLoadInfo[nLoadInfos].rectP = &_parmsR.rectA[i];
					arrLoadInfo[nLoadInfos].top = _parmsR.rectA[i].top;
					arrLoadInfo[nLoadInfos].left = _parmsR.rectA[i].left;
					arrLoadInfo[nLoadInfos].bottom = _parmsR.rectA[i].bottom;
					arrLoadInfo[nLoadInfos].right = _parmsR.rectA[i].right;
					
					if (arrLoadInfo[nLoadInfos].right > m_nWidth) arrLoadInfo[nLoadInfos].right = m_nWidth;
					if (arrLoadInfo[nLoadInfos].bottom > m_nHeight) arrLoadInfo[nLoadInfos].bottom = m_nHeight;
					
					arrLoadInfo[nLoadInfos].width = arrLoadInfo[nLoadInfos].right - arrLoadInfo[nLoadInfos].left;
					arrLoadInfo[nLoadInfos].height = arrLoadInfo[nLoadInfos].bottom - arrLoadInfo[nLoadInfos].top;

					++nLoadInfos;
				}
				else
				{
					if (!_parmsR.prevTexB)
						_parmsR.rectA[i].pTexture = NULL;
				}
			}
		}
		else
		{
			nLoadInfos = 1;
			arrLoadInfo = new LoadInfo[1];
			arrLoadInfo[0].widthP = _parmsR.widthP;
			arrLoadInfo[0].heightP = _parmsR.heightP;
			arrLoadInfo[0].prevTexP = _parmsR.prevTexP;
			arrLoadInfo[0].rectP = NULL;
			arrLoadInfo[0].top = 0;
			arrLoadInfo[0].left = 0;
			arrLoadInfo[0].bottom = m_nHeight;
			arrLoadInfo[0].right = m_nWidth;
			arrLoadInfo[0].width = m_nWidth;
			arrLoadInfo[0].height = m_nHeight;
		}
		
		bool bSkipAll = true;
		
		// loop creating surfaces
		{for (unsigned i=0; i<nLoadInfos; ++i)
		{
			LoadInfo * pLoadInfo = &arrLoadInfo[i];
			
			db_logf4(("\trectangle from image (%u,%u)-(%u,%u)",pLoadInfo->left,pLoadInfo->top,pLoadInfo->right,pLoadInfo->bottom));
			db_logf5(("\treference count on input surface %u",_parmsR.loadTextureB ? GetRefCount(pLoadInfo->prevTexP.textureP) : GetRefCount(pLoadInfo->prevTexP.surfaceP)));
			
			// determine what the width and height of the surface will be
			
			if (_parmsR.loadTextureB || fMyFlags & AW_TLF_TEXTURE)
			{
				awTlLastErr =
					AwGetTextureSize
					(
						&pLoadInfo->surface_width,
						&pLoadInfo->surface_height,
						fMyFlags & AW_TLF_MINSIZE && pLoadInfo->rectP ? pLoadInfo->rectP->right - pLoadInfo->rectP->left : pLoadInfo->width,
						fMyFlags & AW_TLF_MINSIZE && pLoadInfo->rectP ? pLoadInfo->rectP->bottom - pLoadInfo->rectP->top : pLoadInfo->height
					);
				if (awTlLastErr != AW_TLE_OK)
					goto EXIT_WITH_ERROR;
			}
			else
			{
				pLoadInfo->surface_width = fMyFlags & AW_TLF_MINSIZE && pLoadInfo->rectP ? pLoadInfo->rectP->right - pLoadInfo->rectP->left : pLoadInfo->width;
				pLoadInfo->surface_height = fMyFlags & AW_TLF_MINSIZE && pLoadInfo->rectP ? pLoadInfo->rectP->bottom - pLoadInfo->rectP->top : pLoadInfo->height;
				#if 1 // not sure if this is required...
				pLoadInfo->surface_width += 3;
				pLoadInfo->surface_width &= ~3;
				pLoadInfo->surface_height += 3;
				pLoadInfo->surface_height &= ~3;
				#endif
			}
		
			if (pLoadInfo->widthP) *pLoadInfo->widthP = pLoadInfo->surface_width;
			if (pLoadInfo->heightP) *pLoadInfo->heightP = pLoadInfo->surface_height;

			// Create DD Surface	
		
			DD_SURFACE_DESC ddsd;
			INITDXSTRUCT(ddsd);
			ddsd.ddpfPixelFormat = pixelFormat.ddpf;
			ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
			if (_parmsR.loadTextureB || fMyFlags & AW_TLF_TEXTURE)
				pLoadInfo->dwCapsCaps = DDSCAPS_TEXTURE | (fMyFlags & (AW_TLF_COMPRESS|AW_TLF_TEXTURE) ? DDSCAPS_SYSTEMMEMORY : driverDesc.memFlag);
			else
				pLoadInfo->dwCapsCaps = DDSCAPS_OFFSCREENPLAIN | (fMyFlags & AW_TLF_VIDMEM ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
			ddsd.ddsCaps.dwCaps = pLoadInfo->dwCapsCaps;
			ddsd.dwHeight = pLoadInfo->surface_height;
			ddsd.dwWidth = pLoadInfo->surface_width;
		
			if (pLoadInfo->prevTexP.voidP && (!_parmsR.loadTextureB || !(fMyFlags & AW_TLF_COMPRESS)))
			{
				if (_parmsR.loadTextureB)
					awTlLastDxErr = pLoadInfo->prevTexP.textureP->QueryInterface(GUID_DD_SURFACE,(LPVOID *)&pLoadInfo->surfaceP);
				else
					awTlLastDxErr = pLoadInfo->prevTexP.surfaceP->QueryInterface(GUID_DD_SURFACE,(LPVOID *)&pLoadInfo->surfaceP);
				HANDLE_DXERROR("getting direct draw surface interface")
				#if DB_LEVEL >= 5
				if (_parmsR.loadTextureB)
					db_logf5(("\t\tnow prev tex ref %u new surface i/f ref %u",GetRefCount(pLoadInfo->prevTexP.textureP),GetRefCount(pLoadInfo->surfaceP)));
				else
					db_logf5(("\t\tnow prev surf ref %u new surface i/f ref %u",GetRefCount(pLoadInfo->prevTexP.surfaceP),GetRefCount(pLoadInfo->surfaceP)));
				#endif
				
				// check for lost surfaces
				if (fMyFlags & AW_TLF_CHECKLOST)
				{
					awTlLastDxErr = pLoadInfo->surfaceP->IsLost();
					
					if (DDERR_SURFACELOST == awTlLastDxErr)
					{
						db_log4("\tRestoring Lost Surface");
						
						awTlLastDxErr = pLoadInfo->surfaceP->Restore();
					}
					else if (DD_OK == awTlLastDxErr && (fMyFlags & AW_TLF_SKIPNOTLOST))
					{
						db_log4("\tSkipping Surface which was not Lost");
						
						pLoadInfo->skipB = true;
					}
					
					HANDLE_DXERROR("testing for lost surface and restoring if necessary");
				}
				
				if (!pLoadInfo->skipB)
				{
					// check that the surface desc is OK
					// note that SetSurfaceDesc is *only* supported for changing the surface memory pointer
					DD_SURFACE_DESC old_ddsd;
					INITDXSTRUCT(old_ddsd);
					awTlLastDxErr = pLoadInfo->surfaceP->GetSurfaceDesc(&old_ddsd);
					HANDLE_DXERROR("getting previous surface desc")
					// check width, height, RGBBitCount and memory type
					if (old_ddsd.dwFlags & DDSD_ALL || (old_ddsd.dwFlags & (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT)) == (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT))
					{
						if (old_ddsd.dwHeight == pLoadInfo->surface_height && old_ddsd.dwWidth == pLoadInfo->surface_width && (old_ddsd.ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY|DDSCAPS_VIDEOMEMORY|DDSCAPS_TEXTURE)) == pLoadInfo->dwCapsCaps)
						{
							unsigned bpp = 0;
							if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
								bpp = 8;
							else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
								bpp = 4;
							else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
								bpp = 2;
							else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
								bpp = 1;
							else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
								bpp = old_ddsd.ddpfPixelFormat.dwRGBBitCount;
							if (pixelFormat.bitsPerPixel != bpp)
								awTlLastErr = AW_TLE_CANTRELOAD;	
						}
						else
							awTlLastErr = AW_TLE_CANTRELOAD;
					}
					else
						awTlLastErr = AW_TLE_CANTRELOAD;
					if (AW_TLE_OK != awTlLastErr)
					{
						db_log1("AwCreateGraphic() failed because existing surface is incompatible");
						goto EXIT_WITH_ERROR;
					}
				}
				else
				{
					pLoadInfo->surfaceP->Release();
					pLoadInfo->surfaceP = NULL;
				}
			}
			else
			{
				if (pLoadInfo->prevTexP.voidP && (fMyFlags & AW_TLF_CHECKLOST))
				{
					db_assert1(_parmsR.loadTextureB);
					
					awTlLastDxErr = pLoadInfo->prevTexP.textureP->QueryInterface(GUID_DD_SURFACE,(LPVOID *)&pLoadInfo->surfaceP);
					HANDLE_DXERROR("getting direct draw surface interface")
					
					db_logf5(("\t\tnow prev tex ref %u new surface i/f ref %u",GetRefCount(pLoadInfo->prevTexP.textureP),GetRefCount(pLoadInfo->surfaceP)));
					
					awTlLastDxErr = pLoadInfo->surfaceP->IsLost();
					
					if (DDERR_SURFACELOST == awTlLastDxErr)
					{
						db_log4("\tRestoring Lost Surface");
						
						awTlLastDxErr = pLoadInfo->surfaceP->Restore();
					}
					else if (DD_OK == awTlLastDxErr && (fMyFlags & AW_TLF_SKIPNOTLOST))
					{
						db_log4("\tSkipping Surface which was not Lost");
						
						pLoadInfo->skipB = true;
					}
					
					HANDLE_DXERROR("testing for lost surface and restoring if necessary");
					
					pLoadInfo->surfaceP->Release();
					pLoadInfo->surfaceP = NULL;
				}
				
				if (!pLoadInfo->skipB)
				{
					do
					{
						awTlLastDxErr = driverDesc.ddP->CreateSurface(&ddsd,&pLoadInfo->surfaceP,NULL);
					}
						while
						(
							DDERR_OUTOFVIDEOMEMORY == awTlLastDxErr
							&& _parmsR.callbackF
							&& _parmsR.callbackF(_parmsR.callbackParam)
						);
					
					HANDLE_DXERROR("creating direct draw surface")
				}
			}
			
			if (pLoadInfo->skipB)
			{
				db_assert1(pLoadInfo->prevTexP.voidP);
				
				// skipping so result is same as input
				pLoadInfo->resultP = pLoadInfo->prevTexP;
				
				if (_parmsR.loadTextureB)
					pLoadInfo->prevTexP.textureP->AddRef();
				else
					pLoadInfo->prevTexP.surfaceP->AddRef();
			}
		
			bSkipAll = bSkipAll && pLoadInfo->skipB;
		}}
		
		if (!bSkipAll)
		{
			Colour * paletteP = m_nPaletteSize ? GetPalette() : NULL;
			
			unsigned y = 0;
			bool reversed_rowsB = AreRowsReversed();
			if (reversed_rowsB)
			{
				y = m_nHeight-1;
			}
			
			for (unsigned rowcount = m_nHeight; rowcount; --rowcount)
			{
				PtrUnion src_rowP = GetRowPtr(y);
				db_assert1(src_rowP.voidP);
				
				// allow loading of the next row from the file
				LoadNextRow(src_rowP);
				
				// loop for copying data to surfaces
				for (unsigned i=0; i<nLoadInfos; ++i)
				{
					LoadInfo * pLoadInfo = &arrLoadInfo[i];
					
					if (!pLoadInfo->skipB)
					{
						// are we in the vertical range of this surface?
						if (y>=pLoadInfo->top && y<pLoadInfo->bottom)
						{
							if (!pLoadInfo->surface_lockedB)
							{
								// lock the surfaces
								DD_SURFACE_DESC ddsd;
								INITDXSTRUCT(ddsd);
								awTlLastDxErr = pLoadInfo->surfaceP->Lock(NULL,&ddsd,DDLOCK_WRITEONLY|DDLOCK_NOSYSLOCK,NULL);
								HANDLE_DXERROR("locking direct draw surface")
								pLoadInfo->surface_lockedB = true;
								pLoadInfo->surface_dataP.voidP = ddsd.lpSurface;
								pLoadInfo->surface_dataP.byteP += ddsd.lPitch * (y-pLoadInfo->top);
								pLoadInfo->surface_pitch = ddsd.lPitch;
							}
							
							// convert and copy the section of the row to the direct draw surface
							ConvertRow(pLoadInfo->surface_dataP,pLoadInfo->surface_width,src_rowP,pLoadInfo->left,pLoadInfo->width,paletteP db_code1(DB_COMMA m_nPaletteSize));
							
							// do the bottom row twice if the dd surface is bigger
							if (pLoadInfo->bottom-1 == y && pLoadInfo->surface_height > pLoadInfo->height)
							{
								PtrUnion next_surface_rowP = pLoadInfo->surface_dataP;
								next_surface_rowP.byteP += pLoadInfo->surface_pitch;
								ConvertRow(next_surface_rowP,pLoadInfo->surface_width,src_rowP,pLoadInfo->left,pLoadInfo->width,paletteP db_code1(DB_COMMA m_nPaletteSize));
							}
							
							// next ddsurface row
							if (reversed_rowsB)
								pLoadInfo->surface_dataP.byteP -= pLoadInfo->surface_pitch;
							else
								pLoadInfo->surface_dataP.byteP += pLoadInfo->surface_pitch;
						}
						else if (pLoadInfo->surface_lockedB)
						{
							// unlock the surface
							awTlLastDxErr = pLoadInfo->surfaceP->Unlock(NULL);
							HANDLE_DXERROR("unlocking direct draw surface")
							pLoadInfo->surface_lockedB = false;
						}
					}
				}
				
				// next row
				if (reversed_rowsB)
					--y;
				else
					++y;
				
				if (AW_TLE_OK != awTlLastErr)
				{
					db_log1("AwCreateGraphic() failed whilst copying data to direct draw surface");
					goto EXIT_WITH_ERROR;
				}
			}
			
			// create a palette for the surfaces if there is one
			DWORD palcreateflags = 0;
			PALETTEENTRY colour_tableA[256];
			if (pixelFormat.palettizedB)
			{
				if (!_parmsR.loadTextureB && !(fMyFlags & AW_TLF_TEXTURE))
				{
					db_log3("AwCreateGraphic(): WARNING: setting a palette on a DD surface may have no effect");
				}
				
				#if 0
				if (m_bTranspMask)
				{
					colour_tableA[0].peRed = 0;
					colour_tableA[0].peGreen = 0;
					colour_tableA[0].peBlue = 0;
					colour_tableA[0].peFlags = 0;
					for (unsigned i=1; i<m_nPaletteSize; ++i)
					{
						colour_tableA[i].peRed = paletteP[i].r;
						colour_tableA[i].peGreen = paletteP[i].g;
						colour_tableA[i].peBlue = paletteP[i].b;
						if (!(paletteP[i].r + paletteP[i].g + paletteP[i].b))
							colour_tableA[i].peRed = 1;
						colour_tableA[i].peFlags = 0;
					}
				}
				else
				#endif
				{
					for (unsigned i=0; i<m_nPaletteSize; ++i)
					{
						colour_tableA[i].peRed = paletteP[i].r;
						colour_tableA[i].peGreen = paletteP[i].g;
						colour_tableA[i].peBlue = paletteP[i].b;
						colour_tableA[i].peFlags = 0;
					}
				}
				for (unsigned i=m_nPaletteSize; i<256; ++i)
					colour_tableA[i].peFlags = 0;
				switch (pixelFormat.bitsPerPixel)
				{
					default:
						CANT_HAPPEN
					case 8:
						palcreateflags = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
						break;
					case 4:
						palcreateflags = DDPCAPS_4BIT;
						break;
					case 2:
						palcreateflags = DDPCAPS_2BIT;
						break;
					case 1:
						palcreateflags = DDPCAPS_1BIT;
						break;
				}
				awTlLastDxErr = driverDesc.ddP->CreatePalette(palcreateflags,colour_tableA,&dd_paletteP,NULL);
				HANDLE_DXERROR("creating palette for direct draw surface")
			}
				
			{for (unsigned i=0; i<nLoadInfos; ++i)
			{
				LoadInfo * pLoadInfo = &arrLoadInfo[i];
			
				if (!pLoadInfo->skipB)
				{
					// unlock the surface
					if (pLoadInfo->surface_lockedB)
					{
						awTlLastDxErr = pLoadInfo->surfaceP->Unlock(NULL);
						HANDLE_DXERROR("unlocking direct draw surface")
						pLoadInfo->surface_lockedB = false;
					}
					
					if (pixelFormat.palettizedB)
					{
						// set the palette on the surface
						awTlLastDxErr = pLoadInfo->surfaceP->SetPalette(dd_paletteP);
						HANDLE_DXERROR("setting palette on direct draw surface")
					}
				}
				
			}}
			
			if (pixelFormat.palettizedB)
			{
				db_logf5(("\tabout to release palette with ref %u",GetRefCount(dd_paletteP)));
				dd_paletteP->Release();
				dd_paletteP = NULL;
			}
			
			DWORD dwColourKey;
			DDCOLORKEY invis;
			// get colour for chroma keying if required
			if (m_bTranspMask && (!pixelFormat.alphaB || fMyFlags & AW_TLF_CHROMAKEY))
			{
				dwColourKey = GetTransparentColour();
				invis.dwColorSpaceLowValue = dwColourKey;
				invis.dwColorSpaceHighValue = dwColourKey;
			}
			
			{for (unsigned i=0; i<nLoadInfos; ++i)
			{
				LoadInfo * pLoadInfo = &arrLoadInfo[i];
				
				if (!pLoadInfo->skipB)
				{
					// do the copying crap and Texture::Load() stuff - see CopyD3DTexture in d3_func.cpp
					
					if (_parmsR.loadTextureB)
					{
						// get a texture pointer
						awTlLastDxErr = pLoadInfo->surfaceP->QueryInterface(GUID_D3D_TEXTURE,(LPVOID *)&pLoadInfo->textureP);
						HANDLE_DXERROR("getting texture interface on direct draw surface")
						db_logf5(("\t\tnow surface ref %u texture ref %u",GetRefCount(pLoadInfo->surfaceP),GetRefCount(pLoadInfo->textureP)));
						
						if (fMyFlags & AW_TLF_COMPRESS) // deal with Texture::Load and ALLOCONLOAD flag
						{
							if (pLoadInfo->prevTexP.voidP)
							{
								// load into the existing texture
								awTlLastDxErr = pLoadInfo->prevTexP.textureP->QueryInterface(GUID_DD_SURFACE,(LPVOID *)&pLoadInfo->dst_surfaceP);
								HANDLE_DXERROR("getting direct draw surface interface")
								db_logf5(("\t\tnow prev texture ref %u dst surface ref %u",GetRefCount(pLoadInfo->prevTexP.textureP),GetRefCount(pLoadInfo->dst_surfaceP)));
								// check that the surface desc is OK
								// note that SetSurfaceDesc is *only* supported for changing the surface memory pointer
								DD_SURFACE_DESC old_ddsd;
								INITDXSTRUCT(old_ddsd);
								awTlLastDxErr = pLoadInfo->surfaceP->GetSurfaceDesc(&old_ddsd);
								HANDLE_DXERROR("getting previous surface desc")
								// check width, height, RGBBitCount and memory type
								if (old_ddsd.dwFlags & DDSD_ALL || (old_ddsd.dwFlags & (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT)) == (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT))
								{
									if (old_ddsd.dwHeight == pLoadInfo->surface_height && old_ddsd.dwWidth == pLoadInfo->surface_width && (old_ddsd.ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY|DDSCAPS_VIDEOMEMORY|DDSCAPS_TEXTURE)) == pLoadInfo->dwCapsCaps)
									{
										unsigned bpp = 0;
										if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
											bpp = 8;
										else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
											bpp = 4;
										else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
											bpp = 2;
										else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
											bpp = 1;
										else if (old_ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
											bpp = old_ddsd.ddpfPixelFormat.dwRGBBitCount;
										if (pixelFormat.bitsPerPixel != bpp)
											awTlLastErr = AW_TLE_CANTRELOAD;	
									}
									else
										awTlLastErr = AW_TLE_CANTRELOAD;
								}
								else
									awTlLastErr = AW_TLE_CANTRELOAD;
								if (AW_TLE_OK != awTlLastErr)
								{
									db_log1("AwCreateGraphic() failed because existing surface is incompatible");
									goto EXIT_WITH_ERROR;
								}
							}
							else
							{
								DD_SURFACE_DESC ddsd;
								
								INITDXSTRUCT(ddsd);
								
								awTlLastDxErr = pLoadInfo->surfaceP->GetSurfaceDesc(&ddsd);
								HANDLE_DXERROR("getting direct draw surface desc")

								ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
								ddsd.ddsCaps.dwCaps |= DDSCAPS_ALLOCONLOAD | driverDesc.memFlag;
								do
								{
									awTlLastDxErr = driverDesc.ddP->CreateSurface(&ddsd,&pLoadInfo->dst_surfaceP,NULL);
								}
									while
									(
										DDERR_OUTOFVIDEOMEMORY == awTlLastDxErr
										&& _parmsR.callbackF
										&& _parmsR.callbackF(_parmsR.callbackParam)
									);
								HANDLE_DXERROR("creating destination direct draw surface")
							}
							
							// create a zero palette if required -> Texture::Load() will copy in correct palette
							if (pixelFormat.palettizedB)
							{
								memset(colour_tableA,0,sizeof colour_tableA);
								
								awTlLastDxErr = driverDesc.ddP->CreatePalette(palcreateflags,colour_tableA,&dd_paletteP,NULL);
								HANDLE_DXERROR("creating palette for destination direct draw surface")
								awTlLastDxErr = pLoadInfo->dst_surfaceP->SetPalette(dd_paletteP);
								HANDLE_DXERROR("setting palette on destination direct draw surface")
								db_logf5(("\tabout to release dest palette with ref %u",GetRefCount(dd_paletteP)));
								dd_paletteP->Release();
								dd_paletteP = NULL;
							}
							
							// get a texture pointer on the destination
							awTlLastDxErr = pLoadInfo->dst_surfaceP->QueryInterface(GUID_D3D_TEXTURE,(LPVOID *)&pLoadInfo->dst_textureP);
							HANDLE_DXERROR("getting texture interface on destination direct draw surface")
							db_logf5(("\t\tnow dst surface ref %u dst texture ref %u",GetRefCount(pLoadInfo->dst_surfaceP),GetRefCount(pLoadInfo->dst_textureP)));
							
							do
							{
								awTlLastDxErr = pLoadInfo->dst_textureP->Load(pLoadInfo->textureP);
							}
								while
								(
									DDERR_OUTOFVIDEOMEMORY == awTlLastDxErr
									&& _parmsR.callbackF
									&& _parmsR.callbackF(_parmsR.callbackParam)
								);
							HANDLE_DXERROR("loading texture into destination")
							
							// release src texture and surface, and set pointers to point to dst texture and surface
							db_logf5(("\tabout to release internal surface with ref %u",GetRefCount(pLoadInfo->surfaceP)));
							db_code1(refcnt =)
							pLoadInfo->surfaceP->Release();
							db_onlyassert1(1==refcnt);
							pLoadInfo->surfaceP = pLoadInfo->dst_surfaceP;
							pLoadInfo->dst_surfaceP = NULL;
							
							db_logf5(("\tabout to release internal texture i/f with ref %u",GetRefCount(pLoadInfo->textureP)));
							db_code1(refcnt =)
							pLoadInfo->textureP->Release();
							db_onlyassert1(!refcnt);
							pLoadInfo->textureP = pLoadInfo->dst_textureP;
							pLoadInfo->dst_textureP = NULL;
						}
					}
						
					// set chroma keying if required
					if (m_bTranspMask && (!pixelFormat.alphaB || fMyFlags & AW_TLF_CHROMAKEY))
					{
						awTlLastDxErr = pLoadInfo->surfaceP->SetColorKey(DDCKEY_SRCBLT,&invis);
						HANDLE_DXERROR("setting the colour key")
					}
					
					if (_parmsR.loadTextureB)
					{
						// release the direct draw interface:
						// since the textureP was obtained with a call
						// to QueryInterface on the surface (increasing
						// its referenc count), this wont actually release
						// the surface
						
						db_logf5(("\tabout to release surface i/f with ref %u",GetRefCount(pLoadInfo->surfaceP)));
						db_code1(refcnt =)
						pLoadInfo->surfaceP->Release();
						pLoadInfo->surfaceP = NULL;
						// if loading into a previous texture, refcnt may be two or more, our ref and the ref passed to us
						db_onlyassert1(1==refcnt|| pLoadInfo->prevTexP.voidP);
						
						pLoadInfo->resultP = pLoadInfo->textureP;
					}
					else
					{
						db_assert1(pLoadInfo->surfaceP);
						
						DDSurface * pSurfaceReturn = NULL;
						
						awTlLastDxErr = pLoadInfo->surfaceP->QueryInterface(GUID_DD_SURFACE, (LPVOID *)&pSurfaceReturn);
						HANDLE_DXERROR("getting the required DDSurface interface")
						db_logf5(("\t\tnow surface ref %u return surface ref %u",GetRefCount(pLoadInfo->surfaceP),GetRefCount(pSurfaceReturn)));
						
						pLoadInfo->resultP = pSurfaceReturn;
					}
				}
			}}
			
			// release the IDirectDrawSurface interfaces if returning DDSurface interfaces
			if (!_parmsR.loadTextureB)
			{
				for (unsigned i=0; i<nLoadInfos; ++i)
				{
					if (!arrLoadInfo[i].skipB)
					{
						db_assert1(arrLoadInfo[i].surfaceP);
						db_logf5(("\tabout to release internal surface i/f with ref %u",GetRefCount(arrLoadInfo[i].surfaceP)));
						arrLoadInfo[i].surfaceP->Release();
					}
				}
			}
		}

		// OK
		db_log4("AwCreateGraphic() OK");
		
		SurfUnion pRet = static_cast<D3DTexture *>(NULL);
		
		if (!_parmsR.rectA)
		{
			// if loading the entire graphic as one surface/texture, return pointer to that
			pRet = arrLoadInfo[0].resultP;
		}
		else
		{
			// return NULL, but fill in the pTexture or pSurface members of the AwCreateGraphicRegion
			for (unsigned i=0; i<nLoadInfos; ++i)
			{
				LoadInfo * pLoadInfo = &arrLoadInfo[i];
				
				db_assert1(pLoadInfo->rectP);
				
				if (_parmsR.loadTextureB)
				{
					if (pLoadInfo->prevTexP.voidP)
					{
						db_assert1(pLoadInfo->prevTexP.textureP == pLoadInfo->rectP->pTexture);
						db_assert1(pLoadInfo->resultP.textureP == pLoadInfo->rectP->pTexture);
						db_logf5(("\tabout to release duplicate texture i/f with ref %u",GetRefCount(pLoadInfo->resultP.textureP)));
						pLoadInfo->resultP.textureP->Release();
					}
					else
					{
						pLoadInfo->rectP->pTexture = pLoadInfo->resultP.textureP;
					}
					db_logf5(("\tresultant texture for region with ref count %u",GetRefCount(pLoadInfo->rectP->pTexture)));
				}
				else
				{
					if (pLoadInfo->prevTexP.voidP)
					{
						db_assert1(pLoadInfo->prevTexP.surfaceP == pLoadInfo->rectP->pSurface);
						db_assert1(pLoadInfo->resultP.surfaceP == pLoadInfo->rectP->pSurface);
						db_logf5(("\tabout to release duplicate surface i/f with ref %u",GetRefCount(pLoadInfo->resultP.surfaceP)));
						pLoadInfo->resultP.surfaceP->Release();
					}
					else
					{
						pLoadInfo->rectP->pSurface = pLoadInfo->resultP.surfaceP;
					}
					db_logf5(("\tresultant texture for surface with ref count %u",GetRefCount(pLoadInfo->rectP->pSurface)));
				}
			}
		}
		delete[] arrLoadInfo;
		
		#if DB_LEVEL >= 5
		if (_parmsR.loadTextureB)
			db_logf5(("AwCreateGraphic(): returning texture with ref cnt %u",GetRefCount(pRet.textureP)));
		else
			db_logf5(("AwCreateGraphic(): returning surface with ref cnt %u",GetRefCount(pRet.surfaceP)));
		#endif
		
		return pRet;
	}
		
	EXIT_WITH_ERROR:
	{
		
		db_logf2(("AwCreateGraphic(): ERROR: %s",AwTlErrorToString()));
		
		if (arrLoadInfo)
		{
			for (unsigned i=0; i<nLoadInfos; ++i)
			{
				LoadInfo * pLoadInfo = &arrLoadInfo[i];
				
				db_logf5(("\tref counts: dst tex %u dst surf %u int tex %u int surf %u",
					GetRefCount(pLoadInfo->dst_textureP),
					GetRefCount(pLoadInfo->dst_surfaceP),
					GetRefCount(pLoadInfo->textureP),
					GetRefCount(pLoadInfo->surfaceP)));
				
				if (pLoadInfo->dst_textureP)
				{
					pLoadInfo->dst_textureP->Release();
				}
				if (pLoadInfo->textureP)
				{
					pLoadInfo->textureP->Release();
				}
				if (pLoadInfo->dst_surfaceP)
				{
					pLoadInfo->dst_surfaceP->Release();
				}
				if (pLoadInfo->surfaceP)
				{
					if (pLoadInfo->surface_lockedB)
						pLoadInfo->surfaceP->Unlock(NULL);
					db_code1(refcnt =)
					pLoadInfo->surfaceP->Release();
					db_onlyassert1(!refcnt);
				}
				
				if (pLoadInfo->rectP)
				{
					pLoadInfo->rectP->width = 0;
					pLoadInfo->rectP->height = 0;
				}
			}
			
			delete[] arrLoadInfo;
		}
		
		if (dd_paletteP)
		{
			db_code1(refcnt =)
			dd_paletteP->Release();
			db_onlyassert1(!refcnt);
		}
		
		return static_cast<D3DTexture *>(NULL);
	}
#endif	
}

void AwBackupTexture::OnBeginRestoring(unsigned nMaxPaletteSize)
{
	if (nMaxPaletteSize && (nMaxPaletteSize < m_nPaletteSize || !m_nPaletteSize))
	{
		awTlLastErr = AW_TLE_CANTPALETTIZE;
		db_logf3(("AwCreateGraphic(): [restoring] ERROR: Palette size is %u, require %u",m_nPaletteSize,nMaxPaletteSize));
	}
}

bool AwBackupTexture::AreRowsReversed()
{
	return false;
}

void AwBackupTexture::ConvertRow(AwTl::PtrUnion pDest, unsigned nDestWidth, AwTl::PtrUnionConst pSrc, unsigned nSrcOffset, unsigned nSrcWidth, AwTl::Colour * pPalette db_code1(DB_COMMA unsigned nPaletteSize))
{
	using namespace AwTl;

	if (pPalette)
	{
		if (pixelFormat.palettizedB)
		{
			GenericConvertRow<Colour::ConvNull,BYTE>::Do(pDest,nDestWidth,pSrc.byteP+nSrcOffset,nSrcWidth);
		}
		else
		{
			if (m_bTranspMask)
				GenericConvertRow<Colour::ConvTransp,BYTE>::Do(pDest,nDestWidth,pSrc.byteP+nSrcOffset,nSrcWidth,pPalette db_code1(DB_COMMA nPaletteSize));
			else
				GenericConvertRow<Colour::ConvNonTransp,BYTE>::Do(pDest,nDestWidth,pSrc.byteP+nSrcOffset,nSrcWidth,pPalette db_code1(DB_COMMA nPaletteSize));
		}
	}
	else
	{	
		if (m_bTranspMask)
			GenericConvertRow<Colour::ConvTransp,Colour>::Do(pDest,nDestWidth,pSrc.colourP+nSrcOffset,nSrcWidth);
		else
			GenericConvertRow<Colour::ConvNonTransp,Colour>::Do(pDest,nDestWidth,pSrc.colourP+nSrcOffset,nSrcWidth);
	}
}

void AwBackupTexture::OnFinishRestoring(bool)
{
}

namespace AwTl {

	Colour * TypicalBackupTexture::GetPalette()
	{
		return m_pPalette;
	}

	PtrUnion TypicalBackupTexture::GetRowPtr(unsigned nRow)
	{
		return m_ppPixMap[nRow];
	}

	void TypicalBackupTexture::LoadNextRow(PtrUnion)
	{
		// already loaded
	}

	unsigned TypicalBackupTexture::GetNumColours()
	{
		return m_nPaletteSize;
	}

	unsigned TypicalBackupTexture::GetMinPaletteSize()
	{
		return m_nPaletteSize;
	}
	

	SurfUnion TexFileLoader::Load(MediaMedium * pMedium, CreateTextureParms const & rParams)
	{
		m_fFlags = rParams.flags;
		
		awTlLastErr = AW_TLE_OK;
		
		LoadHeaderInfo(pMedium);
		
//		CHECK_MEDIA_ERRORS("loading file headers")
		ON_ERROR_RETURN_NULL("loading file headers")

		ChoosePixelFormat(rParams);
#if 0
		if (!pixelFormat.validB)
			db_log3("AwCreateGraphic(): ERROR: pixel format not valid");
		if (!driverDesc.ddP || !driverDesc.validB && rParams.loadTextureB)
			db_log3("AwCreateGraphic(): ERROR: driver description not valid");
		
		awTlLastErr = pixelFormat.validB && driverDesc.ddP && (driverDesc.validB || !rParams.loadTextureB) ? AW_TLE_OK : AW_TLE_NOINIT;
		
		ON_ERROR_RETURN_NULL("initializing load")
#endif
//		fprintf(stderr, "TexFileLoader::Load Pixel Format?! It's not implemented!\n");

/* TODO */
		AllocateBuffers(/* rParams.backupHP ? true : */ false, /* pixelFormat.palettizedB ? ? 1<<pixelFormat.bitsPerPixel : */ 0);
				
//		CHECK_MEDIA_ERRORS("allocating buffers")
		ON_ERROR_RETURN_NULL("allocating buffers")
		
		db_logf4(("\tThe image in the file is %ux%u with %u %spalette",m_nWidth,m_nHeight,m_nPaletteSize ? m_nPaletteSize : 0,m_nPaletteSize ? "colour " : ""));
		
		SurfUnion pTex = CreateTexture(rParams);
		
		bool bOK = AW_TLE_OK == awTlLastErr;
		
//		CHECK_MEDIA_ERRORS("loading image data")
		
		if (bOK && awTlLastErr != AW_TLE_OK)
		{
			fprintf(stderr, "TexFileLoader::Load()\n");
#if 0		
			// an error occurred which was not detected in CreateTexture()
			if (pTex.voidP)
			{
				if (rParams.loadTextureB)
					pTex.textureP->Release();
				else
					pTex.surfaceP->Release();
				pTex.voidP = NULL;
			}
			else
			{
				db_assert1(rParams.rectA);
				
				for (unsigned i=0; i<rParams.numRects; ++i)
				{
					AwCreateGraphicRegion * pRect = &rParams.rectA[i];
					
					if (!rParams.prevTexB)
					{
						// release what was created
						if (rParams.loadTextureB)
							pRect->pTexture->Release();
						else
							pRect->pSurface->Release();
					}
					pRect->width = 0;
					pRect->height = 0;
				}
			}
			db_logf1(("AwCreateGraphic(): ERROR: %s",AwTlErrorToString()));
#endif			
			bOK = false;
		}
		
		OnFinishLoading(bOK);
		
		return pTex;
	}

	void TexFileLoader::OnFinishLoading(bool)
	{
	}


	TypicalTexFileLoader::~TypicalTexFileLoader()
	{
		if (m_pPalette)
		{
			delete[] m_pPalette;
			
			if (m_pRowBuf) delete[] m_pRowBuf.byteP;
			if (m_ppPixMap)
			{
				delete[] m_ppPixMap->byteP;
				delete[] m_ppPixMap;
			}
		}
		else
		{
			if (m_pRowBuf) delete[] m_pRowBuf.colourP;
			if (m_ppPixMap)
			{
				delete[] m_ppPixMap->colourP;
				delete[] m_ppPixMap;
			}
		}
	}
	
	unsigned TypicalTexFileLoader::GetNumColours()
	{
		return m_nPaletteSize;
	}

	unsigned TypicalTexFileLoader::GetMinPaletteSize()
	{
		return m_nPaletteSize;
	}

	void TypicalTexFileLoader::AllocateBuffers(bool bWantBackup, unsigned /*nMaxPaletteSize*/)
	{
		if (m_nPaletteSize)
		{
			m_pPalette = new Colour [ m_nPaletteSize ];
		}
		
		if (bWantBackup)
		{
			m_ppPixMap = new PtrUnion [m_nHeight];
			if (m_nPaletteSize)
			{
				m_ppPixMap->byteP = new BYTE [m_nHeight*m_nWidth];
				BYTE * pRow = m_ppPixMap->byteP;
				for (unsigned y=1;y<m_nHeight;++y)
				{
					pRow += m_nWidth;
					m_ppPixMap[y].byteP = pRow;
				}
			}
			else
			{
				m_ppPixMap->colourP = new Colour [m_nHeight*m_nWidth];
				Colour * pRow = m_ppPixMap->colourP;
				for (unsigned y=1;y<m_nHeight;++y)
				{
					pRow += m_nWidth;
					m_ppPixMap[y].colourP = pRow;
				}
			}
		}
		else
		{
			if (m_nPaletteSize)
				m_pRowBuf.byteP = new BYTE [m_nWidth];
			else
				m_pRowBuf.colourP = new Colour [m_nWidth];
		}
	}

	PtrUnion TypicalTexFileLoader::GetRowPtr(unsigned nRow)
	{
		if (m_ppPixMap)
		{
			return m_ppPixMap[nRow];
		}
		else
		{
			return m_pRowBuf;
		}
	}

	AwBackupTexture * TypicalTexFileLoader::CreateBackupTexture()
	{
		AwBackupTexture * pBackup = new TypicalBackupTexture(*this,m_ppPixMap,m_pPalette);
		m_ppPixMap = NULL;
		m_pPalette = NULL;
		return pBackup;
	}
	
	/****************************************************************************/
	/* For determining which loader should be used for the file format detected */
	/****************************************************************************/

	static
	class MagicFileIdTree
	{
		public:
			MagicFileIdTree()
				: m_pfnCreate(NULL)
				#ifdef _MSC_VER
				, hack(0)
				#endif
			{
				for (unsigned i=0; i<256; ++i)
					m_arrNextLayer[i]=NULL;
			}
			
			~MagicFileIdTree()
			{
				for (unsigned i=0; i<256; ++i)
					if (m_arrNextLayer[i]) delete m_arrNextLayer[i];
			}
		
			MagicFileIdTree * m_arrNextLayer [256];
			
			TexFileLoader * (* m_pfnCreate) ();
			
		#ifdef _MSC_VER
			unsigned hack;
		#endif
	}
		* g_pMagicFileIdTree = NULL;
	
	void RegisterLoader(char const * pszMagic, TexFileLoader * (* pfnCreate) () )
	{
		static MagicFileIdTree mfidt;

		g_pMagicFileIdTree = &mfidt;
		
		MagicFileIdTree * pLayer = g_pMagicFileIdTree;
		
		while (*pszMagic)
		{
			BYTE c = static_cast<BYTE>(*pszMagic++);
			
			if (!pLayer->m_arrNextLayer[c])
				pLayer->m_arrNextLayer[c] = new MagicFileIdTree;
				
			pLayer = pLayer->m_arrNextLayer[c];
		}
		
		db_assert1(!pLayer->m_pfnCreate);
		
		pLayer->m_pfnCreate = pfnCreate;
	}
	
	static
	TexFileLoader * CreateLoaderObject(MediaMedium * pMedium)
	{
		TexFileLoader * (* pfnBest) () = NULL;
		
		signed nMoveBack = 0;
		
		BYTE c;
		
		MagicFileIdTree * pLayer = g_pMagicFileIdTree;
		
		while (pLayer)
		{
			if (pLayer->m_pfnCreate)
				pfnBest = pLayer->m_pfnCreate;
			
			MediaRead(pMedium,&c);
			
			-- nMoveBack;
			
			pLayer = pLayer->m_arrNextLayer[c];
		}
		
		pMedium->MovePos(nMoveBack);
		
		if (pfnBest)
			return pfnBest();
		else
			return NULL;
	}

	/**********************************/
	/* These are the loader functions */
	/**********************************/
	
	static inline SurfUnion DoLoadTexture(MediaMedium * pMedium, CreateTextureParms const & rParams)
	{
		TexFileLoader * pLoader = CreateLoaderObject(pMedium);
		
		if (!pLoader)
		{
			awTlLastErr = AW_TLE_BADFILEFORMAT;
			db_log1("AwCreateGraphic(): ERROR: file format not recognized");
			return static_cast<D3DTexture *>(NULL);
		}
		else
		{
			SurfUnion pTex = pLoader->Load(pMedium,rParams);
			pLoader->Release();
			return pTex;
		}
	}
	
	static inline SurfUnion LoadTexture(MediaMedium * pMedium, CreateTextureParms const & _parmsR)
	{
		if (_parmsR.bytesReadP||_parmsR.maxReadBytes!=UINT_MAX)
		{
			MediaSection * pMedSect = new MediaSection;
			pMedSect->Open(pMedium,_parmsR.maxReadBytes);
			SurfUnion pTex = DoLoadTexture(pMedSect,_parmsR);
			pMedSect->Close();
			if (_parmsR.bytesReadP) *_parmsR.bytesReadP = pMedSect->GetUsedSize();
			delete pMedSect;
			return pTex;
		}
		else
		{
			return DoLoadTexture(pMedium,_parmsR);
		}
	}
	
	SurfUnion CreateTextureParms::DoCreate() const
	{
		if (NULL != fileH)
		{
			MediaStdFileMedium * pMedium = new MediaStdFileMedium;
			pMedium->Attach(fileH);
			SurfUnion pTex = LoadTexture(pMedium,*this);
			pMedium->Detach();
			pMedium->Release();
			return pTex;
		}
		else if (dataP)
		{
			MediaMemoryReadMedium * pMedium = new MediaMemoryReadMedium;
			pMedium->Open(dataP);
			SurfUnion pTex = LoadTexture(pMedium,*this);
			pMedium->Close();
			pMedium->Release();
			return pTex;
		}
		else
		{
			db_assert1(restoreH);
			return restoreH->Restore(*this);
		}
	}
	
	// Parse the format string and get the parameters
	
	static bool ParseParams(CreateTextureParms * pParams, char const * _argFormatS, va_list ap)
	{
		bool bad_parmsB = false;
		db_code2(unsigned ch_off = 0;)
		db_code2(char ch = 0;)
		
		while (*_argFormatS && !bad_parmsB)
		{
			db_code2(++ch_off;)
			db_code2(ch = *_argFormatS;)
			switch (*_argFormatS++)
			{
				case 's':
					if (pParams->fileNameS || NULL!=pParams->fileH || pParams->dataP || pParams->restoreH)
						bad_parmsB = true;
					else
					{
						pParams->fileNameS = va_arg(ap,char *);
						db_logf4(("\tFilename = \"%s\"",pParams->fileNameS));
					}
					break;
				case 'h':
					if (pParams->fileNameS || NULL!=pParams->fileH || pParams->dataP || pParams->restoreH)
						bad_parmsB = true;
					else
					{
						pParams->fileH = va_arg(ap,FILE *);
						db_logf4(("\tFile HANDLE = %p",pParams->fileH));
					}
					break;
				case 'p':
					if (pParams->fileNameS || NULL!=pParams->fileH || pParams->dataP || pParams->restoreH)
						bad_parmsB = true;
					else
					{
						pParams->dataP = va_arg(ap,void const *);
						db_logf4(("\tData Pointer = %p",pParams->dataP.voidP));
					}
					break;
				case 'r':
					if (pParams->fileNameS || NULL!=pParams->fileH || pParams->dataP || pParams->restoreH || UINT_MAX!=pParams->maxReadBytes || pParams->bytesReadP)
						bad_parmsB = true;
					else
					{
						pParams->restoreH = va_arg(ap,AW_BACKUPTEXTUREHANDLE);
						db_logf4(("\tRestore Handle = 0x%08x",pParams->restoreH));
					}
					break;
				case 'x':
					if (UINT_MAX!=pParams->maxReadBytes || pParams->restoreH)
						bad_parmsB = true;
					else
					{
						pParams->maxReadBytes = va_arg(ap,unsigned);
						db_logf4(("\tMax bytes to read = %u",pParams->maxReadBytes));
					}
					break;
				case 'N':
					if (pParams->bytesReadP || pParams->restoreH)
						bad_parmsB = true;
					else
					{
						pParams->bytesReadP = va_arg(ap,unsigned *);
						db_logf4(("\tPtr to bytes read = %p",pParams->bytesReadP));
					}
					break;
				case 'f':
					if (AW_TLF_DEFAULT!=pParams->flags)
						bad_parmsB = true;
					else
					{
						pParams->flags = va_arg(ap,unsigned);
						db_logf4(("\tFlags = 0x%08x",pParams->flags));
					}
					break;
				case 'W':
					if (pParams->widthP || pParams->rectA)
						bad_parmsB = true;
					else
					{
						pParams->widthP = va_arg(ap,unsigned *);
						db_logf4(("\tPtr to width = %p",pParams->widthP));
					}
					break;
				case 'H':
					if (pParams->heightP || pParams->rectA)
						bad_parmsB = true;
					else
					{
						pParams->heightP = va_arg(ap,unsigned *);
						db_logf4(("\tPtr to height = %p",pParams->heightP));
					}
					break;
				case 'X':
					if (pParams->originalWidthP)
						bad_parmsB = true;
					else
					{
						pParams->originalWidthP = va_arg(ap,unsigned *);
						db_logf4(("\tPtr to image width = %p",pParams->originalWidthP));
					}
					break;
				case 'Y':
					if (pParams->originalHeightP)
						bad_parmsB = true;
					else
					{
						pParams->originalHeightP = va_arg(ap,unsigned *);
						db_logf4(("\tPtr to image height = %p",pParams->originalHeightP));
					}
					break;
				case 'B':
					if (pParams->restoreH)
						bad_parmsB = true;
					break;
				case 't':
					if (pParams->prevTexP.voidP)
						bad_parmsB = true;
					else if (pParams->rectA)
					{
						pParams->prevTexB = true;
						db_log4("\tPrevious DDSurface * or D3DTexture * in rectangle array");
					}
					else if (pParams->loadTextureB)
					{
						pParams->prevTexP = va_arg(ap,D3DTexture *);
						db_logf4(("\tPrevious D3DTexture * = %p",pParams->prevTexP.textureP));
					}
					else
					{
						pParams->prevTexP = va_arg(ap,DDSurface *);
						db_logf4(("\tPrevious DDSurface * = %p",pParams->prevTexP.surfaceP));
					}
					break;
				case 'c':
					if (pParams->callbackF)
						bad_parmsB = true;
					else
					{
						pParams->callbackF = va_arg(ap,AW_TL_PFN_CALLBACK);
						pParams->callbackParam = va_arg(ap,void *);
						db_logf4(("\tCallback function = %p, param = %p",pParams->callbackF,pParams->callbackParam));
					}
					break;
				case 'a':
					if (pParams->prevTexP.voidP || pParams->rectA || pParams->widthP || pParams->heightP)
						bad_parmsB = true;
					else
					{
						pParams->numRects = va_arg(ap,unsigned);
						pParams->rectA = va_arg(ap,AwCreateGraphicRegion *);
						db_logf4(("\tRectangle array = %p, size = %u",pParams->rectA,pParams->numRects));
					}
					break;
				default:
					bad_parmsB = true;
			}
		}
		
		if (!pParams->fileNameS && NULL==pParams->fileH && !pParams->dataP && !pParams->restoreH)
		{
			awTlLastErr = AW_TLE_BADPARMS;
			db_log2("AwCreateGraphic(): ERROR: No data medium is specified");
			return false;
		}
		else if (bad_parmsB)
		{
			awTlLastErr = AW_TLE_BADPARMS;
			db_logf2(("AwCreateGraphic(): ERROR: Unexpected '%c' in format string at character %u",ch,ch_off));
			return false;
		}
		else
		{
			db_log5("\tParameters are OK");
			return true;
		}
	}
	
	// Use the parameters parsed to load the surface or texture
	
	SurfUnion LoadFromParams(CreateTextureParms * pParams)
	{
		if (pParams->fileNameS)
		{
			pParams->fileH = OpenGameFile(pParams->fileNameS, FILEMODE_READONLY, FILETYPE_PERM);
		
			if (NULL==pParams->fileH)
			{
				awTlLastErr = AW_TLE_CANTOPENFILE;
			//	awTlLastWinErr = GetLastError();
				db_logf1(("AwCreateGraphic(): ERROR opening file \"%s\"",pParams->fileNameS));
				db_log2(AwTlErrorToString());
				return static_cast<D3DTexture *>(NULL);
			}
			
			SurfUnion textureP = pParams->DoCreate();
		
			fclose(pParams->fileH);
			
			return textureP;
		}
		else return pParams->DoCreate();
	}

} // namespace AwTl

/******************************/
/* PUBLIC: AwSetTextureFormat */
/******************************/

#define IS_VALID_MEMBER(sP,mem) (reinterpret_cast<unsigned>(&(sP)->mem) - reinterpret_cast<unsigned>(sP) < static_cast<unsigned>((sP)->dwSize))

#define GET_VALID_MEMBER(sP,mem,deflt) (IS_VALID_MEMBER(sP,mem) ? (sP)->mem : (db_logf4((FUNCTION_NAME ": WARNING: %s->%s is not valid",#sP ,#mem )),(deflt)))

#define HANDLE_INITERROR(test,s) \
	if (!(test)) { \
		db_logf3((FUNCTION_NAME " failed becuse %s",s)); \
		db_log1(FUNCTION_NAME ": ERROR: unexpected parameters"); \
		return AW_TLE_BADPARMS; \
	} else { \
		db_logf5(("\t" FUNCTION_NAME " passed check '%s'",#test )); \
	}


AW_TL_ERC AwSetD3DDevice(void * _d3ddeviceP)
{
	using AwTl::driverDesc;
	
	driverDesc.validB = false;
	
	db_logf4(("AwSetD3DDevice(%p) called",_d3ddeviceP));
	
//	HANDLE_INITERROR(_d3ddeviceP,"D3DDevice * is NULL")
	
	driverDesc.validB = true;
	
	return AW_TLE_OK;
}

AW_TL_ERC AwSetDDObject(void * _ddP)
{
	using AwTl::driverDesc;
	
	fprintf(stderr, "AwSetDDObject(%p) called.",_ddP);
	
//	HANDLE_INITERROR(_ddP,"DDObject * is NULL")
	driverDesc.ddP = _ddP;
	
	return AW_TLE_OK;
}

AW_TL_ERC AwSetD3DDevice(void * _ddP, void * _d3ddeviceP)
{
	fprintf(stderr, "AwSetD3DDevice(%p,%p) called",_ddP,_d3ddeviceP);
	
	AW_TL_ERC iResult = AwSetDDObject(_ddP);
	
	if (AW_TLE_OK != iResult)
		return iResult;
	else
		return AwSetD3DDevice(_d3ddeviceP);
}

static AW_TL_ERC AwSetPixelFormat(AwTl::PixelFormat * _pfP, void * _ddpfP)
{
	using AwTl::SetBitShifts;
	
fprintf(stderr, "AwSetPixelFormat(%p, %p)\n", _pfP, _ddpfP);

	_pfP->validB = false;
	
	_pfP->palettizedB = true;

	_pfP->validB = true;
	
_pfP->palettizedB = 0;
_pfP->alphaB = 0;
_pfP->validB = 1;
_pfP->bitsPerPixel = 32;
_pfP->redLeftShift = 0;
_pfP->greenLeftShift = 8;
_pfP->blueLeftShift = 16;
_pfP->redRightShift = 0;
_pfP->greenRightShift = 0;
_pfP->blueRightShift = 0;
_pfP->dwRGBAlphaBitMask = 0xFF000000;

	return AW_TLE_OK;
}

AW_TL_ERC AwSetTextureFormat2(void* _ddpfP)
{
	db_logf4(("AwSetTextureFormat(%p) called",_ddpfP));
	
	using namespace AwTl;
	
	while (listTextureFormats.size())
		listTextureFormats.delete_first_entry();
	
	return AwSetPixelFormat(&pfTextureFormat, _ddpfP);
}

AW_TL_ERC AwSetAdditionalTextureFormat2(void * _ddpfP, unsigned _maxAlphaBits, int _canDoTransp, unsigned _maxColours)
{
	db_logf4(("AwSetAdditionalTextureFormat(%p.%u,%d,%u) called",_ddpfP,_maxAlphaBits,_canDoTransp,_maxColours));
	
	using namespace AwTl;
	
	AdditionalPixelFormat pf;
	
	AW_TL_ERC erc = AwSetPixelFormat(&pf, _ddpfP);
	
	if (AW_TLE_OK == erc)
	{
		pf.canDoTranspB = _canDoTransp ? true : false;
		pf.maxColours = _maxColours;
		
		listTextureFormats.add_entry_end(pf);
	}
	
	return erc;
}

AW_TL_ERC AwSetSurfaceFormat2(void* _ddpfP)
{
	db_logf4(("AwSetSurfaceFormat(%p) called",_ddpfP));
	
	using namespace AwTl;
	
	return AwSetPixelFormat(&pfSurfaceFormat, _ddpfP);
}

/****************************/
/* PUBLIC: AwGetTextureSize */
/****************************/

AW_TL_ERC AwGetTextureSize(unsigned * _widthP, unsigned * _heightP, unsigned _width, unsigned _height)
{
	* _widthP = _width;
	* _heightP = _height;
	
	return AW_TLE_OK;
}


/******************************/
/* PUBLIC: AwCreate functions */
/******************************/

D3DTexture * _AWTL_VARARG AwCreateTexture(char const * _argFormatS, ...)
{
	db_logf4(("AwCreateTexture(\"%s\") called",_argFormatS));
	
	using namespace AwTl;
	
	va_list ap;
	va_start(ap,_argFormatS);
	CreateTextureParms parms;
	parms.loadTextureB = true;
	bool bParmsOK = ParseParams(&parms, _argFormatS, ap);
	va_end(ap);

	return bParmsOK ? LoadFromParams(&parms).textureP : NULL;
}

DDSurface * AwCreateSurface(char const * _argFormatS, ...)
{
	db_logf4(("AwCreateSurface(\"%s\") called",_argFormatS));
	
	using namespace AwTl;
	
	/* Just convert the texture to 32bpp */
	pixelFormat.palettizedB = 0;
	
	pixelFormat.alphaB = 1;
	pixelFormat.validB = 1;
	pixelFormat.texB = 0;
	pixelFormat.bitsPerPixel = 32;
	pixelFormat.redLeftShift = 0;
	pixelFormat.greenLeftShift = 8;
	pixelFormat.blueLeftShift = 16;
	pixelFormat.redRightShift = 0;
	pixelFormat.greenRightShift = 0;
	pixelFormat.blueRightShift = 0;
	pixelFormat.dwRGBAlphaBitMask = 0xFF000000;

	va_list ap;
	va_start(ap,_argFormatS);
	CreateTextureParms parms;
	parms.loadTextureB = false;
	bool bParmsOK = ParseParams(&parms, _argFormatS, ap);
	va_end(ap);
	return bParmsOK ? LoadFromParams(&parms).surfaceP : NULL;
}

AW_TL_ERC AwDestroyBackupTexture(AW_BACKUPTEXTUREHANDLE _bH)
{
	return AW_TLE_OK;
}

/*********************************/
/* PUBLIC DEBUG: LastErr globals */
/*********************************/

AW_TL_ERC awTlLastErr;

/*******************************************/
/* PUBLIC DEBUG: AwErrorToString functions */
/*******************************************/

#ifndef NDEBUG
char const * AwWinErrorToString(DWORD error)
{
	if (NO_ERROR==error)
		return "No error";
	
	return "AwWinErrorToString: No clue!";
}

char const * AwTlErrorToString(AwTlErc error)
{
	char const * defaultS;
	switch (error)
	{
		case AW_TLE_OK:
			return "No error";
		case AW_TLE_DXERROR:
//			if (!awTlLastDxErr)
				return "Unknown DirectX error";
//			else
//				return AwDxErrorToString();
		case AW_TLE_BADPARMS:
			return "Invalid parameters or functionality not supported";
		case AW_TLE_NOINIT:
			return "Initialization failed or not performed";
		case AW_TLE_CANTOPENFILE:
			defaultS = "Unknown error opening file";
			goto WIN_ERR;
		case AW_TLE_CANTREADFILE:
			defaultS = "Unknown error reading file";
		WIN_ERR:
			return defaultS;
		case AW_TLE_EOFMET:
			return "Unexpected end of file during texture load";
		case AW_TLE_BADFILEFORMAT:
			return "Texture file format not recognized";
		case AW_TLE_BADFILEDATA:
			return "Texture file data not consistent";
		case AW_TLE_CANTPALETTIZE:
			return "Texture file data not palettized";
		case AW_TLE_IMAGETOOLARGE:
			return "Image is too large for a texture";
		case AW_TLE_CANTRELOAD:
			return "New image is wrong size or format to load into existing texture";
		default:
			return "Unknown texture loading error";
	}
}

char const * AwDxErrorToString(int error)
{
	return "Unrecognized error value.\0";
}

#endif
