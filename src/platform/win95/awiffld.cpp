#include "fixer.h"

#ifndef DB_LEVEL
#define DB_LEVEL 4
#endif
#include "db.h"
#ifndef DB_COMMA
	#define DB_COMMA ,
#endif

#include "awtexld.hpp"

#include "iff.hpp"
#include "iff_ilbm.hpp"

#include "list_tem.hpp"

#include <limits.h>

// conversion functors for IFF loader
class AwIffConvNull
{
	public:
		static inline unsigned DoConv (unsigned const * _colP, AwTl::Colour const * db_code1(DB_COMMA unsigned))
		{
			db_assert1(AwTl::pixelFormat.palettizedB);
			return *_colP;
		}
};

class AwIffConvNonTransp : public AwTl::Colour::ConvNonTransp
{
	public:
		static inline unsigned DoConv(unsigned const * pCol, AwTl::Colour const * pPalette db_code1(DB_COMMA unsigned nPaletteSize))
		{
			db_assert1(pPalette);
			db_onlyassert1(*pCol < nPaletteSize);
			return AwTl::Colour::ConvNonTransp::DoConv(&pPalette[*pCol]);
		}
};

class AwIffConvTransp
{
	public:
		static unsigned iTranspCol; // the index of the transparent colour
		static unsigned rawTranspCol; // the value of a transparent pixel on the surface

		static inline unsigned DoConv(unsigned const * pCol, AwTl::Colour const * pPalette db_code1(DB_COMMA unsigned nPaletteSize))
		{
			using namespace AwTl;

			if (*pCol == iTranspCol) return rawTranspCol;
			unsigned rv = AwIffConvNonTransp::DoConv(pCol,pPalette db_code1(DB_COMMA nPaletteSize));
			if (rv != rawTranspCol) return rv;

			// make the colour non-transparent (nb: only an occasional case)
			
			// OK, Here's the plan:
			
			// First, suppose that I were to decrease either the Red, Green or Blue
			// component in order to make the colour non-transparent.
			// Taking Red as an example, I'll work out what the resultant red value
			// will be if I decrease red to achieve a non-transparent colour
			// I'll compare this to the actual red value of the colour in question
			// The lower the difference, the better it'll be (ie. closer to the
			// original colour).
			// Obviously, I'll do the same for Green and Blue
			// Then I'll repeat the process but this time considering increasing
			// the components.
			// I'll then have six values which are scores (the lower the better)
			// for what colour component to change and how to change it.
			// If any of the components cannot be decreased (ie. their resulting
			// value is already zero) or increased (ie. their resulting value
			// is at maximum), then I'll set the corresponding score to
			// UINT_MAX so that that colour-changing operation won't be selected
			// (because that'll be the one that buggers everything up).
			
			unsigned nRedDiffDown   =
				(pPalette[*pCol].r < 1<<pixelFormat.redRightShift  ) ? UINT_MAX
					: (pPalette[*pCol].r & (1<<pixelFormat.redRightShift  )-1) + ((1<<pixelFormat.redRightShift  )+1)/2;
					
			unsigned nGreenDiffDown =
				(pPalette[*pCol].g < 1<<pixelFormat.greenRightShift) ? UINT_MAX
					: (pPalette[*pCol].g & (1<<pixelFormat.greenRightShift)-1) + ((1<<pixelFormat.greenRightShift)+1)/2;
					
			unsigned nBlueDiffDown  =
				(pPalette[*pCol].b < 1<<pixelFormat.blueRightShift ) ? UINT_MAX
					: (pPalette[*pCol].b & (1<<pixelFormat.blueRightShift )-1) + ((1<<pixelFormat.blueRightShift )+1)/2;
			
			unsigned nRedDiffUp   =
				(pPalette[*pCol].r >= (255 & ~((1<<pixelFormat.redRightShift  )-1) ) ? UINT_MAX
					: (1<<pixelFormat.redRightShift  )*3/2 - (pPalette[*pCol].r & (1<<pixelFormat.redRightShift  )-1));
					
			unsigned nGreenDiffUp =
				(pPalette[*pCol].g >= (255 & ~((1<<pixelFormat.greenRightShift)-1) ) ? UINT_MAX
					: (1<<pixelFormat.greenRightShift)*3/2 - (pPalette[*pCol].g & (1<<pixelFormat.greenRightShift)-1));
					
			unsigned nBlueDiffUp  =
				(pPalette[*pCol].b >= (255 & ~((1<<pixelFormat.blueRightShift )-1) ) ? UINT_MAX
					: (1<<pixelFormat.blueRightShift )*3/2 - (pPalette[*pCol].b & (1<<pixelFormat.blueRightShift )-1));
			
			// Pick lowest value and do the business
			Colour colAdj = pPalette[*pCol];
			if
			(
				   nBlueDiffUp <= nBlueDiffDown
				&& nBlueDiffUp <= nRedDiffUp
				&& nBlueDiffUp <= nRedDiffDown
				&& nBlueDiffUp <= nGreenDiffUp
				&& nBlueDiffUp <= nGreenDiffDown
			)
			{
				colAdj.b += static_cast<unsigned char>(1<<pixelFormat.blueRightShift);
			}
			else if
			(
				   nBlueDiffDown <= nRedDiffUp
				&& nBlueDiffDown <= nRedDiffDown
				&& nBlueDiffDown <= nGreenDiffUp
				&& nBlueDiffDown <= nGreenDiffDown
			)
			{
				colAdj.b -= static_cast<unsigned char>(1<<pixelFormat.blueRightShift);
			}
			else if
			(
				   nRedDiffUp <= nRedDiffDown
				&& nRedDiffUp <= nGreenDiffUp
				&& nRedDiffUp <= nGreenDiffDown
			)
			{
				colAdj.r += static_cast<unsigned char>(1<<pixelFormat.redRightShift);
			}
			else if
			(
				   nRedDiffDown <= nGreenDiffUp
				&& nRedDiffDown <= nGreenDiffDown
			)
			{
				colAdj.r -= static_cast<unsigned char>(1<<pixelFormat.redRightShift);
			}
			else if (nGreenDiffUp <= nGreenDiffDown)
			{
				colAdj.g += static_cast<unsigned char>(1<<pixelFormat.greenRightShift);
			}
			else
			{
				colAdj.g -= static_cast<unsigned char>(1<<pixelFormat.greenRightShift);
			}
			
			return Colour::ConvNonTransp::DoConv(&colAdj);
		}
};

