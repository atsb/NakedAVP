#include "iff.hpp"

#include <stdio.h>

#if defined(_CPPRTTI) && !defined(NDEBUG)
	#include <typeinfo>
#endif

#ifndef NDEBUG
	#define HT_FAIL(str) (::IFF::DisplayMessage(TEXT("IFF Internal Error"),TEXT(str)),exit(-45))
#endif

#include "hash_tem.hpp"

namespace IFF
{
	/*****************************/
	/* Original File: iffObj.cpp */
	/*****************************/

	#ifndef NDEBUG
	
		static bool g_bAllocListActive = false;
		
		class AllocList : public ::HashTable<Unknown *>
		{
			public:
				AllocList()
				{
					g_bAllocListActive = true;
				}
				~AllocList()
				{
					#ifdef _CPPRTTI // this works in MSVC 5.0 - ie. the macro is defined if RTTI is turned on
						// but there appears to be no preprocessor way of determining if RTTI is turned on under Watcom
						// No, I think it works in Watcom too, actually...
						#pragma message("Run-Time Type Identification (RTTI) is enabled")
						for (Iterator itLeak(*this) ; !itLeak.Done() ; itLeak.Next())
						{
							TCHAR buf[256];
							::wsprintf(buf,TEXT("Object not deallocated:\nType: %s\nRefCnt: %u"),typeid(*itLeak.Get()).name(),itLeak.Get()->m_nRefCnt);
							DisplayMessage(TEXT("Memory Leak!"),buf);
						}
					#else // ! _CPPRTTI
						unsigned nRefs(0);
						for (Iterator itLeak(*this) ; !itLeak.Done() ; itLeak.Next())
						{
							nRefs += itLeak.Get()->m_nRefCnt;
						}
						if (Size())
						{
							char buf[256];
							::sprintf(buf,"Objects not deallocated:\nNumber of Objects: %u\nNumber of References: %u",Size(),nRefs);
							DisplayMessage("Memory Leaks!",buf);
						}
					#endif // ! _CPPRTTI
					g_bAllocListActive = false;
				}
		};
		
		static AllocList g_listAllocated;

		void DbRemember(Unknown * pObj)
		{
			g_listAllocated.AddAsserted(pObj);
		}
		
		void DbForget(Unknown * pObj)
		{
			if (g_bAllocListActive)
				g_listAllocated.RemoveAsserted(pObj);
		}
		
	#endif // ! NDEBUG

	/******************************/
	/* Original File: iffFile.cpp */
	/******************************/

	bool GenericFile::Load(::MediaMedium * pMedium)
	{
		ArchvIn ar;
		
		ar.Open(pMedium);
		if (!ar.m_bError)
			Serialize(&ar);
		ar.Close();
		
		return ! ar.m_bError;
	}
	
	#ifndef IFF_READ_ONLY
		bool GenericFile::Write(::MediaMedium * pMedium)
		{
			ArchvOut ar;
			
			ar.Open(pMedium);
			if (!ar.m_bError)
				Serialize(&ar);
			ar.Close();
			
			return ! ar.m_bError;
		}
	#endif // ! IFF_READ_ONLY
	
	
	#ifdef _IFF_WIN_TARGET
		typedef ::MediaWinFileMedium DeviceFileHandle;
	#else
		typedef ::MediaStdFileMedium DeviceFileHandle;
	#endif
	
	bool GenericFile::Load(TCHAR const * pszFileName)
	{
		#ifndef IFF_READ_ONLY
			if (m_pszFileName) delete[] m_pszFileName;
			m_pszFileName = new TCHAR [_tcslen(pszFileName)+1];
			_tcscpy(m_pszFileName,pszFileName);
		#endif // ! IFF_READ_ONLY
		
		#ifdef _IFF_WIN_TARGET
			::MediaWinFileMedium * pMedium = new ::MediaWinFileMedium;
			pMedium->Open(pszFileName, GENERIC_READ);
		#else
			::MediaStdFileMedium * pMedium = new ::MediaStdFileMedium;
			pMedium->Open(pszFileName, "rb");
		#endif
		
		bool bRet = Load(pMedium);
		
		pMedium->Release();
		
		return bRet;
	}
	
