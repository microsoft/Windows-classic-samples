//*--------------------------------------------------------------------
//
//	This is the Test Module for IGetSession interface, which is a 
//	mandatory interface on ROW objects.
//
//   WARNING:
//   PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//   Copyright (C) 1994-2000 Microsoft Corporation
//*---------------------------------------------------------------------

#include "MODStandard.hpp"
#include "IGetSession.h"
#include "ExtraLib.h"

//*---------------------------------------------------------------------
// Module Values
//*---------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xb39cade0, 0x57e5, 0x11d2, { 0x88, 0xd7, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("IGetSession");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IGetSession interface.");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


//*---------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	ULONG_PTR ulVal=0;

	if(CommonModuleInit(pThisTestModule))
	{
		if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, 
			g_pIDBInitialize, &ulVal))
		{
			if((ulVal & DBPROPVAL_OO_ROWOBJECT) ||
				(ulVal & DBPROPVAL_OO_SINGLETON) ||
				(ulVal & DBPROPVAL_OO_DIRECTBIND) )
				return TEST_PASS;
			else
				return TEST_SKIPPED;
		}
		else
			return TEST_SKIPPED;
	}

	return FALSE;
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
	return CommonModuleTerminate(pThisTestModule);
}


////////////////////////////////////////////////////////////////////////
// TCBase  -  Class for reusing Test Cases.
// This is one of the base classes from which all the Test Case
// classes will inherit. It is used to duplicate test cases, yet
// maintain some sort of distinct identity for each.
//
////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(TC_RowsetByOpenRowset); }

	//Set the m_fWarning and m_fBinder flags.
	virtual void SetTestCaseParam(ETESTCASE eTestCase = TC_RowsetByOpenRowset)
	{
		m_eTestCase = eTestCase;
		m_fWarning = FALSE;
		m_fBinder = FALSE;

		switch(eTestCase)
		{
		case TC_RowsetByOpenRowset:
			break;

		case TC_RowsetByCommand:
			break;

		case TC_SchemaRowset:
			break;

		case TC_ColumnsRowset:
			m_fWarning = TRUE;
			break;

		case TC_DirectBindOnRootBinder:
		case TC_DirectBindOnProvider:
		case TC_GetSourceRow:
			m_fWarning = TRUE;
			m_fBinder = TRUE;
			break;

		case TC_OpenRowsetDirect:
			break;

		case TC_CommandDirect:
			break;
		
		default:
			ASSERT(!L"Unhandled Type...");
			break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
	//This indicates if the variations in a particular test case would
	//flag the return code DB_E_NOSOURCEOBJECT as a warning (or error).
	BOOL		m_fWarning;
	//Indicates if Direct Binding was involved in obtaining the row 
	//object being tested in a particular test case.
	BOOL		m_fBinder;
};


////////////////////////////////////////////////////////////////////////
//  TCTransactions  -  Class for Transaction Test Case.
//
////////////////////////////////////////////////////////////////////////
class TCTransactions : public CTransaction
{
public:
	//constructors
	TCTransactions(WCHAR* pwszTestCaseName = INVALID(WCHAR*)) : CTransaction(pwszTestCaseName) {}
	int TestTxnRow(BOOL bCommit, BOOL fRetaining);
};

//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Helper function for testing transaction commit/abort variations.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransactions::TestTxnRow(BOOL bCommit,BOOL fRetaining)
{
	TBEGIN
	BOOL			fSuccess		= FALSE;
	HRESULT			ExpectedHr		= E_UNEXPECTED;
	HRESULT			hr				= E_FAIL;
	DBCOUNTITEM		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;
	ULONG			cPropSets		= 0;
	DBPROPSET *		rgPropSets		= NULL;
	ULONG_PTR		ulVal			= 0;
	IGetSession *	pIGetSession	= NULL;
	IUnknown*		pSessUnk		= NULL;

	//Not required to set this property. Just doing it for additional testing.
	SetSettableProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets);

	//Check to see if Opening ROW objects thru OpenRowset is supported.

	if(!GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, 
		(IUnknown*)m_pIDBCreateSession, &ulVal) ||
		(!(ulVal & DBPROPVAL_OO_SINGLETON) && !(ulVal & DBPROPVAL_OO_ROWOBJECT)))
	{
		odtLog<<L"INFO: ROW objects are not supported.\n";
		goto CLEANUP;
	}

	TESTC(StartTransaction(USE_OPENROWSET, (IUnknown **)&pIGetSession, 
		cPropSets, rgPropSets, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))

	CHECK(pIGetSession->GetSession(IID_IUnknown, (IUnknown**)&pSessUnk),S_OK);
	COMPARE(DefaultObjectTesting(pSessUnk, SESSION_INTERFACE), TRUE);
	COMPARE(VerifyEqualInterface(pSessUnk, m_pIOpenRowset), TRUE);
	SAFE_RELEASE(pSessUnk);

	if (bCommit)
		TESTC(GetCommit(fRetaining))
	else
		TESTC(GetAbort(fRetaining))

	// Make sure everything still works after commit or abort
	if ((bCommit && m_fCommitPreserve) ||
		((!bCommit) && m_fAbortPreserve))
		ExpectedHr = S_OK;

	// Test zombie
	if(m_pIRowset)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	//Make sure everything still works after commit or abort
	CHECK(hr=pIGetSession->GetSession(IID_IUnknown, (IUnknown**)&pSessUnk),ExpectedHr);
	if(FAILED(hr))
		TESTC(!pSessUnk)
	else
		TESTC(DefaultObjectTesting(pSessUnk, SESSION_INTERFACE))
		
CLEANUP:
	if (m_pIRowset && rghRows)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pSessUnk);
	SAFE_RELEASE(pIGetSession);

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	CleanUpTransaction((fRetaining) ? S_OK : XACT_E_NOTRANSACTION);

	TRETURN
} //TestTxnRow



