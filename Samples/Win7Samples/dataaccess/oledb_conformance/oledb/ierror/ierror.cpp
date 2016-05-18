//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module ERROR.CPP | Source File for OLE DB Extended Error Object
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "ierror.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xd499ab91, 0x61c6, 0x11cf, { 0x97, 0x66, 0x00, 0xaa, 0x00, 0xbd, 0xf9, 0x52 }};
DECLARE_MODULE_NAME("IError");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Extended Error Object and IErrorLookup test");
DECLARE_MODULE_VERSION(825612227);
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
	//Must either call CreateModInfo or CreateModuleDBSession before any testing
	if(!CreateModInfo(pThisTestModule))
		return FALSE;

	return TRUE;
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
	return ReleaseModInfo(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

////////////////////////////////////////////////////////////////////
//A mock custom error object class, only implements the 
//IUnknown interface.
////////////////////////////////////////////////////////////////////
class CCustomObj : public IUnknown 
{
private:
	// @cmember Ref Count
	DBREFCOUNT	m_Ref;
public:
	// @cmember CTOR
	CCustomObj();
	// @cmember DTOR
	virtual ~CCustomObj(){};
	
	// @cmember IUnknown implementation of AddRef
	STDMETHODIMP_(ULONG)	AddRef(void);
	// @cmember IUnknown implementation of Release
	STDMETHODIMP_(ULONG)	Release(void);
	// @cmember IUnknown implementation of QI
	STDMETHODIMP			QueryInterface(REFIID , LPVOID *);
};

CCustomObj::CCustomObj()
{
	//When we create this, we should increment the ref count.
	//We give up rights to this object on AddRecord by calling 
	//release afterward.
	m_Ref = 1;
}

STDMETHODIMP_(ULONG) CCustomObj::AddRef(void)
{
	m_Ref++;
	return m_Ref;
}

STDMETHODIMP_(ULONG) CCustomObj::Release(void)
{
	//Programming error to call this without addrefing first
	ASSERT(m_Ref);

	//We delete ourself if we reach ref count of zero
	if (!--m_Ref)
	{
		delete this;
		return 0;
	}

	return m_Ref;
}

STDMETHODIMP		 CCustomObj::QueryInterface(REFIID riid, LPVOID * ppObj)
{
	//We only support IUnknown
	if (riid == IID_IUnknown)
	{
		*ppObj = this;
		m_Ref++;
		return NOERROR;
	}
	else
		return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////
//A base class for all error tests
////////////////////////////////////////////////////////////////////
class CErrorTest : public CDataSourceObject {
protected:
	// @cmember Valid locale id
	LCID		m_lcid;
	// @cmember Valid Parameter for error
	DISPPARAMS	m_DispParam;
	// @cmember dispidNamedArg for m_DispParam struct
	DISPID		m_DispId;
	// @cmember Variant arg for m_DispParam struct
	VARIANTARG	m_vArg;
	// @cmember Struct to hold retrieved DISPPARAMS value
	DISPPARAMS	m_NullDispParam;
	// @cmember Clsid of component we're testing
	CLSID		m_Clsid;	
	// @cmember IErrorInfo interface retrieved from object
	IErrorInfo *m_pIErrorInfo;
	
public:
	// @cmember CTOR	
	CErrorTest(LPWSTR wszTestCase) : CDataSourceObject(wszTestCase)
	{
		m_lcid = GetSystemDefaultLCID(); 

		//Set up a sample parameter				
		m_vArg.vt						= VT_BOOL;
		V_BOOL(&(m_vArg))				= VARIANT_TRUE;
		m_DispId						= DISPID_VALUE;
		m_DispParam.rgvarg				= &m_vArg;
		m_DispParam.rgdispidNamedArgs	= &m_DispId;
		m_DispParam.cArgs				= 1;
		m_DispParam.cNamedArgs			= 1;

		//Now init a struct which will be used
		//to test null param insertion,
		//and also be used to verify that
		//these values don't get overwritten by
		//the existing error records params
		//for a GetErrorParameters call which 
		//should fail.
		m_NullDispParam.rgvarg				= NULL;
		m_NullDispParam.rgdispidNamedArgs	= NULL;
		m_NullDispParam.cArgs				= 0;
		m_NullDispParam.cNamedArgs			= 0;
		
		m_pIErrorInfo						= NULL;	
	}
	// @cmember DTOR	
	virtual ~CErrorTest(){};
	// @cmember Compares the elements of two DISPPARAM structs, using COMPARE macro
	void CompareDispParams(DISPPARAMS * param1, DISPPARAMS * param2);
	// @cmember Figure out the CLSID of the ExtErrorInfo Object
	CLSID GetExtendedErrorsLookupCLSID();
};

////////////////////////////////////////////////////////////////////
// A base class for IErrorRecords functionality
////////////////////////////////////////////////////////////////////
class CErrorRecords : public CErrorTest
{
protected:

	// @cmember Number of records actually in the object
	ULONG			m_ulRecordNum;
	// @cmember IErrorRecords interface retrieved from object
	IErrorRecords *	m_pIErrorRecords;
	// @cmember Memeory for custom error object interface
	IUnknown *		m_pObject;
	// @cmember Array of 5 ERRORINFO structs, used for our simulated errors
	ERRORINFO		m_rgErrorInfo[5];

public:
	// @cmember CTOR
	CErrorRecords(LPWSTR wszTestCase) : CErrorTest(wszTestCase)
	{
		m_Clsid				= CLSID_EXTENDEDERRORINFO;			
		m_pIErrorRecords	= NULL;			
		m_pObject			= NULL;		
		m_ulRecordNum		= 0;
	}
	// @cmember DTOR
	virtual ~CErrorRecords(){};
	//@cmember Initialization for m_rgErrorInfo
	virtual BOOL Init();
	//@cmember Verifies all IErrorRecords info is returned correctly
	void VerifyErrorRecord(ULONG ulRecordNum, ULONG ulErrorInfoIndex);
	//@cmember Create a IErrorInfo
	BOOL CreateIErrorInfo();
};


////////////////////////////////////////////////////////////////////
// A base class for ISupportErrorInfo functionality
////////////////////////////////////////////////////////////////////
class CSupportError {

protected:
	// @cmember Array of all supported interfaces	
	SUPPORTEDINTERFACES	* m_rgSupportedInterfaces;
	// @cmember Count of supported interfaces in m_rgSuportedInterfaces
	ULONG m_cSupportedInterfaces;
	// @cmember Interface to test
	ISupportErrorInfo * m_pISupportErrorInfo;
	// @cmember Interface to test
	IErrorInfo * m_pIErrorInfo;
	// @cmember Interface to object we are testing
	IUnknown * m_pIUnkObject;
	// @cmember Count of supported interfaces in m_rgSuportedInterfaces
	ULONG m_cInterfaces;
	// @cmember Array of all possible interfaces
	INTERFACEMAP * m_rgInterfaces;

public:

	// @cmember CTOR
	CSupportError()
	{
		m_pISupportErrorInfo	= NULL;
		m_pIErrorInfo			= NULL;
		m_pIUnkObject			= NULL;
		m_rgSupportedInterfaces = NULL;
		m_cSupportedInterfaces	= 0;
		m_cInterfaces			= 0;
		m_rgInterfaces			= NULL;
	};

	// @cmember DTOR
	virtual ~CSupportError(){};
	
	// @cmember Init function
	BOOL	Init(BOOL fOnlyMandatory = FALSE);
	// @cmember Terminate function
	BOOL	Terminate();
	// @cmember Logs all supported interfaces and their error support
	BOOL LogErrorSupport();

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCErrorRecords)
//--------------------------------------------------------------------
// @class API level tests for IErrorRecords
//
class TCErrorRecords : public CErrorRecords { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorRecords,CErrorRecords);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - AddErrorRecord, pErrorInfo = NULL
	int Variation_1();
	// @cmember E_INVALIDARG - GetBasicErrorInfo, pErrorInfo = NULL
	int Variation_2();
	// @cmember DB_E_BADRECORDNUM - GetBasicErrorInfo, bad ulRecordNum
	int Variation_3();
	// @cmember GetCustomErrorObject, NULL *ppObject for no custom error object
	int Variation_4();
	// @cmember E_INVALIDARG - GetCustomErrorObject, ppObject = NULL
	int Variation_5();
	// @cmember DB_E_BADRECORDNUM - GetCustomErrorObject, bad ulRecordNum
	int Variation_6();
	// @cmember E_INVALIDARG - GetErrorInfo, ppErrorInfo = NULL
	int Variation_7();
	// @cmember DB_E_BADRECORDNUM - GetErrorInfo, bad ulRecordNum
	int Variation_8();
	// @cmember E_INVALIDARG - GetErrorParameters, pdispparams = NULL
	int Variation_9();
	// @cmember DB_E_BADRECORDNUM - GetErrorParameters, bad ulRecordNum
	int Variation_10();
	// @cmember E_INVALIDARG - GetRecordCount, pcRecords = NULL
	int Variation_11();
	// @cmember GetCustomErrorObject - valid
	int Variation_12();
	// @cmember GetErrorInfo - valid
	int Variation_13();
	// @cmember GetErrorParameters - valid
	int Variation_14();
	// @cmember GetRecordCount - valid
	int Variation_15();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorRecords)
#define THE_CLASS TCErrorRecords
BEG_TEST_CASE(TCErrorRecords, CErrorRecords, L"API level tests for IErrorRecords")
	TEST_VARIATION(1, 		L"E_INVALIDARG - AddErrorRecord, pErrorInfo = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG - GetBasicErrorInfo, pErrorInfo = NULL")
	TEST_VARIATION(3, 		L"DB_E_BADRECORDNUM - GetBasicErrorInfo, bad ulRecordNum")
	TEST_VARIATION(4, 		L"GetCustomErrorObject, NULL *ppObject for no custom error object")
	TEST_VARIATION(5, 		L"E_INVALIDARG - GetCustomErrorObject, ppObject = NULL")
	TEST_VARIATION(6, 		L"DB_E_BADRECORDNUM - GetCustomErrorObject, bad ulRecordNum")
	TEST_VARIATION(7, 		L"E_INVALIDARG - GetErrorInfo, ppErrorInfo = NULL")
	TEST_VARIATION(8, 		L"DB_E_BADRECORDNUM - GetErrorInfo, bad ulRecordNum")
	TEST_VARIATION(9, 		L"E_INVALIDARG - GetErrorParameters, pdispparams = NULL")
	TEST_VARIATION(10, 		L"DB_E_BADRECORDNUM - GetErrorParameters, bad ulRecordNum")
	TEST_VARIATION(11, 		L"E_INVALIDARG - GetRecordCount, pcRecords = NULL")
	TEST_VARIATION(12, 		L"GetCustomErrorObject - valid")
	TEST_VARIATION(13, 		L"GetErrorInfo - valid")
	TEST_VARIATION(14, 		L"GetErrorParameters - valid")
	TEST_VARIATION(15, 		L"GetRecordCount - valid")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCErrorLookup)
