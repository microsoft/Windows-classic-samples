//----------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IBindResource.CPP | The test module for IBindResource
//

//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#include "modstandard.hpp"		// Standard headers			
#include "IBindResource.h"		// IBindResource testmodule header
#include "ExtraLib.h"


//*---------------------------------------------------------------------
// Module Values
//*------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x7e9d3770, 0x2aff, 0x11d2, { 0x88, 0xc9, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("IBindResource");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IBindResource Interface");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------
//GLOBALS
//--------------------------------------------------------------
ULONG g_cNewURL=0;

CAuthenticate*	g_pCAuth = NULL;


//*------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN
	IBindResource*	pIBR = NULL;
	IBindResource*	pIBindResource = NULL;
	ICreateRow*		pICreateRow = NULL;

	g_cNewURL = 0;

	TESTC(ModuleCreateDBSession(pThisTestModule))

	if(!IsUsableInterface(BINDER_INTERFACE, IID_IBindResource))
	{
		odtLog<<L"SKIP: CONF_STRICT specified and IBindResource & ICreateRow are Level-1 interfaces.\n";
		return TEST_SKIPPED;
	}

	pIBR = GetModInfo()->GetRootBinder();
	TESTC_PROVIDER(pIBR != NULL);

	//Check support in provider
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IBindResource,
		SESSION_INTERFACE,(IUnknown**)&pIBindResource))
	{
		odtLog<<L"SKIP: IBindResource is not supported by the provider.\n";
		return TEST_SKIPPED;
	}

	//If IBindResource is supported, then make sure ICreateRow
	//is also supported.
	COMPAREW(VerifyInterface(pThisTestModule->m_pIUnknown2, IID_ICreateRow,
		SESSION_INTERFACE,(IUnknown**)&pICreateRow), TRUE);

	g_pCAuth = new CAuthenticate();
	COMPARE(g_pCAuth != NULL, TRUE);

CLEANUP:
	SAFE_RELEASE(pICreateRow);
	SAFE_RELEASE(pIBindResource);
	TRETURN
}

//*---------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	SAFE_RELEASE(g_pCAuth);

	return ModuleReleaseDBSession(pThisTestModule);
}


////////////////////////////////////////////////////////////////////////////
//  TCBase  -  Class for reusing Test Cases.
// This is one of the base classes from which all the Test Case
// classes will inherit. It is used to duplicate test cases, yet
// maintain a distinct identity for each.
//
////////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(TC_RBINDER); }

	//methods
	virtual void SetTestCaseParam(ETESTCASE eTestCase)
	{
		m_eTestCase = eTestCase;
		switch(eTestCase)
		{
			case TC_RBINDER:	//Root Binder
				break;
			case TC_PBINDER:	//Provider Binder
				break;
			case TC_SESSION:	//Session object
				break;
			case TC_ROW:		//Row object
				break;
			default:
				ASSERT(!L"Unhandled Type of Test Case");
				break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
};


////////////////////////////////////////////////////////////////////////
//Zombie Class  -  Class for Transaction Test Cases.
//
////////////////////////////////////////////////////////////////////////
class Zombie : public CTransaction
{			 
public:
	Zombie(const LPWSTR wszTestCaseName): CTransaction(wszTestCaseName){};
	int TestTxnCreateRow(BOOL bCommit, BOOL fRetaining);
};


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::TestTxnCreateRow(BOOL bCommit,BOOL fRetaining)
{
	TBEGIN
	HRESULT			hr				= E_FAIL;
	HRESULT			ExpectedHr		= E_UNEXPECTED;
	DBCOUNTITEM		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;
	ICreateRow*		pICreateRow		= NULL;
	IRow*			pIRow			= INVALID(IRow*);
	WCHAR*			pwszRowURL		= NULL;

	TESTC(StartTransaction(USE_OPENROWSET, (IUnknown **)&pICreateRow, 
		0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE));

	//Make sure everything still works after commit or abort
	if(GetModInfo()->GetRootURL())
		pwszRowURL = wcsDuplicate(GetModInfo()->GetRootURL());
	else
		pwszRowURL = wcsDuplicate(GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE));

	TESTC(pwszRowURL != NULL)
	TESTC_(hr = pICreateRow->CreateRow(NULL, pwszRowURL, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS, DBGUID_ROW, 
		IID_IRow, NULL, NULL, NULL, NULL, (IUnknown**)&pIRow), S_OK)

	if(SUCCEEDED(hr))
	{
		TESTC(pIRow != NULL)
		SAFE_RELEASE(pIRow);
	}
	else
		TESTC(!pIRow)

	if (bCommit)
	{
		//Commit the transaction, with retention as specified
		TESTC(GetCommit(fRetaining));
	}
	else
	{
		//Abort the transaction, with retention as specified
		TESTC(GetAbort(fRetaining));
	}

	// Make sure everything still works after commit or abort
	if ((bCommit && m_fCommitPreserve) ||
		((!bCommit) && m_fAbortPreserve))
		ExpectedHr = S_OK;

	// Test zombie
	CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);

	//Make sure everything still works after commit or abort
	CHECK(hr=pICreateRow->CreateRow(NULL, pwszRowURL, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS, DBGUID_ROW, 
		IID_IRow, NULL, NULL, NULL, NULL, (IUnknown**)&pIRow),ExpectedHr);

	if(FAILED(hr))
		TESTC(!pIRow)
	else
		TESTC(DefaultObjectTesting(pIRow, ROW_INTERFACE))
		
CLEANUP:
	if (rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(rghRows);
	}
	SAFE_FREE(pwszRowURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pICreateRow);

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	CleanUpTransaction((fRetaining) ? S_OK : XACT_E_NOTRANSACTION);

	TRETURN
} //TestTxnCreateRow



///////////////////////////////////////////////////////////////
//CAuthenticate Class  -  Wrapper object for IAuthenticate 
//							interface.
//
///////////////////////////////////////////////////////////////

CAuthenticate::~CAuthenticate()
{
	//Shouldn't have any references left
	COMPARE(m_cRef, 0);
}

HRESULT STDMETHODCALLTYPE CAuthenticate::QueryInterface(REFIID riid, void **ppvObject)
{
	if(! ppvObject)
		return E_INVALIDARG;

	*ppvObject = NULL;
	
	if (riid == IID_IUnknown)
		*ppvObject = (IUnknown*)this;
	if (riid == IID_IAuthenticate)
		*ppvObject = (IAuthenticate*)this;

	if(*ppvObject)
	{
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CAuthenticate::AddRef(void)
{
	return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CAuthenticate::Release(void)
{
	ASSERT(m_cRef);

	if(--m_cRef)
		return m_cRef;
	
	delete this;
	return 0;
}

HRESULT STDMETHODCALLTYPE CAuthenticate::Authenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
	if(!phwnd || !ppszUsername || !ppszPassword)
		return E_INVALIDARG;

	GetModInfo()->GetInitStringValue(L"USERID", ppszUsername);
	GetModInfo()->GetInitStringValue(L"PASSWORD", ppszPassword);
	
	*phwnd       = GetDesktopWindow();

	return S_OK;
}



///////////////////////////////////////////////////////////////
//TCBindAndCreate Class - Class for IBindResource and ICreateRow
//						   Test Cases.
//
///////////////////////////////////////////////////////////////
class TCBindAndCreate : public CSessionObject, public TCBase
{
public:

	//Constructor
	TCBindAndCreate(WCHAR* pwszTestCaseName);

	//Destructor
	virtual ~TCBindAndCreate();

protected:

//VARIABLES...

	WCHAR*				m_rgURLs[INVALID_OBJECT];
	WCHAR*				m_pwszCmdURL;
	DBCOUNTITEM			m_cRowsetURLs;
	WCHAR**				m_rgRowsetURLs;
	WCHAR*				m_rgNewURLs[NEWURLS];
	WCHAR*				m_pwszNewFolder;
	ULONG_PTR			m_lGenerateURL;

//INTERFACES...

	IBindResource*			m_pIBindResource;
	ICreateRow*				m_pICreateRow;
	IDBBinderProperties*	m_pIDBBinderProperties;

//METHODS...

	//Initialize routine for most test cases.
	BOOL	InitTC();

	//Terminate routine for most test cases.
	BOOL	TermTC();

	//Release all member pointers to interfaces.
	BOOL	ReleaseAll();

	//Wrapper for m_pIBindResource->Bind(...)
	HRESULT	BindResource(
		LPCOLESTR			pwszURL,
		REFGUID				rguid,
		REFIID				riid,
		DBBINDURLSTATUS*	pdwBindStatus,
		IUnknown**			ppUnk,
		DBBINDURLFLAG		dwBindFlags = DBBINDURLFLAG_READ,
		DBIMPLICITSESSION*	pImplSession = NULL,
		IUnknown*			pUnkOuter = NULL,
		IAuthenticate*		pAuthenticate = NULL
		);

	//Wrapper for m_pICreateRow::CreateRow(...)
	HRESULT	CreateRow(
		LPCOLESTR			pwszURL,
		REFGUID				rguid,
		REFIID				riid,
		DBBINDURLSTATUS*	pdwBindStatus,
		LPOLESTR*			ppwszNewURL,
		IUnknown**			ppUnk,
		DBBINDURLFLAG		dwBindFlags = DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS,
		DBIMPLICITSESSION*	pImplSession = NULL,
		IUnknown*			pUnkOuter = NULL,
		IAuthenticate*		pAuthenticate = NULL
		);

	//Get the Root Binder, requesting IBindResource.
	//Then QI for IDBBinderProperties and ICreateRow.
	BOOL	GetRootBinder();

	//Get the Provider Binder, requesting IBindResource.
	//Then QI for IDBBinderProperties and ICreateRow.
	BOOL	GetProvBinder();

	//Create an instance of the Root Binder, then Bind to a session and 
	//obtain the IBindResource and ICreateRow interfaces on the
	//SESSION object (if supported).
	HRESULT	GetBindOnSession();

	//Create an instance of the Root Binder, then Bind to a row and 
	//obtain the IBindResource and ICreateRow interfaces on the
	//ROW object (if supported).
	HRESULT	GetBindOnRowObj();

	//Set the Initialization properties using IDBBinderProperties.
	BOOL	SetInitProps(IDBBinderProperties* pIDBBindProp);

	//Set m_rgURLs.
	BOOL	InitializeURLs();

	//Set m_rgRowsetURLs.
	BOOL	InitializeRowsetURLs();

	//Set m_rgNewURLs.
	BOOL	InitializeNewURLs();

	//Remove any new URLs that may have been created.
	BOOL	CleanupNewURLs();

	//Get a session object. 
	//Required for calling CreateTable(...).
	BOOL	GetSession();

	//Fetch the specified rows, get their data and compare.
	BOOL		GetDataAndCompare(
					DBROWOFFSET		cSkipRows,			//[IN] num of rows to skip.
					DBROWCOUNT		cGetRows,			//[IN] num of rows to fetch.
					DBCOUNTITEM		numFirstRowInSet,	//[IN] num of the first row in the set of rows to be fetched.
					DBORDINAL		cColumns,			//[IN] num of columns.
					DB_LORDINAL*	rgColumnsOrd,		//[IN] list of column ordinals.
					IRowset*		pIRowset,			//[IN] Pointer to IRowset.
					HACCESSOR		hAccessor,			//[IN] handle to accessor
					DBCOUNTITEM		cBindings,			//[IN] Number of bindings
					DBBINDING*		rgBindings,			//[IN] Binding structs
					DBLENGTH		cbRowSize,			//[IN] row size
					CTable*			pTable = NULL		//[IN] pointer to base table
					);

	//Test the obtained IOpenRowset interface.
	BOOL	testIOpenRowset(IOpenRowset* pIOpenRowset);

	//Test the obtained IGetSession interface.
	BOOL	testIGetSession(IGetSession* pIGetSession);

	//Test the obtained IColumnsInfo2 interface.
	BOOL	testIColumnsInfo2(IColumnsInfo2* pIColumnsInfo2);

	//Test the obtained ICreateRow interface.
	BOOL	testICreateRow(
		ICreateRow*	pICreateRow, 
		WCHAR*		pwszURL
		);

	//Test the obtained IAccessor and IRowset interfaces. Also get the 
	//data and compare it to that in the base table.
	BOOL	testRowset(
				IAccessor*	pIAccessor,
				IRowset*	pIRowset,
				BOOL		bCompData = TRUE
				);

	//Test the obtained IRow interface.
	BOOL	testIRow(IRow* pIRow, DBCOUNTITEM ulRowNum=0);

	//Test the obtained IGetRow interface.
	BOOL	testIGetRow(IGetRow* pIGetRow);

	//Test the obtained IGetSourceRow interface.
	BOOL	testIGetSourceRow(IGetSourceRow* pIGetSourceRow);

	//Test the obtained ICommand interface.
	BOOL	testICommand(ICommand* pICommand);

	//Test the obtained ICommandText interface.
	BOOL	testICommandText(ICommandText* pICT);

	//Test Asynch behaviour.
	BOOL	testAsynchComplete(IDBAsynchStatus* pIDBAS);

	//Test all optional interfaces of given object type with Bind.
	BOOL	testAllIntfBind(EINTERFACE eIntf);

	//Test all optional interfaces of given object type with CreateRow.
	BOOL	testAllIntfCR(EINTERFACE eIntf);

private:

//METHODS...

	//Get value of DBPROP_GENERATEURL property.
	BOOL	GetGUProperty();

};


////////////////////////////////////////////////////////////////////////
//TCBindAndCreate Implementation
//
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
//  TCBindAndCreate::TCBindAndCreate
//
TCBindAndCreate::TCBindAndCreate(WCHAR * pwszTestCaseName)	: CSessionObject(pwszTestCaseName) 
{
	memset(m_rgURLs, 0, sizeof(m_rgURLs));
	memset(m_rgNewURLs, 0, sizeof(m_rgNewURLs));

	m_pwszCmdURL			= NULL;
	m_cRowsetURLs			= 0;
	m_rgRowsetURLs			= NULL;
	m_pwszNewFolder			= NULL;
	m_lGenerateURL			= 0;

	m_pIBindResource		= NULL;
	m_pICreateRow			= NULL;
	m_pIDBBinderProperties	= NULL;
}

//----------------------------------------------------------------------
//  TCBindAndCreate::~TCBindAndCreate
//
TCBindAndCreate::~TCBindAndCreate()
{
	ReleaseAll();
}


//----------------------------------------------------------------------
// TCBindAndCreate::InitTC
//
BOOL TCBindAndCreate::InitTC()
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	CTable*		pTable = NULL;

	QTESTC(InitializeURLs())

	switch (m_eTestCase)
	{
	case TC_RBINDER:

		TESTC(GetRootBinder())
		TESTC(GetGUProperty())
		TESTC(GetSession())
		break;

	case TC_PBINDER:

		TESTC(GetProvBinder())
		TESTC(GetGUProperty())
		TESTC(GetSession())
		break;

	case TC_SESSION:

		TEST2C_(hr = GetBindOnSession(), S_OK, S_FALSE)
		//Skipping test case because direct binding is not supported on SESSION
		TESTC_PROVIDER(hr==S_OK)
		TESTC(GetGUProperty())
		break;

	case TC_ROW:

		TEST2C_(hr = GetBindOnRowObj(), S_OK, S_FALSE)
		//Skipping test case because direct binding is not supported on ROW
		TESTC_PROVIDER(hr==S_OK)
		TESTC(GetGUProperty())
		break;

	default:
		ASSERT(!L"Unhandled Type of Test Case.");

	}

	//The global pointers get addrefed here. 
	//So release in TermTC().
	CHECK(GetDataSourceObject(IID_IDBInitialize, (IUnknown**)
		&g_pIDBInitialize), S_OK);
	CHECK(GetSessionObject(IID_IOpenRowset, (IUnknown**)
		&g_pIOpenRowset), S_OK);

	//If a root table exists, use it. Otherwise call CreateTable.
	if(GetRootTable())
		SetTable(GetRootTable(), DELETETABLE_NO);
	else
	{
		TESTC(CreateTable(&pTable, 5))
		SetTable(pTable, DELETETABLE_NO);
	}

CLEANUP:
	TRETURN
} //InitTC

//----------------------------------------------------------------------
// TCBindAndCreate::TermTC
//
BOOL TCBindAndCreate::TermTC()
{
	//If we had used the root table, do not delete it here.
	if(m_pTable && (!GetRootTable()))
	{
		m_pTable->DropTable();
		SAFE_DELETE(m_pTable);
	}
	ReleaseAll();

	SAFE_RELEASE(g_pIOpenRowset);
	SAFE_RELEASE(g_pIDBInitialize);

	ReleaseDBSession();
	ReleaseDataSourceObject();

	return TRUE;
} //TermTC

//----------------------------------------------------------------------
// TCBindAndCreate::ReleaseAll
//
BOOL TCBindAndCreate::ReleaseAll()
{
	for(ULONG i=0;i<NUMELEM(m_rgNewURLs); i++)
		SAFE_FREE(m_rgNewURLs[i]);

	for(ULONG j=0;j<m_cRowsetURLs; j++)
		SAFE_FREE(m_rgRowsetURLs[j]);

	SAFE_FREE(m_rgRowsetURLs);
	m_cRowsetURLs = 0;

	SAFE_FREE(m_pwszNewFolder);

	SAFE_RELEASE(m_pIBindResource);
	SAFE_RELEASE(m_pICreateRow);
	SAFE_RELEASE(m_pIDBBinderProperties);
	return TRUE;
} //ReleaseAll

//----------------------------------------------------------------------
// TCBindAndCreate::BindResource
//
HRESULT TCBindAndCreate::BindResource(
	LPCOLESTR			pwszURL,
	REFGUID				rguid,
	REFIID				riid,
	DBBINDURLSTATUS*	pdwBindStatus,
	IUnknown**			ppUnk,
	DBBINDURLFLAG		dwBindFlags,
	DBIMPLICITSESSION*	pImplSession,
	IUnknown*			pUnkOuter,
	IAuthenticate*		pAuthenticate
	)
{
	if(!m_pIBindResource)
	{
		odtLog<<L"ERROR: Pointer to IBindResource is NULL.\n";
		return E_FAIL;
	}

	HRESULT hr = m_pIBindResource->Bind(pUnkOuter, pwszURL, 
		dwBindFlags, rguid, riid, pAuthenticate, pImplSession, 
		pdwBindStatus, ppUnk);

	if(DB_S_ASYNCHRONOUS == hr)
		COMPARE(DefaultObjectTesting(*ppUnk, UNKNOWN_INTERFACE), TRUE);

	if(S_OK == hr || DB_S_ERRORSOCCURRED == hr)
	{
		//Check returned interface pointer.
		if(!COMPARE(*ppUnk != NULL, TRUE))
			return E_FAIL;

		//Check bind status.
		if(pdwBindStatus && (S_OK == hr))
			COMPARE(*pdwBindStatus, DBBINDURLSTATUS_S_OK);

		//Do some default object testing on obtained object.
		if(!(dwBindFlags & DBBINDURLFLAG_WAITFORINIT))
		{
			if(DBGUID_DSO == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, DATASOURCE_INTERFACE), TRUE);
			else if(DBGUID_SESSION == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, SESSION_INTERFACE), TRUE);
			else if(DBGUID_ROW == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, ROW_INTERFACE), TRUE);
			else if(DBGUID_ROWSET == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, ROWSET_INTERFACE), TRUE);
			else if(DBGUID_STREAM == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, STREAM_INTERFACE), TRUE);
			else if(DBGUID_COMMAND == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, COMMAND_INTERFACE), TRUE);
		}
	}

	if((DB_S_ERRORSOCCURRED == hr) && pdwBindStatus)
	{
		if(*pdwBindStatus == DBBINDURLSTATUS_S_DENYNOTSUPPORTED
			|| *pdwBindStatus == DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED)
		{
			COMPARE((dwBindFlags & (DBBINDURLFLAG_SHARE_DENY_READ |
			DBBINDURLFLAG_SHARE_DENY_WRITE | 
			DBBINDURLFLAG_SHARE_EXCLUSIVE |
			DBBINDURLFLAG_SHARE_DENY_NONE)) > 0, TRUE);

			if(*pdwBindStatus == DBBINDURLSTATUS_S_DENYNOTSUPPORTED)
				odtLog<<L"INFO: Got DBBINDURLSTATUS_S_DENYNOTSUPPORTED.\n";
			else
				odtLog<<L"INFO: Got DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED.\n";
		}
		else if(*pdwBindStatus == DBBINDURLSTATUS_S_REDIRECTED)
		{
			odtLog<<L"WARNING: Got DBBINDURLSTATUS_S_REDIRECTED.\n";
			CHECKW(hr, S_OK);
		}
		else
		{
			odtLog<<L"ERROR: Got a status value other than the 3 valid ones.\n";
			COMPARE(TRUE, FALSE);
		}
	}

	if(FAILED(hr) && ppUnk)
		COMPARE(*ppUnk, NULL);

	return hr;
} //BindResource

//----------------------------------------------------------------------
// TCBindAndCreate::CreateRow
//
HRESULT	TCBindAndCreate::CreateRow(
	LPCOLESTR			pwszURL,
	REFGUID				rguid,
	REFIID				riid,
	DBBINDURLSTATUS*	pdwBindStatus,
	LPOLESTR*			ppwszNewURL,
	IUnknown**			ppUnk,
	DBBINDURLFLAG		dwBindFlags,
	DBIMPLICITSESSION*	pImplSession,
	IUnknown*			pUnkOuter,
	IAuthenticate*		pAuthenticate
	)
{
	WCHAR* pwszTempURL = NULL;

	if(!m_pICreateRow)
	{
		odtLog<<L"ERROR: Pointer to ICreateRow is NULL.\n";
		return E_FAIL;
	}

	//DO NOT call this wrapper when testing for the pwszURL=NULL
	//case.
	if(!pwszURL)
	{
		odtLog<<L"ERROR: Pointer to URL is NULL.\n";
		return E_FAIL;
	}

	if(m_pwszNewFolder && m_lGenerateURL && (m_lGenerateURL & DBPROPVAL_GU_SUFFIX))
		pwszTempURL = m_pwszNewFolder;
	else
		pwszTempURL = (WCHAR*) pwszURL;

	HRESULT hr = m_pICreateRow->CreateRow(pUnkOuter, pwszTempURL, 
		dwBindFlags, rguid, riid, pAuthenticate, pImplSession, 
		pdwBindStatus, ppwszNewURL, ppUnk);

	if(DB_S_ASYNCHRONOUS == hr)
		COMPARE(DefaultObjectTesting(*ppUnk, UNKNOWN_INTERFACE), TRUE);

	if(S_OK == hr || DB_S_ERRORSOCCURRED == hr)
	{
		//Check returned interface pointer.
		if(!COMPARE(*ppUnk != NULL, TRUE))
			return E_FAIL;

		//Check bind status.
		if(pdwBindStatus && (S_OK == hr))
			COMPARE(*pdwBindStatus, DBBINDURLSTATUS_S_OK);

		if(ppwszNewURL)
		{
			if(*ppwszNewURL==NULL)
			{
				odtLog<<L"ERROR: *ppwszNewURL returned was NULL.\n";
				return E_FAIL;
			}

			if(m_lGenerateURL && (m_lGenerateURL & DBPROPVAL_GU_SUFFIX))
				COMPARE(wcsstr(*ppwszNewURL, pwszTempURL)!=NULL, TRUE);
			else
				COMPARE(wcscmp(*ppwszNewURL, pwszTempURL), 0);
		}

		//Do some default object testing on obtained object.
		if(!(dwBindFlags & DBBINDURLFLAG_WAITFORINIT))
		{
			COMPARE(DBGUID_DSO!=rguid && DBGUID_SESSION!=rguid &&
				DBGUID_COMMAND!=rguid, TRUE);

			if(DBGUID_ROW == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, ROW_INTERFACE), TRUE);
			else if(DBGUID_ROWSET == rguid)
			{
				COMPARE(dwBindFlags & DBBINDURLFLAG_COLLECTION, DBBINDURLFLAG_COLLECTION);
				COMPARE(DefaultObjectTesting(*ppUnk, ROWSET_INTERFACE), TRUE);
			}
			else if(DBGUID_STREAM == rguid)
				COMPARE(DefaultObjectTesting(*ppUnk, STREAM_INTERFACE), TRUE);
		}
	}

	if((DB_S_ERRORSOCCURRED == hr) && pdwBindStatus)
	{
		if(*pdwBindStatus == DBBINDURLSTATUS_S_DENYNOTSUPPORTED
			|| *pdwBindStatus == DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED)
		{
			COMPARE((dwBindFlags & (DBBINDURLFLAG_SHARE_DENY_READ |
			DBBINDURLFLAG_SHARE_DENY_WRITE | 
			DBBINDURLFLAG_SHARE_EXCLUSIVE |
			DBBINDURLFLAG_SHARE_DENY_NONE)) > 0, TRUE);

			if(*pdwBindStatus == DBBINDURLSTATUS_S_DENYNOTSUPPORTED)
				odtLog<<L"INFO: Got DBBINDURLSTATUS_S_DENYNOTSUPPORTED.\n";
			else
				odtLog<<L"INFO: Got DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED.\n";
		}
		else if(*pdwBindStatus == DBBINDURLSTATUS_S_REDIRECTED)
		{
			odtLog<<L"WARNING: Got DBBINDURLSTATUS_S_REDIRECTED.\n";
			CHECKW(hr, S_OK);
		}
		else
		{
			odtLog<<L"ERROR: Got a status value other than the 3 valid ones.\n";
			COMPARE(TRUE, FALSE);
		}
	}

	if(FAILED(hr))
	{
		if(ppwszNewURL)
			COMPARE(*ppwszNewURL, NULL);
		if(ppUnk)
			COMPARE(*ppUnk, NULL);
	}

	if(DB_E_RESOURCEEXISTS == hr)
	{
		if(dwBindFlags & DBBINDURLFLAG_OVERWRITE)
			odtLog<<L"INFO: The provider does not support OVERWRITE behaviour on ICreateRow.\n";
		if(dwBindFlags & DBBINDURLFLAG_OPENIFEXISTS)
			odtLog<<L"INFO: DB_E_RESOURCEEXISTS was returned when OPENIFEXISTS flag was used. This will only happen if the URL used was an existing collection.\n";
	}

	return hr;
} //CreateRow

