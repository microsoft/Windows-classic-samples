//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IROWSCRL.H 
//
//
//

#ifndef _IROWSCRL_H_
#define _IROWSCRL_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

#define TABLE_ROW_COUNT			20
//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszCreateTableFailed[] =L"Create table failed!\n";
const WCHAR wszIRowsetScrollNotSupported[] = L"IRowsetScroll interface is not supported by the provider.  No test will be run.\n";
const WCHAR wszExcuteCommandFailed[] = L"CTable::ExcuteCommand failed!\n";
const WCHAR wszInexactPositionWarning[] = L"Warning, an inexact Row Position was returned.\n";
const WCHAR wszInexactRowCountWarning[] = L"Warning, an inexact Row Count was returned.\n";


#endif 	//_IROWSCRL_H_
