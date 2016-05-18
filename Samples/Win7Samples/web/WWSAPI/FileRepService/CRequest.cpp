//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#include "Service.h"
#include "strsafe.h"
#include "stdlib.h"
#include "intsafe.h"
#include "assert.h"

// This function closes the channel if it was openend and then frees it.
void CleanupChannel(
    __in_opt WS_CHANNEL* channel)
{
    ULONG state = 0;

    if (NULL == channel)
    {
        return;
    }        
#if (DBG || _DEBUG)
    HRESULT hr = WsGetChannelProperty(channel, WS_CHANNEL_PROPERTY_STATE, &state, sizeof(state), NULL);
    assert(SUCCEEDED(hr));
#else
    (void)WsGetChannelProperty(channel, WS_CHANNEL_PROPERTY_STATE, &state, sizeof(state), NULL);
#endif

        
    if (WS_CHANNEL_STATE_OPEN == state || WS_CHANNEL_STATE_FAULTED == state)
    {
        // CloseChannel will close the channel even if it encouters an error. So ignore the error here
        // as this is called only when we destroy the channel.
        WsCloseChannel(channel, NULL, NULL);
    }

    WsFreeChannel(channel);
}

CRequest::CRequest(
    __in CFileRep* server)
{
    assert(NULL != server);
    this->server = server;
    
    channel = NULL;
    requestMessage = NULL;
    replyMessage = NULL;
    error = NULL;
    channelInUse = false;    
}

// Preallocate all state
HRESULT CRequest::Initialize()
{
    assert(NULL == channel);
    assert(NULL == requestMessage);
    assert(NULL == replyMessage);
    assert(NULL == error);

    HRESULT hr = S_OK;

    ULONG propertyCount = 0;
    WS_ENCODING encoding;

    WS_CHANNEL_PROPERTY encodingProperty;
    encodingProperty.id = WS_CHANNEL_PROPERTY_ENCODING;
    
    server->GetEncoding(&encoding, &propertyCount);
    encodingProperty.value = &encoding;
    encodingProperty.valueSize = sizeof(encoding);
    
    IfFailedExit(WsCreateError(NULL, 0, &error));
    IfFailedExit(WsCreateChannelForListener(server->GetListener(), &encodingProperty, propertyCount, &channel, NULL));        
    IfFailedExit(WsCreateMessageForChannel(channel, NULL, 0, &requestMessage, NULL));    
    IfFailedExit(WsCreateMessageForChannel(channel, NULL, 0, &replyMessage, NULL));
            
    EXIT   

    return hr;
}

CRequest* CRequest::GetRequest(
    __in void* callbackState)
{
    assert(NULL != callbackState);
    return ((CRequest *) callbackState);   
}

#pragma warning(disable : 4100) // The callbacks don't always use all parameters.

// The static callback functions.
HRESULT CALLBACK CRequest::ResetChannelCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{       
    return GetRequest(callbackState)->ResetChannel(hr, next, callbackModel, error);
}

HRESULT CALLBACK CRequest::AcceptChannelCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{  
    return GetRequest(callbackState)->AcceptChannel(hr, next, callbackModel, asyncContext, error);
}

HRESULT CALLBACK CRequest::ReceiveFirstMessageCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{         
    return GetRequest(callbackState)->ReceiveFirstMessage(hr, next, callbackModel);
}

HRESULT CALLBACK CRequest::ReceiveMessageCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{    
    return GetRequest(callbackState)->ReceiveMessage(hr, next, callbackModel, asyncContext, error);
}

HRESULT CALLBACK CRequest::ReadHeaderCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    return GetRequest(callbackState)->ReadHeader(hr, next, callbackModel, error);
}

HRESULT CRequest::CloseChannelCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    return GetRequest(callbackState)->CloseChannel(hr, next, callbackModel, asyncContext, error);
}

HRESULT CRequest::RequestCompleteCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{            
    return GetRequest(callbackState)->RequestComplete(hr, next);
}

HRESULT CRequest::HandleFailureCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    return GetRequest(callbackState)->HandleFailure(hr, next, error);
}

#pragma warning(default : 4100)

// This is the main service loop used to process requests. It is identical for both the client and server service.
// The functions are listed in the order in which they are called. The static callback functions are put
// seperately as they don't really do anything.

