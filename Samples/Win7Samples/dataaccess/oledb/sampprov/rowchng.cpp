//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ROWCHNG.CPP | IRowsetChange interface implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------




//  IRowsetChange specific methods

// CImpIRowsetChange::SetData ------------------------------------------------
//
// @mfunc Sets new data values into fields of a row.
//
// @rdesc HRESULT
//      @flag S_OK                   | The method succeeded
//      @flag E_OUTOFMEMORY          | Out of memory
//      @flag DB_E_BADACCESSORHANDLE | Bad accessor handle
//      @flag DB_E_READONLYACCESSOR  | Tried to write through a read-only accessor
//      @flag DB_E_BADROWHANDLE      | Invalid row handle
//      @flag E_INVALIDARG           | pData was NULL
//      @flag E_FAIL                 | Provider-specific error
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIRowsetChange::SetData
    (
    HROW        hRow,       //@parm IN | Handle of the row in which to set the data
    HACCESSOR   hAccessor,  //@parm IN | Handle to the accessor to use
    void*		pData       //@parm IN | Pointer to the data
    )
{
    DBORDINAL       cCols;
	DBCOUNTITEM		cBind;
    BYTE*           pbProvRow;
    HRESULT         hr;
    DBCOUNTITEM     cBindings;
    DBBINDING*      pBinding;
    DBCOUNTITEM     cErrorCount;
    DBTYPE          dwSrcType;
    DBTYPE          dwDstType;
    DWORD           dwPart;
    ULONG           dwSrcLength;
    DWORD           dwSrcStatus;
    void*           pSrc;
    void*           pDst;
    DBLENGTH*		pdwDstLength;
    DBLENGTH        cbDstMaxLength;
    DWORD*          pdwDstStatus;
    PCOLUMNDATA     pColumnData;
    PACCESSOR       paccessor = NULL;
    BYTE*           rgbRowDataSave = NULL;
	DBCOLUMNINFO *	rgdbcolumninfo = NULL;

    // Is row handle right?
    assert( m_pObj->m_prowbitsIBuffer );
    if( (m_pObj->m_prowbitsIBuffer)->IsSlotSet( (ULONG)hRow ) != S_OK )
        return( DB_E_BADROWHANDLE );

    // Check the Accessor Handle
    assert( m_pObj->m_pextbufferAccessor );
	if( FAILED(m_pObj->m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &paccessor)))
        return( DB_E_BADACCESSORHANDLE );
        
    // Ensure a source of data.
    assert( paccessor );
    if( paccessor->cBindings && !pData )
        return( E_INVALIDARG );

    pbProvRow = (BYTE *)(m_pObj->GetRowBuff((DBCOUNTITEM) hRow, TRUE ));

    // Is row handle deleted?
    if( m_pObj->m_pFileio->IsDeleted((DBBKMARK) ((PROWBUFF) pbProvRow)->pbBmk ) == S_OK )
        return( DB_E_DELETEDROW );

    rgbRowDataSave = (BYTE *) malloc( m_pObj->m_cbTotalRowSize );
    if( !rgbRowDataSave )
        return( E_OUTOFMEMORY );

    // Save the row.
    memcpy( rgbRowDataSave, pbProvRow, m_pObj->m_cbTotalRowSize );

    cBindings = paccessor->cBindings;
    pBinding  = paccessor->rgBindings;

    // Apply accessor to data.
	rgdbcolumninfo = m_pObj->m_pFileio->GetColInfo();
    for (cBind = 0, cErrorCount = 0; cBind < cBindings; cBind++)
	{
        cCols = pBinding[cBind].iOrdinal;

        // make sure column number is in range
        if( cCols < 1 || cCols > m_pObj->m_cCols )
            return( DB_E_BADORDINAL );

		pColumnData    = m_pObj->m_pFileio->GetColumnData(cCols, (ROWBUFF *)pbProvRow);

        dwSrcType      = pBinding[cBind].wType;
        pDst           = &(pColumnData->bData);
        pdwDstLength   = &(pColumnData->uLength);
        pdwDstStatus   = &(pColumnData->dwStatus);
        cbDstMaxLength = pBinding[cBind].cbMaxLen;

        dwPart         = pBinding[cBind].dwPart;
        dwDstType      = rgdbcolumninfo[cCols].wType;

        // Get the Length
		if( dwPart & DBPART_LENGTH )
			dwSrcLength = *(ULONG *)((BYTE*)pData + pBinding[cBind].obLength);
		else
			dwSrcLength = 0;

        // Get the Status
        if( dwPart & DBPART_STATUS )
			dwSrcStatus = *(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus);
		else
			dwSrcStatus = DBSTATUS_S_OK;

        // Check the Status for DBSTATUS_S_DEFAULT
		if( dwSrcStatus == DBSTATUS_S_DEFAULT )
			dwSrcStatus = DBSTATUS_S_ISNULL;

        // Check the Status for DBSTATUS_S_IGNORE
		if( dwSrcStatus == DBSTATUS_S_IGNORE || 
			dwSrcStatus == DBSTATUS_S_ISNULL )
			continue;

        // Check the Status of the value being sent in
		if( dwSrcStatus != DBSTATUS_S_OK ) 
		{
			*(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus) = DBSTATUS_E_BADSTATUS;
			cErrorCount++;
			continue;
		}

        // Get the Value
		if( (dwPart & DBPART_VALUE) == 0 )
		{
            if( dwPart & DBPART_STATUS )
				*(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus) = DBSTATUS_E_UNAVAILABLE;

			cErrorCount++;
			continue;
		}
        else 
		{
            pSrc = (void *) ((BYTE*) pData + pBinding[cBind].obValue);
		}

		// Check to see if we have a DC
		if( !g_pIDataConvert )
			return( E_FAIL );

		hr = g_pIDataConvert->DataConvert(
				dwSrcType,
                dwDstType,
                dwSrcLength,
                pdwDstLength,
                pSrc,
                pDst,
                cbDstMaxLength,
                dwSrcStatus,
                pdwDstStatus,
                pBinding[cBind].bPrecision,	// bPrecision for conversion to DBNUMERIC
				pBinding[cBind].bScale,		// bScale for conversion to DBNUMERIC
				(DBDATACONVERT_SETDATABEHAVIOR | 
				 (!dwSrcLength ? DBDATACONVERT_LENGTHFROMNTS : 0)));
        
        if( dwPart & DBPART_STATUS )
			*(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus) = *pdwDstStatus;

		// fatal error
		if( FAILED( hr ) && hr != DB_E_ERRORSOCCURRED )
            return hr;
        
		// rounding or truncation or can't coerce
		if( hr != S_OK )
			cErrorCount++;
	}

    // Carry out the update.
    if( FAILED( m_pObj->m_pFileio->UpdateRow((DBBKMARK) ((PROWBUFF) pbProvRow)->pbBmk, pbProvRow, UPDATE )) )
	{
        // Restore the row to its previous state
        memcpy( pbProvRow, rgbRowDataSave, m_pObj->m_cbTotalRowSize );
		free( rgbRowDataSave );
        return( E_FAIL );
	}

    free( rgbRowDataSave );

	// If everything went OK except errors in rows use DB_S_ERRORSOCCURRED.
	return cErrorCount ? ( cErrorCount < cBindings ) ? 
		( DB_S_ERRORSOCCURRED ) : ( DB_E_ERRORSOCCURRED ) : ( S_OK );
}



