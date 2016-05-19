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
    WS_MESSAGE* requestMessage = NULL;
    WS_MESSAGE* replyMessage = NULL;
    static const WS_STRING uri = WS_STRING_VALUE(L"http://+:80/example");
    
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
        WS_CHANNEL_TYPE_REPLY, 
        WS_HTTP_CHANNEL_BINDING, 
        NULL, 0, 
        NULL, 
        &listener, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Open listener 
    hr = WsOpenListener(
        listener, 
        &uri, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set up a property indicating streamined input and output
    WS_TRANSFER_MODE transferMode = WS_STREAMED_TRANSFER_MODE;
    WS_CHANNEL_PROPERTY transferModeProperty;
    transferModeProperty.id = WS_CHANNEL_PROPERTY_TRANSFER_MODE;
    transferModeProperty.value = &transferMode;
    transferModeProperty.valueSize = sizeof(transferMode);
    
    // Create a channel suitable for accepting from the listener
    hr = WsCreateChannelForListener(
        listener, 
        &transferModeProperty, 
        1, 
        &channel, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        channel,
        NULL, 
        0, 
        &requestMessage, 
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
    
    
    // Receive messages and send replies
    for (int i = 0; i < 10; i++)
    {
        // Accept a channel from the client
        hr = WsAcceptChannel(listener, channel, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Receive the message start (headers)
        hr = WsReadMessageStart(
            channel, 
            requestMessage, 
            NULL, 
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
            WS_READ_REQUIRED_VALUE, NULL, 
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
        
        // Initialize the reply message based on the request
        hr = WsInitializeMessage(
            replyMessage, 
            WS_REPLY_MESSAGE, 
            requestMessage, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Write the start of the reply message (headers)
        hr = WsWriteMessageStart(
            channel, 
            replyMessage, 
            NULL, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Read the contents of the request body, and send response body
        for (;;)
        {
            // Make sure we have at least one purchase order buffered in the request message.  
            // Each purchase order may be up to 1024 bytes in size.
            hr = WsFillBody(
                requestMessage, 
                1024, 
                NULL, 
                error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            // Deserialize purchase order into heap (if any more)
            _PurchaseOrderType* purchaseOrder;
            hr = WsReadBody(
                requestMessage, 
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
            
            // Serialize a confirmation into the reply message
            _OrderConfirmationType orderConfirmation;
            orderConfirmation.expectedShipDate = L"1/1/2006";
            orderConfirmation.orderID = 123;
            
            hr = WsWriteBody(
                replyMessage, 
                &PurchaseOrder_wsdl.globalElements.OrderConfirmationType, 
                WS_WRITE_REQUIRED_VALUE,
                &orderConfirmation, 
                sizeof(orderConfirmation),
                error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            
            // Flush the confirmation data if at least 4096 bytes have been accumulated
            hr = WsFlushBody(
                replyMessage, 
                4096, 
                NULL, 
                error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            
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
            requestMessage, 
            NULL, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Write the end of the message
        hr = WsWriteMessageEnd(
            channel, 
            replyMessage, 
            NULL, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Reset the message so it can be used again
        hr = WsResetMessage(
            requestMessage, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Reset the message so it can be used again
        hr = WsResetMessage(
            replyMessage, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        if (channel != NULL)
        {
            // Close the channel
            WsCloseChannel(channel, NULL, error);
        }
        
        // Reset the channel so it can be used again
        hr = WsResetChannel(
            channel, 
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
    if (requestMessage != NULL)
    {
        WsFreeMessage(requestMessage);
    }
    if (replyMessage != NULL)
    {
        WsFreeMessage(replyMessage);
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
