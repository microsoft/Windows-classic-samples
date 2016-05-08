// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "MainWindow.h"
#include "ManagerEventHandler.h"
#include "UIAnimationSample.h"

#include <strsafe.h>

CMainWindow::CMainWindow() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBackgroundBrush(NULL),
    m_pAnimationManager(NULL),
    m_pAnimationTimer(NULL),
    m_pTransitionLibrary(NULL),
    m_pTransitionFactory(NULL),
    m_uThumbCount(0),
    m_thumbs(NULL),
    m_pLayoutManager(NULL)
{
}

CMainWindow::~CMainWindow()
{   
    // Layout Manager
    
    delete m_pLayoutManager;

    // Thumbnails
    
    delete [] m_thumbs;

    // Animation

    SafeRelease(&m_pAnimationManager);
    SafeRelease(&m_pAnimationTimer);
    SafeRelease(&m_pTransitionLibrary);
    SafeRelease(&m_pTransitionFactory);

    // D2D

    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBackgroundBrush);
}

// Creates the CMainWindow window and initializes 
// device-independent resources

HRESULT CMainWindow::Initialize(
    HINSTANCE hInstance
    )
{
    // Client area dimensions, in inches
    const FLOAT CLIENT_WIDTH = 7.0f;
    const FLOAT CLIENT_HEIGHT = 5.0f; 

    HRESULT hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register the window class

        WNDCLASSEX wcex = {sizeof(wcex)};
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = CMainWindow::WndProc;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = hInstance;
        wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = L"WAMMainWindow";
        
        RegisterClassEx(&wcex);
        
        // Because CreateWindow function takes its size in pixels, 
        // obtain the DPI and use it to calculate the window size.

        // The D2D factory returns the current system DPI. This is
        // also the value it will use to create its own windows.

        FLOAT dpiX, dpiY;
        m_pD2DFactory->GetDesktopDpi(
            &dpiX,
            &dpiY
            );

        // Create the CMainWindow window

        m_hwnd = CreateWindow(
            wcex.lpszClassName,
            L"Windows Animation - Custom Interpolator Demo",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(dpiX * CLIENT_WIDTH),
            static_cast<UINT>(dpiY * CLIENT_HEIGHT),
            NULL,
            NULL,
            hInstance,
            this
            );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            // Initialize Animation
            
            hr = InitializeAnimation();
            if (SUCCEEDED(hr))
            {
                // Display the window
                
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);
            }
        }
    }

    return hr;
}

// Invalidates the client area for redrawing

HRESULT CMainWindow::Invalidate()
{
    BOOL bResult = InvalidateRect(
        m_hwnd,
        NULL,
        FALSE
        );

    HRESULT hr = bResult ? S_OK : E_FAIL;

    return hr;
}

// Creates and initializes the main animation components

HRESULT CMainWindow::InitializeAnimation()
{
    // Create Animation Manager

    HRESULT hr = CoCreateInstance(
        CLSID_UIAnimationManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pAnimationManager)
        );
    if (SUCCEEDED(hr))
    {
        // Create Animation Timer

        hr = CoCreateInstance(
            CLSID_UIAnimationTimer,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pAnimationTimer)
            );
        if (SUCCEEDED(hr))
        {
            // Create Animation Transition Library

            hr = CoCreateInstance(
                CLSID_UIAnimationTransitionLibrary,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&m_pTransitionLibrary)
                );
            if (SUCCEEDED(hr))
            {
                // Create and set the ManagerEventHandler to start updating when animations are scheduled

                IUIAnimationManagerEventHandler *pManagerEventHandler;
                hr = CManagerEventHandler::CreateInstance(
                    this,
                    &pManagerEventHandler
                    );
                if (SUCCEEDED(hr))
                {
                    hr = m_pAnimationManager->SetManagerEventHandler(
                        pManagerEventHandler
                        );
                    pManagerEventHandler->Release();
                    if (SUCCEEDED(hr))
                    {
                        // Create the Transition Factory to wrap interpolators in transitions

                        hr = CoCreateInstance(
                            CLSID_UIAnimationTransitionFactory,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_PPV_ARGS(&m_pTransitionFactory)
                            );
                    }
                }
            }
        }
    }
    
    return hr;
}

// Creates resources which are not bound to any device.
// Their lifetime effectively extends for the duration of the app.
// These resources include the Direct2D factory.

HRESULT CMainWindow::CreateDeviceIndependentResources()
{
    // Create a Direct2D factory
    
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory
        );

    return hr;
}

