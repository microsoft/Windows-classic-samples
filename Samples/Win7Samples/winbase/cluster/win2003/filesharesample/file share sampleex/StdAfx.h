// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#ifndef _UNICODE
#define _UNICODE            // Microsoft Windows NT Cluster Administrator
                            //   Extension DLLs need to be Unicode
                            //   applications.
#endif

// Choose which threading model you want by commenting or uncommenting
// the proper constant definition.  If you want multi-threading
// (i.e. "both"), comment both definitions out.  Also change the
// THREADFLAGS_xxx set in the DECLARE_REGISTRY macro invokation in ExtObj.h
//#define _ATL_SINGLE_THREADED
#define _ATL_APARTMENT_THREADED

/////////////////////////////////////////////////////////////////////////////
// Include Files
/////////////////////////////////////////////////////////////////////////////

//
// Project-wide pragmas
//
#pragma warning( disable : 4127 )   // conditional expression is constant (for BEGIN_COM_MAP & ASSERT)
#pragma warning( disable : 4505 )   // <x>: unreferenced local function has been removed
#pragma warning( disable : 4511 )   // <x>: copy constructor could not be generated
#pragma warning( disable : 4512 )   // <x>: assignment operator could not be generated
#pragma warning( disable : 4514 )   // <x>: unreferenced inline function has been removed
#pragma warning( disable : 4710 )   // function '<x>' not inlined

/////////////////////////////////////////////////////////////////////////////
// External Include Files
/////////////////////////////////////////////////////////////////////////////

#pragma warning( push, 3 )
#pragma warning( disable : 4601 )   // #pragma push_macro : 'new' is not currently defined as a macro

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>
#include <afxtempl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows 95 Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Need to include this for WM_COMMANDHELP.  Unfortunately, both afxpriv.h and
// atlconv.h define some of the same macros.  Since we are using ATL, we'll use
// the ATL versions.
#include <afxpriv.h>

#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

#include <atlcom.h>

#include <clusapi.h>
#include <resapi.h>

#define STRSAFE_LIB
#include <strsafe.h>

#include "proplist.h"

#pragma warning( pop )

/////////////////////////////////////////////////////////////////////////////
// Common Types
/////////////////////////////////////////////////////////////////////////////

typedef UINT    IDS;
typedef UINT    IDD;

/////////////////////////////////////////////////////////////////////////////
// Custom Resource Control Codes
/////////////////////////////////////////////////////////////////////////////

#define CLUSCTL_RESOURCE_FILESHARESAMPLE_CALL_ISALIVE   CLUSCTL_USER_CODE( 1, CLUS_OBJECT_RESOURCE )

/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////

extern const WCHAR g_wszResourceTypeNames[];
