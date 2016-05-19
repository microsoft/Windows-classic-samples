//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ICMDTEXT.CPP | OLE DB ICommandText tests for Provider, 
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "icmdtext.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x712ad741, 0x5d85, 0x11cf, { 0x89, 0x57, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("ICommandText");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test for ICommandText");
DECLARE_MODULE_VERSION(838413338);
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
	IDBCreateCommand * pIDBCreateCommand = NULL;

	// Get connection and session objects
	TESTC(ModuleCreateDBSession(pThisTestModule));

	// IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
								SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
	{
		odtLog << L"Commands are not supported." << ENDL;
		return TEST_SKIPPED;
	}

	//Release the pointer
	SAFE_RELEASE(pIDBCreateCommand);

	// Create a table and store it in pVoid for now
	pThisTestModule->m_pVoid = new CTable(
		(IUnknown *)pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	TESTC(pThisTestModule->m_pVoid != NULL);

		// Start with a table with 10 rows								 
	TESTC_(((CTable *)pThisTestModule->m_pVoid)->CreateTable(10), S_OK);
					
	// If we made it this far, everything has succeeded
	return TRUE;

CLEANUP:
	
	return FALSE;
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
	// Drop the table created in the ModuleInit
	if( pThisTestModule->m_pVoid )
	{
		// Remove table from database and Delete CTable object
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete ((CTable *)pThisTestModule->m_pVoid);
		pThisTestModule->m_pVoid = NULL;
	}
	
	return ModuleReleaseDBSession(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CCmdText
// @base public | CCommandObject
//
class CCmdText : public CCommandObject{
protected:
	// @cmember IDBCreateCommand interface used in active variation
	IDBCreateCommand *	m_pActiveIDBCreateCommand;
	// @cmember ICommand ptr object, around for lifetime of test case
	ICommand *			m_pICommand;
	// @cmember ICommandText Interface pointer
	ICommandText *		m_pICommandText;
	// @cmember IDBIfo Interface pointer
	IDBInfo *			m_pIDBInfo;
	// @cmember Default Command Text
	WCHAR *				m_pDefaultSetText;
	// @cmember Original Text
	WCHAR *				m_pSQLSet;
	// @cmember Changed Text
	WCHAR *				m_pSQLGet;
	// @cmember Dialect of text
	GUID				m_GetguidDialect;
	// @cmember Dialect of text
	GUID				m_SetguidDialect;
	// @cmember Dialect of text
	GUID				m_DBGUID_DEFAULT;
	// @cmember Dialect of text
	GUID				m_DBGUID_SQL;
	// @cmember Is ICommandPrepare Supported
	BOOL				m_PrepareSupport;
	// @cmember Should command object now be in prepared state (PREPARE or UNPREPARE)
	EPREPARE			m_PrepareState;
	// Second Table for library statement
	CTable *			pCTable2;
	// Expected Result for SetCommandText
	HRESULT				m_SetExpHR;
	// Expected Result for GetCommandText
	HRESULT				m_GetExpHR;
	// Result from variation
	BOOL				m_fResult;
	// CRowObject
	CRowObject*			m_pCRowObject;

	// @cmember Constructor
	CCmdText(LPWSTR wszTestCaseName) : CCommandObject (wszTestCaseName)
	{
		pCTable2					= NULL;
		m_pActiveIDBCreateCommand	= NULL;
		m_pICommandText				= NULL;
		m_pICommand					= NULL;				
		m_pIDBInfo					= NULL;
		m_pSQLSet					= NULL;				
		m_pSQLGet					= NULL;			
		m_pDefaultSetText			= NULL;
		m_GetguidDialect			= DBGUID_DEFAULT;
		m_SetguidDialect			= DBGUID_DEFAULT;
		m_DBGUID_DEFAULT			= DBGUID_DEFAULT;
		m_DBGUID_SQL				= DBGUID_SQL;
		m_PrepareSupport			= FALSE;
		m_PrepareState				= UNPREPARE;
		m_fResult					= FALSE;
		m_SetExpHR					= E_FAIL;
		m_GetExpHR					= E_FAIL;
		m_hr						= E_FAIL;
		m_pCRowObject				= NULL;
	};

	// @cmember Destructor
	virtual ~CCmdText()	{};

	//@cmember Init
	BOOL Init();
	//@cmember Init
	BOOL PostInit();
	//@cmember Terminate
	BOOL Terminate();
	//@cmember Initializes varaibles for each variation
	BOOL InitVariation();
	//@cmember frees memory for each varaition
	BOOL TerminateVariation();
	// @cmember Get an ICommandText ptr, only QI's, nothing else
	BOOL GetICommandText();
	//@cmember Validate that text is the same
	BOOL CompareText ();
	// @cmember IsCommandNowUnprepared, this would be due to changing text,
	BOOL IsCommandNowUnprepared();
	// @cmember VerifyDialect
	BOOL VerifyDialect();
	// @cmember VerifyResults
	BOOL VerifyResults();
	// Gets the Max Length of a Command
	ULONG GetCommandTextMaxLength();
	// Get a command object scoped to a row
	BOOL InitializeRowObject();
	// Get default Command Text
	BOOL GetDefaultText(WCHAR** ppwszText);
	// Get the sql support of the provider
	LONG_PTR GetSQLSupport();
	// Check to see if the provider is ReadOnly
	BOOL GetReadOnly();
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::Init()
{
  	if( COLEDB::Init() )
	{
		// QueryInterface for IDBInfo Optional Interface
		VerifyInterface(m_pThisTestModule->m_pIUnknown,IID_IDBInfo,
								DATASOURCE_INTERFACE,(IUnknown **)&m_pIDBInfo);
		// Set needed pointers
		SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
	
		return TRUE;
	}  

	return FALSE; 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember PostInit
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::PostInit()
{
	ICommandPrepare * pICommandPrepare = NULL;

	// Get a Command object
	TESTC_(m_pActiveIDBCreateCommand->CreateCommand(
					NULL, IID_ICommand, (IUnknown **)&m_pICommand),S_OK);

	// Check for Prepare Support
	if(VerifyInterface(m_pICommand,IID_ICommandPrepare,
							ROWSET_INTERFACE,(IUnknown **)&pICommandPrepare))
		m_PrepareSupport = TRUE;

CLEANUP:

	SAFE_RELEASE(pICommandPrepare);
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::Terminate()
{
	// Free memory
	PROVIDER_FREE(m_pDefaultSetText);

	// Release objects
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIDBInfo);
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	return(COLEDB::Terminate());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember InitVariation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::InitVariation()
{
	m_hr			 = E_FAIL;
	m_SetExpHR		 = S_OK;
	m_GetExpHR		 = S_OK;
	m_fResult		 = FALSE;
	m_GetguidDialect = DBGUID_DEFAULT;
	m_SetguidDialect = DBGUID_DEFAULT;
	m_PrepareState	 = UNPREPARE;

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember TerminateVariation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::TerminateVariation()
{
	// Release ICommandText object
	SAFE_RELEASE(m_pICommandText);

	// Free memory
	PROVIDER_FREE(m_pSQLSet);
	PROVIDER_FREE(m_pSQLGet);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember GetICommandText
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::GetICommandText()
{
	TBEGIN;
	IUnknown *	pIUnknown = NULL;

	// Get ICommandText object
	TESTC(VerifyInterface(m_pICommand,IID_ICommandText,
							COMMAND_INTERFACE,(IUnknown **)&m_pICommandText));
	TESTC_(m_pICommandText->SetCommandText(DBGUID_DEFAULT, NULL), S_OK);

 	// Here is where I verify that ICommandText is inherited from ICommand.
	TESTC_(m_pICommandText->GetDBSession(IID_IDBCreateCommand, &pIUnknown), S_OK);
	
	// Make sure the pointer is not NULL
	TESTC(pIUnknown != NULL);
	SAFE_RELEASE(pIUnknown);

CLEANUP:

	TRETURN;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember VerifyResults
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::VerifyResults()
{
	// QueryInterface for all mandatory Interfaces
	if( DefaultObjectTesting(m_pICommandText, COMMAND_INTERFACE) )
		return IsCommandNowUnprepared();
	else 
		return FALSE;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember VerifyDialect
//
//If provider does not support DBGUID_DEFAULT need extra checking
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::VerifyDialect()
{

	if (!m_pICommandText) 
		return TRUE;

	// Check the result
	if (m_GetguidDialect == m_DBGUID_DEFAULT)
		return TRUE;
	// Check gettig the text with not default dialect 
	GUID tmpGuid = m_GetguidDialect;
	return (m_pICommandText->GetCommandText(&tmpGuid, &m_pSQLGet) == S_OK)
					&& (tmpGuid == m_GetguidDialect);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember IsCommandNowUnprepared
//
// If command is not prepared than a mandatory Command object interface
// should return a NOTPREPARED/NOCOMMAND error.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::IsCommandNowUnprepared()
{
	BOOL			fResult			= FALSE;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	DBORDINAL		cDBCOLUMNINFO	= 0;
	DBCOLUMNINFO *	rgDBCOLUMNINFO	= NULL;
	WCHAR *			pStringsBuffer	= NULL;

	// Get a IColumnsInfo pointer
	TESTC(VerifyInterface(m_pICommand,IID_IColumnsInfo,
							COMMAND_INTERFACE,(IUnknown **)&pIColumnsInfo));

	m_hr = pIColumnsInfo->GetColumnInfo(&cDBCOLUMNINFO, 
										&rgDBCOLUMNINFO, &pStringsBuffer);

	// Print a warning that you can get IColumnsInfo after Execution
	if( m_PrepareState == IMPLICITPREPARE && m_hr == NOERROR && m_PrepareSupport )
		odtLog << L"IColumnsInfo returns valid information without calling ICommandPrepare::Prepare." << ENDL;

	// Need to cover both NOTPREPARED and NOCOMMAND cases of providers
	if( (m_PrepareState == UNPREPARE && m_hr == DB_E_NOTPREPARED && m_PrepareSupport) ||
		(m_PrepareState == UNPREPARE && m_hr == DB_E_ERRORSINCOMMAND &&  !m_PrepareSupport) ||
		(m_PrepareState == UNPREPARE && m_hr == DB_E_NOTABLE &&  !m_PrepareSupport) ||
		(m_PrepareState == UNPREPARE && m_hr == NOERROR && !m_PrepareSupport) ||
		(m_PrepareState == UNPREPARE && m_hr == DB_E_NOCOMMAND)   ||
		(m_PrepareState == PREPARE   && m_hr == NOERROR && m_PrepareSupport) ||
		(m_PrepareState == IMPLICITPREPARE && m_hr == DB_E_NOTPREPARED && m_PrepareSupport) ||
		(m_PrepareState == IMPLICITPREPARE && m_hr == NOERROR && !m_PrepareSupport) ||
		(m_PrepareState == IMPLICITPREPARE && m_hr == DB_E_NOTPREPARED && !cDBCOLUMNINFO) ||
		(m_PrepareState == IMPLICITPREPARE && m_hr == NOERROR && cDBCOLUMNINFO) )
		fResult = TRUE;
	else
	{
		if( m_PrepareState != PREPARE )
			odtLog << L"CCmdText::IsCommandNowUnprepared() - Expected Unprepared state but received prepared state." << ENDL;
		else
			odtLog << L"CCmdText::IsCommandNowUnprepared() - Expected Prepared state but received unprepared state." << ENDL;
	}
		
CLEANUP:
	
	// Release and Free memory
	SAFE_RELEASE(pIColumnsInfo);

	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);
	return fResult;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember CompareText
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::CompareText()
{
	// Check the results
	if( (FAILED(m_hr) && !m_pSQLGet) ||
		!wcscmp(m_pSQLSet, m_pSQLGet) )
		return TRUE;

	odtLog <<L"Set=" <<m_pSQLSet <<ENDL;
	odtLog <<L"Get=" <<m_pSQLGet <<ENDL;
	odtLog <<L"Statements are not the same" <<ENDL;
	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Get CommandText Max Length
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ULONG CCmdText::GetCommandTextMaxLength()
{
	DBLITERALINFO* pTextCmdLiteral = m_pTable->GetLiteralInfo(DBLITERAL_TEXT_COMMAND);
	
	//Some providers may not have support for Suffix yet...
	if( pTextCmdLiteral->fSupported && 
		pTextCmdLiteral->cchMaxLen && pTextCmdLiteral->cchMaxLen != ~0 )
		return pTextCmdLiteral->cchMaxLen;
	else
		return LIMIT;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Get a command object scoped to a row
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::InitializeRowObject()
{
	BOOL				fPass = TEST_SKIPPED;
	IBindResource*      pIBindResource= NULL;
	IRowset*			pIRowset = NULL;
	IGetRow*			pIGetRow = NULL;
	IRow*				pIRow = NULL;
	ULONG_PTR			ulOleObj = 0;
	HRESULT				hr = S_OK;
	DBBINDURLSTATUS		dwBindStatus;
	LPOLESTR			pwszURL = NULL;
	DBCOUNTITEM 			cRowsObtained = 0;
	HROW*				rghRows = NULL;

	//Check if provider supports ROW Objects. If not, then SKIP.
	GetProperty(DBPROP_OLEOBJECTS, 
			    DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulOleObj);
	QTESTC((ulOleObj & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND);

	TESTC(NULL != (m_pCRowObject = new CRowObject));

	QTESTC(VerifyInterface(m_pIOpenRowset,IID_IBindResource,
							SESSION_INTERFACE,(IUnknown **)&pIBindResource));
	
	TESTC_(hr = m_pIOpenRowset->OpenRowset(
					NULL,
					&((CTable *)m_pThisTestModule->m_pVoid)->GetTableID(),
					NULL,
					IID_IRowset,
					0,
					NULL,
					(IUnknown**)&pIRowset), S_OK);

	TESTC_(hr = pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows), S_OK);

	QTESTC(VerifyInterface(pIRowset, IID_IGetRow, ROWSET_INTERFACE, (IUnknown**)&pIGetRow));
	TESTC_(hr = pIGetRow->GetURLFromHROW(rghRows[0], &pwszURL), S_OK);

	TESTC_(hr = pIBindResource->Bind(
					NULL,
					pwszURL,
					DBBINDURLFLAG_READ,
					DBGUID_ROW,
					IID_IRow, 
					NULL, 
					NULL,
					&dwBindStatus, 
					(IUnknown **)&pIRow), S_OK);

	TESTC_(hr = m_pCRowObject->SetRowObject(pIRow), S_OK);
	
	if( !m_pCRowObject->pIDBCreateCommand() )
	{
		odtLog << L"Commands are not supported on the Row object." << ENDL;
		goto CLEANUP;
	}
	
	// Get the Query from the ROOT_URL or Create it
	m_pDefaultSetText = wcsDuplicate(GetModInfo()->GetRowScopedQuery());

	if( !m_pDefaultSetText )
		TESTC_(m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pDefaultSetText, NULL, NULL),S_OK);

	m_pActiveIDBCreateCommand = m_pCRowObject->pIDBCreateCommand();
	fPass = CCmdText::PostInit();

CLEANUP:
	
	if( FAILED(hr) )
		SAFE_DELETE(m_pCRowObject);

	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIBindResource);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGetRow);
	
	PROVIDER_FREE(pwszURL);
	PROVIDER_FREE(rghRows);
	
	return fPass;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Copies the default command text
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::GetDefaultText(WCHAR** ppwszText)
{
	*ppwszText = wcsDuplicate(m_pDefaultSetText);
	return *ppwszText != NULL;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Gets the SQL Support level
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LONG_PTR CCmdText::GetSQLSupport()
{
	ULONG_PTR ulValue;

	if( m_pTable )
		return m_pTable->GetSQLSupport();

	if(GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue))
		return (LONG_PTR)ulValue;

	return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Gets the SQL Support level
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmdText::GetReadOnly()
{
	if( m_pTable )
		return m_pTable->GetIsProviderReadOnly();

	return GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//--------------------------------------------------------------------
// @class Class for Get Command Text
//
class GetCmdText : public CCmdText { 
private:
	
public:
	~GetCmdText (void) {};								
    GetCmdText ( wchar_t* pwszTestCaseName) : CCmdText(pwszTestCaseName) { };	
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember S_OK -> Everything is valid
	int Variation_1();
	// @cmember E_INVALIDARG -> ppwszCommand is NULL
	int Variation_2();
	// @cmember DB_S_DIALECTINGORNED -> Null pguidDialect
	int Variation_3();
	// @cmember S_OK -> Prepared command
	int Variation_4();
	// @cmember S_OK -> Unprepared command
	int Variation_5();
	// @cmember S_OK -> Command retrieved back from rowset
	int Variation_6();
	// @cmember S_OK -> All statements in private library
	int Variation_7();
	// @cmember S_OK -> Command with DBPROP_BOOKMARKS set
	int Variation_8();
	// @cmember S_OK -> pguid = NULL
	int Variation_9();
	// @cmember S_OK -> A Command that returns a empty ResultSet
	int Variation_10();
	// @cmember S_OK -> Execute Command, GetCommandText
	int Variation_11();
	// @cmember S_OK -> Set, Get Empty String
	int Variation_12();
	// @cmember S_OK -> Test Boundary Limit for SQL statement size
	int Variation_13();
	// @cmember S_OK -> Set Text, Set Text, Get Text
	int Variation_14();
	// @cmember DB_E_NOCOMMAND -> No CommandText set, get CommandText
	int Variation_15();
	// @cmember S_OK: Providers translates SQL Escape Clause syntax
	int Variation_16();
	// @cmember NULL pguidDialect for GetCommandText
	int Variation_17();
	// @cmember NULL pguidDialect for GetCommandText with Bad CommandText
	int Variation_18();
	// @cmember NULL pguidDialect for GetCommandText with CommandText
	int Variation_19();
	// @cmember S_OK -> Everything is valid with DBGUID_SQL for SQL Providers
	int Variation_20();
	// @cmember S_OK -> SetCommandText with DBGUID_DEFAULT and GetCommandText with DBGUID_SQL
	int Variation_21();
	// @cmember S_OK -> SetCommandText with DBGUID_SQL and GetCommandText with DBGUID_DEFAULT
	int Variation_22();
	// @cmember S_OK -> Set, Get Empty String with DBGUID_SQL
	int Variation_23();
};


// {{ TCW_TEST_CASE_MAP(GetCmdText_GlobalCmd)
//--------------------------------------------------------------------
// @class Class for Get Command Text
//
class GetCmdText_GlobalCmd : public GetCmdText { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetCmdText_GlobalCmd,GetCmdText);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(GetCmdText_GlobalCmd)
#define THE_CLASS GetCmdText_GlobalCmd
BEG_TEST_CASE(GetCmdText_GlobalCmd, GetCmdText, L"Class for Get Command Text")
	TEST_VARIATION(1, 		L"S_OK -> Everything is valid")
	TEST_VARIATION(2, 		L"E_INVALIDARG -> ppwszCommand is NULL")
	TEST_VARIATION(3, 		L"DB_S_DIALECTINGORNED -> Null pguidDialect")
	TEST_VARIATION(4, 		L"S_OK -> Prepared command")
	TEST_VARIATION(5, 		L"S_OK -> Unprepared command")
	TEST_VARIATION(6, 		L"S_OK -> Command retrieved back from rowset")
	TEST_VARIATION(7, 		L"S_OK -> All statements in private library")
	TEST_VARIATION(8, 		L"S_OK -> Command with DBPROP_BOOKMARKS set")
	TEST_VARIATION(9, 		L"S_OK -> pguid = NULL")
	TEST_VARIATION(10, 		L"S_OK -> A Command that returns a empty ResultSet")
	TEST_VARIATION(11, 		L"S_OK -> Execute Command, GetCommandText")
	TEST_VARIATION(12, 		L"S_OK -> Set, Get Empty String")
	TEST_VARIATION(13, 		L"S_OK -> Test Boundary Limit for SQL statement size")
	TEST_VARIATION(14, 		L"S_OK -> Set Text, Set Text, Get Text")
	TEST_VARIATION(15, 		L"DB_E_NOCOMMAND -> No CommandText set, get CommandText")
	TEST_VARIATION(16, 		L"S_OK: Providers translates SQL Escape Clause syntax")
	TEST_VARIATION(17, 		L"NULL pguidDialect for GetCommandText")
	TEST_VARIATION(18, 		L"NULL pguidDialect for GetCommandText with Bad CommandText")
	TEST_VARIATION(19, 		L"NULL pguidDialect for GetCommandText with CommandText")
	TEST_VARIATION(20, 		L"S_OK -> Everything is valid with DBGUID_SQL for SQL Providers")
	TEST_VARIATION(21, 		L"S_OK -> SetCommandText with DBGUID_DEFAULT and GetCommandText with DBGUID_SQL")
	TEST_VARIATION(22, 		L"S_OK -> SetCommandText with DBGUID_SQL and GetCommandText with DBGUID_DEFAULT")
	TEST_VARIATION(23, 		L"S_OK -> Set, Get Empty String with DBGUID_SQL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetCmdText_RowCmd)
//--------------------------------------------------------------------
// @class Class for Get Command Text
//
class GetCmdText_RowCmd : public GetCmdText { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetCmdText_RowCmd,GetCmdText);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

/*  
Note the following variations deliberately not included below. Putting them inside
BEG_TEST_CASE macro below causes TestWiz to break.

//	TEST_VARIATION(7, 		L"S_OK -> All statements in private library")
//	TEST_VARIATION(10, 		L"S_OK -> A Command that returns a empty ResultSet")
//	TEST_VARIATION(16, 		L"S_OK: Providers translates SQL Escape Clause syntax")
//	TEST_VARIATION(19, 		L"NULL pguidDialect for GetCommandText with CommandText")

*/
// {{ TCW_TESTCASE(GetCmdText_RowCmd)
#define THE_CLASS GetCmdText_RowCmd
BEG_TEST_CASE(GetCmdText_RowCmd, GetCmdText, L"Class for Get Command Text")
	TEST_VARIATION(1, 		L"S_OK: Everything is valid")
	TEST_VARIATION(2, 		L"E_INVALIDARG: ppwszCommand is NULL")
	TEST_VARIATION(3, 		L"DB_S_DIALECTINGORNED: Null pguidDialect")
	TEST_VARIATION(4, 		L"S_OK: Prepared command")
	TEST_VARIATION(5, 		L"S_OK -> Unprepared command")
	TEST_VARIATION(6, 		L"S_OK -> Command retrieved back from rowset")
	TEST_VARIATION(8, 		L"S_OK -> Command with DBPROP_BOOKMARKS set")
	TEST_VARIATION(9, 		L"S_OK -> pguid = NULL")
	TEST_VARIATION(11, 		L"S_OK -> Execute Command, GetCommandText")
	TEST_VARIATION(12, 		L"S_OK -> Set, Get Empty String")
	TEST_VARIATION(13, 		L"S_OK -> Test Boundary Limit for SQL statement size")
	TEST_VARIATION(14, 		L"S_OK -> Set Text, Set Text, Get Text")
	TEST_VARIATION(15, 		L"DB_E_NOCOMMAND -> No CommandText set, get CommandText")
	TEST_VARIATION(17, 		L"NULL pguidDialect for GetCommandText")
	TEST_VARIATION(18, 		L"NULL pguidDialect for GetCommandText with Bad CommandText")
	TEST_VARIATION(20, 		L"S_OK -> Everything is valid with DBGUID_SQL for SQL Providers")
	TEST_VARIATION(21, 		L"S_OK -> SetCommandText with DBGUID_DEFAULT and GetCommandText with DBGUID_SQL")
	TEST_VARIATION(22, 		L"S_OK -> SetCommandText with DBGUID_SQL and GetCommandText with DBGUID_DEFAULT")
	TEST_VARIATION(23, 		L"S_OK -> Set, Get Empty String with DBGUID_SQL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @class Class for Set Command Text
//
class SetCmdText : public CCmdText { 
private:
	
public:
	~SetCmdText (void) {};								
    SetCmdText ( wchar_t* pwszTestCaseName) : CCmdText(pwszTestCaseName) { };	
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember S_OK -> Everything is valid
	int Variation_1();
	// @cmember S_OK -> pwszCommand == NULL
	int Variation_2();
	// @cmember S_OK -> *pwszCommand is an Empty String
	int Variation_3();
	// @cmember DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with NULL
	int Variation_4();
	// @cmember DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with a Empty
	int Variation_5();
	// @cmember DB_E_DIALECTNOTSUPPORTED -> invalid dialect
	int Variation_6();
	// @cmember S_OK -> Run through all statements in private library
	int Variation_7();
	// @cmember S_OK -> Invalid sql command
	int Variation_8();
	// @cmember S_OK -> Set, Prepared command, Set, Get
	int Variation_9();
	// @cmember S_OK -> Unprepared command
	int Variation_10();
	// @cmember DB_E_OBJECTOPEN -> Try to set text while rowset is open
	int Variation_11();
	// @cmember S_OK -> A Command that returns a empty ResultSet
	int Variation_12();
	// @cmember S_OK -> command with DBPROP_BOOKMARKS
	int Variation_13();
	// @cmember S_OK -> Open second command on session, no error
	int Variation_14();
	// @cmember S_OK -> Execute command, release rowset, set new text
	int Variation_15();
	// @cmember S_OK -> Set long text query
	int Variation_16();
	// @cmember DB_E_DIALECTNOTSUPPORTED, use non-dialect guid
	int Variation_17();
	// @cmember Very long command text
	int Variation_18();
};


// {{ TCW_TEST_CASE_MAP(SetCmdText_GlobalCmd)
//--------------------------------------------------------------------
// @class Class for Set Command Text
//
class SetCmdText_GlobalCmd : public SetCmdText { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetCmdText_GlobalCmd,SetCmdText);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(SetCmdText_GlobalCmd)
#define THE_CLASS SetCmdText_GlobalCmd
BEG_TEST_CASE(SetCmdText_GlobalCmd, SetCmdText, L"Class for Set Command Text")
	TEST_VARIATION(1, 		L"S_OK -> Everything is valid")
	TEST_VARIATION(2, 		L"S_OK -> pwszCommand == NULL")
	TEST_VARIATION(3, 		L"S_OK -> *pwszCommand is an Empty String")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with NULL")
	TEST_VARIATION(5, 		L"DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with a Empty")
	TEST_VARIATION(6, 		L"DB_E_DIALECTNOTSUPPORTED -> invalid dialect")
	TEST_VARIATION(7, 		L"S_OK -> Run through all statements in private library")
	TEST_VARIATION(8, 		L"S_OK -> Invalid sql command")
	TEST_VARIATION(9, 		L"S_OK -> Set, Prepared command, Set, Get")
	TEST_VARIATION(10, 		L"S_OK -> Unprepared command")
	TEST_VARIATION(11, 		L"DB_E_OBJECTOPEN -> Try to set text while rowset is open")
	TEST_VARIATION(12, 		L"S_OK -> A Command that returns a empty ResultSet")
	TEST_VARIATION(13, 		L"S_OK -> command with DBPROP_BOOKMARKS")
	TEST_VARIATION(14, 		L"S_OK -> Open second command on session, no error")
	TEST_VARIATION(15, 		L"S_OK -> Execute command, release rowset, set new text")
	TEST_VARIATION(16, 		L"S_OK -> Set long text query")
	TEST_VARIATION(17, 		L"DB_E_DIALECTNOTSUPPORTED, use non-dialect guid")
	TEST_VARIATION(18, 		L"Very long command text")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(SetCmdText_RowCmd)
//--------------------------------------------------------------------
// @class Class for Set Command Text
//
class SetCmdText_RowCmd : public SetCmdText { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetCmdText_RowCmd,SetCmdText);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};


/*  
Note the following variations deliberately not included below. Putting them inside
BEG_TEST_CASE macro below causes TestWiz to break.

//	TEST_VARIATION(7, 		L"S_OK -> Run through all statements in private library")
//	TEST_VARIATION(9, 		L"S_OK -> Set, Prepared command, Set, Get")
//	TEST_VARIATION(11, 		L"DB_E_OBJECTOPEN -> Try to set text while rowset is open")
//	TEST_VARIATION(12, 		L"S_OK -> A Command that returns a empty ResultSet")

*/
// {{ TCW_TESTCASE(SetCmdText_RowCmd)
#define THE_CLASS SetCmdText_RowCmd
BEG_TEST_CASE(SetCmdText_RowCmd, SetCmdText, L"Class for Set Command Text")
	TEST_VARIATION(1, 		L"S_OK -> Everything is valid")
	TEST_VARIATION(2, 		L"S_OK -> pwszCommand == NULL")
	TEST_VARIATION(3, 		L"S_OK -> *pwszCommand is an Empty String")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with NULL")
	TEST_VARIATION(5, 		L"DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with a Empty")
	TEST_VARIATION(6, 		L"DB_E_DIALECTNOTSUPPORTED -> invalid dialect")
	TEST_VARIATION(8, 		L"S_OK -> Invalid sql command")
	TEST_VARIATION(10, 		L"S_OK -> Unprepared command")
	TEST_VARIATION(13, 		L"S_OK -> command with DBPROP_BOOKMARKS")
	TEST_VARIATION(14, 		L"S_OK -> Open second command on session, no error")
	TEST_VARIATION(15, 		L"S_OK -> Execute command, release rowset, set new text")
	TEST_VARIATION(16, 		L"S_OK -> Set long text query")
	TEST_VARIATION(17, 		L"DB_E_DIALECTNOTSUPPORTED, use non-dialect guid")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CZombie)
//--------------------------------------------------------------------
// @class Induce zombie states
//
class CZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Row Handle for the Rowset
	HROW *	m_rghRows;
	// @mfunc Does the dirty work
	int TestTxn(EMETHOD eMethod, ETXN eTxn, BOOL fRetaining);
	//@mfunc Cleanup the Rowsets in the Transaction
	void CleanupTransactionRowset();

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort ICommandText::SetCommandText with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Abort ICommandText::SetCommandText with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Commit ICommandText::SetCommandText with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Commit ICommandText::SetCommandText with fRetaining=FALSE
	int Variation_4();
	// @cmember S_OK - Abort ICommandText::GetCommandText with fRetaining=TRUE
	int Variation_5();
	// @cmember S_OK - Abort ICommandText::GetCommandText with fRetaining=FALSE
	int Variation_6();
	// @cmember S_OK - Commit ICommandText::GetCommandText with fRetaining=TRUE
	int Variation_7();
	// @cmember S_OK - Commit ICommandText::GetCommandText with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CZombie)
#define THE_CLASS CZombie
BEG_TEST_CASE(CZombie, CTransaction, L"Induce zombie states")
	TEST_VARIATION(1, 		L"S_OK - Abort ICommandText::SetCommandText with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Abort ICommandText::SetCommandText with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Commit ICommandText::SetCommandText with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Commit ICommandText::SetCommandText with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"S_OK - Abort ICommandText::GetCommandText with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"S_OK - Abort ICommandText::GetCommandText with fRetaining=FALSE")
	TEST_VARIATION(7, 		L"S_OK - Commit ICommandText::GetCommandText with fRetaining=TRUE")
	TEST_VARIATION(8, 		L"S_OK - Commit ICommandText::GetCommandText with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CCmdText { 
private:
	
public:
	~TCExtendedErrors (void) {};								
    TCExtendedErrors ( wchar_t* pwszTestCaseName) : CCmdText(pwszTestCaseName) { };
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember Valid ICommandText calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid ICommandText calls with previous error object existing
	int Variation_2();
	// @cmember Invalid ICommandText calls with no previous error object existing
	int Variation_3();
	// @cmember DB_E_NOCOMMAND SetCommandText calls with no previous error object existing
	int Variation_4();
};


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_GlobalCmd)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors_GlobalCmd : public TCExtendedErrors { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_GlobalCmd,TCExtendedErrors);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCExtendedErrors_GlobalCmd)
#define THE_CLASS TCExtendedErrors_GlobalCmd
BEG_TEST_CASE(TCExtendedErrors_GlobalCmd, TCExtendedErrors, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid ICommandText calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid ICommandText calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid ICommandText calls with no previous error object existing")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND SetCommandText calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_RowCmd)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors_RowCmd : public TCExtendedErrors { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_RowCmd,TCExtendedErrors);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCExtendedErrors_RowCmd)
#define THE_CLASS TCExtendedErrors_RowCmd
BEG_TEST_CASE(TCExtendedErrors_RowCmd, TCExtendedErrors, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid ICommandText calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid ICommandText calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid ICommandText calls with no previous error object existing")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND SetCommandText calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, GetCmdText_GlobalCmd)
	TEST_CASE(2, GetCmdText_RowCmd)
	TEST_CASE(3, SetCmdText_GlobalCmd)
	TEST_CASE(4, SetCmdText_RowCmd)
	TEST_CASE(5, CZombie)
	TEST_CASE(6, TCExtendedErrors_GlobalCmd)
	TEST_CASE(7, TCExtendedErrors_RowCmd)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(GetCmdText)
//*-----------------------------------------------------------------------
//| Test Case:		GetCmdText - Class for Get Command Text
//|	Created:		02/02/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText::Init()
{
	return CCmdText::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Everything is valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_1()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG -> ppwszCommand is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_2()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,NULL),E_INVALIDARG);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_S_DIALECTINGORNED -> Null pguidDialect
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_3()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_GetguidDialect = GUID_NULL;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_S_DIALECTIGNORED);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Prepared command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_4()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Prepare the Command and Get the Text
	if( m_PrepareSupport && SUCCEEDED(PrepareCommand(m_pICommand, PREPARE, 1)) )
		m_PrepareState = PREPARE;

	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Unprepared command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_5()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Prepare the Command and Get the Text
	if( m_PrepareSupport && SUCCEEDED(PrepareCommand(m_pICommand, BOTH, 1)) )
		m_PrepareState = UNPREPARE;

	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Command retrieved back from rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_6()
{
	TBEGIN;

	ICommandText *	pICommandRetrieved	= NULL;	// command
	IRowsetInfo *	pIRowsetInfo		= NULL;	// rowset info
	IRowset *		pIRowset			= NULL;	// array of rowsets
	DBROWCOUNT		cRowsAffected		= 0;	// count of rowsets

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL,IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**)&pIRowset), S_OK);
	
	m_PrepareState = IMPLICITPREPARE;

	// Get the Command object off of the Rowset
	TESTC(VerifyInterface(pIRowset,IID_IRowsetInfo,
							ROWSET_INTERFACE,(IUnknown **)&pIRowsetInfo));
	
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommandText,
									(IUnknown **) &pICommandRetrieved),S_OK);
	
	TESTC_(pICommandRetrieved->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);
	
	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandRetrieved);

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> All statements in private library
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_7()
{
	TBEGIN;

	ULONG index = 0;

	INIT;
	
	// Create a Second Table
	pCTable2 = new CTable(( (IUnknown *)m_pActiveIDBCreateCommand ? 
							(IUnknown *)m_pActiveIDBCreateCommand : 
							(IUnknown *)m_pIOpenRowset),(LPWSTR)gwszModuleName);

	TESTC(pCTable2 != NULL);

	// Start with a table with 10 rows								 
	TESTC_(pCTable2->CreateTable(10), S_OK);

	// for each statement listed in icmdtext.h
	for(index=0; index < gcSQLStmt; index++)
	{
		// Get Command Text
		HRESULT hrCreateStmt = m_pTable->CreateSQLStmt(grgSQLStmt[index], 
					pCTable2->GetTableName(), &m_pSQLSet, NULL, NULL);

		// Check for unsupported statements from provider
		if (hrCreateStmt == DB_E_NOTSUPPORTED)
			continue;

		TESTC_(hrCreateStmt, S_OK);

		TESTC(GetICommandText());



		// Set and get the text
		TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
		TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

		// Verify the Dialect
		COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
		TESTC(VerifyDialect());

		// Verify the text
		TESTC(CompareText());
		TESTC(VerifyResults());
		TERM;
	}

CLEANUP:

	// Remove table from database
	if( pCTable2 ) {
		pCTable2->DropTable();
		delete pCTable2;
	}

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Command with DBPROP_BOOKMARKS set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_8()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Check to see if DBPROP_BOOKMARKS is supported
	if( !SupportedProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown) )
		odtLog << L"Bookmarks not supported." << ENDL;
	else
	{
		if( SUCCEEDED(SetRowsetProperty(m_pICommandText, 
								DBPROPSET_ROWSET, DBPROP_BOOKMARKS, TRUE)) ) {
			TESTC(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommandText));
		}
		else {
			TESTC(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommandText));
		}
	}

	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Turn Bookmarks back off
	SetRowsetProperty(m_pICommandText, DBPROPSET_ROWSET, DBPROP_BOOKMARKS, FALSE);

	TERM;
	TRETURN;
}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> pguid = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_9()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(NULL,&m_pSQLGet),S_OK);

	// 2 sql statements should be different because ODBC to sql translations
	odtLog << L"Set:" << m_pSQLSet << ENDL;
	odtLog << L"Get:" << m_pSQLGet << ENDL;

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> A Command that returns a empty ResultSet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_10()
{
	TBEGIN;

	// If no SQL Support, skip variation
	if( GetSQLSupport() == DBPROPVAL_SQL_NONE || GetReadOnly() )
		return TEST_SKIPPED;
	
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC_(m_pTable->ExecuteCommand(INSERT_ROW_WITH_LITERALS, IID_IRowset, NULL,
		&m_pSQLSet, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL,
		(ICommand **)&m_pICommand), S_OK);
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);
	
	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());
	
	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());
	
CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Execute Command, GetCommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_11()
{
	TBEGIN;

	IRowset * pIRowset		= NULL;	// array of rowsets
	DBROWCOUNT cRowsAffected	= 0;	// count of rowsets

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	TESTC_(m_pICommandText->Execute(NULL,IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**)&pIRowset), S_OK);

	m_PrepareState = IMPLICITPREPARE;
	SAFE_RELEASE(pIRowset);
		
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	SAFE_RELEASE(pIRowset);

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Set, Get Empty String
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_12()
{
	TBEGIN;
	INIT;

	// Set a SQL Stmt and Set the Command
	SAFE_ALLOC(m_pSQLSet, WCHAR, sizeof(WCHAR));
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));
	
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Test Boundary Limit for SQL statement size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_13()
{
	TBEGIN;

	ULONG i = 0;
	
	INIT;

	// Get the max length
	ULONG ulMaxSQL = GetCommandTextMaxLength();

	// Get memory for set text
	SAFE_ALLOC(m_pSQLSet, WCHAR, ulMaxSQL+1);

	// Initialize to all A's
	for(i=0; i<ulMaxSQL; i++)
		m_pSQLSet[i] = L'A';
	m_pSQLSet[ulMaxSQL] = '\0';

	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Set Text, Set Text, Get Text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_14()
{
	TBEGIN;
	INIT;
	
	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Create a SQL Stmt and Set the Command
	PROVIDER_FREE(m_pSQLSet);
	TESTC(GetDefaultText(&m_pSQLSet));

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND -> No CommandText set, get CommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_15()
{
	INIT;

	// Get the Command Text
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);

	// Verify the Data
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Providers translates SQL Escape Clause syntax
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_16()
{
	TBEGIN;

	HRESULT	  hr			= E_FAIL; // HResult
	IRowset * pIRowset		= NULL;	  // array of rowsets
	DBROWCOUNT cRowsAffected	= 0;	  // count of rowsets
	HRESULT hrCreateStmt	= E_FAIL;

	INIT;

	// Create a Second Table
	pCTable2 = new CTable(( (IUnknown *)m_pActiveIDBCreateCommand ? 
							(IUnknown *)m_pActiveIDBCreateCommand : 
							(IUnknown *)m_pIOpenRowset),(LPWSTR)gwszModuleName);

	// Start with a table with 10 rows								 
	TESTC_(pCTable2->CreateTable(10), S_OK);
	TESTC(GetICommandText());

	// Create a SQL Stmt and Set the Command
	hrCreateStmt = m_pTable->CreateSQLStmt(SELECT_LEFTOUTERJOIN,pCTable2->GetTableName(), &m_pSQLSet, NULL, NULL);
	TESTC_PROVIDER(hrCreateStmt != DB_E_NOTSUPPORTED);

	TESTC_(hrCreateStmt, S_OK);

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(NULL,&m_pSQLGet),S_OK);

	// 2 sql statements should be different because ODBC to sql translations
	odtLog << L"Set:" << m_pSQLSet << ENDL;
	odtLog << L"Get:" << m_pSQLGet << ENDL;

	// Execute stmt
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	hr=m_pICommandText->Execute(NULL, IID_IRowset, NULL, 
								&cRowsAffected, (IUnknown **) &pIRowset);
	SAFE_RELEASE(pIRowset);

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	m_hr=m_pICommandText->Execute(NULL, IID_IRowset, NULL, 
								&cRowsAffected, (IUnknown **) &pIRowset);
	SAFE_RELEASE(pIRowset);

	// Compare the HResults
	TESTC_(m_hr, hr);

CLEANUP:

	// Remove table from database
	if( pCTable2 ) {
		pCTable2->DropTable();
		delete pCTable2;
	}

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc NULL pguidDialect for GetCommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_17()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(NULL,&m_pSQLGet),S_OK);

	// 2 sql statements should be different because ODBC to sql translations
	odtLog << L"Set:" << m_pSQLSet << ENDL;
	odtLog << L"Get:" << m_pSQLGet << ENDL;

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc NULL pguidDialect for GetCommandText with Bad CommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_18()
{
	TBEGIN;
	INIT;

	// Set the Text	on the Stack
	m_pSQLSet = wcsDuplicate(L"{}{}{}");
	
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(NULL,&m_pSQLGet),S_OK);

	// 2 sql statements should be different because ODBC to sql translations
	odtLog << L"Set:" << m_pSQLSet << ENDL;
	odtLog << L"Get:" << m_pSQLGet << ENDL;

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc NULL pguidDialect for GetCommandText with CommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_19()
{
	TBEGIN;
	INIT;

	// Set the Text	on the Stack
	m_pSQLSet = wcsDuplicate(L"{?=call alp()}");
	
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_pICommandText->GetCommandText(NULL,&m_pSQLGet),S_OK);

	// 2 sql statements should be different because ODBC to sql translations
	odtLog << L"Set:" << m_pSQLSet << ENDL;
	odtLog << L"Get:" << m_pSQLGet << ENDL;

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Everything is valid with DBGUID_SQL for SQL Providers
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_20()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = m_DBGUID_SQL;
	m_GetguidDialect = m_DBGUID_SQL;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TEST2C_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK,DB_E_DIALECTNOTSUPPORTED);
	COMPARE(m_SetguidDialect, m_DBGUID_SQL);

	// Verify the Data
	if( FAILED(m_hr) ) 
	{
		// SQLSupport should be DBPROPVAL_SQL_NONE
		if( GetSQLSupport() != DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");

		TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);
		COMPARE(m_GetguidDialect, GUID_NULL);
		TESTC(!m_pSQLGet);
	}
	else 
	{
		// SQLSupport should be supported
		if( GetSQLSupport() == DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");

		TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);
		COMPARE(m_GetguidDialect, m_DBGUID_SQL);
		TESTC(CompareText());
	}

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> SetCommandText with DBGUID_DEFAULT and GetCommandText with DBGUID_SQL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_21()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = m_DBGUID_DEFAULT;
	m_GetguidDialect = m_DBGUID_SQL;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	TEST2C_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect, &m_pSQLGet),S_OK,DB_S_DIALECTIGNORED);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	if( m_hr == DB_S_DIALECTIGNORED )
		TESTC(VerifyDialect())
	else
		COMPARE(m_GetguidDialect, m_DBGUID_SQL);

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> SetCommandText with DBGUID_SQL and GetCommandText with DBGUID_DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_22()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = m_DBGUID_SQL;
	m_GetguidDialect = m_DBGUID_DEFAULT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TEST2C_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK,DB_E_DIALECTNOTSUPPORTED);
	COMPARE(m_SetguidDialect, m_DBGUID_SQL);

	// Verify the Data
	if( FAILED(m_hr) ) 
	{
		// SQLSupport should be DBPROPVAL_SQL_NONE
		if( GetSQLSupport() != DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");

		TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);
		COMPARE(m_GetguidDialect, GUID_NULL);
		TESTC(!m_pSQLGet);
	}
	else 
	{
		// SQLSupport should be supported
		if( GetSQLSupport() == DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");

		TEST2C_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK,DB_S_DIALECTIGNORED);
		if( m_hr == DB_S_DIALECTIGNORED )
			COMPARE(m_GetguidDialect, m_DBGUID_SQL);
		else
			TESTC(VerifyDialect());
		TESTC(CompareText());
	}

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Set, Get Empty String with DBGUID_SQL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetCmdText::Variation_23()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = m_DBGUID_SQL;
	m_GetguidDialect = m_DBGUID_SQL;

	// Set a SQL Stmt and Set the Command
	SAFE_ALLOC(m_pSQLSet, WCHAR, sizeof(WCHAR));
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));
	
	TESTC(GetICommandText());

	// Set and get the text
	TEST2C_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK,DB_E_DIALECTNOTSUPPORTED);
	COMPARE(m_SetguidDialect, m_DBGUID_SQL);

	// Verify the Data
	if( FAILED(m_hr) ) 
	{
		// SQLSupport should be DBPROPVAL_SQL_NONE
		if( GetSQLSupport() != DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");
	}
	else 
	{
		// SQLSupport should be supported
		if( GetSQLSupport() == DBPROPVAL_SQL_NONE )
			TWARNING("DBPROP_SQLSUPPORT is not supported?");
	}

	TESTC_(m_pICommandText->GetCommandText(NULL,&m_pSQLGet),DB_E_NOCOMMAND);
	TESTC(!m_pSQLGet);

	// Verify the text
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText::Terminate()
{
	return (CCmdText::Terminate());
}	
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText_GlobalCmd::Init()
{
	if(GetCmdText::Init())
	{
		m_pActiveIDBCreateCommand = m_pIDBCreateCommand;
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

			// Create a SQL Stmt and Set the Command
		TESTC_(m_hr=m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pDefaultSetText, NULL, NULL),S_OK);
		return CCmdText::PostInit();		
	}

CLEANUP:
	return FALSE;
}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText_GlobalCmd::Terminate()
{
	return (GetCmdText::Terminate());
}	
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText_RowCmd::Init()
{
	if(GetCmdText::Init())
	{
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		
		return InitializeRowObject();
	}

	return FALSE;
}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetCmdText_RowCmd::Terminate()
{
	SAFE_DELETE(m_pCRowObject);
	return (GetCmdText::Terminate());
}	
// }}
// }}


