// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_EDITTOOLBAR_H__EB9EA714_DB7E_11D1_AD69_0060083E86DF__INCLUDED_)
#define AFX_EDITTOOLBAR_H__EB9EA714_DB7E_11D1_AD69_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditToolbar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditToolbar window

class CEditToolbar : public CToolBar
{
// Construction
public:
	CEditToolbar();
	CComboBox m_fontNameCombo;
	CComboBox m_fontSizeCombo;
	CFont m_font;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditToolbar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITTOOLBAR_H__EB9EA714_DB7E_11D1_AD69_0060083E86DF__INCLUDED_)
