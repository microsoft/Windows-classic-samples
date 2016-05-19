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
// SendMDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "SendMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//const TCHAR a[]='6';

const TCHAR MULTICAST[] = _T("MULTICAST=");
/////////////////////////////////////////////////////////////////////////////
// CSendMessageDialog dialog


CSendMessageDialog::CSendMessageDialog(CArray<ARRAYQ*, ARRAYQ*>* pStrArray, CWnd* pParent /*=NULL*/)
	: CDialog(CSendMessageDialog::IDD, pParent)
{
	m_pStrArray = pStrArray;

	//{{AFX_DATA_INIT(CSendMessageDialog)
	m_strBody = szLastMessageBody;
	m_strLabel = szLastMessageLabel;
	m_bPriority = (MQ_MAX_PRIORITY - MQ_MIN_PRIORITY)/2;
	m_iAck = MQMSG_ACKNOWLEDGMENT_NONE;
	m_iDelivery = MQMSG_DELIVERY_EXPRESS;
	m_szPathName = _T("");
	m_szAdminPathName = _T("");
	m_Journal = FALSE;
	m_DeadLetter = FALSE;
	m_Authenticated = FALSE;
	m_Encrypted = FALSE;
	m_dwTimeToReachQueue = (DWORD)DEFAULT_M_TIMETOREACHQUEUE;
	m_dwTimeToBeReceived = (DWORD)DEFAULT_M_TIMETOBERECEIVED;
	//}}AFX_DATA_INIT
}


void CSendMessageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMessageDialog)
	DDX_Control(pDX, IDC_ADMIN_COMBO, m_AdminPathNameCB);
	DDX_Control(pDX, IDC_TARGET_COMBO, m_PathNameCB);
	DDX_Text(pDX, IDC_BODY, m_strBody);
	DDV_MaxChars(pDX, m_strBody, 128);
	DDX_Text(pDX, IDC_LABEL, m_strLabel);
	DDV_MaxChars(pDX, m_strLabel, 128);
	DDX_Text(pDX, IDC_MESSAGE_PRIORITY, m_bPriority);
	DDV_MinMaxByte(pDX, m_bPriority, MQ_MIN_PRIORITY, MQ_MAX_PRIORITY);
	DDX_Radio(pDX, IDC_ACK_NONE_RADIO, m_iAck);
	DDX_Radio(pDX, IDC_DELIVERY_EXPRESS_RADIO, m_iDelivery);
	DDX_CBString(pDX, IDC_TARGET_COMBO, m_szPathName);
	DDV_MaxChars(pDX, m_szPathName, 4000);
	DDX_CBString(pDX, IDC_ADMIN_COMBO, m_szAdminPathName);
	DDV_MaxChars(pDX, m_szAdminPathName, 4000);
	DDX_Check(pDX, IDC_JOURNAL, m_Journal);
	DDX_Check(pDX, IDC_DEAD_LETTER, m_DeadLetter);
	DDX_Check(pDX, IDC_AUTHENTICATED, m_Authenticated);
	DDX_Check(pDX, IDC_ENCRYPTED, m_Encrypted);
	DDX_Text(pDX, IDC_TIME_TO_REACH_QUEUE, m_dwTimeToReachQueue);
	DDX_Text(pDX, IDC_TIME_TO_BE_RECEIVED, m_dwTimeToBeReceived);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendMessageDialog, CDialog)
	//{{AFX_MSG_MAP(CSendMessageDialog)
	ON_CBN_SELCHANGE(IDC_TARGET_COMBO, OnSelchangeTargetCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDialog message handlers

BOOL CSendMessageDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	int i;
	
	// TODO: Add extra initialization here
	for  (i=0 ; i<m_pStrArray->GetSize() ; i++)
	{
		if (((*m_pStrArray)[i]->dwAccess & MQ_SEND_ACCESS) != FALSE)
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
		if ((*m_pStrArray)[i]->bFormatNameOnly)
		{
			CString str = (*m_pStrArray)[i]->szFormatName;
			str=str.Left(sizeof(MULTICAST)/sizeof(TCHAR)-1);
			if (str.Left(sizeof(MULTICAST)/sizeof(TCHAR)-1).CompareNoCase(MULTICAST) != 0)
			{
				VERIFY (m_AdminPathNameCB.AddString((*m_pStrArray)[i]->szFormatName) != CB_ERR);
			}
		}
		else
		{
			VERIFY (m_AdminPathNameCB.AddString((*m_pStrArray)[i]->szPathName) != CB_ERR);
		}
	}
	
    //
    // Set the first PathName as default selection.
    //
    if (m_PathNameCB.GetCount() > 0) m_PathNameCB.SetCurSel(0);
    if (m_AdminPathNameCB.GetCount() > 0) m_AdminPathNameCB.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSendMessageDialog::OnOK() 
{

	UpdateData(TRUE);
	if ((m_szAdminPathName == _T("")) && (m_iAck != 0))
	{
		AfxMessageBox(_T("The chosen acknowledgement requires Admin queue. Please enter a path for the admin queue."));
		return;
	}

	CDialog::OnOK();
}

void CSendMessageDialog::OnSelchangeTargetCombo() 
{
	UpdateData(TRUE);
	int a=m_PathNameCB.GetCurSel();
	CString str=(*m_pStrArray)[m_PathNameCB.GetCurSel()]->szFormatName;
	if (str.Left(10).CompareNoCase(_T("MULTICAST=")) == 0)
	{
		m_AdminPathNameCB.SetCurSel(-1);
	}
}
