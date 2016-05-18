// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "propertyview.h"

#include <assert.h>

#include "commctrl.h"
#include "wmcontainer.h"
#include "atlconv.h"
#include "nserror.h"

KeyStringTypeTriplet CPropertyInfo::ms_AttributeKeyStrings[] = {
    { MF_TOPONODE_CONNECT_METHOD, L"MF_TOPONODE_CONNECT_METHOD", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_DECODER, L"MF_TOPONODE_DECODER", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_DECRYPTOR, L"MF_TOPONODE_DECRYPTOR", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_DISCARDABLE, L"MF_TOPONODE_DISCARDABLE", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_ERROR_MAJORTYPE, L"MF_TOPONODE_ERROR_MAJORTYPE", VT_CLSID, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_ERROR_SUBTYPE, L"MF_TOPONODE_ERROR_SUBTYPE", VT_CLSID, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_ERRORCODE, L"MF_TOPONODE_ERRORCODE", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_LOCKED, L"MF_TOPONODE_LOCKED", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_MARKIN_HERE, L"MF_TOPONODE_MARKIN_HERE", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_MARKOUT_HERE, L"MF_TOPONODE_MARKOUT_HERE", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_MEDIASTART, L"MF_TOPONODE_MEDIASTART", VT_UI8, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_MEDIASTOP, L"MF_TOPONODE_MEDIASTOP", VT_UI8, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_PRESENTATION_DESCRIPTOR, L"MF_TOPONODE_PRESENTATION_DESCRIPTOR", VT_UNKNOWN, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_SEQUENCE_ELEMENTID, L"MF_TOPONODE_SEQUENCE_ELEMENTID", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_SOURCE, L"MF_TOPONODE_SOURCE", VT_UNKNOWN, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_STREAM_DESCRIPTOR, L"MF_TOPONODE_STREAM_DESCRIPTOR", VT_UNKNOWN, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_WORKQUEUE_ID, L"MF_TOPONODE_WORKQUEUE_ID", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_WORKQUEUE_MMCSS_CLASS, L"MF_TOPONODE_WORKQUEUE_MMCSS_CLASS", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_WORKQUEUE_MMCSS_TASKID, L"MF_TOPONODE_WORKQUEUE_MMCSS_TASKID", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_D3DAWARE, L"MF_TOPONODE_D3DAWARE", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_DRAIN, L"MF_TOPONODE_DRAIN", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_FLUSH, L"MF_TOPONODE_FLUSH", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_TRANSFORM_OBJECTID, L"MF_TOPONODE_TRANSFORM_OBJECTID", VT_CLSID, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_DISABLE_PREROLL, L"MF_TOPONODE_DISABLE_PREROLL", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, L"MF_TOPONODE_NOSHUTDOWN_ON_REMOVE", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_RATELESS, L"MF_TOPONODE_RATELESS", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_STREAMID, L"MF_TOPONODE_STREAMID", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_TOPONODE_PRIMARYOUTPUT, L"MF_TOPONODE_PRIMARYOUTPUT", VT_UI4, TED_ATTRIBUTE_CATEGORY_TOPONODE },
    { MF_MT_ALL_SAMPLES_INDEPENDENT, L"MF_MT_ALL_SAMPLES_INDEPENDENT", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AM_FORMAT_TYPE, L"MF_MT_AM_FORMAT_TYPE", VT_CLSID, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_COMPRESSED, L"MF_MT_COMPRESSED", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_FIXED_SIZE_SAMPLES, L"MF_MT_FIXED_SIZE_SAMPLES", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MAJOR_TYPE, L"MF_MT_MAJOR_TYPE", VT_CLSID, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_SAMPLE_SIZE, L"MF_MT_SAMPLE_SIZE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_SUBTYPE, L"MF_MT_SUBTYPE", VT_CLSID, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_USER_DATA, L"MF_MT_USER_DATA", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_WRAPPED_TYPE, L"MF_MT_WRAPPED_TYPE", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_AVG_BYTES_PER_SECOND, L"MF_MT_AUDIO_AVG_BYTES_PER_SECOND", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_BITS_PER_SAMPLE, L"MF_MT_AUDIO_BITS_PER_SAMPLE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_BLOCK_ALIGNMENT, L"MF_MT_AUDIO_BLOCK_ALIGNMENT", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_CHANNEL_MASK, L"MF_MT_AUDIO_CHANNEL_MASK", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, L"MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND", VT_R8, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_FOLDDOWN_MATRIX, L"MF_MT_AUDIO_FOLDDOWN_MATRIX", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_NUM_CHANNELS, L"MF_MT_AUDIO_NUM_CHANNELS", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_PREFER_WAVEFORMATEX, L"MF_MT_AUDIO_PREFER_WAVEFORMATEX", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_SAMPLES_PER_BLOCK, L"MF_MT_AUDIO_SAMPLES_PER_BLOCK", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_SAMPLES_PER_SECOND, L"MF_MT_AUDIO_SAMPLES_PER_SECOND", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, L"MF_MT_AUDIO_VALID_BITS_PER_SAMPLE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AVG_BIT_ERROR_RATE, L"MF_MT_AVG_BIT_ERROR_RATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AVG_BITRATE, L"MF_MT_AVG_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_CUSTOM_VIDEO_PRIMARIES, L"MF_MT_CUSTOM_VIDEO_PRIMARIES", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_DEFAULT_STRIDE, L"MF_MT_DEFAULT_STRIDE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_DRM_FLAGS, L"MF_MT_DRM_FLAGS", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_FRAME_RATE, L"MF_MT_FRAME_RATE", VT_UI8, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_FRAME_SIZE, L"MF_MT_FRAME_SIZE", VT_UI8, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_GEOMETRIC_APERTURE, L"MF_MT_GEOMETRIC_APERTURE", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_INTERLACE_MODE, L"MF_MT_INTERLACE_MODE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MAX_KEYFRAME_SPACING, L"MF_MT_MAX_KEYFRAME_SPACING", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MINIMUM_DISPLAY_APERTURE, L"MF_MT_MINIMUM_DISPLAY_APERTURE", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MPEG_SEQUENCE_HEADER, L"MF_MT_MPEG_SEQUENCE_HEADER", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MPEG_START_TIME_CODE, L"MF_MT_MPEG_START_TIME_CODE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MPEG2_FLAGS, L"MF_MT_MPEG2_FLAGS", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MPEG2_LEVEL, L"MF_MT_MPEG2_LEVEL", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_MPEG2_PROFILE, L"MF_MT_MPEG2_PROFILE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_PAD_CONTROL_FLAGS, L"MF_MT_PAD_CONTROL_FLAGS", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_PALETTE, L"MF_MT_PALETTE", (VT_VECTOR | VT_UI1), TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_PAN_SCAN_APERTURE, L"MF_MT_PAN_SCAN_APERTURE", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_PAN_SCAN_ENABLED, L"MF_MT_PAN_SCAN_ENABLED", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_PIXEL_ASPECT_RATIO, L"MF_MT_PIXEL_ASPECT_RATIO", VT_UI8, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_SOURCE_CONTENT_HINT, L"MF_MT_SOURCE_CONTENT_HINT", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_TRANSFER_FUNCTION, L"MF_MT_TRANSFER_FUNCTION", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_VIDEO_CHROMA_SITING, L"MF_MT_VIDEO_CHROMA_SITING", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_VIDEO_LIGHTING, L"MF_MT_VIDEO_LIGHTING", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_VIDEO_NOMINAL_RANGE, L"MF_MT_VIDEO_NOMINAL_RANGE", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_VIDEO_PRIMARIES, L"MF_MT_VIDEO_PRIMARIES", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_YUV_MATRIX, L"MF_MT_YUV_MATRIX", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_WMADRC_PEAKREF, L"MF_MT_AUDIO_WMADRC_PEAKREF", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_MT_AUDIO_WMADRC_AVGREF, L"MF_MT_AUDIO_WMADRC_AVGREF", VT_UI4, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_SD_LANGUAGE, L"MF_SD_LANGUAGE", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_PROTECTED, L"MF_SD_PROTECTED", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_SAMI_LANGUAGE, L"MF_SD_SAMI_LANGUAGE", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_EXTSTRMPROP_AVG_BUFFERSIZE, L"MF_SD_ASF_EXTSTRMPROP_AVG_BUFFERSIZE", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_EXTSTRMPROP_AVG_DATA_BITRATE, L"MF_SD_ASF_EXTSTRMPROP_AVG_DATA_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_EXTSTRMPROP_LANGUAGE_ID_INDEX, L"MF_SD_ASF_EXTSTRMPROP_LANGUAGE_ID_INDEX", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_EXTSTRMPROP_MAX_BUFFERSIZE, L"MF_SD_ASF_EXTSTRMPROP_MAX_BUFFERSIZE", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_EXTSTRMPROP_MAX_DATA_BITRATE, L"MF_SD_ASF_EXTSTRMPROP_MAX_DATA_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_METADATA_DEVICE_CONFORMANCE_TEMPLATE, L"MF_SD_ASF_METADATA_DEVICE_CONFORMANCE_TEMPLATE", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_SD_ASF_STREAMBITRATES_BITRATE, L"MF_SD_ASF_STREAMBITRATES_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR },
    { MF_ASFSTREAMCONFIG_LEAKYBUCKET1, L"MF_ASFSTREAMCONFIG_LEAKYBUCKET1", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_ASFSTREAMCONFIG_LEAKYBUCKET2, L"MF_ASFSTREAMCONFIG_LEAKYBUCKET2", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_MEDIATYPE },
    { MF_PD_APP_CONTEXT, L"MF_PD_APP_CONTEXT", VT_UNKNOWN, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_AUDIO_ENCODING_BITRATE, L"MF_PD_AUDIO_ENCODING_BITRATE ", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_DURATION, L"MF_PD_DURATION", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_LAST_MODIFIED_TIME, L"MF_PD_LAST_MODIFIED_TIME", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR  },
    { MF_PD_MIME_TYPE, L"MF_PD_MIME_TYPE", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_PMPHOST_CONTEXT, L"MF_PD_PMPHOST_CONTEXT", VT_UNKNOWN, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_SAMI_STYLELIST, L"MF_PD_SAMI_STYLELIST", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_TOTAL_FILE_SIZE, L"MF_PD_TOTAL_FILE_SIZE", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_VIDEO_ENCODING_BITRATE, L"MF_PD_VIDEO_ENCODING_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CODECLIST, L"MF_PD_ASF_CODECLIST", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CONTENTENCRYPTION_KEYID, L"MF_PD_ASF_CONTENTENCRYPTION_KEYID", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CONTENTENCRYPTION_LICENSE_URL, L"MF_PD_ASF_CONTENTENCRYPTION_LICENSE_URL", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CONTENTENCRYPTION_SECRET_DATA, L"MF_PD_ASF_CONTENTENCRYPTION_SECRET_DATA", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CONTENTENCRYPTION_TYPE, L"MF_PD_ASF_CONTENTENCRYPTION_TYPE", VT_LPWSTR, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_CONTENTENCRYPTIONEX_ENCRYPTION_DATA, L"MF_PD_ASF_CONTENTENCRYPTIONEX_ENCRYPTION_DATA", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_DATA_LENGTH, L"MF_PD_ASF_DATA_LENGTH", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_DATA_START_OFFSET, L"MF_PD_ASF_DATA_START_OFFSET", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_CREATION_TIME, L"MF_PD_ASF_FILEPROPERTIES_CREATION_TIME", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_FILE_ID, L"MF_PD_ASF_FILEPROPERTIES_FILE_ID", VT_CLSID, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_FLAGS, L"MF_PD_ASF_FILEPROPERTIES_FLAGS", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_MAX_BITRATE, L"MF_PD_ASF_FILEPROPERTIES_MAX_BITRATE", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_MAX_PACKET_SIZE, L"MF_PD_ASF_FILEPROPERTIES_MAX_PACKET_SIZE", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_MIN_PACKET_SIZE, L"MF_PD_ASF_FILEPROPERTIES_MIN_PACKET_SIZE", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_PACKETS, L"MF_PD_ASF_FILEPROPERTIES_PACKETS", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_PLAY_DURATION, L"MF_PD_ASF_FILEPROPERTIES_PLAY_DURATION", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_PREROLL, L"MF_PD_ASF_FILEPROPERTIES_PREROLL", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_FILEPROPERTIES_SEND_DURATION, L"MF_PD_ASF_FILEPROPERTIES_SEND_DURATION", VT_UI8, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_INFO_HAS_AUDIO, L"MF_PD_ASF_INFO_HAS_AUDIO", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_INFO_HAS_NON_AUDIO_VIDEO, L"MF_PD_ASF_INFO_HAS_NON_AUDIO_VIDEO", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_INFO_HAS_VIDEO, L"MF_PD_ASF_INFO_HAS_VIDEO", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_LANGLIST, L"MF_PD_ASF_LANGLIST", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_MARKER, L"MF_PD_ASF_MARKER", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_METADATA_IS_VBR, L"MF_PD_ASF_METADATA_IS_VBR", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_METADATA_LEAKY_BUCKET_PAIRS, L"MF_PD_ASF_METADATA_LEAKY_BUCKET_PAIRS", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_METADATA_V8_BUFFERAVERAGE, L"MF_PD_ASF_METADATA_V8_BUFFERAVERAGE", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_METADATA_V8_VBRPEAK, L"MF_PD_ASF_METADATA_V8_VBRPEAK", VT_UI4, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR },
    { MF_PD_ASF_SCRIPT, L"MF_PD_ASF_SCRIPT", VT_VECTOR | VT_UI1, TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR }
};

#define AttributeKeyStringsLength sizeof(CPropertyInfo::ms_AttributeKeyStrings) / sizeof(KeyStringTypeTriplet)

KeyStringPair CPropertyInfo::ms_AttributeValueStrings[] = {
    { MFMediaType_Audio, L"MFMediaType_Audio" },
    { MFMediaType_Video, L"MFMediaType_Video" },
    { MFMediaType_Protected, L"MFMediaType_Protected" },
    { MFMediaType_SAMI, L"MFMediaType_SAMI" },
    { MFMediaType_Script, L"MFMediaType_Script" },
    { MFMediaType_Image, L"MFMediaType_Image" },
    { MFMediaType_HTML, L"MFMediaType_HTML" },
    { MFMediaType_Binary, L"MFMediaType_Binary" },
    { MFMediaType_FileTransfer, L"MFMediaType_FileTransfer" },
    { MFAudioFormat_Dolby_AC3_SPDIF, L"MFAudioFormat_Dolby_AC3_SPDIF" },
    { MFAudioFormat_DRM, L"MFAudioFormat_DRM" },
    { MFAudioFormat_DTS, L"MFAudioFormat_DTS" },
    { MFAudioFormat_Float, L"MFAudioFormat_Float" },
    { MFAudioFormat_MP3, L"MFAudioFormat_MP3" },
    { MFAudioFormat_MPEG, L"MFAudioFormat_MPEG" },
    { MFAudioFormat_MSP1, L"MFAudioFormat_MSP1" },
    { MFAudioFormat_PCM, L"MFAudioFormat_PCM" },
    { MFAudioFormat_WMASPDIF, L"MFAudioFormat_WMASPDIF" },
    { MFAudioFormat_WMAudio_Lossless, L"MFAudioFormat_WMAudio_Lossless" },
    { MFAudioFormat_WMAudioV8, L"MFAudioFormat_WMAudioV8" },
    { MFAudioFormat_WMAudioV9, L"MFAudioFormat_WMAudioV9" },
    { MFVideoFormat_ARGB32, L"MFVideoFormat_ARGB32" },
    { MFVideoFormat_RGB24, L"MFVideoFormat_RGB24" },
    { MFVideoFormat_RGB32, L"MFVideoFormat_RGB32" },
    { MFVideoFormat_RGB555, L"MFVideoFormat_RGB555" },
    { MFVideoFormat_RGB565, L"MFVideoFormat_RGB565" },
    { MFVideoFormat_AI44, L"MFVideoFormat_AI44" },
    { MFVideoFormat_AYUV, L"MFVideoFormat_AYUV" },
    { MFVideoFormat_NV11, L"MFVideoFormat_NV11" },
    { MFVideoFormat_NV12, L"MFVideoFormat_NV12" },
    { MFVideoFormat_P010, L"MFVideoFormat_P010" },
    { MFVideoFormat_P016, L"MFVideoFormat_P016" },
    { MFVideoFormat_P210, L"MFVideoFormat_P210" },
    { MFVideoFormat_P216, L"MFVideoFormat_P216" },
    { MFVideoFormat_UYVY, L"MFVideoFormat_UYVY" },
    { MFVideoFormat_v210, L"MFVideoFormat_v210" },
    { MFVideoFormat_v410, L"MFVideoFormat_v410" },
    { MFVideoFormat_Y210, L"MFVideoFormat_Y210" },
    { MFVideoFormat_Y216, L"MFVideoFormat_Y216" },
    { MFVideoFormat_Y410, L"MFVideoFormat_Y410" },
    { MFVideoFormat_Y416, L"MFVideoFormat_Y416" },
    { MFVideoFormat_YUY2, L"MFVideoFormat_YUY2" },
    { MFVideoFormat_YV12, L"MFVideoFormat_YV12" },
    { MFVideoFormat_DV25, L"MFVideoFormat_DV25" },
    { MFVideoFormat_DV50, L"MFVideoFormat_DV50" },
    { MFVideoFormat_DVH1, L"MFVideoFormat_DVH1" },
    { MFVideoFormat_DVSD, L"MFVideoFormat_DVSD" },
    { MFVideoFormat_DVSL, L"MFVideoFormat_DVSL" },
    { MFVideoFormat_MP43, L"MFVideoFormat_MP43" },
    { MFVideoFormat_MP4S, L"MFVideoFormat_MP4S" },
    { MFVideoFormat_MPEG2, L"MFVideoFormat_MPEG2" },
    { MFVideoFormat_MPG1, L"MFVideoFormat_MPG1" },
    { MFVideoFormat_MSS1, L"MFVideoFormat_MSS1" },
    { MFVideoFormat_MSS2, L"MFVideoFormat_MSS2" },
    { MFVideoFormat_WMV1, L"MFVideoFormat_WMV1" },
    { MFVideoFormat_WMV2, L"MFVideoFormat_WMV2" },
    { MFVideoFormat_WMV3, L"MFVideoFormat_WMV3" }
};

#define AttributeValueStringsLength sizeof(CPropertyInfo::ms_AttributeValueStrings) / sizeof(KeyStringPair)

DWORD TEDGetAttributeListLength()
{
    return AttributeKeyStringsLength;
}

LPCWSTR TEDGetAttributeName(DWORD dwIndex)
{
    return CPropertyInfo::ms_AttributeKeyStrings[dwIndex].m_str;
}

GUID TEDGetAttributeGUID(DWORD dwIndex)
{
    return CPropertyInfo::ms_AttributeKeyStrings[dwIndex].m_key;
}

VARTYPE TEDGetAttributeType(DWORD dwIndex)
{
    return CPropertyInfo::ms_AttributeKeyStrings[dwIndex].m_vt;
}

TED_ATTRIBUTE_CATEGORY TEDGetAttributeCategory(DWORD dwIndex)
{
    return CPropertyInfo::ms_AttributeKeyStrings[dwIndex].m_category;
}

BOOL ConvertStringToSystemTime(const CAtlString& strValue, LPSYSTEMTIME pSystemTime)
{
    int iCharLoc = strValue.Find('/');
    if(-1 == iCharLoc) return FALSE;
    pSystemTime->wMonth = (WORD) _wtoi( strValue.Left(iCharLoc) );
 
    int iLastLoc = iCharLoc + 1;
    iCharLoc = strValue.Find('/', iLastLoc);
    if(-1 == iCharLoc) return FALSE;
    pSystemTime->wDay = (WORD) _wtoi( strValue.Mid(iLastLoc, iCharLoc - iLastLoc) );
    
    iLastLoc = iCharLoc + 1;
    iCharLoc = strValue.Find(' ', iLastLoc);
    if(-1 == iCharLoc) return FALSE;
    pSystemTime->wYear = (WORD) _wtoi( strValue.Mid(iLastLoc, iCharLoc - iLastLoc) );
    
    iLastLoc = iCharLoc + 1;
    iCharLoc = strValue.Find(':', iLastLoc);
    if(-1 == iCharLoc) return FALSE;
    pSystemTime->wHour = (WORD) _wtoi( strValue.Mid(iLastLoc, iCharLoc - iLastLoc) );
    
    iLastLoc = iCharLoc + 1;
    iCharLoc = strValue.Find(':', iLastLoc);
    if(-1 == iCharLoc) return FALSE;
    pSystemTime->wMinute = (WORD) _wtoi( strValue.Mid(iLastLoc, iCharLoc - iLastLoc) );
    
    pSystemTime->wSecond = (WORD) _wtoi( strValue.Mid(iLastLoc) );
    
    return TRUE;
}

/*********************************\
 * CPropertyInfo                                  *
\*********************************/

CPropertyInfo::CPropertyInfo()
    : m_cRef(0)
{
}

CPropertyInfo::~CPropertyInfo()
{
}

HRESULT CPropertyInfo::QueryInterface(REFIID riid, void** ppInterface)
{
    if(NULL == ppInterface)
    {
        return E_POINTER;
    }

    if(riid == IID_IUnknown || riid == IID_ITedPropertyInfo)
    {
        ITedPropertyInfo* pPropertyInfo = this;
        *ppInterface = pPropertyInfo;
        AddRef();
        return S_OK;
    }

    *ppInterface = NULL;
    return E_NOINTERFACE;
}

ULONG CPropertyInfo::AddRef()
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
 
    return cRef;
}

