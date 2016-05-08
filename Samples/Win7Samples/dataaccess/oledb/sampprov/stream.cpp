//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module STREAM.CPP | CStream object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"
								 

// Code ----------------------------------------------------------------------

// CStream::CStream ----------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CStream::CStream
    (
    LPUNKNOWN pUnkOuter         //@parm IN | Outer Unkown Pointer
    )	// invoke ctor for base class
	: CBaseObj( BOT_STREAM )
{
	//  Initialize simple member vars
    m_cRef          = 0L;
	m_pUnkOuter		= pUnkOuter ? pUnkOuter : this;

	m_pBuffer		= NULL;
	m_pParentObj	= NULL;
	m_iPos			= 0;
	m_cMaxSize		= 0;

    //  Initially, NULL all contained interfaces
	m_pIGetSourceRow	= NULL;
	m_pIStream			= NULL;
}


// CStream::~CStream ---------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CStream::~CStream()
{
    //  Free contained interfaces
	SAFE_DELETE( m_pIGetSourceRow );
	SAFE_DELETE( m_pIStream );

	if( m_pBuffer )
		delete [] m_pBuffer;

	if( m_pParentObj )	
		m_pParentObj->GetOuterUnknown()->Release();
}


// CStream::FInit ------------------------------------------------------------
//
// @mfunc Initialize the stream Object
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE  | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CStream::FInit
    (
	CRow *		pParentRow,		//@parm IN | pointer to parent Row object
	ROWBUFF *	pRowBuff		//@par, IN | pointer to row buffer
    )
{
	// Asserts
	assert(pParentRow);
	assert(pRowBuff);

	m_pParentObj = pParentRow;
	m_pParentObj->GetOuterUnknown()->AddRef();

    // Allocate contained interface objects
    m_pIStream			= new CImpIStream( this, m_pUnkOuter );
	m_pIGetSourceRow	= new CImpIGetSourceRow( this, m_pUnkOuter );

	m_cMaxSize = pParentRow->m_cbRowSize;
	m_pBuffer = new BYTE [ROUND_UP(m_cMaxSize , COLUMN_ALIGNVAL)];
	if( m_pBuffer )
	{
		memcpy(m_pBuffer, pRowBuff->cdData, m_cMaxSize );
	}

    // if all interfaces were created, return success
    return (BOOL) ( m_pIStream && m_pIGetSourceRow && m_pBuffer);
}


// CStream::QueryInterface ---------------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use
// QueryInterface to determine which interfaces the called object
// supports.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG | One or more arguments are invalid.
//
STDMETHODIMP CStream::QueryInterface
    (
    REFIID riid,
    LPVOID * ppv
    )
{	
    if( NULL == ppv )
		return E_INVALIDARG;

	//  Place NULL in *ppv in case of failure
    *ppv = NULL;

	//IUNKNOWN
	if( riid == IID_IUnknown )
		*ppv = this;											 	
	else if( riid == IID_IGetSourceRow )
		*ppv = m_pIGetSourceRow;
	else if( riid == IID_IStream )
		*ppv = m_pIStream;
	else if (riid == IID_ISequentialStream )
		*ppv = m_pIStream;

    //  If we're going to return an interface, AddRef it first
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
        return S_OK;
        }
    else
        return E_NOINTERFACE;
}


// CStream::AddRef -----------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( ULONG ) CStream::AddRef
     (
     void
     )
{
    return ++m_cRef;
}


// CStream::Release ----------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( ULONG ) CStream::Release
     (
     void
     )
{
    if (!--m_cRef)
        {
        delete this;
        return 0;
        }

    return m_cRef;
}


// CImpIGetSourceRow::GetSourceRow -------------------------------------------
//
// @mfunc Returns the interface pointer of the row object that created this
// stream
//
// @rdesc HRESULT
//      @flag S_OK                | Method Succeeded
//      @flag E_INVALIDARG        | ppRow was NULL
//		@flag DB_E_NOSOURCEOBJECT | No row object context for the stream
//
STDMETHODIMP CImpIGetSourceRow::GetSourceRow
	(
	REFIID		riid, 
	IUnknown ** ppRow
	)
{
	assert( m_pObj );
	assert( m_pObj->m_pParentObj );

	//Null output arg
	if( ppRow )
		*ppRow = NULL;

	//Check for E_INVALIDARG condition
	if( NULL == ppRow )
		return E_INVALIDARG;

	return m_pObj->m_pParentObj->GetOuterUnknown()->QueryInterface(riid, (LPVOID*)ppRow);
}


// CImpIStream::Read ---------------------------------------------------------
//
// @mfunc Reads from the stream
//
// @rdesc HRESULT
//      @flag S_OK                | Method Succeeded
//
HRESULT CImpIStream::Read
	(
	void *	pv,
	ULONG	cb,
	ULONG * pcbRead
	)
{
	assert( m_pObj );
	assert( m_pObj->m_pParentObj );

	ULONG cbRead = 0;
	ULONG cBytesLeft = 0;
	ULONG cBytesRead = 0;
	
	if( !pv )
		return STG_E_INVALIDPOINTER;

	if( cb == 0 )
		return S_OK;

	cBytesLeft = m_pObj->m_cMaxSize > m_pObj->m_iPos ? (ULONG)(m_pObj->m_cMaxSize - m_pObj->m_iPos) : 0;
	cBytesRead = cb > cBytesLeft ? cBytesLeft : cb;

	//if no more bytes to retrive return 
	if(cBytesLeft == 0)
		return S_FALSE; 

	//Copy to users buffer the number of bytes requested or remaining
	memcpy(pv, (void*)((BYTE*)m_pObj->m_pBuffer + m_pObj->m_iPos), cBytesRead);

	if(pcbRead)
		*pcbRead = cBytesRead;

	m_pObj->m_iPos += cBytesRead;

	if(cb != cBytesRead)
		return S_FALSE; 

	return S_OK;
}
      

