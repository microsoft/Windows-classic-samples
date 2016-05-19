/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#include "stdafx.h"
#include "resource.h"

#include "BitmapWnd.h"
#include "MainWnd.h"

//////////////////////////////////////////////////////////////////////////
//
// Global variable that holds the module instance
//

HINSTANCE g_hInstance = 0;

void RunGetImage()
{
    CComPtr<CMainWindow> pMainWindow = new CMainWindow;

    if (pMainWindow != NULL)
    {
        TCHAR szTitle[DEFAULT_STRING_SIZE] = _T("");

	    LoadString(g_hInstance, IDS_MAIN_WINDOW_TITLE, szTitle, COUNTOF(szTitle));

        HWND hWnd = CreateWindowEx(
            0,
            _T("MainWindow"),
            szTitle,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT, 
            CW_USEDEFAULT,
            NULL,
            NULL,
            g_hInstance,
            pMainWindow
        );

        if (hWnd != NULL)
        {
            pMainWindow->DoModal();
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Main entry point of the application
//

int 
APIENTRY 
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR      pCmdLine,
    int       nCmdShow
)
{
    // Initialize the global module instance value

    g_hInstance = hInstance;

    // Initialize the GDI+ library

    CGdiplusInit GdiplusInit;

    // Register window classes

    INITCOMMONCONTROLSEX iccex;

    iccex.dwSize = sizeof(iccex);
    iccex.dwICC  = ICC_BAR_CLASSES;

    InitCommonControlsEx(&iccex);

    CBitmapWnd::Register();
    
    CMainWindow::Register();

    // Initialize the COM library 

    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        // Create the main window and enter the message loop

        RunGetImage();

        // Close the COM library 

        CoUninitialize();
    }

    return 0;
}

