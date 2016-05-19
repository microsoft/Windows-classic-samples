/////////////////////////////////////////////////////////////////////////////
//
// CPropertyDialog.h : Declaration of the CPropertyDialog
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "atlwin.h"

class CPropertyDialog : public CDialogImpl<CPropertyDialog>
{
public:
    enum { IDD = IDD_PROPERTYDIALOG };

    BEGIN_MSG_MAP(CPropertyDialog)
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        COMMAND_ID_HANDLER( IDOK, OnOK )
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
    END_MSG_MAP()

    CPropertyDialog(C[!output Safe_root] *pPlugin)
    {
        m_pPlugin = pPlugin;
    }

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
    {
        CenterWindow();
        // initialize text
        SetDlgItemText(IDC_MESSAGEEDIT, m_pPlugin->m_wszPluginText);
        return 1;
    }

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
    {
        // save text
        GetDlgItemText(IDC_MESSAGEEDIT, m_pPlugin->m_wszPluginText, sizeof(m_pPlugin->m_wszPluginText) / sizeof(m_pPlugin->m_wszPluginText[0]));
        EndDialog( IDOK );
        return 0;
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hwndCtl, BOOL& fHandled)
    {
        EndDialog( IDCANCEL );
        return 0;
    }

private:
    C[!output Safe_root]  *m_pPlugin;  // pointer to plugin object
};

