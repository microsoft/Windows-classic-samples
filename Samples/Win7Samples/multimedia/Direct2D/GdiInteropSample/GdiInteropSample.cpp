// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "GdiInteropSample.h"

/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
******************************************************************/

int WINAPI WinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpCmdLine*/,
    int /*nCmdShow*/
    )
{
    // Ignore the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
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
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

DemoApp::DemoApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pDCRT(NULL),
    m_pBlackBrush(NULL)
{
}

/******************************************************************
*                                                                 *
*  DemoApp::~DemoApp destructor                                   *
*                                                                 *
*  Tear down resources                                            *
*                                                                 *
******************************************************************/

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pDCRT);
    SafeRelease(&m_pBlackBrush);
}

/******************************************************************
*                                                                 *
*  DemoApp::Initialize                                            *
*                                                                 *
*  Create the application window and device-independent resources *
*                                                                 *
******************************************************************/
HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = L"D2DDemoApp";

        RegisterClassEx(&wcex);

        // Create the application window.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"Direct2D Demo App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);

            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceIndependentResources                      *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app.                                           *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    // Create D2D factory
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceResources                                 *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pDCRT)
    {
        // Create a DC render target.
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_IGNORE),
            0,
            0,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT
            );

        hr = m_pD2DFactory->CreateDCRenderTarget(&props, &m_pDCRT);
        if (SUCCEEDED(hr))
        {
            // Create a black brush.
            hr = m_pDCRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBlackBrush
                );
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::DiscardDeviceResources                                *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a D3D device is lost                                      *
*                                                                 *
******************************************************************/

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pDCRT);
    SafeRelease(&m_pBlackBrush);
}

/******************************************************************
*                                                                 *
*  DemoApp::RunMessageLoop                                        *
*                                                                 *
*  Main window message loop                                       *
*                                                                 *
******************************************************************/

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  This method draws Direct2D content to a GDI HDC.               *
*                                                                 *
*  This method will automatically discard device-specific         *
*  resources if the D3D device disappears during function         *
*  invocation, and will recreate the resources the next time it's *
*  invoked.                                                       *
*                                                                 *
******************************************************************/
HRESULT DemoApp::OnRender(const PAINTSTRUCT &ps)
{

    HRESULT hr;
    RECT rc;

    // Get the dimensions of the client drawing area.
    GetClientRect(m_hwnd, &rc);

    //
    // Draw the pie chart with Direct2D.
    //

    // Create the DC render target.
    hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        // Bind the DC to the DC render target.
        hr = m_pDCRT->BindDC(ps.hdc, &rc);

        m_pDCRT->BeginDraw();

        m_pDCRT->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pDCRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

        m_pDCRT->DrawEllipse(
            D2D1::Ellipse(
                D2D1::Point2F(150.0f, 150.0f),
                100.0f,
                100.0f),
            m_pBlackBrush,
            3.0
            );

        m_pDCRT->DrawLine(
            D2D1::Point2F(150.0f, 150.0f),
            D2D1::Point2F(
                (150.0f + 100.0f * 0.15425f),
                (150.0f - 100.0f * 0.988f)),
            m_pBlackBrush,
            3.0
            );

        m_pDCRT->DrawLine(
            D2D1::Point2F(150.0f, 150.0f),
            D2D1::Point2F(
                (150.0f + 100.0f * 0.525f),
                (150.0f + 100.0f * 0.8509f)),
            m_pBlackBrush,
            3.0
            );

        m_pDCRT->DrawLine(
            D2D1::Point2F(150.0f, 150.0f),
            D2D1::Point2F(
                (150.0f - 100.0f * 0.988f),
                (150.0f - 100.0f * 0.15425f)),
            m_pBlackBrush,
            3.0
            );

        hr = m_pDCRT->EndDraw();
        if (SUCCEEDED(hr))
        {
            //
            // Draw the pie chart with GDI.
            //

            // Save the original object.
            HGDIOBJ original = NULL;
            original = SelectObject(
                ps.hdc,
                GetStockObject(DC_PEN)
                );

            HPEN blackPen = CreatePen(PS_SOLID, 3, 0);
            SelectObject(ps.hdc, blackPen);

            Ellipse(ps.hdc, 300, 50, 500, 250);

            POINT pntArray1[2];
            pntArray1[0].x = 400;
            pntArray1[0].y = 150;
            pntArray1[1].x = static_cast<LONG>(400 + 100 * 0.15425);
            pntArray1[1].y = static_cast<LONG>(150 - 100 * 0.9885);

            POINT pntArray2[2];
            pntArray2[0].x = 400;
            pntArray2[0].y = 150;
            pntArray2[1].x = static_cast<LONG>(400 + 100 * 0.525);
            pntArray2[1].y = static_cast<LONG>(150 + 100 * 0.8509);


            POINT pntArray3[2];
            pntArray3[0].x = 400;
            pntArray3[0].y = 150;
            pntArray3[1].x = static_cast<LONG>(400 - 100 * 0.988);
            pntArray3[1].y = static_cast<LONG>(150 - 100 * 0.15425);

            Polyline(ps.hdc, pntArray1, 2);
            Polyline(ps.hdc, pntArray2, 2);
            Polyline(ps.hdc, pntArray3, 2);

            DeleteObject(blackPen);

            // Restore the original object.
            SelectObject(ps.hdc, original);
        }
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  DemoApp::WndProc                                               *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pDemoApp)
            );

        result = 1;
    }
    else
    {
        DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                ));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch (message)
            {
            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);
                    pDemoApp->OnRender(ps);
                    EndPaint(hwnd, &ps);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}