// CImpIRowsetChange::DeleteRows ---------------------------------------
//
// @mfunc Deletes rows from the provider.  If Errors on individual rows
// occur, the DBERRORINFO array is updated to reflect the error and S_FALSE
// is returned instead of S_OK.
//
// @rdesc HRESULT indicating the status of the method
//      @Flag S_OK                  | All row handles deleted
//      @Flag DB_S_ERRORSOCCURRED   | Some, but not all, row handles deleted
//      @Flag E_INVALIDARG          | Arguments did not match spec.
//      @Flag E_OUTOFMEMORY         | Could not allocated error array
//
STDMETHODIMP CImpIRowsetChange::DeleteRows
    (
		HCHAPTER		hChapter,       //@parm IN  | The Chapter handle.
		DBCOUNTITEM     cRows,			//@parm IN	| Number of rows to delete
		const HROW      rghRows[],		//@parm IN	| Array of handles to delete
		DBROWSTATUS		rgRowStatus[]	//@parm OUT | Error information
    )
{
    DBCOUNTITEM	chRow       = 0L;
    DBCOUNTITEM cErrors     = 0L;
    BYTE*		pbProvRow	= NULL;

    // If No Row handle, just return.
    if( cRows == 0 )
        return( S_OK );

    // Check for Invalid Arguments
    if ( !rghRows )
        return( E_INVALIDARG );

    // Process row handles
    while (chRow < cRows)
    {
		if( rgRowStatus )
			rgRowStatus[chRow] = DBROWSTATUS_S_OK;

        // Is row handle valid
        if( (m_pObj->m_prowbitsIBuffer)->IsSlotSet( (ULONG)rghRows[chRow]) != S_OK )
        {
            // Log Error
			if( rgRowStatus )
				rgRowStatus[chRow]= DBROWSTATUS_E_INVALID;

            cErrors++;
            chRow++;
            continue;
		}

        // Get RowBuffer to look at which row this applies to
        pbProvRow = (BYTE *) (m_pObj->GetRowBuff((DBCOUNTITEM) rghRows[chRow], TRUE ));

        // Has row already been deleted
        // S_OK means deleted
        if( m_pObj->m_pFileio->IsDeleted((DBBKMARK) ((PROWBUFF) pbProvRow)->pbBmk ) == S_OK )
        {
			if( rgRowStatus )
				rgRowStatus[chRow] = DBROWSTATUS_E_DELETED;
            cErrors++;
            chRow++;
            continue;
        }

        // Delete the Row,
        if( m_pObj->m_pFileio->DeleteRow((DBBKMARK) ((PROWBUFF) pbProvRow)->pbBmk) != S_OK )
        {
			// Some better decision as to what rowstatus to set could be done here..
			if( rgRowStatus )
				rgRowStatus[chRow] = DBROWSTATUS_E_FAIL;
            cErrors++;
            chRow++;
            continue;
        }

		// Delete worked correctly
		chRow++;

	} //while


	// If everything went OK except errors in rows use DB_S_ERRORSOCCURRED.
	return cErrors ? ( cErrors < cRows ) ? 
		( DB_S_ERRORSOCCURRED ) : ( DB_E_ERRORSOCCURRED ) : ( S_OK );
}