ULONG CPropertyInfo::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if(cRef == 0)
    {
        delete this;
    }

    return cRef;
}

void CPropertyInfo::ConvertKeyToString(GUID key, /* out */ CAtlStringW& strName)
{
    USES_CONVERSION;

    bool found = false;
    
    for(DWORD i = 0; i < AttributeKeyStringsLength; i++)
    {
        if(ms_AttributeKeyStrings[i].m_key == key)
        {
            strName = ms_AttributeKeyStrings[i].m_str;
            found = true;
            break;
        }
    }

    if(!found)
    {
        LPOLESTR strClsid = NULL;
    
        StringFromCLSID(key, &strClsid);
        strName = OLE2W(strClsid);
    
        CoTaskMemFree(strClsid);
    }
}

void CPropertyInfo::ConvertPropertyValueToString(GUID key, PROPVARIANT propVal, /* out */ CAtlStringW& strValue)
{
    USES_CONVERSION;
        
    switch(propVal.vt)
    {
        case VT_I1:
            strValue.Format(L"%d", propVal.cVal);
            break;
        case VT_UI1:
            strValue.Format(L"%d", propVal.bVal);
            break;
        case VT_I2:
            strValue.Format(L"%d", propVal.iVal);
            break;
        case VT_UI2:
            strValue.Format(L"%d", propVal.uiVal);
            break;
        case VT_I4:
            strValue.Format(L"%d", propVal.lVal);
            break;
        case VT_UI4:
            if(MF_TOPONODE_ERRORCODE == key)
            {
                strValue.Format(L"%x", propVal.ulVal);
            }
            else
            {
                strValue.Format(L"%d", propVal.ulVal);
            }
            break;
        case VT_UI8:
            if(MF_MT_FRAME_RATE == key ||MF_MT_PIXEL_ASPECT_RATIO == key || MF_MT_FRAME_SIZE == key)
            {
                UINT32 numerator, denominator;
                denominator = propVal.uhVal.LowPart;
                numerator = propVal.uhVal.HighPart;

                strValue.Format(L"N: %d  D: %d", numerator, denominator);
            }
            else
            {
                strValue.Format(L"%ld", propVal.uhVal.QuadPart);
            }
            
            break;
        case VT_INT:
            strValue.Format(L"%d", propVal.intVal);
            break;
        case VT_UINT:
            strValue.Format(L"%d", propVal.uintVal);
            break;
        case VT_R4:
            strValue.Format(L"%f", propVal.fltVal);
            break;
        case VT_R8:
            strValue.Format(L"%f", propVal.dblVal);
            break;
        case VT_BOOL:
            if(VARIANT_TRUE == propVal.boolVal)
            {
                strValue = L"True";
            }
            else
            {
                strValue = L"False";
            }
            break;
        case VT_LPSTR:
            strValue = CA2W(propVal.pszVal);
            break;
        case VT_LPWSTR:
            strValue = propVal.pwszVal;
            break;
        case VT_UNKNOWN:
            if(NULL == propVal.punkVal)
            {
                strValue = L"NULL IUnknown";
            }
            else
            {
                strValue = L"IUnknown";
            }
            break;
        case VT_CLSID:
        {
            bool found = false;

            if(propVal.puuid != NULL)
            {
                for(DWORD i = 0; i < AttributeValueStringsLength; i++)
                {                
                    if(ms_AttributeValueStrings[i].m_key ==  *(propVal.puuid) )
                    {
                        strValue = ms_AttributeValueStrings[i].m_str;
                        found = true;
                        break;
                    }
                }
            

                if(!found)
                {
                    LPOLESTR strClsid = NULL;
        
                    StringFromCLSID(*propVal.puuid, &strClsid);
                    strValue = OLE2W(strClsid);
        
                    CoTaskMemFree(strClsid);
                }
            }
            else
            {
                strValue = L"{00000000-0000-0000-0000-000000000000}";
            }

            break;
        }
        case VT_VECTOR | VT_UI1:
            if(MF_PD_ASF_FILEPROPERTIES_CREATION_TIME == key || MF_PD_LAST_MODIFIED_TIME == key)
            {
                PFILETIME pFileTime;
                SYSTEMTIME SystemTime;
                
                pFileTime = (PFILETIME) propVal.caub.pElems;
                if(FileTimeToSystemTime(pFileTime, &SystemTime))
                {
                    SYSTEMTIME LocalTime;
                    if(SystemTimeToTzSpecificLocalTime(NULL /* current active timezone */, &SystemTime, &LocalTime))
                    {
                        strValue.Format(L"%d/%d/%d %d:%d:%d", LocalTime.wMonth, LocalTime.wDay,
                            LocalTime.wYear, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);
                    }
                }
            }
            else
            {
                CAtlString strTemp;
                for(DWORD i = 0; i < propVal.caub.cElems; i++)
                {
                    strValue.AppendFormat(L"%2.2x%s", propVal.caub.pElems[i], (i == propVal.caub.cElems - 1) ? "" : " ");
                }
            }
            
            break;
    }
}

