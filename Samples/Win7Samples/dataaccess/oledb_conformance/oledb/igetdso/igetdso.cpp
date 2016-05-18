//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IGetDSO.cpp | This module tests the OLEDB IGetDataSource interface 
//

#include "MODStandard.hpp"		// Standard headers			
#include "IGetDSO.h"			// IGetDSO header
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xcde65320, 0xe011, 0x11cf, { 0xb0, 0x1b, 0x00, 0xa0, 0xc9, 0x0d, 0x80, 0x7a }};
DECLARE_MODULE_NAME("IGetDataSource");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IGetDataSource interface test");
DECLARE_MODULE_VERSION(838086926);
// TCW_WizardVersion(2)
// TCW_Automation(False)
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
    return CommonModuleInit(pThisTestModule, IID_IGetDataSource);
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


// {{ TCW_TEST_CASE_MAP(TCGetDSO)
//--------------------------------------------------------------------
// @class Testing IGetDataSource::GetDataSource
//
class TCGetDSO : public COpenRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	IGetDataSource* m_pIGetDSO;


public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetDSO,COpenRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	IGetDataSource* const pIGetDSO();
	
	//Wrapper arround IGetDataSource
	HRESULT GetDataSource(REFIID riid, IUnknown** ppIUnknown = NULL);
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Verify IGetDataSource is mandatory
	int Variation_1();
	// @cmember General - QI for IUnknown
	int Variation_2();
	// @cmember General - QI for IID_IGetDataSource
	int Variation_3();
	// @cmember General - QI for al other session interfaces (IOpenRowset
	int Variation_4();
	// @cmember General - Verify AddRef / Release
	int Variation_5();
	// @cmember Boundary - E_INVALIDARG
	int Variation_6();
	// @cmember Boundary - E_NOINTERFACE
	int Variation_7();
	// @cmember Parameters - IID_IUnknown
	int Variation_8();
	// @cmember Parameters - IID_IDBProperties
	int Variation_9();
	// @cmember Parameters - IID_IDBInitialize
	int Variation_10();
	// @cmember Parameters - IID_IDBCreateSession
	int Variation_11();
	// @cmember Parameters - IID_IPersistFile
	int Variation_12();
	// @cmember Parameters - IID_ISupportErrorInfo
	int Variation_13();
	// @cmember Parameters - IID_IGetDataSource
	int Variation_14();
	// @cmember Parameters - IID_IOpenRowset
	int Variation_15();
	// @cmember Parameters - Other invalid riid's
	int Variation_16();
	// @cmember Sequence - Unitialize - GetDSO
	int Variation_17();
	// @cmember Sequence - IPersist::Load (extension filename
	int Variation_18();
	// @cmember Sequence - IPersist::Load (no extension filename
	int Variation_19();
	// @cmember Sequence - 1 DSO / Multiple Sessions
	int Variation_20();
	// @cmember Sequence - Multiple DSO's / Multiple Sessions
	int Variation_21();
	// @cmember Related - OpenRowset -> Session -> DSO
	int Variation_22();
	// @cmember Related - Rowset -> Session -> DSO
	int Variation_23();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetDSO)
#define THE_CLASS TCGetDSO
BEG_TEST_CASE(TCGetDSO, COpenRowset, L"Testing IGetDataSource::GetDataSource")
	TEST_VARIATION(1, 		L"General - Verify IGetDataSource is mandatory")
	TEST_VARIATION(2, 		L"General - QI for IUnknown")
	TEST_VARIATION(3, 		L"General - QI for IID_IGetDataSource")
	TEST_VARIATION(4, 		L"General - QI for al other session interfaces (IOpenRowset")
	TEST_VARIATION(5, 		L"General - Verify AddRef / Release")
	TEST_VARIATION(6, 		L"Boundary - E_INVALIDARG")
	TEST_VARIATION(7, 		L"Boundary - E_NOINTERFACE")
	TEST_VARIATION(8, 		L"Parameters - IID_IUnknown")
	TEST_VARIATION(9, 		L"Parameters - IID_IDBProperties")
	TEST_VARIATION(10, 		L"Parameters - IID_IDBInitialize")
	TEST_VARIATION(11, 		L"Parameters - IID_IDBCreateSession")
	TEST_VARIATION(12, 		L"Parameters - IID_IPersistFile")
	TEST_VARIATION(13, 		L"Parameters - IID_ISupportErrorInfo")
	TEST_VARIATION(14, 		L"Parameters - IID_IGetDataSource")
	TEST_VARIATION(15, 		L"Parameters - IID_IOpenRowset")
	TEST_VARIATION(16, 		L"Parameters - Other invalid riid's")
	TEST_VARIATION(17, 		L"Sequence - Unitialize - GetDSO")
	TEST_VARIATION(18, 		L"Sequence - IPersist::Load (extension filename")
	TEST_VARIATION(19, 		L"Sequence - IPersist::Load (no extension filename")
	TEST_VARIATION(20, 		L"Sequence - 1 DSO / Multiple Sessions")
	TEST_VARIATION(21, 		L"Sequence - Multiple DSO's / Multiple Sessions")
	TEST_VARIATION(22, 		L"Related - OpenRowset -> Session -> DSO")
	TEST_VARIATION(23, 		L"Related - Rowset -> Session -> DSO")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