// Creates or resets channel and associated data structures.
HRESULT CRequest::ResetChannel(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in_opt WS_ERROR* error)
{
    PrintVerbose(L"Entering CRequest::ResetChannel"); 

    // We requested a long callback but got a short one. This is an error conditon usually 
    // triggered by resource shortage. So treat it that way. 
    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    // We always check for failures of the prior function in the next function. This simplifies error handling.
    if (FAILED(hr))
    {
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::ResetChannel"); 
        return hr;
    }

    next->function = CRequest::AcceptChannelCallback;

    IfFailedExit(WsResetError(error));
    IfFailedExit(WsResetChannel(channel, error));
    IfFailedExit(WsResetMessage(requestMessage, error));     
    IfFailedExit(WsResetMessage(replyMessage, error));  

    PrintVerbose(L"Leaving CRequest::ResetChannel");
    return S_OK;   


    ERROR_EXIT   

    server->PrintError(L"CRequest::ResetChannel", true);
    server->PrintError(hr, error, true);         
     
    PrintVerbose(L"Leaving CRequest::ResetChannel");
    return hr;
}

// Accepts an incoming request on the channel.
HRESULT CRequest::AcceptChannel(
    __in HRESULT hr,
    __inout WS_ASYNC_OPERATION* next, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    PrintVerbose(L"Entering CRequest::AcceptChannel");

    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::AcceptChannel"); 
        return hr;
    }

    next->function = CRequest::ReceiveFirstMessageCallback;  
    
    PrintVerbose(L"Leaving CRequest::AcceptChannel");
    return WsAcceptChannel(server->GetListener(), channel, asyncContext, error);;
}

// Special case for the first message received to keep the bookkeeping of active channels in order. 
HRESULT CRequest::ReceiveFirstMessage(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next, 
    __in WS_CALLBACK_MODEL callbackModel) 
{
    PrintVerbose(L"Entering CRequest::ReceiveFirstMessage");

    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        // We are not destroying a channel on failure, and we also cannot put the channel to sleep
        // on all failures as that opens up DoS attacks. However, this particular failure is different.
        // It signifies a failure in the infrastructure, and we do not want to spin on this failure.
        // So give it some breathing room to recover, unless we are shut down.
        // Obviously 5 seconds is a heuristic, but a more complex algorithm is out of the scope of this sample.
        if (server->GetChannelManager()->IsRunning())
        {
            Sleep(5000);
        }
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::ReceiveFirstMessage"); 
        return hr;
    }

    next->function = CRequest::ReceiveMessageCallback;

    channelInUse = true;

    server->GetChannelManager()->ChannelInUse();
    PrintVerbose(L"Leaving CRequest::ReceiveFirstMessage");

    return hr;
}

// This function and the next (and their non-static counterparts) represent the message processing loop. 
// WsAsyncExecute will loop between these functions until the channel is closed.
HRESULT CRequest::ReceiveMessage(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::ReceiveMessage"); 
        return hr;
    }

    next->function = CRequest::ReadHeaderCallback;
      
    PrintVerbose(L"Leaving CRequest::ReceiveMessage");
    return WsReadMessageStart(channel, requestMessage, asyncContext, error);
}

HRESULT CRequest::ReadHeader(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next,
    __in WS_CALLBACK_MODEL callbackModel, 
    __in_opt WS_ERROR* error)
{
    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::ReadHeader"); 
        return hr;
    }

    // We are done. Break the loop.
    if (hr == WS_S_END)
    {        
        next->function = CRequest::CloseChannelCallback;

        server->PrintVerbose(L"Leaving CRequest::ReadHeader");        
        return S_OK;
    }

    next->function = CRequest::ReceiveMessageCallback;    

    // Get action value
    WS_XML_STRING* receivedAction = NULL;
    IfFailedExit(WsGetHeader(
        requestMessage, 
        WS_ACTION_HEADER, 
        WS_XML_STRING_TYPE,
        WS_READ_REQUIRED_POINTER,  
        NULL, 
        &receivedAction, 
        sizeof(receivedAction), 
        error));

    // This function is implemented by the derived classes, so the execution forks 
    // depending on whether we are client or server.
    IfFailedExit(server->ProcessMessage(this, receivedAction));
    IfFailedExit(WsResetMessage(requestMessage, error));
    
    PrintVerbose(L"Leaving CRequest::ReadHeader");
    return S_OK;

    ERROR_EXIT

    if (WS_E_ENDPOINT_ACTION_NOT_SUPPORTED != hr)
    {
        server->PrintError(L"CRequest::ReadHeader", false);
        server->PrintError(hr, error, false); 
    }
        
    PrintVerbose(L"Leaving CRequest::ReadHeader");    
    return hr;
}

