// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_COMPIMPORT_H__0B7BB739_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_COMPIMPORT_H__0B7BB739_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCompImportDlg dialog

class CCompImportDlg : public CDialog
{
// Construction
public:
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CCompImportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompImportDlg)
	enum { IDD = IDD_IMPORT_COMPONENT };
	CEdit	m_edtID;
	CStatic	m_lblComponentDesc;
	CComboBox	m_cboApplications;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCompImportDlg)
	afx_msg void OnClose();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCLSID();
	afx_msg void OnProgID();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void LoadAppCombo();
	int m_nImportType;
	void ReleaseCatalog();
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPIMPORT_H__0B7BB739_B53B_11D2_86A8_000000000000__INCLUDED_)
