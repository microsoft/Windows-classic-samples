// QueryDlg.h : Declaration of the CQueryDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include "stdafx.h"


/////////////////////////////////////////////////////////////////////////////
// CQueryDlg
class CQueryDlg : 
    public CAxDialogImpl<CQueryDlg>
{
public:
    CQueryDlg(IWMPMediaCollection2* pMC);
    ~CQueryDlg();

    enum { IDD = IDD_QUERYDLG };

BEGIN_MSG_MAP(CQueryDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_ADDCONDITION, OnAddCondition)
    COMMAND_ID_HANDLER(IDC_ADDGROUP, OnAddGroup)
    COMMAND_ID_HANDLER(IDC_SHOWSC, OnShowSC)
    COMMAND_ID_HANDLER(IDC_STARTOVER, OnStartOver)
END_MSG_MAP()

    LRESULT OnInitDialog        (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel            (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAddCondition      (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAddGroup          (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnShowSC            (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnStartOver         (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    HRESULT GetQuery            (IWMPQuery** ppQuery);

private:
    HWND                        m_hQueryList;
    CComPtr<IWMPMediaCollection2>       m_spMC;
    CComPtr<IWMPQuery>                  m_spQuery;
};
