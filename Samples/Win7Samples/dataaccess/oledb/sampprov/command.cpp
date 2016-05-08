//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module COMMAND.CPP | CCommand object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

// CCommand::CCommand --------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//        
CCommand::CCommand
	(
	CDBSession*	pCSession,		//@parm IN | Parent Session Object
	LPUNKNOWN 	pUnkOuter		//@parm IN | Outer Unkown Pointer
	)	// invoke ctor for base class
	: CBaseObj( BOT_COMMAND )
{
	assert( pCSession );

	//	Initialize simple member vars
	m_pUnkOuter					= pUnkOuter ? pUnkOuter : this;
	m_dwStatus					= 0L;
	m_cRowsetsOpen				= 0;
	m_cRef						= 0L;
	
	// Establish the parent object
	m_pCSession					= pCSession;

	//	Initially, NULL all contained interfaces
	m_pIAccessor				= NULL;
	m_pICommandText				= NULL;
	m_pICommandProperties		= NULL;
	m_pIColumnsInfo				= NULL;
	m_pIConvertType				= NULL;

	m_pUtilProp					= NULL;

	m_guidCmdDialect			= DBGUID_DBSQL;
	m_guidImpersonate			= GUID_NULL;

	m_strCmdText				= NULL;

	// Increment global object count.
    OBJECT_CONSTRUCTED();
	return;
}


// CCommand::~CCommand -------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//        
CCommand::~CCommand (void)
{
	//  Free contained interfaces
    SAFE_DELETE( m_pIAccessor );
    SAFE_DELETE( m_pICommandText );
    SAFE_DELETE( m_pICommandProperties );
    SAFE_DELETE( m_pIColumnsInfo );
    SAFE_DELETE( m_pIConvertType );

	SAFE_DELETE( m_pUtilProp );	

	// Since Command Object is going away, we can decrement 
	// our count on the session object.
	// Note that this typically deletes our hdbc, DataSource, Session.
	// (So do this last.)
	if( m_pCSession )
		m_pCSession->GetOuterUnknown()->Release();

	// Decrement global object count.
    OBJECT_DESTRUCTED();	
}


// CCommand::FInit -----------------------------------------------------------
//
// @mfunc Initialize the command Object.  This FInit routine should be used by
// CreateCommand and also be called as a secondary initialization routine for 
// for the other FInit on the command object.
//
// @side If this initialization routine fails, it is the callers responsibility
// to delete this object.  The destructor will then clean up any allocated 
// resources
//
// @rdesc Did the Initialization Succeed
//      @flag  S_OK | Initialization succeeded
//		@flag  E_OUTOFMEMORY | Could not allocate enough memory
//		@flag  E_FAIL | Initializtion failed
//        
HRESULT CCommand::FInit()
{
	assert( m_pUnkOuter );		// Set in constructor
	assert( m_pCSession );		// Set in constructor

	LPUNKNOWN   pIUnknown = (LPUNKNOWN) this;

	if (m_pUnkOuter)
        pIUnknown = m_pUnkOuter;

	if (m_pCSession && m_pCSession->GetOuterUnknown())
		m_pCSession->GetOuterUnknown()->AddRef();

	// Allocate Property Utility object
	m_pUtilProp = new CUtilProp();
	if( !m_pUtilProp )
		return ResultFromScode(E_OUTOFMEMORY);

	//	Allocate contained interface objects
	m_pIAccessor                = new CImpIAccessor( this, pIUnknown );
	m_pICommandText				= new CImpICommandText( this, pIUnknown );
	m_pICommandProperties		= new CImpICommandProperties( this, pIUnknown );
	m_pIColumnsInfo				= new CImpIColumnsInfo( this, pIUnknown );
	m_pIConvertType				= new CImpIConvertType( this, pIUnknown );

	if ( m_pIAccessor )
		{
		if(	FAILED(m_pIAccessor->FInit(FALSE)))
			return ResultFromScode(E_OUTOFMEMORY);
		}
	
	if( m_pIAccessor &&
		m_pICommandText &&
		m_pICommandProperties &&
		m_pIColumnsInfo &&
		m_pIConvertType 
		)
		return ResultFromScode(S_OK);
	else
		return ResultFromScode(E_FAIL);
}


