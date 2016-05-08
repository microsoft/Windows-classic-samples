// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_INSERTTBL_H__EB9EA713_DB7E_11D1_AD69_0060083E86DF__INCLUDED_)
#define AFX_INSERTTBL_H__EB9EA713_DB7E_11D1_AD69_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsertTbl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsertTableDlg dialog

class CInsertTableDlg : public CDialog
{
// Construction
public:
	CInsertTableDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInsertTableDlg)
	enum { IDD = IDD_INSERT_TABLE };
	CString	m_Caption;
	CString	m_CellAttribs;
	UINT	m_NumCols;
	UINT	m_NumRows;
	CString	m_TableAttribs;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsertTableDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsertTableDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSERTTBL_H__EB9EA713_DB7E_11D1_AD69_0060083E86DF__INCLUDED_)
