//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IDBINFO.H | Header file for IDBInfo test module.
//
// @rev 01 | 08-06-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IDBINFO_H_
#define _IDBINFO_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Include private library

// Enum's used in the test
enum ARRANGELITERAL{ZERO,ONE,HALF,ALL,ALLSUPPORTED,ALLREVERSE,INVALID1,INVALIDALL};
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

// According to 2.0 Spec
#define MAX_DBLITERALINFO 29

#define INIT	if(!InitVar())return TEST_FAIL;
#define TERM	if(!TermVar())return TEST_FAIL;

#endif 	//_IDBINFO_H_