// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <crtdbg.h>
#include <stdio.h>

#include <gdiplus.h>
using namespace Gdiplus;

#define CONTROL_FONTFACE 100

const PWSTR controlLibraryName = L"UiaCleanShutdownControl.dll";

typedef HWND CreateCleanShutdownControl(_In_ HWND parent, _In_ int x, _In_ int y, _In_ int width, _In_ int height);

HINSTANCE thisInstance;
HWND loadButton;
HWND loadedControl;
HMODULE controlLibrary = NULL;

LRESULT CALLBACK MainWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT retVal = 0;

    switch (message) 
    {
    case WM_CREATE:
        {
           // Create the Load/Unload Button...
            loadButton = CreateWindow(L"BUTTON", L"Load DLL",
                    WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                    10, 30, 130, 40, hwnd, NULL, thisInstance, NULL);

            WCHAR szFontFace[50];
            LoadString(thisInstance, CONTROL_FONTFACE, szFontFace, ARRAYSIZE(szFontFace));
            HFONT font = CreateFont(0,0,0,0,0,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,szFontFace);
            SendMessage(loadButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), NULL);
            break;
        }

    case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }

    case WM_COMMAND:
        {
            // If the button is pressed
            if ((HWND)lParam == loadButton && HIWORD(wParam) == BN_CLICKED)
            {
                if (controlLibrary == NULL)
                {
                    // Load the Library and Access the CreateCleanShutdownControl in it
                    controlLibrary = LoadLibraryEx(controlLibraryName, NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
                    if (controlLibrary != NULL)
                    {
                        CreateCleanShutdownControl* createControl = reinterpret_cast<CreateCleanShutdownControl *>(GetProcAddress(controlLibrary, "CreateCleanShutdownControl"));
                        if (createControl != NULL)
                        {
                            RECT rc;
                            if (GetClientRect(hwnd, &rc))
                            {
                                // Call the DLL to create the control
                                loadedControl = createControl(hwnd, rc.right/2, 4, rc.right/2 - 4, rc.bottom - 8);
                            }
                        }
                        SendMessage(loadButton, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Unload DLL"));
                    }
                }
                else
                {
                    // Before unloading the DLL first delete the instance of the control
                    if (loadedControl != NULL)
                    {
                        DestroyWindow(loadedControl);
                        loadedControl = NULL;
                    }

                    // Now unload the library
                    if (FreeLibrary(controlLibrary))
                    {
                        controlLibrary = NULL;
                        SendMessage(loadButton, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Load DLL"));
                    }
                }
            }
            break;
        }

    default:
        {
            retVal = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return retVal;
}

bool RegisterMainWndClass(_In_ HINSTANCE instance)
{
    WNDCLASS wc;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = instance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"UiaCleanShutdownHost";

    if (!RegisterClass(&wc))
    {
        return false;
    }

    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
    // Ignore the return value because we want to run the program even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Set the Debug flags so if we're a debug build we'll dump all memory leaks on exit
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 

    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        ULONG_PTR gdiplusToken;
        GdiplusStartupInput gdiplusStartupInput;
        if(GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Ok)
        {
            if (RegisterMainWndClass(instance))
            {
                thisInstance = instance;
                HWND hwnd = CreateWindow(L"UiaCleanShutdownHost", L"UI Automation Clean Shutdown Sample",
                    WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX,
                    CW_USEDEFAULT, CW_USEDEFAULT, 320, 230, NULL, NULL, instance, NULL);

                if (hwnd != NULL)
                {
                    // The message loop, it will exit when it gets a WM_QUIT message
                    MSG msg;
                    while (GetMessage(&msg, NULL, 0, 0)) 
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }

                    // Disconnect all remaining remote Providers to prepare for shutdown
                    UiaDisconnectAllProviders();
                }
            }
            GdiplusShutdown(gdiplusToken);
        }
        CoUninitialize();
    }
}