//--------------------------------------------------------------------
// @class API level tests for IErrorLookup
//
class TCErrorLookup : public CErrorTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
protected:
	// @cmember Valid provider specific number of the error
	DWORD	m_dwMinor;
	// @cmember Valid HRESULT
	HRESULT	m_hrError;
	// @cmember Valid BSTR for holding memory for results
	BSTR	m_bstrValid;
	// @cmember Valid BSTR for holding memory for error description
	BSTR	m_bstrDesc;
	// @cmember Valid memory to hold Help Context
	DWORD	m_dwHelpContext;
	// @cmember IErrorLookup interface
	IErrorLookup * m_pIErrorLookup;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorLookup,CErrorTest);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - GetErrorDescription, pbstrSource = NULL
	int Variation_1();
	// @cmember E_INVALIDARG - GetErrorDescription, pbstrDescription = NULL
	int Variation_2();
	// @cmember E_INVALIDARG - GetHelpInfo, pbstrHelpFile = NULL
	int Variation_3();
	// @cmember E_INVALIDARG - GetHelpInfo, pdwHelpContext = NULL
	int Variation_4();
	// @cmember DB_E_NOLOCALE - GetHelpInfo, lcid = DISP_E_UNKNOWNLCID
	int Variation_5();
	// @cmember DB_E_BADHRESULT - GetErrorDescription, invalid hrError
	int Variation_6();
	// @cmember DB_E_BADHRESULT - GetHelpInfo, invalid hrError
	int Variation_7();
	// @cmember DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID
	int Variation_8();
	// @cmember DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID
	int Variation_9();
	// @cmember S_OK - GetErrorDescription, dwLookupID=IDENTIFIER_SDK_ERROR
	int Variation_10();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorLookup)
#define THE_CLASS TCErrorLookup
BEG_TEST_CASE(TCErrorLookup, CErrorTest, L"API level tests for IErrorLookup")
	TEST_VARIATION(1, 		L"E_INVALIDARG - GetErrorDescription, pbstrSource = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG - GetErrorDescription, pbstrDescription = NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG - GetHelpInfo, pbstrHelpFile = NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG - GetHelpInfo, pdwHelpContext = NULL")
	TEST_VARIATION(5, 		L"DB_E_NOLOCALE - GetHelpInfo, lcid = DISP_E_UNKNOWNLCID")
	TEST_VARIATION(6, 		L"DB_E_BADHRESULT - GetErrorDescription, invalid hrError")
	TEST_VARIATION(7, 		L"DB_E_BADHRESULT - GetHelpInfo, invalid hrError")
	TEST_VARIATION(8, 		L"DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID")
	TEST_VARIATION(9, 		L"DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID")
	TEST_VARIATION(10, 		L"S_OK - GetErrorDescription, dwLookupID=IDENTIFIER_SDK_ERROR")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCNoErrorRecords)
//--------------------------------------------------------------------
// @class No Error Records set
//
class TCNoErrorRecords : public CErrorRecords { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNoErrorRecords,CErrorRecords);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IErrorRecords::GetRecordCount
	int Variation_1();
	// @cmember IErrorRecords::GetErrorParameters
	int Variation_2();
	// @cmember IErrorRecords::GetErrorInfo
	int Variation_3();
	// @cmember IErrorRecords::GetCustomErrorObject
	int Variation_4();
	// @cmember IErrorRecords::GetBasicErrorInfo
	int Variation_5();
	// @cmember SetErrorInfo, then count
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCNoErrorRecords)
#define THE_CLASS TCNoErrorRecords
BEG_TEST_CASE(TCNoErrorRecords, CErrorRecords, L"No Error Records set")
	TEST_VARIATION(1, 		L"IErrorRecords::GetRecordCount")
	TEST_VARIATION(2, 		L"IErrorRecords::GetErrorParameters")
	TEST_VARIATION(3, 		L"IErrorRecords::GetErrorInfo")
	TEST_VARIATION(4, 		L"IErrorRecords::GetCustomErrorObject")
	TEST_VARIATION(5, 		L"IErrorRecords::GetBasicErrorInfo")
	TEST_VARIATION(6, 		L"SetErrorInfo, then count")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAddErrorRecords)
//--------------------------------------------------------------------
// @class Setting Error Records
//
class TCAddErrorRecords : public CErrorRecords { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAddErrorRecords,CErrorRecords);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Set one error record
	int Variation_1();
	// @cmember Set 1000 error records
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCAddErrorRecords)
#define THE_CLASS TCAddErrorRecords
BEG_TEST_CASE(TCAddErrorRecords, CErrorRecords, L"Setting Error Records")
	TEST_VARIATION(1, 		L"Set one error record")
	TEST_VARIATION(2, 		L"Set 1000 error records")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMultipleErrorProviders)
//--------------------------------------------------------------------
// @class Acts as mutliple providers, adding records to one object
//
class TCMultipleErrorProviders : public CErrorRecords { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMultipleErrorProviders,CErrorRecords);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get object and add more records
	int Variation_1();
	// @cmember Reset error object, verify one can't be retrieved
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCMultipleErrorProviders)
#define THE_CLASS TCMultipleErrorProviders
BEG_TEST_CASE(TCMultipleErrorProviders, CErrorRecords, L"Acts as mutliple providers, adding records to one object")
	TEST_VARIATION(1, 		L"Get object and add more records")
	TEST_VARIATION(2, 		L"Reset error object, verify one can't be retrieved")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCErrorInfo)
//--------------------------------------------------------------------
// @class API level tests for IErrorInfo and implicitly IErrorLookup
//
class TCErrorInfo : public CErrorRecords { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	//@cmember Count of error records on current error object
	ULONG			m_cRecords;
	//@cmember Array of IErrorInfo's for each record
	IErrorInfo **	m_rgIErrorInfo;
	//@cmember Index for m_rgIErrorInfo array
	ULONG			m_Index;
	//@cmember Pointer to string to log 
	LPWSTR			m_wszString;
	//@cmember Pointer to string to log 
	LPWSTR			m_wszString2;


public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorInfo,CErrorRecords);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetDescription - valid
	int Variation_1();
	// @cmember E_INVALIDARG - GetDescription, pbstrDescription = NULL
	int Variation_2();
	// @cmember GetGUID - valid
	int Variation_3();
	// @cmember E_INVALIDARG - GetGUID, pguid = NULL
	int Variation_4();
	// @cmember GetHelpContext - valid
	int Variation_5();
	// @cmember E_INVALIDARG - GetHelpContext, pdwHelpContext = NULL
	int Variation_6();
	// @cmember GetHelpFile - valid
	int Variation_7();
	// @cmember E_INVALIDARG - GetHelpFile, pbstrHelpFile = NULL
	int Variation_8();
	// @cmember GetSource - valid
	int Variation_9();
	// @cmember E_INVALIDARG - GetSource, pbstrSource = NULL
	int Variation_10();
	// @cmember GetDescription - record zero
	int Variation_11();
	// @cmember GetGUID - record zero
	int Variation_12();
	// @cmember GetHelpContext - record zero
	int Variation_13();
	// @cmember GetHelpFile - record zero
	int Variation_14();
	// @cmember GetSource - record zero
	int Variation_15();
	// @cmember GetDescription with invalid LCID - DB_E_NOLOCALE
	int Variation_16();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorInfo)
#define THE_CLASS TCErrorInfo
BEG_TEST_CASE(TCErrorInfo, CErrorRecords, L"API level tests for IErrorInfo and implicitly IErrorLookup")
	TEST_VARIATION(1, 		L"GetDescription - valid")
	TEST_VARIATION(2, 		L"E_INVALIDARG - GetDescription, pbstrDescription = NULL")
	TEST_VARIATION(3, 		L"GetGUID - valid")
	TEST_VARIATION(4, 		L"E_INVALIDARG - GetGUID, pguid = NULL")
	TEST_VARIATION(5, 		L"GetHelpContext - valid")
	TEST_VARIATION(6, 		L"E_INVALIDARG - GetHelpContext, pdwHelpContext = NULL")
	TEST_VARIATION(7, 		L"GetHelpFile - valid")
	TEST_VARIATION(8, 		L"E_INVALIDARG - GetHelpFile, pbstrHelpFile = NULL")
	TEST_VARIATION(9, 		L"GetSource - valid")
	TEST_VARIATION(10, 		L"E_INVALIDARG - GetSource, pbstrSource = NULL")
	TEST_VARIATION(11, 		L"GetDescription - record zero")
	TEST_VARIATION(12, 		L"GetGUID - record zero")
	TEST_VARIATION(13, 		L"GetHelpContext - record zero")
	TEST_VARIATION(14, 		L"GetHelpFile - record zero")
	TEST_VARIATION(15, 		L"GetSource - record zero")
	TEST_VARIATION(16, 		L"GetDescription with invalid LCID - DB_E_NOLOCALE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CDataSourceObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorDSO)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all DSO Interfaces
//
class TCErrorDSO : public CDataSourceObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
		
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorDSO,CDataSourceObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorDSO)
#define THE_CLASS TCErrorDSO
BEG_TEST_CASE(TCErrorDSO, CDataSourceObject, L"Checks ISupportErrorInfo for all DSO Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CSessionObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorDBSession)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all DB Session Interfaces
//
class TCErrorDBSession : public CSessionObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorDBSession,CSessionObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorDBSession)
#define THE_CLASS TCErrorDBSession
BEG_TEST_CASE(TCErrorDBSession, CSessionObject, L"Checks ISupportErrorInfo for all DB Session Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CCommandObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorCommand)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all Command Interfaces
//
class TCErrorCommand : public CCommandObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorCommand,CCommandObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorCommand)
#define THE_CLASS TCErrorCommand
BEG_TEST_CASE(TCErrorCommand, CCommandObject, L"Checks ISupportErrorInfo for all Command Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CRowsetObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorRowset)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all Rowset Interfaces
//
class TCErrorRowset : public CRowsetObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorRowset,CRowsetObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorRowset)
#define THE_CLASS TCErrorRowset
BEG_TEST_CASE(TCErrorRowset, CRowsetObject, L"Checks ISupportErrorInfo for all Rowset Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CRowsetObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorSimpleRowset)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all Simple Rowset Interfaces
//
class TCErrorSimpleRowset : public CRowsetObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorSimpleRowset,CRowsetObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorSimpleRowset)
#define THE_CLASS TCErrorSimpleRowset
BEG_TEST_CASE(TCErrorSimpleRowset, CRowsetObject, L"Checks ISupportErrorInfo for all Simple Rowset Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorDSO : public CRowsetObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorIOpenRowset)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all IOpenRowset Interfaces
//
class TCErrorIOpenRowset : public CRowsetObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorIOpenRowset,CRowsetObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorIOpenRowset)
#define THE_CLASS TCErrorIOpenRowset
BEG_TEST_CASE(TCErrorIOpenRowset, CRowsetObject, L"Checks ISupportErrorInfo for all IOpenRowset Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//  class TCErrorIRowRowset : public CRowsetObject, CSupportError {

// {{ TCW_TEST_CASE_MAP(TCErrorIRowRowset)
//--------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all IRow Interfaces
//
class TCErrorIRowRowset : public CRowsetObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorIRowRowset,CRowsetObject);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCErrorIRowRowset)
#define THE_CLASS TCErrorIRowRowset
BEG_TEST_CASE(TCErrorIRowRowset, CRowsetObject, L"Checks ISupportErrorInfo for all IRow Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//  NOTE: This class should inherit from CSupportError
//  but the test wizard will overwrite this.
//  The class declaration should look like:
//	class TCErrorBinder : public CDataSourceObject, CSupportError { 

// {{ TCW_TEST_CASE_MAP(TCErrorBinder)
//*-----------------------------------------------------------------------
// @class Checks ISupportErrorInfo for all Binder Interfaces
//
class TCErrorBinder : public CDataSourceObject, CSupportError { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCErrorBinder,CDataSourceObject);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Log all interfaces and their error support
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCErrorBinder)
#define THE_CLASS TCErrorBinder
BEG_TEST_CASE(TCErrorBinder, CDataSourceObject, L"Checks ISupportErrorInfo for all Binder Interfaces")
	TEST_VARIATION(1, 		L"Log all interfaces and their error support")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(14, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCErrorRecords)
	TEST_CASE(2, TCErrorLookup)
	TEST_CASE(3, TCNoErrorRecords)
	TEST_CASE(4, TCAddErrorRecords)
	TEST_CASE(5, TCMultipleErrorProviders)
	TEST_CASE(6, TCErrorInfo)
	TEST_CASE(7, TCErrorDSO)
	TEST_CASE(8, TCErrorDBSession)
	TEST_CASE(9, TCErrorCommand)
	TEST_CASE(10, TCErrorRowset)
	TEST_CASE(11, TCErrorSimpleRowset)
	TEST_CASE(12, TCErrorIOpenRowset)
	TEST_CASE(13, TCErrorIRowRowset)
	TEST_CASE(14, TCErrorBinder)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END



