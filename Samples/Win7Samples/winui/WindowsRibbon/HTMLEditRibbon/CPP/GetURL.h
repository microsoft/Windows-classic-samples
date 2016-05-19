// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_GETURL_H__2BCEEFB4_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
#define AFX_GETURL_H__2BCEEFB4_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetURL.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGetURL dialog
#include "addresscombo.h"
class CGetURL : public CDialog
{
// Construction
public:
	CGetURL(CWnd* pParent = NULL);   // standard constructor
	CString m_URL;
// Dialog Data
	//{{AFX_DATA(CGetURL)
	enum { IDD = IDD_URLDIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetURL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnSearch();
	void OnGo();
	void OnCloseup();
	CString m_szCurAddr;
	CComQIPtr<IWebBrowser2> m_spBrowser;
	CAddressCombo m_AddrCombo;
	// Generated message map functions
	//{{AFX_MSG(CGetURL)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	void OnDocumentComplete(LPDISPATCH pDisp, LPVARIANT pURL);
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETURL_H__2BCEEFB4_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
