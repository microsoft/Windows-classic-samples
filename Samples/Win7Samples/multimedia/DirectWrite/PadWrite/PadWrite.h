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
#include "Common.h"


class MainWindow
:   public ComBase<QiList<IUnknown> >
{
public:
    MainWindow();

    ~MainWindow()
    {
        SafeRelease(&dwriteFactory_);
        SafeRelease(&wicFactory_);
        SafeRelease(&d2dFactory_);
        SafeRelease(&renderTarget_);
        SafeRelease(&textEditor_);
        SafeRelease(&inlineObjectImages_);
    }

    static ATOM RegisterWindowClass();
    static LRESULT CALLBACK WindowProc(HWND parentHwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT Initialize();
    WPARAM RunMessageLoop();

public:
    STDMETHODIMP CreateFontFromLOGFONT(
        const LOGFONT& logFont,
        OUT IDWriteFont** font
        );

    STDMETHODIMP GetFontFamilyName(
        IDWriteFont* font,
        OUT wchar_t* fontFamilyName,
        UINT32 fontFamilyNameLength
        );

protected:
    enum RenderTargetType
    {
        RenderTargetTypeD2D,
        RenderTargetTypeDW,
        RenderTargetTypeTotal
    };

    HWND hwnd_;
    IDWriteFactory*                     dwriteFactory_;
    IWICImagingFactory*                 wicFactory_;
    ID2D1Factory*                       d2dFactory_;

    RenderTarget*                       renderTarget_;
    RenderTargetType                    renderTargetType_;

    TextEditor*                         textEditor_;
    IWICBitmapSource*                   inlineObjectImages_;

protected:
    HRESULT CreateRenderTarget(HWND hwnd, RenderTargetType renderTargetType);
    HRESULT FormatSampleLayout(IDWriteTextLayout* textLayout);

    void OnSize();
    void OnCommand(UINT commandId);
    void OnDestroy();
    HRESULT OnChooseFont();
    HRESULT OnSetInlineImage();

    void UpdateMenuToCaret();
    void RedrawTextEditor();
};
