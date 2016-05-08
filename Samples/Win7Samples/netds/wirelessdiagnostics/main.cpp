// Main.cpp : Implementation of helper class extension.
//
// 
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// 
#include "precomp.h"
#pragma hdrstop



class CWirelessHelperExtensionModule : public CAtlExeModuleT< CWirelessHelperExtensionModule >
{
public :
	DECLARE_LIBID(LIBID_WirelessHelperExtensionLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_HELPER_EXTENSION, "{1b4031f0-979f-444c-b08e-2c244996eab3}")
};

CWirelessHelperExtensionModule _AtlModule;


OBJECT_ENTRY_AUTO(CLSID_WirelessHelperExtension, CWirelessHelperExtension)

#ifdef _WINDLL
#error This project does not support DLL, only EXE targets
#endif


// -------------------------------------------------------------------
DWORD
WlExtOpenRegKey(
            HKEY    hHive,
    __in    LPWSTR  pwszRegKey,
            BOOL    bCreate,
    __out   HKEY    *phKey
    )
//
// Opens the registry key in the hive specified
// if key not found and bCreate is present we attempt to create the key
//
{
    DWORD dwError = ERROR_SUCCESS;

    if (!hHive)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    dwError = RegOpenKeyEx (hHive, pwszRegKey, 0, KEY_ALL_ACCESS, phKey);
    if (dwError != ERROR_SUCCESS)
    {
        if (bCreate == FALSE)
        {
            return (dwError);
        }

        // Try to create the key
        dwError = RegCreateKeyEx (hHive,
                                  pwszRegKey,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  phKey,
                                  NULL);

        if (dwError != ERROR_SUCCESS)
        {
            return (dwError);
        }
    }

    if (!(*phKey))
    {
        // dont know why we failed
        return (ERROR_FUNCTION_FAILED);
    }

    return (dwError);
}


DWORD
WlExtSetRegistryValue(
                                HKEY    hHive,
    __in                const   LPWSTR  pwszKey,
    __in_opt            const   LPWSTR  pwszSubKey,
    __in_opt            const   LPWSTR  pwszValueName,
                                DWORD   dwType,
    __in_bcount(cbData) const   BYTE    *pData,
                                DWORD   cbData
    )
//
// Sets value pData of type dwType to registry key
// hHive\pwszKey\pwszSubkey
// If pwszSubkey is NULL then value is set to hHive\pwszKey
// If hHive\pwszKey or pwszSubkey is not present, the keys are created.
//

{
    DWORD       dwError = ERROR_SUCCESS;
    HKEY        hKey = NULL;                // handle to the reg key
    HKEY        hSubKey = NULL;             // handle to the reg key

    if ((!hHive) ||
        (!pwszKey) ||
        (!pData))
    {
        return ERROR_INVALID_PARAMETER;
    }

    // Try to open or create the registry key
    dwError = WlExtOpenRegKey (hHive, pwszKey, TRUE, &hKey);
    if ((dwError != ERROR_SUCCESS) ||
        (!hKey))
    {
        return (dwError);
    }

    if (pwszSubKey)
    {
        // Need to open this key as well
        dwError = WlExtOpenRegKey (hKey, pwszSubKey, TRUE, &hSubKey);
        if ((dwError != ERROR_SUCCESS) ||
            (!hSubKey))
        {
            RegCloseKey(hKey);
            return (dwError);
        }
    }

    // Now set the value
    RegSetValueEx ((pwszSubKey ? hSubKey : hKey),
                   pwszValueName,      // if this is null, value will be set to the key itself
                   0,
                   dwType,
                   pData,
                   cbData);

    RegCloseKey(hKey);
    if (hSubKey)
    {
        RegCloseKey(hSubKey);
    }

    return (dwError);
}


DWORD
WlExtDeleteRegistryKey(
                    HKEY    hHive,
    __in    const   LPWSTR  pwszSubKey
    )
//
//  Delete an entry in the registry of the form:
//          HKEY_CLASSES_ROOT\wszKey\wszSubkey = wszValue
//

{
   DWORD dwError = ERROR_SUCCESS;

    if ((!hHive) ||
        (!pwszSubKey))
    {
        return (ERROR_INVALID_PARAMETER);
    }

    // delete the registry key.
    dwError = SHDeleteKey (hHive, pwszSubKey);

    return dwError;
}



