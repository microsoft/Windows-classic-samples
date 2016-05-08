// SessionDlg.cpp : implementation file
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "SessionDlg.h"
#include "QueryDlg.h"
//#include "SetSessionDlg.h"
#include "APIDoc.h"
//#include "wtsapi32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char *VCName[] = {
		"CTXTW  ",
		"CTXLPT1",
		"CTXCPM ",
		"CTXCDM ",
		"CTXCLIP",
		NULL,
	};

/////////////////////////////////////////////////////////////////////////////
// CSessionDlg dialog


CSessionDlg::CSessionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSessionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSessionDlg)
	//}}AFX_DATA_INIT
}

void CSessionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSessionDlg)
	DDX_Control(pDX, IDC_LIST_SESSION, m_sessionList);
	DDX_Control(pDX, IDC_SERVER_NAME, m_serverName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSessionDlg, CDialog)
	//{{AFX_MSG_MAP(CSessionDlg)
	ON_BN_CLICKED(IDC_DISCONNECT_SESSION, OnDisconnectSession)
	ON_BN_CLICKED(IDC_LOGOFF_SESSION, OnLogoffSession)
	ON_BN_CLICKED(IDC_QUERY_SESSION, OnQuerySession)
	ON_BN_CLICKED(IDC_VIRTUAL_CHANNEL, OnVirtualChannel)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSessionDlg message handlers

void CSessionDlg::OnDisconnectSession() 
{
	GetSessionID();
	if (!WTSDisconnectSession(serverHandle,sessionID,0x0))
		AfxMessageBox("Unable to disconnect session");
	else
		Refresh();				
}

void CSessionDlg::OnLogoffSession() 
{
	GetSessionID();
	if (!WTSLogoffSession(serverHandle,sessionID,0x0))
		AfxMessageBox("Unable to logoff session");
	else
		Refresh();	
}

void CSessionDlg::OnQuerySession() 
{	
	SetSessionID();
	CQueryDlg dialog;
	dialog.DoModal();
}

BOOL CSessionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	serverName = g_pDoc->GetServerName();
	serverHandle = WTSOpenServer(serverName);

	CString serverDisplay;
	serverDisplay = "List of sessions on ";
	if (*serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	m_serverName.SetWindowText(serverDisplay);

	
	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 120;
	lvColumn.iSubItem = 0;
	lvColumn.pszText = "WinStation";
	m_sessionList.InsertColumn(0, &lvColumn);
	lvColumn.iSubItem = 1;
	lvColumn.pszText = "Session ID";
	m_sessionList.InsertColumn(1, &lvColumn);
	lvColumn.iSubItem = 2;
	lvColumn.pszText = "Connection Status";
	m_sessionList.InsertColumn(2, &lvColumn);

	connectState[0] = "Active";	
 	connectState[1] = "Connected";
	connectState[2] = "ConnectQuery";
	connectState[3] = "Shadow";
	connectState[4] = "Disconnected";
	connectState[5] = "Idle";
	connectState[6] = "Listen";
	connectState[7] = "Reset";
	connectState[8] = "Down";
	connectState[9] = "Init";
	
	Refresh();
	return TRUE;  
}

void CSessionDlg::Refresh()
{
	if (!m_sessionList.DeleteAllItems())
		AfxMessageBox("Error unable to delete items");
			
	if (!WTSEnumerateSessions(serverHandle,0,1,&pSessionInfo,&count))
		AfxMessageBox("Unable to enumerate sessions");
	PWTS_SESSION_INFO pSession = pSessionInfo;

	CString temp;
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iImage = 0;

	CString tempString;

	for (DWORD i=0; i < count; i++)
	{
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		if (*pSession->pWinStationName == NULL)
			lvItem.pszText = "<No Name>";
		else
			lvItem.pszText = pSession->pWinStationName;
		
		m_sessionList.InsertItem(&lvItem);
		tempString.Format("%d", pSession->SessionId);		
		m_sessionList.SetItemText(i, 1, tempString);
		tempString = connectState[pSession->State];
	    m_sessionList.SetItemText(i, 2, tempString);
		temp = m_sessionList.GetItemText(i,0);
		pSession++;
	}
}

