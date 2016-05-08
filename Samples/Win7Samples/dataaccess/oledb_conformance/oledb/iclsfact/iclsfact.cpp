//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ICLSFACT.CPP | OLE DB IClassFactory tests for Provider, 
// Provider's Error Lookup Service and the SDK Error object.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "iclsfact.h"
#include "msdaguid.h"		// ExtendedError
#include "msdadc.h"			// DataConversion 


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xad830a70, 0x5fd7, 0x11cf, { 0x97, 0x63, 0x00, 0xaa, 0x00, 0xbd, 0xf9, 0x52 }};
DECLARE_MODULE_NAME("IClassFactory");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("OLE DB Class Factory tests for Provider, Provider's Error Lookup Service and SDK Error Object.");
DECLARE_MODULE_VERSION(823743346);
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

class CClsFac : public COLEDB
{
protected:
	CLSID			m_Clsid;
	CLSCTX			m_ClsCtx;
	TCHAR			m_wszLibFileName[MAX_PATH];
	IUnknown *		m_pIUnknown;
	IUnknown *		m_pIUnknown2;
	IClassFactory * m_pIClassFactory;
public:	
	// @cmember CTOR
	CClsFac(LPWSTR wszTestCaseName);
	// @cmember DTOR
	virtual ~CClsFac(){}
	
	// @cmember This function must be called before
	// executing any member function so that the 
	// correct component CLSID is used for valid calls.
	BOOL SetComponent(CLSID ComponentClsid, CLSCTX ComponentClsCtx);	

	// @cmember This function will return the 
	// CLSID for the EXTENDEDERRORS CLSID.
	CLSID GetExtendedErrorsCLSID();	

	// @cmember This function will return the 
	// CLSID for the Provider's Enumerator CLSID.
	CLSID GetProviderEnumCLSID();	
};

class CDllGetClassObj : public CClsFac
{

protected:
	

public:
	// @cmember CTOR
	CDllGetClassObj(LPWSTR wszTestCaseName) : CClsFac(wszTestCaseName){};
	// @cmember DTOR
	virtual ~CDllGetClassObj(){};
	
	//-----------------------------------------
	// Member functions which perform each test
	//-----------------------------------------
	
	// @cmember Valid creation with CoGetClassObject
	int ValidCreationCoGetClassObject();
	// @cmember Valid creation with LoadLibraryExEx
	int ValidCreationLoadLibraryEx();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int NullppvObj();
	// @cmember E_NOINTERFACE - Invalid riid
	int Invalidriid();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryExEx, Bad CLSID
	int Invalidrclsid();
	
};


class CCreateInstance : public CClsFac
{
protected:
	
public:	
	// @cmember CTOR
	CCreateInstance(LPWSTR wszTestCaseName) : CClsFac(wszTestCaseName){};
	// @cmember DTOR
	virtual ~CCreateInstance(){};

	//-----------------------------------------
	// Member functions which perform each test
	//-----------------------------------------
	
	// @cmember Valid creation of 2 objects, release in same order
	int MultipleCreateReleaseSameOrder();	
	// @cmember Valid creation of 2 objects, release in opposite order
	int MultipleCreateReleaseOppositeOrder();
	// @cmember Call LockServer after CreateInstance
	int LockServerAfter();
	// @cmember Non null pUnkOuter, takes expected HRESULT
	int ValidpUnkOuter(HRESULT hr);
	// @cmember Creates an instance for each supported interface
	int CreateEachInterface(ULONG cIids, IID * rgIids);
	// @cmember E_NOINTERFACE for Invalid riid
	int InvalidRiid();
	// @cmember E_INVALIDARG for Null ppvObj
	int NullppvObj();
	
};

class CLockServer : public CClsFac
{
protected:
	
public:	
	// @cmember CTOR
	CLockServer(LPWSTR wszTestCaseName) : CClsFac(wszTestCaseName){};
	// @cmember DTOR
	virtual ~CLockServer(){};

	//-----------------------------------------
	// Member functions which perform each test
	//-----------------------------------------
	
	// @cmember LockServer and CreateInstance
	int LockAndCreate();
	// @cmember LockServer and call CoFreeUnusedLibraries
	int LockAndFree();
	// @cmember LockServer twice
	int LockTwice();
	// @cmember LockServer, CreateInstance and Unlock Server
	int LockAndCreateAndUnlock();
	// @cmember LockServer, Unlock Server and Create
	int LockAndUnlockAndCreate();

	
};


//--------------------------------------------------------------------
//	  CClsFac				CClsFac				CClsFac
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// @mfunc Constructor
//
CClsFac::CClsFac(LPWSTR wszTestCaseName) : COLEDB(wszTestCaseName)
{
	m_Clsid = GUID_NULL;
	m_ClsCtx = CLSCTX_INPROC_SERVER;
	wcscpy((wchar_t*)m_wszLibFileName, (wchar_t*)"");
	m_pIUnknown = NULL;
	m_pIUnknown2 = NULL;
	m_pIClassFactory = NULL;
}


