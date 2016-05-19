//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IDBINFO.CPP | Source code for autotest IDBINFO.CPP.
//

#include "MODStandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "IDBInfo.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xd6bf9810, 0xd00c, 0x11ce, { 0x98, 0x77, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("IDBInfo");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IDBInfo Interfaces");
DECLARE_MODULE_VERSION(831585464);
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
	IDBInfo* pIDBInfo = NULL;
	BOOL bValue = FALSE;

	if(ModuleCreateDBSession(pThisTestModule))
	{
		// IDBInfo
		if(!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBInfo,
								DATASOURCE_INTERFACE, (IUnknown**)&pIDBInfo))
		{
			odtLog << L"IDBInfo is not supported by this Provider." << ENDL;
			return TEST_SKIPPED;
		}

		bValue = TRUE;
	}

	// Release the Interfaces
	SAFE_RELEASE(pIDBInfo);
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
	// Free the interface we got in ModuleCreateDBSession()
	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DBProp : public CDataSourceObject
{

protected:
	// @cmember If the variation passed
	BOOL			m_fSucceed;
	// @cmember Interface pointer
	IDBInfo *		m_pIDBInfo;
	// @cmember Count of DBLITERAL objects
	ULONG			m_cLiterals;
	// @cmember Array of DBLITERAL objects
	DBLITERAL *		m_rgLiterals;
	// @cmember Count of DBLITERALINFO objects
	ULONG 			m_cLiteralInfo;
	// @cmember Array of DBLITERALINFO objects
	DBLITERALINFO *	m_rgLiteralInfo;
	// @cmember Character Buffer for GetLiteralInfo
	WCHAR *			m_pCharBuffer;
	// @cmember Buffer of keywords
	LPWSTR			m_pwszKeywords;
	
	// @cmember Constructor
	DBProp(LPWSTR wszTestCaseName): CDataSourceObject(wszTestCaseName)
	{
		// Initialize the members
		m_cLiterals		= 0;
		m_cLiteralInfo	= 0;

		m_pIDBInfo		= NULL;
		m_rgLiterals	= NULL;
 		m_rgLiteralInfo	= NULL;
  		m_pCharBuffer	= NULL;
		m_pwszKeywords	= NULL;
		
		m_hr			= E_FAIL;
		m_fSucceed		= FALSE;
	};

	// @cmember Destructor
	virtual ~DBProp(){};

	// @cmember Test Case Initialization
	BOOL Init();

	// @cmember Test Case Termination
	BOOL Terminate();
	
	// @cember Test Variation Initialization
	BOOL InitVar();
	
	// @cmember Test Variation Termination
	BOOL TermVar();
	
	// @cmember Verify results of GetKeywords
	BOOL KeywordsVerify(HRESULT hr);
	
	// @cmember Verify results of GetLiteralInfo
	BOOL LiteralsVerify(HRESULT hr, BOOL fToScreen=FALSE);
	
	// @cmember Arrange DBLITERAL objects for GetLiteralInfo
	BOOL ArrangeLiterals(ARRANGELITERAL eArrangeLiteral);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init for Test Case
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::Init()
{
  	if(CDataSourceObject::Init())
	{	
		// IDBInitialize
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, 
							DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBInitialize));
		
		// Get interface pointer for test case
		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBInfo, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBInfo));

		// Release the Session Object
		SAFE_RELEASE(m_pThisTestModule->m_pIUnknown2);
		return TRUE;
	}

CLEANUP:
	
	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Terminate for Test Case
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::Terminate()
{
	// Release the objects
	SAFE_RELEASE(m_pIDBInfo);
	ReleaseDataSourceObject();

	return CDataSourceObject::Terminate();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init for Test Case Variation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::InitVar()
{
	// Initialize the members
	m_cLiterals		= 0;
	m_cLiteralInfo  = 0;

	m_rgLiterals	= NULL;
	m_rgLiteralInfo = NULL;
  	m_pCharBuffer	= NULL;
	m_pwszKeywords	= NULL;

	m_hr			= E_FAIL;
	m_fSucceed		= FALSE;

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Terminate for Test Case Variation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::TermVar()
{
	// Clean everything up	
	if( (m_rgLiteralInfo) && 
		(m_rgLiteralInfo != INVALID(DBLITERALINFO*)) )
		PROVIDER_FREE(m_rgLiteralInfo);
 	
	if( (m_pCharBuffer) && 
		(m_pCharBuffer != INVALID(OLECHAR*)) )
		PROVIDER_FREE(m_pCharBuffer);
	
	if( (m_pwszKeywords) && 
		(m_pwszKeywords != INVALID(OLECHAR*)) )
		PROVIDER_FREE(m_pwszKeywords);

	PROVIDER_FREE(m_rgLiterals);
	
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Keywords)
//--------------------------------------------------------------------
// @class testing keywords
//
class Keywords : public DBProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keywords,DBProp);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK
	int Variation_1();
	// @cmember E_INVALIDARG
	int Variation_2();
	// @cmember E_UNEXPECTED
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Keywords)
#define THE_CLASS Keywords
BEG_TEST_CASE(Keywords, DBProp, L"testing keywords")
	TEST_VARIATION(1, 		L"S_OK")
	TEST_VARIATION(2, 		L"E_INVALIDARG")
	TEST_VARIATION(3, 		L"E_UNEXPECTED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LiteralInfo)
//--------------------------------------------------------------------
// @class testing LiteralInfo
//
class LiteralInfo : public DBProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LiteralInfo,DBProp);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - cLiterals == 0
	int Variation_1();
	// @cmember S_OK - cLiterals == 1
	int Variation_2();
	// @cmember S_OK - cLiterals == ALL Literals
	int Variation_3();
	// @cmember E_INVALIDARG - cLiterals != 0, and rgLiterals == NULL
	int Variation_4();
	// @cmember E_INVALIDARG - pcLiteralInfo was a null pointer
	int Variation_5();
	// @cmember E_INVALIDARG - prgLiteralInfo was a null pointer
	int Variation_6();
	// @cmember E_INVALIDARG - pCharBuffer was a null pointer
	int Variation_7();
	// @cmember S_OK - Half of valid literals
	int Variation_8();
	// @cmember S_OK - Reverse of all valid literals
	int Variation_9();
	// @cmember S_OK - rgLiterals contained at least 1 unsupported or invalid literal with cLiterals = 0
	int Variation_10();
	// @cmember S_OK - all literals invalid or unsupported with cLiterals = 0
	int Variation_11();
	// @cmember DB_S_ERRORSOCCURRED - rgLiterals contained at least 1 unsupported or invalid literal
	int Variation_12();
	// @cmember DB_E_ERRORSOCCURRED - all literals invalid or unsupported
	int Variation_13();
	// @cmember E_UNEXPECTED - Uninitialized
	int Variation_14();
	// @cmember S_OK - 2.x Provider has both QUOTE_PREFIX and QUOTE_SUFFIX
	int Variation_15();
	// @cmember S_OK - Check for both CATALOG_NAME and CATALOG_SEPARATOR
	int Variation_16();
	// @cmember S_OK - 2.x Provider has both SCHEMA_NAME and SCHEMA_SEPARATOR
	int Variation_17();
	// @cmember S_OK - cLiterals == ALL Literals multiple times
	int Variation_18();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LiteralInfo)
