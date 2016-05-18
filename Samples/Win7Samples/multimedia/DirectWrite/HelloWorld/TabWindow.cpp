
/************************************************************************
 *
 * File: TabWindow.cpp
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#include "DWriteHelloWorld.h"


/******************************************************************
*                                                                 *
*  TabWindow::TabWindow constructor                               *
*                                                                 *
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

TabWindow::TabWindow() :
    hwnd_(NULL),
    hwndTab_(NULL),
    hwndChild_(NULL)
{
}

/******************************************************************
*                                                                 *
*  TabWindow::~TabWindow destructor                               *
*                                                                 *
*  Tear down resources                                            *
*                                                                 *
******************************************************************/

TabWindow::~TabWindow()
{
}

/******************************************************************
*                                                                 *
*  TabWindow::Initialize                                          *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/

HRESULT TabWindow::Initialize()
{
    WNDCLASSEX wcex;
    HRESULT hr;
    ATOM atom;

    // Register window class.
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = TabWindow::WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = HINST_THISCOMPONENT;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = NULL;
    wcex.hIcon         = LoadIcon(
                            NULL,
                            IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(
                            NULL,
                            IDC_ARROW);
    wcex.lpszClassName = TEXT("TabWindow");
    wcex.hIconSm       = LoadIcon(
                            NULL,
                            IDI_APPLICATION
                            );

    atom = RegisterClassEx(
        &wcex
        );

    hr = atom ? S_OK : E_FAIL;
     
    hwnd_ = CreateWindow(
        TEXT("TabWindow"),
        TEXT("Direct Write samples"),
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        NULL,
        NULL,
        HINST_THISCOMPONENT,
        this
        );

    if (SUCCEEDED(hr))
    {
        hr = hwnd_ ? S_OK : E_FAIL;
    }

    // Initialize tabs.
    hwndTab_ = CreateTabControl();

    if (hwndTab_ == NULL)
    {
        hr = E_FAIL;
    }
  
    // Initialize child windows.
    if (SUCCEEDED(hr))
    {
        hr = CreateChildWindows();
    }

    ShowWindow(
        hwnd_,
        SW_SHOWNORMAL
        );

    UpdateWindow(
        hwnd_
        );

    return hr;
}

HWND TabWindow::CreateTabControl() 
{ 
    RECT rcClient; 
    HWND hwndTab; 
    TCITEM tie; 

    const wchar_t* tab1Label = TEXT("Simple Text");
    const wchar_t* tab2Label = TEXT("Multiformatted Text");
    const wchar_t* tab3Label = TEXT("Custom Text Renderer");

    // Get the dimensions of the parent window's client area, and 
    // create a tab control child window of that size. 
    GetClientRect(hwnd_, &rcClient); 
    InitCommonControls(); 
    hwndTab = CreateWindowEx(0, WC_TABCONTROL, TEXT(""),
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
        0, 0, rcClient.right, rcClient.bottom, 
        hwnd_, NULL, HINST_THISCOMPONENT, NULL); 

    if (hwndTab == NULL)
    { 
        return NULL; 
    }
 
    // Add tabs.
    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 

    tie.pszText = (LPWSTR) tab1Label; 
 
    if (TabCtrl_InsertItem(hwndTab, 0, &tie) == -1) 
    { 
       DestroyWindow(hwndTab); 
       return NULL; 
    }

    tie.pszText = (LPWSTR) tab2Label; 
 
    if (TabCtrl_InsertItem(hwndTab, 1, &tie) == -1) 
    { 
       DestroyWindow(hwndTab); 
       return NULL; 
    } 
    
    tie.pszText = (LPWSTR) tab3Label; 
 
    if (TabCtrl_InsertItem(hwndTab, 2, &tie) == -1) 
    { 
       DestroyWindow(hwndTab); 
       return NULL; 
    } 
    return hwndTab; 
}

HRESULT TabWindow::CreateChildWindows()
{
    HRESULT hr;

    hr = simpleText_.Initialize(hwnd_);

    if (SUCCEEDED(hr))
    {
        hr = multiformattedText_.Initialize(hwnd_);
    }

    if (SUCCEEDED(hr))
    {
        hr = customText_.Initialize(hwnd_);
    }

    hwndChild_ = simpleText_.GetHwnd();

    return hr;
}


/******************************************************************
*                                                                 *
*  TabWindow::OnPaint                                             *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window.                                                        *
*                                                                 *
******************************************************************/

HRESULT TabWindow::OnPaint(const PAINTSTRUCT &ps)
{
    HRESULT hr = S_OK;

    return hr;
}

/******************************************************************
*                                                                 *
*  TabWindow::OnResize                                            *
*                                                                 *
*  If the application receives a WM_SIZE message                  *
*                                                                 *
******************************************************************/

void TabWindow::OnResize(UINT width, UINT height)
{
    HDWP hdwp; 
    RECT rc; 

    // Calculate the display rectangle, assuming the 
    // tab control is the size of the client area. 
    SetRect(&rc, 0, 0, 
            width, height); 
    TabCtrl_AdjustRect(hwndTab_, FALSE, &rc); 

    // Size the tab control to fit the client area. 
    hdwp = BeginDeferWindowPos(2); 
    DeferWindowPos(hdwp, hwndTab_, NULL, 0, 0, 
        width, height, 
        SWP_NOMOVE | SWP_NOZORDER 
        );

     // Position and size the child window
    // and make sure the 
    // it is in front of the tab control. 
    DeferWindowPos(hdwp, 
        hwndChild_, HWND_TOP, rc.left, rc.top, 
        rc.right - rc.left, rc.bottom - rc.top, 0 
        ); 
    EndDeferWindowPos(hdwp);
}

/******************************************************************
*                                                                 *
*  TabWindow::WndProc                                             *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

LRESULT CALLBACK TabWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        TabWindow *pTabWindow = (TabWindow *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pTabWindow));

        return 1;
    }

    TabWindow *pTabWindow = reinterpret_cast<TabWindow *>(
                ::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (pTabWindow)
    {
        switch(message)
        {
        case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pTabWindow->OnResize(width, height);
            }
            return 0;
        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                pTabWindow->OnPaint(
                    ps
                    );
                EndPaint(
                    hwnd,
                    &ps
                    );
            }
            return 0;
        case WM_ERASEBKGND:
            return true;
        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
            case TCN_SELCHANGE:    
                { 
                    int selection = TabCtrl_GetCurSel(pTabWindow->hwndTab_); 
                    
                    // Hide the current child window.
                    ShowWindow(pTabWindow->hwndChild_, SW_HIDE);

                    // Get the handle to the new child window to display.
                    switch (selection)
                    {
                    case 0:
                        {
                            pTabWindow->hwndChild_ = pTabWindow->simpleText_.GetHwnd();
                            break;
                        }
                    case 1:
                        {
                            pTabWindow->hwndChild_ = pTabWindow->multiformattedText_.GetHwnd();
                            break;
                        }
                    case 2:
                        {
                            pTabWindow->hwndChild_ = pTabWindow->customText_.GetHwnd();
                            break;
                        }
                    }

                    // Show the new child window.
                    ShowWindow(pTabWindow->hwndChild_, SW_SHOWNORMAL);

                    // Get the current window dimensions and have the main window 
                    // resize the child window to fit it in the area below the tab control.
                    //
                    // This is required for the first time a new child is displayed to prevent
                    // it from overlapping the tab control. 
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    pTabWindow->OnResize(rect.right - rect.left, rect.bottom - rect.top);
                } 
            }
            return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
        }
    }
    return DefWindowProc(
        hwnd,
        message,
        wParam,
        lParam
        );
}


