//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module IROWSET.CPP | IRowset interface implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

//  IRowset specific interface methods

// CImpIRowset::GetData --------------------------------------------------
//
// @mfunc Retrieves data from the rowset's cache
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                   | Method Succeeded
//      @flag DB_S_ERRORSOCCURED     | Could not coerce a column value
//      @flag DB_E_BADACCESSORHANDLE | Invalid Accessor given
//      @flag DB_E_BADROWHANDLE      | Invalid row handle given
//      @flag E_INVALIDARG           | pData was NULL
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIRowset::GetData
    (
    HROW        hRow,       //@parm IN | Row Handle
    HACCESSOR   hAccessor,  //@parm IN | Accessor to use
    void       *pData       //@parm OUT | Pointer to buffer where data should go.
    )
{
    PACCESSOR		pAccessor;
    DBORDINAL		cCols;
	DBORDINAL		cBind;
    ROWBUFF			*pRowBuff;
    COLUMNDATA		*pColumnData;
    DBBINDING		*pBinding;
    DBCOUNTITEM		cBindings;
    DBORDINAL		cErrorCount;
    DBTYPE			dwSrcType;
    DBTYPE			dwDstType;
    void			*pSrc;
    void			*pDst;
    DBBYTEOFFSET	ulSrcLength;
    DBLENGTH*		pulDstLength;
    DBLENGTH		cbDstMaxLength;
    ULONG			dwSrcStatus;
    ULONG			*pdwDstStatus;
    DWORD			dwPart;
    HRESULT			hr;

	DBCOLUMNINFO *	rgdbcolumninfo = NULL;

    // Coerce data for row 'hRow', according to hAccessor.
    // Put in location 'pData'.  Offsets and types are in hAccessor's bindings.
    //
	// Return S_OK if all columns retrieved with successful status
    // Return DB_S_ERRORSOCCURED on failure to retrieve one or more column values
    // Return DB_E_ERRORSOCCURED on failure to retrieve all column values

    // GetItemOfExtBuffer is basically operator[].
    // It takes an index (or handle) (referenced from 1...n),
    // and a ptr for where to write the data.
    //
    // It holds ptrs to a variable-length ACCESSOR struct.
    // So we get the ACCESSOR ptr for the client's accessor handle.

    assert( m_pObj->m_pextbufferAccessor );
    m_pObj->m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &pAccessor);
    if( !pAccessor )
        return DB_E_BADACCESSORHANDLE;

    assert( pAccessor );
    cBindings = pAccessor->cBindings;
    pBinding  = pAccessor->rgBindings;

    // IsSlotSet returns S_OK    if row is marked.
    //                   S_FALSE if row is not marked.
    // The "mark" means that there is data present in the row.
    // Rows are [1...n], slot marks are [0...n-1].
    if (m_pObj->m_prowbitsIBuffer->IsSlotSet( (ULONG) hRow ) != S_OK)
        return ResultFromScode( DB_E_BADROWHANDLE );

	// Ensure a place to put data, unless the accessor is the null accessor then
    // a NULL pData is okay.
    if ( pData == NULL && cBindings != 0 )
        return ResultFromScode( E_INVALIDARG );

	// Check to see if we have a DC
	if( !g_pIDataConvert)
		return (E_FAIL);

    // Internal error for a 0 reference count on this row,
    // since we depend on the slot-set stuff.
    pRowBuff = m_pObj->GetRowBuff( (DBCOUNTITEM) hRow, TRUE );
    assert( pRowBuff->ulRefCount );

	// Check for the Deleted Row
	if ( m_pObj->m_pFileio->IsDeleted( (DBBKMARK) pRowBuff->pbBmk ) == S_OK )
        return ResultFromScode( DB_E_DELETEDROW );

    cErrorCount = 0;
	rgdbcolumninfo = m_pObj->m_pFileio->GetColInfo();
    for (cBind=0; cBind < cBindings; cBind++)
	{
		cCols = pBinding[cBind].iOrdinal;

        // make sure column number is in range
        if ( !(0 < cCols && cCols <= m_pObj->m_cCols) )
            return ResultFromScode( DB_E_BADORDINAL );

        pColumnData    = m_pObj->m_pFileio->GetColumnData(cCols, pRowBuff);

        dwSrcType      = rgdbcolumninfo[cCols].wType;
        pSrc           = &(pColumnData->bData);
        ulSrcLength    = pColumnData->uLength;
        dwSrcStatus    = pColumnData->dwStatus;
        cbDstMaxLength = pBinding[cBind].cbMaxLen;
        dwDstType      = pBinding[cBind].wType;
        dwPart         = pBinding[cBind].dwPart;

        pDst           = dwPart & DBPART_VALUE ? ((BYTE *) pData + pBinding[cBind].obValue) : NULL;
        pulDstLength   = dwPart & DBPART_LENGTH ? (DBLENGTH *) ((BYTE*) pData + pBinding[cBind].obLength) : NULL;
        pdwDstStatus   = dwPart & DBPART_STATUS ? (ULONG *) ((BYTE*) pData + pBinding[cBind].obStatus) : NULL;

        hr = g_pIDataConvert->DataConvert(
                dwSrcType,
                dwDstType,
                ulSrcLength,
                pulDstLength,
                pSrc,
                pDst,
                cbDstMaxLength,
                dwSrcStatus,
                pdwDstStatus,
                pBinding[cBind].bPrecision,	// bPrecision for conversion to DBNUMERIC
				pBinding[cBind].bScale,		// bScale for conversion to DBNUMERIC
				DBDATACONVERT_DEFAULT);
        
		// rounding or truncation or can't coerce
		if (hr != S_OK)
            cErrorCount++;
	}

    // We report any lossy conversions with a special status.
    // Note that DB_S_ERRORSOCCURED is a success, rather than failure.
	return cErrorCount ? ( cErrorCount < cBindings ) ? 
		( DB_S_ERRORSOCCURRED ) : ( DB_E_ERRORSOCCURRED ) : ( S_OK );
}



