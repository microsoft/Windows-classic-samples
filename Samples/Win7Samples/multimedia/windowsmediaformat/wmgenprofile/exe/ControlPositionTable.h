//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ControlPositoinTable.h
//
// Abstract:            Contains the tables WINDOW_POSITION and
//                      STREAM_WINDOW_POSITION, which are used to correctly
//                      position the control on the dialog.
//
//*****************************************************************************

#ifndef __CONTROLPOSITIONTABLE_H__
#define __CONTROLPOSITIONTABLE_H__

#include "resource.h"

#define NUM_CONFIGS 5
#define NUM_MOVABLE_CONTROLS 26


#define WINCONFIG_NONE 0
#define WINCONFIG_MUTEX    1
#define WINCONFIG_BANDWIDTHSHARING 2
#define WINCONFIG_STREAMPRIORITIZATION 3
#define WINCONFIG_STREAM 4

struct WindowPlacement
{
    DWORD dwControl;
    BOOL fVisible;
    int nX;
    int nY;
    int nWidth;
    int nHeight;
};

const WindowPlacement WINDOW_POSITION[NUM_CONFIGS][NUM_MOVABLE_CONTROLS] =
{
    { // Hide all
        { IDC_FRAMutexStreams,          FALSE,  130,    7,  115,    135  },
        { IDC_LSTMutexStreams,          FALSE,  133,   20,  109,    117  },
        { IDC_FRAMutexType,             FALSE,  249,    7,  116,     55  },
        { IDC_RBMutexTypeBitrate,       FALSE,  253,   17,  106,     11  },
        { IDC_RBMutexTypeLanguage,      FALSE,  253,   17,  106,     31  },
        { IDC_RBMutexTypePresentation,  FALSE,  253,   17,  106,     44  },

        { IDC_FRABandwidthStreams,      FALSE,  180,    7,  115,    135  },
        { IDC_LSTSharingStreams,        FALSE,  183,   20,  109,    117  },
        { IDC_LBLSharedBitrate,         FALSE,  180,  150,   50,     20  },
        { IDC_TXTSharedBitrate,         FALSE,  250,  150,  100,     20  },
        { IDC_FRABandwidthType,         FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypeExclusive, FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypePartial,   FALSE,  250,  150,  100,     20  },
        { IDC_LBLBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },
        { IDC_TXTBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },

        { IDC_FRAStreamPrioritization,  FALSE,  180,    7,  115,    135  },
        { IDC_LSTPrioritizationStreams, FALSE,  183,   20,  109,    117  },
        { IDC_BTNPrioritizationUp,      FALSE,  133,  125,   50,     20  },
        { IDC_BTNPrioritizationDown,    FALSE,  188,  125,   50,     20  },
        { IDC_LSTMandatoryStreams,      FALSE,  250,    7,  116,     55  },
        { IDC_FRAMandatoryStreams,      FALSE,  253,   17,  106,     45  },

        { IDC_LBLStreamBufferWindow,    FALSE,  180,   52,   50,     20  },
        { IDC_TXTStreamBufferWindow,    FALSE,  250,   50,   50,     20  },
        { IDC_LBLStreamType,            FALSE,  250,   50,   50,     20  },
        { IDC_CBStreamType,             FALSE,  250,   50,   50,     20  },
        { IDC_CHKSMPTE,                 FALSE,  460,    7,  100,     20  },
    },
    { // Mutex
        { IDC_FRAMutexStreams,           TRUE,  180,   11,  115,    219  },
        { IDC_LSTMutexStreams,           TRUE,  183,   26,  109,    200  },
        { IDC_FRAMutexType,              TRUE,  300,   11,  116,    219  },
        { IDC_RBMutexTypeBitrate,        TRUE,  303,   29,  106,     20  },
        { IDC_RBMutexTypeLanguage,       TRUE,  303,   49,  106,     20  },
        { IDC_RBMutexTypePresentation,   TRUE,  303,   69,  106,     20  },

        { IDC_FRABandwidthStreams,      FALSE,  180,    7,  115,    135  },
        { IDC_LSTSharingStreams,        FALSE,  183,   20,  109,    117  },
        { IDC_LBLSharedBitrate,         FALSE,  180,  150,   50,     20  },
        { IDC_TXTSharedBitrate,         FALSE,  250,  150,  100,     20  },
        { IDC_FRABandwidthType,         FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypeExclusive, FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypePartial,   FALSE,  250,  150,  100,     20  },
        { IDC_LBLBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },
        { IDC_TXTBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },

        { IDC_FRAStreamPrioritization,  FALSE,  180,    7,  115,    135  },
        { IDC_LSTPrioritizationStreams, FALSE,  183,   20,  109,    117  },
        { IDC_BTNPrioritizationUp,      FALSE,  133,  125,   50,     20  },
        { IDC_BTNPrioritizationDown,    FALSE,  188,  125,   50,     20  },
        { IDC_LSTMandatoryStreams,      FALSE,  250,    7,  116,     55  },
        { IDC_FRAMandatoryStreams,      FALSE,  253,   17,  106,     45  },

        { IDC_LBLStreamBufferWindow,    FALSE,  180,   52,   50,     20  },
        { IDC_TXTStreamBufferWindow,    FALSE,  250,   50,   50,     20  },
        { IDC_LBLStreamType,            FALSE,  250,   50,   50,     20  },
        { IDC_CBStreamType,             FALSE,  250,   50,   50,     20  },
        { IDC_CHKSMPTE,                 FALSE,  460,    7,  100,     20  },
    },
    { // Bandwidth sharing
        { IDC_FRAMutexStreams,          FALSE,  130,    7,  115,    135  },
        { IDC_LSTMutexStreams,          FALSE,  133,   17,  109,    117  },
        { IDC_FRAMutexType,             FALSE,  249,    7,  116,     55  },
        { IDC_RBMutexTypeBitrate,       FALSE,  253,   17,  106,     11  },
        { IDC_RBMutexTypeLanguage,      FALSE,  253,   17,  106,     31  },
        { IDC_RBMutexTypePresentation,  FALSE,  253,   17,  106,     44  },

        { IDC_FRABandwidthStreams,       TRUE,  180,   11,  200,    156  },
        { IDC_LSTSharingStreams,         TRUE,  183,   26,  194,    136  },
        { IDC_LBLSharedBitrate,          TRUE,  200,  177,   75,     20  },
        { IDC_TXTSharedBitrate,          TRUE,  280,  175,   60,     20  },
        { IDC_FRABandwidthType,          TRUE,  385,   11,  116,    219  },
        { IDC_RBBandwidthTypeExclusive,  TRUE,  388,   31,  106,     20  },
        { IDC_RBBandwidthTypePartial,    TRUE,  388,   51,  106,     20  },
        { IDC_LBLBandwidthBufferWindow,  TRUE,  200,  207,   75,     20  },
        { IDC_TXTBandwidthBufferWindow,  TRUE,  280,  205,   60,     20  },

        { IDC_FRAStreamPrioritization,  FALSE,  180,    7,  115,    135  },
        { IDC_LSTPrioritizationStreams, FALSE,  183,   17,  109,    117  },
        { IDC_BTNPrioritizationUp,      FALSE,  133,  125,   50,     20  },
        { IDC_BTNPrioritizationDown,    FALSE,  188,  125,   50,     20  },
        { IDC_LSTMandatoryStreams,      FALSE,  250,    7,  116,     55  },
        { IDC_FRAMandatoryStreams,      FALSE,  253,   17,  106,     45  },

        { IDC_LBLStreamBufferWindow,    FALSE,  180,   52,   50,     20  },
        { IDC_TXTStreamBufferWindow,    FALSE,  250,   50,   50,     20  },
        { IDC_LBLStreamType,            FALSE,  250,   50,   50,     20  },
        { IDC_CBStreamType,             FALSE,  250,   50,   50,     20  },
        { IDC_CHKSMPTE,                 FALSE,  460,    7,  100,     20  },
    },
    { // Stream prioritization
        { IDC_FRAMutexStreams,          FALSE,  130,    7,  115,    135  },
        { IDC_LSTMutexStreams,          FALSE,  133,   17,  109,    117  },
        { IDC_FRAMutexType,             FALSE,  249,    7,  116,     55  },
        { IDC_RBMutexTypeBitrate,       FALSE,  253,   17,  106,     11  },
        { IDC_RBMutexTypeLanguage,      FALSE,  253,   17,  106,     31  },
        { IDC_RBMutexTypePresentation,  FALSE,  253,   17,  106,     44  },

        { IDC_FRABandwidthStreams,      FALSE,  180,    7,  115,    135  },
        { IDC_LSTSharingStreams,        FALSE,  183,   17,  109,    117  },
        { IDC_LBLSharedBitrate,         FALSE,  180,  150,   50,     20  },
        { IDC_TXTSharedBitrate,         FALSE,  250,  150,  100,     20  },
        { IDC_FRABandwidthType,         FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypeExclusive, FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypePartial,   FALSE,  250,  150,  100,     20  },
        { IDC_LBLBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },
        { IDC_TXTBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },

        { IDC_FRAStreamPrioritization,   TRUE,  180,   11,  115,    219  },
        { IDC_LSTPrioritizationStreams,  TRUE,  183,   26,  109,    166  },
        { IDC_BTNPrioritizationUp,       TRUE,  184,  197,   53,     29  },
        { IDC_BTNPrioritizationDown,     TRUE,  238,  197,   53,     29  },
        { IDC_FRAMandatoryStreams,       TRUE,  300,   11,  115,    219  },
        { IDC_LSTMandatoryStreams,       TRUE,  303,   26,  109,    200  },

        { IDC_LBLStreamBufferWindow,    FALSE,  180,   52,   50,     20  },
        { IDC_TXTStreamBufferWindow,    FALSE,  250,   50,   50,     20  },
        { IDC_LBLStreamType,            FALSE,  250,   50,   50,     20  },
        { IDC_CBStreamType,             FALSE,  250,   50,   50,     20  },
        { IDC_CHKSMPTE,                 FALSE,  460,    7,  100,     20  },
    },
    { // Stream
        { IDC_FRAMutexStreams,          FALSE,  130,    7,  115,    135  },
        { IDC_LSTMutexStreams,          FALSE,  133,   17,  109,    117  },
        { IDC_FRAMutexType,             FALSE,  249,    7,  116,     55  },
        { IDC_RBMutexTypeBitrate,       FALSE,  253,   17,  106,     11  },
        { IDC_RBMutexTypeLanguage,      FALSE,  253,   17,  106,     31  },
        { IDC_RBMutexTypePresentation,  FALSE,  253,   17,  106,     44  },

        { IDC_FRABandwidthStreams,      FALSE,  180,    7,  115,    135  },
        { IDC_LSTSharingStreams,        FALSE,  183,   17,  109,    117  },
        { IDC_LBLSharedBitrate,         FALSE,  180,  150,   50,     20  },
        { IDC_TXTSharedBitrate,         FALSE,  250,  150,  100,     20  },
        { IDC_FRABandwidthType,         FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypeExclusive, FALSE,  250,  150,  100,     20  },
        { IDC_RBBandwidthTypePartial,   FALSE,  250,  150,  100,     20  },
        { IDC_LBLBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },
        { IDC_TXTBandwidthBufferWindow, FALSE,  250,  150,  100,     20  },

        { IDC_FRAStreamPrioritization,  FALSE,  180,    7,  115,    135  },
        { IDC_LSTPrioritizationStreams, FALSE,  183,   17,  109,    117  },
        { IDC_BTNPrioritizationUp,      FALSE,  133,  125,   50,     20  },
        { IDC_BTNPrioritizationDown,    FALSE,  188,  125,   50,     20  },
        { IDC_LSTMandatoryStreams,      FALSE,  250,    7,  116,     55  },
        { IDC_FRAMandatoryStreams,      FALSE,  253,   17,  106,     45  },

        { IDC_LBLStreamType,             TRUE,  190,   21,   80,     20  },
        { IDC_CBStreamType,              TRUE,  270,   18,  100,     20  },
        { IDC_LBLStreamBufferWindow,     TRUE,  190,   47,   80,     20  },
        { IDC_TXTStreamBufferWindow,     TRUE,  270,   45,   50,     20  },
        { IDC_CHKSMPTE,                  TRUE,  380,   19,  100,     20  },
    }
};


