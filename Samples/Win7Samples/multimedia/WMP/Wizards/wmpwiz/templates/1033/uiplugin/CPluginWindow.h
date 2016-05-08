/////////////////////////////////////////////////////////////////////////////
//
// CPluginWindow.h : Declaration of the CPluginWindow
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "atlwin.h"

class CPluginWindow : public CWindowImpl<CPluginWindow>
{
public:
    BEGIN_MSG_MAP(CPluginWindow)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()

    CPluginWindow(C[!output Safe_root] *pPlugin)
    {
        m_pPlugin = pPlugin;
    }

    LRESULT OnEraseBackground(UINT nMsg, WPARAM wParam, 
                   LPARAM lParam, BOOL& bHandled)
    {
        // avoid erasing background to reduce flicker on resize
        return 1;
    }

    LRESULT OnPaint(UINT nMsg, WPARAM wParam, 
                   LPARAM lParam, BOOL& bHandled)
    {
        PAINTSTRUCT ps;

        HDC hDC = BeginPaint(&ps);

        RECT    rc;
        GetClientRect(&rc);

        HBRUSH hNewBrush = ::CreateSolidBrush( RGB(0, 0, 0) );
        
        if (hNewBrush)
        {
[!if HASPROPERTYPAGE]
            WCHAR *wszDisplayString = m_pPlugin->m_wszPluginText;
[!else]
            WCHAR *wszDisplayString = L"[!output root] Plugin";
[!endif]
            ::FillRect(hDC, &rc, hNewBrush );
            ::DeleteObject( hNewBrush );
            ::DrawText(hDC, wszDisplayString, lstrlen(wszDisplayString), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }
        
        EndPaint(&ps);
        return 0;
    }

private:
    C[!output Safe_root]  *m_pPlugin;  // pointer to plugin object
};

