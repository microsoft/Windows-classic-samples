//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfile.cpp
//
// Abstract:            The implementation for the MFC dialog application
//
//*****************************************************************************

#include "stdafx.h"
#include "GenProfile.h"
#include "GenProfileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGenProfileApp, CWinApp)
    //{{AFX_MSG_MAP(CGenProfileApp)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


//------------------------------------------------------------------------------
// Name: CGenProfileApp::CGenProfileApp()
// Desc: Constructor.
//------------------------------------------------------------------------------
CGenProfileApp::CGenProfileApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGenProfileApp object

CGenProfileApp theApp;

//------------------------------------------------------------------------------
// Name: CGenProfileApp::InitInstance()
// Desc: Performs standard initialization.
//       If you are not using these features and wish to reduce the size
//       of your final executable, you should remove from the following
//       the initialization routines you do not need.
//------------------------------------------------------------------------------
BOOL CGenProfileApp::InitInstance()
{

    CoInitialize( NULL ); // Needed in order for codecs to load

    CGenProfileDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.

    CoUninitialize();

    return FALSE;
}
