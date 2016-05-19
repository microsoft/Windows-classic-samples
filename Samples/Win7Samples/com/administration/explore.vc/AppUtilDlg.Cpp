// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "AppUtilDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAppUtilDlg dialog


CAppUtilDlg::CAppUtilDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppUtilDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppUtilDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Initialize the members
	m_pCatalog = NULL;
	m_nUtilityType = UTILITY_TYPE_UNDEFINED;
}


void CAppUtilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppUtilDlg)
	DDX_Control(pDX, IDC_EDIT_APPLICATION_ID_VALUE, m_edtApp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAppUtilDlg, CDialog)
	//{{AFX_MSG_MAP(CAppUtilDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_APPLICATON_ID_SEARCH, OnAppSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppUtilDlg message handlers

void CAppUtilDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	m_pCatalog = pCatalog;
}

void CAppUtilDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CAppUtilDlg::set_UtilityType(int UtilityType)
{
	// Continue only for valid requests
	if ((UtilityType == UTILITY_TYPE_START_APP) || (UtilityType == UTILITY_TYPE_STOP_APP))
		m_nUtilityType = UtilityType;
}

BOOL CAppUtilDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialogs caption and load the appropriate icon resource
	switch (m_nUtilityType)
	{
		case UTILITY_TYPE_START_APP:
			SetWindowText("Start Application");
			m_hIcon = AfxGetApp()->LoadIcon(IDI_START);
			break;
		case UTILITY_TYPE_STOP_APP:
			SetWindowText("Stop Application");
			m_hIcon = AfxGetApp()->LoadIcon(IDI_STOP);
			break;
	}

	// Set the dialog's icon resource
	SetIcon(m_hIcon, true);		// 32x32 icon
	SetIcon(m_hIcon, false);	// 16x16 icon

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAppUtilDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}

void CAppUtilDlg::OnOK() 
{
	// Continue only if we have a valid Catalog pointer
	if (NULL != m_pCatalog)
	{
		HRESULT hr = 0;

		// Retrieve the text from the edit control
		int nTextLength = m_edtApp.GetWindowTextLength() + 1;
		char* pBuf = new char[nTextLength];
		m_edtApp.GetWindowText(pBuf, nTextLength);
		_bstr_t	bstrApp = pBuf;
		delete [] pBuf;

		// Execute specified action
		switch (m_nUtilityType)
		{
		case UTILITY_TYPE_START_APP:
			hr = m_pCatalog->StartApplication(bstrApp);
			break;
		case UTILITY_TYPE_STOP_APP:
			hr = m_pCatalog->ShutdownApplication(bstrApp);
			break;
		default:
			MessageBox("Invalid utility type specified.\n\nNo processing will be performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
			break;
		}

		// Validate success/fail of the method call
		if FAILED(hr)
			MessageBox("Failed to perform the requested action.\n\nPress OK to continue.",
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

void CAppUtilDlg::OnAppSearch() 
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
	if (UTILITY_TYPE_START_APP == m_nUtilityType)
		ofn.lpstrTitle = "Select Application to start";
	else
		ofn.lpstrTitle = "Select Application to stop";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtApp.SetWindowText(ofn.lpstrFile);
}