	#ifndef IFF_READ_ONLY
		bool GenericFile::Write(TCHAR const * pszFileName)
		{
			if (pszFileName)
			{
				if (m_pszFileName) delete[] m_pszFileName;
				m_pszFileName = new TCHAR [_tcslen(pszFileName)+1];
				_tcscpy(m_pszFileName,pszFileName);
			}
			
			if (!m_pszFileName) return false;
			
			#ifdef _IFF_WIN_TARGET
				::MediaWinFileMedium * pMedium = new ::MediaWinFileMedium;
				pMedium->Open(pszFileName, GENERIC_WRITE);
			#else
				::MediaStdFileMedium * pMedium = new ::MediaStdFileMedium;
				pMedium->Open(pszFileName, "wb");
			#endif
			
			bool bRet = Write(pMedium);
			
			pMedium->Release();
			
			return bRet;
		}
	#endif // ! IFF_READ_ONLY

	void File::Serialize(Archive * pArchv)
	{
		if (pArchv->m_bIsLoading)
		{
			if (m_pContents)
			{
				m_pContents->Release();
				m_pContents = NULL;
			}
			
			pArchv->Transfer(m_idType);
			
			if (!!m_idType && (m_idType == ID("FORM") || m_idType == ID("LIST") || m_idType == ID("CAT ")))
			{
				m_pContents = static_cast<Composite *>(Chunk::Load(ID_ANY,pArchv,m_idType));
			}
			else
			{
				pArchv->m_bError = true;
			}
		}
		else
		{
			if (m_pContents)
				m_pContents->Chunk::Write(pArchv);
			else
				pArchv->m_bError = true;
		}
	}
	
	File::~File()
	{
		if (m_pContents)
			m_pContents->Release();
	}
	
	/*******************************/
	/* Original File: iffChunk.cpp */
	/*******************************/

	class RegEntry
	{
		public:
			ID m_idParent;
			ID m_idChunk;
			Chunk * (* m_pfnCreate) ();
			inline bool operator == (RegEntry const & rEntry) const
			{
				return m_idParent == rEntry.m_idParent && m_idChunk == rEntry.m_idChunk;
			}
			inline bool operator != (RegEntry const & rEntry) const
			{
				return ! operator == (rEntry);
			}
	};

} // namespace IFF



namespace IFF {

	inline unsigned HashFunction(IFF::RegEntry const & rEntry)
	{
		return ::HashFunction(rEntry.m_idChunk.m_nID);
	}
	
	static ::HashTable<RegEntry> * g_pRegister = NULL;
	
	void Chunk::Register(ID idParent, ID idChunk, Chunk * (* pfnCreate) () )
	{
		static ::HashTable<RegEntry> reg;
		
		g_pRegister = &reg;
		
		RegEntry entry;
		entry.m_idParent = idParent;
		entry.m_idChunk = idChunk;
		entry.m_pfnCreate = pfnCreate;
		
		reg.AddAsserted(entry);
	}

	Chunk * Chunk::DynCreate(ID idParent, ID idChunk)
	{
		if (g_pRegister)
		{
			RegEntry test;
			test.m_idParent = idParent;
			test.m_idChunk = idChunk;
			test.m_pfnCreate = NULL;
			
			RegEntry const * pEntry = g_pRegister->Contains(test);
			if (pEntry)
			{
				return pEntry->m_pfnCreate();
			}
		}
		return new MiscChunk(idChunk);
	}
	
	void Chunk::Write(Archive * pArchv)
	{
		Archive * pSubArchv = pArchv->OpenSubArchive();
		
		Serialize(pSubArchv);
		
		UINT32 nSize = pSubArchv->GetSize();
		pArchv->Transfer(m_idCk);
		pArchv->Transfer(nSize);
		
		pArchv->CloseSubArchive(pSubArchv);
		
		BYTE z = 0;
		if (nSize & 1) pArchv->Transfer(z);
	}
	
