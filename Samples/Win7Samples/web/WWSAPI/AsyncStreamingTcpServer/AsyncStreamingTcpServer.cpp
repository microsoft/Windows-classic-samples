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


struct RECEIVE_STATE
{
    WS_LISTENER* listener;
    WS_CHANNEL* channel;
    WS_MESSAGE* message;
    WS_HEAP* heap;
    WS_XML_READER* reader;
};

HRESULT CALLBACK Receive1(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive2(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive3(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive4(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive5(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive6(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive7(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive8(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
   
HRESULT CALLBACK Receive9(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);
    
HRESULT CALLBACK Receive10(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error);   
    
HRESULT CALLBACK Receive1(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    // Create a listener
    hr = WsCreateListener(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, NULL, 0, NULL, &receiveState->listener, error);
    if (FAILED(hr))
    {
        return hr;
    }

    // Open listener using TCP duplex session
    static const WS_STRING uri = WS_STRING_VALUE(L"net.tcp://localhost/example");
    next->function = Receive2;
    return WsOpenListener(receiveState->listener, &uri, asyncContext, error);
}

HRESULT CALLBACK Receive2(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }


    hr = WsCreateChannelForListener(receiveState->listener, NULL, 0, &receiveState->channel, error);
    if (FAILED(hr))
    {
        return hr;
    }

    // Accept a channel from the client
    next->function = Receive3;
    return WsAcceptChannel(receiveState->listener, receiveState->channel, asyncContext, error);
}

HRESULT CALLBACK Receive3(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    hr = WsCreateMessageForChannel(receiveState->channel, NULL, 0, &receiveState->message, error);
    if (FAILED(hr))
    {
        return hr;
    }

    // Stop listening for channels
    next->function = Receive4;
    return WsCloseListener(receiveState->listener, asyncContext, error);
}

HRESULT CALLBACK Receive4(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    // Receive the message start (headers)
    next->function = Receive5;
    return WsReadMessageStart(receiveState->channel, receiveState->message, asyncContext, error);
}

HRESULT CALLBACK Receive5(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    if (hr == WS_S_END)
    {
        next->function = NULL;
        // Close the channel
        return WsCloseChannel(receiveState->channel, asyncContext, error);
    }

    // Get action value
    WS_XML_STRING receivedAction;
    hr = WsGetHeader(
        receiveState->message, 
        WS_ACTION_HEADER, 
        WS_XML_STRING_TYPE,
        WS_READ_REQUIRED_VALUE, 
        NULL, 
        &receivedAction, 
        sizeof(receivedAction), 
        error);
    if (FAILED(hr))
    {
        return hr;
    }

    // Make sure action is what we expect
    if (WsXmlStringEquals(&receivedAction, PurchaseOrder_wsdl.messages.PurchaseOrder.action, error) != S_OK)
    {
        return WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
    }

    // Get the reader for the body
    hr = WsGetMessageProperty(receiveState->message, WS_MESSAGE_PROPERTY_BODY_READER, &receiveState->reader, sizeof(receiveState->reader), error);
    if (FAILED(hr))
    {
        return hr;
    }

    next->function = Receive6;
    return S_OK;
}

HRESULT CALLBACK Receive6(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    next->function = Receive8;
    return WsFillReader(receiveState->reader, 128, asyncContext, error);
}

HRESULT CALLBACK Receive8(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);
    UNREFERENCED_PARAMETER(asyncContext);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    // Read purchase order into heap, if there are any more to read.
    _PurchaseOrderType* purchaseOrder;
    hr = WsReadElement(receiveState->reader, &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
        WS_READ_OPTIONAL_POINTER, receiveState->heap, &purchaseOrder, sizeof(purchaseOrder), error);
    if (FAILED(hr))
    {
        return hr;
    }

    // If NULL indicates no more purchase orders
    if (purchaseOrder == NULL)
    {
        next->function = Receive9;
        return S_OK;
    }

    wprintf(L"%ld, %s\n", 
        purchaseOrder->quantity, 
        purchaseOrder->productName);
    fflush(stdout);

    // Free purchase order
    hr = WsResetHeap(receiveState->heap, error);
    if (FAILED(hr))
    {
        return hr;
    }

    next->function = Receive6;
    return S_OK;
}
    
HRESULT CALLBACK Receive9(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    // Free purchase order
    hr = WsResetHeap(receiveState->heap, error);
    if (FAILED(hr))
    {
        return hr;
    }

    next->function = Receive10;
    return WsReadMessageEnd(receiveState->channel, receiveState->message, asyncContext, error);
}

HRESULT CALLBACK Receive10(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackModel);
    UNREFERENCED_PARAMETER(asyncContext);

    RECEIVE_STATE* receiveState = (RECEIVE_STATE*) state;
    if (FAILED(hr))
    {
        return hr;
    }

    hr = WsResetMessage(receiveState->message, error);
    if (FAILED(hr))
    {
        return hr;
    }
    next->function = Receive4;
    return S_OK;
}

struct THREAD_INFO
{
    HRESULT hr;
    HANDLE handle;
};

static void CALLBACK OnReceiveComplete(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state)
{
    UNREFERENCED_PARAMETER(callbackModel);

    THREAD_INFO* threadInfo = (THREAD_INFO*)state;
    threadInfo->hr = hr;
    SetEvent(threadInfo->handle);
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    
    WS_ASYNC_STATE asyncState;
    RECEIVE_STATE receiveState;
    receiveState.listener = NULL;
    receiveState.channel = NULL;
    receiveState.message = NULL;
    receiveState.heap = NULL;
    receiveState.reader = NULL;
    
    THREAD_INFO threadInfo;
    threadInfo.hr = S_OK;
    threadInfo.handle = NULL;
        
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        
    // Create a heap to hold body data, with a max size to limit size of purchase order read
    hr = WsCreateHeap(/*maxSize*/ 1024, /*trimSize*/ 1024, NULL, 0, &receiveState.heap, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    threadInfo.handle = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (threadInfo.handle == NULL)
    {
        goto Exit;
    }
    
    WS_ASYNC_CONTEXT receiveComplete;
    receiveComplete.callback = OnReceiveComplete;
    receiveComplete.callbackState = &threadInfo;
    
    hr = WsAsyncExecute(&asyncState, Receive1, WS_LONG_CALLBACK, &receiveState, &receiveComplete, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    if (hr == WS_S_ASYNC)
    {
        WaitForSingleObject(threadInfo.handle, INFINITE);
        hr = threadInfo.hr;
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
    
    if (receiveState.channel != NULL)
    {
        // Close the channel
        WsCloseChannel(receiveState.channel, NULL, error);
    }
    if (receiveState.channel != NULL)
    {
        WsFreeChannel(receiveState.channel);
    }
    if (receiveState.listener != NULL)
    {
        // Close the listener if it was opened
        WsCloseListener(receiveState.listener, NULL, error);
    }
    if (receiveState.listener != NULL)
    {
        WsFreeListener(receiveState.listener);
    }
    if (receiveState.message != NULL)
    {
        WsFreeMessage(receiveState.message);
    }
    if (receiveState.heap != NULL)
    {
        WsFreeHeap(receiveState.heap);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    
    
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
