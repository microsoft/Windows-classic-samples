// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <comsvcs.h>
#include "ComSpyCtl.h"
#include "ComSpyAudit.h"
#include "AppInfo.h"
#include "CComSpy.h"
#include "selecteventsdlg.h"
#include "commdlg.h"
#include <time.h>
#include "shellapi.h"
#include <strsafe.h>

LONGLONG PerformanceFrequency = 0;

//
//    for some reason this define isn't being included
//
    
// const GUID CLSID_StockFontPage = {0x7ebdaae0, 0x8120, 0x11cf, { 0x89, 0x9f, 0x0, 0xaa, 0x0, 0x68, 0x8b, 0x10}};        


//
//  given a GUID, return its string representation
//
BSTR GuidToBstr(REFGUID guid)
{
    CComBSTR b(40);
    int nRet = StringFromGUID2(guid, b.m_str, 40);
    _ASSERTE(nRet);
    return b.Detach();
}

/////////////////////////////////////////////////////////////////////////////
// CComSpy

#define MAX_MSG_LENGTH  1024
static const LPCWSTR pwszColNames[] =
{ L"Count", L"Event", L"Tick Count", L"Application", L"Parameter", L"Value" };


CComSpy::CComSpy() :
    m_ctlSysListView32(L"SysListView32", this, 1)
{
     m_hWndList = NULL;
    m_bLogToFile = FALSE;
    m_hFile = NULL;
    m_cEvents = 0;
    m_bShowGridLines = TRUE;
    m_bWindowOnly = TRUE; 
    for (int i=0;i<NUMBER_COLUMNS;i++)
    {
        m_nWidth[i] = 60;
    }
    
    m_hFont = NULL;
    m_bShowOnScreen = TRUE;
    m_bCSV = FALSE;
    m_bAudit = FALSE;
    m_spSqlAudit = NULL;
    
    CComBSTR bstrAppName(L"SysApp");
    m_pSysAppInfo = new CAppInfo((LPCWSTR)bstrAppName, this); 
    _ASSERTE(m_pSysAppInfo);
}


HRESULT CComSpy::OnDrawAdvanced(ATL_DRAWINFO& di)
{
    return S_OK;
}
LRESULT CComSpy::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_SPYMENU));    
    _ASSERTE(hMenu != NULL);
    if (!hMenu)
        return 0;
    POINT local;
    local.x = LOWORD(lParam);
    local.y = HIWORD(lParam);
    ClientToScreen(&local);
    HMENU hPopup = GetSubMenu(hMenu, 0);
    _ASSERTE(hPopup != NULL);
    if (!hPopup)
        return 0;
    TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, local.x, local.y, NULL, m_hWndList, NULL); 

    return 0;
}

LRESULT CComSpy::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rc;
    GetWindowRect(&rc);
    rc.right -= rc.left;
    rc.bottom -= rc.top;
    rc.top = rc.left = 0;
    DWORD dwStyle = WS_VISIBLE|WS_CHILD|LVS_REPORT|LVS_SINGLESEL|WS_BORDER;
    if (!m_hWndList) 
        m_hWndList = m_ctlSysListView32.Create(m_hWnd, rc, L"", dwStyle);

    _ASSERTE(m_hWndList);
    put_ShowGridLines(m_bShowGridLines);
    
    //
    //    we always want the list to be Full Row Select --
    //    note: this only works if the user has IE 3.0 installed
    //
    dwStyle = ListView_GetExtendedListViewStyle(m_hWndList);
    dwStyle |= LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyle(m_hWndList, dwStyle);

    //
    //    create the columns in our list 
    //
    LV_COLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_TEXT|LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    int i;
    for (i=0;i<NUMBER_COLUMNS;i++)
    {
        col.cx = m_nWidth[i];    
        col.pszText = (LPWSTR)pwszColNames[i];  // Ok, ListView_InsertColumn does not write to this string
        col.cchTextMax  = lstrlen(col.pszText);
        ListView_InsertColumn(m_hWndList, i, &col);
    }

    QueryPerformanceFrequency((LARGE_INTEGER *)&PerformanceFrequency);

    if (m_hFont)
    {    
        // this is set if the container app has set our properties using the property bag
        ::SendMessage(m_hWndList, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE,0));
    }        

    return 0;
}


