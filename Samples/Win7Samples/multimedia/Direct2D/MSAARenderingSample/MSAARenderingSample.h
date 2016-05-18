// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS      // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE           // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600    // Change this to the appropriate value to target other versions of IE.
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>

#include <d3d10_1.h>
#include <d3dx10math.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <strsafe.h>

/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/

template<class Interface>
inline void
SafeRelease(
    Interface **ppInterfaceToRelease
    )
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#define TIME_RING_BUFFER_SIZE 10

#include "ringbuffer.h"

namespace MyAntialiasMode
{
    enum Enum
    {
        Aliased = 0,
        PerPrimitive,
        MSAA,
        Count
    };
}


namespace SampleType
{
    enum Enum
    {
        Filled,
        Wireframe,
        Count
    };
}

/******************************************************************
*                                                                 *
*  MSAASampleApp                                                  *
*                                                                 *
******************************************************************/

class MSAASampleApp
{
public:
    MSAASampleApp();
    ~MSAASampleApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnMouseMove(
        LPARAM lParam
        );

    void OnKeyDown(
        SHORT vkey
        );

    void OnWheel(
        WPARAM w
        );

    float GetZoom()
    {
        return powf(2.0f, m_logZoom/WHEEL_DELTA);
    }

    PCWSTR GetAntialiasModeString();

    HRESULT RenderD2DContentIntoSurface(
        float time
        );

    HRESULT RenderLoupe();

    HRESULT RenderTextInfo();

    HRESULT CreateD3DDevice(
        IDXGIAdapter *pAdapter,
        D3D10_DRIVER_TYPE driverType,
        UINT flags,
        ID3D10Device1 **ppDevice
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND m_hwnd;

    MyAntialiasMode::Enum m_antialiasMode;
    SampleType::Enum m_sampleType;
    bool m_paused;
    bool m_drawLoupe;
    UINT m_numSquares;
    float m_logZoom;
    D2D1_POINT_2F m_mousePos;
    LONGLONG m_pausedTime;
    LONGLONG m_timeDelta;
    RingBuffer<LONGLONG, TIME_RING_BUFFER_SIZE> m_times;

    //Device-Dependent Resources
    ID3D10Device *m_pDevice;
    IDXGISwapChain *m_pSwapChain;
    ID3D10RasterizerState *m_pState;
    ID3D10RenderTargetView *m_pRenderTargetView;
    ID3D10Texture2D *m_pLoupeTexture;
    ID2D1Bitmap *m_pLoupeBitmap;

    ID2D1RenderTarget *m_pBackBufferRT;

    ID2D1SolidColorBrush *m_pTextBrush;
    ID2D1SolidColorBrush *m_pLoupeBrush;

    // Device-Independent Resources
    ID2D1Factory *m_pD2DFactory;
    IDWriteFactory *m_pDWriteFactory;
    IDWriteTextFormat *m_pTextFormat;
};

