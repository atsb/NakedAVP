#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "fixer.h"

#ifndef DB_LEVEL
#define DB_LEVEL 4
#endif
#include "db.h"

#include "awtexld.hpp"

// PNM loaders

class AwPnmLoader : public AwTl::TypicalTexFileLoader
{
	protected:
		void _AWTL_VARARG ParseHeader(unsigned nFields,...);
		
		MediaMedium * m_pMedium;
};

void _AWTL_VARARG AwPnmLoader::ParseHeader(unsigned nFields,...)
{
	va_list ap;
	va_start(ap,nFields);
	
	m_pMedium->MovePos(+2); // skip past magic
	
	BYTE c = 0;
	while (nFields)
	{
		unsigned * fieldP = va_arg(ap,unsigned *);
		bool comment = false;
		bool done = false;
		do
		{
			MediaRead(m_pMedium, &c);
			switch (c)
			{
				case '\n':
					comment = false;
					break;
				case '#':
					comment = true;
					break;
				default:
					if (!comment && !isspace(c))
						done = true;
			}
		}
			while (!done);
		char bufA[512];
		char * bufP = bufA;
		do
		{
			*bufP++ = c;
			MediaRead(m_pMedium, &c);
		}
			while (!isspace(c));
		*bufP = 0;
		*fieldP = atoi(bufA);

		-- nFields;
	}
	// c should now be a newline character
	if ('\n'!=c)
		awTlLastErr = AW_TLE_BADFILEDATA;
	
	va_end(ap);
}

class AwPpmLoader : public AwPnmLoader
{
	protected:
		virtual void LoadHeaderInfo(MediaMedium * pMedium);
		virtual AwTl::Colour * GetPalette();
		virtual void LoadNextRow(AwTl::PtrUnion pRow);
		
		unsigned pm_maxval;
};

void AwPpmLoader::LoadHeaderInfo(MediaMedium * pMedium)
{
	m_pMedium = pMedium;
	
	db_log4("\tLoading a PPM file");
	
	ParseHeader(3,&m_nWidth,&m_nHeight,&pm_maxval);
	
	db_logf4(("\tPPM_maxval is %u",pm_maxval));
	if (pm_maxval > 255)
	{
//		awTlLastErr = AW_TLE_BADFILEFORMAT;
		db_log3("AwCreateTexture() [AwPpmLoader::LoadHeaderInfo] : PPM_maxval too large");
	}
	
	m_nPaletteSize = 0;
}

AwTl::Colour * AwPpmLoader::GetPalette()
{
	// never palettized
	return NULL;
}

void AwPpmLoader::LoadNextRow(AwTl::PtrUnion pRow)
{
	if (pm_maxval != 255)
		for (unsigned colcount = m_nWidth; colcount; --colcount)
		{
			BYTE byte;
			MediaRead(m_pMedium,&byte);
			pRow.colourP->r = static_cast<BYTE>(static_cast<unsigned>(byte)*255/pm_maxval);
			MediaRead(m_pMedium,&byte);
			pRow.colourP->g = static_cast<BYTE>(static_cast<unsigned>(byte)*255/pm_maxval);
			MediaRead(m_pMedium,&byte);
			pRow.colourP->b = static_cast<BYTE>(static_cast<unsigned>(byte)*255/pm_maxval);
			++pRow.colourP;
		}
	else
		for (unsigned colcount = m_nWidth; colcount; --colcount)
		{
			MediaRead(m_pMedium,&pRow.colourP->r);
			MediaRead(m_pMedium,&pRow.colourP->g);
			MediaRead(m_pMedium,&pRow.colourP->b);
			++pRow.colourP;
		}
}


class AwPgmLoader : public AwPnmLoader
{
	protected:
		virtual void LoadHeaderInfo(MediaMedium * pMedium);
		virtual AwTl::Colour * GetPalette();
		virtual void LoadNextRow(AwTl::PtrUnion pRow);
		
		unsigned pm_maxval;
};

void AwPgmLoader::LoadHeaderInfo(MediaMedium * pMedium)
{
	m_pMedium = pMedium;
	
	db_log4("\tLoading a PGM file");
	
	ParseHeader(3,&m_nWidth,&m_nHeight,&pm_maxval);
	
	db_logf4(("\tPGM_maxval is %u",pm_maxval));
	if (pm_maxval > 255)
	{
		awTlLastErr = AW_TLE_BADFILEFORMAT;
		db_log3("AwCreateTexture() [AwPgmLoader::LoadHeaderInfo] : PGM_maxval too large");
	}
	
	m_nPaletteSize = pm_maxval+1;
}

AwTl::Colour * AwPgmLoader::GetPalette()
{
	db_assert1(m_nPaletteSize);
	db_assert1(m_pPalette);
	
	unsigned step8 = (256*255)/pm_maxval;
	unsigned val8 = 127;
	AwTl::Colour * pmP = m_pPalette;
	for (unsigned pc = m_nPaletteSize; pc; --pc,++pmP,val8+=step8)
		pmP->r = pmP->g = pmP->b = static_cast<BYTE>(val8/256);
		
	return m_pPalette;
}

void AwPgmLoader::LoadNextRow(AwTl::PtrUnion pRow)
{
	m_pMedium->ReadBlock(pRow,m_nWidth);
}


class AwPbmLoader : public AwPnmLoader
{
	protected:
		virtual void LoadHeaderInfo(MediaMedium * pMedium);
		virtual AwTl::Colour * GetPalette();
		virtual void LoadNextRow(AwTl::PtrUnion pRow);
};


void AwPbmLoader::LoadHeaderInfo(MediaMedium * pMedium)
{
	m_pMedium = pMedium;
	
	db_log4("\tLoading a PBM file");
	
	ParseHeader(2,&m_nWidth,&m_nHeight);
	
	m_nPaletteSize = 2;
}

AwTl::Colour * AwPbmLoader::GetPalette()
{
	db_assert1(m_nPaletteSize);
	db_assert1(m_pPalette);
	
	m_pPalette[0].r = 0;
	m_pPalette[0].g = 0;
	m_pPalette[0].b = 0;
	m_pPalette[1].r = 255;
	m_pPalette[1].g = 255;
	m_pPalette[1].b = 255;
	
	return m_pPalette;
}

void AwPbmLoader::LoadNextRow(AwTl::PtrUnion pRow)
{
	unsigned shift = 0;
	BYTE byte = 0;
	
	for (unsigned colcount = m_nWidth; colcount; --colcount)
	{
		if (!shift)
		{
			shift = 8;
			MediaRead(m_pMedium,&byte);
			byte = (BYTE) ~byte;
		}
		--shift;
		*pRow.byteP++ = static_cast<BYTE>(byte>>shift & 1);
	}
}

AWTEXLD_IMPLEMENT_DYNCREATE("P6",AwPpmLoader)
AWTEXLD_IMPLEMENT_DYNCREATE("P5",AwPgmLoader)
AWTEXLD_IMPLEMENT_DYNCREATE("P4",AwPbmLoader)



