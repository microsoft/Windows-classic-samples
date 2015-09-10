//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "D2DPrintingFromDesktopApps.h"

#include <commdlg.h>
#include <wchar.h>
#include <math.h>
#include <Prntvpt.h>
#include <Strsafe.h>

static const FLOAT PAGE_WIDTH_IN_DIPS    = 8.5f * 96.0f;     // 8.5 inches
static const FLOAT PAGE_HEIGHT_IN_DIPS   = 11.0f * 96.0f;    // 11 inches
static const FLOAT PAGE_MARGIN_IN_DIPS   = 96.0f;            // 1 inch
static const FLOAT FRAME_HEIGHT_IN_DIPS  = 400.0f;           // 400 DIPs
static const FLOAT HOURGLASS_SIZE        = 200.0f;           // 200 DIPs

// Provides the main entry point to the application.
int WINAPI WinMain(
    _In_        HINSTANCE /* hInstance */,
    _In_opt_    HINSTANCE /* hPrevInstance */,
    _In_        LPSTR /* lpCmdLine */,
    _In_        int /* nCmdShow */
    )
{
    // Ignore the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    BOOL succeded = HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    if (!succeded)
    {
        // Enable Heap Termination upon corruption failed.
        // For this sample, we ignore when this fails and continue running.
    }

    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();

        return 0;
    }
    else
    {
        return -1;
    }
}