//--------------------------------------------------------------------
// @mfunc Records the components CLSID to be tested in this test case,
// and uses it to find the dll path and name based in the registry.
//
BOOL CClsFac::SetComponent(CLSID ComponentClsid, CLSCTX ComponentClsCtx)
{
	DWORD	cbDllName = sizeof(m_wszLibFileName);	
	HKEY	SubKey = 0;
	BOOL	fResults = FALSE;
	TCHAR	wszSubKeyIProc[MAX_PATH+1];
	TCHAR	wszSubKeyOProc[MAX_PATH+1];
	LPWSTR	wszClsid = NULL;
	LPTSTR	szClsid1 = NULL;
	 
	//Record the class id for the component to be tested
	m_Clsid = ComponentClsid;
	m_ClsCtx = ComponentClsCtx;

	//Make sure we allocated memory ok 
	szClsid1 = (LPTSTR)PROVIDER_ALLOC(CLSID_WCHAR_SIZE_IN_BYTES);
	if (!szClsid1)
		goto CLEANUP;

	//Get string form of component's class ID
	if (SUCCEEDED(StringFromCLSID(ComponentClsid, &wszClsid)))
	{				
		//Plug in clsid string to typical registry key path
		_tcscpy(wszSubKeyIProc, _T("CLSID\\"));
		_tcscpy(wszSubKeyOProc, _T("CLSID\\"));
		#ifndef UNICODE
			WideCharToMultiByte(CP_ACP,0,wszClsid,-1,szClsid1,CLSID_WCHAR_SIZE_IN_BYTES,NULL,NULL);
			_tcscat(wszSubKeyIProc, szClsid1);
			_tcscat(wszSubKeyOProc, szClsid1);
		#else
			wcscat((wchar_t*)wszSubKeyIProc, (wchar_t*)wszClsid);
			wcscat((wchar_t*)wszSubKeyOProc, (wchar_t*)wszClsid);
		#endif
		_tcscat(wszSubKeyIProc, _T("\\InprocServer32"));
		_tcscat(wszSubKeyOProc, _T("\\LocalServer32"));

		//Lookup the dll path and name and store it for use in LoadLibraryEx
		if( (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, wszSubKeyIProc, 0, KEY_QUERY_VALUE, &SubKey)) ||
			(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, wszSubKeyOProc, 0, KEY_QUERY_VALUE, &SubKey)) )
		{
			if (ERROR_SUCCESS == RegQueryValueEx(SubKey, NULL, NULL, NULL, 
				(BYTE *)m_wszLibFileName, &cbDllName))			
			{
				fResults = TRUE;
				goto CLEANUP;
			}
		}
		
		//If we got here, there was a registry error
		odtLog << wszErrorReadingRegistry;
	}

CLEANUP:	

	// Close the SubKey
	if(SubKey) RegCloseKey(SubKey);
	
	PROVIDER_FREE(wszClsid);
	PROVIDER_FREE(szClsid1);
	
	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Records the components CLSID to be tested in this test case,
// and uses it to find the CLSID for Extended Error Lookup.
//
CLSID CClsFac::GetExtendedErrorsCLSID()
{
	DWORD	cbDllName = sizeof(m_wszLibFileName);	
	HKEY	SubKey = 0;
	CLSID	ExtErrCLSID = GUID_NULL;
	TCHAR	wszSubKey[MAX_PATH+1];	
	TCHAR	szSubKeyName[MAX_PATH+1];	
	LPWSTR	wpszKeyCLSID = NULL;
	LPWSTR	wszClsid = NULL;
	LPTSTR	szClsid1 = NULL;
	 
	//Make sure we allocated memory ok 
	szClsid1 = (LPTSTR)PROVIDER_ALLOC(CLSID_WCHAR_SIZE_IN_BYTES);
	if (!szClsid1)
		goto CLEANUP;

	//Get string form of provider class ID
	if (SUCCEEDED(StringFromCLSID(m_pThisTestModule->m_ProviderClsid, &wszClsid)))
	{				
		//Plug in clsid string to typical registry key path
		_tcscpy(wszSubKey, _T("CLSID\\"));
		#ifndef UNICODE
			WideCharToMultiByte(CP_ACP,0,wszClsid,-1,szClsid1,CLSID_WCHAR_SIZE_IN_BYTES,NULL,NULL);
			_tcscat(wszSubKey, szClsid1);
		#else
			wcscat((wchar_t*)wszSubKey, (wchar_t*)wszClsid);
		#endif
		
		_tcscat(wszSubKey, _T("\\EXTENDEDERRORS"));

		//Lookup the dll path and name and store it for use in LoadLibraryEx
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, wszSubKey, 0, KEY_ENUMERATE_SUB_KEYS, &SubKey))
		{
			//If we got here, there was a registry error
			odtLog << L"No Extended Error Support by the Provider\n";
			goto CLEANUP;
		}

		//Lookup the dll path and name and store it for use in LoadLibraryEx
		if (ERROR_SUCCESS == RegEnumKeyEx(SubKey, NULL, szSubKeyName, &cbDllName, 
															NULL, NULL, NULL, NULL))
		{
			wpszKeyCLSID = (LPWSTR)PROVIDER_ALLOC(MAX_PATH+1);
			MultiByteToWideChar(CP_ACP,0,(LPCSTR)szSubKeyName,-1,wpszKeyCLSID,MAX_PATH+1);
			CLSIDFromString((LPOLESTR)wpszKeyCLSID, &ExtErrCLSID);
		}
	}

CLEANUP:	
	
	// Close the SubKey
	if(SubKey) RegCloseKey(SubKey);
	
	PROVIDER_FREE(wszClsid);
	PROVIDER_FREE(szClsid1);
	PROVIDER_FREE(wpszKeyCLSID);
	
	return ExtErrCLSID;
}


//--------------------------------------------------------------------
// @mfunc Records the components CLSID to be tested in this test case,
// and uses it to find the CLSID for theProviders Enumerator.
//
CLSID CClsFac::GetProviderEnumCLSID()
{
	HKEY	SubKey = 0;
	CLSID	ExtErrCLSID = GUID_NULL;
	TCHAR	wszSubKey[MAX_PATH+1] = _T("");	
	TCHAR	szKeyName[MAX_PATH+1] = _T("");
	LONG	cbKeyName;
	LPWSTR	wszClsid = NULL;
	LPTSTR	szClsid1 = NULL;
	 
	//Make sure we allocated memory ok 
	szClsid1 = (LPTSTR)PROVIDER_ALLOC(CLSID_WCHAR_SIZE_IN_BYTES);
	if (!szClsid1)
		goto CLEANUP;

	//Get string form of provider class ID
	if (SUCCEEDED(StringFromCLSID(m_pThisTestModule->m_ProviderClsid, &wszClsid)))
	{				
		//Plug in clsid string to typical registry key path
		_tcscpy(wszSubKey, _T("CLSID\\"));
		#ifndef UNICODE
			WideCharToMultiByte(CP_ACP,0,wszClsid,-1,szClsid1,CLSID_WCHAR_SIZE_IN_BYTES,NULL,NULL);
			_tcscat(wszSubKey, szClsid1);
		#else
			wcscat((wchar_t*)wszSubKey, (wchar_t*)wszClsid);
		#endif
		
		//Lookup the dll path and name and store it for use in LoadLibraryEx
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, wszSubKey, 0, KEY_READ, &SubKey))
			if (ERROR_SUCCESS != RegQueryValue(SubKey, NULL, szKeyName, &cbKeyName))
				goto CLEANUP;

		_tcscat(szKeyName, _T(" Enumerator"));
	}

