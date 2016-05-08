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

#include <new>

#include <d3d10_1.h>
#include <d3dx10math.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

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

template<class DestInterface, class SourceInterace>
inline void
SafeReplace(
    DestInterface **ppDestInterface,
    SourceInterace *pSourceInterface
    )
{
    if (*ppDestInterface != NULL)
    {
        (*ppDestInterface)->Release();
    }

    *ppDestInterface = pSourceInterface;

    if (pSourceInterface)
    {
        (*ppDestInterface)->AddRef();
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

#include "array.h"

#include "d3dmath.h"
#include "resource.h"

/******************************************************************
*                                                                 *
*  SimpleVertex                                                   *
*                                                                 *
******************************************************************/

struct SimpleVertex
{
    D3DXVECTOR3 Pos;
    D3DXVECTOR3 Norm;
};

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp                                     *
*                                                                 *
******************************************************************/

class Interactive3dTextSampleApp
{
public:
    Interactive3dTextSampleApp();
    ~Interactive3dTextSampleApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnTimer();

    void OnChar(
        SHORT vkey
        );

    HRESULT CreateD3DDevice(
        IDXGIAdapter *pAdapter,
        D3D10_DRIVER_TYPE DriverType,
        UINT Flags,
        ID3D10Device1 **ppDevice
        );

    HRESULT GenerateTextOutline(
        bool includeCursor,
        ID2D1Geometry **ppGeometry
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    HRESULT LoadResourceShader(
        ID3D10Device *pDevice,
        PCWSTR pszResource,
        ID3D10Effect **ppEffect
        );

    HRESULT UpdateTextGeometry();

private:
    HWND m_hwnd;
    ID2D1Factory *m_pD2DFactory;
    IDWriteFactory *m_pDWriteFactory;

    // Device-Dependent Resources
    ID3D10Device *m_pDevice;
    IDXGISwapChain *m_pSwapChain;
    ID3D10RasterizerState *m_pState;
    ID3D10Texture2D *m_pDepthStencil;
    ID3D10DepthStencilView *m_pDepthStencilView;
    ID3D10RenderTargetView *m_pRenderTargetView;
    ID3D10Effect *m_pShader;
    ID3D10Buffer *m_pVertexBuffer;
    ID3D10InputLayout *m_pVertexLayout;

    ID3D10EffectTechnique *m_pTechniqueNoRef;
    ID3D10EffectMatrixVariable *m_pWorldVariableNoRef;
    ID3D10EffectMatrixVariable *m_pViewVariableNoRef;
    ID3D10EffectMatrixVariable *m_pProjectionVariableNoRef;

    ID3D10EffectVectorVariable* m_pLightPosVariableNoRef;
    ID3D10EffectVectorVariable* m_pLightColorVariableNoRef;

    // Device-Independent Resources

    ID2D1Geometry *m_pTextGeometry;
    IDWriteTextLayout *m_pTextLayout;

    D3DXMATRIX m_WorldMatrix;
    D3DXMATRIX m_ViewMatrix;
    D3DXMATRIX m_ProjectionMatrix;

    static const D3D10_INPUT_ELEMENT_DESC s_InputLayout[];
    static const UINT sc_vertexBufferCount;

    CArray<WCHAR> m_characters;
};

