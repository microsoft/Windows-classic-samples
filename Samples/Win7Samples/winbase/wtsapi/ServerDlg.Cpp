// ServerDlg.cpp : implementation file
//
//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "ServerDlg.h"
//#include "wtsapi32.h"
#include "string.h"
#include "APIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerDlg)
	//}}AFX_DATA_INIT
}


void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
	DDX_Control(pDX, IDC_SERVER_LIST2, m_serverList2);
	DDX_Control(pDX, IDC_DOMAIN_NAME, m_domainName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	//{{AFX_MSG_MAP(CServerDlg)
	ON_CBN_DROPDOWN(IDC_SERVER_LIST2, OnDropdownServerList2)
	ON_EN_CHANGE(IDC_DOMAIN_NAME, OnChangeDomainName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_domainName.SetWindowText("TEST");
	domainFlag = FALSE;
	return TRUE;  
}

void CServerDlg::OnOK() 
{
	//int nIndex = m_serverList2.GetCurSel();
	//if (nIndex != LB_ERR)
	//	m_serverList2.GetLBText(nIndex, serverName);
  
	WTSFreeMemory(&pServerInfo);
	m_serverList2.GetWindowText( serverName);
	//AfxMessageBox(serverName);
	g_pDoc->SetServerName( (TCHAR *) LPCTSTR(serverName));

	CDialog::OnOK();
}

void CServerDlg::OnDropdownServerList2() 
{
	CString dname;
	m_domainName.GetWindowText(dname);

	if (dname == "")
		AfxMessageBox("Please enter a domain name.");
	else
    {
	if (domainFlag == FALSE) {
	  m_serverList2.ResetContent();
	  if (!WTSEnumerateServers((TCHAR *)LPCTSTR (dname), 
			0, 1, &pServerInfo, &count))
		AfxMessageBox("Unable to enumerate servers");
	  PWTS_SERVER_INFO pServer = pServerInfo;
	  for (DWORD i=0; i < count; i++)
	  {
		  m_serverList2.AddString(pServer->pServerName);
		  pServer++;
	  }
	  domainFlag = TRUE;
	}
	  /*m_serverList2.AddString("purple");
	  m_serverList2.AddString("blue");
	  m_serverList2.AddString("red");
	  m_serverList2.AddString("orange");
	  m_serverList2.AddString("yellow");*/
	}
}

void CServerDlg::OnChangeDomainName() 
{
	domainFlag = FALSE;	
}
