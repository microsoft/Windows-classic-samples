//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ICMDPREP.CPP | Source code for ICommandPrepare.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "icmdprep.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x8f3ec2ae, 0x34b3, 0x11cf, { 0x98, 0xb8, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("ICommandPrepare");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for ICommandPrepare interfaces");
DECLARE_MODULE_VERSION(832456171);
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
	BOOL	bValue = FALSE;

	IDBCreateSession * pIDBCreateSession = NULL;
	IDBCreateCommand * pIDBCreateCommand = NULL;
	ICommand *		   pICommand	     = NULL;
	ICommandPrepare *  pICommandPrepare  = NULL;

	if(ModuleCreateDBSession(pThisTestModule))
	{
		// IDBCreateSession
		if(!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&pIDBCreateSession))
			goto CLEANUP;
		
		// IDBCreateCommand
		if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
		{
			odtLog << L"IDBCreateCommand is not supported by Provider." << ENDL;
			bValue = TEST_SKIPPED;
			goto CLEANUP;
		}

		// Create a Command object
		pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,(IUnknown **)&pICommand);
		if(pICommand)
		{
			// IDBCreateCommand
			if(!VerifyInterface(pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&pICommandPrepare))
			{
				odtLog << L"ICommandPrepare is not supported by Provider." << ENDL;
				bValue = TEST_SKIPPED;
				goto CLEANUP;
			}

			// If we get here we passed
			bValue = TRUE;
		}
	}

CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandPrepare);

	return bValue;
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
	//Free the interface we got in ModuleCreateDBSession()
	return ModuleReleaseDBSession(pThisTestModule);
}	

    
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CCommand Base Class for all ICommandPrepare Testcases
class CCommand : public COLEDB 
{
	public:
		// @cmember Constructor
		CCommand(LPWSTR wszTestCaseName) : COLEDB(wszTestCaseName)
		{
		m_pIDBCreateSession	= NULL;
		m_pIDBCreateCommand = NULL;
		m_pCTable			= NULL;
		m_pCTCTbl			= NULL;
		};

		// @cmember Destructor
		virtual ~CCommand(){};

	protected:	
		// @cmember Common base class initialization
		virtual BOOL Init();
		// @cmember Common base class termination
		virtual BOOL Terminate();
		// @mfunc Can the data type hold Numeric with a Scale		
		BOOL IsColNumWithScale
		(
			DBTYPE wType,			// @parm [IN] Data type
			ULONG  bScale			// @parm [IN] precision for the data type
		);
		// @mfunc Can the data type hold string values		
		BOOL IsColCharacter
		(
			DBTYPE wType,			// @parm [IN] Data type
			DBLENGTH ulColumnSize	// @parm [IN] precision for the data type
		);
		// @mfunc Can the data type hold Date values		
		BOOL IsColDateTime
		(
			DBTYPE wType			// @parm [IN] Data type
		);
		//@member	Interface for DBSession Initialization
		IDBCreateSession*	m_pIDBCreateSession;
		//@cmember	Interface for DB Session, gotten by GetDBSession
		IDBCreateCommand* 	m_pIDBCreateCommand;
		//@cmember CTable object
		CTable*		m_pCTable;	
		//@cmember CTable object
		CTable*		m_pCTCTbl;	
};


// @class CCommandZombie Base Class for all ICommandPrepare Transaction Testcases
class CCommandZombie : public CTransaction 
{
	public:
		// @cmember Constructor
		CCommandZombie(LPWSTR wszTestCaseName) : CTransaction(wszTestCaseName)
		{
		m_pICmdPrepare		= NULL;
		m_pIColInfo			= NULL;
		m_pTableName		= NULL;
		m_pwszSQLStmt		= NULL;
		m_rghRows			= NULL;
		m_rgInfo			= NULL;
		m_pStringsBuffer	= NULL;
		};

		// @cmember Destructor
		virtual ~CCommandZombie(){};

	protected:	
		// @cmember Common base class initialization
		virtual BOOL Init();
		// @cmember Common base class termination
		virtual BOOL Terminate();
		//@mfunc	Cleanup the Rowsets in the Transaction
		void CleanupTransactionRowset(EINTERFACE eInterface, BOOL fRetaining);
		//@member	Interface for ICommandPrepare
		ICommandPrepare *	m_pICmdPrepare;
		//@member	Interface for IColumnsInfo
		IColumnsInfo *		m_pIColInfo;
		//@member	Interface for IColumnsInfo structs
		DBCOLUMNINFO *		m_rgInfo;
		//@member	Interface for IColumnsInfo strings
		WCHAR *				m_pStringsBuffer;
		//@member	Interface for IColumnsInfo
		HROW *				m_rghRows;
		//@cmember CTablename
		WCHAR *				m_pTableName;
		//@cmember SQL Statement
		WCHAR *				m_pwszSQLStmt;
};


//--------------------------------------------------------------------
// @mfunc Base class Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommand::Init()
{
	BOOL				fSuccess			= FALSE;
	ICommand*			pICommand			= NULL;
	ICommandPrepare*	pICommandPrepare	= NULL;

	if (COLEDB::Init())
	{
		//IDBCreateSession
		if(!VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession))
			return FALSE;
		
		//IDBCreateCommand
		if(!VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand))
			goto END;

		// Get a ICommand object
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
												(IUnknown **)&pICommand), S_OK))
			goto END;

		// Check to see if the provider supports ICommandPrepare
		if (!VerifyInterface(pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown **)&pICommandPrepare))
			goto END;

		// Create a table we'll use for the whole test case.
		m_pCTable = new CTable(m_pIDBCreateCommand, (LPWSTR)gwszModuleName, USENULLS);
		if (!m_pCTable)
		{
			odtLog << wszMemoryAllocationError;
			goto END;
		}

		// Create a 2nd table we'll use for the whole test case.
		m_pCTCTbl = new CTable(m_pIDBCreateCommand, (LPWSTR)gwszModuleName, USENULLS);
		if (!m_pCTCTbl)
		{
			odtLog << wszMemoryAllocationError;
			goto END;
		}

		fSuccess = TRUE;
	}  
END:
	
	// Release objects
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandPrepare);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Base Case Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommand::Terminate()
{
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pIDBCreateCommand);

	SAFE_DELETE(m_pCTable);
	SAFE_DELETE(m_pCTCTbl);

	return(COLEDB::Terminate());
}


//--------------------------------------------------------------------
// @mfunc Base class Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommandZombie::Init()
{
	return(CTransaction::Init());	
}


