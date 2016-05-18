// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// COMRTS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "COMRTS.h"
#include "COMRTSDlg.h"
#include <initguid.h>
#include "RTSCOM_i.c"
#include "COMRTS_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CCOMRTSApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

// CCOMRTSApp construction

CCOMRTSApp::CCOMRTSApp()
{

}

// The one and only CCOMRTSApp object

CCOMRTSApp theApp;

// CCOMRTSApp initialization

BOOL CCOMRTSApp::InitInstance()
{
    CCOMRTSDlg dlg;
    m_pMainWnd = &dlg;

    INT_PTR nResponse = dlg.DoModal();

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}

BOOL CCOMRTSApp::ExitInstance(void)
{
#if !defined(_WIN32_WCE) || defined(_CE_DCOM)

#endif
    return CWinApp::ExitInstance();
}