void CPropertyInfo::ConvertStringToKey(CAtlStringW strName, /* out */ GUID& key)
{
    USES_CONVERSION;
    bool found = false;
    
    for(DWORD i = 0; i < AttributeKeyStringsLength; i++)
    {
        if(ms_AttributeKeyStrings[i].m_str == strName)
        {
            key = ms_AttributeKeyStrings[i].m_key;
            found = true;
            break;
        }
    }

    if(!found)
    {
        LPOLESTR strClsid = W2OLE(strName.GetBuffer());
    
        CLSIDFromString(strClsid, &key);
    }
}

void CPropertyInfo::ConvertStringToPropertyValue(GUID key, CAtlStringW strValue, VARTYPE vt, /* out */ PROPVARIANT& propVal)
{
    USES_CONVERSION;
    
    propVal.vt = vt;

    switch(vt)
    {
        case VT_I1:
            propVal.cVal = (CHAR) _wtoi(strValue);
            break;
        case VT_UI1:
            propVal.bVal =(UCHAR)  _wtoi(strValue);
            break;
        case VT_I2:
            propVal.iVal = (SHORT) _wtoi(strValue);
            break;
        case VT_UI2:
            propVal.uiVal = (USHORT) _wtoi(strValue);
            break;
        case VT_I4:
            propVal.lVal = _wtoi(strValue);
            break;
        case VT_UI4:
            propVal.ulVal = (ULONG) _wtoi64(strValue);
            break;
        case VT_UI8:
        {
            if(strValue.GetLength() > 0 && strValue.GetAt(0) == 'N')
            {
                UINT32 numerator, denominator;
                int charLoc = strValue.Find(' ', 4);

                numerator = (UINT32) _wtoi64(strValue.Mid(3, charLoc - 3));
                denominator = (UINT32) _wtoi64(strValue.Mid(charLoc + 5));

                propVal.uhVal.LowPart = denominator;
                propVal.uhVal.HighPart = numerator;
            }
            else
            {
                propVal.uhVal.QuadPart = _wtoi64(strValue);
            }
            break;
        }        
        case VT_INT:
            propVal.intVal = _wtoi(strValue);
            break;
        case VT_UINT:
            propVal.uintVal = (UINT) _wtoi64(strValue);
            break;
        case VT_R4:
            propVal.fltVal = (FLOAT) _wtof(strValue);
            break;
        case VT_R8:
            propVal.dblVal = _wtof(strValue);
            break;
        case VT_BOOL:
            if(strValue == L"True")
            {
                propVal.boolVal = VARIANT_TRUE;
            }
            else
            {
                propVal.boolVal = VARIANT_FALSE;
            }
            break;
        case VT_LPSTR:
            propVal.pszVal = (LPSTR) CoTaskMemAlloc(sizeof(CHAR) * (strValue.GetLength() + 1));
            strcpy_s(propVal.pszVal, strValue.GetLength() + 1, CW2A(strValue.GetBuffer()));
            break;
        case VT_LPWSTR:
            propVal.pwszVal = (LPWSTR) CoTaskMemAlloc(sizeof(WCHAR) * (strValue.GetLength() + 1));
            wcscpy_s(propVal.pwszVal, strValue.GetLength() + 1, strValue.GetBuffer());
            break;
        case VT_CLSID:
        {
            propVal.puuid = (GUID*) CoTaskMemAlloc(sizeof(GUID));
            bool found = false;
    
            for(DWORD i = 0; i < AttributeValueStringsLength; i++)
            {
                if(ms_AttributeValueStrings[i].m_str ==  strValue )
                {
                    *(propVal.puuid) = ms_AttributeValueStrings[i].m_key;
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                CLSIDFromString(W2OLE(strValue.GetBuffer()), propVal.puuid);
            }

            break;
        }
        case VT_VECTOR | VT_UI1:
        {
            if(MF_PD_ASF_FILEPROPERTIES_CREATION_TIME == key || MF_PD_LAST_MODIFIED_TIME == key)
            {
                FILETIME FileTime;
                SYSTEMTIME SystemTime;
                ZeroMemory(&SystemTime, sizeof(SYSTEMTIME));
                
                ConvertStringToSystemTime(strValue, &SystemTime);
                SystemTimeToFileTime(&SystemTime, &FileTime);
             
                propVal.caub.pElems = (BYTE*) CoTaskMemAlloc(sizeof(FileTime));
                CopyMemory(propVal.pbVal, (BYTE*) &FileTime, sizeof(FileTime));
                propVal.caub.cElems = sizeof(FileTime);
            }
            break;
        }
    }
}

/*********************************\
  * CNodePropertyInfo                         *
\ *********************************/

CNodePropertyInfo::CNodePropertyInfo(CComPtr<IMFTopologyNode> spNode)
    : m_spNode(spNode)
{
    HRESULT hr = S_OK;
    
    CComPtr<IPropertyStore> spPropStore;
    hr = spNode->QueryInterface(IID_IPropertyStore, (void**) &m_spNodePropertyStore);

    if(E_NOINTERFACE == hr) 
    {
        CComPtr<IUnknown> spUnk;
 
        hr = spNode->GetObject(&spUnk);

        if(SUCCEEDED(hr) && spUnk != NULL)
        {
           spUnk->QueryInterface(IID_IPropertyStore, (void**) &m_spNodePropertyStore);
        }
    }

    spNode->QueryInterface(IID_IMFAttributes, (void**) &m_spNodeAttributes);
}

CNodePropertyInfo::~CNodePropertyInfo()
{
}

HRESULT CNodePropertyInfo::GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory)
{
    CAtlString str(L"Node Attributes");
    size_t AllocLen = (str.GetLength() + 1) * sizeof(WCHAR);
    *szName = (LPWSTR) CoTaskMemAlloc(AllocLen);
    
    if(NULL == *szName)
    {
        return E_OUTOFMEMORY;
    }
    
    wcscpy_s(*szName, str.GetLength() + 1, str.GetString());
    
    *pCategory = TED_ATTRIBUTE_CATEGORY_TOPONODE;
    
    return S_OK;
}

