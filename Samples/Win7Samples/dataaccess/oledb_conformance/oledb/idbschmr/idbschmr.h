//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IDBSCHMR.H | Header file for IDBSchemaRowset test module.
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 02-23-98 | Microsoft | Updated
//

#ifndef _IDBSCHMR_H_
#define _IDBSCHMR_H_

#define SQL_SCHEMAS
#ifdef SQL_SCHEMAS
	#define DBINITCONSTANTS
	#define INITGUID
#endif
// #include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library

// Disable warnings about deprecated functions until all team members have migrated to VS2005
#pragma warning(disable:4996)



//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define	GET_ROWS 20
#define MAXPROP 5
#define MAXRESTRICTION 7
#define MAX_STATISTIC_ERROR		0.10
#define MAX_MSG_BUF	1000
#define CCHMAXLENGTH_UNKNOWN	(ULONG)~0
#define MAX_PARAM_COUNT			1000
#define MAX_STATS_COLS			30

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum ETXN		{ETXN_COMMIT, ETXN_ABORT};

typedef enum tagROW_COUNT
{
	DEFAULT,		// Use the default for the schema obtained in GetSchemaInfo.
	MIN_VALUE,		// Row count must be at least some number if any rows, warning if 0.
	MIN_REQUIRED,	// Row count must be at least some number
	EXACT_VALUE		// Row count must be exactly some number
} ROW_COUNT;

typedef enum tagCARDINALITY
{
	COLUMN_CARDINALITY,
	TUPLE_CARDINALITY,
	TABLE_CARDINALITY,
	RANGE_ROWS_CARDINALITY,
	EQ_ROWS_CARDINALITY,
	DISTINCT_RANGE_ROWS_CARDINALITY
} CARDINALITY;

typedef enum tagRESTRICT_CAT
{
	RC_UND,			// Undefined restriction type (default)
	RC_CATALOG,		// Catalog name restriction
	RC_SCHEMA,		// Schema name
	RC_TABLE,		// Table name
	RC_CONSTRAINT,	// Constraint name
	RC_COLUMN,		// Column name
	RC_USER,		// User name
	RC_INDEX,		// Index name
	RC_SERVER,		// Server name
	RC_PROCEDURE,	// Procedure name
	RC_PARAMETER,	// Parameter name
	RC_CONSTRAINT_TYPE, // Constraint type name
	RC_STATISTICS,		// Statistics name
	RC_TABLE_TYPE	// Table type name
} RESTRICT_CAT;

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	wszErrorCreatingSession[] =		L"Error Creating Session, CreateDBSession FAILED\n";
const WCHAR wszVerifyInterfaceFailed[] =	L"Verify Interface FAILED\n";
const WCHAR wszGetSchemasFailed[] =			L"IDBSchemaRowset::GetSchemas FAILED\n";
const WCHAR wszGetRowsetFailed[] =			L"IDBSchemaRowset::GetRowset FAILED\n";
const WCHAR wszGetColumnInfoFailed[]=		L"IColumnsInfo::GetColumnInfo FAILED\n";
const WCHAR wszRowsetIsNull[]=				L"Rowset Is Null\n";
const WCHAR wszDropProc[]=					L"DROP PROCEDURE %s";
const WCHAR wszCreateProc[]=				L"CREATE PROCEDURE %s (@IntIn int =1, @IntOut int =2 OUTPUT) AS Select * from %s RETURN (1)";
const WCHAR wszProcDesignator[]=			L"_proc";

const WCHAR wszCATALOG[]=					L"nstl";
const WCHAR wszSCHEMA[]=					L"ODBC";
const WCHAR wszTABLETYPE[]=					L"SYSTEM TABLE";

const WCHAR wszGRANTOR[]=					L"ODBC";
const WCHAR wszGRANTEE[]=					L"ODBC2";

const WCHAR	wszTABLETYPE_ALIAS[]			=L"ALIAS";
const WCHAR	wszTABLETYPE_BASETABLE[]		=L"TABLE";
const WCHAR	wszTABLETYPE_SYNONYM[]			=L"SYNONYM";
const WCHAR	wszTABLETYPE_SYSTEMTABLE[]		=L"SYSTEM TABLE";
const WCHAR	wszTABLETYPE_VIEW[]				=L"VIEW";
const WCHAR	wszTABLETYPE_GLOBALTEMPORARY[]	=L"GLOBAL TEMPORARY";
const WCHAR	wszTABLETYPE_LOCALTEMPORARY[]	=L"LOCAL TEMPORARY";

const WCHAR wszCONSTRAINT_TYPE[]			=L"UNIQUE";
const WCHAR wszCREATESTORPROC[]				=L"Create procedure STORPROC as select * from %s";	
const WCHAR wszSTORPROC[]					=L"STORPROC";

const WCHAR wszPUBS[]						=L"Pubs";
const WCHAR wszNWIND[]						=L"NWIND";

const WCHAR wszDBSCHEMA_ASSERTIONS[]		=L"DBSCHEMA_ASSERTIONS";
const WCHAR wszDBSCHEMA_CATALOGS[]			=L"DBSCHEMA_CATALOGS";
const WCHAR wszDBSCHEMA_CHARACTER_SETS[]	=L"DBSCHEMA_CHARACTER_SETS";
const WCHAR wszDBSCHEMA_CHECK_CONSTRAINTS[]	=L"DBSCHEMA_CHECK_CONSTRAINTS";
const WCHAR wszDBSCHEMA_COLLATIONS[]		=L"DBSCHEMA_COLLATIONS";
const WCHAR wszDBSCHEMA_COLUMN_DOMAIN_USAGE[]=L"DBSCHEMA_COLUMN_DOMAIN_USAGE";
const WCHAR wszDBSCHEMA_COLUMN_PRIVILEGES[]	=L"DBSCHEMA_COLUMN_PRIVILEGES";
const WCHAR wszDBSCHEMA_COLUMNS[]			=L"DBSCHEMA_COLUMNS";
const WCHAR wszDBSCHEMA_CONSTRAINT_COLUMN_USAGE[]=L"DBSCHEMA_CONSTRAINT_COLUMN_USAGE";
const WCHAR wszDBSCHEMA_CONSTRAINT_TABLE_USAGE[]=L"DBSCHEMA_CONSTRAINT_TABLE_USAGE";
const WCHAR wszDBSCHEMA_FOREIGN_KEYS[]		=L"DBSCHEMA_FOREIGN_KEYS";
const WCHAR wszDBSCHEMA_INDEXES[]			=L"DBSCHEMA_INDEXES";
const WCHAR wszDBSCHEMA_KEY_COLUMN_USAGE[]	=L"DBSCHEMA_KEY_COLUMN_USAGE";
const WCHAR wszDBSCHEMA_PRIMARY_KEYS[]		=L"DBSCHEMA_PRIMARY_KEYS";
const WCHAR wszDBSCHEMA_PROCEDURE_COLUMNS[]	=L"DBSCHEMA_PROCEDURE_COLUMNS";
const WCHAR wszDBSCHEMA_PROCEDURE_PARAMETERS[]=L"DBSCHEMA_PROCEDURE_PARAMETERS";
const WCHAR wszDBSCHEMA_PROCEDURES[]		=L"DBSCHEMA_PROCEDURES";
const WCHAR wszDBSCHEMA_PROVIDER_TYPES[]	=L"DBSCHEMA_PROVIDER_TYPES";
const WCHAR wszDBSCHEMA_REFERENTIAL_CONSTRAINTS[]=L"DBSCHEMA_REFERENTIAL_CONSTRAINTS";
const WCHAR wszDBSCHEMA_SCHEMATA[]			=L"DBSCHEMA_SCHEMATA";
const WCHAR wszDBSCHEMA_SQL_LANGUAGES[]		=L"DBSCHEMA_SQL_LANGUAGES";
const WCHAR wszDBSCHEMA_STATISTICS[]		=L"DBSCHEMA_STATISTICS";
const WCHAR wszDBSCHEMA_TABLES[]			=L"DBSCHEMA_TABLES";
const WCHAR wszDBSCHEMA_TABLES_INFO[]		=L"DBSCHEMA_TABLES_INFO";
const WCHAR wszDBSCHEMA_TABLE_CONSTRAINTS[]	=L"DBSCHEMA_TABLE_CONSTRAINTS";
const WCHAR wszDBSCHEMA_TABLE_PRIVILEGES[]	=L"DBSCHEMA_TABLE_PRIVILEGES";
const WCHAR wszDBSCHEMA_TABLE_STATISTICS[]	=L"DBSCHEMA_TABLE_STATISTICS";
const WCHAR wszDBSCHEMA_TRANSLATIONS[]		=L"DBSCHEMA_TRANSLATIONS";
const WCHAR wszDBSCHEMA_TRUSTEE[]			=L"DBSCHEMA_TRUSTEE";
const WCHAR wszDBSCHEMA_USAGE_PRIVILEGES[]	=L"DBSCHEMA_USAGE_PRIVILEGES";
const WCHAR wszDBSCHEMA_VIEW_COLUMN_USAGE[]	=L"DBSCHEMA_VIEW_COLUMN_USAGE";
const WCHAR wszDBSCHEMA_VIEW_TABLE_USAGE[]	=L"DBSCHEMA_VIEW_TABLE_USAGE";
const WCHAR wszDBSCHEMA_VIEWS[]				=L"DBSCHEMA_VIEWS";
const WCHAR wszDBSCHEMA_GUID[]				=L"DBSCHEMA_GUID not recognized or not supported";
const WCHAR wszNOROWS[]						=L"No rows returned";

const WCHAR wszABORTPRESERVE[]				=L"ABORTPRESERVE";
const WCHAR wszACCESSORDER[]				=L"ACCESSORDER";
const WCHAR wszAPPENDONLY[]					=L"APPENDONLY";
const WCHAR wszBLOCKINGSTORAGEOBJECTS[]		=L"BLOCKINGSTORAGEOBJECTS";
const WCHAR wszBOOKMARKINFO[]				=L"BOOKMARKINFO";
const WCHAR wszBOOKMARKS[]					=L"BOOKMARKS";
const WCHAR wszBOOKMARKSKIPPED[]			=L"BOOKMARKSKIPPED";
const WCHAR wszBOOKMARKTYPE[]				=L"BOOKMARKTYPE";
const WCHAR wszCACHEDEFERRED[]				=L"CACHEDEFERRED";
const WCHAR wszCANFETCHBACKWARDS[]			=L"CANFETCHBACKWARDS";
const WCHAR wszCANHOLDROWS[]				=L"CANHOLDROWS";
const WCHAR wszCANRELEASELOCKS[]			=L"CANRELEASELOCKS";
const WCHAR wszCANSCROLLBACKWARDS[]			=L"CANSCROLLBACKWARDS";
const WCHAR wszCHANGEINSERTEDROWS[]			=L"CHANGEINSERTEDROWS";
const WCHAR wszCOLUMNRESTRICT[]				=L"COLUMNRESTRICT";
const WCHAR wszCOMMANDTIMEOUT[]				=L"COMMANDTIMEOUT";
const WCHAR wszCOMMITPRESERVE[]				=L"COMMITPRESERVE";
const WCHAR wszDEFERRED[]					=L"DEFERRED";
const WCHAR wszDELAYSTORAGEOBJECTS[]		=L"DELAYSTORAGEOBJECTS";
const WCHAR wszFILTERCOMPAREOPS[]			=L"FILTERCOMPAREOPS";
const WCHAR wszFINDCOMPAREOPS[]				=L"FINDCOMPAREOPS";
const WCHAR wszHIDDENCOLUMNS[]				=L"HIDDENCOLUMNS";
const WCHAR wszIMMOBILEROWS[]				=L"IMMOBILEROWS";
const WCHAR wszLITERALBOOKMARKS[]			=L"LITERALBOOKMARKS";
const WCHAR wszLITERALIDENTITY[]			=L"LITERALIDENTITY";
const WCHAR wszLOCKMODE[]					=L"LOCKMODE";
const WCHAR wszMAXOPENROWS[]				=L"MAXOPENROWS";
const WCHAR wszMAXPENDINGROWS[]				=L"MAXPENDINGROWS";
const WCHAR wszMAXROWS[]					=L"MAXROWS";
const WCHAR wszMAYWRITECOLUMN[]				=L"MAYWRITECOLUMN";
const WCHAR wszMEMORYUSAGE[]				=L"MEMORYUSAGE";
const WCHAR wszNOTIFICATIONGRANULARITY[]	=L"NOTIFICATIONGRANULARITY";
const WCHAR wszNOTIFICATIONPHASES[]			=L"NOTIFICATIONPHASES";
const WCHAR wszNOTIFYCOLUMNSET[]			=L"NOTIFYCOLUMNSET";
const WCHAR wszNOTIFYROWDELETE[]			=L"NOTIFYROWDELETE";
const WCHAR wszNOTIFYROWFIRSTCHANGE[]		=L"NOTIFYROWFIRSTCHANGE";
const WCHAR wszNOTIFYROWINSERT[]			=L"NOTIFYROWINSERT";
const WCHAR wszNOTIFYROWRESYNCH[]			=L"NOTIFYROWRESYNCH";
const WCHAR wszNOTIFYROWSETRELEASE[]		=L"NOTIFYROWSETRELEASE";
const WCHAR wszNOTIFYROWSETFETCHPOSITIONCHANGE[]=L"NOTIFYROWSETFETCHPOSITIONCHANGE";
const WCHAR wszNOTIFYROWUNDOCHANGE[]		=L"NOTIFYROWUNDOCHANGE";
const WCHAR wszNOTIFYROWUNDODELETE[]		=L"NOTIFYROWUNDODELETE";
const WCHAR wszNOTIFYROWUNDOINSERT[]		=L"NOTIFYROWUNDOINSERT";
const WCHAR wszNOTIFYROWUPDATE[]			=L"NOTIFYROWUPDATE";
const WCHAR wszORDEREDBOOKMARKS[]			=L"ORDEREDBOOKMARKS";
const WCHAR wszOTHERINSERT []				=L"OTHERINSERT";
const WCHAR wszOTHERUPDATEDELETE[]			=L"OTHERUPDATEDELETE";
const WCHAR wszOWNINSERT[]					=L"OWNINSERT";
const WCHAR wszOWNUPDATEDELETE[]			=L"OWNUPDATEDELETE";
const WCHAR wszPROPERTIESINERROR[]			=L"PROPERTIESINERROR";
const WCHAR wszQUICKRESTART[]				=L"QUICKRESTART";
const WCHAR wszREENTRANTEVENTS[]			=L"REENTRANTEVENTS";
const WCHAR wszREMOVEDELETED[]				=L"REMOVEDELETED";
const WCHAR wszREPORTMULTIPLECHANGES[]		=L"REPORTMULTIPLECHANGES";
const WCHAR wszRETURNPENDINGINSERTS[]		=L"RETURNPENDINGINSERTS";
const WCHAR wszROWRESTRICT[]				=L"ROWRESTRICT";
const WCHAR wszROWSET_ASYNCH[]				=L"ROWSET_ASYNCH";
const WCHAR wszROWTHREADMODEL[]				=L"ROWTHREADMODEL";
const WCHAR wszSERVERCURSOR[]				=L"SERVERCURSOR";
const WCHAR wszSERVERDATAONINSERT[]			=L"SERVERDATAONINSERT";
const WCHAR wszTRANSACTEDOBJECT[]			=L"TRANSACTEDOBJECT";
const WCHAR wszUPDATABILITY[]				=L"UPDATABILITY";
const WCHAR wszUNIQUEROWS[]					=L"UNIQUEROWS";
const WCHAR wszSTRONGIDENTITY[]				=L"STRONGIDENTITY";
const WCHAR wszIAccessor[]					=L"IAccessor";
const WCHAR wszIChapteredRowset[]			=L"IChapteredRowset";
const WCHAR wszIColumnsInfo[]				=L"IColumnsInfo";
const WCHAR wszIColumnsRowset[]				=L"IColumnsRowset";
const WCHAR wszIConnectionPointContainer[]	=L"IConnectionPointContainer";
const WCHAR wszIConvertType[]				=L"IConvertType";
const WCHAR wszIDBAsynchStatus[]			=L"IDBAsynchStatus";
const WCHAR wszILockBytes[]					=L"ILockBytes";
const WCHAR wszIMultipleResults[]			=L"IMultipleResults";
const WCHAR wszIParentRowset[]				=L"IParentRowset";
const WCHAR wszIRowset[]					=L"IRowset";
const WCHAR wszIRowsetChange[]				=L"IRowsetChange";
const WCHAR wszIRowsetCurrentIndex[]		=L"IRowsetCurrentIndex";
// const WCHAR wszIRowsetExactScroll[]			=L"IRowsetExactScroll";
const WCHAR wszIRowsetFind[]				=L"IRowsetFind";
const WCHAR wszIRowsetIdentity[]			=L"IRowsetIdentity";
const WCHAR wszIRowsetIndex[]				=L"IRowsetIndex";
const WCHAR wszIRowsetInfo[]				=L"IRowsetInfo";
const WCHAR wszIRowsetLocate[]				=L"IRowsetLocate";
const WCHAR wszIRowsetRefresh[]				=L"IRowsetRefresh";
const WCHAR wszIRowsetResynch[]				=L"IRowsetResynch";
const WCHAR wszIRowsetScroll[]				=L"IRowsetScroll";
const WCHAR wszIRowsetUpdate[]				=L"IRowsetUpdate";
const WCHAR wszIRowsetView[]				=L"IRowsetView";
const WCHAR wszISupportErrorInfo[]			=L"ISupportErrorInfo";
const WCHAR wszISequentialStream[]			=L"ISequentialStream";
const WCHAR wszIStorage[]					=L"IStorage";
const WCHAR wszIStream[]					=L"IStream";
const WCHAR wszROWSETPROP[]					=L"Rowset Property NOT recognized";
const WCHAR wszRESTRICTION[]				=L"Restriction not known, so can't test\n";
const WCHAR wszRESTRICTIONNOTSUPPORTED[]	=L"This schema/restriction is not supported\n";

