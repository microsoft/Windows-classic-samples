//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DBSESS.CPP | CDBSession object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------

// CDBSession::CDBSession --------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CDBSession::CDBSession
	(
    LPUNKNOWN pUnkOuter         //@parm IN | Outer Unkown Pointer
	)	// invoke ctor for base class
	: CBaseObj( BOT_SESSION )
{
    //
	// Initialize simple member vars
	//
    m_cRef                = 0L;
	m_pUnkOuter			  = pUnkOuter ? pUnkOuter : this;
    m_pUtilProp			  = NULL;

    //
    // Initially, NULL all contained interfaces
    //
    m_pIGetDataSource	  = NULL;
    m_pIOpenRowset		  = NULL;
	m_pISessionProperties = NULL;
	m_pIDBCreateCommand	  = NULL;
	m_pIBindResource      = NULL;	

    //
	// Pointer to parent object
    //
	m_pCDataSource		  = NULL;

    //
    // Increment global object count.
    //
    OBJECT_CONSTRUCTED();
    return;
}


// CDBSession::~CDBSession -------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CDBSession:: ~CDBSession
(
    void
)
{
	//
	// Asserts
    //
	assert(m_pCDataSource);
	assert(m_pCDataSource->m_pUnkOuter);

    //
    // Free properties management object
    //
    SAFE_DELETE(m_pUtilProp);

    //
    // Free contained interfaces
    //
    SAFE_DELETE(m_pIGetDataSource);
    SAFE_DELETE(m_pIOpenRowset);
    SAFE_DELETE(m_pISessionProperties);
	SAFE_DELETE(m_pIDBCreateCommand);
	SAFE_DELETE(m_pIBindResource);

    //
    // Release the Data Source Object
    //
	m_pCDataSource->RemoveSession();
	m_pCDataSource->m_pUnkOuter->Release();	

    //
    // Decrement global object count.
    //
    OBJECT_DESTRUCTED();
    return;
}


// CDBSession::FInit --------------------------------------------------------
//
// @mfunc Initialize the command Object
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CDBSession::FInit
(
	CDataSource	* pCDataSource
)
{
	//
	// Asserts
    //
	assert(pCDataSource);

	//
	// Establish parent object pointer
	//
	m_pCDataSource = pCDataSource;
	m_pCDataSource->m_pUnkOuter->AddRef();

 	//
    // Allocate properties management object
	//
    m_pUtilProp = new CUtilProp();

    // Allocate contained interface objects
    m_pIOpenRowset		  = new CImpIOpenRowset(this, m_pUnkOuter);
    m_pIGetDataSource	  = new CImpIGetDataSource(this, m_pUnkOuter);
	m_pIDBCreateCommand	  = new CImpIDBCreateCommand(this, m_pUnkOuter);
    m_pISessionProperties = new CImpISessionProperties(this, m_pUnkOuter);
	m_pIBindResource	  = new CImpIBindResource(this, m_pUnkOuter);

    return (BOOL)(m_pUtilProp && m_pIGetDataSource && m_pIOpenRowset &&
		          m_pISessionProperties && m_pIDBCreateCommand && 
				  m_pIBindResource);
}



// CDBSession::QueryInterface -----------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use
// QueryInterface to determine which interfaces the called object
// supports.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK          | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG  | One or more arguments are invalid.
//
STDMETHODIMP CDBSession::QueryInterface
(
    REFIID riid,   //@parm IN | Interface ID of the interface being queried for.
    LPVOID * ppv   //@parm OUT | Pointer to interface that was instantiated
)
{
    //
    // Check in-params and NULL out-params in case of error
    //
    if( !ppv )
        return (E_INVALIDARG);

    //
    //  This is the non-delegating IUnknown implementation
    //
    if( riid == IID_IUnknown )
        *ppv = (LPVOID) this;
    else if( riid == IID_IGetDataSource )
        *ppv = (LPVOID) m_pIGetDataSource;
    else if( riid == IID_IOpenRowset )
        *ppv = (LPVOID) m_pIOpenRowset;
    else if( riid == IID_ISessionProperties )
        *ppv = (LPVOID) m_pISessionProperties;
	else if( riid == IID_IDBCreateCommand )
		*ppv = (LPVOID) m_pIDBCreateCommand;
	else if( riid == IID_IBindResource )
		*ppv = (LPVOID) m_pIBindResource;
    else {
        *ppv = NULL;
        return (E_NOINTERFACE);
    }

	((LPUNKNOWN) *ppv)->AddRef();
    return (S_OK);
}


// CDBSession::AddRef -------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CDBSession::AddRef
(
	void
)
{
    return ++m_cRef;
}