// CCommand::QueryInterface --------------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use 
// QueryInterface to determine which interfaces the called object 
// supports. 
//
// @rdesc HResult indicating the status of the method
//		@flag S_OK | Interface is supported and ppvObject is set.
//		@flag E_NOINTERFACE | Interface is not supported by the object
//		@flag E_INVALIDARG | One or more arguments are invalid. 
//
STDMETHODIMP CCommand::QueryInterface
	(
	REFIID riid,				//@parm IN | Interface ID of the interface being queried for. 
	LPVOID * ppv				//@parm OUT | Pointer to interface that was instantiated      
	)
{
	if( ppv == NULL )
		return ResultFromScode(E_INVALIDARG);

	//	This is the non-delegating IUnknown implementation
	if( riid == IID_IAccessor )
		*ppv = (LPVOID)m_pIAccessor;
	else if( riid == IID_ICommand )
		*ppv = (LPVOID)m_pICommandText;
	else if( riid == IID_ICommandText )
		*ppv = (LPVOID)m_pICommandText;
	else if( riid == IID_ICommandProperties )
		*ppv = (LPVOID)m_pICommandProperties;
	else if( riid == IID_IColumnsInfo )
		*ppv = (LPVOID)m_pIColumnsInfo;
	else if( riid == IID_IConvertType )
		*ppv = (LPVOID)m_pIConvertType;
	else if( riid == IID_IUnknown )
		*ppv = (LPVOID)this;
	else
		*ppv = NULL;

	//	If we're going to return an interface, AddRef it first
	if( *ppv )
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}

	return ResultFromScode(E_NOINTERFACE);
}


// CCommand::AddRef ----------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_(DBREFCOUNT) CCommand::AddRef (void)
{
    return ++m_cRef;
}


// CCommand::Release -------------------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_(DBREFCOUNT) CCommand::Release (void)
{
    if (!--m_cRef)
        {
        delete this;
        return 0;
        }

    return m_cRef;
}


//  UTILITY FUNCTIONS   UTILITY FUNCTIONS

//	ICommandText specific interfaces

//-----------------------------------------------------------------------------------
// CImpICommandText::GetCommandText 
//
// @mfunc Echos the current command as text, including all post-processing 
//		  operations added.
//
// @rdesc HResult
//		@flag S_OK 					| Method Succeeded
//		@flag DB_S_DIALECTIGNORED	| Method succeeded, but dialect ignored
//		@flag E_INVALIDARG			| ppwszCommand was a null pointer
//		@flag E_OUTOFMEMORY			| Out of Memory
//
STDMETHODIMP CImpICommandText::GetCommandText
	(								 
	GUID		*pguidDialect,	//@parm INOUT | Guid denoting the dialect of sql
	LPOLESTR	*ppwszCommand	//@parm OUT   | Pointer for the command text
	)
{
	HRESULT		hr;

	// Check Function Arguments
	if( ppwszCommand == NULL )
	{
		hr = ResultFromScode(E_INVALIDARG);
		goto exit;
	}

	*ppwszCommand = NULL;

	// If the command has not been set, make sure the buffer
	// contains an empty stringt to return to the consumer
	if( !m_pObj->IsCommandSet() )
	{
		hr = ResultFromScode(DB_E_NOCOMMAND);
		goto exit;
	}

	assert( m_pObj->m_strCmdText );

	hr = NOERROR;

	if(  pguidDialect != NULL && 
		*pguidDialect != DBGUID_DEFAULT &&
		*pguidDialect != DBGUID_SAMPLEDIALECT )
	{
		hr = DB_S_DIALECTIGNORED;
		*pguidDialect = DBGUID_DEFAULT;
	}

	// Allocate memory for the string we're going to return to the caller
	*ppwszCommand = (LPWSTR) PROVIDER_ALLOC(
					(wcslen(m_pObj->m_strCmdText) + 1) * sizeof(WCHAR));
	
	if( !(*ppwszCommand) )
	{
		hr = ResultFromScode(E_OUTOFMEMORY);
		goto exit;
	}
	
	// Copy our saved text into the newly allocated string
	StringCchCopyW(*ppwszCommand,wcslen(m_pObj->m_strCmdText) + 1,m_pObj->m_strCmdText);

exit:

	if( FAILED(hr) && pguidDialect )
		memset(pguidDialect, 0, sizeof(GUID));

	return hr;
}

