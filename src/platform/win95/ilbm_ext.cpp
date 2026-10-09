#include "ilbm_ext.hpp"

IFF_IMPLEMENT_DYNCREATE("ILBM","TRAN",IlbmTranChunk)
IFF_IMPLEMENT_DYNCREATE("ILBM","ALPH",IlbmAlphChunk)
IFF_IMPLEMENT_DYNCREATE("MIPM","CONT",MipmContChunk)
IFF_IMPLEMENT_DYNCREATE("ILBM","S3TC",IlbmS3tcChunk)
IFF_IMPLEMENT_DYNCREATE("MIPM","FLAG",MipmFlagChunk)

namespace IFF
{
	void IlbmTranChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(eTransType);
		pArchv->Transfer(xPos);
		pArchv->Transfer(yPos);
		pArchv->Transfer(rgb);
	}

	void IlbmAlphChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(width);
		pArchv->Transfer(height);
		pArchv->Transfer(nBitPlanes);
		pArchv->Transfer(eCompression);

		IlbmBodyChunk::Serialize(pArchv);
	}

	bool IlbmAlphChunk::GetHeaderInfo() const
	{
		IlbmBodyChunk::nWidth = width;
		IlbmBodyChunk::eCompression = eCompression;
		IlbmBodyChunk::nBitPlanes = nBitPlanes;
		return true;
	}

	void MipmContChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(nMipMaps);
		pArchv->Transfer(eFilter);
	}

	void MipmFlagChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(flags);
	}

	IlbmS3tcChunk::IlbmS3tcChunk()
	{
		m_idCk = "S3TC";

		pData = NULL;
		dataSize = 0;
	}

	IlbmS3tcChunk::~IlbmS3tcChunk()
	{
		if(pData) delete [] pData;
		pData = NULL;
	}	
	
	void IlbmS3tcChunk::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(flags);
		pArchv->Transfer(fourCC);
		pArchv->Transfer(redWeight);
		pArchv->Transfer(blueWeight);
		pArchv->Transfer(greenWeight);
		pArchv->Transfer(width);
		pArchv->Transfer(height);

		pArchv->Transfer(dataSize);

		if (pArchv->m_bIsLoading)
		{
			if(pData) delete [] pData;
			pData = new UBYTE[dataSize];
		}

		UBYTE *pDataPos = pData;
		for(unsigned i=0;i<dataSize;i++)
		{
			pArchv->Transfer(*pDataPos++);
		}
	}
	
}
