// ShutdownDlg.cpp : implementation file
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "APIDoc.h"
#include "ShutdownDlg.h"
//#include "wtsapi32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


Shutdown ShutdownItems[5] =
{
	{TEXT("Delete WinStations"), WTS_WSD_LOGOFF},
	{TEXT("Normal Shutdown"), WTS_WSD_SHUTDOWN},
	{TEXT("Reboot after Shutdown"), WTS_WSD_REBOOT},		 
	{TEXT("Power off computer after shutdown"), WTS_WSD_POWEROFF},
	{TEXT("CTRL-SHIFT-ALT-DEL fast reboot"), WTS_WSD_FASTREBOOT}	
};

/////////////////////////////////////////////////////////////////////////////
// CShutdownDlg dialog


CShutdownDlg::CShutdownDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShutdownDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShutdownDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShutdownDlg)
	DDX_Control(pDX, IDC_SERVERNAME, m_serverName);
	DDX_Control(pDX, IDC_SHUTDOWN_PARAMETERS, m_shutdown);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShutdownDlg, CDialog)
	//{{AFX_MSG_MAP(CShutdownDlg)
	ON_BN_CLICKED(IDSHUTDOWN, OnShutdown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShutdownDlg message handlers

BOOL CShutdownDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	serverName = g_pDoc->GetServerName();
	//AfxMessageBox(serverName);
	serverHandle = WTSOpenServer(serverName);

	CString serverDisplay;
	serverDisplay = "Shutdown parameters for ";
	if (*serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	m_serverName.SetWindowText(serverDisplay);

	for (int i=0; i < MAXSHUTDOWNITEMS; i++)
		m_shutdown.InsertString(i, ShutdownItems[i].ShutdownName);
	m_shutdown.SetCurSel(0);
	return TRUE;
}	


void CShutdownDlg::OnShutdown() 
{
	int nIndex = m_shutdown.GetCurSel();
	if (!WTSShutdownSystem(serverHandle, ShutdownItems[nIndex].ShutdownFlag))
		AfxMessageBox("Unable to shutdown system");
	EndDialog(0);
}
