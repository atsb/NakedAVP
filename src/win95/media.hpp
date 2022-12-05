#ifndef _INCLUDED_MEDIA_HPP_
#define _INCLUDED_MEDIA_HPP_

#include <stdio.h>
#include <limits.h>
#include <string.h>
	
class MediaMedium;

// use this to read in simple data types
// note especially that if the size of TYPE is greater than the
// default buffer size, then the operation will fail
// and the virtual end of file error flag will be set
// - use ReadBlock instead
template <class TYPE>
void MediaRead(MediaMedium * pThis, TYPE * p);

// use this to write simple data types
// note especially that if the size of TYPE is greater than the
// default buffer size, then the operation will fail
// and the virtual end of file error flag will be set
// - use WriteBlock instead
template <class TYPE>
void MediaWrite(MediaMedium * pThis, TYPE d);

class MediaMedium
{
	protected:
		// standard constructor
		MediaMedium() :
			m_fError(0),
			m_nDefBufSize(1024),
			m_pWriteBuffer(NULL),
			m_pReadBuffer(NULL),
			m_nReadBufPos(0),
			m_nWriteBufPos(0),
			m_nBufSize(0),
			m_nBufLenUsed(0),
			m_nRefCnt(1)
			{}
			
		virtual ~MediaMedium() {}
	
	public:
		// allow reference counting
		unsigned AddRef() { return ++m_nRefCnt; }
		unsigned Release() { if (0==(--m_nRefCnt)) { delete this; return 0;} else return m_nRefCnt; }
	
		enum
		{
			// error code flags
			 MME_VEOFMET   = 0x00000001 // virtual end of file met
			,MME_EOFMET    = 0x00000002 // actual end of file met
			,MME_OPENFAIL  = 0x00000004 // failed to open medium
			,MME_CLOSEFAIL = 0x00000008 // failed to close medium
			,MME_UNAVAIL   = 0x00000010 // requested operation was not available
			,MME_IOERROR   = 0x00000020 // read/write operation failed
		};
		unsigned m_fError;
		
		unsigned m_nDefBufSize; // default read or write buffer sizes for buffering small objects
		
		// flush read/write buffers. You should call this function after the last read or write operation on the object
		// alternatively, derived (implementation) classes close methods should call this
		void Flush()
		{
			if (m_pReadBuffer)
			{
				unsigned nBufStartPos = DoGetPos();
				CloseReadBuffer(m_nReadBufPos > m_nBufLenUsed ? m_nReadBufPos : m_nBufLenUsed);
				DoSetPos(nBufStartPos + m_nReadBufPos);
				m_pReadBuffer = NULL;
				m_nReadBufPos = 0;
			}
			else if (m_pWriteBuffer)
			{
				unsigned nBufStartPos = DoGetPos();
				CloseWriteBuffer(m_nWriteBufPos > m_nBufLenUsed ? m_nWriteBufPos : m_nBufLenUsed);
				DoSetPos(nBufStartPos + m_nWriteBufPos);
				m_pWriteBuffer = NULL;
				m_nWriteBufPos = 0;
			}
			m_nBufSize = 0;
			m_nBufLenUsed = 0;
		}
		
		// use this to write a block of raw data
		void WriteBlock(void const * pData, unsigned nSize)
		{
			Flush();
			DoWriteBlock(pData,nSize);
		}
		
		// this may be faster, but will only work if the block size no more than the default buffer size
		void WriteBufferedBlock(void const * pData, unsigned nSize)
		{
			if (m_nWriteBufPos + nSize <= m_nBufSize)
			{
				memcpy(static_cast<char *>(m_pWriteBuffer) + m_nWriteBufPos/sizeof(char), pData, nSize);
				m_nWriteBufPos += nSize;
			}
			else
			{
				Flush();
				m_pWriteBuffer = GetWriteBuffer(&m_nBufSize,m_nDefBufSize);
				if (nSize <= m_nBufSize)
				{
					memcpy(m_pWriteBuffer, pData, nSize);
					m_nWriteBufPos = nSize;
				}
				else
				{
					m_fError |= MME_VEOFMET;
				}
			}
		}
		
