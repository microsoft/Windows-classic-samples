//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module COLINFO.CPP | IColumnsInfo interface implementation
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

//  IColumnsInfo specific methods

// CImpIColumnsInfo::GetColumnInfo -------------------------------------------
//
// @mfunc Returns the column metadata needed by most consumers.
//
// @rdesc HRESULT
//      @flag S_OK              | The method succeeded
//      @flag E_OUTOFMEMORY     | Out of memory
//      @flag E_INVALIDARG      | pcColumns or prginfo or ppStringsbuffer was NULL
//
STDMETHODIMP CImpIColumnsInfo::GetColumnInfo
(
    DBORDINAL*      pcColumns,      //@parm OUT | Number of columns in rowset
    DBCOLUMNINFO**  prgInfo,        //@parm OUT | Array of DBCOLUMNINFO Structures
    WCHAR**         ppStringsBuffer //@parm OUT | Storage for all string values
)
{
	HRESULT			hr = S_OK;
	DBORDINAL		cCols = 0;
	DBORDINAL		cExtraCols = 0;
	DBORDINAL		cbHeapUsed = 0;
	DBORDINAL		cIter = 0;
	BYTE *			pbHeap = NULL;
	WCHAR *			pwstrBuffer = NULL;
	DBCOLUMNINFO *	rgdbcolinfo = NULL;
    DBCOLUMNINFO *	rgdbInternalcolinfo = NULL;
	DBCOLUMNINFO *	rgdbExtracolinfo = NULL;
	CRowset *		pCRowset = NULL;
	CFileIO	*		pFileio = NULL;

	//
	// Asserts
    //
	assert(m_pObj);

    //
    // Check in-params and NULL out-params in case of error
    //
	if( pcColumns )
		*pcColumns = 0;

	if( prgInfo )
		*prgInfo = NULL;
	
	if( ppStringsBuffer )
		*ppStringsBuffer = NULL;

    if( !pcColumns || !prgInfo || !ppStringsBuffer )
        return E_INVALIDARG;
	
	//
	// Get the Column Information off of the Command or Rowset
	//
	if( m_pObj->GetBaseObjectType() == BOT_COMMAND )
	{
		//
		// Asserts
		//
		assert(((CCommand *) m_pObj)->m_pCSession);
		assert(((CCommand *) m_pObj)->m_pCSession->m_pCDataSource);

		//
		// Check that a command has been set
		//
		if( !((CCommand *) m_pObj)->IsCommandSet() )
			return DB_E_NOCOMMAND;

		//
		// Try to open the file...
		//
		hr = ((CCommand *) m_pObj)->m_pCSession->m_pCDataSource->OpenFile(
						   ((CCommand *) m_pObj)->GetCommandText(), &pFileio);
		if( FAILED(hr) )
			return hr;
	}
	else 
	{
		if( m_pObj->GetBaseObjectType() == BOT_ROWSET )
			pFileio = ((CRowset *) m_pObj)->GetFileObj();
		else
		{
			pFileio = ((CRow *) m_pObj)->GetFileObj();

			cExtraCols = ((CRow *) m_pObj)->GetExtraColCount();
			rgdbExtracolinfo = ((CRow *) m_pObj)->GetExtraColumnInfo();
		}
	}

	//
	// Get the column count and delete unneeded info
	//
	cCols				= pFileio->GetColumnCnt();
	pbHeap				= pFileio->GetColNameHeap();		
	cbHeapUsed			= pFileio->GetColNameHeapSize();
	rgdbInternalcolinfo = pFileio->GetColInfo();		

	//
	// Return the column information
	//
	SAFE_ALLOC(rgdbcolinfo, DBCOLUMNINFO, (cCols + cExtraCols) * sizeof(DBCOLUMNINFO));
	SAFE_ALLOC(pwstrBuffer, WCHAR, cbHeapUsed);

	memcpy(rgdbcolinfo, &(rgdbInternalcolinfo[1]), cCols*sizeof(DBCOLUMNINFO));
	memcpy(rgdbcolinfo+cCols, rgdbExtracolinfo, cExtraCols*sizeof(DBCOLUMNINFO));
	
	//
	// Need to fix up column ordinals for extra columns
	//
	for (cIter=cCols; cIter < cCols+cExtraCols; cIter++)
		rgdbcolinfo[cIter].iOrdinal = cIter+1;

	//
	// Copy the heap for column names.
	//
	if( cbHeapUsed )
	{
		ptrdiff_t dp;
		
		memcpy(pwstrBuffer, pbHeap, cbHeapUsed);
		dp = (DBBYTEOFFSET)pwstrBuffer - (DBBYTEOFFSET)(pbHeap);
		dp >>= 1;

		// Loop through columns and adjust pointers to column names.
		for (ULONG icol=0; icol < cCols; icol++)
		{
			if( rgdbcolinfo[icol].pwszName )
				rgdbcolinfo[icol].pwszName += dp;
		}
	}
	
    //
	// Assign in the values
	//
    *pcColumns       = cCols + cExtraCols;
	*prgInfo         = rgdbcolinfo;
    *ppStringsBuffer = pwstrBuffer;

CLEANUP:

	//
	// Cleanup the File Information
	//
	if( m_pObj->GetBaseObjectType() == BOT_COMMAND )
		SAFE_DELETE(pFileio);

	if( FAILED(hr) )
	{
		SAFE_DELETE(rgdbcolinfo);
		SAFE_DELETE(pwstrBuffer);
	}

    return hr;
}

