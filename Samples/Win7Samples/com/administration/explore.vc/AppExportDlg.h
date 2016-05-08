// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_APPEXPORTDLG_H__0B7BB738_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_APPEXPORTDLG_H__0B7BB738_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CAppExportDlg dialog

class CAppExportDlg : public CDialog
{
// Construction
public:
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CAppExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAppExportDlg)
	enum { IDD = IDD_EXPORT_APPLICATION };
	CEdit	m_edtExportPath;
	CEdit	m_edtApplicationID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAppExportDlg)
	afx_msg void OnClose();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnExportWithoutUsers();
	afx_msg void OnExportClients();
	afx_msg void OnExportUsers();
	afx_msg void OnOverwriteFiles();
	afx_msg void OnExportPathSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_bOverwriteFiles;
	int m_nExportType;
	void ReleaseCatalog();
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPEXPORTDLG_H__0B7BB738_B53B_11D2_86A8_000000000000__INCLUDED_)
