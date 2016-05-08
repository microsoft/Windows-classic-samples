// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

//
// mqfdraw.cpp : Defines the class behaviors for the application.
//


#include "stdafx.h"
#include "mqfdraw.h"
#include "drawdlg.h"
#include "logindlg.h"
#include "connectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_USERNAME_LEN 32
#define MAX_VAR 20

/////////////////////////////////////////////////////////////////////////////
// CDisdrawApp

BEGIN_MESSAGE_MAP(CDisdrawApp, CWinApp)
	//{{AFX_MSG_MAP(CDisdrawApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisdrawApp construction

CDisdrawApp::CDisdrawApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDisdrawApp object

CDisdrawApp theApp;

/////////////////////////////////////////////////////////////////////////////


// CDisdrawApp initialization

BOOL CDisdrawApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//
	// Prompt for the user's name
	//
	char mbsUserName[MAX_USERNAME_LEN];
	DWORD dwNumBytes = MAX_USERNAME_LEN;
	GetUserName(mbsUserName, &dwNumBytes);
	CLoginDlg dlgLogin;
	dlgLogin.m_strLogin = mbsUserName;
	if (dlgLogin.DoModal() == IDCANCEL || dlgLogin.m_strLogin == "" )
	{
		return FALSE;
	}

	dlgLogin.m_strLogin.MakeUpper();
	

	CConnectDlg dlgConnect;

	if(dlgLogin.m_fDsEnabledLocaly)
	{
		dlgConnect.m_strLogin = dlgLogin.m_strLogin;
		if (dlgConnect.DoModal() == IDCANCEL )
		{
			return FALSE;
		}
	}
	CDisdrawDlg dlg;
	m_pMainWnd = &dlg;
	dlg.m_strLogin = dlgLogin.m_strLogin;
	dlg.m_iRadioDS = dlgConnect.m_iRadioDS;
	dlg.m_fDsEnabledLocaly = dlgLogin.m_fDsEnabledLocaly;


	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