// Creates resources which are bound to a particular Direct3D device.
// It's all centralized here, so the resources can be easily recreated
// in the event of Direct3D device loss (e.g. display change, remoting,
// removal of video card, etc). The resources will only be created
// if necessary.
             
HRESULT CMainWindow::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    
    if (m_pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(
            m_hwnd,
            &rc
            );

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );
 
        // Create a Direct2D render target

        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                m_hwnd,
                size
                ),
            &m_pRenderTarget
            );
        if (SUCCEEDED(hr))
        {
            // Create a gradient brush for the background

            static const D2D1_GRADIENT_STOP stops[] =
            {
                { 0.0f, { 0.75f, 0.75f, 0.75f, 1.0f } },
                { 1.0f, { 0.25f, 0.25f, 0.25f, 1.0f } },
            };

            ID2D1GradientStopCollection *pGradientStops;
            hr = m_pRenderTarget->CreateGradientStopCollection(
                stops,
                ARRAYSIZE(stops),
                &pGradientStops
                );
            if (SUCCEEDED(hr))
            {
                D2D1_SIZE_F sizeRenderTarget = m_pRenderTarget->GetSize();
                hr = m_pRenderTarget->CreateLinearGradientBrush(
                    D2D1::LinearGradientBrushProperties(
                        D2D1::Point2F(
                            sizeRenderTarget.width * 0.5f,
                            0.0f
                            ),
                        D2D1::Point2F(
                            sizeRenderTarget.width * 0.5f,
                            sizeRenderTarget.height * 0.5f
                            )
                        ),
                    D2D1::BrushProperties(
                        ),
                    pGradientStops,
                    &m_pBackgroundBrush
                    );
                if (SUCCEEDED(hr))
                {
                    // Create a brush for outlining the thumbnails

                    hr = m_pRenderTarget->CreateSolidColorBrush(
                        D2D1::ColorF(
                            D2D1::ColorF::White
                            ),
                        &m_pOutlineBrush
                        );
                    if (SUCCEEDED(hr))
                    {
                        m_pOutlineBrush->SetOpacity(
                            0.5f
                            );
                    }
                }
            }
        }
    }

    return hr;
}
                                                                                             
//  Discards device-specific resources that need to be recreated   
//  when a Direct3D device is lost                                      
                                                              
void CMainWindow::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBackgroundBrush);
    SafeRelease(&m_pOutlineBrush);
}

// Locates images in the Pictures library

HRESULT CMainWindow::FindImages()
{
    HRESULT hr = S_OK;
    
    if (m_thumbs == NULL)
    {
        // Walk the Pictures library

        IShellItem *pShellItemPicturesLibrary;
        hr = SHGetKnownFolderItem(
            FOLDERID_PicturesLibrary,
            KF_FLAG_CREATE,
            NULL,
            IID_PPV_ARGS(&pShellItemPicturesLibrary)
            );
        if (SUCCEEDED(hr))
        {
            INamespaceWalk *pNamespaceWalk;
            hr = CoCreateInstance(
                CLSID_NamespaceWalker,
                NULL,
                CLSCTX_INPROC,
                IID_PPV_ARGS(&pNamespaceWalk)
                );
            if (SUCCEEDED(hr))
            {
                hr = pNamespaceWalk->Walk(
                    pShellItemPicturesLibrary,
                    NSWF_NONE_IMPLIES_ALL,
                    1,
                    NULL
                    );
                if (SUCCEEDED(hr))
                {
                    // Retrieve the array of PIDLs gathered in the walk
                
                    UINT itemCount;
                    PIDLIST_ABSOLUTE *ppidls;
                    hr = pNamespaceWalk->GetIDArrayResult(
                        &itemCount,
                        &ppidls
                        );
                    if (SUCCEEDED(hr))
                    {
                        // Create the uninitialized thumbnails
                        
                        m_thumbs = new CThumbnail[itemCount];                 
                    
                        // Get the bitmap for each item and initialize the corresponding thumbnail object
                    
                        for (UINT i = 0; i < itemCount; i++)
                        {
                            IShellItem *pShellItem;
                            hr = SHCreateItemFromIDList(
                                ppidls[i],
                                IID_PPV_ARGS(&pShellItem)
                                );
                            if (SUCCEEDED(hr))
                            {
                                ID2D1Bitmap *pBitmap;
                                hr = DecodeImageFromThumbCache(
                                    pShellItem,
                                    &pBitmap
                                    );
                                if (SUCCEEDED(hr))
                                {
                                    D2D1_SIZE_F size = pBitmap->GetSize();

                                    hr = m_thumbs[m_uThumbCount].Initialize(
                                        pBitmap,
                                        m_pAnimationManager,
                                        m_pRenderTarget->GetSize().width * 0.5,
                                        -size.height * 0.5 - 1.0
                                        );
                                    if (SUCCEEDED(hr))
                                    {
                                        m_uThumbCount++;
                                    }
                                        
                                    pBitmap->Release();
                                }
                                
                                pShellItem->Release();
                            }
                        }
                        
                        // The calling function is responsible for freeing the PIDL array
                        
                        FreeIDListArray(ppidls, itemCount);
                        
                        if (SUCCEEDED(hr))
                        {
                            // Arrange the images when they are first loaded

                            m_pLayoutManager = new CLayoutManager();
                            hr = m_pLayoutManager->Initialize(
                                m_pAnimationManager,
                                m_pAnimationTimer,
                                m_pTransitionLibrary,
                                m_pTransitionFactory,
                                m_uThumbCount,
                                m_thumbs
                                );
                            if (SUCCEEDED(hr))
                            {
                                hr = m_pLayoutManager->Arrange(
                                    m_pRenderTarget->GetSize()
                                    );
                            }
                        }
                    }
                }
            
                pNamespaceWalk->Release();
            }
        
            pShellItemPicturesLibrary->Release();
        }
    }
    
    return hr;
}

