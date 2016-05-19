// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C382F3EE_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)
#define AFX_STDAFX_H__C382F3EE_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
//#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <mmsystem.h>
#include <ks.h>
#include <ksmedia.h>
#include <strsafe.h>

// macros ------------------------------------------------------------------------------
#define IsValidHandle(h)    ((h) != NULL && (h) != INVALID_HANDLE_VALUE)

#define SafeCloseHandle(h)  if(IsValidHandle((h))) CloseHandle((h)); (h) = NULL

#define SafeLocalFree(p)    if((p)) LocalFree((p)); (p) = NULL

#define ARRAY_ELEMENTS(a)   (sizeof((a)) / sizeof((a)[0]))


// structs ------------------------------------------------------------------------------
typedef struct
{
    const GUID* pguidConstant;
    LPCSTR      pstrRep;

} CONST_GUID_REP, *PCONST_GUID_REP;

typedef struct
{
    DWORD   dwConstant;
    LPCSTR  pstrRep;

} CONST_DWORD_REP, *PCONST_DWORD_REP;

typedef struct
{
    WORD        wTag;
    LPCSTR      szRep;
} CONST_WORD_REP, *PCONST_WORD_REP;

extern CONST_DWORD_REP cdrSpeakers[];


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C382F3EE_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)