//----------------------------------------------------------------------
// TCBindAndCreate::GetRootBinder
//
BOOL TCBindAndCreate::GetRootBinder()
{
	TBEGIN
	IBindResource*	pIBR = NULL;

	pIBR = GetModInfo()->GetRootBinder();

	TESTC(VerifyInterface(pIBR, IID_IBindResource,
		BINDER_INTERFACE,(IUnknown**)&m_pIBindResource))

	TESTC(VerifyInterface(m_pIBindResource, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_pIDBBinderProperties))
	TESTC(VerifyInterface(m_pIBindResource, IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&m_pICreateRow))
	TESTC(SetInitProps(m_pIDBBinderProperties))
	
CLEANUP:
	TRETURN
} //GetRootBinder

//----------------------------------------------------------------------
// TCBindAndCreate::GetProvBinder
//
BOOL TCBindAndCreate::GetProvBinder()
{
	TBEGIN
	CLSID				clsid;
	CHAR*				pszClsid=NULL;
	WCHAR*				pwszClsid=NULL;
	CHAR				szKeyName[100];
	LONG				lResult;
	HKEY				hKey = NULL;
	IRegisterProvider*	pIReg = NULL;
	IBindResource*		pIBR = NULL;

	pIBR = GetModInfo()->GetRootBinder();

	TESTC(VerifyInterface(pIBR, IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&pIReg))
	TESTC_(pIReg->GetURLMapping(m_rgURLs[ROWSET], 0, &clsid), S_OK)

	//Verify that this clsid has an "OLE DB Binder" key in the
	//registry.

	CHECK(StringFromCLSID(clsid, &pwszClsid), S_OK);
	TESTC(pwszClsid != NULL)
	pszClsid = ConvertToMBCS(pwszClsid);
	TESTC(pszClsid != NULL)

	strcpy(szKeyName, "CLSID\\");
	strcat(szKeyName, pszClsid);
	strcat(szKeyName, "\\");
	strcat(szKeyName, "OLE DB Binder");

	lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
						   szKeyName,
						   0,
						   KEY_READ,
						   &hKey);
	COMPARE(lResult, ERROR_SUCCESS );

	TESTC_(CoCreateInstance(clsid, NULL, GetModInfo()->GetClassContext(), 
		IID_IBindResource, (void**)&m_pIBindResource), S_OK)

	TESTC(VerifyInterface(m_pIBindResource, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_pIDBBinderProperties))
	TESTC(VerifyInterface(m_pIBindResource, IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&m_pICreateRow))
	TESTC(SetInitProps(m_pIDBBinderProperties))
	
CLEANUP:
	if(hKey)
		RegCloseKey(hKey);
	SAFE_FREE(pszClsid);
	SAFE_FREE(pwszClsid);
	SAFE_RELEASE(pIReg);
	TRETURN
} //GetProvBinder

//----------------------------------------------------------------------
// TCBindAndCreate::GetBindOnSession
//
HRESULT TCBindAndCreate::GetBindOnSession()
{
	HRESULT					hrRet = E_FAIL;
	HRESULT					hr;
	BOOL					bDirectBind = FALSE;
	ULONG_PTR				ulPropVal = 0;
	IBindResource*			pIBR = NULL;
	IDBBinderProperties*	pIDBBindProp = NULL;
	IDBInitialize*			pIDBInit = NULL;
	IGetDataSource*			pIGetDataSource = NULL;
	IUnknown*				pIUnk = NULL;

	pIBR = GetModInfo()->GetRootBinder();
	TESTC(pIBR != NULL)

	TESTC(VerifyInterface(pIBR, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBindProp))

	TESTC(SetInitProps(pIDBBindProp));

	//If the provider can be direct bound thru the root binder,
	//then it should also have the IBindResource interface on 
	//it's Session.
	TESTC_(hr=pIBR->Bind(NULL, m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_SESSION, IID_IBindResource, NULL, NULL, NULL, 
		(IUnknown**)&m_pIBindResource), S_OK)

	TESTC(m_pIBindResource != NULL)

	//Get a DataSource to verify DBPROP_OLEOBJECTS property.
	TESTC_(hr=pIBR->Bind(NULL, m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBInitialize, NULL, NULL, NULL, 
		(IUnknown**)&pIDBInit), S_OK)
	TESTC(pIDBInit!=NULL)

	//Verify this property.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		pIDBInit, &ulPropVal))
		COMPAREW(ulPropVal & DBPROPVAL_OO_DIRECTBIND, DBPROPVAL_OO_DIRECTBIND);

	VerifyInterface(m_pIBindResource, IID_ICreateRow,
		SESSION_INTERFACE,(IUnknown**)&m_pICreateRow);

	SetDBSession(m_pIBindResource);

	TESTC(VerifyInterface(m_pIBindResource,IID_IGetDataSource,
		SESSION_INTERFACE,(IUnknown**)&pIGetDataSource))

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown,
		(IUnknown**)&pIUnk), S_OK)

	SetDataSourceObject(pIUnk, TRUE);

	hrRet = S_OK;
	
CLEANUP:
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIDBInit);
	SAFE_RELEASE(pIDBBindProp);
	SAFE_RELEASE(pIGetDataSource);
	return hrRet;
} //GetBindOnSession

//----------------------------------------------------------------------
// TCBindAndCreate::GetBindOnRowObj
//
HRESULT TCBindAndCreate::GetBindOnRowObj()
{
	HRESULT					hrRet = E_FAIL;
	HRESULT					hr;
	BOOL					bDirectBind = FALSE;
	BOOL					bRowObj = FALSE;
	ULONG_PTR				ulPropVal = 0;
	IBindResource*			pIBR = NULL;
	IDBBinderProperties*	pIDBBindProp = NULL;
	IDBInitialize*			pIDBInit = NULL;
	IGetDataSource*			pIGetDataSource = NULL;
	IGetSession*			pIGS = NULL;
	IUnknown*				pIUnk = NULL;

	pIBR = GetModInfo()->GetRootBinder();
	TESTC(pIBR != NULL)

	TESTC(VerifyInterface(pIBR, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBindProp))

	TESTC(SetInitProps(pIDBBindProp));

	TESTC_(hr=pIBR->Bind(NULL, m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBInitialize, NULL, NULL, NULL, 
		(IUnknown**)&pIDBInit), S_OK)
	TESTC(pIDBInit!=NULL)

	//Will verify this property against actual behavior.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		pIDBInit, &ulPropVal) && 
		((ulPropVal & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND))
		bDirectBind = TRUE;

	//Will verify this property against actual behaviour.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		pIDBInit, &ulPropVal) && 
		((ulPropVal & DBPROPVAL_OO_ROWOBJECT) == DBPROPVAL_OO_ROWOBJECT))
		bRowObj = TRUE;

	TEST3C_(hr=pIBR->Bind(NULL, m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_ICreateRow, NULL, NULL, NULL, 
		(IUnknown**)&m_pICreateRow), S_OK, E_NOINTERFACE, DB_E_NOTSUPPORTED)

	if(E_NOINTERFACE == hr)
	{
		odtLog<<L"INFO: ICreateRow is not supported on ROW object.\n";
		hrRet = S_FALSE;
		goto CLEANUP;
	}
	else if(DB_E_NOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: Binding to Row objects is not supported.\n";
		COMPAREW(bRowObj, FALSE);
		hrRet = S_FALSE;
		goto CLEANUP;
	}
	else
	{
		COMPAREW(bDirectBind, TRUE);
		COMPAREW(bRowObj, TRUE);
	}

	TESTC(m_pICreateRow != NULL)

	//Try to get IBindResource on the Row obj.
	if(!VerifyInterface(m_pICreateRow,IID_IBindResource,
		ROW_INTERFACE,(IUnknown**)&m_pIBindResource))
		odtLog<<L"WARNING: IBindResource is not supported on the Row object.\n";

	TESTC(VerifyInterface(m_pICreateRow, IID_IGetSession,
		ROW_INTERFACE,(IUnknown**)&pIGS))

	TEST2C_(hr=pIGS->GetSession(IID_IUnknown, (IUnknown**)
		&pIUnk), S_OK, DB_E_NOSOURCEOBJECT)
	if(DB_E_NOSOURCEOBJECT == hr)
	{
		odtLog<<L"INFO: Cannot get to session from Row object.\n";
		hrRet = S_FALSE;
		goto CLEANUP;
	}

	SetDBSession(pIUnk);

	TESTC(VerifyInterface(pIUnk,IID_IGetDataSource,
		SESSION_INTERFACE,(IUnknown**)&pIGetDataSource))

	SAFE_RELEASE(pIUnk);

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown,
		(IUnknown**)&pIUnk), S_OK)

	SetDataSourceObject(pIUnk, TRUE);

	hrRet = S_OK;
	
CLEANUP:
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIDBInit);
	SAFE_RELEASE(pIDBBindProp);
	SAFE_RELEASE(pIGS);
	SAFE_RELEASE(pIGetDataSource);
	return hrRet;
} //GetBindOnRowObj

//----------------------------------------------------------------------
// TCBindAndCreate::SetInitProps
//
BOOL TCBindAndCreate::SetInitProps(IDBBinderProperties* pIDBBindProp)
{
	BOOL		bRet = FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;

	if(!pIDBBindProp)
		return FALSE;

	if(COMPARE(GetInitProps(&cPropSets, &rgPropSets), TRUE))
	{
		if(CHECK(pIDBBindProp->SetProperties(cPropSets, rgPropSets), 
			S_OK))
			bRet = TRUE;
	}

	FreeProperties(&cPropSets, &rgPropSets);
	return bRet;
} //SetInitProps


//----------------------------------------------------------------------
// TCBindAndCreate::InitializeURLs
//
BOOL TCBindAndCreate::InitializeURLs()
{
	m_pwszCmdURL		= GetModInfo()->GetParseObject()->GetURL(COMMAND_INTERFACE);
	m_rgURLs[STREAM]	= GetModInfo()->GetParseObject()->GetURL(STREAM_INTERFACE);	
	m_rgURLs[ROWSET]	= GetModInfo()->GetParseObject()->GetURL(ROWSET_INTERFACE);	
	m_rgURLs[ROW]		= GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE);
	m_rgURLs[SESSION]	= GetModInfo()->GetParseObject()->GetURL(SESSION_INTERFACE);
	m_rgURLs[DSO]		= GetModInfo()->GetParseObject()->GetURL(DATASOURCE_INTERFACE);	

	for(ULONG ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		if((m_rgURLs[ulIndex] == NULL) || (wcslen(m_rgURLs[ulIndex]) < 2))
		{
			odtLog<<L"ERROR: There was an error obtaining URLs from the INI file. The INI file has to have a [URL] section for this test to run.\n";
			return FALSE;
		}
	}

	if(m_pwszCmdURL && wcslen(m_pwszCmdURL) < 2)
		m_pwszCmdURL = NULL;

	return TRUE;
} //InitializeURLs

//----------------------------------------------------------------------
// TCBindAndCreate::InitializeRowsetURLs
//
BOOL TCBindAndCreate::InitializeRowsetURLs()
{
	TBEGIN
	DBCOUNTITEM	ulIndex = 0;
	BOOL		bRowObj = FALSE;
	DBID		dbidTable;
	DBCOUNTITEM	cRowsObtained = 0;
	ULONG_PTR	ulPropVal = 0;
	HROW		hRow = DB_NULL_HROW;
	IGetRow*	pIGetRow = NULL;
	CRowset		rowsetA;

	if(m_rgRowsetURLs)
		return TRUE;

	if(!pIOpenRowset() || !m_pTable || !m_rgURLs[ROWSET])
		return FALSE;

	//If ROW objects are not supported, skip this func.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		pIDBInitialize(), &ulPropVal) && 
		((ulPropVal & DBPROPVAL_OO_ROWOBJECT) == DBPROPVAL_OO_ROWOBJECT))
		bRowObj = TRUE;

	dbidTable = m_pTable->GetTableID();

	TESTC_(rowsetA.CreateRowset(USE_OPENROWSET, IID_IRowset, 
		m_pTable), S_OK);

	TESTC(rowsetA.pIRowset() != NULL)

	if(VerifyInterface(rowsetA.pIRowset(), IID_IGetRow,
		ROWSET_INTERFACE,(IUnknown**)&pIGetRow))
	{
		COMPAREW(bRowObj, TRUE);
	}
	else
	{
		odtLog<<L"INFO: IGetRow is not supported.\n";
		COMPAREW(bRowObj, FALSE);
		m_rgRowsetURLs = NULL;
		return TRUE;
	}

	while(rowsetA.GetNextRows(&hRow) == S_OK)
	{
		TESTC(hRow != DB_NULL_HROW)
		cRowsObtained++;
		SAFE_REALLOC(m_rgRowsetURLs, WCHAR*, cRowsObtained);
		TESTC_(pIGetRow->GetURLFromHROW(hRow, &(m_rgRowsetURLs[ulIndex])), S_OK)
		COMPARE((m_rgRowsetURLs[ulIndex] != NULL) && (wcslen(m_rgRowsetURLs[ulIndex]) > 1), TRUE);
		rowsetA.ReleaseRows(hRow);
		ulIndex++;
	}

	m_cRowsetURLs = cRowsObtained;

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	TRETURN
} //InitializeRowsetURLs

//----------------------------------------------------------------------
// TCBindAndCreate::InitializeNewURLs
//
BOOL TCBindAndCreate::InitializeNewURLs()
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszFolder = NULL;
	IRow*				pIRow = NULL;
	WCHAR				pwszNum[3];

	//The extensions to the URL (for generating new non-existing
	//URLs) is hardcoded here. It is being assumed that the 
	//provider accepts extensions of the form "/1/1" to its URLs
	//to still form a valid URL. This assumption may not always
	//be true. But this is the best we can do for now, since
	//we have a way of knowing how best to extend a URL for a
	//particular provider.
	WCHAR				wszNewFolder[] = L"/1";

	if(m_rgNewURLs[0])
		return TRUE;

	C_ASSERT(NEWURLS>=1 && NEWURLS<100);

	if(NEWURLS < 1 || NEWURLS >99)
		return FALSE;

	//Display value of DBPROP_GENERATEURL
	if((!m_lGenerateURL) || (m_lGenerateURL & DBPROPVAL_GU_NOTSUPPORTED))
		odtLog<<L"INFO: Generating new URLs is NOT supported.\n";
	else
		odtLog<<L"INFO: Generating new URLs IS supported.\n";

	//Create a new folder.

	SAFE_ALLOC(pwszFolder, WCHAR, wcslen(m_rgURLs[ROWSET])
		+wcslen(wszNewFolder)+sizeof(WCHAR))
	wcscpy(pwszFolder, m_rgURLs[ROWSET]);
	if((!m_lGenerateURL) || (m_lGenerateURL & DBPROPVAL_GU_NOTSUPPORTED))
		wcscat(pwszFolder, wszNewFolder);

	SAFE_FREE(m_pwszNewFolder);

	TESTC_(hr=CreateRow(pwszFolder, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &m_pwszNewFolder, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(m_pwszNewFolder != NULL)

	for(ulIndex=0; ulIndex<NEWURLS; ulIndex++)
	{
		SAFE_ALLOC(m_rgNewURLs[ulIndex], WCHAR, wcslen(m_pwszNewFolder)
			+wcslen(L"/11")+1)

		_ultow(ulIndex+1, pwszNum, 10);
		wcscpy(m_rgNewURLs[ulIndex], m_pwszNewFolder);
		wcscat(m_rgNewURLs[ulIndex], L"/");
		wcscat(m_rgNewURLs[ulIndex], pwszNum);
	}

	SAFE_RELEASE(pIRow);

	//Make sure the new URLs work.
	TESTC_(hr=CreateRow(m_rgNewURLs[0], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, NULL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		S_OK)

	g_cNewURL++;

CLEANUP:
	SAFE_FREE(pwszFolder);
	SAFE_RELEASE(pIRow);
	TRETURN
} //InitializeNewURLs

//----------------------------------------------------------------------
// TCBindAndCreate::CleanupNewURLs
//
BOOL TCBindAndCreate::CleanupNewURLs()
{
	TBEGIN
	ULONG				ulIndex;
	HRESULT				hr;
	BOOL				bWrongStatus = FALSE;
	DBSTATUS			rgdwStatus[NEWURLS];
	IBindResource*		pIBR = NULL;
	IScopedOperations*	pISO = NULL;

	if(!m_pwszNewFolder || !m_rgNewURLs || !m_rgNewURLs[0])
		return TRUE;

	pIBR = GetModInfo()->GetRootBinder();
	TESTC(pIBR != NULL)

	TEST3C_(hr=pIBR->Bind(NULL, m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pISO), S_OK, E_NOINTERFACE, DB_E_NOTSUPPORTED)

	if(hr != S_OK)
		goto CLEANUP;

	TEST3C_(pISO->Delete(NEWURLS, (LPCOLESTR*)m_rgNewURLs, 0, rgdwStatus), S_OK, 
		DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED)

	for(ulIndex=0; ulIndex<NEWURLS; ulIndex++)
	{
		if(rgdwStatus[ulIndex] != DBSTATUS_S_OK &&
			rgdwStatus[ulIndex] != DBSTATUS_E_DOESNOTEXIST)
		{
			bWrongStatus = TRUE;
			break;
		}
	}

	if(bWrongStatus)
		odtLog<<L"INFO: At least one status returned from IScopOps::Delete was wrong : "<<GetStatusName(rgdwStatus[ulIndex])<<".\n";

	CHECKW(pISO->Delete(1, (LPCOLESTR*)&m_pwszNewFolder, 0, rgdwStatus), S_OK);

CLEANUP:
	SAFE_RELEASE(pISO);
	TRETURN
} //CleanupNewURLs

//----------------------------------------------------------------------
// TCBindAndCreate::GetSession
//
BOOL TCBindAndCreate::GetSession()
{
	TBEGIN
	IGetDataSource*	pIGetDataSource = NULL;
	IUnknown*		pIUnk = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_SESSION, 
		IID_IUnknown, NULL, (IUnknown**)
		&pIUnk), S_OK)

	SetDBSession(pIUnk);

	TESTC(VerifyInterface(pIUnk,IID_IGetDataSource,
		SESSION_INTERFACE,(IUnknown**)&pIGetDataSource))

	SAFE_RELEASE(pIUnk);

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown,
		(IUnknown**)&pIUnk), S_OK)

	SetDataSourceObject(pIUnk, TRUE);

CLEANUP:
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIGetDataSource);
	TRETURN
} //GetSession


//-----------------------------------------------------------------------
// TCBindAndCreate::GetDataAndCompare
//
BOOL TCBindAndCreate::GetDataAndCompare(
	DBROWOFFSET		cSkipRows,
	DBROWCOUNT		cGetRows,
	DBCOUNTITEM		numFirstRowInSet,
	DBORDINAL		cColumns,
	DB_LORDINAL*	rgColumnsOrd,
	IRowset*		pIRowset,
	HACCESSOR		hAccessor,
	DBCOUNTITEM		cBindings,
	DBBINDING*		rgBindings,
	DBLENGTH		cbRowSize,
	CTable*			pTable
	)
{
	BOOL		bRet = TRUE;
	DBCOUNTITEM	ulIndex = 0;
	DBCOUNTITEM	rowNum = 0;	  //Row number of row to be compared.
	DBCOUNTITEM	cRowsObtained = 0;
	HRESULT		hr = S_OK;
	BYTE*		pData = NULL;
	HROW*		rghRows = NULL;

	TESTC(pIRowset != NULL)

	if(pTable == NULL)
		pTable = m_pTable;

	TESTC(pTable != NULL)

	//Use IRowset to retrieve data
	TESTC_(pIRowset->GetNextRows(NULL, cSkipRows, cGetRows, 
		&cRowsObtained, (HROW **)&rghRows), S_OK)

	//Allocate a new data buffer. 
	SAFE_ALLOC(pData, BYTE, cbRowSize)

	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
	{
		if(!GCHECK(hr = pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData),S_OK))
			continue;

		//adjust the rowNum according to the fetch direction.
		if(cGetRows >= 0)
			rowNum = numFirstRowInSet+ulIndex;
		else
			rowNum = numFirstRowInSet-ulIndex;

		//Verify data value, length and status are what is expected.
		if(!CompareData(cColumns, rgColumnsOrd, rowNum,
			pData, cBindings, rgBindings, pTable,
			m_pIMalloc, PRIMARY, COMPARE_ONLY))
		{
			bRet = FALSE;
		}

		GCHECK(ReleaseInputBindingsMemory(cBindings, rgBindings,
			pData), S_OK);
	}

CLEANUP:
	if(rghRows)
		GCHECK(pIRowset->ReleaseRows(cRowsObtained, rghRows,NULL,NULL,NULL), S_OK);
	SAFE_FREE(pData);
	SAFE_FREE(rghRows);
	return bRet;
} //GetDataAndCompare

//----------------------------------------------------------------------
// TCBindAndCreate::testIOpenRowset
//
BOOL TCBindAndCreate::testIOpenRowset(IOpenRowset* pIOpenRowset)
{
	TBEGIN
	DBID        dbidTable;
	IRowset*	pIRowset = NULL;
	IAccessor*	pIAccessor = NULL;

	TESTC(pIOpenRowset != NULL)

	TESTC(DefTestInterface(pIOpenRowset))

	dbidTable = m_pTable->GetTableID();

	TESTC_(pIOpenRowset->OpenRowset(NULL, &dbidTable, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK)

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} //testIOpenRowset

//----------------------------------------------------------------------
// TCBindAndCreate::testIGetSession
//
BOOL TCBindAndCreate::testIGetSession(IGetSession* pIGetSession)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	IGetDataSource*		pGetDS = NULL;

	TESTC(pIGetSession != NULL)

	TESTC_(pIGetSession->GetSession(IID_IGetDataSource, NULL), E_INVALIDARG)

	TEST2C_(hr = pIGetSession->GetSession(IID_IGetDataSource, 
		(IUnknown**)&pGetDS), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC((SUCCEEDED(hr) && pGetDS!=NULL) || (FAILED(hr) && pGetDS==NULL))

CLEANUP:
	SAFE_RELEASE(pGetDS);
	TRETURN
} //testIGetSession

//----------------------------------------------------------------------
// TCBindAndCreate::testIColumnsInfo2
//
BOOL TCBindAndCreate::testIColumnsInfo2(IColumnsInfo2* pIColumnsInfo2)
{
	TBEGIN
	DBORDINAL			cColumns = 0;
	DBID*				rgColumnIDs = NULL;
	DBCOLUMNINFO*		rgColumnInfo = NULL;
	OLECHAR*			pStringsBuffer = NULL;

	TESTC(pIColumnsInfo2 != NULL)

	TESTC_(pIColumnsInfo2->GetColumnInfo(&cColumns, &rgColumnInfo, 
		&pStringsBuffer), S_OK)
	TESTC(cColumns>0 && pStringsBuffer && rgColumnInfo)

	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringsBuffer);

	TESTC_(pIColumnsInfo2->MapColumnIDs(1, NULL, NULL), E_INVALIDARG)

	TESTC_(pIColumnsInfo2->GetRestrictedColumnInfo(0, NULL,
		0, &cColumns, &rgColumnIDs, &rgColumnInfo, 
		&pStringsBuffer), S_OK)
	TESTC(cColumns>0 && rgColumnIDs && pStringsBuffer && 
		rgColumnInfo)

CLEANUP:
	SAFE_FREE(rgColumnIDs);
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} //testIColumnsInfo2

//----------------------------------------------------------------------
// TCBindAndCreate::testICreateRow
//
BOOL TCBindAndCreate::testICreateRow(
	ICreateRow*	pICreateRow, 
	WCHAR*		pwszURL)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	LPOLESTR	pwszNewURL = NULL;
	IRow*		pIRow = NULL;

	TESTC(pICreateRow!=NULL && pwszURL!=NULL)

	TESTC_(pICreateRow->CreateRow(NULL, NULL, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_OPENIFEXISTS, DBGUID_ROW, 
		IID_IRow, NULL, NULL, NULL, NULL, NULL), E_INVALIDARG)

	TEST2C_(hr=pICreateRow->CreateRow(NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_COLLECTION|
		DBBINDURLFLAG_OPENIFEXISTS, DBGUID_ROW, 
		IID_IRow, NULL, NULL, NULL, &pwszNewURL, (IUnknown**)&pIRow), 
		S_OK, DB_E_NOTCOLLECTION)

	if(S_OK == hr)
	{
		TESTC(pwszNewURL != NULL)
		TESTC(pIRow != NULL)
	}
	else
	{
		TESTC(!pwszNewURL)
		TESTC(!pIRow)
	}

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} //testICreateRow

