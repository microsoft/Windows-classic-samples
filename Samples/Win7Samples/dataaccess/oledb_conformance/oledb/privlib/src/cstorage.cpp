//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTable Implementation Module | 	This module contains definition information
//						for CStorage class for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//
// @head3 CStorage Elements|
//
// @subindex CStorage
//
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "privstd.h"	 //Precompiled Header
#include "privcnst.h"
#include "miscfunc.h"
#include "CStorage.hpp"

///////////////////////////////////////////////////////////////////////////////
// Class CStorage
// 
///////////////////////////////////////////////////////////////////////////////
CStorage::CStorage()
{
	m_fS_OKonEOF	= FALSE;
	m_fQISeqStream	= TRUE;
	m_fQIStream		= TRUE;
	m_fQILockBytes	= TRUE;

	m_iPos			= 0;
	m_cRef			= 1;
	
	m_pBuffer		= NULL;
	m_cSize			= 0;

	//Debugging
	m_cbRead		= 0;
	m_cEndReached	= 0;
}


///////////////////////////////////////////////////////////////////////////////
// ~CStorage
// 
///////////////////////////////////////////////////////////////////////////////
CStorage::~CStorage()
{
	//Shouldn't have any references left
	//NOTE: We can't use GCOMPARE since this object may go away after the test is complete.
	//For example with rmeoting, RPC cleans this up after the test finishes, thus causing a crash
	//Better to just ASSERT, LTM will catch the ASSERT and log it as an error...
	ASSERT(m_cRef == 0);

	PROVIDER_FREE(m_pBuffer);
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::AddRef
// 
///////////////////////////////////////////////////////////////////////////////
ULONG	CStorage::AddRef()
{
	return ++m_cRef;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Release
// 
///////////////////////////////////////////////////////////////////////////////
ULONG	CStorage::Release()
{
	ASSERT(m_cRef);

	if(--m_cRef)
		return m_cRef;
	
	delete this;
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::QueryInterface
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::QueryInterface(REFIID riid, void** ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if (riid == IID_IUnknown)
		*ppv = (IUnknown*)(ISequentialStream*)this;
	else if (riid==IID_ISequentialStream && m_fQISeqStream)
		*ppv = (ISequentialStream*)this;
	else if (riid==IID_IStream && m_fQIStream)
		*ppv = (IStream*)this;
	else if (riid==IID_ILockBytes && m_fQILockBytes)
		*ppv = (ILockBytes*)this;
//	else if (riid == IID_IStorage)
//		*ppv = (IStorage*)this;
	
	if(*ppv)
	{
		((IUnknown*)*ppv)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Clear
// 
///////////////////////////////////////////////////////////////////////////////
BOOL CStorage::Clear()
{
	//Frees the buffer
	m_iPos		   = 0;
	m_cSize		   = 0;

	PROVIDER_FREE(m_pBuffer);
	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// CStorage::Compare
// 
///////////////////////////////////////////////////////////////////////////////
BOOL CStorage::Compare(ULONG cBytes, void* pBuffer)
{
	//First make sure there are the same amount of bytes...
	if(cBytes != m_cSize)
		return FALSE;

	//Check for null streams, (both must be null is one is null...)
	if(!pBuffer || !m_pBuffer)
		return pBuffer == m_pBuffer;

	//Now compare the byte streams...
	return memcmp(pBuffer, m_pBuffer, m_cSize)==0;
}


//64bit TODO - modify most of the functions below for number of bytes, offset, 
//etc. after the stream interfaces are converted to 64bit.



///////////////////////////////////////////////////////////////////////////////
// CStorage::Read
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Read(void *pv, ULONG cb, ULONG* pcbRead)
{
	ULARGE_INTEGER largeOffset = { m_iPos };
	ULONG cbRead = 0;

	//Delegate (to our ILockBytes::ReadAt implementation), 
	//reading from the current position
	HRESULT hr = ReadAt(largeOffset, pv, cb, &cbRead);
	if(SUCCEEDED(hr))
		m_iPos += cbRead;

	if(pcbRead)
		*pcbRead = cbRead;

	if(m_fS_OKonEOF && hr==S_FALSE)
		hr = S_OK;

	return hr;
}
      


///////////////////////////////////////////////////////////////////////////////
// CStorage::Write
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Write(const void *pv, ULONG cb, ULONG* pcbWritten)
{
	ULARGE_INTEGER largeOffset = { m_iPos };
	ULONG cbWritten = 0;

	//Delegate (to our ILockBytes::WriteAt implementation), 
	//writting to the current position
	HRESULT hr = WriteAt(largeOffset, pv, cb, &cbWritten);
	if(SUCCEEDED(hr))
		m_iPos += cbWritten;

	if(pcbWritten)
		*pcbWritten = cbWritten;

	return hr;
}
	

///////////////////////////////////////////////////////////////////////////////
// CStorage::Seek
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Seek(LONG lOffset)
{
	//NOTE: You can't use the bracket notation (ie: = { lOffset}) on the union, since the first
	//type of the union is a unsigned DWORD, whereas the QuadPart is signed.  And the compiler
	//first converts the signed to a large unsigned, which is not what we want...
	LARGE_INTEGER largeOffset;
	largeOffset.QuadPart = lOffset;

	//Delegate
	return Seek(largeOffset, STREAM_SEEK_SET, NULL);
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Seek
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
{
	DWORDLONG lOffset = 0;
	
	//Determine with respect to what the origin is...
	switch(dwOrigin)
	{
		case STREAM_SEEK_SET:
			lOffset = dlibMove.QuadPart;
			break;

		case STREAM_SEEK_CUR:
			lOffset = m_iPos + dlibMove.QuadPart;
			break;

		case STREAM_SEEK_END:
			lOffset = m_cSize + dlibMove.QuadPart;
			break;

		default:
			//The value of the dwOrigin parameter is not valid
			return STG_E_INVALIDFUNCTION;
	};

	//Make sure the new offset is within the range, for some reason ::Seek
	//allows going off the end of the stream...
	if(lOffset > m_cSize)
		return STG_E_INVALIDFUNCTION; //No good return code for this situaiton?

	//Reset the current buffer position
	m_iPos = (ULONG)lOffset;

	if(plibNewPosition)
	   plibNewPosition->QuadPart = m_iPos;
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::SetSize
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::SetSize(	ULARGE_INTEGER libNewSize)		//Specifies the new size of the stream object
{
	//The value of the libNewSize parameter is not valid. 
	//Since streams cannot be greater than 2^32 bytes in the COM-provided 
	//implementation, the high DWORD of libNewSize must be 0. 
	//If it is nonzero, this parameter is not valid. 
	if(libNewSize.QuadPart > ULONG_MAX)
		STG_E_INVALIDFUNCTION;
	
	//Use a copy variable, incase allocations fail...
	void* pBuffer = PROVIDER_REALLOC(m_pBuffer, (ULONG)libNewSize.QuadPart);
	if(!pBuffer)
		return STG_E_MEDIUMFULL;

	m_pBuffer = pBuffer;
	m_cSize = (ULONG)libNewSize.QuadPart;
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::CopyTo
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::CopyTo(	IStream * pIStream,				//Points to the destination stream
							ULARGE_INTEGER cb,				//Specifies the number of bytes to copy
							ULARGE_INTEGER * pcbRead,		//Pointer to the actual number of bytes read from the source
							ULARGE_INTEGER * pcbWritten)	//Pointer to the actual number of bytes written to the destination
{
	//This is equivlent of doing Read from our stream then Write into the destination.
	//It also moves the seek position.  Bug this would require allocating a temp buffer...
	//Just access our stream directly...
	
	//Parameter checking
	if(pcbRead)
		pcbRead->QuadPart = 0;
	if(pcbWritten)
		pcbWritten->QuadPart = 0;

	if(!pIStream)
		return STG_E_INVALIDPOINTER;

	if(cb.QuadPart == 0)
		return S_OK;

	//Actual code
	ULONG cBytesLeft = m_cSize - m_iPos;
	ULONG cBytesRead = (ULONG)cb.QuadPart > cBytesLeft ? cBytesLeft : (ULONG)cb.QuadPart;
	ULONG cBytesWritten = 0;

	//if no more bytes to retrive return 
	if(cBytesLeft == 0)
		return S_FALSE; 

	//Copy to users buffer the number of bytes requested or remaining
	pIStream->Write((BYTE*)m_pBuffer + m_iPos, cBytesRead, &cBytesWritten);
	m_iPos += cBytesRead;

	if(pcbRead)
		pcbRead->QuadPart = cBytesRead;
	if(pcbWritten)
		pcbWritten->QuadPart = cBytesWritten;

	if(cb.QuadPart != cBytesRead)
		return S_FALSE; 

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Commit
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Commit(	DWORD grfCommitFlags)			//Specifies how changes are committed
{
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// CStorage::Revert
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Revert(	)
{
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::LockRegion
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::LockRegion(ULARGE_INTEGER libOffset,		//Specifies the byte offset for the beginning of the range
							ULARGE_INTEGER cb,				//Specifies the length of the range in bytes
							DWORD dwLockType)				//Specifies the restriction on accessing the specified range
{
	//Locking is optional
	return STG_E_INVALIDFUNCTION;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::UnlockRegion
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::UnlockRegion(  ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
								ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
								DWORD dwLockType)			//Specifies the access restriction previously placed on the range);
{
	//Locking is optional
	return STG_E_INVALIDFUNCTION;
}



///////////////////////////////////////////////////////////////////////////////
// CStorage::Stat
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Stat(		STATSTG * pstatstg,				//Location for STATSTG structure
							DWORD grfStatFlag)				//Values taken from the STATFLAG enumeration
{
	if(!pstatstg)
		return STG_E_INVALIDPOINTER;
	
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
			return STG_E_INVALIDFLAG;
	};

	//type
	pstatstg->type = STGTY_STREAM;
	pstatstg->grfMode = STGM_READ;
	pstatstg->grfLocksSupported = 0; 

	ULARGE_INTEGER largeInteger = { m_cSize };
	pstatstg->cbSize = largeInteger;
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Clone
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Clone(IStream** ppIStream)				//Points to location for pointer to the new stream object
{
	return E_NOTIMPL;
}

	
///////////////////////////////////////////////////////////////////////////////
// CStorage::ReadAt
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::ReadAt( 
							ULARGE_INTEGER ulOffset,
            /* [out] */		void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbRead)
{
	//Parameter checking
	if(pcbRead)
		*pcbRead = 0;

	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(cb == 0)
		return S_OK;

	//Actual code
	ULONG cbLeft = m_cSize > (ULONG)ulOffset.QuadPart ? m_cSize - (ULONG)ulOffset.QuadPart : 0;
	ULONG cbRead = cb > cbLeft ? cbLeft : cb;

	//Copy to users buffer the number of bytes requested or remaining
	memcpy(pv, (void*)((BYTE*)m_pBuffer + (ULONG)ulOffset.QuadPart), cbRead);
	
	if(pcbRead)
		*pcbRead = cbRead;
	m_cbRead += cbRead;

	if(cb != cbRead)
	{
		m_cEndReached++;
		return S_FALSE; 
	}
	return S_OK;
}
        


///////////////////////////////////////////////////////////////////////////////
// CStorage::WriteAt
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::WriteAt( 
							ULARGE_INTEGER ulOffset,
            /* [in] */		const void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbWritten)
{
	//Parameter checking
	if(!pv)
		return STG_E_INVALIDPOINTER;

	if(pcbWritten)
		*pcbWritten = 0;

	if(cb == 0)
		return S_OK;

	//May need to Enlarge the current buffer
	if((ULONG)ulOffset.QuadPart + cb >= m_cSize)
	{
		m_cSize += cb;

		//Need to append to the end of the stream
		ULARGE_INTEGER largeSize = { m_cSize };
		if(FAILED(SetSize(largeSize)))
			return E_OUTOFMEMORY;
	}
	
	//Copy to the buffer
	memcpy((void*)((BYTE*)m_pBuffer + (ULONG)ulOffset.QuadPart), pv, cb);

	if(pcbWritten)
		*pcbWritten = cb;

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CStorage::Flush
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CStorage::Flush()
{
	return S_OK;
}



// {CB21F4D6-878D-11d1-9528-00C04FB66A50}
extern const GUID IID_IAggregate = 
{ 0xcb21f4d6, 0x878d, 0x11d1, { 0x95, 0x28, 0x0, 0xc0, 0x4f, 0xb6, 0x6a, 0x50 } };


///////////////////////////////////////////////////////////////////////////////
// CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
CAggregate::CAggregate(IUnknown* pIUnkParent)
{
	m_cRef = 1;
	m_pIUnkInner	= NULL;
	
	m_ulRefParent	= 0;
	m_pIUnkCheckRefCount		= pIUnkParent;
	if(m_pIUnkCheckRefCount)
		m_ulRefParent = m_pIUnkCheckRefCount->AddRef()-1;
}


///////////////////////////////////////////////////////////////////////////////
// ~CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
CAggregate::~CAggregate() 
{
	//COM Aggregation rule #6
	//To free an inner pointer (other than IUnknown), the outer object calls its 
	//own outer unknown's AddRef followed by Release on the inner object's pointer
	//Currently we don't have this case, since we only have IUnknown
	//AddRef()
	//SAFE_RELEASE(m_pNonIUnknownInner);

	//Inner object free
	//ReleaseInner(); Last chance to free this, since the object
	//is getting destroyed. So, release anyway.
	SAFE_RELEASE(m_pIUnkInner);

	//Verify RefCount on outer object, is the same as when we started
	VerifyRefCounts(GetRefCount(), 1);
    
	//Verify the main reference count of calling object is the same as when we started...
	if(m_pIUnkCheckRefCount)
		VerifyRefCounts(m_pIUnkCheckRefCount->Release(), m_ulRefParent);
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::GetRefCount
//
/////////////////////////////////////////////////////////////////////////////
ULONG CAggregate::GetRefCount() 
{
	return m_cRef;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::SetUnkInner
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CAggregate::SetUnkInner(IUnknown* pIUnkInner)
{
	if(pIUnkInner == NULL)
		return E_FAIL;

	SAFE_RELEASE(m_pIUnkInner);
	return pIUnkInner->QueryInterface(IID_IUnknown, (void**)&m_pIUnkInner);
}

	
/////////////////////////////////////////////////////////////////////////////
//	CAggregate::ReleaseInner
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CAggregate::ReleaseInner()
{
	//Only release the inner if the RefCount of the outer has gone to its
	//orginal refcount
	if(m_cRef <= 1)
	{
		SAFE_RELEASE(m_pIUnkInner);
		return S_OK;
	}
	
	return S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::AddRef
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CAggregate::AddRef(void)
{
	return ++m_cRef;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::Release
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CAggregate::Release(void)
{
	ASSERT(m_cRef);
	if(--m_cRef)
		return m_cRef;

	//COM Aggregation rule #5
	//The outer object must protect its implementation of Release from 
	//reentrantcy with an artifical reference count arround its destruction code
	m_cRef++;

	//Delete this object
	delete this;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::QueryInterface
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAggregate::QueryInterface(REFIID riid, LPVOID *ppv)
{
	HRESULT hr = S_OK;
	
	//TEST_ NULL
	if(ppv == NULL)
		return E_INVALIDARG;

	//Null output params
	*ppv = NULL;

	//Support IID_IUnknown
	if(riid == IID_IUnknown)
	{
		*ppv = (IUnknown*)this;
		SAFE_ADDREF((IUnknown*)*ppv);
	}
	else if(riid == IID_IAggregate)
	{
		*ppv = (IUnknown*)this;
		SAFE_ADDREF((IUnknown*)*ppv);
	}
	else if(m_pIUnkInner)
	{
		//Delegate to the Inner Object
		//This is not "circular" since this interface is the IID_IUnknown
		//interface only, which has its own non-delegating QI...
		hr = m_pIUnkInner->QueryInterface(riid, ppv);
	}
	else
	{
		return E_NOINTERFACE;
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::VerifyAggregationQI
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAggregate::VerifyAggregationQI(HRESULT hrReturned, REFIID riidInner, IUnknown** ppIDelegate, BOOL fInitialized)
{
	ASSERT(riidInner != IID_IUnknown);

    IUnknown* pIUnkInner	= NULL;
    IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIDelegate	= NULL;
	IUnknown* pIAggregate	= NULL;
	HRESULT hr = S_OK;
	BOOL bReturn = FALSE;

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	TESTC(VerifyRefCounts(GetRefCount(), 1));

	//Verify results
	//Some Providers may not support Aggregation
	//NOTE:  We put the error codes first the list, since if it miscompares the privlib will
	//mark other errors returned instead of DB_E_NOAGGREGATION as warnings...
	TEST3C_(hrReturned, DB_E_NOAGGREGATION, CLASS_E_NOAGGREGATION, S_OK);
	if(FAILED(hrReturned))
	{
		TEST2C_(hrReturned, DB_E_NOAGGREGATION, CLASS_E_NOAGGREGATION);
		TWARNING("Aggregation not support by provider?");
		TESTC(m_pIUnkInner == NULL);
		QTESTC(FALSE);
	}

	//Before calling This function the caller should have called CAggregate::SetUnkInner
	//to setup the containing object...
	TESTC(m_pIUnkInner != NULL);

	//Obtain the Delegating interface
	TESTC_(hr = m_pIUnkInner->QueryInterface(riidInner, (void**)&pIDelegate),S_OK);
	
	//Make sure the non-Delegating can't see the outer object interfaces
	//According to 2.0 RTM spec, QI before Initialized for any other interface
	//than those metioned, can return either E_UNEXPECTED or E_NOINTERFACE
	if(fInitialized)
	{	
		TESTC_(hr = m_pIUnkInner->QueryInterface(IID_IAggregate, (void**)&pIUnkInner), E_NOINTERFACE);
	}
	else
	{	
		TEST2C_(hr = m_pIUnkInner->QueryInterface(IID_IAggregate, (void**)&pIUnkInner), E_NOINTERFACE, E_UNEXPECTED);
	}

	//Verify we are hooked up...
	TESTC_(hr = pIDelegate->QueryInterface(IID_IAggregate, (void**)&pIAggregate),S_OK);

	//Now just make sure that the Delegating QI for IUnknown produces the outer
	TESTC_(hr = pIAggregate->QueryInterface(IID_IUnknown, (void**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIUnkOuter));
	TESTC(m_pIUnkInner != pIUnkOuter);

	//Now make sure the the non-Delegating QI for IUnknown produces inner
	TESTC_(hr = m_pIUnkInner->QueryInterface(IID_IUnknown, (void**)&pIUnkInner),S_OK);
	TESTC(VerifyEqualInterface(m_pIUnkInner, pIUnkInner));
	TESTC(m_pIUnkInner == pIUnkInner);
	bReturn = TRUE;

CLEANUP:
	if(ppIDelegate)
		*ppIDelegate = pIDelegate;
	else
		SAFE_RELEASE(pIDelegate);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIUnkInner);
	SAFE_RELEASE(pIAggregate);
	return bReturn;
}


///////////////////////////////////////////////////////////////////////////////
// Class CDispatch
// 
///////////////////////////////////////////////////////////////////////////////
CDispatch::CDispatch()
{
	m_cRef		= 1;
}

CDispatch::~CDispatch()
{
	//Shouldn't have any references left
	COMPARE(m_cRef, 0);
}

ULONG	CDispatch::AddRef()
{
	return ++m_cRef;
}

ULONG	CDispatch::Release()
{
	ASSERT(m_cRef);

	if(--m_cRef)
		return m_cRef;
	
	delete this;
	return 0;
}

HRESULT CDispatch::QueryInterface(REFIID riid, void** ppv)
{
	ASSERT(ppv);
	
	*ppv = NULL;

	if (riid == IID_IUnknown ||
		riid == IID_IDispatch) 
		*ppv = (IUnknown*)this;
	
	if(*ppv)
	{
		((IUnknown*)*ppv)->AddRef();
		return S_OK;
	}

	// You can uncomment this line to see what riid was requested.
//	odtLog << L"Invalid riid:" << GetIIDString(riid) << L"\n";

	return E_NOINTERFACE;
}

HRESULT CDispatch::GetTypeInfoCount(UINT *pctinfo)
{
	if (pctinfo)
		*pctinfo = 0;

	return S_OK;
}

HRESULT CDispatch::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	if (ppTInfo)
		*ppTInfo = NULL;

	return E_FAIL;
}

HRESULT CDispatch::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, 
	DISPID *rgDispId)
{
	if (rgDispId)
		*rgDispId = NULL;

	return E_FAIL;
}

HRESULT CDispatch::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
	VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	return E_FAIL;
}
