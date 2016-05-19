//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IGETSOURCEROW.CPP | IGETSOURCEROW source file for all test modules.
//

#include "modstandard.hpp"
#include "IGETSOURCEROW.h"
#include "ExtraLib.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xd88d6ef0, 0x5be5, 0x11d2, { 0x92, 0xd2, 0x00, 0x60, 0x08, 0x93, 0xa2, 0xb2} };
DECLARE_MODULE_NAME("IGetSourceRow");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IGetSourceRow test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	// return CommonModuleInit(pThisTestModule /*, IID_IGetSourceRow */);

    // Check for Row object support in the provider
    return CommonModuleInit(pThisTestModule, IID_IRow, SIZEOF_TABLE, ROW_INTERFACE);
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
    return CommonModuleTerminate(pThisTestModule);
}	


////////////////////////////////////////////////////////////////////////
//CStreamObject
//
////////////////////////////////////////////////////////////////////////
class CStreamObject
{
public:
	// Constructors
	CStreamObject();
	virtual ~CStreamObject(void);
	
	// Members
	HRESULT	InitStreamObject
			(
				IBindResource * pIBindResource,
				WCHAR *			pwszURL
			);

	HRESULT	InitStreamObject
			(
				IRow *		pIRow
			);

	HRESULT	InitStreamObject
			(
				IScopedOperations * pIScopedOperations,
				REFIID				riid = IID_IStream
			);

	HRESULT	InitStreamObject
			(
				ICreateRow *	pICreateRow,
				WCHAR *			pwszURL,
				REFIID			riid = IID_IStream,
				WCHAR **		ppwszNewURL = NULL
			);

	HRESULT InitStreamObject
			(
				IRow *	pIRow,
				WCHAR * pwszColName
			);

	HRESULT InitStreamUsingGetColumns
			(
				IRow *	pIRow
			);

	HRESULT	SetStreamObject
			(
				IUnknown *	 pUnkStream
			);

	HRESULT	GetSourceRow
			(
				REFIID		riid,
				IUnknown ** ppIRow
			);

	void	ReleaseStreamObject();

	IStream *		pIStream()
	{
		ASSERT(m_pIStream);  return m_pIStream; 
	}

	IGetSourceRow *	pIGetSourceRow()
	{
		ASSERT(m_pIGetSourceRow);  return m_pIGetSourceRow; 
	}

	inline DBBINDURLFLAG GetBindURLFlags()
	{
		return m_dwBindFlags;
	}

	inline void SetBindURLFlags(DBBINDURLFLAG dwBindFlags)
	{
		m_dwBindFlags = dwBindFlags;
	}

protected:
	// Data
	IStream *			m_pIStream;
	IGetSourceRow *		m_pIGetSourceRow;

	DBBINDURLFLAG		m_dwBindFlags;
	DBBINDURLSTATUS		m_dwBindStatus;
};

/////////////////////////////////////////////////////////////////////////////
// CStreamObject
//
/////////////////////////////////////////////////////////////////////////////
CStreamObject::CStreamObject()
{	
	//Interfaces
	m_pIStream			= NULL;
	m_pIGetSourceRow	= NULL;

	//data
	m_dwBindFlags		= DBBINDURLFLAG_READ;
	m_dwBindStatus		= 0;
}


