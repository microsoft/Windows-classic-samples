//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IROWRESY.H | Header file for IRowsetResynch test module.
//
// @rev 01 | 06-01-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//
// @module IROWRESY.H | IRowsetResynch Test Module
//

#ifndef _IROWRESY_H_
#define _IROWRESY_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------
enum	ETXN  {ECOMMIT, EABORT};
enum	ETESTROWSETTYPE {EREADONLY, ECHANGEABLE};
enum	EVERIFY {VERIFY_NEW, VERIFY_OLD, VERIFY_IGNORE};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
//Number of times to call resynch methods for stress testing
const ULONG STRESS_RESYNCH_REPS = 20;	

//Consistent Junk with which to fill pointers which shouldn't get touched
const BYTE * JUNK_PTR = (BYTE *)0x12345678;

const WCHAR wszNoProviderSupport[] = L"Provider does not support isolation level:  ";
const WCHAR wszResynchNotSupported[] = L"IRowsetResynch cannot be supported.  This is expected if keyset driven cursors are not available. \n";

//This enumeration represents the different interfaces the test covers
enum ETESTINTERFACE
{
	TI_IRowsetResynch = 1,	//use the IRowsetResynch interface
	TI_IRowsetRefreshTRUE,	//use the IRowsetRefresh interface with RefreshVisibleData(fOverWrite=TRUE)
	TI_IRowsetRefreshFALSE	//use the IRowsetRefresh interface with RefreshVisibleData(fOverWrite=FALSE)
};

//to see if DBROWSTATUS_S_NOCHANGE is returned by 
//the provider when no change is made by resynch
BOOL fnNOCHANGE(	IOpenRowset		*pIOpenRowset,
					CThisTestModule	*pThisTestModule);

//to see if the provider has a visual cache
BOOL fnVisualCache(IOpenRowset		*pIOpenRowset,
				   CThisTestModule	*pThisTestModule);


#endif 	//_IROWRESY_H_