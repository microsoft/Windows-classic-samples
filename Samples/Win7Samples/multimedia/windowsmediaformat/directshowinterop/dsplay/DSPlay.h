//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DSPlay.h
//
// Abstract:            Windows Media / DirectShow sample code
//                      Header for DSPlay sample application
//
//*****************************************************************************

//
// Function prototypes
//
HRESULT InitPlayerWindow(void);
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT HandleGraphEvent(void);

BOOL GetClipFileName(__out_ecount(MAX_PATH) LPTSTR szName);

void PaintAudioWindow(void);
void MoveVideoWindow(void);
void CheckVisibility(void);
void CloseInterfaces(void);
void CheckSizeMenu(WPARAM wParam);
void EnablePlaybackMenu(BOOL bEnable, int nMediaType);

void OpenClip(void);
void PauseClip(void);
void StopClip(void);
void CloseClip(void);

void UpdateMainTitle(void);
void GetFilename(__in LPCTSTR pszFull, __out_ecount(MAX_PATH) LPTSTR pszFile);
void Msg(__in LPCTSTR szFormat, ...);

//
// Constants
//
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

// File filter for OpenFile dialog
#define FILE_FILTER_TEXT \
    TEXT("Windows Media Files (*.asf; *.wma; *.wmv)\0*.asf; *.wma; *.wmv\0") \
    TEXT("All Files (*.*)\0*.*;\0\0")

// Begin default media search at root directory
#define DEFAULT_MEDIA_PATH  TEXT("\\\0")

// Defaults used with audio-only files
#define DEFAULT_AUDIO_WIDTH     240
#define DEFAULT_AUDIO_HEIGHT    120
#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    240
#define MINIMUM_VIDEO_WIDTH     200
#define MINIMUM_VIDEO_HEIGHT    120

#define APPLICATIONNAME TEXT("DSPlay Sample\0")
#define CLASSNAME       TEXT("DSPlayWMSample\0")

#define WM_GRAPHNOTIFY  WM_USER+20

enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr);}

