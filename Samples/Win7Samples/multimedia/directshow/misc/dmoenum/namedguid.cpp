//------------------------------------------------------------------------------
// File: NamedGuid.cpp
//
// Desc: DirectShow sample code - helps in converting GUIDs to strings
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <dshow.h>
#include <dmo.h>
#include <strsafe.h>

#include "namedguid.h"

//
// Create a large table to allow searches by CLSID, media type, IID, etc.
//
const NamedGuid rgng[] =
{
    {&MEDIASUBTYPE_AIFF, TEXT("MEDIASUBTYPE_AIFF\0")},
    {&MEDIASUBTYPE_AU, TEXT("MEDIASUBTYPE_AU\0")},
    {&MEDIASUBTYPE_AnalogVideo_NTSC_M, TEXT("MEDIASUBTYPE_AnalogVideo_NTSC_M\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_B, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_B\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_D, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_D\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_G, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_G\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_H, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_H\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_I, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_I\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_M, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_M\0")},
    {&MEDIASUBTYPE_AnalogVideo_PAL_N, TEXT("MEDIASUBTYPE_AnalogVideo_PAL_N\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_B, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_B\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_D, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_D\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_G, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_G\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_H, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_H\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_K\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K1, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_K1\0")},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_L, TEXT("MEDIASUBTYPE_AnalogVideo_SECAM_L\0")},

    {&MEDIASUBTYPE_ARGB1555, TEXT("MEDIASUBTYPE_ARGB1555\0")},
    {&MEDIASUBTYPE_ARGB4444, TEXT("MEDIASUBTYPE_ARGB4444\0")},
    {&MEDIASUBTYPE_ARGB32, TEXT("MEDIASUBTYPE_ARGB32\0")},
    {&MEDIASUBTYPE_A2R10G10B10, TEXT("MEDIASUBTYPE_A2R10G10B10\0")},
    {&MEDIASUBTYPE_A2B10G10R10, TEXT("MEDIASUBTYPE_A2B10G10R10\0")},

    {&MEDIASUBTYPE_AYUV, TEXT("MEDIASUBTYPE_AYUV\0")},
    {&MEDIASUBTYPE_AI44, TEXT("MEDIASUBTYPE_AI44\0")},
    {&MEDIASUBTYPE_IA44, TEXT("MEDIASUBTYPE_IA44\0")},
    {&MEDIASUBTYPE_NV12, TEXT("MEDIASUBTYPE_NV12\0")},
    {&MEDIASUBTYPE_IMC1, TEXT("MEDIASUBTYPE_IMC1\0")},
    {&MEDIASUBTYPE_IMC2, TEXT("MEDIASUBTYPE_IMC2\0")},
    {&MEDIASUBTYPE_IMC3, TEXT("MEDIASUBTYPE_IMC3\0")},
    {&MEDIASUBTYPE_IMC4, TEXT("MEDIASUBTYPE_IMC4\0")},

    {&MEDIASUBTYPE_Asf, TEXT("MEDIASUBTYPE_Asf\0")},
    {&MEDIASUBTYPE_Avi, TEXT("MEDIASUBTYPE_Avi\0")},
    {&MEDIASUBTYPE_CFCC, TEXT("MEDIASUBTYPE_CFCC\0")},
    {&MEDIASUBTYPE_CLJR, TEXT("MEDIASUBTYPE_CLJR\0")},
    {&MEDIASUBTYPE_CPLA, TEXT("MEDIASUBTYPE_CPLA\0")},
    {&MEDIASUBTYPE_CLPL, TEXT("MEDIASUBTYPE_CLPL\0")},
    {&MEDIASUBTYPE_DOLBY_AC3, TEXT("MEDIASUBTYPE_DOLBY_AC3\0")},
    {&MEDIASUBTYPE_DOLBY_AC3_SPDIF, TEXT("MEDIASUBTYPE_DOLBY_AC3_SPDIF\0")},
    {&MEDIASUBTYPE_DVCS, TEXT("MEDIASUBTYPE_DVCS\0")},
    {&MEDIASUBTYPE_DVD_LPCM_AUDIO, TEXT("MEDIASUBTYPE_DVD_LPCM_AUDIO\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_DSI, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_DSI\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PCI, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_PCI\0")},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, TEXT("MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER\0")},
    {&MEDIASUBTYPE_DVD_SUBPICTURE, TEXT("MEDIASUBTYPE_DVD_SUBPICTURE\0")},
    {&MEDIASUBTYPE_DVSD, TEXT("MEDIASUBTYPE_DVSD\0")},
    {&MEDIASUBTYPE_DRM_Audio, TEXT("MEDIASUBTYPE_DRM_Audio\0")},
    {&MEDIASUBTYPE_DssAudio, TEXT("MEDIASUBTYPE_DssAudio\0")},
    {&MEDIASUBTYPE_DssVideo, TEXT("MEDIASUBTYPE_DssVideo\0")},
    {&MEDIASUBTYPE_IF09, TEXT("MEDIASUBTYPE_IF09\0")},
    {&MEDIASUBTYPE_IEEE_FLOAT, TEXT("MEDIASUBTYPE_IEEE_FLOAT\0")},
    {&MEDIASUBTYPE_IJPG, TEXT("MEDIASUBTYPE_IJPG\0")},
    {&MEDIASUBTYPE_IYUV, TEXT("MEDIASUBTYPE_IYUV\0")},
    {&MEDIASUBTYPE_Line21_BytePair, TEXT("MEDIASUBTYPE_Line21_BytePair\0")},
    {&MEDIASUBTYPE_Line21_GOPPacket, TEXT("MEDIASUBTYPE_Line21_GOPPacket\0")},
    {&MEDIASUBTYPE_Line21_VBIRawData, TEXT("MEDIASUBTYPE_Line21_VBIRawData\0")},
    {&MEDIASUBTYPE_MDVF, TEXT("MEDIASUBTYPE_MDVF\0")},
    {&MEDIASUBTYPE_MJPG, TEXT("MEDIASUBTYPE_MJPG\0")},
    {&MEDIASUBTYPE_MPEG1Audio, TEXT("MEDIASUBTYPE_MPEG1Audio\0")},
    {&MEDIASUBTYPE_MPEG1AudioPayload, TEXT("MEDIASUBTYPE_MPEG1AudioPayload\0")},
    {&MEDIASUBTYPE_MPEG1Packet, TEXT("MEDIASUBTYPE_MPEG1Packet\0")},
    {&MEDIASUBTYPE_MPEG1Payload, TEXT("MEDIASUBTYPE_MPEG1Payload\0")},
    {&MEDIASUBTYPE_MPEG1System, TEXT("MEDIASUBTYPE_MPEG1System\0")},
    {&MEDIASUBTYPE_MPEG1Video, TEXT("MEDIASUBTYPE_MPEG1Video\0")},
    {&MEDIASUBTYPE_MPEG1VideoCD, TEXT("MEDIASUBTYPE_MPEG1VideoCD\0")},
    {&MEDIASUBTYPE_MPEG2_AUDIO, TEXT("MEDIASUBTYPE_MPEG2_AUDIO\0")},
    {&MEDIASUBTYPE_MPEG2_PROGRAM, TEXT("MEDIASUBTYPE_MPEG2_PROGRAM\0")},
    {&MEDIASUBTYPE_MPEG2_TRANSPORT, TEXT("MEDIASUBTYPE_MPEG2_TRANSPORT\0")},
    {&MEDIASUBTYPE_MPEG2_VIDEO, TEXT("MEDIASUBTYPE_MPEG2_VIDEO\0")},
    {&MEDIASUBTYPE_None, TEXT("MEDIASUBTYPE_None\0")},
    {&MEDIASUBTYPE_Overlay, TEXT("MEDIASUBTYPE_Overlay\0")},
    {&MEDIASUBTYPE_PCM, TEXT("MEDIASUBTYPE_PCM\0")},
    {&MEDIASUBTYPE_PCMAudio_Obsolete, TEXT("MEDIASUBTYPE_PCMAudio_Obsolete\0")},
    {&MEDIASUBTYPE_Plum, TEXT("MEDIASUBTYPE_Plum\0")},
    {&MEDIASUBTYPE_QTJpeg, TEXT("MEDIASUBTYPE_QTJpeg\0")},
    {&MEDIASUBTYPE_QTMovie, TEXT("MEDIASUBTYPE_QTMovie\0")},
    {&MEDIASUBTYPE_QTRle, TEXT("MEDIASUBTYPE_QTRle\0")},
    {&MEDIASUBTYPE_QTRpza, TEXT("MEDIASUBTYPE_QTRpza\0")},
    {&MEDIASUBTYPE_QTSmc, TEXT("MEDIASUBTYPE_QTSmc\0")},
    {&MEDIASUBTYPE_RAW_SPORT, TEXT("MEDIASUBTYPE_RAW_SPORT\0")},
    {&MEDIASUBTYPE_RGB1, TEXT("MEDIASUBTYPE_RGB1\0")},
    {&MEDIASUBTYPE_RGB24, TEXT("MEDIASUBTYPE_RGB24\0")},
    {&MEDIASUBTYPE_RGB32, TEXT("MEDIASUBTYPE_RGB32\0")},
    {&MEDIASUBTYPE_RGB4, TEXT("MEDIASUBTYPE_RGB4\0")},
    {&MEDIASUBTYPE_RGB555, TEXT("MEDIASUBTYPE_RGB555\0")},
    {&MEDIASUBTYPE_RGB565, TEXT("MEDIASUBTYPE_RGB565\0")},
    {&MEDIASUBTYPE_RGB8, TEXT("MEDIASUBTYPE_RGB8\0")},
    {&MEDIASUBTYPE_SPDIF_TAG_241h, TEXT("MEDIASUBTYPE_SPDIF_TAG_241h\0")},
    {&MEDIASUBTYPE_TELETEXT, TEXT("MEDIASUBTYPE_TELETEXT\0")},
    {&MEDIASUBTYPE_TVMJ, TEXT("MEDIASUBTYPE_TVMJ\0")},
    {&MEDIASUBTYPE_UYVY, TEXT("MEDIASUBTYPE_UYVY\0")},
    {&MEDIASUBTYPE_VPVBI, TEXT("MEDIASUBTYPE_VPVBI\0")},
    {&MEDIASUBTYPE_VPVideo, TEXT("MEDIASUBTYPE_VPVideo\0")},
    {&MEDIASUBTYPE_WAKE, TEXT("MEDIASUBTYPE_WAKE\0")},
    {&MEDIASUBTYPE_WAVE, TEXT("MEDIASUBTYPE_WAVE\0")},
    {&MEDIASUBTYPE_Y211, TEXT("MEDIASUBTYPE_Y211\0")},
    {&MEDIASUBTYPE_Y411, TEXT("MEDIASUBTYPE_Y411\0")},
    {&MEDIASUBTYPE_Y41P, TEXT("MEDIASUBTYPE_Y41P\0")},
    {&MEDIASUBTYPE_YUY2, TEXT("MEDIASUBTYPE_YUY2\0")},
    {&MEDIASUBTYPE_YV12, TEXT("MEDIASUBTYPE_YV12\0")},
    {&MEDIASUBTYPE_YVU9, TEXT("MEDIASUBTYPE_YVU9\0")},
    {&MEDIASUBTYPE_YVYU, TEXT("MEDIASUBTYPE_YVYU\0")},
    {&MEDIASUBTYPE_YUYV, TEXT("MEDIASUBTYPE_YUYV\0")},
    {&MEDIASUBTYPE_dvhd, TEXT("MEDIASUBTYPE_dvhd\0")},
    {&MEDIASUBTYPE_dvsd, TEXT("MEDIASUBTYPE_dvsd\0")},
    {&MEDIASUBTYPE_dvsl, TEXT("MEDIASUBTYPE_dvsl\0")},

    {&MEDIATYPE_AUXLine21Data, TEXT("MEDIATYPE_AUXLine21Data\0")},
    {&MEDIATYPE_AnalogAudio, TEXT("MEDIATYPE_AnalogAudio\0")},
    {&MEDIATYPE_AnalogVideo, TEXT("MEDIATYPE_AnalogVideo\0")},
    {&MEDIATYPE_Audio, TEXT("MEDIATYPE_Audio\0")},
    {&MEDIATYPE_DVD_ENCRYPTED_PACK, TEXT("MEDIATYPE_DVD_ENCRYPTED_PACK\0")},
    {&MEDIATYPE_DVD_NAVIGATION, TEXT("MEDIATYPE_DVD_NAVIGATION\0")},
    {&MEDIATYPE_File, TEXT("MEDIATYPE_File\0")},
    {&MEDIATYPE_Interleaved, TEXT("MEDIATYPE_Interleaved\0")},
    {&MEDIATYPE_LMRT, TEXT("MEDIATYPE_LMRT\0")},
    {&MEDIATYPE_MPEG1SystemStream, TEXT("MEDIATYPE_MPEG1SystemStream\0")},
    {&MEDIATYPE_MPEG2_PES, TEXT("MEDIATYPE_MPEG2_PES\0")},
    {&MEDIATYPE_Midi, TEXT("MEDIATYPE_Midi\0")},
    {&MEDIATYPE_ScriptCommand, TEXT("MEDIATYPE_ScriptCommand\0")},
    {&MEDIATYPE_Stream, TEXT("MEDIATYPE_Stream\0")},
    {&MEDIATYPE_Text, TEXT("MEDIATYPE_Text\0")},
    {&MEDIATYPE_Timecode, TEXT("MEDIATYPE_Timecode\0")},
    {&MEDIATYPE_URL_STREAM, TEXT("MEDIATYPE_URL_STREAM\0")},
    {&MEDIATYPE_VBI, TEXT("MEDIATYPE_VBI\0")},
    {&MEDIATYPE_Video, TEXT("MEDIATYPE_Video\0")},

    {&WMMEDIATYPE_Audio,  TEXT("WMMEDIATYPE_Audio\0")},
    {&WMMEDIATYPE_Video,  TEXT("WMMEDIATYPE_Video\0")},
    {&WMMEDIATYPE_Script, TEXT("WMMEDIATYPE_Script\0")},
    {&WMMEDIATYPE_Image,  TEXT("WMMEDIATYPE_Image\0")},
    {&WMMEDIATYPE_FileTransfer, TEXT("WMMEDIATYPE_FileTransfer\0")},
    {&WMMEDIATYPE_Text,   TEXT("WMMEDIATYPE_Text\0")},

    {&WMMEDIASUBTYPE_Base, TEXT("WMMEDIASUBTYPE_Base\0")},
    {&WMMEDIASUBTYPE_RGB1, TEXT("WMMEDIASUBTYPE_RGB1\0")},
    {&WMMEDIASUBTYPE_RGB4, TEXT("WMMEDIASUBTYPE_RGB4\0")},
    {&WMMEDIASUBTYPE_RGB8, TEXT("WMMEDIASUBTYPE_RGB8\0")},
    {&WMMEDIASUBTYPE_RGB565, TEXT("WMMEDIASUBTYPE_RGB565\0")},
    {&WMMEDIASUBTYPE_RGB555, TEXT("WMMEDIASUBTYPE_RGB555\0")},
    {&WMMEDIASUBTYPE_RGB24, TEXT("WMMEDIASUBTYPE_RGB24\0")},
    {&WMMEDIASUBTYPE_RGB32, TEXT("WMMEDIASUBTYPE_RGB32\0")},
    {&WMMEDIASUBTYPE_I420, TEXT("WMMEDIASUBTYPE_I420\0")},
    {&WMMEDIASUBTYPE_IYUV, TEXT("WMMEDIASUBTYPE_IYUV\0")},
    {&WMMEDIASUBTYPE_YV12, TEXT("WMMEDIASUBTYPE_YV12\0")},
    {&WMMEDIASUBTYPE_YUY2, TEXT("WMMEDIASUBTYPE_YUY2\0")},
    {&WMMEDIASUBTYPE_UYVY, TEXT("WMMEDIASUBTYPE_UYVY\0")},
    {&WMMEDIASUBTYPE_YVYU, TEXT("WMMEDIASUBTYPE_YVYU\0")},
    {&WMMEDIASUBTYPE_YVU9, TEXT("WMMEDIASUBTYPE_YVU9\0")},
    {&WMMEDIASUBTYPE_MP43, TEXT("WMMEDIASUBTYPE_MP43\0")},
    {&WMMEDIASUBTYPE_MP4S, TEXT("WMMEDIASUBTYPE_MP4S\0")},

    {&WMMEDIASUBTYPE_WMV1, TEXT("WMMEDIASUBTYPE_WMV1\0")},
    {&WMMEDIASUBTYPE_WMV2, TEXT("WMMEDIASUBTYPE_WMV2\0")},
    {&WMMEDIASUBTYPE_WMV3, TEXT("WMMEDIASUBTYPE_WMV3\0")},
    {&WMMEDIASUBTYPE_MSS1, TEXT("WMMEDIASUBTYPE_MSS1\0")},
    {&WMMEDIASUBTYPE_MSS2, TEXT("WMMEDIASUBTYPE_MSS2\0")},
    {&WMMEDIASUBTYPE_MPEG2_VIDEO,  TEXT("WMMEDIASUBTYPE_MPEG2_VIDEO\0")},
    {&WMMEDIASUBTYPE_PCM, TEXT("WMMEDIASUBTYPE_PCM\0")},
    {&WMMEDIASUBTYPE_DRM, TEXT("WMMEDIASUBTYPE_DRM\0")},
    {&WMMEDIASUBTYPE_WMAudioV9, TEXT("WMMEDIASUBTYPE_WMAudioV9\0")},
    {&WMMEDIASUBTYPE_WMAudio_Lossless, TEXT("WMMEDIASUBTYPE_WMAudio_Lossless\0")},
    {&WMMEDIASUBTYPE_WMAudioV8, TEXT("WMMEDIASUBTYPE_WMAudioV8\0")},
    {&WMMEDIASUBTYPE_WMAudioV7, TEXT("WMMEDIASUBTYPE_WMAudioV7\0")},
    {&WMMEDIASUBTYPE_WMAudioV2, TEXT("WMMEDIASUBTYPE_WMAudioV2\0")},
    {&WMMEDIASUBTYPE_ACELPnet, TEXT("WMMEDIASUBTYPE_ACELPnet\0")},
    {&WMMEDIASUBTYPE_WMSP1, TEXT("WMMEDIASUBTYPE_WMSP1\0")},

    {&WMFORMAT_VideoInfo,    TEXT("WMFORMAT_VideoInfo\0")},
    {&WMFORMAT_WaveFormatEx, TEXT("WMFORMAT_WaveFormatEx\0")},
    {&WMFORMAT_Script,       TEXT("WMFORMAT_Script\0")},
    {&WMFORMAT_MPEG2Video,   TEXT("WMFORMAT_MPEG2Video\0")},

    {&WMSCRIPTTYPE_TwoStrings, TEXT("WMSCRIPTTYPE_TwoStrings\0")},

    {&PIN_CATEGORY_ANALOGVIDEOIN, TEXT("PIN_CATEGORY_ANALOGVIDEOIN\0")},
    {&PIN_CATEGORY_CAPTURE, TEXT("PIN_CATEGORY_CAPTURE\0")},
    {&PIN_CATEGORY_CC, TEXT("PIN_CATEGORY_CC\0")},
    {&PIN_CATEGORY_EDS, TEXT("PIN_CATEGORY_EDS\0")},
    {&PIN_CATEGORY_NABTS, TEXT("PIN_CATEGORY_NABTS\0")},
    {&PIN_CATEGORY_PREVIEW, TEXT("PIN_CATEGORY_PREVIEW\0")},
    {&PIN_CATEGORY_STILL, TEXT("PIN_CATEGORY_STILL\0")},
    {&PIN_CATEGORY_TELETEXT, TEXT("PIN_CATEGORY_TELETEXT\0")},
    {&PIN_CATEGORY_TIMECODE, TEXT("PIN_CATEGORY_TIMECODE\0")},
    {&PIN_CATEGORY_VBI, TEXT("PIN_CATEGORY_VBI\0")},
    {&PIN_CATEGORY_VIDEOPORT, TEXT("PIN_CATEGORY_VIDEOPORT\0")},
    {&PIN_CATEGORY_VIDEOPORT_VBI, TEXT("PIN_CATEGORY_VIDEOPORT_VBI\0")},

    {&CLSID_ACMWrapper, TEXT("CLSID_ACMWrapper\0")},
    {&CLSID_AVICo, TEXT("CLSID_AVICo\0")},
    {&CLSID_AVIDec, TEXT("CLSID_AVIDec\0")},
    {&CLSID_AVIDoc, TEXT("CLSID_AVIDoc\0")},
    {&CLSID_AVIDraw, TEXT("CLSID_AVIDraw\0")},
    {&CLSID_AVIMIDIRender, TEXT("CLSID_AVIMIDIRender\0")},
    {&CLSID_ActiveMovieCategories, TEXT("CLSID_ActiveMovieCategories\0")},
    {&CLSID_AnalogVideoDecoderPropertyPage, TEXT("CLSID_AnalogVideoDecoderPropertyPage\0")},
    {&CLSID_WMAsfReader, TEXT("CLSID_WMAsfReader\0")},
    {&CLSID_WMAsfWriter, TEXT("CLSID_WMAsfWriter\0")},
    {&CLSID_AsyncReader, TEXT("CLSID_AsyncReader\0")},
    {&CLSID_AudioCompressorCategory, TEXT("CLSID_AudioCompressorCategory\0")},
    {&CLSID_AudioInputDeviceCategory, TEXT("CLSID_AudioInputDeviceCategory\0")},
    {&CLSID_AudioProperties, TEXT("CLSID_AudioProperties\0")},
    {&CLSID_AudioRecord, TEXT("CLSID_AudioRecord\0")},
    {&CLSID_AudioRender, TEXT("CLSID_AudioRender\0")},
    {&CLSID_AudioRendererCategory, TEXT("CLSID_AudioRendererCategory\0")},
    {&CLSID_AviDest, TEXT("CLSID_AviDest\0")},
    {&CLSID_AviMuxProptyPage, TEXT("CLSID_AviMuxProptyPage\0")},
    {&CLSID_AviMuxProptyPage1, TEXT("CLSID_AviMuxProptyPage1\0")},
    {&CLSID_AviReader, TEXT("CLSID_AviReader\0")},
    {&CLSID_AviSplitter, TEXT("CLSID_AviSplitter\0")},
    {&CLSID_CAcmCoClassManager, TEXT("CLSID_CAcmCoClassManager\0")},
    {&CLSID_CDeviceMoniker, TEXT("CLSID_CDeviceMoniker\0")},
    {&CLSID_CIcmCoClassManager, TEXT("CLSID_CIcmCoClassManager\0")},
    {&CLSID_CMidiOutClassManager, TEXT("CLSID_CMidiOutClassManager\0")},
    {&CLSID_CMpegAudioCodec, TEXT("CLSID_CMpegAudioCodec\0")},
    {&CLSID_CMpegVideoCodec, TEXT("CLSID_CMpegVideoCodec\0")},
    {&CLSID_CQzFilterClassManager, TEXT("CLSID_CQzFilterClassManager\0")},
    {&CLSID_CVidCapClassManager, TEXT("CLSID_CVidCapClassManager\0")},
    {&CLSID_CWaveOutClassManager, TEXT("CLSID_CWaveOutClassManager\0")},
    {&CLSID_CWaveinClassManager, TEXT("CLSID_CWaveinClassManager\0")},
    {&CLSID_CameraControlPropertyPage, TEXT("CLSID_CameraControlPropertyPage\0")},
    {&CLSID_CaptureGraphBuilder, TEXT("CLSID_CaptureGraphBuilder\0")},
    {&CLSID_CaptureProperties, TEXT("CLSID_CaptureProperties\0")},
    {&CLSID_Colour, TEXT("CLSID_Colour\0")},
    {&CLSID_CrossbarFilterPropertyPage, TEXT("CLSID_CrossbarFilterPropertyPage\0")},
    {&CLSID_DSoundRender, TEXT("CLSID_DSoundRender\0")},
    {&CLSID_DVDHWDecodersCategory, TEXT("CLSID_DVDHWDecodersCategory\0")},
    {&CLSID_DVDNavigator, TEXT("CLSID_DVDNavigator\0")},
    {&CLSID_DVDecPropertiesPage, TEXT("CLSID_DVDecPropertiesPage\0")},
    {&CLSID_DVEncPropertiesPage, TEXT("CLSID_DVEncPropertiesPage\0")},
    {&CLSID_DVMux, TEXT("CLSID_DVMux\0")},
    {&CLSID_DVMuxPropertyPage, TEXT("CLSID_DVMuxPropertyPage\0")},
    {&CLSID_DVSplitter, TEXT("CLSID_DVSplitter\0")},
    {&CLSID_DVVideoCodec, TEXT("CLSID_DVVideoCodec\0")},
    {&CLSID_DVVideoEnc, TEXT("CLSID_DVVideoEnc\0")},
    {&CLSID_DirectDraw, TEXT("CLSID_DirectDraw\0")},
    {&CLSID_DirectDrawClipper, TEXT("CLSID_DirectDrawClipper\0")},
    {&CLSID_DirectDrawProperties, TEXT("CLSID_DirectDrawProperties\0")},
    {&CLSID_Dither, TEXT("CLSID_Dither\0")},
    {&CLSID_DvdGraphBuilder, TEXT("CLSID_DvdGraphBuilder\0")},
    {&CLSID_FGControl, TEXT("CLSID_FGControl\0")},
    {&CLSID_FileSource, TEXT("CLSID_FileSource\0")},
    {&CLSID_FileWriter, TEXT("CLSID_FileWriter\0")},
    {&CLSID_FilterGraph, TEXT("CLSID_FilterGraph\0")},
    {&CLSID_FilterGraphNoThread, TEXT("CLSID_FilterGraphNoThread\0")},
    {&CLSID_FilterMapper, TEXT("CLSID_FilterMapper\0")},
    {&CLSID_FilterMapper2, TEXT("CLSID_FilterMapper2\0")},
    {&CLSID_InfTee, TEXT("CLSID_InfTee\0")},
    {&CLSID_LegacyAmFilterCategory, TEXT("CLSID_LegacyAmFilterCategory\0")},
    {&CLSID_Line21Decoder, TEXT("CLSID_Line21Decoder\0")},
    {&CLSID_MOVReader, TEXT("CLSID_MOVReader\0")},
    {&CLSID_MPEG1Doc, TEXT("CLSID_MPEG1Doc\0")},
    {&CLSID_MPEG1PacketPlayer, TEXT("CLSID_MPEG1PacketPlayer\0")},
    {&CLSID_MPEG1Splitter, TEXT("CLSID_MPEG1Splitter\0")},
    {&CLSID_MediaPropertyBag, TEXT("CLSID_MediaPropertyBag\0")},
    {&CLSID_MemoryAllocator, TEXT("CLSID_MemoryAllocator\0")},
    {&CLSID_MidiRendererCategory, TEXT("CLSID_MidiRendererCategory\0")},
    {&CLSID_ModexProperties, TEXT("CLSID_ModexProperties\0")},
    {&CLSID_ModexRenderer, TEXT("CLSID_ModexRenderer\0")},
    {&CLSID_OverlayMixer, TEXT("CLSID_OverlayMixer\0")},
    {&CLSID_PerformanceProperties, TEXT("CLSID_PerformanceProperties\0")},
    {&CLSID_PersistMonikerPID, TEXT("CLSID_PersistMonikerPID\0")},
    {&CLSID_ProtoFilterGraph, TEXT("CLSID_ProtoFilterGraph\0")},
    {&CLSID_QualityProperties, TEXT("CLSID_QualityProperties\0")},
    {&CLSID_SeekingPassThru, TEXT("CLSID_SeekingPassThru\0")},
    {&CLSID_SmartTee, TEXT("CLSID_SmartTee\0")},
    {&CLSID_SystemClock, TEXT("CLSID_SystemClock\0")},
    {&CLSID_SystemDeviceEnum, TEXT("CLSID_SystemDeviceEnum\0")},
    {&CLSID_TVAudioFilterPropertyPage, TEXT("CLSID_TVAudioFilterPropertyPage\0")},
    {&CLSID_TVTunerFilterPropertyPage, TEXT("CLSID_TVTunerFilterPropertyPage\0")},
    {&CLSID_TextRender, TEXT("CLSID_TextRender\0")},
    {&CLSID_URLReader, TEXT("CLSID_URLReader\0")},
    {&CLSID_VBISurfaces, TEXT("CLSID_VBISurfaces\0")},
    {&CLSID_VPObject, TEXT("CLSID_VPObject\0")},
    {&CLSID_VPVBIObject, TEXT("CLSID_VPVBIObject\0")},
    {&CLSID_VfwCapture, TEXT("CLSID_VfwCapture\0")},
    {&CLSID_VideoCompressorCategory, TEXT("CLSID_VideoCompressorCategory\0")},
    {&CLSID_VideoInputDeviceCategory, TEXT("CLSID_VideoInputDeviceCategory\0")},
    {&CLSID_VideoProcAmpPropertyPage, TEXT("CLSID_VideoProcAmpPropertyPage\0")},
    {&CLSID_VideoRenderer, TEXT("CLSID_VideoRenderer\0")},
    {&CLSID_VideoStreamConfigPropertyPage, TEXT("CLSID_VideoStreamConfigPropertyPage\0")},

    {&CLSID_WMMUTEX_Language,      TEXT("CLSID_WMMUTEX_Language\0")},
    {&CLSID_WMMUTEX_Bitrate,       TEXT("CLSID_WMMUTEX_Bitrate\0")},
    {&CLSID_WMMUTEX_Presentation,  TEXT("CLSID_WMMUTEX_Presentation\0")},
    {&CLSID_WMMUTEX_Unknown,       TEXT("CLSID_WMMUTEX_Unknown\0")},

    {&CLSID_WMBandwidthSharing_Exclusive, TEXT("CLSID_WMBandwidthSharing_Exclusive\0")},
    {&CLSID_WMBandwidthSharing_Partial,   TEXT("CLSID_WMBandwidthSharing_Partial\0")},

    {&FORMAT_AnalogVideo, TEXT("FORMAT_AnalogVideo\0")},
    {&FORMAT_DVD_LPCMAudio, TEXT("FORMAT_DVD_LPCMAudio\0")},
    {&FORMAT_DolbyAC3, TEXT("FORMAT_DolbyAC3\0")},
    {&FORMAT_DvInfo, TEXT("FORMAT_DvInfo\0")},
    {&FORMAT_MPEG2Audio, TEXT("FORMAT_MPEG2Audio\0")},
    {&FORMAT_MPEG2Video, TEXT("FORMAT_MPEG2Video\0")},
    {&FORMAT_MPEG2_VIDEO, TEXT("FORMAT_MPEG2_VIDEO\0")},
    {&FORMAT_MPEGStreams, TEXT("FORMAT_MPEGStreams\0")},
    {&FORMAT_MPEGVideo, TEXT("FORMAT_MPEGVideo\0")},
    {&FORMAT_None, TEXT("FORMAT_None\0")},
    {&FORMAT_VIDEOINFO2, TEXT("FORMAT_VIDEOINFO2\0")},
    {&FORMAT_VideoInfo, TEXT("FORMAT_VideoInfo\0")},
    {&FORMAT_VideoInfo2, TEXT("FORMAT_VideoInfo2\0")},
    {&FORMAT_WaveFormatEx, TEXT("FORMAT_WaveFormatEx\0")},

    {&TIME_FORMAT_BYTE, TEXT("TIME_FORMAT_BYTE\0")},
    {&TIME_FORMAT_FIELD, TEXT("TIME_FORMAT_FIELD\0")},
    {&TIME_FORMAT_FRAME, TEXT("TIME_FORMAT_FRAME\0")},
    {&TIME_FORMAT_MEDIA_TIME, TEXT("TIME_FORMAT_MEDIA_TIME\0")},
    {&TIME_FORMAT_SAMPLE, TEXT("TIME_FORMAT_SAMPLE\0")},

    {&AMPROPSETID_Pin, TEXT("AMPROPSETID_Pin\0")},
    {&AM_INTERFACESETID_Standard, TEXT("AM_INTERFACESETID_Standard\0")},
    {&AM_KSCATEGORY_AUDIO, TEXT("AM_KSCATEGORY_AUDIO\0")},
    {&AM_KSCATEGORY_CAPTURE, TEXT("AM_KSCATEGORY_CAPTURE\0")},
    {&AM_KSCATEGORY_CROSSBAR, TEXT("AM_KSCATEGORY_CROSSBAR\0")},
    {&AM_KSCATEGORY_DATACOMPRESSOR, TEXT("AM_KSCATEGORY_DATACOMPRESSOR\0")},
    {&AM_KSCATEGORY_RENDER, TEXT("AM_KSCATEGORY_RENDER\0")},
    {&AM_KSCATEGORY_TVAUDIO, TEXT("AM_KSCATEGORY_TVAUDIO\0")},
    {&AM_KSCATEGORY_TVTUNER, TEXT("AM_KSCATEGORY_TVTUNER\0")},
    {&AM_KSCATEGORY_VIDEO, TEXT("AM_KSCATEGORY_VIDEO\0")},
    {&AM_KSPROPSETID_AC3, TEXT("AM_KSPROPSETID_AC3\0")},
    {&AM_KSPROPSETID_CopyProt, TEXT("AM_KSPROPSETID_CopyProt\0")},
    {&AM_KSPROPSETID_DvdSubPic, TEXT("AM_KSPROPSETID_DvdSubPic\0")},
    {&AM_KSPROPSETID_TSRateChange, TEXT("AM_KSPROPSETID_TSRateChange\0")},

    {&IID_IAMDirectSound, TEXT("IID_IAMDirectSound\0")},
    {&IID_IAMLine21Decoder, TEXT("IID_IAMLine21Decoder\0")},
    {&IID_IBaseVideoMixer, TEXT("IID_IBaseVideoMixer\0")},
    {&IID_IDDVideoPortContainer, TEXT("IID_IDDVideoPortContainer\0")},
    {&IID_IDirectDraw, TEXT("IID_IDirectDraw\0")},
    {&IID_IDirectDraw2, TEXT("IID_IDirectDraw2\0")},
    {&IID_IDirectDrawClipper, TEXT("IID_IDirectDrawClipper\0")},
    {&IID_IDirectDrawColorControl, TEXT("IID_IDirectDrawColorControl\0")},
    {&IID_IDirectDrawKernel, TEXT("IID_IDirectDrawKernel\0")},
    {&IID_IDirectDrawPalette, TEXT("IID_IDirectDrawPalette\0")},
    {&IID_IDirectDrawSurface, TEXT("IID_IDirectDrawSurface\0")},
    {&IID_IDirectDrawSurface2, TEXT("IID_IDirectDrawSurface2\0")},
    {&IID_IDirectDrawSurface3, TEXT("IID_IDirectDrawSurface3\0")},
    {&IID_IDirectDrawSurfaceKernel, TEXT("IID_IDirectDrawSurfaceKernel\0")},
    {&IID_IDirectDrawVideo, TEXT("IID_IDirectDrawVideo\0")},
    {&IID_IFullScreenVideo, TEXT("IID_IFullScreenVideo\0")},
    {&IID_IFullScreenVideoEx, TEXT("IID_IFullScreenVideoEx\0")},
    {&IID_IKsDataTypeHandler, TEXT("IID_IKsDataTypeHandler\0")},
    {&IID_IKsInterfaceHandler, TEXT("IID_IKsInterfaceHandler\0")},
    {&IID_IKsPin, TEXT("IID_IKsPin\0")},
    {&IID_IMixerPinConfig, TEXT("IID_IMixerPinConfig\0")},
    {&IID_IMixerPinConfig2, TEXT("IID_IMixerPinConfig2\0")},
    {&IID_IMpegAudioDecoder, TEXT("IID_IMpegAudioDecoder\0")},
    {&IID_IQualProp, TEXT("IID_IQualProp\0")},
    {&IID_IVPConfig, TEXT("IID_IVPConfig\0")},
    {&IID_IVPControl, TEXT("IID_IVPControl\0")},
    {&IID_IVPNotify, TEXT("IID_IVPNotify\0")},
    {&IID_IVPNotify2, TEXT("IID_IVPNotify2\0")},
    {&IID_IVPObject, TEXT("IID_IVPObject\0")},
    {&IID_IVPVBIConfig, TEXT("IID_IVPVBIConfig\0")},
    {&IID_IVPVBINotify, TEXT("IID_IVPVBINotify\0")},
    {&IID_IVPVBIObject, TEXT("IID_IVPVBIObject\0")},

    {&LOOK_DOWNSTREAM_ONLY, TEXT("LOOK_DOWNSTREAM_ONLY\0")},
    {&LOOK_UPSTREAM_ONLY, TEXT("LOOK_UPSTREAM_ONLY\0")},
    {0, 0},
};



