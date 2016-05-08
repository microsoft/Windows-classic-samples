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

#include <stdio.h>
#include <windows.h>
#include <wincodec.h>
#include <commdlg.h>
#include <commctrl.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include "WICProgressiveDecoding.h"


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
        {
            DemoApp app;
            HWND hWndMain = NULL;

            hWndMain = app.Initialize(hInstance);
            hr = hWndMain ? S_OK : E_FAIL;

            if (SUCCEEDED(hr))
            {
                BOOL fRet;
                MSG msg;

                // Load accelerator table
                HACCEL haccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_ACCEL_TABLE));
                if (haccel == NULL) 
                {
                    hr = E_FAIL;
                }
                // Main message loop:
                while ((fRet = GetMessage(&msg, NULL, 0, 0)) != 0)
                {
                    if (fRet == -1)
                    {
                        break;
                    }
                    else
                    {
                        if (!TranslateAccelerator(hWndMain, haccel, &msg))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }
            }
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
    m_pD2DBitmap(NULL),
    m_pSourceFrame(NULL),
    m_pConvertedSourceBitmap(NULL),
    m_pIWICFactory(NULL),
    m_pD2DFactory(NULL),
    m_pRT(NULL),
    m_uCurrentLevel(0),
    m_hStatusWnd(NULL)
{
}

/******************************************************************
*  Tear down resources                                            *
******************************************************************/

DemoApp::~DemoApp()
{
    SafeRelease(m_pD2DBitmap);
    SafeRelease(m_pSourceFrame);
    SafeRelease(m_pConvertedSourceBitmap);
    SafeRelease(m_pIWICFactory);
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pRT);
}

/******************************************************************
*  Create application window and resources                        *
******************************************************************/

HWND DemoApp::Initialize(HINSTANCE hInstance)
{
    HRESULT hr = S_OK;

    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pIWICFactory)
        );

    if (SUCCEEDED(hr))
    {
        // Create D2D factory
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    }

    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex;

        // Register window class
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
        wcex.lpszClassName = L"WICProgressiveDecoding";
        wcex.hIconSm       = NULL;

        m_hInst = hInstance;

        hr = RegisterClassEx(&wcex) ? S_OK : E_FAIL;
    }

    HWND hWnd = NULL;
    if (SUCCEEDED(hr))
    {
        // Create main window
        hWnd = CreateWindow(
            L"WICProgressiveDecoding",
            L"WIC Progressive Decoding Sample",
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
    }

    // Create Status Bar window
    if (hWnd && !m_hStatusWnd)
    {
        m_hStatusWnd = CreateStatusBar(hWnd, IDS_STATUS, 1);
    }

    return hWnd;
}

/**********************************************************************************
*  Load an image file and create the frame that has the IWICProgressiveLeveLCtrl  *
**********************************************************************************/

HRESULT DemoApp::CreateProgressiveCtrlFromFile(HWND hWnd)
{
    HRESULT hr = S_OK;

    WCHAR szFileName[MAX_PATH];

    // Create the open dialog box and locate the image file
    if (LocateImageFile(hWnd, szFileName, ARRAYSIZE(szFileName)))
    {
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
        if (SUCCEEDED(hr))
        {
            // Need to release the previously source bitmap.
            // For each new image file, we need to create a new source bitmap
            SafeRelease(m_pSourceFrame);
            hr = pDecoder->GetFrame(0, &m_pSourceFrame);
        }

        IWICProgressiveLevelControl *pProgressive = NULL;

        if (SUCCEEDED(hr))
        {
            // Query for Progressive Level Control Interface
            hr = m_pSourceFrame->QueryInterface(IID_PPV_ARGS(&pProgressive));

            // Update progressive level count and reset the current level
            if (SUCCEEDED(hr))
            {
                m_uCurrentLevel = 0;
                hr = pProgressive->GetLevelCount(&m_uLevelCount);
            }
            else
            {
                MessageBox(hWnd, L"Image has no progressive encoding, select another one.", L"Application Error", 
                    MB_ICONEXCLAMATION | MB_OK);
            }
        }
        else
        {
            MessageBox(hWnd, L"Failed to load image, select another one.", L"Application Error", 
                MB_ICONEXCLAMATION | MB_OK);
        }

        SafeRelease(pDecoder);
        SafeRelease(pProgressive);
    }

    return hr;
}

