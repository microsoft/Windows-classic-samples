//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ASSERTS.H | Assertion Routines
//
//
#ifndef _ASSERTS_H_
#define _ASSERTS_H_


///////////////////////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////////////////////
#include <crtdbg.h>		//_ASSERTE


//-----------------------------------------------------------------------------
// Global function prototypes -- helper stuff
//-----------------------------------------------------------------------------

LRESULT wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszBuffer);
HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cStrLen);
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cStrLen);

HRESULT FreeProperties(ULONG* pcPropSets, DBPROPSET** prgPropSets);
HRESULT FreeProperties(ULONG* pcProperties, DBPROP** prgProperties);


//-----------------------------------------------------------------------------
// Debugging macros
//-----------------------------------------------------------------------------

# define ASSERT			_ASSERTE
# define assert			ASSERT

#ifdef _DEBUG
# define TRACE			OLEDB_Trace
# define DEBUGCODE(p)	p
#else	//_DEBUG
# define TRACE			OLEDB_Trace
  inline void			OLEDB_Trace( const char *format, ... ) { /* do nothing */ }
# define DEBUGCODE(p)
#endif	//_DEBUG

#endif


