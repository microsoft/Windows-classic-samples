// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "resource.h"
#include "tedapp.h"
#include "tedmaintoolbar.h"
#include "dock.h"
#include "mfapi.h"
#include "mferror.h"
#include "commdlg.h"
#include "splitterbar.h"
#include "tedtransformdialog.h"
#include "dmo.h"
#include "propertyview.h"
#include "tedinputguiddialog.h"
#include "Tedcontentprotectionmanager.h"
#include "tedplayer.h"
#include <initguid.h>
#include <mferror.h>
#include <uuids.h>
#include <Windowsx.h>
#include "tedtranscode.h"

#include <assert.h>

unsigned int CTedApp::MAIN_TOOLBAR_ID = 5000;
const int CTedApp::m_nSeekerRange = 100;
const UINT_PTR CTedApp::ms_nTimerID = 0;
const DWORD CTedApp::ms_dwTimerLen = 200;
const double CTedApp::m_dblInitialSplitterPos = 0.65;

///////////////////////////////////////////////////////////////////////////////
//
HINSTANCE g_hInst = NULL;
HINSTANCE g_hTedUtilInst = NULL;
CTedApp * g_pApp;

//////////////////////////////////
// CTedAppVideoWindowHandler
// Provides video windows to TEDUTIL.  

CTedAppVideoWindowHandler::CTedAppVideoWindowHandler(HWND hWndParent)
    : m_hWndParent(hWndParent)
    , m_cRef(0)
{
}

CTedAppVideoWindowHandler::~CTedAppVideoWindowHandler()
{
    for(size_t i = 0; i < m_arrWindows.GetCount(); i++)
    {
        if(m_arrWindows.GetAt(i)->m_hWnd) m_arrWindows.GetAt(i)->DestroyWindow();
        delete m_arrWindows.GetAt(i);
    }
}

HRESULT CTedAppVideoWindowHandler::GetVideoWindow(LONG_PTR* phWnd)
{
    HRESULT hr = S_OK;

    RECT rectLastWindow = {0, 0, 0, 0};
    if(m_arrWindows.GetCount() >= 1)
    {
        m_arrWindows.GetAt(m_arrWindows.GetCount() - 1)->GetWindowRect(&rectLastWindow);
    }

    RECT rect;
    rect.left = m_dwCascadeMargin + rectLastWindow.left;
    rect.top = m_dwCascadeMargin + rectLastWindow.top;
    rect.right = rect.left + m_dwDefaultWindowWidth;
    rect.bottom = rect.top + m_dwDefaultWindowHeight;
    
    if(NULL == phWnd)
    {
        IFC( E_POINTER );
    }
    
    CTedVideoWindow* pVideoWindow = new CTedVideoWindow();
    if(pVideoWindow->Create(m_hWndParent, &rect, LoadAtlString(IDS_VIDEO_PLAYBACK), WS_CAPTION | WS_POPUPWINDOW, 0, 0U, NULL) == NULL)
    {
        IFC( HRESULT_FROM_WIN32(GetLastError()) );
    }

    *phWnd = (LONG_PTR) pVideoWindow->m_hWnd;
    m_arrWindows.Add(pVideoWindow);

Cleanup:
    return hr;
}

HRESULT CTedAppVideoWindowHandler::ReleaseVideoWindow(LONG_PTR hWnd)
{
    for(size_t i = 0; i < m_arrWindows.GetCount(); ++i)
    {
        CTedVideoWindow* pWindow = m_arrWindows.GetAt(i);

        if(pWindow->m_hWnd == (HWND) hWnd)
        {
            m_arrWindows.RemoveAt(i);
            --i;
            
            pWindow->DestroyWindow();
            delete pWindow;
            break;
        }
    }

    return S_OK;
}

HRESULT CTedAppVideoWindowHandler::ShowWindows(int nCmdShow)
{
    for(size_t i = 0; i < m_arrWindows.GetCount(); i++)
    {
        m_arrWindows.GetAt(i)->ShowWindow(nCmdShow);
    }

    return S_OK;
}

