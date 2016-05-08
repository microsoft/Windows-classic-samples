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
#include "LayeredChannel.h"
#include "CalculatorService.wsdl.h"

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

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_SERVICE_PROXY* proxy = NULL;
    
    int result = 0;
    WS_ENDPOINT_ADDRESS address = {};
    static const WS_STRING url= WS_STRING_VALUE(L"http://localhost/example");
    address.url = url;
    
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
    
    // In order to use the custom channel through Service Model,
    // we need to configure it to disable all timeouts.
    WS_CHANNEL_PROPERTY channelPropertyArray[1];
    BOOL enableTimeouts = FALSE;
    channelPropertyArray[0].id = WS_CHANNEL_PROPERTY_ENABLE_TIMEOUTS;
    channelPropertyArray[0].value = &enableTimeouts;
    channelPropertyArray[0].valueSize = sizeof(enableTimeouts);
    
    // Set up channel properties for the custom channel
    WS_CHANNEL_PROPERTY customChannelPropertyArray[2];
    
    // Set up channel property that specifies the callbacks that implement the custom channel
    customChannelPropertyArray[0].id = WS_CHANNEL_PROPERTY_CUSTOM_CHANNEL_CALLBACKS;
    customChannelPropertyArray[0].value = &layeredChannelCallbacks;
    customChannelPropertyArray[0].valueSize = sizeof(layeredChannelCallbacks);
    
    // Initialize parameters to pass to the layered channel.
    // Note that the parameters structure and it's contents must 
    // remain valid until the proxy object has been freed.  In this 
    // example, the parameters are declared on the stack for
    // simplicity, but in other scenarios they may need to be
    // allocated from the heap.
    LayeredChannelParameters layeredChannelParameters;
    layeredChannelParameters.channelBinding = WS_HTTP_CHANNEL_BINDING;
    layeredChannelParameters.channelProperties = channelPropertyArray;
    layeredChannelParameters.channelPropertyCount = WsCountOf(channelPropertyArray);
    layeredChannelParameters.securityDescription = NULL;
    
    // Specify the channel parameters as a channel property
    customChannelPropertyArray[1].id = WS_CHANNEL_PROPERTY_CUSTOM_CHANNEL_PARAMETERS;
    customChannelPropertyArray[1].value = &layeredChannelParameters;
    customChannelPropertyArray[1].valueSize = sizeof(layeredChannelParameters);
    
    
    // Create the proxy
    hr = WsCreateServiceProxy(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_CUSTOM_CHANNEL_BINDING, 
        NULL, 
        NULL, 
        0, 
        customChannelPropertyArray,
        WsCountOf(customChannelPropertyArray),
        &proxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    
    hr = WsOpenServiceProxy(
        proxy, 
        &address, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = DefaultBinding_ICalculator_Add(
        proxy, 
        1, 
        2, 
        &result, 
        heap, 
        NULL, 
        0, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"%d + %d = %d\n", 1, 2, result);
                   
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (proxy != NULL)
    {
        WsCloseServiceProxy(
            proxy, 
            NULL, 
            NULL);
    
        WsFreeServiceProxy(
            proxy);
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
