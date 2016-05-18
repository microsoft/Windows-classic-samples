//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#endif
#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "PurchaseOrder.wsdl.h"
#include "winsock2.h"
#include "Iphlpapi.h"

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
    WS_MESSAGE* requestMessage = NULL;
    WS_CHANNEL* channel = NULL;
    WS_LISTENER* listener = NULL;
    WS_MESSAGE* replyMessage = NULL;
    WS_HEAP* heap = NULL;
    ULONG addressFamily = AF_INET;
    ULONG retVal = 0;
    IP_ADAPTER_ADDRESSES* adapterAddresses = NULL;
    
    // To get list of adapters GetAdaptersAddresses is called twice - first to get size of the list
    // and second to get the actual list. As the number of adapters may change between these two calls
    // to GetAdaptersAddresses, we retry up to 4 times.
    for (ULONG i = 0; i < 4; i++)
    {
        // Free memory if it was not big enough
        if (adapterAddresses != NULL)
        {
            HeapFree(GetProcessHeap(), 0, adapterAddresses);
            adapterAddresses = NULL;
        }
        
        // First see how much space is needed for adapter addresses
        ULONG adapterBufferSize = 0;
        retVal = GetAdaptersAddresses(addressFamily, 0, NULL, NULL, &adapterBufferSize);
        if (retVal != ERROR_BUFFER_OVERFLOW)
        {
            hr = HRESULT_FROM_WIN32(retVal);
            goto Exit;
        }
            
        // Allocate space for information about adapters
        adapterAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, adapterBufferSize);
        if (adapterAddresses == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        // Get list of adapters
        retVal = GetAdaptersAddresses(AF_INET, 0, NULL, adapterAddresses, &adapterBufferSize);
        
        if (retVal != ERROR_BUFFER_OVERFLOW)
        {
            break;
        }
        // number of adapter have changed between calls to GetAdaptersAddresses, retry.
    }
    
    if (retVal != 0)
    {
        hr = HRESULT_FROM_WIN32(retVal);
        goto Exit;
    }
    
    
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
    
    WS_LISTENER_PROPERTY properties[2];
    // Specify that the listener will be used with multicast addresses
    BOOL isMulticast = TRUE;
    properties[0].id = WS_LISTENER_PROPERTY_IS_MULTICAST;
    properties[0].value = &isMulticast;
    properties[0].valueSize = sizeof(isMulticast);
    // Disable checking of the To header since it will differ from the transport address
    ULONG toHeaderMatchingOptions = 0;
    properties[1].id = WS_LISTENER_PROPERTY_TO_HEADER_MATCHING_OPTIONS;
    properties[1].value = &toHeaderMatchingOptions;
    properties[1].valueSize = sizeof(toHeaderMatchingOptions);
    
    // Create a listener
    hr = WsCreateListener(
        WS_CHANNEL_TYPE_DUPLEX, 
        WS_UDP_CHANNEL_BINDING, 
        properties, 
        WsCountOf(properties), 
        NULL, 
        &listener, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Open listener at address
    WS_STRING uri;
    uri.chars = L"soap.udp://239.255.255.250:809";
    uri.length = (ULONG)wcslen(uri.chars);
    
    hr = WsOpenListener(
        listener, 
        &uri, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    BOOL foundLocalAdapter = FALSE;
    
    // For each adapter
    for (IP_ADAPTER_ADDRESSES* adapterAddress = adapterAddresses; 
         adapterAddress != NULL; adapterAddress = adapterAddress->Next)
    {
        // Only listen on the loopback adapter
        if (adapterAddress->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
        {
            // Get multicast interface index
            ULONG interfaceIndex = adapterAddress->IfIndex;
    
            // Specify that the listener should listen for multicast 
            // traffic on this interface.
            hr = WsSetListenerProperty(listener, WS_LISTENER_PROPERTY_MULTICAST_INTERFACES, 
                &interfaceIndex, sizeof(interfaceIndex), error);
            if (FAILED(hr))
            {
                goto Exit;
            }
    
            foundLocalAdapter = TRUE;
    
            break;
        }
    }
    
    if (!foundLocalAdapter)
    {
        wprintf(L"No local adapter found\n");
        hr = E_FAIL;
        goto Exit;
    }
    
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
    
    
    // Receive two multicast requestMessages
    for (int i = 0; i < 2; i++)
    {
        // Wait for requestMessage
        // Accept a channel from the client
        hr = WsAcceptChannel(listener, channel, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Read start of requestMessage (headers)
        hr = WsReadMessageStart(channel, requestMessage, NULL, error);
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
        if (WsXmlStringEquals(&receivedAction, PurchaseOrder_wsdl.messages.PurchaseOrder.action, error) != S_OK)
        {
            hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
            goto Exit;
        }
        
        // Read purchase order
        _PurchaseOrderType* purchaseOrder;
        hr = WsReadBody(requestMessage, &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
            WS_READ_REQUIRED_POINTER, heap, &purchaseOrder, sizeof(purchaseOrder), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Read end of requestMessage
        hr = WsReadMessageEnd(channel, requestMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Print out purchase order contents
        wprintf(L"%ld, %s\n", 
            purchaseOrder->quantity, 
            purchaseOrder->productName);
        
        // Get the address that the datagram was received from
        WS_ENDPOINT_ADDRESS remoteAddress;
        hr = WsGetChannelProperty(
            channel, 
            WS_CHANNEL_PROPERTY_REMOTE_ADDRESS, 
            &remoteAddress, 
            sizeof(remoteAddress), 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Initialize order confirmation data
        _OrderConfirmationType orderConfirmation;
        orderConfirmation.expectedShipDate = L"1/8/2006";
        orderConfirmation.orderID = 123;
        
        // Send a reply message.  The reply message will be sent to the
        // source IP address of the received datagram.  This function
        // will automatically set the RelatesTo header of the replyMessage
        // to the MessageID of the requestMessage.
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
        
        // Reset replyMessage so it can be used again
        hr = WsResetMessage(replyMessage, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Reset requestMessage so it can be used again
        hr = WsResetMessage(requestMessage, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        hr = WsCloseChannel(channel, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        hr = WsResetChannel(channel, error);
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
    if (channel != NULL)
    {
        WsFreeChannel(channel);
    }
    
    if (listener != NULL)
    {
        // Close the listener if it was opened
        WsCloseListener(listener, NULL, error);
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
