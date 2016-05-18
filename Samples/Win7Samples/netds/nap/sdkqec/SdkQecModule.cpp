// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <NapUtil.h>
#include <naperror.h>
#include "SdkQecModule.h"

using namespace SDK_SAMPLE_COMMON;

/// Registers the SDK SHA with the NAP Server.
HRESULT CSdkQecModule::RegisterSdkQec() throw()
{
    HRESULT hr = S_OK;
    /// Pointer to the INapServerManagement interface
    CComPtr<INapClientManagement> m_pQECMgmt = NULL;

    /// Registration Information
    NapComponentRegistrationInfo m_qecInfo;

    ZeroMemory (&m_qecInfo, sizeof(m_qecInfo));
    hr = m_pQECMgmt.CoCreateInstance(CLSID_NapClientManagement,
                                        NULL,
                                        CLSCTX_INPROC_SERVER);

    if (FAILED(hr))
    {
        wprintf(L"RegisterSdkQec: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = FillQecComponentRegistrationInfo(&m_qecInfo);
    if(FAILED(hr))
    {
        wprintf(L"RegisterSdkQec:: FillQecComponentRegistrationInfo Failed with %#x",hr);
        goto Cleanup;
    }

    hr = m_pQECMgmt->RegisterEnforcementClient(&m_qecInfo);
    if ( FAILED(hr) )
    {
        if (NAP_E_CONFLICTING_ID == hr)
            wprintf(L"\nRegisterSdkQec: SdkQec registration with NapAgent returned NAP_E_CONFLICTING_ID - already registered? ");
        else
            wprintf(L"\nRegisterSdkQec: Failed to register SdkQec with NapAgent -unknown (error = %x)\n", hr);

        goto Cleanup;
    }

    wprintf(L"\nRegisterSdkQec: SdkQec registration with NapAgent returned SUCCESS");


Cleanup:
    FreeComponentRegistration(&m_qecInfo);
    return hr;
}

/// Unregisters the SDK SHA with the NAP Server.
HRESULT CSdkQecModule::UnregisterSdkQec() throw()
{
    HRESULT hr = S_OK;

    CComPtr<INapClientManagement> m_pQECMgmt = NULL;

    // create an instance of the Management interface
    hr = m_pQECMgmt.CoCreateInstance(CLSID_NapClientManagement,
                                        NULL,
                                        CLSCTX_INPROC_SERVER);

    if (FAILED(hr))
    {
        wprintf(L"UnregisterSdkQec: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = m_pQECMgmt->UnregisterEnforcementClient(NapSdkQecId);
    if ( FAILED(hr) )
    {
        if (NAP_E_STILL_BOUND == hr)
            wprintf(L"\nUnregisterSdkQec: SdkQec unregistration with NapAgent returned NAP_E_STILL_BOUND - ; stop 'SDKQEC /execute' prior to attempting Unregister.");
        else
            wprintf(L"\nUnregisterSdkQec: SdkQec unregistration with NapAgent failed -unknown (error = %x)\n", hr);

        goto Cleanup;
    }

    wprintf(L"UnregisterSdkQec: UnregisterSdkQec: SdkQec unregistration with NapAgent returned SUCCESS ");

Cleanup:
    return hr;
}

/// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
HRESULT CSdkQecModule::FillQecComponentRegistrationInfo (NapComponentRegistrationInfo *agentInfo) throw()
{
    HRESULT hr = S_OK;
    agentInfo->id = NapSdkQecId;
    agentInfo->infoClsid = CLSID_INFO; 

    hr = FillCountedString( QEC_FRIENDLY_NAME, &(agentInfo->friendlyName) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for friendlyName returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( QEC_DESCRIPTION, &(agentInfo->description) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for description returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( QEC_VERSION, &(agentInfo->version) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for version returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString( QEC_VENDOR_NAME, &(agentInfo->vendorName) );
    if (FAILED(hr))
    {
        DebugPrintfW(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for vendorName returned error %#x!",hr);
        goto Cleanup;
    }

Cleanup:
    return hr;
}



// Helper Function for releaseing QEC ComponentRegistrationInfo members
void CSdkQecModule::FreeComponentRegistration(NapComponentRegistrationInfo *agentInfo) throw()
{
    EmptyCountedString(&agentInfo->friendlyName);
    EmptyCountedString(&agentInfo->description);
    EmptyCountedString(&agentInfo->version);
    EmptyCountedString(&agentInfo->vendorName);
}

