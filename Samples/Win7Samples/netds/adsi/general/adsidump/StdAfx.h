// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__396FF7B8_0B8F_11D2_964F_00C04F8EF907__INCLUDED_)
#define AFX_STDAFX_H__396FF7B8_0B8F_11D2_964F_00C04F8EF907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// TODO: reference additional headers your program requires here
#include <windows.h>

#include <atlbase.h>

// system headers
#include <activeds.h>


// debug info
// Debug macros
#ifdef _DEBUG
#include <crtdbg.h>
#define Assert(x) { _ASSERTE(x); }
#define Verify(x) Assert(x)
#else	// _DEBUG
#define Assert(x) { }
#define Verify(x) { x; }
#endif	// _DEBU
#define Implies(x,y) Assert((!(x)) || (y))

typedef WCHAR * SZ;
typedef const WCHAR * CSZ;

#define UNUSED(x) { x; }


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__396FF7B8_0B8F_11D2_964F_00C04F8EF907__INCLUDED_)