//----------------------------------------------------------------------
// TCBindAndCreate::testRowset
//
BOOL TCBindAndCreate::testRowset(
	IAccessor*	pIAccessor,
	IRowset*	pIRowset,
	BOOL		bCompData
	)
{
	TBEGIN
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRows = 0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	WCHAR*			pStringsBuffer = NULL;
	DBLENGTH		cbRowSize = 0;
	DBORDINAL		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	BOOL			fMatch = FALSE;

	TESTC(pIAccessor!=NULL && pIRowset!=NULL && m_pTable!=NULL)

	TESTC(DefTestInterface(pIAccessor))
	TESTC(DefTestInterface(pIRowset))

	//Create Accessor with a binding using length, status and value.
	//All columns are bound including BLOB_LONG.
	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, &pStringsBuffer, DBTYPE_EMPTY, 0, NULL, NULL, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Get number of rows.
	cRows = m_pTable->CountRowsOnTable();

	TEST2C_(pIRowset->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED)

	//For each row in the table, fetch the row, compare the data in the 
	//row and release the row. Due to the issue of rows coming back
	//in a different order, we will check data for first and last rows only.
	if(bCompData)
	{
		for(ulIndex=0; ulIndex<cRows; ulIndex++)
		{
			if(GetDataAndCompare(0, 1, 1, cRowsetCols, 
				rgColumnsOrd, pIRowset, hAccessor, cBindings,
				rgBindings, cbRowSize))
				fMatch = TRUE;
		}
		COMPARE(fMatch, TRUE);
		fMatch = FALSE;
		TEST2C_(pIRowset->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED)
		for(ulIndex=0; ulIndex<cRows; ulIndex++)
		{
			if(GetDataAndCompare(0, 1, cRows, cRowsetCols, 
				rgColumnsOrd, pIRowset, hAccessor, cBindings,
				rgBindings, cbRowSize))
				fMatch = TRUE;
		}
		COMPARE(fMatch, TRUE);

	}

CLEANUP:
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rgColumnsOrd);
	if(hAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
} //testRowset

//----------------------------------------------------------------------
// TCBindAndCreate::testIRow
//
BOOL TCBindAndCreate::testIRow(IRow* pIRow, DBCOUNTITEM ulRowNum)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	CRowObject	CRow;
	IRowset*	pIRowset = NULL;

	TESTC(pIRow!=NULL && m_pTable!=NULL)

	TESTC_(pIRow->GetColumns(1, NULL), E_INVALIDARG)

	TESTC_(pIRow->Open(NULL, NULL, DBGUID_ROW, 0, IID_IRow, NULL), E_INVALIDARG)

	TEST2C_(hr=pIRow->GetSourceRowset(IID_IRowset, (IUnknown**)&pIRowset,
		NULL), S_OK, DB_E_NOSOURCEOBJECT)

	if(hr==S_OK)
		TESTC(pIRowset != NULL)
	else
		odtLog<<L"INFO: There is no rowset object as source for the row.\n";

	if(ulRowNum)
	{
		TESTC_(CRow.SetRowObject(pIRow), S_OK)

		//Verify row data without blob cols.
		TESTC(CRow.VerifyGetColumns(ulRowNum, m_pTable, ALL_COLS_BOUND,
			NO_BLOB_COLS, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, 0, 
			NULL))

		/*
		//Verify row data with blob cols.
		TESTC(CRow.VerifyGetColumns(ulRowNum, m_pTable, ALL_COLS_BOUND,
			BLOB_LONG, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, 0, 
			NULL))
		*/
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} //testIRow

//----------------------------------------------------------------------
// TCBindAndCreate::testIGetRow
//
BOOL TCBindAndCreate::testIGetRow(IGetRow* pIGetRow)
{
	TBEGIN
	HRESULT		hr;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		rghRows = NULL;
	WCHAR*		pwszURL = NULL;
	IRow*		pIRow = NULL;
	IRowset*	pIRowset = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;

	TESTC(pIGetRow != NULL)

	TESTC(VerifyInterface(pIGetRow,IID_IRowset,
		ROWSET_INTERFACE,(IUnknown**)&pIRowset))

	TESTC_(hr=pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, 
		&rghRows), S_OK)

	TESTC((cRowsObtained==1)&&(rghRows!=NULL)&&(*rghRows!=NULL))

	//Use the Row Handle to obtain IRow on the Row Object.
	TEST2C_(pIGetRow->GetRowFromHROW(NULL, rghRows[0], IID_IRow, 
		(IUnknown**)&pIRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS)
	TESTC(pIRow != NULL)

	TESTC_(pIGetRow->GetURLFromHROW(rghRows[0], &pwszURL), S_OK)
	TESTC(pwszURL != NULL)

	TESTC_(GetModInfo()->GetRootBinder()->Bind(NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, IID_IColumnsInfo, NULL, 
		NULL, NULL, (IUnknown**)&pIColumnsInfo), S_OK)

CLEANUP:
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	TRETURN
} //testIGetRow

//----------------------------------------------------------------------
// TCBindAndCreate::testIGetSourceRow
//
BOOL TCBindAndCreate::testIGetSourceRow(IGetSourceRow* pIGetSourceRow)
{
	TBEGIN
	HRESULT		hr;
	IRow*		pIRow = NULL;

	TESTC(pIGetSourceRow != NULL)

	TESTC_(hr=pIGetSourceRow->GetSourceRow(IID_IRow, NULL), E_INVALIDARG)

	TESTC_(hr=pIGetSourceRow->GetSourceRow(IID_IRowset, (IUnknown**)&pIRow),
		E_NOINTERFACE)
	TESTC(!pIRow)

	TEST2C_(hr=pIGetSourceRow->GetSourceRow(IID_IRow, (IUnknown**)&pIRow),
		S_OK, DB_E_NOSOURCEOBJECT)

	if(hr==S_OK)
		TESTC(pIRow != NULL)
	else
		odtLog<<L"INFO: There is no row object as context for the stream.\n";

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
} //testIGetSourceRow

//----------------------------------------------------------------------
// TCBindAndCreate::testICommand
//
BOOL TCBindAndCreate::testICommand(ICommand* pICommand)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	cRowsAffected = 0;
	IUnknown*	pSessUnk = NULL;
	IRowset*	pIRowset = NULL;

	TESTC(pICommand != NULL)
	TEST2C_(hr = pICommand->GetDBSession(IID_IUnknown, (IUnknown**)&pSessUnk), S_OK, S_FALSE)
	if((TC_RBINDER != m_eTestCase) && (TC_PBINDER != m_eTestCase))
	{
		TESTC_(hr, S_OK)
		TESTC(VerifyEqualInterface(pSessUnk, pIOpenRowset()))
	}

	TESTC_(hr = pICommand->Execute(NULL, IID_IRowset, NULL, &cRowsAffected, NULL), E_INVALIDARG)

	TEST2C_(hr = pICommand->Execute(NULL, IID_IRowset, NULL, &cRowsAffected, (IUnknown**)&pIRowset), S_OK, DB_E_NOCOMMAND)
	if(S_OK == hr)
		TESTC(pIRowset != NULL)

CLEANUP:
	SAFE_RELEASE(pSessUnk);
	SAFE_RELEASE(pIRowset);
	TRETURN
} //testICommand

//----------------------------------------------------------------------
// TCBindAndCreate::testICommandText
//
BOOL TCBindAndCreate::testICommandText(ICommandText* pICT)
{
	TBEGIN
	HRESULT			hr = E_FAIL;
	GUID			guid = DB_NULLGUID;
	WCHAR*			pwszCmd = NULL;

	TESTC(testICommand((ICommand*)pICT))

	TEST3C_(pICT->GetCommandText(&guid, &pwszCmd), S_OK, 
		DB_S_DIALECTIGNORED, DB_E_NOCOMMAND)
	if(SUCCEEDED(hr))
		TESTC(pwszCmd && (wcslen(pwszCmd) > 0))

	guid = DBGUID_DSO;
	TESTC_(pICT->SetCommandText(guid, L"NotACommand"), DB_E_DIALECTNOTSUPPORTED)

CLEANUP:
	SAFE_FREE(pwszCmd);
	TRETURN
} //testICommandText

//----------------------------------------------------------------------
// TCBindAndCreate::testAsynchComplete
//
BOOL TCBindAndCreate::testAsynchComplete(IDBAsynchStatus* pAsynchStatus)
{
	TBEGIN
	HRESULT	hr;
	DBCOUNTITEM	ulProgress=0, ulProgressMax=0;
	DBASYNCHPHASE ulAsynchPhase=0;
	WCHAR*	pwszStatusText = NULL;

	TESTC(pAsynchStatus != NULL)

	while (ulAsynchPhase != DBASYNCHPHASE_COMPLETE)
	{
		CHECKW(hr = pAsynchStatus->GetStatus(DB_NULL_HCHAPTER, 
			DBASYNCHOP_OPEN, &ulProgress, &ulProgressMax, &ulAsynchPhase, 
			&pwszStatusText), S_OK);
		if(hr != S_OK)
			odtLog<<L"WARNING: The method which was being performed asynchronously returned a code other than S_OK.\n";
	}

	TESTC(ulProgress == ulProgressMax)
	TESTC(ulAsynchPhase == DBASYNCHPHASE_COMPLETE)

	if(pwszStatusText)
	{
		odtLog<<L"INFO: Status Text - "<<pwszStatusText<<L".\n";
		SAFE_FREE(pwszStatusText);
	}

CLEANUP:
	SAFE_FREE(pwszStatusText);
	TRETURN
} //testAsynchComplete

//----------------------------------------------------------------------
// TCBindAndCreate::testAllIntfBind
//
BOOL TCBindAndCreate::testAllIntfBind(EINTERFACE eIntf)
{
	TBEGIN
	HRESULT			hr;
	ULONG			ulIndex = 0;
	ULONG			cInterfaces = 0;
	INTERFACEMAP*	rgInterfaces = NULL;
	IUnknown*		pIUnk = NULL;
	IUnknown*		pMandInterface = NULL;

	TESTC(GetInterfaceArray(eIntf, &cInterfaces, &rgInterfaces))

	switch (eIntf)
	{
	case DATASOURCE_INTERFACE:

		TESTC_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a DSO.\n";
			}
		}

		TESTC_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case SESSION_INTERFACE:

		TESTC_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a SESSION.\n";
			}
		}

		TESTC_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case ROW_INTERFACE:

		TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a ROW.\n";
			}
		}

		TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case ROWSET_INTERFACE:

		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a ROWSET.\n";
			}
		}

		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case STREAM_INTERFACE:

		TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a STREAM.\n";
			}
		}

		TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case COMMAND_INTERFACE:

		TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
			IID_IUnknown, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
				*rgInterfaces[ulIndex].pIID, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a COMMAND.\n";
			}
		}

		TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
			IID_NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	default:
		break;
	}


CLEANUP:
	SAFE_RELEASE(pMandInterface);
	SAFE_RELEASE(pIUnk);
	TRETURN
} //testAllIntfBind

//----------------------------------------------------------------------
// TCBindAndCreate::testAllIntfCR
//
BOOL TCBindAndCreate::testAllIntfCR(EINTERFACE eIntf)
{
	TBEGIN
	HRESULT			hr;
	ULONG			ulIndex = 0;
	ULONG			cInterfaces = 0;
	INTERFACEMAP*	rgInterfaces = NULL;
	IUnknown*		pIUnk = NULL;
	IUnknown*		pMandInterface = NULL;

	TESTC(GetInterfaceArray(eIntf, &cInterfaces, &rgInterfaces))

	switch (eIntf)
	{
	case ROW_INTERFACE:

		TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
			IID_IUnknown, NULL, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
				*rgInterfaces[ulIndex].pIID, NULL, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a ROW.\n";
			}
		}

		TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
			IID_NULL, NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	case ROWSET_INTERFACE:

		TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IUnknown, NULL, NULL, (IUnknown**)
			&pMandInterface, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
			DBBINDURLFLAG_COLLECTION), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
				*rgInterfaces[ulIndex].pIID, NULL, NULL, (IUnknown**)
				&pIUnk, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
				DBBINDURLFLAG_COLLECTION), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a ROWSET.\n";
			}
		}

		TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_NULL, NULL, NULL, (IUnknown**)&pIUnk, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
			DBBINDURLFLAG_COLLECTION), E_NOINTERFACE)
		break;

	case STREAM_INTERFACE:

		TESTC_(hr = CreateRow(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_IUnknown, NULL, NULL, (IUnknown**)
			&pMandInterface), S_OK)

		for(ulIndex=0; ulIndex<cInterfaces; ulIndex++)
		{
			TEST2C_(hr = CreateRow(m_rgURLs[STREAM], DBGUID_STREAM, 
				*rgInterfaces[ulIndex].pIID, NULL, NULL, (IUnknown**)
				&pIUnk), S_OK, E_NOINTERFACE)
			SAFE_RELEASE(pIUnk);
			if(hr == E_NOINTERFACE)
			{
				TESTC_(pMandInterface->QueryInterface(*rgInterfaces[ulIndex].pIID, (void**)&pIUnk), E_NOINTERFACE)
				odtLog<<L"INFO: "<<GetInterfaceName(*rgInterfaces[ulIndex].pIID)<<L" is NOT supported on a STREAM.\n";
			}
		}

		TESTC_(hr = CreateRow(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_NULL, NULL, NULL, (IUnknown**)&pIUnk), E_NOINTERFACE)
		break;

	default:
		break;
	}


CLEANUP:
	SAFE_RELEASE(pMandInterface);
	SAFE_RELEASE(pIUnk);
	TRETURN
} //testAllIntfCR

//----------------------------------------------------------------------
// TCBindAndCreate::GetGUProperty
//
BOOL TCBindAndCreate::GetGUProperty()
{
	TBEGIN
	ULONG_PTR			ulVal = 0;
	DBBINDURLSTATUS		dwBindStatus = 0;
	IDBProperties*		pIDBProperties = NULL;

	TESTC_(GetModInfo()->GetRootBinder()->Bind(NULL, m_rgURLs[ROW], DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIDBProperties), S_OK)

	if(GetProperty(DBPROP_GENERATEURL, DBPROPSET_DATASOURCEINFO, 
		(IUnknown*)pIDBProperties, &ulVal))
		m_lGenerateURL = ulVal;
	else
		m_lGenerateURL = 0;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	TRETURN
} //GetGUProperty




//*---------------------------------------------------------------------
// Test Case Section
//*---------------------------------------------------------------------


// {{ TCW_TEST_CASE_MAP(TCBindDSO)
//*---------------------------------------------------------------------
// @class Test DSOs (IBindResource)
//
class TCBindDSO : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindDSO,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Initialize DSO
	int Variation_1();
	// @cmember General - Create Session
	int Variation_2();
	// @cmember General - Get IDBProperties
	int Variation_3();
	// @cmember General - Get IPersist
	int Variation_4();
	// @cmember General - Optional Interfaces
	int Variation_5();
	// @cmember General - Aggregate DSO
	int Variation_6();
	// @cmember General - Create 101 DSOs
	int Variation_7();
	// @cmember Flag - WAITFORINIT
	int Variation_8();
	// @cmember Flag - ASYNCH
	int Variation_9();
	// @cmember Aggregate Implicit Session (try to ...)
	int Variation_10();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindDSO)
#define THE_CLASS TCBindDSO
BEG_TEST_CASE(TCBindDSO, TCBindAndCreate, L"Test DSOs (IBindResource)")
	TEST_VARIATION(1, 		L"General - Initialize DSO")
	TEST_VARIATION(2, 		L"General - Create Session")
	TEST_VARIATION(3, 		L"General - Get IDBProperties")
	TEST_VARIATION(4, 		L"General - Get IPersist")
	TEST_VARIATION(5, 		L"General - Optional Interfaces")
	TEST_VARIATION(6, 		L"General - Aggregate DSO")
	TEST_VARIATION(7, 		L"General - Create 101 DSOs")
	TEST_VARIATION(8, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(9, 		L"Flag - ASYNCH")
	TEST_VARIATION(10, 		L"Aggregate Implicit Session (try to ...)")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCBindSession)
//*---------------------------------------------------------------------
// @class Test SESSION objects (IBindResource)
//
class TCBindSession : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindSession,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get ISessionProperties
	int Variation_1();
	// @cmember General - Get IOpenRowset
	int Variation_2();
	// @cmember General - Get IGetDataSource
	int Variation_3();
	// @cmember General - Optional Interfaces
	int Variation_4();
	// @cmember General - Aggregate Session
	int Variation_5();
	// @cmember General - Create 101 Sessions
	int Variation_6();
	// @cmember Aggregate Implicit Session (try to ...)
	int Variation_7();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindSession)
#define THE_CLASS TCBindSession
BEG_TEST_CASE(TCBindSession, TCBindAndCreate, L"Test SESSION objects (IBindResource)")
	TEST_VARIATION(1, 		L"General - Get ISessionProperties")
	TEST_VARIATION(2, 		L"General - Get IOpenRowset")
	TEST_VARIATION(3, 		L"General - Get IGetDataSource")
	TEST_VARIATION(4, 		L"General - Optional Interfaces")
	TEST_VARIATION(5, 		L"General - Aggregate Session")
	TEST_VARIATION(6, 		L"General - Create 101 Sessions")
	TEST_VARIATION(7, 		L"Aggregate Implicit Session (try to ...)")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindRow)
//*---------------------------------------------------------------------
// @class Test ROW objects (IBindResource)
//
class TCBindRow : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindRow,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IRow
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IGetSession
	int Variation_4();
	// @cmember General - Get IColumnsInfo2 (optional)
	int Variation_5();
	// @cmember General - Get ICreateRow (optional)
	int Variation_6();
	// @cmember General - Aggregate Row
	int Variation_7();
	// @cmember General - Aggregate implicit session
	int Variation_8();
	// @cmember Flag - WAITFORINIT
	int Variation_9();
	// @cmember Flag - READWRITE
	int Variation_10();
	// @cmember Flag - SHARE_DENY_READ
	int Variation_11();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_12();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_13();
	// @cmember Flag - SHARE_DENY_NONE
	int Variation_14();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_15();
	// @cmember Flag - OUTPUT
	int Variation_16();
	// @cmember Flag - ASYNCH
	int Variation_17();
	// @cmember Flag - DELAYFETCHSTREAM
	int Variation_18();
	// @cmember Flag - DELAYFETCHCOLUMNS
	int Variation_19();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindRow)
