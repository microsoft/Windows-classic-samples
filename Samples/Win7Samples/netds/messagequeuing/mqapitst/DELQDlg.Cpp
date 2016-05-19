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
// DelQDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "DelQDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeleteQueueDialog dialog


CDeleteQueueDialog::CDeleteQueueDialog(CArray<ARRAYQ*, ARRAYQ*>* pStrArray, CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteQueueDialog::IDD, pParent)
{
	m_pStrArray = pStrArray;

	//{{AFX_DATA_INIT(CDeleteQueueDialog)
	m_szPathName = _T("");
	//}}AFX_DATA_INIT
}


void CDeleteQueueDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteQueueDialog)
	DDX_Control(pDX, IDC_DELETE_QUEUE_COMBO, m_PathNameCB);
	DDX_CBString(pDX, IDC_DELETE_QUEUE_COMBO, m_szPathName);
	DDV_MaxChars(pDX, m_szPathName, 4000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteQueueDialog, CDialog)
	//{{AFX_MSG_MAP(CDeleteQueueDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteQueueDialog message handlers

BOOL CDeleteQueueDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i;

for  (i=0 ; i<m_pStrArray->GetSize() ; i++)
	VERIFY (m_PathNameCB.AddString((*m_pStrArray)[i]->szPathName) != CB_ERR);

    if (m_PathNameCB.GetCount() > 0) m_PathNameCB.SetCurSel(0);        

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

