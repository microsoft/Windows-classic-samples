#if 0  // makefile definitions
DESCRIPTION = LaunchTutorial custom action sample
MODULENAME = tutorial
FILEVERSION = Msi
LINKLIBS = shell32.lib
ENTRY = LaunchTutorial
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif // end of makefile definitions

// Required headers
#define WINDOWS_LEAN_AND_MEAN  // faster compile
#include <windows.h>
#ifndef RC_INVOKED    // start of source code

#include <tchar.h>   // define UNICODE=1 on nmake command line to build UNICODE
#include <shellapi.h>
#include "msiquery.h"
#include "strsafe.h"

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, All rights reserved.
//
//  File:       tutorial.cpp
//
//  Purpose: DLL custom action sample code to demonstrate how to launch an
//           installed file at the end of setup
//
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
//		%vcbin%\nmake -f tutorial.cpp include="%include;SDK\Include" lib="%lib%;SDK\Lib"
//
// Using MsDev:
//		1. Create a new Win32 DLL project
//      2. Add tutorial.cpp to the project
//      3. Add SDK\Include and SDK\Lib directories on the Tools\Options Directories tab
//      4. Add msi.lib to the library list in the Project Settings dialog
//          (in addition to the standard libs included by MsDev)
//
//------------------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// LaunchTutorial
//
// Launches a installed file at the end of setup
//
UINT __stdcall LaunchTutorial(MSIHANDLE hInstall)
{
	BOOL fSuccess = FALSE;

	// szTutorialFileKey is the primary key of the file in the
	// File table that identifies the file we wish to launch
	const TCHAR szTutorialFileKey[] = TEXT("[#Tutorial]");

	PMSIHANDLE hRecTutorial = MsiCreateRecord(1);

	if ( !hRecTutorial
		|| ERROR_SUCCESS != MsiRecordSetString(hRecTutorial, 0, szTutorialFileKey))
		return ERROR_INSTALL_FAILURE;

	// determine buffer size
	DWORD cchPath = 0;
	if (ERROR_MORE_DATA == MsiFormatRecord(hInstall, hRecTutorial, TEXT(""), &cchPath))
	{
		// add 1 to cchPath since return count from MsiFormatRecord does not include terminating null
		TCHAR* szPath = new TCHAR[++cchPath];
		if (szPath)
		{
			if (ERROR_SUCCESS == MsiFormatRecord(hInstall, hRecTutorial, szPath, &cchPath))
			{
				// ensure quoted path to ShellExecute
				DWORD cchQuotedPath = lstrlen(szPath) + 1 + 2; // szPath + null terminator + enclosing quotes
				TCHAR* szQuotedPath = new TCHAR[cchQuotedPath];
				if (szQuotedPath
					&& SUCCEEDED(StringCchCopy(szQuotedPath, cchQuotedPath, TEXT("\"")))
					&& SUCCEEDED(StringCchCat(szQuotedPath, cchQuotedPath, szPath))
					&& SUCCEEDED(StringCchCat(szQuotedPath, cchQuotedPath, TEXT("\""))))
				{
					// set up ShellExecute structure
					// file is the full path to the installed file
					SHELLEXECUTEINFO sei;
					ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
					sei.fMask = SEE_MASK_FLAG_NO_UI; // don't show error UI, we'll just silently fail
					sei.hwnd = 0;
					sei.lpVerb = NULL; // use default verb, typically open
					sei.lpFile = szQuotedPath;
					sei.lpParameters = NULL;
					sei.lpDirectory = NULL;
					sei.nShow = SW_SHOWNORMAL;
					sei.cbSize = sizeof(sei);

					// spawn the browser to display HTML tutorial
					fSuccess = ShellExecuteEx(&sei);

					delete [] szQuotedPath;
				}
			}
			delete [] szPath;
		}
	}

	return (fSuccess) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

#else // RC_INVOKED, end of source code, start of resources
// resource definition go here

#endif // RC_INVOKED
#if 0 
!endif // makefile terminator
#endif