////////////////////////////////////////////////////////////////////////
//  CGetSession  - Class for IGetSession Test Cases.
//
////////////////////////////////////////////////////////////////////////
class CGetSession : public CSessionObject, public TCBase
{
public:
	//constructors
	CGetSession(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~CGetSession() {}

	//methods
	virtual BOOL		Init(ETESTCASE eTC=TC_RowsetByOpenRowset);
	virtual BOOL		Terminate();

protected:

//VARIABLES...

	HRESULT				m_hr;
	DBCOUNTITEM			m_cRowsObtained;
	HROW*				m_rghRows;
	WCHAR*				m_pwszRowURL;
	CRowset*			m_pCRowset;
	CRowset*			m_pCRowsetA;
	CRowObject*			m_pCRowObj;

//INTERFACES...

	IBindResource*		m_pIBindResource;
	IUnknown*			m_pImplSess;

//METHODS...

	//Cleanup operation for terminating a test case.
	BOOL	ReleaseAll();

	//Execute a select * from [table] command to obtain a rowset.
	BOOL	GetRowsetFromCommand(
				ICommandText*	pICT,
				IRowset**		ppIRowset);

	//Test the obtained IOpenRowset interface.
	BOOL	testIOpenRowset(IOpenRowset* pIOpenRowset);

	//Helper function for testing variations
	BOOL	VerifyGetSession(REFIID	riid);

	//Get the IBindResource on Root Binder and get a ROW URL.
	BOOL	GetRootBinder();

};


//----------------------------------------------------------------------
// CGetSession::CGetSession
//
CGetSession::CGetSession(WCHAR * wstrTestCaseName)	: CSessionObject(wstrTestCaseName) 
{
	m_pwszRowURL		= NULL;

	m_cRowsObtained		= 0;
	m_rghRows			= NULL;
	m_pwszRowURL		= NULL;
	m_pCRowset			= NULL;
	m_pCRowsetA			= NULL;
	m_pCRowObj			= NULL;

	m_pIBindResource	= NULL;
	m_pImplSess			= NULL;
}

//----------------------------------------------------------------------
// CGetSession::Init
//
BOOL CGetSession::Init(ETESTCASE eTC)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	IID					iid = IID_IOpenRowset;
	DBID				dbid;
	DBIMPLICITSESSION	dbImplSess;
	ULONG_PTR			ulVal = 0;
	BOOL				bSingleton = FALSE;
	BOOL				fSkip = FALSE;
	IDBCreateCommand*	pIDBCC = NULL;
	IDBSchemaRowset*	pIDBSR = NULL;
	IColumnsRowset*		pIColRowset = NULL;
	ICommandText*		pICT = NULL;
	IRowset*			pIRowset = NULL;
	IGetSourceRow*		pIGSR = NULL;
	IRow*				pIRow = NULL;
	IBindResource*		pIBRProv = NULL;
	CRowset				RowsetA;

	memset(&dbImplSess, 0, sizeof(DBIMPLICITSESSION));
	m_pCRowset = NULL;
	m_pCRowObj = NULL;

	TESTC(CSessionObject::Init())

	SetTable(g_pTable, DELETETABLE_NO);
	m_pCRowset = new CRowset();
	TESTC(m_pCRowset!=NULL)
	m_pCRowObj = new CRowObject();
	TESTC(m_pCRowObj!=NULL)

	GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, 
		(IUnknown*)g_pIDBInitialize, &ulVal); 

	//The INIT will vary depending on type of Test Case. They all involve
	//getting a ROW object (by various different means) and putting it
	//in m_pCRowObj.

	switch(eTC)
	{
		//Get a Rowset using OpenRowset, and obtain a Row obj from it.
		case TC_RowsetByOpenRowset:
		{
			VARIANT_BOOL bValue;

			m_pCRowset->SetProperty(DBPROP_CANHOLDROWS);
			TESTC_(m_pCRowset->CreateRowset(),S_OK);

			TESTC_(m_pCRowset->GetNextRows(0, 1, &m_cRowsObtained, &m_rghRows),S_OK)
	
			TEST3C_(hr = m_pCRowObj->CreateRowObject(m_pCRowset->pIRowset(), 
				m_rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE)

			if(!(ulVal & DBPROPVAL_OO_ROWOBJECT))
				fSkip = TRUE;

			if(hr == E_NOINTERFACE)
			{
				COMPARE(fSkip, TRUE);
				TESTC_PROVIDER(!fSkip)
			}
			else
				COMPARE(fSkip, FALSE);

			//Make sure DBPROP_IRow prop is supported by the rowset.
			COMPARE(GetProperty(DBPROP_IRow, DBPROPSET_ROWSET, (IUnknown*)
				m_pCRowset->pIRowset(), &bValue), TRUE);
		}
			break;

		//Get a Rowset by executing a command, and obtain a Row obj from it.
		case TC_RowsetByCommand:
		{
			VARIANT_BOOL bValue;

			TESTC_PROVIDER(VerifyInterface(g_pIOpenRowset,IID_IDBCreateCommand,
				SESSION_INTERFACE, (IUnknown**)&pIDBCC))

			TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandText, (IUnknown**)
				&pICT), S_OK)

			TESTC(GetRowsetFromCommand(pICT, &pIRowset))

			TESTC_(m_pCRowset->CreateRowset(pIRowset),S_OK);

			TESTC_(m_pCRowset->GetNextRows(0, 1, &m_cRowsObtained, &m_rghRows),S_OK)
		
			TEST3C_(hr = m_pCRowObj->CreateRowObject(m_pCRowset->pIRowset(), 
				m_rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE)

			if(!(ulVal & DBPROPVAL_OO_ROWOBJECT))
				fSkip = TRUE;

			if(hr == E_NOINTERFACE)
			{
				COMPARE(fSkip, TRUE);
				TESTC_PROVIDER(!fSkip)
			}
			else
				COMPARE(fSkip, FALSE);

			//Make sure DBPROP_IRow prop is supported by the rowset.
			COMPARE(GetProperty(DBPROP_IRow, DBPROPSET_ROWSET, (IUnknown*)
				m_pCRowset->pIRowset(), &bValue), TRUE);
		}
			break;

		//Get a Schema Rowset, and obtain a Row obj from it.
		case TC_SchemaRowset:
		{
			TESTC_PROVIDER(VerifyInterface(g_pIOpenRowset,IID_IDBSchemaRowset,
				SESSION_INTERFACE, (IUnknown**)&pIDBSR))

			TESTC_(pIDBSR->GetRowset(NULL, DBSCHEMA_PROVIDER_TYPES, 0,
				NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK)

			TESTC_(m_pCRowset->CreateRowset(pIRowset),S_OK);

			TESTC_(m_pCRowset->GetNextRows(0, 1, &m_cRowsObtained, &m_rghRows),S_OK)
		
			TEST3C_(hr = m_pCRowObj->CreateRowObject(m_pCRowset->pIRowset(), 
				m_rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE)

			TESTC_PROVIDER(hr != E_NOINTERFACE)
		}
			break;

		//Get a Columns Rowset, and obtain a Row obj from it.
		case TC_ColumnsRowset:
		{
			m_pCRowsetA = new CRowset();
			TESTC(m_pCRowsetA!=NULL)

			m_pCRowsetA->SetProperty(DBPROP_IColumnsRowset);
			TESTC_PROVIDER(S_OK == m_pCRowsetA->CreateRowset())

			TESTC(VerifyInterface(m_pCRowsetA->pIRowset(),IID_IColumnsRowset,
				ROWSET_INTERFACE, (IUnknown**)&pIColRowset))

			TESTC_(pIColRowset->GetColumnsRowset(NULL, 0, NULL, IID_IRowset,
				0, NULL, (IUnknown**)&pIRowset), S_OK)

			TESTC_(m_pCRowset->CreateRowset(pIRowset),S_OK);

			TESTC_(m_pCRowset->GetNextRows(0, 1, &m_cRowsObtained, &m_rghRows),S_OK)
		
			TEST3C_(hr = m_pCRowObj->CreateRowObject(m_pCRowset->pIRowset(), 
				m_rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE)

			TESTC_PROVIDER(hr != E_NOINTERFACE)
		}
			break;

		//Get a Row obj by direct binding from Root Binder.
		case TC_DirectBindOnRootBinder:
		{
			TESTC(GetRootBinder())

			dbImplSess.pUnkOuter = NULL;
			dbImplSess.pSession = NULL;
			dbImplSess.piid = &iid;

			TESTC_PROVIDER(m_pwszRowURL!=NULL)

			TESTC_PROVIDER((ulVal & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND)

			TESTC_(m_pIBindResource->Bind(NULL, m_pwszRowURL, 
				DBBINDURLFLAG_READ, DBGUID_ROW, IID_IRow, NULL, &dbImplSess,
				NULL, (IUnknown**)&pIRow), S_OK)
			TESTC((pIRow != NULL) && (dbImplSess.pSession != NULL))

			TESTC(VerifyInterface(dbImplSess.pSession,IID_IOpenRowset,
				SESSION_INTERFACE, (IUnknown**)&m_pImplSess))
			
			TESTC_(m_pCRowObj->SetRowObject(pIRow),S_OK)
		}
			break;

		//Get a Row obj by direct binding from Provider.
		case TC_DirectBindOnProvider:
		{
			TESTC(GetRootBinder())

			dbImplSess.pUnkOuter = NULL;
			dbImplSess.pSession = NULL;
			dbImplSess.piid = &iid;

			TESTC_PROVIDER(m_pwszRowURL!=NULL)

			TESTC_PROVIDER((ulVal & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND)

			TESTC_(m_pIBindResource->Bind(NULL, m_pwszRowURL, 
				DBBINDURLFLAG_READ, DBGUID_SESSION, IID_IBindResource, NULL,
				NULL, NULL, (IUnknown**)&pIBRProv), S_OK)
			TESTC(pIBRProv!=NULL)

			TESTC_(pIBRProv->Bind(NULL, m_pwszRowURL, 
				DBBINDURLFLAG_READ, DBGUID_ROW, IID_IRow, NULL, NULL,
				NULL, (IUnknown**)&pIRow), S_OK)
			TESTC(pIRow != NULL)

			TESTC(VerifyInterface(pIBRProv,IID_IOpenRowset,
				SESSION_INTERFACE, (IUnknown**)&m_pImplSess))
			
			TESTC_(m_pCRowObj->SetRowObject(pIRow),S_OK)
		}
			break;

		//Get a Row obj by calling GetSourceRow from a Stream obj.
		case TC_GetSourceRow:
		{
			TESTC(GetRootBinder())

			dbImplSess.pUnkOuter = NULL;
			dbImplSess.pSession = NULL;
			dbImplSess.piid = &iid;

			TESTC_PROVIDER(m_pwszRowURL!=NULL)

			TESTC_PROVIDER((ulVal & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND)

			TESTC_(m_pIBindResource->Bind(NULL, m_pwszRowURL, 
				DBBINDURLFLAG_READ, DBGUID_STREAM, IID_IGetSourceRow, NULL, 
				&dbImplSess, NULL, (IUnknown**)&pIGSR), S_OK)
			TESTC((pIGSR != NULL) && (dbImplSess.pSession != NULL))

			TESTC(VerifyInterface(dbImplSess.pSession,IID_IOpenRowset,
				SESSION_INTERFACE, (IUnknown**)&m_pImplSess))

			TESTC_PROVIDER(pIGSR->GetSourceRow(IID_IRow, (IUnknown**)&pIRow) == S_OK)
			TESTC(pIRow!=NULL)
			
			TESTC_(m_pCRowObj->SetRowObject(pIRow),S_OK)
		}
			break;

		//Get a Row obj from OpenRowset.
		case TC_OpenRowsetDirect:
		{
			TESTC((g_pIOpenRowset!=NULL) && (m_pTable!=NULL))
			dbid = m_pTable->GetTableID();

			TEST3C_(hr = g_pIOpenRowset->OpenRowset(NULL, &dbid, NULL,
				IID_IRow, 0, NULL, (IUnknown**)&pIRow), S_OK, DB_S_NOTSINGLETON, 
				E_NOINTERFACE)

			if(!(ulVal & DBPROPVAL_OO_SINGLETON))
				fSkip = TRUE;

			if(hr == E_NOINTERFACE)
			{
				COMPARE(fSkip, TRUE);
				TESTC_PROVIDER(!fSkip)
			}
			else
				COMPARE(fSkip, FALSE);

			TESTC(pIRow!=NULL)
		
			TESTC_(m_pCRowObj->SetRowObject(pIRow), S_OK)
		}
			break;

		//Get a Row obj directly by executing a command.
		case TC_CommandDirect:
		{
			if(ulVal & DBPROPVAL_OO_SINGLETON)
				bSingleton = TRUE;

			TEST3C_(hr = m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRow,
				NULL, NULL, NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, NULL,
				(IUnknown**)&pIRow, NULL), S_OK,
				DB_S_NOTSINGLETON, E_NOINTERFACE)

			if(!(ulVal & DBPROPVAL_OO_SINGLETON))
				fSkip = TRUE;

			if(hr == E_NOINTERFACE)
			{
				COMPARE(fSkip, TRUE);
				TESTC_PROVIDER(!fSkip)
			}
			else
				COMPARE(fSkip, FALSE);

			TESTC(pIRow != NULL)
		
			TESTC_(m_pCRowObj->SetRowObject(pIRow), S_OK)
		}
			break;
		
		default:
			ASSERT(!L"Unhandled Type...");
			break;
	};


CLEANUP:
	SAFE_RELEASE(pICT);
	SAFE_RELEASE(pIBRProv);
	SAFE_RELEASE(pIGSR);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(dbImplSess.pSession);
	SAFE_RELEASE(pIColRowset);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSR);
	SAFE_RELEASE(pIDBCC);
	TRETURN
} //Init

//----------------------------------------------------------------------
// CGetSession::Terminate
//
BOOL CGetSession::Terminate()
{
	ReleaseAll();
	return CSessionObject::Terminate();
} //Terminate

//----------------------------------------------------------------------
// CGetSession::ReleaseAll
//
BOOL CGetSession::ReleaseAll()
{
	if(m_rghRows && m_pCRowset)
		m_pCRowset->ReleaseRows(m_cRowsObtained, m_rghRows);
	SAFE_FREE(m_rghRows);
	SAFE_FREE(m_pwszRowURL);
	SAFE_DELETE(m_pCRowObj);
	SAFE_DELETE(m_pCRowset);
	SAFE_DELETE(m_pCRowsetA);

	SAFE_RELEASE(m_pIBindResource);
	SAFE_RELEASE(m_pImplSess);
	return TRUE;
} //ReleaseAll

//----------------------------------------------------------------------
// CGetSession::testIOpenRowset
// Perform some basic testing of IOpenRowset interface that cannot be 
// done in DefTestInterface func.
//
BOOL CGetSession::testIOpenRowset(IOpenRowset* pIOpenRowset)
{
	TBEGIN
	HRESULT			hr;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBID*			pTableID = NULL;
	IOpenRowset*	pIOR = NULL;
	IRowsetInfo*	pIRowsetInfo = NULL;

	TESTC((pIOpenRowset!=NULL) && (m_pTable!=NULL))

	pTableID = &(m_pTable->GetTableIDRef());
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	//Open a rowset with IRowsetInfo and test it.

	TESTC_(pIOpenRowset->OpenRowset(NULL, pTableID, NULL, IID_IRowsetInfo,
		cPropSets, rgPropSets, (IUnknown**)&pIRowsetInfo), S_OK)
	TESTC(pIRowsetInfo!=NULL)
	TEST2C_(hr=pIRowsetInfo->GetSpecification(IID_IOpenRowset, (IUnknown**)
		&pIOR), S_OK, S_FALSE)
	if(hr == S_OK)
		TESTC(VerifyEqualInterface(pIOpenRowset, pIOR))
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, pIRowsetInfo))
	TESTC(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, pIRowsetInfo))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
} //testIOpenRowset


