//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module IALTERINDEX.H | Header file for IAlterIndex test module.
//
// @rev 01 | 08-03-99 | Microsoft | Created
//

#ifndef _IALTERINDEX_H_
#define _IALTERINDEX_H_


//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library

///////////////////////////////////////////////////////////////////////
//Defines
//
///////////////////////////////////////////////////////////////////////

#define	Refresh_m_pIndexID(hr, dbidNew)			\
	if(SUCCEEDED(hr))						\
	{										\
		ReleaseDBID(m_pIndexID, FALSE);			\
		DuplicateDBID(dbidNew, m_pIndexID);	\
	}

// Open Index Support
#define OIS_NONE		0	// No open index support
#define	OIS_INTEGRATED	1	// Integrated open index support
#define OIS_ROWSET		2	// Separate open index support

#define MAX_INDEX_COLS	2	// Default for max cols in index.  3 takes too long
#define MAX_INDEX_COUNT	3	// Default for max valid indexes to test
#define EXPAND(x)		x, L#x
#define MIN_TABLE_ROWS	10	// Need at least 10 rows in table
#define SAFE_RELEASE_ACCESSOR(pIAcc, hAcc) {if ((pIAcc) && (hAcc) && \
	CHECK((pIAcc)->ReleaseAccessor((hAcc), NULL), S_OK)) (hAcc) = DB_NULL_HACCESSOR;}
#define THREAD_ARG6 ((THREADARG6*)pv)->pArg6
#define TABLE_RESTRICT		0x3		// Restriction bit set for TableName restriction
#define RESTRICTION_COUNT	4		// Count of restrictions
#define TABLES_COLS			3		// Count of columns needed from Tables rowset

///////////////////////////////////////////////////////////////////////
//Enumerations
//
///////////////////////////////////////////////////////////////////////

//This enumeration represents various types of Indexes.

enum ETESTCASE
{
	TC_SingleColNoProps = 1,
	TC_SingleColProps,
	TC_MultipleColsNoProps,
	TC_MultipleColsProps,
	TC_PropSingCol,
	TC_PropMultCol
};

enum EINDEXSCHEMA	// Index of data in schema rowset
{
	IS_TABLE_CATALOG = 0,
	IS_TABLE_SCHEMA,
	IS_TABLE_NAME,
	IS_INDEX_CATALOG,
	IS_INDEX_SCHEMA,
	IS_INDEX_NAME,
	IS_PRIMARY_KEY,
	IS_UNIQUE,
	IS_CLUSTERED,
	IS_TYPE,
	IS_FILL_FACTOR,
	IS_INITIAL_SIZE,
	IS_NULLS,
	IS_SORT_BOOKMARKS,
	IS_AUTO_UPDATE,
	IS_NULL_COLLATION,
	IS_ORDINAL_POSITION,
	IS_COLUMN_NAME,
	IS_COLUMN_GUID,
	IS_COLUMN_PROPID,
	IS_COLLATION,
	IS_CARDINALITY,
	IS_PAGES,
	IS_FILTER_CONDITION,
	IS_INTEGRATED,
	CINDEXFIELDS		// Must remain the last enumeration
};

enum ETXN	{ETXN_COMMIT, ETXN_ABORT};

///////////////////////////////////////////////////////////////////////
//Structure definitions
//
///////////////////////////////////////////////////////////////////////

typedef struct tagPropVal
{
	ULONG	ulPropVal;
	LPWSTR	pwszPropVal;
} PropVal;

typedef struct tagIndexInfo
{
   ULONG					cKeyColumns;
   DBINDEXCOLUMNDESC *		rgIndexColumnDesc;
   ULONG 					cIndexPropSets;
   DBPROPSET *				rgIndexPropSets;
} IndexInfo;

typedef struct tagPropMap
{
	DBPROPID dwProp;
	LPWSTR	pwszPropVal;
	enum EINDEXSCHEMA eSchemaField;
} PropMap;

// Thread arg structure for one more argument than Extralib
struct THREADARG6
{
	LPVOID	pFunc;
	LPVOID	pArg1;
	LPVOID	pArg2;
	LPVOID	pArg3;
	LPVOID	pArg4;
	LPVOID	pArg5;
	LPVOID	pArg6;
};

///////////////////////////////////////////////////////////////////////
//Providers only supporting setting props to default via AlterIndex
//
//	You may need to add your provider to this list
///////////////////////////////////////////////////////////////////////
const LPWSTR ProviderSupportsDefaultOnly[] =
{
	L"MSJETOLEDB40.DLL"
};

///////////////////////////////////////////////////////////////////////
//Properties
//
///////////////////////////////////////////////////////////////////////
const PropMap IndexProperties[] =
{
	EXPAND(DBPROP_INDEX_AUTOUPDATE),	IS_AUTO_UPDATE,
	EXPAND(DBPROP_INDEX_CLUSTERED),		IS_CLUSTERED,
	EXPAND(DBPROP_INDEX_FILLFACTOR),	IS_FILL_FACTOR,
	EXPAND(DBPROP_INDEX_INITIALSIZE),	IS_INITIAL_SIZE,
	EXPAND(DBPROP_INDEX_NULLCOLLATION),	IS_NULL_COLLATION,
	EXPAND(DBPROP_INDEX_NULLS),			IS_NULLS,
	EXPAND(DBPROP_INDEX_PRIMARYKEY),	IS_PRIMARY_KEY,
	EXPAND(DBPROP_INDEX_SORTBOOKMARKS),	IS_SORT_BOOKMARKS,
	EXPAND(DBPROP_INDEX_TEMPINDEX),		CINDEXFIELDS,		// No field entry in indexes rowset
	EXPAND(DBPROP_INDEX_TYPE),			IS_TYPE,
	EXPAND(DBPROP_INDEX_UNIQUE),		IS_UNIQUE
};


///////////////////////////////////////////////////////////////////////
//Property values
//
///////////////////////////////////////////////////////////////////////
const VARIANT_BOOL VariantBoolVals[] =
{
	VARIANT_TRUE,
	VARIANT_FALSE
};

const DBINDEX_COL_ORDER IndexOrderVals[] =
{
	DBINDEX_COL_ORDER_ASC,
	DBINDEX_COL_ORDER_DESC
};

const PropVal IndexNullVals[] =
{
	EXPAND(DBPROPVAL_IN_ALLOWNULL),
	EXPAND(DBPROPVAL_IN_DISALLOWNULL),
	EXPAND(DBPROPVAL_IN_IGNORENULL),
	EXPAND(DBPROPVAL_IN_IGNOREANYNULL)
};

const PropVal IndexTypeVals[] =
{
	EXPAND(DBPROPVAL_IT_BTREE),
	EXPAND(DBPROPVAL_IT_HASH),
	EXPAND(DBPROPVAL_IT_CONTENT),
	EXPAND(DBPROPVAL_IT_OTHER)
};

const PropVal NullCollationVals[] =
{
	EXPAND(DBPROPVAL_NC_END),
	EXPAND(DBPROPVAL_NC_START),
	EXPAND(DBPROPVAL_NC_HIGH),
	EXPAND(DBPROPVAL_NC_LOW)
};

#endif 	//_IALTERINDEX_H_
