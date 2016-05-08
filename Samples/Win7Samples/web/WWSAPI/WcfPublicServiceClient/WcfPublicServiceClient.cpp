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
#include "schemas.microsoft.com.2003.10.Serialization.xsd.h"
#include "tempuri.org.xsd.h"
#include "tempuri.org.wsdl.h"

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
    WS_SERVICE_PROXY* proxy = NULL;
    WS_HEAP* heap = NULL;
    WS_CHANNEL_PROPERTY channelProperties[2];
    WS_ENDPOINT_ADDRESS address = {};
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"http://131.107.72.15/Example_HelloWorld_Service_Indigo/HelloWorld.svc");
    WCHAR* greeting = NULL;
    
    WS_ADDRESSING_VERSION addressingVersion = WS_ADDRESSING_VERSION_TRANSPORT;
    channelProperties[0].id = WS_CHANNEL_PROPERTY_ADDRESSING_VERSION;
    channelProperties[0].value = &addressingVersion;
    channelProperties[0].valueSize = sizeof(addressingVersion);
    
    WS_ENVELOPE_VERSION envelopeVersion = WS_ENVELOPE_VERSION_SOAP_1_1;
    channelProperties[1].id = WS_CHANNEL_PROPERTY_ENVELOPE_VERSION;
    channelProperties[1].value = &envelopeVersion;
    channelProperties[1].valueSize = sizeof(envelopeVersion);
    
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
    
    hr = WsCreateServiceProxy(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, 
        NULL, 
        NULL, 
        0, 
        channelProperties,
        WsCountOf(channelProperties),
        &proxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    address.url = serviceUrl;
    // Open channel to address
    hr = WsOpenServiceProxy(
        proxy, 
        &address, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = BasicHttpBinding_IHelloWorldService_PersonalizedGreeting(
        proxy,
        L"Native Web Services",
        &greeting,
        heap,
        NULL,
        0,
        NULL,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"%s\n", 
        greeting);
    
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