//----------------------------------------------------------------------
// CGetSession::GetRowsetFromCommand
// Execute a select * from [table] command to get back a rowset.
//
BOOL CGetSession::GetRowsetFromCommand(
	ICommandText*	pICT,
	IRowset**		ppIRowset)
{
	TBEGIN

	TESTC((pICT!=NULL) && (ppIRowset!=NULL) && (m_pTable!=NULL))
	*ppIRowset = NULL;

	TESTC_(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset,
		NULL,NULL,NULL,NULL, EXECUTE_IFNOERROR, 0, NULL, NULL,
		(IUnknown**)ppIRowset, (ICommand**)&pICT), S_OK)

	TESTC(ppIRowset != NULL)
	TESTC(*ppIRowset != NULL)

CLEANUP:
	TRETURN
} //GetRowsetFromCommand


//----------------------------------------------------------------------
// CGetSession::VerifyGetSession
// Common testing function for all positive case variations.
//
BOOL CGetSession::VerifyGetSession(REFIID riid)
{
	TBEGIN

	IUnknown* pISessUnk = INVALID(IUnknown*);

	TEST3C_(m_hr = m_pCRowObj->GetSession(riid, (IUnknown**)&pISessUnk), 
		S_OK, DB_E_NOSOURCEOBJECT, E_NOINTERFACE)

	if(FAILED(m_hr))
		TESTC(!pISessUnk)
	else
		TESTC(pISessUnk != NULL)

	//Should not get E_NOINTERFACE when a mandatory interface was asked.

	if(riid==IID_IOpenRowset || riid==IID_IGetDataSource || 
		riid==IID_ISessionProperties || riid==IID_IUnknown)
		TEST2C_(m_hr, S_OK, DB_E_NOSOURCEOBJECT)

	if(E_NOINTERFACE == m_hr)
	{
		odtLog<<L"INFO: "<<GetInterfaceName(riid)<<L" is not supported on session.\n";
		goto CLEANUP;			//Passed.
	}

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
	{
		CHECKW(m_hr, S_OK);
		goto CLEANUP;			//Passed.
	}
	else
		TESTC_(m_hr, S_OK)

	TESTC(DefaultInterfaceTesting(pISessUnk, SESSION_INTERFACE, riid))

	if((IID_IOpenRowset==riid) && (!m_fBinder))
		TESTC(testIOpenRowset((IOpenRowset*)pISessUnk))

	//If the ROW object was obtained by going thru a Binder, then
	//it's Session is stored in m_pImplSess, else in g_pIOpenRowset.
	if(m_fBinder)
		TESTC(VerifyEqualInterface(pISessUnk, m_pImplSess))
	else
		TESTC(VerifyEqualInterface(pISessUnk, g_pIOpenRowset))

CLEANUP:
	SAFE_RELEASE(pISessUnk);
	TRETURN
} //VerifyGetSession


