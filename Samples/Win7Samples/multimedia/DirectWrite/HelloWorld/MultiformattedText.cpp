
/************************************************************************
 *
 * File: MultiformattedText.cpp
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

#include "DWriteHelloWorld.h"


/******************************************************************
*                                                                 *
*  MultiformattedText::MultiformattedText constructor             *
*                                                                 *
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

MultiformattedText::MultiformattedText() :
    hwnd_(NULL),
    wszText_(NULL),
    cTextLength_(0),
    pD2DFactory_(NULL),
    pRT_(NULL),
    pBlackBrush_(NULL),
    pDWriteFactory_(NULL),
    pTextFormat_(NULL),
    pTextLayout_(NULL)
{
}

/******************************************************************
*                                                                 *
*  MultiformattedText::~MultiformattedText destructor             *
*                                                                 *
*  Tear down resources                                            *
*                                                                 *
******************************************************************/

MultiformattedText::~MultiformattedText()
{
    SafeRelease(&pD2DFactory_);
    SafeRelease(&pRT_);
    SafeRelease(&pBlackBrush_);
    SafeRelease(&pDWriteFactory_);
    SafeRelease(&pTextFormat_);
    SafeRelease(&pTextLayout_);
}

/******************************************************************
*                                                                 *
*  MultiformattedText::Initialize                                 *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/

HRESULT MultiformattedText::Initialize(HWND hwndParent)
{
    WNDCLASSEX wcex;
    
    // Get the dpi information.
    HDC screen = GetDC(0);
    dpiScaleX_ = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
    dpiScaleY_ = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
    ReleaseDC(0, screen);

    HRESULT hr = S_OK;
    ATOM atom;

    // Register window class.
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = MultiformattedText::WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = HINST_THISCOMPONENT;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.hIcon         = LoadIcon(
                            NULL,
                            IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(
                            NULL,
                            IDC_ARROW);
    wcex.lpszClassName = TEXT("D2DMultiformattedText");
    wcex.hIconSm       = LoadIcon(
                            NULL,
                            IDI_APPLICATION
                            );

    atom = RegisterClassEx(
        &wcex
        );

    hr = atom ? S_OK : E_FAIL;

    // Create window.
    hwnd_ = CreateWindow(
        TEXT("D2DMultiformattedText"),
        TEXT(""),
        WS_CHILD,
        0,
        0,
        static_cast<int>(640.0f / dpiScaleX_),
        static_cast<int>(480.0f / dpiScaleY_),
        hwndParent,
        NULL,
        HINST_THISCOMPONENT,
        this
        );

    if (SUCCEEDED(hr))
    {
        hr = hwnd_ ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceIndependentResources(
            );
    }

    if (SUCCEEDED(hr))
    {
        DrawD2DContent();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  MultiformattedText::CreateDeviceIndependentResources           *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app. These resources include the Direct2D and  *
*  DirectWrite factories; and a DirectWrite Text Format object    *
*  (used for identifying particular font characteristics) and     *
*  a Direct2D geometry.                                           *
*                                                                 *
******************************************************************/

HRESULT MultiformattedText::CreateDeviceIndependentResources()
{
    HRESULT hr;

    // Create Direct2D factory.
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory_
        );

    // Create a shared DirectWrite factory.
    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory_)
            );
    }

    // The string to display.
    wszText_ = L"Hello World using   DirectWrite!";
    cTextLength_ = (UINT32) wcslen(wszText_);

    // Create a text format using Gabriola with a font size of 72.
    // This sets the default font, weight, stretch, style, and locale.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextFormat(
            L"Gabriola",                 // Font family name.
            NULL,                        // Font collection (NULL sets it to use the system font collection).
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            72.0f,
            L"en-us",
            &pTextFormat_
            );
    }

        // Center align (horizontally) the text.
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

        // Center align (vertically) the text.
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }


    // Create a text layout using the text format.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextLayout(
            wszText_,      // The string to be laid out and formatted.
            cTextLength_,  // The length of the string.
            pTextFormat_,  // The text format to apply to the string (contains font information, etc).
            640.0f,         // The width of the layout box.
            480.0f,         // The height of the layout box.
            &pTextLayout_  // The IDWriteTextLayout interface pointer.
            );
    }


    // Format the "DirectWrite" substring to be of font size 100.
    if (SUCCEEDED(hr))
    {
        DWRITE_TEXT_RANGE textRange = {20,        // Start index where "DirectWrite" appears.
            6 };      // Length of the substring "Direct" in "DirectWrite".
        hr = pTextLayout_->SetFontSize(100.0f, textRange);
    }

    // Format the word "DWrite" to be underlined.
    if (SUCCEEDED(hr))
    {
        
        DWRITE_TEXT_RANGE textRange = {20,      // Start index where "DirectWrite" appears.
            11 };    // Length of the substring "DirectWrite".
        hr = pTextLayout_->SetUnderline(TRUE, textRange);
    }

    if (SUCCEEDED(hr))
    {
        // Format the word "DWrite" to be bold.
        DWRITE_TEXT_RANGE textRange = {20,
            11 };
        hr = pTextLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, textRange);
    }


    // Declare a typography pointer.
    IDWriteTypography* pTypography = NULL;

    // Create a typography interface object.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTypography(&pTypography);
    }



    // Set the stylistic set.
    DWRITE_FONT_FEATURE fontFeature = {DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,
                                       1};
    if (SUCCEEDED(hr))
    {
        hr = pTypography->AddFontFeature(fontFeature);
    }



    if (SUCCEEDED(hr))
    {
        // Set the typography for the entire string.
        DWRITE_TEXT_RANGE textRange = {0,
                                       cTextLength_};
        hr = pTextLayout_->SetTypography(pTypography, textRange);
    }


    SafeRelease(&pTypography);

    return hr;
}