unsigned AwIffConvTransp::iTranspCol;
unsigned AwIffConvTransp::rawTranspCol;

// IFF Loader

class AwIffLoader : public AwTl::TexFileLoader
{
	public:
		AwIffLoader() : m_pPalette(NULL), m_bDecoding(false) {}
	protected:
		virtual ~AwIffLoader();
	
		virtual void LoadHeaderInfo(MediaMedium * pMedium);

		virtual unsigned GetNumColours();
		
		virtual unsigned GetMinPaletteSize();

		virtual bool HasTransparentMask(bool bDefault);
		
		virtual void AllocateBuffers(bool bWantBackup, unsigned nMaxPaletteSize);
		
		virtual void OnBeginRestoring(unsigned nMaxPaletteSize);
		
		virtual AwTl::Colour * GetPalette();
		
		
		virtual AwTl::PtrUnion GetRowPtr(unsigned nRow);
		
		virtual void LoadNextRow(AwTl::PtrUnion pRow);
		
		virtual void ConvertRow(AwTl::PtrUnion pDest, unsigned nDestWidth, AwTl::PtrUnionConst pSrc, unsigned nSrcOffset, unsigned nSrcWidth, AwTl::Colour * pPalette db_code1(DB_COMMA unsigned nPaletteSize));
		
		virtual DWORD GetTransparentColour();
		
		virtual void OnFinishLoading(bool bSuccess);
		
		virtual void OnFinishRestoring(bool bSuccess);
		
		virtual AwBackupTexture * CreateBackupTexture();
		
