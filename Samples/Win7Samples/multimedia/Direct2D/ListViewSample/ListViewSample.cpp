// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ListViewSample.h"

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
        // Allocate ListViewApp on the heap since it is a big object.
        ListViewApp *pApp = new (std::nothrow) ListViewApp;
        if (pApp)
        {
            if (SUCCEEDED(pApp->Initialize()))
            {
                pApp->RunMessageLoop();
            }

            delete pApp;
        }
        CoUninitialize();
    }

    return 0;
}

/******************************************************************
*                                                                 *
*  ListViewApp::ListViewApp constructor                           *
*                                                                 *
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

ListViewApp::ListViewApp() :
    m_d2dHwnd(NULL),
    m_parentHwnd(NULL),
    m_pD2DFactory(NULL),
    m_pWICFactory(NULL),
    m_pDWriteFactory(NULL),
    m_pRT(NULL),
    m_pTextFormat(NULL),
    m_pBlackBrush(NULL),
    m_pBindContext(NULL),
    m_pBitmapAtlas(NULL),
    m_numItemInfos(0),
    m_scrollRange(0),
    m_currentScrollPos(0),
    m_previousScrollPos(0),
    m_animatingItems(0),
    m_animatingScroll(0)
{
}


/******************************************************************
*                                                                 *
*  ListViewApp::~ListViewApp destructor                           *
*                                                                 *
*  Tear down resources                                            *
*                                                                 *
******************************************************************/

ListViewApp::~ListViewApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pTextFormat);
    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pBindContext);
    SafeRelease(&m_pBitmapAtlas);
}


/******************************************************************
*                                                                 *
*  ListViewApp::Initialize                                        *
*                                                                 *
*  Create application window and device-independent resources.    *
*                                                                 *
******************************************************************/

HRESULT ListViewApp::Initialize()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Create parent window
        {
            //register window class
            WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
            wcex.style         = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc   = ListViewApp::ParentWndProc;
            wcex.cbClsExtra    = 0;
            wcex.cbWndExtra    = sizeof(LONG_PTR);
            wcex.hInstance     = HINST_THISCOMPONENT;
            wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground = NULL;
            wcex.lpszMenuName  = NULL;
            wcex.lpszClassName = L"DemoAppWindow";

            RegisterClassEx(&wcex);

            // Create the application window.
            //
            // Because the CreateWindow function takes its size in pixels, we
            // obtain the system DPI and use it to scale the window size.
            FLOAT dpiX;
            FLOAT dpiY;
            m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

            m_parentHwnd = CreateWindow(
                L"DemoAppWindow",
                L"D2D ListView",
                WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                static_cast<UINT>(ceil(640.0f * dpiX / 96.0f)),
                static_cast<UINT>(ceil(480.0f * dpiY / 96.0f)),
                NULL,
                NULL,
                HINST_THISCOMPONENT,
                this);

            hr = m_parentHwnd ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                ShowWindow(m_parentHwnd, SW_SHOWNORMAL);
                UpdateWindow(m_parentHwnd);
            }
        }

        // Create child window (for D2D content)
        {
            //register window class
            WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
            wcex.style         = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc   = ListViewApp::ChildWndProc;
            wcex.cbClsExtra    = 0;
            wcex.cbWndExtra    = sizeof(LONG_PTR);
            wcex.hInstance     = HINST_THISCOMPONENT;
            wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground = NULL;
            wcex.lpszMenuName  = NULL;
            wcex.lpszClassName = L"D2DListViewApp";

            RegisterClassEx(&wcex);

            D2D1_SIZE_U d2dWindowSize = CalculateD2DWindowSize();

            //create window
            m_d2dHwnd = CreateWindow(
                L"D2DListViewApp",
                L"",
                WS_CHILDWINDOW | WS_VISIBLE,
                0,
                0,
                d2dWindowSize.width,
                d2dWindowSize.height,
                m_parentHwnd,
                NULL,
                HINST_THISCOMPONENT,
                this
                );

            hr = m_d2dHwnd ? S_OK : E_FAIL;
        }
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  ListViewApp::CalculateD2DWindowSize                            *
*                                                                 *
*  Determine the size of the D2D child window.                    *
*                                                                 *
******************************************************************/

D2D1_SIZE_U ListViewApp::CalculateD2DWindowSize()
{
    RECT rc;
    GetClientRect(m_parentHwnd, &rc);

    D2D1_SIZE_U d2dWindowSize = {0};
    d2dWindowSize.width = rc.right;
    d2dWindowSize.height = rc.bottom;

    return d2dWindowSize;
}