#define THE_CLASS TCBindRow
BEG_TEST_CASE(TCBindRow, TCBindAndCreate, L"Test ROW objects (IBindResource)")
	TEST_VARIATION(1, 		L"General - Get IRow")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IGetSession")
	TEST_VARIATION(5, 		L"General - Get IColumnsInfo2 (optional)")
	TEST_VARIATION(6, 		L"General - Get ICreateRow (optional)")
	TEST_VARIATION(7, 		L"General - Aggregate Row")
	TEST_VARIATION(8, 		L"General - Aggregate implicit session")
	TEST_VARIATION(9, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(10, 		L"Flag - READWRITE")
	TEST_VARIATION(11, 		L"Flag - SHARE_DENY_READ")
	TEST_VARIATION(12, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(13, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(14, 		L"Flag - SHARE_DENY_NONE")
	TEST_VARIATION(15, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(16, 		L"Flag - OUTPUT")
	TEST_VARIATION(17, 		L"Flag - ASYNCH")
	TEST_VARIATION(18, 		L"Flag - DELAYFETCHSTREAM")
	TEST_VARIATION(19, 		L"Flag - DELAYFETCHCOLUMNS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindRowset)
//*---------------------------------------------------------------------
// @class Test ROWSET objects (IBindResource)
//
class TCBindRowset : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindRowset,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IAccessor
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IRowset
	int Variation_4();
	// @cmember General - Get IRowsetInfo
	int Variation_5();
	// @cmember General - Get IRowsetIdentity
	int Variation_6();
	// @cmember General - Bind to URLs of rows of the Rowset
	int Variation_7();
	// @cmember General - Optional interfaces
	int Variation_8();
	// @cmember General - IGetRow (if ROW objects are supported)
	int Variation_9();
	// @cmember General - Aggregate Rowset
	int Variation_10();
	// @cmember General - Aggregate implicit session
	int Variation_11();
	// @cmember Flag - WAITFORINIT
	int Variation_12();
	// @cmember Flag - READWRITE
	int Variation_13();
	// @cmember Flag - SHARE_DENY_READ
	int Variation_14();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_15();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_16();
	// @cmember Flag - SHARE_DENY_NONE
	int Variation_17();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_18();
	// @cmember Flag - ASYNCH
	int Variation_19();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindRowset)
#define THE_CLASS TCBindRowset
BEG_TEST_CASE(TCBindRowset, TCBindAndCreate, L"Test ROWSET objects (IBindResource)")
	TEST_VARIATION(1, 		L"General - Get IAccessor")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IRowset")
	TEST_VARIATION(5, 		L"General - Get IRowsetInfo")
	TEST_VARIATION(6, 		L"General - Get IRowsetIdentity")
	TEST_VARIATION(7, 		L"General - Bind to URLs of rows of the Rowset")
	TEST_VARIATION(8, 		L"General - Optional interfaces")
	TEST_VARIATION(9, 		L"General - IGetRow (if ROW objects are supported)")
	TEST_VARIATION(10, 		L"General - Aggregate Rowset")
	TEST_VARIATION(11, 		L"General - Aggregate implicit session")
	TEST_VARIATION(12, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(13, 		L"Flag - READWRITE")
	TEST_VARIATION(14, 		L"Flag - SHARE_DENY_READ")
	TEST_VARIATION(15, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(16, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(17, 		L"Flag - SHARE_DENY_NONE")
	TEST_VARIATION(18, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(19, 		L"Flag - ASYNCH")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindStream)
//*---------------------------------------------------------------------
// @class Test STREAM objects (IBindResource)
//
class TCBindStream : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindStream,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IGetSourceRow (optional)
	int Variation_1();
	// @cmember General - Get ISequentialStream
	int Variation_2();
	// @cmember General - Get IStream
	int Variation_3();
	// @cmember General - Aggregate Stream
	int Variation_4();
	// @cmember General - Aggregate Implicit Session
	int Variation_5();
	// @cmember Flag - WAITFORINIT
	int Variation_6();
	// @cmember Flag - READWRITE
	int Variation_7();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_8();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_9();
	// @cmember Flag - OUTPUT
	int Variation_10();
	// @cmember Flag - ASYNCH
	int Variation_11();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindStream)
#define THE_CLASS TCBindStream
BEG_TEST_CASE(TCBindStream, TCBindAndCreate, L"Test STREAM objects (IBindResource)")
	TEST_VARIATION(1, 		L"General - Get IGetSourceRow (optional)")
	TEST_VARIATION(2, 		L"General - Get ISequentialStream")
	TEST_VARIATION(3, 		L"General - Get IStream")
	TEST_VARIATION(4, 		L"General - Aggregate Stream")
	TEST_VARIATION(5, 		L"General - Aggregate Implicit Session")
	TEST_VARIATION(6, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(7, 		L"Flag - READWRITE")
	TEST_VARIATION(8, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(9, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(10, 		L"Flag - OUTPUT")
	TEST_VARIATION(11, 		L"Flag - ASYNCH")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindCommand)
//*-----------------------------------------------------------------------
// @class Test COMMAND objects (IBindResource)
//
class TCBindCommand : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindCommand,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get ICommand
	int Variation_1();
	// @cmember General - Get ICommandText
	int Variation_2();
	// @cmember General - Get ICommandProperties
	int Variation_3();
	// @cmember General - Get IAccessor
	int Variation_4();
	// @cmember General - Get IColumnsInfo
	int Variation_5();
	// @cmember General - Get IConvertType
	int Variation_6();
	// @cmember General - Optional Interfaces
	int Variation_7();
	// @cmember General - Aggregate Command
	int Variation_8();
	// @cmember General - Aggregate Implicit Session
	int Variation_9();
	// @cmember Flag - WAITFORINIT
	int Variation_10();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindCommand)
#define THE_CLASS TCBindCommand
BEG_TEST_CASE(TCBindCommand, TCBindAndCreate, L"Test COMMAND objects (IBindResource)")
	TEST_VARIATION(1, 		L"General - Get ICommand")
	TEST_VARIATION(2, 		L"General - Get ICommandText")
	TEST_VARIATION(3, 		L"General - Get ICommandProperties")
	TEST_VARIATION(4, 		L"General - Get IAccessor")
	TEST_VARIATION(5, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(6, 		L"General - Get IConvertType")
	TEST_VARIATION(7, 		L"General - Optional Interfaces")
	TEST_VARIATION(8, 		L"General - Aggregate Command")
	TEST_VARIATION(9, 		L"General - Aggregate Implicit Session")
	TEST_VARIATION(10, 		L"Flag - WAITFORINIT")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCBind_Boundary)
//*-----------------------------------------------------------------------
// @class IBindResource boundary cases
//
class TCBind_Boundary : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBind_Boundary,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG : pwszURL=NULL
	int Variation_1();
	// @cmember E_INVALIDARG : ppUnk=NULL
	int Variation_2();
	// @cmember E_INVALIDARG : pImplSess->piid = NULL
	int Variation_3();
	// @cmember E_INVALIDARG : No flags set
	int Variation_4();
	// @cmember E_INVALIDARG : RECURSIVE flag
	int Variation_5();
	// @cmember E_INVALIDARG : READWRITE on DSO
	int Variation_6();
	// @cmember E_INVALIDARG : READWRITE on SESSION
	int Variation_7();
	// @cmember E_INVALIDARG : WAITFORINIT on SESSION
	int Variation_8();
	// @cmember E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET
	int Variation_9();
	// @cmember E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM
	int Variation_10();
	// @cmember E_INVALIDARG : Invalid Flags on COMMAND
	int Variation_11();
	// @cmember E_NOINTERFACE : IID_IRowset on DSO
	int Variation_12();
	// @cmember E_NOINTERFACE : IID_IRowset on SESSION
	int Variation_13();
	// @cmember E_NOINTERFACE : IID_IDBProperties on ROWSET
	int Variation_14();
	// @cmember E_NOINTERFACE : IID_IDBProperties on ROW
	int Variation_15();
	// @cmember E_NOINTERFACE : IID_ISessionProperties on STREAM
	int Variation_16();
	// @cmember E_NOINTERFACE : IID_IRowsetInfo on COMMAND
	int Variation_17();
	// @cmember E_NOINTERFACE : riid = IID_NULL
	int Variation_18();
	// @cmember DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs
	int Variation_19();
	// @cmember DB_E_NOTFOUND : bogus URL (DSO)
	int Variation_20();
	// @cmember DB_E_NOTFOUND : bogus URL (ROW)
	int Variation_21();
	// @cmember DB_E_NOTFOUND: Wierd URLs
	int Variation_22();
	// @cmember DB_E_NOAGGREGATION : riid not IID_IUnknown
	int Variation_23();
	// @cmember DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown
	int Variation_24();
	// @cmember DB_E_NOTCOLLECTION : bind to stream URL as Rowset.
	int Variation_25();
	// @cmember REGDB_E_CLASSNOTREG: Dummy Provider Binder
	int Variation_26();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBind_Boundary)
#define THE_CLASS TCBind_Boundary
BEG_TEST_CASE(TCBind_Boundary, TCBindAndCreate, L"IBindResource boundary cases")
	TEST_VARIATION(1, 		L"E_INVALIDARG : pwszURL=NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG : ppUnk=NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG : pImplSess->piid = NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG : No flags set")
	TEST_VARIATION(5, 		L"E_INVALIDARG : RECURSIVE flag")
	TEST_VARIATION(6, 		L"E_INVALIDARG : READWRITE on DSO")
	TEST_VARIATION(7, 		L"E_INVALIDARG : READWRITE on SESSION")
	TEST_VARIATION(8, 		L"E_INVALIDARG : WAITFORINIT on SESSION")
	TEST_VARIATION(9, 		L"E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET")
	TEST_VARIATION(10, 		L"E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM")
	TEST_VARIATION(11, 		L"E_INVALIDARG : Invalid Flags on COMMAND")
	TEST_VARIATION(12, 		L"E_NOINTERFACE : IID_IRowset on DSO")
	TEST_VARIATION(13, 		L"E_NOINTERFACE : IID_IRowset on SESSION")
	TEST_VARIATION(14, 		L"E_NOINTERFACE : IID_IDBProperties on ROWSET")
	TEST_VARIATION(15, 		L"E_NOINTERFACE : IID_IDBProperties on ROW")
	TEST_VARIATION(16, 		L"E_NOINTERFACE : IID_ISessionProperties on STREAM")
	TEST_VARIATION(17, 		L"E_NOINTERFACE : IID_IRowsetInfo on COMMAND")
	TEST_VARIATION(18, 		L"E_NOINTERFACE : riid = IID_NULL")
	TEST_VARIATION(19, 		L"DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs")
	TEST_VARIATION(20, 		L"DB_E_NOTFOUND : bogus URL (DSO)")
	TEST_VARIATION(21, 		L"DB_E_NOTFOUND : bogus URL (ROW)")
	TEST_VARIATION(22, 		L"DB_E_NOTFOUND: Wierd URLs")
	TEST_VARIATION(23, 		L"DB_E_NOAGGREGATION : riid not IID_IUnknown")
	TEST_VARIATION(24, 		L"DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown")
	TEST_VARIATION(25, 		L"DB_E_NOTCOLLECTION : bind to stream URL as Rowset.")
	TEST_VARIATION(26, 		L"REGDB_E_CLASSNOTREG: Dummy Provider Binder")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateRowExist)
//*-----------------------------------------------------------------------
// @class Test ROW objects (ICreateRow)
//
class TCCreateRowExist : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateRowExist,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IRow
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IGetSession
	int Variation_4();
	// @cmember General - Get IColumnsInfo2 (optional)
	int Variation_5();
	// @cmember General - Get ICreateRow (optional)
	int Variation_6();
	// @cmember General - Aggregate Row
	int Variation_7();
	// @cmember General - Aggregate implicit session
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateRowExist)
#define THE_CLASS TCCreateRowExist
BEG_TEST_CASE(TCCreateRowExist, TCBindAndCreate, L"Test ROW objects (ICreateRow)")
	TEST_VARIATION(1, 		L"General - Get IRow")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IGetSession")
	TEST_VARIATION(5, 		L"General - Get IColumnsInfo2 (optional)")
	TEST_VARIATION(6, 		L"General - Get ICreateRow (optional)")
	TEST_VARIATION(7, 		L"General - Aggregate Row")
	TEST_VARIATION(8, 		L"General - Aggregate implicit session")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateRowsetExist)
//*-----------------------------------------------------------------------
// @class Test existing rowset objects (ICreateRow)
//
class TCCreateRowsetExist : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateRowsetExist,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IAccessor
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IRowset
	int Variation_4();
	// @cmember General - Get IRowsetInfo
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateRowsetExist)
#define THE_CLASS TCCreateRowsetExist
BEG_TEST_CASE(TCCreateRowsetExist, TCBindAndCreate, L"Test existing rowset objects (ICreateRow)")
	TEST_VARIATION(1, 		L"General - Get IAccessor")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IRowset")
	TEST_VARIATION(5, 		L"General - Get IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateNewRow)
//*-----------------------------------------------------------------------
// @class Test ROW objects (ICreateRow)
//
class TCCreateNewRow : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateNewRow,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IRow
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IGetSession
	int Variation_4();
	// @cmember General - Aggregate Row
	int Variation_5();
	// @cmember General - Aggregate implicit session
	int Variation_6();
	// @cmember Flag - READWRITE
	int Variation_7();
	// @cmember Flag - WAITFORINIT
	int Variation_8();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_9();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_10();
	// @cmember Flag - ASYNCH
	int Variation_11();
	// @cmember DB_E_RESOURCENOTSUPPORTED
	int Variation_12();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateNewRow)
#define THE_CLASS TCCreateNewRow
BEG_TEST_CASE(TCCreateNewRow, TCBindAndCreate, L"Test ROW objects (ICreateRow)")
	TEST_VARIATION(1, 		L"General - Get IRow")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IGetSession")
	TEST_VARIATION(5, 		L"General - Aggregate Row")
	TEST_VARIATION(6, 		L"General - Aggregate implicit session")
	TEST_VARIATION(7, 		L"Flag - READWRITE")
	TEST_VARIATION(8, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(9, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(10, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(11, 		L"Flag - ASYNCH")
	TEST_VARIATION(12, 		L"DB_E_RESOURCENOTSUPPORTED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateNewRowset)
//*-----------------------------------------------------------------------
// @class Test Creating Rowsets (ICreateRow)
//
class TCCreateNewRowset : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateNewRowset,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IAccessor
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IRowset
	int Variation_4();
	// @cmember General - Get IRowsetInfo
	int Variation_5();
	// @cmember General - Get IRowsetIdentity
	int Variation_6();
	// @cmember General - Get IGetRow (optional)
	int Variation_7();
	// @cmember General - Aggregate rowset
	int Variation_8();
	// @cmember General - Aggregate implicit session
	int Variation_9();
	// @cmember Flag - READWRITE
	int Variation_10();
	// @cmember Flag - WAITFORINIT
	int Variation_11();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_12();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_13();
	// @cmember Flag - ASYNCH
	int Variation_14();
	// @cmember DB_E_RESOURCENOTSUPPORTED
	int Variation_15();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateNewRowset)
#define THE_CLASS TCCreateNewRowset
BEG_TEST_CASE(TCCreateNewRowset, TCBindAndCreate, L"Test Creating Rowsets (ICreateRow)")
	TEST_VARIATION(1, 		L"General - Get IAccessor")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IRowset")
	TEST_VARIATION(5, 		L"General - Get IRowsetInfo")
	TEST_VARIATION(6, 		L"General - Get IRowsetIdentity")
	TEST_VARIATION(7, 		L"General - Get IGetRow (optional)")
	TEST_VARIATION(8, 		L"General - Aggregate rowset")
	TEST_VARIATION(9, 		L"General - Aggregate implicit session")
	TEST_VARIATION(10, 		L"Flag - READWRITE")
	TEST_VARIATION(11, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(12, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(13, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(14, 		L"Flag - ASYNCH")
	TEST_VARIATION(15, 		L"DB_E_RESOURCENOTSUPPORTED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateStream)
//*-----------------------------------------------------------------------
// @class Test creating Streams (directly)
//
class TCCreateStream : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateStream,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IGetSourceRow (optional)
	int Variation_1();
	// @cmember General - Get ISequentialStream
	int Variation_2();
	// @cmember General - Get IStream (optional)
	int Variation_3();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateStream)
#define THE_CLASS TCCreateStream
BEG_TEST_CASE(TCCreateStream, TCBindAndCreate, L"Test creating Streams (directly)")
	TEST_VARIATION(1, 		L"General - Get IGetSourceRow (optional)")
	TEST_VARIATION(2, 		L"General - Get ISequentialStream")
	TEST_VARIATION(3, 		L"General - Get IStream (optional)")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCreate_Boundary)
//*-----------------------------------------------------------------------
// @class ICreateRow boundary cases
//
class TCCreate_Boundary : public TCBindAndCreate { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreate_Boundary,TCBindAndCreate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG : pwszURL=NULL
	int Variation_1();
	// @cmember E_INVALIDARG : ppUnk=NULL
	int Variation_2();
	// @cmember E_INVALIDARG : pImplSess->piid =NULL
	int Variation_3();
	// @cmember E_INVALIDARG : No flags set
	int Variation_4();
	// @cmember E_INVALIDARG : RECURSIVE flag
	int Variation_5();
	// @cmember E_INVALIDARG : OPENIFEXISTS and OVERWRITE
	int Variation_6();
	// @cmember E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET
	int Variation_7();
	// @cmember E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM
	int Variation_8();
	// @cmember E_NOINTERFACE : IID_IDBProperties on ROWSET
	int Variation_9();
	// @cmember E_NOINTERFACE : IID_IDBProperties on ROW
	int Variation_10();
	// @cmember E_NOINTERFACE : IID_ISessionProperties on STREAM
	int Variation_11();
	// @cmember E_NOINTERFACE : riid = IID_NULL
	int Variation_12();
	// @cmember DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs
	int Variation_13();
	// @cmember DB_E_NOTSUPPORTED : rguid = DSO, SESSION, COMMAND
	int Variation_14();
	// @cmember DB_E_NOTFOUND : bogus URL (ROW)
	int Variation_15();
	// @cmember DB_E_NOTFOUND: Wierd URLs
	int Variation_16();
	// @cmember DB_E_NOAGGREGATION : riid not IID_IUnknown
	int Variation_17();
	// @cmember DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown
	int Variation_18();
	// @cmember DB_E_NOTCOLLECTION : create row under a non-collection parent
	int Variation_19();
	// @cmember DB_E_NOTCOLLECTION: Flags (OPENIFEXISTS|COLLECTION) on existing non-collection.
	int Variation_20();
	// @cmember DB_E_RESOURCEEXISTS: Flag (OPENIFEXISTS) on an existing collection.
	int Variation_21();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreate_Boundary)
#define THE_CLASS TCCreate_Boundary
BEG_TEST_CASE(TCCreate_Boundary, TCBindAndCreate, L"ICreateRow boundary cases")
	TEST_VARIATION(1, 		L"E_INVALIDARG : pwszURL=NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG : ppUnk=NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG : pImplSess->piid =NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG : No flags set")
	TEST_VARIATION(5, 		L"E_INVALIDARG : RECURSIVE flag")
	TEST_VARIATION(6, 		L"E_INVALIDARG : OPENIFEXISTS and OVERWRITE")
	TEST_VARIATION(7, 		L"E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET")
	TEST_VARIATION(8, 		L"E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM")
	TEST_VARIATION(9, 		L"E_NOINTERFACE : IID_IDBProperties on ROWSET")
	TEST_VARIATION(10, 		L"E_NOINTERFACE : IID_IDBProperties on ROW")
	TEST_VARIATION(11, 		L"E_NOINTERFACE : IID_ISessionProperties on STREAM")
	TEST_VARIATION(12, 		L"E_NOINTERFACE : riid = IID_NULL")
	TEST_VARIATION(13, 		L"DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs")
	TEST_VARIATION(14, 		L"DB_E_NOTSUPPORTED : rguid = DSO, SESSION, COMMAND")
	TEST_VARIATION(15, 		L"DB_E_NOTFOUND : bogus URL (ROW)")
	TEST_VARIATION(16, 		L"DB_E_NOTFOUND: Wierd URLs")
	TEST_VARIATION(17, 		L"DB_E_NOAGGREGATION : riid not IID_IUnknown")
	TEST_VARIATION(18, 		L"DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown")
	TEST_VARIATION(19, 		L"DB_E_NOTCOLLECTION : create row under a non-collection parent")
	TEST_VARIATION(20, 		L"DB_E_NOTCOLLECTION: Flags (OPENIFEXISTS|COLLECTION) on existing non-collection.")
	TEST_VARIATION(21, 		L"DB_E_RESOURCEEXISTS: Flag (OPENIFEXISTS) on an existing collection.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCTransactCreateRow)
//*-----------------------------------------------------------------------
// @class ICreateRow transaction tests
//
class TCTransactCreateRow : public Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransactCreateRow,Zombie);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining set to TRUE
	int Variation_1();
	// @cmember Abort with fRetaining set to FALSE
	int Variation_2();
	// @cmember Commit with fRetaining set to TRUE
	int Variation_3();
	// @cmember Commit with fRetaining set to FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCTransactCreateRow)
#define THE_CLASS TCTransactCreateRow
BEG_TEST_CASE(TCTransactCreateRow, Zombie, L"ICreateRow transaction tests")
	TEST_VARIATION(1, 		L"Abort with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"Abort with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"Commit with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"Commit with fRetaining set to FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()



////////////////////////////////////////////////////////////////////////
// Copying Test Cases to make duplicate ones.
//
////////////////////////////////////////////////////////////////////////
#define COPY_TEST_CASE(theClass, baseClass)						\
	class theClass : public baseClass							\
	{															\
	public:														\
		static const WCHAR		m_wszTestCaseName[];			\
		DECLARE_TEST_CASE_FUNCS(theClass, baseClass);			\
	};															\
const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\


#define TEST_CASE_WITH_PARAM(iCase, theClass, param)			\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetTestCaseParam(param);		\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;


//Bind_Session
COPY_TEST_CASE(TCBindDSO_PB, TCBindDSO)
COPY_TEST_CASE(TCBindSession_PB, TCBindSession)
COPY_TEST_CASE(TCBindRow_PB, TCBindRow)
COPY_TEST_CASE(TCBindRowset_PB, TCBindRowset)
COPY_TEST_CASE(TCBindStream_PB, TCBindStream)
COPY_TEST_CASE(TCBindCommand_PB, TCBindCommand)
COPY_TEST_CASE(TCBind_Boundary_PB, TCBind_Boundary)

COPY_TEST_CASE(TCBindDSO_SESS, TCBindDSO)
COPY_TEST_CASE(TCBindSession_SESS, TCBindSession)
COPY_TEST_CASE(TCBindRow_SESS, TCBindRow)
COPY_TEST_CASE(TCBindRowset_SESS, TCBindRowset)
COPY_TEST_CASE(TCBindStream_SESS, TCBindStream)
COPY_TEST_CASE(TCBindCommand_SESS, TCBindCommand)
COPY_TEST_CASE(TCBind_Boundary_SESS, TCBind_Boundary)

//CreateRow_Session
COPY_TEST_CASE(TCCreateRowExist_PB, TCCreateRowExist)
COPY_TEST_CASE(TCCreateRowsetExist_PB, TCCreateRowsetExist)
COPY_TEST_CASE(TCCreateNewRow_PB, TCCreateNewRow)
COPY_TEST_CASE(TCCreateNewRowset_PB, TCCreateNewRowset)
COPY_TEST_CASE(TCCreateStream_PB, TCCreateStream)
COPY_TEST_CASE(TCCreate_Boundary_PB, TCCreate_Boundary)

COPY_TEST_CASE(TCCreateRowExist_SESS, TCCreateRowExist)
COPY_TEST_CASE(TCCreateRowsetExist_SESS, TCCreateRowsetExist)
COPY_TEST_CASE(TCCreateNewRow_SESS, TCCreateNewRow)
COPY_TEST_CASE(TCCreateNewRowset_SESS, TCCreateNewRowset)
COPY_TEST_CASE(TCCreateStream_SESS, TCCreateStream)
COPY_TEST_CASE(TCCreate_Boundary_SESS, TCCreate_Boundary)

//CreateRow_Row
COPY_TEST_CASE(TCCreateRowExist_ROW, TCCreateRowExist)
COPY_TEST_CASE(TCCreateRowsetExist_ROW, TCCreateRowsetExist)
COPY_TEST_CASE(TCCreateNewRow_ROW, TCCreateNewRow)
COPY_TEST_CASE(TCCreateNewRowset_ROW, TCCreateNewRowset)
COPY_TEST_CASE(TCCreateStream_ROW, TCCreateStream)
COPY_TEST_CASE(TCCreate_Boundary_ROW, TCCreate_Boundary)


//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a parameter which changes the behvior.
//So we make LTM think there are 15 cases in here with different names, but in
//reality we only have to maintain code for the unique cases.  This way we can
//easily get testing of the interfaces on other objects, without maintaining different
//tests with almost identical code...

#if 0 
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(14, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCBindDSO)
	TEST_CASE(2, TCBindSession)
	TEST_CASE(3, TCBindRow)
	TEST_CASE(4, TCBindRowset)
	TEST_CASE(5, TCBindStream)
	TEST_CASE(6, TCBindCommand)
	TEST_CASE(7, TCBind_Boundary)
	TEST_CASE(8, TCCreateRowExist)
	TEST_CASE(9, TCCreateRowsetExist)
	TEST_CASE(10, TCCreateNewRow)
	TEST_CASE(11, TCCreateNewRowset)
	TEST_CASE(12, TCCreateStream)
	TEST_CASE(13, TCCreate_Boundary)
	TEST_CASE(14, TCTransactCreateRow)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(46, ThisModule, gwszModuleDescrip)
	//IID_ISequentialStream
	TEST_CASE(1, TCBindDSO)
	TEST_CASE(2, TCBindSession)
	TEST_CASE(3, TCBindRow)
	TEST_CASE(4, TCBindRowset)
	TEST_CASE(5, TCBindStream)
	TEST_CASE(6, TCBindCommand)
	TEST_CASE(7, TCBind_Boundary)

	TEST_CASE_WITH_PARAM(8, TCBindDSO_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(9, TCBindSession_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(10, TCBindRow_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(11, TCBindRowset_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(12, TCBindStream_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(13, TCBindCommand_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(14, TCBind_Boundary_PB, TC_PBINDER)

	TEST_CASE_WITH_PARAM(15, TCBindDSO_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(16, TCBindSession_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(17, TCBindRow_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(18, TCBindRowset_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(19, TCBindStream_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(20, TCBindCommand_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(21, TCBind_Boundary_SESS, TC_SESSION)

	TEST_CASE(22, TCCreateRowExist)
	TEST_CASE(23, TCCreateRowsetExist)
	TEST_CASE(24, TCCreateNewRow)
	TEST_CASE(25, TCCreateNewRowset)
	TEST_CASE(26, TCCreateStream)
	TEST_CASE(27, TCCreate_Boundary)

	TEST_CASE_WITH_PARAM(28, TCCreateRowExist_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(29, TCCreateRowsetExist_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(30, TCCreateNewRow_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(31, TCCreateNewRowset_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(32, TCCreateStream_PB, TC_PBINDER)
	TEST_CASE_WITH_PARAM(33, TCCreate_Boundary_PB, TC_PBINDER)

	TEST_CASE_WITH_PARAM(34, TCCreateRowExist_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(35, TCCreateRowsetExist_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(36, TCCreateNewRow_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(37, TCCreateNewRowset_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(38, TCCreateStream_SESS, TC_SESSION)
	TEST_CASE_WITH_PARAM(39, TCCreate_Boundary_SESS, TC_SESSION)

	TEST_CASE_WITH_PARAM(40, TCCreateRowExist_ROW, TC_ROW)
	TEST_CASE_WITH_PARAM(41, TCCreateRowsetExist_ROW, TC_ROW)
	TEST_CASE_WITH_PARAM(42, TCCreateNewRow_ROW, TC_ROW)
	TEST_CASE_WITH_PARAM(43, TCCreateNewRowset_ROW, TC_ROW)
	TEST_CASE_WITH_PARAM(44, TCCreateStream_ROW, TC_ROW)
	TEST_CASE_WITH_PARAM(45, TCCreate_Boundary_ROW, TC_ROW)

	TEST_CASE(46, TCTransactCreateRow)

END_TEST_MODULE()
#endif


// {{ TCW_TC_PROTOTYPE(TCBindDSO)
//*---------------------------------------------------------------------
//| Test Case:		TCBindDSO - Test DSOs (IBindResource)
//| Created:  	8/3/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindDSO::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc General - Initialize DSO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_1()
{ 
	TBEGIN
	ULONG			ulIndex;
	DBBINDURLSTATUS	dwBindStatus = 0; 
	IGetDataSource*	pIGDS = NULL;
	IUnknown*		pIUnk = NULL;
	IDBInitialize*	pIDBInitialize = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IDBInitialize, &dwBindStatus, (IUnknown**)
			&pIDBInitialize), S_OK)

		//Make sure DSO obtained is initialized.
		TESTC_(pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED)

		TESTC(DefTestInterface(pIDBInitialize))

		//If Bind is implemented on a session, the DSO returned
		//should be same as the one on which this session exists.
		if(TC_SESSION == m_eTestCase)
		{
			TESTC(VerifyInterface(m_pIBindResource, IID_IGetDataSource, 
				SESSION_INTERFACE, (IUnknown**)&pIGDS))	
			TESTC_(pIGDS->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnk), S_OK)
			TESTC(VerifyEqualInterface(pIUnk, pIDBInitialize))
		}

		SAFE_RELEASE(pIDBInitialize);
		SAFE_RELEASE(pIUnk);
		SAFE_RELEASE(pIGDS);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIGDS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc General - Create Session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_2()
{ 
	TBEGIN
	ULONG				ulIndex;
	IGetDataSource*		pIGDS = NULL;
	IUnknown*			pIUnk = NULL;
	IDBCreateSession*	pIDBCreateSession = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IDBCreateSession, NULL, (IUnknown**)
			&pIDBCreateSession), S_OK)

		TESTC(DefTestInterface(pIDBCreateSession))

		//If Bind is implemented on a session, the DSO returned
		//should be same as the one on which this session exists.
		if(TC_SESSION == m_eTestCase)
		{
			TESTC(VerifyInterface(m_pIBindResource, IID_IGetDataSource, 
				SESSION_INTERFACE, (IUnknown**)&pIGDS))	
			TESTC_(pIGDS->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnk), S_OK)
			TESTC(VerifyEqualInterface(pIUnk, pIDBCreateSession))
		}

		SAFE_RELEASE(pIDBCreateSession);
		SAFE_RELEASE(pIUnk);
		SAFE_RELEASE(pIGDS);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIGDS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc General - Get IDBProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_3()
{ 
	TBEGIN
	ULONG				ulIndex;
	ULONG_PTR			ulOleObj=0;
	IGetDataSource*		pIGDS = NULL;
	IUnknown*			pIUnk = NULL;
	IDBProperties*		pIDBProperties = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IDBProperties, NULL, (IUnknown**)
			&pIDBProperties), S_OK)

		TESTC(DefTestInterface(pIDBProperties))

		//Get the value of DBPROP_OLEOBJECTS using the obtained
		//IDBProperties interface. Verify its value.
		COMPARE(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
			pIDBProperties, &ulOleObj), TRUE);

		//The prop should have DBPROPVAL_OO_DIRECTBIND bit set.
		COMPARE(ulOleObj & DBPROPVAL_OO_DIRECTBIND, DBPROPVAL_OO_DIRECTBIND);

		//Check if prop has DBPROPVAL_OO_DIRECTBIND bit set.
		if(!(ulOleObj & DBPROPVAL_OO_ROWOBJECT))
		{
			odtLog<<L"INFO: ROW objects are not supported.\n";
			COMPAREW(ulOleObj & DBPROPVAL_OO_ROWOBJECT, DBPROPVAL_OO_ROWOBJECT);
		}

		//If Bind is implemented on a session, the DSO returned
		//should be same as the one on which this session exists.
		if(TC_SESSION == m_eTestCase)
		{
			TESTC(VerifyInterface(m_pIBindResource, IID_IGetDataSource, 
				SESSION_INTERFACE, (IUnknown**)&pIGDS))	
			TESTC_(pIGDS->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnk), S_OK)
			TESTC(VerifyEqualInterface(pIUnk, pIDBProperties))
		}

		SAFE_RELEASE(pIDBProperties);
		SAFE_RELEASE(pIUnk);
		SAFE_RELEASE(pIGDS);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIGDS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc General - Get IPersist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_4()
{ 
	TBEGIN
	ULONG			ulIndex;
	CLSID			clsid;
	DBBINDURLSTATUS	dwBindStatus = 0; 
	IPersist*		pIPersist = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IPersist, &dwBindStatus, (IUnknown**)
			&pIPersist), S_OK)

		TESTC(DefTestInterface(pIPersist))

		//Call GetClassID method on this interface and verify.
		TESTC_(pIPersist->GetClassID(&clsid), S_OK)
		TESTC(clsid == GetModInfo()->GetThisTestModule()->m_ProviderClsid)
		SAFE_RELEASE(pIPersist);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIPersist);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc General - Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	LPOLESTR			pwszKeywords = NULL;
	IDBInfo*			pIDBInfo = NULL;
	ISupportErrorInfo*	pISupportErrorInfo = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		//Bind to get IDBInfo.
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IDBInfo, &dwBindStatus, (IUnknown**)
			&pIDBInfo), S_OK, E_NOINTERFACE)

		if(hr == S_OK)
			TESTC_(pIDBInfo->GetKeywords(&pwszKeywords), S_OK)

		//Bind to get ISupportErrorInfo.
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_ISupportErrorInfo, &dwBindStatus, (IUnknown**)
			&pISupportErrorInfo), S_OK, E_NOINTERFACE)

		if(hr == S_OK)
			TEST2C_(hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(
				IID_IDBProperties), S_OK, S_FALSE)

		SAFE_FREE(pwszKeywords);
		SAFE_RELEASE(pIDBInfo);
		SAFE_RELEASE(pISupportErrorInfo);
	}

	//Call Bind with all interfaces listed as DATASOURCE_INTERFACE.
	QTESTC(testAllIntfBind(DATASOURCE_INTERFACE))

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIDBInfo);
	SAFE_RELEASE(pISupportErrorInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate DSO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnkInner = NULL;
	IUnknown*			pIUnk = NULL;
	IDBCreateSession*	pIDBCS = NULL;
	IGetDataSource*		pIGDS = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		CAggregate Aggregate(m_pIBindResource);

		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IUnknown, &dwBindStatus, (IUnknown**)
			&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
			&Aggregate), S_OK, DB_E_NOAGGREGATION)

		Aggregate.SetUnkInner(pIUnkInner);

		//Verify Aggregation.
		if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
		{
			TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));
			
			//Get a session from this DSO interface, go back to the DSO using
			//IGetDataSource, and verify you got outer IUnknown,
			//NOT inner.
			TESTC(VerifyInterface(pIUnkInner, IID_IDBCreateSession, 
				DATASOURCE_INTERFACE, (IUnknown**)&pIDBCS))	
			TESTC_(pIDBCS->CreateSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGDS), S_OK)
			TESTC_(pIGDS->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnk), S_OK)
			TESTC(VerifyEqualInterface(pIUnk, pIDBCS))
			TESTC(!VerifyEqualInterface(pIUnk, pIUnkInner))

			SAFE_RELEASE(pIDBCS);
			SAFE_RELEASE(pIGDS);
			SAFE_RELEASE(pIUnk);
		}
		else
			//If Bind is implemeted on a Session, pUnkOuter has
			//to be NULL, otherwise DB_E_NOAGGREGATION is returned.
			TESTC_(hr, DB_E_NOAGGREGATION)

		SAFE_RELEASE(pIUnkInner);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*---------------------------------------------------------------------
// @mfunc General - Create 101 DSOs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_7()
{ 
	TBEGIN
	ULONG			ulIndex;
	DBBINDURLSTATUS	dwBindStatus = 0; 
	IDBInitialize*	rgIDBInitialize[101] ;

	memset(rgIDBInitialize, 0, 101*sizeof(IDBInitialize*));

	//Create a DSO 101 times.
	for(ulIndex=0; ulIndex<101; ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[DSO], DBGUID_DSO, 
			IID_IDBInitialize, &dwBindStatus, (IUnknown**)
			&rgIDBInitialize[ulIndex]), S_OK)

		SAFE_RELEASE(rgIDBInitialize[ulIndex]);
	}

CLEANUP:
	for(ulIndex=0; ulIndex<101; ulIndex++)
		SAFE_RELEASE(rgIDBInitialize[ulIndex]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IDBInitialize*		pIDBInitialize = NULL;
	IDBCreateSession*	pIDBCS = NULL;
	IOpenRowset*		pIOR = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_DSO, 
			IID_IDBInitialize, &dwBindStatus, (IUnknown**)
			&pIDBInitialize, DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), 
			S_OK, E_INVALIDARG)

		//Cannot use WAITFORINIT flag when Bind is on Session.
		if(TC_SESSION == m_eTestCase)
			TESTC_(hr, E_INVALIDARG)
		else
		{
			//WAITFORINIT has to be supported when Bind is on a
			//Binder and object being asked for is DSO.
			TESTC_(hr, S_OK)

			//Try to QI for IDBCreateSession. This might fail
			//since DSo is not initialized.
			TEST3C_(hr=pIDBInitialize->QueryInterface(IID_IDBCreateSession, 
				(void**)&pIDBCS), E_UNEXPECTED, E_NOINTERFACE, S_OK)

			//If the QI succeeded, then the call to CreateSession
			//should fail.
			if(hr==S_OK)
			{
				TESTC(pIDBCS != NULL)
				TESTC_(pIDBCS->CreateSession(NULL, IID_IOpenRowset, 
					(IUnknown**)&pIOR), E_UNEXPECTED)
			}

			//Initialize the DSO now and verify.
			TESTC_(pIDBInitialize->Initialize(), S_OK)
			TESTC(DefTestInterface(pIDBInitialize))
		}

		SAFE_RELEASE(pIOR);
		SAFE_RELEASE(pIDBCS);
		SAFE_RELEASE(pIDBInitialize);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IDBAsynchStatus*	pAsynchStatus = NULL;

	//Bind aynchronously.
	TEST3C_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_ASYNCHRONOUS), S_OK, 
		DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)
	
	//If S_OK, make sure DSO is in an initialized state.
	if(S_OK == hr)
	{
		odtLog<<L"WARNING: The Bind call with ASYNCH returned immediately !\n";
		TESTC(DefaultObjectTesting(pAsynchStatus, DATASOURCE_INTERFACE))
		goto CLEANUP;
	}
	//Skip test if asynch is not supported.
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: ASYNCH binding is not supported.\n";
		goto CLEANUP;
	}

	//Allow the asynch operation to complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))
	TESTC(DefaultObjectTesting(pAsynchStatus, DATASOURCE_INTERFACE))
	SAFE_RELEASE(pAsynchStatus);

	//Bind aynchronously again and try to abort.

	TEST3C_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_ASYNCHRONOUS), S_OK, 
		DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)

	if(hr == DB_S_ASYNCHRONOUS)
	{
		TESTC(pAsynchStatus != NULL)
		TESTC_(pAsynchStatus->Abort(NULL, DBASYNCHOP_OPEN), S_OK)
	}

CLEANUP:
	SAFE_RELEASE(pAsynchStatus);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Aggregate Implicit Session (try to ...)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindDSO::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IID					iid = IID_IUnknown;
	ULONG				ulRef = 0;
	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;
	IDBInitialize*		pIDBInitialize = NULL;

	memset(&dbImplSess, 0, sizeof(DBIMPLICITSESSION));
	dbImplSess.piid	= &iid;
	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.pSession = NULL;
	ulRef = Aggregate.GetRefCount();

	//The implicit session struct should get ignored.
	TEST2C_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IDBInitialize, NULL, (IUnknown**)
		&pIDBInitialize, DBBINDURLFLAG_READ, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION) ;

	//Make sure the dbImplSess struct has not been modified,
	//and Aggregate has not been AddRef'ed.
	TESTC(*dbImplSess.piid == iid)
	TESTC(!dbImplSess.pSession)
	TESTC(Aggregate.GetRefCount() == ulRef)

	if(hr == S_OK)
		TESTC(DefTestInterface(pIDBInitialize))

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindDSO::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBindSession)
//*---------------------------------------------------------------------
//| Test Case:		TCBindSession - Test SESSION objects (IBindResource)
//| Created:  	8/4/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindSession::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc General - Get ISessionProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_1()
{ 
	TBEGIN
	ULONG				ulIndex;
	ISessionProperties*	pISessionProperties = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_ISessionProperties, NULL, (IUnknown**)
			&pISessionProperties), S_OK)

		//Test obtained interface.
		TESTC(DefTestInterface(pISessionProperties))

		//If the Bind is implemented on a session, then make
		//sure the session returned is the same one that the 
		//Bind is implemented on.
		if(TC_SESSION == m_eTestCase)
			TESTC(VerifyEqualInterface(m_pIBindResource, pISessionProperties))

		SAFE_RELEASE(pISessionProperties);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pISessionProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc General - Get IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_2()
{ 
	TBEGIN
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IOpenRowset*		pIOpenRowset = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_IOpenRowset, &dwBindStatus, (IUnknown**)
			&pIOpenRowset), S_OK)

		//Test obtained interface.
		TESTC(testIOpenRowset(pIOpenRowset))

		//If the Bind is implemented on a session, then make
		//sure the session returned is the same one that the 
		//Bind is implemented on.
		if(TC_SESSION == m_eTestCase)
			TESTC(VerifyEqualInterface(m_pIBindResource, pIOpenRowset))

		SAFE_RELEASE(pIOpenRowset);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIOpenRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc General - Get IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_3()
{ 
	TBEGIN
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IGetDataSource*		pIGetDataSource = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		TESTC_(BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_IGetDataSource, &dwBindStatus, (IUnknown**)
			&pIGetDataSource), S_OK)

		//Test obtained interface.
		TESTC(DefTestInterface(pIGetDataSource))

		//If the Bind is implemented on a session, then make
		//sure the session returned is the same one that the 
		//Bind is implemented on.
		if(TC_SESSION == m_eTestCase)
			TESTC(VerifyEqualInterface(m_pIBindResource, pIGetDataSource))

		SAFE_RELEASE(pIGetDataSource);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIGetDataSource);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IDBSchemaRowset*	pIDBSchemaRowset = NULL;
	IDBCreateCommand*	pIDBCreateCommand = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		//Call bind and ask for IDBSchemaRowset.
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_IDBSchemaRowset, &dwBindStatus, (IUnknown**)
			&pIDBSchemaRowset), S_OK, E_NOINTERFACE)

		if(hr==S_OK)
			TESTC(DefTestInterface(pIDBSchemaRowset))

		SAFE_RELEASE(pIDBSchemaRowset);

		//Call bind and ask for IDBCreateCommand.
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_IDBCreateCommand, &dwBindStatus, (IUnknown**)
			&pIDBCreateCommand), S_OK, E_NOINTERFACE)

		if(hr==S_OK)
			TESTC(DefTestInterface(pIDBCreateCommand))

		SAFE_RELEASE(pIDBCreateCommand);
	}

	//Call Bind with all interfaces listed as SESSION_INTERFACE.
	QTESTC(testAllIntfBind(SESSION_INTERFACE))

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIDBCreateCommand);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate Session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown* pIUnkInner = NULL;

	for(ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
	{
		CAggregate Aggregate(m_pIBindResource);

		//Aggregate the session being bound to.
		TEST2C_(hr = BindResource(m_rgURLs[ulIndex], DBGUID_SESSION, 
			IID_IUnknown, &dwBindStatus, (IUnknown**)
			&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
			&Aggregate), S_OK, DB_E_NOAGGREGATION)

		Aggregate.SetUnkInner(pIUnkInner);

		//Verify Aggregation.
		if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
			TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISessionProperties))
		else
			//If Bind is implemeted on a Session, pUnkOuter has
			//to be NULL, otherwise DB_E_NOAGGREGATION is returned.
			TESTC_(hr, DB_E_NOAGGREGATION)

		SAFE_RELEASE(pIUnkInner);
	}

