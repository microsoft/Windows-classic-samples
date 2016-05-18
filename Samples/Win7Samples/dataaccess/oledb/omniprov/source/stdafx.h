// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__B14C8EA8_3632_11D3_AC81_00C04F8DB3D5__INCLUDED_)
#define AFX_STDAFX_H__B14C8EA8_3632_11D3_AC81_00C04F8DB3D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define _ATL_DEBUG_QI
// When the OLEDB_SERVICES key is set to -1 it is assumed that the Provider
// supports Aggregation.
// With _ATL_DEBUG_INTERFACES - 
// _QIThunk::Release does not handle the aggregated release properly. Hence, 
// do not use _ATL_DEBUG_INTERFACES with the OLEDB_SERVICES key set to -1

//#define _ATL_DEBUG_INTERFACES

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
#include <atlctl.h>
#include <atldb.h>
#include <stdio.h> // for FILE operations 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B14C8EA8_3632_11D3_AC81_00C04F8DB3D5__INCLUDED)

