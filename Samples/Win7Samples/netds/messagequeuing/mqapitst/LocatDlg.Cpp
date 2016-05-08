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
// LocatDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "LocatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocateDialog dialog


CLocateDialog::CLocateDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLocateDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLocateDialog)
	m_szLabel = TEXT("MQ API test");
	//}}AFX_DATA_INIT
}


void CLocateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocateDialog)
	DDX_Text(pDX, IDC_LOCATE_LABEL, m_szLabel);
	DDV_MaxChars(pDX, m_szLabel, 128);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocateDialog, CDialog)
	//{{AFX_MSG_MAP(CLocateDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocateDialog message handlers
