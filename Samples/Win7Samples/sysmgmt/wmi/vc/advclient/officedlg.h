// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OfficeDlg.h
//
// Description:
//	This file declares the COfficeDlg dialog class which 
//		collects information for the OnAddEquipment() routine.
// 
// Part of: WMI Tutorial #1.
//
// Used by: OnAddEquipment().
//
// History:
//
// **************************************************************************

#if !defined(AFX_OFFICEDLG_H__DE6FFC13_FF91_11D0_AD84_00AA00B8E05A__INCLUDED_)
#define AFX_OFFICEDLG_H__DE6FFC13_FF91_11D0_AD84_00AA00B8E05A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OfficeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COfficeDlg dialog

class COfficeDlg : public CDialog
{
// Construction
public:
	COfficeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COfficeDlg)
	enum { IDD = IDD_OFFICEEQUIP };
	CString	m_item;
	CString	m_SKU;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COfficeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COfficeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OFFICEDLG_H__DE6FFC13_FF91_11D0_AD84_00AA00B8E05A__INCLUDED_)
