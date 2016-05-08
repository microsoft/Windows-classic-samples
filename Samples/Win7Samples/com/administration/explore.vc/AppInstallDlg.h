// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_APPINSTALLDLG_H__0B7BB73A_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_APPINSTALLDLG_H__0B7BB73A_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAppInstallDlg dialog

class CAppInstallDlg : public CDialog
{
// Construction
public:
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CAppInstallDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAppInstallDlg)
	enum { IDD = IDD_INSTALL_APPLICATION };
	CEdit	m_edtUID;
	CEdit	m_edtPWD;
	CEdit	m_edtRemoteServerName;
	CEdit	m_edtInstallDir;
	CEdit	m_edtAppFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppInstallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAppInstallDlg)
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnAppFileSearch();
	afx_msg void OnInstallDirSearch();
	virtual BOOL OnInitDialog();
	afx_msg void OnInstallWithUsers();
	afx_msg void OnInstallWithoutUsers();
	afx_msg void OnOverwriteFiles();
	afx_msg void OnRemoteServerInstall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ToggleRemoteServerState(bool Flag);
	bool m_bRemoteInstall;
	bool m_bOverwriteFiles;
	int m_nInstallType;
	void ReleaseCatalog();
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPINSTALLDLG_H__0B7BB73A_B53B_11D2_86A8_000000000000__INCLUDED_)