IGetDataSource* const TCGetDSO::pIGetDSO()
{
	ASSERT(m_pIGetDSO);
	return m_pIGetDSO;
}

HRESULT TCGetDSO::GetDataSource(REFIID riid, IUnknown** ppIUnknown)
{
	IUnknown* pIUnknown = INVALID(IUnknown*);
	
	//Call IGetDataSource->GetDataSource
	HRESULT hr = pIGetDSO()->GetDataSource(riid,&pIUnknown);

	//Verify output params
	if(FAILED(hr))
		TESTC(pIUnknown==NULL)
	else
		TESTC(pIUnknown!=NULL)

							  	
CLEANUP:
	//Does the caller want the returned pointer
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	
	return hr;
}

// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public TCGetDSO { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,TCGetDSO);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IGetDataSource calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IGetDataSource calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IGetDataSource calls with no previous error object existing
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, TCGetDSO, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IGetDataSource calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IGetDataSource calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid IGetDataSource calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCZombie)
//--------------------------------------------------------------------
// @class Zombie test cases for GetDataSource
//
class TCZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Zombie - ABORT with fRetaining == TRUE
	int Variation_1();
	// @cmember Zombie - ABORT with fRetaining == FALSE
	int Variation_2();
	// @cmember Zombie - COMMIT with fRetaining == TRUE
	int Variation_3();
	// @cmember Zombie - COMMIT with fRetaining == FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCZombie)
#define THE_CLASS TCZombie
BEG_TEST_CASE(TCZombie, CTransaction, L"Zombie test cases for GetDataSource")
	TEST_VARIATION(1, 		L"Zombie - ABORT with fRetaining == TRUE")
	TEST_VARIATION(2, 		L"Zombie - ABORT with fRetaining == FALSE")
	TEST_VARIATION(3, 		L"Zombie - COMMIT with fRetaining == TRUE")
	TEST_VARIATION(4, 		L"Zombie - COMMIT with fRetaining == FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAggregation)
//--------------------------------------------------------------------
// @class Test aggreagation senarios
//
class TCAggregation : public TCGetDSO { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAggregation,TCGetDSO);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Aggregation - DataSource - non IUnknown
	int Variation_1();
	// @cmember Aggregation - DataSource -> Session -> GetDataSource
	int Variation_2();
	// @cmember Aggregation - CreateSession - non IID_IUnknown
	int Variation_3();
	// @cmember Aggregation - CreateSession - IID_IUnknown
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAggregation)
#define THE_CLASS TCAggregation
BEG_TEST_CASE(TCAggregation, TCGetDSO, L"Test aggreagation senarios")
	TEST_VARIATION(1, 		L"Aggregation - DataSource - non IUnknown")
	TEST_VARIATION(2, 		L"Aggregation - DataSource -> Session -> GetDataSource")
	TEST_VARIATION(3, 		L"Aggregation - CreateSession - non IID_IUnknown")
	TEST_VARIATION(4, 		L"Aggregation - CreateSession - IID_IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetDSO)
	TEST_CASE(2, TCExtendedErrors)
	TEST_CASE(3, TCZombie)
	TEST_CASE(4, TCAggregation)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCGetDSO)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetDSO - Testing IGetDataSource::GetDataSource
