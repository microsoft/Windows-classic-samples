// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "resource.h"

const FLOAT DEFAULT_DPI = 96.f;   // Default DPI that maps image resolution directly to screen resoltuion

/******************************************************************
*                                                                 *
*  DemoApp                                                        *
*                                                                 *
******************************************************************/

class DemoApp
{
public:

    DemoApp();
    ~DemoApp();

    HRESULT Initialize(HINSTANCE hInstance);

private:

    enum DISPOSAL_METHODS
    {
        DM_UNDEFINED  = 0,
        DM_NONE       = 1,
        DM_BACKGROUND = 2,
        DM_PREVIOUS   = 3 
    };

    HRESULT CreateDeviceResources();
    HRESULT RecoverDeviceResources();

    HRESULT OnResize(UINT uWidth, UINT uHeight);
    HRESULT OnRender();

    BOOL    GetFileOpen(WCHAR *pszFileName, DWORD cchFileName);
    HRESULT SelectAndDisplayGif();

    HRESULT GetRawFrame(UINT uFrameIndex);
    HRESULT GetGlobalMetadata();
    HRESULT GetBackgroundColor(IWICMetadataQueryReader *pMetadataQueryReader);

    HRESULT ComposeNextFrame();
    HRESULT DisposeCurrentFrame();
    HRESULT OverlayNextFrame();

    HRESULT SaveComposedFrame();
    HRESULT RestoreSavedFrame();
    HRESULT ClearCurrentFrameArea();

    BOOL IsLastFrame()
    {
        return (m_uNextFrameIndex == 0);
    }

    BOOL EndOfAnimation()
    {
        return m_fHasLoop && IsLastFrame() && m_uLoopNumber == m_uTotalLoopCount + 1;
    }

    HRESULT CalculateDrawRectangle(D2D1_RECT_F &drawRect);

    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

    HWND                        m_hWnd;

    ID2D1Factory               *m_pD2DFactory;
    ID2D1HwndRenderTarget      *m_pHwndRT;
    ID2D1BitmapRenderTarget    *m_pFrameComposeRT;
    ID2D1Bitmap                *m_pRawFrame;
    ID2D1Bitmap                *m_pSavedFrame;          // The temporary bitmap used for disposal 3 method
    D2D1_COLOR_F                m_backgroundColor;

    IWICImagingFactory         *m_pIWICFactory;
    IWICBitmapDecoder          *m_pDecoder;

    UINT    m_uNextFrameIndex;
    UINT    m_uTotalLoopCount;  // The number of loops for which the animation will be played
    UINT    m_uLoopNumber;      // The current animation loop number (e.g. 1 when the animation is first played)
    BOOL    m_fHasLoop;         // Whether the gif has a loop
    UINT    m_cFrames;
    UINT    m_uFrameDisposal;
    UINT    m_uFrameDelay;
    UINT    m_cxGifImage;
    UINT    m_cyGifImage;
    UINT    m_cxGifImagePixel;  // Width of the displayed image in pixel calculated using pixel aspect ratio
    UINT    m_cyGifImagePixel;  // Height of the displayed image in pixel calculated using pixel aspect ratio
    D2D1_RECT_F m_framePosition;
};