//----------------------------------------------------------------------
// CGetSession::GetRootBinder
// Get the IBindResource interface on Root Binder, Set Init Props, and 
// get a ROW URL.
//
BOOL CGetSession::GetRootBinder()
{
	TBEGIN
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	IBindResource*			pIBR = NULL;
	IDBBinderProperties*	pIDBBProp = NULL;

	pIBR = GetModInfo()->GetRootBinder();
	if(!pIBR)
		return FALSE;

	if(!VerifyInterface(pIBR, IID_IBindResource,
		BINDER_INTERFACE,(IUnknown**)&m_pIBindResource))
		return FALSE;

	if(!VerifyInterface(m_pIBindResource, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBProp))
		return FALSE;

	TESTC(GetInitProps(&cPropSets, &rgPropSets))

	TESTC_(pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	//If an INI file exists and there is a [URL] section in it, get the
	//ROW URL from there, otherwise look for it in CModInfo.

	if(GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE))
		m_pwszRowURL = wcsDuplicate(GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE));
	else if(GetModInfo()->GetRootURL())
		m_pwszRowURL = wcsDuplicate(GetModInfo()->GetRootURL());

	if(!m_pwszRowURL)
		odtLog<<L"WARNING: Could not get a ROW URL. Some Test Cases won't run.\n;";
	
CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBBProp);
	TRETURN
} //GetRootBinder




//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------


// {{ TCW_TEST_CASE_MAP(TCRowsetByOpenRowset)
//*-----------------------------------------------------------------------
// @class GetSession is tested on Row objects obtained from a Rowset which was opened using IOpenRowset
//
class TCRowsetByOpenRowset : public CGetSession { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetByOpenRowset,CGetSession);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IUnknown
	int Variation_1();
	// @cmember IID_IOpenRowset
	int Variation_2();
	// @cmember IID_IGetDataSource
	int Variation_3();
	// @cmember IID_ISessionProperties
	int Variation_4();
	// @cmember IID_IDBCreateCommand
	int Variation_5();
	// @cmember IID_IDBSchemaRowset
	int Variation_6();
	// @cmember ALL Optional Interfaces
	int Variation_7();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRowsetByOpenRowset)
#define THE_CLASS TCRowsetByOpenRowset
BEG_TEST_CASE(TCRowsetByOpenRowset, CGetSession, L"GetSession is tested on Row objects obtained from a Rowset which was opened using IOpenRowset")
	TEST_VARIATION(1, 		L"IID_IUnknown")
	TEST_VARIATION(2, 		L"IID_IOpenRowset")
	TEST_VARIATION(3, 		L"IID_IGetDataSource")
	TEST_VARIATION(4, 		L"IID_ISessionProperties")
	TEST_VARIATION(5, 		L"IID_IDBCreateCommand")
	TEST_VARIATION(6, 		L"IID_IDBSchemaRowset")
	TEST_VARIATION(7, 		L"ALL Optional Interfaces")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowsetByOpenRowset_Boundary)
//*-----------------------------------------------------------------------
// @class Boundary cases.
//
class TCRowsetByOpenRowset_Boundary : public CGetSession { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetByOpenRowset_Boundary,CGetSession);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG
	int Variation_1();
	// @cmember E_INVALIDARG or E_NOINTERFACE
	int Variation_2();
	// @cmember E_NOINTERFACE - IID_NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - IID_IDBCreateSession
	int Variation_4();
	// @cmember E_NOINTERFACE - IID_ICommandText
	int Variation_5();
	// @cmember E_NOINTERFACE - IID_IRowsetInfo
	int Variation_6();
	// @cmember E_NOINTERFACE - IID_IRow
	int Variation_7();
	// @cmember E_NOINTERFACE - IID_IGetSourceRow
	int Variation_8();
	// @cmember E_NOINTERFACE - IID_IDBBinderProperties
	int Variation_9();
	// @cmember E_NOINTERFACE - IID_IRegisterProvider
	int Variation_10();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRowsetByOpenRowset_Boundary)
#define THE_CLASS TCRowsetByOpenRowset_Boundary
BEG_TEST_CASE(TCRowsetByOpenRowset_Boundary, CGetSession, L"Boundary cases.")
	TEST_VARIATION(1, 		L"E_INVALIDARG")
	TEST_VARIATION(2, 		L"E_INVALIDARG or E_NOINTERFACE")
	TEST_VARIATION(3, 		L"E_NOINTERFACE - IID_NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - IID_IDBCreateSession")
	TEST_VARIATION(5, 		L"E_NOINTERFACE - IID_ICommandText")
	TEST_VARIATION(6, 		L"E_NOINTERFACE - IID_IRowsetInfo")
	TEST_VARIATION(7, 		L"E_NOINTERFACE - IID_IRow")
	TEST_VARIATION(8, 		L"E_NOINTERFACE - IID_IGetSourceRow")
	TEST_VARIATION(9, 		L"E_NOINTERFACE - IID_IDBBinderProperties")
	TEST_VARIATION(10, 		L"E_NOINTERFACE - IID_IRegisterProvider")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCMiscellaneous)
//*-----------------------------------------------------------------------
// @class Miscellaneous variations
//
class TCMiscellaneous : public CGetSession{ 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMiscellaneous,CGetSession);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Aggregate a session and get a row.
	int Variation_1();
	// @cmember Release parent objects before calling GetSession.
	int Variation_2();
	// @cmember Delete an HROW and call GetSession.
	int Variation_3();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCMiscellaneous)
