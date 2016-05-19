// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "AppInstallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define APP_INSTALL_TYPE_UNDEFINED		-1
#define APP_INSTALL_TYPE_WITHOUT_USERS	0
#define APP_INSTALL_TYPE_WITH_USERS		1


/////////////////////////////////////////////////////////////////////////////
// CAppInstallDlg dialog


CAppInstallDlg::CAppInstallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppInstallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppInstallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Load the dialogs icon resource
	m_hIcon = AfxGetApp()->LoadIcon(IDI_INSTALL);

	// Initialize the members
	m_pCatalog = NULL;
	m_nInstallType = APP_INSTALL_TYPE_UNDEFINED;
	m_bOverwriteFiles = false;
	m_bRemoteInstall = false;
}


void CAppInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppInstallDlg)
	DDX_Control(pDX, IDC_EDIT_UID, m_edtUID);
	DDX_Control(pDX, IDC_EDIT_PWD, m_edtPWD);
	DDX_Control(pDX, IDC_EDIT_REMOTE_SERVER_NAME, m_edtRemoteServerName);
	DDX_Control(pDX, IDC_EDIT_INSTALL_DIRECTORY, m_edtInstallDir);
	DDX_Control(pDX, IDC_EDIT_APPLICATION_FILE, m_edtAppFile);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CAppInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CAppInstallDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_APPLICATION_FILE_SEARCH, OnAppFileSearch)
	ON_BN_CLICKED(IDC_BUTTON_INSTALL_DIRECTORY_SEARCH, OnInstallDirSearch)
	ON_BN_CLICKED(IDC_RADIO_INSTALL_WITH_USERS, OnInstallWithUsers)
	ON_BN_CLICKED(IDC_RADIO_INSTALL_WITHOUT_USERS, OnInstallWithoutUsers)
	ON_BN_CLICKED(IDC_CHECK_OVERWRITE_FILES, OnOverwriteFiles)
	ON_BN_CLICKED(IDC_CHECK_REMOTE_SERVER_INSTALL, OnRemoteServerInstall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppInstallDlg message handlers

void CAppInstallDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	// Assign the new Catalog reference
	m_pCatalog = pCatalog;
}

void CAppInstallDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CAppInstallDlg::OnOK() 
{
	// Continue only if we have a valid Catalog pointer
	if (NULL != m_pCatalog)
	{
		long	lOptions = 0;

		// Determine the install options
		if (m_nInstallType == APP_INSTALL_TYPE_WITHOUT_USERS)
			lOptions = lOptions | COMAdminInstallNoUsers;
		else
			lOptions = lOptions | COMAdminInstallUsers;

		if (m_bOverwriteFiles)
			lOptions = lOptions | COMAdminInstallForceOverwriteOfFiles;

		// Extract method parameters from the UI
		int nAppFileLength = m_edtAppFile.GetWindowTextLength() + 1;
		char* pAppFileBuf = new char[nAppFileLength];
		m_edtAppFile.GetWindowText(pAppFileBuf, nAppFileLength);
		_bstr_t	bstrAppFile = pAppFileBuf;
		delete [] pAppFileBuf;

		int nInstallDirLength = m_edtInstallDir.GetWindowTextLength() + 1;
		char* pInstallDirBuf = new char[nInstallDirLength];
		m_edtInstallDir.GetWindowText(pInstallDirBuf, nInstallDirLength);
		_bstr_t	bstrInstallDir = pInstallDirBuf;
		delete [] pInstallDirBuf;

		HRESULT hr = 0;
		// Perform local vs. remote server install
		if (m_bRemoteInstall)
		{
			int nUIDLength = m_edtUID.GetWindowTextLength() + 1;
			char* pUIDBuf = new char[nUIDLength];
			m_edtUID.GetWindowText(pUIDBuf, nUIDLength);
			_bstr_t	bstrUID = pUIDBuf;
			delete [] pUIDBuf;

			int nPWDLength = m_edtPWD.GetWindowTextLength() + 1;
			char* pPWDBuf = new char[nPWDLength];
			m_edtPWD.GetWindowText(pPWDBuf, nPWDLength);
			_bstr_t	bstrPWD	= pPWDBuf;
			delete [] pPWDBuf;

			int nRSNLength = m_edtRemoteServerName.GetWindowTextLength() + 1;
			char* pRSNBuf = new char[nRSNLength];
			m_edtRemoteServerName.GetWindowText(pRSNBuf, nRSNLength);
			_bstr_t	bstrRSN = pRSNBuf;
			delete [] pRSNBuf;

			// Attempt a remote server install
			hr = m_pCatalog->InstallApplication(bstrAppFile,
												bstrInstallDir,
												lOptions,
												bstrUID,
												bstrPWD,
												bstrRSN);
		}
		else
		{
			// Attempt a local install
			hr = m_pCatalog->InstallApplication(bstrAppFile,
												bstrInstallDir,
												lOptions,
												NULL,
												NULL,
												NULL);
		}

		// Validate success/fail of attempted app install
		if FAILED(hr)
			MessageBox("Failed to install the specified application.\n\nPress OK to continue.",
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

void CAppInstallDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}

void CAppInstallDlg::OnAppFileSearch() 
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
	ofn.lpstrTitle = "Select Application to install";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtAppFile.SetWindowText(ofn.lpstrFile);
}

void CAppInstallDlg::OnInstallDirSearch() 
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
	ofn.lpstrTitle = "Select the install directory";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtInstallDir.SetWindowText(ofn.lpstrFile);
}

BOOL CAppInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialog's icon resource
	SetIcon(m_hIcon, true);			// 32x32 icon
	SetIcon(m_hIcon, false);		// 16x16 icon

	// Select and set default export type
	CheckRadioButton(IDC_RADIO_INSTALL_WITHOUT_USERS, IDC_RADIO_INSTALL_WITH_USERS, IDC_RADIO_INSTALL_WITHOUT_USERS);

	// Set the import type and description to match the current selection
	OnInstallWithoutUsers();
	
	// Set the state of the Remote Server Install control group
	ToggleRemoteServerState(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAppInstallDlg::OnInstallWithUsers() 
{
	// Set the install type
	m_nInstallType = APP_INSTALL_TYPE_WITHOUT_USERS;	
}

void CAppInstallDlg::OnInstallWithoutUsers() 
{
	// Set the install type
	m_nInstallType = APP_INSTALL_TYPE_WITH_USERS;	
}

void CAppInstallDlg::OnOverwriteFiles() 
{
	// Toggle the overwrite flag
	m_bOverwriteFiles = !m_bOverwriteFiles;
}

void CAppInstallDlg::ToggleRemoteServerState(bool Flag)
{
	// Clear the controls and toggle it's state
	m_edtRemoteServerName.SetSel(0, -1, false);
	m_edtRemoteServerName.Clear();
	m_edtRemoteServerName.EnableWindow(Flag);

	m_edtUID.SetSel(0, -1, false);
	m_edtUID.Clear();
	m_edtUID.EnableWindow(Flag);

	m_edtPWD.SetSel(0, -1, false);
	m_edtPWD.Clear();
	m_edtPWD.EnableWindow(Flag);
}

void CAppInstallDlg::OnRemoteServerInstall() 
{
	// Toggle the variable used to manage the Remote Server Install group
	m_bRemoteInstall = !m_bRemoteInstall;

	// Toggle the UI to reflect the new state of the control group
	ToggleRemoteServerState(m_bRemoteInstall);
}
