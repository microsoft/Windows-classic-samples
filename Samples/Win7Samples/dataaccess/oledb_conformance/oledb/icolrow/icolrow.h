//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICOLROW.H | Header file for IColumnsRowset test module.
//
// @rev 01 | 09-06-95 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _ICOLROW_H_
#define _ICOLROW_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		//include private library, which includes

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define PPI (IUnknown **)
#define TABLE_RESTRICT		0x3		// Restriction bit set for TableName restriction
#define RESTRICTION_COUNT	4		// Count of restrictions
#define TABLES_COLS			3		// Count of columns needed from Tables rowset
const WCHAR wszNULLERROR[]=L"This field should never return null but it did\n";

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum PREPARATION {SUPPORTED, NOTSUPPORTED, PREP_UNKNOWN};

enum OPTCOLUMNS
{
	ALLDBID,
	SOMEDBID,
	REVERSEDBID,
	DUPLICATEDBID,
	INVALIDDBID,
	NONEDBID
};

enum ETXN		{ETXN_COMMIT, ETXN_ABORT};
enum EMETHOD	{EMETHOD_AVAILCOL, EMETHOD_COLROWSET};

enum PROPVALS
{
	BADCOLID,
	BADTYPE,
	BADVALUE,
	VALIDROWSET
};

enum PROPOPTION
{
	BADOPTION,
	ISOPTIONAL,
	REQUIRED
};

enum FEATURE
{
	CATALOGNAME,
	SCHEMANAME
};

// Structure for checking IColumnsRowset's ColumnsInfo
struct COLROWINFO
{
	// These are meant to be filled in by derived classes:
	ULONG		 ulOrdinal;		// Ordinal of the Column.
	const DBID * columnid;		// Column ID.
	WCHAR		 pwszName[100];	// Column Name.
	ULONG		 ulColumnSize;	// Size of the Column.
	DBTYPE		 dbtype;		// OLE DB type.
	BYTE		 bPrecision;	// Max column precision.
	BYTE		 bScale;		// Max column scale.
	DWORD		 dwFlags;		// DBColumn_Flags.
};

#define MANCOL 11	// Number of mandatory columns
#ifdef _WIN64
	#define DBPRECISION	20
#else
	#define DBPRECISION	10
#endif

