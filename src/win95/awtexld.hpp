#ifndef _INCLUDED_AWTEXLD_HPP_
#define _INCLUDED_AWTEXLD_HPP_

#include "awtexld.h"
#include "media.hpp"
#include "db.h"
#ifndef DB_COMMA
	#define DB_COMMA ,
#endif

namespace AwTl {

	#define CANT_HAPPEN db_msgf1(("AwCreateTexture(): (Line %u) CAN'T HAPPEN!",__LINE__));
	
	/*********************************/
	/* Pixel format global structure */
	/*********************************/
	
	struct PixelFormat
	{
		PixelFormat() : validB(false){}
		
		bool palettizedB : 1;
		bool alphaB : 1;
		bool validB : 1;
		bool texB : 1;
		
		unsigned bitsPerPixel;
		unsigned redLeftShift;
		unsigned redRightShift;
		unsigned greenLeftShift;
		unsigned greenRightShift;
		unsigned blueLeftShift;
		unsigned blueRightShift;
		
		unsigned dwRGBAlphaBitMask;
//		DDPIXELFORMAT ddpf;
	};

	// DO SOMTHING ABOUT THIS
	extern PixelFormat pixelFormat;
	extern PixelFormat pfSurfaceFormat;
	
	class CreateTextureParms;
	
	/********************/
	/* Colour structure */
	/********************/

	struct Colour
	{
		BYTE r,g,b;
		
		class ConvNonTransp
		{
			public:
				static inline unsigned DoConv (Colour const * _colP, Colour const * = NULL db_code1(DB_COMMA unsigned = 0))
				{
					return
						 static_cast<unsigned>(_colP->r)>>pixelFormat.redRightShift<<pixelFormat.redLeftShift
						|static_cast<unsigned>(_colP->g)>>pixelFormat.greenRightShift<<pixelFormat.greenLeftShift
						|static_cast<unsigned>(_colP->b)>>pixelFormat.blueRightShift<<pixelFormat.blueLeftShift
						|pixelFormat.dwRGBAlphaBitMask;
				}
				static inline unsigned DoConv(BYTE const * _colP, Colour const * _paletteP db_code1(DB_COMMA unsigned _paletteSize))
				{
					db_assert1(_paletteP);
					db_onlyassert1(*_colP < _paletteSize);
					return DoConv(&_paletteP[*_colP]);
				}
		};

		class ConvTransp
		{
			private:
				static inline unsigned MakeNonTranspCol(Colour const * _colP)
				{
					unsigned rv = ConvNonTransp::DoConv(_colP);
					if (rv) return rv;
					// make one of r,g or b in the output == 1, choose the one which is closest to the input
					unsigned rdiff = (1<<pixelFormat.redRightShift) - _colP->r;
					unsigned bdiff = (1<<pixelFormat.blueRightShift) - _colP->b;
					unsigned gdiff = (1<<pixelFormat.greenRightShift) - _colP->g;
					if (bdiff<=rdiff && bdiff<=gdiff)
						return 1<<pixelFormat.blueLeftShift;
					else if (rdiff<=gdiff)
						return 1<<pixelFormat.redLeftShift;
					else
						return 1<<pixelFormat.greenLeftShift;
				}
			public:
				static inline unsigned DoConv (Colour const * _colP, Colour const * = NULL db_code1(DB_COMMA unsigned = 0))
				{
					if (!_colP->b && !_colP->r && !_colP->g)
						//return pixelFormat.alphaB ? pixelFormat.ddsd.ddpfPixelFormat.dwRBitMask|pixelFormat.ddsd.ddpfPixelFormat.dwGBitMask|pixelFormat.ddsd.ddpfPixelFormat.dwBBitMask : 0;
						return 0;
					else
						return MakeNonTranspCol(_colP);
				}
				static inline unsigned DoConv(BYTE const * _colP, Colour const * _paletteP db_code1(DB_COMMA unsigned _paletteSize))
				{
					db_assert1(_paletteP);
					db_onlyassert1(*_colP < _paletteSize);
					if (!*_colP)
						//return pixelFormat.alphaB ? pixelFormat.ddsd.ddpfPixelFormat.dwRBitMask|pixelFormat.ddsd.ddpfPixelFormat.dwGBitMask|pixelFormat.ddsd.ddpfPixelFormat.dwBBitMask : 0;
						return 0;
					else
						return MakeNonTranspCol(&_paletteP[*_colP]);
				}
		};
		