CLEANUP:	
	
	odtLog <<szKeyName <<ENDL;

	// Close the SubKey
	if(SubKey) RegCloseKey(SubKey);

	PROVIDER_FREE(wszClsid);
	PROVIDER_FREE(szClsid1);
	
	return ExtErrCLSID;
}


//--------------------------------------------------------------------
//	  CDllGetClassObject	CDllGetClassObject		CDllGetClassObject
//--------------------------------------------------------------------

	
//--------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
int CDllGetClassObj::ValidCreationCoGetClassObject()
{		
	BOOL fResults = FALSE;

	//Get a class factory from the component being tested
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx,  NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		// Invalid for Remoting AV
		if (m_ClsCtx == CLSCTX_INPROC_SERVER)
		{
			//Make sure that CreateInstance checks for NULL ppvObj
			m_hr=m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, NULL);
			if ((m_hr != E_INVALIDARG) && (m_hr != E_POINTER))
				CHECK(m_hr, E_INVALIDARG);
		}

		//Make sure we can use this class factory interface successfully
		if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
			(void **)&m_pIUnknown), S_OK))
		{
			//We've gotten the object successfully
			fResults = TRUE;

			if(m_pIUnknown->Release() != 0)
				odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
			m_pIUnknown = NULL;
		}

		if(m_pIClassFactory->Release() != 0)
			odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
		m_pIClassFactory = NULL;
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

//--------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
int CDllGetClassObj::ValidCreationLoadLibraryEx()
{
 	BOOL					fResults = FALSE;
	HINSTANCE				hinst;
	DLLGETCLASSOBJECTFUNC	proc;															  

	//Use LoadLibraryEx/GetProcAddress to access the DllGetClassObject
	if (hinst = LoadLibraryEx(m_wszLibFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH))
	{
		if (proc = (DLLGETCLASSOBJECTFUNC)GetProcAddress(hinst, "DllGetClassObject"))
			if (CHECK(m_hr = ((proc)(m_Clsid, IID_IClassFactory, (void**)&m_pIClassFactory)), S_OK)) 					
			{
				//Make sure we can use this class factory interface successfully
				if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
					(void **)&m_pIUnknown), S_OK))
				{
					//We've gotten the object successfully
					fResults = TRUE;

					if(m_pIUnknown->Release() != 0)
						odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
					m_pIUnknown = NULL;
				}

				if(m_pIClassFactory->Release() != 0)
					odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
				m_pIClassFactory = NULL;
			}	

		FreeLibrary(hinst);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
int CDllGetClassObj::NullppvObj()
{	
	BOOL					fResults = FALSE;
	HINSTANCE				hinst;
	DLLGETCLASSOBJECTFUNC	proc;															  

	//Use LoadLibraryEx/GetProcAddress to access the DllGetClassObject
	//We use LoadLibraryEx because CoGetClassObject tries to validate 
	//the pointer, causing an annoying Access Violation (which it still handles).
	if (hinst = LoadLibraryEx(m_wszLibFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH))
	{
		if (proc = (DLLGETCLASSOBJECTFUNC)GetProcAddress(hinst, "DllGetClassObject"))
		{
			//Pass a NULL ppvObj to DllGetClassObject
			m_hr = ((proc)(m_Clsid, IID_IClassFactory, NULL));

			if ((m_hr == E_INVALIDARG) || (m_hr == E_POINTER))
				fResults = TRUE;
		}
		
		FreeLibrary(hinst);
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
	
}

//--------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
int CDllGetClassObj::Invalidriid()
{
	//Pass an Invalid riid (IID_IRowset) to DllGetClassObject via CoGetClassObject
	m_hr = CoGetClassObject(m_Clsid, m_ClsCtx,  NULL, 
								IID_IRowset, (void **)&m_pIClassFactory);

	if ((m_hr == E_NOINTERFACE) || (m_hr == REGDB_E_CLASSNOTREG))
		return TEST_PASS;
	else
		return TEST_FAIL;


}


//--------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
int CDllGetClassObj::Invalidrclsid()
{
 	BOOL fResults = FALSE;
	HINSTANCE	hinst;
	DLLGETCLASSOBJECTFUNC proc;

	//Use LoadLibraryEx/GetProcAddress to access the DllGetClassObject
	if (hinst = LoadLibraryEx(m_wszLibFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH))
	{
		if (proc = (DLLGETCLASSOBJECTFUNC)GetProcAddress(hinst, "DllGetClassObject"))		
			//Now call the function with a bogus class id
			if (CHECK( m_hr = ((proc)(GUID_NULL, IID_IClassFactory, 
				(void **)&m_pIClassFactory)) , CLASS_E_CLASSNOTAVAILABLE)) 		
				//We got the correct return value
				fResults = TRUE;				

		FreeLibrary(hinst);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}


//--------------------------------------------------------------------
//	  CCreateInstance		CCreateInstance		CCreateInstance
//--------------------------------------------------------------------


//--------------------------------------------------------------------
//  @mfunc Valid creation of 2 objects, release in same order
//
int CCreateInstance::MultipleCreateReleaseSameOrder()
{
	BOOL fResults = FALSE;

	//Create first object
	if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
		(void **)&m_pIUnknown), S_OK))	
		
		//Create second object
		if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
			(void **)&m_pIUnknown2), S_OK))
		{
			//We've gotten the object successfully
			fResults = TRUE;

			//Release first object
			if(m_pIUnknown->Release() != 0)
				odtLog <<L"Release did not return 0 for CoCreateInstance." <<ENDL;
			m_pIUnknown = NULL;

			if(m_pIUnknown2->Release() != 0)
				odtLog <<L"Release did not return 0 for CoCreateInstance." <<ENDL;
			m_pIUnknown2 = NULL;
		}
		
	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(m_pIUnknown2);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}


//--------------------------------------------------------------------
// @mfunc Valid creation of 2 objects, release in opposite order
int CCreateInstance::MultipleCreateReleaseOppositeOrder()
//
{

	BOOL fResults = FALSE;

	//Create first object
	if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
		(void **)&m_pIUnknown), S_OK))	
		
		//Create second object
		if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
			(void **)&m_pIUnknown2), S_OK))
		{
			//We've gotten the object successfully
			fResults = TRUE;

			//Release second object
			if(m_pIUnknown2->Release() != 0)
				odtLog <<L"Release did not return 0 for CoCreateInstance." <<ENDL;
			m_pIUnknown2 = NULL;

			if(m_pIUnknown->Release() != 0)
				odtLog <<L"Release did not return 0 for CoCreateInstance." <<ENDL;
			m_pIUnknown = NULL;
		}

	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(m_pIUnknown2);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Call LockServer after CreateInstance