// CImpIColumnsInfo::MapColumnIDs --------------------------------------------
//
// @mfunc Returns an array of ordinals of the columns in a rowset that are
// identified by the specified column IDs.
//
// @rdesc HRESULT
//      @flag S_OK                      | The method succeeded
//      @flag E_INVALIDARG              | cColumnIDs was not 0 and rgColumnIDs was NULL,
//                                        rgColumns was NULL
//      @flag DB_E_COLUMNUNAVAILABLE    | An element of rgColumnIDs was invalid
//
STDMETHODIMP CImpIColumnsInfo::MapColumnIDs
(
    DBORDINAL   cColumnIDs,     //@parm IN | Number of Column IDs to map
    const DBID	rgColumnIDs[],  //@parm IN | Column IDs to map
    DBORDINAL   rgColumns[]     //@parm OUT | Ordinal values
)
{
	DBORDINAL cCols = 0;
	ULONG	  ulError = 0;

	//
	// Asserts
    //
	assert(m_pObj);

    //
	// NO-OP if cColumnIds is 0
	//
	if( cColumnIDs == 0 )
        return S_OK;

    //
    // Check in-params and NULL out-params in case of error
    //
    if( !rgColumnIDs || !rgColumns )
        return E_INVALIDARG;

	//
	// Get the Column count
	//
	if( m_pObj->GetBaseObjectType() == BOT_COMMAND )
	{
		HRESULT hr = E_FAIL;
		CFileIO	* pFileio = NULL;
		
		//
		// Asserts
		//
		assert(((CCommand *) m_pObj)->m_pCSession);
		assert(((CCommand *) m_pObj)->m_pCSession->m_pCDataSource);

		//
		// Check that a command has been set
		//
		if( !((CCommand *) m_pObj)->IsCommandSet() )
			return DB_E_NOCOMMAND;

		//
		// Open the File and get the column count
		//
		hr = ((CCommand *) m_pObj)->m_pCSession->m_pCDataSource->OpenFile(
							((CCommand *) m_pObj)->GetCommandText(), &pFileio);
		if( FAILED(hr) )
			return hr;
		
		cCols = pFileio->GetColumnCnt();
		SAFE_DELETE(pFileio);
	}
	else
	{
		if( m_pObj->GetBaseObjectType() == BOT_ROWSET )
			cCols = ((CRowset *) m_pObj)->m_cCols;
		else
			cCols = ((CRow *) m_pObj)->GetFileObj()->GetColumnCnt();
	}

    //
	// Walk the Column ID structs and determine the ordinal value
	//
    for (ULONG i=0; i < cColumnIDs; i++)
    {	
		if( m_pObj->GetBaseObjectType() == BOT_ROW &&
			IsEqualDBID(&rgColumnIDs[i], &DBROWCOL_DEFAULTSTREAM) )
		{
			rgColumns[i] = cCols + DEFAULT_STREAM_ORDINAL;
		}
        else if( (rgColumnIDs[i].eKind != DBKIND_GUID_PROPID) ||
				 (rgColumnIDs[i].uGuid.guid != GUID_NULL)     ||
				 (rgColumnIDs[i].uName.ulPropid < 1)          ||
				 (rgColumnIDs[i].uName.ulPropid > cCols) )
        {
            rgColumns[i] = DB_INVALIDCOLUMN;
            ulError++;
        }
		else
            rgColumns[i] = rgColumnIDs[i].uName.ulPropid;
    }

	//
	// Return the correct HResult
	//
	return ulError ? (ulError < cColumnIDs) ? 
			DB_S_ERRORSOCCURRED : DB_E_ERRORSOCCURRED : S_OK;
}
