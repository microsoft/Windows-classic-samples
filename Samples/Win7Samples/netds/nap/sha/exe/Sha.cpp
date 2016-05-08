// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "Sha.h"
#include "Callback.h"
#include <Strsafe.h>
#include <naperror.h>
#include <objbase.h>



using namespace SDK_SAMPLE_COMMON;
using namespace SDK_SAMPLE_SHA;

extern BOOL g_setHealthySoh;

// Helper Function for displaying selection menu.
void showSdkShaInterfaceOptions()
{
    wprintf(L"\n x : To Quit");
    wprintf(L"\n 1 : NotifySohChange");
    wprintf(L"\n 2 : Flush Cache");
    wprintf(L"\n 3 : Set Unhealthy");
    wprintf(L"\n 4 : Set Healthy");
    wprintf(L"\nOption: \n");
}

void showSdkShaExecutionOptions() throw()
{
    wprintf(L"\n sdksha.exe  /register   : To Register the SHA with NAPAgent");
    wprintf(L"\n sdksha.exe  /unregister : To Unregister the SHA from NAPAgent");
    wprintf(L"\n sdksha.exe  /execute  : To execute the SHA and observe activity");
    wprintf(L"\n                   (for this to succeed, registration must be completed prior)");
}

ShaActionCode GetActionFromParams ( WCHAR * arg ) throw()
{
    HRESULT hr = S_OK;
    // default to noaction in case parsing fails
    ShaActionCode actionCode = NOACTION;
    size_t cCount = 0;
    size_t maxcCount = 0;

    wprintf(L"\nSHA GetActionFromParams: parsing command params");

    //size of largest valid argument
    maxcCount = sizeof(ShaActionCode_Unregister)/sizeof(WCHAR);

    hr = StringCchLengthW( arg, maxcCount, &cCount);
    if ( FAILED(hr) )
    {
        wprintf(L"\nSHA GetActionFromParams: unable to determine length of parameter");
        goto Cleanup;
    }

    //performing lowercase comparisons against valid parameters

    if ( 0 == _wcsicmp(arg, ShaActionCode_Register) )
    {
        actionCode = DOREGISTER;
        goto Cleanup;
    }

    if ( 0 == _wcsicmp(arg, ShaActionCode_Unregister) )
    {
        actionCode = DOUNREGISTER;
        goto Cleanup;
    }

    if ( 0 == _wcsicmp(arg, ShaActionCode_Execute) )
    {
        actionCode = DOEXECUTE;
        goto Cleanup;
    }

Cleanup:
    return actionCode;
}

//Register the SDKSHA with the NAPAgent
void DoSHARegistration ( CSdkShaModule * sdkShaModule ) throw()
{
    HRESULT hr = S_OK;
    
    // Register SHA with NapAgent
    hr = sdkShaModule->RegisterSdkSha();
    if (FAILED(hr))
    {
        wprintf(L"\nSHA DoSHAUnregistration: Failed to Register SdkSha with NapAgent (error = %x)\n", hr);
    }
    return;
}


//Unregister the SDKSHA from the NAPAgent
void DoSHAUnregistration( CSdkShaModule * sdkShaModule ) throw()
{
    HRESULT hr = S_OK;

    // UnRegister SHA with NapAgent
    hr = sdkShaModule->UnregisterSdkSha();
    if (FAILED(hr))
    {
        wprintf(L"\nSHA DoSHAUnregistration: Failed to Unregister SdkSha with NapAgent (error = %x)\n", hr);
    }
    return;
}