// The main window message loop.
void DemoApp::RunMessageLoop()
{
    MSG message;

    while (GetMessage(&message, nullptr, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

// Initializes members.
DemoApp::DemoApp() :
    m_resourcesValid(false),
    m_parentHwnd(nullptr),
    m_d2dHwnd(nullptr),
    m_multiPageMode(TRUE),
    m_currentScrollPosition(0),
    m_d2dFactory(nullptr),
    m_wicFactory(nullptr),
    m_dwriteFactory(nullptr),
    m_swapChain(nullptr),
    m_d2dDevice(nullptr),
    m_d2dContext(nullptr),
    m_textFormat(nullptr),
    m_smallTextFormat(nullptr),
    m_pathGeometry(nullptr),
    m_linearGradientBrush(nullptr),
    m_blackBrush(nullptr),
    m_gridPatternBrush(nullptr),
    m_customBitmap(nullptr),
    m_jobPrintTicketStream(nullptr),
    m_printControl(nullptr),
    m_documentTarget(nullptr),
    m_printJobChecker(nullptr),
    m_pageHeight(PAGE_HEIGHT_IN_DIPS),
    m_pageWidth(PAGE_WIDTH_IN_DIPS)
{
}

// Releases resources.
DemoApp::~DemoApp()
{
    // Release device-independent resources.
    SafeRelease(&m_textFormat);
    SafeRelease(&m_smallTextFormat);
    SafeRelease(&m_pathGeometry);

    // Release device-dependent resources.
    SafeRelease(&m_linearGradientBrush);
    SafeRelease(&m_blackBrush);
    SafeRelease(&m_gridPatternBrush);
    SafeRelease(&m_customBitmap);
    SafeRelease(&m_swapChain);
    SafeRelease(&m_d2dDevice);
    SafeRelease(&m_d2dContext);

    // Release printing-specific resources.
    SafeRelease(&m_jobPrintTicketStream);
    SafeRelease(&m_printControl);
    SafeRelease(&m_documentTarget);
    SafeRelease(&m_printJobChecker);

    // Release factories.
    SafeRelease(&m_d2dFactory);
    SafeRelease(&m_wicFactory);
    SafeRelease(&m_dwriteFactory);
}

// Creates the application window and initializes
// device-independent and device-dependent resources.
HRESULT DemoApp::Initialize()
{
    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    HRESULT hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        // Register the parent window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::ParentWndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName  = nullptr;
        wcex.lpszClassName = L"DemoAppWindow";

        RegisterClassEx(&wcex);

        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;
        m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

        // Create the parent window.
        m_parentHwnd = CreateWindow(
            L"DemoAppWindow",
            L"Direct2D desktop app printing sample - Press 'p' to print, press <Tab> to toggle multi-page mode",
            WS_OVERLAPPEDWINDOW | WS_VSCROLL,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            nullptr,
            nullptr,
            HINST_THISCOMPONENT,
            this
            );

        hr = m_parentHwnd ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            ShowWindow(m_parentHwnd, SW_SHOWNORMAL);
            UpdateWindow(m_parentHwnd);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Register the child window class (for D2D content).
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::ChildWndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName  = nullptr;
        wcex.lpszClassName = L"D2DDemoApp";

        RegisterClassEx(&wcex);

        D2D1_SIZE_U d2dWindowSize = CalculateD2DWindowSize();

        // Create the child window.
        m_d2dHwnd = CreateWindow(
            L"D2DDemoApp",
            L"",
            WS_CHILDWINDOW | WS_VISIBLE,
            0,
            0,
            d2dWindowSize.width,
            d2dWindowSize.height,
            m_parentHwnd,
            nullptr,
            HINST_THISCOMPONENT,
            this
            );

        hr = m_d2dHwnd ? S_OK : E_FAIL;
    }

    // Create D2D device context and device-dependent resources.
    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceResources();
    }

    if (FAILED(hr))
    {
        MessageBox(
            m_d2dHwnd,
            L"Failed to initialize the application. Sample will exit.",
            L"Sample initialization error",
            MB_OK | MB_ICONSTOP
            );
    }

    return hr;
}

// Calculates the size of the D2D (child) window.
D2D1_SIZE_U DemoApp::CalculateD2DWindowSize()
{
    RECT rc;
    GetClientRect(m_parentHwnd, &rc);

    D2D1_SIZE_U d2dWindowSize = {0};
    d2dWindowSize.width = rc.right;
    d2dWindowSize.height = rc.bottom;

    return d2dWindowSize;
}

// Creates resources which are not bound to any device.
// Their lifetimes effectively extend for the duration
// of the app.
HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    ID2D1GeometrySink* sink = nullptr;

    if (SUCCEEDED(hr))
    {
        // Create a Direct2D factory.
        D2D1_FACTORY_OPTIONS options;
        ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
        // If the project is in a debug build, enable Direct2D debugging via SDK Layers
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            options,
            &m_d2dFactory
            );
    }
    if (SUCCEEDED(hr))
    {
        // Create a WIC factory.
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_wicFactory)
            );
    }
    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_dwriteFactory)
            );
    }
    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite text format object.
        hr = m_dwriteFactory->CreateTextFormat(
            L"Verdana",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            50.0f,
            L"en-us",
            &m_textFormat
            );
    }
    if (SUCCEEDED(hr))
    {
        // Center the text horizontally.
        hr = m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    if (SUCCEEDED(hr))
    {
        // Center the text vertically.
        hr = m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    if (SUCCEEDED(hr))
    {
        // Create a second DirectWrite text format for the multi-page text
        hr = m_dwriteFactory->CreateTextFormat(
            L"Verdana",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            12.0f,
            L"en-us",
            &m_smallTextFormat
            );
    }

    // Create a path geometry for an hourglass shape.
    if (SUCCEEDED(hr))
    {
        // Create a path geometry.
        hr = m_d2dFactory->CreatePathGeometry(&m_pathGeometry);
    }
    if (SUCCEEDED(hr))
    {
        // Use the geometry sink to write to the path geometry.
        hr = m_pathGeometry->Open(&sink);
    }
    if (SUCCEEDED(hr))
    {
        // Write an hourglass shape to the geometry.
        sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

        sink->BeginFigure(
            D2D1::Point2F(0.0f, 0.0f),
            D2D1_FIGURE_BEGIN_FILLED
            );

        sink->AddLine(D2D1::Point2F(HOURGLASS_SIZE, 0.0f));

        sink->AddBezier(
            D2D1::BezierSegment(
            D2D1::Point2F(HOURGLASS_SIZE / 4.0f * 3.0f, HOURGLASS_SIZE / 4.0f),
            D2D1::Point2F(HOURGLASS_SIZE / 4.0f * 3.0f, HOURGLASS_SIZE / 4.0f * 3.0f),
            D2D1::Point2F(HOURGLASS_SIZE, HOURGLASS_SIZE))
            );

        sink->AddLine(D2D1::Point2F(0.0f, HOURGLASS_SIZE));

        sink->AddBezier(
            D2D1::BezierSegment(
            D2D1::Point2F(HOURGLASS_SIZE / 4.0f, HOURGLASS_SIZE / 4.0f * 3.0f),
            D2D1::Point2F(HOURGLASS_SIZE / 4.0f, HOURGLASS_SIZE / 4.0f),
            D2D1::Point2F(0.0f, 0.0f))
            );

        sink->EndFigure(D2D1_FIGURE_END_CLOSED);

        hr = sink->Close();
    }
    SafeRelease(&sink);

    return hr;
}

// Create D2D context for display (Direct3D) device
HRESULT DemoApp::CreateDeviceContext()
{
    HRESULT hr = S_OK;

    // Get the size of the child window.
    D2D1_SIZE_U size = CalculateD2DWindowSize();

    // Create a D3D device and a swap chain sized to the child window.
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };
    UINT countOfDriverTypes = ARRAYSIZE(driverTypes);

    DXGI_SWAP_CHAIN_DESC swapDescription;
    ZeroMemory(&swapDescription, sizeof(swapDescription));
    swapDescription.BufferDesc.Width = size.width;
    swapDescription.BufferDesc.Height = size.height;
    swapDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapDescription.BufferDesc.RefreshRate.Numerator = 60;
    swapDescription.BufferDesc.RefreshRate.Denominator = 1;
    swapDescription.SampleDesc.Count = 1;
    swapDescription.SampleDesc.Quality = 0;
    swapDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDescription.BufferCount = 1;
    swapDescription.OutputWindow = m_d2dHwnd;
    swapDescription.Windowed = TRUE;

    ID3D11Device* d3dDevice = nullptr;
    for (UINT driverTypeIndex = 0; driverTypeIndex < countOfDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,       // use default adapter
            driverTypes[driverTypeIndex],
            nullptr,       // no external software rasterizer
            createDeviceFlags,
            nullptr,       // use default set of feature levels
            0,
            D3D11_SDK_VERSION,
            &swapDescription,
            &m_swapChain,
            &d3dDevice,
            nullptr,       // do not care about what feature level is chosen
            nullptr        // do not retain D3D device context
            );

        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    IDXGIDevice* dxgiDevice = nullptr;
    if (SUCCEEDED(hr))
    {
        // Get a DXGI device interface from the D3D device.
        hr = d3dDevice->QueryInterface(&dxgiDevice);
    }
    if (SUCCEEDED(hr))
    {
        // Create a D2D device from the DXGI device.
        hr = m_d2dFactory->CreateDevice(
            dxgiDevice,
            &m_d2dDevice
            );
    }
    if (SUCCEEDED(hr))
    {
        // Create a device context from the D2D device.
        hr = m_d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_d2dContext
            );
    }

    SafeRelease(&dxgiDevice);
    SafeRelease(&d3dDevice);
    return hr;
}

