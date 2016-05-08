//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IACCESSR.H | Header file for test module IAccessr
//
// @rev 01 | 10-17-95 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _IACCESSR_H_
#define _IACCESSR_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "msdasql.h"		// ODBC Provider specific header file

#include "privlib.h"		// Private Library
#include "math.h"			// pow function
#include "stddef.h"			// offsetof



//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszErrorInserting[] = L"Error occured inserting row into table, following variations will not succeed!\n";
const WCHAR wszCacheDeferredNotSupported[] = L"DBPROP_CACHEDEFERRED is not supported.\n";
const WCHAR wszDeferredNotSupported[] = L"DBPROP_DEFERRED is not supported.\n";
const WCHAR wszDeferredSupported[] = L"DBPROP_DEFERRED is supported, but can't be for this variation.\n";
const WCHAR wszColOrdinal[] = L"The column with ordinal ";
const WCHAR wszNotReferenceable[] = L" is not referenceable.\n";
const WCHAR wszAllColsReferenceable[] = L"All columns in the rowset have MAYREFERENCE set, this variation does not apply to this provider.\n";
const WCHAR wszColsNotReferenceable[] = L"At least one column does not have MAYREFERENCE set, this variation does not apply to this provider.\n";
const WCHAR	wszCommandNotSupported[] = L"Commands not supported, this variation is not applicable\n";
const WCHAR	wszParamAccesNotSupported[] = L"Parameter accessor is not supported, this variation is not applicable\n";
const WCHAR wszNoMSDASQLSupport[] = L"ODBC Provider does not provide support for this variation, returning TEST_PASS\n";

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define MAX_ROW_SIZE		1000
#define MAX_BIND_LIMIT		500
#define FREE_DATA(pData)	{ if (pData) { m_pIMalloc->Free (pData); pData = NULL; } }

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum ETXN	{ETXN_COMMIT, ETXN_ABORT};
enum DEFER_MODE {IMMEDIATE, MAY_DEFERR, MUST_DEFERR};
enum FAILURE_MODE {MAY_FAIL, MUST_FAIL};
enum FIXED_BYREF_SUPPORT {FIXED_BYREF_NONE, FIXED_BYREF_SOME, FIXED_BYREF_ALL};

#endif 	//_IACCESSR_H_
