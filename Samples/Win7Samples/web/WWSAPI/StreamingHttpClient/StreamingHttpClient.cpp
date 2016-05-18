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
    WS_MESSAGE* requestMessage = NULL;
    WS_MESSAGE* replyMessage = NULL;
    WS_HEAP* heap = NULL;
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"http://localhost/example");
    WS_ENDPOINT_ADDRESS address = {};
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
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
    
    // Create a HTTP request channel
    hr = WsCreateChannel(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, 
        &transferModeProperty, 
        1, 
        NULL, 
        &channel, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Initialize address of service
    address.url = serviceUrl;
    
    // Open channel to address
    hr = WsOpenChannel(
        channel, 
        &address, 
        NULL, 
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
    
    // Send request messages and receive reply messages
    for (int i = 0; i < 10; i++)
    {
        // Initialize message headers of the request message
        hr = WsInitializeMessage(
            requestMessage, 
            WS_BLANK_MESSAGE, 
            NULL, 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Add the action header to the request message
        hr = WsSetHeader(
            requestMessage, 
            WS_ACTION_HEADER, 
            WS_XML_STRING_TYPE,
            WS_WRITE_REQUIRED_VALUE,
            PurchaseOrder_wsdl.messages.PurchaseOrder.action, 
            sizeof(*PurchaseOrder_wsdl.messages.PurchaseOrder.action), 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        
        // Generate a unique message ID that will be used for the request message
        WS_UNIQUE_ID messageID;
        ZeroMemory(
            &messageID, 
            sizeof(messageID));
        
        DWORD status = UuidCreate(
            &messageID.guid);
        if (status != RPC_S_OK)
        {
            hr = E_FAIL;
            goto Exit;
        }
    
        // Add the message ID to the request message
        hr = WsSetHeader(
            requestMessage, 
            WS_MESSAGE_ID_HEADER, 
            WS_UNIQUE_ID_TYPE,
            WS_WRITE_REQUIRED_VALUE,
            &messageID, 
            sizeof(messageID), 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Send the message headers of the request message
        hr = WsWriteMessageStart(
            channel, 
            requestMessage, 
            NULL, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Stream out some purchase orders
        for (int j = 0; j < 10; j++)
        {
            // Initialize body data
            _PurchaseOrderType purchaseOrder;
            purchaseOrder.quantity = 1;
            purchaseOrder.productName = L"Pencil";
    
            // Serialize body data into message
            hr = WsWriteBody(
                requestMessage, 
                &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
                WS_WRITE_REQUIRED_VALUE,
                &purchaseOrder, 
                sizeof(purchaseOrder),
                error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
            // Send accumulated message data once at least 4096 bytes have been accumulated
            hr = WsFlushBody(
                requestMessage, 
                4096, 
                NULL, 
                error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        }
    
        // Send the end of the request message
        hr = WsWriteMessageEnd(
            channel, 
            requestMessage, 
            NULL, 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Receive the headers of the reply message
        hr = WsReadMessageStart(
            channel, 
            replyMessage, 
            NULL, 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Stream in all the confirmations
        for (;;)
        {
            // Make sure we have at least once confirmation buffered.  Each confirmation
            // may be up to 1024 bytes in size.
            hr = WsFillBody(
                replyMessage, 
                1024, 
                NULL, 
                error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
            // Try to deserialize a confirmation into the heap
            _OrderConfirmationType* orderConfirmation;
            hr = WsReadBody(
                replyMessage, 
                &PurchaseOrder_wsdl.globalElements.OrderConfirmationType,
                WS_READ_OPTIONAL_POINTER, 
                heap, 
                &orderConfirmation, 
                sizeof(orderConfirmation), 
                error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
            // If there are no more confirmations, break out of the loop
            if (orderConfirmation == NULL)
            {
                break;
            }
    
            // Print out confirmation contents
            wprintf(L"%s\n",
                orderConfirmation->expectedShipDate);
    
            // Reset the heap which frees the confirmation data that was deserialized
            hr = WsResetHeap(
                heap, 
                error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        }
    
        // Receive the end of the reply message
        hr = WsReadMessageEnd(
            channel, 
            replyMessage, 
            NULL, 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Reset message so it can be used again
        hr = WsResetMessage(
            replyMessage, 
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        // Reset message so it can be used again
        hr = WsResetMessage(
            requestMessage, 
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
    if (requestMessage != NULL)
    {
        WsFreeMessage(requestMessage);
    }
    if (replyMessage != NULL)
    {
        WsFreeMessage(replyMessage);
    }
    if (channel != NULL)
    {
        WsFreeChannel(channel);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