// CImpIRowsetChange::InsetRow  --------------------------------------------
//
// @mfunc Insert row into provider
//
//  Returns:   S_OK                    if data changed successfully
//             E_FAIL                  if Catch all (NULL pData, etc.)
//             E_INVALIDARG            if pcErrors!=NULL and paErrors==NULL
//             E_OUTOFMEMORY           if output error array couldn't be allocated
//             DB_E_BADACCESSORHANDLE  if invalid accessor
//
STDMETHODIMP    CImpIRowsetChange::InsertRow
    (
		HCHAPTER    hChapter,        //@parm IN | The Chapter handle.
		HACCESSOR	hAccessor,
		void*		pData,
		HROW*		phRow
	)
{
    DBORDINAL       cCols;
	DBCOUNTITEM		cBind;
    BYTE*           pbProvRow;
    HRESULT         hr;
    ULONG			irow;
    DBCOUNTITEM     cBindings;
    DBBINDING*      pBinding;
    DBCOUNTITEM     cErrorCount;
    DBTYPE          dwSrcType;
    DBTYPE          dwDstType;
    void*           pSrc;
    void*           pDst;
    ULONG           dwSrcLength;
    DBLENGTH*		pdwDstLength;
    DBLENGTH        cbDstMaxLength;
    DWORD           dwSrcStatus;
    DWORD*          pdwDstStatus;
    DWORD           dwPart;
    PCOLUMNDATA     pColumnData;
    PACCESSOR       paccessor = NULL;
    BYTE*           rgbRowDataSave = NULL;
	BOOL			fCanHoldRows = FALSE;
	DBPROPIDSET		rgPropertyIDSets[1];
	ULONG			cPropertySets;
	DBPROPSET*		prgPropertySets;
	DBPROPID		rgPropId[1];
	DBCOLUMNINFO *	rgdbcolumninfo = NULL;

	// Initialize values
	if( phRow )
		*phRow = NULL;

    // Check the Accessor Handle
	assert( m_pObj->m_pextbufferAccessor );
	if( FAILED( m_pObj->m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &paccessor)) )
        return( DB_E_BADACCESSORHANDLE );
        
    // Ensure a source of data.
    assert( paccessor );
    if( paccessor->cBindings && !pData )
        return( E_INVALIDARG );

    // Get the value of the DBPROP_CANHOLDROWS property
	rgPropertyIDSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropertyIDSets[0].rgPropertyIDs	= rgPropId;
	rgPropertyIDSets[0].cPropertyIDs	= 1;
	rgPropId[0]							= DBPROP_CANHOLDROWS;

    assert( m_pObj->m_pUtilProp );
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
        return( DB_E_ROWSNOTRELEASED );

    if( FAILED( hr = GetNextSlots( m_pObj->m_pIBuffer, 1, &irow )) )
        return hr;

    if( FAILED( m_pObj->Rebind((BYTE *) m_pObj->GetRowBuff( irow, TRUE ))) )
        return( E_FAIL );

    // Get the rowbuffer and set the new bookmark
	pbProvRow = (BYTE *) (m_pObj->GetRowBuff( irow, TRUE ));
    ((PROWBUFF) pbProvRow)->ulRefCount++;
    ((PROWBUFF) pbProvRow)->pbBmk = /*(BYTE*)*/ m_pObj->m_pFileio->GetRowCnt()+1;
    m_pObj->m_ulRowRefCount++;

    cBindings = paccessor->cBindings;
    pBinding  = paccessor->rgBindings;

	// NULL Accessor (set Status to NULL)
	if( !cBindings )
	{
		for (cCols = 1; cCols <= m_pObj->m_cCols; cCols++)
		{
			pColumnData = m_pObj->m_pFileio->GetColumnData(cCols, (ROWBUFF *) pbProvRow);
			pColumnData->dwStatus = DBSTATUS_S_ISNULL;			
		}
	}

    // Apply accessor to data.
	rgdbcolumninfo = m_pObj->m_pFileio->GetColInfo();
    for (cBind = 0, cErrorCount = 0; cBind < cBindings; cBind++)
	{
        cCols = pBinding[cBind].iOrdinal;

        // make sure column number is in range
        if( cCols < 1 || cCols > m_pObj->m_cCols )
            return( DB_E_BADORDINAL );

		pColumnData = m_pObj->m_pFileio->GetColumnData(cCols, (ROWBUFF *)pbProvRow);

        dwSrcType      = pBinding[cBind].wType;
        pDst           = &(pColumnData->bData);
        pdwDstLength   = &(pColumnData->uLength);
        pdwDstStatus   = &(pColumnData->dwStatus);
        cbDstMaxLength = pBinding[cBind].cbMaxLen;

        dwPart         = pBinding[cBind].dwPart;
        dwDstType      = rgdbcolumninfo[cCols].wType;

        // Get the Length
		if( dwPart & DBPART_LENGTH )
			dwSrcLength = *(ULONG *)((BYTE*)pData + pBinding[cBind].obLength);
		else
			dwSrcLength = 0;

        // Get the Status
        if( dwPart & DBPART_STATUS )
			dwSrcStatus = *(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus);
		else
			dwSrcStatus = DBSTATUS_S_OK;

        // Check the Status for DBSTATUS_S_DEFAULT
		if( dwSrcStatus == DBSTATUS_S_DEFAULT )
			dwSrcStatus = DBSTATUS_S_ISNULL;

        // Check the Status for DBSTATUS_S_IGNORE
		if( dwSrcStatus == DBSTATUS_S_IGNORE || 
			dwSrcStatus == DBSTATUS_S_ISNULL )
			continue;

        // Check the Status of the value being sent in
		if( dwSrcStatus != DBSTATUS_S_OK ) 
		{
			*(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus) = DBSTATUS_E_BADSTATUS;
			cErrorCount++;
			continue;
		}

        // Get the Value
		if( (dwPart & DBPART_VALUE) == 0 )
		{
            if( dwPart & DBPART_STATUS )
				*(ULONG *)((BYTE*)pData + pBinding[cBind].obStatus) = DBSTATUS_E_UNAVAILABLE;

			cErrorCount++;
			continue;
		}
        else 
		{
            pSrc = (void *) ((BYTE*) pData + pBinding[cBind].obValue);
		}

		// Check to see if we have a DC
		if( !g_pIDataConvert )
			return( E_FAIL );

        hr = g_pIDataConvert->DataConvert(
				dwSrcType,
                dwDstType,
                dwSrcLength,
                pdwDstLength,
                pSrc,
                pDst,
                cbDstMaxLength,
                dwSrcStatus,
                pdwDstStatus,
                pBinding[cBind].bPrecision,	// bPrecision for conversion to DBNUMERIC
				pBinding[cBind].bScale,		// bScale for conversion to DBNUMERIC
				(DBDATACONVERT_SETDATABEHAVIOR | 
				 (!dwSrcLength ? DBDATACONVERT_LENGTHFROMNTS : 0)));
        
		// fatal error
		if( FAILED( hr ) )
            return hr;
        
		// rounding or truncation or can't coerce
		if( hr != S_OK )
            cErrorCount++;
	}

	// If all bindings are bad and not a NULL Accessor
	if( !cBindings || cErrorCount < cBindings )
	{
		// Carry out the insert.
		if( FAILED(m_pObj->m_pFileio->UpdateRow((DBBKMARK) ((PROWBUFF) pbProvRow)->pbBmk, pbProvRow, INSERT )) )
			return( E_FAIL );

		// Set the RowHandle
		if( phRow )
			*phRow = irow;
	}

	// If everything went OK except errors in rows use DB_S_ERRORSOCCURRED.
	return cErrorCount ? ( cErrorCount < cBindings ) ? 
		( DB_S_ERRORSOCCURRED ) : ( DB_E_ERRORSOCCURRED ) : ( S_OK );
}

