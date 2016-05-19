// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "ContextUI.h"
#include "RibbonFramework.h"

void GetDisplayLocation(POINT &pt, HWND hWnd)
{
    if (pt.x == -1 && pt.y == -1)
    {
        HRESULT hr = E_FAIL;

        // Display the menu in the upper-left corner of the client area, below the ribbon.
        IUIRibbon* pRibbon;
        hr = g_pFramework->GetView(0, IID_PPV_ARGS(&pRibbon));
        if (SUCCEEDED(hr))
        {
            UINT32 uRibbonHeight = 0;
            hr = pRibbon->GetHeight(&uRibbonHeight);
            if (SUCCEEDED(hr))
            {
                pt.x = 0;
                pt.y = uRibbonHeight;
                ClientToScreen(hWnd, &pt);
            }
            pRibbon->Release();
        }
        if (FAILED(hr))
        {
            // Default to just the upper-right corner of the entire screen.
            pt.x = 0;
            pt.y = 0;
        }
    }
}

HRESULT ShowContextualUI(POINT& ptLocation, HWND hWnd)
{   
    GetDisplayLocation(ptLocation, hWnd);

    HRESULT hr = E_FAIL;

    // The basic pattern of how to show Contextual UI in a specified location.
    IUIContextualUI* pContextualUI = NULL;
    if (SUCCEEDED(g_pFramework->GetView(g_pApplication->GetCurrentContext(), IID_PPV_ARGS(&pContextualUI))))
    {
        hr = pContextualUI->ShowAtLocation(ptLocation.x, ptLocation.y);
        pContextualUI->Release();
    }

    return hr;
}
