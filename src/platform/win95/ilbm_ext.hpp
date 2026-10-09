#ifndef _INCLUDED_ILBM_EXT_HPP_
#define _INCLUDED_ILBM_EXT_HPP_

#include "iff.hpp"
#include "iff_ilbm.hpp"

namespace IFF
{
	class IlbmTranChunk : public Chunk
	{
		public:
			enum
			{
				TRANS_NONE = 0,
				TRANS_TOPLEFT = 1,
				TRANS_BOTTOMLEFT = 2,
				TRANS_TOPRIGHT = 3,
				TRANS_BOTTOMRIGHT = 4,
				TRANS_XY = 5,
				TRANS_RGB = 6
			};
			UBYTE eTransType;
			UINT16 xPos;
			UINT16 yPos;
			RGBTriple rgb;
			
			IlbmTranChunk() { m_idCk = "TRAN"; }
			
		protected:
			virtual void Serialize(Archive * pArchv);
	};

	class IlbmAlphChunk : public IlbmBodyChunk // uses same encoding methodology
	{
		public:
			UINT16 width;
			UINT16 height;
			UBYTE nBitPlanes;
			UBYTE eCompression;

			IlbmAlphChunk()
				{ m_idCk = "ALPH"; }

		protected:
			virtual void Serialize(Archive * pArchv);

			virtual bool GetHeaderInfo() const;
	};

	class IlbmS3tcChunk : public Chunk
	{
		public:
			IlbmS3tcChunk();
			virtual ~IlbmS3tcChunk();
			
			
			
			UINT32 flags; // none at the moment
			UINT32 fourCC; //the fourcc code 'DXT1' - 'DXT5'

			UINT16 redWeight; //weighting values used in compression
			UINT16 blueWeight;
			UINT16 greenWeight;

			UINT16 width;
			UINT16 height;

			UINT32 dataSize;
			UBYTE* pData;  //the compressed texture itself

		protected:
			virtual void Serialize(Archive * pArchv);
	};


	class MipmContChunk : public Chunk
	{
		public:
			enum
			{
				FILTER_DEFAULT = 0,
				FILTER_BOX = 1,
				FILTER_TRIANGLE = 2,
				FILTER_BELL = 3,
				FILTER_BSPLINE = 4,
				FILTER_LANCZOS3 = 5,
				FILTER_MITCHELL = 6
			};
			UBYTE nMipMaps;
			UBYTE eFilter;

			MipmContChunk()
				{ m_idCk = "CONT"; }

		protected:
			virtual void Serialize(Archive * pArchv);
	};

	class MipmFlagChunk : public Chunk
	{
		public:
			enum
			{
				FLAG_MANUAL_MIPS = 0x00000001,//some of the mip maps have been set by hand
			};
			UINT32 flags;

			MipmFlagChunk()
				{ m_idCk = "FLAG"; flags = 0;}

		protected:
			virtual void Serialize(Archive * pArchv);
	};

}

#endif