// CImpIRowset::GetNextRows --------------------------------------------------
//
// @mfunc Fetches rows in a sequential style, remembering the previous position
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                      | Method Succeeded
//      @flag DB_S_ENDOFROWSET          | Reached end of rowset
//      @flag DB_E_CANTFETCHBACKWARDS   | cRows was negative and we can't fetch backwards
//      @flag DB_E_ROWSNOTRELEASED      | Must release all HROWs before calling GetNextRows
//      @flag E_FAIL                    | Provider-specific error
//      @flag E_INVALIDARG              | pcRowsObtained or prghRows was NULL
//      @flag E_OUTOFMEMORY             | Out of Memory
//      @flag OTHER                     | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIRowset::GetNextRows
    (
    HCHAPTER    hChapter,        //@parm IN | The Chapter handle.
    DBROWOFFSET lRowOffset,      //@parm IN | Rows to skip before reading
    DBROWCOUNT  cRows,           //@parm IN | Number of rows to fetch
    DBCOUNTITEM *pcRowsObtained, //@parm OUT | Number of rows obtained
    HROW        **prghRows       //@parm OUT | Array of Hrows obtained
    )
{
    ULONG		cRowsTmp;
    ULONG		cSlotAlloc =0;
    DBROWCOUNT	irow, ih;
    ULONG		cRowFirst, cRowLast;
    PROWBUFF	prowbuff;
    HRESULT		hr;
	BOOL		fCanHoldRows = FALSE;
	DBPROPIDSET	rgPropertyIDSets[1];
	ULONG		cPropertySets;
	DBPROPSET*	prgPropertySets;
	DBPROPID	rgPropId[1];


    // init out-params
	if ( pcRowsObtained )
		*pcRowsObtained = 0;

    // Check validity of arguments.
    if ( pcRowsObtained == NULL || prghRows == NULL )
        return ResultFromScode( E_INVALIDARG );

    // No-op case always succeeds.
    if ( cRows == 0 )
        return ResultFromScode( S_OK );

    // This implementation doesn't support scrolling backward.
    if ( cRows < 0 )
        return ResultFromScode( DB_E_CANTFETCHBACKWARDS );

    // This implementation doesn't support scrolling backward.
    if ( lRowOffset < 0 )
        return ResultFromScode( DB_E_CANTSCROLLBACKWARDS );

    // Get the value of the DBPROP_CANHOLDROWS property
	rgPropertyIDSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropertyIDSets[0].rgPropertyIDs	= rgPropId;
	rgPropertyIDSets[0].cPropertyIDs	= 1;
	rgPropId[0]							= DBPROP_CANHOLDROWS;

    m_pObj->m_pUtilProp->GetProperties( 
									PROPSET_ROWSET,
									1, 
									rgPropertyIDSets,
									&cPropertySets,
									&prgPropertySets );

	// Get the Prompt value
	if( V_BOOL(&prgPropertySets->rgProperties->vValue) == VARIANT_TRUE )
		fCanHoldRows = TRUE;

	// Free the memory
	SAFE_FREE(prgPropertySets[0].rgProperties);	
	SAFE_FREE(prgPropertySets);

    // Are there any unreleased rows?
    if( ((m_pObj->m_prowbitsIBuffer)->ArrayEmpty() != S_OK) && (!fCanHoldRows) )
        return ResultFromScode( DB_E_ROWSNOTRELEASED );

    // Is the cursor fully materialized (end-of-cursor condition)?
    if (m_pObj->m_dwStatus & STAT_ENDOFCURSOR)
        return ResultFromScode( DB_S_ENDOFROWSET );

    assert( m_pObj->m_rgbRowData );
    if (FAILED( m_pObj->Rebind((BYTE *) m_pObj->GetRowBuff( m_pObj->m_irowMin, TRUE ))))
        return ResultFromScode( E_FAIL );

    //
    // Fetch Data
    //
    if (lRowOffset)
        {
        // Calculate the new position
        m_pObj->m_irowFilePos += lRowOffset;

        // Check if skip causes END_OF_ROWSET
        if (m_pObj->m_irowFilePos > m_pObj->m_pFileio->GetRowCnt() ||
            m_pObj->m_irowFilePos <= 0)
            {
            m_pObj->m_dwStatus |= STAT_ENDOFCURSOR;
            return ResultFromScode( DB_S_ENDOFROWSET );
            }
        }

	if (FAILED( hr = GetNextSlots( m_pObj->m_pIBuffer, (ULONG)cRows, &cRowFirst )))
        return hr;

    cSlotAlloc = (ULONG)cRows;

    for (irow =1; irow <= cRows; irow++)
        {
		// Setup the row
		prowbuff = m_pObj->GetRowBuff( cRowFirst + irow - 1, TRUE );
		memset(prowbuff->cdData, 0, m_pObj->m_cbRowSize);
		if (FAILED( m_pObj->Rebind((BYTE *) prowbuff)))
			return ResultFromScode( E_FAIL );

		// Get the Data from the File into the row buffer
        if (S_FALSE == ( hr = m_pObj->m_pFileio->Fetch( m_pObj->m_irowFilePos + irow )))
            {
            m_pObj->m_dwStatus |= STAT_ENDOFCURSOR;
            break;
            }
        else
            {
            if (FAILED( hr ))
                return ResultFromScode( E_FAIL );
            }
        }

    cRowsTmp = (ULONG)(irow - 1); //Irow will be +1 because of For Loop
    m_pObj->m_irowLastFilePos = m_pObj->m_irowFilePos;
    m_pObj->m_irowFilePos += cRowsTmp;


    //
    // Through fetching many rows of data
    //
    // Allocate row handles for client.
    // Note that we need to use IMalloc for this.
    //
    // Should only malloc cRowsTmp, instead of cRows.
    //
    // Modified to use IMalloc.
    // Should malloc cRows, since client will assume it's that big.
    //

    *pcRowsObtained = cRowsTmp;
    
	if ( *prghRows == NULL && cRowsTmp )
        *prghRows = (HROW *) PROVIDER_ALLOC( cRows*sizeof( HROW ));

    if ( *prghRows == NULL  && cRowsTmp )
        return ResultFromScode( E_OUTOFMEMORY );

    //
    // Fill in the status information: Length, IsNull
    // May be able to wait until first call to GetData,
    // but have to do it sometime.
    //
    // Suggest keeping an array of structs of accessor info.
    // One element is whether accessor requires any status info or length info.
    // Then we could skip this whole section.
    //
    // Added check for cRowsTmp to MarkRows call.
    // Don't want to call if cRowsTmp==0.
    // (Range passed to MarkRows is inclusive, so can't specify marking 0 rows.)
    //
    // Note that SetSlots is a CBitArray member function -- not an IBuffer function.
    //
    // Bits are [0...n-1], row handles are [1...n].
    //
    // Cleanup. Row handles, bits, indices are the same [m....(m+n)], where m is some # >0,
    //
    // Added row-wise reference counts and cursor-wise reference counts.
    //

    // Set row handles, fix data length field and compute data status field.//
    m_pObj->m_cRows   = cRowsTmp;
    cRowLast = cRowFirst + cRowsTmp - 1;

    // Cleanup extra slots where no hRow actually was put..
    //  ** Because of less rows than asked for
    //  ** Because of temporary for for data transfer.
    if (cSlotAlloc > (cRowsTmp))
        if (FAILED( hr = ReleaseSlots( m_pObj->m_pIBuffer, cRowFirst + cRowsTmp, (cSlotAlloc - cRowsTmp))))
            return hr;

    for (irow = (LONG) (cRowFirst), ih =0; irow <= (LONG) cRowLast; irow++, ih++)
        {
        // Increment the rows-read count,
        // then store it as the bookmark in the very first DWORD of the row.
        prowbuff = m_pObj->GetRowBuff( irow, TRUE );

        // Insert the bookmark and its row number (from 1...n) into a hash table.
        // This allows us to quickly determine the presence of a row in mem, given the bookmark.
        // The bookmark is contained in the row buffer, at the very beginning.
        // Bookmark is the row number within the entire result set [1...num_rows_read].

        // This was a new Bookmark, not in memory,
        // so return to user (in *prghRows) the hRow we stored.
        prowbuff->ulRefCount++;
        prowbuff->pbBmk = /*(BYTE*)*/ m_pObj->m_irowLastFilePos + ih + 1;
        m_pObj->m_ulRowRefCount++;

        (*prghRows)[ih] = (HROW) ( irow );
        }

    if (m_pObj->m_dwStatus & STAT_ENDOFCURSOR)
        return ResultFromScode( DB_S_ENDOFROWSET );
    else
        return ResultFromScode( S_OK );
}



