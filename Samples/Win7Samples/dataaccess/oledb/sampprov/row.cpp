//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ROW.CPP | CRow object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------

// CRow::CRow ----------------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CRow::CRow
    (
    LPUNKNOWN pUnkOuter         //@parm IN | Outer Unkown Pointer
    )	// invoke ctor for base class
	: CBaseObj( BOT_ROW )
{
	//  Initialize simple member vars
    m_cRef          = 0L;
	m_pUnkOuter		= pUnkOuter ? pUnkOuter : this;
	m_pFileio		= NULL;
	m_cbRowSize		= 0;
	m_pRowBuff		= NULL;
	
	m_pParentObj	= NULL;

	m_pIRow				= NULL;
	m_pIColumnsInfo		= NULL;
	m_pIConvertType		= NULL;
	m_pIGetSession		= NULL;
	m_pIRowChange		= NULL;

	// Fill in information about the ROW object's default stream
	// This is the only extra column that Sample Provider support
	m_rgExtracolinfo[DEFAULT_STREAM].wType			= DBTYPE_IUNKNOWN;
	m_rgExtracolinfo[DEFAULT_STREAM].ulColumnSize	= sizeof(IUnknown *);
	m_rgExtracolinfo[DEFAULT_STREAM].pwszName		= NULL;
	m_rgExtracolinfo[DEFAULT_STREAM].pTypeInfo		= NULL;
	m_rgExtracolinfo[DEFAULT_STREAM].iOrdinal		= 0;		
	m_rgExtracolinfo[DEFAULT_STREAM].dwFlags		= DBCOLUMNFLAGS_ISDEFAULTSTREAM;
	m_rgExtracolinfo[DEFAULT_STREAM].bPrecision		= ~0;
	m_rgExtracolinfo[DEFAULT_STREAM].bScale			= ~0;
	memcpy(&m_rgExtracolinfo[DEFAULT_STREAM].columnid, &DBROWCOL_DEFAULTSTREAM,
			sizeof(DBID));

	m_cExtraCols		= EXTRA_COLUMNS;
	
	//Associated Row Handle
	m_hRow				= DB_NULL_HROW;

	// Increment global object count.
    OBJECT_CONSTRUCTED();
}


// CRow::~CRow ---------------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CRow:: ~CRow
    (
    void
    )
{
	delete [] m_pRowBuff;

	SAFE_DELETE(m_pFileio);

	SAFE_DELETE(m_pIRow);
	SAFE_DELETE(m_pIColumnsInfo);
	SAFE_DELETE(m_pIConvertType);
	SAFE_DELETE(m_pIGetSession);
	SAFE_DELETE(m_pIRowChange);

	//Release outstanding row handle...
	IRowset *	pIRowset = NULL;
	if( m_pParentObj &&
		m_pParentObj->GetBaseObjectType() == BOT_ROWSET &&
		SUCCEEDED(m_pParentObj->QueryInterface(IID_IRowset, (void**)&pIRowset)) )
	{
		if( pIRowset )
		{
			pIRowset->ReleaseRows(1, &m_hRow, NULL, NULL, NULL);
			pIRowset->Release();
		}
	}

	if( m_pParentObj )	
		m_pParentObj->GetOuterUnknown()->Release();
}


// CRow::FInit ---------------------------------------------------------------
//
// @mfunc Initialize the row Object when it is a part of a rowset
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE  | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CRow::FInit
	(
	CRowset *	pParentObj		//@parm IN | pointer to parent rowset object
	)
{
	assert(pParentObj);
	assert(pParentObj->m_pFileio);
    assert(m_pUnkOuter);
 
	m_pParentObj = pParentObj;
	m_pParentObj->GetOuterUnknown()->AddRef();

    // Allocate contained interface objects
	m_pIRow						= new CImpIRow( this, m_pUnkOuter );
    m_pIColumnsInfo	            = new CImpIColumnsInfo( this, m_pUnkOuter );
	m_pIConvertType				= new CImpIConvertType( this, m_pUnkOuter );
	m_pIGetSession				= new CImpIGetSession( this, m_pUnkOuter );
    m_pIRowChange				= new CImpIRowChange( this, m_pUnkOuter );

	m_cbRowSize = pParentObj->m_cbRowSize;

    // if all interfaces were created, return success
    return (BOOL) (m_pIRow && m_pIColumnsInfo && m_pIConvertType &&
                   m_pIGetSession && m_pIRowChange);
}


