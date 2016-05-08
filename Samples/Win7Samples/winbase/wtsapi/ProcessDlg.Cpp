// ProcessDlg.cpp : implementation file
//
//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "ProcessDlg.h"
//#include <wtsapi32.h>
#include "APIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog


CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcessDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessDlg)
	DDX_Control(pDX, IDC_SERVER_NAME, m_serverName);
	DDX_Control(pDX, IDC_PROCESS, m_processList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	//{{AFX_MSG_MAP(CProcessDlg)
	ON_BN_CLICKED(ID_TERMINATE_PROCESS, OnTerminateProcess)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg message handlers



void CProcessDlg::OnTerminateProcess() 
{
	DWORD		     ProcessId;
	LPSTR serverName = g_pDoc->GetServerName();
	HANDLE serverHandle = WTSOpenServer(serverName);
	CString	tempName;
	int    temp;
	PWTS_PROCESS_INFO pProcess = pProcessInfo;

	temp = m_processList.GetCurSel();
	m_processList.GetText(temp, tempName);
	
	for (DWORD i=0; i < count; i++)
	{
		if (tempName == (pProcess->pProcessName))
		{
			ProcessId = pProcess->ProcessId;
			if (!WTSTerminateProcess(serverHandle,ProcessId,0x0))
				AfxMessageBox("Unable to terminate process");
			i = count;
		}
		pProcess++;
	}
	Refresh();
}

BOOL CProcessDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	serverName = g_pDoc->GetServerName();
	serverHandle = WTSOpenServer(serverName);
	if (!WTSEnumerateProcesses(serverHandle,0,1,&pProcessInfo,&count))
		AfxMessageBox("Unable to enumerate processes");	
	PWTS_PROCESS_INFO pProcess = pProcessInfo;

	CString serverDisplay;
	serverDisplay = "List of processes on ";
	if (*serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	m_serverName.SetWindowText(serverDisplay);	


	for (DWORD i=0; i < count; i++)
	{
		m_processList.AddString(pProcess->pProcessName);
		pProcess++;
	}
	m_processList.SetCurSel(0);
	
	return TRUE;  
}

void CProcessDlg::Refresh()
{
	m_processList.ResetContent();
	BOOL result = WTSEnumerateProcesses(serverHandle,0,1,&pProcessInfo,&count);	
	PWTS_PROCESS_INFO pProcess = pProcessInfo;

	for (DWORD i=0; i < count; i++)
	{
		m_processList.AddString(pProcess->pProcessName);
		pProcess++;
	}
	m_processList.SetCurSel(0);
}

void CProcessDlg::OnCancel() 
{
	WTSCloseServer(serverHandle);
	WTSFreeMemory(pProcessInfo);
	CDialog::OnCancel();
}

void CProcessDlg::OnRefresh() 
{
	Refresh();	
}
