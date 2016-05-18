//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module BINDER.CPP | CBinder object implementation and contained
//	interfaces
//
//

// Includes ------------------------------------------------------------------
#include "headers.h"

// Globals -------------------------------------------------------------------
WCHAR g_wszDataSourceKeyword[]	= L"DataSource";
WCHAR g_wszFileKeyword[]		= L"File";   
WCHAR g_wszRowKeyword[]			= L"Row";


// Code ----------------------------------------------------------------------

// CBinder::CBinder ----------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CBinder::CBinder
    (
	LPUNKNOWN	pUnkOuter	 //@parm IN | Outer Unkown Pointer
    ) // invoke ctor for base class
	: CBaseObj( BOT_BINDER )
{
	m_cRef = 0;
	m_pUnkOuter = pUnkOuter ? pUnkOuter : this;

	m_pUtilProp			= NULL;

	// Contained interfaces
	m_pIBindResource		= NULL;
	m_pIDBBinderProperties	= NULL;
	m_pICreateRow			= NULL;
}


// CBinder::~CBinder ---------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CBinder::~CBinder()
{
	SAFE_DELETE( m_pUtilProp );	
	SAFE_DELETE( m_pIBindResource );
	SAFE_DELETE( m_pIDBBinderProperties );
	SAFE_DELETE( m_pICreateRow );
}


// CBinder::FInit ------------------------------------------------------------
//
// @mfunc Initialize the Binder Object
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CBinder::FInit()
{
	assert(m_pUnkOuter);

	m_pUtilProp			= new CUtilProp();

	// contained interfaces
	m_pIBindResource		= new CImpIBindResource( this, m_pUnkOuter );
	m_pIDBBinderProperties	= new CImpIDBBinderProperties( this, m_pUnkOuter );
	m_pICreateRow			= new CImpICreateRow( this, m_pUnkOuter );

	return (m_pUtilProp && m_pIBindResource && m_pIDBBinderProperties &&
			m_pICreateRow);
}


// CBinder::QueryInterface ---------------------------------------------------
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
STDMETHODIMP CBinder::QueryInterface
    (
    REFIID riid,
    LPVOID * ppv
    )
{    
    if (NULL == ppv)
        return ResultFromScode( E_INVALIDARG );

	//  Place NULL in *ppv in case of failure
    *ppv = NULL;
	
	//IUNKNOWN
	if (riid == IID_IUnknown)
		*ppv = this;											 
	else if(riid == IID_IBindResource)
		*ppv = (LPVOID) m_pIBindResource;
	else if(riid == IID_IDBProperties)
		*ppv = (LPVOID) m_pIDBBinderProperties;
	else if(riid == IID_IDBBinderProperties)
		*ppv = (LPVOID) m_pIDBBinderProperties;
	else if(riid == IID_ICreateRow)
		*ppv = (LPVOID) m_pICreateRow;

    //  If we're going to return an interface, AddRef it first
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
        return ResultFromScode( S_OK );
        }
    else
        return ResultFromScode( E_NOINTERFACE );
}


// CBinder::AddRef -----------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CBinder::AddRef
     (
     void
     )
{
    return ++m_cRef;
}


// CBinder::Release ----------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG | One or more arguments are invalid.
//
STDMETHODIMP_( ULONG ) CBinder::Release
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


