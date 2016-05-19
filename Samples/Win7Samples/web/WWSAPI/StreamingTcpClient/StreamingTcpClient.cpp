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
    WS_MESSAGE* message = NULL;
    __nullterminated WCHAR* productName = L"Pencil";
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Create a TCP duplex session channel
    hr = WsCreateChannel(
        WS_CHANNEL_TYPE_DUPLEX_SESSION, 
        WS_TCP_CHANNEL_BINDING, 
        NULL, 
        0, 
        NULL, 
        &channel, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize address of service
    WS_ENDPOINT_ADDRESS address;
    address.url.chars = L"net.tcp://localhost/example";
    address.url.length = (ULONG)::wcslen(address.url.chars);
    address.headers = NULL;
    address.extensions = NULL;
    address.identity = NULL;
    
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
        &message, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Send some messages
    for (int i = 0; i < 100; i++)
    {
            // Initialize message headers
            hr = WsInitializeMessage(
                message, 
                WS_BLANK_MESSAGE, 
                NULL, 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            // Add the action header
            hr = WsSetHeader(
                message, 
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
        
            // Send the message headers
            hr = WsWriteMessageStart(
                channel, 
                message, 
                NULL, 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            // Get the writer for the body
            WS_XML_WRITER* writer;
            hr = WsGetMessageProperty(message, WS_MESSAGE_PROPERTY_BODY_WRITER, &writer, sizeof(writer), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            // Stream out the body contents
            for (int j = 0; j < 100; j++)
            {
                    // Initialize body data
                    _PurchaseOrderType purchaseOrder;
                    purchaseOrder.quantity = 1;
                    purchaseOrder.productName = productName;
            
                    // Write body data
                    hr = WsWriteElement(
                        writer, 
                        &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
                        WS_WRITE_REQUIRED_VALUE,
                        &purchaseOrder, 
                        sizeof(purchaseOrder), 
                        error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            }
        
            // Send the end of the message
            hr = WsWriteMessageEnd(
                channel, 
                message, 
                NULL, 
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
            // Reset message so it can be used again
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
    if (message != NULL)
    {
        WsFreeMessage(message);
    }
    if (channel != NULL)
    {
        WsFreeChannel(channel);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
