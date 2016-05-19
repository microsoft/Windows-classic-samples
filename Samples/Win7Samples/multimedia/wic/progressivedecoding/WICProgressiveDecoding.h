// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Resource.h"

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

    HWND Initialize(HINSTANCE hInstance);

private:
    BOOL    LocateImageFile(HWND hWnd, LPWSTR pszFileName, DWORD cbFileName);
    HRESULT CreateProgressiveCtrlFromFile(HWND hWnd);
    HRESULT CreateD2DBitmapFromProgressiveCtrl(HWND hWnd, UINT uLevel);
    HRESULT IncreaseProgressiveLevel(HWND hWnd);
    HRESULT DecreaseProgressiveLevel(HWND hWnd);
    HRESULT CreateDeviceResources(HWND hWnd);
    HWND    CreateStatusBar(HWND hWndParent, UINT nStatusID, UINT nParts);

    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(HWND hWnd);
    LRESULT OnSize(LPARAM lParam);

    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    
    HINSTANCE                    m_hInst;
    HWND                         m_hStatusWnd;
    IWICImagingFactory          *m_pIWICFactory;
    UINT                         m_uCurrentLevel;
    UINT                         m_uLevelCount;
    ID2D1Factory                *m_pD2DFactory;
    ID2D1HwndRenderTarget       *m_pRT;
    IWICBitmapFrameDecode       *m_pSourceFrame;
    ID2D1Bitmap                 *m_pD2DBitmap;
    IWICFormatConverter         *m_pConvertedSourceBitmap;
};