//
//    update the UI in menus
//
LRESULT CComSpy::OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HMENU hMenu = (HMENU)wParam;
    CComBSTR temp = "Log to File ";
    if (m_sLogFile.Length() > 5)
        temp += m_sLogFile;
    else
        temp += "...";

    ModifyMenu(hMenu, ID_LOG, MF_BYCOMMAND|MF_STRING, ID_LOG, temp);
    CheckMenuItem(hMenu, ID_LOG,  m_bLogToFile ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    CheckMenuItem(hMenu, ID_OPTIONS_GRID_LINES,  m_bShowGridLines ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED); 
    CheckMenuItem(hMenu, ID_SHOW_ON_SCREEN,  m_bShowOnScreen ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);     
    CheckMenuItem(hMenu, ID_AUDIT,  m_bAudit ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);     

    m_hMenuDebug = GetSubMenu(hMenu, 5);
    _ASSERTE(m_hMenuDebug);
    AddRunningAspsToDebugMenu(m_hMenuDebug);

    return 0;
}


//
//    sets a glog to log to file
//    if the user hasn't chosen a log file, show the common dialog to allow them to select one
//
LRESULT CComSpy::OnLogToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{


    m_bLogToFile = 1 - m_bLogToFile;
    if (m_bLogToFile)
    {
        if (m_sLogFile.Length() == 0)
        {
            BOOL bHandledIgnored = TRUE;
            OnChooseLogFile(0,0,0,bHandledIgnored);
            if (m_sLogFile.Length() == 0)
                return 0;
        }
        m_hFile = CreateFile(m_sLogFile, GENERIC_WRITE,
                            0, (LPSECURITY_ATTRIBUTES) NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            (HANDLE) NULL);

        if (!m_hFile)
        {
            m_bLogToFile = FALSE;
            return 0;
        }

    }
    else
    {
        if (m_hFile)
        {
            CloseHandle(m_hFile);
            m_hFile = NULL;
        }

    }
    
    return 0;    
}
//
//    User wants to choose a log file -- show the common dialog
//
LRESULT CComSpy::OnChooseLogFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    OPENFILENAME OpenFileName;
    WCHAR         szFile[MAX_PATH];
    ZeroMemory(szFile, sizeof(szFile));
    if (m_sLogFile.Length() > 2 && m_sLogFile.Length() < MAX_PATH-1)
        StringCchCopy(szFile, MAX_PATH, m_sLogFile.m_str);
    else
        StringCchCopy(szFile, MAX_PATH, L"ComSpy.Log");
    
    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = m_hWndList;
    OpenFileName.hInstance         = _Module.GetModuleInstance( );
    OpenFileName.lpstrFilter       = L"TAB seperated Files (*.log)\0*.log\0CSV Files (*.csv)\0*.csv\0";
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = ARRAYSIZE(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    OpenFileName.lpstrInitialDir   = NULL;
    OpenFileName.lpstrTitle        = L"Choose Log File";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook            = NULL;
    OpenFileName.lpTemplateName    = NULL;
    OpenFileName.Flags             = OFN_EXPLORER | OFN_HIDEREADONLY |OFN_NOREADONLYRETURN |OFN_CREATEPROMPT;

    // Call the common dialog function.
    if (GetSaveFileName(&OpenFileName))
    {
        m_sLogFile = OpenFileName.lpstrFile;            
        if (_wcsicmp(OpenFileName.lpstrFile + OpenFileName.nFileExtension, L"csv") == 0)
            m_bCSV = TRUE;

    }
     
    return 0;
    
}


//
//    show/hide gridlines in the list
//
//    NOTE:  This will only work if the user has installed IE 3.0 or greater
//
LRESULT CComSpy::OnToggleGridLines(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    m_bShowGridLines = 1 - m_bShowGridLines;

    DWORD dwStyle = ListView_GetExtendedListViewStyle(m_hWndList);
    (m_bShowGridLines) ? dwStyle |= LVS_EX_GRIDLINES : 
                         dwStyle &= ~LVS_EX_GRIDLINES;

    ListView_SetExtendedListViewStyle(m_hWndList, dwStyle);

    return 0;
}

//
//    enable/disable auditing
//
//
LRESULT CComSpy::OnToggleAudit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    m_bAudit = !m_bAudit;
    HRESULT hr = EnableAudit(m_bAudit);
    if (hr != S_OK)
        m_bAudit = FALSE;
    return 0;
}


LRESULT CComSpy::OnToggleShowOnScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bShowOnScreen = 1-m_bShowOnScreen;
    return 0;

}

//
//    show the standard Shell About box
//
LRESULT CComSpy::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HICON hIcon =  LoadIcon(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_MAIN));

    ShellAbout(m_hWnd, L"COM Spy", L"Version 1.00", hIcon);
    return 0;
}

