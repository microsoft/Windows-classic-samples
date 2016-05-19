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
HANDLE closeServer = NULL;  

#include "BlockUnBlockService.wsdl.h"

static void CALLBACK AbortNotification(
    __in WS_SERVICE_CANCEL_REASON reason, 
    __in_opt void* callbackState)
{
    wprintf(L"Abort Reason is: %d\n", reason);
    fflush(stdout);
    HANDLE unblockMethod = (HANDLE) callbackState;
    SetEvent(unblockMethod);
}

static void CALLBACK FreeState(
    __in_opt void* callbackState)
{
    HANDLE unblockMethod = (HANDLE) callbackState;
    CloseHandle(unblockMethod);
}

static HRESULT CALLBACK Block(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr = S_OK;
    HANDLE unblockMethod = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!unblockMethod)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    hr = WsRegisterOperationForCancel(
        context, 
        AbortNotification, 
        FreeState, 
        unblockMethod, 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    WaitForSingleObject(unblockMethod, INFINITE);
Exit:
    SetEvent(closeServer);
    if (FAILED(hr))
    {
        CloseHandle(unblockMethod);
    }
    return hr;
}

static HRESULT CALLBACK UnBlock(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    SetEvent(closeServer);
    return S_OK;
}

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


static const BlockServiceBindingFunctionTable serviceFunctions = {Block, UnBlock};

// Method contract for the service
static const WS_SERVICE_CONTRACT blockServiceContract = 
{
    &BlockUnBlockService_wsdl.contracts.BlockServiceBinding, // comes from the generated header.
    NULL, // for not specifying the default contract
    &serviceFunctions // specified by the user
};


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    WS_ERROR* error = NULL;
    WS_SERVICE_ENDPOINT_PROPERTY serviceProperties[1];
    const ULONG maxConcurrency = 100;
    serviceEndpoints[0] = &serviceEndpoint;
    serviceProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_MAX_CONCURRENCY;
    serviceProperties[0].value = (void*)&maxConcurrency;
    serviceProperties[0].valueSize = sizeof(maxConcurrency);
    
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
    
    // Initialize service endpoint
    serviceEndpoint.address.url.chars = L"net.tcp://+/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_TCP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_DUPLEX_SESSION; // the channel type
    serviceEndpoint.contract = &blockServiceContract;  // the contract
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
    // Aborts the service host so that the blocked method can complete.
    WsAbortServiceHost(host, NULL);
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
