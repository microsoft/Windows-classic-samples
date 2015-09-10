#include "stdafx.h"
#include "utils.h"

long g_lObjsInUse = 0;
long g_lServerLocks = 0;
DWORD g_dwMainThreadID = 0;

TCHAR szClassKey[MAX_PATH];
TCHAR szAppIDKey[MAX_PATH];
TCHAR szClassRoot[] = _T("CLSID\\");
TCHAR szAppIDRoot[] = _T("SOFTWARE\\Classes\\AppID\\");
TCHAR szCLSID[] = _T("{C48FF713-B257-4242-B1FF-A86B61DF3B3E}");

void FormatObjectPath()
{
	HRESULT hr;

	// Compose the Classes Key of COM server
	hr = StringCchPrintf(szClassKey,MAX_PATH,TEXT("%s%s"),szClassRoot,szCLSID);
    	
	// Compose the AppID Key of COM server
	hr = StringCchPrintf(szAppIDKey,MAX_PATH,TEXT("%s%s"),szAppIDRoot,szCLSID);
}

// This code demonstrate the COM server registration code. Generally this happens during the installation of the component.
HRESULT RegisterServer()
{	
	HKEY  hCLSIDKey1 = NULL, hCLSIDKey2 = NULL, hInProcSvrKey = NULL, hImpCategoriesKey = NULL, hIIDMarshalKey = NULL;
	LONG  lRet;
	HRESULT hr = S_OK;
	TCHAR szModulePath [MAX_PATH];
	TCHAR szClassDescription[] = _T("NotifyInterfaceImp class");
	TCHAR szRunAs[] = _T("Interactive User");

	try
	{
		// Create a key under CLSID for our COM server.
		lRet = RegCreateKeyEx ( HKEY_CLASSES_ROOT, szClassKey,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
			NULL, &hCLSIDKey1, NULL );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// The default value of the key is a human-readable description of the coclass.
		lRet = RegSetValueEx ( hCLSIDKey1, NULL, 0, REG_SZ, (const BYTE*) szClassDescription,
			sizeof(szClassDescription) );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// Create the "AppID" key
		lRet = RegSetValueEx ( hCLSIDKey1, _T("AppID"), 0, REG_SZ, (const BYTE*) szCLSID,
			sizeof(szCLSID) );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// Create the LocalServer32 key, which holds info about our coclass.
		lRet = RegCreateKeyEx ( hCLSIDKey1, _T("LocalServer32"), 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, NULL, &hInProcSvrKey, NULL );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// The default value of the LocalServer32 key holds the full path to our executable.
		GetModuleFileName (NULL, szModulePath, MAX_PATH );
		lRet = RegSetValueEx ( hInProcSvrKey, NULL, 0, REG_SZ, (const BYTE*) szModulePath, 
			sizeof(TCHAR) * (_tcslen(szModulePath)+1) );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// Create the "Implemented Categories" key
		lRet = RegCreateKeyEx ( hCLSIDKey1, _T("Implemented Categories"), 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, NULL, &hImpCategoriesKey, NULL );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// Create the IID_IMarshal key
		LPWSTR strIIDIMarshal = NULL;
		StringFromCLSID(IID_IMarshal, &strIIDIMarshal);

		lRet = RegCreateKeyEx ( hImpCategoriesKey, strIIDIMarshal, 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, NULL, &hIIDMarshalKey, NULL );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}
		CoTaskMemFree(strIIDIMarshal);

		// Create a key under SOFTWARE\\Classes\\AppID for our COM server.
		lRet = RegCreateKeyEx ( HKEY_LOCAL_MACHINE, szAppIDKey,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
			NULL, &hCLSIDKey2, NULL );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		// The default value of the key is a human-readable description of the coclass.
		lRet = RegSetValueEx ( hCLSIDKey2,  _T("RunAs"), 0, REG_SZ, (const BYTE*) szRunAs, sizeof(szRunAs) );

		if ( ERROR_SUCCESS != lRet )
		{
			throw lRet;
		}

		hr = S_OK;
	}
    catch(HRESULT err)
    {
        printf("Registration failed - %x", err);
        UnRegisterServer();
    }
        
    if ( NULL != hCLSIDKey1 )
    {
        RegCloseKey ( hCLSIDKey1 );
    }

    if ( NULL != hCLSIDKey2 )
    {
        RegCloseKey ( hCLSIDKey2 );
    }

    if ( NULL != hInProcSvrKey )
    {
        RegCloseKey ( hInProcSvrKey );
    }

    if ( NULL != hImpCategoriesKey )
    {
        RegCloseKey ( hImpCategoriesKey );
    }

    if ( NULL != hIIDMarshalKey )
    {
        RegCloseKey ( hIIDMarshalKey );
    }

    return hr;
}

HRESULT UnRegisterServer()
{
	//Delete the ClassID key
	RegDeleteTree( HKEY_CLASSES_ROOT, szClassKey);
	//Delete the AppID key
	RegDeleteTree( HKEY_LOCAL_MACHINE, szAppIDKey);
	return S_OK;
}