// Alter table strings for Primary, Foriegnkeys.
const WCHAR g_wszAddPrimaryKey[]			=L"ALTER TABLE %s ADD CONSTRAINT pk%d%s PRIMARY KEY (%s)";
const WCHAR g_wszAddPrimaryKey2[]			=L"ALTER TABLE %s ADD CONSTRAINT pk%d%s PRIMARY KEY (%s, %s)";
const WCHAR g_wszAddForeignKey[]			=L"ALTER TABLE %s ADD CONSTRAINT fk%d%s FOREIGN KEY (%s) REFERENCES %s (%s)";
const WCHAR g_wszAddForeignKey2[]			=L"ALTER TABLE %s ADD CONSTRAINT fk%d%s FOREIGN KEY (%s, %s) REFERENCES %s (%s, %s)";
const WCHAR g_wszDropPrimaryKeyConstraint[]	=L"ALTER TABLE %s DROP CONSTRAINT pk%d%s";
const WCHAR g_wszDropForeignKeyConstraint[]	=L"ALTER TABLE %s DROP CONSTRAINT fk%d%s";

typedef DWORD RESTRICTIONS;

enum RESTRICTIONSENUM
{
	ZERO	= 0x0,
	FIRST	= 0x1,
	SECOND	= 0x2,
	THIRD	= 0x4,
	FOURTH	= 0x8,
	FIFTH	= 0x10,
	SIXTH	= 0x20,
	SEVENTH = 0x40,
	ALLRES	= 0x80

};

typedef DWORD SUPPORTEDRESTRICTIONS;

enum SUPPORTEDRESTRICTIONSENUM
{
	one =	0x1,
	two =	0x2,
	three = 0x4,
	four =	0x8,
	five =	0x10,
	six =	0x20,
	seven = 0x40

};

enum REQUESTED_SCHEMA
{
	SUPPORTED,		// only a supported schema
	SPECIFIC		// schema, supported or not, that I am requesting
};

struct SUP_RESTRICT
{
	GUID schema;
	ULONG bitmask;
};

#define TYPE_I1 signed char
#define TYPE_I2 SHORT
#define TYPE_I4 LONG
#define TYPE_I8 LARGE_INTEGER
#define TYPE_UI1 BYTE
#define TYPE_UI2 unsigned short
#define TYPE_UI4 unsigned int
#define TYPE_UI8 ULARGE_INTEGER
#define TYPE_R4 float
#define TYPE_R8 double
#define TYPE_CY LARGE_INTEGER
#define TYPE_DECIMAL DBDECIMAL
#define TYPE_NUMERIC DBNUMERIC
#define TYPE_DATE DATE
#define TYPE_BOOL VARIANT_BOOL
#define TYPE_BYTES BYTE *
#define TYPE_BSTR BSTR
#define TYPE_STR char *
#define TYPE_WSTR wchar_t *
#define TYPE_VARIANT VARIANT
#define TYPE_IDISPATCH Dispatch *
#define TYPE_IUNKNOWN IUnknown *
#define TYPE_GUID GUID
#define TYPE_ERROR SCODE
#define TYPE_BYREF void *
#define TYPE_ARRAY SAFEARRAY *
#define TYPE_VECTOR DBVECTOR
#define TYPE_DBDATE DBDATE
#define TYPE_DBTIME DBTIME
#define TYPE_DBTIMESTAMP DBTIMESTAMP

//-----------------------------------------------------------------------------
// MANDATORY SCHEMA
//-----------------------------------------------------------------------------
#define cMANDATORY_SCHEMA 3
#define cSCHEMA 31
//-----------------------------------------------------------------------------
// ROWSET RIIDs, want non-mandatory
//-----------------------------------------------------------------------------

#define cROWSET_RIID  18

const WCHAR	*	rgwszRIID[]={
				L"IID_IAccessor",
				L"IID_IColumnsInfo",
				L"IID_IColumnsRowset",
				L"IID_IConnectionPointContainer",
				L"IID_IConvertType",
				L"IID_IRowset",
				L"IID_IRowsetChange",
				L"IID_IRowsetIdentity",
				L"IID_IRowsetInfo",
				L"IID_IRowsetLocate",
				L"IID_IRowsetResynch",
				L"IID_IRowsetScroll",
				L"IID_IRowsetUpdate",
				L"IID_ISupportErrorInfo",
				L"IID_IChapteredRowset",
				L"IID_IDBAsynchStatus",
				L"IID_IRowsetFind",
				L"IID_IRowsetView"

};

const IID *	rgRowsetIID[]={
				&IID_IAccessor,
				&IID_IColumnsInfo,
				&IID_IColumnsRowset,
				&IID_IConnectionPointContainer,
				&IID_IConvertType,
				&IID_IRowset,
				&IID_IRowsetChange,
				&IID_IRowsetIdentity,
				&IID_IRowsetInfo,
				&IID_IRowsetLocate,
				&IID_IRowsetResynch,
				&IID_IRowsetScroll,
				&IID_IRowsetUpdate,
				&IID_ISupportErrorInfo,
				&IID_IChapteredRowset,
				&IID_IDBAsynchStatus,
				&IID_IRowsetFind,
				&IID_IRowsetView
};
const DBPROPID	rgRowsetDBPROP_IID[]={
				DBPROP_IAccessor,
				DBPROP_IColumnsInfo,
				DBPROP_IColumnsRowset,
				DBPROP_IConnectionPointContainer,
				DBPROP_IConvertType,
				DBPROP_IRowset,
				DBPROP_IRowsetChange,
				DBPROP_IRowsetIdentity,
				DBPROP_IRowsetInfo,
				DBPROP_IRowsetLocate,
				DBPROP_IRowsetResynch,
				DBPROP_IRowsetScroll,
				DBPROP_IRowsetUpdate,
				DBPROP_ISupportErrorInfo,
				DBPROP_IChapteredRowset,
				DBPROP_IDBAsynchStatus,
				DBPROP_IRowsetFind,
				DBPROP_IRowsetView
};


//-----------------------------------------------------------------------------
// COLUMNS
//-----------------------------------------------------------------------------
#define cCOLUMNS 28
#define cCOLUMNS_RESTRICTIONS 4
const DBTYPE	rgtypeCOLUMNS[cCOLUMNS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_BOOL,
				DBTYPE_WSTR,
				DBTYPE_UI4,
				DBTYPE_BOOL,
				DBTYPE_UI2,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_UI2,
				DBTYPE_I2,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
const WCHAR	*	rgwszCOLUMNS[cCOLUMNS]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"ORDINAL_POSITION",
				L"COLUMN_HASDEFAULT",
				L"COLUMN_DEFAULT",
				L"COLUMN_FLAGS",
				L"IS_NULLABLE",
				L"DATA_TYPE",
				L"TYPE_GUID",
				L"CHARACTER_MAXIMUM_LENGTH",
				L"CHARACTER_OCTET_LENGTH",
				L"NUMERIC_PRECISION",
				L"NUMERIC_SCALE",
				L"DATETIME_PRECISION",
				L"CHARACTER_SET_CATALOG",
				L"CHARACTER_SET_SCHEMA",
				L"CHARACTER_SET_NAME",
				L"COLLATION_CATALOG",
				L"COLLATION_SCHEMA",
				L"COLLATION_NAME",
				L"DOMAIN_CATALOG",
				L"DOMAIN_SCHEMA",
				L"DOMAIN_NAME",
				L"DESCRIPTION"
};
//-----------------------------------------------------------------------------
// TABLES
//-----------------------------------------------------------------------------
#define cTABLES 9 
#define cTABLES_RESTRICTIONS 4
const WCHAR	*	rgwszTABLES[cTABLES]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"TABLE_TYPE",
				L"TABLE_GUID",
				L"DESCRIPTION",
				L"TABLE_PROPID",
				L"DATE_CREATED",
				L"DATE_MODIFIED"
};
const DBTYPE	rgtypeTABLES[cTABLES]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_WSTR,
				DBTYPE_UI4,
				DBTYPE_DATE,
				DBTYPE_DATE
};
//-----------------------------------------------------------------------------
// PROVIDER_TYPES
//-----------------------------------------------------------------------------
#define cPROVIDER_TYPES 21
#define cPROVIDER_TYPES_RESTRICTIONS 2
const WCHAR	*	rgwszPROVIDER_TYPES[cPROVIDER_TYPES]={
				L"TYPE_NAME",
				L"DATA_TYPE",
				L"COLUMN_SIZE",
				L"LITERAL_PREFIX",
				L"LITERAL_SUFFIX",
				L"CREATE_PARAMS",
				L"IS_NULLABLE",
				L"CASE_SENSITIVE",
				L"SEARCHABLE",
				L"UNSIGNED_ATTRIBUTE",
				L"FIXED_PREC_SCALE",
				L"AUTO_UNIQUE_VALUE",
				L"LOCAL_TYPE_NAME",
				L"MINIMUM_SCALE",
				L"MAXIMUM_SCALE",
				L"GUID",
				L"TYPELIB",
				L"VERSION",
				L"IS_LONG",
				L"BEST_MATCH",
				L"IS_FIXEDLENGTH"
};
const DBTYPE	rgtypePROVIDER_TYPES[cPROVIDER_TYPES]={
				DBTYPE_WSTR,
				DBTYPE_UI2,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
 				DBTYPE_UI4,
 				DBTYPE_BOOL,
 				DBTYPE_BOOL,
				DBTYPE_BOOL,
  				DBTYPE_WSTR,
				DBTYPE_I2,
				DBTYPE_I2,
 				DBTYPE_GUID,
 				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_BOOL
};
//-----------------------------------------------------------------------------
// ASSERTIONS
//-----------------------------------------------------------------------------
#define cASSERTIONS 6
#define cASSERTIONS_RESTRICTIONS 3
const WCHAR	*	rgwszASSERTIONS[cASSERTIONS]= {
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME",
				L"IS_DEFERRABLE",
				L"INITIALLY_DEFERRED",
				L"DESCRIPTION"
};
const DBTYPE	rgtypeASSERTIONS[cASSERTIONS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// CHARACTER_SETS
//-----------------------------------------------------------------------------
#define cCHARACTER_SETS	8
#define cCHARACTER_SETS_RESTRICTIONS 3
const WCHAR	*	rgwszCHARACTER_SETS[cCHARACTER_SETS]={
				L"CHARACTER_SET_CATALOG",
				L"CHARACTER_SET_SCHEMA",
				L"CHARACTER_SET_NAME",
				L"FORM_OF_USE",
				L"NUMBER_OF_CHARACTERS",
				L"DEFAULT_COLLATE_CATALOG",
				L"DEFAULT_COLLATE_SCHEMA",
				L"DEFAULT_COLLATE_NAME"
};
const DBTYPE	rgtypeCHARACTER_SETS[cCHARACTER_SETS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_I8,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// CHECK_CONSTRAINTS
//-----------------------------------------------------------------------------
#define cCHECK_CONSTRAINTS 5
#define cCHECK_CONSTRAINTS_RESTRICTIONS 3
const WCHAR	*	rgwszCHECK_CONSTRAINTS[cCHECK_CONSTRAINTS]={
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME",
				L"CHECK_CLAUSE",
				L"DESCRIPTION"
};
const DBTYPE	rgtypeCHECK_CONSTRAINTS[cCHECK_CONSTRAINTS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// COLLATIONS 
//-----------------------------------------------------------------------------
#define cCOLLATIONS 7
#define cCOLLATIONS_RESTRICTIONS 3
const WCHAR	*	rgwszCOLLATIONS[cCOLLATIONS]={
				L"COLLATION_CATALOG",
				L"COLLATION_SCHEMA",
				L"COLLATION_NAME",
				L"CHARACTER_SET_CATALOG",
				L"CHARACTER_SET_SCHEMA",
				L"CHARACTER_SET_NAME",
				L"PAD_ATTRIBUTE"
};
const DBTYPE	rgtypeCOLLATIONS[cCOLLATIONS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// COLUMN_DOMAIN_USAGE
//-----------------------------------------------------------------------------
#define cCOLUMN_DOMAIN_USAGE 9
#define cCOLUMN_DOMAIN_USAGE_RESTRICTIONS 4
const WCHAR	*	rgwszCOLUMN_DOMAIN_USAGE[cCOLUMN_DOMAIN_USAGE]=	{
				L"DOMAIN_CATALOG",
				L"DOMAIN_SCHEMA",
				L"DOMAIN_NAME",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID"
};
const DBTYPE	rgtypeCOLUMN_DOMAIN_USAGE[cCOLUMN_DOMAIN_USAGE]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4
};
//-----------------------------------------------------------------------------
// COLUMN_PRIVILEGES
//-----------------------------------------------------------------------------
#define cCOLUMN_PRIVILEGES 10
#define cCOLUMN_PRIVILEGES_RESTRICTIONS 6
const WCHAR	*	rgwszCOLUMN_PRIVILEGES[cCOLUMN_PRIVILEGES]=	{
				L"GRANTOR",
				L"GRANTEE",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"PRIVILEGE_TYPE",
				L"IS_GRANTABLE"
};
const DBTYPE	rgtypeCOLUMN_PRIVILEGES[cCOLUMN_PRIVILEGES]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_BOOL
};
//-----------------------------------------------------------------------------
// CONSTRAINT_COLUMN_USAGE 
//-----------------------------------------------------------------------------
#define cCONSTRAINT_COLUMN_USAGE 9
#define cCONSTRAINT_COLUMN_USAGE_RESTRICTIONS 4
const WCHAR	*	rgwszCONSTRAINT_COLUMN_USAGE[cCONSTRAINT_COLUMN_USAGE]=	{
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME"
};
const DBTYPE	rgtypeCONSTRAINT_COLUMN_USAGE[cCONSTRAINT_COLUMN_USAGE]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// CONSTRAINT_TABLE_USAGE 
//-----------------------------------------------------------------------------
#define cCONSTRAINT_TABLE_USAGE 6
#define cCONSTRAINT_TABLE_USAGE_RESTRICTIONS 3
const WCHAR	*	rgwszCONSTRAINT_TABLE_USAGE[cCONSTRAINT_TABLE_USAGE]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME"
};
const DBTYPE	rgtypeCONSTRAINT_TABLE_USAGE[cCONSTRAINT_TABLE_USAGE]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// KEY_COLUMN_USAGE 
//-----------------------------------------------------------------------------
#define cKEY_COLUMN_USAGE 10 
#define cKEY_COLUMN_USAGE_RESTRICTIONS 7
const WCHAR	*	rgwszKEY_COLUMN_USAGE[cKEY_COLUMN_USAGE]={
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"ORDINAL_POSITION"
};
const DBTYPE	rgtypeKEY_COLUMN_USAGE[cKEY_COLUMN_USAGE]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4
};
//-----------------------------------------------------------------------------
// PROCEDURES 
//-----------------------------------------------------------------------------
#define cPROCEDURES 8 
#define cPROCEDURES_RESTRICTIONS 4
#define cPROCEDURE_TYPES 3