CLEANUP:
	OUTPUT_FAILEDURL(ulIndex)
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc General - Create 101 Sessions
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_6()
{ 
	TBEGIN
	HRESULT			hr = E_FAIL;
	ULONG			ulIndex;
	DBBINDURLSTATUS	dwBindStatus = 0; 
	IOpenRowset*	rgIOpenRowset[101] ;

	memset(rgIOpenRowset, 0, 101*sizeof(IOpenRowset*));

	//Create a session 101 times.
	for(ulIndex=0; ulIndex<101; ulIndex++)
	{
		TEST2C_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
			IID_IOpenRowset, &dwBindStatus, (IUnknown**)
			&rgIOpenRowset[ulIndex]), S_OK, DB_E_OBJECTCREATIONLIMITREACHED)

		SAFE_RELEASE(rgIOpenRowset[ulIndex]);
	}

CLEANUP:
	for(ulIndex=0; ulIndex<101; ulIndex++)
		SAFE_RELEASE(rgIOpenRowset[ulIndex]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Aggregate Implicit Session (try to ...)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindSession::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IID					iid = IID_IUnknown;
	ULONG				ulRef = 0;
	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;
	IOpenRowset*		pIOR = NULL;

	memset(&dbImplSess, 0, sizeof(DBIMPLICITSESSION));
	dbImplSess.piid	= &iid;
	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.pSession = NULL;
	ulRef = Aggregate.GetRefCount();

	//The implicit session struct should get ignored.
	TEST2C_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_IOpenRowset, NULL, (IUnknown**)
		&pIOR, DBBINDURLFLAG_READ, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION) ;

	//Make sure the dbImplSess struct has not been modified,
	//and Aggregate has not been AddRef'ed.
	TESTC(*dbImplSess.piid == iid)
	TESTC(!dbImplSess.pSession)
	TESTC(Aggregate.GetRefCount() == ulRef)

	if(hr == S_OK)
		TESTC(DefTestInterface(pIOR))

CLEANUP:
	SAFE_RELEASE(pIOR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindSession::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBindRow)
//*---------------------------------------------------------------------
//| Test Case:		TCBindRow - Test ROW objects (IBindResource)
//| Created:  	8/5/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindRow::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		QTESTC(InitTC())
		TESTC(InitializeRowsetURLs())

		//Skip test case if ROW objects are not supported.
		TESTC_PROVIDER(m_rgRowsetURLs)

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc General - Get IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_1()
{ 
	TBEGIN
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TESTC_(BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**)
			&pIRow), S_OK)

		//Test the obtained IRow interface.
		TESTC(testIRow(pIRow, ulIndex+1))

		SAFE_RELEASE(pIRow);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_2()
{ 
	TBEGIN
	DBCOUNTITEM			ulIndex;
	IColumnsInfo*		pIColumnsInfo = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TESTC_(BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IColumnsInfo, NULL, (IUnknown**)
			&pIColumnsInfo), S_OK)

		//Test the obtained IColumnsInfo interface.
		TESTC(DefTestInterface(pIColumnsInfo))

		SAFE_RELEASE(pIColumnsInfo);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_3()
{ 
	TBEGIN
	DBCOUNTITEM			ulIndex; 
	IConvertType*		pIConvertType = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TESTC_(BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IConvertType, NULL, (IUnknown**)
			&pIConvertType), S_OK)

		TESTC(DefTestInterface(pIConvertType))

		SAFE_RELEASE(pIConvertType);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc General - Get IGetSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_4()
{ 
	TBEGIN
	DBCOUNTITEM			ulIndex;
	IGetSession*		pIGetSession = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TESTC_(BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IGetSession, NULL, (IUnknown**)
			&pIGetSession), S_OK)

		TESTC(testIGetSession(pIGetSession))

		SAFE_RELEASE(pIGetSession);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIGetSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo2 (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IColumnsInfo2*		pIColumnsInfo2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IColumnsInfo2, &dwBindStatus, (IUnknown**)
			&pIColumnsInfo2), S_OK, E_NOINTERFACE)

		if(hr==S_OK)
			TESTC(testIColumnsInfo2(pIColumnsInfo2))

		SAFE_RELEASE(pIColumnsInfo2);
	}

	//Call Bind with all interfaces listed as ROW_INTERFACE.
	QTESTC(testAllIntfBind(ROW_INTERFACE))

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIColumnsInfo2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Get ICreateRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ICreateRow*			pICreateRow = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_ICreateRow, &dwBindStatus, (IUnknown**)
			&pICreateRow), S_OK, E_NOINTERFACE)

		if(hr == S_OK)
			TESTC(testICreateRow(pICreateRow, m_rgRowsetURLs[ulIndex]))

		SAFE_RELEASE(pICreateRow);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pICreateRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown* pIUnkInner = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		CAggregate Aggregate(m_pIBindResource);

		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IUnknown, &dwBindStatus, (IUnknown**)
			&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
			&Aggregate), S_OK, DB_E_NOAGGREGATION)

		Aggregate.SetUnkInner(pIUnkInner);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRow));

		SAFE_RELEASE(pIUnkInner);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	ULONG				ulRef = 0;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IID					iid = IID_IUnknown;
	IRow*				pIRow = NULL;
	IGetSession*		pIGS = NULL;
	IUnknown*			pIAgg = NULL;

	DBIMPLICITSESSION	dbImplSess;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		CAggregate			Aggregate(m_pIBindResource);

		dbImplSess.pUnkOuter = &Aggregate;
		dbImplSess.piid = &iid;
		dbImplSess.pSession = NULL;
		ulRef = Aggregate.GetRefCount();

		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**)&pIRow, 
			DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

		if(hr != S_OK)
			goto CLEANUP;

		if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
		{
			TESTC(dbImplSess.pSession != NULL)
			Aggregate.SetUnkInner(dbImplSess.pSession);

			//Verify Aggregation.
			TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISessionProperties));

			TESTC(VerifyInterface(pIRow, IID_IGetSession,
				ROW_INTERFACE,(IUnknown**)&pIGS))

			//From the ROW, get session and verify aggregation.
			TESTC_(pIGS->GetSession(IID_IAggregate, (IUnknown**)&pIAgg), S_OK)

			//pIAgg is outer unknown whereas dbImplSess.pSession is
			//inner unknown. Verify they are different.
			TESTC(!VerifyEqualInterface(pIAgg, dbImplSess.pSession))
		}
		else  //If Bind is implemented on a Session ...
		{
			//Verify that no aggregation took place.
			TESTC_(hr, S_OK)
			TESTC(!dbImplSess.pSession)
			TESTC(Aggregate.GetRefCount() == ulRef)
		}

CLEANUP:
		SAFE_RELEASE(pIAgg);
		SAFE_RELEASE(pIGS);
		SAFE_RELEASE(pIRow);
		SAFE_RELEASE(dbImplSess.pSession);
		if(TESTB != TEST_PASS)
			break;
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	DBID				dbid = DBROWCOL_DEFAULTSTREAM;
	IRow*				pIRow = NULL;
	IDBInitialize*		pIDBI = NULL;
	ISequentialStream*	pISS = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TEST3C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IDBInitialize, &dwBindStatus, (IUnknown**)
			&pIDBI, DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), 
			S_OK, E_NOINTERFACE, E_INVALIDARG)

		TESTC_PROVIDER(hr == S_OK)

		//Before initialization of row object, cannot get the
		//IRow interface.
		TESTC_(pIDBI->QueryInterface(IID_IRow, (void**)&pIRow),
			E_NOINTERFACE)
		TESTC(!pIRow)

		//Initialize the row object.
		TESTC_(pIDBI->Initialize(), S_OK)

		TESTC(DefaultObjectTesting(pIDBI, ROW_INTERFACE))

		TESTC(VerifyInterface(pIDBI, IID_IRow,
			ROW_INTERFACE,(IUnknown**)&pIRow))

		//Verify that after initialization the Row object is usable.
		TEST2C_(hr=pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, 
			IID_ISequentialStream, (IUnknown**)&pISS), S_OK, 
			DB_E_BADCOLUMNID)
		if(hr == S_OK)
			TESTC(pISS != NULL)

		SAFE_RELEASE(pISS);
		SAFE_RELEASE(pIRow);
		SAFE_RELEASE(pIDBI);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow = NULL;
	IRowChange*			pIRC = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow, 
			DBBINDURLFLAG_READWRITE), S_OK, DB_E_READONLY)

		if(hr == DB_E_READONLY)
			continue;

		TESTC(testIRow(pIRow))

		TESTC(VerifyInterface(pIRow, IID_IRowChange, ROW_INTERFACE,
			(IUnknown**)&pIRC))

		SAFE_RELEASE(pIRC);
		SAFE_RELEASE(pIRow);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRC);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_READ
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_11()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		//Apply read lock.
		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow1, 
			DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_READ), 
			S_OK, DB_S_ERRORSOCCURRED)

		if(hr == DB_S_ERRORSOCCURRED)
		{
			//Verify resource was not locked.
			TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
				IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
				DBBINDURLFLAG_READ), S_OK)
			SAFE_RELEASE(pIRow2);
			break;
		}

		//Verify that another read lock cannot be applied to
		//same resource.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_READ), 
			DB_E_RESOURCELOCKED)

		//Verify resource is read locked.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

		SAFE_RELEASE(pIRow1);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_12()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		//Apply write lock.
		TEST3C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow1, 
			DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
			S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

		if(hr == DB_S_ERRORSOCCURRED)
		{
			//Verify resource was not locked.
			TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
				IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
				DBBINDURLFLAG_WRITE), S_OK)
			SAFE_RELEASE(pIRow2);
			break;
		}

		//Verify that another write lock cannot be applied to
		//same resource.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
			DB_E_RESOURCELOCKED)

		//Verify resource is write locked.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE), 
			DB_E_RESOURCELOCKED)

		SAFE_RELEASE(pIRow1);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_13()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		//Apply a read-write lock.
		TEST3C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow1, 
			DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
			S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

		if(hr == DB_S_ERRORSOCCURRED)
		{
			//Verify resource was not locked.
			TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
				IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
				DBBINDURLFLAG_READ), S_OK)
			SAFE_RELEASE(pIRow2);

			TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
				IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
				DBBINDURLFLAG_WRITE), S_OK)
			SAFE_RELEASE(pIRow2);

			break;
		}

		//Verify that another read-write lock cannot be applied to
		//same resource.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
			DB_E_RESOURCELOCKED)

		//Verify resource is write locked.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

		//Verify resource is read locked.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

		SAFE_RELEASE(pIRow1);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_NONE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_14()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		//Apply a "No Lock".
		TEST3C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow1, 
			DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_DENY_NONE), 
			S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

		TESTC_PROVIDER(hr == S_OK)

		//Verify resource is not locked by calling bind with 
		//read-write, read and write flags.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READWRITE), S_OK)
		SAFE_RELEASE(pIRow2);

		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READ), S_OK)
		SAFE_RELEASE(pIRow2);

		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRow2);

		SAFE_RELEASE(pIRow1);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_15()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TEST3C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow1, 
			DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
			S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

		if(hr == DB_S_ERRORSOCCURRED)
		{
			//Verify resource was not locked.
			TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
				IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
				DBBINDURLFLAG_WRITE), S_OK)
			SAFE_RELEASE(pIRow2);
			break;
		}

		//Verify that another write lock cannot be applied to
		//same resource.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
			DB_E_RESOURCELOCKED)

		//Verify resource is locked.
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READ), S_OK)

		SAFE_RELEASE(pIRow1);
		SAFE_RELEASE(pIRow2);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Flag - OUTPUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_16()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes=50000;	//Read 50kb.
	ULONG				cBytes1=0, cBytes2=0;
	void				*pBuffer1=NULL, *pBuffer2=NULL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	DBID				dbid = DBROWCOL_DEFAULTSTREAM;
	IRow*				pIRow = NULL;
	ISequentialStream*	pISS = NULL;

	//Bind without OUTPUT flag.
	TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, IID_IRow, 
		&dwBindStatus, (IUnknown**) &pIRow), S_OK)

	TESTC(testIRow(pIRow))

	TEST2C_(hr = pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, IID_ISequentialStream, 
		(IUnknown**)&pISS), S_OK, DB_E_BADCOLUMNID)

	//Open could have failed if it does not recognize DBROWCOL_DEFAULTSTREAM.
	//In this case skip test.
	TESTC_PROVIDER(hr == S_OK)

	SAFE_RELEASE(pIRow);

	SAFE_ALLOC(pBuffer1, BYTE, cBytes);

	//Read the stream bound without OUTPUT flag.
	TEST2C_(hr = StorageRead(IID_ISequentialStream, pISS, pBuffer1, cBytes, 
		&cBytes1), S_OK, S_FALSE)
	TESTC(cBytes1 <= cBytes)

	SAFE_RELEASE(pISS);

	//Bind with OUTPUT flag.
	TEST2C_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_OUTPUT), S_OK, E_INVALIDARG)

	if(E_INVALIDARG == hr)
	{
		odtLog<<L"INFO: Flag OUTPUT is not supported.\n";
		goto CLEANUP;
	}

	TESTC(testIRow(pIRow))

	TESTC_(pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, IID_ISequentialStream, 
		(IUnknown**)&pISS), S_OK)

	SAFE_ALLOC(pBuffer2, BYTE, cBytes);

	//Read the stream bound with OUTPUT flag.
	TEST2C_(hr = StorageRead(IID_ISequentialStream, pISS, pBuffer2, cBytes, &cBytes2), S_OK, S_FALSE)
	TESTC(cBytes2 <= cBytes)

	TESTC(cBytes1!=0 && cBytes2!=0)
	cBytes = min(cBytes1, cBytes2);

	odtLog<<L"INFO: The number of bytes with OUTPUT flag is "<<cBytes2<<L" and without it is "<<cBytes1<<L".\n";

	//Compare the streams obtained with and without the OUTPUT
	//flag. For URLs which have an executed output, this flag
	//will make a difference in the streams obtained, otherwise
	//they will be same.
	if(!memcmp(pBuffer1, pBuffer2, cBytes))
	{
		odtLog<<L"INFO: This should be a FAILURE if "<<m_rgURLs[ROW]<<L" represents a resource which can have a source as well as an executed output (e.g. ASP files).\n";
		COMPAREW(TRUE, FALSE);
	}

CLEANUP:
	SAFE_FREE(pBuffer1);
	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_17()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IDBAsynchStatus*	pAsynchStatus = NULL;
	IRow*				pIRow = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_ASYNCHRONOUS), S_OK, 
		DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)
		
	if(S_OK == hr)
	{
		odtLog<<L"WARNING: The Bind call with ASYNCH returned immediately !!\n";
		TESTC(VerifyInterface(pAsynchStatus, IID_IRow,
			ROW_INTERFACE,(IUnknown**)&pIRow))
		TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE, IID_IRow))
		goto CLEANUP;
	}
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: ASYNCH binding is not supported.\n";
		goto CLEANUP;
	}

	//Allow the asynch operation to complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))
	TESTC(VerifyInterface(pAsynchStatus, IID_IRow,
		ROW_INTERFACE,(IUnknown**)&pIRow))
	TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE, IID_IRow))

	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pAsynchStatus);

	//Bind aynchronously again and try to abort.

	TEST3C_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_ASYNCHRONOUS), S_OK, 
		DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)

	if(hr == DB_S_ASYNCHRONOUS)
	{
		TESTC(pAsynchStatus != NULL)
		TESTC_(pAsynchStatus->Abort(NULL, DBASYNCHOP_OPEN), S_OK)
	}

