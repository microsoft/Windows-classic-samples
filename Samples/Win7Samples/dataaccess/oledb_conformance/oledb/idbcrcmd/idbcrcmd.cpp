//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IDBCRCMD.CPP | This is the IDBCreateCommand.
//

#include "modstandard.hpp"	// Standard headers, precompiled in modcore.cpp			
#include "idbcrcmd.h"		// Testcase's header 
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x9f079350, 0xd81c, 0x11ce, { 0x98, 0x81, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("IDBCreateCommand");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IDBCreateCommand Interface.");
DECLARE_MODULE_VERSION(832461938);
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
	IDBCreateSession* pIDBCreateSession	= NULL;
	IDBCreateCommand* pIDBCreateCommand = NULL;
	BOOL bValue = FALSE;

	TESTC(CommonModuleInit(pThisTestModule));

	// IDBCreateSession
	TESTC(VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBCreateSession,
						DATASOURCE_INTERFACE, (IUnknown**)&pIDBCreateSession));
		
	// IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand,
							SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
	{
		odtLog << L"Commands are not supported by this Provider." << ENDL;
		bValue = TEST_SKIPPED;
		goto CLEANUP;
	}

	bValue = TRUE;

CLEANUP:
	
	// Release the Interfaces
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIDBCreateCommand);
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
	return CommonModuleTerminate(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCIDBCRCMD Base Class for IDBCreateCommand:CreateCommand Testcases
class TCIDBCRCMD : public CRowset
{
	public:
		// @cmember Constructor
		TCIDBCRCMD(LPWSTR wstrTestCaseName) : CRowset(wstrTestCaseName)
		{
			m_pIDBCreateSession	= NULL;
			m_pIDBCreateCommand = NULL;
		};

		// @cmember Destructor
		virtual ~TCIDBCRCMD(){};

		BOOL	InitializeRowObject();

	protected:
		// @cmember IDBCreateSession Interface
		IDBCreateSession*	m_pIDBCreateSession;
		// @cmember IDBCreateCommand Interface
		IDBCreateCommand*	m_pIDBCreateCommand;
		// @cmember CRowObject pointer
		CRowObject*			m_pCRowObject;		
		// @cmember Row handle
		HROW				m_hRow;
};


// @class TCZOMBIE Base Class for IDBCreateCommand:ZombieCommand Testcases
class TCZOMBIE : public CTransaction
{
	public:
		// @cmember Constructor
		TCZOMBIE(LPWSTR wstrTestCaseName) : CTransaction(wstrTestCaseName) 
		{
			m_fRowCommand = FALSE;
		};

		// @cmember Destructor
		virtual ~TCZOMBIE(){};

	protected:
		// @cmember flag to indicate source of IDBCreateCommand
		BOOL	m_fRowCommand;
};


// @class TCMULTICMD Base Class for IDBCreateCommand:MultipleObject Testcases
class TCMULTICMD : public TCIDBCRCMD
{
	public:
		// @cmember Constructor
		TCMULTICMD(LPWSTR wstrTestCaseName) : TCIDBCRCMD(wstrTestCaseName)
		{
			m_pIDBCreateSession	= NULL;
			m_pIDBCreateCommand = NULL;
		};

		// @cmember Destructor
		virtual ~TCMULTICMD(){};

		// @cmember Tests QI on IUnknown from the ICommand Pointer
		int TestInterface(IID iid, BOOL mandatory);
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIDBCRCMD_CreateCommand)
//--------------------------------------------------------------------
// @class IDBCreateCommand::CreateCommand
//
class TCIDBCRCMD_CreateCommand : public TCIDBCRCMD { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	~TCIDBCRCMD_CreateCommand (void) {};								
    TCIDBCRCMD_CreateCommand ( wchar_t* pwszTestCaseName) : TCIDBCRCMD(pwszTestCaseName) { };	
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_NOINTERFACE - Invalid REFIID
	int Variation_1();
	// @cmember E_NOINTERFACE - IID_IDBInitialize REFIID
	int Variation_2();
	// @cmember E_NOINTERFACE - IID_IDBCreateCommand REFIID
	int Variation_3();
	// @cmember E_INVALIDARG - NULL pCommand
	int Variation_4();
	// @cmember S_OK - IID_ICommand for REFIID
	int Variation_5();
	// @cmember S_OK - IID_ICommandText for REFIID
	int Variation_6();
	// @cmember S_OK - IID_IColumnsInfo for REFIID
	int Variation_7();
	// }} TCW_TESTVARS_END
};


// {{ TCW_TEST_CASE_MAP(TCIDBCRCMD_CreateCommand_FromSession)
//--------------------------------------------------------------------
// @class IDBCreateCommand::CreateCommand
//
class TCIDBCRCMD_CreateCommand_FromSession : public TCIDBCRCMD_CreateCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRCMD_CreateCommand_FromSession,TCIDBCRCMD_CreateCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCIDBCRCMD_CreateCommand_FromSession)
#define THE_CLASS TCIDBCRCMD_CreateCommand_FromSession
BEG_TEST_CASE(TCIDBCRCMD_CreateCommand_FromSession, TCIDBCRCMD_CreateCommand, L"IDBCreateCommand::CreateCommand")
	TEST_VARIATION(1, 		L"E_NOINTERFACE - Invalid REFIID")
	TEST_VARIATION(2, 		L"E_NOINTERFACE - IID_IDBInitialize REFIID")
	TEST_VARIATION(3, 		L"E_NOINTERFACE - IID_IDBCreateCommand REFIID")
	TEST_VARIATION(4, 		L"E_INVALIDARG - NULL pCommand")
	TEST_VARIATION(5, 		L"S_OK - IID_ICommand for REFIID")
	TEST_VARIATION(6, 		L"S_OK - IID_ICommandText for REFIID")
	TEST_VARIATION(7, 		L"S_OK - IID_IColumnsInfo for REFIID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBCRCMD_CreateCommand_FromRow)
//--------------------------------------------------------------------
// @class IDBCreateCommand::CreateCommand
//
class TCIDBCRCMD_CreateCommand_FromRow : public TCIDBCRCMD_CreateCommand { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRCMD_CreateCommand_FromRow,TCIDBCRCMD_CreateCommand);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCIDBCRCMD_CreateCommand_FromRow)
#define THE_CLASS TCIDBCRCMD_CreateCommand_FromRow
BEG_TEST_CASE(TCIDBCRCMD_CreateCommand_FromRow, TCIDBCRCMD, L"IDBCreateCommand::CreateCommand")
	TEST_VARIATION(1, 		L"E_NOINTERFACE - Invalid REFIID")
	TEST_VARIATION(2, 		L"E_NOINTERFACE - IID_IDBInitialize REFIID")
	TEST_VARIATION(3, 		L"E_NOINTERFACE - IID_IDBCreateCommand REFIID")
	TEST_VARIATION(4, 		L"E_INVALIDARG - NULL pCommand")
	TEST_VARIATION(5, 		L"S_OK - IID_ICommand for REFIID")
	TEST_VARIATION(6, 		L"S_OK - IID_ICommandText for REFIID")
	TEST_VARIATION(7, 		L"S_OK - IID_IColumnsInfo for REFIID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @class IDBCreateCommand::MultipleObjects
//
class TCMULTICMD_QueryInterface : public TCMULTICMD { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	~TCMULTICMD_QueryInterface(void) {} ;								
    TCMULTICMD_QueryInterface( wchar_t* pwszTestCaseName) : TCMULTICMD(pwszTestCaseName) { };	

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember S_OK - ICommand
	int Variation_1();
	// @cmember S_OK - ICommandText
	int Variation_2();
	// @cmember S_OK - ICommandWithParameters
	int Variation_3();
	// @cmember S_OK - ICommandPrepare
	int Variation_4();
	// @cmember S_OK - ICommandProperties
	int Variation_5();
	// @cmember S_OK - IColumnsInfo
	int Variation_6();
	// @cmember S_OK - IColumnsRowset
	int Variation_7();
	// @cmember S_OK - IAccessor
	int Variation_8();
	// @cmember S_OK - ISupportErrorInfo
	int Variation_9();
	// @cmember S_OK - IConvertType
	int Variation_10();
	// @cmember S_OK - IUnknown
	int Variation_11();
};


// {{ TCW_TEST_CASE_MAP(TCMULTICMD_QueryInterface_FromSession)
//--------------------------------------------------------------------
// @class IDBCreateCommand::MultipleObjects
//
class TCMULTICMD_QueryInterface_FromSession : public TCMULTICMD_QueryInterface { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMULTICMD_QueryInterface_FromSession,TCMULTICMD_QueryInterface);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCMULTICMD_QueryInterface_FromSession)
#define THE_CLASS TCMULTICMD_QueryInterface_FromSession
BEG_TEST_CASE(TCMULTICMD_QueryInterface_FromSession, TCMULTICMD_QueryInterface, L"IDBCreateCommand::MultipleObjects")
	TEST_VARIATION(1, 		L"S_OK - ICommand")
	TEST_VARIATION(2, 		L"S_OK - ICommandText")
	TEST_VARIATION(3, 		L"S_OK - ICommandWithParameters")
	TEST_VARIATION(4, 		L"S_OK - ICommandPrepare")
	TEST_VARIATION(5, 		L"S_OK - ICommandProperties")
	TEST_VARIATION(6, 		L"S_OK - IColumnsInfo")
	TEST_VARIATION(7, 		L"S_OK - IColumnsRowset")
	TEST_VARIATION(8, 		L"S_OK - IAccessor")
	TEST_VARIATION(9, 		L"S_OK - ISupportErrorInfo")
	TEST_VARIATION(10, 		L"S_OK - IConvertType")
	TEST_VARIATION(11, 		L"S_OK - IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMULTICMD_QueryInterface_FromRow)
//--------------------------------------------------------------------
// @class IDBCreateCommand::MultipleObjects
//
class TCMULTICMD_QueryInterface_FromRow : public TCMULTICMD_QueryInterface { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMULTICMD_QueryInterface_FromRow,TCMULTICMD_QueryInterface);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCMULTICMD_QueryInterface_FromRow)
#define THE_CLASS TCMULTICMD_QueryInterface_FromRow
BEG_TEST_CASE(TCMULTICMD_QueryInterface_FromRow, TCMULTICMD_QueryInterface, L"IDBCreateCommand::MultipleObjects")
	TEST_VARIATION(1, 		L"S_OK - ICommand")
	TEST_VARIATION(2, 		L"S_OK - ICommandText")
	TEST_VARIATION(3, 		L"S_OK - ICommandWithParameters")
	TEST_VARIATION(4, 		L"S_OK - ICommandPrepare")
	TEST_VARIATION(5, 		L"S_OK - ICommandProperties")
	TEST_VARIATION(6, 		L"S_OK - IColumnsInfo")
	TEST_VARIATION(7, 		L"S_OK - IColumnsRowset")
	TEST_VARIATION(8, 		L"S_OK - IAccessor")
	TEST_VARIATION(9, 		L"S_OK - ISupportErrorInfo")
	TEST_VARIATION(10, 		L"S_OK - IConvertType")
	TEST_VARIATION(11, 		L"S_OK - IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @class IDBCreateCommand::ZombieCommand
//
class TCZOMBIE_Zombie : public TCZOMBIE { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	~TCZOMBIE_Zombie(void) {} ;								
    TCZOMBIE_Zombie( wchar_t* pwszTestCaseName) : TCZOMBIE(pwszTestCaseName) { };	

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember TestTxn
	int TestTxn(ETXN eTxn, BOOL fRetaining);

	// @cmember S_OK - Abort IDBCreateCommand with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit IDBCreateCommand with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Abort IDBCreateCommand with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit IDBCreateCommand with fRetaining=FALSE
	int Variation_4();

};


// {{ TCW_TEST_CASE_MAP(TCZOMBIE_Zombie_FromSession)
//--------------------------------------------------------------------
// @class IDBCreateCommand::ZombieCommand
//
class TCZOMBIE_Zombie_FromSession : public TCZOMBIE_Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZOMBIE_Zombie_FromSession,TCZOMBIE_Zombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCZOMBIE_Zombie_FromSession)
#define THE_CLASS TCZOMBIE_Zombie_FromSession
BEG_TEST_CASE(TCZOMBIE_Zombie_FromSession, TCZOMBIE_Zombie, L"IDBCreateCommand::ZombieCommand")
	TEST_VARIATION(1, 		L"S_OK - Abort IDBCreateCommand with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit IDBCreateCommand with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort IDBCreateCommand with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit IDBCreateCommand with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCZOMBIE_Zombie_FromRow)
//--------------------------------------------------------------------
// @class IDBCreateCommand::ZombieCommand
//
class TCZOMBIE_Zombie_FromRow : public TCZOMBIE_Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZOMBIE_Zombie_FromRow,TCZOMBIE_Zombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCZOMBIE_Zombie_FromRow)
#define THE_CLASS TCZOMBIE_Zombie_FromRow
BEG_TEST_CASE(TCZOMBIE_Zombie_FromRow, TCZOMBIE_Zombie, L"IDBCreateCommand::ZombieCommand")
	TEST_VARIATION(1, 		L"S_OK - Abort IDBCreateCommand with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit IDBCreateCommand with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort IDBCreateCommand with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit IDBCreateCommand with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @class IDBCreateCommand::CommandLimit
//
class TCIDBCRCMD_CommandLimit : public TCIDBCRCMD { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	~TCIDBCRCMD_CommandLimit(void) {};								
    TCIDBCRCMD_CommandLimit( wchar_t* pwszTestCaseName) : TCIDBCRCMD(pwszTestCaseName) { };	

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember S_OK - 100 Command Objects
	int Variation_1();
	// @cmember S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE
	int Variation_2();
	// @cmember DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE
	int Variation_3();
};


// {{ TCW_TEST_CASE_MAP(TCIDBCRCMD_CommandLimit_FromSession)
//--------------------------------------------------------------------
// @class IDBCreateCommand::CommandLimit
//
class TCIDBCRCMD_CommandLimit_FromSession : public TCIDBCRCMD_CommandLimit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRCMD_CommandLimit_FromSession,TCIDBCRCMD_CommandLimit);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCIDBCRCMD_CommandLimit_FromSession)
#define THE_CLASS TCIDBCRCMD_CommandLimit_FromSession
BEG_TEST_CASE(TCIDBCRCMD_CommandLimit_FromSession, TCIDBCRCMD_CommandLimit, L"IDBCreateCommand::CommandLimit")
	TEST_VARIATION(1, 		L"S_OK - 100 Command Objects")
	TEST_VARIATION(2, 		L"S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE")
	TEST_VARIATION(3, 		L"DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBCRCMD_CommandLimit_FromRow)
//--------------------------------------------------------------------
// @class IDBCreateCommand::CommandLimit
//
class TCIDBCRCMD_CommandLimit_FromRow : public TCIDBCRCMD_CommandLimit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRCMD_CommandLimit_FromRow,TCIDBCRCMD_CommandLimit);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(TCIDBCRCMD_CommandLimit_FromRow)
#define THE_CLASS TCIDBCRCMD_CommandLimit_FromRow
BEG_TEST_CASE(TCIDBCRCMD_CommandLimit_FromRow, TCIDBCRCMD_CommandLimit, L"IDBCreateCommand::CommandLimit")
	TEST_VARIATION(1, 		L"S_OK - 100 Command Objects")
	TEST_VARIATION(2, 		L"S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE")
	TEST_VARIATION(3, 		L"DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public TCIDBCRCMD { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	~TCExtendedErrors(void) {};								
    TCExtendedErrors( wchar_t* pwszTestCaseName) : TCIDBCRCMD(pwszTestCaseName) { };	
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember Valid IDBCreateCommand calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IDBCreateCommand calls with previous error object existing.
	int Variation_2();
	// @cmember Invalid IDBCreateCommand calls with no previous error object existing.
	int Variation_3();
};


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_FromSession)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors_FromSession : public TCExtendedErrors { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_FromSession,TCExtendedErrors);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};


