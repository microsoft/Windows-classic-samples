/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#include "stdafx.h"
#include "resource.h"

#include "WiaWrap.h"
#include "BitmapWnd.h"
#include "MainWnd.h"

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::CMainWindow
//

CMainWindow::CMainWindow()
{
    m_cRef = 0;

    m_nNumImages = 0;

    m_bDisplayWaitCursor = FALSE;

    m_pEventCallback = new WiaWrap::CEventCallback;

    if (m_pEventCallback != NULL)
    {
        m_pEventCallback->Register();
    }
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::QueryInterface
//

STDMETHODIMP CMainWindow::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == NULL)
    {
	    return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
	    *ppvObj = (IUnknown *) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

	AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::AddRef
//

STDMETHODIMP_(ULONG) CMainWindow::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::Release
//

STDMETHODIMP_(ULONG) CMainWindow::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::Register
//

ATOM CMainWindow::Register()
{
    WNDCLASSEX wcex = { 0 };

    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowProc;
    wcex.hInstance      = g_hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH) (COLOR_APPWORKSPACE + 1); 
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName  = _T("MainWindow");

    return RegisterClassEx(&wcex);
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::DoModal
//

int CMainWindow::DoModal()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (!TranslateMDISysAccel(m_hMDIClient, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::WindowProc 
//

LRESULT 
CALLBACK
CMainWindow::WindowProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    CMainWindow *that = (CMainWindow *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;

            that = (CMainWindow *) pcs->lpCreateParams;

            // Create the MDI client window that will handle the MDI child windows

            CLIENTCREATESTRUCT ccs = { 0 };

            ccs.hWindowMenu  = GetSubMenu(GetMenu(hWnd), WINDOW_MENU_POSITION);
            ccs.idFirstChild = FIRST_MDI_CHILD;

            that->m_hMDIClient = CreateWindowEx(
                0,
                _T("MDICLIENT"), 
                0, 
                WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE, 
                0,
                0,
                0, 
                0,
                hWnd,
                NULL, 
                g_hInstance, 
                &ccs
            );

            if (that->m_hMDIClient == NULL)
            {
                return -1;
            }

            that->AddRef();

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) that);

            break;
        }

        case WM_DESTROY:
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

            that->Release();

            PostQuitMessage(0);

            break;
        }

        case WM_SETCURSOR:
        {
            if (that->m_bDisplayWaitCursor)
            {
                SetCursor(LoadCursor(NULL, IDC_WAIT));

                return TRUE;
            }

            break;
        }

        case WM_INITMENU:
        {
            if (that->m_pEventCallback != NULL)
            {
                LONG nNumDevices = that->m_pEventCallback->GetNumDevices();

                UINT uEnable = nNumDevices > 0 ? MF_ENABLED : MF_GRAYED;

                EnableMenuItem((HMENU) wParam, ID_FILE_FROM_SCANNER_OR_CAMERA, uEnable);
            }

            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_FILE_FROM_SCANNER_OR_CAMERA:
                {
                    that->OnFromScannerOrCamera();
                    break;
                }

                case ID_FILE_EXIT:
                {
                    DestroyWindow(hWnd);
                    break;
                }

                case ID_WINDOW_CASCADE:
                {
                    SendMessage(that->m_hMDIClient, WM_MDICASCADE, 0, 0);
                    break;
                }

                case ID_WINDOW_TILE_HORIZONTALLY:
                {
                    SendMessage(that->m_hMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
                    break;
                }

                case ID_WINDOW_TILE_VERTICALLY:
                {
                    SendMessage(that->m_hMDIClient, WM_MDITILE, MDITILE_VERTICAL, 0);
                    break;
                }

                case ID_WINDOW_ARRANGE_ICONS:
                {
                    SendMessage(that->m_hMDIClient, WM_MDIICONARRANGE, 0, 0);
                    break;
                }

                case ID_HELP_ABOUT:
                {
                    TCHAR szAppName[DEFAULT_STRING_SIZE] = _T("");

	                LoadString(g_hInstance, IDS_APP_NAME, szAppName, COUNTOF(szAppName));

                    ShellAbout(hWnd, szAppName, NULL, NULL);

                    break;
                }
            }

            break;
        }
    }

    return DefFrameProc(
        hWnd,
        that == NULL ? NULL : that->m_hMDIClient,
        uMsg,
        wParam,
        lParam
    );
}

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow::OnFromScannerOrCamera
//

LRESULT CMainWindow::OnFromScannerOrCamera()
{
    HRESULT hr;

    m_bDisplayWaitCursor = TRUE;

    // Launch the get image dialog

    WiaWrap::CComPtrArray<IStream> ppStream;

    hr = WiaWrap::WiaGetImage(
        m_hMDIClient,
        StiDeviceTypeDefault,
        0,
        WIA_INTENT_NONE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &ppStream.Count(),
        &ppStream
    );

    m_bDisplayWaitCursor = FALSE;

    // If there was an error, display an error message box

	if (FAILED(hr)) 
    {
		TCHAR szError[DEFAULT_STRING_SIZE] = _T("");

		LoadString(g_hInstance, IDS_ERROR_GET_IMAGE_DLG, szError, COUNTOF(szError));

        MessageBox(m_hMDIClient, szError, NULL, MB_ICONHAND | MB_OK);
	}

    // Open a new window for each successfully transferred image

    for (int i = 0; i < ppStream.Count(); ++i)
    {
        CComPtr<CBitmapWnd> pBitmapWnd = new CBitmapWnd(ppStream[i]);

        if (pBitmapWnd != NULL)
        {
            m_nNumImages += 1;

            TCHAR szFormat[DEFAULT_STRING_SIZE] = _T("%d");

		    LoadString(g_hInstance, IDS_BITMAP_WINDOW_TITLE, szFormat, COUNTOF(szFormat));

		    TCHAR szTitle[DEFAULT_STRING_SIZE];

    	    _sntprintf_s(szTitle, COUNTOF(szTitle) -1 ,_TRUNCATE, szFormat, m_nNumImages);

            szTitle[COUNTOF(szTitle) - 1] = _T('\0');

            CreateWindowEx(
                WS_EX_MDICHILD,
                _T("BitmapWindow"),
                szTitle,
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                m_hMDIClient, 
                NULL, 
                g_hInstance, 
                pBitmapWnd
            );
        }
    }

    return 0;
}
