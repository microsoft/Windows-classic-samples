// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
                       "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <windowsx.h>
#include <shobjidl.h>
#include <dwmapi.h>
#include <strsafe.h>

#include "TabApp.h"
#include "TabWnd.h"

HINSTANCE g_hInstance = NULL;

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE /* hPrevInstance */,
    LPWSTR /* lpCmdLine */,
    int /* nShowCmd */)
{
    int retVal = -1;
    g_hInstance = hInstance;

    HRESULT hrInit = CoInitialize(NULL);
    if (SUCCEEDED(hrInit))
    {
        // Register window classes
        CTabWnd::RegisterClass();

        // Allow DWMSENDICONICTHUMBNAIL even if running in high rights.
        ChangeWindowMessageFilter(WM_DWMSENDICONICTHUMBNAIL, MSGFLT_ADD);
        ChangeWindowMessageFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, MSGFLT_ADD);

        // Display main application dialog
        CMainDlg dlg;
        HRESULT hr = dlg.Initialize();
        if (SUCCEEDED(hr))
        {
            dlg.DoModal();
            retVal = 0;
        }

        CoUninitialize();
    }

    return retVal;
}

CMainDlg::~CMainDlg()
{
    _CleanUp();
}

HRESULT CMainDlg::Initialize()
{
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_pTBL));
    if (SUCCEEDED(hr))
    {
        hr = _pTBL->HrInit();
    }
    return hr;
}


INT_PTR CMainDlg::DoModal()
{
    if (_hwnd == NULL)
    {
        return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, CMainDlg::_DlgProc, (LONG_PTR)this);
    }
    else
    {
        return IDCANCEL;
    }
}

INT_PTR CALLBACK CMainDlg::_DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CMainDlg* pDlg = (CMainDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pDlg == NULL && message == WM_INITDIALOG)
    {
        pDlg = (CMainDlg*)lParam;
        pDlg->_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDlg);
    }

    if (pDlg != NULL)
    {
        return pDlg->DlgProc(message, wParam, lParam);
    }
    else
    {
        return TRUE;
    }
}

static UINT g_mDragList = 0;

INT_PTR CMainDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nResult = FALSE;

    switch (msg)
    {
        case WM_INITDIALOG:
            // Set up the listbox to allow drag and drop
            if (MakeDragList(GetDlgItem(_hwnd, IDC_TABLIST)))
            {
                g_mDragList = RegisterWindowMessage(DRAGLISTMSGSTRING);
            }

            nResult = TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_ADD:
                    _AddTab();
                    break;

                case IDC_REMOVE:
                    _RemoveSelectedTab();
                    break;

                case IDC_INVALIDATE:
                    _InvalidateSelectedTab();
                    break;

                case IDC_DEACTIVATE:
                    _DeactivateTab();
                    break;

                case IDC_MOVEPREV:
                    _MoveTabByOffset(_GetSelectedTab(), -1);
                    break;

                case IDC_MOVENEXT:
                    _MoveTabByOffset(_GetSelectedTab(), 2);
                    break;

                case IDOK:
                    _CleanUp();
                    EndDialog(_hwnd, LOWORD(wParam));
                    nResult = TRUE;
                    break;

                case IDC_TABLIST:
                    switch (HIWORD(wParam))
                    {
                        case LBN_SELCHANGE:
                            _ActivateSelectedTab();
                            break;
                    }
                    break;
            }
            break;

        default:
            if (msg == g_mDragList)
            {
                DRAGLISTINFO *pdli = (DRAGLISTINFO *)lParam;
                switch (pdli->uNotification)
                {
                    case DL_BEGINDRAG:
                        _iDrag = LBItemFromPt(pdli->hWnd, pdli->ptCursor, TRUE);
                        DrawInsert(_hwnd, pdli->hWnd, _iDrag);

                        // Set the return value to allow the drag to contine
                        SetDlgMsgResult(_hwnd, g_mDragList, TRUE);
                        break;

                    case DL_CANCELDRAG:
                        DrawInsert(_hwnd, pdli->hWnd, -1);
                        break;

                    case DL_DRAGGING:
                    {
                        int iDragOver = LBItemFromPt(pdli->hWnd, pdli->ptCursor, TRUE);
                        DrawInsert(_hwnd, pdli->hWnd, iDragOver);

                        // Set the return value to show feedback
                        SetDlgMsgResult(_hwnd, g_mDragList, (iDragOver != -1) ? DL_MOVECURSOR : DL_STOPCURSOR);
                        break;
                    }

                    case DL_DROPPED:
                    {
                        int iDragOver = LBItemFromPt(pdli->hWnd, pdli->ptCursor, TRUE);
                        DrawInsert(_hwnd, pdli->hWnd, -1);

                        _MoveTab(_iDrag, iDragOver);
                        _ActivateSelectedTab();
                        break;
                    }
                }
                break;
            }
            break;
    }

    return nResult;
}

void CMainDlg::ActivateTab(CTabWnd *pTabWnd)
{
    int iTab = _FindTab(pTabWnd);
    if (iTab >= 0 && iTab < _cTabs)
    {
        _pTBL->SetTabActive(_apTabs[iTab]->GetHwnd(), GetHwnd(), 0);
        _LBSelectTab(iTab);
    }
    SetForegroundWindow(_hwnd);
}

void CMainDlg::DestroyTab(CTabWnd *pTabWnd)
{
    int iTab = _FindTab(pTabWnd);
    _RemoveTab(iTab);
}

