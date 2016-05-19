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


struct THREADINFO
{
    HANDLE event;
    HRESULT hr;
};

static const WS_STRING relayUrl = WS_STRING_VALUE(L"net.tcp://localhost:809/example/relay");
static const WS_STRING serviceUrl = WS_STRING_VALUE(L"net.tcp://localhost:808/example/service");

// Messages are sent from this thread to the relay thread
DWORD WINAPI SenderThread(
    __in void* parameter)
{
    UNREFERENCED_PARAMETER(parameter);
    
    HRESULT hr;
    WS_ERROR* error = NULL;
    WS_CHANNEL* sendChannel = NULL;
    WS_MESSAGE* sendMessage = NULL;
    
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
    hr = WsCreateChannel(WS_CHANNEL_TYPE_DUPLEX_SESSION,
        WS_TCP_CHANNEL_BINDING, NULL, 0, NULL, &sendChannel, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize address of service
    WS_ENDPOINT_ADDRESS address;
    address.url = relayUrl;
    address.headers = NULL;
    address.extensions = NULL;
    address.identity = NULL;
    
    // Open channel to address
    hr = WsOpenChannel(sendChannel, &address, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        sendChannel,
        NULL, 
        0, 
        &sendMessage, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Send some messages
    for (ULONG i = 0; i < 100; i++)
    {
        // Initialize send message
        hr = WsInitializeMessage(sendMessage, WS_BLANK_MESSAGE, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Add the action header
        hr = WsSetHeader(
            sendMessage, 
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
        
        // Send message start
        hr = WsWriteMessageStart(sendChannel, sendMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Initialize purchase order
        _PurchaseOrderType purchaseOrderToWrite;
        purchaseOrderToWrite.quantity = 100;
        purchaseOrderToWrite.productName = L"Pencil"; 
    
        // Write purchase order as the body of the message
        hr = WsWriteBody(
            sendMessage, 
            &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
            WS_WRITE_REQUIRED_VALUE,
            &purchaseOrderToWrite, 
            sizeof(purchaseOrderToWrite), 
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Send message end
        hr = WsWriteMessageEnd(sendChannel, sendMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Reset message to enable reuse
        hr = WsResetMessage(sendMessage, error);
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
    
    if (sendChannel != NULL)
    {
        // Close the channel
        WsCloseChannel(sendChannel, NULL, error);
    }
    if (sendMessage != NULL)
    {
        WsFreeMessage(sendMessage);
    }
    if (sendChannel != NULL)
    {
        WsFreeChannel(sendChannel);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    return 1;
}

// Messages arrive on this thread from the relay thread
DWORD WINAPI ReceiverThread(
    __in void* parameter)
{
    HRESULT hr;
    THREADINFO* threadInfo = (THREADINFO*) parameter;
    HANDLE receiverReadyEvent = threadInfo->event;
    WS_ERROR* error = NULL;
    WS_CHANNEL* receiveChannel = NULL;
    WS_MESSAGE* receiveMessage = NULL;
    WS_LISTENER* listener = NULL;
    WS_HEAP* heap = NULL;
    
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
    
    // Create a listener
    hr = WsCreateListener(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, NULL, 0, NULL, &listener, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Open listener using TCP duplex session
    hr = WsOpenListener(listener, &serviceUrl, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a channel
    hr = WsCreateChannelForListener(listener, NULL, 0, &receiveChannel, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Notify that thread is ready
    threadInfo->hr = S_OK;
    SetEvent(receiverReadyEvent);
    
    // Accept a channel from the client
    hr = WsAcceptChannel(listener, receiveChannel, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        receiveChannel,
        NULL, 
        0, 
        &receiveMessage, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Receive all messages
    for (;;)
    {
        // Receive start of message (headers)
        hr = WsReadMessageStart(receiveChannel, receiveMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        if (hr == WS_S_END)
        {
            // No more messages on this channel
            break;
        }
    
        // Get action value
        WS_XML_STRING receivedAction;
        hr = WsGetHeader(
            receiveMessage, 
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
    
        // Read the body of the message as a purchase order
        _PurchaseOrderType* purchaseOrder;
        hr = WsReadBody(receiveMessage, &PurchaseOrder_wsdl.globalElements.PurchaseOrderType,
            WS_READ_REQUIRED_POINTER, heap, &purchaseOrder, sizeof(purchaseOrder), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Print out purchase order
        wprintf(L"%ld, %s\n", 
            purchaseOrder->quantity, 
            purchaseOrder->productName);
    
        // Receive message end
        hr = WsReadMessageEnd(receiveChannel, receiveMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Reset message so we can use it again
        hr = WsResetMessage(receiveMessage, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Reset heap
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
    
    if (receiveChannel != NULL)
    {
        // Close the channel
        WsCloseChannel(receiveChannel, NULL, error);
    }
    if (receiveChannel != NULL)
    {
        WsFreeChannel(receiveChannel);
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
    
    if (receiveMessage != NULL)
    {
        WsFreeMessage(receiveMessage);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    if (FAILED(hr))
    {
        // Notify that thread is ready
        SetEvent(receiverReadyEvent);
        threadInfo->hr = hr;
    }
    return 1;
}

// Messages are relayed from the sender thread to the receiver thread by this thread
DWORD WINAPI RelayThread(
    __in void* parameter)
{
    HRESULT hr;
    THREADINFO* threadInfo = (THREADINFO*)parameter;
    HANDLE relayReadyEvent = threadInfo->event;
    WS_ERROR* error = NULL;
    WS_CHANNEL* sendChannel = NULL;
    WS_CHANNEL* receiveChannel = NULL;
    WS_MESSAGE* sendMessage = NULL;
    WS_MESSAGE* receiveMessage = NULL;
    WS_LISTENER* listener = NULL;
    
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
    hr = WsCreateListener(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, NULL, 0, NULL, &listener, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Open listener using TCP duplex session
    hr = WsOpenListener(listener, &relayUrl, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a TCP duplex session channel
    hr = WsCreateChannel(WS_CHANNEL_TYPE_DUPLEX_SESSION,
        WS_TCP_CHANNEL_BINDING, NULL, 0, NULL, &sendChannel, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize address of service
    WS_ENDPOINT_ADDRESS address;
    address.url = serviceUrl;
    address.headers = NULL;
    address.extensions = NULL;
    address.identity = NULL;
    
    // Open channel to address
    hr = WsOpenChannel(sendChannel, &address, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a channel
    hr = WsCreateChannelForListener(listener, NULL, 0, &receiveChannel, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Notify that thread is ready
    threadInfo->hr = S_OK;
    SetEvent(relayReadyEvent);
    
    // Accept a channel from the client
    hr = WsAcceptChannel(listener, receiveChannel, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        receiveChannel,
        NULL, 
        0, 
        &receiveMessage, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateMessageForChannel(
        sendChannel,
        NULL, 
        0, 
        &sendMessage, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Forward all messages
    for (;;)
    {
        // Receive start of message (headers)
        hr = WsReadMessageStart(receiveChannel, receiveMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        if (hr == WS_S_END)
        {
            // No more messages on this channel
            break;
        }
    
        // Copy headers from received message
        hr = WsInitializeMessage(sendMessage, WS_DUPLICATE_MESSAGE, receiveMessage, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Send message start (headers)
        hr = WsWriteMessageStart(sendChannel, sendMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Get reader for receive message
        WS_XML_READER* xmlReader;
        hr = WsGetMessageProperty(receiveMessage, WS_MESSAGE_PROPERTY_BODY_READER, &xmlReader, sizeof(xmlReader), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Get writer for send message
        WS_XML_WRITER* xmlWriter;
        hr = WsGetMessageProperty(sendMessage, WS_MESSAGE_PROPERTY_BODY_WRITER, &xmlWriter, sizeof(xmlWriter), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read and write contents of body
        for (;;)
        {
            const WS_XML_NODE* node;
            hr = WsGetReaderNode(xmlReader, &node, error);
            if (FAILED(hr))
            {
                goto Exit;
            }
    
            if (node->nodeType == WS_XML_NODE_TYPE_END_ELEMENT)
            {
                break;
            }
    
            hr = WsCopyNode(xmlWriter, xmlReader, error);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
    
        // Receive message end
        hr = WsReadMessageEnd(receiveChannel, receiveMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Send message end
        hr = WsWriteMessageEnd(sendChannel, sendMessage, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Reset message to enable reuse
        hr = WsResetMessage(sendMessage, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Reset message to enable reuse
        hr = WsResetMessage(receiveMessage, error);
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
    
    if (sendChannel != NULL)
    {
        // Close the channel
        WsCloseChannel(sendChannel, NULL, error);
    }
    if (sendChannel != NULL)
    {
        WsFreeChannel(sendChannel);
    }
    
    if (receiveChannel != NULL)
    {
        // Close the channel
        WsCloseChannel(receiveChannel, NULL, error);
    }
    if (receiveChannel != NULL)
    {
        WsFreeChannel(receiveChannel);
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
    
    if (receiveMessage != NULL)
    {
        WsFreeMessage(receiveMessage);
    }
    if (sendMessage != NULL)
    {
        WsFreeMessage(sendMessage);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (FAILED(hr))
    {
        // Notify that thread is ready
        threadInfo->hr = hr;
        SetEvent(relayReadyEvent);
    }
    return 1;
}

// Main entry point
// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    HANDLE receiverThreadHandle = NULL;
    HANDLE relayThreadHandle = NULL;
    HANDLE senderThreadHandle = NULL;
    THREADINFO receiverThreadInfo = { NULL, S_OK };
    THREADINFO relayThreadInfo = { NULL, S_OK };
    
    // Start Receiver thread and wait for it to be ready
    
    receiverThreadInfo.event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (receiverThreadInfo.event == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    receiverThreadHandle = CreateThread(NULL, 0, ReceiverThread, &receiverThreadInfo, 0, NULL);
    if (receiverThreadHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    WaitForSingleObject(receiverThreadInfo.event, INFINITE);
    if (FAILED(receiverThreadInfo.hr))
    {
        hr = receiverThreadInfo.hr;
        goto Exit;
    }
    
    // Start Relay thread and wait for it to be ready
    
    relayThreadInfo.event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (relayThreadInfo.event == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    relayThreadHandle = CreateThread(NULL, 0, RelayThread, &relayThreadInfo, 0, NULL);
    if (relayThreadHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    WaitForSingleObject(relayThreadInfo.event, INFINITE);
    if (FAILED(relayThreadInfo.hr))
    {
        hr = relayThreadInfo.hr;
        goto Exit;
    }
    
    // Start Sender thread
    
    senderThreadHandle = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL);
    if (senderThreadHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Wait for all threads to finish
    
Exit:
    if (FAILED(hr))
    {
        PrintError(hr, NULL);
    }
    if (receiverThreadHandle != NULL)
    {
        WaitForSingleObject(receiverThreadHandle, INFINITE);
    }
    if (senderThreadHandle != NULL)
    {
        WaitForSingleObject(senderThreadHandle, INFINITE);
    }
    if (relayThreadHandle != NULL)
    {
        WaitForSingleObject(relayThreadHandle, INFINITE);
    }
    
    if (receiverThreadInfo.event != NULL)
    {
        CloseHandle(receiverThreadInfo.event);
    }
    if (relayThreadInfo.event != NULL)
    {
        CloseHandle(relayThreadInfo.event);
    }
    if (receiverThreadHandle != NULL)
    {
        CloseHandle(receiverThreadHandle);
    }
    if (senderThreadHandle != NULL)
    {
        CloseHandle(senderThreadHandle);
    }
    if (relayThreadHandle != NULL)
    {
        CloseHandle(relayThreadHandle);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
