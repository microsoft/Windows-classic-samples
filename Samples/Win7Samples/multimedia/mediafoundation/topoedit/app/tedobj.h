// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TEDOBJ__
#define __TEDOBJ__

class CTedApp;
class CTedAppVideoWindowHandler;
class CDock;
class CSplitterBar;

class CTedVideoWindow;
class CTedPlayer;

class CTedToolbar;
class CTedMainToolbar;

class CPropertyController;
class CPropertyView;

class CTedContentProtectionManager;

extern HINSTANCE g_hInst;

class CStatic : public CWindowImpl<CStatic> 
{
public:

    DECLARE_WND_SUPERCLASS(NULL, WC_STATIC);
    
protected:
    BEGIN_MSG_MAP(CStatic)
    END_MSG_MAP()
};

class CButton : public CWindowImpl<CButton> 
{
public:
    DECLARE_WND_SUPERCLASS(NULL, WC_BUTTON);

protected:
    BEGIN_MSG_MAP(CButton)
    END_MSG_MAP()
};


class CToolTipControl : public CWindowImpl<CToolTipControl>
{
public:
    CToolTipControl()
    {
    }
    
    DECLARE_WND_SUPERCLASS(NULL, TOOLTIPS_CLASS);
    
    
    HRESULT AddTool(HWND hWndParent, CAtlString strToolText, RECT rectTool, UINT nID)
    {
        TOOLINFO ToolInfo;
        // This is required because the build environment uses the 6.0 version of commctrl.h, but
        // the 5.8 version of comctl32.lib.  The tooltip class must pretend that it is using an old
        // toolinfo structure.
        ToolInfo.cbSize = TTTOOLINFO_V1_SIZE;
        ToolInfo.uFlags = TTF_SUBCLASS;
        ToolInfo.hwnd = hWndParent;
        ToolInfo.uId = nID;
        ToolInfo.rect.left = rectTool.left;
        ToolInfo.rect.right = rectTool.right;
        ToolInfo.rect.top = rectTool.top;
        ToolInfo.rect.bottom = rectTool.bottom;
        ToolInfo.hinst = 0;
        ToolInfo.lpszText = strToolText.GetBuffer();
        ToolInfo.lParam = 0;
        ToolInfo.lpReserved = NULL;
        if(FALSE == SendMessage(TTM_ADDTOOL, 0, (LPARAM) &ToolInfo))
        {
            return E_FAIL;
        }
        
        return S_OK;
    }
    
protected:
    BEGIN_MSG_MAP(CToolTipControl)
    END_MSG_MAP()
};

class CEdit : public CWindowImpl<CEdit> 
{
public:
    DECLARE_WND_SUPERCLASS(NULL, WC_EDIT);

    void SetToolTipControl(CToolTipControl* pToolTipControl) { m_pToolTipControl = pToolTipControl; }
protected:
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return RelayMessageToTooltipControl(uMsg, wParam, lParam);
    }
    
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return RelayMessageToTooltipControl(uMsg, wParam, lParam);
    }
    
    BEGIN_MSG_MAP(CEdit)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    END_MSG_MAP()
    
    LRESULT RelayMessageToTooltipControl(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if(!m_pToolTipControl) return 0;
        
        MSG msg;
        msg.hwnd = m_hWnd;
        msg.message = uMsg;
        msg.wParam = wParam;
        msg.lParam = lParam;
        msg.time = 0;
        msg.pt.x = LOWORD(lParam);
        msg.pt.y = HIWORD(lParam);
        return m_pToolTipControl->SendMessage(TTM_RELAYEVENT, 0, (LPARAM)&msg);
    }

private:
    CToolTipControl* m_pToolTipControl;
};

#define WM_MF_TOPOLOGYSET   (WM_APP + 1)
#define WM_MF_SESSIONENDED   (WM_APP + 2)
#define WM_MF_SESSIONPLAY   (WM_APP + 3)
#define WM_SPLITTERSIZE (WM_APP + 4)
#define WM_MF_HANDLE_UNTRUSTED_COMPONENT (WM_APP + 5)
#define WM_MF_HANDLE_PROTECTED_CONTENT (WM_APP + 6)
#define WM_MF_HANDLE_INDIVIDUALIZATION (WM_APP + 7)
#define WM_MF_TOPOLOGYREADY (WM_APP + 8)
#define WM_MF_CAPABILITIES_CHANGED (WM_APP + 9)

#define FACILITY_TED 255

#define TED_E_TRANSCODE_PROFILES_FILE_INVALID MAKE_HRESULT(1, FACILITY_TED, 1)
#define TED_E_INVALID_TRANSCODE_PROFILE MAKE_HRESULT(1, FACILITY_TED, 2)

#endif

