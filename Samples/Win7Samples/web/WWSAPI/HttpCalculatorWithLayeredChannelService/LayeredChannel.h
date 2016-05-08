//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// A structure containing parameters passed to the custom channel 
// using WS_CHANNEL_PROPERTY_CUSTOM_CHANNEL_PARAMETERS.
struct LayeredChannelParameters
{
    // The type of the underlying channel
    WS_CHANNEL_BINDING channelBinding;

    // Channel properties to pass to the underlying channel
    WS_CHANNEL_PROPERTY* channelProperties;
    ULONG channelPropertyCount;

    // Security settings for the underlying channel
    WS_SECURITY_DESCRIPTION* securityDescription;
};

// The structure containing instance state for the custom channel
struct CustomChannel
{
    // Underlying channel handle
    WS_CHANNEL* channel;
    BOOL disabledTimeouts;
};

// The set of callbacks that make up the custom channel implementation.
extern WS_CUSTOM_CHANNEL_CALLBACKS layeredChannelCallbacks;
