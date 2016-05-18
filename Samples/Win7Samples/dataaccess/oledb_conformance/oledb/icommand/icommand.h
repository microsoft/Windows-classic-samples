//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module COMMAND.H | Header file for ICommand test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _COMMAND_H_
#define _COMMAND_H_
						  
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"
#include <process.h>		// for multi-threading routines

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define GETROWS 20
#define MAX_ROWS 15
#define TABLE_RESTRICT		0x3		// Restriction bit set for TableName restriction
#define RESTRICTION_COUNT	4		// Count of restrictions
#define TABLES_COLS			3		// Count of columns needed from Tables rowset

#define	DEFINTF		IRowset *	pRowset = NULL;		\
					IRow	*	pRow     = NULL;

#define	RELINTF		SAFE_RELEASE(pRowset);		\
					SAFE_RELEASE(pRow);

#define	IFROWSET	(m_eTestCase==TC_Rowset)
#define	IFROW		(m_eTestCase==TC_Row)

#define	ONLYROWSETVAR		if(m_eTestCase!=TC_Rowset)	\
							{							\
								odtLog << L"This variation only applicable to TC_Rowset.\n";\
								return TEST_SKIPPED;	\
							}

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const BOOL	fMemNotAllocated=FALSE;
const BOOL  fMemAllocated = TRUE;

const ULONG ulMAXROW = 989;

const WCHAR  wszSemicolon[]=L";";
const WCHAR	 wszTABLE1[]=L"BigTable1";
const WCHAR	 wszTABLE2[]=L"BigTable2";
const WCHAR  wszTABLE3[]=L"BigTable3";
const WCHAR  wszTABLE4[]=L"BigTable4";

const WCHAR  wszSELECT_BIGQUERY[]=L"Select * from BigTable1,BigTable2,BigTable3,BigTable4";
const WCHAR  wszWAIT[]=L"waitfor delay '00:01:00'";

const WCHAR	 wszRowsetName[]=L"DBROWSET_ROOT";

const WCHAR  wszInsertInvalidNum[]=L"Insert into %s values (256)";
const WCHAR  wszInsertInvalidChar[]=L"Insert into %s values ('PUNT')";
const WCHAR  wszDropProc[]=L"Drop Proc proc1";
const WCHAR  wszInsertProc[]=L"Create Proc proc1 as Insert into %s values(7)";
const WCHAR  wszExecuteProc[]=L"proc1";

const WCHAR	 wszThreadFailure[]=L"Second Thread didn't cancel execution\n";
const WCHAR  wszExecutionFailure[]=L"Expected S_OK or DB_E_CANCELED but didn't get it\n";
const WCHAR  wszV2[]=L"Waiting for v2\n";

const WCHAR wszCreateStringTable[]		= L"Create table %s (col1 %s(%d))";
const WCHAR	wszInsertInvalidCharValue[]	= L"Insert into %s values ('%s')";
const WCHAR	wszDropTable[]				= L"Drop table %s";
const WCHAR	wszDropView[]				= L"Drop view %s";
const WCHAR wszCreateTable[]			= L"Create table %s (col1 char(10))";
const WCHAR	wszCreateView[]				= L"Create view %s as select * from %s";
const WCHAR	wszSelectAll[]				= L"Select * from %s";
const WCHAR	wszSelectBadSelect[]		= L"Select BOGUS STATEMENT";
const WCHAR	wszSelectBadColName[]		= L"Select BadColumnName from %s";
const WCHAR	wszInsertInvalidValue[]		= L"Insert into %s values (%s)";
const WCHAR	wszInsertInvalidDateValue[]	= L"Insert into %s values (%s%s%s)";
const WCHAR	wszSelectBadTblName[]		= L"Select * from %s";
const WCHAR	wszBogusTblName[]			= L"BOGUSTABLENAME";
const WCHAR	wszInvalidDate[]			= L"0000-00-00";
const WCHAR	wszInvalidTime[]			= L"00:60:60";
const WCHAR	wszInvalidDateTime[]		= L"0000-00-00 00:60:60";

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum ACCESSOR
{
	READWRITE,
	READ
};

enum SET_OPTION_ENUM
{
	SINGLE_PROP,
	PAIRED_PROP
};

//This enumeration represents various objects that can be 
//obtained from Execute. For e.g., TC_Row represents the Test 
//Case which will test getting a ROW object from Execute.
enum ETESTCASE
{
	TC_Rowset = 1,				//Getting Rowset from Execute
	TC_Row,						//Getting Row from Execute (singleton)
};

#endif 	//_COMMAND_H_
