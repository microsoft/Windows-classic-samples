//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module CRTSESS.CPP | IDBCreateSession interface implementation
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------

// CImpIDBCreateSession::CreateSession ------------------------------------------------
//
// @mfunc Creates a new DB Session object from the DSO, and returns the
// requested interface on the newly created object.
//
// @rdesc HRESULT
//      @flag S_OK                  | The method succeeded.
//      @flag E_INVALIDARG          | ppDBSession was NULL
//      @flag DB_E_NOAGGREGATION    | pUnkOuter was not NULL (this object does not support
//                                    being aggregated)
//      @flag E_FAIL                | Provider-specific error. This provider can only create
//                                    one DBSession
//      @flag E_OUTOFMEMORY         | Out of memory
//      @flag E_NOINTERFACE         | Could not obtain requested interface on DBSession object
//
STDMETHODIMP CImpIDBCreateSession::CreateSession
(
    IUnknown*   pUnkOuter,  //@parm IN | Controlling IUnknown if being aggregated 
    REFIID      riid,       //@parm IN | The ID of the interface 
    IUnknown**  ppDBSession //@parm OUT | A pointer to memory in which to return the interface pointer
)
{
	//
	// Asserts
    //
	assert(m_pObj);

    //
    // Check in-params and NULL out-params in case of error
    //
    if( !ppDBSession )
        return (E_INVALIDARG);

    *ppDBSession = NULL;

    //
	// Check to see if the DSO is Uninitialized
    //
	if( !m_pObj->m_fDSOInitialized )
        return (E_UNEXPECTED);

    //
    // This Data Source object can only create 1 DBSession...
    //
    if( m_pObj->m_fDBSessionCreated )
        return (DB_E_OBJECTCREATIONLIMITREACHED);

    //
	// We do not allow any other iid than IID_IUnknown for aggregation
    //
	if( pUnkOuter && riid != IID_IUnknown )
		return (DB_E_NOAGGREGATION);

    //
    // Open a DBSession object
    //
    CDBSession* pDBSession = new CDBSession(pUnkOuter);

    if( !pDBSession || !pDBSession->FInit(m_pObj) )
	{
        SAFE_DELETE(pDBSession);
        return (E_OUTOFMEMORY);
	}

    //
    // Get requested interface pointer on DBSession
    //
    HRESULT hr=pDBSession->QueryInterface(riid, (void **)ppDBSession);

    if( FAILED(hr) ) 
	{
        SAFE_DELETE(pDBSession);
		return (hr);
	}

	m_pObj->m_fDBSessionCreated = TRUE;
    return (hr);
}