// {{ TCW_TESTCASE(TCExtendedErrors_FromSession)
#define THE_CLASS TCExtendedErrors_FromSession
BEG_TEST_CASE(TCExtendedErrors_FromSession, TCExtendedErrors, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IDBCreateCommand calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IDBCreateCommand calls with previous error object existing.")
	TEST_VARIATION(3, 		L"Invalid IDBCreateCommand calls with no previous error object existing.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_FromRow)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors_FromRow : public TCExtendedErrors { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_FromRow,TCExtendedErrors);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};


// {{ TCW_TESTCASE(TCExtendedErrors_FromRow)
#define THE_CLASS TCExtendedErrors_FromRow
BEG_TEST_CASE(TCExtendedErrors_FromRow, TCExtendedErrors, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IDBCreateCommand calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IDBCreateCommand calls with previous error object existing.")
	TEST_VARIATION(3, 		L"Invalid IDBCreateCommand calls with no previous error object existing.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(10, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIDBCRCMD_CreateCommand_FromSession)
	TEST_CASE(2, TCIDBCRCMD_CreateCommand_FromRow)
	TEST_CASE(3, TCMULTICMD_QueryInterface_FromSession)
	TEST_CASE(4, TCMULTICMD_QueryInterface_FromRow)
	TEST_CASE(5, TCZOMBIE_Zombie_FromSession)
	TEST_CASE(6, TCZOMBIE_Zombie_FromRow)
	TEST_CASE(7, TCIDBCRCMD_CommandLimit_FromSession)
	TEST_CASE(8, TCIDBCRCMD_CommandLimit_FromRow)
	TEST_CASE(9, TCExtendedErrors_FromSession)
	TEST_CASE(10, TCExtendedErrors_FromRow)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIDBCRCMD_CreateCommand)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBCRCMD_CreateCommand - IDBCreateCommand::CreateCommand
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CreateCommand::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD::Init())
	// }}
	{
		return TRUE;
	}

	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_1()
{
	TBEGIN;

	ICommand* pCommand = INVALID(ICommand*);

	// IRowset isn't a valid REFIID on CreateCommand
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_IRowset, 
								(IUnknown**)&pCommand), E_NOINTERFACE);
	TESTC(!pCommand);

CLEANUP:

	// Release Object
    if( pCommand != INVALID(ICommand*) )
		SAFE_RELEASE(pCommand);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IDBInitialize REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_2()
{
	TBEGIN;

	ICommand* pCommand = INVALID(ICommand*);

	// IDBInitialize isn't a valid REFIID on CreateCommand
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_IDBInitialize, 
								(IUnknown**)&pCommand), E_NOINTERFACE);
	TESTC(!pCommand);