HRESULT CTedAppVideoWindowHandler::QueryInterface(REFIID riid, void** ppInterface)
{
    if(NULL == ppInterface)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    if(riid == IID_IUnknown)
    {
        IUnknown* punk = this;
        *ppInterface = punk;
        AddRef();
    }
    else if(riid == IID_ITedVideoWindowHandler)
    {
        ITedVideoWindowHandler* ptvwh = this;
        *ppInterface = ptvwh;
        AddRef();
    }
    else
    {
        *ppInterface = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}

ULONG CTedAppVideoWindowHandler::AddRef()
{
    LONG cRef = InterlockedIncrement(&m_cRef);

    return cRef;
}

ULONG CTedAppVideoWindowHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

///////////////////////////////////////////////////////////////////////////////
// CTedAppMediaEventHandler
// Decouples player from app class.  Initiates application response to media events

CTedAppMediaEventHandler::CTedAppMediaEventHandler(CTedApp* pApp)
    : m_pApp(pApp)
{
}

CTedAppTopoEventHandler::~CTedAppTopoEventHandler()
{
}

void CTedAppMediaEventHandler::NotifyEventError(HRESULT hr)
{
    m_pApp->HandleMMError(LoadAtlString(IDS_E_MEDIA_EVENT), hr);
}

void CTedAppMediaEventHandler::HandleMediaEvent(IMFMediaEvent* pEvent)
{
    HRESULT hr = S_OK;
    
    MediaEventType met;
    HRESULT hrEvent;

    IFC( pEvent->GetType(&met) );
    IFC( pEvent->GetStatus(&hrEvent) );

    
    if(SUCCEEDED(hrEvent))
    {
        switch(met)
        {
            case MESessionStarted:
                m_pApp->PostMessage(WM_MF_SESSIONPLAY, hrEvent, 0);
                break;
            case MESessionEnded:
                m_pApp->PostMessage(WM_MF_SESSIONENDED, hrEvent, 0);
                break;
            case MESessionTopologySet:
                m_pApp->PostMessage(WM_MF_TOPOLOGYSET, hrEvent, 0);
                break;
            case MESessionTopologyStatus:
                {
                    UINT32 unTopoStatus = MFGetAttributeUINT32(pEvent, MF_EVENT_TOPOLOGY_STATUS, 0);
                    if(MF_TOPOSTATUS_READY == unTopoStatus)
                    {
                        m_pApp->PostMessage(WM_MF_TOPOLOGYREADY, hrEvent, 0);
                    }
                }
                break;
            case MESessionCapabilitiesChanged:
                m_pApp->PostMessageW(WM_MF_CAPABILITIES_CHANGED, hrEvent, 0);
                break;
        }
    }
    else
    {
        switch(met)
        {
            case MESessionStarted:
                m_pApp->HandleMMError(LoadAtlString(IDS_E_PLAYBACK_START), hrEvent);
                break;
            case MESessionTopologySet:
                m_pApp->PostMessage(WM_MF_TOPOLOGYSET, hrEvent, 0);
                break;
            case MESessionTopologiesCleared:
                // This can fail if the session has been closed, but
                // this is OK -- the session will still accept new topologies.
                break;
            default:
                NotifyEventError(hrEvent);
        }
    }
    
Cleanup:
    if(FAILED(hr))
    {
        NotifyEventError(hr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// CTedAppTopoEventHandler
// Receives event notifications from the topology editor

CTedAppTopoEventHandler::CTedAppTopoEventHandler(CTedApp* pApp)
    : m_pApp(pApp)
{
}

HRESULT CTedAppTopoEventHandler::NotifyAddedNode(int nNodeID)
{
    m_pApp->NotifyTopoChange();
    return S_OK;
}

HRESULT CTedAppTopoEventHandler::NotifyRemovedNode(int nNodeID)
{
    m_pApp->NotifyTopoChange();
    return S_OK;
}

HRESULT CTedAppTopoEventHandler::NotifyConnection(int nUpNodeID, int nDownNodeID)
{
    m_pApp->NotifyTopoChange();
    return S_OK;
}

HRESULT CTedAppTopoEventHandler::NotifyDisconnection(int nUpNodeID, int nDownNodeID)
{
    m_pApp->NotifyTopoChange();
    return S_OK;
}

HRESULT CTedAppTopoEventHandler::QueryInterface(REFIID riid, void** ppInterface)
{
    if(NULL == ppInterface)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    if(riid == IID_IUnknown)
    {
        IUnknown* punk = this;
        *ppInterface = punk;
        AddRef();
    }
    else if(riid == IID_ITedTopoEventHandler)
    {
        ITedTopoEventHandler* ptteh = this;
        *ppInterface = ptteh;
        AddRef();
    }
    else
    {
        *ppInterface = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}

ULONG CTedAppTopoEventHandler::AddRef()
{
    LONG cRef = InterlockedIncrement(&m_cRef);

    return cRef;
}

ULONG CTedAppTopoEventHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

///////////////////////////////////////////////////////////////////////////////
//CTedChooserDialog
// Generic drop-down selection dialog

CTedChooserDialog::CTedChooserDialog(const CAtlString& strTitle)
    : m_strTitle(strTitle)
{
}

void CTedChooserDialog::AddPossibleChoice(CAtlStringW strChoice)
{
    m_arrChoices.Add(strChoice);
}


LRESULT CTedChooserDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetWindowText(m_strTitle);

    m_hChooserCombo = GetDlgItem(IDC_CHOOSER);

    for(DWORD i = 0; i < m_arrChoices.GetCount(); i++)
    {
        ::SendMessage(m_hChooserCombo, CB_ADDSTRING, 0, (LPARAM) m_arrChoices.GetAt(i).GetString());
    }
    
    return 0;
}

LRESULT CTedChooserDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    HRESULT hr;
    int comboLength = ::GetWindowTextLength(m_hChooserCombo);
    
    LPWSTR strCombo = new WCHAR[comboLength + 1];
    CHECK_ALLOC( strCombo );
    
    ::GetWindowText(m_hChooserCombo, strCombo, comboLength + 1);
    m_strChoice = CAtlStringW(strCombo);

    delete[] strCombo;

Cleanup:
    EndDialog(IDOK);

    return 0;
}

LRESULT CTedChooserDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//

LRESULT CTedAboutDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WCHAR szFileName[1024];
    if ( !GetModuleFileName(g_hInst, szFileName, 1024) )
    {
        return GetLastError();
    }

    HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(NULL == hFile)
    {
        return GetLastError();
    }

    FILETIME LastModifiedTime;
    if( !GetFileTime(hFile, NULL, NULL, &LastModifiedTime) )
    {
        CloseHandle(hFile);
        return GetLastError();
    }

    CloseHandle(hFile);

    SYSTEMTIME SystemTime;
    if( !FileTimeToSystemTime(&LastModifiedTime, &SystemTime) )
    {
        return GetLastError();
    }

    SYSTEMTIME LocalTime;
    if( !SystemTimeToTzSpecificLocalTime(NULL, &SystemTime, &LocalTime) )
    {
        return GetLastError();
    }

    WCHAR szVersion[10];
    if( 0 == LoadString(g_hInst, IDS_VERSION, szVersion, 10) )
    {
        return GetLastError();
    }

    CAtlString strVersion;
    strVersion.FormatMessage(IDS_APP_VERSION, szVersion, LocalTime.wMonth, LocalTime.wDay, LocalTime.wYear);
    SetDlgItemText(IDS_VERSION, strVersion.GetString());

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//

CAtlString CTedInputURLDialog::GetURL()
{
    return m_strURL;
}

LRESULT CTedInputURLDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr;
    HWND hInputURLWnd = GetDlgItem(IDC_INPUTURL);

    int iTextLength = ::GetWindowTextLength(hInputURLWnd);

    LPWSTR szURL = new WCHAR[iTextLength + 1];
    CHECK_ALLOC( szURL );

    ::GetWindowText(hInputURLWnd, szURL, iTextLength + 1);
    m_strURL.SetString(szURL);

    delete[] szURL;

Cleanup:
    EndDialog(IDOK);

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//

CTedCaptureSourceDialog::CTedCaptureSourceDialog( bool bVideo )
    : m_ppSourceActivates( NULL )
    , m_dwActivates( 0 )
    , m_dwSelectedIndex( 0 )
{
    EnumCaptureSources( bVideo );
}

CTedCaptureSourceDialog::~CTedCaptureSourceDialog()
{
    //
    // Cleanup
    //
    if ( m_ppSourceActivates )
    {
        for ( DWORD i = 0; i < m_dwActivates; i++ )
        {
            m_ppSourceActivates[ i ]->Release();
        }

        CoTaskMemFree( m_ppSourceActivates );
    }
}

IMFActivate* CTedCaptureSourceDialog::GetSourceActivate()
{
    IMFActivate* pActivate = NULL;

    if ( ( NULL != m_ppSourceActivates ) &&
         ( m_dwSelectedIndex < m_dwActivates ) )
    {
        pActivate = m_ppSourceActivates[ m_dwSelectedIndex ];
        pActivate ->AddRef();
    }

    return pActivate;
}

LRESULT CTedCaptureSourceDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //
    // Fill in the dropdown values based on enumerated activates
    //
    for ( DWORD i = 0; i < m_dwActivates; i++ )
    {
        UINT32 nLen = 0;
        WCHAR* pwsz = NULL;
        HRESULT hr = S_OK;

        //
        // Get the friendly name
        //
        hr = m_ppSourceActivates[ i ]->GetStringLength( MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
                                                        &nLen );

        if ( SUCCEEDED( hr ) )
        {
            pwsz = new WCHAR[ nLen + 1 ];
            if ( pwsz == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = m_ppSourceActivates[ i ]->GetString( MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                                      pwsz,
                                                      nLen + 1,
                                                      &nLen );
        }

        //
        // Add to the dropdown
        //
        if ( SUCCEEDED( hr ) )
        {
            ComboBox_AddString( GetDlgItem( IDC_COMBOSOURCES ), pwsz );
        }
        else
        {
            ComboBox_AddString( GetDlgItem( IDC_COMBOSOURCES ), TEXT("Capture Source") );
        }

        if ( pwsz )
        {
            delete[] pwsz;
        }
    }

    if ( m_dwActivates != 0 )
    {
        //
        // Preselect the first one
        //
        ComboBox_SetCurSel( GetDlgItem( IDC_COMBOSOURCES ), 0 );
    }

    return 0;
}

LRESULT CTedCaptureSourceDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_dwSelectedIndex = ComboBox_GetCurSel( GetDlgItem( IDC_COMBOSOURCES ) );

    EndDialog(IDOK);
    return 0;
}

HRESULT CTedCaptureSourceDialog::EnumCaptureSources( bool bVideo )
{
    HRESULT hr = S_OK;
    IMFAttributes* pAttributes = NULL;

    //
    // Set source type GUID to indicate video capture devices
    // 
    hr = MFCreateAttributes( &pAttributes, 10 );

    if ( SUCCEEDED( hr ) )
    {
        if ( bVideo )
        {
            hr = pAttributes->SetGUID( MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID );
        }
        else
        {
            hr = pAttributes->SetGUID( MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID );
        }
    }

    //
    // Enumerate the capture sources
    //
    if ( SUCCEEDED( hr ) )
    {
        hr = MFEnumDeviceSources( pAttributes,
                                  &m_ppSourceActivates,
                                  (UINT32*)&m_dwActivates );
    }

    //
    // Cleanup
    //
    if ( pAttributes )
    {
        pAttributes->Release();
    }
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//CTedApp
// Main application window controller

CTedApp::CTedApp()
    : m_pPlayer(NULL)
    , m_pVideoWindowHandler(NULL)
    , m_pTopoEventHandler(NULL)
    , m_pMediaEventHandler(NULL)
    , m_pCPM(NULL)
    , m_pPropertyController(NULL)
    , m_pDock(NULL)
    , m_pSplitter(NULL)
    , m_pMainToolbar(NULL)
    , m_fMergeRequired(false)
    , m_fResolved(false)
    , m_fPendingPlay(false)
    , m_fStopTrackingUntilSessionStarted(false)
    , m_pPendingTopo(NULL)
    , m_pTopoView(NULL)
    , m_fCanSeek(false)
{
    CoInitializeEx( NULL, COINIT_MULTITHREADED );
        
    if(FAILED(MFStartup( MF_VERSION ))) 
    {
        assert(false);
    }
}

CTedApp::~CTedApp()
{
    if(m_pTopoView)
    {
        m_pTopoView->CloseTopoWindow();
        m_pTopoView->Release();
    }

    if(m_pVideoWindowHandler) m_pVideoWindowHandler->Release();
    if(m_pTopoEventHandler) m_pTopoEventHandler->Release();
    if(m_pPropertyController) m_pPropertyController->Release();
    if(m_pCPM) m_pCPM->Release();
    

    delete m_pMainToolbar;
    delete m_pDock;
    delete m_pSplitter;
    delete m_pPlayer;
    delete m_pPropSplitter;
    delete m_pMediaEventHandler;
    
    if(m_pPendingTopo) m_pPendingTopo->Release();
    
    MFShutdown();
    CoUninitialize(); 
}

HRESULT CTedApp::Init(LPCWSTR lpCmdLine)
{
    HRESULT hr = S_OK;
            
    // create window
    m_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDC_TRV));
    if(m_hMenu == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    if(Create(NULL, NULL, LoadAtlString(IDS_WDW_NAME), WS_OVERLAPPEDWINDOW, 0, m_hMenu, NULL) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    if(lpCmdLine && lpCmdLine[0] != 0)
    {
        LoadFile(lpCmdLine);
    }

Cleanup:
    if(FAILED(hr)) 
    {
        HandleMMError(LoadAtlString(IDS_E_APP_INIT), hr);
    }

    return hr;
}

void CTedApp::HandleMMError(const CAtlStringW& message, HRESULT errResult)
{
	m_MFErrorHandler.HandleMFError(message, errResult);
}

void CTedApp::NotifySplitterMoved()
{
   // No processing currently needed after splitter movement
}

void CTedApp::NotifyTopoChange()
{
    m_pMainToolbar->MarkResolved(false);
    m_fResolved = false;
}

void CTedApp::HandleSeekerScroll(WORD wPos) 
{
    HRESULT hr = S_OK;

    if(m_pPlayer && m_pPlayer->IsPlaying() && m_fCanSeek) 
    {
        MFTIME duration;
        IFC( m_pPlayer->GetDuration(duration) );

        MFTIME seekTime = (wPos * duration) / m_nSeekerRange;
        IFC( m_pPlayer->PlayFrom(seekTime) );

        m_fStopTrackingUntilSessionStarted = true;
    }

Cleanup:
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_PLAY_MEDIA), hr);
    }
}

