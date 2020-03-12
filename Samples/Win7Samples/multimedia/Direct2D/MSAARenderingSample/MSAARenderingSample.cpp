// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "MSAARenderingSample.h"

static const UINT sc_msaaSampleCount = 4;

static const PCWSTR sc_fontName = L"Calibri";

static const float sc_fontSize = 20.0f;

static const UINT sc_defaultNumSquares = 16;
static const UINT sc_minNumSquares = 1;
static const UINT sc_maxNumSquares = 1024;

static const float sc_boardWidth = 700.0f;

static const float sc_loupeSize = 300.0f;

static const float sc_loupeDefaultLogZoom = 2.0f;
static const float sc_loupeMinLogZoom = 0.0f;
static const float sc_loupeMaxLogZoom = 4.0f;

static const float sc_rotationSpeed = 0.25f;

static const float sc_loupeInset = 20.0f;

static const float sc_srcLoupeRingThickness = 2.0f;

static const float sc_destLoupeRingThickness = 5.0f;

/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
******************************************************************/

int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
    )
{
    // Ignoring the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            MSAASampleApp app;

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
*  MSAASampleApp::MSAASampleApp                                   *
*                                                                 *
*  Constructor -- initialize member data                          *
*                                                                 *
******************************************************************/

MSAASampleApp::MSAASampleApp() :
    m_hwnd(NULL),
    m_antialiasMode(MyAntialiasMode::Aliased),
    m_sampleType(SampleType::Filled),
    m_paused(false),
    m_pausedTime(0),
    m_drawLoupe(true),
    m_numSquares(sc_defaultNumSquares),
    m_logZoom(sc_loupeDefaultLogZoom*WHEEL_DELTA),
    m_pDevice(NULL),
    m_pSwapChain(NULL),
    m_pState(NULL),
    m_pRenderTargetView(NULL),
    m_pLoupeTexture(NULL),
    m_pLoupeBitmap(NULL),
    m_pBackBufferRT(NULL),
    m_pTextBrush(NULL),
    m_pLoupeBrush(NULL),
    m_pD2DFactory(NULL),
    m_pDWriteFactory(NULL),
    m_pTextFormat(NULL)
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    m_timeDelta = -time.QuadPart;
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::~MSAASampleApp                                  *
*                                                                 *
*  Destructor -- tear down member data                            *
*                                                                 *
******************************************************************/

MSAASampleApp::~MSAASampleApp()
{
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pState);
    SafeRelease(&m_pRenderTargetView);
    SafeRelease(&m_pLoupeTexture);
    SafeRelease(&m_pLoupeBitmap);
    SafeRelease(&m_pBackBufferRT);
    SafeRelease(&m_pTextBrush);
    SafeRelease(&m_pLoupeBrush);
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pTextFormat);
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::Initialize                                      *
*                                                                 *
*  This method is used to create and display the application      *
*  window, and provides a convenient place to create any device   *
*  independent resources that will be required.                   *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::Initialize()
{
    HRESULT hr;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register window class
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = MSAASampleApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.lpszClassName = L"D2DMSAASampleApp";
        RegisterClassEx(&wcex);

        // Create the application window.
        //
        // This sample does not handle resize so we create the window such that
        // it can't be resized.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        //
        FLOAT dpiX;
        FLOAT dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DMSAASampleApp",
            L"D2D Demo App",
            WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(1024.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(768.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            hr = CreateDeviceIndependentResources();
            if (SUCCEEDED(hr))
            {
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);
            }
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::CreateDeviceIndependentResources                *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app.                                           *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::CreateDeviceIndependentResources()
{
    static const WCHAR msc_fontName[] = L"Calibri";
    static const FLOAT msc_fontSize = 50;
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
         hr = m_pDWriteFactory->CreateTextFormat(
            sc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(sc_fontSize),
            L"", // locale
            &m_pTextFormat
            );
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::CreateDeviceResources                           *
*                                                                 *
*  This method is responsible for creating the D3D device and     *
*  all corresponding device-bound resources that are required to  *
*  render.                                                        *
*                                                                 *
*  Of the objects created in this method, 5 are interesting ...   *
*                                                                 *
*     1. D3D Device (m_pDevice) -- This device must support 32bpp *
*           BGRA                                                  *
*                                                                 *
*     2. DXGI Swap Chain (m_pSwapChain) -- Mapped to current      *
*           window (m_hwnd)                                       *
*                                                                 *
*     3. D2D Surface Render Target (m_pBackBufferRT) -- Allows    *
*           us to use D2D to draw directly into a swap chain      *
*           buffer, in order to show (for example) a background   *
*           gradient.                                             *
*                                                                 *
*     4. D3D Offscreen Texture (m_pOffscreenTexture) -- Texture   *
*           which is mapped to our 3D model.                      *
*                                                                 *
*     5. D2D Surface Render Target (m_pBackBufferRT) --           *
*           Allows us to use D2D to draw into the (4) D3D         *
*           offscreen texture. This texture will then be mapped   *
*           and displayed on our 3D model.                        *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    RECT rcClient;
    ID3D10Resource *pBackBufferResource = NULL;
    IDXGIDevice *pDXGIDevice = NULL;
    IDXGIAdapter *pAdapter = NULL;
    IDXGIFactory *pDXGIFactory = NULL;
    IDXGISurface *pSurface = NULL;
    IDXGISurface *pDxgiSurface = NULL;
    IDXGISurface *pResolveSurface = NULL;
    IDXGISurface *pBackBuffer = NULL;

    Assert(m_hwnd);

    GetClientRect(m_hwnd, &rcClient);

    UINT nWidth = abs(rcClient.right - rcClient.left);
    UINT nHeight = abs(rcClient.bottom - rcClient.top);


    // If we don't have a device, need to create one now and all
    // accompanying D3D resources.
    if (!m_pDevice)
    {
        ID3D10Device1 *pDevice = NULL;
        UINT nDeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;

        // Create device
        hr = CreateD3DDevice(
            NULL,
            D3D10_DRIVER_TYPE_HARDWARE,
            nDeviceFlags,
            &pDevice
            );
        if (SUCCEEDED(hr))
        {
            UINT msaaQuality = 0;
            hr = pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, sc_msaaSampleCount, &msaaQuality);
            if (SUCCEEDED(hr) && msaaQuality == 0)
            {
                // If the hardware doesn't support MSAA, fall back to WARP.
                hr = E_FAIL;
                SafeRelease(&pDevice);
            }
        }

        if (FAILED(hr))
        {
            hr = CreateD3DDevice(
                NULL,
                D3D10_DRIVER_TYPE_WARP,
                nDeviceFlags,
                &pDevice
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = pDevice->QueryInterface(&m_pDevice);
        }

        SafeRelease(&pDevice);
    }

    // This sample discards all resources except for the device when
    // transitioning between antialiasing states. While this is not the fastest
    // way to toggle multisampling at runtime, it simplifies the code to reuse
    // the device creation logic here.
    //
    // Given that these transitions discard all resources except for the
    // device, we have different checks for to see if we have a swap chain or a
    // device.
    if (SUCCEEDED(hr) && !m_pSwapChain)
    {
        UINT msaaSampleCount = (m_antialiasMode == MyAntialiasMode::MSAA) ? sc_msaaSampleCount : 1;
        UINT msaaQuality = 0;

        if (SUCCEEDED(hr))
        {
            hr = m_pDevice->QueryInterface(&pDXGIDevice);
        }
        if (SUCCEEDED(hr))
        {
            hr = pDXGIDevice->GetAdapter(&pAdapter);
        }
        if (SUCCEEDED(hr))
        {
            hr = pAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, msaaSampleCount, &msaaQuality);
            if (SUCCEEDED(hr) && msaaQuality == 0)
            {
                hr = E_FAIL;
            }
        }
        if (SUCCEEDED(hr))
        {
            DXGI_SWAP_CHAIN_DESC swapDesc;
            ::ZeroMemory(&swapDesc, sizeof(swapDesc));

            swapDesc.BufferDesc.Width = nWidth;
            swapDesc.BufferDesc.Height = nHeight;
            swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            swapDesc.BufferDesc.RefreshRate.Numerator = 60;
            swapDesc.BufferDesc.RefreshRate.Denominator = 1;
            swapDesc.SampleDesc.Count = msaaSampleCount;
            swapDesc.SampleDesc.Quality = msaaQuality-1;
            swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapDesc.BufferCount = 1;
            swapDesc.OutputWindow = m_hwnd;
            swapDesc.Windowed = TRUE;

            hr = pDXGIFactory->CreateSwapChain(m_pDevice, &swapDesc, &m_pSwapChain);
        }
        if (SUCCEEDED(hr))
        {
            // Create rasterizer state object
            D3D10_RASTERIZER_DESC rsDesc;
            rsDesc.AntialiasedLineEnable = FALSE;
            rsDesc.CullMode = D3D10_CULL_NONE;
            rsDesc.DepthBias = 0;
            rsDesc.DepthBiasClamp = 0;
            rsDesc.DepthClipEnable = TRUE;
            rsDesc.FillMode = D3D10_FILL_SOLID;
            rsDesc.FrontCounterClockwise = FALSE; // Must be FALSE for 10on9
            rsDesc.MultisampleEnable = TRUE;
            rsDesc.ScissorEnable = FALSE;
            rsDesc.SlopeScaledDepthBias = 0;

            hr = m_pDevice->CreateRasterizerState(&rsDesc, &m_pState);
        }
        if (SUCCEEDED(hr))
        {
            m_pDevice->RSSetState(m_pState);

            // The debug layer will output a spurious warning message when restoring
            // state if we leave the state as D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED,
            // so we set it to something else to suppress the warning.
            m_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }
        if (SUCCEEDED(hr))
        {
            // Create views on the RT buffers and set them on the device
            hr = m_pSwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&pBackBufferResource)
                );
        }
        if (SUCCEEDED(hr))
        {
            D3D10_RENDER_TARGET_VIEW_DESC renderDesc;
            renderDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            renderDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
            renderDesc.Texture2D.MipSlice = 0;

            hr = m_pDevice->CreateRenderTargetView(pBackBufferResource, &renderDesc, &m_pRenderTargetView);
        }
        if (SUCCEEDED(hr))
        {
            ID3D10RenderTargetView *viewList[1] = {m_pRenderTargetView};
            m_pDevice->OMSetRenderTargets(1, viewList, NULL);

            // Set a new viewport based on the new dimensions
            D3D10_VIEWPORT viewport;
            viewport.Width = nWidth;
            viewport.Height = nHeight;
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.MinDepth = 0;
            viewport.MaxDepth = 1;
            m_pDevice->RSSetViewports(1, &viewport);

            D3D10_TEXTURE2D_DESC texDesc;
            ::ZeroMemory(&texDesc, sizeof(texDesc));

            texDesc.Width = nWidth;
            texDesc.Height = nHeight;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
            texDesc.Usage = D3D10_USAGE_DEFAULT;

            hr = m_pDevice->CreateTexture2D(&texDesc, NULL, &m_pLoupeTexture);
        }
        if (SUCCEEDED(hr))
        {
            // Get a surface in the swap chain
            hr = m_pSwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&pBackBuffer)
                );
        }
        if (SUCCEEDED(hr))
        {
            FLOAT dpiX;
            FLOAT dpiY;
            m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

            // Create a D2D render target which can draw into the surface in the swap chain
            D2D1_RENDER_TARGET_PROPERTIES props =
                D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
                    dpiX,
                    dpiY
                    );

            hr = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(
                pBackBuffer,
                &props,
                &m_pBackBufferRT
                );
        }
        if (SUCCEEDED(hr))
        {
            // create brushes
            hr = m_pBackBufferRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &m_pTextBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pBackBufferRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Red),
                &m_pLoupeBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pLoupeTexture->QueryInterface(
                IID_PPV_ARGS(&pResolveSurface)
                );
        }
        if (SUCCEEDED(hr))
        {
            FLOAT dpiX;
            FLOAT dpiY;
            m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

            D2D1_BITMAP_PROPERTIES bp =
                D2D1::BitmapProperties(
                    D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
                    dpiX,
                    dpiY
                    );

            hr = m_pBackBufferRT->CreateSharedBitmap(
                __uuidof(pResolveSurface),
                pResolveSurface,
                &bp,
                &m_pLoupeBitmap
                );
        }
    }

    SafeRelease(&pBackBufferResource);
    SafeRelease(&pDXGIDevice);
    SafeRelease(&pAdapter);
    SafeRelease(&pDXGIFactory);
    SafeRelease(&pSurface);
    SafeRelease(&pDxgiSurface);
    SafeRelease(&pResolveSurface);
    SafeRelease(&pBackBuffer);

    return hr;
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::DiscardDeviceResources                          *
*                                                                 *
*  Certain resources (eg. device, swap chain, RT) are bound to a  *
*  particular D3D device. Under certain conditions (eg. change    *
*  display mode, remoting, removing a video adapter), it is       *
*  necessary to discard device-specific resources. This method    *
*  just releases all of the device-bound resources that we're     *
*  holding onto.                                                  *
*                                                                 *
******************************************************************/