const WCHAR *	rgwszPROCEDURE_TYPES[cPROCEDURE_TYPES]={
				L"DB_PT_UNKNOWN",
				L"DB_PT_PROCEDURE",
				L"DB_PT_FUNCTION"
};

const WCHAR	*	rgwszPROCEDURES[cPROCEDURES]={
				L"PROCEDURE_CATALOG",
				L"PROCEDURE_SCHEMA",
				L"PROCEDURE_NAME",
				L"PROCEDURE_TYPE",
				L"PROCEDURE_DEFINITION",
				L"DESCRIPTION",
				L"DATE_CREATED",
				L"DATE_MODIFIED"
};
const DBTYPE	rgtypePROCEDURES[cPROCEDURES]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_I2,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_DATE,
				DBTYPE_DATE
};
//-----------------------------------------------------------------------------
// REFERENTIAL_CONSTRAINTS
//-----------------------------------------------------------------------------
#define cREFERENTIAL_CONSTRAINTS 10
#define cREFERENTIAL_CONSTRAINTS_RESTRICTIONS 3
const WCHAR	*	rgwszREFERENTIAL_CONSTRAINTS[cREFERENTIAL_CONSTRAINTS]={
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME",
				L"UNIQUE_CONSTRAINT_CATALOG",
				L"UNIQUE_CONSTRAINT_SCHEMA",
				L"UNIQUE_CONSTRAINT_NAME",
				L"MATCH_OPTION",
				L"UPDATE_RULE",
				L"DELETE_RULE",
				L"DESCRIPTION"
};
const DBTYPE	rgtypeREFERENTIAL_CONSTRAINTS[cREFERENTIAL_CONSTRAINTS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// SCHEMATA 
//-----------------------------------------------------------------------------
#define cSCHEMATA 6
#define cSCHEMATA_RESTRICTIONS 3
const WCHAR	*	rgwszSCHEMATA[cSCHEMATA]={
				L"CATALOG_NAME",
				L"SCHEMA_NAME",
				L"SCHEMA_OWNER",
				L"DEFAULT_CHARACTER_SET_CATALOG",
				L"DEFAULT_CHARACTER_SET_SCHEMA",
				L"DEFAULT_CHARACTER_SET_NAME"
};
const DBTYPE	rgtypeSCHEMATA[cSCHEMATA]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// SQL_LANGUAGES 
//-----------------------------------------------------------------------------
#define cSQL_LANGUAGES 7 
#define cSQL_LANGUAGES_RESTRICTIONS 0
const WCHAR	*	rgwszSQL_LANGUAGES[cSQL_LANGUAGES]={
				L"SQL_LANGUAGE_SOURCE",
				L"SQL_LANGUAGE_YEAR",
				L"SQL_LANGUAGE_CONFORMANCE",
				L"SQL_LANGUAGE_INTEGRITY",
				L"SQL_LANGUAGE_IMPLEMENTATION",
				L"SQL_LANGUAGE_BINDING_STYLE",
				L"SQL_LANGUAGE_PROGRAMMING_LANGUAGE"
};
const DBTYPE	rgtypeSQL_LANGUAGES[cSQL_LANGUAGES]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
};
//-----------------------------------------------------------------------------
// TABLES_INFO
//-----------------------------------------------------------------------------
#define cTABLES_INFO 14 
#define cTABLES_INFO_RESTRICTIONS 4
const WCHAR	*	rgwszTABLES_INFO[cTABLES_INFO]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"TABLE_TYPE",
				L"TABLE_GUID",
				L"BOOKMARKS",
				L"BOOKMARK_TYPE",
				L"BOOKMARK_DATATYPE",
				L"BOOKMARK_MAXIMUM_LENGTH",
				L"BOOKMARK_INFORMATION",
				L"TABLE_VERSION",
				L"CARDINALITY",
				L"DESCRIPTION",
				L"TABLE_PROPID",
};
const DBTYPE	rgtypeTABLES_INFO[cTABLES_INFO]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_BOOL,
				DBTYPE_I4,
				DBTYPE_UI2,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_I8,
				DBTYPE_UI8,
				DBTYPE_WSTR,
				DBTYPE_UI4
};
//-----------------------------------------------------------------------------
// TABLE_CONSTRAINTS 
//-----------------------------------------------------------------------------
#define cTABLE_CONSTRAINTS 10
#define cTABLE_CONSTRAINTS_RESTRICTIONS 7
const WCHAR	*	rgwszTABLE_CONSTRAINTS[cTABLE_CONSTRAINTS]={
				L"CONSTRAINT_CATALOG",
				L"CONSTRAINT_SCHEMA",
				L"CONSTRAINT_NAME",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"CONSTRAINT_TYPE",
				L"IS_DEFERRABLE",
				L"INITIALLY_DEFERRED",
				L"DESCRIPTION"
};
const DBTYPE	rgtypeTABLE_CONSTRAINTS[cTABLE_CONSTRAINTS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_WSTR

};
//-----------------------------------------------------------------------------
// TABLE_PRIVILEGES 
//-----------------------------------------------------------------------------
#define cTABLE_PRIVILEGES 7
#define cTABLE_PRIVILEGES_RESTRICTIONS 5
const WCHAR	*	rgwszTABLE_PRIVILEGES[cTABLE_PRIVILEGES]= {
				L"GRANTOR",
				L"GRANTEE",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"PRIVILEGE_TYPE",
				L"IS_GRANTABLE"
};
const DBTYPE	rgtypeTABLE_PRIVILEGES[cTABLE_PRIVILEGES]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL
};
//-----------------------------------------------------------------------------
// TABLE_STATISTICS
//-----------------------------------------------------------------------------
#define cTABLE_STATISTICS 18 
#define cTABLE_STATISTICS_RESTRICTIONS 7
const WCHAR	*	rgwszTABLESTATISTICS[cTABLE_STATISTICS]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"STATISTICS_CATALOG",
				L"STATISTICS_SCHEMA",
				L"STATISTICS_NAME",
				L"STATISTICS_TYPE",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"ORDINAL_POSITION",
				L"SAMPLE_PCT",
				L"LAST_UPDATE_TIME",
				L"NO_OF_RANGES",
				L"COLUMN_CARDINALITY",
				L"TUPLE_CARDINALITY",
				L"TABLE_CARDINALITY",
				L"AVG_COLUMN_LENGTH"
};
const DBTYPE	rgtypeTABLESTATISTICS[cTABLE_STATISTICS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_UI2,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_UI2,
				DBTYPE_DATE,
				DBTYPE_UI4,
				DBTYPE_I8,
				DBTYPE_I8,
				DBTYPE_I8,
				DBTYPE_UI4
};

enum TS_ENUM	// Table Statistics column ordinals
{
	TS_TABLE_CATALOG = 1,
	TS_TABLE_SCHEMA,
	TS_TABLE_NAME,
	TS_STATISTICS_CATALOG,
	TS_STATISTICS_SCHEMA,
	TS_STATISTICS_NAME,
	TS_STATISTICS_TYPE,
	TS_COLUMN_NAME,
	TS_COLUMN_GUID,
	TS_COLUMN_PROPID,
	TS_ORDINAL_POSITION,
	TS_SAMPLE_PCT,
	TS_LAST_UPDATE_TIME,
	TS_NO_OF_RANGES,
	TS_COLUMN_CARDINALITY,
	TS_TUPLE_CARDINALITY,
	TS_TABLE_CARDINALITY,
	TS_AVG_COLUMN_LENGTH
};

//-----------------------------------------------------------------------------
// TRANSLATIONS 
//-----------------------------------------------------------------------------
#define cTRANSLATIONS 9
#define cTRANSLATIONS_RESTRICTIONS 3
const WCHAR	*	rgwszTRANSLATIONS[cTRANSLATIONS]={
				L"TRANSLATION_CATALOG",
				L"TRANSLATION_SCHEMA",
				L"TRANSLATION_NAME",
				L"SOURCE_CHARACTER_SET_CATALOG",
				L"SOURCE_CHARACTER_SET_SCHEMA",
				L"SOURCE_CHARACTER_SET_NAME",
				L"TARGET_CHARACTER_SET_CATALOG",
				L"TARGET_CHARACTER_SET_SCHEMA",
				L"TARGET_CHARACTER_SET_NAME"
};
const DBTYPE	rgtypeTRANSLATIONS[cTRANSLATIONS]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};

//-----------------------------------------------------------------------------
// TRUSTEE
//-----------------------------------------------------------------------------
#define cTRUSTEE 4
#define cTRUSTEE_RESTRICTIONS 4
const WCHAR	*	rgwszTRUSTEE[]={
				L"TRUSTEE_NAME",
				L"TRUSTEE_GUID",
				L"TRUSTEE_PROPID",
				L"TRUSTEE_TYPE"
};
const DBTYPE	rgtypeTRUSTEE[]= {
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4
};

//-----------------------------------------------------------------------------
// USAGE_PRIVILEGES 
//-----------------------------------------------------------------------------
#define cUSAGE_PRIVILEGES 8
#define cUSAGE_PRIVILEGES_RESTRICTIONS 6
#define cOBJECT_TYPES 4

const WCHAR *	rgwszOBJECT_TYPES[cOBJECT_TYPES]={
				L"DOMAIN",
				L"CHARACTER_SET",
				L"COLLATION",
				L"TRANSLATION"
};
const WCHAR	*	rgwszUSAGE_PRIVILEGES [cUSAGE_PRIVILEGES ]=	{
				L"GRANTOR",
				L"GRANTEE",
				L"OBJECT_CATALOG",
				L"OBJECT_SCHEMA",
				L"OBJECT_NAME",
				L"OBJECT_TYPE",
				L"PRIVILEGE_TYPE",
				L"IS_GRANTABLE"
};
const DBTYPE	rgtypeUSAGE_PRIVILEGES [cUSAGE_PRIVILEGES ]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL
};
//-----------------------------------------------------------------------------
// VIEW_TABLE_USAGE
//-----------------------------------------------------------------------------
#define cVIEW_TABLE_USAGE 6
#define cVIEW_TABLE_USAGE_RESTRICTIONS 3
const WCHAR	*	rgwszVIEW_TABLE_USAGE[cVIEW_TABLE_USAGE]={
				L"VIEW_CATALOG",
				L"VIEW_SCHEMA",
				L"VIEW_NAME",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME"
};
const DBTYPE	rgtypeVIEW_TABLE_USAGE[cVIEW_TABLE_USAGE]= {
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// VIEW_COLUMN_USAGE 
//-----------------------------------------------------------------------------
#define cVIEW_COLUMN_USAGE 9
#define cVIEW_COLUMN_USAGE_RESTRICTIONS 3
const WCHAR	*	rgwszVIEW_COLUMN_USAGE[cVIEW_COLUMN_USAGE]=	{
				L"VIEW_CATALOG",
				L"VIEW_SCHEMA",
				L"VIEW_NAME",
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID"
};
const DBTYPE	rgtypeVIEW_COLUMN_USAGE[cVIEW_COLUMN_USAGE]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4
};

//-----------------------------------------------------------------------------
// VIEWS 
//-----------------------------------------------------------------------------
#define cVIEWS 9
#define cVIEWS_RESTRICTIONS 3
const WCHAR	*	rgwszVIEWS[cVIEWS]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"VIEW_DEFINITION",
				L"CHECK_OPTION",
				L"IS_UPDATABLE",
				L"DESCRIPTION",
				L"DATE_CREATED",
				L"DATE_MODIFIED"
};
const DBTYPE	rgtypeVIEWS[cVIEWS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_WSTR,
				DBTYPE_DATE,
				DBTYPE_DATE
};

//-----------------------------------------------------------------------------
// CATALOGS
//-----------------------------------------------------------------------------
#define cCATALOGS 2
#define cCATALOGS_RESTRICTIONS 1
const WCHAR	*	rgwszCATALOGS[cCATALOGS]={
				L"CATALOG_NAME",
				L"DESCRIPTION",
};
const DBTYPE	rgtypeCATALOGS[cCATALOGS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
};
//-----------------------------------------------------------------------------
// INDEXES 
//-----------------------------------------------------------------------------
#define cINDEXES 25
#define cINDEXES_RESTRICTIONS 5
#define cINDEX_TYPES 4

const WCHAR *	rgwszINDEX_TYPES[cINDEX_TYPES]={
				L"DBPROP_IT_BTREE",
				L"DBPROP_IT_HASH",
				L"DBPROP_IT_CONTENT",
				L"DBPROP_IT_OTHER"
};
const WCHAR	*	rgwszINDEXES[cINDEXES]=	{
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"INDEX_CATALOG",
				L"INDEX_SCHEMA",
				L"INDEX_NAME",
				L"PRIMARY_KEY",
				L"UNIQUE",
				L"CLUSTERED",
				L"TYPE",
				L"FILL_FACTOR",
				L"INITIAL_SIZE",
				L"NULLS",
				L"SORT_BOOKMARKS",
				L"AUTO_UPDATE",
				L"NULL_COLLATION",
				L"ORDINAL_POSITION",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"COLLATION",
				L"CARDINALITY",
				L"PAGES",
				L"FILTER_CONDITION",
				L"INTEGRATED"
};
const DBTYPE	rgtypeINDEXES[cINDEXES]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_UI2,
				DBTYPE_I4,
				DBTYPE_I4,
				DBTYPE_I4,
				DBTYPE_BOOL,
				DBTYPE_BOOL,
				DBTYPE_I4,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_I2,
				DBTYPE_UI8,
				DBTYPE_I4,
				DBTYPE_WSTR,
				DBTYPE_BOOL
};

