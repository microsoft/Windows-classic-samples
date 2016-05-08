//--------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// File name: TABLEDUMP.H
//
//      TableDump routines for the Test TableDump.
//
//


#ifndef _TABLEDUMP_H_
#define _TABLEDUMP_H_


////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "Common.h"


////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////
#define MAX_ARGS 100

enum ARG_VALUES
{
	//Provider
	ARG_PROVIDER = 1,

	//Data
	ARG_BINDINGTYPE,
	ARG_NODATA,
	
	//Log File
	ARG_CREATETABLE,
	ARG_CREATEINDEX,

	//Log File
	ARG_OUTPUT
};



////////////////////////////////////////////////////////
// Globals
//
////////////////////////////////////////////////////////
extern FILE*	g_fpLogFile;


////////////////////////////////////////////////////////
// Prototypes
//
////////////////////////////////////////////////////////
HRESULT DumpTable(ULONG cArgs, WCHAR* rgArgs[], ULONG cPropSets, DBPROPSET* rgPropSets, CHAR* pszCmdLine);

HRESULT GetDataSource(WCHAR* pwszProgID, ULONG cPropSets, DBPROPSET* rgPropSets, IDBInitialize** ppIDBInitialize);
HRESULT GetSession(IDBInitialize* pIDBInitialize, IOpenRowset** ppIOpenRowset);
HRESULT GetRowset(IDBInitialize* pIDBInitialize, IOpenRowset* pIOpenRowset, WCHAR* pwszTableName, WCHAR* pwszDefaultQuery, REFIID riid, IUnknown** ppIUnknown);
HRESULT GetColumnsInfo(IUnknown* pIUnkRowset, DBORDINAL* pcColumns,	DBCOLUMNINFO** ppColumnInfo, WCHAR** ppStringsBuffer, BOOL fWantBookmark = FALSE);

HRESULT SetupBindings(DBORDINAL cColumns, DBCOLUMNINFO*	rgColumnInfo, DBTYPE wBindingType, DBCOUNTITEM* pcBindings, DBBINDING** prgBindings, DBLENGTH* pulRowSize);
HRESULT CreateAccessor(IUnknown* pIUnkRowset, DBCOUNTITEM cBindings, DBBINDING* rgBindings, HACCESSOR* phAccessor);
HRESULT CleanupRowset(IRowset* pIRowset, HACCESSOR hAccessor);
    
HRESULT DumpRowset(IRowset*	pIRowset, DBTYPE wBindingType,	CTable* pCTable);
HRESULT DumpAllRows(IRowset* pIRowset, DBCOUNTITEM cBindings, DBBINDING* rgBindings, HACCESSOR hAccessor, DBTYPE wBindingType, DBLENGTH cMaxRowSize,	CTable* pCTable);

#endif //_TABLEDUMP_H_