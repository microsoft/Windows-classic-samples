//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ISESPRPT.H | Header file for ISessionProperties test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _ISESPRPT_H_
#define _ISESPRPT_H_

#include "oledb.h"			//OLE DB Header Files
#include "oledberr.h"
#include "msdasql.h"
#include "privlib.h"		//Include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszPropertyNotSupportedAt[]		= L"Not supported property at index ";
const WCHAR	wszDataSourceInfoFlagNotSetAt[]	= L"DBPROPFLAGS_DATASOURCEINFO is not set at index ";
const WCHAR	wszDataSourceFlagNotSetAt[]		= L"DBPROPFLAGS_DATASOURCE is not set at index ";
const WCHAR	wszInitializeFlagNotSetAt[]		= L"DBPROPFLAGS_DBINIT is not set at index ";
const WCHAR wszReadFlagNotSetAt[]			= L"DBPROPFLAGS_READ is not set at index ";
const WCHAR wszInvalidFlagSetAt[]			= L"Invalid flag is set at index ";
const WCHAR wszWriteFlagNotSetAt[]			= L"DBPROPFLAGS_WRITE is not set at index ";
const WCHAR wszWriteFlagSetAt[]				= L"DBPROPFLAGS_WRITE is set at index ";
const WCHAR wszIncorrectVtType[]			= L"Incorrect vtPropType returned from GetPropertyInfo at index ";
const WCHAR wszDefaultValueFailed[]			= L"The default value from GetPropertyInfo is invalid ";
const WCHAR	wszTotalGetPropertiesIs[]		= L"The total number of properties supported from GetProperties is ";
const WCHAR	wszIndexNotValid[]				= L"Test module error: The index for the global array not valid! \n";

//-----------------------------------------------------------------------------
// Constant defines
//-----------------------------------------------------------------------------
#define DESCRIPTION_SIZE	256		//Same as Kagera has defined

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum EPROPERTYATTR	{PROPERTY_NOT_SUPPORTED, PROPERTY_NOT_SETTABLE, PROPERTY_SETTABLE,
					 PROPERTY_INVALID};

// Used for the Zombie cases
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

#endif 	//_ISESPRPT_H_