//
int CCreateInstance::LockServerAfter()
{
 	BOOL fResults = FALSE;
	HINSTANCE	hinst;
	DLLGETCLASSOBJECTFUNC proc;

	//Use LoadLibraryEx/GetProcAddress to access the IClassFactory interface
	if (hinst = LoadLibraryEx(m_wszLibFileName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH))
	{
		if (proc = (DLLGETCLASSOBJECTFUNC)GetProcAddress(hinst, "DllGetClassObject"))					
			if (CHECK( m_hr = ((proc)(m_Clsid, IID_IClassFactory, 
				(void **)&m_pIClassFactory)) , S_OK)) 		
			{	
				//Call CreateInstance first
				if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
					(void **)&m_pIUnknown), S_OK))
				{
					//Call LockServer next
					if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
					{
						fResults = TRUE;
						CHECK(m_pIClassFactory->LockServer(FALSE), S_OK);
					}
					
					SAFE_RELEASE(m_pIUnknown);
				}

				SAFE_RELEASE(m_pIClassFactory);
			}

		FreeLibrary(hinst);
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;			
}

//--------------------------------------------------------------------
// @mfunc Non null pUnkOuter, takes expected HRESULT
//
int CCreateInstance::ValidpUnkOuter(HRESULT hr)
{
	BOOL fResults = FALSE;

	//Make a bogus IUnkOuter, just to see if its taken by the object
	//which we are attempting to aggregate.
	IUnknown * pBogusIUnknown = NULL;
	if( !CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
		(void **)&pBogusIUnknown), S_OK) )	
		return TEST_FAIL;

	//Try to make an aggregated object, checking that the result is what user expected
	if (CHECK(CoCreateInstance(m_Clsid, pBogusIUnknown, CLSCTX_INPROC_SERVER, IID_IUnknown,
		(void **)&m_pIUnknown), hr))	
		fResults = TRUE;

	//Release object if we successfully created it
	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(pBogusIUnknown);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc Creates an instance for each supported interface
//
int CCreateInstance::CreateEachInterface(ULONG cIids, IID * rgIids)
{
	BOOL fResults = TRUE;	//This will only change if we fail at least once

	//Loop thru array of IIDs and create one object using each interface
	while (cIids > 0)
	{
		//Change the one based count into an array index
		cIids--;

		if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, rgIids[cIids],
			(void **)&m_pIUnknown), S_OK))
		{
			SAFE_RELEASE(m_pIUnknown);
		}
		else
			fResults = FALSE;	
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

//--------------------------------------------------------------------
// @mfunc E_NOINTERFACE for Invalid riid
//
int CCreateInstance::InvalidRiid()
{
	//Try to create the object asking for a invalid interface at this level
	if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IRowsetLocate,
			(void **)&m_pIUnknown), E_NOINTERFACE))	
		return TEST_PASS;
	else
		return TEST_FAIL;
		
}

//--------------------------------------------------------------------
// @mfunc E_INVALIDARG for Null ppvObj
//
int CCreateInstance::NullppvObj()
{
	if (CHECK(CoCreateInstance(m_Clsid, NULL,  m_ClsCtx, IID_IClassFactory, NULL), E_INVALIDARG))
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
//	  CLockServer			CLockServer			CLockServer
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc LockServer and CreateInstance
//	
int CLockServer::LockAndCreate()
{
	BOOL fResults = FALSE;

	//Check that our object can be created after locking and 
	//released after unlocking the server
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx,  NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
			if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
				(void **)&m_pIUnknown), S_OK))
				if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
				{
					//We've gotten the object successfully
					fResults = TRUE;

					if(m_pIUnknown->Release() != 0)
						odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
					m_pIUnknown = NULL;
				}
	
		if(m_pIClassFactory->Release() != 0)
			odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
		m_pIClassFactory = NULL;
	}
		
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

//--------------------------------------------------------------------
// @mfunc LockServer and call CoFreeUnusedLibraries
//		
int CLockServer::LockAndFree()
{
	BOOL fResults = FALSE;

	//Check that our object can be created after locking and 
	//calling CoFreeUnusedLibraries. Note, we can't really
	//verify that the library doesn't get freed and then reloaded,
	//but we can make sure that it doesn't blow up following a call
	//to CoFreeUnusedLibraries.
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx,  NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
		{
			CoFreeUnusedLibraries();
			if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
				(void **)&m_pIUnknown), S_OK))
				if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
				{
					//We've gotten the object successfully
					fResults = TRUE;

					if(m_pIUnknown->Release() != 0)
						odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
					m_pIUnknown = NULL;
				}
		}
	
		if(m_pIClassFactory->Release() != 0)
			odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
		m_pIClassFactory = NULL;
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}


//--------------------------------------------------------------------
// @mfunc LockServer twice
//			
int CLockServer::LockTwice()
{
	BOOL fResults = FALSE;

	//Check that our object can be created after locking twice and 
	//released before unlocking the server
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx,  NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
			if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
				if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
					(void **)&m_pIUnknown), S_OK))
					//Lock here again just to be sure we can do it after Create
					if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
						if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
						{
							if(m_pIUnknown->Release() != 0)
								odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
							m_pIUnknown = NULL;

							if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
								if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
									fResults = TRUE;
						}
							
		if(m_pIClassFactory->Release() != 0)
			odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
		m_pIClassFactory = NULL;
	}
		
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

//--------------------------------------------------------------------
// @mfunc LockServer, CreateInstance and Unlock Server
//				
int CLockServer::LockAndCreateAndUnlock()
{
	BOOL fResults = FALSE;

	//Check that our object can be created after locking twice and 
	//released before unlocking the server
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx, NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
			if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
				(void **)&m_pIUnknown), S_OK))
				if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
				{
					//We've gotten the object successfully
					fResults = TRUE;

					if(m_pIClassFactory->Release() != 0)
						odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
					m_pIClassFactory = NULL;

					if(m_pIUnknown->Release() != 0)
						odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
					m_pIUnknown = NULL;
				}
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
	