// CImpIBindResource::BindDSO ------------------------------------------------
//
// @mfunc Returns an initialized datasource object
//
// @rdesc 
//      @flag S_OK                   | Method succeeded
//      @flag E_OUTOFMEMORY          | Out of memory
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindDSO
	(
	IUnknown *	pUnkOuter,		// [in] controllig IUnknown
	REFIID		riid,			// [in] interface to be requested
	BOOL		fWaitForInit,   // [in] flag indicating if DSO should be initialized
	WCHAR *		pwszDataSource,	// [in] datasource path 
	IUnknown **	ppUnk			// [out] interface retrieved on DSO
	)
{
	HRESULT				hr				= E_FAIL;
	IGetDataSource *	pIGetDataSource	= NULL;
	IDBProperties *		pIDBProperties	= NULL;
	IDBInitialize *		pIDBInitialize	= NULL;
	CDataSource *		pDataSource		= NULL;
	ULONG				cPropSets		= 0;
	DBPROPSET *			rgPropSets		= NULL;

	if( BOT_SESSION == m_pObj->GetBaseObjectType() )
	{
		// In this case the datasource is alreay initialized
		if( fWaitForInit )
			return E_INVALIDARG;

		// Obtain an instance of the current session Session::IBindResource
		// pUnkOuter is always ignored when the DSO already exists
		TESTC(hr = ((CDBSession*)m_pObj)->QueryInterface(IID_IGetDataSource, (void**)&pIGetDataSource));
		TESTC(hr = pIGetDataSource->GetDataSource(riid, ppUnk));
	}
	else
	{
		// create DSO (aggregating if necessary)
		pDataSource = new CDataSource(pUnkOuter);
		if( pDataSource == NULL || !pDataSource->FInit() )
		{
			hr = E_OUTOFMEMORY;
			SAFE_DELETE(pDataSource);
			goto CLEANUP;
		}
	
		// initialize the DSO with the cached properties
		TESTC(hr = pDataSource->QueryInterface(IID_IDBProperties, (void**)&pIDBProperties));
		TESTC(hr = pDataSource->QueryInterface(IID_IDBInitialize, (void**)&pIDBInitialize));

		// Get the cached initialization properties
		// and set them on the requested DataSource
		TESTC(hr = ((CBinder*)m_pObj)->m_pUtilProp->GetProperties(PROPSET_DSO, 0,
												NULL, &cPropSets, &rgPropSets));
		TESTC(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets));
	
		// Set the DBPROP_INIT_DATASOURCE property if specified in the URL
		if( pwszDataSource )
		{
			DBPROPSET	PropSet;
			DBPROP		PropDataSource;

			PropSet.guidPropertySet	= DBPROPSET_DBINIT;
			PropSet.rgProperties	= &PropDataSource;
			PropSet.cProperties		= 1;

			PropDataSource.dwPropertyID		= DBPROP_INIT_DATASOURCE;
			PropDataSource.dwOptions		= DBPROPOPTIONS_REQUIRED;
			PropDataSource.dwStatus			= 0;
			V_VT(&PropDataSource.vValue)	= VT_BSTR;
			V_BSTR(&PropDataSource.vValue)	= SysAllocString(pwszDataSource);

			if( FAILED(hr = pIDBProperties->SetProperties(1, &PropSet)) )
			{
				VariantClear(&PropDataSource.vValue);
				goto CLEANUP;
			}
			VariantClear(&PropDataSource.vValue);
		}

		if( !fWaitForInit )
		{
			// Initialize 
			TESTC(hr = pIDBInitialize->Initialize());
		}

		// Return the requested interface
		TESTC(hr = pDataSource->QueryInterface(riid, (void**)ppUnk));
	}

	hr = S_OK;

CLEANUP:

	// release properties 
	FreeProperties(&cPropSets, &rgPropSets);

	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);

	return hr;
}


// CImpIBindResource::BindSession --------------------------------------------
//
// @mfunc Returns a session object
//
// @rdesc 
//      @flag S_OK                   | Method succeeded
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindSession
	(
	IUnknown *	pUnkOuter,		// [in] controllig IUnknown
	REFIID		riid,			// [in] interface to be requested
	WCHAR *		pwszDataSource,	// [in] datasource path 
	IUnknown **	ppUnk			// [out] session interface
	)
{
	HRESULT				hr					= E_FAIL;
	IDBCreateSession *	pIDBCreateSession	= NULL;

	if( BOT_SESSION == m_pObj->GetBaseObjectType() )
	{
		// Obtain an instance of the current session Session::IBindResource
		// pUnkOuter is always ignored when the DSO already exists
		TESTC(hr = ((CDBSession*)m_pObj)->QueryInterface(riid, (void**)ppUnk));
	}
	else
	{
		// bind a DSO first
		TESTC(hr = BindDSO(NULL, IID_IDBCreateSession, FALSE, pwszDataSource, 
							(IUnknown**)&pIDBCreateSession));

		// Create Session.
		TESTC(hr = pIDBCreateSession->CreateSession(pUnkOuter, riid, (IUnknown**)ppUnk));
	}

	hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pIDBCreateSession);

	return hr;
}