//--------------------------------------------------------------------
// @mfunc Base Case Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommandZombie::Terminate()
{
	return(CTransaction::Terminate());
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(ICommandPrepare_Invalid_Cases)
//--------------------------------------------------------------------
// @class Invalid test variations for method ICommand::Prepare
//
class ICommandPrepare_Invalid_Cases : public CCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandPrepare_Invalid_Cases,CCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists [S0001]
	int Variation_1();
	// @cmember DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists [S0001]
	int Variation_2();
	// @cmember DB_E_ERRORSINCOMMAND - Select statement with invalid table name [S0002]
	int Variation_3();
	// @cmember DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist [S0002]
	int Variation_4();
	// @cmember DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist [S0002]
	int Variation_5();
	// @cmember DB_E_ERRORSINCOMMAND - Select statement with invalid column name [S0022]
	int Variation_6();
	// @cmember DB_E_NOCOMMAND - Prepare a Empty text string [S1009]
	int Variation_7();
	// @cmember DB_E_NOCOMMAND - Prepare after a NULL ppwszCommand is set
	int Variation_8();
	// @cmember S_OK - Numeric Truncation [01004]
	int Variation_9();
	// @cmember DB_E_DATAOVERFLOW - Arithmetic overflow [22003]
	int Variation_10();
	// @cmember DB_E_DATAOVERFLOW - String Right Truncation [22001]
	int Variation_11();
	// @cmember DB_E_CANTCONVERTVALUE - String Right Truncation [22005]
	int Variation_12();
	// @cmember DB_E_CANTCONVERTVALUE - Implicit conversion [22005]
	int Variation_13();
	// @cmember DB_E_CANTCONVERTVALUE - Implicit conversion [22008]
	int Variation_14();
	// @cmember DB_E_ERRORSINCOMMAND - Invalid nodes in a command [37000]
	int Variation_15();
	// @cmember DB_E_NOCOMMAND - Prepare before setting text
	int Variation_16();
	// @cmember DB_E_OBJECTOPEN - Prepare with an open Rowset Object
	int Variation_17();
	// @cmember DB_E_ERRORSINCOMMAND - Prepare an invalid escape clause
	int Variation_18();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandPrepare_Invalid_Cases)
#define THE_CLASS ICommandPrepare_Invalid_Cases
BEG_TEST_CASE(ICommandPrepare_Invalid_Cases, CCommand, L"Invalid test variations for method ICommand::Prepare")
	TEST_VARIATION(1, 		L"DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists [S0001]")
	TEST_VARIATION(2, 		L"DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists [S0001]")
	TEST_VARIATION(3, 		L"DB_E_ERRORSINCOMMAND - Select statement with invalid table name [S0002]")
	TEST_VARIATION(4, 		L"DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist [S0002]")
	TEST_VARIATION(5, 		L"DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist [S0002]")
	TEST_VARIATION(6, 		L"DB_E_ERRORSINCOMMAND - Select statement with invalid column name [S0022]")
	TEST_VARIATION(7, 		L"DB_E_NOCOMMAND - Prepare a Empty text string [S1009]")
	TEST_VARIATION(8, 		L"DB_E_NOCOMMAND - Prepare after a NULL ppwszCommand is set")
	TEST_VARIATION(9, 		L"S_OK - Numeric Truncation [01004]")
	TEST_VARIATION(10, 		L"DB_E_DATAOVERFLOW - Arithmetic overflow [22003]")
	TEST_VARIATION(11, 		L"DB_E_DATAOVERFLOW - String Right Truncation [22001]")
	TEST_VARIATION(12, 		L"DB_E_CANTCONVERTVALUE - String Right Truncation [22005]")
	TEST_VARIATION(13, 		L"DB_E_CANTCONVERTVALUE - Implicit conversion [22005]")
	TEST_VARIATION(14, 		L"DB_E_CANTCONVERTVALUE - Implicit conversion [22008]")
	TEST_VARIATION(15, 		L"DB_E_ERRORSINCOMMAND - Invalid nodes in a command [37000]")
	TEST_VARIATION(16, 		L"DB_E_NOCOMMAND - Prepare before setting text")
	TEST_VARIATION(17, 		L"DB_E_OBJECTOPEN - Prepare with an open Rowset Object")
	TEST_VARIATION(18, 		L"DB_E_ERRORSINCOMMAND - Prepare an invalid escape clause")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandPrepare_Valid_Cases)
//--------------------------------------------------------------------
// @class Valid test variations for method ICommandPrepare::Prepare
//
class ICommandPrepare_Valid_Cases : public CCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandPrepare_Valid_Cases,CCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Select statement with all columns
	int Variation_1();
	// @cmember S_OK - Prepare after IColumnsInfo::GetColumnInfo FAILS
	int Variation_2();
	// @cmember S_OK - IColumnsInfo::GetColumnInfo after Prepare
	int Variation_3();
	// @cmember S_OK - Prepare, SetCommandText, and IColumnsInfo
	int Variation_4();
	// @cmember S_OK - Prepare Fails, and IColumnsInfo
	int Variation_5();
	// @cmember S_OK - Prepare, Execute Fails, and IColumnsInfo
	int Variation_6();
	// @cmember S_OK - Prepare and then try to set Properties
	int Variation_7();
	// @cmember S_OK - Prepare an valid escape clause
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandPrepare_Valid_Cases)
#define THE_CLASS ICommandPrepare_Valid_Cases
BEG_TEST_CASE(ICommandPrepare_Valid_Cases, CCommand, L"Valid test variations for method ICommandPrepare::Prepare")
	TEST_VARIATION(1, 		L"S_OK - Select statement with all columns")
	TEST_VARIATION(2, 		L"S_OK - Prepare after IColumnsInfo::GetColumnInfo FAILS")
	TEST_VARIATION(3, 		L"S_OK - IColumnsInfo::GetColumnInfo after Prepare")
	TEST_VARIATION(4, 		L"S_OK - Prepare, SetCommandText, and IColumnsInfo")
	TEST_VARIATION(5, 		L"S_OK - Prepare Fails, and IColumnsInfo")
	TEST_VARIATION(6, 		L"S_OK - Prepare, Execute Fails, and IColumnsInfo")
	TEST_VARIATION(7, 		L"S_OK - Prepare and then try to set Properties")
	TEST_VARIATION(8, 		L"S_OK - Prepare an valid escape clause")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandPrepare_Trans_Cases)
//--------------------------------------------------------------------
// @class Transaction test variations for method ICommandPrepare::Prepare
//
class ICommandPrepare_Trans_Cases : public CCommandZombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandPrepare_Trans_Cases,CCommandZombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort ICommandPrepare with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit ICommandPrepare with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Abort ICommandPrepare with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit ICommandPrepare with fRetaining=FALSE
	int Variation_4();
	// @cmember DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=TRUE
	int Variation_5();
	// @cmember DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=TRUE
	int Variation_6();
	// @cmember DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=FALSE
	int Variation_7();
	// @cmember DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandPrepare_Trans_Cases)
