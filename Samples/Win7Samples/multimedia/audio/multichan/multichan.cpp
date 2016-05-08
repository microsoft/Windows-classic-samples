// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// multichan.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "multichan.h"

#include "MainFrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp

BEGIN_MESSAGE_MAP(CMultichanApp, CWinApp)
	//{{AFX_MSG_MAP(CMultichanApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp construction

CMultichanApp::CMultichanApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp destruction
CMultichanApp::~CMultichanApp()
{
    if( g_pdlgDest )
        delete g_pdlgDest;

	POSITION	pos		= g_listSources.GetHeadPosition();
	CDlgSrc	*	pdlgSrc = 0;

	for( ; pos; )
	{
		pdlgSrc = g_listSources.GetNext( pos );
		delete pdlgSrc;
		pdlgSrc = 0;
	}

	g_listSources.RemoveAll();

    ASSERT( g_listSources.IsEmpty() );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMultichanApp object

CMultichanApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp initialization

BOOL CMultichanApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

    ASSERT(g_pChildView);
    g_pdlgDest = new CDlgDest(g_pChildView);
    g_pdlgDest->Create();

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMultichanApp message handlers

void CMultichanApp::OnAppAbout() 
{
	CDialog* pdlg = new CDialog(MAKEINTRESOURCE(IDD_ABOUTBOX));

    pdlg->DoModal();
}
