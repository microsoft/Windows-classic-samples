#if 0  // makefile definitions
DESCRIPTION = Process UserAccounts Database Table
MODULENAME = process
FILEVERSION = Msi
ENTRY = ProcessUserAccounts,UninstallUserAccounts
LINKLIBS = netapi32.lib
UNICODE=1
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif // end of makefile definitions

// Required headers
#define WINDOWS_LEAN_AND_MEAN  // faster compile
#include <windows.h>
#ifndef RC_INVOKED    // start of source code

#include "msiquery.h"
#include <windows.h>
#include <basetsd.h>
#include <lm.h>
#include "strsafe.h"

#define UNICODE 1

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, all rights reserved.
//
//  File:       process.cpp
//
//  Notes: DLL custom actions, must be used in conjunction with the DLL
//         custom actions included in create.cpp and remove.cpp
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//
// BUILD Instructions
//
// notes:
//	- SDK represents the full path to the install location of the
//     Windows Installer SDK
//
// Using NMake:
//		%vcbin%\nmake -f process.cpp include="%include;SDK\Include" lib="%lib%;SDK\Lib"
//
// Using MsDev:
//		1. Create a new Win32 DLL project
//      2. Add process.cpp to the project
//      3. Add SDK\Include and SDK\Lib directories on the Tools\Options Directories tab
//      4. Add msi.libto the library list in the Project Settings dialog
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

//////////////////////////////////////////////////////////////////////////
// ProcessUserAccounts (resides in process.dll)
//
//     Process the UserAccounts custom table generating deferred actions
//       to handle account creation (requires elevated priviledges) and
//       rollback
//
UINT __stdcall ProcessUserAccounts(MSIHANDLE hInstall)
{
	PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
	if ( !hDatabase )
		return ERROR_INSTALL_FAILURE;

	PMSIHANDLE hView = 0;
	
	//
	// constants -- custom action names, SQL query, separator
	//
	// wszCreateCA = name of deferred CA to create account
	// wszRollbackCA = name of rollback CA to rollback account creation
	//
	const WCHAR wszCreateCA[] = L"CreateAccount";
	const WCHAR wszRollbackCA[] = L"RollbackAccount";
	const WCHAR wszSQL[] = L"SELECT `UserName`, `Password`, `Attributes` FROM `CustomUserAccounts`";
	const WCHAR wchSep = '\001';

	UINT uiStat = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (uiStat = MsiDatabaseOpenViewW(hDatabase, wszSQL, &hView)))
	{
		if (ERROR_BAD_QUERY_SYNTAX == uiStat
			&& MSICONDITION_NONE == MsiDatabaseIsTablePersistentW(hDatabase, L"CustomUserAccounts"))
			return ERROR_SUCCESS; // table not present
		else
			return ERROR_INSTALL_FAILURE; // error -- should never happen
	}
	if (ERROR_SUCCESS != (uiStat = MsiViewExecute(hView, 0)))
		return ERROR_INSTALL_FAILURE; // error -- should never happen

	// Fetch all entries from the CustomUserAccounts table
	PMSIHANDLE hRecFetch = 0;
	while (ERROR_SUCCESS == (uiStat = MsiViewFetch(hView, &hRecFetch)))
	{
		// Obtain user name
		WCHAR* wszUser = 0;
		DWORD cchUser = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 1, L"", &cchUser))
			return ERROR_INSTALL_FAILURE;

		wszUser = new WCHAR[++cchUser];
		if ( !wszUser )
			return ERROR_INSTALL_FAILURE; // out of memory

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 1, wszUser, &cchUser))
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE; // error -- should never happen
		}
		
		// Obtain name of property containing password
		WCHAR* wszPassWdProp = 0;
		DWORD cchPassWdProp = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 2, L"", &cchPassWdProp))
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE;
		}

		wszPassWdProp = new WCHAR[++cchPassWdProp];
		if ( !wszPassWdProp )
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE; // out of memory
		}

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 2, wszPassWdProp, &cchPassWdProp))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE; // error -- should never happen
		}

		// Obtain property value containing password
		WCHAR* wszAccountPassWd = 0;
		DWORD cchAccountPassWd = 0;
		if (ERROR_MORE_DATA != MsiGetProperty(hInstall, wszPassWdProp, L"", &cchAccountPassWd))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE;
		}

		wszAccountPassWd = new WCHAR[++cchAccountPassWd];
		if ( !wszAccountPassWd )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE;
		}

		ZeroMemory(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));

		if (ERROR_SUCCESS != MsiGetProperty(hInstall, wszPassWdProp, wszAccountPassWd, &cchAccountPassWd)
			|| *wszAccountPassWd == 0) // do not allow blank password
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE;
		}
		
		// Obtain attributes of user account
		WCHAR* wszAttrib  = 0;
		DWORD cchAttrib = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 3, L"", &cchAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd);
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE;
		}

		wszAttrib = new WCHAR[++cchAttrib];
		if ( !wszAttrib )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd);
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE; // out of memory
		}

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 3, wszAttrib, &cchAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd);
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			return ERROR_INSTALL_FAILURE; // error -- should never happen
		}

		// Generate the customized property that the deferred action will read
		DWORD cchBuf = cchUser + cchAccountPassWd + cchAttrib + 4;
		WCHAR* wszBuf = new WCHAR[cchBuf];
		if ( !wszBuf )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd);
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			return ERROR_INSTALL_FAILURE; // out of memory
		}

		wszBuf[0] = 0;

		if (FAILED(StringCchPrintfW(wszBuf, cchBuf, L"%s%c%s%c%s", wszUser, wchSep, wszAccountPassWd, wchSep, wszAttrib)))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd);
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}

		// clear wszAccountPassWd
		ClearSecret(wszAccountPassWd, cchAccountPassWd);


		// Add action data (template is in ActionText table), but do not display temp passwd
		PMSIHANDLE hRecInfo = MsiCreateRecord(2);
		if ( !hRecInfo 
            || ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 1, wszUser)
			|| ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 2, wszAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf);
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}
		
		int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRecInfo);
		if (IDCANCEL == iRet || IDABORT == iRet)
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf);
			delete [] wszBuf;
			return ERROR_INSTALL_USEREXIT;
		}

		// Rollback custom action goes first
		// Create a rollback custom action (in case install is stopped and rolls back)
		// Rollback custom action can't read tables, so we have to set a property
		if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRollbackCA, wszUser)
			|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszRollbackCA)
			|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRollbackCA, L""))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf);
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}

		// Create a deferred custom action (gives us the right priviledges to create the user account)
		// Deferred custom actions can't read tables, so we have to set a property
		if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszCreateCA, wszBuf)
			|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszCreateCA)
			|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszCreateCA, L""))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf);
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}


		ClearSecret(wszBuf, cchBuf);
		delete [] wszBuf;
		delete [] wszUser;
		delete [] wszPassWdProp;
		delete [] wszAccountPassWd;
		delete [] wszAttrib;
	}
	return (ERROR_NO_MORE_ITEMS != uiStat) ? ERROR_INSTALL_FAILURE : ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// UninstallUserAccounts (resides in process.dll)