HRESULT CRequest::CloseChannel(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    PrintVerbose(L"Entering CRequest::CloseChannel"); 

    if (WS_SHORT_CALLBACK == callbackModel)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        next->function = CRequest::HandleFailureCallback;
        PrintVerbose(L"Leaving CRequest::CloseChannel"); 
        return hr;
    }
    else if (S_FALSE != hr)
    {
        // WsCloseChannel overwrites the error so print this here.
        // Note: We also print this if for example the file was 
        // not found as this is not an error from our end.
        server->PrintInfo(L"Request completed without error.");
    }

    next->function = CRequest::RequestCompleteCallback;

    PrintVerbose(L"Leaving CRequest::CloseChannel"); 
    return WsCloseChannel(channel, asyncContext, error); 
}

HRESULT CRequest::RequestComplete(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next)
{
    PrintVerbose(L"Entering CRequest::RequestComplete");
    
    // The function that got us here is WsCloseChannel. If the channel is in a closeable state,
    // WsCloseChannel is guaranteed to close it. However, it may not be able to close it gracefully.
    // If it is not then it will return an error. As the channel is still getting closed we do not
    // treat that as an error here and thus only print and informational message.
    // If the channel is not in a closeable state, WsCloseChannel will return WS_E_INVALID_OPERATION 
    // and leave the channel unchanged. That is bad because it means our state machine is broken as
    // the channel should be open or faulted when we call WsCloseChannel, and those states are closeable.

    // We don't check for the proper callback type here either, because this is just a pass bookkeeping function.

    assert(hr != WS_E_INVALID_OPERATION);

    if (FAILED(hr))
    {
        server->PrintInfo(L"WsCloseChannel failed. Channel was closed ungracefully.");
    }

    CChannelManager* manager = server->GetChannelManager();        
             
    if (manager->ShouldDestroyChannel())
    {
        // The channel is not needed. Destroy it.
        next->function = NULL; 
    }
    else
    {
        next->function = CRequest::ResetChannelCallback;
    }

    manager->ChannelIdle();
    channelInUse = false;

    PrintVerbose(L"Leaving CRequest::RequestComplete");
    return S_OK;
}

HRESULT CRequest::HandleFailure(
    __in HRESULT hr, 
    __inout WS_ASYNC_OPERATION* next, 
    __in_opt WS_ERROR* error)                                
{
    PrintVerbose(L"Entering CRequest::HandleFailure");   
    assert(FAILED(hr));

    CChannelManager* manager = server->GetChannelManager(); 

    if (manager->IsRunning())
    {
        WCHAR msg[100];
        hr = StringCchPrintfW(msg, CountOf(msg), L"Request failed with %x.", hr);
        if (SUCCEEDED(hr))
        {
            server->PrintInfo(msg);
        }
        else
        {
            server->PrintInfo(L"Request failed.");   
            assert(FALSE);
        }
        
        if (channelInUse)
        {
            next->function = CRequest::CloseChannelCallback;
#if (DBG || _DEBUG)
            hr = WsAbortChannel(GetChannel(), error);
            assert(SUCCEEDED(hr));
#else 
            (void)WsAbortChannel(GetChannel(), error);
#endif

        }
        else
        {
            next->function = CRequest::ResetChannelCallback;    
        }
    }
    else
    {
        if (channelInUse)
        {
            next->function = CRequest::CloseChannelCallback;
#if (DBG || _DEBUG)
            hr = WsAbortChannel(GetChannel(), error);
            assert(SUCCEEDED(hr));
#else
            (void)WsAbortChannel(GetChannel(), error);
#endif

        }
        else
        {
            next->function = NULL;    
        }
    }
  
    PrintVerbose(L"Leaving CRequest::HandleFailure");    
    return S_FALSE;
}