//--------------------------------------------------------------------
// @mfunc Base Class Initialization Routine, sets up records to use for Add
//
// @rdesc TRUE or FALSE
//
BOOL	CErrorRecords::Init()
{
	//Create an interesting mix of bogus error info
	m_rgErrorInfo[0].hrError = DB_S_ENDOFROWSET;
	m_rgErrorInfo[0].dwMinor = 0;
	m_rgErrorInfo[0].clsid	 = CLSID_TESTMODULE;
	m_rgErrorInfo[0].iid	 = IID_IRowset;
	m_rgErrorInfo[0].dispid  = DISPID_VALUE;

	m_rgErrorInfo[1].hrError = DB_E_BADBINDINFO;
	m_rgErrorInfo[1].dwMinor = 1;
	m_rgErrorInfo[1].clsid	 = CLSID_TESTMODULE;
	m_rgErrorInfo[1].iid	 = IID_ICommand;
	m_rgErrorInfo[1].dispid  = DISPID_VALUE;

	m_rgErrorInfo[2].hrError = E_INVALIDARG;
	m_rgErrorInfo[2].dwMinor = 2;
	m_rgErrorInfo[2].clsid	 = CLSID_TESTMODULE;
	m_rgErrorInfo[2].iid	 = IID_IRowsetIndex;
	m_rgErrorInfo[2].dispid  = DISPID_VALUE;

	m_rgErrorInfo[3].hrError = DB_S_BOOKMARKSKIPPED;
	m_rgErrorInfo[3].dwMinor = 3;
	m_rgErrorInfo[3].clsid	 = CLSID_TESTMODULE;
	m_rgErrorInfo[3].iid	 = IID_IRowsetLocate;
	m_rgErrorInfo[3].dispid  = DISPID_VALUE;

	m_rgErrorInfo[4].hrError = S_OK;
	m_rgErrorInfo[4].dwMinor = MAX_ULONG;
	m_rgErrorInfo[4].clsid	 = GUID_NULL;
	m_rgErrorInfo[4].iid	 = IID_IUnknown;
	m_rgErrorInfo[4].dispid  = DISPID_VALUE;

	return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Uses m_rgErrorInfo[ulErrorInfoIndex], m_pObject, and m_DispParams 
// as basis for verification of error record ulRecordNum's info
// Uses CHECK and COMPARE macros to increment error count appropriately,
// thus there is no need for a return code
// @rdesc TRUE or FALSE
//
void CErrorRecords::VerifyErrorRecord(
			ULONG ulRecordNum,		//@parm Zero based record number to check
			ULONG ulErrorInfoIndex	//@parm Index of m_rgErrorInfo to use for 
			)						//verification of ErrorInfo struct returned
{
	DISPPARAMS	DispParam;
	ERRORINFO	ErrorInfo;
	IUnknown *  pObject		= NULL;
	IErrorInfo* pIErrorInfo = NULL;

	if (CHECK(m_pIErrorRecords->GetErrorParameters(ulRecordNum, 
		&DispParam), S_OK))	
	{
		//Make sure struct was filled correctly
		//with what the error record should contain		
		CompareDispParams(&DispParam, &m_DispParam);
		PROVIDER_FREE(DispParam.rgvarg);
		PROVIDER_FREE(DispParam.rgdispidNamedArgs);
	}
	
	if(CHECK(m_pIErrorRecords->GetBasicErrorInfo(ulRecordNum, &ErrorInfo), S_OK))
	{
		COMPARE(ErrorInfo.hrError, m_rgErrorInfo[ulErrorInfoIndex].hrError);
		COMPARE(ErrorInfo.dwMinor, m_rgErrorInfo[ulErrorInfoIndex].dwMinor);
		COMPARE(ErrorInfo.clsid, m_rgErrorInfo[ulErrorInfoIndex].clsid);
		COMPARE(ErrorInfo.iid, m_rgErrorInfo[ulErrorInfoIndex].iid);
		COMPARE(ErrorInfo.dispid, m_rgErrorInfo[ulErrorInfoIndex].dispid);
	}
			
	if (CHECK(m_pIErrorRecords->GetCustomErrorObject(ulRecordNum, IID_IUnknown, &pObject), S_OK))
	{
		COMPARE(pObject, m_pObject);
		SAFE_RELEASE(pObject);
	}

	if (CHECK(m_pIErrorRecords->GetErrorInfo(ulRecordNum, m_lcid, &pIErrorInfo), S_OK))
		SAFE_RELEASE(pIErrorInfo);
}


//--------------------------------------------------------------------
// @mfunc Setup an IErrorInfo
// @rdesc TRUE or FALSE
//
BOOL CErrorRecords::CreateIErrorInfo()
{
	//Note:  Ref counting takes care of deleting this
	//object as long as we call release the right number
	//of times (and so does the provider)		
	m_pObject = (CCustomObj *)new CCustomObj();
	if (!m_pObject)
		goto FAILED;

	//Init pIErrorInfo so we know when it has been NULL'ed 
	m_pIErrorInfo = (IErrorInfo *)0x12345678;

	//Emulate the provider by first checking if an error object exists
	//There shouldn't be one, and S_FALSE should be returned
	if (!CHECK(GetErrorInfo(0, &m_pIErrorInfo), S_FALSE))
		goto FAILED;
	
	COMPARE(m_pIErrorInfo, NULL);
		
	//Create a new object, asking for IErrorInfo instead of
	//IErrorRecords just to be different from the other test cases
	if (!CHECK(CoCreateInstance(m_Clsid, NULL, CLSCTX_INPROC_SERVER, IID_IErrorInfo,
		(void **)&m_pIErrorInfo), S_OK))		
		goto FAILED;
	
	if (FAILED(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, (void **)&m_pIErrorRecords)))
		goto FAILED;
	
	//Add one record
	if (CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[0], IDENTIFIER_SDK_ERROR,
		&m_DispParam, m_pObject, 0), S_OK))		
	{
		//Release here like a provider would
		m_pObject->Release();
		m_ulRecordNum++;
	}
	else
		goto FAILED;

	//Turn our pointer into OLE Automation
	if (FAILED(SetErrorInfo(0, m_pIErrorInfo)))
		goto FAILED;
	
	//Release all our claims to this object, OLE should still have a ref count on it
	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);
	return TRUE;

FAILED:

	//Release all our claims to this object, OLE should still have a ref count on it
	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc Records the components CLSID to be tested in this test case,
// and uses it to find the CLSID for Extended Error Lookup.
//
CLSID CErrorTest::GetExtendedErrorsLookupCLSID()
{
	CLSID	ExtErrCLSID = GUID_NULL;
	WCHAR*	wszClsid = NULL;
	
	WCHAR	wszSubKey[256];	
	WCHAR	wszSubKeyName[256];	
	
	//Get string form of provider class ID
	if(wszClsid = m_pThisTestModule->m_pwszProviderName)
	{				
		wcscpy(wszSubKeyName, L" ErrorLookup");
	   	wcscpy(wszSubKey, wszClsid);
		wcscat(wszSubKey, wszSubKeyName);
		CLSIDFromProgID(wszSubKey, &ExtErrCLSID);
	}
	
	return ExtErrCLSID;
}

//--------------------------------------------------------------------
// @mfunc Compares the elements of DISPPARAM sturcts and
// increments error count if they are not the same
//
void	CErrorTest::CompareDispParams(DISPPARAMS * param1, DISPPARAMS * param2)
{
	ULONG i = 0;

	//Check if arrays are NULL for rgvarg
	if (param1->rgvarg == NULL || param2->rgvarg == NULL)
	{
		COMPARE(param1->rgvarg, param2->rgvarg);
	}	
	else //Compare each element of the array	
	{	
		//This function does the variant compare for us
		for (i=0; i < param1->cArgs; i++)	
			COMPARE(CompareVariant(&(param1->rgvarg[i]), &(param2->rgvarg[i])), TRUE);			
	}
	
	//Check if arrays are NULL for rgdispidNamedArgs
	if (param1->rgdispidNamedArgs == NULL || param2->rgdispidNamedArgs == NULL)
	{
		COMPARE(param1->rgdispidNamedArgs, param2->rgdispidNamedArgs);
	}
	else //Compare each element of the array
	{
		//Make sure the disppids are the same
		for (i=0; i < param1->cNamedArgs; i++)	
			COMPARE(param1->rgdispidNamedArgs[i], param2->rgdispidNamedArgs[i]);
	}
	
	//Now compare the other two members of the struct
	COMPARE(param1->cArgs, param2->cArgs);
	COMPARE(param1->cNamedArgs, param2->cNamedArgs);						
}


