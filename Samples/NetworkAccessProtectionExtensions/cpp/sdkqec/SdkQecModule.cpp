// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <NapUtil.h>
#include "SdkQecModule.h"

using namespace SDK_SAMPLE_COMMON;

/// Registers the SDK SHA with the NAP Server.
HRESULT CSdkQecModule::RegisterSdkQec()
{
    HRESULT hr = S_OK;
    /// Pointer to the INapServerManagement interface
    INapClientManagement* pQECMgmt = NULL;

    /// Registration Information
    NapComponentRegistrationInfo qecInfo;
    ZeroMemory (&qecInfo, sizeof(qecInfo));

    hr = CoCreateInstance(CLSID_NapClientManagement,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_INapClientManagement,
                          reinterpret_cast<void**>(&pQECMgmt));
    if (FAILED(hr))
    {
        wprintf(L"RegisterSdkQec: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = FillQecComponentRegistrationInfo(&qecInfo);
    if(FAILED(hr))
    {
        wprintf(L"RegisterSdkQec:: FillQecComponentRegistrationInfo Failed with %#x",hr);
		ZeroMemory(&qecInfo, sizeof(qecInfo));
		goto Cleanup;
	}

    hr = pQECMgmt->RegisterEnforcementClient(&qecInfo);
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
    FreeComponentRegistration(&qecInfo);
    ReleaseObject(pQECMgmt);
    return hr;
}

/// Unregisters the SDK SHA with the NAP Server.
HRESULT CSdkQecModule::UnregisterSdkQec()
{
    HRESULT hr = S_OK;

    INapClientManagement* pQECMgmt = NULL;

    // create an instance of the Management interface
    hr = CoCreateInstance(CLSID_NapClientManagement,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_INapClientManagement,
                           reinterpret_cast<void**>(&pQECMgmt));
    if (FAILED(hr))
    {
        wprintf(L"UnregisterSdkQec: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = pQECMgmt->UnregisterEnforcementClient(NapSdkQecId);
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
    ReleaseObject(pQECMgmt);
    return hr;
}

/// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
HRESULT CSdkQecModule::FillQecComponentRegistrationInfo (
    _Out_ NapComponentRegistrationInfo *agentInfo)
{
    HRESULT hr = S_OK;
    agentInfo->id = NapSdkQecId;
    agentInfo->infoClsid = CLSID_INFO; 

    hr = FillCountedString( QEC_FRIENDLY_NAME, &(agentInfo->friendlyName) );
    if (FAILED(hr))
    {
        wprintf(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for friendlyName returned error %#x!",hr);
		goto Cleanup;
    }

    hr = FillCountedString( QEC_DESCRIPTION, &(agentInfo->description) );
    if (FAILED(hr))
    {
        wprintf(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for description returned error %#x!",hr);
		goto Cleanup;
    }

    hr = FillCountedString( QEC_VERSION, &(agentInfo->version) );
    if (FAILED(hr))
    {
        wprintf(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for version returned error %#x!",hr);
		goto Cleanup;
    }

    hr = FillCountedString( QEC_VENDOR_NAME, &(agentInfo->vendorName) );
    if (FAILED(hr))
    {
        wprintf(L"SdkQec::FillQecComponentRegistrationInfo(): AllocCountedString for vendorName returned error %#x!",hr);
		goto Cleanup;
    }

	return hr;

Cleanup:
	FreeComponentRegistration(agentInfo);
	return hr;
}



// Helper Function for releasing QEC ComponentRegistrationInfo members
void CSdkQecModule::FreeComponentRegistration(
    _Inout_ NapComponentRegistrationInfo *agentInfo)
{

    EmptyCountedString(&agentInfo->friendlyName);
    EmptyCountedString(&agentInfo->description);
    EmptyCountedString(&agentInfo->version);
    EmptyCountedString(&agentInfo->vendorName);

}

