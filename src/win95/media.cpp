#include "fixer.h"

#include "media.hpp"

void * MediaMedium::GetWriteBuffer(unsigned * pSize, unsigned /*nDesiredSize*/)
{
	*pSize = 0;
	m_fError |= MME_UNAVAIL;
	return NULL;
}

void const * MediaMedium::GetReadBuffer(unsigned * pSize, unsigned /*nDesiredSize*/)
{
	*pSize = 0;
	m_fError |= MME_UNAVAIL;
	return NULL;
}

void MediaMedium::CloseWriteBuffer(unsigned /*nPosOffset*/)
{
	m_fError |= MME_UNAVAIL;
}

void MediaMedium::CloseReadBuffer(unsigned /*nPosOffset*/)
{
	m_fError |= MME_UNAVAIL;
}

void MediaMedium::DoWriteBlock(void const * /*pData*/, unsigned /*nSize*/)
{
	m_fError |= MME_UNAVAIL;
}

void MediaMedium::DoReadBlock(void * /*pData*/, unsigned /*nSize*/)
{
	m_fError |= MME_UNAVAIL;
}

unsigned MediaMedium::GetRemainingSize()
{
	return UINT_MAX;
}

// MediaWinFileMedium

#ifdef _MEDIA_WIN_TARGET

unsigned MediaWinFileMedium::GetRemainingSize()
{
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	unsigned nSize = GetFileSize(m_hFile, 0);
	
	if (0xffffffff == nSize)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	return nSize - GetPos();	
}

void * MediaWinFileMedium::GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	m_pBuffer = new char [nDesiredSize];
	*pSize = nDesiredSize;
	return m_pBuffer;
}

void const * MediaWinFileMedium::GetReadBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (INVALID_HANDLE_VALUE == m_hFile)
		// the base/default implementation raises an error
		return MediaMedium::GetReadBuffer(pSize,nDesiredSize);
		
	m_pBuffer = new char [nDesiredSize];
	
	DWORD nBytesRead = 0;
	
	if (!ReadFile(m_hFile, m_pBuffer, nDesiredSize, &nBytesRead, 0))
		m_fError |= MME_IOERROR;
	else if (!nBytesRead)
		m_fError |= MME_EOFMET;
		
	*pSize = m_nReadBufLen = nBytesRead;
	
	return m_pBuffer;
}

void MediaWinFileMedium::CloseWriteBuffer(unsigned nPosOffset)
{
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseWriteBuffer(nPosOffset);
		return;
	}
		
	DWORD nBytesWritten = 0;
	
	if (!WriteFile(m_hFile, m_pBuffer, nPosOffset, &nBytesWritten, 0))
		m_fError |= MME_IOERROR;
	else if (nBytesWritten < nPosOffset)
		m_fError |= MME_EOFMET;
		
	delete [] m_pBuffer;
}

void MediaWinFileMedium::CloseReadBuffer(unsigned nPosOffset)
{
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseReadBuffer(nPosOffset);
		return;
	}
		
	if (nPosOffset != m_nReadBufLen && 0xffffffff == SetFilePointer(m_hFile,nPosOffset - m_nReadBufLen,0,FILE_CURRENT))
		m_fError |= MME_UNAVAIL;
		
	m_nReadBufLen = 0;
	
	delete [] m_pBuffer;
}

void MediaWinFileMedium::DoWriteBlock(void const * pData, unsigned nSize)
{
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		MediaMedium::DoWriteBlock(pData,nSize);
		return;
	}
	
	DWORD nBytesWritten = 0;
	
	if (!WriteFile(m_hFile, pData, nSize, &nBytesWritten, 0))
		m_fError |= MME_IOERROR;
	else if (nBytesWritten < nSize)
		m_fError |= MME_EOFMET;
}

void MediaWinFileMedium::DoReadBlock(void * pData, unsigned nSize)
{
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		MediaMedium::DoReadBlock(pData,nSize);
		return;
	}
	
	DWORD nBytesRead = 0;
	
	if (!ReadFile(m_hFile, pData, nSize, &nBytesRead, 0))
		m_fError |= MME_IOERROR;
	else if (nBytesRead < nSize)
		m_fError |= MME_EOFMET;
}

unsigned MediaWinFileMedium::DoGetPos()
{
	unsigned nFilePos = SetFilePointer(m_hFile,0,0,FILE_CURRENT);
	
	if (0xffffffff == nFilePos)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	return nFilePos - m_nReadBufLen;
}

void MediaWinFileMedium::DoSetPos(unsigned nPos)
{
	if (0xffffffff == SetFilePointer(m_hFile,nPos,0,FILE_BEGIN))
		m_fError |= MME_UNAVAIL;
}