/******************************************************************
*                                                                 *
*  ListViewApp::CreateDeviceIndependentResources                  *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app. These resources include the D2D,          *
*  DWrite, and WIC factories; and a DWrite Text Format object     *
*  (used for identifying particular font characteristics) and     *
*  a D2D geometry.                                                *
*                                                                 *
******************************************************************/

HRESULT ListViewApp::CreateDeviceIndependentResources()
{
    static const WCHAR msc_fontName[] = L"Calibri";
    static const FLOAT msc_fontSize = 20;
    HRESULT hr = S_OK;

    //create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (SUCCEEDED(hr))
    {
        //create WIC factory
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
        //create DWrite factory
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
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
            DWRITE_FONT_WEIGHT_THIN,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            msc_fontSize,
            L"", //locale
            &m_pTextFormat
            );
    }
    if (SUCCEEDED(hr))
    {
        //center the text both horizontally and vertically.
        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        IDWriteInlineObject *pEllipsis = NULL;

        hr = m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTextFormat, &pEllipsis);

        if (SUCCEEDED(hr))
        {
            static const DWRITE_TRIMMING sc_trimming =
            {
                DWRITE_TRIMMING_GRANULARITY_CHARACTER,
                0,
                0
            };

            // Set the trimming back on the trimming format object.
            hr = m_pTextFormat->SetTrimming(&sc_trimming, pEllipsis);

            pEllipsis->Release();
        }
    }
    if (SUCCEEDED(hr))
    {
        // Set the text format not to allow word wrapping.
        hr = m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  ListViewApp::CreateDeviceResources                             *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

HRESULT ListViewApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRT)
    {
        RECT rc;
        GetClientRect(m_d2dHwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        //create a D2D render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_d2dHwnd, size),
            &m_pRT
            );
        if (SUCCEEDED(hr))
        {
            //create a black brush
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBlackBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pRT->CreateBitmap(
                D2D1::SizeU(msc_atlasWidth, msc_atlasHeight),
                D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
                &m_pBitmapAtlas
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = CreateBindCtx(0, &m_pBindContext);
        }
        if (SUCCEEDED(hr))
        {
            hr = LoadDirectory();
        }
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  ListViewApp::DiscardDeviceResources                            *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a D3D device is lost                                      *
*                                                                 *
******************************************************************/

void ListViewApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBitmapAtlas);
    SafeRelease(&m_pBlackBrush);
}


/******************************************************************
*                                                                 *
*  ListViewApp::RunMessageLoop                                    *
*                                                                 *
*  Main window message loop                                       *
*                                                                 *
******************************************************************/

void ListViewApp::RunMessageLoop()
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
*  ListViewApp::LoadDirectory                                     *
*                                                                 *
*  Load item info from files in the current directory.            *
*  Thumbnails/Icons are also loaded into the atlas and their      *
*  location is stored within each ItemInfo object.                *
*                                                                 *
******************************************************************/

