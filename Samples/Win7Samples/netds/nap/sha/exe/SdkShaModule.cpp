// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <NapUtil.h>
#include <naperror.h>
#include "SdkShaModule.h"

using namespace SDK_SAMPLE_COMMON;
using namespace SDK_SAMPLE_SHA;

/// Registers the SDK SHA with the NAP Server.
HRESULT CSdkShaModule::RegisterSdkSha()
{
    HRESULT hr = S_OK;
    /// Pointer to the INapClientManagement interface
    CComPtr<INapClientManagement> m_pNAPClientMgmt = NULL;
    /// Registration Information
    NapComponentRegistrationInfo m_shaInfo;

    ZeroMemory (&m_shaInfo, sizeof(m_shaInfo));

    hr = m_pNAPClientMgmt.CoCreateInstance(CLSID_NapClientManagement,
                                            NULL,
                                            CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        wprintf(L"RegisterSdkSha: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = FillShaComponentRegistrationInfo(&m_shaInfo);
    if(FAILED(hr))
    {
        wprintf(L"RegisterSdkSha:: FillShaComponentRegistrationInfo Failed with %#x",hr);
        goto Cleanup;
    }

    hr = m_pNAPClientMgmt->RegisterSystemHealthAgent(&m_shaInfo);
    if ( FAILED(hr) )
    {
        if  (NAP_E_CONFLICTING_ID == hr)
            wprintf(L"RegisterSdkSha: SdkSha registration with NapAgent returned NAP_E_CONFLICTING_ID - already registered?");
        else
            wprintf(L"RegisterSdkSha: Failed to register SdkSha with NapAgent -unknown (error = %x)\n", hr);

        goto Cleanup;
    }

    wprintf(L"RegisterSdkSha: SdkSha registration with NapAgent returned SUCCESS");

Cleanup:
    FreeComponentRegistration(&m_shaInfo);
    return hr;
}

/// Unregisters the SDK SHA with the NAP Server.
HRESULT CSdkShaModule::UnregisterSdkSha()
{
    HRESULT hr = S_OK;
    CComPtr<INapClientManagement> m_pNAPClientMgmt = NULL;

    hr = m_pNAPClientMgmt.CoCreateInstance(CLSID_NapClientManagement,
                                            NULL,
                                            CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        wprintf(L"UnregisterSdkSha: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = m_pNAPClientMgmt->UnregisterSystemHealthAgent(QuarSampleSystemHealthId);
    if ( FAILED(hr) )
    {
        if (NAP_E_STILL_BOUND == hr)
            wprintf(L"UnregisterSdkSha: UnregisterSystemHealthAgent failed with NAP_E_STILL_BOUND; stop 'SDKSHA /execute' prior to attempting Unregister.");
        else
            wprintf(L"UnregisterSdkSha: Failed to unregister SdkSha from NapAgent -unknown (error = %x)\n", hr);
            
        goto Cleanup;
    }        

    wprintf(L"UnregisterSdkSha: SdkSha unregistration from NapAgent returned SUCCESS");

Cleanup:
    return hr;
}

/// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
HRESULT CSdkShaModule::FillShaComponentRegistrationInfo (NapComponentRegistrationInfo *agentInfo)
{
    HRESULT hr = S_OK;
    agentInfo->id = QuarSampleSystemHealthId;
    agentInfo->infoClsid = CLSID_INFO; 

    hr = FillCountedString( SHA_FRIENDLY_NAME, &(agentInfo->friendlyName) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkSha::FillShaComponentRegistrationInfo(): AllocCountedString for friendlyName returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( SHA_DESCRIPTION, &(agentInfo->description) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkSha::FillShaComponentRegistrationInfo(): AllocCountedString for description returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( SHA_VERSION, &(agentInfo->version) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkSha::FillShaComponentRegistrationInfo(): AllocCountedString for version returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( SHA_VENDOR_NAME, &(agentInfo->vendorName) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkSha::FillShaComponentRegistrationInfo(): AllocCountedString for vendorName returned error %#x!",hr);
        goto Cleanup;
    }

Cleanup:
    return hr;
}


// Helper Function for releaseing ShaComponentRegistrationInfo members
void CSdkShaModule::FreeComponentRegistration(NapComponentRegistrationInfo *agentInfo)
{
    EmptyCountedString(&agentInfo->friendlyName);
    EmptyCountedString(&agentInfo->description);
    EmptyCountedString(&agentInfo->version);
    EmptyCountedString(&agentInfo->vendorName);
}

