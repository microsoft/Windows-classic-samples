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
#include "OrderSessionHeader.xsd.h"
#include "PurchaseOrder.wsdl.h"

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


HRESULT CALLBACK AddSessionHeader(
    __in WS_MESSAGE* message,
    __in WS_HEAP* heap,
    __in void* state,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(heap);

    _OrderSession* orderSession = (_OrderSession*) state;
    // Add reply sessionID
    return WsAddCustomHeader(
        message, 
        &OrderSessionHeader_xsd.globalElements.OrderSession, 
        WS_WRITE_REQUIRED_VALUE,
        orderSession, 
        sizeof(*orderSession), 
        0, 
        error);
}

HRESULT CALLBACK RetrieveSessionHeader(
    __in WS_MESSAGE* message,
    __in WS_HEAP* heap,
    __in void* state,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(heap);

    _OrderSession* inputOrderSession = (_OrderSession*) state;
    _OrderSession* outputOrderSession = NULL;

    HRESULT hr = WsGetCustomHeader(
        message, 
        &OrderSessionHeader_xsd.globalElements.OrderSession, 
        WS_SINGLETON_HEADER,
        0,
        WS_READ_REQUIRED_POINTER, 
        NULL, 
        &outputOrderSession, 
        sizeof(outputOrderSession), 
        NULL, 
        error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    wprintf(L"%s == %s\n", 
        outputOrderSession->sessionId,
        inputOrderSession->sessionId);
    fflush(stdout);
    
    return S_OK;
}



// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_SERVICE_PROXY* serviceProxy = NULL;
    WS_HEAP* heap = NULL;
    WS_ENDPOINT_ADDRESS address = {0};
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"http://localhost/example");
    address.url = serviceUrl;
    WS_CALL_PROPERTY callProperties[2];
    _OrderSession orderSession;
    orderSession.sessionId = L"ExampleSession";
    WS_PROXY_MESSAGE_CALLBACK_CONTEXT inputMessageContext = {0};
    WS_PROXY_MESSAGE_CALLBACK_CONTEXT outputMessageContext = {0};
    
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
        NULL, 
        0, 
        &serviceProxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Open channel to address
    hr = WsOpenServiceProxy(
        serviceProxy, 
        &address, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    inputMessageContext.callback = AddSessionHeader;
    inputMessageContext.state = &orderSession;
    outputMessageContext.callback = RetrieveSessionHeader;
    outputMessageContext.state = &orderSession;
    
    callProperties[0].id = WS_CALL_PROPERTY_SEND_MESSAGE_CONTEXT;
    callProperties[0].value = &inputMessageContext;
    callProperties[0].valueSize = sizeof(inputMessageContext);
    
    callProperties[1].id = WS_CALL_PROPERTY_RECEIVE_MESSAGE_CONTEXT;
    callProperties[1].value = &outputMessageContext;
    callProperties[1].valueSize = sizeof(outputMessageContext);
    
    for (int i = 0; i < 100; i++)
    {
        static const WCHAR* productName = L"Pencil";
        WCHAR* expectedShipDate = {0};
        unsigned int orderID;
    
        hr = PurchaseOrderBinding_Order(
            serviceProxy, 
            100, 
            (WCHAR*)productName, 
            &orderID, 
            &expectedShipDate, 
            heap, 
            callProperties, 
            WsCountOf(callProperties), 
            NULL, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Print out confirmation contents
        wprintf(L"Expected ship date for order %lu is %s\n",
            orderID,
            expectedShipDate);
    
        hr = WsResetHeap(heap, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
    
        
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
