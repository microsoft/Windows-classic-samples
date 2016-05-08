//*---------------------------------------------------------------------
//
//   Conformance Test for IRegisterProvider interface.
//
//   WARNING:
//   PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//   Copyright (C) 1994-2000 Microsoft Corporation
//*---------------------------------------------------------------------

#include "MODStandard.hpp"
#include "IRegisterProvider.h"
#include "ExtraLib.h"


//*---------------------------------------------------------------------
// Module Values
//*---------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x607bb940, 0x4c23, 0x11d2, { 0x88, 0xd2, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("IRegisterProvider");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRegisterProvider Test Module");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////
IRegisterProvider*	g_pIRegProv=NULL;


//----------------------------------------------------------------------
// Cleans the registry so that all mappings from the static table 
// g_rgURLSchemes are removed. Leave no traces of the test having run.
//
BOOL CleanupRegistry()
{
	TBEGIN
	ULONG	ulIndex;

	if(!g_pIRegProv)
		return TRUE;

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		HRESULT	hr;
		CLSID	clsidTemp = DB_NULLGUID;

		hr = g_pIRegProv->GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme, 0, &clsidTemp);

		if(S_OK == hr)
			CHECK(g_pIRegProv->UnregisterProvider(g_rgURLSchemes[ulIndex].pwszURLScheme,
				0, clsidTemp), S_OK);
		else
			CHECK(hr, S_FALSE);
	}

	TRETURN
} //CleanupRegistry


//*-----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN

	TESTC(CreateModInfo(pThisTestModule))
	TESTC_PROVIDER(VerifyInterface(GetModInfo()->GetRootBinder(), 
		IID_IRegisterProvider, BINDER_INTERFACE,
		(IUnknown**)&g_pIRegProv))

	TESTC(g_pIRegProv != NULL)

	TESTC(CleanupRegistry())

CLEANUP:
	TRETURN
}

//*-----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	CleanupRegistry();
	SAFE_RELEASE(g_pIRegProv);
	return ReleaseModInfo(pThisTestModule);
}


////////////////////////////////////////////////////////////////////////
//TCRegProv Class
//
////////////////////////////////////////////////////////////////////////
class TCRegProv : public CSessionObject
{
public:

	//Constructor
	TCRegProv(WCHAR* pwszTestCaseName);

	//Destructor
	virtual ~TCRegProv();

protected:

//VARIABLES...

	HRESULT				m_hr;

//INTERFACES...

	IRegisterProvider*		m_pIRegProv;

//METHODS...

	//Wrapper for corresponding method.
	HRESULT	GetURLMapping(
		LPOLESTR	pwszURL,
		CLSID*		pclsidProvider);

	//Wrapper for corresponding method.
	HRESULT	SetURLMapping(
		LPOLESTR	pwszURL,
		REFCLSID	rclsidProvider);

	//Wrapper for corresponding method.
	HRESULT	UnregProv(
		LPOLESTR	pwszURL,
		REFCLSID	rclsidProvider);

	//Gets the IRegisterProvider interface from the Root Binder.
	BOOL	GetRootBinder();

	//Calls SetURLMapping on all entries in the static table 
	//g_rgURLSchemes.
	BOOL	RegisterAll();

	//Calls UnregisterProvider on all entries in the static table 
	//g_rgURLSchemes.
	BOOL	UnregisterAll();

	BOOL	CheckRegistry(
		LPOLESTR	pwszURL,
		CLSID		clsidProvider);

private:

//METHODS...

	//Set Init props on Root Binder.
	BOOL	SetInitProps(IDBBinderProperties* pIDBBindProp);

};


////////////////////////////////////////////////////////////////////////
//TCRegProv Implementation
//
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
//  TCRegProv::TCRegProv
//
TCRegProv::TCRegProv(WCHAR * pwszTestCaseName)	: CSessionObject(pwszTestCaseName) 
{
	m_pIRegProv			= NULL;
}

//----------------------------------------------------------------------
//  TCRegProv::~TCRegProv
//
TCRegProv::~TCRegProv()
{
	SAFE_RELEASE(m_pIRegProv);
}

//----------------------------------------------------------------------
// TCRegProv::GetURLMapping
// Wrapper for corresponding method.
//
HRESULT TCRegProv::GetURLMapping(LPOLESTR pwszURL, CLSID* pclsidProvider)
{
	HRESULT		hr = E_FAIL;
	CLSID		clsidRow = DBGUID_ROW;
	CLSID		clsidNull = DB_NULLGUID;

	if(!m_pIRegProv)
		return E_FAIL;

	if(pclsidProvider)
		*pclsidProvider = clsidRow;

	hr = m_pIRegProv->GetURLMapping(pwszURL, 0, pclsidProvider);

	if(hr != E_INVALIDARG)
		TESTC(pclsidProvider != NULL);

	if(hr==S_OK)
	{
		COMPARE(*pclsidProvider != clsidRow, TRUE);
		COMPARE(*pclsidProvider != clsidNull, TRUE);
		COMPARE(CheckRegistry(pwszURL, *pclsidProvider), TRUE);
	}
	if(hr==S_FALSE)
	{
		COMPARE(*pclsidProvider == clsidNull, TRUE);
		COMPARE(CheckRegistry(pwszURL, *pclsidProvider), FALSE);
	}
	if(FAILED(hr) && pclsidProvider)
		COMPARE(*pclsidProvider == clsidNull, TRUE);

CLEANUP:
	return hr;
} //GetURLMapping

//----------------------------------------------------------------------
// TCRegProv::SetURLMapping
// Wrapper for corresponding method.
//
HRESULT TCRegProv::SetURLMapping(LPOLESTR pwszURL, REFCLSID rclsidProvider)
{
	HRESULT		hr = E_FAIL;

	if(!m_pIRegProv)
		return hr;

	hr = m_pIRegProv->SetURLMapping(pwszURL, 0, rclsidProvider);

	//If cannot find entry in registry for L".:" it is Not 
	//an error because SetURLMapping on URL L".:" will succeed,
	//but the entry is made under scheme "file". 
	if(hr == S_OK)
	{
		if(wcscmp(pwszURL,L".:")==0)
		{

			COMPARE(CheckRegistry(L"file", rclsidProvider),TRUE);
		}
		else
		{
			COMPARE(CheckRegistry(pwszURL, rclsidProvider),TRUE);
		}

	}

	return hr;
} //SetURLMapping

//----------------------------------------------------------------------
// TCRegProv::UnregProv
// Wrapper for corresponding method.
//
HRESULT TCRegProv::UnregProv(LPOLESTR pwszURL, REFCLSID rclsidProvider)
{
	HRESULT		hr = E_FAIL;

	if(!m_pIRegProv)
		return hr;

	hr = m_pIRegProv->UnregisterProvider(pwszURL, 0, rclsidProvider);

	return hr;
} //UnregProv

//----------------------------------------------------------------------
// TCRegProv::GetRootBinder
// Create a new Root Binder and get its IRegisterProvider interface.
//
BOOL TCRegProv::GetRootBinder()
{
	TBEGIN
	IBindResource*			pIBR = NULL;
	IDBBinderProperties*	pIDBBProp = NULL;

	TESTC_(m_hr=CoCreateInstance(CLSID_RootBinder, NULL, GetModInfo()->GetClassContext(), 
		IID_IBindResource, (void**)&pIBR), S_OK)

	TESTC(pIBR != NULL)

	TESTC(VerifyInterface(pIBR, IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&m_pIRegProv))
	TESTC(DefaultObjectTesting(m_pIRegProv, BINDER_INTERFACE))

	TESTC(VerifyInterface(m_pIRegProv, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBProp))
	TESTC(DefaultObjectTesting(pIDBBProp, BINDER_INTERFACE))

	TESTC(SetInitProps(pIDBBProp))
	
CLEANUP:
	SAFE_RELEASE(pIDBBProp);
	SAFE_RELEASE(pIBR);
	TRETURN
} //GetRootBinder

