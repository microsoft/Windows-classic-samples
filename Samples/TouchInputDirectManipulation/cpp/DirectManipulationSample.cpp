// Copyright (c) Microsoft Corporation. All rights reserved 

#include "stdafx.h"
#include "DirectManipulationSample.h"
#include "AppWindow.h"

using namespace DManipSample;

int __stdcall  wWinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/
                 , _In_ LPWSTR /*pszArgs*/, _In_ int /*nCmdShow*/)
{
    // Initialize COM apartment threaded. This is the recommended way to initialize COM for the UI thread.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    CAppWindow appWindow;

    int returnValue = appWindow.ShowAndServiceWindow();

    ::CoUninitialize();

    return returnValue;
}