// CRow::FInit ---------------------------------------------------------------
//
// @mfunc Initialize the row Object on a direct bind
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE  | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CRow::FInit
	(
	CDBSession *	pParentObj,			//@parm IN | pointer to parent session object
	CFileIO *		pFileIO,			//@parm IN | pointer to FileIO object
	DBCOUNTITEM		ulRowNum			//@parm IN | Row number to fetch
	)
{
	DBORDINAL		cCols;
	COLUMNDATA *	pColumn = NULL;

	assert(pFileIO);
    assert(m_pUnkOuter);

	m_pParentObj = pParentObj;
	m_pParentObj->GetOuterUnknown()->AddRef();

	m_pFileio = pFileIO;

    // Allocate contained interface objects
	m_pIRow						= new CImpIRow( this, m_pUnkOuter );
    m_pIColumnsInfo	            = new CImpIColumnsInfo( this, m_pUnkOuter );
	m_pIConvertType				= new CImpIConvertType( this, m_pUnkOuter );
	m_pIGetSession				= new CImpIGetSession( this, m_pUnkOuter );
    m_pIRowChange				= new CImpIRowChange( this, m_pUnkOuter );

	m_cbRowSize = m_pFileio->GetRowSize();
	m_pRowBuff = new BYTE [ ROUND_UP( m_pFileio->GetRowSize() + sizeof(ROWBUFF), COLUMN_ALIGNVAL ) ];
	memset(m_pRowBuff, 0, ROUND_UP( m_pFileio->GetRowSize() + sizeof(ROWBUFF), COLUMN_ALIGNVAL ) );

	// Set up the row buff bindings
	for (cCols=1; cCols <= m_pFileio->GetColumnCnt(); cCols++)
	{
		pColumn = m_pFileio->GetColumnData(cCols, (ROWBUFF *)m_pRowBuff);
		if (FAILED(m_pFileio->SetColumnBind(cCols, pColumn)))
			return FALSE;
	}

	// Actually fetch the data
    if( S_OK != m_pFileio->Fetch( ulRowNum ) )
        return FALSE;

	((ROWBUFF *)m_pRowBuff)->ulRefCount = 1;
	((ROWBUFF *)m_pRowBuff)->pbBmk = /*(void *)*/ulRowNum;
    
    


    // if all interfaces were created, return success
    return (BOOL) (m_pIRow && m_pIColumnsInfo && m_pIConvertType &&
                   m_pIGetSession && m_pIRowChange && m_pFileio &&
				   m_pRowBuff);
}


// CRow::QueryInterface ------------------------------------------------------
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
STDMETHODIMP CRow::QueryInterface
    (
    REFIID riid,
    LPVOID * ppv
    )
{
	if( NULL == ppv )
		return E_INVALIDARG;

	//  Place NULL in *ppv in case of failure
	*ppv = NULL;

    //  This is the non-delegating IUnknown implementation
	if( riid == IID_IUnknown )
		*ppv = (LPVOID) this;											 
	else if( riid == IID_IRow )
		*ppv = (LPVOID) m_pIRow;
	else if( riid == IID_IColumnsInfo )
		*ppv = (LPVOID) m_pIColumnsInfo;
	else if( riid == IID_IConvertType ) 
		*ppv = (LPVOID) m_pIConvertType;
	else if( riid == IID_IGetSession )
		*ppv = m_pIGetSession;
	else if( riid == IID_IRowChange )
		*ppv = m_pIRowChange;

    //  If we're going to return an interface, AddRef it first
    if( *ppv )
	{
        ((LPUNKNOWN) *ppv)->AddRef();
        return ResultFromScode( S_OK );
	}
    else
        return ResultFromScode( E_NOINTERFACE );
}


