// QueryDlg.cpp : implementation file
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"
#include "APIDoc.h"
#include "QueryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


Query QueryItems[MAXQUERYITEMS] =
{
#ifdef ICA_STUFF_INCLUDED
	{TEXT("Version"), WTSVersion},
#endif
	{TEXT("Initial Program"), WTSInitialProgram},
	{TEXT("Working Directory"), WTSWorkingDirectory},		 
	{TEXT("OEM ID"), WTSOEMId},
	{TEXT("Session ID"), WTSSessionId},
	{TEXT("UserName"), WTSUserName},
	{TEXT("WinStation Name"), WTSWinStationName},
	{TEXT("Domain Name"), WTSDomainName},
	{TEXT("Connnection State"), WTSConnectState},
	{TEXT("Client Build Number"), WTSClientBuildNumber},
	{TEXT("Client Name"), WTSClientName},
	{TEXT("Client Directory"), WTSClientDirectory},
	{TEXT("Client Product ID"), WTSClientProductId},
	{TEXT("Client Address"), WTSClientAddress},
	{TEXT("Client Display"), WTSClientDisplay},
#ifdef ICA_STUFF_INCLUDED
	{TEXT("Client Cache"), WTSClientCache},
	{TEXT("Client Drives"), WTSClientDrives},
	{TEXT("ICA Buffer Length"), WTSICABufferLength},
	{TEXT("License Enabler"), WTSLicenseEnabler},
#endif
	{TEXT("Application Name"), WTSApplicationName}
};


/////////////////////////////////////////////////////////////////////////////
// CQueryDlg dialog

CQueryDlg::CQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQueryDlg::IDD, pParent)
{
}


void CQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryDlg)
	DDX_Control(pDX, IDC_SESSION_INFO, m_displayInfo);
	DDX_Control(pDX, IDC_SERVERNAME, m_serverName);
	DDX_Control(pDX, IDC_QUERY_SESSION, m_querySession);
	DDX_Text(pDX, IDC_SESSION_INFO, m_sessionInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryDlg)
	ON_BN_CLICKED(IDQUERY, OnQuery)
	ON_LBN_DBLCLK(IDC_QUERY_SESSION, OnDblclkQuerySession)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg message handlers

BOOL CQueryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	LPSTR serverName = g_pDoc->GetServerName();

	CString serverDisplay;
	serverDisplay = "Query types for ";
	//if (*serverName == NULL)
	if (serverName == NULL)
		serverDisplay += "local server";
	else
		serverDisplay += serverName;
	m_serverName.SetWindowText(serverDisplay);

	for (int i=0; i < MAXQUERYITEMS; i++)
		m_querySession.InsertString(i, QueryItems[i].QueryName);
	m_querySession.SetCurSel(0);
	return TRUE;
}

