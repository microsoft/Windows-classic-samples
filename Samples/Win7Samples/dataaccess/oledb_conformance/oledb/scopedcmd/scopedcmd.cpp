//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module SCOPEDCMD.CPP | SCOPEDCMD source file for all test modules.
//
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "modstandard.hpp"
#include "SCOPEDCMD.h"
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x6bf08870, 0x798f, 0x11d2, { 0xaf, 0x40, 0x00, 0xc0, 0x4f, 0x78, 0x29, 0x26} };
DECLARE_MODULE_NAME("ScopedCmd");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Scoped Command Test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTree *		g_pTree = NULL;
IUnknown *	pUnkSession = NULL;	
BOOL		g_fResetIniFile		= FALSE;
CTable			*g_pConfProvTable2	= NULL;
CTree			*g_pConfProvTree2	= NULL;
const CLSID		CLSID_ConfProv		= {0xb2a233c1, 0x5b20, 0x11d0, {0x84, 0x18, 0x0, 0xaa, 0x00, 0x3f, 0xd, 0xd4}};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CScopedCmd
// @base public | CCommandObject
//
class CScopedCmd : public CSessionObject
{
protected:

	// Root Row Object
	CRowObject *		m_pCRootRowObj;

	// IDBCreateCommand object
	IDBCreateCommand *	m_pIDBCreateCommand;

	// Keeps track of the Root Row Object's Session
	IUnknown * 			m_pIRowSession;

	// Tree Object
	CTree *				m_pTree;

	// Alter way IBindResource is obtained.
	BOOL				m_fUseSessionBinder;

	// @cmember Constructor
	CScopedCmd(LPWSTR wszTestCaseName) : CSessionObject (wszTestCaseName) {	};

	// @cmember Constructor
	virtual ~CScopedCmd()	{};

	//@cmember Init
	BOOL Init();
	//@cmember Terminate
	BOOL Terminate();
	//@cmember Get tree member's root row object
	IRow * GetRootRow();
	//@cmember Recurse through every level in m_pTree
	BOOL VerifyRowAndChildren(CRowObject *pCRowObject, CSchema* pSchema);
	//@cmember Create a command object
	BOOL CreateCommand
	(
		REFIID iid,						
		IUnknown** ppIUnknownCommand,		
		IUnknown*	pIUnknownSession = NULL	
	);
	//@cmember Set a scoped query text (e.g. select * from scope())
	HRESULT SetScopedCmdTxt(IUnknown* pIUnknownCommand, EQUERY eQuery);
	//@cmember Set command text
	HRESULT SetCmdTxt(IUnknown* pIUnknownCommand, WCHAR *pwszCmd);
	//@cmember Prepares a command
	HRESULT PrepareCmd(IUnknown* pIUnknownCommand, ULONG cExpectedRuns);
	//@cmember UnPrepares a command
	HRESULT UnPrepareCmd(IUnknown* pIUnknownCommand);
	//@cmember Checks session object against Root Row Session
	BOOL VerifyRowSession(IUnknown* pIUnknownSession);
	//@cmember Checks equality of two command objects
	BOOL VerifyEqualCommands(IUnknown* pIUnkCmd1, IUnknown* pIUnkCmd2);
	//@cmember Iterates through Session interfaces
	HRESULT TestGetDBSession(ICommand* pICmd);
	//@cmember Checks GetDBSession for multiple commands
	HRESULT TestMultipleCommands(ULONG cCmds);
	//@cmember Checks rowset creation using cmds from mult rows
	BOOL TestMultipleRowObjects(ULONG cRows);

public:
	//@cmember Alter way the tests obtains the IBindResource interface
	virtual void SetTestCaseParam(BOOL fUseSessionBinder)
	{
		m_fUseSessionBinder = fUseSessionBinder;
	}

	// Command/Execute thread variations
	static ULONG WINAPI Thread_Cancel(LPVOID pv);
	static ULONG WINAPI Thread_Execute(LPVOID pv);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CScopedCmd::Init()
{
	IRow *	pIRootRow;
	BOOL	fPass = FALSE;

  	if (COLEDB::Init())	
	{	
		SetDBSession(pUnkSession);

		m_pCRootRowObj = new CRowObject;
		TESTC(m_pCRootRowObj != NULL);

		pIRootRow = GetRootRow();
		TESTC_(m_pCRootRowObj->SetRowObject(pIRootRow),S_OK);

		fPass = TRUE;
	}  

CLEANUP:

	SAFE_RELEASE(pIRootRow);

	return fPass; 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CScopedCmd::Terminate()
{
	ReleaseDBSession();
	SAFE_DELETE(m_pCRootRowObj);
	return(COLEDB::Terminate());
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember GetRootRow
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
IRow * CScopedCmd::GetRootRow()
{
	IRow *			pIRow = NULL;
	IBindResource *	pIBindResource = NULL;
	WCHAR *			pwszRowURL = NULL;
	DBBINDSTATUS	dwBindStatus;
	
	ASSERT(g_pTree);

	pwszRowURL = g_pTree->GetRootURL();
	TESTC(NULL != pwszRowURL);

	if( m_fUseSessionBinder )
	{
		TESTC_(GetSessionObject(IID_IBindResource, (IUnknown **)&pIBindResource), S_OK);
	}
	else
	{
		pIBindResource = GetModInfo()->GetRootBinder();
		SAFE_ADDREF(pIBindResource);
	}

	TESTC(NULL != pIBindResource);

	TESTC_(pIBindResource->Bind(NULL, pwszRowURL, DBBINDURLFLAG_READ, DBGUID_ROW,
		IID_IRow, NULL, NULL, &dwBindStatus, (IUnknown **)&pIRow), S_OK);
	TESTC(dwBindStatus == DBBINDURLSTATUS_S_OK);

CLEANUP:

	SAFE_RELEASE(pIBindResource);

	return pIRow;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember VerifyRowAndChildren
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CScopedCmd::VerifyRowAndChildren(CRowObject* pCParentRowObject, CSchema* pSchema)
{
	BOOL			fSuccess = TEST_FAIL;
	IRowset *		pIRowset = NULL;
	IColumnsInfo *	pIColumnsInfo = NULL;
	DBCOUNTITEM		cRowsetCols = 0;
	DBCOLUMNINFO *	rgRowsetColInfo = NULL;
	DBORDINAL *		rgRowsetColOrdinals = NULL;
	WCHAR *			pStringsBuffer = NULL;
	HROW *			rghRows = NULL;
	HRESULT			hr;
	CRowObject		ChildRowObj;
	ULONG_PTR		cIter;
	ULONG_PTR		ulRow = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	DBREFCOUNT		RefCount = 0;
	DBROWSTATUS		RowStatus = DBROWSTATUS_E_FAIL;

	if (pCParentRowObject->pIDBCreateCommand() == NULL)
	{
		fSuccess = TEST_SKIPPED;
		odtLog << L"Row object doesn't support scoped commands." << ENDL;
		goto CLEANUP;
	}

	TESTC_(hr = pCParentRowObject->ExecuteCommand
									(
									SHALLOW_SCOPED_SELECT,
									IID_IRowset,	
									0,
									NULL,
									(IUnknown **)&pIRowset
									), S_OK);	
	TESTC(pIRowset != NULL);

	TESTC(VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, 
			(IUnknown **)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cRowsetCols, &rgRowsetColInfo, &pStringsBuffer), S_OK);

	rgRowsetColOrdinals = (DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * cRowsetCols);
	for(cIter = 0; cIter < cRowsetCols; cIter++)
	{
		rgRowsetColOrdinals[cIter] = rgRowsetColInfo[cIter].iOrdinal;
	}

	hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows);

	if (hr == DB_S_ENDOFROWSET)
	{
		// leaf node
		fSuccess = TEST_PASS;
		goto CLEANUP;
	}
	
	TESTC_(hr, S_OK);

	do
	{
		TESTC(cRowsObtained == 1);
		TESTC(pSchema != NULL);

		TESTC_(ChildRowObj.CreateRowObject(pIRowset, rghRows[0]),S_OK)

		TESTC(ChildRowObj.VerifyGetColumns(ulRow, pSchema,
			UPDATEABLE_NONINDEX_COLS_BOUND | USE_COLS_TO_BIND_ARRAY, NO_BLOB_COLS,
			FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cRowsetCols, rgRowsetColOrdinals));
		
		m_pTree->MoveDownToChildNode(ulRow);
		TESTC(VerifyRowAndChildren(&ChildRowObj,reinterpret_cast<CSchema*>(m_pTree->GetCurrentSchema())));		
		m_pTree->ReturnFromChildNode();

		ChildRowObj.ReleaseRowObject();
		
		TESTC_(hr =pIRowset->ReleaseRows(1, rghRows, NULL, &RefCount, &RowStatus), S_OK);
		COMPARE(RefCount, 0);
		COMPARE(RowStatus, DBROWSTATUS_S_OK); 
		SAFE_FREE(rghRows);

		ulRow++;
		hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows);
	} while (hr == S_OK);

	TESTC_(hr, DB_S_ENDOFROWSET);
	fSuccess = TEST_PASS;

CLEANUP:

	SAFE_FREE(rgRowsetColOrdinals);
	SAFE_FREE(rgRowsetColInfo);
	SAFE_FREE(pStringsBuffer);

	SAFE_FREE(rghRows);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRowset)

	return fSuccess;
}


