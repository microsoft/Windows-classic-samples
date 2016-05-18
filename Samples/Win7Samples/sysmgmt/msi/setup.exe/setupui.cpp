//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File:       setupui.cpp
//
//  Implementation of CDownloadUI class
//--------------------------------------------------------------------------

#define WIN // scope W32 API
#define COMCTL32 // scope COMCTRL32

#include "resource.h"
#include "setupui.h"
#include <commctrl.h>
#include <strsafe.h>

//{B506A5D1-9716-4F35-8ED5-9ECB0E9A55F8}
const GUID IID_IDownloadBindStatusCallback = {0xB506A5D1L,0x9716,0x4F35,{0x8E,0xD5,0x9E,0xCB,0x0E,0x9A,0x55,0xF8}};
//{00000000-9716-4F35-8ED5-9ECB0E9A55F8}
const GUID IID_IUnknown = {0x00000000L,0x9716,0x4F35,{0x8E,0xD5,0x9E,0xCB,0x0E,0x9A,0x55,0xF8}};


/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::CDownloadUI constructor
//

CDownloadUI::CDownloadUI() : m_hwndProgress(0), m_hwndParent(0), m_hInst(0),
                            m_fInitialized(false), m_fUserCancel(false),
                            m_ulProgressMax(0), m_ulProgressSoFar(0)
{
    StringCchCopy(m_szCaption, ARRAYSIZE(this->m_szCaption), "");
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::~CDownloadUI destructor
//

CDownloadUI::~CDownloadUI()
{
}

/////////////////////////////////////////////////////////////////////////////
// ProgressProc - callback routine for IDD_PROGRESS dialog
//

BOOL CALLBACK ProgressProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if (uiMsg == WM_INITDIALOG)
    {
        return TRUE;
    }
    else if (uiMsg == WM_COMMAND && wParam == IDCANCEL)
    {
        ((CDownloadUI*)lParam)->SetUserCancel();
        return TRUE;
    }
    else if (uiMsg == WM_SETCURSOR)
    {
        // always display WAIT cursor if mouse not over Cancel button
        if ( (HWND)wParam != WIN::GetDlgItem(hDlg, IDC_DOWNLOAD_CANCEL))
        {
            WIN::SetCursor(WIN::LoadCursor(0, MAKEINTRESOURCE(IDC_WAIT)));
            return TRUE;
        }
    }
    else if (uiMsg == WM_CLOSE)
    {

    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// GetScreenCenterCoord
//

bool GetScreenCenterCoord(HWND hDlg, int& iDialogLeft, int& iDialogTop, int& iDialogWidth, int& iDialogHeight)
{
    RECT rcDialog;
    if (!WIN::GetWindowRect(hDlg, &rcDialog))
        return false;

    RECT rcScreen;
    if (!WIN::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0))
    {
        rcScreen.left = 0;
        rcScreen.top = 0;
        rcScreen.right = WIN::GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = WIN::GetSystemMetrics(SM_CYSCREEN);
    }
    iDialogWidth = rcDialog.right - rcDialog.left;
    iDialogHeight = rcDialog.bottom - rcDialog.top;
    iDialogLeft = rcScreen.left + (rcScreen.right - rcScreen.left - iDialogWidth)/2;
    iDialogTop = rcScreen.top + (rcScreen.bottom - rcScreen.top - iDialogHeight)/2;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::Initialize
//

bool CDownloadUI::Initialize(HINSTANCE hInst, HWND hwndParent, LPCSTR szCaption)
{
    INITCOMMONCONTROLSEX iccData = {sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS};
    COMCTL32::InitCommonControlsEx(&iccData);

    char szText[MAX_STR_CAPTION] = {0};

    // set member variables
    m_hwndParent = hwndParent;
    m_hInst = hInst;

    StringCchCopy(m_szCaption, ARRAYSIZE(this->m_szCaption), szCaption);

    if (!m_hwndProgress)
    {
        // create Progress Dialog
        m_hwndProgress = WIN::CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_PROGRESS), m_hwndParent, ProgressProc, (LPARAM)this);
        if (!m_hwndProgress)
            return false;

        // set window caption
        WIN::SetWindowText(m_hwndProgress, m_szCaption);

        // center dialog on screen
        int iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight;
        ::GetScreenCenterCoord(m_hwndProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight);
        WIN::MoveWindow(m_hwndProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight, TRUE);

        // set CANCEL button text
        WIN::LoadString(m_hInst, IDS_CANCEL, szText, MAX_STR_CAPTION);
        WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_CANCEL, szText);

        // set to foreground and make visible all controls
        WIN::SetFocus(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR));
        WIN::ShowWindow(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_CANCEL), SW_SHOW);
        WIN::SetForegroundWindow(m_hwndProgress);
        WIN::ShowWindow(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR), SW_SHOW);

        // set icon
        HICON hIcon = (HICON) WIN::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_INSTALLER));
        if (hIcon)
            WIN::SendMessage(m_hwndProgress, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

        // make window visible
        WIN::ShowWindow(m_hwndProgress, SW_SHOW);
    }

    // message pump
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }

    m_fInitialized = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::Terminate
