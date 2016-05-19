//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DBPROP.CPP | IDBProperties and IDBInfo interface implementations
//
//

// Includes ------------------------------------------------------------------
#include "headers.h"

// Code ----------------------------------------------------------------------

//  IDBInfo specific interface methods
// CImpIDBInfo::GetKeywords  --------------------------------------------
//
// @mfunc Returns information about keywords used in text commands
//
// @rdesc HRESULT
//		@flag S_OK | Keywords successfully returned, NULL because no keywords
//      @flag E_INVALIDARG  | ppwszKeywords was NULL
//      @flag E_UNEXPECTED	| Can not be called unless initialized
//
STDMETHODIMP    CImpIDBInfo::GetKeywords
    (
	    LPWSTR*	ppwszKeywords
    )
{
    //
	// Asserts
    //
    assert(m_pObj);

    //
    // Check in-params and NULL out-params in case of error
    //
    if( !ppwszKeywords )
		return E_INVALIDARG;

	*ppwszKeywords = NULL;

    //
	// Check to see if initialized
    //
	if( !m_pObj->m_fDSOInitialized )
		return E_UNEXPECTED;

    return S_OK;
}

// CImpIDBInfo::GetLiteralInfo  -----------------------------------------
//
// @mfunc Returns information about literals used in text command
//
// @rdesc HRESULT
//		@flag S_OK			| cLiterals was 0
//      @flag E_INVALIDARG  | cLiterals not equal to 0 and rgLiterals was NULL or
//							| pcLiteralInfo, prgLiteralInfo, or ppCharBuffer was NULL
//      @flag E_UNEXPECTED	| Can not be called unless initialized
//		@flag DB_E_ERRORSOCCURRED | None of the requested literals are supported
//
STDMETHODIMP    CImpIDBInfo::GetLiteralInfo
    (
	    ULONG           cLiterals,      //@parm IN | # of literals
		const DBLITERAL rgLiterals[],   //@parm IN | Array of literals
		ULONG*          pcLiteralInfo,  //@parm OUT | # of literals returned
		DBLITERALINFO** prgLiteralInfo, //@parm OUT | Array of info structures
		WCHAR**         ppCharBuffer    //@parm OUT | Buffer for characters
    )
{
    HRESULT	hr = DB_E_ERRORSOCCURRED;
	ULONG   ulIndex = 0;

    //
	// Asserts
    //
    assert(m_pObj);

    //
    // Check in-params and NULL out-params in case of error
    //
	if( pcLiteralInfo )
		*pcLiteralInfo = 0;
	
	if( prgLiteralInfo )
		*prgLiteralInfo = NULL;
	
	if( ppCharBuffer )
		*ppCharBuffer = NULL;

    //
	// Check input and output values pointers
    //
    if( cLiterals && !rgLiterals )
        return E_INVALIDARG;

    if( !pcLiteralInfo || !prgLiteralInfo || !ppCharBuffer )
        return E_INVALIDARG;
    
    //
	// Check to see if initialized
    //
	if( !m_pObj->m_fDSOInitialized )
		return E_UNEXPECTED;
	
    //
	// If cLiterals is 0, ignore rgLiterals
	// return all supported values
    //
	if( cLiterals == 0 ) 
		return S_OK;

    //
	// Allocate memory for DBLITERALINFO array
    //
    SAFE_ALLOC(*prgLiteralInfo, DBLITERALINFO, cLiterals);

    //
	// Process each of the DBLITERAL values that are in the restriction array
    //
	*pcLiteralInfo = cLiterals;

	for(ulIndex=0; ulIndex < cLiterals; ulIndex++)
	{
		(*prgLiteralInfo)[ulIndex].lt = rgLiterals[ulIndex];
		(*prgLiteralInfo)[ulIndex].fSupported = FALSE;
		(*prgLiteralInfo)[ulIndex].pwszLiteralValue = NULL;
		(*prgLiteralInfo)[ulIndex].pwszInvalidChars = NULL;
		(*prgLiteralInfo)[ulIndex].pwszInvalidStartingChars = NULL;
		(*prgLiteralInfo)[ulIndex].cchMaxLen = 0;
	}
	
CLEANUP:

	return hr;
}