// CDBSession::Release ------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CDBSession::Release
(
	void
)
{
    if( !--m_cRef )
	{
		delete this;
		return 0;
	}

    return m_cRef;
}


//-----------------------------------------------------------------------------
// CImpIGetDataSource::GetDataSource 
//
// @mfunc Retrieve an interface pointer on the session object
//
// @rdesc 
//		@flag S_OK | Session Object Interface returned
//		@flag E_INVALIDARG | ppDataSource was NULL
//		@flag E_NOINTERFACE | IID not supported
//
STDMETHODIMP CImpIGetDataSource::GetDataSource
(
	REFIID		riid,			// @parm IN  | IID desired
	IUnknown**	ppDataSource	// @parm OUT | ptr to interface
)
{
	//
	// Asserts
    //
	assert(m_pObj);
	assert(m_pObj->m_pCDataSource);
	assert(m_pObj->m_pCDataSource->m_pUnkOuter);

    //
    // Check in-params and NULL out-params in case of error
    //
	if( !ppDataSource )
		return (E_INVALIDARG);

	//
	// Handle Aggregated DataSource (if aggregated)
	//
	return m_pObj->m_pCDataSource->m_pUnkOuter->QueryInterface(riid, (LPVOID*)ppDataSource);
}


// ISessionProperties::GetProperties ----------------------------------------------------
//
// @mfunc Returns current settings of all properties in the DBPROPFLAGS_SESSION property 
//			group
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CImpISessionProperties::GetProperties
(
    ULONG				cPropertySets,		//@parm IN | count of restiction guids
	const DBPROPIDSET	rgPropertySets[],	//@parm IN | restriction guids
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
	hr = m_pObj->m_pUtilProp->GetPropertiesArgChk(PROPSET_SESSION, 
				cPropertySets, rgPropertySets, pcProperties, prgProperties);
	
	if( FAILED(hr) )
		return hr;

    //
    // Just pass this call on to the utility object that manages our properties
    //
    return m_pObj->m_pUtilProp->GetProperties(PROPSET_SESSION, cPropertySets, 
								  rgPropertySets, pcProperties, prgProperties);
}


// CImpISessionProperties::SetProperties  --------------------------------------------
//
// @mfunc Set properties in the DBPROPFLAGS_SESSION property group
//
// @rdesc HRESULT
//      @flag E_INVALIDARG  | cProperties was not equal to 0 and rgProperties was NULL
//      @flag E_NOTIMPL		| this method is not implemented
//
STDMETHODIMP    CImpISessionProperties::SetProperties
	(
	ULONG		cPropertySets,		//@parm IN | Count of structs returned
	DBPROPSET	rgPropertySets[]    //@parm IN | Array of Properties
	)
{
    HRESULT hr = E_FAIL;

    //
	// Asserts
    //
	assert( m_pObj );
    assert( m_pObj->m_pUtilProp );

    //
	// Quick return if the Count of cPropertySets is 0
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
    return m_pObj->m_pUtilProp->SetProperties(PROPSET_SESSION, 
											  cPropertySets, rgPropertySets);
}


// CImpIDBCreateCommand::CreateCommand  ---------------------------------------------
//
// @mfunc Creates a Command object.
//
// @rdesc HRESULT
//		@flag S_OK			| success
//		@flag E_INVALIDARG	| ppICommand was NULL
//		@flag E_NOINTERFACE | IID not supported
//		@flag E_OUTOFMEMORY | Not enough resource to create command object
//

STDMETHODIMP CImpIDBCreateCommand::CreateCommand
(
	IUnknown *pUnkOuter,		//@parm IN | Outer unknown for new Command
	REFIID   riid,				//@parm IN | Desired interface on new Command
	IUnknown **ppCommand		//@parm OUT | New Command object
)
{
    //
    // Check in-params and NULL out-params in case of error
    //
	if( !ppCommand )
		return (E_INVALIDARG);

	*ppCommand = NULL;
	
    //
	// We do not allow any other iid than IID_IUnknown for aggregation
    //
	if( pUnkOuter && riid != IID_IUnknown )
		return (DB_E_NOAGGREGATION);

	//
	// This is the outer unknown from the user, for the new Command,
	// not to be confused with the outer unknown of this DBSession object.
	//
	HRESULT	hr = E_OUTOFMEMORY;
	CCommand* pCCommand = new CCommand(m_pObj, pUnkOuter);
	
	if( !pCCommand || FAILED(hr=pCCommand->FInit()) )
	{
		SAFE_DELETE(pCCommand);
		return (hr);
	}

	hr = pCCommand->QueryInterface(riid, (void **) ppCommand);

    if( FAILED(hr) ) 
        SAFE_DELETE(pCCommand);

	return (hr);
}
