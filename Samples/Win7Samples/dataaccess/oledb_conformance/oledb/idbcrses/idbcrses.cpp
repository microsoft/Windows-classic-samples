//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IDBCRSES.CPP | This is the IDBCreateSession.
//

#include "modstandard.hpp"	// Standard headers, precompiled in modcore.cpp			
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "idbcrses.h"		// Testcase's header 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x0d086300, 0xb4c9, 0x11cf, { 0x99, 0x00, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("IDBCreateSession");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IDBCreateSession Interface.");
DECLARE_MODULE_VERSION(832876340);
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
	// Check to see if you can get IDBCreateSession
	return ModuleCreateDBSession(pThisTestModule);
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
	// Free the interface we got in ModuleCreateDBSession()
	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCIDBCRSES Base Class for IDBCreateSession:CreateSession Testcases
class TCIDBCRSES : public CDataSourceObject
{
	public:
		// @cmember Constructor
		TCIDBCRSES(LPWSTR wstrTestCaseName) : CDataSourceObject(wstrTestCaseName)
		{
			m_pIDBCreateSession	= NULL;
		};

		// @cmember Destructor
		virtual ~TCIDBCRSES(){};

		virtual BOOL TestCreateSession(BOOL fDSOInit);

	protected:
		// @cmember IDBCreateSession Interface
		IDBCreateSession * m_pIDBCreateSession;
};


// @class TCZOMBIE Base Class for IDBCreateSession:ZombieSession Testcases
class TCZOMBIE : public CTransaction
{
	public:
		// @cmember Constructor
		TCZOMBIE(LPWSTR wstrTestCaseName) : CTransaction(wstrTestCaseName){};

		// @cmember Destructor
		virtual ~TCZOMBIE(){};

	protected:
};


// @class TCMULTISES Base Class for IDBCreateSession:MultipleObject Testcases
class TCMULTISES : public COLEDB
{
	public:
		// @cmember Constructor
		TCMULTISES(LPWSTR wstrTestCaseName) : COLEDB(wstrTestCaseName)
		{
			m_pIDBCreateSession	= NULL;
		};

		// @cmember Destructor
		virtual ~TCMULTISES(){};

		// @cmember Tests QI on IUnknown from a IDBCreateSession Pointer
		int TestInterface(IID iid, BOOL mandatory);

	protected:
		// @cmember IDBCreateSession Interface
		IDBCreateSession * m_pIDBCreateSession;
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIDBCRSES_CreateSession)
//--------------------------------------------------------------------
// @class IDBCreateSession::CreateSession
//
class TCIDBCRSES_CreateSession : public TCIDBCRSES { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRSES_CreateSession,TCIDBCRSES);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_NOINTERFACE - DATASOURCE REFID's
	int Variation_1();
	// @cmember E_NOINTERFACE - COMMAND REFID's
	int Variation_2();
	// @cmember E_NOINTERFACE - ROWSET REFID's
	int Variation_3();
	// @cmember E_NOINTERFACE - IID_NULL REFID
	int Variation_4();
	// @cmember E_INVALIDARG - NULL ppDBSession
	int Variation_5();
	// @cmember S_OK - IID_IOpenRowset for REFIID
	int Variation_6();
	// @cmember S_OK - IID_IGetDataSource for REFIID
	int Variation_7();
	// @cmember S_OK - IID_ITransaction for REFIID
	int Variation_8();
	// @cmember E_UNEXPECTED - CreateSession while Uninitialized
	int Variation_9();
	// @cmember E_UNEXPECTED - CreateSession after calling Uninitialize twice
	int Variation_10();
	// @cmember E_UNEXPECTED - QI for IDBCreateSession before Initialized
	int Variation_11();
	// @cmember S_OK - Default Interface testing
	int Variation_12();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBCRSES_CreateSession)