/******************************************************************
*  Create a D2DBitmap at a specific progressive level             *
******************************************************************/
HRESULT DemoApp::CreateD2DBitmapFromProgressiveCtrl(HWND hWnd, UINT uLevel)
{
    HRESULT hr = E_FAIL;
    
    // Make sure source frame has been decoded successfully
    if (m_pSourceFrame)
    {
        // Query for Progressive Interface Control
        IWICProgressiveLevelControl *pProgressive = NULL;
        
        hr = m_pSourceFrame->QueryInterface(IID_PPV_ARGS(&pProgressive));

        // Set the progessive level before converting the format
        if (SUCCEEDED(hr))
        {
            hr = pProgressive->SetCurrentLevel(uLevel);
        }

        if (SUCCEEDED(hr))
        {
            SafeRelease(m_pConvertedSourceBitmap);
            hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
        }

        if (SUCCEEDED(hr) && m_pConvertedSourceBitmap)
        {
            // Format convert the current progressive level frame to 32bppPBGRA 
            hr = m_pConvertedSourceBitmap->Initialize(
                m_pSourceFrame,                  // Source bitmap to convert
                GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
                WICBitmapDitherTypeNone,         // Specified dither pattern
                NULL,                            // Specify a particular palette 
                0.f,                             // Alpha threshold
                WICBitmapPaletteTypeCustom       // Palette translation type
                );
        }

        if (SUCCEEDED(hr))
        {
            // Create device render target
            hr = CreateDeviceResources(hWnd);
        }

        // Create D2D bitmap from the converted source bitmap.
        if (SUCCEEDED(hr))
        {
            // Need to release the previous D2DBitmap if there is one.
            // For every new progressive level, we need to create a new D2DBitmap
            SafeRelease(m_pD2DBitmap);
            hr = m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, NULL, &m_pD2DBitmap);
        }

        // Update status window display
        if (SUCCEEDED(hr) && IsWindow(m_hStatusWnd))
        {
            WCHAR szLevelName[MAX_PATH];
            swprintf_s(szLevelName, ARRAYSIZE(szLevelName), L"Current Progress Level: %d", uLevel);
            SetWindowText(m_hStatusWnd, szLevelName);
        }

        // Force re-render
        if (SUCCEEDED(hr))
        {
            hr = InvalidateRect(hWnd, NULL, FALSE) ? S_OK : E_FAIL;
        }

        SafeRelease(pProgressive);
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
    ofn.lpstrFilter     = L"All Image Files\0"              L"*.gif;*.png;*.jpg;*.jpeg\0"
                          L"Graphics Interchange Format\0"  L"*.gif\0"
                          L"Portable Network Graphics\0"    L"*.png\0"
                          L"JPEG File Interchange Format\0" L"*.jpg;*.jpeg\0"
                          L"All Files\0"                    L"*.*\0"
                          L"\0";
    ofn.lpstrFile       = pszFileName;
    ofn.nMaxFile        = cchFileName;
    ofn.lpstrTitle      = L"Open Image";
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    // Display the Open dialog box. 
    return GetOpenFileName(&ofn);
}


/******************************************************************
*  This method creates resources which are bound to a particular  *
*  D2D device. It's all centralized here, in case the resources   *
*  need to be recreated in the event of D2D device loss           *
* (e.g. display change, remoting, removal of video card, etc).    *
******************************************************************/

HRESULT DemoApp::CreateDeviceResources(HWND hWnd)
{
    HRESULT hr = S_OK;

    if (!m_pRT)
    {
        RECT rc;
        hr = GetClientRect(hWnd, &rc) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            // Create a D2D render target properties
            D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties();

            // Set the DPI to be the default system DPI to allow direct mapping
            // between image pixels and desktop pixels in different system DPI
            // settings
            renderTargetProperties.dpiX = DEFAULT_DPI;
            renderTargetProperties.dpiY = DEFAULT_DPI;

            // Create a D2D render target
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

            hr = m_pD2DFactory->CreateHwndRenderTarget(
                renderTargetProperties,
                D2D1::HwndRenderTargetProperties(hWnd, size),
                &m_pRT
                );
        }
    }

    return hr;
}

/******************************************************************
*  This method creates a status bar of nParts                     *
******************************************************************/

HWND DemoApp::CreateStatusBar(HWND hWndParent, UINT nStatusID, UINT nParts) 
{
    HWND hWndStatus = NULL;

    // Required initialization for common ctrls.
    InitCommonControls();

    // Create a status bar window with default text
    hWndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE|WS_BORDER, L"No Image loaded", hWndParent, nStatusID);

    if (hWndStatus)
    {
        LPINT lpParts = NULL;

        lpParts = new INT[nParts]; 

        if (lpParts)
        {
            //Calculate width of each part
            RECT rcClient;
            UINT nWidth;

            if (GetClientRect(hWndParent, &rcClient))
            {
                nWidth = rcClient.right / nParts; 

                for (UINT i = 0; i < nParts; i++) 
                { 
                    lpParts[i] = nWidth; 
                    nWidth += nWidth; 
                } 
                // Divide the status bar to multiple parts 
                SendMessage(hWndStatus, SB_SETPARTS, (WPARAM) nParts, (LPARAM) lpParts);
            }
            delete[] lpParts;
        }
    }

    return hWndStatus; 
}

