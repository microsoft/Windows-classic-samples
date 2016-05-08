// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently
//
// This is a part of the Active Template Library.
// Copyright (C) 1996-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#if !defined(AFX_STDAFX_H__48C9E972_76B0_11D1_B293_0040053089AE__INCLUDED_)
#define AFX_STDAFX_H__48C9E972_76B0_11D1_B293_0040053089AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include <afxres.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__48C9E972_76B0_11D1_B293_0040053089AE__INCLUDED)
