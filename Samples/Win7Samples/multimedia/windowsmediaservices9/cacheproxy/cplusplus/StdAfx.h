//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            StdAfx.h
//
// Abstract:            
//
//*****************************************************************************

#if !defined(AFX_STDAFX_H__A0A27F9F_B09D_11D2_BEF1_00A0C95EC343__INCLUDED_)
#define AFX_STDAFX_H__A0A27F9F_B09D_11D2_BEF1_00A0C95EC343__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A0A27F9F_B09D_11D2_BEF1_00A0C95EC343__INCLUDED)
