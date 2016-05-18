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
#include <d2d1.h>

#include "WICAnimatedGif.h"

const UINT DELAY_TIMER_ID = 1;    // Global ID for the timer, only one timer is used

// Utility inline functions

template <typename T>
inline void SafeRelease(T *&pI)
{
    if (NULL != pI)
    {
        pI->Release();
        pI = NULL;
    }
}

inline LONG RectWidth(RECT rc)
{
    return rc.right - rc.left;
}

inline LONG RectHeight(RECT rc)
{
    return rc.bottom - rc.top;
}


//                           Gif Animation Overview
// In order to play a gif animation, raw frames (which are compressed frames 
// directly retrieved from the image file) and image metadata are loaded 
// and used to compose the frames that are actually displayed in the animation 
// loop (which we call composed frames in this sample).  Composed frames have 
// the same sizes as the global gif image size, while raw frames can have their own sizes.
//
// At the highest level, a gif animation contains a fixed or infinite number of animation
// loops, in which the animation will be displayed repeatedly frame by frame; once all 
// loops are displayed, the animation will stop and the last frame will be displayed 
// from that point.
//
// In each loop, first the entire composed frame will be initialized with the background 
// color retrieved from the image metadata.  The very first raw frame then will be loaded 
// and directly overlaid onto the previous composed frame (i.e. in this case, the frame 
// cleared with background color) to produce the first  composed frame, and this frame 
// will then be displayed for a period that equals its delay.  For any raw frame after 
// the first raw frame (if there are any), the composed frame will first be disposed based 
// on the disposal method associated with the previous raw frame. Then the next raw frame 
// will be loaded and overlaid onto the result (i.e. the composed frame after disposal).  
// These two steps (i.e. disposing the previous frame and overlaying the current frame) together 
// 'compose' the next frame to be displayed.  The composed frame then gets displayed.  
// This process continues until the last frame in a loop is reached.
//
// An exception is the zero delay intermediate frames, which are frames with 0 delay 
// associated with them.  These frames will be used to compose the next frame, but the 
// difference is that the composed frame will not be displayed unless it's the last frame 
// in the loop (i.e. we move immediately to composing the next composed frame).