// {{ TCW_TC_PROTOTYPE(SetCmdText)
//*-----------------------------------------------------------------------
//| Test Case:		SetCmdText - Class for Set Command Text
//|	Created:		02/02/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText::Init()
{
  	return CCmdText::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Everything is valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_1()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> pwszCommand == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_2()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// NULL pwszCommand -> Clear and Uprepares the Command
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,NULL), S_OK);
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));

	// Try to Prepare the Command
	if( m_PrepareSupport ) {
		TESTC_(PrepareCommand(m_pICommand, PREPARE, 1), DB_E_NOCOMMAND);
	}

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), DB_E_NOCOMMAND);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> *pwszCommand is an Empty String
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_3()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// pwszCommand is Empty String -> Clear and Uprepares the Command
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Try to Prepare the Command
	if( m_PrepareSupport ) {
		TESTC_(PrepareCommand(m_pICommand, PREPARE, 1), DB_E_NOCOMMAND);
	}

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), DB_E_NOCOMMAND);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_4()
{
	TBEGIN;

	IRowset *	pIRowset		= NULL;	// array of rowsets
	DBROWCOUNT	cRowsAffected	= 0;	// count of rowsets

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// NULL pwszCommand -> Clear and Uprepares the Command
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,NULL), S_OK);
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
				NULL, &cRowsAffected, (IUnknown**) &pIRowset), DB_E_NOCOMMAND);

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND -> ICommand::Execute after SetCommandText with a Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_5()
{
	TBEGIN;

	IRowset *	pIRowset		= NULL;	// array of rowsets
	DBROWCOUNT	cRowsAffected	= 0;	// count of rowsets

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// NULL pwszCommand -> Clear and Uprepares the Command
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
				NULL, &cRowsAffected, (IUnknown**) &pIRowset), DB_E_NOCOMMAND);

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DIALECTNOTSUPPORTED -> invalid dialect
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_6()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = GUID_NULL;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), DB_E_DIALECTNOTSUPPORTED);
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, GUID_NULL);
	COMPARE(m_GetguidDialect, GUID_NULL);

	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Run through all statements in private library
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_7()
{
	TBEGIN;

	ULONG index = 0;

	INIT;
	
	// Create a Second Table
	pCTable2 = new CTable(( (IUnknown *)m_pActiveIDBCreateCommand ? 
							(IUnknown *)m_pActiveIDBCreateCommand : 
							(IUnknown *)m_pIOpenRowset),(LPWSTR)gwszModuleName);

	TESTC(pCTable2 != NULL);

	// Start with a table with 10 rows								 
	TESTC_(pCTable2->CreateTable(10), S_OK);

	// for each statement listed in icmdtext.h
	for(index=0; index < gcSQLStmt; index++)
	{	
		// Get Command Text
		HRESULT hrCreateStmt = m_pTable->CreateSQLStmt(grgSQLStmt[index], 
					pCTable2->GetTableName(), &m_pSQLSet, NULL, NULL);

		// Check for unsupported statements from provider
		if (hrCreateStmt == DB_E_NOTSUPPORTED)
			continue;

		TESTC_(hrCreateStmt, S_OK);

		TESTC(GetICommandText());

		// Set and get the text
		TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
		TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

		// Verify the Dialect
		COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
		TESTC(VerifyDialect());

		// Verify the text
		TESTC(CompareText());
		TESTC(VerifyResults());
		TERM;
	}

CLEANUP:

	// Remove table from database
	if( pCTable2 ) {
		pCTable2->DropTable();
		delete pCTable2;
	}

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Invalid sql command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_8()
{
	TBEGIN;
	INIT;
	
	// Set the Command Text
	SAFE_ALLOC(m_pSQLSet, WCHAR, wcslen(L"Select * from xxx")+1);
	wcscpy(m_pSQLSet, L"Select * from xxx");

	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Set, Prepared command, Set, Get
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_9()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);
	PROVIDER_FREE(m_pSQLSet);

	// Prepare the Command and Get the Text
	if( m_PrepareSupport && SUCCEEDED(PrepareCommand(m_pICommand, PREPARE, 1)) )
		m_PrepareState = PREPARE;

	// Check to see if the Command is Prepared
	TESTC(VerifyResults());

	// Create a SQL Stmt and Set new Command Text
	TESTC_(m_pTable->CreateSQLStmt(SELECT_REVCOLLIST,NULL,&m_pSQLSet,NULL,NULL),S_OK);

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);
	m_PrepareState = UNPREPARE;

	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Unprepared command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_10()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Prepare the Command and Get the Text
	if( m_PrepareSupport && SUCCEEDED(PrepareCommand(m_pICommand, BOTH, 1)) )
		m_PrepareState = UNPREPARE;

	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN -> Try to set text while rowset is open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_11()
{
	TBEGIN;

	IRowset *	pIRowset		= NULL;	// array of rowsets
	DBROWCOUNT	cRowsAffected	= 0;	// count of rowsets
	WCHAR *		pSQLSet1		= NULL; // SQL Text

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**) &pIRowset), S_OK);
	m_PrepareState = IMPLICITPREPARE;

	// Check to see if the Command is Prepared
	TESTC(VerifyResults());

	// Create a SQL Stmt and Set new Command Text
	TESTC_(m_pTable->CreateSQLStmt(SELECT_REVCOLLIST,NULL,&pSQLSet1,NULL,NULL),S_OK);
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,pSQLSet1), DB_E_OBJECTOPEN);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Rowset object must be closed in order to check state of command object
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(pSQLSet1);

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> A Command that returns a empty ResultSet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_12()
{
	TBEGIN;

	// If no SQL Support, skip variation
	if( GetSQLSupport() == DBPROPVAL_SQL_NONE || GetReadOnly() )
		return TEST_SKIPPED;
	
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC_(m_pTable->ExecuteCommand(INSERT_ROW_WITH_LITERALS, IID_IRowset, NULL,
		&m_pSQLSet, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL,
		(ICommand **)&m_pICommand), S_OK);
	TESTC(GetICommandText());
	
	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),S_OK);
	
	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());
	
	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> command with DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_13()
{
	TBEGIN;
	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// Check to see if DBPROP_BOOKMARKS is supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize) )
		odtLog << L"Bookmarks not supported." << ENDL;
	else
	{
		if( SUCCEEDED(SetRowsetProperty(m_pICommandText, 
								DBPROPSET_ROWSET, DBPROP_BOOKMARKS, TRUE)) ) {
			TESTC(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommandText));
		}
		else {
			TESTC(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommandText));
		}
	}

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Turn Bookmarks back off
	SetRowsetProperty(m_pICommandText, DBPROPSET_ROWSET, DBPROP_BOOKMARKS, FALSE);

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Open second command on session, no error
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_14()
{
	TBEGIN;

	ICommand *		pICommand2		= NULL;
	ICommandText *	pICommandText2	= NULL;
	WCHAR *			pSQLGet1		= NULL;

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Get second command object
	TESTC_(m_pActiveIDBCreateCommand->CreateCommand(NULL, 
								IID_ICommand, (IUnknown **)&pICommand2), S_OK);
	
	// Get CommandText pointer
	TESTC(VerifyInterface(pICommand2,IID_ICommandText,
							COMMAND_INTERFACE,(IUnknown **)&pICommandText2));

	// Get Command Text
	TESTC_(pICommandText2->GetCommandText(&m_GetguidDialect,&pSQLGet1), DB_E_NOCOMMAND);
	COMPARE(m_GetguidDialect, GUID_NULL);
	TESTC(!pSQLGet1);

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), DB_S_DIALECTIGNORED);
	TESTC(VerifyDialect());
	PROVIDER_FREE(m_pSQLGet);

	// Get Command Text
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Release the 2nd Command
	SAFE_RELEASE(pICommand2);
	SAFE_RELEASE(pICommandText2);
	PROVIDER_FREE(pSQLGet1);

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Execute command, release rowset, set new text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_15()
{
	TBEGIN;

	IRowset *	pIRowset		= NULL;	// array of rowsets
	DBROWCOUNT	cRowsAffected	= 0;	// count of rowsets

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**) &pIRowset), S_OK);
	m_PrepareState = IMPLICITPREPARE;

	// Check to see if the Command is Prepared
	TESTC(VerifyResults());
	SAFE_RELEASE(pIRowset);
			
	// Get Command Text
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

	// Free first stmt
	PROVIDER_FREE(m_pSQLSet);
	PROVIDER_FREE(m_pSQLGet);

	TESTC_(m_pTable->CreateSQLStmt(SELECT_REVCOLLIST,NULL,
												&m_pSQLSet, NULL, NULL),S_OK);

	// Set the Command Text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);
	m_PrepareState = UNPREPARE;

	// Get Command Text
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());
	
	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:
			
	// Release the Rowset
	SAFE_RELEASE(pIRowset);
			
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK -> Set long text query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_16()
{
	TBEGIN;

	IRowset* pIRowset = NULL;
	DBROWCOUNT cRowsAffected = -5;
	ULONG i = 0;

	INIT;

	// Get the max length
	ULONG ulMaxSQL = GetCommandTextMaxLength();

	// Get memory for set text
	SAFE_ALLOC(m_pSQLSet, WCHAR, ulMaxSQL+1);

	// Initialize to all x's
	for(i=0; i<ulMaxSQL; i++)
		m_pSQLSet[i] = L'x';
	m_pSQLSet[ulMaxSQL] = '\0';

	TESTC(GetICommandText());

	// Set and get the text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), S_OK);

	// Verify the Data
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());
	
	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DIALECTNOTSUPPORTED, use non-dialect guid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_17()
{
	TBEGIN;
	INIT;

	// Set pguidDialect
	m_SetguidDialect = GUID_NULL;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet), DB_E_DIALECTNOTSUPPORTED);
	memset(m_pSQLSet, L'\0', sizeof(WCHAR));

	// Get Command Text
	TESTC_(m_pICommandText->GetCommandText(&m_GetguidDialect, &m_pSQLGet), DB_E_NOCOMMAND);

	// Verify the Data
	COMPARE(m_SetguidDialect, GUID_NULL);
	COMPARE(m_GetguidDialect, GUID_NULL);
	
	// Verify the text
	TESTC(!m_pSQLGet);
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Very long command text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetCmdText::Variation_18()
{
	TBEGIN;
	INIT;
	IRowset* pIRowset = NULL;
	DBROWCOUNT cRowsAffected = -5;
	WCHAR* longstr = NULL;
	int i=0;

	// Get the max command length
	ULONG ulMaxSQL = GetCommandTextMaxLength();

	SAFE_ALLOC(longstr, WCHAR, 99000*sizeof(WCHAR));
	ASSERT(longstr!=NULL);
	wcscpy(longstr, L"");
	TESTC(GetICommandText());
	for(i=0; i<1000; i++)
		wcscat(longstr, L"adghtountionhdkkkkkkkkkkkkkkkhjlhlkjhjhuiydf;dsfffffoooooa;ouifiusdaydfkhkjhdsfalkdjfhslkjdhfslk");

	CHECK(m_pICommandText->SetCommandText(DBGUID_DEFAULT, longstr), S_OK);
	odtLog<<L"Call Execute.\n";
	CHECK(m_pICommandText->Execute(NULL,IID_IRowset, 
					NULL, &cRowsAffected, (IUnknown**)&pIRowset), DB_E_ERRORSINCOMMAND);
	odtLog<<L"Finished calling Execute.\n";

CLEANUP:
	SAFE_FREE(longstr);
	TERM;
	TRETURN;
}
// }}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText::Terminate()
{
	return (CCmdText::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText_GlobalCmd::Init()
{
	if(SetCmdText::Init())
	{
		m_pActiveIDBCreateCommand = m_pIDBCreateCommand;
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

			// Create a SQL Stmt and Set the Command
		TESTC_(m_hr=m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pDefaultSetText, NULL, NULL),S_OK);
		return CCmdText::PostInit();		
	}

CLEANUP:
	return FALSE;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText_GlobalCmd::Terminate()
{
	return (SetCmdText::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText_RowCmd::Init()
{
	if(SetCmdText::Init())
	{
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		
		return InitializeRowObject();
	}

	return FALSE;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetCmdText_RowCmd::Terminate()
{
	SAFE_DELETE(m_pCRowObject);
	return (SetCmdText::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CZombie)
//*-----------------------------------------------------------------------
//| Test Case:		CZombie - Induce zombie states
//|	Created:		02/02/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Init()
{
	// Check to see if Transactions are usable
	if( !IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal) )
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if( CTransaction::Init() )
	// }}
	{
		// Register Interface with Zombie
		if( RegisterInterface(COMMAND_INTERFACE, IID_ICommandText, 0, NULL) )
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if( !m_pITransactionLocal )
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if( m_pITransactionLocal == INVALID(ITransactionLocal*) )
		m_pITransactionLocal = NULL;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandText::SetCommandText with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_1()
{
	return TestTxn(EMETHOD_SETTEXT, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandText::SetCommandText with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_2()
{
	return TestTxn(EMETHOD_SETTEXT, ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandText::SetCommandText with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_3()
{
	return TestTxn(EMETHOD_SETTEXT, ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandText::SetCommandText with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_4()
{
	return TestTxn(EMETHOD_SETTEXT, ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandText::GetCommandText with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_5()
{
	return TestTxn(EMETHOD_GETTEXT, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandText::GetCommandText with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_6()
{
	return TestTxn(EMETHOD_GETTEXT, ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandText::GetCommandText with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_7()
{
	return TestTxn(EMETHOD_GETTEXT, ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandText::GetCommandText with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_8()
{
	return TestTxn(EMETHOD_GETTEXT, ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Terminate()
{
	return(CTransaction::Terminate());
}	// }}
// }}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to ICommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::TestTxn
(
	EMETHOD eMethod,
	ETXN eTxn,
	BOOL fRetaining
)
{
	TBEGIN;

	HRESULT			Exp_Sethr		= DB_E_OBJECTOPEN;
	DBCOUNTITEM 		cRowsObtained	= 0;

	ICommandText *	pICommandText	= NULL;
	WCHAR *			pSQLSet			= NULL;
	WCHAR *			pSQLGet			= NULL;
	GUID			GetguidDialect	= DBGUID_DEFAULT;
	GUID			SetguidDialect	= DBGUID_DEFAULT;
	
	ICommandPrepare*pICommandPrepare= NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	DBORDINAL		cDBCOLUMNINFO	= 0;
	DBCOLUMNINFO *	rgDBCOLUMNINFO	= NULL;
	WCHAR *			pStringsBuffer	= NULL;
	
	// Initialize and Start a Transaction
	m_rghRows = NULL;

	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pICommandText,
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE));
		
	// Commit or Abort the transaction, with retention as specified
	if( eTxn == ETXN_COMMIT ) 
	{
		TESTC(GetCommit(fRetaining));
	}
	else 
	{
		TESTC(GetAbort(fRetaining));
	}

	// Test zombie
	if( (!m_fAbortPreserve  && eTxn == ETXN_ABORT) ||
		(!m_fCommitPreserve && eTxn == ETXN_COMMIT) ) 
	{
		TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	}
	else 
	{
		TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
	}

	// Create a SQL Stmt and Set the Command
	TESTC_(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
												NULL, &pSQLSet, NULL, NULL), S_OK);
	// Get a IColumnsInfo pointer
	TESTC(VerifyInterface(m_pICommand,IID_IColumnsInfo,
							COMMAND_INTERFACE,(IUnknown **)&pIColumnsInfo));

	// SetText Method is tested
	if( eMethod == EMETHOD_SETTEXT )
	{
		Exp_Sethr = S_OK;
		TESTC_(pICommandText->SetCommandText(SetguidDialect, 
														pSQLSet), DB_E_OBJECTOPEN);
		CleanupTransactionRowset();
	}

	// Set Command Text	and expect ColumnsInfo to return S_OK
	TESTC_(pICommandText->SetCommandText(SetguidDialect, pSQLSet), Exp_Sethr);
	TESTC_(pICommandText->GetCommandText(&GetguidDialect, &pSQLGet), S_OK);

	if( eMethod == EMETHOD_SETTEXT )
	{
		TESTC_(pIColumnsInfo->GetColumnInfo(&cDBCOLUMNINFO, 
							&rgDBCOLUMNINFO, &pStringsBuffer), DB_E_NOTPREPARED);
	}
	else 
	{
		TEST2C_(m_hr=pIColumnsInfo->GetColumnInfo(&cDBCOLUMNINFO, 
						&rgDBCOLUMNINFO, &pStringsBuffer),S_OK, DB_E_NOTPREPARED);

		// Verify the property
		if( (eTxn == ETXN_COMMIT &&
			 SupportedProperty(DBPROP_PREPARECOMMITBEHAVIOR,
							DBPROPSET_DATASOURCEINFO,m_pIDBCreateSession)) ||
			(eTxn == ETXN_ABORT &&
			 SupportedProperty(DBPROP_PREPAREABORTBEHAVIOR,
								DBPROPSET_DATASOURCEINFO,m_pIDBCreateSession)) )
		{
			// Check the HResult
			if( (m_fPrepareCommitPreserve && eTxn == ETXN_COMMIT) ||
				(m_fPrepareAbortPreserve  && eTxn == ETXN_ABORT ) )
				CHECK(m_hr, S_OK);
			else
				CHECK(m_hr, DB_E_NOTPREPARED);
		}
		else
		{
			// Post a warning if DBPROPVAL_CB_PRESERVE is valid
			if( SUCCEEDED(m_hr) )
			{
				if( eTxn == ETXN_COMMIT )
					TWARNING("DBPROP_PREPARECOMMITBEHAVIOR is not supported?")
				else
					TWARNING("DBPROP_PREPAREABORTBEHAVIOR is not supported?")
			}
		}
	}

	// Verify the Dialect
	COMPARE(SetguidDialect, DBGUID_DEFAULT);
	COMPARE(GetguidDialect, DBGUID_DEFAULT);

	// Verify the text
	TESTC(!wcscmp(pSQLSet, pSQLGet));

CLEANUP:

	// Release objects
	if( m_rghRows ) {
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICommandPrepare);

	PROVIDER_FREE(pSQLSet);
	PROVIDER_FREE(pSQLGet);

	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);

	// Return code of Commit/Abort will vary depending on whether
	// or not we have an open txn, so adjust accordingly
	if( fRetaining )
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);

	TRETURN;
}

void CZombie::CleanupTransactionRowset()
{
	// Release the Rowset Objects
	if( m_rghRows ) {
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetInfo);
	SAFE_RELEASE(m_pIColumnsInfo);
	SAFE_RELEASE(m_pIAccessor);
}

// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		07/04/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors::Init()
{
  	return CCmdText::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid ICommandText calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	TBEGIN;

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

   	// For each method of the interface, first create an error object on
	// the current thread, then try get S_OK from the ICommandText method.
	// We then check extended errors to verify nothing is set since an 
	// error object shouldn't exist following a successful call.
	m_pExtError->CauseError();

	// Do extended check following SetCommandText
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect, m_pSQLSet),S_OK);
	
	//Do extended check following CreateAccessor
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	
	m_pExtError->CauseError();
	
	// Do extended check following GetCommandText
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect, &m_pSQLGet),S_OK);
	
	//Do extended check following CreateAccessor
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid ICommandText calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	TBEGIN;

	DBROWCOUNT	cRowsAffected	= 0;			// count of rowsets
	IRowset *	pIRowset		= NULL;			// rowset info
	WCHAR *		pSQLSet1		= NULL;

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// For each method of the interface, first create an error object on
	// the current thread, then try get a failure from the ICommandText method.
	// We then check extended errors to verify the right extended error behavior.
  
	// Do SetCommandText and then cause an Error
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect, m_pSQLSet),S_OK);
	m_pExtError->CauseError();

	// Do extended check following GetCommandText
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect, NULL),E_INVALIDARG);
	
	//Do extended check following GetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);
	TESTC(VerifyResults());

	// Do SetCommandText
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect, m_pSQLSet),S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**) &pIRowset), S_OK);

	m_PrepareState = IMPLICITPREPARE;

	// Check to see if the Command is Prepared and check pointer
	TESTC(VerifyResults());
	TESTC(pIRowset != NULL);

	// Create a SQL Stmt and Set new Command Text
	TESTC(GetDefaultText(&pSQLSet1));
	m_pExtError->CauseError();

	// try to set stmt
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,pSQLSet1), DB_E_OBJECTOPEN);

 	//Do extended check following SetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	

	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet), DB_S_DIALECTIGNORED);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Rowset object must be closed in order to check state of command object
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(pSQLSet1);

	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid ICommandText calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	TBEGIN;

	DBROWCOUNT	cRowsAffected	= 0;			// count of rowsets
	IRowset *	pIRowset		= NULL;			// rowset info
	WCHAR *		pSQLSet1		= NULL;

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// For each method of the interface, with no error object on
	// the current thread, try get a failure from the ICommandText method.
	// We then check extended errors to verify the right extended error behavior.
  
	// Do SetCommandText
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Do extended check following GetCommandText
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,NULL),E_INVALIDARG);
	
	//Do extended check following GetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	

	// Do SetCommandText
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),S_OK);

	// Execute command
	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, 
						NULL, &cRowsAffected, (IUnknown**) &pIRowset), S_OK);

	m_PrepareState = IMPLICITPREPARE;

	// Check to see if the Command is Prepared and check pointer
	TESTC(VerifyResults());
	TESTC(pIRowset != NULL);

	// Create a SQL Stmt and Set new Command Text
	TESTC(GetDefaultText(&pSQLSet1));

 	//Do extended check following SetCommandText
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,pSQLSet1),DB_E_OBJECTOPEN);

	//Do extended check following GetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	

	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_S_DIALECTIGNORED);

	// Verify the Dialect
	COMPARE(m_SetguidDialect, m_DBGUID_DEFAULT);
	TESTC(VerifyDialect());

	// Verify the text
	TESTC(CompareText());
	TESTC(VerifyResults());

