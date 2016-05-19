// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: PlayCap.h
//
// Desc: DirectShow sample code - header file for PlayCap previewer
//       application.
//------------------------------------------------------------------------------


//
// Function prototypes
//
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndMainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT GetInterfaces(void);
HRESULT CaptureVideo();
HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);
HRESULT SetupVideoWindow(void);
HRESULT ChangePreviewState(int nShow);
HRESULT HandleGraphEvent(void);

void Msg(TCHAR *szFormat, ...);
void CloseInterfaces(void);
void ResizeVideoWindow(void);

// Remote graph viewing functions
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void RemoveGraphFromRot(DWORD pdwRegister);

enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

//
// Constants
//
#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    320

#define APPLICATIONNAME TEXT("Video Capture Previewer (PlayCap)\0")
#define CLASSNAME       TEXT("VidCapPreviewer\0")

// Application-defined message to notify app of filtergraph events
#define WM_GRAPHNOTIFY  WM_APP+1