#define NUM_STREAMTYPES 7
#define NUM_STREAM_CONTROLS 28

#define WINSTREAMCONFIG_NONE 0
#define WINSTREAMCONFIG_AUDIO 1
#define WINSTREAMCONFIG_VIDEO 2
#define WINSTREAMCONFIG_SCRIPT 3
#define WINSTREAMCONFIG_IMAGE 4
#define WINSTREAMCONFIG_WEB 5
#define WINSTREAMCONFIG_FILE 6

const WindowPlacement STREAM_WINDOW_POSITION[NUM_STREAMTYPES][NUM_STREAM_CONTROLS] =
{
    { // None
        { IDC_LBLStreamCodec,                   FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamCodec,                    FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamFormat,                  FALSE,  190,     99,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                 FALSE,  190,     99,    50,      20 },
        { IDC_TXTStreamBitrate,                 FALSE,  270,     97,    50,      20 },
        { IDC_LBLStreamVideoWidth,              FALSE,  180,    132,     50,     20 },
        { IDC_TXTStreamVideoWidth,              FALSE,  180,    130,     50,     20 },
        { IDC_LBLStreamVideoHeight,             FALSE,  180,    162,     50,     20 },
        { IDC_TXTStreamVideoHeight,             FALSE,  180,    160,     50,     20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  180,    192,     50,     20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    192,     50,     20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    222,     50,     20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    222,     50,     20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     92,     50,     20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     90,     50,     20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    120,     50,     20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    120,     50,     20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    150,     50,     20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    150,     50,     20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    180,     50,     20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    180,     50,     20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    180,     50,     20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    180,     50,     20 },
        { IDC_CHKUncompressed,                  FALSE,  340,     55,    100,     20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     53,     80,    120 },
        { IDC_LBLLanguage,                      FALSE,  190,    232,     80,     20 },
        { IDC_CBLanguage,                       FALSE,  270,    230,    250,    120 },
    },
    { // Audio
        { IDC_LBLStreamCodec,                    TRUE,  190,     74,    80,      20 },
        { IDC_CBStreamCodec,                     TRUE,  270,     72,   250,      20 },
        { IDC_LBLStreamFormat,                   TRUE,  190,     99,    80,      20 },
        { IDC_CBStreamFormat,                    TRUE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                 FALSE,  190,     99,    50,      20 },
        { IDC_TXTStreamBitrate,                 FALSE,  270,     97,    50,      20 },
        { IDC_LBLStreamVideoWidth,              FALSE,  180,    124,    50,      20 },
        { IDC_TXTStreamVideoWidth,              FALSE,  180,    122,    50,      20 },
        { IDC_LBLStreamVideoHeight,             FALSE,  180,    154,    50,      20 },
        { IDC_TXTStreamVideoHeight,             FALSE,  180,    152,    50,      20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  180,    184,    50,      20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    184,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    214,    50,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    214,    50,      20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     84,    50,      20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     82,    50,      20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    112,    50,      20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    112,    50,      20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    142,    50,      20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    142,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    172,    50,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    172,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    172,    50,      20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    172,    50,      20 },
        { IDC_CHKUncompressed,                   TRUE,  340,     47,   100,      20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     45,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,    124,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,    122,   250,     120 },
    },
    { // Video
        { IDC_LBLStreamFormat,                  FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamCodec,                    TRUE,  190,     74,    80,      20 },
        { IDC_CBStreamCodec,                     TRUE,  270,     72,   250,      20 },
        { IDC_LBLStreamBitrate,                  TRUE,  190,     99,    50,      20 },
        { IDC_TXTStreamBitrate,                  TRUE,  270,     97,    50,      20 },
        { IDC_LBLStreamVideoQuality,             TRUE,  330,     99,    50,      20 },
        { IDC_TXTStreamVideoQuality,             TRUE,  400,     97,    50,      20 },
        { IDC_LBLStreamVideoWidth,               TRUE,  190,    124,    50,      20 },
        { IDC_TXTStreamVideoWidth,               TRUE,  270,    122,    50,      20 },
        { IDC_LBLStreamVideoHeight,              TRUE,  190,    149,    50,      20 },
        { IDC_TXTStreamVideoHeight,              TRUE,  270,    147,    50,      20 },
        { IDC_LBLStreamVideoFPS,                 TRUE,  190,    174,    50,      20 },
        { IDC_TXTStreamVideoFPS,                 TRUE,  270,    172,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,     TRUE,  190,    199,    90,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe,  TRUE,  270,    197,    50,      20 },
        { IDC_CHKStreamVideoVBR,                 TRUE,  330,    124,    50,      20 },
        { IDC_CBStreamVideoVBRMode,              TRUE,  400,    122,   100,      20 },
        { IDC_LBLStreamVideoMaxBitrate,          TRUE,  340,    149,    70,      20 },
        { IDC_TXTStreamVideoMaxBitrate,          TRUE,  430,    147,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,     TRUE,  340,    174,    70,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,     TRUE,  430,    172,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,          TRUE,  340,    199,    70,      20 },
        { IDC_TXTStreamVideoVBRQuality,          TRUE,  430,    197,    50,      20 },
        { IDC_CHKUncompressed,                   TRUE,  340,     47,   100,      20 },
        { IDC_CBPixelFormat,                     TRUE,  440,     45,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,    224,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,    222,   250,     120 },
    },
    { // Script
        { IDC_LBLStreamFormat,                  FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamCodec,                   FALSE,  190,     99,    80,      20 },
        { IDC_CBStreamCodec,                    FALSE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                  TRUE,  190,     74,    50,      20 },
        { IDC_TXTStreamBitrate,                  TRUE,  270,     72,    50,      20 },
        { IDC_LBLStreamVideoWidth,              FALSE,  180,    122,    50,      20 },
        { IDC_TXTStreamVideoWidth,              FALSE,  235,    120,    50,      20 },
        { IDC_LBLStreamVideoHeight,             FALSE,  180,    152,    50,      20 },
        { IDC_TXTStreamVideoHeight,             FALSE,  235,    150,    50,      20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  180,    182,    50,      20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    182,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    212,    50,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    212,    50,      20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     92,    50,      20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     90,    50,      20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    120,    50,      20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    120,    50,      20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    150,    50,      20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    150,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    180,    50,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    180,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_CHKUncompressed,                  FALSE,  340,     55,   100,      20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     53,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,     99,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,     97,   250,     120 },
    },
    { // Image
        { IDC_LBLStreamFormat,                  FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamCodec,                   FALSE,  190,     99,    80,      20 },
        { IDC_CBStreamCodec,                    FALSE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                  TRUE,  190,     74,    50,      20 },
        { IDC_TXTStreamBitrate,                  TRUE,  270,     72,    50,      20 },
        { IDC_LBLStreamVideoWidth,               TRUE,  190,     99,    50,      20 },
        { IDC_TXTStreamVideoWidth,               TRUE,  270,     97,    50,      20 },
        { IDC_LBLStreamVideoHeight,              TRUE,  190,    124,    50,      20 },
        { IDC_TXTStreamVideoHeight,              TRUE,  270,    122,    50,      20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  190,    172,    50,      20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    172,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    212,    50,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    212,    50,      20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     92,    50,      20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     90,    50,      20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    120,    50,      20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    120,    50,      20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    150,    50,      20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    150,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    180,    50,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    180,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_CHKUncompressed,                  FALSE,  340,     55,   100,      20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     53,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,    149,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,    147,   250,     120 },
    },
    { // Web
        { IDC_LBLStreamFormat,                  FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamCodec,                   FALSE,  190,     99,    80,      20 },
        { IDC_CBStreamCodec,                    FALSE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                  TRUE,  190,     74,    50,      20 },
        { IDC_TXTStreamBitrate,                  TRUE,  270,     72,    50,      20 },
        { IDC_LBLStreamVideoWidth,              FALSE,  180,    122,    50,      20 },
        { IDC_TXTStreamVideoWidth,              FALSE,  235,    120,    50,      20 },
        { IDC_LBLStreamVideoHeight,             FALSE,  180,    152,    50,      20 },
        { IDC_TXTStreamVideoHeight,             FALSE,  235,    150,    50,      20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  180,    182,    50,      20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    182,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    212,    50,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    212,    50,      20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     92,    50,      20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     90,    50,      20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    120,    50,      20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    120,    50,      20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    150,    50,      20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    150,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    180,    50,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    180,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_CHKUncompressed,                  FALSE,  340,     55,   100,      20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     53,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,     99,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,     97,   250,     120 },
    },
    { // File
        { IDC_LBLStreamFormat,                  FALSE,  190,     74,    80,      20 },
        { IDC_CBStreamFormat,                   FALSE,  270,     72,   250,      20 },
        { IDC_LBLStreamCodec,                   FALSE,  190,     99,    80,      20 },
        { IDC_CBStreamCodec,                    FALSE,  270,     97,   250,      20 },
        { IDC_LBLStreamBitrate,                  TRUE,  190,     74,    50,      20 },
        { IDC_TXTStreamBitrate,                  TRUE,  270,     72,    50,      20 },
        { IDC_LBLStreamVideoWidth,              FALSE,  180,    122,    50,      20 },
        { IDC_TXTStreamVideoWidth,              FALSE,  235,    120,    50,      20 },
        { IDC_LBLStreamVideoHeight,             FALSE,  180,    152,    50,      20 },
        { IDC_TXTStreamVideoHeight,             FALSE,  235,    150,    50,      20 },
        { IDC_LBLStreamVideoFPS,                FALSE,  180,    182,    50,      20 },
        { IDC_TXTStreamVideoFPS,                FALSE,  235,    182,    50,      20 },
        { IDC_LBLStreamVideoSecsPerKeyframe,    FALSE,  180,    212,    50,      20 },
        { IDC_TXTStreamVideoSecondsPerKeyframe, FALSE,  235,    212,    50,      20 },
        { IDC_LBLStreamVideoQuality,            FALSE,  400,     92,    50,      20 },
        { IDC_TXTStreamVideoQuality,            FALSE,  460,     90,    50,      20 },
        { IDC_CHKStreamVideoVBR,                FALSE,  460,    120,    50,      20 },
        { IDC_CBStreamVideoVBRMode,             FALSE,  460,    120,    50,      20 },
        { IDC_LBLStreamVideoMaxBitrate,         FALSE,  300,    150,    50,      20 },
        { IDC_TXTStreamVideoMaxBitrate,         FALSE,  360,    150,    50,      20 },
        { IDC_CHKStreamVideoMaxBufferWindow,    FALSE,  300,    180,    50,      20 },
        { IDC_TXTStreamVideoMaxBufferWindow,    FALSE,  360,    180,    50,      20 },
        { IDC_LBLStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_TXTStreamVideoVBRQuality,         FALSE,  360,    180,    50,      20 },
        { IDC_CHKUncompressed,                  FALSE,  340,     55,   100,      20 },
        { IDC_CBPixelFormat,                    FALSE,  440,     53,    80,     120 },
        { IDC_LBLLanguage,                       TRUE,  190,     99,    80,      20 },
        { IDC_CBLanguage,                        TRUE,  270,     97,   250,     120 },
    },
};



#endif // __CONTROLPOSITIONTABLE_H__