// CRow::AddRef --------------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CRow::AddRef
     (
     void
     )
{
    return ++m_cRef;
}


// CRow::Release -------------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CRow::Release
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


// CRow::SetRowHandle --------------------------------------------------------
//
// @mfunc Internal member function which sets the HROW which this row object 
// represents
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK               | method succeeded
//      @flag DB_E_DELETEDROW    | hrow referred to a deleted row
//      @flag DB_E_BADROWHANDLE  | hrow did not belong to the parent rowset
//
STDMETHODIMP CRow::SetRowHandle(HROW hRow)
{
	IRowset *		pIRowset	= NULL;
	HRESULT			hr			= DB_E_NOSOURCEOBJECT;
	DBROWSTATUS		dwRowStatus = DBROWSTATUS_S_OK;

	//Obtain the parent rowset...
	if( m_pParentObj )
	{
		hr = m_pParentObj->QueryInterface(IID_IRowset, (void**)&pIRowset);

		if( pIRowset )
		{
			//AddRef the row,
			//To be able to hang onto the row, and determine if its invalid or not...
			if(FAILED(hr = pIRowset->AddRefRows(1, &hRow, NULL, &dwRowStatus)))
			{
				switch(dwRowStatus)
				{
					case DBROWSTATUS_E_DELETED:
						hr = DB_E_DELETEDROW;
						break;

					default:
						hr = DB_E_BADROWHANDLE;
						break;
				};
			}

			//We made it this far, store the row...
			if( SUCCEEDED(hr) )
				m_hRow = hRow;
			pIRowset->Release();
		}
	}

	return hr;
}
 

// CRow::GetRowBuff ----------------------------------------------------------
//
// @mfunc Returns the underlying ROWBUFF
//
ROWBUFF *	CRow::GetRowBuff()
{
	if( BOT_ROWSET == m_pParentObj->GetBaseObjectType() )
		return ((CRowset *)m_pParentObj)->GetRowBuff( m_hRow, TRUE );
	else
	{
		assert( BOT_SESSION == m_pParentObj->GetBaseObjectType() );
		return (ROWBUFF *)m_pRowBuff;
	}
}


// CRow::GetFileObj ----------------------------------------------------------
//
// @mfunc Returns the underlying Fileio object
//
CFileIO *	CRow::GetFileObj()
{
	if( BOT_ROWSET == m_pParentObj->GetBaseObjectType() )
		return ((CRowset *)m_pParentObj)->m_pFileio;
	else
	{
		assert( BOT_SESSION == m_pParentObj->GetBaseObjectType() );
		return m_pFileio;
	}
}


// CRow::GetColumnData -------------------------------------------------------
//
// @mfunc Returns TRUE if pColumnID is valid
//
BOOL	CRow::GetColumnOrdinal
	(
	CFileIO *	pFileio,
	DBID *		pColumnID,
	DBORDINAL *	pcCol
	)
{
	if( (pColumnID->eKind == DBKIND_GUID_PROPID) &&
		(pColumnID->uGuid.guid == GUID_NULL)     &&
		(pColumnID->uName.ulPropid >= 1)          &&
        (pColumnID->uName.ulPropid <= pFileio->GetColumnCnt()) )
    {
        if( pcCol )
			*pcCol = pColumnID->uName.ulPropid;	
		return TRUE;
    }
    else
        return FALSE;
}
 

