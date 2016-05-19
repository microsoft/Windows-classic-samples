// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

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
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

HRESULT SaveToImageFile()
{
    HRESULT hr = S_OK;


    //
    // Create Factories
    //

    IWICImagingFactory *pWICFactory = NULL;
    ID2D1Factory *pD2DFactory = NULL;
    IDWriteFactory *pDWriteFactory = NULL;
    IWICBitmap *pWICBitmap = NULL;
    ID2D1RenderTarget *pRT = NULL;
    IDWriteTextFormat *pTextFormat = NULL;
    ID2D1PathGeometry *pPathGeometry = NULL;
    ID2D1GeometrySink *pSink = NULL;
    ID2D1GradientStopCollection *pGradientStops = NULL;
    ID2D1LinearGradientBrush *pLGBrush = NULL;
    ID2D1SolidColorBrush *pBlackBrush = NULL;
    IWICBitmapEncoder *pEncoder = NULL;
    IWICBitmapFrameEncode *pFrameEncode = NULL;
    IWICStream *pStream = NULL;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void **>(&pWICFactory)
        );

    if (SUCCEEDED(hr))
    {
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    }

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(pDWriteFactory),
            reinterpret_cast<IUnknown **>(&pDWriteFactory)
            );
    }

    //
    // Create IWICBitmap and RT
    //

    static const UINT sc_bitmapWidth = 640;
    static const UINT sc_bitmapHeight = 480;
    if (SUCCEEDED(hr))
    {
        hr = pWICFactory->CreateBitmap(
            sc_bitmapWidth,
            sc_bitmapHeight,
            GUID_WICPixelFormat32bppBGR,
            WICBitmapCacheOnLoad,
            &pWICBitmap
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = pD2DFactory->CreateWicBitmapRenderTarget(
            pWICBitmap,
            D2D1::RenderTargetProperties(),
            &pRT
            );
    }

    if (SUCCEEDED(hr))
    {
        //
        // Create text format
        //

        static const WCHAR sc_fontName[] = L"Calibri";
        static const FLOAT sc_fontSize = 50;

        hr = pDWriteFactory->CreateTextFormat(
            sc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            sc_fontSize,
            L"", //locale
            &pTextFormat
            );
    }
    if (SUCCEEDED(hr))
    {
        pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        //
        // Create a path geometry representing an hour glass
        //

        hr = pD2DFactory->CreatePathGeometry(&pPathGeometry);
    }
    if (SUCCEEDED(hr))
    {
        hr = pPathGeometry->Open(&pSink);
    }
    if (SUCCEEDED(hr))
    {
        pSink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

        pSink->BeginFigure(
            D2D1::Point2F(0, 0),
            D2D1_FIGURE_BEGIN_FILLED
            );

        pSink->AddLine(D2D1::Point2F(200, 0));

        pSink->AddBezier(
            D2D1::BezierSegment(
                D2D1::Point2F(150, 50),
                D2D1::Point2F(150, 150),
                D2D1::Point2F(200, 200))
            );

        pSink->AddLine(D2D1::Point2F(0, 200));

        pSink->AddBezier(
            D2D1::BezierSegment(
                D2D1::Point2F(50, 150),
                D2D1::Point2F(50, 50),
                D2D1::Point2F(0, 0))
            );

        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

        hr = pSink->Close();
    }
    if (SUCCEEDED(hr))
    {
        //
        // Create a linear-gradient brush
        //

        static const D2D1_GRADIENT_STOP stops[] =
        {
            {   0.f,  { 0.f, 1.f, 1.f, 1.f }  },
            {   1.f,  { 0.f, 0.f, 1.f, 1.f }  },
        };

        hr = pRT->CreateGradientStopCollection(
            stops,
            ARRAYSIZE(stops),
            &pGradientStops
            );
    }
    if (SUCCEEDED(hr))
    {
        hr = pRT->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(100, 0),
                D2D1::Point2F(100, 200)),
            D2D1::BrushProperties(),
            pGradientStops,
            &pLGBrush
            );
    }
    if (SUCCEEDED(hr))
    {
        hr = pRT->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &pBlackBrush
            );
    }
    if (SUCCEEDED(hr))
    {
        //
        // Render into the bitmap
        //

        pRT->BeginDraw();

        pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = pRT->GetSize();

        // Set the world transform to a 45 degree rotation at the center of the render target
        // and write "Hello, World".
        pRT->SetTransform(
            D2D1::Matrix3x2F::Rotation(
                45,
                D2D1::Point2F(
                    rtSize.width / 2,
                    rtSize.height / 2))
                );

        static const WCHAR sc_helloWorld[] = L"Hello, World!";
        pRT->DrawText(
            sc_helloWorld,
            ARRAYSIZE(sc_helloWorld) - 1,
            pTextFormat,
            D2D1::RectF(0, 0, rtSize.width, rtSize.height),
            pBlackBrush);

        //
        // Reset back to the identity transform
        //
        pRT->SetTransform(D2D1::Matrix3x2F::Translation(0, rtSize.height - 200));

        pRT->FillGeometry(pPathGeometry, pLGBrush);

        pRT->SetTransform(D2D1::Matrix3x2F::Translation(rtSize.width - 200, 0));

        pRT->FillGeometry(pPathGeometry, pLGBrush);

        hr = pRT->EndDraw();
    }
    if (SUCCEEDED(hr))
    {

        //
        // Save image to file
        //

        hr = pWICFactory->CreateStream(&pStream);
    }

    WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
    if (SUCCEEDED(hr))
    {
        static const WCHAR filename[] = L"output.png";
        hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
    }
    if (SUCCEEDED(hr))
    {
        hr = pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
    }
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
    }
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->Initialize(NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->SetSize(sc_bitmapWidth, sc_bitmapHeight);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->WriteSource(pWICBitmap, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->Commit();
    }
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Commit();
    }

    SafeRelease(&pWICFactory);
    SafeRelease(&pD2DFactory);
    SafeRelease(&pDWriteFactory);
    SafeRelease(&pWICBitmap);
    SafeRelease(&pRT);
    SafeRelease(&pTextFormat);
    SafeRelease(&pPathGeometry);
    SafeRelease(&pSink);
    SafeRelease(&pGradientStops);
    SafeRelease(&pLGBrush);
    SafeRelease(&pBlackBrush);
    SafeRelease(&pEncoder);
    SafeRelease(&pFrameEncode);
    SafeRelease(&pStream);

    return hr;
}


/*=========================================================================*\
    Main
\*=========================================================================*/

int __cdecl wmain()
{
    // Ignoring the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        HRESULT hr = SaveToImageFile();
        if (FAILED(hr))
        {
            wprintf(L"Unexpected error: 0x%x", hr);
        }

        CoUninitialize();
    }

    return 0;
}
