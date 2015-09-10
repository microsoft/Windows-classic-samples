// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Sha.h"
#include "Callback.h"
#include <Strsafe.h>
#include <objbase.h>
#include "SdkCommon.h"


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

void showSdkShaExecutionOptions()
{
    wprintf(L"\n sdksha.exe  /register   : To Register the SHA with NAPAgent");
    wprintf(L"\n sdksha.exe  /unregister : To Unregister the SHA from NAPAgent");
    wprintf(L"\n sdksha.exe  /execute  : To execute the SHA and observe activity");
    wprintf(L"\n                   (for this to succeed, registration must be completed prior)");
}

ShaActionCode GetActionFromParams (_In_ WCHAR* arg)
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
void DoSHARegistration (_In_ CSdkShaModule * sdkShaModule )
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
void DoSHAUnregistration(_In_  CSdkShaModule * sdkShaModule)
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
void DoSHAExecution()
{
    HRESULT hr = S_OK;

    // pointer to the callback interface
    INapSystemHealthAgentCallback* pCallback = NULL;

    // pointer to the binding interface
    INapSystemHealthAgentBinding* pBinding = NULL;

    // Create binding.
    hr = CoCreateInstance(CLSID_NapSystemHealthAgentBinding,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_INapSystemHealthAgentBinding,
                          reinterpret_cast<void**>(&pBinding));
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to create an instance to QuarSystemHealthAgentBinding (error = %x)\n", hr);
        goto Cleanup;
    }

    pCallback = ShaCallback::CreateInstance(pBinding);
    if (!pCallback)
    {
        wprintf(L"\nSHA main: Failed to call ShaCallback::CreateInstance (error = %x)\n", hr);
        goto Cleanup;
    }

    hr = pBinding->Initialize(QuarSampleSystemHealthId, pCallback);
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
                    hr = pBinding->NotifySoHChange();
                    break;
                case L'2':
                    wprintf(L"SDKSHA::calling FlushCache");
                    hr = pBinding->FlushCache();
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
    hr = pBinding->Uninitialize();
    if (FAILED(hr))
    {
         wprintf(L"\nSHA main: Failed to call QuarSystemHealthAgentBinding-> Uninitialize (error = %x)\n", hr);
         goto Cleanup;
    }

    wprintf(L"\nSHA stopped successfully \n");

Cleanup:
    ReleaseObject(pBinding);
    ReleaseObject(pCallback);
    return;

}


// Main function
DWORD __cdecl wmain(
    _In_ DWORD argc, 
    _In_reads_(argc) WCHAR * pArgv[]) 
{
    HRESULT hr = S_OK;
    ShaActionCode actionCode = NOACTION;
    CSdkShaModule sdkModule;
    BOOL comInitialized = FALSE;

    // only ever expect 2 arguments, 1st is always exe name,
    // second should be one of the execution options
    if (2 != argc)
    {
        showSdkShaExecutionOptions();
        goto Cleanup;
    }

    // Start up COM.
    hr  = CoInitializeEx(NULL, COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to initialize COM (error = %x)\n", hr);
        goto Cleanup;
    }
    comInitialized = TRUE;

    // setting security on COM
    hr = InitializeSecurity();
    if (FAILED(hr))
    {
        wprintf(L"\nSHA main: Failed to initialize COM security (error = %x)\n", hr);
    }

    // got an argument - parse it and decide what to do
    actionCode = GetActionFromParams( pArgv[1] );
    switch (actionCode)
    {
        case DOREGISTER:
                wprintf(L"\nSHA main: Action code is DOREGISTER, proceeding\n");
                DoSHARegistration(&sdkModule);
                break;

        case DOUNREGISTER:
                wprintf(L"\nSHA main: Action code is DOUNREGISTER, proceeding\n");
                DoSHAUnregistration(&sdkModule);
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


