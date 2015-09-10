//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 7 or later.
#define WINVER 0x0800       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 7 or later.
#define _WIN32_WINNT 0x0800 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE
#define UNICODE
#endif

// DirectX header files.
#include <d2d1_1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <wincodec.h>
#include <Windows.h>
#include <WinUser.h>

#include <xpsobjectmodel_1.h>
#include <DocumentTarget.h>

#include "D2DPrintJobChecker.h"

// SafeRelease inline function.
template <class Interface> inline void SafeRelease(
    Interface** interfaceToRelease
    )
{
    if (*interfaceToRelease != nullptr)
    {
        (*interfaceToRelease)->Release();

        (*interfaceToRelease) = nullptr;
    }
}

// Macros.
#ifndef Assert
#if defined(DEBUG) || defined(_DEBUG) || DBG
#define Assert(b) if (!(b)) { OutputDebugStringA("Assert: " #b "\n"); }
#else
#define Assert(b)
#endif // DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// The main class for this sample application.
class DemoApp
{
public:
    DemoApp();

    ~DemoApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();

    HRESULT CreateDeviceContext();

    HRESULT CreateDeviceResources();

    void DiscardDeviceResources();

    HRESULT CreateGridPatternBrush(
        _Outptr_ ID2D1ImageBrush** imageBrush
        );

    HRESULT OnRender();

    HRESULT OnPrint();

    HRESULT InitializePrintJob();

    HRESULT GetPrintTicketFromDevmode(
        _In_ PCTSTR printerName,
        _In_reads_bytes_(devModeSize) PDEVMODE devMode,
        WORD devModeSize,
        _Out_ LPSTREAM* printTicketStream
        );

    HRESULT FinalizePrintJob();

    HRESULT DrawToContext(
        _In_ ID2D1DeviceContext* d2dContext,
        UINT pageNumber, // 1-based page number
        BOOL printing
        );

    void OnChar(
        SHORT key
        );

    void ToggleMultiPageMode();

    void OnResize();

    void OnVScroll(
        WPARAM wParam,
        LPARAM lParam
        );

    void ResetScrollBar();

    void OnMouseWheel(
        WPARAM wParam,
        LPARAM lParam
        );

    D2D1_SIZE_U CalculateD2DWindowSize();

    static LRESULT CALLBACK ParentWndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    static LRESULT CALLBACK ChildWndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    static HRESULT LoadBitmapFromFile(
        _In_ ID2D1DeviceContext* d2dContext,
        _In_ IWICImagingFactory* wicFactory,
        _In_ PCWSTR uri,
        UINT destinationWidth,
        UINT destinationHeight,
        _Outptr_ ID2D1Bitmap** bitmap
        );

    LRESULT OnClose();

private:
    bool m_resourcesValid;                      // Whether or not the device-dependent resources are ready to use.

    HWND m_parentHwnd;                          // The outer window containing the scroll bar and inner window.
    HWND m_d2dHwnd;                             // The inner window onto which D2D renders.
    bool m_multiPageMode;                       // Whether or not the application is currently in multi-page mode.
    static const UINT m_scrollRange = 1100;     // The maximum distance the user is allowed to scroll.

    // The current distance between the top of the scene and the top
    // of the displayed rectangle.
    INT m_currentScrollPosition;

    // Device-independent resources.
    ID2D1Factory1* m_d2dFactory;
    IWICImagingFactory2* m_wicFactory;
    IDWriteFactory* m_dwriteFactory;
    IDWriteTextFormat* m_textFormat;
    IDWriteTextFormat* m_smallTextFormat;
    ID2D1PathGeometry* m_pathGeometry;

    // Device-dependent resources.
    IDXGISwapChain* m_swapChain;
    ID2D1Device* m_d2dDevice;
    ID2D1DeviceContext* m_d2dContext;
    ID2D1LinearGradientBrush* m_linearGradientBrush;
    ID2D1SolidColorBrush* m_blackBrush;
    ID2D1ImageBrush* m_gridPatternBrush;
    ID2D1Bitmap* m_customBitmap;

    // Printing-specific resources.
    IStream* m_jobPrintTicketStream;
    ID2D1PrintControl* m_printControl;
    IPrintDocumentPackageTarget* m_documentTarget;
    D2DPrintJobChecker* m_printJobChecker;

    // Page size (in DIPs).
    float m_pageHeight;
    float m_pageWidth;
};
