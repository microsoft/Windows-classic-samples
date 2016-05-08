//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ROWINFO.CPP | IRowsetInfo interface implementation
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------


// IRowsetInfo specific methods

// CImpIRowsetInfo::GetReferencedRowset ----------------------------------------------------
//
// @mfunc Returns an interface pointer to the rowset to which the bookmark
// applies
//
// @rdesc HRESULT
//		@flag E_INVALIDARG		| ppReferencedRowset was NULL
//		@flag DB_E_BADORDINAL	| iOrdinal was greater than number of columns in rowset
//      @flag DB_E_NOTAREFERENCECOLUMN | This rowset does not support bookmarks
//
STDMETHODIMP CImpIRowsetInfo::GetReferencedRowset
(
    DBORDINAL	iOrdinal,			//@parm IN | Bookmark Column
    REFIID		riid,				//@parm IN | ID of the interface pointer to return
	IUnknown **	ppReferencedRowset	//@parm OUT | IRowset Interface Pointer
)
{
	//
	// Asserts
    //
	assert( m_pObj );

    //
    // Check in-params and NULL out-params in case of error
    //
	if( !ppReferencedRowset )
        return( E_INVALIDARG );

	*ppReferencedRowset = NULL;

    //
	// The oridinal was greater than the number of columns that we have.
    //
	if( iOrdinal == 0 || iOrdinal > m_pObj->m_cCols )
		return( DB_E_BADORDINAL );
   
    //
	// Since we don't support bookmarks, this will alway return an error
    //
    return( DB_E_NOTAREFERENCECOLUMN );
}



// CImpIRowsetInfo::GetProperties ----------------------------------------------------
//
// @mfunc Returns current settings of all properties supported by the rowset
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgProperties was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpIRowsetInfo::GetProperties
(
    const ULONG         cPropertySets,		//@parm IN | # of property sets
    const DBPROPIDSET	rgPropertySets[],	//@parm IN | Array of DBPROPIDSET
    ULONG*              pcProperties,		//@parm OUT | count of properties returned
    DBPROPSET**			prgProperties		//@parm OUT | property information returned
)
{
    HRESULT hr;

	//
	// Asserts
    //
	assert(m_pObj);
    assert(m_pObj->m_pUtilProp);

    //
    // Check in-params and NULL out-params in case of error
    //
	hr = m_pObj->m_pUtilProp->GetPropertiesArgChk(PROPSET_ROWSET,cPropertySets,
									rgPropertySets,pcProperties,prgProperties);
	if( FAILED(hr) )
		return hr;

    //
    // Just pass this call on to the utility object that manages our properties
    //
    return m_pObj->m_pUtilProp->GetProperties(PROPSET_ROWSET,cPropertySets,
									rgPropertySets,pcProperties,prgProperties);
}



// CImpIRowsetInfo::GetSpecification ---------------------------------------
//
// @mfunc Returns the interface pointer of the object that created the rowset
//
// @rdesc HRESULT
//      @flag S_OK          | Method Succeeded
//      @flag E_INVALIDARG  | Invalid parameters were specified
//
STDMETHODIMP CImpIRowsetInfo::GetSpecification
(
	REFIID   riid,              //@parm IN | Interface ID of the interface being queried for.
	IUnknown **ppSpecification  //@parm OUT | Pointer to interface that instantiated this object
)
{
    //
	// Asserts
    //
	assert(m_pObj);
	assert(m_pObj->m_pParentObj);
	assert(m_pObj->m_pParentObj->GetOuterUnknown());

    //
    // Check in-params and NULL out-params in case of error
    //
    if( !ppSpecification )
        return (E_INVALIDARG);

    *ppSpecification = NULL;

	return m_pObj->m_pParentObj->GetOuterUnknown()->QueryInterface(riid,(void**)ppSpecification);
}
