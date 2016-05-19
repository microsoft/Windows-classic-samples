//-----------------------------------------------------------------------------
// File: Dialog.h
// Desc: Dialog class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

// Misc helper functions

HINSTANCE	GetInstance();
HBITMAP		SetBitmapImg(HINSTANCE hinst, WORD nID, HWND hwnd);
void        ShowLastError(HWND hwnd);



/******************************************************************************
 *
 *  CBaseDialog Class
 *  Implements a Win32 modal dialog. 
 *
 *  Examle of usage:
 *
 *  Derive a new class from CBaseDialog
 *      class CMyDialog : public CBaseDialog
 *
 *  Create an instance of the class:
 *	    CMyDialog *pDlg = new CMyDialog();
 *
 *  Call ShowDialog to show the dialog:
 *      if (pDlg)
 *	    {
 *          pDlg->ShowDialog(m_hinst, m_hwndParent); 
 *      }
 *
 *****************************************************************************/

class CBaseDialog
{

private:
	LONG		m_NcTop;
	LONG		m_NcBottom;
	LONG		m_NcWidth;
	void		CalcNcSize();

protected:
    HINSTANCE   m_hinst;    // application instance
    HWND        m_hwnd;     // parent window - can be NULL
    HWND        m_hDlg;     // this dialog window
    int         m_nID;      // Resource ID of the dialog window 
                            // (Set this in the constructor)

protected:

    // Dialog proc for the dialog we manage
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Return one of our dialog controls
	HWND GetDlgItem(int nID) { return ::GetDlgItem(m_hDlg, nID); }

    void EnableWindow(int nID, BOOL bEnable)
    {
        HWND hControl = GetDlgItem(nID);
        assert(hControl != NULL);

        if (!bEnable &&  hControl == GetFocus())
        {
            // If we're being disabled and this control has focus,
            // set the focus to the next control.

            ::SendMessage(m_hDlg, WM_NEXTDLGCTL, 0, FALSE);
        }

        ::EnableWindow(hControl, bEnable);
    }

    // Associate a Control object with a control window.
    void SetControlWindow(Control& control, int nID);

    // Redraw a control
	void RedrawControl(int nID);


    // some wrappers for Win32 functions
    void EnableMenuItem(int nID, BOOL bEnable)
    {
        ::EnableMenuItem(GetMenu(m_hDlg), nID, (bEnable ? MF_ENABLED : MF_GRAYED));
    }


    BOOL SetDlgItemText(int nIDDlgItem, LPCTSTR lpString)
    {
        return ::SetDlgItemText(m_hDlg, nIDDlgItem, lpString);
    }

    UINT GetDlgItemText(int nIDDlgItem, LPTSTR lpString, int nMaxCount)
    {
        return ::GetDlgItemText(m_hDlg,  nIDDlgItem, lpString, nMaxCount);
    }

    BOOL SetDlgItemInt(int nIDDlgItem, UINT uValue, BOOL bSigned)
    {
        return ::SetDlgItemInt(m_hDlg, nIDDlgItem, uValue, bSigned);
    }

    UINT GetDlgItemInt(int nIDDlgItem, BOOL *lpTranslated, BOOL bSigned)
    {
        return ::GetDlgItemInt(m_hDlg, nIDDlgItem, lpTranslated, bSigned);
    }

    BOOL CheckDlgButton(int nIDButton, UINT uCheck)
    {
        return ::CheckDlgButton(m_hDlg, nIDButton, uCheck);
    }

    UINT IsDlgButtonChecked(int nIDButton)
    {
        return ::IsDlgButtonChecked(m_hDlg, nIDButton);
    }


    // Override the following to handle various window messages

    // WM_INIT_DIALOG
    virtual HRESULT OnInitDialog() { return S_OK; }   

    // IDOK and IDCANCEL. Return TRUE to close the dialog or FALSE to leave it open
    virtual BOOL OnOK() { return TRUE; }
    virtual BOOL OnCancel() { return TRUE; }

    // WM_COMMAND (except IDOK and IDCANCEL)
    virtual INT_PTR OnCommand(HWND /*hControl*/, WORD /*idControl*/, WORD /*msg*/)
    {
        return 0;
    }

    // WM_NOTIFY
    virtual INT_PTR OnNotify(NMHDR * /*pNotifyHeader*/)
    {
        return 0;
    }

    // All other window messages
    virtual INT_PTR OnReceiveMsg(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        return FALSE;
    }

	virtual void EndDialog(INT_PTR cmd)
	{
		if (m_hDlg)
		{
			::EndDialog(m_hDlg, cmd);
		}
	}

public:
    CBaseDialog(int nID);
    virtual ~CBaseDialog();

    virtual BOOL ShowDialog(HINSTANCE hinst, HWND hwnd);

	LONG NonClientTop() const { return m_NcTop; }
	LONG NonClientBottom() const { return m_NcBottom; }
	LONG NonClientWidth() const { return m_NcWidth; }
};