//-------------------------------------------------------------------
// CreateCommand
//--------------------------------------------------------------------
BOOL CScopedCmd::CreateCommand
(
	REFIID iid,						// [OUT] IID for command
	IUnknown** ppIUnknownCommand,	// [OUT] command pointer
	IUnknown*	pIUnknownSessionRow	// [IN] session object
)
{
	BOOL				fResult = FALSE;
	IDBCreateCommand*	pSession = NULL;

	ASSERT(ppIUnknownCommand);

	// If no session specified, just use m_pIDBCreateCommand
	if(pIUnknownSessionRow)
	{
		TESTC(VerifyInterface(pIUnknownSessionRow, IID_IDBCreateCommand,
									SESSION_INTERFACE, (IUnknown**)&pSession));
		TESTC_(pSession->CreateCommand(NULL,iid,ppIUnknownCommand),S_OK);
	}
	else
	{
		TESTC_(m_pIDBCreateCommand->CreateCommand(NULL,iid,ppIUnknownCommand),S_OK);		
	}
	
	fResult = TRUE;

CLEANUP:
	
	SAFE_RELEASE(pSession);
	return fResult;
}


//-------------------------------------------------------------------
// SetScopedCmdText
//--------------------------------------------------------------------
HRESULT CScopedCmd::SetScopedCmdTxt(IUnknown* pIUnknownCommand, EQUERY eQuery)
{
	HRESULT			hr = E_FAIL;
	WCHAR*			pwszCmd = NULL;
	ICommandText*	pICommandText=NULL;

	ASSERT(pIUnknownCommand);

	// Get commandtext interface
	if (!VerifyInterface(pIUnknownCommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	pwszCmd = FetchRowScopedQuery(eQuery);
	TESTC(pwszCmd != NULL);

	// Set text 
	hr = pICommandText->SetCommandText(DBGUID_DEFAULT,pwszCmd);

CLEANUP:

	SAFE_RELEASE(pICommandText);
	return hr;
}


//-------------------------------------------------------------------
// SetCmdText
//--------------------------------------------------------------------
HRESULT CScopedCmd::SetCmdTxt(IUnknown* pIUnknownCommand, WCHAR *pwszCmd)
{
	HRESULT			hr = E_FAIL;
	ICommandText*	pICommandText=NULL;

	ASSERT(pIUnknownCommand);

	// Get commandtext interface
	if (!VerifyInterface(pIUnknownCommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}	

	// Set text 
	hr = pICommandText->SetCommandText(DBGUID_DEFAULT,pwszCmd);

CLEANUP:

	SAFE_RELEASE(pICommandText);
	return hr;
}


//-------------------------------------------------------------------
// PrepareCmd
//-------------------------------------------------------------------
HRESULT CScopedCmd::PrepareCmd(IUnknown* pIUnknownCommand, ULONG cExpectedRuns)
{
	HRESULT				hr = E_FAIL;
	ICommandPrepare*	pICommandPrepare=NULL;

	ASSERT(pIUnknownCommand);

	// Get commandtext interface
	if (!VerifyInterface(pIUnknownCommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&pICommandPrepare))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}	

	// Set text 
	hr = pICommandPrepare->Prepare(cExpectedRuns);

CLEANUP:

	SAFE_RELEASE(pICommandPrepare);
	return hr;
}


//-------------------------------------------------------------------
// UnPrepareCmd
//-------------------------------------------------------------------
HRESULT CScopedCmd::UnPrepareCmd(IUnknown* pIUnknownCommand)
{
	HRESULT				hr = E_FAIL;
	ICommandPrepare*	pICommandPrepare=NULL;

	ASSERT(pIUnknownCommand);

	// Get commandtext interface
	if (!VerifyInterface(pIUnknownCommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&pICommandPrepare))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}	

	// Set text 
	hr = pICommandPrepare->Unprepare();

CLEANUP:

	SAFE_RELEASE(pICommandPrepare);
	return hr;
}


//--------------------------------------------------------------------
// VerifyRowSession
//
// Checks session object against Root Row Session
//--------------------------------------------------------------------
BOOL CScopedCmd::VerifyRowSession(IUnknown* pIUnknownSession)
{
	BOOL	fSame = FALSE;

	TESTC(VerifyEqualInterface(pIUnknownSession, m_pIRowSession));
	TESTC(DefaultObjectTesting(pIUnknownSession, SESSION_INTERFACE));

	fSame = TRUE;

CLEANUP:
	return fSame;
}


//--------------------------------------------------------------------
// VerifyRowSession
//
// Checks equality of two commands
//--------------------------------------------------------------------
BOOL CScopedCmd::VerifyEqualCommands
(
IUnknown* pIUnknownCmd1,
IUnknown* pIUnknownCmd2
)
{
	BOOL	fSame = FALSE;

	TESTC(VerifyEqualInterface(pIUnknownCmd1, pIUnknownCmd2));
	TESTC(DefaultObjectTesting(pIUnknownCmd1, COMMAND_INTERFACE));

	fSame = TRUE;

CLEANUP:
	return fSame;
}


//--------------------------------------------------------------------
// CScopedCmd::TestGetDBSession
//
//--------------------------------------------------------------------
HRESULT CScopedCmd::TestGetDBSession(ICommand* pICmd)
{
	HRESULT			hr;
	IUnknown*		pIUnknown		= NULL;	
	ULONG			i				= 0;
	ULONG			cSessionIIDs	= 0;
	INTERFACEMAP*	rgSessionIIDs	= NULL;

	//Obtain the Session Interfaces..
	GetInterfaceArray(SESSION_INTERFACE, &cSessionIIDs, &rgSessionIIDs);

	//For every [MANDATORY] interface, try to get the parent Session object...
	for(i=0; i<cSessionIIDs; i++)
	{
		//ICommand::GetDBSession
		hr = pICmd->GetDBSession(*rgSessionIIDs[i].pIID, (IUnknown**)&pIUnknown);
		
		if (hr == DB_E_NOSOURCEOBJECT || hr == S_FALSE)
		{
			TWARNING(L"This command does not have a session");
		}

		//Determine results
		if(rgSessionIIDs[i].fMandatory)
		{
			//[MANDATORY]
			if(hr!=S_OK && hr!=DB_E_NOSOURCEOBJECT && hr!=S_FALSE)
			{
				CHECK(hr, S_OK);
				TOUTPUT_(L"ERROR: Interface Incorrect for " << GetInterfaceName(*rgSessionIIDs[i].pIID) << "\n");
			}
			// Match against what we know to be the command's session
			VerifyRowSession(pIUnknown);
		}
		else
		{
			//[OPTIONAL]
			if(hr!=S_OK && hr!=DB_E_NOSOURCEOBJECT && hr!=S_FALSE && hr!=E_NOINTERFACE)
			{
				CHECK(hr, S_OK);
				TOUTPUT_(L"ERROR: Interface Incorrect for " << GetInterfaceName(*rgSessionIIDs[i].pIID) << "\n");
			}
		}
		SAFE_RELEASE(pIUnknown);
	}

	return S_OK;
} 


//--------------------------------------------------------------------
// CScopedCmd::TestMultipleCommands
//
//--------------------------------------------------------------------
HRESULT CScopedCmd::TestMultipleCommands(ULONG cCmds)
{
	ICommand**	rgpICommands = NULL;
	IUnknown*	pIUnknown = NULL;
	ULONG		ulCmd;

	//Create cCmds number of commands
	SAFE_ALLOC(rgpICommands, ICommand*, cCmds);
	memset(rgpICommands, 0, cCmds * sizeof(ICommand*));
		

	for(ulCmd=0; ulCmd<cCmds; ulCmd++)
	{
		//Create the Command
		TESTC(CreateCommand(IID_ICommand, (IUnknown **)&(rgpICommands[ulCmd])));

		TESTC(rgpICommands[ulCmd] != NULL);		
		TEST3C_(rgpICommands[ulCmd]->GetDBSession(IID_IUnknown, &pIUnknown), S_OK, S_FALSE, DB_E_NOSOURCEOBJECT);
		if(pIUnknown)
		{
			//Make sure its returning the original object
			TESTC(VerifyEqualInterface(pIUnknown, m_pIRowSession));
		}
		SAFE_RELEASE(pIUnknown);
	}			
	
CLEANUP:

	for(ulCmd=0; ulCmd<cCmds; ulCmd++)
		SAFE_RELEASE(rgpICommands[ulCmd]);

	SAFE_RELEASE(pIUnknown);
	SAFE_FREE(rgpICommands);

	return S_OK;
}


//--------------------------------------------------------------------
// CScopedCmd::TestMultipleRowObjects
//
//--------------------------------------------------------------------
BOOL CScopedCmd::TestMultipleRowObjects(ULONG cRows)
{
	CRowObject**	rgpCRowObjects = NULL;
	IUnknown*		pIUnknown = NULL;
	IRowset*		pIRowset = NULL;
	IRowset**		rgpIRowsets = NULL;
	HROW*			rghRows = NULL;
	DBCOUNTITEM		cRowsCreated = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	ULONG_PTR		ulRow = 0;
	BOOL			fSuccess = TEST_FAIL;

	// Looks at the Root Row's rowset.
	// Try to instantiate up to "cRows"# of Row objects 
	SAFE_ALLOC(rgpCRowObjects, CRowObject*, cRows);
	SAFE_ALLOC(rgpIRowsets, IRowset*, cRows);
	memset(rgpCRowObjects, 0, cRows * sizeof(CRowObject*));
	memset(rgpIRowsets, 0, cRows * sizeof(IRowset*));

	TESTC_(m_hr = m_pCRootRowObj->ExecuteCommand
									(
									SHALLOW_SCOPED_SELECT,
									IID_IRowset,	
									0,
									NULL,
									(IUnknown **)&pIRowset
									), S_OK);	
	TESTC(pIRowset != NULL);

	while(cRowsCreated<cRows && 
		  pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows) == S_OK)
	{
		COMPARE(cRowsObtained,1);

		rgpCRowObjects[cRowsCreated] = new CRowObject;
		TESTC_(rgpCRowObjects[cRowsCreated]->CreateRowObject(pIRowset, rghRows[0]),S_OK);

		SAFE_FREE(rghRows);
		cRowsCreated++;
	}
	
	if (cRowsCreated == 0)
	{
		CHECK(m_hr, DB_S_ENDOFROWSET);
		fSuccess = TEST_SKIPPED;
		goto CLEANUP;
	}
		
	//Create a command for each Row Object we have
	for(ulRow=0; ulRow<cRowsCreated; ulRow++)
	{	
		TESTC_(m_hr = rgpCRowObjects[ulRow]->ExecuteCommand
								(
								SHALLOW_SCOPED_SELECT,
								IID_IRowset,	
								0,
								NULL,
								(IUnknown **)&(rgpIRowsets[ulRow])
								), S_OK);

		TESTC(DefaultObjectTesting(rgpIRowsets[ulRow],ROWSET_INTERFACE));
	}			
	
	fSuccess = TEST_PASS;

CLEANUP:

	for(ulRow=0; ulRow<cRowsCreated; ulRow++)
	{
		SAFE_RELEASE(rgpIRowsets[ulRow]);
		SAFE_DELETE(rgpCRowObjects[ulRow]);

	}

	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(rgpIRowsets);
	SAFE_FREE(rgpCRowObjects)

	return fSuccess;
}


