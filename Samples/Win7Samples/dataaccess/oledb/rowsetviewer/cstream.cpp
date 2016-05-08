//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CSTREAM.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


///////////////////////////////////////////////////////////////////////////////
// Class CStream
// 
///////////////////////////////////////////////////////////////////////////////
CStream::CStream(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CAsynchBase(eCStream, pCMainWindow, pCMDIChild)
{
	//Storage Interfaces	
	m_pISequentialStream	= NULL;
	m_pIStream				= NULL;
	m_pIStorage				= NULL;
	m_pILockBytes			= NULL;

	//Data
	m_wType					= DBTYPE_STR;
}


///////////////////////////////////////////////////////////////////////////////
// ~CStream
// 
///////////////////////////////////////////////////////////////////////////////
CStream::~CStream()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CStream::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CStream::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(ISequentialStream);
	HANDLE_GETINTERFACE(IStream);
	HANDLE_GETINTERFACE(IStorage);
	HANDLE_GETINTERFACE(ILockBytes);

	//Otherwise delegate
	return CAsynchBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CStream::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::AutoRelease()
{
	//Stream
	RELEASE_INTERFACE(ISequentialStream);
	RELEASE_INTERFACE(IStream);
	RELEASE_INTERFACE(IStorage);
	RELEASE_INTERFACE(ILockBytes);

	//Delegate
	return CAsynchBase::AutoRelease();
}


////////////////////////////////////////////////////////////////
// CStream::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CAsynchBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(ISequentialStream);
	}

	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		//[OPTIONAL]
		OBTAIN_INTERFACE(IStream);
		OBTAIN_INTERFACE(IStorage);
		OBTAIN_INTERFACE(ILockBytes);
	}

	return S_OK;
}



////////////////////////////////////////////////////////////////
// CStream::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void CStream::OnDefOperation() 
{ 
	//Need to bring up the StreamViewer
	if(m_pCMainWindow)
	{
		UINT uID = IDM_ISEQSTREAM_READ;

		//Determine which interface to use for reading.
		//Since the user just clicked on the object we have to guess which interface to use from those 
		//that are available, and most functional...
		if(m_pIStream)
			uID = IDM_ISTREAM_READ;

		//Display the Stream Viewer...
		//NOTE: This dialog could be displayed from numerous sources, either the execute dialog,
		//or directly from clicking on the stream - (ie: use GetFocus to determine the parent window)
		m_pCMainWindow->DisplayDialog(IDD_STREAM_VIEWER, GetFocus(), CMainWindow::StreamViewerProc, this, uID);
	}
}


