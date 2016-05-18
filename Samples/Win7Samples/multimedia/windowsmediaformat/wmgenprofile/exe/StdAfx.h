//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            stdafx.cpp
//
// Abstract:            Include file for standard system include files, or
//                      project specific include files that are used
//                      frequently, but are changed infrequently
//
//*****************************************************************************

#if !defined(AFX_STDAFX_H__A74C9049_8B1B_4609_97A4_39AC691990F1__INCLUDED_)
#define AFX_STDAFX_H__A74C9049_8B1B_4609_97A4_39AC691990F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WINVER		
#define WINVER 0x0501		
#endif



#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>        // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>            // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <assert.h>
#include <mmreg.h>
#include <mlang.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A74C9049_8B1B_4609_97A4_39AC691990F1__INCLUDED_)