CLEANUP:

	// Release Object
    if( pCommand != INVALID(ICommand*) )
		SAFE_RELEASE(pCommand);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_IDBCreateCommand REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_3()
{
	TBEGIN;

	ICommand* pCommand = INVALID(ICommand*);

	// IDBCreateCommand isn't a valid REFIID on CreateCommand
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_IDBCreateCommand, 
								(IUnknown**)&pCommand), E_NOINTERFACE);
	TESTC(!pCommand);

CLEANUP:

	// Release Object
    if( pCommand != INVALID(ICommand*) )
		SAFE_RELEASE(pCommand);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL pCommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_4()
{
	TBEGIN;

	// NULL pICommand on CreateCommand 
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, 
								IID_ICommand, NULL), E_INVALIDARG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_ICommand for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_5()
{
	TBEGIN;

	ICommand* pCommand = NULL;

	// CreateCommand with IID_ICommand
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
											(IUnknown**)&pCommand), S_OK);
	TESTC(pCommand != NULL);

CLEANUP:

	// Release Object
	SAFE_RELEASE_(pCommand);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_ICommandText for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_6()
{
	TBEGIN;

	ICommand* pCommand = NULL;

	// CreateCommand with IID_ICommandText
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, 
											(IUnknown**)&pCommand), S_OK);
	TESTC(pCommand != NULL);

