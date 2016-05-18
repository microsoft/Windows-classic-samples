//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#include "Service.h"
#include "assert.h"

CChannelManager::CChannelManager(
    __in CFileRep* server, 
    __in long minIdleChannels,
    __in long maxIdleChannels, 
    __in long maxTotalChannels)
{    
    this->minIdleChannels = minIdleChannels;
    this->maxIdleChannels = maxIdleChannels;
    this->maxTotalChannels = maxTotalChannels;

    assert(this->maxIdleChannels >= minIdleChannels);
    assert(this->minIdleChannels <= maxTotalChannels);
    assert(NULL != server);
        
    idleChannels = 0;
    activeChannels = 0;
    totalChannels = 0;
    this->server = server;

    stopEvent = NULL;
    running = true;
#if (DBG || _DEBUG)
    initialized = false;
#endif
}

HRESULT CChannelManager::Initialize()
{
    HRESULT hr = S_OK;

    stopEvent = CreateEvent(NULL, true, false, NULL); 

    if (NULL == stopEvent)
    {
        server->PrintError(L"CChannelManager::Initialize", true);
        server->PrintError(L"Initialization of event failed. Aborting service startup.", true);

        hr = HRESULT_FROM_WIN32(GetLastError());
    }
#if (DBG || _DEBUG)
    else
    {
        initialized = true;
    }
#endif

    return hr;
}

CChannelManager::~CChannelManager()
{
    // At this point, no outstanding channels should be left.
    assert(0 == idleChannels);
    assert(0 == activeChannels);
    assert(0 == totalChannels);

    if (NULL != stopEvent)
    {
        CloseHandle(stopEvent);
        stopEvent = NULL;
    }   
}

void CChannelManager::ChannelCreated() 
{
    InterlockedIncrement(&idleChannels);
    InterlockedIncrement(&totalChannels);
}

// Channel is processing a request.
void CChannelManager::ChannelInUse() 
{
    InterlockedIncrement(&activeChannels); 

    assert(idleChannels > 0);
    InterlockedDecrement(&idleChannels);    

    // See if we fell below the threshold for available channels.
    // Ignore return value as the failure to create a new channel should not impact the existing channel.
    (void) CreateChannels();
}

// Channel and associated data structures are freed.
void CChannelManager::ChannelFreed() 
{
    assert(idleChannels > 0);
    InterlockedDecrement(&idleChannels);    

    assert(totalChannels > 0);
    long totalChannelsChannelFreed = InterlockedDecrement(&totalChannels);
    
    if (0 == totalChannelsChannelFreed)
    {
        // We only destroy superfluous channels so it should never hit 0
        // unless we are shutting down.
        assert(!IsRunning());
#if (DBG || _DEBUG)
        BOOL eventReturn = 
#endif
        SetEvent(stopEvent);
        assert(eventReturn);
    }
}

// Channel is done processing a request and is ready to accept more work.
void CChannelManager::ChannelIdle() 
{
    assert(activeChannels > 0);
    InterlockedDecrement(&activeChannels);     

    InterlockedIncrement(&idleChannels);    
}

// Creates new channels if we are below the threshold for minumum available channels and if we are not at the channel cap.
HRESULT CChannelManager::CreateChannels()
{
    CRequest* request = NULL;
    HRESULT hr = S_OK;
    server->PrintVerbose(L"Entering CChannelManager::CreateChannels");
  
    if (idleChannels >= minIdleChannels || idleChannels + activeChannels >= maxTotalChannels)
    {
        server->PrintVerbose(L"Leaving CChannelManager::CreateChannels");
        return S_OK;
    }

    long newChannels = minIdleChannels - idleChannels;
    if (newChannels > maxTotalChannels - idleChannels - activeChannels)
    {
        newChannels = maxTotalChannels - idleChannels - activeChannels;
    }

    for (long i = 0; i < newChannels; i++)
    {
        // Even though our main request processing loop is asynchronous, there is enough
        // synchronous work done (eg state creartion) to warrant farming this out to work items. Also, 
        // WsAsyncExecute can return synchronously if the request ends before the first asynchronous
        // function is called and in that case we dont want to get stuck here by doing this synchronously.

        // This is done here to prevent a race condition. If QueueUserWorkItem fails during startup the
        // service will get torn down. But there could be other work items out there waiting to be executed.
        // So to make sure the shutdown waits until those have been scheduled we increment the channel count here.
        
        if (!IsRunning())
        {
            break;
        }

        // This object contains all request-specific state and the common message processing methods.
#pragma prefast(suppress: 28197)
        request = new CRequest(server);
        IfNullExit(request);

        // Preallocate as much state as possible.
        if (FAILED(request->Initialize()))
        {
            delete request;
            EXIT_FUNCTION
        }

        ChannelCreated();        
        if (!::QueueUserWorkItem(CChannelManager::CreateChannelWorkItem, request, WT_EXECUTELONGFUNCTION))
        {
            // If this fails we are in bad shape, so don't try again.
            hr = HRESULT_FROM_WIN32(GetLastError());
            server->PrintError(L"CChannelManager::CreateChannels", true);
            server->PrintError(hr, NULL, true); 
            delete request;
            
            break;
        }
    }

    EXIT

    server->PrintVerbose(L"Leaving CChannelManager::CreateChannels");
    return hr;
}