//-----------------------------------------------------------------------------------
// CImpICommandText::SetCommandText 
//
// @mfunc Sets the current command text
//
// @rdesc HResult
//		@flag S_OK | Method Succeeded
//		@flag E_FAIL | A provider-specific error occurred
//		@flag DB_E_DIALECTNOTSUPPORTED | The dialect given was not supported
//		@flag DB_E_OBJECTOPEN | A rowset was already open on the command object
//
STDMETHODIMP CImpICommandText::SetCommandText
	(
	REFGUID		rguidDialect,	//@parm IN | Guid denoting the dialect of sql
	LPCOLESTR	pwszCommand		//@parm IN | Command Text
	)
{
	HRESULT hr = S_OK;

	// Don't allow text to be set if we've got a rowset open
	if( m_pObj->IsRowsetOpen() )
		return ResultFromScode(DB_E_OBJECTOPEN);

	// Check Dialect
	if( rguidDialect != DBGUID_SAMPLEDIALECT && 
		rguidDialect != DBGUID_DEFAULT )
		return ResultFromScode(DB_E_DIALECTNOTSUPPORTED);

	if( !pwszCommand || *pwszCommand == L'\0' )
	{
		// Free the current string
		SAFE_DELETE_ARRAY(m_pObj->m_strCmdText);
		m_pObj->m_dwStatus &= ~CMD_TEXT_SET;
		return NOERROR;
	}

	WCHAR * pwszCMD = new WCHAR[wcslen(pwszCommand)+1];
	
	if ( !pwszCMD )
		hr = ResultFromScode(E_OUTOFMEMORY);

	// Free the old memory and set the text
	SAFE_DELETE_ARRAY(m_pObj->m_strCmdText);
	m_pObj->m_strCmdText = pwszCMD;
	StringCchCopyW(m_pObj->m_strCmdText,wcslen(pwszCommand)+1, pwszCommand);

	// Set status flag that we have set text
	m_pObj->m_dwStatus |= CMD_TEXT_SET;

	return NOERROR;
}


//---------------------------------------------------------------------------
// CImpICommandText::Cancel
//
// @mfunc The consumer can allocate a secondary thread in which to cancel 
// the currently executing thread.  This cancel will only succeed if 
// the result set is still being generated.  If the rowset object is being
// created, then it will be to late to cancel.
//
// @rdesc Status of Method
//		@flag S_OK | Cancel Succeeded
//		@flag DB_E_CANTCANCEL | Execution could not be canceled.
// 		@flag OTHER	| other result codes returned by called functions.
//---------------------------------------------------------------------------
STDMETHODIMP CImpICommandText::Cancel
	(
	void
	)
{
	if( m_pObj->m_dwStatus & CMD_EXECUTING )
		return DB_E_CANTCANCEL;

	return S_OK;
}