// Retrieves a bitmap from the thumbnail cache and converts it to
// a D2D bitmap

HRESULT CMainWindow::DecodeImageFromThumbCache(
    IShellItem *pShellItem,
    ID2D1Bitmap **ppBitmap
    )
{
    const UINT THUMB_SIZE = 96;

    // Read the bitmap from the thumbnail cache

    IThumbnailCache *pThumbCache;
    HRESULT hr = CoCreateInstance(
        CLSID_LocalThumbnailCache,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pThumbCache)
        );
    if (SUCCEEDED(hr))
    {
        ISharedBitmap *pBitmap;
        hr = pThumbCache->GetThumbnail(
            pShellItem,
            THUMB_SIZE,
            WTS_SCALETOREQUESTEDSIZE,
            &pBitmap,
            NULL,
            NULL
            );
        if (SUCCEEDED(hr))
        {
            HBITMAP hBitmap;
            hr = pBitmap->GetSharedBitmap(
                &hBitmap
                );
            if (SUCCEEDED(hr))
            {
                // Create a WIC bitmap from the shared bitmap

                IWICImagingFactory *pFactory;
                hr = CoCreateInstance(
                    CLSID_WICImagingFactory,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&pFactory)
                    );
                if (SUCCEEDED(hr))
                {
                    IWICBitmap *pWICBitmap;
                    hr = pFactory->CreateBitmapFromHBITMAP(
                        hBitmap,
                        NULL,
                        WICBitmapIgnoreAlpha,
                        &pWICBitmap
                        );
                    if (SUCCEEDED(hr))
                    {
                        // Create a D2D bitmap from the WIC bitmap

                        hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
                            pWICBitmap,
                            NULL,
                            ppBitmap
                            );
                        pWICBitmap->Release();
                    }

                    pFactory->Release();
                }
            }
            
            pBitmap->Release();
        }
        
        pThumbCache->Release();
    }

    return hr;
}

// The window message handler

LRESULT CALLBACK CMainWindow::WndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    const int MESSAGE_PROCESSED = 0;

    if (uMsg == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        CMainWindow *pMainWindow = (CMainWindow *)pcs->lpCreateParams;

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pMainWindow)
            );

        return MESSAGE_PROCESSED;
    }

    CMainWindow *pMainWindow = reinterpret_cast<CMainWindow *>(static_cast<LONG_PTR>(
        GetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA
            )));

    if (pMainWindow != NULL)
    {
        switch (uMsg)
        {
        case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pMainWindow->OnResize(
                    width,
                    height
                    );
            }
            return MESSAGE_PROCESSED;

        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                BeginPaint(
                    hwnd,
                    &ps
                    );

                pMainWindow->OnPaint(
                    ps.hdc,
                    ps.rcPaint
                    );

                EndPaint(
                    hwnd,
                    &ps
                    );
            }
            return MESSAGE_PROCESSED;

        case WM_KEYDOWN:
            {
                UINT uVirtKey = wParam;
                   
                pMainWindow->OnKeyDown(
                    uVirtKey
                    );
            }
            return MESSAGE_PROCESSED;

        case WM_DESTROY:
            {
                pMainWindow->OnDestroy();
            
                PostQuitMessage(
                    0
                    );
            }
            return MESSAGE_PROCESSED;
        }
    }
    
    return DefWindowProc(
        hwnd,
        uMsg,
        wParam,
        lParam
        );
}
                                                        