#define THE_CLASS TCIDBCRSES_CreateSession
BEG_TEST_CASE(TCIDBCRSES_CreateSession, TCIDBCRSES, L"IDBCreateSession::CreateSession")
	TEST_VARIATION(1, 		L"E_NOINTERFACE - DATASOURCE REFID's")
	TEST_VARIATION(2, 		L"E_NOINTERFACE - COMMAND REFID's")
	TEST_VARIATION(3, 		L"E_NOINTERFACE - ROWSET REFID's")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - IID_NULL REFID")
	TEST_VARIATION(5, 		L"E_INVALIDARG - NULL ppDBSession")
	TEST_VARIATION(6, 		L"S_OK - IID_IOpenRowset for REFIID")
	TEST_VARIATION(7, 		L"S_OK - IID_IGetDataSource for REFIID")
	TEST_VARIATION(8, 		L"S_OK - IID_ITransaction for REFIID")
	TEST_VARIATION(9, 		L"E_UNEXPECTED - CreateSession while Uninitialized")
	TEST_VARIATION(10, 		L"E_UNEXPECTED - CreateSession after calling Uninitialize twice")
	TEST_VARIATION(11, 		L"E_UNEXPECTED - QI for IDBCreateSession before Initialized")
	TEST_VARIATION(12, 		L"S_OK - Default Interface testing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMULTISES_QueryInterface)
//--------------------------------------------------------------------
// @class IDBCreateSession::MultipleObjects
//
class TCMULTISES_QueryInterface : public TCMULTISES { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMULTISES_QueryInterface,TCMULTISES);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - IUnknown
	int Variation_1();
	// @cmember S_OK - IOpenRowset
	int Variation_2();
	// @cmember S_OK - ISessionProperties
	int Variation_3();
	// @cmember S_OK - IGetDataSource
	int Variation_4();
	// @cmember S_OK - IDBCreateCommand
	int Variation_5();
	// @cmember S_OK - ITransaction
	int Variation_6();
	// @cmember S_OK - ITransactionLocal
	int Variation_7();
	// @cmember S_OK - ITransactionObject
	int Variation_8();
	// @cmember S_OK - ITransactionJoin
	int Variation_9();
	// @cmember S_OK - ITableDefinition
	int Variation_10();
	// @cmember S_OK - IIndexDefinition
	int Variation_11();
	// @cmember S_OK - IDBSchemaRowset
	int Variation_12();
	// @cmember S_OK - ISupportErrorInfo
	int Variation_13();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCMULTISES_QueryInterface)
