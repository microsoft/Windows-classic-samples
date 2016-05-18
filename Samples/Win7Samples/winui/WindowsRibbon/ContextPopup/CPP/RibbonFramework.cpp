// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <assert.h>

#include "RibbonFramework.h"

IUIFramework *g_pFramework = NULL;  // Reference to the Ribbon framework.
CApplication *g_pApplication = NULL;  // Reference to the Application object.

//
//  FUNCTION: InitializeFramework(HWND)
//
//  PURPOSE:  Initialize the Ribbon framework and bind a Ribbon to the application.
//
//  COMMENTS:
//
//    To get a Ribbon to display, the Ribbon framework must be initialized. 
//    This involves three important steps:
//      1) Instantiating the Ribbon framework object (CLSID_UIRibbonFramework).
//      2) Passing the host HWND and IUIApplication object to the framework.
//      3) Loading the binary markup compiled by UICC.exe.
//
//
__checkReturn bool InitializeFramework(HWND hWnd)
{
    // Here we instantiate the Ribbon framework object.
    HRESULT hr = CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pFramework));
    if (FAILED(hr))
    {
        return false;
    }

    // Next, we create the application object and call the framework Initialize method.
    hr = CApplication::CreateInstance(&g_pApplication);
    if (FAILED(hr))
    {
        return false;
    }

    IUIApplication *pApplication = NULL;
    __analysis_assume(g_pApplication != NULL);
    hr = g_pApplication->QueryInterface(IID_PPV_ARGS(&pApplication));
    if (SUCCEEDED(hr))
    {
        // Passing the application object and the host HWND that the Ribbon will attach itself to.
        hr = g_pFramework->Initialize(hWnd, pApplication);
        pApplication->Release();
    }
    if (FAILED(hr))
    {
        return false;
    }

    // Finally, we load the binary markup.  This will initiate callbacks to the IUIApplication object 
    // that was provided to the framework earlier, allowing command handlers to be bound to individual
    // commands.
    hr = g_pFramework->LoadUI(GetModuleHandle(NULL), L"CONTEXTPOPUP_RIBBON");
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

//
//  FUNCTION: DestroyFramework()
//
//  PURPOSE:  Tears down the Ribbon framework.
//
//
void DestroyFramework()
{
    if (g_pFramework)
    {
        HRESULT hr = g_pFramework->Destroy();
        UNREFERENCED_PARAMETER(hr);
        g_pFramework->Release();
        g_pFramework = NULL;
    }

    if (g_pApplication)
    {
        g_pApplication->Release();
        g_pApplication = NULL;
    }
}