//--------------------------------------------------------------------
// Execute
//
//--------------------------------------------------------------------
ULONG CScopedCmd::Thread_Execute(LPVOID pv)
{
	THREAD_BEGIN
	ASSERT(pv);

	//Thread Stack Variables
	CScopedCmd*	pThis				= (CScopedCmd*)THREAD_FUNC;
	ICommand*	pICommand			= (ICommand *)THREAD_ARG1;
	HRESULT		hr					= *(HRESULT *)THREAD_ARG2;	// expected result
	HRESULT		hrOptionalResult	= *(HRESULT *)THREAD_ARG3;	// Alternate expected result
	
	//Local Variables
	IRowset*	pIRowset = NULL;
	DBROWCOUNT	cRowsAffected = -111;
	HRESULT		hrReturnResult = E_FAIL;

	ThreadSwitch(); //Let the other thread(s) catch up	
	hrReturnResult = pICommand->Execute(NULL,IID_IRowset,NULL,&cRowsAffected,(IUnknown **)&pIRowset);

	TEST2C_(hrReturnResult, hr, hrOptionalResult);		
	if (hrReturnResult == S_OK)
	{
		TESTC(pIRowset != NULL);
	}
	else // DB_E_CANCELED
	{
		TESTC(pIRowset == NULL);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	
	SAFE_RELEASE(pIRowset);
	THREAD_END(hrReturnResult);
}


//--------------------------------------------------------------------
// Cancel
//
//--------------------------------------------------------------------
ULONG CScopedCmd::Thread_Cancel(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hrReturnResult;
	ASSERT(pv);

	//Thread Stack Variables
	CScopedCmd*	pThis				= (CScopedCmd*)THREAD_FUNC;
	ICommand*	pICommand			= (ICommand *)THREAD_ARG1;
	HRESULT		hr					= *(HRESULT *)THREAD_ARG2;
	HRESULT		hrOptionalResult	= *(HRESULT *)THREAD_ARG3;

	ThreadSwitch(); //Let the other thread(s) catch up
	TEST2C_(hrReturnResult = pICommand->Cancel(), hr, hrOptionalResult);	

	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	THREAD_END(hrReturnResult);
}


// {{ TCW_TEST_CASE_MAP(Traversal)
//*-----------------------------------------------------------------------
// @class Navigate a hierarchy
//
class Traversal : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Traversal,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Complete traversal
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Traversal)
#define THE_CLASS Traversal
BEG_TEST_CASE(Traversal, CScopedCmd, L"Navigate a hierarchy")
	TEST_VARIATION(1, 		L"Complete traversal")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Aggregation)
//*-----------------------------------------------------------------------
// @class Test Aggregation cases
//
class Aggregation : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Aggregation,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Test pIUnkOuter != NULL and refiid != IID_IUnknown
	int Variation_1();
	// @cmember ::Execute on row scoped command with pIUnkOuter != NULL and refiid!=IID_IUnknown
	int Variation_2();
	// @cmember Aggregate returned command and verify aggregation
	int Variation_3();
	// @cmember Aggregate returned command, get rowset and call GetSpecification
	int Variation_4();
	// @cmember Create command, aggregate returned rowset and verify agg
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Aggregation)
#define THE_CLASS Aggregation
BEG_TEST_CASE(Aggregation, CScopedCmd, L"Test Aggregation cases")
	TEST_VARIATION(1, 		L"Test pIUnkOuter != NULL and refiid != IID_IUnknown")
	TEST_VARIATION(2, 		L"::Execute on row scoped command with pIUnkOuter != NULL and refiid!=IID_IUnknown")
	TEST_VARIATION(3, 		L"Aggregate returned command and verify aggregation")
	TEST_VARIATION(4, 		L"Aggregate returned command, get rowset and call GetSpecification")
	TEST_VARIATION(5, 		L"Create command, aggregate returned rowset and verify agg")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CCancel)
//*-----------------------------------------------------------------------
// @class General cases
//
class CCancel : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CCancel,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Before and After executing
	int Variation_1();
	// @cmember Two selects cancel  during execution
	int Variation_2();
	// @cmember Two cmds, cancel 1 during, cancel 1 after
	int Variation_3();
	// @cmember Two cmds, cancel 1 before, cancel 1 during
	int Variation_4();
	// @cmember 1 cmd, cancel before execution
	int Variation_5();
	// @cmember 1 cmd obj, cancel after execution
	int Variation_6();
	// @cmember 1 cmd, execute, cancel, cancel
	int Variation_7();
	// @cmember 1 cmd, no query set, one cancel per thrd
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CCancel)
#define THE_CLASS CCancel
BEG_TEST_CASE(CCancel, CScopedCmd, L"General cases")
	TEST_VARIATION(1, 		L"Before and After executing")
	TEST_VARIATION(2, 		L"Two selects cancel  during execution")
	TEST_VARIATION(3, 		L"Two cmds, cancel 1 during, cancel 1 after")
	TEST_VARIATION(4, 		L"Two cmds, cancel 1 before, cancel 1 during")
	TEST_VARIATION(5, 		L"1 cmd, cancel before execution")
	TEST_VARIATION(6, 		L"1 cmd obj, cancel after execution")
	TEST_VARIATION(7, 		L"1 cmd, execute, cancel, cancel")
	TEST_VARIATION(8, 		L"1 cmd, no query set, one cancel per thrd")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CDBSession)
//*-----------------------------------------------------------------------
// @class Test GetDBSession method
//
class CDBSession : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CDBSession,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IRowsetInfo::GetSpecification
	int Variation_1();
	// @cmember Verify GetDBSession on non executed command
	int Variation_2();
	// @cmember E_NOINTERFACE, dso iid, valid ptr
	int Variation_3();
	// @cmember E_NOINTERFACE, iid_null, valid ptr
	int Variation_4();
	// @cmember E_INVALIDARG, valid session, ptr=NULL
	int Variation_5();
	// @cmember E_NOINTERFACE, row iid, valid ptr
	int Variation_6();
	// @cmember Multiple Command Objects
	int Variation_7();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CDBSession)
#define THE_CLASS CDBSession
BEG_TEST_CASE(CDBSession, CScopedCmd, L"Test GetDBSession method")
	TEST_VARIATION(1, 		L"IRowsetInfo::GetSpecification")
	TEST_VARIATION(2, 		L"Verify GetDBSession on non executed command")
	TEST_VARIATION(3, 		L"E_NOINTERFACE, dso iid, valid ptr")
	TEST_VARIATION(4, 		L"E_NOINTERFACE, iid_null, valid ptr")
	TEST_VARIATION(5, 		L"E_INVALIDARG, valid session, ptr=NULL")
	TEST_VARIATION(6, 		L"E_NOINTERFACE, row iid, valid ptr")
	TEST_VARIATION(7, 		L"Multiple Command Objects")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CExecute)
//*-----------------------------------------------------------------------
// @class General Execute variations
//
class CExecute : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CExecute,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Multiple executions on same command object
	int Variation_1();
	// @cmember Multiple Commands on same Row
	int Variation_2();
	// @cmember Multilpe Row objects open
	int Variation_3();
	// @cmember Properties tests
	int Variation_4();
	// @cmember E_INVALIDARG, pcRowsAffected and pIRowset NULL
	int Variation_5();
	// @cmember Valid, iid=IRowsetLocate
	int Variation_6();
	// @cmember Valid, iid=IColumnsInfo
	int Variation_7();
	// @cmember Valid, iid=IUnknown
	int Variation_8();
	// @cmember cParam=0
	int Variation_9();
	// @cmember ICommand::Execute iid=IID_IRow
	int Variation_10();
	// @cmember ICommandText::Execute, iid=IID_IRow
	int Variation_11();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CExecute)