#define THE_CLASS TCMULTISES_QueryInterface
BEG_TEST_CASE(TCMULTISES_QueryInterface, TCMULTISES, L"IDBCreateSession::MultipleObjects")
	TEST_VARIATION(1, 		L"S_OK - IUnknown")
	TEST_VARIATION(2, 		L"S_OK - IOpenRowset")
	TEST_VARIATION(3, 		L"S_OK - ISessionProperties")
	TEST_VARIATION(4, 		L"S_OK - IGetDataSource")
	TEST_VARIATION(5, 		L"S_OK - IDBCreateCommand")
	TEST_VARIATION(6, 		L"S_OK - ITransaction")
	TEST_VARIATION(7, 		L"S_OK - ITransactionLocal")
	TEST_VARIATION(8, 		L"S_OK - ITransactionObject")
	TEST_VARIATION(9, 		L"S_OK - ITransactionJoin")
	TEST_VARIATION(10, 		L"S_OK - ITableDefinition")
	TEST_VARIATION(11, 		L"S_OK - IIndexDefinition")
	TEST_VARIATION(12, 		L"S_OK - IDBSchemaRowset")
	TEST_VARIATION(13, 		L"S_OK - ISupportErrorInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCZOMBIE_Zombie)
//--------------------------------------------------------------------
// @class IDBCreateSession::ZombieSession
//
class TCZOMBIE_Zombie : public TCZOMBIE { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZOMBIE_Zombie,TCZOMBIE);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember TestTxn
	int TestTxn(ETXN eTxn, BOOL fRetaining);
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IDBCreateSession with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit IDBCreateSession with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Abort IDBCreateSession with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit IDBCreateSession with fRetaining=FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCZOMBIE_Zombie)
#define THE_CLASS TCZOMBIE_Zombie
BEG_TEST_CASE(TCZOMBIE_Zombie, TCZOMBIE, L"IDBCreateSession::ZombieSession")
	TEST_VARIATION(1, 		L"S_OK - Abort IDBCreateSession with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit IDBCreateSession with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort IDBCreateSession with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit IDBCreateSession with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBCRSES_SessionLimit)
//--------------------------------------------------------------------
// @class IDBCreateSession::SessionLimit
//
class TCIDBCRSES_SessionLimit : public TCIDBCRSES { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBCRSES_SessionLimit,TCIDBCRSES);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Create the Maximum Session Objects
	int Variation_1();
	// @cmember S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE
	int Variation_2();
	// @cmember DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBCRSES_SessionLimit)
#define THE_CLASS TCIDBCRSES_SessionLimit
BEG_TEST_CASE(TCIDBCRSES_SessionLimit, TCIDBCRSES, L"IDBCreateSession::SessionLimit")
	TEST_VARIATION(1, 		L"S_OK - Create the Maximum Session Objects")
	TEST_VARIATION(2, 		L"S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE")
	TEST_VARIATION(3, 		L"DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public TCIDBCRSES { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,TCIDBCRSES);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IDBCreateSession calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IDBCreateSession calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IDBCreateSession calls with no previous error object existing
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, TCIDBCRSES, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IDBCreateSession calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IDBCreateSession calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid IDBCreateSession calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCInterface)
//*-----------------------------------------------------------------------
// @class Default interface testing
//
class TCInterface : public TCIDBCRSES { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCInterface,TCIDBCRSES);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Default Testing
	int Variation_1();
	// @cmember Default Testing after init and uninit
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCInterface)
#define THE_CLASS TCInterface
BEG_TEST_CASE(TCInterface, TCIDBCRSES, L"Default interface testing")
	TEST_VARIATION(1, 		L"Default Testing")
	TEST_VARIATION(2, 		L"Default Testing after init and uninit")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(6, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIDBCRSES_CreateSession)
	TEST_CASE(2, TCMULTISES_QueryInterface)
	TEST_CASE(3, TCZOMBIE_Zombie)
	TEST_CASE(4, TCIDBCRSES_SessionLimit)
	TEST_CASE(5, TCExtendedErrors)
	TEST_CASE(6, TCInterface)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIDBCRSES_CreateSession)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBCRSES_CreateSession - IDBCreateSession::CreateSession
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRSES_CreateSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRSES::Init())
	// }}
	{
		// Get an IDBInitialize Pointer
		TESTC_(CreateDataSourceObject(), S_OK);

		// Initialize the DSO
		TESTC_(InitializeDSO(REINITIALIZE_YES), S_OK);

		// QueryInterface for IDBCreateSession
		return VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
					DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession);
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - DATASOURCE REFID's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_1()
{
	TBEGIN;
	ULONG		  ulIndex	   = 0;
	ULONG		  cInterfaces  = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	IUnknown*	  pDBSession   = INVALID(IUnknown*);

	//Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(DATASOURCE_INTERFACE, &cInterfaces, &rgInterfaces));

	// Loop thru the DSO interface
	for(ulIndex=0; ulIndex < cInterfaces; ulIndex++)
	{
		// Check to see if the DSO interface is in the SESSION interface
		if( IsValidInterface(SESSION_INTERFACE, *(rgInterfaces[ulIndex].pIID)) )
			continue;

		// DSO interfaces are not valid REFIID on CreateSession 
		TESTC_(m_pIDBCreateSession->CreateSession(NULL, *(rgInterfaces[ulIndex].pIID), 
									(IUnknown**)&pDBSession), E_NOINTERFACE);
		TESTC(!pDBSession);
	}

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - COMMAND REFID's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_2()
{
	TBEGIN;
	ULONG		  ulIndex	   = 0;
	ULONG		  cInterfaces  = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	IUnknown*	  pDBSession   = INVALID(IUnknown*);

	//Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(COMMAND_INTERFACE, &cInterfaces, &rgInterfaces));

	// Loop thru the DSO interface
	for(ulIndex=0; ulIndex < cInterfaces; ulIndex++)
	{
		// Check to see if the DSO interface is in the SESSION interface
		if( IsValidInterface(SESSION_INTERFACE, *(rgInterfaces[ulIndex].pIID)) )
			continue;

		// COMMAND interfaces are not valid REFIID on CreateSession 
		TESTC_(m_pIDBCreateSession->CreateSession(NULL, *(rgInterfaces[ulIndex].pIID), 
									(IUnknown**)&pDBSession), E_NOINTERFACE);
		TESTC(!pDBSession);
	}

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - ROWSET REFID's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_3()
{
	TBEGIN;
	ULONG		  ulIndex	   = 0;
	ULONG		  cInterfaces  = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	IUnknown*	  pDBSession   = INVALID(IUnknown*);

	//Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cInterfaces, &rgInterfaces));

	// Loop thru the DSO interface
	for(ulIndex=0; ulIndex < cInterfaces; ulIndex++)
	{
		// Check to see if the DSO interface is in the SESSION interface
		if( IsValidInterface(SESSION_INTERFACE, *(rgInterfaces[ulIndex].pIID)) )
			continue;

		// ROWSET interfaces are not valid REFIID on CreateSession 
		TESTC_(m_pIDBCreateSession->CreateSession(NULL, *(rgInterfaces[ulIndex].pIID), 
									(IUnknown**)&pDBSession), E_NOINTERFACE);
		TESTC(!pDBSession);
	}

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - IID_NULL REFID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_4()
{
	TBEGIN;
	IUnknown*	  pDBSession   = INVALID(IUnknown*);

	// ROWSET interfaces are not valid REFIID on CreateSession 
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_NULL, 
								(IUnknown**)&pDBSession), E_NOINTERFACE);
	TESTC(!pDBSession);

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL ppDBSession
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_5()
{
	TBEGIN;

	// NULL ppDBSession on CreateSession
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
									IID_IOpenRowset, NULL), E_INVALIDARG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_IOpenRowset for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_6()
{
	TBEGIN;
	IUnknown* pDBSession = INVALID(IUnknown*);

	// CreateSession with IID_IOpenRowset (Mandatory)
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, 
											(IUnknown**)&pDBSession), S_OK);
	TESTC(pDBSession != NULL);
	TESTC(pDBSession != INVALID(IUnknown*));

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_IGetDataSource for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_7()
{
	TBEGIN;
	IUnknown* pDBSession = INVALID(IUnknown*);

	// CreateSession with IID_IGetDataSource (Mandatory)
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, 
											(IUnknown**)&pDBSession), S_OK);
	TESTC(pDBSession != NULL);
	TESTC(pDBSession != INVALID(IUnknown*));

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IID_ITransaction for REFIID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_8()
{
	TBEGIN;
	HRESULT	    hr		 = E_FAIL;
	IUnknown    *pDBSession = INVALID(IUnknown*);
	ULONG_PTR   ulValue	 = DBPROPVAL_TC_NONE;

	// Get the value of the property
	GetProperty(DBPROP_SUPPORTEDTXNDDL, DBPROPSET_DATASOURCEINFO, 
												m_pIDBCreateSession, &ulValue);

	// CreateSession with IID_ITransaction (optional)
	TEST2C_(hr=m_pIDBCreateSession->CreateSession(NULL, IID_ITransaction, 
								(IUnknown**)&pDBSession), S_OK, E_NOINTERFACE);

	// pDBSession should get a valid address on S_OK and NULL on E_NOINTERFACE
	// Check to see if the DBPROP_SUPPORTEDTXNDDL property is supported
	if( FAILED(hr) ) 
	{
		TESTC(pDBSession == NULL);
		if( ulValue != DBPROPVAL_TC_NONE )
			TWARNING("DBPROP_SUPPORTEDTXNDDL = returned something other than DBPROPVAL_TC_NONE, is this correct?");
	}
	else 
	{
		TESTC(pDBSession != NULL);
		if( ulValue == DBPROPVAL_TC_NONE )
			TWARNING("DBPROP_SUPPORTEDTXNDDL = DBPROPVAL_TC_NONE, is this correct?");
	}

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_UNEXPECTED - CreateSession while Uninitialized
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_9()
{
	TBEGIN;
	IUnknown* pDBSession = INVALID(IUnknown*);

	// Uninitialize the DSO
	TESTC_(UninitializeDSO(), S_OK);

	// CreateSession while Uninitialized
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IUnknown, 
									(IUnknown**)&pDBSession), E_UNEXPECTED);
	TESTC(!pDBSession);

	// Initialize  the DSO again
	TESTC_(InitializeDSO(REINITIALIZE_YES), S_OK);

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_UNEXPECTED - CreateSession after calling Uninitialize twice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_10()
{
	TBEGIN;
	IUnknown* pDBSession = INVALID(IUnknown*);

	// Uninitialize the DSO twice
	TESTC_(UninitializeDSO(), S_OK);
	TESTC_(UninitializeDSO(), S_OK);

	// CreateSession while Uninitialized
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IUnknown, 
									(IUnknown**)&pDBSession), E_UNEXPECTED);
	TESTC(!pDBSession);

	// Initialize  the DSO again
	TESTC_(InitializeDSO(REINITIALIZE_YES), S_OK);