// CImpIRow::GetColumns ------------------------------------------------------
//
// @mfunc Gets multiple columns from the row
//
// @rdesc HRESULT
//      @flag S_OK                 | Method Succeeded
//      @flag E_INVALIDARG         | rgColumns was null and cColumns != 0
//      @flag DB_E_DELETEDROW      | The row has been deleted 
//      @flag DB_S_ERRORSOCCURRED  | Failure to retrieve one or more values
//
STDMETHODIMP CImpIRow::GetColumns
	(
	DBORDINAL cColumns, 
	DBCOLUMNACCESS rgColumns[]
	)
{
	HRESULT			hr;
	DBORDINAL		cCols;
	DBORDINAL		cIter;
	DBORDINAL		cErrorCount;
	ROWBUFF *		pRowBuff = NULL;
	COLUMNDATA *	pColumnData = NULL;
	CFileIO *		pFileio = NULL;
	DBCOLUMNINFO *	rgdbcolumninfo = NULL;
	CStream *		pCStream = NULL;
	
	//
	// Asserts
    //
	assert( m_pObj );

	// cColumns == 0 is a no-op
	if( cColumns == 0 )
		return S_OK;

	if( !rgColumns )
		return E_INVALIDARG;

	// Check to see if we have a DC
	if( !g_pIDataConvert )
		return E_FAIL;

	pFileio = m_pObj->GetFileObj();
	assert( pFileio );

    // Internal error for a 0 reference count on this row,
    // since we depend on the slot-set stuff.
    pRowBuff = m_pObj->GetRowBuff();
    assert( pRowBuff->ulRefCount );

	// Check for the Deleted Row
	if ( pFileio->IsDeleted( (DBBKMARK) pRowBuff->pbBmk ) == S_OK )
        return DB_E_DELETEDROW;

    cErrorCount = 0;
	rgdbcolumninfo = pFileio->GetColInfo();
    for (cIter = 0; cIter < cColumns; cIter++)
	{
		if( IsEqualDBID(&rgColumns[cIter].columnid, &DBROWCOL_DEFAULTSTREAM) )
		{
			if( rgColumns[cIter].wType == DBTYPE_IUNKNOWN )
			{
				pCStream = new CStream(NULL);
				if( pCStream && pCStream->FInit(m_pObj, pRowBuff) )
				{
					//Obtain the requested interface
					pCStream->AddRef();
					if( rgColumns[cIter].pData )
						*((IUnknown **)rgColumns[cIter].pData) = (IUnknown *)pCStream;
					else
						pCStream->Release();
			
					rgColumns[cIter].dwStatus = DBSTATUS_S_OK;
					rgColumns[cIter].cbDataLen = sizeof(IUnknown *);
				}
				else
				{
					rgColumns[cIter].dwStatus = DBSTATUS_E_CANTCREATE;
					cErrorCount++;

				}
			}
			else
			{
				rgColumns[cIter].dwStatus = DBSTATUS_E_BADACCESSOR;
				cErrorCount++;
			}
			continue;
		}
	
		if( !m_pObj->GetColumnOrdinal(pFileio, &rgColumns[cIter].columnid, &cCols) )
		{
			rgColumns[cIter].dwStatus = DBSTATUS_E_DOESNOTEXIST;
			cErrorCount++;
			continue;
		}

		pColumnData = pFileio->GetColumnData(cCols, pRowBuff);

        hr = g_pIDataConvert->DataConvert(
                rgdbcolumninfo[cCols].wType,	// src Type
                rgColumns[cIter].wType,			// dst Type
                pColumnData->uLength,			// src Length
                &rgColumns[cIter].cbDataLen,	// dst Length
                &(pColumnData->bData),			// pSrc
                rgColumns[cIter].pData,			// pDst
                rgColumns[cIter].cbMaxLen,		// cbMaxLen
                pColumnData->dwStatus,			// src Status
                &rgColumns[cIter].dwStatus,		// dst Status
                rgColumns[cIter].bPrecision,	// bPrecision for conversion to DBNUMERIC
				rgColumns[cIter].bScale,		// bScale for conversion to DBNUMERIC
				DBDATACONVERT_DEFAULT);
        
		if (hr != S_OK)
            cErrorCount++;
	}

	// Return S_OK if all columns retrieved with successful status
    // Return DB_S_ERRORSOCCURED on failure to retrieve one or more column values
    // Return DB_E_ERRORSOCCURED on failure to retrieve all column values

	return cErrorCount ? ( cErrorCount < cColumns ) ? 
		ResultFromScode( DB_S_ERRORSOCCURRED ) : 
		ResultFromScode( DB_E_ERRORSOCCURRED ) : 
		ResultFromScode( S_OK );
}


