
/************************************************************************
 *
 * File: DemoApp.h
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#pragma once

/******************************************************************
*                                                                 *
*  DemoApp                                                     *
*                                                                 *
******************************************************************/

class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    HRESULT Initialize();

private:
    HRESULT CreateDeviceIndependentResources(
        );

    void DiscardDeviceIndependentResources(
        );

    HRESULT CreateDeviceResources(
        );

    void DiscardDeviceResources(
        );

    HRESULT DrawD2DContent(
        );

    HRESULT DrawText(
        );

    void OnResize(
        UINT width,
        UINT height
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND hwnd_;

    // how much to scale a design that assumes 96-DPI pixels
    float dpiScaleX_;
    float dpiScaleY_;

    // Direct2D
    ID2D1Factory* pD2DFactory_;
    ID2D1HwndRenderTarget* pRT_;
    ID2D1SolidColorBrush* pBlackBrush_;

    // DirectWrite
    IDWriteFactory* pDWriteFactory_;
    IDWriteTextFormat* pTextFormat_;

    const wchar_t* wszText_;
    UINT32 cTextLength_;
};