//-----------------------------------------------------------------------------
// STATISTICS 
//-----------------------------------------------------------------------------
#define cSTATISTICS	4
#define cSTATISTICS_RESTRICTIONS 3
const WCHAR	*	rgwszSTATISTICS[cSTATISTICS]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"CARDINALITY"
};
const DBTYPE	rgtypeSTATISTICS[cSTATISTICS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_UI8,
};
//-----------------------------------------------------------------------------
// PROCEDURE_PARAMETERS
//-----------------------------------------------------------------------------
#define cPROCEDURE_PARAMETERS 17
#define cPROCEDURE_PARAMETERS_RESTRICTIONS 4 
const WCHAR	*	rgwszPROCEDURE_PARAMETERS[cPROCEDURE_PARAMETERS]={
				L"PROCEDURE_CATALOG",
				L"PROCEDURE_SCHEMA",
				L"PROCEDURE_NAME",
				L"PARAMETER_NAME",
				L"ORDINAL_POSITION",
				L"PARAMETER_TYPE",
				L"PARAMETER_HASDEFAULT",
				L"PARAMETER_DEFAULT",
				L"IS_NULLABLE",
				L"DATA_TYPE",
				L"CHARACTER_MAXIMUM_LENGTH",
				L"CHARACTER_OCTET_LENGTH",
				L"NUMERIC_PRECISION",
				L"NUMERIC_SCALE",
				L"DESCRIPTION",
				L"TYPE_NAME",
				L"LOCAL_TYPE_NAME"
};
const DBTYPE	rgtypePROCEDURE_PARAMETERS[cPROCEDURE_PARAMETERS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_UI2,
				DBTYPE_UI2,
				DBTYPE_BOOL,
				DBTYPE_WSTR,
				DBTYPE_BOOL,
				DBTYPE_UI2,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_UI2,
				DBTYPE_I2,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// FOREIGN KEYS
//-----------------------------------------------------------------------------
#define cFOREIGN_KEYS 18
#define cFOREIGN_KEYS_RESTRICTIONS 6 
const WCHAR	*	rgwszFOREIGN_KEYS[cFOREIGN_KEYS]={
				L"PK_TABLE_CATALOG",
				L"PK_TABLE_SCHEMA",
				L"PK_TABLE_NAME",
				L"PK_COLUMN_NAME",
				L"PK_COLUMN_GUID",
				L"PK_COLUMN_PROPID",
				L"FK_TABLE_CATALOG",
				L"FK_TABLE_SCHEMA",
				L"FK_TABLE_NAME",
				L"FK_COLUMN_NAME",
				L"FK_COLUMN_GUID",
				L"FK_COLUMN_PROPID",
				L"ORDINAL",
				L"UPDATE_RULE",
				L"DELETE_RULE",
				L"PK_NAME",
				L"FK_NAME",
				L"DEFERRABILITY",
};
const DBTYPE	rgtypeFOREIGN_KEYS[cFOREIGN_KEYS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_I2
};
//-----------------------------------------------------------------------------
// PRIMARY KEYS
//-----------------------------------------------------------------------------
#define cPRIMARY_KEYS 8
#define cPRIMARY_KEYS_RESTRICTIONS 3 
const WCHAR	*	rgwszPRIMARY_KEYS[cPRIMARY_KEYS]={
				L"TABLE_CATALOG",
				L"TABLE_SCHEMA",
				L"TABLE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"ORDINAL",
				L"PK_NAME"
};
const DBTYPE	rgtypePRIMARY_KEYS[cPRIMARY_KEYS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_WSTR
};
//-----------------------------------------------------------------------------
// PROCEDURE_COLUMNS
//-----------------------------------------------------------------------------
#define cPROCEDURE_COLUMNS 16
#define cPROCEDURE_COLUMNS_RESTRICTIONS 4 
const WCHAR	*	rgwszPROCEDURE_COLUMNS[cPROCEDURE_COLUMNS]={
				L"PROCEDURE_CATALOG",
				L"PROCEDURE_SCHEMA",
				L"PROCEDURE_NAME",
				L"COLUMN_NAME",
				L"COLUMN_GUID",
				L"COLUMN_PROPID",
				L"ROWSET_NUMBER",
				L"ORDINAL_POSITION",
				L"IS_NULLABLE",
				L"DATA_TYPE",
				L"TYPE_GUID",
				L"CHARACTER_MAXIMUM_LENGTH",
				L"CHARACTER_OCTET_LENGTH",
				L"NUMERIC_PRECISION",
				L"NUMERIC_SCALE",
				L"DESCRIPTION"
};
const DBTYPE	rgtypePROCEDURE_COLUMNS[cPROCEDURE_COLUMNS]={
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_WSTR,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_BOOL,
				DBTYPE_UI2,
				DBTYPE_GUID,
				DBTYPE_UI4,
				DBTYPE_UI4,
				DBTYPE_UI2,
				DBTYPE_I2,
				DBTYPE_WSTR
};

//-----------------------------------------------------------------------------
// Histogram Rowset column info
//-----------------------------------------------------------------------------
#define cHistogramCols	4
const WCHAR	*	rgwszHistogramCols[cHistogramCols]={
				L"RANGE_HI_KEY",
				L"RANGE_ROWS",
				L"EQ_ROWS",
				L"DISTINCT_RANGE_ROWS"
};
DBTYPE		rgtypeHistogramCols[cHistogramCols]={
				DBTYPE_EMPTY,		// Changed at runtime to match base column
				DBTYPE_R8,
				DBTYPE_R8,
				DBTYPE_I8
};

//-----------------------------------------------------------------------------
// Updateable Properties
//-----------------------------------------------------------------------------
#define cUPDATE_PROPERTIES 6
const DBPROPID rgUpdateProperties[cUPDATE_PROPERTIES]=
{
	DBPROP_APPENDONLY,
	DBPROP_OTHERINSERT,
	DBPROP_OTHERUPDATEDELETE,
	DBPROP_REMOVEDELETED,
	DBPROP_IRowsetChange,
	DBPROP_IRowsetUpdate
};
const WCHAR * rgwszUpdateProperties[cUPDATE_PROPERTIES]=
{
	L"DBPROP_APPENDONLY",
	L"DBPROP_OTHERINSERT",
	L"DBPROP_OTHERUPDATEDELETE",
	L"DBPROP_REMOVEDELETED",
	L"DBPROP_IRowsetChange",
	L"DBPROP_IRowsetUpdate"
};

inline ULONG
GetLongRandomNumber()
{
	LONG num = 0;
	static ULONG seed=5;	

	// Seed the random number generator
	srand((unsigned)time(NULL)*seed);

	// We don't want a number more than 4 characters.
	num = rand()%99999;

	// Update the seed value
	seed = (ULONG)num;

	return (num < 0)? (0-num):(num);

}


const GUID * g_prgSchemas[] = 
{
	&DBSCHEMA_ASSERTIONS,
	&DBSCHEMA_CATALOGS,
	&DBSCHEMA_CHARACTER_SETS,
	&DBSCHEMA_COLLATIONS,
	&DBSCHEMA_COLUMNS,
	&DBSCHEMA_CHECK_CONSTRAINTS,
	&DBSCHEMA_CONSTRAINT_COLUMN_USAGE,
	&DBSCHEMA_CONSTRAINT_TABLE_USAGE,
	&DBSCHEMA_KEY_COLUMN_USAGE,
	&DBSCHEMA_REFERENTIAL_CONSTRAINTS,
	&DBSCHEMA_TABLE_CONSTRAINTS,
	&DBSCHEMA_COLUMN_DOMAIN_USAGE,
	&DBSCHEMA_INDEXES,
	&DBSCHEMA_COLUMN_PRIVILEGES,
	&DBSCHEMA_TABLE_PRIVILEGES,
	&DBSCHEMA_USAGE_PRIVILEGES,
	&DBSCHEMA_PROCEDURES,
	&DBSCHEMA_SCHEMATA,
	&DBSCHEMA_SQL_LANGUAGES,
	&DBSCHEMA_STATISTICS,
	&DBSCHEMA_TABLES,
	&DBSCHEMA_TABLES_INFO,
	&DBSCHEMA_TRANSLATIONS,
	&DBSCHEMA_PROVIDER_TYPES,
	&DBSCHEMA_VIEWS,
	&DBSCHEMA_VIEW_COLUMN_USAGE,
	&DBSCHEMA_VIEW_TABLE_USAGE,
	&DBSCHEMA_PROCEDURE_PARAMETERS,
	&DBSCHEMA_FOREIGN_KEYS,
	&DBSCHEMA_PRIMARY_KEYS,
	&DBSCHEMA_PROCEDURE_COLUMNS,
	&DBSCHEMA_TABLE_STATISTICS
};

// This lists provider specific syntax for sorting and statistics updating.
// For the statistics update syntax, the test currently expects that if there is a replacable param
// then it is the schema (owner).
struct DBMSList
{
	LPWSTR pwszDBMSName;
	LPWSTR pwszUpdateStatsFormat;
	LPWSTR pwszSortSetting;
} g_DBMSList[] = 
{
//	L"Microsoft SQL Server",	L"sp_updatestats",							NULL,
//	L"Oracle",					L"{call DBMS_STATS.GATHER_SCHEMA_STATS('%s',NULL, FALSE, 'FOR ALL COLUMNS SIZE 10')}",	L"ALTER SESSION SET NLS_SORT = PUNCTUATION"
	L"Microsoft SQL Server",	L"update statistics %s",					NULL,
	L"Oracle",					L"analyze table %s estimate statistics",	L"ALTER SESSION SET NLS_SORT = PUNCTUATION"
//	L"Oracle",					L"analyze table %s compute statistics for all columns",L"ALTER SESSION SET NLS_SORT = PUNCTUATION"
};

typedef struct tagColList
{
	LPWSTR pwszName;
	DBTYPE wType;
	ULONG cValidVals;
	LPVOID pValidVals;
	BOOL	fAllowNull;
} ColList;

typedef struct tagSchemaList
{
	const GUID * pguid;
	LPWSTR pwszName;
	ULONG cRestrictions;
	ULONG * pulRestrictions;
	RESTRICT_CAT * peRestrictCat;
	ULONG cColumns;
	ColList * pColList;
	ULONG cSort;
	ULONG *	pulSort;
	ULONG * pulColCount;	// Count of columns in this schema for each OLEDB version
} SchemaList;

enum RESTRICTIONS_ENUM
{
	// Restriction Values
	RV_ONE = 1,
	RV_TWO,
	RV_THREE,
	RV_FOUR,
	RV_FIVE,
	RV_SIX,
	RV_SEVEN,
	RV_NULL = 1000,
	RV_ALL,
};

enum RESTRICT_ARRAY
{
	RT_CACHED,
	RT_SUPPORTED,
	RT_CUSTOM
};

enum eOLEDBVER
{
	// Version of OLEDB supported by the provider
	VER_20 = 0,
	VER_21,
	VER_25,		
	VER_26,
	VER_27,
	VER_MAX	// Must remain last enum value
};

//-----------------------------------------------------------------------------
// DBSCHEMA_ASSERTIONS
//-----------------------------------------------------------------------------
VARIANT_BOOL rgBoolList[] = {VARIANT_TRUE, VARIANT_FALSE};

enum ASSERTIONS_ENUM	// Column ordinals
{
	AS_CONSTRAINT_CATALOG = 1,
	AS_CONSTRAINT_SCHEMA,
	AS_CONSTRAINT_NAME,
	AS_IS_DEFERRABLE,
	AS_INITIALLY_DEFERRED,
	AS_DESCRIPTION
};

ULONG rgAssertionsRestrict[] = {AS_CONSTRAINT_CATALOG, AS_CONSTRAINT_SCHEMA, AS_CONSTRAINT_NAME};
RESTRICT_CAT rgAssertionsCat[]= {RC_CATALOG, RC_SCHEMA,	RC_TABLE};			

ULONG rgAssertionsSort[] = {AS_CONSTRAINT_CATALOG, AS_CONSTRAINT_SCHEMA, AS_CONSTRAINT_NAME};

ColList AssertionsCols[] = 
{
	// Name					Type			cValidVals			pValidVals	fAllowNull
	L"CONSTRAINT_CATALOG",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"CONSTRINT_SCHEMA",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"CONSTRAINT_NAME",		DBTYPE_WSTR,	0,					NULL,		FALSE,
	L"IS_DEFERRABLE",		DBTYPE_BOOL,	NUMELEM(rgBoolList),rgBoolList, FALSE,
	L"INITIALLY_DEFERRED",	DBTYPE_BOOL,	NUMELEM(rgBoolList),rgBoolList, FALSE,
	L"DESCRIPTION",			DBTYPE_WSTR,	0,					NULL,		TRUE
};

//-----------------------------------------------------------------------------
// DBSCHEMA_CATALOGS
//-----------------------------------------------------------------------------
enum CATALOGS_ENUM
{
	CATS_CATALOG_NAME = 1,
	CATS_DESCRIPTION
};

ULONG rgCatalogsSort[] = {CATS_CATALOG_NAME};
ULONG rgCatalogsRestrict[] = {CATS_CATALOG_NAME};
RESTRICT_CAT rgCatalogsCat[]= {RC_CATALOG};			

ColList CatalogsCols[] = 
{
	// Name					Type			cValidVals			pValidVals	fAllowNull
	L"CATALOG_NAME",		DBTYPE_WSTR,	0,					NULL,		FALSE,
	L"DESCRIPTION",			DBTYPE_WSTR,	0,					NULL,		TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_CHARACTER_SETS
//-----------------------------------------------------------------------------
enum CHARACTER_SETS_ENUM
{
	CHSS_CHARACTER_SET_CATALOG = 1,
	CHSS_CHARACTER_SET_SCHEMA,
	CHSS_CHARACTER_SET_NAME,
	CHSS_FORM_OF_USE,
	CHSS_NUMBER_OF_CHARACTERS,
	CHSS_DEFAULT_COLLATE_CATALOG,
	CHSS_DEFAULT_COLLATE_SCHEMA,
	CHSS_DEFAULT_COLLATE_NAME
};

ULONG rgCharacterSetsSort[] = {CHSS_CHARACTER_SET_CATALOG, CHSS_CHARACTER_SET_SCHEMA, CHSS_CHARACTER_SET_NAME};
ULONG rgCharacterSetsRestrict[] = {CHSS_CHARACTER_SET_CATALOG, CHSS_CHARACTER_SET_SCHEMA, CHSS_CHARACTER_SET_NAME};
RESTRICT_CAT rgCharacterSetsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE};			

ColList CharacterSetsCols[] = 
{
	// Name					Type			cValidVals				pValidVals	fAllowNull
	L"CHARACTER_SET_CATALOG",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"CHARACTER_SET_SCHEMA",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"CHARACTER_SET_NAME",		DBTYPE_WSTR,	0,					NULL,		FALSE,
	L"FORM_OF_USE",				DBTYPE_WSTR,	0,					NULL,		FALSE,
	L"NUMBER_OF_CHARACTERS",	DBTYPE_I8,		0,					NULL,		FALSE,
	L"DEFAULT_COLLATE_CATALOG",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"DEFAULT_COLLATE_SCHEMA",	DBTYPE_WSTR,	0,					NULL,		TRUE,
	L"DEFAULT_COLLATE_NAME",	DBTYPE_WSTR,	0,					NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_COLLATIONS
//-----------------------------------------------------------------------------
LPWSTR rgwszPadVals[] = {L"NO PAD", L"PAD SPACE"};

enum COLLATIONS_ENUM
{
	COLLS_COLLATION_CATALOG = 1,
	COLLS_COLLATION_SCHEMA,
	COLLS_COLLATION_NAME,
	COLLS_CHARACTER_SET_CATALOG,
	COLLS_CHARACTER_SET_SCHEMA,
	COLLS_CHARACTER_SET_NAME,
	COLLS_PAD_ATTRIBUTE
};

ULONG rgCollationsSort[] = {COLLS_COLLATION_CATALOG, COLLS_COLLATION_SCHEMA, COLLS_COLLATION_NAME, 
	COLLS_CHARACTER_SET_CATALOG, COLLS_CHARACTER_SET_SCHEMA, COLLS_CHARACTER_SET_NAME};
ULONG rgCollationsRestrict[] = {COLLS_COLLATION_CATALOG, COLLS_COLLATION_SCHEMA, COLLS_COLLATION_NAME};
RESTRICT_CAT rgCollationsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE};			

ColList CollationsCols[] = 
{
	// Name						Type			cValidVals				pValidVals	fAllowNull
	L"COLLATION_CATALOG",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLLATION_SCHEMA",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLLATION_NAME",			DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"CHARACTER_SET_CATALOG",	DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CHARACTER_SET_SCHEMA",	DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CHARACTER_SET_NAME",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PAD_ATTRIBUTE",			DBTYPE_WSTR,	NUMELEM(rgwszPadVals),	rgwszPadVals,FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_COLUMNS
//-----------------------------------------------------------------------------

// OLEDB defined columns
enum COLUMNS_ENUM
{
	COLS_TABLE_CATALOG = 1,
	COLS_TABLE_SCHEMA,
	COLS_TABLE_NAME,
	COLS_COLUMN_NAME,
	COLS_COLUMN_GUID,
	COLS_COLUMN_PROPID,
	COLS_ORDINAL_POSITION,
	COLS_COLUMN_HASDEFAULT,
	COLS_COLUMN_DEFAULT,
	COLS_COLUMN_FLAGS,
	COLS_IS_NULLABLE,
	COLS_DATA_TYPE,
	COLS_TYPE_GUID,
	COLS_CHARACTER_MAXIMUM_LENGTH,
	COLS_CHARACTER_OCTET_LENGTH,
	COLS_NUMERIC_PRECISION,
	COLS_NUMERIC_SCALE,
	COLS_DATETIME_PRECISION,
	COLS_CHARACTER_SET_CATALOG,
	COLS_CHARACTER_SET_SCHEMA,
	COLS_CHARACTER_SET_NAME,
	COLS_COLLATION_CATALOG,
	COLS_COLLATION_SCHEMA,
	COLS_COLLATION_NAME,
	COLS_DOMAIN_CATALOG,
	COLS_DOMAIN_SCHEMA,
	COLS_DOMAIN_NAME,
	COLS_DESCRIPTION
};

ULONG rgColumnsRestrict[] = {COLS_TABLE_CATALOG, COLS_TABLE_SCHEMA, COLS_TABLE_NAME, COLS_COLUMN_NAME};
RESTRICT_CAT rgColumnsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_COLUMN};			
ULONG rgColumnsSort[] = {COLS_TABLE_CATALOG, COLS_TABLE_SCHEMA, COLS_TABLE_NAME};
ULONG rgColumnsCount[VER_MAX] = {COLS_DESCRIPTION};

ColList ColumnsCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL_POSITION",			DBTYPEFOR_DBORDINAL,0,					NULL,		TRUE,
	L"COLUMN_HASDEFAULT",			DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"COLUMN_DEFAULT",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_FLAGS",				DBTYPE_UI4,		0,						NULL,		FALSE,
	L"IS_NULLABLE",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"DATA_TYPE",					DBTYPE_UI2,		0,						NULL,		FALSE,
	L"TYPE_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"CHARACTER_MAXIMUM_LENGTH",	DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"CHARACTER_OCTET_LENGTH",		DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"NUMERIC_PRECISION",			DBTYPE_UI2,		0,						NULL,		TRUE,
	L"NUMERIC_SCALE",				DBTYPE_I2,		0,						NULL,		TRUE,
	L"DATETIME_PRECISION",			DBTYPE_UI4,		0,						NULL,		TRUE,
	L"CHARACTER_SET_CATALOG",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CHARACTER_SET_SCHEMA",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CHARACTER_SET_NAME",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLLATION_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLLATION_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLLATION_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DOMAIN_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DOMAIN_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DOMAIN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_CHECK_CONSTRAINTS
//-----------------------------------------------------------------------------

enum CHECK_CONSTRAINTS_ENUM
{
	CCS_CONSTRAINT_CATALOG = 1,
	CCS_CONSTRAINT_SCHEMA,
	CCS_CONSTRAINT_NAME,
	CCS_CHECK_CLAUSE,
	CCS_DESCRIPTION,
};

ULONG rgCheckConstraintsSort[] = {CCS_CONSTRAINT_CATALOG, CCS_CONSTRAINT_SCHEMA, CCS_CONSTRAINT_NAME};
ULONG rgCheckConstraintsRestrict[] = {CCS_CONSTRAINT_CATALOG, CCS_CONSTRAINT_SCHEMA, CCS_CONSTRAINT_NAME};
RESTRICT_CAT rgCheckConstraintsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_CONSTRAINT};			

