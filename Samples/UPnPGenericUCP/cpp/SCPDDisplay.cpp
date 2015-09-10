// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DeviceProp.cpp : implementation file
//

#include "stdafx.h"
#include "genericucp.h"
#include "SCPDDisplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DATA_BUFSIZE 2048
/////////////////////////////////////////////////////////////////////////////
// CSCPDDisplay dialog


CSCPDDisplay::CSCPDDisplay(BSTR bstrDocument, CWnd* pParent)
: CDialog(CSCPDDisplay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSCPDDisplay)
	//}}AFX_DATA_INIT
    m_bstrSCPDDocument = SysAllocString(bstrDocument);
}

CSCPDDisplay::~CSCPDDisplay(){

	SysFreeString(m_bstrSCPDDocument);
}

void CSCPDDisplay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSCPDDisplay)
	DDX_Control(pDX, IDC_SCPDOK, m_OkButton);
	DDX_Control(pDX, IDC_SCPDDISPLAY, m_DocumentDisplay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSCPDDisplay, CDialog)
	//{{AFX_MSG_MAP(CSCPDDisplay)
	ON_BN_CLICKED(IDC_SCPDOK, OnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DeviceProp message handlers

void CSCPDDisplay::OnOk() 
{
	// TODO: Add your control notification handler code here
	EndDialog(1);
	return;
}

//+---------------------------------------------------------------------------
//
//  Member:		OnInitDialog
//
//  Purpose:    Initialization of the dialog box. 
//
//  Arguments:
//				None
//
//  Returns:    TRUE
//
//  Notes:
//				
//



BOOL CSCPDDisplay::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_bstrSCPDDocument)
    {
        m_DocumentDisplay.SetWindowText(m_bstrSCPDDocument);
    }
     
    return TRUE;
}