		class ConvNull
		{
			public:
				static inline unsigned DoConv (BYTE const * _colP, Colour const * db_code1(DB_COMMA unsigned = 0))
				{
					db_assert1(pixelFormat.palettizedB);
					return *_colP;
				}
		};
	};

	/*****************/
	/* Pointer union */
	/*****************/
	
	union SurfUnion
	{
		D3DTexture * textureP;
		DDSurface * surfaceP;
		void * voidP;
		SurfUnion(){}
		SurfUnion(void * p) : voidP(p){}
	};

	union PtrUnion
	{
		void * voidP;
		char * charP;
		signed char * scharP;
		unsigned char * ucharP;
		BYTE * byteP;
		short * shortP;
		unsigned short * ushortP;
		WORD * wordP;
		signed * intP;
		unsigned * uintP;
		DWORD * dwordP;
		long * longP;
		unsigned long * ulongP;
		Colour * colourP;
		
		inline PtrUnion(){}
		inline PtrUnion(void * _voidP):voidP(_voidP){}
		inline operator void * () const { return voidP; }
	};

	union PtrUnionConst
	{
		void const * voidP;
		char const * charP;
		signed char const * scharP;
		unsigned char const * ucharP;
		BYTE const * byteP;
		short const * shortP;
		unsigned short const * ushortP;
		WORD const * wordP;
		signed const * intP;
		unsigned const * uintP;
		DWORD const * dwordP;
		long const * longP;
		unsigned long const * ulongP;
		Colour const * colourP;
		
		inline PtrUnionConst(){}
		inline PtrUnionConst(void const * _voidP):voidP(_voidP){}
		inline PtrUnionConst(PtrUnion _uP):voidP(_uP.voidP){}
		inline operator void const * () const { return voidP; }
	};
	
	/***************************************/
	/* Generic copying to surface function */
	/***************************************/
	
	template<class CONVERT, class SRCTYPE>
	class GenericConvertRow
	{
		public:
			static void Do (PtrUnion _dstRowP, unsigned _dstWidth, SRCTYPE const * _srcRowP, unsigned _srcWidth, Colour const * _paletteP = NULL db_code1(DB_COMMA unsigned _paletteSize = 0));
	};

	template<class CONVERT, class SRCTYPE>
	void GenericConvertRow<CONVERT, SRCTYPE>::Do (PtrUnion _dstRowP, unsigned _dstWidth, SRCTYPE const * _srcRowP, unsigned _srcWidth, Colour const * _paletteP db_code1(DB_COMMA unsigned _paletteSize))
	{
		switch (pixelFormat.bitsPerPixel)
		{
			default:
				CANT_HAPPEN
			case 16:
			{
				db_assert1(!pixelFormat.palettizedB);
				for (unsigned colcount = _srcWidth; colcount; --colcount)
				{
					*_dstRowP.wordP++ = static_cast<WORD>(CONVERT::DoConv(_srcRowP++,_paletteP db_code1(DB_COMMA _paletteSize)));
				}
				if (_srcWidth<_dstWidth)
					*_dstRowP.wordP = static_cast<WORD>(CONVERT::DoConv(_srcRowP-1,_paletteP db_code1(DB_COMMA _paletteSize)));
				break;
			}
			case 24:
			{
				db_assert1(!pixelFormat.palettizedB);
				union { DWORD dw; BYTE b[3]; } u;
				for (unsigned colcount = _srcWidth; colcount; --colcount)
				{
					u.dw = static_cast<DWORD>(CONVERT::DoConv(_srcRowP++,_paletteP db_code1(DB_COMMA _paletteSize)));
					*_dstRowP.byteP++ = u.b[0];
					*_dstRowP.byteP++ = u.b[1];
					*_dstRowP.byteP++ = u.b[2];
				}
				if (_srcWidth<_dstWidth)
				{
					*_dstRowP.byteP++ = u.b[0];
					*_dstRowP.byteP++ = u.b[1];
					*_dstRowP.byteP = u.b[2];
				}
				break;
			}
			case 32:
			{
				db_assert1(!pixelFormat.palettizedB);
				for (unsigned colcount = _srcWidth; colcount; --colcount)
				{
					*_dstRowP.dwordP++ = static_cast<DWORD>(CONVERT::DoConv(_srcRowP++,_paletteP db_code1(DB_COMMA _paletteSize)));
				}
				if (_srcWidth<_dstWidth)
					*_dstRowP.dwordP = static_cast<DWORD>(CONVERT::DoConv(_srcRowP-1,_paletteP db_code1(DB_COMMA _paletteSize)));
				break;
			}
			case 8:
			{
				for (unsigned colcount = _srcWidth; colcount; --colcount)
				{
					*_dstRowP.byteP++ = static_cast<BYTE>(CONVERT::DoConv(_srcRowP++,_paletteP db_code1(DB_COMMA _paletteSize)));
				}
				if (_srcWidth<_dstWidth)
					*_dstRowP.byteP = static_cast<BYTE>(CONVERT::DoConv(_srcRowP-1,_paletteP db_code1(DB_COMMA _paletteSize)));
				break;
			}
			case 1:
			case 2:
				db_assert1(pixelFormat.palettizedB);
			case 4:
			{
				unsigned shift=0;
				unsigned val=0;
				--_dstRowP.byteP; // decrement here because we increment before the first write
				for (unsigned colcount = _srcWidth; colcount; --colcount)
				{
					val = CONVERT::DoConv(_srcRowP++,_paletteP db_code1(DB_COMMA _paletteSize));
					if (!shift)
						*++_dstRowP.byteP = static_cast<BYTE>(val);
					else
						*_dstRowP.byteP |= static_cast<BYTE>(val<<shift);
					shift += pixelFormat.bitsPerPixel;
					shift &= 7;
				}
				if (_srcWidth<_dstWidth)
				{
					if (!shift)
						*++_dstRowP.byteP = static_cast<BYTE>(val);
					else
						*_dstRowP.byteP |= static_cast<BYTE>(val<<shift);
				}
				break;
			}
		}
	}
	
