//------------------------------------------------------------------------------
//
// File: winmain.cpp
// Application entry-point.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#include "MFPlayer.h"
#include "MainDialog.h"

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize the COM library.
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (FAILED(hr))
    {
        MessageBox(NULL, L"CoInitialize failed.", NULL, MB_ICONSTOP);
        return 0;
    }

    // Initialize the common control library.
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&icc))
    {
        MessageBox(NULL, L"InitCommonControlsEx failed.", NULL, MB_ICONSTOP);
        CoUninitialize();
        return 0;
    }

    // Initialize our custom slider class.
    hr = Slider_Init(); 
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Slider_Init failed.", NULL, MB_ICONSTOP);
        CoUninitialize();
        return 0;
    }

    // Create and show the dialog.
    MainDialog *pDlg = new (std::nothrow) MainDialog();
    if (pDlg == NULL)
    {
        MessageBox(NULL, L"Out of memory.", NULL, MB_ICONSTOP);
    }
    else
    {
        pDlg->ShowDialog(hInstance);

        delete pDlg;
    }

    CoUninitialize();
    return 0;
}