#define THE_CLASS ICommandPrepare_Trans_Cases
BEG_TEST_CASE(ICommandPrepare_Trans_Cases, CCommandZombie, L"Transaction test variations for method ICommandPrepare::Prepare")
	TEST_VARIATION(1, 		L"S_OK - Abort ICommandPrepare with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit ICommandPrepare with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort ICommandPrepare with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit ICommandPrepare with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=TRUE")
	TEST_VARIATION(7, 		L"DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=FALSE")
	TEST_VARIATION(8, 		L"DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandUnprepare_Invalid_Cases)
//--------------------------------------------------------------------
// @class Invalid test variations for method ICommandPrepare::Unprepare
//
class ICommandUnprepare_Invalid_Cases : public CCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandUnprepare_Invalid_Cases,CCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_OBJECTOPEN - Unprepare with an open Rowset Object
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandUnprepare_Invalid_Cases)
#define THE_CLASS ICommandUnprepare_Invalid_Cases
BEG_TEST_CASE(ICommandUnprepare_Invalid_Cases, CCommand, L"Invalid test variations for method ICommandPrepare::Unprepare")
	TEST_VARIATION(1, 		L"DB_E_OBJECTOPEN - Unprepare with an open Rowset Object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandUnprepare_Valid_Cases)
//--------------------------------------------------------------------
// @class Valid test variations for method ICommandPrepare::Unprepare
//
class ICommandUnprepare_Valid_Cases : public CCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandUnprepare_Valid_Cases,CCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Unprepare a Select statement with all columns
	int Variation_1();
	// @cmember S_OK - Unprepare and the IColumnsInfo::GetColumnInfo should FAILS
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandUnprepare_Valid_Cases)
#define THE_CLASS ICommandUnprepare_Valid_Cases
BEG_TEST_CASE(ICommandUnprepare_Valid_Cases, CCommand, L"Valid test variations for method ICommandPrepare::Unprepare")
	TEST_VARIATION(1, 		L"S_OK - Unprepare a Select statement with all columns")
	TEST_VARIATION(2, 		L"S_OK - Unprepare and the IColumnsInfo::GetColumnInfo should FAILS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandUnprepare_Trans_Cases)
//--------------------------------------------------------------------
// @class Transaction test variations for method ICommandPrepare::Unprepare
//
class ICommandUnprepare_Trans_Cases : public CCommandZombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandUnprepare_Trans_Cases,CCommandZombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort ICommandUnprepare with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit ICommandUnprepare with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Abort ICommandUnprepare with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit ICommandUnprepare with fRetaining=FALSE
	int Variation_4();
	// @cmember DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=TRUE
	int Variation_5();
	// @cmember DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=TRUE
	int Variation_6();
	// @cmember DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=FALSE
	int Variation_7();
	// @cmember DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandUnprepare_Trans_Cases)
#define THE_CLASS ICommandUnprepare_Trans_Cases
BEG_TEST_CASE(ICommandUnprepare_Trans_Cases, CCommandZombie, L"Transaction test variations for method ICommandPrepare::Unprepare")
	TEST_VARIATION(1, 		L"S_OK - Abort ICommandUnprepare with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit ICommandUnprepare with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort ICommandUnprepare with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit ICommandUnprepare with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=TRUE")
	TEST_VARIATION(7, 		L"DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=FALSE")
	TEST_VARIATION(8, 		L"DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ICommandPrep_ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ICommandPrep_ExtendedErrors : public CCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ICommandPrep_ExtendedErrors,CCommand);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid ICommandPrepare calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid ICommandPrepare calls with previous error object existing
	int Variation_2();
	// @cmember Invalid Prepare call with no previous error object existing
	int Variation_3();
	// @cmember Invalid Unprepare call with no previous error object existing
	int Variation_4();
	// @cmember DB_E_ERRORINCOMMAND call with no previous error object existing
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ICommandPrep_ExtendedErrors)
#define THE_CLASS ICommandPrep_ExtendedErrors
BEG_TEST_CASE(ICommandPrep_ExtendedErrors, CCommand, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid ICommandPrepare calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid ICommandPrepare calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid Prepare call with no previous error object existing")
	TEST_VARIATION(4, 		L"Invalid Unprepare call with no previous error object existing")
	TEST_VARIATION(5, 		L"DB_E_ERRORINCOMMAND call with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, ICommandPrepare_Invalid_Cases)
	TEST_CASE(2, ICommandPrepare_Valid_Cases)
	TEST_CASE(3, ICommandPrepare_Trans_Cases)
	TEST_CASE(4, ICommandUnprepare_Invalid_Cases)
	TEST_CASE(5, ICommandUnprepare_Valid_Cases)
	TEST_CASE(6, ICommandUnprepare_Trans_Cases)
	TEST_CASE(7, ICommandPrep_ExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(ICommandPrepare_Invalid_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandPrepare_Invalid_Cases - Invalid test variations for method ICommand::Prepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Invalid_Cases::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCommand::Init())
	// }}
	{
		// Create a table
		if (CHECK(m_pCTable->CreateTable(5, 1, NULL, PRIMARY),S_OK))
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists [S0001]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = m_pCTable->GetTableName();

	// Format SQL Statement
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszCreateTable) + wcslen(pTableName))));
	swprintf(pwszSQLStmt,wszCreateTable,pTableName);

	// Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists [S0001]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pViewName		= NULL;		// Name of the view
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Drop View just in case if it exists, do not check error here 
	//in case of CONFPROV when tabledump generating SQL Statements it creates the View
	m_pCTable->CreateSQLStmt(DROP_VIEW, 
										NULL, &pwszSQLStmt, NULL, NULL);
	m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);
	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	
	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(CREATE_VIEW, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Create View
	hr = m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);

	// Check to see if the create view failed
	if (FAILED(hr)) {
		odtLog<<L"Create view not supported" <<ENDL;
		fSuccess = TRUE;
		goto END;
	}

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Drop View
	m_pCTable->DropView();

	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Select statement with invalid table name [S0002]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_3()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_INVALIDTBLNAME, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT.  Some providers may return DB_E_NOTABLE here, which we
	// will allow as it's more descriptive.
	if (hr == S_OK)
	{
		hr = m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);
	}

	if (hr == DB_E_ERRORSINCOMMAND || CHECK(hr, DB_E_NOTABLE))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist [S0002]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_4()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	WCHAR*				pszStartTblName = NULL;

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}


	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTCTbl->CreateSQLStmt(DROP_TABLE, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Drop the table, also deletes the table name
	m_pCTCTbl->DropTable();

	//In the case of ini file pwszSQLStmt can still contain the name of main table
	//confprov.ini contains DROP_TABLE with the main table name
	// Just in case Replace the TableName
	pszStartTblName = wcsstr(pwszSQLStmt, m_pCTable->GetTableName());
	if (pszStartTblName)
		*pszStartTblName = L'X';


	// Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_NOTABLE))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist [S0002]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_5()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pViewName		= NULL;		// Name of the view
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

/*
	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(CREATE_VIEW, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Create View
	hr = m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);

	// check to see if the create view failed
	if (FAILED(hr))
	{
		odtLog<<L"Create view not supported" <<ENDL;
		fSuccess = TRUE;
		goto END;
	}

	// Drop View
	m_pCTable->DropView();
	PROVIDER_FREE(pwszSQLStmt);
*/
	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(DROP_VIEW, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Drop View just in case if it exists, do not check error here 
	//In case of CONFPROV when tabledump generating SQL Statements it creates the View
	//The second time whn executing DROP_VIEW we should receive error
	m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_NOTABLE))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Select statement with invalid column name [S0022]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_6()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pTableName		= NULL;		// Name of the table
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Get the name of the table just created
	pTableName = m_pCTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * 
				  (wcslen(wszSelectBadColName) + wcslen(pTableName))) + sizeof(WCHAR) );

	// Put the SQL statement together
	swprintf(pwszSQLStmt, wszSelectBadColName, pTableName);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND - Prepare a Empty text string [S1009]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_7()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandText*		pICommandText	= NULL;		// ICommandText Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR));

	// Make a Empty String Command
	wcscpy(pwszSQLStmt, L"\0");

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// Get an ICommandText object
	if (!CHECK(pICommand->QueryInterface(IID_ICommandText,(void **)&pICommandText),S_OK))
		goto END;

	// Set the SQL statement
	if (!CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQLStmt),S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Prepare(1), DB_E_NOCOMMAND))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND - Prepare after a NULL ppwszCommand is set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_8()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandText*		pICommandText	= NULL;		// ICommandText Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// Get an ICommandText object
	if (!CHECK(pICommand->QueryInterface(IID_ICommandText,(void **)&pICommandText),S_OK))
		goto END;

	// Set the SQL statement
	if (!CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, NULL),S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Prepare(1), DB_E_NOCOMMAND))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Numeric Truncation [01004]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_9()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				ColScale		= 0;		// Column Scale
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR* ,WCHAR*> NativeTypesList;
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}
	
	//If we use ini file m_pCTCTbl->CreateTable doesn' t create a table but uses the table name from ini file
	//so we can not really create a table with one column
	//so we'll skip this variation otherwise it'll always fail in the case of ini file
	if(GetModInfo()->GetFileName())
	{
		odtLog << L".ini file is used!!! Can not create a table with one column in this case" << ENDL;
		return TEST_SKIPPED;
	}

	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if (IsColNumWithScale(NewCol.GetProviderType(),
							  NewCol.GetScale()))
			break;
	}

	// If No numeric column, skip out of the test
	if(count > pcColumns)
	{
		odtLog << L"Provider does not support a Numeric Column." << ENDL;
		return TEST_SKIPPED;
	}

	NativeTypesList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if(!CHECK(m_pCTCTbl->CreateTable(NativeTypesList,1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get Numeric Scale
	ColScale = NewCol.GetScale();

	// Alloc Memory
	pwszNumeric = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
										  (sizeof(WCHAR)*(ColScale+3)));

	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidValue) + wcslen(pTableName) + (ColScale+3))));

	// Create Numeric Valus out of range
	wcscpy(pwszNumeric, L"1.");
	for(count=1; count <= ColScale+1; count++)
		wcscat(pwszNumeric, L"1");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidValue, pTableName, pwszNumeric);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), S_OK))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, S_OK))
			fSuccess = TRUE;
	}

