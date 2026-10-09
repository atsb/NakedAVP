#ifndef _INCLUDED_IFF_HPP_
#define _INCLUDED_IFF_HPP_

#include "fixer.h"

//#if defined(_WIN32) || defined(WIN32) || defined(WINDOWS) || defined(_WINDOWS)
//	#define _IFF_WIN_TARGET
//	#include <windows.h>
//#else // ! WIN32 && ! _WIN32 && ! WINDOWS && ! _WINDOWS
	#include <stdio.h>
//#endif // ! WIN32 && ! _WIN32 && ! WINDOWS && ! _WINDOWS

#include "media.hpp"

namespace IFF
{
	/*************************/
	/* Class Hierarchy:      */
	/*                       */
	/* + Unknown             */
	/*   + SerializableObj   */
	/*     + GenericFile     */
	/*       + File          */
	/*     + Chunk           */
	/*       + Composite     */
	/*         + Form        */
	/*         + Cat         */
	/*         + List        */
	/*         + Prop        */
	/*       + MiscChunk     */
	/*       + <user chunks> */
	/*   + Arvhive           */
	/*     + ArchvIn         */
	/*     + ArchvOut(*)     */
	/*   + DataBlock         */
	/*   + DataNode          */
	/*   + SerialData        */
	/*     + ArchvOut(*)     */
	/*   + ChildNode         */
	/*   + DeviceHandle      */
	/*************************/
	
	#ifdef _IFF_WIN_TARGET
		inline void DisplayMessage(TCHAR const * pszTitle,TCHAR const * pszText)
		{
			::MessageBox(NULL,pszText,pszTitle,MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);
		}
	#else
		inline void DisplayMessage(char const * pszTitle,char const * pszText)
		{
			::printf("DisplayMessage\n%s\n%s\n",pszTitle,pszText);
		/*
			while (::kbhit())
				::getch();
			while (!::kbhit() || '\r' != ::getch())
				;
		*/
		}
	#endif
	
	// forward refs
	//class Unknown;
	//class SerializableObj;
	//class GenericFile;
	//class File;
	//class Chunk;
	  class Composite;
	//class Form;
	//class Cat;
	//class List;
	//class Prop;
	//class MiscChunk;
	//class Archive;
	//class ArchvIn;
	//class ArchvOut;
	//class DataBlock;
	//class DataNode;
	//class SerialData;
	  class ChildNode;
	//class DeviceHandle;
	
	/*****************************/
	/* Original File: iffObj.hpp */
	/*****************************/
	
	class Unknown
	{
		public:
			unsigned AddRef() { return ++m_nRefCnt; }
			unsigned Release() { if (0==(--m_nRefCnt)) { delete this; return 0;} else return m_nRefCnt; }
		protected:
			virtual ~Unknown() {
				#ifndef NDEBUG
					DbForget(this);
				#endif
			}
			Unknown() : m_nRefCnt(1) {
				#ifndef NDEBUG
					DbRemember(this);
				#endif
			}
			Unknown(Unknown const &) : m_nRefCnt(1) {
				#ifndef NDEBUG
					DbRemember(this);
				#endif
			}
		private:
			unsigned m_nRefCnt;
			
		#ifndef NDEBUG
			friend void DbRemember(Unknown * pObj);
			friend void DbForget(Unknown * pObj);
			friend class AllocList;
		#endif
	};
	
	/*******************************/
	/* Original File: iffTypes.hpp */
	/*******************************/
	
	// deal with any silly bastard who's put #define BYTE ... in a header, etc.
	#ifdef BYTE
		#undef BYTE
		#pragma message("BYTE was defined - undefining")
	#endif

#if !defined(_MSC_VER)
	typedef int8_t BYTE;
	typedef uint8_t UBYTE;

	typedef int16_t INT16;
	typedef uint16_t UINT16;

	typedef int32_t INT32;
	typedef uint32_t UINT32;

