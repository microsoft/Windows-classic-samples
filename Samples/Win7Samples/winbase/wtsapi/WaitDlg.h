#if !defined(AFX_WAITDLG_H__5D915AC2_3390_11D1_8312_00C04FBEFCDA__INCLUDED_)
#define AFX_WAITDLG_H__5D915AC2_3390_11D1_8312_00C04FBEFCDA__INCLUDED_

//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WaitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg dialog

class CWaitDlg : public CDialog
{
// Construction
public:
	CWaitDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaitDlg)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWaitDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITDLG_H__5D915AC2_3390_11D1_8312_00C04FBEFCDA__INCLUDED_)
