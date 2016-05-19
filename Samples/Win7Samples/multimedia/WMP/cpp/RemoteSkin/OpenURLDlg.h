// OpenURLDlg.h : Declaration of the COpenURLDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#ifndef __OPENURLDLG_H_
#define __OPENURLDLG_H_

#include "resource.h"       // main symbols
#include <atlhost.h>

/////////////////////////////////////////////////////////////////////////////
// COpenURLDlg
class COpenURLDlg : 
    public CAxDialogImpl<COpenURLDlg>
{
public:
    COpenURLDlg();
    ~COpenURLDlg(){}

    enum { IDD = IDD_OPENURLDLG };

BEGIN_MSG_MAP(COpenURLDlg)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
END_MSG_MAP()

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    CComBSTR    m_bstrURL;
};

#endif //__OPENURLDLG_H_