CLEANUP:

	// Release Object
	SAFE_RELEASE_(pCommand);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_IColumnsInfo for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CreateCommand::Variation_7()
{
	TBEGIN;

	ICommand* pCommand = NULL;

	// CreateCommand with IID_IColumnsInfo
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_IColumnsInfo, 
												(IUnknown**)&pCommand), S_OK);
	TESTC(pCommand != NULL);

CLEANUP:

	// Release Object
	SAFE_RELEASE_(pCommand);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CreateCommand::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CreateCommand_FromSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD_CreateCommand::Init())
	// }}
	{
		// QueryInterface for a IDBCreateSession
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
		// QueryInterface for a IDBCreateCommand
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
							SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand));
		
		return TRUE;
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
BOOL TCIDBCRCMD_CreateCommand_FromSession::Terminate()
{
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pIDBCreateCommand);
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CreateCommand_FromRow::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD_CreateCommand::Init())
	// }}
	{
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
BOOL TCIDBCRCMD_CreateCommand_FromRow::Terminate()
{
	// Destroy the row object
	SAFE_DELETE(m_pCRowObject);

	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pIDBCreateSession);
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCMULTICMD_QueryInterface)
//*-----------------------------------------------------------------------
//| Test Case:		TCMULTICMD_QueryInterface - IDBCreateCommand::MultipleObjects
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTICMD_QueryInterface::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMULTICMD::Init())
	// }}
	{
		return TRUE;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ICommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_1()
{
	// ICommand REFIID is mandatory 
	return TestInterface(IID_ICommand,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ICommandText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_2()
{
	// ICommandText REFIID is mandatory 
	return TestInterface(IID_ICommandText,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ICommandWithParameters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_3()
{
	// ICommandWithParameters REFIID is not mandatory 
	return TestInterface(IID_ICommandWithParameters,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ICommandPrepare
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_4()
{
	// ICommandPrepare REFIID is not mandatory 
	return TestInterface(IID_ICommandPrepare,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ICommandProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_5()
{
	// ICommandProperties REFIID is mandatory 
	return TestInterface(IID_ICommandProperties,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_6()
{
	// IColumnsInfo REFIID is mandatory 
	return TestInterface(IID_IColumnsInfo,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_7()
{
	// IColumnsRowset REFIID is not mandatory 
	return TestInterface(IID_IColumnsRowset,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_8()
{
	// IAccessor REFIID is mandatory 
	return TestInterface(IID_IAccessor,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ISupportErrorInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_9()
{
	// ISupportErrorInfo REFIID is not mandatory 
	return TestInterface(IID_ISupportErrorInfo,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_10()
{
	// IConvertType REFIID is mandatory 
	return TestInterface(IID_IConvertType,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTICMD_QueryInterface::Variation_11()
{
	// IUnknown REFIID is mandatory 
	return TestInterface(IID_IUnknown,TRUE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTICMD_QueryInterface::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCMULTICMD::Terminate());
}	// }}
// }}
// }}

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTICMD_QueryInterface_FromSession::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMULTICMD_QueryInterface::Init())
	// }}
	{
		// QueryInterface for a IDBCreateSession
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
		// QueryInterface for a IDBCreateCommand
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
							SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand));
			
		return TRUE;
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
BOOL TCMULTICMD_QueryInterface_FromSession::Terminate()
{
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pIDBCreateCommand);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCMULTICMD_QueryInterface::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTICMD_QueryInterface_FromRow::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMULTICMD_QueryInterface::Init())
	// }}
	{
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
BOOL TCMULTICMD_QueryInterface_FromRow::Terminate()
{
	// Destroy the row object
	SAFE_DELETE(m_pCRowObject);

	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCMULTICMD_QueryInterface::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZOMBIE_Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZOMBIE_Zombie - IDBCreateCommand::ZombieCommand
//|	Created:		11/08/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCZOMBIE::Init())
	// }}
	{
		return TRUE;
	}
	
	// Check to see if ITransaction is supported
    if( !m_pITransactionLocal )
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBCreateCommand with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_1()
{
	// S_OK - Abort IDBCreateCommand with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBCreateCommand with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_2()
{
	// S_OK - Commit IDBCreateCommand with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBCreateCommand with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_3()
{
	// S_OK - Abort IDBCreateCommand with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBCreateCommand with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_4()
{
	// S_OK - Commit IDBCreateCommand with fRetaining=FALSE
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCZOMBIE::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie_FromSession::Init()
{	
	BOOL fPass = TCZOMBIE_Zombie::Init();
	// {{ TCW_INIT_BASECLASS_CHECK
	if(fPass == TRUE)
	// }}
	{
		m_fRowCommand = FALSE;
		return RegisterInterface(SESSION_INTERFACE,IID_IDBCreateCommand,0,NULL);
	}

	return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie_FromSession::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCZOMBIE_Zombie::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie_FromRow::Init()
{	
	BOOL fPass = TCZOMBIE_Zombie::Init();
	// {{ TCW_INIT_BASECLASS_CHECK
	if(fPass == TRUE)
	// }}
	{
		m_fRowCommand = TRUE;
		// Even if IDBCreateCommand is support on the session,
		// it may not be supported on a row object
		if(!RegisterInterface(ROW_INTERFACE,IID_IDBCreateCommand,0,NULL))
			return TEST_SKIPPED;
		else
			return TRUE;
	}

	return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZOMBIE_Zombie_FromRow::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCZOMBIE_Zombie::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBCRCMD_CommandLimit)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBCRCMD_CommandLimit - IDBCreateCommand::CommandLimit
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CommandLimit::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD::Init())
	// }}
	{
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - 100 Command Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CommandLimit::Variation_1()
{
	BOOL		fSuccess = TRUE;
	ICommand*	rgpCommand[MAX_CMDOBJ];
	HRESULT		hr = E_FAIL;
	ULONG		udCmdCount = 0;

	// LOOP 100 times creating ICommands
	for(udCmdCount=0; udCmdCount < MAX_CMDOBJ; udCmdCount++)
	{
		// ICommand REFIID on IDBCreateCommand 
		hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
									  (IUnknown**)&rgpCommand[udCmdCount]);
		
		// If this fails on the 1st CreateCommand return E_FAIL
		if((hr != S_OK) || (!rgpCommand[udCmdCount]))
		{
			// Return FALSE because of an ERROR
			if(((hr == S_OK) && (!rgpCommand[udCmdCount])) ||
			   ((hr != S_OK) && (!udCmdCount)))
				fSuccess = FALSE;

			break;
		}
	}

	// Display the number of Commands created
	odtLog <<L"IDBCreateCommand created " <<udCmdCount <<L" objects." <<ENDL;
	
	// Checking ref count is correct after release
	for(udCmdCount; udCmdCount > 0; udCmdCount--)
		SAFE_RELEASE_(rgpCommand[udCmdCount-1]);

	// Create 1 more ICommand to see if Release really worked
	if(!CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,
						(IUnknown**)&rgpCommand[0]), S_OK) || (!rgpCommand[0]))
		fSuccess = FALSE;

	// Release the Interfaces
	SAFE_RELEASE_(rgpCommand[0]);

	if(fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE

//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CommandLimit::Variation_2()
{
	BOOL		fSuccess = FALSE;
	ICommand*	rgpCommand[2];
	HRESULT		hr = E_FAIL;

	// Check to see if the provider supports DBPROP_MULTIPLECONNECTIONS
	if( !::GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) ||
		!SettableProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) )
	{
		odtLog << L"DBPROP_MULTIPLECONNECTIONS can not be set to VARIANT_TRUE." << ENDL;
		return TEST_SKIPPED;
	}

	// ICommand REFIID on IDBCreateCommand 
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
											(IUnknown**)&rgpCommand[0]),S_OK);
	
	// ICommand REFIID on IDBCreateCommand 
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
											(IUnknown**)&rgpCommand[1]),S_OK);
	
	// Release the Interfaces
	SAFE_RELEASE(rgpCommand[0]);

	// Create 1 more ICommand to see if Release really worked
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,
											(IUnknown**)&rgpCommand[0]), S_OK);

	COMPARE(rgpCommand[1] != NULL, TRUE);
	fSuccess = TRUE;

CLEANUP:

	// Release the Interfaces
	SAFE_RELEASE(rgpCommand[0]);
	SAFE_RELEASE_(rgpCommand[1]);

	if(fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE

//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRCMD_CommandLimit::Variation_3()
{
	BOOL				fSuccess = FALSE;
	HRESULT				hr = E_FAIL;
	ULONG				cPropSets  = 0;
	DBPROPSET *			rgPropSets = NULL;
	ICommand*			rgpCommand[2]	  = {NULL,NULL};
	IDBProperties *		pIDBProperties	  = NULL;
	IDBCreateSession *	pIDBCreateSession = NULL;
	IDBCreateCommand *	pIDBCreateCommand = NULL;

	// Create a fresh DataSource
	TESTC_(GetModInfo()->CreateProvider(NULL,IID_IDBProperties,
										(IUnknown**)&pIDBProperties), S_OK);
	
	TESTC_(InitializeDataSource(pIDBProperties), S_OK);

	// Check to see if the provider supports DBPROP_MULTIPLECONNECTIONS
	if( !SupportedProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIDBProperties) ||
		(::GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIDBProperties) &&
		 !SettableProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIDBProperties)) )
	{
		odtLog << L"DBPROP_MULTIPLECONNECTIONS can not be set to VARIANT_FALSE." << ENDL;
		SAFE_RELEASE(pIDBProperties);
		return TEST_SKIPPED;
	}

	// Set the Property to VARIANT_TRUE
	TESTC_(::SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, 
								&cPropSets, &rgPropSets, DBTYPE_BOOL, (ULONG_PTR) VARIANT_FALSE), S_OK);

	TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK);

	// Get a new Session object
	QTESTC(VerifyInterface(pIDBProperties, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown**)&pIDBCreateSession));

	TESTC_(pIDBCreateSession->CreateSession(NULL, IID_IDBCreateCommand, 
											(IUnknown**)&pIDBCreateCommand), S_OK);

	// ICommand REFIID on IDBCreateCommand 
	TESTC_(pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
											(IUnknown**)&rgpCommand[0]),S_OK);
	
	// ICommand REFIID on IDBCreateCommand 
	TEST2C_(pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
											(IUnknown**)&rgpCommand[1]),S_OK,DB_E_OBJECTOPEN);
	SAFE_RELEASE(rgpCommand[1]);
	
	TEST2C_(pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,
											(IUnknown**)&rgpCommand[1]),S_OK,DB_E_OBJECTOPEN);
	fSuccess = TRUE;

CLEANUP:

	// Release the Interfaces
	SAFE_RELEASE(rgpCommand[0]);
	SAFE_RELEASE_(rgpCommand[1]);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pIDBCreateSession);
	::FreeProperties(&cPropSets, &rgPropSets);

	if(fSuccess)
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
BOOL TCIDBCRCMD_CommandLimit::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CommandLimit_FromSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD_CommandLimit::Init())
	// }}
	{
		// IDBCreateSession
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown,IID_IDBCreateSession,
							DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
		// IDBCreateCommand
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown2,IID_IDBCreateCommand,
							SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand));
		return TRUE;
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
BOOL TCIDBCRCMD_CommandLimit_FromSession::Terminate()
{
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pIDBCreateCommand);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD_CommandLimit::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD_CommandLimit_FromRow::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD_CommandLimit::Init())
	// }}
	{
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
BOOL TCIDBCRCMD_CommandLimit_FromRow::Terminate()
{
	// Destroy the row object
	SAFE_DELETE(m_pCRowObject);

	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD_CommandLimit::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		07/11/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRCMD::Init())
	// }}
	{
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IDBCreateCommand calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	BOOL	  fSuccess	= FALSE;
	HRESULT   hr		= E_FAIL;
	ICommand* pCommand  = NULL;
	
	// Cause an Error
	m_pExtError->CauseError();

	// CreateCommand with IID_ICommand
	if(CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
							(IUnknown**)&pCommand), S_OK) && (pCommand))
		fSuccess = TRUE;

	// Do extended check following CreateCommand
	fSuccess &= XCHECK(m_pIDBCreateCommand, IID_IDBCreateCommand, hr);	
 
	// Release the Command
	SAFE_RELEASE_(pCommand);

	if(fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IDBCreateCommand calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL	  fSuccess	= FALSE;
	HRESULT   hr		= E_FAIL;
	ICommand* pCommand  = NULL;

	// Cause an Error
	m_pExtError->CauseError();
  
 	// IID_IDBCreateCommand REFIID on CreateCommand 
	if(CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_IDBCreateCommand, 
							(IUnknown**)&pCommand), E_NOINTERFACE) && (!pCommand))
		fSuccess = TRUE;
  	
	//Do extended check following CreateCommand
	fSuccess &= XCHECK(m_pIDBCreateCommand, IID_IDBCreateCommand, hr);	

	// Release the Command
	SAFE_RELEASE_(pCommand);

	if(fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IDBCreateCommand calls with no previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	BOOL	  fSuccess	= FALSE;
	HRESULT   hr		= E_FAIL;

	// NULL pICommand on CreateCommand 
	if(CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, 
										IID_ICommand, NULL), E_INVALIDARG))
		fSuccess = TRUE;

	// Do extended check following CreateCommand
	fSuccess &= XCHECK(m_pIDBCreateCommand, IID_IDBCreateCommand, hr);	
	
	if(fSuccess)
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
BOOL TCExtendedErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRCMD::Terminate());
}	// }}

// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_FromSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCExtendedErrors::Init())
	// }}
	{
		// IDBCreateSession
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown,IID_IDBCreateSession,
							DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
		// IDBCreateCommand
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown2,IID_IDBCreateCommand,
							SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand));

		return TRUE;
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
BOOL TCExtendedErrors_FromSession::Terminate()
{
	// Release Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pIDBCreateCommand);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCExtendedErrors::Terminate());
}	// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_FromRow::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCExtendedErrors::Init())
	// }}
	{
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
BOOL TCExtendedErrors_FromRow::Terminate()
{
	// Destroy the row object
	SAFE_DELETE(m_pCRowObject);

	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCExtendedErrors::Terminate());
}	// }}


//--------------------------------------------------------------------
// @mfunc Create a row object and try to QI for IDBCreateCommand
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRCMD::InitializeRowObject()
{
	HRESULT	hr = E_FAIL;
	CRowset Rowset;

	m_pCRowObject = NULL;
	m_pIDBCreateCommand = NULL;
	m_pIDBCreateSession = NULL;
	
	// IDBCreateSession
	TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown,IID_IDBCreateSession,
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
	//Create a new row object 
	m_pCRowObject = new CRowObject;
	TESTC(m_pCRowObject != NULL);	

	//Create the Rowset object and obtain first row
	hr = Rowset.CreateRowset();
	if (hr==DB_E_NOTSUPPORTED)
			TESTC_PROVIDER(FALSE);

	TESTC_(hr, S_OK);
	TESTC_(Rowset.GetNextRows(&m_hRow),S_OK);

	//Now create the row object.
	QTESTC(SUCCEEDED(hr = m_pCRowObject->CreateRowObject(Rowset.pIRowset(), m_hRow)));
	Rowset.ReleaseRows(m_hRow);

	if(!VerifyInterface(m_pCRowObject->pIRow(), IID_IDBCreateCommand, ROW_INTERFACE,(IUnknown**)&m_pIDBCreateCommand))
		return TEST_SKIPPED;

	return TEST_PASS;

CLEANUP:
	if (hr==E_NOINTERFACE || hr==DB_E_NOTSUPPORTED)
		return TEST_SKIPPED;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Test Interface Routine
//
// @rdesc S_OK or E_FAIL
//
int TCMULTICMD::TestInterface(IID iid, BOOL mandatory)
{
	TBEGIN;

	ICommand* pCommand  = NULL;
	IUnknown* pIUnknown = NULL;
	
	// Check the levels
	if( mandatory || IsReqInterface(COMMAND_INTERFACE, iid) ) 
	{
		// CreateCommand with a mandatory interface should return S_OK
		TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, iid, (IUnknown**)&pCommand), S_OK);
		TESTC(DefaultObjectTesting(pCommand, COMMAND_INTERFACE));
	}
	else
	{
		// CreateCommand should return S_OK or E_NOINTERFACE
		HRESULT hr=m_pIDBCreateCommand->CreateCommand(NULL, iid, (IUnknown**)&pCommand);
		
		// Check the returncode and pointer
		if( hr == E_NOINTERFACE ) {
			TESTC(!pCommand);
		}
		else {
			TESTC(DefaultObjectTesting(pCommand, COMMAND_INTERFACE));
		}
	}

CLEANUP:
	
	// Release Interfaces
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE_(pCommand);

	TRETURN;
}