	// reference counting support
	class RefCntObj
	{
		public:
			unsigned AddRef() { return ++m_nRefCnt; }
			unsigned Release() { if (0==(--m_nRefCnt)) { delete this; return 0;} else return m_nRefCnt; }
		protected:
			virtual ~RefCntObj(){
				#ifndef NDEBUG
					DbForget(this);
				#endif
			}
			RefCntObj() : m_nRefCnt(1){
				#ifndef NDEBUG
					DbRemember(this);
				#endif
			}
			RefCntObj(RefCntObj const &) : m_nRefCnt(1){
				#ifndef NDEBUG
					DbRemember(this);
				#endif
			}
			RefCntObj & operator = (RefCntObj const &){ return *this;}
		private:		
			unsigned m_nRefCnt;
			
		#ifndef NDEBUG
			friend void DbRemember(RefCntObj * pObj);
			friend void DbForget(RefCntObj * pObj);
			friend class AllocList;
		#endif
	};
	
	SurfUnion LoadFromParams(CreateTextureParms *);

} // namespace AwTl

struct AwBackupTexture : public AwTl::RefCntObj
{
	public:
		AwTl::SurfUnion Restore(AwTl::CreateTextureParms const & rParams);
	protected:
		AwTl::SurfUnion CreateTexture(AwTl::CreateTextureParms const & rParams);
	
		void ChoosePixelFormat(AwTl::CreateTextureParms const & rParams);
		
		virtual ~AwBackupTexture(){}
		
		// return the number of unique colours in the image or zero if this cannot be determined
		virtual unsigned GetNumColours() = 0;
		
		// return the smallest palette size that is available for the image
		virtual unsigned GetMinPaletteSize() = 0;
		
		// return true if the image has a single transparent colour
		virtual bool HasTransparentMask(bool bDefault);
		
		// called when a backup texture is about to be used for restoring, but after the above two functions have been called
		virtual void OnBeginRestoring(unsigned nMaxPaletteSize);
		
		virtual AwTl::Colour * GetPalette() = 0;
		
		virtual bool AreRowsReversed();
		
		virtual AwTl::PtrUnion GetRowPtr(unsigned nRow) = 0;
		
		virtual void LoadNextRow(AwTl::PtrUnion pRow) = 0;
		
		virtual void ConvertRow(AwTl::PtrUnion pDest, unsigned nDestWidth, AwTl::PtrUnionConst pSrc, unsigned nSrcOffset, unsigned nSrcWidth, AwTl::Colour * pPalette db_code1(DB_COMMA unsigned nPaletteSize));
		
		virtual DWORD GetTransparentColour();
		
		virtual void OnFinishRestoring(bool bSuccess);
		
		// metrics
		unsigned m_nWidth;
		unsigned m_nHeight;
		unsigned m_nPaletteSize; // 0 inicates no palette