//
//    clear the list box
//
LRESULT CComSpy::OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    ListView_DeleteAllItems(m_hWndList);
    m_cEvents = 0;
    return 0;
}


//
//    save the items in the list box to the log file
//
LRESULT CComSpy::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    OPENFILENAME OpenFileName;
    WCHAR         szFile[MAX_PATH];
    ZeroMemory(szFile, sizeof(szFile));
    if (m_sLogFile.Length() > 2 && m_sLogFile.Length() < MAX_PATH -1)
        StringCchCopy(szFile, MAX_PATH, m_sLogFile.m_str);
    else
        StringCchCopy(szFile, MAX_PATH, L"COMSpy.Log");
    
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = m_hWndList;
    OpenFileName.hInstance         = _Module.GetModuleInstance( );
    OpenFileName.lpstrFilter       = NULL;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = ARRAYSIZE(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    OpenFileName.lpstrInitialDir   = NULL;
    OpenFileName.lpstrTitle        = L"Log ToFile";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook            = NULL;
    OpenFileName.lpTemplateName    = NULL;
    OpenFileName.Flags             = OFN_EXPLORER | OFN_HIDEREADONLY |OFN_NOREADONLYRETURN;

    // Call the common dialog function.
    if (GetSaveFileName(&OpenFileName))
    {
    
        FILE* f;
        if (_tfopen_s(&f, OpenFileName.lpstrFile, L"a+") == 0)
        {
            int nCount = ListView_GetItemCount(m_hWndList);
            int i, j;
            WCHAR wszBuffer[MAX_MSG_LENGTH]; 
            for (i=0; i<nCount; i++)
            {
                for (j=0; j < 4; j++)
                {
                    ListView_GetItemText(m_hWndList, i, j, wszBuffer, ARRAYSIZE(wszBuffer));
                    _ftprintf(f, L"%s\t",wszBuffer);                    
                }        
                while ( i < nCount - 1)
                {
                    i++; // move to next column

                    // now get the name/value pairs
                    ListView_GetItemText(m_hWndList, i, 4, wszBuffer, ARRAYSIZE(wszBuffer));                    
                    if (*wszBuffer)
                    {
                        
                        _ftprintf(f, L"\n\t\t\t\t%s\t", wszBuffer);                    
                        ListView_GetItemText(m_hWndList, i, 5, wszBuffer, ARRAYSIZE(wszBuffer));
                        _ftprintf(f, L"%s",wszBuffer);                                    
                    }
                    else
                    {
                        i--;
                        _ftprintf(f, L"\n");
                        break;
                    }
                }
            }

            fclose(f);
        }
                    

    }
     
    return 0;


}

//
//    given the index to the list, we return TRUE if the index
//    should scroll -- this occurs when the user is looking
//    at the last element in the list.  If they have scrolled
//    up to look at a previous element in the list, then we
//    don't scroll when an event is added
//
bool CComSpy::ShouldScroll(int nIndex)
{
    bool bRet = false;
    int nCountPage = ListView_GetCountPerPage(m_hWndList);
    int nTopIndex = ListView_GetTopIndex(m_hWndList);
    if (nTopIndex + nCountPage>= nIndex)    
        bRet = true;
    return bRet;
}

//
//    disconnect & destroy all sinks associated with the given App
//
HRESULT CComSpy::ShutdownApplication(LPCWSTR pwszApplicationName)
{
    CAppInfo * pInfo = m_map[pwszApplicationName];
    if (!pInfo)
    {
        _ASSERTE(0); // we should never get this unless we have a sink for (at least) the Applicatioin events
        return E_FAIL;
    }

    size_t nElementsRemoved = m_map.erase(pwszApplicationName);
    _ASSERTE(nElementsRemoved == 1);
    delete pInfo;    
    return S_OK;
}

