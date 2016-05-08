
/************************************************************************
 *
 * File: TextDialogSample.h
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#pragma once

// Ignore unreferenced parameters, since they are very common
// when implementing callbacks.
#pragma warning(disable : 4100)

// Target version information.
// Modify the following defines if you have to target a platform prior to the ones pecified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER              // Allow use of features pecific to Windows 7 or later.
#define WINVER 0x0701       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features pecific to Windows 7 or later.
#define _WIN32_WINNT 0x0701 // Change this to the appropriate value to target other versions of Windows.
#endif


/******************************************************************
*                                                                 *
*  Includes                                                       *
*                                                                 *
******************************************************************/


#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <intsafe.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

// Additional includes.
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <string.h>
#include <commctrl.h>
#include <wchar.h>

#include <new>

// SafeRelease inline function.
template <class T> inline void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// Macros

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// If an error occurred, display the HRESULT and exit.
#ifndef EXIT_ON_ERROR
#define EXIT_ON_ERROR(hr) if (FAILED(hr)) { wchar_t error[255]; swprintf_s(error, 254, L"HRESULT = %x", hr); MessageBox(0, error, L"Error, exiting.", 0); exit(1);    }
#endif

// Local files.
#include "resource.h"
#include "TextDialog.h"