//---------------------------------------------------------------------------
// CImpICommandText::Execute 
//
// @mfunc Execute the command.  But before execution, it sets the ODBC
// statement and connection options based on the current properties.
//
// @rdesc Status of Execution
//		@flag S_OK | Execution succeeded
//		@flag E_INVALIDARG | Invalid parameter passed in.
//		@flag E_FAIL | Provider specific error
//		@flag E_OUTOFMEMORY | Not enough resources
//		@flag E_NOINTERFACE | RIID specified was not supported
//		@flag DB_E_NOCOMMAND | No Text or Tree set
//		@flag DB_E_INTERFACECONFLICT | Interface conflict with previous properties
// 		@flag OTHER	| other result codes returned by called functions.
//
STDMETHODIMP CImpICommandText::Execute
	(
	IUnknown*		pUnkOuter,		//@parm IN | Outer Unknown
	REFIID			riid,			//@parm IN | Interface ID of the interface being queried for.
	DBPARAMS*		pParams,		//@parm INOUT | Parameter Array
	DBROWCOUNT*		pcRowsAffected,	//@parm OUT | count of rows affected by command
	IUnknown**		ppRowset		//@parm OUT | Pointer to interface that was instantiated     
	)
{
	HRESULT		hr, hrProp = S_OK;
	UDWORD		dwFlags = 0;	
	UDWORD		dwStatus=0;
	ULONG		cPropSets = 0;
	ULONG		ul;
	ULONG		ul2;
	DBPROPSET	*rgPropSets = NULL;
	const IID*	pIID = &IID_ICommand;

    CFileIO*    pFileio = NULL;
    CRowset*    pRowset = NULL;

	BOOL		fCreateRowObject = FALSE;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		phRow = NULL;
	IRowset *	pIRowset = NULL;
	IGetRow *	pIGetRow = NULL;

	assert( m_pObj->m_pUtilProp );

	// Initialize Return Buffers
	if( ppRowset )
		*ppRowset = NULL;
	
	if( pcRowsAffected )
		*pcRowsAffected = -1;
	
	if( riid != IID_NULL && !ppRowset )
		return ResultFromScode( E_INVALIDARG );

	// Only 1 ParamSet if ppRowset is non-Null
	if( pParams && (pParams->cParamSets > 1) && ppRowset )
		return ResultFromScode( E_INVALIDARG );

	// Check that a command has been set
	if( !m_pObj->IsCommandSet() )
		return DB_E_NOCOMMAND;

	// We do not allow the riid to be anything other than IID_IUnknown for aggregation
	if( pUnkOuter && riid != IID_IUnknown )
		return ResultFromScode( DB_E_NOAGGREGATION );

	// Set Status
	m_pObj->m_dwStatus |= CMD_EXECUTING;

	// Check to see if IID_IOpenRowset is calling Execute to 
	// open a rowset.
	if( m_pObj->m_guidImpersonate != GUID_NULL ) 
	{		
		assert(m_pObj->m_guidImpersonate == IID_IOpenRowset);
		pIID = &m_pObj->m_guidImpersonate;
	}

	hr = m_pObj->m_pUtilProp->GetProperties(
					PROPSET_ROWSET,
					0,
					NULL,
					&cPropSets,
					&rgPropSets);

	if( hr != S_OK )
	{
		// The only valid reason to fail is out of memory case
		assert( hr == E_OUTOFMEMORY );
		goto exit;
	}

	// Traverse the properties to see if a ROW object is requested.
	for(ul=0;ul<cPropSets; ul++)
	{
		for(ul2=0;ul2<rgPropSets[ul].cProperties; ul2++)
		{			
			// Check if a row object was requested.
			// DBPROP_IRow, DBPROP_IRowChange, and DBPROP_IRowSchemaChange indicate
			// that a ROW object is desired.
			// Sample Provider does not support IRowSchemaChange so no need to check for that 
			// property
			if( (rgPropSets[ul].rgProperties[ul2].dwPropertyID == DBPROP_IRow ||
				 rgPropSets[ul].rgProperties[ul2].dwPropertyID == DBPROP_IRowChange) &&
				rgPropSets[ul].rgProperties[ul2].dwStatus == DBPROPSTATUS_OK &&
				V_BOOL(&rgPropSets[ul].rgProperties[ul2].vValue) == VARIANT_TRUE )				
				fCreateRowObject = TRUE;				
		}
	}
  
	//Try to open the file...
	if(FAILED(hr = m_pObj->m_pCSession->m_pCDataSource->OpenFile(m_pObj->m_strCmdText, &pFileio)))
		goto exit;

    // open and initialize a rowset\cursor object
    pRowset = new CRowset( fCreateRowObject ? NULL : pUnkOuter );
    if (!pRowset)
	{
        SAFE_DELETE( pFileio );
        hr = E_OUTOFMEMORY;
		goto exit;
	}

	// Initialize the rowset\cursor.
    // For now, since don't yet support "settable" properties, so no properties to pass.
    // The rowset will always create all of its interfaces.
    // This is all-or-nothing.

	//Assign creator pointer. Used to keep track of open rowsets
    pRowset->m_pCreator = m_pObj;     
	pRowset->m_pCreator->m_pUnkOuter->AddRef();

    if( !pRowset->FInit(
						pFileio,
						(*pIID == IID_IOpenRowset) ? (CBaseObj *) m_pObj->m_pCSession : (CBaseObj *) m_pObj,
						m_pObj->m_strCmdText,
						m_pObj->m_pCSession->m_pCDataSource->GetFilePath()
						) )
	{
        SAFE_DELETE( pRowset );
        hr = DB_E_NOTABLE;
		goto exit;
	}

	// set the properties
	if ( cPropSets )
		hr = pRowset->GetCUtilProp()->SetProperties(PROPSET_ROWSET, cPropSets, rgPropSets);
	
	// If all the properties set were OPTIONAL then we return
	// DB_S_ERRORSOCCURRED and continue.
	for(ul=0;ul<cPropSets; ul++)
	{
		for(ul2=0;ul2<rgPropSets[ul].cProperties; ul2++)
		{
			// Check for a required property that failed, if found, we must return
			// DB_E_ERRORSOCCURRED
			if( (rgPropSets[ul].rgProperties[ul2].dwStatus != DBPROPSTATUS_OK) &&
				(rgPropSets[ul].rgProperties[ul2].dwOptions != DBPROPOPTIONS_OPTIONAL) )
			{
					SAFE_DELETE( pRowset );
					hr = DB_E_ERRORSOCCURRED;
					goto exit;
			}					
		}
	}

	// Optional property failure is still a success
	if( hr == DB_E_ERRORSOCCURRED )
		hrProp = DB_S_ERRORSOCCURRED;	

	// if properties failed or ppRowset NULL
	if( (FAILED(hr) && (hrProp==S_OK)) || 
		(!ppRowset) )
    {
        SAFE_DELETE( pRowset );
		goto exit;
    }

    // get requested interface pointer on rowset\cursor
	if( fCreateRowObject || riid == IID_IRow || riid == IID_IRowChange )
	{
		hr = E_FAIL;

		if( FAILED(pRowset->QueryInterface(IID_IRowset, (LPVOID *)&pIRowset)) )
			goto exit;

		if( FAILED(pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow)) )
			goto exit;

		assert( cRowsObtained == 1);
		if( FAILED(pRowset->QueryInterface(IID_IGetRow, (LPVOID *)&pIGetRow)) )
			goto exit;

		if( FAILED(hr = pIGetRow->GetRowFromHROW(pUnkOuter, phRow[0], riid, ppRowset)) )
			goto exit;

		// Sample Provider does not support selection using a critera.
		// Hence, DB_S_NOTSINGLETON is reported simply if the target file
		// contains more than one row.
		if( pRowset->GetFileObj()->GetRowCnt() > 1 )
			hr = DB_S_NOTSINGLETON;
		else
			hr = S_OK;
	}
	else
	{
		hr = pRowset->QueryInterface( riid, (void **) ppRowset );
		if( FAILED(hr) )
		{
			// Special case IID_NULL
			if( riid == IID_NULL )
				hr = S_OK;

			SAFE_DELETE( pRowset );
			goto exit;
		}
	}

	if (SUCCEEDED(hr))
		m_pObj->IncrementOpenRowsets();

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGetRow);