//
//    this method is called by all of the sinks to add an item to the list box
//
bool CComSpy::AddEventToList(LONGLONG perfCount, LPCWSTR pwszEvent, LPCWSTR pwszApplication)
{
    long TickCount = 0;
    if (PerformanceFrequency != 0)
        TickCount = (long)((1000 * perfCount) / PerformanceFrequency);

    bool bRet;
    if (m_bLogToFile)
    {
        WCHAR sSep[2];
        sSep[0] = m_bCSV ? L',' : L'\t';
        sSep[1] = L'\0';
        _ASSERTE(m_hFile);    
        WCHAR wszBuffer[MAX_MSG_LENGTH];
        ZeroMemory(wszBuffer, sizeof(wszBuffer));

        StringCchPrintf(wszBuffer, ARRAYSIZE(wszBuffer), L"%s%s%d%s%s\n", pwszEvent, sSep, TickCount, sSep, pwszApplication);

        if (m_hFile)
        {

            DWORD cbWritten;            
            bRet = (FALSE != WriteFile(m_hFile, wszBuffer, lstrlen(wszBuffer) * sizeof(WCHAR), &cbWritten, NULL));

        }
    }
    if (m_bShowOnScreen)
    {
        int nCount = ListView_GetItemCount(m_hWndList); 
        LV_ITEM item;
        WCHAR sz[8];
        StringCchPrintf(sz, ARRAYSIZE(sz), L"%ld", m_cEvents++);
        ZeroMemory(&item, sizeof(item));
        item.mask = LVIF_TEXT;
        item.iItem = nCount;
        item.pszText = sz;
        item.cchTextMax = lstrlen(sz);
        
        ListView_InsertItem(m_hWndList, &item);
        ListView_SetItemText(m_hWndList, nCount, 1, (LPWSTR)pwszEvent);

        WCHAR szTick[16];
        StringCchPrintf(szTick, ARRAYSIZE(szTick), L"%ld", TickCount);
        ListView_SetItemText(m_hWndList, nCount, 2, szTick);
        ListView_SetItemText(m_hWndList, nCount, 3, (LPWSTR)pwszApplication);
        if (ShouldScroll(nCount))
            ListView_EnsureVisible(m_hWndList, nCount, FALSE);        
    }
    return TRUE;
}
//
//    this method is called by all the sinks to add a Named/Value pair to the 
//    list
//
bool CComSpy::AddParamValueToList(LPCWSTR pwszParamName, LPCWSTR pwszValue)
{
    bool bRet;
    if (m_bLogToFile)
    {
        WCHAR sSep[2];
        sSep[0] = m_bCSV ? L',' : L'\t';
        sSep[1] = L'\0';
        _ASSERTE(m_hFile);    
        WCHAR wszBuffer[MAX_MSG_LENGTH];
        ZeroMemory(wszBuffer, sizeof(wszBuffer));
        StringCchPrintf(wszBuffer, ARRAYSIZE(wszBuffer), L"%s%s%s%s%s%s\n", sSep, sSep, sSep, pwszParamName,sSep, pwszValue);
        if (m_hFile)
        {

            DWORD cbWritten;            
            bRet = (FALSE != WriteFile(m_hFile, wszBuffer, lstrlen(wszBuffer) * sizeof(WCHAR), &cbWritten, NULL));
        }        
    }
    if (m_bShowOnScreen)
    {
        int nCount = ListView_GetItemCount(m_hWndList); 
        LV_ITEM item;
        item.mask = LVIF_TEXT;
        item.iItem = nCount;
        item.pszText = L"";
        item.cchTextMax = 1;
        item.iSubItem = 0;
        int nItem;
        nItem = ListView_InsertItem(m_hWndList, &item);
        ListView_SetItemText(m_hWndList, nItem, 4, (LPWSTR)pwszParamName);
        ListView_SetItemText(m_hWndList, nItem, 5, (LPWSTR)pwszValue);
        if (ShouldScroll(nCount))
            ListView_EnsureVisible(m_hWndList, nItem, FALSE);        
    }


    return TRUE;
}

//
//    show the select Applications dialog
//
LRESULT CComSpy::OnSelectApplications(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CSelectEventsDlg dlg(&m_map, m_pSysAppInfo, this); 
    dlg.DoModal();
    return 0;
}


LRESULT CComSpy::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE; // continue processing

    //
    //    clean up our application information
    //
    MapStringToAppInfo::iterator iter;
    CAppInfo * pInfo;
    for (iter = m_map.begin(); iter != m_map.end(); ++iter)
    {
    
        pInfo = (*iter).second;        
        delete pInfo;
    }
    m_map.clear();
    
    if(m_pSysAppInfo) 
        delete m_pSysAppInfo;


    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    m_bLogToFile = FALSE;
    if (m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }

    m_cEvents = 0;
    m_bShowGridLines = TRUE;
    return 0;
}
//
//    show only the ChooseFont property page
//
LRESULT CComSpy::OnChooseFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;        
    CComPtr<IUnknown> pUnk;
    ControlQueryInterface(IID_PPV_ARGS(&pUnk));
    _ASSERTE(pUnk != NULL);
    LPCWSTR pwszTitle = L"Com Spy - Choose Font";
    hr = OleCreatePropertyFrame(m_hWnd, m_rcPos.top, m_rcPos.left, pwszTitle,
        1, &pUnk.p, 1, (LPCLSID)&CLSID_StockFontPage, // Ok, OleCreatePropertyFrame will not write to this
        LOCALE_USER_DEFAULT, 0, 0);
    
    return 0;
}

