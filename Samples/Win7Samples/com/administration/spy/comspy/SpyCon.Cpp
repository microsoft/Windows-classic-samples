// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "COMSpy.h"
#include "ComSpyCtl.h"
#include "SpyCon.h"

static const LPCWSTR COMSPY_REG_KEY = L"Software\\Microsoft\\COMSpy";
static const LPCWSTR REG_VALUE_LEFT = L"Left";
static const LPCWSTR REG_VALUE_TOP = L"Top";
static const LPCWSTR REG_VALUE_RIGHT = L"Right";
static const LPCWSTR REG_VALUE_BOTTOM = L"Bottom";
static const LPCWSTR REG_VALUE_SHOW_WINDOW = L"ShowWindow";
static const LPCWSTR REG_VALUE_ALWAYS_ON_TOP = L"AlwaysOnTop";
static const LPCWSTR REG_VALUE_SAVE_ON_EXIT = L"SaveOnExit";
static const LPCWSTR REG_VALUE_CONTROL_PROPERTIES = L"ControlProperties";

/////////////////////////////////////////////////////////////////////////////
// CSpyCon


STDMETHODIMP CSpyCon::Run()
{
    // Create our window
    RECT rcPos = { CW_USEDEFAULT, 0, 0, 0 };

    CString sWindowTitle;
    BOOL bResult = sWindowTitle.LoadStringW(IDS_WINDOW_TITLE);
    _ASSERTE(bResult);

    HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU1));
    _ASSERTE(hMenu);

    HWND hwnd = Create(NULL, rcPos, sWindowTitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, hMenu);
    _ASSERTE(hwnd);

    // Set its icon
    HICON hIcon = LoadIcon(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_MAIN));
    _ASSERTE(hIcon);
    SetIcon(hIcon, TRUE);

    // Create the spy control
    HRESULT hr = CoCreateInstance(CLSID_ComSpy, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&m_spOleObject));
    if (FAILED(hr))
    {
        MessageBox(L"CoCreateInstance failed");
        return hr;
    }
    _ASSERTE(m_spOleObject);

    m_spOleObject->SetClientSite(this);

    hr = m_spOleObject->QueryInterface(IID_PPV_ARGS(&m_spSpy));
    _ASSERTE(SUCCEEDED(hr) && m_spSpy);

    // Get our window state from the registry
    HKEY hKey = NULL;
    (void)RegCreateKeyEx(HKEY_LOCAL_MACHINE, COMSPY_REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,  NULL, &hKey, NULL);
    if (hKey)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_LEFT, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_wp.rcNormalPosition.left = (LONG)dwValue;
        }

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_TOP, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_wp.rcNormalPosition.top = (LONG)dwValue;
        }

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_RIGHT, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_wp.rcNormalPosition.right = (LONG)dwValue;
        }

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_BOTTOM, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_wp.rcNormalPosition.bottom = (LONG)dwValue;
        }

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_SHOW_WINDOW, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_wp.showCmd = (UINT)dwValue;
        }

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_ALWAYS_ON_TOP, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_bAlwaysOnTop = (dwValue != 0);
        }

        // Using OnAlwaysOnTop to handle the always on top option, which toggles the value
        m_bAlwaysOnTop = !m_bAlwaysOnTop;
        BOOL bHandledIgnored = TRUE;
        OnAlwaysOnTop(0,0,0,bHandledIgnored);

        dwSize = sizeof(dwValue);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_SAVE_ON_EXIT, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) &&
            (dwType == REG_DWORD) && (dwSize == sizeof(dwValue)))
        {
            m_bSaveOnExit = (dwValue != 0);
        }

        // Get the control properties persisted in a stream
        dwSize = 0;
        if ((ERROR_SUCCESS == RegQueryValueEx(hKey, REG_VALUE_CONTROL_PROPERTIES, NULL, &dwType, NULL, &dwSize)) &&
            (dwType == REG_BINARY))
        {
            HGLOBAL hGlobal = GlobalAlloc(GPTR, dwSize);
            if (hGlobal)
            {
                BYTE* pbValue = (BYTE*)GlobalLock(hGlobal);
                _ASSERTE(pbValue);
                LONG lResult = RegQueryValueEx(hKey, REG_VALUE_CONTROL_PROPERTIES, NULL, &dwType, pbValue, &dwSize);
                GlobalUnlock(hGlobal);

                CComPtr<IStream> spStream;
                if ((ERROR_SUCCESS == lResult) &&
                    SUCCEEDED(CreateStreamOnHGlobal(hGlobal, TRUE, &spStream)))
                {
                    CComPtr<IPersistStreamInit> spPersist;
                    hr = m_spSpy->QueryInterface(IID_PPV_ARGS(&spPersist));
                    _ASSERTE(SUCCEEDED(hr) && spPersist);

                    // Best effort (ignore failure)
                    (void)spPersist->Load(spStream);
                }
                else
                {
                    GlobalFree(hGlobal);
                }
            }
        }        
    }

    if (!(( m_wp.rcNormalPosition.right - m_wp.rcNormalPosition.left < 10) || (m_wp.rcNormalPosition.bottom - m_wp.rcNormalPosition.top < 10)))
        SetWindowPlacement(&m_wp);
    else
        ShowWindow(SW_SHOWNORMAL);

    GetClientRect(&rcPos);
    MSG msg;
    return m_spOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, this, 0, m_hWnd, &rcPos);    
}

