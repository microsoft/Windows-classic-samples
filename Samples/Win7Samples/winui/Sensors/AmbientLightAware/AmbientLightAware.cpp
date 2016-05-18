//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAware.cpp
//
// Description:
//        Dialog for Ambient Light Aware SDK Sample
//
// Comments: 
//        Standard vc++ dialog created by VS 2005 wizard.
//        This code just creates a dialog and cleans it up when finished.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "AmbientLightAware.h"
#include "AmbientLightAwareSensorEvents.h"
#include "AmbientLightAwareSensorManagerEvents.h"
#include "AmbientLightAwareDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAmbientLightAwareApp

BEGIN_MESSAGE_MAP(CAmbientLightAwareApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAmbientLightAwareApp construction
// Wizard generated constructor
CAmbientLightAwareApp::CAmbientLightAwareApp()
{
    // Place all significant initialization in InitInstance

    // Protect against heap corruption
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
}


// The one and only CAmbientLightAwareApp object
CAmbientLightAwareApp theApp;


// CAmbientLightAwareApp initialization
BOOL CAmbientLightAwareApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // ************************************************************************
    // Added COM initialization, dialog cleanup, and COM uninitialization.
    // ************************************************************************
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Initializes COM
    CAmbientLightAwareDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    dlg.CleanUp(); // Once the dialog has returned, clean up its private members
    ::CoUninitialize(); // Uninitializes COM

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
