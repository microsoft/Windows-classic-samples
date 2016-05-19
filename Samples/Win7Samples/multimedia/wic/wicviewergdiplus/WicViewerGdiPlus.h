// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Resource.h"

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
    HRESULT CreateDIBFromFile(HWND hWnd);
    BOOL LocateImageFile(HWND hWnd, LPWSTR pszFileName, DWORD cbFileName);
    HRESULT ConvertBitmapSource(HWND hWnd, IWICBitmapSource **ppToRenderBitmapSource);
    HRESULT CreateDIBSectionFromBitmapSource(IWICBitmapSource *pToRenderBitmapSource);
    void DeleteBufferAndBmp();

    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(HWND hWnd);

    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE           m_hInst;
    Gdiplus::Bitmap    *m_pGdiPlusBitmap;
    BYTE               *m_pbBuffer;

    IWICImagingFactory *m_pIWICFactory;
    IWICBitmapSource   *m_pOriginalBitmapSource;
};








