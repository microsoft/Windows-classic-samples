// HTMLEdit.h : main header file for the HTMLEDIT application
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_HTMLEDIT_H__2BCEEF85_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
#define AFX_HTMLEDIT_H__2BCEEF85_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CHTMLEditApp:
// See HTMLEdit.cpp for the implementation of this class
//

class CHTMLEditApp : public CWinApp
{
public:
	CHTMLEditApp();

    CRecentFileList& GetRecentFileList()
    {
        return *m_pRecentFileList;
    }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHTMLEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CHTMLEditApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CHTMLEditApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HTMLEDIT_H__2BCEEF85_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
