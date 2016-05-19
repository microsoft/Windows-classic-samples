//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include <intsafe.h>
#include "SessionBasedCalculatorService.wsdl.h"
HANDLE closeServer = NULL;  
class SessionfulCalculator 
{
    int total;

public:
    static HRESULT CALLBACK Add(
        __in const WS_OPERATION_CONTEXT* context, 
        __in int a, 
        __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
        __in_opt WS_ERROR* error)
    {
        UNREFERENCED_PARAMETER(asyncContext);

        HRESULT hr = S_OK;
        SessionfulCalculator* calculator = NULL;
        hr = WsGetOperationContextProperty(
                context, 
                WS_OPERATION_CONTEXT_PROPERTY_CHANNEL_USER_STATE, 
                &calculator, 
                sizeof(SessionfulCalculator*), 
                error);
if (FAILED(hr))
{
    goto Exit;
}
        hr = calculator->Add(a);
    Exit:
        return hr;
    }

    static HRESULT CALLBACK Subtract(
        __in const WS_OPERATION_CONTEXT* context, 
        __in int a, 
        __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
        __in_opt WS_ERROR* error)
    {
        UNREFERENCED_PARAMETER(asyncContext);

        HRESULT hr = S_OK;
        SessionfulCalculator* calculator = NULL;
        hr = WsGetOperationContextProperty(
                context, 
                WS_OPERATION_CONTEXT_PROPERTY_CHANNEL_USER_STATE, 
                &calculator, 
                sizeof(SessionfulCalculator*), 
                error);
if (FAILED(hr))
{
    goto Exit;
}
        hr = calculator->Subtract(a);
    Exit:
        return hr;
    }

    static HRESULT CALLBACK Clear(
        __in const WS_OPERATION_CONTEXT* context, 
        __in const WS_ASYNC_CONTEXT* asyncContext, 
        __in_opt WS_ERROR* error)
    {
        UNREFERENCED_PARAMETER(asyncContext);

        HRESULT hr = S_OK;
        SessionfulCalculator* calculator = NULL;
        hr = WsGetOperationContextProperty(
                context, 
                WS_OPERATION_CONTEXT_PROPERTY_CHANNEL_USER_STATE, 
                &calculator, 
                sizeof(SessionfulCalculator*), 
                error);
if (FAILED(hr))
{
    goto Exit;
}
        hr = calculator->Clear();
    Exit:
        return hr;
    }

    static HRESULT CALLBACK Total(
        __in const WS_OPERATION_CONTEXT* context, 
        __out int* total, 
        __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
        __in_opt WS_ERROR* error)
    {
        UNREFERENCED_PARAMETER(asyncContext);

        HRESULT hr = S_OK;
        SessionfulCalculator* calculator = NULL;
        hr = WsGetOperationContextProperty(
                context, 
                WS_OPERATION_CONTEXT_PROPERTY_CHANNEL_USER_STATE, 
                &calculator, 
                sizeof(SessionfulCalculator*), 
                error);
if (FAILED(hr))
{
    goto Exit;
}
        hr = calculator->Total(total);
    Exit:
        return hr;
    }

private:
    HRESULT Add(
        __in int a)
    {
        wprintf(L"Adding %d to a total of %d\n", a, total);
        
        total += a;
        
        fflush(stdout);
        return S_OK;
    }

    HRESULT Subtract(
        __in int a)
    {
        wprintf(L"Subtracting %d from a total of %d\n", a, total);
        
        total -= a;
        
        fflush(stdout);
        return S_OK;
    }

    HRESULT Clear()
    {
        total = 0;
        wprintf(L"Cleared!\n");
        
        fflush(stdout);
        return S_OK;
    }

    HRESULT Total(
        __out int* result)
    {
        wprintf(L"The total is %d\n",total);
        *result = total;
        fflush(stdout);
        return S_OK;
    }
};      

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

HRESULT CALLBACK CreateSessionCalculator(
    __in const WS_OPERATION_CONTEXT* context, 
    __deref_out void** channelState, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    wprintf(L"Creating Calculator instance\n");

    SessionfulCalculator* calculator = new SessionfulCalculator();
    if (calculator)
    {
        *channelState = (void*)calculator;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

HRESULT CALLBACK FreeSessionCalculator(
    __in const WS_OPERATION_CONTEXT* context, 
    __in const WS_ASYNC_CONTEXT* asyncContext)
{
    UNREFERENCED_PARAMETER(asyncContext);

    SessionfulCalculator* calculator = NULL;
    WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_CHANNEL_USER_STATE, 
        &calculator, 
        sizeof(SessionfulCalculator*), 
        NULL);
    if (calculator != NULL)
    {    
        wprintf(L"Deleting Calculator instance\n");
        delete calculator;
    }
    
    SetEvent(closeServer);
    return S_OK;
}

static const 
CalculatorBindingFunctionTable 
calculatorFunctions = {
    SessionfulCalculator::Add, 
    SessionfulCalculator::Subtract, 
    SessionfulCalculator::Total,
    SessionfulCalculator::Clear 
};

// Method contract for the service
static const WS_SERVICE_CONTRACT calculatorServiceContract = 
{
    &SessionBasedCalculatorService_wsdl.contracts.CalculatorBinding, // comes from the generated header.
    NULL, // for not specifying the default contract
    &calculatorFunctions // specified by the user
};


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    serviceEndpoints[0] = &serviceEndpoint;
    WS_ERROR* error = NULL;
    WS_SERVICE_ENDPOINT_PROPERTY serviceProperties[2];
    WS_SERVICE_PROPERTY_ACCEPT_CALLBACK acceptCallbackProperty = {CreateSessionCalculator};
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {FreeSessionCalculator};
    
    serviceProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_ACCEPT_CHANNEL_CALLBACK;
    serviceProperties[0].value = &acceptCallbackProperty;
    serviceProperties[0].valueSize = sizeof(acceptCallbackProperty);
    serviceProperties[1].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceProperties[1].value = &closeCallbackProperty;
    serviceProperties[1].valueSize = sizeof(closeCallbackProperty);
    
    
    // Initialize service endpoint
    serviceEndpoint.address.url.chars = L"net.tcp://+/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_TCP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_DUPLEX_SESSION; // the channel type
    serviceEndpoint.contract = &calculatorServiceContract;  // the contract
    serviceEndpoint.properties = serviceProperties;
    serviceEndpoint.propertyCount = WsCountOf(serviceProperties);
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create Event object for closing the server
    closeServer = CreateEvent(
        NULL, 
        TRUE, 
        FALSE, 
        NULL);
    if (closeServer == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }   
    // Creating a service host
    hr = WsCreateServiceHost(
        serviceEndpoints, 
        1, 
        NULL, 
        0, 
        &host, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // WsOpenServiceHost to start the listeners in the service host 
    hr = WsOpenServiceHost(
        host, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    WaitForSingleObject(closeServer, INFINITE);
    // Close the service host
    hr = WsCloseServiceHost(host, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (host != NULL)
    {
        WsFreeServiceHost(host);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (closeServer != NULL)
    {
        CloseHandle(closeServer);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