void CTedApp::HandleRateScroll(WORD wPos)
{
    HRESULT hr = S_OK;

    if(m_pPlayer && m_pPlayer->IsTopologySet())
    {
        float flRate = wPos / 10.0f;

        if(flRate != 0) 
        {
            hr = m_pPlayer->SetRate(flRate);
        }
        
        m_pMainToolbar->UpdateRateDisplay(flRate);
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_PLAY_RATE), hr);
    }
}

LRESULT CTedApp::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    RECT rect;
    RECT toolRect;

    m_pMediaEventHandler = new CTedAppMediaEventHandler(this);
    CHECK_ALLOC( m_pMediaEventHandler );
    
    // create toolbar
    m_pMainToolbar = new CTedMainToolbar();
    CHECK_ALLOC( m_pMainToolbar );
    IFC( m_pMainToolbar->Init(m_hWnd, MAIN_TOOLBAR_ID) );
    m_pMainToolbar->SetTrackbarScrollCallback(&HandleSeekerScrollFunc);
    m_pMainToolbar->GetRateBar()->SetScrollCallback(&HandleRateScrollFunc);
    m_pMainToolbar->ShowRateBar(SW_HIDE);

    m_pMainToolbar->GetClientRect(&toolRect);
    GetClientRect(&rect);
    rect.top = toolRect.bottom;
    rect.bottom = rect.bottom - 100;
    
    m_pDock = new CDock;
    CHECK_ALLOC( m_pDock );
    if(m_pDock->Create(m_hWnd, rect, LoadAtlString(IDS_DOCK), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    ZeroMemory(&rect, sizeof(rect));

    m_pSplitter = new CSplitterBar(m_pDock, false, m_hWnd);
    CHECK_ALLOC( m_pSplitter );
    if(m_pSplitter->Create(m_pDock->m_hWnd, &rect, LoadAtlString(IDS_SPLITTER)) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    m_pPropSplitter = new CSplitterBar(m_pDock, true, m_hWnd);
    CHECK_ALLOC( m_pPropSplitter );
    if(m_pPropSplitter->Create(m_pDock->m_hWnd, &rect, LoadAtlString(IDS_SPLITTER), WS_CHILD | WS_CLIPSIBLINGS) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    rect.top += 5;
    CPropertyEditWindow* pPropView = new CPropertyEditWindow();
    CHECK_ALLOC( pPropView );
    if(pPropView->Create(m_pDock->m_hWnd, rect, LoadAtlString(IDS_PROP_VIEW), WS_CHILD | WS_BORDER | WS_VISIBLE) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }
    
    m_pCPM = new CTedContentProtectionManager(this);
    CHECK_ALLOC( m_pCPM );
    m_pCPM->AddRef();
        
    m_pPlayer = new CTedPlayer(m_pMediaEventHandler, m_pCPM);
    CHECK_ALLOC( m_pPlayer );
    
    m_pVideoWindowHandler = new CTedAppVideoWindowHandler(m_hWnd);
    CHECK_ALLOC( m_pVideoWindowHandler );
    m_pVideoWindowHandler->AddRef();

    m_pTopoEventHandler = new CTedAppTopoEventHandler(this);
    CHECK_ALLOC( m_pTopoEventHandler );
    m_pTopoEventHandler->AddRef();
    
    m_pPropertyController = new CPropertyController(pPropView);
    CHECK_ALLOC( m_pPropertyController );
    m_pPropertyController->AddRef();
    
    IFC( TEDCreateTopoViewer(m_pVideoWindowHandler, m_pPropertyController, m_pTopoEventHandler, &m_pTopoView) );
    IFC( m_pTopoView->CreateTopoWindow(LoadAtlString(IDS_WDW_TOPOVIEW), WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, (LONG_PTR) m_pDock->m_hWnd, (LONG_PTR*) &m_hEditWnd) );

    m_EditWindow.Attach(m_hEditWnd);

    RebuildDockWithOneView();

    m_MFErrorHandler.SetParentWnd(m_hWnd);
    
    HICON hIcon = ::LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TRV));
    SetIcon(hIcon, TRUE);
    SetIcon(hIcon, FALSE);
    
    EnableInput(ID_PLAY_PLAY, TRUE);

Cleanup:

    return (LRESULT) hr;
}

LRESULT CTedApp::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PostQuitMessage(0);
    return 0;
}

