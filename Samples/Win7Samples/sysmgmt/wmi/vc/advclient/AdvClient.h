// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  HMMSamp.h
//
// Description:
//	This file declares the CHMMSampleApp class. It is the main
//		class for the tutorial
// 
// Part of: WMI Tutorial #1.
//
// Used by: Nobody.
//
// History:
//
// **************************************************************************

#if !defined(AFX_HMMENUM_H__6AC7FBF7_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_)
#define AFX_HMMENUM_H__6AC7FBF7_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHMMSampleApp:
// See WBEMEnum.cpp for the implementation of this class
//

class CHMMSampleApp : public CWinApp
{
public:
	CHMMSampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHMMSampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	BOOL InitSecurity(void);
	virtual int ExitInstance();
// Implementation

	//{{AFX_MSG(CHMMSampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HMMENUM_H__6AC7FBF7_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_)
