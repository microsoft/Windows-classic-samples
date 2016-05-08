// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SPYCON_H_
#define __SPYCON_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSpyCon
class ATL_NO_VTABLE CSpyCon : 
    public CComObjectRoot,
    public CComCoClass<CSpyCon, &CLSID_SpyCon>,
    public ISpyCon, 
    public CWindowImpl<CSpyCon>,
    public IOleClientSite,
    public IOleInPlaceSite

{
private:
    WINDOWPLACEMENT m_wp;
    CComPtr<IOleObject> m_spOleObject;
    CComPtr<IComSpy> m_spSpy;    
    bool m_bInPlaceActive;
    bool m_bAlwaysOnTop;
    bool m_bSaveOnExit;

protected:
    CSpyCon()
    {
        ZeroMemory(&m_wp, sizeof(m_wp));
        m_wp.length = sizeof(m_wp);
        m_bInPlaceActive = false;
        m_bAlwaysOnTop = false;
        m_bSaveOnExit = true;
    }

    ~CSpyCon()
    {
    }

    void FinalRelease()
    {
    }

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SPYCON)
DECLARE_NOT_AGGREGATABLE(CSpyCon)

BEGIN_COM_MAP(CSpyCon)
    COM_INTERFACE_ENTRY(ISpyCon)
    COM_INTERFACE_ENTRY(IOleClientSite)
    COM_INTERFACE_ENTRY(IOleWindow)
    COM_INTERFACE_ENTRY(IOleInPlaceSite)
END_COM_MAP()

BEGIN_MSG_MAP(CSpyCon)
    MESSAGE_HANDLER(WM_NCDESTROY, OnNCDestroy)    
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
    MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
    COMMAND_ID_HANDLER(IDM_EXIT, OnExit)
    COMMAND_ID_HANDLER(WM_CLOSE, OnExit)
    COMMAND_ID_HANDLER(IDM_SELECT_APPLICATIONS, OnSelectFilter)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    COMMAND_ID_HANDLER(ID_LOG, OnLogToFile)
    COMMAND_ID_HANDLER(ID_CHOOSE_LOG_FILE_NAME, OnChooseLogFile)
    COMMAND_ID_HANDLER(ID_SAVE, OnSave)
    COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
    COMMAND_ID_HANDLER(ID_OPTIONS_GRID_LINES, OnToggleGridLines)
    COMMAND_ID_HANDLER(IDM_ON_TOP, OnAlwaysOnTop)
    COMMAND_ID_HANDLER(IDM_ABOUT, OnAbout)
    COMMAND_ID_HANDLER(ID_CHOOSEFONT, OnChooseFont)
    COMMAND_ID_HANDLER(ID_SHOW_ON_SCREEN, OnToggleShowOnScreen)
    COMMAND_ID_HANDLER(ID_SAVE_ON_EXIT, OnToggleSaveOnExit)
    COMMAND_ID_HANDLER(ID_SAVE_NOW, OnSaveNow)
END_MSG_MAP()

protected:
    LRESULT OnSaveNow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleSaveOnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleShowOnScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelectFilter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLogToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChooseLogFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToggleGridLines(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAlwaysOnTop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChooseFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNCDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// ISpyCon
public:
    STDMETHOD(Run)();

// IOleClientSite
public:
    STDMETHOD(SaveObject)()
    {
        ATLTRACENOTIMPL(L"IOleClientSite::SaveObject");
    }
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
    {
        ATLTRACENOTIMPL(L"IOleClientSite::GetMoniker");
    }
    STDMETHOD(GetContainer)(IOleContainer **ppContainer)
    {
        ATLTRACENOTIMPL(L"IOleClientSite::GetContainer");
    }
    STDMETHOD(ShowObject)()
    {
        ATLTRACENOTIMPL(L"IOleClientSite::ShowObject");
    }
    STDMETHOD(OnShowWindow)(BOOL fShow)
    {
        ATLTRACENOTIMPL(L"IOleClientSite::OnShowWindow");
    }
    STDMETHOD(RequestNewObjectLayout)()
    {
        ATLTRACENOTIMPL(L"IOleClientSite::RequestNewObjectLayout");
    }


// IOleWindow
public:
    STDMETHOD(GetWindow)(HWND *phwnd)
    {
        *phwnd = m_hWnd;
        return S_OK;
    }
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)
    {
        ATLTRACENOTIMPL(L"IOleWindow::ContextSensitiveHelp");
    }

// IOleInPlaceSite
public:
    HRESULT SaveSettings();
    STDMETHOD(CanInPlaceActivate)()
    {
        return S_OK;
    }
    STDMETHOD(OnInPlaceActivate)()
    {
        m_bInPlaceActive = true;
        return S_OK;
    }
    STDMETHOD(OnUIActivate)()
    {
        return S_OK;
    }
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
        LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
    {
        GetClientRect(lprcPosRect);
        GetClientRect(lprcClipRect);
        return S_OK;
    }
    STDMETHOD(Scroll)(SIZE scrollExtant)
    {
        ATLTRACENOTIMPL(L"IOleInPlaceSite::Scroll");
    }
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable)
    {
        ATLTRACENOTIMPL(L"IOleInPlaceSite::OnUIDeactivate");
    }
    STDMETHOD(OnInPlaceDeactivate)()
    {
        m_bInPlaceActive = false;
        return S_OK;
    }
    STDMETHOD(DiscardUndoState)()
    {
        ATLTRACENOTIMPL(L"IOleInPlaceSite::DiscardUndoState");
    }
    STDMETHOD(DeactivateAndUndo)()
    {
        ATLTRACENOTIMPL(L"IOleInPlaceSite::DeactivateAndUndo");
    }
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect)
    {
        ATLTRACENOTIMPL(L"IOleInPlaceSite::OnPosRectChange");
    }
};

#endif //__SPYCON_H_