#define THE_CLASS TCMiscellaneous
BEG_TEST_CASE(TCMiscellaneous, CGetSession, L"Miscellaneous variations")
	TEST_VARIATION(1, 		L"Aggregate a session and get a row.")
	TEST_VARIATION(2, 		L"Release parent objects before calling GetSession.")
	TEST_VARIATION(3, 		L"Delete an HROW and call GetSession.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowZombie)
//*-----------------------------------------------------------------------
// @class Zombie state tests
//
class TCRowZombie : public TCTransactions { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowZombie,TCTransactions);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining=FALSE
	int Variation_1();
	// @cmember Abort with fRetaining=TRUE
	int Variation_2();
	// @cmember Commit with fRetaining=FALSE
	int Variation_3();
	// @cmember Commit with fRetaining=TRUE
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRowZombie)
#define THE_CLASS TCRowZombie
BEG_TEST_CASE(TCRowZombie, TCTransactions, L"Zombie state tests")
	TEST_VARIATION(1, 		L"Abort with fRetaining=FALSE")
	TEST_VARIATION(2, 		L"Abort with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"Commit with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"Commit with fRetaining=TRUE")
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

//2
COPY_TEST_CASE(TCRowsetByCommand, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCRowsetByCommand_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCRowsetByCommand_Zombie, TCRowZombie)

//3
COPY_TEST_CASE(TCSchemaRowset, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCSchemaRowset_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCSchemaRowset_Zombie, TCRowZombie)

//4
COPY_TEST_CASE(TCColumnsRowset, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCColumnsRowset_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCColumnsRowset_Zombie, TCRowZombie)

//5
COPY_TEST_CASE(TCDirectBindOnRootBinder, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCDirectBindOnRootBinder_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCDirectBindOnRootBinder_Zombie, TCRowZombie)

//6
COPY_TEST_CASE(TCDirectBindOnProvider, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCDirectBindOnProvider_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCDirectBindOnProvider_Zombie, TCRowZombie)

//7
COPY_TEST_CASE(TCGetSourceRow, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCGetSourceRow_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCGetSourceRow_Zombie, TCRowZombie)

//8
COPY_TEST_CASE(TCOpenRowsetDirect, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCOpenRowsetDirect_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCOpenRowsetDirect_Zombie, TCRowZombie)

//9
COPY_TEST_CASE(TCCommandDirect, TCRowsetByOpenRowset)
COPY_TEST_CASE(TCCommandDirect_Boundary, TCRowsetByOpenRowset_Boundary)
COPY_TEST_CASE(TCCommandDirect_Zombie, TCRowZombie)


//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a parameter which changes the behvior.
//So we make LTM think there are 15 cases in here with different names, but in
//reality we only have to maintain code for the unique cases. 

#if 0 
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCRowsetByOpenRowset)
	TEST_CASE(2, TCRowsetByOpenRowset_Boundary)
	TEST_CASE(3, TCMiscellaneous)
	TEST_CASE(4, TCRowZombie)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(20, ThisModule, gwszModuleDescrip)
	//1
	TEST_CASE(1, TCRowsetByOpenRowset)
	TEST_CASE(2, TCRowsetByOpenRowset_Boundary)

	//2
	TEST_CASE_WITH_PARAM(3, TCRowsetByCommand, TC_RowsetByCommand)
	TEST_CASE_WITH_PARAM(4, TCRowsetByCommand_Boundary, TC_RowsetByCommand)

	//3
	TEST_CASE_WITH_PARAM(5, TCSchemaRowset, TC_SchemaRowset)
	TEST_CASE_WITH_PARAM(6, TCSchemaRowset_Boundary, TC_SchemaRowset)

	//4
	TEST_CASE_WITH_PARAM(7, TCColumnsRowset, TC_ColumnsRowset)
	TEST_CASE_WITH_PARAM(8, TCColumnsRowset_Boundary, TC_ColumnsRowset)

	//5
	TEST_CASE_WITH_PARAM(9, TCDirectBindOnRootBinder, TC_DirectBindOnRootBinder)
	TEST_CASE_WITH_PARAM(10, TCDirectBindOnRootBinder_Boundary, TC_DirectBindOnRootBinder)

	//6
	TEST_CASE_WITH_PARAM(11, TCDirectBindOnProvider, TC_DirectBindOnProvider)
	TEST_CASE_WITH_PARAM(12, TCDirectBindOnProvider_Boundary, TC_DirectBindOnProvider)

	//7
	TEST_CASE_WITH_PARAM(13, TCGetSourceRow, TC_GetSourceRow)
	TEST_CASE_WITH_PARAM(14, TCGetSourceRow_Boundary, TC_GetSourceRow)

	//8
	TEST_CASE_WITH_PARAM(15, TCOpenRowsetDirect, TC_OpenRowsetDirect)
	TEST_CASE_WITH_PARAM(16, TCOpenRowsetDirect_Boundary, TC_OpenRowsetDirect)

	//9
	TEST_CASE_WITH_PARAM(17, TCCommandDirect, TC_CommandDirect)
	TEST_CASE_WITH_PARAM(18, TCCommandDirect_Boundary, TC_CommandDirect)

	TEST_CASE(19, TCMiscellaneous)
	TEST_CASE(20, TCRowZombie)

END_TEST_MODULE()
#endif



