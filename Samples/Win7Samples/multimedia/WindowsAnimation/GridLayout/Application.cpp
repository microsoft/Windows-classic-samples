// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "MainWindow.h"

// Entry point to the application

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        {
            CMainWindow mainWindow;
            hr = mainWindow.Initialize(hInstance);
            if (SUCCEEDED(hr))
            {
                MSG msg;
                while (GetMessage(&msg, NULL, 0, 0) > 0)
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        CoUninitialize();
    }

    return 0;
}