LRESULT CTedApp::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CTedApp::OnMediaCapabilitiesChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DWORD dwCaps;
    m_pPlayer->GetCapabilities(&dwCaps);

    if(dwCaps & MFSESSIONCAP_SEEK)
    {
        m_fCanSeek = true;
    }
    else
    {
        m_fCanSeek = false;
    }

    return 0;
}

LRESULT CTedApp::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pMainToolbar->SetWindowPos(NULL, 0, 0, LOWORD(lParam), 30,
        SWP_NOZORDER | SWP_NOREDRAW);
    
    m_pDock->SetWindowPos(NULL, 0, 30, LOWORD(lParam), HIWORD(lParam) - 30, 
                    SWP_NOZORDER | SWP_NOREDRAW);

    return 0;
}

LRESULT CTedApp::OnSessionPlay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HRESULT hr = (HRESULT) wParam;

    KillTimer(ms_nTimerID);
    if(SUCCEEDED(hr))
    {
        SetTimer(ms_nTimerID, ms_dwTimerLen, NULL);
    }

    m_fStopTrackingUntilSessionStarted = false;
    
    return 0;
}

LRESULT CTedApp::OnTopologySet(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HRESULT hrTopologySet = (HRESULT) wParam;
    HandleTopologySet(hrTopologySet);
    
    return 0;
}

LRESULT CTedApp::OnTopologyReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    float flSlowest, flFastest;
    hr = m_pPlayer->GetRateBounds(MFRATE_FORWARD, &flSlowest, &flFastest);

    if(FAILED(hr) || flSlowest == 0.0f && flFastest == 0.0f)
    {
        m_pMainToolbar->ShowRateBar(SW_HIDE);
    }
    else
    {
        m_pMainToolbar->ShowRateBar(SW_SHOW);
        m_pMainToolbar->GetRateBar()->SetRange(DWORD(flSlowest * 10), DWORD(flFastest * 10));
        m_pMainToolbar->GetRateBar()->SetPos(10);

        m_pMainToolbar->GetRateBar()->SendMessage(TBM_CLEARTICS, 0, 0);

        for(DWORD dwTicPos = 10; dwTicPos < flFastest * 10; dwTicPos += 10)
        {
            m_pMainToolbar->GetRateBar()->SendMessage(TBM_SETTIC, 0, dwTicPos);
        }
    }
    
    return 0;
}

LRESULT CTedApp::OnSessionEnded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    m_pVideoWindowHandler->ShowWindows(SW_HIDE);

    EnableInput(ID_PLAY_PLAY, TRUE);
    EnableInput(ID_PLAY_STOP, FALSE);
    EnableInput(ID_PLAY_PAUSE, FALSE);

    MFTIME duration;
    m_pPlayer->GetDuration(duration);
    
    m_pMainToolbar->UpdateTimeDisplay(0, duration);

    KillTimer(ms_nTimerID);
    
    return 0;
}

LRESULT CTedApp::OnSplitterMoved(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    NotifySplitterMoved();
    
    return 0;
}

LRESULT CTedApp::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(wParam == ms_nTimerID)
    {
        if(m_pPlayer->IsPlaying())
        {
            MFTIME time, duration;
            HRESULT hr = m_pPlayer->GetTime(&time);
            HRESULT hr2 = m_pPlayer->GetDuration(duration);

            if(SUCCEEDED(hr) && SUCCEEDED(hr2) && !m_fStopTrackingUntilSessionStarted)
            {
                m_pMainToolbar->UpdateTimeDisplay(time, duration);
            }
        }
    }

    SetTimer(ms_nTimerID, ms_dwTimerLen, NULL);
    return 0;
}

LRESULT CTedApp::OnUntrustedComponent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    MessageBox(LoadAtlString(IDS_E_UNTRUSTED_COMPONENTS), NULL, MB_OK);
    
    return 0;
}

LRESULT CTedApp::OnProtectedContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = m_pCPM->ManualEnableContent();

    if(FAILED(hr))
    {
        m_MFErrorHandler.HandleMFError(LoadAtlString(IDS_E_MANUAL_LICENSE), hr);
    }
        
    return 0;
}

LRESULT CTedApp::OnIndividualization(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = m_pCPM->Individualize();

    if(FAILED(hr))
    {
        m_MFErrorHandler.HandleMFError(LoadAtlString(IDS_E_PROT_CONTENT_PLAYBACK), hr);
    }

    return 0;
}

void CTedApp::HandleTopologySet(HRESULT hrTopologySet)
{
    assert(m_pPlayer != NULL);
    
    CComPtr<IMFTopology> spFullTopo;
    CAtlStringW strTime;

    HRESULT hr = S_OK;

    if(FAILED(hrTopologySet))
    {
        // We failed to set the topology; merge the old topology
        m_pTopoView->MergeTopology(m_pPendingTopo);
        
        HandleMMError(LoadAtlString(IDS_E_TOPO_RESOLUTION), hrTopologySet);
        m_fPendingPlay = false;
        return;
    }
    
    if(m_fMergeRequired)
    {
        hr = m_pPlayer->GetFullTopology(&spFullTopo);
        
        if(spFullTopo != NULL) 
        {
            TOPOID TopoID = 0;
            spFullTopo->GetTopologyID(&TopoID);
            
            // Ensure this is the topology that was most recently set on the player
            if(TopoID == m_PendingTopoID)
            {
                m_fMergeRequired = false;
                hr = m_pTopoView->MergeTopology(spFullTopo);

                if(FAILED(hr))
                {
                    HandleMMError(LoadAtlString(IDS_E_TOPO_MERGE), hr);
                }
                else
                {
                    m_pMainToolbar->MarkResolved(true);
                    m_fResolved = true;

                    if(m_fPendingPlay)
                    {
                        hr = Play();
                        if(FAILED(hr))
                        {
                            HandleMMError(LoadAtlString(IDS_E_TOPO_PLAY), hr);
                        }
                    }
                }
            }
        }
        else
        {
            HandleMMError(LoadAtlString(IDS_E_TOPO_RETRIEVE), hr);
        }
    }

    MFTIME duration = 0;
    m_pPlayer->GetDuration(duration);

    m_pMainToolbar->UpdateTimeDisplay(0, duration);
    m_fPendingPlay = false;
}