CLEANUP:
	SAFE_RELEASE(pAsynchStatus);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Flag - DELAYFETCHSTREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_18()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes=50000;	//Read 50kb.
	ULONG				cBytes1=0;
	void				*pBuffer1 = NULL;
	DBID				dbid = DBROWCOL_DEFAULTSTREAM;
	IRow*				pIRow = NULL;
	ISequentialStream*	pISS = NULL;

	//This flag (DELAYFETCHSTREAM) is only a hint. Providers 
	//may or may not use it.
	TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, NULL, (IUnknown**) &pIRow, DBBINDURLFLAG_READ | 
		DBBINDURLFLAG_DELAYFETCHSTREAM), S_OK)

	TEST2C_(hr = pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, IID_ISequentialStream, 
		(IUnknown**)&pISS), S_OK, DB_E_BADCOLUMNID)

	//Open could have failed if it does not recognize DBROWCOL_DEFAULTSTREAM.
	//In this case skip test.
	TESTC_PROVIDER(hr == S_OK)

	SAFE_RELEASE(pIRow);

	SAFE_ALLOC(pBuffer1, BYTE, cBytes);

	//Read the stream.
	TEST2C_(hr = StorageRead(IID_ISequentialStream, pISS, pBuffer1, cBytes, 
		&cBytes1), S_OK, S_FALSE)
	TESTC(cBytes1 <= cBytes)

CLEANUP:
	SAFE_FREE(pBuffer1);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Flag - DELAYFETCHCOLUMNS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_19()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IRow*				pIRow = NULL;

	//This flag (DELAYFETCHCOLUMNS) is only a hint. Providers 
	//may or may not use it.
	TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, NULL, (IUnknown**) &pIRow, DBBINDURLFLAG_READ | 
		DBBINDURLFLAG_DELAYFETCHCOLUMNS), S_OK)

	TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE, IID_IRow))

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindRow::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBindRowset)
//*---------------------------------------------------------------------
//| Test Case:		TCBindRowset - Test ROWSET objects (IBindResource)
//| Created:  	8/5/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindRowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc General - Get IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_1()
{ 
	TBEGIN
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IAccessor, NULL, (IUnknown**)
		&pIAccessor), S_OK)

	TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))			

	//Create an accessor and test rowset data.
	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_2()
{ 
	TBEGIN 
	IColumnsInfo*		pIColumnsInfo = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IColumnsInfo, NULL, (IUnknown**)
		&pIColumnsInfo), S_OK)

	//Test obtained interface.
	TESTC(DefTestInterface(pIColumnsInfo))

CLEANUP:
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_3()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IConvertType*		pIConvertType = NULL;

	//The OUTPUT flag is used here. This is to verify that it will be 
	//ignored since we are binding to a collection.
	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, &dwBindStatus, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_READ|DBBINDURLFLAG_OUTPUT), S_OK)

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc General - Get IRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_4()
{ 
	TBEGIN
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, NULL, (IUnknown**)
		&pIRowset), S_OK)

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	//Create an accessor and test rowset data.
	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc General - Get IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_5()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowsetInfo*		pIRowsetInfo = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, (IUnknown**)
		&pIRowsetInfo), S_OK)

	TESTC(DefTestInterface(pIRowsetInfo))

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc General - Get IRowsetIdentity
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_6()
{ 
	TBEGIN
	IAccessor*			pIAcc = NULL;
	IRowset*			pIRowset = NULL;
	IRowsetIdentity*	pIRowsetIdentity = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetIdentity, NULL, (IUnknown**)
		&pIRowsetIdentity), S_OK)

	TEST2C_(pIRowsetIdentity->IsSameRow(1,2), S_FALSE, DB_E_BADROWHANDLE)

	//QI for IRowset and IAccessor.
	TESTC(VerifyInterface(pIRowsetIdentity, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAcc))
	TESTC(VerifyInterface(pIRowsetIdentity, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))

	//Create an accessor and test rowset data.
	TESTC(testRowset(pIAcc, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAcc);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetIdentity);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Bind to URLs of rows of the Rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;

	//Check to see if IGetRow is supported. Get URLs of all rows
	//of rowset pointed to by the URL m_rgURLs[ROWSET].
	if(COMPARE(InitializeRowsetURLs(), TRUE))
	{
		if(!m_rgRowsetURLs)
			return TEST_SKIPPED;
	}
	else
		return TEST_FAIL;

	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		//Bind to all URLs as Rowsets. Some may fail because
		//of not being a collection type.
		TEST2C_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**)
			&pIRowset), S_OK, DB_E_NOTCOLLECTION)

		if(hr==S_OK)
		{
			TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
				ROWSET_INTERFACE, (IUnknown**)&pIAccessor))	
			TESTC(testRowset(pIAccessor, pIRowset, FALSE))
		}
		else
			odtLog<<L"INFO: URL - "<<m_rgRowsetURLs[ulIndex]<<L" is not a COLLECTION.\n";

		SAFE_RELEASE(pIAccessor);
		SAFE_RELEASE(pIRowset);
	}

CLEANUP:
	OUTPUT_FAILEDROWURL(ulIndex)
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*---------------------------------------------------------------------
// @mfunc General - Optional interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_8()
{ 
	TBEGIN

	//Call Bind with all interfaces listed as ROWSET_INTERFACE.
	QTESTC(testAllIntfBind(ROWSET_INTERFACE))

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*---------------------------------------------------------------------
// @mfunc General - IGetRow (if ROW objects are supported)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG_PTR			ulPropVal = 0;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IGetRow*			pIGetRow = NULL;

	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IGetRow, &dwBindStatus, (IUnknown**)
		&pIGetRow), S_OK, E_NOINTERFACE)

	if(hr == E_NOINTERFACE)
	{
		if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
			pIDBInitialize(), &ulPropVal))
			TESTC((ulPropVal & DBPROPVAL_OO_ROWOBJECT) == 0)
		odtLog<<L"INFO: IGetRow is not supported.\n";
	}
	else
	{
		TESTC(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
			pIDBInitialize(), &ulPropVal))
		TESTC((ulPropVal & DBPROPVAL_OO_ROWOBJECT) == DBPROPVAL_OO_ROWOBJECT)
		TESTC(testIGetRow(pIGetRow))
	}

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate Rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pIBindResource);

	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, &Aggregate), 
		S_OK, DB_E_NOAGGREGATION)

	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_11()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulRef = 0;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IID					iid = IID_IUnknown;
	IRowsetInfo*		pIRI = NULL;
	IUnknown*			pIAgg = NULL;

	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;
	ulRef = Aggregate.GetRefCount();

	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, (IUnknown**)&pIRI, 
		DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IGetDataSource));

		//From the rowset, get session and verify aggregation.
		TESTC_(pIRI->GetSpecification(IID_IAggregate, (IUnknown**)&pIAgg), S_OK)

		//pIAgg is outer unknown whereas dbImplSess.pSession is
		//inner unknown. Verify they are different.
		TESTC(!VerifyEqualInterface(pIAgg, dbImplSess.pSession))
	}
	else
	{
		//Verify that no aggregation took place.
		TESTC_(hr, S_OK)
		TESTC(!dbImplSess.pSession)
		TESTC(Aggregate.GetRefCount() == ulRef)
	}

CLEANUP:
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pIAgg);
	SAFE_RELEASE(pIRI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_12()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset = NULL;
	IDBInitialize*		pIDBI = NULL;
	IAccessor*			pIAcc = NULL;

	//Call Bind with WAITFORINIT flag.
	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IDBInitialize, &dwBindStatus, (IUnknown**)&pIDBI, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), S_OK, 
		E_NOINTERFACE, E_INVALIDARG)

	TESTC_PROVIDER(hr == S_OK)

	//Make sure the rowset is uninitialized.
	TESTC_(pIDBI->QueryInterface(IID_IRowset, (void**)&pIRowset),
		E_NOINTERFACE)
	TESTC(!pIRowset)

	//Initialize the rowset.
	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, ROWSET_INTERFACE))

	//QI for IRowset and IAccessor.
	TESTC(VerifyInterface(pIDBI, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAcc))
	TESTC(VerifyInterface(pIDBI, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))

	//Create an accessor and test rowset data.
	TESTC(testRowset(pIAcc, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAcc);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_13()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset,
		DBBINDURLFLAG_READWRITE), S_OK, DB_E_READONLY)

	TESTC_PROVIDER(S_OK == hr)

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_READ
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_14()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;

	//Bid with read lock.
	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_READ), S_OK,
		DB_S_ERRORSOCCURRED)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//Verify resource was not locked.
		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**) &pIRowset2, 
			DBBINDURLFLAG_READ), S_OK)
		SAFE_RELEASE(pIRowset2);

		goto CLEANUP;
	}

	//Verify resource is locked.

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_READ), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_15()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;

	//Call bind with write lock.
	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//Verify resource was not locked.
		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**) &pIRowset2, 
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRowset2);

		goto CLEANUP;
	}

	//Verify resource is locked.
	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_16()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//Verify resource was not locked.
		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**) &pIRowset2, 
			DBBINDURLFLAG_READ), S_OK)
		SAFE_RELEASE(pIRowset2);

		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**) &pIRowset2, 
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRowset2);

		goto CLEANUP;
	}

	//Verify resouce is locked.
	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_NONE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_17()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;
	IAccessor*			pIAccessor = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_NONE), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY)

	TESTC_PROVIDER(S_OK == hr)

	//Verify the resource can be bound with read permission.
	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READ), S_OK)

	TESTC(VerifyInterface(pIRowset2, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset2))

CLEANUP:
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIAccessor);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_18()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			ulIndex;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;
	IRow*				pIRow = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_SHARE_DENY_WRITE), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//Verify resource was not locked.
		TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**) &pIRowset2, 
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRowset2);

		goto CLEANUP;
	}

	//Verify that the resource is locked.
	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	if(!COMPARE(InitializeRowsetURLs(), TRUE) || !m_rgRowsetURLs)
		goto CLEANUP;

	//Verify that the children of this resource are also locked.
	for(ulIndex=0; ulIndex<m_cRowsetURLs; ulIndex++)
	{
		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**)&pIRow, 
			DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
			DB_E_RESOURCELOCKED)

		TESTC_(hr = BindResource(m_rgRowsetURLs[ulIndex], DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**)&pIRow, 
			DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)
	}

CLEANUP:
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_19()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IDBAsynchStatus*	pAsynchStatus = NULL;
	IRowset*			pIRowset = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_ASYNCHRONOUS), S_OK, 
		DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)
		
	if(S_OK == hr)
	{
		//The asych operation completed immediately.
		odtLog<<L"WARNING: The Bind call with ASYNCH returned immediately !!\n";
		TESTC(VerifyInterface(pAsynchStatus, IID_IRowset, 
			ROWSET_INTERFACE, (IUnknown**)&pIRowset))
		TESTC(DefaultInterfaceTesting(pIRowset, ROWSET_INTERFACE, IID_IRowset))
		goto CLEANUP;
	}
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		//ASYNCH is not supported.
		odtLog<<L"INFO: ASYNCH binding is not supported.\n";
		goto CLEANUP;
	}

	//The operation is going on asynchronuosly. Get it to
	//complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))

	TESTC(VerifyInterface(pAsynchStatus, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))
	TESTC(DefaultInterfaceTesting(pIRowset, ROWSET_INTERFACE, IID_IRowset))

CLEANUP:
	SAFE_RELEASE(pAsynchStatus);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindRowset::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBindStream)
//*---------------------------------------------------------------------
//| Test Case:		TCBindStream - Test STREAM objects (IBindResource)
//| Created:  	8/5/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindStream::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc General - Get IGetSourceRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IGetSourceRow*		pIGetSourceRow = NULL;

	TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IGetSourceRow, &dwBindStatus, (IUnknown**)
		&pIGetSourceRow), S_OK, E_NOINTERFACE)

	if(hr==S_OK)
		TESTC(testIGetSourceRow(pIGetSourceRow))

	//Call Bind with all interfaces listed as STREAM_INTERFACE.
	QTESTC(testAllIntfBind(STREAM_INTERFACE))

CLEANUP:
	SAFE_RELEASE(pIGetSourceRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc General - Get ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes = 199;
	ULONG				cBytes2 = 101;
	ULONG				cLen = 0;
	DBID				dbid = DBROWCOL_DEFAULTSTREAM;
	void*				pBuffer = NULL;
	void*				pBuffer2 = NULL;
	IRow*				pIRow = NULL;
	ISequentialStream*	pISS = NULL;
	ISequentialStream*	pISS2 = NULL;

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, (IUnknown**)
		&pISS), S_OK)

	SAFE_ALLOC(pBuffer, BYTE, cBytes)

	while(cBytes == 199)
	{
		TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
			&cBytes), S_OK)
		TESTC(cBytes>0 && cBytes<=199)
		cLen += cBytes;

		//Bind to same stream again, having read the first one 
		//partially.
		TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_ISequentialStream, NULL, (IUnknown**)
			&pISS2), S_OK)

		TESTC_(GetStorageData(IID_ISequentialStream, pISS2, pBuffer, 
			&cBytes2), S_OK)
		TESTC(cBytes2>0 && cBytes2<=101)
	}

	odtLog<<L"INFO: Length of the stream "<<m_rgURLs[STREAM]<<L" is "<<cLen<<L".\n";

	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS2);
	SAFE_FREE(pBuffer);

	//Read entire direct bound stream. Then bind to the row obj
	//and get it's default stream. Compare the data in the 2 
	//streams.

	SAFE_ALLOC(pBuffer, BYTE, cLen)
	SAFE_ALLOC(pBuffer2, BYTE, cLen)
	cBytes = cLen;
	cBytes2 = cLen;

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, (IUnknown**)
		&pISS), S_OK)

	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == cLen)

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_ROW, 
		IID_IRow, NULL, (IUnknown**)&pIRow), S_OK)

	TEST2C_(hr = pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, IID_ISequentialStream, (IUnknown**)&pISS2), S_OK, DB_E_BADCOLUMNID)

	TESTC_PROVIDER(hr == S_OK)

	TESTC_(GetStorageData(IID_ISequentialStream, pISS2, pBuffer2, 
		&cBytes2), S_OK)
	TESTC(cBytes2 == cLen)

	TESTC(memcmp(pBuffer, pBuffer2, (size_t)cLen) == 0)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_3()
{ 
	TBEGIN
	HRESULT				hr;
	LARGE_INTEGER		ulMove = { 0 };
	ULONG				cBytes = 399;
	void*				pBuffer = NULL;
	void*				pBuffer2 = NULL;
	IStream*			pIStream = NULL;

	//Bind to a stream asking for IStream.
	TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IStream, NULL, (IUnknown**)&pIStream), S_OK, E_NOINTERFACE)

	TESTC_PROVIDER(hr == S_OK)

	SAFE_ALLOC(pBuffer, BYTE, cBytes)

	//Read the stream.
	TESTC_(GetStorageData(IID_IStream, pIStream, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes>0 && cBytes<=399)

	//Reset the stream seek pointer to beginning of stream.
	TESTC_(pIStream->Seek(ulMove, STREAM_SEEK_SET, NULL), S_OK)

	SAFE_ALLOC(pBuffer2, BYTE, cBytes)

	//Read data again. Compare data to make sure it has started 
	//reading from beginning of the stream.

	TESTC_(GetStorageData(IID_IStream, pIStream, pBuffer2, 
		&cBytes), S_OK)
	TESTC(cBytes>0 && cBytes<=399)

	TESTC(memcmp(pBuffer, pBuffer2, cBytes) == 0)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pIStream);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc General - Aggregate Stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pIBindResource);

	TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, &Aggregate), 
		S_OK, DB_E_NOAGGREGATION)

	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISequentialStream));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Implicit Session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulRef = 0;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IID					iid = IID_IUnknown;
	ISequentialStream*	pISS = NULL;

	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;
	ulRef = Aggregate.GetRefCount();

	TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS, 
		DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IGetDataSource));
	}
	else
	{
		//Verify that no aggregation took place.
		TESTC_(hr, S_OK)
		TESTC(!dbImplSess.pSession)
		TESTC(Aggregate.GetRefCount() == ulRef)
	}

CLEANUP:
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISequentialStream*	pISS = NULL;
	IDBInitialize*		pIDBI = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IDBInitialize, &dwBindStatus, (IUnknown**)&pIDBI, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), S_OK, 
		E_NOINTERFACE, E_INVALIDARG)

	TESTC_PROVIDER(hr == S_OK)

	//Verify stream object is not initialized.
	TESTC_(pIDBI->QueryInterface(IID_ISequentialStream, (void**)&pISS),
		E_NOINTERFACE)
	TESTC(!pISS)

	//Initialize the stream object.
	TESTC_(pIDBI->Initialize(), S_OK)

	//Verify stream object is initialized.
	TESTC(DefaultObjectTesting(pIDBI, STREAM_INTERFACE))

CLEANUP:
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_7()
{ 
	TBEGIN
	LARGE_INTEGER		ulMove = { 0 };
	HRESULT				hr = E_FAIL;
	ULONG				cBytes = 51;
	ULONG				cBytesWrote = 0;
	void*				pBuffer = NULL;
	void*				pBuffer2 = NULL;
	void*				pBuffer3 = NULL;
	IStream*			pISS = NULL;

	TEST3C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IStream, NULL, (IUnknown**)&pISS, DBBINDURLFLAG_READWRITE), 
		S_OK, DB_E_READONLY, E_NOINTERFACE)

	TESTC_PROVIDER(hr != E_NOINTERFACE)

	if(hr == DB_E_READONLY)
	{
		SAFE_ALLOC(pBuffer, BYTE, cBytes);
		TESTC_(GetStorageData(IID_IStream, pISS, pBuffer, 
			&cBytes), S_OK)
		TESTC(cBytes>0 && cBytes<=51)

		//If stream was read-only, then write should fail.
		TESTC(FAILED(StorageWrite(IID_IStream, pISS, pBuffer, cBytes, 
			&cBytesWrote)))

		goto CLEANUP;
	}

	//Get stream data and put it in pBuffer.
	SAFE_ALLOC(pBuffer, BYTE, cBytes);
	TESTC_(GetStorageData(IID_IStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes>0 && cBytes<=51)

	//Reset the stream seek pointer to beginning of stream.
	TESTC_(pISS->Seek(ulMove, STREAM_SEEK_SET, NULL), S_OK)

	//make another buffer with data "11111..."
	SAFE_ALLOC(pBuffer2, BYTE, cBytes);
	memset(pBuffer2, 1, cBytes);

	//write "11111..." into the stream.
	TESTC_(StorageWrite(IID_IStream, pISS, pBuffer2, cBytes, 
		&cBytesWrote), S_OK)
	TESTC(cBytesWrote == cBytes)

	//Reset the stream seek pointer to beginning of stream.
	TESTC_(pISS->Seek(ulMove, STREAM_SEEK_SET, NULL), S_OK)

	//read new data.
	SAFE_ALLOC(pBuffer3, BYTE, cBytes);
	TESTC_(GetStorageData(IID_IStream, pISS, pBuffer3, 
		&cBytes), S_OK)
	TESTC(cBytes>0 && cBytes<=51)

	//Make sure the data read is the new data.
	TESTC(memcmp(pBuffer3, pBuffer2, cBytes) == 0)

CLEANUP:
	//Reset original data.
	CHECK(StorageWrite(IID_IStream, pISS, pBuffer, cBytes, 
		&cBytesWrote), S_OK);
	SAFE_FREE(pBuffer);
	SAFE_FREE(pBuffer2);
	SAFE_FREE(pBuffer3);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISequentialStream*	pISS = NULL;
	ISequentialStream*	pISS2 = NULL;

	//call bind with SHARE_EXCLUSIVE.
	TEST3C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
		S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//another bind with READWRITE should succeed.
		TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_ISequentialStream, NULL, (IUnknown**)&pISS2, 
			DBBINDURLFLAG_READWRITE), S_OK)
		SAFE_RELEASE(pISS2);

		goto CLEANUP;
	}

	TESTC_PROVIDER(hr == S_OK)

	//Verify the resource is locked.

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISequentialStream*	pISS = NULL;
	ISequentialStream*	pISS2 = NULL;

	//call bind with RECURSIVE & SHARE_DENY_WRITE
	TEST3C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS, 
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE|
		DBBINDURLFLAG_RECURSIVE), S_OK, DB_S_ERRORSOCCURRED, DB_E_READONLY)

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//verify resource is not locked.
		TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
			IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
			DBBINDURLFLAG_WRITE), 
			S_OK)
		SAFE_RELEASE(pISS2);

		goto CLEANUP;
	}

	TESTC_PROVIDER(hr == S_OK)

	//verify resource is locked.

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)&pISS2, 
		DBBINDURLFLAG_READ), S_OK)

CLEANUP:
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Flag - OUTPUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes=101;
	ULONG				cLen1=0, cLen2=0;
	ULONG				cBytes1=0, cBytes2=0;
	void				*pBuffer1=NULL, *pBuffer2=NULL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISequentialStream*	pISS = NULL;
	ISequentialStream*	pISS2 = NULL;

	//Read the stream bound without OUTPUT flag.

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, IID_ISequentialStream, 
		&dwBindStatus, (IUnknown**) &pISS), S_OK)

	SAFE_ALLOC(pBuffer1, BYTE, cBytes);

	//read buffer in chunks of 101 bytes to determine entire length.
	while(cBytes == 101)
	{
		TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer1, 
			&cBytes), S_OK)
		TESTC(cBytes>0 && cBytes<=101)
		cLen1 += cBytes;
	}

	SAFE_FREE(pBuffer1);
	SAFE_RELEASE(pISS);
	cBytes = cLen1;

	//Get the stream again. Read entire stream into pBuffer1.

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, IID_ISequentialStream, 
		&dwBindStatus, (IUnknown**) &pISS), S_OK)

	SAFE_ALLOC(pBuffer1, BYTE, cLen1);

	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer1, 
		&cBytes), S_OK)
	TESTC(cBytes == cLen1)

	//Read the stream bound with OUTPUT flag.

	cBytes = 101;

	TEST2C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**) &pISS2, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_OUTPUT), S_OK, E_INVALIDARG)

	if(E_INVALIDARG == hr)
	{
		odtLog<<L"INFO: Flag OUTPUT is not supported.\n";
		goto CLEANUP;
	}

	SAFE_ALLOC(pBuffer2, BYTE, cBytes);

	//read buffer in chunks of 101 bytes to determine entire length.
	while(cBytes == 101)
	{
		TESTC_(GetStorageData(IID_ISequentialStream, pISS2, pBuffer2, 
			&cBytes), S_OK)
		TESTC(cBytes>0 && cBytes<=101)
		cLen2 += cBytes;
	}

	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pISS2);
	cBytes = cLen2;

	//Bind again with OUTPUT flag. Read entire stream and store
	//it in pBuffer2.

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**) &pISS2, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_OUTPUT), S_OK)

	SAFE_ALLOC(pBuffer2, BYTE, cLen2);

	TESTC_(GetStorageData(IID_ISequentialStream, pISS2, pBuffer2, 
		&cBytes), S_OK)
	TESTC(cBytes == cLen2)

	//The 2 streams could be of different lengths. So we will
	//compare the streams for a length of min of the two.

	TESTC(cLen1!=0 && cLen2!=0)
	cBytes = min(cLen1, cLen2);

	if(!memcmp(pBuffer1, pBuffer2, cBytes))
	{
		odtLog<<L"INFO: Length of stream bound without OUTPUT flag is "<<cLen1<<L".\n";
		odtLog<<L"INFO: Length of stream bound with OUTPUT flag is "<<cLen2<<L".\n";
		odtLog<<L"WARNING: This should be a FAILURE if "<<m_rgURLs[ROW]<<L" represents a resource which can have a source as well as an executed output (e.g. ASP files).\n";
		COMPAREW(TRUE, FALSE);
	}

CLEANUP:
	SAFE_FREE(pBuffer1);
	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_11()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes = 36000;
	void*				pBuffer = NULL;
	IDBAsynchStatus*	pAsynchStatus = NULL;
	ISequentialStream*	pISS = NULL;

	SAFE_ALLOC(pBuffer, BYTE, cBytes);

	TEST3C_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IDBAsynchStatus, NULL, (IUnknown**) &pAsynchStatus, DBBINDURLFLAG_READ | 
		DBBINDURLFLAG_ASYNCHRONOUS), S_OK, DB_S_ASYNCHRONOUS, 
		DB_E_ASYNCNOTSUPPORTED)
		
	if(S_OK == hr)
	{
		//The asynch operation completed immediately.
		odtLog<<L"WARNING: The Bind call with ASYNCH returned immediately !!\n";

		//Verify the stream object is ready to use.
		TESTC(VerifyInterface(pAsynchStatus, IID_ISequentialStream, 
			STREAM_INTERFACE, (IUnknown**)&pISS))

		TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
			&cBytes), S_OK)
		TESTC(cBytes>0 && cBytes<=36000)

		goto CLEANUP;
	}
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: ASYNCH binding is not supported.\n";
		goto CLEANUP;
	}
	
	//The operation is going on asynchronuosly. Get it to
	//complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))

	TESTC(VerifyInterface(pAsynchStatus, IID_ISequentialStream, 
		STREAM_INTERFACE, (IUnknown**)&pISS))

	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes>0 && cBytes<=36000)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pAsynchStatus);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindStream::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBindCommand)
