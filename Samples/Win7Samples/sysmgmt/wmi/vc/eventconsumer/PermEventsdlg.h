// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  PermEventsDlg.h 
//
// Description:
//    header file for event consumer sample dialog box
//
// History:
//
// **************************************************************************

#if !defined(AFX_PermEventsDLG_H__E868569A_0774_11D1_AD85_00AA00B8E05A__INCLUDED_)
#define AFX_PermEventsDLG_H__E868569A_0774_11D1_AD85_00AA00B8E05A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CPermEventsDlg dialog

class CPermEventsDlg : public CDialog
{
// Construction
public:
	CPermEventsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPermEventsDlg)
	enum { IDD = IDD_PERMEVENTS_DIALOG };
	CListBox	m_outputList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPermEventsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPermEventsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PermEventsDLG_H__E868569A_0774_11D1_AD85_00AA00B8E05A__INCLUDED_)
