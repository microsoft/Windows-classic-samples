// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "AppExportDlg.h"

#include <comdef.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	EXPORT_TYPE_UNDEFINED		-1
#define	EXPORT_TYPE_WITHOUT_USERS	0
#define EXPORT_TYPE_WITH_USERS		1
#define EXPORT_TYPE_WITH_CLIENTS	2


/////////////////////////////////////////////////////////////////////////////
// CAppExportDlg dialog


CAppExportDlg::CAppExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppExportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Load the dialog's icon resource
	m_hIcon = AfxGetApp()->LoadIcon(IDI_EXPORT);

	// Initialize the members
	m_pCatalog = NULL;
	m_nExportType = EXPORT_TYPE_UNDEFINED;
	m_bOverwriteFiles = false;
}


void CAppExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppExportDlg)
	DDX_Control(pDX, IDC_EDIT_EXPORT_PATH, m_edtExportPath);
	DDX_Control(pDX, IDC_EDIT_APPLICATION_ID, m_edtApplicationID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAppExportDlg, CDialog)
	//{{AFX_MSG_MAP(CAppExportDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_DO_NOT_EXPORT_USERS, OnExportWithoutUsers)
	ON_BN_CLICKED(IDC_RADIO_EXPORT_CLIENTS, OnExportClients)
	ON_BN_CLICKED(IDC_RADIO_EXPORT_USERS, OnExportUsers)
	ON_BN_CLICKED(IDC_CHECK_OVERWRITE_FILES, OnOverwriteFiles)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_PATH_SEARCH, OnExportPathSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppExportDlg message handlers

void CAppExportDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	m_pCatalog = pCatalog;
}

void CAppExportDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CAppExportDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}

void CAppExportDlg::OnOK() 
{
	if (NULL != m_pCatalog)
	{
		// Determine the export options
		long lOptions = 0;
		switch (m_nExportType)
		{
		case EXPORT_TYPE_WITHOUT_USERS:
			lOptions = COMAdminExportNoUsers;
			break;
		case EXPORT_TYPE_WITH_USERS:
			lOptions = COMAdminExportUsers;
			break;
		case EXPORT_TYPE_WITH_CLIENTS:
			lOptions = COMAdminExportApplicationProxy;
			break;
		}

		if (m_bOverwriteFiles)
			lOptions = lOptions | COMAdminExportForceOverwriteOfFiles;

		// Extract method parameters from the UI
		int nAppIDLength = m_edtApplicationID.GetWindowTextLength() + 1;
		char* pAppIDBuf = new char[nAppIDLength];
		m_edtApplicationID.GetWindowText(pAppIDBuf, nAppIDLength);
		_bstr_t	bstrAppID = pAppIDBuf;
		delete [] pAppIDBuf;

		int nExportPathLength = m_edtExportPath.GetWindowTextLength() + 1;
		char* pExportPathBuf = new char[nExportPathLength];
		m_edtExportPath.GetWindowText(pExportPathBuf, nExportPathLength);
		_bstr_t	bstrExportPath = pExportPathBuf;
		delete [] pExportPathBuf;

		// Attempt to export the application
		HRESULT hr = m_pCatalog->ExportApplication(bstrAppID,
													bstrExportPath,
													lOptions);

		// Validate success/fail
		if FAILED(hr)
			MessageBox("Failed to export the application.\n\nNo processing performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));

	}
	else
		MessageBox("Invalid Catalog reference.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Release the Catalog reference
	ReleaseCatalog();

	CDialog::OnOK();
}

BOOL CAppExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialog's icon resource
	SetIcon(m_hIcon, true);			// 32x32 icon
	SetIcon(m_hIcon, true);			// 16x16 icon

	// Select and set default export type
	CheckRadioButton(IDC_RADIO_DO_NOT_EXPORT_USERS, IDC_RADIO_EXPORT_CLIENTS, IDC_RADIO_DO_NOT_EXPORT_USERS);

	// Set the import type and description to match the current selection
	OnExportWithoutUsers();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAppExportDlg::OnExportWithoutUsers() 
{
	m_nExportType = EXPORT_TYPE_WITHOUT_USERS;	
}

void CAppExportDlg::OnExportClients() 
{
	m_nExportType = EXPORT_TYPE_WITH_CLIENTS;
}

void CAppExportDlg::OnExportUsers() 
{
	m_nExportType = EXPORT_TYPE_WITH_USERS;	
}

void CAppExportDlg::OnOverwriteFiles() 
{
	m_bOverwriteFiles = !m_bOverwriteFiles;	
}

void CAppExportDlg::OnExportPathSearch() 
{
	OPENFILENAME ofn;
	char pFilePath[256];
	char pFileTitle[256];

	// Setup of OPENFILENAME structure
	ofn.lStructSize = sizeof(OPENFILENAME); 
    ofn.hwndOwner = m_hWnd; 
    ofn.hInstance = NULL; 
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0\0"; 
    ofn.lpstrCustomFilter = (LPSTR) NULL; 
    ofn.nMaxCustFilter = 0; 
    ofn.nFilterIndex = 1;
    pFilePath[0] = '\0'; 
    ofn.lpstrFile = pFilePath;
    ofn.nMaxFile = sizeof(pFilePath); 
    ofn.lpstrFileTitle = pFileTitle;
    ofn.nMaxFileTitle = sizeof(pFileTitle); 
    ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Select the export directory";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtExportPath.SetWindowText(ofn.lpstrFile);
}