ULONG WINAPI CChannelManager::CreateChannelWorkItem(
    __in void* state)
{
    assert(NULL != state);

    CRequest* request = (CRequest*) state;  

    request->GetServer()->GetChannelManager()->CreateChannel(request);

    return 0;
}

// A channel goes through a series of states involving the channel manager.
// It starts its life here. We call into CFileRep::CreateOrResetChannel to create the actual channel state.
// After creation, the channel waits for an incoming request and once it gets one processes it.
// Once it is done, we call back into the channel manager which checks if the service is still running
// and if the channel is still needed. That happens in CChannelManager::RequestComplete 
// If it is not needed, the channel and related data structures are freed.
// If it is still needed, the loop repeats as we call into CFileRep::CreateOrResetChannel again. 
// Only this time it reuses the channel data structures instead of creating them.
void CChannelManager::CreateChannel(
    __in CRequest* request)
{
    PrintVerbose(L"Entering CChannelManager::CreateChannel");

    WS_ERROR* error = request->GetError();

    WS_ASYNC_CONTEXT asyncContext;
    HRESULT hr = S_OK;

    if (!IsRunning())
    {
        PrintVerbose(L"Leaving CChannelManager::CreateChannel");
        return;
    }
           
    // We do nothing in the callback
    asyncContext.callback = CleanupCallback;  
    asyncContext.callbackState = request;

    // Start the message processing loop asynchronously. 
    // Use long callbacks since we are going to do significant work in there.
    IfFailedExit(WsAsyncExecute(&request->asyncState, CRequest::AcceptChannelCallback, WS_LONG_CALLBACK,
        request, &asyncContext, error));   
    
    // In the sync case, the cleanup callback is never called so we have to do it here.
    // We only get here after we are done with the channel for good so it is safe to do this.
    if (WS_S_ASYNC != hr)
    {
        delete request;
    }
    
    PrintVerbose(L"Leaving CChannelManager::CreateChannel");
    return;

    ERROR_EXIT

    server->PrintError(L"CChannelManager::CreateChannel", true);
    server->PrintError(hr, error, true); 
    if (NULL != request)
    {
        // Cleans up all the state associated with a request.
        delete request;
    }  

    PrintVerbose(L"Leaving CChannelManager::CreateChannel");
}

#pragma warning(disable : 4100) // The callbacks doesn't use all parameters.

// This is called at the end of an async execution chain. Here we can clean up.
void CALLBACK CChannelManager::CleanupCallback(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state)
{
    assert(NULL != state);

    CRequest* request = (CRequest*) state;  
    delete request;
}

#pragma warning(default : 4100)

// Sets state to stopped, which prevents new requests from being processed.
void CChannelManager::Stop()
{
    server->PrintVerbose(L"Entering CChannelManager::Stop");
    
    assert(initialized);

    // This is a special case. As we never destroy all channels, there should
    // only be zero channels before shutdown if the creation of all channels failed.
    // In that case we stop here as the regular stop will not be hit.
    if (0 == totalChannels)
    {
#if (DBG || _DEBUG)
        BOOL eventReturn = 
#endif
        SetEvent(stopEvent);
        assert(eventReturn);                
    }

    running = false;

    server->PrintVerbose(L"Leaving CChannelManager::Stop");
}

// Wait for all outstanding requests to drain.
void CChannelManager::WaitForCleanup()
{
    server->PrintVerbose(L"Entering CChannelManager::WaitForCleanup");
    
    assert(!IsRunning());
    assert(initialized);

    WaitForSingleObject(stopEvent, INFINITE);

    server->PrintVerbose(L"Leaving CChannelManager::WaitForCleanup");    
}