HRESULT ListViewApp::LoadDirectory()
{
    HRESULT hr = S_OK;

    // Locals that need to be cleaned up before we exit
    HDC memoryDC = CreateCompatibleDC(NULL);
    HBITMAP iconImage = NULL;
    WCHAR *wszAbsolutePath = NULL;
    BYTE *pBits = NULL;
    HANDLE directoryTraversalHandle = INVALID_HANDLE_VALUE;
    IShellItemImageFactory *pShellItemImageFactory = NULL;

    // Other locals
    static const UINT sc_bitsArraySize = msc_iconSize * msc_iconSize * 4; // 4 bytes per pixel in BGRA format.
    UINT absolutePathArraySize = 0;
    UINT currentX = 0;
    UINT currentY = 0;
    UINT i = 0;

    if (SUCCEEDED(hr))
    {
        pBits = new (std::nothrow) BYTE[sc_bitsArraySize];
        hr = pBits ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        // We have a static array of ItemInfo objects, so we can only load
        // msc_maxItemInfos ItemInfos.
        while (i < msc_maxItemInfos)
        {
            // We always load files from the current directory. We do navigation
            // by changing the current directory. The first time through the
            // while loop directoryTraversalHandle will be equal to
            // INVALID_HANDLE_VALUE and so we'll call FindFirstFile. On
            // subsequent interations we'll call FindNextFile to find other
            // items in the current directory.
            WIN32_FIND_DATA findFileData = {0};
            if (directoryTraversalHandle == INVALID_HANDLE_VALUE)
            {
                directoryTraversalHandle = FindFirstFile(L".\\*", &findFileData);

                if (directoryTraversalHandle == INVALID_HANDLE_VALUE)
                {
                    if (GetLastError() != ERROR_FILE_NOT_FOUND)
                    {
                        hr = E_FAIL;
                    }

                    break;
                }
            }
            else
            {
                if (!FindNextFile(directoryTraversalHandle, &findFileData))
                {
                    if (GetLastError() != ERROR_NO_MORE_FILES)
                    {
                        hr = E_FAIL;
                    }

                    break;
                }
            }

            m_pFiles[i].placement.left = currentX;
            m_pFiles[i].placement.top = currentY;

            //
            // Increment bitmap atlas position here so that we notice if we
            // don't have enough room for any more icons.
            //
            currentX += msc_iconSize;

            if (currentX + msc_iconSize > msc_atlasWidth)
            {
                currentX = 0;
                currentY += msc_iconSize;
            }

            if (currentY + msc_iconSize > msc_atlasHeight)
            {
                // Exceeded atlas size
                // We break without any error so that the contents up until this
                // point will be shown.
                break;
            }

            //
            // Determine the size of array needed to store the full path name.
            // We need the full path name to call SHCreateItemFromParsingName.
            //
            UINT requiredLength = GetFullPathName(findFileData.cFileName, 0, NULL, NULL);
            if (requiredLength == 0)
            {
                hr = E_FAIL;
                break;
            }

            //
            // Allocate a bigger buffer if necessary.
            //
            if (requiredLength > absolutePathArraySize)
            {
                delete[] wszAbsolutePath;
                wszAbsolutePath = new (std::nothrow) WCHAR[requiredLength];
                if (wszAbsolutePath == NULL)
                {
                    hr = E_FAIL;
                    break;
                }
                absolutePathArraySize = requiredLength;
            }

            if (!GetFullPathName(findFileData.cFileName, requiredLength, wszAbsolutePath, NULL))
            {
                hr = E_FAIL;
                break;
            }

            // Create an IShellItemImageFactory for the current directory item
            // so that we can get a icon/thumbnail for it.
            SafeRelease(&pShellItemImageFactory);
            hr = SHCreateItemFromParsingName(
                    wszAbsolutePath,
                    m_pBindContext,
                    IID_PPV_ARGS(&pShellItemImageFactory)
                    );
            if (FAILED(hr))
            {
                break;
            }

            SIZE iconSize;
            iconSize.cx = msc_iconSize;
            iconSize.cy = msc_iconSize;

            // If iconImage isn't NULL that means we're looping around. We call
            // DeleteObject to avoid leaking the HBITMAP.
            if (iconImage != NULL)
            {
                DeleteObject(iconImage);
                iconImage = NULL;
            }

            // Get the icon/thumbnail for the current directory item in HBITMAP
            // form.
            // In the interests of brevity, this sample calls GetImage from the
            // UI thread. However this function can be time consuming, so a real
            // application should call GetImage from a separate thread, showing
            // a placeholder icon until the icon has been loaded.
            hr = pShellItemImageFactory->GetImage(iconSize, 0x0, &iconImage);
            if (FAILED(hr))
            {
                break;
            }

            BITMAPINFO bi = {0};
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

            // Get the bitmap info header.
            if (0 == GetDIBits(
                        memoryDC,   // hdc
                        iconImage,  // hbmp
                        0,          // uStartScan
                        0,          // cScanLines
                        NULL,       // lpvBits
                        &bi,
                        DIB_RGB_COLORS
                        ))
            {
                hr = E_FAIL;
                break;
            }

            // Positive bitmap info header height means bottom-up bitmaps. We
            // always use top-down bitmaps, so we set the height negative.
            if (bi.bmiHeader.biHeight > 0)
            {
                bi.bmiHeader.biHeight = -bi.bmiHeader.biHeight;
            }

            // If we happen to find an icon that's too big, skip over this item.
            if (   (-bi.bmiHeader.biHeight > msc_iconSize)
                || (bi.bmiHeader.biWidth > msc_iconSize)
                || (bi.bmiHeader.biSizeImage > sc_bitsArraySize))
            {
                continue;
            }

            m_pFiles[i].isDirectory = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            // Now that we know the size of the icon/thumbnail we can initialize
            // the rest of placement rectangle. We avoid using currentX/currentY
            // since we've already incremented those values in anticipation of
            // the next iteration of the loop.
            m_pFiles[i].placement.right = m_pFiles[i].placement.left + bi.bmiHeader.biWidth;
            m_pFiles[i].placement.bottom = m_pFiles[i].placement.top + -bi.bmiHeader.biHeight;

            // Now we copy the bitmap bits into a buffer.
            if (0 == GetDIBits(
                        memoryDC,
                        iconImage,
                        0,
                        -bi.bmiHeader.biHeight,
                        pBits,
                        &bi,
                        DIB_RGB_COLORS))
            {
                hr = E_FAIL;
                break;
            }

            // Now we copy the buffer into video memory.
            hr = m_pBitmapAtlas->CopyFromMemory(
                    &m_pFiles[i].placement,
                    pBits,
                    bi.bmiHeader.biSizeImage / -bi.bmiHeader.biHeight
                    );

            if (FAILED(hr))
            {
                break;
            }

            StringCchCopy(m_pFiles[i].szFilename, MAX_PATH, findFileData.cFileName);

            // Set the previous position to 0 so that the items animate
            // downwards when they are first shown.
            m_pFiles[i].previousPosition = 0.0f;
            m_pFiles[i].currentPosition = static_cast<FLOAT>(i * (msc_iconSize + msc_lineSpacing));
            i++;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_numItemInfos = i;

        //
        // The total size of our document.
        //
        m_scrollRange = msc_iconSize * m_numItemInfos + msc_lineSpacing * (m_numItemInfos - 1);

        SCROLLINFO si = {0};
        si.cbSize = sizeof(si);
        si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin = 0;
        si.nMax = m_scrollRange;
        si.nPage = static_cast<UINT>(m_pRT->GetSize().height);
        si.nPos = 0;
        SetScrollInfo(m_parentHwnd, SB_VERT, &si, TRUE);

        //
        // Animate the item positions into place.
        //
        m_animatingItems = msc_totalAnimatingItemFrames;

        //
        // Set the scroll to zero, don't animate.
        //
        m_animatingScroll = 0;
        m_currentScrollPos = 0;
        m_previousScrollPos = 0;
    }


    //
    // Clean up locals.
    //

    SafeRelease(&pShellItemImageFactory);

    delete[] pBits;
    delete[] wszAbsolutePath;

    if (directoryTraversalHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(directoryTraversalHandle);
    }

    if (memoryDC != NULL)
    {
        DeleteDC(memoryDC);
    }

    if (iconImage != NULL)
    {
        DeleteObject(iconImage);
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  ListViewApp::OnRender                                          *
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

HRESULT ListViewApp::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        // We animate scrolling to achieve a smooth scrolling effect.
        // GetInterpolatedScrollPosition() returns the scroll position
        // for the current frame.
        FLOAT interpolatedScroll = GetInterpolatedScrollPosition();

        m_pRT->BeginDraw();

        // Displaying the correctly scrolled view is as simple as setting the
        // transform to translate by the current scroll amount.
        m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(0, -interpolatedScroll));

        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = m_pRT->GetSize();

        FLOAT interpolationFactor = GetAnimatingItemInterpolationFactor();

        for (UINT i = 0; i < m_numItemInfos; i++)
        {
            Assert(m_pFiles[i].szFilename[0] != L'\0');

            // We animate item position changes. The interpolation factor is the
            // a ratio between 0 and 1 used to interpolate between the previous
            // position and the current position. The position that we draw for
            // this frame is somewhere between the two.
            FLOAT interpolatedPosition = GetFancyAccelerationInterpolatedValue(
                interpolationFactor,
                m_pFiles[i].previousPosition,
                m_pFiles[i].currentPosition
                );

            // We do a quick check to see if the items we are drawing will be in
            // the visible region. If they are not, we don't bother issues the
            // draw commands. This is a substantial perf win.
            FLOAT topOfIcon = interpolatedPosition;
            FLOAT bottomOfIcon = interpolatedPosition + msc_iconSize;

            if (   bottomOfIcon < interpolatedScroll
                || topOfIcon > interpolatedScroll + m_pRT->GetSize().height)
            {
                // Some further items could be in the visible region. Continue
                // the loop so that they will be drawn.
                continue;
            }

            // When the items change position we draw them mostly transparent
            // and then gradually make them more opaque as they get closer to
            // their final positions. This function was chosen after a bit of
            // experimentation and I thought it looked nice.
            FLOAT opacity = max(0.2f, interpolationFactor * interpolationFactor);

            // The icon is stored in the image atlas. We reference it's position
            // in the atlas and it's destination on the screen.
            m_pRT->DrawBitmap(
                m_pBitmapAtlas,
                D2D1::RectF(
                    0,
                    interpolatedPosition,
                    static_cast<FLOAT>(msc_iconSize),
                    interpolatedPosition + static_cast<FLOAT>(msc_iconSize)),
                opacity,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                D2D1::RectF(
                    (FLOAT)m_pFiles[i].placement.left,
                    (FLOAT)m_pFiles[i].placement.top,
                    (FLOAT)m_pFiles[i].placement.right,
                    (FLOAT)m_pFiles[i].placement.bottom)
                );

            // Draw the filename. For brevity we just use DrawText. A real
            // application should consider caching the TextLayout object during
            // animations to reduce CPU cost.
            m_pBlackBrush->SetOpacity(opacity);
            m_pRT->DrawText(
                m_pFiles[i].szFilename,
                static_cast<UINT>(wcsnlen(m_pFiles[i].szFilename, ARRAYSIZE(m_pFiles[i].szFilename))),
                m_pTextFormat,
                D2D1::RectF(
                    msc_iconSize + msc_lineSpacing,
                    interpolatedPosition,
                    rtSize.width,
                    interpolatedPosition + static_cast<FLOAT>(msc_iconSize)),
                m_pBlackBrush
                );
        }

        hr = m_pRT->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
            hr = S_OK;
        }
    }

    // Advance the position of the current item animation.
    if (m_animatingItems > 0)
    {
        --m_animatingItems;
        if (m_animatingItems == 0)
        {
            for (UINT i = 0; i < m_numItemInfos; i++)
            {
                m_pFiles[i].previousPosition = m_pFiles[i].currentPosition;
            }
        }

        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }

    // Advance the position of the current scroll animation
    if (m_animatingScroll > 0)
    {
        --m_animatingScroll;
        if (m_animatingScroll == 0)
        {
            m_previousScrollPos = m_currentScrollPos;
        }

        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }

    return hr;
}


