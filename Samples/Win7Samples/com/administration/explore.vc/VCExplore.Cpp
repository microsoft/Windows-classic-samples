// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "VCExploreDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern g_pCurrCatalog;
//extern g_pCurrCollection;

/////////////////////////////////////////////////////////////////////////////
// CVCExploreApp

BEGIN_MESSAGE_MAP(CVCExploreApp, CWinApp)
	//{{AFX_MSG_MAP(CVCExploreApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVCExploreApp construction

CVCExploreApp::CVCExploreApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVCExploreApp object

CVCExploreApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVCExploreApp initialization

BOOL CVCExploreApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

/*
    In MFC 5.0, Enable3dControls and Enable3dControlsStatic are obsolete because 
    their functionality is incorporated into Microsoft's 32-bit operating systems.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
*/
	// Attempt to get a reference to current catalog
	//_bstr_t bstrCollectionName = "Root";
	//g_pCurrCatalog = new ICatalog;
	//g_pCurrCollection = g_pCurrCatalog->GetCollection(bstrCollectionName);

	CVCExploreDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