//--------------------------------------------------------------------
// @mfunc LockServer, Unlock Server and CreateInstance 
//				
int CLockServer::LockAndUnlockAndCreate()
{
	BOOL fResults = FALSE;

	//Check that our object can be created after locking twice and 
	//released before unlocking the server
	if (CHECK(CoGetClassObject(m_Clsid, m_ClsCtx, NULL, 
		IID_IClassFactory, (void **)&m_pIClassFactory), S_OK))
	{
		if (CHECK(m_pIClassFactory->LockServer(TRUE), S_OK))
			if (CHECK(m_pIClassFactory->LockServer(FALSE), S_OK))
				if (CHECK(m_pIClassFactory->CreateInstance(NULL, IID_IUnknown, 
					(void **)&m_pIUnknown), S_OK))					
				{
					//We've gotten the object successfully
					fResults = TRUE;

					if(m_pIClassFactory->Release() != 0)
						odtLog <<L"Release did not return 0 for CoGetClassObject." <<ENDL;
					m_pIClassFactory = NULL;

					if(m_pIUnknown->Release() != 0)
						odtLog <<L"Release did not return 0 for CreateInstance." <<ENDL;
					m_pIUnknown = NULL;
				}
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_ErrorObject)
//--------------------------------------------------------------------
// @class DllGetClassObject test for OLE DB Error Object
//
class DllGetClassObj_ErrorObject : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_ErrorObject,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_ErrorObject)
#define THE_CLASS DllGetClassObj_ErrorObject
BEG_TEST_CASE(DllGetClassObj_ErrorObject, CDllGetClassObj, L"DllGetClassObject test for OLE DB Error Object")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_Provider)
//--------------------------------------------------------------------
// @class DllGetClassObject test for Provider
//
class DllGetClassObj_Provider : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_Provider,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_Provider)
#define THE_CLASS DllGetClassObj_Provider
BEG_TEST_CASE(DllGetClassObj_Provider, CDllGetClassObj, L"DllGetClassObject test for Provider")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_ErrorLookup)
//--------------------------------------------------------------------
// @class DllGetClassObject test for Provider's Error Lookup service
//
class DllGetClassObj_ErrorLookup : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_ErrorLookup,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_ErrorLookup)
#define THE_CLASS DllGetClassObj_ErrorLookup
BEG_TEST_CASE(DllGetClassObj_ErrorLookup, CDllGetClassObj, L"DllGetClassObject test for Provider's Error Lookup service")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_ConversionLibrary)
//--------------------------------------------------------------------
// @class DllGetClassObject test for OLE DB Conversion Library Object
//
class DllGetClassObj_ConversionLibrary : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_ConversionLibrary,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_ConversionLibrary)
#define THE_CLASS DllGetClassObj_ConversionLibrary
BEG_TEST_CASE(DllGetClassObj_ConversionLibrary, CDllGetClassObj, L"DllGetClassObject test for OLE DB Conversion Library Object")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_OleDBEnumerator)
//--------------------------------------------------------------------
// @class DllGetClassObject test for OLE DB Root Enumerator Object
//
class DllGetClassObj_OleDBEnumerator : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_OleDBEnumerator,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_OleDBEnumerator)
#define THE_CLASS DllGetClassObj_OleDBEnumerator
BEG_TEST_CASE(DllGetClassObj_OleDBEnumerator, CDllGetClassObj, L"DllGetClassObject test for OLE DB Root Enumerator Object")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DllGetClassObj_MsdasqlEnumerator)
//--------------------------------------------------------------------
// @class DllGetClassObject test for MSDASQL's Enumerator Object
//
class DllGetClassObj_MsdasqlEnumerator : public CDllGetClassObj { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DllGetClassObj_MsdasqlEnumerator,CDllGetClassObj);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid creation with CoGetClassObject
	int Variation_1();
	// @cmember Valid creation with LoadLibraryEx
	int Variation_2();
	// @cmember E_INVALIDARG - ppvObj = NULL
	int Variation_3();
	// @cmember E_NOINTERFACE - Invalid riid
	int Variation_4();
	// @cmember CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DllGetClassObj_MsdasqlEnumerator)
#define THE_CLASS DllGetClassObj_MsdasqlEnumerator
BEG_TEST_CASE(DllGetClassObj_MsdasqlEnumerator, CDllGetClassObj, L"DllGetClassObject test for MSDASQL's Enumerator Object")
	TEST_VARIATION(1, 		L"Valid creation with CoGetClassObject")
	TEST_VARIATION(2, 		L"Valid creation with LoadLibraryEx")
	TEST_VARIATION(3, 		L"E_INVALIDARG - ppvObj = NULL")
	TEST_VARIATION(4, 		L"E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(5, 		L"CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_ErrorObject)
