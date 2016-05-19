// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Main user interface window.
//
//----------------------------------------------------------------------------
#pragma once


class MainWindow
{
public:
    MainWindow()
    :   hwnd_(NULL),
        hmonitor_(NULL),
        textMode_(CommandIdTextLatin),
        dwriteFactory_(),
        renderingParams_(),
        renderTarget_(),
        flowLayoutSource_(),
        flowLayoutSink_(),
        flowLayout_()
    { }

    ~MainWindow()
    {
        SafeRelease(&dwriteFactory_);
        SafeRelease(&renderingParams_);
        SafeRelease(&renderTarget_);
        SafeRelease(&flowLayoutSource_);
        SafeRelease(&flowLayoutSink_);
        SafeRelease(&flowLayout_);
    }

    static ATOM RegisterWindowClass();
    static LRESULT CALLBACK WindowProc(HWND parentHwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT Initialize();
    WPARAM RunMessageLoop();

public:
    const static wchar_t* g_windowClassName;

protected:
    void OnPaint(const PAINTSTRUCT& ps);
    void OnSize();
    void OnMove();
    void OnCommand(UINT commandId);

    STDMETHODIMP ReflowLayout();
    STDMETHODIMP SetLayoutText(UINT commandId);
    STDMETHODIMP SetLayoutShape(UINT commandId);
    STDMETHODIMP SetLayoutNumbers(UINT commandId);

    HWND hwnd_;
    HMONITOR hmonitor_;

    IDWriteFactory*             dwriteFactory_;
    IDWriteRenderingParams*     renderingParams_;
    IDWriteBitmapRenderTarget*  renderTarget_;

    FlowLayoutSource*           flowLayoutSource_;
    FlowLayoutSink*             flowLayoutSink_;
    FlowLayout*                 flowLayout_;

    int textMode_;

private:
    // No copy construction allowed.
    MainWindow(const MainWindow& b);
    MainWindow& operator=(const MainWindow&);
};
