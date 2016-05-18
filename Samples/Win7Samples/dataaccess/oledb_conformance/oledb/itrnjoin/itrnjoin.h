//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ITRANJOIN.H | Header file for ITransactionLocal test module.
//
// @rev 01 | 04-17-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _ITRANJOIN_H_
#define _ITRANJOIN_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "transact.h"
#include "msdasql.h"		//ODBC Provider specific header file

#include "privlib.h"		//Include private library, which includes
							//the "transact.h"
#include "process.h"		//For multithreading

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
const ULONG	TIMEOUT = 5;
const char szDESCRIPTION[] = "Txn Description";

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum ETXN_END_TYPE {ECOMMIT, EABORT};

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszNoProviderSupport[] = L"Provider does not support Isolation Level:  ";

#endif 	//_ITRANLOC_H_