/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
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
        {
            DemoApp app;
            hr = app.Initialize(hInstance);
            if (SUCCEEDED(hr))
            {
                // Main message loop:
                MSG msg;
                while (GetMessage(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        CoUninitialize();
    }

    return 0;
}

/******************************************************************
*                                                                 *
*  DemoApp::DemoApp constructor                                   *
*                                                                 *
*  Initializes member data                                        *
*                                                                 *
******************************************************************/

DemoApp::DemoApp()
:
    m_hWnd(NULL), 
    m_pD2DFactory(NULL),
    m_pHwndRT(NULL),
    m_pFrameComposeRT(NULL),
    m_pRawFrame(NULL),
    m_pSavedFrame(NULL),
    m_pIWICFactory(NULL),
    m_pDecoder(NULL)
{
}

/******************************************************************
*                                                                 *
*  DemoApp::~DemoApp destructor                                   *
*                                                                 *
*  Tears down resources                                           *
*                                                                 *
******************************************************************/

DemoApp::~DemoApp()
{
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pHwndRT);
    SafeRelease(m_pFrameComposeRT);
    SafeRelease(m_pRawFrame);
    SafeRelease(m_pSavedFrame);
    SafeRelease(m_pIWICFactory);
    SafeRelease(m_pDecoder);
}

/******************************************************************
*                                                                 *
*  DemoApp::Initialize                                            *
*                                                                 *
*  Creates application window and device-independent resources    *
*                                                                 *
******************************************************************/

HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
    // Register window class
    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = DemoApp::s_WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = hInstance;
    wcex.hIcon         = NULL;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_WICANIMATEDGIF);
    wcex.lpszClassName = L"WICANIMATEDGIF";
    wcex.hIconSm       = NULL;

    HRESULT hr = (RegisterClassEx(&wcex) == 0) ? E_FAIL : S_OK;

    if (SUCCEEDED(hr))
    {
        // Create D2D factory
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    }

    if (SUCCEEDED(hr))
    {
        // Create WIC factory
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pIWICFactory));
    }

    if (SUCCEEDED(hr))
    {
        // Create window
        m_hWnd = CreateWindow(
            L"WICANIMATEDGIF",
            L"WIC Animated Gif Sample",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            hInstance,
            this);

        hr = (m_hWnd == NULL) ? E_FAIL : S_OK;
    }

    if (SUCCEEDED(hr))
    {
        hr = SelectAndDisplayGif();
        if (FAILED(hr))
        {
            DestroyWindow(m_hWnd);
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceResources                                 *
*                                                                 *
*  Creates a D2D hwnd render target for displaying gif frames     *
*  to users and a D2D bitmap render for composing frames.         *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    RECT rcClient;
    if (!GetClientRect(m_hWnd, &rcClient))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        if (m_pHwndRT == NULL)
        {
            // Create a D2D hwnd render target
            D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties 
                = D2D1::RenderTargetProperties();

            // Set the DPI to be the default system DPI to allow direct mapping
            // between image pixels and desktop pixels in different system DPI
            // settings
            renderTargetProperties.dpiX = DEFAULT_DPI;
            renderTargetProperties.dpiY = DEFAULT_DPI;

            D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRenderTragetproperties
                = D2D1::HwndRenderTargetProperties(m_hWnd,
                    D2D1::SizeU(RectWidth(rcClient), RectHeight(rcClient)));

            hr = m_pD2DFactory->CreateHwndRenderTarget(
                renderTargetProperties,
                hwndRenderTragetproperties,
                &m_pHwndRT);
        }
        else
        {
            // We already have a hwnd render target, resize it to the window size
            D2D1_SIZE_U size;
            size.width = RectWidth(rcClient);
            size.height = RectHeight(rcClient);
            hr = m_pHwndRT->Resize(size);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Create a bitmap render target used to compose frames. Bitmap render 
        // targets cannot be resized, so we always recreate it.
        SafeRelease(m_pFrameComposeRT);
        hr = m_pHwndRT->CreateCompatibleRenderTarget(
            D2D1::SizeF(
                static_cast<FLOAT>(m_cxGifImage),
                static_cast<FLOAT>(m_cyGifImage)),
            &m_pFrameComposeRT);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window.                                                        *
*                                                                 *
*  Renders the pre-composed frame by drawing it onto the hwnd     *
*  render target.                                                 *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnRender()
{
    HRESULT hr = S_OK;
    ID2D1Bitmap *pFrameToRender = NULL;

    // Check to see if the render targets are initialized
    if (m_pHwndRT && m_pFrameComposeRT)
    {
        if (SUCCEEDED(hr))
        {
            // Only render when the window is not occluded
            if (!(m_pHwndRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
            {
                D2D1_RECT_F drawRect;
                hr = CalculateDrawRectangle(drawRect);

                if (SUCCEEDED(hr))
                {
                    // Get the bitmap to draw on the hwnd render target
                    hr = m_pFrameComposeRT->GetBitmap(&pFrameToRender);
                }

                if (SUCCEEDED(hr))
                {
                    // Draw the bitmap onto the calculated rectangle
                    m_pHwndRT->BeginDraw();

                    m_pHwndRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
                    m_pHwndRT->DrawBitmap(pFrameToRender, drawRect);

                    hr = m_pHwndRT->EndDraw();
                }
            }
        }
    }

    SafeRelease(pFrameToRender);
    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::GetFileOpen                                           *
*                                                                 *
*  Creates an open file dialog box and returns the filename       *
*  of the file selected(if any).                                  *  
*                                                                 *
******************************************************************/

BOOL DemoApp::GetFileOpen(WCHAR *pszFileName, DWORD cchFileName)
{
    pszFileName[0] = L'\0';

    OPENFILENAME ofn;
    RtlZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = m_hWnd;
    ofn.lpstrFilter     = L"*Gif Files\0*.gif\0";
    ofn.lpstrFile       = pszFileName;
    ofn.nMaxFile        = cchFileName;
    ofn.lpstrTitle      = L"Select an image to display...";
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    // Display the Open dialog box.
    return GetOpenFileName(&ofn);
}

/******************************************************************
*                                                                 *
*  DemoApp::OnResize                                              *
*                                                                 *
*  If the application receives a WM_SIZE message, this method     *
*  will resize the render target appropriately.                   *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnResize(UINT uWidth, UINT uHeight)
{
    HRESULT hr = S_OK;

    if (m_pHwndRT)
    {
        D2D1_SIZE_U size;
        size.width = uWidth;
        size.height = uHeight;
        hr = m_pHwndRT->Resize(size);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::s_WndProc                                             *
*                                                                 *
*  Static window message handler used to initialize the           *
*  application object and call the object's member WndProc        * 
*                                                                 *
******************************************************************/

LRESULT CALLBACK DemoApp::s_WndProc(
    HWND hWnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    DemoApp *pThis = NULL;
    LRESULT lRet = 0;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<DemoApp *>(pcs->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pThis));
        lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else
    {
        pThis = reinterpret_cast<DemoApp *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
*                                                                 *
*  DemoApp::WndProc                                               *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

LRESULT DemoApp::WndProc(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case WM_COMMAND:
        {
            // Parse the menu selections
            switch (LOWORD(wParam))
            {
            case IDM_FILE:
                hr = SelectAndDisplayGif();
                if (FAILED(hr))
                {
                    MessageBox(hWnd, L"Load gif file failed. Exiting application.", L"Error", MB_OK);
                    PostQuitMessage(1);
                    return 1;
                }
                break;

            case IDM_EXIT:
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            }
        }
        break;

    case WM_SIZE:
        {
            UINT uWidth = LOWORD(lParam);
            UINT uHeight = HIWORD(lParam);
            hr = OnResize(uWidth, uHeight);
        }
        break;

    case WM_PAINT:
        {
            hr = OnRender(); 
            ValidateRect(hWnd, NULL);
        }
        break;

    case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;

    case WM_TIMER:
        {
            // Timer expired, display the next frame and set a new timer
            // if needed
            hr = ComposeNextFrame();
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    // In case of a device loss, recreate all the resources and start playing
    // gif from the beginning
    //
    // In case of other errors from resize, paint, and timer event, we will
    // try our best to continue displaying the animation
    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = RecoverDeviceResources();
        if (FAILED(hr))
        {
            MessageBox(hWnd, L"Device loss recovery failed. Exiting application.", L"Error", MB_OK);
            PostQuitMessage(1);
        }
    }

    return 0;
}

/******************************************************************
*                                                                 *
*  DemoApp::GetGlobalMetadata()                                   *
*                                                                 *
*  Retrieves global metadata which pertains to the entire image.  *
*                                                                 *
******************************************************************/

HRESULT DemoApp::GetGlobalMetadata()
{
    PROPVARIANT propValue;
    PropVariantInit(&propValue);
    IWICMetadataQueryReader *pMetadataQueryReader = NULL;

    // Get the frame count
    HRESULT hr = m_pDecoder->GetFrameCount(&m_cFrames);
    if (SUCCEEDED(hr))
    {
        // Create a MetadataQueryReader from the decoder
        hr = m_pDecoder->GetMetadataQueryReader(
            &pMetadataQueryReader);
    }

    if (SUCCEEDED(hr))
    {
        // Get background color
        if(FAILED(GetBackgroundColor(pMetadataQueryReader)))
        {
            // Default to transparent if failed to get the color
            m_backgroundColor = D2D1::ColorF(0, 0.f);
        }
    }

    // Get global frame size
    if (SUCCEEDED(hr))
    {
        // Get width
        hr = pMetadataQueryReader->GetMetadataByName(
            L"/logscrdesc/Width", 
            &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                m_cxGifImage = propValue.uiVal;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Get height
        hr = pMetadataQueryReader->GetMetadataByName(
            L"/logscrdesc/Height",
            &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                m_cyGifImage = propValue.uiVal;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Get pixel aspect ratio
        hr = pMetadataQueryReader->GetMetadataByName(
            L"/logscrdesc/PixelAspectRatio",
            &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI1 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                UINT uPixelAspRatio = propValue.bVal;

                if (uPixelAspRatio != 0)
                {
                    // Need to calculate the ratio. The value in uPixelAspRatio 
                    // allows specifying widest pixel 4:1 to the tallest pixel of 
                    // 1:4 in increments of 1/64th
                    FLOAT pixelAspRatio = (uPixelAspRatio + 15.f) / 64.f;

                    // Calculate the image width and height in pixel based on the
                    // pixel aspect ratio. Only shrink the image.
                    if (pixelAspRatio > 1.f)
                    {
                        m_cxGifImagePixel = m_cxGifImage;
                        m_cyGifImagePixel = static_cast<UINT>(m_cyGifImage / pixelAspRatio);
                    }
                    else
                    {
                        m_cxGifImagePixel = static_cast<UINT>(m_cxGifImage * pixelAspRatio);
                        m_cyGifImagePixel = m_cyGifImage;
                    }
                }
                else
                {
                    // The value is 0, so its ratio is 1
                    m_cxGifImagePixel = m_cxGifImage;
                    m_cyGifImagePixel = m_cyGifImage;
                }
            }
            PropVariantClear(&propValue);
        }
    }

    // Get looping information
    if (SUCCEEDED(hr))
    {
        // First check to see if the application block in the Application Extension
        // contains "NETSCAPE2.0" and "ANIMEXTS1.0", which indicates the gif animation
        // has looping information associated with it.
        // 
        // If we fail to get the looping information, loop the animation infinitely.
        if (SUCCEEDED(pMetadataQueryReader->GetMetadataByName(
                    L"/appext/application", 
                    &propValue)) &&
            propValue.vt == (VT_UI1 | VT_VECTOR) &&
            propValue.caub.cElems == 11 &&  // Length of the application block
            (!memcmp(propValue.caub.pElems, "NETSCAPE2.0", propValue.caub.cElems) ||
             !memcmp(propValue.caub.pElems, "ANIMEXTS1.0", propValue.caub.cElems)))
        {
            PropVariantClear(&propValue);

            hr = pMetadataQueryReader->GetMetadataByName(L"/appext/data", &propValue);
            if (SUCCEEDED(hr))
            {
                //  The data is in the following format:
                //  byte 0: extsize (must be > 1)
                //  byte 1: loopType (1 == animated gif)
                //  byte 2: loop count (least significant byte)
                //  byte 3: loop count (most significant byte)
                //  byte 4: set to zero
                if (propValue.vt == (VT_UI1 | VT_VECTOR) &&
                    propValue.caub.cElems >= 4 &&
                    propValue.caub.pElems[0] > 0 &&
                    propValue.caub.pElems[1] == 1)
                {
                    m_uTotalLoopCount = MAKEWORD(propValue.caub.pElems[2], 
                        propValue.caub.pElems[3]);
                    
                    // If the total loop count is not zero, we then have a loop count
                    // If it is 0, then we repeat infinitely
                    if (m_uTotalLoopCount != 0) 
                    {
                        m_fHasLoop = TRUE;
                    }
                }
            }
        }
    }

    PropVariantClear(&propValue);
    SafeRelease(pMetadataQueryReader);
    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::GetRawFrame()                                         *
*                                                                 *
*  Decodes the current raw frame, retrieves its timing            *
*  information, disposal method, and frame dimension for          *
*  rendering.  Raw frame is the frame read directly from the gif  *
*  file without composing.                                        *
*                                                                 *
******************************************************************/

HRESULT DemoApp::GetRawFrame(UINT uFrameIndex)
{
    IWICFormatConverter *pConverter = NULL;
    IWICBitmapFrameDecode *pWicFrame = NULL;
    IWICMetadataQueryReader *pFrameMetadataQueryReader = NULL;
    
    PROPVARIANT propValue;
    PropVariantInit(&propValue);

    // Retrieve the current frame
    HRESULT hr = m_pDecoder->GetFrame(uFrameIndex, &pWicFrame);
    if (SUCCEEDED(hr))
    {
        // Format convert to 32bppPBGRA which D2D expects
        hr = m_pIWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(
            pWicFrame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeCustom);
    }

    if (SUCCEEDED(hr))
    {
        // Create a D2DBitmap from IWICBitmapSource
        SafeRelease(m_pRawFrame);
        hr = m_pHwndRT->CreateBitmapFromWicBitmap(
            pConverter,
            NULL,
            &m_pRawFrame);
    }

    if (SUCCEEDED(hr))
    {
        // Get Metadata Query Reader from the frame
        hr = pWicFrame->GetMetadataQueryReader(&pFrameMetadataQueryReader);
    }

    // Get the Metadata for the current frame
    if (SUCCEEDED(hr))
    {
        hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Left", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                m_framePosition.left = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Top", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                m_framePosition.top = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Width", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                m_framePosition.right = static_cast<FLOAT>(propValue.uiVal) 
                    + m_framePosition.left;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Height", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                m_framePosition.bottom = static_cast<FLOAT>(propValue.uiVal)
                    + m_framePosition.top;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Get delay from the optional Graphic Control Extension
        if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
            L"/grctlext/Delay", 
            &propValue)))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                // Convert the delay retrieved in 10 ms units to a delay in 1 ms units
                hr = UIntMult(propValue.uiVal, 10, &m_uFrameDelay);
            }
            PropVariantClear(&propValue);
        }
        else
        {
            // Failed to get delay from graphic control extension. Possibly a
            // single frame image (non-animated gif)
            m_uFrameDelay = 0;
        }

        if (SUCCEEDED(hr))
        {
            // Insert an artificial delay to ensure rendering for gif with very small
            // or 0 delay.  This delay number is picked to match with most browsers' 
            // gif display speed.
            //
            // This will defeat the purpose of using zero delay intermediate frames in 
            // order to preserve compatibility. If this is removed, the zero delay 
            // intermediate frames will not be visible.
            if (m_uFrameDelay < 90)
            {
                m_uFrameDelay = 90;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
                L"/grctlext/Disposal", 
                &propValue)))
        {
            hr = (propValue.vt == VT_UI1) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                m_uFrameDisposal = propValue.bVal;
            }
        }
        else
        {
            // Failed to get the disposal method, use default. Possibly a 
            // non-animated gif.
            m_uFrameDisposal = DM_UNDEFINED;
        }
    }

    PropVariantClear(&propValue);

    SafeRelease(pConverter);
    SafeRelease(pWicFrame);
    SafeRelease(pFrameMetadataQueryReader);

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::GetBackgroundColor()                                  *
*                                                                 *
*  Reads and stores the background color for gif.                 *
*                                                                 *
******************************************************************/

HRESULT DemoApp::GetBackgroundColor(
    IWICMetadataQueryReader *pMetadataQueryReader)
{
    DWORD dwBGColor;
    BYTE backgroundIndex = 0;
    WICColor rgColors[256];
    UINT cColorsCopied = 0;
    PROPVARIANT propVariant;
    PropVariantInit(&propVariant);
    IWICPalette *pWicPalette = NULL;

    // If we have a global palette, get the palette and background color
    HRESULT hr = pMetadataQueryReader->GetMetadataByName(
        L"/logscrdesc/GlobalColorTableFlag",
        &propVariant);
    if (SUCCEEDED(hr))
    {
        hr = (propVariant.vt != VT_BOOL || !propVariant.boolVal) ? E_FAIL : S_OK;
        PropVariantClear(&propVariant);
    }

    if (SUCCEEDED(hr))
    {
        // Background color index
        hr = pMetadataQueryReader->GetMetadataByName(
            L"/logscrdesc/BackgroundColorIndex", 
            &propVariant);
        if (SUCCEEDED(hr))
        {
            hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
            if (SUCCEEDED(hr))
            {
                backgroundIndex = propVariant.bVal;
            }
            PropVariantClear(&propVariant);
        }
    }

    // Get the color from the palette
    if (SUCCEEDED(hr))
    {
        hr = m_pIWICFactory->CreatePalette(&pWicPalette);
    }

    if (SUCCEEDED(hr))
    {
        // Get the global palette
        hr = m_pDecoder->CopyPalette(pWicPalette);
    }

    if (SUCCEEDED(hr))
    {
        hr = pWicPalette->GetColors(
            ARRAYSIZE(rgColors),
            rgColors,
            &cColorsCopied);
    }

    if (SUCCEEDED(hr))
    {
        // Check whether background color is outside range 
        hr = (backgroundIndex >= cColorsCopied) ? E_FAIL : S_OK;
    }

    if (SUCCEEDED(hr))
    {
        // Get the color in ARGB format
        dwBGColor = rgColors[backgroundIndex];

        // The background color is in ARGB format, and we want to 
        // extract the alpha value and convert it in FLOAT
        FLOAT alpha = (dwBGColor >> 24) / 255.f;
        m_backgroundColor = D2D1::ColorF(dwBGColor, alpha);
    }

    SafeRelease(pWicPalette);
    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CalculateDrawRectangle()                              *
*                                                                 *
*  Calculates a specific rectangular area of the hwnd             *
*  render target to draw a bitmap containing the current          *
*  composed frame.                                                *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CalculateDrawRectangle(D2D1_RECT_F &drawRect)
{
    HRESULT hr = S_OK;
    RECT rcClient;

    // Top and left of the client rectangle are both 0
    if (!GetClientRect(m_hWnd, &rcClient))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the area to display the image
        // Center the image if the client rectangle is larger
        drawRect.left = (static_cast<FLOAT>(rcClient.right) - m_cxGifImagePixel) / 2.f;
        drawRect.top = (static_cast<FLOAT>(rcClient.bottom) - m_cyGifImagePixel) / 2.f;
        drawRect.right = drawRect.left + m_cxGifImagePixel;
        drawRect.bottom = drawRect.top + m_cyGifImagePixel;

        // If the client area is resized to be smaller than the image size, scale
        // the image, and preserve the aspect ratio
        FLOAT aspectRatio = static_cast<FLOAT>(m_cxGifImagePixel) /
            static_cast<FLOAT>(m_cyGifImagePixel);

        if (drawRect.left < 0)
        {
            FLOAT newWidth = static_cast<FLOAT>(rcClient.right);
            FLOAT newHeight = newWidth / aspectRatio;
            drawRect.left = 0;
            drawRect.top = (static_cast<FLOAT>(rcClient.bottom) - newHeight) / 2.f;
            drawRect.right = newWidth;
            drawRect.bottom = drawRect.top + newHeight;
        }

        if (drawRect.top < 0)
        {
            FLOAT newHeight = static_cast<FLOAT>(rcClient.bottom);
            FLOAT newWidth = newHeight * aspectRatio;
            drawRect.left = (static_cast<FLOAT>(rcClient.right) - newWidth) / 2.f;
            drawRect.top = 0;
            drawRect.right = drawRect.left + newWidth;
            drawRect.bottom = newHeight;
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::RestoreSavedFrame()                                   *
*                                                                 *
*  Copys the saved frame to the frame in the bitmap render        * 
*  target.                                                        *
*                                                                 *
******************************************************************/

HRESULT DemoApp::RestoreSavedFrame()
{
    HRESULT hr = S_OK;

    ID2D1Bitmap *pFrameToCopyTo = NULL;

    hr = m_pSavedFrame ? S_OK : E_FAIL;

    if(SUCCEEDED(hr))
    {
        hr = m_pFrameComposeRT->GetBitmap(&pFrameToCopyTo);
    }

    if (SUCCEEDED(hr))
    {
        // Copy the whole bitmap
        hr = pFrameToCopyTo->CopyFromBitmap(NULL, m_pSavedFrame, NULL);
    }

    SafeRelease(pFrameToCopyTo);

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::ClearCurrentFrameArea()                               *
*                                                                 *
*  Clears a rectangular area equal to the area overlaid by the    *
*  current raw frame in the bitmap render target with background  *
*  color.                                                         *
*                                                                 *
******************************************************************/

HRESULT DemoApp::ClearCurrentFrameArea()
{
    m_pFrameComposeRT->BeginDraw();

    // Clip the render target to the size of the raw frame
    m_pFrameComposeRT->PushAxisAlignedClip(
        &m_framePosition, 
        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    m_pFrameComposeRT->Clear(m_backgroundColor);

    // Remove the clipping
    m_pFrameComposeRT->PopAxisAlignedClip();

    return m_pFrameComposeRT->EndDraw();
}

/******************************************************************
*                                                                 *
*  DemoApp::DisposeCurrentFrame()                                 *
*                                                                 *
*  At the end of each delay, disposes the current frame           *
*  based on the disposal method specified.                        *
*                                                                 *
******************************************************************/

HRESULT DemoApp::DisposeCurrentFrame()
{
    HRESULT hr = S_OK;

    switch (m_uFrameDisposal)
    {
    case DM_UNDEFINED:
    case DM_NONE:
        // We simply draw on the previous frames. Do nothing here.
        break;
    case DM_BACKGROUND:
        // Dispose background
        // Clear the area covered by the current raw frame with background color
        hr = ClearCurrentFrameArea();
        break;
    case DM_PREVIOUS:
        // Dispose previous
        // We restore the previous composed frame first
        hr = RestoreSavedFrame();
        break;
    default:
        // Invalid disposal method
        hr = E_FAIL;
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::OverlayNextFrame()                                    *
*                                                                 *
*  Loads and draws the next raw frame into the composed frame     *
*  render target. This is called after the current frame is       *
*  disposed.                                                      *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OverlayNextFrame()
{
    // Get Frame information
    HRESULT hr = GetRawFrame(m_uNextFrameIndex);
    if (SUCCEEDED(hr))
    {
        // For disposal 3 method, we would want to save a copy of the current
        // composed frame
        if (m_uFrameDisposal == DM_PREVIOUS)
        {
            hr = SaveComposedFrame();
        }
    }

    if (SUCCEEDED(hr))
    {
        // Start producing the next bitmap
        m_pFrameComposeRT->BeginDraw();

        // If starting a new animation loop
        if (m_uNextFrameIndex == 0)
        {
            // Draw background and increase loop count
            m_pFrameComposeRT->Clear(m_backgroundColor);
            m_uLoopNumber++;
        }

        // Produce the next frame
        m_pFrameComposeRT->DrawBitmap(
            m_pRawFrame,
            m_framePosition);

        hr = m_pFrameComposeRT->EndDraw();
    }

    // To improve performance and avoid decoding/composing this frame in the 
    // following animation loops, the composed frame can be cached here in system 
    // or video memory.

    if (SUCCEEDED(hr))
    {
        // Increase the frame index by 1
        m_uNextFrameIndex = (++m_uNextFrameIndex) % m_cFrames;
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::SaveComposedFrame()                                   *
*                                                                 *
*  Saves the current composed frame in the bitmap render target   *
*  into a temporary bitmap. Initializes the temporary bitmap if   *
*  needed.                                                        *
*                                                                 *
******************************************************************/

HRESULT DemoApp::SaveComposedFrame()
{
    HRESULT hr = S_OK;

    ID2D1Bitmap *pFrameToBeSaved = NULL;

    hr = m_pFrameComposeRT->GetBitmap(&pFrameToBeSaved);
    if (SUCCEEDED(hr))
    {
        // Create the temporary bitmap if it hasn't been created yet 
        if (m_pSavedFrame == NULL)
        {
            D2D1_SIZE_U bitmapSize = pFrameToBeSaved->GetPixelSize();
            D2D1_BITMAP_PROPERTIES bitmapProp;
            pFrameToBeSaved->GetDpi(&bitmapProp.dpiX, &bitmapProp.dpiY);
            bitmapProp.pixelFormat = pFrameToBeSaved->GetPixelFormat();

            hr = m_pFrameComposeRT->CreateBitmap(
                bitmapSize,
                bitmapProp,
                &m_pSavedFrame);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Copy the whole bitmap
        hr = m_pSavedFrame->CopyFromBitmap(NULL, pFrameToBeSaved, NULL);
    }

    SafeRelease(pFrameToBeSaved);

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::SelectAndDisplayGif()                                 *
*                                                                 *
*  Opens a dialog and displays a selected image.                  *
*                                                                 *
******************************************************************/

HRESULT DemoApp::SelectAndDisplayGif()
{
    HRESULT hr = S_OK;

    WCHAR szFileName[MAX_PATH];
    RECT rcClient = {};
    RECT rcWindow = {};

    // If the user cancels selection, then nothing happens
    if (GetFileOpen(szFileName, ARRAYSIZE(szFileName)))
    {
        // Reset the states
        m_uNextFrameIndex = 0;
        m_uFrameDisposal = DM_NONE;  // No previous frame, use disposal none
        m_uLoopNumber = 0;
        m_fHasLoop = FALSE;
        SafeRelease(m_pSavedFrame); 

        // Create a decoder for the gif file
        SafeRelease(m_pDecoder);
        hr = m_pIWICFactory->CreateDecoderFromFilename(
            szFileName,
            NULL,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &m_pDecoder);
        if (SUCCEEDED(hr))
        {
            hr = GetGlobalMetadata();
        }

        if (SUCCEEDED(hr))
        {
            rcClient.right = m_cxGifImagePixel;
            rcClient.bottom = m_cyGifImagePixel;

            if (!AdjustWindowRect(&rcClient, WS_OVERLAPPEDWINDOW, TRUE)) 
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (SUCCEEDED(hr))
        {
            // Get the upper left corner of the current window
            if (!GetWindowRect(m_hWnd, &rcWindow)) 
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (SUCCEEDED(hr))
        {
            // Resize the window to fit the gif
            MoveWindow(          
                m_hWnd,
                rcWindow.left,
                rcWindow.top,
                RectWidth(rcClient),
                RectHeight(rcClient),
                TRUE);

            hr = CreateDeviceResources();
        }
        
        if (SUCCEEDED(hr))
        {
            // If we have at least one frame, start playing
            // the animation from the first frame
            if (m_cFrames > 0)
            {
                hr = ComposeNextFrame();
                InvalidateRect(m_hWnd, NULL, FALSE);
            }
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::ComposeNextFrame()                                    *
*                                                                 *
*  Composes the next frame by first disposing the current frame   *
*  and then overlaying the next frame. More than one frame may    *
*  be processed in order to produce the next frame to be          *
*  displayed due to the use of zero delay intermediate frames.    *
*  Also, sets a timer that is equal to the delay of the frame.    *
*                                                                 *
******************************************************************/

HRESULT DemoApp::ComposeNextFrame()
{
    HRESULT hr = S_OK;

    // Check to see if the render targets are initialized
    if (m_pHwndRT && m_pFrameComposeRT)
    {
        // First, kill the timer since the delay is no longer valid
        KillTimer(m_hWnd, DELAY_TIMER_ID);

        // Compose one frame
        hr = DisposeCurrentFrame();
        if (SUCCEEDED(hr))
        {
            hr = OverlayNextFrame();
        }

        // Keep composing frames until we see a frame with delay greater than
        // 0 (0 delay frames are the invisible intermediate frames), or until
        // we have reached the very last frame.
        while (SUCCEEDED(hr) && m_uFrameDelay == 0 && !IsLastFrame())
        {
            hr = DisposeCurrentFrame();
            if (SUCCEEDED(hr))
            {
                hr = OverlayNextFrame();
            }
        }

        // If we have more frames to play, set the timer according to the delay.
        // Set the timer regardless of whether we succeeded in composing a frame
        // to try our best to continue displaying the animation.
        if (!EndOfAnimation() && m_cFrames > 1)
        {
            // Set the timer according to the delay
            SetTimer(m_hWnd, DELAY_TIMER_ID, m_uFrameDelay, NULL);
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::RecoverDeviceResources                                *
*                                                                 *
*  Discards device-specific resources and recreates them.         *
*  Also starts the animation from the beginning.                  *
*                                                                 *
******************************************************************/

HRESULT DemoApp::RecoverDeviceResources()
{
    SafeRelease(m_pHwndRT);
    SafeRelease(m_pFrameComposeRT);
    SafeRelease(m_pSavedFrame);

    m_uNextFrameIndex = 0;
    m_uFrameDisposal = DM_NONE;  // No previous frames. Use disposal none.
    m_uLoopNumber = 0;

    HRESULT hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        if (m_cFrames > 0)
        {
            // Load the first frame
            hr = ComposeNextFrame();
            InvalidateRect(m_hWnd, NULL, FALSE);
        }
    }

    return hr;
}