ColList CheckConstraintsCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"CONSTRAINT_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"CHECK_CLAUSE",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE
//-----------------------------------------------------------------------------

enum CHECK_CONSTRAINTS_BY_TABLE_ENUM
{
	CCBTS_TABLE_CATALOG = 1,
	CCBTS_TABLE_SCHEMA,
	CCBTS_TABLE_NAME,
	CCBTS_CONSTRAINT_CATALOG,
	CCBTS_CONSTRAINT_SCHEMA,
	CCBTS_CONSTRAINT_NAME,
	CCBTS_CHECK_CLAUSE,
	CCBTS_DESCRIPTION,
};

ULONG rgCheckConstraintsByTableRestrict[] = {CCBTS_TABLE_CATALOG, CCBTS_TABLE_SCHEMA, CCBTS_TABLE_NAME, CCBTS_CONSTRAINT_CATALOG,
	CCBTS_CONSTRAINT_SCHEMA, CCBTS_CONSTRAINT_NAME};
RESTRICT_CAT rgCheckConstraintsByTableCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT};			

ULONG rgCheckConstraintsByTableSort[] = {CCBTS_TABLE_CATALOG, CCBTS_TABLE_SCHEMA, CCBTS_TABLE_NAME, CCBTS_CONSTRAINT_CATALOG,
	CCBTS_CONSTRAINT_SCHEMA, CCBTS_CONSTRAINT_NAME};

ColList CheckConstraintsByTableCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"CONSTRAINT_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"CHECK_CLAUSE",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
};



//-----------------------------------------------------------------------------
// DBSCHEMA_COLUMN_DOMAIN_USAGE
//-----------------------------------------------------------------------------

enum COLUMN_DOMAIN_USAGE_ENUM
{
	CDUS_DOMAIN_CATALOG = 1,
	CDUS_DOMAIN_SCHEMA,
	CDUS_DOMAIN_NAME,
	CDUS_TABLE_CATALOG,
	CDUS_TABLE_SCHEMA,
	CDUS_TABLE_NAME,
	CDUS_COLUMN_NAME,
	CDUS_COLUMN_GUID,
	CDUS_COLUMN_PROPID,
};

ULONG rgColumnDomainUsageSort[] = {CDUS_DOMAIN_CATALOG, CDUS_DOMAIN_SCHEMA, CDUS_DOMAIN_NAME, CDUS_TABLE_CATALOG,
	CDUS_TABLE_SCHEMA, CDUS_TABLE_NAME, CDUS_COLUMN_NAME, CDUS_COLUMN_GUID, CDUS_COLUMN_PROPID};
ULONG rgColumnDomainUsageRestrict[] = {CDUS_DOMAIN_CATALOG, CDUS_DOMAIN_SCHEMA, CDUS_DOMAIN_NAME, CDUS_COLUMN_NAME};
RESTRICT_CAT rgColumnDomainUsageCat[]= {RC_CATALOG, RC_SCHEMA, RC_UND, RC_COLUMN};			

ColList ColumnDomainUsageCols[] = 
{
	// Name							Type		cValidVals				pValidVals	fAllowNull
	L"DOMAIN_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DOMAIN_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DOMAIN_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_COLUMN_PRIVILEGES
//-----------------------------------------------------------------------------
LPWSTR rgwszPrivilegeTypeVals[] = {L"SELECT", L"DELETE", L"INSERT", L"UPDATE", L"REFERENCES"};

enum COLUMN_PRIVILEGES_ENUM
{
	CPS_GRANTOR = 1,
	CPS_GRANTEE,
	CPS_TABLE_CATALOG,
	CPS_TABLE_SCHEMA,
	CPS_TABLE_NAME,
	CPS_COLUMN_NAME,
	CPS_COLUMN_GUID,
	CPS_COLUMN_PROPID,
	CPS_PRIVILEGE_TYPE,
	CPS_IS_GRANTABLE,
};

ULONG rgColumnPrivilegesSort[] = {CPS_TABLE_CATALOG, CPS_TABLE_SCHEMA, CPS_TABLE_NAME, CPS_COLUMN_NAME,
	CPS_COLUMN_GUID, CPS_COLUMN_PROPID, CPS_PRIVILEGE_TYPE};
ULONG rgColumnPrivilegesRestrict[] = {CPS_TABLE_CATALOG, CPS_TABLE_SCHEMA, CPS_TABLE_NAME, CPS_COLUMN_NAME,
	CPS_GRANTOR, CPS_GRANTEE};
RESTRICT_CAT rgColumnPrivilegesCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_COLUMN, RC_USER, RC_USER};			

ColList ColumnPrivilegesCols[] = 
{
	// Name							Type		cValidVals								pValidVals				fAllowNull
	L"GRANTOR",						DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"GRANTEE",						DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,									NULL,					TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,									NULL,					TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,									NULL,					TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,									NULL,					TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,									NULL,					TRUE,
	L"PRIVILEGE_TYPE",				DBTYPE_WSTR,	NUMELEM(rgwszPrivilegeTypeVals),	rgwszPrivilegeTypeVals,	FALSE,
	L"IS_GRANTABLE",				DBTYPE_BOOL,	NUMELEM(rgBoolList),				rgBoolList,				FALSE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_CONSTRAINT_COLUMN_USAGE
//-----------------------------------------------------------------------------

enum CONSTRAINT_COLUMN_USAGE_ENUM
{
	CCUS_TABLE_CATALOG = 1,
	CCUS_TABLE_SCHEMA,
	CCUS_TABLE_NAME,
	CCUS_COLUMN_NAME,
	CCUS_COLUMN_GUID,
	CCUS_COLUMN_PROPID,
	CCUS_CONSTRAINT_CATALOG,
	CCUS_CONSTRAINT_SCHEMA,
	CCUS_CONSTRAINT_NAME,
};

ULONG rgConstraintColumnUsageRestrict[] = {CCUS_TABLE_CATALOG, CCUS_TABLE_SCHEMA, CCUS_TABLE_NAME, CCUS_COLUMN_NAME,
	CCUS_CONSTRAINT_CATALOG, CCUS_CONSTRAINT_SCHEMA, CCUS_CONSTRAINT_NAME};
ULONG rgConstraintColumnUsageSort[] = {CCUS_TABLE_CATALOG, CCUS_TABLE_SCHEMA, CCUS_TABLE_NAME, CCUS_COLUMN_NAME,
	CCUS_COLUMN_GUID, CCUS_COLUMN_PROPID, CCUS_CONSTRAINT_CATALOG, CCUS_CONSTRAINT_SCHEMA, CCUS_CONSTRAINT_NAME};
RESTRICT_CAT rgConstraintColumnUsageCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_COLUMN, RC_UND, RC_UND, RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT};			

ColList ConstraintColumnUsageCols[] = 
{
	// Name							Type		cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",				DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",			DBTYPE_UI4,		0,						NULL,		TRUE,
	L"CONSTRAINT_CATALOG",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_SCHEMA",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_NAME",			DBTYPE_WSTR,	0,						NULL,		FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_CONSTRAINT_TABLE_USAGE
//-----------------------------------------------------------------------------

enum CONSTRAINT_TABLE_USAGE_ENUM
{
	CTUS_TABLE_CATALOG = 1,
	CTUS_TABLE_SCHEMA,
	CTUS_TABLE_NAME,
	CTUS_CONSTRAINT_CATALOG,
	CTUS_CONSTRAINT_SCHEMA,
	CTUS_CONSTRAINT_NAME,
};

ULONG rgConstraintTableUsageSort[] = {CTUS_TABLE_CATALOG, CTUS_TABLE_SCHEMA, CTUS_TABLE_NAME, CTUS_CONSTRAINT_CATALOG,
	CTUS_CONSTRAINT_SCHEMA, CTUS_CONSTRAINT_NAME};
ULONG rgConstraintTableUsageRestrict[] = {CCUS_TABLE_CATALOG, CCUS_TABLE_SCHEMA, CCUS_TABLE_NAME, CCUS_COLUMN_NAME,
	CCUS_CONSTRAINT_CATALOG, CCUS_CONSTRAINT_SCHEMA, CCUS_CONSTRAINT_NAME};
RESTRICT_CAT rgConstraintTableUsageCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_COLUMN, RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT};			

ColList ConstraintTableUsageCols[] = 
{
	// Name							Type		cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"CONSTRAINT_CATALOG",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_SCHEMA",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_NAME",			DBTYPE_WSTR,	0,						NULL,		FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_FOREIGN_KEYS
//-----------------------------------------------------------------------------
LPWSTR rgwszRuleVals[] = {L"CASCADE", L"SET NULL", L"SET DEFAULT", L"NO ACTION"};
SHORT	rgDeferrabilityVals[] = {DBPROPVAL_DF_INITIALLY_DEFERRED, DBPROPVAL_DF_INITIALLY_IMMEDIATE,
	DBPROPVAL_DF_NOT_DEFERRABLE};

enum FOREIGN_KEYS
{
	FKS_PK_TABLE_CATALOG = 1,
	FKS_PK_TABLE_SCHEMA,
	FKS_PK_TABLE_NAME,
	FKS_PK_COLUMN_NAME,
	FKS_PK_COLUMN_GUID,
	FKS_PK_COLUMN_PROPID,
	FKS_FK_TABLE_CATALOG,
	FKS_FK_TABLE_SCHEMA,
	FKS_FK_TABLE_NAME,
	FKS_FK_COLUMN_NAME,
	FKS_FK_COLUMN_GUID,
	FKS_FK_COLUMN_PROPID,
	FKS_ORDINAL,
	FKS_UPDATE_RULE,
	FKS_DELETE_RULE,
	FKS_PK_NAME,
	FKS_FK_NAME,
	FKS_DEFERRABILITY,
};

ULONG rgForeignKeysRestrict[] = {FKS_PK_TABLE_CATALOG, FKS_PK_TABLE_SCHEMA, FKS_PK_TABLE_NAME,
	FKS_FK_TABLE_CATALOG, FKS_FK_TABLE_SCHEMA, FKS_FK_TABLE_NAME
};
ULONG rgForeignKeysSort[] = {FKS_FK_TABLE_CATALOG, FKS_FK_TABLE_SCHEMA, FKS_FK_TABLE_NAME};
RESTRICT_CAT rgForeignKeysCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_CATALOG, RC_SCHEMA, RC_TABLE};			

ColList ForeignKeysCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"PK_TABLE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PK_TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PK_TABLE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"PK_COLUMN_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PK_COLUMN_GUID",				DBTYPE_GUID,	0,						NULL,		TRUE,
	L"PK_COLUMN_PROPID",			DBTYPE_UI4,		0,						NULL,		TRUE,
	L"FK_TABLE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"FK_TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"FK_TABLE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"FK_COLUMN_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"FK_COLUMN_GUID",				DBTYPE_GUID,	0,						NULL,		TRUE,
	L"FK_COLUMN_PROPID",			DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL",						DBTYPEFOR_DBORDINAL,0,					NULL,		FALSE,
	L"UPDATE_RULE",					DBTYPE_WSTR,	NUMELEM(rgwszRuleVals),	rgwszRuleVals,	TRUE,
	L"DELETE_RULE",					DBTYPE_WSTR,	NUMELEM(rgwszRuleVals),	rgwszRuleVals,	TRUE,
	L"PK_NAME",						DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"FK_NAME",						DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DEFERRABILITY",				DBTYPE_I2,		NUMELEM(rgDeferrabilityVals), rgDeferrabilityVals, FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_INDEXES
//-----------------------------------------------------------------------------

USHORT	rgIndexTypeVals[]={DBPROPVAL_IT_BTREE, DBPROPVAL_IT_HASH, DBPROPVAL_IT_CONTENT, DBPROPVAL_IT_OTHER};
LONG rgNullVals[] = {DBPROPVAL_IN_DISALLOWNULL, DBPROPVAL_IN_IGNORENULL, DBPROPVAL_IN_IGNOREANYNULL};
LONG rgNullCollationVals[] = {DBPROPVAL_NC_END, DBPROPVAL_NC_START, DBPROPVAL_NC_HIGH, DBPROPVAL_NC_LOW};
LONG rgCollationVals[] = {DB_COLLATION_ASC, DB_COLLATION_DESC};

enum INDEXES_ENUM
{
	IS_TABLE_CATALOG = 1,
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
};

ULONG rgIndexesRestrict[] = {IS_TABLE_CATALOG, IS_TABLE_SCHEMA, IS_INDEX_NAME, IS_TYPE, IS_TABLE_NAME};
ULONG rgIndexesSort[] = {IS_UNIQUE, IS_TYPE, IS_INDEX_CATALOG, IS_INDEX_SCHEMA,
	IS_INDEX_NAME, IS_ORDINAL_POSITION};
RESTRICT_CAT rgIndexesCat[]= {RC_CATALOG, RC_SCHEMA, RC_INDEX, RC_UND, RC_TABLE};			

