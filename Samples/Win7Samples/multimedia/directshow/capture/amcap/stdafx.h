// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__08E7CDC6_7F58_11D2_8CF1_00A0C9441E20__INCLUDED_)
#define AFX_STDAFX_H__08E7CDC6_7F58_11D2_8CF1_00A0C9441E20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0501

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#define _ATL_APARTMENT_THREADED
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


#include <windows.h>
#include <streams.h>
#include <strsafe.h>

#include "DShowUtil.h"
#include "smartptr.h"   // smart pointer class


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__08E7CDC6_7F58_11D2_8CF1_00A0C9441E20__INCLUDED)