// CImpIBindResource::BindSession --------------------------------------------
//
// @mfunc Returns an implicit session object
//
// @rdesc 
//      @flag S_OK                   | Method succeeded
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindSession
	(
	DBIMPLICITSESSION *	pImplSession,	// [in|out] implicit session pointer
	WCHAR *				pwszDataSource  // [in] DataSource path name
	)
{
	HRESULT				hr					= E_FAIL;
	IDBCreateSession *	pIDBCreateSession	= NULL;

	// check arguments
	if( !pImplSession || !pImplSession->piid )
	{
		hr = E_INVALIDARG;
		goto CLEANUP;
	}
	
	if( pImplSession->pUnkOuter && IID_IUnknown != *(pImplSession->piid) )
	{
		hr = DB_E_NOAGGREGATION;
		goto CLEANUP;
	}

	// This method should not be called when on a session's
	// implementation of IBindResource.
	// pImplSession should just be ignored in this case
	if( BOT_SESSION == m_pObj->GetBaseObjectType() )
		return E_FAIL;

	// bind a DSO first
	TESTC(hr = BindDSO(NULL, IID_IDBCreateSession, FALSE, pwszDataSource, 
						(IUnknown**)&pIDBCreateSession));

	//Create Session.
	pImplSession->pSession = NULL;	
	TESTC(hr = pIDBCreateSession->CreateSession(pImplSession->pUnkOuter, 
		*(pImplSession->piid), (IUnknown**)&pImplSession->pSession));
				
	hr = S_OK;

CLEANUP:

	if (FAILED(hr))
		pImplSession->pSession = NULL;

	SAFE_RELEASE(pIDBCreateSession);

	return hr;
}


// CImpIBindResource::BindRowset ---------------------------------------------
//
// @mfunc Retrieves a rowset object
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                   | Method Succeeded
//		@flag DB_E_NOTFOUND			 | Object does not exist
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindRowset
	(
	IUnknown *			pUnkOuter,		// [in] controllig IUnknown
	REFIID				riid,			// [in] interface to be requested
	DBIMPLICITSESSION * pImplSession,	// [in|out] implicit session pointer
	WCHAR *				pwszDataSource, // [in] DataSource path name
	WCHAR *				pwszFile,		// [in] URL name	
	IUnknown **			ppUnk			// [out] session interface
	)
{
	HRESULT			hr				= E_FAIL;
	IOpenRowset *	pIOpenRowset	= NULL;
	DBID			TableID;

	if( BOT_SESSION != m_pObj->GetBaseObjectType() && pImplSession )
	{
		TESTC(hr = BindSession(pImplSession, pwszDataSource));
		TESTC(hr = pImplSession->pSession->QueryInterface(IID_IOpenRowset, (void**)&pIOpenRowset));
	}
	else
		TESTC(hr = BindSession(NULL, IID_IOpenRowset, pwszDataSource, (IUnknown**)&pIOpenRowset));

	TableID.eKind			= DBKIND_NAME;
	TableID.uName.pwszName	= pwszFile;

	TESTC(hr = pIOpenRowset->OpenRowset(pUnkOuter, &TableID, NULL, riid, 
		0, NULL, ppUnk));

	hr = S_OK;

CLEANUP:

	if( DB_E_NOTABLE == hr )
		hr = DB_E_NOTFOUND;

	SAFE_RELEASE(pIOpenRowset);

	return hr;
} 