LRESULT CTedApp::OnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CAtlString strFilter = LoadAtlString(IDS_FILE_XML); 
    CAtlString strTitle = LoadAtlString(IDS_FILE_LOAD); 
    strFilter.SetAt(strFilter.GetLength()-1,0); // force double-null termination
    strFilter.SetAt(strFilter.GetLength()-2,0);

    TCHAR fileBuffer[m_dwMaxAcceptedFileNameLength];
    fileBuffer[0] = 0;

    OPENFILENAME openFileInfo;
    openFileInfo.lStructSize = sizeof(OPENFILENAME);
    openFileInfo.hwndOwner = m_hWnd;
    openFileInfo.hInstance = 0;
    openFileInfo.lpstrFilter = strFilter;
    openFileInfo.lpstrCustomFilter = NULL;
    openFileInfo.nFilterIndex = 1;
    openFileInfo.lpstrFile = fileBuffer;
    openFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
    openFileInfo.lpstrFileTitle = NULL;
    openFileInfo.nMaxFileTitle = 0;
    openFileInfo.lpstrInitialDir = NULL;
    openFileInfo.lpstrTitle = strTitle;
    openFileInfo.Flags = 0;
    openFileInfo.nFileOffset = 0;
    openFileInfo.nFileExtension = 0;
    openFileInfo.lpstrDefExt = NULL;
    openFileInfo.lCustData = NULL;
    openFileInfo.pvReserved = NULL;
    openFileInfo.dwReserved = 0;
    openFileInfo.FlagsEx = 0;

    if(GetOpenFileName(&openFileInfo))
    {
        LoadFile(openFileInfo.lpstrFile);
    }

    return 0;
}

LRESULT CTedApp::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CAtlString strFilter = LoadAtlString(IDS_FILE_XML); 
    CAtlString strTitle = LoadAtlString(IDS_FILE_SAVE); 
    strFilter.SetAt(strFilter.GetLength()-1,0); // force double-null termination
    strFilter.SetAt(strFilter.GetLength()-2,0);

    TCHAR fileBuffer[m_dwMaxAcceptedFileNameLength];
    fileBuffer[0] = 0;

    OPENFILENAME openFileInfo;
    openFileInfo.lStructSize = sizeof(OPENFILENAME);
    openFileInfo.hwndOwner = m_hWnd;
    openFileInfo.hInstance = 0;
    openFileInfo.lpstrFilter = strFilter;
    openFileInfo.lpstrCustomFilter = NULL;
    openFileInfo.nFilterIndex = 1;
    openFileInfo.lpstrFile = fileBuffer;
    openFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
    openFileInfo.lpstrFileTitle = NULL;
    openFileInfo.nMaxFileTitle = 0;
    openFileInfo.lpstrInitialDir = NULL;
    openFileInfo.lpstrTitle = strTitle;
    openFileInfo.Flags = 0;
    openFileInfo.nFileOffset = 0;
    openFileInfo.nFileExtension = 0;
    openFileInfo.lpstrDefExt = L"XML";
    openFileInfo.lCustData = NULL;
    openFileInfo.pvReserved = NULL;
    openFileInfo.dwReserved = 0;
    openFileInfo.FlagsEx = 0;

    if(GetSaveFileName(&openFileInfo)) 
    {
        HRESULT hr = m_pTopoView->SaveTopology(openFileInfo.lpstrFile);
        if(FAILED(hr)) 
        {
            HandleMMError(LoadAtlString(IDS_E_FILE_SAVE), hr);
        }
    }

    return 0;
}

LRESULT CTedApp::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    m_pTopoView->DeleteSelectedNode();

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnNewTopology(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    BOOL fIsSaved = FALSE;
    m_pTopoView->IsSaved(&fIsSaved);
    
    if(!fIsSaved)
    {
        int iMBResult = MessageBox(LoadAtlString(IDS_TOPO_NOT_SAVED), LoadAtlString(IDS_TOPO_NEW), MB_YESNO);

        if(iMBResult == IDNO) return 0;
    }

    m_pTopoView->NewTopology();

    ResetInterface();

    return 0;
}

LRESULT CTedApp::OnAddSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CAtlString strFilter = LoadAtlString(IDS_FILE_MEDIA); 
    CAtlString strTitle = LoadAtlString(IDS_FILE_SELECT); 
    strFilter.SetAt(strFilter.GetLength()-1,0); // force double-null termination
    strFilter.SetAt(strFilter.GetLength()-2,0);

    TCHAR fileBuffer[m_dwMaxAcceptedFileNameLength];
    fileBuffer[0] = 0;

    OPENFILENAME openFileInfo;
    openFileInfo.lStructSize = sizeof(OPENFILENAME);
    openFileInfo.hwndOwner = m_hWnd;
    openFileInfo.hInstance = 0;
    openFileInfo.lpstrFilter = strFilter;
    openFileInfo.lpstrCustomFilter = NULL;
    openFileInfo.nFilterIndex = 1;
    openFileInfo.lpstrFile = fileBuffer;
    openFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
    openFileInfo.lpstrFileTitle = NULL;
    openFileInfo.nMaxFileTitle = 0;
    openFileInfo.lpstrInitialDir = NULL;
    openFileInfo.lpstrTitle = strTitle;
    openFileInfo.Flags = OFN_FILEMUSTEXIST;
    openFileInfo.nFileOffset = 0;
    openFileInfo.nFileExtension = 0;
    openFileInfo.lpstrDefExt = NULL;
    openFileInfo.lCustData = NULL;
    openFileInfo.pvReserved = NULL;
    openFileInfo.dwReserved = 0;
    openFileInfo.FlagsEx = 0;

    if(GetOpenFileName(&openFileInfo))
    {
        HRESULT hr = m_pTopoView->AddSource(openFileInfo.lpstrFile);

        if(FAILED(hr))
        {
            HandleMMError(LoadAtlString(IDS_E_SOURCE_CREATE), hr);
        }
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnAddSink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    // For future use
    
    return 0;
}

LRESULT CTedApp::OnAddSAR(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = m_pTopoView->AddSAR();
    
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_AUDIO_RENDERER_CREATE), hr);
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnAddEVR(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = m_pTopoView->AddEVR();
    
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_VIDEO_RENDERER_CREATE), hr);
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnAddTransform(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;

    CTedTransformDialog transformDialog;

    if(transformDialog.DoModal() == IDOK)
    {
        hr = m_pTopoView->AddTransformActivate(transformDialog.GetChosenActivate());
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_TRANSFORM_CREATE), hr);
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnAddTee(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = m_pTopoView->AddTee();

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_TEE_CREATE), hr);
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnAddCustomMFT(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CTedInputGuidDialog dialog;
    HRESULT hr = S_OK;
    
    if(dialog.DoModal() == IDOK)
    {
        GUID gidTransID = dialog.GetInputGuid();
        hr = m_pTopoView->AddTransform(gidTransID, LoadAtlString(IDS_MFT_CUSTOM));
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_TRANSFORM_CREATE), hr);
    }

    return 0;
}

LRESULT CTedApp::OnAddCustomSink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CTedInputGuidDialog dialog;
    HRESULT hr = S_OK;
    
    if(dialog.DoModal() == IDOK)
    {
        GUID gidSinkID = dialog.GetInputGuid();
        hr = m_pTopoView->AddCustomSink(gidSinkID);
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_SINK_CREATE), hr);
    }

    return 0; 
}

LRESULT CTedApp::OnAddVideoCaptureSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    IMFMediaSource* pSource = NULL;
    CTedCaptureSourceDialog dialog( true );

    if ( dialog.DoModal() == IDOK )
    {
        IMFActivate* pActivate = dialog.GetSourceActivate();
        if ( pActivate )
        {
            hr = pActivate->ActivateObject( IID_IMFMediaSource, (void**)&pSource );

            if ( SUCCEEDED( hr ) )
            {
                hr = m_pTopoView->AddCaptureSource( pSource );
            }
        }

        if ( pActivate )
        {
            pActivate->Release();
        }
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_VIDEO_CAP_SOURCE_CREATE), hr);
    }

    return 0;
}