STDMETHODIMP CComSpy::get_LogFile(BSTR * pVal)
{
    *pVal = m_sLogFile.Copy();
    return S_OK;
}

STDMETHODIMP CComSpy::put_LogFile(BSTR newVal)
{

    HANDLE hTemp;
    hTemp = CreateFile(newVal, GENERIC_WRITE,
                        0, (LPSECURITY_ATTRIBUTES) NULL,
                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) NULL);


    if (hTemp)
    {
        if (m_hFile)
        {
            CloseHandle(m_hFile);                        
        }

        m_hFile = hTemp;
    }
    else
        return E_INVALIDARG;

    
    m_sLogFile = newVal;
    return S_OK;
}

STDMETHODIMP CComSpy::get_ShowGridLines(BOOL * pVal)
{
    *pVal = m_bShowGridLines;
    return S_OK;
}

STDMETHODIMP CComSpy::put_ShowGridLines(BOOL newVal)
{
    
    m_bShowGridLines = newVal;
    DWORD dwStyle = ListView_GetExtendedListViewStyle(m_hWndList);
    (m_bShowGridLines) ? dwStyle |= LVS_EX_GRIDLINES : 
                         dwStyle &= ~LVS_EX_GRIDLINES;

    ListView_SetExtendedListViewStyle(m_hWndList, dwStyle);
    FireViewChange();
    return S_OK;
}


STDMETHODIMP CComSpy::get_Audit(BOOL * pVal)
{
    *pVal = m_bAudit;
    return S_OK;
}

STDMETHODIMP CComSpy::put_Audit(BOOL newVal)
{
    
    m_bAudit = newVal;    
    EnableAudit(m_bAudit);
    return S_OK;
}


STDMETHODIMP CComSpy::get_ColWidth(short nColumn, long * pVal)
{
    _ASSERTE(m_hWndList);
    _ASSERTE(nColumn >= 0 && nColumn <= NUMBER_COLUMNS);
    *pVal = ListView_GetColumnWidth(m_hWndList, nColumn);
    return S_OK;
}

STDMETHODIMP CComSpy::put_ColWidth(short nColumn, long newVal)
{        
    if (nColumn >= NUMBER_COLUMNS)
        return E_INVALIDARG;

    m_nWidth[nColumn] = newVal;
    ListView_SetColumnWidth(m_hWndList, nColumn, newVal);
    FireViewChange();
    return S_OK;
}