HRESULT CNodePropertyInfo::GetPropertyCount(DWORD* pdwCount)
{
    HRESULT hr = S_OK;
    
    if(NULL == pdwCount)
    {
        IFC(E_POINTER);
    }

    if(NULL != m_spNodeAttributes)
    {
        IFC( m_spNodeAttributes->GetCount((UINT32*) pdwCount) );
    }
    else if(NULL != m_spNodePropertyStore)
    {
        IFC( m_spNodePropertyStore->GetCount(pdwCount) );
    }
    else
    {
        *pdwCount = 0;
    }
    
Cleanup:
    return hr;
}

HRESULT CNodePropertyInfo::GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue)
{
    HRESULT hr = S_OK;
    CAtlStringW strNameTemp, strValueTemp;

    if(NULL == strName || NULL == strValue)
    {
        return E_POINTER;
    }

    *strName = NULL;
    *strValue = NULL;
    
    if(NULL != m_spNodeAttributes)
    {
        GUID key;
        PROPVARIANT propVal;

        PropVariantInit(&propVal);
        
        IFC( m_spNodeAttributes->GetItemByIndex(dwIndex, &key, &propVal) );

        ConvertKeyToString(key, strNameTemp);
        ConvertPropertyValueToString(key, propVal, strValueTemp);

        PropVariantClear(&propVal);
    }
    else if(NULL != m_spNodePropertyStore)
    {
        PROPERTYKEY propKey;
        PROPVARIANT propVal;

        PropVariantInit(&propVal);
        
        IFC( m_spNodePropertyStore->GetAt(dwIndex, &propKey) );
        IFC( m_spNodePropertyStore->GetValue(propKey, &propVal) );

        ConvertKeyToString(propKey.fmtid, strNameTemp);
        ConvertPropertyValueToString(propKey.fmtid, propVal, strValueTemp);

        PropVariantClear(&propVal);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    *strName = (LPWSTR) CoTaskMemAlloc((strNameTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strName) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strName, strNameTemp.GetLength() + 1, strNameTemp.GetBuffer());

    *strValue = (LPWSTR) CoTaskMemAlloc((strValueTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strValue) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strValue, strValueTemp.GetLength() + 1, strValueTemp.GetBuffer());

Cleanup:
    if(FAILED(hr))
    {
        CoTaskMemFree(*strName);
        CoTaskMemFree(*strValue);
    }
    
    return hr;
}

HRESULT CNodePropertyInfo::GetPropertyType(DWORD dwIndex, __out VARTYPE* vt)
{
    HRESULT hr = S_OK;
 
    if(NULL == vt)
    {
        IFC( E_POINTER );
    }
   
    if(NULL != m_spNodeAttributes)
    {
        GUID key;
        PROPVARIANT propVal;

        PropVariantInit(&propVal);
        
        IFC( m_spNodeAttributes->GetItemByIndex(dwIndex, &key, &propVal) );

        *vt = propVal.vt;

        PropVariantClear(&propVal);
    }
    else if(NULL != m_spNodePropertyStore)
    {
        PROPERTYKEY propKey;
        PROPVARIANT propVal;

        PropVariantInit(&propVal);
        
        IFC( m_spNodePropertyStore->GetAt(dwIndex, &propKey) );
        IFC( m_spNodePropertyStore->GetValue(propKey, &propVal) );

        *vt = propVal.vt;

        PropVariantClear(&propVal);
    }
    
Cleanup:
    return hr;
}

HRESULT CNodePropertyInfo::SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue)
{
    HRESULT hr = S_OK;
    PROPVARIANT propVal;
    PropVariantInit(&propVal);
    GUID key;

    ConvertStringToKey(strName, key);
    ConvertStringToPropertyValue(key, strValue, vt, propVal);
    
    if(NULL != m_spNodeAttributes)
    {
        IFC( m_spNodeAttributes->SetItem(key, propVal) );
    }
    else
    {
        PROPERTYKEY propKey;

        propKey.fmtid = key;
        propKey.pid = PID_FIRST_USABLE;
        
        IFC( m_spNodePropertyStore->SetValue(propKey, propVal) );
    }

Cleanup:
    PropVariantClear(&propVal);
    return hr;
}

