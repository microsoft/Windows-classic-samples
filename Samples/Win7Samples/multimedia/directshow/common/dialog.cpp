//-----------------------------------------------------------------------------
// File: Dialog.cpp
// Desc: Dialog class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "wincontrol.h"
#include "dialog.h"


//-----------------------------------------------------------------------------
// Name: ShowLastError
// Description: Display the last error that was set.
//-----------------------------------------------------------------------------

void ShowLastError(HWND hwnd)
{
    LPVOID lpMsgBuf;
    if (FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
        MessageBox( hwnd, (LPCWSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
    }
}



//-----------------------------------------------------------------------------
// Name: CBaseDialog()
// Desc: Constructor
// 
// nID: Resource ID of the dialog
//-----------------------------------------------------------------------------

CBaseDialog::CBaseDialog(int nID)
: m_nID(nID), m_hDlg(0), m_hinst(0), m_hwnd(0),
  m_NcTop(0), m_NcBottom(0), m_NcWidth(0)
{
}


CBaseDialog::~CBaseDialog()
{
}


//-----------------------------------------------------------------------------
// Name: ShowDialog()
// Desc: Displays the dialog
//
// hinst: Application instance
// hwnd:  Handle to the parent window. Use NULL if no parent.
//
// Returns TRUE if successful or FALSE otherwise
//-----------------------------------------------------------------------------
BOOL CBaseDialog::ShowDialog(HINSTANCE hinst, HWND hwnd)
{
    // Cache these...
    m_hinst = hinst;
    m_hwnd = hwnd;

    // Show the dialog. Pass a pointer to ourselves as the LPARAM
    INT_PTR ret = DialogBoxParam(hinst, MAKEINTRESOURCE(m_nID), 
        hwnd, DialogProc, (LPARAM)this);

    if (ret == 0 || ret == -1)
    {
        ShowLastError(NULL);
        return FALSE;
    }

    return (IDOK == ret);
}


//-----------------------------------------------------------------------------
// Name: DialogProc()
// Desc: DialogProc for the dialog. This is a static class method.
//
// lParam: Pointer to the CBaseDialog object. 
//
// The CBaseDialog class specifies lParam when it calls DialogBoxParam. We store the 
// pointer as user data in the window. 
//
// (Note: The DirectShow CBasePropertyPage class uses the same technique.)
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CBaseDialog::DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CBaseDialog *pDlg = 0;  // Pointer to the dialog class that manages the dialog 

    if (msg == WM_INITDIALOG)
    {
        // Get the pointer to the dialog object and store it in 
        // the window's user data

        _SetWindowLongPtr(hDlg, DWLP_USER, lParam);

        pDlg = (CBaseDialog*)lParam;
        if (pDlg)
        {
            pDlg->m_hDlg = hDlg;
            pDlg->CalcNcSize();
            HRESULT hr = pDlg->OnInitDialog();
            if (FAILED(hr))
            {
                pDlg->EndDialog(0);
            }
        }
        return FALSE;
    }

    // Get the dialog object from the window's user data
    pDlg = _GetWindowLongPtr<CBaseDialog*>(hDlg, DWLP_USER);

    if (pDlg != NULL)
    {
        switch (msg)
        {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDOK:
                if (pDlg->OnOK())
                {
                    pDlg->EndDialog(IDOK);
                }
                return TRUE;

            case IDCANCEL:
                if (pDlg->OnCancel())
                {
                    pDlg->EndDialog(IDCANCEL);
                }
                return TRUE;

            default:
                return pDlg->OnCommand((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
            }
            break;

        case WM_NOTIFY:
            return pDlg->OnNotify((NMHDR*)lParam);

        default:
            return pDlg->OnReceiveMsg(msg, wParam, lParam);
        }
    }
    else
    {
        return FALSE;
    }
}


//-----------------------------------------------------------------------------
// Name: CalcNcSize()
// Desc: Calculate the non-client top, bottom, and width
//-----------------------------------------------------------------------------

void CBaseDialog::CalcNcSize()
{
    LONG_PTR dwStyles = GetWindowLongPtr(m_hDlg, GWL_STYLE);

    m_NcTop = 0;
    m_NcBottom = 0;
    m_NcWidth = 0;

    if (dwStyles & WS_SIZEBOX)
    {
        m_NcTop += GetSystemMetrics(SM_CYSIZEFRAME);
        m_NcBottom += GetSystemMetrics(SM_CYSIZEFRAME);
        m_NcWidth += GetSystemMetrics(SM_CXSIZEFRAME);
    }
    else if (dwStyles & WS_BORDER)
    {
        m_NcTop += GetSystemMetrics(SM_CYBORDER);
        m_NcBottom += GetSystemMetrics(SM_CYBORDER);
        m_NcWidth += GetSystemMetrics(SM_CXBORDER);
    }

    if (dwStyles & WS_CAPTION)
    {
        m_NcTop += GetSystemMetrics(SM_CYCAPTION);
    }
    if (GetMenu(m_hDlg))
    {
        m_NcTop += GetSystemMetrics(SM_CYMENU);
    }
}


//-----------------------------------------------------------------------------
// Name: SetControlWindow
// Desc: Associate a Control object with a control window.
//
// nID: Resource ID of the control
//-----------------------------------------------------------------------------

void CBaseDialog::SetControlWindow(Control& control, int nID)
{
    control.SetWindow(GetDlgItem(nID));
}


//-----------------------------------------------------------------------------
// Name: RedrawControl()
// Desc: Repaints a control
//
// nID: Resource ID of the control
//-----------------------------------------------------------------------------

void CBaseDialog::RedrawControl(int nID)
{
    // Find the dialog rect and the control rect, both relative to the display
    RECT rcDlg, rcControl;
    GetWindowRect(m_hDlg, &rcDlg);
    GetWindowRect(GetDlgItem(nID), &rcControl);

    // Adjust the dialog rect by the size of the border and caption 
    rcDlg.top += NonClientTop();
    rcDlg.left += NonClientWidth();

    // Find the dialog rect relative to the dialog position
    OffsetRect(&rcControl, - rcDlg.left, - rcDlg.top);

    InvalidateRect(m_hDlg, &rcControl, TRUE);
    UpdateWindow(m_hDlg);
}