HRESULT CComSpy::IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap)
{    
    
    ULONG cbRead;
    
    // read in the version
    UINT nVersion;
    pStm->Read(&nVersion, sizeof(UINT), &cbRead);
    if (nVersion != 4)
    {
        ::MessageBox(m_hWndList, L"The saved information is from a different version of the control.\n\nThe saved state will have to be discarded.", L"COM Spy", MB_ICONINFORMATION);
        return S_OK;
    }

    // get log file size
    long lLen;
    pStm->Read(&lLen, sizeof(long), &cbRead);

    // read in the chars for the log name
    char * pLog = new char[lLen+1];
    ZeroMemory(pLog, lLen+1);
    pStm->Read((void*)pLog, lLen, &cbRead);

    m_sLogFile = pLog;
    delete [] pLog;

    // gridlines
    pStm->Read(&m_bShowGridLines, sizeof(m_bShowGridLines),&cbRead);
    put_ShowGridLines(m_bShowGridLines);

    // show on screen    
    pStm->Read(&m_bShowOnScreen, sizeof(m_bShowOnScreen),&cbRead);
    put_ShowOnScreen(m_bShowOnScreen);

    // log to file
    pStm->Read(&m_bLogToFile, sizeof(m_bLogToFile),&cbRead);
    put_LogToFile(m_bLogToFile);

    // csv files
    pStm->Read(&m_bCSV, sizeof(m_bCSV), &cbRead);    
    
    // csv files
    pStm->Read(&m_bAudit, sizeof(m_bAudit), &cbRead);    
    EnableAudit(m_bAudit);

    // read the col widths
    int i;
    long nWidth;
    for (i=0;i<NUMBER_COLUMNS;i++)
    {        
        pStm->Read(&nWidth, sizeof(nWidth), &cbRead);
        put_ColWidth(i, nWidth);
    }

    BOOL bCustomFont = FALSE;
    pStm->Read(&bCustomFont, sizeof(bCustomFont),&cbRead);
    if (bCustomFont)
    {
        if (!m_pFont)
        {
            //create an IFontDisp
            HRESULT hr= CoCreateInstance(CLSID_StdFont, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_pFont));
            _ASSERTE(SUCCEEDED(hr));
        }
        if (m_pFont)
        {
            CComPtr<IPersistStream> spFontSt;
            m_pFont->QueryInterface(IID_PPV_ARGS(&spFontSt));
            _ASSERTE(spFontSt);
            spFontSt->Load(pStm);
            if (m_hFont) // should I tell the list that I deleted its font?
                DeleteObject(m_hFont);
            CComPtr<IFont> spFont;
            m_pFont->QueryInterface(IID_PPV_ARGS(&spFont));
            _ASSERTE(spFont);
            m_hFont = CreateHFontFromIFont(spFont);                    
        }
    }
        

    return S_OK;
}
HRESULT CComSpy::IPersistStreamInit_Save(LPSTREAM pStm, BOOL /* fClearDirty */, const ATL_PROPMAP_ENTRY* pMap)
{
    _ASSERTE(this);
    _ASSERTE(m_hWndList);

    ULONG cbWritten = 0;
    // write a version
    UINT nVersion  = 4;
    pStm->Write(&nVersion, sizeof(UINT), &cbWritten);
    
    // write the log file size
    long nChars = m_sLogFile.Length();
    pStm->Write(&nChars, sizeof(long), &cbWritten);
    _ASSERTE(cbWritten == sizeof(long));

    // write the log file name
    pStm->Write((LPCWSTR)m_sLogFile, m_sLogFile.Length(), &cbWritten);

    // write the BOOL if gridlines should be shown
    pStm->Write(&m_bShowGridLines, sizeof(m_bShowGridLines), &cbWritten);

    // write the BOOL if the user wants to see the messages 
    pStm->Write(&m_bShowOnScreen, sizeof(m_bShowOnScreen), &cbWritten);

    // write the BOOL if the user wants to log the messages
    pStm->Write(&m_bLogToFile, sizeof(m_bLogToFile), &cbWritten);

    // write the BOOL if we have CSV files
    pStm->Write(&m_bCSV, sizeof(m_bCSV), &cbWritten);

    // write the BOOL if Auditing is enabled
    pStm->Write(&m_bAudit, sizeof(m_bAudit), &cbWritten);

    // write the col widths
    int i;
    long nWidth;
    for (i=0;i<NUMBER_COLUMNS;i++)
    {
        get_ColWidth(i, &nWidth);
        _ASSERTE(nWidth);
        pStm->Write(&nWidth, sizeof(nWidth), &cbWritten);
    }
    
    // ask the font to persist itself in the stream
    BOOL bCustomFont;
    if (m_pFont)
    {
        bCustomFont = TRUE;
        pStm->Write(&bCustomFont, sizeof(bCustomFont), &cbWritten);
        CComPtr<IPersistStream> pPersistStream;    
        m_pFont->QueryInterface(IID_PPV_ARGS(&pPersistStream));
        _ASSERTE(pPersistStream);
        pPersistStream->Save(pStm, FALSE);
    }
    else
    {
        bCustomFont = FALSE;
        pStm->Write(&bCustomFont, sizeof(bCustomFont), &cbWritten);
    }

    return S_OK;
}



HRESULT CComSpy::Close( DWORD dwSaveOption )
{
    return IOleObject_Close(dwSaveOption);
}

HFONT CComSpy::CreateHFontFromIFont(__in IFont* pFont)
{

    _ASSERTE(pFont);
    HFONT hFont;    
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    CComBSTR bstr;
    pFont->get_Name(&bstr);
    StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), bstr.m_str);
    CY cy;
    pFont->get_Size(&cy);
    HDC hDC = ::GetDC(NULL);
    _ASSERTE(hDC);
    long lCaps = GetDeviceCaps(hDC, LOGPIXELSY);
    lf.lfHeight =  -MulDiv(cy.Hi, lCaps, 72); 
    lf.lfHeight += -MulDiv(cy.Lo, lCaps, 720000); 
    ::ReleaseDC(NULL, hDC);
    short sVal;
    pFont->get_Weight(&sVal);
    lf.lfWeight  = sVal; 
    BOOL bFlag;
    pFont->get_Italic(&bFlag);
    lf.lfItalic = bFlag; 
    pFont->get_Underline(&bFlag);
    lf.lfUnderline = bFlag; 
    pFont->get_Strikethrough(&bFlag);
    lf.lfStrikeOut = bFlag; 
    pFont->get_Charset(&sVal);
    lf.lfCharSet = (BYTE)sVal; 
    hFont = CreateFontIndirect(&lf);
    return hFont;

}