// This method creates resources which are bound to a particular
// Direct3D device. It's all centralized here, in case the resources
// need to be recreated in case of Direct3D device loss (e.g. display
// change, remoting, removal of video card, etc). The resources created
// here can be used by multiple Direct2D device contexts (in this
// sample, one for display and another for print) which are created
// from the same Direct2D device.
HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_resourcesValid)
    {
        hr = CreateDeviceContext();

        IDXGISurface* surface = nullptr;
        if (SUCCEEDED(hr))
        {
            // Get a surface from the swap chain.
            hr = m_swapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&surface)
                );
        }
        ID2D1Bitmap1* bitmap = nullptr;
        if (SUCCEEDED(hr))
        {
            FLOAT dpiX, dpiY;
            m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

            // Create a bitmap pointing to the surface.
            D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE
                    ),
                dpiX,
                dpiY
                );

            hr = m_d2dContext->CreateBitmapFromDxgiSurface(
                surface,
                &properties,
                &bitmap
                );
        }
        if (SUCCEEDED(hr))
        {
            // Set the bitmap as the target of our device context.
            m_d2dContext->SetTarget(bitmap);
        }
        if (SUCCEEDED(hr))
        {
            // Create a black brush.
            hr = m_d2dContext->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_blackBrush
                );
        }
        // Create a linear gradient for the hourglasses.
        ID2D1GradientStopCollection* gradientStops = nullptr;
        if (SUCCEEDED(hr))
        {
            static const D2D1_GRADIENT_STOP stops[] =
            {
                { 0.f, { 0.f, 1.f, 1.f, 0.25f } },
                { 1.f, { 0.f, 0.f, 1.f, 1.f } },
            };

            hr = m_d2dContext->CreateGradientStopCollection(
                stops,
                ARRAYSIZE(stops),
                &gradientStops
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = m_d2dContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(HOURGLASS_SIZE/2, 0),
                D2D1::Point2F(HOURGLASS_SIZE/2, HOURGLASS_SIZE)),
                D2D1::BrushProperties(),
                gradientStops,
                &m_linearGradientBrush
                );
        }
        SafeRelease(&gradientStops);
        if (SUCCEEDED(hr))
        {
            // Create a bitmap by loading it from a file.
            hr = LoadBitmapFromFile(
                m_d2dContext,
                m_wicFactory,
                L".\\sampleImage.jpg",
                100,
                0,
                &m_customBitmap
                );
        }
        if (SUCCEEDED(hr))
        {
            // Create a brush to use for the gridded background.
            hr = CreateGridPatternBrush(&m_gridPatternBrush);
        }
        if (SUCCEEDED(hr))
        {
            ResetScrollBar();
        }

        SafeRelease(&bitmap);
        SafeRelease(&surface);
    }

    if (FAILED(hr))
    {
        DiscardDeviceResources();
    }
    else
    {
        m_resourcesValid = true;
    }

    return hr;
}

// Creates a bitmap-backed pattern brush to be used for the grid
// background in the scene.
HRESULT DemoApp::CreateGridPatternBrush(
    _Outptr_ ID2D1ImageBrush** imageBrush
    )
{
    HRESULT hr = S_OK;

    ID2D1CommandList* gridCommandList = nullptr;
    if (SUCCEEDED(hr))
    {
        // Create a command list that will store the grid pattern.
        hr = m_d2dContext->CreateCommandList(&gridCommandList);
    }
    ID2D1SolidColorBrush* gridBrush = nullptr;
    if (SUCCEEDED(hr))
    {
        // Create a brush with which to draw the grid pattern.
        hr = m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF(0.93f, 0.94f, 0.96f, 1.0f)),
            &gridBrush
            );
    }
    ID2D1Image* originalTarget = nullptr;
    if (SUCCEEDED(hr))
    {
        // Save the context's current target.
        m_d2dContext->GetTarget(&originalTarget);

        // Put the new target in place.
        m_d2dContext->SetTarget(gridCommandList);

        // Draw the grid pattern on the new target.
        m_d2dContext->BeginDraw();
        m_d2dContext->FillRectangle(D2D1::RectF(0.0f, 0.0f, 10.0f, 1.0f), gridBrush);
        m_d2dContext->FillRectangle(D2D1::RectF(0.0f, 0.1f, 1.0f, 10.0f), gridBrush);
        hr = m_d2dContext->EndDraw();

        // Restore the original target.
        m_d2dContext->SetTarget(originalTarget);
    }
    if (SUCCEEDED(hr))
    {
        hr = gridCommandList->Close();
    }
    if (SUCCEEDED(hr))
    {
        // Create image brush with the grid command list.
        hr = m_d2dContext->CreateImageBrush(
            gridCommandList,
            D2D1::ImageBrushProperties(
                D2D1::RectF(0, 0, 10, 10),
                D2D1_EXTEND_MODE_WRAP,
                D2D1_EXTEND_MODE_WRAP
                ),
            imageBrush
            );
    }

    SafeRelease(&gridBrush);
    SafeRelease(&originalTarget);
    SafeRelease(&gridCommandList);

    return hr;
}

// Discards device-specific resources which need to be recreated
// when a Direct3D device is lost.
void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_swapChain);
    SafeRelease(&m_d2dDevice);
    SafeRelease(&m_d2dContext);

    SafeRelease(&m_customBitmap);
    SafeRelease(&m_blackBrush);
    SafeRelease(&m_linearGradientBrush);
    SafeRelease(&m_gridPatternBrush);

    m_resourcesValid = false;
}

