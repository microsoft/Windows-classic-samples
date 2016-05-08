// CWMPHost.h : Declaration of the CWMPHost
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "resource.h"       // main symbols
#include <oledlg.h>
#include "wmp.h"
#include "CWMPEventDispatch.h"


/////////////////////////////////////////////////////////////////////////////
// CWMPHost

class CWMPHost : public CWindowImpl<CWMPHost, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> >
{
public:
    DECLARE_WND_CLASS_EX(NULL, 0, 0)

    BEGIN_MSG_MAP(CWMPHost)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
        MESSAGE_HANDLER(WM_CONTEXTMENU, FowardMsgToWMP)

        COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)

        COMMAND_ID_HANDLER(ID_WMPCORE_CLOSE, OnWMPCoreClose)
        COMMAND_ID_HANDLER(ID_WMPCORE_URL, OnWMPCoreURL)
        COMMAND_ID_HANDLER(ID_WMPCORE_OPENSTATE, OnWMPCoreOpenState)
        COMMAND_ID_HANDLER(ID_WMPCORE_PLAYSTATE, OnWMPCorePlayState)
        COMMAND_ID_HANDLER(ID_WMPCORE_VERSIONINFO, OnWMPCoreVersionInfo)
        COMMAND_ID_HANDLER(ID_WMPCORE_LAUNCHURL, OnWMPCoreLaunchURL)
        COMMAND_ID_HANDLER(ID_WMPCORE_ISONLINE, OnWMPCoreIsOnline)
        COMMAND_ID_HANDLER(ID_WMPCORE_STATUS, OnWMPCoreStatus)
        COMMAND_ID_HANDLER(ID_WMPCORE_CONTROLS, OnWMPCoreInterface)       
        COMMAND_ID_HANDLER(ID_WMPCORE_SETTINGS, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CURRENTMEDIA, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_MEDIACOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_PLAYLISTCOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_NETWORK, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CURRENTPLAYLIST, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CDROMCOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CLOSEDCAPTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_ERROR, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE2_DVD, OnWMPCoreInterface)

        COMMAND_ID_HANDLER(ID_WMPPLAYER_ENABLED, OnWMPPlayerEnabled)        
        COMMAND_ID_HANDLER(ID_WMPPLAYER_FULLSCREEN, OnWMPPlayerFullScreen)        
        COMMAND_ID_HANDLER(ID_WMPPLAYER_ENABLECONTEXTMENU, OnWMPPlayerEnableContextMenu)        
        COMMAND_ID_HANDLER(ID_WMPPLAYER_UIMODE, OnWMPPlayerUIMode)        
        COMMAND_ID_HANDLER(ID_WMPPLAYER2_STRETCHTOFIT, OnWMPPlayer2StretchToFit)        
    END_MSG_MAP()

    void OnFinalMessage(HWND /*hWnd*/);

    LRESULT OnDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled );
    LRESULT OnCreate(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);

    LRESULT OnErase(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT OnSize(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT OnFileOpen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnFileExit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);

    LRESULT OnWMPCoreClose(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreOpenState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCorePlayState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreVersionInfo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreLaunchURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreIsOnline(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreStatus(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreInterface(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);

    LRESULT OnWMPPlayerEnabled(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerFullScreen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerEnableContextMenu(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerUIMode(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
 
    LRESULT OnWMPPlayer2StretchToFit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT FowardMsgToWMP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    CAxWindow                   m_wndView;
    CComPtr<IWMPPlayer>         m_spWMPPlayer;
    CComPtr<IConnectionPoint>   m_spConnectionPoint;
    DWORD                       m_dwAdviseCookie;
};