/*********************************\
  * CConnectionPropertyInfo                 *
\ *********************************/

CConnectionPropertyInfo::CConnectionPropertyInfo(CComPtr<IMFMediaType> spUpstreamType, CComPtr<IMFMediaType> spDownstreamType)
    : m_spUpstreamType(spUpstreamType)
    , m_spDownstreamType(spDownstreamType)
{
}

CConnectionPropertyInfo::~CConnectionPropertyInfo()
{
}

HRESULT CConnectionPropertyInfo::GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory)
{
    CAtlString str(L"Media Types");
    size_t AllocLen = (str.GetLength() + 1) * sizeof(WCHAR);
    *szName = (LPWSTR) CoTaskMemAlloc(AllocLen);
    
    if(NULL == *szName)
    {
        return E_OUTOFMEMORY;
    }
    
    wcscpy_s(*szName, str.GetLength() + 1, str.GetString());
    
    *pCategory = TED_ATTRIBUTE_CATEGORY_MEDIATYPE;
    
    return S_OK;
}

HRESULT CConnectionPropertyInfo::GetPropertyCount(DWORD* pdwCount)
{
    HRESULT hr = S_OK;
    
    if(NULL == pdwCount)
    {
        return E_POINTER;
    }

    UINT32 unUpstreamCount;
    UINT32 unDownstreamCount;

    if(m_spUpstreamType)
    {
        IFC( m_spUpstreamType->GetCount(&unUpstreamCount) );
    }
    else
    {
        unUpstreamCount = 1;
    }
    
    if(m_spDownstreamType)
    {
        IFC( m_spDownstreamType->GetCount(&unDownstreamCount) );
    }
    else
    {
        unDownstreamCount = 1;
    }

    *pdwCount = DWORD( unUpstreamCount + unDownstreamCount + 3);
    
Cleanup:
    return hr;
}

