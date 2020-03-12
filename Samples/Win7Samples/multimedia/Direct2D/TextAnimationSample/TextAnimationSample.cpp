// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "TextAnimationSample.h"


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
    m_pDWriteFactory(NULL),
    m_pRT(NULL),
    m_pTextFormat(NULL),
    m_pTextLayout(NULL),
    m_pBlackBrush(NULL),
    m_pOpacityRT(NULL)
{
    m_startTime = 0;
    m_animationStyle = AnimationStyle::Translation;
    m_renderingMethod = TextRenderingMethod::Default;
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
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pTextFormat);
    SafeRelease(&m_pTextLayout);
    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pOpacityRT);
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

    m_fRunning = FALSE;

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
        FLOAT dpiX;
        FLOAT dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"D2D Demo App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.0f * dpiX / 96.0f)),
            static_cast<UINT>(ceil(480.0f * dpiY / 96.0f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            m_fRunning = TRUE;

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
*  DWrite factories; and a DWrite Text Format object              *
*  (used for identifying particular font characteristics) and     *
*  a D2D geometry.                                                *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    static const WCHAR msc_fontName[] = L"Gabriola";
    static const FLOAT msc_fontSize = 50;
    static const WCHAR sc_helloWorld[] = L"The quick brown fox jumped over the lazy dog!";
    static const UINT stringLength = ARRAYSIZE(sc_helloWorld) - 1;
    HRESULT hr;

    //create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (SUCCEEDED(hr))
    {
        //create DWrite factory
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_ISOLATED, //DWRITE_FACTORY_TYPE_SHARED
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
        //create DWrite text format object
        hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            msc_fontSize,
            L"", //locale
            &m_pTextFormat
            );
    }
    if (SUCCEEDED(hr))
    {
        //center the text horizontally
        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        hr = m_pDWriteFactory->CreateTextLayout(
            &sc_helloWorld[0],
            stringLength,
            m_pTextFormat,
            300, // maxWidth
            1000, // maxHeight
            &m_pTextLayout
            );
    }
    if (SUCCEEDED(hr))
    {
        //
        // We use typographic features here to show how to account for the
        // overhangs that these features will produce. See the code in
        // ResetAnimation that calls GetOverhangMetrics(). Note that there are
        // fonts that can produce overhangs even without the use of typographic
        // features- this is just one example.
        //
        IDWriteTypography *pTypography = NULL;
        hr = m_pDWriteFactory->CreateTypography(&pTypography);
        if (SUCCEEDED(hr))
        {
            DWRITE_FONT_FEATURE fontFeature =
            {
                DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,
                1
            };
            hr = pTypography->AddFontFeature(fontFeature);
            if (SUCCEEDED(hr))
            {
                DWRITE_TEXT_RANGE textRange = {0, stringLength};
                hr = m_pTextLayout->SetTypography(pTypography, textRange);
            }

            pTypography->Release();
        }
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
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        //
        // Create a D2D render target
        //
        // Note: we only use D2D1_PRESENT_OPTIONS_IMMEDIATELY so that we can
        // easily measure the framerate. Most apps should not use this
        // flag.
        //
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
            &m_pRT
            );
        if (SUCCEEDED(hr))
        {
            //
            // Nothing in this sample requires antialiasing so we set the antialias
            // mode to aliased up front.
            //
            m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

            //create a black brush
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBlackBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = ResetAnimation(
                true // resetClock
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
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pOpacityRT);
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
    while (this->IsRunning())
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

/******************************************************************
*                                                                 *
*  DemoApp::OnChar                                                *
*                                                                 *
*  Responds to input from the user.                               *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnChar(SHORT key)
{
    HRESULT hr = S_OK;

    bool resetAnimation = true;
    bool resetClock = true;

    switch (key)
    {
    case 't':
        if (m_animationStyle & AnimationStyle::Translation)
        {
            m_animationStyle &= ~AnimationStyle::Translation;
        }
        else
        {
            m_animationStyle |= AnimationStyle::Translation;
        }
        break;

    case 'r':
        if (m_animationStyle & AnimationStyle::Rotation)
        {
            m_animationStyle &= ~AnimationStyle::Rotation;
        }
        else
        {
            m_animationStyle |= AnimationStyle::Rotation;
        }
        break;

    case 's':
        if (m_animationStyle & AnimationStyle::Scaling)
        {
            m_animationStyle &= ~AnimationStyle::Scaling;
        }
        else
        {
            m_animationStyle |= AnimationStyle::Scaling;
        }
        break;

    case '1':
    case '2':
    case '3':
        m_renderingMethod = static_cast<TextRenderingMethod::Enum>(key - '1');
        resetClock = false;
        break;

    default:
        resetAnimation = false;
        resetClock = false;
    }

    if (resetAnimation)
    {
        hr = ResetAnimation(resetClock);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::UpdateWindowText                                      *
*                                                                 *
*  This method updates the window title bar with info about the   *
*  current animation style and rendering method. It also outputs  *
*  the framerate.                                                 *
*                                                                 *
******************************************************************/

void DemoApp::UpdateWindowText()
{
    static LONGLONG sc_lastTimeStatusShown = 0;

    //
    // Update the window status no more than 10 times a second. Without this
    // check, the performance bottleneck could potentially be the time it takes
    // for Windows to update the title.
    //
    if (   m_times.GetCount() > 0
        && m_times.GetLast() > sc_lastTimeStatusShown + 1000000
       )
    {
        //
        // Determine the frame rate by computing the difference in clock time
        // between this frame and the frame we rendered 10 frames ago.
        //
        sc_lastTimeStatusShown = m_times.GetLast();

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        float fps = 0.0f;
        if (m_times.GetCount() > 0)
        {
            fps = (m_times.GetCount()-1) * frequency.QuadPart /
                    static_cast<float>((m_times.GetLast() - m_times.GetFirst()));
        }

        //
        // Add other useful information to the window title.
        //

        wchar_t *style = NULL;
        switch (m_animationStyle)
        {
        case AnimationStyle::None:
            style = L"None";
            break;
        case AnimationStyle::Translation:
            style = L"Translation";
            break;
        case AnimationStyle::Rotation:
            style = L"Rotation";
            break;
        case AnimationStyle::Scaling:
            style = L"Scale";
            break;
        }

        wchar_t *method = NULL;
        switch (m_renderingMethod)
        {
        case TextRenderingMethod::Default:
            method = L"Default";
            break;
        case TextRenderingMethod::Outline:
            method = L"Outline";
            break;
        case TextRenderingMethod::UseA8Target:
            method = L"UseA8Target";
            break;
        }

        wchar_t title[255];
        StringCchPrintf(
            title,
            ARRAYSIZE(title),
            L"AnimationStyle: %s%s%s, Method: %s, %#.1f fps",
            (m_animationStyle & AnimationStyle::Translation) ? L"+t" : L"-t",
            (m_animationStyle & AnimationStyle::Rotation) ? L"+r" : L"-r",
            (m_animationStyle & AnimationStyle::Scaling) ? L"+s" : L"-s",
            method,
            fps
            );

        SetWindowText(m_hwnd, title);
    }
}

/******************************************************************
*                                                                 *
*  DemoApp::ResetAnimation                                        *
*                                                                 *
*  This method does the necessary work to change the current      *
*  animation style.                                               *
*                                                                 *
******************************************************************/

HRESULT DemoApp::ResetAnimation(bool resetClock)
{
    HRESULT hr = S_OK;

    if (resetClock)
    {
        m_startTime = GetTickCount();
    }

    //
    // Release the opacity mask. We will regenerate it if the current animation
    // style demands it.
    //
    SafeRelease(&m_pOpacityRT);

    if (m_renderingMethod == TextRenderingMethod::Outline)
    {
        //
        // Set the rendering mode to OUTLINE mode. To do this we first create
        // a default params object and then make a copy with the given modification.
        //
        IDWriteRenderingParams *pDefaultParams = NULL;
        hr = m_pDWriteFactory->CreateRenderingParams(&pDefaultParams);

        if (SUCCEEDED(hr))
        {
            IDWriteRenderingParams *pRenderingParams = NULL;
            hr = m_pDWriteFactory->CreateCustomRenderingParams(
                pDefaultParams->GetGamma(),
                pDefaultParams->GetEnhancedContrast(),
                pDefaultParams->GetClearTypeLevel(),
                pDefaultParams->GetPixelGeometry(),
                DWRITE_RENDERING_MODE_OUTLINE,
                &pRenderingParams
                );
            if (SUCCEEDED(hr))
            {
                m_pRT->SetTextRenderingParams(pRenderingParams);

                pRenderingParams->Release();
            }
            pDefaultParams->Release();
        }
    }
    else
    {
        // Reset the rendering mode to default.
        m_pRT->SetTextRenderingParams(NULL);
    }

    if (SUCCEEDED(hr) && m_renderingMethod == TextRenderingMethod::UseA8Target)
    {
        //
        // Create a compatible A8 Target to store the text as an opacity mask.
        //
        // Note: To reduce sampling error in the scale animation, it might be
        //       preferable to create multiple masks for the text at different
        //       resolutions.
        //

        FLOAT dpiX;
        FLOAT dpiY;
        m_pRT->GetDpi(&dpiX, &dpiY);

        //
        // It is important to obtain the overhang metrics here in case the text
        // extends beyond the layout max-width and max-height.
        //
        DWRITE_OVERHANG_METRICS overhangMetrics;
        m_pTextLayout->GetOverhangMetrics(&overhangMetrics);

        //
        // Because the overhang metrics can be off slightly given that these
        // metrics do not account for antialiasing, we add an extra pixel for
        // padding.
        //
        D2D1_SIZE_F padding = D2D1::SizeF(96.0f / dpiX, 96.0f / dpiY);
        m_overhangOffset = D2D1::Point2F(ceil(overhangMetrics.left + padding.width), ceil(overhangMetrics.top + padding.height));

        //
        // The true width of the text is the max width + the overhang
        // metrics + padding in each direction.
        //
        D2D1_SIZE_F maskSize = D2D1::SizeF(
            overhangMetrics.right + padding.width + m_overhangOffset.x + m_pTextLayout->GetMaxWidth(),
            overhangMetrics.bottom + padding.height + m_overhangOffset.y + m_pTextLayout->GetMaxHeight()
            );

        // Round up to the nearest pixel
        D2D1_SIZE_U maskPixelSize = D2D1::SizeU(
            static_cast<UINT>(ceil(maskSize.width * dpiX / 96.0f)),
            static_cast<UINT>(ceil(maskSize.height * dpiY / 96.0f))
            );


        //
        // Create the compatible render target using desiredPixelSize to avoid
        // blurriness issues caused by a fractional-pixel desiredSize.
        //
        D2D1_PIXEL_FORMAT alphaOnlyFormat = D2D1::PixelFormat(DXGI_FORMAT_A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        hr = m_pRT->CreateCompatibleRenderTarget(
                NULL,
                &maskPixelSize,
                &alphaOnlyFormat,
                D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
                &m_pOpacityRT
                );
        if (SUCCEEDED(hr))
        {
            //
            // Draw the text to the opacity mask. Note that we can use pixel
            // snapping now given that subpixel translation can now happen during
            // the FillOpacityMask method.
            //
            m_pOpacityRT->BeginDraw();
            m_pOpacityRT->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
            m_pOpacityRT->DrawTextLayout(
                m_overhangOffset,
                m_pTextLayout,
                m_pBlackBrush,
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
                );
            hr = m_pOpacityRT->EndDraw();
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CalculateTransform                                    *
*                                                                 *
*  Calculates the transform based on the current time             *
*                                                                 *
******************************************************************/

void DemoApp::CalculateTransform(D2D1_MATRIX_3X2_F *pTransform)
{
    // calculate a 't' value that will linearly interpolate from 0 to 1 and back every 20 seconds
    DWORD currentTime = GetTickCount();
    if ( m_startTime == 0 )
    {
        m_startTime = currentTime;
    }
    float t = 2 * (( currentTime - m_startTime) % 20000) / 20000.0f;
    if (t > 1.0f)
    {
        t = 2 - t;
    }

    // calculate animation parameters
    float rotation = 0;
    float translationOffset = 0;
    float scaleMultiplier = 1.0f;
    if (m_animationStyle & AnimationStyle::Translation)
    {
        // range from -100 to 100
        translationOffset = (t - 0.5f) * 200;
    }

    if (m_animationStyle & AnimationStyle::Rotation)
    {
        // range from 0 to 360
        rotation = t * 360.0f;
    }

    if (m_animationStyle & AnimationStyle::Scaling)
    {
        // range from 1/4 to 2x the normal size
        scaleMultiplier = t * 1.75f + 0.25f;
    }

    D2D1_SIZE_F size = m_pRT->GetSize();

    *pTransform =
          D2D1::Matrix3x2F::Rotation(rotation)
        * D2D1::Matrix3x2F::Scale(scaleMultiplier, scaleMultiplier)
        * D2D1::Matrix3x2F::Translation(translationOffset + size.width / 2.0f, translationOffset + size.height / 2.0f);
}

/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window.                                                        *
*                                                                 *
*  Note that this function will not render anything if the window *
*  is occluded (e.g. when the screen is locked).                  *
*  Also, this function will automatically discard device-specific *
*  resources if the D3D device disappears during function         *
*  invocation, and will recreate the resources the next time it's *
*  invoked.                                                       *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnRender()
{
    HRESULT hr;

    //
    // We use a ring buffer to store the clock time for the last 10 frames.
    // This lets us eliminate a lot of noise when computing framerate.
    //
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    m_times.Add(time.QuadPart);

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        D2D1_MATRIX_3X2_F transform;
        CalculateTransform(&transform);

        m_pRT->BeginDraw();

        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

        m_pRT->SetTransform(transform);

        DWRITE_TEXT_METRICS textMetrics;
        m_pTextLayout->GetMetrics(&textMetrics);

        if (m_renderingMethod == TextRenderingMethod::UseA8Target)
        {
            //
            // Offset the destination rect such that the text will be centered
            // on the render target. Given that we have offset the text in the
            // A8 target by the overhang offset, we must factor that into the
            // destination rect now.
            //
            D2D1_SIZE_F opacityRTSize = m_pOpacityRT->GetSize();
            D2D1_POINT_2F offset = D2D1::Point2F(
                -textMetrics.width / 2.0f - m_overhangOffset.x,
                -textMetrics.height / 2.0f - m_overhangOffset.y
                );

            //
            // Round the offset to the nearest pixel. Note that the rounding
            // done here is unecessary, but it causes the text to be less
            // blurry.
            //
            FLOAT dpiX;
            FLOAT dpiY;
            m_pRT->GetDpi(&dpiX, &dpiY);
            D2D1_POINT_2F roundedOffset = D2D1::Point2F(
                floor(offset.x * dpiX / 96.0f + 0.5f) * 96.0f / dpiX,
                floor(offset.y * dpiY / 96.0f + 0.5f) * 96.0f / dpiY
                );

            D2D1_RECT_F destinationRect = D2D1::RectF(
                roundedOffset.x,
                roundedOffset.y,
                roundedOffset.x + opacityRTSize.width,
                roundedOffset.y + opacityRTSize.height
                );

            ID2D1Bitmap *pBitmap = NULL;
            m_pOpacityRT->GetBitmap(&pBitmap);

            pBitmap->GetDpi(&dpiX, &dpiY);

            //
            // The antialias mode must be set to D2D1_ANTIALIAS_MODE_ALIASED
            // for this method to succeed. We've set this mode already though
            // so no need to do it again.
            //
            m_pRT->FillOpacityMask(
                pBitmap,
                m_pBlackBrush,
                D2D1_OPACITY_MASK_CONTENT_TEXT_NATURAL,
                &destinationRect
                );

            pBitmap->Release();
        }
        else
        {
            // Disable pixel snapping to get a smoother animation.
            m_pRT->DrawTextLayout(
                D2D1::Point2F(-textMetrics.width / 2.0f, -textMetrics.height / 2.0f),
                m_pTextLayout,
                m_pBlackBrush,
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
                );
        }

        hr = m_pRT->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
        // To animate as quickly as possible, we request another WM_PAINT
        // immediately.
        InvalidateRect(m_hwnd, NULL, FALSE);
    }

    UpdateWindowText();

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
*  DemoApp::OnDestroy                                             *
*                                                                 *
*  If the application receives a WM_MOVE message, this method     *
*  takes the appropriate action.                                  *
*                                                                 *
******************************************************************/

void DemoApp::OnDestroy()
{
    m_fRunning = FALSE;
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
            switch(message)
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

            case WM_CHAR:
                {
                    pDemoApp->OnChar(static_cast<SHORT>(wParam));
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
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    pDemoApp->OnDestroy();
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