// Have the QEC Bind and run its observation loop, logging data to the console
void DoSHAExecution ( ) throw()
{
    HRESULT hr = S_OK;
    // Create Callback
    IShaCallbackPtr callback = NULL;

    // pointer to the binding  interface
    CComPtr<INapSystemHealthAgentBinding> binding = NULL;

    // Create binding.
    hr = binding.CoCreateInstance(CLSID_NapSystemHealthAgentBinding,
                                            NULL,
                                            CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to create an instance to QuarSystemHealthAgentBinding (error = %x)\n", hr);
        goto Cleanup;
    }

    callback = ShaCallback::CreateInstance(binding);
    if (!callback)
    {
        wprintf(L"\nSHA main: Failed to call ShaCallback::CreateInstance (error = %x)\n", hr);
        goto Cleanup;
    }


    hr = binding->Initialize(QuarSampleSystemHealthId,callback);
    if (FAILED(hr))
    {
         wprintf(L"\nSHA main: Failed to call QuarSystemHealthAgentBinding-> Initialize (error = %x)\n", hr);
         goto Cleanup;
    }


    // SDK Note:
    // The following is just an example to show how to send NotifySoHChange and How to stop the service
    // When implementing SHA as a service this routine should check for an SoH change on the client
    // and if the SoH changed it will send NotifySoHChange
    // When you stop the service it should call QuarSystemHealthAgentBinding->Uninitialize()

    WCHAR input = 0;
    showSdkShaInterfaceOptions();
    input = getwchar();
    while ((input != L'x') && (input != L'X'))
    {
        if ((input >= L'1') && (input <= L'4'))
        {
            switch (input)
            {
                case L'1':
                    wprintf(L"SDKSHA::calling notifySohChange");
                    hr = binding->NotifySoHChange();
                    break;
                case L'2':
                    wprintf(L"SDKSHA::calling FlushCache");
                    hr = binding->FlushCache();
                    break;
                case L'3':
                    wprintf(L"SDKSHA::setting SoH to UNHEALTHY");
                    g_setHealthySoh = false;
                    break;
                case L'4':
                    wprintf(L"SDKSHA::setting SoH to HEALTHY");
                    g_setHealthySoh = true;
                    break;
                default:
                    wprintf(L"SDKSHA::invalid option(): %#x\n", input);
            }
            if (FAILED (hr))
            {
                wprintf(L"Execute operation failed: code was (0x%08x) \n", hr);
                //reset code for next loop
                hr = S_OK;
            }
            // Sleep until it finishes processing the response
            Sleep (1000);
        }

		//set up for next loop
		input = 0;
		showSdkShaInterfaceOptions();
		input = getwchar();
    }

    // Stopping the SHA
    hr = binding->Uninitialize();
    if (FAILED(hr))
    {
         wprintf(L"\nSHA main: Failed to call QuarSystemHealthAgentBinding-> Uninitialize (error = %x)\n", hr);
         goto Cleanup;
    }

    wprintf(L"\nSHA stopped successfully \n");

Cleanup:
    return;

}

// Main function
DWORD __cdecl wmain(DWORD argc, WCHAR * pArgv[])  throw()
{
    HRESULT hr = S_OK;
    CSecurityDescriptor sd;
    ShaActionCode actionCode = NOACTION;
    CSdkShaModule _AtlModule;
    BOOL comInitialized = FALSE;

    // only ever expect 2 arguments, 1st is always exe name,
    // second should be one of the execution options
    if (2 != argc)
    {
        showSdkShaExecutionOptions();
        goto Cleanup;
    }

    // Start up COM and ATL.
    hr  = CoInitializeEx(NULL, COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to initialize COM (error = %x)\n", hr);
        goto Cleanup;
    }
    comInitialized = TRUE;

    // setting security on COM to allow communication to/from the
    // NAPAgent service, which runs as NetworkService
    sd.InitializeFromThreadToken();
    sd.Allow("NETWORK_SERVICE", COM_RIGHTS_EXECUTE);
    hr = CoInitializeSecurity( sd,
                               -1,
                               NULL,
                               NULL,
                               RPC_C_AUTHN_LEVEL_PKT, 
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               NULL,
                               EOAC_NONE,
                               NULL );
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to initialize COM security (error = %x)\n", hr);
        goto Cleanup;
    }

    // got an argument - parse it and decide what to do
    actionCode = GetActionFromParams( pArgv[1] );
    switch (actionCode)
    {
        case DOREGISTER:
                wprintf(L"\nSHA main: Action code is DOREGISTER, proceeding\n");
                DoSHARegistration(&_AtlModule);
                break;

        case DOUNREGISTER:
                wprintf(L"\nSHA main: Action code is DOUNREGISTER, proceeding\n");
                DoSHAUnregistration(&_AtlModule);
                break;

        case DOEXECUTE:
                wprintf(L"\nSHA main: Action code is DOEXECUTE, proceeding\n");
                DoSHAExecution();
                break;

        default:
                wprintf(L"\nSHA main: No action code, or unrecognized, proceeding\n");
                showSdkShaExecutionOptions();
                break;
    }

    Cleanup:
        if (comInitialized)
        {
            CoUninitialize();
        }
        return 0;
}


