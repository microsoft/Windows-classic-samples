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


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_CHANNEL* channel = NULL;
    WS_LISTENER* listener = NULL;
    WS_HEAP* heap = NULL;
    WS_MESSAGE* message = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a listener
    hr = WsCreateListener(
        WS_CHANNEL_TYPE_DUPLEX_SESSION, 
        WS_TCP_CHANNEL_BINDING, 
        NULL, 
        0, 
        NULL, 
        &listener, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a channel suitable for the listener
    hr = WsCreateChannelForListener(
        listener, 
        NULL, 
        0, 
        &channel, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Open listener using TCP duplex session
    WS_STRING uri;
    uri.chars = L"net.tcp://localhost/example";
    uri.length = (ULONG)::wcslen(uri.chars);
    hr = WsOpenListener(
        listener, 
        &uri, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Accept a channel from the client
    hr = WsAcceptChannel(listener, channel, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        channel,
        NULL, 
        0, 
        &message, 
        error);
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
    
    // Receive messages
    for (;;)
    {
            // Receive the message start (headers)
            hr = WsReadMessageStart(
                channel, 
                message, 
                NULL, 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            if (hr == WS_S_END)
            {
                // No more messages
                break;
            }
        
            // Get action value
            WS_XML_STRING receivedAction;
            hr = WsGetHeader(
                message, 
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
            hr = WsXmlStringEquals(
                &receivedAction, 
                PurchaseOrder_wsdl.messages.PurchaseOrder.action, 
                error);
            
            if (hr != S_OK)
            {
                hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
                goto Exit;
            }
        
        
            // Get the reader for the body
            WS_XML_READER* reader;
            hr = WsGetMessageProperty(
                message, 
                WS_MESSAGE_PROPERTY_BODY_READER, 
                &reader, 
                sizeof(reader), 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            // Stream in the body data
            for (;;)
            {
                    // Read purchase order into heap, if there are any more.
                    _PurchaseOrderType* purchaseOrder;
                    hr = WsReadElement(
                        reader, 
                        &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
                        WS_READ_OPTIONAL_POINTER, 
                        heap, 
                        &purchaseOrder, 
                        sizeof(purchaseOrder), 
                        error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            
                    // NULL indicates no more purchase orders
                    if (purchaseOrder == NULL)
                    {
                        break;
                    }
            
                    // Print out purchase order contents
                    wprintf(L"%ld, %s\n", 
                        purchaseOrder->quantity, 
                        purchaseOrder->productName);
            
                    // Free purchase order
                    hr = WsResetHeap(
                        heap, 
                        error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            }
        
            // Read the end of the message
            hr = WsReadMessageEnd(
                channel, 
                message, 
                NULL, 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            hr = WsResetMessage(
                message, 
                error);
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
    
    if (channel != NULL)
    {
        // Close the channel
        WsCloseChannel(channel, NULL, error);
    }
    if (listener != NULL)
    {
        // Close the listener if it was opened
        WsCloseListener(listener, NULL, error);
    }
    if (channel != NULL)
    {
        WsFreeChannel(channel);
    }
    if (listener != NULL)
    {
        WsFreeListener(listener);
    }
    if (message != NULL)
    {
        WsFreeMessage(message);
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
