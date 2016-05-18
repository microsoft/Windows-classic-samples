//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// A structure containing parameters passed to the custom listener
// using WS_LISTENER_PROPERTY_CUSTOM_LISTENER_PARAMETERS.
struct LayeredListenerParameters
{
    // The type of the underlying channel
    WS_CHANNEL_BINDING channelBinding;

    // Listener properties to pass to the underlying listener
    WS_LISTENER_PROPERTY* listenerProperties;
    ULONG listenerPropertyCount;

    // Security settings for the underlying listener
    WS_SECURITY_DESCRIPTION* securityDescription;
};

// The structure containing instance state for the custom listener
struct CustomListener
{
    // Underlying listener handle
    WS_LISTENER* listener;
};

// The set of callbacks that make up the custom listener implementation.
extern WS_CUSTOM_LISTENER_CALLBACKS layeredListenerCallbacks;