// Draws the scene to a rendering device context or a printing device context.
// If the "printing" parameter is set, this function will add margins to
// the target and render the text across two pages. Otherwise, it fits the
// content to the target and renders the text in one block.
HRESULT DemoApp::DrawToContext(
    _In_ ID2D1DeviceContext* d2dContext,
    UINT pageNumber,
    BOOL printing
    )
{
    HRESULT hr = S_OK;

    // Get the size of the displayed window.
    D2D1_SIZE_U windowSize = CalculateD2DWindowSize();

    // Compute the margin sizes.
    FLOAT margin = PAGE_MARGIN_IN_DIPS;

    // Get the size of the destination context ("page").
    D2D1_SIZE_F targetSize = {0};
    if (printing)
    {
        targetSize.width = m_pageWidth - 2 * margin;
        targetSize.height = m_pageHeight - 2 * margin;
    }
    else
    {
        targetSize.width = static_cast<FLOAT>(windowSize.width);  // assuming both in DIPs
        targetSize.height = static_cast<FLOAT>(windowSize.height);
    }

    // Compute the size of the gridded background rectangle.
    D2D1_SIZE_F frameSize = D2D1::SizeF(
        targetSize.width,
        FRAME_HEIGHT_IN_DIPS
        );

    // Compute the translation matrix that simulates scrolling or printing.
    D2D1_MATRIX_3X2_F scrollTransform = printing ?
        D2D1::Matrix3x2F::Translation(margin, margin) :
        D2D1::Matrix3x2F::Translation(0.0f, static_cast<FLOAT>(-m_currentScrollPosition));

    d2dContext->BeginDraw();

    d2dContext->SetTransform(scrollTransform);

    d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White));

    if (!printing || pageNumber == 1)
    {
        // Display geometries and text on screen. In printing case, display only on page 1.

        // Paint a grid background
        d2dContext->FillRectangle(
            D2D1::RectF(0.0f, 0.0f, frameSize.width, frameSize.height),
            m_gridPatternBrush
            );

        // Draw a bitmap in the upper-left corner of the window.
        D2D1_SIZE_F bitmapSize = m_customBitmap->GetSize();
        d2dContext->DrawBitmap(
            m_customBitmap,
            D2D1::RectF(0.0f, 0.0f, bitmapSize.width, bitmapSize.height)
            );

        // Draw a bitmap at the lower-right corner of the window.
        d2dContext->DrawBitmap(
            m_customBitmap,
            D2D1::RectF(
                frameSize.width - bitmapSize.width,
                frameSize.height - bitmapSize.height,
                frameSize.width,
                frameSize.height
                )
            );

        // Draw the rotated "Hello world" message.
        D2D1_MATRIX_3X2_F sidewaysTransform = D2D1::Matrix3x2F::Rotation(
            45,
            D2D1::Point2F(
                frameSize.width / 2,
                frameSize.height / 2
                )
            );

        d2dContext->SetTransform(sidewaysTransform * scrollTransform);

        static const WCHAR helloString[] = L"Hello, World!";

        d2dContext->DrawText(
            helloString,
            ARRAYSIZE(helloString) - 1,
            m_textFormat,
            D2D1::RectF(
                0.0f,
                0.0f,
                frameSize.width,
                frameSize.height
                ),
            m_blackBrush
            );

        // Draw the hour glass in the bottom-left.
        D2D1_MATRIX_3X2_F hourglassTransform = D2D1::Matrix3x2F::Translation(0.0f, frameSize.height - HOURGLASS_SIZE);

        d2dContext->SetTransform(hourglassTransform * scrollTransform);

        d2dContext->FillGeometry(
            m_pathGeometry,
            m_linearGradientBrush
            );

        // Draw the hour glass in the upper-right.
        hourglassTransform = D2D1::Matrix3x2F::Translation(frameSize.width - HOURGLASS_SIZE, 0.0f);

        d2dContext->SetTransform(hourglassTransform * scrollTransform);

        d2dContext->FillGeometry(
            m_pathGeometry,
            m_linearGradientBrush
            );
    }

    if (m_multiPageMode)
    {
        static const WCHAR longString[] = L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus tincidunt elementum diam. Sed semper ligula nec orci egestas sit amet tincidunt risus vulputate. Aliquam tempus quam nec neque facilisis vestibulum tristique est condimentum. Vestibulum ultrices tortor quis sapien dapibus non laoreet nunc posuere. Nullam semper, mi et consectetur mollis, dui ante vestibulum quam, quis fringilla diam purus nec urna. Nullam justo odio, posuere sed fringilla rutrum, ornare ac metus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum bibendum rutrum pharetra. Suspendisse eros est, porta ut adipiscing sed, rutrum quis augue. Ut lacinia, mi sit amet pretium dapibus, velit est scelerisque odio, ac viverra augue enim at eros. Donec consectetur, purus a ullamcorper venenatis, risus libero placerat ligula, et rutrum est justo at orci. Fusce fringilla tristique iaculis. Nulla facilisi. Maecenas et libero augue, sit amet luctus nisl.\r\n\r\nPellentesque ac est ac est mattis venenatis vitae sit amet ligula. Proin pharetra erat vel urna sagittis sed tincidunt est feugiat. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Sed vitae eleifend massa. In ac orci turpis. Nam in erat nunc, vel rhoncus mauris. Ut fermentum, urna ac commodo tincidunt, orci enim consequat sem, ut fringilla enim quam non diam. Proin et ipsum nisi, sit amet ultricies nulla. Suspendisse eu dolor metus, ac consectetur ligula. Maecenas at est ligula.\r\n\r\nMauris sagittis, ligula sit amet vestibulum facilisis, lacus lorem porta neque, nec tristique leo mi vestibulum erat. Cras luctus sollicitudin nulla sed egestas. Aenean pellentesque suscipit sapien, vitae mollis sapien vehicula et. Proin feugiat leo et nulla porttitor facilisis. Nullam a lacus lorem. Praesent vitae mi dui, sit amet placerat mauris. Maecenas iaculis lacinia tellus sit amet egestas. Nam a metus orci. Cras non neque eget massa hendrerit accumsan a a tortor. Phasellus lacinia enim quam. Phasellus tristique suscipit nibh, sed molestie orci eleifend sed. Praesent lobortis tortor non mi ultrices euismod ut ac lectus. Quisque laoreet facilisis diam quis aliquam. Phasellus at quam non felis pretium mollis. Fusce vel posuere ipsum. Sed gravida tortor justo, ac volutpat velit.\r\n\r\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Cras vitae convallis ipsum. Aliquam mauris dui, imperdiet eget porttitor ac, bibendum ac sapien. Praesent ut dignissim sem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla id sapien facilisis nisi tristique venenatis. Donec vitae mauris sem. Sed condimentum erat tortor, sit amet commodo nibh. Nam euismod, magna et cursus auctor, enim dui condimentum felis, at aliquet ligula tellus ac est. Aenean viverra, urna at adipiscing dignissim, diam dui blandit sem, varius mollis enim ante sit amet ante. Curabitur lobortis condimentum odio non ultricies. Maecenas lorem elit, laoreet ut semper vel, egestas vel ipsum. Etiam adipiscing facilisis libero, sit amet placerat nisi aliquam id. Vestibulum fermentum quam a nisl semper gravida. Aliquam vestibulum interdum turpis. Pellentesque nunc nulla, aliquam vel consequat eu, aliquam vitae justo. Praesent iaculis, urna at ullamcorper placerat, neque lacus accumsan velit, eu mollis lacus leo nec nisl. Nulla massa risus, tincidunt vitae rhoncus et, feugiat a turpis. Phasellus orci quam, ultricies eu sollicitudin vel, vulputate eu lectus. Etiam quis velit ipsum, in rhoncus justo.";
        static const UINT totalChars = ARRAYSIZE(longString) - 1;
        static const UINT numberOfCharsFirstPage = 955;

        d2dContext->SetTransform(scrollTransform);

        if (printing)
        {
            // Lay out the text in parts, some on the first page, after
            // the gridded rectangle, then some on the second page.
            // A real application would contain much more robust layout logic.
            // Here we simply render the first 955 characters on the first page and
            // the remaining characters on the second page.

            if (pageNumber == 1)
            {
                d2dContext->DrawText(
                    longString,
                    numberOfCharsFirstPage,
                    m_smallTextFormat,
                    D2D1::RectF(
                        0.0f,
                        frameSize.height + 5.0f,
                        frameSize.width,
                        targetSize.height
                        ),
                    m_blackBrush
                    );
            }
            else if (pageNumber == 2)
            {
                d2dContext->DrawText(
                    longString + numberOfCharsFirstPage,
                    totalChars - numberOfCharsFirstPage,
                    m_smallTextFormat,
                    D2D1::RectF(
                        0.0f,
                        0.0f,
                        frameSize.width,
                        targetSize.height
                        ),
                    m_blackBrush
                    );
            }
        }
        else
        {
            // Lay out the text in a single, long rectangle.
            d2dContext->DrawText(
                longString,
                totalChars,
                m_smallTextFormat,
                D2D1::RectF(
                    0.0f,
                    frameSize.height + 5.0f,
                    targetSize.width,
                    targetSize.height
                    ),
                m_blackBrush
                );
        }
    }

    hr = d2dContext->EndDraw();

    return hr;
}


