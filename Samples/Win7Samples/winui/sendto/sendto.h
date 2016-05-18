/*****************************************************************************
 *
 *  SendTo.h
 *
 *  Copyright (c) 1997 Microsoft Corporation.  All rights reserved.
 *
 *       This source code is only intended as a supplement to
 *       Microsoft Development Tools, q.v. for detailed
 *       information regarding the Microsoft samples programs.
 *
 *  Abstract:
 *
 *      Common header file for the Send To sample.
 *
 *****************************************************************************/

#define STRICT
#define _WIN32_WINDOWS 0x0400
#include <windows.h>
#ifndef RC_INVOKED
#include <windowsx.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <commdlg.h>
#endif

#define IDM_OPEN                0x0100
#define IDM_SENDTOPOPUP         0x0101

#define IDM_SENDTOFIRST         0x0200
#define IDM_SENDTOLAST          0x02FF
