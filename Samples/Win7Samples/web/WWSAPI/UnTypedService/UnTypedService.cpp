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

HANDLE closeServer = NULL;  

volatile long numberOfItems = 0;

HRESULT CALLBACK ProcessMessage(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr = S_OK;
    WS_CHANNEL* channel = NULL;    
    WS_HEAP* heap = NULL;
    WS_MESSAGE* replyMessage = NULL;

    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_HEAP, 
        &heap, 
        sizeof(heap), 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    WS_MESSAGE* requestMessage = NULL;
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_INPUT_MESSAGE, 
        &requestMessage, 
        sizeof(requestMessage), 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = WsGetOperationContextProperty(
    context, 
    WS_OPERATION_CONTEXT_PROPERTY_CHANNEL, 
    &channel, 
    sizeof(channel), 
    error);
if (FAILED(hr))
{
    goto Exit;
}
    
hr = WsCreateMessageForChannel(
    channel,
    NULL, 
    0, 
    &replyMessage, 
    error);
if (FAILED(hr))
{
    goto Exit;
}
    
    // Get action value
    WS_XML_STRING receivedAction;
    hr = WsGetHeader(
        requestMessage, 
        WS_ACTION_HEADER, 
        WS_XML_STRING_TYPE,
        WS_READ_REQUIRED_VALUE, 
        NULL, 
        &receivedAction, 
        sizeof(receivedAction), 
        error);
if (FAILED(hr))
{
    goto Exit;
}

    // Make sure action is what we expect
    if (WsXmlStringEquals(
        &receivedAction, 
        PurchaseOrder_wsdl.messages.PurchaseOrder.action, 
        error) != S_OK)
    {
        hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
        goto Exit;
    }

    // Read purchase order
    _PurchaseOrderType purchaseOrder;
    hr = WsReadBody(
        requestMessage, 
        &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
        WS_READ_REQUIRED_VALUE, 
        heap, &purchaseOrder, 
        sizeof(purchaseOrder), 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    // Read end of message
    hr = WsReadMessageEnd(
        channel, 
        requestMessage, 
        NULL, 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    // Print out purchase order contents
    wprintf(L"%ld, %s\n", 
        purchaseOrder.quantity, 
        purchaseOrder.productName);
    
    // Initialize order confirmation data
    _OrderConfirmationType orderConfirmation;
    orderConfirmation.expectedShipDate = L"1/1/2006";
    orderConfirmation.orderID = 123;

    // Send a reply message
    hr = WsSendReplyMessage(
        channel, 
        replyMessage, 
        &PurchaseOrder_wsdl.messages.OrderConfirmation, 
        WS_WRITE_REQUIRED_VALUE,
        &orderConfirmation, 
        sizeof(orderConfirmation),
        requestMessage, 
        NULL, 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = WS_S_END;
    
Exit:
    fflush(stdout);
    if (replyMessage != NULL)
    {
        WsFreeMessage(
            replyMessage);
    }
    return hr;    
}

HRESULT CALLBACK CloseChannelCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);

    const ULONG totalItems = InterlockedIncrement(
        &numberOfItems);
    if (totalItems == 100)
    {
        SetEvent(closeServer);
    }
    return S_OK;
}


static const WS_SERVICE_CONTRACT messageContract = { NULL, ProcessMessage, NULL};

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    serviceEndpoints[0] = &serviceEndpoint;
    WS_ERROR* error = NULL;
    WS_SERVICE_ENDPOINT_PROPERTY serviceEndpointProperties[1];
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {CloseChannelCallback};
    serviceEndpointProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceEndpointProperties[0].value = &closeCallbackProperty;
    serviceEndpointProperties[0].valueSize = sizeof(closeCallbackProperty);
    
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
    serviceEndpoint.address.url.chars = L"http://+:80/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_HTTP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_REPLY; // the channel type
    serviceEndpoint.contract = &messageContract;  // the contract
    serviceEndpoint.properties = serviceEndpointProperties;
    serviceEndpoint.propertyCount = WsCountOf(serviceEndpointProperties);
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