CLEANUP:
	
	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_UNEXPECTED - QI for IDBCreateSession before Initialized
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_11()
{
	IDBProperties *	   pIDBProperties	 = INVALID(IDBProperties*);
	IDBCreateSession * pIDBCreateSession = INVALID(IDBCreateSession*);
	IUnknown *		   pDBSession		 = INVALID(IUnknown*);

	TESTC_(GetModInfo()->CreateProvider(NULL,IID_IDBProperties,
										(IUnknown**)&pIDBProperties), S_OK);

	// QI for IDBCreateSession before Initialized
	TEST3C_(pIDBProperties->QueryInterface(IID_IDBCreateSession,
				(LPVOID*)&pIDBCreateSession),S_OK, E_UNEXPECTED, E_NOINTERFACE);

	// Check either E_UNEXPECTED on the QI or on the CreateSession
	if( pIDBCreateSession ) {
		TESTC_(pIDBCreateSession->CreateSession(NULL,IID_IUnknown,
												&pDBSession), E_UNEXPECTED);
	}

CLEANUP:

	// Release Object
    if( pDBSession != INVALID(IUnknown*) )
		SAFE_RELEASE(pDBSession);

    if( pIDBCreateSession != INVALID(IDBCreateSession*) )
		SAFE_RELEASE(pIDBCreateSession);

    if( pIDBProperties != INVALID(IDBProperties*) )
		SAFE_RELEASE(pIDBProperties);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Default Interface testing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_CreateSession::Variation_12()
{
	TBEGIN;

	QTESTC(DefaultInterfaceTesting(m_pIDBCreateSession, 
								 DATASOURCE_INTERFACE, IID_IDBCreateSession));
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRSES_CreateSession::Terminate()
{
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRSES::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCMULTISES_QueryInterface)
//*-----------------------------------------------------------------------
//| Test Case:		TCMULTISES_QueryInterface - IDBCreateSession::MultipleObjects
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTISES_QueryInterface::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMULTISES::Init())
	// }}
	{
		// Release the Session Object
		SAFE_RELEASE(m_pThisTestModule->m_pIUnknown2);
		return TRUE;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_1()
{
	// IUnknown REFIID is mandatory 
	return TestInterface(IID_IUnknown,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_2()
{
	// IOpenRowset REFIID is mandatory 
	return TestInterface(IID_IOpenRowset,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ISessionProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_3()
{
	// IID_ISessionProperties REFIID is mandatory 
	return TestInterface(IID_ISessionProperties,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_4()
{
	// IGetDataSource REFIID is mandatory 
	return TestInterface(IID_IGetDataSource,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IDBCreateCommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_5()
{
	// IDBCreateCommand REFIID is not mandatory 
	return TestInterface(IID_IDBCreateCommand,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_6()
{
	// ITransaction REFIID is not mandatory 
	return TestInterface(IID_ITransaction,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ITransactionLocal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_7()
{
	// ITransactionLocal REFIID is not mandatory 
	return TestInterface(IID_ITransactionLocal,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ITransactionObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_8()
{
	// ITransactionObject REFIID is not mandatory 
	return TestInterface(IID_ITransactionObject,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ITransactionJoin
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_9()
{
	// ITransactionJoin REFIID is not mandatory 
	return TestInterface(IID_ITransactionJoin,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ITableDefinition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_10()
{
	// ITableDefinition REFIID is not mandatory 
	return TestInterface(IID_ITableDefinition,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IIndexDefinition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_11()
{
	// IIndexDefinition REFIID is not mandatory 
	return TestInterface(IID_IIndexDefinition,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK - IDBSchemaRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_12()
{
	// IDBSchemaRowset REFIID is not mandatory 
	return TestInterface(IID_IDBSchemaRowset,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK - ISupportErrorInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMULTISES_QueryInterface::Variation_13()
{
	// ISupportErrorInfo REFIID is not mandatory 
	return TestInterface(IID_ISupportErrorInfo,FALSE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMULTISES_QueryInterface::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCMULTISES::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZOMBIE_Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZOMBIE_Zombie - IDBCreateSession::ZombieSession
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
	if( !IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal) )
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);

	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCZOMBIE::Init() )
	// }}
	{
		// IDBCreateSession
		TESTC(m_pIDBCreateSession != NULL);
		
		// Register Interface with Zombie
		return RegisterInterface(DATASOURCE_INTERFACE,IID_IDBCreateSession,0,NULL);
	}

	// Check to see if ITransaction is supported
    if( !m_pITransactionLocal )
		return TEST_SKIPPED;

CLEANUP:

    // Clear the bad pointer value
	if( m_pITransactionLocal == INVALID(ITransactionLocal*) )
		m_pITransactionLocal = NULL;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBCreateSession with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_1()
{
	// S_OK - Abort IDBCreateSession with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBCreateSession with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_2()
{
	// S_OK - Commit IDBCreateSession with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBCreateSession with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_3()
{
	// S_OK - Abort IDBCreateSession with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBCreateSession with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_4()
{
	// S_OK - Commit IDBCreateSession with fRetaining=FALSE
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
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCZOMBIE::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBCRSES_SessionLimit)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBCRSES_SessionLimit - IDBCreateSession::SessionLimit
//|	Created:		11/25/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRSES_SessionLimit::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRSES::Init())
	// }}
	{
		// IDBCreateSession
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown,IID_IDBCreateSession,
							DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		
		// Release the Session Object
		SAFE_RELEASE(m_pThisTestModule->m_pIUnknown2);
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Create the Maximum Session Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_SessionLimit::Variation_1()
{
	TBEGIN;
	HRESULT		hr = E_FAIL;
	ULONG		ulIndex  = 0;
	ULONG_PTR	ulSesLmt = 0;
	ULONG_PTR	ulValue	 = 0;
	IUnknown **	rgpDBSession = NULL;

	// Figure out how many sessions are valid
	if( GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, 
											m_pIDBCreateSession, &ulValue) )
		odtLog <<L"DBPROP_ACTIVESESSIONS returned " <<ulValue <<ENDL;

	// Set to the MAX_SESOBJ
	ulValue ? ulSesLmt=ulValue : ulSesLmt=MAX_SESOBJ ;
	
	// Allocate space for max sessions
	SAFE_ALLOC(rgpDBSession, IUnknown*, ulSesLmt+1);
	ZeroMemory(rgpDBSession, (ulSesLmt+1) * sizeof(IUnknown*));

	// LOOP 100 times tring to create the Session
	for(ulIndex=0; ulIndex < ulSesLmt+1; ulIndex++)
	{
		// IOpenRowset REFIID on IDBCreateSession 
		TEST3C_(hr=m_pIDBCreateSession->CreateSession(NULL, IID_IUnknown, 
				&rgpDBSession[ulIndex]), S_OK, DB_E_OBJECTCREATIONLIMITREACHED, E_FAIL);
		
		// Test the return code and pointer
		if( FAILED(hr) ) 
		{
			TESTC(!rgpDBSession[ulIndex]);
			TESTC(ulIndex != 0);
			if( !SupportedProperty(DBPROP_ACTIVESESSIONS,
								DBPROPSET_DATASOURCEINFO,m_pIDBCreateSession) )
			{
				if( ulIndex == 1 )
					TWARNING("DBPROP_ACTIVESESSIONS is not supported?");
				continue;
			}
			
			// Check the property value
			if( hr == E_FAIL )
				COMPARE(ulValue, 0);
			else
				COMPARE(ulValue != 0, TRUE);
		}
		else
		{
			TESTC(rgpDBSession[ulIndex] != NULL);
		}
	}

	// Display the number of Sessions created
	odtLog <<L"IDBCreateSession created " <<ulSesLmt <<L" objects." <<ENDL;
	
	// If the limit is between 1 and 100 check to see if it is correct
	COMPARE(ulIndex-1, ulSesLmt);

	// Checking ref count is correct after release
	for(; ulIndex > 0; ulIndex--)
		SAFE_RELEASE(rgpDBSession[ulIndex-1]);

	// Create 1 more Session Object	to see if Release really worked
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IUnknown, 
													&rgpDBSession[0]), S_OK);
	TESTC(rgpDBSession[0] != NULL);

CLEANUP:

	// Release the Interfaces
	for(ulIndex=0; ulIndex < ulSesLmt; ulIndex++)
		SAFE_RELEASE(rgpDBSession[ulIndex]);
	PROVIDER_FREE(rgpDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_SessionLimit::Variation_2()
{
	TBEGIN;
	ULONG				 cPropSets  = 0;
	DBPROPSET *			 rgPropSets = NULL;
	IDBProperties *		 pIDBProperties	= NULL;
	IOpenRowset *		 pIOpenRowset = NULL;
	ISessionProperties * pISessionProperties = NULL;

	// Check to see if the provider supports DBPROP_MULTIPLECONNECTIONS
	if( !::GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) &&
		!SettableProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) )
	{
		odtLog<< L"DBPROP_MULTIPLECONNECTIONS can not be set to VARIANT_TRUE." <<ENDL;
		return TEST_SKIPPED;
	}

	// Get a new Session object
	TESTC(VerifyInterface(m_pIDBCreateSession, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Set the Property to VARIANT_TRUE
	TESTC_(SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, 
								&cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE), S_OK);

	TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK);

	// Create a new Session
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, 
											(IUnknown**)&pIOpenRowset),S_OK);
	
	// Release the Interfaces
	SAFE_RELEASE(pIOpenRowset);

	// Create 1 more new Session
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, 
											(IUnknown**)&pIOpenRowset),S_OK);
	
	// Create 1 more new Session
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_ISessionProperties, 
											(IUnknown**)&pISessionProperties),S_OK);
	
	TESTC(pIOpenRowset != NULL);
	TESTC(pISessionProperties != NULL);

CLEANUP:

	// Release the Interfaces
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pISessionProperties);
	FreeProperties(&cPropSets, &rgPropSets);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - 2 Command Objects with DBPROP_MULTIPLECONNECTIONS set to VARIANT_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBCRSES_SessionLimit::Variation_3()
{
	TBEGIN;
	HRESULT				 hr = E_FAIL;
	ULONG				 cPropSets = 0;
	DBPROPSET *			 rgPropSets = NULL;
	IDBProperties *		 pIDBProperties	= NULL;
	IOpenRowset *		 pIOpenRowset = NULL;
	ISessionProperties * pISessionProperties = NULL;

	// Check to see if the provider supports DBPROP_MULTIPLECONNECTIONS
	if( ::GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) &&
		 !SettableProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) )
	{
		odtLog << L"DBPROP_MULTIPLECONNECTIONS can not be set to VARIANT_FALSE." << ENDL;
		return TEST_SKIPPED;
	}

	// Get a new Session object
	TESTC(VerifyInterface(m_pIDBCreateSession, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Set the Property to VARIANT_TRUE
	TESTC_(SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, 
								&cPropSets, &rgPropSets, DBTYPE_BOOL, (ULONG_PTR) VARIANT_FALSE), S_OK);

	if( SupportedProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, m_pIDBCreateSession) )
		TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK);

	// Create a new Session
	hr=m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, 
												(IUnknown**)&pIOpenRowset);
	TEST2C_(hr,S_OK, DB_E_OBJECTOPEN);
	
	// Create 1 more new Session
	hr=m_pIDBCreateSession->CreateSession(NULL, IID_ISessionProperties, 
											(IUnknown**)&pISessionProperties);
	
	TEST3C_(hr,S_OK, DB_E_OBJECTOPEN, DB_E_OBJECTCREATIONLIMITREACHED);

CLEANUP:

	// Release the Interfaces
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pISessionProperties);
	FreeProperties(&cPropSets, &rgPropSets);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBCRSES_SessionLimit::Terminate()
{
	// Release the Interfaces
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRSES::Terminate());
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
	if(TCIDBCRSES::Init())
	// }}
	{
		// Get an IDBInitialize Pointer
		TESTC_(CreateDataSourceObject(),S_OK);

		// Initialize the DSO
		TESTC_(InitializeDSO(REINITIALIZE_YES),S_OK);

		// Get a Session Pointer
		return VerifyInterface(m_pIDBInitialize,IID_IDBCreateSession,
						DATASOURCE_INTERFACE,(IUnknown **) &m_pIDBCreateSession);
	}

CLEANUP:
	
	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IDBCreateSession calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	TBEGIN;
	HRESULT   hr		 = E_FAIL;
	IUnknown* pDBSession = NULL;
	
	// Cause an Error
	m_pExtError->CauseError();

	// CreateSession with IID_IOpenRowset (Mandatory)
	TESTC_(hr=m_pIDBCreateSession->CreateSession(NULL, 
						IID_IOpenRowset, &pDBSession), S_OK);

	TESTC(pDBSession != NULL);

	// Do extended check following CreateSession
	TESTC(XCHECK(m_pIDBCreateSession, IID_IDBCreateSession, hr));
 
CLEANUP:

	// Release the Session
	SAFE_RELEASE(pDBSession);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IDBCreateSession calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;
	IUnknown* pDBSession = NULL;

	// Cause an Error
	m_pExtError->CauseError();
  
	// CreateSession with IID_IRowset
	TESTC_(hr=m_pIDBCreateSession->CreateSession(NULL, 
							IID_IRowset, &pDBSession), E_NOINTERFACE);
	TESTC(!pDBSession);

	// Do extended check following CreateSession
	TESTC(XCHECK(m_pIDBCreateSession, IID_IDBCreateSession, hr));
		
CLEANUP:
	
	// Release the Session
	SAFE_RELEASE(pDBSession);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IDBCreateSession calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	TBEGIN;
	HRESULT hr = E_FAIL;

	// NULL ppDBSession on CreateSession
	TESTC_(hr=m_pIDBCreateSession->CreateSession(NULL, 
								IID_IOpenRowset, NULL), E_INVALIDARG);

	// Do extended check following CreateSession
	TESTC(XCHECK(m_pIDBCreateSession, IID_IDBCreateSession, hr));

CLEANUP:

	TRETURN;
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
	// Release IDBInitialize Pointer
	SAFE_RELEASE(m_pIDBCreateSession);
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRSES::Terminate());
}	// }}

// }}


// {{ TCW_TC_PROTOTYPE(TCInterface)
//*-----------------------------------------------------------------------
//| Test Case:		TCInterface - Default interface testing
//| Created:  	3/15/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCInterface::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBCRSES::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Default Testing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCInterface::Variation_1()
{ 
	TBEGIN;

	// Get an IDBInitialize Pointer
	TESTC_(CreateDataSourceObject(), S_OK);

	// Initialize the DSO
	TESTC_(InitializeDSO(REINITIALIZE_YES), S_OK);

	// QueryInterface for IDBCreateSession
	TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
				DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));

	QTESTC(DefaultObjectTesting(m_pIDBCreateSession, DATASOURCE_INTERFACE, DSO_INIT));
	QTESTC(TestCreateSession(DSO_INIT));

CLEANUP:

	SAFE_RELEASE(m_pIDBCreateSession);
	ReleaseDataSourceObject();

	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Default Testing after init and uninit
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCInterface::Variation_2()
{ 
	TBEGIN;

	// Get an IDBInitialize Pointer
	TESTC_(CreateDataSourceObject(), S_OK);

	TESTC_(InitializeDSO(REINITIALIZE_YES), S_OK);

	// QueryInterface for IDBCreateSession
	TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
				DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));

	TESTC_(UninitializeDSO(), S_OK);
	
	QTESTC(DefaultObjectTesting(m_pIDBCreateSession, DATASOURCE_INTERFACE, DSO_UNINIT));
	QTESTC(TestCreateSession(DSO_UNINIT));
	
CLEANUP:

	SAFE_RELEASE(m_pIDBCreateSession);
	ReleaseDataSourceObject();

	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCInterface::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBCRSES::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


//--------------------------------------------------------------------
// @mfunc Test Create Session
//
// @rdesc S_OK or E_FAIL
//
BOOL TCIDBCRSES::TestCreateSession
(
	BOOL	fDSOInit
)
{
	BOOL		  fSuccess	   = FALSE;
	ULONG		  i			   = 0;
	ULONG		  cInterfaces  = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	IUnknown *	  pIUnknown    = NULL;	

	// Obtain the SESSION interface array
	TESTC(GetInterfaceArray(SESSION_INTERFACE, &cInterfaces, &rgInterfaces));

	// Try to create every session interface
	for(i=0; i < cInterfaces; i++)
	{
		// Verify HRESULT (dependant on whether the DSO is initialized or uninitialized
		if( fDSOInit )
		{
			if( rgInterfaces[i].fMandatory )
			{
				// IDBCreateSession::CreateSession [MADATORY]
				TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
									*(rgInterfaces[i].pIID), &pIUnknown),S_OK);
			}
			else
			{
				// IDBCreateSession::CreateSession [OPTIONAL]
				TEST2C_(m_pIDBCreateSession->CreateSession(NULL, 
						*(rgInterfaces[i].pIID), &pIUnknown),S_OK,E_NOINTERFACE);
			}

			// Test the interface pointer returned
			if( pIUnknown ) {
				QTESTC(DefaultInterfaceTesting(pIUnknown, SESSION_INTERFACE, *(rgInterfaces[i].pIID)));
			}
		}
		else
		{
			// IDBCreateSession::CreateSession while uninitialized
			TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
							*(rgInterfaces[i].pIID), &pIUnknown),E_UNEXPECTED);
		}

		SAFE_RELEASE(pIUnknown);
	}

	// Verify E_NOINTERFACE for IID_NULL
	if( fDSOInit )
	{
		TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
										IID_NULL, &pIUnknown), E_NOINTERFACE);
		SAFE_RELEASE(pIUnknown);
	}

	// Verify E_INVALIDARG condition
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
										IID_IUnknown, NULL), E_INVALIDARG);
	fSuccess = TRUE;

CLEANUP:

	SAFE_RELEASE(pIUnknown);
	return fSuccess;
} 


//--------------------------------------------------------------------
// @mfunc Test Interface Routine
//
// @rdesc S_OK or E_FAIL
//
int TCMULTISES::TestInterface(IID iid, BOOL mandatory)
{
	TBEGIN;
	HRESULT	  hr = E_FAIL;
	IUnknown* pSessionObject = NULL;
	IUnknown* pIUnknown		 = NULL;
	IUnknown* pIUnknown1	 = NULL;

	// IDBCreateSession
	TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
	
	// Check the IID to see if the Interface is supported
	if( (!mandatory) && (IsReqInterface(SESSION_INTERFACE, iid)) ) {
		odtLog<<L"Interface is a required Interface." << ENDL;
		goto CLEANUP;
	}

	// Check the levels
	if( (mandatory) || (IsReqInterface(SESSION_INTERFACE, iid)) ) 
	{
		// CreateSession with a mandatory interface should return S_OK
		TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
												iid, &pSessionObject), S_OK);
		TESTC_(pSessionObject->QueryInterface(IID_IUnknown,
												(LPVOID*)&pIUnknown), S_OK);
	}
	else
	{
		// CreateSession should return S_OK or E_NOINTERFACE
		TEST2C_(hr=m_pIDBCreateSession->CreateSession(NULL, iid, 
							(IUnknown**)&pSessionObject), S_OK, E_NOINTERFACE);
		
		// Check the returncode and pointer
		if( FAILED(hr) ) {
			TESTC(!pSessionObject);
		}
		else {
			TESTC(pSessionObject != NULL);
			TESTC_(pSessionObject->QueryInterface(IID_IUnknown,(LPVOID*)&pIUnknown), S_OK);
			TESTC_(pSessionObject->QueryInterface(iid,(LPVOID*)&pIUnknown1), S_OK);
		}
	}

CLEANUP:
	
	// Release Interfaces
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIUnknown1);
	SAFE_RELEASE(pSessionObject);
	SAFE_RELEASE(m_pIDBCreateSession);

	TRETURN;
}

//--------------------------------------------------------------------
// @mfunc Test Zombie cases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	TBEGIN;
	HRESULT				ExpectedHr			= E_UNEXPECTED;	// Expected HRESULT
	DBCOUNTITEM 		cRowsObtained		= 0;			// Number of rows returned, should be 1
	HROW *				rghRows				= NULL;			// Array of Row Handles
	IUnknown *			pIOpenRowset		= NULL;			// IUnknown Pointer
	IDBCreateSession *	pIDBCreateSession	= NULL;			// IDBCreateSessoin
	ULONG_PTR			ulMaxSessions		= 0;			// Nuber of Sessions

	// Retrieve an Interface pointer to IDBCreateSession within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIDBCreateSession));

	// Commit or Abort the transaction, with retention as specified
	if( eTxn == ETXN_COMMIT ) {
		TESTC(GetCommit(fRetaining));
	}
	else {
		TESTC(GetAbort(fRetaining));
	}

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT) && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Test zombie
	TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	//Provider only allows 1 active Session
	ExpectedHr = S_OK;
	GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, pIDBCreateSession, &ulMaxSessions);
	
	if( ulMaxSessions == 1 )
		ExpectedHr = DB_E_OBJECTCREATIONLIMITREACHED;

	// CreateSession with IID_IOpenRowset
	TESTC_(pIDBCreateSession->CreateSession(NULL, 
								IID_IOpenRowset, &pIOpenRowset), ExpectedHr);

CLEANUP:
	
	// Release the row handle on the 1st rowset
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Release the IOpenRowset and IDBCreateSession
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIDBCreateSession);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	TRETURN;
}