/******************************************************************
*                                                                 *
*  ListViewApp::GetFancyAccelerationInterpolatedValue             *
*                                                                 *
*  Do a fancy interpolation between two points.                   *
*                                                                 *
******************************************************************/

FLOAT ListViewApp::GetFancyAccelerationInterpolatedValue(FLOAT linearFactor, FLOAT p1, FLOAT p2)
{
    Assert(linearFactor >= 0.0f && linearFactor <= 1.0f);

    // Don't overshoot by more than the icon size.
    FLOAT apex = 0.0f;
    if (fabs(p1 - p2) > 0.01f)
    {
        apex = min(0.10f * fabs(p1 - p2), msc_iconSize) / fabs(p1 - p2);
    }
    else
    {
        apex = 0.0f;
    }

    // Stretch so that the initial overshoot (occurring 33% of the way along) is
    // 70% of the animation.
    FLOAT rearrangedDomain = 0.0f;
    if (linearFactor < 0.7f)
    {
        rearrangedDomain = (linearFactor / 0.7f) / 3.0f;
    }
    else
    {
        rearrangedDomain = ((linearFactor - 0.7f) / 0.3f) * (2.0f / 3.0f) + 1.0f/3.0f;
    }

    //
    // We will use sin to approximate the curve. Since we want to start at a
    // minimum value, we start at -PI/2. Since we want to finish at the second
    // max. We stretch the interval [0..1] to [-PI/2 .. 5PI/2].
    //

    FLOAT stretchedDomain = rearrangedDomain * 3.0f * static_cast<FLOAT>(M_PI);
    FLOAT translatedDomain = stretchedDomain - static_cast<FLOAT>(M_PI_2);

    FLOAT fancyFactor = sin(translatedDomain) + 1.0f; // Now between 0 and 2

    //
    // Before the first max, we want the bounds to go from 0 to 1+apex
    //
    if (translatedDomain < static_cast<FLOAT>(M_PI_2))
    {
        fancyFactor = fancyFactor * (1.0f+apex) / 2.0f; // Now between 0 and 1+apex
    }
    //
    // After the first max, we want to ease the bounds down so that when
    // translatedDomain is 5PI/2, fancyFactor is 1.0f. We also want the bounce
    // to be small, so we reduce the magnitude of the oscillation.
    //
    else
    {
        //
        // When we want the bounce (the undershoot after reaching the apex), to
        // be reach 1.0f - apex / 2.0f at a minimum.
        //
        FLOAT oscillationMin = (1.0f - apex / 2.0f);

        //
        // We want the max to start out at 1.0f + apex (so that we are
        // continuous) and finish at 1.0f (the final position). We square our
        // interpolation factor to stretch the bounce and compress the
        // correction. Since the correction is a smaller distance, this looks
        // better. Another benefit is that it prevents us from overshooting 1.0f
        // during the correction phase.
        //
        FLOAT interpolationFactor = (translatedDomain - static_cast<FLOAT>(M_PI_2)) / (2.0f * static_cast<FLOAT>(M_PI));
        interpolationFactor *= interpolationFactor;
        FLOAT oscillationMax = 1.0f * interpolationFactor + (1.0f + apex) * (1.0f - interpolationFactor);

        Assert(oscillationMax >= oscillationMin);

        FLOAT oscillationMidPoint = (oscillationMin + oscillationMax) / 2.0f;

        FLOAT oscillationMagnitude = oscillationMax - oscillationMin;

        // Oscillate around the midpoint
        fancyFactor = (fancyFactor / 2.0f - 0.5f) * oscillationMagnitude + oscillationMidPoint;
    }

    return p2 * fancyFactor + p1 * (1.0f - fancyFactor);
}


