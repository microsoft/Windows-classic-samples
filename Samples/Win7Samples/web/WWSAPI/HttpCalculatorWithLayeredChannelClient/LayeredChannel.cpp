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

HRESULT CALLBACK CustomCreateChannel(
    __in WS_CHANNEL_TYPE channelType, 
    __in_bcount(channelParametersSize) const void* channelParameters, 
    __in ULONG channelParametersSize, 
    __deref_out void** channelInstance, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr;
    CustomChannel* customChannel = NULL;

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
    hr = WsCreateChannel(
        channelType,
        layeredChannelParameters->channelBinding,
        layeredChannelParameters->channelProperties,
        layeredChannelParameters->channelPropertyCount,
        layeredChannelParameters->securityDescription,
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

void CALLBACK CustomFreeChannel(
    __in void* channelInstance)
{
    CustomChannel* customChannel = (CustomChannel*)channelInstance;

    // Free the underlying channel
    WsFreeChannel(customChannel->channel);

    // Free the instance that was allocated by CustomCreateChannel
    HeapFree(GetProcessHeap(), 0, channelInstance);
}

HRESULT CALLBACK CustomResetChannel(
    __in void* channelInstance, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsResetChannel(customChannel->channel, error);
}

HRESULT CALLBACK CustomAbortChannel(
    __in void* channelInstance, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAbortChannel(customChannel->channel, error);
}

HRESULT CALLBACK CustomOpenChannel(
    __in void* channelInstance, 
    __in const WS_ENDPOINT_ADDRESS* endpointAddress, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsOpenChannel(customChannel->channel, endpointAddress, asyncContext, error);
}

HRESULT CALLBACK CustomCloseChannel(
    __in void* channelInstance, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsCloseChannel(customChannel->channel, asyncContext, error);
}

HRESULT CALLBACK CustomSetChannelProperty(
    __in void* channelInstance, 
    __in WS_CHANNEL_PROPERTY_ID id, 
    __in_bcount(valueSize) const void* value, 
    __in ULONG valueSize, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsSetChannelProperty(customChannel->channel, id, value, valueSize, error);
}

HRESULT CALLBACK CustomGetChannelProperty(
    __in void* channelInstance, 
    __in WS_CHANNEL_PROPERTY_ID id, 
    __out_bcount(valueSize) void* value, 
    __in ULONG valueSize, 
    __in_opt WS_ERROR* error)
{
    CustomChannel* customChannel = (CustomChannel*)channelInstance;

    // Underlying channels do not support querying WS_CHANNEL_PROPERTY_ENABLE_TIMEOUTS.
    // Custom channel keeps track of whether timeouts were disabled and returns here.
    // Service Model queries this property to ensure the timeouts were disabled in the custom channel.
    if (WS_CHANNEL_PROPERTY_ENABLE_TIMEOUTS == id)
    {
        if (sizeof(BOOL) != valueSize)
        {
            return E_INVALIDARG;
        }

        *(BOOL*)value = customChannel->disabledTimeouts;

        return S_OK;
    }

    // Delegate the rest of the property queries to the underlying channel
    return WsGetChannelProperty(customChannel->channel, id, value, valueSize, error);
}

HRESULT CALLBACK CustomReadMessageStart(
    __in void* channelInstance, 
    __in WS_MESSAGE* message, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsReadMessageStart(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomReadMessageEnd(
    __in void* channelInstance, 
    __in WS_MESSAGE* message, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsReadMessageEnd(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomWriteMessageStart(
    __in void* channelInstance, 
    __in WS_MESSAGE* message, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsWriteMessageStart(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomWriteMessageEnd(
    __in void* channelInstance, 
    __in WS_MESSAGE* message, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsWriteMessageEnd(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomAbandonMessage(
    __in void* channelInstance, 
    __in WS_MESSAGE* message, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAbandonMessage(customChannel->channel, message, error);
}

HRESULT CALLBACK CustomShutdownSessionChannel(
    __in void* channelInstance, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsShutdownSessionChannel(customChannel->channel, asyncContext, error);
}

// Initialize the callbacks that will implement the custom channel
WS_CUSTOM_CHANNEL_CALLBACKS layeredChannelCallbacks =
{
    &CustomCreateChannel, 
    &CustomFreeChannel, 
    &CustomResetChannel, 
    &CustomOpenChannel, 
    &CustomCloseChannel, 
    &CustomAbortChannel, 
    &CustomGetChannelProperty, 
    &CustomSetChannelProperty, 
    &CustomWriteMessageStart, 
    &CustomWriteMessageEnd, 
    &CustomReadMessageStart, 
    &CustomReadMessageEnd, 
    &CustomAbandonMessage, 
    &CustomShutdownSessionChannel,
};