// CImpIBindResource::BindRow ------------------------------------------------
//
// @mfunc Retrieves a row object
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                   | Method Succeeded
//		@flag DB_E_NOTFOUND			 | Object does not exist
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindRow
	(
	IUnknown *			pUnkOuter,		// [in] controllig IUnknown
	REFIID				riid,			// [in] interface to be requested
	DBIMPLICITSESSION * pImplSession,	// [in|out] implicit session pointer
	WCHAR *				pwszDataSource, // [in] DataSource path name
	WCHAR *				pwszFile,		// [in] URL name	
	ULONG				ulRowNum,		// [in] Row number to retrieve
	IUnknown **			ppUnk			// [out] session interface
	)
{
	HRESULT			hr	= E_FAIL;
	CDBSession *	pCSession = NULL;
	CFileIO *		pFileio = NULL;
	CRow *			pCRow = NULL;
	
	if( BOT_SESSION != m_pObj->GetBaseObjectType() && pImplSession )
	{
		TESTC(hr = BindSession(pImplSession, pwszDataSource));
		TESTC(hr = pImplSession->pSession->QueryInterface(IID_IUnknown, (void**)&pCSession));
	}
	else
		TESTC(hr = BindSession(NULL, IID_IUnknown, pwszDataSource, (IUnknown**)&pCSession));

	//Try to open the file...
	TESTC(hr = pCSession->m_pCDataSource->OpenFile(pwszFile, &pFileio));

	// If no row number specified, just fetch the first 1
	if( ulRowNum == 0 )
		ulRowNum = 1;

	if( pFileio->GetRowCnt() < ulRowNum )
	{
		hr = DB_E_NOTFOUND;
		goto CLEANUP;
	}

	pCRow = new CRow(pUnkOuter);
	if( !pCRow || !pCRow->FInit(pCSession, pFileio, ulRowNum) )
	{
		hr = E_OUTOFMEMORY;
		goto CLEANUP;
	}
	
	TESTC(hr = pCRow->QueryInterface(riid, (LPVOID*)ppUnk));

CLEANUP:

	if( FAILED(hr) )
	{
		if( DB_E_NOTABLE == hr )
			hr = DB_E_NOTFOUND;

		SAFE_DELETE(pCRow);
	}

	SAFE_RELEASE(pCSession);

	return hr;
} 


// CImpIBindResource::BindStream ---------------------------------------------
//
// @mfunc Retrieves a stream object
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                   | Method Succeeded
//		@flag DB_E_NOTFOUND			 | Object does not exist
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
HRESULT CImpIBindResource::BindStream
	(
	IUnknown *			pUnkOuter,		// [in] controllig IUnknown
	REFIID				riid,			// [in] interface to be requested
	DBIMPLICITSESSION * pImplSession,	// [in|out] implicit session pointer
	WCHAR *				pwszDataSource, // [in] DataSource path name
	WCHAR *				pwszFile,		// [in] URL name	
	ULONG				ulRowNum,		// [in] Row number to retrieve
	IUnknown **			ppUnk			// [out] session interface
	)
{
	HRESULT			hr	= E_FAIL;
	CRow *			pCRow = NULL;
	CStream *		pCStream = NULL;
	
	TESTC(hr = BindRow(NULL, IID_IUnknown, pImplSession, pwszDataSource,
					pwszFile, ulRowNum, (IUnknown **)&pCRow));

	pCStream = new CStream(pUnkOuter);
	if( pCStream && pCStream->FInit(pCRow, pCRow->GetRowBuff()) )
		hr = pCStream->QueryInterface(riid, (LPVOID *)ppUnk);
	else
		hr = E_OUTOFMEMORY;

CLEANUP:
	if( FAILED(hr) )
	{
		if( DB_E_NOTABLE == hr )
			hr = DB_E_NOTFOUND;

		SAFE_DELETE(pCStream);
	}

	SAFE_RELEASE(pCRow);

	return hr;
} 


