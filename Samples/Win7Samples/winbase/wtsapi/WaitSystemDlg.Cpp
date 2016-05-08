// WaitSystemDlg.cpp : implementation file
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "APIDoc.h"
#include "WaitSystemDlg.h"
//#include "wtsapi32.h"
//#include "\program files\devstudio\myprojects\api\citrix\winsta20.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


Events EventFlags[12] =
{
	{TEXT("Wait on nothing"), WTS_EVENT_NONE},
	{TEXT("New WinStation Created"), WTS_EVENT_CREATE},
	{TEXT("Existing WinStation Deleted"), WTS_EVENT_DELETE},
	{TEXT("Existing WinStation Renamed"), WTS_EVENT_RENAME},		 
	{TEXT("WinStation is connected to client"), WTS_EVENT_CONNECT},
	{TEXT("WinStation is disconnected from client"), WTS_EVENT_DISCONNECT},
	{TEXT("User logon to existing WinStation"), WTS_EVENT_LOGON},
	{TEXT("User logoff from existing WinStation"), WTS_EVENT_LOGOFF},
	{TEXT("WinStation state change"), WTS_EVENT_STATECHANGE},
	{TEXT("License state change"), WTS_EVENT_LICENSE},
	{TEXT("Wait for all event types"), WTS_EVENT_ALL},
	{TEXT("Unblock all waiters"), WTS_EVENT_FLUSH}
};


/////////////////////////////////////////////////////////////////////////////
// CWaitSystemDlg dialog


CWaitSystemDlg::CWaitSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWaitSystemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitSystemDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWaitSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitSystemDlg)
	DDX_Control(pDX, IDC_SERVERNAME, m_serverName);
	DDX_Control(pDX, IDC_EVENT_FLAGS, m_eventFlags);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaitSystemDlg, CDialog)
	//{{AFX_MSG_MAP(CWaitSystemDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitSystemDlg message handlers

BOOL CWaitSystemDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	serverName = g_pDoc->GetServerName();
	serverHandle = WTSOpenServer(serverName);

	CString serverDisplay;
	serverDisplay = "Choose event for ";
	if (*serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	serverDisplay += " to wait on";

	m_serverName.SetWindowText(serverDisplay);


	for (int i=0; i < MAXEVENTITEMS; i++)
		m_eventFlags.InsertString(i, EventFlags[i].EventName);
	m_eventFlags.SetCurSel(0);
	return TRUE;
}


void CWaitSystemDlg::OnOK() 
{
	DWORD EventFlag;
	//CString message;
	int nIndex = m_eventFlags.GetCurSel();
	//message.Format("The current selection is  %x, %u", EventFlags[nIndex].EventFlag, EventFlags[nIndex].EventFlag);
	//MessageBox((LPCTSTR)message);
	if (!WTSWaitSystemEvent(serverHandle, 
	     EventFlags[nIndex].EventFlag, &EventFlag))
		 AfxMessageBox("Unable to wait on system event");

	CDialog::OnOK();
}