/******************************************************************
*                                                                 *
*  ListViewApp::GetAnimatingItemInterpolationFactor               *
*                                                                 *
*  Return the interpolation factor for a linear interpolation     *
*  for the current item animation for the the current frame       *
*                                                                 *
******************************************************************/

FLOAT ListViewApp::GetAnimatingItemInterpolationFactor()
{
    return static_cast<FLOAT>(msc_totalAnimatingItemFrames - m_animatingItems) / msc_totalAnimatingItemFrames;
}


/******************************************************************
*                                                                 *
*  ListViewApp::GetAnimatingScrollInterpolationFactor             *
*                                                                 *
*  Return the interpolation factor for a linear interpolation     *
*  for the current scroll animation for the the current frame     *
*                                                                 *
******************************************************************/

FLOAT ListViewApp::GetAnimatingScrollInterpolationFactor()
{
    return static_cast<FLOAT>(msc_totalAnimatingScrollFrames - m_animatingScroll) / msc_totalAnimatingScrollFrames;
}


/******************************************************************
*                                                                 *
*  ListViewApp::GetInterpolatedScrollPosition                     *
*                                                                 *
*  Return the scroll position for the current frame               *
*                                                                 *
******************************************************************/

FLOAT ListViewApp::GetInterpolatedScrollPosition()
{
    FLOAT interpolationFactor = GetAnimatingScrollInterpolationFactor();

    return m_currentScrollPos * interpolationFactor + m_previousScrollPos * (1.0f - interpolationFactor);
}