//  Called whenever the client area needs to be drawn

HRESULT CMainWindow::OnPaint(
    HDC hdc,
    const RECT &rcPaint
    )
{
    // Update the animation manager with the current time

    UI_ANIMATION_SECONDS secondsNow;
    HRESULT hr = m_pAnimationTimer->GetTime(
        &secondsNow
        );
    if (SUCCEEDED(hr))
    {
        hr = m_pAnimationManager->Update(
            secondsNow
            );
        if (SUCCEEDED(hr))
        {
            // Read the values of the animation variables and draw the client area
        
            hr = DrawClientArea();
            if (SUCCEEDED(hr))
            {
                // Continue redrawing the client area as long as there are animations scheduled
                
                UI_ANIMATION_MANAGER_STATUS status;
                hr = m_pAnimationManager->GetStatus(
                    &status
                    );
                if (SUCCEEDED(hr))
                {
                    if (status == UI_ANIMATION_MANAGER_BUSY)
                    {
                        InvalidateRect(
                            m_hwnd,
                            NULL,
                            FALSE
                            );
                    }
                }
            }
        }
    }

    return hr;
}
                                        
// Resizes the render target in response to a change in client area size                        

HRESULT CMainWindow::OnResize(
    UINT width,
    UINT height
    )
{
    HRESULT hr = S_OK;

    if (m_pRenderTarget)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;
        hr = m_pRenderTarget->Resize(
            size
            );
        if (SUCCEEDED(hr))
        {
            D2D1_SIZE_F sizeRenderTarget = m_pRenderTarget->GetSize();
            m_pBackgroundBrush->SetStartPoint(
                D2D1::Point2F(
                    sizeRenderTarget.width * 0.5f,
                    0.0f
                    )
                );
            m_pBackgroundBrush->SetEndPoint(
                D2D1::Point2F(
                    sizeRenderTarget.width * 0.5f,
                    sizeRenderTarget.height * 0.5f
                    )
                );

            hr = m_pLayoutManager->Arrange(
                m_pRenderTarget->GetSize()
                );
        }
    }

    return hr;
}

// Moves the images in various ways depending on the key pressed

HRESULT CMainWindow::OnKeyDown(
    UINT uVirtKey
    )
{
    HRESULT hr = S_OK;

    switch (uVirtKey)
    {
    case VK_UP:
        hr = m_pLayoutManager->Attract(
            0
            );
        break;

    case VK_DOWN:
        hr = m_pLayoutManager->Attract(
            m_pRenderTarget->GetSize().height
            );
        break;

    case VK_SPACE:
        hr = m_pLayoutManager->Arrange(
            m_pRenderTarget->GetSize()
            );
        break;
    
    default:
        break;
    }

    return hr;
}

// Prepares for window destruction

void CMainWindow::OnDestroy()
{
    if (m_pAnimationManager != NULL)
    {
        // Clear the manager event handler, to ensure that the one that points to this object is released
    
        (void)m_pAnimationManager->SetManagerEventHandler(NULL);
    }
}

// Draws the contents of the client area.
//
// Note that this function will automatically discard device-specific
// resources if the Direct3D device disappears during function
// invocation, and will recreate the resources the next time it's
// invoked.

HRESULT CMainWindow::DrawClientArea()
{
    // Begin drawing the client area

    HRESULT hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        hr = FindImages();
        if (SUCCEEDED(hr))
        {
            m_pRenderTarget->BeginDraw();

            // Retrieve the size of the render target

            D2D1_SIZE_F sizeRenderTarget = m_pRenderTarget->GetSize();
            
            // Paint the background

            m_pRenderTarget->FillRectangle(
                D2D1::RectF(
                    0.0f,
                    0.0f,
                    sizeRenderTarget.width,
                    sizeRenderTarget.height
                    ),
                m_pBackgroundBrush
                );

            // Paint all the thumbnails

            for (UINT i = 0; i < m_uThumbCount; i++)
            {
                hr = m_thumbs[i].Render(
                    m_pRenderTarget,
                    m_pOutlineBrush
                    );
                if (FAILED(hr))
                {
                    break;
                }
            }

            // Can only return one failure HRESULT
            
            HRESULT hrEndDraw = m_pRenderTarget->EndDraw();
            if (SUCCEEDED(hr))
            {
                hr = hrEndDraw;
            }

            if (hr == D2DERR_RECREATE_TARGET)
            {
                DiscardDeviceResources();
            }
        }
    }
    
    return hr;
}
