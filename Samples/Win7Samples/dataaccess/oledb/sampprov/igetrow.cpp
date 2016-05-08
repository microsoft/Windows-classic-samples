//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module IGETROW.CPP | IGetRow interface implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

//  IGetRow specific interface methods

// CImpIGetRow::GetRowFromHROW -----------------------------------------------
//
// @mfunc	Returns a row object given an HROW
//
// @rdesc HRESULT
//      @flag S_OK					 | Method Succeeded
//		@flag E_INVALIDARG           | ppUnk was NULL
//		@flag E_OUTOFMEMORY          | Out of memory
//      @flag DB_E_BADROWHANDLE      | Invalid row handle given
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIGetRow::GetRowFromHROW
(
	IUnknown *	pIUnkOuter,
	HROW		hRow,
	REFIID		riid,
	IUnknown **	ppUnk
)
{
	HRESULT hr = S_OK;
	CRow* pCRow = NULL;

	if( NULL == ppUnk )
		return E_INVALIDARG;
	
	*ppUnk = NULL;
	
	if( pIUnkOuter && riid != IID_IUnknown )
		return DB_E_NOAGGREGATION;

	if( DB_NULL_HROW == hRow )
		return DB_E_BADROWHANDLE;
	
	//Create a CRow object
	pCRow = new CRow(pIUnkOuter);
	if( NULL == pCRow || !pCRow->FInit(m_pObj) )
		return E_OUTOFMEMORY;

	hr = pCRow->QueryInterface(riid, (void**)ppUnk);
	if( SUCCEEDED(hr) )
	{
		//Set the row handle to this object...
		hr = pCRow->SetRowHandle(hRow);
	}
	else
	{
		delete pCRow;
	}

	return hr;
}


// CImpIGetRow::GetURLFromHROW -----------------------------------------------
//
// @mfunc	Returns a row object given an HROW
//
// @rdesc HRESULT
//      @flag S_OK					 | Method Succeeded
//		@flag E_INVALIDARG           | ppwszURL was NULL
//      @flag DB_E_BADROWHANDLE      | Invalid row handle given
//      @flag OTHER                  | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIGetRow::GetURLFromHROW
(
	HROW		hRow,
	LPOLESTR *	ppwszURL
)
{
	size_t			cchLenURL = 0;
	ROWBUFF *		pRowBuff = NULL;
	static WCHAR	s_wszURLFmt[] = L"SampProv:Datasource=%s,File=%s,Row=%Id";

	assert( m_pObj );

	if( !ppwszURL )
		return E_INVALIDARG;

	*ppwszURL = NULL;

	if( DB_NULL_HROW == hRow ||
		m_pObj->m_prowbitsIBuffer->IsSlotSet( (ULONG)hRow ) != S_OK )
        return DB_E_BADROWHANDLE;

	pRowBuff = m_pObj->GetRowBuff( hRow, TRUE );
    assert( pRowBuff && pRowBuff->ulRefCount );

	if ( m_pObj->m_pFileio &&
		 m_pObj->m_pFileio->IsDeleted( (DBBKMARK) pRowBuff->pbBmk ) == S_OK )
		return DB_E_DELETEDROW;

	cchLenURL = wcslen(s_wszURLFmt) + (2 * MAX_PATH) + INT_DISPLAY_SIZE;
	*ppwszURL = (WCHAR *)PROVIDER_ALLOC( cchLenURL * sizeof(WCHAR));
	
	if( *ppwszURL )
	{
		StringCchPrintfW(*ppwszURL, cchLenURL, s_wszURLFmt, m_pObj->m_wszDataSourcePath,
					m_pObj->m_wszFilePath, (DBBKMARK) pRowBuff->pbBmk);

		return S_OK;
	}
	else
		return E_OUTOFMEMORY;
}