/******************************************************************
*                                                                 *
*  ListViewApp::OnResize                                          *
*                                                                 *
*  If the application receives a WM_SIZE message, this method     *
*  resizes the render target appropriately.                       *
*                                                                 *
******************************************************************/

void ListViewApp::OnResize()
{
    if (m_pRT)
    {
        D2D1_SIZE_U size = CalculateD2DWindowSize();
        MoveWindow(m_d2dHwnd, 0, 0, size.width, size.height, FALSE);

        // Note: This method can fail, but it's okay to ignore the
        // error here -- it will be repeated on the next call to
        // EndDraw.
        m_pRT->Resize(size);

        m_scrollRange = (msc_lineSpacing + msc_iconSize) * m_numItemInfos - msc_lineSpacing;

        SCROLLINFO si = {0};
        si.cbSize = sizeof(si);
        si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE;
        si.nMin = 0;
        si.nMax = m_scrollRange;
        si.nPage = size.height;
        SetScrollInfo(m_parentHwnd, SB_VERT, &si, TRUE);

        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }
}


/******************************************************************
*                                                                 *
*  ListViewApp::CompareAToZ (static)                              *
*                                                                 *
*  A comparator function for sorting ItemInfos alphabetically     *
*                                                                 *
******************************************************************/

int ListViewApp::CompareAToZ(const void *a, const void *b)
{
    const ListViewApp::ItemInfo *pA = reinterpret_cast<const ListViewApp::ItemInfo *>(a);
    const ListViewApp::ItemInfo *pB = reinterpret_cast<const ListViewApp::ItemInfo *>(b);

    return wcscmp(pA->szFilename, pB->szFilename);
}


/******************************************************************
*                                                                 *
*  ListViewApp::CompareZToA (static)                              *
*                                                                 *
*  A comparator function for sorting ItemInfos in reverse         *
*  alphabetical order.                                            *
*                                                                 *
******************************************************************/

int ListViewApp::CompareZToA(const void *a, const void *b)
{
    const ListViewApp::ItemInfo *pA = reinterpret_cast<const ListViewApp::ItemInfo *>(a);
    const ListViewApp::ItemInfo *pB = reinterpret_cast<const ListViewApp::ItemInfo *>(b);

    return wcscmp(pB->szFilename, pA->szFilename);
}


/******************************************************************
*                                                                 *
*  ListViewApp::CompareDirFirstAToZ (static)                      *
*                                                                 *
*  A comparator function for sorting ItemInfos in alphabetical    *
*  order, with all directories before all other files.            *
*                                                                 *
******************************************************************/