// CImpIBindResource::ValidateBindArgs ---------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK               | Method succeeded
//      @flag DB_E_NOAGGREGATION | valid pUnkOuter but riid != IID_IUnknown
//      @flag E_INVALIDARG	     | One or more arguments are invalid.
//		@flag E_NOINTERFACE	     | riid == IID_NULL
//
HRESULT CImpIBindResource::ValidateBindArgs
	(
	IUnknown *			pUnkOuter,
	LPCOLESTR			pwszURL,
	DBBINDURLFLAG		dwBindFlags,
	REFGUID				rguid,
	REFIID				riid,
	DBIMPLICITSESSION * pImplSession,
	IAuthenticate *		pAuthenticate,
	DWORD *				pdwBindStatus,
	IUnknown **			ppUnk
	)
{
    // Check in-params and NULL out-params in case of error
	*pdwBindStatus = DBBINDURLSTATUS_S_OK;

	if ( ppUnk )
		*ppUnk = NULL;

	if( !ppUnk || !pwszURL )
		return E_INVALIDARG;
	
	if( pUnkOuter && riid != IID_IUnknown )
		return DB_E_NOAGGREGATION;

	if( riid == IID_NULL )
		return E_NOINTERFACE;

	if( dwBindFlags == 0 )
		return E_INVALIDARG;

	if( pImplSession )
	{
		if( pImplSession->piid == NULL )
		{
			return E_INVALIDARG;
		}

		if( pImplSession->pUnkOuter && (*(pImplSession->piid) != IID_IUnknown) ) 
		{
			return DB_E_NOAGGREGATION;
		}
	}

	// Special cases - Detect invalid or unsupported flags 
	
	// WAITFORINIT is only valid for DBGUID_DSO
	if( rguid != DBGUID_DSO && (dwBindFlags & DBBINDURLFLAG_WAITFORINIT) )
		return E_INVALIDARG;

	// OUTPUT flag is ignored when binding to DBGUID_ROWSET
	if( rguid != DBGUID_ROWSET && (dwBindFlags & DBBINDURLFLAG_OUTPUT) )
		return E_INVALIDARG;

	// WAITFORINIT and OUTPUT have been handled
	// Sample Provider has no notion of a hierarchy, so the recursive flag is always ignored
	dwBindFlags &= ~(DBBINDURLFLAG_WAITFORINIT | DBBINDURLFLAG_OUTPUT | DBBINDURLFLAG_RECURSIVE);

	if( dwBindFlags & 
		~(DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_SHARE_DENY_NONE | DBBINDURLFLAG_SHARE_EXCLUSIVE) )
	{
		if( dwBindFlags & DBBINDURLFLAG_ASYNCHRONOUS )
		{
			return DB_E_ASYNCNOTSUPPORTED;
		}	
		else
			return E_INVALIDARG;
	}	
	
	// Allow the bind to succeed, but warn user
	if( dwBindFlags & DBBINDURLFLAG_SHARE_EXCLUSIVE )
	{
		*pdwBindStatus = DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED;
	}	

	if( rguid == DBGUID_DSO || rguid == DBGUID_SESSION )
	{
		if( dwBindFlags & ~(DBBINDURLFLAG_READ) )
		{
			return E_INVALIDARG;
		}
	}
	else if( rguid == DBGUID_ROW || rguid == DBGUID_ROWSET )		
	{
		NULL;
	}
	else if( rguid == DBGUID_STREAM ) 
	{
		// Sample Provider's Streams are read only 
		if( dwBindFlags & DBBINDURLFLAG_WRITE )
			return DB_E_READONLY;
	}
	else
	{
		// invalid rguid
		return DB_E_NOTSUPPORTED;
	}
	
	return S_OK;
}