/////////////////////////////////////////////////////////////////
// CStream::DisplayObject
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::DisplayObject()
{
	HRESULT hr = S_OK;

	//Display the object...
	OnDefOperation();

	//Delegate
	TESTC(hr = CAsynchBase::DisplayObject());

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::ReadBytes
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::ReadBytes(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, DBLENGTH cbMaxSize, void* pBuffer, DBLENGTH* pcbRead)
{
	HRESULT hr = S_OK;
	BOOL	bUseDefault = FALSE;

	ULONG	cbRead = 0;
	cBytes = min(cBytes, cbMaxSize);
	
	//Determine object type
	if(riid != IID_ISequentialStream && riid != IID_ILockBytes && riid != IID_IStream)
		bUseDefault = TRUE;
	
	//IID_ISequentialStream
	if((riid == IID_ISequentialStream && m_pISequentialStream) || (bUseDefault && m_pISequentialStream))
	{
		//ISequentialStream::Read
		//TODO64:  I*::Read only takes a ULONG
		//NOTE: ::Read can post errorinfo objects in the case of warnings.  SQLOLEDB's XML Stream
		//object will post execution errors on the last read...
		XTEST_(hr = m_pISequentialStream->Read(pBuffer, (ULONG)cBytes, &cbRead),S_FALSE);
		TESTC(TRACE_METHOD(hr, L"ISequentialStream::Read(0x%p, %d, &%d)", pBuffer, cBytes, cbRead));
	}
	//IID_ILockBytes
	else if((riid == IID_ILockBytes && m_pILockBytes) || (bUseDefault && m_pILockBytes))
	{
		ULARGE_INTEGER ulgOffset;
		ulgOffset.QuadPart = ulOffset;
		
		//ILockBytes::ReadAt
		//TODO64:  I*::Read only takes a ULONG
		XTEST_(hr = m_pILockBytes->ReadAt(ulgOffset, pBuffer, (ULONG)cBytes, &cbRead),S_FALSE);
		TESTC(TRACE_METHOD(hr, L"ILockBytes::ReadAt(%d, 0x%p, %d, &%d)", ulOffset, pBuffer, cBytes, cbRead));
	}
	//IID_IStream
	else if((riid == IID_IStream && m_pIStream) || (bUseDefault && m_pIStream))
	{
		//IStream::Read
		//TODO64:  I*::Read only takes a ULONG
		XTEST_(hr = m_pIStream->Read(pBuffer, (ULONG)cBytes, &cbRead),S_FALSE);
		TESTC(TRACE_METHOD(hr, L"IStream::Read(0x%p, %d, &%d)", pBuffer, cBytes, cbRead));
	}
//	else if(m_pIStorage)
//	{
		//TODO
//	}
	else
	{
		TESTC(hr = E_FAIL);
	}

CLEANUP:
	if(pcbRead)
		*pcbRead = cbRead;
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::ReadString
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::ReadString(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, DBLENGTH ulMaxSize, WCHAR* pwszBuffer, DBLENGTH* pcbRead)
{
	ASSERT(pwszBuffer);
	HRESULT hr = S_OK;
	pwszBuffer[0] = wEOL;
	
	WCHAR	pBuffer[MAX_COL_SIZE+1] = {0};
	DWORD	dwConvFlags = GetOptions()->m_dwConvFlags; 

	DBLENGTH cbRead = 0;
	cBytes = min(min(sizeof(pBuffer), cBytes), ulMaxSize*sizeof(WCHAR));
	
	//Delegate
	TESTC(hr = ReadBytes(riid, ulOffset, cBytes, ulMaxSize*sizeof(WCHAR), pBuffer, &cbRead));

	//What type of data are we reading
	switch(m_wType)
	{
		case DBTYPE_WSTR:
		case DBTYPE_STR:
			//Add a NULL terminator on the end of the data before conversion
			//since the spec indicates the stream is not NULL terminated...
			//NOTE: We made sure we had a full extra WCHAR in the buffer ahead of time
			memset((BYTE*)pBuffer + min(cBytes, cbRead), 0, sizeof(WCHAR));
			cbRead += (m_wType == DBTYPE_WSTR) ? sizeof(WCHAR) : sizeof(CHAR);
			break;
	};

	//Now that we have read the data, coerce it into a string for display purposes...
	XTESTC_(hr = DataConvert
		(
			DBSTATUS_S_OK,
			cbRead,
			sizeof(pBuffer),	
			(m_wType == DBTYPE_WSTR || m_wType == DBTYPE_STR) ? m_wType : DBTYPE_BYTES,
			pBuffer,
			0,
			0,
			
			DBTYPE_WSTR,
			NULL,
			NULL,
			pwszBuffer,
			ulMaxSize*sizeof(WCHAR),
			dwConvFlags
		),S_OK);

CLEANUP:
	if(pcbRead)
		*pcbRead = cbRead;
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::WriteBytes
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::WriteBytes(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, void* pBuffer, DBLENGTH* pcbWritten)
{
	HRESULT hr = S_OK;
	BOOL	bUseDefault = FALSE;
	ULONG	cbWritten = 0;

	//Determine object type
	if(riid != IID_ISequentialStream && riid != IID_ILockBytes && riid != IID_IStream)
		bUseDefault = TRUE;
	
	//IID_ISequentialStream
	if((riid == IID_ISequentialStream && m_pISequentialStream) || (bUseDefault && m_pISequentialStream))
	{
		//ISequentialStream::Write
		//TODO64:  I*::Write only takes a ULONG
		XTEST(hr = m_pISequentialStream->Write(pBuffer, (ULONG)cBytes, &cbWritten));
		TESTC(TRACE_METHOD(hr, L"ISequentialStream::Write(0x%p, %d, &%d)", pBuffer, cBytes, cbWritten));
	}
	//IID_ILockBytes
	else if((riid == IID_ILockBytes && m_pILockBytes) || (bUseDefault && m_pILockBytes))
	{
		ULARGE_INTEGER ulgOffset;
		ulgOffset.QuadPart = ulOffset;
		
		//ILockBytes::WriteAt
		//TODO64:  I*::Write only takes a ULONG
		XTEST(hr = m_pILockBytes->WriteAt(ulgOffset, pBuffer, (ULONG)cBytes, &cbWritten));
		TESTC(TRACE_METHOD(hr, L"ILockBytes::WriteAt(%d, 0x%p, %d, &%d)", ulOffset, pBuffer, cBytes, cbWritten));
	}
	//IID_IStream
	else if((riid == IID_IStream && m_pIStream) || (bUseDefault && m_pIStream))
	{
		//IStream::Write
		//TODO64:  I*::Write only takes a ULONG
		XTEST(hr = m_pIStream->Write(pBuffer, (ULONG)cBytes, &cbWritten));
		TESTC(TRACE_METHOD(hr, L"IStream::Write(0x%p, %d, &%d)", pBuffer, cBytes, cbWritten));
	}
//	else if(m_pIStorage)
//	{
		//TODO
//	}
	else
	{
		TESTC(hr = E_FAIL);
	}

CLEANUP:
	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::WriteString
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::WriteString(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, WCHAR* pwszBuffer, DBLENGTH* pcbWritten)
{
	HRESULT hr = S_OK;
	WCHAR	pBuffer[MAX_COL_SIZE] = {0};
	DWORD	dwConvFlags = GetOptions()->m_dwConvFlags; 

	DBLENGTH cbWritten = 0;
	cBytes = min(sizeof(pBuffer), cBytes);

	//First Convert the user entered data into the backend format...
	//So the stream is a native stream of bytes to set into the backend...
	XTESTC_(hr = DataConvert
		(
			DBSTATUS_S_OK, 
			cBytes, 
			cBytes + sizeof(WCHAR),
			DBTYPE_WSTR,
			pwszBuffer,
			0,	//Precision
			0,	//Scale
			
			(m_wType == DBTYPE_IUNKNOWN || m_wType == (DBTYPE_BYREF|DBTYPE_IUNKNOWN))
				? DBTYPE_BYTES : m_wType,
			
			NULL,		//&dbDstStatus
			&cBytes,	//&cbDstLength
			pBuffer,
			sizeof(pBuffer),
			dwConvFlags
		),S_OK);


	//Delegate
	TESTC(hr = WriteBytes(riid, ulOffset, cBytes, pBuffer, &cbWritten));

CLEANUP:
	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::Clone
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::Clone(IStream** ppStream)
{
	HRESULT hr = S_OK;

	if(m_pIStream)
	{	
		//IStream::Clone
		XTEST(hr = m_pIStream->Clone(ppStream));
		TESTC(TRACE_METHOD(hr, L"IStream::Clone(&0x%p)", ppStream ? *ppStream : NULL));
	}

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::Seek
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
	HRESULT hr = E_INVALIDARG;

	if(m_pIStream)
	{	
		//IStream::Seek
		TEST(hr = m_pIStream->Seek(dlibMove, dwOrigin, plibNewPosition));
		TESTC(TRACE_METHOD(hr, L"IStream::Seek(%lld, %d, &%lld)", dlibMove.QuadPart, dwOrigin, plibNewPosition ? plibNewPosition->QuadPart : 0));
	}

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CStream::Stat
//
/////////////////////////////////////////////////////////////////
HRESULT CStream::Stat(STATSTG* pstatstg)
{
	ASSERT(pstatstg);
	HRESULT hr = S_OK;
	WCHAR* pwszProgID = NULL;

	if(m_pIStream)
	{	
		memset(pstatstg, 0, sizeof(STATSTG));

		//IStream::Stat
		XTEST(hr = m_pIStream->Stat(pstatstg, STATFLAG_DEFAULT));

		pwszProgID = GetProgID(pstatstg->clsid);
		TESTC(TRACE_METHOD(hr, L"IStream::Stat(&{\"%s\", %d, %ld, 0x%08x, 0x%08x, 0x%08x, %d, %d, \"%s\", 0x%08x, %d}, \"%s\")", pstatstg->pwcsName, pstatstg->type, (ULONG)pstatstg->cbSize.QuadPart, &pstatstg->mtime, &pstatstg->ctime, &pstatstg->atime, pstatstg->grfMode, pstatstg->grfLocksSupported, pwszProgID, pstatstg->grfStateBits, pstatstg->reserved, L"STATFLAG_DEFAULT"));
	}

CLEANUP:
	SAFE_FREE(pwszProgID);
	return hr;
}



///////////////////////////////////////////////////////////////////////////////
// Class CStorageBuffer
// 
// My implementation of storage interfaces
///////////////////////////////////////////////////////////////////////////////
CStorageBuffer::CStorageBuffer() 
{
	m_iPos		= 0;
	m_cRef		= 1;
	
	m_pBuffer   = NULL;
	m_cMaxSize  = 0;
}


CStorageBuffer::~CStorageBuffer()
{
	SAFE_FREE(m_pBuffer);
}

ULONG	CStorageBuffer::AddRef()
{
	m_cRef++;
	return m_cRef;
}

ULONG	CStorageBuffer::Release()
{
	ASSERT(m_cRef);
	m_cRef--;
	
	if(m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

HRESULT CStorageBuffer::QueryInterface(REFIID riid, void** ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;
	HRESULT hr = E_NOINTERFACE;

	if (riid == IID_IUnknown)
		*ppv = (IUnknown*)(ISequentialStream*)this;
	else if (riid == IID_ISequentialStream)
		*ppv = (ISequentialStream*)this;
	else if (riid == IID_IStream)
		*ppv = (IStream*)this;
	else if (riid == IID_ILockBytes)
		*ppv = (ILockBytes*)this;
//	else if (riid == IID_IStorage)
//		*ppv = (IStorage*)this;
	
	if(*ppv)
	{
		//Avoid internal notiifcations
		m_cRef++;
		hr = S_OK;
	}

	return hr;
}


HRESULT CStorageBuffer::ReadBuffer(ULONG ulOffset, void *pv, ULONG cb, ULONG* pcbRead)
{
	//Parameter checking
	if(pcbRead)
		*pcbRead = 0;

	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(cb == 0)
		return S_OK;

	//Actual code
	ULONG cBytesLeft = m_cMaxSize > ulOffset ? m_cMaxSize - ulOffset : 0;
	ULONG cBytesRead = cb > cBytesLeft ? cBytesLeft : cb;

	//if no more bytes to retrive return 
	if(cBytesLeft == 0)
		return S_FALSE; 

	//Copy to users buffer the number of bytes requested or remaining
	memcpy(pv, (void*)((BYTE*)m_pBuffer + ulOffset), cBytesRead);

	if(pcbRead)
		*pcbRead = cBytesRead;

	if(cb != cBytesRead)
		return S_FALSE; 

	return S_OK;
}


HRESULT CStorageBuffer::WriteBuffer(ULONG ulOffset, const void *pv, ULONG cb, ULONG* pcbWritten)
{
	//Parameter checking
	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(pcbWritten)
		*pcbWritten = 0;

	if(cb == 0)
		return S_OK;

	//May need to Enlarge the current buffer
	if(ulOffset + cb >= m_cMaxSize)
	{
		//Need to append to the end of the stream
		ULARGE_INTEGER largeSize = { ulOffset + cb };
		if(FAILED(SetSize(largeSize)))
			return E_OUTOFMEMORY;
	}
	
	//Copy to the buffer
	memcpy((void*)((BYTE*)m_pBuffer + ulOffset), pv, cb);

	if(pcbWritten)
		*pcbWritten = cb;

	return S_OK;
}

	
HRESULT CStorageBuffer::Read(void *pv, ULONG cb, ULONG* pcbRead)
{
	ULONG cbRead = 0;
	HRESULT hr = S_OK;

	//Delegate, reading from the current position
	XTEST(hr = ReadBuffer(m_iPos, pv, cb, &cbRead));
	TRACE_METHOD(hr, L"\tISequentialStream::Read(0x%p, %d, &%d)", pv, cb, cbRead);

	if(SUCCEEDED(hr))
		m_iPos += cbRead;

	if(pcbRead)
		*pcbRead = cbRead;

	return hr;
}
      

HRESULT CStorageBuffer::Write(const void *pv, ULONG cb, ULONG* pcbWritten)
{
	ULONG cbWritten = 0;
	HRESULT hr = S_OK;

	//Delegate, writting to the current position
	XTEST(hr = WriteBuffer(m_iPos, pv, cb, &cbWritten));
	TRACE_METHOD(hr, L"\tISequentialStream::Write(0x%p, %d, &%d)", pv, cb, cbWritten);

	if(SUCCEEDED(hr))
		m_iPos += cbWritten;

	if(pcbWritten)
		*pcbWritten = cbWritten;

	return hr;
}
	

HRESULT CStorageBuffer::Seek(LONG lOffset, DWORD dwOrigin, ULONG* pulNewPosition)
{
	HRESULT hr = S_OK;
	LONG lNewPos = 0;

	//Determine with respect to what the origin is...
	switch(dwOrigin)
	{
		case STREAM_SEEK_SET:
			lNewPos = lOffset;
			break;

		case STREAM_SEEK_CUR:
			lNewPos = m_iPos + lOffset;
			break;

		case STREAM_SEEK_END:
			lNewPos = m_cMaxSize + lOffset;
			break;

		default:
			//The value of the dwOrigin parameter is not valid
			hr = STG_E_INVALIDFUNCTION;
			goto CLEANUP;
			break;
	};

	if(lNewPos<0 || (ULONG)lNewPos>m_cMaxSize)
		return STG_E_INVALIDFUNCTION; //No good return code for this situaiton?

	//Reset the current buffer position
	m_iPos = lNewPos;

CLEANUP:	
	if(pulNewPosition)
	   *pulNewPosition = m_iPos;
	TRACE_METHOD(hr, L"\tIStream::Seek(%d, %d, &%d)", lOffset, dwOrigin, m_iPos);
	return S_OK;
}

//IStream interfaces
HRESULT CStorageBuffer::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
{
	HRESULT hr = S_OK;
	ULONG ulNewPosition = 0;
	
	//Delegate to the simplier version
	TESTC(hr = Seek((LONG)dlibMove.QuadPart, dwOrigin, &ulNewPosition));

CLEANUP:
	if(plibNewPosition)
	   plibNewPosition->QuadPart = ulNewPosition;
	return hr;
}


HRESULT CStorageBuffer::SetSize(	ULARGE_INTEGER libNewSize)		//Specifies the new size of the stream object
{
	HRESULT hr = STG_E_MEDIUMFULL;
	void* pBuffer = m_pBuffer;

	//The value of the libNewSize parameter is not valid. 
	//Since streams cannot be greater than 2^32 bytes in the COM-provided 
	//implementation, the high DWORD of libNewSize must be 0. 
	//If it is nonzero, this parameter is not valid. 
	if(libNewSize.QuadPart > ULONG_MAX)
	{
		hr = STG_E_INVALIDFUNCTION;
		goto CLEANUP;
	}
	
	//Use a copy variable, incase allocations fail...
	SAFE_REALLOC(pBuffer, BYTE, libNewSize.QuadPart);

	m_pBuffer = pBuffer;
	m_cMaxSize = (ULONG)libNewSize.QuadPart;
	hr = S_OK;

CLEANUP:
	TRACE_METHOD(hr, L"\tIStream::SetSize(%d)", (ULONG)libNewSize.QuadPart);
	return hr;
}

HRESULT CStorageBuffer::CopyTo(	IStream * pIStream,				//Points to the destination stream
							ULARGE_INTEGER cb,				//Specifies the number of bytes to copy
							ULARGE_INTEGER * pcbRead,		//Pointer to the actual number of bytes read from the source
							ULARGE_INTEGER * pcbWritten)	//Pointer to the actual number of bytes written to the destination
{
	HRESULT hr = S_OK;

	//This is equivlent of doing Read from our stream then Write into the destination.
	//It also moves the seek position.  Bug this would require allocating a temp buffer...
	//Just access our stream directly...
	
	//Parameter checking
	if(pcbRead)
		pcbRead->QuadPart = 0;
	if(pcbWritten)
		pcbWritten->QuadPart = 0;

	//Actual code
	ULONG cBytesLeft = m_cMaxSize - m_iPos;
	ULONG cBytesRead = (ULONG)cb.QuadPart > cBytesLeft ? cBytesLeft : (ULONG)cb.QuadPart;
	ULONG cBytesWritten = 0;

	if(!pIStream)
	{
		hr = STG_E_INVALIDPOINTER;
		goto CLEANUP;
	}

	if(cb.QuadPart == 0)
	{
		hr = S_OK;
		goto CLEANUP;
	}

	//if no more bytes to retrive return 
	if(cBytesLeft == 0)
	{
		hr = S_FALSE; 
		goto CLEANUP;
	}

	//Copy to users buffer the number of bytes requested or remaining
	pIStream->Write((BYTE*)m_pBuffer + m_iPos, cBytesRead, &cBytesWritten);
	m_iPos += cBytesRead;

	if(pcbRead)
		pcbRead->QuadPart = cBytesRead;
	if(pcbWritten)
		pcbWritten->QuadPart = cBytesWritten;

	if(cb.QuadPart != cBytesRead)
		hr = S_FALSE; 

CLEANUP:
	TRACE_METHOD(hr, L"\tIStream::CopyTo(0x%p, %Id, &%d, &%d)", pIStream, cb, cBytesRead, cBytesWritten);
	return hr;
}

HRESULT CStorageBuffer::Commit(	DWORD grfCommitFlags)			//Specifies how changes are committed
{
	HRESULT hr = S_OK;
	TRACE_METHOD(hr, L"\tIStream::Commit()");
	return hr;
}

HRESULT CStorageBuffer::Revert(	)
{
	HRESULT hr = S_OK;
	TRACE_METHOD(hr, L"\tIStream::Revert()");
	return hr;
}

HRESULT CStorageBuffer::LockRegion(ULARGE_INTEGER libOffset,		//Specifies the byte offset for the beginning of the range
							ULARGE_INTEGER cb,				//Specifies the length of the range in bytes
							DWORD dwLockType)				//Specifies the restriction on accessing the specified range
{
	//Locking is optional
	HRESULT hr = STG_E_INVALIDFUNCTION;
	TRACE_METHOD(hr, L"\tIStream::LockRegion()");
	return hr;
}

HRESULT CStorageBuffer::UnlockRegion(  ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
								ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
								DWORD dwLockType)			//Specifies the access restriction previously placed on the range);
{
	//Locking is optional
	HRESULT hr = STG_E_INVALIDFUNCTION;
	TRACE_METHOD(hr, L"\tIStream::UnlockRegion()");
	return hr;
}

HRESULT CStorageBuffer::Stat(		STATSTG * pstatstg,				//Location for STATSTG structure
							DWORD grfStatFlag)				//Values taken from the STATFLAG enumeration
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER largeInteger = { m_cMaxSize };

	if(!pstatstg)
	{
		hr = STG_E_INVALIDPOINTER;
		goto CLEANUP;
	}
	
	//Initialize the struct
	memset(pstatstg, 0, sizeof(STATSTG));

	//Set the string...
	switch(grfStatFlag)
	{
		case STATFLAG_NONAME:
			pstatstg->pwcsName = NULL;
			break;

		case STATFLAG_DEFAULT:
			pstatstg->pwcsName = wcsDuplicate(L"OLE DB Test Storage Object...");
			break;
		
		default:
			hr = STG_E_INVALIDFLAG;
			goto CLEANUP;
	};

	//type
	pstatstg->type = STGTY_STREAM;
	pstatstg->grfMode = STGM_READ;
	pstatstg->grfLocksSupported = 0; 
	pstatstg->cbSize = largeInteger;
	
CLEANUP:
	TRACE_METHOD(hr, L"\tIStream::Stat(0x%p, 0x%08x)", pstatstg, grfStatFlag);
	return hr;
}

HRESULT CStorageBuffer::Clone(IStream** ppIStream)				//Points to location for pointer to the new stream object
{
	HRESULT hr = E_NOTIMPL;
	TRACE_METHOD(hr, L"\tILockBytes::Clone()");
	return hr;
}

	
HRESULT CStorageBuffer::ReadAt( 
							ULARGE_INTEGER ulOffset,
            /* [out] */		void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbRead)
{
	HRESULT hr = S_OK;
	ULONG cbRead = 0;

	//Delegate, reading from the current position
	XTEST(hr = ReadBuffer((ULONG)ulOffset.QuadPart, pv, cb, &cbRead));
	TRACE_METHOD(hr, L"\tILockBytes::ReadAt(%d, 0x%p, %d, &%d)", (ULONG)ulOffset.QuadPart, pv, cb, cbRead);

	if(pcbRead)
		*pcbRead = cbRead;
	return hr;
}
        
HRESULT CStorageBuffer::WriteAt( 
							ULARGE_INTEGER ulOffset,
            /* [in] */		const void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbWritten)
{
	HRESULT hr = S_OK;
	ULONG cbWritten = 0;

	//Delegate, reading from the current position
	XTEST(hr = WriteBuffer((ULONG)ulOffset.QuadPart, pv, cb, &cbWritten));
	TRACE_METHOD(hr, L"\tILockBytes::WriteAt(%d, 0x%p, %d, &%d)", (ULONG)ulOffset.QuadPart, pv, cb, cbWritten);

	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}

HRESULT CStorageBuffer::Flush()
{
	HRESULT hr = S_OK;
	TRACE_METHOD(hr, L"\tILockBytes::Flush()");
	return hr;
}


///////////////////////////////////////////////////////////////////////////////
// Class CFileStream
// 
// My implementation of stream interface on top of a file
///////////////////////////////////////////////////////////////////////////////
CFileStream::CFileStream()
{
	m_pFile = NULL;
	m_cRef	= 1;
}

CFileStream::~CFileStream()
{
	CloseFile();
}

HRESULT	CFileStream::OpenFile(WCHAR* pwszFile, WCHAR* pwszMode)
{
	if(IsUnicodeOS())
	{
		_wfopen_s(&m_pFile, pwszFile, pwszMode);
	}
	else
	{
		CHAR szFileName[MAX_NAME_LEN];
		ConvertToMBCS(pwszFile, szFileName, MAX_NAME_LEN);
		CHAR szMode[MAX_NAME_LEN];
		ConvertToMBCS(pwszMode, szMode, MAX_NAME_LEN);

		 fopen_s(&m_pFile, szFileName, szMode);
	}

	return m_pFile ? S_OK : STG_E_ACCESSDENIED;
}


HRESULT	CFileStream::CloseFile()
{
	if(m_pFile)
		fclose(m_pFile);

	return S_OK;
}


ULONG	CFileStream::AddRef()
{
	m_cRef++;
	return m_cRef;
}

ULONG	CFileStream::Release()
{
	ASSERT(m_cRef);
	m_cRef--;
	
	if(m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

HRESULT CFileStream::QueryInterface(REFIID riid, void** ppv)
{
	ASSERT(ppv);
	HRESULT hr = E_NOINTERFACE;

	*ppv = NULL;

	if (riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else if (riid == IID_ISequentialStream)
		*ppv = (ISequentialStream*)this;
	else if (riid == IID_IStream)
		*ppv = (IStream*)this;
	
	if(*ppv)
	{
		m_cRef++;
		hr = S_OK;
	}

	return hr;
}


STDMETHODIMP CFileStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	ULONG cbRead = 0;

	//Parameter checking
	if(!m_pFile)
		return STG_E_INVALIDHANDLE;
	if(!pv)
		return STG_E_INVALIDPOINTER;
	if(cb == 0)
		return S_OK;

	//Read from the file...
	cbRead = (ULONG)fread(pv, sizeof(BYTE), cb, m_pFile);

	//Results..
	if(pcbRead)
		*pcbRead = cbRead;
	return cbRead==cb ? S_OK : S_FALSE;
}


STDMETHODIMP CFileStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	ULONG cbWritten = 0;

	//Parameter checking
	if(!m_pFile)
		return STG_E_INVALIDHANDLE;
	if(!pv)
		return STG_E_INVALIDPOINTER;
	if(cb == 0)
		return S_OK;

	//Read from the file...
	cbWritten = (ULONG)fwrite(pv, sizeof(BYTE), cb, m_pFile);

	//Results..
	if(pcbWritten)
		*pcbWritten = cbWritten;
	return cbWritten==cb ? S_OK : S_FALSE;
}

//IStream interfaces
HRESULT CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
{
	//Parameter checking
	if(!m_pFile)
		return STG_E_INVALIDHANDLE;

	//Seek
	LONG lNewPosition = fseek(m_pFile, dlibMove.LowPart, dwOrigin);
	if (lNewPosition >= 0 && plibNewPosition)
		plibNewPosition->LowPart = lNewPosition;

	return (lNewPosition >= 0) ? S_OK : E_FAIL;		
}


HRESULT CFileStream::SetSize(	ULARGE_INTEGER libNewSize)		//Specifies the new size of the stream object
{
	return E_NOTIMPL;
}

HRESULT CFileStream::CopyTo(	IStream * pIStream,				//Points to the destination stream
							ULARGE_INTEGER cb,				//Specifies the number of bytes to copy
							ULARGE_INTEGER * pcbRead,		//Pointer to the actual number of bytes read from the source
							ULARGE_INTEGER * pcbWritten)	//Pointer to the actual number of bytes written to the destination
{
	return E_NOTIMPL;
}

HRESULT CFileStream::Commit(	DWORD grfCommitFlags)			//Specifies how changes are committed
{
	return E_NOTIMPL;
}

HRESULT CFileStream::Revert(	)
{
	return E_NOTIMPL;
}

HRESULT CFileStream::LockRegion(ULARGE_INTEGER libOffset,		//Specifies the byte offset for the beginning of the range
							ULARGE_INTEGER cb,				//Specifies the length of the range in bytes
							DWORD dwLockType)				//Specifies the restriction on accessing the specified range
{
	return E_NOTIMPL;
}

HRESULT CFileStream::UnlockRegion(  ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
								ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
								DWORD dwLockType)			//Specifies the access restriction previously placed on the range);
{
	return E_NOTIMPL;
}

HRESULT CFileStream::Stat(		STATSTG * pstatstg,				//Location for STATSTG structure
							DWORD grfStatFlag)				//Values taken from the STATFLAG enumeration
{
	return E_NOTIMPL;
}

HRESULT CFileStream::Clone(IStream** ppIStream)				//Points to location for pointer to the new stream object
{
	return E_NOTIMPL;
}

	



	
