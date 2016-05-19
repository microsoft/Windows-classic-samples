// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "RibbonFramework.h"
#include "Application.h"


IUIFramework* g_pFramework = NULL;

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
bool InitializeFramework(HWND hWindowFrame)
{
    // Here we instantiate the Ribbon framework object.
    HRESULT hr = ::CoCreateInstance(CLSID_UIRibbonFramework, NULL, 
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pFramework));
    if (FAILED(hr))
    {
        return false;
    }

    // Next, we create the application object (IUIApplication) and call the framework Initialize method, 
    // passing the application object and the host HWND that the Ribbon will attach itself to.
    CApplication *pApplication = NULL;
    hr = CApplication::CreateInstance(&pApplication, hWindowFrame);
    if (FAILED(hr))
    {
        return false;
    }

    hr = g_pFramework->Initialize(hWindowFrame, pApplication);
    if (FAILED(hr))
    {
        pApplication->Release();
        return false;
    }

    // Finally, we load the binary markup.  This will initiate callbacks to the IUIApplication object 
    // that was provided to the framework earlier, allowing command handlers to be bound to individual
    // commands.
    hr = g_pFramework->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
    if (FAILED(hr))
    {
        pApplication->Release();
        return false;
    }

    pApplication->Release();
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
    if(g_pFramework)
    {
        g_pFramework->Destroy();
        g_pFramework->Release();
        g_pFramework = NULL;
    }
}

//
//  FUNCTION: GetRibbonHeight()
//
//  PURPOSE:  Get the ribbon height.
//
//
UINT GetRibbonHeight()
{
    UINT ribbonHeight = 0;
    if (g_pFramework)
    {
        IUIRibbon* pRibbon;    
        g_pFramework->GetView(0, IID_PPV_ARGS(&pRibbon));
        pRibbon->GetHeight(&ribbonHeight);    
        pRibbon->Release();
    }
    return ribbonHeight;
}