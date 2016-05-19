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
// logindlg.cpp : implementation file
//


#include "stdafx.h"
#include "mqfdraw.h"
#include "logindlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MAX_VAR 20

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog box


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_strLogin = _T("");
	m_fDsEnabledLocaly = IsDsEnabledLocaly();

	//}}AFX_DATA_INIT
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Text(pDX, IDC_EDIT_LOGIN, m_strLogin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
		// NOTE: The ClassWizard will add message map macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


BOOL CLoginDlg::IsDsEnabledLocaly()
/*++

Routine Description:
    
      The routine checks whether the local computer is operating in domain (DS-enabled) mode
      or in workgroup (DS-disabled) mode.

Arguments:
    
      None

Return Value:
    
      TRUE     -  domain (DS-enabled) mode.
      FALSE    -  workgroup (DS-disabled) mode.

--*/

{
       

    MQPRIVATEPROPS PrivateProps;
    QMPROPID       aPropId[MAX_VAR];
    MQPROPVARIANT  aPropVar[MAX_VAR];
    DWORD          cProp;  
    HRESULT        hr;
    //
    // Specify the PROPID_PC_DS_ENABLED property, which indicates whether
	// the computer is enabled to access the directory service.
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
    // This code is used to detect a DS connection.
    // This code is designed to allow compilation both on 
    // NT 4.0 and on Windows 2000 adn later.
    //
    HINSTANCE hMqrtLibrary = GetModuleHandle(TEXT("mqrt.dll"));
	if(hMqrtLibrary == NULL)
	{
		AfxMessageBox("An incomplete installation of MSMQ was detected. Exiting...");
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
        // It will be OK to handle this case as a case of domain (DS-enabled) mode.
        //
        return TRUE;
    }

	hr = pfMQGetPrivateComputerInformation(
				     NULL,
					 &PrivateProps);
	if(FAILED(hr))
	{
        //
        // We were not able to determine whether the local computer is enabled to access the DS.
        // Notify the user and assume the worst case, i.e., that access to the DS is disasbled.
        //
        AfxMessageBox("No connection to the DS was detected.");        
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
// CLoginDlg message handlers