HRESULT CConnectionPropertyInfo::GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue)
{
    HRESULT hr = S_OK;

    if(NULL == strName || NULL == strValue)
    {
        return E_POINTER;
    }
    
    UINT32 unUpstreamCount;
    CAtlStringW strNameTemp, strValueTemp;

    *strName = NULL;
    *strValue = NULL;
    
    if(m_spUpstreamType)
    {
        IFC( m_spUpstreamType->GetCount(&unUpstreamCount) );
    }
    else
    {
        unUpstreamCount = 1;
    }
   
    if(unUpstreamCount + 1 > dwIndex)
    {
        if(0 == dwIndex)
        {
            strNameTemp = L"Upstream Media Type";
        }
        else
        {
            if(!m_spUpstreamType)
            {
                strNameTemp = L"None";
            }
            else
            {
                GUID key;
                PROPVARIANT propVal;

                PropVariantInit(&propVal);

                IFC( m_spUpstreamType->GetItemByIndex(dwIndex - 1, &key, &propVal) );

                ConvertKeyToString(key, strNameTemp);
                ConvertPropertyValueToString(key, propVal, strValueTemp);

                PropVariantClear(&propVal);
            }
        }            
    }
    else if(unUpstreamCount + 1 == dwIndex)
    {
        strNameTemp = L"";
    }
    else if(unUpstreamCount + 2 == dwIndex)
    {
        strNameTemp = L"Downstream Media Type";
    }
    else
    {
        if(!m_spDownstreamType)
        {
            strNameTemp = L"None";
        }
        else
        {
            DWORD realIndex = dwIndex - unUpstreamCount - 3;
            GUID key;
            PROPVARIANT propVal;

            PropVariantInit(&propVal);

            IFC( m_spDownstreamType->GetItemByIndex(realIndex, &key, &propVal) );

            ConvertKeyToString(key, strNameTemp);
            ConvertPropertyValueToString(key, propVal, strValueTemp);

            PropVariantClear(&propVal);
        }
    }
    
    *strName = (LPWSTR) CoTaskMemAlloc((strNameTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strName) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strName, strNameTemp.GetLength() + 1, strNameTemp.GetBuffer());

    *strValue = (LPWSTR) CoTaskMemAlloc((strValueTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strValue) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strValue, strValueTemp.GetLength() + 1, strValueTemp.GetBuffer());

Cleanup:
    if(FAILED(hr))
    {
        CoTaskMemFree(*strName);
        CoTaskMemFree(*strValue);
    }
    
    return hr;
}

