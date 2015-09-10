// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "LayeredChannel.h"

HRESULT CALLBACK CustomCreateChannel(
    _In_ WS_CHANNEL_TYPE channelType,
    _In_reads_bytes_(channelParametersSize) const void* channelParameters,
    _In_ ULONG channelParametersSize,
    _Outptr_ void** channelInstance,
    _In_opt_ WS_ERROR* error)
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
    _In_ void* channelInstance)
{
    CustomChannel* customChannel = (CustomChannel*)channelInstance;

    // Free the underlying channel
    WsFreeChannel(customChannel->channel);

    // Free the instance that was allocated by CustomCreateChannel
    HeapFree(GetProcessHeap(), 0, channelInstance);
}

HRESULT CALLBACK CustomResetChannel(
    _In_ void* channelInstance,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsResetChannel(customChannel->channel, error);
}

HRESULT CALLBACK CustomAbortChannel(
    _In_ void* channelInstance,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAbortChannel(customChannel->channel, error);
}

HRESULT CALLBACK CustomOpenChannel(
    _In_ void* channelInstance,
    _In_ const WS_ENDPOINT_ADDRESS* endpointAddress,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsOpenChannel(customChannel->channel, endpointAddress, asyncContext, error);
}

HRESULT CALLBACK CustomCloseChannel(
    _In_ void* channelInstance,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsCloseChannel(customChannel->channel, asyncContext, error);
}

HRESULT CALLBACK CustomSetChannelProperty(
    _In_ void* channelInstance,
    _In_ WS_CHANNEL_PROPERTY_ID id,
    _In_reads_bytes_(valueSize) const void* value,
    _In_ ULONG valueSize,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsSetChannelProperty(customChannel->channel, id, value, valueSize, error);
}

HRESULT CALLBACK CustomGetChannelProperty(
    _In_ void* channelInstance,
    _In_ WS_CHANNEL_PROPERTY_ID id,
    _Out_writes_bytes_(valueSize) void* value,
    _In_ ULONG valueSize,
    _In_opt_ WS_ERROR* error)
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
    _In_ void* channelInstance,
    _In_ WS_MESSAGE* message,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsReadMessageStart(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomReadMessageEnd(
    _In_ void* channelInstance,
    _In_ WS_MESSAGE* message,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsReadMessageEnd(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomWriteMessageStart(
    _In_ void* channelInstance,
    _In_ WS_MESSAGE* message,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsWriteMessageStart(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomWriteMessageEnd(
    _In_ void* channelInstance,
    _In_ WS_MESSAGE* message,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsWriteMessageEnd(customChannel->channel, message, asyncContext, error);
}

HRESULT CALLBACK CustomAbandonMessage(
    _In_ void* channelInstance,
    _In_ WS_MESSAGE* message,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying channel
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAbandonMessage(customChannel->channel, message, error);
}

HRESULT CALLBACK CustomShutdownSessionChannel(
    _In_ void* channelInstance,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
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