#endif // _MEDIA_WIN_TARGET

// MediaStdFileMedium

unsigned MediaStdFileMedium::GetRemainingSize()
{
	if (!m_pFile)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	long nPos = ftell(m_pFile);
	
	if (-1L == nPos || fseek(m_pFile,0,SEEK_END))
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	long nSize = ftell(m_pFile);
	
	fseek(m_pFile,nPos,SEEK_SET);
	
	if (-1L == nSize)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	return nSize - GetPos();	
}

void * MediaStdFileMedium::GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	m_pBuffer = new char [nDesiredSize];
	*pSize = nDesiredSize;
	return m_pBuffer;
}

void const * MediaStdFileMedium::GetReadBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (!m_pFile)
		// the base/default implementation raises an error
		return MediaMedium::GetReadBuffer(pSize,nDesiredSize);
		
	m_pBuffer = new char [nDesiredSize];
	
	*pSize = m_nReadBufLen = fread(m_pBuffer, 1, nDesiredSize, m_pFile);
	
	if (!m_nReadBufLen)
	{
		if (feof(m_pFile))
			m_fError |= MME_EOFMET;
		else
			m_fError |= MME_IOERROR;
	}
	
	return m_pBuffer;
}

void MediaStdFileMedium::CloseWriteBuffer(unsigned nPosOffset)
{
	if (!m_pFile)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseWriteBuffer(nPosOffset);
		return;
	}
		
	if (fwrite(m_pBuffer, 1, nPosOffset, m_pFile) < nPosOffset)
	{
		if (feof(m_pFile))
			m_fError |= MME_EOFMET;
		else
			m_fError |= MME_IOERROR;
	}
	
	delete [] m_pBuffer;
}

void MediaStdFileMedium::CloseReadBuffer(unsigned nPosOffset)
{
	if (!m_pFile)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseReadBuffer(nPosOffset);
		return;
	}
		
	if (nPosOffset != m_nReadBufLen && fseek(m_pFile,nPosOffset - m_nReadBufLen,SEEK_CUR))
		m_fError |= MME_UNAVAIL;
		
	m_nReadBufLen = 0;
	delete [] m_pBuffer;
}

void MediaStdFileMedium::DoWriteBlock(void const * pData, unsigned nSize)
{
	if (!m_pFile)
	{
		MediaMedium::DoWriteBlock(pData,nSize);
		return;
	}
	
	if (fwrite(pData, 1, nSize, m_pFile) < nSize)
	{
		if (feof(m_pFile))
			m_fError |= MME_EOFMET;
		else
			m_fError |= MME_IOERROR;
	}
}

void MediaStdFileMedium::DoReadBlock(void * pData, unsigned nSize)
{
	if (!m_pFile)
	{
		MediaMedium::DoReadBlock(pData,nSize);
		return;
	}
	
	if (fread(pData, 1, nSize, m_pFile) < nSize)
	{
		if (feof(m_pFile))
			m_fError |= MME_EOFMET;
		else
			m_fError |= MME_IOERROR;
	}
}

unsigned MediaStdFileMedium::DoGetPos()
{
	long nFilePos = ftell(m_pFile);
	
	if (-1L == nFilePos)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	return nFilePos - m_nReadBufLen;
}

void MediaStdFileMedium::DoSetPos(unsigned nPos)
{
	if (fseek(m_pFile,nPos,SEEK_SET))
		m_fError |= MME_UNAVAIL;
}

// MediaMemoryReadMedium

void const * MediaMemoryReadMedium::GetReadBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (!m_pMem)
		// the base/default implementation raises an error
		return MediaMedium::GetReadBuffer(pSize,nDesiredSize);
		
	*pSize = nDesiredSize;
	
	return static_cast<char const *>(m_pMem)+m_nOffset/sizeof(char);
}

void MediaMemoryReadMedium::CloseReadBuffer(unsigned nPosOffset)
{
	if (!m_pMem)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseReadBuffer(nPosOffset);
		return;
	}
	
	m_nOffset += nPosOffset;
}

void MediaMemoryReadMedium::DoReadBlock(void * pData, unsigned nSize)
{
	if (!m_pMem)
	{
		MediaMedium::DoReadBlock(pData,nSize);
		return;
	}
	
	memcpy(pData,static_cast<char const *>(m_pMem)+m_nOffset/sizeof(char),nSize);
	
	m_nOffset += nSize;
}

unsigned MediaMemoryReadMedium::DoGetPos()
{
	return m_nOffset;
}

void MediaMemoryReadMedium::DoSetPos(unsigned nPos)
{
	m_nOffset = nPos;
}

// MediaMemoryMedium

