#if 0  // makefile definitions
DESCRIPTION = CreateUserAccount on Local Machine
MODULENAME = create
FILEVERSION = Msi
ENTRY = CreateUserAccount
UNICODE=1
LINKLIBS = netapi32.lib
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif // end of makefile definitions

// Required headers
#define WINDOWS_LEAN_AND_MEAN  // faster compile
#include <windows.h>
#ifndef RC_INVOKED    // start of source code

#include "msiquery.h"
#include "msidefs.h"
#include <windows.h>
#include <basetsd.h>
#include <stdlib.h>
#include <lm.h>

#define UNICODE 1

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, all rights reserved
//
//  File:       create.cpp
//
//  Notes: DLL custom action sample , must be used in conjunction with the DLL
//         custom actions included in process.cpp and remove.cpp
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//
// BUILD Instructions
//
// notes:
//  - SDK represents the full path to the install location of the
//     Windows Installer SDK
//
// Using NMake:
//      %vcbin%\nmake -f create.cpp include="%include;SDK\Include" lib="%lib%;SDK\Lib"
//
// Using MsDev:
//      1. Create a new Win32 DLL project
//      2. Add create.cpp to the project
//      3. Add SDK\Include and SDK\Lib directories on the Tools\Options Directories tab
//      4. Add msi.lib and netapi32.lib to the library list in the Project Settings dialog
//          (in addition to the standard libs included by MsDev)
//      5. Add /DUNICODE to the project options in the Project Settings dialog
//
//------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// ClearSecret
//
//     Zeroes the secret data in the wszSecret buffer.  This is to reduce
//     the amount of time the secret data is kept in clear text in memory. 
//
void ClearSecret(__out_ecount(cchSecret) WCHAR* wszSecret, DWORD cchSecret)
{
	if (!wszSecret)
		return; // nothing to do

	volatile char* vpch = reinterpret_cast<volatile char*>(wszSecret);
	DWORD cbSecret = cchSecret*sizeof(WCHAR);
	while (cbSecret)
	{
		*vpch = 0;
		vpch++;
		cbSecret--;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CreateUserAccount
//
//     Attempts to create a user account on the local machine according
//       to the "instructions" provided in the CustomActionData property
//
//     As a deferred custom action, you do not have access to the database.
//       The only source of infromation comes from a property that another
//       custom action can set to provide the information you need.  This
//       property is written into the script
//
UINT __stdcall CreateUserAccount(MSIHANDLE hInstall)
{
	const WCHAR* wszSep = L"\001";
	const int iCreationError = 25001;
	const int iCreationDuplicate = 25002;

	// Grab the CustomActionData property
	WCHAR* wszCAData = 0;
	WCHAR* wszTokenContext = NULL;
	DWORD cchCAData = 0;
	
	if (ERROR_MORE_DATA != MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, L"", &cchCAData))
		return ERROR_INSTALL_FAILURE;
	
	wszCAData = new WCHAR[++cchCAData];
	if ( !wszCAData )
		return ERROR_INSTALL_FAILURE; // out of memory
	
	wszCAData[0] = 0;

	if (ERROR_SUCCESS != MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, wszCAData, &cchCAData))
	{
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE; // error -- should never happen
	}

	USER_INFO_1 ui;
	ZeroMemory(&ui, sizeof(USER_INFO_1));

	DWORD dwLevel = 1; // represents USER_INFO_1 struct
	NET_API_STATUS nStatus = NERR_Success;

	//
	// Parse CustomActionDataProperty
	//
	WCHAR* wszUserName = wcstok_s(wszCAData, wszSep, &wszTokenContext);
	if ( !wszUserName )
	{
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	WCHAR* wszPassWd   = wcstok_s(NULL, wszSep, &wszTokenContext);
	if ( !wszPassWd )
	{
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	WCHAR* pwch = wcstok_s(NULL, wszSep, &wszTokenContext);
	if ( !pwch )
	{
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE; // error -- should never happen
	}

	int iUserFlags = wcstol(pwch, 0, 10);
	
	//
	// Set up the USER_INFO_1 structure.
	//  USER_PRIV_USER: name identifies a user, 
	//    rather than an administrator or a guest.
	//  UF_SCRIPT: required for LAN Manager 2.0 and
	//    Windows NT/Windows 2000.
	//
	ui.usri1_name = wszUserName;
	ui.usri1_password = wszPassWd;
	ui.usri1_priv = USER_PRIV_USER;
	ui.usri1_flags = UF_SCRIPT | iUserFlags;
	ui.usri1_home_dir = NULL;
	ui.usri1_comment = NULL;
	ui.usri1_script_path = NULL;

	// Send ActionData message (template in ActionText table)
	PMSIHANDLE hRec = MsiCreateRecord(1);
	if ( !hRec || ERROR_SUCCESS != MsiRecordSetStringW(hRec, 1, wszUserName))
	{
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	
	int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
	if (IDCANCEL == iRet || IDABORT == iRet)
	{
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_USEREXIT;
	}


	//
	// Call the NetUserAdd function, specifying level 1.
	//
	nStatus = NetUserAdd(NULL /*local machine*/, dwLevel, (LPBYTE)&ui, NULL);  

	if (nStatus != NERR_Success)
	{
		PMSIHANDLE hRecErr = MsiCreateRecord(3);
		if ( !hRecErr 
			|| ERROR_SUCCESS != MsiRecordSetInteger(hRecErr, 1, (nStatus == NERR_UserExists) ? iCreationDuplicate : iCreationError)
			|| ERROR_SUCCESS != MsiRecordSetStringW(hRecErr, 2, wszUserName)
			|| ERROR_SUCCESS != MsiRecordSetInteger(hRecErr, 3, nStatus))
		{
			ClearSecret(wszCAData, cchCAData);
			delete [] wszCAData;
			return ERROR_INSTALL_FAILURE;
		}
		
		// ignore MsiProcessMessage return below because we are aborting the install
		MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecErr);
	
		ClearSecret(wszCAData, cchCAData);
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE; // error
	}

	ClearSecret(wszCAData, cchCAData);
	delete [] wszCAData;
	return ERROR_SUCCESS;
}

#else // RC_INVOKED, end of source code, start of resources
// resource definition go here

#endif // RC_INVOKED
#if 0 
!endif // makefile terminator
#endif