// CImpIBindResource::ParseURL -----------------------------------------------
//
// @mfunc Parses the URL for tablename and row number information
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK               | Method succeeded
//      @flag DB_E_NOTFOUND      | The object does not exist
//
HRESULT CImpIBindResource::ParseURL
	(
	LPCOLESTR			pwszURL,
	WCHAR **			ppwszDataSource,
	WCHAR **			ppwszTableName,
	ULONG *				pulRowNum
	)
{
	// parse the URL for destination file.
	// The sample provider recognizes URLs of the form
	// sampprov: [datasource=c:\oledbtst], file=customer.csv, [row=n]
	//
	// The datasource keyword is ignored when using a session object's
	// IBindResource implementation.

	const WCHAR *	pwsz = NULL;
	WCHAR *			pwszRowNum = NULL;
	WCHAR *			pwszStop = NULL;
	ULONG			cchPrefix = (sizeof(SAMPPROV_URL_PREFIX)-sizeof(WCHAR))/sizeof(WCHAR);

	if( !pwszURL )
		return E_INVALIDARG;

	// Check that the prefix is of the form  "sampprov:..."
	if( 0 != _wcsnicmp(pwszURL, SAMPPROV_URL_PREFIX, cchPrefix) ||
		L':' != *(pwszURL+cchPrefix) )
		return DB_E_NOTFOUND;

	pwsz = pwszURL+cchPrefix;
	while ( iswspace(*pwsz) )
		pwsz++;

	FindKeyword(pwsz, g_wszDataSourceKeyword, ppwszDataSource);
	FindKeyword(pwsz, g_wszFileKeyword, ppwszTableName);

	if( FindKeyword(pwsz, g_wszRowKeyword, &pwszRowNum) )
	{
		*pulRowNum = wcstol(pwszRowNum, &pwszStop, 10);
		if( pwszStop && pwszStop[0] != L'\0' )
		{
			delete [] pwszRowNum;
			return DB_E_NOTFOUND;
		}
		delete [] pwszRowNum;
	}
	
    return S_OK;
}


// CImpIBindResource::FindKeyword---------------------------------------------
//
// @mfunc Extracts a keyword value
//
// @rdesc BOOL 
//      @flag TRUE				| Found a keyword value
//      @flag FALSE             | Failed to find a value
//
BOOL CImpIBindResource::FindKeyword
	(
	LPCOLESTR			pwszURL,
	LPCOLESTR			pwszKeyword,
	WCHAR **			ppwszToken
	)
{
	assert( pwszKeyword && pwszURL && ppwszToken );

	while( *pwszURL )
    {
		const WCHAR* p1 = pwszURL;
		const WCHAR* p2 = pwszKeyword;		
		
        while(*p1 && *p2 && (towlower(*p1)==towlower(*p2)) )
			p1++, p2++;

        if( !*p2 ) 
		{
			const WCHAR* pTokBegin = NULL;
			const WCHAR* pTokEnd = NULL;

			pTokBegin = pwszURL + wcslen(pwszKeyword);
			while( iswspace(*pTokBegin) )
				pTokBegin++;

			if( *pTokBegin == L'=' )
			{
				pTokBegin += 1;
				while( iswspace(*pTokBegin) )
					pTokBegin++;
				
				pTokEnd = pTokBegin;
				while( *pTokEnd && *pTokEnd != L',' )
					pTokEnd++;

				*ppwszToken = new WCHAR [pTokEnd - pTokBegin + 1];
				if( !ppwszToken )
					return FALSE;

				StringCchCopyNW(*ppwszToken,pTokEnd - pTokBegin + 1,pTokBegin, pTokEnd - pTokBegin);
				*(*ppwszToken + (pTokEnd - pTokBegin)) = L'\0';
				return TRUE;
			}
			return FALSE;
		}

        pwszURL++;
    }

	return FALSE;
}


