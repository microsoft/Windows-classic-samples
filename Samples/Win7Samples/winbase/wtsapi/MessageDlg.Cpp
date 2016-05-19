// MessageDlg.cpp : implementation file
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "APIDoc.h"
#include "MessageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMessageDlg dialog


CMessageDlg::CMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageDlg)
	m_message = _T("");
	//}}AFX_DATA_INIT
}


void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageDlg)
	DDX_Control(pDX, IDC_SERVERNAME, m_serverName);
	DDX_Control(pDX, IDC_SEND_MESSAGE, m_sendMessage);
	DDX_Text(pDX, IDC_SEND_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageDlg, CDialog)
	//{{AFX_MSG_MAP(CMessageDlg)
	ON_BN_CLICKED(ID_SEND_MESSAGE, OnSendMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageDlg message handlers

void CMessageDlg::OnSendMessage() 
{
	LPSTR serverName = g_pDoc->GetServerName();
	HANDLE serverHandle =  WTSOpenServer(serverName);
	DWORD Response;
	int messageLength;
	m_sendMessage.GetWindowText(m_message);
	messageLength = m_message.GetLength();	

	//int GetLine( int nIndex, LPTSTR lpszBuffer ) const;
	//int GetLineCount( ) const;

	if (WTSSendMessage(serverHandle, 0, "Test Message", sizeof("Test Message"), 
			(char *) LPCTSTR(m_message), messageLength+1,
			MB_OKCANCEL, 5, &Response, 1))
		AfxMessageBox("Message was sent");
	else 
		AfxMessageBox("Unable to send message");

	//CString temp
	//temp.Format("%d, 
	
 	WTSCloseServer(serverHandle);
}

BOOL CMessageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	LPSTR serverName = g_pDoc->GetServerName();	
	CString serverDisplay;
	serverDisplay = "Enter in message to send to ";
	if (*serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	m_serverName.SetWindowText(serverDisplay);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