// Called whenever the application needs to display the client
// window. Draws a 2D scene onto the rendering device context.
// Note that this function will automatically discard device-specific
// resources if the Direct3D device disappears during function
// invocation and will recreate the resources the next time it's
// invoked.
HRESULT DemoApp::OnRender()
{
    HRESULT hr = S_OK;

    if (!m_resourcesValid)
    {
        hr = CreateDeviceResources();
    }

    if (SUCCEEDED(hr))
    {
        // Render the scene on the device context tied to
        // the swap chain (i.e. to the screen).
        hr = DrawToContext(m_d2dContext, 1, FALSE); // FALSE specifies drawing to screen.
    }

    if (SUCCEEDED(hr))
    {
        // Present the swap chain immediately
        hr = m_swapChain->Present(0, 0);
    }

    if (hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        hr = S_OK;
        // Recreate device resources then force repaint.
        DiscardDeviceResources();
        InvalidateRect(m_d2dHwnd, nullptr, FALSE);
    }

    return hr;
}

// Called whenever the application receives a keystroke (a
// WM_CHAR message). Starts printing or toggles multi-page
// mode, depending on the key pressed.
void DemoApp::OnChar(SHORT key)
{
    if ((key == 'p') || (key == 'P'))
    {
        OnPrint();
    }
    else if (key == '\t')
    {
        ToggleMultiPageMode();
    }
}

// Switches the application between single-page and multi-page mode.
void DemoApp::ToggleMultiPageMode()
{
    m_multiPageMode = !m_multiPageMode;
    InvalidateRect(m_d2dHwnd, nullptr, FALSE);
}

// Called whenever the application begins a print job. Initializes
// the printing subsystem, draws the scene to a printing device
// context, and commits the job to the printing subsystem.
HRESULT DemoApp::OnPrint()
{
    HRESULT hr = S_OK;

    if (!m_resourcesValid)
    {
        hr = CreateDeviceResources();
    }

    if (SUCCEEDED(hr))
    {
        // Initialize printing-specific resources and prepare the
        // printing subsystem for a job.
        hr = InitializePrintJob();
    }

    ID2D1DeviceContext* d2dContextForPrint = nullptr;
    if (SUCCEEDED(hr))
    {
        // Create a D2D Device Context dedicated for the print job.
        hr = m_d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &d2dContextForPrint
            );
    }

    if (SUCCEEDED(hr))
    {
        ID2D1CommandList* commandList = nullptr;

        for (INT pageIndex = 1; pageIndex <= (m_multiPageMode ? 2 : 1); pageIndex++)
        {
            hr = d2dContextForPrint->CreateCommandList(&commandList);

            // Create, draw, and add a Direct2D Command List for each page.
            if (SUCCEEDED(hr))
            {
                d2dContextForPrint->SetTarget(commandList);
                hr = DrawToContext(d2dContextForPrint, pageIndex, TRUE);  // TRUE specifies rendering for printing
                commandList->Close();
            }

            if (SUCCEEDED(hr))
            {
                hr = m_printControl->AddPage(commandList, D2D1::SizeF(m_pageWidth, m_pageHeight), nullptr);
            }

            SafeRelease(&commandList);
        }

        // Release the print device context.
        SafeRelease(&d2dContextForPrint);

        // Send the job to the printing subsystem and discard
        // printing-specific resources.
        HRESULT hrFinal = FinalizePrintJob();

        if (SUCCEEDED(hr))
        {
            hr = hrFinal;
        }
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }

    return hr;
}

