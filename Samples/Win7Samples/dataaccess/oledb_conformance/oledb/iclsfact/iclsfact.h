//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICLSFACT.H | Header file for IClassFactory test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _ICLSFACT_H_
#define _ICLSFACT_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "msdasql.h"		// ODBC Provider

#include "privlib.h"		// Private library

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef HRESULT (__stdcall  *DLLGETCLASSOBJECTFUNC)(REFCLSID, REFIID, LPVOID FAR *);

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszErrorReadingRegistry[] = L"Error occured retrieving dll path from registry.\n";

//-----------------------------------------------------------------------------
// Consts
//-----------------------------------------------------------------------------
const ULONG CLSID_WCHAR_SIZE_IN_BYTES = 78;

#endif 	//_ICLSFACT_H_