#define THE_CLASS LiteralInfo
BEG_TEST_CASE(LiteralInfo, DBProp, L"testing LiteralInfo")
	TEST_VARIATION(1, 		L"S_OK - cLiterals == 0")
	TEST_VARIATION(2, 		L"S_OK - cLiterals == 1")
	TEST_VARIATION(3, 		L"S_OK - cLiterals == ALL Literals")
	TEST_VARIATION(4, 		L"E_INVALIDARG - cLiterals != 0, and rgLiterals == NULL")
	TEST_VARIATION(5, 		L"E_INVALIDARG - pcLiteralInfo was a null pointer")
	TEST_VARIATION(6, 		L"E_INVALIDARG - prgLiteralInfo was a null pointer")
	TEST_VARIATION(7, 		L"E_INVALIDARG - pCharBuffer was a null pointer")
	TEST_VARIATION(8, 		L"S_OK - Half of valid literals")
	TEST_VARIATION(9, 		L"S_OK - Reverse of all valid literals")
	TEST_VARIATION(10, 		L"S_OK - rgLiterals contained at least 1 unsupported or invalid literal with cLiterals = 0")
	TEST_VARIATION(11, 		L"S_OK - all literals invalid or unsupported with cLiterals = 0")
	TEST_VARIATION(12, 		L"DB_S_ERRORSOCCURRED - rgLiterals contained at least 1 unsupported or invalid literal")
	TEST_VARIATION(13, 		L"DB_E_ERRORSOCCURRED - all literals invalid or unsupported")
	TEST_VARIATION(14, 		L"E_UNEXPECTED - Uninitialized")
	TEST_VARIATION(15, 		L"S_OK - 2.x Provider has both QUOTE_PREFIX and QUOTE_SUFFIX")
	TEST_VARIATION(16, 		L"S_OK - Check for both CATALOG_NAME and CATALOG_SEPARATOR")
	TEST_VARIATION(17, 		L"S_OK - 2.x Provider has both SCHEMA_NAME and SCHEMA_SEPARATOR")
	TEST_VARIATION(18, 		L"S_OK - cLiterals == ALL Literals multiple times")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCZOMBIE_Zombie)