//|	Created:		07/17/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetDSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COpenRowset::Init())
	// }}
	{
		//Obtain a valid GetDataSource interface
		if(QI(pIOpenRowset(),IID_IGetDataSource,(void**)&m_pIGetDSO)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify IGetDataSource is mandatory
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_1()
{
	TBEGIN  
	IGetDataSource* pIGetDSO = NULL;

	//Obtain the IID_IGetDataSource interface
	//Need to create from a new DataSource, for those providers that only support
	//1 session per DataSource.
	TESTC_(CreateNewSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGetDSO),S_OK);
		
CLEANUP:
	SAFE_RELEASE(pIGetDSO);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - QI for IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_2()
{
	TBEGIN
	
	//QI for IUnknown
	TEST_(QI(pIGetDSO(),IID_IUnknown),S_OK)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - QI for IID_IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_3()
{
	TBEGIN

	//QI for IGetDataSource
	TEST_(QI(pIGetDSO(),IID_IGetDataSource),S_OK)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - QI for al other session interfaces (IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_4()
{
	TBEGIN
	HRESULT hrTableDef, hrCommand, hrTransaction;
	ULONG_PTR ulValue = DBPROPVAL_TC_NONE;
	BOOL bSupported = TRUE;

	//[manadatory] session interfaces
	TESTC_(QI(pIGetDSO(),IID_IOpenRowset),S_OK)
	TESTC_(QI(pIGetDSO(),IID_IGetDataSource),S_OK)
	TESTC_(QI(pIGetDSO(),IID_ISessionProperties),S_OK)

	//[optional] session interfaces (provider specific)
	TEST2C_(QI(pIGetDSO(),IID_IDBSchemaRowset), S_OK, E_NOINTERFACE);
	TEST2C_(QI(pIGetDSO(),IID_ISupportErrorInfo), S_OK, E_NOINTERFACE);
	
	//[optional] session interfaces (provider specific)
	hrTableDef = QI(pIGetDSO(),IID_ITableDefinition);
	hrCommand = QI(pIGetDSO(),IID_IDBCreateCommand);
	hrTransaction = QI(pIGetDSO(),IID_ITransactionLocal);

	//Combinations:
	//		DDL (TableDef or Commands)	Transactions	SupportedDDL
	//	#1		N							N				Not Supported or TC_NONE
	//	#2		N							Y				Not Supported or TC_NONE
	//	#3		Y							N				Not Supported or TC_NONE
	//	#4		Y							Y				Should be supported at some level or TC_NONE

	//DBPROP_SUPPORTEDTXNDDL
	bSupported = GetProperty(DBPROP_SUPPORTEDTXNDDL, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession, &ulValue);
	if(bSupported)
	{
		//(case #1, #2, #3, or #4)
		if(ulValue==DBPROPVAL_TC_NONE)
		{
			//If supported but the value is NONE, then there is not much we can
			//test.  Since transactions could still be supported, but just
			//not all DDL (TableDef or Commands).  You could even be supporting this property
			//if DDL (TableDef or Commands) are not supported, since your supporting a property just
			//to advertise you don't support it at any level...
			TEST2C_(hrTableDef, S_OK, E_NOINTERFACE);
			TEST2C_(hrCommand, S_OK, E_NOINTERFACE);
			TEST2C_(hrTransaction, S_OK, E_NOINTERFACE);
			TWARNING("DBPROP_SUPPORTEDTXNDDL = DBPROPVAL_TC_NONE, is this correct?");
		}
		//(case #4)
		else
		{
			//If supported and some level, then obviously ITransactionLocal and 
			//DDL (TableDef or Commands) should be supported. (since this is the supported "TXN DDL")
			TESTC(hrTableDef==S_OK || hrCommand==S_OK);
			TESTC_(hrTransaction, S_OK)
		}
	}
	else
	{
		//And ITransactionLocal may or may not be supported,
		//(since transactions do not require commands)
		TEST2C_(hrTransaction, S_OK, E_NOINTERFACE);
		if(SUCCEEDED(hrTransaction))
		{
			//If transactions are supported, and the property is not, then
			//DDL (TableDef or Commands) must not be supported.  Since if you support transacitons
			//and DDL (TableDef or Commands) you should at least advertise the level of DDL or TC_NONE
			TESTW_(hrTableDef, E_NOINTERFACE);
			TESTW_(hrCommand, E_NOINTERFACE);
		}
		else
		{
			//Transactions are not supported, and the property is not supported,
			//which tells us nothing about DDL (TableDef or Commands) support
			TEST2C_(hrTableDef, S_OK, E_NOINTERFACE);
			TEST2C_(hrCommand, S_OK, E_NOINTERFACE);
		}
	}

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Verify AddRef / Release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_5()
{
	TBEGIN
	//Get the original RefCount
	ULONG OrgRefCount = GetRefCount(pIGetDSO());
	
	//AddRef a few times
	SetRefCount(pIGetDSO(), 10);
	//Release a few times
	SetRefCount(pIGetDSO(), -2);

	//Verify new RefCount
	TESTC(VerifyRefCounts(GetRefCount(pIGetDSO()), OrgRefCount + (10-2)));

CLEANUP:
	//Restore the original ref count
	SetRefCount(pIGetDSO(), -(10-2));
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_6()
{
	TBEGIN

	//GetDataSource(valid,NULL)
	TEST_(pIGetDSO()->GetDataSource(IID_IDBInitialize,NULL),E_INVALIDARG)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_7()
{
	TBEGIN

	//GetDataSource(NULLID,valid)  	 
	TEST_(GetDataSource(NULLGUID),E_NOINTERFACE)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_8()
{
	TBEGIN
	IUnknown* pIDSOUnknown = NULL;
	IDBInitialize* pIDBInitialize = NULL;


	//GetDataSource  	 
	TESTC_(GetDataSource(IID_IUnknown,&pIDSOUnknown),S_OK)
	TESTC(pIDSOUnknown!=NULL)

	//Verify this a valid DSO IUnknown
	TESTC_(QI(pIDSOUnknown,IID_IDBInitialize,(void**)&pIDBInitialize),S_OK)
	TESTC(pIDBInitialize != NULL)


CLEANUP:
	SAFE_RELEASE(pIDSOUnknown);
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IDBProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_9()
{
	TBEGIN
		
	//IDBProperties  	 
	TEST_(GetDataSource(IID_IDBProperties),S_OK)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IDBInitialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_10()
{
	TBEGIN
		
	//IDBInitialize  	 
	TEST_(GetDataSource(IID_IDBInitialize),S_OK)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IDBCreateSession
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_11()
{
	TBEGIN
	
	//GetDataSource  	 
	TEST_(GetDataSource(IID_IDBCreateSession),S_OK)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IPersistFile
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_12()
{
	TBEGIN
	
	//GetDataSource  	 
	HRESULT hr = GetDataSource(IID_IPersistFile);
	
	if(MSDASQL)
		TEST_(hr,S_OK)
	else
		TEST(hr==S_OK || hr==E_NOINTERFACE)

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_ISupportErrorInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_13()
{
	TBEGIN
	
	//GetDataSource  	 
	HRESULT hr = GetDataSource(IID_ISupportErrorInfo);
	
	if(MSDASQL)
		TEST_(hr,S_OK)
	else
		TEST(hr==S_OK || hr==E_NOINTERFACE)

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_14()
{
	TBEGIN

	//GetDataSource
	TEST_(GetDataSource(IID_IGetDataSource),E_NOINTERFACE)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - IID_IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_15()
{
	TBEGIN

	//GetDataSource
	TEST_(GetDataSource(IID_IOpenRowset),E_NOINTERFACE)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Other invalid riid's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_16()
{
	TBEGIN

	//IDBCreateCommand
	TEST_(GetDataSource(IID_IDBCreateCommand),E_NOINTERFACE)
	//IDBSchemaRowset
	TEST_(GetDataSource(IID_IDBSchemaRowset),E_NOINTERFACE)
	//IAccessor
	TEST_(GetDataSource(IID_IAccessor),E_NOINTERFACE)
	//IRowset
	TEST_(GetDataSource(IID_IRowset),E_NOINTERFACE)
	//IRowsetInfo
	TEST_(GetDataSource(IID_IRowsetInfo),E_NOINTERFACE)

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Unitialize - GetDSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_17()
{
	TBEGIN

	IDBInitialize* pIDBInitialize = NULL;
	IGetDataSource* pIGetDataSource = NULL; 
	IPersist* pIPersist = NULL; 

	//Obtain a new DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK);
	//Obtain a new Session from that DSO
	TESTC_(CreateNewSession(pIDBInitialize,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),S_OK)
	
	//Now Unitialize, shouldn't since there's a session open
	TESTC_(pIDBInitialize->Uninitialize(),DB_E_OBJECTOPEN)

	//There is no way of uninitializing and having a pIGetDataSource interface
	//So we'll just make sure that an error call to Unint followed by GetDataSource
	//succeeds

	//GetDSO
	TESTC_(pIGetDataSource->GetDataSource(IID_IPersist,(IUnknown**)&pIPersist),S_OK)
	TESTC(pIPersist!=NULL)

	
CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIGetDataSource); 
	SAFE_RELEASE(pIPersist); 
	TRETURN
}													  
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Sequence - IPersist::Load (extension filename
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_18()
{
	TBEGIN

	IDBInitialize* pIDBInitialize = NULL; 
	IDBCreateSession* pIDBCreateSession = NULL; 
	IPersistFile* pIPersistFile = NULL; 
	IGetDataSource* pIGetDataSource = NULL; 

	//Since IPersistFile is Optional check to see if supported
	if(QI(pIGetDSO(),IID_IPersistFile)==E_NOINTERFACE)
		goto CLEANUP;

	//Need to persist the current DSO
	TESTC_(SaveDSO(PERSIST_FILE),S_OK); 

	//Create another instance, since our current one from the privlib has too many
	//references to uninitilize
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile, (IUnknown**)&pIPersistFile),S_OK);

	//Now Initialize
	TESTC_(pIPersistFile->Load(PERSIST_FILE,0),S_OK)
	TESTC_(QI(pIPersistFile,IID_IDBInitialize,(void**)&pIDBInitialize),S_OK)
	TESTC_(pIDBInitialize->Initialize(),S_OK)

	//Create the Session Object
	TESTC_(CreateNewSession(pIPersistFile,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),S_OK)
		
	//GetDSO
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBCreateSession,(IUnknown**)&pIDBCreateSession),S_OK)
	TESTC(pIDBCreateSession!=NULL)

	
CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIGetDataSource); 
	SAFE_RELEASE(pIPersistFile); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Sequence - IPersist::Load (no extension filename
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_19()
{
	TBEGIN

	IDBInitialize* pIDBInitialize = NULL; 
	IDBCreateSession* pIDBCreateSession = NULL; 
	IPersistFile* pIPersistFile = NULL; 
	IGetDataSource* pIGetDataSource = NULL; 

	//Since IPersistFile is Optional check to see if supported
	if(QI(pIGetDSO(),IID_IPersistFile)==E_NOINTERFACE)
		goto CLEANUP;

	//Need to persist the current DSO
	TESTC_(SaveDSO(L"Noext"),S_OK); 

	//Create another instance, since our current one from the privlib has too many
	//references to uninitilize
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile, (IUnknown**)&pIPersistFile),S_OK);

	//Now Initialize
	TESTC_(pIPersistFile->Load(L"Noext",0),S_OK)
	TESTC_(QI(pIPersistFile,IID_IDBInitialize,(void**)&pIDBInitialize),S_OK)
	TESTC_(pIDBInitialize->Initialize(),S_OK)

	//Create the Session Object
	TESTC_(CreateNewSession(pIPersistFile,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),S_OK)
		
	//GetDSO
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBCreateSession,(IUnknown**)&pIDBCreateSession),S_OK)
	TESTC(pIDBCreateSession!=NULL)

	
CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIGetDataSource); 
	SAFE_RELEASE(pIPersistFile); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Sequence - 1 DSO / Multiple Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_20()
{
	TBEGIN

	IGetDataSource* pIGetDataSource1 = NULL; 
	IGetDataSource* pIGetDataSource2 = NULL; 
	IGetDataSource* pIGetDataSource3 = NULL; 
	
	IPersistFile* pIPersistFile1 = NULL; 
	IPersistFile* pIPersistFile2 = NULL; 
	IPersistFile* pIPersistFile3 = NULL; 

	//Since IPersistFile is Optional check to see if supported
	if(QI(pIGetDSO(),IID_IPersistFile)==E_NOINTERFACE)
		goto CLEANUP;

	//Create 3 new Sessions from the current DSO
	TESTC_(CreateNewSession(m_pIDBInitialize,IID_IGetDataSource,(IUnknown**)&pIGetDataSource1),S_OK)
	TESTC_(CreateNewSession(m_pIDBInitialize,IID_IGetDataSource,(IUnknown**)&pIGetDataSource2),S_OK)
	TESTC_(CreateNewSession(m_pIDBInitialize,IID_IGetDataSource,(IUnknown**)&pIGetDataSource3),S_OK)
		
	//GetDSO from seprate sessions
	TESTC_(pIGetDataSource1->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile1),S_OK)
	TESTC_(pIGetDataSource2->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile2),S_OK)
	TESTC_(pIGetDataSource3->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile3),S_OK)
	
	//Verify all equal
	TESTC(pIPersistFile1 == pIPersistFile2)
	TESTC(pIPersistFile2 == pIPersistFile3)

	
CLEANUP:
	SAFE_RELEASE(pIGetDataSource1); 
	SAFE_RELEASE(pIGetDataSource2); 
	SAFE_RELEASE(pIGetDataSource3); 

	SAFE_RELEASE(pIPersistFile1); 
	SAFE_RELEASE(pIPersistFile2); 
	SAFE_RELEASE(pIPersistFile3); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple DSO's / Multiple Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_21()
{
	TBEGIN

	IGetDataSource* pIGetDataSource1 = NULL; 
	IGetDataSource* pIGetDataSource2 = NULL; 
	IGetDataSource* pIGetDataSource3 = NULL; 
	
	IPersistFile* pIPersistFile1 = NULL; 
	IPersistFile* pIPersistFile2 = NULL; 
	IPersistFile* pIPersistFile3 = NULL; 

	//Since IPersistFile is Optional check to see if supported
	if(QI(pIGetDSO(),IID_IPersistFile)==E_NOINTERFACE)
		goto CLEANUP;

	//Create 3 new Sessions from new DSO's
	TESTC_(CreateNewSession(NULL,IID_IGetDataSource,(IUnknown**)&pIGetDataSource1),S_OK)
	TESTC_(CreateNewSession(NULL,IID_IGetDataSource,(IUnknown**)&pIGetDataSource2),S_OK)
	TESTC_(CreateNewSession(NULL,IID_IGetDataSource,(IUnknown**)&pIGetDataSource3),S_OK)
		
	//GetDSO from seprate sessions
	TESTC_(pIGetDataSource1->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile1),S_OK)
	TESTC_(pIGetDataSource2->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile2),S_OK)
	TESTC_(pIGetDataSource3->GetDataSource(IID_IPersistFile,(IUnknown**)&pIPersistFile3),S_OK)
	
	//Verify not equal (different DSO's)
	TESTC(pIPersistFile1 != pIPersistFile2)
	TESTC(pIPersistFile2 != pIPersistFile3)

	
CLEANUP:
	SAFE_RELEASE(pIGetDataSource1); 
	SAFE_RELEASE(pIGetDataSource2); 
	SAFE_RELEASE(pIGetDataSource3); 

	SAFE_RELEASE(pIPersistFile1); 
	SAFE_RELEASE(pIPersistFile2); 
	SAFE_RELEASE(pIPersistFile3); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Related - OpenRowset -> Session -> DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_22()
{
	TBEGIN
	HRESULT hr = S_OK;

	IRowset* pIRowset = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	
	IGetDataSource* pIGetDataSource = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;

	//Create an OpenRowset
	TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)

	//Now get back to the session pointer
	TESTC_(QI(pIRowset,IID_IRowsetInfo,(void**)&pIRowsetInfo),S_OK)
	
	//GetSpecificiation can return S_FALSE if no parent object is stored
	hr = pIRowsetInfo->GetSpecification(IID_IGetDataSource,(IUnknown**)&pIGetDataSource);
	TESTC(hr==S_OK || hr==S_FALSE);
	
	//Exit Successfully if provider doesn't support GetSpecificaiton
	TESTC_PROVIDER(hr==S_OK);

	//Now get back to the DSO
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBCreateSession,(IUnknown**)&pIDBCreateSession),S_OK)
	TESTC(pIDBCreateSession!=NULL)


CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Related - Rowset -> Session -> DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetDSO::Variation_23()
{
	TBEGIN
	HRESULT hr = S_OK;

	IDBCreateSession* pIDBCreateSession = NULL;
	IGetDataSource* pIGetDataSource = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;

	//Create the rowset using commands
	CRowset RowsetA;
	TESTC_(RowsetA.CreateRowset(USE_OPENROWSET),S_OK);

	//Now get back to the Command pointer
	TESTC_(QI(RowsetA(),IID_IRowsetInfo,(void**)&pIRowsetInfo),S_OK)

	//GetSpecificiation can return S_FALSE if no parent object is stored
	//Parent object will be IOpenRowset since it was USE_OPENROWSET
	hr = pIRowsetInfo->GetSpecification(IID_IGetDataSource,(IUnknown**)&pIGetDataSource);
	TESTC(hr==S_OK || hr==S_FALSE);
	
	//Exit Successfully if provider doesn't support GetSpecificaiton
	TESTC_PROVIDER(hr==S_OK);

	//Now get back to the DSO
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBCreateSession,(IUnknown**)&pIDBCreateSession),S_OK)
	TESTC(pIDBCreateSession!=NULL)


CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetDSO::Terminate()
{
	SAFE_RELEASE(m_pIGetDSO);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(COpenRowset::Terminate());
}	// }}
// }}
//}}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		07/21/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCGetDSO::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IGetDataSource calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	//For the method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IGetDataSource method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	TBEGIN

	IDBInitialize* pIDBInitialize = NULL;
	IGetDataSource* pIGetDataSource = NULL; 
	IPersist* pIPersist = NULL; 

	//Obtain a new DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK)
	//Obtain a new Session from that DSO
	TESTC_(CreateNewSession(pIDBInitialize,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),S_OK)
	
	//Now Unitialize, shouldn't since there's a session open
	TESTC_(pIDBInitialize->Uninitialize(),DB_E_OBJECTOPEN)

	TESTC(m_pExtError->CauseError())
	
	//There is no way of uninitializing and having a pIGetDataSource interface
	//So we'll just make sure that an error call to Unint followed by GetDataSource
	//succeeds

	//GetDSO
	TESTC_(m_hr=(pIGetDataSource->GetDataSource(IID_IPersist,(IUnknown**)&pIPersist)),S_OK)
	//Do extended check following 
	TESTC(m_pExtError->ValidateExtended(m_hr, pIGetDataSource, IID_IGetDataSource, LONGSTRING(__FILE__), __LINE__))
	TESTC(pIPersist!=NULL)

	
CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIGetDataSource); 
	SAFE_RELEASE(pIPersist); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IGetDataSource calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
    //For the method of the interface, first create an error object on
	//the current thread, then try get a failure from the IGetDataSource method.
	//We then check extended errors to verify the right extended error behavior.
	TBEGIN

	TESTC(m_pExtError->CauseError())

	//GetDataSource(valid,NULL)
   
	TEST_(m_hr=(pIGetDSO()->GetDataSource(IID_IDBInitialize,NULL)),E_INVALIDARG)
	//do extended error check
	TESTC(m_pExtError->ValidateExtended(m_hr, pIGetDSO(), IID_IGetDataSource, LONGSTRING(__FILE__), __LINE__))
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IGetDataSource calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	//For the method of the interface, with no error object on
	//the current thread, try get a failure from the IGetDataSource method.
	//We then check extended errors to verify the right extended error behavior.
	TBEGIN

	//GetDataSource(valid,NULL)
	TEST_(m_hr=(pIGetDSO()->GetDataSource(IID_IDBInitialize,NULL)),E_INVALIDARG)
	//do extended error check
	TESTC(m_pExtError->ValidateExtended(m_hr, pIGetDSO(), IID_IGetDataSource, LONGSTRING(__FILE__), __LINE__))
	
CLEANUP:
	TRETURN
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
	return(TCGetDSO::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZombie - Zombie test cases for GetDataSource
//|	Created:		07/22/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		//register interface to be tested                                         
   		if(RegisterInterface(SESSION_INTERFACE, IID_IGetDataSource)) 
   			return TRUE;
		
	}

	//Not all providers have to support transactions
	//If a required interface, an error would ahve been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_1()
{
	IGetDataSource* pIGetDSO = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIGetDSO))
	TESTC(pIGetDSO!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	
	//Obtain the first row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr)

	//Verify we still can use IOpenRowset after an ABORT			
	TESTC_(pIGetDSO->GetDataSource(IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK)

CLEANUP:
	//Release the rows
	if(m_pIRowset && rghRow)
		m_pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIGetDSO);
	SAFE_RELEASE(pIDBInitialize);
	PROVIDER_FREE(rghRow);
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_2()
{
	IGetDataSource* pIGetDSO = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIGetDSO))
	TESTC(pIGetDSO!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	
	//Obtain the first row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr)

	//Verify we still can use IOpenRowset after an ABORT			
	TESTC_(pIGetDSO->GetDataSource(IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK)

CLEANUP:
	//Release the rows
	if(m_pIRowset && rghRow)
		m_pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIGetDSO);
	SAFE_RELEASE(pIDBInitialize);
	PROVIDER_FREE(rghRow);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_3()
{
	IGetDataSource* pIGetDSO = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIGetDSO))
	TESTC(pIGetDSO!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	
	//Obtain the first row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr)

	//Verify we still can use IOpenRowset after a COMMIT			
	TESTC_(pIGetDSO->GetDataSource(IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK)

CLEANUP:
	//Release the rows
	if(m_pIRowset && rghRow)
		m_pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIGetDSO);
	SAFE_RELEASE(pIDBInitialize);
	PROVIDER_FREE(rghRow);
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_4()
{
	IGetDataSource* pIGetDSO = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIGetDSO))
	TESTC(pIGetDSO!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	
	//Obtain the first row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr)

	//Verify we still can use IOpenRowset after a COMMIT
	TESTC_(pIGetDSO->GetDataSource(IID_IDBInitialize,(IUnknown**)&pIDBInitialize),S_OK)

CLEANUP:
	//Release the rows
	if(m_pIRowset && rghRow)
		m_pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIGetDSO);
	SAFE_RELEASE(pIDBInitialize);
	PROVIDER_FREE(rghRow);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAggregation)
//*-----------------------------------------------------------------------
//| Test Case:		TCAggregation - Test aggreagation senarios
//|	Created:			10/31/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAggregation::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCGetDSO::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - DataSource - non IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_1()
{ 
	TBEGIN
    CAggregate Aggregate(pIGetDSO());
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TEST2C_(GetModInfo()->CreateProvider(&Aggregate, IID_IDBInitialize, &pIUnkInner), DB_E_NOAGGREGATION, CLASS_E_NOAGGREGATION);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	
	//For some reason COM is not nulling out the output pointer.  This will be 
	//considered a warning, all other OLE DB methods explcitly state to NULL the 
	//output param on error...
	COMPAREW(pIUnkInner, NULL);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - DataSource -> Session -> GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_2()
{
	TBEGIN
    CAggregate Aggregate(pIGetDSO());
    IGetDataSource* pIGetDataSource = NULL;
    IDBCreateSession* pIDBCreateSession = NULL;
	IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIUnkInner	= NULL;
	IUnknown* pIAggregate	= NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	HRESULT hr = S_OK;

	//Aggregation
	hr = CreateNewDSO(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner, CREATEDSO_NONE);
	Aggregate.SetUnkInner(pIUnkInner);

	//VerifyArregation
	//Indicate we are not Initialized at this point...
	TESTC_PROVIDER(hr = Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize, NULL, FALSE));

	//Now Initialize
	//Since the Service Components create a session (for session pooling)
	//Calling CreateNewDSO in on step has extra references...
	TESTC_(hr = InitializeDataSource(pIUnkInner),S_OK);

	//Obtain a new Session from that DSO
	TESTC_(hr = QI(pIUnkInner, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK);
	
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGetDataSource),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//Verify the child correctly addref'd the parent outer.
	//The is an absolute requirement that the child keep the parent outer alive.
	//If it doesn't addref the outer, the outer can be released externally since
	//its not being used anymore due to the fact the outer controls the refcount
	//of the inner.  Many providers incorrectly addref the inner, which does nothing
	//but guareentee the inner survives, but the inner will delegate to the outer
	//and crash since it no longer exists...
	TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	
	//Verify we are hooked up...
	//This call we are using the Session and asking for IID_IAggregate of the DataSource, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIDBCreateSession));

	//Now make sure the Session GetDataSource for IUnknown give me the outer
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIUnkOuter, pIDBCreateSession));

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession - non IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_3()
{ 
	TBEGIN
    CAggregate Aggregate(pIGetDSO());
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(CreateNewSession(NULL, IID_IOpenRowset, &pIUnkInner, &Aggregate), DB_E_NOAGGREGATION);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession - IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_4()
{
	TBEGIN
    CAggregate Aggregate(pIGetDSO());
	IUnknown* pIUnkInner = NULL;

	//Aggregation
	HRESULT hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//VerifyArregation
	TESTC_PROVIDER(hr = Aggregate.VerifyAggregationQI(hr, IID_IGetDataSource));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAggregation::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCGetDSO::Terminate());
}	// }}
// }}
// }}