// CImpIRowset::ReleaseRows ---------------------------------------
//
// @mfunc Releases row handles
//
// @rdesc	Returns one of the following values:
// 			@flag S_OK 						| success
// 			@flag DB_S_ERRORSOCCURRED		| some elements of rghRows were invalid
// 			@flag DB_E_ERRORSOCCURRED		| all elements of rghRows were invalid
// 			@flag E_INVALIDARG 				| rghRows was a NULL pointer and crow > 0
//
STDMETHODIMP CImpIRowset::ReleaseRows
    (
    DBCOUNTITEM		cRows,          //@parm IN | Number of rows to release
    const HROW		rghRows[],      //@parm IN | Array of handles of rows to be released
	DBROWOPTIONS	rgRowOptions[],	//@parm IN | Additional Options
	DBREFCOUNT		rgRefCounts[],	//@parm OUT | array of ref counts of released rows
	DBROWSTATUS		rgRowStatus[]	//@parm OUT | status array of for input rows
    )
{
    HRESULT hr			  = S_OK;
    DBCOUNTITEM chRow	  = 0L;
    DBCOUNTITEM	cErrors	  = 0L;
    ROWBUFF     *pRowBuff = NULL;

    // check params
    if ( cRows && !rghRows )
        return ResultFromScode( E_INVALIDARG );

    while ( chRow < cRows )
	{
        // check the row handle
		hr = (m_pObj->m_prowbitsIBuffer)->IsSlotSet((ULONG) rghRows[chRow]);
        if ( (hr == S_OK) && (m_pObj->m_ulRowRefCount) &&
			 (pRowBuff=m_pObj->GetRowBuff((DBCOUNTITEM) rghRows[chRow], TRUE)) && 
			 (pRowBuff->ulRefCount) )
		{
            // Found valid row, so decrement reference counts.
            // (Internal error for refcount to be 0 here, since slot set.)
            --pRowBuff->ulRefCount;
            --m_pObj->m_ulRowRefCount;

			// stuff new refcount into caller's array
			if ( rgRefCounts )
				rgRefCounts[chRow] = pRowBuff->ulRefCount;

			if ( rgRowStatus )
				rgRowStatus[chRow] = DBROWSTATUS_S_OK;

            if ( pRowBuff->ulRefCount == 0 )
                ReleaseSlots( m_pObj->m_pIBuffer, (ULONG) rghRows[chRow], 1 );
		}
        else
		{
            // It is an error for client to try to release a row
            // for which "IsSetSlot" is false.  Client gave us an invalid handle.
            // Ignore it (we can't release it...) and report error when done.
			if ( rgRefCounts )
				rgRefCounts[chRow] = 0;

			if ( rgRowStatus )
				rgRowStatus[chRow] = DBROWSTATUS_E_INVALID;

            ++ cErrors;
		}

		chRow++;
	}

	// If everything went OK except errors in rows use DB_S_ERRORSOCCURRED.
	return cErrors ? ( cErrors < cRows ) ? 
			ResultFromScode( DB_S_ERRORSOCCURRED ) : 
		 	ResultFromScode( DB_E_ERRORSOCCURRED ) : 
		 	ResultFromScode( S_OK );
}






