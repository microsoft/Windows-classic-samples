// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  PermEvents.h 
//
// Description:
//    main header file for the PermEvents application
//
// History:
//
// **************************************************************************

#if !defined(AFX_PermEvents_H__E8685698_0774_11D1_AD85_00AA00B8E05A__INCLUDED_)
#define AFX_PermEvents_H__E8685698_0774_11D1_AD85_00AA00B8E05A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "factory.h"

/////////////////////////////////////////////////////////////////////////////
// CPermEventsApp:
// See PermEvents.cpp for the implementation of this class
//

class CPermEventsApp : public CWinApp
{
public:
	CPermEventsApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPermEventsApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	int ExitInstance();

// Implementation

	//{{AFX_MSG(CPermEventsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	DWORD m_clsReg;
	CProviderFactory *m_factory;

	void RegisterServer(void);
	void UnregisterServer(void);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PermEvents_H__E8685698_0774_11D1_AD85_00AA00B8E05A__INCLUDED_)
