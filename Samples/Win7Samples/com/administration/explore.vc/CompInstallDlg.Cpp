// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "CompInstallDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCompInstallDlg dialog


CCompInstallDlg::CCompInstallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompInstallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompInstallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Load the dialog's icon resource
	m_hIcon = AfxGetApp()->LoadIcon(IDI_INSTALL);

	// Initialize the members
	m_pCatalog = NULL;
}


void CCompInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompInstallDlg)
	DDX_Control(pDX, IDC_COMBO_APPLICATION, m_cboApplications);
	DDX_Control(pDX, IDC_EDIT_TYPE_LIBRARY, m_edtTypeLib);
	DDX_Control(pDX, IDC_EDIT_PROXY_STUB_DLL, m_edtProxyStub);
	DDX_Control(pDX, IDC_EDIT_COMPONENT, m_edtComponent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CCompInstallDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_COMPONENT_SEARCH, OnComponentSearch)
	ON_BN_CLICKED(IDC_BUTTON_TYPE_LIBRARY, OnTypeLibrary)
	ON_BN_CLICKED(IDC_BUTTON_PROXY_STUB_DLL, OnProxyStubDll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompInstallDlg message handlers

void CCompInstallDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	// Assign the new Catalog reference
	m_pCatalog = pCatalog;
}

void CCompInstallDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CCompInstallDlg::OnComponentSearch() 
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
    ofn.lpstrTitle = "Select Component To Install";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtComponent.SetWindowText(ofn.lpstrFile);
}

void CCompInstallDlg::OnTypeLibrary() 
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
    ofn.lpstrTitle = "Select TypeLib To Install";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtTypeLib.SetWindowText(ofn.lpstrFile);
}

void CCompInstallDlg::OnProxyStubDll() 
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
    ofn.lpstrTitle = "Select Proxy/Stub To Install";
    ofn.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);
    ofn.lpstrDefExt = "DLL"; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lCustData = 0;

	// Invoke the file open common dialog
	GetOpenFileName(&ofn);

	// Update the value in the control
	m_edtProxyStub.SetWindowText(ofn.lpstrFile);
}

void CCompInstallDlg::LoadAppCombo()
{
	// Continue only if we have a valid Catalog pointer
	if (NULL == m_pCatalog)
	{
		MessageBox("Invalid Catalog reference.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
		return;
	}

	_bstr_t				bstrAppCollection = "Applications";
	ICatalogCollection* pAppCollection = NULL;

	// Attempt to get a reference to the Catalog's "Application" collection
	HRESULT hr = m_pCatalog->GetCollection(bstrAppCollection, 
											(IDispatch**) &pAppCollection);
	if SUCCEEDED(hr)
	{
		// Attempt to populate the collection
		hr = pAppCollection->Populate();
		if SUCCEEDED(hr)
		{
			// Determine the number of items in the current collection
			long lCount = 0;
			hr = pAppCollection->get_Count(&lCount);
			if SUCCEEDED(hr)
			{
				_variant_t		vtItemName;
				ICatalogObject*	pCatalogObject = NULL;
				
				// Enumerate through the objects in the current collection and 
				//	populate the combo box
				for (long lIndex = 0; lIndex < lCount; lIndex++)
				{
					hr = pAppCollection->get_Item(lIndex, 
													(IDispatch**) &pCatalogObject);
					if SUCCEEDED(hr)
					{
						hr = pCatalogObject->get_Name(&vtItemName);
						if SUCCEEDED(hr)
							m_cboApplications.AddString(_bstr_t(vtItemName));

						pCatalogObject->Release();
					}
					else
						MessageBox("Error when attempting to get the current items name.\n\nPress OK to continue.",
									"Error",
									(MB_OK | MB_ICONERROR));
				}
			}
			else
				MessageBox("Failed to successfully determine the number of items in the Applications collection.\n\nNo processing will be performed.\n\nPress OK to continue.",
							"Error",
							(MB_OK | MB_ICONERROR));
		}
		else
			MessageBox("Failed to successfully populate the Applications collection.\n\nNo processing will be performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
	}
	else
		MessageBox("Failed to get a reference to the Applications collection.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
}

BOOL CCompInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialog's icon resource
	SetIcon(m_hIcon, true);			// 32x32 icon
	SetIcon(m_hIcon, false);		// 16x16 icon

	// Load the application combo box
	LoadAppCombo();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCompInstallDlg::OnOK() 
{
	// Continue only if we have a valid Catalog pointer
	if (NULL != m_pCatalog)
	{
		// Extract the method parameters from the UI
		int nAppNameSize = m_cboApplications.GetWindowTextLength() + 1;
		char* pAppNameBuf = new char[nAppNameSize];
		m_cboApplications.GetWindowText(pAppNameBuf, nAppNameSize);
		_bstr_t	bstrAppName = pAppNameBuf;
		delete [] pAppNameBuf;

		int nComponentNameSize = m_edtComponent.GetWindowTextLength() + 1;
		char* pComponentNameBuf = new char[nComponentNameSize];
		m_edtComponent.GetWindowText(pComponentNameBuf, nComponentNameSize);
		_bstr_t	bstrComponentName = pComponentNameBuf;
		delete [] pComponentNameBuf;

		int nTLBNameSize = m_edtTypeLib.GetWindowTextLength() + 1;
		char* pTLBNameBuf = new char[nTLBNameSize];
		m_edtTypeLib.GetWindowText(pTLBNameBuf, nTLBNameSize);
		_bstr_t	bstrTLB = pTLBNameBuf;
		delete [] pTLBNameBuf;

		int nProxyStubNameSize = m_edtProxyStub.GetWindowTextLength() + 1;
		char* pProxyStubNameBuf = new char[nProxyStubNameSize];
		m_edtProxyStub.GetWindowText(pProxyStubNameBuf, nProxyStubNameSize);
		_bstr_t	bstrProxyStub = pProxyStubNameBuf;
		delete [] pProxyStubNameBuf;

		// Invoke the method
		HRESULT hr = m_pCatalog->InstallComponent(bstrAppName, 
													bstrComponentName, 
													bstrTLB, 
													bstrProxyStub);
		// Report errors, if any
		if FAILED(hr)
			MessageBox("Failed to install the specified component.\n\nPress OK to continue.",
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

void CCompInstallDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}
