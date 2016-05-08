//////////////////////////////////////////////////////////////////////////
//
// trace.h : Functions to return the names of constants.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "logging.h"
#include <mfidl.h>

#define NAME(x) case x: return L#x

namespace MediaFoundationSamples
{

    // IMPORTANT: No function here can return a NULL pointer - caller assumes
    // the return value is a valid null-terminated string. You should only
    // use these functions for debugging purposes.

    // Media Foundation event names (subset)
    inline const WCHAR* EventName(MediaEventType met)
    {
        switch (met)
        {
            NAME(MEError);
            NAME(MEExtendedType);
            NAME(MESessionTopologySet);
            NAME(MESessionTopologiesCleared);
            NAME(MESessionStarted);
            NAME(MESessionPaused);
            NAME(MESessionStopped);
            NAME(MESessionClosed);
            NAME(MESessionEnded);
            NAME(MESessionRateChanged);
            NAME(MESessionScrubSampleComplete);
            NAME(MESessionCapabilitiesChanged);
            NAME(MESessionTopologyStatus);
            NAME(MESessionNotifyPresentationTime);
            NAME(MENewPresentation);
            NAME(MELicenseAcquisitionStart);
            NAME(MELicenseAcquisitionCompleted);
            NAME(MEIndividualizationStart);
            NAME(MEIndividualizationCompleted);
            NAME(MEEnablerProgress);
            NAME(MEEnablerCompleted);
            NAME(MEPolicyError);
            NAME(MEPolicyReport);
            NAME(MEBufferingStarted);
            NAME(MEBufferingStopped);
            NAME(MEConnectStart);
            NAME(MEConnectEnd);
            NAME(MEReconnectStart);
            NAME(MEReconnectEnd);
            NAME(MERendererEvent);
            NAME(MESessionStreamSinkFormatChanged);
            NAME(MESourceStarted);
            NAME(MEStreamStarted);
            NAME(MESourceSeeked);
            NAME(MEStreamSeeked);
            NAME(MENewStream);
            NAME(MEUpdatedStream);
            NAME(MESourceStopped);
            NAME(MEStreamStopped);
            NAME(MESourcePaused);
            NAME(MEStreamPaused);
            NAME(MEEndOfPresentation);
            NAME(MEEndOfStream);
            NAME(MEMediaSample);
            NAME(MEStreamTick);
            NAME(MEStreamThinMode);
            NAME(MEStreamFormatChanged);
            NAME(MESourceRateChanged);
            NAME(MEEndOfPresentationSegment);
            NAME(MESourceCharacteristicsChanged);
            NAME(MESourceRateChangeRequested);
            NAME(MESourceMetadataChanged);
            NAME(MESequencerSourceTopologyUpdated);
            NAME(MEStreamSinkStarted);
            NAME(MEStreamSinkStopped);
            NAME(MEStreamSinkPaused);
            NAME(MEStreamSinkRateChanged);
            NAME(MEStreamSinkRequestSample);
            NAME(MEStreamSinkMarker);
            NAME(MEStreamSinkPrerolled);
            NAME(MEStreamSinkScrubSampleComplete);
            NAME(MEStreamSinkFormatChanged);
            NAME(MEStreamSinkDeviceChanged);
            NAME(MEQualityNotify);
            NAME(MESinkInvalidated);
            NAME(MEAudioSessionNameChanged);
            NAME(MEAudioSessionVolumeChanged);
            NAME(MEAudioSessionDeviceRemoved);
            NAME(MEAudioSessionServerShutdown);
            NAME(MEAudioSessionGroupingParamChanged);
            NAME(MEAudioSessionIconChanged);
            NAME(MEAudioSessionFormatChanged);
            NAME(MEAudioSessionDisconnected);
            NAME(MEAudioSessionExclusiveModeOverride);
            NAME(MEPolicyChanged);
            NAME(MEContentProtectionMessage);
            NAME(MEPolicySet);

        default:
            return L"Unknown event";
        }
    }

    // Names of VARIANT data types. 
    inline const WCHAR* VariantTypeName(const PROPVARIANT& prop)
    {
        switch (prop.vt & VT_TYPEMASK)
        {
            NAME(VT_EMPTY);
            NAME(VT_NULL);
            NAME(VT_I2);
            NAME(VT_I4);
            NAME(VT_R4);
            NAME(VT_R8);
            NAME(VT_CY);
            NAME(VT_DATE);
            NAME(VT_BSTR);
            NAME(VT_DISPATCH);
            NAME(VT_ERROR);
            NAME(VT_BOOL);
            NAME(VT_VARIANT);
            NAME(VT_UNKNOWN);
            NAME(VT_DECIMAL);
            NAME(VT_I1);
            NAME(VT_UI1);
            NAME(VT_UI2);
            NAME(VT_UI4);
            NAME(VT_I8);
            NAME(VT_UI8);
            NAME(VT_INT);
            NAME(VT_UINT);
            NAME(VT_VOID);
            NAME(VT_HRESULT);
            NAME(VT_PTR);
            NAME(VT_SAFEARRAY);
            NAME(VT_CARRAY);
            NAME(VT_USERDEFINED);
            NAME(VT_LPSTR);
            NAME(VT_LPWSTR);
            NAME(VT_RECORD);
            NAME(VT_INT_PTR);
            NAME(VT_UINT_PTR);
            NAME(VT_FILETIME);
            NAME(VT_BLOB);
            NAME(VT_STREAM);
            NAME(VT_STORAGE);
            NAME(VT_STREAMED_OBJECT);
            NAME(VT_STORED_OBJECT);
            NAME(VT_BLOB_OBJECT);
            NAME(VT_CF);
            NAME(VT_CLSID);
            NAME(VT_VERSIONED_STREAM);
        default:
            return L"Unknown VARIANT type";
        }
    }

    // Names of topology node types.
    inline const WCHAR* TopologyNodeTypeName(MF_TOPOLOGY_TYPE nodeType)
    {
        switch (nodeType)
        {
            NAME(MF_TOPOLOGY_OUTPUT_NODE);
            NAME(MF_TOPOLOGY_SOURCESTREAM_NODE);
            NAME(MF_TOPOLOGY_TRANSFORM_NODE);
            NAME(MF_TOPOLOGY_TEE_NODE);
        default:
            return L"Unknown node type";
        }
    }

    inline const WCHAR* MFTMessageName(MFT_MESSAGE_TYPE msg)
    {
        switch (msg)
        {
            NAME(MFT_MESSAGE_COMMAND_FLUSH);
            NAME(MFT_MESSAGE_COMMAND_DRAIN);
            NAME(MFT_MESSAGE_SET_D3D_MANAGER);
            NAME(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING);
            NAME(MFT_MESSAGE_NOTIFY_END_STREAMING);
            NAME(MFT_MESSAGE_NOTIFY_END_OF_STREAM);
            NAME(MFT_MESSAGE_NOTIFY_START_OF_STREAM);
        default:
            return L"Unknown message";
        }
    }

}; // namespace MediaFoundationSamples
