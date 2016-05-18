// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>
#include <shobjidl.h>
#include <dwmapi.h>
#include <strsafe.h>

#include "TabWnd.h"
#include "TabApp.h"

WCHAR const c_szWindowClass[] = L"TabApp_TdiWnd";
ATOM CTabWnd::RegisterClass()
{
    WNDCLASSEX wcex = {0};
    wcex.cbSize         = sizeof(wcex);
    wcex.lpfnWndProc    = CTabWnd::_WndProc;
    wcex.hInstance      = g_hInstance;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = c_szWindowClass;

    return ::RegisterClassEx(&wcex);
}

CTabWnd *CTabWnd::Create(int iTab, CMainDlg *pMainDlg)
{
    CTabWnd *pWnd = new CTabWnd(iTab, pMainDlg);

    if (pWnd)
    {
        WCHAR szTab[100];
        StringCchPrintf(szTab, ARRAYSIZE(szTab), L"Tab %d", iTab);

        // Create the CTabWnd window offscreen.
        HWND hwnd = ::CreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            c_szWindowClass,
            szTab,
            WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION,
            -32000,
            -32000,
            10,
            10,
            NULL,
            NULL,
            g_hInstance,
            (LPVOID)pWnd);

        if (hwnd == NULL)
        {
            delete pWnd;
            pWnd = NULL;
        }
    }

    return pWnd;
}

VOID CTabWnd::Destroy()
{
    if (_hwnd != NULL)
    {
        _pMainDlg->UnregisterTab(this);
        SetWindowLongPtr(_hwnd, GWLP_USERDATA, 0);

        HWND hwnd = _hwnd;
        _hwnd = NULL;
        DestroyWindow(hwnd);
    }
}

LRESULT CALLBACK CTabWnd::_WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT lResult = 0;

    CTabWnd *pWnd = (CTabWnd*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (pWnd == NULL && message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        pWnd = (CTabWnd*)lpcs->lpCreateParams;
        pWnd->_hwnd = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        lResult = ::DefWindowProc(hWnd, message, wParam, lParam);
    }
    else if (pWnd != NULL)
    {
        lResult = pWnd->WndProc(message, wParam, lParam);
    }
    else
    {
        lResult = ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lResult;
}

//
// Processes messages for the CTabWnd window
//
LRESULT CTabWnd::WndProc(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT lResult = 0;

    switch (message)
    {
        case WM_CREATE:
        {
            // Set DWM window attributes to indicate we'll provide the iconic bitmap, and
            // to always render the thumbnail using the iconic bitmap.
            BOOL fForceIconic = TRUE;
            BOOL fHasIconicBitmap = TRUE;

            DwmSetWindowAttribute(
                _hwnd,
                DWMWA_FORCE_ICONIC_REPRESENTATION,
                &fForceIconic,
                sizeof(fForceIconic));

            DwmSetWindowAttribute(
                _hwnd,
                DWMWA_HAS_ICONIC_BITMAP,
                &fHasIconicBitmap,
                sizeof(fHasIconicBitmap));

            // Tell the taskbar about this tab window
            _pMainDlg->RegisterTab(this);
            break;
        }

        case WM_ACTIVATE:
            // The taskbar will activate this window, so pass along the activation
            // to the tab window outer frame.
            if (LOWORD(wParam) == WA_ACTIVE)
            {
                _pMainDlg->ActivateTab(this);
            }
            break;

        case WM_SYSCOMMAND:
            // All syscommands except for close will be passed along to the tab window
            // outer frame. This allows functions such as move/size to occur properly.
            if (wParam != SC_CLOSE)
            {
                lResult = SendMessage(_pMainDlg->GetHwnd(), WM_SYSCOMMAND, wParam, lParam);
            }
            else
            {
                lResult = ::DefWindowProc(_hwnd, message, wParam, lParam);
            }
            break;

        case WM_CLOSE:
            // The taskbar (or system menu) is asking this tab window to close. Ask the
            // tab window outer frame to destroy this tab.
            _pMainDlg->DestroyTab(this);
            break;

        case WM_DWMSENDICONICTHUMBNAIL:
            // This tab window is being asked to provide its iconic bitmap. This indicates
            // a thumbnail is being drawn.
            _SendIconicRepresentation(HIWORD(lParam), LOWORD(lParam));
            break;

        case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
            // This tab window is being asked to provide a bitmap to show in live preview.
            // This indicates the tab's thumbnail in the taskbar is being previewed.
            _SendLivePreviewBitmap();
            break;

        default:
            lResult = ::DefWindowProc(_hwnd, message, wParam, lParam);
            break;
    }

    return lResult;
}

HRESULT CTabWnd::_SendIconicRepresentation(int nWidth, int nHeight)
{
    HRESULT hr = E_FAIL;

    HBITMAP hbm = _CreateDIB(nWidth, nHeight);
    if (hbm)
    {
        hr = DwmSetIconicThumbnail(_hwnd, hbm, 0);
        DeleteObject(hbm);
    }

    return hr;
}

HRESULT CTabWnd::_SendLivePreviewBitmap()
{
    HRESULT hr = S_OK;

    HWND hwndTabFrame = _pMainDlg->GetHwnd();
    DWORD dwStyle = GetWindowLong(hwndTabFrame, GWL_STYLE);
    DWORD dwStyleEx = GetWindowLong(hwndTabFrame, GWL_EXSTYLE);

    // Compute the actual size the thumbnail will occupy on-screen in order to
    // render the live preview bitmap. We use the tab window outer frame window
    // to compute this. In case that window is minimized, we use GetWindowPlacement
    // to give the correct information.
    RECT rcClient = {};
    RECT rcNCA = {};
    WINDOWPLACEMENT wp;
    if (AdjustWindowRectEx(&rcNCA, dwStyle, FALSE, dwStyleEx) != 0 &&
        GetWindowPlacement(hwndTabFrame, &wp) != 0)
    {
        if (wp.flags & WPF_RESTORETOMAXIMIZED)
        {
            HMONITOR hmon = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONULL);
            if (hmon)
            {
                MONITORINFO monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfo(hmon, &monitorInfo))
                {
                    rcClient = monitorInfo.rcWork;
                }
            }
        }
        else
        {
            CopyRect(&rcClient, &wp.rcNormalPosition);
        }

        rcClient.right -= (-rcNCA.left + rcNCA.right);
        rcClient.bottom -= (-rcNCA.top + rcNCA.bottom);
    }

    if ((rcClient.right - rcClient.left) > 0 && (rcClient.bottom - rcClient.top) > 0)
    {
        POINT ptOffset;
        ptOffset.x = ptOffset.y = 10;

        HBITMAP hbm = _CreateDIB(rcClient.right - rcClient.left - 2*ptOffset.x, rcClient.bottom - rcClient.top - 2*ptOffset.y);
        if (hbm)
        {
            hr = DwmSetIconicLivePreviewBitmap(_hwnd, hbm, &ptOffset, 0);
            DeleteObject(hbm);
        }
    }

    return hr;
}