void CSessionDlg::OnVirtualChannel() 
{
	HANDLE VCHandle;
	//int i, j, k;
	int i;
	//PVOID DataBuf;
	ULONG BytesReturned = 0;
	//int Words;
	//PULONG pw;

	serverName = g_pDoc->GetServerName();
	serverHandle = WTSOpenServer(serverName);

	GetSessionID();
	
	AfxMessageBox("Starting Virtual Channel Test");
	CString virtualMessage, data1, data2;
    for (i=0; VCName[i]; i++) 
	{
		virtualMessage.Format("String length of %s is %d\r\n",VCName[i],strlen(VCName[i]));
        AfxMessageBox(virtualMessage);

	  if (VCHandle = WTSVirtualChannelOpen(serverHandle, sessionID, VCName[i])) 
	  {

#ifdef ICA_STUFF_INCLUDED
        if (WTSVirtualChannelQuery(VCHandle,
                                      WTSVirtualClientData,
                                      &DataBuf,
                                      &BytesReturned)) 
		{
		  int Words;
		  PULONG pw;
		  PVOID DataBuf;
		  int j, k;

		  virtualMessage.Format("WTSVirtualChannelQuery(WTSVirtualClientData) Passed Bytes Returned: %d\r\n",
                          BytesReturned);
		  AfxMessageBox(virtualMessage);

          Words = (BytesReturned + sizeof(ULONG) - 1) / sizeof(ULONG);
		  
		  virtualMessage = "";
		  for (j=0,pw=(PULONG)DataBuf;j<Words;) {
               data1.Format("%04x: ",j*4);
               for(k=0;((k < 4) && (j<Words));j++,k++,pw++) {
                    data2.Format("%08x  ",*pw);
					data1 += data2;
               }
			   virtualMessage += data1;
			   virtualMessage += "\r\n";
          }
		  AfxMessageBox(virtualMessage);
          WTSFreeMemory(DataBuf);
        } 
		else {
                virtualMessage.Format("*** WTSVirtualChannelQuery(WTSVirtualClientData) failed - error: %d\r\n",
                       GetLastError());
				AfxMessageBox(virtualMessage);
		}
#endif

        if (WTSVirtualChannelPurgeInput(VCHandle)) {
            virtualMessage.Format("WTSVirtualChannelPurgeInput Passed\r\n");
			AfxMessageBox(virtualMessage);
        } else {
            virtualMessage.Format("*** WTSVirtualChannelPurgeInput failed - error: %d\r\n",
                GetLastError());
			AfxMessageBox(virtualMessage);
        }

            if (WTSVirtualChannelPurgeOutput(VCHandle)) {
                virtualMessage.Format("WTSVirtualChannelPurgeOutput Passed\r\n" );
				AfxMessageBox(virtualMessage);
            } else {
                virtualMessage.Format("*** WTSVirtualChannelPurgeOutput failed - error: %d\r\n",
                       GetLastError() );
				AfxMessageBox(virtualMessage);
            }

            if (WTSVirtualChannelClose(VCHandle)) {
                virtualMessage.Format("Open/Close channel %s Passed\r\n", VCName[i]);
				AfxMessageBox(virtualMessage);
            } else {
                virtualMessage.Format("*** Close channel %s failed - error: %d\r\n",
			             VCName[i], GetLastError());
			    AfxMessageBox(virtualMessage);
            }
        } else {
            virtualMessage.Format("*** Open channel %s failed - error: %d\r\n",
                     VCName[i], GetLastError());
			AfxMessageBox(virtualMessage);
		}
    }
    virtualMessage.Format("Ending Virtual Channel Test\r\n" );
	AfxMessageBox(virtualMessage);
} 

void CSessionDlg::OnRefresh() 
{
	Refresh();		
}

void CSessionDlg::OnCancel() 
{
	WTSFreeMemory(pSessionInfo);
	WTSCloseServer(serverHandle);
	CDialog::OnCancel();
}

void CSessionDlg::SetSessionID()
{
	PWTS_SESSION_INFO pSession = pSessionInfo;
	CString	tempName;
	UINT state;
	for (DWORD i=0; i < count; i++)
	{
		state = m_sessionList.GetItemState(i, LVIS_SELECTED);
		if (state == LVIS_SELECTED)
		{
			sessionID = pSession->SessionId;
			i = count;
		}
		pSession++;
	}
	g_pDoc->SetSessionID(sessionID);
}


void CSessionDlg::GetSessionID()
{
	PWTS_SESSION_INFO pSession = pSessionInfo;
	UINT state;
	for (DWORD i=0; i < count; i++)
	{
		state = m_sessionList.GetItemState(i, LVIS_SELECTED); 
		if (state == LVIS_SELECTED)
			sessionID = pSession->SessionId;
		pSession++;
	}
}
