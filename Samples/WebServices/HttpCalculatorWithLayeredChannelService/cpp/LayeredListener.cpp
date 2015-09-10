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
#include "LayeredListener.h"

HRESULT CALLBACK CustomCreateListener(
    _In_ WS_CHANNEL_TYPE channelType,
    _In_reads_bytes_(listenerParametersSize) const void* listenerParameters,
    _In_ ULONG listenerParametersSize,
    _Outptr_ void** listenerInstance,
    _In_opt_ WS_ERROR* error)
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
    _In_ void* listenerInstance)
{
    CustomListener* customListener = (CustomListener*)listenerInstance;

    // Free the underlying listener
    WsFreeListener(customListener->listener);

    // Free the instance that was allocated by CustomCreateListener
    HeapFree(GetProcessHeap(), 0, listenerInstance);
}

HRESULT CALLBACK CustomResetListener(
    _In_ void* listenerInstance,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsResetListener(customListener->listener, error);
}

HRESULT CALLBACK CustomOpenListener(
    _In_ void* listenerInstance,
    _In_ const WS_STRING* url,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsOpenListener(customListener->listener, url, asyncContext, error);
}

HRESULT CALLBACK CustomCloseListener(
    _In_ void* listenerInstance,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsCloseListener(customListener->listener, asyncContext, error);
}

HRESULT CALLBACK CustomAbortListener(
    _In_ void* listenerInstance,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsAbortListener(customListener->listener, error);
}

HRESULT CALLBACK CustomGetListenerProperty(
    _In_ void* listenerInstance,
    _In_ WS_LISTENER_PROPERTY_ID id,
    _Out_writes_bytes_(valueSize) void* value,
    _In_ ULONG valueSize,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsGetListenerProperty(customListener->listener, id, value, valueSize, error);
}

HRESULT CALLBACK CustomSetListenerProperty(
    _In_ void* listenerInstance,
    _In_ WS_LISTENER_PROPERTY_ID id,
    _In_reads_bytes_(valueSize) const void* value,
    _In_ ULONG valueSize,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    return WsSetListenerProperty(customListener->listener, id, value, valueSize, error);
}

HRESULT CALLBACK CustomAcceptChannel(
    _In_ void* listenerInstance,
    _In_ void* channelInstance,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    // Delegate to the underlying listener
    CustomListener* customListener = (CustomListener*)listenerInstance;
    CustomChannel* customChannel = (CustomChannel*)channelInstance;
    return WsAcceptChannel(customListener->listener, customChannel->channel, asyncContext, error);
}

HRESULT CALLBACK CustomCreateChannelForListener(
    _In_ void* listenerInstance,
    _In_reads_bytes_(channelParametersSize) const void* channelParameters,
    _In_ ULONG channelParametersSize,
    _Outptr_ void** channelInstance,
    _In_opt_ WS_ERROR* error)
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
