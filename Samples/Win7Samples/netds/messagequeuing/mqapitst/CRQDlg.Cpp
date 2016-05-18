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
// CrQDlg.cpp : implementation file
//



#include "stdafx.h"
#include "MQApitst.h"
#include "CrQDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateQueueDialog dialog box


CCreateQueueDialog::CCreateQueueDialog(CWnd* pParent /*=NULL*/)
   : CDialog(CCreateQueueDialog::IDD, pParent)
{

	TCHAR szTmpBuffer[MAX_COMPUTERNAME_LENGTH + 2] ={0};
	DWORD dwTmpBufferSize = MAX_COMPUTERNAME_LENGTH+1;
	if (!GetComputerName(szTmpBuffer, &dwTmpBufferSize))
	{

		AfxMessageBox(_T("The computer name is too long for the buffer specified.\n"));
		exit(1);

	}
	//
	// The following line is OK, but any further concatenations will need a bigger array to be safe.
	//
	_tcsncat_s(szTmpBuffer, sizeof(szTmpBuffer)/sizeof(szTmpBuffer[0]), TEXT("\\"), 1);


   //{{AFX_DATA_INIT(CCreateQueueDialog)
   m_strLabel = TEXT("MQ API test");
   m_strPathName = szTmpBuffer;
   //}}AFX_DATA_INIT
}


void CCreateQueueDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CCreateQueueDialog)
   DDX_Text(pDX, IDC_QUEUE_LABEL, m_strLabel);
   DDV_MaxChars(pDX, m_strLabel, 128);
   DDX_Text(pDX, IDC_QUEUE_PATHNAME, m_strPathName);
   DDV_MaxChars(pDX, m_strPathName, 128);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateQueueDialog, CDialog)
   //{{AFX_MSG_MAP(CCreateQueueDialog)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateQueueDialog message handlers