HRESULT CConnectionPropertyInfo::GetPropertyType(DWORD dwIndex, __out VARTYPE* vt)
{
    HRESULT hr = S_OK;
    
    UINT32 unUpstreamCount;

    if(NULL == vt)
    {
        IFC( E_POINTER );
    }

    if(m_spUpstreamType)
    {
        IFC( m_spUpstreamType->GetCount(&unUpstreamCount) );
    }
    else
    {
        unUpstreamCount = 1;
    }
    
    if(unUpstreamCount + 1 > dwIndex)
    {
        if(0 == dwIndex)
        {
            *vt = VT_EMPTY;
        }
        else
        {
            if(!m_spUpstreamType)
            {
                *vt = VT_EMPTY;
            }
            else
            {
                GUID key;
                PROPVARIANT propVal;

                PropVariantInit(&propVal);

                IFC( m_spUpstreamType->GetItemByIndex(dwIndex - 1, &key, &propVal) );

                *vt = propVal.vt;

                PropVariantClear(&propVal);
            }
        }            
    }
    else if(unUpstreamCount + 1 == dwIndex)
    {
        *vt = VT_EMPTY;
    }
    else if(unUpstreamCount + 2 == dwIndex)
    {
        *vt = VT_EMPTY;
    }
    else
    {
        if(!m_spDownstreamType)
        {
            *vt = VT_EMPTY;
        }
        else
        {
            DWORD realIndex = dwIndex - unUpstreamCount - 3;
            GUID key;
            PROPVARIANT propVal;

            PropVariantInit(&propVal);

            IFC( m_spDownstreamType->GetItemByIndex(realIndex, &key, &propVal) );

            *vt = propVal.vt;

            PropVariantClear(&propVal);
        }
    }
    
Cleanup:
    return hr;
}

HRESULT CConnectionPropertyInfo::SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue)
{
    HRESULT hr = S_OK;
    UINT32 unUpstreamCount;
    PROPVARIANT propVal;
    PropVariantInit(&propVal);
    GUID key;

    ConvertStringToKey(strName, key);
    ConvertStringToPropertyValue(key, strValue, vt, propVal);
    
    IFC( m_spUpstreamType->GetCount(&unUpstreamCount) );

    if(dwIndex <= unUpstreamCount + 1)
    {
        IFC( m_spUpstreamType->SetItem(key, propVal) );
    }
    else
    {
        IFC( m_spDownstreamType->SetItem(key, propVal) );
    }

Cleanup:
    PropVariantClear(&propVal);
    return hr;
}

/*********************************\
  * CAttributesPropertyInfo                    *
\ *********************************/

CAttributesPropertyInfo::CAttributesPropertyInfo(CComPtr<IMFAttributes> spAttributes, CAtlString strName, TED_ATTRIBUTE_CATEGORY Category)
    : m_spAttributes(spAttributes)
    , m_strName(strName)
    , m_Category(Category)
{
}

CAttributesPropertyInfo::~CAttributesPropertyInfo()
{
}

HRESULT CAttributesPropertyInfo::GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory)
{
    size_t AllocLen = (m_strName.GetLength() + 1) * sizeof(WCHAR);
    *szName = (LPWSTR) CoTaskMemAlloc(AllocLen);
    
    if(NULL == *szName)
    {
        return E_OUTOFMEMORY;
    }
    
    wcscpy_s(*szName, m_strName.GetLength() + 1, m_strName.GetString());
    
    *pCategory = m_Category;
    
    return S_OK;
}

