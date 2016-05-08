//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IRowIden.h | Header file for IRowsetIdentity test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _IROWIDEN_H_
#define _IROWIDEN_H_

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
#define TABLE_ROW_MIN			10
#define TABLE_ROW_MAX			50

enum	EACCESSORLOCATION
{
	NO_ACCESSOR,				// no accessor to be created
	ON_ROWSET_ACCESSOR,			// the accessor is created on the rowset
	ON_COMMAND_ACCESSOR,		// the accessor is created on the command
	ON_ROWSET_FETCH_ACCESSOR	// the accessori s created on the rowset object
								// after a row handle is fetched.
};							


enum	ECURSOR
{
	FORWARD_ONLY_CURSOR,		//the cursor is forward only
	STATIC_CURSOR,				//the cursor is static
	KEYSET_DRIVEN_CURSOR,		//the cursor is key set driven
	DYNAMIC_CURSOR				//the cursor is dynamic
};		
   
//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszCreateTableFailed[]			 = L"Create table failed!\n";
const WCHAR wszIRowsetIdentityNotSupported[] = L"IRowsetIdentity interface is not supported by the provider.  No test will be run.\n";
const WCHAR	wszExcuteCommandFailed[]		 = L"Execute command failed!\n";

#endif 	//_IROWIDEN_H_
