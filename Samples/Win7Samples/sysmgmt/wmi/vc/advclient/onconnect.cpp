// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnConnect.cpp
//
// Description:
//	This file implements the OnConnect() routine which 
//		demonstrates how to connect to CIMOM and one if
//		its namespaces.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"

// **************************************************************************
//
//	CAdvClientDlg::OnConnect()
//
// Description:
//		Connects to the namespace specified in the edit box.
//
// Parameters:
//		None.
//
// Returns:
//		Nothing.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
void CAdvClientDlg::OnConnect() 
{
	IWbemLocator *pIWbemLocator = NULL;

	UpdateData();
	//------------------------
   // Create an instance of the WbemLocator interface.
   if(CoCreateInstance(CLSID_WbemLocator,
					  NULL,
					  CLSCTX_INPROC_SERVER,
					  IID_IWbemLocator,
					  (LPVOID *) &pIWbemLocator) == S_OK)
   {
		//------------------------
		// Use the pointer returned in step two to connect to
		//     the server using the passed in namespace.
		BSTR pNamespace = m_namespace.AllocSysString();
		if (!pNamespace)
		{
			m_outputList.AddString(_T("Out of memory"));
			pIWbemLocator->Release();
			return;
		}
		HRESULT hr = S_OK;

		if((hr = pIWbemLocator->ConnectServer(pNamespace,
								NULL,   //using current account for simplicity
								NULL,	//using current password for simplicity
								0L,		// locale
								0L,		// securityFlags
								NULL,	// authority (domain for NTLM)
								NULL,	// context
								&m_pIWbemServices)) == S_OK) 
		{	
			// indicate success.
			m_outputList.ResetContent();
			m_outputList.AddString(_T("Connected To Namespace"));

			// its safe the hit the other buttons now.
			m_enumServices.EnableWindow(TRUE);
			m_enumServicesAsync.EnableWindow(TRUE);
			m_enumDisks.EnableWindow(TRUE);
			m_diskDetails.EnableWindow(TRUE);
			m_connect.EnableWindow(TRUE);
			m_addEquipment.EnableWindow(TRUE);
			m_diskDescriptions.EnableWindow(TRUE);

			// dont allow reconnection.
			m_connect.EnableWindow(FALSE);

			// since these guys register for the non-standard
			// root/default/office namespace, dont enable them unless 
			// that namespace exists.
			if(CheckOfficeNamespace())
			{
				m_perm.EnableWindow(TRUE);
				m_temp.EnableWindow(TRUE);
			}

			// initialize the Register Perm button.
			if(PermRegistered())
			{
				// init for UNregistering.
				m_regPerm = FALSE;
				m_perm.SetWindowText(_T("Unregister &Perm"));
			}
			else // its not registered...
			{
				// init for registering.
				m_regPerm = TRUE;
				m_perm.SetWindowText(_T("Register &Perm"));
			}
		}
		else
		{	
			// failed ConnectServer()
			AfxMessageBox(_T("Bad namespace"));
		}

		// done with pNamespace.
		if(pNamespace) 
		{
			SysFreeString(pNamespace);
		}

		//------------------------
		// done with pIWbemLocator. 
		if (pIWbemLocator)
		{ 
			pIWbemLocator->Release(); 
			pIWbemLocator = NULL;
		}
	}
	else // failed CoCreateInstance()
	{	
		AfxMessageBox(_T("Failed to create IWbemLocator object"));

	} // endif CoCreateInstance()
	
}