int ListViewApp::CompareDirFirstAToZ(const void *a, const void *b)
{
    const ListViewApp::ItemInfo *pA = reinterpret_cast<const ListViewApp::ItemInfo *>(a);
    const ListViewApp::ItemInfo *pB = reinterpret_cast<const ListViewApp::ItemInfo *>(b);

    if (pA->isDirectory && !pB->isDirectory)
    {
        return -1;
    }
    else if (!pA->isDirectory && pB->isDirectory)
    {
        return 1;
    }
    else
    {
        return wcscmp(pA->szFilename, pB->szFilename);
    }
}


/******************************************************************
*                                                                 *
*  ListViewApp::OnChar                                            *
*                                                                 *
*  Called when the app receives a WM_CHAR message (which happens  *
*  when a key is pressed).                                        *
*                                                                 *
******************************************************************/

void ListViewApp::OnChar(SHORT aChar)
{
    // We only do stuff for 'a', 'z', or 'd'
    if (aChar == 'a' || aChar == 'z' || aChar == 'd')
    {
        typedef int (__cdecl * Comparator) (const void *, const void *);

        Comparator comparator = NULL;

        // 'a' means alphabetical sort
        if (aChar == 'a')
        {
            comparator = ListViewApp::CompareAToZ;
        }
        // 'z' means reverse alphabetical sort
        else if (aChar == 'z')
        {
            comparator = ListViewApp::CompareZToA;
        }
        // 'd' means alphabetical sort, directories first
        else
        {
            comparator = ListViewApp::CompareDirFirstAToZ;
        }

        // Freeze file position to the current interpolated position so that
        // when we animate to the new positions, the items don't jump back to
        // their previous position momentarily.
        for (UINT i = 0; i < m_numItemInfos; i++)
        {
            FLOAT interpolationFactor = GetAnimatingItemInterpolationFactor();

            m_pFiles[i].previousPosition = GetFancyAccelerationInterpolatedValue(
                interpolationFactor,
                m_pFiles[i].previousPosition,
                m_pFiles[i].currentPosition
                );
        }

        // Apply the new sort.
        qsort(m_pFiles, m_numItemInfos, sizeof(ListViewApp::ItemInfo), comparator);

        // Set the new positions based up on the position of each item within
        // the sorted array.
        for (UINT i = 0; i < m_numItemInfos; i++)
        {
            m_pFiles[i].currentPosition = static_cast<FLOAT>(i * (msc_iconSize + msc_lineSpacing));
        }

        // Animate the items to their new positions.
        m_animatingItems = msc_totalAnimatingItemFrames;
        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }
}


/******************************************************************
*                                                                 *
*  ListViewApp::GetScrolledDIPositionFromPixelPosition            *
*                                                                 *
*  Translate a pixel position to a position within our document,  *
*  taking scrolling into account.                                 *
*                                                                 *
******************************************************************/

D2D1_POINT_2F ListViewApp::GetScrolledDIPositionFromPixelPosition(D2D1_POINT_2U pixelPosition)
{
    D2D1_POINT_2F dpi = {0};
    m_pRT->GetDpi(&dpi.x, &dpi.y);

    return D2D1::Point2F(pixelPosition.x * 96 / dpi.x, pixelPosition.y * 96 / dpi.y + GetInterpolatedScrollPosition());
}


/******************************************************************
*                                                                 *
*  ListViewApp::ParentWndProc                                     *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

LRESULT CALLBACK ListViewApp::ParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ListViewApp *pListViewApp = (ListViewApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pListViewApp)
            );

        result = 1;
    }
    else
    {
        ListViewApp *pListViewApp = reinterpret_cast<ListViewApp *>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                ));

        bool wasHandled = false;

        if (pListViewApp)
        {
            switch (message)
            {
            case WM_SIZE:
                {
                    pListViewApp->OnResize();
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_VSCROLL:
                {
                    pListViewApp->OnVScroll(wParam, lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_MOUSEWHEEL:
                {
                    pListViewApp->OnMouseWheel(wParam, lParam);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_CHAR:
                {
                    pListViewApp->OnChar(static_cast<SHORT>(wParam));
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
*  ListViewApp::ChildWndProc                                      *
*                                                                 *
*  Window message handler for the Child D2D window                *
*                                                                 *
******************************************************************/

