//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_MESSAGEDLG_H__4CE42083_334F_11D1_8311_00C04FBEFCDA__INCLUDED_)
#define AFX_MESSAGEDLG_H__4CE42083_334F_11D1_8311_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MessageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessageDlg dialog

class CMessageDlg : public CDialog
{
// Construction
public:
	CMessageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessageDlg)
	enum { IDD = IDD_SEND_MESSAGE };
	CEdit	m_serverName;
	CEdit	m_sendMessage;
	CString	m_message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMessageDlg)
	afx_msg void OnSendMessage();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEDLG_H__4CE42083_334F_11D1_8311_00C04FBEFCDA__INCLUDED_)