// This represents the IColumnsRowset column info
COLROWINFO g_rgColRowInfo[] =
{
	//Ord Guid							Name							Size					Type		Precision	Scale	Flags	
	// Bookmark
	{  0, &DB_NULLID,					L"",							sizeof(ULONG),			DBTYPE_UI4,		 10,	255,	81,},

	// Mandatory columns
	{  1, &DBCOLUMN_IDNAME,				L"DBCOLUMN_IDNAME",				128,					DBTYPE_WSTR,	255,	255,   112,},
	{  2, &DBCOLUMN_GUID,				L"DBCOLUMN_GUID",				sizeof(GUID),			DBTYPE_GUID,	255,	255,   112,},
	{  3, &DBCOLUMN_PROPID,				L"DBCOLUMN_PROPID",				sizeof(ULONG),			DBTYPE_UI4,		 10,	255,   112,},
	{  4, &DBCOLUMN_NAME,				L"DBCOLUMN_NAME",				128,					DBTYPE_WSTR,	255,	255,   112,},
	{  5, &DBCOLUMN_NUMBER,				L"DBCOLUMN_NUMBER",				sizeof(ULONG_PTR),		DBTYPEFOR_DBORDINAL, DBPRECISION,	255,	80,},
	{  6, &DBCOLUMN_TYPE,				L"DBCOLUMN_TYPE",				sizeof(USHORT),			DBTYPE_UI2,		  5,	255,	80,},
	{  7, &DBCOLUMN_TYPEINFO,			L"DBCOLUMN_TYPEINFO",			sizeof(IUnknown),		DBTYPE_IUNKNOWN,255,	255,   112,},
	{  8, &DBCOLUMN_COLUMNSIZE,			L"DBCOLUMN_COLUMNSIZE",			sizeof(ULONG_PTR),		DBTYPEFOR_DBLENGTH,	 DBPRECISION,	255,	80,},
	{  9, &DBCOLUMN_PRECISION,			L"DBCOLUMN_PRECISION",			sizeof(USHORT),			DBTYPE_UI2,		  5,	255,   112,},
	{ 10, &DBCOLUMN_SCALE,				L"DBCOLUMN_SCALE",				sizeof(SHORT),			DBTYPE_I2,		  5,	255,   112,},
	{ 11, &DBCOLUMN_FLAGS,				L"DBCOLUMN_FLAGS",				sizeof(ULONG),			DBTYPE_UI4,		 10,	255,	80,},

	// Optional columns
	{ 12, &DBCOLUMN_BASECATALOGNAME,	L"DBCOLUMN_BASECATALOGNAME",	128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 13, &DBCOLUMN_BASECOLUMNNAME,		L"DBCOLUMN_BASECOLUMNNAME",		128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 14, &DBCOLUMN_BASESCHEMANAME,		L"DBCOLUMN_BASESCHEMANAME",		128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 15, &DBCOLUMN_BASETABLENAME,		L"DBCOLUMN_BASETABLENAME",		128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 16, &DBCOLUMN_CLSID,				L"DBCOLUMN_CLSID",				sizeof(GUID),			DBTYPE_GUID,	255,	255,   112,},
	{ 17, &DBCOLUMN_COLLATINGSEQUENCE,	L"DBCOLUMN_COLLATINGSEQUENCE",	sizeof(LONG),			DBTYPE_I4,		 10,	255,   112,},
	{ 18, &DBCOLUMN_COMPUTEMODE,		L"DBCOLUMN_COMPUTEMODE",		sizeof(LONG),			DBTYPE_I4,		 10,	255,   112,},
	{ 19, &DBCOLUMN_DATETIMEPRECISION,	L"DBCOLUMN_DATETIMEPRECISION",	sizeof(LONG),			DBTYPE_UI4,		 10,	255,   112,},
	{ 20, &DBCOLUMN_DEFAULTVALUE,		L"DBCOLUMN_DEFAULTVALUE",		sizeof(VARIANT),		DBTYPE_VARIANT,	255,	255,   112,},
	{ 21, &DBCOLUMN_DOMAINCATALOG,		L"DBCOLUMN_DOMAINCATALOG",		128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 22, &DBCOLUMN_DOMAINSCHEMA,		L"DBCOLUMN_DOMAINSCHEMA",		128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 23, &DBCOLUMN_DOMAINNAME,			L"DBCOLUMN_DOMAINNAME",			128,					DBTYPE_WSTR,	255,	255,   112,},
	{ 24, &DBCOLUMN_HASDEFAULT,			L"DBCOLUMN_HASDEFAULT",			sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,   112,},
	{ 25, &DBCOLUMN_ISAUTOINCREMENT,	L"DBCOLUMN_ISAUTOINCREMENT",	sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,    16,},
	{ 26, &DBCOLUMN_ISCASESENSITIVE,	L"DBCOLUMN_ISCASESENSITIVE",	sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,   112,},
	{ 27, &DBCOLUMN_ISSEARCHABLE,		L"DBCOLUMN_ISSEARCHABLE",		sizeof(ULONG),			DBTYPE_UI4,		 10,	255,   112,},
	{ 28, &DBCOLUMN_ISUNIQUE,			L"DBCOLUMN_ISUNIQUE",			sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,   112,},
	{ 29, &DBCOLUMN_MAYSORT,			L"DBCOLUMN_MAYSORT",			sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,    16,},
	{ 30, &DBCOLUMN_OCTETLENGTH,		L"DBCOLUMN_OCTETLENGTH",		sizeof(ULONG_PTR),		DBTYPEFOR_DBLENGTH,	 DBPRECISION,	255,   112,},
	{ 31, &DBCOLUMN_KEYCOLUMN,			L"DBCOLUMN_KEYCOLUMN",			sizeof(VARIANT_BOOL),	DBTYPE_BOOL,	255,	255,    16,},
	{ 32, &DBCOLUMN_BASETABLEVERSION,	L"DBCOLUMN_BASETABLEVERSION",	sizeof(ULARGE_INTEGER),	DBTYPE_UI8,		20,		255,    16,}
};																			


//-----------------------------------------------------------------------------
// Mandatory Rowset Object interfaces
//-----------------------------------------------------------------------------

const IID * g_rgIIDRowset[]=
{
	&IID_IUnknown,
	&IID_IAccessor,
	&IID_IRowsetInfo,
	&IID_IColumnsInfo,
	&IID_IConvertType,
	&IID_IRowset
};

const WCHAR * g_rgwszRowset[]=
{
	L"IID_IUnknown",
	L"IID_IAccessor",
	L"IID_IRowsetInfo",
	L"IID_IColumnsInfo",
	L"IID_IConvertType",
	L"IID_IRowset",
};

#define ROWSET NUMELEM(g_rgIIDRowset)

// The following structure and #defines are for dev bug 2259
#define SQL_COLUMN_COUNT                0
#define SQL_COLUMN_NAME                 1
#define SQL_COLUMN_TYPE                 2
#define SQL_COLUMN_LENGTH               3
#define SQL_COLUMN_PRECISION            4
#define SQL_COLUMN_SCALE                5
#define SQL_COLUMN_DISPLAY_SIZE         6
#define SQL_COLUMN_NULLABLE             7
#define SQL_COLUMN_UNSIGNED             8
#define SQL_COLUMN_MONEY                9
#define SQL_COLUMN_UPDATABLE            10
#define SQL_COLUMN_AUTO_INCREMENT       11
#define SQL_COLUMN_CASE_SENSITIVE       12
#define SQL_COLUMN_SEARCHABLE           13
#define SQL_COLUMN_TYPE_NAME            14
#define COLATTRIB_MAX 15