	Chunk * Chunk::Load(ID idParent, Archive * pArchv, ID idChunk, bool bKnown)
	{
		if (!idChunk)
			pArchv->Transfer(idChunk);
		
		Chunk * pChunk = bKnown ? DynCreate(idParent, idChunk) : new MiscChunk(idChunk);
		
		UINT32 nSize;
		pArchv->Transfer(nSize);
		
		Archive * pSubArchv = pArchv->OpenSubArchive(nSize);
		pChunk->Serialize(pSubArchv);
		pArchv->CloseSubArchive(pSubArchv);
		
		BYTE z = 0;
		if (nSize & 1) pArchv->Transfer(z);

		return pChunk;
	}
	
	Chunk * Chunk::GetProperty(ID idProp) const
	{
		if (m_pParent && m_pNode)
			return m_pParent->GetProperty(m_pNode,idProp);
		else
			return NULL;
	}
	
	/*******************************/
	/* Original File: iffBlock.cpp */
	/*******************************/

	Composite::~Composite()
	{
		DeleteAllChildren();
	}
	
	ChildNode * Composite::GetFirstChild(ID idMatch) const
	{
		for (ChildNode * pSrchNode = m_pFirst; pSrchNode; pSrchNode = GetNextChild(pSrchNode))
		{
			if (pSrchNode->GetChunk()->m_idCk == idMatch && !pSrchNode->GetChunk()->IsUnknown()) return pSrchNode;
		}
		return NULL;
	}
	
	ChildNode * Composite::GetLastChild(ID idMatch) const
	{
		for (ChildNode * pSrchNode = m_pLast; pSrchNode; pSrchNode = GetPrevChild(pSrchNode))
		{
			if (pSrchNode->GetChunk()->m_idCk == idMatch && !pSrchNode->GetChunk()->IsUnknown()) return pSrchNode;
		}
		return NULL;
	}
	
	ChildNode * Composite::GetNextChild(ChildNode const * pNode, ID idMatch)
	{
		for (ChildNode * pSrchNode = GetNextChild(pNode); pSrchNode; pSrchNode = GetNextChild(pSrchNode))
		{
			if (pSrchNode->GetChunk()->m_idCk == idMatch && !pSrchNode->GetChunk()->IsUnknown()) return pSrchNode;
		}
		return NULL;
	}
	
	ChildNode * Composite::GetPrevChild(ChildNode const * pNode, ID idMatch)
	{
		for (ChildNode * pSrchNode = GetPrevChild(pNode); pSrchNode; pSrchNode = GetPrevChild(pSrchNode))
		{
			if (pSrchNode->GetChunk()->m_idCk == idMatch && !pSrchNode->GetChunk()->IsUnknown()) return pSrchNode;
		}
		return NULL;
	}
	
	Chunk * Composite::GetProperty(ChildNode const * pNode, ID idProp) const
	{
		// search backward for ID
		
		ChildNode * pFindNode = GetPrevChild(pNode, idProp);
		
		if (pFindNode) return pFindNode->GetChunk();
		
		// if not found, search parent backwards, for "PROP ...." then get that
		// and if not in the parent, search its parent similarly
		// provided all these parents are of type LIST ....
		
		for (Composite const * pThis = this; pThis->m_pParent && pThis->m_pParent->m_idCk == ID("LIST"); pThis = pThis->m_pParent)
		{
			if (pThis->m_pNode)
			{
				for (ChildNode * pFindProp = pThis->m_pParent->GetPrevChild(pThis->m_pNode,"PROP"); pFindProp; pFindProp = pThis->m_pParent->GetPrevChild(pFindProp,"PROP"))
				{
					Composite * pProp = static_cast<Composite *>(pFindProp->GetChunk());
					if (pProp->m_idData == m_idData)
					{
						ChildNode * pFindNode = pProp->GetLastChild(idProp);
						
						if (pFindNode) return pFindNode->GetChunk();
					}
				}
				
			}
		}
		
		return NULL;
	}