//
//     Process the UserAccounts custom table generating deferred actions
//       to handle removal of user accounts
//
UINT __stdcall UninstallUserAccounts(MSIHANDLE hInstall)
{
	//
	// constants -- custom action name SQL query
	//
	// wszRemoveCA = name of deferred CA to remove user account
	//
	const WCHAR wszRemoveCA[] = L"RemoveAccount";
	const WCHAR wszSQL[] = L"SELECT `UserName` FROM `CustomUserAccounts`";

	PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
	if ( !hDatabase )
		return ERROR_INSTALL_FAILURE;

	PMSIHANDLE hView = 0;

	UINT uiStat = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (uiStat = MsiDatabaseOpenViewW(hDatabase, wszSQL, &hView)))
	{
		if (ERROR_BAD_QUERY_SYNTAX == uiStat 
			&& MSICONDITION_NONE == MsiDatabaseIsTablePersistentW(hDatabase, L"CustomUserAccounts"))
			return ERROR_SUCCESS; // table not present
		else
			return ERROR_INSTALL_FAILURE;
	}

	if (ERROR_SUCCESS != MsiViewExecute(hView, 0))
		return ERROR_INSTALL_FAILURE; // error -- should never happen

	// Fetch all entries from the UserAccounts custom table
	PMSIHANDLE hRecFetch = 0;
	while (ERROR_SUCCESS == (uiStat = MsiViewFetch(hView, &hRecFetch)))
	{
		// Obtain user name
		DWORD cchUser = 0;
		if (ERROR_MORE_DATA == MsiRecordGetStringW(hRecFetch, 1, L"", &cchUser))
		{
			WCHAR* wszUser = new WCHAR[++cchUser]; // add 1 for null-terminator which is not included in return count
			if (wszUser)
			{
				if (ERROR_SUCCESS == MsiRecordGetStringW(hRecFetch, 1, wszUser, &cchUser))
				{
					// Send ActionData message (template is in ActionText table)
					PMSIHANDLE hRecInfo = MsiCreateRecord(1);
					if ( !hRecInfo
						|| ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 1, wszUser))
					{
						delete [] wszUser;
						return ERROR_INSTALL_FAILURE;
					}

					int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRecInfo);
					if (IDCANCEL == iRet || IDABORT == iRet)
					{
						delete [] wszUser;
						return ERROR_INSTALL_USEREXIT;
					}

					// We can't do a rollback custom action here (well, we could, but it wouldn't be correct)
					// After a user account has been deleted, it cannot be recreated exactly as it was before
					// because it will have been assigned a new SID.  In the case of uninstall, we won't
					// rollback the deletion.

					// Create a deferred custom action (gives us the right priviledges to create the user account)
					// Deferred custom actions can't read tables, so we have to set a property
					if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRemoveCA, wszUser)
						|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszRemoveCA)
						|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRemoveCA, L""))
					{
						delete [] wszUser;
						return ERROR_INSTALL_FAILURE;
					}
				}
				delete [] wszUser;
			}
			else
				return ERROR_INSTALL_FAILURE;
		}
		else
			return ERROR_INSTALL_FAILURE;
	}
	if (ERROR_NO_MORE_ITEMS != uiStat)
		return ERROR_INSTALL_FAILURE; // error -- should never happen

	return ERROR_SUCCESS;
}

#else // RC_INVOKED, end of source code, start of resources
// resource definition go here

#endif // RC_INVOKED
#if 0 
!endif // makefile terminator
#endif