	typedef int64_t INT64;
	typedef uint64_t UINT64;
#else
	#pragma message("might want to move these typedefs elsewhere")
	typedef signed char BYTE;
	typedef unsigned char UBYTE;

	typedef signed short INT16;
	typedef unsigned short UINT16;

	typedef signed int INT32;
	typedef unsigned int UINT32;

	typedef signed __int64 INT64;
	typedef unsigned __int64 UINT64;
#endif

	struct RGBTriple
	{
		UBYTE r;
		UBYTE g;
		UBYTE b;
	};

	union ID
	{
		UINT32 m_nID;
		char m_sID[4];
		
		inline ID(){}
		inline ID(char const * pszID) { m_nID = *reinterpret_cast<UINT32 const *>(pszID); }
		inline ID(UINT32 nID) : m_nID(nID) {}
		inline operator UINT32 () const { return m_nID; }
		inline operator char const * () const { return &m_sID[0]; }
		inline bool operator == (ID const & rId) const { return !m_nID || !rId.m_nID || m_nID == rId.m_nID; }
		inline bool operator != (ID const & rId) const { return ! operator == (rId); }
		inline bool operator ! () const { return !m_nID; }
	};
	
	ID const ID_ANY(0U);
	
	#ifndef IFF_READ_ONLY
	
		/*******************************/
		/* Original File: iffSData.hpp */
		/*******************************/
	
		class DataBlock : public Unknown
		{
			public:
				virtual ~DataBlock();
				DataBlock() : m_pBlock(new UBYTE [128]), m_nMaxSize(128), m_nCurSize(0) {}
				
				unsigned GetDataSize() const;
				UBYTE const * GetDataPtr() const;
				
				bool WriteToFile(::MediaMedium * pMedium) const;
				
				void Append(UBYTE byte);
				void Append(void const * pData, unsigned nSize);
				
			private:
				void Expand(unsigned nMinSize);
			
				UBYTE * m_pBlock;
				unsigned m_nMaxSize;
				unsigned m_nCurSize;
		};
		
		inline unsigned DataBlock::GetDataSize() const
		{
			return m_nCurSize;
		}
		
		inline UBYTE const * DataBlock::GetDataPtr() const
		{
			return m_pBlock;
		}
		
		inline void DataBlock::Append(UBYTE byte)
		{
			if (m_nCurSize >= m_nMaxSize)
				Expand(m_nCurSize+1);
			
			m_pBlock[m_nCurSize++] = byte;
		}
		
		inline void DataBlock::Append(void const * pData, unsigned nSize)
		{
			if (m_nCurSize+nSize > m_nMaxSize)
				Expand(m_nCurSize+nSize+32);
			
			memcpy(&m_pBlock[m_nCurSize],pData,nSize);
			
			m_nCurSize += nSize;
		}
		
		class DataNode : public Unknown
		{
			public:
				DataNode(DataNode * pNext, DataBlock * pData, DataNode * pPrev)
					: m_pNext(pNext)
					, m_pData(pData)
					, m_pPrev(pPrev)
				{
					if (pNext) pNext->AddRef();
					if (pData) pData->AddRef();
					if (pPrev) pPrev->AddRef();
				}
				virtual ~DataNode();
				
				unsigned GetDataSize() const;
				
				bool WriteToFile(::MediaMedium * pMedium) const;
				
			private:
				DataNode * m_pNext;
				DataBlock * m_pData;
				DataNode * m_pPrev;
		};
		
		class SerialData : public Unknown
		{
			public:
				virtual ~SerialData();
				SerialData() : m_pData(new DataBlock), m_pPrev(NULL) {};
				
				void Clear();
				
				unsigned GetDataSize() const;

				bool WriteToFile(::MediaMedium * pMedium) const;
				
				void Append(UBYTE byte);
				void Append(void const * pData, unsigned nSize);
				
				void Append(SerialData * pData);
				