	void Composite::DeleteChild(ChildNode * pNode)
	{
		if (pNode->m_pPrev)
			pNode->m_pPrev->m_pNext = pNode->m_pNext;
		else
			m_pFirst = pNode->m_pNext;
			
		if (pNode->m_pNext)
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		else
			m_pLast = pNode->m_pPrev;
		
		pNode->m_pChunk->m_pParent = NULL;
		pNode->m_pChunk->m_pNode = NULL;
		pNode->m_pChunk->Release();
		pNode->Release();
	}
	
	void Composite::DeleteAllChildren()
	{
		while (m_pFirst)
			DeleteChild(m_pFirst);
	}
	
	ChildNode * Composite::InsertChildFirst(Chunk * pChunk)
	{
		pChunk->AddRef();
		pChunk->m_pParent = this;
		
		ChildNode * pNode = new ChildNode;
		pChunk->m_pNode = pNode;
		
		pNode->m_pChunk = pChunk;
		pNode->m_pPrev = NULL;
		pNode->m_pNext = m_pFirst;
		
		if (m_pFirst)
			m_pFirst->m_pPrev = pNode;
		
		m_pFirst = pNode;
		
		if (!m_pLast)
			m_pLast = m_pFirst;
			
		return pNode;
	}
	
	ChildNode * Composite::InsertChildLast(Chunk * pChunk)
	{
		pChunk->m_pParent = this;
		pChunk->AddRef();
		
		ChildNode * pNode = new ChildNode;
		pChunk->m_pNode = pNode;
		
		pNode->m_pChunk = pChunk;
		pNode->m_pNext = NULL;
		pNode->m_pPrev = m_pLast;
		
		if (m_pLast)
			m_pLast->m_pNext = pNode;
			
		m_pLast = pNode;
		
		if (!m_pFirst)
			m_pFirst = m_pLast;
			
		return pNode;
	}
	
	ChildNode * Composite::InsertChildAfter(ChildNode * pRefNode, Chunk * pChunk)
	{
		pChunk->m_pParent = this;
		pChunk->AddRef();
		
		ChildNode * pNode = new ChildNode;
		pChunk->m_pNode = pNode;
		
		pNode->m_pChunk = pChunk;
		pNode->m_pNext = pRefNode->m_pNext;
		pNode->m_pPrev = pRefNode;
		
		if (pRefNode->m_pNext)
			pRefNode->m_pNext->m_pPrev = pNode;
		else
			m_pLast = pNode;
			
		pRefNode->m_pNext = pNode;
			
		return pNode;
	}
	
	ChildNode * Composite::InsertChildBefore(ChildNode * pRefNode, Chunk * pChunk)
	{
		pChunk->m_pParent = this;
		pChunk->AddRef();
		
		ChildNode * pNode = new ChildNode;
		pChunk->m_pNode = pNode;
		
		pNode->m_pChunk = pChunk;
		pNode->m_pPrev = pRefNode->m_pPrev;
		pNode->m_pNext = pRefNode;
		
		if (pRefNode->m_pPrev)
			pRefNode->m_pPrev->m_pNext = pNode;
		else
			m_pFirst = pNode;
			
		pRefNode->m_pPrev = pNode;
			
		return pNode;
	}
	
	bool Composite::EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const
	{
		if (m_idData != idData) return true;
		
		for (ChildNode * pNode = GetFirstChild(idChunk); pNode; pNode = GetNextChild(pNode,idChunk))
		{
			if (!pfnCallback(pNode->GetChunk(),pData)) return false;
		}
		
		return true;
	}
	
	bool Composite::IsValidChildID(ID) const
	{
		return true;
	}
	
	Chunk * Composite::LoadChunk(Archive * pArchv) const
	{
		ID idChunk;
		pArchv->Transfer(idChunk);
		
		return Chunk::Load(m_idData, pArchv, idChunk, IsValidChildID(idChunk));
	}
	
	void Composite::Serialize(Archive * pArchv)
	{
		pArchv->Transfer(m_idData);
		
		if (pArchv->m_bIsLoading)
		{
			DeleteAllChildren();
			
			while (pArchv->GetSize())
			{
				Chunk * pChunk = LoadChunk(pArchv);
				InsertChildLast(pChunk);
				pChunk->Release();
			}
		}
		else
		{
			for (ChildNode * pNode = m_pFirst; pNode; pNode = GetNextChild(pNode))
			{
				pNode->GetChunk()->Write(pArchv);
			}
		}
	}
	