LRESULT CTedApp::OnAddAudioCaptureSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    IMFMediaSource* pSource = NULL;
    CTedCaptureSourceDialog dialog( false );

    if ( dialog.DoModal() == IDOK )
    {
        IMFActivate* pActivate = dialog.GetSourceActivate();
        if ( pActivate )
        {
            hr = pActivate->ActivateObject( IID_IMFMediaSource, (void**)&pSource );

            if ( SUCCEEDED( hr ) )
            {
                hr = m_pTopoView->AddCaptureSource( pSource );
            }
        }

        if ( pActivate )
        {
            pActivate->Release();
        }
    }

    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_AUDIO_CAP_SOURCE_CREATE), hr);
    }

    return 0;
}

LRESULT CTedApp::OnLoadTopology(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;

    hr = ResolveTopologyFromEditor();
    
    if(FAILED(hr)) 
    {
        HandleMMError(LoadAtlString(IDS_E_TOPO_RESOLUTION), hr);
    }

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnActionPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;

    if(!m_fResolved)
    {
        m_fPendingPlay = true;
        hr = ResolveTopologyFromEditor();
        if(FAILED(hr))
        {
            m_fPendingPlay = false;

            HandleMMError(LoadAtlString(IDS_E_TOPO_RESOLUTION), hr);
        }

        return 0;
    }

    hr = Play();

    if(FAILED(hr)) 
    {
        HandleMMError(LoadAtlString(IDS_E_TOPO_PLAY), hr);
    }
    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnActionStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr;
    
    IFC(m_pPlayer->Stop());
    
    m_pVideoWindowHandler->ShowWindows(SW_HIDE);
    
    EnableInput(ID_PLAY_PLAY, TRUE);
    EnableInput(ID_PLAY_STOP, FALSE);
    EnableInput(ID_PLAY_PAUSE, FALSE);

    MFTIME duration;
    m_pPlayer->GetDuration(duration);
    
    m_pMainToolbar->UpdateTimeDisplay(0, duration);
    
    KillTimer(ms_nTimerID);
    
Cleanup:

    bHandled = TRUE;
    return 0;
}

LRESULT CTedApp::OnActionPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    HRESULT hr;

    IFC( m_pPlayer->Pause() );

    EnableInput(ID_PLAY_PLAY, TRUE);
    EnableInput(ID_PLAY_STOP, TRUE);
    EnableInput(ID_PLAY_PAUSE, FALSE);

Cleanup:
	bHandled = TRUE;
	return 0;
}

LRESULT CTedApp::OnSpy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = m_pTopoView->SpySelectedNode();

    if(FAILED(hr))
    {
        m_MFErrorHandler.HandleMFError(LoadAtlString(IDS_E_NODE_SPY), hr);
    }

    return 0;
}

LRESULT CTedApp::OnCustomTopoloader(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CTedInputGuidDialog dialog;
    HRESULT hr = S_OK;
    
    if(dialog.DoModal() == IDOK && dialog.IsValidGuid())
    {
        CComPtr<IMFTopoLoader> spTopoLoader;
        GUID gidTopoloader = dialog.GetInputGuid();
        
        // Test creation of a topoloader with this CLSID to ensure it is valid, rather than
        // fail when creating the session in a seemingly unrelated error.
        hr = CoCreateInstance(gidTopoloader, NULL, CLSCTX_INPROC_SERVER, IID_IMFTopoLoader, (void **)&spTopoLoader);
        if(FAILED(hr))
        {
            HandleMMError(LoadAtlString(IDS_E_TOPOLOADER_CREATE), hr);
        }
        else
        {
            m_pPlayer->SetCustomTopoloader(gidTopoloader);
        }
    }

    return 0;
}

LRESULT CTedApp::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    SendMessage(WM_CLOSE, 0, 0);

    return 0;
}

LRESULT CTedApp::OnHelpHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HINSTANCE hReturned = ShellExecute(NULL, L"open", L"http://go.microsoft.com/fwlink/?LinkId=92748", NULL, NULL, SW_SHOW);
    
    if(hReturned <= HINSTANCE(32))
    {
        MessageBox(LoadAtlString(IDS_E_URL_OPEN), LoadAtlString(IDS_ERROR), MB_OK);
    }
    
    return 0;
}

LRESULT CTedApp::OnHelpAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    CTedAboutDialog dialog;
    dialog.DoModal();
    
    return 0;
}

LRESULT CTedApp::OnRenderFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
{
    HRESULT hr = S_OK;
    CAtlString strFilter = LoadAtlString(IDS_FILE_MEDIA); 
    CAtlString strTitle = LoadAtlString(IDS_FILE_SELECT); 
    strFilter.SetAt(strFilter.GetLength()-1,0); // force double-null termination
    strFilter.SetAt(strFilter.GetLength()-2,0);

    TCHAR fileBuffer[m_dwMaxAcceptedFileNameLength];
    fileBuffer[0] = 0;

    OPENFILENAME openFileInfo;
    openFileInfo.lStructSize = sizeof(OPENFILENAME);
    openFileInfo.hwndOwner = m_hWnd;
    openFileInfo.hInstance = 0;
    openFileInfo.lpstrFilter = strFilter;
    openFileInfo.lpstrCustomFilter = NULL;
    openFileInfo.nFilterIndex = 1;
    openFileInfo.lpstrFile = fileBuffer;
    openFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
    openFileInfo.lpstrFileTitle = NULL;
    openFileInfo.nMaxFileTitle = 0;
    openFileInfo.lpstrInitialDir = NULL;
    openFileInfo.lpstrTitle = strTitle;
    openFileInfo.Flags = OFN_FILEMUSTEXIST;
    openFileInfo.nFileOffset = 0;
    openFileInfo.nFileExtension = 0;
    openFileInfo.lpstrDefExt = NULL;
    openFileInfo.lCustData = NULL;
    openFileInfo.pvReserved = NULL;
    openFileInfo.dwReserved = 0;
    openFileInfo.FlagsEx = 0;

    if(GetOpenFileName(&openFileInfo))
    {
        CComPtr<IMFTopology> spTopology;
        CComPtr<IMFTopology> spTedTopo;
        
        HRESULT hrConstructor;
        CTedMediaFileRenderer TedMediaFileRenderer(openFileInfo.lpstrFile, m_pVideoWindowHandler, hrConstructor);
        IFC( hrConstructor );
        
        IFC( TedMediaFileRenderer.Load(&spTopology) );
        IFC( m_pTopoView->ShowTopology(spTopology, openFileInfo.lpstrFile) );
        
        ResetInterface();
        
        BOOL fIsProtected;
        IFC( m_pTopoView->GetTopology(&spTedTopo, &fIsProtected) );
        IFC( SetTopologyOnPlayer(spTedTopo, fIsProtected, FALSE) );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( CommDlgExtendedError() );
    }

Cleanup:
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_MEDIA_RENDER), hr);
    }
    
    return 0;
}