//--------------------------------------------------------------------
// @class Induce zombie states
//
class TCZOMBIE_Zombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZOMBIE_Zombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember IDBInfo object
	IDBInfo *		m_pIDBInfo;
	
	// @cmember TestTxn
	int TestTxn(ETXN eTxn, BOOL fRetaining);
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Commit IDBInfo with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit IDBInfo with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Abort IDBInfo with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Abort IDBInfo with fRetaining=FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCZOMBIE_Zombie)
#define THE_CLASS TCZOMBIE_Zombie
BEG_TEST_CASE(TCZOMBIE_Zombie, CTransaction, L"Induce zombie states")
	TEST_VARIATION(1, 		L"S_OK - Commit IDBInfo with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit IDBInfo with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Abort IDBInfo with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Abort IDBInfo with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public DBProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,DBProp);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid GetKeywords call with previous error object existing.
	int Variation_1();
	// @cmember Valid GetLiteralInfo call with previous error object existing.
	int Variation_2();
	// @cmember E_INVALIDARG GetKeywords call with previous error object existing.
	int Variation_3();
	// @cmember E_INVALIDARG GetLiteralInfo call with previous error object existing.
	int Variation_4();
	// @cmember DB_S_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.
	int Variation_5();
	// @cmember DB_E_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, DBProp, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid GetKeywords call with previous error object existing.")
	TEST_VARIATION(2, 		L"Valid GetLiteralInfo call with previous error object existing.")
	TEST_VARIATION(3, 		L"E_INVALIDARG GetKeywords call with previous error object existing.")
	TEST_VARIATION(4, 		L"E_INVALIDARG GetLiteralInfo call with previous error object existing.")
	TEST_VARIATION(5, 		L"DB_S_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.")
	TEST_VARIATION(6, 		L"DB_E_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Keywords)
	TEST_CASE(2, LiteralInfo)
	TEST_CASE(3, TCZOMBIE_Zombie)
	TEST_CASE(4, TCExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(Keywords)
//*-----------------------------------------------------------------------
//| Test Case:		Keywords - testing keywords
//|	Created:		06/03/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keywords::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	return (DBProp::Init());
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keywords::Variation_1()
{
	// Initialize
	TBEGIN;
	INIT;

	// Call with a valid pwszKeywords pointer
	TESTC_(m_hr=m_pIDBInfo->GetKeywords(&m_pwszKeywords), S_OK);
	TESTC(KeywordsVerify(m_hr));
	
CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keywords::Variation_2()
{
	// Initialize
	TBEGIN;
	INIT;

	// Call with a NULL pwszKeywords pointer
	TESTC_(m_hr=m_pIDBInfo->GetKeywords(NULL), E_INVALIDARG);
	TESTC(KeywordsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keywords::Variation_3()
{
	// Initialize
	TBEGIN;
	INIT;
	
	// Dirty the output params
	m_pwszKeywords = INVALID(OLECHAR*);

	// Uninitialize the DSO
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);
	
	// Call with a valid pwszKeywords pointer
	TESTC_(m_hr=m_pIDBInfo->GetKeywords(&m_pwszKeywords), E_UNEXPECTED);
	TESTC(KeywordsVerify(m_hr));
	
	// Reinitialize the DSO
	InitializeDSO(REINITIALIZE_YES);
		
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
BOOL Keywords::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DBProp::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LiteralInfo)
//*-----------------------------------------------------------------------
//| Test Case:		LiteralInfo - testing LiteralInfo
//|	Created:		06/03/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LiteralInfo::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	return (DBProp::Init());
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - cLiterals == 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_1()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check the first valid Literal case
	ArrangeLiterals(ZERO);

	// Check the 0 cLiteral case
	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr, TRUE));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - cLiterals == 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_2()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check the first valid Literal case
	ArrangeLiterals(ONE);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - cLiterals == ALL Literals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_3()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case
	ArrangeLiterals(ALLSUPPORTED);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - cLiterals != 0, and rgLiterals == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_4()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case with a NULL rgLiterals
	ArrangeLiterals(ALLSUPPORTED);

	// Dirty the output params
	m_cLiteralInfo =  INVALID(ULONG);
	m_rgLiteralInfo = INVALID(DBLITERALINFO*);
	m_pCharBuffer =   INVALID(OLECHAR*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(1, NULL,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), E_INVALIDARG);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - pcLiteralInfo was a null pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_5()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case with a NULL pcLiteralInfo
	ArrangeLiterals(ALLSUPPORTED);

	// Dirty the output params
	m_rgLiteralInfo = INVALID(DBLITERALINFO*);
	m_pCharBuffer =   INVALID(OLECHAR*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
					NULL, &m_rgLiteralInfo, &m_pCharBuffer), E_INVALIDARG);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - prgLiteralInfo was a null pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_6()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case with a NULL prgLiteralInfo
	ArrangeLiterals(ALLSUPPORTED);

	// Dirty the output params
	m_cLiteralInfo = INVALID(ULONG);
	m_pCharBuffer =  INVALID(OLECHAR*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
					&m_cLiteralInfo, NULL, &m_pCharBuffer), E_INVALIDARG);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - pCharBuffer was a null pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_7()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case with a NULL pCharBuffer
	ArrangeLiterals(ALLSUPPORTED);

	// Dirty the output params
	m_cLiteralInfo =  INVALID(ULONG);
	m_rgLiteralInfo = INVALID(DBLITERALINFO*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
					&m_cLiteralInfo, &m_rgLiteralInfo, NULL), E_INVALIDARG);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Half of valid literals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_8()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check the first half of the valid Literals
	ArrangeLiterals(HALF);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Reverse of all valid literals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_9()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all valid Literals case in reverse order
	ArrangeLiterals(ALLREVERSE);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK - rgLiterals contained at least 1 unsupported or invalid literal with cLiterals = 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_10()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check first invalid Literal case
	ArrangeLiterals(INVALID1);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(0, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK - all literals invalid or unsupported with cLiterals = 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_11()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all invalid Literals case
	ArrangeLiterals(INVALIDALL);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(0, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - rgLiterals contained at least 1 unsupported or invalid literal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_12()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check first invalid Literal case
	ArrangeLiterals(INVALID1);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals, &m_cLiteralInfo,
			&m_rgLiteralInfo, &m_pCharBuffer), (m_cLiterals ? DB_S_ERRORSOCCURRED : S_OK));
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - all literals invalid or unsupported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_13()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check all invalid Literals case
	ArrangeLiterals(INVALIDALL);

	// Dirty the output params
	m_pCharBuffer = INVALID(OLECHAR*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), DB_E_ERRORSOCCURRED);
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc E_UNEXPECTED - Uninitialized
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_14()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check before Initializing the Provider
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);

	// Dirty the output params
	m_cLiteralInfo =  INVALID(ULONG);
	m_rgLiteralInfo = INVALID(DBLITERALINFO*);
	m_pCharBuffer =   INVALID(OLECHAR*);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(0, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), E_UNEXPECTED);
	TESTC(LiteralsVerify(m_hr));

	// Reinitialize the DSO
	InitializeDSO(REINITIALIZE_YES);

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK - 2.x Provider has both QUOTE_PREFIX and QUOTE_SUFFIX
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_15()
{
	// Initialize
	TBEGIN;
	INIT;

	// Allocate memory and set QUOTE_PREFIX and QUOTE_SUFFIX
	SAFE_ALLOC(m_rgLiterals, DBLITERAL, 2);
	
	m_rgLiterals[0] = DBLITERAL_QUOTE_PREFIX;
	m_rgLiterals[1] = DBLITERAL_QUOTE_SUFFIX;

	TEST2C_(m_hr=m_pIDBInfo->GetLiteralInfo(2, m_rgLiterals,
			&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer),S_OK, DB_E_ERRORSOCCURRED);

	// Compare DBLITERAL_QUOTE_PREFIX and DBLITERAL_QUOTE_SUFFIX
	TESTC(m_cLiteralInfo == 2);
	TESTC(m_rgLiteralInfo[0].fSupported == m_rgLiteralInfo[1].fSupported);

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Check for both CATALOG_NAME and CATALOG_SEPARATOR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_16()
{
	// Initialize
	TBEGIN;
	INIT;

	// Allocate memory and set CATALOG_NAME and CATALOG_SEPARATOR
	SAFE_ALLOC(m_rgLiterals, DBLITERAL, 2);
	
	m_rgLiterals[0] = DBLITERAL_CATALOG_NAME;
	m_rgLiterals[1] = DBLITERAL_CATALOG_SEPARATOR;

	TEST2C_(m_hr=m_pIDBInfo->GetLiteralInfo(2, m_rgLiterals,
			&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer),S_OK, DB_E_ERRORSOCCURRED);

	// Compare DBLITERAL_CATALOG_NAME and DBLITERAL_CATALOG_SEPARATOR
	TESTC(m_cLiteralInfo == 2);
	TESTC(m_rgLiteralInfo[0].fSupported == m_rgLiteralInfo[1].fSupported);

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK - 2.x Provider has both SCHEMA_NAME and SCHEMA_SEPARATOR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_17()
{
	// Initialize
	TBEGIN;
	INIT;

	// Allocate memory and set SCHEMA_NAME and SCHEMA_SEPARATOR
	SAFE_ALLOC(m_rgLiterals, DBLITERAL, 2);
	
	m_rgLiterals[0] = DBLITERAL_SCHEMA_NAME;
	m_rgLiterals[1] = DBLITERAL_SCHEMA_SEPARATOR;

	TEST2C_(m_hr=m_pIDBInfo->GetLiteralInfo(2, m_rgLiterals,
			&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer),S_OK, DB_E_ERRORSOCCURRED);

	// Compare DBLITERAL_SCHEMA_NAME and DBLITERAL_SCHEMA_SEPARATOR
	TESTC(m_cLiteralInfo == 2);
	TESTC(m_rgLiteralInfo[0].fSupported == m_rgLiteralInfo[1].fSupported);

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK - cLiterals == ALL Literals multiple times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralInfo::Variation_18()
{
	TBEGIN;

	// Initialize
	ULONG ulIndex = 0;
	INIT;

	// Check all valid Literals case
	ArrangeLiterals(ALLSUPPORTED);

	// Allocate memory and set Literals
	#define MAX_LITERALS 2050
	SAFE_REALLOC(m_rgLiterals, DBLITERAL, m_cLiterals*MAX_LITERALS);

	for(ulIndex=0; ulIndex < MAX_LITERALS; ulIndex++)
		memcpy(&m_rgLiterals[m_cLiterals*ulIndex],m_rgLiterals,(m_cLiterals*sizeof(DBLITERAL)));

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo((m_cLiterals * MAX_LITERALS), m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	TESTC(LiteralsVerify(m_hr));

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
BOOL LiteralInfo::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DBProp::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZOMBIE_Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZOMBIE_Zombie - Induce zombie states
//|	Created:		02/02/96
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
	m_pIDBInfo = NULL;
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		// IDBInfo
		TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBInfo, 
								DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBInfo));
		
		// Register Interface with Zombie
		TESTC(RegisterInterface(DATASOURCE_INTERFACE, IID_IDBInfo,0,NULL));
		return TRUE;
	}

	// Check to see if ITransaction is supported
    if( !m_pITransactionLocal )
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

CLEANUP:

	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInfo with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_1()
{
	// S_OK - Commit IDBInfo with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInfo with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_2()
{
	// S_OK - Commit IDBInfo with fRetaining=FALSE
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInfo with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_3()
{
	// S_OK - Abort IDBInfo with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInfo with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::Variation_4()
{
	// S_OK - Abort IDBInfo with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
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
	SAFE_RELEASE(m_pIDBInfo);

	return(CTransaction::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		08/12/96
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
	return (DBProp::Init());
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid GetKeywords call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	// Initialize
	TBEGIN;
	INIT;

	// Cause an Error
 	m_pExtError->CauseError();

	TESTC_(m_hr=m_pIDBInfo->GetKeywords(&m_pwszKeywords),S_OK);

	// Do extended check following GetKeywords
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));
	TESTC(KeywordsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid GetLiteralInfo call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	// Initialize
	TBEGIN;
	INIT;

	// Cause an Error
 	m_pExtError->CauseError();
	
	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(0, m_rgLiterals,
					&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), S_OK);
	
	// Do extended check following GetLiteralInfo
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetKeywords call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	// Initialize
	TBEGIN;
	INIT;

	// Cause an Error
	m_pExtError->CauseError();

	TESTC_(m_hr=m_pIDBInfo->GetKeywords(NULL),E_INVALIDARG);
	
	// Do extended check following GetKeywords
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));
	TESTC(KeywordsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetLiteralInfo call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	// Initialize
	TBEGIN;
	INIT;

	ArrangeLiterals(ALLSUPPORTED);

	// Cause an Error
	m_pExtError->CauseError();

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(1, NULL,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), E_INVALIDARG);

	// Do extended check following GetLiteralInfo
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
	// Initialize
	TBEGIN;
	INIT;

	// Check first invalid Literal case
	ArrangeLiterals(INVALID1);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals, &m_cLiteralInfo,
			&m_rgLiteralInfo, &m_pCharBuffer), (m_cLiterals ? DB_S_ERRORSOCCURRED : S_OK));

	// Do extended check following GetLiteralInfo
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));
	TESTC(LiteralsVerify(m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
//}}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED GetLiteralInfo call with no previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	// Initialize
	TBEGIN;
	INIT;

	ArrangeLiterals(INVALIDALL);

	TESTC_(m_hr=m_pIDBInfo->GetLiteralInfo(m_cLiterals, m_rgLiterals,
				&m_cLiteralInfo, &m_rgLiteralInfo, &m_pCharBuffer), DB_E_ERRORSOCCURRED);

	// Do extended check following GetLiteralInfo
	TESTC(XCHECK(m_pIDBInfo, IID_IDBInfo, m_hr));

CLEANUP:
	
	TERM;
	TRETURN;
}
//}}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	return (DBProp::Terminate());

}	// }}
// }}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Verify that Keywords returned what it should have
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::KeywordsVerify(HRESULT hr)
{
	//============================================================================
	//  KEYWORDS Constants from V2.0
	//============================================================================
	const LPWSTR rgpwszKeywords[] = {
		L"ABSOLUTE",L"ACTION",L"ADD",L"ALL",L"ALLOCATE",L"ALTER",L"AND",L"ANY",L"ARE",L"AS",L"ASC",
		L"ASSERTION",L"AT",L"AUTHORIZATION",L"AVG",L"BEGIN",L"BETWEEN",L"BIT",L"BIT_LENGTH",L"BOTH",
		L"BY",L"CASCADE",L"CASCADED",L"CASE",L"CAST",L"CATALOG",L"CHAR",L"CHAR_LENGTH",L"CHARACTER",
		L"CHARACTER_LENGTH",L"CHECK",L"CLOSE",L"COALESCE",L"COLLATE",L"COLLATION",L"COLUMN",
		L"COMMIT",L"CONNECT",L"CONNECTION",L"CONSTRAINT",L"CONSTRAINTS",L"CONTINUE",L"CONVERT",
		L"CORRESPONDING",L"COUNT",L"CREATE",L"CROSS",L"CURRENT",L"CURRENT_DATE",L"CURRENT_TIME",
		L"CURRENT_TIMESTAMP",L"CURRENT_USER",L"CURSOR",L"DATE",L"DAY",L"DEALLOCATE",L"DEC",
		L"DECIMAL",L"DECLARE",L"DEFAULT",L"DEFERRABLE",L"DEFERRED",L"DELETE",L"DESC",L"DESCRIBE",
		L"DESCRIPTOR",L"DIAGNOSTICS",L"DISCONNECT",L"DISTINCT",L"DISTINCTROW",L"DOMAIN",L"DOUBLE",
		L"DROP",L"ELSE",L"END",L"END-EXEC",L"ESCAPE",L"EXCEPT",L"EXCEPTION",L"EXEC",L"EXECUTE",
		L"EXISTS",L"EXTERNAL",L"EXTRACT",L"FALSE",L"FETCH",L"FIRST",L"FLOAT",L"FOR",L"FOREIGN",
		L"FOUND",L"FROM",L"FULL",L"GET",L"GLOBAL",L"GO",L"GOTO",L"GRANT",L"GROUP",L"HAVING",L"HOUR",
		L"IDENTITY",L"IMMEDIATE",L"IN",L"INDICATOR",L"INITIALLY",L"INNER",L"INPUT",L"INSENSITIVE",
		L"INSERT",L"INT",L"INTEGER",L"INTERSECT",L"INTERVAL",L"INTO",L"IS",L"ISOLATION",L"JOIN",
		L"KEY",L"LANGUAGE",L"LAST",L"LEADING",L"LEFT",L"LEVEL",L"LIKE",L"LOCAL",L"LOWER",L"MATCH",L"MAX",
		L"MIN",L"MINUTE",L"MODULE",L"MONTH",L"NAMES",L"NATIONAL",L"NATURAL",L"NCHAR",L"NEXT",L"NO",
		L"NOT",L"NULL",L"NULLIF",L"NUMERIC",L"OCTET_LENGTH",L"OF",L"ON",L"ONLY",L"OPEN",L"OPTION",
		L"OR",L"ORDER",L"OUTER",L"OUTPUT",L"OVERLAPS",L"PARTIAL",L"POSITION",L"PRECISION",L"PREPARE",
		L"PRESERVE",L"PRIMARY",L"PRIOR",L"PRIVILEGES",L"PROCEDURE",L"PUBLIC",L"READ",L"REAL",
		L"REFERENCES",L"RELATIVE",L"RESTRICT",L"REVOKE",L"RIGHT",L"ROLLBACK",L"ROWS",L"SCHEMA",
		L"SCROLL",L"SECOND",L"SECTION",L"SELECT",L"SESSION",L"SESSION_USER",L"SET",L"SIZE",L"SMALLINT",
		L"SOME",L"SQL",L"SQLCODE",L"SQLERROR",L"SQLSTATE",L"SUBSTRING",L"SUM",L"SYSTEM_USER",L"TABLE",
		L"TEMPORARY",L"THEN",L"TIME",L"TIMESTAMP",L"TIMEZONE_HOUR",L"TIMEZONE_MINUTE",L"TO",
		L"TRAILING",L"TRANSACTION",L"TRANSLATE",L"TRANSLATION",L"TRIGGER",L"TRIM",L"TRUE",L"UNION",
		L"UNIQUE",L"UNKNOWN",L"UPDATE",L"UPPER",L"USAGE",L"USER",L"USING",L"VALUE",L"VALUES",L"VARCHAR",
		L"VARYING",L"VIEW",L"WHEN",L"WHENEVER",L"WHERE",L"WITH",L"WORK",L"WRITE",L"YEAR",L"ZONE",
	};

	ULONG ulErrors = 0;
	ULONG ulSpaces = 0;
	ULONG ulExtra  = 0;

	// If HRESULT is a failure code
	if((FAILED(hr)) && (!m_pwszKeywords))
		return TRUE;

	// If HRESULT is a success code
	if(SUCCEEDED(hr))
	{
		// It is legal to return a NULL on S_OK
		if(!m_pwszKeywords) {
			odtLog << L"The Provider returned no Keywords." <<ENDL;
			return TRUE;
		}
		
		// Check the number of commas
		for(ULONG ulIndex=0; *(m_pwszKeywords+ulIndex); ulIndex++)
		{
			if(*(m_pwszKeywords+ulIndex) == ',')
				ulExtra++;
		}
		
		// Find the first keyword and loop thru all the Keywords
		WCHAR * wtoken = wcstok(m_pwszKeywords, L",");
		
		// It is illegal to return a empty string on S_OK
		if(!wtoken) {
			odtLog << L"The Provider should of returned a NULL instead of <EMPTY> for pwszKeywords." <<ENDL;
			return FALSE;
		}

		// Loop thru all the Keywords
		while(wtoken)
		{
			// Static Keywords should not be in the list
			for(ULONG i=0; i < (sizeof(rgpwszKeywords)/sizeof(rgpwszKeywords[0])); i++)
			{
				if(!(_wcsicmp(rgpwszKeywords[i], wtoken))) {
					odtLog<<ENDL <<rgpwszKeywords[i] 
						<<L" should not be returned in the list." <<ENDL;
					ulErrors++;
				} 
			}

			// Check for leading and trailing spaces
			if( (*wtoken == L' ') || (*(wtoken + (wcslen(wtoken)-1)) == L' ') )
				ulSpaces++;

			// Print the current and get the next keyword
			odtLog <<wtoken;
			wtoken = wcstok(NULL, L",");
			wtoken ? odtLog <<L"," : odtLog <<ENDL;

			// Subtract one if another keyword
			if(wtoken)
				ulExtra--;
		}

		// Check for leading or trailing spaces
		if(ulSpaces)
			odtLog <<L"The provider had leading or trailing spaces on the keywords." <<ENDL;

		// Check for leading or trailing spaces
		if(ulExtra)
			odtLog <<L"The provider had an extra comma in the keywords list." <<ENDL;

		// If no Keywords were in the list and
		// no leading or trailing spaces return TRUE
		if(!ulErrors && !ulSpaces && !ulExtra)
			return TRUE;
	}
	
	return FALSE;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Arrange Literals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::ArrangeLiterals(ARRANGELITERAL eArrangeLiteral)
{
	ULONG ulIndex = 0;

	// Get all valid Literals
	if(FAILED(m_pIDBInfo->GetLiteralInfo(0, NULL, 
							&m_cLiterals, &m_rgLiteralInfo, &m_pCharBuffer)))
		return FALSE;

	// Check to see if we got Literals 
	if((!m_cLiterals) && (eArrangeLiteral != INVALIDALL)) {
		odtLog << L"Provider did not return any supported Literals." <<ENDL;
		return TRUE;
	}

	// Create the default returned by GetLiterals
	m_rgLiterals = (DBLITERAL *)PROVIDER_ALLOC(sizeof(DBLITERAL) * max(m_cLiterals,MAX_DBLITERALINFO));
	
	for(ulIndex=0; ulIndex < m_cLiterals; ulIndex++)
		m_rgLiterals[ulIndex] = m_rgLiteralInfo[ulIndex].lt;

	// Allocate the memory for the Literal
	switch(eArrangeLiteral)
	{
		// No Literals
		case ZERO:
			m_cLiterals = 0;
			break;

		// First supported Literal
		case ONE:
			m_cLiterals = 1;
			break;

		// First half of the supported Literals
		case HALF:
			m_cLiterals = (m_cLiterals / 2);
			break;

		// ALL supported Literals
		case ALLSUPPORTED:
			break;

		// ALL Literals in the V2.0 ENUM (DBLITERAL_INVALID -> DBLITERAL_QUOTE_SUFFIX)
		case INVALID1:
			m_cLiterals = MAX_DBLITERALINFO;
			for(ulIndex=DBLITERAL_INVALID; ulIndex <= DBLITERAL_QUOTE_SUFFIX; ulIndex++)
				m_rgLiterals[ulIndex] = ulIndex;
			break;

		// ALL unsupported Literals
		case INVALIDALL:
			m_cLiterals = MAX_DBLITERALINFO;
			for(ulIndex=0; ulIndex < MAX_DBLITERALINFO; ulIndex++)
				m_rgLiterals[ulIndex] = DBLITERAL_INVALID;
			break;

		// ALL supported Literals in reverse order
		case ALLREVERSE:
			for(ulIndex=0; ulIndex < m_cLiterals; ulIndex++)
				m_rgLiterals[ulIndex] = m_rgLiteralInfo[(m_cLiterals-ulIndex)-1].lt;
			break;

		default:
			odtLog << L"ArrangeLiterals used incorrectly" <<ENDL;
			break;
	}

	// Should never hit this since we asked for all LiteralInfos
	PROVIDER_FREE(m_rgLiteralInfo);
	PROVIDER_FREE(m_pCharBuffer);
	
 	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Verify that GetLiteralInfo returned what it should have
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL DBProp::LiteralsVerify(HRESULT hr, BOOL fToScreen)
{
	//============================================================================
	//  LITERALS Constants from V2.0
	//============================================================================
	const LPWSTR rgpwszLiterals[] = {
		L"DBLITERAL_INVALID",L"DBLITERAL_BINARY_LITERAL",L"DBLITERAL_CATALOG_NAME",
		L"DBLITERAL_CATALOG_SEPARATOR",L"DBLITERAL_CHAR_LITERAL",L"DBLITERAL_COLUMN_ALIAS",
		L"DBLITERAL_COLUMN_NAME",L"DBLITERAL_CORRELATION_NAME",L"DBLITERAL_CURSOR_NAME",
		L"DBLITERAL_ESCAPE_PERCENT_PREFIX",L"DBLITERAL_ESCAPE_UNDERSCORE_PREFIX",
		L"DBLITERAL_INDEX_NAME",L"DBLITERAL_LIKE_PERCENT",L"DBLITERAL_LIKE_UNDERSCORE",
		L"DBLITERAL_PROCEDURE_NAME",L"DBLITERAL_QUOTE_PREFIX",L"DBLITERAL_SCHEMA_NAME",
		L"DBLITERAL_TABLE_NAME",L"DBLITERAL_TEXT_COMMAND",L"DBLITERAL_USER_NAME",
		L"DBLITERAL_VIEW_NAME",L"DBLITERAL_CUBE_NAME",L"DBLITERAL_DIMENSION_NAME",
		L"DBLITERAL_HIERARCHY_NAME",L"DBLITERAL_LEVEL_NAME",L"DBLITERAL_MEMBER_NAME",
		L"DBLITERAL_PROPERTY_NAME",L"DBLITERAL_SCHEMA_SEPARATOR",L"DBLITERAL_QUOTE_SUFFIX",
		L"DBLITERAL_ESCAPE_PERCENT_SUFFIX",L"DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX"
	};

	BOOL  fSucceed	    = FALSE;
	ULONG errors	    = 0;
	ULONG ulSupported   = 0;
	
	// Check the pointers to see if they are updated
	if( (m_cLiteralInfo  == INVALID(ULONG)) || 
		(m_rgLiteralInfo == INVALID(DBLITERALINFO*)) || 
		(m_pCharBuffer   == INVALID(OLECHAR*)) )
		return FALSE;

	// If HRESULT is a E_CODE other than DB_E_ERRORSOCCURRED
	// or HRESULT is S_OK and 0 Literals where supported
	if( (((FAILED(hr)) && (hr != DB_E_ERRORSOCCURRED)) || (hr == S_OK)) && 
		(!m_cLiteralInfo) && (!m_rgLiteralInfo) && (!m_pCharBuffer) )
		return TRUE;

	// If the count of literals is > 0
	if(m_cLiteralInfo)
	{
		// Count the supported Literals
		for(ULONG i=0; i < m_cLiteralInfo; i++)
			if(m_rgLiteralInfo[i].fSupported)
				ulSupported++;

		// All supported
		if(hr == S_OK)
			COMPARE(m_cLiteralInfo, ulSupported);
		
		// Atleast 1 unsupported
		if(hr == DB_S_ERRORSOCCURRED)
			COMPARE(((m_cLiteralInfo > ulSupported) && ulSupported), TRUE);
		
		// Atleast 1 unsupported
		if(hr == DB_E_ERRORSOCCURRED)
			COMPARE((!ulSupported && m_rgLiteralInfo), TRUE);

		// Check the pCharBuffer, it should be NULL on ERROR
		if(SUCCEEDED(hr))
			COMPARE(!m_pCharBuffer, NULL);
		else
			COMPARE(m_pCharBuffer, NULL);

		// Check the Data returned
		for(ULONG ulIndex=0; ulIndex < m_cLiteralInfo; ulIndex++)
		{
			// Check for unsupported Literals
			if(!(m_rgLiteralInfo[ulIndex].fSupported))
			{
				// Everything should be NULL or 0
				COMPARE(((m_rgLiteralInfo[ulIndex].pwszLiteralValue) ||
						 (m_rgLiteralInfo[ulIndex].pwszInvalidChars) ||
						 (m_rgLiteralInfo[ulIndex].pwszInvalidStartingChars) ||
						 (m_rgLiteralInfo[ulIndex].cchMaxLen)), FALSE);

				continue;
			}

			// These are the only 8 that should modify pwszLiteralValue and cchMaxLen, 
			// and not modify pwszInvalidChar and pwszInvalidStartingChar.
			if( (m_rgLiteralInfo[ulIndex].lt == DBLITERAL_CATALOG_SEPARATOR)		||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_PERCENT_PREFIX)    ||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_PERCENT_SUFFIX)    ||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_UNDERSCORE_PREFIX)	||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX)	||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_LIKE_PERCENT)				||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_LIKE_UNDERSCORE)			||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_QUOTE_PREFIX)				||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_QUOTE_SUFFIX)				||
				(m_rgLiteralInfo[ulIndex].lt == DBLITERAL_SCHEMA_SEPARATOR) )
			{
				// Check pwszLiteralValue exceptions 
				if(!m_rgLiteralInfo[ulIndex].pwszLiteralValue)
				{
					odtLog << L"ERROR: pwszLiteralValue exceptions:  ";
					odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
					odtLog << L" should not be NULL." <<ENDL;
					errors++;
				}

				// Check pwszInvalidChar exceptions
				if(m_rgLiteralInfo[ulIndex].pwszInvalidChars)
				{
					odtLog << L"ERROR: pwszInvalidChars exceptions:  ";
					odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
					odtLog << L" should be NULL." <<ENDL;
					errors++;
				}

				// Check pwszInvalidStartingChar exceptions
				if(m_rgLiteralInfo[ulIndex].pwszInvalidStartingChars)
				{
					odtLog << L"ERROR: pwszInvalidStartingChars exceptions:  ";
					odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
					odtLog << L" should be NULL." <<ENDL;
					errors++;
				}

				// Check cchMaxLen exceptions
				if( (m_rgLiteralInfo[ulIndex].pwszLiteralValue) && 
					(m_rgLiteralInfo[ulIndex].cchMaxLen != wcslen(m_rgLiteralInfo[ulIndex].pwszLiteralValue)) )
				{
					odtLog << L"ERROR: cchMaxLen exceptions:  ";
					odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
					odtLog << L" expect " << (ULONG) wcslen(m_rgLiteralInfo[ulIndex].pwszLiteralValue)
						<< 	L" but returned " << m_rgLiteralInfo[ulIndex].cchMaxLen 
						<< 	L" charcters." << ENDL;
					errors++;
				}
			}
			else if(m_rgLiteralInfo[ulIndex].pwszLiteralValue)
			{
				odtLog << L"ERROR: pwszLiteralValue exceptions:  ";
				odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
				odtLog << L" should be NULL." <<ENDL;
				errors++;
			}

			// Nothing should return 0, should be ~0
			if(!m_rgLiteralInfo[ulIndex].cchMaxLen && fToScreen)
			{
				odtLog << L"ERROR: cchMaxLen exceptions:  ";
				odtLog << rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt];
				odtLog << L" should not be 0, should be ~0." <<ENDL;
				errors++;
			}

			// Print the data to the screen
			if(fToScreen)
			{
				odtLog << L"[" <<ulIndex << L"]:DBLITERAL = "
						<< rgpwszLiterals[m_rgLiteralInfo[ulIndex].lt] <<ENDL; 
				
				odtLog << L"[" <<ulIndex << L"]:pwszLiteralValue = "; 
				if(m_rgLiteralInfo[ulIndex].pwszLiteralValue)
					odtLog << m_rgLiteralInfo[ulIndex].pwszLiteralValue <<ENDL;
				else
					odtLog << L"<null>" <<ENDL;

				odtLog << L"[" <<ulIndex << L"]:pwszInvalidChars = ";
				if(m_rgLiteralInfo[ulIndex].pwszInvalidChars)
					odtLog << m_rgLiteralInfo[ulIndex].pwszInvalidChars <<ENDL;
				else
					odtLog << L"<null>" <<ENDL;

				odtLog << L"[" << ulIndex << L"]:pwszInvalidStartingChars = ";
				if(m_rgLiteralInfo[ulIndex].pwszInvalidStartingChars)
					odtLog << m_rgLiteralInfo[ulIndex].pwszInvalidStartingChars <<ENDL;
				else
					odtLog << L"<null>" <<ENDL;

				odtLog << L"[" << ulIndex << L"]:fSupported = "
					<< m_rgLiteralInfo[ulIndex].fSupported <<ENDL; 
				odtLog << L"[" << ulIndex << L"]:cchMaxLen = "
					<< m_rgLiteralInfo[ulIndex].cchMaxLen <<ENDL; 
				odtLog << L"_________________________________________________" << ENDL;
			}
		}
		
		if(!errors)
			fSucceed = TRUE;
	}
		
	return fSucceed;
}


