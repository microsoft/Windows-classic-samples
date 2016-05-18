// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 7 or later.
#define WINVER 0x0700       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 7 or later.
#define _WIN32_WINNT 0x0700 // Change this to the appropriate value to target other versions of Windows.
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

#include "d3dmath.h"
#include "resource.h"

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

/******************************************************************
*                                                                 *
*  SimpleVertex                                                   *
*                                                                 *
******************************************************************/

struct SimpleVertex
{
    D3DXVECTOR3 Pos;
    D3DXVECTOR2 Tex;
};

/******************************************************************
*                                                                 *
*  DXGISampleApp                                                  *
*                                                                 *
******************************************************************/

class DXGISampleApp
{
public:
    DXGISampleApp();
    ~DXGISampleApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    HRESULT RecreateSizedResources(UINT nWidth, UINT nHeight);
    HRESULT CreateD3DDeviceResources();
    HRESULT CreateD2DDeviceResources();

    HRESULT CreateGridPatternBrush(
        ID2D1RenderTarget *pRenderTarget,
        ID2D1BitmapBrush **ppBitmapBrush
        );

    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnResize(UINT width, UINT height);
    void OnGetMinMaxInfo(MINMAXINFO *pMinMaxInfo);
    void OnTimer();

    HRESULT RenderD2DContentIntoSurface();

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

    HRESULT LoadResourceBitmap(
        ID2D1RenderTarget *pRenderTarget,
        IWICImagingFactory *pIWICFactory,
        PCWSTR resourceName,
        PCWSTR resourceType,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap **ppBitmap
        );

    HRESULT LoadResourceShader(
        ID3D10Device *pDevice,
        PCWSTR pszResource,
        ID3D10Effect **ppEffect
        );

private:
    HWND m_hwnd;
    ID2D1Factory *m_pD2DFactory;
    IWICImagingFactory *m_pWICFactory;
    IDWriteFactory *m_pDWriteFactory;

    //Device-Dependent Resources
    ID3D10Device *m_pDevice;
    IDXGISwapChain *m_pSwapChain;
    ID3D10RenderTargetView *m_pRenderTargetView;
    ID3D10RasterizerState *m_pState;
    ID3D10Texture2D *m_pDepthStencil;
    ID3D10DepthStencilView *m_pDepthStencilView;
    ID3D10Texture2D *m_pOffscreenTexture;
    ID3D10Effect *m_pShader;
    ID3D10Buffer *m_pVertexBuffer;
    ID3D10InputLayout *m_pVertexLayout;
    ID3D10Buffer *m_pFacesIndexBuffer;
    ID3D10ShaderResourceView *m_pTextureRV;

    ID2D1RenderTarget *m_pBackBufferRT;
    ID2D1SolidColorBrush *m_pBackBufferTextBrush;
    ID2D1LinearGradientBrush *m_pBackBufferGradientBrush;
    ID2D1BitmapBrush *m_pGridPatternBitmapBrush;

    ID2D1RenderTarget *m_pRenderTarget;
    ID2D1LinearGradientBrush *m_pLGBrush;
    ID2D1SolidColorBrush *m_pBlackBrush;
    ID2D1Bitmap *m_pBitmap;

    ID3D10EffectTechnique *m_pTechniqueNoRef;
    ID3D10EffectMatrixVariable *m_pWorldVariableNoRef;
    ID3D10EffectMatrixVariable *m_pViewVariableNoRef;
    ID3D10EffectMatrixVariable *m_pProjectionVariableNoRef;
    ID3D10EffectShaderResourceVariable *m_pDiffuseVariableNoRef;

    // Device-Independent Resources
    IDWriteTextFormat *m_pTextFormat;
    ID2D1PathGeometry *m_pPathGeometry;

    D3DXMATRIX m_WorldMatrix;
    D3DXMATRIX m_ViewMatrix;
    D3DXMATRIX m_ProjectionMatrix;

    static const D3D10_INPUT_ELEMENT_DESC s_InputLayout[];
    static const SimpleVertex s_VertexArray[];
    static const SHORT DXGISampleApp::s_FacesIndexArray[];
};