exit:

	if( FAILED(hr) )
		SAFE_DELETE(pRowset);		

	// Free the RowHandle
	SAFE_FREE(phRow);

	// Exiting the execution state
	m_pObj->m_dwStatus &= ~CMD_EXECUTING;

	for (ul=0; ul < cPropSets; ul++)
		SAFE_FREE(rgPropSets[ul].rgProperties);

	SAFE_FREE(rgPropSets);

	return hr;
}


//---------------------------------------------------------------------------
// CImpICommandText::GetDBSession 
//
// @mfunc Return an interface from the session object that created
// this command object
//
// @rdesc Status of Method
//		@flag S_OK | Interface pointer returned
//		@flag E_INVALIDARG | ppSession was invalid
// 		@flag OTHER	| other result codes returned by called functions.
//
STDMETHODIMP CImpICommandText::GetDBSession
	(
	REFIID		riid,			//@parm IN | IID of the interface
	IUnknown**	ppSession		//@parm OUT | Interface pointer
	)
{
	HRESULT hr;

	assert( m_pObj->m_pCSession );

	// Check Arguments
	if( ppSession == NULL )
		return ResultFromScode(E_INVALIDARG);
			
	// Query for the interface on the session object.  If failure,
	// return the error from QueryInterface.
	hr = m_pObj->m_pCSession->GetOuterUnknown()->QueryInterface(riid, (VOID**)ppSession);
	if( FAILED(hr) )
	{
		*ppSession = NULL;
		return hr;
	}

	return ResultFromScode(S_OK);
}