//  IDBProperties specific interface methods

// CImpIDBProperties::GetPropertyInfo  -----------------------------------------
//
// @mfunc Returns information about rowset and data source properties supported
// by the provider
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcPropertyInfo or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP    CImpIDBProperties::GetPropertyInfo
    (
	    ULONG				cPropertySets,		//@parm IN  | # of properties
	    const DBPROPIDSET	rgPropertySets[],	//@parm IN  | Array of properties
	    ULONG*				pcPropertyInfoSets,	//@parm OUT | # of properties returned
	    DBPROPINFOSET**		prgPropertyInfoSets,//@parm OUT | Property values returned
		WCHAR**				ppDescBuffer		//@parm OUT	| Description Buffer
    )
{
    //
	// Asserts
    //
	assert(m_pObj);
    assert(m_pObj->m_pUtilProp);

    //
    // Just pass this call on to the utility object that manages our properties
    //
    return m_pObj->m_pUtilProp->GetPropertyInfo(
									m_pObj->m_fDSOInitialized,
									cPropertySets, 
									rgPropertySets,
									pcPropertyInfoSets, 
									prgPropertyInfoSets,
									ppDescBuffer );
}


// IDBProperties::GetProperties ----------------------------------------------------
//
// @mfunc Returns current settings of all properties in the FLAGS_DATASRCINF 
//			property group
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpIDBProperties::GetProperties
    (
	    ULONG				cPropertySets,		//@parm IN | count of restiction guids
		const DBPROPIDSET	rgPropertySets[],	//@parm IN | restriction guids
		ULONG*              pcProperties,		//@parm OUT | count of properties returned
		DBPROPSET**			prgProperties		//@parm OUT | property information returned
    )
{
    HRESULT hr = E_FAIL;

    //
	// Asserts
    //
	assert(m_pObj);
    assert(m_pObj->m_pUtilProp);

    //
    // Check in-params and NULL out-params in case of error
    //
	hr = m_pObj->m_pUtilProp->GetPropertiesArgChk(
									m_pObj->m_fDSOInitialized ? 
									PROPSET_DSOINIT : 
									PROPSET_DSO, 
									cPropertySets, 
									rgPropertySets, 
									pcProperties, 
									prgProperties );
	if( FAILED(hr) )
		return hr;

    //
    // Just pass this call on to the utility object that manages our properties
    //
    return m_pObj->m_pUtilProp->GetProperties(
									m_pObj->m_fDSOInitialized ? 
									PROPSET_DSOINIT : 
									PROPSET_DSO, 
									cPropertySets, 
									rgPropertySets,
									pcProperties, 
									prgProperties );
}


// CImpIDBProperties::SetProperties  --------------------------------------------
//
// @mfunc Set properties in the FLAGS_DATASRCINF property group
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | cProperties was not equal to 0 and rgProperties was NULL
//
STDMETHODIMP    CImpIDBProperties::SetProperties
    (
	ULONG		cPropertySets,		//@parm IN | Count of structs returned
	DBPROPSET	rgPropertySets[]    //@parm IN | Array of Properties
	)
{
    HRESULT hr = E_FAIL;

    //
	// Asserts
    //
	assert(m_pObj);
    assert(m_pObj->m_pUtilProp);

    //
	// No-op if cPropertySets is 0
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
    // Just pass this call on to the utility object that manages our properties
    //
    return m_pObj->m_pUtilProp->SetProperties(
									m_pObj->m_fDSOInitialized ? 
									PROPSET_DSOINIT : PROPSET_DSO, 
									cPropertySets, 
									rgPropertySets );
}


