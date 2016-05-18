// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "CompImportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#define IMPORT_TYPE_UNDEFINED	-1
#define IMPORT_TYPE_PROGID		0
#define IMPORT_TYPE_CLSID		1


/////////////////////////////////////////////////////////////////////////////
// CCompImportDlg dialog


CCompImportDlg::CCompImportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompImportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Load the dialogs icon resource
	m_hIcon = AfxGetApp()->LoadIcon(IDI_IMPORT);

	// Initialize the members
	m_pCatalog = NULL;
	m_nImportType = IMPORT_TYPE_UNDEFINED;
}


void CCompImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompImportDlg)
	DDX_Control(pDX, IDC_EDIT_COMPONENT, m_edtID);
	DDX_Control(pDX, IDC_LABEL_COMPONENT, m_lblComponentDesc);
	DDX_Control(pDX, IDC_COMBO_APPLICATION, m_cboApplications);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompImportDlg, CDialog)
	//{{AFX_MSG_MAP(CCompImportDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_CLSID, OnCLSID)
	ON_BN_CLICKED(IDC_RADIO_PROG_ID, OnProgID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompImportDlg message handlers

void CCompImportDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	// Assign the new Catalog reference
	m_pCatalog = pCatalog;
}

void CCompImportDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CCompImportDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}

void CCompImportDlg::OnOK() 
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

		int nIDSize = m_edtID.GetWindowTextLength() + 1;
		char* pComponentNameBuf = new char[nIDSize];
		m_edtID.GetWindowText(pComponentNameBuf, nIDSize);
		_bstr_t	bstrID = pComponentNameBuf;
		delete [] pComponentNameBuf;

		// Attempt to invoke the method
		HRESULT hr = m_pCatalog->ImportComponent(bstrAppName, 
													bstrID);
		// Report errors, if any
		if FAILED(hr)
			MessageBox("Failed to export the specified component.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
	}
	else
		MessageBox("Invalid Catalog reference.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Release the catalog reference
	ReleaseCatalog();

	CDialog::OnOK();
}

BOOL CCompImportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialogs icon resources
	SetIcon(m_hIcon, true);			// 32x32 icon
	SetIcon(m_hIcon, false);		// 16x16 icon

	// Select and set default export type
	CheckRadioButton(IDC_RADIO_PROG_ID, IDC_RADIO_CLSID, IDC_RADIO_PROG_ID);

	// Set the import type and description to match the current/default selection
	OnProgID();

	// Load the available applications
	LoadAppCombo();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCompImportDlg::OnCLSID() 
{
	// Set the import type....
	m_nImportType = IMPORT_TYPE_CLSID;
	//... and set the description to match the selection
	m_lblComponentDesc.SetWindowText("CLSID:");
}

void CCompImportDlg::OnProgID() 
{
	// Set the import type....
	m_nImportType = IMPORT_TYPE_PROGID;
	//... and set the description to match the selection
	m_lblComponentDesc.SetWindowText("ProgID:");
}

void CCompImportDlg::LoadAppCombo()
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
