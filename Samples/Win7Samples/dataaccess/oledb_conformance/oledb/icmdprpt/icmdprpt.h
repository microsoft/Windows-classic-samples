//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICMDPRPT.H | Header file for ICommandProperties test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _ICMDPRPT_H_
#define _ICMDPRPT_H_

#include "oledb.h" 			//OLE DB Header Files
#include "oledberr.h"

#include "msdasql.h"		//KAGERA Header File

#include "Privlib.h"		//Privlib header
#include "assert.h"			//assert header

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszCMDMemoryAllocationError[] = L"Unable to allocate sufficient memory.\n";
const WCHAR	wszCMDInterfaceNotSupported[] = L"Interface ICommandProperties is not supported. Test Module Pass!\n";
const WCHAR	wszTestCaseInitFail[] = L"The init routine for the test case failed!\n";
const WCHAR wszInterfaceUnexpectedErr[] = L"Unexpected error in queryinterface on the command object. \n";
const WCHAR wszCanNotCreateTable[] = L"Can not create a table.\n";
const WCHAR	wszCanNotOpenRowset[] = L"Can not open a rowset. \n";
const WCHAR	wszICommandTextNotSupprted[] = L"ICommandText not supported. \n";
const WCHAR	wszCMDSelectStar[] = L"Select * from ";
const WCHAR wszIAccessorNotSupported[] = L"ERROR::IAccessor on rowset object not supported. \n";
const WCHAR wszIRowsetInfoNotSupported[] = L"ERROR::IRowsetInfo on rowset object not supported. \n";
const WCHAR wszIColumnsInfoNotSupported[] = L"ERROR::IColumnsInfo on rowset object not supported. \n";
const WCHAR wszIConvertTypeNotSupported[] = L"ERROR::IConvertType on rowset object not supported. \n";
const WCHAR wszIRowsetNotSupported[] = L"ERROR::IRowset on rowset object not supported. \n";
const WCHAR	wszIndexNotValid[] = L"Test module error: The index for the global array not valid! \n";
const WCHAR	wszUnexpectedPropertyValue[]=L"Unexpected DBProperty options returned on InitSupported! the index is ";
const WCHAR	wszIRowsetORIReadData[]=L"Either IRowset or IReadData should be supported.\n";
const WCHAR	wszSQLServerOnly[]=L"This test variation is for SQL Server only!.\n";
const WCHAR	wszNoneSupported[]=L"None of the commnad properties is supported!. \n";
const WCHAR	wszTotalSupportedIs[]=L"The total number of properties supported is ";
const WCHAR	wszTotalGetPropertiesIs[]=L"The total number of properties supported from GetProperties is ";
const WCHAR wszPropertySupportedAt[]=L"Supported property at index ";
const WCHAR wszPropertyNotSupportedAt[]=L"Not supported property at index ";
const WCHAR	wszRowsetFlagNotSetAt[]=L"DBPROPFLAGS_ROWSET is not set at index ";
const WCHAR wszDataSourceFlagSetAt[]=L"DBPROPFLAGS_DATASOURCE is set at index ";
const WCHAR wszReadFlagNotSetAt[]=L"DBPROPFLAGS_READ is not set at index ";
const WCHAR wszWriteFlagNotSetAt[]=L"DBPROPFLAGS_WRITE is not set at index ";
const WCHAR wszWriteFlagSetAt[]=L"DBPROPFLAGS_WRITE is set at index ";
const WCHAR wszColumnFlagNotSetAt[]=L"DBPROPFLAGS_COLUMNSOK is not set at index ";
const WCHAR wszColumnFlagSetAt[]=L"DBPROPFLAGS_COLUMNSOK is set at index ";
const WCHAR wszIncorrectVtType[]=L"Incorrect vtPropType returned from IDBProperties at index ";
const WCHAR wszDefaultValueFailed[]=L"The default value from IDBProperties is invalid ";
const WCHAR wszPropertySet[]=L"WARNING: IDBProperties::GetPropertiesInfo says the property is readonly, but ICommandProperties::SetProperties succeeded.\n";

//-----------------------------------------------------------------------------
// Constant defines
//-----------------------------------------------------------------------------
#define TABLE_ROW_COUNT		10		//Row count for the table created
#define NOINDEX				0		//Do not create an index on the table
#define STRESS_MAX_CALL		5000	//Maximum # of calls for GetProperties in stress test
#define SELECT_SIZE			120		//The select statement
#define DESCRIPTION_SIZE	256		//Maximum length of description, same as Kagera

//Enum for command properties
enum EPROPERTYATTR	{INTERFACE_MANDATORY, INTERFACE_NOT_SUPPORTED, INTERFACE_SUPPORTED, 
					ROWSET_NOT_SUPPORTED, ROWSET_NOT_SETTABLE, ROWSET_SETTABLE, PROPERTY_INVALID};


#endif 	//_ICMDPRPT_H_