END:
	// Free memory in the list
	NativeTypesList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszNumeric);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DATAOVERFLOW - Arithmetic overflow [22003]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_10()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				ColPrec			= 0;		// Column Precision
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR *, WCHAR *> DBTypeList;
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if( (IsColNumWithScale(NewCol.GetProviderType(),NewCol.GetScale())) &&
			(NewCol.GetUpdateable()) )
			break;
	}

	// If No numeric column, skip out of the test
	if(count > pcColumns)
	{
		odtLog << L"Provider does not support an Updatable Numeric Column." << ENDL;
		m_pCTCTbl->DropTable();
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(DBTypeList,1,1,NULL,PRIMARY),	S_OK))
		goto END;

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get Numeric Precision
	ColPrec = NewCol.GetPrecision();

	// Alloc Memory
	pwszNumeric= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
										 (sizeof(WCHAR)*(ColPrec+1)));

	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidValue) + wcslen(pTableName) + ColPrec+1)));

	// Create Numeric Valus out of range
	wcscpy(pwszNumeric, L"\0");
	for(count=1; count <= ColPrec+1; count++)
		wcscat(pwszNumeric, L"9");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidValue, pTableName, pwszNumeric);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_DATAOVERFLOW))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Free memory in the list
	DBTypeList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszNumeric);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DATAOVERFLOW - String Right Truncation [22001]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_11()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pwszSQLStmt1	= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pTableNCpy		= NULL;		// Copy of the Table Name
	WCHAR *				pwszValue		= NULL;		// String value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	DBLENGTH			ColSize			= 0;		// Column Size
	ULONG				count			= 0;		// Loop counter
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Creates a column list from the Ctable
	pcColumns = m_pCTCTbl->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTCTbl->GetColInfo(count, NewCol);
		
		// If first column is already character then were done
		if (IsColCharacter(NewCol.GetProviderType(),
						   NewCol.GetColumnSize()))
			break;
	}
	
	// If no string column, skip out of the test
	if((count > pcColumns) || (!NewCol.GetProviderTypeName()))
	{
		odtLog << L"Provider does not support a String Column " << ENDL;
		odtLog << L"or the provider does not support type names." << ENDL;
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get a copy of the table name
	pTableNCpy = (WCHAR *) PROVIDER_ALLOC((wcslen(pTableName) *
					sizeof(WCHAR)) + sizeof(WCHAR));
	wcscpy(pTableNCpy, pTableName);

	// Get Column Size
	ColSize = NewCol.GetMaxSize();

	// Create a table
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszCreateStringTable) + wcslen(pTableNCpy) + 
				  (NewCol.GetProviderTypeName() ? wcslen(NewCol.GetProviderTypeName()) :0) + 3)));

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszCreateStringTable, pTableNCpy, 
			(NewCol.GetProviderTypeName() ? NewCol.GetProviderTypeName() :L""), ColSize-1);

	// Drop the table, also deletes the table name
	m_pCTCTbl->DropTable();

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// Set the TableName
	m_pCTCTbl->SetTableName(pTableNCpy);

	// Alloc Memory
	pwszValue= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
									   (sizeof(WCHAR)*(ColSize)));

	pwszSQLStmt1= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidCharValue) + wcslen(pTableNCpy) + ColSize)));

	// Create String value out of range
	wcscpy(pwszValue, L"\0");
	for(count=1; count <= ColSize; count++)
		wcscat(pwszValue, L"A");

	// Format SQL Statement
	swprintf(pwszSQLStmt1, wszInsertInvalidCharValue, pTableNCpy, pwszValue);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt1, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_DATAOVERFLOW))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pTableNCpy);
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszSQLStmt1);
	PROVIDER_FREE(pwszValue);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE - String Right Truncation [22005]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_12()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszValue		= NULL;		// String value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	DBLENGTH			ColSize			= 0;		// Column Size
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR* ,WCHAR*> NativeTypesList;
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already character then were done
		if (IsColCharacter(NewCol.GetProviderType(),
						   NewCol.GetColumnSize()))
			break;
	}

	// If no string column, skip out of the test
	if((count > pcColumns) || (!NewCol.GetProviderTypeName()))
	{
		odtLog << L"Provider does not support a String Column." << ENDL;
		odtLog << L"or the provider does not support type names." << ENDL;
		return TEST_SKIPPED;
	}

	NativeTypesList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(NativeTypesList,1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get Column Size
	ColSize = NewCol.GetMaxSize();

	// Alloc Memory
	pwszValue= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
									   (sizeof(WCHAR)*(ColSize+1)));

	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidCharValue) + wcslen(pTableName) + ColSize+1)));

	// Create String value out of range
	wcscpy(pwszValue, L"\0");
	for(count=1; count <= ColSize+1; count++)
		wcscat(pwszValue, L"A");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidCharValue, pTableName, pwszValue);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_DATAOVERFLOW))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Free memory in the list
	NativeTypesList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszValue);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE - Implicit conversion [22005]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_13()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR *, WCHAR *> DBTypeList;
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if ((IsNumericType(NewCol.GetProviderType())) &&
			(NewCol.GetUpdateable()))
			break;
	}

	// If No numeric column, skip out of the test
	if(count > pcColumns|| (!NewCol.GetProviderTypeName()))
	{
		odtLog << L"Provider does not support an Updatable Numeric Column." << ENDL;
		odtLog << L"or the provider does not support type names." << ENDL;
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(DBTypeList,1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidChar) + wcslen(pTableName))));

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidChar, pTableName);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_CANTCONVERTVALUE))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Free memory in the list
	DBTypeList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE - Implicit conversion [22008]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_14()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pPrefix			= NULL;		// Prefix of DataType
	WCHAR *				pSuffix			= NULL;		// Suffix of DataType
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol;						// Class CCol
	
	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession))
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	//If we use ini file m_pCTCTbl->CreateTable doesn' t create a table but uses the table name from ini file
	//so we can not really create a table with one column
	//so we'll skip this variation otherwise it'll always fail in the case of ini file
	if(GetModInfo()->GetFileName())
	{
		odtLog << L"ini file is used!!! Can not create a table with one column in this case" << ENDL;
		return TEST_SKIPPED;
	}

	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already DateTime then were done
		if (IsColDateTime(NewCol.GetProviderType()))
			break;
	}

	// If No numeric column, skip out of the test
	if(count > pcColumns)
	{
		odtLog << L"Provider does not support a Date/Time Column." << ENDL;
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(DBTypeList,1,1,NULL,PRIMARY), S_OK))
		goto END;

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get DataType Prefix
	pPrefix = NewCol.GetPrefix();

	// Get DataType Suffix
	pSuffix = NewCol.GetSuffix();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
							(sizeof(WCHAR) * (wcslen(wszInsertInvalidDateValue) +
											  wcslen(pTableName) + 
											  (pPrefix ? wcslen(pPrefix) : 0) +
											  (pSuffix ? wcslen(pSuffix) : 0) +
											  wcslen(wszInvalidDateTime))));

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidDateValue, pTableName, 
				(pPrefix ? pPrefix : L""), wszInvalidDateTime, (pSuffix ? pSuffix : L""));

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_CANTCONVERTVALUE))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Free memory in the list
	DBTypeList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Invalid nodes in a command [37000]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_15()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * 
				  (wcslen(wszSelectBadSelect))) + sizeof(WCHAR));

	// Put the SQL statement together
	wcscpy(pwszSQLStmt, wszSelectBadSelect);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND - Prepare before setting text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_16()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Create a command object ourselves
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
										(IUnknown**)&pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
										(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Prepare(1), DB_E_NOCOMMAND))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Prepare with an open Rowset Object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_17()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IRowset*			pIRowset		= NULL;		// Array of IRowsets

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Execute Command to return a Rowset
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, (IUnknown **)&pIRowset, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Prepare(1), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pIRowset)
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Prepare an invalid escape clause
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Invalid_Cases::Variation_18()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	
	// Check the SQL Support of the Provider
	if( !(m_pCTable->GetSQLSupport() & DBPROPVAL_SQL_ESCAPECLAUSES) )
	{
		odtLog << L"Provider does not support escape clauses." << ENDL;
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = m_pCTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * 
				  (wcslen(L"Select {Convert(} from ") + wcslen(pTableName))) + sizeof(WCHAR));

	// Format SQL Statement
	swprintf(pwszSQLStmt, L"Select {Convert(} from %s", pTableName);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Invalid_Cases::Terminate()
{
	// Drop the table
	if (m_pCTable)
		m_pCTable->DropTable();
	
	return(CCommand::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ICommandPrepare_Valid_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandPrepare_Valid_Cases - Valid test variations for method ICommandPrepare::Prepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Valid_Cases::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCommand::Init())
	// }}
	{
		// Create a table
		if (CHECK(m_pCTable->CreateTable(5,1,NULL,PRIMARY),	S_OK))
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Select statement with all columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	CHECK(hr=pICommandPrep->Prepare(0), S_OK);

	if (CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		fSuccess = TRUE;

END:
	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare after IColumnsInfo::GetColumnInfo FAILS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IColumnsInfo*		pIColumnsInfo	= NULL;		// IColumnsInfo Object
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*		rgInfo			= NULL;		// Info Structure
	WCHAR*				pStringsBuffer	= NULL;		// String Buffer

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
	if (!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
											 &pStringsBuffer), DB_E_NOTPREPARED))
		goto END;

	// Compare Results from the DB_E_NOTPREPARED call
	COMPARE(cColumns, 0);
	COMPARE(rgInfo, NULL);
	COMPARE(pStringsBuffer, NULL);

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Prepare(ULONG_MAX), S_OK))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IColumnsInfo::GetColumnInfo after Prepare
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_3()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IColumnsInfo*		pIColumnsInfo	= NULL;		// IColumnsInfo Object
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*		rgInfo			= NULL;		// Info Structure
	WCHAR*				pStringsBuffer	= NULL;		// String Buffer

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (!CHECK(pICommandPrep->Prepare(1), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetInfo and expect it to return S_OK
	if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
											 &pStringsBuffer), S_OK))
	{
		// Add 1 to the to the count if Bookmarks are on
		if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, pIColumnsInfo))
			COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
		else
			COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		fSuccess = TRUE;
	}

END:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare, SetCommandText, and IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_4()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IColumnsInfo*		pIColumnsInfo	= NULL;		// IColumnsInfo Object
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*		rgInfo			= NULL;		// Info Structure
	WCHAR*				pStringsBuffer	= NULL;		// String Buffer

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Create a ICommandPrepare Object
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandPrepare,
									(IUnknown **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (!CHECK(pICommandPrep->Prepare(1), DB_E_NOCOMMAND))
		goto END;

	//  Command to return a ICommand with SetText
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
	if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
											 &pStringsBuffer), DB_E_NOTPREPARED))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare Fails, and IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_5()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IColumnsInfo*		pIColumnsInfo	= NULL;		// IColumnsInfo Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*		rgInfo			= NULL;		// Info Structure
	WCHAR*				pStringsBuffer	= NULL;		// String Buffer
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pPrefix			= NULL;		// Prefix of DataType
	WCHAR *				pSuffix			= NULL;		// Suffix of DataType
	DBORDINAL				pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol;						// Class CCol
	
	// Creates a column list from the Ctable
	pcColumns = m_pCTable->CountColumnsOnTable();

	// Loop thru column types
	for(count=1; count <= pcColumns; count++)
	{
		m_pCTable->GetColInfo(count, NewCol);
		
		// If first column is already DateTime then were done
		if (IsColDateTime(NewCol.GetProviderType()))
			break;
	}

	// If No numeric column, skip out of the test
	if(count > pcColumns)
	{
		odtLog << L"Provider does not support a Date/Time Column." << ENDL;
		m_pCTCTbl->DropTable();
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(DBTypeList,1,1,NULL,PRIMARY),	S_OK))
	{
		// Free memory in the list
		DBTypeList.RemoveAll();
		return TEST_FAIL;
	}

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Get DataType Prefix
	pPrefix = NewCol.GetPrefix();

	// Get DataType Suffix
	pSuffix = NewCol.GetSuffix();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + 
							(sizeof(WCHAR) * (wcslen(wszInsertInvalidDateValue) +
											  wcslen(pTableName) + 
											  (pPrefix ? wcslen(pPrefix) : 0) +
											  (pSuffix ? wcslen(pSuffix) : 0) +
											  wcslen(wszInvalidDateTime))));

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidDateValue, pTableName, 
				(pPrefix ? pPrefix : L""), wszInvalidDateTime, (pSuffix ? pSuffix : L""));

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset);
		
		if(SUCCEEDED(hr))
			goto END;

		// Call IColumnsInfo::GetColumnInfo
		hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer);
		
		if (FAILED(hr)) 
			CHECK(hr, E_FAIL);
		else
			CHECK(hr, S_OK);

		// Check  the return values
		COMPARE(cColumns, 0);
		COMPARE(rgInfo, NULL);
		COMPARE(pStringsBuffer, NULL);

		fSuccess = TRUE;
	}
	else
	{
		// Check HResult
		CHECK(hr, DB_E_ERRORSINCOMMAND);

		// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
		if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
												 &pStringsBuffer), DB_E_NOTPREPARED))
		{
			COMPARE(cColumns, 0);
			fSuccess = TRUE;
		}
	}