// CImpIRow::GetSourceRowset -------------------------------------------------
//
// @mfunc Returns the rowset interface of which this row is a part
//
// @rdesc HRESULT
//      @flag S_OK          | Method Succeeded
//      @flag E_INVALIDARG  | Invalid parameters were specified
//
STDMETHODIMP CImpIRow::GetSourceRowset
	(
	REFIID		riid, 
	IUnknown ** ppRowset,
	HROW *		phRow
	)
{
	HRESULT		hr;
	CRowset *	pCRowset = NULL;
	IUnknown *	pSourceRowset = NULL;
	ROWBUFF *	pRowBuff = NULL;
	
	//
	// Asserts
    //
	assert(m_pObj);
	assert(m_pObj->m_pParentObj);

	if( !ppRowset && !phRow )
		return E_INVALIDARG;

	if( ppRowset )
		*ppRowset = NULL;
	if( phRow )
		*phRow = DB_NULL_HROW;

	if( BOT_SESSION == m_pObj->m_pParentObj->GetBaseObjectType() )
		return DB_E_NOSOURCEOBJECT;

	pCRowset = (CRowset *)m_pObj->m_pParentObj;
	if( !pCRowset )
		return E_FAIL;

	hr = pCRowset->m_pUnkOuter->QueryInterface(riid, (LPVOID *)&pSourceRowset);
	if( FAILED(hr) )
		return hr;		
	
	// Check for the Deleted Row
    pRowBuff = pCRowset->GetRowBuff( (DBCOUNTITEM)m_pObj->m_hRow, TRUE );
    assert( pRowBuff && pRowBuff->ulRefCount );
	
	if ( pCRowset->m_pFileio &&
		 pCRowset->m_pFileio->IsDeleted( (DBBKMARK) pRowBuff->pbBmk ) == S_OK )
	{
		pSourceRowset->Release();
        return DB_E_DELETEDROW;
	}

	if( ppRowset )
		*ppRowset = pSourceRowset;
	else
		pSourceRowset->Release();

	if( phRow )
	{
		// return the HROW and add a refcout
		*phRow = m_pObj->m_hRow;

		pRowBuff->ulRefCount++;
		pCRowset->m_ulRowRefCount++;
	}

	return S_OK;
}


// CImpIRow::Open ------------------------------------------------------------
//
// @mfunc Retrieves interface from an object valued column
//
// @rdesc HRESULT
//		@flag S_OK                 | method succeeded
//      @flag DB_E_OBJECTMISMATCH  | Requested an object that does not match
//									 the object type of the column
//
STDMETHODIMP CImpIRow::Open
	(
	IUnknown *	pUnkOuter, 
	DBID *		pColumnID, 
	REFGUID		rguidColumnType, 
	DWORD		dwBindFlags, 
	REFIID		riid, 
	IUnknown ** ppUnk
	)
{
	HRESULT		hr = S_OK;
	CStream *	pCStream = NULL;
	CFileIO *	pFileio = NULL;
	ROWBUFF *	pRowBuff = NULL;

	//
	// Asserts
    //
	assert(m_pObj);

	if( ppUnk )
		*ppUnk = NULL;
	
	if(!pColumnID )
		return E_INVALIDARG;
		
	if( pUnkOuter && riid != IID_IUnknown )
		return DB_E_NOAGGREGATION;

	pFileio = m_pObj->GetFileObj();
	pRowBuff = m_pObj->GetRowBuff();

	if( pFileio->IsDeleted( (DBBKMARK) pRowBuff->pbBmk ) == S_OK )
        return DB_E_DELETEDROW;

	// The only supported object valued column is the default stream object
	if( rguidColumnType != DBGUID_STREAM &&
		rguidColumnType != GUID_NULL )
		return DB_E_OBJECTMISMATCH;

	if( IsEqualDBID(pColumnID, &DBROWCOL_DEFAULTSTREAM) )
	{
		pCStream = new CStream(pUnkOuter);
		if( pCStream && pCStream->FInit(m_pObj, pRowBuff) )
		{
			//Obtain the requested interface
			hr = pCStream->QueryInterface(riid, (void**)ppUnk);
		}
		else
			hr = E_OUTOFMEMORY;
	}
	else
	{		
		//Either this columnid does not exist or
		//this columnid is not an object valued column
		if( !m_pObj->GetColumnOrdinal(pFileio, pColumnID, NULL) )
		{
			hr = DB_E_BADCOLUMNID;
		}
		else			
			hr = DB_E_OBJECTMISMATCH;
	}

	if( FAILED(hr) )
		SAFE_DELETE( pCStream );

	return hr;
}


