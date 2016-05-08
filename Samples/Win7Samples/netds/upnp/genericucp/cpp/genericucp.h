// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// GenericUCP.h : main header file for the GenericUCP application
//

#if !defined(AFX_GenericUCP_H__CF142341_F90B_42D6_8B74_2FC0D5FBAB02__INCLUDED_)
#define AFX_GenericUCP_H__CF142341_F90B_42D6_8B74_2FC0D5FBAB02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


#include "resource.h"		// main symbols





/////////////////////////////////////////////////////////////////////////////
// CGenericUCPApp:
// See GenericUCP.cpp for the implementation of this class
//

class CGenericUCPApp : public CWinApp
{
public:
	CGenericUCPApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenericUCPApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGenericUCPApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GenericUCP_H__CF142341_F90B_42D6_8B74_2FC0D5FBAB02__INCLUDED_)
