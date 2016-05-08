// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_UTILITIESDLG_H__0B7BB73C_B53B_11D2_86A8_000000000000__INCLUDED_)
#define AFX_UTILITIESDLG_H__0B7BB73C_B53B_11D2_86A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUtilitiesDlg dialog

class CUtilitiesDlg : public CDialog
{
// Construction
public:
	void set_Catalog(ICOMAdminCatalog* pCatalog);
	CUtilitiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUtilitiesDlg)
	enum { IDD = IDD_UTILITIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUtilitiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUtilitiesDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnJNEC();
	afx_msg void OnRefresh();
	afx_msg void OnRouter();
	afx_msg void OnStart();
	afx_msg void OnStop();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ReleaseCatalog();
	int m_nUtilityOption;
	int m_nUtilityType;
	ICOMAdminCatalog* m_pCatalog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UTILITIESDLG_H__0B7BB73C_B53B_11D2_86A8_000000000000__INCLUDED_)
