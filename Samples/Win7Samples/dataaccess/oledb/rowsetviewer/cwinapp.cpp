//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module MAIN.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//												   
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "CWinApp.h"

	
/////////////////////////////////////////////////////////////////////////////
// CWinApp
//
/////////////////////////////////////////////////////////////////////////////
CWinApp::CWinApp()
	: CAppLite(IDR_ROWSETVIEWER)
{
	//Windows
	m_pCMainWindow		= NULL;

	//Data
	m_pCMallocSpy		= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CWinApp
//
/////////////////////////////////////////////////////////////////////////////
CWinApp::~CWinApp()
{
}


/////////////////////////////////////////////////////////////////////////////
// CWinApp
//
/////////////////////////////////////////////////////////////////////////////
CWinApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CWinApp::InitInstance
//
/////////////////////////////////////////////////////////////////////////////
BOOL CWinApp::InitInstance()
{
	//CoInitialize before we do anything...
	//NOTE: We have to use OleInitialize for Drag-n-Drop.  RegisterDragDrop fails
	//with E_OUTOFMEMORY if just CoInitialize is done...
	OleInitialize(NULL);

	// Ensure that common control DLL is loaded
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&iccex);

	//Set the Locale for all C runtime functions...
	setlocale(LC_ALL, ".ACP");

/*
	// Load all saved user options
	LoadUserOptions();

	//C-runtime hooks
	EnableCRTReportHook();
	EnableCRTAllocHook();
*/
	//Create the Main Window and Child Windows...
	m_pCMainWindow = new CMainWindow;
	if(!m_pCMainWindow->Create(NULL, L"RowsetViewer", L"Microsoft OLE DB RowsetViewer", IDR_ROWSETVIEWER, LoadIcon(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDR_ROWSETVIEWER))))
		return FALSE;

	//Register IMallocSpy (if requested)
	if(GetErrorPosting(EP_IMALLOC_SPY))
	{
		m_pCMallocSpy = new CMallocSpy;
		if(m_pCMallocSpy)
			m_pCMallocSpy->Register(); 
	}

	//Restore the window positions and state
	if(m_pCMainWindow->m_wndPlacement.length)
	{
		//This already does a ShowWindow/UpdateWindow
		m_pCMainWindow->SetWindowPlacement();
	}
	else
	{
		//Show the window...
		m_pCMainWindow->ShowWindow(GetAppLite()->m_nCmdShow);
		m_pCMainWindow->UpdateWindow(); 
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CWinApp::ExitInstance
//
/////////////////////////////////////////////////////////////////////////////
int CWinApp::ExitInstance()
{
	//Since we use the CRT hooks, it will be called on delete.
	//So we need to first NULL the global pointer, then delete the actual object...
	CFrameWndLite* pCMainWindow = m_pCMainWindow;
	m_pCMainWindow = NULL;
	SAFE_DELETE(pCMainWindow);

	//Globals
	g_spDataConvert.Release();

	//Unitialize COM
	SetErrorInfo(0, NULL);
	OleUninitialize();

	//Display any Leaks
	if(m_pCMallocSpy)
	{
		if(GetErrorPosting(EP_IMALLOC_SPY))
			m_pCMallocSpy->DumpLeaks();
		m_pCMallocSpy->Unregister();
		SAFE_RELEASE(m_pCMallocSpy);
	}

	return CAppLite::ExitInstance();
}


