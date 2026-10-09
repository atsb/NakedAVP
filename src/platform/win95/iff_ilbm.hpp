#ifndef _INCLUDED_IFF_ILBM_HPP_
#define _INCLUDED_IFF_ILBM_HPP_

#include "iff.hpp"

namespace IFF
{
	class IlbmBmhdChunk : public Chunk
	{
		public:
			UINT16 width;
			UINT16 height;
			UINT16 xTopLeft;
			UINT16 yTopLeft;
			UBYTE nBitPlanes;
			enum
			{
				MASK_NONE = 0,
				MASK_HASMASK = 1,
				MASK_TRANSPARENTCOL = 2,
				MASK_LASSO = 3
			};
			UBYTE eMasking;
			enum
			{
				COMPRESS_NONE = 0,
				COMPRESS_RUNLENGTH = 1,
				COMPRESS_S3TC =2   //will have s3tc chunk instead of body chunk
			};
			UBYTE eCompression;
			UBYTE flags;
			UINT16 iTranspCol;
			UBYTE xAspectRatio;
			UBYTE yAspectRatio;
			UINT16 xMax;
			UINT16 yMax;
			
			IlbmBmhdChunk() { m_idCk = "BMHD"; }
			
		protected:
			virtual void Serialize(Archive * pArchv);
	};
	
	class IlbmCmapChunk : public Chunk
	{
		public:
			unsigned nEntries;
			RGBTriple * pTable;
			
			void CreateNew(unsigned nSize);
			
			IlbmCmapChunk() : pTable(NULL) { m_idCk = "CMAP"; }
			virtual ~IlbmCmapChunk();
			
		protected:
			virtual void Serialize(Archive * pArchv);
	};
	
	inline void IlbmCmapChunk::CreateNew(unsigned nSize)
	{
		if (pTable) delete[] pTable;
		pTable = new RGBTriple [nSize];
		nEntries = nSize;
	}
	
	class IlbmBodyChunk : public Chunk
	{
		public:
			IlbmBodyChunk() : pData(NULL), pDecodeDst(NULL)
				#ifndef IFF_READ_ONLY
					, pEncodeDst(NULL), pEncodeSrc(NULL)
				#endif
				{ m_idCk = "BODY"; }
			virtual ~IlbmBodyChunk();
			
			#ifndef IFF_READ_ONLY
				bool BeginEncode();
				bool EncodeFirstRow(unsigned const * pRow);
				bool EncodeNextRow(unsigned const * pRow);
				bool EndEncode();
				
				float GetCompressionRatio() const;
			#endif
			
			bool BeginDecode() const;
			unsigned const * DecodeFirstRow() const;
			unsigned const * DecodeNextRow() const;
			bool EndDecode() const;
			
		protected:
			virtual void Serialize(Archive * pArchv);

			virtual bool GetHeaderInfo() const; // fills in these three data members
			mutable unsigned nWidth;
			mutable unsigned eCompression;
			mutable unsigned nBitPlanes;

		private:
			unsigned nSize;
			UBYTE * pData;
			
			mutable UBYTE const * pDecodeSrc;
			mutable unsigned nRemaining;
			mutable unsigned * pDecodeDst;
			
			#ifndef IFF_READ_ONLY
				DataBlock * pEncodeDst;
				UBYTE * pEncodeSrc;
				
				unsigned nSizeNonCprss;
				unsigned nSizeCprss;
			#endif
	};
	
	#ifndef IFF_READ_ONLY
		inline bool IlbmBodyChunk::EncodeFirstRow(unsigned const * pRow)
		{
			if (BeginEncode())
				return EncodeNextRow(pRow);
			else
				return false;
		}
		
		inline float IlbmBodyChunk::GetCompressionRatio() const
		{
			if (!nSizeNonCprss) return 0.0F;
			
			return (static_cast<float>(nSizeNonCprss)-static_cast<float>(nSizeCprss))/static_cast<float>(nSizeNonCprss);
		}
	#endif
	
	inline unsigned const * IlbmBodyChunk::DecodeFirstRow() const
	{
		if (BeginDecode())
			return DecodeNextRow();
		else
			return NULL;
	}
	
	inline bool IlbmBodyChunk::EndDecode() const
	{
		if (pDecodeDst)
		{
			delete[] pDecodeDst;
			pDecodeDst = NULL;
			return true;
		}
		else return false;
	}
	
	class IlbmGrabChunk : public Chunk
	{
		public:
			UINT16 xHotSpot;
			UINT16 yHotSpot;
			
			IlbmGrabChunk() { m_idCk = "GRAB"; }
			
		protected:
			virtual void Serialize(Archive * pArchv);
	};
}

#endif // ! _INCLUDED_IFF_ILBM_HPP_
