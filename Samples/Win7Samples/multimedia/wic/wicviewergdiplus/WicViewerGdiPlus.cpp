// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER                  // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600        // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <wincodec.h>
#include <commdlg.h>
#include <gdiplus.h>
#include "WICViewerGDIPlus.h"


template <typename T>
inline void SafeRelease(T *&p)
{
    if (NULL != p)
    {
        p->Release();
        p = NULL;
    }
}

/******************************************************************
*  Application entrypoint                                         *
******************************************************************/

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR pszCmdLine,
    int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pszCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        // Initialize GDI+.
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;

        // Checking return status from GdiplusStartup 
        Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        if(status == Gdiplus::Ok)
        {

            {
                DemoApp app;
                hr = app.Initialize(hInstance);

                if (SUCCEEDED(hr))
                {
                    BOOL fRet;
                    MSG msg;

                    // Main message loop:
                    while ((fRet = GetMessage(&msg, NULL, 0, 0)) != 0)
                    {
                        if (fRet == -1)
                        {
                            break;
                        }
                        else
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }
            }

            Gdiplus::GdiplusShutdown(gdiplusToken);
        }

        CoUninitialize();
    }

    return 0;
}

/******************************************************************
*  Initialize member data                                         *
******************************************************************/

DemoApp::DemoApp()
    :
    m_pIWICFactory(NULL),
    m_pOriginalBitmapSource(NULL),
    m_pGdiPlusBitmap(NULL),
    m_pbBuffer(NULL)
{
}

/******************************************************************
*  Tear down resources                                            *
******************************************************************/

DemoApp::~DemoApp()
{
    SafeRelease(m_pIWICFactory);
    SafeRelease(m_pOriginalBitmapSource);
    DeleteBufferAndBmp();
}

/******************************************************************
*  Create application window and resources                        *
******************************************************************/

HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
    HRESULT hr = S_OK;

    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pIWICFactory)
        );

    // Register window class
    WNDCLASSEX wcex;
    if (SUCCEEDED(hr))
    {
        wcex.cbSize        = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::s_WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = hInstance;
        wcex.hIcon         = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
        wcex.lpszClassName = L"WICViewerGDIPlus";
        wcex.hIconSm       = NULL;

        m_hInst = hInstance;

        hr = RegisterClassEx(&wcex) ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Create window
        HWND hWnd = CreateWindow(
            L"WICViewerGDIPlus",
            L"WIC Viewer GDI Plus Sample",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            640,
            480,
            NULL,
            NULL,
            hInstance,
            this
            );

        hr = hWnd ? S_OK : E_FAIL;
    }

    return hr;
}

/******************************************************************
*  Load an image file and create an DIB Section                   *
******************************************************************/

HRESULT DemoApp::CreateDIBFromFile(HWND hWnd)
{
    HRESULT hr = S_OK;
    
    WCHAR szFileName[MAX_PATH];

    // Step 1: Create the open dialog box and locate the image file
    if (LocateImageFile(hWnd, szFileName, ARRAYSIZE(szFileName)))
    {
        // Step 2: Decode the source image to IWICBitmapSource

        // Create a decoder
        IWICBitmapDecoder *pDecoder = NULL;

        hr = m_pIWICFactory->CreateDecoderFromFilename(
            szFileName,                      // Image to be decoded
            NULL,                            // Do not prefer a particular vendor
            GENERIC_READ,                    // Desired read access to the file
            WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
            &pDecoder                        // Pointer to the decoder
            );

        // Retrieve the first frame of the image from the decoder
        IWICBitmapFrameDecode *pFrame = NULL;

        if (SUCCEEDED(hr))
        {
            hr = pDecoder->GetFrame(0, &pFrame);
        }

        // Retrieve IWICBitmapSource from the frame
        // m_pOriginalBitmapSource contains the original bitmap and acts as an intermediate
        if (SUCCEEDED(hr))
        {
            SafeRelease(m_pOriginalBitmapSource);
            hr = pFrame->QueryInterface(
                IID_IWICBitmapSource, 
                reinterpret_cast<void **>(&m_pOriginalBitmapSource));
        }

        // Step 3: Scale the original IWICBitmapSource to the client rect size
        // and convert the pixel format
        IWICBitmapSource *pToRenderBitmapSource = NULL;

        if (SUCCEEDED(hr))
        {
            hr = ConvertBitmapSource(hWnd, &pToRenderBitmapSource);
        }

        // Step 4: Create a DIB from the converted IWICBitmapSource
        if (SUCCEEDED(hr))
        {
            hr = CreateDIBSectionFromBitmapSource(pToRenderBitmapSource);
        }

        SafeRelease(pToRenderBitmapSource);
        SafeRelease(pDecoder);
        SafeRelease(pFrame);
    }

    return hr;
}