ColList IndexesCols[] = 
{
	// Name							Type		cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"INDEX_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"INDEX_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"INDEX_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"PRIMARY_KEY",					DBTYPE_BOOL,	0,						NULL,		TRUE,
	L"UNIQUE",						DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList, FALSE,
	L"CLUSTERED",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList, TRUE,
	L"TYPE",						DBTYPE_UI2,		NUMELEM(rgIndexTypeVals), rgIndexTypeVals,		TRUE,
	L"FILL_FACTOR",					DBTYPE_I4,		0,						NULL,		TRUE,
	L"INITIAL_SIZE",				DBTYPEFOR_DBROWCOUNT,0,						NULL,		TRUE,
	L"NULLS",						DBTYPE_I4,		NUMELEM(rgNullVals),	rgNullVals, TRUE,
	L"SORT_BOOKMARKS",				DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList, TRUE,
	L"AUTO_UPDATE",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList, FALSE,
	L"NULL_COLLATION",				DBTYPE_I4,		NUMELEM(rgNullCollationVals),	rgNullCollationVals, FALSE,
	L"ORDINAL_POSITION",			DBTYPEFOR_DBORDINAL,0,					NULL,		TRUE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"COLLATION",					DBTYPE_I2,		NUMELEM(rgCollationVals),rgCollationVals, TRUE,
	L"CARDINALITY",					DBTYPE_UI8,		0,						NULL,		TRUE,
	L"PAGES",						DBTYPE_I4,		0,						NULL,		TRUE,
	L"FILTER_CONDITION",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"INTEGRATED",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList, TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_KEY_COLUMN_USAGE
//-----------------------------------------------------------------------------

enum KEY_COLUMN_USAGE_ENUM
{
	KCUS_CONSTRAINT_CATALOG = 1,
	KCUS_CONSTRAINT_SCHEMA,
	KCUS_CONSTRAINT_NAME,
	KCUS_TABLE_CATALOG,
	KCUS_TABLE_SCHEMA,
	KCUS_TABLE_NAME,
	KCUS_COLUMN_NAME,
	KCUS_COLUMN_GUID,
	KCUS_COLUMN_PROPID,
	KCUS_ORDINAL_POSITION,
};

ULONG rgKeyColumnUsageRestrict[] = {KCUS_CONSTRAINT_CATALOG, KCUS_CONSTRAINT_SCHEMA, KCUS_CONSTRAINT_NAME, KCUS_TABLE_CATALOG,
	KCUS_TABLE_SCHEMA, KCUS_TABLE_NAME, KCUS_COLUMN_NAME};
ULONG rgKeyColumnUsageSort[] = {KCUS_CONSTRAINT_CATALOG, KCUS_CONSTRAINT_SCHEMA, KCUS_CONSTRAINT_NAME, KCUS_TABLE_CATALOG,
	KCUS_TABLE_SCHEMA, KCUS_TABLE_NAME, KCUS_ORDINAL_POSITION};
RESTRICT_CAT rgKeyColumnUsageCat[]= {RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT, RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_COLUMN};			

ColList KeyColumnUsageCols[] = 
{
	// Name							Type		cValidVals				pValidVals	fAllowNull
	L"CONSTRAINT_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CONSTRAINT_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL_POSITION",			DBTYPEFOR_DBORDINAL,	0,					NULL,		FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_PRIMARY_KEYS
//-----------------------------------------------------------------------------
enum PRIMARY_KEYS
{
	PKS_TABLE_CATALOG = 1,
	PKS_TABLE_SCHEMA,
	PKS_TABLE_NAME,
	PKS_COLUMN_NAME,
	PKS_COLUMN_GUID,
	PKS_COLUMN_PROPID,
	PKS_ORDINAL,
	PKS_PK_NAME,
};

ULONG rgPrimaryKeysRestrict[] = {PKS_TABLE_CATALOG, PKS_TABLE_SCHEMA, PKS_TABLE_NAME};
ULONG rgPrimaryKeysSort[] = {PKS_TABLE_CATALOG, PKS_TABLE_SCHEMA, PKS_TABLE_NAME};
RESTRICT_CAT rgPrimaryKeysCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE};			

ColList PrimaryKeysCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL",						DBTYPEFOR_DBORDINAL,0,					NULL,		FALSE,
	L"PK_NAME",						DBTYPE_WSTR,	0,						NULL,		TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_PROCEDURES
//-----------------------------------------------------------------------------

SHORT rgProcedureTypeVals[]={DB_PT_UNKNOWN, DB_PT_PROCEDURE, DB_PT_FUNCTION};

enum PROCEDURES
{
	PRS_PROCEDURE_CATALOG = 1,
	PRS_PROCEDURE_SCHEMA,
	PRS_PROCEDURE_NAME,
	PRS_PROCEDURE_TYPE,
	PRS_PROCEDURE_DEFINITION,
	PRS_DESCRIPTION,
	PRS_DATE_CREATED,
	PRS_DATE_MODIFIED,
};

ULONG rgProceduresRestrict[] = {PRS_PROCEDURE_CATALOG, PRS_PROCEDURE_SCHEMA, PRS_PROCEDURE_NAME, PRS_PROCEDURE_TYPE};
ULONG rgProceduresSort[] = {PRS_PROCEDURE_CATALOG, PRS_PROCEDURE_SCHEMA, PRS_PROCEDURE_NAME};
RESTRICT_CAT rgProceduresCat[]= {RC_CATALOG, RC_SCHEMA, RC_PROCEDURE};			

ColList ProceduresCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"PROCEDURE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"PROCEDURE_TYPE",				DBTYPE_I2,		NUMELEM(rgProcedureTypeVals),rgProcedureTypeVals, FALSE,
	L"PROCEDURE_DEFINITION",		DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"DATE_CREATED",				DBTYPE_DATE,	0,						NULL,		TRUE,
	L"DATE_MODIFIED",				DBTYPE_DATE,	0,						NULL,		TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_PROCEDURE_COLUMNS
//-----------------------------------------------------------------------------

enum PROCEDURE_COLUMNS
{
	PCS_PROCEDURE_CATALOG = 1,
	PCS_PROCEDURE_SCHEMA,
	PCS_PROCEDURE_NAME,
	PCS_COLUMN_NAME,
	PCS_COLUMN_GUID,
	PCS_COLUMN_PROPID,
	PCS_ROWSET_NUMBER,
	PCS_ORDINAL_POSITION,
	PCS_COLS_IS_NULLABLE,
	PCS_COLS_DATA_TYPE,
	PCS_COLS_TYPE_GUID,
	PCS_COLS_CHARACTER_MAXIMUM_LENGTH,
	PCS_COLS_CHARACTER_OCTET_LENGTH,
	PCS_COLS_NUMERIC_PRECISION,
	PCS_COLS_NUMERIC_SCALE,
	PCS_COLS_DESCRIPTION
};

ULONG rgProcedureColumnsRestrict[] = {PCS_PROCEDURE_CATALOG, PCS_PROCEDURE_SCHEMA, PCS_PROCEDURE_NAME, PCS_COLUMN_NAME};
ULONG rgProcedureColumnsSort[] = {PCS_PROCEDURE_CATALOG, PCS_PROCEDURE_SCHEMA, PCS_PROCEDURE_NAME};
RESTRICT_CAT rgProcedureColumnsCat[]= {RC_CATALOG, RC_SCHEMA, RC_PROCEDURE};			

ColList ProcedureColumnsCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"PROCEDURE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ROWSET_NUMBER",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL_POSITION",			DBTYPEFOR_DBORDINAL,0,					NULL,		TRUE,
	L"IS_NULLABLE",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"DATA_TYPE",					DBTYPE_UI2,		0,						NULL,		FALSE,
	L"TYPE_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"CHARACTER_MAXIMUM_LENGTH",	DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"CHARACTER_OCTET_LENGTH",		DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"NUMERIC_PRECISION",			DBTYPE_UI2,		0,						NULL,		TRUE,
	L"NUMERIC_SCALE",				DBTYPE_I2,		0,						NULL,		TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_PROCEDURE_PARAMETERS
//-----------------------------------------------------------------------------

USHORT rgParameterTypeVals[] = {DBPARAMTYPE_INPUT, DBPARAMTYPE_INPUTOUTPUT, DBPARAMTYPE_OUTPUT,
	DBPARAMTYPE_RETURNVALUE};

enum PROCEDURE_PARAMETERS
{
	PPS_PROCEDURE_CATALOG = 1,
	PPS_PROCEDURE_SCHEMA,
	PPS_PROCEDURE_NAME,
	PPS_PARAMETER_NAME,
	PPS_ORDINAL_POSITION,
	PPS_PARAMETER_TYPE,
	PPS_PARAMETER_HASDEFAULT,
	PPS_PARAMETER_DEFAULT,
	PPS_IS_NULLABLE,
	PPS_DATA_TYPE,
	PPS_CHARACTER_MAXIMUM_LENGTH,
	PPS_CHARACTER_OCTET_LENGTH,
	PPS_NUMERIC_PRECISION,
	PPS_NUMERIC_SCALE,
	PPS_DESCRIPTION,
	PPS_TYPE_NAME,
	PPS_LOCAL_TYPE_NAME
};

ULONG rgProcedureParametersRestrict[] = {PPS_PROCEDURE_CATALOG, PPS_PROCEDURE_SCHEMA, PPS_PROCEDURE_NAME, PPS_PARAMETER_NAME};
ULONG rgProcedureParametersSort[] = {PPS_PROCEDURE_CATALOG, PPS_PROCEDURE_SCHEMA, PPS_PROCEDURE_NAME};
ULONG rgProcedureParametersCount[VER_MAX] = {PPS_LOCAL_TYPE_NAME};
RESTRICT_CAT rgProcedureParametersCat[]= {RC_CATALOG, RC_SCHEMA, RC_PROCEDURE, RC_PARAMETER};

ColList ProcedureParametersCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"PROCEDURE_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"PROCEDURE_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"PARAMETER_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"ORDINAL_POSITION",			DBTYPE_UI2,		0,						NULL,		FALSE,
	L"PARAMETER_TYPE",				DBTYPE_UI2,		NUMELEM(rgParameterTypeVals),	rgParameterTypeVals,TRUE,
	L"PARAMETER_HASDEFAULT",		DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	TRUE,
	L"PARAMETER_DEFAULT",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"IS_NULLABLE",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"DATA_TYPE",					DBTYPE_UI2,		0,						NULL,		FALSE,
	L"CHARACTER_MAXIMUM_LENGTH",	DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"CHARACTER_OCTET_LENGTH",		DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
	L"NUMERIC_PRECISION",			DBTYPE_UI2,		0,						NULL,		TRUE,
	L"NUMERIC_SCALE",				DBTYPE_I2,		0,						NULL,		TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TYPE_NAME",					DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"LOCAL_TYPE_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_PROVIDER_TYPES
//-----------------------------------------------------------------------------
ULONG rgSearchableVals[] = {DB_UNSEARCHABLE, DB_LIKE_ONLY, DB_ALL_EXCEPT_LIKE, DB_SEARCHABLE};

enum PROVIDER_TYPES
{
	PTS_TYPE_NAME = 1,
	PTS_DATA_TYPE,
	PTS_COLUMN_SIZE,
	PTS_LITERAL_PREFIX,
	PTS_LITERAL_SUFFIX,
	PTS_CREATE_PARAMS,
	PTS_IS_NULLABLE,
	PTS_CASE_SENSITIVE,
	PTS_SEARCHABLE,
	PTS_UNSIGNED_ATTRIBUTE,
	PTS_FIXED_PREC_SCALE,
	PTS_AUTO_UNIQUE_VALUE,
	PTS_LOCAL_TYPE_NAME,
	PTS_MINIMUM_SCALE,
	PTS_MAXIMUM_SCALE,
	PTS_GUID,
	PTS_TYPELIB,
	PTS_VERSION,
	PTS_IS_LONG,
	PTS_BEST_MATCH,
	PTS_IS_FIXEDLENGTH,
};

ULONG rgProviderTypesRestrict[] = {PTS_DATA_TYPE, PTS_BEST_MATCH};
ULONG rgProviderTypesSort[] = {PTS_DATA_TYPE};
RESTRICT_CAT rgProviderTypesCat[]= {RC_UND, RC_UND};

ColList ProviderTypesCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"TYPE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"DATA_TYPE",					DBTYPE_UI2,		0,						NULL,		FALSE,
	L"COLUMN_SIZE",					DBTYPEFOR_DBLENGTH,0,					NULL,		FALSE,
	L"LITERAL_PREFIX",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"LITERAL_SUFFIX",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"CREATE_PARAMS",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"IS_NULLABLE",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"CASE_SENSITIVE",				DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"SEARCHABLE",					DBTYPE_UI4,		NUMELEM(rgSearchableVals),	rgSearchableVals,	FALSE,
	L"UNSIGNED_ATTRIBUTE",			DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	TRUE,
	L"FIXED_PREC_SCALE",			DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"AUTO_UNIQUE_VALUE",			DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	TRUE,
	L"LOCAL_TYPE_NAME",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"MINIMUM_SCALE",				DBTYPE_I2,		0,						NULL,		TRUE,
	L"MAXIMUM_SCALE",				DBTYPE_I2,		0,						NULL,		TRUE,
	L"GUID",						DBTYPE_GUID,	0,						NULL,		TRUE,
	L"TYPELIB",						DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"VERSION",						DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"IS_LONG",						DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	FALSE,
	L"BEST_MATCH",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	TRUE,
	L"IS_FIXEDLENGTH",				DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,	TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_REFERENTIAL_CONSTRAINTS
//-----------------------------------------------------------------------------
LPWSTR rgwszMatchOptionVals[] = {L"NONE", L"PARTIAL", L"FULL"};

enum REFERENTIAL_CONSTRAINTS_ENUM
{
	RCS_CONSTRAINT_CATALOG = 1,
	RCS_CONSTRAINT_SCHEMA,
	RCS_CONSTRAINT_NAME,
	RCS_UNIQUE_CONSTRAINT_CATALOG,
	RCS_UNIQUE_CONSTRAINT_SCHEMA,
	RCS_UNIQUE_CONSTRAINT_NAME,
	RCS_MATCH_OPTION,
	RCS_UPDATE_RULE,
	RCS_DELETE_RULE,
	RCS_DESCRIPTION,
};

ULONG rgReferentialConstraintsRestrict[] = {RCS_CONSTRAINT_CATALOG, RCS_CONSTRAINT_SCHEMA, RCS_CONSTRAINT_NAME};
ULONG rgReferentialConstraintsSort[] = {RCS_CONSTRAINT_CATALOG, RCS_CONSTRAINT_SCHEMA, RCS_CONSTRAINT_NAME,
	RCS_UNIQUE_CONSTRAINT_CATALOG, RCS_UNIQUE_CONSTRAINT_SCHEMA, RCS_UNIQUE_CONSTRAINT_NAME};
RESTRICT_CAT rgReferentialConstraintsCat[]= {RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT, RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT};

ColList ReferentialConstraintsCols[] = 
{
	// Name							Type			cValidVals						pValidVals					fAllowNull
	L"CONSTRAINT_CATALOG",			DBTYPE_WSTR,	0,								NULL,						TRUE,
	L"CONSTRAINT_SCHEMA",			DBTYPE_WSTR,	0,								NULL,						TRUE,
	L"CONSTRAINT_NAME",				DBTYPE_WSTR,	0,								NULL,						FALSE,
	L"UNIQUE_CONSTRAINT_CATALOG",	DBTYPE_WSTR,	0,								NULL,						TRUE,
	L"UNIQUE_CONSTRAINT_SCHEMA",	DBTYPE_WSTR,	0,								NULL,						TRUE,
	L"UNIQUE_CONSTRAINT_NAME",		DBTYPE_WSTR,	0,								NULL,						FALSE,
	L"MATCH_OPTION",				DBTYPE_WSTR,	NUMELEM(rgwszMatchOptionVals),	rgwszMatchOptionVals,		FALSE,
	L"UPDATE_RULE",					DBTYPE_WSTR,	NUMELEM(rgwszRuleVals),			rgwszRuleVals,				FALSE,
	L"DELETE_RULE",					DBTYPE_WSTR,	NUMELEM(rgwszRuleVals),			rgwszRuleVals,				FALSE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,								NULL,						FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_SCHEMATA
//-----------------------------------------------------------------------------
enum SCHEMATA_ENUM
{
	SS_CATALOG_NAME = 1,
	SS_SCHEMA_NAME,
	SS_SCHEMA_OWNER,
	SS_DEFAULT_CHARACTER_SET_CATALOG,
	SS_DEFAULT_CHARACTER_SET_SCHEMA,
	SS_DEFAULT_CHARACTER_SET_NAME,
};

ULONG rgSchemataRestrict[] = {SS_CATALOG_NAME, SS_SCHEMA_NAME, SS_SCHEMA_OWNER};
ULONG rgSchemataSort[] = {SS_CATALOG_NAME, SS_SCHEMA_NAME, SS_SCHEMA_OWNER};
RESTRICT_CAT rgSchemataCat[]= {RC_CATALOG, RC_SCHEMA, RC_USER};

ColList SchemataCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"CATALOG_NAME",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"SCHEMA_NAME",					DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"SCHEMA_OWNER",				DBTYPE_WSTR,	0,						NULL,				TRUE, // For Kagera
	L"DEFAULT_CHARACTER_SET_CATALOG",DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"DEFAULT_CHARACTER_SET_SCHEMA",DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"DEFAULT_CHARACTER_SET_NAME",	DBTYPE_WSTR,	0,						NULL,				TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_STATISTICS
//-----------------------------------------------------------------------------
enum STATISTICS_ENUM
{
	STS_TABLE_CATALOG = 1,
	STS_TABLE_SCHEMA,
	STS_TABLE_NAME,
	STS_CARDINALITY,
};

ULONG rgStatisticsRestrict[] = {STS_TABLE_CATALOG, STS_TABLE_SCHEMA, STS_TABLE_NAME};

ULONG rgStatisticsSort[] = {STS_TABLE_CATALOG, STS_TABLE_SCHEMA, STS_TABLE_NAME};
RESTRICT_CAT rgStatisticsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE};

ColList StatisticsCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,				FALSE,
	L"CARDINALITY",					DBTYPE_UI8,		0,						NULL,				FALSE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_TABLE_CONSTRAINTS
//-----------------------------------------------------------------------------
LPWSTR rgwszConstraintTypeVals[] = {L"UNIQUE", L"PRIMARY KEY", L"FOREIGN KEY", L"CHECK"};

enum TABLE_CONSTRAINTS_ENUM
{
	TCS_CONSTRAINT_CATALOG = 1,
	TCS_CONSTRAINT_SCHEMA,
	TCS_CONSTRAINT_NAME,
	TCS_TABLE_CATALOG,
	TCS_TABLE_SCHEMA,
	TCS_TABLE_NAME,
	TCS_CONSTRAINT_TYPE,
	TCS_IS_DEFERRABLE,
	TCS_INITIALLY_DEFERRED,
	TCS_DESCRIPTION,
};

ULONG rgTableConstraintsRestrict[] = {TCS_CONSTRAINT_CATALOG, TCS_CONSTRAINT_SCHEMA, TCS_CONSTRAINT_NAME, TCS_TABLE_CATALOG, TCS_TABLE_SCHEMA,
	TCS_TABLE_NAME, TCS_CONSTRAINT_TYPE};
ULONG rgTableConstraintsSort[] = {TCS_CONSTRAINT_CATALOG, TCS_CONSTRAINT_SCHEMA, TCS_CONSTRAINT_NAME, TCS_TABLE_CATALOG,
	TCS_TABLE_SCHEMA, TCS_TABLE_NAME, TCS_CONSTRAINT_TYPE};
RESTRICT_CAT rgTableConstraintsCat[]= {RC_CATALOG, RC_SCHEMA, RC_CONSTRAINT, RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_CONSTRAINT_TYPE};

ColList TableConstraintsCols[] = 
{
	// Name							Type		cValidVals								pValidVals					fAllowNull
	L"CONSTRAINT_CATALOG",			DBTYPE_WSTR,	0,									NULL,						TRUE,
	L"CONSTRAINT_SCHEMA",			DBTYPE_WSTR,	0,									NULL,						TRUE,
	L"CONSTRAINT_NAME",				DBTYPE_WSTR,	0,									NULL,						FALSE,
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,									NULL,						TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,									NULL,						TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,									NULL,						FALSE,
	L"CONSTRAINT_TYPE",				DBTYPE_WSTR,	NUMELEM(rgwszConstraintTypeVals),	rgwszConstraintTypeVals,	FALSE,
	L"IS_DEFERRABLE",				DBTYPE_BOOL,	NUMELEM(rgBoolList),				rgBoolList,					FALSE,
	L"INITIALLY_DEFERRED",			DBTYPE_BOOL,	NUMELEM(rgBoolList),				rgBoolList,					FALSE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,									NULL,						TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_TABLE_PRIVILEGES
//-----------------------------------------------------------------------------
enum TABLE_PRIVILEGES_ENUM
{
	TPS_GRANTOR = 1,
	TPS_GRANTEE,
	TPS_TABLE_CATALOG,
	TPS_TABLE_SCHEMA,
	TPS_TABLE_NAME,
	TPS_PRIVILEGE_TYPE,
	TPS_IS_GRANTABLE,
};

ULONG rgTablePrivilegesRestrict[] = {TPS_TABLE_CATALOG, TPS_TABLE_SCHEMA, TPS_TABLE_NAME, TPS_GRANTOR, TPS_GRANTEE};
ULONG rgTablePrivilegesSort[] = {TPS_TABLE_CATALOG, TPS_TABLE_SCHEMA, TPS_TABLE_NAME, TPS_PRIVILEGE_TYPE};
RESTRICT_CAT rgTablePrivilegesCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_USER, RC_USER};

ColList TablePrivilegesCols[] = 
{
	// Name							Type		cValidVals								pValidVals				fAllowNull
	L"GRANTOR",						DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"GRANTEE",						DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,									NULL,					TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,									NULL,					TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,									NULL,					FALSE,
	L"PRIVILEGE_TYPE",				DBTYPE_WSTR,	NUMELEM(rgwszPrivilegeTypeVals),	rgwszPrivilegeTypeVals,	FALSE,
	L"IS_GRANTABLE",				DBTYPE_BOOL,	NUMELEM(rgBoolList),				rgBoolList,				FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_TABLE_STATISTICS
//-----------------------------------------------------------------------------
USHORT rgStatisticsTypesVals[] = {DBSTAT_HISTOGRAM, DBSTAT_COLUMN_CARDINALITY, 
	DBSTAT_TUPLE_CARDINALITY};

enum TABLE_STATISTICS_ENUM
{
	TSS_TABLE_CATALOG = 1,
	TSS_TABLE_SCHEMA,
	TSS_TABLE_NAME,
	TSS_STATISTICS_CATALOG,
	TSS_STATISTICS_SCHEMA,
	TSS_STATISTICS_NAME,
	TSS_STATISTICS_TYPE,
	TSS_COLUMN_NAME,
	TSS_COLUMN_GUID,
	TSS_COLUMN_PROPID,
	TSS_ORDINAL_POSITION,
	TSS_SAMPLE_PCT,
	TSS_LAST_UPDATE_TIME,
	TSS_NO_OF_RANGES,
	TSS_COLUMN_CARDINALITY,
	TSS_TUPLE_CARDINALITY,
	TSS_TABLE_CARDINALITY,
	TSS_AVG_COLUMN_LENGTH
};

ULONG rgTableStatisticsRestrict[] = {TSS_TABLE_CATALOG, TSS_TABLE_SCHEMA, TSS_TABLE_NAME,
	TSS_STATISTICS_CATALOG, TSS_STATISTICS_SCHEMA, TSS_STATISTICS_NAME, TSS_STATISTICS_TYPE};
ULONG rgTableStatisticsSort[] = {TSS_TABLE_CATALOG, TSS_TABLE_SCHEMA, TSS_TABLE_NAME, 
	TSS_STATISTICS_CATALOG, TSS_STATISTICS_SCHEMA, TSS_STATISTICS_NAME, TSS_ORDINAL_POSITION};
RESTRICT_CAT rgTableStatisticsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_CATALOG, RC_SCHEMA, RC_STATISTICS, RC_UND};

ColList TableStatisticsCols[] = 
{
	// Name							Type			cValidVals				pValidVals	fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"STATISTICS_CATALOG",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"STATISTICS_SCHEMA",			DBTYPE_WSTR,	0,						NULL,		TRUE,
	L"STATISTICS_NAME",				DBTYPE_WSTR,	0,						NULL,		FALSE,
// Type is a bitmask, and currently we don't support automatic value checking for bitmasks
//	L"STATISTICS_TYPE",				DBTYPE_UI2,		NUMELEM(rgStatisticsTypesVals),rgStatisticsTypesVals, FALSE,
	L"STATISTICS_TYPE",				DBTYPE_UI2,		0,						NULL,		FALSE,
	L"COLUMN_NAME",					DBTYPE_WSTR,	0,						NULL,		FALSE,
	L"COLUMN_GUID",					DBTYPE_GUID,	0,						NULL,		TRUE,
	L"COLUMN_PROPID",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"ORDINAL_POSITION",			DBTYPEFOR_DBORDINAL,0,					NULL,		FALSE,
	L"SAMPLE_PCT",					DBTYPE_UI2,		0,						NULL,		TRUE,
	L"LAST_UPDATE_TIME",			DBTYPE_DATE,	0,						NULL,		TRUE,
	L"NO_OF_RANGES",				DBTYPE_UI4,		0,						NULL,		TRUE,
	L"COLUMN_CARDINALITY",			DBTYPE_I8,		0,						NULL,		TRUE,
	L"TUPLE_CARDINALITY",			DBTYPE_I8,		0,						NULL,		TRUE,
	L"TABLE_CARDINALITY",			DBTYPE_I8,		0,						NULL,		FALSE,
	L"AVG_COLUMN_LENGTH",			DBTYPEFOR_DBLENGTH,0,					NULL,		TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_TABLES
//-----------------------------------------------------------------------------
LPWSTR rgwszTableType[] = {L"ALIAS", L"TABLE", L"SYNONYM", L"SYSTEM TABLE", L"VIEW", L"GLOBAL TEMPORARY",
	L"LOCAL TEMPORARY", L"SYSTEM VIEW"};

enum TABLES_ENUM
{
	TABLES_TABLE_CATALOG = 1,
	TABLES_TABLE_SCHEMA,
	TABLES_TABLE_NAME,
	TABLES_TABLE_TYPE,
	TABLES_TABLE_GUID,
	TABLES_DESCRIPTION,
	TABLES_TABLE_PROPID,
	TABLES_DATE_CREATED,
	TABLES_DATE_MODIFIED,
};

ULONG rgTablesRestrict[] = {TABLES_TABLE_CATALOG, TABLES_TABLE_SCHEMA, TABLES_TABLE_NAME, TABLES_TABLE_TYPE};

ULONG rgTablesSort[] = {TABLES_TABLE_TYPE, TABLES_TABLE_CATALOG, TABLES_TABLE_SCHEMA, TABLES_TABLE_NAME};
RESTRICT_CAT rgTablesCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_TABLE_TYPE};

ColList TablesCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,				FALSE,
	L"TABLE_TYPE",					DBTYPE_WSTR,	NUMELEM(rgwszTableType),rgwszTableType,		FALSE,
	L"TABLE_GUID",					DBTYPE_GUID,	0,						NULL,				TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_PROPID",				DBTYPE_UI4,		0,						NULL,				TRUE,
	L"DATE_CREATED",				DBTYPE_DATE,	0,						NULL,				TRUE,
	L"DATE_MODIFIED",				DBTYPE_DATE,	0,						NULL,				TRUE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_TABLES_INFO
//-----------------------------------------------------------------------------
LONG rgBookMarkTypeVals[] = {DBPROPVAL_BMK_NUMERIC, DBPROPVAL_BMK_KEY};

enum TABLES_INFO_ENUM
{
	TIS_TABLE_CATALOG = 1,
	TIS_TABLE_SCHEMA,
	TIS_TABLE_NAME,
	TIS_TABLE_TYPE,
	TIS_TABLE_GUID,
	TIS_BOOKMARKS,
	TIS_BOOKMARK_TYPE,
	TIS_BOOKMARK_DATATYPE,
	TIS_BOOKMARK_MAXIMUM_LENGTH,
	TIS_BOOKMARK_INFORMATION,
	TIS_TABLE_VERSION,
	TIS_CARDINALITY,
	TIS_DESCRIPTION,
	TIS_TABLE_PROPID,
};

ULONG rgTablesInfoRestrict[] = {TIS_TABLE_CATALOG, TIS_TABLE_SCHEMA, TIS_TABLE_NAME, TIS_TABLE_TYPE};

ULONG rgTablesInfoSort[] = {TIS_TABLE_TYPE, TIS_TABLE_CATALOG, TIS_TABLE_SCHEMA, TIS_TABLE_NAME};
RESTRICT_CAT rgTablesInfoCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE, RC_TABLE_TYPE};
ULONG rgTablesInfoCount[VER_MAX] = {TIS_TABLE_PROPID};