		unsigned m_fFlags;
		
	private:
		bool m_bTranspMask;
		
	friend AwTl::SurfUnion AwTl::LoadFromParams(AwTl::CreateTextureParms *);
};

namespace AwTl {

	class TypicalBackupTexture : public ::AwBackupTexture
	{
		public:
			TypicalBackupTexture(AwBackupTexture const & rBase, PtrUnion * ppPixMap, Colour * pPalette)
				: AwBackupTexture(rBase)
				, m_ppPixMap(ppPixMap)
				, m_pPalette(pPalette)
			{}
			
			virtual ~TypicalBackupTexture()
			{
				if (m_pPalette)
				{
					delete[] m_pPalette;
					if (m_ppPixMap)
					{
						delete[] m_ppPixMap->byteP;
						delete[] m_ppPixMap;
					}
				}
				else
				{
					if (m_ppPixMap)
					{
						delete[] m_ppPixMap->colourP;
						delete[] m_ppPixMap;
					}
				}
			}
			
			virtual Colour * GetPalette();
			
			virtual PtrUnion GetRowPtr(unsigned nRow);
			
			virtual void LoadNextRow(PtrUnion pRow);
			
			// note: the palette size member must be set in
			// LoadHeaderInfo() for these functions to work correctly
			virtual unsigned GetNumColours();
			
			virtual unsigned GetMinPaletteSize();
			
		private:
			PtrUnion * m_ppPixMap;
			Colour * m_pPalette;
	};

	class TexFileLoader : public AwBackupTexture
	{
		public:
			SurfUnion Load(MediaMedium * pMedium, CreateTextureParms const & rParams);

		protected:
			// standard constructor
			// & destructor
			
			// Interface Functions. Each overridden version should set awTlLastErr
			// when an error occurs
		
			// Called to set the width and height members; the palette size member can also be safely set at this point.
			// Neither the width height or palette size members need actually be set until AllocateBuffers returns
			virtual void LoadHeaderInfo(MediaMedium * pMedium) = 0;
			
			// should ensure that the palette size is set
			virtual void AllocateBuffers(bool bWantBackup, unsigned nMaxPaletteSize) = 0;
			
			virtual void OnFinishLoading(bool bSuccess);
			
			virtual AwBackupTexture * CreateBackupTexture() = 0;
	};

	class TypicalTexFileLoader : public TexFileLoader
	{
		protected:
			TypicalTexFileLoader() : m_pPalette(NULL), m_ppPixMap(NULL), m_pRowBuf(NULL) {}
		
			virtual ~TypicalTexFileLoader();
		
			virtual unsigned GetNumColours();
			
			virtual unsigned GetMinPaletteSize();
		
			virtual void AllocateBuffers(bool bWantBackup, unsigned nMaxPaletteSize);
			
			virtual PtrUnion GetRowPtr(unsigned nRow);
			
			virtual AwBackupTexture * CreateBackupTexture();
		
			Colour * m_pPalette;
		private:
			PtrUnion * m_ppPixMap;
			
			PtrUnion m_pRowBuf;
	};
	
	extern void RegisterLoader(char const * pszMagic, AwTl::TexFileLoader * (* pfnCreate) () );
	
} // namespace AwTl

#define AWTEXLD_IMPLEMENT_DYNCREATE(pszMagic, tokenClassName) _AWTEXLD_IMPLEMENT_DYNCREATE_LINE_EX(pszMagic,tokenClassName,__LINE__)
#define _AWTEXLD_IMPLEMENT_DYNCREATE_LINE_EX(pszMagic, tokenClassName, nLine) _AWTEXLD_IMPLEMENT_DYNCREATE_LINE(pszMagic,tokenClassName,nLine)

#define _AWTEXLD_IMPLEMENT_DYNCREATE_LINE(pszMagic,tokenClassName,nLine) \
	AwTl::TexFileLoader * AwTlCreateClassObject ##_## tokenClassName ##_## nLine () { \
		return new tokenClassName; \
	} \
	class AwTlRegisterLoaderClass ##_## tokenClassName ##_## nLine { \
		public: AwTlRegisterLoaderClass ##_## tokenClassName ##_## nLine () { \
			AwTl::RegisterLoader(pszMagic, AwTlCreateClassObject ##_## tokenClassName ##_## nLine); \
		} \
	} rlc ## tokenClassName ##_## nLine;

#endif // ! _INCLUDED_AWTEXLD_HPP_
