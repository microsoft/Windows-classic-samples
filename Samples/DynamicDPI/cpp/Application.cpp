//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "Application.h"
#include "SampleDynamicDpiWindow.h"
#include <CommCtrl.h>

// <summary>
// Based on the screen dimensions, decide the size of the application window.
// </summary>
HRESULT 
CalcInitialWindowBounds(
    _In_    float logicalDefaultWidth,
    _In_    float logicalDefaultHeight,
    _Out_   RECT *bounds
    )
{
    // Retrieve the screen height and width in logical pixels.
    HDC hDC = GetDC(NULL);
    if (!hDC)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    float sysDpiFloat = static_cast<float>(GetDeviceCaps(hDC, LOGPIXELSX));

    if (!ReleaseDC(NULL, hDC))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the dimensions of the bounds RECT provided.
    bounds->left = 10;
    bounds->top = 10;
    bounds->right = static_cast<int>(logicalDefaultWidth*(sysDpiFloat / 96.0F));
    bounds->bottom = static_cast<int>(logicalDefaultHeight*(sysDpiFloat / 96.0F));

    return S_OK;
}

//
// Main application entry point.
//
int
WINAPI WinMain(
    _In_        HINSTANCE hInstance,
    _In_opt_    HINSTANCE hPrevInstance,
    _In_        LPSTR lpCmdLine,
    _In_        int nShowCmd)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    HRESULT hr = S_OK;
    RECT initialBounds;
    auto dynamicDpiWindow = std::make_shared<CSampleDynamicDpiWindow>();

    // Initialize the main application window using
    // a default size and window title.
    hr = CalcInitialWindowBounds(1120, 660, &initialBounds);
    if (SUCCEEDED(hr))
    {
        InitCommonControls();
        hr = dynamicDpiWindow->Initialize(initialBounds, L"Dynamic DPI Sample Window");
    }

    // Process application messages.
    if (SUCCEEDED(hr))
    {
        hr = dynamicDpiWindow->Run();
    }

    return hr;
}