//----------------------------------------------------------------------
// TCRegProv::RegisterAll
// Register ALL entries from the static table g_rgURLSchemes.
//
BOOL TCRegProv::RegisterAll()
{
	TBEGIN

	for(ULONG ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
		CHECK(SetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK);

	TRETURN
} //RegisterAll

//----------------------------------------------------------------------
// TCRegProv::UnregisterAll
// Unregister ALL entries from the static table g_rgURLSchemes.
//
BOOL TCRegProv::UnregisterAll()
{
	TBEGIN
	HRESULT	hr;

	for(ULONG ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		hr = UnregProv(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid);
		if(hr != S_FALSE)
			CHECK(hr, S_OK);
	}

	TRETURN
} //UnregisterAll

//----------------------------------------------------------------------
// TCRegProv::SetInitProps
// Set Init Props on the Root Binder.
//
BOOL TCRegProv::SetInitProps(IDBBinderProperties* pIDBBindProp)
{
	BOOL		bRet = FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;

	if(!pIDBBindProp)
		return FALSE;

	if(COMPARE(GetInitProps(&cPropSets, &rgPropSets), TRUE))
	{
		CHECK(pIDBBindProp->Reset(), S_OK);
		if(CHECK(pIDBBindProp->SetProperties(cPropSets, rgPropSets), 
			S_OK))
			bRet = TRUE;
	}

	FreeProperties(&cPropSets, &rgPropSets);
	return bRet;
} //SetInitProps

//----------------------------------------------------------------------
// TCRegProv::CheckRegistry
//
BOOL TCRegProv::CheckRegistry(
		LPOLESTR	pwszURL,
		CLSID		clsidProvider)
{
	HRESULT			hr=E_FAIL;
	BOOL			bFound = FALSE;
	CHAR			szKeyName[200];
	LONG			lResult=0;
	HKEY			hKey = NULL;
	DWORD			dwIndex=0;
	CHAR			szValueName[600];
	DWORD			cbValueName=0;
	CHAR			szData[100];
	DWORD			cbData=0;
	DWORD			dwType=0;
	CHAR			szScheme[50];
	CHAR*			pszURL=NULL;
	CHAR*			pdest=NULL;
	LPOLESTR		pwszClsid=NULL;
	CHAR*			pszClsid=NULL;

	TESTC_(StringFromCLSID(clsidProvider, &pwszClsid), S_OK)
	TESTC(pwszClsid != NULL)
	pszClsid = ConvertToMBCS(pwszClsid);
	TESTC(pszClsid != NULL)

	TESTC(pwszURL != NULL)
	pszURL = ConvertToMBCS(pwszURL);
	TESTC(pszURL != NULL)
	pdest = strchr( pszURL, ':');
	if(pdest)
	{
		strncpy(szScheme, pszURL, pdest-pszURL);
		szScheme[pdest-pszURL] = '\0';
	}
	else
		strcpy(szScheme, pszURL);

	strcpy(szKeyName, "SOFTWARE\\Microsoft\\DataAccess\\RootBinder\\");
	strcat(szKeyName, szScheme);

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						   szKeyName,
						   0,
						   KEY_READ,
						   &hKey);

	if( lResult != ERROR_SUCCESS )
		goto CLEANUP;

	for( dwIndex = 0; ; dwIndex++ )
	{
		cbValueName	= sizeof(szValueName);
		cbData		= sizeof(szData);

		lResult = RegEnumValue(hKey,
							   dwIndex,
							   szValueName,
							   &cbValueName,
							   NULL,
							   &dwType,
							   (LPBYTE)szData,
							   &cbData);

		if(lResult == ERROR_NO_MORE_ITEMS)
			break;

		if(!strcmp(szData, pszClsid))
		{
			bFound = TRUE;
			break;
		}
	}

CLEANUP:
	RegCloseKey(hKey);
	SAFE_FREE(pszURL);
	SAFE_FREE(pszClsid);
	SAFE_FREE(pwszClsid);
	return bFound;
} //CheckRegistry




//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------


// {{ TCW_TEST_CASE_MAP(TCGetURLMapping)
//*-----------------------------------------------------------------------
// @class Test case for GetURLMapping method.
//
class TCGetURLMapping : public TCRegProv { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetURLMapping,TCRegProv);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: Use registered URL schemes and prefixes
	int Variation_1();
	// @cmember S_OK: Use one-level extensions of registered URL schemes and prefixes
	int Variation_2();
	// @cmember S_OK: Use two-level extensions of registered URL schemes and prefixes
	int Variation_3();
	// @cmember S_OK: Unregister a scheme+prefix where the scheme is also registered
	int Variation_4();
	// @cmember S_OK: Unregister a URL and verify it finds the next shortest match
	int Variation_5();
	// @cmember S_FALSE: New URL scheme
	int Variation_6();
	// @cmember S_FALSE: New URL scheme+prefix
	int Variation_7();
	// @cmember S_FALSE: Unreg a registered URL
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetURLMapping)
#define THE_CLASS TCGetURLMapping
BEG_TEST_CASE(TCGetURLMapping, TCRegProv, L"Test case for GetURLMapping method.")
	TEST_VARIATION(1, 		L"S_OK: Use registered URL schemes and prefixes")
	TEST_VARIATION(2, 		L"S_OK: Use one-level extensions of registered URL schemes and prefixes")
	TEST_VARIATION(3, 		L"S_OK: Use two-level extensions of registered URL schemes and prefixes")
	TEST_VARIATION(4, 		L"S_OK: Unregister a scheme+prefix where the scheme is also registered")
	TEST_VARIATION(5, 		L"S_OK: Unregister a URL and verify it finds the next shortest match")
	TEST_VARIATION(6, 		L"S_FALSE: New URL scheme")
	TEST_VARIATION(7, 		L"S_FALSE: New URL scheme+prefix")
	TEST_VARIATION(8, 		L"S_FALSE: Unreg a registered URL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCSetURLMapping)
//*-----------------------------------------------------------------------
// @class Test case for SetURLMapping method.
//
class TCSetURLMapping : public TCRegProv { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSetURLMapping,TCRegProv);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: Register URL schemes only
	int Variation_1();
	// @cmember S_OK: Register URLs with same scheme but diff prefix
	int Variation_2();
	// @cmember S_OK: Register all schemes and scheme+prefixes
	int Variation_3();
	// @cmember S_OK: Register URLs with chars like + - . etc
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCSetURLMapping)
#define THE_CLASS TCSetURLMapping
BEG_TEST_CASE(TCSetURLMapping, TCRegProv, L"Test case for SetURLMapping method.")
	TEST_VARIATION(1, 		L"S_OK: Register URL schemes only")
	TEST_VARIATION(2, 		L"S_OK: Register URLs with same scheme but diff prefix")
	TEST_VARIATION(3, 		L"S_OK: Register all schemes and scheme+prefixes")
	TEST_VARIATION(4, 		L"S_OK: Register URLs with chars like + - . etc")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCUnregisterProvider)