// This is a helper method to render the tab window contents at a specified
// width and height.
HBITMAP CTabWnd::_CreateDIB(int nWidth, int nHeight)
{
    HBITMAP hbm = NULL;
    HDC hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem != NULL)
    {
        BITMAPINFO bmi;
        ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWidth;
        bmi.bmiHeader.biHeight = -nHeight;  // Use a top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;

        PBYTE pbDS = NULL;
        hbm = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (VOID**)&pbDS, NULL, NULL);
        if (hbm != NULL)
        {
            int nRed = 0, nBlue = 0, nGreen = 0;

            // Compute tab color: red, orange, yellow, green, blue, indigo, violet
            switch (_iTab % 7)
            {
                case 1:
                    nRed = 255;
                    break;
                case 2:
                    nRed = 255;
                    nGreen = 155;
                    break;
                case 3:
                    nRed = 255;
                    nGreen = 255;
                    break;
                case 4:
                    nGreen = 255;
                    break;
                case 5:
                    nBlue = 255;
                    break;
                case 6:
                    nRed = 155;
                    nBlue = 255;
                    break;
                case 0:
                    nRed = 255;
                    nBlue = 255;
                    break;
            }

            // Fill in the pixels of the bitmap
            for (int y = 0; y < nHeight; y++)
            {
                for (int x = 0; x < nWidth; x++)
                {
                    int edgeDistance = min(min(y, nHeight - y), min(x, nWidth - x));
                    int ring = min((edgeDistance / RINGWIDTH) + 1, MAXRING);
                    int nAlpha = ring * (255 / MAXRING);

                    pbDS[0] = (BYTE)(nBlue * nAlpha / 255);
                    pbDS[1] = (BYTE)(nGreen * nAlpha / 255);
                    pbDS[2] = (BYTE)(nRed * nAlpha / 255);
                    pbDS[3] = (BYTE)(nAlpha);

                    pbDS += 4;
                }
            }
        }

        DeleteDC(hdcMem);
    }

    return hbm;
}