	private:
		static bool Enumerator(IFF::Chunk * pChunk, void * pData);
		// list of chunks found by enumerator
		List<IFF::IlbmBodyChunk *> m_listBodyChunks;
		
		// smallest and largest palette sizes of versions of this image
		unsigned m_nMaxPaletteSize;
		unsigned m_nMinPaletteSize;
		
		// buffer for palette -
		// since the IFF cmap table is in a different format to what the Aw loaders require
		// (maybe should think about standardizing the data types?)
		AwTl::Colour * m_pPalette;
		
		// iff data
		IFF::File m_ifData;
		IFF::IlbmBmhdChunk * m_pHdr;
		IFF::IlbmCmapChunk * m_pCmap;
		IFF::IlbmBodyChunk * m_pBody;
		
		bool m_bDecoding;
};

AwIffLoader::~AwIffLoader()
{
	if (m_pPalette) delete[] m_pPalette;
}

void AwIffLoader::LoadHeaderInfo(MediaMedium * pMedium)
{
	db_log4("\tLoading an IFF file");
	
	while (m_listBodyChunks.size())
		m_listBodyChunks.delete_first_entry();
	
	if (!m_ifData.Load(pMedium) || !m_ifData.GetContents())
	{
//		if (NO_ERROR == (awTlLastWinErr = GetLastError()))
			awTlLastErr = AW_TLE_BADFILEDATA;
//		else
//			awTlLastErr = AW_TLE_CANTREADFILE;
			
		db_log3("AwCreateTexture() [AwIffLoader::LoadHeaderInfo] : ERROR: IFF file load failed");
	}
	else
	{
		m_nMinPaletteSize = UINT_MAX;
		m_nMaxPaletteSize = 0;
		m_ifData.GetContents()->EnumChildren("ILBM","BODY",Enumerator,this);
	}
}

unsigned AwIffLoader::GetNumColours()
{
	return m_nMaxPaletteSize;
}

unsigned AwIffLoader::GetMinPaletteSize()
{
	return m_nMinPaletteSize;
}

void AwIffLoader::AllocateBuffers(bool /*bWantBackup*/, unsigned nMaxPaletteSize)
{
	// we will need to allocate buffers when restoring as well as first-time loading
	// so allocate buffers in OnBeginRestoring() which we'll call here

	OnBeginRestoring(nMaxPaletteSize);
}

bool AwIffLoader::Enumerator(IFF::Chunk * pChunk, void * pData)
{
	db_assert1(pChunk);
	db_assert1(pData);
	AwIffLoader * pThis = static_cast<AwIffLoader *>(pData);
	
	IFF::Chunk * pCmap = pChunk->GetProperty("CMAP");
	IFF::Chunk * pHdr = pChunk->GetProperty("BMHD");
	if (pCmap && pHdr) // must have these two properties
	{
		unsigned nThisPaletteSize = static_cast<IFF::IlbmCmapChunk *>(pCmap)->nEntries;
		db_logf4(("\tfound a %u colour %scompressed IFF body chunk",nThisPaletteSize,static_cast<IFF::IlbmBmhdChunk *>(pHdr)->eCompression ? "" : "un"));
		
		pThis->m_listBodyChunks.add_entry(static_cast<IFF::IlbmBodyChunk *>(pChunk));
		
		if (nThisPaletteSize < pThis->m_nMinPaletteSize)
			pThis->m_nMinPaletteSize = nThisPaletteSize;
		
		if (nThisPaletteSize > pThis->m_nMaxPaletteSize)
			pThis->m_nMaxPaletteSize = nThisPaletteSize;
	}
	else db_log3("AwCreateTexture(): WARNING: IFF body chunk found with insufficient associated property chunks");
	
	return true; // continue enumeration
}