//	ICommandProperties specific interfaces

//---------------------------------------------------------------------------
// CImpICommandProperties::GetProperties 
//
// @mfunc Retrieve the Properties
//
// @rdesc HRESULT indicating the status of the method
// 		@flag OTHER	| other result codes returned by called functions.
//
STDMETHODIMP CImpICommandProperties::GetProperties
	(
	const ULONG			cPropertySets,		//@parm IN | Number of property sets
	const DBPROPIDSET	rgPropertySets[],	//@parm IN | Property Sets
	ULONG*				pcProperties,		//@parm OUT | Count of structs returned
	DBPROPSET**			prgProperties		//@parm OUT | Array of Properties
	)
{
	HRESULT hr;

	assert( m_pObj );
	assert( m_pObj->m_pUtilProp );

	//NOTE: Since we are non-chaptered, we just ignore the 
	// rowset name argument.


	hr = m_pObj->m_pUtilProp->GetPropertiesArgChk(
				PROPSET_ROWSET,
				cPropertySets,
				rgPropertySets,
				pcProperties,
				prgProperties);
	if( SUCCEEDED(hr) )
	{
		hr = m_pObj->m_pUtilProp->GetProperties(
					PROPSET_ROWSET,
					cPropertySets,
					rgPropertySets,
					pcProperties,
					prgProperties);
	}

	// Just return the HResult
	return hr;
}


//---------------------------------------------------------------------------
// CImpICommandProperties::SetProperties 
//
// @mfunc Set the Properties
//
// @rdesc HRESULT indicating the status of the method
//		@flag DB_E_OPENOBJECT | Can't prepare w/ open rowset
// 		@flag OTHER	| other result codes returned by called functions.
//
STDMETHODIMP CImpICommandProperties::SetProperties
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
	// Quick return if the Count of Properties is 0
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
	// Don't allow properties to be set if we've got a rowset open
    //
	if( m_pObj->IsRowsetOpen() )
		return DB_E_OBJECTOPEN;

	return m_pObj->m_pUtilProp->SetProperties(PROPSET_ROWSET, 
											  cPropertySets, rgPropertySets);
}