		// use this to read a block of raw data
		void ReadBlock(void * pData, unsigned nSize)
		{
			Flush();
			DoReadBlock(pData,nSize);
		}
		
		// this may be faster, but will only work if the block size no more than the default buffer size
		void ReadBufferedBlock(void * pData, unsigned nSize)
		{
			if (m_nReadBufPos + nSize <= m_nBufSize)
			{
				memcpy(pData, static_cast<char const *>(m_pReadBuffer) + m_nReadBufPos/sizeof(char), nSize);
				m_nReadBufPos += nSize;
			}
			else
			{
				Flush();
				m_pReadBuffer = GetReadBuffer(&m_nBufSize,m_nDefBufSize);
				if (nSize <= m_nBufSize)
				{
					memcpy(pData, m_pReadBuffer, nSize);
					m_nReadBufPos = nSize;
				}
				else
				{
					m_fError |= MME_VEOFMET;
				}
			}
		}
		
		// move the 'file' pointer nOffset bytes
		// this will not necessarily cause buffers to be flushed
		// if the pointer can be moved within the current buffer,
		// some of the buffer may be left uninitialized, and no
		// error will occur, which otherwise might (particularly
		// if the object has write access)
		void MovePos(signed nOffset)
		{
			if (m_pReadBuffer)
			{
				if (nOffset>0 && m_nReadBufPos+nOffset<=m_nBufSize)
				{
					m_nReadBufPos+=nOffset;
					return;
				}
				else if (nOffset<=0 && m_nReadBufPos>=static_cast<unsigned>(-nOffset))
				{
					if (m_nBufLenUsed < m_nReadBufPos) m_nBufLenUsed = m_nReadBufPos;
					m_nReadBufPos+=nOffset;
					return;
				}
			}
			else if (m_pWriteBuffer)
			{
				if (nOffset>0 && m_nWriteBufPos+nOffset<=m_nBufSize)
				{
					m_nWriteBufPos+=nOffset;
					return;
				}
				else if (nOffset<=0 && m_nWriteBufPos>=static_cast<unsigned>(-nOffset))
				{
					if (m_nBufLenUsed < m_nWriteBufPos) m_nBufLenUsed = m_nWriteBufPos;
					m_nWriteBufPos+=nOffset;
					return;
				}
			}
			// else
			Flush();
			DoSetPos(DoGetPos()+nOffset);
		}
		
		// set the 'file' pointer
		// you would normally only pass values which have been
		// previously returned by a call to GetPos
		// note that this will not necessarily cause buffers to be flushed
		// if the pointer can be moved within the current buffer,
		// some of the buffer may be left uninitialized, and no
		// error will occur, which otherwise might (particularly
		// if the object has write access)
		void SetPos(unsigned nPos)
		{
			unsigned nNewBufPos = nPos - DoGetPos();
			if (nNewBufPos <= m_nBufSize)
			{
				if (m_pReadBuffer)
				{
					if (m_nBufLenUsed < m_nReadBufPos) m_nBufLenUsed = m_nReadBufPos;
					m_nReadBufPos = nNewBufPos;
				}
				else // pWriteBuffer
				{
					if (m_nBufLenUsed < m_nWriteBufPos) m_nBufLenUsed = m_nWriteBufPos;
					m_nWriteBufPos = nNewBufPos;
				}
			}
			else
			{
				Flush();
				DoSetPos(nPos);
			}
		}
		
		// get the 'file' pointer. The returned value
		// can be used in a call to SetPos
		unsigned GetPos()
		{
			return DoGetPos()+m_nReadBufPos+m_nWriteBufPos;
		}
		
		virtual unsigned GetRemainingSize();
		
	private:
		void * m_pWriteBuffer;
		void const * m_pReadBuffer;
		unsigned m_nReadBufPos;
		unsigned m_nWriteBufPos;
		unsigned m_nBufSize;
		unsigned m_nBufLenUsed;
		
		unsigned m_nRefCnt;
	
	protected:
	
		// the non-pure functions default implementation sets the unavailable error flag
		
		// it is safe to assume that these four functions will be called in a logical order
		// and that only one buffer (read or write) will be required at once
		
