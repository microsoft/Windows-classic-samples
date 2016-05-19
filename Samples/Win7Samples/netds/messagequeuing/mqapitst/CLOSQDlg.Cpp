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
// ClosQDlg.cpp : implementation file
//




#include "stdafx.h"
#include "MQApitst.h"
#include "ClosQDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCloseQueueDialog dialog


CCloseQueueDialog::CCloseQueueDialog(CArray <ARRAYQ*, ARRAYQ*>*  pStrArray, CWnd* pParent /*=NULL*/)
	: CDialog(CCloseQueueDialog::IDD, pParent)
{
	m_pStrArray = pStrArray;

	//{{AFX_DATA_INIT(CCloseQueueDialog)
	m_szPathName = _T("");
	//}}AFX_DATA_INIT
}


void CCloseQueueDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCloseQueueDialog)
	DDX_Control(pDX, IDC_COMBO, m_PathNameCB);
	DDX_CBString(pDX, IDC_COMBO, m_szPathName);
	DDV_MaxChars(pDX, m_szPathName, 4000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCloseQueueDialog, CDialog)
	//{{AFX_MSG_MAP(CCloseQueueDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCloseQueueDialog message handlers

BOOL CCloseQueueDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int i;
	
	// TODO: Add extra initialization here
	for  (i=0 ; i<m_pStrArray->GetSize() ; i++)
	{
		if ((*m_pStrArray)[i]->bFormatNameOnly)
		{
			VERIFY (m_PathNameCB.AddString((*m_pStrArray)[i]->szFormatName) != CB_ERR);
		}
		else
		{
			VERIFY (m_PathNameCB.AddString((*m_pStrArray)[i]->szPathName) != CB_ERR);
		}
	}
	
    if (m_PathNameCB.GetCount() > 0) m_PathNameCB.SetCurSel(0);        

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

