//------------------------------------------------------------------------------
// File: Types.h
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
// Merit for pins
//
typedef struct _meritinfo
{
    DWORD dwMerit;
    TCHAR szName[64];

} MERITINFO;

// Minimum merit must be specified, so there is no <Don't Care> entry
const MERITINFO merittypes[] = {

    MERIT_HW_COMPRESSOR,        TEXT("Hardware compressor\0"),
    MERIT_SW_COMPRESSOR,        TEXT("Software compressor\0"),
    MERIT_DO_NOT_USE,           TEXT("Do not use\0"),
    MERIT_UNLIKELY,             TEXT("Unlikely\0"),
    MERIT_NORMAL,               TEXT("Normal\0"),
    MERIT_PREFERRED,            TEXT("Preferred\0"),

};

#define NUM_MERIT_TYPES     (sizeof(merittypes) / sizeof(merittypes[0]))

//
// Media types
//
typedef struct _guidinfo
{
    const GUID *pGUID;
    TCHAR szName[64];

} GUIDINFO;

const GUIDINFO pintypes[] = {

    0                           ,TEXT("<Don't care>\0"),
    &PIN_CATEGORY_ANALOGVIDEOIN ,TEXT("Analog video in\0"),
    &PIN_CATEGORY_CAPTURE       ,TEXT("Capture\0"),
    &PIN_CATEGORY_CC            ,TEXT("Closed Captioning (Line21)\0"),
    &PIN_CATEGORY_EDS           ,TEXT("EDS (Line 21)\0"),
    &PIN_CATEGORY_NABTS         ,TEXT("NABTS\0"),
    &PIN_CATEGORY_PREVIEW       ,TEXT("Preview\0"),
    &PIN_CATEGORY_STILL         ,TEXT("Still\0"),
    &PIN_CATEGORY_TELETEXT      ,TEXT("Teletext (CC)\0"),
    &PIN_CATEGORY_TIMECODE      ,TEXT("Timecode\0"),
    &PIN_CATEGORY_VBI           ,TEXT("VBI\0"),
    &PIN_CATEGORY_VIDEOPORT     ,TEXT("VideoPort (connect to Overlay Mixer)\0"),
    &PIN_CATEGORY_VIDEOPORT_VBI ,TEXT("VideoPort VBI\0"),

};

#define NUM_PIN_TYPES       (sizeof(pintypes)   / sizeof(pintypes[0]))

const GUIDINFO majortypes[] = {

    0                           ,TEXT("<Don't care>\0"),  /* No selection */
    &MEDIATYPE_AnalogAudio      ,TEXT("Analog audio\0"), 
    &MEDIATYPE_AnalogVideo      ,TEXT("Analog video\0"),
    &MEDIATYPE_Audio            ,TEXT("Audio\0"),
    &MEDIATYPE_AUXLine21Data    ,TEXT("Line 21 data (CC)\0"),
    &MEDIATYPE_File             ,TEXT("File (CC)\0"),
    &MEDIATYPE_Interleaved      ,TEXT("Interleaved (DV)\0"),
    &MEDIATYPE_LMRT             ,TEXT("LMRT (Obsolete)\0"),
    &MEDIATYPE_Midi             ,TEXT("MIDI\0"),
    &MEDIATYPE_MPEG2_PES        ,TEXT("MPEG2 (DVD)\0"),
    &MEDIATYPE_ScriptCommand    ,TEXT("ScriptCommand (CC)\0"),
    &MEDIATYPE_Stream           ,TEXT("Byte stream (no time stamps)\0"),
    &MEDIATYPE_Text             ,TEXT("Text\0"),
    &MEDIATYPE_Timecode         ,TEXT("Timecode data\0"),
    &MEDIATYPE_URL_STREAM       ,TEXT("URL_STREAM (Obsolete)\0"),
    &MEDIATYPE_Video            ,TEXT("Video\0"),

};

#define NUM_MAJOR_TYPES     (sizeof(majortypes) / sizeof(majortypes[0]))