		// this two functions may return NULL only if *pSize is set to zero
		// *pSize should otherwise be set to the actual size of the buffer returned
		
		// get a pointer to memory where data can be written to directly
		virtual void * GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// get a pointer to memory where data can be read from directly
		virtual void const * GetReadBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer 'allocated' above and assume nPosOffset bytes were transferred
		// and that the 'file' pointer should be positioned at the end of the transferred data
		virtual void CloseWriteBuffer(unsigned nPosOffset);
		virtual void CloseReadBuffer(unsigned nPosOffset);
		
		// transfer a block of data
		// it is safe to assume that no buffer will be open
		virtual void DoWriteBlock(void const * pData, unsigned nSize);
		virtual void DoReadBlock(void * pData, unsigned nSize);
		
		// if a buffer is open, should return pos at start of buffer
		virtual unsigned DoGetPos() = 0;
		
		// it is safe to assume that no buffer will be open
		virtual void DoSetPos(unsigned nPos) = 0;
		
	friend class MediaSection;
	
	friend class _Media_CompilerHack;
};

class _Media_CompilerHack
{
	public:
		template <class TYPE>
		static inline void MediaRead(MediaMedium * pThis, TYPE * p)
		{
			if (pThis->m_nReadBufPos + sizeof(TYPE) <= pThis->m_nBufSize)
			{
				*p = *reinterpret_cast<TYPE const *>(static_cast<char const *>(pThis->m_pReadBuffer) + pThis->m_nReadBufPos/sizeof(char));
				
				pThis->m_nReadBufPos += sizeof(TYPE);
			}
			else
			{
				pThis->Flush();
				pThis->m_pReadBuffer = pThis->GetReadBuffer(&pThis->m_nBufSize,pThis->m_nDefBufSize);
				if (sizeof(TYPE) <= pThis->m_nBufSize)
				{
					*p = *static_cast<TYPE const *>(pThis->m_pReadBuffer);
					pThis->m_nReadBufPos = sizeof(TYPE);
				}
				else
				{
					pThis->m_fError |= MediaMedium::MME_VEOFMET;
				}
			}
		}
	
		template <class TYPE>
		static inline void MediaWrite(MediaMedium * pThis, TYPE d)
		{
			if (pThis->m_nWriteBufPos + sizeof(TYPE) <= pThis->m_nBufSize)
			{
				*reinterpret_cast<TYPE *>(static_cast<char *>(pThis->m_pWriteBuffer) + pThis->m_nWriteBufPos/sizeof(char)) = d;
				
				pThis->m_nWriteBufPos += sizeof(TYPE);
			}
			else
			{
				pThis->Flush();
				pThis->m_pWriteBuffer = pThis->GetWriteBuffer(&pThis->m_nBufSize,pThis->m_nDefBufSize);
				if (sizeof(TYPE) <= pThis->m_nBufSize)
				{
					*static_cast<TYPE *>(pThis->m_pWriteBuffer) = d;
					pThis->m_nWriteBufPos = sizeof(TYPE);
				}
				else
				{
					pThis->m_fError |= MediaMedium::MME_VEOFMET;
				}
			}
		}
};

// use this to read in simple data types
// note especially that if the size of TYPE is greater than the
// default buffer size, then the operation will fail
// and the virtual end of file error flag will be set
// - use ReadBlock instead
template <class TYPE>
inline void MediaRead(MediaMedium * pThis, TYPE * p)
{
	_Media_CompilerHack::MediaRead(pThis,p);
}

// use this to write simple data types
// note especially that if the size of TYPE is greater than the
// default buffer size, then the operation will fail
// and the virtual end of file error flag will be set
// - use WriteBlock instead
template <class TYPE>
inline void MediaWrite(MediaMedium * pThis, TYPE d)
{
	_Media_CompilerHack::MediaWrite(pThis,d);
}

#ifdef _MEDIA_WIN_TARGET

class MediaWinFileMedium : public MediaMedium
{
	public:
		MediaWinFileMedium() : m_hFile(INVALID_HANDLE_VALUE), m_nReadBufLen(0) {}
		