//*-----------------------------------------------------------------------
//| Test Case:		TCBindCommand - Test COMMAND objects (IBindResource)
//| Created:  	2/1/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindCommand::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		if(InitTC())
		{
			if(!m_pwszCmdURL)
			{
				odtLog<<L"WARNING: There was no URL_COMMAND entry in the INI file.\n";
				return TEST_SKIPPED;
			}

			//check if binding to COMMANDs is supported.
			ICommand*			pICommand = NULL;

			if(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
				IID_ICommand, NULL, (IUnknown**)&pICommand) != S_OK)
			{
				odtLog<<L"INFO: Binding to DBGUID_COMMAND is not supported.\n";
				return TEST_SKIPPED;
			}

			SAFE_RELEASE(pICommand);
			return TRUE;
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get ICommand
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ICommand*			pICommand = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICommand), S_OK)

	TESTC(testICommand(pICommand))

CLEANUP:
	SAFE_RELEASE(pICommand);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get ICommandText
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ICommandText*		pICT = NULL;
	IRowset*			pIRowset = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommandText, NULL, (IUnknown**)
		&pICT), S_OK)

	TESTC(testICommandText(pICT))

	//Execute a command.
	TESTC(m_pTable != NULL)
	TESTC_(hr = m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, NULL,
		(IUnknown**)&pIRowset, (ICommand**)&pICT), S_OK)

	TESTC(DefTestInterface(pIRowset))

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICT);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get ICommandProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	VARIANT_BOOL		bCanHR = VARIANT_FALSE;
	ULONG				cPropSets = 0;
	DBPROPSET*			rgPropSets = NULL;
	ICommandProperties*	pICP = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommandProperties, &dwBindStatus, (IUnknown**)
		&pICP), S_OK)

	//Verify the obtained ICommandProperties interface.
	TESTC_(pICP->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets && rgPropSets && rgPropSets[0].rgProperties)

	//CANHOLDROWS has to be supported. Verify.
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (IUnknown*)pICP, &bCanHR))
	TESTC((bCanHR == VARIANT_FALSE) || (bCanHR == VARIANT_TRUE))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pICP);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IAccessor*			pIAccessor = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IAccessor, &dwBindStatus, (IUnknown**)
		&pIAccessor), S_OK)

	//verify obtained IAccessor interface.
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, NULL, 0, NULL, NULL), E_INVALIDARG)
	TEST2C_(pIAccessor->GetBindings(0, NULL, NULL,NULL), E_INVALIDARG, DB_E_BADACCESSORHANDLE)

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	DBORDINAL			cCols = 0;
	DBCOLUMNINFO*		rgInfo = NULL;
	WCHAR*				pBuff = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;
	ICommandPrepare*	pICP = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), S_OK)

	//The command object returned can be in one of two states :-
	//(1) Initial - no text set, no props, not prepared.
	//(2) Unprepared - text set, but not prepared.

	TEST3C_(hr = pIColumnsInfo->GetColumnInfo(&cCols, &rgInfo, &pBuff), S_OK, DB_E_NOCOMMAND, DB_E_NOTPREPARED)
	if(DB_E_NOCOMMAND == hr)
	{
		odtLog<<L"INFO: The direct bound Command object does not have a command text set.\n";

		//Set the command text.
		TESTC(m_pTable != NULL)
		TESTC_(hr = m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset,
			NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL,
			NULL, (ICommand**)&pIColumnsInfo), S_OK)
	}
	else if(DB_E_NOTPREPARED == hr)
		odtLog<<L"INFO: The direct bound Command object is not prepared.\n";
	else if(S_OK == hr)
	{
		//This means the command does not support preparing. If
		//it did, then the state should have been unprepared.
		TESTC(!VerifyInterface(pIColumnsInfo, IID_ICommandPrepare, 
			COMMAND_INTERFACE, (IUnknown**)&pICP))
		TESTC(!pICP)
	}

	//Prepare the command.
	if(VerifyInterface(pIColumnsInfo, IID_ICommandPrepare, 
		COMMAND_INTERFACE, (IUnknown**)&pICP))
		pICP->Prepare(1);

	//Test the methods of IColumnsInfo.
	TESTC(DefTestInterface(pIColumnsInfo))

CLEANUP:
	SAFE_FREE(rgInfo);
	SAFE_FREE(pBuff);
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IConvertType*		pIConvertType = NULL;

	TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IConvertType, NULL, (IUnknown**)
		&pIConvertType), S_OK)

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_7()
{ 
	TBEGIN

	QTESTC(testAllIntfBind(COMMAND_INTERFACE))

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pIBindResource);

	TEST2C_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), S_OK, DB_E_NOAGGREGATION)

	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ICommand));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Implicit Session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				ulRef = 0;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IID					iid = IID_IUnknown;
	ICommand*			pICommand = NULL;
	IUnknown*			pIAgg = NULL;

	DBIMPLICITSESSION	dbImplSess;

	CAggregate			Aggregate(m_pIBindResource);

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;
	ulRef = Aggregate.GetRefCount();

	TEST2C_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)&pICommand, 
		DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISessionProperties));

		//From the command, get session and verify aggregation.
		TESTC_(pICommand->GetDBSession(IID_IAggregate, (IUnknown**)&pIAgg), S_OK)

		//pIAgg is outer unknown whereas dbImplSess.pSession is
		//inner unknown. Verify they are different.
		TESTC(!VerifyEqualInterface(pIAgg, dbImplSess.pSession))
	}
	else  //If Bind is implemented on a Session ...
	{
		//Verify that no aggregation took place.
		TESTC_(hr, S_OK)
		TESTC(!dbImplSess.pSession)
		TESTC(Aggregate.GetRefCount() == ulRef)
	}

CLEANUP:
	SAFE_RELEASE(pIAgg);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(dbImplSess.pSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindCommand::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ICommand*			pICommand = NULL;
	IDBInitialize*		pIDBI = NULL;
	IUnknown*			pIUnk = NULL;

	TEST2C_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND,
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk, DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), 
		S_OK, E_INVALIDARG)

	TESTC_PROVIDER(hr == S_OK)

	TESTC_(pIUnk->QueryInterface(IID_ICommand, (void**)&pICommand),
		E_NOINTERFACE)
	TESTC(!pICommand)

	TESTC(VerifyInterface(pIUnk, IID_IDBInitialize, 
		COMMAND_INTERFACE, (IUnknown**)&pIDBI))

	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, COMMAND_INTERFACE))

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindCommand::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBind_Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCBind_Boundary - IBindResource boundary cases
//| Created:  	11/16/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBind_Boundary::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : pwszURL=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_1()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnk = NULL;

	TESTC_(BindResource(NULL, DBGUID_DSO, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, E_INVALIDARG));

	TESTC_(BindResource(NULL, DBGUID_SESSION, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

	TESTC_(BindResource(NULL, DBGUID_ROW, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

	TESTC_(BindResource(NULL, DBGUID_ROWSET, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

	TESTC_(BindResource(NULL, DBGUID_STREAM, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

	TESTC_(BindResource(NULL, DBGUID_COMMAND, 
		IID_IUnknown, &dwBindStatus, (IUnknown**)
		&pIUnk), E_INVALIDARG);

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : ppUnk=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_2()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 

	TESTC_(BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IUnknown, &dwBindStatus, NULL), E_INVALIDARG)

	TESTC_(BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_IUnknown, &dwBindStatus, NULL), E_INVALIDARG)

	TESTC_(BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IConvertType, &dwBindStatus, NULL), E_INVALIDARG)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, E_INVALIDARG));

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, &dwBindStatus, NULL), E_INVALIDARG)

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IUnknown, &dwBindStatus, NULL), E_INVALIDARG)

	TESTC_(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IConvertType, &dwBindStatus, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : pImplSess->piid = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset = NULL;

	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = NULL;
	dbImplSess.pSession = NULL;

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset, 
		DBBINDURLFLAG_READ, &dbImplSess), E_INVALIDARG)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, E_INVALIDARG));

	//Also make pwszURL and ppUnk NULL. Should get 
	//E_INVALIDARG again.
	TESTC_(hr = BindResource(NULL, DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, NULL, 
		DBBINDURLFLAG_READ, &dbImplSess), E_INVALIDARG)

CLEANUP:
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : No flags set
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IConvertType*		pIConvertType = NULL;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(m_pIDBBinderProperties != NULL)
		TESTC_(m_pIDBBinderProperties->Reset(), S_OK)
	}

	//Calling Bind with no props and no flags.

	hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, &dwBindStatus, (IUnknown**)
		&pIConvertType, 0);

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		if(FAILED(hr))
			odtLog<<L"INFO: Call to Bind failed with "<<GetErrorName(hr)<<L". If there were some INIT props required, then this failure is OK.\n";
		else
			TESTC(DefTestInterface(pIConvertType))
	}
	else
		TESTC_(hr, E_INVALIDARG)

CLEANUP:
	SetInitProps(m_pIDBBinderProperties);
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : RECURSIVE flag
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_5()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IConvertType*		pIConvertType = NULL;
	IRow*				pIRow = NULL;

	CHECK(BindResource(m_rgURLs[ROWSET], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**)
		&pIRow, DBBINDURLFLAG_RECURSIVE), E_INVALIDARG);

	CHECK(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, NULL, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_READ), 
		E_INVALIDARG);

	CHECK(BindResource(m_rgURLs[ROWSET], DBGUID_ROW, 
		IID_IConvertType, &dwBindStatus, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_READWRITE), 
		E_INVALIDARG);

	CHECK(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, &dwBindStatus, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_SHARE_DENY_NONE), 
		E_INVALIDARG);

	CHECK(BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IConvertType, NULL, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_DELAYFETCHCOLUMNS), 
		E_INVALIDARG);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : READWRITE on DSO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_6()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0;
	IDBProperties*		pIDBProperties = NULL;

	TESTC_(BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IDBProperties, &dwBindStatus, (IUnknown**)
		&pIDBProperties, DBBINDURLFLAG_READWRITE), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : READWRITE on SESSION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_7()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0;
	ISessionProperties*	pISessionProperties = NULL;

	TESTC_(BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_ISessionProperties, &dwBindStatus, (IUnknown**)
		&pISessionProperties, DBBINDURLFLAG_READWRITE), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : WAITFORINIT on SESSION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_8()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0;
	ISessionProperties*	pISessionProperties = NULL;

	TESTC_(BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_ISessionProperties, &dwBindStatus, (IUnknown**)
		&pISessionProperties, DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT), 
		E_INVALIDARG)

	TEST2C_(BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_ISessionProperties, &dwBindStatus, (IUnknown**)
		&pISessionProperties, DBBINDURLFLAG_READ|DBBINDURLFLAG_ASYNCHRONOUS), 
		E_INVALIDARG, DB_E_ASYNCNOTSUPPORTED)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_9()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowsetInfo*		pIRowsetInfo = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, (IUnknown**)
		&pIRowsetInfo, DBBINDURLFLAG_READ|DBBINDURLFLAG_DELAYFETCHCOLUMNS), 
		E_INVALIDARG)

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, (IUnknown**)
		&pIRowsetInfo, DBBINDURLFLAG_READ|DBBINDURLFLAG_DELAYFETCHSTREAM), 
		E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_10()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISequentialStream*	pISS = NULL;

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)
		&pISS, DBBINDURLFLAG_READ|DBBINDURLFLAG_DELAYFETCHSTREAM), E_INVALIDARG)

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISequentialStream, &dwBindStatus, (IUnknown**)
		&pISS, DBBINDURLFLAG_READ|DBBINDURLFLAG_DELAYFETCHCOLUMNS), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : Invalid Flags on COMMAND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_11()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ICommand*			pICmd = NULL;

	TESTC_PROVIDER(m_pwszCmdURL != NULL)

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_WRITE), E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_READ), E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_NONE), E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_SHARE_DENY_READ), 
		E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_RECURSIVE), 
		E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_OUTPUT), E_INVALIDARG);

	CHECK(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_ICommand, &dwBindStatus, (IUnknown**)
		&pICmd, DBBINDURLFLAG_READ|DBBINDURLFLAG_ASYNCHRONOUS), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IRowset on DSO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_12()
{ 
	TBEGIN
	IRowset*	pIRowset = NULL;

	TESTC_(BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IRowset, NULL, (IUnknown**)&pIRowset), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IRowset on SESSION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_13()
{ 
	TBEGIN
	IRowset*	pIRowset = NULL;

	TESTC_(BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_IRowset, NULL, (IUnknown**)&pIRowset), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IDBProperties on ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_14()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IDBProperties*		pIDBP = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IDBProperties, &dwBindStatus, (IUnknown**)
		&pIDBP), E_NOINTERFACE)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, E_NOINTERFACE));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IDBProperties on ROW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_15()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IDBProperties*		pIDBP = NULL;

	TESTC_(BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IDBProperties, &dwBindStatus, (IUnknown**)
		&pIDBP), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_ISessionProperties on STREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_16()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	ISessionProperties*	pISP = NULL;

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_ISessionProperties, &dwBindStatus, (IUnknown**)
		&pISP), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IRowsetInfo on COMMAND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_17()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowsetInfo*		pIRI = NULL;

	TESTC_PROVIDER(m_pwszCmdURL != NULL)

	TESTC_(BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
		IID_IRowsetInfo, &dwBindStatus, (IUnknown**)
		&pIRI), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : riid = IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_18()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pUnk = NULL;

	TESTC_(BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_NULL, &dwBindStatus, (IUnknown**)
		&pUnk), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_19()
{ 
	TBEGIN
	GUID			guidNull = DB_NULLGUID;
	DBBINDURLSTATUS	dwBindStatus = 0; 
	IUnknown*		pIUnk = NULL;

	CHECK(BindResource(m_rgURLs[DSO], guidNull, 
		IID_IDBInitialize, &dwBindStatus, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	CHECK(BindResource(m_rgURLs[ROW], DBGUID_SQL, 
		IID_IColumnsInfo, NULL, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	COMPARE(XCHECK(m_pIBindResource, IID_IBindResource, DB_E_NOTSUPPORTED), TRUE);

	CHECK(BindResource(m_rgURLs[ROWSET], DBGUID_DEFAULT, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	CHECK(BindResource(m_rgURLs[STREAM], DBGUID_DBSQL, 
		IID_IStream, &dwBindStatus, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	CHECK(BindResource(m_rgURLs[ROW], DBGUID_MDX, 
		IID_IDBInfo, &dwBindStatus, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	CHECK(BindResource(m_rgURLs[ROW], DBGUID_CONTAINEROBJECT, 
		IID_IRow, &dwBindStatus, (IUnknown**)
		&pIUnk), DB_E_NOTSUPPORTED);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTFOUND : bogus URL (DSO)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_20()
{ 
	TBEGIN
	WCHAR*			pwszBogusURL = NULL; //Non-existent URL.
	IDBInitialize*	pIDBInitialize = NULL;
	ISessionProperties*	pISP = NULL;

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgURLs[DSO])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgURLs[DSO]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TEST2C_(BindResource(pwszBogusURL, DBGUID_DSO, 
		IID_IDBInitialize, NULL, (IUnknown**)&pIDBInitialize), 
		S_OK, DB_E_NOTFOUND)
	SAFE_FREE(pwszBogusURL);

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgURLs[SESSION])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgURLs[SESSION]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TEST2C_(BindResource(pwszBogusURL, DBGUID_SESSION, 
		IID_ISessionProperties, NULL, (IUnknown**)&pISP), 
		S_OK, DB_E_NOTFOUND)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, DB_E_NOTFOUND));

CLEANUP:
	SAFE_FREE(pwszBogusURL);
	SAFE_RELEASE(pISP);
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTFOUND : bogus URL (ROW)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_21()
{ 
	TBEGIN
	WCHAR*				pwszBogusURL = NULL; //Non-existent URL.
	IRow*				pIRow = NULL;
	IRowset*			pIRowset = NULL;
	ISequentialStream*	pISS = NULL;

	//ROW

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgURLs[ROW])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgURLs[ROW]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TESTC_(BindResource(pwszBogusURL, DBGUID_ROW, 
		IID_IRow, NULL, (IUnknown**)&pIRow), DB_E_NOTFOUND)
	SAFE_FREE(pwszBogusURL);

	//ROWSET

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgURLs[ROWSET])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgURLs[ROWSET]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TESTC_(BindResource(pwszBogusURL, DBGUID_ROWSET, 
		IID_IRowset, NULL, (IUnknown**)&pIRowset), DB_E_NOTFOUND)
	SAFE_FREE(pwszBogusURL);

	//STREAM

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgURLs[STREAM])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgURLs[STREAM]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TESTC_(BindResource(pwszBogusURL, DBGUID_STREAM, 
		IID_ISequentialStream, NULL, (IUnknown**)&pISS), DB_E_NOTFOUND)

CLEANUP:
	SAFE_FREE(pwszBogusURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTFOUND: Wierd URLs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_22()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	HRESULT				hrExp = DB_E_NOTFOUND;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IColumnsInfo*		pIColumnsInfo = NULL;

	//When Bind is implemented on a Session, then some providers
	//interpret URLs like " " to be the URL provided as the
	//DATASOURCE property. Hence, binding to " " would give S_OK.
	if(TC_SESSION == m_eTestCase)
		hrExp = S_OK;

	CHECK(BindResource(L"NonExistentURL", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), DB_E_NOTFOUND);

	CHECKW(BindResource(L"", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);

	CHECKW(BindResource(L" ", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);

	CHECKW(BindResource(L"                ", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);

	CHECKW(BindResource(L":", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
		&pIColumnsInfo), DB_E_NOTFOUND);

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		CHECK(BindResource(L"NonExistentScheme:NonExistentURL", DBGUID_ROWSET, 
			IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
			&pIColumnsInfo), DB_E_NOTFOUND);

		CHECK(BindResource(L"NonExistentScheme://NonExistentURL", DBGUID_ROWSET, 
			IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
			&pIColumnsInfo), DB_E_NOTFOUND);
	}
	else
	{
		hr = BindResource(L"NonExistentScheme:NonExistentURL", DBGUID_ROWSET, 
			IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
			&pIColumnsInfo);
		COMPAREW(hr==DB_E_NOTFOUND || hr==DB_E_CANNOTCONNECT, TRUE);

		hr = BindResource(L"NonExistentScheme://NonExistentURL", DBGUID_ROWSET, 
			IID_IColumnsInfo, &dwBindStatus, (IUnknown**)
			&pIColumnsInfo);
		COMPAREW(hr==DB_E_NOTFOUND || hr==DB_E_CANNOTCONNECT, TRUE);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION : riid not IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_23()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pIBindResource);

	TESTC_(hr = BindResource(m_rgURLs[DSO], DBGUID_DSO, 
		IID_IDBProperties, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), DB_E_NOAGGREGATION)

	TESTC_(hr = BindResource(m_rgURLs[SESSION], DBGUID_SESSION, 
		IID_ISessionProperties, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), DB_E_NOAGGREGATION)

	TESTC_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), DB_E_NOAGGREGATION)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, DB_E_NOAGGREGATION));

	TESTC_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), DB_E_NOAGGREGATION)

	TESTC_(hr = BindResource(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IStream, &dwBindStatus, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
		&Aggregate), DB_E_NOAGGREGATION)

	if(m_pwszCmdURL)
		TESTC_(hr = BindResource(m_pwszCmdURL, DBGUID_COMMAND, 
			IID_ICommand, &dwBindStatus, (IUnknown**)
			&pIUnkInner, DBBINDURLFLAG_READ, NULL, 
			&Aggregate), DB_E_NOAGGREGATION)

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_24()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IID					iid = IID_ISessionProperties;
	IRow*				pIRow = NULL;
	IRowset*			pIRowset = NULL;

	CAggregate			Aggregate(m_pIBindResource);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;

	TEST2C_(hr = BindResource(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
		TESTC_(hr, DB_E_NOAGGREGATION)
	else
	{
		TESTC_(hr, S_OK)
		SAFE_RELEASE(pIRow);
		SAFE_RELEASE(dbImplSess.pSession);
	}

	TEST2C_(hr = BindResource(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset, 
		DBBINDURLFLAG_READ, &dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
		TESTC_(hr, DB_E_NOAGGREGATION)
	else
	{
		TESTC_(hr, S_OK)
		SAFE_RELEASE(pIRowset);
		SAFE_RELEASE(dbImplSess.pSession);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(dbImplSess.pSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTCOLLECTION : bind to stream URL as Rowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_25()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset = NULL;

	TESTC_(BindResource(m_rgURLs[STREAM], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)
		&pIRowset), DB_E_NOTCOLLECTION)

	TESTC(XCHECK(m_pIBindResource, IID_IBindResource, DB_E_NOTCOLLECTION));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc REGDB_E_CLASSNOTREG: Dummy Provider Binder
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBind_Boundary::Variation_26()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	CLSID				clsidDummy = { 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
	WCHAR*				pwszDummy = L"DummyScheme123://DummyURL-Scheme-Specific-Part";
	IRegisterProvider*	pIReg = NULL;
	IRowset*			pIRowset = NULL;

	TESTC(VerifyInterface(GetModInfo()->GetRootBinder(), IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&pIReg))
	TESTC_(pIReg->SetURLMapping(pwszDummy, 0, clsidDummy), S_OK)

	hr = BindResource(pwszDummy, DBGUID_ROWSET, 
		IID_IRowset, NULL, (IUnknown**)&pIRowset);

	if(TC_RBINDER == m_eTestCase)
		TESTC_(hr, REGDB_E_CLASSNOTREG)
	else
		TESTC_(hr, DB_E_NOTFOUND)
	
CLEANUP:
	if(pIReg)
		pIReg->UnregisterProvider(pwszDummy, 0, clsidDummy);
	SAFE_RELEASE(pIReg);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBind_Boundary::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateRowExist)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateRowExist - Test ROW objects (ICreateRow)
//| Created:  	8/25/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateRowExist::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		QTESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

		TESTC(InitializeRowsetURLs())

		//Skip test case if ROW objects are not supported.
		TESTC_PROVIDER(m_rgRowsetURLs)

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRow), 
		S_OK)

	TESTC(testIRow(pIRow))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IColumnsInfo, NULL, &pwszNewURL,(IUnknown**)
		&pIColumnsInfo), S_OK)

	TESTC(DefTestInterface(pIColumnsInfo))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IConvertType*		pIConvertType = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IConvertType, NULL, &pwszNewURL, (IUnknown**)
		&pIConvertType), S_OK)

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IGetSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IGetSession*		pIGetSession = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IGetSession, &dwBindStatus, NULL,(IUnknown**)
		&pIGetSession), S_OK)

	TESTC(testIGetSession(pIGetSession))