void AwIffLoader::OnBeginRestoring(unsigned nMaxPaletteSize)
{
	using namespace AwTl;
	
	if (m_listBodyChunks.size())
	{
		// if decodeing, m_pBody will be valid
		if (m_bDecoding)
		{
			m_pBody->EndDecode();
			m_bDecoding = false;
		}
		
		m_pBody = NULL;
		unsigned nBestPaletteSize = 0;
		
		for (LIF<IFF::IlbmBodyChunk *> itChunks(&m_listBodyChunks); !itChunks.done(); itChunks.next())
		{
			IFF::IlbmCmapChunk * pCmap = static_cast<IFF::IlbmCmapChunk *>(itChunks()->GetProperty("CMAP"));
			db_assert1(pCmap);
			
			if ((!nMaxPaletteSize || pCmap->nEntries <= nMaxPaletteSize) && pCmap->nEntries > nBestPaletteSize)
			{
				m_pBody = itChunks();
				m_pCmap = pCmap;
				nBestPaletteSize = pCmap->nEntries;
			}
		}
		
		if (m_pBody)
		{
			m_pHdr = static_cast<IFF::IlbmBmhdChunk *>(m_pBody->GetProperty("BMHD"));
			// delete old buffers
			if (m_pPalette) delete[] m_pPalette;
			// allocate buffer for palette, make it extra big to cope with corrupt files
			unsigned nAllocPaletteSize = m_pCmap->nEntries;
			unsigned nAllocPaletteSizeShift = 0;
			while (0!=(nAllocPaletteSize >>= 1))
			{
				++nAllocPaletteSizeShift;
			}
			m_pPalette = new Colour [2<<nAllocPaletteSizeShift]; // prevent corrupt data causing a crash
			//m_pPalette = new Colour [m_pCmap->nEntries];
			// copy the palette
			for (unsigned i=0; i<m_pCmap->nEntries; ++i)
			{
				// hacked testa
				m_pPalette[i].r = m_pCmap->pTable[i].r;
				m_pPalette[i].g = m_pCmap->pTable[i].g;
				m_pPalette[i].b = m_pCmap->pTable[i].b;
			}
			// set the width height and palette size in the base class
			m_nPaletteSize = m_pCmap->nEntries;
			m_nWidth = m_pHdr->width;
			m_nHeight = m_pHdr->height;
			// prepare to decode the data			
			m_pBody->BeginDecode();
			m_bDecoding = true;
			// set the transparent mask colours
			switch (m_pHdr->eMasking)
			{
				case IFF::IlbmBmhdChunk::MASK_NONE:
					break;
				case IFF::IlbmBmhdChunk::MASK_TRANSPARENTCOL:
					AwIffConvTransp::iTranspCol = m_pHdr->iTranspCol;
					if (pixelFormat.palettizedB)
						AwIffConvTransp::rawTranspCol = AwIffConvTransp::iTranspCol;
					else
						AwIffConvTransp::rawTranspCol = 
							 static_cast<unsigned>(m_pPalette[AwIffConvTransp::iTranspCol].r)>>pixelFormat.redRightShift<<pixelFormat.redLeftShift
							|static_cast<unsigned>(m_pPalette[AwIffConvTransp::iTranspCol].g)>>pixelFormat.greenRightShift<<pixelFormat.greenLeftShift
							|static_cast<unsigned>(m_pPalette[AwIffConvTransp::iTranspCol].b)>>pixelFormat.blueRightShift<<pixelFormat.blueLeftShift;
					break;
				default:
					db_log3("AwCreateTexture() [AwIffLoader::OnBeginRestoring] : ERROR: IFF mask field wrong");
					awTlLastErr = AW_TLE_BADFILEDATA;
			}
		}
		else
		{
	 		awTlLastErr = AW_TLE_CANTPALETTIZE; // no suitable chunk found
		 	db_log3("AwCreateTexture() [AwIffLoader::OnBeginRestoring] : ERROR: No suitable IFF body chunk found");
	 	}
	}
	else
	{
	 	awTlLastErr = AW_TLE_BADFILEDATA;
	 	db_log3("AwCreateTexture() [AwIffLoader::OnBeginRestoring] : ERROR: IFF file not loaded or contains no image data");
	}
}

AwTl::Colour * AwIffLoader::GetPalette()
{
	return m_pPalette;
}