//*-----------------------------------------------------------------------
// @class Test case for UnregisterProvider method.
//
class TCUnregisterProvider : public TCRegProv { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCUnregisterProvider,TCRegProv);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: Unreg URL schemes
	int Variation_1();
	// @cmember S_OK: Unreg URL scheme+prefixes
	int Variation_2();
	// @cmember S_OK: pwszURL = NULL
	int Variation_3();
	// @cmember S_FALSE: Unregister a URL twice
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCUnregisterProvider)
#define THE_CLASS TCUnregisterProvider
BEG_TEST_CASE(TCUnregisterProvider, TCRegProv, L"Test case for UnregisterProvider method.")
	TEST_VARIATION(1, 		L"S_OK: Unreg URL schemes")
	TEST_VARIATION(2, 		L"S_OK: Unreg URL scheme+prefixes")
	TEST_VARIATION(3, 		L"S_OK: pwszURL = NULL")
	TEST_VARIATION(4, 		L"S_FALSE: Unregister a URL twice")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBoundary)
//*-----------------------------------------------------------------------
// @class Boundary cases
//
class TCBoundary : public TCRegProv { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBoundary,TCRegProv);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember GetURLMap: S_FALSE - Non-existent URL
	int Variation_1();
	// @cmember GetURLMap: E_INVALIDARG - pwszURL is NULL
	int Variation_2();
	// @cmember GetURLMap: E_INVALIDARG - pclsidProvider is NULL
	int Variation_3();
	// @cmember GetURLMap: E_INVALIDARG - pwszURL and pclsidProvider are NULL
	int Variation_4();
	// @cmember GetURLMap: Weird URLs like empty strings, \\, //, :, etc.
	int Variation_5();
	// @cmember SetURLMap: DB_E_RESOURCEEXISTS
	int Variation_6();
	// @cmember SetURLMap: E_INVALIDARG - pwszURL is NULL
	int Variation_7();
	// @cmember SetURLMap: S_OK - rclsidProvider is DB_NULLGUID
	int Variation_8();
	// @cmember SetURLMap: E_INVALIDARG - pwszURL is NULL and rclsidProvider is DB_NULLGUID
	int Variation_9();
	// @cmember SetURLMap: Weird URLs like empty strings, \\, //, :, etc.
	int Variation_10();
	// @cmember UnregProv: S_FALSE - Non-existent URL
	int Variation_11();
	// @cmember UnregProv: S_FALSE - rclsid is DB_NULLGUID
	int Variation_12();
	// @cmember UnregProv: S_FALSE - Unreg CLSID_RootBinder
	int Variation_13();
	// @cmember UnregProv: Weird URLs
	int Variation_14();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBoundary)
#define THE_CLASS TCBoundary
BEG_TEST_CASE(TCBoundary, TCRegProv, L"Boundary cases")
	TEST_VARIATION(1, 		L"GetURLMap: S_FALSE - Non-existent URL")
	TEST_VARIATION(2, 		L"GetURLMap: E_INVALIDARG - pwszURL is NULL")
	TEST_VARIATION(3, 		L"GetURLMap: E_INVALIDARG - pclsidProvider is NULL")
	TEST_VARIATION(4, 		L"GetURLMap: E_INVALIDARG - pwszURL and pclsidProvider are NULL")
	TEST_VARIATION(5, 		L"GetURLMap: Weird URLs like empty strings, \\, //, :, etc.")
	TEST_VARIATION(6, 		L"SetURLMap: DB_E_RESOURCEEXISTS")
	TEST_VARIATION(7, 		L"SetURLMap: E_INVALIDARG - pwszURL is NULL")
	TEST_VARIATION(8, 		L"SetURLMap: S_OK - rclsidProvider is DB_NULLGUID")
	TEST_VARIATION(9, 		L"SetURLMap: E_INVALIDARG - pwszURL is NULL and rclsidProvider is DB_NULLGUID")
	TEST_VARIATION(10, 		L"SetURLMap: Weird URLs like empty strings, \\, //, :, etc.")
	TEST_VARIATION(11, 		L"UnregProv: S_FALSE - Non-existent URL")
	TEST_VARIATION(12, 		L"UnregProv: S_FALSE - rclsid is DB_NULLGUID")
	TEST_VARIATION(13, 		L"UnregProv: S_FALSE - Unreg CLSID_RootBinder")
	TEST_VARIATION(14, 		L"UnregProv: Weird URLs")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCSpecialCases)
//*-----------------------------------------------------------------------
// @class Special Cases
//
class TCSpecialCases : public TCRegProv { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSpecialCases,TCRegProv);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General: Very Long URL
	int Variation_1();
	// @cmember General: URLs with backslashes
	int Variation_2();
	// @cmember General: Case-insensitivity of URLs
	int Variation_3();
	// @cmember General: Known schemes like http, file, etc.
	int Variation_4();
	// @cmember General: Known schemes (e.g. http, file) with prefixes.
	int Variation_5();
	// @cmember General: Register a 1000 URLs
	int Variation_6();
	// @cmember Incomplete entry.
	int Variation_7();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCSpecialCases)