HRESULT GetGUIDString(TCHAR *szString, int cchBuffer, GUID *pGUID)
{
    int i=0;
    HRESULT hr = E_FAIL;

	if (cchBuffer < 1)
	{
		return E_INVALIDARG;
	}

	szString[0] = TEXT('\0');

    // Find format GUID's name in the named guids table
    while (rgng[i].pguid != 0)
    {
        if(*pGUID == *(rgng[i].pguid))
        {
            hr = StringCchCat(szString, cchBuffer, rgng[i].psz);
            break;
        }
        i++;
    }

	if (FAILED(hr))
	{
	    // No match, use the string representation of the GUID
		hr = StringFromGUID2(*pGUID, szString, cchBuffer);
	}

	return hr;
}


HRESULT GetFormatString(TCHAR *szFormat, int cchBuffer, DMO_MEDIA_TYPE *pType)
{
	return GetGUIDString(szFormat, cchBuffer, &pType->formattype);
}



HRESULT GetTypeSubtypeString(TCHAR *szString, int cchBuffer, DMO_PARTIAL_MEDIATYPE& aList)
{
    HRESULT hr;

	const size_t TEMP_BUFFER_SIZE = 128;
	TCHAR tmp[TEMP_BUFFER_SIZE];

	hr = GetGUIDString(szString, cchBuffer, &aList.type);

	if (SUCCEEDED(hr))
	{
		hr = StringCchCat(szString, cchBuffer, TEXT("/"));
	}

	if (SUCCEEDED(hr))
	{
		hr = GetGUIDString(tmp, TEMP_BUFFER_SIZE, &aList.subtype);
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCat(szString, cchBuffer, tmp);
	}
	
	return hr;
}