			private:
				DataBlock * m_pData;
				DataNode * m_pPrev;
		};
		
		inline void SerialData::Append(UBYTE byte)
		{
			m_pData->Append(byte);
		}
		
		inline void SerialData::Append(void const * pData, unsigned nSize)
		{
			m_pData->Append(pData,nSize);
		}
		
		inline void SerialData::Append(SerialData * pIns)
		{
			DataNode * pNewNode = new DataNode(pIns->m_pPrev,m_pData,m_pPrev);
			
			m_pData->Release();
			m_pData = pIns->m_pData;
			pIns->m_pData->AddRef();
			
			if (m_pPrev)
				m_pPrev->Release();
			m_pPrev = pNewNode;
		}
	
		/*******************************/
		/* Original File: iffArchv.hpp */
		/*******************************/
		
		class Archive : public Unknown
		{
			public:
				// constructor - construct with true iff the archive is for loading data
				Archive(bool bIsLoading) : m_bIsLoading(bIsLoading), m_bError(false) {}
				
				// to determine easily whenever necessary if the archive is loading data
				bool const m_bIsLoading;
				bool m_bError;
				
				// returns either the size in bytes of the data so far written to a storing archive
				// or the number of bytes remaining to be read from a loading archive, could be negative
				// if too many bytes were read
				virtual signed GetSize() const = 0;
				
				// if the archive is loading, nSize bytes are sectioned off to be read from a sub-archive
				// or if the archive is storing, nSize is ignored and data written to the sub-archive
				// will be stored when the sub-archive is closed
				virtual Archive * OpenSubArchive(unsigned nSize = 0) = 0;
				
				// commits data written temporarily to a sub-archive (if storing)
				// and advances the archive to the next byte after the sub-archive's data
				virtual void CloseSubArchive(Archive * pSub) = 0;
				
				virtual void Open(::MediaMedium * pMedium) = 0;
				virtual void Close() = 0;
				
				virtual void Transfer(RGBTriple &) = 0;
				virtual void Transfer(BYTE &) = 0;
				virtual void Transfer(UBYTE &) = 0;
				virtual void Transfer(INT16 &) = 0;
				virtual void Transfer(UINT16 &) = 0;
				virtual void Transfer(INT32 &) = 0;
				virtual void Transfer(UINT32 &) = 0;
				virtual void Transfer(INT64 &) = 0;
				virtual void Transfer(UINT64 &) = 0;
				virtual void Transfer(ID &) = 0;
				
				virtual void TransferBlock(void * pData,unsigned nSize) = 0;
		};
		
		/*******************************/
		/* Original File: iffArchO.hpp */
		/*******************************/
		
		class ArchvOut : public Archive, public SerialData
		{
			public:
				ArchvOut() : Archive(false), m_pMedium(NULL) {}
				~ArchvOut();
				
				signed GetSize() const;
				
				Archive * OpenSubArchive(unsigned nSize = 0);
				
				void CloseSubArchive(Archive * pSub);
				
				void Open(::MediaMedium * pMedium);
				void Close();
				
				void Transfer(RGBTriple &);
				void Transfer(BYTE &);
				void Transfer(UBYTE &);
				void Transfer(INT16 &);
				void Transfer(UINT16 &);
				void Transfer(INT32 &);
				void Transfer(UINT32 &);
				void Transfer(INT64 &);
				void Transfer(UINT64 &);
				void Transfer(ID &);
				
				void TransferBlock(void * pData,unsigned nSize);
				
			private:
				::MediaMedium * m_pMedium;
		};
		
		inline signed ArchvOut::GetSize() const
		{
			return GetDataSize();
		}

		inline void ArchvOut::Transfer(RGBTriple & n)
		{
			Append(n.r);
			Append(n.g);
			Append(n.b);
		}
		
		inline void ArchvOut::Transfer(BYTE & n)
		{
			Append(n);
		}
		