ULONG g_rgCOLATTRIB[COLATTRIB_MAX]=
{
	SQL_COLUMN_COUNT,
	SQL_COLUMN_NAME,
	SQL_COLUMN_TYPE,
	SQL_COLUMN_LENGTH,
	SQL_COLUMN_PRECISION,
	SQL_COLUMN_SCALE,
	SQL_COLUMN_DISPLAY_SIZE,
	SQL_COLUMN_NULLABLE,
	SQL_COLUMN_UNSIGNED,
	SQL_COLUMN_MONEY,
	SQL_COLUMN_UPDATABLE,
	SQL_COLUMN_AUTO_INCREMENT,
	SQL_COLUMN_CASE_SENSITIVE,
	SQL_COLUMN_SEARCHABLE,
	SQL_COLUMN_TYPE_NAME
};
		
// from odbcss.h
#define SQL_CA_SS_BASE				1200
#define SQL_CA_SS_COLUMN_SSTYPE		(SQL_CA_SS_BASE+0)	//	dbcoltype/dbaltcoltype
#define SQL_CA_SS_COLUMN_UTYPE		(SQL_CA_SS_BASE+1)	//	dbcolutype/dbaltcolutype
#define SQL_CA_SS_NUM_ORDERS		(SQL_CA_SS_BASE+2)	//	dbnumorders
#define SQL_CA_SS_COLUMN_ORDER		(SQL_CA_SS_BASE+3)	//	dbcolorder
#define SQL_CA_SS_COLUMN_VARYLEN	(SQL_CA_SS_BASE+4)	//	dbvarylen
#define SQL_CA_SS_NUM_COMPUTES		(SQL_CA_SS_BASE+5)	//	dbnumcompute
#define SQL_CA_SS_COMPUTE_ID		(SQL_CA_SS_BASE+6)	//	dbnextrow status return
#define SQL_CA_SS_COMPUTE_BYLIST	(SQL_CA_SS_BASE+7)	//	dbbylist
#define SQL_CA_SS_COLUMN_ID			(SQL_CA_SS_BASE+8)	//	dbaltcolid
#define SQL_CA_SS_COLUMN_OP			(SQL_CA_SS_BASE+9)	//	dbaltcolop
#define SQL_CA_SS_COLUMN_SIZE		(SQL_CA_SS_BASE+10)	//	dbcollen
#define SQL_CA_SS_COLUMN_HIDDEN		(SQL_CA_SS_BASE+11) //	Column is hidden (FOR BROWSE)
#define SQL_CA_SS_COLUMN_KEY		(SQL_CA_SS_BASE+12) //	Column is key column (FOR BROWSE)

// necessary for dev bug 2358, for browse clause
const ULONG MAX_SQL_CA_SS= 10;
const ULONG g_rgCOLATTRIB_SS[MAX_SQL_CA_SS]=
{
	SQL_CA_SS_COLUMN_SSTYPE,	//	dbcoltype/dbaltcoltype
	SQL_CA_SS_COLUMN_UTYPE,		//	dbcolutype/dbaltcolutype
	SQL_CA_SS_NUM_ORDERS,		//	dbnumorders
	SQL_CA_SS_COLUMN_ORDER,		//	dbcolorder
	SQL_CA_SS_COLUMN_VARYLEN,	//	dbvarylen
	SQL_CA_SS_NUM_COMPUTES,		//	dbnumcompute
	SQL_CA_SS_COMPUTE_ID,		//	dbnextrow status return
	SQL_CA_SS_COLUMN_SIZE,		//	dbcollen
	SQL_CA_SS_COLUMN_HIDDEN,	//	Column is hidden (FOR BROWSE)
	SQL_CA_SS_COLUMN_KEY		//	Column is key column (FOR BROWSE)
};

// necessary for dev bug 2358, compute by clause
const ULONG MAX_SQL_CA_SS_COMPUTE= 3;
const ULONG g_rgCOLATTRIB_SS_COMPUTE[MAX_SQL_CA_SS_COMPUTE]=
{
	SQL_CA_SS_COMPUTE_BYLIST,	//	dbbylist
	SQL_CA_SS_COLUMN_ID,		//	dbaltcolid
	SQL_CA_SS_COLUMN_OP,		//	dbaltcolop
};

enum STATUS_ENUM
{
	NO_NULLS_ALLOWED,
	ALLOW_NULLS
};

enum AGGREGATION
{
	NONE,
	NOAGGREGATION,
	AGGREGATE
};

#endif 	//_ICOLROW_H_