// Brings up a Print Dialog to collect user print
// settings, then creates and initializes a print
// control object properly for a new print job.
HRESULT DemoApp::InitializePrintJob()
{
    HRESULT hr = S_OK;
    WCHAR messageBuffer[512] = {0};

    // Bring up Print Dialog and receive user print settings.
    PRINTDLGEX printDialogEx = {0};
    printDialogEx.lStructSize = sizeof(PRINTDLGEX);
    printDialogEx.Flags = PD_HIDEPRINTTOFILE | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_USEDEVMODECOPIESANDCOLLATE;
    printDialogEx.hwndOwner = m_parentHwnd;
    printDialogEx.nStartPage = START_PAGE_GENERAL;

    HRESULT hrPrintDlgEx = PrintDlgEx(&printDialogEx);

    if (FAILED(hrPrintDlgEx))
    {
        // Error occured.
        StringCchPrintf(
            messageBuffer,
            ARRAYSIZE(messageBuffer),
            L"Error 0x%4X occured during printer selection and/or setup.",
            hrPrintDlgEx
            );
        MessageBox(m_parentHwnd, messageBuffer, L"Message", MB_OK);
        hr = hrPrintDlgEx;
    }
    else if (printDialogEx.dwResultAction == PD_RESULT_APPLY)
    {
        // User clicks the Apply button and later clicks the Cancel button.
        // For simpicity, this sample skips print settings recording.
        hr = E_FAIL;
    }
    else if (printDialogEx.dwResultAction == PD_RESULT_CANCEL)
    {
        // User clicks the Cancel button.
        hr = E_FAIL;
    }

    // Retrieve DEVNAMES from print dialog.
    DEVNAMES* devNames = nullptr;
    if (SUCCEEDED(hr))
    {
        if (printDialogEx.hDevNames != nullptr)
        {
            devNames = reinterpret_cast<DEVNAMES*>(GlobalLock(printDialogEx.hDevNames));
            if (devNames == nullptr)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = E_HANDLE;
        }
    }

    // Retrieve user settings from print dialog.
    DEVMODE* devMode = nullptr;
    PCWSTR printerName = nullptr;
    if (SUCCEEDED(hr))
    {
        printerName = reinterpret_cast<PCWSTR>(devNames) + devNames->wDeviceOffset;

        if (printDialogEx.hDevMode != nullptr)
        {
            devMode = reinterpret_cast<DEVMODE*>(GlobalLock(printDialogEx.hDevMode));   // retrieve DevMode

            if (devMode)
            {
                // Must check corresponding flags in devMode->dmFields
                if ((devMode->dmFields & DM_PAPERLENGTH) && (devMode->dmFields & DM_PAPERWIDTH))
                {
                    // Convert 1/10 of a millimeter DEVMODE unit to 1/96 of inch D2D unit
                    m_pageHeight = devMode->dmPaperLength / 254.0f * 96.0f;
                    m_pageWidth  = devMode->dmPaperWidth / 254.0f * 96.0f;
                }
                else
                {
                    // Use default values if the user does not specify page size.
                    m_pageHeight = PAGE_HEIGHT_IN_DIPS;
                    m_pageWidth  = PAGE_WIDTH_IN_DIPS;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = E_HANDLE;
        }
    }

    // Convert DEVMODE to a job print ticket stream.
    if (SUCCEEDED(hr))
    {
        hr = GetPrintTicketFromDevmode(
            printerName,
            devMode,
            devMode->dmSize + devMode->dmDriverExtra, // Size of DEVMODE in bytes, including private driver data.
            &m_jobPrintTicketStream
            );
    }

    // Create a factory for document print job.
    IPrintDocumentPackageTargetFactory* documentTargetFactory = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = ::CoCreateInstance(
            __uuidof(PrintDocumentPackageTargetFactory),
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&documentTargetFactory)
            );
    }

    // Initialize the print subsystem and get a package target.
    if (SUCCEEDED(hr))
    {
        hr = documentTargetFactory->CreateDocumentPackageTargetForPrintJob(
            printerName,                                // printer name
            L"Direct2D desktop app printing sample",    // job name
            nullptr,                                    // job output stream; when nullptr, send to printer
            m_jobPrintTicketStream,                     // job print ticket
            &m_documentTarget                           // result IPrintDocumentPackageTarget object
            );
    }

    // Create a new print control linked to the package target.
    if (SUCCEEDED(hr))
    {
        hr = m_d2dDevice->CreatePrintControl(
            m_wicFactory,
            m_documentTarget,
            nullptr,
            &m_printControl
            );
    }

    // Create and register a print job checker.
    if (SUCCEEDED(hr))
    {
        SafeRelease(&m_printJobChecker);
        m_printJobChecker = new D2DPrintJobChecker;
        hr = (m_printJobChecker != nullptr) ? S_OK : E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
        hr = m_printJobChecker->Initialize(m_documentTarget);
    }

    // Release resources.
    if (devMode)
    {
        GlobalUnlock(printDialogEx.hDevMode);
        devMode = nullptr;
    }
    if (devNames)
    {
        GlobalUnlock(printDialogEx.hDevNames);
        devNames = nullptr;
    }
    if (printDialogEx.hDevNames)
    {
        GlobalFree(printDialogEx.hDevNames);
    }
    if (printDialogEx.hDevMode)
    {
        GlobalFree(printDialogEx.hDevMode);
    }

    SafeRelease(&documentTargetFactory);

    return hr;
}