// CImpIRowset::ResartPosition ---------------------------------------
//
// @mfunc Repositions the next fetch position to the start of the rowset
//
// - all rows must be released before calling this method
// - it is not expensive to Restart us, because we are from a single table
//
//
// @rdesc	Returns one of the following values:
//      @flag S_OK                  | Method Succeeded
//      @flag DB_E_ROWSNOTRELEASED  | All HROWs must be released before calling
//
STDMETHODIMP CImpIRowset::RestartPosition
    (
    HCHAPTER    hChapter        //@parm IN | The Chapter handle.
    )
{    
	// make sure all rows have been released
	// Fail even if CANHOLDROWS is true
    if( ((m_pObj->m_prowbitsIBuffer)->ArrayEmpty() != S_OK) )
        return ResultFromScode( DB_E_ROWSNOTRELEASED );

    // set "next fetch" position to the start of the rowset
	m_pObj->m_irowFilePos = 0;

    // clear "end of cursor" flag
    m_pObj->m_dwStatus &= ~STAT_ENDOFCURSOR;

    return ResultFromScode( S_OK );
}


// CImpIRowset::AddRefRows --------------------------------------------------
//
// @mfunc Adds a reference count to an existing row handle
//
// @rdesc	Returns one of the following values:
// 			@flag S_OK 						| success
// 			@flag DB_S_ERRORSOCCURRED		| some elements of rghRows were invalid
// 			@flag DB_E_ERRORSOCCURRED		| all elements of rghRows were invalid
// 			@flag E_INVALIDARG 				| rghRows was a NULL pointer and crow > 0

