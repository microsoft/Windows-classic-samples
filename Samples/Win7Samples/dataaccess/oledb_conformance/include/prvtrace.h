//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module PRVTRACE.H | TRACING support
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 PRVTRACE Elements|
//
//---------------------------------------------------------------------------

#ifndef __PRVTRACE_INCL__
#define __PRVTRACE_INCL__


/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "crtdbg.h"		//_ASSERTE


/////////////////////////////////////////////////////////////////////
// Memory debugging code
//
/////////////////////////////////////////////////////////////////////
#define NUMELEM(rgEle) (sizeof(rgEle) / sizeof(rgEle[0]))
#define MAX_TRACE_LEN  4096

INT MessageBox(HWND hwnd, UINT uiStyle, CHAR* pszTitle, CHAR* pszFmt, ...);
void InternalTrace(const CHAR* pszExp, ...);
void InternalTrace(const WCHAR* pwszExp, ...);

#undef ASSERT
#undef TRACE

//DebugBreak
#if     defined(_M_IX86)
#define _DbgBreak() __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

#ifdef _DEBUG
#define ASSERT(exp)		_ASSERTE(exp)
#define PRVTRACE		InternalTrace
#define TRACE			InternalTrace
#else  //_DEBUG
#define ASSERT(exp)
#define PRVTRACE		if(0) InternalTrace
#define TRACE			if(0) InternalTrace
#endif //_DEBUG

#endif	// __PRVTRACE_INCL__
