// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  PermEvents.cpp
//
// Description:
//    Defines the class behaviors for the application.
//
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "PermEvents.h"
#include "PermEventsDlg.h"
#include <objbase.h>
#include <strsafe.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPermEventsApp

BEGIN_MESSAGE_MAP(CPermEventsApp, CWinApp)
	//{{AFX_MSG_MAP(CPermEventsApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPermEventsApp construction

CPermEventsApp::CPermEventsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_clsReg = 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPermEventsApp object

CPermEventsApp theApp;

// {1E069401-087E-11d1-AD85-00AA00B8E05A}
static const GUID CLSID_WBEMSampleConsumer = 
{ 0x1e069401, 0x87e, 0x11d1, { 0xad, 0x85, 0x0, 0xaa, 0x0, 0xb8, 0xe0, 0x5a } };

/////////////////////////////////////////////////////////////////////////////
// CPermEventsApp initialization

BOOL CPermEventsApp::InitInstance()
{
	HRESULT hRes;
	BOOL regEmpty = FALSE; // did a self-unregister happen?

	// OLE initialization. This is 'lighter' than OleInitialize()
	//  which also setups DnD, etc.
	if(SUCCEEDED(CoInitialize(NULL))) 
	{

		//  NOTE:
		//  When using asynchronous WMI API's remotely in an environment where the "Local System" account 
		//  has no network identity (such as non-Kerberos domains), the authentication level of 
		//  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to 
		//  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
		//	use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.
	
		hRes = CoInitializeSecurity( NULL, -1, NULL, NULL, 
											RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
											RPC_C_IMP_LEVEL_IMPERSONATE, 
											NULL, 
											EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
											NULL );
		if (FAILED(hRes))
		{
			AfxMessageBox(_T("CoInitializeSecurity Failed"));
			return FALSE;
		}
	}
	else // didnt CoInitialize()
	{
		AfxMessageBox(_T("CoInitialize Failed"));
		return FALSE;
	} // endif OleInitialize()

	// NOTE: To advertise that we can self-register, put 'OLESelfRegister'
	// in the version resource.

	// see if this is a self-Unregister call.
	TCHAR temp[128];
	TCHAR seps[] = _T(" ");
	TCHAR *token = NULL;
	CHAR *t = NULL;

	StringCbCopy(temp, sizeof(temp), (LPCTSTR)m_lpCmdLine);
	token = strtok_s( temp, seps, &t );
	while( token != NULL )
	{
		/* While there are tokens in "string" */
		if(_tcsicmp(token, _T("/UNREGSERVER")) == 0)
		{
			UnregisterServer();
			return FALSE;		// no use doing any more.
		}
		/* Get next token: */
		token = strtok_s( NULL, seps, &t );
	}

	// if we got here, the unregister didn't return out and we should
	// make sure we're registered now.
	RegisterServer();

	// creating the dlg earlier than usual so the class factory 
	//	can pass m_outputList.
	CPermEventsDlg dlg;

	m_factory = new CProviderFactory(&(dlg.m_outputList));
	if (m_factory == NULL)
	{
		TRACE(_T("Out of memory\n"));	
		return FALSE; 
	}

	if((hRes = CoRegisterClassObject(CLSID_WBEMSampleConsumer,
							(IUnknown *)m_factory,
							CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
							REGCLS_MULTIPLEUSE,
							&m_clsReg)) == S_OK)
	{
		TRACE(_T("registered\n"));
	}
	else
	{
		TRACE(_T("not registered\n"));
	}

#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CPermEventsApp::ExitInstance()
{
	if(m_clsReg)
	{
		HRESULT hres = CoRevokeClassObject(m_clsReg);

		CoUninitialize();
	}

	return CWinApp::ExitInstance();
}

#define TCHAR_LEN_IN_BYTES(str)	 (unsigned long)(_tcslen(str)*sizeof(TCHAR)+sizeof(TCHAR))

//***************************************************************************
//
// DllRegisterServer
//
// Purpose: Called during setup or by regsvr32.
//
// Return:  NOERROR if registration successful, error otherwise.
// Note: Key setups are:
//		HKCR\CLSID\[guid]= friendly name
//		HKCR\CLSID\[guid]\LocalServer32 = exe's path.
//		HKCR\CLSID\AppID = [guid]
//		HKCR\AppID\[guid] = friendly name
//		HKCR\AppID\[guid] = 'RunAs' = "Interactive User"
//			'RunAs' is a value name; not a subkey.
//***************************************************************************
void CPermEventsApp::RegisterServer(void)
{   
	HKEY hKey1, hKey2;

	TCHAR       wcConsID[] = _T("{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    TCHAR       wcCLSID[] = _T("SOFTWARE\\Classes\\CLSID\\{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    TCHAR       wcAppID[] = _T("SOFTWARE\\Classes\\AppID\\{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    TCHAR      wcModule[MAX_PATH + 1];	// this will hold the exe's path.
	TCHAR ConsumerTextForm[] = _T("WMI Sample Permanent Consumer Object");

	// this will allow the server to display its windows on the active desktop instead
	// of the hidden desktop where services run.
	TCHAR Interactive[] = _T("Interactive User");

	memset(&wcModule, NULL, sizeof(wcModule));
	GetModuleFileName(NULL, wcModule,  sizeof(wcModule)/sizeof(TCHAR) - 1);


	//Set the "default" text under CLSID
	//==========================
	RegCreateKeyEx( HKEY_LOCAL_MACHINE, wcCLSID, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey1, NULL );
	RegSetValueEx(hKey1, NULL, 0, REG_SZ, 
					(LPBYTE)ConsumerTextForm, 
					TCHAR_LEN_IN_BYTES(ConsumerTextForm));

	// create the LocalServer32 key so the server can be found.
	RegCreateKeyEx( hKey1, _T("LocalServer32"), 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey2, NULL );
	RegSetValueEx(hKey2, NULL, 0, REG_SZ, (LPBYTE)wcModule, TCHAR_LEN_IN_BYTES(wcModule));
	RegSetValueEx(hKey1, _T("AppID"), 0, REG_SZ, (LPBYTE)wcConsID, TCHAR_LEN_IN_BYTES(wcConsID));

	RegCloseKey(hKey2);
	RegCloseKey(hKey1);

	// now do the AppID keys.
	RegCreateKeyEx( HKEY_LOCAL_MACHINE, wcAppID, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey1, NULL );
	RegSetValueEx(hKey1, NULL, 0, REG_SZ, 
					(LPBYTE)ConsumerTextForm, 
					TCHAR_LEN_IN_BYTES(ConsumerTextForm));

	// this makes the local server run on the active desktop (the one you're seeing) 
	// instead of the hidden desktop that services run on (which doesn't have UI)
	RegSetValueEx(hKey1, _T("RunAs"), 0, REG_SZ, (LPBYTE)Interactive, TCHAR_LEN_IN_BYTES(Interactive));
	RegCloseKey(hKey1);
}


//***************************************************************************
//
// DllUnregisterServer
//
// Purpose: Called when it is time to remove the registry entries.
//
// Return:  NOERROR if registration successful, error otherwise.
//***************************************************************************
void CPermEventsApp::UnregisterServer(void)
{
 
	TCHAR       wcConsID[] = _T("{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    TCHAR       wcCLSID[] = _T("SOFTWARE\\Classes\\CLSID\\{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    TCHAR       wcAppID[] = _T("SOFTWARE\\Classes\\AppID\\{1E069401-087E-11d1-AD85-00AA00B8E05A}");
    HKEY hKey1;
	DWORD dwRet;

	// delete the keys under CLSID\[guid]
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wcCLSID, 0, KEY_WRITE, &hKey1);
    if(dwRet == NOERROR)
    {
        RegDeleteKey(hKey1, _T("LocalServer32"));
        RegCloseKey(hKey1);
    }

	// delete CLSID\[guid] <default>
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\CLSID"), 0, KEY_WRITE, &hKey1);
    if(dwRet == NOERROR)
    {
        RegDeleteKey(hKey1,wcConsID);
        RegCloseKey(hKey1);
    }

	// delete AppID\[guid] <default>
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\AppID"), 0, KEY_WRITE, &hKey1);
    if(dwRet == NOERROR)
    {
        RegDeleteKey(hKey1, wcConsID);
        RegCloseKey(hKey1);
    }
}


