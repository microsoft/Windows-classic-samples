// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

static const PCWSTR sc_fontName = L"Calibri";

static const float sc_fontSize = 20.0f;

static const D2D1_RECT_F sc_textInfoBox = {10, 10, 350, 200 };
static const float sc_textInfoBoxInset = 10;

static const UINT sc_defaultNumSquares = 16;
static const UINT sc_minNumSquares = 1;
static const UINT sc_maxNumSquares = 1024;

static const float sc_boardWidth = 900.0f;

static const float sc_rotationSpeed = 3.0f;

static const float sc_loupeInset = 20.0f;

static const float sc_maxZoom = 15.0f;
static const float sc_minZoom = 1.0f;
static const float sc_zoomStep = 1.5f;
static const float sc_zoomSubStep = 1.1f;

static const float sc_strokeWidth = 1.0f;

// This determines that maximum texture size we will
// generate for our realizations.
static const UINT sc_maxRealizationDimension = 2000;

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
    m_antialiasMode(m_antialiasMode),
    m_useRealizations(false),
    m_updateRealization(true),
    m_drawStroke(true),
    m_autoGeometryRegen(true),
    m_paused(false),
    m_pausedTime(0),
    m_numSquares(sc_defaultNumSquares),
    m_targetZoomFactor(1.0f),
    m_currentZoomFactor(1.0f),
    m_pD2DFactory(NULL),
    m_pWICFactory(NULL),
    m_pDWriteFactory(NULL),
    m_pRT(NULL),
    m_pTextFormat(NULL),
    m_pSolidColorBrush(NULL),
    m_pRealization(NULL),
    m_pGeometry(NULL)
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    m_timeDelta = -time.QuadPart;

    m_mousePos = D2D1::Point2F(0.0f, 0.0f);
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
    SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pTextFormat);
    SafeRelease(&m_pSolidColorBrush);
    SafeRelease(&m_pRealization);
    SafeRelease(&m_pGeometry);
}

/******************************************************************
*                                                                 *
*  DemoApp::Initialize                                            *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/

HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        //register window class
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
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
            L"D2D Demo App",
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
*  duration of the app. These resources include the D2D,          *
*  DWrite, and WIC factories; and a DWrite Text Format object     *
*  (used for identifying particular font characteristics) and     *
*  a D2D geometry.                                                *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create the Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    if (SUCCEEDED(hr))
    {
        // Create a WIC factory.
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void **>(&m_pWICFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite text format object.
        hr = m_pDWriteFactory->CreateTextFormat(
            sc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(sc_fontSize),
            L"", //locale
            &m_pTextFormat
            );
    }

    return hr;
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

    if (!m_pRT)
    {
        IGeometryRealizationFactory *pRealizationFactory = NULL;

        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        // Create a Direct2D render target.
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );
        if (SUCCEEDED(hr))
        {
            // Create brushes.
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &m_pSolidColorBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = CreateGeometryRealizationFactory(
                m_pRT,
                sc_maxRealizationDimension,
                &pRealizationFactory
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = pRealizationFactory->CreateGeometryRealization(&m_pRealization);
        }
        if (SUCCEEDED(hr))
        {
            m_updateRealization = true;
        }

        SafeRelease(&pRealizationFactory);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::DiscardDeviceResources                                *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a Direct3D device is lost.                                *
*                                                                 *
******************************************************************/

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pSolidColorBrush);
    SafeRelease(&m_pRealization);
}

/******************************************************************
*                                                                 *
*  DemoApp::DiscardGeometryData                                   *
*                                                                 *
******************************************************************/

