/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PeoplePickerDialog.h

Abstract:

    This C header file declares functions for use in the
    with the People Picker Dialog sample code.

--********************************************************************/

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <strsafe.h>
#include <commctrl.h>
#include <p2p.h>

// People Picker Headers
#include "PeoplePickerDialogResource.h"
#include "PeoplePickerModel.h"

//People Picker Dialog API

#define WM_ADDPERSON WM_APP
#define WM_REMOVEPERSON WM_ADDPERSON + 1
#define WM_CLEARPEOPLE WM_REMOVEPERSON + 1

INT_PTR ShowPeoplePicker(HINSTANCE hInst,HWND hWnd, PEER_PEOPLE_NEAR_ME ** ppPerson);
void PeoplePickerFreePerson(PEER_PEOPLE_NEAR_ME * pPerson);
