//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module PERSIST.CPP | IPersist interface implementation
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------

// CImpIPersist::GetClassID --------------------------------------------------
//
// @mfunc Get the CLSID of the DSO.
//
// @rdesc HRESULT
//      @flag S_OK                  | The method succeeded.
//      @flag E_FAIL                | Provider-specific error.
//
STDMETHODIMP CImpIPersist::GetClassID
( 
	CLSID *pClassID 
)
{
    //
    // Check in-params and NULL out-params in case of error
    //
	if( !pClassID )
		return (E_FAIL);
	
	memcpy(pClassID, &CLSID_SampProv, sizeof(CLSID));
	return (S_OK);
}