// CImpIGetSession::GetSession -----------------------------------------------
//
// @mfunc Returns the session interface on which the row has been created
//
// @rdesc HRESULT
//      @flag S_OK                 | Method Succeeded
//		@flag DB_E_NOSOURCEOBJECT  | There is no session object for the row
//      @flag E_INVALIDARG         | Invalid parameters were specified
//
STDMETHODIMP CImpIGetSession::GetSession
	( 
	REFIID		riid,
    IUnknown ** ppunkSession
	)
{
	CRowset *		pCRowset = NULL;
	CBaseObj *		pCRowsetParent = NULL;
	CDBSession *	pCSession = NULL;

	//
	// Asserts
    //
	assert(m_pObj);
	assert(m_pObj->m_pParentObj);

	if( NULL == ppunkSession )
		return (E_INVALIDARG);
	
	*ppunkSession = NULL;

	if( BOT_SESSION == m_pObj->m_pParentObj->GetBaseObjectType() )
	{
		pCSession = (CDBSession *)m_pObj->m_pParentObj;
	}
	else
	{
		assert( BOT_ROWSET == m_pObj->m_pParentObj->GetBaseObjectType() );
		pCRowset = (CRowset *)m_pObj->m_pParentObj;
		if( !pCRowset )
			return E_FAIL;

		pCRowsetParent = pCRowset->m_pParentObj;
		if( !pCRowsetParent )
			return E_FAIL;

		if( BOT_SESSION == pCRowsetParent->GetBaseObjectType() )
			pCSession = (CDBSession *)pCRowsetParent;
		else if( BOT_COMMAND == pCRowsetParent->GetBaseObjectType() )
			pCSession = ((CCommand *)pCRowsetParent)->m_pCSession;
		else
			return E_FAIL;
	}

	if( pCSession )
		return pCSession->GetOuterUnknown()->QueryInterface(riid, (LPVOID*)ppunkSession);
	else
		return DB_E_NOSOURCEOBJECT;
}


