// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include "resource.h"
#include "tabwnd.h"

extern HINSTANCE g_hInstance;

#define RINGWIDTH 10
#define MAXRING 5
#define MAXTABS 20

class CMainDlg
{
public:
    CMainDlg() : _hwnd(NULL), _iDrag(0), _nNextTab(1), _cTabs(0), _pTBL(NULL)  {};
    ~CMainDlg();

    HRESULT Initialize();
    INT_PTR DoModal();

    HWND GetHwnd() { return _hwnd; }

    void RegisterTab(CTabWnd *pTabWnd);
    void UnregisterTab(CTabWnd *pTabWnd);
    void ActivateTab(CTabWnd *pTabWnd);
    void DestroyTab(CTabWnd *pTabWnd);

protected:
    HWND _hwnd;
    int _iDrag;
    int _nNextTab;
    CTabWnd* _apTabs[MAXTABS];
    int _cTabs;

    ITaskbarList4* _pTBL;

    void _CleanUp();

    int  _GetSelectedTab();
    int  _FindTab(CTabWnd *pTabWnd);

    // Listbox maintenance functions
    void _LBInsertTab(int iTab);
    void _LBRemoveTab(int iTab);
    void _LBSelectTab(int iTab);

    // Tab maintenance functions
    void _RemoveSelectedTab() { _RemoveTab(_GetSelectedTab()); }
    void _InvalidateSelectedTab() { _InvalidateTab(_GetSelectedTab()); }
    void _ActivateSelectedTab();

    void _AddTab();
    void _RemoveTab(int iTab);
    void _MoveTab(int iTab, int iTabTo);
    void _MoveTabByOffset(int iTab, int iTabOffsetTo);
    void _InvalidateTab(int iTab);
    void _DeactivateTab();

    static INT_PTR CALLBACK _DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};