#define MAX_LENGTH                  256
#define ARRAY_SIZE(s)               (sizeof(s) / sizeof(s[0]))
#define CLSIDSTR_WL_HC_EXT          L"{00102030-4050-6070-8090-A0B0C0D0E0F0}"
#define CLSID_WL_EXT_FRIENDLY_NAME  L"WirelessHelperClassExtension"

HRESULT
WlExtRegisterServer()
{
    HRESULT hr = S_OK;

/*
    WCHAR  wszModule[MAX_PATH];                     // path name of server
    WCHAR wszCLSIDKey[MAX_LENGTH];                  // CLSID\\wszCLSID.

    if (0 == (GetModuleFileName (NULL, wszModule, ARRAY_SIZE(wszModule))))
    {
        // Trace error message
        return (HRESULT_FROM_WIN32 (GetLastError()));
    }

    // create entries under CLSID.
    // Description
    hr = WlExtSetRegistryValue (HKEY_CLASSES_ROOT,
                                L"CLSID",
                                CLSIDSTR_WL_HC_EXT,
                                NULL,
                                REG_SZ,
                                (PBYTE) CLSID_WL_EXT_FRIENDLY_NAME,
                                (((DWORD) wcslen (CLSID_WL_EXT_FRIENDLY_NAME)+1)*sizeof (wchar_t)));
    if (S_OK != hr)
    {
        return hr;
    }
*/

    hr = WlExtSetRegistryValue (HKEY_CLASSES_ROOT,
                                L"CLSID",
                                CLSIDSTR_WL_HC_EXT,
                                L"AppID",
                                REG_SZ,
                                (PBYTE) CLSIDSTR_WL_HC_EXT,
                                (((DWORD) wcslen (CLSIDSTR_WL_HC_EXT)+1)*sizeof (wchar_t)));
    if (S_OK != hr)
    {
        return hr;
    }

/*
    // get the class ID strings.
    hr = StringCchPrintf (wszCLSIDKey,
                          MAX_LENGTH-1,
                          L"%ws\\%ws",
                          L"CLSID",
                          CLSIDSTR_WL_HC_EXT);
    if (S_OK != hr)
    {
        return hr;
    }

    // set the server path.
    hr = WlExtSetRegistryValue (HKEY_CLASSES_ROOT,
                                wszCLSIDKey,
                                L"LocalServer32",
                                NULL,
                                REG_SZ,
                                (PBYTE) wszModule,
                                (((DWORD) wcslen (wszModule)+1)*sizeof (wchar_t)));
*/

    return hr;
}


HRESULT
WlExtUnRegisterServer(
    )
{
    HRESULT     hr = S_OK;
    wchar_t     wszCLSIDKey[MAX_LENGTH];                      // CLSID\\wszCLSID.

    // get the class ID strings.

    hr = StringCchPrintf (wszCLSIDKey,
                          MAX_LENGTH-1,
                          L"%ws\\%ws",
                          L"CLSID",
                          CLSIDSTR_WL_HC_EXT);
    if (S_OK != hr)
    {
        return hr;
    }

    // delete Class ID key
    hr = WlExtDeleteRegistryKey (HKEY_CLASSES_ROOT, wszCLSIDKey);

    return (hr);
}


/////////////////////////////////////////////////////////////////////////////
// EXE Entry Point


extern "C" int WINAPI
_tWinMain(
    __in    HINSTANCE   hInst,      // hInstance
    __in    HINSTANCE   hPrevInst,  // hPrevInstance
    __in    LPTSTR      lpCmdLine,  // lpCmdLine
    __in    int         nCmdShow
    )
{
    INT         RetCode;
    TCHAR       szTokens[] = _T("-/");
    TCHAR       *pszContext;
    TCHAR       *pszToken;

    UNREFERENCED_PARAMETER (hInst);
    UNREFERENCED_PARAMETER (hPrevInst);

    InitLogging (L"Wireless Helper Class Extension");

    LogEntry ((PVOID) (ULONG_PTR) nCmdShow);

    RetCode = _AtlModule.WinMain (nCmdShow);

    //
    // We need to do some post-processing to ensure that WMI knows where to write the security descriptor
    //
    pszToken = _tcstok_s (lpCmdLine, szTokens, &pszContext);
    while (pszToken)
    {
        if (0 == _tcsicmp (pszToken, _T("UnregServer")))
        {
            WlExtUnRegisterServer ();
            break;
        }
        else if (0 == _tcsicmp (pszToken, _T("RegServer")))
        {
            WlExtRegisterServer ();
            break;
        }

        pszToken = _tcstok_s (NULL, szTokens, &pszContext);
    }

    LogExit (RetCode);
    return (RetCode);
}