LRESULT CALLBACK ListViewApp::ChildWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ListViewApp *pListViewApp = (ListViewApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pListViewApp)
            );

        return 1;
    }

    ListViewApp *pListViewApp = reinterpret_cast<ListViewApp *>(static_cast<LONG_PTR>(
        ::GetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA
            )));

    if (pListViewApp)
    {
        switch (message)
        {
        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);

                pListViewApp->OnRender();
                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_LBUTTONDOWN:
            {
                D2D1_POINT_2F diPosition = pListViewApp->GetScrolledDIPositionFromPixelPosition(D2D1::Point2U(LOWORD(lParam), HIWORD(lParam)));

                pListViewApp->OnLeftButtonDown(diPosition);
            }
            return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


/******************************************************************
*                                                                 *
*  ListViewApp::OnLeftButtonDown                                  *
*                                                                 *
*  Called when the left mouse button is pressed inside the child  *
*  D2D window                                                     *
*                                                                 *
******************************************************************/

void ListViewApp::OnLeftButtonDown(D2D1_POINT_2F diPosition)
{
    INT index = static_cast<INT>(diPosition.y / (msc_iconSize + msc_lineSpacing));
    if (index >= 0 && index < static_cast<INT>(m_numItemInfos))
    {
        // Only process the click if the item isn't animating
        if (m_pFiles[index].currentPosition == m_pFiles[index].previousPosition)
        {
            if (diPosition.y < m_pFiles[index].currentPosition + msc_iconSize)
            {
                if (SetCurrentDirectory(m_pFiles[index].szFilename))
                {
                    if (SUCCEEDED(LoadDirectory()))
                    {
                        InvalidateRect(m_d2dHwnd, NULL, FALSE);
                    }
                }
            }
        }
    }
}

/******************************************************************
*                                                                 *
*  ListViewApp::OnLeftButtonDown                                  *
*                                                                 *
*  Called when the mouse wheel is moved.                          *
*                                                                 *
******************************************************************/

void ListViewApp::OnMouseWheel(WPARAM wParam, LPARAM /* lParam */)
{
    m_previousScrollPos = static_cast<INT>(GetInterpolatedScrollPosition());
    m_currentScrollPos -= GET_WHEEL_DELTA_WPARAM(wParam);
    m_currentScrollPos = max(0, min(m_currentScrollPos, static_cast<INT>(m_scrollRange) - static_cast<INT>(m_pRT->GetSize().height)));

    SCROLLINFO si = {0};
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    BOOL ret = GetScrollInfo(m_parentHwnd, SB_VERT, &si);
    if (!ret)
    {
        Assert(ret);
        return;
    }

    if (m_currentScrollPos != si.nPos)
    {
        si.nPos = m_currentScrollPos;
        SetScrollInfo(m_parentHwnd, SB_VERT, &si, TRUE);

        m_animatingScroll = msc_totalAnimatingScrollFrames;
        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }
}


/******************************************************************
*                                                                 *
*  ListViewApp::OnVScroll                                         *
*                                                                 *
*  Called when a WM_VSCROLL message is sent.                      *
*                                                                 *
******************************************************************/

void ListViewApp::OnVScroll(WPARAM wParam, LPARAM /* lParam */)
{
    INT newScrollPos = m_currentScrollPos;

    switch (LOWORD(wParam))
    {
    case SB_LINEUP:
        newScrollPos -= 1;
        break;

    case SB_LINEDOWN:
        newScrollPos += 1;
        break;

    case SB_PAGEUP:
        newScrollPos -= static_cast<UINT>(m_pRT->GetSize().height);
        break;

    case SB_PAGEDOWN:
        newScrollPos += static_cast<UINT>(m_pRT->GetSize().height);
        break;

    case SB_THUMBTRACK:
        {
            SCROLLINFO si = {0};
            si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
            BOOL ret = GetScrollInfo(m_parentHwnd, SB_VERT, &si);
            if (!ret)
            {
                Assert(ret);
                return;
            }
            newScrollPos = si.nTrackPos;
        }
        break;

    default:
        break;
    }

    newScrollPos = max(0, min(newScrollPos, static_cast<INT>(m_scrollRange)));

    m_previousScrollPos = static_cast<INT>(GetInterpolatedScrollPosition());

    m_currentScrollPos = newScrollPos;

    SCROLLINFO si = {0};
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    BOOL ret = GetScrollInfo(m_parentHwnd, SB_VERT, &si);
    if (!ret)
    {
        Assert(ret);
        return;
    }

    if (m_currentScrollPos != si.nPos)
    {
        si.nPos = m_currentScrollPos;
        SetScrollInfo(m_parentHwnd, SB_VERT, &si, TRUE);

        m_animatingScroll = msc_totalAnimatingScrollFrames;
        InvalidateRect(m_d2dHwnd, NULL, FALSE);
    }
}