		inline void ArchvOut::Transfer(UBYTE & n)
		{
			Append(n);
		}
		
		inline void ArchvOut::Transfer(INT16 & n)
		{
			Append(static_cast<BYTE>(n >> 8));
			Append(static_cast<BYTE>(n));
		}
		
		inline void ArchvOut::Transfer(UINT16 & n)
		{
			Append(static_cast<UBYTE>(n >> 8));
			Append(static_cast<UBYTE>(n));
		}
		
		inline void ArchvOut::Transfer(INT32 & n)
		{
			Append(static_cast<BYTE>(n >> 24));
			Append(static_cast<BYTE>(n >> 16));
			Append(static_cast<BYTE>(n >> 8));
			Append(static_cast<BYTE>(n));
		}

		inline void ArchvOut::Transfer(UINT32 & n)
		{
			Append(static_cast<UBYTE>(n >> 24));
			Append(static_cast<UBYTE>(n >> 16));
			Append(static_cast<UBYTE>(n >> 8));
			Append(static_cast<UBYTE>(n));
		}

		inline void ArchvOut::Transfer(INT64 & n)
		{
			Append(static_cast<BYTE>(n >> 56));
			Append(static_cast<BYTE>(n >> 48));
			Append(static_cast<BYTE>(n >> 40));
			Append(static_cast<BYTE>(n >> 32));
			Append(static_cast<BYTE>(n >> 24));
			Append(static_cast<BYTE>(n >> 16));
			Append(static_cast<BYTE>(n >> 8));
			Append(static_cast<BYTE>(n));
		}

		inline void ArchvOut::Transfer(UINT64 & n)
		{
			Append(static_cast<UBYTE>(n >> 56));
			Append(static_cast<UBYTE>(n >> 48));
			Append(static_cast<UBYTE>(n >> 40));
			Append(static_cast<UBYTE>(n >> 32));
			Append(static_cast<UBYTE>(n >> 24));
			Append(static_cast<UBYTE>(n >> 16));
			Append(static_cast<UBYTE>(n >> 8));
			Append(static_cast<UBYTE>(n));
		}
		
		inline void ArchvOut::Transfer(ID & n)
		{
			Append(&n,4);
		}

		inline void ArchvOut::TransferBlock(void * pData,unsigned nSize)
		{
			Append(pData,nSize);
		}
		
	#endif // ! IFF_READ_ONLY

	/*******************************/
	/* Original File: iffArchI.hpp */
	/*******************************/
	
	#ifdef IFF_READ_ONLY
		#define _IFF_ARCHI_BASE Unknown
		#define _IFF_ARCHI_GENR ArchvIn
		#define _IFF_ARCHI_FLAG m_bIsLoading
		#define _IFF_ARCHI_INIT ,m_bError(false)
	#else // ! IFF_READ_ONLY
		#define _IFF_ARCHI_BASE Archive
		#define _IFF_ARCHI_GENR Archive
		#define _IFF_ARCHI_FLAG Archive
		#define _IFF_ARCHI_INIT
	#endif // ! IFF_READ_ONLY
	
	class ArchvIn : public _IFF_ARCHI_BASE
	{
		public:
			ArchvIn() : _IFF_ARCHI_FLAG(true), m_pMedium(NULL) _IFF_ARCHI_INIT {}
			~ArchvIn();
			
			#ifdef IFF_READ_ONLY
				bool const m_bIsLoading;
				bool m_bError;
			#endif // IFF_READ_ONLY
			
			signed GetSize() const;
			
			_IFF_ARCHI_GENR * OpenSubArchive(unsigned nSize = 0);
			
			void CloseSubArchive(_IFF_ARCHI_GENR * pSub);
			
			void Open(::MediaMedium * pMedium);
			void Close();
			