#define THE_CLASS CExecute
BEG_TEST_CASE(CExecute, CScopedCmd, L"General Execute variations")
	TEST_VARIATION(1, 		L"Multiple executions on same command object")
	TEST_VARIATION(2, 		L"Multiple Commands on same Row")
	TEST_VARIATION(3, 		L"Multilpe Row objects open")
	TEST_VARIATION(4, 		L"Properties tests")
	TEST_VARIATION(5, 		L"E_INVALIDARG, pcRowsAffected and pIRowset NULL")
	TEST_VARIATION(6, 		L"Valid, iid=IRowsetLocate")
	TEST_VARIATION(7, 		L"Valid, iid=IColumnsInfo")
	TEST_VARIATION(8, 		L"Valid, iid=IUnknown")
	TEST_VARIATION(9, 		L"cParam=0")
	TEST_VARIATION(10, 		L"ICommand::Execute iid=IID_IRow")
	TEST_VARIATION(11, 		L"ICommandText::Execute, iid=IID_IRow")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CPrepare)
//*-----------------------------------------------------------------------
// @class test ICommandPrepare cases
//
class CPrepare : public CScopedCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CPrepare,CScopedCmd);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Invalid, prepare empty text string
	int Variation_1();
	// @cmember Invalid, prepare after NULL ppwszCommand
	int Variation_2();
	// @cmember DB_E_NOCOMMAND, prepare before setting text
	int Variation_3();
	// @cmember DB_E_NOCOMMAND - Prepare with open rowset object
	int Variation_4();
	// @cmember S_OK - valid select
	int Variation_5();
	// @cmember S_OK - prepare after ::GetColumnInfo FAILS
	int Variation_6();
	// @cmember S_OK ::GetColumnsInfo after Prepare
	int Variation_7();
	// @cmember S_OK - Prepare, SetCommandText, GetColumnsInfo
	int Variation_8();
	// @cmember Prepare and set properties
	int Variation_11();
	// @cmember DB_E_OBJECTOPEN - unprepare with open rowset
	int Variation_12();
	// @cmember S_OK - Unprepare valid select
	int Variation_13();
	// @cmember S_OK - Unprepare and verify GetColumnsInfo fails
	int Variation_14();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CPrepare)
#define THE_CLASS CPrepare
BEG_TEST_CASE(CPrepare, CScopedCmd, L"test ICommandPrepare cases")
	TEST_VARIATION(1, 		L"Invalid, prepare empty text string")
	TEST_VARIATION(2, 		L"Invalid, prepare after NULL ppwszCommand")
	TEST_VARIATION(3, 		L"DB_E_NOCOMMAND, prepare before setting text")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND - Prepare with open rowset object")
	TEST_VARIATION(5, 		L"S_OK - valid select")
	TEST_VARIATION(6, 		L"S_OK - prepare after ::GetColumnInfo FAILS")
	TEST_VARIATION(7, 		L"S_OK ::GetColumnsInfo after Prepare")
	TEST_VARIATION(8, 		L"S_OK - Prepare, SetCommandText, GetColumnsInfo")
	TEST_VARIATION(11, 		L"Prepare and set properties")
	TEST_VARIATION(12, 		L"DB_E_OBJECTOPEN - unprepare with open rowset")
	TEST_VARIATION(13, 		L"S_OK - Unprepare valid select")
	TEST_VARIATION(14, 		L"S_OK - Unprepare and verify GetColumnsInfo fails")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

//--------------------------------------------------------------------
// @func Reinitialize the Conformance Provider by ignoring the Ini file
//  and constructing the tree - added on 04/30/2001 
//

