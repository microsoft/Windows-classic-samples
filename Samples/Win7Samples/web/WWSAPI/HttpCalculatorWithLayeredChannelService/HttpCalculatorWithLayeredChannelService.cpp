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
#include "CalculatorService.wsdl.h"
//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// A structure containing parameters passed to the custom channel 
// using WS_CHANNEL_PROPERTY_CUSTOM_CHANNEL_PARAMETERS.
struct LayeredChannelParameters
{
    // The type of the underlying channel
    WS_CHANNEL_BINDING channelBinding;

    // Channel properties to pass to the underlying channel
    WS_CHANNEL_PROPERTY* channelProperties;
    ULONG channelPropertyCount;

    // Security settings for the underlying channel
    WS_SECURITY_DESCRIPTION* securityDescription;
};

// The structure containing instance state for the custom channel
struct CustomChannel
{
    // Underlying channel handle
    WS_CHANNEL* channel;
    BOOL disabledTimeouts;
};

// The set of callbacks that make up the custom channel implementation.
extern WS_CUSTOM_CHANNEL_CALLBACKS layeredChannelCallbacks;
//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// A structure containing parameters passed to the custom listener
// using WS_LISTENER_PROPERTY_CUSTOM_LISTENER_PARAMETERS.
struct LayeredListenerParameters
{
    // The type of the underlying channel
    WS_CHANNEL_BINDING channelBinding;

    // Listener properties to pass to the underlying listener
    WS_LISTENER_PROPERTY* listenerProperties;
    ULONG listenerPropertyCount;

    // Security settings for the underlying listener
    WS_SECURITY_DESCRIPTION* securityDescription;
};

// The structure containing instance state for the custom listener
struct CustomListener
{
    // Underlying listener handle
    WS_LISTENER* listener;
};

// The set of callbacks that make up the custom listener implementation.
extern WS_CUSTOM_LISTENER_CALLBACKS layeredListenerCallbacks;


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

HANDLE closeServer = NULL;  


HRESULT CALLBACK Add(
    __in const WS_OPERATION_CONTEXT* context, 
    __in int a, 
    __in int b, 
    __out int* result, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    *result = a + b;
    printf ("%d + %d = %d\n", a, b, *result);
    fflush(stdout);
    return S_OK;
}

HRESULT CALLBACK Subtract(
    __in const WS_OPERATION_CONTEXT* context, 
    __in int a, 
    __in int b, 
    __out int* result, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    *result = a - b;
    printf ("%d - %d = %d\n", a, b, *result);
    fflush(stdout);
    return S_OK;
}

HRESULT CALLBACK CloseChannelCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);

    SetEvent(closeServer);
    return S_OK;
}

static const DefaultBinding_ICalculatorFunctionTable calculatorFunctions = {Add, Subtract};

// Method contract for the service
static const WS_SERVICE_CONTRACT calculatorContract = 
{
    &CalculatorService_wsdl.contracts.DefaultBinding_ICalculator, // comes from the generated header.
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
    // remain valid until the service host object has been freed.  In this 
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
    
    // Set up the channel properties value
    WS_CHANNEL_PROPERTIES channelProperties;
    channelProperties.properties = customChannelPropertyArray;
    channelProperties.propertyCount = WsCountOf(customChannelPropertyArray);
    
    // Set up listener properties for the custom listener
    WS_LISTENER_PROPERTY listenerPropertyArray[2];
    
    // Set up listener property that specifies the callbacks that implement the custom listener
    listenerPropertyArray[0].id = WS_LISTENER_PROPERTY_CUSTOM_LISTENER_CALLBACKS;
    listenerPropertyArray[0].value = &layeredListenerCallbacks;
    listenerPropertyArray[0].valueSize = sizeof(layeredListenerCallbacks);
    
    // Initialize parameters to pass to the layered listener.
    // Note that the parameters structure and it's contents must 
    // remain valid until the service host object has been freed.  In this 
    // example, the parameters are declared on the stack for
    // simplicity, but in other scenarios they may need to be
    // allocated from the heap.
    LayeredListenerParameters layeredListenerParameters;
    layeredListenerParameters.channelBinding = WS_HTTP_CHANNEL_BINDING;
    layeredListenerParameters.listenerProperties = NULL;
    layeredListenerParameters.listenerPropertyCount = 0;
    layeredListenerParameters.securityDescription = NULL;
    
    // Specify the listener parameters as a listener property
    listenerPropertyArray[1].id = WS_LISTENER_PROPERTY_CUSTOM_LISTENER_PARAMETERS;
    listenerPropertyArray[1].value = &layeredListenerParameters;
    listenerPropertyArray[1].valueSize = sizeof(layeredListenerParameters);
    
    // Set up the listener properties value
    WS_LISTENER_PROPERTIES listenerProperties;
    listenerProperties.properties = listenerPropertyArray;
    listenerProperties.propertyCount = WsCountOf(listenerPropertyArray);
    
    // Set up service endpoint properties
    WS_SERVICE_ENDPOINT_PROPERTY serviceEndpointProperties[2];
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {CloseChannelCallback};
    serviceEndpointProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceEndpointProperties[0].value = &closeCallbackProperty;
    serviceEndpointProperties[0].valueSize = sizeof(closeCallbackProperty);
    serviceEndpointProperties[1].id = WS_SERVICE_ENDPOINT_PROPERTY_LISTENER_PROPERTIES;
    serviceEndpointProperties[1].value = &listenerProperties;
    serviceEndpointProperties[1].valueSize = sizeof(listenerProperties);
    
    
    // Initialize service endpoint
    serviceEndpoint.address.url.chars = L"http://+:80/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_CUSTOM_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_REPLY; // the channel type
    serviceEndpoint.securityDescription = NULL; // security description
    serviceEndpoint.contract = &calculatorContract;  // the contract
    serviceEndpoint.properties = serviceEndpointProperties;
    serviceEndpoint.propertyCount = WsCountOf(serviceEndpointProperties);
    serviceEndpoint.channelProperties.properties = customChannelPropertyArray; // Channel properties
    serviceEndpoint.channelProperties.propertyCount = WsCountOf(customChannelPropertyArray); // Channel property Count
    
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

