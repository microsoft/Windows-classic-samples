// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "RibbonFramework.h"
#include "RichEditMng.h"
#include "ids.h"

//
//  FUNCTION: ShowMiniToolbar()
//
//  PURPOSE:  Shows a mini toolbar at specified screen point.
//
//
bool ShowMiniToolbar(__in const POINT& point)
{
    HRESULT hr = E_FAIL;
    POINT pt = point;

    if (pt.x == -1 && pt.y == -1)
    {
        // Keyboard initiated the context menu.
        RECT rect;
        GetWindowRect(g_pFCSampleAppManager->GetRichEditWnd(), &rect);
        
        // Show the mini toolbar where the cursor is.
        GetCursorPos(&pt);
        
        if (!PtInRect(&rect, pt))
        {
            // The cursor is not in the RichEdit window so use top left corner of the RicEdit control.
            pt.x = rect.left;
            pt.y = rect.top;
        }
    }
    // If there's a selection in the RichEdit control then the selection will be lost
    // because of mouse click, so show selection again before showing the context menu.
    g_pFCSampleAppManager->ShowSelection();
    
    if (g_pFramework)
    {
        // Get the view for contextual menu, which only has a mini toolbar.
        IUIContextualUI *pContextualUI;
        hr = g_pFramework->GetView(IDC_CMD_CONTEXTMAP, IID_PPV_ARGS(&pContextualUI));

        if (SUCCEEDED(hr))
        {
            // Show the mini toolbar at specified location.
            hr = pContextualUI->ShowAtLocation(pt.x, pt.y);
            pContextualUI->Release();
        }
    }

    return (SUCCEEDED(hr));
}