void CQueryDlg::OnQuery() 
{	
	LPSTR serverName = g_pDoc->GetServerName();
	HANDLE serverHandle = WTSOpenServer(serverName);
	DWORD sessionID = g_pDoc->GetSessionID();
	BytesReturned = 0;
	int nIndex = m_querySession.GetCurSel();
	if (!WTSQuerySessionInformation(serverHandle, sessionID,
		QueryItems[nIndex].InfoClass, &pSessionInfo, &BytesReturned))
	   AfxMessageBox("Query failed!");
	else
	{

    switch ( QueryItems[nIndex].InfoClass ) {
#ifdef ICA_STUFF_INCLUDED
        case WTSVersion:
		{
			LPOSVERSIONINFO pVerInfo = (LPOSVERSIONINFO) pSessionInfo;
			m_sessionInfo.Format("Version: major %u \r\n minor %u \r\n build %u \r\n CSD Version: %s",
                      pVerInfo->dwMajorVersion, pVerInfo->dwMinorVersion,
                      pVerInfo->dwBuildNumber, pVerInfo->szCSDVersion);
		}
			break;
#endif
        case WTSInitialProgram:   
			m_sessionInfo.Format("Initial Program:\r\n ");
			//m_sessionInfo = "Initial Program: ";
			m_sessionInfo += pSessionInfo;
			break;

        case WTSWorkingDirectory: 
			m_sessionInfo.Format("Working Directory:\r\n ");
			m_sessionInfo += pSessionInfo;
           	break;

        case WTSOEMId:
            m_sessionInfo = "OEM ID: ";
			m_sessionInfo += pSessionInfo;
            break;

        case WTSSessionId:        
			m_sessionInfo.Format(TEXT("Session ID: %u"), *(DWORD *) pSessionInfo);
            break;

        case WTSUserName:
            m_sessionInfo = "UserName: ";
			m_sessionInfo += pSessionInfo;
            break;

        case WTSWinStationName:
            m_sessionInfo = "WinStation Name: ";
			m_sessionInfo += pSessionInfo;
			break;

        case WTSDomainName:
            m_sessionInfo = "Domain Name: ";
			m_sessionInfo += pSessionInfo;
            break;

        case WTSConnectState:
			{CString connectState[10];
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
			m_sessionInfo.Format("Connection State: %s", connectState[(*(DWORD *) pSessionInfo)]);
			}
			break;

        case WTSClientBuildNumber :
			m_sessionInfo.Format("Client Build Number: %d",(*(WORD*) pSessionInfo));
            break;

        case WTSClientName :
            m_sessionInfo = "Client Name: ";
			m_sessionInfo += pSessionInfo;
            break;

        case WTSClientDirectory :
			m_sessionInfo.Format("Client Directory:\r\n %s", pSessionInfo);
            break;

        case WTSClientProductId :
			m_sessionInfo.Format("Client Product ID: %u",(*(WORD*) pSessionInfo));
			break;

		case WTSClientAddress:
		{
			BYTE * pByte;
			PWTS_CLIENT_ADDRESS pClientAddress = (PWTS_CLIENT_ADDRESS) pSessionInfo;
            m_sessionInfo.Format( TEXT("ClientAddress: (%u) "),
                      pClientAddress->AddressFamily );
            
			CString tempString;
			switch ( pClientAddress->AddressFamily ) {
                case 0 :
                    tempString.Format( TEXT("%s"), pClientAddress->Address );
                    m_sessionInfo += tempString;
					break;
                case 2 :
                    pByte = &pClientAddress->Address[2];
                    tempString.Format( TEXT("%u.%u.%u.%u"),
                              pByte[0], pByte[1], pByte[2], pByte[3] );
					m_sessionInfo += tempString;
                    break;
                case 6 :
                    pByte = pClientAddress->Address;
                    for ( int i=0; i<4; i++ ) {
                        tempString.Format( TEXT("%02x"), pByte[i] );
						m_sessionInfo += tempString;
					}
					m_sessionInfo += ":";
                    for ( int i=4; i<10; i++ ){
                        tempString.Format( TEXT("%02x"), pByte[i] );
						m_sessionInfo += tempString;
					}
                    break;
            }
	
		}
			break;

		case WTSClientDisplay:
		{	PWTS_CLIENT_DISPLAY SessionInfo = (PWTS_CLIENT_DISPLAY) pSessionInfo;
			m_sessionInfo.Format("Client Display:\r\n  Resolution: %d X %d\r\n  Color Depth %d",
				SessionInfo->HorizontalResolution,
				SessionInfo->VerticalResolution,
				SessionInfo->ColorDepth);
		}
			break;

#ifdef ICA_STUFF_INCLUDED
		case WTSClientCache:
		
		{	PWTS_CLIENT_CACHE SessionInfo = (PWTS_CLIENT_CACHE) pSessionInfo;
			m_sessionInfo.Format("Client Cache:\r\n  Tiny Cache %d\r\n  Low Memory Cache %d\r\n  XMS Cache %d\r\n  Disk Cache %d\r\n  Cache Size Dim %d\r\n  Bitmap Min Dim %d\r\n  Signature Level Dim %d\r\n  File System Overhead %d",
				SessionInfo->CacheTiny,
				SessionInfo->CacheLowMem,
				SessionInfo->CacheDisk,
				SessionInfo->DimCacheSize,
				SessionInfo->DimBitmapMin,
				SessionInfo->DimSignatureLevel,
				SessionInfo->DimFilesysOverhead);
		}
			break;

		case WTSClientDrives:
		{
			PWTS_CLIENT_DRIVES pClientDrives = (PWTS_CLIENT_DRIVES) pSessionInfo;
            CString tempString;
			CString driveType[10];
			driveType[0] = "Removeable";
			driveType[1] = "Fixed";
			driveType[2] = "Remote";
			driveType[3] = "CD-Rom";
			driveType[4] = "Ram Disk";
			driveType[5] = "AutoConnect";
			driveType[6] = "Unknown";

			m_sessionInfo.Format( TEXT("WTSClientDrives: fAuto %u, count %u \r\n"),
                      pClientDrives->fAutoClientDrives,
                      pClientDrives->Count );
            
			for ( DWORD i=0; i < pClientDrives->Count; i++ ) {
                switch (pClientDrives->Drives[i].Flags)
				{	  
				  case  1: nIndex = 0; break;
				  case  2: nIndex = 1; break;
				  case  4: nIndex = 2; break;
				  case  8: nIndex = 3; break;
				  case 10: nIndex = 4; break;
				  case 20: nIndex = 5; break;
				  default: nIndex = 6;
				}			  
				tempString.Format( TEXT(" %c:%s \r\n"),							
					pClientDrives->Drives[i].DriveLetter,
					driveType[nIndex]);						
				m_sessionInfo += tempString;
			}
		}		
			break;

		case WTSICABufferLength:
			m_sessionInfo.Format("ICA Buffer Length: %u", (*(DWORD *) pSessionInfo));
			break;

		case WTSLicenseEnabler:
			m_sessionInfo.Format("License Enabler: %x", (*(DWORD *) pSessionInfo));
			break;
#endif
		
		case WTSApplicationName:
            m_sessionInfo.Format("ApplicationName: %s", pSessionInfo );
            break;
		}

		SetDlgItemText(IDC_SESSION_INFO, m_sessionInfo);	
		WTSFreeMemory( pSessionInfo );
	}
	
}

void CQueryDlg::OnCancel() 
{
    WTSFreeMemory(pSessionInfo);
	CDialog::OnCancel();
}


void CQueryDlg::OnDblclkQuerySession() 
{
	OnQuery();	
}
