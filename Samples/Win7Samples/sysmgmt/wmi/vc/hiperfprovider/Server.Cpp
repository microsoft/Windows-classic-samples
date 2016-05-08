////////////////////////////////////////////////////////////////////////

//

//  Server.cpp

//

//	Module:	WMI high performance provider sample code

//

//  Generic COM server framework, adapted for the BasicHiPerf provider 

//	sample.  This module contains nothing specific to the BasicHiPerf 

//	provider except what is defined in the section bracketed by the 

//	CLSID SPECIFIC comments below.

//

//

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <initguid.h>

#include "Factory.h"

/////////////////////////////////////////////////////////////////////////////
//
//  BEGIN CLSID SPECIFIC SECTION
//
//

// For new providers, this is the only area that needs to be modified in this
// file.  Include any appropriate header files, a unique CLSID and update 
// the macros.

#include "wbemprov.h"
#include "BasicHiPerf.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

// {B56B1D80-AA4F-11d2-B348-00105A1469B7}
DEFINE_GUID(CLSID_BasicHiPerf_v1,
0xb56b1d80, 0xaa4f, 0x11d2, 0xb3, 0x48, 0x0, 0x10, 0x5a, 0x14, 0x69, 0xb7);

#define IMPLEMENTED_CLSID           CLSID_BasicHiPerf_v1
#define SERVER_REGISTRY_COMMENT     L"WMI Basic HiPerf Provider"
#define CPP_CLASS_NAME              CHiPerfProvider
#define INTERFACE_CAST              (IWbemHiPerfProvider*)

//
//  END CLSID SPECIFIC SECTION
//
/////////////////////////////////////////////////////////////////////////////


HINSTANCE g_hInstance;
long g_lLocks = 0;
long g_lObjects = 0;

//***************************************************************************
//
//  DllMain
//
//  Dll entry point.
//
//  PARAMETERS:
//
//      HINSTANCE hinstDLL      The handle to our DLL.
//      DWORD dwReason          DLL_PROCESS_ATTACH on load,
//                              DLL_PROCESS_DETACH on shutdown,
//                              DLL_THREAD_ATTACH/DLL_THREAD_DETACH otherwise.
//      LPVOID lpReserved       Reserved
//
//  RETURN VALUES:
//
//      TRUE is successful, FALSE if a fatal error occured.
//      NT behaves very ugly if FALSE is returned.
//
//***************************************************************************
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        setlocale(LC_ALL, "");      // Set to the 'current' locale
        g_hInstance = hinstDLL;
    }

    return TRUE;
}



//***************************************************************************
//
//  DllGetClassObject
//
//  Standard OLE In-Process Server entry point to return an class factory
//  instance.
//
//  PARAMETERS:
//
//  RETURNS:
//
//      S_OK                Success
//      E_NOINTERFACE       An interface other that IClassFactory was asked for
//      E_OUTOFMEMORY
//      E_FAILED            Initialization failed, or an unsupported clsid was
//                          asked for.
//
//***************************************************************************

STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv
    )
{
    CClassFactory *pClassFactory = NULL;
	HRESULT hRes;

    //  Verify the caller is asking for our type of object
	// ===================================================

    if (IMPLEMENTED_CLSID == rclsid) 
	{
		// Create the class factory
		// ========================

		pClassFactory = new CClassFactory;

		if (!pClassFactory)
			return E_OUTOFMEMORY;
		
		hRes = pClassFactory->QueryInterface(riid, ppv);
		if (FAILED(hRes))
		{
			delete pClassFactory;
			return hRes;
		}
		hRes = S_OK;
	}
	else 
		hRes = CLASS_E_CLASSNOTAVAILABLE;

    return hRes;
}

//***************************************************************************
//
//  DllCanUnloadNow
//
//  Standard OLE entry point for server shutdown request. Allows shutdown
//  only if no outstanding objects or locks are present.
//
//  RETURN VALUES:
//
//      S_OK        May unload now.
//      S_FALSE     May not.
//
//***************************************************************************

STDAPI DllCanUnloadNow(void)
{
    HRESULT hRes = S_FALSE;

    if (0 == g_lLocks && 0 == g_lObjects)
        hRes = S_OK;

    return hRes;
}

//***************************************************************************
//
//  DllRegisterServer
//
//  Standard OLE entry point for registering the server.
//
//  RETURN VALUES:
//
//      S_OK        Registration was successful
//      E_FAIL      Registration failed.
//
//***************************************************************************

STDAPI DllRegisterServer(void)
{
    wchar_t Path[MAX_PATH + 1];
    wchar_t pGuidStr[128];
    wchar_t KeyPath[1024];

    // Get the dll's filename
    // ======================

	memset (Path, NULL, sizeof(Path));
    GetModuleFileNameW(g_hInstance, Path, sizeof(Path)/sizeof(wchar_t) - 1);

    // Convert CLSID to string.
    // ========================

    StringFromGUID2(IMPLEMENTED_CLSID, pGuidStr,(sizeof pGuidStr)/(sizeof pGuidStr[0]));
	StringCbPrintfW(KeyPath, sizeof(KeyPath), L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

    // Place it in registry.
    // CLSID\\CLSID_Nt5PerProvider_v1 : <no_name> : "name"
    //      \\CLSID_Nt5PerProvider_v1\\InProcServer32 : <no_name> : "path to DLL"
    //                                    : ThreadingModel : "both"
    // ==============================================================

    HKEY hKey;
    LONG lRes = RegCreateKeyExW( HKEY_LOCAL_MACHINE, KeyPath, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey, NULL );
    if (lRes)
        return E_FAIL;

    wchar_t *pName = SERVER_REGISTRY_COMMENT; 
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, 
		(DWORD)(wcslen(pName) * sizeof(wchar_t) + sizeof(wchar_t)));

    HKEY hSubkey;
    lRes = RegCreateKeyEx(hKey, "InprocServer32", 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, 
					NULL, &hSubkey, NULL);

    RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, 
		(DWORD)(wcslen(Path) * sizeof(wchar_t) + sizeof(wchar_t)));
    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (const BYTE *) L"Both", 
		(DWORD)(wcslen(L"Both") * sizeof(wchar_t) + sizeof(wchar_t)));

    RegCloseKey(hSubkey);
    RegCloseKey(hKey);


    return S_OK;
}

//***************************************************************************
//
//  DllUnregisterServer
//
//  Standard OLE entry point for unregistering the server.
//
//  RETURN VALUES:
//
//      S_OK        Unregistration was successful
//      E_FAIL      Unregistration failed.
//
//***************************************************************************

STDAPI DllUnregisterServer(void)
{
    wchar_t pGuidStr[128];
    HKEY hKey;
    wchar_t KeyPath[256];

    StringFromGUID2(IMPLEMENTED_CLSID, pGuidStr,(sizeof pGuidStr)/(sizeof pGuidStr[0]));
	StringCbPrintfW(KeyPath, sizeof(KeyPath), L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

    // Delete InProcServer32 subkey.
    // =============================
    LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, KeyPath, 0, KEY_WRITE, &hKey);
    if (lRes)
        return E_FAIL;


    RegDeleteKeyW(hKey, L"InprocServer32");
    RegCloseKey(hKey);

    // Delete CLSID GUID key.
    // ======================

    lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Classes\\CLSID", 0, KEY_WRITE, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    return S_OK;
}
