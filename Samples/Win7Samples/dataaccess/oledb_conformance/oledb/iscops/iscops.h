//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1998-2000 Microsoft Corporation
//
// @doc
//
// @module IScOps.h | This module contains header information for 
//	    				OLE DB IScopedOperations interface Test
//
// @rev 01 | 10-08-98 | Microsoft | Created
//

#ifndef _ISCOPS_H_
#define _ISCOPS_H_
						  

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		// Private Library

const ULONG	cbMaxLen		= 1000;
const ULONG	c_ulMaxRow		= 1024;
const ULONG	c_ulRootRow0	= 1301;
const ULONG c_ulRootRow		= 7117;
const ULONG c_ulTestRow		= 5000;

const ULONG	cMaxRows		= 31;

WCHAR		*g_pwszRootRow0	= NULL;
WCHAR		*g_pwszRootRow	= NULL;

#define		AGGREGATION_SUPPORT(hr)					\
				if(hr == DB_E_NOAGGREGATION)		\
				{									\
					odtLog<<L"INFO: Aggregation is not supported.\n";	\
					TESTB = TEST_SKIPPED;								\
					goto CLEANUP;										\
				}


#endif  // _ISCOPS_H_