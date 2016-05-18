//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_SESSIONDLG_H__1A55D424_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_SESSIONDLG_H__1A55D424_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <wtsapi32.h>

// SessionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSessionDlg dialog

class CSessionDlg : public CDialog
{
// Construction
public:
	void Refresh();
	PWTS_SESSION_INFO pSessionInfo;
	CSessionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSessionDlg)
	enum { IDD = IDD_SESSION };
	CListCtrl	m_sessionList;
	CEdit	m_serverName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSessionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetSessionID();

	// Generated message map functions
	//{{AFX_MSG(CSessionDlg)
	afx_msg void OnDisconnectSession();
	afx_msg void OnLogoffSession();
	afx_msg void OnQuerySession();
	virtual BOOL OnInitDialog();
	afx_msg void OnVirtualChannel();
	afx_msg void OnRefresh();
	virtual void OnCancel();
	afx_msg void OnSetsession();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void GetSessionID();
	CString connectState[10];
	CString WinStationName;
	HANDLE serverHandle;
	LPSTR serverName;
	DWORD count;
	DWORD sessionID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SESSIONDLG_H__1A55D424_30F9_11D1_8310_00C04FBEFCDA__INCLUDED_)