/******************************************************************
* Increase the current progressive level, and recreate resources  *
******************************************************************/

HRESULT DemoApp::IncreaseProgressiveLevel(HWND hWnd)
{
    HRESULT hr = S_OK;

    // Level range check
    if (m_uCurrentLevel < m_uLevelCount - 1)
    {
        m_uCurrentLevel++;

        // Re-create D2D Bitmap from the source frame at new progressive level
        hr = CreateD2DBitmapFromProgressiveCtrl(hWnd, m_uCurrentLevel);
    }

    return hr;
}

/******************************************************************
* Decrease the current progressive level, recreate resources      *
******************************************************************/

HRESULT DemoApp::DecreaseProgressiveLevel(HWND hWnd)
{
    HRESULT hr = S_OK;

    // Level range check
    if (m_uCurrentLevel > 0)
    {
        m_uCurrentLevel--;

        // Re-create D2D Bitmap from the source frame at new progressive level
        hr = CreateD2DBitmapFromProgressiveCtrl(hWnd, m_uCurrentLevel);
    }

    return hr;
}


/******************************************************************
*  Window message handler                                         *
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

LRESULT DemoApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
        case WM_COMMAND:
        {
            // Parse the menu selections:
            switch (LOWORD(wParam))
            {
                case IDM_FILE:
                {
                    // Load the new image file
                    hr = CreateProgressiveCtrlFromFile(hWnd);

                    if (SUCCEEDED(hr))
                    {
                        // Create D2DBitmap for rendering at default progessive level
                        hr = CreateD2DBitmapFromProgressiveCtrl(hWnd, 0);
                    }
                    break;
                }
                case IDM_NEXT:
                case IDA_ACTION_INCRE:
                {
                    hr = IncreaseProgressiveLevel(hWnd);
                    break;
                }
                case IDM_PRE:
                case IDA_ACTION_DECRE:
                {
                    hr = DecreaseProgressiveLevel(hWnd);
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
            return OnSize(lParam);
        }
        case WM_PAINT:
        {
            return OnPaint(hWnd);
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return SUCCEEDED(hr)? 0 : 1;
}

/******************************************************************
* Rendering callback using D2D                                    *
******************************************************************/

LRESULT DemoApp::OnPaint(HWND hWnd)
{
    HRESULT hr = S_OK;
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hWnd, &ps);

    if (hdc)
    {
        // Create render target if not yet created
        hr = CreateDeviceResources(hWnd);

        if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            m_pRT->BeginDraw();

            m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

            // Clear the background
            m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

            D2D1_SIZE_F rtSize = m_pRT->GetSize();

            // Create a rectangle same size of current window
            D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

            // D2DBitmap may have been released due to device loss. 
            // If so, re-create it from the source bitmap
            if (m_pConvertedSourceBitmap && !m_pD2DBitmap)
            {
                m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, NULL, &m_pD2DBitmap);
            }

            // Draws an image and scales it to the current window size
            if (m_pD2DBitmap)
            {
                m_pRT->DrawBitmap(m_pD2DBitmap, rectangle);
            }

            hr = m_pRT->EndDraw();

            // In case of device loss, discard D2D render target and D2DBitmap.
            // They will be re-create in the next rendering pass
            if (hr == D2DERR_RECREATE_TARGET)
            {
                SafeRelease(m_pD2DBitmap);
                SafeRelease(m_pRT);
                // Force a re-render
                hr = InvalidateRect(hWnd, NULL, TRUE)? S_OK : E_FAIL;
            }
        }

        EndPaint(hWnd, &ps);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}  

/******************************************************************
* Resizing callback                                               *
******************************************************************/

LRESULT DemoApp::OnSize(LPARAM lParam)
{
    LRESULT lRet = 1;

    // Resize the status window
    if (IsWindow(m_hStatusWnd))
    {
        SendMessage(m_hStatusWnd, WM_SIZE, 0, lParam);
    }

    // Resize rendering target
    D2D1_SIZE_U size = D2D1::SizeU(LOWORD(lParam), HIWORD(lParam));

    if (m_pRT)
    {
        // If we couldn't resize, release the device and we'll recreate it
        // during the next render pass.
        if (FAILED(m_pRT->Resize(size)))
        {
            SafeRelease(m_pRT);
            SafeRelease(m_pD2DBitmap);
        }
        else
        {
            lRet = 0;
        }
    }

    return lRet;
}