			void Transfer(RGBTriple &);
			void Transfer(BYTE &);
			void Transfer(UBYTE &);
			void Transfer(INT16 &);
			void Transfer(UINT16 &);
			void Transfer(INT32 &);
			void Transfer(UINT32 &);
			void Transfer(INT64 &);
			void Transfer(UINT64 &);
			void Transfer(ID &);
			
			void TransferBlock(void * pData,unsigned nSize);
			
		private:
			ArchvIn(ArchvIn * pParent, unsigned nSize);

			::MediaMedium * m_pMedium;
			signed m_nBytesRemaining;
			unsigned m_nEndPos;
	};
	
	#ifdef IFF_READ_ONLY
		typedef ArchvIn Archive;
	#endif // IFF_READ_ONLY
	
	inline signed ArchvIn::GetSize() const
	{
		return m_nBytesRemaining;
	}

	inline void ArchvIn::Transfer(RGBTriple & n)
	{
		m_nBytesRemaining -= 3;
		if (m_nBytesRemaining >= 0)
		{
			::MediaRead(m_pMedium, &n.r);
			::MediaRead(m_pMedium, &n.g);
			::MediaRead(m_pMedium, &n.b);
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}
	
	inline void ArchvIn::Transfer(UBYTE & n)
	{
		m_nBytesRemaining -= 1;
		if (m_nBytesRemaining >= 0)
		{
			::MediaRead(m_pMedium, &n);
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(BYTE & n)
	{
		m_nBytesRemaining -= 1;
		if (m_nBytesRemaining >= 0)
		{
			::MediaRead(m_pMedium, &n);
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(UINT16 & n)
	{
		m_nBytesRemaining -= 2;
		if (m_nBytesRemaining >= 0)
		{
			UBYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(INT16 & n)
	{
		m_nBytesRemaining -= 2;
		if (m_nBytesRemaining >= 0)
		{
			BYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(UINT32 & n)
	{
		m_nBytesRemaining -= 4;
		if (m_nBytesRemaining >= 0)
		{
			UBYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(INT32 & n)
	{
		m_nBytesRemaining -= 4;
		if (m_nBytesRemaining >= 0)
		{
			BYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(UINT64 & n)
	{
		m_nBytesRemaining -= 8;
		if (m_nBytesRemaining >= 0)
		{
			UBYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(INT64 & n)
	{
		m_nBytesRemaining -= 8;
		if (m_nBytesRemaining >= 0)
		{
			BYTE b;
			::MediaRead(m_pMedium, &b);
			n = b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			::MediaRead(m_pMedium, &b);
			n <<= 8;
			n |= b;
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::Transfer(ID & n)
	{
		m_nBytesRemaining -= 4;
		if (m_nBytesRemaining >= 0)
		{
			// cast pointer to pointer to 4 byte data type to force 4 byte 'fast' read
			::MediaRead(m_pMedium, reinterpret_cast<UINT32 *>(&n.m_sID[0]));
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}

	inline void ArchvIn::TransferBlock(void * pData,unsigned nSize)
	{
		m_nBytesRemaining -= nSize;
		if (m_nBytesRemaining >= 0)
		{
			m_pMedium->ReadBlock(pData,nSize);
			if (m_pMedium->m_fError) m_bError = true;
		}
		else m_bError = true;
	}
	
	/*******************************/
	/* Original File: iffSrlOb.hpp */
	/*******************************/
	
	class SerializableObj : public Unknown
	{
		public:
			virtual void Serialize(Archive * pArchv) = 0;
	};
	
	/*******************************/
	/* Original File: iffChunk.hpp */
	/*******************************/
	
	class Chunk : public SerializableObj
	{
		public:
			ID m_idCk;
			
			Chunk * GetProperty(ID idProp) const;
			
			virtual bool IsUnknown() const { return false; }
			
			void Write(Archive * pArchv);
			static Chunk * Load(ID idParent, Archive * pArchv, ID idChunk = ID_ANY, bool bKnown = true);
			
			static Chunk * DynCreate(ID idParent, ID idChunk);
			static void Register(ID idParent, ID idChunk, Chunk * (* pfnCreate) () );
			
			Composite const * GetParent() const { return m_pParent; }
			
		protected:
			Chunk() : m_pParent(NULL) {}
			
		private:
			Composite const * m_pParent; // mot reference counted
			ChildNode const * m_pNode; // not reference counted either
			
			friend class Composite;
	};
	
	#define IFF_IMPLEMENT_DYNCREATE(idParent,idChunk,tokenClassName) _IFF_IMPLEMENT_DYNCREATE_LINE_EX(idParent,idChunk,tokenClassName,__LINE__)
	#define _IFF_IMPLEMENT_DYNCREATE_LINE_EX(idParent,idChunk,tokenClassName,nLine) _IFF_IMPLEMENT_DYNCREATE_LINE(idParent,idChunk,tokenClassName,nLine)
	
	#define _IFF_IMPLEMENT_DYNCREATE_LINE(idParent,idChunk,tokenClassName,nLine) \
	static IFF::Chunk * CreateClassObject ## tokenClassName ##_## nLine () { \
		IFF::Chunk * pChunk = new IFF::tokenClassName; \
		pChunk->m_idCk = idChunk; \
		return pChunk; \
	} \
	 class RegisterChunkClass ## tokenClassName ##_## nLine { \
		public: RegisterChunkClass ## tokenClassName ##_## nLine () { \
			IFF::Chunk::Register(idParent , idChunk , CreateClassObject ## tokenClassName ##_## nLine); \
		} \
	} rcc ## tokenClassName ##_## nLine;
	
	/*******************************/
	/* Original File: iffBlock.hpp */
	/*******************************/
	
	class ChildNode : public Unknown
	{
		public:
			Chunk * GetChunk() const { return m_pChunk; }
		private:
			ChildNode * m_pNext;
			ChildNode * m_pPrev;
			Chunk * m_pChunk;
			
			friend class Composite;
	};
	
	class Composite : public Chunk
	{
		public:
			ID m_idData;
			
			Composite() : m_pFirst(NULL), m_pLast(NULL) {}
			virtual ~Composite();
			
			ChildNode * GetFirstChild() const { return m_pFirst; }
			ChildNode * GetFirstChild(ID idMatch) const;
			ChildNode * GetLastChild() const { return m_pLast; }
			ChildNode * GetLastChild(ID idMatch) const;
			static ChildNode * GetNextChild(ChildNode const * pNode) { return pNode->m_pNext; }
			static ChildNode * GetNextChild(ChildNode const * pNode, ID idMatch);
			static ChildNode * GetPrevChild(ChildNode const * pNode) { return pNode->m_pPrev; }
			static ChildNode * GetPrevChild(ChildNode const * pNode, ID idMatch);
			
			Chunk * GetProperty(ChildNode const * pNode, ID idProp) const;
			
			void DeleteChild(ChildNode * pNode);
			void DeleteAllChildren();
			
			ChildNode * InsertChildFirst(Chunk * pChunk);
			ChildNode * InsertChildLast(Chunk * pChunk);
			ChildNode * InsertChildAfter(ChildNode * pNode, Chunk * pChunk);
			ChildNode * InsertChildBefore(ChildNode * pNode, Chunk * pChunk);
			
			// pfnCallback should return true to continue the enumeration, false to finish
			virtual bool EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const;
			
		protected:
			virtual void Serialize(Archive * pArchv);
			
			virtual Chunk * LoadChunk(Archive * pArchv) const;
			virtual bool IsValidChildID(ID id) const;
			
		private:
			ChildNode * m_pFirst;
			ChildNode * m_pLast;
	};
	
	class Form : public Composite
	{
		public:
			Form() { m_idCk = "FORM"; }
		protected:
			virtual bool IsValidChildID(ID id) const;
	};
	
	class Cat : public Composite
	{
		public:
			Cat() { m_idCk = "CAT "; }
			virtual bool EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const;
		protected:
			virtual bool IsValidChildID(ID id) const;
	};
	
	class List : public Composite
	{
		public:
			List() { m_idCk = "LIST"; }
			virtual bool EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const;
		protected:
			virtual bool IsValidChildID(ID id) const;
	};
	
	class Prop : public Composite
	{
		public:
			Prop() { m_idCk = "PROP"; }
		protected:
			virtual bool IsValidChildID(ID id) const;
	};
	
	/*******************************/
	/* Original File: iffMscCk.hpp */
	/*******************************/
	
	class MiscChunk : public Chunk
	{
		public:
			MiscChunk(ID id)
				#ifndef IFF_READ_ONLY
					: m_pData(NULL)
				#endif // ! IFF_READ_ONLY
				{
					m_idCk = id;
				}
				
			virtual bool IsUnknown() const { return true; }
				
		protected:
			virtual void Serialize(Archive * pArchv);
			#ifndef IFF_READ_ONLY
				virtual ~MiscChunk();
			#endif // ! IFF_READ_ONLY
			
		private:
			#ifndef IFF_READ_ONLY
				BYTE * m_pData;
				unsigned m_nSize;
			#endif // ! IFF_READ_ONLY
	};
	
	/******************************/
	/* Original File: iffFile.hpp */
	/******************************/

	class GenericFile : public SerializableObj
	{
		public:
			#ifndef IFF_READ_ONLY
				GenericFile() : m_pszFileName(NULL) {}
				virtual ~GenericFile() { if (m_pszFileName) delete[] m_pszFileName; }
			#endif
		
			bool Load(TCHAR const * pszFileName);
			bool Load(::MediaMedium * pMedium);
			
			#ifndef IFF_READ_ONLY
				bool Write(TCHAR const * pszFileName = NULL);
				bool Write(::MediaMedium * pMedium);
			#endif // ! IFF_READ_ONLY
			
		private:
			#ifndef IFF_READ_ONLY
				TCHAR * m_pszFileName;
			#endif // ! IFF_READ_ONLY
	};
	
	class File : public GenericFile
	{
		public:
			virtual ~File();
			File() : m_pContents(NULL) {}
			
			Composite * GetContents() const;
			void SetContents(Composite * pContents);
			
		protected:
			virtual void Serialize(Archive * pArchv);
			
		private:
			ID m_idType;
			Composite * m_pContents;
	};
	
	inline Composite * File::GetContents() const
	{
		return m_pContents;
	}
	
	inline void File::SetContents(Composite * pContents)
	{
		if (m_pContents) m_pContents->Release();
		if (pContents)
		{
			pContents->AddRef();
			m_idType = pContents->m_idCk;
		}
		m_pContents = pContents;
	}
	
	// Have a static object of this in each file:
	// It will detect if some files are compiled with
	// IFF_READ_ONLY defined differently to each other
	static class ConsistencyCheck
	{
		public: inline ConsistencyCheck()
		{
			#ifdef IFF_READ_ONLY
				static bool bReadOnly = true;
				if (!bReadOnly)
			#else
				static bool bReadOnly = false;
				if (bReadOnly)
			#endif
			{
				DisplayMessage
				(
					#ifdef NDEBUG
						TEXT("Error"),
						TEXT("IFF_READ_ONLY definition not consistent")
					#else
						TEXT("IFF Compile Option Error"),
						TEXT("Some files which #include \"iff.hpp\"\n")
						TEXT("have the macro IFF_READ_ONLY defined\n")
						TEXT("and some don't.\n\n")
						TEXT("Please ensure this is consistent and recompile.")
					#endif
				);
				exit(-45);
			}
		}
	}
		consistencyCheck;
	
} // namespace IFF

#endif // ! _INCLUDED_IFF_HPP_