//--------------------------------------------------------------------
// @mfunc Init Support error base class
//
// @rdesc 
//
BOOL	CSupportError::Init(BOOL fOnlyMandatory)
{
	ULONG		i,j		  = 0;
	IUnknown *	pIUnknown = NULL;	
	BOOL		fResults  = FALSE;

	//If our interface to test isn't supported, bail immediately
	TESTC_(m_pIUnkObject->QueryInterface(IID_ISupportErrorInfo,
										(void **)&m_pISupportErrorInfo), S_OK);

	//Populate array containing error support info for all supported interfaces
	for (i=0, j=0; i < m_cInterfaces; i++)
	{
		// Only ask for Mandatory interfaces
		if( fOnlyMandatory && !m_rgInterfaces[i].fMandatory )
			continue;

		// Initialize to NULL
		m_rgSupportedInterfaces[j].pIUnknown = NULL;
		m_rgSupportedInterfaces[j].fSupportsErrors = FALSE;				

		//We will ignore which interfaces are supposed to be mandatory/optional 
		//in this testcase and assume what is returned is correct
		if( VerifyInterface(m_pIUnkObject, *m_rgInterfaces[i].pIID, 
							m_rgInterfaces[i].eInterface, &pIUnknown) )
		{
			m_rgSupportedInterfaces[j].pIUnknown = pIUnknown;
			m_rgSupportedInterfaces[j].iid = *m_rgInterfaces[i].pIID;				
			wcscpy(m_rgSupportedInterfaces[j].wszName, m_rgInterfaces[i].pwszName);
			m_cSupportedInterfaces++;

			//Determine error support
			if( m_pISupportErrorInfo->InterfaceSupportsErrorInfo(m_rgSupportedInterfaces[j].iid) == S_OK )
				m_rgSupportedInterfaces[j].fSupportsErrors = TRUE;				
			else
			{
				//Make sure the right fail code is returned
				TESTC_(m_pISupportErrorInfo->InterfaceSupportsErrorInfo(m_rgSupportedInterfaces[j].iid), S_FALSE);
			}
			
			j++;			
		}		
	}

	m_cSupportedInterfaces = j;		
	fResults = TRUE;

CLEANUP:

	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Terminate Support error base class
//
// @rdesc 
//
BOOL	CSupportError::Terminate()
{
	ULONG i;
	
	//Release our array of interfaces
	for (i=0; i < m_cSupportedInterfaces; i++)
	{
		if ((IUnknown *)m_rgSupportedInterfaces[i].pIUnknown)
			SAFE_RELEASE((IUnknown *)m_rgSupportedInterfaces[i].pIUnknown);
	}

	//We know the ref count should be 1 since 
	//ReleaseObject should not have yet been called
	if (m_pISupportErrorInfo)
		m_pISupportErrorInfo->Release();

	return TRUE;
}	

//--------------------------------------------------------------------
// @mfunc Logs all supported errors and their error support.
//
//
BOOL CSupportError::LogErrorSupport()
{

	ULONG	i;
	
	//We don't have any arrays set up for us by derived class
	if (!m_rgSupportedInterfaces || !m_cSupportedInterfaces)
		return FALSE;

	//Make the output look nice, and call attention to the
	//fact that these need to be manually verified	
	odtLog << wszVerifyTheseResultsManually;

	//Print the interfaces which support errors
	odtLog << wszErrorSupportedInterfaces;

	for (i=0; i < m_cSupportedInterfaces; i++)	
		if (m_rgSupportedInterfaces[i].fSupportsErrors)		
			odtLog <<L"     "<< m_rgSupportedInterfaces[i].wszName << ENDL;

	//Print the interfaces which do not support erros
	odtLog << wszErrorNotSupportedInterfaces;

	for (i=0; i < m_cSupportedInterfaces; i++)	
		if (!m_rgSupportedInterfaces[i].fSupportsErrors)		
			odtLog <<L"     "<< m_rgSupportedInterfaces[i].wszName << ENDL;

	//End of printed output
	odtLog << wszStar;	

	return TRUE;
}

// {{ TCW_TC_PROTOTYPE(TCErrorRecords)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorRecords - API level tests for IErrorRecords
//|	Created:		02/14/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorRecords::Init()
{				
	ULONG	i;	//Loop index

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorRecords::Init())
	// }}
	{				
		//Note:  Ref counting takes care of deleting this
		//object as long as we call release the right number
		//of times (and so does the provider)
		m_pObject = new CCustomObj();

		if (!m_pObject)
			return FALSE;		

		//AddRef the object so we can check the release
		//count when we release it for the last time.  Other
		//wise error object owns it and calls release the
		//last time
		m_pObject->AddRef();
			
		//Create our error object
		if (CHECK(CoCreateInstance(m_Clsid, NULL, 
			CLSCTX_INPROC_SERVER, IID_IErrorRecords,
			(void **)&m_pIErrorRecords), S_OK))		
		{
			////////////////////////////////////////////
			//Now add some fake error records.  Note the
			//record 0 is the most recently added record.
			////////////////////////////////////////////
			m_ulRecordNum = 5;
			
			//RECORD 4: Param struct which is empty
			CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[4], IDENTIFIER_SDK_ERROR,
				&m_NullDispParam, NULL, 0), S_OK);

			//Non record:  Make sure an invalid record returns the right
			//code and does not screw up the record count
			CHECK(m_pIErrorRecords->AddErrorRecord(NULL, IDENTIFIER_SDK_ERROR,
				NULL, NULL, 0), E_INVALIDARG);

			//RECORD 3: Custom error object and params
			CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[3], IDENTIFIER_SDK_ERROR,
				&m_DispParam, m_pObject, 0), S_OK);

			//Release here like the provider is supposed to so we can check
			//the ref count, although we still maintain a ref count from
			//the AddRef we did explicitly, so we will get the last call to release
			m_pObject->Release();

			//RECORD 2: Params
			CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[2],IDENTIFIER_SDK_ERROR,
				&m_DispParam, NULL, 0), S_OK);

			//RECORD 1: Custom error object
			//NOTE:  Since we used the same custom error object
			//and only had one count on it ourselves, we don't
			//release it again
			CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[1], IDENTIFIER_SDK_ERROR,
				NULL, m_pObject, 0), S_OK);				

			//RECORD 0: No params or custom error object
			CHECK(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[0], IDENTIFIER_SDK_ERROR,
				NULL, NULL,0), S_OK);

			//Regression test:
			//Make sure we can get all these records at this point,
			//before any other interfaces are retrieved or anything is
			//done to the error object
			for (i = 0; i<m_ulRecordNum; i++)
			{
				CHECK(m_pIErrorRecords->GetErrorInfo(i, m_lcid, &m_pIErrorInfo), S_OK);
				SAFE_RELEASE(m_pIErrorInfo);
			}
		}
		
		return TRUE;
	}

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - AddErrorRecord, pErrorInfo = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIErrorRecords->AddErrorRecord(NULL, IDENTIFIER_SDK_ERROR, 
									&m_DispParam, m_pObject, 0), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetBasicErrorInfo, pErrorInfo = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIErrorRecords->GetBasicErrorInfo(1, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADRECORDNUM - GetBasicErrorInfo, bad ulRecordNum
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_3()
{
	TBEGIN;

	ERRORINFO	GetErrorInfo;

	//Init struct
	GetErrorInfo.hrError = DB_S_BOOKMARKSKIPPED;
	GetErrorInfo.dwMinor = 3;
	GetErrorInfo.clsid = CLSID_TESTMODULE;
	GetErrorInfo.iid = IID_IRowsetLocate;
	GetErrorInfo.dispid = DISPID_VALUE;

	//Get a record one greater than exists
	TESTC_(m_pIErrorRecords->GetBasicErrorInfo(m_ulRecordNum, 
											&GetErrorInfo), DB_E_BADRECORDNUM);

	//Verify struct wasn't touched
	TESTC(GetErrorInfo.hrError == DB_S_BOOKMARKSKIPPED);
	TESTC(GetErrorInfo.dwMinor == 3);		
	TESTC(GetErrorInfo.clsid == CLSID_TESTMODULE);
	TESTC(GetErrorInfo.iid == IID_IRowsetLocate);
	TESTC(GetErrorInfo.dispid == DISPID_VALUE);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetCustomErrorObject, NULL *ppObject for no custom error object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_4()
{
	TBEGIN;
	IUnknown * pObject = INVALID(IUnknown *);

	//Try to get a custom error object on a record which doesn't have one
	TESTC_(m_pIErrorRecords->GetCustomErrorObject(2, IID_IUnknown, &pObject), S_OK);

	//Make sure NULL is returned to us
	TESTC(pObject == NULL);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetCustomErrorObject, ppObject = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_5()
{
	TBEGIN;

	//Try to get a custom error object which exists, but pass NULL for ppObject
	TESTC_(m_pIErrorRecords->GetCustomErrorObject(1, IID_IUnknown, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADRECORDNUM - GetCustomErrorObject, bad ulRecordNum
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_6()
{
	TBEGIN;
	IUnknown * pObject = INVALID(IUnknown *);

	//Try to get a custom error object for a bad record number
	TESTC_(m_pIErrorRecords->GetCustomErrorObject(m_ulRecordNum, 
								IID_IUnknown, &pObject), DB_E_BADRECORDNUM);

	//Make sure the pObject was NULLed out on error
	TESTC(pObject == NULL);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetErrorInfo, ppErrorInfo = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_7()
{			
	TBEGIN;

	TESTC_(m_pIErrorRecords->GetErrorInfo(0, m_lcid, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADRECORDNUM - GetErrorInfo, bad ulRecordNum
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_8()
{
	TBEGIN;
	IErrorInfo * pIErrorInfo = INVALID(IErrorInfo *);

	//Get an error record which doesn't exist
	TESTC_(m_pIErrorRecords->GetErrorInfo(m_ulRecordNum, 
							m_lcid, &pIErrorInfo), DB_E_BADRECORDNUM);

	//Make sure the memory is NULLed out on error
	TESTC(pIErrorInfo == NULL);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetErrorParameters, pdispparams = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_9()
{
	TBEGIN;

	//Try getting an error record which has parameters, with a NULL ptr
	TESTC_(m_pIErrorRecords->GetErrorParameters(2, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADRECORDNUM - GetErrorParameters, bad ulRecordNum
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_10()
{
	TBEGIN;

	//Try to get parameters from an error record which doesn't exist
	TESTC_(m_pIErrorRecords->GetErrorParameters(m_ulRecordNum, 
								&m_NullDispParam), DB_E_BADRECORDNUM);

	//Make sure struct wasn't touched and still
	//contains the values with which we initialized it		
	TESTC(m_NullDispParam.rgvarg == NULL);
	TESTC(m_NullDispParam.rgdispidNamedArgs == NULL);
	TESTC(m_NullDispParam.cArgs == 0);
	TESTC(m_NullDispParam.cNamedArgs == 0);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetRecordCount, pcRecords = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_11()
{
	TBEGIN;

	TESTC_(m_pIErrorRecords->GetRecordCount(NULL), E_INVALIDARG);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc GetCustomErrorObject - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_12()
{
	TBEGIN;
	IUnknown * pObject = INVALID(IUnknown *);

	//Try to get a custom error object for a record which has one
	TESTC_(m_pIErrorRecords->GetCustomErrorObject(3, IID_IUnknown, &pObject), S_OK);

	//Make sure the pObject is correct
	TESTC(pObject == m_pObject);

	//Now release, note we did AddRef in Init, 
	//and we've added this object twice, so count is 3
	//count in Init
	SAFE_RELEASE(pObject);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc GetErrorInfo - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_13()
{
	TBEGIN;

	BOOL		fResults	= TRUE;
	IUnknown *	pObject		= NULL;
	ULONG		cRecords	= 0;
	DISPPARAMS	DispParam;
	BSTR		bstr;

	ERRORINFO		ErrorInfo;
	IErrorInfo *	pIErrorInfo		= NULL;
	IErrorInfo *	pIErrorInfo2	= NULL;
	IErrorRecords * pIErrorRecords	= NULL;

	TESTC_(m_pIErrorRecords->GetErrorInfo(0, m_lcid, &pIErrorInfo), S_OK);

	//This is the special zeroth record which also
	//contains the IErrorRecords info		
	TESTC_(pIErrorInfo->QueryInterface(IID_IErrorRecords, (void **)&pIErrorRecords), S_OK);

	TESTC_(pIErrorRecords->GetBasicErrorInfo(1, &ErrorInfo), S_OK);
	TESTC(ErrorInfo.hrError == m_rgErrorInfo[1].hrError);
	TESTC(ErrorInfo.dwMinor == m_rgErrorInfo[1].dwMinor);
	TESTC(ErrorInfo.clsid == m_rgErrorInfo[1].clsid);
	TESTC(ErrorInfo.iid == m_rgErrorInfo[1].iid);
	TESTC(ErrorInfo.dispid == m_rgErrorInfo[1].dispid);
					
	TESTC_(pIErrorRecords->GetCustomErrorObject(3, IID_IUnknown, &pObject), S_OK);
	TESTC(pObject == m_pObject);
	SAFE_RELEASE(pObject);

	TESTC_(pIErrorRecords->GetErrorInfo(4, m_lcid, &pIErrorInfo2), S_OK);	
	TESTC_(pIErrorInfo2->GetDescription(&bstr), S_OK);	
	SAFE_RELEASE_(pIErrorInfo2);
	SYSSTRING_FREE(bstr);
					
	TESTC_(pIErrorRecords->GetErrorParameters(2, &DispParam), S_OK);
	CompareDispParams(&DispParam, &m_DispParam);

	//Free the memory provider allocated for parameter arrays
	PROVIDER_FREE(DispParam.rgvarg);
	PROVIDER_FREE(DispParam.rgdispidNamedArgs);

	TESTC_(pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == m_ulRecordNum);

	//Release, expecting a ref count for pIErrorInfo and m_pIErrorInfo
	SAFE_RELEASE(pIErrorRecords);
	SAFE_RELEASE(pIErrorInfo);

	//Try a non special zeroth record
	TESTC_(m_pIErrorRecords->GetErrorInfo(1, m_lcid, &pIErrorInfo), S_OK);

	//This IErrorInfo is at the record level, thus
	//QI for IErrorRecords should fail
	//Make sure it was NULLed out
	TESTC_(pIErrorInfo->QueryInterface(IID_IErrorRecords, (void **)&pIErrorRecords), E_NOINTERFACE);
	TESTC(pIErrorRecords == NULL);
	SAFE_RELEASE_(pIErrorInfo);
	
CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc GetErrorParameters - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_14()
{
	TBEGIN;
	
	DISPPARAMS DispParam;
	DISPPARAMS DispParam2;

	//Init to NULL, to verify the values get changed
	//and to also make sure NULL variants are acceptable
	//as the spec doesn't require them to be validly initialized
	DispParam.rgvarg = NULL;
	DispParam.rgdispidNamedArgs = NULL;
	DispParam.cArgs = 1;	//Should ignore these anyway
	DispParam.cNamedArgs = 3;

	//Also set up a struct which has all valid values,
	//to make sure this is allowed as well
	DISPID		DispId;
	VARIANTARG	vArg;
	vArg.vt = VT_BOOL;
	V_BOOL(&(vArg)) = VARIANT_TRUE;
	DispId = DISPID_VALUE;
	DispParam2.rgvarg = &vArg;
	DispParam2.rgdispidNamedArgs = &DispId;
	DispParam2.cArgs = 1;	
	DispParam2.cNamedArgs = 1;

	//Try to get parameters from an error record which has them,
	//starting with a partially uninitialized output buffer
	TESTC_(m_pIErrorRecords->GetErrorParameters(2, &DispParam), S_OK);

	//Make sure struct was filled correctly
	//with what the error record should contain		
	CompareDispParams(&DispParam, &m_DispParam);
	
	//Free memory allocated for parameter arrays by provider
	PROVIDER_FREE(DispParam.rgvarg);
	PROVIDER_FREE(DispParam.rgdispidNamedArgs);

	//Try getting again with initialized output buffer
	TESTC_(m_pIErrorRecords->GetErrorParameters(2, &DispParam2), S_OK);

	//Make sure struct was filled correctly
	//with what the error record should contain
	CompareDispParams(&DispParam2, &m_DispParam);
	
	//Free memory allocated for parameter arrays by provider
	PROVIDER_FREE(DispParam2.rgvarg);
	PROVIDER_FREE(DispParam2.rgdispidNamedArgs);
			
	//Now get the parameters which are all NULL members
	//in a different record
	TESTC_(m_pIErrorRecords->GetErrorParameters(4, &DispParam2), S_OK);

	//Make sure struct was filled correctly
	//with what the error record should contain
	CompareDispParams(&DispParam2, &m_NullDispParam);
	
	//Free memory allocated for parameter arrays by provider
	PROVIDER_FREE(DispParam2.rgvarg);
	PROVIDER_FREE(DispParam2.rgdispidNamedArgs);

CLEANUP:
	TRETURN;
}

// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc GetRecordCount - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRecords::Variation_15()
{
	TBEGIN;
	ULONG	cRecords = 0;

	TESTC_(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == m_ulRecordNum);

CLEANUP:
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
BOOL TCErrorRecords::Terminate()
{
	//Make sure error object released all its counts on
	//this object
	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE_(m_pObject);
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorRecords::Terminate());
}


// {{ TCW_TC_PROTOTYPE(TCErrorLookup)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorLookup - API level tests for IErrorLookup
//|	Created:		02/15/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorLookup::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorTest::Init())
	// }}
	{		
		//Pick values which should be valid in all scenarios
		m_dwMinor = 1;	//We arbitrarily picked this number
		m_hrError = ResultFromScode(E_FAIL);			
		m_pIErrorLookup = NULL;
		m_pIErrorInfo = NULL;
		m_bstrValid = NULL;			
		m_bstrDesc = NULL;			
		m_dwHelpContext = 0;

		// Get the IErrorLookup CLSID
		m_Clsid = GetExtendedErrorsLookupCLSID();

		// Check to see if ErrorsLookup is supported
		if ( m_Clsid == GUID_NULL )
		{
			odtLog <<L"Provider does not support IErrorLookup." << ENDL;
			return TEST_SKIPPED;
		}

		if (CHECK(CoCreateInstance(m_Clsid, NULL, CLSCTX_INPROC_SERVER, IID_IErrorLookup,
			(void **)&m_pIErrorLookup), S_OK))		
			return TRUE;
	}

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetErrorDescription, pbstrSource = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIErrorLookup->GetErrorDescription(m_hrError, m_dwMinor,
					&m_DispParam, m_lcid, NULL, &m_bstrValid), E_INVALIDARG);

	//Make sure it didn't fill anything in the valid bstr
	TESTC(m_bstrValid == 0);

CLEANUP:
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetErrorDescription, pbstrDescription = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIErrorLookup->GetErrorDescription(m_hrError, m_dwMinor,
					&m_DispParam, m_lcid, &m_bstrValid, NULL), E_INVALIDARG);

	//Make sure it didn't fill anything in the valid bstr
	TESTC(m_bstrValid == 0);

CLEANUP:
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetHelpInfo, pbstrHelpFile = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_3()
{
	TBEGIN;

	TESTC_(m_pIErrorLookup->GetHelpInfo(m_hrError, m_dwMinor,
							m_lcid, NULL, &m_dwHelpContext), E_INVALIDARG);

	//Make sure it didn't fill anything in the valid DWORD
	TESTC(m_dwHelpContext == 0);

CLEANUP:
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetHelpInfo, pdwHelpContext = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_4()
{
	TBEGIN;

	TESTC_(m_pIErrorLookup->GetHelpInfo(m_hrError, m_dwMinor,
								m_lcid, &m_bstrValid, NULL), E_INVALIDARG);

	//Make sure it didn't fill anything in the valid bstr
	TESTC(m_bstrValid == 0);

CLEANUP:
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOLOCALE - GetHelpInfo, lcid = DISP_E_UNKNOWNLCID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_5()
{
	TBEGIN;

	TESTC_(m_pIErrorLookup->GetHelpInfo(m_hrError, m_dwMinor,
			DISP_E_UNKNOWNLCID, &m_bstrValid, &m_dwHelpContext), DB_E_NOLOCALE);

	//Make sure it didn't fill anything in the valid bstr
	TESTC(m_bstrValid == 0);
	TESTC(m_dwHelpContext == 0);

CLEANUP:
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADHRESULT - GetErrorDescription, invalid hrError
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_6()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADHRESULT - GetHelpInfo, invalid hrError
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_7()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_8()
{
	return TEST_PASS;
}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADLOOKUPID - GetErrorDescription, invalid dwLookupID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_9()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK - GetErrorDescription, dwLookupID=IDENTIFIER_SDK_ERROR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorLookup::Variation_10()
{
	TBEGIN;

	//IDENTIFIER_SDK_ERROR dwLookupID
	DWORD dwMinor = IDENTIFIER_SDK_ERROR;
							
	TESTC_(m_pIErrorLookup->GetErrorDescription(m_hrError, dwMinor,
					&m_DispParam, m_lcid, &m_bstrValid, &m_bstrDesc), S_OK);

	//Make sure it didn't fill anything in the valid bstr
	SYSSTRING_FREE(m_bstrValid);
	SYSSTRING_FREE(m_bstrDesc);

CLEANUP:
	TRETURN;
}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorLookup::Terminate()
{
	SAFE_RELEASE_(m_pIErrorLookup);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorTest::Terminate());
}


// {{ TCW_TC_PROTOTYPE(TCNoErrorRecords)
//*-----------------------------------------------------------------------
//| Test Case:		TCNoErrorRecords - No Error Records set
//|	Created:		02/16/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNoErrorRecords::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorRecords::Init())
	// }}
	{
		//Get an error object, but don't insert any records. NOTE:  This 
		//test will cover the scenario where a provider calls other
		//methods before AddErrorRecords
		if (CHECK(CoCreateInstance(m_Clsid, NULL, CLSCTX_INPROC_SERVER, IID_IErrorRecords,
			(void **)&m_pIErrorRecords), S_OK))		
			return TRUE;
	}

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IErrorRecords::GetRecordCount
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_1()
{
	TBEGIN;
	ULONG	cRecords = INVALID(ULONG);

	//Make sure count comes back as 0
	TESTC_(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == 0);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IErrorRecords::GetErrorParameters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_2()
{
	TBEGIN;

	//Make sure we don't return anything here
	TESTC_(m_pIErrorRecords->GetErrorParameters(0, &m_NullDispParam), DB_E_BADRECORDNUM);
	TESTC(m_NullDispParam.rgvarg == NULL);
	TESTC(m_NullDispParam.rgdispidNamedArgs == NULL);
	TESTC(m_NullDispParam.cArgs == 0);
	TESTC(m_NullDispParam.cNamedArgs == 0);
			
CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IErrorRecords::GetErrorInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_3()
{
	TBEGIN;

	//Make sure we don't return anything here
	TESTC_(m_pIErrorRecords->GetErrorInfo(0, m_lcid, &m_pIErrorInfo), DB_E_BADRECORDNUM);
	TESTC(m_pIErrorInfo == NULL);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IErrorRecords::GetCustomErrorObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_4()
{
	TBEGIN;

	//Make sure we don't return anything here
	TESTC_(m_pIErrorRecords->GetCustomErrorObject(0, IID_IUnknown, &m_pObject), DB_E_BADRECORDNUM);
	TESTC(m_pObject == NULL);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IErrorRecords::GetBasicErrorInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_5()
{
	TBEGIN;

	ERRORINFO	ErrorInfo;
	ERRORINFO	ErrorInfo2;

	memset(&ErrorInfo, 0, sizeof(ErrorInfo));
	memset(&ErrorInfo2, 0, sizeof(ErrorInfo));

	//Make sure we don't return anything here
	TESTC_(m_pIErrorRecords->GetBasicErrorInfo(0, &ErrorInfo), DB_E_BADRECORDNUM);

	//Make sure the buffer wasn't touched
	TESTC(memcmp(&ErrorInfo, &ErrorInfo2, sizeof(ERRORINFO)) == 0);

CLEANUP:
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc SetErrorInfo, then count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNoErrorRecords::Variation_6()
{
	TBEGIN;

	ULONG	cRecords = 0;
	BOOL	fResults = FALSE;
	
	TESTC_(m_pIErrorRecords->QueryInterface(IID_IErrorInfo, 
											(void **)&m_pIErrorInfo), S_OK);

	//Set the error object with no records 
	TESTC_(SetErrorInfo(0, m_pIErrorInfo), S_OK);

	//Release all control over the object
	SAFE_RELEASE(m_pIErrorInfo);
	SAFE_RELEASE(m_pIErrorRecords);

			//Now get the object back
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_OK);

	//Verify our count is zero
	TESTC_(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, 
										(void **)&m_pIErrorRecords), S_OK);
	
	TESTC_(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == 0);
				
	//Leave the m_pIErrorRecords for other variations, 
	//since that's the state Init gave us
	SAFE_RELEASE(m_pIErrorInfo);

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
BOOL TCNoErrorRecords::Terminate()
{
	SAFE_RELEASE(m_pIErrorRecords);
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorRecords::Terminate());
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCAddErrorRecords)
//*-----------------------------------------------------------------------
//| Test Case:		TCAddErrorRecords - Setting Error Records
//|	Created:		02/16/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddErrorRecords::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorRecords::Init())
	// }}
	{
		//Note:  Ref counting takes care of deleting this
		//object as long as we call release the right number
		//of times (and so does the error object)		
		m_pObject = (CCustomObj *)new CCustomObj();
		if (!m_pObject)
			return FALSE;
		
		//Get IErrorRecords interface
		if (CHECK(CoCreateInstance(m_Clsid, NULL, CLSCTX_INPROC_SERVER, IID_IErrorRecords,
			(void **)&m_pIErrorRecords), S_OK))		
			return TRUE;
	}

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set one error record
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddErrorRecords::Variation_1()
{
	TBEGIN;

	ULONG cRecords = 0;

	//Add one record, and make sure that's what the count returns
	TESTC_(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[0], 
					IDENTIFIER_SDK_ERROR,&m_DispParam, m_pObject, 0), S_OK);
	//Record the total number of records added to this object
	m_ulRecordNum++;

	TESTC_(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == m_ulRecordNum);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set 1000 error records
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddErrorRecords::Variation_2()
{
	TBEGIN;

	ULONG	cRecords = 0;
	ULONG	index = 1;	//Start at one so our total isn't over 1000

	while (index <= 1000)
	{
		TESTC_(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[0], 
						IDENTIFIER_SDK_ERROR,&m_DispParam, m_pObject, 0), S_OK);

		//Record the total number of records added to this object
		m_ulRecordNum++;
		index++;
	}

	//Now count them all
	TESTC_(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecords == m_ulRecordNum);

	//Now do a bunch of stuff on the last record added		
	VerifyErrorRecord(m_ulRecordNum-1, 0);
	
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
BOOL TCAddErrorRecords::Terminate()
{
	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE_(m_pObject);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorRecords::Terminate());
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCMultipleErrorProviders)
//*-----------------------------------------------------------------------
//| Test Case:		TCMultipleErrorProviders - Acts as mutliple providers, adding records to one object
//|	Created:		02/19/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMultipleErrorProviders::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorRecords::Init())
	// }}
		return TRUE;

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get object and add more records
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleErrorProviders::Variation_1()
{
	TBEGIN;

	TESTC(CreateIErrorInfo());

	//Add one record
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_OK);
	TESTC_(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, 
											(void**)&m_pIErrorRecords), S_OK);

	//Don't release custom object here since we did once already in Init				
	TESTC_(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[1], 
						IDENTIFIER_SDK_ERROR,&m_DispParam, m_pObject, 0), S_OK);

	//Return our pointer to OLE Automation
	m_ulRecordNum++;
	TESTC_(SetErrorInfo(0, m_pIErrorInfo), S_OK);

	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);
			
	//Now try to get the object again
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_OK);
	TESTC_(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, 
											(void**)&m_pIErrorRecords), S_OK);

	//Don't release custom object here since we did once already in Init				
	TESTC_(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[2], 
						IDENTIFIER_SDK_ERROR,&m_DispParam, m_pObject, 0), S_OK);

	//Return our pointer to OLE Automation
	m_ulRecordNum++;
	TESTC_(SetErrorInfo(0, m_pIErrorInfo), S_OK);

	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);

	//Now try to get the object once more
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_OK);
	TESTC_(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, 
											(void**)&m_pIErrorRecords), S_OK);
	//Verify everything is correct 
	VerifyErrorRecord(2, 0);
	VerifyErrorRecord(1, 1);
	VerifyErrorRecord(0, 2);
														
	//Add another record
	TESTC_(m_pIErrorRecords->AddErrorRecord(&m_rgErrorInfo[3], 
						IDENTIFIER_SDK_ERROR,&m_DispParam, m_pObject, 0), S_OK);
			
	//Return our pointer to OLE Automation
	m_ulRecordNum++;
	TESTC_(SetErrorInfo(0, m_pIErrorInfo), S_OK);

	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);

	//Finally act like consumer and just verify info
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_OK);
	TESTC_(m_pIErrorInfo->QueryInterface(IID_IErrorRecords, 
											(void**)&m_pIErrorRecords), S_OK);
	//Verify everything is correct 
	VerifyErrorRecord(3, 0);
	VerifyErrorRecord(2, 1);
	VerifyErrorRecord(1, 2);
	VerifyErrorRecord(0, 3);
			
	// Release the custom Object
	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);
	
	// Releasing the IErrorInfo releases m_pObject
	m_pObject = NULL;

CLEANUP:

	// Release the custom Object
	SAFE_RELEASE_(m_pObject);
	SAFE_RELEASE(m_pIErrorInfo);

	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Reset error object, verify one can't be retrieved
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleErrorProviders::Variation_2()
{
	TBEGIN;

	//Note this is mostly an OLE Automation test, but verify
	//it works anyway.
	TESTC(CreateIErrorInfo());

	//Set the error object for this thread to NULL
	TESTC_(SetErrorInfo(0, NULL), S_OK);

	//Init so we know when it has been NULLed
	m_pIErrorInfo = INVALID(IErrorInfo *);

	//Verify we can't get one back
	TESTC_(GetErrorInfo(0, &m_pIErrorInfo), S_FALSE);
	TESTC(m_pIErrorInfo == NULL);

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
BOOL TCMultipleErrorProviders::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorRecords::Terminate());
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorInfo)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorInfo - API level tests for IErrorInfo and implicitly IErrorLookup
//|	Created:		02/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorInfo::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	
	IDBCreateSession *	pIDBCreateSession = NULL;
	
	m_cRecords = 0;			
	m_rgIErrorInfo = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CErrorRecords::Init())
	// }}
	{	
		// Get a DSO			
		if(!CHECK(CreateDataSourceObject(), S_OK))
			goto CLEANUP;

		// Initialize the DSO to get the ISupportErrorInfo Pointer
		if(!CHECK(InitializeDSO(), S_OK))
			goto CLEANUP;

		//Make sure we can get errors for initialize, we'll consider
		//ISupportErrorInfo mandatory for purposes of this test
		if (!VerifyInterface(m_pIDBInitialize, IID_ISupportErrorInfo, DATASOURCE_INTERFACE, 
			(IUnknown **)&pISupportErrorInfo))
		{
			odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
			return TEST_SKIPPED;
		}
		
		m_hr=pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize);
		
		// Check to see if IDBInitialize has error support
		if (m_hr == S_FALSE)
		{
			SAFE_RELEASE(pISupportErrorInfo);
			return TEST_SKIPPED;
		}

		if (!CHECK(m_hr, S_OK))
			goto CLEANUP;

		SAFE_RELEASE(pISupportErrorInfo);

		// Release the DSO
		ReleaseDataSourceObject();

		// Get a DSO			
		if(!CHECK(CreateDataSourceObject(), S_OK))
			goto CLEANUP;

		//Setup our initialization, but use invalid userid and password
		m_hr = m_pIDBInitialize->Initialize();

		// Make sure we didn't succeed
		if SUCCEEDED(m_hr)
		{		
			// Create an Error with QI'ing for IDBCreateSession before Initialization
			if (!VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE,
				(IUnknown **)&pIDBCreateSession))
				goto CLEANUP;

			//Setup our initialization, but use invalid userid and password
			m_hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, NULL);

			// Make sure we didn't succeed
			if SUCCEEDED(m_hr)
			{		
				COMPARE(m_hr, E_INVALIDARG);
				odtLog << L"Error was not generated by CreateSession with a NULL, cannot continue with test case." << ENDL;
				goto CLEANUP;
			}
		}

		//Now get our error object, and find out how many records were created
		if (!CHECK(GetErrorInfo(0, &m_pIErrorInfo), S_OK))
			goto CLEANUP;

		if (!VerifyInterface(m_pIErrorInfo, IID_IErrorRecords, ERROR_INTERFACE,
			(IUnknown **)&m_pIErrorRecords))
			goto CLEANUP;

		if (!CHECK(m_pIErrorRecords->GetRecordCount(&m_cRecords), S_OK))
			goto CLEANUP;

		if (m_cRecords == 0)
		{
			odtLog << L"No error records were found in the error object." << ENDL;
			goto CLEANUP;
		}

		//Now get an IErrorInfo for every record
		m_rgIErrorInfo = (IErrorInfo **)PROVIDER_ALLOC(sizeof(IErrorInfo *) * m_cRecords);
		if (!m_rgIErrorInfo)
			goto CLEANUP;
		
		for (m_Index=0; m_Index<m_cRecords; m_Index++)
		{
			//Place each interface in an element of our array
			CHECK(m_pIErrorRecords->GetErrorInfo(m_Index, m_lcid, 
				(IErrorInfo**)&m_rgIErrorInfo[m_Index]), S_OK);
		}

		//If we got this far, everything succeeded
		return TRUE;
	}

CLEANUP:

	//Cleanup everything not done in Terminate
	SAFE_RELEASE(pISupportErrorInfo);
	SAFE_RELEASE(pIDBCreateSession);

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetDescription - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_1()
{
	TBEGIN;

	BSTR bstrDescription = NULL;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{
		//Subtract one from index since our array index is obviously zero based
		TESTC_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetDescription(&bstrDescription), S_OK);

		//Convert to printable WSTR and log, freeing bstrs
		m_wszString = BSTR2WSTR(bstrDescription, TRUE);
		PROVIDER_FREE(m_wszString);
	}

CLEANUP:

	PROVIDER_FREE(m_wszString);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetDescription, pbstrDescription = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_2()
{	
	TBEGIN;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		//Make sure every record comes back with the right error
		TEST2C_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetDescription(NULL),
					E_INVALIDARG, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER) );
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetGUID - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_3()
{
	TBEGIN;

	GUID guid;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		TESTC_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetGUID(&guid), S_OK);
		//Make sure the right interface was returned
		if (guid != IID_IDBInitialize)
			odtLog <<L"Warning: the GUID does not match the iid of the interface caused the error." << ENDL;
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetGUID, pguid = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_4()
{
	TBEGIN;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		//Make sure every record comes back with the right error
		TEST2C_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetGUID(NULL),
					E_INVALIDARG, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER) );
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetHelpContext - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_5()
{
	TBEGIN;

	DWORD	dwHelpContext = 0;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{
		TESTC_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetHelpContext(&dwHelpContext), S_OK);
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetHelpContext, pdwHelpContext = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_6()
{
	TBEGIN;
	
	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		TEST2C_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetHelpContext(NULL),
					E_INVALIDARG, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER) );
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc GetHelpFile - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_7()
{
	TBEGIN;

	BSTR bstrHelpFile = NULL;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{
		TESTC_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetHelpFile(&bstrHelpFile), S_OK);
		
		//Convert to printable WSTR and log, freeing bstr
		m_wszString = BSTR2WSTR(bstrHelpFile, TRUE);
		PROVIDER_FREE(m_wszString);
	}

CLEANUP:
	
	PROVIDER_FREE(m_wszString);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetHelpFile, pbstrHelpFile = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_8()
{
	TBEGIN;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		TEST2C_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetHelpFile(NULL),
					E_INVALIDARG, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER) );
	}