void CMainDlg::RegisterTab(CTabWnd *pTabWnd)
{
    _pTBL->RegisterTab(pTabWnd->GetHwnd(), GetHwnd());
}
void CMainDlg::UnregisterTab(CTabWnd *pTabWnd)
{
    _pTBL->UnregisterTab(pTabWnd->GetHwnd());
}

void CMainDlg::_CleanUp()
{
    for (int i = 0; i < _cTabs; i++)
    {
        _apTabs[i]->Destroy();
        delete _apTabs[i];
    }
    _cTabs = 0;

    if (_pTBL)
    {
        _pTBL->Release();
        _pTBL = NULL;
    }
}

void CMainDlg::_LBInsertTab(int iTab)
{
    if (iTab >= 0 && iTab < _cTabs)
    {
        CTabWnd* pTabWnd = _apTabs[iTab];
        WCHAR szTab[100];
        GetWindowText(pTabWnd->GetHwnd(), szTab, ARRAYSIZE(szTab));
        SendDlgItemMessage(_hwnd, IDC_TABLIST, LB_INSERTSTRING, iTab, (LPARAM)szTab);
    }
}

void CMainDlg::_LBRemoveTab(int iTab)
{
    SendDlgItemMessage(_hwnd, IDC_TABLIST, LB_DELETESTRING, iTab, 0);
}

void CMainDlg::_LBSelectTab(int iTab)
{
    SendDlgItemMessage(_hwnd, IDC_TABLIST, LB_SETCURSEL, iTab, 0);
}

void CMainDlg::_AddTab()
{
    // Create a new tab and:
    //    1) Insert it before the selected tab
    // or 2) Append it
    if (_cTabs < MAXTABS)
    {
        CTabWnd* pTabWnd = CTabWnd::Create(_nNextTab, this);
        if (pTabWnd != NULL)
        {
            int iNewTab = _cTabs;
            _cTabs++;
            _nNextTab++;

            _apTabs[iNewTab] = pTabWnd;
            int iTab = _GetSelectedTab();
            _LBInsertTab(iNewTab);

            if (iTab == LB_ERR)
            {
                _pTBL->SetTabOrder(pTabWnd->GetHwnd(), NULL);
            }
            else
            {
                _MoveTab(iNewTab, iTab);
            }

            _ActivateSelectedTab();
        }
    }
}

void CMainDlg::_RemoveTab(int iTab)
{
    if (iTab >= 0 && iTab < _cTabs)
    {
        _LBRemoveTab(iTab);
        _apTabs[iTab]->Destroy();
        delete _apTabs[iTab];

        if (iTab < _cTabs - 1)
        {
            memmove(&_apTabs[iTab], &_apTabs[iTab+1], (_cTabs - iTab - 1) * sizeof(*_apTabs));
        }
        _cTabs--;
    }
}

void CMainDlg::_MoveTabByOffset(int iTab, int iTabOffset)
{
    _MoveTab(iTab, iTab + iTabOffset);
}

void CMainDlg::_MoveTab(int iTab, int iTabTo)
{
    if (iTab >= 0 && iTab < _cTabs &&
        iTabTo >= 0 && iTabTo <= _cTabs &&
        iTab != iTabTo)
    {
        CTabWnd *pTabWnd = _apTabs[iTab];

        HWND hwndTo = NULL;
        if (iTabTo < _cTabs)
        {
            hwndTo = _apTabs[iTabTo]->GetHwnd();
        }
        _pTBL->SetTabOrder(pTabWnd->GetHwnd(), hwndTo);

        if (iTab < iTabTo)
        {
            // Shift elements between iMove and iMoveTo left by one, writing over iMove.
            memmove(&_apTabs[iTab], &_apTabs[iTab+1], (iTabTo - iTab) * sizeof(*_apTabs));
            iTabTo--;
        }
        else if (iTab > iTabTo)
        {
            // Shift elements between iMoveTo+1 and iMove right by one, writing over iMove.
            memmove(&_apTabs[iTabTo+1], &_apTabs[iTabTo], (iTab - iTabTo) * sizeof(*_apTabs));
        }
        _apTabs[iTabTo] = pTabWnd;

        _LBRemoveTab(iTab);
        _LBInsertTab(iTabTo);
        _LBSelectTab(iTabTo);
    }
}

void CMainDlg::_ActivateSelectedTab()
{
    int iTab = static_cast<int>(SendDlgItemMessage(_hwnd, IDC_TABLIST, LB_GETCURSEL, 0, 0));
    if (iTab >= 0 && iTab < _cTabs)
    {
        _pTBL->SetTabActive(_apTabs[iTab]->GetHwnd(), GetHwnd(), 0);
    }
    else
    {
        _pTBL->SetTabActive(NULL, GetHwnd(), 0);
    }
}

void CMainDlg::_InvalidateTab(int iTab)
{
    if (iTab >= 0 && iTab < _cTabs)
    {
        DwmInvalidateIconicBitmaps(_apTabs[iTab]->GetHwnd());
    }
}

void CMainDlg::_DeactivateTab()
{
    _LBSelectTab(-1);
    _ActivateSelectedTab();
}

int CMainDlg::_GetSelectedTab()
{
    return static_cast<int>(SendDlgItemMessage(_hwnd, IDC_TABLIST, LB_GETCURSEL, 0, 0));
}

int CMainDlg::_FindTab(CTabWnd *pTabWnd)
{
    int iTab = -1;

    for (int i = 0; i < _cTabs; i++)
    {
        if (_apTabs[i] == pTabWnd)
        {
            iTab = i;
            break;
        }
    }

    return iTab;
}