// {{ TCW_TC_PROTOTYPE(TCRowsetByOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetByOpenRowset - GetSession is tested on Row objects obtained from a Rowset which was opened using IOpenRowset
//| Created:  	9/29/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetByOpenRowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	return CGetSession::Init(m_eTestCase);
	// }}
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_1()
{ 
	return VerifyGetSession(IID_IUnknown);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IID_IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_2()
{ 
	return VerifyGetSession(IID_IOpenRowset);
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IID_IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_3()
{ 
	return VerifyGetSession(IID_IGetDataSource);
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IID_ISessionProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_4()
{ 
	return VerifyGetSession(IID_ISessionProperties);
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBCreateCommand
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_5()
{ 
	return VerifyGetSession(IID_IDBCreateCommand);
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBSchemaRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_6()
{ 
	return VerifyGetSession(IID_IDBSchemaRowset);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc ALL Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset::Variation_7()
{ 
	TBEGIN
	ULONG		  i			   = 0;
	ULONG		  cInterfaces  = 0;
	INTERFACEMAP* rgInterfaces = NULL;

	// Obtain the SESSION interface array
	TESTC(GetInterfaceArray(SESSION_INTERFACE, &cInterfaces, &rgInterfaces));

	for(i=0; i < cInterfaces; i++)
	{
		//If interface is optional
		if(!rgInterfaces[i].fMandatory)
			TESTC(VerifyGetSession(*(rgInterfaces[i].pIID)));
	}

CLEANUP:
	if(TESTB==TEST_FAIL)
		odtLog<<L"INFO: Failed on interface "<<GetInterfaceName(*(rgInterfaces[i].pIID))<<".\n";
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRowsetByOpenRowset::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSession::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCRowsetByOpenRowset_Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetByOpenRowset_Boundary - Boundary cases.
//| Created:  	10/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetByOpenRowset_Boundary::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	return CGetSession::Init(m_eTestCase);
	// }}
} 

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_1()
{ 
	TBEGIN

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IOpenRowset, NULL), 
		E_INVALIDARG, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG or E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_2()
{ 
	TBEGIN

	TEST3C_(m_hr = m_pCRowObj->GetSession(IID_IDBCreateSession, NULL), 
		E_INVALIDARG, E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_3()
{ 
	TBEGIN
	IUnknown*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_NULL, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IDBCreateSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_4()
{ 
	TBEGIN
	IDBCreateSession*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IDBCreateSession, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_ICommandText
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_5()
{ 
	TBEGIN
	ICommandText*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_ICommandText, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_6()
{ 
	TBEGIN
	IRowsetInfo*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IRowsetInfo, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_7()
{ 
	TBEGIN
	IRow*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IRow, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IGetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_8()
{ 
	TBEGIN
	IGetSourceRow*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IGetSourceRow, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IDBBinderProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_9()
{ 
	TBEGIN
	IDBBinderProperties*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IDBBinderProperties, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IRegisterProvider
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetByOpenRowset_Boundary::Variation_10()
{ 
	TBEGIN
	IRegisterProvider*	pIUnk = NULL;

	TEST2C_(m_hr = m_pCRowObj->GetSession(IID_IRegisterProvider, (IUnknown**)&pIUnk), 
		E_NOINTERFACE, DB_E_NOSOURCEOBJECT)

	if((DB_E_NOSOURCEOBJECT == m_hr) && m_fWarning)
		CHECKW(m_hr, E_NOINTERFACE);

	TESTC(!pIUnk)

CLEANUP:
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRowsetByOpenRowset_Boundary::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSession::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCMiscellaneous)
//*-----------------------------------------------------------------------
//| Test Case:	TCMiscellaneous - Miscellaneous variations
//| Created:  	5/18/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMiscellaneous::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSessionObject::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Aggregate a session and get a row.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMiscellaneous::Variation_1()
{ 
	TBEGIN
	HRESULT				hr;
	DBCOUNTITEM			cRowsObtained=0;
	HROW*				rghRows=NULL;
	IDBCreateSession*	pIDBCS=NULL;
	IOpenRowset*		pIOR=NULL;
	IGetRow*			pIGR=NULL;
	IRowset*			pIRowset=NULL;
	IGetSession*		pIGS=NULL;
	IUnknown*			pISessUnk=NULL;
	IUnknown*			pIUnkInner = NULL;
	IRow*				pIRow = NULL;
	CAggregate			Aggregate;

	//Create a new aggregated session.
	TEST2C_(hr = CreateNewSession(NULL, IID_IUnknown, 
		(IUnknown**)&pIUnkInner, &Aggregate), S_OK, DB_E_NOAGGREGATION);
	Aggregate.SetUnkInner(pIUnkInner);

	TESTC_PROVIDER(hr == S_OK);

	TESTC(VerifyInterface(pIUnkInner,IID_IOpenRowset,
		SESSION_INTERFACE, (IUnknown**)&pIOR))

	//Open a rowset on this aggregated session and get a row
	//object from it.

	TEST2C_(hr = pIOR->OpenRowset(NULL, &(g_pTable->GetTableID()), 
		NULL, IID_IGetRow, 0, NULL, (IUnknown**)&pIGR), S_OK, E_NOINTERFACE)

	if(hr == S_OK)
	{
		TESTC(VerifyInterface(pIGR,IID_IRowset,
			ROWSET_INTERFACE, (IUnknown**)&pIRowset))
		TESTC_(hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained,
			&rghRows), S_OK)
		TESTC_(hr = pIGR->GetRowFromHROW(NULL, rghRows[0], 
			IID_IGetSession, (IUnknown**)&pIGS), S_OK)
	}
	else
	{
		TEST2C_(hr = pIOR->OpenRowset(NULL, &(g_pTable->GetTableID()), 
			NULL, IID_IRow, 0, NULL, (IUnknown**)&pIRow), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK);
		TESTC(VerifyInterface(pIRow,IID_IGetSession,
			ROW_INTERFACE, (IUnknown**)&pIGS))
	}

	//Call GetSession and verify the correct IUnknown is returned.
	TEST2C_(hr = pIGS->GetSession(IID_IUnknown, 
		(IUnknown**)&pISessUnk), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_PROVIDER(hr == S_OK)
	TESTC(VerifyEqualInterface(pISessUnk, (IUnknown*)&Aggregate))

CLEANUP:
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pIUnkInner);
	SAFE_RELEASE(pISessUnk);
	SAFE_RELEASE(pIGS);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGR);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Release parent objects before calling GetSession.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMiscellaneous::Variation_2()
{ 
	TBEGIN
	HRESULT				hr;
	DBCOUNTITEM			cRowsObtained=0;
	HROW*				rghRows=NULL;
	IDBCreateSession*	pIDBCS=NULL;
	IOpenRowset*		pIOR=NULL;
	IGetRow*			pIGR=NULL;
	IRowset*			pIRowset=NULL;
	IGetSession*		pIGS=NULL;
	IUnknown*			pISessUnk=NULL;
	IRow*				pIRow = NULL;

	//Create a new dso and session.
	TESTC_(CreateNewDSO(NULL, IID_IDBCreateSession, 
		(IUnknown**)&pIDBCS), S_OK)
	TESTC_(hr = pIDBCS->CreateSession(NULL, IID_IOpenRowset,
		(IUnknown**)&pIOR), S_OK)

	//Open a rowset and get a row object from first row.
	TEST2C_(hr = pIOR->OpenRowset(NULL, &(g_pTable->GetTableID()), 
		NULL, IID_IGetRow, 0, NULL, (IUnknown**)&pIGR), S_OK, E_NOINTERFACE)

	if(hr == S_OK)
	{
		TESTC(VerifyInterface(pIGR,IID_IRowset,
			ROWSET_INTERFACE, (IUnknown**)&pIRowset))
		TESTC_(hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained,
			&rghRows), S_OK)
		TESTC_(hr = pIGR->GetRowFromHROW(NULL, rghRows[0], 
			IID_IGetSession, (IUnknown**)&pIGS), S_OK)
	}
	else
	{
		TEST2C_(hr = pIOR->OpenRowset(NULL, &(g_pTable->GetTableID()), 
			NULL, IID_IRow, 0, NULL, (IUnknown**)&pIRow), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK);
		TESTC(VerifyInterface(pIRow,IID_IGetSession,
			ROW_INTERFACE, (IUnknown**)&pIGS))
	}

	//Call GetSession and verify.
	TEST2C_(hr = pIGS->GetSession(IID_IUnknown, 
		(IUnknown**)&pISessUnk), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_PROVIDER(hr == S_OK)
	TESTC(VerifyEqualInterface(pISessUnk, pIOR))

	//Release all parent objects.
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pISessUnk);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGR);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);

	//Verify that GetSession still works.
	TESTC_(hr = pIGS->GetSession(IID_IOpenRowset, 
		(IUnknown**)&pIOR), S_OK)
	TESTC(pIOR!=NULL)
	TESTC(DefTestInterface((IOpenRowset*)pIOR))

CLEANUP:
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pISessUnk);
	SAFE_RELEASE(pIGS);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGR);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete an HROW and call GetSession.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMiscellaneous::Variation_3()
{ 
	TBEGIN
	HRESULT				hr;
	DBCOUNTITEM			cRowsObtained=0;
	ULONG				cPropSets=0;
	DBPROPSET*			rgPropSets=NULL;
	HROW*				rghRows=NULL;
	IDBCreateSession*	pIDBCS=NULL;
	IOpenRowset*		pIOR=NULL;
	IGetRow*			pIGR=NULL;
	IRowset*			pIRowset=NULL;
	IRowsetChange*		pIRowsetChange=NULL;
	IGetSession*		pIGS=NULL;
	IUnknown*			pISessUnk=NULL;

	//Create a new dso and session.
	TESTC_(CreateNewDSO(NULL, IID_IDBCreateSession, 
		(IUnknown**)&pIDBCS), S_OK)
	TESTC_(hr = pIDBCS->CreateSession(NULL, IID_IOpenRowset,
		(IUnknown**)&pIOR), S_OK)

	//Set props to get an updateable rowset.
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets) ;
	SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets, (void*)DBPROPVAL_UP_DELETE, DBTYPE_I4) ;

	//Get an updateable rowset, and get a row object from
	//it's first row.

	TEST2C_(hr = pIOR->OpenRowset(NULL, &(g_pTable->GetTableID()), 
		NULL, IID_IRowsetChange, cPropSets, rgPropSets, (IUnknown**)&pIRowsetChange), 
		S_OK, DB_E_ERRORSOCCURRED)
	TESTC_PROVIDER(hr == S_OK);

	TESTC_PROVIDER(VerifyInterface(pIRowsetChange,IID_IGetRow,
		ROWSET_INTERFACE, (IUnknown**)&pIGR))
	TESTC(VerifyInterface(pIGR,IID_IRowset,
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))
	TESTC_(hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained,
		&rghRows), S_OK)
	TESTC_(hr = pIGR->GetRowFromHROW(NULL, rghRows[0], 
		IID_IGetSession, (IUnknown**)&pIGS), S_OK)

	//Call Getsession and verify.
	TEST2C_(hr = pIGS->GetSession(IID_IUnknown, 
		(IUnknown**)&pISessUnk), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_PROVIDER(hr == S_OK)
	TESTC(VerifyEqualInterface(pISessUnk, pIOR))
	SAFE_RELEASE(pISessUnk);

	//Delete the row.
	TESTC_(hr=pIRowsetChange->DeleteRows(NULL, 1, rghRows, NULL), S_OK)
	SAFE_FREE(rghRows);

	//Call GetSession again and verify.
	TESTC_(hr = pIGS->GetSession(IID_IUnknown, 
		(IUnknown**)&pISessUnk), S_OK)
	TESTC(pISessUnk!=NULL)
	TESTC(VerifyEqualInterface(pISessUnk, pIOR))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pISessUnk);
	SAFE_RELEASE(pIGS);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIGR);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCMiscellaneous::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSessionObject::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCRowZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowZombie - Zombie state tests
//| Created:  	10/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowZombie::Init()
{ 
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCTransactions::Init())
	// }}
	{
		//This is a mandatory interface, it should always succeed
		if(!RegisterInterface(ROW_INTERFACE, IID_IGetSession, 0, NULL))
			return TEST_SKIPPED;
		else
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
// @mfunc Abort with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowZombie::Variation_1()
{ 
	return TestTxnRow(FALSE, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowZombie::Variation_2()
{ 
	return TestTxnRow(FALSE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowZombie::Variation_3()
{ 
	return TestTxnRow(TRUE, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowZombie::Variation_4()
{ 
	return TestTxnRow(TRUE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRowZombie::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCTransactions::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END
