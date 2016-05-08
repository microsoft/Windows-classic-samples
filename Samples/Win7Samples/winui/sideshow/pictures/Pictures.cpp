// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Pictures.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PictureClient.h"
#include <gdiplus.h>

using namespace Gdiplus;

DWORD g_threadID = 0;

ULONG_PTR g_gdiToken = 0;

void InitClient(CPictureClient& client)
{
    //
    // Initialize GDI+
    //
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiToken, &gdiplusStartupInput, NULL);
    
    //
    // Register this client application with the Windows SideShow
    // platform
    //
    client.Register();

    //
    // Add content to the display
    //
    client.AddContent();
}

void UnInitClient(CPictureClient& client)
{
    //
    // Remove all of the content from the display
    // so it's no longer available once this application
    // closes
    //
    client.RemoveAllContent();
    
    //
    // Unregister this client application from the platform
    //
    client.Unregister();
    
    //
    // Take-down GDI+
    //
    GdiplusShutdown(g_gdiToken);
}

/////////////////////////////////////////////////////////////////////////////
// Entry Point
int __stdcall WinMain(HINSTANCE /*hInst*/,
                      HINSTANCE /*hInstPrev*/,
                      LPSTR /*szCommandLine*/,
                      int /*nCmdShow*/)
{
    //
    // Ensure that only one process may run per-user.
    //
    HANDLE hMutex = CreateMutex(
                        NULL,
                        TRUE,
                        L"{00BA1AD8-5381-449b-9A13-551FA4ADE0F2}"
                        );
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (0 != hMutex)
        {
            CloseHandle(hMutex);
        }
        return 0;
    }
    
    //
    // Initialize COM
    //
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
    g_threadID = GetCurrentThreadId();

    //
    // Run the client application
    //
    CPictureClient client;
    InitClient(client);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnInitClient(client);

    //
    // Finally, uninitialize COM and close the mutex handle.
    //
    if (0 != hMutex)
    {
        CloseHandle(hMutex);
    }

    ::CoUninitialize();
    return 0;
}

