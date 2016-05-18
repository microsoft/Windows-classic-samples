// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "UtilitiesDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define UTILITY_TYPE_UNDEFINED		-1
#define UTILITY_TYPE_ROUTER			0
#define UTILITY_TYPE_JNEC			1

#define UTILITY_OPTION_UNDEFINED	-1
#define UTILITY_OPTION_START		0
#define UTILITY_OPTION_STOP			1
#define UTILITY_OPTION_REFRESH		2


/////////////////////////////////////////////////////////////////////////////
// CUtilitiesDlg dialog


CUtilitiesDlg::CUtilitiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUtilitiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUtilitiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Set the dialogs icon
	m_hIcon = AfxGetApp()->LoadIcon(IDI_UTILITIES);

	// Initialize the members
	m_pCatalog = NULL;
	m_nUtilityType = UTILITY_TYPE_UNDEFINED;
	m_nUtilityOption = UTILITY_OPTION_UNDEFINED;
}


void CUtilitiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUtilitiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUtilitiesDlg, CDialog)
	//{{AFX_MSG_MAP(CUtilitiesDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_JNEC, OnJNEC)
	ON_BN_CLICKED(IDC_RADIO_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_RADIO_ROUTER, OnRouter)
	ON_BN_CLICKED(IDC_RADIO_START, OnStart)
	ON_BN_CLICKED(IDC_RADIO_STOP, OnStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUtilitiesDlg message handlers

void CUtilitiesDlg::set_Catalog(ICOMAdminCatalog *pCatalog)
{
	// Only can have one Catalog object reference... release all prior references before assignment
	ReleaseCatalog();

	// Assign the new Catalog reference
	m_pCatalog = pCatalog;
}

void CUtilitiesDlg::OnClose() 
{
	// Release the reference to the Catalog object if needed
	ReleaseCatalog();
	
	CDialog::OnClose();
}

BOOL CUtilitiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Setup the dialogs icon resource
	SetIcon(m_hIcon, true);		// 32x32 Icon
	SetIcon(m_hIcon, false);	// 16x16 Icon

	// Check the default radio buttons, set default UI state, and set appropriate member variables
	//	used to manage UI state
	CheckRadioButton(IDC_RADIO_ROUTER, IDC_RADIO_JNEC, IDC_RADIO_ROUTER);

	// Set the import type and description to match the current selection
	OnRouter();

	// Enable the "Refresh" option
	CWnd* pWnd = GetDlgItem(IDC_RADIO_REFRESH);
	if (NULL != pWnd)
		pWnd->EnableWindow(true);

	CheckRadioButton(IDC_RADIO_START, IDC_RADIO_REFRESH, IDC_RADIO_START);
	m_nUtilityOption = UTILITY_OPTION_START;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUtilitiesDlg::OnJNEC() 
{

}

void CUtilitiesDlg::OnRefresh() 
{
	m_nUtilityOption = UTILITY_OPTION_REFRESH;
}

void CUtilitiesDlg::OnRouter() 
{
	// Set the utility type
	m_nUtilityType = UTILITY_TYPE_ROUTER;
	
	// Enable the "Refresh" option
	CWnd* pWnd = GetDlgItem(IDC_RADIO_REFRESH);
	if (NULL != pWnd)
		pWnd->EnableWindow(true);

	// Set default option
	CheckRadioButton(IDC_RADIO_START, IDC_RADIO_REFRESH, IDC_RADIO_START);
}

void CUtilitiesDlg::OnStart() 
{
	m_nUtilityOption = UTILITY_OPTION_START;	
}

void CUtilitiesDlg::OnStop() 
{
	m_nUtilityOption = UTILITY_OPTION_STOP;
}

void CUtilitiesDlg::OnOK() 
{
	// Continue only if we have a valid Catalog pointer
	if (NULL != m_pCatalog)
	{
		// Perform actions depending upon specified utility type (i.e., router )
		switch (m_nUtilityType)
		{
		case UTILITY_TYPE_ROUTER:
			switch (m_nUtilityOption)
			{
			case UTILITY_OPTION_START:
				m_pCatalog->StartRouter();
				break;
			case UTILITY_OPTION_STOP:
				m_pCatalog->StopRouter();
				break;
			case UTILITY_OPTION_REFRESH:
				m_pCatalog->RefreshRouter();
				break;
			default:
				MessageBox("Invalid utility option.\n\nNo processing will be performed.\n\nPress OK to continue.",
							"Error",
							(MB_OK | MB_ICONERROR));
				break;
			}
			break;
		case UTILITY_TYPE_JNEC:
			break;

		default:
			MessageBox("Invalid utility type.\n\nNo processing will be performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
		}
	}
	else
		MessageBox("Invalid Catalog reference.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Release the reference to the Catalog object
	ReleaseCatalog();

	CDialog::OnOK();
}

void CUtilitiesDlg::ReleaseCatalog()
{
	// Release the reference to the Catalog object if needed
	if (NULL != m_pCatalog)
		m_pCatalog->Release();
}
