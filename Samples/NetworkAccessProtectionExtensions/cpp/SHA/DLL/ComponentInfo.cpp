// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// ComponentInfo.cpp : Implementation of CComponentInfo

#include "ComponentInfo.h"
#include "DebugHelper.h"
#include "SdkCommon.h"

using namespace SDK_SAMPLE_COMMON;

extern HMODULE g_hModule;
extern LONG g_nComObjsInUse;

// {E19DDEC2-3FBE-4C3B-9317-679760C13AAE}
const CLSID CLSID_ComponentInfo = 
{ 0xe19ddec2, 0x3fbe, 0x4c3b, { 0x93, 0x17, 0x67, 0x97, 0x60, 0xc1, 0x3a, 0xae } };

// CComponentInfo

CComponentInfo::CComponentInfo() : 
    m_cRef(0)
{
    InterlockedIncrement(&g_nComObjsInUse) ;
}

CComponentInfo::~CComponentInfo()
{
    InterlockedDecrement(&g_nComObjsInUse);
}


STDMETHODIMP CComponentInfo::GetFriendlyName(
    __RPC__out MessageId * pFriendlyName)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pFriendlyName
    if (pFriendlyName == NULL)
    {
        return E_INVALIDARG;
    }

    // Put here the exact UINT32 MessageId for the friendly name
    *pFriendlyName = SDK_SAMPLE_FRIENDLY_NAME_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetDescription(
    __RPC__out MessageId * pDescription)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to description
    if (pDescription == NULL)
    {
        return E_INVALIDARG;
    }

    // Put here the exact UINT32 MessageId for the description
    *pDescription = SDK_SAMPLE_SHA_DESCRIPTION_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetVendorName(
    __RPC__out MessageId * pVendorName)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pVendorName
    if (pVendorName == NULL)
    {
        return E_INVALIDARG;
    }

    // Put here the UINT32 Message ID for the company name for this SHA
    *pVendorName = SDK_SAMPLE_COMPANY_NAME_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetVersion(
    __RPC__out MessageId * pVersion)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    // The caller should pass a valid pointer to pVersionInfo
    if (pVersion == NULL)
    {
        return E_INVALIDARG;
    }

    // Put here the UINT32 Message ID for the version info
    *pVersion = SDK_SAMPLE_VERSION_INFO_MSG_ID;

    return hr;
}

STDMETHODIMP CComponentInfo::GetIcon(
    __RPC__deref_out CountedString** ppDllFilePath,
    __RPC__out UINT* pIconResourceId)
{
    HRESULT hr = S_OK;

    // The caller should pass valid pointers to ppDllFilePath* and pIconResourceId
    if ((ppDllFilePath == NULL) || (pIconResourceId == NULL))
    {
        return E_INVALIDARG;
    }

    hr = AllocateMemory(*ppDllFilePath, MAX_PATH_WCHAR_SIZE);
    if (FAILED(hr))
    {
		*ppDllFilePath = NULL;
		goto Cleanup;
    }

    DWORD fileNameLength = 0;
    fileNameLength = GetModuleFileNameW(g_hModule,
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

STDMETHODIMP CComponentInfo::ConvertErrorCodeToMessageId(
    __RPC__in HRESULT errorCode, 
    __RPC__out MessageId * pMsgId)
{
    HRESULT hr = S_OK;

    // Check parameters make sure the pointer is not NULL
    if (pMsgId == NULL)
    {
        return E_INVALIDARG;
    }

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

STDMETHODIMP CComponentInfo::GetLocalizedString(
    MessageId msgId, 
    __RPC__deref_out CountedString ** ppString)
{
    HRESULT hr = S_OK;

    // The caller should pass valid pointers to ppString* and pRetrievedLangId
    if (ppString == NULL)
    {
        return E_INVALIDARG;
    }

    // Allocate memory for string
    hr = AllocateMemory(*ppString, MAX_PATH_WCHAR_SIZE);  
    if (FAILED(hr))
    {
		*ppString = NULL;
		goto Cleanup;
    }

    // Read from the resource file
    DWORD stringLength = 0;
    stringLength = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE,
                            g_hModule,
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


// Implementation of IUnknown

STDMETHODIMP CComponentInfo::QueryInterface(
    __RPC__in const IID& iid, 
    __RPC__out void** ppv)
{
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_INapComponentInfo)
    {
        *ppv = static_cast<INapComponentInfo*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG CComponentInfo::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CComponentInfo::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

