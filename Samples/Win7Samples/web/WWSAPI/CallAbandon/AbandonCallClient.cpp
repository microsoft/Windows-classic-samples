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
#include "BlockUnBlockService.wsdl.h"

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

void CALLBACK BlockMethodComplete(
    __in HRESULT hr,
    __in WS_CALLBACK_MODEL callbackModel,
    __in void* callbackState)
{
    UNREFERENCED_PARAMETER(hr);
    UNREFERENCED_PARAMETER(callbackModel);
    UNREFERENCED_PARAMETER(callbackState);

    wprintf(L"Method Completed\n");
    fflush(stdout);
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_SERVICE_PROXY* serviceProxy = NULL;
    WS_ENDPOINT_ADDRESS address = {};
    const WS_ASYNC_CONTEXT asyncContext = {BlockMethodComplete, NULL};
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"net.tcp://localhost/example");
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create a heap to store deserialized data
    hr = WsCreateHeap(
        /*maxSize*/ 2048, 
        /*trimSize*/ 512, 
        NULL, 
        0, 
        &heap, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsCreateServiceProxy(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, NULL, NULL, 0, NULL, 0, &serviceProxy, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Initialize address of service
    address.url = serviceUrl;                    
    hr = WsOpenServiceProxy(serviceProxy, &address, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"Calling blocking method async\n");
    hr = BlockServiceBinding_Block(serviceProxy, heap, NULL, 0, &asyncContext, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"Now abandoning call\n");
    hr = WsAbandonCall(serviceProxy, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    wprintf(L"Now unblocking service\n");
    hr = BlockServiceBinding_UnBlock(serviceProxy, heap, NULL, 0, &asyncContext, error);
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
    if (serviceProxy != NULL)
    {
        WsCloseServiceProxy(serviceProxy, NULL, NULL);
        WsFreeServiceProxy(serviceProxy);
    }
    
    
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
