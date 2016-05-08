//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICMDPREP.H | Header file for ICommandPrepare test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _ICMDPREP_H_
#define _ICMDPREP_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszDropTable[]				= L"Drop table %s";
const WCHAR	wszDropView[]				= L"Drop view %s";
const WCHAR wszCreateTable[]			= L"Create table %s (col1 char(10))";
const WCHAR wszCreateStringTable[]		= L"Create table %s (col1 %s(%d))";
const WCHAR	wszCreateView[]				= L"Create view %s as select * from %s";
const WCHAR	wszSelectAll[]				= L"SELECT * FROM %s";
const WCHAR	wszSelectParam[]			= L"SELECT * FROM %s where %s > ?";
const WCHAR	wszSelectBadSelect[]		= L"Select * BOGUS STATEMENT";
const WCHAR	wszSelectBadColName[]		= L"Select BadColumnName from %s";
const WCHAR	wszInsertInvalidValue[]		= L"Insert into %s values (%s)";
const WCHAR	wszInsertInvalidCharValue[]	= L"Insert into %s values ('%s')";
const WCHAR	wszInsertInvalidDateValue[]	= L"Insert into %s values (%s%s%s)";
const WCHAR	wszInsertInvalidChar[]		= L"Insert into %s values ('PUNT')";
const WCHAR	wszSelectBadTblName[]		= L"Select * from %s";
const WCHAR	wszBogusTblName[]			= L"BOGUSTABLENAME";
const WCHAR	wszInvalidDate[]			= L"0000-00-00";
const WCHAR	wszInvalidTime[]			= L"00:60:60";
const WCHAR	wszInvalidDateTime[]		= L"0000-00-00 00:60:60";
const WCHAR  wszPropertySet[]			= L"WARNING: IDBProperties::GetPropertiesInfo says the property is readonly, but SetRowsetProperty succeeded.\n";

#endif 	//_ICMDPREP_H_