STDMETHODIMP  CImpIRowset::AddRefRows
    (
    DBCOUNTITEM     cRows,          // @parm IN     | Number of rows to refcount
    const HROW      rghRows[],      // @parm IN     | Array of row handles to refcount
    DBREFCOUNT		rgRefCounts[],  // @parm OUT    | Array of refcounts
    DBROWSTATUS     rgRowStatus[]   // @parm OUT    | Array of row status
    )
{
    HRESULT hr			  = S_OK;
    DBCOUNTITEM chRow	  = 0L;
    DBCOUNTITEM	cErrors	  = 0L;
    ROWBUFF     *pRowBuff = NULL;

    // check params
    if ( cRows && !rghRows )
        return ResultFromScode( E_INVALIDARG );

    // for each of the HROWs the caller provided...
    for (chRow = 0; chRow < cRows; chRow++)
	{
        // check the row handle
		if( ((m_pObj->m_prowbitsIBuffer)->IsSlotSet((ULONG) rghRows[chRow]) == S_OK) &&
			(pRowBuff=m_pObj->GetRowBuff((DBCOUNTITEM) rghRows[chRow], TRUE )) &&
			(m_pObj->m_pFileio->IsDeleted((DBBKMARK) pRowBuff->pbBmk) != S_OK) )
		{
			// bump refcount
			pRowBuff = m_pObj->GetRowBuff((DBCOUNTITEM) rghRows[chRow], TRUE );
			assert( pRowBuff->ulRefCount != 0 );
			assert( m_pObj->m_ulRowRefCount != 0 );
			++pRowBuff->ulRefCount;
			++m_pObj->m_ulRowRefCount;

			// stuff new refcount into caller's array
			if ( rgRefCounts )
				rgRefCounts[chRow] = pRowBuff->ulRefCount;

			if ( rgRowStatus )
				rgRowStatus[chRow] = DBROWSTATUS_S_OK;
		}
		else
		{
			if ( rgRefCounts )
				rgRefCounts[chRow] = 0;

			if ( rgRowStatus )
			{
				if ( pRowBuff && m_pObj->m_pFileio->IsDeleted((DBBKMARK) pRowBuff->pbBmk) == S_OK )
					rgRowStatus[chRow] = DBROWSTATUS_E_DELETED;
				else
					rgRowStatus[chRow] = DBROWSTATUS_E_INVALID;
			}

            ++ cErrors;
		}
	}

	// If everything went OK except errors in rows use DB_S_ERRORSOCCURRED.
	return cErrors ? ( cErrors < cRows ) ? 
			ResultFromScode( DB_S_ERRORSOCCURRED ) : 
		 	ResultFromScode( DB_E_ERRORSOCCURRED ) : 
		 	ResultFromScode( S_OK );
}

