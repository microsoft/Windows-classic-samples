//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module IROWIDEN.CPP | IRowsetIdentity interface implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

//  IRowsetIdentity specific interface methods

// CImpIRowsetIdentity::IsSameRow --------------------------------------------------
//
// @mfunc	Compares two row handles to see if they refer to the same row instance.
//
// @rdesc HRESULT
//      @flag S_FALSE                | Method Succeeded but did not refer to the same row
//      @flag S_OK					 | Method Succeeded
//      @flag DB_E_BADROWHANDLE      | Invalid row handle given
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIRowsetIdentity::IsSameRow
(
	HROW hThisRow,	//@parm IN | The handle of an active row
	HROW hThatRow	//@parm IN | The handle of an active row
)
{
	//
	// Asserts
    //
	assert(m_pObj);
	assert(m_pObj->m_prowbitsIBuffer);
	assert(m_pObj->m_pFileio);

    //
	// Check validity of input handles
    //
	if( m_pObj->m_prowbitsIBuffer->IsSlotSet((ULONG)hThisRow) != S_OK ||
		m_pObj->m_prowbitsIBuffer->IsSlotSet((ULONG)hThatRow) != S_OK )
		return (DB_E_BADROWHANDLE);

    //
	// Obtain a pointers to corresponding row buffers.
    //
	PROWBUFF prowbuffThis = m_pObj->GetRowBuff((DBCOUNTITEM)hThisRow, TRUE);
	PROWBUFF prowbuffThat = m_pObj->GetRowBuff((DBCOUNTITEM)hThatRow, TRUE);

    //
	// Check to see if the row is Deleted
    //
	if( m_pObj->m_pFileio->IsDeleted((DBBKMARK) prowbuffThis->pbBmk) == S_OK ||
		m_pObj->m_pFileio->IsDeleted((DBBKMARK) prowbuffThat->pbBmk) == S_OK )
		return (DB_E_DELETEDROW);
	
    //
	// Check for a released row handle (most likely). 
    //
	return (hThisRow == hThatRow) ? S_OK : S_FALSE;
}