// CImpIBindResource::Bind ---------------------------------------------------
//
// @mfunc Retrieves an interface pointer to service a URL
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                   | Method Succeeded
//      @flag DB_S_ERRORSOCCURED     | Some bind flags were not satisfied
//		@flag DB_E_NOTFOUND          | The object does not exist
//      @flag E_INVALIDARG           | pwszURL or ppUnk were NULL
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIBindResource::Bind
	(
	IUnknown *			pUnkOuter, 
	LPCOLESTR			pwszURL,
	DBBINDURLFLAG		dwBindFlags,
	REFGUID				rguid,
	REFIID				riid,
	IAuthenticate *		pAuthenticate,
	DBIMPLICITSESSION *	pImplSession,
	DBBINDURLSTATUS *	pdwBindStatus,
	IUnknown **			ppUnk
	)
{
	HRESULT			hr;
	WCHAR *			pwszDataSource = NULL;
	WCHAR *			pwszFile = NULL;
	ULONG			ulRowNum = 0;
	DBBINDURLSTATUS	dwBindStatus = DBBINDURLSTATUS_S_OK;

	// Ignore pImplSession when binding to a DSO 
	// Also, on a session object, always ignore pImplSession
	if( BOT_SESSION == m_pObj->GetBaseObjectType() ||
		rguid == DBGUID_DSO )
		pImplSession = NULL;

	// On a session object, ignore pUnkOuter when binding to
	// objects that already exist
	if( BOT_SESSION == m_pObj->GetBaseObjectType() &&
		(rguid == DBGUID_DSO || rguid == DBGUID_SESSION) )
		pUnkOuter = NULL;

	if( !pdwBindStatus )
		pdwBindStatus = &dwBindStatus;

	// When implemented on a session object, a more functional
	// provider would examine it's DBPROP_INIT_MODE and DBPROP_INIT_BINDFLAGS
	// properties to determine if it should use its inherited binding 
	// options.  In the case of the sample provider, there are no
	// binding options to inherit.

	// Validate the Bind Arguments
	TESTC(hr = ValidateBindArgs(pUnkOuter, pwszURL, dwBindFlags, rguid, 
						riid, pImplSession, pAuthenticate, pdwBindStatus, ppUnk));
	
	// Parse the URL for binding information
	// Ignore datasource keyword for now...
	TESTC(hr = ParseURL(pwszURL, &pwszDataSource, &pwszFile, &ulRowNum));

	if( rguid == DBGUID_DSO )
	{
		TESTC(hr = BindDSO(pUnkOuter, riid, dwBindFlags & DBBINDURLFLAG_WAITFORINIT,
							pwszDataSource, ppUnk));
	}
	else if( rguid == DBGUID_SESSION )
	{
		TESTC(hr = BindSession(pUnkOuter, riid, pwszDataSource, ppUnk));
	}
	else if( rguid == DBGUID_ROWSET )
	{
		if( 0 != ulRowNum)
		{
			hr = DB_E_NOTCOLLECTION;
			goto CLEANUP;
		}

		TESTC(hr = BindRowset(pUnkOuter, riid, pImplSession, pwszDataSource, pwszFile, ppUnk));
	}
	else if( rguid == DBGUID_ROW )
	{
		TESTC(hr = BindRow(pUnkOuter, riid, pImplSession, pwszDataSource, pwszFile, ulRowNum, ppUnk));
	}
	else if( rguid == DBGUID_STREAM )
	{
		TESTC(hr = BindStream(pUnkOuter, riid, pImplSession, pwszDataSource, pwszFile, ulRowNum, ppUnk));
	}
	else
	{
		assert(!"Bad rguid type not detected!");		
	}


CLEANUP:
	if( pwszDataSource )
		delete [] pwszDataSource;
	if( pwszFile )
		delete [] pwszFile;

	if( FAILED(hr) && ppUnk )
	{
		*ppUnk = NULL;
	}

	// Bind status is only set on DB_S_ERRORSOCCURRED
	if( hr == S_OK )
	{
		if( *pdwBindStatus )
			hr = DB_S_ERRORSOCCURRED;
	}
	else
		*pdwBindStatus = DBBINDURLSTATUS_S_OK;;

	return hr;
}


//  ICreateRow specific interface methods

// CImpICreateRow::CreateRow -------------------------------------------------
//
// @mfunc Retrieves an interface pointer to service a URL
//
// @rdesc	Returns one of the following values:
//      @flag E_NOINTERFACE           |	Sample provider does not support 
//										ICreateRow
//
STDMETHODIMP CImpICreateRow::CreateRow 
	(
	IUnknown *			pUnkOuter, 
	LPCOLESTR			pwszURL,
	DBBINDURLFLAG		dwBindFlags,
	REFGUID				rguid,
	REFIID				riid,
	IAuthenticate *		pAuthenticate,
	DBIMPLICITSESSION *	pImplSession,
	DBBINDURLSTATUS	*	pdwBindStatus,	
	LPOLESTR *			ppwszNewURL,
	IUnknown **			ppUnk
	)
{
	// ICreateRow is a mandatory interface on Provider Binders
	// Although there is no support for object creation, the Sample Provider
	// must support this interface and simply return E_NOINTERFACE
	return E_NOINTERFACE;
}