ColList TablesInfoCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,				FALSE,
	L"TABLE_TYPE",					DBTYPE_WSTR,	NUMELEM(rgwszTableType),rgwszTableType,		FALSE,
	L"TABLE_GUID",					DBTYPE_GUID,	0,						NULL,				TRUE,
	L"BOOKMARKS",					DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,			FALSE,
	L"BOOKMARK_TYPE",				DBTYPE_I4,		NUMELEM(rgBookMarkTypeVals),rgBookMarkTypeVals, TRUE,
	L"BOOKMARK_DATATYPE",			DBTYPE_UI2,		0,						NULL,				TRUE,
	L"BOOKMARK_MAXIMUM_LENGTH",		DBTYPE_UI4,		0,						NULL,				TRUE,
	L"BOOKMARK_INFORMATION",		DBTYPE_UI4,		0,						NULL,				TRUE,
	L"TABLE_VERSION",				DBTYPE_I8,		0,						NULL,				TRUE,
	L"CARDINALITY",					DBTYPE_UI8,		0,						NULL,				TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_PROPID",				DBTYPE_UI4,		0,						NULL,				TRUE,
};


//-----------------------------------------------------------------------------
// DBSCHEMA_TRUSTEE
//-----------------------------------------------------------------------------
ULONG rgTrusteeTypeVals[] = {TRUSTEE_IS_UNKNOWN, TRUSTEE_IS_USER, TRUSTEE_IS_GROUP};

enum TRUSTEE_ENUM
{
	TRS_TRUSTEE_NAME = 1,
	TRS_TRUSTEE_GUID,
	TRS_TRUSTEE_PROPID,
	TRS_TRUSTEE_TYPE,
};

ULONG rgTrusteeRestrict[] = {TRS_TRUSTEE_NAME, TRS_TRUSTEE_GUID, TRS_TRUSTEE_PROPID, TRS_TRUSTEE_TYPE};

ULONG rgTrusteeSort[] = {TRS_TRUSTEE_NAME, TRS_TRUSTEE_GUID, TRS_TRUSTEE_PROPID, TRS_TRUSTEE_TYPE};
RESTRICT_CAT rgTrusteeCat[]= {RC_USER, RC_UND, RC_UND, RC_UND};

ColList TrusteeCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"TRUSTEE_NAME",				DBTYPE_WSTR,	0,						NULL,				FALSE,
	L"TRUSTEE_GUID",				DBTYPE_GUID,	0,						NULL,				TRUE,
	L"TRUSTEE_PROPID",				DBTYPE_UI4,		0,						NULL,				TRUE,
	L"TRUSTEE_TYPE",				DBTYPE_UI4,		NUMELEM(rgTrusteeTypeVals),	rgTrusteeTypeVals,	FALSE,
};

//-----------------------------------------------------------------------------
// DBSCHEMA_VIEWS
//-----------------------------------------------------------------------------
enum VIEWS_ENUM
{
	VS_TABLE_CATALOG = 1,
	VS_TABLE_SCHEMA,
	VS_TABLE_NAME,
	VS_VIEW_DEFINITION,
	VS_CHECK_OPTION,
	VS_IS_UPDATABLE,
	VS_DESCRIPTION,
	VS_DATE_CREATED,
	VS_DATE_MODIFIED,
};

ULONG rgViewsRestrict[] = {VS_TABLE_CATALOG, VS_TABLE_SCHEMA, VS_TABLE_NAME};
ULONG rgViewsSort[] = {VS_TABLE_CATALOG, VS_TABLE_SCHEMA, VS_TABLE_NAME};
RESTRICT_CAT rgViewsCat[]= {RC_CATALOG, RC_SCHEMA, RC_TABLE};

ColList ViewsCols[] = 
{
	// Name							Type			cValidVals				pValidVals			fAllowNull
	L"TABLE_CATALOG",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_SCHEMA",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"TABLE_NAME",					DBTYPE_WSTR,	0,						NULL,				FALSE,
	L"VIEW_DEFINITION",				DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"CHECK_OPTION",				DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,			TRUE,
	L"IS_UPDATABLE",				DBTYPE_BOOL,	NUMELEM(rgBoolList),	rgBoolList,			TRUE,
	L"DESCRIPTION",					DBTYPE_WSTR,	0,						NULL,				TRUE,
	L"DATE_CREATED",				DBTYPE_DATE,	0,						NULL,				TRUE,
	L"DATE_MODIFIED",				DBTYPE_DATE,	0,						NULL,				TRUE,
};

//-----------------------------------------------------------------------------
// List of all schemas
//-----------------------------------------------------------------------------