///////////////////////////////////////////////////////////////////////////////
// CImpIStream::Write
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CImpIStream::Write
	(
	const void *	pv, 
	ULONG			cb,
	ULONG *			pcbWritten
	)
{
	// Sample Provider's stream are strictly read only
	return STG_E_ACCESSDENIED;
}
	

///////////////////////////////////////////////////////////////////////////////
// HRESULT CImpIStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CImpIStream::Seek
	(
	LARGE_INTEGER		dlibMove,
	DWORD				dwOrigin, 
	ULARGE_INTEGER *	plibNewPosition
	)
{
	DWORDLONG	ullOffset = 0;
	HRESULT		hr = S_OK;
	CStream *	pCStream = (CStream *) m_pObj;
	
	//Determine with respect to what the origin is...
	switch(dwOrigin)
	{
		case STREAM_SEEK_SET:
			ullOffset = dlibMove.QuadPart;
			break;

		case STREAM_SEEK_CUR:
			ullOffset = pCStream->m_iPos + dlibMove.QuadPart;
			break;

		case STREAM_SEEK_END:
			ullOffset = pCStream->m_cMaxSize + dlibMove.QuadPart;
			break;

		default:
			//The value of the dwOrigin parameter is not valid
			return STG_E_INVALIDFUNCTION;
	};

	//Make sure the new offset is within the range, for some reason ::Seek
	//allows going off the end of the stream...
	if( ullOffset < 0 || ullOffset > pCStream->m_cMaxSize )
		return STG_E_INVALIDFUNCTION; 

	//Reset the current buffer position
	pCStream->m_iPos = (ULONG)ullOffset;

	if(plibNewPosition)
		plibNewPosition->QuadPart = pCStream->m_iPos;

	return hr;
}


///////////////////////////////////////////////////////////////////////////////
// HRESULT CImpIStream::SetSize(	ULARGE_INTEGER libNewSize)		
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CImpIStream::SetSize
	(
	ULARGE_INTEGER libNewSize		//Specifies the new size of the stream object
	)
{
	return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////
// HRESULT CImpIStream::CopyTo
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CImpIStream::CopyTo
	(
	IStream	*			pIStream,	//Points to the destination stream
	ULARGE_INTEGER		cb,			//Specifies the number of bytes to copy
	ULARGE_INTEGER *	pcbRead,	//Pointer to the actual number of bytes read from the source
	ULARGE_INTEGER *	pcbWritten	//Pointer to the actual number of bytes written to the destination
	)
{
	return E_NOTIMPL;
}


// CImpIStream::Commit -------------------------------------------------------
//
// @mfunc Commit
//
// @rdesc HRESULT
//      @flag S_OK
//
HRESULT CImpIStream::Commit
	(
	DWORD grfCommitFlags			//Specifies how changes are committed
	)
{
	return S_OK;
}


// CImpIStream::Revert -------------------------------------------------------
//
// @mfunc Revert
//
// @rdesc HRESULT
//      @flag S_OK
//
HRESULT CImpIStream::Revert()
{
	return S_OK;
}


// CImpIStream::LockRegion ---------------------------------------------------
//
// @mfunc Unlock
//
// @rdesc HRESULT
//      @flag STG_E_INVALIDFUNCTION               |  Not implemented
//
HRESULT CImpIStream::LockRegion
	(
	ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
	ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
	DWORD dwLockType			//Specifies the restriction on accessing the specified range
	)				
{
	return STG_E_INVALIDFUNCTION;
}


// CImpIStream::UnlockRegion -------------------------------------------------
//
// @mfunc Unlock
//
// @rdesc HRESULT
//      @flag STG_E_INVALIDFUNCTION               |  Not implemented
//
HRESULT CImpIStream::UnlockRegion
	(
	ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
	ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
	DWORD dwLockType			//Specifies the access restriction previously placed on the range
	)
{
	//Locking is optional
	return STG_E_INVALIDFUNCTION;
}


// CImpIStream::Stat ---------------------------------------------------------
//
// @mfunc Stat
//
// @rdesc HRESULT
//      @flag S_OK                  | method succeeded
//		@flag STG_E_INVALIDPOINTER  | pstatstg was null
//		@flag STG_E_INVALIDFLAG		| invalid flag
//
HRESULT CImpIStream::Stat
	(
	STATSTG *	pstatstg,				//Location for STATSTG structure
	DWORD		grfStatFlag				//Values taken from the STATFLAG enumeration
	)
{
	if(!pstatstg)
		return STG_E_INVALIDPOINTER;
#if 0	
	//Initialize the struct
	memset(pstatstg, 0, sizeof(STATSTG));

	//Set the string...
	if(grfStatFlag & STATFLAG_NONAME)
		pstatstg->pwcsName = NULL;
	else if(grfStatFlag & STATFLAG_DEFAULT)
		pstatstg->pwcsName = wcsDuplicate(L"Sample Provider Storage Object...");
	else
		return STG_E_INVALIDFLAG;

	//type
	pstatstg->type = STGTY_STREAM;
	pstatstg->grfMode = STGM_READ;
	pstatstg->grfLocksSupported = 0; 

	ULARGE_INTEGER largeInteger = { m_pObj->m_cMaxSize };
	pstatstg->cbSize = largeInteger;
#endif

	return S_OK;
}


// CImpIStream::Clone --------------------------------------------------------
//
// @mfunc Clone the stream
//
// @rdesc HRESULT
//      @flag E_NOTIMPL                |  Not implemented
//
HRESULT CImpIStream::Clone(IStream ** ppIStream)			
{
	return E_NOTIMPL;
}

	