//
//    show the select filter dialog
//
LRESULT CSpyCon::OnSelectFilter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    m_spSpy->SelectApplications();
    return 0;
}

//
//    the user clicks on the system menu
//
LRESULT CSpyCon::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE; // always do default processing

    if (wParam == SC_CLOSE)
        OnExit(NULL, NULL, NULL, bHandled);

    return 0;
}

//
//    sets a glog to log to file
//    if the user hasn't chosen a log file, show the common dialog to allow them to select one
//
LRESULT CSpyCon::OnLogToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    BOOL bVal;
    m_spSpy->get_LogToFile(&bVal);
    bVal = !bVal;
    m_spSpy->put_LogToFile(bVal);
    return 0;    
}

//
//    User wants to choose a log file -- show the common dialog
//
LRESULT CSpyCon::OnChooseLogFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    CComBSTR bstrFileName;
    BOOL bCanceled;
    m_spSpy->ChooseLogFile(&bstrFileName, &bCanceled);
    return 0;
    
}

//
//    toggle the save on exit flag
//
LRESULT CSpyCon::OnToggleSaveOnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)

{
    m_bSaveOnExit = !m_bSaveOnExit;
    // For most of the options, we do not save until exit or "Save Settings Now" is clicked.
    // If Save On Exit is disabled, however, we need to save this now.  Otherwise, this
    // option won't be saved and will still be enabled the next time we run.
    if (!m_bSaveOnExit)
    {
        // Best effort (ignore failures)
        HKEY hKey = NULL;
        (void)RegCreateKeyEx(HKEY_LOCAL_MACHINE, COMSPY_REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,  NULL,  &hKey, NULL);
        if (hKey)
        {
            DWORD dwValue = 0;
            (void)RegSetValueEx(hKey, REG_VALUE_SAVE_ON_EXIT, NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
            RegCloseKey(hKey);
        }
    }

    return 0;    
}

//
//    toggle the show on screen flag
//
LRESULT CSpyCon::OnToggleShowOnScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    BOOL bVal;
    m_spSpy->get_ShowOnScreen(&bVal);
    bVal = !bVal;
    m_spSpy->put_ShowOnScreen(bVal);
    return 0;    

}
//
//    update the UI in menus
//
LRESULT CSpyCon::OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    HMENU hMenu = (HMENU)wParam;
    CComBSTR temp = "&Log to File ";
    CComBSTR bstrFileName;
    m_spSpy->get_LogFile(&bstrFileName);
    if (bstrFileName.Length() > 5)
        temp += bstrFileName;
    else
        temp = "&Log To file...";

    ModifyMenu(hMenu, ID_LOG, MF_BYCOMMAND|MF_STRING, ID_LOG, temp);
    BOOL bVal;
    m_spSpy->get_LogToFile(&bVal);
    CheckMenuItem(hMenu, ID_LOG,   bVal ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    m_spSpy->get_ShowGridLines(&bVal);    
    CheckMenuItem(hMenu, ID_OPTIONS_GRID_LINES,  bVal ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    m_spSpy->get_ShowOnScreen(&bVal);
    CheckMenuItem(hMenu, ID_SHOW_ON_SCREEN,  bVal ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    CheckMenuItem(hMenu, IDM_ON_TOP,  m_bAlwaysOnTop ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    CheckMenuItem(hMenu, ID_SAVE_ON_EXIT,  m_bSaveOnExit ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 

    return 0;
}

//
//    show/hide gridlines in the list
//
//    NOTE:  This will only work if the user has installed IE 3.0 or greater
//
LRESULT CSpyCon::OnToggleGridLines(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    BOOL bVal;
    m_spSpy->get_ShowGridLines(&bVal);
    bVal = !bVal;
    m_spSpy->put_ShowGridLines(bVal);    
    return 0;
}
//
//    Toggle the Always on top option
//
LRESULT CSpyCon::OnAlwaysOnTop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bAlwaysOnTop = !m_bAlwaysOnTop;
    if (m_bAlwaysOnTop)
        SetWindowPos(HWND_TOPMOST, 0,0,0,0,    SWP_NOMOVE|SWP_NOSIZE);
    else
        SetWindowPos(HWND_NOTOPMOST, 0,0,0,0,    SWP_NOMOVE|SWP_NOSIZE);
    return 0;
}
//
//    show the standard Shell About box
//
LRESULT CSpyCon::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    m_spSpy->About();
    return 0;
}
//
//    pick the font to use in the list
//
LRESULT CSpyCon::OnChooseFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    m_spSpy->ChooseFont();
    return 0;
}
//
//    called on a WM_CLOSE & WM_SYSCOMMAND (SC_CLOSE) message.  If appropriate, save settings
//    and then destroy the window
//
LRESULT CSpyCon::OnExit(WORD /*wNotifyCode*/, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_bSaveOnExit)
        SaveSettings();

    m_spOleObject->Close(OLECLOSE_NOSAVE);
    m_spOleObject.Release();
    m_spSpy.Release();
    DestroyWindow();
    return 0;
}