//--------------------------------------------------------------------
// @class CreateInstance test for OLE DB Error Object
//
class CreateInstance_ErrorObject : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_ErrorObject,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_ErrorObject)
#define THE_CLASS CreateInstance_ErrorObject
BEG_TEST_CASE(CreateInstance_ErrorObject, CCreateInstance, L"CreateInstance test for OLE DB Error Object")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_Provider)
//--------------------------------------------------------------------
// @class CreateInstance test for the Provider
//
class CreateInstance_Provider : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_Provider,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_Provider)
#define THE_CLASS CreateInstance_Provider
BEG_TEST_CASE(CreateInstance_Provider, CCreateInstance, L"CreateInstance test for the Provider")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_ErrorLookup)
//--------------------------------------------------------------------
// @class CreateInstance test for Provider's Error Lookup Service
//
class CreateInstance_ErrorLookup : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_ErrorLookup,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_ErrorLookup)
#define THE_CLASS CreateInstance_ErrorLookup
BEG_TEST_CASE(CreateInstance_ErrorLookup, CCreateInstance, L"CreateInstance test for Provider's Error Lookup Service")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_ConversionLibrary)
//--------------------------------------------------------------------
// @class CreateInstance test for OLE DB Conversion Library Object
//
class CreateInstance_ConversionLibrary : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_ConversionLibrary,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_ConversionLibrary)
#define THE_CLASS CreateInstance_ConversionLibrary
BEG_TEST_CASE(CreateInstance_ConversionLibrary, CCreateInstance, L"CreateInstance test for OLE DB Conversion Library Object")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_OleDBEnumerator)
//--------------------------------------------------------------------
// @class CreateInstance test for OLE DB Root Enumerator Object
//
class CreateInstance_OleDBEnumerator : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_OleDBEnumerator,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_OleDBEnumerator)
#define THE_CLASS CreateInstance_OleDBEnumerator
BEG_TEST_CASE(CreateInstance_OleDBEnumerator, CCreateInstance, L"CreateInstance test for OLE DB Root Enumerator Object")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CreateInstance_MsdasqlEnumerator)
//--------------------------------------------------------------------
// @class CreateInstance test for MSDASQL's Enumerator Object
//
class CreateInstance_MsdasqlEnumerator : public CCreateInstance { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CreateInstance_MsdasqlEnumerator,CCreateInstance);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Creations, Release in same order
	int Variation_1();
	// @cmember Multiple Creations, Release in opposite order
	int Variation_2();
	// @cmember LockServer After CreateInstance
	int Variation_3();
	// @cmember Valid pUnkOuter
	int Variation_4();
	// @cmember Create one object with each supported interface
	int Variation_5();
	// @cmember Invalid riid
	int Variation_6();
	// @cmember Null ppvObj
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CreateInstance_MsdasqlEnumerator)
#define THE_CLASS CreateInstance_MsdasqlEnumerator
BEG_TEST_CASE(CreateInstance_MsdasqlEnumerator, CCreateInstance, L"CreateInstance test for MSDASQL's Enumerator Object")
	TEST_VARIATION(1, 		L"Multiple Creations, Release in same order")
	TEST_VARIATION(2, 		L"Multiple Creations, Release in opposite order")
	TEST_VARIATION(3, 		L"LockServer After CreateInstance")
	TEST_VARIATION(4, 		L"Valid pUnkOuter")
	TEST_VARIATION(5, 		L"Create one object with each supported interface")
	TEST_VARIATION(6, 		L"Invalid riid")
	TEST_VARIATION(7, 		L"Null ppvObj")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_ErrorObject)
//--------------------------------------------------------------------
// @class LockServer test for the OLE DB Error Object
//
class LockServer_ErrorObject : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_ErrorObject,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_ErrorObject)
#define THE_CLASS LockServer_ErrorObject
BEG_TEST_CASE(LockServer_ErrorObject, CLockServer, L"LockServer test for the OLE DB Error Object")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_Provider)
//--------------------------------------------------------------------
// @class LockServer test for the Provider
//
class LockServer_Provider : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_Provider,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_Provider)
#define THE_CLASS LockServer_Provider
BEG_TEST_CASE(LockServer_Provider, CLockServer, L"LockServer test for the Provider")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_ErrorLookup)
//--------------------------------------------------------------------
// @class LockServer test for the Provider's Error Lookup Service
//
class LockServer_ErrorLookup : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_ErrorLookup,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_ErrorLookup)
#define THE_CLASS LockServer_ErrorLookup
BEG_TEST_CASE(LockServer_ErrorLookup, CLockServer, L"LockServer test for the Provider's Error Lookup Service")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_ConversionLibrary)
//--------------------------------------------------------------------
// @class LockServer test for OLE DB Conversion Library Object
//
class LockServer_ConversionLibrary : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_ConversionLibrary,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_ConversionLibrary)
#define THE_CLASS LockServer_ConversionLibrary
BEG_TEST_CASE(LockServer_ConversionLibrary, CLockServer, L"LockServer test for OLE DB Conversion Library Object")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_OleDBEnumerator)
//--------------------------------------------------------------------
// @class LockServer test for OLE DB Root Enumerator Object
//
class LockServer_OleDBEnumerator : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_OleDBEnumerator,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_OleDBEnumerator)
#define THE_CLASS LockServer_OleDBEnumerator
BEG_TEST_CASE(LockServer_OleDBEnumerator, CLockServer, L"LockServer test for OLE DB Root Enumerator Object")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LockServer_MsdasqlEnumerator)
//--------------------------------------------------------------------
// @class LockServer test for MSDASQL's Enumerator Object
//
class LockServer_MsdasqlEnumerator : public CLockServer { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LockServer_MsdasqlEnumerator,CLockServer);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember LockAndCreate
	int Variation_1();
	// @cmember LockAndFree
	int Variation_2();
	// @cmember LockTwice
	int Variation_3();
	// @cmember LockAndCreateAndUnlock
	int Variation_4();
	// @cmember LockAndUnlockAndCreate
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LockServer_MsdasqlEnumerator)
#define THE_CLASS LockServer_MsdasqlEnumerator
BEG_TEST_CASE(LockServer_MsdasqlEnumerator, CLockServer, L"LockServer test for MSDASQL's Enumerator Object")
	TEST_VARIATION(1, 		L"LockAndCreate")
	TEST_VARIATION(2, 		L"LockAndFree")
	TEST_VARIATION(3, 		L"LockTwice")
	TEST_VARIATION(4, 		L"LockAndCreateAndUnlock")
	TEST_VARIATION(5, 		L"LockAndUnlockAndCreate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(18, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, DllGetClassObj_ErrorObject)
	TEST_CASE(2, DllGetClassObj_Provider)
	TEST_CASE(3, DllGetClassObj_ErrorLookup)
	TEST_CASE(4, DllGetClassObj_ConversionLibrary)
	TEST_CASE(5, DllGetClassObj_OleDBEnumerator)
	TEST_CASE(6, DllGetClassObj_MsdasqlEnumerator)
	TEST_CASE(7, CreateInstance_ErrorObject)
	TEST_CASE(8, CreateInstance_Provider)
	TEST_CASE(9, CreateInstance_ErrorLookup)
	TEST_CASE(10, CreateInstance_ConversionLibrary)
	TEST_CASE(11, CreateInstance_OleDBEnumerator)
	TEST_CASE(12, CreateInstance_MsdasqlEnumerator)
	TEST_CASE(13, LockServer_ErrorObject)
	TEST_CASE(14, LockServer_Provider)
	TEST_CASE(15, LockServer_ErrorLookup)
	TEST_CASE(16, LockServer_ConversionLibrary)
	TEST_CASE(17, LockServer_OleDBEnumerator)
	TEST_CASE(18, LockServer_MsdasqlEnumerator)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_ErrorObject)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_ErrorObject - DllGetClassObject test for OLE DB Error Object
