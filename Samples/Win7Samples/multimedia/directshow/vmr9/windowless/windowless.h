//------------------------------------------------------------------------------
// File: Windowless.h
//
// Desc: DirectShow sample code - header file for video in window movie
//       player application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Function prototypes
//
HRESULT InitPlayerWindow(void);
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT HandleGraphEvent(void);
HRESULT StepOneFrame(void);
HRESULT StepFrames(int nFramesToStep);
HRESULT ModifyRate(double dRateAdjust);
HRESULT SetRate(double dRate);

BOOL GetFrameStepInterface(void);
BOOL GetClipFileName(LPTSTR szName);

void PaintAudioWindow(void);
void MoveVideoWindow(void);
void CheckVisibility(void);
void CloseInterfaces(void);

void OpenClip(void);
void PauseClip(void);
void StopClip(void);
void CloseClip(void);
void OnPaint(HWND hwnd);

void UpdateMainTitle(void);
void CheckSizeMenu(WPARAM wParam);
void EnablePlaybackMenu(BOOL bEnable, int nMediaType);
void GetFilename(TCHAR *pszFull, TCHAR *pszFile);
void Msg(TCHAR *szFormat, ...);

HRESULT InitializeWindowlessVMR(IBaseFilter **ppVmr9);
void DisplayCapturedImage(LPCTSTR szFile);
BOOL CaptureImage(LPCTSTR szFile);

BOOL VerifyVMR9(void);

// Macros
#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))

//
// Constants
//
#define CAPTURED_IMAGE_NAME		TEXT("VMRImage.bmp\0")
#define BFT_BITMAP 0x4d42   /* 'BM' */

#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

// File filter for OpenFile dialog
#define FILE_FILTER_TEXT \
    TEXT("Video Files (*.asf; *.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv)\0*.asf; *.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv\0")\
    TEXT("Audio files (*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd)\0*.wav; *.mpa; *.mp2; *.mp3; *.au; *.aif; *.aiff; *.snd\0")\
    TEXT("MIDI Files (*.mid, *.midi, *.rmi)\0*.mid; *.midi; *.rmi\0") \
    TEXT("Image Files (*.jpg, *.bmp, *.gif, *.tga)\0*.jpg; *.bmp; *.gif; *.tga\0") \
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

#define APPLICATIONNAME TEXT("Windowless9 Player\0")
#define CLASSNAME       TEXT("VMR9WindowlessPlayer\0")

#define WM_GRAPHNOTIFY  WM_USER+13

enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr);}

//
// Resource constants
//
#define IDR_MENU                        101
#define IDD_ABOUTBOX                    200
#define ID_FILE_OPENCLIP                40001
#define ID_FILE_EXIT                    40002
#define ID_FILE_PAUSE                   40003
#define ID_FILE_STOP                    40004
#define ID_FILE_CLOSE                   40005
#define ID_FILE_MUTE                    40006
#define ID_FILE_FULLSCREEN              40007
#define ID_FILE_SIZE_NORMAL             40008
#define ID_FILE_SIZE_HALF               40009
#define ID_FILE_SIZE_DOUBLE             40010
#define ID_FILE_SIZE_QUARTER            40011
#define ID_FILE_SIZE_THREEQUARTER       40012
#define ID_HELP_ABOUT                   40014
#define ID_RATE_INCREASE                40020
#define ID_RATE_DECREASE                40021
#define ID_RATE_NORMAL                  40022
#define ID_RATE_DOUBLE                  40023
#define ID_RATE_HALF                    40024
#define ID_SINGLE_STEP                  40025
#define ID_CAPTURE_IMAGE				40030
#define ID_DISPLAY_IMAGE				40031