//
//    save current settings to the registry
//
LRESULT CSpyCon::OnSaveNow(WORD /*wNotifyCode*/, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SaveSettings();
    return 0;
}


//
//    clear the list box
//
LRESULT CSpyCon::OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    m_spSpy->ClearAllEvents();
    return 0;
}


//
//    save the items in the list box to the log file
//
LRESULT CSpyCon::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    _ASSERTE(m_spSpy);

    m_spSpy->SaveToFile();
    return 0;

}
//
//    saves settings.  asks the control fro a stream to save its state
//
HRESULT CSpyCon::SaveSettings()
{
    _ASSERTE(m_spSpy);

    // Best effort (ignore failures)
    HKEY hKey = NULL;
    (void)RegCreateKeyEx(HKEY_LOCAL_MACHINE, COMSPY_REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,  NULL,  &hKey, NULL);
    if (hKey)
    {
        DWORD dwValue = (DWORD)m_wp.rcNormalPosition.left;
        (void)RegSetValueEx(hKey, REG_VALUE_LEFT, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = (DWORD)m_wp.rcNormalPosition.top;
        (void)RegSetValueEx(hKey, REG_VALUE_TOP, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = (DWORD)m_wp.rcNormalPosition.right;
        (void)RegSetValueEx(hKey, REG_VALUE_RIGHT, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = (DWORD)m_wp.rcNormalPosition.bottom;
        (void)RegSetValueEx(hKey, REG_VALUE_BOTTOM, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = (DWORD)m_wp.showCmd;
        (void)RegSetValueEx(hKey, REG_VALUE_SHOW_WINDOW, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = m_bAlwaysOnTop ? 1 : 0;
        (void)RegSetValueEx(hKey, REG_VALUE_ALWAYS_ON_TOP, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        dwValue = m_bSaveOnExit ? 1 : 0;
        (void)RegSetValueEx(hKey, REG_VALUE_SAVE_ON_EXIT, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        CComPtr<IPersistStreamInit> spPersist;
        HRESULT hr = m_spSpy->QueryInterface(IID_PPV_ARGS(&spPersist));
        _ASSERTE(SUCCEEDED(hr) && spPersist);

        CComPtr<IStream> spStream;
        if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &spStream)))
        {
            if (SUCCEEDED(spPersist->Save(spStream, TRUE)))
            {
                HGLOBAL hGlobal = NULL;
                hr = GetHGlobalFromStream(spStream, &hGlobal);
                _ASSERTE(SUCCEEDED(hr) && spStream);

                BYTE* pbValue = (BYTE *)GlobalLock(hGlobal);
                _ASSERTE(pbValue);

                DWORD dwSize = (DWORD)GlobalSize(hGlobal);
                (void)RegSetValueEx(hKey, REG_VALUE_CONTROL_PROPERTIES, 0, REG_BINARY, pbValue, dwSize);
                GlobalUnlock(hGlobal);
            }
        }

        RegCloseKey(hKey);
    }
    return S_OK;
}

LRESULT CSpyCon::OnNCDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{            
    PostQuitMessage(0);
    return 0;
}

LRESULT CSpyCon::OnErase(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled)
{
    if (m_bInPlaceActive)
        return 0;
    bHandled = FALSE;
    return 1;
}

LRESULT CSpyCon::OnSize(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{
    if (m_spOleObject && m_bInPlaceActive)
    {
        CComPtr<IOleInPlaceObject> spInPlaceObject;
        HRESULT hr = m_spOleObject->QueryInterface(IID_PPV_ARGS(&spInPlaceObject));
        _ASSERTE(SUCCEEDED(hr) && spInPlaceObject);

        RECT rcClient;
        GetClientRect(&rcClient);
        spInPlaceObject->SetObjectRects(&rcClient, &rcClient);
        GetWindowPlacement(&m_wp); // for storing in the registry
    }
    return 0;
}