//

bool CDownloadUI::Terminate()
{
    if (m_hwndProgress)
    {
        // destroy the progress window
        WIN::DestroyWindow(m_hwndProgress);
        m_hwndProgress = 0;
    }

    m_hInst                 = 0;
    m_hwndParent            = 0;
    m_fInitialized          = false;
    m_fUserCancel           = false;
    m_ulProgressMax         = 0;
    m_ulProgressSoFar       = 0;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::GetCurrentWindow
//

HWND CDownloadUI::GetCurrentWindow()
{
    return (m_hwndProgress) ? m_hwndProgress : m_hwndParent;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::SetUserCancel
//

void CDownloadUI::SetUserCancel()
{
    m_fUserCancel = true;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::HasUserCanceled
//

bool CDownloadUI::HasUserCanceled()
{
    return (m_fUserCancel);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::SetBannerText
//

irmProgress CDownloadUI::SetBannerText(LPCSTR szBanner)
{
    if (!m_fInitialized)
        return irmNotInitialized;

    if (m_fUserCancel)
        return irmCancel;

    WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_BANNER, szBanner);

    return irmOK;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::SetActionText
//

irmProgress CDownloadUI::SetActionText(LPCSTR szAction)
{
    if (!m_fInitialized)
        return irmNotInitialized;

    if (m_fUserCancel)
        return irmCancel;

    WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_ACTIONTEXT, szAction);

    return irmOK;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::InitProgressBar
//

void CDownloadUI::InitProgressBar(ULONG ulProgressMax)
{
    // init progress bar values
    m_ulProgressMax         = ulProgressMax;
    m_ulProgressSoFar       = 0;

    // set range on progress bar of [0, ulProgressMax]
    HWND hwndProgBar = WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR);
    WIN::SendMessage(hwndProgBar, PBM_SETRANGE32, /* WPARAM = */ 0, /* LPARAM = */ m_ulProgressMax);

    // initialize the position of the progress bar -- forward direction, so set at 0
    WIN::SendMessage(hwndProgBar, PBM_SETPOS, /* WPARAM = */ (WPARAM)0, /* LPARAM = */ 0);

    // message pump
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadUI::IncrementProgressBar
//

void CDownloadUI::IncrementProgressBar(ULONG ulProgress)
{
    // increment progress bar

    HWND hwndProgBar = WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR);
    WIN::SendMessage(hwndProgBar, PBM_DELTAPOS, /* WPARAM = */ (WPARAM) (ulProgress), /* LPARAM = */ 0);

    m_ulProgressSoFar += ulProgress;

    // message pump
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::CDownloadBindStatusCallback constructor
//

CDownloadBindStatusCallback::CDownloadBindStatusCallback(CDownloadUI* pDownloadUI) : m_pDownloadUI(pDownloadUI), m_iRefCnt(1), m_ulProgressSoFar(0)
{
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::~CDownloadBindStatusCallback destructor
//

CDownloadBindStatusCallback::~CDownloadBindStatusCallback()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::QueryInterface
//

HRESULT CDownloadBindStatusCallback::QueryInterface(const IID& riid, void** ppvObj)
{
    if (!ppvObj)
        return E_INVALIDARG;

    if (riid == IID_IUnknown || riid == IID_IDownloadBindStatusCallback)
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::AddRef
//

unsigned long CDownloadBindStatusCallback::AddRef()
{
    return ++m_iRefCnt;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::Release
//

unsigned long CDownloadBindStatusCallback::Release()
{
    if (--m_iRefCnt != 0)
        return m_iRefCnt;
    delete this;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadBindStatusCallback::OnProgress
//

HRESULT CDownloadBindStatusCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR /*szStatusText*/)
{
    switch (ulStatusCode)
    {
    case BINDSTATUS_BEGINDOWNLOADDATA:
        {
            // initialize progress bar with max # of ticks
            m_pDownloadUI->InitProgressBar(ulProgressMax);

            // init progress so far
            m_ulProgressSoFar = 0;

            // check for cancel
            if (m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

            // fall through
        }
    case BINDSTATUS_DOWNLOADINGDATA:
        {
            // calculate how far we have moved since the last time
            ULONG ulProgIncrement = ulProgress - m_ulProgressSoFar;

            // set progress so far to current value
            m_ulProgressSoFar = ulProgress;

            // send progress message (if we have progressed)
            if (ulProgIncrement > 0)
                m_pDownloadUI->IncrementProgressBar(ulProgIncrement);

            // check for cancel
            if(m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

            break;
        }
    case BINDSTATUS_ENDDOWNLOADDATA:
        {
            // send any remaining progress to complete download portion of progress bar
            ULONG ulProgIncrement = ulProgressMax - m_ulProgressSoFar;
            if (ulProgIncrement > 0)
                m_pDownloadUI->IncrementProgressBar(ulProgIncrement);
            
            // check for cancel
            if(m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

            break;
        }
    }

    return S_OK;
}