bool AwIffLoader::HasTransparentMask(bool bDefault)
{
	if (m_listBodyChunks.size())
	{
		IFF::IlbmBmhdChunk * pHdr = static_cast<IFF::IlbmBmhdChunk *>(m_listBodyChunks.first_entry()->GetProperty("BMHD"));
		db_assert1(pHdr);
		return (IFF::IlbmBmhdChunk::MASK_TRANSPARENTCOL == pHdr->eMasking);
	}
	else
		return bDefault;
}

DWORD AwIffLoader::GetTransparentColour()
{
	return AwIffConvTransp::rawTranspCol;
}

AwTl::PtrUnion AwIffLoader::GetRowPtr(unsigned /*nRow*/)
{
	// the iff object has an internal buffer to which a pointer
	// is returned when we decode each row
	// unfortunately we have to cast constness away, but never mind
	return const_cast<unsigned *>(m_pBody->DecodeNextRow());
}

void AwIffLoader::LoadNextRow(AwTl::PtrUnion /*pRow*/)
{
	// GetRowPtr() has called DecodeNextRow()
	// which has filled in the data already
	// so do nothing here
}

void AwIffLoader::ConvertRow(AwTl::PtrUnion pDest, unsigned nDestWidth, AwTl::PtrUnionConst pSrc, unsigned nSrcOffset, unsigned nSrcWidth, AwTl::Colour * pPalette db_code1(DB_COMMA unsigned nPaletteSize))
{
	using namespace AwTl;
	
	// we have overridden this function for two reasons:
	// 1. The data type for each texel in the row is unsigned int
	//    to allow for the fact that all images are palettized
	//    with no limit on the palette size. The default
	//    implementation would assume BYTE (unsigned char)
	// 2. The transparency flag and colour is stored in the
	//    file and the transparency flag passed to AwCreateTexture()
	//    is ignored. The transparent colour does not have to be 0,0,0
	//    either
	
	db_assert1(pPalette);
	db_assert1(pPalette == m_pPalette);
	// we can still use GenericConvertRow though, using the conversion functors above
	
	if (pixelFormat.palettizedB)
	{
		GenericConvertRow<AwIffConvNull,unsigned>::Do(pDest, nDestWidth, pSrc.uintP+nSrcOffset, nSrcWidth);
	}
	else
	{
		switch (m_pHdr->eMasking)
		{
			case IFF::IlbmBmhdChunk::MASK_NONE:
				GenericConvertRow<AwIffConvNonTransp,unsigned>::Do(pDest, nDestWidth, pSrc.uintP+nSrcOffset, nSrcWidth, pPalette db_code1(DB_COMMA nPaletteSize));
				break;
			case IFF::IlbmBmhdChunk::MASK_TRANSPARENTCOL:
				GenericConvertRow<AwIffConvTransp,unsigned>::Do(pDest, nDestWidth, pSrc.uintP+nSrcOffset, nSrcWidth, pPalette db_code1(DB_COMMA nPaletteSize));
				break;
			default:
				db_log3("AwCreateTexture() [AwIffLoader::ConvertRow] : ERROR: IFF mask field wrong");
				awTlLastErr = AW_TLE_BADFILEDATA;
		}
	}
}

void AwIffLoader::OnFinishLoading(bool /*bSuccess*/)
{
	if (m_bDecoding)
	{
		m_pBody->EndDecode();
		m_bDecoding = false;
	}
}

void AwIffLoader::OnFinishRestoring(bool /*bSuccess*/)
{
	if (m_bDecoding)
	{
		m_pBody->EndDecode();
		m_bDecoding = false;
	}
}
		
AwBackupTexture * AwIffLoader::CreateBackupTexture()
{
	// use the same object for restoring
	AddRef();
	return this;
}

// Valid file ID fields: 'FORM' 'LIST' 'CAT ' - we can load them all

AWTEXLD_IMPLEMENT_DYNCREATE("FORM",AwIffLoader)
AWTEXLD_IMPLEMENT_DYNCREATE("LIST",AwIffLoader)
AWTEXLD_IMPLEMENT_DYNCREATE("CAT ",AwIffLoader)