STDMETHODIMP CComSpy::SelectApplications()
{
    BOOL bHandled = TRUE;
    OnSelectApplications(0,0,0,bHandled);
    return S_OK;
}

STDMETHODIMP CComSpy::SaveToFile()
{
    
    BOOL bHandled = TRUE;
    OnSave(0,0,0,bHandled);
    return S_OK;
}

STDMETHODIMP CComSpy::ClearAllEvents()
{
    
    BOOL bHandled = TRUE;
    OnClear(0,0,0,bHandled);
    return S_OK;
}

STDMETHODIMP CComSpy::About()
{
    BOOL bHandled = TRUE;
    OnAbout(0,0,0,bHandled);
    return S_OK;
}

STDMETHODIMP CComSpy::get_LogToFile(BOOL * pVal)
{
    *pVal = m_bLogToFile;
    return S_OK;
}

STDMETHODIMP CComSpy::put_LogToFile(BOOL newVal)
{
    m_bLogToFile = newVal;
    if (m_bLogToFile)
    {
        if (m_sLogFile.Length() == 0)
        {
            BOOL bHandled = TRUE;
            OnChooseLogFile(0,0,0,bHandled);
            if (m_sLogFile.Length() == 0)
                return 0;
        }

        if (m_hFile)
        {
            CloseHandle(m_hFile);            
            m_hFile = NULL;
        }

        m_hFile = CreateFile(m_sLogFile, GENERIC_WRITE,
                            0, (LPSECURITY_ATTRIBUTES) NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            (HANDLE) NULL);

        if (!m_hFile)
        {
            m_bLogToFile = FALSE;
            return E_FAIL;
        }

    }
    else
    {
        if (m_hFile)
        {
            CloseHandle(m_hFile);
            m_hFile = NULL;
        }

    }
    return S_OK;
}

STDMETHODIMP CComSpy::ChooseFont()
{
    BOOL bHandled = TRUE;
    OnChooseFont(0,0,0,bHandled);
    return S_OK;
}

STDMETHODIMP CComSpy::get_ShowOnScreen(BOOL * pVal)
{
    *pVal = m_bShowOnScreen;
    return S_OK;
}

STDMETHODIMP CComSpy::put_ShowOnScreen(BOOL newVal)
{
    m_bShowOnScreen = newVal;
    return S_OK;
}

STDMETHODIMP CComSpy::ChooseLogFile(BSTR * sLogFileName, BOOL * bCanceled)
{
    *bCanceled = TRUE;
    BOOL bHandled = TRUE;
    OnChooseLogFile(0,0,0,bHandled);
    if (m_sLogFile.Length() == 0)
    {
        return S_OK;
    }

    *sLogFileName = ::SysAllocString(m_sLogFile.m_str);
    *bCanceled = FALSE;
    return S_OK;
}
//
//    loop through the ROT and add Application names
//    to the debug menu
//
bool CComSpy::AddRunningAspsToDebugMenu(HMENU hMenu)
{

//
//    remove the <No Running Applications> menu item
//

    RemoveMenu(hMenu, 0, MF_BYPOSITION);

    HRESULT hr = E_FAIL;
    CComPtr<IMtsGrp> spMtsGrp;
    hr = CoCreateInstance (CLSID_MtsGrp, NULL, CLSCTX_ALL, IID_PPV_ARGS(&spMtsGrp));
    if (!spMtsGrp)
    {
        _ASSERTE(0);
        return false;
    }

    long lApplications;
    spMtsGrp->Refresh(); // its important to call this!
    spMtsGrp->get_Count(&lApplications);
    
    CComPtr<IUnknown> spUnk;
    CComPtr<IMtsEvents> spEvents;
    long lPID;
    WCHAR szBuf[128];
    for (int i=0; i<lApplications; i++)
    {
        spMtsGrp->Item(i, &spUnk);
        _ASSERTE(spUnk);
        spUnk->QueryInterface(IID_PPV_ARGS(&spEvents));
        _ASSERTE(spEvents);
        if (SUCCEEDED(hr))
        {

            CComBSTR bstrName;
            spEvents->get_PackageName(&bstrName.m_str);                    
            spEvents->GetProcessID(&lPID);
            ZeroMemory(szBuf, sizeof(szBuf));
            if (bstrName.m_str)
            {
                StringCchPrintf(szBuf, ARRAYSIZE(szBuf), L"%s - PID: %ld", bstrName.m_str, lPID);                    
                int nRet = InsertMenu(hMenu, -1, MF_BYPOSITION|MF_STRING, ID_DEBUG_BEGIN + i, szBuf);                
            }            
            
        }
        spUnk.Release();
        spEvents.Release();
    }

    return true;
}

