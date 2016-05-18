//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IROWLOCA.H | Header file for IRowsetLocate test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IROWLOCA_H_
#define _IROWLOCA_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "msdasql.h"
#include "msdaguid.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------
#define TABLE_ROW_COUNT			20

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszCreateTableFailed[] =L"Create table failed!\n";
const WCHAR wszIRowsetLocateNotSupported[] = L"IRowsetLocate interface is not supported by the provider.  No test will be run.\n";
const WCHAR wszOpenRowsetFailed[] = L"CTable::CreateRowset failed!\n";
const WCHAR	wszInexactRefCountWarning[] = L"Warning, Row Handle reference counting is inexact.\n";

#endif 	//_IROWLOCA_H_
