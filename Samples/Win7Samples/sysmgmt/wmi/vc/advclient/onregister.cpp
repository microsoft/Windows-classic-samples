// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnAddUser.cpp
//
// Description:
//	This file implements the OnAddUser() routine which 
//		demonstrates how to add WMI user accounts, user
//		groups and NTLM authenticator instances.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"
#include <objbase.h>

// **************************************************************************
//
//	CAdvClientDlg::OnRegPerm()
//
// Description:
//		Handles the GUI aspects of the Register Perm button
//		and calls the right worker routine.
//
// Parameters:
//		none.
//
// Returns:
//		nothing
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================

void CAdvClientDlg::OnRegPerm() 
{
	// if registered.
	if(m_regPerm)
	{
		if(OnPermRegister())
		{
			m_regPerm = FALSE;
			m_perm.SetWindowText(_T("Unregister &Perm"));
		}
	}
	else
	{
		m_regPerm = TRUE;
		OnPermUnregister();
		m_perm.SetWindowText(_T("Register &Perm"));
	}
	
}

// **************************************************************************
//
//	CAdvClientDlg::OnRegTemp()
//
// Description:
//		Handles the GUI aspects of the Register Temp button
//		and calls the right worker routine.
//
// Parameters:
//		none.
//
// Returns:
//		nothing
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
void CAdvClientDlg::OnRegTemp() 
{
	if(m_regTemp)
	{
		if(OnTempRegister())
		{
			m_regTemp = FALSE;
			m_temp.SetWindowText(_T("Unregister &Temp"));
		}
	}
	else
	{
		m_regTemp = TRUE;
		OnTempUnregister();
		m_temp.SetWindowText(_T("Register &Temp"));
	}
	
}