SchemaList AllSchemas[] = 
{
	&DBSCHEMA_ASSERTIONS,
		L"DBSCHEMA_ASSERTIONS",
		CRESTRICTIONS_DBSCHEMA_ASSERTIONS,
		rgAssertionsRestrict,
		rgAssertionsCat,
		NUMELEM(AssertionsCols),
		(ColList *)&AssertionsCols,
		NUMELEM(rgAssertionsSort),
		rgAssertionsSort,
		NULL,

	&DBSCHEMA_CATALOGS,
		L"DBSCHEMA_CATALOGS",
 		CRESTRICTIONS_DBSCHEMA_CATALOGS,
		rgCatalogsRestrict,
		rgCatalogsCat,
		NUMELEM(CatalogsCols),
		(ColList *)&CatalogsCols,
		NUMELEM(rgCatalogsSort),
		rgCatalogsSort,
		NULL,

	&DBSCHEMA_CHARACTER_SETS,
		L"DBSCHEMA_CHARACTER_SETS",
		CRESTRICTIONS_DBSCHEMA_CHARACTER_SETS,
		rgCharacterSetsRestrict,
		rgCharacterSetsCat,
		NUMELEM(CharacterSetsCols),
		(ColList *)&CharacterSetsCols,
		NUMELEM(rgCharacterSetsSort),
		rgCharacterSetsSort,
		NULL,

	&DBSCHEMA_CHECK_CONSTRAINTS,
		L"DBSCHEMA_CHECK_CONSTRAINTS",
		CRESTRICTIONS_DBSCHEMA_CHECK_CONSTRAINTS,
		rgCheckConstraintsRestrict,
		rgCheckConstraintsCat,
		NUMELEM(CheckConstraintsCols),
		(ColList *)&CheckConstraintsCols,
		NUMELEM(rgCheckConstraintsSort),
		rgCheckConstraintsSort,
		NULL,

	&DBSCHEMA_COLLATIONS,
		L"DBSCHEMA_COLLATIONS",
		CRESTRICTIONS_DBSCHEMA_COLLATIONS,
		rgCollationsRestrict,
		rgCollationsCat,
		NUMELEM(CollationsCols),
		(ColList *)&CollationsCols,
		NUMELEM(rgCollationsSort),
		rgCollationsSort,
		NULL,

	&DBSCHEMA_COLUMN_DOMAIN_USAGE,
		L"DBSCHEMA_COLUMN_DOMAIN_USAGE",
		CRESTRICTIONS_DBSCHEMA_COLUMN_DOMAIN_USAGE,
		rgColumnDomainUsageRestrict,
		rgColumnDomainUsageCat,
		NUMELEM(ColumnDomainUsageCols),
		(ColList *)&ColumnDomainUsageCols,
		NUMELEM(rgColumnDomainUsageSort),
		rgColumnDomainUsageSort,
		NULL,

	&DBSCHEMA_COLUMN_PRIVILEGES,
		L"DBSCHEMA_COLUMN_PRIVILEGES",
		CRESTRICTIONS_DBSCHEMA_COLUMN_PRIVILEGES,
		rgColumnPrivilegesRestrict,
		rgColumnPrivilegesCat,
		NUMELEM(ColumnPrivilegesCols),
		(ColList *)&ColumnPrivilegesCols,
		NUMELEM(rgColumnPrivilegesSort),
		rgColumnPrivilegesSort,
		NULL,

	&DBSCHEMA_COLUMNS,
		L"DBSCHEMA_COLUMNS",
		CRESTRICTIONS_DBSCHEMA_COLUMNS,
		rgColumnsRestrict,
		rgColumnsCat,
		NUMELEM(ColumnsCols),
		(ColList *)&ColumnsCols,
		NUMELEM(rgColumnsSort),
		rgColumnsSort,
		rgColumnsCount,

	&DBSCHEMA_CONSTRAINT_COLUMN_USAGE,
		L"DBSCHEMA_CONSTRAINT_COLUMN_USAGE",
		NUMELEM(rgConstraintColumnUsageRestrict),	// CRESTRICTIONS_DBSCHEMA_CONSTRAINT_COLUMN_USAGE,	// CRESTRICTIONS is 4, but there are 7 restrictions
		rgConstraintColumnUsageRestrict,
		rgConstraintColumnUsageCat,
		NUMELEM(ConstraintColumnUsageCols),
		(ColList *)&ConstraintColumnUsageCols,
		NUMELEM(rgConstraintColumnUsageSort),
		rgConstraintColumnUsageSort,
		NULL,
/*
	&DBSCHEMA_CONSTRAINT_TABLE_USAGE,
		L"DBSCHEMA_CONSTRAINT_TABLE_USAGE",
		CRESTRICTIONS_DBSCHEMA_CONSTRAINT_TABLE_USAGE,
		rgConstraintTableRestrict,
		rgConstraintTableUsageCat,
		NUMELEM(ConstraintTableCols),
		(ColList *)&ConstraintTableCols,
		NUMELEM(rgConstraintTablesSort),
		rgConstraintTablesSort,
		NULL,
*/
	&DBSCHEMA_KEY_COLUMN_USAGE,
		L"DBSCHEMA_KEY_COLUMN_USAGE",
		CRESTRICTIONS_DBSCHEMA_KEY_COLUMN_USAGE,
		rgKeyColumnUsageRestrict,
		rgKeyColumnUsageCat,
		NUMELEM(KeyColumnUsageCols),
		(ColList *)&KeyColumnUsageCols,
		NUMELEM(rgKeyColumnUsageSort),
		rgKeyColumnUsageSort,
		NULL,

	&DBSCHEMA_REFERENTIAL_CONSTRAINTS,
		L"DBSCHEMA_REFERENTIAL_CONSTRAINTS",
		CRESTRICTIONS_DBSCHEMA_REFERENTIAL_CONSTRAINTS,
		rgReferentialConstraintsRestrict,
		rgReferentialConstraintsCat,
		NUMELEM(ReferentialConstraintsCols),
		(ColList *)&ReferentialConstraintsCols,
		NUMELEM(rgReferentialConstraintsSort),
		rgReferentialConstraintsSort,
		NULL,

	&DBSCHEMA_INDEXES,
		L"DBSCHEMA_INDEXES",
		CRESTRICTIONS_DBSCHEMA_INDEXES,
		rgIndexesRestrict,
		rgIndexesCat,
		NUMELEM(IndexesCols),
		(ColList *)&IndexesCols,
		NUMELEM(rgIndexesSort),
		rgIndexesSort,
		NULL,

	// Two unknown schemas in header???
	// DBSCHEMA_OBJECT_ACTIONS	
	// DBSCHEMA_OBJECTS


	&DBSCHEMA_COLUMN_PRIVILEGES,
		L"DBSCHEMA_COLUMN_PRIVILEGES",
		CRESTRICTIONS_DBSCHEMA_COLUMN_PRIVILEGES,
		rgColumnPrivilegesRestrict,
		rgColumnPrivilegesCat,
		NUMELEM(ColumnPrivilegesCols),
		(ColList *)&ColumnPrivilegesCols,
		NUMELEM(rgColumnPrivilegesSort),
		rgColumnPrivilegesSort,
		NULL,

	&DBSCHEMA_TABLE_PRIVILEGES,
		L"DBSCHEMA_TABLE_PRIVILEGES",
		CRESTRICTIONS_DBSCHEMA_TABLE_PRIVILEGES,
		rgTablePrivilegesRestrict,
		rgTablePrivilegesCat,
		NUMELEM(TablePrivilegesCols),
		(ColList *)&TablePrivilegesCols,
		NUMELEM(rgTablePrivilegesSort),
		rgTablePrivilegesSort,
		NULL,
/*
	&DBSCHEMA_USAGE_PRIVILEGES,
		L"DBSCHEMA_USAGE_PRIVILEGES",
		CRESTRICTIONS_DBSCHEMA_USAGE_PRIVILEGES,
		rgUsagePrivilegesRestrict,
		rgUsagePrivilegesCat,
		NUMELEM(UsagePrivilegesCols),
		(ColList *)&UsagePrivilegesCols,
		NUMELEM(rgUsagePrivilegesSort),
		rgUsagePrivilegesSort,
		NULL,
*/
	&DBSCHEMA_PROCEDURES,
		L"DBSCHEMA_PROCEDURES",
		CRESTRICTIONS_DBSCHEMA_PROCEDURES,
		rgProceduresRestrict,
		rgProceduresCat,
		NUMELEM(ProceduresCols),
		(ColList *)&ProceduresCols,
		NUMELEM(rgProceduresSort),
		rgProceduresSort,
		NULL,

	&DBSCHEMA_SCHEMATA,
		L"DBSCHEMA_SCHEMATA",
		CRESTRICTIONS_DBSCHEMA_SCHEMATA,
		rgSchemataRestrict,
		rgSchemataCat,
		NUMELEM(SchemataCols),
		(ColList *)&SchemataCols,
		NUMELEM(rgSchemataSort),
		rgSchemataSort,
		NULL,
/*
	&DBSCHEMA_SQL_LANGUAGES,
		L"DBSCHEMA_SQL_LANGUAGES",
		CRESTRICTIONS_DBSCHEMA_SQL_LANGUAGES,
		rgSQLLanguagesRestrict,
		rgSQLLanguagesCat,
		NUMELEM(SQLLanguagesCols),
		(ColList *)&SQLLanguagesCols,
		NUMELEM(rgSQLLanguagesSort),
		rgSQLLanguagesSort,
		NULL,
*/
	&DBSCHEMA_STATISTICS,
		L"DBSCHEMA_STATISTICS",
		CRESTRICTIONS_DBSCHEMA_STATISTICS,
		rgStatisticsRestrict,
		rgStatisticsCat,
		NUMELEM(StatisticsCols),
		(ColList *)&StatisticsCols,
		NUMELEM(rgStatisticsSort),
		rgStatisticsSort,
		NULL,

	&DBSCHEMA_TABLE_CONSTRAINTS,
		L"DBSCHEMA_TABLE_CONSTRAINTS",
		CRESTRICTIONS_DBSCHEMA_TABLE_CONSTRAINTS,
		rgTableConstraintsRestrict,
		rgTableConstraintsCat,
		NUMELEM(TableConstraintsCols),
		(ColList *)&TableConstraintsCols,
		NUMELEM(rgTableConstraintsSort),
		rgTableConstraintsSort,
		NULL,

	&DBSCHEMA_TABLES,
		L"DBSCHEMA_TABLES",
		CRESTRICTIONS_DBSCHEMA_TABLES,
		rgTablesRestrict,
		rgTablesCat,
		NUMELEM(TablesCols),
		(ColList *)&TablesCols,
		NUMELEM(rgTablesSort),
		rgTablesSort,
		NULL,

/*
	&DBSCHEMA_TRANSLATIONS,
		L"DBSCHEMA_TRANSLATIONS",
		CRESTRICTIONS_DBSCHEMA_TRANSLATIONS,
		rgTranslationsRestrict,
		rgTranslationsCat,
		NUMELEM(TranslationsCols),
		(ColList *)&TranslationsCols,
		NUMELEM(rgTranslationsSort),
		rgTranslationsSort,
		NULL,
*/
	&DBSCHEMA_PROVIDER_TYPES,
		L"DBSCHEMA_PROVIDER_TYPES",
		CRESTRICTIONS_DBSCHEMA_PROVIDER_TYPES,
		rgProviderTypesRestrict,
		rgProviderTypesCat,
		NUMELEM(ProviderTypesCols),
		(ColList *)&ProviderTypesCols,
		NUMELEM(rgProviderTypesSort),
		rgProviderTypesSort,
		NULL,

	&DBSCHEMA_VIEWS,
		L"DBSCHEMA_VIEWS",
		CRESTRICTIONS_DBSCHEMA_VIEWS,
		rgViewsRestrict,
		rgViewsCat,
		NUMELEM(ViewsCols),
		(ColList *)&ViewsCols,
		NUMELEM(rgViewsSort),
		rgViewsSort,
		NULL,
/*
	&DBSCHEMA_VIEW_COLUMN_USAGE,
		L"DBSCHEMA_VIEW_COLUMN_USAGE",
		CRESTRICTIONS_DBSCHEMA_VIEW_COLUMN_USAGE,
		rgViewColumnUsageRestrict,
		rgViewColumnUsageCat,
		NUMELEM(ViewColumnUsageCols),
		(ColList *)&ViewColumnUsageCols,
		NUMELEM(rgViewColumnUsageSort),
		rgViewColumnUsageSort,
		NULL,

	&DBSCHEMA_VIEW_TABLE_USAGE,
		L"DBSCHEMA_VIEW_TABLE_USAGE",
		CRESTRICTIONS_DBSCHEMA_VIEW_TABLE_USAGE,
		rgViewTableUsageRestrict,
		rgViewTableUsageCat,
		NUMELEM(ViewTableUsageCols),
		(ColList *)&ViewTableUsageCols,
		NUMELEM(rgViewTableUsageSort),
		rgViewTableUsageSort,
		NULL,
*/
	&DBSCHEMA_FOREIGN_KEYS,
		L"DBSCHEMA_FOREIGN_KEYS",
		CRESTRICTIONS_DBSCHEMA_FOREIGN_KEYS,
		rgForeignKeysRestrict,
		rgForeignKeysCat,
		NUMELEM(ForeignKeysCols),
		(ColList *)&ForeignKeysCols,
		NUMELEM(rgForeignKeysSort),
		rgForeignKeysSort,
		NULL,

	&DBSCHEMA_PRIMARY_KEYS,
		L"DBSCHEMA_PRIMARY_KEYS",
		CRESTRICTIONS_DBSCHEMA_PRIMARY_KEYS,
		rgPrimaryKeysRestrict,
		rgPrimaryKeysCat,
		NUMELEM(PrimaryKeysCols),
		(ColList *)&PrimaryKeysCols,
		NUMELEM(rgPrimaryKeysSort),
		rgPrimaryKeysSort,
		NULL,

	&DBSCHEMA_PROCEDURE_COLUMNS,
		L"DBSCHEMA_PROCEDURE_COLUMNS",
		CRESTRICTIONS_DBSCHEMA_PROCEDURE_COLUMNS,
		rgProcedureColumnsRestrict,
		rgProcedureColumnsCat,
		NUMELEM(ProcedureColumnsCols),
		(ColList *)&ProcedureColumnsCols,
		NUMELEM(rgProcedureColumnsSort),
		rgProcedureColumnsSort,
		NULL,

	&DBSCHEMA_PROCEDURE_PARAMETERS,
		L"DBSCHEMA_PROCEDURE_PARAMETERS",
		CRESTRICTIONS_DBSCHEMA_PROCEDURE_PARAMETERS,
		rgProcedureParametersRestrict,
		rgProcedureParametersCat,
		NUMELEM(ProcedureParametersCols),
		(ColList *)&ProcedureParametersCols,
		NUMELEM(rgProcedureParametersSort),
		rgProcedureParametersSort,
		rgProcedureParametersCount,

	&DBSCHEMA_TABLES_INFO,
		L"DBSCHEMA_TABLES_INFO",
		CRESTRICTIONS_DBSCHEMA_TABLES_INFO,
		rgTablesInfoRestrict,
		rgTablesInfoCat,
		NUMELEM(TablesInfoCols),
		(ColList *)&TablesInfoCols,
		NUMELEM(rgTablesInfoSort),
		rgTablesInfoSort,
		rgTablesInfoCount,

	&DBSCHEMA_TRUSTEE,
		L"DBSCHEMA_TRUSTEE",
		CRESTRICTIONS_DBSCHEMA_TRUSTEE,
		rgTrusteeRestrict,
		rgTrusteeCat,
		NUMELEM(TrusteeCols),
		(ColList *)&TrusteeCols,
		NUMELEM(rgTrusteeSort),
		rgTrusteeSort,
		NULL,

	&DBSCHEMA_TABLE_STATISTICS,
		L"DBSCHEMA_TABLE_STATISTICS",
		CRESTRICTIONS_DBSCHEMA_TABLE_STATISTICS,
		rgTableStatisticsRestrict,
		rgTableStatisticsCat,
		NUMELEM(TableStatisticsCols),
		(ColList *)&TableStatisticsCols,
		NUMELEM(rgTableStatisticsSort),
		rgTableStatisticsSort,
		NULL,

	&DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE,
		L"DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE",
		CRESTRICTIONS_DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE,
		rgCheckConstraintsByTableRestrict,
		rgCheckConstraintsByTableCat,
		NUMELEM(CheckConstraintsByTableCols),
		(ColList *)&CheckConstraintsByTableCols,
		NUMELEM(rgCheckConstraintsByTableSort),
		rgCheckConstraintsByTableSort,
		NULL,
};


//-----------------------------------------------------------------------------
// Histogram Rowset info
//-----------------------------------------------------------------------------
enum HR_ENUM	// Histogram rowset column ordinals
{
	HR_RANGE_HI_KEY = 1,
	HR_RANGE_ROWS,
	HR_EQ_ROWS,
	HR_DISTINCT_RANGE_ROWS
};

ULONG rgHistogramSort[] = {HR_RANGE_HI_KEY};
ULONG rgHistogramCount[VER_MAX] = {HR_DISTINCT_RANGE_ROWS};

ColList HistogramCols[] = 
{
	// Name					Type			cValidVals			pValidVals	fAllowNull
	L"RANGE_HI_KEY",		DBTYPE_EMPTY,	0,					NULL,		FALSE,
	L"RANGE_ROWS",			DBTYPE_R8,		0,					NULL,		TRUE,
	L"EQ_ROWS",				DBTYPE_R8,		0,					NULL,		TRUE,
	L"DISTINCT_RANGE_ROWS",	DBTYPE_I8,		0,					NULL,		TRUE,
};

SchemaList HistogramRowset[] = 
{
	&GUID_NULL,
		L"",
		0,
		NULL,
		NULL,
		NUMELEM(HistogramCols),
		(ColList *)&HistogramCols,
		NUMELEM(rgHistogramSort),
		rgHistogramSort,
		rgHistogramCount
};

#endif 	//_IDBSCHMR_H_


