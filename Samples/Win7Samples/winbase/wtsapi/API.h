// API.h : main header file for the API application
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#if !defined(AFX_API_H__C090CEEA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_API_H__C090CEEA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

//typedef VOID (CALLBACK* WTSCLOSESERVER)(HANDLE);
//typedef VOID WINAPI (CALLBACK* WTSCLOSESERVER)(IN HANDLE hServer);
//typedef (CALLBACK* WTSOPENSERVER)(HANDLE);
//typedef (CALLBACK* WTSENUMERATESESSIONS)(HANDLE);
//typedef (CALLBACK* WTSENUMERATEPROCESSES)(HANDLE);
//extern WTSCLOSESERVER* pWTSCloseServer;


/////////////////////////////////////////////////////////////////////////////
// CAPIApp:
// See API.cpp for the implementation of this class
//

class CAPIApp : public CWinApp
{
public:
	CAPIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAPIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAPIApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_API_H__C090CEEA_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
