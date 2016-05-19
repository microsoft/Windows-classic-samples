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
// RecvMDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "RecvMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReceiveMessageDialog dialog


CReceiveMessageDialog::CReceiveMessageDialog(CArray <ARRAYQ*, ARRAYQ*>* pStrArray, CWnd* pParent /*=NULL*/)
   : CDialog(CReceiveMessageDialog::IDD, pParent)
{
   m_pStrArray = pStrArray;

   //{{AFX_DATA_INIT(CReceiveMessageDialog)
   m_szPathName = _T("");
   m_iTimeout = 20000;
	m_dwBodySize = BUFFERSIZE;
	//}}AFX_DATA_INIT
}


void CReceiveMessageDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CReceiveMessageDialog)
   DDX_Control(pDX, IDC_COMBO, m_PathNameCB);
   DDX_CBString(pDX, IDC_COMBO, m_szPathName);
	DDV_MaxChars(pDX, m_szPathName, 4000);
   DDX_Text(pDX, IDC_TIMEOUT, m_iTimeout);
	DDX_Text(pDX, IDC_EDT_BODY_SIZE, m_dwBodySize);
	DDV_MinMaxDWord(pDX, m_dwBodySize, 1, 100000000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReceiveMessageDialog, CDialog)
   //{{AFX_MSG_MAP(CReceiveMessageDialog)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReceiveMessageDialog message handlers

BOOL CReceiveMessageDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   // TODO: Add extra initialization here

   int i;

   for  (i=0 ; i<m_pStrArray->GetSize() ; i++)
   {
      if (((*m_pStrArray)[i]->dwAccess & MQ_RECEIVE_ACCESS) != FALSE)
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
   }

    if (m_PathNameCB.GetCount() > 0) m_PathNameCB.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}

