//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_PROCESSDLG_H__1A55D422_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_PROCESSDLG_H__1A55D422_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <wtsapi32.h>

// ProcessDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog

class CProcessDlg : public CDialog
{
// Construction
public:
	PWTS_PROCESS_INFO pProcessInfo;
	CProcessDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcessDlg)
	enum { IDD = IDD_PROCESS };
	CEdit	m_serverName;
	CListBox	m_processList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessDlg)
	afx_msg void OnTerminateProcess();
	afx_msg void Refresh();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD count;
	HANDLE serverHandle;
	LPSTR serverName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSDLG_H__1A55D422_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_)
