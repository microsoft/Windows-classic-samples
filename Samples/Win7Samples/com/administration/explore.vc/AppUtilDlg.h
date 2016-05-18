// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_APPUTILDLG_H__0B7BB736_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_APPUTILDLG_H__0B7BB736_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define UTILITY_TYPE_UNDEFINED		-1
#define UTILITY_TYPE_START_APP		0
#define UTILITY_TYPE_STOP_APP		1


/////////////////////////////////////////////////////////////////////////////
// CAppUtilDlg dialog

class CAppUtilDlg : public CDialog
{
// Construction
public:
	void set_UtilityType(int UtilityType);
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CAppUtilDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAppUtilDlg)
	enum { IDD = IDD_APP_UTILITY };
	CEdit	m_edtApp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppUtilDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAppUtilDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAppSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ReleaseCatalog();
	int			m_nUtilityType;
	ICOMAdminCatalog*	m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPUTILDLG_H__0B7BB736_B53B_11D2_86A8_000000000000__INCLUDED_)