//--------------------------------------------------------------------
// @mfunc Test Zombie cases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	int					fPassFail			= TEST_FAIL;	// ReturnValue
	HRESULT				ExpectedHr			= E_UNEXPECTED;	// Expected HRESULT
	DBCOUNTITEM			cRowsObtained		= 0;			// Number of rows returned, should be 1
	HROW *				rghRows				= NULL;			// Array of Row Handles
	IDBCreateCommand *	pIDBCreateCommand	= NULL;			// IDBCreateCommand
	ICommand *			pICommand			= NULL;			// ICommand Pointer

	// Retrieve an Interface pointer to IDBCreateCommand within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIDBCreateCommand));

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT)  && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Commit or Abort the transaction, with retention as specified
	if( eTxn == ETXN_COMMIT )
		TESTC(GetCommit(fRetaining));

	if( eTxn == ETXN_ABORT )
		TESTC(GetAbort(fRetaining));

	// Test zombie
	TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
	
	// Release the row handle on the 1st rowset
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	if( m_fRowCommand )
	{
		// CreateCommand with IID_ICommand
		TESTC_(pIDBCreateCommand->CreateCommand(NULL, 
									IID_ICommand, (IUnknown**)&pICommand), ExpectedHr);
		fPassFail = TEST_PASS;
	}
	else
	{
		// CreateCommand with IID_ICommand
		TESTC_(pIDBCreateCommand->CreateCommand(NULL, 
									IID_ICommand, (IUnknown**)&pICommand), S_OK);
		fPassFail = TEST_PASS;
	}

CLEANUP:

	// Release the ICommand and IDBCreateCommand
	SAFE_RELEASE_(pICommand);
	SAFE_RELEASE(pIDBCreateCommand);

	return fPassFail;
}
