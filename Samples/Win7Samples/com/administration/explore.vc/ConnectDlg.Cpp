// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "ConnectDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




#define CONNECTION_TYPE_UNDEFINED	-1
#define CONNECTION_TYPE_LOCAL		0
#define CONNECTION_TYPE_REMOTE		1


/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog


CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Load the dialogs icon resource
	m_hIcon = AfxGetApp()->LoadIcon(IDI_CONNECT);

	// Initialize members
	m_pCatalog = NULL;
	m_pParentDlg = NULL;
	m_nConnectionType = CONNECTION_TYPE_UNDEFINED;
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectDlg)
	DDX_Control(pDX, IDC_EDIT_REMOTE_COMPUTER_NAME, m_edtRemoteComputerName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectDlg)
	ON_BN_CLICKED(IDC_RADIO_REMOTE_COMPUTER, OnRemoteComputer)
	ON_BN_CLICKED(IDC_RADIO_THIS_COMPUTER, OnLocalComputer)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg message handlers

void CConnectDlg::set_Catalog(ICOMAdminCatalog* pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();
	
	// Assign the new Catalog reference
	m_pCatalog = pCatalog;
}

void CConnectDlg::OnOK() 
{
	if (NULL != m_pCatalog)
	{
		ICatalogCollection* pRootCollection = NULL;
		HRESULT				hr = 0;
		_bstr_t				bstrInputValue = "";

		// Get a reference to the "Root" collection depending upon the
		//	specified connection type
		switch (m_nConnectionType)
		{
		case CONNECTION_TYPE_LOCAL:
				bstrInputValue = "Root";
				hr = m_pCatalog->GetCollection(bstrInputValue,
											(IDispatch**) &pRootCollection);
				bstrInputValue = "( Local )";
				break;
		case CONNECTION_TYPE_REMOTE:
				int nMaxLength = m_edtRemoteComputerName.GetWindowTextLength() + 1;
				char* pBuf = new char[nMaxLength];
				m_edtRemoteComputerName.GetWindowText(pBuf, nMaxLength);
				bstrInputValue = pBuf;
				delete [] pBuf;
				hr = m_pCatalog->Connect(bstrInputValue, 
										(IDispatch**) &pRootCollection);
				break;
		}
		
		// Contine only if we successfully get a reference to the specified "Root" collection
		if SUCCEEDED(hr)
		{
			// Attempt assignment only if correctly setup
			if (NULL != m_pParentDlg)
			{
				// Increment the reference count on the object before we pass it
				//	back to the parent dialog
				pRootCollection->AddRef();
				// Set the new remote connection
				m_pParentDlg->set_CurrentCollection(pRootCollection);
				// Set the computer name
				m_pParentDlg->set_ComputerName(bstrInputValue);
				// Release the temporary object
				pRootCollection->Release();
			}
			else
				MessageBox("Invalid parent reference.\n\nYou must call CConnectDlg->set_ParentDlg() before attempting to execute ICatalog::Connect().\n\nPress OK to continue.",
							"Error",
							(MB_OK | MB_ICONERROR));
		}
		else
			MessageBox("Failed to connect to specified root collection.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
	}
	else
		MessageBox("Invalid Catalog reference.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Release the reference to our Catalog object
	ReleaseCatalog();

	CDialog::OnOK();
}

void CConnectDlg::OnRemoteComputer() 
{
	// Enable the edit control
	ToggleEditControl(true);
}

void CConnectDlg::OnLocalComputer() 
{
	// Clear and disable the edit control
	ToggleEditControl(false);
}

BOOL CConnectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the dialogs icon resource
	SetIcon(m_hIcon, true);		// 32x32 icon
	SetIcon(m_hIcon, false);	// 16x16 icon

	// Select and set default connection type
	CheckRadioButton(IDC_RADIO_THIS_COMPUTER, IDC_RADIO_REMOTE_COMPUTER, IDC_RADIO_THIS_COMPUTER);

	// Set the import type and description to match the current selection
	OnLocalComputer();

	// Set the edit control to it's default state
	ToggleEditControl(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConnectDlg::ToggleEditControl(bool Flag)
{
	// Clear the control if we are disabling it
	if (Flag)
		m_nConnectionType = CONNECTION_TYPE_REMOTE;
	else
	{
		m_nConnectionType = CONNECTION_TYPE_LOCAL;
		m_edtRemoteComputerName.SetSel(0, -1, false);
		m_edtRemoteComputerName.Clear();
	}

	// Toggle the control state
	m_edtRemoteComputerName.EnableWindow(Flag);
}

void CConnectDlg::set_ParentDlg(CVCExploreDlg *pDlg)
{
	m_pParentDlg = pDlg;
}

void CConnectDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

void CConnectDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}
