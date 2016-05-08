//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICmdText.h | Header file for ICommandText test module.
//
// @rev 01 | 02-02-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _ICMDTEXT_H_
#define _ICMDTEXT_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes

#define INIT 		if(!InitVariation()){odtLog << wszINITfailed; return TEST_FAIL;}
#define TERM		if(!TerminateVariation()){odtLog << wszTERMfailed; return TEST_FAIL;}

enum ETXN		{ETXN_COMMIT, ETXN_ABORT};
enum EMETHOD	{EMETHOD_SETTEXT, EMETHOD_GETTEXT};

#define LIMIT 10050

const WCHAR wszINITfailed[]=L"Variation initialization failed\n";
const WCHAR wszTERMfailed[]=L"Variation termination failed\n";


//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
#define gcSQLStmt 45

const EQUERY grgSQLStmt[gcSQLStmt]={
	SELECT_ALLFROMTBL,
	SELECT_SEARCHABLE,
	SELECT_UPDATEABLE,
	SELECT_ABCANDCOLLIST,
	SELECT_DISTINCTCOLLISTORDERBY,
	SELECT_REVCOLLIST,
	SELECT_COLLISTGROUPBY,
	SELECT_COLLISTWHERELASTCOLINSELECT,
	SELECT_REVCOLLISTFROMVIEW,
	SELECT_COUNT,
	SELECT_COLLISTSELECTREVCOLLIST,
	SELECT_EMPTYROWSET,
	SELECT_COLLISTFROMTBL,
	SELECT_COLLISTTBLUNIONTBL,
	SELECT_COLLISTORDERBYCOLONECOMPUTEBY,
	SELECT_CROSSPRODUCT,
	SELECT_LEFTOUTERJOIN,
	SELECT_RIGHTOUTERJOIN,
	SELECT_LEFTOUTERJOIN_ESC,
	SELECT_RIGHTOUTERJOIN_ESC,
	SELECT_FROMTBLWITHPARAMS,
	SELECT_CHANGECOLNAME,
	SELECT_DUPLICATECOLUMNS,
	SELECT_REVERSEDUPLICATECOLUMNS,
	SELECT_MAXCOLINQUERY,
	SELECT_COMPUTEDCOLLIST,
	SELECT_UPDATEABLEALLROWS,
	SELECT_ORDERBYNUMERIC,
	SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,
	SELECT_ALL_WITH_BLOB_AT_END,
	SELECT_ALL_WITH_FOR_BROWSE,
	SELECT_ALL_WITH_FOR_UPDATE,
	CREATE_VIEW,
	DROP_VIEW,
	DROP_INDEX,
	ALTER_TABLE_ADD,
	ALTER_TABLE_ADD_EX,
	SELECT_ROW_WITH_LITERALS,
	CREATE_PROC,
	DROP_PROC,
	EXEC_PROC,
	DROP_TABLE,
	SELECT_NO_TABLE,
	INSERT_NO_TABLE,
	SELECT_INVALIDGROUPBY
};

#endif 	//_ICMDTEXT_H_
