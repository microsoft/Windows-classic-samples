// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

class CMainDlg;

class CTabWnd
{
public:
    static ATOM RegisterClass();
    static CTabWnd* Create(int i, CMainDlg *pMainDlg);

    HWND GetHwnd() { return _hwnd; }

    VOID Destroy();

protected:
    HWND                _hwnd;
    int                 _iTab;
    CMainDlg *          _pMainDlg;

    CTabWnd(int i, CMainDlg *pMainDlg) : _hwnd(NULL), _iTab(i), _pMainDlg(pMainDlg) {};

    HRESULT _SendIconicRepresentation(int nWidth, int nHeight);
    HRESULT _SendLivePreviewBitmap();
    HBITMAP _CreateDIB(int nWidth, int nHeight);

    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
};
