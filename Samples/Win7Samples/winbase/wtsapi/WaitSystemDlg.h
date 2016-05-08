//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_WAITSYSTEMDLG_H__5D915AC1_3390_11D1_8312_00C04FBEFCDA__INCLUDED_)
#define AFX_WAITSYSTEMDLG_H__5D915AC1_3390_11D1_8312_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WaitSystemDlg.h : header file
//

#define MAXEVENTITEMS 12

typedef struct _Events {
	TCHAR EventName[40];
	DWORD EventFlag;
} Events;


/////////////////////////////////////////////////////////////////////////////
// CWaitSystemDlg dialog

class CWaitSystemDlg : public CDialog
{
// Construction
public:
	CWaitSystemDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaitSystemDlg)
	enum { IDD = IDD_WAIT_SYSTEM_EVENT };
	CEdit	m_serverName;
	CListBox	m_eventFlags;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitSystemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWaitSystemDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HANDLE serverHandle;
	LPSTR serverName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITSYSTEMDLG_H__5D915AC1_3390_11D1_8312_00C04FBEFCDA__INCLUDED_)