	bool Form::IsValidChildID(ID id) const
	{
		// contents: FORM | LIST | CAT | LocalChunk
		return ID("PROP") != id;
	}

	bool Cat::EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const
	{
		for (ChildNode * pNode = GetFirstChild(); pNode; pNode = GetNextChild(pNode))
		{
			Composite const * pComposite = static_cast<Composite const *>(pNode->GetChunk());
			if (pComposite->m_idData == idData && !pComposite->EnumChildren(idData,idChunk,pfnCallback,pData)) return false;
		}
		
		return true;
	}
	
	bool Cat::IsValidChildID(ID id) const
	{
		// contentes: FROM | LIST | CAT
		return ID("FORM") == id || ID("LIST") == id || ID("CAT ") == id;
	}

	bool List::EnumChildren(ID idData, ID idChunk, bool (* pfnCallback) (Chunk *, void *), void * pData) const
	{
		for (ChildNode * pNode = GetFirstChild(); pNode; pNode = GetNextChild(pNode))
		{
			Composite const * pComposite = static_cast<Composite const *>(pNode->GetChunk());
			if (pComposite->m_idData == idData && !pComposite->EnumChildren(idData,idChunk,pfnCallback,pData)) return false;
		}
		
		return true;
	}
	
	bool List::IsValidChildID(ID id) const
	{
		// contentes: FROM | LIST | CAT | PROP
		return ID("FORM") == id || ID("LIST") == id || ID("CAT ") == id || ID("PROP") == id;
	}
	
	bool Prop::IsValidChildID(ID id) const
	{
		// contentes: LocalChunk
		return ID("FORM") != id && ID("LIST") != id && ID("CAT ") != id && ID("PROP") != id;
	}

	IFF_IMPLEMENT_DYNCREATE(ID_ANY,"FORM",Form)
	IFF_IMPLEMENT_DYNCREATE(ID_ANY,"LIST",List)
	IFF_IMPLEMENT_DYNCREATE(ID_ANY,"CAT ",Cat)
	IFF_IMPLEMENT_DYNCREATE(ID_ANY,"PROP",Prop)
	
	/*******************************/
	/* Original File: iffMscCk.cpp */
	/*******************************/

	#ifdef IFF_READ_ONLY
		
		void MiscChunk::Serialize(Archive * ){}
	
	#else // ! IFF_READ_ONLY
	
		void MiscChunk::Serialize(Archive * pArchv)
		{
			if (pArchv->m_bIsLoading)
			{
				if (m_pData)
					delete[] m_pData;
				m_nSize = pArchv->GetSize();
				m_pData = new BYTE [m_nSize];
			}
			
			pArchv->TransferBlock(m_pData,m_nSize);
		}
	
		MiscChunk::~MiscChunk()
		{
			if (m_pData)
				delete[] m_pData;
		}
	
		/*******************************/
		/* Original File: iffSData.cpp */
		/*******************************/

		DataBlock::~DataBlock()
		{
			delete[] m_pBlock;
		}
		
		inline bool DataBlock::WriteToFile(::MediaMedium * pMedium) const
		{
			pMedium->WriteBlock(m_pBlock,m_nCurSize);
			return pMedium->m_fError ? false : true;
		}
		
		void DataBlock::Expand(unsigned nMinSize)
		{
			while (m_nMaxSize < nMinSize)
				m_nMaxSize*=2;
			
			UBYTE * pNewBlock = new UBYTE [m_nMaxSize];
			memcpy(pNewBlock,m_pBlock,m_nCurSize);
			delete[] m_pBlock;
			m_pBlock = pNewBlock;
		}
		
		DataNode::~DataNode()
		{
			if (m_pPrev)
				m_pPrev->Release();
			if (m_pData)
				m_pData->Release();
			if (m_pNext)
				m_pNext->Release();
		}
		