// CImpIRowChange::SetColumns ------------------------------------------------
//
// @mfunc Sets multiple column values on a row
//
// @rdesc HRESULT
//      @flag S_OK                 | Method Succeeded
//      @flag E_INVALIDARG         | rgColumns was null and cColumns != 0
//      @flag DB_E_DELETEDROW      | The row has been deleted or moved
//      @flag DB_E_ERRORSOCCURRED  | No columns set
//		@flag DB_E_DELETEDROW	   | The row has been deleted
//
STDMETHODIMP CImpIRowChange::SetColumns
	(
	DBORDINAL cColumns, 
	DBCOLUMNACCESS rgColumns[]
	)
{
	HRESULT			hr;
	DBORDINAL		cIter = 0;
	DBORDINAL		cErrorCount = 0;
	DBORDINAL		cCols = 0;
	DWORD			dwSrcStatus;
	ROWBUFF *		pRowBuff = NULL;
	COLUMNDATA *	pColumnData = NULL;
	BYTE *			rgbRowDataSave = NULL;
	DBCOLUMNINFO *	rgdbcolumninfo = NULL;
	CFileIO *		pFileio = NULL;

	//
	// Asserts
    //
	assert( m_pObj );
	assert( m_pObj->m_pParentObj );

	// cColumns == 0 is a no-op
	if( cColumns == 0 )
		return S_OK;

	if( !rgColumns )
		return E_INVALIDARG;

	// Check to see if we have a DC
	if( !g_pIDataConvert )
		return E_FAIL;

	pFileio		= m_pObj->GetFileObj();
    pRowBuff	= m_pObj->GetRowBuff();

    // Is row handle deleted?
    if ( pFileio->IsDeleted((DBBKMARK) ((PROWBUFF) pRowBuff)->pbBmk ) == S_OK )
        return DB_E_DELETEDROW;

    rgbRowDataSave = new BYTE[m_pObj->m_cbRowSize];
    if ( rgbRowDataSave == NULL )
        return E_OUTOFMEMORY;

    // Save the row.
    memcpy( rgbRowDataSave, pRowBuff, m_pObj->m_cbRowSize );

	rgdbcolumninfo = pFileio->GetColInfo();
    for (cIter = 0; cIter < cColumns; cIter++)
	{
		// Check the Status for DBSTATUS_S_IGNORE
		dwSrcStatus = rgColumns[cIter].dwStatus;
		if( dwSrcStatus == DBSTATUS_S_IGNORE )
			continue;

		if( IsEqualDBID(&rgColumns[cIter].columnid, &DBROWCOL_DEFAULTSTREAM) )
		{
			// Default stream column is read only
			rgColumns[cIter].dwStatus = DBSTATUS_E_PERMISSIONDENIED;
			cErrorCount++;
			continue;
		}
		else
		{
			if( !m_pObj->GetColumnOrdinal(pFileio, &rgColumns[cIter].columnid, &cCols) )
			{
				rgColumns[cIter].dwStatus = DBSTATUS_E_DOESNOTEXIST;
				cErrorCount++;
				continue;
			}
		}

		pColumnData = pFileio->GetColumnData(cCols, pRowBuff);

        // Check the Status for DBSTATUS_S_DEFAULT
		if( dwSrcStatus == DBSTATUS_S_DEFAULT )
			dwSrcStatus = DBSTATUS_S_ISNULL;

        // Check the Status of the value being sent in
		if( (dwSrcStatus != DBSTATUS_S_OK && dwSrcStatus != DBSTATUS_S_ISNULL &&
			 dwSrcStatus != DBSTATUS_S_IGNORE && dwSrcStatus != DBSTATUS_S_DEFAULT) ) 
		{
			rgColumns[cIter].dwStatus = DBSTATUS_E_BADSTATUS;
			cErrorCount++;
			continue;
		}

		hr = g_pIDataConvert->DataConvert(
				rgColumns[cIter].wType,					// src Type
                rgdbcolumninfo[cCols].wType,			// dst Type
                rgColumns[cIter].cbDataLen,				// src Length
                &(pColumnData->uLength),				// dst Length
                rgColumns[cIter].pData,					// pSrc
                &(pColumnData->bData),					// pDst
                rgColumns[cIter].cbMaxLen,				// cbMaxLen
                dwSrcStatus,							// src Status
                &(pColumnData->dwStatus),				// dst Status
                rgColumns[cIter].bPrecision,			// bPrecision for conversion to DBNUMERIC
				rgColumns[cIter].bScale,				// bScale for conversion to DBNUMERIC
				DBDATACONVERT_SETDATABEHAVIOR);
        
		// fatal error
		if(	FAILED(hr) )
		{
			cErrorCount++;
			rgColumns[cIter].dwStatus = pColumnData->dwStatus;
			continue;
		}
	}

    // Carry out the update.
    if( FAILED( pFileio->UpdateRow((DBBKMARK) ((PROWBUFF) pRowBuff)->pbBmk, (BYTE *)pRowBuff, UPDATE )) )
	{
        // Restore the row to its previous state
        memcpy( pRowBuff, rgbRowDataSave, m_pObj->m_cbRowSize );
        return ResultFromScode( E_FAIL );
	}

    delete [] rgbRowDataSave;

	// Return S_OK if all columns were updated
    // Return DB_S_ERRORSOCCURED one or more columns failed to be updated
    // Return DB_E_ERRORSOCCURED on failure to set all columns

	return cErrorCount ? ( cErrorCount < cColumns ) ? 
			ResultFromScode( DB_S_ERRORSOCCURRED ) : 
		 	ResultFromScode( DB_E_ERRORSOCCURRED ) : 
		 	ResultFromScode( S_OK );
}