void * MediaMemoryMedium::GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (!m_pMem)
		// the base/default implementation raises an error
		return MediaMedium::GetWriteBuffer(pSize,nDesiredSize);
		
	*pSize = nDesiredSize;
	
	return static_cast<char *>(m_pMem)+m_nOffset/sizeof(char);
}

void MediaMemoryMedium::CloseWriteBuffer(unsigned nPosOffset)
{
	if (!m_pMem)
	{
		// the base/default implementation raises an error
		MediaMedium::CloseWriteBuffer(nPosOffset);
		return;
	}
	
	m_nOffset += nPosOffset;
}

void MediaMemoryMedium::DoWriteBlock(void const * pData, unsigned nSize)
{
	if (!m_pMem)
	{
		MediaMedium::DoWriteBlock(pData,nSize);
		return;
	}
	
	memcpy(static_cast<char *>(m_pMem)+m_nOffset/sizeof(char),pData,nSize);
	
	m_nOffset += nSize;
}

// MediaSection

unsigned MediaSection::GetRemainingSize()
{
	if (!m_pMedium)
	{
		m_fError |= MME_UNAVAIL;
		return 0;
	}
	
	unsigned nSectionSize = m_nMaxSize - GetPos();
	unsigned nMediaSize = m_pMedium->GetRemainingSize();
	m_fError |= m_pMedium->m_fError;
	
	return nSectionSize < nMediaSize ? nSectionSize : nMediaSize;
}

void * MediaSection::GetWriteBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (!m_pMedium)
		return MediaMedium::GetWriteBuffer(pSize,nDesiredSize);
	
	if (m_nPos + nDesiredSize > m_nMaxSize)
	{
		nDesiredSize = m_nMaxSize - m_nPos;
	}
	
	void * p = m_pMedium->GetWriteBuffer(pSize,nDesiredSize);
	m_fError |= m_pMedium->m_fError;
	return p;
}

void const * MediaSection::GetReadBuffer(unsigned * pSize, unsigned nDesiredSize)
{
	if (!m_pMedium)
		return MediaMedium::GetReadBuffer(pSize,nDesiredSize);
	
	if (m_nPos + nDesiredSize > m_nMaxSize)
	{
		nDesiredSize = m_nMaxSize - m_nPos;
	}
	
	void const * p = m_pMedium->GetReadBuffer(pSize,nDesiredSize);
	m_fError |= m_pMedium->m_fError;
	return p;
}

void MediaSection::CloseWriteBuffer(unsigned nPosOffset)
{
	if (!m_pMedium)
	{
		MediaMedium::CloseWriteBuffer(nPosOffset);
		return;
	}
	
	m_nPos += nPosOffset;
	
	m_pMedium->CloseWriteBuffer(nPosOffset);
	m_fError |= m_pMedium->m_fError;
}

void MediaSection::CloseReadBuffer(unsigned nPosOffset)
{
	if (!m_pMedium)
	{
		MediaMedium::CloseReadBuffer(nPosOffset);
		return;
	}
	
	m_nPos += nPosOffset;
	
	m_pMedium->CloseReadBuffer(nPosOffset);
	m_fError |= m_pMedium->m_fError;
}

void MediaSection::DoWriteBlock(void const * pData, unsigned nSize)
{
	if (!m_pMedium)
	{
		MediaMedium::DoWriteBlock(pData,nSize);
		return;
	}
	
	if (m_nPos + nSize > m_nMaxSize)
	{
		m_fError |= MME_VEOFMET;
		return;
	}
	
	m_nPos += nSize;
	
	m_pMedium->DoWriteBlock(pData,nSize);
	m_fError |= m_pMedium->m_fError;
}

void MediaSection::DoReadBlock(void * pData, unsigned nSize)
{
	if (!m_pMedium)
	{
		MediaMedium::DoReadBlock(pData,nSize);
		return;
	}
	
	if (m_nPos + nSize > m_nMaxSize)
	{
		m_fError |= MME_VEOFMET;
		return;
	}
	
	m_nPos += nSize;
	
	m_pMedium->DoReadBlock(pData,nSize);
	m_fError |= m_pMedium->m_fError;
}

unsigned MediaSection::DoGetPos()
{
	return m_nPos;
}

void MediaSection::DoSetPos(unsigned nPos)
{
	if (!m_pMedium)
	{
		m_fError |= MME_UNAVAIL;
		return;
	}
	
	if (nPos > m_nMaxSize)
	{
		m_fError |= MME_VEOFMET;
		return;
	}
	
	m_pMedium->DoSetPos(m_pMedium->DoGetPos()+nPos-m_nPos);
	if (m_nPos > m_nUsedPos) m_nUsedPos = m_nPos;
	m_nPos = nPos;
	
	m_fError |= m_pMedium->m_fError;
}