//--------------------------------------------------------------------
// @mfunc Test Zombie cases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZOMBIE_Zombie::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	TBEGIN;

	HRESULT		ExpectedHr		= E_UNEXPECTED;	// Expected HRESULT
	DBCOUNTITEM	cRowsObtained	= 0;			// Number of rows returned, should be 1
	HROW *		rghRows			= NULL;			// Array of Row Handles
	IDBInfo *	pIDBInfo		= NULL;			// IDBInfo object
	LPWSTR		pwszKeywords	= NULL;			// List of Keywords
	ULONG 		cLiteralInfo	= 0;			// Number of Literals
	WCHAR *		pCharBuffer		= NULL;			// Character buffer
	DBLITERALINFO *	rgLiteralInfo = NULL;		// Array of Literals

	// Retrieve an Interface pointer to IDBInfo within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIDBInfo));

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT) && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Commit or Abort the transaction, with retention as specified
	if( ((eTxn == ETXN_COMMIT) && (!GetCommit(fRetaining))) ||
	    ((eTxn == ETXN_ABORT)  && (!GetAbort(fRetaining))) )
		goto CLEANUP;

	// Test zombie
	TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	// Make sure that this Interface is unaffected
	TESTC_(m_pIDBInfo->GetKeywords(&pwszKeywords),S_OK);

	// Make sure that this Interface is unaffected
	TESTC_(m_pIDBInfo->GetLiteralInfo(0, NULL,
				&cLiteralInfo, &rgLiteralInfo, &pCharBuffer), S_OK);

CLEANUP:
	
	// Release the row handle on the 1st rowset
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Release the IDBInfo
	SAFE_RELEASE(pIDBInfo);
	
	PROVIDER_FREE(pwszKeywords);
 	PROVIDER_FREE(rgLiteralInfo);
	PROVIDER_FREE(pCharBuffer);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	TRETURN;
}
