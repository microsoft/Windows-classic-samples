// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "VCExploreDlg.h"

#if !defined(AFX_CONNECTDLG_H__0B7BB737_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_CONNECTDLG_H__0B7BB737_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog

class CConnectDlg : public CDialog
{
// Construction
public:
	void set_ParentDlg(CVCExploreDlg* pDlg);
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CConnectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConnectDlg)
	enum { IDD = IDD_CONNECT };
	CEdit	m_edtRemoteComputerName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CConnectDlg)
	virtual void OnOK();
	afx_msg void OnRemoteComputer();
	afx_msg void OnLocalComputer();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ReleaseCatalog();
	CVCExploreDlg* m_pParentDlg;
	void ToggleEditControl(bool Flag);
	int m_nConnectionType;
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTDLG_H__0B7BB737_B53B_11D2_86A8_000000000000__INCLUDED_)