void MSAASampleApp::DiscardDeviceResources()
{
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pState);

    SafeRelease(&m_pRenderTargetView);
    SafeRelease(&m_pBackBufferRT);
    SafeRelease(&m_pTextBrush);
    SafeRelease(&m_pLoupeTexture);
    SafeRelease(&m_pLoupeBitmap);
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::RunMessageLoop                                  *
*                                                                 *
*  This is the main message pump for the application              *
*                                                                 *
******************************************************************/

void MSAASampleApp::RunMessageLoop()
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
*  MSAASampleApp::OnRender                                        *
*                                                                 *
*  This method is called when the app needs to paint the window.  *
*  It uses a D2D RT to draw a gradient background into the swap   *
*  chain buffer. Then, it uses a separate D2D RT to draw a        *
*  2D scene into a D3D texture. This texture is mapped onto a     *
*  simple planar 3D model and displayed using D3D.                *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::OnRender()
{
    HRESULT hr;
    static float t = 0.0f;
    static DWORD dwTimeStart = 0;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && m_pBackBufferRT)
    {
        LARGE_INTEGER time;
        LARGE_INTEGER frequency;
        QueryPerformanceCounter(&time);
        QueryPerformanceFrequency(&frequency);

        float floatTime;

        if (!m_paused)
        {
            floatTime = static_cast<float>(time.QuadPart + m_timeDelta)/static_cast<float>(frequency.QuadPart);
        }
        else
        {
            floatTime = static_cast<float>(m_pausedTime + m_timeDelta)/static_cast<float>(frequency.QuadPart);
        }

        m_times.Add(time.QuadPart);

        // Swap chain will tell us how big the back buffer is
        DXGI_SWAP_CHAIN_DESC swapDesc;
        hr = m_pSwapChain->GetDesc(&swapDesc);

        if (SUCCEEDED(hr))
        {
            if (m_pBackBufferRT)
            {
                hr = RenderD2DContentIntoSurface(floatTime);
                if (SUCCEEDED(hr))
                {
                    if (m_drawLoupe)
                    {
                        hr = RenderLoupe();
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = RenderTextInfo();
                    }
                }

            }
            if (SUCCEEDED(hr))
            {
                hr = m_pSwapChain->Present(1, 0);
            }
        }
    }

    // If the device is lost for any reason, we need to recreate it
    if (hr == DXGI_ERROR_DEVICE_RESET ||
        hr == DXGI_ERROR_DEVICE_REMOVED ||
        hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;

        SafeRelease(&m_pDevice);
        DiscardDeviceResources();
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  MSAASampleApp::GetAntialiasModeString                          *
*                                                                 *
******************************************************************/

PCWSTR MSAASampleApp::GetAntialiasModeString()
{
    if (m_antialiasMode == MyAntialiasMode::Aliased)
    {
        return L"Aliased";
    }
    else if (m_antialiasMode == MyAntialiasMode::PerPrimitive)
    {
        return L"PerPrimitive";
    }
    else
    {
        Assert(m_antialiasMode == MyAntialiasMode::MSAA);

        return L"MSAA";
    }
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::RenderD2DContentIntoSurface                     *
*                                                                 *
*  This method renders a simple 2D scene into a D2D render target *
*  that maps to a D3D texture. It's important that the return     *
*  code from RT->EndDraw() is handed back to the caller, since    *
*  it's possible for the render target device to be lost while    *
*  the application is running, and the caller needs to handle     *
*  that error condition.                                          *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::RenderD2DContentIntoSurface(float time)
{
    HRESULT hr = S_OK;
    static DWORD dwTimeStart = 0;
    static DWORD dwTimeLast = 0;

    D2D1_SIZE_F rtSize = m_pBackBufferRT->GetSize();

    float squareWidth = sc_boardWidth / m_numSquares;

    m_pBackBufferRT->SetAntialiasMode(
        m_antialiasMode == MyAntialiasMode::PerPrimitive ?
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE :
            D2D1_ANTIALIAS_MODE_ALIASED
        );

    m_pBackBufferRT->BeginDraw();

    m_pBackBufferRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    m_pBackBufferRT->SetTransform(
        D2D1::Matrix3x2F::Translation(
            0.5f * (rtSize.width - squareWidth * m_numSquares),
            0.5f * (rtSize.height - squareWidth * m_numSquares)
            ) *
        D2D1::Matrix3x2F::Rotation(
            (sc_rotationSpeed * time * 360.0f) * ((float)D3DX_PI / 180.0f),
            D2D1::Point2F(rtSize.width / 2, rtSize.height / 2)
            )
        );

    ID2D1SolidColorBrush *pBrush = NULL;
    hr = m_pBackBufferRT->CreateSolidColorBrush(
            D2D1::ColorF(0, 0, 0),
            &pBrush
            );
    if (SUCCEEDED(hr))
    {
        if (m_sampleType == SampleType::Filled)
        {
            for (UINT i = 0; i < m_numSquares; ++i)
            {
                for (UINT j = 0; j < m_numSquares; ++j)
                {
                    D2D1_RECT_F rect =
                        D2D1::RectF(
                            i*squareWidth,
                            j*squareWidth,
                            (i+1)*squareWidth,
                            (j+1)*squareWidth
                            );

                    float dx = i+0.5f - 0.5f*m_numSquares;
                    float dy = j+0.5f - 0.5f*m_numSquares;

                    float length = sqrtf(2)*m_numSquares;

                    float intensity =
                        0.5f * (1+sinf( (0.2f * time + 10.0f * sqrtf(static_cast<float>(dx*dx+dy*dy))/length)));

                    pBrush->SetColor(
                        D2D1::ColorF(
                            0.0f,
                            intensity,
                            1.0f - intensity)
                        );

                    m_pBackBufferRT->FillRectangle(rect, pBrush);
                }
            }
        }
        else
        {
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::AntiqueWhite));

            for (UINT i = 0; i < m_numSquares+1; ++i)
            {
                m_pBackBufferRT->DrawLine(
                    D2D1::Point2F(i*squareWidth, 0),
                    D2D1::Point2F(i*squareWidth, m_numSquares*squareWidth),
                    pBrush,
                    1.0f
                    );

                m_pBackBufferRT->DrawLine(
                    D2D1::Point2F(0, i*squareWidth),
                    D2D1::Point2F(m_numSquares*squareWidth, i*squareWidth),
                    pBrush,
                    1.0f
                    );
            }

        }

        hr = m_pBackBufferRT->EndDraw();

        pBrush->Release();
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  MSAASampleApp::RenderLoupe                                     *
*                                                                 *
*  Draw the magnification Loupe                                   *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::RenderLoupe()
{
    HRESULT hr = S_OK;

    //
    // Read back the current contents of the swap chain buffer.
    //

    ID3D10Resource *pBackBufferResource = NULL;
    hr = m_pSwapChain->GetBuffer(
        0,
        IID_PPV_ARGS(&pBackBufferResource)
        );
    if (SUCCEEDED(hr))
    {
        m_pDevice->ResolveSubresource(
            m_pLoupeTexture,
            0, // DstSubresource
            pBackBufferResource,
            0, // SrcSubresource,
            DXGI_FORMAT_B8G8R8A8_UNORM
            );

        //
        // Render the magnified view.
        //

        D2D1_SIZE_F size = m_pBackBufferRT->GetSize();

        D2D1_POINT_2F destCenter =
            D2D1::Point2F(
                size.width - sc_loupeInset - 0.5f*sc_loupeSize,
                sc_loupeInset + 0.5f*sc_loupeSize
                );

        D2D1_ELLIPSE destEllipse =
            D2D1::Ellipse(
                destCenter,
                sc_loupeSize / 2,
                sc_loupeSize / 2
                );

        float loupeSrcRadius = sc_loupeSize / (2*GetZoom());

        D2D1_ELLIPSE srcEllipse =
            D2D1::Ellipse(
                m_mousePos,
                loupeSrcRadius,
                loupeSrcRadius
                );

        ID2D1BitmapBrush *pLoupeBrush = NULL;
        hr = m_pBackBufferRT->CreateBitmapBrush(
            m_pLoupeBitmap,
            D2D1::BitmapBrushProperties(
                D2D1_EXTEND_MODE_CLAMP,
                D2D1_EXTEND_MODE_CLAMP,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR),
            D2D1::BrushProperties(
                1.0f, // opacity
                D2D1::Matrix3x2F::Scale(
                    GetZoom(),
                    GetZoom(),
                    m_mousePos
                    ) *
                D2D1::Matrix3x2F::Translation(
                    destCenter.x - m_mousePos.x,
                    destCenter.y - m_mousePos.y
                    )),
            &pLoupeBrush
            );
        if (SUCCEEDED(hr))
        {
            m_pBackBufferRT->BeginDraw();

            m_pBackBufferRT->SetTransform(D2D1::Matrix3x2F::Identity());
            m_pBackBufferRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            m_pBackBufferRT->DrawEllipse(srcEllipse, m_pLoupeBrush, sc_srcLoupeRingThickness);
            m_pBackBufferRT->FillEllipse(destEllipse, pLoupeBrush);
            m_pBackBufferRT->DrawEllipse(destEllipse, m_pLoupeBrush, sc_destLoupeRingThickness);

            hr = m_pBackBufferRT->EndDraw();

            pLoupeBrush->Release();
        }

        pBackBufferResource->Release();
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  MSAASampleApp::RenderTextInfo                                  *
*                                                                 *
*  Draw the stats text (AA type, fps, etc...).                    *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::RenderTextInfo()
{
    HRESULT hr = S_OK;

    WCHAR textBuffer[400];
    LARGE_INTEGER frequency;
    float fps = 0.0f;
    float primsPerSecond = 0.0f;

    QueryPerformanceFrequency(&frequency);

    if (m_times.GetCount() > 0)
    {
        fps = (m_times.GetCount()-1) * frequency.QuadPart /
                static_cast<float>((m_times.GetLast() - m_times.GetFirst()));

        primsPerSecond = fps * m_numSquares * m_numSquares;
    }

    hr = StringCchPrintf(
        textBuffer,
        400,
        L"%s\n"
        L"# squares: %d x %d = %d\n"
        L"Fps: %.2f\n"
        L"Primitives / sec : %.0f\n",
        GetAntialiasModeString(),
        m_numSquares,
        m_numSquares,
        m_numSquares*m_numSquares,
        fps,
        primsPerSecond
        );
    if (SUCCEEDED(hr))
    {
        m_pBackBufferRT->BeginDraw();

        m_pBackBufferRT->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pBackBufferRT->DrawText(
            textBuffer,
            static_cast<UINT>(wcsnlen(textBuffer, ARRAYSIZE(textBuffer))),
            m_pTextFormat,
            D2D1::RectF(10.0f, 10.0f, 1000.0f, 1000.0f),
            m_pTextBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE
            );

        hr = m_pBackBufferRT->EndDraw();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::OnKeyDown                                       *
*                                                                 *
******************************************************************/

void MSAASampleApp::OnKeyDown(SHORT vkey)
{
    switch (vkey)
    {
    case 'A':
    case VK_RIGHT:
        m_antialiasMode =
            static_cast<MyAntialiasMode::Enum>(
                (m_antialiasMode+1)%MyAntialiasMode::Count
                );

        // This sample could be smarter about only discarding resources it doesn't
        // need in order to make transitions from one state to another more quickly.
        DiscardDeviceResources();
        break;

    case VK_LEFT:
        m_antialiasMode =
            static_cast<MyAntialiasMode::Enum>(
                (m_antialiasMode+MyAntialiasMode::Count-1)%MyAntialiasMode::Count
                );

        // This sample could be smarter about only discarding resources it doesn't
        // need in order to make transitions from one state to another more quickly.
        DiscardDeviceResources();
        break;

    case VK_SPACE:

        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);

        if (!m_paused)
        {
            m_pausedTime = time.QuadPart;
        }
        else
        {
            m_timeDelta += (m_pausedTime - time.QuadPart);
        }

        m_paused = !m_paused;
        break;

    case VK_UP:
        m_numSquares = min(m_numSquares*2, sc_maxNumSquares);
        break;

    case VK_DOWN:
        m_numSquares = max(m_numSquares/2, sc_minNumSquares);
        break;

    case 'W':
        m_sampleType = static_cast<SampleType::Enum>((m_sampleType+1) % SampleType::Count);
        break;

    case 'L':
        m_drawLoupe = !m_drawLoupe;
        break;

    default:
        break;
    }
}

/******************************************************************
*                                                                 *
*  OnMouseMove                                                    *
*                                                                 *
******************************************************************/

void MSAASampleApp::OnMouseMove(LPARAM lParam)
{
    // Calculate the mouse position in DIPs.
    FLOAT dpiX;
    FLOAT dpiY;
    m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

    m_mousePos = D2D1::Point2F(LOWORD(lParam) * 96.0f / dpiX, HIWORD(lParam) * 96.0f / dpiY);
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::OnWheel                                         *
*                                                                 *
******************************************************************/

void MSAASampleApp::OnWheel(WPARAM wParam)
{
    m_logZoom += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam));

    m_logZoom = min(
        max(m_logZoom, sc_loupeMinLogZoom*WHEEL_DELTA),
        sc_loupeMaxLogZoom*WHEEL_DELTA
        );
}

/******************************************************************
*                                                                 *
*  MSAASampleApp::WndProc                                         *
*                                                                 *
*  This static method handles our app's window messages           *
*                                                                 *
******************************************************************/

LRESULT CALLBACK MSAASampleApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        MSAASampleApp *pMSAASampleApp = (MSAASampleApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pMSAASampleApp)
            );

        result = 1;
    }
    else
    {
        MSAASampleApp *pMSAASampleApp = reinterpret_cast<MSAASampleApp *>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                ));

        bool wasHandled = false;

        if (pMSAASampleApp)
        {
            switch (message)
            {
            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);
                    pMSAASampleApp->OnRender();
                    EndPaint(hwnd, &ps);

                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_KEYDOWN:
                {
                    pMSAASampleApp->OnKeyDown(static_cast<SHORT>(wParam));
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEMOVE:
                {
                    pMSAASampleApp->OnMouseMove(lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEWHEEL:
                {
                    pMSAASampleApp->OnWheel(wParam);
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

/******************************************************************
*                                                                 *
*  MSAASampleApp::CreateD3DDevice                                 *
*                                                                 *
******************************************************************/

HRESULT MSAASampleApp::CreateD3DDevice(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE driverType,
    UINT flags,
    ID3D10Device1 **ppDevice
    )
{
    HRESULT hr = S_OK;

    static const D3D10_FEATURE_LEVEL1 levelAttempts[] =
    {
        D3D10_FEATURE_LEVEL_10_0,
        D3D10_FEATURE_LEVEL_9_3,
        D3D10_FEATURE_LEVEL_9_2,
        D3D10_FEATURE_LEVEL_9_1,
    };

    for (UINT level = 0; level < ARRAYSIZE(levelAttempts); level++)
    {
        ID3D10Device1 *pDevice = NULL;
        hr = D3D10CreateDevice1(
            pAdapter,
            driverType,
            NULL,
            flags,
            levelAttempts[level],
            D3D10_1_SDK_VERSION,
            &pDevice
            );

        if (SUCCEEDED(hr))
        {
            // transfer reference
            *ppDevice = pDevice;
            pDevice = NULL;
            break;
        }
    }

    return hr;
}
