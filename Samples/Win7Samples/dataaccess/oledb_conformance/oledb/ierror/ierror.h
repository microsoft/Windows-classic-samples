//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IERROR.H | Header file for OLE DB Extended Error Object test module.
//
// @rev 01 | 02-07-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IERROR_H_
#define _IERROR_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "msdaguid.h"

#include "privlib.h"		//Include private library

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszErrorSupportedInterfaces[] = L"Interfaces which report supporting Extended Errors: \n";
const WCHAR wszErrorNotSupportedInterfaces[] = L"Interfaces which do not report supporting Extended Errors: \n";
const WCHAR wszStar[] =						  L"************************************************************************\n";
const WCHAR wszVerifyTheseResultsManually[] = L"***************** Verify These Results Manually *****************\n";
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
typedef struct 	
{
	IID	iid;	
	IUnknown * pIUnknown;
	WCHAR	wszName[30];	
	BOOL	fSupportsErrors;

} SUPPORTEDINTERFACES;

typedef struct 	
{
	IID	iid;	
	WCHAR	wszName[30];	
} TOTALINTERFACES;

#endif 	//_IERROR_H_
