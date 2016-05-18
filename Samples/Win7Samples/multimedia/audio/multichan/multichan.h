// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// multichan.h : main header file for the MULTICHAN application
//

#if !defined(AFX_MULTICHAN_H__C382F3EC_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)
#define AFX_MULTICHAN_H__C382F3EC_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp:
// See multichan.cpp for the implementation of this class
//

class CMultichanApp : public CWinApp
{
public:
	CMultichanApp();    //  ctor
    ~CMultichanApp();   //  dtor

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultichanApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CMultichanApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTICHAN_H__C382F3EC_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)