/******************************************************************
*                                                                 *
*  MultiformattedText::CreateDeviceResources                      *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

HRESULT MultiformattedText::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(
        hwnd_,
        &rc
        );

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    if (!pRT_)
    {
        // Create a Direct2D render target.
        hr = pD2DFactory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd_,
                size
                ),
            &pRT_
            );

        // Create a black brush.
        if (SUCCEEDED(hr))
        {
            hr = pRT_->CreateSolidColorBrush(
                D2D1::ColorF(
                D2D1::ColorF::Black
                ),
                &pBlackBrush_
                );
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  MultiformattedText::DiscardDeviceResources                     *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a D3D device is lost                                      *
*                                                                 *
******************************************************************/

void MultiformattedText::DiscardDeviceResources()
{
    SafeRelease(&pRT_);
    SafeRelease(&pBlackBrush_);
}


/******************************************************************
*                                                                 *
*  MultiformattedText::DrawText                                   *
*                                                                 *
*  This method will draw text using the IDWriteTextLayout         *
*  via the Direct2D render target                                 *
*                                                                 *
******************************************************************/

HRESULT MultiformattedText::DrawText()
{
    RECT rc;

    GetClientRect(
        hwnd_,
        &rc
        );


    D2D1_POINT_2F origin = D2D1::Point2F(
        static_cast<FLOAT>(rc.top / dpiScaleY_),
        static_cast<FLOAT>(rc.left / dpiScaleX_)
        );



    pRT_->DrawTextLayout(
        origin,
        pTextLayout_,
        pBlackBrush_
        );

    return S_OK;
}

/******************************************************************
*                                                                 *
*  MultiformattedText::DrawD2DContent                             *
*                                                                 *
*  This method draws a bitmap a couple times, draws some          *
*  geometry, and writes "Hello World"                             *
*                                                                 *
*  Note that this function will not render anything if the window *
*  is occluded (eg. obscured by other windows or off monitor).    *
*  Also, this function will automatically discard device-specific *
*  resources if the D3D device disappears during execution, and   *
*  will recreate the resources the next time it's invoked.        *
*                                                                 *
******************************************************************/

HRESULT MultiformattedText::DrawD2DContent()
{
    HRESULT hr;

    hr = CreateDeviceResources();

    if (!(pRT_->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        pRT_->BeginDraw();

        pRT_->SetTransform(D2D1::IdentityMatrix());

        pRT_->Clear(D2D1::ColorF(D2D1::ColorF::White));

        // Call the DrawText method of this class.
        if (SUCCEEDED(hr))
        {
            hr = DrawText();
        }

        if (SUCCEEDED(hr))
        {
            hr = pRT_->EndDraw(
                );
        }
    }

    if (FAILED(hr))
    {
        DiscardDeviceResources();
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  MultiformattedText::OnResize                                   *
*                                                                 *
*  If the application receives a WM_SIZE message, this method     *
*  resize the render target appropriately.                        *
*                                                                 *
******************************************************************/

void MultiformattedText::OnResize(UINT width, UINT height)
{
    if (pTextLayout_)
    {
        pTextLayout_->SetMaxWidth(static_cast<FLOAT>(width / dpiScaleX_));
        pTextLayout_->SetMaxHeight(static_cast<FLOAT>(height / dpiScaleY_));
    }

    if (pRT_)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;
        pRT_->Resize(size);
    }
}

/******************************************************************
*                                                                 *
*  MultiformattedText::WndProc                                    *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

LRESULT CALLBACK MultiformattedText::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        MultiformattedText *pMultiformattedText = (MultiformattedText *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pMultiformattedText));

        return 1;
    }

    MultiformattedText *pMultiformattedText = reinterpret_cast<MultiformattedText *>(
                ::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (pMultiformattedText)
    {
        switch(message)
        {
        case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pMultiformattedText->OnResize(width, height);
            }
            return 0;

        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                pMultiformattedText->DrawD2DContent(
                    );
                EndPaint(
                    hwnd,
                    &ps
                    );
            }
            return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
        }
    }
    return DefWindowProc(
        hwnd,
        message,
        wParam,
        lParam
        );
}