//
// Media subtypes
//
const GUIDINFO audiosubtypes[] = {

    &MEDIASUBTYPE_PCM           ,TEXT("PCM audio\0"), 
    &MEDIASUBTYPE_MPEG1Packet   ,TEXT("MPEG1 Audio Packet\0"), 
    &MEDIASUBTYPE_MPEG1Payload  ,TEXT("MPEG1 Audio Payload\0"), 
    0, 0
};

const GUIDINFO line21subtypes[] = {

    &MEDIASUBTYPE_Line21_BytePair       ,TEXT("BytePairs\0"),
    &MEDIASUBTYPE_Line21_GOPPacket      ,TEXT("DVD GOP Packet\0"),
    &MEDIASUBTYPE_Line21_VBIRawData     ,TEXT("VBI Raw Data\0"),
    0, 0
};

const GUIDINFO mpeg2subtypes[] = {

    &MEDIASUBTYPE_DVD_SUBPICTURE        ,TEXT("DVD Subpicture\0"),
    &MEDIASUBTYPE_DVD_LPCM_AUDIO        ,TEXT("DVD Audio (LPCM)\0"),
    &MEDIASUBTYPE_DOLBY_AC3             ,TEXT("Dolby AC3\0"),
    &MEDIASUBTYPE_MPEG2_AUDIO           ,TEXT("MPEG-2 Audio\0"),
    &MEDIASUBTYPE_MPEG2_TRANSPORT       ,TEXT("MPEG-2 Transport Stream\0"),
    &MEDIASUBTYPE_MPEG2_PROGRAM         ,TEXT("MPEG-2 Program Stream\0"),
    0, 0
};

const GUIDINFO streamsubtypes[] = {

    &MEDIASUBTYPE_AIFF              ,TEXT("AIFF\0"),
    &MEDIASUBTYPE_Asf               ,TEXT("ASF\0"),
    &MEDIASUBTYPE_Avi               ,TEXT("AVI\0"),
    &MEDIASUBTYPE_AU                ,TEXT("AU\0"),
    &MEDIASUBTYPE_DssAudio          ,TEXT("DSS Audio\0"),
    &MEDIASUBTYPE_DssVideo          ,TEXT("DSS Video\0"),
    &MEDIASUBTYPE_MPEG1Audio        ,TEXT("MPEG1 Audio\0"),
    &MEDIASUBTYPE_MPEG1System       ,TEXT("MPEG1 System\0"),
    &MEDIASUBTYPE_MPEG1Video        ,TEXT("MPEG1 Video\0"),
    &MEDIASUBTYPE_MPEG1VideoCD      ,TEXT("MPEG1 VideoCD\0"),
    &MEDIASUBTYPE_WAVE              ,TEXT("Wave\0"),
    0, 0
};