/////////////////////////////////////////////////////////////////////////////
// ~CStreamObject
//
/////////////////////////////////////////////////////////////////////////////
CStreamObject::~CStreamObject()
{
	ReleaseStreamObject();
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::InitStreamObject
(
	IBindResource *	pIBindResource,
	WCHAR *			pwszURL
)
{
	HRESULT			hr;
	DBBINDURLSTATUS	dwStatus = 0;

	if(!pIBindResource)
	{
		odtLog<<L"Pointer to IBindResource is NULL.\n";
		return E_FAIL;
	}

	TESTC_(hr = pIBindResource->Bind
								(
								NULL,
								pwszURL,
								m_dwBindFlags,
								DBGUID_STREAM,
								IID_IStream, 
								NULL,  
								NULL,
								&m_dwBindStatus, 
								(IUnknown **)&m_pIStream
								),S_OK);

	TESTC_(hr = m_pIStream->QueryInterface(IID_IGetSourceRow, (void **)&m_pIGetSourceRow), S_OK);	

CLEANUP:
	
	if( FAILED(hr) )
	{
		ReleaseStreamObject();		
	}

	return hr;	
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::InitStreamObject
(
	IRow *		pIRow
)
{
	HRESULT hr;
	DBID	dbidCol = DBROWCOL_DEFAULTSTREAM;

	if(!pIRow)
	{
		odtLog<<L"Pointer to IRow is NULL.\n";
		return E_FAIL;
	}
	
	TEST2C_(hr = pIRow->Open(NULL, &dbidCol, DBGUID_STREAM, 0, IID_IStream, 
							(IUnknown**)&m_pIStream), S_OK, DB_E_BADCOLUMNID);
	if( SUCCEEDED(hr) )
	{
		TESTC_(hr = m_pIStream->QueryInterface(IID_IGetSourceRow, (void **)&m_pIGetSourceRow), S_OK);	
	}
	
CLEANUP:

	if( FAILED(hr) )
	{
		ReleaseStreamObject();
	}

	return hr;	
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::InitStreamObject
(
	ICreateRow *	pICreateRow,
	WCHAR *			pwszURL,
	REFIID			riid,
	WCHAR **		ppwszNewURL
)
{
	HRESULT			hr;
	DBBINDURLSTATUS	dwStatus = 0;
	IUnknown *		pUnk = NULL;
	WCHAR *			pwszNewURL = NULL;

	if(!pICreateRow)
	{
		odtLog<<L"Pointer to ICreateRow is NULL.\n";
		return E_FAIL;
	}

	// The spec requires all 2.5 providers to support ICreateRow on the provider binder
	// However, if a provider cannot naturally support ICreateRow, then 
	// that Provider should return E_NOINTERFACE to indicate that its ICreateRow interface
	// is not supported.	
	TEST2C_(hr = pICreateRow->CreateRow
							(
								NULL,
								pwszURL,
								m_dwBindFlags,
								DBGUID_STREAM,
								riid, 
								NULL, 
								NULL,
								&m_dwBindStatus, 
								&pwszNewURL,
								(IUnknown **)&pUnk
							 ), S_OK, E_NOINTERFACE);

	if( hr == S_OK )
	{
		TESTC_(hr = pUnk->QueryInterface(IID_IStream, 
						(void **)&m_pIStream), S_OK);			

		TESTC_(hr = pUnk->QueryInterface(IID_IGetSourceRow, 
						(void **)&m_pIGetSourceRow), S_OK);			
	}

CLEANUP:

	SAFE_RELEASE(pUnk);

	if( FAILED(hr) )
	{
		ReleaseStreamObject();
		SAFE_FREE(pwszNewURL);
	}
	else
	{
		if (ppwszNewURL)
			*ppwszNewURL = pwszNewURL;
		else
			SAFE_FREE(pwszNewURL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::InitStreamObject
(
	IScopedOperations *	pIScopedOperations,
	REFIID				riid
)
{
	HRESULT			hr;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);
	IUnknown *		pUnk = NULL;
	DBBINDURLSTATUS	dwStatus = 0;


	if(!pIScopedOperations)
	{
		odtLog<<L"Pointer to IScopedOperations is NULL.\n";
		return E_FAIL;
	}

	TESTC_(hr = pIScopedOperations->Bind
									(
									NULL,
									pwszURL,
									m_dwBindFlags,
									DBGUID_STREAM,
									riid, 
									NULL, 
									NULL,
									&m_dwBindStatus, 
									(IUnknown **)&pUnk
									 ),S_OK);

	TESTC_(hr = pUnk->QueryInterface(IID_IStream, (void **)&m_pIStream), S_OK);			
	TESTC_(hr = pUnk->QueryInterface(IID_IGetSourceRow, (void **)&m_pIGetSourceRow), S_OK);			
	
CLEANUP:

	SAFE_RELEASE(pUnk);

	if( FAILED(hr) )
	{
		ReleaseStreamObject();
	}

	return hr;		
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::InitStreamObject
(
	IRow *	pIRow,
	WCHAR *	pwszStreamColName
)
{
	HRESULT			hr = E_FAIL;
	DBCOLUMNACCESS	dbColAccess;
	IUnknown *		pIUnknown = NULL;;

	if(!pIRow)
	{
		odtLog<<L"Pointer to IRow is NULL.\n";
		return E_FAIL;
	}

	if (pwszStreamColName)
	{	
		dbColAccess.columnid.eKind = DBKIND_NAME;
		dbColAccess.columnid.uName.pwszName = pwszStreamColName;
	}
	else
		dbColAccess.columnid = DBROWCOL_DEFAULTSTREAM;
	
	dbColAccess.pData		= PROVIDER_ALLOC(sizeof(IUnknown *));
	dbColAccess.cbDataLen	= sizeof(IUnknown *);
	dbColAccess.dwStatus	= DBSTATUS_S_OK;
	dbColAccess.wType		= DBTYPE_IUNKNOWN;
	
	TESTC(dbColAccess.pData != NULL);
	TEST2C_(hr = pIRow->GetColumns(1, &dbColAccess), S_OK, DB_E_ERRORSOCCURRED);

	if( SUCCEEDED(hr) )
	{
		pIUnknown = *(IUnknown **)(dbColAccess.pData);
		TESTC(pIUnknown != NULL);

		// IGetSourceRow may not be available from storage object retrieved by IRow::GetColumns
		QTESTC_(hr = pIUnknown->QueryInterface(IID_IGetSourceRow, (void **)&m_pIGetSourceRow), S_OK);		
		QTESTC_(hr = pIUnknown->QueryInterface(IID_IStream, (void **)&m_pIStream), S_OK);		
	}
	else
	{
		if( hr == DB_E_ERRORSOCCURRED )
		{
			// Raise error on failure to get DEFAULTSTREAM
			TESTC( dbColAccess.dwStatus == DBSTATUS_E_DOESNOTEXIST );
			hr = DB_E_BADCOLUMNID;
		}
	}

CLEANUP:

	SAFE_RELEASE(pIUnknown);
	SAFE_FREE(dbColAccess.pData);

	if( FAILED(hr) )
	{
		ReleaseStreamObject();
	}

	return hr;	
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::InitStreamUsingGetColumns
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CStreamObject::InitStreamUsingGetColumns
(
	IRow *	pIRow
)
{
	HRESULT				hr = E_FAIL;
	CRowObject			RowObjectA;
	DBCOUNTITEM			cColAccess;
	ULONG_PTR			cbRowSize;
	DBCOLUMNACCESS *	rgColAccess = NULL;
	void *				pData = NULL;
	IUnknown *			pUnk = NULL;

	// Try to bind to the first column that may get us a Stream object
	TESTC_(hr = RowObjectA.SetRowObject(pIRow),S_OK);
	TESTC_(hr = RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, &cbRowSize,
				BLOB_COLS_BOUND, BLOB_IID_IUNKNOWN), S_OK);
	if( cColAccess )
	{
		TESTC_(hr = RowObjectA.GetColumns(1, rgColAccess), S_OK);

		pUnk = *(IUnknown **)rgColAccess[0].pData;
		
		hr = SetStreamObject(pUnk);
	}
	else
		hr = E_NOINTERFACE;

CLEANUP:

	SAFE_RELEASE(pUnk);

	SAFE_FREE(pData);
	SAFE_FREE(rgColAccess);

	if( FAILED(hr) )
	{
		ReleaseStreamObject();
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CStreamObject::SetStreamObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::SetStreamObject
(
	IUnknown *	 pUnkStream
)
{
	HRESULT		hr = S_OK;
	IStream *	pIStream = NULL;

	// ISequentialStream is a mandatory interface on the stream object, but IStream
	// is optional...
	if(!VerifyInterface(pUnkStream, IID_ISequentialStream, STREAM_INTERFACE, 
						(IUnknown**)&pIStream))
		return E_NOINTERFACE; 

	//Release the previous stream object
	ReleaseStreamObject();
	
	//Now that everything worked successfully, save the interface...
	m_pIStream = pIStream;

	// IGetSourceRow is optional on the stream object
	if(!VerifyInterface(m_pIStream, IID_IGetSourceRow, STREAM_INTERFACE,
						(IUnknown**)&m_pIGetSourceRow))
	{
		SAFE_RELEASE(m_pIStream);
		return E_NOINTERFACE;
	}

	return hr;		
}

/////////////////////////////////////////////////////////////////////////////
// CStreamObject::ReleaseStreamObject
//
/////////////////////////////////////////////////////////////////////////////
void CStreamObject::ReleaseStreamObject()
{
	//Interfaces
	SAFE_RELEASE(m_pIStream);
	SAFE_RELEASE(m_pIGetSourceRow);
}


////////////////////////////////////////////////////////////////////////////
//  CStreamObject::GetSourceRow
//
////////////////////////////////////////////////////////////////////////////
HRESULT	CStreamObject::GetSourceRow
(
	REFIID		riid,
	IUnknown ** ppIRow
)
{
	//IGetSource::GetSourceRowset
	HRESULT hr;

	if(!m_pIGetSourceRow)
		return E_NOINTERFACE;

	hr = m_pIGetSourceRow->GetSourceRow(riid, ppIRow);
	
	//Do some postprocessing
	if(SUCCEEDED(hr) && ppIRow)
	{
		TESTC(*ppIRow != NULL);
	}
	else
	{
		if(ppIRow)
		{
			TESTC(*ppIRow == NULL);
		}
	}

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCBase
//
////////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(TC_Empty); }

	//methods
	virtual void SetTestCaseParam(ETESTCASE eTestCase = TC_Empty)
	{
		m_eTestCase = eTestCase;
	}

	//data
	ETESTCASE	m_eTestCase;
};


////////////////////////////////////////////////////////////////////////
//TCIGetSourceRow Class
//
////////////////////////////////////////////////////////////////////////
class TCIGetSourceRow : public CRowset, public TCBase
{
public:

	//Constructor
	TCIGetSourceRow(WCHAR* pwszTestCaseName);
	//Destructor
	virtual ~TCIGetSourceRow();

	//methods
	virtual BOOL		Init(ETESTCASE eTestCase = TC_Empty);
	virtual BOOL		Terminate();

	virtual HRESULT		TestGetSourceRow
						(
							CStreamObject * pCStreamObj
						);

	virtual HRESULT		TestMultipleStreams
						(
							ULONG_PTR		cStreamObjects,
							ESTREAMSOURCE	eStreamSource
						);

	static ULONG WINAPI Thread_VerifyGetSourceRow
						(
							LPVOID pv
						);

	virtual BOOL		VerifyGetSourceRow
						(
							REFIID		riid,
							IUnknown ** ppIRow = NULL
						);

	static ULONG WINAPI Thread_GetColumns_VerifyGetSourceRow
						(
							LPVOID pv
						);

	virtual	BOOL		GetColumns_VerifyGetSourceRow
						(
							CStreamObject *	pStream,
							REFIID			riid, 
							IUnknown **		ppIRow = NULL
						);

	static ULONG WINAPI Thread_ReleaseRow
						(
							LPVOID pv
						);

	virtual HRESULT		BindRow
						(
							IUnknown *	pUnkOuter,
							IUnknown **	pUnknown
						);

	virtual BOOL		VerifyBLOBSFromRowObject
						(
							CRowObject *	pRowObj
						);

protected:

	IDBBinderProperties *	m_pIDBBinderProperties;

	IBindResource *			m_pIBindResource;

	ICreateRow *			m_pICreateRow;

	HROW					m_hRow;

	CRowObject *			m_pCRowObject;

	CStreamObject *			m_pCStreamObject;

	IRowset *				m_pIRowset;

private:

	HRESULT				CreateRootBinder();

	BOOL				SetInitProps
						(
							IDBBinderProperties * pIDBBindProp
						);
};


////////////////////////////////////////////////////////////////////////////
//  TCIGetSourceRow::TCIGetSourceRow
//
////////////////////////////////////////////////////////////////////////////
TCIGetSourceRow::TCIGetSourceRow(WCHAR * wstrTestCaseName)	: CRowset(wstrTestCaseName) 
{
	m_pCRowObject			= NULL;
	m_pCStreamObject		= NULL;
	m_pIRowset				= NULL;

	m_pIDBBinderProperties	= NULL;
	m_pIBindResource		= NULL;
	m_pICreateRow			= NULL;

	m_hRow					= DB_NULL_HROW;
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetSourceRow::~TCIGetSourceRow
//
////////////////////////////////////////////////////////////////////////////
TCIGetSourceRow::~TCIGetSourceRow()
{
	if( m_pCRowObject )
		delete m_pCRowObject;
	if( m_pCStreamObject )
		delete m_pCStreamObject;

	ASSERT(m_pIBindResource == NULL);
	ASSERT(m_pICreateRow == NULL);
	ASSERT(m_pIDBBinderProperties == NULL);
	ASSERT(m_pIRowset == NULL);
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetSourceRow::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetSourceRow::Init(ETESTCASE eTestCase)
{
	BOOL		fPass = TEST_FAIL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		rghRows = &m_hRow;
	IRow *		pIRow = NULL;

	if(!CRowset::Init())
		return FALSE;

	// Create Root Binder
 	TESTC(SUCCEEDED(CreateRootBinder()))

	//Create a new row and stream object 
	m_pCRowObject = new CRowObject;
	m_pCStreamObject = new CStreamObject;
	TESTC(m_pCRowObject != NULL && m_pCStreamObject != NULL);	

		//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);

	//May require ACCESSORDER RANDOM to allow multiple accesses to streams.  
	//Otherwise reading a stream over a BLOB column may may prevent reading it again
	//because we've read past that BLOB column.  Needed for multiple stream scenarios.
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void *)DBPROPVAL_AO_RANDOM, VT_I4);

	//Create the Rowset object
	TESTC_(CreateRowset(), S_OK);

	TESTC_(GetRowObject(1, m_pCRowObject),S_OK);

	fPass = TEST_PASS;

CLEANUP:

	ReleaseRows(m_hRow);
	SAFE_RELEASE(pIRow);
	
	return fPass;	
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetSourceRow::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetSourceRow::Terminate()
{
	SAFE_DELETE(m_pCRowObject);
	SAFE_DELETE(m_pCStreamObject);

	// Release the root binder interfaces
	m_pIBindResource = NULL;
	SAFE_RELEASE(m_pICreateRow);
	SAFE_RELEASE(m_pIDBBinderProperties);

	return CRowset::Terminate();
}


//----------------------------------------------------------------------
// TCIGetSourceRow::CreateRootBinder
//
// lifted from IBindResource.cpp
// probably should be in privlib
HRESULT TCIGetSourceRow::CreateRootBinder()
{
	HRESULT hrRet = E_FAIL;

	m_pIBindResource = GetModInfo()->GetRootBinder();

	if(!m_pIBindResource)
	{
		odtLog<<L"Failed to retrieve IBindResource.\n";
		hrRet = E_FAIL;
		goto CLEANUP;
	}

	TESTC(VerifyInterface(m_pIBindResource, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_pIDBBinderProperties))
	TESTC(VerifyInterface(m_pIBindResource, IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&m_pICreateRow))

	TESTC(SetInitProps(m_pIDBBinderProperties))
	hrRet = S_OK;

CLEANUP:
	return hrRet;
} //CreateRootBinder


//----------------------------------------------------------------------
// TCIGetSourceRow::SetInitProps
//
// lifted from IBindResource.cpp
// probably should be in privlib
BOOL TCIGetSourceRow::SetInitProps
(
	IDBBinderProperties *	pIDBBindProp
)
{
	BOOL		bRet = FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET *	rgPropSets = NULL;

	if(!pIDBBindProp)
		return FALSE;

	if(COMPARE(GetInitProps(&cPropSets, &rgPropSets), TRUE))
	{
		if(CHECK(pIDBBindProp->SetProperties(cPropSets, rgPropSets), 
			S_OK))
			bRet = TRUE;
	}

	::FreeProperties(&cPropSets, &rgPropSets);
	return bRet;
} //SetInitProps


//----------------------------------------------------------------------
// TCIGetSourceRow::TestGetSourceRow
//
HRESULT TCIGetSourceRow::TestGetSourceRow
(
	CStreamObject *	 pCStreamObject
)
{
	HRESULT			hr;
	IUnknown *		pIUnknown = NULL;	
	ULONG			cRowIIDs = 0;
	ULONG			i;
	INTERFACEMAP *	rgRowIIDs = NULL;

	//Obtain the Rowset interfaces...
	GetInterfaceArray(ROW_INTERFACE, &cRowIIDs, &rgRowIIDs);

	//For every [MANDATORY] interface, try to get the parent ROW object...
	for(i=0; i<cRowIIDs; i++)
	{
		//IGetSourceRow::GetSourceRow
		hr = pCStreamObject->GetSourceRow(*rgRowIIDs[i].pIID, (IUnknown**)&pIUnknown);
		
		if (hr == DB_E_NOSOURCEOBJECT)
		{
			TWARNING(L"This stream did not have a source row object.");
		}

		//Determine results
		if(rgRowIIDs[i].fMandatory)
		{
			//[MANDATORY]
			if(hr!=S_OK && hr!=DB_E_NOSOURCEOBJECT)
			{
				CHECK(hr, S_OK);
				TOUTPUT_(L"ERROR: Interface Incorrect for " << GetInterfaceName(*rgRowIIDs[i].pIID) << "\n");
			}
		}
		else
		{
			//[OPTIONAL]
			if(hr!=S_OK && hr!=DB_E_NOSOURCEOBJECT && hr!=E_NOINTERFACE)
			{
				CHECK(hr, S_OK);
				TOUTPUT_(L"ERROR: Interface Incorrect for " << GetInterfaceName(*rgRowIIDs[i].pIID) << "\n");
			}
		}
		SAFE_RELEASE(pIUnknown);
	}

	//Verify E_NOINTERFACE for IID_NULL
	CHECK(pCStreamObject->GetSourceRow(IID_NULL, &pIUnknown), E_NOINTERFACE);
	SAFE_RELEASE(pIUnknown);

	//Verify E_INVALIDARG condition
	CHECK(pCStreamObject->GetSourceRow(IID_NULL, NULL), E_INVALIDARG);

	return S_OK;
} 


//----------------------------------------------------------------------
// TCIGetSourceRow::TestMultipleStreams
//
HRESULT TCIGetSourceRow::TestMultipleStreams
(
	ULONG_PTR		cStreamObjects, 
	ESTREAMSOURCE	eStreamSource
)
{
	HRESULT				hr;
	CStreamObject **	rgpCStreamObjects = NULL;
	IUnknown *			pIUnknown = NULL;
	ULONG				iStream;

	//Create the Stream Objects from the row handle obtained in ::Init
	SAFE_ALLOC(rgpCStreamObjects, CStreamObject*, cStreamObjects);
	memset(rgpCStreamObjects, 0, cStreamObjects * sizeof(CStreamObject*));
		
	// Get cStreamObjects
	for(iStream=0; iStream<cStreamObjects; iStream++)
	{
		//Create the Stream object
		CStreamObject* pCStreamObject = new CStreamObject;

		TESTC(pCStreamObject != NULL);
		rgpCStreamObjects[iStream] = pCStreamObject;

		// Open the stream object dependent on eStreamSource
		if (EROWOPEN == eStreamSource || (EROWBOTH == eStreamSource && (0 == iStream % 2)))
		{
			TEST2C_(hr = pCStreamObject->InitStreamObject(m_pCRowObject->pIRow()), S_OK, DB_E_BADCOLUMNID)
		}
		else if (EROWGETCOL == eStreamSource || (EROWBOTH == eStreamSource && (1 == iStream % 2)))
		{
			TEST3C_(hr = pCStreamObject->InitStreamUsingGetColumns(m_pCRowObject->pIRow()), 
								S_OK, E_NOINTERFACE, DB_E_BADCOLUMNID);
		}
		else 
			ASSERT(!"No Support yet");
			
		if( S_OK == hr )
		{
			//Try to get back to the creating rowset
			TEST2C_(pCStreamObject->GetSourceRow(IID_IUnknown, &pIUnknown), S_OK, DB_E_NOSOURCEOBJECT);
			if(pIUnknown)
			{
				//Make sure its returning the original object
				TESTC(VerifyEqualInterface(pIUnknown, m_pCRowObject->pIRow()));
			}

			SAFE_RELEASE(pIUnknown);
		}
	}			
	
CLEANUP:
	for(iStream=0; iStream<cStreamObjects && rgpCStreamObjects; iStream++)
		SAFE_DELETE(rgpCStreamObjects[iStream]);
	SAFE_RELEASE(pIUnknown);
	SAFE_FREE(rgpCStreamObjects);

	return S_OK;
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::Thread_VerifyGetSourceRow
//
ULONG TCIGetSourceRow::Thread_VerifyGetSourceRow(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIGetSourceRow* pThis = (TCIGetSourceRow*)THREAD_FUNC;
	ASSERT(pThis);

	ThreadSwitch(); //Let the other thread(s) catch up

	//IRow::GetColumns
	TESTC(pThis->VerifyGetSourceRow(IID_IRow));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::VerifyGetSourceRow
//
BOOL TCIGetSourceRow::VerifyGetSourceRow
(
	REFIID			riid, 
	IUnknown **		ppIRow
)
{
	HRESULT			hr = S_OK;
	BOOL			fPass = TEST_FAIL;	
	HROW			hRow = DB_NULL_HROW;
	IUnknown *		pIUnknown = NULL;
	
	//IGetSourceRow::GetSourceRow
	TESTC_(hr = m_pCStreamObject->GetSourceRow(riid, &pIUnknown),S_OK);

	//Verify the rowset returned
	TESTC(DefaultObjectTesting(pIUnknown, ROW_INTERFACE));
	fPass = TEST_PASS;

CLEANUP:
	if(ppIRow)
		*ppIRow = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	
	return fPass;
}

//----------------------------------------------------------------------
//  TCIGetSourceRow::Thread_GetColumns_VerifyGetSourceRow
//
ULONG TCIGetSourceRow::Thread_GetColumns_VerifyGetSourceRow
(
	LPVOID pv
)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIGetSourceRow* pThis = (TCIGetSourceRow*)THREAD_FUNC;
	CStreamObject * pStream = (CStreamObject *)THREAD_ARG1;
	ASSERT(pThis);

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->GetColumns_VerifyGetSourceRow(pStream, IID_IRow));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::GetColumns_VerifyGetSourceRow
//
BOOL TCIGetSourceRow::GetColumns_VerifyGetSourceRow
(
	CStreamObject *	pStream,
	REFIID			riid, 
	IUnknown **		ppIRow
)
{
	HRESULT			hr = S_OK;
	BOOL			fPass = TEST_FAIL;	
	IUnknown *		pIUnknown = NULL;

	if( S_OK == hr )
	{
		TESTC_(hr = pStream->GetSourceRow(riid, &pIUnknown), S_OK);

		//Verify the rowset returned
		TESTC(DefaultObjectTesting(pIUnknown, ROW_INTERFACE));
	}

	fPass = TEST_PASS;

CLEANUP:

	if(ppIRow)
		*ppIRow = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	
	return fPass;
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::Thread_ReleaseRow
//
ULONG TCIGetSourceRow::Thread_ReleaseRow
(
	LPVOID pv
)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CRowset *	pRowset = (CRowset *)THREAD_FUNC;
	HROW *		phrow	= (HROW *)THREAD_ARG1;

	ASSERT(pRowset);
	ASSERT(phrow);

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pRowset->ReleaseRows(*phrow), S_OK);
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::BindRow
//
HRESULT	TCIGetSourceRow::BindRow
(
	IUnknown *	pUnkOuter,
	IUnknown **	ppUnknown
)
{
	HRESULT			hr;
	WCHAR *			pwszRowURL = GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE);
	DBBINDURLSTATUS dwBindStatus = ~0;

	if(!pwszRowURL)
	{
		return S_FALSE;
	}

	TEST2C_(hr = m_pIBindResource->Bind
								(
									pUnkOuter,
									pwszRowURL,
									DBBINDURLFLAG_READ,
									DBGUID_ROW,
									IID_IUnknown, 
									NULL, 
									NULL,
									&dwBindStatus, 
									ppUnknown
								), S_OK, DB_E_NOAGGREGATION);

	if( S_OK == hr )
		TESTC(dwBindStatus == 0);
	
CLEANUP:
	
	if( FAILED(hr) && ppUnknown )
	{
		*ppUnknown = NULL;	
	}

	return hr;	
}


//----------------------------------------------------------------------
//  TCIGetSourceRow::VerifyBLOBSFromRowObject
//
BOOL TCIGetSourceRow::VerifyBLOBSFromRowObject
(
	CRowObject *	pRowObj
)
{	
	BOOL				fPass = TEST_FAIL;
	HRESULT				hr;
	IRow *				pIRow = NULL;
	DBCOUNTITEM			cColAccess = 0;
	DBCOLUMNACCESS *	rgColAccess = NULL;
	void *				pData = NULL;
	ULONG_PTR			cbRowSize = 0;
	ULONG_PTR			cIter = 0;

	TESTC_(pRowObj->CreateColAccess(&cColAccess, &rgColAccess, &pData, &cbRowSize,
				BLOB_COLS_BOUND, BLOB_IID_IUNKNOWN), S_OK);

	TESTC_(pRowObj->GetColumns(cColAccess, rgColAccess), S_OK);

	for( cIter = 0; cIter < cColAccess; cIter++ )
	{
		CStreamObject	StreamObjectA;
		IUnknown *		pUnkStream = NULL;
		ILockBytes *	pILockBytes = NULL;

		TESTC(rgColAccess[cIter].dwStatus ==  DBSTATUS_S_OK);
		pUnkStream = *(IUnknown **)rgColAccess[cIter].pData;

		if( S_OK == (hr = StreamObjectA.SetStreamObject(pUnkStream)) )
		{
			CHECK(TestGetSourceRow(&StreamObjectA), S_OK);
			COMPARE(DefaultObjectTesting(StreamObjectA.pIGetSourceRow(), STREAM_INTERFACE), TRUE);	
		}
		else
		{
			TWARNING(L"This storage object does not support IGetSourceRow.");
		}
		
		// Optionally see if the object supports ILockBytes
		// If so, just perform a simple read
		if(VerifyInterface(pUnkStream, IID_ILockBytes, STREAM_INTERFACE,(IUnknown**)&pILockBytes))	
		{
			BYTE	bBuffer[MAX_COL_SIZE];
			ULONG	cBytes = 0;
			HRESULT hr;

			hr = StorageRead(IID_ILockBytes, pILockBytes, bBuffer, MAX_COL_SIZE, &cBytes);
			COMPARE(hr == S_OK || hr == S_FALSE, TRUE);
			COMPARE(DefaultObjectTesting(pILockBytes, STREAM_INTERFACE), TRUE);
			SAFE_RELEASE(pILockBytes);
		}
		
		SAFE_RELEASE(pUnkStream);
	}

	fPass = TEST_PASS;

CLEANUP:
	
	SAFE_FREE(rgColAccess);
	SAFE_FREE(pData);

	SAFE_RELEASE(pIRow);

	return fPass;
}



// {{ TCW_TEST_CASE_MAP(TCIGetSourceRow_IUnknown)
//--------------------------------------------------------------------
// @class Test IUnknown
//
class TCIGetSourceRow_IUnknown : public TCIGetSourceRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIGetSourceRow_IUnknown,TCIGetSourceRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DefaultTesting IBindResource
	int Variation_1();
	// @cmember DefaultTesting IRow_Open from GetRow
	int Variation_2();
	// @cmember DefaultTesting IScopedOperations
	int Variation_3();
	// @cmember DefaultTesting ICreateRow
	int Variation_4();
	// @cmember DefaultTesting Session IBindResource
	int Variation_5();
	// @cmember DefaultTesting Session ICreateRow
	int Variation_8();
	// @cmember DefaultTesting IRow_GetColumns
	int Variation_9();
	// @cmember DefaultTesting IScopedOperations requesting requesting IID_IGetSourceRow
	int Variation_10();
	// @cmember DefaultTesting ICreateRow requesting requesting IID_IGetSourceRow
	int Variation_11();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIGetSourceRow_IUnknown)
#define THE_CLASS TCIGetSourceRow_IUnknown
BEG_TEST_CASE(TCIGetSourceRow_IUnknown, TCIGetSourceRow, L"Test IUnknown")
	TEST_VARIATION(1, 		L"DefaultTesting IBindResource")
	TEST_VARIATION(2, 		L"DefaultTesting IRow_Open from GetRow")
	TEST_VARIATION(3, 		L"DefaultTesting IScopedOperations")
	TEST_VARIATION(4, 		L"DefaultTesting ICreateRow")
	TEST_VARIATION(5, 		L"DefaultTesting Session IBindResource")
	TEST_VARIATION(8, 		L"DefaultTesting Session ICreateRow")
	TEST_VARIATION(9, 		L"DefaultTesting IRow_GetColumns")
	TEST_VARIATION(10, 		L"DefaultTesting IScopedOperations requesting requesting IID_IGetSourceRow")
	TEST_VARIATION(11, 		L"DefaultTesting ICreateRow requesting requesting IID_IGetSourceRow")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIGetSourceRow_GetSourceRow)
//--------------------------------------------------------------------
// @class Test GetSourceRow method
//
class TCIGetSourceRow_GetSourceRow : public TCIGetSourceRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIGetSourceRow_GetSourceRow,TCIGetSourceRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IRow_Open riid all Mandatory interfaces
	int Variation_1();
	// @cmember IBindResource riid all Mandatory interfaces
	int Variation_2();
	// @cmember ICreateRow riid all Mandatory interfaces
	int Variation_3();
	// @cmember IScopedOperations riid all Mandatory interfaces
	int Variation_4();
	// @cmember NULL ppIRow
	int Variation_5();
	// @cmember IRow from ICommand riid all Mandatory Interfaces
	int Variation_7();
	// @cmember IRow from IOpenRwst riid all Mandatory Interfaces
	int Variation_8();
	// @cmember Sequence - 2 streams from IRow_Open
	int Variation_9();
	// @cmember Verify Stream addrefs parent row
	int Variation_12();
	// @cmember Mutliple open Row objects
	int Variation_13();
	// @cmember Agg Stream
	int Variation_14();
	// @cmember Agg Row A, unagg Stream
	int Variation_15();
	// @cmember Agg Row A, unagg Stream, release source Row
	int Variation_16();
	// @cmember Agg Row A, agg Stream B
	int Variation_17();
	// @cmember Threads  GetSourceRow
	int Variation_18();
	// @cmember Session IBindResource riid all Mandatory interfaces
	int Variation_19();
	// @cmember Session ICreateRow riid all Mandatory interfaces
	int Variation_20();
	// @cmember IRow from ICommand  Open Stream using GetColumns
	int Variation_21();
	// @cmember IRow from IOpenRowset Open Stream using GetColumns
	int Variation_22();
	// @cmember IRow from IGetRow Open Stream using GetColumns
	int Variation_23();
	// @cmember IRow from IBindResource Open Stream using GetColumns
	int Variation_24();
	// @cmember Threads, row from IGetRow, streams from GetColumns
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember Sequence - Multiple streams from IRow_GetColumns
	int Variation_28();
	// @cmember Sequence - Multiple streams from IRow_Open and IRowGetColumns
	int Variation_29();
	// @cmember Call GetSourceRow on stream whose parent is a rowset
	int Variation_30();
	// @cmember Call GetSourceRow after SetColumns
	int Variation_31();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIGetSourceRow_GetSourceRow)
#define THE_CLASS TCIGetSourceRow_GetSourceRow
BEG_TEST_CASE(TCIGetSourceRow_GetSourceRow, TCIGetSourceRow, L"Test GetSourceRow method")
	TEST_VARIATION(1, 		L"IRow_Open riid all Mandatory interfaces")
	TEST_VARIATION(2, 		L"IBindResource riid all Mandatory interfaces")
	TEST_VARIATION(3, 		L"ICreateRow riid all Mandatory interfaces")
	TEST_VARIATION(4, 		L"IScopedOperations riid all Mandatory interfaces")
	TEST_VARIATION(5, 		L"NULL ppIRow")
	TEST_VARIATION(7, 		L"IRow from ICommand riid all Mandatory Interfaces")
	TEST_VARIATION(8, 		L"IRow from IOpenRwst riid all Mandatory Interfaces")
	TEST_VARIATION(9, 		L"Sequence - 2 streams from IRow_Open")
	TEST_VARIATION(12, 		L"Verify Stream addrefs parent row")
	TEST_VARIATION(13, 		L"Mutliple open Row objects")
	TEST_VARIATION(14, 		L"Agg Stream")
	TEST_VARIATION(15, 		L"Agg Row A, unagg Stream")
	TEST_VARIATION(16, 		L"Agg Row A, unagg Stream, release source Row")
	TEST_VARIATION(17, 		L"Agg Row A, agg Stream B")
	TEST_VARIATION(18, 		L"Threads  GetSourceRow")
	TEST_VARIATION(19, 		L"Session IBindResource riid all Mandatory interfaces")
	TEST_VARIATION(20, 		L"Session ICreateRow riid all Mandatory interfaces")
	TEST_VARIATION(21, 		L"IRow from ICommand  Open Stream using GetColumns")
	TEST_VARIATION(22, 		L"IRow from IOpenRowset Open Stream using GetColumns")
	TEST_VARIATION(23, 		L"IRow from IGetRow Open Stream using GetColumns")
	TEST_VARIATION(24, 		L"IRow from IBindResource Open Stream using GetColumns")
	TEST_VARIATION(25, 		L"Threads, row from IGetRow, streams from GetColumns")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"Sequence - Multiple streams from IRow_GetColumns")
	TEST_VARIATION(29, 		L"Sequence - Multiple streams from IRow_Open and IRowGetColumns")
	TEST_VARIATION(30, 		L"Call GetSourceRow on stream whose parent is a rowset")
	TEST_VARIATION(31, 		L"GetSourceRow after SetColumns")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(2, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIGetSourceRow_IUnknown)
	TEST_CASE(2, TCIGetSourceRow_GetSourceRow)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIGetSourceRow_IUnknown)
//*-----------------------------------------------------------------------
//| Test Case:		TCIGetSourceRow_IUnknown - Test IUnknown
//| Created:  	10/4/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetSourceRow_IUnknown::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIGetSourceRow::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting IBindResource
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_1()
{ 
	BOOL			fPass = TEST_FAIL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( !pwszURL )
		return TEST_SKIPPED;

	TESTC_(StreamObject.InitStreamObject(m_pIBindResource, pwszURL), S_OK);
	fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	

CLEANUP:
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting IRow_Open from GetRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_2()
{ 
	BOOL			fPass = TEST_FAIL;
	HRESULT			hr;
	CStreamObject	StreamObject;

	// Get stream using IRow::Open on the default stream
	TEST2C_(hr = StreamObject.InitStreamObject(m_pCRowObject->pIRow()), S_OK, DB_E_BADCOLUMNID);
	if( SUCCEEDED(hr) )
		fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	
	else
		fPass = TEST_SKIPPED;

CLEANUP:
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting IScopedOperations
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_3()
{ 
	BOOL				fPass = TEST_FAIL;
	CStreamObject		StreamObject;
	IScopedOperations * pIScopedOperations = NULL;
		
	if(!VerifyInterface(m_pCRowObject->pIRow(), IID_IScopedOperations,
		ROW_INTERFACE,(IUnknown**)&pIScopedOperations))	
		return TEST_SKIPPED;

	TESTC_(StreamObject.InitStreamObject(pIScopedOperations), S_OK);
	fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting ICreateRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_4()
{	
	HRESULT			hr;
	BOOL			fPass = TEST_FAIL;
	CStreamObject	StreamObject;
	ICreateRow *	pICreateRow = NULL;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( !pwszURL )
		return TEST_SKIPPED;

	StreamObject.SetBindURLFlags(DBBINDURLFLAG_OPENIFEXISTS | DBBINDURLFLAG_READ);	
	if(!VerifyInterface(m_pIBindResource, IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&pICreateRow))		
		return TEST_SKIPPED;

	TEST2C_(hr = StreamObject.InitStreamObject(pICreateRow, pwszURL), S_OK, E_NOINTERFACE);

	if( hr == S_OK )
		fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	
	else
		fPass = TEST_SKIPPED;

CLEANUP:
	SAFE_RELEASE(pICreateRow);
	return fPass;
}
// }}




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting Session IBindResource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_IUnknown::Variation_5()
{ 
	BOOL			fPass = TEST_FAIL;
	IBindResource *	pIBindResource = NULL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( E_NOINTERFACE == GetSessionObject(IID_IBindResource, (IUnknown **)&pIBindResource) )
		return TEST_SKIPPED;

	if( !pwszURL )
		return TEST_SKIPPED;

	TESTC_(StreamObject.InitStreamObject(pIBindResource, pwszURL), S_OK);
	fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	

CLEANUP:
	SAFE_RELEASE(pIBindResource);
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting Session ICreateRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_IUnknown::Variation_8()
{ 
	BOOL			fPass = TEST_FAIL;
	ICreateRow *	pICreateRow = NULL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( E_NOINTERFACE == GetSessionObject(IID_ICreateRow, (IUnknown **)&pICreateRow) )
		return TEST_SKIPPED;

	if( !pwszURL )
		return TEST_SKIPPED;

	StreamObject.SetBindURLFlags(DBBINDURLFLAG_OPENIFEXISTS | DBBINDURLFLAG_READ);	
	TESTC_(StreamObject.InitStreamObject(pICreateRow, pwszURL), S_OK);

	fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	

CLEANUP:
	SAFE_RELEASE(pICreateRow);
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting IRow_GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_IUnknown::Variation_9()
{ 
	HRESULT			hr;
	BOOL			fPass = TEST_FAIL;
	CStreamObject	StreamObject;

	TEST3C_(hr = StreamObject.InitStreamObject(m_pCRowObject->pIRow(), NULL), S_OK, E_NOINTERFACE, DB_E_BADCOLUMNID);
	if( SUCCEEDED(hr) )
	{
		TESTC(DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE));
	}
	else
	{
		TWARNING(L"IGetSourceRow was not available on this object");
	}

	fPass = TEST_PASS;

CLEANUP:
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting IScopedOperations requesting requesting IID_IGetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_10()
{ 
	BOOL				fPass = TEST_FAIL;
	CStreamObject		StreamObject;
	IScopedOperations * pIScopedOperations = NULL;
		
	if(!VerifyInterface(m_pCRowObject->pIRow(), IID_IScopedOperations,
		ROW_INTERFACE,(IUnknown**)&pIScopedOperations))	
		return TEST_SKIPPED;

	TESTC_(StreamObject.InitStreamObject(pIScopedOperations, IID_IGetSourceRow), S_OK);
	fPass = DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DefaultTesting ICreateRow requesting requesting IID_IGetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_IUnknown::Variation_11()
{	
	HRESULT			hr;
	BOOL			fPass = TEST_FAIL;
	CStreamObject	StreamObject;
	ICreateRow *	pICreateRow = NULL;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	StreamObject.SetBindURLFlags(DBBINDURLFLAG_OPENIFEXISTS | DBBINDURLFLAG_READ);	
	if(!VerifyInterface(m_pCRowObject->pIRow(), IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&pICreateRow))		
		return TEST_SKIPPED;

	if( !pwszURL )
		return TEST_SKIPPED;

	TEST2C_(hr = StreamObject.InitStreamObject(pICreateRow, pwszURL, IID_IStream), S_OK, E_NOINTERFACE);
	if( hr == S_OK )
	{
		TESTC(DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE));
	}

	StreamObject.ReleaseStreamObject();

	TEST2C_(hr = StreamObject.InitStreamObject(pICreateRow, pwszURL, IID_IGetSourceRow), S_OK, E_NOINTERFACE);
	if( hr == S_OK )
	{
		TESTC(DefaultObjectTesting(StreamObject.pIGetSourceRow(), STREAM_INTERFACE));
		fPass = TEST_PASS;
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:
	SAFE_RELEASE(pICreateRow);
	return fPass;
}
// }}




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIGetSourceRow_IUnknown::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIGetSourceRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCIGetSourceRow_GetSourceRow)
//*-----------------------------------------------------------------------
//| Test Case:		TCIGetSourceRow_GetSourceRow - Test GetSourceRow method
//|	Created:			10/11/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetSourceRow_GetSourceRow::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIGetSourceRow::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRow_Open riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_1()
{
	HRESULT			hr;
	CStreamObject	StreamObject;
	
	// Open using IRow::Open on the default stream
	TEST2C_(hr = StreamObject.InitStreamObject(m_pCRowObject->pIRow()), S_OK, DB_E_BADCOLUMNID);		
	if( SUCCEEDED(hr) )
	{
		TESTC_(TestGetSourceRow(&StreamObject), S_OK);
	}
	else
		return TEST_SKIPPED;

CLEANUP:

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IBindResource riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_2()
{ 
	HRESULT			hr;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( !pwszURL )
		return TEST_SKIPPED;
	
	TESTC_(hr = StreamObject.InitStreamObject(m_pIBindResource, pwszURL), S_OK);
	TESTC_(hr = TestGetSourceRow(&StreamObject), S_OK);	

CLEANUP:	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc ICreateRow riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_3()
{ 
	HRESULT			hr;
	BOOL			fPass = TEST_FAIL;
	ICreateRow *	pICreateRow = NULL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( !pwszURL )
		return TEST_SKIPPED;

	StreamObject.SetBindURLFlags(DBBINDURLFLAG_OPENIFEXISTS | DBBINDURLFLAG_READ);	

	TESTC(VerifyInterface(m_pIBindResource, IID_ICreateRow,
		STREAM_INTERFACE,(IUnknown**)&pICreateRow))		

	TEST2C_(hr = StreamObject.InitStreamObject(pICreateRow, pwszURL), S_OK, E_NOINTERFACE);
	if( S_OK == hr )
	{
		TESTC_(TestGetSourceRow(&StreamObject), S_OK);	
		fPass = TEST_PASS;
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:
	SAFE_RELEASE(pICreateRow);
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IScopedOperations riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_4()
{ 
	CStreamObject		StreamObject;
	IScopedOperations * pIScopedOperations = NULL;
	
	if(!VerifyInterface(m_pCRowObject->pIRow(), IID_IScopedOperations,
		ROW_INTERFACE,(IUnknown**)&pIScopedOperations))	
		return TEST_SKIPPED;
	
	TESTC_(StreamObject.InitStreamObject(pIScopedOperations), S_OK);
	TESTC_(TestGetSourceRow(&StreamObject), S_OK);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc NULL ppIRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_5()
{ 
	HRESULT			hr = S_OK;
	CStreamObject	StreamObject;

	TEST2C_(hr = StreamObject.InitStreamObject(m_pCRowObject->pIRow()), S_OK, DB_E_BADCOLUMNID);
	if( SUCCEEDED(hr) )
	{
		TESTC_(hr = StreamObject.GetSourceRow(IID_IUnknown, NULL), E_INVALIDARG);
	}
	else
		return TEST_SKIPPED;

CLEANUP:	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//--------------------------------------------------------------------
// @mfunc IRow from ICommand riid all Mandatory Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_7()
{
	CRowset			RowsetA;
	CRowObject		RowObjectA;
	CStreamObject	StreamObjectA;
	IRow *			pIRow = NULL;
	HRESULT			hr;
	BOOL			fPass = TEST_PASS;

	//Create a row object from ICommand::Execute
	TEST2C_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRow, 0,
				NULL, (IUnknown**)&pIRow), S_OK, DB_S_NOTSINGLETON);

	TESTC_(RowObjectA.SetRowObject(pIRow),S_OK);
	TEST2C_(hr = StreamObjectA.InitStreamObject(pIRow), S_OK, DB_E_BADCOLUMNID);

	if( SUCCEEDED(hr) )
	{
		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);

		TESTC(DefaultObjectTesting(StreamObjectA.pIGetSourceRow(), STREAM_INTERFACE));	
	}
	else
		fPass = TEST_SKIPPED;


CLEANUP:
	SAFE_RELEASE(pIRow);
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//--------------------------------------------------------------------
// @mfunc IRow from IOpenRwst riid all Mandatory Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIGetSourceRow_GetSourceRow::Variation_8()
{
	CRowset			RowsetA;
	CRowObject		RowObjectA;
	CStreamObject	StreamObjectA;
	IRow *			pIRow = NULL;
	HRESULT			hr;
	BOOL			fPass = TEST_PASS;

	//Create a row object from IOpenRowset
	TEST2C_(RowsetA.pTable()->CreateRowset(USE_OPENROWSET, IID_IRow, 0,
				NULL, (IUnknown**)&pIRow), S_OK, DB_S_NOTSINGLETON);

	TESTC_(RowObjectA.SetRowObject(pIRow),S_OK);
	TEST2C_(hr = StreamObjectA.InitStreamObject(pIRow), S_OK, DB_E_BADCOLUMNID);
	if( SUCCEEDED(hr) )
	{
		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);

		TESTC(DefaultObjectTesting(StreamObjectA.pIGetSourceRow(), STREAM_INTERFACE));	
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:

	SAFE_RELEASE(pIRow);
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Sequence - 2 streams from IRow_Open
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_9()
{ 
	const ULONG cStreamObjects = 20;
	TESTC_(TestMultipleStreams(cStreamObjects, EROWOPEN), S_OK);

CLEANUP:
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Verify Stream addrefs parent row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_12()
{ 
	IUnknown *		pIUnknown = NULL;
	IRow *			pIRow = NULL;
	CStreamObject	StreamObjectA;
	HRESULT			hr;
	BOOL			fPass = TEST_PASS;

	TEST2C_(hr = BindRow(NULL, (IUnknown **)&pIUnknown), S_OK, S_FALSE);
	if( hr == S_OK )
	{
		TESTC(VerifyInterface(pIUnknown, IID_IRow, ROW_INTERFACE,(IUnknown**)&pIRow))	
		TESTC_(StreamObjectA.InitStreamObject(pIRow), S_OK);
		SAFE_RELEASE(pIUnknown);
		SAFE_RELEASE(pIRow);

		TESTC_(StreamObjectA.GetSourceRow(IID_IRow, (IUnknown **)&pIRow), S_OK);
		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);
		
		TESTC(DefaultObjectTesting(StreamObjectA.pIGetSourceRow(), STREAM_INTERFACE));	
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRow);
	return fPass;	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Mutliple open Row objects
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_13()
{ 
	const ULONG		cOpenRows = 5;
	CRowset			RowsetA;
	CRowObject		RowObjects[cOpenRows];
	CStreamObject	StreamObjects[cOpenRows];
	
	HRESULT			hr = S_OK;
	IUnknown *		pIRow = NULL;
	ULONG			cRowsObtained = 0;
	ULONG			i = 0;
	HROW			rghRows[cOpenRows];
	HROW			hRow = NULL;
	
	if( (m_ulpOleObjects & DBPROPVAL_OO_ROWOBJECT) == 0)
		return TEST_SKIPPED;
	
	//Create RowsetA
	//May require IRowsetLocate to position on Blobs
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(DBPROP_CANHOLDROWS),S_OK);

	//Create Row objects from the a set of rows belonging to same rowset
	for(i=0; i<cOpenRows; i++)
	{		
		//Grab the First row...
		TESTC_(RowsetA.GetNextRows(&rghRows[i]), S_OK);

		//Create Row Objects...
		TEST2C_(RowObjects[i].CreateRowObject(RowsetA.pIRowset(), rghRows[i]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

		TESTC_(StreamObjects[i].InitStreamObject(RowObjects[i].pIRow()), S_OK);

		RowsetA.ReleaseRows(rghRows[i]);
	}

	//Now try and Obtain the Source Rows
	for(i=0; i<cOpenRows; i++)
	{
		//Try to get back to the creating rowset
		TEST2C_(hr = StreamObjects[i].GetSourceRow(IID_IRow, &pIRow), S_OK, DB_E_NOSOURCEOBJECT);
		if(SUCCEEDED(hr))
		{
			//Make sure its returning the original object
			TESTC(VerifyEqualInterface(pIRow, RowObjects[i].pIRow()));
		}
		SAFE_RELEASE(pIRow);		
	}
	
CLEANUP:
	SAFE_RELEASE(pIRow);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Agg Stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_14()
{ 
	HRESULT			hr;
	BOOL			fPass = TEST_FAIL;
	IUnknown *		pUnkInner = NULL;
	IGetSourceRow * pIGetSourceRow = NULL;
	CAggregate		Aggregate(m_pIBindResource);
	CStreamObject	StreamObject;
	DBID			dbid = DBROWCOL_DEFAULTSTREAM;

	hr = m_pCRowObject->pIRow()->Open(&Aggregate, &dbid, DBGUID_STREAM,
			0, IID_IUnknown, &pUnkInner);
	if( DB_E_BADCOLUMNID == hr )
		return TEST_SKIPPED;

	Aggregate.SetUnkInner(pUnkInner);
	
	if(Aggregate.VerifyAggregationQI(hr, IID_IGetSourceRow, (IUnknown**)&pIGetSourceRow))
	{
		TESTC_(StreamObject.SetStreamObject(pIGetSourceRow), S_OK);
		TESTC_(TestGetSourceRow(&StreamObject), S_OK);	
	}

CLEANUP:

	SAFE_RELEASE(pUnkInner);
	SAFE_RELEASE(pIGetSourceRow);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Agg Row A, unagg Stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_15()
{ 
	HRESULT			hr;
	IRow *			pIRow = NULL;
	IUnknown *		pIUnknown = NULL;
	IUnknown *		pUnkInner = NULL;
	CAggregate		Aggregate(m_pIBindResource);
	CStreamObject	StreamObjectA;

	hr = BindRow(&Aggregate, &pUnkInner);
	if( S_FALSE == hr )
		return TEST_SKIPPED;

	Aggregate.SetUnkInner(pUnkInner);

	if(Aggregate.VerifyAggregationQI(hr, IID_IRow, (IUnknown**)&pIRow))
	{
		TESTC_(StreamObjectA.InitStreamObject(pIRow), S_OK);

		TESTC_(hr = StreamObjectA.pIGetSourceRow()->GetSourceRow(IID_IAggregate, &pIUnknown), S_OK);
		TESTC(VerifyEqualInterface(pIUnknown, pIRow));

		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);

		TESTC(DefaultObjectTesting(StreamObjectA.pIGetSourceRow(), STREAM_INTERFACE));	
	}

CLEANUP:

	SAFE_RELEASE(pUnkInner);
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRow);

	return TEST_PASS;		
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Agg Row A, unagg Stream, release source Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_16()
{ 
	HRESULT			hr = E_FAIL;
	IRow *			pIRow = NULL;
	IUnknown *		pUnkInner = NULL;
	CAggregate		Aggregate(m_pIBindResource);
	CStreamObject	StreamObjectA;

	hr = BindRow(&Aggregate, &pUnkInner);
	if( S_FALSE == hr )
		return TEST_SKIPPED;

	Aggregate.SetUnkInner(pUnkInner);

	if(Aggregate.VerifyAggregationQI(hr, IID_IRow, (IUnknown**)&pIRow))
	{
		TESTC_(StreamObjectA.InitStreamObject(pIRow), S_OK);
		SAFE_RELEASE(pIRow);
		TESTC(Aggregate.Release() > 0);

		hr = TestGetSourceRow(&StreamObjectA);
		Aggregate.AddRef();
	}
	else
		hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pUnkInner);
	SAFE_RELEASE(pIRow);

	return ( S_OK == hr );	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Agg Row A, agg Stream B
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_17()
{ 
	HRESULT			hr;
	IRow *			pIRow = NULL;
	IUnknown *		pRowUnkInner = NULL;
	IUnknown *		pStreamUnkInner = NULL;
	IGetSourceRow *	pIGetSourceRow = NULL;
	DBID			dbid = DBROWCOL_DEFAULTSTREAM;
	CAggregate		AggregateRow(m_pIBindResource);
	CAggregate		AggregateStream(m_pIBindResource);
	CStreamObject	StreamObjectA;

	hr = BindRow(&AggregateRow, &pRowUnkInner);
	if( S_FALSE == hr )
		return TEST_SKIPPED;

	AggregateRow.SetUnkInner(pRowUnkInner);

	if(AggregateRow.VerifyAggregationQI(hr, IID_IRow, (IUnknown**)&pIRow))
	{
		hr = pIRow->Open(&AggregateStream, &dbid, DBGUID_STREAM,
				0, IID_IUnknown, &pStreamUnkInner);
		AggregateStream.SetUnkInner(pStreamUnkInner);
		
		TESTC(AggregateStream.VerifyAggregationQI(hr, IID_IGetSourceRow, (IUnknown**)&pIGetSourceRow));
		TESTC_(StreamObjectA.SetStreamObject(pIGetSourceRow), S_OK);
		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);	
	}

CLEANUP:

	SAFE_RELEASE(pRowUnkInner);
	SAFE_RELEASE(pStreamUnkInner);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIGetSourceRow);

	return TEST_PASS;		
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Threads  GetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_18()
{ 
	HRESULT	hr;
	BOOL	fPass = TEST_PASS;
	INIT_THREADS(MAX_THREADS);	
	
	//Setup Thread Arguments
	THREADARG T1Arg = { this };

	//Setup Stream object
	TEST2C_(hr = m_pCStreamObject->InitStreamObject(m_pCRowObject->pIRow()), S_OK, DB_E_BADCOLUMNID);
	if( S_OK == hr )
	{
		//Create Threads
		CREATE_THREADS(Thread_VerifyGetSourceRow, &T1Arg);

		START_THREADS();
		END_THREADS();	
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:

	m_pCStreamObject->ReleaseStreamObject();

	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Session IBindResource riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_19()
{ 
	HRESULT			hr;
	IBindResource *	pIBindResource = NULL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);
	
	if( !pwszURL )
		return TEST_SKIPPED;

	if( E_NOINTERFACE == GetSessionObject(IID_IBindResource, (IUnknown **)&pIBindResource))
		return TEST_SKIPPED;

	TESTC_(hr = StreamObject.InitStreamObject(pIBindResource, pwszURL), S_OK);
	TESTC_(hr = TestGetSourceRow(&StreamObject), S_OK);	

CLEANUP:	
	SAFE_RELEASE(pIBindResource);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Session ICreateRow riid all Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_20()
{ 
	HRESULT			hr;
	ICreateRow *	pICreateRow = NULL;
	CStreamObject	StreamObject;
	WCHAR *			pwszURL = GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);

	if( !pwszURL )
		return TEST_SKIPPED;

	if( E_NOINTERFACE == GetSessionObject(IID_ICreateRow, (IUnknown **)&pICreateRow))
		return TEST_SKIPPED;

	StreamObject.SetBindURLFlags(DBBINDURLFLAG_OPENIFEXISTS | DBBINDURLFLAG_READ);		
	TESTC_(hr = StreamObject.InitStreamObject(pICreateRow, pwszURL), S_OK);
	TESTC_(hr = TestGetSourceRow(&StreamObject), S_OK);	

CLEANUP:	
	SAFE_RELEASE(pICreateRow);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc IRow from ICommand  Open Stream using GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_21()
{ 
	CRowset		RowsetA;
	CRowObject	RowObjectA;
	IRow *		pIRow = NULL;

	//Create a row object from ICommand::Execute
	TEST2C_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRow, 0,
				NULL, (IUnknown**)&pIRow), S_OK, DB_S_NOTSINGLETON);

	TESTC_(RowObjectA.SetRowObject(pIRow),S_OK);

	TESTC(VerifyBLOBSFromRowObject(&RowObjectA));

CLEANUP:
	SAFE_RELEASE(pIRow);	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc IRow from IOpenRowset Open Stream using GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_22()
{ 
	CRowset		RowsetA;
	CRowObject	RowObjectA;
	IRow *		pIRow = NULL;

	//Create a row object from IOpenRowset
	TEST2C_(RowsetA.pTable()->CreateRowset(USE_OPENROWSET, IID_IRow, 0,
				NULL, (IUnknown**)&pIRow), S_OK, DB_S_NOTSINGLETON);

	TESTC_(RowObjectA.SetRowObject(pIRow),S_OK);
	
	TESTC(VerifyBLOBSFromRowObject(&RowObjectA));

CLEANUP:

	SAFE_RELEASE(pIRow);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc IRow from IGetRow Open Stream using GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_23()
{ 
	CRowObject	RowObjectA;

	TESTC_(RowObjectA.SetRowObject(m_pCRowObject->pIRow()),S_OK);

	TESTC(VerifyBLOBSFromRowObject(&RowObjectA));

CLEANUP:
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc IRow from IBindResource Open Stream using GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_24()
{ 
	CRowObject	RowObjectA;
	IUnknown *	pUnk = NULL;
	IRow *		pIRow = NULL;
	HRESULT		hr;
	BOOL		fPass = TEST_PASS;

	TEST2C_(hr = BindRow(NULL, (IUnknown **)&pUnk), S_OK, S_FALSE);
	if( hr == S_OK )
	{
		TESTC(VerifyInterface(pUnk, IID_IRow, ROW_INTERFACE,(IUnknown**)&pIRow))	
		
		TESTC_(RowObjectA.SetRowObject(pIRow),S_OK);
		
		TESTC(VerifyBLOBSFromRowObject(&RowObjectA));
	}
	else
		fPass = TEST_SKIPPED;

CLEANUP:

	SAFE_RELEASE(pUnk);

	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Threads, row from IGetRow, streams from GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_25()
{ 
	INIT_THREADS(MAX_THREADS);	
	HRESULT			hr;
	CStreamObject	StreamObjectA;

	hr = StreamObjectA.InitStreamUsingGetColumns(m_pCRowObject->pIRow());
	if( FAILED(hr))
	{
		if( E_NOINTERFACE == hr )
			return TEST_SKIPPED;
		else
			return TEST_FAIL;
	}

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &StreamObjectA };

	//Create Threads
	CREATE_THREADS(Thread_GetColumns_VerifyGetSourceRow, &T1Arg);

	START_THREADS();
	END_THREADS();	

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_26()
{ 
	TBEGIN

	HRESULT			hr;
	HROW			hRow = DB_NULL_HROW;
	CRowset			RowsetA;
	CRowObject		RowObjectA;
	CStreamObject	StreamObjectA;
	CStreamObject	StreamObjectB;

	if( (m_ulpOleObjects & DBPROPVAL_OO_ROWOBJECT) == 0)
		return TEST_SKIPPED;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.	
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	TESTC_(RowsetA.CreateRowset(), S_OK);
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(hr = StreamObjectA.InitStreamUsingGetColumns(RowObjectA.pIRow()), S_OK, E_NOINTERFACE);
	TEST2C_(hr = StreamObjectB.InitStreamUsingGetColumns(RowObjectA.pIRow()), S_OK, E_NOINTERFACE);

	if( S_OK == hr )
	{
		// Release the Row Object
		RowObjectA.ReleaseRowObject();		

		TESTC_(TestGetSourceRow(&StreamObjectA), S_OK);	
		StreamObjectA.ReleaseStreamObject();
		TESTC_(TestGetSourceRow(&StreamObjectB), S_OK);	
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);		

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_27()
{ 
	INIT_THREADS(2);	
	HRESULT			hr;
	HROW			hRow = DB_NULL_HROW;
	CRowset			RowsetA;
	CRowObject		RowObjectA;
	CStreamObject	StreamObjectA;

	if( (m_ulpOleObjects & DBPROPVAL_OO_ROWOBJECT) == 0)
		return TEST_SKIPPED;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.	
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	TESTC_(RowsetA.CreateRowset(), S_OK);
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(hr = StreamObjectA.InitStreamUsingGetColumns(RowObjectA.pIRow()), S_OK, E_NOINTERFACE);
	RowObjectA.ReleaseRowObject();

	if( S_OK == hr )
	{
		//Setup Thread Arguments
		THREADARG T1Arg = { this, &StreamObjectA };
		THREADARG T2Arg = { &RowsetA, &hRow};

		//Create Threads
		CREATE_FIRST_THREADS(Thread_GetColumns_VerifyGetSourceRow, &T1Arg);
		CREATE_SECOND_THREADS(Thread_ReleaseRow, &T2Arg);

		START_THREADS();
		END_THREADS();	
	}

CLEANUP:
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple streams from IRow_GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_28()
{ 
	const ULONG cStreamObjects = 20;
	TESTC_(TestMultipleStreams(cStreamObjects, EROWGETCOL), S_OK);

CLEANUP:
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple streams from IRow_Open and IRowGetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_29()
{ 
	const ULONG cStreamObjects = 20;
	TESTC_(TestMultipleStreams(cStreamObjects, EROWBOTH), S_OK);

CLEANUP:
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Call GetSourceRow on stream whose parent is a rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_30()
{ 
	HRESULT				hr;
	HROW				hRow = DB_NULL_HROW;
	CRowset				RowsetA;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	void *				pData = NULL;
	ULONG_PTR			cBytes = 0;
	DBCOUNTITEM			cBindings = 0;
	DBBINDING *			rgBindings = NULL;
	IUnknown *			pUnkStorage = NULL;
	IGetSourceRow *		pIGetSourceRow = NULL;

	if( (m_ulpOleObjects & DBPROPVAL_OO_ROWOBJECT) == 0)
		return TEST_SKIPPED;


	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	TESTC_(RowsetA.CreateRowset(), S_OK);	
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, BLOB_IID_ISEQSTREAM), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data
	TESTC_(hr = RowsetA.pIRowset()->GetData(hRow, hAccessor, pData), S_OK)	
	//Display any binding errors and status'
	TESTC(VerifyBindings(hr, RowsetA.pIAccessor(), hAccessor, pData));

	if( GetStorageObject(cBindings, rgBindings, pData, IID_ISequentialStream, (IUnknown **)&pUnkStorage) )

	{
		if( S_OK == pUnkStorage->QueryInterface(IID_IGetSourceRow, (void **)&pIGetSourceRow) )
		{
			IUnknown *	pUnkSource = NULL;

			// Provider may choose to allow all storage objects to support IGetSourceRow.
			// However, if a storage object's parent was a rowset instead of row, they should
			// return DB_E_NOSOURCEOBJECT.
			TESTC(pIGetSourceRow != NULL);
			TESTC_(pIGetSourceRow->GetSourceRow(IID_IUnknown, &pUnkSource), DB_E_NOSOURCEOBJECT);
			SAFE_RELEASE(pUnkSource);
		}
	}

CLEANUP:

	SAFE_RELEASE(pIGetSourceRow);
	SAFE_RELEASE(pUnkStorage);
	SAFE_FREE(pData);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Call GetSourceRow after IRow::SetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetSourceRow_GetSourceRow::Variation_31()
{ 
	HRESULT				hr;
	BOOL				fPass = TEST_FAIL;
	HROW				hRow = DB_NULL_HROW;
	CRowset				RowsetA;
	CRowObject			RowObjectA;
	CStreamObject		StreamObjectA;
	DBCOUNTITEM			cColAccess = 0;
	DBCOLUMNACCESS *	rgColAccess = NULL;
	void *				pData = NULL;
	ULONG_PTR			cbRowSize = 0;
	IRow *				pIRow = NULL;
	DBCOUNTITEM			ulChangeSeed;

	if( (m_ulpOleObjects & DBPROPVAL_OO_ROWOBJECT) == 0)
		return TEST_SKIPPED;

	//Use a new rowset, and ask for an updatable cursor, 
	//so we can update the data
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	RowsetA.SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);

	TESTC_(RowsetA.CreateRowset(), S_OK);
	ulChangeSeed = RowsetA.GetTotalRows()+1;
	
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(hr = StreamObjectA.InitStreamUsingGetColumns(RowObjectA.pIRow()), S_OK, E_NOINTERFACE);
	
	TESTC_(RowsetA.ReleaseRows(hRow), S_OK);

	// Bind all BLOB columns
	TESTC_(RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, &cbRowSize, 
				BLOB_COLS_BOUND, BLOB_IID_ISEQSTREAM), S_OK);
	if( cColAccess == 0 )
	{
		fPass = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	TESTC_(RowObjectA.FillColAccess(RowsetA.pTable(), cColAccess, rgColAccess, ulChangeSeed), S_OK);
	TESTC_(RowObjectA.SetColumns(cColAccess, rgColAccess), S_OK)

	// Changing the streams may zombie the row
	TEST2C_(StreamObjectA.GetSourceRow(IID_IRow, (IUnknown **)&pIRow), S_OK, E_UNEXPECTED);
	SAFE_RELEASE(pIRow);

	// Release all Storage references and obtain a new one
	StreamObjectA.ReleaseStreamObject();
	TEST2C_(hr = StreamObjectA.InitStreamUsingGetColumns(RowObjectA.pIRow()), S_OK, E_NOINTERFACE);
	TEST2C_(StreamObjectA.GetSourceRow(IID_IRow, (IUnknown **)&pIRow), S_OK, DB_E_NOSOURCEOBJECT);

	fPass = TEST_PASS;

CLEANUP:
	
	SAFE_RELEASE(pIRow);

	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);

	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetSourceRow_GetSourceRow::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIGetSourceRow::Terminate());
}	// }}
// }}
// }}