END:
	// Free memory in the list
	DBTypeList.RemoveAll();

	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare, Execute Fails, and IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_6()
{
	BOOL					fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*					pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*					pTableName		= NULL;		// Name of the table
	HRESULT					hr				= E_FAIL;	// HRESULT
	ICommand*				pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*		pICommandPrep	= NULL;		// ICommandPrepare Object
	ICommandWithParameters*	pICommandParam	= NULL;		// ICommandWithParameters Object
	IColumnsInfo*			pIColumnsInfo	= NULL;		// IColumnsInfo Object
	IUnknown *				pRowset			= NULL;		// IRowset Object
	DBORDINAL				cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*			rgInfo			= NULL;		// Info Structure
	WCHAR*					pStringsBuffer	= NULL;		// String Buffer
	CCol					NewCol;						// Class CCol
	
	// Create a table
	if (!CHECK(m_pCTCTbl->CreateTable(1,1,NULL,PRIMARY), S_OK))
		return TEST_FAIL;
	
	// Get the column name
	m_pCTCTbl->GetColInfo(1, NewCol);

	// Get the name of the table just created
	pTableName = m_pCTCTbl->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * 
				  (wcslen(wszSelectParam) + wcslen(pTableName) + wcslen(NewCol.GetColName())))
				   + sizeof(WCHAR));

	// Put the SQL statement together
	swprintf(pwszSQLStmt, wszSelectParam, pTableName, NewCol.GetColName());

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr = pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		// Execute the Command
		hr = pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset);

		// If Parameters are supported
		if(VerifyInterface(pICommandPrep, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown**)&pICommandParam))
			CHECK(hr, DB_E_PARAMNOTOPTIONAL);
		else
			CHECK(hr, DB_E_ERRORSINCOMMAND);

		// Call IColumnsInfo::GetInfo and expect it to return S_OK
		hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer);
		
		if (FAILED(hr))
			CHECK(hr, E_FAIL);
		else
			CHECK(hr, S_OK);

		fSuccess = TRUE;
	}
	else
	{
		// Check HResult
		CHECK(hr, DB_E_ERRORSINCOMMAND);

		// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
		if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
												 &pStringsBuffer), DB_E_NOTPREPARED))
		{
			COMPARE(cColumns, 0);
			fSuccess = TRUE;
		}
	}

END:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE(pICommandParam);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	// Drop the table
	m_pCTCTbl->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare and then try to set Properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_7()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (!CHECK(pICommandPrep->Prepare(1), S_OK))
		goto END;

	// Check to see if the Property is supported
	if( (SettableProperty(DBPROP_OWNINSERT, DBPROPSET_ROWSET, m_pIDBCreateSession)) ||
		(GetProperty(DBPROP_OWNINSERT, DBPROPSET_ROWSET, m_pIDBCreateSession)) )
		CHECK(hr=SetRowsetProperty(pICommand, DBPROPSET_ROWSET, DBPROP_OWNINSERT), S_OK);
	else
	{
		hr=SetRowsetProperty(pICommand, DBPROPSET_ROWSET, DBPROP_OWNINSERT);
		if(SUCCEEDED(hr))
		{
			CHECKW(hr, DB_E_ERRORSOCCURRED);
			odtLog <<wszPropertySet;
		}
		else
			CHECK(hr, DB_E_ERRORSOCCURRED);
	}

	// Execute the Command
	if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), S_OK))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);
 
	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Prepare an valid escape clause
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Valid_Cases::Variation_8()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IUnknown *			pRowset			= NULL;		// IRowset Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	
	// Check the SQL Support of the Provider
	if( !(m_pCTable->GetSQLSupport() & DBPROPVAL_SQL_ESCAPECLAUSES) )
	{
		odtLog << L"Provider does not support escape clauses." << ENDL;
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = m_pCTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * 
				  (wcslen(L"Select {d '1993-10-10'} from ") + wcslen(pTableName))) + sizeof(WCHAR));

	// Format SQL Statement
	swprintf(pwszSQLStmt, L"Select {d '1993-10-10'} from %s", pTableName);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTCTbl->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	CHECK(hr=pICommandPrep->Prepare(1), S_OK);

	// Execute the Command
	if (CHECK(hr=pICommand->Execute(NULL,IID_IRowset,0, NULL, &pRowset), S_OK))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Valid_Cases::Terminate()
{
	// Drop the table
	if (m_pCTable)
		m_pCTable->DropTable();
	
	return(CCommand::Terminate());
}
// }}
// }}

  
// {{ TCW_TC_PROTOTYPE(ICommandPrepare_Trans_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandPrepare_Trans_Cases - Transaction test variations for method ICommandPrepare::Prepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Trans_Cases::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if (CCommandZombie::Init())
	// }}
	{
		// Register the Interface
		if (!RegisterInterface(COMMAND_INTERFACE, IID_ICommandPrepare, 0, NULL))
			return FALSE;

		// Create a SQL Stmt and Set the Command
		if (CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pwszSQLStmt, NULL, NULL), S_OK))
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
// @mfunc S_OK - Abort ICommandPrepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_1()
{
	BOOL	fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM cRowsObtained= 0;		// Number of rows returned, should be 1
	DBORDINAL cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==TRUE
	if (!GetAbort(TRUE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), S_OK))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandPrepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==TRUE
	if (!GetCommit(TRUE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), S_OK))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandPrepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_3()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==FALSE
	if (!GetAbort(FALSE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), S_OK))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandPrepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_4()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==FALSE
	if (!GetCommit(FALSE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), S_OK))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_5()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==TRUE
	if (!GetAbort(TRUE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_6()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==TRUE
	if (!GetCommit(TRUE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Abort ICommandPrepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_7()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==FALSE
	if (!GetAbort(FALSE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Commit ICommandPrepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrepare_Trans_Cases::Variation_8()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==FALSE
	if (!GetCommit(FALSE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
									(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Prepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Prepare(1), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrepare_Trans_Cases::Terminate()
{
	// Release Objects
	SAFE_RELEASE(m_pIColInfo);
	SAFE_RELEASE(m_pICmdPrepare);

	// Free Memory
	PROVIDER_FREE(m_pwszSQLStmt);
	return(CCommandZombie::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ICommandUnprepare_Invalid_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandUnprepare_Invalid_Cases - Invalid test variations for method ICommandPrepare::Unprepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Invalid_Cases::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCommand::Init())
	// }}
	{
		// Create a table
		if (CHECK(m_pCTable->CreateTable(5,1,NULL,PRIMARY), S_OK))
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Unprepare with an open Rowset Object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Invalid_Cases::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IRowset*			pIRowset		= NULL;		// IRowset Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Execute Command to return a Rowset
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, (IUnknown**)&pIRowset, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(pICommandPrep->Unprepare(), DB_E_OBJECTOPEN))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Invalid_Cases::Terminate()
{
	// Drop the table
	if (m_pCTable)
		m_pCTable->DropTable();
	
	return(CCommand::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ICommandUnprepare_Valid_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandUnprepare_Valid_Cases - Valid test variations for method ICommandPrepare::Unprepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Valid_Cases::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCommand::Init())
	// }}
	{
		// Create a table
		if (CHECK(m_pCTable->CreateTable(5,1,NULL,PRIMARY), S_OK))
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Unprepare a Select statement with all columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Valid_Cases::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Unprepare the SQL Statement
	if (!CHECK(pICommandPrep->Unprepare(), S_OK))
		goto END;

	// Prepare the SQL Statement
	if (!CHECK(pICommandPrep->Prepare(1), S_OK))
		goto END;

	// Unprepare the SQL Statement
	if (CHECK(pICommandPrep->Unprepare(), S_OK))
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Unprepare and the IColumnsInfo::GetColumnInfo should FAILS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Valid_Cases::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IColumnsInfo*		pIColumnsInfo	= NULL;		// IColumnsInfo Object
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset
	DBCOLUMNINFO*		rgInfo			= NULL;		// Info Structure
	WCHAR*				pStringsBuffer	= NULL;		// String Buffer

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (!CHECK(pICommandPrep->Prepare(1), S_OK))
		goto END;

	// Unprepare the SQL Statement
	if (!CHECK(pICommandPrep->Unprepare(), S_OK))
		goto END;

	// QI for IColumnsInfo
	if (!CHECK(pICommand->QueryInterface(IID_IColumnsInfo,
									(void **)&pIColumnsInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetInfo and expect it to return DB_E_NOTPREPARED
	if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
											 &pStringsBuffer), DB_E_NOTPREPARED))
	{
		// Compare Results from the DB_E_NOTPREPARED
		COMPARE(cColumns, 0);
		COMPARE(rgInfo, NULL);
		COMPARE(pStringsBuffer, NULL);

		fSuccess = TRUE;
	}


END:
	// Release Objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Valid_Cases::Terminate()
{
	// Drop the table
	if (m_pCTable)
		m_pCTable->DropTable();
	
	return(CCommand::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ICommandUnprepare_Trans_Cases)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandUnprepare_Trans_Cases - Transaction test variations for method ICommandPrepare::Unprepare
//|	Created:		12/10/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Trans_Cases::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if (CCommandZombie::Init())
	// }}
	{
		// Register the Interface
		if (!RegisterInterface(COMMAND_INTERFACE, IID_ICommandPrepare, 0, NULL))
			return FALSE;

		// Create a SQL Stmt and Set the Command
		if (CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
											NULL, &m_pwszSQLStmt, NULL, NULL), S_OK))
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
// @mfunc S_OK - Abort ICommandUnprepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==TRUE
	if (!GetAbort(TRUE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup the Rowset objects in the transaction
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandUnprepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==TRUE
	if (!GetCommit(TRUE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup the Rowset objects in the transaction
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort ICommandUnprepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_3()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==FALSE
	if (!GetAbort(FALSE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup the Rowset objects in the transaction
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit ICommandUnprepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_4()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==FALSE
	if (!GetCommit(FALSE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Cleanup the Rowset objects in the transaction
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_5()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==TRUE
	if (!GetAbort(TRUE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	CHECK(m_pICmdPrepare->Unprepare(), DB_E_OBJECTOPEN);

	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_6()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==TRUE
	if (!GetCommit(TRUE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	CHECK(m_pICmdPrepare->Unprepare(), DB_E_OBJECTOPEN);

	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, TRUE);

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, TRUE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Abort ICommandUnprepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_7()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Abort the transaction with fRetaining==FALSE
	if (!GetAbort(FALSE))
		goto END;

	// Test zombie
	if (!m_fAbortPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
											(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareAbortPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	CHECK(m_pICmdPrepare->Unprepare(), DB_E_OBJECTOPEN);

	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Commit ICommandUnprepare with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandUnprepare_Trans_Cases::Variation_8()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	DBCOUNTITEM 			cRowsObtained	= 0;		// Number of rows returned, should be 1
	DBORDINAL			cColumns		= 0;		// Number of Columns in Rowset

	// Retrieve an Interface pointer to ICommandPrepare within a Transaction
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&m_pICmdPrepare, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto END;

	// Commit the transaction with fRetaining==FALSE
	if (!GetCommit(FALSE))
		goto END;

	// Test zombie
	if (!m_fCommitPreserve)
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), E_UNEXPECTED);
	else
		CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&m_rghRows), S_OK);
		
	// Release the row handle on the 1st rowset
	if (m_rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(m_rghRows);
	}

	// QI for IColumnsInfo
	if (!CHECK(m_pICmdPrepare->QueryInterface(IID_IColumnsInfo,
									(void **)&m_pIColInfo), S_OK))
		goto END;

	// Call IColumnsInfo::GetColumnInfo
	if (!m_fPrepareCommitPreserve)
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), DB_E_NOTPREPARED))
			COMPARE(cColumns, 0);
	}
	else
	{
		if (CHECK(m_pIColInfo->GetColumnInfo(&cColumns, &m_rgInfo, 
												 &m_pStringsBuffer), S_OK))
		{
			// Add 1 to the to the count if Bookmarks are on
			if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColInfo))
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable()+1);
			else
				COMPARE(cColumns, m_pCTable->CountColumnsOnTable());
		}
	}

	// Unprepare the SQL Statement
	CHECK(m_pICmdPrepare->Unprepare(), DB_E_OBJECTOPEN);

	// Cleanup Transactions
	CleanupTransactionRowset(ROWSET_INTERFACE, FALSE);

	// Unprepare the SQL Statement
	if (CHECK(m_pICmdPrepare->Unprepare(), S_OK))
		fSuccess = TEST_PASS;

END:
	// Cleanup Transactions
	CleanupTransactionRowset(COMMAND_INTERFACE, FALSE);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandUnprepare_Trans_Cases::Terminate()
{
	// Release Objects
	SAFE_RELEASE(m_pIColInfo);
	SAFE_RELEASE_(m_pICmdPrepare);
	
	// Free Memory
	PROVIDER_FREE(m_pwszSQLStmt);
	return(CCommandZombie::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ICommandPrep_ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ICommandPrep_ExtendedErrors - Extended Errors
//|	Created:		07/08/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrep_ExtendedErrors::Init()
{
// {{ TCW_INIT_BASECLASS_CHECK
	if(CCommand::Init())
	// }}
	{
		// Create a table
		if( CHECK(m_pCTable->CreateTable(5,			// Number of rows to insert
										1,			// Column to put index on
										NULL,		// Table name
										PRIMARY),	// Primary or secondary values
										S_OK) )
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid ICommandPrepare calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrep_ExtendedErrors::Variation_1()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ICommandPrepare method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	//cause an error object
	m_pExtError->CauseError();

	// Prepare the SQL Statement
	if (CHECK(hr = pICommandPrep->Prepare(1), S_OK))
	{
		//Do extended check following Prepare
		fSuccess = XCHECK(pICommandPrep, IID_ICommandPrepare, hr);

		if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;
	}
	else
		goto END;
	
	//cause an error object   	
	m_pExtError->CauseError();

	if (CHECK(hr = pICommandPrep->Unprepare(), S_OK))
		//Do extended check following Unprepare
		fSuccess &= XCHECK(pICommandPrep, IID_ICommandPrepare, hr);
	else
		fSuccess &=FALSE;

END:
	// Release Objects
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE_(pICommandPrep);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid ICommandPrepare calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrep_ExtendedErrors::Variation_2()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IRowset*			pIRowset		= NULL;		// Array of IRowsets

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommandPrepare method.
	//We then check extended errors to verify the right extended error behavior.
	
	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Execute Command to return a Rowset
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, (IUnknown **)&pIRowset, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;

	//cause an error object
	m_pExtError->CauseError();

	// Prepare the SQL Statement
	if (CHECK(hr=pICommandPrep->Prepare(1), DB_E_OBJECTOPEN))
		//Do extended check following Prepare
		fSuccess = XCHECK(pICommandPrep, IID_ICommandPrepare, hr);
	else
		goto END;

	m_pExtError->CauseError();

	if (CHECK(hr=pICommandPrep->Unprepare(), DB_E_OBJECTOPEN))
		//Do extended check following Unprepare
		fSuccess &= XCHECK(pICommandPrep, IID_ICommandPrepare, hr);

	else
		fSuccess &= FALSE;

END:
	// Release Objects
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE_(pICommandPrep);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid Prepare call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrep_ExtendedErrors::Variation_3()
{	
	HRESULT				hr				= E_FAIL;
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object

	//For the method of the interface, with no error object on
	//the current thread, try get a failure from the ICommandPrepare method.
	//We then check extended errors to verify the right extended error behavior.
  

	// Create a command object ourselves
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
										(IUnknown**)&pICommand), S_OK))
		return TEST_FAIL;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
										(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(hr=pICommandPrep->Prepare(1), DB_E_NOCOMMAND))
		//Do extended check following Prepare
		fSuccess = XCHECK(pICommandPrep, IID_ICommandPrepare, hr);

END:
	// Release Objects
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE_(pICommandPrep);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid Unprepare call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrep_ExtendedErrors::Variation_4()
{	
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	IRowset*			pIRowset		= NULL;		// IRowset Object

	//For the method of the interface, with no error object on
	//the current thread, try get a failure from the ICommandPrepare method.
	//We then check extended errors to verify the right extended error behavior.

	// Create a SQL Stmt and Set the Command
	if (!CHECK(m_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, 
										NULL, &pwszSQLStmt, NULL, NULL), S_OK))
		goto END;

	// Execute Command to return a Rowset
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, (IUnknown**)&pIRowset, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	if (CHECK(hr=pICommandPrep->Unprepare(), DB_E_OBJECTOPEN))
		//Do extended check following Prepare
		fSuccess = XCHECK(pICommandPrep, IID_ICommandPrepare, hr);

END:
	// Release Objects
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE_(pICommandPrep);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORINCOMMAND call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ICommandPrep_ExtendedErrors::Variation_5()
{	
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	
	//For the method of the interface returning DB_E_NOCOMMAND, with no error object
	//on the current thread, try get a failure from the ICommandPrepare method.
	//We then check extended errors to verify the right extended error behavior.

	// Get the name of the table just created
	pTableName = m_pCTable->GetTableName();

	pwszSQLStmt	= (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszCreateTable) + wcslen(pTableName))) );

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszCreateTable, pTableName);

	//  Command to return a ICommand with Text Set
	if (!CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK))
		goto END;

	// QI for ICommandPrepare
	if (!CHECK(pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrep), S_OK))
		goto END;
	
	// Prepare the SQL Statement
	hr= pICommandPrep->Prepare(1);

	// Compare the HRESULT
	if (hr == S_OK)
	{
		if(CHECK(m_pCTable->BuildCommand(pwszSQLStmt, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND))
			fSuccess = TRUE;
	}
	else
	{
		// Do extended check following Prepare
		if (CHECK(hr, DB_E_ERRORSINCOMMAND))
			fSuccess = XCHECK(pICommandPrep, IID_ICommandPrepare, hr);
	}

END:
	// Release Objects
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE_(pICommandPrep);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ICommandPrep_ExtendedErrors::Terminate()
{
	// Drop the table
	if (m_pCTable)
		m_pCTable->DropTable();

	return CCommand::Terminate();
}	// }}
// }}
// }}


//---------------------------------------------------------------------------
//	CCommand::IsColNumWithScale
//
//	@mfunc	BOOL				|
//			CCommand			|
//			IsColNumWithScale	|
//			Can the data type hold Numeric values with Scale?
//
//
//---------------------------------------------------------------------------
BOOL CCommand::IsColNumWithScale
(
	DBTYPE wType,			// @parm [IN] provider data type
	ULONG  Scale			// @parm [IN] precision for the data type
)
{
	switch(wType)
	{
			case DBTYPE_NUMERIC:	// Numeric, Decimal
			case DBTYPE_DECIMAL:	// Numeric, Decimal
				if (Scale)
					return TRUE;
				else
					return FALSE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCommand::IsColCharacter 
//
//	@mfunc	BOOL			|
//			CCommand		|
//			IsColCharacter	|
//			Can the data type hold string values?
//
//
//---------------------------------------------------------------------------
BOOL CCommand::IsColCharacter
(
	DBTYPE wType,			// @parm [IN] provider data type
	DBLENGTH ulColumnSize	// @parm [IN] precision for the data type
)
{
	switch(wType)
	{
			case DBTYPE_STR:	// Character
			case DBTYPE_WSTR:	
			case DBTYPE_BSTR:	
				if (ulColumnSize < 8000)
					return TRUE;
				else
					return FALSE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCommand::IsColDateTime 
//
//	@mfunc	BOOL			|
//			CCommand		|
//			IsColDateTime	|
//			Can the data type hold DateTime values?
//
//
//---------------------------------------------------------------------------
BOOL CCommand::IsColDateTime			
(
	DBTYPE wType		// @parm [IN] provider data type
)
{
	switch(wType)
	{
			case DBTYPE_DATE:			// OLE Auto. Date
			case DBTYPE_DBDATE:			// Date
			case DBTYPE_DBTIME:			// Time
			case DBTYPE_DBTIMESTAMP:	// TimeStamp
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCommandZombie::CleanupTransactionRowset
//
//	@mfunc	void			|
//			CCommandZombie		|
//			CleanupTransactionRowset	|
//			Cleanup the Rowset objects in the transaction
//
//
//---------------------------------------------------------------------------
void CCommandZombie::CleanupTransactionRowset(EINTERFACE eInterface, BOOL fRetaining)
{

	// Switch on the enum passed in by user
	switch (eInterface)
	{
		case ROWSET_INTERFACE:
			// Release the row handle on the 1st rowset
			if (m_rghRows) {
				CHECK(m_pIRowset->ReleaseRows(1, m_rghRows, NULL, NULL, NULL),S_OK);
				PROVIDER_FREE(m_rghRows);
			}

			// Release the Rowset Objects
			SAFE_RELEASE(m_pIRowset);
			SAFE_RELEASE(m_pIRowsetInfo);
			SAFE_RELEASE(m_pIAccessor);
			SAFE_RELEASE(m_pIColumnsInfo);

		break;
		
		case COMMAND_INTERFACE:
			// Cleanup Transactions
			if (fRetaining)
				CleanUpTransaction(S_OK);
			else
				CleanUpTransaction(XACT_E_NOTRANSACTION);

			// Release objects created in StartTransaction
			SAFE_RELEASE(m_pICmdPrepare);
			SAFE_RELEASE(m_pIColInfo);

			// Free Memory
			PROVIDER_FREE(m_rgInfo);
			PROVIDER_FREE(m_pStringsBuffer);

		break;
	}
}