		void Attach(HANDLE hFile)
		{
			m_hFile = hFile;
		}
		void Detach()
		{
			Flush();
			m_hFile = INVALID_HANDLE_VALUE;
		}
		
		void Open(char *pszFileName, DWORD dwDesiredAccess)
		{
			DWORD dwShareMode;
			DWORD dwCreationDistribution;
			switch (dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE))
			{
				case 0:
					dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
					dwCreationDistribution = OPEN_EXISTING;
					break;
				case GENERIC_READ:
					dwShareMode = FILE_SHARE_READ;
					dwCreationDistribution = OPEN_EXISTING;
					break;
				case GENERIC_WRITE:
					dwShareMode = 0;
					dwCreationDistribution = CREATE_ALWAYS;
					break;
				default: // GENERIC_WRITE|GENERIC_READ
					dwCreationDistribution = OPEN_ALWAYS;
					dwShareMode = 0;
			}
			m_hFile = CreateFile
			(
				pszFileName,
				dwDesiredAccess,
				dwShareMode,
				0,
				dwCreationDistribution,
				FILE_ATTRIBUTE_NORMAL,
				0
			);
			if (INVALID_HANDLE_VALUE == m_hFile)
				m_fError |= MME_OPENFAIL;
		}
		void Close()
		{
			if (INVALID_HANDLE_VALUE == m_hFile)
				m_fError |= MME_CLOSEFAIL;
			else
			{
				Flush();
				if (!CloseHandle(m_hFile))
					m_fError |= MME_CLOSEFAIL;
				else
					m_hFile = INVALID_HANDLE_VALUE;
			}
		}
		
		~MediaWinFileMedium()
		{
			// should already be closed...
			Close();
		}

		virtual unsigned GetRemainingSize();
	
	private:
		HANDLE m_hFile;
		
		char * m_pBuffer;
		unsigned m_nReadBufLen;

	protected:
	
		// get a pointer to memory where data can be written to directly
		virtual void * GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// get a pointer to memory where data can be read from directly
		virtual void const * GetReadBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer allocated above and assume nPosOffset were transferred
		virtual void CloseWriteBuffer(unsigned nPosOffset);
		virtual void CloseReadBuffer(unsigned nPosOffset);
		
		// transfer a block of data: the buffer should be closed
		virtual void DoWriteBlock(void const * pData, unsigned nSize);
		virtual void DoReadBlock(void * pData, unsigned nSize);
		
		// if a buffer is open, should return pos at start of buffer
		virtual unsigned DoGetPos();
		
		// requires that no buffer is oben
		virtual void DoSetPos(unsigned nPos);
};

#endif // _MEDIA_WIN_TARGET

class MediaStdFileMedium : public MediaMedium
{
	public:
		MediaStdFileMedium() : m_pFile(NULL), m_nReadBufLen(0) {}
		
		void Attach(FILE * pFile)
		{
			m_pFile = pFile;
		}
		void Detach()
		{
			Flush();
			m_pFile = NULL;
		}
		
		void Open(char const * pszFileName, char const * pszOpenMode)
		{
			if (pszOpenMode[0] != 'r' || pszOpenMode[1] != 'b') {
				fprintf(stderr, "Open(%s, %s)\n", pszFileName, pszOpenMode);
				m_fError |= MME_OPENFAIL;
				return;
			} 
			m_pFile = OpenGameFile(pszFileName, FILEMODE_READONLY, FILETYPE_PERM);
			if (!m_pFile)
				m_fError |= MME_OPENFAIL;
		}
		void Close()
		{
			if (!m_pFile)
				m_fError |= MME_CLOSEFAIL;
			else
			{
				Flush();
				if (fclose(m_pFile))
					m_fError |= MME_CLOSEFAIL;
				else
					m_pFile = NULL;
			}
		}
		
		~MediaStdFileMedium()
		{
			// should already be closed...
			Close();
		}
	
		virtual unsigned GetRemainingSize();

	private:
		FILE * m_pFile;
		
		char * m_pBuffer;
		unsigned m_nReadBufLen;

	protected:
	
