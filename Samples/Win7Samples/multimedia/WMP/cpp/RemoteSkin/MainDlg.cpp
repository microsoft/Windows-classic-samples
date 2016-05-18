// MainDlg.cpp : Implementation of CMainDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "MainDlg.h"
#include "OpenURLDlg.h"

// This constant is copied from wmp_i.c which can be generated from wmp.idl
const IID DIID__WMPOCXEvents = {0x6BF52A51,0x394A,0x11d3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};

/////////////////////////////////////////////////////////////////////////////
// CMainDlg

//***************************************************************************
// Constructor
//
//***************************************************************************
CMainDlg::CMainDlg()
{
    m_pView = NULL;
}

//***************************************************************************
// Destructor
//
//***************************************************************************
CMainDlg::~CMainDlg()
{
}

//***************************************************************************
// OnInitDialog()
// Initialize the dialog and create WMP OCX
//
//***************************************************************************
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HRESULT                             hr = S_OK;
    RECT                                rcClient;
    CComPtr<IObjectWithSite>            spHostObject;
    CComPtr<IAxWinHostWindow>           spHost;
    CComObject<CRemoteHost>             *pRemoteHost = NULL;


    // Create an ActiveX control container
    AtlAxWinInit();
    m_pView = new CAxWindow();  
    if(!m_pView)
    {
        hr = E_OUTOFMEMORY;
    }
    
    if(SUCCEEDED(hr))
    {
        ::GetWindowRect(GetDlgItem(IDC_RANGE), &rcClient);
        ScreenToClient(&rcClient);
        m_pView->Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

        if(::IsWindow(m_pView->m_hWnd))
        {
            hr = m_pView->QueryHost(IID_IObjectWithSite, (void **)&spHostObject);
            if(!spHostObject.p)
            {
                hr = E_POINTER;
            }
        }
    }

    // Create remote host which implements IServiceProvider and IWMPRemoteMediaServices
    if(SUCCEEDED(hr))
    {
        hr = CComObject<CRemoteHost>::CreateInstance(&pRemoteHost);
        if(pRemoteHost)
        {
            pRemoteHost->AddRef();
        }
        else
        {
            hr = E_POINTER;
        }
    }

    // Set site to the remote host
    if(SUCCEEDED(hr))
    {
        hr = spHostObject->SetSite((IWMPRemoteMediaServices *)pRemoteHost);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pView->QueryHost(&spHost);
        if(!spHost.p)
        {
            hr = E_NOINTERFACE;
        }
    }

    // Create WMP Control here
    if(SUCCEEDED(hr))
    {
        hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_pView->m_hWnd, NULL);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pView->QueryControl(&m_spPlayer);
        if(!m_spPlayer.p)
        {
            hr = E_NOINTERFACE;
        }
    }

    // Set skin to be custom skin
    if(SUCCEEDED(hr))
    {
        // Hook the event listener
        DispEventAdvise(m_spPlayer);
        // Put the UI mode to be a skin
        hr = m_spPlayer->put_uiMode(CComBSTR(L"custom"));
    }

    // Release remote host object
    if(pRemoteHost)
    {
        pRemoteHost->Release();
    }
    return 1;  // Let the system set the focus
}

//***************************************************************************
// OnDestroy()
// Release WMP OCX and its container here
//
//***************************************************************************
LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if(m_spPlayer)
    {
        // Unhook the event listener
        DispEventUnadvise(m_spPlayer);
        m_spPlayer.Release();
    }
    if(m_pView != NULL)
    {
        delete m_pView;
    }

    return 1;  // Let the system set the focus
}

//***************************************************************************
// OnCancel()
// When users click close button or press Esc, this function is called
// to close main dialog
//
//***************************************************************************
LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

//***************************************************************************
// OnOpenURL()
// When users click OpenURL button, this function is called to open
// a dialog so that users can give URL to play
//
//***************************************************************************
LRESULT CMainDlg::OnOpenURL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    COpenURLDlg dlg;
    if(IDOK == dlg.DoModal() && m_spPlayer.p)
    {
        m_spPlayer->put_URL(dlg.m_bstrURL);

        CComPtr<IWMPControls>   spControls;
        m_spPlayer->get_controls(&spControls);
        if(spControls.p)
        {
            spControls->play();
        }
    }
    return 0;
}

//***************************************************************************
// OnGoToML()
// When users click Go to Media Library button, this function is called to
// undock the player and go to Media Library pane
//
//***************************************************************************
LRESULT CMainDlg::OnGoToML(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    HRESULT                         hr = E_FAIL;
    CComPtr<IWMPPlayerApplication>  spPlayerApp;
    CComPtr<IWMPPlayerServices>     spPlayerServices;  

    if(m_spPlayer.p)
    {
        hr = m_spPlayer->QueryInterface(&spPlayerServices);
        if(!spPlayerServices.p)
        {
            hr = E_NOINTERFACE;
        }
    }

    if(SUCCEEDED(hr))
    {
        // Switch to media library pane
        spPlayerServices->setTaskPane(CComBSTR(L"MediaLibrary"));
    }

    if(m_spPlayer.p)
    {
        hr = m_spPlayer->get_playerApplication(&spPlayerApp);
        if(!spPlayerApp.p)
        {
            hr = E_NOINTERFACE;
        }
    }

    if(SUCCEEDED(hr))
    {
        // Undock the player
        spPlayerApp->switchToPlayerApplication();
    }

    return 0;
}

//***************************************************************************
// OnPlayStateChange()
// PlayStateChange event handler. When the player is undocked and user starts
// to play an item in it, this function docks the player
//
//***************************************************************************
HRESULT CMainDlg::OnPlayStateChange(long NewState)
{
    HRESULT                         hr = E_FAIL;
    CComPtr<IWMPPlayerApplication>  spPlayerApp;
    VARIANT_BOOL                    bDocked;

    if(m_spPlayer.p && (NewState == wmppsPlaying))
    {
        // When playState is wmppsPlaying, try to dock the player
        hr = m_spPlayer->get_playerApplication(&spPlayerApp);
        if(!spPlayerApp.p)
        {
            hr = E_NOINTERFACE;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = spPlayerApp->get_playerDocked(&bDocked);
        // If the player is now in undocked state, dock it.
        if(SUCCEEDED(hr) && (bDocked == VARIANT_FALSE))
        {
            hr = spPlayerApp->switchToControl();
        }
    }

    return S_OK;
}