LRESULT CTedApp::OnRenderURL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    CTedInputURLDialog dialog;

    if(dialog.DoModal() == IDOK)
    {
        CComPtr<IMFTopology> spTopology;
        CComPtr<IMFTopology> spTedTopo;
        
        HRESULT hrConstructor;
        CTedMediaFileRenderer TedMediaFileRenderer(dialog.GetURL(), m_pVideoWindowHandler, hrConstructor);
        IFC( hrConstructor );
        
        IFC( TedMediaFileRenderer.Load(&spTopology) );
        IFC( m_pTopoView->ShowTopology(spTopology, dialog.GetURL()) );
        
        ResetInterface();
        
        BOOL fIsProtected;
        IFC( m_pTopoView->GetTopology(&spTedTopo, &fIsProtected) );
        IFC( SetTopologyOnPlayer(spTedTopo, fIsProtected, FALSE) );
    }
    
Cleanup:
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_MEDIA_RENDER), hr);
    }
    
    return 0;
}

LRESULT CTedApp::OnRenderTranscode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopology> spTopology;
    CComPtr<IMFTopology> spTedTopo;
    TCHAR fileBuffer[m_dwMaxAcceptedFileNameLength];
    fileBuffer[0] = 0;

    OPENFILENAME openFileInfo;
    ZeroMemory(&openFileInfo, sizeof(OPENFILENAME));
    openFileInfo.lStructSize = sizeof(OPENFILENAME);
    openFileInfo.hwndOwner = m_hWnd;
    openFileInfo.hInstance = 0;
    openFileInfo.lpstrFilter = L"Media Files\0*.*\0";
    openFileInfo.lpstrCustomFilter = NULL;
    openFileInfo.nFilterIndex = 1;
    openFileInfo.lpstrFile = fileBuffer;
    openFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
    openFileInfo.lpstrFileTitle = NULL;
    openFileInfo.nMaxFileTitle = 0;
    openFileInfo.lpstrInitialDir = NULL;
    openFileInfo.lpstrTitle = L"Select Media Source";
    openFileInfo.Flags = OFN_FILEMUSTEXIST;
    openFileInfo.nFileOffset = 0;
    openFileInfo.nFileExtension = 0;
    openFileInfo.lpstrDefExt = NULL;
    openFileInfo.lCustData = NULL;
    openFileInfo.pvReserved = NULL;
    openFileInfo.dwReserved = 0;
    openFileInfo.FlagsEx = 0;

    if(GetOpenFileName(&openFileInfo))
    {
        CTedChooserDialog ChooserDialog(L"Choose Transcode Profile");

        CTedTranscodeTopologyBuilder Builder(openFileInfo.lpstrFile, &hr);
        IFC( hr );

        size_t cProfiles = Builder.GetProfileCount();
        for(size_t i = 0; i < cProfiles; i++)
        {
            ChooserDialog.AddPossibleChoice(Builder.GetProfileName(i));
        }

        if(ChooserDialog.DoModal() == IDOK)
        {
            TCHAR fileBuffer2[m_dwMaxAcceptedFileNameLength];
            fileBuffer2[0] = 0;

            OPENFILENAME targetFileInfo;
            targetFileInfo.lStructSize = sizeof(OPENFILENAME);
            targetFileInfo.hwndOwner = m_hWnd;
            targetFileInfo.hInstance = 0;
            targetFileInfo.lpstrFilter = L"Media Files\0*.*\0";
            targetFileInfo.lpstrCustomFilter = NULL;
            targetFileInfo.nFilterIndex = 1;
            targetFileInfo.lpstrFile = fileBuffer2;
            targetFileInfo.nMaxFile = m_dwMaxAcceptedFileNameLength;
            targetFileInfo.lpstrFileTitle = NULL;
            targetFileInfo.nMaxFileTitle = 0;
            targetFileInfo.lpstrInitialDir = NULL;
            targetFileInfo.lpstrTitle = L"Choose target file";
            targetFileInfo.Flags = OFN_OVERWRITEPROMPT;
            targetFileInfo.nFileOffset = 0;
            targetFileInfo.nFileExtension = 0;
            targetFileInfo.lpstrDefExt = NULL;
            targetFileInfo.lCustData = NULL;
            targetFileInfo.pvReserved = NULL;
            targetFileInfo.dwReserved = 0;
            targetFileInfo.FlagsEx = 0;

            if(GetSaveFileName(&targetFileInfo)) 
            {
                IFC( Builder.BuildTranscodeTopology(ChooserDialog.GetChoice(), targetFileInfo.lpstrFile, &spTopology) );

                IFC( m_pTopoView->ShowTopology(spTopology, openFileInfo.lpstrFile) );
            
                ResetInterface();
            
                BOOL fIsProtected;
                IFC( m_pTopoView->GetTopology(&spTedTopo, &fIsProtected) );
                IFC( SetTopologyOnPlayer(spTedTopo, fIsProtected, TRUE) );
            }
            else
            {
                hr = HRESULT_FROM_WIN32( CommDlgExtendedError() );
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

Cleanup:
    if(FAILED(hr))
    {
        HandleMMError(L"Error rendering transcode topology", hr);
    }

    return 0;
}

void CTedApp::EnableInput(UINT item, BOOL enabled)
{
    m_pMainToolbar->EnableButtonByCommand(item, enabled);
    if(enabled)
    {
        EnableMenuItem(m_hMenu, item, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(m_hMenu, item, MF_GRAYED);
    }
}

void CTedApp::RebuildDockWithOneView()
{
    m_pDock->RemoveAllAreas();

    CDock::CArea* pEditArea = m_pDock->AddArea(CDock::MOVE_NO);
    CDock::CArea* pPropertiesArea = m_pDock->AddArea(CDock::MOVE_NO);
    CDock::CArea* pVerticalSplit = m_pDock->AddArea(CDock::MOVE_VERTICAL);

    // Create the splitter between the properties view and the edit view
    pVerticalSplit->m_Attach.pTop = m_pDock->GetStockArea(CDock::STOCK_AREA_TOP);
    pVerticalSplit->m_Attach.pBottom = m_pDock->GetStockArea(CDock::STOCK_AREA_BOTTOM);
    pVerticalSplit->m_pWindow = m_pPropSplitter;

    pVerticalSplit->m_posFixed.left = m_dblInitialSplitterPos;
    pVerticalSplit->m_posFixed.width = m_dwSplitterWidth;

    // Create the edit view 
    pEditArea->m_Attach.pLeft = m_pDock->GetStockArea(CDock::STOCK_AREA_LEFT);
    pEditArea->m_Attach.pRight = pVerticalSplit;
    pEditArea->m_Attach.pTop = m_pDock->GetStockArea(CDock::STOCK_AREA_TOP);
    pEditArea->m_Attach.pBottom = m_pDock->GetStockArea(CDock::STOCK_AREA_BOTTOM);
    pEditArea->m_pWindow = &m_EditWindow; //m_pEditorView;

    // Create the properties view
    pPropertiesArea->m_Attach.pLeft = pVerticalSplit;
    pPropertiesArea->m_Attach.pRight = m_pDock->GetStockArea(CDock::STOCK_AREA_RIGHT);
    pPropertiesArea->m_Attach.pTop = m_pDock->GetStockArea(CDock::STOCK_AREA_TOP);
    pPropertiesArea->m_Attach.pBottom = m_pDock->GetStockArea(CDock::STOCK_AREA_BOTTOM);
    pPropertiesArea->m_pWindow = m_pPropertyController->GetWindow();

    m_pPropSplitter->ShowWindow(SW_SHOW);
    m_pPropertyController->GetWindow()->ShowWindow(SW_SHOW);
    m_pSplitter->ShowWindow(SW_HIDE);
    
    m_pDock->UpdateDock();
}

void CTedApp::DisablePlayback()
{
    EnableInput(ID_PLAY_PLAY, FALSE);
    EnableInput(ID_PLAY_STOP, FALSE);
    EnableInput(ID_PLAY_PAUSE, FALSE);
}

HRESULT CTedApp::HasBuggedPins(IMFTopology* pTopology, bool* fBuggedPins)
{
    HRESULT hr;
    *fBuggedPins = false;
    WORD cNodes;
    IFC( pTopology->GetNodeCount(&cNodes) );
    for(WORD i = 0; i < cNodes && !(*fBuggedPins); i++)
    {
        CComPtr<IMFTopologyNode> spNode;
        IFC( pTopology->GetNode(i, &spNode) );
        
        DWORD cInputs;
        IFC( spNode->GetInputCount(&cInputs) )
        for(DWORD j = 0; j < cInputs; j++)
        {
            CComPtr<IMFTopologyNode> spUpNode;
            DWORD dwUpIndex;
            HRESULT hrPin = spNode->GetInput(j, &spUpNode, &dwUpIndex);
            if(FAILED(hrPin))
            {
                *fBuggedPins = true;
                break;
            }
        }
        
        if(!(*fBuggedPins))
        {
            DWORD cOutputs;
            IFC( spNode->GetOutputCount(&cOutputs) );
            for(DWORD j = 0; j < cOutputs; j++)
            {
                CComPtr<IMFTopologyNode> spDownNode;
                DWORD dwDownIndex;
                HRESULT hrPin = spNode->GetOutput(j, &spDownNode, &dwDownIndex);
                if(FAILED(hrPin))
                {
                    *fBuggedPins = true;
                    break;
                }
            }
        }
    }
    
Cleanup:
    return hr;
}

void CTedApp::ResetInterface()
{
    if(m_pPlayer && m_pPlayer->IsPlaying())
    {
        m_pPlayer->Stop();
    }
    
    m_pMainToolbar->UpdateTimeDisplay(0, 0);
    m_pMainToolbar->ShowRateBar(SW_HIDE);
    m_pMainToolbar->MarkResolved(false);
    m_fResolved = false;
}

HRESULT CTedApp::ResolveTopologyFromEditor()
{
    HRESULT hr;
    BOOL fIsProtected;

    CComPtr<IMFTopology> spTopo;
    IFC( m_pTopoView->GetTopology(&spTopo, &fIsProtected) );

    // WORKAROUND: Currently, disconnected pins are bugged in media foundation.
    // The pin is disconnected, but the node will still have an input/output count of
    // one.  When trying to resolve a topology with a bugged pin like this, the
    // topoloader will create a work item with a 'NULL' destination node.
    // Executing this work item causes E_POINTER to be returned to the client.
    // Since E_POINTER make little sense from the context of the UI, handle
    // this issue here.
    bool fHasBuggedPins;
    IFC( HasBuggedPins(spTopo, &fHasBuggedPins) );
    if(fHasBuggedPins)
    {
        MessageBox(LoadAtlString(IDS_E_TOPO_RESOLUTION_PINS), LoadAtlString(IDS_ERROR), MB_OK);
        goto Cleanup;
    }
    
    IFC( SetTopologyOnPlayer(spTopo, fIsProtected, FALSE) );

Cleanup:
    return hr;
}

HRESULT CTedApp::SetTopologyOnPlayer(IMFTopology* pTopo, BOOL fIsProtected, BOOL fIsTranscode)
{
    HRESULT hr;
    
    if(m_pPendingTopo) m_pPendingTopo->Release();
    m_pPendingTopo = pTopo;
    m_pPendingTopo->AddRef();
    
    m_pPendingTopo->GetTopologyID(&m_PendingTopoID);
    
    if(fIsProtected)
    {
        hr = m_pPlayer->InitProtected();
    }
    else
    {
        hr = m_pPlayer->InitClear();
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pPlayer->SetTopology(pTopo, fIsTranscode);
        if(FAILED(hr))
        {
            if(hr == MF_E_NOT_FOUND)
            {
                MessageBox(LoadAtlString(IDS_E_TOPO_RESOLUTION_NO_SOURCE), LoadAtlString(IDS_ERROR), MB_OK);
                hr = S_OK;
                m_fPendingPlay = false;
            }
            
            goto Cleanup;
        }        
        m_fMergeRequired = true;
    }
    else
    {
        HandleMMError(LoadAtlString(IDS_E_TOPO_RESOLUTION_INIT), hr);
        hr = S_OK;
    }
    
Cleanup:
    return hr;
}

HRESULT CTedApp::Play()
{
    HRESULT hr;

    m_pVideoWindowHandler->ShowWindows(SW_SHOW);

    if(m_fCanSeek)
    {
        MFTIME duration;
        WORD wPos = m_pMainToolbar->GetSeekBar()->GetPos();

        IFC( m_pPlayer->GetDuration(duration) );
        MFTIME seekTime = (wPos * duration) / m_nSeekerRange;
        IFC( m_pPlayer->PlayFrom(seekTime) );
    }
    else
    {
        IFC( m_pPlayer->Start() );
    }

    EnableInput(ID_PLAY_PLAY, FALSE);
    EnableInput(ID_PLAY_STOP, TRUE);
    EnableInput(ID_PLAY_PAUSE, TRUE);

    SetTimer(ms_nTimerID, ms_dwTimerLen, NULL);

Cleanup:
    return hr;
}

void CTedApp::LoadFile(LPCWSTR szFile)
{
    HRESULT hr = m_pTopoView->LoadTopology(szFile);
    if(FAILED(hr))
    {
        HandleMMError(LoadAtlString(IDS_E_FILE_LOAD_XML), hr);
    }
}

void HandleSeekerScrollFunc(WORD wPos) 
{
    g_pApp->HandleSeekerScroll(wPos);
}

void HandleRateScrollFunc(WORD wPos)
{
    g_pApp->HandleRateScroll(wPos);
}

///////////////////////////////////////////////////////////////////////////////
//
HRESULT InitTedApp(LPCWSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;

    g_pApp = new CTedApp;
    if(g_pApp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    IFC(g_pApp->Init(lpCmdLine));

    /* Make the window visible; update its client area; and return "success" */
    g_pApp->ShowWindow(nCmdShow);
    g_pApp->UpdateWindow();

Cleanup:

    return hr;
}

BOOL WINAPI wWinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in_opt LPWSTR lpCmdLine,
    __in int nCmdShow
    )
{
    HRESULT hr = S_OK;
    MSG  msg;
    HACCEL hAccelTable;
    INITCOMMONCONTROLSEX  iccex;

    g_hInst = hInstance;

    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    
    g_hTedUtilInst = LoadLibraryEx(L"TEDUTIL.dll", NULL, 0);

    // Initialize common controls
    iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    iccex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&iccex);

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRV));

    /* Perform initializations that apply to a specific instance */
    IFC(InitTedApp(lpCmdLine, nCmdShow));

    /* Acquire and dispatch messages until a WM_QUIT uMessage is received. */
    while(GetMessage( &msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(g_pApp->m_hWnd, hAccelTable, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete g_pApp;

    DestroyAcceleratorTable(hAccelTable);

    ::FreeLibrary(g_hTedUtilInst);
    
    return (int)msg.wParam;

Cleanup:
    assert(SUCCEEDED(hr));
    return FALSE;
}