		unsigned DataNode::GetDataSize() const
		{
			return
				  (m_pPrev ? m_pPrev->GetDataSize() : 0)
				+ (m_pData ? m_pData->GetDataSize() : 0)
				+ (m_pNext ? m_pNext->GetDataSize() : 0)
			;
		}
		
		bool DataNode::WriteToFile(::MediaMedium * pMedium) const
		{
			return
				   (m_pPrev ? m_pPrev->WriteToFile(pMedium) : true)
				&& (m_pData ? m_pData->WriteToFile(pMedium) : true)
				&& (m_pNext ? m_pNext->WriteToFile(pMedium) : true)
			;
		}
		
		SerialData::~SerialData()
		{
			m_pData->Release();
			if (m_pPrev)
				m_pPrev->Release();
		}
		
		void SerialData::Clear()
		{
			m_pData->Release();
			if (m_pPrev)
				m_pPrev->Release();
			m_pData = new DataBlock;
			m_pPrev = NULL;
		}
		
		unsigned SerialData::GetDataSize() const
		{
			return
				  (m_pPrev ? m_pPrev->GetDataSize() : 0)
				+ m_pData->GetDataSize()
			;
		}
		
		bool SerialData::WriteToFile(::MediaMedium * pMedium) const
		{
			return
				   (m_pPrev ? m_pPrev->WriteToFile(pMedium) : true)
				&& m_pData->WriteToFile(pMedium)
			;
		}
		
		/*******************************/
		/* Original File: iffArchO.cpp */
		/*******************************/

		void ArchvOut::Open(::MediaMedium * pMedium)
		{
			if (m_pMedium) m_pMedium->Release();
			
			m_pMedium = pMedium;
			m_pMedium->AddRef();
			
			if (m_pMedium->m_fError)
			{
				m_bError = true;
				m_pMedium->Release();
				m_pMedium = NULL;
			}
		}
		
		void ArchvOut::Close()
		{
			if (m_pMedium)
			{
				if (!WriteToFile(m_pMedium))
					m_bError = true;
				m_pMedium->Release();
				m_pMedium = NULL;
			}
		}
		
		ArchvOut::~ArchvOut()
		{
			Close();
		}
		
		Archive * ArchvOut::OpenSubArchive(unsigned)
		{
			return new ArchvOut;
		}
		
		void ArchvOut::CloseSubArchive(Archive * pSub)
		{
			m_bError = m_bError || pSub->m_bError;
			Append(static_cast<ArchvOut *>(pSub));
			pSub->Release();
		}
		
	#endif // IFF_READ_ONLY

	/*******************************/
	/* Original File: iffArchI.cpp */
	/*******************************/

	void ArchvIn::Open(::MediaMedium * pMedium)
	{
		if (m_pMedium) m_pMedium->Release();
		
		m_pMedium = pMedium;
		m_pMedium->AddRef();
		
		if (m_pMedium->m_fError)
		{
			m_bError = true;
			m_pMedium->Release();
			m_pMedium = NULL;
		}
		else
		{
			m_nEndPos = m_pMedium->GetRemainingSize();
			m_nBytesRemaining = m_nEndPos;
		}
	}
	
	void ArchvIn::Close()
	{
		if (m_pMedium)
		{
			m_pMedium->SetPos(m_nEndPos);
			m_pMedium->Release();
			m_pMedium = NULL;
		}
	}
	
	inline ArchvIn::ArchvIn(ArchvIn * pParent, unsigned nSize)
		: _IFF_ARCHI_FLAG(true)
		, m_pMedium(pParent->m_pMedium)
		, m_nBytesRemaining(nSize)
	{
		m_nEndPos = pParent->m_pMedium->GetPos();
		m_nEndPos += nSize;
		pParent->m_nBytesRemaining -= nSize;
		m_pMedium->AddRef();
	}
	
	ArchvIn::~ArchvIn()
	{
		Close();
	}

	Archive * ArchvIn::OpenSubArchive(unsigned nSize)
	{
		return new ArchvIn(this,nSize);
	}
	
	void ArchvIn::CloseSubArchive(Archive * pSub)
	{
		m_bError = m_bError || pSub->m_bError;
		pSub->Release();
	}

} // namespace IFF