//|	Created:			02/05/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ErrorObject::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		//Set the correct component for this testcase
		return SetComponent(CLSID_EXTENDEDERRORINFO,CLSCTX_INPROC_SERVER);
	}
	
	return FALSE;		
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorObject::Variation_1()
{
	return ValidCreationCoGetClassObject();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorObject::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorObject::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorObject::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorObject::Variation_5()
{
	return Invalidrclsid();
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ErrorObject::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_Provider)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_Provider - DllGetClassObject test for Provider
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_Provider::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		return SetComponent(m_pThisTestModule->m_ProviderClsid, m_pThisTestModule->m_clsctxProvider);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_Provider::Variation_1()
{
	return ValidCreationCoGetClassObject();

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_Provider::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_Provider::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_Provider::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_Provider::Variation_5()
{
	return Invalidrclsid();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_Provider::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_ErrorLookup)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_ErrorLookup - DllGetClassObject test for Provider's Error Lookup service
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ErrorLookup::Init()
{
	CLSID ErrorLookupCLSID;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		// Get the CLSID out of the Registry
		ErrorLookupCLSID = GetExtendedErrorsCLSID();

		if(ErrorLookupCLSID != GUID_NULL)
			return SetComponent(ErrorLookupCLSID,CLSCTX_INPROC_SERVER);
		else
			return TEST_SKIPPED;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorLookup::Variation_1()
{
	return ValidCreationCoGetClassObject();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorLookup::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorLookup::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorLookup::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ErrorLookup::Variation_5()
{
	return Invalidrclsid();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ErrorLookup::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_ConversionLibrary)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_ConversionLibrary - DllGetClassObject test for OLE DB Conversion Library Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ConversionLibrary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_CONVERSIONLIBRARY,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ConversionLibrary::Variation_1()
{
	return ValidCreationCoGetClassObject();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ConversionLibrary::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ConversionLibrary::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ConversionLibrary::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_ConversionLibrary::Variation_5()
{
	return Invalidrclsid();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_ConversionLibrary::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_OleDBEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_OleDBEnumerator - DllGetClassObject test for OLE DB Root Enumerator Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_OleDBEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_OleDBEnumerator::Variation_1()
{
	return ValidCreationCoGetClassObject();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_OleDBEnumerator::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_OleDBEnumerator::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_OleDBEnumerator::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_OleDBEnumerator::Variation_5()
{
	return Invalidrclsid();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_OleDBEnumerator::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DllGetClassObj_MsdasqlEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		DllGetClassObj_MsdasqlEnumerator - DllGetClassObject test for MSDASQL's Enumerator Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_MsdasqlEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDllGetClassObj::Init())
	// }}
	{
		return SetComponent(CLSID_MSDASQL_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with CoGetClassObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_MsdasqlEnumerator::Variation_1()
{
	return ValidCreationCoGetClassObject();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid creation with LoadLibraryEx
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_MsdasqlEnumerator::Variation_2()
{
	return ValidCreationLoadLibraryEx();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - ppvObj = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_MsdasqlEnumerator::Variation_3()
{
	return NullppvObj();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_MsdasqlEnumerator::Variation_4()
{
	return Invalidriid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CLASS_E_CLASSNOTAVAILABLE - LoadLibraryEx, Bad CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DllGetClassObj_MsdasqlEnumerator::Variation_5()
{
	return Invalidrclsid();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DllGetClassObj_MsdasqlEnumerator::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDllGetClassObj::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_ErrorObject)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_ErrorObject - CreateInstance test for OLE DB Error Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ErrorObject::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		return SetComponent(CLSID_EXTENDEDERRORINFO,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_4()
{
	return ValidpUnkOuter(CLASS_E_NOAGGREGATION);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_5()
{
	GUID rgIids[2];

	//We have to do separate inits because IIDs are consts
	//and won't work with the direct initialization of the array
	rgIids[0] = IID_IErrorRecords;
	rgIids[1] =  IID_IErrorInfo;
	
	return CreateEachInterface(2, rgIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorObject::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ErrorObject::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_Provider)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_Provider - CreateInstance test for the Provider
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_Provider::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		 return SetComponent(m_pThisTestModule->m_ProviderClsid,m_pThisTestModule->m_clsctxProvider);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_4()
{
	//Aggregation is mandatory for provider,
	//this should return S_OK
	return ValidpUnkOuter(S_OK);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_5()
{
	
	const ULONG TOTAL_INTERFACES = 8;		//8 Possible interfaces on the DSO
	IID			rgIids[TOTAL_INTERFACES]; 
	IID			rgSupportedIids[TOTAL_INTERFACES];	
	ULONG		cSupportedIids = 0;	
	ULONG		ul;	
	
	//We have to do separate inits because IIDs are consts
	//and won't work with the direct initialization of the array
	
	//List of all optional interfaces we need to determine support for					
	rgIids[0] = IID_IUnknown;
	rgIids[1] = IID_IDBInitialize;
	rgIids[2] = IID_IDBProperties;
	rgIids[3] = IID_IDBCreateSession;
	rgIids[4] = IID_IDBInfo;
	rgIids[5] = IID_IPersistFile;
	rgIids[6] = IID_ISupportErrorInfo; 	
	rgIids[7] = IID_IPersist; 	
	
	//Record all the mandatory interfaces as supported	
	//NOTE:  We can't include IDBCreateSession and IDBProperties
	//because they can't be directly asked for on CreateInstance,
	//they require Initialization first.
								
	if (CHECK(CoCreateInstance(m_Clsid, NULL, m_ClsCtx, IID_IUnknown,
		(void **)&m_pIUnknown), S_OK))	
	{
		//Loop thru optional interfaces, finding out which ones are supported
		for (ul = 0; ul < TOTAL_INTERFACES; ul++)
		{
			if (S_OK == (m_hr = m_pIUnknown->QueryInterface(rgIids[ul], (void **)&m_pIUnknown2)))
			{
				//Record this interface in our supported array
				rgSupportedIids[cSupportedIids] = rgIids[ul];
				cSupportedIids++;				
				m_pIUnknown2->Release();
			}
			else
			{
				//Make sure we failed because it wasn't supported
				if( m_hr != E_NOINTERFACE )
					CHECK(m_hr, E_UNEXPECTED);
			}
		}

		//Release our first interface
		m_pIUnknown->Release();
	}
	
	//Now pass the array of supported interfaces to the function
	return CreateEachInterface(cSupportedIids, rgSupportedIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_Provider::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_Provider::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_ErrorLookup)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_ErrorLookup - CreateInstance test for Provider's Error Lookup Service
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ErrorLookup::Init()
{
	CLSID ErrorLookupCLSID;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		// Get the CLSID out of the Registry
		ErrorLookupCLSID = GetExtendedErrorsCLSID();

		if(ErrorLookupCLSID != GUID_NULL)
			return SetComponent(ErrorLookupCLSID,CLSCTX_INPROC_SERVER);
		else
			return TEST_SKIPPED;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_4()
{
	return ValidpUnkOuter(S_OK);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_5()
{
	//Only interface we support is IErrorLookup
	IID rgIids[1];
	
	rgIids[0] = IID_IErrorLookup;

	//Create object using this interface	
	return CreateEachInterface(1, rgIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ErrorLookup::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ErrorLookup::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_ConversionLibrary)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_ConversionLibrary - CreateInstance test for OLE DB Conversion Library Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ConversionLibrary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_CONVERSIONLIBRARY,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_4()
{
	return ValidpUnkOuter(S_OK);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_5()
{
	GUID rgIids[1];

	//We have to do separate inits because IIDs are consts
	//and won't work with the direct initialization of the array
	rgIids[0] = IID_IDataConvert;
	
	return CreateEachInterface(1, rgIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_ConversionLibrary::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_ConversionLibrary::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_OleDBEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_OleDBEnumerator - CreateInstance test for OLE DB Root Enumerator Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_OleDBEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_4()
{
	return ValidpUnkOuter(S_OK);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_5()
{
	GUID rgIids[2];

	//We have to do separate inits because IIDs are consts
	//and won't work with the direct initialization of the array
	rgIids[0] = IID_ISourcesRowset;
	rgIids[1] =  IID_IParseDisplayName;
	
	return CreateEachInterface(2, rgIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_OleDBEnumerator::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_OleDBEnumerator::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CreateInstance_MsdasqlEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		CreateInstance_MsdasqlEnumerator - CreateInstance test for MSDASQL's Enumerator Object
//|	Created:			02/06/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_MsdasqlEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCreateInstance::Init())
	// }}
	{
		return SetComponent(CLSID_MSDASQL_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in same order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_1()
{
	return MultipleCreateReleaseSameOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Creations, Release in opposite order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_2()
{
	return MultipleCreateReleaseOppositeOrder();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockServer After CreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_3()
{
	return LockServerAfter();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid pUnkOuter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_4()
{
	return ValidpUnkOuter(S_OK);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create one object with each supported interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_5()
{
	GUID rgIids[2];

	//We have to do separate inits because IIDs are consts
	//and won't work with the direct initialization of the array
	rgIids[0] = IID_ISourcesRowset;
	rgIids[1] =  IID_IParseDisplayName;
	
	return CreateEachInterface(2, rgIids);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_6()
{
	return InvalidRiid();
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Null ppvObj
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CreateInstance_MsdasqlEnumerator::Variation_7()
{
	return NullppvObj();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CreateInstance_MsdasqlEnumerator::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCreateInstance::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_ErrorObject)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_ErrorObject - LockServer test for the OLE DB Error Object
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ErrorObject::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		return SetComponent(CLSID_EXTENDEDERRORINFO,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorObject::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorObject::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorObject::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorObject::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorObject::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ErrorObject::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_Provider)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_Provider - LockServer test for the Provider
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_Provider::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		return SetComponent(m_pThisTestModule->m_ProviderClsid,m_pThisTestModule->m_clsctxProvider);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_Provider::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_Provider::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_Provider::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_Provider::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_Provider::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_Provider::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_ErrorLookup)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_ErrorLookup - LockServer test for the Provider's Error Lookup Service
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ErrorLookup::Init()
{
	CLSID ErrorLookupCLSID;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		// Get the CLSID out of the Registry
		ErrorLookupCLSID = GetExtendedErrorsCLSID();

		if(ErrorLookupCLSID != GUID_NULL)
			return SetComponent(ErrorLookupCLSID,CLSCTX_INPROC_SERVER);
		else
			return TEST_SKIPPED;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorLookup::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorLookup::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorLookup::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorLookup::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ErrorLookup::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ErrorLookup::Terminate()
{	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_ConversionLibrary)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_ConversionLibrary - LockServer test for OLE DB Conversion Library Object
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ConversionLibrary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_CONVERSIONLIBRARY,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ConversionLibrary::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ConversionLibrary::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ConversionLibrary::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ConversionLibrary::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_ConversionLibrary::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_ConversionLibrary::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_OleDBEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_OleDBEnumerator - LockServer test for OLE DB Root Enumerator Object
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_OleDBEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		return SetComponent(CLSID_OLEDB_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_OleDBEnumerator::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_OleDBEnumerator::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_OleDBEnumerator::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_OleDBEnumerator::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_OleDBEnumerator::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_OleDBEnumerator::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LockServer_MsdasqlEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		LockServer_MsdasqlEnumerator - LockServer test for MSDASQL's Enumerator Object
//|	Created:			02/07/96
//|	Updated:			04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_MsdasqlEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CLockServer::Init())
	// }}
	{
		return SetComponent(CLSID_MSDASQL_ENUMERATOR,CLSCTX_INPROC_SERVER);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_MsdasqlEnumerator::Variation_1()
{
	return LockAndCreate();
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LockAndFree
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_MsdasqlEnumerator::Variation_2()
{
	return LockAndFree();
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LockTwice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_MsdasqlEnumerator::Variation_3()
{
	return LockTwice();
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LockAndCreateAndUnlock
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_MsdasqlEnumerator::Variation_4()
{
	return LockAndCreateAndUnlock();
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LockAndUnlockAndCreate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LockServer_MsdasqlEnumerator::Variation_5()
{
	return LockAndUnlockAndCreate();
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LockServer_MsdasqlEnumerator::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CLockServer::Terminate());
}	// }}
// }}
// }}


