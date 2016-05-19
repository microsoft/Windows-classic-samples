#include "Regutil.h"
#include "shvuicf.h"
#include "Registration.h"
#include "strsafe.h"


STDMETHODIMP
ShvUIRegisterServer()
{
    HRESULT hr = S_OK;
    WCHAR  wszModule[MAX_PATH];                     // path name of server
    WCHAR wszCLSIDKey[MAX_LENGTH];                  // CLSID\\wszCLSID.
    DWORD result = ERROR_SUCCESS;

    result = GetModuleFileName(
                    ShvUIClassFactory::s_hModule, 
                    wszModule, 
                    ARRAYSIZE(wszModule));

    if (result == 0)
    {
        // Trace error message
        return HRESULT_FROM_WIN32(GetLastError());
    }

  
    // create entries under CLSID.
    // Description
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    REGCLSID,
                    CLSIDSTR_MS_SHVUI,
                    NULL,
                    REG_SZ,
                    (PBYTE)CLSID_MS_SHVUI_FRIENDLY_NAME,
                    (DWORD)(wcslen(CLSID_MS_SHVUI_FRIENDLY_NAME)+1)*sizeof (wchar_t));
	if (FAILED(hr))
		goto cleanup;

    // Add AppID value under CLSIDSTR_MS_SHVUI. 
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    REGCLSID,
                    CLSIDSTR_MS_SHVUI,
                    REGAPPID,
                    REG_SZ,
                    (PBYTE)STR_APPID_SDK_SHV_CONFIG,
                    (DWORD)(wcslen(STR_APPID_SDK_SHV_CONFIG)+1)*sizeof (wchar_t));
    
    if (FAILED(hr))
		goto cleanup;

    // get the class ID strings.
    hr = StringCchPrintf(wszCLSIDKey, MAX_LENGTH-1, L"%ws\\%ws", REGCLSID, CLSIDSTR_MS_SHVUI);
    if (FAILED(hr))
	    goto cleanup;
   

    // set the server path.
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    wszCLSIDKey, 
                    LOCALSERVER32,
                    NULL,
                    REG_SZ,
                    (PBYTE)wszModule,
                    (DWORD)(wcslen(wszModule)+1)*sizeof (wchar_t));

cleanup:
    return hr;    
}

STDMETHODIMP
ShvUIUnRegisterServer()
{
    HRESULT hr = S_OK;

    wchar_t wszCLSIDKey[MAX_LENGTH];                      // CLSID\\wszCLSID.
    
    // get the class ID strings.

    hr = StringCchPrintf(wszCLSIDKey, MAX_LENGTH-1, L"%ws\\%ws", REGCLSID, CLSIDSTR_MS_SHVUI);
	if (FAILED(hr))
		goto cleanup;


    // delete Class ID key
    hr = ShvuiDeleteRegistryKey(
                    HKEY_CLASSES_ROOT,
                    wszCLSIDKey);
	if (FAILED(hr))
		goto cleanup;

cleanup:
    return hr;       
}

