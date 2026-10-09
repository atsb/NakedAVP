#include "fixer.h"

#ifndef DB_LEVEL
#define DB_LEVEL 4
#endif
#include "db.h"

#include "awtexld.hpp"

// BMP Loader

class AwBmpLoader : public AwTl::TypicalTexFileLoader
{
	protected:
		virtual void LoadHeaderInfo(MediaMedium * pMedium);
		virtual AwTl::Colour * GetPalette();
		virtual bool AreRowsReversed();
		virtual void LoadNextRow(AwTl::PtrUnion pRow);
		
		WORD bmp_bitdepth;
		DWORD bmp_offset;
		DWORD bmp_headsize;
		
		unsigned bmp_filepitchpad;
		unsigned bmp_bitmask;
		
		MediaMedium * m_pMedium;
};

void AwBmpLoader::LoadHeaderInfo(MediaMedium * pMedium)
{
	m_pMedium = pMedium; // store for later
	
	db_log4("\tLoading a BMP file");

	pMedium->MovePos(+10); // 2 bytes BM (magic) and 4 bytes bmp_filesize and 4 bytes reserved
	
	MediaRead(pMedium,&bmp_offset);
	MediaRead(pMedium,&bmp_headsize);
	// 12 for OS/2 1.x 40 for Windows, 64 for OS/2 2.x
	if (12 != bmp_headsize && 40 != bmp_headsize && 64 != bmp_headsize)
	{
		awTlLastErr = AW_TLE_BADFILEFORMAT;
		db_logf3(("AwCreateTexture(): ERROR: BMP_headersize (%u) is not a recognized BMP format",bmp_headsize));
	}
	#if DB_LEVEL >= 4
	switch (bmp_headsize)
	{
		case 12:
			db_log4("\tBMP format is OS/2 1.x");
			break;
		case 40:
			db_log4("\tBMP format is Windows 3.x");
			break;
		case 64:
			db_log4("\tBMP format is OS/2 2.x");
	}
	#endif
	if (bmp_headsize >= 40)
	{
		DWORD width, height;
		MediaRead(pMedium,&width);
		MediaRead(pMedium,&height);
		m_nWidth = width;
		m_nHeight = height;
	}
	else
	{
		WORD width, height;
		MediaRead(pMedium,&width);
		MediaRead(pMedium,&height);
		m_nWidth = width;
		m_nHeight = height;
	}
	// next WORD is planes and should == 1
	WORD bmp_planes = 0;	// ALEX: added in initialization to prevent compiler warnings
	MediaRead(pMedium,&bmp_planes);
	if (1!=bmp_planes)
	{
		awTlLastErr = AW_TLE_BADFILEDATA;
		db_log3("AwCreateTexture(): ERROR: BMP_planes should be 1");
	}
	MediaRead(pMedium,&bmp_bitdepth);
	db_logf4(("\tBMP_bitdepth is %hd",bmp_bitdepth));
	if (1!=bmp_bitdepth && 2!=bmp_bitdepth && 4!=bmp_bitdepth && 8!=bmp_bitdepth && 24!=bmp_bitdepth)
	{
		awTlLastErr = AW_TLE_BADFILEDATA;
		db_logf3(("AwCreateTexture(): ERROR: BMP_bitdepth (%u) should be 1,2,4,8 or 24",bmp_bitdepth));
	}
	if (bmp_headsize >= 40)
	{
		// next DWORD is compression, not supported so only accept 0
		DWORD compression = 0;
		MediaRead(pMedium,&compression);
		if (compression)
		{
			db_log3("AwCreateTexture(): ERROR: Cannont read compressed BMP files");
			awTlLastErr = AW_TLE_BADFILEFORMAT;
		}
		// DWORD bmp_size - ignored, 2xDWORDS ignored
		pMedium->MovePos(+12);
		DWORD palette_size = 0;		// ALEX: added in initialization to prevent compiler warnings
		MediaRead(pMedium,&palette_size);
		m_nPaletteSize = palette_size;
		// skip to the start of the palette if there would be one
		pMedium->MovePos(bmp_headsize-36);
	}
	else
	{
		m_nPaletteSize = 0;
	}
	bmp_offset -= (bmp_headsize+14);
	if (!m_nPaletteSize && bmp_bitdepth<=8)
		m_nPaletteSize = 1<<bmp_bitdepth;
	if (m_nPaletteSize)
		bmp_offset -= bmp_headsize >= 40 ? m_nPaletteSize*4 : m_nPaletteSize*3;
		
	db_logf4(("\tBMP_palettesize is %u",m_nPaletteSize));
	bmp_filepitchpad = (~(m_nWidth*bmp_bitdepth-1))/8 & 3;
	db_logf4(("\tBMP has %u bytes padding per row",bmp_filepitchpad));
	
	bmp_bitmask = (1<<bmp_bitdepth)-1;
}

AwTl::Colour * AwBmpLoader::GetPalette()
{
	db_assert1(m_nPaletteSize);
	db_assert1(m_pPalette);
	
	if (m_nPaletteSize > 256)
	{
		awTlLastErr = AW_TLE_BADFILEDATA;
		db_log3("AwCreateTexture(): ERROR: BMP_palettesize is too large");
	}
	else
	{
		AwTl::Colour * pmP = m_pPalette;
		for (unsigned pc = m_nPaletteSize; pc; --pc,++pmP)
		{
			MediaRead(m_pMedium,&pmP->b);
			MediaRead(m_pMedium,&pmP->g);
			MediaRead(m_pMedium,&pmP->r);
			if (bmp_headsize >= 40) m_pMedium->MovePos(+1);
		}
	}
	// skip to the start of the pixel data
	m_pMedium->MovePos(bmp_offset);
	
	return m_pPalette;
}

bool AwBmpLoader::AreRowsReversed()
{
	return true;
}

void AwBmpLoader::LoadNextRow(AwTl::PtrUnion pRow)
{
	if (m_nPaletteSize)
	{
		unsigned shift = 0;
		BYTE byte = 0;	// Not needed.
		
		for (unsigned colcount = m_nWidth; colcount; --colcount)
		{
			if (!shift)
			{
				shift = 8;
				MediaRead(m_pMedium, &byte);
			}
			shift -= bmp_bitdepth;
			*pRow.byteP++ = static_cast<BYTE>(byte>>shift & bmp_bitmask);
		}
	}
	else
	{
		for (unsigned colcount = m_nWidth; colcount; --colcount)
		{
			MediaRead(m_pMedium,&pRow.colourP->b);
			MediaRead(m_pMedium,&pRow.colourP->g);
			MediaRead(m_pMedium,&pRow.colourP->r);
			++pRow.colourP;
		}
	}
	m_pMedium->MovePos(bmp_filepitchpad);
}

AWTEXLD_IMPLEMENT_DYNCREATE("BM",AwBmpLoader)