/******************************************************************
* Creates an open file dialog box and locate the image to decode. *
******************************************************************/

BOOL DemoApp::LocateImageFile(HWND hWnd, LPWSTR pszFileName, DWORD cchFileName)
{
    pszFileName[0] = L'\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hWnd;
    ofn.lpstrFilter     = L"All Image Files\0"              L"*.bmp;*.dib;*.wdp;*.mdp;*.hdp;*.gif;*.png;*.jpg;*.jpeg;*.tif;*.ico\0"
                          L"Windows Bitmap\0"               L"*.bmp;*.dib\0"
                          L"High Definition Photo\0"        L"*.wdp;*.mdp;*.hdp\0"
                          L"Graphics Interchange Format\0"  L"*.gif\0"
                          L"Portable Network Graphics\0"    L"*.png\0"
                          L"JPEG File Interchange Format\0" L"*.jpg;*.jpeg\0"
                          L"Tiff File\0"                    L"*.tif\0"
                          L"Icon\0"                         L"*.ico\0"
                          L"All Files\0"                    L"*.*\0"
                          L"\0";
    ofn.lpstrFile       = pszFileName;
    ofn.nMaxFile        = cchFileName;
    ofn.lpstrTitle      = L"Open Image";
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    // Display the Open dialog box. 
    return GetOpenFileName(&ofn);
}


/*******************************************************************************
*  Scales original IWICBitmapSource to the client rect size  and convert       *
*  the pixel format. Store the converted bitmap as *ppToRenderBitmapSource     *
********************************************************************************/

HRESULT DemoApp::ConvertBitmapSource(HWND hWnd, IWICBitmapSource **ppToRenderBitmapSource)
{
    HRESULT hr = S_OK;

    *ppToRenderBitmapSource = NULL;

    // Get the client Rect
    RECT rcClient;

    hr = GetClientRect(hWnd, &rcClient) ? S_OK: E_FAIL;

    if (SUCCEEDED(hr))
    {
        // Create a BitmapScaler
        IWICBitmapScaler *pScaler = NULL;

        hr = m_pIWICFactory->CreateBitmapScaler(&pScaler);

        // Initialize the bitmap scaler from the original bitmap map bits
        if (SUCCEEDED(hr))
        {
            hr = pScaler->Initialize(
                m_pOriginalBitmapSource, 
                rcClient.right - rcClient.left, 
                rcClient.bottom - rcClient.top, 
                WICBitmapInterpolationModeFant);
        }

        // Format convert the bitmap into 32bppBGR, a convenient 
        // pixel format for GDI+ rendering 
        if (SUCCEEDED(hr))
        {
            IWICFormatConverter *pConverter = NULL;

            hr = m_pIWICFactory->CreateFormatConverter(&pConverter);

            // Format convert to 32bppBGR 
            if (SUCCEEDED(hr))
            {
                hr = pConverter->Initialize(
                    pScaler,                         // Input bitmap to convert
                    GUID_WICPixelFormat32bppBGR,     // Destination pixel format
                    WICBitmapDitherTypeNone,         // Specified dither patterm
                    NULL,                            // Specify a particular palette 
                    0.f,                             // Alpha threshold
                    WICBitmapPaletteTypeCustom       // Palette translation type
                    );

                // Store the converted bitmap as ppToRenderBitmapSource 
                if (SUCCEEDED(hr))
                {
                    hr = pConverter->QueryInterface(
                        IID_PPV_ARGS(ppToRenderBitmapSource));
                }
            }

            SafeRelease(pConverter);
        }

        SafeRelease(pScaler);
    }

    return hr;
}


/******************************************************************
*  Creates a DIB Section from the converted IWICBitmapSource      *
******************************************************************/