const GUIDINFO videosubtypes[] = {

    &MEDIASUBTYPE_YVU9              ,TEXT("YVU9\0"),
    &MEDIASUBTYPE_Y411              ,TEXT("YUV 411\0"),
    &MEDIASUBTYPE_Y41P              ,TEXT("Y41P\0"),
    &MEDIASUBTYPE_YUY2              ,TEXT("YUY2\0"),
    &MEDIASUBTYPE_YVYU              ,TEXT("YVYU\0"),
    &MEDIASUBTYPE_UYVY              ,TEXT("UYVY\0"),
    &MEDIASUBTYPE_Y211              ,TEXT("YUV 211\0"),
    &MEDIASUBTYPE_CLJR              ,TEXT("Cirrus YUV 411\0"),
    &MEDIASUBTYPE_IF09              ,TEXT("Indeo YVU9\0"),
    &MEDIASUBTYPE_CPLA              ,TEXT("Cinepak UYVY\0"),
    &MEDIASUBTYPE_MJPG              ,TEXT("Motion JPEG\0"),
    &MEDIASUBTYPE_TVMJ              ,TEXT("TrueVision MJPG\0"),
    &MEDIASUBTYPE_WAKE              ,TEXT("MJPG (Wake)\0"),
    &MEDIASUBTYPE_CFCC              ,TEXT("MJPG (CFCC)\0"),
    &MEDIASUBTYPE_IJPG              ,TEXT("Intergraph JPEG\0"),
    &MEDIASUBTYPE_Plum              ,TEXT("Plum MJPG\0"),
    &MEDIASUBTYPE_RGB1              ,TEXT("RGB1 (Palettized)\0"),
    &MEDIASUBTYPE_RGB4              ,TEXT("RGB4 (Palettized)\0"),
    &MEDIASUBTYPE_RGB8              ,TEXT("RGB8 (Palettized)\0"),
    &MEDIASUBTYPE_RGB565            ,TEXT("RGB565\0"),
    &MEDIASUBTYPE_RGB555            ,TEXT("RGB555\0"),
    &MEDIASUBTYPE_RGB24             ,TEXT("RGB24\0"),
    &MEDIASUBTYPE_RGB32             ,TEXT("RGB32\0"),
    &MEDIASUBTYPE_ARGB32            ,TEXT("ARGB32\0"),
    &MEDIASUBTYPE_Overlay           ,TEXT("Overlay video (from HW)\0"),
    &MEDIASUBTYPE_QTMovie           ,TEXT("Apple QuickTime\0"),
    &MEDIASUBTYPE_QTRpza            ,TEXT("QuickTime RPZA\0"),
    &MEDIASUBTYPE_QTSmc             ,TEXT("QuickTime SMC\0"),
    &MEDIASUBTYPE_QTRle             ,TEXT("QuickTime RLE\0"),
    &MEDIASUBTYPE_QTJpeg            ,TEXT("QuickTime JPEG\0"),
    &MEDIASUBTYPE_dvsd              ,TEXT("Standard DV\0"),
    &MEDIASUBTYPE_dvhd              ,TEXT("High Definition DV\0"),
    &MEDIASUBTYPE_dvsl              ,TEXT("Long Play DV\0"),
    &MEDIASUBTYPE_MPEG1Packet       ,TEXT("MPEG1 Video Packet\0"),
    &MEDIASUBTYPE_MPEG1Payload      ,TEXT("MPEG1 Video Payload\0"),
    &MEDIASUBTYPE_VPVideo           ,TEXT("Video port video\0"),
    &MEDIASUBTYPE_VPVBI             ,TEXT("Video port VBI\0"),
    0, 0
};

const GUIDINFO analogvideosubtypes[] = {

    &MEDIASUBTYPE_AnalogVideo_NTSC_M   ,TEXT("(M) NTSC\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_B    ,TEXT("(B) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_D    ,TEXT("(D) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_G    ,TEXT("(G) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_H    ,TEXT("(H) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_I    ,TEXT("(I) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_M    ,TEXT("(M) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_PAL_N    ,TEXT("(N) PAL\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_B  ,TEXT("(B) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_D  ,TEXT("(D) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_G  ,TEXT("(G) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_H  ,TEXT("(H) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_K  ,TEXT("(K) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_K1 ,TEXT("(K1) SECAM\0"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_L  ,TEXT("(L) SECAM\0"),
    0, 0
};


const GUIDINFO *pSubTypes[] = {

    audiosubtypes,      // Analog audio
    analogvideosubtypes,// Analog video
    audiosubtypes,      // Audio
    line21subtypes,     // Line21 data
    NULL,               // File. Used by closed captions
    NULL,               // Interleaved.  Used by Digital Video (DV)
    NULL,               // Obsolete. Do not use.
    NULL,               // MIDI format
    mpeg2subtypes,      // MPEG-2.  Used by DVD.
    NULL,               // Script command, used by closed captions
    streamsubtypes,     // Byte stream with no time stamps
    NULL,               // Text
    NULL,               // Timecode data
    NULL,               // Obsolete.  Do not use.
    videosubtypes,      // Video
};

