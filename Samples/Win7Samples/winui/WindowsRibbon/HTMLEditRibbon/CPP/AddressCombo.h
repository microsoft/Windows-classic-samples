// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_ADDRESSCOMBO_H__45AEB6CD_F0C3_11D1_AD85_0060083E86DF__INCLUDED_)
#define AFX_ADDRESSCOMBO_H__45AEB6CD_F0C3_11D1_AD85_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddressCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddressCombo window

class CAddressCombo : public CComboBox
{
// Construction
public:
	CAddressCombo();
	void FillWithExplorerHistory();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddressCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	CFont m_font;
	virtual ~CAddressCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAddressCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDRESSCOMBO_H__45AEB6CD_F0C3_11D1_AD85_0060083E86DF__INCLUDED_)