HRESULT DemoApp::CreateDIBSectionFromBitmapSource(IWICBitmapSource *pToRenderBitmapSource)
{
    HRESULT hr = S_OK;

    UINT width = 0;
    UINT height = 0;

    // Check BitmapSource format
    WICPixelFormatGUID pixelFormat;
    hr = pToRenderBitmapSource->GetPixelFormat(&pixelFormat);

    if (SUCCEEDED(hr))
    {
        hr = (pixelFormat == GUID_WICPixelFormat32bppBGR) ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = pToRenderBitmapSource->GetSize(&width, &height); 
    }

    UINT cbStride = 0;
    if (SUCCEEDED(hr))
    {
        // Size of a scan line represented in bytes: 4 bytes each pixel
        hr = UIntMult(width, sizeof(Gdiplus::ARGB), &cbStride);
    }

    UINT cbBufferSize = 0;
    if (SUCCEEDED(hr))
    {
        // Size of the image, represented in bytes
        hr = UIntMult(cbStride, height, &cbBufferSize);
    }

    if (SUCCEEDED(hr))
    {
        // Make sure to free previously allocated buffer and bitmap
        DeleteBufferAndBmp();
        m_pbBuffer  = new BYTE[cbBufferSize];

        hr = (m_pbBuffer) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {

            WICRect rc = { 0, 0, width, height };

            // Extract the image into the GDI+ Bitmap
            hr = pToRenderBitmapSource->CopyPixels(
                &rc,
                cbStride,
                cbBufferSize,
                m_pbBuffer
                );

            if (SUCCEEDED(hr))
            {
                m_pGdiPlusBitmap = new Gdiplus::Bitmap(
                    width,
                    height,
                    cbStride,
                    PixelFormat32bppRGB,
                    m_pbBuffer
                    );
                 
                hr = m_pGdiPlusBitmap ? S_OK : E_FAIL;

                if (SUCCEEDED(hr))
                { 
                    if (m_pGdiPlusBitmap->GetLastStatus() != Gdiplus::Ok)
                    {
                        hr = E_FAIL;
                    }
                }
            }

            if (FAILED(hr))
            {
                DeleteBufferAndBmp();
            }
        }
    }

    return hr;
}

/******************************************************************
*  Clean up Bitmap resources                                      *
******************************************************************/

void DemoApp::DeleteBufferAndBmp()
{

    delete m_pGdiPlusBitmap;
    m_pGdiPlusBitmap = NULL;

    delete[] m_pbBuffer;
    m_pbBuffer = NULL;

}

/******************************************************************
*  Registered window message handler                              *
******************************************************************/

LRESULT CALLBACK DemoApp::s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DemoApp *pThis;
    LRESULT lRet = 0;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT> (lParam);
        pThis = reinterpret_cast<DemoApp *> (pcs->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pThis));
        lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else
    {
        pThis = reinterpret_cast<DemoApp *> (GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis)
        {
            lRet = pThis->WndProc(hWnd, uMsg, wParam, lParam);
        }
        else
        {
            lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    return lRet;
}

/******************************************************************
*  Internal Window message handler                                *
******************************************************************/

LRESULT DemoApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            // Parse the menu selections:
            switch (LOWORD(wParam))
            {
                case IDM_FILE:
                {
                    if (SUCCEEDED(CreateDIBFromFile(hWnd)))
                    {
                        InvalidateRect(hWnd, NULL, TRUE);
                    }
                    else
                    {
                        MessageBox(NULL, L"Failed to load image, select another one.", L"Application Error", MB_ICONEXCLAMATION | MB_OK);
                    }
                    break;
                }
                case IDM_EXIT:
                {
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
                }
            }
            break;
         }
        case WM_SIZE:
        {
            IWICBitmapSource *pToRenderBitmapSource = NULL;

            if (SUCCEEDED(ConvertBitmapSource(hWnd, &pToRenderBitmapSource)))
            {
                CreateDIBSectionFromBitmapSource(pToRenderBitmapSource);
                SafeRelease(pToRenderBitmapSource);
            }
            break;
        }
        case WM_PAINT:
        {
            return OnPaint(hWnd);
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/******************************************************************
* Rendering callback using GDI+                                   *
******************************************************************/

LRESULT DemoApp::OnPaint(HWND hWnd)
{
    LRESULT lRet = 1;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    if (hdc)
    {       
        Gdiplus::Graphics graphics(hdc);

        if (m_pGdiPlusBitmap)
        {
            // Create rendering area
            Gdiplus::SizeF sizef = Gdiplus::SizeF(
                (Gdiplus::REAL)m_pGdiPlusBitmap->GetWidth(),
                (Gdiplus::REAL)m_pGdiPlusBitmap->GetHeight());

            Gdiplus::RectF rcClient = Gdiplus::RectF(Gdiplus::PointF(0,0), sizef);
            // Render the Bitmap
            graphics.DrawImage(m_pGdiPlusBitmap, rcClient);

            lRet = 0;
        }

        EndPaint(hWnd, &ps);
    }

    return lRet;
}  
