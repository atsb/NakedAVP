#include "iff_ilbm.hpp"

IFF_IMPLEMENT_DYNCREATE("ILBM","BMHD",IlbmBmhdChunk)
IFF_IMPLEMENT_DYNCREATE("ILBM","CMAP",IlbmCmapChunk)
IFF_IMPLEMENT_DYNCREATE("ILBM","BODY",IlbmBodyChunk)
IFF_IMPLEMENT_DYNCREATE("ILBM","GRAB",IlbmGrabChunk)

namespace IFF
{
	void IlbmBmhdChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(width);
		pArchv->Transfer(height);
		pArchv->Transfer(xTopLeft);
		pArchv->Transfer(yTopLeft);
		pArchv->Transfer(nBitPlanes);
		pArchv->Transfer(eMasking);
		pArchv->Transfer(eCompression);
		pArchv->Transfer(flags);
		pArchv->Transfer(iTranspCol);
		pArchv->Transfer(xAspectRatio);
		pArchv->Transfer(yAspectRatio);
		pArchv->Transfer(xMax);
		pArchv->Transfer(yMax);
	}
	
	void IlbmCmapChunk::Serialize(Archive * pArchv)
	{
		if (pArchv->m_bIsLoading)
		{
			nEntries = pArchv->GetSize()/3;
			if (pTable) delete[] pTable;
			pTable = new RGBTriple [nEntries];
		}
		
		for (unsigned i=0; i<nEntries; ++i)
			pArchv->Transfer(pTable[i]);
	}
	
	IlbmCmapChunk::~IlbmCmapChunk()
	{
		if (pTable) delete[] pTable;
	}
	
	void IlbmBodyChunk::Serialize(Archive * pArchv)
	{
		if (pArchv->m_bIsLoading)
		{
			nSize = pArchv->GetSize();
			if (pData) delete[] pData;
			pData = new UBYTE [nSize];
		}
		
		pArchv->TransferBlock(pData,nSize);
	}

	bool IlbmBodyChunk::GetHeaderInfo() const
	{
		Chunk * pHdr = GetProperty("BMHD");
		if (!pHdr)
		{
			return false;
		}
		nWidth = static_cast<IlbmBmhdChunk *>(pHdr)->width;
		eCompression = static_cast<IlbmBmhdChunk *>(pHdr)->eCompression;
		nBitPlanes = static_cast<IlbmBmhdChunk *>(pHdr)->nBitPlanes;
		return true;
	}
	
	#ifndef IFF_READ_ONLY
		bool IlbmBodyChunk::BeginEncode()
		{
			if (!GetHeaderInfo())
			{
				return false;
			}
			
			pEncodeDst = new DataBlock;
			pEncodeSrc = new UBYTE [(nWidth+7)/8];
			
			nSizeNonCprss = 0;
			nSizeCprss = 0;
			
			return true;
		}

		bool IlbmBodyChunk::EncodeNextRow(unsigned const * pRow)
		{
			if (!pEncodeDst) return false;
			
			for (unsigned b=0; b<nBitPlanes; ++b)
			{
				UBYTE * pBuf = pEncodeSrc;
				
				unsigned byte=0;
				for (unsigned x=0; x<nWidth; ++x)
				{
					byte <<= 1;
					byte |= pRow[x]>>b & 1;
					
					if (7==(x&7)) *pBuf++ = static_cast<UBYTE>(byte);
				}
				if (nWidth & 7)
				{
					byte <<= 8-(nWidth & 7);
					*pBuf = static_cast<UBYTE>(byte);
				}
				
				if (eCompression)
				{
					nSizeNonCprss += (nWidth+7)/8;
					
					UBYTE const * pBuf = pEncodeSrc;
					unsigned i=(nWidth+7)/8;
					
					while (i)
					{
						if (1==i)
						{
							pEncodeDst->Append(0);
							pEncodeDst->Append(*pBuf);
							nSizeCprss += 2;
							i=0;
						}
						else if (i>1)
						{
							if (pBuf[0]==pBuf[1])
							{
								unsigned j=2;
								while (j<i && j<0x80 && pBuf[j-1]==pBuf[j])
									++j;
								pEncodeDst->Append(static_cast<UBYTE>(0x101-j));
								pEncodeDst->Append(*pBuf);
								pBuf += j;
								i -= j;
								nSizeCprss += 2;
							}
							else
							{
								unsigned j=2;
								while (j<i && j<0x80 && (pBuf[j]!=pBuf[j-1] || pBuf[j-1]!=pBuf[j-2]))
									++j;
								if (j<i && pBuf[j]==pBuf[j-1] && pBuf[j-1]==pBuf[j-2])
									j-=2;
								pEncodeDst->Append(static_cast<UBYTE>(j-1));
								pEncodeDst->Append(pBuf,j);
								pBuf += j;
								i -= j;
								nSizeCprss += j+1;
							}
						}
					}
					
				}
				else
				{
					pEncodeDst->Append(pEncodeSrc,(nWidth+7)/8);
				}
			}
			
			return true;
		}
		
		bool IlbmBodyChunk::EndEncode()
		{
			if (!pEncodeDst) return false;
			
			if (pData) delete[] pData;
			nSize = pEncodeDst->GetDataSize();
			pData = new UBYTE[nSize];
			
			memcpy(pData,pEncodeDst->GetDataPtr(),nSize);
			
			delete pEncodeDst;
			delete[] pEncodeSrc;
			
			pEncodeDst = NULL;
			pEncodeSrc = NULL;
			
			return true;
		}
	#endif
	
	bool IlbmBodyChunk::BeginDecode() const
	{
		if (pDecodeDst) delete[] pDecodeDst;
		if (!pData || !GetHeaderInfo())
		{
			pDecodeSrc = NULL;
			pDecodeDst = NULL;
			return false;
		}
		
		pDecodeSrc = pData;
		nRemaining = nSize;
		
		pDecodeDst = new unsigned [nWidth];
		
		return pData != NULL;
	}

	unsigned const * IlbmBodyChunk::DecodeNextRow() const
	{
		if (!pDecodeSrc || !pDecodeDst) return NULL;
		
		for (unsigned x=0; x<nWidth; ++x)
			pDecodeDst[x]=0;
			
		if (eCompression)
		{
			unsigned repcnt = 0;
			unsigned rawcnt = 0;
			
			for (unsigned b=0; b<nBitPlanes; ++b)
			{
				unsigned byte=0;
				for (unsigned x=0; x<nWidth; ++x)
				{
					if (!(x&7))
					{
						if (!nRemaining) return NULL;
						
					REPEAT_SKIP:
						byte = *pDecodeSrc;
						
						if (rawcnt)
						{
							--rawcnt;
							++pDecodeSrc;
							--nRemaining;
						}
						else if (repcnt)
						{
							--repcnt;
							if (!repcnt)
							{
								++pDecodeSrc;
								--nRemaining;
							}
						}
						else // byte is control byte
						{
							++pDecodeSrc;
							--nRemaining;
							
							if (!nRemaining) return NULL;
							
							if (byte<0x80)
							{
								rawcnt = byte;
								byte = *pDecodeSrc++;
								--nRemaining;
							}
							else if (byte>0x80)
							{
								repcnt = 0x100 - byte;
								byte = *pDecodeSrc;
							}
							else goto REPEAT_SKIP;
						}
					}
					
					pDecodeDst[x] |= (byte>>7 & 1)<<b;
					
					byte <<= 1;
				}
				
			}
		}
		else
		{
			for (unsigned b=0; b<nBitPlanes; ++b)
			{
				unsigned byte=0;
				for (unsigned x=0; x<nWidth; ++x)
				{
					if (!(x&7))
					{
						if (!nRemaining) return NULL;
						
						byte = *pDecodeSrc++;
						--nRemaining;
					}
					
					pDecodeDst[x] |= (byte>>7 & 1)<<b;
					
					byte <<= 1;
				}
				
			}
		}
			
		
		return pDecodeDst;
	}
	
	IlbmBodyChunk::~IlbmBodyChunk()
	{
		if (pData) delete[] pData;
		if (pDecodeDst) delete[] pDecodeDst;
		#ifndef IFF_READ_ONLY
			if (pEncodeDst) delete pEncodeDst;
			if (pEncodeSrc) delete[] pEncodeSrc;
		#endif
	}
	
	void IlbmGrabChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(xHotSpot);
		pArchv->Transfer(yHotSpot);
	}
}
