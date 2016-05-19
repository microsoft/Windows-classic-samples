// APIDoc.cpp : implementation of the CAPIDoc class
//
//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation

#include "stdafx.h"
#include "API.h"
#include "string.h"
#include <wtsapi32.h>
#include "APIDoc.h"
#include "ServerDlg.h"
#include "ProcessDlg.h"
#include "SessionDlg.h"
#include "MessageDlg.h"
#include "WaitSystemDlg.h"
#include "ShutdownDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAPIDoc

IMPLEMENT_DYNCREATE(CAPIDoc, CDocument)

BEGIN_MESSAGE_MAP(CAPIDoc, CDocument)
	//{{AFX_MSG_MAP(CAPIDoc)
	ON_COMMAND(ID_RUN_TEST, OnRunTest)
	ON_COMMAND(ID_PROCESS, OnProcess)
	ON_COMMAND(ID_SESSION, OnSession)
	ON_COMMAND(ID_SHUTDOWN_SYSTEM, OnShutdownSystem)
	ON_COMMAND(ID_SEND_MESSAGE, OnSendMessage)
	ON_COMMAND(ID_WAIT_EVENT, OnWaitEvent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAPIDoc construction/destruction

CAPIDoc *g_pDoc;

CAPIDoc::CAPIDoc()
{
	g_pDoc = this;
	serverName = new TCHAR[SERVERNAME_MAXSTRLEN];
	serverName[0] = NULL;
}

CAPIDoc::~CAPIDoc()
{
}



/////////////////////////////////////////////////////////////////////////////
// CAPIDoc serialization

void CAPIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAPIDoc diagnostics

#ifdef _DEBUG
void CAPIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAPIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAPIDoc commands

void CAPIDoc::OnRunTest() 
{
	CServerDlg dlg;
	dlg.DoModal();
}

void CAPIDoc::OnProcess() 
{
	CProcessDlg dlg;
	dlg.DoModal();
}

void CAPIDoc::OnSession() 
{
	CSessionDlg dlg;
	dlg.DoModal();	
}

void CAPIDoc::OnShutdownSystem() 
{
	CShutdownDlg dlg;
	dlg.DoModal();
}

void CAPIDoc::OnSendMessage() 
{
	CMessageDlg dlg;
	dlg.DoModal();	
}

void CAPIDoc::OnWaitEvent() 
{
	CWaitSystemDlg dlg;
	dlg.DoModal();
}

