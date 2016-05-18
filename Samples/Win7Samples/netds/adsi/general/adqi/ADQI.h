// ADQI.h : main header file for the ADQI application
//

#if !defined(AFX_ADQI_H__8170976F_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_ADQI_H__8170976F_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CADQIApp:
// See ADQI.cpp for the implementation of this class
//

#define App ((CADQIApp*)AfxGetApp())
class CADQIApp : public CWinApp
{
public:
	CADQIApp(); 
	HRESULT ADsOpenObject( LPWSTR pszPath, REFIID riid, void**pUnk );		                                             
	void SetCredentials( LPCTSTR pszUserName, DWORD dwFlag );
	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CADQIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	protected:
		CComBSTR m_sUserName;
		DWORD    m_dwFlag;

	
// Implementation

	//{{AFX_MSG(CADQIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADQI_H__8170976F_0672_11D2_B218_0000F87A6B50__INCLUDED_)
