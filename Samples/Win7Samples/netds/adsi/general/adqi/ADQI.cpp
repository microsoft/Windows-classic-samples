//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADQI.cxx
//
//  Contents:   ADQI Main 
//
//
//----------------------------------------------------------------------------



// ADQI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ADQI.h"
#include "ADQIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CADQIApp

BEGIN_MESSAGE_MAP(CADQIApp, CWinApp)
	//{{AFX_MSG_MAP(CADQIApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CADQIApp construction

CADQIApp::CADQIApp()
{
	m_dwFlag = 0;
	m_sUserName.Empty();
}


HRESULT CADQIApp::ADsOpenObject( LPWSTR pszPath, REFIID riid, void**pUnk )
{

	if ( m_sUserName.Length() )
	{
		return ::ADsOpenObject( pszPath, m_sUserName, NULL, m_dwFlag, riid, pUnk );
	}
	else
	{
		return ADsGetObject( pszPath, riid, pUnk );
	}
}

void CADQIApp::SetCredentials( LPCTSTR pszUserName, DWORD dwFlag )
{
	m_sUserName = pszUserName;
	m_dwFlag    = dwFlag;

}
/////////////////////////////////////////////////////////////////////////////
// The one and only CADQIApp object

CADQIApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CADQIApp initialization

BOOL CADQIApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CADQIDlg dlg;
	AfxOleInit();
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