CLEANUP:
	SAFE_RELEASE(pIGetSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo2 (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IColumnsInfo2*		pIColumnsInfo2 = NULL;

	TEST2C_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IColumnsInfo2, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo2), S_OK, E_NOINTERFACE)

	if(hr==S_OK)
		TESTC(testIColumnsInfo2(pIColumnsInfo2))

	//Call CreateRow with all interfaces listed as ROW_INTERFACE.
	QTESTC(testAllIntfCR(ROW_INTERFACE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIColumnsInfo2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Get ICreateRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	ICreateRow*			pICreateRow = NULL;

	TEST2C_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_ICreateRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pICreateRow), S_OK, E_NOINTERFACE)

	if(hr == S_OK)
		TESTC(testICreateRow(pICreateRow, m_rgURLs[ROW]))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pICreateRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pICreateRow);

	TEST2C_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IUnknown, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS, 
		NULL, &Aggregate), S_OK, DB_E_NOAGGREGATION)

	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRow));

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowExist::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IID					iid = IID_IUnknown;
	IRow*				pIRow = NULL;

	CAggregate			Aggregate(m_pICreateRow);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;

	TEST2C_(hr = CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISessionProperties));
	}
	else
		TESTC_(hr, S_OK)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(dbImplSess.pSession)
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateRowExist::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateRowsetExist)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateRowsetExist - Test existing rowset objects (ICreateRow)
//| Created:  	11/2/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateRowsetExist::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		TESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowsetExist::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IAccessor, &dwBindStatus, NULL, (IUnknown**)
		&pIAccessor, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowsetExist::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IColumnsInfo, NULL, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(DefTestInterface(pIColumnsInfo))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowsetExist::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IConvertType*		pIConvertType = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IConvertType, NULL, &pwszNewURL, (IUnknown**)
		&pIConvertType, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIConvertType);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowsetExist::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateRowsetExist::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowsetInfo*		pIRowsetInfo = NULL;

	TESTC_(hr = CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRowsetInfo, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK)

	TESTC(DefTestInterface(pIRowsetInfo))

	//Call CreateRow with all interfaces listed as ROWSET_INTERFACE.
	QTESTC(testAllIntfCR(ROWSET_INTERFACE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateRowsetExist::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateNewRow)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateNewRow - Test ROW objects (ICreateRow)
//| Created:  	8/26/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateNewRow::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		TESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

		TESTC(InitializeNewURLs())

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, NULL, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(testIRow(pIRow))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IColumnsInfo*		pICI = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pICI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(DefTestInterface(pICI))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pICI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IConvertType*		pICT = NULL;

	NEW_URL;

	//First call CreateRow without OVERWRITE flag. This could
	//fail for a new URL with DB_E_RESOURCEEXISTS, or the 
	//provider may be smart enough to create one.
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IConvertType, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pICT, DBBINDURLFLAG_READWRITE), S_OK, DB_E_RESOURCEEXISTS)

	if(hr == S_OK)
		TESTC(DefTestInterface(pICT))

	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pICT);

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IConvertType, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pICT, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(DefTestInterface(pICT))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pICT);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IGetSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IGetSession*		pIGS = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IGetSession, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIGS, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(testIGetSession(pIGS))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIGS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IUnknown*			pIUnkInner = NULL;

	NEW_URL;
	CAggregate Aggregate(m_pICreateRow);

	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IUnknown, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE, 
		NULL, &Aggregate), S_OK, DB_E_NOAGGREGATION, DB_E_RESOURCEEXISTS)

	Aggregate.SetUnkInner(pIUnkInner);

	RESOURCE_EXISTS(hr);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRow));

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IID					iid = IID_IUnknown;
	IRow*				pIRow = NULL;

	CAggregate			Aggregate(m_pICreateRow);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISessionProperties));
	}
	else
		TESTC_(hr, S_OK)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;
	IRowChange*			pIRC = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(VerifyInterface(pIRow, IID_IRowChange, ROW_INTERFACE,
		(IUnknown**)&pIRC))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRC);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;
	IDBInitialize*		pIDBI = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IDBInitialize, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIDBI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_WAITFORINIT), S_OK, 
		E_NOINTERFACE, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC_PROVIDER(hr == S_OK)

	TESTC_(pIDBI->QueryInterface(IID_IRow, (void**)&pIRow),
		E_NOINTERFACE)
	TESTC(!pIRow)

	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, ROW_INTERFACE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow1, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_SHARE_DENY_WRITE), S_OK, DB_S_ERRORSOCCURRED, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	SKIPIF_NOBINDONROW;

	if(hr == DB_S_ERRORSOCCURRED)
	{
		//verify resource is not locked.
		TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRow2);

		goto CLEANUP;
	}

	//verify resource is locked.

	TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow1 = NULL;
	IRow*				pIRow2 = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow1, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_SHARE_EXCLUSIVE), S_OK, DB_S_ERRORSOCCURRED, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	SKIPIF_NOBINDONROW;

	if(hr == DB_S_ERRORSOCCURRED)
	{
		TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
			IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
			DBBINDURLFLAG_READWRITE), S_OK)
		SAFE_RELEASE(pIRow2);

		goto CLEANUP;
	}

	TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, (IUnknown**) &pIRow2, 
		DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow1);
	SAFE_RELEASE(pIRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_11()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IDBAsynchStatus*	pAsynchStatus = NULL;
	IRow*				pIRow = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IDBAsynchStatus, NULL, &pwszNewURL, (IUnknown**)
		&pAsynchStatus, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_ASYNCHRONOUS), 
		S_OK, DB_S_ASYNCHRONOUS, DB_E_ASYNCNOTSUPPORTED)
		
	if(S_OK == hr)
	{
		odtLog<<L"WARNING: The CreateRow call with ASYNCH returned immediately !!\n";
		TESTC(VerifyInterface(pAsynchStatus, IID_IRow,
			ROW_INTERFACE,(IUnknown**)&pIRow))
		TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE, IID_IRow))
		goto CLEANUP;
	}
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: Asynchronous CreateRow is not supported.\n";
		goto CLEANUP;
	}

	//Allow asynch operation to complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))

	TESTC(VerifyInterface(pAsynchStatus, IID_IRow,
		ROW_INTERFACE,(IUnknown**)&pIRow))
	TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE, IID_IRow))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pAsynchStatus);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_RESOURCENOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRow::Variation_12()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, NULL, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCENOTSUPPORTED, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	if(hr == S_OK)
		TESTC(testIRow(pIRow))
	else
		odtLog<<L"INFO: CreateRow does not support DBBINDURLFLAG_ISSTRUCTUREDDOCUMENT.\n";

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateNewRow::Terminate()
{ 
	CleanupNewURLs();
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateNewRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateNewRowset - Test Creating Rowsets (ICreateRow)
//| Created:  	11/2/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateNewRowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		TESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

		TESTC(InitializeNewURLs())

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IAccessor, NULL, &pwszNewURL, (IUnknown**)
		&pIAccessor, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))			

	TESTC(testRowset(pIAccessor, pIRowset, FALSE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IColumnsInfo*		pICI = NULL;

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IColumnsInfo, NULL, &pwszNewURL, (IUnknown**)
		&pICI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);			

	TESTC(DefTestInterface(pICI))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pICI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IConvertType*		pICT = NULL;

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IConvertType, NULL, NULL, (IUnknown**)
		&pICT, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);			

	TESTC(DefTestInterface(pICT))

CLEANUP:
	SAFE_RELEASE(pICT);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_4()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset, FALSE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_5()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowsetInfo*		pIRI = NULL;

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowsetInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);			

	TESTC(DefTestInterface(pIRI))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowsetIdentity
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_6()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowsetIdentity*	pIRI = NULL;

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowsetIdentity, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)	

	RESOURCE_EXISTS(hr);

	//Verify the obtained IRowsetIdentity interface.
	//TO DO:

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Get IGetRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_7()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IGetRow*			pIGR = NULL;

	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IGetRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIGR, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, E_NOINTERFACE,
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);
	
	TESTC_PROVIDER(S_OK == hr)

	TESTC(testIGetRow(pIGR))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIGR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_8()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IUnknown*			pIUnkInner = NULL;

	CAggregate Aggregate(m_pICreateRow);

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IUnknown, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION, NULL, &Aggregate), 
		S_OK, DB_E_NOAGGREGATION, DB_E_RESOURCEEXISTS)

	Aggregate.SetUnkInner(pIUnkInner);

	RESOURCE_EXISTS(hr);

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_9()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IID					iid = IID_IUnknown;
	IRowset*			pIRowset = NULL;

	CAggregate			Aggregate(m_pICreateRow);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRowset, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	if(hr != S_OK)
		goto CLEANUP;

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
	{
		TESTC(dbImplSess.pSession != NULL)
		Aggregate.SetUnkInner(dbImplSess.pSession);

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IGetDataSource));
	}
	else
		TESTC_(hr, S_OK)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_10()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRowset,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_READONLY, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC_PROVIDER(S_OK == hr)

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_11()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowset*			pIRowset = NULL;
	IDBInitialize*		pIDBI = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IDBInitialize, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIDBI, 
		DBBINDURLFLAG_READWRITE||DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION|DBBINDURLFLAG_WAITFORINIT), S_OK, 
		E_NOINTERFACE, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	TESTC_PROVIDER(hr == S_OK)

	TESTC_(pIDBI->QueryInterface(IID_IRowset, (void**)&pIRowset),
		E_NOINTERFACE)
	TESTC(!pIRowset)

	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, ROWSET_INTERFACE))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_12()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;

	NEW_URL;
	TEST4C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_DENY_WRITE|
		DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	SKIPIF_NOBINDONROW;

	if(DB_S_ERRORSOCCURRED == hr)
	{
		//verify resource is not locked.
		TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
			DBBINDURLFLAG_WRITE), S_OK)
		SAFE_RELEASE(pIRowset2);
		goto CLEANUP;
	}

	//verify resource is locked.

	TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_SHARE_DENY_WRITE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_13()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;

	NEW_URL;
	TEST4C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRowset1,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE|
		DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION), S_OK,
		DB_S_ERRORSOCCURRED, DB_E_READONLY, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	SKIPIF_NOBINDONROW;

	if(DB_S_ERRORSOCCURRED == hr)
	{
		//verify resource is not locked.
		TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
			IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
			DBBINDURLFLAG_READWRITE), S_OK)
		SAFE_RELEASE(pIRowset2);
		goto CLEANUP;
	}

	//verify resource is locked.

	TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_SHARE_EXCLUSIVE), 
		DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_WRITE), DB_E_RESOURCELOCKED)

	TESTC_(hr = BindResource(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, (IUnknown**)&pIRowset2,
		DBBINDURLFLAG_READ), DB_E_RESOURCELOCKED)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_14()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IDBAsynchStatus*	pAsynchStatus = NULL;
	IRowset*			pIRowset = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IDBAsynchStatus, NULL, &pwszNewURL, (IUnknown**) &pAsynchStatus, DBBINDURLFLAG_READ | 
		DBBINDURLFLAG_ASYNCHRONOUS), S_OK, DB_S_ASYNCHRONOUS, 
		DB_E_ASYNCNOTSUPPORTED)
		
	if(S_OK == hr)
	{
		odtLog<<L"WARNING: The CreateRow call with ASYNCH returned immediately !!\n";
		TESTC(VerifyInterface(pAsynchStatus, IID_IRowset, 
			ROWSET_INTERFACE, (IUnknown**)&pIRowset))
		TESTC(DefaultInterfaceTesting(pIRowset, ROWSET_INTERFACE, IID_IRowset))
		goto CLEANUP;
	}
	else if(DB_E_ASYNCNOTSUPPORTED == hr)
	{
		odtLog<<L"INFO: Asynchronous CreateRow is not supported.\n";
		goto CLEANUP;
	}

	//wait for asynch operation to complete. Then verify.
	TESTC(testAsynchComplete(pAsynchStatus))

	TESTC(VerifyInterface(pAsynchStatus, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))
	TESTC(DefaultInterfaceTesting(pIRowset, ROWSET_INTERFACE, IID_IRowset))	

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pAsynchStatus);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_RESOURCENOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateNewRowset::Variation_15()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				pwszNewURL = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset = NULL;

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IAccessor, NULL, &pwszNewURL, (IUnknown**)
		&pIAccessor, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCENOTSUPPORTED, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	if(hr == S_OK)
	{
		TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
			ROWSET_INTERFACE, (IUnknown**)&pIRowset))			

		TESTC(testRowset(pIAccessor, pIRowset, FALSE))
	}
	else
		odtLog<<L"INFO: CreateRow does not support DBBINDURLFLAG_ISSTRUCTUREDDOCUMENT.\n";

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateNewRowset::Terminate()
{ 
	CleanupNewURLs();
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateStream)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateStream - Test creating Streams (directly)
//| Created:  	11/16/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateStream::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		TESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

		TESTC(InitializeNewURLs())

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IGetSourceRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateStream::Variation_1()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IGetSourceRow*		pIGSR = NULL;

	//Call CreateRow with all interfaces listed as STREAM_INTERFACE.
	QTESTC(testAllIntfCR(STREAM_INTERFACE))

	NEW_URL;
	TEST3C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_STREAM, 
		IID_IGetSourceRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIGSR, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		S_OK, E_NOINTERFACE, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);			

	if(hr == S_OK)
		TESTC(testIGetSourceRow(pIGSR))

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIGSR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateStream::Variation_2()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cBytes = 4999;
	ULONG				cBytesWrote = 0;
	ULONG				cBytesRead = 4999;
	void*				pBuffer = NULL;
	void*				pBuffer2 = NULL;
	WCHAR*				pwszNewURL = NULL;
	ISequentialStream*	pISS = NULL;

	//Create a new stream object.

	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, &pwszNewURL, (IUnknown**)
		&pISS, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		S_OK, DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);			

	//Make a buffer like "2222222..."
	SAFE_ALLOC(pBuffer, BYTE, cBytes);
	memset(pBuffer, 2, cBytes);

	//Write "22222222..." to the newly created stream.
	TESTC_(StorageWrite(IID_ISequentialStream, pISS, pBuffer, cBytes, 
		&cBytesWrote), S_OK)
	TESTC(cBytesWrote == cBytes)

	SAFE_RELEASE(pISS);

	//Bind to the stream again. If Bind is not supported, use
	//CreateRow again.

	TEST2C_(hr = BindResource(pwszNewURL, DBGUID_STREAM, 
		IID_ISequentialStream, NULL, (IUnknown**)
		&pISS), S_OK, E_FAIL)

	if(hr == E_FAIL) //Bind may not be supported on a row or session.
		TESTC_(hr = CreateRow(pwszNewURL, DBGUID_STREAM, 
			IID_ISequentialStream, NULL, NULL, (IUnknown**)
			&pISS, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS), 
			S_OK);

	SAFE_ALLOC(pBuffer2, BYTE, cBytes);
	memset(pBuffer2, 0, cBytes);

	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer2, 
		&cBytesRead), S_OK)
	TESTC(cBytesRead == cBytes)
	TESTC(memcmp(pBuffer, pBuffer2, cBytes) == 0)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_FREE(pBuffer2);
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pISS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IStream (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateStream::Variation_3()
{ 
	TBEGIN
	HRESULT				hr;
	LARGE_INTEGER		ulMove = { 0 };
	ULONG				cBytes = 5001;
	ULONG				cBytesWrote = 0;
	void*				pBuffer = NULL;
	void*				pBuffer2 = NULL;
	WCHAR*				pwszNewURL = NULL;
	IStream*			pIStream = NULL;

	//Create a stream asking for IStream.
	NEW_URL;
	TEST2C_(hr = CreateRow(m_rgURLs[STREAM], DBGUID_STREAM, 
		IID_IStream, NULL, &pwszNewURL, (IUnknown**)&pIStream), S_OK, E_NOINTERFACE)

	TESTC_PROVIDER(hr == S_OK)

	//Make a buffer like "2222222..."
	SAFE_ALLOC(pBuffer, BYTE, cBytes);
	memset(pBuffer, 2, cBytes);

	//Write "22222222..." to the newly created stream.
	TESTC_(StorageWrite(IID_IStream, pIStream, pBuffer, cBytes, 
		&cBytesWrote), S_OK)
	TESTC(cBytesWrote == cBytes)

	//Reset the buffer.
	memset(pBuffer, 0, cBytes);

	//Reset the stream seek pointer to beginning of stream.
	TESTC_(pIStream->Seek(ulMove, STREAM_SEEK_SET, NULL), S_OK)

	//Read the stream.
	TESTC_(GetStorageData(IID_IStream, pIStream, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 5001)

	//Reset the stream seek pointer to beginning of stream.
	TESTC_(pIStream->Seek(ulMove, STREAM_SEEK_SET, NULL), S_OK)

	SAFE_ALLOC(pBuffer2, BYTE, cBytes)

	//Read data again. Compare data to make sure it has started 
	//reading from beginning of the stream.

	TESTC_(GetStorageData(IID_IStream, pIStream, pBuffer2, 
		&cBytes), S_OK)
	TESTC(cBytes == 5001)

	TESTC(memcmp(pBuffer, pBuffer2, cBytes) == 0)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_FREE(pBuffer);
	SAFE_FREE(pBuffer2);
	SAFE_RELEASE(pIStream);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateStream::Terminate()
{ 
	CleanupNewURLs();
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreate_Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreate_Boundary - ICreateRow boundary cases
//| Created:  	11/16/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreate_Boundary::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCBindAndCreate::Init())
	// }}
	{ 
		TBEGIN

		TESTC(InitTC())

		//Skip test case if being run on Session and ICreateRow
		//is not supported on Session.
		TESTC_PROVIDER(m_eTestCase!=TC_SESSION || m_pICreateRow!=NULL)

		TESTC(InitializeNewURLs())

		NEW_URL;

CLEANUP:
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : pwszURL=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_1()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	TESTC_(CreateRow(NULL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)&pIRow), 
		E_INVALIDARG)

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : ppUnk=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_2()
{ 
	TBEGIN
	WCHAR*		pwszNewURL = NULL;

	TESTC_(CreateRow(m_rgURLs[ROW], DBGUID_ROW, 
		IID_IRow, NULL, &pwszNewURL, NULL), 
		E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : pImplSess->piid =NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_3()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	IRowset*			pIRowset = NULL;

	CAggregate			Aggregate(m_pICreateRow);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = NULL;
	dbImplSess.pSession = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, NULL, (IUnknown**)&pIRowset, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION, 
		&dbImplSess), E_INVALIDARG)

	//Also make pwszURL and ppUnk NULL. Should still get E_INVALIDARG.
	TESTC_(CreateRow(NULL, DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, NULL, NULL, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION, 
		&dbImplSess), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : No flags set
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_4()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	TESTC_PROVIDER(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)

	TESTC(m_pIDBBinderProperties != NULL)
	TESTC_(m_pIDBBinderProperties->Reset(), S_OK)

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, 0), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : RECURSIVE flag
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_5()
{ 
	TBEGIN
	WCHAR*		pwszNewURL = NULL;	
	IRowset*	pIRowset = NULL;

	TESTC_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, NULL, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_RECURSIVE), E_INVALIDARG)

	TESTC_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, NULL, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_READ), 
		E_INVALIDARG)

	TESTC_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, NULL, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_READWRITE), 
		E_INVALIDARG)

	TESTC_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, NULL, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_RECURSIVE|DBBINDURLFLAG_SHARE_DENY_NONE), 
		E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : OPENIFEXISTS and OVERWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_6()
{ 
	TBEGIN
	IRowsetInfo*		pIRI = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowsetInfo, NULL, NULL, (IUnknown**)
		&pIRI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_OVERWRITE|DBBINDURLFLAG_COLLECTION), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_7()
{ 
	TBEGIN
	IRowsetInfo*		pIRI = NULL;

	//These flags may not cause a failure since they are only
	//meant to be hints.

	TEST2C_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowsetInfo, NULL, NULL, (IUnknown**)
		&pIRI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION|DBBINDURLFLAG_DELAYFETCHCOLUMNS), 
		E_INVALIDARG, S_OK)
	SAFE_RELEASE(pIRI);

	TEST2C_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IRowsetInfo, NULL, NULL, (IUnknown**)
		&pIRI, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION|DBBINDURLFLAG_DELAYFETCHSTREAM), 
		E_INVALIDARG, S_OK)
	SAFE_RELEASE(pIRI);

CLEANUP:
	SAFE_RELEASE(pIRI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG : DELAYFETCHCOLUMNS & DELAYFETCHSTREAM on STREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_8()
{ 
	TBEGIN
	ISequentialStream*		pISS = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, NULL, (IUnknown**)
		&pISS, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_DELAYFETCHSTREAM), E_INVALIDARG)

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_STREAM, 
		IID_ISequentialStream, NULL, NULL, (IUnknown**)
		&pISS, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_DELAYFETCHCOLUMNS), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IDBProperties on ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_9()
{ 
	TBEGIN
	IDBProperties*		pIDBP = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROWSET, 
		IID_IDBProperties, NULL, NULL, (IUnknown**)
		&pIDBP, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE|
		DBBINDURLFLAG_COLLECTION), E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_IDBProperties on ROW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_10()
{ 
	TBEGIN
	IDBProperties*		pIDBP = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IDBProperties, NULL, NULL, (IUnknown**)
		&pIDBP, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : IID_ISessionProperties on STREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_11()
{ 
	TBEGIN
	ISessionProperties*		pISP = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_STREAM, 
		IID_ISessionProperties, NULL, NULL, (IUnknown**)
		&pISP, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE : riid = IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_12()
{ 
	TBEGIN
	IRow*		pIRow = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_NULL, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		E_NOINTERFACE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTSUPPORTED : DB_NULLGUID and other unsupported GUIDs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_13()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0;
	IUnknown*			pIRow = NULL;
	GUID				guidNull = DB_NULLGUID;

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], guidNull, 
		IID_IRow, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_SQL, 
		IID_IRowset, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_DBSQL, 
		IID_ISessionProperties, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_DEFAULT, 
		IID_IDBInfo, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_MDX, 
		IID_IRow, &dwBindStatus, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	CHECK(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_CONTAINEROBJECT, 
		IID_IRow, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTSUPPORTED : rguid = DSO, SESSION, COMMAND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_14()
{ 
	TBEGIN
	IDBProperties*		pIDBP = NULL;
	ISessionProperties*	pISP = NULL;
	ICommandText*		pICT = NULL;

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_DSO, 
		IID_IDBProperties, NULL, NULL, (IUnknown**)&pIDBP, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED)

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_SESSION, 
		IID_ISessionProperties, NULL, NULL, (IUnknown**)&pISP, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED)

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_COMMAND, 
		IID_ICommandText, NULL, NULL, (IUnknown**)&pICT, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTSUPPORTED)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTFOUND : bogus URL (ROW)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_15()
{ 
	TBEGIN
	WCHAR*	pwszBogusURL = NULL; //Non-existent URL.
	IRow*	pIRow = NULL;

	SAFE_ALLOC(pwszBogusURL, WCHAR, wcslen(m_rgNewURLs[g_cNewURL])
		+wcslen(L"/NonExistentURL")+sizeof(WCHAR))
	wcscpy(pwszBogusURL, m_rgNewURLs[g_cNewURL]);
	wcscat(pwszBogusURL, L"/NonExistentURL");

	TESTC_(CreateRow(pwszBogusURL, DBGUID_ROW, 
		IID_IRow, NULL, NULL, (IUnknown**)&pIRow, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTFOUND)

CLEANUP:
	SAFE_FREE(pwszBogusURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTFOUND: Wierd URLs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_16()
{ 
	TBEGIN
	HRESULT				hrExp = DB_E_NOTFOUND;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;

	//When CreateRow is implemented on a Session, then some providers
	//interpret URLs like " " to be the URL provided as the
	//DATASOURCE property. Hence, binding to " " would give S_OK.

	if(TC_RBINDER != m_eTestCase && TC_PBINDER != m_eTestCase)
		hrExp = S_OK;
	
	CHECK(CreateRow(L"NonExistentURL", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszNewURL);

	CHECKW(CreateRow(L"", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszNewURL);

	CHECKW(CreateRow(L" ", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszNewURL);

	CHECKW(CreateRow(L"                ", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), hrExp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszNewURL);

	CHECK(CreateRow(L":", DBGUID_ROW, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), DB_E_NOTFOUND);

	CHECKW(CreateRow(L"NonExistentScheme:NonExistentURL", DBGUID_ROWSET, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), DB_E_NOTFOUND);

	CHECKW(CreateRow(L"NonExistentScheme://NonExistentURL", DBGUID_ROWSET, 
		IID_IColumnsInfo, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIColumnsInfo), DB_E_NOTFOUND);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION : riid not IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_17()
{ 
	TBEGIN
	IUnknown* pIUnkInner = NULL;
	CAggregate Aggregate(m_pICreateRow);

	TESTC_(CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, NULL, NULL, (IUnknown**)
		&pIUnkInner, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE, 
		NULL, &Aggregate), DB_E_NOAGGREGATION)

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION : pImplSess->piid not IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_18()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	IID					iid = IID_IRow;
	IRow*				pIRow = NULL;

	CAggregate			Aggregate(m_pICreateRow);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = &iid;
	dbImplSess.pSession = NULL;

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, NULL, NULL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE, 
		&dbImplSess), S_OK, DB_E_NOAGGREGATION)

	if(TC_RBINDER == m_eTestCase || TC_PBINDER == m_eTestCase)
		TESTC_(hr, DB_E_NOAGGREGATION)
	else
		TESTC_(hr, S_OK)

CLEANUP:
	SAFE_RELEASE(dbImplSess.pSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTCOLLECTION : create row under a non-collection parent
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_19()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	WCHAR*				pwszURLToCreate = NULL;
	IRow*				pIRow = NULL;

	//Create a non-collection type of resource. Then try to
	//create a child under it. This should fail with DB_E_NOTCOLLECTION.

	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)

	RESOURCE_EXISTS(hr);

	if(!m_lGenerateURL || (m_lGenerateURL & DBPROPVAL_GU_NOTSUPPORTED))
	{
		SAFE_ALLOC(pwszURLToCreate, WCHAR, wcslen(pwszNewURL)
			+wcslen(L"/NonExistentURL")+sizeof(WCHAR));
		wcscpy(pwszURLToCreate, pwszNewURL);
		wcscat(pwszURLToCreate, L"/NonExistentURL");
	}
	else
	{
		SAFE_ALLOC(pwszURLToCreate, WCHAR, wcslen(pwszNewURL)+sizeof(WCHAR));
		wcscpy(pwszURLToCreate, pwszNewURL);
	}

	SAFE_FREE(pwszNewURL);

	TESTC_(hr = CreateRow(pwszURLToCreate, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), 
		DB_E_NOTCOLLECTION)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_FREE(pwszURLToCreate);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTCOLLECTION: Flags (OPENIFEXISTS|COLLECTION) on existing non-collection.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_20()
{ 
	TBEGIN
	HRESULT				hr;
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRow*				pIRow = NULL;

	//Create a Non-Collection row object.
	TEST2C_(hr = CreateRow(m_rgNewURLs[g_cNewURL], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OVERWRITE), S_OK, 
		DB_E_RESOURCEEXISTS)
	SAFE_RELEASE(pIRow);

	RESOURCE_EXISTS(hr);

	TESTC_(CreateRow(pwszNewURL, DBGUID_ROW, 
		IID_IRow, &dwBindStatus, NULL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), DB_E_NOTCOLLECTION)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_E_RESOURCEEXISTS: Flag (OPENIFEXISTS) on an existing collection.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreate_Boundary::Variation_21()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus = 0; 
	WCHAR*				pwszNewURL = NULL;
	IRowset*			pIRowset = NULL;
	IRow*				pIRow = NULL;

	//Create a Collection type object.
	TEST2C_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROWSET, 
		IID_IRowset, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRowset, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|
		DBBINDURLFLAG_COLLECTION), S_OK, DB_E_RESOURCEEXISTS)
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszNewURL);

	TESTC_(CreateRow(m_rgURLs[ROWSET], DBGUID_ROW, 
		IID_IRow, &dwBindStatus, &pwszNewURL, (IUnknown**)
		&pIRow, DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS), 
		DB_E_RESOURCEEXISTS)

CLEANUP:
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreate_Boundary::Terminate()
{ 
	CleanupNewURLs();
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCBindAndCreate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCTransactCreateRow)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransactCreateRow - ICreateRow transaction tests
//| Created:  	2/4/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransactCreateRow::Init()
{ 
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(Zombie::Init())
	// }}
	{
		//This is a mandatory interface, it should always succeed
		if(RegisterInterface(ROW_INTERFACE, IID_ICreateRow, 0, NULL))
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactCreateRow::Variation_1()
{ 
	return TestTxnCreateRow(FALSE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactCreateRow::Variation_2()
{ 
	return TestTxnCreateRow(FALSE, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactCreateRow::Variation_3()
{ 
	return TestTxnCreateRow(TRUE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactCreateRow::Variation_4()
{ 
	return TestTxnCreateRow(TRUE, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCTransactCreateRow::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(Zombie::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END