CLEANUP:
	
	TRETURN;
}	
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc GetSource - valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_9()
{
	TBEGIN;

	BSTR bstrSource = NULL;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		TESTC_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetSource(&bstrSource), S_OK);

		//Convert to printable WSTR and log, freeing bstr
		m_wszString = BSTR2WSTR(bstrSource, TRUE);
		PROVIDER_FREE(m_wszString);
	}

CLEANUP:
	
	PROVIDER_FREE(m_wszString);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - GetSource, pbstrSource = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_10()
{
	TBEGIN;

	//Loop through each error record, in reverse order to make it interesting
	//We use the one based index in loop to allow us to quit when we hit zero
	for (m_Index = m_cRecords; m_Index > 0; m_Index--)
	{		
		//Make sure every record comes back with the right error
		TEST2C_(((IErrorInfo *)m_rgIErrorInfo[m_Index-1])->GetSource(NULL),
					E_INVALIDARG, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER) );
	}

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc GetDescription - record zero
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_11()
{
	TBEGIN;
	
	BSTR bstrDescription = NULL;
	BSTR bstrDescription2 = NULL;

	TESTC_(((IErrorInfo *)m_rgIErrorInfo[0])->GetDescription(&bstrDescription), S_OK);
	TESTC_(m_pIErrorInfo->GetDescription(&bstrDescription2), S_OK);

	//Convert to printable WSTR and log, freeing bstrs
	m_wszString  = BSTR2WSTR(bstrDescription, TRUE);
	m_wszString2 = BSTR2WSTR(bstrDescription2, TRUE);

	TESTC(wcscmp(m_wszString, m_wszString2) == 0);

CLEANUP:
	
	PROVIDER_FREE(m_wszString2);		
	PROVIDER_FREE(m_wszString);
				
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc GetGUID - record zero
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_12()
{
	TBEGIN;

	GUID	guid;
	GUID	guid2;

	TESTC_(((IErrorInfo *)m_rgIErrorInfo[0])->GetGUID(&guid), S_OK);
	TESTC_(m_pIErrorInfo->GetGUID(&guid2), S_OK);

	TESTC(guid == guid2);
				
CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc GetHelpContext - record zero
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_13()
{
	TBEGIN;

	DWORD dwContext;
	DWORD dwContext2;	

	TESTC_(((IErrorInfo *)m_rgIErrorInfo[0])->GetHelpContext(&dwContext), S_OK);	
	TESTC_(m_pIErrorInfo->GetHelpContext(&dwContext2), S_OK);
	
	TESTC(dwContext == dwContext2);
				
CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc GetHelpFile - record zero
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_14()
{
	TBEGIN;

	BSTR bstrFile = NULL;
	BSTR bstrFile2 = NULL;

	TESTC_(((IErrorInfo *)m_rgIErrorInfo[0])->GetHelpFile(&bstrFile), S_OK);
	TESTC_(m_pIErrorInfo->GetHelpFile(&bstrFile2), S_OK);
		
	//Convert to printable WSTR and log, freeing bstrs
	m_wszString  = BSTR2WSTR(bstrFile, TRUE);
	m_wszString2 = BSTR2WSTR(bstrFile2, TRUE);

	TESTC(wcscmp(m_wszString, m_wszString2) == 0);

CLEANUP:
	
	PROVIDER_FREE(m_wszString2);		
	PROVIDER_FREE(m_wszString);
				
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc GetSource - record zero
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_15()
{
	TBEGIN;

	BSTR bstrSource = NULL;
	BSTR bstrSource2 = NULL;

	TESTC_(((IErrorInfo *)m_rgIErrorInfo[0])->GetHelpFile(&bstrSource), S_OK);
	TESTC_(m_pIErrorInfo->GetHelpFile(&bstrSource2), S_OK);

	//Convert to printable WSTR and log, freeing bstrs
	m_wszString  = BSTR2WSTR(bstrSource, TRUE);
	m_wszString2 = BSTR2WSTR(bstrSource2, TRUE);

	TESTC(wcscmp(m_wszString, m_wszString2) == 0);

CLEANUP:
	
	PROVIDER_FREE(m_wszString2);		
	PROVIDER_FREE(m_wszString);
				
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc GetDescription with invalid LCID - DB_E_NOLOCALE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorInfo::Variation_16()
{
	TBEGIN;

	IErrorInfo * pIErrorInfo = NULL;
	BSTR		 bstrDescription = NULL;

	//Place each interface in an element of our array
	TESTC_(m_pIErrorRecords->GetErrorInfo(0, 0, &pIErrorInfo), S_OK);

	//Subtract one from index since our array index is obviously zero based
	TESTC_(pIErrorInfo->GetDescription(&bstrDescription), DB_E_NOLOCALE);
	TESTC(bstrDescription == NULL);

CLEANUP:

	SYSSTRING_FREE(bstrDescription);
	SAFE_RELEASE(pIErrorInfo);
	
	TRETURN;
}
// }}




// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorInfo::Terminate()
{	
	if (m_rgIErrorInfo)
	{
		//Release each records IErrorInfo interface
		for (m_Index=0; m_Index<m_cRecords; m_Index++)
			SAFE_RELEASE((IErrorInfo*)m_rgIErrorInfo[m_Index]);

		//Delete array of interface pointers
		PROVIDER_FREE(m_rgIErrorInfo);
	}

	ReleaseDataSourceObject();

	SAFE_RELEASE(m_pIErrorRecords);
	SAFE_RELEASE(m_pIErrorInfo);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CErrorRecords::Terminate());
}	// }}					
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorDSO)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorDSO - Checks ISupportErrorInfo for all DSO Interfaces
//|	Created:		02/22/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorDSO::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	

	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(DATASOURCE_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CDataSourceObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Initialize the DSO to get the ISupportErrorInfo Pointer
	TESTC_(CreateDataSourceObject(), S_OK);
	TESTC_(InitializeDSO(), S_OK);

	// Assign the session into pIUnkObject
	m_pIUnkObject = m_pIDBInitialize;

	// Make sure we can get errors for initialize, we'll consider
	// ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					DATASOURCE_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	return TRUE;
	
CLEANUP:
	
	return FALSE;
}





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorDSO::Variation_1()
{	
	if( LogErrorSupport() )
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
BOOL TCErrorDSO::Terminate()
{
	// Release
	BOOL fResults = (CDataSourceObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseDataSourceObject();

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorDBSession)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorDBSession - Checks ISupportErrorInfo for all DB Session Interfaces
//|	Created:		02/23/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorDBSession::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	

	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(SESSION_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CSessionObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Get our object and put it in m_pIUnkObject
	TESTC_(CreateDBSession(), S_OK);
	
	// Assign the session into pIUnkObject
	m_pIOpenRowset ? (m_pIUnkObject = m_pIOpenRowset) : 
					 (m_pIUnkObject = m_pIDBCreateCommand);

	// Make sure we can get errors for initialize, we'll consider
	// ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					SESSION_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	return TRUE;
	
CLEANUP:
	
	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorDBSession::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorDBSession::Terminate()
{
	// Release
	BOOL fResults = (CSessionObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseDataSourceObject();
	ReleaseDBSession();

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorCommand)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorCommand - Checks ISupportErrorInfo for all Command Interfaces
//|	Created:		02/23/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorCommand::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	
	
	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(COMMAND_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CCommandObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Get our object and put it in m_pIUnkObject
	if( FAILED(m_hr=CreateCommandObject()) )
	{
		TESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Provider does not support Commands." << ENDL;
		return TEST_SKIPPED;
	}

	// Assign the command into pIUnkObject
	m_pIUnkObject = m_pICommand;

	//Make sure we can get errors for initialize, we'll consider
	//ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					COMMAND_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	return TRUE;
	
CLEANUP:
	
	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorCommand::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorCommand::Terminate()
{
	// Release 
	BOOL fResults = (CCommandObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseCommandObject();	
	ReleaseDataSourceObject();
	ReleaseDBSession();

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;

}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorRowset - Checks ISupportErrorInfo for all Rowset Interfaces
//|	Created:		02/23/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorRowset::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	
	BOOL fResults = FALSE;
	ULONG i;
	DBPROP	* rgProperties = NULL;
	DBPROPSET DBPropSet;
	
	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CRowsetObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Allocate an array for all object interface properties.  
	// Note, after SetRowsetProperties is called, this memory is owned by CRowsetObject.
	// We shouln't free it unless SetRowsetProperties does not get called
	SAFE_ALLOC(rgProperties, DBPROP, m_cInterfaces);
	
	//Set up part of each property struct we'll use to request every interface.
	//for optional interfaces
	for (i=0; i < m_cInterfaces; i++)
	{		
		rgProperties[i].dwPropertyID = m_rgInterfaces[i].dwPropertyID;
		rgProperties[i].dwOptions = DBPROPOPTIONS_OPTIONAL;
		rgProperties[i].colid = DB_NULLID;
		rgProperties[i].vValue.vt = VT_BOOL;
		V_BOOL(&(rgProperties[i].vValue)) = VARIANT_TRUE;
	}

	DBPropSet.rgProperties = rgProperties;
	DBPropSet.cProperties = m_cInterfaces;
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
	
	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	//Get our object and put it in m_pIUnkObject
	TEST2C_(CreateRowsetObject(SELECT_ALLFROMTBL), S_OK, DB_S_ERRORSOCCURRED);

	//Set Required on the supported Properties
	for (i=0; i < m_cInterfaces; i++)
	{
		if( (m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSETTABLE) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSET) )
			rgProperties[i].dwOptions = DBPROPOPTIONS_REQUIRED;
	}
	ReleaseRowsetObject();

	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	//Get our object and put it in m_pIUnkObject
	TEST2C_(CreateRowsetObject(SELECT_ALLFROMTBL), S_OK, DB_S_ERRORSOCCURRED);

	// Assign the rowset into pIUnkObject
	TESTC(m_pIAccessor != NULL);
	m_pIUnkObject = m_pIAccessor;

	//Make sure we can get errors for initialize, we'll consider
	//ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					ROWSET_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		PROVIDER_FREE(rgProperties);
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	//Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	fResults = TRUE;

CLEANUP:
	
	PROVIDER_FREE(rgProperties);
	return fResults;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorRowset::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorRowset::Terminate()
{
	// Release
	BOOL fResults  = (CRowsetObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Drop the table, also deletes the table name
	if(m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;

}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCErrorSimpleRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorSimpleRowset - Checks ISupportErrorInfo for all Simple Rowset Interfaces
//|	Created:		02/29/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorSimpleRowset::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	

	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CRowsetObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Get our object and put it in m_pIUnkObject
	TESTC_(CreateRowsetObject(SELECT_ALLFROMTBL), S_OK);

	// Assign the rowset into pIUnkObject
	TESTC(m_pIAccessor != NULL);
	m_pIUnkObject = m_pIAccessor;

	// Make sure we can get errors for initialize, we'll consider
	// ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					ROWSET_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init(TRUE));
	return TRUE;

CLEANUP:	

	return FALSE;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorSimpleRowset::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorSimpleRowset::Terminate()
{		
	// Release
	BOOL fResults = (CRowsetObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Drop the table, also deletes the table name
	if(m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCErrorIOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorIOpenRowset - Checks ISupportErrorInfo for all IOpenRowset Interfaces
//|	Created:		02/29/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorIOpenRowset::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	
	BOOL fResults = FALSE;
	ULONG i;
	DBPROP	* rgProperties = NULL;
	DBPROPSET DBPropSet;
	
	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CRowsetObject::Init());

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Allocate an array for all object interface properties.  
	// Note, after SetRowsetProperties is called, this memory is owned by CRowsetObject.
	// We shouln't free it unless SetRowsetProperties does not get called
	SAFE_ALLOC(rgProperties, DBPROP, m_cInterfaces);
	
	//Set up part of each property struct we'll use to request every interface.
	//for optional interfaces
	for (i=0; i < m_cInterfaces; i++)
	{		
		rgProperties[i].dwPropertyID = m_rgInterfaces[i].dwPropertyID;
		rgProperties[i].dwOptions = DBPROPOPTIONS_OPTIONAL;
		rgProperties[i].colid = DB_NULLID;
		rgProperties[i].vValue.vt = VT_BOOL;
		V_BOOL(&(rgProperties[i].vValue)) = VARIANT_TRUE;
	}

	DBPropSet.rgProperties = rgProperties;
	DBPropSet.cProperties = m_cInterfaces;
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
	
	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	//Get our object and put it in m_pIUnkObject
	TEST2C_(CreateRowsetObject(USE_OPENROWSET), S_OK, DB_S_ERRORSOCCURRED);

	//Set Required on the supported Properties
	for (i=0; i < m_cInterfaces; i++)
	{
		if( (m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSETTABLE) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSET) )
			rgProperties[i].dwOptions = DBPROPOPTIONS_REQUIRED;
	}
	ReleaseRowsetObject();

	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	//Get our object and put it in m_pIUnkObject
	TEST2C_(CreateRowsetObject(USE_OPENROWSET), S_OK, DB_S_ERRORSOCCURRED);

	// Assign the rowset into pIUnkObject
	TESTC(m_pIAccessor != NULL);
	m_pIUnkObject = m_pIAccessor;

	//Make sure we can get errors for initialize, we'll consider
	//ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					ROWSET_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		PROVIDER_FREE(rgProperties);
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	//Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	fResults = TRUE;

CLEANUP:
	
	PROVIDER_FREE(rgProperties);
	return fResults;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorIOpenRowset::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorIOpenRowset::Terminate()
{
	// Release
	BOOL fResults = (CRowsetObject::Terminate());
	fResults &= (CSupportError::Terminate());
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Drop the table, also deletes the table name
	if(m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCErrorIRowRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorIRowRowset - Checks ISupportErrorInfo for all IRow Interfaces
//|	Created:		11/07/98
//|	Updated:		11/07/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorIRowRowset::Init()
{
	ISupportErrorInfo * pISupportErrorInfo = NULL;	
	BOOL fResults = FALSE;
	ULONG i;
	DBPROP	* rgProperties = NULL;
	DBPROPSET DBPropSet;
	
	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(ROW_INTERFACE, &m_cInterfaces, &m_rgInterfaces));
	TESTC(CRowsetObject::Init());

	// Get our object and put it in m_pIUnkObject
	TESTC_(CreateDBSession(), S_OK);

	// Create a Table
	m_pTable = new CTable(m_pIOpenRowset, m_pwszTestCaseName);
	TESTC(m_pTable != NULL);
	TESTC_(m_pTable->CreateTable(NUM_ROWS), S_OK);

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	// Allocate an array for all object interface properties.  
	// Note, after SetRowsetProperties is called, this memory is owned by CRowsetObject.
	// We shouln't free it unless SetRowsetProperties does not get called
	SAFE_ALLOC(rgProperties, DBPROP, m_cInterfaces);
	
	//Set up part of each property struct we'll use to request every interface.
	//for optional interfaces
	for (i=0; i < m_cInterfaces; i++)
	{		
		rgProperties[i].dwPropertyID = m_rgInterfaces[i].dwPropertyID;
		rgProperties[i].dwOptions = DBPROPOPTIONS_OPTIONAL;
		rgProperties[i].colid = DB_NULLID;
		rgProperties[i].vValue.vt = VT_BOOL;
		V_BOOL(&(rgProperties[i].vValue)) = VARIANT_TRUE;
	}

	DBPropSet.rgProperties = rgProperties;
	DBPropSet.cProperties = m_cInterfaces;
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
	
	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	//Get our object and put it in m_pIUnkObject
	if( FAILED(m_hr=m_pTable->CreateRowset(USE_OPENROWSET, IID_IRow,
								1, m_rgPropSets, (IUnknown**)&m_pIUnkObject)) )
	{
		TESTC_(m_hr, E_NOINTERFACE);
		PROVIDER_FREE(rgProperties);
		odtLog << L"Interface IRow was not supported, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	//Set Required on the supported Properties
	for (i=0; i < m_cInterfaces; i++)
	{
		if( (m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSETTABLE) &&
			(m_rgPropSets->rgProperties[i].dwStatus != DBPROPSTATUS_NOTSET) )
			rgProperties[i].dwOptions = DBPROPOPTIONS_REQUIRED;
	}
	SAFE_RELEASE(m_pIUnkObject);

	//Set all possible properties so we can check
	//all supported interfaces.  We don't care if some
	//of them fail, as not all of them will be supported
	SetRowsetProperties(&DBPropSet, 1);		

	// Get our object and put it in m_pIUnkObject
	TEST3C_(m_pTable->CreateRowset(USE_OPENROWSET, IID_IRow,
				1, m_rgPropSets, (IUnknown**)&m_pIUnkObject), S_OK, DB_S_NOTSINGLETON, DB_S_ERRORSOCCURRED);

	// Assign the rowset into pIUnkObject
	TESTC(m_pIUnkObject != NULL);

	// Make sure we can get errors for initialize, we'll consider
	// ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					ROW_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported, cannot continue with test case." << ENDL;
		PROVIDER_FREE(rgProperties);
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	fResults = TRUE;

CLEANUP:
	
	PROVIDER_FREE(rgProperties);
	return fResults;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCErrorIRowRowset::Variation_1()
{
	if( LogErrorSupport() )
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
BOOL TCErrorIRowRowset::Terminate()
{
	// Release
	SAFE_RELEASE( m_pIUnkObject );

	BOOL fResults = (CSupportError::Terminate());
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Drop the table, also deletes the table name
	if(m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCErrorBinder)
//*-----------------------------------------------------------------------
//| Test Case:		TCErrorBinder - Checks ISupportErrorInfo for all Binder Interfaces
//| Created:  	1/18/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCErrorBinder::Init()
{ 
	ISupportErrorInfo * pISupportErrorInfo = NULL;
	IBindResource*		pIBR = NULL;

	// Obtain the array of interfaces for this object...
	TESTC(GetInterfaceArray(BINDER_INTERFACE, &m_cInterfaces, &m_rgInterfaces));

	// Allocate the max possible we'll need for supported interfaces
	SAFE_ALLOC(m_rgSupportedInterfaces, SUPPORTEDINTERFACES, m_cInterfaces);

	pIBR = GetModInfo()->GetRootBinder();
	if( !pIBR )
	{
		odtLog << L"Could not get a Binder object." << ENDL;
		return TEST_SKIPPED;
	}

	// Assign the Binder into pIUnkObject
	m_pIUnkObject = pIBR;

	//Make sure we can get errors for initialize, we'll consider
	//ISupportErrorInfo mandatory for purposes of this test
	if( !VerifyInterface(m_pIUnkObject, IID_ISupportErrorInfo, 
					BINDER_INTERFACE, (IUnknown **)&pISupportErrorInfo) )
	{
		odtLog << L"Interface ISupportErrorInfo was not supported on Binder, cannot continue with test case." << ENDL;
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pISupportErrorInfo);

	// Now find all the supported interfaces	
	TESTC(CSupportError::Init());
	return TRUE;
	
CLEANUP:
	
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Log all interfaces and their error support
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCErrorBinder::Variation_1()
{ 
	if( LogErrorSupport() )
		return TEST_PASS;
	else	
		return TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCErrorBinder::Terminate()
{ 
	// Release
	BOOL fResults = (CSupportError::Terminate());

	PROVIDER_FREE(m_rgSupportedInterfaces);

	return fResults;
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

