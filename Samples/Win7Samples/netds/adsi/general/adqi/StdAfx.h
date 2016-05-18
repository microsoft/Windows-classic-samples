// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__81709773_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_STDAFX_H__81709773_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxole.h>
#include <atlbase.h>
#include <atlconv.h>
#include <activeds.h>


typedef void (* ADSDLGPROC)(LPUNKNOWN, LPUNKNOWN*);

typedef struct tagADSIIF 
{
	const GUID    *pIID;
	LPTSTR		   szIf;
	ADSDLGPROC     pFn;	
} ADSIIF;

typedef struct tagADSERRMSG
{
	HRESULT    hr;
	LPCTSTR    pszError;
}ADSERRMSG;

#define MAKEADSENTRY(x)  &IID_##x, _T(#x) 
#define ADDADSERROR(x)   x, _T(#x)
#define DECLAREADSPROC(x) void DlgProc##x(LPUNKNOWN, LPUNKNOWN *ppNew=NULL); \
void DlgProc##x(LPUNKNOWN pUnk, LPUNKNOWN *ppNew) { CDlg##x dlg( pUnk); dlg.DoModal(); }

#define ARROW_SYMBOL _T("--> ")

#include "helper.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__81709773_0672_11D2_B218_0000F87A6B50__INCLUDED_)