//
//    user wants to debug a Application --
//
//    NOTE:   This is designed to work with Microsoft Developer Studio
//            if you want to add support for another tool, modify the 
//            ShellExecute() line below.  This is alson non-robust in 
//            the sense that it requires MSDEV.EXE to be in the path
//            and set up correctly.
//
LRESULT CComSpy::OnDebugApplication(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    HCURSOR hBusy = LoadCursor(NULL, IDC_WAIT);
    HCURSOR hOldCur = SetCursor(hBusy);
    WCHAR szBuf[128];    
    ZeroMemory(szBuf, sizeof(szBuf));
    int nChars;
    _ASSERTE(m_hMenuDebug);
    nChars = GetMenuString(m_hMenuDebug, wID, szBuf, 128, MF_BYCOMMAND);
    if (nChars)
    {
        int nCount = lstrlen(szBuf);
        LPWSTR p = szBuf + nCount;
        while (*p-- != ':' && nCount)
        {
            nCount --;
        }

        if (nCount == 0)
        {
            SetCursor(hOldCur);
            return 0;
        }

        p+=3;        
        WCHAR wszBuffer[MAX_PATH];        
        StringCchPrintf(wszBuffer, MAX_PATH, L"-p %s -e %s", p, p);
        HINSTANCE hInst = ShellExecute( NULL, L"open", L"msdev", wszBuffer, NULL, SW_SHOWNORMAL);
    }
    SetCursor(hOldCur);
    return 0;
}
//
//    Enable/Disable auditing
//
HRESULT CComSpy::EnableAudit(BOOL bEnable)
{

    if (!bEnable)
    {
        m_spSqlAudit.Release();
        return S_OK;
    }

    if (m_spSqlAudit)        // make EnableAudit() idempotent -- we are already enabled
        return S_OK;
    
    HRESULT hr;
    hr = CoCreateInstance(CLSID_ComSpyAudit, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_spSqlAudit));
    if (SUCCEEDED(hr))
    {
        //
        //    these hard coded strings (user and pw) should be looked up in the registry for a production
        //    app
        hr = m_spSqlAudit->Init(L"ComSpyAudit", L"sa", L"");
        if (FAILED(hr))
        {
            LPCWSTR pwszMsg =
                L"Unable to connect to DSN 'ComSpyAudit'.\n"
                L"Make sure that the DSN exists, the Database has the\n"
                L"schema contained in ComSpyAudit.SQL, and you have\n"
                L"the permission to access the database.";
                                                                    
            ::MessageBox(m_hWndList, pwszMsg,  L"ComSpy", MB_ICONSTOP);
            m_spSqlAudit.Release();
        }
    }
    else
    {
        
        ::MessageBox(m_hWndList, L"Unable to create the ComSpyAudit component!",  L"ComSpy", MB_ICONSTOP);            
    }

    return hr;
}

HRESULT CComSpy::putref_Font(__in IFontDisp* pFontDisp)
{
    if (!pFontDisp)
        return E_POINTER;

    CComPtr<IFont> spFont;
    if (SUCCEEDED(pFontDisp->QueryInterface(IID_PPV_ARGS(&spFont))))
    {
        if (m_hFont)
        {
            DeleteObject(m_hFont);
            m_hFont = NULL;
        }
        m_hFont = CreateHFontFromIFont(spFont);
        if (m_hFont)
        {                                
            ::SendMessage(m_hWndList, WM_SETFONT, (WPARAM)m_hFont, 0L);
        }
    }

    if (FireOnRequestEdit(DISPID_FONT) == S_FALSE)
        return S_FALSE;
    m_pFont = pFontDisp;
    m_bRequiresSave = TRUE;
    FireOnChanged(DISPID_FONT);
    FireViewChange();
    return S_OK;
}