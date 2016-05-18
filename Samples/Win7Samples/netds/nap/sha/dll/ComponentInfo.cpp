// ComponentInfo.cpp : Implementation of CComponentInfo

#include "stdafx.h"
#include "ComponentInfo.h"
#include "DebugHelper.h"

#include "SdkCommon.h"
using namespace SDK_SAMPLE_COMMON;

#include <assert.h>

// CComponentInfo

STDMETHODIMP CComponentInfo::GetFriendlyName(MessageId * pFriendlyName)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pFriendlyName
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(pFriendlyName != NULL);

    // Put here the exact UINT32 MessageId for the friendly name
    *pFriendlyName = SDK_SAMPLE_FRIENDLY_NAME_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetDescription(MessageId * pDescription)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to description
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(pDescription != NULL);

    // Put here the exact UINT32 MessageId for the friendly name
    *pDescription = SDK_SAMPLE_SHA_DESCRIPTION_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetVendorName(MessageId * pVendorName)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pCompanyName
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(pVendorName != NULL);

    // Put here the UINT32 Message ID for the company name for this SHA
    *pVendorName = SDK_SAMPLE_COMPANY_NAME_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetVersion(MessageId * pVersion)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pVersionInfo
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(pVersion != NULL);

    // Put here the UINT32 Message ID for the version info
    *pVersion = SDK_SAMPLE_VERSION_INFO_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::
GetIcon(
    OUT CountedString** ppDllFilePath,
    OUT UINT* pIconResourceId)
{
    HRESULT hr = S_OK;
    HMODULE dllModuleHandle = NULL;

    // The caller should pass valid pointers to ppDllFilePath* and pIconResourceId
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert((ppDllFilePath != NULL) && (pIconResourceId != NULL));

    *ppDllFilePath = NULL;

    hr = AllocateMemory(*ppDllFilePath, MAX_PATH_WCHAR_SIZE);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    dllModuleHandle = GetModuleHandleW(SHA_SDK_SAMPLE_DLL_FILE_NAME);
    if (NULL == dllModuleHandle)
    {
      // Failed to get the module handle
      hr = HRESULT_FROM_WIN32(GetLastError());
      DebugPrintfW(L" ---          GetModuleHandleW failed to get the module handle (error = 0x%08x)",hr);
      goto Cleanup;
    }

    DWORD fileNameLength = 0;
    fileNameLength = GetModuleFileNameW(
                                        dllModuleHandle,
                                        (*ppDllFilePath)->string,
                                        (MAX_PATH+1));

    if (fileNameLength == (MAX_PATH+1) && ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
        hr = E_OUTOFMEMORY;
        DebugPrintfW(L" ---          buffer is too small to hold the module name (error = 0x%08x)",hr);
        // if failed goto cleanup to free the allocated memory
        goto Cleanup;
    }
    else if (fileNameLength == 0)
    {
        // Failed to get the module name
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintfW(L" ---          GetModuleFileNameW failed to get the module name (error = 0x%08x)",hr);
        goto Cleanup;
    }

    (*ppDllFilePath)->length = (UINT16) fileNameLength;

    *pIconResourceId = IDI_ICON1; // Defined in Resource.h


    Cleanup:
        if (FAILED(hr) && (*ppDllFilePath))
        {
            FreeMemory(*ppDllFilePath);
        }
        return hr;
}

STDMETHODIMP CComponentInfo::ConvertErrorCodeToMessageId(HRESULT errorCode, MessageId * pMsgId)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(pMsgId != NULL);

    // SDKNote:
    // The following cases are examples, add more error codes here
    switch (errorCode)
    {
        case QUAR_E_NOTPATCHED:
            *pMsgId = SDK_SAMPLE_CLIENT_NOT_PATCHED_MSG_ID;
            break;
        case S_OK:
            *pMsgId = SDK_SAMPLE_COMPLIANT_CLIENT_MSG_ID;
            break;
        default:
            // return an error
            hr = E_UNEXPECTED;
            DebugPrintfW(L" ---          Unknown error code (error = 0x%08x)",
            hr);
            break;

    }

    return hr;
}

STDMETHODIMP CComponentInfo::GetLocalizedString(MessageId msgId, CountedString ** ppString)
{
    HRESULT hr = S_OK;
    HMODULE dllModuleHandle = NULL;

    // The caller should pass valid pointers to ppString* and pRetrievedLangId
    // MIDL protects this on Retail builds
    // whereas the assert() will fire on Debug builds
    assert(ppString != NULL);

    *ppString = NULL;

    // Allocate memory for string
    hr = AllocateMemory(*ppString, MAX_PATH_WCHAR_SIZE);  
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    dllModuleHandle = GetModuleHandleW(SHA_SDK_SAMPLE_DLL_FILE_NAME);
    if (NULL == dllModuleHandle)
    {
      // Failed to get the module handle
      hr = HRESULT_FROM_WIN32(GetLastError());
      DebugPrintfW(L" ---          GetModuleHandleW failed to get the module handle (error = 0x%08x)",hr);
      goto Cleanup;
    }
	
    // Read from the resource file
    DWORD stringLength = 0;
    stringLength = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE,
                            dllModuleHandle,
                            msgId,
                            LANG_ID,
                            (*ppString)->string,
                            (MAX_PATH+1) ,
                            NULL);

    // if failed to read from the mc file, return error    
    if (stringLength == 0)  
    {
	hr = HRESULT_FROM_WIN32(GetLastError());        
        DebugPrintfW(L" ---          Error getting FormatMessageW (error = 0x%08x)", hr);
        goto Cleanup;
    }

    (*ppString)->length = (UINT16) stringLength;


Cleanup:
    if (FAILED(hr) &&(*ppString))
    {
        FreeMemory(*ppString);
    }
    return hr;
}