// Creates a print job ticket stream to define options for the next print job.
HRESULT DemoApp::GetPrintTicketFromDevmode(
    _In_ PCTSTR printerName,
    _In_reads_bytes_(devModesize) PDEVMODE devMode,
    WORD devModesize,
    _Out_ LPSTREAM* printTicketStream)
{
    HRESULT hr = S_OK;
    HPTPROVIDER provider = nullptr;

    *printTicketStream = nullptr;

    // Allocate stream for print ticket.
    hr = CreateStreamOnHGlobal(nullptr, TRUE, printTicketStream);

    if (SUCCEEDED(hr))
    {
        hr = PTOpenProvider(printerName, 1, &provider);
    }

    // Get PrintTicket from DEVMODE.
    if (SUCCEEDED(hr))
    {
        hr = PTConvertDevModeToPrintTicket(provider, devModesize, devMode, kPTJobScope, *printTicketStream);
    }

    if (FAILED(hr) && printTicketStream != nullptr)
    {
        // Release printTicketStream if fails.
        SafeRelease(printTicketStream);
    }

    if (provider)
    {
        PTCloseProvider(provider);
    }

    return hr;
}


// Commits the current print job to the printing subystem by
// closing the print control, and releases all printing-
// specific resources.
HRESULT DemoApp::FinalizePrintJob()
{
    // Send the print job to the print subsystem. (When this call
    // returns, we are safe to release printing resources.)
    HRESULT hr = m_printControl->Close();

    SafeRelease(&m_jobPrintTicketStream);
    SafeRelease(&m_printControl);

    return hr;
}

// Called whenever the application window is resized. Recreates
// the swap chain with buffers sized to the new window.
void DemoApp::OnResize()
{
    if (m_d2dContext)
    {
        HRESULT hr = S_OK;

        D2D1_SIZE_U newSize = CalculateD2DWindowSize();

        // Resize the child window.
        MoveWindow(m_d2dHwnd, 0, 0, newSize.width, newSize.height, FALSE);

        // Remove the bitmap from rendering device context.
        m_d2dContext->SetTarget(nullptr);

        // Resize the swap chain.
        if (SUCCEEDED(hr))
        {
            hr = m_swapChain->ResizeBuffers(
                0,
                newSize.width,
                newSize.height,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                0
                );
        }

        // Get a surface from the swap chain.
        IDXGISurface* surface = nullptr;
        if (SUCCEEDED(hr))
        {
            hr = m_swapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&surface)
                );
        }

        // Create a bitmap pointing to the surface.
        ID2D1Bitmap1* bitmap = nullptr;
        if (SUCCEEDED(hr))
        {
            FLOAT dpiX, dpiY;
            m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
            D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE
                    ),
                dpiX,
                dpiY
                );
            hr = m_d2dContext->CreateBitmapFromDxgiSurface(
                surface,
                &properties,
                &bitmap
                );
        }

        // Set bitmap back onto device context.
        if (SUCCEEDED(hr))
        {
            m_d2dContext->SetTarget(bitmap);
        }

        SafeRelease(&bitmap);
        SafeRelease(&surface);

        ResetScrollBar();

        // Force a repaint.
        InvalidateRect(m_d2dHwnd, nullptr, FALSE);
    }
}

