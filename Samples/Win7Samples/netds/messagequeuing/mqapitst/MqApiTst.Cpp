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
// Mqapitest.cpp : Defines the class behaviors for the application.




#include "stdafx.h"
#include "MQApitst.h"

#include "MainFrm.h"
#include "testDoc.h"
#include "testView.h"
#include "mq.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestApp

BEGIN_MESSAGE_MAP(CTestApp, CWinApp)
	//{{AFX_MSG_MAP(CTestApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestApp construction

CTestApp::CTestApp()
{
	// TODO: Add construction code here.
	// Place all significant initialization in InitInstance.
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTestApp object

CTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTestApp initialization

BOOL CTestApp::InitInstance()
{
    //
    // This application should only be used when the computer is enabled to 
	// access the directory service (DS-enabled). In workgroup (DS-disabled) mode,
	// we should stop execution.
    //
    if (!IsDsEnabledLocaly())
    {
        ::MessageBox(NULL,_T("This application is not designed to run in workgroup (DS-disabled) mode."), NULL,MB_OK);
        exit(0);
    }
    //
	// Standard initialization.
	// If you are not using these features and wish to reduce the size
	// of your final executable, remove the specific initialization routines 
	// that you do not need from the following.

#ifdef _AFXDLL
#else
	Enable3dControlsStatic();	// Call this function when linking to MFC statically.
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU).

	// Register the application's document templates. Document templates
	// serve as the connection between documents, frame windows, and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTestDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CTestView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, and open file.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog box used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Application command for running the dialog box
void CTestApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CTestApp commands


//
// Pointer to hold the main window pointer.
//
CWnd* pMainView;

extern "C" void  PrintToScreen(const TCHAR * Format, ...)
{
   

   TCHAR szText[500];
   va_list l;
   static int len = 0;


        // Format the string.
        va_start(l, Format); 

        if(_vsntprintf_s(szText, sizeof(szText)/sizeof(szText[0]), sizeof(szText)/sizeof(szText[0])-1, Format, l) < 0)
	{
		::MessageBox(NULL,TEXT("The text is too long for the text buffer. Exiting...\n"),NULL,MB_OK);
		exit(1);
	}
			


   	CEdit& rfMainEdit = ((CTestView*)pMainView)->GetEditCtrl();

	int i = rfMainEdit.GetLineCount();

	((CTestView*)pMainView)->GetEditCtrl().SetSel(INT_MAX,INT_MAX);
	((CTestView*)pMainView)->GetEditCtrl().ReplaceSel(szText);
	((CTestView*)pMainView)->GetEditCtrl().ReplaceSel(TEXT("\r\n"));
	len += lstrlen(szText) + 2;
	((CTestView*)pMainView)->GetDocument()->SetModifiedFlag(FALSE);
}

BOOL CTestApp::IsDsEnabledLocaly()
/*++

Routine Description:
    
      The routine checks whether the computer is enabled to access the directory service (DS-enabled).

Arguments:
    
      None

Return Value:
    
      TRUE     -  DS-enabled.
      FALSE    -  DS-disabled.

--*/

{
       
    MQPRIVATEPROPS PrivateProps;
    QMPROPID       aPropId[MAX_VAR];
    MQPROPVARIANT  aPropVar[MAX_VAR];
    DWORD          cProp;  
    HRESULT        hr;
    //
    // Specify the PROPID_PC_DS_ENABLED, which indicates whether 
	// the computer is DS-enabled.
    //
    cProp = 0;

    aPropId[cProp] = PROPID_PC_DS_ENABLED;
    aPropVar[cProp].vt = VT_NULL;
    ++cProp;	
    //
    // Create a PRIVATEPROPS structure.
    //
    PrivateProps.cProp = cProp;
	PrivateProps.aPropID = aPropId;
	PrivateProps.aPropVar = aPropVar;
    PrivateProps.aStatus = NULL;

    //
    // Retrieve the information.
    //


    //
    // This code detects a DS connection.
    // This code is designed to allow the sample to compile on NT 4.0 and on Windows 2000
    // and later. The MQGetPrivateComputerInformation() function can be called directly
    // on Windows 2000 and later.
    //
    HINSTANCE hMqrtLibrary = GetModuleHandle(TEXT("mqrt.dll"));
	if (hMqrtLibrary == NULL)
	{
		AfxMessageBox(_T("An incomplete installation of MSMQ was detected. Exiting..."));
		exit(1);
	}

    typedef HRESULT (APIENTRY *MQGetPrivateComputerInformation_ROUTINE)(LPCWSTR , MQPRIVATEPROPS*);
    MQGetPrivateComputerInformation_ROUTINE pfMQGetPrivateComputerInformation = 
          (MQGetPrivateComputerInformation_ROUTINE)GetProcAddress(hMqrtLibrary,
													 "MQGetPrivateComputerInformation");
    if(pfMQGetPrivateComputerInformation == NULL)
    {
        //
        // There is no entry point in the DLL matching this routine.
        // It must be an old version of mqrt.dll (MSMQ 1.0).
        // It will be OK to handle this case as a case of DS-enabled mode.
        //
        return TRUE;
    }

	hr = pfMQGetPrivateComputerInformation(
				     NULL,
					 &PrivateProps);
	if(FAILED(hr))
	{
        //
        // We were not able to determine whether the computer is enabled to access the DS.
        // Notify the user and assume the worst case, i.e., that the computer is DS-disasbled.
        //
        AfxMessageBox(_T("No DS connection could be detected."));
        return FALSE;
    }                             
	
    
    if(PrivateProps.aPropVar[0].boolVal == 0)
    {
        //
        // DS-disabled.
        //
        return FALSE;
    }

    return TRUE;

}