BOOL ReInitializeConfProv(CThisTestModule* pThisTestModule)
{
	BOOL			fSuccess = FALSE;
	WCHAR*			pwszRootURL = NULL;
	WCHAR*			pwszNewURL = NULL;
	WCHAR*			pwszCmdURL = NULL;
	WCHAR*			pwszRowURL = NULL;
	WCHAR*			pwszRowQuery = NULL;

	CList<WCHAR *,WCHAR *>	NativeTypesList;
	CList<DBTYPE,DBTYPE>	ProviderTypesList;
	CCol					col;
	DBCOLUMNDESC			*rgColumnDesc			= NULL;
	DBORDINAL				cColumnDesc				= 0;
	DBORDINAL				ulIndxCol				= 0;
	BOOL					fTableExist				= FALSE;
	
	CList <CCol, CCol&>		colList;
	POSITION				pos;
	DBORDINAL				cIter=0;
	DBORDINAL				ulParentOrdinal;
	DBORDINAL				rgOrdinals[2];	// ConfProv reserves two columns for its own use


	pwszRootURL = (WCHAR *)PROVIDER_ALLOC((wcslen(L"confprov://dso/session/")+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszRootURL != NULL);
	wcscpy(pwszRootURL, L"confprov://dso/session/");
	GetModInfo()->SetRootURL(pwszRootURL);

	// Create a table.
	if (g_pConfProvTable2)
	{
		g_pConfProvTable2->DropTable();
		SAFE_DELETE(g_pConfProvTable2);
	}
	g_pConfProvTable2 = new CTable(pThisTestModule->m_pIUnknown2);
	
	g_pConfProvTable2->CreateColInfo(NativeTypesList, ProviderTypesList);	
	g_pConfProvTable2->DuplicateColList(colList);

	pos = colList.GetHeadPosition();
	TESTC(NULL != pos)
	
	cColumnDesc = 2;
	for (; pos; )
	{
		POSITION	oldPos = pos;

		col = colList.GetNext(pos);
		if (!col.GetNullable())
			colList.RemoveAt(oldPos);
		else
		{
			col.SetColNum(++cColumnDesc);
			colList.SetAt(oldPos, col);
		}
	}

	TESTC(g_pConfProvTable2->GetColWithAttr(COL_COND_AUTOINC, &ulIndxCol)); 
	// duplicate the first column - use one for index and one for values
	TESTC_(g_pConfProvTable2->GetColInfo(ulIndxCol, col), S_OK);
	col.SetColName(L"RESOURCE_PARSENAME");
	col.SetNullable(FALSE); 
	col.SetColNum(1);
	colList.AddHead(col);

	// Find a candidate for the RESOURCE_PARENTNAME columns
	for(cIter=1; cIter <= g_pConfProvTable2->CountColumnsOnTable(); cIter++)
	{
		TESTC_(g_pConfProvTable2->GetColInfo(cIter, col), S_OK);
			
		if (col.GetIsLong() == FALSE && col.GetIsFixedLength() == FALSE &&
			(col.GetProviderType() == DBTYPE_WSTR ||
			 col.GetProviderType() == DBTYPE_BSTR ||
			 col.GetProviderType() == DBTYPE_STR ))
		break;
	}
	
	// Did we find a candidate?
	TESTC(cIter < g_pConfProvTable2->CountColumnsOnTable());
	ulParentOrdinal = col.GetColNum();

	col.SetColName(L"RESOURCE_PARENTNAME");
	col.SetIsFixedLength(FALSE);
	col.SetColumnSize(200);
	col.SetColNum(2);
	colList.AddHead(col);

	g_pConfProvTable2->SetColList(colList);

	
	g_pConfProvTable2->SetBuildColumnDesc(FALSE);	// do not create ColList again
	
	cColumnDesc = g_pConfProvTable2->CountColumnsOnTable();
	g_pConfProvTable2->BuildColumnDescs(&rgColumnDesc);
	
	// make sure the first column is not autoincrementable 
	FreeProperties(&rgColumnDesc[0].cPropertySets, &rgColumnDesc[0].rgPropertySets);
	SAFE_FREE(rgColumnDesc[0].pwszTypeName);
	
	// make sure the parent column doesn't specify a type name
	SAFE_FREE(rgColumnDesc[ulParentOrdinal-1].pwszTypeName);

	g_pConfProvTable2->SetColumnDesc(rgColumnDesc, cColumnDesc);
//	TESTC_(g_pConfProvTable->CreateTable(0, cColumnDesc), S_OK);
	TESTC_(g_pConfProvTable2->CreateTable(0, 0), S_OK); // avoid creating a rowset on the last col

	// create a unique index on the two special columns
	rgOrdinals[0] = 1;
	rgOrdinals[1] = 2;
	TESTC_(g_pConfProvTable2->CreateIndex(rgOrdinals,2,UNIQUE), S_OK);

	// get the name of the created table
	// and alter the ROOT_URL.
	pwszNewURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszNewURL != NULL);
	wcscpy(pwszNewURL, pwszRootURL);
	wcscat(pwszNewURL, L"/");
	wcscat(pwszNewURL, g_pConfProvTable2->GetTableName());

	//CreateTree with one node.
	g_pConfProvTree2 = new CTree(pThisTestModule->m_pIUnknown2);
	g_pConfProvTree2->CreateTree(pwszNewURL, 1, 2);

	PROVIDER_FREE(pwszRootURL);
	pwszRootURL = g_pConfProvTree2->GetRootURL();
	TESTC(pwszRootURL && wcslen(pwszRootURL)>1)

	pwszCmdURL = (WCHAR *)PROVIDER_ALLOC((wcslen(L"confprov://dso/session/")+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszCmdURL != NULL);
	wcscpy(pwszCmdURL, L"confprov://dso/session/");
	wcscat(pwszCmdURL, L"select * from ");
	wcscat(pwszCmdURL, g_pConfProvTable2->GetTableName());

	pwszRowURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszRowURL != NULL);
	wcscpy(pwszRowURL, pwszRootURL);
	wcscat(pwszRowURL, L"/0");

	GetModInfo()->SetRootURL(pwszRowURL);

	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(DATASOURCE_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(SESSION_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(ROWSET_INTERFACE, pwszNewURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(ROW_INTERFACE, pwszRowURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(STREAM_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(COMMAND_INTERFACE, pwszCmdURL));

	// Override the default Row Scoped Command Query
	pwszRowQuery = (WCHAR *)PROVIDER_ALLOC((wcslen(g_pConfProvTable2->GetTableName())+wcslen(wszSELECT_ALLFROMTBL)+1)*sizeof(WCHAR));
	TESTC(pwszRowQuery != NULL);
	swprintf(pwszRowQuery, wszSELECT_ALLFROMTBL, g_pConfProvTable2->GetTableName());
	
	GetModInfo()->SetRowScopedQuery(pwszRowQuery);

	fSuccess = TRUE;

CLEANUP:
	PROVIDER_FREE(pwszRowQuery);	
	PROVIDER_FREE(pwszNewURL);
	PROVIDER_FREE(pwszCmdURL);
	PROVIDER_FREE(pwszRowURL);
	PROVIDER_FREE(pwszRootURL);

	return fSuccess;	
}


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	ULONG_PTR				ulOleObj				= 0; 
	BOOL					fConfProv				= FALSE;

	// Get connection and session objects
	if (!ModuleCreateDBSession(pThisTestModule))
	{
		odtLog << L"Fail to initialize\n";
		return FALSE;
	}	
	//Check if provider supports direct binding. If the provider doesn't support 
	//direct binding then we skip all test cases. As per the OLE DB spec if the provider sets
	//DBPROPVAL_OO_DIRECTBIND value of the DBPROP_OLE_OBJECTS, then the consumer
	//can assume that direct binding is supported.

	TESTC_PROVIDER(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		(IUnknown*)pThisTestModule->m_pIUnknown, &ulOleObj) &&
		((ulOleObj & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND));

		if(	!VerifyInterface((IUnknown *)pThisTestModule->m_pIUnknown2, IID_IUnknown,
				SESSION_INTERFACE, &pUnkSession) )
				return FALSE;

		if(CLSID_ConfProv == GetModInfo()->GetProviderCLSID()) 
		{
			fConfProv = TRUE;
		}

	// Added on April 30th 2001 so that test does not fail if ini file is used

		if(GetModInfo()->GetFileName())
	{	odtLog << L"WARNING: Test does not support using ini file. \n";
		if(fConfProv)
		{
			odtLog << L" Resetting to ignore ini file. \n";
			odtLog << L" This test will construct internally the table and tree based on the ROOT_URL : confprov://dso/session . \n";
			GetModInfo()->ResetIniFile();
			g_fResetIniFile = TRUE;
			ReInitializeConfProv(pThisTestModule);
		}
		else
		{
			odtLog << L"Skipping all test cases.\n";		
			TESTB = TEST_SKIPPED;
			goto CLEANUP;
		}
	}

		// Create a hierarchy
		g_pTree = new CTree((IUnknown *)pThisTestModule->m_pIUnknown2,
							(LPWSTR)gwszModuleName);

		if (!g_pTree) 
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		if (NULL == GetModInfo()->GetRootURL()  && !g_fResetIniFile)
		{
			TWARNING(L"A Root URL must be specified in the initialization string.");
			TESTW(FALSE);
			return TEST_SKIPPED;
		}

		if (NULL == GetModInfo()->GetRowScopedQuery()  && !g_fResetIniFile)
		{
			TWARNING(L"A Row scoped query must be specified to run the test.");
			TESTW(FALSE);
			return TEST_SKIPPED;
		}

		// Create a tree								
		if (FAILED(g_pTree->CreateTree(GetModInfo()->GetRootURL(),2,8)))
			return FALSE;
					
		// If we made it this far, everything has succeeded
		return TRUE;
		
	
	CLEANUP:
	
	TRETURN
	//return FALSE;
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
	SAFE_RELEASE(pUnkSession);

	// Drop the tree created in the ModuleInit
	if (g_pTree)
	{
		g_pTree->DestroyTree();
		SAFE_DELETE(g_pTree);
	}

	if (g_pConfProvTable2)
	{
		g_pConfProvTable2->DropTable();
		SAFE_DELETE(g_pConfProvTable2);
	}

	if (g_pConfProvTree2)
	{
		g_pConfProvTree2->DestroyTree();
		SAFE_DELETE(g_pConfProvTree2);
	}

	return ModuleReleaseDBSession(pThisTestModule);
}	



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()



////////////////////////////////////////////////////////////////////////
// Cloning loginc
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

	
COPY_TEST_CASE(Traversal_RootBind, Traversal)

COPY_TEST_CASE(Traversal_SessBind, Traversal)

COPY_TEST_CASE(Aggregation_RootBind, Aggregation)

COPY_TEST_CASE(Aggregation_SessBind, Aggregation)

COPY_TEST_CASE(CCancel_RootBind, CCancel)

COPY_TEST_CASE(CCancel_SessBind, CCancel)

COPY_TEST_CASE(CDBSession_RootBind, CDBSession)

COPY_TEST_CASE(CDBSession_SessBind, CDBSession)

COPY_TEST_CASE(CExecute_RootBind, CExecute)

COPY_TEST_CASE(CExecute_SessBind, CExecute)

COPY_TEST_CASE(CPrepare_RootBind, CPrepare)

COPY_TEST_CASE(CPrepare_SessBind, CPrepare)

// }} END_DECLARE_TEST_CASES()


//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a paraemter which changes the behvior.
//So we make LTM think there are 15 cases in here with different names, but in
//reality we only have to maintain code for the unique cases.  This way we can
//easily get testing of other storage interfaces, without maintaining 4 different
//tests with almost identical code...

#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(6, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Traversal)
	TEST_CASE(2, Aggregation)
	TEST_CASE(3, CCancel)
	TEST_CASE(4, CDBSession)
	TEST_CASE(5, CExecute)
	TEST_CASE(6, CPrepare)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(12, ThisModule, gwszModuleDescrip)	

	// Clone 1 for Traversal
	TEST_CASE_WITH_PARAM(1, Traversal_RootBind, ROOTBINDER)	

	// Clone 2 for Traversal
	TEST_CASE_WITH_PARAM(2, Traversal_SessBind, SESSIONBINDER)	

	// Clone 1 for Aggregation
	TEST_CASE_WITH_PARAM(3, Aggregation_RootBind, ROOTBINDER)

	// Clone 2 for Aggregation
	TEST_CASE_WITH_PARAM(4, Aggregation_SessBind, SESSIONBINDER)

	// Clone 1 for CCancel
	TEST_CASE_WITH_PARAM(5, CCancel_RootBind, ROOTBINDER)

	// Clone 2 for CCancel
	TEST_CASE_WITH_PARAM(6, CCancel_SessBind, SESSIONBINDER)
	
	// Clone 1 for CDBSession
	TEST_CASE_WITH_PARAM(7, CDBSession_RootBind, ROOTBINDER)

	// Clone 2 for CDBSession
	TEST_CASE_WITH_PARAM(8, CDBSession_SessBind, SESSIONBINDER)

	// Clone 1 for CExecute
	TEST_CASE_WITH_PARAM(9, CExecute_RootBind, ROOTBINDER)

	// Clone 2 for CExecute
	TEST_CASE_WITH_PARAM(10, CExecute_SessBind, SESSIONBINDER)

	// Clone 1 for CPrepare
	TEST_CASE_WITH_PARAM(11, CPrepare_RootBind, ROOTBINDER)

	// Clone 2 for CPrepare
	TEST_CASE_WITH_PARAM(12, CPrepare_SessBind, SESSIONBINDER)

END_TEST_MODULE()
#endif


// {{ TCW_TC_PROTOTYPE(Traversal)
//*-----------------------------------------------------------------------
//| Test Case:		Traversal - Navigate a hierarchy
//| Created:  	11/11/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Traversal::Init()
{ 
	ASSERT(g_pTree);
	if(!g_pTree)
	{
		return FALSE;
	}

	m_pTree = g_pTree;	
	m_pTree->ResetPosition();

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Complete traversal
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Traversal::Variation_1()
{ 
	// Completely traverse a tree using row scoped commands
	return VerifyRowAndChildren(m_pCRootRowObj, reinterpret_cast<CSchema*>(m_pTree->GetRootSchema()));

} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Traversal::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END






// {{ TCW_TC_PROTOTYPE(Aggregation)
//*-----------------------------------------------------------------------
//| Test Case:		Aggregation - Test Aggregation cases
//| Created:  	11/17/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Aggregation::Init()
{ 
	m_pTree = g_pTree;
	m_pTree->ResetPosition();

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 
		if (NULL == FetchRowScopedQuery(SHALLOW_SCOPED_SELECT))
		{
			odtLog << L"Unable to run this test case without a Row Scoped Query." << ENDL;
			return TEST_SKIPPED;
		}

		m_pIDBCreateCommand = m_pCRootRowObj->pIDBCreateCommand();
		if (m_pIDBCreateCommand == NULL)
			return TEST_SKIPPED;
		else
			return TRUE;
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test pIUnkOuter != NULL and refiid != IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Aggregation::Variation_1()
{ 
	CAggregate	Aggregate(m_pCRootRowObj->pIRow());
	ICommand*	pICommand = INVALID(ICommand*);

	TESTC_(m_pIDBCreateCommand->CreateCommand
									(
									&Aggregate,
									IID_ICommand,
									(IUnknown **)&pICommand
									), DB_E_NOAGGREGATION);

	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pICommand == NULL);

CLEANUP:

	if (pICommand != INVALID(ICommand*))
		SAFE_RELEASE(pICommand);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ::Execute on row scoped command with pIUnkOuter != NULL and refiid!=IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Aggregation::Variation_2()
{ 
	CAggregate Aggregate(m_pCRootRowObj->pIRow());
	ICommand*	pICommand = NULL;
	IRowset*	pIRowset = INVALID(IRowset*);

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICommand));
	TESTC_(SetScopedCmdTxt(pICommand, SHALLOW_SCOPED_SELECT),S_OK);

	TESTC_(pICommand->Execute(&Aggregate, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset), DB_E_NOAGGREGATION);
	COMPARE(Aggregate.GetRefCount(), 1);
	TEST(pIRowset == NULL);

CLEANUP:

	SAFE_RELEASE(pICommand);
	if (pIRowset != INVALID(IRowset*))
		SAFE_RELEASE(pIRowset);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Aggregate returned command and verify aggregation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Aggregation::Variation_3()
{ 
	CAggregate	Aggregate(m_pCRootRowObj->pIRow());
	IUnknown *	pUnkInner = NULL;

	m_hr = m_pIDBCreateCommand->CreateCommand
									(
									&Aggregate,
									IID_IUnknown,
									&pUnkInner
									);
	Aggregate.SetUnkInner(pUnkInner);

	if(Aggregate.VerifyAggregationQI(m_hr, IID_IConvertType))
		odtLog << L"Provider supports aggregation.";
	
	SAFE_RELEASE(pUnkInner);
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Aggregate returned command, get rowset and call GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Aggregation::Variation_4()
{ 
	CAggregate		Aggregate(m_pCRootRowObj->pIRow());
	CAggregate *	pIAggregate = NULL;
	ICommandText *	pICmdText = NULL;
	IRowsetInfo *	pIRowsetInfo = NULL;
	IUnknown *		pUnkInner = NULL;
	ULONG			ulRefCountAfter, ulRefCountBefore;

	m_hr = m_pIDBCreateCommand->CreateCommand
									(
									&Aggregate,
									IID_IUnknown,
									&pUnkInner
									);
	Aggregate.SetUnkInner(pUnkInner);

	if(Aggregate.VerifyAggregationQI(m_hr, IID_ICommandText, (IUnknown **)&pICmdText))
	{
		TESTC_(SetScopedCmdTxt(pICmdText, SHALLOW_SCOPED_SELECT),S_OK);

		ulRefCountBefore = Aggregate.GetRefCount();
		TESTC_(pICmdText->Execute(NULL, IID_IRowsetInfo, NULL, NULL, (IUnknown **)&pIRowsetInfo),S_OK);
		ulRefCountAfter = Aggregate.GetRefCount();

		TEST2C_(m_hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown **)&pIAggregate),S_OK,S_FALSE);

		if(m_hr==S_OK)
		{
			TESTC(VerifyEqualInterface(pIAggregate, pICmdText));

			//Verify the child correctly addref'd the parent outer.
			TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
		}
		else
		{
			TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
		}
	}

CLEANUP:

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICmdText);
	SAFE_RELEASE(pUnkInner);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create command, aggregate returned rowset and verify agg
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Aggregation::Variation_5()
{
    CAggregate	Aggregate(m_pCRootRowObj->pIRow());
	ICommand *	pICmd1 = NULL;
	IUnknown *	pUnkInner = NULL;

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	m_hr = pICmd1->Execute(&Aggregate, IID_IUnknown, NULL, NULL, &pUnkInner);
	Aggregate.SetUnkInner(pUnkInner);
	
	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(m_hr, IID_IColumnsInfo));

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
    SAFE_RELEASE(pUnkInner);
    
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Aggregation::Terminate()
{ 
	m_pIDBCreateCommand = NULL;
	m_pTree = NULL;

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(CCancel)
//*-----------------------------------------------------------------------
//| Test Case:		CCancel - General cases
//| Created:  	11/17/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCancel::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 		
		if (NULL == FetchRowScopedQuery(SHALLOW_SCOPED_SELECT))
		{
			odtLog << L"Unable to run this test case without a Row Scoped Query." << ENDL;
			return TEST_SKIPPED;
		}

		m_pIDBCreateCommand = m_pCRootRowObj->pIDBCreateCommand();
		if (m_pIDBCreateCommand == NULL)
			return TEST_SKIPPED;
		else
			return TRUE;
	} 

	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Before and After executing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_1()
{ 
	ICommand*	pICmd1=NULL;
	ICommand*	pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd2));
	
	// Set text in command object
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(SetScopedCmdTxt(pICmd2, SHALLOW_SCOPED_SELECT),S_OK);
	
	TESTC_(pICmd1->Cancel(),S_OK);

	// cRowsAffected is null on purpose
	TESTC_(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);

	// cRowsAffected is null on purpose
	TESTC_(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);
	TEST2C_(hr = pICmd2->Cancel(), S_OK, DB_E_CANTCANCEL);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Two selects cancel  during execution
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_2()
{ 
	ICommand*	pICmd1 = NULL;
	ICommand*	pICmd2 = NULL;
	HRESULT		hr = NOERROR;
	
	INIT_THREADS(FOUR_THREADS);	

	// create a pair of commands	
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd2));
	
	// Set text in command objects
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(SetScopedCmdTxt(pICmd2, SHALLOW_SCOPED_SELECT),S_OK);

	{	// new block for thread args
	
		// Setup Thread Arguments
		// Expect either hrExecute or hrExecuteOr result from Execute command.
		HRESULT hrExecute	= DB_E_CANCELED;
		HRESULT hrExecuteOr = S_OK;		
		HRESULT hrCancel	= S_OK;
		HRESULT hrCancelOr	= DB_E_CANTCANCEL;

		THREADARG ExecuteFirstCmd = { this, pICmd1,&hrExecute, &hrExecuteOr};
		THREADARG CancelFirstCmd =  { this, pICmd1,&hrCancelOr, &hrCancel};

		THREADARG ExecuteSecondCmd ={ this, pICmd2,&hrExecute, &hrExecuteOr};
		THREADARG CancelSecondCmd = { this, pICmd2,&hrCancelOr, &hrCancel};

		//Create Threads
		CREATE_THREAD(THREAD_ONE, Thread_Execute, &ExecuteFirstCmd);
		CREATE_THREAD(THREAD_TWO, Thread_Cancel, &CancelFirstCmd);

		CREATE_THREAD(THREAD_THREE, Thread_Execute, &ExecuteSecondCmd);
		CREATE_THREAD(THREAD_FOUR, Thread_Cancel, &CancelSecondCmd);

		// Execute
		START_THREADS();
		END_THREADS();			
	}

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Two cmds, cancel 1 during, cancel 1 after
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_3()
{ 
	ICommand*	pICmd1=NULL;
	ICommand*	pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
	INIT_THREADS(TWO_THREADS);

	// create a pair of commands	
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd2));
	
	// Set text in command objects
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(SetScopedCmdTxt(pICmd2, SHALLOW_SCOPED_SELECT),S_OK);
	
	{	// new block for thread args

		// Set up Thread arguments
		// Expect either Canceled or S_OK
		HRESULT hrExecute1	= DB_E_CANCELED;
		HRESULT hrExecuteOr = S_OK; 
		HRESULT hrCancel1	= S_OK;
		HRESULT hrCancel1Or	= DB_E_CANTCANCEL;

		THREADARG ExecuteFirstCmd = { this, pICmd1, &hrExecute1, &hrExecuteOr};
		THREADARG CancelFirstCmd =  { this, pICmd1, &hrCancel1, &hrCancel1Or};

		//Create Thread
		CREATE_THREAD(THREAD_ONE, Thread_Execute, &ExecuteFirstCmd);
		CREATE_THREAD(THREAD_TWO, Thread_Cancel, &CancelFirstCmd);
		
		//Execute
		START_THREADS();
		END_THREADS();
	}
		
	// Second cmd object
	// cRowsAffected is null on purpose
	TESTC_(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);
	TEST2C_(hr = pICmd2->Cancel(), S_OK, DB_E_CANTCANCEL);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);	
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Two cmds, cancel 1 before, cancel 1 during
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_4()
{ 
	ICommand*	pICmd1=NULL;
	ICommand*	pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
	INIT_THREADS(TWO_THREADS);

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd2));
	
	// Set text in command object
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(SetScopedCmdTxt(pICmd2, SHALLOW_SCOPED_SELECT),S_OK);
	
	// Second cmd object
	// cRowsAffected is null on purpose
	TESTC_(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);
	TEST2C_(hr = pICmd2->Cancel(), S_OK, DB_E_CANTCANCEL);

	{	// new block for thread args

		// Set up thread arguments
		// Expect DB_E_CANCELED or S_OK.
		HRESULT hrExecute1	= DB_E_CANCELED;
		HRESULT hrExecuteOr	= S_OK; 
		HRESULT hrCancel1	= S_OK;
		HRESULT hrCancel1Or	= DB_E_CANTCANCEL;
		
		THREADARG ExecuteFirstCmd = { this, pICmd1, &hrExecute1, &hrExecuteOr};
		THREADARG CancelFirstCmd =  { this, pICmd1, &hrCancel1, &hrCancel1Or};

		// Create Threads
		CREATE_THREAD(THREAD_ONE, Thread_Execute, &ExecuteFirstCmd);
		CREATE_THREAD(THREAD_TWO, Thread_Cancel, &CancelFirstCmd);
		
		// Execute
		START_THREADS();
		END_THREADS();
	}


CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc 1 cmd, cancel before execution
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_5()
{ 
	ICommand*	pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(pICmd1->Cancel(),S_OK);

	// cRowsAffected is null on purpose
	TESTC_(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);

CLEANUP:
	
	SAFE_RELEASE(pICmd1);	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc 1 cmd obj, cancel after execution
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_6()
{
	HRESULT		hr;
	ICommand*	pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);

	// cRowsAffected is null on purpose
	TESTC_(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK);
	TEST2C_(hr = pICmd1->Cancel(), S_OK, DB_E_CANTCANCEL);

CLEANUP:
	
	SAFE_RELEASE(pICmd1);

	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc 1 cmd, execute, cancel, cancel
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_7()
{ 
	ICommand*	pICmd1 = NULL;

	INIT_THREADS(THREE_THREADS);
	
	// Create command and set text in command object
	TESTC(CreateCommand(IID_ICommand,(IUnknown **) &pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);

	{	// new block for thread args

		// Set up thread arguments
		// Expect S_OK or DB_E_CANCELED.
		HRESULT hrExecute1	= DB_E_CANCELED;
		HRESULT hrExecuteOr	= S_OK; 
		HRESULT hrCancel1	= S_OK;
		HRESULT hrCancel1Or	= DB_E_CANTCANCEL;
		HRESULT hrCancel2	= S_OK;
		HRESULT hrCancel2Or	= DB_E_CANTCANCEL;

		THREADARG ExecuteFirstCmd = { this, pICmd1, &hrExecute1, &hrExecuteOr};
		THREADARG CancelFirstCmd =  { this, pICmd1, &hrCancel1, &hrCancel1Or};
		THREADARG CancelFirstCmd2 = { this, pICmd1, &hrCancel2, &hrCancel2Or}; 

		// Create Threads
		CREATE_THREAD(THREAD_ONE, Thread_Execute, &ExecuteFirstCmd);
		CREATE_THREAD(THREAD_TWO, Thread_Cancel, &CancelFirstCmd);
		CREATE_THREAD(THREAD_THREE, Thread_Cancel, &CancelFirstCmd2);

		// Execute
		START_THREADS();
		END_THREADS();
	}

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc 1 cmd, no query set, one cancel per thrd
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCancel::Variation_8()
{ 
	ICommand*	pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	
	// cRowsAffected is null on purpose
	TESTC_(pICmd1->Cancel(),S_OK);
	TESTC_(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),DB_E_NOCOMMAND);

CLEANUP:
	
	SAFE_RELEASE(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CCancel::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(CDBSession)
//*-----------------------------------------------------------------------
//| Test Case:		CDBSession - Test GetDBSession method
//| Created:  	11/18/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CDBSession::Init()
{ 
	m_pIRowSession = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 
		if (NULL == FetchRowScopedQuery(SHALLOW_SCOPED_SELECT))
		{
			odtLog << L"Unable to run this test case without a Row Scoped Query." << ENDL;
			return TEST_SKIPPED;
		}

		m_pIDBCreateCommand = m_pCRootRowObj->pIDBCreateCommand();
		if (m_pIDBCreateCommand == NULL)
			return TEST_SKIPPED;

		TEST2C_(m_pCRootRowObj->pIGetSession()->GetSession
												(
												IID_IUnknown,
												&m_pIRowSession
												),S_OK, DB_E_NOSOURCEOBJECT);
		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRowsetInfo::GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_1()
{ 
	ICommand*	 pICmd1				= NULL;
	IRowsetInfo* pIRowsetInfo		= NULL;
	ICommand*	 pICmd2				= NULL;
	IUnknown*	 pIUnknownSession	= NULL;
	
	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT), S_OK);

	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowsetInfo,NULL,NULL, (IUnknown **)&pIRowsetInfo),S_OK);
	TESTC_(m_hr = pIRowsetInfo->GetSpecification(IID_ICommand, (IUnknown **)&pICmd2),S_OK);
	TESTC(VerifyEqualCommands(pICmd1, pICmd2));

	TESTC_(m_hr=pICmd2->GetDBSession(IID_IUnknown,&pIUnknownSession),S_OK);
	TESTC(VerifyRowSession(pIUnknownSession));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnknownSession);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Verify GetDBSession on non executed command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_2()
{ 
	ICommand* pICmd1 = NULL;
	
	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(TestGetDBSession(pICmd1),S_OK);

CLEANUP:
	SAFE_RELEASE(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE, dso iid, valid ptr
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_3()
{ 
	ICommand*	   pICmd1 = NULL;
	IDBInitialize* pIDBInitialize = INVALID(IDBInitialize *);

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr=pICmd1->GetDBSession(IID_IDBInitialize,(IUnknown **)&pIDBInitialize),E_NOINTERFACE);
	
	TESTC(pIDBInitialize == NULL);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	if(pIDBInitialize != INVALID(IDBInitialize *))
		SAFE_RELEASE(pIDBInitialize);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE, iid_null, valid ptr
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_4()
{ 
	ICommand*		  pICmd1 = NULL;
	IDBCreateCommand* pIDBCreateCommand = INVALID(IDBCreateCommand *);
	
	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr=pICmd1->GetDBSession(IID_NULL, (IUnknown **)&pIDBCreateCommand),E_NOINTERFACE);
	
	TESTC(pIDBCreateCommand == NULL);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	if(pIDBCreateCommand != INVALID(IDBCreateCommand *))
		SAFE_RELEASE(pIDBCreateCommand);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG, valid session, ptr=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_5()
{ 
	ICommand*	pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC(CHECKW(m_hr=pICmd1->GetDBSession(IID_IDBCreateCommand,NULL),E_INVALIDARG));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE, row iid, valid ptr
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_6()
{ 
	ICommand*	pICmd1 = NULL;
	IRow*		pIRow = INVALID(IRow *);

	TESTC(CreateCommand(IID_ICommand, (IUnknown **) &pICmd1,NULL));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr=pICmd1->GetDBSession(IID_IRow,(IUnknown **)&pIRow),E_NOINTERFACE);
	
	TESTC(pIRow == NULL);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	if(pIRow != INVALID(IRow *))
		SAFE_RELEASE(pIRow);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Multiple Command Objects
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CDBSession::Variation_7()
{ 
	return CHECK(TestMultipleCommands(20), S_OK);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CDBSession::Terminate()
{ 
	SAFE_RELEASE(m_pIRowSession);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(CExecute)
//*-----------------------------------------------------------------------
//| Test Case:		CExecute - General Execute variations
//| Created:  	11/18/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CExecute::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 
		if (NULL == FetchRowScopedQuery(SHALLOW_SCOPED_SELECT))
		{
			odtLog << L"Unable to run this test case without a Row Scoped Query." << ENDL;
			return TEST_SKIPPED;
		}

		m_pIDBCreateCommand = m_pCRootRowObj->pIDBCreateCommand();
		if (m_pIDBCreateCommand == NULL)
			return TEST_SKIPPED;
		else
			return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple executions on same command object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_1()
{ 
	ICommand*	pICmd1   = NULL;
	IRowset*	pRowset1 = NULL;
	IRowset*	pRowset2 = NULL;
	IRowset*	pRowset3 = NULL;

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));

	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pRowset1),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pRowset2),S_OK);			
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pRowset3),S_OK);

	TESTC(DefaultObjectTesting(pRowset1, ROWSET_INTERFACE));
	TESTC(DefaultObjectTesting(pRowset2, ROWSET_INTERFACE));
	TESTC(DefaultObjectTesting(pRowset3, ROWSET_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pRowset1);
	SAFE_RELEASE(pRowset2);
	SAFE_RELEASE(pRowset3);
	SAFE_RELEASE(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Commands on same Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_2()
{ 
	IRowset*	pRowset1	= NULL;
	IRowset*	pRowset2	= NULL;
	ICommand*	pICmd1		= NULL;
	ICommand*	pICmd2		= NULL;
 
	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd2));
	TESTC(pICmd2 && pICmd2);
	
	// First row object's command
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pRowset1),S_OK);

	// Second row object's command
	TESTC_(SetScopedCmdTxt(pICmd2, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd2->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pRowset2),S_OK);

