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
#include "LayeredChannel.h"
#include "LayeredListener.h"

HRESULT CALLBACK CustomCreateListener(
    __in WS_CHANNEL_TYPE channelType, 
    __in_bcount(listenerParametersSize) const void* listenerParameters, 
    __in ULONG listenerParametersSize, 
    __deref_out void** listenerInstance, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr;
    CustomListener* customListener = NULL;

    // Get the parameters passed via WS_LISTENER_PROPERTY_CUSTOM_CHANNEL_PARAMETERS
    if (listenerParametersSize != sizeof(LayeredListenerParameters))
    {
        return E_INVALIDARG;
    }
    LayeredListenerParameters* layeredListenerParameters = (LayeredListenerParameters*)listenerParameters;

    // Allocate the custom listener instance
    customListener = (CustomListener*)HeapAlloc(GetProcessHeap(), 0, sizeof(CustomListener));
    if (customListener == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    // Create the underlying listener using the passed in parameters
    hr = WsCreateListener(
        channelType,
        layeredListenerParameters->channelBinding,
        layeredListenerParameters->listenerProperties,
        layeredListenerParameters->listenerPropertyCount,
        layeredListenerParameters->securityDescription,
        &customListener->listener,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

    // Return the listener instance.  The instance
    // will be freed by the CustomFreeListener function.
    *listenerInstance = customListener;
    customListener = NULL;
    hr = S_OK;

Exit:
    if (customListener != NULL)
    {
        HeapFree(GetProcessHeap(), 0, customListener);
    }

    return hr;  
}

void CALLBACK CustomFreeListener(
    __in void* listenerInstance)
{
    CustomListener* customListener = (CustomListener*)listenerInstance;

    // Free the underlying listener
    WsFreeListener(customListener->listener);

    // Free the instance that was allocated by CustomCreateListener
    HeapFree(GetProcessHeap(), 0, listenerInstance);
}

HRESULT CALLBACK CustomResetListener(
    __in void* listenerInstance, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsResetListener(customListener->listener, error);
}

HRESULT CALLBACK CustomOpenListener(
    __in void* listenerInstance, 
    __in const WS_STRING* url, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsOpenListener(customListener->listener, url, asyncContext, error);
}

HRESULT CALLBACK CustomCloseListener(
    __in void* listenerInstance, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsCloseListener(customListener->listener, asyncContext, error);
}

HRESULT CALLBACK CustomAbortListener(
    __in void* listenerInstance, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsAbortListener(customListener->listener, error);
}

HRESULT CALLBACK CustomGetListenerProperty(
    __in void* listenerInstance, 
    __in WS_LISTENER_PROPERTY_ID id, 
    __out_bcount(valueSize) void* value, 
    __in ULONG valueSize, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsGetListenerProperty(customListener->listener, id, value, valueSize, error);
}

HRESULT CALLBACK CustomSetListenerProperty(
    __in void* listenerInstance, 
    __in WS_LISTENER_PROPERTY_ID id, 
    __in_bcount(valueSize) const void* value, 
    __in ULONG valueSize, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsSetListenerProperty(customListener->listener, id, value, valueSize, error);
}

HRESULT CALLBACK CustomAcceptChannel(
    __in void* listenerInstance, 
    __in void* channelInstance, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAcceptChannel(customListener->listener, customChannel->channel, asyncContext, error);
}

HRESULT CALLBACK CustomCreateChannelForListener(
    __in void* listenerInstance, 
    __in_bcount(channelParametersSize) const void* channelParameters, 
    __in ULONG channelParametersSize, 
    __deref_out void** channelInstance, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr;
    CustomChannel* customChannel = NULL;
    CustomListener* customListener = (CustomListener*)listenerInstance;

    // Get the parameters passed via WS_CHANNEL_PROPERTY_CUSTOM_CHANNEL_PARAMETERS
    if (channelParametersSize != sizeof(LayeredChannelParameters))
    {
        return E_INVALIDARG;
    }
    LayeredChannelParameters* layeredChannelParameters = (LayeredChannelParameters*)channelParameters;

    // Allocate the custom channel instance
    customChannel = (CustomChannel*)HeapAlloc(GetProcessHeap(), 0, sizeof(CustomChannel));
    if (customChannel == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    // Create the underlying channel using the passed in parameters
    hr = WsCreateChannelForListener(
        customListener->listener,
        layeredChannelParameters->channelProperties,
        layeredChannelParameters->channelPropertyCount,
        &customChannel->channel,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

    // we need to keep track of whether timeouts were disabled to be able to serve
    // the WsGetChannelProperty calls later on.
    for (ULONG i = 0; i < layeredChannelParameters->channelPropertyCount; i ++)
    {
        if (WS_CHANNEL_PROPERTY_ENABLE_TIMEOUTS == layeredChannelParameters->channelProperties[i].id)
        {
            customChannel->disabledTimeouts = *(BOOL*)layeredChannelParameters->channelProperties[i].value;
            break;
        }
    }

    // Return the channel instance.  The instance
    // will be freed by the CustomFreeChannel function.
    *channelInstance = customChannel;
    customChannel = NULL;
    hr = S_OK;

Exit:
    if (customChannel != NULL)
    {
        HeapFree(GetProcessHeap(), 0, customChannel);
    }

    return hr;  
}

// Initialize the callbacks that will implement the custom listener
WS_CUSTOM_LISTENER_CALLBACKS layeredListenerCallbacks =
{
    &CustomCreateListener, 
    &CustomFreeListener, 
    &CustomResetListener, 
    &CustomOpenListener, 
    &CustomCloseListener, 
    &CustomAbortListener, 
    &CustomGetListenerProperty, 
    &CustomSetListenerProperty, 
    &CustomCreateChannelForListener, 
    &CustomAcceptChannel, 
};
