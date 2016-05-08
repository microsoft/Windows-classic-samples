//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_QUERYDLG_H__4CE42081_334F_11D1_8311_00C04FBEFCDA__INCLUDED_)
#define AFX_QUERYDLG_H__4CE42081_334F_11D1_8311_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QueryDlg.h : header file
//

#include <wtsapi32.h>

#define MAXQUERYITEMS 20

typedef struct _Query {
	TCHAR QueryName[25];
	WTS_INFO_CLASS InfoClass;
} Query;


/////////////////////////////////////////////////////////////////////////////
// CQueryDlg dialog

class CQueryDlg : public CDialog
{
// Construction
public:
	DWORD BytesReturned;
	LPTSTR pSessionInfo;
	CQueryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQueryDlg)
	enum { IDD = IDD_QUERY_SESSION };
	CEdit	m_displayInfo;
	CEdit	m_serverName;
	CListBox m_querySession;
	CString	m_sessionInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQueryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnQuery();
	virtual void OnCancel();
	afx_msg void OnDblclkQuerySession();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LPSTR serverName;
	HANDLE serverHandle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUERYDLG_H__4CE42081_334F_11D1_8311_00C04FBEFCDA__INCLUDED_)