// The window message handler for the parent window.
LRESULT CALLBACK DemoApp::ParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        DemoApp* demoApp = reinterpret_cast<DemoApp*>(createStruct->lpCreateParams);

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<ULONG_PTR>(demoApp)
            );

        result = 1;
    }
    else
    {
        DemoApp* demoApp = reinterpret_cast<DemoApp*>(
            static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );

        bool wasHandled = false;

        if (demoApp)
        {
            switch (message)
            {
            case WM_CHAR:
                {
                    demoApp->OnChar(static_cast<SHORT>(wParam));
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_SIZE:
                if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
                {
                    demoApp->OnResize();
                    result = 0;
                    wasHandled = true;
                }
                break;

            case WM_VSCROLL:
                {
                    demoApp->OnVScroll(wParam, lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEWHEEL:
                {
                    demoApp->OnMouseWheel(wParam, lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_CLOSE:
                {
                    result = demoApp->OnClose();
                }
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

// The window message handler for the child window (with the D2D content).
LRESULT CALLBACK DemoApp::ChildWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        DemoApp* demoApp = reinterpret_cast<DemoApp*>(createStruct->lpCreateParams);

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<ULONG_PTR>(demoApp)
            );

        result = 1;
    }
    else
    {
        DemoApp* demoApp = reinterpret_cast<DemoApp*>(
            static_cast<LONG_PTR>(
                ::GetWindowLongPtrW(hwnd, GWLP_USERDATA)
                )
            );

        bool wasHandled = false;

        if (demoApp)
        {
            switch (message)
            {
            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
                {
                    PAINTSTRUCT paintStruct;
                    BeginPaint(hwnd, &paintStruct);
                    demoApp->OnRender();
                    EndPaint(hwnd, &paintStruct);
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

// Creates a Direct2D bitmap from the specified file name.
HRESULT DemoApp::LoadBitmapFromFile(
    _In_ ID2D1DeviceContext* d2dContext,
    _In_ IWICImagingFactory* wicFactory,
    _In_ PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight,
    _Outptr_ ID2D1Bitmap** bitmap
    )
{
    IWICBitmapDecoder* bitmapDecoder = nullptr;
    HRESULT hr = wicFactory->CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &bitmapDecoder
        );

    IWICBitmapFrameDecode* frameDecode = nullptr;
    if (SUCCEEDED(hr))
    {
        // Create the initial frame.
        hr = bitmapDecoder->GetFrame(0, &frameDecode);
    }

    IWICFormatConverter* formatConverter = nullptr;
    if (SUCCEEDED(hr))
    {
        // Convert the image format to 32bppPBGRA
        // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
        hr = wicFactory->CreateFormatConverter(&formatConverter);
    }

    IWICBitmapScaler* bitmapScaler = nullptr;
    if (SUCCEEDED(hr))
    {
        if (destinationWidth == 0 && destinationHeight == 0)
        {
            // Don't scale the image.
            hr = formatConverter->Initialize(
                frameDecode,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0f,
                WICBitmapPaletteTypeMedianCut
                );
        }
        else
        {
            // If a new width or height was specified, create an
            // IWICBitmapScaler and use it to resize the image.
            UINT originalWidth;
            UINT originalHeight;
            hr = frameDecode->GetSize(&originalWidth, &originalHeight);

            if (SUCCEEDED(hr))
            {
                if (destinationWidth == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                    destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (destinationHeight == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                    destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = wicFactory->CreateBitmapScaler(&bitmapScaler);

                if (SUCCEEDED(hr))
                {
                    hr = bitmapScaler->Initialize(
                        frameDecode,
                        destinationWidth,
                        destinationHeight,
                        WICBitmapInterpolationModeCubic
                        );
                }

                if (SUCCEEDED(hr))
                {
                    hr = formatConverter->Initialize(
                        bitmapScaler,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        nullptr,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Create a Direct2D bitmap from the WIC bitmap.
        hr = d2dContext->CreateBitmapFromWicBitmap(
            formatConverter,
            nullptr,
            bitmap
            );
    }

    SafeRelease(&bitmapDecoder);
    SafeRelease(&frameDecode);
    SafeRelease(&formatConverter);
    SafeRelease(&bitmapScaler);

    return hr;
}

// Called when the application receives a WM_VSCROLL message is sent.
// Adjusts the application's scroll position so that subsequent renderings
// reveal a higher or lower section of the scene.
void DemoApp::OnVScroll(WPARAM wParam, LPARAM /* lParam */)
{
    INT newScrollPosition = m_currentScrollPosition;

    switch (LOWORD(wParam))
    {
    case SB_LINEUP:
        newScrollPosition -= 5;
        break;

    case SB_LINEDOWN:
        newScrollPosition += 5;
        break;

    case SB_PAGEUP:
        newScrollPosition -= static_cast<UINT>(m_d2dContext->GetSize().height);
        break;

    case SB_PAGEDOWN:
        newScrollPosition += static_cast<UINT>(m_d2dContext->GetSize().height);
        break;

    case SB_THUMBTRACK:
        {
            SCROLLINFO scrollInfo = {0};
            scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
            BOOL succeeded = GetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo);
            if (!succeeded)
            {
                Assert(succeeded);
                return;
            }
            newScrollPosition = scrollInfo.nTrackPos;
        }
        break;

    default:
        break;
    }

    newScrollPosition = max(0, min(newScrollPosition, static_cast<INT>(m_scrollRange) - static_cast<INT>(m_d2dContext->GetSize().height)));

    m_currentScrollPosition = newScrollPosition;

    SCROLLINFO scrollInfo = {0};
    scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    BOOL succeeded = GetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo);
    if (!succeeded)
    {
        Assert(succeeded);
        return;
    }

    if (m_currentScrollPosition != scrollInfo.nPos)
    {
        scrollInfo.nPos = m_currentScrollPosition;
        SetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo, TRUE);

        InvalidateRect(m_d2dHwnd, nullptr, FALSE);
    }
}

// Called when the mouse wheel is moved. Adjusts the application's
// scroll position.
void DemoApp::OnMouseWheel(WPARAM wParam, LPARAM /* lParam */)
{
    m_currentScrollPosition -= GET_WHEEL_DELTA_WPARAM(wParam);
    m_currentScrollPosition = max(0, min(m_currentScrollPosition, static_cast<INT>(m_scrollRange) - static_cast<INT>(m_d2dContext->GetSize().height)));

    SCROLLINFO scrollInfo = {0};
    scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    BOOL succeeded = GetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo);
    if (!succeeded)
    {
        Assert(succeeded);
        return;
    }

    if (m_currentScrollPosition != scrollInfo.nPos)
    {
        scrollInfo.nPos = m_currentScrollPosition;
        SetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo, TRUE);

        InvalidateRect(m_d2dHwnd, nullptr, FALSE);
    }
}

// Resets the scroll bar to represent the current size of the
// application window and the current scroll position.
void DemoApp::ResetScrollBar()
{
    INT scrollPos = static_cast<INT>(m_scrollRange - m_d2dContext->GetSize().height + 0.5);
    m_currentScrollPosition = max(0, min(m_currentScrollPosition, scrollPos));

    SCROLLINFO scrollInfo = {0};
    scrollInfo.cbSize = sizeof(scrollInfo);
    scrollInfo.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
    scrollInfo.nMin = 0;
    scrollInfo.nMax = m_scrollRange;
    scrollInfo.nPage = static_cast<UINT>(m_d2dContext->GetSize().height);
    scrollInfo.nPos = m_currentScrollPosition;
    SetScrollInfo(m_parentHwnd, SB_VERT, &scrollInfo, TRUE);
}

// Close the sample window after checking print job status.
LRESULT DemoApp::OnClose()
{
    bool close = true;

    if (m_printJobChecker != nullptr)
    {
        PrintDocumentPackageStatus status = m_printJobChecker->GetStatus();
        if (status.Completion == PrintDocumentPackageCompletion_InProgress)
        {
            int selection = MessageBox(
                m_d2dHwnd,
                L"Print job still in progress.\nYES to force to exit;\nNO to exit after print job is complete;\nCANCEL to return to sample.",
                L"Sample exit error",
                MB_YESNOCANCEL | MB_ICONSTOP
                );
            switch (selection)
            {
                case IDYES:
                    // Force to exit.
                    break;

                case IDNO:
                    // Exit after print job is complete.
                    m_printJobChecker->WaitForCompletion();
                    break;

                case IDCANCEL:
                    // Return to sample.
                    close = false;
                    break;

                default:
                    close = false;
                    break;
            }
        }
    }

    if (close)
    {
        DestroyWindow(m_d2dHwnd);
        DestroyWindow(m_parentHwnd);
    }

    return close ? 0 : 1;
}
