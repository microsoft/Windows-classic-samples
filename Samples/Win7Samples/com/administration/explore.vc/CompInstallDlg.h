// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_COMPINSTALLDLG_H__0B7BB73B_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_COMPINSTALLDLG_H__0B7BB73B_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCompInstallDlg dialog

class CCompInstallDlg : public CDialog
{
// Construction
public:
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CCompInstallDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompInstallDlg)
	enum { IDD = IDD_INSTALL_COMPONENT };
	CComboBox	m_cboApplications;
	CEdit	m_edtTypeLib;
	CEdit	m_edtProxyStub;
	CEdit	m_edtComponent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompInstallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCompInstallDlg)
	afx_msg void OnClose();
	afx_msg void OnComponentSearch();
	afx_msg void OnTypeLibrary();
	afx_msg void OnProxyStubDll();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ReleaseCatalog();
	void LoadAppCombo();
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPINSTALLDLG_H__0B7BB73B_B53B_11D2_86A8_000000000000__INCLUDED_)