CLEANUP:	

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);

	SAFE_RELEASE(pRowset1);
	SAFE_RELEASE(pRowset2);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Multilpe Row objects open
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_3()
{ 
	return TestMultipleRowObjects(20);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Properties tests
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG, pcRowsAffected and pIRowset NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_5()
{ 
	ICommand*	pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,NULL),E_INVALIDARG);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Valid, iid=IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_6()
{ 
	DBROWCOUNT		cRowsAffected=0;
	ICommand*		pICmd1=NULL;
	IRowsetLocate*	pIRowsetLocate=NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT),S_OK);

	m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,&cRowsAffected,(IUnknown **)&pIRowsetLocate);
	TEST2C_(m_hr, S_OK, E_NOINTERFACE);
	if (m_hr == S_OK)
	{
		TESTC(DefaultObjectTesting(pIRowsetLocate, ROWSET_INTERFACE));
	}
	else
	{
		TESTC(pIRowsetLocate == NULL);		
	}

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetLocate);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid, iid=IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_7()
{ 
	DBROWCOUNT		cRowsAffected=0;
	ICommand*		pICmd1=NULL;
	IRowsetLocate*	pIColumnsInfo=NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT),S_OK);

	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,(IUnknown **)&pIColumnsInfo),S_OK);
	TESTC(DefaultObjectTesting(pIColumnsInfo, ROWSET_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Valid, iid=IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_8()
{ 
	DBROWCOUNT		cRowsAffected = 0;
	ICommand*		pICmd1 = NULL;
	IUnknown*		pIUnknown = NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT),S_OK);

	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,(IUnknown **)&pIUnknown),S_OK);
	TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc cParam=0
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_9()
{ 
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnknown=NULL;
	DBPARAMS	pParams;

	pParams.pData=NULL;
	pParams.cParamSets=0;
	pParams.hAccessor = DB_NULL_HACCESSOR;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IUnknown,&pParams,NULL,(IUnknown **)&pIUnknown),S_OK);
	TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ICommand::Execute iid=IID_IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_10()
{ 
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnknown=NULL;

	TESTC(CreateCommand(IID_ICommand,(IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmd1->Execute(NULL,IID_IRow,NULL,NULL,(IUnknown **)&pIUnknown),S_OK);
	TESTC(DefaultObjectTesting(pIUnknown, ROW_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ICommandText::Execute, iid=IID_IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute::Variation_11()
{ 
	ICommandText*	pICmdText1=NULL;
	IUnknown*		pIUnknown=NULL;

	TESTC(CreateCommand(IID_ICommandText,(IUnknown **)&pICmdText1));
	TESTC_(SetScopedCmdTxt(pICmdText1, SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(m_hr = pICmdText1->Execute(NULL,IID_IRow,NULL,NULL,(IUnknown **)&pIUnknown),S_OK);
	TESTC(DefaultObjectTesting(pIUnknown, ROW_INTERFACE));

CLEANUP:

	SAFE_RELEASE(pICmdText1);
	SAFE_RELEASE(pIUnknown);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CExecute::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(CPrepare)
//*-----------------------------------------------------------------------
//| Test Case:		CPrepare - test ICommandPrepare cases
//| Created:  	11/23/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CPrepare::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScopedCmd::Init())
	// }}
	{ 
		if (NULL == FetchRowScopedQuery(SHALLOW_SCOPED_SELECT))
		{
			odtLog << L"Unable to run this test case without a Row Scoped Query." << ENDL;
			return TEST_SKIPPED;
		}

		m_pIDBCreateCommand = m_pCRootRowObj->pIDBCreateCommand();
		if (m_pIDBCreateCommand == NULL)
			return TEST_SKIPPED;
		else
			return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Invalid, prepare empty text string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_1()
{ 
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICmd1			= NULL;		// ICommand Object

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR));
	TESTC(pwszSQLStmt != NULL);

	// Make a Empty String Command
	wcscpy(pwszSQLStmt, L"\0");

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(SetCmdTxt(pICmd1, pwszSQLStmt), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), DB_E_NOCOMMAND);

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_FREE(pwszSQLStmt);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid, prepare after NULL ppwszCommand
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_2()
{ 
	ICommand* pICmd1 = NULL;

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(SetCmdTxt(pICmd1, NULL), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), DB_E_NOCOMMAND);

CLEANUP:

	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND, prepare before setting text
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_3()
{ 
	ICommand* pICmd1 = NULL;	

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(PrepareCmd(pICmd1, 1), DB_E_NOCOMMAND);
	
CLEANUP:
	
	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND - Prepare with open rowset object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_4()
{ 
	ICommand*	pICmd1		= NULL;		
	IRowset*	pIRowset	= NULL;		

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(pICmd1->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset), S_OK);
	TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE));
	TESTC_(PrepareCmd(pICmd1, 1), DB_E_OBJECTOPEN);

CLEANUP:

	SAFE_RELEASE(pIRowset)
	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - valid select
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_5()
{ 
	ICommand*	pICmd1		= NULL;		
	IRowset*	pIRowset	= NULL;		

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT),S_OK);
	TESTC_(PrepareCmd(pICmd1, 0),S_OK);
	TESTC_(pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pIRowset),S_OK);
	TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE));
	
CLEANUP:

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - prepare after ::GetColumnInfo FAILS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_6()
{ 
	ICommand*			pICmd1			= NULL;		
	IColumnsInfo*		pIColumnsInfo	= NULL;		
	DBCOUNTITEM			cColumns		= 0;		
	DBCOLUMNINFO*		rgInfo			= NULL;		
	WCHAR*				pStringsBuffer	= NULL;		

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1,SHALLOW_SCOPED_SELECT), S_OK);
	TESTC(VerifyInterface(pICmd1, IID_IColumnsInfo, COMMAND_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC(DefaultObjectTesting(pIColumnsInfo, COMMAND_INTERFACE));

	// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer), DB_E_NOTPREPARED);

	// Compare Results from the DB_E_NOTPREPARED call
	COMPARE(cColumns, 0);
	COMPARE(rgInfo, NULL);
	COMPARE(pStringsBuffer, NULL);

	TESTC_(PrepareCmd(pICmd1, ULONG_MAX),S_OK);

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE_(pICmd1);

	// Free Memory
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return TEST_PASS;	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK ::GetColumnsInfo after Prepare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_7()
{ 
	ICommand*			pICmd1			= NULL;		
	IColumnsInfo*		pIColumnsInfo	= NULL;	
	DBCOUNTITEM			cColumns		= 0;	
	DBCOLUMNINFO*		rgInfo			= NULL;	
	WCHAR*				pStringsBuffer	= NULL;	

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), S_OK);
	
	TESTC(VerifyInterface(pICmd1, IID_IColumnsInfo, COMMAND_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC(DefaultObjectTesting(pIColumnsInfo, COMMAND_INTERFACE));

	// Call IColumnsInfo::GetInfo and expect it to return S_OK
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer), S_OK);

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE_(pICmd1);

	// Free Memory
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare, SetCommandText, GetColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_8()
{ 
	ICommandPrepare*	pICmdPrep		= NULL;	
	IColumnsInfo*		pIColumnsInfo	= NULL;	
	DBCOUNTITEM			cColumns		= 0;	
	DBCOLUMNINFO*		rgInfo			= NULL;	
	WCHAR*				pStringsBuffer	= NULL;	

	TESTC(CreateCommand(IID_ICommandPrepare, (IUnknown **)&pICmdPrep));
	TESTC_(pICmdPrep->Prepare(1), DB_E_NOCOMMAND);

	TESTC_(SetScopedCmdTxt(pICmdPrep,SHALLOW_SCOPED_SELECT),S_OK);

	TESTC(VerifyInterface(pICmdPrep, IID_IColumnsInfo, COMMAND_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC(DefaultObjectTesting(pIColumnsInfo, COMMAND_INTERFACE));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer), DB_E_NOTPREPARED);

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICmdPrep);

	// Free Memory
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Prepare and set properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_11()
{ 
	HRESULT				hr				= E_FAIL;	
	ICommand*			pICmd1			= NULL;
	ICommandProperties* pICommandProp	= NULL;
	IUnknown *			pRowset			= NULL;
	DBPROPSET			PropSet;
	DBPROP				PropIdOwnInsert = {DBPROP_OWNINSERT, DBPROPOPTIONS_REQUIRED, 0, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_EMPTY, 0, 0, 0}};

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), S_OK);

	TESTC(VerifyInterface(pICmd1, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProp));

	PropSet.cProperties = 1;
	PropSet.guidPropertySet = DBPROPSET_ROWSET;
	PropSet.rgProperties = &PropIdOwnInsert;

	m_hr = pICommandProp->SetProperties(1, &PropSet);
	TEST2C_(m_hr, S_OK, DB_E_ERRORSOCCURRED);

	if (m_hr == DB_E_ERRORSOCCURRED)
	{
		CHECKW(hr, DB_E_ERRORSOCCURRED);
		odtLog << L"OwnInsert property was not set for this variation" << ENDL;
	}

	// Execute the Command
	TESTC_(pICmd1->Execute(NULL, IID_IRowset, 0, NULL, &pRowset), S_OK);
	TESTC(DefaultObjectTesting(pRowset, ROWSET_INTERFACE));

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandProp);
	SAFE_RELEASE_(pICmd1);
 
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - unprepare with open rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_12()
{ 
	ICommand*	pICmd1		= NULL;		
	IRowset*	pIRowset	= NULL;		

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);

	TESTC_(pICmd1->Execute(NULL, IID_IRowset, 0, NULL, (IUnknown **)&pIRowset), S_OK);
	TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE));

	TESTC_(UnPrepareCmd(pICmd1), DB_E_OBJECTOPEN);

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Unprepare valid select
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_13()
{ 
	ICommand*	pICmd1 = NULL;	

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(UnPrepareCmd(pICmd1), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), S_OK);
	TESTC_(UnPrepareCmd(pICmd1), S_OK);

