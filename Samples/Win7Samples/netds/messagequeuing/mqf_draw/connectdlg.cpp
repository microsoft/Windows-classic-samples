// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

//
// connectdlg.cpp : implementation file
//


#include "stdafx.h"
#include "mqfdraw.h"
#include "connectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog

CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectDlg)
	m_iRadioDS = 0;	
	//}}AFX_DATA_INIT
}

void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectDlg)
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_RADIO_DS, m_cRadioDS);
	DDX_Control(pDX, IDOK, m_cContinueButton);
	DDX_Radio(pDX, IDC_RADIO_DS, m_iRadioDS);
	DDX_Control(pDX, IDC_RADIO_WORKGROUP, m_cRadioWorkgroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CConnectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
    
	//
	// Display the login name in the window title.
	//
	SetWindowText("Listening to: " + m_strLogin);

   	return TRUE;  // return TRUE  unless you set the focus to a control.
}


















