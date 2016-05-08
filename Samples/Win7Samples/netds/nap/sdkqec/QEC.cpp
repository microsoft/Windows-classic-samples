// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "QEC.h"
#include <Strsafe.h>
#include <objbase.h>


using namespace SDK_SAMPLE_COMMON;
using namespace SDK_SAMPLE_QEC;


// Helper Function for displaying selection menu.
void showSdkQecInterfaceOptions() throw()
{
    wprintf(L"\n x : To Quit");
    wprintf(L"\nOption: \n");
}

void showSdkQecExecutionOptions() throw()
{
    wprintf(L"\n sdkqec.exe  /register   : To Register the QEC with NAPAgent");
    wprintf(L"\n sdkqec.exe  /unregister : To Unregister the QEC from NAPAgent");
    wprintf(L"\n sdkqec.exe  /execute  : To execute the QEC and observe activity");
    wprintf(L"\n                   (for this to succeed, registration must be completed prior)");
}


QecActionCode GetActionFromParams ( __in WCHAR * arg ) throw()
{
    HRESULT hr = S_OK;
    // default to noaction in case parsing fails
    QecActionCode actionCode = NOACTION;
    size_t cCount = 0;
    size_t maxcCount = 0;

    wprintf(L"\nQEC GetActionFromParams: parsing command params");

    //size of largest valid argument
    maxcCount = sizeof(QecActionCode_Unregister)/sizeof(WCHAR);

    hr = StringCchLengthW( arg, maxcCount, &cCount);
    if ( FAILED(hr) )
    {
        wprintf(L"\nQEC GetActionFromParams: unable to determine length of parameter");
        goto Cleanup;
    }

    //performing lowercase comparisons against valid parameters

    if ( 0 == _wcsicmp(arg, QecActionCode_Register) )
    {
        actionCode = DOREGISTER;
        goto Cleanup;
    }

    if ( 0 == _wcsicmp(arg, QecActionCode_Unregister) )
    {
        actionCode = DOUNREGISTER;
        goto Cleanup;
    }

    if ( 0 == _wcsicmp(arg, QecActionCode_Execute) )
    {
        actionCode = DOEXECUTE;
        goto Cleanup;
    }

Cleanup:
    return actionCode;
}

//Register the SDKQEC with the NAPAgent
void DoQECRegistration ( CSdkQecModule * sdkQecModule ) throw()
{
    HRESULT hr = S_OK;
    
    // Register QEC with NapAgent
    hr = sdkQecModule->RegisterSdkQec();
    if (FAILED(hr))
    {
        wprintf(L"\nQEC DoQECUnRegistration: Failed to Register SdkQec with NapAgent (error = %x)\n", hr);
    }
    return;
}


//Unregister the SDKQEC from the NAPAgent
void DoQECUnRegistration( CSdkQecModule * sdkQecModule ) throw()
{
    HRESULT hr = S_OK;

    // UnRegister QEC with NapAgent
    hr = sdkQecModule->UnregisterSdkQec();
    if (FAILED(hr))
    {
        wprintf(L"\nQEC DoQECUnRegistration: Failed to Unregister SdkQec with NapAgent (error = %x)\n", hr);
    }
    return;
}

// Have the QEC Bind and run its observation loop, logging data to the console
void DoQECExecution ( ) throw()
{
    HRESULT hr = S_OK;
    // Create Callback
    IQecCallbackPtr callback = NULL;

    // pointer to the binding  interface
    CComPtr<INapEnforcementClientBinding> binding = NULL;

    // Create binding.
    hr = binding.CoCreateInstance(CLSID_NapEnforcementClientBinding,
                                            NULL,
                                            CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        wprintf(L"\nQEC DoQECExecution: Failed to create an instance to NapEnforcementClientBinding (error = %x)\n", hr);
        goto Cleanup;
    }

    // Create callback using the binding
    callback = QecCallback::CreateInstance(binding);
    if (!callback)
    {
        wprintf(L"\nQEC DoQECExecution: Failed to call QecCallback::CreateInstance (error = %x)\n", hr);
        goto Cleanup;
    }

    // Initialize the binding with the callback
    hr = binding->Initialize(NapSdkQecId,callback);
    if (FAILED(hr))
    {
         wprintf(L"\nQEC DoQECExecution: Failed to call NapEnforcementClientBinding::Initialize (error = %x)\n", hr);
         wprintf(L"\n\tIs the SDKQEC enabled ?  (enable via netsh or the NAP Client Configuration console) \n");
         goto Cleanup;
    }

    // SDK Note:
    // The following is just an example to show how QEC will responding to NapAgent's
    // Calls like NotifySoHChange etc.  Callback functions will be called and will generate
    // output while the loop below is running.

    //throw-away var used for the input loop
    WCHAR input = 0;
    showSdkQecInterfaceOptions();
    input = getwchar();
    while ((input != L'x') && (input != L'X'))
    {
        showSdkQecInterfaceOptions();
        input = getwchar();
    }

    // Stopping the QEC
    hr = binding->Uninitialize();
    if (FAILED(hr))
    {
         wprintf(L"\nQEC DoQECExecution: Failed to call QuarSystemHealthAgentBinding-> Uninitialize (error = %x)\n", hr);
         goto Cleanup;
    }

    wprintf(L"\nQEC DoQECExecution: QEC stopped successfully \n");

Cleanup:
    return;
}

// Main function
DWORD __cdecl wmain(DWORD argc, WCHAR * pArgv[])  throw()
{
    HRESULT hr = S_OK;
    CSecurityDescriptor sd;
    QecActionCode actionCode = NOACTION;
    CSdkQecModule _AtlModule;
    BOOL comInitialized = FALSE;

    // only ever expect 2 arguments, 1st is always exe name,
    // second should be one of the execution options
    if (2 != argc)
    {
        showSdkQecExecutionOptions();
        goto Cleanup;
    }

    // Start up COM and ATL.
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        wprintf(L"\nQEC main: Failed to initialize COM (error = %x)\n", hr);
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
        wprintf(L"\nQEC main: Failed to initialize COM security (error = %x)\n", hr);
        goto Cleanup;
    }

    // got an argument - parse it and decide what to do
    actionCode = GetActionFromParams( pArgv[1] );
    switch (actionCode)
    {
        case DOREGISTER:
                wprintf(L"\nQEC main: Action code is DOREGISTER, proceeding\n");
                DoQECRegistration(&_AtlModule);
                break;

        case DOUNREGISTER:
                wprintf(L"\nQEC main: Action code is DOUNREGISTER, proceeding\n");
                DoQECUnRegistration(&_AtlModule);
                break;

        case DOEXECUTE:
                wprintf(L"\nQEC main: Action code is DOEXECUTE, proceeding\n");
                DoQECExecution();
                break;

        default:
                wprintf(L"\nQEC main: No action code, or unrecognized, proceeding\n");
                showSdkQecExecutionOptions();
                break;
    }

    Cleanup:
        if (comInitialized)
        {
            CoUninitialize();
        }
        return 0;
}


