//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_SERVERDLG_H__C090CEFA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_SERVERDLG_H__C090CEFA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <wtsapi32.h>

// ServerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

class CServerDlg : public CDialog
{
// Construction
public:
	BOOL domainFlag;
	CString serverName;
	DWORD count;
	PWTS_SERVER_INFO pServerInfo;
	
	// standard constructor
	CServerDlg(CWnd* pParent = NULL);   

// Dialog Data
	//{{AFX_DATA(CServerDlg)
	enum { IDD = IDD_SERVERDLG };
	CComboBox	m_serverList2;
	CEdit	m_domainName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CServerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDropdownServerList2();
	afx_msg void OnChangeDomainName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__C090CEFA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