#define THE_CLASS TCSpecialCases
BEG_TEST_CASE(TCSpecialCases, TCRegProv, L"Special Cases")
	TEST_VARIATION(1, 		L"General: Very Long URL")
	TEST_VARIATION(2, 		L"General: URLs with backslashes")
	TEST_VARIATION(3, 		L"General: Case-insensitivity of URLs")
	TEST_VARIATION(4, 		L"General: Known schemes like http, file, etc.")
	TEST_VARIATION(5, 		L"General: Known schemes (e.g. http, file) with prefixes.")
	TEST_VARIATION(6, 		L"General: Register a 1000 URLs")
	TEST_VARIATION(7, 		L"Incomplete entry.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()


// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetURLMapping)
	TEST_CASE(2, TCSetURLMapping)
	TEST_CASE(3, TCUnregisterProvider)
	TEST_CASE(4, TCBoundary)
	TEST_CASE(5, TCSpecialCases)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END



// {{ TCW_TC_PROTOTYPE(TCGetURLMapping)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetURLMapping - Test case for GetURLMapping method.
//| Created:  	9/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetURLMapping::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRegProv::Init())
	// }}
	{
		//This Test Case will assume that ALL entries from the static
		//table are registered at all times.
		if(GetRootBinder())
			return RegisterAll();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Use registered URL schemes and prefixes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_1()
{ 
	TBEGIN
	ULONG	ulIndex;
	ULONG	ulHighOdd=0;

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;
		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

	//Get the URL Mappings for even indexed entries, and then
	//the odd indexed entries in reverse order.

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex+=2)
	{
		CLSID	clsidTemp;
		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

	if(g_cURLSchemes%2)
		ulHighOdd = g_cURLSchemes;
	else
		ulHighOdd = g_cURLSchemes-1;

	for(ulIndex=ulHighOdd; ulIndex<100; ulIndex-=2) //change here
	{
		CLSID	clsidTemp;
		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Use one-level extensions of registered URL schemes and prefixes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_2()
{ 
	TBEGIN

	ULONG ulIndex=0;
	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszExt1, &clsidTemp), S_OK)

		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

CLEANUP:
	CHECK_FAIL(ulIndex);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Use two-level extensions of registered URL schemes and prefixes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_3()
{ 
	TBEGIN

	ULONG ulIndex=0;
	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszExt2, &clsidTemp), S_OK)

		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

CLEANUP:
	CHECK_FAIL(ulIndex);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Unregister a scheme+prefix where the scheme is also registered
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_4()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(GetURLMapping(L"X-IRPabcd://klmn", &clsidTemp), S_OK)

	TESTC(clsidTemp == CLSID_IREGPROV2)

	TESTC_(UnregProv(L"X-IRPabcd://klmn", CLSID_IREGPROV2), S_OK)

	//After having unregistered "X-IRPabcd://klmn", GetURLMapping
	//will match it to the scheme "X-IRPabcd" which is mapped 
	//to CLSID_IREGPROV1.
	TESTC_(GetURLMapping(L"X-IRPabcd://klmn", &clsidTemp), S_OK)

	TESTC(clsidTemp == CLSID_IREGPROV1)

CLEANUP:
	CHECK(SetURLMapping(L"X-IRPabcd://klmn", CLSID_IREGPROV2), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Unregister a URL and verify it finds the next shortest match
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_5()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(GetURLMapping(L"X-IRPasp://rr/ss/tt/uu/vv",
			&clsidTemp), S_OK)

	TESTC(clsidTemp == CLSID_IREGPROV1)

	TESTC_(UnregProv(L"X-IRPasp://rr/ss/tt/uu/vv",
		CLSID_IREGPROV1), S_OK)

	//After having unregistered "X-IRPasp://rr/ss/tt/uu/vv", 
	//GetURLMapping will match it to the URL 
	//"X-IRPasp://rr/ss/tt" which is mapped to CLSID_IREGPROV9.
	TESTC_(GetURLMapping(L"X-IRPasp://rr/ss/tt/uu/vv",
		&clsidTemp), S_OK)

	TESTC(clsidTemp == CLSID_IREGPROV9)

CLEANUP:
	CHECK(SetURLMapping(L"X-IRPasp://rr/ss/tt/uu/vv",
		CLSID_IREGPROV1), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_FALSE: New URL scheme
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_6()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(GetURLMapping(L"newURLScheme", &clsidTemp), S_FALSE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_FALSE: New URL scheme+prefix
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_7()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(GetURLMapping(L"newURLScheme://abcd", &clsidTemp), S_FALSE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_FALSE: Unreg a registered URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetURLMapping::Variation_8()
{ 
	TBEGIN

	ULONG ulIndex=0;
	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(UnregProv(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK)

		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_FALSE)
	}

CLEANUP:
	COMPARE(RegisterAll(),TRUE);
	CHECK_FAIL(ulIndex);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGetURLMapping::Terminate()
{ 
	COMPARE(UnregisterAll(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRegProv::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCSetURLMapping)
//*-----------------------------------------------------------------------
//| Test Case:		TCSetURLMapping - Test case for SetURLMapping method.
//| Created:  	9/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetURLMapping::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRegProv::Init())
	// }}
	{ 
		//This Test Case will assume that ALL entries from the static
		//table are unregistered at all times.
		if(GetRootBinder())
			return UnregisterAll();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Register URL schemes only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetURLMapping::Variation_1()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(SetURLMapping(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK)
	TESTC_(SetURLMapping(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK)

	TESTC_(GetURLMapping(L"X-IRPabcd:", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV1)

	TESTC_(GetURLMapping(L"X-IRPwxyz:", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV7)

CLEANUP:
	CHECK(UnregProv(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK);
	CHECK(UnregProv(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Register URLs with same scheme but diff prefix
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetURLMapping::Variation_2()
{ 
	TBEGIN
	ULONG	ulIndex;

	for(ulIndex=0; ulIndex<5; ulIndex++)
		TESTC_(SetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK)

	for(ulIndex=0; ulIndex<5; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

CLEANUP:
	CHECK_FAIL(ulIndex);
	COMPARE(UnregisterAll(), TRUE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Register all schemes and scheme+prefixes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetURLMapping::Variation_3()
{ 
	TBEGIN
	ULONG	ulIndex;

	//Register all from static array.

	TESTC(RegisterAll())

	//Verify all from static aray were registered.

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

	//Clear registry

	TESTC(UnregisterAll())

	//Register only one at a time and verify.

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(SetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK)
		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

	//Calling SetURLMapping twice in a row
	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		CLSID	clsidTemp;

		TESTC_(SetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK)
		TESTC_(SetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			g_rgURLSchemes[ulIndex].clsid), S_OK)
		TESTC_(GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex].clsid)
	}

CLEANUP:
	CHECK_FAIL(ulIndex);
	COMPARE(UnregisterAll(), TRUE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Register URLs with chars like + - . etc
// chars are valid for URL schemes as per the RFC 1738.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetURLMapping::Variation_4()
{ 
	TBEGIN
	ULONG		ulIndex;
	CLSID		clsidProv1 = CLSID_IREGPROV1;
	CLSID		clsidNull = DB_NULLGUID;
	CLSID		clsidTemp = DB_NULLGUID;
	WCHAR*		pwszProgID = NULL;
	LPOLESTR	rgwURLs[10] = {L"20+20:", L"ab-cd:", L"abc.666:", 
								L"ABCabc:", L"1224:", L"+-+-", 
								L"20+30-40.5", L"5a6b", L".rt20",
								L"-000.xxx"};
	ULONG		cURLs = NUMELEM(rgwURLs);

	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
		TESTC_(SetURLMapping(rgwURLs[ulIndex], clsidProv1), S_OK)

	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
	{
		TESTC_(GetURLMapping(rgwURLs[ulIndex], &clsidTemp), S_OK)
		TESTC(clsidTemp == clsidProv1)
		clsidTemp = clsidNull;
	}

	//Cleanup
	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
		CHECK(UnregProv(rgwURLs[ulIndex], clsidProv1), S_OK);

	//Verify cleanup
	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
		TESTC_(GetURLMapping(rgwURLs[ulIndex], &clsidTemp), S_FALSE)

CLEANUP:
	UnregProv(NULL, clsidProv1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCSetURLMapping::Terminate()
{ 
	COMPARE(UnregisterAll(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRegProv::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCUnregisterProvider)
//*-----------------------------------------------------------------------
//| Test Case:		TCUnregisterProvider - Test case for UnregisterProvider method.
//| Created:  	9/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUnregisterProvider::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRegProv::Init())
	// }}
	{ 
		//This Test Case will assume that ALL entries from the static
		//table are registered at all times.
		if(GetRootBinder())
			return RegisterAll();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Unreg URL schemes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnregisterProvider::Variation_1()
{ 
	TBEGIN
	CLSID	clsidTemp;

	//Unreg schemes
	TESTC_(UnregProv(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK)
	TESTC_(UnregProv(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK)

	//Verify Schemes were removed.
	TESTC_(GetURLMapping(L"X-IRPabcd:", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPwxyz:", &clsidTemp), S_FALSE)

	//Further verification using extensions to the scheme.

	TESTC_(GetURLMapping(L"X-IRPabcd:123%20@abc", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPabcd:abcd/extension", &clsidTemp), S_FALSE)

	TESTC_(GetURLMapping(L"X-IRPwxyz:<1+2.3>&^", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPwxyz:=12/13//", &clsidTemp), S_FALSE)

CLEANUP:
	CHECK(SetURLMapping(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK);
	CHECK(SetURLMapping(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Unreg URL scheme+prefixes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnregisterProvider::Variation_2()
{ 
	TBEGIN
	CLSID	clsidTemp;

	//Unreg schemes+prefixes
	TESTC_(UnregProv(L"X-IRPabcd://klmn/wxyz", CLSID_IREGPROV3), S_OK)
	TESTC_(UnregProv(L"X-IRPabcd://hijk", CLSID_IREGPROV4), S_OK)

	//Verify Schemes+prefixes were removed.
	TESTC_(GetURLMapping(L"X-IRPabcd://klmn/wxyz", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV2)

	TESTC_(GetURLMapping(L"X-IRPabcd://hijk", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV1)

	//Further verification using extensions to the URL.

	TESTC_(GetURLMapping(L"X-IRPabcd://klmn/wxyz/#", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV2)

	TESTC_(GetURLMapping(L"X-IRPabcd://hijk/wxyz", &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_IREGPROV1)

CLEANUP:
	CHECK(SetURLMapping(L"X-IRPabcd://klmn/wxyz", CLSID_IREGPROV3), S_OK);
	CHECK(SetURLMapping(L"X-IRPabcd://hijk", CLSID_IREGPROV4), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: pwszURL = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnregisterProvider::Variation_3()
{ 
	TBEGIN
	ULONG	ulIndex;
	CLSID	clsidTemp;
	HRESULT	hr;

	TESTC(RegisterAll())

	//Unregister ALL mappings for the provider CLSID_IREGPROV1.
	TESTC_(UnregProv(NULL, CLSID_IREGPROV1), S_OK)

	for(ulIndex=0; ulIndex<g_cURLSchemes; ulIndex++)
	{
		TEST2C_(hr = GetURLMapping(g_rgURLSchemes[ulIndex].pwszURLScheme,
			&clsidTemp), S_OK, S_FALSE)

		//No URL should cause GetURLMapping to return CLSID_IREGPROV1.
		TESTC( clsidTemp != CLSID_IREGPROV1)
	}

CLEANUP:
	RegisterAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_FALSE: Unregister a URL twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnregisterProvider::Variation_4()
{ 
	TBEGIN
	CLSID	clsidTemp;

	//Unreg schemes
	TESTC_(UnregProv(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK)
	TESTC_(UnregProv(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK)

	//Unreg again
	TESTC_(UnregProv(L"X-IRPabcd:", CLSID_IREGPROV1), S_FALSE)
	TESTC_(UnregProv(L"X-IRPwxyz:", CLSID_IREGPROV7), S_FALSE)

	//Verify Schemes were removed.
	TESTC_(GetURLMapping(L"X-IRPabcd:", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPwxyz:", &clsidTemp), S_FALSE)

	//Further verification using extensions to the scheme.

	TESTC_(GetURLMapping(L"X-IRPabcd:123%20@abc", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPabcd:abcd/extension", &clsidTemp), S_FALSE)

	TESTC_(GetURLMapping(L"X-IRPwxyz:<1+2.3>&^", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRPwxyz:=12/13//", &clsidTemp), S_FALSE)

CLEANUP:
	CHECK(SetURLMapping(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK);
	CHECK(SetURLMapping(L"X-IRPwxyz:", CLSID_IREGPROV7), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCUnregisterProvider::Terminate()
{ 
	COMPARE(UnregisterAll(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRegProv::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBoundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCBoundary - Boundary cases
//| Created:  	11/30/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBoundary::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRegProv::Init())
	// }}
	{ 
		//This Test Case will assume that ALL entries from the static
		//table are unregistered at all times.
		if(GetRootBinder())
			return UnregisterAll();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetURLMap: S_FALSE - Non-existent URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_1()
{ 
	TBEGIN
	CLSID	clsidTemp = CLSID_IREGPROV1;

	TESTC_(GetURLMapping(L"NonExistentURLScheme://a",&clsidTemp), S_FALSE)

	clsidTemp = CLSID_IREGPROV1;

	TESTC_(GetURLMapping(L"NonExistentURLScheme",&clsidTemp), S_FALSE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetURLMap: E_INVALIDARG - pwszURL is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_2()
{ 
	TBEGIN
	CLSID	clsidTemp;

	TESTC_(GetURLMapping(NULL, &clsidTemp), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetURLMap: E_INVALIDARG - pclsidProvider is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_3()
{ 
	TBEGIN

	TESTC_(GetURLMapping(L"X-IRPabcd:", NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetURLMap: E_INVALIDARG - pwszURL and pclsidProvider are NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_4()
{ 
	TBEGIN

	TESTC_(GetURLMapping(NULL, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetURLMap: Weird URLs like empty strings, \\, //, :, etc.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_5()
{ 
	TBEGIN

	HRESULT	hr=E_FAIL;
	CLSID	clsidTemp = CLSID_IREGPROV1;

	CHECK(GetURLMapping(L"\\", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"//", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L" ", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L":", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"@123", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"#$:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L">:)", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"(:-)", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L".:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L",:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"{[abc]}", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"%20:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"~:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"*&^", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"+-", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"a|b", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L":xyz/123", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"==", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"?+?", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"a+b-1.20:", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"!!!", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"###", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"$$$", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"&&&", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L";;;", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L":;?,<>()~*^|", &clsidTemp), S_FALSE);
	CHECK(GetURLMapping(L"a:|", &clsidTemp), S_FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc SetURLMap: DB_E_RESOURCEEXISTS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_6()
{ 
	TBEGIN

	TESTC_(SetURLMapping(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK)
	TESTC_(SetURLMapping(L"X-IRPabcd:", CLSID_IREGPROV2), DB_E_RESOURCEEXISTS)

	TESTC_(SetURLMapping(L"X-IRPabcd://klmn/wxyz", CLSID_IREGPROV3), S_OK)
	TESTC_(SetURLMapping(L"X-IRPabcd://klmn/wxyz", CLSID_IREGPROV2), DB_E_RESOURCEEXISTS)

CLEANUP:
	CHECK(UnregProv(L"X-IRPabcd:", CLSID_IREGPROV1), S_OK);
	CHECK(UnregProv(L"X-IRPabcd://klmn/wxyz", CLSID_IREGPROV3), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc SetURLMap: E_INVALIDARG - pwszURL is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_7()
{ 
	TBEGIN

	TESTC_(SetURLMapping(NULL, CLSID_IREGPROV1), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc SetURLMap: S_OK - rclsidProvider is DB_NULLGUID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_8()
{ 
	TBEGIN
	CLSID	clsidNull = DB_NULLGUID;

	CHECK(SetURLMapping(L"X-IRPabcd:", clsidNull), S_OK);
	TESTC_(UnregProv(L"X-IRPabcd:", clsidNull), S_OK)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc SetURLMap: E_INVALIDARG - pwszURL is NULL and rclsidProvider is DB_NULLGUID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_9()
{ 
	TBEGIN
	CLSID	clsidNull = DB_NULLGUID;

	TESTC_(SetURLMapping(NULL, clsidNull), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc SetURLMap: Weird URLs like empty strings, \\, //, :, etc.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_10()
{ 
	TBEGIN

	HRESULT	hr=E_FAIL;
	CLSID	clsidNull = DB_NULLGUID;
	CLSID	clsidProv1 = CLSID_IREGPROV1;

	CHECK(SetURLMapping(L"\\", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"//", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L" ", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L":", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"@123", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"@$:abc", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"#$:", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L">:)", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"(:-)", clsidNull), E_INVALIDARG);
	//Can register schemes with "."
	CHECK(SetURLMapping(L".:", clsidNull), S_OK);
	CHECK(SetURLMapping(L",:", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"{[abc]}", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"%20:", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"~:", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"*&^", clsidNull), E_INVALIDARG);
	//Can register schemes with "+" and "-"
	CHECK(SetURLMapping(L"+-", clsidNull), S_OK);
	CHECK(SetURLMapping(L"a|b", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L":xyz/123", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"==", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"?+?", clsidNull), E_INVALIDARG);
	//Can register schemes with "+","-",".",lowalpha and digit
	CHECK(SetURLMapping(L"a+b-1.20:", clsidNull), S_OK);
	CHECK(SetURLMapping(L"!!!", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"###", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"$$$", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"&&&", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L";;;", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L";:?,<>()~*^|", clsidNull), E_INVALIDARG);
	CHECK(SetURLMapping(L"a:|", clsidNull), E_INVALIDARG);

	//UNREG -------
	CHECK(UnregProv(NULL, clsidNull), S_OK);

	CHECK(SetURLMapping(L"\\", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"//", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L" ", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L":", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"@123", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"@$:abc", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"#$:", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L">:)", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"(:-)", clsidProv1), E_INVALIDARG);
	//Can register schemes with "."
	CHECK(SetURLMapping(L".:", clsidProv1), S_OK);
	CHECK(SetURLMapping(L",:", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"{[abc]}", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"%20:", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"~:", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"*&^", clsidProv1), E_INVALIDARG);
	//Can register schemes with "+" and "-"
	CHECK(SetURLMapping(L"+-", clsidProv1), S_OK);
	CHECK(SetURLMapping(L"a|b", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L":xyz/123", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"==", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"?+?", clsidProv1), E_INVALIDARG);
	//Can register schemes with "+","-",".",lowalpha and digit
	CHECK(SetURLMapping(L"a+b-1.20:", clsidProv1), S_OK);
	CHECK(SetURLMapping(L"!!!", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"###", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"$$$", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"&&&", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L";;;", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L";:?,<>()~*^|", clsidProv1), E_INVALIDARG);
	CHECK(SetURLMapping(L"a:|", clsidProv1), E_INVALIDARG);

	//UNREG -------
	CHECK(UnregProv(NULL, clsidProv1), S_OK);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc UnregProv: S_FALSE - Non-existent URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_11()
{ 
	TBEGIN

	TESTC_(UnregProv(L"NonExistentURLScheme://a",
		CLSID_IREGPROV1), S_FALSE)

	TESTC_(UnregProv(L"NonExistentURLScheme",
		CLSID_IREGPROV1), S_FALSE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc UnregProv: S_FALSE - rclsid is DB_NULLGUID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_12()
{ 
	TBEGIN
	CLSID	clsidNull = DB_NULLGUID;
	HRESULT	hr = S_FALSE;

	TESTC_(hr = UnregProv(L"X-IRPabcd:", clsidNull), S_FALSE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc UnregProv: S_FALSE - Unreg CLSID_RootBinder
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_13()
{ 
	TBEGIN

	TESTC_(UnregProv(NULL, CLSID_RootBinder), S_FALSE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc UnregProv: Weird URLs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary::Variation_14()
{ 
	TBEGIN
	CLSID	clsidNull = DB_NULLGUID;
	CLSID	clsidProv1 = CLSID_IREGPROV1;

	CHECK(UnregProv(L"\\", clsidNull), S_FALSE);
	CHECK(UnregProv(L"//", clsidNull), S_FALSE);
	CHECK(UnregProv(L"", clsidNull), S_FALSE);
	CHECK(UnregProv(L" ", clsidNull), S_FALSE);
	CHECK(UnregProv(L":", clsidNull), S_FALSE);
	CHECK(UnregProv(L"@123", clsidNull), S_FALSE);
	CHECK(UnregProv(L"#$:", clsidNull), S_FALSE);
	CHECK(UnregProv(L">:)", clsidNull), S_FALSE);
	CHECK(UnregProv(L"(:-)", clsidNull), S_FALSE);
	CHECK(UnregProv(L".:", clsidNull), S_FALSE);
	CHECK(UnregProv(L",:", clsidNull), S_FALSE);
	CHECK(UnregProv(L"{[abc]}", clsidNull), S_FALSE);
	CHECK(UnregProv(L"%20:", clsidNull), S_FALSE);
	CHECK(UnregProv(L"~:", clsidNull), S_FALSE);
	CHECK(UnregProv(L"*&^", clsidNull), S_FALSE);
	CHECK(UnregProv(L"+-", clsidNull), S_FALSE);
	CHECK(UnregProv(L"a|b", clsidNull), S_FALSE);
	CHECK(UnregProv(L":xyz/123", clsidNull), S_FALSE);
	CHECK(UnregProv(L"==", clsidNull), S_FALSE);
	CHECK(UnregProv(L"?+?", clsidNull), S_FALSE);
	CHECK(UnregProv(L"a+b-1.20:", clsidNull), S_FALSE);
	CHECK(UnregProv(L"!!!", clsidNull), S_FALSE);
	CHECK(UnregProv(L"###", clsidNull), S_FALSE);
	CHECK(UnregProv(L"$$$", clsidNull), S_FALSE);
	CHECK(UnregProv(L"&&&", clsidNull), S_FALSE);
	CHECK(UnregProv(L";;;", clsidNull), S_FALSE);
	CHECK(UnregProv(L":;?,<>()~*^|", clsidNull), S_FALSE);
	CHECK(UnregProv(L"a:|", clsidNull), S_FALSE);

	CHECK(UnregProv(L"\\", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"//", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"", clsidProv1), S_FALSE);
	CHECK(UnregProv(L" ", clsidProv1), S_FALSE);
	CHECK(UnregProv(L":", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"@123", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"#$:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L">:)", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"(:-)", clsidProv1), S_FALSE);
	CHECK(UnregProv(L".:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L",:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"{[abc]}", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"%20:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"~:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"*&^", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"+-", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"a|b", clsidProv1), S_FALSE);
	CHECK(UnregProv(L":xyz/123", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"==", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"?+?", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"a+b-1.20:", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"!!!", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"###", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"$$$", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"&&&", clsidProv1), S_FALSE);
	CHECK(UnregProv(L";;;", clsidProv1), S_FALSE);
	CHECK(UnregProv(L":;?,<>()~*^|", clsidProv1), S_FALSE);
	CHECK(UnregProv(L"a:|", clsidProv1), S_FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBoundary::Terminate()
{ 
	COMPARE(UnregisterAll(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRegProv::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCSpecialCases)
//*-----------------------------------------------------------------------
//| Test Case:		TCSpecialCases - Special Cases
//| Created:  	4/30/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSpecialCases::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRegProv::Init())
	// }}
	{ 
		//This Test Case will assume that ALL entries from the static
		//table are unregistered at all times.
		if(GetRootBinder())
			return UnregisterAll();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General: Very Long URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_1()
{ 
	TBEGIN
	ULONG	ulIndex;
	ULONG	ulMaxPath = 255;
	WCHAR*	pwszLongURL = NULL;
	CLSID	clsidProv1 = CLSID_IREGPROV1;
	CLSID	clsidTemp = DB_NULLGUID;

	//-----------------------------------
	//String length 260. Border case.
	//-----------------------------------

	SAFE_ALLOC(pwszLongURL, WCHAR, wcslen(L"X-IRPaspxxxxx://")+(61*wcslen(L"xxx/"))+sizeof(WCHAR));
	wcscpy(pwszLongURL, L"X-IRPaspxxxxx://");
	for(ulIndex=0; ulIndex<61; ulIndex++)
		wcscat(pwszLongURL, L"xxx/");

	TESTC_(m_hr = SetURLMapping(pwszLongURL, clsidProv1), S_OK)
	TESTC_(GetURLMapping(pwszLongURL, &clsidTemp), S_OK)
	TESTC(clsidTemp == clsidProv1)
	TESTC_(UnregProv(pwszLongURL, clsidTemp), S_OK)
	SAFE_FREE(pwszLongURL);

	//-----------------------------------
	//String length 260+16.
	//-----------------------------------

	SAFE_ALLOC(pwszLongURL, WCHAR, wcslen(L"X-IRPaspxxxxx://")+(65*wcslen(L"xxx/"))+sizeof(WCHAR));
	wcscpy(pwszLongURL, L"X-IRPaspxxxxx://");
	for(ulIndex=0; ulIndex<65; ulIndex++)
		wcscat(pwszLongURL, L"xxx/");

	TESTC_(m_hr = SetURLMapping(pwszLongURL, clsidProv1), S_OK)
	TESTC_(GetURLMapping(pwszLongURL, &clsidTemp), S_OK)
	TESTC(clsidTemp == clsidProv1)
	TESTC_(UnregProv(pwszLongURL, clsidTemp), S_OK)
	SAFE_FREE(pwszLongURL);

	//-----------------------------------
	//String length 260-16.
	//-----------------------------------

	SAFE_ALLOC(pwszLongURL, WCHAR, wcslen(L"X-IRPaspxxxxx://")+(57*wcslen(L"xxx/"))+sizeof(WCHAR));
	wcscpy(pwszLongURL, L"X-IRPaspxxxxx://");
	for(ulIndex=0; ulIndex<57; ulIndex++)
		wcscat(pwszLongURL, L"xxx/");

	TESTC_(m_hr = SetURLMapping(pwszLongURL, clsidProv1), S_OK)
	TESTC_(GetURLMapping(pwszLongURL, &clsidTemp), S_OK)
	TESTC(clsidTemp == clsidProv1)
	TESTC_(UnregProv(pwszLongURL, clsidTemp), S_OK)
	SAFE_FREE(pwszLongURL);

CLEANUP:
	UnregProv(pwszLongURL, clsidTemp);
	SAFE_FREE(pwszLongURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General: URLs with backslashes
// INFO: For known URL schemes like http, file, etc. the backslashes
// will be converted to forward slashes for matching. Should not
// affect unknown URL schemes.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_2()
{ 
	TBEGIN
	CLSID	clsidTemp = DB_NULLGUID;
	WCHAR*	pwszURL1 = L"X-IRPBackSls://abcd";
	WCHAR*	pwszURL2 = L"X-IRPBackSls://abcd/";
	WCHAR*	pwszURL3 = NULL;

	TESTC_(SetURLMapping(pwszURL1,CLSID_IREGPROV1), S_OK);

	CHECK(GetURLMapping(pwszURL1, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);
	CHECK(GetURLMapping(pwszURL2, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);
	CHECK(GetURLMapping(L"X-IRPBackSls://abcd/rrrr", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);
	//For unknown URL schemes, forward slashes and backslashes are treated differently.
	CHECK(GetURLMapping(L"X-IRPBackSls:\\\\abcd", &clsidTemp), S_FALSE);

	TESTC_(UnregProv(pwszURL1,CLSID_IREGPROV1), S_OK);

	TESTC_(SetURLMapping(pwszURL2,CLSID_IREGPROV2), S_OK);

	CHECK(GetURLMapping(pwszURL1, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(pwszURL2, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(L"X-IRPBackSls://abcd/rrrr", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(L"X-IRPBackSls:\\\\abcd\\rrr", &clsidTemp), S_FALSE);

	TESTC_(UnregProv(pwszURL2,CLSID_IREGPROV2), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"X-IRPBackSls:\\\\abcd/";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV2), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	//For unknown URL schemes, forward slashes and backslashes are treated differently.
	CHECK(GetURLMapping(L"X-IRPBackSls://abcd/", &clsidTemp), S_FALSE);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV2), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"http:\\\\abcd/";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV3), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"http:\\\\abcd", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"http://abcd", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	//For known URL schemes like http, forward slashes and backslashes are treated as the same.
	CHECK(GetURLMapping(L"http://abcd/", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV3), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"http:\\\\abcd\\";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV3), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"http:\\\\abcd", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"http://abcd", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"http://abcd/", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV3), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"file://abcd";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV3), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd\\", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd/", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	//For known URL schemes like file, forward slashes and backslashes are treated as the same.
	CHECK(GetURLMapping(L"file:\\\\abcd\\", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV3), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"file:\\\\abcd\\";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV3), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd\\", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd/", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file:\\\\abcd\\", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV3), S_OK);


	//--------------------------------------------------------


	pwszURL3 = L"file:\\\\abcd/xyz/\\";

	TESTC_(SetURLMapping(pwszURL3,CLSID_IREGPROV3), S_OK);

	CHECK(GetURLMapping(pwszURL3, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd\\xyz\\\\", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file://abcd/xyz\\/", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);
	CHECK(GetURLMapping(L"file:\\\\abcd\\xyz//", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV3);

	TESTC_(UnregProv(pwszURL3,CLSID_IREGPROV3), S_OK);

CLEANUP:
	UnregProv(pwszURL1,CLSID_IREGPROV1);
	UnregProv(pwszURL2,CLSID_IREGPROV2);
	UnregProv(pwszURL3,CLSID_IREGPROV3);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General: Case-insensitivity of URLs
// INFO: URLs are treated case-insensitive by the Root Binder.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_3()
{ 
	TBEGIN
	CLSID	clsidTemp = DB_NULLGUID;
	WCHAR*	pwszURL1 = L"X-IRPCaseSen://AB/cd/EF";
	WCHAR*	pwszURL2 = L"X-IRPCaseSen://ab/cd/ef";

	TESTC_(SetURLMapping(pwszURL1,CLSID_IREGPROV1), S_OK);
	TESTC_(SetURLMapping(pwszURL2,CLSID_IREGPROV2), DB_E_RESOURCEEXISTS);

	CHECK(GetURLMapping(pwszURL1, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);
	CHECK(GetURLMapping(pwszURL2, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);
	CHECK(GetURLMapping(L"X-IRPCASESEN://ab/cd/ef/CSEN", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV1);

	TESTC_(UnregProv(L"X-IRPCaseSen://Ab/Cd/eF",CLSID_IREGPROV1), S_OK);
	CHECK(GetURLMapping(pwszURL1, &clsidTemp), S_FALSE);


	//--------------------------------------------------------


	TESTC_(SetURLMapping(pwszURL2,CLSID_IREGPROV2), S_OK);
	TESTC_(SetURLMapping(pwszURL1,CLSID_IREGPROV1), DB_E_RESOURCEEXISTS);

	CHECK(GetURLMapping(pwszURL1, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(pwszURL2, &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(L"X-IRPCASESEN://ab/cd/ef/CSEN", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);

	TESTC_(UnregProv(pwszURL2,CLSID_IREGPROV2), S_OK);


	//--------------------------------------------------------


	TESTC_(SetURLMapping(L"HTTP://IREGPROV",CLSID_IREGPROV2), S_OK);

	CHECK(GetURLMapping(L"HTTP://IREGPROV", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);
	CHECK(GetURLMapping(L"http://iregprov", &clsidTemp), S_OK);
	COMPARE(clsidTemp, CLSID_IREGPROV2);

	TESTC_(UnregProv(L"http://iregProv",CLSID_IREGPROV2), S_OK);

CLEANUP:
	UnregProv(pwszURL1,CLSID_IREGPROV1);
	UnregProv(pwszURL2,CLSID_IREGPROV2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General: Known schemes like http, file, etc.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_4()
{ 
	TBEGIN

	HRESULT		hr=E_FAIL;
	ULONG		ulIndex=0;
	CLSID		clsidTemp = CLSID_IREGPROV1;
	WCHAR*		pwszProgID = NULL;

	LPOLESTR	rgwURLs[12] = {L"http:", L"msdaipp:", L"file:", L"exstore:", 
								L"ftp:", L"gopher:", L"mailto:", L"news:", 
								L"nntp:", L"telnet:", L"wais:", L"https:"};
	ULONG		cURLs = NUMELEM(rgwURLs);

	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
	{
		hr = GetURLMapping(rgwURLs[ulIndex], &clsidTemp);
		if(S_OK == hr)
		{
			CHECK(ProgIDFromCLSID(clsidTemp, &pwszProgID), S_OK);
			TESTC(pwszProgID != NULL)
			odtLog<<L"INFO: "<<rgwURLs[ulIndex]<<L" is mapped to "<<pwszProgID<<".\n";
		}
		else
		{
			CHECK(hr, S_FALSE);
			odtLog<<L"INFO: No mapping found for "<<rgwURLs[ulIndex]<<L".\n";
			//Try to register it
			CHECK(SetURLMapping(rgwURLs[ulIndex],CLSID_IREGPROV1), S_OK);
			CHECK(GetURLMapping(rgwURLs[ulIndex], &clsidTemp), S_OK);
			CHECK(UnregProv(rgwURLs[ulIndex],CLSID_IREGPROV1), S_OK);
		}

		SAFE_FREE(pwszProgID);
	}

CLEANUP:
	SAFE_FREE(pwszProgID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General: Known schemes (e.g. http, file) with prefixes.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_5()
{ 
	TBEGIN

	HRESULT		hrSet=E_FAIL, hrGet=E_FAIL, hrUnreg=E_FAIL;
	ULONG		ulIndex=0;
	CLSID		clsidProv1 = CLSID_IREGPROV1;
	CLSID		clsidTemp = DB_NULLGUID;

	LPOLESTR	rgwURLs[7] = {L"msdaipp://markedforoffline", L"file:\\@:%",
								L"file:\\\\abcd", L"ftp:\\\\myname@host",
								L"http://abcd\\.../xx  xx\\?@&^%#!",
								L"https://( @:$%)@", L"file:/\\%@X"};
	ULONG		cURLs = NUMELEM(rgwURLs);

	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
	{
		TEST2C_(hrSet = SetURLMapping(rgwURLs[ulIndex], clsidProv1), S_OK, E_INVALIDARG)

		TEST2C_(hrGet = GetURLMapping(rgwURLs[ulIndex], &clsidTemp), S_OK, S_FALSE)
		if(S_OK == hrSet)
		{
			CHECK(hrGet, S_OK);
			COMPARE(clsidTemp, clsidProv1);
		}
		else
			CHECK(hrGet, S_FALSE);

		hrUnreg = UnregProv(rgwURLs[ulIndex], clsidProv1);
		if(hrGet==S_OK)
			CHECK(hrUnreg, S_OK);
		else
			CHECK(hrUnreg, S_FALSE);
	}

CLEANUP:
	for(ulIndex=0; ulIndex<cURLs; ulIndex++)
		UnregProv(rgwURLs[ulIndex], clsidProv1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General: Register a 1000 URLs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_6()
{ 
	TBEGIN
	ULONG		ulIndex;
	LPOLESTR	rgwszURL[1000];
	WCHAR		pwszNum[4];
	CLSID		clsidTemp = DB_NULLGUID;

	memset(rgwszURL, 0, 1000*sizeof(LPOLESTR));

	for(ulIndex=0; ulIndex<1000; ulIndex++)
	{
		SAFE_ALLOC(rgwszURL[ulIndex], WCHAR, wcslen(L"X-IRP1000://111")+sizeof(WCHAR))

		_ultow(ulIndex, pwszNum, 10);
		wcscpy(rgwszURL[ulIndex], L"X-IRP1000://");
		wcscat(rgwszURL[ulIndex], pwszNum);

		TESTC_(m_hr = SetURLMapping(rgwszURL[ulIndex], 
			g_rgURLSchemes[ulIndex%10].clsid), S_OK)
	}

	for(ulIndex=0; ulIndex<1000; ulIndex++)
	{
		TESTC_(GetURLMapping(rgwszURL[ulIndex], &clsidTemp), S_OK)
		TESTC(clsidTemp == g_rgURLSchemes[ulIndex%10].clsid)
	}

	for(ulIndex=0; ulIndex<1000; ulIndex++)
		TESTC_(UnregProv(rgwszURL[ulIndex], g_rgURLSchemes[ulIndex%10].clsid), S_OK)

	TESTC_(GetURLMapping(L"X-IRP1000://0", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRP1000://111", &clsidTemp), S_FALSE)
	TESTC_(GetURLMapping(L"X-IRP1000://999", &clsidTemp), S_FALSE)

CLEANUP:
	for(ulIndex=0; ulIndex<10; ulIndex++)
		UnregProv(rgwszURL[ulIndex], g_rgURLSchemes[ulIndex%10].clsid);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Incomplete entry.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_7()
{ 
	TBEGIN
	HRESULT			hr=E_FAIL;
	BOOL			bFound = FALSE;
	CHAR			szKeyName[200];
	LONG			lResult;
	HKEY			hKey = NULL;
	HKEY			hKey2 = NULL;
	CHAR*			pdest=NULL;
	LONG			nRegCreateKey=0;
	HKEY			hSchemeKey=NULL;
    DWORD			dwDisposition;
	CLSID			clsidTemp;

	strcpy(szKeyName, "SOFTWARE\\Microsoft\\DataAccess\\RootBinder\\");

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						   szKeyName,
						   0,
						   KEY_READ,
						   &hKey);

	TESTC( lResult == ERROR_SUCCESS )

    nRegCreateKey = RegCreateKeyEx(hKey, 
							             "X-IRPIncomplEntry", 
                                         0, 
							             NULL,
                                         0,
                                         KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_CREATE_SUB_KEY | KEY_SET_VALUE,
                                         NULL,
                                         &hSchemeKey,
                                         &dwDisposition);

    TESTC(ERROR_SUCCESS == nRegCreateKey)

	TESTC_(GetURLMapping(L"X-IRPIncomplEntry", &clsidTemp), S_FALSE);

	strcat(szKeyName, "X-IRPIncomplEntry");

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						   szKeyName,
						   0,
						   KEY_READ,
						   &hKey2);

	TESTC( lResult == ERROR_SUCCESS )

CLEANUP:
	if(hKey && hSchemeKey)
	{
		RegCloseKey(hSchemeKey);
		COMPARE(RegDeleteKey(hKey, "X-IRPIncomplEntry"), ERROR_SUCCESS);
	}
	RegCloseKey(hKey);
	RegCloseKey(hKey2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCSpecialCases::Terminate()
{ 
	COMPARE(UnregisterAll(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRegProv::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END