HRESULT CAttributesPropertyInfo::GetPropertyCount(DWORD* pdwCount)
{
     HRESULT hr = S_OK;
    
    if(NULL == pdwCount)
    {
        return E_POINTER;
    }

    IFC( m_spAttributes->GetCount((UINT32*) pdwCount) );

Cleanup:
    return hr;
}

HRESULT CAttributesPropertyInfo::GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue)
{
    HRESULT hr = S_OK;

    CAtlStringW strNameTemp, strValueTemp;
    GUID key;
    PROPVARIANT var;

    PropVariantInit(&var);

    *strName = NULL;
    *strValue = NULL;
    
    IFC( m_spAttributes->GetItemByIndex(dwIndex, &key, &var) );

    ConvertKeyToString(key, strNameTemp);
    ConvertPropertyValueToString(key, var, strValueTemp);

    *strName = (LPWSTR) CoTaskMemAlloc((strNameTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strName) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strName, strNameTemp.GetLength() + 1, strNameTemp.GetBuffer());

    *strValue = (LPWSTR) CoTaskMemAlloc((strValueTemp.GetLength() + 1) * sizeof(WCHAR));
    if(!*strValue) IFC( E_OUTOFMEMORY );
    wcscpy_s(*strValue, strValueTemp.GetLength() + 1, strValueTemp.GetBuffer());
    
Cleanup:
    if(FAILED(hr))
    {
        CoTaskMemFree(*strName);
        CoTaskMemFree(*strValue);
    }
    
    PropVariantClear(&var);
    return hr;
}

HRESULT CAttributesPropertyInfo::GetPropertyType(DWORD dwIndex, __out VARTYPE* vt)
{
    HRESULT hr = S_OK;

    if(NULL == vt)
    {
        return E_POINTER;
    }

    GUID key;
    PROPVARIANT var;

    PropVariantInit(&var);

    IFC( m_spAttributes->GetItemByIndex(dwIndex, &key, &var) );

    *vt = var.vt;
    
Cleanup:
    PropVariantClear(&var);
    return hr;
}

HRESULT CAttributesPropertyInfo::SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue)
{
    HRESULT hr = S_OK;
    GUID key;
    PROPVARIANT var;
    PropVariantInit(&var);

    ConvertStringToKey(strName, key);
    ConvertStringToPropertyValue(key, strValue, vt, var);

    IFC( m_spAttributes->SetItem(key, var) );

Cleanup:
    PropVariantClear(&var);
    return hr;
}

/*********************************\
  * COTAPropertyInfo                            *
\ *********************************/

COTAPropertyInfo::COTAPropertyInfo(CComPtr<IMFOutputTrustAuthority>* arrOTA, DWORD cOTACount)
    : m_arrOTA(arrOTA)
    , m_cOTACount(cOTACount)
{
}

COTAPropertyInfo::~COTAPropertyInfo()
{
    delete[] m_arrOTA;
}

HRESULT COTAPropertyInfo::GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory)
{
    CAtlString str(L"OTA Attributes");
    size_t AllocLen = (str.GetLength() + 1) * sizeof(WCHAR);
    *szName = (LPWSTR) CoTaskMemAlloc(AllocLen);
    
    if(NULL == *szName)
    {
        return E_OUTOFMEMORY;
    }
    
    wcscpy_s(*szName, str.GetLength() + 1, str.GetString());
    
    *pCategory = TED_ATTRIBUTE_CATEGORY_OTA;
    
    return S_OK;
}

HRESULT COTAPropertyInfo::GetPropertyCount(DWORD* pdwCount)
{
    if(NULL == pdwCount)
    {
        return E_POINTER;
    }

    *pdwCount = m_cOTACount + 1;

    return S_OK;
}

HRESULT COTAPropertyInfo::GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue)
{
    HRESULT hr = S_OK;
    
    if(NULL == strName || NULL == strValue)
    {
        return E_POINTER;
    }

    if(m_cOTACount + 1 <= dwIndex)
    {
        return E_INVALIDARG;
    }

    *strName = NULL;
    *strValue = NULL;
    
    if(0 == dwIndex)
    {
        *strName = (LPWSTR) CoTaskMemAlloc(25 * sizeof(WCHAR));
        CHECK_ALLOC(*strName);
        wcscpy_s(*strName, 25, L"Output Trust Authorities");
    }
    else
    {
        MFPOLICYMANAGER_ACTION action;
        IFC( m_arrOTA[dwIndex - 1]->GetAction(&action) );

        switch(action)
        {
            case PEACTION_NO:
                *strName = (LPWSTR) CoTaskMemAlloc(12 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 12, L"PEACTION_NO");
                break;
            case PEACTION_PLAY:
                *strName = (LPWSTR) CoTaskMemAlloc(14 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 14, L"PEACTION_PLAY");
                break;
            case PEACTION_COPY:
                *strName = (LPWSTR) CoTaskMemAlloc(14 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 14, L"PEACTION_COPY");
                break;
            case PEACTION_EXPORT:
                *strName = (LPWSTR) CoTaskMemAlloc(16 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 16, L"PEACTION_EXPORT");
                break;
            case PEACTION_EXTRACT:
                *strName = (LPWSTR) CoTaskMemAlloc(17 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 17, L"PEACTION_EXTRACT");
                break;
            case PEACTION_LAST:
                *strName = (LPWSTR) CoTaskMemAlloc(14 * sizeof(WCHAR));
                CHECK_ALLOC(*strName);
                wcscpy_s(*strName, 14, L"PEACTION_LAST");
                break;
        }
    }

    *strValue = (LPWSTR) CoTaskMemAlloc(1 * sizeof(WCHAR) );
    CHECK_ALLOC(*strValue);
    *strValue[0] = 0;

Cleanup:
    if(FAILED(hr))
    {
        CoTaskMemFree(*strName);
        CoTaskMemFree(*strValue);
    }
    
    return hr;
}

HRESULT COTAPropertyInfo::GetPropertyType(DWORD dwIndex, __out VARTYPE* vt)
{
    if(NULL == vt)
    {
        return E_POINTER;
    }

    if(m_cOTACount + 1 <= dwIndex)
    {
        return E_INVALIDARG;
    }

    *vt = VT_EMPTY;

    return S_OK;
}

HRESULT COTAPropertyInfo::SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue)
{
    return E_NOTIMPL;
}