		// get a pointer to memory where data can be written to directly
		virtual void * GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// get a pointer to memory where data can be read from directly
		virtual void const * GetReadBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer allocated above and assume nPosOffset were transferred
		virtual void CloseWriteBuffer(unsigned nPosOffset);
		virtual void CloseReadBuffer(unsigned nPosOffset);
		
		// transfer a block of data: the buffer should be closed
		virtual void DoWriteBlock(void const * pData, unsigned nSize);
		virtual void DoReadBlock(void * pData, unsigned nSize);
		
		// if a buffer is open, should return pos at start of buffer
		virtual unsigned DoGetPos();
		
		// requires that no buffer is oben
		virtual void DoSetPos(unsigned nPos);
};

class MediaMemoryReadMedium : public MediaMedium
{
	public:
		MediaMemoryReadMedium() : m_pMem(NULL) {}
		
		void Open(void const * p)
		{
			m_pMem = p;
			m_nOffset = 0;
		}
		
		void Close()
		{
			if (m_pMem)
			{
				Flush();
				m_pMem = NULL;
			}
			else
				m_fError |= MME_CLOSEFAIL;
		}
		
	private:
		void const * m_pMem;
		
	protected:
		unsigned m_nOffset;
	
		// get a pointer to memory where data can be read from directly
		virtual void const * GetReadBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer allocated above and assume nPosOffset were transferred
		virtual void CloseReadBuffer(unsigned nPosOffset);
		
		// transfer a block of data: the buffer should be closed
		virtual void DoReadBlock(void * pData, unsigned nSize);
		
		// if a buffer is open, should return pos at start of buffer
		virtual unsigned DoGetPos();
		
		// requires that no buffer is oben
		virtual void DoSetPos(unsigned nPos);
};

class MediaMemoryMedium : public MediaMemoryReadMedium
{
	public:
		MediaMemoryMedium() : m_pMem(NULL) {}
		
		void Open(void * p)
		{
			m_pMem = p;
			MediaMemoryReadMedium::Open(p);
		}
		
		void Close()
		{
			MediaMemoryReadMedium::Close();
			m_pMem = NULL;
		}
		
	private:
		void * m_pMem;
		
	protected:
	
		// get a pointer to memory where data can be written to directly
		virtual void * GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer allocated above and assume nPosOffset were transferred
		virtual void CloseWriteBuffer(unsigned nPosOffset);
		
		// transfer a block of data: the buffer should be closed
		virtual void DoWriteBlock(void const * pData, unsigned nSize);
};


class MediaSection : public MediaMedium
{
	public:
		MediaSection() : m_pMedium(NULL) {}
		
		void Open(MediaMedium * pMedium, unsigned nMaxSize = UINT_MAX)
		{
			m_pMedium = pMedium;
			m_nMaxSize = nMaxSize;
			m_nPos = 0;
			m_nUsedPos = 0;
		}
		void Close()
		{
			if (m_pMedium)
				Flush();
			if (m_nPos > m_nUsedPos) m_nUsedPos = m_nPos;
			m_pMedium = NULL;
		}
		unsigned GetUsedSize() const
		{
			return (m_nPos > m_nUsedPos) ? m_nPos : m_nUsedPos;
		}
		
		virtual unsigned GetRemainingSize();

	private:
		MediaMedium * m_pMedium;
		unsigned m_nMaxSize;
		unsigned m_nPos;
		unsigned m_nUsedPos;
		
	protected:
		// get a pointer to memory where data can be written to directly
		virtual void * GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// get a pointer to memory where data can be read from directly
		virtual void const * GetReadBuffer(unsigned * pSize, unsigned nDesiredSize);
		
		// close the buffer allocated above and assume nPosOffset were transferred
		virtual void CloseWriteBuffer(unsigned nPosOffset);
		virtual void CloseReadBuffer(unsigned nPosOffset);
		
		// transfer a block of data: the buffer should be closed
		virtual void DoWriteBlock(void const * pData, unsigned nSize);
		virtual void DoReadBlock(void * pData, unsigned nSize);
		
		// if a buffer is open, should return pos at start of buffer
		virtual unsigned DoGetPos();
		
		// requires that no buffer is oben
		virtual void DoSetPos(unsigned nPos);
};


#endif