// This function creates a fault with a custom string and sends it back to the client.
HRESULT CRequest::SendFault(
    __in FAULT_TYPE faultType)
{
    PrintVerbose(L"Entering CRequest::SendFault");

    HRESULT hr = S_OK;
    WS_HEAP* heap = NULL;
    WS_FAULT fault;
    WS_MESSAGE* replyMessage = GetReplyMessage();
    WS_CHANNEL* channel = GetChannel();
    WS_ERROR* error = GetError();
    WS_ERROR* returnError = NULL;
    HMODULE module = NULL;

    // We cannot use the existing error here as we are filling it with custom state.
    // This error could be cached and reused, but given that errors should be rare
    // we simply destroy and recreate it.
    IfFailedExit(WsCreateError(NULL, 0, &returnError));
    
    // Get the appropriate error string.
    BOOL ret = GetModuleHandleEx(0, NULL, &module);
    if (!ret)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        EXIT_FUNCTION
    }

    WCHAR errorString[128];
    DWORD lengthInCharacters = FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, module, 
        (DWORD)faultType, 0, errorString, WsCountOf(errorString), NULL);
    if (lengthInCharacters == 0)
    {            
        hr = HRESULT_FROM_WIN32(GetLastError());
        EXIT_FUNCTION
    }

    WS_STRING string;
    string.chars = errorString;
    string.length = lengthInCharacters;
    
// lengthInChanarters is valid length of the errorString by the FormatMessageW definition.
#pragma warning(suppress:26018)
    IfFailedExit(WsAddErrorString(returnError, &string));
        
    FreeLibrary(module);
    module = NULL;

    WS_ELEMENT_DESCRIPTION elementDescription;
    ZeroMemory(&elementDescription, sizeof(elementDescription));
    elementDescription.type = WS_FAULT_TYPE;
    
    IfFailedExit(WsResetMessage(replyMessage, error));
    IfFailedExit(WsInitializeMessage(replyMessage, WS_BLANK_MESSAGE, GetRequestMessage(), error));
    IfFailedExit(WsSetHeader(
        replyMessage, 
        WS_ACTION_HEADER, 
        WS_XML_STRING_TYPE, 
        WS_WRITE_REQUIRED_VALUE, 
        &faultAction, 
        sizeof(faultAction), 
        error));
    IfFailedExit(WsGetMessageProperty(replyMessage, WS_MESSAGE_PROPERTY_HEAP, &heap, sizeof(heap), error));

    // We put it on the message heap so its cleaned up later when the heap is reset or freed.
    IfFailedExit(WsCreateFaultFromError(returnError, E_FAIL, WS_FULL_FAULT_DISCLOSURE, heap, &fault));
    IfFailedExit(WsWriteMessageStart(channel, replyMessage, NULL, error)); 
    IfFailedExit(WsWriteBody(replyMessage, &elementDescription, WS_WRITE_REQUIRED_VALUE, &fault, sizeof(fault), error));
    
    WsWriteMessageEnd(channel, replyMessage, NULL, error);

    WsFreeError(returnError);

    PrintVerbose(L"Leaving CRequest::SendFault");
    return hr;

    ERROR_EXIT

    server->PrintError(L"CRequest::SendFault", true);
    server->PrintError(hr, error, true); 

    if (NULL != module)
    {
        CloseHandle(module);
    }
    if (NULL != returnError)
    {
        WsFreeError(returnError);
    }
    if (NULL != module)
    {
        FreeLibrary(module);
    }
   
    PrintVerbose(L"Leaving CRequest::SendFault");
    return hr;
}

// The CRequest destructor marks the end of a request loop. So in order to keep the functions in
// the order they are used, this is placed here.
CRequest::~CRequest()
{
    server->PrintVerbose(L"Entering CRequest::~CRequest");    

    CleanupChannel(channel);
    
    if (NULL != requestMessage)
    {
        WsFreeMessage(requestMessage);
    }

    if (NULL != replyMessage)
    {
        WsFreeMessage(replyMessage);
    }

    if (NULL != error)
    {
        WsFreeError(error);
    }

    if (NULL != channel)
    {
        server->GetChannelManager()->ChannelFreed();
    }

    server->PrintVerbose(L"Leaving CRequest::~CRequest");
}