CLEANUP:

	// Rowset object must be closed in order to check state of command object
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(pSQLSet1);

	TERM;
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND SetCommandText calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	TBEGIN;

	INIT;

	// Create a SQL Stmt and Set the Command
	TESTC(GetDefaultText(&m_pSQLSet));
	TESTC(GetICommandText());

	// For each method of the interface, with no error object on
	// the current thread, try get a failure from the ICommandText method.
	// We then check extended errors to verify the right extended error behavior.

	// Clear Text
	TESTC_(m_pICommandText->SetCommandText(m_SetguidDialect, NULL),S_OK);
	TESTC_(m_hr=m_pICommandText->GetCommandText(&m_GetguidDialect,&m_pSQLGet),DB_E_NOCOMMAND);
	TESTC(!m_pSQLGet);
	
 	//Do extended check following GetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	

	// Set text, but m_guidDialect and m_pSQLSet are NULL
	m_SetguidDialect = GUID_NULL;
	TESTC_(m_hr=m_pICommandText->SetCommandText(m_SetguidDialect,m_pSQLSet),DB_E_DIALECTNOTSUPPORTED);
	// Do extended check following SetCommandText
	XCHECK(m_pICommandText, IID_ICommandText, m_hr);	
	
CLEANUP:

	TERM;
	TRETURN;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	return (CCmdText::Terminate());

}	
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors_GlobalCmd::Init()
{
	if(TCExtendedErrors::Init())
	{
		m_pActiveIDBCreateCommand = m_pIDBCreateCommand;
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

			// Create a SQL Stmt and Set the Command
		TESTC_(m_hr=m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pDefaultSetText, NULL, NULL),S_OK);
		return CCmdText::PostInit();		
	}

CLEANUP:
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_GlobalCmd::Terminate()
{
	return (TCExtendedErrors::Terminate());

}	


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors_RowCmd::Init()
{
	if(TCExtendedErrors::Init())
	{
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		
		return InitializeRowObject();
	}

	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_RowCmd::Terminate()
{
	SAFE_DELETE(m_pCRowObject);
	return (TCExtendedErrors::Terminate());
}	