void DemoApp::DiscardGeometryData()
{
    SafeRelease(&m_pGeometry);

    m_updateRealization = true;
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


HRESULT DemoApp::CreateGeometries()
{
    HRESULT hr = S_OK;

    if (!m_pGeometry)
    {
        IGeometryRealizationFactory *pRealizationFactory = NULL;
        IGeometryRealization *pRealization = NULL;

        ID2D1TransformedGeometry *pGeometry = NULL;
        ID2D1PathGeometry *pPathGeometry = NULL;
        ID2D1GeometrySink *pSink = NULL;

        float squareWidth = 0.9f * sc_boardWidth / m_numSquares;

        // Create the path geometry.
        hr = m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
        if (SUCCEEDED(hr))
        {
            // Write to the path geometry using the geometry sink to 
            // create an hour glass shape.
            hr = pPathGeometry->Open(&pSink);
        }
        if (SUCCEEDED(hr))
        {
            pSink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

            pSink->BeginFigure(
                D2D1::Point2F(0, 0),
                D2D1_FIGURE_BEGIN_FILLED
                );

            pSink->AddLine(D2D1::Point2F(1.0f, 0));

            pSink->AddBezier(
                D2D1::BezierSegment(
                    D2D1::Point2F(0.75f, 0.25f),
                    D2D1::Point2F(0.75f, 0.75f),
                    D2D1::Point2F(1.0f, 1.0f))
                );

            pSink->AddLine(D2D1::Point2F(0, 1.0f));

            pSink->AddBezier(
                D2D1::BezierSegment(
                    D2D1::Point2F(0.25f, 0.75f),
                    D2D1::Point2F(0.25f, 0.25f),
                    D2D1::Point2F(0, 0))
                );

            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

            hr = pSink->Close();
        }

        if (SUCCEEDED(hr))
        {
            D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(squareWidth, squareWidth);
            D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(-squareWidth / 2, -squareWidth / 2);

            hr = m_pD2DFactory->CreateTransformedGeometry(
                    pPathGeometry,
                    scale * translation,
                    &pGeometry
                    );
        }

        if (SUCCEEDED(hr))
        {
            // Transfer the reference.
            m_pGeometry = pGeometry;
            pGeometry = NULL;
        }

        SafeRelease(&pRealizationFactory);
        SafeRelease(&pRealization);
        SafeRelease(&pGeometry);
        SafeRelease(&pPathGeometry);
        SafeRelease(&pSink);
    }

    return hr;
}


HRESULT DemoApp::RenderMainContent(float time)
{
    HRESULT hr = S_OK;

    static DWORD dwTimeStart = 0;
    static DWORD dwTimeLast = 0;

    D2D1_SIZE_F rtSize = m_pRT->GetSize();

    float squareWidth = sc_boardWidth / m_numSquares;

    m_pRT->SetAntialiasMode(m_antialiasMode);

    m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    D2D1_MATRIX_3X2_F currentTransform;

    m_pRT->GetTransform(&currentTransform);

    D2D1_MATRIX_3X2_F worldTransform =
        D2D1::Matrix3x2F::Translation(
            0.5f*(rtSize.width - squareWidth*m_numSquares),
            0.5f*(rtSize.height - squareWidth*m_numSquares)
            ) * currentTransform;

    for (UINT i = 0; SUCCEEDED(hr) && (i < m_numSquares); ++i)
    {
        for (UINT j = 0; SUCCEEDED(hr) && (j < m_numSquares); ++j)
        {
            float dx = i+0.5f-0.5f*m_numSquares;
            float dy = j+0.5f-0.5f*m_numSquares;

            float length = sqrtf(2)*m_numSquares;

            //
            // The intensity variable determines the color and speed of rotation of the
            // realization instance. We choose a function that is rotationaly
            // symmetric about the center of the grid, which produces a nice
            // effect.
            //
            float intensity =
                0.5f * (1+sinf( (0.2f * time + 10.0f * sqrtf(static_cast<float>(dx*dx+dy*dy))/length)));

            D2D1_MATRIX_3X2_F rotateTransform =
                D2D1::Matrix3x2F::Rotation(
                    (intensity * sc_rotationSpeed * time * 360.0f) * ((float)M_PI / 180.0f)
                    );

            D2D1_MATRIX_3X2_F newWorldTransform =
                rotateTransform *
                D2D1::Matrix3x2F::Translation(
                    (i+0.5f)*squareWidth,
                    (j+0.5f)*squareWidth
                    ) * worldTransform;

            if (m_updateRealization)
            {
                //
                // Note: It would actually be a little simpler to generate our
                // realizations prior to entering RenderMainContent. We instead
                // generate the realizations based on the top-left primitive in
                // the grid, so we can illustrate the fact that realizations
                // appear identical to their unrealized counter-parts when the
                // exact same world transform is supplied. Only the top left
                // realization will look identical, though, as shifting or
                // rotating an AA realization can introduce fuzziness.
                //
                // Realizations are regenerated every frame, so to
                // demonstrate that the realization geometry produces identical
                // results, you actually need to pause (<space>), which forces
                // a regeneration.
                //
                hr = m_pRealization->Update(
                    m_pGeometry,
                    static_cast<REALIZATION_CREATION_OPTIONS>(
                        REALIZATION_CREATION_OPTIONS_ANTI_ALIASED |
                        REALIZATION_CREATION_OPTIONS_ALIASED |
                        REALIZATION_CREATION_OPTIONS_FILLED |
                        REALIZATION_CREATION_OPTIONS_STROKED |
                        REALIZATION_CREATION_OPTIONS_UNREALIZED
                        ),
                    &newWorldTransform,
                    sc_strokeWidth,
                    NULL //pIStrokeStyle
                    );
                if (SUCCEEDED(hr))
                {
                    m_updateRealization = false;
                }
            }
            if (SUCCEEDED(hr))
            {
                m_pRT->SetTransform(newWorldTransform);

                m_pSolidColorBrush->SetColor(
                    D2D1::ColorF(
                        0.0f,
                        intensity,
                        1.0f - intensity
                        ));

                hr = m_pRealization->Fill(
                        m_pRT,
                        m_pSolidColorBrush,
                        m_useRealizations ?
                            REALIZATION_RENDER_MODE_DEFAULT :
                            REALIZATION_RENDER_MODE_FORCE_UNREALIZED
                        );

                if (SUCCEEDED(hr) && m_drawStroke)
                {
                    m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

                    hr = m_pRealization->Draw(
                            m_pRT,
                            m_pSolidColorBrush,
                            m_useRealizations ?
                                REALIZATION_RENDER_MODE_DEFAULT :
                                REALIZATION_RENDER_MODE_FORCE_UNREALIZED
                            );
                }
            }
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window. This method draws the main content (a 2D array of      *
*  spinning geometries) and some perf statistics.                 *
*                                                                 *
*  Note that this function will not render anything if the window *
*  is occluded (e.g. when the screen is locked).                   *
*  Also, this function will automatically discard device-specific *
*  resources if the D3D device disappears during function         *
*  invocation, and will recreate the resources the next time it's *
*  invoked.                                                       *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnRender()
{
    HRESULT hr = S_OK;

    LARGE_INTEGER time;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&time);
    QueryPerformanceFrequency(&frequency);

    float floatTime;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        if (!m_paused)
        {
            floatTime = static_cast<float>(time.QuadPart + m_timeDelta)/static_cast<float>(frequency.QuadPart);
        }
        else
        {
            floatTime = static_cast<float>(m_pausedTime + m_timeDelta)/static_cast<float>(frequency.QuadPart);
        }

        m_times.Add(time.QuadPart);

        if (m_currentZoomFactor < m_targetZoomFactor)
        {
            m_currentZoomFactor *= sc_zoomSubStep;

            if (m_currentZoomFactor > m_targetZoomFactor)
            {
                m_currentZoomFactor = m_targetZoomFactor;

                if (m_autoGeometryRegen)
                {
                    m_updateRealization = true;
                }
            }
        }
        else if (m_currentZoomFactor > m_targetZoomFactor)
        {
            m_currentZoomFactor /= sc_zoomSubStep;

            if (m_currentZoomFactor < m_targetZoomFactor)
            {
                m_currentZoomFactor = m_targetZoomFactor;

                if (m_autoGeometryRegen)
                {
                    m_updateRealization = true;
                }
            }
        }

        m_pRT->SetTransform(
            D2D1::Matrix3x2F::Scale(
                m_currentZoomFactor,
                m_currentZoomFactor,
                m_mousePos)
            );

        hr = CreateGeometries();

        if (SUCCEEDED(hr) && m_pRT && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            m_pRT->BeginDraw();

            hr = RenderMainContent(floatTime);
            if (SUCCEEDED(hr))
            {
                hr = RenderTextInfo();
                if (SUCCEEDED(hr))
                {
                    hr = m_pRT->EndDraw();
                    if (SUCCEEDED(hr))
                    {
                        if (hr == D2DERR_RECREATE_TARGET)
                        {
                            DiscardDeviceResources();
                        }
                    }
                }
            }
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::RenderTextInfo                                        *
*                                                                 *
*  Draw the stats text (AA type, fps, etc...).                    *
*                                                                 *
******************************************************************/

HRESULT DemoApp::RenderTextInfo()
{
    HRESULT hr = S_OK;

    WCHAR textBuffer[400];
    LARGE_INTEGER frequency;
    float fps = 0.0f;
    float primsPerSecond = 0.0f;

    QueryPerformanceFrequency(&frequency);

    UINT numPrimitives = m_numSquares * m_numSquares;

    if (m_drawStroke)
    {
        numPrimitives *= 2;
    }

    if (m_times.GetCount() > 0)
    {
        fps = (m_times.GetCount()-1) * frequency.QuadPart /
                static_cast<float>((m_times.GetLast() - m_times.GetFirst()));

        primsPerSecond = fps * numPrimitives;
    }

    hr = StringCchPrintf(
        textBuffer,
        400,
        L"%s\n"
        L"%s\n"
        L"%s\n"
        L"# primitives: %d x %d%s = %d\n"
        L"Fps: %.2f\n"
        L"Primitives / sec : %.0f\n",
        m_antialiasMode == D2D1_ANTIALIAS_MODE_ALIASED ?
             L"Aliased" : L"PerPrimitive",
        m_useRealizations ?
            L"Realized"  : L"Unrealized",
        m_autoGeometryRegen?
            L"Auto Realization Regeneration"  : L"No Auto Realization Regeneration",
        m_numSquares,
        m_numSquares,
        m_drawStroke ? L" x 2" : L"",
        numPrimitives,
        fps,
        primsPerSecond
        );
    if (SUCCEEDED(hr))
    {
        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pSolidColorBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.5f));

        m_pRT->FillRoundedRectangle(
            D2D1::RoundedRect(
                sc_textInfoBox,
                sc_textInfoBoxInset,
                sc_textInfoBoxInset),
            m_pSolidColorBrush
            );

        m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

        m_pRT->DrawText(
            textBuffer,
            static_cast<UINT>(wcsnlen(textBuffer, ARRAYSIZE(textBuffer))),
            m_pTextFormat,
            D2D1::RectF(
                sc_textInfoBox.left + sc_textInfoBoxInset,
                sc_textInfoBox.top + sc_textInfoBoxInset,
                sc_textInfoBox.right - sc_textInfoBoxInset,
                sc_textInfoBox.bottom - sc_textInfoBoxInset),
            m_pSolidColorBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE
            );
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::OnResize                                              *
*                                                                 *
*  If the application receives a WM_SIZE message, this method     *
*  resizes the render target appropriately.                       *
*                                                                 *
******************************************************************/

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRT)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;

        // Note: This method can fail, but it's okay to ignore the
        // error here -- it will be repeated on the next call to
        // EndDraw.
        m_pRT->Resize(size);
    }
}

/******************************************************************
*                                                                 *
*  DemoApp::OnKeyDown                                             *
*                                                                 *
******************************************************************/

void DemoApp::OnKeyDown(SHORT vkey)
{
    switch (vkey)
    {
    case 'A':
        m_antialiasMode =
            (m_antialiasMode == D2D1_ANTIALIAS_MODE_ALIASED) ?
                D2D1_ANTIALIAS_MODE_PER_PRIMITIVE :
                D2D1_ANTIALIAS_MODE_ALIASED;
        break;

    case 'R':
        m_useRealizations = !m_useRealizations;
        break;

    case 'G':
        m_autoGeometryRegen = !m_autoGeometryRegen;
        break;

    case 'S':
        m_drawStroke = !m_drawStroke;
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
        m_updateRealization = true;

        break;

    case VK_UP:
        m_numSquares = min(m_numSquares * 2, sc_maxNumSquares);

        // Regenerate the geometries.
        DiscardGeometryData();
        break;

    case VK_DOWN:
        m_numSquares = max(m_numSquares / 2, sc_minNumSquares);

        // Regenerate the geometries.
        DiscardGeometryData();
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

void DemoApp::OnMouseMove(LPARAM lParam)
{
    float dpiX = 96.0f;
    float dpiY = 96.0f;

    if (m_pRT)
    {
        m_pRT->GetDpi(&dpiX, &dpiY);
    }

    m_mousePos = D2D1::Point2F(
        LOWORD(lParam) * 96.0f / dpiX,
        HIWORD(lParam) * 96.0f / dpiY
        );
}

/******************************************************************
*                                                                 *
*  DemoApp::OnWheel                                               *
*                                                                 *
******************************************************************/

void DemoApp::OnWheel(WPARAM wParam)
{
    m_targetZoomFactor *=
        pow(sc_zoomStep, GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f);

    m_targetZoomFactor = min(
        max(m_targetZoomFactor, sc_minZoom),
        sc_maxZoom
        );
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
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDemoApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);

                    pDemoApp->OnRender();
                    EndPaint(hwnd, &ps);

                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_KEYDOWN:
                {
                    pDemoApp->OnKeyDown(static_cast<SHORT>(wParam));
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEMOVE:
                {
                    pDemoApp->OnMouseMove(lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEWHEEL:
                {
                    pDemoApp->OnWheel(wParam);
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