CLEANUP:

	SAFE_RELEASE_(pICmd1);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Unprepare and verify GetColumnsInfo fails
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CPrepare::Variation_14()
{ 
	ICommand*			pICmd1			= NULL;		
	IColumnsInfo*		pIColumnsInfo	= NULL;		
	DBCOUNTITEM			cColumns		= 0;		
	DBCOLUMNINFO*		rgInfo			= NULL;		
	WCHAR*				pStringsBuffer	= NULL;		

	TESTC(CreateCommand(IID_ICommand, (IUnknown **)&pICmd1));
	TESTC_(SetScopedCmdTxt(pICmd1, SHALLOW_SCOPED_SELECT), S_OK);
	TESTC_(PrepareCmd(pICmd1, 1), S_OK);
	TESTC_(UnPrepareCmd(pICmd1), S_OK);

	TESTC(VerifyInterface(pICmd1, IID_IColumnsInfo, COMMAND_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC(DefaultObjectTesting(pIColumnsInfo, COMMAND_INTERFACE));

	// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer), DB_E_NOTPREPARED);

	// Compare Results from the DB_E_NOTPREPARED
	COMPARE(cColumns, 0);
	COMPARE(rgInfo, NULL);
	COMPARE(pStringsBuffer, NULL);

CLEANUP:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE_(pICmd1);

	// Free Memory
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return TEST_PASS;	
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CPrepare::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScopedCmd::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

