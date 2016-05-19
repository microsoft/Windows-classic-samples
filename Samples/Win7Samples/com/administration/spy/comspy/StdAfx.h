// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_STDAFX_H__CBFE257B_B030_11D0_B188_00AA00BA3258__INCLUDED_)
#define AFX_STDAFX_H__CBFE257B_B030_11D0_B188_00AA00BA3258__INCLUDED_

#if _MSC_VER >= 1000
// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently
#define STRICT


#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif // !defined(_WIN32_WINNT)

#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CExeModule : public CComModule
{
public:
	LONG Unlock();
	DWORD dwThreadID;
};
extern CExeModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <atlwin.h>
#include <atlstr.h>

#endif // _MSC_VER >= 1000
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CBFE257B_B030_11D0_B188_00AA00BA3258__INCLUDED)