//  IDBBinderProperties specific interface methods

// CImpIDBBinderProperties::GetProperties ------------------------------------
//
// @mfunc Returns current settings of all properties in the FLAGS_DATASRCINF 
//			property group
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpIDBBinderProperties::GetProperties
	(
	ULONG				cPropertySets,		
	const DBPROPIDSET*	rgPropertySets, 	
	ULONG*              pcPropertySets, 	
	DBPROPSET**			prgPropertySets 	    
	)
{
    assert( m_pObj );
    assert( m_pObj->m_pUtilProp );

	HRESULT hr = m_pObj->m_pUtilProp->GetPropertiesArgChk(PROPSET_DSO, cPropertySets, 
								rgPropertySets, pcPropertySets, prgPropertySets);
	if ( FAILED(hr) )
		return hr;

    // Just pass this call on to the utility object that manages our properties
    return m_pObj->m_pUtilProp->GetProperties(
									PROPSET_DSO,
									cPropertySets, 
									rgPropertySets,
									pcPropertySets, 
									prgPropertySets );
}


// CImpIDBBinderProperties::GetPropertyInfo  ---------------------------------
//
// @mfunc Returns information about rowset and data source properties supported
// by the provider
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcPropertyInfo or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpIDBBinderProperties::GetPropertyInfo
	( 
	ULONG				cPropertySets, 
	const DBPROPIDSET*	rgPropertySets,
	ULONG*				pcPropertyInfoSets, 
	DBPROPINFOSET**		prgPropertyInfoSets,
	WCHAR**				ppDescBuffer
	)
{
    assert( m_pObj );
    assert( m_pObj->m_pUtilProp );

    // just pass this call on to the utility object that manages our properties
    return m_pObj->m_pUtilProp->GetPropertyInfo(
									FALSE,
									cPropertySets, 
									rgPropertySets,
									pcPropertyInfoSets, 
									prgPropertyInfoSets,
									ppDescBuffer);
}


// CImpIDBBinderProperties::SetProperties  --------------------------------------
//
// @mfunc Set properties in the FLAGS_DATASRCINF property group
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | cProperties was not equal to 0 and rgProperties was NULL
//
STDMETHODIMP CImpIDBBinderProperties::SetProperties
	(
	ULONG		cPropertySets,		//@parm IN | Count of structs returned
	DBPROPSET	rgPropertySets[]    //@parm IN | Array of Properties
	)
{
    HRESULT hr = E_FAIL;

	//
	// Asserts
	//
	assert( m_pObj );
    assert( m_pObj->m_pUtilProp );

	//
	// Quick return if the Count of cPropertySets is 0
	//
	if( cPropertySets == 0 )
		return S_OK;

    //
    // Check in-params and NULL out-params in case of error
    //
	hr=m_pObj->m_pUtilProp->SetPropertiesArgChk(cPropertySets, rgPropertySets);
	
	if( FAILED(hr) )
		return hr;

	//
    // just pass this call on to the utility object that manages our properties
	//
    return m_pObj->m_pUtilProp->SetProperties(PROPSET_DSO,
											  cPropertySets, rgPropertySets);
}


// CImpIDBBinderProperties::Reset  ----------------------------------------------
//
// @mfunc Reset properties to their default value
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpIDBBinderProperties::Reset()
{
	PCUTILPROP	pCUtilProp = NULL;

	pCUtilProp = new CUtilProp();
	if( !pCUtilProp )
		return E_OUTOFMEMORY;

	SAFE_DELETE( m_pObj->m_pUtilProp );
	m_pObj->m_pUtilProp = pCUtilProp;
	
	return S_OK;
}