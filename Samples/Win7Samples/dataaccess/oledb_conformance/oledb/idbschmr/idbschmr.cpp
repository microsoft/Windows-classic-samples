//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IDBSCHMR.CPP | Source file for IDBSchemaRowset test
//
//--------------------------------------------------------------------

#include "modstandard.hpp"
#include "IDBSchmR.h"
#include "extralib.h"

#define CLEANUP(x) if(x) goto CLEANUP;
#define TERMINATE(x) if(x) return FALSE;
#define CONTINUE(x) if (x) return TRUE;

#define	PI	(IUnknown *)
#define PPI (IUnknown **)
#define PPPI (IUnknown ***)
#define WC (WCHAR *)

#define INIT Init_Stuff();
#define FREE Free_Stuff();

// free other pointer
#define RESTRICTNOTSUPPORTED(x) if(!(m_currentBitMask & x))m_fAtLeast1UnsupportedRestrictionIsSet=TRUE;


IDBSchemaRowset * g_pIDBSchemaRowset=NULL;

BOOL	g_fKagera;					// Track whether running against Kagera
BOOL	g_fSQLServer;				// Track whether the backend is SQL Server

// Initially use Sql Server provider specific guid and prop value
GUID	g_guidHistogramRowset = DBGUID_HISTOGRAM_ROWSET;
DBPROPID g_propTableStatistics = DBPROP_TABLESTATISTICS;

ULONG cSchemas;
GUID * rgSchemas;
ULONG * rgRestrictions=NULL;

// Global strings for primary and foreign keys.
WCHAR *g_pwszAddPrimaryKeyOnTable1=NULL;
WCHAR *g_pwszAddPrimaryKeyOnTable2=NULL;
WCHAR *g_pwszAddForeignKeyOnTable1=NULL;
WCHAR *g_pwszDropPrimaryKeyConstraint1=NULL;
WCHAR *g_pwszDropForeignKeyConstraint1=NULL;
WCHAR *g_pwszDropPrimaryKeyConstraint2=NULL;
CTable	*g_pKeyTable1=NULL;
CTable  *g_pKeyTable2=NULL;
BOOL	g_fKeysOnTable=FALSE;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CErrorCache
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define CCHECK(ErrObj, hrAct, hrExp, ulErr, pwszMsg, fWarn)  (ErrObj.CCheck((hrAct), \
	(hrExp), (ulErr), (pwszMsg), (fWarn), LONGSTRING(__FILE__), (__LINE__)))
#define CCOMPARE(ErrObj, fResult, ulErr, pwszMsg, fWarn)  (ErrObj.CCompare((fResult), \
	(ulErr), (pwszMsg), (fWarn), LONGSTRING(__FILE__), (__LINE__)))

enum ERROR_CACHE_ENUM
{
	// DBSCHEMA_TABLE_STATISTICS cached errors
	EC_EXTRA_COLUMN = 1,
	EC_BAD_COL_OR_TUPLE_CARD,
	EC_BAD_TABLE_CARDINALITY,
	EC_BAD_HR_OPENHISTOGRAM,
	EC_NULL_RANGE_COUNT,
	EC_BAD_RANGE_ROWS,
	EC_BAD_EQ_ROWS,
	EC_BAD_DISTINCT_RANGE_ROWS,
	// DBSCHEMA_COLUMNS cached errors
	EC_BAD_COLUMN_FLAGS,
	EC_BAD_CHARACTER_MAXIMUM_LENGTH,
	EC_BAD_CHARACTER_OCTET_LENGTH,
	EC_BAD_DATETIME_PRECISION,
	EC_COLUMNS_NULL_NUMERIC_SCALE,
	EC_COLUMNS_BAD_NUMERIC_PRECISION,
	// DBSCHEMA_INDEXES cached errors
	EC_INDEXES_NULLS_ISNULL,
	EC_INDEXES_INDEX_NAME_ISNULL,
	EC_INDEXES_UNIQUE_ISNULL,
	EC_INDEXES_AUTO_UPDATE_ISNULL,
	// DBSCHEMA_PROCEDURES cached errors
	EC_PROCEDURES_PROC_DEF_ISNULL,
	// DBSCHEMA_PROCEDURE_COLUMNS cached errors
	EC_PROC_COLS_IS_NULLABLE_ISNULL,
	EC_INVALID_IS_NULLABLE,
	// DBSCHEMA_PROVIDER_TYPES cached errors
	EC_INCORRECT_SORT,
	EC_MAX_ERROR_NUMBER			// Must always be last enum
};

LPWSTR g_ppwszErrorStrings[] = 
{
	// DBSCHEMA_TABLE_STATISTICS cached errors
	L"EC_EXTRA_COLUMN",
	L"EC_BAD_COL_OR_TUPLE_CARD",
	L"EC_BAD_TABLE_CARDINALITY",
	L"EC_BAD_HR_OPENHISTOGRAM",
	L"EC_NULL_RANGE_COUNT",
	L"EC_BAD_RANGE_ROWS",
	L"EC_BAD_EQ_ROWS",
	L"EC_BAD_DISTINCT_RANGE_ROWS",
	// DBSCHEMA_COLUMNS cached errors
	L"EC_BAD_COLUMN_FLAGS",
	L"EC_BAD_CHARACTER_MAXIMUM_LENGTH",
	L"EC_BAD_CHARACTER_OCTET_LENGTH",
	L"EC_BAD_DATETIME_PRECISION",
	L"EC_COLUMNS_NULL_NUMERIC_SCALE",
	L"EC_COLUMNS_BAD_NUMERIC_PRECISION",
	// DBSCHEMA_INDEXES cached errors
	L"EC_INDEXES_NULLS_ISNULL",
	L"EC_INDEXES_INDEX_NAME_ISNULL",
	L"EC_INDEXES_UNIQUE_ISNULL",
	L"EC_INDEXES_AUTO_UPDATE_ISNULL",
	// DBSCHEMA_PROCEDURES cached errors
	L"EC_PROCEDURES_PROC_DEF_ISNULL",
	// DBSCHEMA_PROCEDURE_COLUMNS cached errors
	L"EC_PROC_COLS_IS_NULLABLE_ISNULL",
	L"EC_INVALID_IS_NULLABLE",
	// DBSCHEMA_PROVIDER_TYPES cached errors
	L"EC_INCORRECT_SORT",
};


class CErrorCache
{

private:

	ULONG m_ulDebugMode;
	ULONG m_ulMaxCachedErrors;
	ULONG * m_pulErrorCache;
	ULONG m_cErrorsCached;

	BOOL IsErrorCached(ULONG ulError);

public:

	CErrorCache(void);

	~CErrorCache(void);

	BOOL Init(ULONG ulDebugMode);

	BOOL CCheck(HRESULT hrActual, HRESULT hrExpected, ULONG ulError = 0,
		LPWSTR pwszMessage = NULL, BOOL fWarning = FALSE, LPWSTR pwszFile = NULL,
		ULONG ulLine = 0);
	
	BOOL CCompare(BOOL fResult, ULONG ulError = 0, LPWSTR pwszMessage = NULL,
		BOOL fWarning = FALSE, LPWSTR pwszFile = NULL, ULONG ulLine = 0);
};

CErrorCache::CErrorCache(void)
{
	m_ulDebugMode = 0;
	m_ulMaxCachedErrors = EC_MAX_ERROR_NUMBER-1;
	m_pulErrorCache = NULL;
	m_cErrorsCached = 0;
}

CErrorCache::~CErrorCache(void)
{
	// Print summary of errors cached if there is at least one
	// error cached.
	if (m_cErrorsCached)
	{
		odtLog << L"\n\nThe following errors were cached:\n\n";

		for (ULONG iErr = 0; iErr < m_ulMaxCachedErrors; iErr++)
			if (m_pulErrorCache[iErr])
				odtLog << L"\t" << g_ppwszErrorStrings[iErr] <<
				L"\t\t" << m_pulErrorCache[iErr] << L"\n";

		odtLog << L"\n\n";
		odtLog << L"To see error details instead of this summary please add 'DEBUGMODE=FULL;' to the init string.\n\n";

	}

	SAFE_FREE(m_pulErrorCache);
}

BOOL CErrorCache::Init(ULONG ulDebugMode)
{
	BOOL fResult = FALSE;

	m_ulDebugMode = ulDebugMode;

	// Allocate memory for error cache.  Since we don't expect this to be more than
	// a few 10's of items just use a static array.
	SAFE_ALLOC(m_pulErrorCache, ULONG, m_ulMaxCachedErrors);

	// Init all cache locations to 0, no error cached
	memset(m_pulErrorCache, 0, m_ulMaxCachedErrors*sizeof(ULONG));

	fResult = TRUE;

CLEANUP:

	return fResult;
}

BOOL CErrorCache::IsErrorCached(ULONG ulError)
{
	ASSERT(ulError <= m_ulMaxCachedErrors);

	// In case this gets called before init.
	if (!m_pulErrorCache)
		return FALSE;

	// If the cache has a value other than 0 then it's been cached
	return m_pulErrorCache[ulError-1];
}

BOOL CErrorCache::CCheck(HRESULT hrActual, HRESULT hrExpected, ULONG ulError,
		LPWSTR pwszMessage, BOOL fWarning, LPWSTR pwszFile,
		ULONG ulLine)
{
	BOOL fReturn = hrActual == hrExpected;

	// Check for valid error number
	if (ulError == 0 || ulError > m_ulMaxCachedErrors)
		return FALSE;

	// If this error is already cached then we allow it to pass unless debugmode is full.
	if (IsErrorCached(ulError) && !(m_ulDebugMode & DEBUGMODE_FULL))
	{
		if (!fReturn)
			// Update the cache
			m_pulErrorCache[ulError-1]++;
		return fReturn;
	}

	// Otherwise we have to perform the comparison.  Note we can't just return the
	// value from PrivLibValidate because on warning it always returns TRUE even on
	// a miscompare
	PrivlibValidate(hrActual, hrExpected, fWarning, pwszFile, ulLine);

	// If the comparison failed, then print failure message
	if (!fReturn)
	{
		// Update the cache
		if (!(m_ulDebugMode & DEBUGMODE_FULL))
		{
			m_pulErrorCache[ulError-1]++;
			m_cErrorsCached++;
		}

		if (pwszMessage)
			odtLog << pwszMessage << L"\n";
	}

	return fReturn;
}

BOOL CErrorCache::CCompare(BOOL fResult, ULONG ulError, LPWSTR pwszMessage,
		BOOL fWarning, LPWSTR pwszFile, ULONG ulLine)
{

	// Check for valid error number
	if (ulError == 0 || ulError > m_ulMaxCachedErrors)
		return FALSE;

	// If this error is already cached then we allow it to pass unless debugmode is full.
	if (IsErrorCached(ulError) && !(m_ulDebugMode & DEBUGMODE_FULL))
	{
		if (!fResult)
			// Update the cache
			m_pulErrorCache[ulError-1]++;
		return fResult;
	}

	// Otherwise we have to perform the comparison.  Note we can't just return the
	// value from PrivLibValidate because on warning it always returns TRUE even on
	// a miscompare
	PrivlibCompare(fResult, fWarning, pwszFile, ulLine);

	// If the comparison failed, then print failure message
	if (!fResult)
	{
		if (!(m_ulDebugMode & DEBUGMODE_FULL))
		{
			m_pulErrorCache[ulError-1]++;
			m_cErrorsCached++;
		}

		if (pwszMessage)
			odtLog << pwszMessage << L"\n";
	}

	return fResult;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TraceSchemaName
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ULONG TraceSchemaName
(
	GUID guid,
	BOOL fToScreen,
	BOOL fAddNewLine
)
{
	LPWSTR pwszSchemaName = L"";

	// Find the schema
	if(IsEqualGUID(DBSCHEMA_ASSERTIONS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_ASSERTIONS;
	else if(IsEqualGUID(DBSCHEMA_CATALOGS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_CATALOGS;
	else if(IsEqualGUID(DBSCHEMA_CHARACTER_SETS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_CHARACTER_SETS;
	else if(IsEqualGUID(DBSCHEMA_CHECK_CONSTRAINTS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_CHECK_CONSTRAINTS;
	else if(IsEqualGUID(DBSCHEMA_COLLATIONS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_COLLATIONS;
	else if(IsEqualGUID(DBSCHEMA_COLUMN_DOMAIN_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_COLUMN_DOMAIN_USAGE;
	else if(IsEqualGUID(DBSCHEMA_COLUMN_PRIVILEGES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_COLUMN_PRIVILEGES;
	else if(IsEqualGUID(DBSCHEMA_COLUMNS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_COLUMNS;
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_COLUMN_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_CONSTRAINT_COLUMN_USAGE;
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_TABLE_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_CONSTRAINT_TABLE_USAGE;
	else if(IsEqualGUID(DBSCHEMA_FOREIGN_KEYS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_FOREIGN_KEYS;
	else if(IsEqualGUID(DBSCHEMA_INDEXES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_INDEXES;
	else if(IsEqualGUID(DBSCHEMA_KEY_COLUMN_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_KEY_COLUMN_USAGE;
	else if(IsEqualGUID(DBSCHEMA_PRIMARY_KEYS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_PRIMARY_KEYS;
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_COLUMNS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_PROCEDURE_COLUMNS;
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_PARAMETERS, guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_PROCEDURE_PARAMETERS;
	else if(IsEqualGUID(DBSCHEMA_PROCEDURES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_PROCEDURES;
	else if(IsEqualGUID(DBSCHEMA_PROVIDER_TYPES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_PROVIDER_TYPES;
	else if(IsEqualGUID(DBSCHEMA_REFERENTIAL_CONSTRAINTS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_REFERENTIAL_CONSTRAINTS;
	else if(IsEqualGUID(DBSCHEMA_SCHEMATA,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_SCHEMATA;
	else if(IsEqualGUID(DBSCHEMA_SQL_LANGUAGES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_SQL_LANGUAGES;
	else if(IsEqualGUID(DBSCHEMA_STATISTICS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_STATISTICS;
	else if(IsEqualGUID(DBSCHEMA_TABLES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TABLES;
	else if(IsEqualGUID(DBSCHEMA_TABLES_INFO,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TABLES_INFO;
	else if(IsEqualGUID(DBSCHEMA_TABLE_CONSTRAINTS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TABLE_CONSTRAINTS;
	else if(IsEqualGUID(DBSCHEMA_TABLE_PRIVILEGES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TABLE_PRIVILEGES;
	else if(IsEqualGUID(DBSCHEMA_TABLE_STATISTICS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TABLE_STATISTICS;
	else if(IsEqualGUID(DBSCHEMA_TRANSLATIONS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TRANSLATIONS;
	else if(IsEqualGUID(DBSCHEMA_TRUSTEE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_TRUSTEE;
	else if(IsEqualGUID(DBSCHEMA_USAGE_PRIVILEGES,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_USAGE_PRIVILEGES;
	else if(IsEqualGUID(DBSCHEMA_VIEW_COLUMN_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_VIEW_COLUMN_USAGE;
	else if(IsEqualGUID(DBSCHEMA_VIEW_TABLE_USAGE,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_VIEW_TABLE_USAGE;
	else if(IsEqualGUID(DBSCHEMA_VIEWS,guid))
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_VIEWS;
	else
 		pwszSchemaName = (LPWSTR)wszDBSCHEMA_GUID;

	PRVTRACE(L"%s",pwszSchemaName);
	if(fAddNewLine)
		PRVTRACE(L"\n");

	if(fToScreen)
	{
		odtLog << pwszSchemaName;
		if(fAddNewLine)
			odtLog <<ENDL;
	}
	
	return 0;
}

void TracePropertyName
(
	DBPROPID prop,	
 	BOOL fToScreen,
	BOOL fAddNewLine
)
{
 	LPWSTR pwszProp = L"PROPERTY unknown";
	
	
	if(prop==DBPROP_ABORTPRESERVE)
		pwszProp = (LPWSTR)wszABORTPRESERVE;
	if(prop==DBPROP_ACCESSORDER)
		pwszProp = (LPWSTR)wszACCESSORDER;
	if(prop==DBPROP_APPENDONLY)
		pwszProp = (LPWSTR)wszAPPENDONLY;
	else if(prop==DBPROP_BLOCKINGSTORAGEOBJECTS)
		pwszProp = (LPWSTR)wszBLOCKINGSTORAGEOBJECTS;
	else if(prop==DBPROP_BOOKMARKINFO)
		pwszProp = (LPWSTR)wszBOOKMARKINFO;
	else if(prop==DBPROP_BOOKMARKS)
		pwszProp = (LPWSTR)wszBOOKMARKS;
	else if(prop==DBPROP_BOOKMARKSKIPPED)
		pwszProp = (LPWSTR)wszBOOKMARKSKIPPED;
	else if(prop==DBPROP_BOOKMARKTYPE)
		pwszProp = (LPWSTR)wszBOOKMARKTYPE;
	else if(prop==DBPROP_CACHEDEFERRED)
		pwszProp = (LPWSTR)wszCACHEDEFERRED;
	else if(prop==DBPROP_CANFETCHBACKWARDS)
		pwszProp = (LPWSTR)wszCANFETCHBACKWARDS;
	else if(prop==DBPROP_CANHOLDROWS)
		pwszProp = (LPWSTR)wszCANHOLDROWS;
	else if(prop==DBPROP_CANSCROLLBACKWARDS)
		pwszProp = (LPWSTR)wszCANSCROLLBACKWARDS;
	else if(prop==DBPROP_CHANGEINSERTEDROWS)
		pwszProp = (LPWSTR)wszCHANGEINSERTEDROWS;
	else if(prop==DBPROP_COLUMNRESTRICT)
		pwszProp = (LPWSTR)wszCOLUMNRESTRICT;
	else if(prop==DBPROP_COMMANDTIMEOUT)
		pwszProp = (LPWSTR)wszCOMMANDTIMEOUT;
	else if(prop==DBPROP_COMMITPRESERVE)
		pwszProp = (LPWSTR)wszCOMMITPRESERVE;
	else if(prop==DBPROP_DEFERRED)
		pwszProp = (LPWSTR)wszDEFERRED;
	else if(prop==DBPROP_DELAYSTORAGEOBJECTS)
		pwszProp = (LPWSTR)wszDELAYSTORAGEOBJECTS;
	else if(prop==DBPROP_FILTERCOMPAREOPS)
		pwszProp = (LPWSTR)wszFILTERCOMPAREOPS;
	else if(prop==DBPROP_FINDCOMPAREOPS)
		pwszProp = (LPWSTR)wszFINDCOMPAREOPS;
	else if(prop==DBPROP_HIDDENCOLUMNS)
		pwszProp = (LPWSTR)wszHIDDENCOLUMNS;
	else if(prop==DBPROP_IMMOBILEROWS)
		pwszProp = (LPWSTR)wszIMMOBILEROWS;
	else if(prop==DBPROP_LITERALBOOKMARKS)
		pwszProp = (LPWSTR)wszLITERALBOOKMARKS;
	else if(prop==DBPROP_LITERALIDENTITY)
		pwszProp = (LPWSTR)wszLITERALIDENTITY;
	else if(prop==DBPROP_LOCKMODE)
		pwszProp = (LPWSTR)wszLOCKMODE;
	else if(prop==DBPROP_MAXOPENROWS)
		pwszProp = (LPWSTR)wszMAXOPENROWS;
	else if(prop==DBPROP_MAXPENDINGROWS)
		pwszProp = (LPWSTR)wszMAXPENDINGROWS;
	else if(prop==DBPROP_MAXROWS)
		pwszProp = (LPWSTR)wszMAXROWS;
	else if(prop==DBPROP_MAYWRITECOLUMN)
		pwszProp = (LPWSTR)wszMAYWRITECOLUMN;
	else if(prop==DBPROP_MEMORYUSAGE)
		pwszProp = (LPWSTR)wszMEMORYUSAGE;
	else if(prop==DBPROP_NOTIFICATIONGRANULARITY)
		pwszProp = (LPWSTR)wszNOTIFICATIONGRANULARITY;
	else if(prop==DBPROP_NOTIFICATIONPHASES)
		pwszProp = (LPWSTR)wszNOTIFICATIONPHASES;
	else if(prop==DBPROP_NOTIFYCOLUMNSET)
		pwszProp = (LPWSTR)wszNOTIFYCOLUMNSET;
	else if(prop==DBPROP_NOTIFYROWDELETE)
		pwszProp = (LPWSTR)wszNOTIFYROWDELETE;
	else if(prop==DBPROP_NOTIFYROWFIRSTCHANGE)
		pwszProp = (LPWSTR)wszNOTIFYROWFIRSTCHANGE;
	else if(prop==DBPROP_NOTIFYROWINSERT)
		pwszProp = (LPWSTR)wszNOTIFYROWINSERT;
	else if(prop==DBPROP_NOTIFYROWRESYNCH)
		pwszProp = (LPWSTR)wszNOTIFYROWRESYNCH;
	else if(prop==DBPROP_NOTIFYROWSETRELEASE)
		pwszProp = (LPWSTR)wszNOTIFYROWSETRELEASE;
	else if(prop==DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE)
		pwszProp = (LPWSTR)wszNOTIFYROWSETFETCHPOSITIONCHANGE;
	else if(prop==DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE)
		pwszProp = (LPWSTR)wszNOTIFYROWSETFETCHPOSITIONCHANGE;
	else if(prop==DBPROP_NOTIFYROWUNDOCHANGE)
		pwszProp = (LPWSTR)wszNOTIFYROWUNDOCHANGE;
	else if(prop==DBPROP_NOTIFYROWUNDOCHANGE)
		pwszProp = (LPWSTR)wszNOTIFYROWUNDOCHANGE;
	else if(prop==DBPROP_NOTIFYROWUNDODELETE)
		pwszProp = (LPWSTR)wszNOTIFYROWUNDODELETE;
	else if(prop==DBPROP_NOTIFYROWUNDOINSERT)
		pwszProp = (LPWSTR)wszNOTIFYROWUNDOINSERT;
	else if(prop==DBPROP_NOTIFYROWUPDATE)
		pwszProp = (LPWSTR)wszNOTIFYROWUPDATE;
	else if(prop==DBPROP_ORDEREDBOOKMARKS)
		pwszProp = (LPWSTR)wszORDEREDBOOKMARKS;
	else if(prop==DBPROP_OTHERINSERT)
		pwszProp = (LPWSTR)wszOTHERINSERT;
	else if(prop==DBPROP_OTHERUPDATEDELETE)
		pwszProp = (LPWSTR)wszOTHERUPDATEDELETE;
	else if(prop==DBPROP_OWNINSERT)
		pwszProp = (LPWSTR)wszOWNINSERT;
	else if(prop==DBPROP_OWNUPDATEDELETE)
		pwszProp = (LPWSTR)wszOWNUPDATEDELETE;
	else if(prop==DBPROP_QUICKRESTART)
		pwszProp = (LPWSTR)wszQUICKRESTART;
	else if(prop==DBPROP_REENTRANTEVENTS)
		pwszProp = (LPWSTR)wszREENTRANTEVENTS;
	else if(prop==DBPROP_REMOVEDELETED)
		pwszProp = (LPWSTR)wszREMOVEDELETED;
	else if(prop==DBPROP_REPORTMULTIPLECHANGES)
		pwszProp = (LPWSTR)wszREPORTMULTIPLECHANGES;
	else if(prop==DBPROP_RETURNPENDINGINSERTS)
		pwszProp = (LPWSTR)wszRETURNPENDINGINSERTS;
	else if(prop==DBPROP_ROWRESTRICT)
		pwszProp = (LPWSTR)wszROWRESTRICT;
	else if(prop==DBPROP_ROWSET_ASYNCH)
		pwszProp = (LPWSTR)wszROWSET_ASYNCH;
	else if(prop==DBPROP_ROWTHREADMODEL)
		pwszProp = (LPWSTR)wszROWTHREADMODEL;
	else if(prop==DBPROP_SERVERCURSOR)
		pwszProp = (LPWSTR)wszSERVERCURSOR;
	else if(prop==DBPROP_SERVERDATAONINSERT)
		pwszProp = (LPWSTR)wszSERVERDATAONINSERT;
	else if(prop==DBPROP_TRANSACTEDOBJECT)
		pwszProp = (LPWSTR)wszTRANSACTEDOBJECT;
	else if(prop==DBPROP_UNIQUEROWS)
		pwszProp = (LPWSTR)wszUNIQUEROWS;
	else if(prop==DBPROP_UPDATABILITY)
		pwszProp = (LPWSTR)wszUPDATABILITY;
	else if(prop==DBPROP_STRONGIDENTITY)
		pwszProp = (LPWSTR)wszSTRONGIDENTITY;
	else if(prop==DBPROP_IAccessor)
		pwszProp = (LPWSTR)wszIAccessor;
	else if(prop==DBPROP_IChapteredRowset)
		pwszProp = (LPWSTR)wszIChapteredRowset;
	else if(prop==DBPROP_IColumnsInfo)
		pwszProp = (LPWSTR)wszIColumnsInfo;
	else if(prop==DBPROP_IColumnsRowset)
		pwszProp = (LPWSTR)wszIColumnsRowset;
	else if(prop==DBPROP_IConnectionPointContainer)
		pwszProp = (LPWSTR)wszIConnectionPointContainer;
	else if(prop==DBPROP_IConvertType)
		pwszProp = (LPWSTR)wszIConvertType;
	else if(prop==DBPROP_IDBAsynchStatus)
		pwszProp = (LPWSTR)wszIDBAsynchStatus;
	else if(prop==DBPROP_ILockBytes)
		pwszProp = (LPWSTR)wszILockBytes;
	else if(prop==DBPROP_IMultipleResults)
		pwszProp = (LPWSTR)wszIMultipleResults;
	else if(prop==DBPROP_IParentRowset)
		pwszProp = (LPWSTR)wszIParentRowset;
	else if(prop==DBPROP_IRowset)
		pwszProp = (LPWSTR)wszIRowset;
	else if(prop==DBPROP_IRowsetChange)
		pwszProp = (LPWSTR)wszIRowsetChange;
	else if(prop==DBPROP_IRowsetCurrentIndex)
		pwszProp = (LPWSTR)wszIRowsetCurrentIndex;
	else if(prop==DBPROP_IRowsetFind)
		pwszProp = (LPWSTR)wszIRowsetFind;
	else if(prop==DBPROP_IRowsetIdentity)
		pwszProp = (LPWSTR)wszIRowsetIdentity;
	else if(prop==DBPROP_IRowsetInfo)
		pwszProp = (LPWSTR)wszIRowsetInfo;
	else if(prop==DBPROP_IRowsetIndex)
		pwszProp = (LPWSTR)wszIRowsetIndex;
	else if(prop==DBPROP_IRowsetInfo)
		pwszProp = (LPWSTR)wszIRowsetInfo;
	else if(prop==DBPROP_IRowsetLocate)
		pwszProp = (LPWSTR)wszIRowsetLocate;
	else if(prop==DBPROP_IRowsetRefresh)
		pwszProp = (LPWSTR)wszIRowsetRefresh;
	else if(prop==DBPROP_IRowsetResynch)
		pwszProp = (LPWSTR)wszIRowsetResynch;
	else if(prop==DBPROP_IRowsetScroll)
		pwszProp = (LPWSTR)wszIRowsetScroll;
	else if(prop==DBPROP_IRowsetUpdate)
		pwszProp = (LPWSTR)wszIRowsetUpdate;
	else if(prop==DBPROP_IRowsetView)
		pwszProp = (LPWSTR)wszIRowsetView;
	else if(prop==DBPROP_ISupportErrorInfo)
		pwszProp = (LPWSTR)wszISupportErrorInfo;
	else if(prop==DBPROP_ISequentialStream)
		pwszProp = (LPWSTR)wszISequentialStream;
	else if(prop==DBPROP_IStorage)
		pwszProp = (LPWSTR)wszIStorage;
	else if(prop==DBPROP_IStream)
		pwszProp = (LPWSTR)wszIStream;
	else if(prop==DBPROP_ISupportErrorInfo)
		pwszProp = (LPWSTR)wszISupportErrorInfo;

 	PRVTRACE(L"%s",pwszProp);
	if(fToScreen)
		odtLog << L"\t" << pwszProp;

	if(fAddNewLine)
	{
		PRVTRACE(L"\n");
		if(fToScreen)
			odtLog << L"\n";
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRestrictionsFromSchema
//
// This returns the bitmask for the schema restrictions
// based on the schema.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL TraceRestrictions
(
	ULONG * restrictions,
	ULONG schemaIndex, 
	ULONG * bitmask,
	BOOL fOutLog
)
{
	ULONG mask=0;

	mask = restrictions[schemaIndex];


	if(mask!=0)
	{
		if((mask) & FIRST)
		{
			PRVTRACE(L"1 ");
			if(fOutLog)
				odtLog << L"1 ";
		}
		if((mask) & SECOND)
		{
			PRVTRACE(L"2 ");
			if(fOutLog)
				odtLog << L"2 ";
		}
		if((mask) & THIRD)
		{
			PRVTRACE(L"3 ");
			if(fOutLog)
				odtLog << L"3 ";
		}
		if((mask) & FOURTH)
		{
			PRVTRACE(L"4 ");
			if(fOutLog)
				odtLog << L"4 ";
		}
		if((mask) & FIFTH)
		{
			PRVTRACE(L"5 ");
			if(fOutLog)
				odtLog << L"5 ";
		}
		if((mask) & SIXTH)
		{
			PRVTRACE(L"6 ");
			if(fOutLog)
				odtLog << L"6 ";
		}
		if((mask) & SEVENTH)
		{
			PRVTRACE(L"7 ");
			if(fOutLog)
				odtLog << L"7 ";
		}


		PRVTRACE(L"\n");
		if(fOutLog)
			odtLog << ENDL;

		if(bitmask)
			*bitmask = mask;

		return TRUE;
	}
	return FALSE;
}

LPWSTR GetResourceString(UINT idsString)
{
	CHAR szBuf[2*MAX_MSG_BUF];
	LPWSTR pwszBuf = NULL;
	ULONG ulStrLen;
	HMODULE hMod = GetModuleHandle("idbschmr.dll");

	TESTC(hMod != NULL);

	// Load the string.  LoadString truncates and null terminates if the string is too long for the buffer
	// and returns the number of characters copied.  So from that it is not possible to easily tell if
	// truncation occurred.  Calling GetLastError may work, but I think it is easier to just make sure we
	// have an extra char in the buffer.
	TESTC((ulStrLen = LoadString(hMod, idsString, szBuf, sizeof(szBuf)))>0);
	TESTC(ulStrLen > 0);	
	TESTC(strlen(szBuf) < sizeof(szBuf)-1); // May have been truncation if this fails

	// Convert to Unicode
	pwszBuf = ConvertToWCHAR(szBuf);

CLEANUP:

	return pwszBuf;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CSchemaTest : public CSessionObject
{
	public:

		// Static variables.  (Set once but wont change values).
		// @todo change them to real static variable.

		ULONG		m_cRowsetPropSet;
		DBPROPSET *	m_rgRowsetPropSet;

		BOOL m_fAtLeast1UnsupportedRestrictionIsSet;

		BOOL m_fPrintedSchemasAndRestrictions;

		BOOL m_PrintSchemaName;

		// Capture restrictions for this provider
		BOOL m_fDontCaptureRestrictions;

		// @cmember Pass all restrictions as init'd variants
		BOOL m_fRestrictionsAsInitVariants;

		// @cmember Do I want to pass in restriction when I know restriction is not supported?
		BOOL m_fPassUnsupportedRestrictions;

		// @cmember Global hresult
		HRESULT m_HR;

		// @cmember Variation level return
		BOOL m_fResult;

		// @cmember Was a bookmark requested for rowset
		BOOL m_fBOOKMARK_REQUESTED;

		// @cmember Was a bookmark found on rowset
		BOOL m_fBOOKMARK_FOUND;

		// @cmember Use first restriction
		BOOL m_fRes1;

		// @cmember Use second restriciton
		BOOL m_fRes2;

		// @cmember Use third restriciton
		BOOL m_fRes3;

		// @cmember Use fourth restriciton
		BOOL m_fRes4;

		// @cmember Use FIFTH restriction
		BOOL m_fRes5;

		// @cmember use sixth restriciton
		BOOL m_fRes6;

		// @cmember use seventh restriciton
		BOOL m_fRes7;

		// @cmember If catalog is found
		BOOL m_fMyTableCatalogFound;

		// @cmember If Schema is found
		BOOL m_fMyTableSchemaFound;

		// @cmember If first table name is found
		BOOL m_MyTableName1Found;

		// @cmember If second table name is found
		BOOL m_MyTableName2Found;

		// @cmember If column is found
		BOOL m_MyColumnNameFound;

		// @cmember If data type is found
		BOOL m_MyTableTypeFound;

		// @cmember Count of restrictions used 
		ULONG	m_cErrors;

		// @cmember Expect count of column in schema
		ULONG	m_cColumns;				

		// @cmember Expected count of restrictions in schema
		ULONG	m_cRestrictions;	

		// @cmember Count of restrictions being tested
 		ULONG	m_cRestrictionsCurrent;	

		// @cmember Count of schemas provider supports
		ULONG	m_cSchemasSupported;	

		// @cmember Enum giving provider's OLEDB version
		ULONG	m_ulOLEDBVer;

		// @cmember Count of Bindings
		DBCOUNTITEM	m_cDBBINDING;			

		// @cmember Count of Property Sets
		ULONG	m_cDBPROPSET;
		
		// @cmember Count of DBPROPINFOSETs
		ULONG	m_cDBPROPINFOSET;

		// @cmember Count of rowset DBPROPINFOSETs, including provider specific
		ULONG	m_cRowsetDBPROPINFOSET;

		// @cmember Count of columns in rowset
		DBORDINAL m_cDBCOLUMNINFO;		

		// @cmember Current schema being tested
		GUID	m_guid;					

		// @cmember Interface pointer I want returned
		IID		m_iid;					

		// Bitmask of which restrictions are requested
		RESTRICTIONS	m_restrict;
	
		// Array of FIRST property
		DBPROP	m_rgDBPROP[MAXPROP];

		// @cmember Array of Property Sets
		DBPROPSET m_rgDBPROPSET[MAXPROP];

		// @cmember CCol object
		CCol m_col;
	
		// @cmember First restriction
		WCHAR * m_wszR1;

		// @cmember Second restriction
		WCHAR * m_wszR2;

		// @cmember Third restriction
		WCHAR * m_wszR3;

		// @cmember Fourth restriction
		WCHAR * m_wszR4;

		// @cmember Fifth restriction
		WCHAR * m_wszR5;

		// @cmember Sixth restriction
		WCHAR * m_wszR6;

		// @cmember Seventh restriciton
		WCHAR * m_wszR7;

		// @cmember Non-string restriction (Currently only for Provider Type)
		ULONG	m_ulR;

		// @cmember Bool restriction (Currently only for Best Match in Provider Types)
		TYPE_BOOL m_fR;
		
		// @cmember Rowset pointer should be NULL
		BOOL m_fRowsetPointerNULL;

		// @cmember Count of Restrictions should be 0
		BOOL m_fCountRestrictionsNULL;

		// @cmember Range of Restrictions shoulbd be NULL
		BOOL m_fRangeRestrictionsNULL;

		// @cmember Count of Property Sets should be 0
		BOOL m_fCountPropSetNULL;

		// @cmember Range of Property Sets should be NULL
		BOOL m_fRangePropSetNULL;

		// Array of restrictions, according to spec, no schema has more than 7
  		VARIANT	m_rgvarRestrict[MAXRESTRICTION];	

		// Do I want to copy the restriction strings into my class members
		BOOL m_fCaptureRestrictions;

		// Print Supported Schemas prior to Testing
		BOOL m_fPrintSupportedSchemas;

		// DBTYPE
		DBTYPE m_DataTypeRestriction;

		// Object Type
		SHORT m_ProcedureTypeRestriction;

		// BEST MATCH
		TYPE_BOOL m_BestMatchRestriction;

		// Structure of guid and restrictions on guid
		ULONG  m_currentBitMask;

		// if backend is sql server
		BOOL	m_bSqlServer;

		///////////////
		// Dynamic
		//////////////

		// @cmember Supported Property sets
		DBPROP *	m_rgSupportedProperties;

		// @cmember Count of supported property sets
		ULONG		m_cSupportedProperties;

		// @cmember Range of Restrictions that are supported
		ULONG * m_rgRestrictionSupport;

		// @cmember Expected column names
		WCHAR ** m_rgColumnNames;			

		// @cmember Expected column types
 		DBTYPE * m_rgColumnTypes;			

		// @cmember Array of schemas provider supports
		GUID *	m_rgSchemas;			

		// @cmember Interface pointer
		IDBSchemaRowset *	m_pIDBSchemaRowset;		

		// @cmember Rowset pointer
		IRowset *	m_pIRowset;				

		// @cmember Count of DBPROPINFOSETs
		DBPROPINFOSET *	m_rgDBPROPINFOSET;

		// @cmember Rowset property sets, including provider specific
		DBPROPINFOSET *	m_rgRowsetDBPROPINFOSET;

		// @cmember Description Buffer
		WCHAR *	m_pDescBuffer;
		LPWSTR	m_pRowsetDescBuffer;

		// @cmember Array of column information in rowset
		DBCOLUMNINFO *	m_rgDBCOLUMNINFO;		

		// @cmember Buffer of strings for column information 
		WCHAR *	m_pStringsBuffer;		

		// @cmember Array of Bindings
		DBBINDING *	m_rgDBBINDING;
		
		// @cmember Aggregation Pointer
		IUnknown * m_punkOuter;

		// @cmember IAccessor pointer
		IAccessor * m_pIAccessor;

		// @cmember IRowsetInfo pointer
		IRowsetInfo * m_pIRowsetInfo;

		// @cmember IColumnsInfo pointer
		IColumnsInfo * m_pIColumnsInfo;

		// @cmember IRowsetChange
		IRowsetChange * m_pIRowsetChange;

		// @cmember IUnknown
		IUnknown * m_pIUnknown;

		// @cmember m_pwszDropProc
		WCHAR * m_pwszDropProc;

		// @cmember m_eRowCount - how to interpret row count below
		ROW_COUNT m_eRowCount;

		// @cmember m_lRowCount - count of rows to expect from schema rowset
		DBORDINAL m_ulRowCount;

		// @cmember Restrictions
		// If I can't find pubs for sql server or nwind for access, 
		// I'll take first catalog I find.
		WCHAR * m_pwszCatalogRestriction;
		WCHAR * m_pwszSchemaRestriction;
		WCHAR * m_pwszTableRestriction;
		WCHAR * m_pwszColumnRestriction;

		WCHAR * m_pwszAssertion_ConstraintRestriction;
		WCHAR * m_pwszCheck_ConstraintRestriction;
		WCHAR * m_pwszKey_Column_Usage_ConstraintRestriction;
		WCHAR * m_pwszReferential_ConstraintRestriction;
		WCHAR * m_pwszTable_ConstraintRestriction;
		
		WCHAR * m_pwszCharacter_SetRestriction;
		WCHAR * m_pwszCollationRestriction;
		WCHAR * m_pwszDomainRestriction;
		WCHAR * m_pwszGrantorRestriction;
		WCHAR * m_pwszGranteeRestriction;
		WCHAR * m_pwszPK_TableRestriction;
		WCHAR * m_pwszFK_TableRestriction;
		WCHAR * m_pwszIndexRestriction;
		WCHAR * m_pwszProcedureRestriction;
		WCHAR * m_pwszProcedureColumnsRestriction;
		WCHAR * m_pwszParameterRestriction;
		TYPE_UI2 m_uiProcedureType;
		WCHAR * m_pwszSchema_OwnerRestriction;
		WCHAR * m_pwszTable_TypeRestriction;
		WCHAR * m_pwszConstraint_TypeRestriction;
		WCHAR * m_pwszTranslationReplace;
		WCHAR * m_pwszObjectRestriction;
		WCHAR * m_pwszObject_TypeRestriction;
		WCHAR * m_pwszViewRestriction;
		WCHAR * m_pwszStatisticsCatalogRestriction;
		WCHAR * m_pwszStatisticsSchemaRestriction;
		WCHAR * m_pwszStatisticsNameRestriction;
		TYPE_UI2 m_uiStatisticsTypeRestriction;
		LPWSTR	m_pwszUpdateStatsFormat;
		LPWSTR	m_pwszSortSetting;

		//--------------------
		//  Functions
		//--------------------

		// @cmember Constructor
		CSchemaTest(const LPWSTR wszTestCaseName = NULL): CSessionObject(wszTestCaseName)
		{
			INIT

			m_bSqlServer = FALSE;
			m_rgDBPROPINFOSET=NULL;
			m_pDescBuffer=NULL;
			m_pRowsetDescBuffer = NULL;
			m_cDBPROPINFOSET=0;
			m_rgSupportedProperties=NULL;
			m_cSupportedProperties=0;
			m_fAtLeast1UnsupportedRestrictionIsSet=FALSE;


			m_fPrintedSchemasAndRestrictions=FALSE;
			m_currentBitMask = 0;
			m_fDontCaptureRestrictions = FALSE;

			m_rgColumnNames=NULL;			
			m_rgColumnTypes=NULL;			
			m_rgSchemas=NULL;			
			m_pDescBuffer=NULL;
			m_pRowsetDescBuffer = NULL;
			m_rgDBCOLUMNINFO=NULL;		
			m_pStringsBuffer=NULL;		
			m_rgDBBINDING=NULL;
			m_punkOuter=NULL;
			m_pIAccessor=NULL;
			m_pIRowset=NULL;				
			m_pIRowsetInfo=NULL;
			m_pIColumnsInfo=NULL;
			m_pIRowsetChange=NULL;
			m_pIOpenRowset = NULL;
			m_pIDBSchemaRowset=NULL;		
			m_pIUnknown = NULL;
			m_PrintSchemaName=TRUE;

			m_pwszCatalogRestriction=NULL;
			m_pwszSchemaRestriction=NULL;
			m_pwszTableRestriction=NULL;
			m_pwszColumnRestriction=NULL;
			m_pwszAssertion_ConstraintRestriction=NULL;
			m_pwszCheck_ConstraintRestriction=NULL;
			m_pwszKey_Column_Usage_ConstraintRestriction=NULL;
			m_pwszReferential_ConstraintRestriction=NULL;
			m_pwszTable_ConstraintRestriction=NULL;
			m_pwszCharacter_SetRestriction=NULL;
			m_pwszCollationRestriction=NULL;
			m_pwszDomainRestriction=NULL;
			m_pwszGrantorRestriction=NULL;
			m_pwszGranteeRestriction=NULL;
			m_pwszPK_TableRestriction=NULL;
			m_pwszFK_TableRestriction=NULL;
			m_pwszIndexRestriction=NULL;
			m_pwszProcedureRestriction=NULL;
			m_pwszParameterRestriction=NULL;
			m_pwszProcedureColumnsRestriction=NULL;
			m_pwszStatisticsCatalogRestriction = NULL;
			m_pwszStatisticsSchemaRestriction = NULL;
			m_pwszStatisticsNameRestriction = NULL;
			m_pwszUpdateStatsFormat = NULL;
			m_pwszSortSetting = NULL;

			m_uiProcedureType=0;
			m_uiStatisticsTypeRestriction=0;
			m_pwszSchema_OwnerRestriction=NULL;
			m_pwszConstraint_TypeRestriction=NULL;
			m_pwszTable_TypeRestriction=NULL;
			m_pwszTranslationReplace=NULL;
			m_pwszObjectRestriction=NULL;
			m_pwszObject_TypeRestriction=NULL;
			m_pwszViewRestriction=NULL;
			m_rgRestrictionSupport=NULL;

			m_rgRestrictionSupport=NULL;
			m_pwszProcedureColumnsRestriction=NULL;

			m_fPrintSupportedSchemas=TRUE;
			m_fBOOKMARK_REQUESTED=FALSE;
			m_fBOOKMARK_FOUND=FALSE;
			m_fRes1=TRUE;
			m_fRes2=TRUE;
			m_fRes3=TRUE;
			m_fRes4=TRUE;
			m_fRes5=TRUE;
			m_fRes6=TRUE;
			m_fRes7=TRUE;
			m_fMyTableCatalogFound=FALSE;
			m_fMyTableSchemaFound=FALSE;
			m_MyTableName1Found=FALSE;
			m_MyTableName2Found=FALSE;
			m_MyColumnNameFound=FALSE;
			m_MyTableTypeFound=FALSE;
			m_cErrors=0;
			m_cColumns=0;				
			m_cRestrictions=0;	
			m_cRestrictionsCurrent=0;	
			m_cSchemasSupported=0;	
			m_cDBBINDING=0;			
			m_cDBPROPSET=0;
			m_cDBCOLUMNINFO=0;		
			m_iid=IID_NULL;					
			m_guid=GUID_NULL;					
			m_restrict=0;
			m_HR=E_FAIL;
			m_fResult=FALSE;
			m_fCaptureRestrictions=FALSE;
			m_DataTypeRestriction=0;
			m_ProcedureTypeRestriction=0;


			m_fRowsetPointerNULL=FALSE;
			m_fCountRestrictionsNULL=FALSE;
			m_fRangeRestrictionsNULL=FALSE;
			m_fCountPropSetNULL=FALSE;
			m_fRangePropSetNULL=FALSE;
			m_fPassUnsupportedRestrictions=FALSE;
			m_fRestrictionsAsInitVariants=FALSE;
			
			
			m_wszR1=NULL;
			m_wszR2=NULL;
			m_wszR3=NULL;
			m_wszR4=NULL;
			m_wszR5=NULL;
			m_wszR6=NULL;
			m_wszR7=NULL;
			m_ulR=0;
			m_fR = FALSE;

			m_cRowsetDBPROPINFOSET = 0;
			m_rgRowsetDBPROPINFOSET = NULL;
			m_cRowsetPropSet=0;
			m_rgRowsetPropSet=NULL;
			m_eRowCount = MIN_VALUE;
			m_ulRowCount = 1;
			m_pwszTableName = NULL;
			m_pwszStatName = NULL;
			m_iOrdinalExpected = 0;
			m_fDetailCheck = FALSE;
			m_prgColInfo = NULL;
			m_cColInfo = 0;
			m_pwszStringsBuffer = NULL;
			m_fPrimaryKey=FALSE;
			m_fForeignKey=FALSE;

  			for(ULONG index=0;index<MAXRESTRICTION;index++)
				VariantInit(&(m_rgvarRestrict[index]));
		};

		// @cmember Destructor
		virtual ~CSchemaTest()
		{
			ULONG index;
			FREE

			ASSERT(!m_rgColumnNames);			
			ASSERT(!m_rgColumnTypes);			
			ASSERT(!m_rgSchemas);			
			ASSERT(!m_pIDBSchemaRowset);		
			ASSERT(!m_pIRowset);				
			ASSERT(!m_rgDBCOLUMNINFO);		
			ASSERT(!m_pStringsBuffer);		
			ASSERT(!m_rgDBBINDING);	
			ASSERT(!m_punkOuter);
			ASSERT(!m_pIAccessor);
			ASSERT(!m_pIRowsetInfo);
			ASSERT(!m_pIColumnsInfo);
			ASSERT(!m_pIRowsetChange);
			ASSERT(!m_pIUnknown);
			ASSERT(!m_pwszAssertion_ConstraintRestriction);
			ASSERT(!m_pwszCheck_ConstraintRestriction);
			ASSERT(!m_pwszKey_Column_Usage_ConstraintRestriction);
			ASSERT(!m_pwszReferential_ConstraintRestriction);
			ASSERT(!m_pwszTable_ConstraintRestriction);
			ASSERT(!m_pwszCharacter_SetRestriction);
			ASSERT(!m_pwszCollationRestriction);
			ASSERT(!m_pwszDomainRestriction);
			ASSERT(!m_pwszGrantorRestriction);
			ASSERT(!m_pwszGranteeRestriction);
			ASSERT(!m_pwszPK_TableRestriction);
			ASSERT(!m_pwszFK_TableRestriction);
			ASSERT(!m_pwszProcedureRestriction);
			ASSERT(!m_pwszParameterRestriction);
			ASSERT(!m_pwszProcedureColumnsRestriction);
			ASSERT(!m_pwszSchema_OwnerRestriction);
			ASSERT(!m_pwszConstraint_TypeRestriction);
			ASSERT(!m_pwszTable_TypeRestriction);
			ASSERT(!m_pwszTranslationReplace);
			ASSERT(!m_pwszObjectRestriction);
			ASSERT(!m_pwszObject_TypeRestriction);
			ASSERT(!m_pwszViewRestriction);
			ASSERT(!m_rgRestrictionSupport);
			ASSERT(!m_rgRestrictionSupport);
			ASSERT(!m_pwszProcedureColumnsRestriction);

			for(index=0;index<MAXRESTRICTION;index++)
				VariantClear(&(m_rgvarRestrict[index]));
  			for(index=0;index<MAXPROP;index++)
				VariantClear(&(m_rgDBPROP[index].vValue));

			SAFE_FREE(m_pwszTableName);
			SAFE_FREE(m_pwszStatName);
			SAFE_FREE(m_prgColInfo);
			SAFE_FREE(m_pwszStringsBuffer);
		};

		// @cmember Init
		BOOL Init(void);

		// @cmember Terminate
		BOOL Terminate(void);

 		// Check Results of Method
		BOOL CheckResults(IUnknown * pIRowset, IID iid);

		// Check Riid can be used
		BOOL CheckRIID(	IUnknown * pColRowset,IID iid);

		// Verify Metadata of schema
		BOOL CheckAgainstIColumnsInfo(void);

		// GetRow
		HRESULT GetRow(void);

		// Set any interesting params and execute test method
		HRESULT GetRowset(BOOL fFreeRowset=TRUE);

		// Free Variation stuff
		void Free_Stuff(void);

		// Init Variation Stuff
		void Init_Stuff(void);
 
		// Get Next Schema Information, includes cRestrictions and rgRestrictions
		BOOL GetSchemaInfo(
			REQUESTED_SCHEMA schemaType,			// [IN] if I want the schema supported, or not, or I have a specific schema in mind
			ULONG ulIndexOfSchemaRequesting,		// [IN] Index of Schema
			GUID schema=GUID_NULL					// [IN] index of schema (in header file)
		);

		// Verify Rowset for column names, column data types, and restrictions met.
		// All TRACEable data will be put to debug window.
		BOOL VerifyRowset(IUnknown * pIUnknown = NULL);
			
		// @cmember VerifyRow, verifies restrictions only.
		virtual BOOL VerifyRow(
			GUID			m_guid,
			DBCOUNTITEM		iRow,
			BYTE *			pRow
		);	

		// @cmember SetRestriction
		void SetRestriction(ULONG bit);

		// @cmember ClearRestriction
		void ClearRestriction(ULONG bit);

		// @cmember IsSchemaSupported
		BOOL IsSchemaSupported(GUID schema);

		// @cmember TestSchemaRestrictions
		BOOL TestSchemaRestrictions(GUID schema,ULONG bit, ROW_COUNT eRowCount=DEFAULT, ULONG cExpRows=0);

		// @cmember Test Data Returned from IDBSchema
		BOOL TestReturnData(DBCOUNTITEM iRow,DATA * pColumn,ULONG bit,BOOL * fRes,WCHAR * wszRes, BOOL fNullable=TRUE);

		// @cmember Test the Table Name Restriction
		void SetRestriction(RESTRICTIONSENUM ebit,ULONG ulRestriction,WCHAR ** pwszR,WCHAR * wszRestriction);

		BOOL PrepareParams_ASSERTIONS();
		BOOL VerifyRow_ASSERTIONS(DBCOUNTITEM iRow,BYTE * pData);	
		
		BOOL PrepareParams_CATALOGS();
		BOOL VerifyRow_CATALOGS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_CHARACTER_SETS();
		BOOL VerifyRow_CHARACTER_SETS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_CHECK_CONSTRAINTS();
		BOOL VerifyRow_CHECK_CONSTRAINTS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_COLLATIONS();
		BOOL VerifyRow_COLLATIONS(DBCOUNTITEM iRow,BYTE * pData);

		BOOL PrepareParams_COLUMN_DOMAIN_USAGE();
		BOOL VerifyRow_COLUMN_DOMAIN_USAGE(DBCOUNTITEM iRow,BYTE * pData);

		BOOL PrepareParams_COLUMN_PRIVILEGES();
		BOOL VerifyRow_COLUMN_PRIVILEGES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_COLUMNS();
		BOOL VerifyRow_COLUMNS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_CONSTRAINT_COLUMN_USAGE();
		BOOL VerifyRow_CONSTRAINT_COLUMN_USAGE(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_CONSTRAINT_TABLE_USAGE();
		BOOL VerifyRow_CONSTRAINT_TABLE_USAGE(DBCOUNTITEM iRow,BYTE * pData);
		
		BOOL PrepareParams_FOREIGN_KEYS();
		BOOL VerifyRow_FOREIGN_KEYS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_INDEXES();
		BOOL VerifyRow_INDEXES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_KEY_COLUMN_USAGE();
		BOOL VerifyRow_KEY_COLUMN_USAGE(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_PRIMARY_KEYS();
		BOOL VerifyRow_PRIMARY_KEYS(DBCOUNTITEM iRow,BYTE * pData);

		BOOL PrepareParams_PROCEDURE_COLUMNS();
		BOOL VerifyRow_PROCEDURE_COLUMNS(DBCOUNTITEM iRow,BYTE * pData);

		BOOL PrepareParams_PROCEDURE_PARAMETERS();
		BOOL VerifyRow_PROCEDURE_PARAMETERS(DBCOUNTITEM iRow, BYTE * pData);

		BOOL PrepareParams_PROCEDURES();
		BOOL VerifyRow_PROCEDURES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_PROVIDER_TYPES();
		BOOL VerifyRow_PROVIDER_TYPES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_REFERENTIAL_CONSTRAINTS();
		BOOL VerifyRow_REFERENTIAL_CONSTRAINTS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_SCHEMATA();
		BOOL VerifyRow_SCHEMATA(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_SQL_LANGUAGES();
		BOOL VerifyRow_SQL_LANGUAGES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_STATISTICS();
		BOOL VerifyRow_STATISTICS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TABLE_CONSTRAINTS();
		BOOL VerifyRow_TABLE_CONSTRAINTS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TABLE_PRIVILEGES();
		BOOL VerifyRow_TABLE_PRIVILEGES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TABLE_STATISTICS();
		BOOL VerifyRow_TABLE_STATISTICS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TABLES();
		BOOL VerifyRow_TABLES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TABLES_INFO();
		BOOL VerifyRow_TABLES_INFO(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TRANSLATIONS();
		BOOL VerifyRow_TRANSLATIONS(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_TRUSTEE();
		BOOL VerifyRow_TRUSTEE(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_USAGE_PRIVILEGES();
		BOOL VerifyRow_USAGE_PRIVILEGES(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_VIEW_COLUMN_USAGE();
		BOOL VerifyRow_VIEW_COLUMN_USAGE(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_VIEW_TABLE_USAGE();
		BOOL VerifyRow_VIEW_TABLE_USAGE(DBCOUNTITEM iRow,BYTE * pData);	

		BOOL PrepareParams_VIEWS();
		BOOL VerifyRow_VIEWS(DBCOUNTITEM iRow,BYTE * pData);	

		// Capture Restrictions
		BOOL CaptureRestrictions();
		void FreeRestrictions();

		BOOL Find_Catalog_and_Schema();
		BOOL Find_TableInfo();
		BOOL Find_Assertion_Constraint();
		BOOL Find_Character_Set();
		BOOL Find_Collation();
		BOOL Find_Domain();
		BOOL Find_Grantor_and_Grantee();
		BOOL Find_Key_Column_Usage_Constraint();
		BOOL Find_Procedure();
		BOOL Find_ProcedureColumn();
		BOOL Find_Parameter();
		BOOL Find_Referential_Constraint();
		BOOL Find_Constraint_Type();
		BOOL Find_Translation();
		BOOL Find_View();
		BOOL Find_Check_Constraint();
		BOOL Find_BestMatch();
		BOOL Find_PK_and_FK();
		BOOL Find_Table_Statistics_Restrictions();
		BOOL UpdateStatistics(void);


		BOOL Release_Catalog_and_Schema();
		BOOL Release_TableInfo();
		BOOL Release_Assertion_Constraint();
		BOOL Release_Character_Set();
		BOOL Release_Collation();
		BOOL Release_Domain();
		BOOL Release_Grantor_and_Grantee();
		BOOL Release_Key_Column_Usage_Constraint();
		BOOL Release_Procedure();
		BOOL Release_ProcedureColumn();
		BOOL Release_Parameter();
		BOOL Release_Referential_Constraint();
		BOOL Release_Constraint_Type();
		BOOL Release_Translation();
		BOOL Release_View();
		BOOL Release_Check_Constraint();
		BOOL Release_Index_Type();
		BOOL Release_PK_and_FK();
		BOOL Release_Table_Statistics_Restrictions();


		HRESULT GetAllPropertySets();
		HRESULT FreeAllPropertySets();

		HRESULT ShouldTestSchemaRestriction(GUID schema,ULONG restrictions);
		HRESULT GetRowsetPropertySet();
		ULONG	NumberofRestrictions(GUID schema);
		
		BOOL PropIsBool(DBPROPID prop);
		BOOL IsRowsetPropertySupported(DBPROPID propid);
		BOOL IsRowsetPropertySet(GUID guidPropset);

		// Functions for creating tables with Primary and foreign keys.
		BOOL InitKeysOnTable();
		BOOL TerminateKeysOnTable();

		// Returns restriction number if it is known the schema guid for index has a catalog restriction.
		ULONG SchemaCatalogRestriction(GUID guidSchema);
		ULONG SchemaCatalogRestriction(ULONG ulIndex);

		// Returns restriction number if it is known the schema guid for index has a schema restriction.
		ULONG SchemaSchemaRestriction(GUID guidSchema);
		ULONG SchemaSchemaRestriction(ULONG ulIndex);

		// Returns restriction number if it is known the schema guid for index has a table name restriction.
		ULONG SchemaTableRestriction(GUID guidSchema);
		ULONG SchemaTableRestriction(ULONG ulIndex);

		// Sets catalog restriction for those schemas that support it to prevent excessive run times.
		void LimitRestrictions(ULONG ulIndex);

protected:
	ULONG m_ulTableOrdinal;
	LPWSTR m_pwszStringsBuffer;
	DBCOLUMNINFO * m_prgColInfo;
	DBORDINAL m_cColInfo;
	LPWSTR m_pwszTableName;
	LPWSTR m_pwszStatName;


	BOOL m_fPrimaryKey;
	BOOL m_fForeignKey;
	ULONG m_iOrdinalExpected;
	BOOL m_fDetailCheck;

	// Returns the count of columns and column info for the given table
	HRESULT GetColumnInfo(LPBYTE pData, DBBINDING * pBinding);

	void SetRowCount(ROW_COUNT eRowCount, DBORDINAL ulRowCount);

	virtual HRESULT GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRow);

	LPBYTE GetValuePtr(DBORDINAL iOrdinal, LPBYTE pData, DBBINDING * pBinding = NULL);

	BOOL CheckHistogramColInfo(IRowset * pIHistogramRowset, DBTYPE wRangeColType,
		DBORDINAL * pcBinding, DBBINDING ** ppBinding, DBLENGTH * pcbRowSize, 
		HACCESSOR * phAccessor);

	// Returns cardinality (COUNT) information for the given table using ulColIndex index into
	// columns info for table.
	DBCOUNTITEM Cardinality(LPWSTR pwszTableName, ULONG ulColIndex, CARDINALITY eCardinality,
		DBBINDING * pStartBind = NULL, DBBINDING * pEndBind = NULL, LPBYTE pDataHist = NULL,
		DBLENGTH cbRowSize = 0);

	CErrorCache		m_EC;

};	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::Init()
{
	ULONG			iDBMS			= 0;
	WCHAR *			pwszDBMSName	= NULL;

	m_cRowsetPropSet=0;
	m_rgRowsetPropSet=NULL;
	
	// Initialize error cache
	m_EC.Init(GetModInfo()->GetDebugMode());

	if(COLEDB::Init())	
	{
		WCHAR * pwszOLEDBVER = NULL;

		m_pwszDropProc=NULL;

		// Set DSO pointer
		if(m_pThisTestModule->m_pIUnknown)
		{
			m_pIDBInitialize = (IDBInitialize *)m_pThisTestModule->m_pIUnknown;
			m_pIDBInitialize->AddRef();
		}

		// Set CSession pointer
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
	
		// Set Table pointer
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

		// Get IDBSchemaRowset pointer
		if(m_pIOpenRowset)
		{
			if(FAILED(m_pIOpenRowset->QueryInterface(
				IID_IDBSchemaRowset,
				(void **) &m_pIDBSchemaRowset)))
				return FALSE;

		}
		else //(m_pIDBCreateCommand)
		{
			if(FAILED(m_pIDBCreateCommand->QueryInterface(
				IID_IDBSchemaRowset,
				(void **) &m_pIDBSchemaRowset)))
				return FALSE;
		}

		// First check for support for alter table syntax.
		ULONG_PTR ulSQLSupport = 0;
		GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulSQLSupport);

		if( ulSQLSupport & DBPROPVAL_SQL_ANSI89_IEF )
			InitKeysOnTable();

		// Get Supported Schemas
		if(FAILED(m_pIDBSchemaRowset->GetSchemas(
			&m_cSchemasSupported,
			&m_rgSchemas,
			&m_rgRestrictionSupport)))
		{
			m_pIDBSchemaRowset->Release();
			m_pIDBSchemaRowset = NULL;
			return FALSE;
		}

		if(m_cSchemasSupported==0)
		{
			odtLog << L"No schemas supported\n";
			return FALSE;
		}
		if(FAILED(GetAllPropertySets()))
			return FALSE;

		if(FAILED(GetRowsetPropertySet()))
			return FALSE;

		// Get DBPROP_PROVIDEROLEDBVER for use in setting correct column count for each schema
		if (GetProperty(DBPROP_PROVIDEROLEDBVER, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &pwszOLEDBVER))
		{	
			if (!wcscmp(pwszOLEDBVER, L"02.00"))
				m_ulOLEDBVer = VER_20;
			else if (!wcscmp(pwszOLEDBVER, L"02.10"))
				m_ulOLEDBVer = VER_21;
			else if (!wcscmp(pwszOLEDBVER, L"02.50"))
				m_ulOLEDBVer = VER_25;
			else if (!wcscmp(pwszOLEDBVER, L"02.60"))
				m_ulOLEDBVer = VER_26;
			else if (!wcscmp(pwszOLEDBVER, L"02.70"))
				m_ulOLEDBVer = VER_27;
			else
				ASSERT(!L"Unknown schema version.");

			SAFE_FREE(pwszOLEDBVER);

		}

		// Call the GetProperty for the DBMS Name
		if(GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &pwszDBMSName))
		{
			if (!wcscmp(pwszDBMSName, L"Microsoft SQL Server"))
				m_bSqlServer = TRUE;

			// Get the DBMS specific command to create/update statistics and set sorts
			for (iDBMS = 0; iDBMS < NUMELEM(g_DBMSList); iDBMS++)
			{
				if (!wcscmp(pwszDBMSName, g_DBMSList[iDBMS].pwszDBMSName))
				{
					m_pwszUpdateStatsFormat = g_DBMSList[iDBMS].pwszUpdateStatsFormat;
					m_pwszSortSetting = g_DBMSList[iDBMS].pwszSortSetting;
					break;
				}
			}
		}

		// Set the sort sequence desired
		if (m_pwszSortSetting)
		{
			CHECK(m_pTable->BuildCommand(m_pwszSortSetting, IID_IRowset, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, NULL), S_OK);
		}

//		if(!m_fDontCaptureRestrictions)
//			CaptureRestrictions();

		PROVIDER_FREE(pwszDBMSName);
	
		return TRUE;
	}  
	return FALSE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::Terminate()
{

	// Release everything from the init
	SAFE_RELEASE(m_pIDBSchemaRowset);

	// Free the memory
	PROVIDER_FREE(m_rgSchemas);
	PROVIDER_FREE(m_rgRestrictionSupport);
	PROVIDER_FREE(m_rgSupportedProperties);

	// Free the Properties
	FreeProperties(&m_cDBPROPINFOSET, &m_rgDBPROPINFOSET, &m_pDescBuffer);
	FreeProperties(&m_cRowsetDBPROPINFOSET, &m_rgRowsetDBPROPINFOSET, &m_pRowsetDescBuffer);
	FreeProperties(&m_cRowsetPropSet, &m_rgRowsetPropSet);

	FreeRestrictions();

	ULONG_PTR ulSQLSupport = 0;
	if (m_pIDBInitialize)
		GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulSQLSupport);
	
	if( ulSQLSupport & DBPROPVAL_SQL_ANSI89_IEF )
		TerminateKeysOnTable();

	ReleaseDBSession();
	SAFE_RELEASE(m_pIDBInitialize);

	if(!COLEDB::Terminate())
		return FALSE;

	return(CTestCases::Terminate());
}

// Is the property set GUID one of the Rowset property GUIDs
BOOL CSchemaTest::IsRowsetPropertySet(GUID guidPropset)
{
	// Loop through all the Rowset property info from the DBPROPSET_ROWSETALL
	// Includes provider specific
	for (ULONG iPropertyInfoSet=0;
		iPropertyInfoSet < m_cRowsetDBPROPINFOSET;
		iPropertyInfoSet++
		)
		if (m_rgRowsetDBPROPINFOSET[iPropertyInfoSet].guidPropertySet == guidPropset)
			return TRUE;

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Is rowset property supported by temp table
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::IsRowsetPropertySupported(DBPROPID propid)
{
	ULONG i,j;
	ULONG iRowsetSet=0;

	for(i=0;i<m_rgRowsetPropSet[0].cProperties;i++)
	{
		if(propid==m_rgRowsetPropSet[0].rgProperties[i].dwPropertyID)
		{
			if (m_rgRowsetPropSet[0].rgProperties[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
			{
				PRVTRACE(L"Property is supported\n");
				goto CHECK_WRITABLE;
			}
		}
	}
	PRVTRACE(L"Property is not supported\n");
	return FALSE;


CHECK_WRITABLE:
	for(i=0;i<m_cDBPROPINFOSET;i++)
	{
		if(IsEqualGUID(m_rgDBPROPINFOSET[i].guidPropertySet,DBPROPSET_ROWSET))
		{
			for(j=0;j<m_rgDBPROPINFOSET[i].cPropertyInfos;j++)
			{
				if(m_rgDBPROPINFOSET[i].rgPropertyInfos[j].dwPropertyID==propid)
				{
					if(m_rgDBPROPINFOSET[i].rgPropertyInfos[j].dwFlags &DBPROPFLAGS_WRITE)
					{
						PRVTRACE(L"Property is writable\n");
						return TRUE;
					}
					else
					{
						PRVTRACE(L"Property is not writable\n");
						//odtLog << L"Property is not writable\n";
						return FALSE;
					}
				}
			}
		}
	}
 	
	return FALSE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Find all rowset properties that temp table supports
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT CSchemaTest::GetRowsetPropertySet()
{
	HRESULT			hr=E_FAIL;
	IRowsetInfo *	pIRowsetInfo=NULL;

	if(!m_pIDBSchemaRowset)
		goto CLEANUP;

	// generate rowset with types as schema
	// since it is mandatory
	if(FAILED(hr=m_pIDBSchemaRowset->GetRowset(
		NULL,
		DBSCHEMA_PROVIDER_TYPES,
		0,
		NULL,
		IID_IRowsetInfo,
		0,
		NULL,
		(IUnknown **) &pIRowsetInfo)))
		goto CLEANUP;
	if(FAILED(hr=pIRowsetInfo->GetProperties(
		0,
		NULL,
		&m_cRowsetPropSet,
		&m_rgRowsetPropSet)))
		goto CLEANUP;

CLEANUP:

	if(pIRowsetInfo)
		pIRowsetInfo->Release();

	return hr;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Is Property a VT_BOOL
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::PropIsBool(DBPROPID prop)
{
	if (( prop == DBPROP_ABORTPRESERVE) ||
		 (prop == DBPROP_APPENDONLY) ||
 		 (prop == DBPROP_BLOCKINGSTORAGEOBJECTS) ||
		 (prop == DBPROP_BOOKMARKS) ||
		 (prop == DBPROP_BOOKMARKSKIPPED) ||
		 (prop == DBPROP_CACHEDEFERRED) ||
		 (prop == DBPROP_CANFETCHBACKWARDS) ||
		 (prop == DBPROP_CANHOLDROWS) ||
		 (prop == DBPROP_CANSCROLLBACKWARDS) ||
		 (prop == DBPROP_CHANGEINSERTEDROWS) ||
		 (prop == DBPROP_COLUMNRESTRICT) ||
		 (prop == DBPROP_COMMITPRESERVE) ||
		 (prop == DBPROP_DEFERRED) ||
		 (prop == DBPROP_DELAYSTORAGEOBJECTS) ||
		 (prop == DBPROP_IMMOBILEROWS) ||
		 (prop == DBPROP_LITERALBOOKMARKS) ||
		 (prop == DBPROP_LITERALIDENTITY) ||
		 (prop == DBPROP_MAYWRITECOLUMN) ||
		 (prop == DBPROP_ORDEREDBOOKMARKS) ||
		 (prop == DBPROP_OTHERINSERT) ||
		 (prop == DBPROP_OTHERUPDATEDELETE) ||
		 (prop == DBPROP_OWNINSERT) ||
		 (prop == DBPROP_OWNUPDATEDELETE) ||
 		 (prop == DBPROP_QUICKRESTART) ||
		 (prop == DBPROP_REENTRANTEVENTS) ||
		 (prop == DBPROP_REMOVEDELETED) ||
		 (prop == DBPROP_REPORTMULTIPLECHANGES) ||
		 (prop == DBPROP_RETURNPENDINGINSERTS) ||
		 (prop == DBPROP_ROWRESTRICT) ||
		 (prop == DBPROP_SERVERCURSOR) ||
		 (prop == DBPROP_STRONGIDENTITY) ||
		 (prop == DBPROP_TRANSACTEDOBJECT) ||
		 (prop == DBPROP_OTHERINSERT) ||
		 (prop == DBPROP_IAccessor) ||
		 (prop == DBPROP_IColumnsInfo) ||		 
		 (prop == DBPROP_IColumnsRowset) ||
 		 (prop == DBPROP_IConnectionPointContainer) ||
		 (prop == DBPROP_IRowset) ||
 		 (prop == DBPROP_IRowsetChange) ||
 		 (prop == DBPROP_IRowsetIdentity) ||
 		 (prop == DBPROP_IRowsetInfo) ||
		 (prop == DBPROP_IRowsetLocate) ||
 		 (prop == DBPROP_IRowsetResynch) ||
		 (prop == DBPROP_IRowsetScroll) ||
		 (prop == DBPROP_IRowsetUpdate) ||
 		 (prop == DBPROP_ISupportErrorInfo) ||
		 (prop == DBPROP_ILockBytes) ||
 		 (prop == DBPROP_ISequentialStream) ||
		 (prop == DBPROP_IStorage) ||
 		 (prop == DBPROP_IStream))
		return TRUE;
	else
		return FALSE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT CSchemaTest::GetAllPropertySets()
{

	HRESULT hr=E_FAIL;
	ULONG index1=0;
	ULONG index2=0;
	IDBProperties * pIDBProperties=NULL;

	// make sure I have a DSO object
	if(!m_pIDBInitialize)
		goto CLEANUP;

	// get necessary pointer
	if(!CHECK(hr=m_pIDBInitialize->QueryInterface(IID_IDBProperties,(void**)&pIDBProperties),S_OK))
		goto CLEANUP;

	// get property info
	if(FAILED(hr=pIDBProperties->GetPropertyInfo(0,NULL,&m_cDBPROPINFOSET,&m_rgDBPROPINFOSET,&m_pDescBuffer)))
		goto CLEANUP;

	// Get all the rowset properties
	DBPROPIDSET rgRowsetIDSET[1];
	rgRowsetIDSET[0].rgPropertyIDs=NULL;
	rgRowsetIDSET[0].cPropertyIDs=0;
	rgRowsetIDSET[0].guidPropertySet=DBPROPSET_ROWSETALL;

	if(FAILED(hr=pIDBProperties->GetPropertyInfo(1,rgRowsetIDSET,&m_cRowsetDBPROPINFOSET,&m_rgRowsetDBPROPINFOSET,&m_pRowsetDescBuffer)))
		goto CLEANUP;

	for(index1=0;index1<m_cDBPROPINFOSET;index1++)
	{
		if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_COLUMN))
			PRVTRACE(L"[DBPROPSET_COLUMN]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DATASOURCE))
			PRVTRACE(L"[DBPROPSET_DATASOURCE]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DATASOURCEALL))
			PRVTRACE(L"[DBPROPSET_DATASOURCEALL]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DATASOURCEINFOALL))
			PRVTRACE(L"[DBPROPSET_DATASOURCEINFOALL]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DATASOURCEINFO))
			PRVTRACE(L"[DBPROPSET_DATASOURCEINFO]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DBINIT))
			PRVTRACE(L"[DBPROPSET_DBINIT]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_DBINITALL))
			PRVTRACE(L"[DBPROPSET_DBINITALL]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_INDEX))
			PRVTRACE(L"[DBPROPSET_INDEX]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_ROWSET))
			PRVTRACE(L"[DBPROPSET_ROWSET]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_ROWSETALL))
			PRVTRACE(L"[DBPROPSET_ROWSETALL]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_SESSION))
			PRVTRACE(L"[DBPROPSET_SESSION]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_SESSIONALL))
			PRVTRACE(L"[DBPROPSET_SESSIONALL]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_TABLE))
			PRVTRACE(L"[DBPROPSET_TABLE]\n");
		else if(IsEqualGUID(m_rgDBPROPINFOSET[index1].guidPropertySet,DBPROPSET_PROPERTIESINERROR))
			PRVTRACE(L"[DBPROPSET_PROPERTIESINERROR]\n");
		else
			PRVTRACE(L"[Unknown DBPROPSET]\n");

		for(index2=0;index2<m_rgDBPROPINFOSET[index1].cPropertyInfos;index2++)
		{
			
			PRVTRACE(L"[desc=%s],[propid=%d],[dwflags=%d],[vartype=%d]\n",
					m_rgDBPROPINFOSET[index1].rgPropertyInfos[index2].pwszDescription,
					m_rgDBPROPINFOSET[index1].rgPropertyInfos[index2].dwPropertyID,
					m_rgDBPROPINFOSET[index1].rgPropertyInfos[index2].dwFlags,
					m_rgDBPROPINFOSET[index1].rgPropertyInfos[index2].vtType);
		}

	}

CLEANUP:

	
	if(pIDBProperties)
		pIDBProperties->Release();

	return hr;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ShouldTestSchemaRestriction
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT CSchemaTest::ShouldTestSchemaRestriction(GUID schema,ULONG restrictions)
{
	// Figure out the return code
	for(ULONG index=0; index < m_cSchemasSupported; index++)
	{
		if(IsEqualGUID(schema,m_rgSchemas[index]))
		{
			if( (ALLRES & restrictions) || 
				(m_rgRestrictionSupport[index] & restrictions) )
				return S_OK;
			
			if(restrictions <= NumberofRestrictions(schema))
				return E_INVALIDARG;

			break;
		}
	}

	return E_INVALIDARG;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// NumberofRestrictions
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ULONG CSchemaTest::NumberofRestrictions(GUID schema)
{
	// all schemas with 0 restrictions
	// don't need to check schemas anyway
	if(IsEqualGUID(schema,DBSCHEMA_SQL_LANGUAGES))
		return 0;

	// all schemas with 1 retriction
	if(IsEqualGUID(schema,DBSCHEMA_CATALOGS))
		return 1;

	// all schemas with 2 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_PROVIDER_TYPES))
		return 3;

	// all schemas with 3 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_ASSERTIONS) ||
		IsEqualGUID(schema,DBSCHEMA_CHARACTER_SETS) ||
		IsEqualGUID(schema,DBSCHEMA_CHECK_CONSTRAINTS) ||
		IsEqualGUID(schema,DBSCHEMA_COLLATIONS) ||
		IsEqualGUID(schema,DBSCHEMA_CONSTRAINT_TABLE_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_PRIMARY_KEYS) ||
		IsEqualGUID(schema,DBSCHEMA_REFERENTIAL_CONSTRAINTS) ||
		IsEqualGUID(schema,DBSCHEMA_SCHEMATA) ||
		IsEqualGUID(schema,DBSCHEMA_STATISTICS) ||
		IsEqualGUID(schema,DBSCHEMA_TRANSLATIONS) ||
		IsEqualGUID(schema,DBSCHEMA_VIEW_COLUMN_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_VIEW_TABLE_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_VIEWS))
		return 7;

	// all schemas with 4 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_COLUMN_DOMAIN_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_COLUMNS) ||
		IsEqualGUID(schema,DBSCHEMA_CONSTRAINT_COLUMN_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_PROCEDURE_COLUMNS) ||
		IsEqualGUID(schema,DBSCHEMA_PROCEDURE_PARAMETERS) ||
		IsEqualGUID(schema,DBSCHEMA_PROCEDURES) ||
		IsEqualGUID(schema,DBSCHEMA_TABLES) || 
		IsEqualGUID(schema,DBSCHEMA_TABLES_INFO)) 
		return 15;

	// all schemas with 5 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_INDEXES) ||
		IsEqualGUID(schema,DBSCHEMA_TABLE_PRIVILEGES))
		return 31;
	
	// all schemas with 6 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_COLUMN_PRIVILEGES) ||
		IsEqualGUID(schema,DBSCHEMA_FOREIGN_KEYS) ||
		IsEqualGUID(schema,DBSCHEMA_USAGE_PRIVILEGES))
		return 63;

	// all schemas with 7 restrictions
	if(IsEqualGUID(schema,DBSCHEMA_KEY_COLUMN_USAGE) ||
		IsEqualGUID(schema,DBSCHEMA_TABLE_CONSTRAINTS)||
		IsEqualGUID(schema,DBSCHEMA_TABLE_STATISTICS))
		return 127;

	return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Set Bit
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSchemaTest::SetRestriction(ULONG bit)
{
	m_restrict |= bit;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Clear Bit
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSchemaTest::ClearRestriction(ULONG bit)
{
	m_restrict &= ~(bit);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Schema Restrictions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::TestSchemaRestrictions(GUID schema, ULONG bit, ROW_COUNT eRowCount, ULONG cExpRows)
{
	HRESULT hr	  = E_FAIL;
	HRESULT ExpHR = S_OK;
	
	// Initialize the needed pointers
	INIT;

	// This is not initialized on purpose
	ExpHR=ShouldTestSchemaRestriction(schema,bit);

	if(FAILED(ExpHR)) 
	{
		odtLog <<wszRESTRICTIONNOTSUPPORTED;
		m_fPassUnsupportedRestrictions = TRUE;
	}

	// Set the correct info
	SetRestriction(bit);
	m_iid = IID_IRowset;

	if(GetSchemaInfo(SPECIFIC,0,schema))
	{
		// Update expected row count if required for this variation.
		if (eRowCount != DEFAULT)
			SetRowCount(eRowCount, cExpRows);

		// Test method with invalid schema guid
		hr=GetRowset();

		// Provider either returns E_INVALIDARG or DB_E_NOTSUPPORTED
		if( (FAILED(ExpHR) && (hr == E_INVALIDARG || hr == DB_E_NOTSUPPORTED)) ||
			(CHECK(hr, ExpHR)) )
			m_fResult = TEST_PASS;
	}

	FREE;

	return m_fResult;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Schema Restrictions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::TestReturnData(DBCOUNTITEM iRow, DATA *	pColumn, ULONG bit, BOOL * fRes, WCHAR * wszRes, BOOL fNullable)
{
	BOOL fResults = TRUE;

	if(m_currentBitMask & bit || !fNullable)
	{
		// Check Status and Value
		if( (*fRes) && 
			((!COMPARE((iRow>=1), TRUE)) || 
			 (!COMPARE(pColumn->sStatus, DBSTATUS_S_OK)) ||
			 ((m_restrict & bit) &&
			  ((!COMPARE(pColumn->ulLength, wcslen(wszRes)*2)) ||
			   (!COMPARE(0, RelCompareString(wszRes, (TYPE_WSTR)pColumn->bValue)))))) )
			*fRes = fResults = FALSE;
	}
	else
	{
		COMPARE(pColumn->sStatus, DBSTATUS_S_ISNULL);
//		COMPARE(pColumn->ulLength, 0);  // Spec does not require this.  Length should be ignored for NULL.
	}

	return fResults;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test the Restriction for IDBSchemaRowset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSchemaTest::SetRestriction(RESTRICTIONSENUM bit, ULONG ulRestriction, WCHAR ** pwszR, WCHAR * wszRestriction)
{
	// Check if the restriction is supported
	if((m_currentBitMask & bit) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & bit) || (m_restrict & ALLRES))
		{
			// if NULL set to a bogus restriction
			m_rgvarRestrict[ulRestriction-1].bstrVal = SysAllocString(wszRestriction ? wszRestriction : L"BogusRestriction");
			*pwszR = wcsDuplicate(wszRestriction ? wszRestriction : L"BogusRestriction");

			m_rgvarRestrict[ulRestriction-1].vt = VT_BSTR;
			m_cRestrictionsCurrent ++;
			SetRestriction(bit);
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init_Stuff
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSchemaTest::Init_Stuff(void)
{
	ULONG index;

	m_fBOOKMARK_REQUESTED=FALSE;
	m_fBOOKMARK_FOUND=FALSE;
	m_fRes1=TRUE;
	m_fRes2=TRUE;
	m_fRes3=TRUE;
	m_fRes4=TRUE;
	m_fRes5=TRUE;
	m_fRes6=TRUE;
	m_fRes7=TRUE;
	m_fMyTableCatalogFound=FALSE;
	m_fMyTableSchemaFound=FALSE;
	m_MyTableName1Found=FALSE;
	m_MyTableName2Found=FALSE;
	m_MyColumnNameFound=FALSE;
	m_MyTableTypeFound=FALSE;
	
	m_cErrors=0;
	m_cColumns=0;				
	m_cRestrictions=0;	
	m_cRestrictionsCurrent=0;	
	m_cDBBINDING=0;			
	m_cDBPROPSET=0;
	m_cDBCOLUMNINFO=0;		
	m_iid=IID_NULL;					
	m_guid=GUID_NULL;					
	m_restrict=0;
	m_HR=E_FAIL;
	m_fResult=FALSE;
	m_fAtLeast1UnsupportedRestrictionIsSet=FALSE;


	m_fRowsetPointerNULL=FALSE;
	m_fCountRestrictionsNULL=FALSE;
	m_fRangeRestrictionsNULL=FALSE;
	m_fCountPropSetNULL=FALSE;
	m_fRangePropSetNULL=FALSE;
	m_fPassUnsupportedRestrictions=FALSE;
	m_fRestrictionsAsInitVariants=FALSE;

	
	m_wszR1=NULL;
	m_wszR2=NULL;
	m_wszR3=NULL;
	m_wszR4=NULL;								    
	m_wszR5=NULL;
	m_wszR6=NULL;
	m_wszR7=NULL;
	m_ulR=0;

	for(index=0;index<MAXRESTRICTION;index++)
		VariantInit(&(m_rgvarRestrict[index]));

	for(index=0;index<MAXPROP;index++)
		VariantInit(&(m_rgDBPROP[index].vValue));


}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Free_Stuff
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CSchemaTest::Free_Stuff(void)
{
	ULONG index;

	// don't need to be freed because these aren't dynamic
 	m_rgColumnNames=NULL;
	m_rgColumnTypes=NULL;
	
	SAFE_RELEASE_(m_pIRowset);
	SAFE_RELEASE_(m_pIAccessor);
	SAFE_RELEASE_(m_pIRowsetInfo);
	SAFE_RELEASE_(m_pIColumnsInfo);
	SAFE_RELEASE_(m_pIRowsetChange);
	SAFE_RELEASE_(m_pIUnknown);

	PROVIDER_FREE(m_rgDBCOLUMNINFO);
	PROVIDER_FREE(m_pStringsBuffer);

	for(index=0;index<MAXRESTRICTION;index++)
  		VariantClear(&(m_rgvarRestrict[index]));

	for(index=0;index<MAXPROP;index++)
		VariantClear(&(m_rgDBPROP[index].vValue));

	// Free the Restrictions
	PROVIDER_FREE(m_wszR1);
	PROVIDER_FREE(m_wszR2);
	PROVIDER_FREE(m_wszR3);
	PROVIDER_FREE(m_wszR4);
	PROVIDER_FREE(m_wszR5);
	PROVIDER_FREE(m_wszR6);
	PROVIDER_FREE(m_wszR7);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowset, execute method and check results
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT CSchemaTest::GetRowset(BOOL fFreeRowset)
{
	IUnknown * pIUnknown = NULL;
	IUnknown **ppIUnknown=&pIUnknown;

	if(!m_fCaptureRestrictions && m_PrintSchemaName)
		TraceSchemaName(m_guid, TRUE, TRUE);

	if(m_iid==IID_IRowset)
		ppIUnknown = (IUnknown **)&m_pIRowset;
	else if(m_iid==IID_IAccessor)
		ppIUnknown = (IUnknown **)&m_pIAccessor;
	else if(m_iid==IID_IRowsetInfo)
		ppIUnknown = (IUnknown **)&m_pIRowsetInfo;
	else if(m_iid==IID_IColumnsInfo)
		ppIUnknown = (IUnknown **)&m_pIColumnsInfo;
	else if(m_iid==IID_IRowsetChange)
		ppIUnknown = (IUnknown **)&m_pIRowsetChange;
	else if(m_iid==IID_IUnknown)
		ppIUnknown = (IUnknown **)&m_pIUnknown;

	// Now get Schema rowset.
	m_HR=m_pIDBSchemaRowset->GetRowset(
		m_punkOuter, m_guid, (m_fCountRestrictionsNULL ? 0 : m_cRestrictions),
		(m_fRangeRestrictionsNULL ? NULL : m_rgvarRestrict), m_iid,
		(m_fCountPropSetNULL ? 0 : m_cDBPROPSET), (m_fRangePropSetNULL ? 0 : m_rgDBPROPSET),
		(m_fRowsetPointerNULL ? NULL : ppIUnknown));

	if (SUCCEEDED(m_HR))
		CheckResults(*ppIUnknown, m_iid);

	if (fFreeRowset && ppIUnknown)
		SAFE_RELEASE(*ppIUnknown);

	return m_HR;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This routine should be called when the information for the next schema (guid) is needed
// I either want the next supported schema or a specific schema.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::GetSchemaInfo
(
	REQUESTED_SCHEMA schemaType,			// [IN] if I want the schema supported, or not, or I have a specific schema in mind
	ULONG ulIndexOfSchemaRequesting,		// [IN] Index of supported Schema
	GUID schema								// [IN] specific schema
)
{
	BOOL fResult = FALSE;
	m_cRestrictionsCurrent = 0;

	// Set default row count information to expect 1 row but only
	// warn if we get less
	SetRowCount(MIN_VALUE, 1);

	// Initialize Variants
	VariantInit(&m_rgvarRestrict[0]);
	VariantInit(&m_rgvarRestrict[1]);
	VariantInit(&m_rgvarRestrict[2]);
	VariantInit(&m_rgvarRestrict[3]);
	VariantInit(&m_rgvarRestrict[4]);
	VariantInit(&m_rgvarRestrict[5]);
	VariantInit(&m_rgvarRestrict[6]);

	// grab schema from list of supported schemas
	if(schemaType==SUPPORTED)
		memcpy(&m_guid,&(m_rgSchemas[ulIndexOfSchemaRequesting]),sizeof(GUID));
	else if(schemaType==SPECIFIC)
		memcpy(&m_guid,&schema,sizeof(GUID));
	else
		return FALSE;

	for(ULONG index=0;index<m_cSchemasSupported;index++)
	{
		if(IsEqualGUID(m_guid,m_rgSchemas[index])) {
			m_currentBitMask = m_rgRestrictionSupport[index];
			break;
		}
	}

 	PRVTRACE(L"Getting ");
	TraceSchemaName(m_guid,FALSE,TRUE);
 	PRVTRACE(L"\n");

	// Find the schema
	if(IsEqualGUID(DBSCHEMA_ASSERTIONS,m_guid))
		fResult= PrepareParams_ASSERTIONS();
	else if(IsEqualGUID(DBSCHEMA_CATALOGS,m_guid))
		fResult= PrepareParams_CATALOGS();
	else if(IsEqualGUID(DBSCHEMA_CHARACTER_SETS,m_guid))
 		fResult= PrepareParams_CHARACTER_SETS();
	else if(IsEqualGUID(DBSCHEMA_CHECK_CONSTRAINTS,m_guid))
 		fResult= PrepareParams_CHECK_CONSTRAINTS();
	else if(IsEqualGUID(DBSCHEMA_COLLATIONS,m_guid))
 		fResult= PrepareParams_COLLATIONS();
	else if(IsEqualGUID(DBSCHEMA_COLUMN_DOMAIN_USAGE,m_guid))
 		fResult= PrepareParams_COLUMN_DOMAIN_USAGE();
	else if(IsEqualGUID(DBSCHEMA_COLUMN_PRIVILEGES,m_guid))
 		fResult= PrepareParams_COLUMN_PRIVILEGES();
	else if(IsEqualGUID(DBSCHEMA_COLUMNS,m_guid))
		fResult= PrepareParams_COLUMNS();
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_COLUMN_USAGE,m_guid))
 		fResult= PrepareParams_CONSTRAINT_COLUMN_USAGE();
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_TABLE_USAGE,m_guid))
 		fResult= PrepareParams_CONSTRAINT_TABLE_USAGE();
	else if(IsEqualGUID(DBSCHEMA_FOREIGN_KEYS,m_guid))
 		fResult= PrepareParams_FOREIGN_KEYS();
	else if(IsEqualGUID(DBSCHEMA_INDEXES,m_guid))
 		fResult= PrepareParams_INDEXES();
	else if(IsEqualGUID(DBSCHEMA_KEY_COLUMN_USAGE,m_guid))
 		fResult= PrepareParams_KEY_COLUMN_USAGE();
	else if(IsEqualGUID(DBSCHEMA_PRIMARY_KEYS,m_guid))
 		fResult= PrepareParams_PRIMARY_KEYS();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_COLUMNS,m_guid))
 		fResult= PrepareParams_PROCEDURE_COLUMNS();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_PARAMETERS, m_guid))
 		fResult= PrepareParams_PROCEDURE_PARAMETERS();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURES,m_guid))
 		fResult= PrepareParams_PROCEDURES();
	else if(IsEqualGUID(DBSCHEMA_PROVIDER_TYPES,m_guid))
		fResult= PrepareParams_PROVIDER_TYPES();
	else if(IsEqualGUID(DBSCHEMA_REFERENTIAL_CONSTRAINTS,m_guid))
 		fResult= PrepareParams_REFERENTIAL_CONSTRAINTS();
	else if(IsEqualGUID(DBSCHEMA_SCHEMATA,m_guid))
 		fResult= PrepareParams_SCHEMATA();
	else if(IsEqualGUID(DBSCHEMA_SQL_LANGUAGES,m_guid))
 		fResult= PrepareParams_SQL_LANGUAGES();
	else if(IsEqualGUID(DBSCHEMA_STATISTICS,m_guid))
 		fResult= PrepareParams_STATISTICS();
	else if(IsEqualGUID(DBSCHEMA_TABLES,m_guid))
		fResult= PrepareParams_TABLES();
	else if(IsEqualGUID(DBSCHEMA_TABLES_INFO,m_guid))
		fResult= PrepareParams_TABLES_INFO();
	else if(IsEqualGUID(DBSCHEMA_TABLE_CONSTRAINTS,m_guid))
 		fResult= PrepareParams_TABLE_CONSTRAINTS();
	else if(IsEqualGUID(DBSCHEMA_TABLE_PRIVILEGES,m_guid))
 		fResult= PrepareParams_TABLE_PRIVILEGES();
	else if(IsEqualGUID(DBSCHEMA_TABLE_STATISTICS,m_guid))
 		fResult= PrepareParams_TABLE_STATISTICS();
	else if(IsEqualGUID(DBSCHEMA_TRANSLATIONS,m_guid))
 		fResult= PrepareParams_TRANSLATIONS();
	else if(IsEqualGUID(DBSCHEMA_TRUSTEE,m_guid))
		fResult = PrepareParams_TRUSTEE();
	else if(IsEqualGUID(DBSCHEMA_USAGE_PRIVILEGES,m_guid))
 		fResult= PrepareParams_USAGE_PRIVILEGES();
	else if(IsEqualGUID(DBSCHEMA_VIEW_COLUMN_USAGE,m_guid))
 		fResult= PrepareParams_VIEW_COLUMN_USAGE();
	else if(IsEqualGUID(DBSCHEMA_VIEW_TABLE_USAGE,m_guid))
 		fResult= PrepareParams_VIEW_TABLE_USAGE();
	else if(IsEqualGUID(DBSCHEMA_VIEWS,m_guid))
 		fResult= PrepareParams_VIEWS();
	else
		return FALSE;

	if(!m_fRestrictionsAsInitVariants)
	{
		if(m_rgvarRestrict[0].vt == VT_EMPTY)
			ClearRestriction(FIRST);
		if(m_rgvarRestrict[1].vt == VT_EMPTY)
			ClearRestriction(SECOND);
		if(m_rgvarRestrict[2].vt == VT_EMPTY)
			ClearRestriction(THIRD);
		if(m_rgvarRestrict[3].vt == VT_EMPTY)
			ClearRestriction(FOURTH);
		if(m_rgvarRestrict[4].vt == VT_EMPTY)
			ClearRestriction(FIFTH);
		if(m_rgvarRestrict[5].vt == VT_EMPTY)
			ClearRestriction(SIXTH);
		if(m_rgvarRestrict[6].vt == VT_EMPTY)
			ClearRestriction(SEVENTH);
	}

	return fResult;
}	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRow
//
//	Pass through function to VerifyRow for specific schema
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow
(
	GUID	m_guid,
	DBCOUNTITEM	iRow,
	BYTE * pRow
)
{
		// 1. ASSERTIONS
		if(IsEqualGUID(m_guid,DBSCHEMA_ASSERTIONS))
 			return VerifyRow_ASSERTIONS(iRow,pRow);
		// 2. CATALOGS
		else if(IsEqualGUID(m_guid,DBSCHEMA_CATALOGS))
 			return VerifyRow_CATALOGS(iRow,pRow);
		// 3. CHARACTER_SETS
		else if(IsEqualGUID(m_guid,DBSCHEMA_CHARACTER_SETS))
 			return VerifyRow_CHARACTER_SETS(iRow,pRow);
		// 4. CHECK_CONSTRAINTS
		else if(IsEqualGUID(m_guid,DBSCHEMA_CHECK_CONSTRAINTS))
 			return VerifyRow_CHECK_CONSTRAINTS(iRow,pRow);
		// 5. COLLATIONS
		else if(IsEqualGUID(m_guid,DBSCHEMA_COLLATIONS))
 			return VerifyRow_COLLATIONS(iRow,pRow);
		// 6. COLUMN_DOMAIN_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_COLUMN_DOMAIN_USAGE))
 			return VerifyRow_COLUMN_DOMAIN_USAGE(iRow,pRow);
		// 7. COLUMN_PRIVILEGES
		else if(IsEqualGUID(m_guid,DBSCHEMA_COLUMN_PRIVILEGES))
 			return VerifyRow_COLUMN_PRIVILEGES(iRow,pRow);
		// 8. COLUMNS
		else if(IsEqualGUID(m_guid,DBSCHEMA_COLUMNS))
 			return VerifyRow_COLUMNS(iRow,pRow);
		// 9. CONSTRAINT_COLUMN_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_CONSTRAINT_COLUMN_USAGE))
 			return VerifyRow_CONSTRAINT_COLUMN_USAGE(iRow,pRow);
		// 10. CONSTRAINT_TABLE_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_CONSTRAINT_TABLE_USAGE))
 			return VerifyRow_CONSTRAINT_TABLE_USAGE(iRow,pRow);
		// 11. FOREIGN_KEYS
		else if(IsEqualGUID(m_guid,DBSCHEMA_FOREIGN_KEYS))
 			return VerifyRow_FOREIGN_KEYS(iRow,pRow);
		// 12. INDEXES
		else if(IsEqualGUID(m_guid,DBSCHEMA_INDEXES))
 			return VerifyRow_INDEXES(iRow,pRow);
		// 13. KEY_COLUMN_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_KEY_COLUMN_USAGE))
 			return VerifyRow_KEY_COLUMN_USAGE(iRow,pRow);
 		// 14. PRIMARY_KEYS
		else if(IsEqualGUID(m_guid,DBSCHEMA_PRIMARY_KEYS))
 			return VerifyRow_PRIMARY_KEYS(iRow,pRow);
 		// 14. PROCEDURE_COLUMNS
		else if(IsEqualGUID(m_guid,DBSCHEMA_PROCEDURE_COLUMNS))
 			return VerifyRow_PROCEDURE_COLUMNS(iRow,pRow);
		// 15. PROCEDURE_PARAMETERS
		else if(IsEqualGUID(m_guid,DBSCHEMA_PROCEDURE_PARAMETERS))
			return VerifyRow_PROCEDURE_PARAMETERS(iRow,pRow);
		// 16. PROCEDURES
		else if(IsEqualGUID(m_guid,DBSCHEMA_PROCEDURES))
 			return VerifyRow_PROCEDURES(iRow,pRow);
		// 17. PROVIDER TYPES
		else if(IsEqualGUID(m_guid,DBSCHEMA_PROVIDER_TYPES))
 			return VerifyRow_PROVIDER_TYPES(iRow,pRow);
		// 18. REFERENTIAL_CONSTRAINTS
		else if(IsEqualGUID(m_guid,DBSCHEMA_REFERENTIAL_CONSTRAINTS))
 			return VerifyRow_REFERENTIAL_CONSTRAINTS(iRow,pRow);
		// 19. SCHEMATA
		else if(IsEqualGUID(m_guid,DBSCHEMA_SCHEMATA))
 			return VerifyRow_SCHEMATA(iRow+1,pRow);
		// 20. SQL_LANGUAGES
		else if(IsEqualGUID(m_guid,DBSCHEMA_SQL_LANGUAGES))
 			return VerifyRow_SQL_LANGUAGES(iRow,pRow);
		// 21. STATISTICS
		else if(IsEqualGUID(m_guid,DBSCHEMA_STATISTICS))
 			return VerifyRow_STATISTICS(iRow,pRow);
		// 22. TABLE_CONSTRAINTS
		else if(IsEqualGUID(m_guid,DBSCHEMA_TABLE_CONSTRAINTS))
 			return VerifyRow_TABLE_CONSTRAINTS(iRow,pRow);
		// 23. TABLE_PRIVILEGES
		else if(IsEqualGUID(m_guid,DBSCHEMA_TABLE_PRIVILEGES))
 			return VerifyRow_TABLE_PRIVILEGES(iRow,pRow);
		else if(IsEqualGUID(m_guid,DBSCHEMA_TABLE_STATISTICS))
 			return VerifyRow_TABLE_STATISTICS(iRow,pRow);
		// 24. TABLES
		else if(IsEqualGUID(m_guid,DBSCHEMA_TABLES))
 			return VerifyRow_TABLES(iRow,pRow);
		// 25. TABLES_INFO
		else if(IsEqualGUID(m_guid,DBSCHEMA_TABLES_INFO))
 			return VerifyRow_TABLES_INFO(iRow,pRow);
		// 26. TRANSLATIONS
		else if(IsEqualGUID(m_guid,DBSCHEMA_TRANSLATIONS))
 			return VerifyRow_TRANSLATIONS(iRow,pRow);
		// 27. USAGE_PRIVILEGES
		else if(IsEqualGUID(m_guid,DBSCHEMA_USAGE_PRIVILEGES))
 			return VerifyRow_USAGE_PRIVILEGES(iRow,pRow);
		// 28. VIEW_COLUMN_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_VIEW_COLUMN_USAGE))
 			return VerifyRow_VIEW_COLUMN_USAGE(iRow,pRow);
		// 29. VIEW_TABLE_USAGE
		else if(IsEqualGUID(m_guid,DBSCHEMA_VIEW_TABLE_USAGE))
 			return VerifyRow_VIEW_TABLE_USAGE(iRow,pRow);
		// 30. VIEWS
		else if(IsEqualGUID(m_guid,DBSCHEMA_VIEWS))
 			return VerifyRow_VIEWS(iRow,pRow);
		else {
			odtLog << L"Warning - Provider specific schema rowset found.\n";
			_ASSERTE(!L"Unexpected schema rowset.");
			return FALSE;
		}

	return FALSE;
}
//--------------------------------------------------------------------
// Capture Restrictions from schema rowsets returned, if schema is 
// supported
//
//--------------------------------------------------------------------
BOOL CSchemaTest::CaptureRestrictions()
{
	// Need to turn Capture off
	m_fCaptureRestrictions = TRUE;

	Find_Catalog_and_Schema();
	Find_TableInfo();
	Find_Assertion_Constraint();
	Find_Character_Set();
	Find_Collation();
	Find_Domain();
	Find_Grantor_and_Grantee();
	Find_Key_Column_Usage_Constraint();
	Find_Procedure(); 

	//I'm wrapping all procedure stuff into one place
	//Find_ProcedureColumn();
	//Find_Parameter();
	Find_Referential_Constraint();
	Find_Constraint_Type();
	Find_Translation();
	Find_View();
	Find_BestMatch();
	Find_PK_and_FK();
	Find_Table_Statistics_Restrictions();

	// Need to turn Capture off
	m_fDontCaptureRestrictions = TRUE;
	m_fCaptureRestrictions=FALSE;

	return TRUE;
}
//--------------------------------------------------------------------
// Free Restrictions that were captured
//--------------------------------------------------------------------
void CSchemaTest::FreeRestrictions()
{
	Release_Catalog_and_Schema();
	Release_TableInfo();
	Release_Assertion_Constraint();
	Release_Character_Set();
	Release_Collation();
	Release_Domain();
	Release_Grantor_and_Grantee();
	Release_Key_Column_Usage_Constraint();
	Release_Procedure();
	Release_Parameter();
	Release_Referential_Constraint();
	Release_Constraint_Type();
	Release_Translation();
	Release_View();
	Release_PK_and_FK();
	Release_Table_Statistics_Restrictions();

	// Need to Capture the Restrictions
	m_fDontCaptureRestrictions = FALSE;
}

//--------------------------------------------------------------------
// Is this schema supported
//--------------------------------------------------------------------
BOOL CSchemaTest::IsSchemaSupported(GUID schema)
{
	// grab schema from list of supported schemas
	for(ULONG index=0; index<m_cSchemasSupported; index++)
	{
		if(IsEqualGUID(schema,m_rgSchemas[index]))
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------
// Find Catalog
//
// With DBSCHEMA_CATALOGS, will fill m_pwszCatalogRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Catalog_and_Schema()
{
	// Initialize the needed pointers
	INIT;

	// Call the GetProperty for the CurrentCatalog
	GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, m_pIDBInitialize, &m_pwszCatalogRestriction);

	// Call the GetProperty for the UserName
	GetProperty(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &m_pwszSchemaRestriction);

	FREE;
	
	return TRUE;
}
//--------------------------------------------------------------------
// Find Catalog
//
// With DBSCHEMA_CATALOGS, will fill m_pwszCatalogRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Catalog_and_Schema()
{
	// Free the memory
	PROVIDER_FREE(m_pwszCatalogRestriction);
	PROVIDER_FREE(m_pwszSchemaRestriction);

	return TRUE;
}
//--------------------------------------------------------------------
// Find Tables
//
// With DBSCHEMA_TABLES, will fill m_pwszTableRestriction and m_pwszTable_TypeRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_TableInfo()
{
	// Initialize the needed pointers
	INIT;

	// Get Index Name
	if(m_pTable->GetIndexName()) {
		PROVIDER_FREE(m_pwszIndexRestriction);
		m_pwszIndexRestriction = wcsDuplicate(m_pTable->GetIndexName());
	}

	// Get Table Name
	if(m_pTable->GetTableName()) {
		VARIANT Variant;
		PROVIDER_FREE(m_pwszTableRestriction);
		m_pwszTableRestriction = wcsDuplicate(m_pTable->GetTableName());
		m_pwszTable_TypeRestriction = wcsDuplicate(L"TABLE");

		// Adjust for identifier case
		VariantInit(&Variant);
		GetProperty(DBPROP_IDENTIFIERCASE, 
			DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &Variant);
		switch(Variant.lVal)
		{
			case DBPROPVAL_IC_UPPER:
				_wcsupr(m_pwszTableRestriction);
				break;
			case DBPROPVAL_IC_LOWER:
				_wcslwr(m_pwszTableRestriction);
				break;
		}

	}
	
	// Get Column Name
	CCol col;
	DBORDINAL m_cOrdinals = m_pTable->CountColumnsOnTable();

	for(ULONG ulIndex = 1; m_cOrdinals >= ulIndex; ulIndex++)
	{
		if(SUCCEEDED(m_pTable->GetColInfo(1,col)) && col.GetColName()) 
		{
			// Copy the new values
			m_DataTypeRestriction = col.GetProviderType();

			PROVIDER_FREE(m_pwszColumnRestriction);
			m_pwszColumnRestriction = wcsDuplicate(col.GetColName());
			break;
		}
	}

	FREE;

	return TRUE;
}
//--------------------------------------------------------------------
// Release Tables
//
// With DBSCHEMA_TABLES, will free m_pwszTableRestriction, m_pwszColumnRestriction, and m_pwszIndexRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_TableInfo()
{
	// Free the memory
	PROVIDER_FREE(m_pwszTableRestriction);
	PROVIDER_FREE(m_pwszTable_TypeRestriction);
	PROVIDER_FREE(m_pwszColumnRestriction);
	PROVIDER_FREE(m_pwszIndexRestriction);
	return TRUE;
}

//--------------------------------------------------------------------
// Find Assertion Constraint
//
// With DBSCHEMA_ASSERTIONS, will fill m_pwszAssertion_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Assertion_Constraint()
{
	// Initialize the needed pointers
	INIT;

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_ASSERTIONS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
 	m_iid = IID_IRowset;

	// If this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_ASSERTIONS))
		GetRowset();

	FREE;

	return TRUE;
}
//--------------------------------------------------------------------
// Find Assertion Constraint
//
// With DBSCHEMA_ASSERTIONS, will fill m_pwszAssertion_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Assertion_Constraint()
{
	// Free the memory
	PROVIDER_FREE(m_pwszAssertion_ConstraintRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Character Set
//
// With DBSCHEMA_CHARACTER_SETS, will fill m_pwszCharacter_SetRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Character_Set()
{
	// Initialize the needed pointers
	INIT;

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_CHARACTER_SETS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_CHARACTER_SETS))
		GetRowset();

	FREE;

	return TRUE;
}
//--------------------------------------------------------------------
// Find Character Set
//
// With DBSCHEMA_CHARACTER_SETS, will fill m_pwszCharacter_SetRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Character_Set()
{
	// Free the memory
	PROVIDER_FREE(m_pwszCharacter_SetRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Check Constraint
//
// With DBSCHEMA_CHECK_CONSTRAINTS, will fill m_pwszCheck_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Check_Constraint()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_CHECK_CONSTRAINTS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_CHECK_CONSTRAINTS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Check Constraint
//
// With DBSCHEMA_CHECK_CONSTRAINTS, will fill m_pwszCheck_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Check_Constraint()
{
	// Free the memory
	PROVIDER_FREE(m_pwszCheck_ConstraintRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Collation
//
// With DBSCHEMA_COLLATIONS, will fill m_pwszCollationRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Collation()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_COLLATIONS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_COLLATIONS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Collation
//
// With DBSCHEMA_COLLATIONS, will fill m_pwszCollationRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Collation()
{
	// Free the memory
	PROVIDER_FREE(m_pwszCollationRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Domain
//
// With DBSCHEMA_COLUMN_DOMAIN_USAGE, will fill m_pwszDomainRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Domain()
{
	INIT;

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_COLUMN_DOMAIN_USAGE))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= FOURTH;
 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_COLUMN_DOMAIN_USAGE))
		GetRowset();

	FREE;

	return TRUE;
}
//--------------------------------------------------------------------
// Find Domain
//
// With DBSCHEMA_COLUMN_DOMAIN_USAGE, will fill m_pwszDomainRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Domain()
{
	// Free the memory
	PROVIDER_FREE(m_pwszDomainRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Grantor and Grantee
//
// With DBSCHEMA_COLUMN_PRIVILEGES, will fill m_pwszGrantorRestriction and m_pwszGranteeRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Grantor_and_Grantee()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_COLUMN_PRIVILEGES))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= THIRD;
	m_restrict |= FOURTH;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_COLUMN_PRIVILEGES))
	{
		// Since we're restricting COLUMN_PRIVILEGES to a single column
		// we need to adjust expected row count to one row minimum
		SetRowCount(MIN_REQUIRED, 1);

		GetRowset();
	}

	FREE

	return TRUE;

}
//--------------------------------------------------------------------
// Find Grantor and Grantee
//
// With DBSCHEMA_COLUMN_PRIVILEGES, will fill m_pwszGrantorRestriction and m_pwszGranteeRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Grantor_and_Grantee()
{
	// Free the memory
	PROVIDER_FREE(m_pwszGrantorRestriction);
	PROVIDER_FREE(m_pwszGranteeRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Table Statistics Restrictions
//
// Uses DBSCHEMA_TABLE_STATISTICS to fill restrictions on
// STATISTICS_CATALOG, STATISTICS_SCHEMA, STATISTICS_NAME, and 
// STATISTICS_TYPE
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Table_Statistics_Restrictions()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_TABLE_STATISTICS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= THIRD;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_TABLE_STATISTICS))
	{
		// Execute the provider-specific statement to update statistics
		// for our test table
		COMPARE(UpdateStatistics(), TRUE);

		// We need at least one row in TABLE_STATISTICS rowset
		SetRowCount(MIN_REQUIRED, 1);

		GetRowset();
	}

	FREE

	return TRUE;

}
//--------------------------------------------------------------------
// Release Table Statistics Restrictions
//
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Table_Statistics_Restrictions()
{
	// Free the memory
	PROVIDER_FREE(m_pwszStatisticsCatalogRestriction);
	PROVIDER_FREE(m_pwszStatisticsSchemaRestriction);
	PROVIDER_FREE(m_pwszStatisticsNameRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Update Statistics
//
//--------------------------------------------------------------------
BOOL CSchemaTest::UpdateStatistics(void)
{
	LPWSTR pwszUpdateStats = NULL;
	size_t	ccUpdateStats = 0;
	BOOL	fReturn = FALSE;

	if (m_pwszUpdateStatsFormat)
	{
		ccUpdateStats = wcslen(m_pwszUpdateStatsFormat) +
						wcslen(m_pTable->GetTableName() +1);

		SAFE_ALLOC(pwszUpdateStats, WCHAR, ccUpdateStats);

		swprintf(pwszUpdateStats, m_pwszUpdateStatsFormat, m_pTable->GetTableName());

		TESTC_(m_pTable->BuildCommand(pwszUpdateStats, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL), S_OK);

	}

	fReturn = TRUE;

CLEANUP:

	SAFE_FREE(pwszUpdateStats);

	return fReturn;
}

//--------------------------------------------------------------------
// Find Key Column Usage Constraint
//
// With DBSCHEMA_KEY_COLUMN_USAGE, will fill m_pwszKey_Column_Usage_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Key_Column_Usage_Constraint()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_KEY_COLUMN_USAGE))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= FOURTH;
	m_restrict |= FIFTH;
	m_restrict |= SIXTH;
	m_restrict |= SEVENTH;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_KEY_COLUMN_USAGE))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Key Column Usage Constraint
//
// With DBSCHEMA_KEY_COLUMN_USAGE, will fill m_pwszKey_Column_Usage_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Key_Column_Usage_Constraint()
{
	// Free the memory
	PROVIDER_FREE(m_pwszKey_Column_Usage_ConstraintRestriction);
	return TRUE;

}
//--------------------------------------------------------------------
// Find Procedure
//
// With DBSCHEMA_PROCEDURE, will fill in 
// m_pwszProcedureRestriction and m_pwszProcedure_Type, 
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Procedure()
{
	BOOL			fSucceed		= FALSE;
	ICommandText *	pICommandText	= NULL;
	WCHAR *			pwszProcName	= NULL;
	WCHAR *			pSQL			= NULL;

	// Initialize the needed pointers
	INIT;
	
	if(!m_pIDBInitialize) {
		odtLog << L"m_pIDBInitialize is null\n";
		return TEST_FAIL;
	}


	if (!m_bSqlServer)
	{
//		odtLog << L"Not a sql server so can't test stored proc restrictions\n";
		goto CLEANUP;
	}

	if(!(m_pTable->get_ICommandPTR()))
		goto CLEANUP;

	if(!CHECK(m_pTable->get_ICommandPTR()->QueryInterface(IID_ICommandText,(void**)&pICommandText),S_OK))
		goto CLEANUP;

	pwszProcName = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(m_pTable->GetTableName()) +
		wcslen(wszProcDesignator)+1) * sizeof(WCHAR));

	if(!pwszProcName) {
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}

	wcscpy(pwszProcName, m_pTable->GetTableName());
	wcscat(pwszProcName, wszProcDesignator);

	m_pwszDropProc = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(wszDropProc) +
		wcslen(pwszProcName)+1) * sizeof(WCHAR));

	if(!m_pwszDropProc) {
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}

	swprintf(m_pwszDropProc, wszDropProc, pwszProcName);

	pSQL = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(wszCreateProc) + 
		wcslen(pwszProcName) +
		wcslen(m_pTable->GetTableName()) +
		1) * sizeof(WCHAR));

	if(!pSQL)
	{
		COMPARE(pSQL,NULL);
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}

	swprintf(pSQL, wszCreateProc, pwszProcName, m_pTable->GetTableName());

//CREATE PROCEDURE <procname> (@IntIn int =1, @IntOut int =2 OUTPUT) AS Select * from %s RETURN (1)";
// will give me:
//	1 input
//	1 output
//	1 return

	if(!CHECK(pICommandText->SetCommandText(DBGUID_DBSQL,m_pwszDropProc),S_OK))
		goto CLEANUP;

	pICommandText->Execute(NULL,IID_NULL,NULL,NULL,NULL);

	if(!CHECK(pICommandText->SetCommandText(DBGUID_DBSQL,pSQL),S_OK))
		goto CLEANUP;

	if(!CHECK(pICommandText->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	// fill these in
	m_pwszProcedureRestriction  = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(pwszProcName) +
		1) * sizeof(WCHAR*));

	if(!m_pwszProcedureRestriction)
	{
		COMPARE(m_pwszProcedureRestriction,NULL);
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}
		
	swprintf(m_pwszProcedureRestriction, pwszProcName);

	//WCHAR * m_pwszProcedureColumnsRestriction;
	// since we are doing select * from table we can use the 
	// column we found previously
	m_pwszProcedureColumnsRestriction  = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(m_pwszColumnRestriction) +
		1) * sizeof(WCHAR*));

	if(!m_pwszProcedureColumnsRestriction)
	{
		COMPARE(m_pwszProcedureColumnsRestriction,NULL);
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}
		
	swprintf(m_pwszProcedureColumnsRestriction, m_pwszColumnRestriction);

	//WCHAR * m_pwszParameterRestriction;
	m_pwszParameterRestriction  = (WCHAR *) PROVIDER_ALLOC(
		(wcslen(L"IntIn") +
		1) * sizeof(WCHAR*));

	if(!m_pwszParameterRestriction)
	{
		COMPARE(m_pwszParameterRestriction,NULL);
		odtLog << L"out of memory\n";
		goto CLEANUP;
	}
		
	swprintf(m_pwszParameterRestriction, L"IntIn");
	
	
	//WCHAR *	m_pwszProcedureType;
	// default will be return value but I'll have
	// to add more variations in the Procedure Parameter class
	// to test all 4 types.
	m_ProcedureTypeRestriction = DBPARAMTYPE_RETURNVALUE;
	m_uiProcedureType = DBPARAMTYPE_RETURNVALUE;

	fSucceed = TRUE;


CLEANUP:

	FREE;
	
	SAFE_RELEASE(pICommandText);

	// Free the memory
	PROVIDER_FREE(pSQL);
	PROVIDER_FREE(pwszProcName);
	
	return fSucceed;
}
//--------------------------------------------------------------------
// Find Procedure
//
// With DBSCHEMA_PROCEDURE, will fill in m_pwszProcedureRestriction and m_pwszProcedure_Type
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Procedure()
{
	BOOL fSucceed=FALSE;
	
	ICommandText * pICommandText=NULL;

	if(m_bSqlServer)
	{

		if(!(m_pTable->get_ICommandPTR()))
			goto CLEANUP;

		if(!CHECK(m_pTable->get_ICommandPTR()->QueryInterface(IID_ICommandText,(void**)&pICommandText),S_OK))
			goto CLEANUP;

		if (m_pwszDropProc)
		{
			if(!CHECK(pICommandText->SetCommandText(DBGUID_DBSQL,m_pwszDropProc),S_OK))
				goto CLEANUP;

			if(!CHECK(pICommandText->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
				goto CLEANUP;
		}

	CLEANUP:

		SAFE_RELEASE(pICommandText);

		// Free the memory
		PROVIDER_FREE(m_pwszDropProc);
		PROVIDER_FREE(m_pwszProcedureRestriction);
		PROVIDER_FREE(m_pwszProcedureColumnsRestriction);
		PROVIDER_FREE(m_pwszParameterRestriction);
	}

	return TRUE;
}
//--------------------------------------------------------------------
// Find Find_ProcedureColumn
//
// With DBSCHEMA_PROCEDURE_COLUMNS, will fill in 
// m_pwszProcedureColumnRestriction 
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_ProcedureColumn()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_PROCEDURE_COLUMNS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= THIRD;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_PROCEDURE_COLUMNS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Procedure
//
// With DBSCHEMA_PROCEDURE, will fill in m_pwszProcedureRestriction and m_pwszProcedure_Type
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_ProcedureColumn()
{
	// Free the memory
	PROVIDER_FREE(m_pwszProcedureColumnsRestriction);
	return TRUE;
}

//--------------------------------------------------------------------
// Find Parameter
//
// With DBSCHEMA_PROCEDURE_PARAMETER, will fill in m_pwszParameterRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Parameter()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_PROCEDURE_PARAMETERS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= THIRD;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_PROCEDURE_PARAMETERS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Parameter
//
// With DBSCHEMA_PROCEDURE_PARAMETER, will fill in m_pwszParameterRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Parameter()
{
	// Free the memory
	PROVIDER_FREE(m_pwszParameterRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Referential Constraint
//
// With DBSCHEMA_REFERENTIAL_CONSTRAINTS, will fill m_pwszReferential_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Referential_Constraint()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_REFERENTIAL_CONSTRAINTS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_REFERENTIAL_CONSTRAINTS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Referential Constraint
//
// With DBSCHEMA_REFERENTIAL_CONSTRAINTS, will fill m_pwszReferential_ConstraintRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Referential_Constraint()
{
	// Free the memory
	PROVIDER_FREE(m_pwszReferential_ConstraintRestriction);
	return TRUE;

}
//--------------------------------------------------------------------
// Find Table Constraint
//
// With DBSCHEMA_TABLE_CONSTRAINTS, will fill m_pwszTable_ConstraintRestriction
// and m_pwszTable_Constraint_Type
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Constraint_Type()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_TABLE_CONSTRAINTS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= FOURTH;
	m_restrict |= FIFTH;
//	m_restrict |= SIXTH;  // No restriction on table name because we didn't add one

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_TABLE_CONSTRAINTS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Table Constraint
//
// With DBSCHEMA_TABLE_CONSTRAINTS, will fill m_pwszTable_ConstraintRestriction
// and m_pwszTable_Constraint_Type
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Constraint_Type()
{
	// Free the memory
	PROVIDER_FREE(m_pwszTable_ConstraintRestriction);
	PROVIDER_FREE(m_pwszConstraint_TypeRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find Translation
//
// With DBSCHEMA_TRANSLATIONS, will fill in m_pwszTranslationReplace
//
// While the translation should be the same as the 
// character set, I'm not taking any chances
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_Translation()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_TRANSLATIONS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_TRANSLATIONS))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find Translation
//
// With DBSCHEMA_TRANSLATIONS, will fill in m_pwszTranslationReplace
//
// While the translation should be the same as the 
// character set, I'm not taking any chances
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_Translation()
{
	// Free the memory
	PROVIDER_FREE(m_pwszTranslationReplace);
	return TRUE;

}
//--------------------------------------------------------------------
// Find View
//
// With DBSCHEMA_VIEW_TABLE_USAGE, will fill in m_pwszViewRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_View()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_VIEW_TABLE_USAGE))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;

 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_VIEW_TABLE_USAGE))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find View
//
// With DBSCHEMA_VIEW_TABLE_USAGE, will fill in m_pwszViewRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_View()
{
	// Free the memory
	PROVIDER_FREE(m_pwszViewRestriction);
	return TRUE;
}
//--------------------------------------------------------------------
// Find BestMatch for DBSCHEMA_PROVIDER_TYPES
//
// With DBSCHEMA_VIEW_TABLE_USAGE, will fill in m_pwszViewRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_BestMatch()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_PROVIDER_TYPES))
		return FALSE;

	m_restrict |= FIRST;
 	m_iid = IID_IRowset;

	// if this is a schema I want then get information for schema
	if(GetSchemaInfo(SPECIFIC,0,DBSCHEMA_PROVIDER_TYPES))
		GetRowset();

	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find BestMatch for DBSCHEMA_PROVIDER_TYPES
//
// With DBSCHEMA_FOREIGN_KEYS, 		
// m_pwszPK_TableRestriction, m_pwszFK_TableRestriction;
//--------------------------------------------------------------------
BOOL CSchemaTest::Find_PK_and_FK()
{
	INIT

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	if(!IsSchemaSupported(DBSCHEMA_FOREIGN_KEYS))
		return FALSE;

	m_restrict |= FIRST;
	m_restrict |= SECOND;
	m_restrict |= FOURTH;
	m_restrict |= FIFTH;

 	m_iid = IID_IRowset;

	if(m_fCaptureRestrictions)
	{
		// PK TABLE_NAME
		if ((g_pKeyTable2) && (g_pKeyTable2->GetTableName()))
		{
			m_pwszPK_TableRestriction = (WCHAR *) PROVIDER_ALLOC
				((wcslen((WCHAR *) g_pKeyTable2->GetTableName())*sizeof(WCHAR)) + sizeof(WCHAR));
			
			if(m_pwszPK_TableRestriction)
				wcscpy(m_pwszPK_TableRestriction,(TYPE_WSTR) g_pKeyTable2->GetTableName());
		}

		if ((g_pKeyTable1) && (g_pKeyTable1->GetTableName()))
		{
			// FK TABLE_NAME
			m_pwszFK_TableRestriction = (WCHAR *) PROVIDER_ALLOC
				((wcslen((WCHAR *) g_pKeyTable1->GetTableName())*sizeof(WCHAR)) + sizeof(WCHAR));
			if(m_pwszFK_TableRestriction)
				wcscpy(m_pwszFK_TableRestriction,(TYPE_WSTR)g_pKeyTable1->GetTableName());
		}
		
	}


	FREE

	return TRUE;
}
//--------------------------------------------------------------------
// Find BestMatch for DBSCHEMA_PROVIDER_TYPES
//
// With DBSCHEMA_VIEW_TABLE_USAGE, will fill in m_pwszViewRestriction
//--------------------------------------------------------------------
BOOL CSchemaTest::Release_PK_and_FK()
{
	// Free the memory
	PROVIDER_FREE(m_pwszPK_TableRestriction);
	PROVIDER_FREE(m_pwszFK_TableRestriction);
	return TRUE;
}

//--------------------------------------------------------------------
// ASSERTIONS
// 1. Constraint Catalog
// 2. Constraint Schema
// 3. Constraint Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_ASSERTIONS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **) rgwszASSERTIONS;
	m_rgColumnTypes = (DBTYPE *) rgtypeASSERTIONS;
	
	// Set the count of columns and restrictions
	m_cColumns = cASSERTIONS;
	m_cRestrictions = cASSERTIONS_RESTRICTIONS;

	// Set Constraint Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszAssertion_ConstraintRestriction);
	
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowASSERTIONS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_ASSERTIONS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszAssertion_ConstraintRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cASSERTIONS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:ASSERTIONS:", iRow,m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// CONSTRAINT CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR) pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_ASSERTIONS:CONSTRAINT CATALOG restriction failed\n";
							m_fRes1 = FALSE;
							fResults = FALSE;
						}
					}
				}

				PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2://CONSTRAINT SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR) pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_ASSERTIONS:CONSTRAINT SCHEMA restriction failed\n";
							m_fRes1 = FALSE;
							fResults = FALSE;
						}
					}
				}

				PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3://CONSTRAINT NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR) pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_ASSERTIONS:CONSTRAINT NAME restriction failed\n";
							m_fRes3 = FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszAssertion_ConstraintRestriction = (TYPE_WSTR) PROVIDER_ALLOC
						((wcslen((TYPE_WSTR) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszAssertion_ConstraintRestriction)
						wcscpy(m_pwszAssertion_ConstraintRestriction,(TYPE_WSTR) pColumn->bValue);
				}

				PRVTRACE(L"= %s\n",(WCHAR *)pColumn->bValue);
				break;
			case 4: //IS_DEFERRABLE
			case 5: //INITIALLY_DEFERRED
				if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
					PRVTRACE(L"TRUE\n");
				else
					PRVTRACE(L"FALSE\n");
				break;
			case 6: // DESCRIPTION
					PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue );
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"ASSERTIONS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - ASSERTIONS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
				
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// CATALOGS
// 1. Catalog Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_CATALOGS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **) rgwszCATALOGS;
	m_rgColumnTypes = (DBTYPE *) rgtypeCATALOGS;
	
	// Set the count of columns and restrictions
	m_cColumns = cCATALOGS;
	m_cRestrictions = cCATALOGS_RESTRICTIONS;

	// Set Catalog Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCATALOGS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_CATALOGS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCATALOGS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"CATALOGS:Row[%lu],Col[%d,%s]:", 
//			iRow, 
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// CATALOG_NAME
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR) pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_CATALOGS:CATALOG_NAME restriction failed\n";
							m_fRes1 = FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2://DESCRIPTION
				PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue );
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"CATALOGS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - CATALOGS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
			
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
	}
	return fResults;
}

//--------------------------------------------------------------------
// CHARACTER_SETS
// 1. Character Set Catalog 
// 2. Character Set Schema
// 3. Character Set Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_CHARACTER_SETS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCHARACTER_SETS;
	m_rgColumnTypes = (DBTYPE *)rgtypeCHARACTER_SETS;

	// Set the count of columns and restrictions
	m_cColumns = cCHARACTER_SETS;
	m_cRestrictions = cCHARACTER_SETS_RESTRICTIONS;

	// Set Character Sets Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszCharacter_SetRestriction);

	PRVTRACE(L"GetSchemaInfo::CHARACTER_SETS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCHARACTER_SETS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_CHARACTER_SETS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_fCaptureRestrictions)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCHARACTER_SETS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:CHARACTER_SETS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// CHARACTER SET CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{	
							odtLog << L"VerifyRow_CHARACTER_SETS:CHARACTER SET CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// CHARACTER SET SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_CHARACTER_SETS:CHARACTER SET SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:// CHARACTER_SET_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_CHARACTER_SETS:CHARACTER SET NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszCharacter_SetRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszCharacter_SetRestriction)
						wcscpy(m_pwszCharacter_SetRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}
				
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// FORM_OF_USE
			case 6:// DEFAULT_COLLATE_CATALOG
			case 7:// DEFAULT_COLLATE_SCHEMA
			case 8:// DEFAULT_COLLATE_NAME
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// NUMBER OF CHARACTERS
				PRVTRACE(L"%d\n",*(TYPE_I8 *)pColumn->bValue);
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"CHARACTER SETS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - CHARACTER SETS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// CHECK_CONSTRAINTS
// 1. Constraint Catalog
// 2. Constraint Schema
// 3. Constraint Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_CHECK_CONSTRAINTS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCHECK_CONSTRAINTS;
	m_rgColumnTypes = (DBTYPE *)rgtypeCHECK_CONSTRAINTS;

	// Set the count of columns and restrictions
	m_cColumns = cCHECK_CONSTRAINTS;
	m_cRestrictions = cCHECK_CONSTRAINTS_RESTRICTIONS;
	
	// Set Constraint Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszCheck_ConstraintRestriction);

	PRVTRACE(L"GetSchemaInfo::CHECK_CONSTRAINTS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCHECK_CONSTRAINTS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_CHECK_CONSTRAINTS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszCheck_ConstraintRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCHECK_CONSTRAINTS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:CHECK_CONSTRAINTS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// CHECK_CONSTRAINTS CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{	
							odtLog << L"VerifyRow_CHECK_CONSTRAINTS:CHECK_CONSTRAINTS CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// CHECK_CONSTRAINTS SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog <<  L"VerifyRow_CHECK_CONSTRAINTS:CHECK_CONSTRAINTS SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:// CHECK_CONSTRAINTS NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<L"VerifyRow_CHECK_CONSTRAINTS:CHECK_CONSTRAINTS NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszCheck_ConstraintRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszCheck_ConstraintRestriction)
						wcscpy(m_pwszCheck_ConstraintRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// CHECK_CLAUSE
			case 5:// DESCRIPTION
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"CHECK CONSTRAINTS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - CHECK CONSTRAINTS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// COLLATIONS
// 1. Collation Catalog
// 2. Collation Schema
// 3. Collation Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_COLLATIONS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCOLLATIONS;
	m_rgColumnTypes = (DBTYPE *)rgtypeCOLLATIONS;

	// Set the count of columns and restrictions
	m_cColumns = cCOLLATIONS;
	m_cRestrictions = cCOLLATIONS_RESTRICTIONS;
	
	// Set Collation Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszCollationRestriction);

	PRVTRACE(L"GetSchemaInfo::COLLATIONS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCOLLATIONS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_COLLATIONS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;
    
	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszCollationRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCOLLATIONS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:COLLATIONS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// COLLATIONS CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{	
							odtLog <<  L"VerifyRow_COLLATIONS:COLLATIONS CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// COLLATIONS SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_COLLATIONS:COLLATIONS SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:// COLLATIONS_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_COLLATIONS:COLLATIONS NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszCollationRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszCollationRestriction)
						wcscpy(m_pwszCollationRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// CHARACTER SET CATALOG
			case 5:// CHARACTER SET SCHEMA
			case 6:// CHARACTER SET NAME
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 7:// PAD ATTRIBUTE
				if( (!COMPARE(0, wcscmp((TYPE_WSTR)pColumn->bValue,L"NO PAD")) && (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"PAD SPACE"))))
				{
					odtLog << L"VerifyRow_COLLATIONS:PAD ATTRIBUTE expected NO PAD or PAD SPACE but recieved " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"COLLATIONS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - COLLATIONS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// COLUMN_DOMAIN_USAGE
// 1. Domain Catalog
// 2. Domain Schema
// 3. Domain Name
// 4. Column Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_COLUMN_DOMAIN_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCOLUMN_DOMAIN_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeCOLUMN_DOMAIN_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cCOLUMN_DOMAIN_USAGE;
	m_cRestrictions = cCOLUMN_DOMAIN_USAGE_RESTRICTIONS;
	
	// Set Domain Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszDomainRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszColumnRestriction);

	PRVTRACE(L"GetSchemaInfo::COLUMN_DOMAIN_USAGE\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowDOMAIN_COLUMN_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_COLUMN_DOMAIN_USAGE(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszDomainRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCOLUMN_DOMAIN_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:COLUMN_DOMAIN_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// DOMAIN CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{	
							odtLog << L"VerifyRow_COLUMN_DOMAIN_USAGE:DOMAIN CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// DOMAIN SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog <<  L"VerifyRow_COLUMN_DOMAIN_USAGE:DOMAIN SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:// DOMAIN_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_COLUMN_DOMAIN_USAGE:DOMAIN NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszDomainRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszDomainRestriction)
						wcscpy(m_pwszDomainRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE CATALOG
			case 5:// TABLE SCHEMA
			case 6:// TABLE NAME
			case 7:// COLUMN NAME
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 8:// COLUMN GUID
				PRVTRACE(L"\n");
			case 9:// COLUMN PROPID
				PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"COLUMN DOMAIN USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - COLUMN DOMAIN USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
				// Have to flag error.
				odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
				fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// COLUMN_PRIVILEGES
// 1. Table Catalog
// 2. Table Schema 
// 3. Table Name
// 4. Column Name
// 5. Grantor
// 6. Grantee
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_COLUMN_PRIVILEGES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCOLUMN_PRIVILEGES;
	m_rgColumnTypes = (DBTYPE *)rgtypeCOLUMN_PRIVILEGES;

	// Set the count of columns and restrictions
	m_cColumns = cCOLUMN_PRIVILEGES;
	m_cRestrictions = cCOLUMN_PRIVILEGES_RESTRICTIONS;
	
	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszColumnRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszGrantorRestriction);
	SetRestriction(SIXTH, 6, &m_wszR6, m_pwszGranteeRestriction);

	// Set expected row count.  Since we create a table and we know 
	// how many columns there are we must have at least that number of 
	// rows, unless restricted to a given column.
	if (!m_wszR4)
		SetRowCount(MIN_REQUIRED, m_pTable->CountColumnsOnTable());

	PRVTRACE(L"GetSchemaInfo::COLUMN_PRIVILEGES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCOLUMN_PRIVILEGES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_COLUMN_PRIVILEGES(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;

	int test=0;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszGrantorRestriction && m_pwszGranteeRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCOLUMN_PRIVILEGES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"COLUMN_PRIVILEGES:Row[%lu],Col[%d,%s]:", 
//			iRow,
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// GRANTOR
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{	
							odtLog << L"VerifyRow_COLUMN_PRIVILEGES:GRANTOR 5 restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszGrantorRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszGrantorRestriction)
						wcscpy(m_pwszGrantorRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}

				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// GRANTEE
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog <<  L"VerifyRow_COLUMN_PRIVILEGES:GRANTEE 6 restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszGranteeRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszGranteeRestriction)
						wcscpy(m_pwszGranteeRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:// TABLE_CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog <<  L"VerifyRow_COLUMN_PRIVILEGES:TABLE_CATALOG 1 restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE_SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_COLUMN_PRIVILEGES:TABLE_SCHEMA 2 restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// TABLE_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_COLUMN_PRIVILEGES:TABLE_NAME 3 restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 6:// COLUMN_NAME
				if(m_restrict & FOURTH)
				{
					test=_wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue);
					
					if(!COMPARE(0, _wcsicmp((WCHAR *)m_wszR4,(WCHAR *)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog <<  L"VerifyRow_COLUMN_PRIVILEGES:COLUMN_NAME 4 restriction failed\n";
							m_fRes4 = FALSE;
							fResults = FALSE;
						}

					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 7:// COLUMN_GUID
				// TODO: Can I do anything interesting with a GUID?
				PRVTRACE(L"\n");
				break;
			case 8:// COLUMN_PROPID
				PRVTRACE(L"%ul\n",*(TYPE_UI4 *)pColumn->bValue);
				break;
			case 9:// PRIVILEGE_TYPE
				if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SELECT")) && 
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"DELETE")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"INSERT")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"UPDATE")) &&
 					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"REFERENCES")))
				{
					odtLog << L"VerifyRow_COLUMN_PRIVILEGES:PRIVILEGE_TYPE expected SELECT,DELETE,INSERT,UPDATE, or REFERENCES but received " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 10:// IS_GRANTABLE
				if((*(TYPE_BOOL *)pColumn->bValue)==VARIANT_TRUE)
					PRVTRACE(L"YES\n");
				else
					PRVTRACE(L"NO\n");
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"COLUMN PRIVILEGES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - COLUMN PRIVILEGES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}	
		else
		{
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}


	return fResults;
}

//--------------------------------------------------------------------
// COLUMNS
// 1. Table Catalog
// 2. Table Schema 
// 3. Table Name
// 4. Column Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_COLUMNS()
{	
	// Set the Schema column Names and Types
	m_cColumns = cCOLUMNS;
	m_rgColumnNames = (WCHAR **)rgwszCOLUMNS;
	m_rgColumnTypes = (DBTYPE *)rgtypeCOLUMNS;

	// Set the count of columns and restrictions
	m_cRestrictions = cCOLUMNS_RESTRICTIONS;

	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszColumnRestriction);

	// Set an Invalid Table Restriction
	if(m_restrict & FIFTH) {
		SetRestriction(FIFTH,5, &m_wszR5, m_pwszTableRestriction);
		m_cRestrictions = 5;
	}

	// Set expected row count.  Since we create a table and we know 
	// how many columns there are we must have at least that number of 
	// rows
	SetRowCount(MIN_REQUIRED, m_pTable->CountColumnsOnTable());

	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCOLUMNS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_COLUMNS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind=0;		// Binding Count
	ULONG	iIndex = 0;		// Index into rgColInfo array
	CCol	col;
	BOOL	fResults=TRUE;
	ULONG	cColumns = 0;
	ULONG	iOrdinalBinding = 6+!m_rgDBCOLUMNINFO[0].iOrdinal;

	// Per spec, the columns rowset must list columns a user has access to, therefore the user
	// must have access to the associated table.  Retrieve columns info for the table if this is a new
	// table.
	if (S_OK != GetColumnInfo(pData, m_rgDBBINDING))
		return FALSE;

	// If we don't have an m_prgColInfo array, then GetColumnInfo returns S_FALSE above.

	// Make our best guess of the index
	iIndex = m_ulTableOrdinal-1+!m_prgColInfo[0].iOrdinal;

	// If the provider returned the data out of order we have to find the ordinal
	ASSERT(m_rgDBBINDING[iOrdinalBinding].iOrdinal == 7);
	if (STATUS_BINDING(m_rgDBBINDING[iOrdinalBinding], pData) == DBSTATUS_S_OK)
	{
		DBORDINAL iOrdinal = (ULONG_PTR)VALUE_BINDING(m_rgDBBINDING[iOrdinalBinding], pData);

		// If this isn't the right index ordinals won't match
		if (m_prgColInfo[iIndex].iOrdinal != iOrdinal)
		{
			// Find the ordinal value in m_prgColInfo array as this row may not be in order
			for (ULONG iColInfo = 0; iColInfo < m_cColumns; iColInfo++)
			{
				if (m_prgColInfo[iColInfo].iOrdinal == iOrdinal)
				{
					iIndex = iColInfo;
					break;
				}
			}
		}
	}

	// Make sure we don't crash, but this will cause lots of errors
	if (!COMPARE(iIndex <= m_cColInfo - !m_prgColInfo[0].iOrdinal, TRUE))
		iIndex = 0;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCOLUMNS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		DBBINDING * pBind = &m_rgDBBINDING[iBind];
		DBORDINAL iOrdinal = m_rgDBCOLUMNINFO[iBind].iOrdinal;
		DBSTATUS ulStatus = STATUS_BINDING(*pBind, pData);
	    DATA *	pColumn=(DATA *) (pData + pBind->obStatus);
		BYTE * pValue = pColumn->bValue;

		if (m_fRes4 && m_fCaptureRestrictions)
		{
			m_fCaptureRestrictions = FALSE;
			return FALSE;
		}


		switch(iOrdinal)
		{
			// TABLE_CATALOG
			case 1:
				fResults &= TestReturnData(iRow,pColumn,FIRST,&m_fRes1,m_wszR1);
				break;

			// TABLE_SCHEMA
			case 2:
				fResults &= TestReturnData(iRow,pColumn,SECOND,&m_fRes2,m_wszR2);
				break;

			// TABLE_NAME
			case 3:
				fResults &= TestReturnData(iRow,pColumn,THIRD,&m_fRes3,m_wszR3);
				break;

			// COLUMN_NAME
			case 4:
				fResults &= TestReturnData(iRow,pColumn,FOURTH,&m_fRes4,m_wszR4);
				break;

			case 5:	// COLUMN_GUID
				break;
			case 13:// TYPE_GUID
				break;
			case 6:	//COLUMN_PROPID
				break;
			case 7:	// ORDINAL_POSITION
				if (ulStatus == DBSTATUS_S_OK)
				{
					// Status is OK, warning if ordinals don't match
					// Providers are not required to return results in ordinal order, since
					// sort is only on catalog, schema, and name, so only a warning.
					// If restriction is only on column then the ordinal may not match anyway.
					if (!m_fRes4)
					{						
						COMPAREW(m_ulTableOrdinal, (ULONG)(ULONG_PTR)VALUE_BINDING(*pBind, pData));
						if (m_ulTableOrdinal != (ULONG)(ULONG_PTR)VALUE_BINDING(*pBind, pData))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Ordinal didn't match expected value.\n";
					}
					else
					{
						// We will assume if the index for the matching ordinal contains
						// the proper column name that it was the right ordinal.
						if (RelCompareString(m_wszR4, m_prgColInfo[iIndex].pwszName))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Ordinal didn't match expected value.\n";
					}
				}
				else if (ulStatus == DBSTATUS_S_ISNULL)
				{
					// Status is NULL.  While this is allowed, it's not normally the case, so warn
					COMPAREW(ulStatus, DBSTATUS_S_OK);
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"DBSTATUS_S_ISNULL returned.\n";
				}
				else
				{
					// Otherwise this is a failure. 
					if (!COMPARE(ulStatus, DBSTATUS_S_OK))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Invalid value returned.\n";
				}
				break;
			case 10:// COLUMN FLAGS
				// This column cannot contain NULL
				if (COMPARE(ulStatus, DBSTATUS_S_OK))
					CCOMPARE(m_EC, m_prgColInfo[iIndex].dwFlags == *(DBCOLUMNFLAGS *)pValue, 
						EC_BAD_COLUMN_FLAGS,
						L"COLUMN_FLAGS value did not match GetColumnInfo",
						FALSE);
				break;
			case 14:// CHARACTER_MAXIMUM_LENGTH
				switch(m_prgColInfo[iIndex].wType)
				{
//					case DBTYPE_BOOL:  // We assume BOOL is not a "bit" column per spec.
					case DBTYPE_BYTES:
					case DBTYPE_STR:
					case DBTYPE_WSTR:
						if (COMPARE(ulStatus, DBSTATUS_S_OK))
						{
							if (m_prgColInfo[iIndex].ulColumnSize == ~0 && 
								!COMPARE(*(ULONG *)pValue, 0))
								odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
									L"Must be 0 for column without defined max length.\n";
							else 
								CCOMPARE(m_EC, *(ULONG *)pValue == m_prgColInfo[iIndex].ulColumnSize, 
									EC_BAD_CHARACTER_MAXIMUM_LENGTH,
									L"CHARACTER_MAXIMUM_LENGTH value did not match GetColumnInfo",
									FALSE);
						}
						else
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Cannot be NULL for variable length column.\n";
						break;
					// NULL for all other data types
					default:
						if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
				}
				break;
			case 15:// CHARACTER_OCTET_LENGTH
				switch(m_prgColInfo[iIndex].wType)
				{
					case DBTYPE_BYTES:
					case DBTYPE_STR:
					case DBTYPE_WSTR:
						if (COMPARE(ulStatus, DBSTATUS_S_OK))
						{
							if (m_prgColInfo[iIndex].ulColumnSize == ~0 && 
								!COMPARE(*(ULONG *)pValue, 0))
								odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
									L"Must be 0 for column without defined max length.\n";
							else 
							{
								DBLENGTH ulOctetLen = m_prgColInfo[iIndex].ulColumnSize;
								if (m_prgColInfo[iIndex].wType == DBTYPE_WSTR)
									ulOctetLen *= sizeof(WCHAR);

								CCOMPARE(m_EC, ulOctetLen == *(ULONG *)pValue, 
									EC_BAD_CHARACTER_OCTET_LENGTH,
									L"CHARACTER_OCTET_LENGTH value did not match GetColumnInfo",
									FALSE);
							}
						}
						break;
					// NULL for all other data types
					default:
						if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
				}
				break;
			case 18:// DATETIME_PRECISION
				switch(m_prgColInfo[iIndex].wType)
				{
					case DBTYPE_DBTIMESTAMP:
						if (COMPARE(ulStatus, DBSTATUS_S_OK))
							CCOMPARE(m_EC, *(ULONG *)pValue == (ULONG)m_prgColInfo[iIndex].bScale, 
								EC_BAD_DATETIME_PRECISION,
								L"DATETIME_PRECISION value did not match GetColumnInfo",
								FALSE);
						break;
					// NULL for all other data types
					default:
						if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
				}
				break;
			case 8:	// COLUMN_HASDEFAULT
				// We can't actually try the default value except for our test table, so we don't know
				// if it actually would work or not for other tables.
				// Note that COLUMN_DEFAULT may be NULL even if HASDEFAULT is TRUE but the converse
				// is not true.
				
				if (COMPARE(ulStatus, DBSTATUS_S_OK))
				{

					// Make sure we know the proper binding for DBCOLUMN_DEFAULT
					ASSERT (m_rgDBBINDING[8+!m_rgDBCOLUMNINFO[0].iOrdinal].iOrdinal == 9);
					DBSTATUS ulDefaultStatus = STATUS_BINDING(m_rgDBBINDING[8+!m_rgDBCOLUMNINFO[0].iOrdinal], pData);

					// If the COLUMN_DEFAULT is non-NULL, then HASDEFAULT should be TRUE, otherwise
					// we're not sure
					if (S_OK == ulDefaultStatus && !COMPARE(*(VARIANT_BOOL *)pValue, VARIANT_TRUE))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Invalid value returned.\n";

					// TODO: If the current table is the automaketable for this test try to use the default
					// value and make sure it matches expected.
				}
				else
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid status returned.\n";
				break;
			case 9: // COLUMN DEFAULT
				if (ulStatus == DBSTATUS_S_OK)
				{
					// For now, just make sure the length is right so we read the default value
					if (!COMPARE(wcslen((WCHAR *)pValue)*sizeof(WCHAR), LENGTH_BINDING(m_rgDBBINDING[iBind], pData)))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Default value has the wrong length.\n";

					// TODO: If the current table is the automaketable for this test try to use the default
					// value and make sure it matches expected.  
				}
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid status returned.\n";
				break;
			case 11:// IS_NULLABLE
			{
				VARIANT_BOOL fNullable = m_prgColInfo[iIndex].dwFlags & DBCOLUMNFLAGS_ISNULLABLE ? VARIANT_TRUE : VARIANT_FALSE;
				if (!COMPARE(fNullable, *(VARIANT_BOOL *)pValue))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid value returned.\n";
					break;
			}
			case 12:// DATA_TYPE
				// This column cannot contain NULL
				if (COMPARE(ulStatus, DBSTATUS_S_OK) &&
					!COMPARE(m_prgColInfo[iIndex].wType,	*(DBTYPE *)pValue))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid value returned.\n";
				break;
			case 16:// NUMERIC_PRECISION
				if (IsNumericType(m_prgColInfo[iIndex].wType))
				{
					if (COMPARE(ulStatus, DBSTATUS_S_OK) && 
						!COMPARE(*(USHORT *)pValue, (USHORT)m_prgColInfo[iIndex].bPrecision))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Invalid value returned.\n";
				}
				// NULL for all other data types
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Status was not DBSTATUS_S_ISNULL for non-numeric column\n";
				break;
			case 17:// NUMERIC_SCALE
				if (m_prgColInfo[iIndex].wType == DBTYPE_DECIMAL ||
					m_prgColInfo[iIndex].wType == DBTYPE_NUMERIC ||
					m_prgColInfo[iIndex].wType == DBTYPE_VARNUMERIC)
				{
					if (COMPARE(ulStatus, DBSTATUS_S_OK))
					{
						SHORT sScale = (SHORT)m_prgColInfo[iIndex].bScale;

						// Account for negative scale values
						if (m_prgColInfo[iIndex].dwFlags & DBCOLUMNFLAGS_SCALEISNEGATIVE)
							sScale = -sScale;

						if (!COMPARE(*(SHORT *)pValue, sScale))
							odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
								L"Invalid value returned.\n";
					}
				}
				// NULL for all other data types
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Status was not DBSTATUS_S_ISNULL for non-DECIMAL, NUMERIC column\n";
				break;
			case 19:// CHARACTER_SET_CATALOG
			case 20:// CHARACTER_SET_SCHEMA
			case 21:// CHARACTER_SET_NAME
			case 22:// COLLATION_CATALOG
			case 23:// COLLATION_SCHEMA
			case 24:// COLLATION_NAME
			case 25:// DOMAIN_CATALOG
			case 26:// DOMAIN_SCHEMA
			case 27:// DOMAIN_NAME
				if (ulStatus == DBSTATUS_S_OK)
				{
					// For now, just make sure the length is right so we read the value
					if (!COMPARE(wcslen((WCHAR *)pValue)*sizeof(WCHAR), LENGTH_BINDING(m_rgDBBINDING[iBind], pData)))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Length is incorrect.\n";

					// We really don't expect empty strings for any of these
					if (!COMPARE(wcslen((WCHAR *)pValue) > 0, TRUE))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Empty string returned.\n";
					
				}
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid status returned.\n";
				break;
			case 28:// DESCRIPTION
				// If Description is valid it may still be an empty string.  All we test
				// here is if it's null terminated properly.
				if (ulStatus == DBSTATUS_S_OK)
				{
					if (!COMPARE(wcslen((WCHAR *)pValue)*sizeof(WCHAR), LENGTH_BINDING(m_rgDBBINDING[iBind], pData)))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Length is incorrect.\n";
				}
				// Description is allowed to be NULL
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Unexpected status value found.\n";
				break;
			// PROVIDER SPECIFIC
			default:
				if (iRow == 1)
				{
					if(!m_rgDBCOLUMNINFO[iBind].iOrdinal) 
					{
						if(!COMPARE(GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset), TRUE))
							odtLog << L"COLUMNS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - COLUMNS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}

				// We expect to retrieve the column successfully
				if (ulStatus != DBSTATUS_S_OK &&
					ulStatus != DBSTATUS_S_ISNULL &&
					!COMPARE(ulStatus, DBSTATUS_S_OK))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Invalid status returned.\n";

				break;
		}
	}

	
	return fResults;
}

//--------------------------------------------------------------------
// CONSTRAINT_COLUMN_USAGE
// 1. Table Catalog 
// 2. Table Schema 
// 3. Table Name
// 4. Column Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_CONSTRAINT_COLUMN_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCONSTRAINT_COLUMN_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeCONSTRAINT_COLUMN_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cCONSTRAINT_COLUMN_USAGE;
	m_cRestrictions = cCONSTRAINT_COLUMN_USAGE_RESTRICTIONS;

	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszColumnRestriction);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCONSTRAINT_COLUMN_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_CONSTRAINT_COLUMN_USAGE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCONSTRAINT_COLUMN_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:CONSTRAINT_COLUMN_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);					  

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
				switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
				{
				case 1:// TABLE_CATALOG
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog <<  L"VerifyRow_CONSTRAINT_COLUMN_USAGE:TABLE_CATALOG restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 2:// TABLE_SCHEMA
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog <<  L"VerifyRow_CONSTRAINT_COLUMN_USAGE:TABLE_SCHEMA restriction failed\n";
								m_fRes2=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
					break;
				case 3:	// TABLE_NAME
					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog <<  L"VerifyRow_CONSTRAINT_COLUMN_USAGE:TABLE_NAME restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 4:// COLUMN_NAME
					if(m_restrict & FOURTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes4)
							{
								odtLog << L"VerifyRow_CONSTRAINT_COLUMN_USAGE:COLUMN_NAME restriction failed\n";
								m_fRes4=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 5:	// COLUMN_GUID
						PRVTRACE(L"\n");
						break;
				case 6:	//COLUMN_PROPID
						PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
						break;
				case 7: // CONSTRAINT CATALOG
				case 8: // CONSTRAINT SCHEMA
				case 9: // CONSTRAINT NAME
						PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
						break;
				default:
					// We found a column not spec'd for this schema rowset, print a warning.
					if (iRow == 1)
					{
						if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
						{
							if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
								odtLog << L"CONSTRAINT COLUMN USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
						}
						else
							odtLog << L"Warning - CONSTRAINT COLUMN USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
					}
					break;
				}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// CONSTRAINT_TABLE_USAGE
// 1. Table Catalog 
// 2. Table Schema 
// 3. Table Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_CONSTRAINT_TABLE_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszCONSTRAINT_TABLE_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeCONSTRAINT_TABLE_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cCONSTRAINT_TABLE_USAGE;
	m_cRestrictions = cCONSTRAINT_TABLE_USAGE_RESTRICTIONS;
	
	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);

	PRVTRACE(L"GetSchemaInfo::CONSTRAINT_TABLE_USAGE\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowCONSTRAINT_TABLE_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_CONSTRAINT_TABLE_USAGE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cCONSTRAINT_TABLE_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:CONSTRAINT_TABLE_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);				  

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// TABLE_CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_CONSTRAINT_TABLE_USAGE:TABLE_CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// TABLE_SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_CONSTRAINT_TABLE_USAGE:TABLE_SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// TABLE_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_CONSTRAINT_TABLE_USAGE:TABLE_NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4: // CONSTRAINT CATALOG
			case 5:	// CONSTRAINT SCHEMA
			case 6:	//CONSTRAINT NAME
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"CONSTRAINT TABLE USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - CONSTRAINT TABLE USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// FOREIGN_KEYS
// 1. PK_TABLE_CATALOG
// 2. PK TABLE SCHEMA
// 3. PK TABLE NAME
// 4. FK TABLE CATALOG
// 5. FK TABLE SCHEMA
// 6. FK TABLE ANEM
//
// Caveat: These restrictions will be tested as part of ad-hoc for the
// following reasons, The following code is a place holder in case
// the problems listed below are ever fixed:
// 1. Can't change private library's create table because not all
// drivers will support PK and FK systax, create table would fail
// 2. Can't use alter table statement because I can only add column
// that allow nulls and pk and fk are not nullable.
//
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_FOREIGN_KEYS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszFOREIGN_KEYS;
	m_rgColumnTypes = (DBTYPE *)rgtypeFOREIGN_KEYS;

	// Set the count of columns and restrictions
	m_cColumns = cFOREIGN_KEYS;
	m_cRestrictions = cFOREIGN_KEYS_RESTRICTIONS;

	// Set PK Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszPK_TableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszCatalogRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszSchemaRestriction);
	SetRestriction(SIXTH, 6, &m_wszR6, m_pwszFK_TableRestriction);

	// Set expected row count
	if (m_fForeignKey)
		// We created a foreign key, so there must be one row
		SetRowCount(MIN_REQUIRED, 1);
	
	PRVTRACE(L"GetSchemaInfo::FOREIGN_KEYS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowFOREIGN_KEYS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_FOREIGN_KEYS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cFOREIGN_KEYS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:FOREIGN_KEYS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
				case 1:// PK TABLE_CATALOG
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog << L"VerifyRow_FOREIGN_KEYS:PK TABLE_CATALOG restriction failed\n";
								m_fRes1=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 2:// PK TABLE_SCHEMA
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog << L"VerifyRow_FOREIGN_KEYS:PK TABLE_SCHEMA restriction failed\n";
								m_fRes2=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 3:	// PK TABLE_NAME
					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog << L"VerifyRow_FOREIGN_KEYS:PK TABLE_NAME restriction failed\n";
								m_fRes3=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 4: // PK COLUMN NAME
				case 10:// FK_COLUMN NAME
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 14:// UPDATE RULE
				case 15:// DELETE RULE
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"CASCADE")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SET NULL"))	&&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SET DEFAULT")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"NO ACTION")))
					{
						odtLog << L"VerifyRow_FOREIGN_KEYS:UPDATE or DELETE RULE expected CASCADE,SET NULL,SET DEFAULT, or NO ACTION but received " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults=FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 5: // PK COLUMN GUID
				case 11:// FK COLUMN GUID
				case 16:// PK_NAME
				case 17:// FK_NAME
				case 18:// DEFERRABILITY
					PRVTRACE(L"\n");
				case 6: // PK COLUMN PROPID
				case 12:// FK COLUMN PROPID
				case 13:// ORDINAL
						PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
						break;
				case 7:// FK TABLE_CATALOG
					if(m_restrict & FOURTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes4)
							{
								odtLog <<  L"VerifyRow_FOREIGN_KEYS:FK TABLE_CATALOG restriction failed\n";
								m_fRes4=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 8:// FK TABLE_SCHEMA
					if(m_restrict & FIFTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes5)
							{
								odtLog << L"VerifyRow_FOREIGN_KEYS:FK TABLE_SCHEMA restriction failed\n";
								m_fRes5=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 9:	// FK TABLE_NAME
					if(m_restrict & SIXTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes6)
							{
								odtLog << L"VerifyRow_FOREIGN_KEYS:FK TABLE_NAME restriction failed\n";
								m_fRes6=FALSE;
								fResults=FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				default:
					// We found a column not spec'd for this schema rowset, print a warning.
					if (iRow == 1)
					{
						if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
						{
							if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
								odtLog << L"FOREIGN KEYS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
						}
						else
							odtLog << L"Warning - FOREIGN KEYS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
					}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults=FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// INDEXES
// 1. Table Catalog 
// 2. Table Schema
// 3. Index Name
// 4. Type
// 5. Table Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_INDEXES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszINDEXES;
	m_rgColumnTypes = (DBTYPE *)rgtypeINDEXES;

	// Set the count of columns and restrictions
	m_cColumns = cINDEXES;
	m_cRestrictions = cINDEXES_RESTRICTIONS;
	
	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszIndexRestriction);

	if((m_currentBitMask & FOURTH) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & FOURTH)|| (m_restrict & ALLRES))
		{
			m_ulR = DBPROPVAL_IT_BTREE;
			m_rgvarRestrict[3].vt = VT_UI2;
			m_rgvarRestrict[3].lVal = DBPROPVAL_IT_BTREE;
			m_cRestrictionsCurrent ++;
			SetRestriction(FOURTH);

			RESTRICTNOTSUPPORTED(FOURTH)
		}
	}

	// TABLE NAME
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszTableRestriction);

	PRVTRACE(L"GetSchemaInfo::INDEXES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowINDEXES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_INDEXES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cINDEXES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		DBBINDING * pBind = &m_rgDBBINDING[iBind];
		DBORDINAL iOrdinal = m_rgDBCOLUMNINFO[iBind].iOrdinal;
		DBSTATUS ulStatus = STATUS_BINDING(*pBind, pData);
	    DATA *	pColumn=(DATA *) (pData + pBind->obStatus);
		BYTE * pValue = pColumn->bValue;

//		PRVTRACE(L"INDEXES:Row[%lu],Col[%d,%s]:", 
//			iRow,
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// TABLE_CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog  << L"VerifyRow_INDEXES:TABLE_CATALOG 1 restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// TABLE_SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog  << L"VerifyRow_INDEXES:TABLE_SCHEMA 2 restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// TABLE_NAME
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_INDEXES:TABLE_NAME 5 restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// INDEX_CATALOG
			case 5:	// INDEX_SCHEMA
			case 18:// COLUMN_NAME
			case 24:// FILTER_CONDIITON
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 6:// INDEX_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_INDEXES:INDEX_NAME 3 restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 7: // PRIMARY KEY
			case 8: // UNIQUE
			case 9: // CLUSTERED
			case 14:// SORTBOOKMARKS
			case 15:// AUTOUPDATE
			case 25:// INTEGRATED
					if((*(TYPE_BOOL *)pColumn->bValue)==VARIANT_TRUE)
						PRVTRACE(L"TRUE\n");
					else
						PRVTRACE(L"FALSE\n");
					break;
			case 10:// TYPE
					if(m_restrict & FOURTH)
					{
						if(m_ulR!=(*(TYPE_UI2*)pColumn->bValue))
						{
							odtLog <<  L"VerifyRow_INDEXES:INDEX_TYPE 4 restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
					PRVTRACE(L"%d\n",*(TYPE_UI2 *)pColumn->bValue);
					break;
			case 11:// FILLFACTOR
				// This column cannot be NULL
				if (COMPARE(ulStatus, DBSTATUS_S_OK))
				{
					ULONG ulFillFactor = *(ULONG *)pValue;

					// Spec change: Fill factor may be 0
					if (!COMPARE(ulFillFactor <= 100, TRUE))
						odtLog << L"FillFactor: Invalid value returned.\n";
				}
				break;
			case 12:// INITIALSIZE
			case 13:// NULLS
			case 16:// NULCOLLATION
			case 22:// CARDINALITY
			case 23:// PAGES
					PRVTRACE(L"%d\n",*(TYPE_I4 *)pColumn->bValue);
					break;
			case 19: // COLUMN_GUID
					PRVTRACE(L"\n");
					break;
			case 21:// COLLATION
					PRVTRACE(L"%d\n",*(TYPE_I2 *)pColumn->bValue);
					break;
			case 20:// COLUMN_PROPID
			case 17:// ORDINAL_POSITION
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"INDEXES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - INDEXES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
		}
		else
		{
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// KEY_COLUMN_USAGE
// 1. Constraint Catalog Name
// 2. Constraint Schema Name
// 3. Constraint Name
// 4. Table Catalog 
// 5. Table Schema 
// 6. Table Name
// 7. Column Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_KEY_COLUMN_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszKEY_COLUMN_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeKEY_COLUMN_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cKEY_COLUMN_USAGE;
	m_cRestrictions = cKEY_COLUMN_USAGE_RESTRICTIONS;
	
	// Set Constraint Restrictions
	SetRestriction(FIRST,  1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND, 2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD,  3, &m_wszR3, m_pwszKey_Column_Usage_ConstraintRestriction);
	SetRestriction(FOURTH, 4, &m_wszR4, m_pwszCatalogRestriction);
	SetRestriction(FIFTH,  5, &m_wszR5, m_pwszSchemaRestriction);
	SetRestriction(SIXTH,  6, &m_wszR6, m_pwszTableRestriction);
	SetRestriction(SEVENTH,7, &m_wszR7, m_pwszColumnRestriction);
		
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowKEY_COLUMN_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_KEY_COLUMN_USAGE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 	iBind;			// Binding Count
    DATA *	pColumn;		// Data Structure
	CCol	col;
	BOOL	fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszKey_Column_Usage_ConstraintRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cKEY_COLUMN_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
//		PRVTRACE(L"Row[%lu],Col[%s]:KEY_COLUMN_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// CONSTRAINT CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog <<  L"VerifyRow_KEY_COLUMN_USAGE:CONSTRAINT CATALOG 1 restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// CONSTRAINT SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_KEY_COLUMN_USAGE:CONSTRAINT SCHEMA 2 restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// CONSTRAINT NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszKey_Column_Usage_ConstraintRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszKey_Column_Usage_ConstraintRestriction)
						wcscpy(m_pwszKey_Column_Usage_ConstraintRestriction,(TYPE_WSTR) pColumn->bValue);
					
				}

				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_KEY_COLUMN_USAGE:CONSTRAINT NAME 3 restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE_CATALOG
				if(m_restrict & FOURTH)
				{
					if(!COMPARE(0,_wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog <<  L"VerifyRow_KEY_COLUMN_USAGE:TABLE_CATALOG 4 restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// TABLE_SCHEMA
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_KEY_COLUMN_USAGE:TABLE_SCHEMA 5 restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 6:	// TABLE_NAME
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog <<  L"VerifyRow_KEY_COLUMN_USAGE:TABLE_NAME 6 restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 7:// COLUMN NAME
				if(m_restrict & SEVENTH)
				{
					if(0,_wcsicmp((TYPE_WSTR)m_wszR7,(TYPE_WSTR)pColumn->bValue))
					{
						if(m_fRes7)
						{
							odtLog << L"VerifyRow_KEY_COLUMN_USAGE:COLUMN NAME 7 restriction failed\n";
							m_fRes7=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 8: // COLUMN_GUID
					PRVTRACE(L"\n");
					break;
			case 9: // COLUMN PROPID
			case 10:// ORDINAL POSITION
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (!m_rgDBCOLUMNINFO[iBind].iOrdinal)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"KEY COLUMN USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - KEY COLUMN USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// PRIMARY_KEYS
// 1. Table Catalog 
// 2. Table Name
// 3. Table Schem a
//
// Caveat: this will be handled in ad-hoc see PrepareParams_Foreign_keys()
// for explanation.
//
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_PRIMARY_KEYS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszPRIMARY_KEYS;
	m_rgColumnTypes = (DBTYPE *)rgtypePRIMARY_KEYS;

	// Set the count of columns and restrictions
	m_cColumns = cPRIMARY_KEYS;
	m_cRestrictions = cPRIMARY_KEYS_RESTRICTIONS;

	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszPK_TableRestriction);

	// Set expected count of rows
	if (m_fPrimaryKey)
		// We created a primary key, so there must be one row
		SetRowCount(MIN_REQUIRED, 1);
	
	PRVTRACE(L"GetSchemaInfo::PRIMARY_KEYS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowPRIMARY_KEYS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_PRIMARY_KEYS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cPRIMARY_KEYS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		DBBINDING * pBind = &m_rgDBBINDING[iBind];
		DBORDINAL iOrdinal = m_rgDBCOLUMNINFO[iBind].iOrdinal;
		DBSTATUS ulStatus = STATUS_BINDING(*pBind, pData);
	    DATA *	pColumn=(DATA *) (pData + pBind->obStatus);
		BYTE * pValue = pColumn->bValue;
		
//		PRVTRACE(L"Row[%lu],Col[%s]:PRIMARY_KEYS", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// TABLE CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog <<  L"VerifyRow_PRIMARY_KEYS:TABLE CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// TABLE SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_PRIMARY_KEYS:TABLE SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// TABLE NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_PRIMARY_KEYS:TABLE NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// COLUMN NAME
			case 8:// PK_NAME
				if (ulStatus == DBSTATUS_S_OK)
				{
					// For now, just make sure the length is right so we read the default value
					if (!COMPARE(wcslen((WCHAR *)pValue)*sizeof(WCHAR), LENGTH_BINDING(m_rgDBBINDING[iBind], pData)))
						odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Default value has the wrong length.\n";

					// TODO: If the current table is the primary table for this test make sure it matches expected.  
				}
				else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid status returned.\n";
				break;
			case 5: // COLUMN_GUID
			case 6: // COLUMN PROPID
			case 7: // ORDINAL POSITION
				if (ulStatus != DBSTATUS_S_OK &&
					!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
					odtLog << m_pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
						L"Invalid status returned.\n";
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"PRIMARY KEYS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - PRIMARY KEYS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// PROCEDURE_COLUMNS
// 1. Procedure Catalog 
// 2. Procedure Schema
// 3. Procedure Name
// 4. Column Name
//
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_PROCEDURE_COLUMNS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszPROCEDURE_COLUMNS;
	m_rgColumnTypes = (DBTYPE *)rgtypePROCEDURE_COLUMNS;

	// Set the count of columns and restrictions
	m_cColumns = cPROCEDURE_COLUMNS;
	m_cRestrictions = cPROCEDURE_COLUMNS_RESTRICTIONS;
	
	// Set Procedure Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszProcedureRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszProcedureColumnsRestriction);

	PRVTRACE(L"GetSchemaInfo::PROCEDURE_COLUMNS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowPROCEDURE_COLUMNS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_PROCEDURE_COLUMNS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cPROCEDURE_COLUMNS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:PROCEDURE_COLUMNS", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// PROCEDURE CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog <<  L"VerifyRow_PROCEDURE_COLUMNS:PROCEDURE CATALOG 1 restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// PROCEDURE SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_PROCEDURE_COLUMNS:PROCEDURE SCHEMA 2 restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// PROCEDURE NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_PROCEDURE_COLUMNS:PROCEDURE NAME 3 restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// COLUMN NAME
					if(m_fCaptureRestrictions)
					{
						m_pwszProcedureColumnsRestriction = (WCHAR *) PROVIDER_ALLOC
							((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszProcedureColumnsRestriction)
							wcscpy(m_pwszProcedureColumnsRestriction,(TYPE_WSTR) pColumn->bValue);
					}

					if(m_restrict & FOURTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes4)
							{
								odtLog << L"VerifyRow_PROCEDURE_COLUMNS:COLUMN NAME 4 restriction failed\n";
								m_fRes4=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 5: // COLUMN_GUID
			case 11:// TYPE_GUID
					PRVTRACE(L"\n");
					break;
			case 6: // COLUMN PROPID
			case 7: // ROWSET NUMBER
			case 8: // ORDINAL POSITION
			case 12:// CHARACTER_MAXIMUM_LENGTH
			case 13:// CHARACTER_OCTET_LENGTH
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			case 9:// IS_NULLABLE
					PRVTRACE(L"%d\n",*(TYPE_BOOL *)pColumn->bValue);
					break;
			case 10:// DATA_TYPE
			case 14:// NUMERIC_PRECISION
					PRVTRACE(L"%d\n",*(TYPE_UI2 *)pColumn->bValue);
					break;
			case 15:// NUMERIC_SCALE
					PRVTRACE(L"%d\n",*(TYPE_I2 *)pColumn->bValue);
					break;
			case 16:// DESCRIPTION
					PRVTRACE(L"%d\n",(TYPE_WSTR)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"PROCEDURE COLUMNS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - PROCEDURE COLUMNS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// PROCEDURE_PARAMETERS
// 1. Procedure Catalog
// 2. Procedure Schema
// 3. Procedure Name
// 4. Parameter Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_PROCEDURE_PARAMETERS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszPROCEDURE_PARAMETERS;
	m_rgColumnTypes = (DBTYPE *)rgtypePROCEDURE_PARAMETERS;

	// Set the count of columns and restrictions
	m_cColumns = cPROCEDURE_PARAMETERS;
	m_cRestrictions = cPROCEDURE_PARAMETERS_RESTRICTIONS;
	
	// Set Procedure Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszProcedureRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszParameterRestriction);

	PRVTRACE(L"GetSchemaInfo::PROCEDURES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowPROCEDURE_PARAMETERS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_PROCEDURE_PARAMETERS(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	
	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszParameterRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cPROCEDURE_PARAMETERS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:PROCEDURE_PARAMETERS", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// PROCEDURE CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// PROCEDURE SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// PROCEDURE_NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsnicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue,wcslen(m_wszR3))))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE_NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
					else
					{
						// Check the length of the proc name
						if( (wcslen(m_wszR3) != wcslen((TYPE_WSTR)pColumn->bValue)) &&
							(!COMPARE(_wcsicmp((TYPE_WSTR)&pColumn->bValue[wcslen(m_wszR3)*sizeof(WCHAR)], L";1")==0,TRUE)) )
						{
							odtLog <<  L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE_NAME length did not match\n";
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// PARAMETER NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszParameterRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszParameterRestriction)
						wcscpy(m_pwszParameterRestriction,(TYPE_WSTR) pColumn->bValue);
				}

				if(m_restrict & FOURTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog << L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE NAME restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5: // ORDINAL POSITION
			case 6: // PARAMETER TYPE
			case 10:// DATA TYPE
					PRVTRACE(L"%d\n",*(TYPE_UI2 *)pColumn->bValue);
					break;
			case 7: // PARAMETER_HASDEFAULT
			case 9: // IS NULLABLE
					if((*(TYPE_BOOL *)pColumn->bValue)==VARIANT_TRUE)
						PRVTRACE(L"TRUE\n");
					else
						PRVTRACE(L"FALSE\n");
					break;
			case 11:// CHARACTER_MAXIMUM_LENGTH
			case 12:// CHARACTER_OCTECT_LENGTH
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			case 13:// NUMERIC_PRECISION
					PRVTRACE(L"%d\n",*(TYPE_UI2 *)pColumn->bValue);
					break;
			case 14:// NUMERIC_SCALE
					PRVTRACE(L"%d\n",*(TYPE_I2 *)pColumn->bValue);
					break;
			case 8: // PARAMETER DEFAULT
			case 15:// DESCRIPTION
			case 16:// TYPE_NAME
			case 17:// LOCAL_TYPE_NAME
					PRVTRACE(L"'%s'\n",(TYPE_WSTR *)pColumn->bValue);
					break;

			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"PROCEDURE PARAMETERS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - PROCEDURE PARAMETERS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// PROCEDURES
// 1. Catalog Name
// 2. Schema Name
// 3. Procedure Name
// 4. Procedure Type
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_PROCEDURES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszPROCEDURES;
	m_rgColumnTypes = (DBTYPE *)rgtypePROCEDURES;

	// Set the count of columns and restrictions
	m_cColumns = cPROCEDURES;
	m_cRestrictions = cPROCEDURES_RESTRICTIONS;
	
	// Set Procedure Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszProcedureRestriction);

	if((m_currentBitMask & FOURTH) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & FOURTH)|| (m_restrict & ALLRES))
		{
			if(m_ProcedureTypeRestriction || m_fPassUnsupportedRestrictions)
			{
				m_ulR = (m_fPassUnsupportedRestrictions) ? DB_PT_PROCEDURE : m_ProcedureTypeRestriction;
				m_rgvarRestrict[3].vt = VT_I2;
				V_I2(&m_rgvarRestrict[3]) = (SHORT)m_ulR;
				m_cRestrictionsCurrent ++;
				SetRestriction(FOURTH);

				RESTRICTNOTSUPPORTED(FOURTH)

			}
		}
	}

	PRVTRACE(L"GetSchemaInfo::PROCEDURES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowPROCEDURES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_PROCEDURES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszProcedureRestriction && m_ProcedureTypeRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cPROCEDURES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"PROCEDURES:Row[%lu],Col[%d.%s]:", 
//			iRow, 
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// PROCEDURE_CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog <<  L"VerifyRow_PROCEDURES:PROCEDURE_CATALOG 1 restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// PROCEDURE_SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog <<  L"VerifyRow_PROCEDURES:PROCEDURE_SCHEMA 2 restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// PROCEDURE_NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszProcedureRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszProcedureRestriction)
						wcscpy(m_pwszProcedureRestriction,(TYPE_WSTR) pColumn->bValue);
				}

				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsnicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue,wcslen(m_wszR3))))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_PROCEDURES:PROCEDURE_NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
					else
					{
						// Check the length of the proc name
						if( (wcslen(m_wszR3) != wcslen((TYPE_WSTR)pColumn->bValue)) &&
							(!COMPARE(_wcsicmp((TYPE_WSTR)&pColumn->bValue[wcslen(m_wszR3)*sizeof(WCHAR)], L";1")==0,TRUE)) )
						{
							odtLog <<  L"VerifyRow_PROCEDURE_PARAMETERS:PROCEDURE_NAME length did not match\n";
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:	// PROCEDURE_TYPE
					if(m_fCaptureRestrictions)
						m_ProcedureTypeRestriction = *(TYPE_I2 *) pColumn->bValue;

					if(m_restrict & FOURTH)
					{
						if((TYPE_I2) m_ulR  != (*(TYPE_I2 *)pColumn->bValue))
						{
							if(m_fRes4)
							{
								odtLog << L"VerifyRow_PROCEDURES:DATA_TYPE restriction failed\n";
								m_fRes4=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"%d\n",*(TYPE_I2 *)pColumn->bValue);
					break;
			case 5:// PROCEDURE DEFINITION
			case 6:// DESCRIPTION
					PRVTRACE(L"%d\n",(TYPE_WSTR)pColumn->bValue);
					break;
			// DATE_CREATED
			// DATE_MODIFIED
			case 7:
			case 8:
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"PROCEDURES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - PROCEDURES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// PROVIDER_TYPES
// 1. Data Type
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_PROVIDER_TYPES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszPROVIDER_TYPES;
	m_rgColumnTypes = (DBTYPE *)rgtypePROVIDER_TYPES;

	// Set the count of columns and restrictions
	m_cColumns = cPROVIDER_TYPES;
	m_cRestrictions = cPROVIDER_TYPES_RESTRICTIONS;

	if((m_currentBitMask & FIRST) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & FIRST) || (m_restrict & ALLRES))
		{
			if(m_DataTypeRestriction)
			{
				m_ulR = m_DataTypeRestriction;
				m_rgvarRestrict[0].vt = VT_UI2;
				m_rgvarRestrict[0].iVal	= m_DataTypeRestriction;		
				m_cRestrictionsCurrent ++;
				SetRestriction(FIRST);

				RESTRICTNOTSUPPORTED(FIRST)

			}
		}
	}
	if((m_currentBitMask & SECOND) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & SECOND) || (m_restrict & ALLRES))
		{
			m_fR = (m_BestMatchRestriction==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;
			m_rgvarRestrict[1].vt = VT_BOOL;
			m_rgvarRestrict[1].boolVal	= (m_BestMatchRestriction==VARIANT_TRUE) ? VARIANT_TRUE : VARIANT_FALSE;
			m_cRestrictionsCurrent ++;
			SetRestriction(SECOND);

			RESTRICTNOTSUPPORTED(SECOND)
		}
	}

	PRVTRACE(L"GetSchemaInfo::PROVIDER_TYPES\n");
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRow_PROVIDER_TYPES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_PROVIDER_TYPES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cPROVIDER_TYPES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"PROVIDER_TYPES:Row[%lu],Col[%d,%s]:", 
//			iRow, 
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1: // TYPE_NAME
			case 4: // LITERAL_PREFIX
			case 5: // LITERAL_SUFFIX
			case 6: // CREATE PARAMS
			case 13:// LOCAL_TYPE_NAME
			case 17:// TYPELIB
			case 18:// VERSION
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 2:// DATA TYPE
					if(m_restrict & FIRST)
					{
						if(m_ulR!= (*(TYPE_UI2 *)pColumn->bValue))
						{
							if(m_fRes1)
							{
								odtLog << L"VerifyRow_PROVIDER_TYPES:TYPE NAME restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"%d\n",*(TYPE_UI2 *)pColumn->bValue);
					break;
			case 3:	// COLUMN SIZE
			case 9: // SEARCHABLE
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
						break;
			case 7: // IS_NULLABLE
			case 8: // CASE_SENSITIVE
			case 10:// UNSIGNED_ATTRIBUTE
			case 11:// FIXED PREC SCALE
			case 12:// AUTO_UNIQUE_VALUE
			case 19:// IS_LONG
			case 21:// IS_FIXEDLENGTH
					if((*(TYPE_BOOL *)pColumn->bValue)==VARIANT_TRUE)
						PRVTRACE(L"TRUE\n");
					else
						PRVTRACE(L"FALSE\n");
					break;
			case 14:// MINIMUM_SCALE
			case 15: // MAXIMUM_SCALE
					PRVTRACE(L"%d\n",*(TYPE_I2 *)pColumn->bValue);
						break;
			case 20: // BEST_MATCH
					if(m_fCaptureRestrictions)
						m_BestMatchRestriction = *(TYPE_BOOL *) pColumn->bValue;

					if(m_restrict & SECOND)
					{
						if(m_fR != (*(TYPE_BOOL *)pColumn->bValue))
						{
							odtLog << L"VerifyRow_PROVIDER_TYPES:BEST_MATCH restriction failed\n";
							fResults = FALSE;
						}
					}
					
					if((*(TYPE_BOOL *)pColumn->bValue)==VARIANT_TRUE)
						PRVTRACE(L"TRUE\n");
					else
						PRVTRACE(L"FALSE\n");
					break;

			case 16:// GUID
					PRVTRACE(L"\n");
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"PROVIDER TYPES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - PROVIDER TYPES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// REFERENTIAL_CONSTRAINTS
// 1. Constraint Catalog 
// 2. Constraint Schema 
// 3. Constraint Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_REFERENTIAL_CONSTRAINTS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszREFERENTIAL_CONSTRAINTS;
	m_rgColumnTypes = (DBTYPE *)rgtypeREFERENTIAL_CONSTRAINTS;

	// Set the count of columns and restrictions
	m_cColumns = cREFERENTIAL_CONSTRAINTS;
	m_cRestrictions = cREFERENTIAL_CONSTRAINTS_RESTRICTIONS;
	
	// Set Constraint Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszReferential_ConstraintRestriction);

	PRVTRACE(L"GetSchemaInfo::REFERENTIAL_CONSTRAINTS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowREFERENTIAL_CONSTRAINTS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_REFERENTIAL_CONSTRAINTS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszReferential_ConstraintRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cREFERENTIAL_CONSTRAINTS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"REFERENTIAL_CONSTRAINTS:Row[%lu],Col[%d,%s]:", 
//			iRow, 
//			m_rgDBCOLUMNINFO[iBind].iOrdinal,
//			m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
 			// TODO: not currently supported
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:	 // CONSTRAINT_CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:CONSTRAINT_CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:	 // CONSTRAINT_SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:CONSTRAINT_SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	 // CONSTRAINT_NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszReferential_ConstraintRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR *)) + sizeof(WCHAR));
					if(m_pwszReferential_ConstraintRestriction)
						wcscpy(m_pwszReferential_ConstraintRestriction,(TYPE_WSTR) pColumn->bValue);
				}

				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:CONSTRAINT_NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:	 // UNIQUE_CONSTRAINT_CATALOG
			case 5:	 // UNIQUE_CONSTRAINT_SCHEMA
			case 6:	 // UNIQUE_CONSTRAINT_NAME
			case 10: // DESCRIPTION
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 7:  // MATCH_OPTION
				if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"NONE")) && 
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"PARTIAL"))&&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"FULL")))
				{
					odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:MATCH OPTION expected NONE/PARTIAL/FULL but recieved " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;

			case 8:  // UPDATE_RULE
			case 9:  // DELETE_RULL
				if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"CASCASE")) && 
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SET NULL"))	&&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SET DEFAULT"))&&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"NO ACTION")))
				{
					odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:UPDATE/DELETE RULE expected SELECT,DELETE,INSERT,UPDATE, or REFERENCES but recieved " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"REFERENTIAL CONSTRAINTS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - REFERENTIAL CONSTRAINTS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}

	}
	return fResults;
}

//--------------------------------------------------------------------
// SCHEMATA
// 1. Catalog Name
// 2. Schema Name
// 3. Schema Owner
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_SCHEMATA()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszSCHEMATA;
	m_rgColumnTypes = (DBTYPE *)rgtypeSCHEMATA;

	// Set the count of columns and restrictions
	m_cColumns = cSCHEMATA;
	m_cRestrictions = cSCHEMATA_RESTRICTIONS;
	
	// Set Schema Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszSchemaRestriction);

	PRVTRACE(L"GetSchemaInfo::SCHEMATA\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowSCHEMATA
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_SCHEMATA
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cSCHEMATA <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn=(DATA *)(pData + m_rgDBBINDING[iBind].obStatus);

		if(pColumn->sStatus == DBSTATUS_S_OK)
		{
			switch(m_rgDBBINDING[iBind].iOrdinal)
			{
			case 1:// CATALOG_NAME
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog << L"VerifyRow_SCHEMATA:CATALOG_NAME restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;

			case 2:// SCHEMA_NAME
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog <<  L"VerifyRow_SCHEMATA:SCHEMA_NAME restriction failed\n";
								m_fRes2=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;

			case 3:	// SCHEMA_OWNER
					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog << L"VerifyRow_SCHEMATA:SCHEMA_OWNER restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;

			case 4: // DEFAULT_CHARACTER_SET_CATALOG
			case 5:	// DEFAULT_CHARACTER_SET_SCHEMA
			case 6: // DEFAULT_CHARACTER_SET_NAME
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"SCHEMATA: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - SCHEMATA provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// SQL_LANGUAGES
// No Constraints
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_SQL_LANGUAGES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszSQL_LANGUAGES;
	m_rgColumnTypes = (DBTYPE *)rgtypeSQL_LANGUAGES;
	
	// Set the count of columns and restrictions
	m_cColumns = cSQL_LANGUAGES;
	m_cRestrictions = cSQL_LANGUAGES_RESTRICTIONS;
	
	m_cRestrictionsCurrent = 0;
	
	PRVTRACE(L"GetSchemaInfo::SQL_LANGUAGES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowSQL_LANGUAGES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_SQL_LANGUAGES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cSQL_LANGUAGES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:VerifyRow_SQL_LANGUAGES:", iRow,m_rgDBCOLUMNINFO[iBind].pwszName);
		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(iBind)
			{
			case 1: // SQL LANGUAGE SOURCE
					if(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"ISO 9075"))
					{
						odtLog << L"VerifyRow_SQL_LANGUAGES:SQL_LANGUAGE_SOURCE expected ISO 9075 but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 2: // SQL LANGUAGE YEAR
					if(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"ISO 9075"))
					{
						odtLog << L"VerifyRow_SQL_LANGUAGES:SQL_LANGUAGE_YEAR expected 1992 but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 3: // SQL LANGUAGE CONFORMANCE
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"ENTRY")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"INTERMEDIATE")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"FULL")))
					{
						odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:SQL_LANGUAGE_CONFORMANCE expected ENTRY/INTERMEDIATE/FULL but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 4: // SQL LANGUAGE INTEGRITY
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"YES")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"NO")))
					{
						odtLog << L"VerifyRow_REFERENTIAL_CONSTRAINTS:SQL_LANGUAGE_INTEGRITY expected YES/NO but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 5: // SQL LANGUAGE IMPLEMENTATION, should be null if SQL_LANGUAGE_SOURCE is 'ISO_9075'
			case 6: // SQL LANGUAGE BINDING SYTLE
			case 7: // SQL LANGUAGE PROGRAMMING LANGUAGE
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			default:
					PRVTRACE(L"%s not expected\n",m_rgDBCOLUMNINFO[iBind].pwszName);
					break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// STATISTICS
// 1. Catalog Name
// 2. Schema Name
// 3. Table Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_STATISTICS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszSTATISTICS;
	m_rgColumnTypes = (DBTYPE *)rgtypeSTATISTICS;
	
	// Set the count of columns and restrictions
	m_cColumns = cSTATISTICS;
	m_cRestrictions = cSTATISTICS_RESTRICTIONS;
	
	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);

	PRVTRACE(L"GetSchemaInfo::STATISTICS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowSTATISTICS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_STATISTICS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cSTATISTICS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:STATISTICS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);
		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1:// TABLE CATALOG
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog << L"VerifyRow_STATISTICS:TABLE_CATALOG restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 2:// TABLE SCHEMA
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog << L"VerifyRow_STATISTICS:TABLE_SCHEMA restriction failed\n";
								m_fRes2=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 3:	//TABLE NAME
					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog << L"VerifyRow_STATISTICS:TABLE_NAME restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 4: // CARDINALITY
					PRVTRACE(L"%d\n",*(TYPE_I4 *)pColumn->bValue);
					break;
			default:
					PRVTRACE(L"%s not expected\n",m_rgDBCOLUMNINFO[iBind].pwszName);
					break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// TABLE_CONSTRAINTS
// 1. Constraint Catalog 
// 2. Constraint Schema 
// 3. Constraint Name
// 4. Table Catalog
// 5. Table Schema
// 6. Table Name
// 7. Constraint Type
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TABLE_CONSTRAINTS()
{	
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszTABLE_CONSTRAINTS;
	m_rgColumnTypes = (DBTYPE *)rgtypeTABLE_CONSTRAINTS;

	// Set the count of columns and restrictions
	m_cColumns = cTABLE_CONSTRAINTS;
	m_cRestrictions = cTABLE_CONSTRAINTS_RESTRICTIONS;

	// Set Constraint Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTable_ConstraintRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszCatalogRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszSchemaRestriction);
	SetRestriction(SIXTH, 6, &m_wszR6, m_pwszTableRestriction);
	SetRestriction(THIRD, 7, &m_wszR7, m_pwszConstraint_TypeRestriction);

	PRVTRACE(L"GetSchemaInfo::TABLE_CONSTRAINTS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTABLE_CONSTRAINTS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TABLE_CONSTRAINTS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszTable_ConstraintRestriction && m_pwszConstraint_TypeRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTABLE_CONSTRAINTS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:TABLE_CONSTRAINTS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);
		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://CONSTRAINT CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// CONSTRAINT SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// CONSTRAINT NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszTable_ConstraintRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszTable_ConstraintRestriction)
						wcscpy(m_pwszTable_ConstraintRestriction,(TYPE_WSTR) pColumn->bValue);
				}
				
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE CATALOG
				if(m_restrict & FOURTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog << L"VerifyRow_TABLE_CONSTRAINTS:TABLE CATALOG restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// TABLE SCHEMA
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_TABLE_CONSTRAINTS:TABLE SCHEMA restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 6:	// TABLE NAME
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog <<  L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT NAME restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case 7: // CONSTRAINT TYPE
					if(m_fCaptureRestrictions)
					{
						m_pwszConstraint_TypeRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszConstraint_TypeRestriction)
							wcscpy(m_pwszConstraint_TypeRestriction,(WCHAR *) pColumn->bValue);
					}

					if(m_restrict & SEVENTH)
					{
						if(!COMPARE(0, _wcsicmp((WCHAR* )m_wszR7,(const WCHAR *)pColumn->bValue)))
						{
							if(m_fRes7)
							{
								odtLog <<  L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT NAME restriction failed\n";
								m_fRes7=FALSE;
								fResults = FALSE;
							}
						}
					}
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"UNIQUE")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"PRIMARY KEY")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"FOREIGN KEY"))&&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"CHECK")))
					{
						odtLog << L"VerifyRow_TABLE_CONSTRAINTS:CONSTRAINT TYPE expected UNIQUE/PRIMARYKEY/FOREIGNKEY/CHECK but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 8: // IS_DEFERRABLE
			case 9:	// INITIALLY_DEFERRED
				if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
					PRVTRACE(L"TRUE\n");
				else
					PRVTRACE(L"FALSE\n");
				break;
			case 10: // DESCRIPTION
					PRVTRACE(L"%d\n",(TYPE_WSTR *)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"TABLE CONSTRAINTS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - TABLE CONSTRAINTS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}
	return fResults;
}

//--------------------------------------------------------------------
// TABLE_PRIVILEGES
// 1. Catalog Name
// 2. Schema Name
// 3. Table Name
// 4. Grantor
// 5. Grantee
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TABLE_PRIVILEGES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszTABLE_PRIVILEGES;
	m_rgColumnTypes = (DBTYPE *)rgtypeTABLE_PRIVILEGES;

	// Set the count of columns and restrictions
	m_cColumns = cTABLE_PRIVILEGES;
	m_cRestrictions = cTABLE_PRIVILEGES_RESTRICTIONS;

	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszGrantorRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszGranteeRestriction);

	PRVTRACE(L"GetSchemaInfo::TABLE_PRIVILEGES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTABLE_PRIVILEGES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TABLE_PRIVILEGES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTABLE_PRIVILEGES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:TABLE_PRIVILEGES:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);
		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://GRANTOR
				if(m_restrict & FOURTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog << L"VerifyRow_TABLE_PRIVILEGES:GRANTOR restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case 2:// GRANTEE
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_TABLE_PRIVILEGES:GRANTEE restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// TABLE CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_TABLE_PRIVILEGES:TABLE CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_TABLE_PRIVILEGES:TABLE SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// TABLE NAME
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_TABLE_PRIVILEGES:TABLE NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 6:	// PRIVILEGE TYPE
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SELECT")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"INSERT")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"DELETE")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"REFERENCES")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"UPDATE")))
					{
						odtLog << L"VerifyRow_TABLE_PRIVILEGES:PRIVILEGE TYPE expected SELECT/INSERT/DELETE/REFERENCES/UPDATE but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 7: // IS_GRANTABLE
				if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
					PRVTRACE(L"TRUE\n");
				else
					PRVTRACE(L"FALSE\n");
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"TABLE PRIVILEGES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - TABLE PRIVILEGES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}

//--------------------------------------------------------------------
// TABLE_STATISTICS
// 1. Catalog Name
// 2. Schema Name
// 3. Table Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TABLE_STATISTICS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszTABLESTATISTICS;
	m_rgColumnTypes = (DBTYPE *)rgtypeTABLESTATISTICS;

	// Set the count of columns and restrictions
	m_cColumns = cTABLE_STATISTICS;
	m_cRestrictions = cTABLE_STATISTICS_RESTRICTIONS;

	// Set Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszStatisticsCatalogRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszStatisticsSchemaRestriction);
	SetRestriction(SIXTH, 6, &m_wszR6, m_pwszStatisticsNameRestriction);

	// Seventh restriction
	if((m_currentBitMask & SEVENTH) || m_fPassUnsupportedRestrictions)
	{
		if((m_restrict & SEVENTH)|| (m_restrict & ALLRES))
		{
			m_ulR = m_uiStatisticsTypeRestriction;
			V_VT(&m_rgvarRestrict[6]) = VT_UI2;
			V_UI2(&m_rgvarRestrict[6]) = m_uiStatisticsTypeRestriction;
			m_cRestrictionsCurrent ++;
			SetRestriction(SEVENTH);

			RESTRICTNOTSUPPORTED(SEVENTH)
		}
	}

	PRVTRACE(L"GetSchemaInfo::TABLE_STATISTICS\n");
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTABLE_STATISTICS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TABLE_STATISTICS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;
	HRESULT			hrOpenTable = E_FAIL;
	LPWSTR			pwszTableName = (LPWSTR)GetValuePtr(TS_TABLE_NAME, pData);
	LPWSTR			pwszStatName = (LPWSTR)GetValuePtr(TS_STATISTICS_NAME, pData);
	LPWSTR			pwszColumnName = (LPWSTR)GetValuePtr(TS_COLUMN_NAME, pData);
	ULONG *			pulTableCardinality = (ULONG *)GetValuePtr(TS_TABLE_CARDINALITY, pData);
	ULONG *			pulOrdinal = (ULONG *)GetValuePtr(TS_ORDINAL_POSITION, pData);
	ULONG			ulStatType = 0;
	ULONG			ulIndex = 0;
	DBID			dbidTable = DB_NULLID;
	BOOL			fNewTable = (!m_pwszTableName || !pwszTableName || (pwszTableName && RelCompareString(pwszTableName, m_pwszTableName)));
	BOOL			fNewStat = fNewTable | (!m_pwszStatName || !pwszStatName || (pwszStatName && RelCompareString(pwszStatName, m_pwszStatName)));
	CARDINALITY		eCardinality = TUPLE_CARDINALITY;
	LPWSTR			pwszCat = (LPWSTR)GetValuePtr(TS_TABLE_CATALOG, pData);
	LPWSTR			pwszSch = (LPWSTR)GetValuePtr(TS_TABLE_SCHEMA, pData);
	LPWSTR			pwszQualifiedName = NULL;

	TESTC_(m_pTable->GetQualifiedName(pwszCat, pwszSch, 
							pwszTableName,&pwszQualifiedName, TRUE), S_OK);

	// *********************************************************************
	// For current time, set this on for debugging test code.
	// *********************************************************************
	m_fDetailCheck = TRUE;

	if (m_fDetailCheck)
	{
		ULONG * pulStatType = (ULONG *)GetValuePtr(TS_STATISTICS_TYPE, pData);

		// Attempt to open table and get columns info
		if (fNewTable)
		{
			hrOpenTable = GetColumnInfo(pData, m_rgDBBINDING);

			// Should be S_OK, or perhaps DB_E_NOTABLE
			if (DB_E_NOTABLE != hrOpenTable)
				CHECK(hrOpenTable, S_OK);
			else
				// We warn here because it's possible the table name is
				// bogus, or just deleted.
				CHECKW(hrOpenTable, S_OK);
		}
		else
			// If we obtained col info for the table we successfully opened it.
			hrOpenTable = (m_prgColInfo) ? S_OK : E_FAIL;

#ifdef BASE_TABLE_ORDINALS
		// Set index into columns info given ordinal
		if (m_prgColInfo && pulOrdinal)
			ulIndex = *pulOrdinal-1+!m_prgColInfo[0].iOrdinal;
#else
		// Set index into columns info given column name
		if (m_prgColInfo && pwszColumnName)
		{
			for (ulIndex = 0; ulIndex < m_cColInfo; ulIndex++)
			{
				if (!wcscmp(m_prgColInfo[ulIndex].pwszName, pwszColumnName))
					break;
			}
		}
#endif

		// Set up table dbid
		dbidTable.eKind = DBKIND_NAME;
		dbidTable.uName.pwszName = pwszTableName;

		// Get statistic type
		if (pulStatType)
			ulStatType = *pulStatType;
		
	}

	// Only capture restrictions once.  Note we assume catalog and schema
	// may not be supported but that statistics name will be
	if(m_fCaptureRestrictions && m_pwszStatisticsNameRestriction)
	{
		m_fCaptureRestrictions = FALSE;
		return FALSE;
	}

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTABLE_STATISTICS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:TABLE_STATISTICS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		// Set cardinality 
		eCardinality = TUPLE_CARDINALITY;

		if(pColumn->sStatus==DBSTATUS_S_OK ||
			pColumn->sStatus==DBSTATUS_S_ISNULL)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case TS_TABLE_CATALOG:
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_TABLE_STATISTICS:TABLE_CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				// Column must have non-empty string
				if (pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case TS_TABLE_SCHEMA:
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog << L"VerifyRow_TABLE_STATISTICS:TABLE_SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				// Column must have non-empty string
				if (pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case TS_TABLE_NAME:
				if(m_restrict & THIRD)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes3)
						{
							odtLog <<  L"VerifyRow_TABLE_STATISTICS:TABLE NAME restriction failed\n";
							m_fRes3=FALSE;
							fResults = FALSE;
						}
					}
				}
				// Column must have non-empty, non-null string
				if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case TS_STATISTICS_CATALOG:
				if(m_restrict & FOURTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes4)
						{
							odtLog << L"VerifyRow_TABLE_STATISTICS:STATISTICS_CATALOG restriction failed\n";
							m_fRes4=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszStatisticsCatalogRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszStatisticsCatalogRestriction)
						wcscpy(m_pwszStatisticsCatalogRestriction,(TYPE_WSTR) pColumn->bValue);
				}
				// Column must have non-empty string
				if (pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case TS_STATISTICS_SCHEMA:
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_TABLE_STATISTICS:STATISTICS_SCHEMA restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszStatisticsSchemaRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszStatisticsSchemaRestriction)
						wcscpy(m_pwszStatisticsSchemaRestriction,(TYPE_WSTR) pColumn->bValue);
				}
				// Column must have non-empty string
				if (pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case TS_STATISTICS_NAME:
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog << L"VerifyRow_TABLE_STATISTICS:STATISTICS_NAME restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				if(m_fCaptureRestrictions)
				{
					m_pwszStatisticsNameRestriction = (WCHAR *) PROVIDER_ALLOC((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
					if(m_pwszStatisticsNameRestriction)
						wcscpy(m_pwszStatisticsNameRestriction,(TYPE_WSTR) pColumn->bValue);
				}
				// Column must have non-empty string and cannot be NULL
				if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);
				PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
				break;
			case TS_STATISTICS_TYPE:
			{
				TYPE_UI2 ulStatType = 0;

				// Column cannot be NULL
				if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
				{
					ulStatType = *(TYPE_UI2 *)pColumn->bValue;

					if(m_restrict & SEVENTH)
					{
						if(!COMPARE(m_uiStatisticsTypeRestriction, ulStatType))
						{
							if(m_fRes7)
							{
								odtLog << L"VerifyRow_TABLE_STATISTICS:STATISTICS_TYPE restriction failed\n";
								m_fRes7=FALSE;
								fResults = FALSE;
							}
						}
					}

					if(m_fCaptureRestrictions)
					{
						m_uiStatisticsTypeRestriction = ulStatType;
						// Avoid posting failures when just capturing restrictions
						continue;
					}
				
					// Make sure no other bits are set than those spec'd
					COMPARE(0, ulStatType & (~(DBSTAT_HISTOGRAM | DBSTAT_COLUMN_CARDINALITY |
						DBSTAT_TUPLE_CARDINALITY)));

					// If we claim histogram support then NO_OF_RANGES cannot be null
					// but may be 0 if the table is empty.
					if (ulStatType & DBSTAT_HISTOGRAM)
						COMPARE(GetValuePtr(TS_NO_OF_RANGES, pData) != NULL, TRUE);
					else
					{
						CCOMPARE(m_EC, GetValuePtr(TS_NO_OF_RANGES, pData) == NULL, 
							EC_NULL_RANGE_COUNT,
							L"DBSTAT_HISTOGRAM not set but NO_OF_RANGES is non-NULL",
							FALSE);
					}

					// If we claim column cardinality support then it must not be null
					if (ulStatType & DBSTAT_COLUMN_CARDINALITY)
					{
						COMPARE(GetValuePtr(TS_COLUMN_CARDINALITY, pData) != NULL, TRUE);
					}
					else
						COMPARE(GetValuePtr(TS_COLUMN_CARDINALITY, pData) == NULL, TRUE);

					// If we claim tuple cardinality support then not be null
					if (ulStatType & DBSTAT_TUPLE_CARDINALITY)
					{
						COMPARE(GetValuePtr(TS_TUPLE_CARDINALITY, pData) != NULL, TRUE);
					}
					else
						COMPARE(GetValuePtr(TS_TUPLE_CARDINALITY, pData) == NULL, TRUE);

					// DetailCheckStatType(ulStatType, pData);

					// Detail check
					if (m_fDetailCheck)
					{
						IRowset * pIRowset = NULL;
						DBID dbidStat;
						LPWSTR pwszStatName = (LPWSTR)GetValuePtr(TS_STATISTICS_NAME, pData);
						HRESULT hrOpenHistogram = E_FAIL;

						dbidStat.eKind = DBKIND_GUID_NAME;
						dbidStat.uName.pwszName = pwszStatName;
						dbidStat.uGuid.guid = g_guidHistogramRowset;

						// Attempt to open histogram rowset
						hrOpenHistogram = m_pIOpenRowset->OpenRowset(NULL,
							&dbidTable,
							&dbidStat,
							IID_IRowset,
							0,
							NULL,
							(IUnknown **)&pIRowset);

						// If there's a histogram then validate values
						if (ulStatType & DBSTAT_HISTOGRAM)
						{
							// Histograms are only supported on first column
							// of a multicolumn statistic.
							if (pulOrdinal && *pulOrdinal == 1)
							{

								ULONG * pcRanges = (ULONG *)GetValuePtr(TS_NO_OF_RANGES, pData);

								// It's possible the table was deleted after we opened
								// the schema rowset
								if (S_OK == hrOpenHistogram)
								{
									if (CHECK(pIRowset != NULL, TRUE))
									{
										HRESULT		hrGetNextRows = S_OK;
										ULONG		cRows = 0;
										HROW *		phRows = NULL;
										DBCOUNTITEM	cRowsObtained = 0;
										DBORDINAL	cBinding = 0;
										DBLENGTH	cbRowSize = 0;
										DBBINDING * pBinding = NULL;
										HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
										LPBYTE		pDataRows = NULL;
										
										// Check histogram rowset columns info
										if (m_prgColInfo && !COMPARE(CheckHistogramColInfo(
											pIRowset,
											m_prgColInfo[ulIndex].wType,
											&cBinding,
											&pBinding,
											&cbRowSize,
											&hAccessor
											), TRUE))
											odtLog << L"Histogram colinfo doesn't match.\n";

										// Make sure the provider returned valid info
										if (cBinding > 0 &&
											pBinding &&
											cbRowSize > 0 &&
											hAccessor != DB_NULL_HACCESSOR)
										{
											LPBYTE pDataHist = NULL;
											DBBINDING dbBind;
											DBCOUNTITEM ulRangeCount = 0;
											DBCOUNTITEM ulTotal = 0;

											// We need the table cardinality so if not available
											// we must compute ourselves
											if (pulTableCardinality)
												ulTotal = *pulTableCardinality;
											else
												ulTotal = Cardinality(pwszQualifiedName, 0, TABLE_CARDINALITY); 

											// Create a binding structure for the other row
											// data
											memcpy(&dbBind, pBinding, sizeof(DBBINDING));
											dbBind.obStatus+=cbRowSize;
											dbBind.obLength+=cbRowSize;
											dbBind.obValue+=cbRowSize;

											// Allocate memory for two rows of histogram
											SAFE_ALLOC(pDataRows, BYTE, cbRowSize*2);

											pDataHist = pDataRows;

											while ((hrGetNextRows = pIRowset->GetNextRows(
													NULL,
													0,
													1,
													&cRowsObtained,
													&phRows)) == S_OK)
											{

												cRows++;

												if (COMPARE(cRowsObtained, 1) &&
													COMPARE(phRows != NULL, TRUE) &&
													CHECK(pIRowset->GetData(phRows[0], hAccessor, pDataHist), S_OK))
												{
													LPBYTE pRANGE_HI_KEY = GetValuePtr(HR_RANGE_HI_KEY, pDataHist, pBinding);
													TYPE_R8 * pRANGE_ROWS = (TYPE_R8 *)GetValuePtr(HR_RANGE_ROWS, pDataHist, pBinding);
													TYPE_R8 * pEQ_ROWS = (TYPE_R8 *)GetValuePtr(HR_EQ_ROWS, pDataHist, pBinding);
													TYPE_I8 * pDISTINCT_RANGE_ROWS = 
														(TYPE_I8 *)GetValuePtr(HR_DISTINCT_RANGE_ROWS, pDataHist, pBinding);
													DBBINDING * pPrevBind=NULL;
													DBBINDING * pCurrentBind = pBinding;
													double fFraction, fPercent;

													// Set pointers to previous and current bindings for RANGE_HIGH_KEY
													if (cRows > 1)
													{
														if (pDataHist == pDataRows)
														{
															pCurrentBind = pBinding;
															pPrevBind = &dbBind;
														}
														else
														{
															pCurrentBind = &dbBind;
															pPrevBind = pBinding;
														}
													}


													// Validate this row
													
													// RANGE_HI_KEY is mandatory
													COMPARE(pRANGE_HI_KEY != NULL, TRUE);

													// One of RANGE_ROWS or EQ_ROWS is required
													COMPARE(pRANGE_ROWS != NULL ||
															pEQ_ROWS != NULL, TRUE);

													// Validate sort is by RANGE_HI_KEY, ascending order

													// Validate data for each returned column
													// RANGE_HI_KEY

													/* Actually, this may not necessarily be a value in the column
														It may not even be within the range of values in the table, so we
														really can't test this.

														// Must be a value in the column
														if (pRANGE_HI_KEY)
														{																			
															ulRangeCount = Cardinality(pwszQualifiedName, ulIndex, EQ_ROWS_CARDINALITY, 
																NULL, pCurrentBind, pDataRows, cbRowSize*2);

															// Fail if no value in table == RANGE_HIGH_KEY
															if (!COMPARE(ulRangeCount > 0, TRUE))
																odtLog << pwszTableName << L": RANGE_HI_KEY not in base table.\n";
														}
													*/

													// RANGE_ROWS
														// Fraction of number of rows that fall in this histogram 
														// range.  Select rows in this range and
														// divide by table cardinality.
														if (pRANGE_ROWS)
														{																			
															ulRangeCount = Cardinality(pwszQualifiedName, ulIndex, RANGE_ROWS_CARDINALITY, 
																pPrevBind, pCurrentBind, pDataRows, cbRowSize*2);
															fFraction = (float)(DB_LORDINAL)ulRangeCount/(float)(DB_LORDINAL)ulTotal;

															// Compute percentage error in result, avoid division by 0
															if (fFraction > 0.0)
																fPercent = fabs((*(double *)pRANGE_ROWS - fFraction)/fFraction);
															else
																fPercent = 0.0;

															// Fail if off by more than 10%
															CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
																EC_BAD_RANGE_ROWS,
																L"RANGE_ROWS not within 10%",
																FALSE);
														}


													// EQ_ROWS
														// Fraction of rows equal to RANGE_HIGH_KEY.
														// Select rows equal and divide by table cardinality.
														if (pEQ_ROWS)
														{																			
															ulRangeCount = Cardinality(pwszQualifiedName, ulIndex,	EQ_ROWS_CARDINALITY, 
																NULL, pCurrentBind, pDataRows, cbRowSize*2);
															fFraction = (float)(DB_LORDINAL)ulRangeCount/(float)(DB_LORDINAL)ulTotal;

															// Compute percentage error in result, avoid division by 0
															if (fFraction > 0.0)
																fPercent = fabs((*(double *)pEQ_ROWS - fFraction)/fFraction);
															else
																fPercent = 0.0;

															// Fail if off by more than 10%
															CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
																EC_BAD_EQ_ROWS,
																L"EQ_ROWS not within 10%",
																FALSE);
														}

													// DISTINCT_RANGE_ROWS
														// Number of distinct values in this range.
														// Select distinct rows in this range.
														if (pDISTINCT_RANGE_ROWS)
														{			
															ulRangeCount = Cardinality(pwszQualifiedName, ulIndex,	DISTINCT_RANGE_ROWS_CARDINALITY, 
																pPrevBind, pCurrentBind, pDataRows, cbRowSize*2);

															// Compute percentage error in result, avoid division by 0
															if (ulRangeCount)
																fPercent = fabs(float((*(LONGLONG *)pDISTINCT_RANGE_ROWS - (LONGLONG)ulRangeCount))/(float)(DB_LORDINAL)ulRangeCount);
															else
																fPercent = 0.0;

															// Fail if off by more than 10%
															CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
																EC_BAD_DISTINCT_RANGE_ROWS,
																L"DISTINCT_RANGE_ROWS not within 10%",
																FALSE);
														}
													
												}

												// Release the row
												CHECK(pIRowset->ReleaseRows(1, phRows, NULL, NULL, NULL), S_OK);

												// Point the pDataHist buffer to other half of total buffer
												pDataHist = pDataRows+cbRowSize*(cRows % 2);
												ReleaseInputBindingsMemory(cBinding, pBinding, pDataHist, FALSE);
											}

											// Free the row handles
											SAFE_FREE(phRows);

											// Release memory for the previous row we retrieved
											if (cRows)
												ReleaseInputBindingsMemory(cBinding, pBinding, pDataRows+cbRowSize*((cRows-1) % 2), FALSE);

											SAFE_FREE(pDataRows);

											// Make sure we got DB_S_ENDOFROWSET
											CHECK(hrGetNextRows, DB_S_ENDOFROWSET);
											
											// Make sure row count matches NO_OF_RANGES
											if (pcRanges)
												COMPARE(cRows, *pcRanges);
										}
									}

								}
								else
								{
									CHECK(pIRowset == NULL, TRUE);
									// We allow the histogram failure if the table no longer
									// exists.
									if (!CHECK(hrOpenHistogram, DB_E_NOTABLE))
										odtLog << L"Histogram rowset not returned.\n";
								}
							}
						}
						else
						{
							// If the table was deleted the provider may return
							// DB_E_NOTABLE instead of DB_E_NOSTATISTIC
							if (hrOpenHistogram != DB_E_NOTABLE)
							{
								CCHECK(m_EC, hrOpenHistogram, DB_E_NOSTATISTIC,
									EC_BAD_HR_OPENHISTOGRAM,
									L"Unexpected return code from OpenRowset on histogram.",
									FALSE);
							}
						}

						SAFE_RELEASE(pIRowset);


					} 

				}

				PRVTRACE(L"'%d'\n",(WCHAR *)pColumn->bValue);
				break;
			}
			case TS_COLUMN_NAME:
				// Sanity check

				// Column must have non-empty string and cannot be NULL
				if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
				{
					COMPARE(wcslen((TYPE_WSTR)pColumn->bValue) > 0, TRUE);

					// Detail check
					if (m_fDetailCheck && SUCCEEDED(hrOpenTable))
						// Column name must match columns info for this ordinal
						COMPARE(RelCompareString(m_prgColInfo[ulIndex].pwszName, (TYPE_WSTR)pColumn->bValue), 0);
				}
				break;
			case TS_COLUMN_GUID:
				// Sanity check: none

				// Detail check
				if (m_fDetailCheck && SUCCEEDED(hrOpenTable))
				{
					// Column guid must match columns info
					if (S_OK == pColumn->sStatus)
					{
						COMPARE(m_prgColInfo[ulIndex].columnid.uGuid.guid == *(GUID *)pColumn->bValue, TRUE);
					}
					else
					{
						// Must be NULL
//						COMPARE(m_prgColInfo[ulIndex].columnid.uGuid.guid == DB_NULLGUID, TRUE);
					}
				}
				break;
			case TS_COLUMN_PROPID:
				// Sanity check: none

				// Detail check
				if (m_fDetailCheck && SUCCEEDED(hrOpenTable))
				{
					// Column propid must match columns info
					if (S_OK == pColumn->sStatus)
						COMPARE(m_prgColInfo[ulIndex].columnid.uName.ulPropid == *(ULONG *)pColumn->bValue, TRUE);

					// If status == DBSTATUS_S_ISNULL, then value is undefined and can't be
					// compared.  And it appears ulPropid in columnsinfo is uninitialized.
				}
				break;
			case TS_ORDINAL_POSITION:
			// The ordinal position must be sequential within the table
			{
				// Column cannot be null
				if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
				{
					// If we have a valid column name
					if (pwszTableName && pwszStatName)
					{
						if (fNewStat)
						{
							// This is a new statistic, ordinal must be 1
							m_iOrdinalExpected = 1;
							SAFE_FREE(m_pwszTableName);
							SAFE_FREE(m_pwszStatName);
							m_pwszTableName = wcsDuplicate(pwszTableName);
							m_pwszStatName = wcsDuplicate(pwszStatName);
						}
						else
							m_iOrdinalExpected++;

						// Compare Ordinal returned with expected
						if (!COMPARE(m_iOrdinalExpected, *(TYPE_UI4 *)pColumn->bValue))
							odtLog << pwszTableName << L": Invalid ordinal value returned.\n";
					}

				}
				break;
			}
			case TS_SAMPLE_PCT:
			case TS_LAST_UPDATE_TIME:
			case TS_NO_OF_RANGES:
				// This is tested for accuracy by counting rows in histogram rowset.
				break;
			case TS_COLUMN_CARDINALITY:
				eCardinality = COLUMN_CARDINALITY;
				// Fall through
			case TS_TUPLE_CARDINALITY:
				// This should equal number distinct rows in table for this column
				if (m_fDetailCheck && SUCCEEDED(hrOpenTable) &&
					pColumn->sStatus == DBSTATUS_S_OK)
				{
					DBCOUNTITEM	cRows = Cardinality(pwszQualifiedName, ulIndex, eCardinality);
					
					// Note test bug here Cardinality() doesn't include NULL rows.
					CCOMPARE(m_EC, cRows == *(ULONG *)pColumn->bValue, 
						EC_BAD_COL_OR_TUPLE_CARD,
						L"Column or Tuple cardinality is incorrect",
						FALSE);
				}
				break;
			case TS_TABLE_CARDINALITY:
				// Spec doesn't indicate this can be NULL.
				COMPARE(pColumn->sStatus, DBSTATUS_S_OK);

				// Detail check

				// This should equal number of rows in table
				if (m_fDetailCheck && SUCCEEDED(hrOpenTable) &&
					pColumn->sStatus == DBSTATUS_S_OK)
				{
					DBCOUNTITEM	cRows = Cardinality(pwszQualifiedName, ulIndex, TABLE_CARDINALITY);

					CCOMPARE(m_EC, cRows == *(ULONG *)pColumn->bValue, 
						EC_BAD_TABLE_CARDINALITY,
						L"Table cardinality is incorrect",
						FALSE);
				}
				break;
			case TS_AVG_COLUMN_LENGTH:
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"TABLE STATISTICS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - TABLE STATISTICS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
			COMPARE(pColumn->sStatus, DBSTATUS_S_OK);
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
			COMPARE(pColumn->sStatus, DBSTATUS_S_OK);
		}
	}

CLEANUP:

	SAFE_FREE(pwszQualifiedName);

	return fResults;
}

//--------------------------------------------------------------------
// TABLES
// 1. Catalog Name
// 2. Schema Name
// 3. Table Name
// 4. Table Type
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TABLES()
{
	// Set the Schema column Count, Names and Types
	m_cColumns = cTABLES;
	m_rgColumnNames = (WCHAR **) rgwszTABLES;
	m_rgColumnTypes = (DBTYPE *) rgtypeTABLES;
	
	// Set the count of restrictions
	m_cRestrictions = cTABLES_RESTRICTIONS;

	// Set Valid Table Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszTable_TypeRestriction);

	// Set an Invalid Table Restriction
	if(m_restrict & FIFTH) {
		SetRestriction(FIFTH,5, &m_wszR5, m_pwszTableRestriction);
		m_cRestrictions = 5;
	}

	// Set expected row count.  Since we create a table there must be 
	// at least one.
	SetRowCount(MIN_REQUIRED, 1);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTABLES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TABLES(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind;			// Binding Count
    DATA *	pColumn;		// Data Structure
	CCol	col;
	BOOL	fResults = TRUE;

	// Try to open a rowset on the table
	GetColumnInfo(pData, m_rgDBBINDING);
	
	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTABLES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
		switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
		{
			// TABLE_CATALOG
			case 1:
				fResults &= TestReturnData(iRow,pColumn,FIRST,&m_fRes1,m_wszR1);
				break;

			// TABLE_SCHEMA
			case 2:
				fResults &= TestReturnData(iRow,pColumn,SECOND,&m_fRes2,m_wszR2);
				break;

			// TABLE_NAME
			case 3:
				fResults &= TestReturnData(iRow,pColumn,THIRD,&m_fRes3,m_wszR3);
				break;

			// TABLE_TYPE
			case 4:
				fResults &= TestReturnData(iRow,pColumn,FOURTH,&m_fRes4,m_wszR4,FALSE);

				// Check the only spec'ed Types
				if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"ALIAS")) && 
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"TABLE")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYNONYM")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYSTEM TABLE")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"GLOBAL TEMPORARY")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"LOCAL TEMPORARY")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYSTEM VIEW")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"VIEW")))
				{
					odtLog << L"Provider specific TABLE TYPE was returned by the provider: " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}
				break;
			
			// TABLE_GUID
			case 5:
				if(pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(pColumn->ulLength, sizeof(GUID));
				else if(pColumn->sStatus == DBSTATUS_S_ISNULL)
				{
					// COMPARE(pColumn->ulLength, 0);  // Not required per spec.
				}
				break;
			
			// DESCRIPTION
			case 6:
				if(pColumn->sStatus == DBSTATUS_S_ISNULL)
				{
					// COMPARE(pColumn->ulLength, 0);  // Not required per spec.
				}
				break;

			// TABLE_PROPID
			case 7:
				if(pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(pColumn->ulLength, sizeof(ULONG));
				else if(pColumn->sStatus == DBSTATUS_S_ISNULL)
				{
					// COMPARE(pColumn->ulLength, 0);  // Not required per spec.
				}
				break;

			// DATE_CREATED
			// DATE_MODIFIED
			case 8:
			case 9:
				if(pColumn->sStatus == DBSTATUS_S_OK)
					COMPARE(pColumn->ulLength, sizeof(DATE));
				else if(pColumn->sStatus == DBSTATUS_S_ISNULL)
				{
					// COMPARE(pColumn->ulLength, 0);  // Not required per spec.
				}
				break;

			// PROVIDER SPECIFIC
			default:
				if (iRow == 1)
				{
					if(!m_rgDBCOLUMNINFO[iBind].iOrdinal) 
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"TABLES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - TABLES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
		}
	}

	return fResults;
}

//--------------------------------------------------------------------
// TABLES_INFO
// 1. Catalog Name
// 2. Schema Name
// 3. Table Name
// 4. Table Type
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TABLES_INFO()
{
	// Set the Schema column Count, Names and Types
	m_cColumns = cTABLES_INFO;
	m_rgColumnNames = (WCHAR **) rgwszTABLES_INFO;
	m_rgColumnTypes = (DBTYPE *) rgtypeTABLES_INFO;
	
	// Set the count of restrictions
	m_cRestrictions = cTABLES_INFO_RESTRICTIONS;

	// Set Valid TableInfo Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszTable_TypeRestriction);

	// Set an Invalid TableInfo Restriction
	if(m_restrict & FIFTH) {
		SetRestriction(FIFTH,5, &m_wszR5, m_pwszTableRestriction);
		m_cRestrictions = 5;
	}

	// Set expected row count.  Since we create a table there must be 
	// at least one.
	SetRowCount(MIN_REQUIRED, 1);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTABLES_INFO
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TABLES_INFO(DBCOUNTITEM iRow, BYTE * pData)
{
	ULONG 	iBind;			// Binding Count
    DATA *	pColumn;		// Data Structure
	CCol	col;
	BOOL	fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTABLES_INFO <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
		switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
		{
			// TABLE_CATALOG
			case 1:
				fResults &= TestReturnData(iRow,pColumn,FIRST,&m_fRes1,m_wszR1);
				break;

			// TABLE_SCHEMA
			case 2:
				fResults &= TestReturnData(iRow,pColumn,SECOND,&m_fRes2,m_wszR2);
				break;

			// TABLE_NAME
			case 3:
				fResults &= TestReturnData(iRow,pColumn,THIRD,&m_fRes3,m_wszR3);
				break;

			// TABLE_TYPE
			case 4:
				fResults &= TestReturnData(iRow,pColumn,FOURTH,&m_fRes4,m_wszR4);

				// Check the only spec'ed Types
				if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"ALIAS")) && 
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"TABLE")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYNONYM")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYSTEM TABLE")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"GLOBAL TEMPORARY")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYSTEM TEMPORARY")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"SYSTEM VIEW")) &&
					(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"VIEW")))
				{
					odtLog << L"VerifyRow_TABLES_INFO:TABLE TYPE expected ALIAS/TABLE/SYNONYM/SYSTEM TABLE/GLOBAL/SYSTEM/VIEW but recieved " 
						<< (TYPE_WSTR)pColumn->bValue << ENDL;
					fResults = FALSE;
				}

				break;
			
			// TABLE_GUID
			case 5:
				break;
			
			// BOOKMARKS
			case 6:
				break;

			// BOOKMARK_TYPE
			// BOOKMARK_DATATYPE
			// BOOKMARK_MAXIMUM
			// BOOKMARK_INFORMATION
			case 7:
			case 8:
			case 9:
			case 10:
				break;

			// TABLE_VERSION
			case 11:
				break;

			// CARDINALITY
			case 12:
				break;

			// DESCRIPTION
			case 13:
				break;

			// TABLE_PROPID
			case 14:
				break;

			// PROVIDER SPECIFIC
			default:
				if (iRow == 1)
				{
					if(!m_rgDBCOLUMNINFO[iBind].iOrdinal) 
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"TABLES_INFO: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - TABLES_INFO provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
		}
	}

	return fResults;
}

//--------------------------------------------------------------------
// TRANSLATIONS
// 1. Catalog Name
// 2. Schema Name
// 3. Translation Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TRANSLATIONS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszTRANSLATIONS;
	m_rgColumnTypes = (DBTYPE *)rgtypeTRANSLATIONS;

	// Set the count of columns and restrictions
	m_cColumns = cTRANSLATIONS;
	m_cRestrictions = cTRANSLATIONS_RESTRICTIONS;
	
	// Set Transalation Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTranslationReplace);

	PRVTRACE(L"GetSchemaInfo::TRANSLATIONS\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTRANSLATIONS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TRANSLATIONS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszTranslationReplace)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTRANSLATIONS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:TRANSLATIONS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
				switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
				{
				case 1:// TABLE_CATALOG
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog <<  L"VerifyRow_TRANSLATIONS: TRANSLATION CATALOG restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 2:// TABLE_SCHEMA
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog <<  L"VerifyRow_TRANSLATIONS: TRANSLATION SCHEMA restriction failed\n";
								m_fRes2=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 3:// Translation Name
					if(m_fCaptureRestrictions)
					{
						m_pwszTranslationReplace = (TYPE_WSTR) PROVIDER_ALLOC
							((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszTranslationReplace)
							wcscpy(m_pwszTranslationReplace,(TYPE_WSTR) pColumn->bValue);
					}

					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog <<  L"VerifyRow_TRANSLATIONS: TRANSLATION NAME restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
					break;
				case 4:	// SOURCE CHARACTER SET CATALOG
				case 5: // SOURCE CHARACTER SET SCHEMA
				case 6: // SOURCE CHARACTER SET NAME
				case 7: // TARGET CHARACTER SET CATALOG
				case 8: // TARGET CHARACTER SET SCHEMA
				case 9: // TARGET CHARACTER SET NAME
						PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
						break;
				default:
					// We found a column not spec'd for this schema rowset, print a warning.
					if (iRow == 1)
					{
						if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
						{
							if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
								odtLog << L"TRANSLATIONS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
						}
						else
							odtLog << L"Warning - TRANSLATIONS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
					}
					break;
				}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}


//--------------------------------------------------------------------
// TRUSTEE
// 1. TRUSTEE_NAME
// 2. TRUSTEE_GUID
// 3. TRUSTEE_TYPE
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_TRUSTEE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszTRUSTEE;
	m_rgColumnTypes = (DBTYPE *)rgtypeTRUSTEE;

	// Set the count of columns and restrictions
	m_cColumns = cTRUSTEE;
	m_cRestrictions = cTRUSTEE_RESTRICTIONS;

	odtLog << L"*** Add restriction values for DBSCHEMA_TRUSTEE.\n";

	// Set TRUSTEE Restrictions
//	SetRestriction(FIRST, 1, &m_wszR1, m_pwszTrusteeName);
//	SetRestriction(SECOND,2, &m_wszR2, m_pwszTrusteeGUID);
//	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTrusteePROPID);
//	SetRestriction(FOURTH,4, &m_wszR4, m_pwszTrusteeType);

	PRVTRACE(L"GetSchemaInfo::TRUSTEE\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowTRANSLATIONS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_TRUSTEE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszTranslationReplace)
		return FALSE;

	odtLog << L"*** Currently not testing values returned from DBSCHEMA_TRUSTEE.\n";

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cTRUSTEE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:TRUSTEE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
				switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
				{
				case 1:// TRUSTEE_NAME
/*
					if(m_restrict & FIRST)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes1)
							{
								odtLog <<  L"VerifyRow_TRUSTEE: TRUSTEE_NAME restriction failed\n";
								m_fRes1=FALSE;
								fResults = FALSE;
							}
						}
					}
*/
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 2:// TRUSTEE_GUID
/*
					if(m_restrict & SECOND)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes2)
							{
								odtLog <<  L"VerifyRow_TRUSTEE: TRUSTEE_GUID restriction failed\n";
								m_fRes2=FALSE;
								fResults = FALSE;
							}
						}
					}
*/
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
				case 3:// TRUSTEE_PROPID
/*
					if(m_fCaptureRestrictions)
					{
						m_pwszTranslationReplace = (TYPE_WSTR) PROVIDER_ALLOC
							((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszTranslationReplace)
							wcscpy(m_pwszTranslationReplace,(TYPE_WSTR) pColumn->bValue);
					}

					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog <<  L"VerifyRow_TRUSTEE: TRUSTEE_PROPID restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
*/
					PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
					break;
				case 4:	// TRUSTEE_TYPE
						PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
						break;
				default:
					// We found a column not spec'd for this schema rowset, print a warning.
					if (iRow == 1)
					{
						if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
						{
							if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
								odtLog << L"TRANSLATIONS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
						}
						else
							odtLog << L"Warning - TRANSLATIONS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
					}
					break;
				}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}


//--------------------------------------------------------------------
// USAGE_PRIVILGES
// 1. Catalog Name
// 2. Schema Name
// 3. Object Name
// 4. Object Type
// 5. Grantor
// 6. Grantee
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_USAGE_PRIVILEGES()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszUSAGE_PRIVILEGES;
	m_rgColumnTypes = (DBTYPE *)rgtypeUSAGE_PRIVILEGES;

	// Set the count of columns and restrictions
	m_cColumns = cUSAGE_PRIVILEGES;
	m_cRestrictions = cUSAGE_PRIVILEGES_RESTRICTIONS;
	
	// Set Object Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszObjectRestriction);
	SetRestriction(FOURTH,4, &m_wszR4, m_pwszObject_TypeRestriction);
	SetRestriction(FIFTH, 5, &m_wszR5, m_pwszGrantorRestriction);
	SetRestriction(SIXTH, 6, &m_wszR6, m_pwszGranteeRestriction);

	PRVTRACE(L"GetSchemaInfo::USAGE_PRIVILGES\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowUSAGE_PRIVILEGES
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_USAGE_PRIVILEGES
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszObjectRestriction && m_pwszObject_TypeRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cUSAGE_PRIVILEGES <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:USAGE_PRIVILEGES:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://GRANTOR
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog <<  L"VerifyRow_USAGE_PRIVILEGES:GRANTOR restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// GRANTEE
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog << L"VerifyRow_USAGE_PRIVILEGES:GRANTEE restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// OBJECT CATALOG
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_USAGE_PRIVILEGES:OBJECT CATALOG restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// OBJECT SCHEMA
				if(m_restrict & SECOND)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR2,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes2)
						{
							odtLog <<  L"VerifyRow_USAGE_PRIVILEGES:OBJECT SCHEMA restriction failed\n";
							m_fRes2=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 5:// OBJECT NAME
					if(m_fCaptureRestrictions)
					{
						m_pwszObjectRestriction = (WCHAR *) PROVIDER_ALLOC
							((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszObjectRestriction)
							wcscpy(m_pwszObjectRestriction,(TYPE_WSTR) pColumn->bValue);
					}
					
					if(m_restrict & THIRD)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR3,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes3)
							{
								odtLog << L"VerifyRow_USAGE_PRIVILEGES:OBJECT NAME restriction failed\n";
								m_fRes3=FALSE;
								fResults = FALSE;
							}
						}
					}
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 6:	// OBJECT TYPE
					if(m_restrict & FOURTH)
					{
						if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR4,(TYPE_WSTR)pColumn->bValue)))
						{
							if(m_fRes4)
							{
								odtLog << L"VerifyRow_USAGE_PRIVILEGES:OBJECT TYPE restriction failed\n";
								m_fRes4=FALSE;
								fResults = FALSE;
							}
						}
					}
					if(m_fCaptureRestrictions)
					{
						m_pwszObject_TypeRestriction = (WCHAR *) PROVIDER_ALLOC
							((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR)) + sizeof(WCHAR));
						if(m_pwszObject_TypeRestriction)
							wcscpy(m_pwszObject_TypeRestriction,(TYPE_WSTR) pColumn->bValue);
					}
					if( (0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"DOMAIN")) && 
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"CHARACTER SET")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"COLLATION")) &&
						(0!=wcscmp((TYPE_WSTR)pColumn->bValue,L"TRANSLATION")))
					{
						odtLog << L"VerifyRow_USAGE_PRIVILEGES:OBJECT TYPE expected DOMAIN/CHARACTERSET/COLLATION/TRANSLATION but recieved " 
							<< (TYPE_WSTR)pColumn->bValue << ENDL;
						fResults = FALSE;
					}
					PRVTRACE(L"'%s'\n",(WCHAR *)pColumn->bValue);
					break;
			case 7: // PRIVILEGE TYPE
  					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 8: // IS_GRANTABLE
				if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
					PRVTRACE(L"TRUE\n");
				else
					PRVTRACE(L"FALSE\n");
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"USAGE PRIVILEGES: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - USAGE PRIVILEGES provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
			
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}

//--------------------------------------------------------------------
// VIEW_COLUMN_USAGE
// 1. Catalog Name
// 2. Schema Name
// 3. View Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_VIEW_COLUMN_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszVIEW_COLUMN_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeVIEW_COLUMN_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cVIEW_COLUMN_USAGE;
	m_cRestrictions = cVIEW_COLUMN_USAGE_RESTRICTIONS;
	
	// Set View Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszViewRestriction);

	PRVTRACE(L"GetSchemaInfo::VIEW_COLUMN_USAGE\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowVIEW_COLUMN_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_VIEW_COLUMN_USAGE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cVIEW_COLUMN_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:VIEW_COLUMN_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://VIEW CATALOG
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_VIEW_COLUMN_USAGE: VIEW CATALOG restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// VIEW SCHEMA
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog << L"VerifyRow_VIEW_COLUMN_USAGE:VIEW SCHEMA restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// VIEW NAME
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_VIEW_COLUMN_USAGE:VIEW NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE CATALOG
			case 5:// TABLE SCHEMA
			case 6:// TABLE NAME
			case 7:// COLUMN NAME
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 8: // COLUMN GUID
  					PRVTRACE(L"\n");
					break;
			case 9: // COLUMN PROPID
					PRVTRACE(L"%d\n",*(TYPE_UI4 *)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"VIEW COLUMN USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - VIEW COLUMN USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}

//--------------------------------------------------------------------
// VIEW_TABLE_USAGE
// 1. Catalog Name
// 2. Schema Name
// 3. View Name
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_VIEW_TABLE_USAGE()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszVIEW_TABLE_USAGE;
	m_rgColumnTypes = (DBTYPE *)rgtypeVIEW_TABLE_USAGE;

	// Set the count of columns and restrictions
	m_cColumns = cVIEW_TABLE_USAGE;
	m_cRestrictions = cVIEW_TABLE_USAGE_RESTRICTIONS;
	
	// Set View Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszViewRestriction);

	PRVTRACE(L"GetSchemaInfo::VIEW_TABLE_USAGE\n");
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowVIEW_TABLE_USAGE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_VIEW_TABLE_USAGE
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{

	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// don't need to go farther if I have what I'm looking for
	if(m_fCaptureRestrictions && m_pwszViewRestriction)
		return FALSE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cVIEW_TABLE_USAGE <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		
//		PRVTRACE(L"Row[%lu],Col[%s]:VIEW_TABLE_USAGE:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://VIEW CATALOG
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_VIEW_TABLE_USAGE: VIEW CATALOG restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// VIEW SCHEMA
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog << L"VerifyRow_VIEW_TABLE_USAGE:VIEW SCHEMA restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// VIEW NAME
				if(m_fCaptureRestrictions)
				{
					m_pwszViewRestriction = (WCHAR *) PROVIDER_ALLOC
						((wcslen((WCHAR *) pColumn->bValue)*sizeof(WCHAR*)) + sizeof(WCHAR));
					if(m_pwszViewRestriction)
						wcscpy(m_pwszViewRestriction,(TYPE_WSTR) pColumn->bValue);
				}

				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_VIEW_TABLE_USAGE:VIEW NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// TABLE CATALOG
			case 5:// TABLE SCHEMA
			case 6:// TABLE NAME
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"VIEW TABLE USAGE: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - VIEW TABLE USAGE provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	
	return fResults;
}

//--------------------------------------------------------------------
// VIEWS
// 1. Table Catalog 
// 2. Table Schema 
// 3. Table View 
//--------------------------------------------------------------------
BOOL CSchemaTest::PrepareParams_VIEWS()
{
	// Set the Schema column Names and Types
	m_rgColumnNames = (WCHAR **)rgwszVIEWS;
	m_rgColumnTypes = (DBTYPE *)rgtypeVIEWS;

	// Set the count of columns and restrictions
	m_cColumns = cVIEWS;
	m_cRestrictions = cVIEWS_RESTRICTIONS;
	
	// Set View Restrictions
	SetRestriction(FIRST, 1, &m_wszR1, m_pwszCatalogRestriction);
	SetRestriction(SECOND,2, &m_wszR2, m_pwszSchemaRestriction);
	SetRestriction(THIRD, 3, &m_wszR3, m_pwszTableRestriction);
		
	PRVTRACE(L"GetSchemaInfo::VIEWS\n");
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyRowVIEWS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRow_VIEWS
(
	DBCOUNTITEM iRow,
	BYTE * pData
)
{
	ULONG 			iBind;			// Binding Count
    DATA *			pColumn;		// Data Structure
	CCol			col;
	BOOL			fResults = TRUE;

	// Check the count of columns returned
	if(iRow == 1)
		COMPARE(cVIEWS <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);

//		PRVTRACE(L"Row[%lu],Col[%s]:VIEWS:", iRow, m_rgDBCOLUMNINFO[iBind].pwszName);

		if(pColumn->sStatus==DBSTATUS_S_OK)
		{
			switch(m_rgDBCOLUMNINFO[iBind].iOrdinal)
			{
			case 1://TABLE CATALOG
				if(m_restrict & FIFTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR5,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes5)
						{
							odtLog << L"VerifyRow_VIEWS: TABLE CATALOG restriction failed\n";
							m_fRes5=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 2:// TABLE SCHEMA
				if(m_restrict & SIXTH)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR6,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes6)
						{
							odtLog <<  L"VerifyRow_VIEWS:TABLE SCHEMA restriction failed\n";
							m_fRes6=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 3:	// TABLE NAME
				if(m_restrict & FIRST)
				{
					if(!COMPARE(0, _wcsicmp((TYPE_WSTR)m_wszR1,(TYPE_WSTR)pColumn->bValue)))
					{
						if(m_fRes1)
						{
							odtLog << L"VerifyRow_VIEWS:TABLE NAME restriction failed\n";
							m_fRes1=FALSE;
							fResults = FALSE;
						}
					}
				}
				PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
				break;
			case 4:// VIEW_DEFINITION
			case 7:// DESCRIPTION
					PRVTRACE(L"'%s'\n",(TYPE_WSTR)pColumn->bValue);
					break;
			case 5:// CHECK_OPTION
			case 6:// IS_UPDATEABLE
				if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
					PRVTRACE(L"TRUE\n");
				else
					PRVTRACE(L"FALSE\n");
				break;
			// DATE_CREATED
			// DATE_MODIFIED
			case 8:
			case 9:
				break;
			default:
				// We found a column not spec'd for this schema rowset, print a warning.
				if (iRow == 1)
				{
					if (m_rgDBCOLUMNINFO[iBind].iOrdinal == 0)
					{
						if(!GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset))
							odtLog << L"VIEWS: Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
					}
					else
						odtLog << L"Warning - VIEWS provider specific column name: " << m_rgDBCOLUMNINFO[iBind].pwszName << "\n";
				}
				break;
			}
		}
		else if (pColumn->sStatus==DBSTATUS_S_TRUNCATED)
		{
			// Have to flag error.
			odtLog << L"DBSTATUS_S_TRUNCATED: " << (TYPE_WSTR)pColumn->bValue << L"\n";
			// We shouldn't be flagging an error for LONG columns, we expect truncation.
			if (!(m_rgDBCOLUMNINFO[iBind].dwFlags & DBCOLUMNFLAGS_ISLONG))
				fResults = FALSE;
		}
		else
		{
			PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
			RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
		}
	}

	return fResults;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::CheckResults(IUnknown * pIRowset, IID iid)
{	
	BOOL fResult = TRUE;

	// if I didn't get rowset back when I expected FIRST, return error
	if(SUCCEEDED(m_HR))
	{ 
		if( ((m_iid==IID_IRowset) && (!m_pIRowset)) ||
			((m_iid==IID_IAccessor) && (!m_pIAccessor)) ||
			((m_iid==IID_IRowsetInfo) && (!m_pIRowsetInfo)) ||
			((m_iid==IID_IColumnsInfo) && (!m_pIColumnsInfo))||
			((m_iid==IID_IRowsetChange) && (!m_pIRowsetChange)) )
		{	
			odtLog <<  L"CheckResults:expected rowset but didn't get FIRST\n";
			return FALSE;
		}

   		// check that the riid pointer can be used,
		// regardless of which riid it is
		fResult &= CheckRIID(pIRowset, iid);
		fResult &= VerifyRowset(pIRowset);			
	}
	else // make sure rowset if empty
	{
		if( ((m_iid==IID_IRowset) && (m_pIRowset)) ||
			((m_iid==IID_IAccessor) && (m_pIAccessor)) ||
			((m_iid==IID_IRowsetInfo) && (m_pIRowsetInfo))||
			((m_iid==IID_IColumnsInfo) && (m_pIColumnsInfo))||
			((m_iid==IID_IRowsetChange) && (m_pIRowsetChange)))
		{
			odtLog << L"CheckResults: method returned error and valid rowset pointer!\n";
			return FALSE;
		}
	}

	return fResult;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CheckAgainstIColumnsInfo
//
// Checks the DBCOLUMNINFO array returned by IColumnsInfo::GetColumnsInfo against
// the what the spec says the column name, type and order should be
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::CheckAgainstIColumnsInfo()
{
	BOOL	fResult			= TRUE;
	BOOL	fCountNotChecked= FALSE;
	ULONG	ulIndex			= 0;

	COMPARE(m_cColumns <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// Make sure the rowset is not null
	if((!m_pIRowset) || (!m_cDBCOLUMNINFO) || (!m_rgDBCOLUMNINFO) || (!m_pStringsBuffer)) {
		odtLog << L"CheckAgainstIColumnsInfo:Either rowset or DBCOLUMNINFO variables are null."<<ENDL;
		return FALSE;
	}

	// Check for the Bookmark
	if(!m_rgDBCOLUMNINFO[0].iOrdinal)
		m_fBOOKMARK_FOUND=TRUE;
	else
		m_fBOOKMARK_FOUND=FALSE;
	
	// Check column ordering, column name and data type, skip bookmark column
	for(ulIndex = (0 + m_fBOOKMARK_FOUND); ulIndex < m_cDBCOLUMNINFO; ulIndex ++)
	{
		// only check spec'd columns, not any provider-specific columns
		if((ulIndex-m_fBOOKMARK_FOUND) < m_cColumns)
		{
			// Check column ordinal
			if(m_rgDBCOLUMNINFO[ulIndex].iOrdinal != (ulIndex + 1 - m_fBOOKMARK_FOUND))
			{
				odtLog << L"ORDINAL ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex
						<< L"]: Expected "<< ulIndex  	<< L", Received "	<< m_rgDBCOLUMNINFO[ulIndex].iOrdinal	<< ENDL;
				fResult = FALSE;
			}

			// If there is a name in this column, check to make sure it is correct
			if( (m_rgDBCOLUMNINFO[ulIndex].pwszName) && 
				(!COMPARE(0, _wcsicmp(m_rgDBCOLUMNINFO[ulIndex].pwszName,m_rgColumnNames[ulIndex-m_fBOOKMARK_FOUND]))) )
			{
				odtLog << L"NAME ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L","
					<< ulIndex 	<< L"]: Expected " 	<< 	m_rgColumnNames[ulIndex-m_fBOOKMARK_FOUND] << L", Received "
					<< m_rgDBCOLUMNINFO[ulIndex].pwszName << ENDL;
				fResult = FALSE;
			}

			// Check column type
			if(m_rgDBCOLUMNINFO[ulIndex].wType != m_rgColumnTypes[ulIndex-m_fBOOKMARK_FOUND])
			{
				odtLog << L"TYPE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
						<< 	m_rgColumnTypes[ulIndex-m_fBOOKMARK_FOUND] 	<< L", Received " << m_rgDBCOLUMNINFO[ulIndex].wType << ENDL;
				fResult = FALSE;
			}

			// Check column size
			if((!IsFixedLength(m_rgDBCOLUMNINFO[ulIndex].wType)) && (!m_rgDBCOLUMNINFO[ulIndex].ulColumnSize))
			{
				odtLog << L"COLUMNSIZE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: "
						<< L"Column Size for variable length Column is ZERO, expected non-zero." << ENDL;
				fResult = FALSE;
			}
			else if((IsFixedLength(m_rgDBCOLUMNINFO[ulIndex].wType)) && 
					((ULONG)GetDBTypeSize(m_rgDBCOLUMNINFO[ulIndex].wType) != m_rgDBCOLUMNINFO[ulIndex].ulColumnSize))
			{
				odtLog << L"COLUMNSIZE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
						<< L"Column Size for the Column is incorrect. " << ENDL;
				fResult = FALSE;
			}

			// Check column precision and scale
			// Note some provider return an alternate precision/scale for 
			// DBTYPE_CY
			if(IsNumericType(m_rgDBCOLUMNINFO[ulIndex].wType) &&
				m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_CY)
			{
				ULONG MaxPrecision = 0;

				// Switch on the Type
				switch(m_rgDBCOLUMNINFO[ulIndex].wType)
				{
					case DBTYPE_I1:
					case DBTYPE_UI1:
						MaxPrecision = 3;
						break;
					case DBTYPE_I2:
					case DBTYPE_UI2:
						MaxPrecision = 5;
						break;
					case DBTYPE_R4:
						MaxPrecision = 7;
						break;
					case DBTYPE_I4:
					case DBTYPE_UI4:
						MaxPrecision = 10;
						break;
					case DBTYPE_R8:
						MaxPrecision = 15;
						break;
					case DBTYPE_CY:
					case DBTYPE_I8:
						MaxPrecision = 19;
						break;
					case DBTYPE_UI8:
						MaxPrecision = 20;
						break;
				}

				// Precision is valid
				if (((m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_DECIMAL) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_NUMERIC) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_VARNUMERIC)) && 
					(MaxPrecision != m_rgDBCOLUMNINFO[ulIndex].bPrecision) )
				{
					odtLog << L"PRECISION ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
							<< 	MaxPrecision 	<< L", Received " << m_rgDBCOLUMNINFO[ulIndex].bPrecision << ENDL;
					fResult = FALSE;
				}

				// Scale is  ~0
				if (((m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_DECIMAL) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_NUMERIC) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_VARNUMERIC)) && 
					((BYTE)(~0) != m_rgDBCOLUMNINFO[ulIndex].bScale) )
				{
					odtLog << L"SCALE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  
						<< L"]: Expected the Scale to be ~0" << L", Received " << m_rgDBCOLUMNINFO[ulIndex].bScale << ENDL;
					fResult = FALSE;
				}
			}
			else if (m_rgDBCOLUMNINFO[ulIndex].wType == DBTYPE_DBTIMESTAMP ||
					m_rgDBCOLUMNINFO[ulIndex].wType == DBTYPE_CY)
			{
				CCol TempCol;
				BYTE bPrecision = 0;
				BYTE bScale = 0;

				// Get bPrecision and bScale for DBTYPE_DBTIMESTAMP or
				// DBTYPE_CY from CCol object
				for (DBORDINAL iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
				{
					// Get the information about the column
					if (!CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK))
					{
						fResult = FALSE;
						goto CLEANUP;
					}

					// If this is DBTYPE_DBTIMESTAMP it may be the right one
					if (m_rgDBCOLUMNINFO[ulIndex].wType == TempCol.GetProviderType())
					{
						bPrecision = TempCol.GetPrecision();
						bScale = TempCol.GetScale();

						if (bPrecision == m_rgDBCOLUMNINFO[ulIndex].bPrecision &&
							bScale == m_rgDBCOLUMNINFO[ulIndex].bScale)
						{
							// We found a matching value for precision and scale
							break;
						}
					}
				}

				// Precision is defined as length of string representation assuming max
				// allowed precision of fractional seconds component.
				// TODO: Compare against CCol. 
				if (m_rgDBCOLUMNINFO[ulIndex].bPrecision != bPrecision)
				{
					odtLog <<L"PRECISION [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:expected valid precision."<<ENDL;
					fResult = FALSE;
				}

				// Scale is just the maximum length of the string representation of 
				// fractional seconds component.
				if (m_rgDBCOLUMNINFO[ulIndex].bScale != bScale)
				{
					odtLog <<L"SCALE [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:expected valid scale."<<ENDL;
					fResult = FALSE;
				}
			}
			else
			{
				// Precision and scale are ~0
				if (((BYTE)(~0) != m_rgDBCOLUMNINFO[ulIndex].bPrecision) || ((BYTE)(~0) != m_rgDBCOLUMNINFO[ulIndex].bScale))
				{
					odtLog <<L"PRECISION & SCALE [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:non-numeric expect ~0."<<ENDL;
					fResult = FALSE;
				}
			}
		}
	}

CLEANUP:

	return fResult;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CheckRIID, QI for every mandatory interface on this object
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::CheckRIID(IUnknown * pColRowset, IID iid)
{
	HRESULT		hr			= E_FAIL;
	ULONG		cErrors		= 0;
	IUnknown*	pInterface	= NULL;

	if(!pColRowset || iid == IID_NULL)
		return FALSE;
	
	// QI for IID_IRowset
	if(FAILED(hr=pColRowset->QueryInterface(IID_IRowset,(void **) &pInterface)))
		cErrors++;
	
	SAFE_RELEASE(pInterface);

	// QI for IID_IAccessor
	if(FAILED(hr=pColRowset->QueryInterface(IID_IAccessor,(void **) &pInterface)))
		cErrors++;
	
	SAFE_RELEASE(pInterface);

	// QI for IID_IColumnsInfo
	if(FAILED(hr=pColRowset->QueryInterface(IID_IColumnsInfo,(void **) &pInterface)))
		cErrors++;
	
	SAFE_RELEASE(pInterface);
	
	// QI for IID_IRowsetInfo
	if(FAILED(hr=pColRowset->QueryInterface(IID_IRowsetInfo,(void **) &pInterface)))
		cErrors++;
	
	SAFE_RELEASE(pInterface);

	// QI for IID_IConvertType
	if(FAILED(hr=pColRowset->QueryInterface(IID_IConvertType,(void **) &pInterface)))
		cErrors++;
	
	SAFE_RELEASE(pInterface);

	if(!cErrors)
		return TRUE;
	else 
		return FALSE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// VerifyRowset, verify rowset and row order
//
// 1) Verify Maxlength, Precision, and Scale for non-applicable datatypes
// 2) Check ComputeMode
// 3) Order of rows is order of columns in query
//
// ----------------------------------------------
// 1. IColumnsInfo::GetColumnInfo, build binding (from GetAccessorAndBindings)
// 2. IAccessor::CreateAccessor (from GetAccessorAndBindings)
// Loop
//    3. IRowset::GetNextRows
//    4. IRowset::GetData
//    5. IRowset::ReleaseRows
// End Loop
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CSchemaTest::VerifyRowset(IUnknown * pIUnknown)
{
	BOOL		fVerifyRow = TRUE;
	BOOL		fFirstTimeThruLoop = TRUE;
	HRESULT		hr = E_FAIL;
	BOOL		fReleaseRowset = FALSE;			// whether I should release pIRowset;
	HACCESSOR	hAccessor=DB_NULL_HACCESSOR;	// handle to accessor
	ULONG		iBind = 0;						// current binding
	ULONG		iRow = 0;						// current row
	DBLENGTH	cbRowSize = 0;					// size of row
	DBCOUNTITEM	cRowsObtained = 0;				// total rows obtained from getnextrows
	DBCOUNTITEM	cTotalRows = 0;					// total row count
	BYTE *		pRow = NULL;					// row of Data
	HROW *		prghRows = NULL;					// array of hrows
	IAccessor * pIAccessor = NULL;
	ULONG		ulRowsToVerify;
	BOOL		fCapturingRestrictions = m_fCaptureRestrictions;
	BOOL		fContinue = TRUE;

	m_fResult = FALSE;
	m_fRes1=m_fRes2=m_fRes3=m_fRes4=m_fRes5=m_fRes6=m_fRes7=TRUE;

	// if I don't have the rowset pointer, I need to go get FIRST
	if (!pIUnknown)
	{
		if(m_iid == IID_IColumnsInfo)
			pIUnknown = m_pIColumnsInfo;
		else if(m_iid == IID_IAccessor)
			pIUnknown = m_pIAccessor;
		else if(m_iid == IID_IRowsetChange)
			pIUnknown = m_pIRowsetChange;
		else if(m_iid == IID_IRowsetInfo)
			pIUnknown = m_pIRowsetInfo;
		else if((m_iid == IID_IRowset) && (!m_pIRowset))
			return FALSE;
	}
	
	if(pIUnknown && !m_pIRowset && FAILED(hr=pIUnknown->QueryInterface(IID_IRowset,(void **)&m_pIRowset)) )
		return FALSE;

	if (!VerifyInterface(m_pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor))
		goto CLEANUP;

	fReleaseRowset = TRUE;

	// get bindings and column info
	if(!CHECK(hr=GetAccessorAndBindings(
		m_pIRowset,									// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&hAccessor,									// @parmopt [OUT] Accessor created
		&m_rgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cDBBINDING,								// @parmopt [OUT] Count of bindings
		&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Array of DBCOLUMNINFO
		&m_cDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
		goto CLEANUP;

	// Don't need to check if only capturing restrictions
	if(!m_fCaptureRestrictions)
		fVerifyRow &= CheckAgainstIColumnsInfo();

	// check if we need to go on
	if((!m_cDBBINDING) || (!m_cDBCOLUMNINFO) || (!m_rgDBBINDING) || (!m_rgDBCOLUMNINFO))
		goto CLEANUP;

	if(m_fPassUnsupportedRestrictions)
	{
		m_fResult = TRUE;
		goto CLEANUP;
	}

	// create space for row of data
	pRow = (BYTE *) PROVIDER_ALLOC(cbRowSize);
	if(!pRow)
		goto CLEANUP;

	if(GetModInfo()->GetDebugMode() & DEBUGMODE_FULL )
		ulRowsToVerify = 90000;
	else
		// This should be larger than NUMROWS_CHUNK to force at least one additional
		// GetNextRows call below.
		ulRowsToVerify = 100;
 
	// Process all the rows, NUMROWS_CHUNK rows at a time
	while(fContinue)
	{
		// get rows to process
		hr=GetNextRows(0,30,&cRowsObtained,&prghRows);


		TEST4C_(hr, S_OK, DB_S_ENDOFROWSET, DB_S_STOPLIMITREACHED, DB_S_ROWLIMITEXCEEDED);


		// verify that we have rows to process
		if(cRowsObtained==0)
		{
			if (CHECK(hr, DB_S_ENDOFROWSET))
				break;
			else
				goto CLEANUP;
		}

		// Only verify required number of rows
		if (cTotalRows < ulRowsToVerify)
		{
			// Make sure we don't call verify with no valid restrictions.
			ASSERT((m_restrict != ALLRES) || (m_currentBitMask == 0));

			// Loop over rows obtained, getting data for each
			for(iRow=0;iRow<cRowsObtained && fContinue;iRow++)
			{
				cTotalRows++;

				// get row
				if(FAILED(hr=m_pIRowset->GetData(prghRows[iRow],hAccessor,pRow)))
					goto CLEANUP;

				// make sure we got the row
				if(pRow==NULL)
					goto CLEANUP;

 				// do something with row
				fVerifyRow &= VerifyRow(m_guid,cTotalRows,pRow);

				// If we know we've filled the restriction then break out of loop
				// Otherwise we continue for all rows
				if (fCapturingRestrictions && !m_fCaptureRestrictions)
				{
					m_fCaptureRestrictions = fCapturingRestrictions;
					fContinue = FALSE;
					break;
				}
			}
		}
		else
			cTotalRows += cRowsObtained;

		if (!CHECK(hr=m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK))
			goto CLEANUP;

		fFirstTimeThruLoop = FALSE;

	}

	switch (m_eRowCount)
	{	
		case MIN_VALUE:
			// Warn if we didn't get any rows but expected some
			if (m_ulRowCount && !cTotalRows)
				odtLog << L"Warning - this rowset didn't return any rows so no values were checked.\n";

			// Fail if we got some rows but less than minimum expected
			if (cTotalRows)
				TESTC(cTotalRows >= m_ulRowCount);
			break;
		case MIN_REQUIRED:
			// Fail if we got less than minimum expected
			TESTC(cTotalRows >= m_ulRowCount);
			break;
		case EXACT_VALUE:
	//		ASSERT(cTotalRows == m_ulRowCount);
			TESTC(cTotalRows == m_ulRowCount);
			break;
		default:
			ASSERT(!L"Unexpected row count value for schema rowset.");
	}

	// If we made it to here and verified each row then set success
	if (fVerifyRow)
		m_fResult = TRUE;

CLEANUP:

	// Clear any table name information that might be left over
	SAFE_FREE(m_pwszTableName);

	if (pIAccessor && hAccessor != DB_NULL_HACCESSOR)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(m_pIRowset);

	m_cDBCOLUMNINFO = 0;
	m_cDBBINDING = 0;

	// Free the memory
	PROVIDER_FREE(m_rgDBCOLUMNINFO);
	PROVIDER_FREE(m_pStringsBuffer);
	PROVIDER_FREE(m_rgDBBINDING);
	PROVIDER_FREE(pRow);
	PROVIDER_FREE(prghRows);

	return m_fResult;
}

ULONG CSchemaTest::SchemaCatalogRestriction(ULONG ulIndex)
{
	GUID guidSchema = m_rgSchemas[ulIndex];

	return SchemaCatalogRestriction(guidSchema);
}

ULONG CSchemaTest::SchemaCatalogRestriction(GUID guidSchema)
{
	// If the GUID is one for which we know a catalog restriction exists then return FIRST.
	// Catalog restriction is always the first one.
	// This is most of them but we can't use negative logic 'cause we might get a guid we
	// don't recognize.
	if (guidSchema == DBSCHEMA_ASSERTIONS ||
		guidSchema == DBSCHEMA_CATALOGS ||
		guidSchema == DBSCHEMA_CHARACTER_SETS ||
		guidSchema == DBSCHEMA_CHECK_CONSTRAINTS ||
		guidSchema == DBSCHEMA_COLLATIONS ||
		guidSchema == DBSCHEMA_COLUMN_DOMAIN_USAGE ||
		guidSchema == DBSCHEMA_COLUMN_PRIVILEGES ||
		guidSchema == DBSCHEMA_COLUMNS ||
		guidSchema == DBSCHEMA_CONSTRAINT_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_CONSTRAINT_TABLE_USAGE ||
		guidSchema == DBSCHEMA_FOREIGN_KEYS ||
		guidSchema == DBSCHEMA_INDEXES ||
		guidSchema == DBSCHEMA_KEY_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_PRIMARY_KEYS ||
		guidSchema == DBSCHEMA_PROCEDURE_COLUMNS ||
		guidSchema == DBSCHEMA_PROCEDURE_PARAMETERS ||
		guidSchema == DBSCHEMA_PROCEDURES ||
		guidSchema == DBSCHEMA_REFERENTIAL_CONSTRAINTS ||
		guidSchema == DBSCHEMA_SCHEMATA ||
		guidSchema == DBSCHEMA_STATISTICS ||
		guidSchema == DBSCHEMA_TABLE_CONSTRAINTS ||
		guidSchema == DBSCHEMA_TABLE_PRIVILEGES ||
		guidSchema == DBSCHEMA_TABLE_STATISTICS ||
		guidSchema == DBSCHEMA_TABLES ||
		guidSchema == DBSCHEMA_TABLES_INFO ||
		guidSchema == DBSCHEMA_TRANSLATIONS ||
		guidSchema == DBSCHEMA_USAGE_PRIVILEGES ||
		guidSchema == DBSCHEMA_VIEW_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_VIEW_TABLE_USAGE ||
		guidSchema == DBSCHEMA_VIEWS)
			return FIRST;

	// else return ZERO restrictions
	return ZERO;
}

ULONG CSchemaTest::SchemaSchemaRestriction(ULONG ulIndex)
{
	GUID guidSchema = m_rgSchemas[ulIndex];

	return SchemaSchemaRestriction(guidSchema);
}

ULONG CSchemaTest::SchemaSchemaRestriction(GUID guidSchema)
{
	// If the GUID is one for which we know a catalog restriction exists then return FIRST.
	// Catalog restriction is always the first one.
	// This is most of them but we can't use negative logic 'cause we might get a guid we
	// don't recognize.
	if (guidSchema == DBSCHEMA_ASSERTIONS ||
		guidSchema == DBSCHEMA_CHARACTER_SETS ||
		guidSchema == DBSCHEMA_CHECK_CONSTRAINTS ||
		guidSchema == DBSCHEMA_COLLATIONS ||
		guidSchema == DBSCHEMA_COLUMN_DOMAIN_USAGE ||
		guidSchema == DBSCHEMA_COLUMN_PRIVILEGES ||
		guidSchema == DBSCHEMA_COLUMNS ||
		guidSchema == DBSCHEMA_CONSTRAINT_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_CONSTRAINT_TABLE_USAGE ||
		guidSchema == DBSCHEMA_FOREIGN_KEYS ||
		guidSchema == DBSCHEMA_INDEXES ||
		guidSchema == DBSCHEMA_KEY_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_PRIMARY_KEYS ||
		guidSchema == DBSCHEMA_PROCEDURE_COLUMNS ||
		guidSchema == DBSCHEMA_PROCEDURE_PARAMETERS ||
		guidSchema == DBSCHEMA_PROCEDURES ||
		guidSchema == DBSCHEMA_REFERENTIAL_CONSTRAINTS ||
		guidSchema == DBSCHEMA_SCHEMATA ||
		guidSchema == DBSCHEMA_STATISTICS ||
		guidSchema == DBSCHEMA_TABLE_CONSTRAINTS ||
		guidSchema == DBSCHEMA_TABLE_PRIVILEGES ||
		guidSchema == DBSCHEMA_TABLE_STATISTICS ||
		guidSchema == DBSCHEMA_TABLES ||
		guidSchema == DBSCHEMA_TABLES_INFO ||
		guidSchema == DBSCHEMA_TRANSLATIONS ||
		guidSchema == DBSCHEMA_USAGE_PRIVILEGES ||
		guidSchema == DBSCHEMA_VIEW_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_VIEW_TABLE_USAGE ||
		guidSchema == DBSCHEMA_VIEWS)
			return SECOND;

	// else return ZERO restrictions
	return ZERO;
}

ULONG CSchemaTest::SchemaTableRestriction(ULONG ulIndex)
{
	GUID guidSchema = m_rgSchemas[ulIndex];

	return SchemaTableRestriction(guidSchema);
}

ULONG CSchemaTest::SchemaTableRestriction(GUID guidSchema)
{
	// This is most of them but we can't use negative logic 'cause we might get a guid we
	// don't recognize.
	if (guidSchema == DBSCHEMA_COLUMN_PRIVILEGES ||
		guidSchema == DBSCHEMA_COLUMNS ||
		guidSchema == DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE ||
		guidSchema == DBSCHEMA_CONSTRAINT_COLUMN_USAGE ||
		guidSchema == DBSCHEMA_CONSTRAINT_TABLE_USAGE ||
		guidSchema == DBSCHEMA_FOREIGN_KEYS ||		// This is really PK_TABLE_NAME, but it will suffice
		guidSchema == DBSCHEMA_INDEXES ||			// This is really INDEX_NAME, but we always use table name
		guidSchema == DBSCHEMA_PRIMARY_KEYS ||
		guidSchema == DBSCHEMA_STATISTICS ||
		guidSchema == DBSCHEMA_TABLE_PRIVILEGES ||
		guidSchema == DBSCHEMA_TABLE_STATISTICS ||
		guidSchema == DBSCHEMA_TABLES ||
		guidSchema == DBSCHEMA_TABLES_INFO ||
		guidSchema == DBSCHEMA_VIEWS)
			return THIRD;
	else if	(guidSchema == DBSCHEMA_KEY_COLUMN_USAGE ||
			guidSchema == DBSCHEMA_TABLE_CONSTRAINTS)
		return SIXTH;

	// else return ZERO restrictions
	return ZERO;
}

void CSchemaTest::LimitRestrictions(ULONG ulIndex)
{

	// Don't pass restrictions for most schemas, but for those that have a catalog, schema, 
	// or table name limit to current catalog, schema, table.  This is done to keep the test
	// run time shorter. It took several hours on some providers without these restrictions.
	// Without a catalog restriction ALL catalogs (databases) should be returned.
	SetRestriction(SchemaCatalogRestriction(ulIndex));
	SetRestriction(SchemaSchemaRestriction(ulIndex));
	SetRestriction(SchemaTableRestriction(ulIndex));
}

// GetValuePtr
// Assumptions: All columns are bound
LPBYTE CSchemaTest::GetValuePtr(DBORDINAL iOrdinal, LPBYTE pData, DBBINDING * pBinding)
{
	LPBYTE pValue = NULL;
	DBORDINAL iBind = 0;
	
	// If no binding structure passed in assume user wants m_rgDBBINDING
	if (!pBinding)
		pBinding = m_rgDBBINDING;

	if (pBinding)
	{
		iBind = iOrdinal - !pBinding[0].iOrdinal - 1;

		if (STATUS_BINDING(pBinding[iBind], pData) == DBSTATUS_S_OK)
		{
			pValue = (LPBYTE)&VALUE_BINDING(pBinding[iBind], pData);
		}
	}

	return pValue;
}

// Compare histogram rowset columns info
BOOL CSchemaTest::CheckHistogramColInfo(IRowset * pIHistogramRowset, DBTYPE wRangeColType,
	DBORDINAL * pcBinding, DBBINDING ** ppBinding, DBLENGTH * pcbRowSize, HACCESSOR * phAccessor)
{
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBLENGTH		cbRowSize = 0;
	BOOL			fColInfo = FALSE;
	DBORDINAL		cActualCols = 0;	// Count of columns returned in histogram rowset

	// Save off member vars so we can call CheckAgainstIColumnsInfo()
	DBBINDING *		pBinding = m_rgDBBINDING;
	DBCOUNTITEM		cBinding = m_cDBBINDING;
	DBCOLUMNINFO *  pColInfo = m_rgDBCOLUMNINFO;
	DBORDINAL		cColInfo = m_cDBCOLUMNINFO;
	LPWSTR			pStringsBuf = m_pStringsBuffer;
	LPWSTR *		ppColNames = m_rgColumnNames;
	DBTYPE *		pColTypes = m_rgColumnTypes;
	ULONG			cCols = m_cColumns;

	m_cDBCOLUMNINFO = 0;
	m_rgDBCOLUMNINFO = NULL;
	m_pStringsBuffer = NULL;

	// Set up pointers to column name and type information
	m_rgColumnNames = (WCHAR **)rgwszHistogramCols;
	m_rgColumnTypes = (DBTYPE *)rgtypeHistogramCols;

	// Set the count of columns and restrictions
	m_cColumns = cHistogramCols;

	// Update expected type for RANGE_HI_KEY
	m_rgColumnTypes[0] = wRangeColType;

	// get bindings and column info
	if(!CHECK(GetAccessorAndBindings(
		pIHistogramRowset,							// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&hAccessor,									// @parmopt [OUT] Accessor created
		&m_rgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cDBBINDING,								// @parmopt [OUT] Count of bindings
		&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Array of DBCOLUMNINFO
		&m_cDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
		goto CLEANUP;

	fColInfo = CheckAgainstIColumnsInfo();

	// CheckAgainstIColumnsInfo doesn't test for any extra columns, so warn here
	// if there are extras
	cActualCols = m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal;

	// Check for extra columns in histogram rowset.  This is only a warning
	// since it's allowed to return extra columns.  Cache the warning so we don't 
	// get it for every histogram over and over.
	CCOMPARE(m_EC, cHistogramCols == cActualCols, EC_EXTRA_COLUMN,
		L"Extra column in Histogram rowset.", TRUE);

CLEANUP:

	// Populate output params
	if (pcBinding)
		*pcBinding = m_cDBCOLUMNINFO;
	if (ppBinding)
		*ppBinding = m_rgDBBINDING;
	else
		SAFE_FREE(m_rgDBBINDING);
	if (pcbRowSize)
		*pcbRowSize = cbRowSize;
	if (phAccessor)
		*phAccessor = hAccessor;

	SAFE_FREE(m_rgDBCOLUMNINFO);
	SAFE_FREE(m_pStringsBuffer);

	// Restore member vars to avoid side effects
	m_rgDBBINDING	= pBinding;
	m_cDBBINDING	= cBinding;
	m_rgDBCOLUMNINFO= pColInfo;
	m_cDBCOLUMNINFO	= cColInfo;
	m_pStringsBuffer= pStringsBuf;
	m_rgColumnNames	= ppColNames;
	m_rgColumnTypes	= pColTypes;
	m_cColumns		= cCols;

	return fColInfo;
}

DBCOUNTITEM CSchemaTest::Cardinality(LPWSTR pwszTableName, ULONG ulColIndex, CARDINALITY eCardinality,
	DBBINDING * pStartBind, DBBINDING * pEndBind, LPBYTE pDataRows, DBLENGTH cbRowSize)
{
	HRESULT hr						= S_OK;
	ULONG	ulIndexFirst			= 0;
	size_t	cChars					= 0;
	LPWSTR	pwszColList				= NULL;
	LPWSTR	pwszSelectFmt			= (LPWSTR)wszSELECT_DISTINCTCOLLIST;
	LPWSTR	pwszSelect				= NULL;
	LPWSTR	pwszGreaterFmt			= L"%s > ?";
	LPWSTR	pwszLessThanEqFmt		= L"%s <= ?";
	LPWSTR	pwszEqFmt				= L"%s = ?";
	LPWSTR	pwszRange				= NULL;
	DBCOUNTITEM	ulCardinality		= 0;
	IRowset * pIRowset				= NULL;
	IAccessor * pIAccessor			= NULL;
	ICommand * pICommand			= NULL;
	IAccessor * pICmdAccessor		= NULL;
	DBBINDING dbCountBind;
	DBBINDING dbParamBind[2];				// Only two params required for all cases
	ULONG	cParams					= 0;
	HACCESSOR hAcc					= DB_NULL_HACCESSOR;
	HACCESSOR hParamAcc				= DB_NULL_HACCESSOR;
	HROW * phRows					= NULL;
	ULONG * pulCardinality			= NULL;
	DBORDINAL iCol; 
	DBCOUNTITEM cRowsObtained;
	DBPARAMS dbParams;
	DBBINDSTATUS rgStatus[2];

	// Previously used the count aggregate function, but that was bad because:
	//		1) Doesn't include NULL rows
	//		2) Fails against GUID column on some providers.
		// select count(*) from (select distinct col1,col2 from <table> where col1 > ? and 
		// col1 <= ?) t1

	// Now we use just the select distinct, but this still will fail for BLOB columns,
	// so we have to avoid calling this function for BLOBS.
	// TODO:  If this function is made more sophisticated we can actually call for BLOBS.
	//		Possible improvements 1) Use conversion function, 2) actually retrieve data
	//		ourselves and count distinct values.
	// select distinct col1,col2 from <table> where col1 > ? and col1 <= ?

	// Initialize binding structures
	memset(&dbCountBind, 0, sizeof(DBBINDING));
	memset(&dbParamBind, 0, sizeof(DBBINDING)*2);

	// Initialize dbParams
	memset(&dbParams, 0, sizeof(DBPARAMS));

	// Note we use params for this to avoid having to convert range limits to strings and
	// obtain the appropriate literal prefix and suffix.  I am assuming that any provider
	// that has gone to the trouble to provide extensive support for query processors has
	// also supported parameters.  

	switch (eCardinality)
	{
		// Both range rows and eq rows are non-distinct, and all the cardinalities
		// for histogram are for only one column.  But they all need a 'where' clause
		case EQ_ROWS_CARDINALITY:
			// We don't have a range for this cardinality, we're only looking for 
			// values equal to RANGE_HIGH_KEY
			ASSERT(!pStartBind);
			// Fall through
		case RANGE_ROWS_CARDINALITY:
			// Both of these counts are non-distinct, so change our select
			// clause to not return distinct counts.
			pwszSelectFmt = (LPWSTR)wszSELECT_COLLISTFROMTBL;			
			// Fall through
		case DISTINCT_RANGE_ROWS_CARDINALITY:
		{
			size_t cchWhere = wcslen(wszWHERE); // Length of " where "
			size_t cchColName = wcslen(m_prgColInfo[ulColIndex].pwszName);
			LPWSTR	pwszFmt = pwszLessThanEqFmt;

			if (eCardinality == EQ_ROWS_CARDINALITY)
				pwszFmt = pwszEqFmt;

			// Fill out param info
			if (pStartBind)
			{
				cchWhere += wcslen(pwszGreaterFmt) -2 + cchColName + wcslen(wszAND);

				// Set parameter binding info
				memcpy(&dbParamBind[cParams], pStartBind, sizeof(DBBINDING));
				dbParamBind[cParams].eParamIO = DBPARAMIO_INPUT;
				dbParamBind[cParams].iOrdinal = cParams+1;

				cParams++;
			}
			if (pEndBind)
			{
				cchWhere += wcslen(pwszFmt) -2 + cchColName;

				// Set parameter binding info
				memcpy(&dbParamBind[cParams], pEndBind, sizeof(DBBINDING));
				dbParamBind[cParams].eParamIO = DBPARAMIO_INPUT;
				dbParamBind[cParams].iOrdinal = cParams+1;

				cParams++;
			}

			// Now we know all the pieces for the range clause, allocate memory and create
			SAFE_ALLOC(pwszRange, WCHAR, cchWhere+1);

			wcscpy(pwszRange, wszWHERE);

			// Put in the starting range clause
			if (pStartBind)
			{
				swprintf(pwszRange+wcslen(pwszRange), pwszGreaterFmt, m_prgColInfo[ulColIndex].pwszName);
				wcscat(pwszRange, wszAND);
			}

			// Put in ending range clause (always needed)
			swprintf(pwszRange+wcslen(pwszRange), pwszFmt, m_prgColInfo[ulColIndex].pwszName);

		}
			// Fall through
		case COLUMN_CARDINALITY:
			ulIndexFirst = ulColIndex;
			// Fall through
		case TUPLE_CARDINALITY:
			// Compute memory for column list (column name plus comma)
			for (iCol = 0; iCol < ulColIndex-ulIndexFirst+1; iCol++)
				cChars+=wcslen(m_prgColInfo[iCol+ulIndexFirst].pwszName)+1;

			// Allocate memory for col list
			SAFE_ALLOC(pwszColList, WCHAR, cChars+1);

			// Fill col list
			// Note we leave out BLOB cols because providers can't do a 
			// select distinct on a BLOB, which may have some impact on 
			// the cardinality results.
			pwszColList[0] = L'\0';
			for (iCol = 0; iCol < ulColIndex-ulIndexFirst+1; iCol++)
			{
				if (m_prgColInfo[iCol+ulIndexFirst].dwFlags & DBCOLUMNFLAGS_ISLONG)
					continue;

				// Put in comma separator if needed 
				if (pwszColList[0])
					wcscat(pwszColList, L",");
				wcscat(pwszColList, m_prgColInfo[iCol+ulIndexFirst].pwszName);
			}

			// Compute memory required for select distinct statement
			// Length of select, 2 parenthesis, table name, table alias name
			cChars+=wcslen(pwszSelectFmt)+wcslen(pwszTableName);

			// Add space for 'where' clause if needed
			if (pwszRange)
				cChars+=wcslen(pwszRange);

			// Allocate mem for select
			SAFE_ALLOC(pwszSelect, WCHAR, cChars+1);

			// select distinct col1,col2 from <table>
			swprintf(pwszSelect, pwszSelectFmt, pwszColList, pwszTableName);

			// Tack on where clause if needed
			if (pwszRange)
				wcscat(pwszSelect, pwszRange);

			// Free the range clause
			SAFE_FREE(pwszRange);

			break;
		case TABLE_CARDINALITY:
			pwszSelectFmt = (LPWSTR)wszSELECT_ALLFROMTBL;

			cChars+=wcslen(pwszSelectFmt)+wcslen(pwszTableName);

			// Allocate mem for select
			SAFE_ALLOC(pwszSelect, WCHAR, cChars+1);

			// Create select stmt
			swprintf(pwszSelect, pwszSelectFmt, pwszTableName);

			break;
	}

	// Execute the command
	if (cParams)
	{

		// Get the command object for the table object
		pICommand = m_pTable->get_ICommandPTR();

		// Addref the command object so we can release later.
		pICommand->AddRef();

		// Get accessor interface
		TESTC(VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&pICmdAccessor));

		// Create the parameter accessor
		TESTC_(pICmdAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cParams,
			dbParamBind, cbRowSize, &hParamAcc, rgStatus), S_OK);

		// Fill DBPARAMS info
		dbParams.pData = pDataRows;
		dbParams.cParamSets = 1;
		dbParams.hAccessor = hParamAcc;

	}

	if (!CHECK(m_pTable->BuildCommand(pwszSelect, IID_IRowset, 
		EXECUTE_IFNOERROR, 0, NULL, &dbParams, NULL, (IUnknown **)&pIRowset, &pICommand), S_OK))
		goto CLEANUP;

	// Get accessor and bindings
	dbCountBind.obStatus = offsetof(DATA, sStatus);
	dbCountBind.obLength = offsetof(DATA, ulLength);
	dbCountBind.obValue = offsetof(DATA, bValue);
	dbCountBind.wType = DBTYPE_UI4;
	dbCountBind.iOrdinal = 1;
	dbCountBind.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor));
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbCountBind, 0, &hAcc, NULL), S_OK);

	// Count the rows
	while(hr != DB_S_ENDOFROWSET)
	{
		TEST2C_(hr = pIRowset->GetNextRows(NULL, 0, 10000, &cRowsObtained, &phRows), S_OK, DB_S_ENDOFROWSET);

		if (hr == S_OK)
		{
			TESTC(cRowsObtained);
		}

		ulCardinality+=cRowsObtained;

		if (cRowsObtained)
		{
			CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
			*phRows = DB_NULL_HROW;
		}
	}

CLEANUP:

	// Release rows
	if (pIRowset && phRows && *phRows != DB_NULL_HROW)
		CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);

	// Release parameter accessor
	if (pICmdAccessor && hParamAcc != DB_NULL_HACCESSOR)
		CHECK(pICmdAccessor->ReleaseAccessor(hParamAcc, NULL), S_OK);

	SAFE_FREE(phRows);
	SAFE_FREE(pwszColList);
	SAFE_FREE(pwszSelect);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pICmdAccessor);

	return ulCardinality;

}

void CSchemaTest::SetRowCount(ROW_COUNT eRowCount, DBORDINAL ulRowCount)
{
	m_eRowCount = eRowCount;
	m_ulRowCount = ulRowCount;
}

HRESULT CSchemaTest::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRows)
{
	ASSERT(prghRows);
	TBEGIN
	
	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM i=0;
	
	//Record if we passed in consumer allocated array...
	HROW* rghRowsInput = *prghRows;

	//GetNextRows
	HRESULT hr = m_pIRowset->GetNextRows(NULL, lOffset, cRows, &cRowsObtained, prghRows);
	
	//Verify Correct values returned
	if(SUCCEEDED(hr))
	{
		if(hr == S_OK)
		{
			TESTC(cRowsObtained==(DBCOUNTITEM)ABS(cRows));
		}
		else
		{
			TESTC(cRowsObtained < (DBCOUNTITEM)ABS(cRows));
		}

		//Verify row array
		for(i=0; i<cRowsObtained; i++)
		{
			TESTC(*prghRows != NULL);
			TESTC((*prghRows)[i]!=DB_NULL_HROW)
		}
	}
	else
	{
		TESTC(cRowsObtained == 0);
	}

	//Verify output array, depending upon consumer or provider allocated...
	if(rghRowsInput)
	{
		//This is a users allocated static array,
		//This had better not be nulled out by the provider, if non-null on input
		TESTC(*prghRows == rghRowsInput);
	}
	else
	{
		TESTC(cRowsObtained ? *prghRows != NULL : *prghRows == NULL);
	}

CLEANUP:
	if(pcRowsObtained)
		*pcRowsObtained = cRowsObtained;
	return hr;
}


HRESULT CSchemaTest::GetColumnInfo(LPBYTE pData, DBBINDING * pBinding)
{
	IOpenRowset * pIOpenRowset = NULL;
	IColumnsInfo * pIColumnsInfo = NULL;
	DBID dbid = DB_NULLID;
	HRESULT hr = E_FAIL;
	ULONG iBind = 0, iFirst = 0;
	LPWSTR pwszCat = NULL;
	LPWSTR pwszSch = NULL;
	LPWSTR pwszTable = NULL;
	LPWSTR pwszQualifiedName = NULL;

	// pData buffer contains bindings to TABLE_CATALOG, TABLE_SCHEMA, and TABLE_NAME, in that 
	// order.
	
	// Skip any bookmark
	if (!pBinding[iBind].iOrdinal)
		iFirst++;

	for (iBind = 0; iBind < 3; iBind++)
	{
		// If value isn't bound then we assume NULL
		if (!VALUE_IS_BOUND(pBinding[iBind+iFirst]))
			continue;

		// Get a pointer to catalog, schema, and table name
		if (STATUS_IS_BOUND(pBinding[iBind+iFirst]))
		{
			// Check the status, which should always be OK or NULL
			if (STATUS_BINDING(pBinding[iBind+iFirst], pData) != DBSTATUS_S_OK)
			{
				COMPARE(STATUS_BINDING(pBinding[iBind+iFirst], pData), DBSTATUS_S_ISNULL);
				continue;
			}
		}

		// If status wasn't bound we assume OK, and length doesn't matter for null terminated string
		switch(pBinding[iBind+iFirst].iOrdinal)
		{
			case 1:
				pwszCat = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			case 2:
				pwszSch = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			case 3:
				pwszTable = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			default:
				COMPARE(pBinding[iBind+iFirst].iOrdinal < 4, TRUE);
				COMPARE(pBinding[iBind+iFirst].iOrdinal > 0, TRUE);
		}
	}

	// HACK FOR ORACLE
	if (pwszSch && wcslen(pwszSch) == 0)
		pwszSch = NULL;

	// Now we have either NULL or valid pointers to catalog, schema, table names
	TESTC_(m_pTable->GetQualifiedName(pwszCat, pwszSch, 
							pwszTable,&pwszQualifiedName, TRUE), S_OK);

	// If we already have information for this table then just don't bother
	if (m_pwszTableName && !wcscmp(pwszQualifiedName, m_pwszTableName))
	{
		m_ulTableOrdinal++;

		// If we couldn't actually retrieve the colinfo for this table then
		// return S_FALSE
		if (!m_prgColInfo)
			return S_FALSE;
		// Otherwise return success
		else
			return S_OK;
	}

	// Copy the table name into member var
	SAFE_FREE(m_pwszTableName);
	m_pwszTableName = pwszQualifiedName; 

	// Initialize out params
	m_ulTableOrdinal = 1;	// Note that schema rowset never reports bookmarks
	SAFE_FREE(m_prgColInfo);
	SAFE_FREE(m_pwszStringsBuffer);

	// Get IOpenRowset interface
	TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset));

	// Create DBID from table name
	dbid.eKind = DBKIND_NAME;
	dbid.uName.pwszName = m_pwszTableName;

	// Call OpenRowset
	hr = pIOpenRowset->OpenRowset(NULL, &dbid, NULL, IID_IColumnsInfo, 
		0, NULL, (IUnknown **)&pIColumnsInfo);

	if (FAILED(hr))
		odtLog << m_pwszTableName << L": Unable to open table.\n";

	// It's possible the table was deleted between the time we called GetRowset and attempting
	// to open the table, so we have to allow DB_E_NOTABLE, but post a warning.
	switch(hr)
	{
		case DB_E_NOTABLE:
		case DB_SEC_E_PERMISSIONDENIED:
			TESTW_(hr, S_OK);
			goto CLEANUP;
		default: 
			TESTC_(hr, S_OK);
	}

	// Make sure we've got a valid interface ptr
	TESTC(pIColumnsInfo != NULL);

	// Now call GetColumnInfo.  We already know cColumns so no need to return in member
	// var.
	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&m_cColInfo, &m_prgColInfo, &m_pwszStringsBuffer), S_OK);

CLEANUP:

	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIColumnsInfo);

	return hr;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x4ace5741, 0x13bb, 0x11cf, { 0x89, 0x0f, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("IDBSchemaRowset");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("test module for IDBSchemaRowset");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------------
// @func Initialises up indexes, Primary and Foreign keys on the primary table.
//	Should call TerminateKeysOnTable() upon failure of InitKeysOnTable or completion of test.
//
// Global values set.   g_fPrimaryKey  (TRUE if Primary key is available on the table).
//						g_fForeignKey  (TRUE if Foreign key is available on the table).
//						g_fIndexes	   (TRUE if indexes are available on the table.)
//						g_pExtraTable	Creates it.
//	
BOOL CSchemaTest::InitKeysOnTable()
{
	CCol	TempCol1, TempCol2;
	LONG	lrand1 =0;
	LONG	lrand2 =0;
	LONG	lrand3 =0;
	ULONG		i;
	BOOL	fSuccess = FALSE;
	ICommand * pICmd = NULL;
	IDBCreateCommand * pIDBCrtCmd = (IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2;

	// Create the second table.
	g_pKeyTable1 = new CTable(pIDBCrtCmd, (LPWSTR)gwszModuleName, NONULLS);

	if (!g_pKeyTable1)
	{
		odtLog << wszMemoryAllocationError;
		return FALSE;
	}
	
	g_pKeyTable2 = new CTable(pIDBCrtCmd, (LPWSTR)gwszModuleName, NONULLS);

	if (!g_pKeyTable2)
	{
		odtLog << wszMemoryAllocationError;
		// g_pKeyTable1 will be cleaned up in TerminateKeysOnTable();
		return FALSE;
	}

	if (FAILED(g_pKeyTable2->CreateTable(0)))
		goto CLEANUP;
	
	if (FAILED(g_pKeyTable1->CreateTable(0)))
		goto CLEANUP;
	
	// Now alter the tables to create Primary keys and foreign keys.
	for (i = 1; i <= g_pKeyTable1->CountColumnsOnTable(); i++)
	{
		if (FAILED(g_pKeyTable1->GetColInfo(i, TempCol1)))
			goto CLEANUP;

		// if it is  not a long column and is fixed length We found our candidate for Primary and Foreign Key.
		if (!TempCol1.GetIsLong() &&
			!TempCol1.GetNullable())		// Primary keys typically can't be nullable columns
		{
			fSuccess = TRUE;
			break;
		}
	
	}
	if (! fSuccess )
		goto CLEANUP;

	fSuccess = FALSE;
	for (i = 1; i <= g_pKeyTable2->CountColumnsOnTable(); i++)
	{
		if (FAILED(g_pKeyTable2->GetColInfo(i, TempCol2)))
			goto CLEANUP;

		// if it is  not a long column and is fixed length We found our candidate for Primary and Foreign Key.
		if (// (TempCol2.GetProviderType() == DBTYPE_STR) && 
			!(TempCol2.GetIsLong()) &&
			!TempCol2.GetNullable())		// Primary keys typically can't be nullable columns
		{
			fSuccess = TRUE;
			break;
		}
	
	}
	if (! fSuccess )
		goto CLEANUP;

	// Now alloc space for the  strings.
	g_pwszAddPrimaryKeyOnTable1 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszAddPrimaryKey)*sizeof (WCHAR)) + sizeof (WCHAR)	
													+	(wcslen(g_pKeyTable1->GetTableName())*sizeof (WCHAR))
													+	2 * (wcslen(TempCol1.GetColName())*sizeof (WCHAR))
													+	20	// Extra space for the KeyNumber (Random number).
													);
	if (!g_pwszAddPrimaryKeyOnTable1)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	g_pwszAddPrimaryKeyOnTable2 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszAddPrimaryKey)*sizeof (WCHAR)) + sizeof (WCHAR)	
													+	(wcslen(g_pKeyTable2->GetTableName())*sizeof (WCHAR))
													+	2 * (wcslen(TempCol2.GetColName())*sizeof (WCHAR))
													+	20	// Extra space for the KeyNumber (Random number).
													);
	if (!g_pwszAddPrimaryKeyOnTable2)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	
	g_pwszAddForeignKeyOnTable1 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszAddForeignKey)*sizeof (WCHAR)) + sizeof (WCHAR)	
													+	(wcslen(g_pKeyTable1->GetTableName())*sizeof (WCHAR))
													+	2 * (wcslen(TempCol1.GetColName())*sizeof (WCHAR))
													+	(wcslen(g_pKeyTable1->GetTableName())*sizeof (WCHAR))
													+	(wcslen(TempCol1.GetColName())*sizeof (WCHAR))
													+	20	// Extra space for the KeyNumber (Random number).
													);
	if (!g_pwszAddForeignKeyOnTable1)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	
	g_pwszDropPrimaryKeyConstraint1 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszDropPrimaryKeyConstraint)*sizeof (WCHAR)) + sizeof (WCHAR)	
														+	(wcslen(g_pKeyTable1->GetTableName())*sizeof (WCHAR))
														+	(wcslen(TempCol1.GetColName())*sizeof (WCHAR))
														+	20	// Extra space for the KeyNumber (Random number).
														);
	if (!g_pwszDropPrimaryKeyConstraint1)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	g_pwszDropPrimaryKeyConstraint2 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszDropPrimaryKeyConstraint)*sizeof (WCHAR)) + sizeof (WCHAR)	
														+	(wcslen(g_pKeyTable2->GetTableName())*sizeof (WCHAR))
														+	(wcslen(TempCol2.GetColName())*sizeof (WCHAR))
														+	20	// Extra space for the KeyNumber (Random number).
														);
	if (!g_pwszDropPrimaryKeyConstraint2)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	g_pwszDropForeignKeyConstraint1 = (WCHAR *)PROVIDER_ALLOC(	(wcslen(g_wszDropForeignKeyConstraint)*sizeof (WCHAR)) + sizeof (WCHAR)	
														+	(wcslen(g_pKeyTable1->GetTableName())*sizeof (WCHAR))
														+	(wcslen(TempCol1.GetColName())*sizeof (WCHAR))
														+	20	// Extra space for the KeyNumber (Random number).
														);
	if (!g_pwszDropForeignKeyConstraint1)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	// Add primary key on Table 1.
	swprintf(g_pwszAddPrimaryKeyOnTable1, g_wszAddPrimaryKey, 
		g_pKeyTable1->GetTableName(), (lrand1 = GetLongRandomNumber()), TempCol1.GetColName(), TempCol1.GetColName());

	// Add primary key on table 2.
	swprintf(g_pwszAddPrimaryKeyOnTable2, g_wszAddPrimaryKey, 
		g_pKeyTable2->GetTableName(), (lrand2 = GetLongRandomNumber()), TempCol2.GetColName(), TempCol2.GetColName());

	// Add foriegn key on table 1.  (from table 2).
	swprintf(g_pwszAddForeignKeyOnTable1, g_wszAddForeignKey, 
		g_pKeyTable1->GetTableName(), (lrand3 = GetLongRandomNumber()), TempCol1.GetColName(), TempCol1.GetColName(),
		g_pKeyTable2->GetTableName(), TempCol2.GetColName() );

	// Drop primary key on table 1.
	swprintf(g_pwszDropPrimaryKeyConstraint1, g_wszDropPrimaryKeyConstraint, 
		g_pKeyTable1->GetTableName(), lrand1, TempCol1.GetColName());
	
	// Drop primary key on table 2.
	swprintf(g_pwszDropPrimaryKeyConstraint2, g_wszDropPrimaryKeyConstraint, 
		g_pKeyTable2->GetTableName(), lrand2, TempCol2.GetColName());
	
	// Drop the foreign key on table 1.
	swprintf(g_pwszDropForeignKeyConstraint1, g_wszDropForeignKeyConstraint, 
		g_pKeyTable1->GetTableName(), lrand3, TempCol1.GetColName());

	pICmd = g_pKeyTable1->get_ICommandPTR();

	// If we don't already have a primary key on the table
	if (!g_pKeyTable1->GetPrimaryKeyColumn())
	{

		if( !CHECK(g_pKeyTable1->BuildCommand(g_pwszAddPrimaryKeyOnTable1,		// SQL STMT
				IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK) )
			goto CLEANUP;
	
		// Set m_fKeyOnTable to Success so that we can drop them.(even if One constraint goes through).
		g_fKeysOnTable = TRUE;
	}
	else
		m_fPrimaryKey = TRUE; // There should be a minimum of one row in PRIMARY_KEYS rowset

	pICmd = g_pKeyTable2->get_ICommandPTR();

	// If we don't already have a primary key on the table
	if (!g_pKeyTable2->GetPrimaryKeyColumn())
	{
		TESTC_(g_pKeyTable2->BuildCommand(g_pwszAddPrimaryKeyOnTable2,		// SQL STMT
				IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK);
	}

	m_fPrimaryKey = TRUE; // There should be a minimum of one row in PRIMARY_KEYS rowset

	pICmd = g_pKeyTable2->get_ICommandPTR();
	TESTC_(g_pKeyTable1->BuildCommand(g_pwszAddForeignKeyOnTable1,		// SQL STMT
			IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK);

	m_fForeignKey = TRUE; // There should be a minimum of one row in FOREIGN_KEYS rowset

	// Now we add rows.
	// First add rows in 2nd Table and then on 1st table.
	// Start with a table with 1 rows	
	if (FAILED(g_pKeyTable2->Insert(1)))
		goto CLEANUP;
	
	if (FAILED(g_pKeyTable1->Insert(1)))
		goto CLEANUP;
	
	fSuccess = TRUE;

CLEANUP:

	if (!fSuccess)
	{
		// CLEANUP memory incase of failure.
		PROVIDER_FREE(g_pwszAddPrimaryKeyOnTable1);
		PROVIDER_FREE(g_pwszAddPrimaryKeyOnTable2);
		PROVIDER_FREE(g_pwszAddForeignKeyOnTable1);
		PROVIDER_FREE(g_pwszDropPrimaryKeyConstraint1);
		PROVIDER_FREE(g_pwszDropPrimaryKeyConstraint2);
		PROVIDER_FREE(g_pwszDropForeignKeyConstraint1);
	}

	return fSuccess;

}


//--------------------------------------------------------------------
// @func Drops Key constraintes, indexes etc, created in InitKeysOnTable()
//	Drops the extra table after dropping the constraints.
BOOL CSchemaTest::TerminateKeysOnTable()
{
	// Drop the constraints.
	ICommand * pICmd = NULL;

	if (g_fKeysOnTable)
	{
		// We still need to flag an error if the Drop constraint fails.
		pICmd = g_pKeyTable1->get_ICommandPTR();

		CHECK(g_pKeyTable1->BuildCommand(g_pwszDropPrimaryKeyConstraint1,		// SQL STMT
			IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK);
		
		CHECK(g_pKeyTable1->BuildCommand(g_pwszDropForeignKeyConstraint1,		// SQL STMT
			IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK);
		
		CHECK(g_pKeyTable2->BuildCommand(g_pwszDropPrimaryKeyConstraint2,		// SQL STMT
			IID_NULL, EXECUTE_ALWAYS, NULL, NULL, NULL,	NULL, NULL,	&pICmd), S_OK);
	}

	if (g_pKeyTable1)
	{
		// remove table from database
		(g_pKeyTable1)->DropTable();

		// delete CTable object
		delete g_pKeyTable1;

		g_pKeyTable1 = NULL;
	}
	
	if (g_pKeyTable2)
	{
		// remove table from database
		(g_pKeyTable2)->DropTable();

		// delete CTable object
		delete g_pKeyTable2;

		g_pKeyTable2 = NULL;
	}

	// CLEANUP memory incase of failure.
	PROVIDER_FREE(g_pwszAddPrimaryKeyOnTable1);
	PROVIDER_FREE(g_pwszAddPrimaryKeyOnTable2);
	PROVIDER_FREE(g_pwszAddForeignKeyOnTable1);
	PROVIDER_FREE(g_pwszDropPrimaryKeyConstraint1);
	PROVIDER_FREE(g_pwszDropPrimaryKeyConstraint2);
	PROVIDER_FREE(g_pwszDropForeignKeyConstraint1);

	return TRUE;
}

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	HRESULT	hr;

	// Need to initialize these in ModuleInit to make sure they get reset if test is
	// run against two providers.
	g_fKeysOnTable=FALSE;
	g_pIDBSchemaRowset=NULL;
	g_fKagera = FALSE;
	g_fSQLServer = FALSE;

	cSchemas=0;
	rgSchemas=NULL;
	rgRestrictions=NULL;

	// Get connection and session objects
	if (ModuleCreateDBSession(pThisTestModule))
	{
		LPWSTR	pwszName=NULL;
		g_fKagera = IsMSDASQL();

		// Needed for ExtraLib
		g_pIDBInitialize = (IDBInitialize *)pThisTestModule->m_pIUnknown;

		// Fail gracefully and quit module if we don't support IDBSchemaRowset
		if (FAILED(hr = pThisTestModule->m_pIUnknown2->QueryInterface(
			IID_IDBSchemaRowset,(void **)&g_pIDBSchemaRowset)))
		{
			// Make sure we returned E_NOINTERFACE if we've failed
			if (pThisTestModule->m_pError->Validate(hr,	
								LONGSTRING(__FILE__), __LINE__, E_NOINTERFACE))
				odtLog << L"IDBSchemaRowset is not supported.\n";

			return TEST_SKIPPED;
		}

		// Get the name of the backend
		GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown, &pwszName);
		if (pwszName)
		{
			if (!wcscmp(pwszName, L"Microsoft SQL Server"))
				g_fSQLServer = TRUE;
		}
		SAFE_FREE(pwszName);

		// Just Print out What is Supported
		if (FAILED(g_pIDBSchemaRowset->GetSchemas(&cSchemas, 
												  &rgSchemas, &rgRestrictions)))
		{
			SAFE_RELEASE(g_pIDBSchemaRowset);
			return FALSE;
		}
		
		for(ULONG index=0; index<cSchemas; index++)
		{
			LPWSTR pwszSchemaName = L"Unknown schema";
//			TraceSchemaName(rgSchemas[index],TRUE,TRUE);
			for (ULONG iSchema=0; iSchema < NUMELEM(AllSchemas); iSchema++)
			{
				if (rgSchemas[index] == *AllSchemas[iSchema].pguid)
					pwszSchemaName = AllSchemas[iSchema].pwszName;
			}
			odtLog << pwszSchemaName << L"\n";
			TraceRestrictions(rgRestrictions,index,NULL,FALSE);
		}
		
		// Create a table we'll use for the whole test module,
		pThisTestModule->m_pVoid = new CTable(
			(IUnknown *)pThisTestModule->m_pIUnknown2,(LPWSTR)gwszModuleName);

		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		// Start with a table with 30 rows								 
		if (!CHECK(((CTable *)pThisTestModule->m_pVoid)->CreateTable(30), S_OK))
			return FALSE;
	
		return TRUE;
	}
	
	return FALSE;
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	// We still own the table since all of our testcases
	// have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{
		// Remove table from database
		((CTable *)pThisTestModule->m_pVoid)->DropTable();

		// Delete CTable object
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	SAFE_RELEASE(g_pIDBSchemaRowset);
	SAFE_FREE(rgSchemas);
	SAFE_FREE(rgRestrictions);

	return ModuleReleaseDBSession(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Helper classes
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CHelper
{

protected:

	CTable * m_pTable;

	GUID m_guidSchema;

	SchemaList * m_pSchemaInfo;

	CSchemaTest * m_pCSchemaTest;

	BOOL m_fPassUnsupportedRestrictions;

	BOOL m_fUseCommonArgumentDefaults;

	BOOL m_fSupported;	// Is this schema supported?

	ULONG  m_ulRestrictionSupport;

	DBORDINAL m_ulRowCount;

	ROW_COUNT m_eRowCount;
	
	LPWSTR m_pwszCatalogRestriction;

	LPWSTR m_pwszSchemaName;

	ULONG_PTR m_ulNullCollation;

	DBCOUNTITEM	m_cTotalRows;

	// @cmember Count of columns in rowset
	DBORDINAL m_cDBCOLUMNINFO;		

	// @cmember Array of column information in rowset
	DBCOLUMNINFO *	m_rgDBCOLUMNINFO;
	
	// @cmember String buffer for column info
	LPWSTR m_pStringsBuffer;

	// @cmember Array of column information in rowset
	DBCOLUMNINFO *	m_rgDBCOLUMNINFO_Prev;
	
	// @cmember String buffer for column info
	LPWSTR m_pStringsBuffer_Prev;

	// @cmember Count of columns in rowset
	DBCOUNTITEM m_cDBCOLUMNINFO_Prev;

	// @cmember Count of Bindings
	DBCOUNTITEM	m_cDBBINDING;			

	// @cmember Array of Bindings
	DBBINDING *	m_rgDBBINDING;

	// Bitmask of which restrictions are requested
	RESTRICTIONS	m_restrict;

	// @cmember Count of restrictions
	ULONG	m_cRestrict;			

	// All cached restrictions
	VARIANT m_rgRestrict[MAXRESTRICTION];

	// Valid restrictions for this schema
	VARIANT m_rgRestrictValid[MAXRESTRICTION];

	// Restrictions for this schema
	VARIANT m_rgRestrictUsed[MAXRESTRICTION];

	// Track actual restriction array used
	VARIANT * m_prgRestrictUsed;
	ULONG m_cRestrictionsUsed;

	// @cmember Rowset pointer
	IRowset *	m_pIRowset;				

	// Do I want to capture restrictions only
	BOOL m_fCaptureRestrictions;
	BOOL m_fCaptured;

	// Pointer to current row of schema
	LPBYTE m_pRow;

	// Pointer to previous row of schema used to verify sorting/ordinals
	LPBYTE m_pRowPrev;

	// IDBSchemaRowset interface pointer to use
	IDBSchemaRowset * m_pIDBSchemaRowset;

	// Row to use for restrictions
	DBCOUNTITEM m_ulRestrictRow;

	// Column ordinal to use for restriction
	ULONG m_iRestrict;

	enum RESTRICT_ARRAY m_eRestrictArray;

	LPWSTR	m_pwszTableName;
	ULONG	m_ulTableOrdinal;
	DBCOLUMNINFO * m_prgColInfo;
	LPWSTR	m_pwszStringsBuffer;
	DBORDINAL m_cColInfo;

	CErrorCache		m_ECHelper;	// Error cache

	ULONG m_ulSchemaVersion;
	ULONG m_ulOLEDBVer;

	// Helper functions
	HRESULT GetColumnInfo(LPBYTE pData, DBBINDING * pBinding);
	BOOL FillRestrictions(DBCOUNTITEM iRow, LPBYTE pData);
	LPWSTR GetRestrictionName(ULONG iRestrict);
	RESTRICT_CAT GetRestrictCat(ULONG iRestrict);
	ULONG GetLiteralMaxSize(DBLITERAL eLiteral);

	// Function to create at least two rows in schema, so sorting test will actually test something.
	// It's possible this function may just return TRUE if we know the schema already has many rows.
	virtual BOOL InsertRows(void) {return TRUE;}

	// Function to pick a row from the schema and use values from that row for restrictions
	BOOL GetRestrictions(ULONG iRestrict);
	BOOL IsRestricted(ULONG iRestrict);

	// Row verification functions
	BOOL CheckAgainstIColumnsInfo(SchemaList * pSchemaInfo);
	BOOL VerifyRowset(IRowset * pIRowset);
	LPBYTE GetValuePtr(DBORDINAL iOrdinal, LPBYTE pData, DBBINDING * pBinding = NULL);
	void GetVersionInfo(IDBInitialize * pIDBInit);
	BOOL VerifyRowCommon(DBCOUNTITEM iRow, BYTE * pData);
	BOOL VerifySort(DBCOUNTITEM iRow, BYTE * pData);
	BOOL VerifyValidTable(DBCOUNTITEM iRow, LPBYTE pData);
	BOOL VerifyColumnCommon(DBORDINAL iOrdinal, DATA * pColumn);
	virtual BOOL VerifyRestriction(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL VerifyLength(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL VerifyTypeSpecific(DBORDINAL iOrdinal, DATA * pColumn);
	virtual BOOL VerifyValue(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL VerifyNull(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL VerifyNonEmptyString(DATA * pColumn, LPWSTR pwszTable, LPWSTR pwszColumn);
	BOOL VerifyColumnMatch(DBORDINAL iOrdinal1, DBORDINAL iOrdinal2);

	// Schema-specific validation
	virtual BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn) {return TRUE;}
	virtual BOOL CompareNull(DBORDINAL iOrdinal)
	{
		if (!COMPARE(m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull, TRUE))
		{
			odtLog << L"Column " << m_pSchemaInfo->pColList[iOrdinal-1].pwszName << L" was NULL.\n";
			return FALSE;
		}

		return TRUE;
	}

public:

	CHelper(void);
	virtual ~CHelper(void);
	virtual BOOL Init(CSchemaTest * pThis);
	virtual BOOL CreateCheckConstraints(void);
	virtual BOOL CreateStoredProcs(void);
	virtual LPWSTR CreateObject(UINT idsObject, LPWSTR pwszObjectName = NULL);
	virtual BOOL VerifyObject(UINT idsObject);
	virtual BOOL VerifyObjectValue(UINT idsObject, ULONG iOrdinal);

	BOOL CheckResults(HRESULT hr, REFIID iid, IUnknown * pIUnknown);
	HRESULT GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRows);

	BOOL IsSupported(void);
	void UseArgumentDefaults(BOOL fUseDefaults) {m_fUseCommonArgumentDefaults = fUseDefaults;}
	GUID GetSchema(void) {return m_guidSchema;}
	LPWSTR GetCatalogName();

	HRESULT GetRowsetHelper
	(
		IUnknown *		pIUnknown = NULL,
		ULONG			cRestrictions = RV_ALL,
		VARIANT	*		prgRestrictions = NULL,
		REFIID			riid = IID_IRowset,
		ULONG			cPropertySets = 0,
		DBPROPSET		rgPropertySets[] = NULL,
		IUnknown **		ppRowset = NULL,
		GUID			guidSchema = GUID_NULL,
		BOOL fFreeRowset = TRUE
	);

	BOOL HasUnsupportedRestriction(void);
	BOOL IsSupportedRestriction(ULONG iRestrict);
	void ClearRestrictions(VARIANT * prgVariant = NULL);
	BOOL GetValidColumnValues(DBORDINAL iOrdinal, ULONG * pcValidValues, LPVOID * ppValidValues);
	BOOL SetValidColumnValues(DBORDINAL iOrdinal, ULONG cValidValues, LPVOID pValidValues);
	void SetValidRestriction(ULONG iRestrict);
	void SetEmptyRestrict(ULONG iRestrict);
	void SetRestriction(ULONG iRestrict);
	BOOL SetRestriction(ULONG iRestrict, LPVOID pValue);
	BOOL SetRestriction(ULONG iRestrict, LPVOID pValue, DBTYPE wType);
	BOOL SetRestriction(ULONG iRestrict, VARIANT * pVariant);
	BOOL SetInvalidRestriction(ULONG iRestrict, LPVOID pValue, DBTYPE wType);
	void SetRestrictionType(ULONG iRestrict, DBTYPE wType);
	ULONG RestrictionCount(void) {return m_pSchemaInfo->cRestrictions;}
	void SetRestrictionArray(enum RESTRICT_ARRAY eRestrictArray) {m_eRestrictArray = eRestrictArray;}
	VARIANT * GetRestrictPtr(ULONG iRestrict);
	ULONG GetRestrictionMaxLength(ULONG iRestrict);

	void SetRowCount(ROW_COUNT eRowCount, DBORDINAL ulRowCount);

	int TestRestriction(ULONG iRestrict);

	// Obsolete functions
	void SetRestrictions(ULONG ulRestrict) {m_restrict = ulRestrict;}

};

CHelper::CHelper(void)
{
	m_pTable =	NULL;
	m_guidSchema = GUID_NULL;
	m_pSchemaInfo = NULL;
	m_fPassUnsupportedRestrictions = FALSE;
	m_fUseCommonArgumentDefaults = TRUE;
	m_fSupported = FALSE;
	m_ulRestrictionSupport = 0;
	m_ulRowCount = 1;	// By default we will require at least one row back from schema
	m_pwszCatalogRestriction = NULL;
	m_pwszSchemaName = NULL;
	m_ulNullCollation = DBPROPVAL_NC_HIGH;
	m_cTotalRows = 0;
	m_eRowCount = MIN_REQUIRED;
	m_cDBBINDING = 0;
	m_cDBCOLUMNINFO = 0;
	m_cRestrict = 0;
	m_fCaptureRestrictions = FALSE;
	m_iRestrict = 0;
	m_pIDBSchemaRowset = NULL;
	m_ulRestrictRow = 2;
	m_prgRestrictUsed = NULL;
	m_cRestrictionsUsed = 0;
	m_pRow = NULL;
	m_pRowPrev = NULL;
	m_pIRowset = NULL;
	m_rgDBBINDING = NULL;
	m_rgDBCOLUMNINFO = NULL;
	m_pStringsBuffer = NULL;
	m_eRestrictArray = RT_SUPPORTED;
	m_rgDBCOLUMNINFO_Prev = NULL;
	m_pStringsBuffer_Prev = NULL;
	m_cDBCOLUMNINFO_Prev = 0;

	// Information specific to a given table in a schema
	m_pwszTableName = NULL;
	m_ulTableOrdinal = 0;
	m_prgColInfo = NULL;
	m_pwszStringsBuffer = NULL;
	m_cColInfo = 0;

	// Init all restrictions variants
	for (ULONG iRestrict = 0; iRestrict < MAXRESTRICTION; iRestrict++)
	{
		VariantInit(&m_rgRestrict[iRestrict]);
		VariantInit(&m_rgRestrictValid[iRestrict]);
		VariantInit(&m_rgRestrictUsed[iRestrict]);
	}
}

CHelper::~CHelper(void)
{
	ClearRestrictions(m_rgRestrict);
	ClearRestrictions(m_rgRestrictValid);
	ClearRestrictions(m_rgRestrictUsed);
	SAFE_FREE(m_rgDBBINDING);
	SAFE_FREE(m_rgDBCOLUMNINFO);
	SAFE_FREE(m_pStringsBuffer);
	SAFE_RELEASE(m_pIDBSchemaRowset);
	SAFE_RELEASE(m_pIRowset);
	SAFE_FREE(m_pwszTableName);
	SAFE_FREE(m_prgColInfo);
	SAFE_FREE(m_pwszStringsBuffer);
	SAFE_FREE(m_rgDBCOLUMNINFO_Prev);
	SAFE_FREE(m_pStringsBuffer_Prev);
	SAFE_FREE(m_pwszCatalogRestriction);
	SAFE_FREE(m_pwszSchemaName);
}

BOOL CHelper::Init(CSchemaTest * pThis)
{
	ULONG cSchemas = 0;
	WCHAR * pwszOLEDBVER = NULL;
	GUID * rgSchemas = NULL;
	ULONG * rgRestrictionSupport = NULL;
	BOOL fFoundSchema = FALSE;
	ULONG iSchema;
	BOOL fResult = FALSE;
	IDBInitialize * pIDBInit = NULL; 

	TESTC(pThis != NULL)

	pIDBInit = pThis->m_pIDBInitialize;

	m_ECHelper.Init(GetModInfo()->GetDebugMode());

	// CHelper is a virtual (not pure) base class and m_guidSchema is populated
	// in the derived class with the proper schema, so don't allow users to
	// Init without a valid schema.
	TESTC(m_guidSchema != GUID_NULL)

	m_pCSchemaTest = pThis;
	m_pTable = pThis->m_pTable;
	m_pIDBSchemaRowset = pThis->m_pIDBSchemaRowset;
	m_pIDBSchemaRowset->AddRef();

	// Locate this schema in the global schema information array
	for (iSchema = 0; iSchema < NUMELEM(AllSchemas); iSchema++)
	{
		if (*AllSchemas[iSchema].pguid == m_guidSchema)
		{
			m_pSchemaInfo = &AllSchemas[iSchema];
			break;
		}
	}

	TESTC(m_pSchemaInfo != NULL)

	// Get Restriction support for this schema
	m_fSupported = FALSE;

	TESTC_(m_pIDBSchemaRowset->GetSchemas(&cSchemas, &rgSchemas, &rgRestrictionSupport), S_OK)

	for(iSchema=0; iSchema<cSchemas; iSchema++)
	{
		if(m_guidSchema == rgSchemas[iSchema])
		{
			m_fSupported = TRUE;
			m_ulRestrictionSupport = rgRestrictionSupport[iSchema];
			break;
		}
	}

	// Insert any rows into this schema that are needed if schema is supported.
	if (m_fSupported)
	{
		TESTC(InsertRows());
	}

	TESTC(GetRestrictions(0));

	// Call the GetProperty for the CurrentCatalog
	GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pIDBInit, &m_pwszCatalogRestriction);

	// Call the GetProperty for the UserName, which matches the schema name
	GetProperty(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO, pIDBInit, &m_pwszSchemaName);

	// Get DBPROP_NULLCOLLATION for use in comparing sort of NULL values
	GetProperty(DBPROP_NULLCOLLATION, DBPROPSET_DATASOURCEINFO, pIDBInit, &m_ulNullCollation);

	// Figure out what schema version and OLEDB version we're using for verification
	GetVersionInfo(pIDBInit);

	// Update count of columns if needed.  
	if (m_pSchemaInfo->pulColCount)
	{
		m_pSchemaInfo->cColumns = m_pSchemaInfo->pulColCount[0];
	}

	fResult = TRUE;

CLEANUP:

	SAFE_FREE(rgSchemas);
	SAFE_FREE(rgRestrictionSupport);

	return fResult;
}

// Default implementation does nothing
LPWSTR CHelper::CreateObject(UINT idsObject, LPWSTR pwszObjectName) {return NULL;}
BOOL CHelper::VerifyObject(UINT idsObject) {return TRUE;}
BOOL CHelper::VerifyObjectValue(UINT idsObject, ULONG iOrdinal) {return TRUE;}

// Determine the schema version and OLEDB version
void CHelper::GetVersionInfo(IDBInitialize * pIDBInit)
{
	LPWSTR pwszOLEDBVER = NULL;

	// Get DBPROP_PROVIDEROLEDBVER for use in setting correct column count for each schema
	if (GetProperty(DBPROP_PROVIDEROLEDBVER, DBPROPSET_DATASOURCEINFO, pIDBInit, &pwszOLEDBVER))
	{	
		if (!wcscmp(pwszOLEDBVER, L"02.00"))
		{
			m_ulSchemaVersion = VER_20;			
			m_ulOLEDBVer = VER_20;
		}
		else if (!wcscmp(pwszOLEDBVER, L"02.10"))
		{
			m_ulSchemaVersion = VER_21;			
			m_ulOLEDBVer = VER_21;
		}
		else if (!wcscmp(pwszOLEDBVER, L"02.50"))
		{
			m_ulSchemaVersion = VER_25;			
			m_ulOLEDBVer = VER_25;
		}
		else if (!wcscmp(pwszOLEDBVER, L"02.60"))
		{
			m_ulSchemaVersion = VER_26;			
			m_ulOLEDBVer = VER_26;
		}
		else if (!wcscmp(pwszOLEDBVER, L"02.70"))
		{
			m_ulSchemaVersion = VER_27;			
			m_ulOLEDBVer = VER_27;
		}
		else
		{
			odtLog << L"Unexpected provider OLEDB version.\n";
			COMPARE(0,1);
		}

		SAFE_FREE(pwszOLEDBVER);
	}
}

BOOL CHelper::CreateCheckConstraints(void)
{

	// Create at least two check constraints that will not impact other test variations
	// using the test's default table.
	
	// alter table slh add constraint slhconstraint check (col1 >=  'bbb' or col1 < 'bbb')
	// %1 tablename, %2 constraintname, %3 columnname, %4 constraintvalue

	BOOL fRet = FALSE;
	CCol TempCol;
	size_t ccStmt = 0;
	DBORDINAL iCol;
	LPWSTR pwszStmtFormat = L"alter table %s add constraint %s check (%s >= '%s' or %s < '%s')";
	LPWSTR pwszConstraintName = NULL;
	LPWSTR pwszColumnName = NULL;
	LPWSTR pwszTableName = NULL;
	LPWSTR pwszConstraintValue = L"a"; // We use any bogus value because the constriant is not constrained
	LPWSTR pwszStmt = NULL;

	// This function uses SQL Server syntax.  For other DBMS's we assume the check constraints
	// already exist and so just return TRUE;
	if (!g_fSQLServer)
		return TRUE;

	// Populate the table name
	pwszTableName = m_pTable->GetTableName();

	for (iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{
		// Get the information about the column
		if (!CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK))
		{
			fRet = FALSE;
			goto CLEANUP;
		}

		// Get a char column name
		if ((TempCol.GetProviderType() != DBTYPE_STR &&
			TempCol.GetProviderType() != DBTYPE_WSTR) ||
			TempCol.GetIsLong()) 
			continue;

		// Create a unique constraint name
		pwszConstraintName = MakeObjectName(L"Cnst", 30);
		CHECK_MEMORY(pwszConstraintName);

		pwszColumnName = TempCol.GetColName();

		ccStmt = wcslen(pwszStmtFormat) + wcslen(pwszTableName) + wcslen(pwszConstraintName) +
			wcslen(pwszColumnName) + wcslen(pwszConstraintValue) + 1;

		SAFE_ALLOC(pwszStmt, WCHAR, ccStmt);

		swprintf(pwszStmt, pwszStmtFormat, pwszTableName, pwszConstraintName, pwszColumnName, pwszConstraintValue, pwszColumnName, pwszConstraintValue);

		// Execute the command expecting S_OK
		TESTC_(m_pTable->BuildCommand(pwszStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL), S_OK);

		SAFE_FREE(pwszConstraintName);
		SAFE_FREE(pwszStmt);

		fRet = TRUE;
	}

CLEANUP:
	SAFE_FREE(pwszConstraintName);
	SAFE_FREE(pwszStmt);

	return fRet;
}

BOOL CHelper::CreateStoredProcs(void)
{

	// Create at least two stored procs using the test's default table.
	
	// create proc slhproc(@p1 char(30), @p2 char(30)) as begin select * from <testtable> where charcol > 'aaa' or charcol < 'zzz'; end
	BOOL fRet = FALSE;
	CCol TempCol;
	size_t ccStmt = 0;
	DBORDINAL iCol;
	LPWSTR pwszStmtFormat = L"create procedure %s(@p1 char(30), @p2 char(30)) as begin select * from %s where %s > 'a' or %s < 'z'; end";
	LPWSTR pwszColumnName = NULL;
	LPWSTR pwszSprocName = NULL;
	LPWSTR pwszTableName = NULL;
	LPWSTR pwszStmt = NULL;

	// This function uses SQL Server syntax.  For other DBMS's we assume the procs
	// already exist and so just return TRUE;
	if (!g_fSQLServer)
		return TRUE;

	// Populate the table name
	pwszTableName = m_pTable->GetTableName();

	for (iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{
		// Get the information about the column
		if (!CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK))
		{
			fRet = FALSE;
			goto CLEANUP;
		}

		// Get a char column name
		if ((TempCol.GetProviderType() != DBTYPE_STR &&
			TempCol.GetProviderType() != DBTYPE_WSTR) ||
			TempCol.GetIsLong()) 
			continue;

		// Create a unique sproc name
		pwszSprocName = MakeObjectName(L"IDBSproc", 30);
		CHECK_MEMORY(pwszSprocName);

		pwszColumnName = TempCol.GetColName();

		ccStmt = wcslen(pwszStmtFormat) + wcslen(pwszTableName) + wcslen(pwszSprocName) +
			wcslen(pwszColumnName) + wcslen(pwszColumnName) + 1;

		SAFE_ALLOC(pwszStmt, WCHAR, ccStmt);

		swprintf(pwszStmt, pwszStmtFormat, pwszSprocName, pwszTableName, pwszColumnName, pwszColumnName);

		// Execute the command expecting S_OK
		TESTC_(m_pTable->BuildCommand(pwszStmt, IID_IRowset, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL), S_OK);

		odtLog << L"Created stored proc " << pwszSprocName << L"\n";

		SAFE_FREE(pwszSprocName);
		SAFE_FREE(pwszStmt);

		fRet = TRUE;
	}

CLEANUP:
	SAFE_FREE(pwszSprocName);
	SAFE_FREE(pwszStmt);

	return fRet;
}


BOOL CHelper::IsSupported(void)
{
	return m_fSupported;
}

void CHelper::ClearRestrictions(VARIANT * prgVariant)
{
	// By default clear the restrictions actually used
	if (!prgVariant)
		prgVariant = m_rgRestrictUsed;

	for (ULONG iRestrict = 0; iRestrict < MAXRESTRICTION; iRestrict++)
		VariantClear(&prgVariant[iRestrict]);
}

ULONG CHelper::GetLiteralMaxSize(DBLITERAL eLiteral)
{
	DBLITERALINFO* pLiteralInfo = NULL;
	pLiteralInfo = m_pTable->GetLiteralInfo(eLiteral);
	TESTC(pLiteralInfo != NULL);
	return pLiteralInfo->cchMaxLen;

CLEANUP:

	return CCHMAXLENGTH_UNKNOWN;
}


ULONG CHelper::GetRestrictionMaxLength(ULONG iRestrict)
{
	ULONG ulRestrictBit = 1 << (iRestrict-1);
	RESTRICT_CAT eRestrictType = GetRestrictCat(iRestrict);

	// Restrictions are 1 based
	_ASSERTE(iRestrict > 0);

	// We don't care about undefined restrictions
	if (eRestrictType == RC_UND)
		return CCHMAXLENGTH_UNKNOWN;

	switch (eRestrictType)
	{
		case RC_CATALOG:
			// Use max catalog name size
			return GetLiteralMaxSize(DBLITERAL_CATALOG_NAME);
		case RC_SCHEMA:
			// Use max schema name size
			return GetLiteralMaxSize(DBLITERAL_SCHEMA_NAME);
		case RC_TABLE:
			// Use max table name size
			return GetLiteralMaxSize(DBLITERAL_TABLE_NAME);
		case RC_CONSTRAINT:
//			return GetLiteralMaxSize(DBLITERAL_CONSTRAINT_NAME);
			odtLog << L"Missing DBLITERAL_CONSTRAINT_NAME in OLEDB.H, using DBLITERAL_TABLE_NAME\n";
			return GetLiteralMaxSize(DBLITERAL_TABLE_NAME);
		case RC_COLUMN:
			return GetLiteralMaxSize(DBLITERAL_COLUMN_NAME);
		case RC_USER:
			return GetLiteralMaxSize(DBLITERAL_USER_NAME);
		case RC_INDEX:
			return GetLiteralMaxSize(DBLITERAL_INDEX_NAME);
		case RC_PROCEDURE:
			return GetLiteralMaxSize(DBLITERAL_PROCEDURE_NAME);
		case RC_PARAMETER:
			// No literal value for COLUMN NAME, use COLUMN_NAME
			return GetLiteralMaxSize(DBLITERAL_COLUMN_NAME);
		case RC_CONSTRAINT_TYPE:
			return (ULONG)wcslen(L"PRIMARY KEY");
		case RC_STATISTICS:
			// No literal value for STATISTICS NAME, use COLUMN_NAME
			return (ULONG)GetLiteralMaxSize(DBLITERAL_COLUMN_NAME);
		case RC_TABLE_TYPE:
			return (ULONG)wcslen(L"GLOBAL TEMPORARY");
		// Unknown length values
		case RC_SERVER:
		case RC_UND:
			odtLog << L"Unknown restriction length, using DBLITERAL_COLUMN_NAME values.\n";
			return GetLiteralMaxSize(DBLITERAL_COLUMN_NAME);
		default:
			odtLog << L"Unknown restriction length, restriction not tested.\n";
			COMPARE(0, 1);
			return CCHMAXLENGTH_UNKNOWN;
	}

}

LPWSTR CHelper::GetRestrictionName(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		DBORDINAL iOrdinal = m_pSchemaInfo->pulRestrictions[iRestrict-1];
		return m_pSchemaInfo->pColList[iOrdinal-1].pwszName;
	}

	return NULL;
}

LPWSTR CHelper::GetCatalogName()
{
	return m_pwszCatalogRestriction;
};

RESTRICT_CAT CHelper::GetRestrictCat(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
		return m_pSchemaInfo->peRestrictCat[iRestrict-1];

	return RC_UND;
}

int CHelper::TestRestriction(ULONG iRestrict)
{
	HRESULT hrExpected = S_OK;
	HRESULT hr = E_FAIL;
	BOOL fResult = FALSE;
	BOOL fIsOutsideRange = FALSE;
	VARIANT * pVariantFirst = GetRestrictPtr(RV_ONE);

	if (!IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// If this restriction is > number of spec'd restrictions -> E_INVALIDARG
	if (iRestrict > m_pSchemaInfo->cRestrictions)
	{
		odtLog << L"Restriction " << iRestrict << L" is NOT supported.\n";
		fIsOutsideRange = TRUE;
		hrExpected = E_INVALIDARG;
	}
	else if (IsSupportedRestriction(iRestrict))
		odtLog << L"Restriction on " << GetRestrictionName(iRestrict) << L" is supported.\n";
	else
	{
		odtLog << L"Restriction on " << GetRestrictionName(iRestrict) << L" is NOT supported.\n";
		hrExpected = DB_E_NOTSUPPORTED;
	}

	SetRestrictionArray(RT_CUSTOM);

	// If this restrictions is outside the range of valid restrictions 
	// just set to the same as the first restriction, since there is
	// no value cached and we don't know which column to cache from anyway
	// or the restriction type
	if (fIsOutsideRange)
		SetRestriction(iRestrict, pVariantFirst);
	else
		SetRestriction(iRestrict);

	hr = GetRowsetHelper(NULL, iRestrict);

	// Provider is allowed to return E_INVALIDARG if restriction is not supported.
	if (hrExpected == DB_E_NOTSUPPORTED && hr == E_INVALIDARG)
		hrExpected = hr;

	TESTC_(hr, hrExpected);

	fResult = TRUE;

CLEANUP:

	ClearRestrictions();

	SetRestrictionArray(RT_SUPPORTED);

	return fResult;
}

BOOL CHelper::IsRestricted(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		return (!m_rgRestrictUsed || m_rgRestrictUsed[iRestrict-1].vt != VT_EMPTY);
	}

	return FALSE;
}

void CHelper::SetRestriction(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		CHECK(VariantCopy(&m_rgRestrictUsed[iRestrict-1], &m_rgRestrict[iRestrict-1]), S_OK);
	}
}

// Set a list of allowed values for this columm
BOOL CHelper::GetValidColumnValues(DBORDINAL iOrdinal, ULONG * pcValidValues, LPVOID * ppValidValues)
{
	// Make sure ordinal is valid
	if (!iOrdinal || iOrdinal > m_pSchemaInfo->cColumns)
		return FALSE;

	// Make sure out params were passed
	if (!pcValidValues || !ppValidValues)
		return FALSE;

	*pcValidValues = m_pSchemaInfo->pColList[iOrdinal-1].cValidVals;
	*ppValidValues = m_pSchemaInfo->pColList[iOrdinal-1].pValidVals;

	return TRUE;
}

// Set a list of allowed values for this columm
BOOL CHelper::SetValidColumnValues(DBORDINAL iOrdinal, ULONG cValidValues, LPVOID pValidValues)
{
	// Make sure ordinal is valid
	if (!iOrdinal || iOrdinal > m_pSchemaInfo->cColumns)
		return FALSE;

	m_pSchemaInfo->pColList[iOrdinal-1].cValidVals = cValidValues;
	m_pSchemaInfo->pColList[iOrdinal-1].pValidVals = pValidValues;

	return TRUE;
}

void CHelper::SetValidRestriction(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		CHECK(VariantCopy(&m_rgRestrictUsed[iRestrict-1], &m_rgRestrictValid[iRestrict-1]), S_OK);
	}
}

void CHelper::SetEmptyRestrict(ULONG iRestrict)
{
	// A valid restriction must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		V_VT(&m_rgRestrictUsed[iRestrict-1]) = VT_EMPTY;
	}
}

BOOL CHelper::SetRestriction(ULONG iRestrict, VARIANT * pVariant)
{
	BOOL fResult = FALSE;

	TESTC(pVariant != NULL);

	VariantClear(&m_rgRestrictUsed[iRestrict-1]);

	TESTC_(VariantCopy(&m_rgRestrictUsed[iRestrict-1], pVariant), S_OK);

	fResult = TRUE;

CLEANUP:

	return fResult;
}


BOOL CHelper::SetRestriction(ULONG iRestrict, LPVOID pValue)
{
	VARIANT * pVariant = NULL;
	DBTYPE wType;
	DBORDINAL iOrdinal;
	BOOL fResult = FALSE;

	// Must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= m_pSchemaInfo->cRestrictions, TRUE))
	{
		iOrdinal = m_pSchemaInfo->pulRestrictions[iRestrict-1];
		wType = m_pSchemaInfo->pColList[iOrdinal-1].wType;

		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		pVariant = DBTYPE2VARIANT(pValue, wType);
		TESTC(pVariant != NULL);

		TESTC_(VariantCopy(&m_rgRestrictUsed[iRestrict-1], pVariant), S_OK);

		fResult = TRUE;
	}

CLEANUP:
	if (pVariant)
		VariantClear(pVariant);
	SAFE_FREE(pVariant);

	return fResult;
}

BOOL CHelper::SetRestriction(ULONG iRestrict, LPVOID pValue, DBTYPE wType)
{
	VARIANT * pVariant = NULL;
	BOOL fResult = FALSE;

	// Must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= MAXRESTRICTION, TRUE))
	{
		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		pVariant = DBTYPE2VARIANT(pValue, wType);
		TESTC(pVariant != NULL);

		TESTC_(VariantCopy(&m_rgRestrictUsed[iRestrict-1], pVariant), S_OK);

		fResult = TRUE;
	}

CLEANUP:
	if (pVariant)
		VariantClear(pVariant);
	SAFE_FREE(pVariant);

	return fResult;
}

BOOL CHelper::SetInvalidRestriction(ULONG iRestrict, LPVOID pValue, DBTYPE wType)
{
	BOOL fResult = FALSE;

	// Must be within range
	if (COMPARE(iRestrict > 0 && iRestrict <= MAXRESTRICTION, TRUE))
	{
		VariantClear(&m_rgRestrictUsed[iRestrict-1]);
		V_VT(&m_rgRestrictUsed[iRestrict-1]) = wType;
		m_rgRestrictUsed[iRestrict-1].punkVal = (struct IUnknown *)pValue;

		fResult = TRUE;
	}

	return fResult;
}


void CHelper::SetRestrictionType(ULONG iRestrict, DBTYPE wType)
{
	if (COMPARE(iRestrict > 0 && iRestrict <= MAXRESTRICTION, TRUE))
		V_VT(&m_rgRestrictUsed[iRestrict-1]) = wType;
}

VARIANT * CHelper::GetRestrictPtr(ULONG iRestrict)
{
	VARIANT	*		prgRestrictions = NULL;

	if (COMPARE(iRestrict > 0 && iRestrict <= MAXRESTRICTION, TRUE))
	{
		switch(m_eRestrictArray)
		{
			case RT_CACHED:
				prgRestrictions = m_rgRestrict;
				break;
			case RT_SUPPORTED:
				prgRestrictions = m_rgRestrictValid;
				break;
			case RT_CUSTOM:
				prgRestrictions = m_rgRestrictUsed;
				break;
		}

		_ASSERTE(prgRestrictions);

		return &prgRestrictions[iRestrict-1];
	}

	return NULL;
}

BOOL CHelper::FillRestrictions(DBCOUNTITEM iRow, LPBYTE pData)
{
	// Fill restrictions with values from this row
	DBTYPE wType;
	ULONG iRestrict = 0;
	ULONG iEnd = m_pSchemaInfo->cRestrictions;
	VARIANT * pVariant = NULL;
	LPBYTE pValue = NULL;
	BOOL fResult = FALSE;

	// If we're not on the right row, then bail
	if (iRow != m_ulRestrictRow)
		return TRUE;

	// This is the right row, fill the restriction column(s) desired
	if (m_iRestrict)
	{
		// Only want to restrict this restriction, convert to 0-based
		iRestrict = m_iRestrict-1;
		TESTC(iRestrict < iEnd); // Make sure a bad restriction wasn't passed
		iEnd = m_iRestrict;
	}


	for (; iRestrict < iEnd; iRestrict++)
	{
		DBORDINAL iOrdinal = m_pSchemaInfo->pulRestrictions[iRestrict];
		wType = m_pSchemaInfo->pColList[iOrdinal-1].wType;
		pValue = GetValuePtr(iOrdinal, pData, m_rgDBBINDING);

		// If there is no value, then restriction must be VT_NULL;
		if (!pValue)
		{
			V_VT(&m_rgRestrict[iRestrict]) = VT_NULL;	
			continue;
		}

		// GUID restrictions are mapped to BSTR
		if (wType == DBTYPE_GUID)
		{
			wType = DBTYPE_BSTR;
			// We don't support this yet, need conversionn from GUID->BSTR
			TESTC(FALSE);
		}

		pVariant = DBTYPE2VARIANT(pValue, wType);
		
		TESTC(pVariant != NULL);

		TESTC_(VariantCopy(&m_rgRestrict[iRestrict], pVariant), S_OK)

		VariantClear(pVariant);
		SAFE_FREE(pVariant);
	}

	fResult = TRUE;

CLEANUP:

	if (pVariant)
		VariantClear(pVariant);
	SAFE_FREE(pVariant);

	m_fCaptured = TRUE;	// Otherwise we'll loop forever looking for restriction

	return fResult;
}

BOOL CHelper::GetRestrictions(ULONG iRestrict)
{
	IRowset * pIRowset = NULL;
	BOOL fResult = FALSE;

	// Pick a row and use this row's value for restriction iRestrict (1-based)
	// If iRestrict is 0, then all restrictions are filled from this row.

	// At some later time we will add code to select an interesting restriction such
	// as a NULL value.

	// For now the row is hard-coded, but later we will use some heuristic to determine
	m_ulRestrictRow = 2;
	m_iRestrict = iRestrict;
	m_fCaptureRestrictions = TRUE;
	m_fCaptured = FALSE;

	// Clear all cached restrictions
	ClearRestrictions(m_rgRestrict);

	// Call the GetRowsetHelper to actually do the work of filling the restrictions
	// if the schema is supported, otherwise just leave VT_EMPTY
	if (m_fSupported)
	{
		TESTC_(GetRowsetHelper(NULL, RV_NULL), S_OK);

		// Now m_rgRestrict is filled with valid known values for each possible restriction,
		// but not all of them may be supported restrictions.  Copy supported restrictions to
		// m_rgRestrictValid.
		for (iRestrict = 1; iRestrict <= m_pSchemaInfo->cRestrictions; iRestrict++)
		{
			if (IsSupportedRestriction(iRestrict))
			{
				VariantClear(&m_rgRestrictValid[iRestrict-1]);
				TESTC_(VariantCopy(&m_rgRestrictValid[iRestrict-1], &m_rgRestrict[iRestrict-1]), S_OK)
			}
		}
	}

	fResult = TRUE;

CLEANUP:

	return fResult;
	
}

BOOL CHelper::IsSupportedRestriction(ULONG iRestrict)
{
	if (COMPARE(iRestrict > 0, TRUE))
		return m_ulRestrictionSupport & (1<<(iRestrict-1));
	else
		return FALSE;
}

BOOL CHelper::HasUnsupportedRestriction(void)
{
	for(ULONG iRestrict = 1; iRestrict <= m_pSchemaInfo->cRestrictions; iRestrict++)
	{
		if (!IsSupportedRestriction(iRestrict))
			return TRUE;
	}

	return FALSE;
}

void CHelper::SetRowCount(ROW_COUNT eRowCount, DBORDINAL ulRowCount)
{
	m_eRowCount = eRowCount;
	m_ulRowCount = ulRowCount;
}

BOOL CHelper::CheckAgainstIColumnsInfo(SchemaList * pSchemaInfo)
{
	BOOL	fResult			= TRUE;
	BOOL	fCountNotChecked= FALSE;
	BOOL	fBookmark		= FALSE;
	ULONG	ulIndex			= 0;
	ColList * pColList = pSchemaInfo->pColList;
	ULONG	cCols	= pSchemaInfo->cColumns;
	ULONG	iOrdinalExpected = 0;
	ULONG	iSchemaInfo	= 0;

	// Make sure the right number of columns were returned for this schema
	COMPARE(cCols <= (m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal), TRUE);

	// Make sure the rowset is not null
	if((!m_pIRowset) || (!m_cDBCOLUMNINFO) || (!m_rgDBCOLUMNINFO) || (!m_pStringsBuffer)) {
		odtLog << L"CheckAgainstIColumnsInfo:Either rowset or DBCOLUMNINFO variables are null."<<ENDL;
		return FALSE;
	}

	// Check for the Bookmark
	if(!m_rgDBCOLUMNINFO[0].iOrdinal)
		fBookmark=TRUE;

	if (fBookmark)
	{
		if (!COMPARE(GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIRowset), TRUE))
			odtLog << m_pSchemaInfo->pwszName << L": Bookmark column was found but the DBPROP_BOOKMARKS was not set.\n";
	}
	
	// Check column ordering, column name and data type, skip bookmark column
	for(ulIndex = (0 + fBookmark); ulIndex < m_cDBCOLUMNINFO; ulIndex ++)
	{
		iOrdinalExpected = ulIndex-fBookmark+1;
		iSchemaInfo = ulIndex-fBookmark;

		// only check spec'd columns, not any provider-specific columns
		if(iSchemaInfo < cCols)
		{
			// Check column ordinal
			if(!COMPARE(m_rgDBCOLUMNINFO[ulIndex].iOrdinal, iOrdinalExpected))
			{
				odtLog << L"ORDINAL ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex
						<< L"]: Expected "<< iOrdinalExpected  	<< L", Received "	<< m_rgDBCOLUMNINFO[ulIndex].iOrdinal	<< ENDL;
				fResult = FALSE;
			}

			// If there is a name in this column, check to make sure it is correct
			if( (m_rgDBCOLUMNINFO[ulIndex].pwszName) && 
				(!COMPARE(0, _wcsicmp(m_rgDBCOLUMNINFO[ulIndex].pwszName,pColList[iSchemaInfo].pwszName))))
			{
				odtLog << L"NAME ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L","
					<< ulIndex 	<< L"]: Expected " 	<< 	pColList[iSchemaInfo].pwszName << L", Received "
					<< m_rgDBCOLUMNINFO[ulIndex].pwszName << ENDL;
				fResult = FALSE;
			}

			// Check column type
			if(!COMPARE(m_rgDBCOLUMNINFO[ulIndex].wType, pColList[iSchemaInfo].wType))
			{
				odtLog << L"TYPE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
						<< 	pColList[iSchemaInfo].wType	<< L", Received " << m_rgDBCOLUMNINFO[ulIndex].wType << ENDL;
				fResult = FALSE;
			}

			// Check column size
			if((!IsFixedLength(m_rgDBCOLUMNINFO[ulIndex].wType)) && (!m_rgDBCOLUMNINFO[ulIndex].ulColumnSize))
			{
				odtLog << L"COLUMNSIZE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: "
						<< L"Column Size for variable length Column is ZERO, expected non-zero." << ENDL;
				fResult = FALSE;
			}
			else if((IsFixedLength(m_rgDBCOLUMNINFO[ulIndex].wType)) && 
					(!COMPARE((ULONG)GetDBTypeSize(m_rgDBCOLUMNINFO[ulIndex].wType), m_rgDBCOLUMNINFO[ulIndex].ulColumnSize)))
			{
				odtLog << L"COLUMNSIZE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
						<< L"Column Size for the Column is incorrect. " << ENDL;
				fResult = FALSE;
			}

			// Check column precision and scale
			// Note some provider return an alternate precision/scale for 
			// DBTYPE_CY
			if(IsNumericType(m_rgDBCOLUMNINFO[ulIndex].wType) &&
				m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_CY)
			{
				ULONG MaxPrecision = 0;

				// Switch on the Type
				switch(m_rgDBCOLUMNINFO[ulIndex].wType)
				{
					case DBTYPE_I1:
					case DBTYPE_UI1:
						MaxPrecision = 3;
						break;
					case DBTYPE_I2:
					case DBTYPE_UI2:
						MaxPrecision = 5;
						break;
					case DBTYPE_R4:
						MaxPrecision = 7;
						break;
					case DBTYPE_I4:
					case DBTYPE_UI4:
						MaxPrecision = 10;
						break;
					case DBTYPE_R8:
						MaxPrecision = 15;
						break;
					case DBTYPE_CY:
					case DBTYPE_I8:
						MaxPrecision = 19;
						break;
					case DBTYPE_UI8:
						MaxPrecision = 20;
						break;
				}

				// Precision is valid
				if (((m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_DECIMAL) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_NUMERIC) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_VARNUMERIC)) && 
					(MaxPrecision != m_rgDBCOLUMNINFO[ulIndex].bPrecision) )
				{
					odtLog << L"PRECISION ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  << L"]: Expected "
							<< 	MaxPrecision 	<< L", Received " << m_rgDBCOLUMNINFO[ulIndex].bPrecision << ENDL;
					fResult = FALSE;
				}

				// Scale is  ~0
				if (((m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_DECIMAL) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_NUMERIC) && 
					 (m_rgDBCOLUMNINFO[ulIndex].wType != DBTYPE_VARNUMERIC)) && 
					(!COMPARE((BYTE)(~0), m_rgDBCOLUMNINFO[ulIndex].bScale)))
				{
					odtLog << L"SCALE ["	<< m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  
						<< L"]: Expected the Scale to be ~0" << L", Received " << m_rgDBCOLUMNINFO[ulIndex].bScale << ENDL;
					fResult = FALSE;
				}
			}
			else if (m_rgDBCOLUMNINFO[ulIndex].wType == DBTYPE_DBTIMESTAMP ||
					m_rgDBCOLUMNINFO[ulIndex].wType == DBTYPE_CY)
			{
				CCol TempCol;
				BYTE bPrecision = 0;
				BYTE bScale = 0;

				// Get bPrecision and bScale for DBTYPE_DBTIMESTAMP or
				// DBTYPE_CY from CCol object
				for (DBORDINAL iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
				{
					// Get the information about the column
					if (!CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK))
					{
						fResult = FALSE;
						goto CLEANUP;
					}

					// If this is DBTYPE_DBTIMESTAMP it may be the right one
					if (m_rgDBCOLUMNINFO[ulIndex].wType == TempCol.GetProviderType())
					{
						bPrecision = TempCol.GetPrecision();
						bScale = TempCol.GetScale();

						if (bPrecision == m_rgDBCOLUMNINFO[ulIndex].bPrecision &&
							bScale == m_rgDBCOLUMNINFO[ulIndex].bScale)
						{
							// We found a matching value for precision and scale
							break;
						}
					}
				}

				// Precision is defined as length of string representation assuming max
				// allowed precision of fractional seconds component.
				// TODO: Compare against CCol. 
				if (!COMPARE(m_rgDBCOLUMNINFO[ulIndex].bPrecision, bPrecision))
				{
					odtLog <<L"PRECISION [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:expected valid precision."<<ENDL;
					fResult = FALSE;
				}

				// Scale is just the maximum length of the string representation of 
				// fractional seconds component.
				if (!COMPARE(m_rgDBCOLUMNINFO[ulIndex].bScale, bScale))
				{
					odtLog <<L"SCALE [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:expected valid scale."<<ENDL;
					fResult = FALSE;
				}
			}
			else
			{
				// Precision and scale are ~0
				if (((BYTE)(~0) != m_rgDBCOLUMNINFO[ulIndex].bPrecision) || ((BYTE)(~0) != m_rgDBCOLUMNINFO[ulIndex].bScale))
				{
					COMPARE(0, 1);
					odtLog <<L"PRECISION & SCALE [" << m_rgDBCOLUMNINFO[ulIndex].pwszName << L"," << ulIndex  <<"]:non-numeric expect ~0."<<ENDL;
					fResult = FALSE;
				}
			}
		}
		else
		{
			// Provider-specific column was found, print warning
			// Since I really don't want to see this warning every time I verify a schema's rowset, but
			// I really do want to see it if there are different provider-specific columns than last
			// time, I guess I have to cache the columns info and compare here before printing the
			// warning - what a pain.
			LPWSTR pwszName		= m_rgDBCOLUMNINFO[ulIndex].pwszName;
			LPWSTR pwszPrevName	= NULL;
			ULONG ulIndexPrev	= 0;

			if (m_rgDBCOLUMNINFO_Prev)
			{
				ulIndexPrev	= ulIndex - !m_rgDBCOLUMNINFO[0].iOrdinal + !m_rgDBCOLUMNINFO_Prev[0].iOrdinal;

				if (ulIndexPrev < m_cDBCOLUMNINFO_Prev)
					pwszPrevName	= m_rgDBCOLUMNINFO_Prev[ulIndexPrev].pwszName;
			}

			// If we don't know whether the names match print the warning
			if (!pwszName || !pwszPrevName)
			{
				if (!pwszName)
					pwszName = L"<NULL>";
				odtLog << L"Warning - " << m_pSchemaInfo->pwszName << L" provider specific column name: " << pwszName << "\n";
			}
			// If we can tell the names match, don't print the warning
			else if (wcscmp(pwszName, pwszPrevName))
				odtLog << L"Warning - " << m_pSchemaInfo->pwszName << L" provider specific column name: " << pwszName << "\n";
		}
	}

CLEANUP:

	return fResult;
}

HRESULT CHelper::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRows)
{
	ASSERT(prghRows);
	TBEGIN
	
	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM i=0;
	
	//Record if we passed in consumer allocated array...
	HROW* rghRowsInput = *prghRows;

	//GetNextRows
	HRESULT hr = m_pIRowset->GetNextRows(NULL, lOffset, cRows, &cRowsObtained, prghRows);
	
	//Verify Correct values returned
	if(SUCCEEDED(hr))
	{
		if(hr == S_OK)
		{
			TESTC(cRowsObtained==(DBCOUNTITEM)ABS(cRows));
		}
		else
		{
			TESTC(cRowsObtained < (DBCOUNTITEM)ABS(cRows));
		}

		//Verify row array
		for(i=0; i<cRowsObtained; i++)
		{
			TESTC(*prghRows != NULL);
			TESTC((*prghRows)[i]!=DB_NULL_HROW)
		}
	}
	else
	{
		TESTC(cRowsObtained == 0);
	}

	//Verify output array, depending upon consumer or provider allocated...
	if(rghRowsInput)
	{
		//This is a users allocated static array,
		//This had better not be nulled out by the provider, if non-null on input
		TESTC(*prghRows == rghRowsInput);
	}
	else
	{
		TESTC(cRowsObtained ? *prghRows != NULL : *prghRows == NULL);
	}

CLEANUP:
	if(pcRowsObtained)
		*pcRowsObtained = cRowsObtained;
	return hr;
}

// GetValuePtr
// Assumptions: All columns are bound
LPBYTE CHelper::GetValuePtr(DBORDINAL iOrdinal, LPBYTE pData, DBBINDING * pBinding)
{
	LPBYTE pValue = NULL;
	DBORDINAL iBind = 0;

	// If no pData passed in then we can't give the value
	if (!pData)
		return NULL;

	// If no binding structure passed in assume user wants m_rgDBBINDING
	if (!pBinding)
		pBinding = m_rgDBBINDING;

	if (pBinding)
	{
		iBind = iOrdinal - pBinding[0].iOrdinal;

		// We always expect the status to be DBSTATUS_S_OK or DBSTATUS_S_ISNULL, but this is verified
		// earlier in VerifyColumnCommon()
		if (STATUS_BINDING(pBinding[iBind], pData) == DBSTATUS_S_OK)
			pValue = (LPBYTE)&VALUE_BINDING(pBinding[iBind], pData);
	}

	return pValue;
}


BOOL CHelper::VerifyRowset(IRowset * pIRowset)
{
	BOOL		fVerifyRow = TRUE;
	HRESULT		hr = E_FAIL;
	BOOL		fReleaseRowset = FALSE;			// whether I should release pIRowset;
	HACCESSOR	hAccessor=DB_NULL_HACCESSOR;	// handle to accessor
	ULONG		iBind = 0;						// current binding
	ULONG		iRow = 0;						// current row
	DBLENGTH	cbRowSize = 0;					// size of row
	DBCOUNTITEM	cRowsObtained = 0;				// total rows obtained from getnextrows
	BYTE *		pRowBuff = NULL;					// row of Data
	HROW *		prghRows = NULL;					// array of hrows
	IAccessor * pIAccessor = NULL;
	ULONG		ulRowsToVerify;
	BOOL		fCapturingRestrictions = m_fCaptureRestrictions;
	BOOL		fContinue = TRUE;
	GUID		guidSchema = *m_pSchemaInfo->pguid;

	// Reset row count from this schema
	m_cTotalRows = 0;

	// get bindings and column info
	if(!CHECK(hr=GetAccessorAndBindings(
		pIRowset,									// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&hAccessor,									// @parmopt [OUT] Accessor created
		&m_rgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cDBBINDING,								// @parmopt [OUT] Count of bindings
		&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Array of DBCOLUMNINFO
		&m_cDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
		goto CLEANUP;

	// Don't need to check if only capturing restrictions
	if(!m_fCaptureRestrictions)
	{
		fVerifyRow &= CheckAgainstIColumnsInfo(m_pSchemaInfo);

		// Free any previous columninfo
		SAFE_FREE(m_rgDBCOLUMNINFO_Prev);
		SAFE_FREE(m_pStringsBuffer_Prev);

		// Save the pointer to the previous columninfo
		m_cDBCOLUMNINFO_Prev = m_cDBCOLUMNINFO;
		m_rgDBCOLUMNINFO_Prev = m_rgDBCOLUMNINFO;
		m_pStringsBuffer_Prev = m_pStringsBuffer;
	}

	// check if we need to go on
	TESTC(m_cDBBINDING && m_cDBCOLUMNINFO && m_rgDBBINDING && m_rgDBCOLUMNINFO)

	if(m_fPassUnsupportedRestrictions)
	{
		fVerifyRow = TRUE;
		goto CLEANUP;
	}

	// create space for two rows of data
	SAFE_ALLOC(pRowBuff, BYTE, cbRowSize*2);

	m_pRow = pRowBuff;	// Start by writing into beginning of buffer

	if(GetModInfo()->GetDebugMode() & DEBUGMODE_FULL )
		ulRowsToVerify = 90000;
	else
		// This should be larger than NUMROWS_CHUNK to force at least one additional
		// GetNextRows call below.
		ulRowsToVerify = 100;

	_ASSERTE(m_pRowPrev == NULL);
 
	// Process all the rows, NUMROWS_CHUNK rows at a time
	while(fContinue)
	{
		// get rows to process
		hr=GetNextRows(0,30,&cRowsObtained,&prghRows);

		// odtLog << L"Rows obtained: " << m_cTotalRows + cRowsObtained << L"\n";

		TEST4C_(hr, S_OK, DB_S_ENDOFROWSET, DB_S_STOPLIMITREACHED, DB_S_ROWLIMITEXCEEDED);

		// verify that we have rows to process
		if(cRowsObtained==0)
		{
			// Hack for Kagera.  Kagera will not return rows from many schemas when unrestricted,
			// so we are unable to capture any restrictions.  In this case Kagera will return rows
			// when restricted to a table, and since we know at least one valid table, use that one
			if (g_fKagera && m_fCaptureRestrictions)
			{
				ULONG iRestrict = 0;	// 0 based restriction index
				LPWSTR pwszRestrict = NULL;

				if (guidSchema == DBSCHEMA_COLUMN_PRIVILEGES ||
					guidSchema == DBSCHEMA_COLUMNS ||
					guidSchema == DBSCHEMA_CONSTRAINT_COLUMN_USAGE ||
					guidSchema == DBSCHEMA_CONSTRAINT_TABLE_USAGE ||
					guidSchema == DBSCHEMA_STATISTICS ||
					guidSchema == DBSCHEMA_TABLE_PRIVILEGES ||
					guidSchema == DBSCHEMA_TABLE_STATISTICS ||
					guidSchema == DBSCHEMA_TABLES ||
					guidSchema == DBSCHEMA_TABLES_INFO ||
					guidSchema == DBSCHEMA_VIEWS)
				{
					// Use table name as restriction
					pwszRestrict = m_pTable->GetTableName();
					iRestrict = 2;
				}
				else if	(guidSchema == DBSCHEMA_INDEXES)
				{
					// Use table name as restriction
					pwszRestrict = m_pTable->GetTableName();
					iRestrict = 4;
				}
				else if	(guidSchema == DBSCHEMA_PRIMARY_KEYS)
				{
					// Use primary/foreign key table name as restriction
					pwszRestrict = g_pKeyTable1->GetTableName();
					iRestrict = 2;
				}
				else if	(guidSchema == DBSCHEMA_FOREIGN_KEYS)	// This is really FK_TABLE_NAME, but it will suffice
				{
					// Use primary/foreign key table name as restriction
					pwszRestrict = g_pKeyTable1->GetTableName();
					iRestrict = 5;
				}
				else if	(guidSchema == DBSCHEMA_KEY_COLUMN_USAGE ||
						guidSchema == DBSCHEMA_TABLE_CONSTRAINTS)
				{
					// Use table name as restriction
					pwszRestrict = m_pTable->GetTableName();
					iRestrict = 5;
				}

				// Set the restriction to use
				V_VT(&m_rgRestrictValid[iRestrict]) = VT_BSTR;
				V_BSTR(&m_rgRestrictValid[iRestrict]) = SysAllocString(pwszRestrict);

				// Clean up array binding information and release memory associated with empty rowset 
				if (pIAccessor && hAccessor != DB_NULL_HACCESSOR)
					CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

				SAFE_RELEASE(pIAccessor);
				SAFE_RELEASE(m_pIRowset);
				SAFE_FREE(m_rgDBCOLUMNINFO);
				SAFE_FREE(m_pStringsBuffer);
				SAFE_FREE(m_rgDBBINDING);
				SAFE_FREE(prghRows);

				// Call again with a table name restriction
				hr=m_pIDBSchemaRowset->GetRowset(NULL, guidSchema, m_pSchemaInfo->cRestrictions, m_rgRestrictValid,
					IID_IRowset, 0, NULL, (IUnknown **)&m_pIRowset);

				TESTC_(hr, S_OK);

				// get bindings and column info
				if(!CHECK(hr=GetAccessorAndBindings(
					m_pIRowset,									// @parmopt [IN]  Rowset to create Accessor for
					DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
					&hAccessor,									// @parmopt [OUT] Accessor created
					&m_rgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
					&m_cDBBINDING,								// @parmopt [OUT] Count of bindings
					&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
					DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
					ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
					FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
					NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
					&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Array of DBCOLUMNINFO
					&m_cDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
					&m_pStringsBuffer,
					DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
					0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
					NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
					NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
					NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
					DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
					BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
					NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
					goto CLEANUP;

				// Call GetNextRows again
				hr=GetNextRows(0,30,&cRowsObtained,&prghRows);

				// Make sure we don't leak the restriction
				VariantClear(&m_rgRestrictValid[iRestrict]);

				// Theoretically we now have some rows, but if not we're still in the same boat we were before
			}

			if (cRowsObtained == 0)
			{
				TESTC_(hr, DB_S_ENDOFROWSET);
				break;
			}
		}

		// Only verify required number of rows
		if (m_cTotalRows < ulRowsToVerify)
		{
			// Make sure we don't call verify with no valid restrictions.
//			ASSERT((m_restrict != ALLRES) || (m_currentBitMask == 0));

			// Loop over rows obtained, getting data for each
			for(iRow=0;iRow<cRowsObtained && fContinue;iRow++)
			{
				m_cTotalRows++;

				// get row
				TESTC_(m_pIRowset->GetData(prghRows[iRow],hAccessor,m_pRow), S_OK)

				// If we are just capturing restrictions then don't do verification
				if (m_fCaptureRestrictions)
				{
					// Adjust our restriction row if the rowset doesn't have enough rows
					if (m_cTotalRows - 1 + cRowsObtained < m_ulRestrictRow)
						m_ulRestrictRow = m_cTotalRows - 1 + cRowsObtained;

					TESTC(FillRestrictions(m_cTotalRows, m_pRow))
				}
				else
				{
 					// Verify the row
					fVerifyRow &= VerifyRowCommon(m_cTotalRows,m_pRow);

					// Reset to point to the next available row location
					m_pRowPrev = m_pRow;
					if (m_pRow == pRowBuff)
						m_pRow += cbRowSize;
					else
						m_pRow = pRowBuff;
				}

				// If we know we've filled the restriction then break out of loop
				// Otherwise we continue for all rows
				if (m_fCaptureRestrictions && m_fCaptured)
				{
					m_fCaptureRestrictions = FALSE;
					fContinue = FALSE;
					break;
				}
			}
		}
		else
			m_cTotalRows += cRowsObtained;

		TESTC_(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK)

		SAFE_FREE(prghRows);
	}

	switch (m_eRowCount)
	{	
		case MIN_VALUE:
			// Warn if we didn't get any rows but expected some
			if (m_ulRowCount && !m_cTotalRows)
				odtLog << L"Warning - this rowset didn't return any rows so no values were checked.\n";

			// Fail if we got some rows but less than minimum expected
			if (m_cTotalRows)
				TESTC(m_cTotalRows >= m_ulRowCount);
			break;
		case MIN_REQUIRED:
			// Fail if we got less than minimum expected
			if (!COMPARE(m_cTotalRows >= m_ulRowCount, TRUE))
				odtLog << L"Min row count not retrieved. Expected: >= " << m_ulRowCount << L" Received: " << m_cTotalRows
					<< L"\n";
			break;
		case EXACT_VALUE:
	//		ASSERT(m_cTotalRows == m_ulRowCount);
			if (m_cTotalRows != m_ulRowCount)
				odtLog << L"Expected " << m_ulRowCount << " rows, received " << m_cTotalRows << " rows.\n";

			TESTC(m_cTotalRows == m_ulRowCount);
			break;
		default:
			ASSERT(!L"Unexpected row count value for schema rowset.");
	}


CLEANUP:

	// If we were capturing restrictions but didn't capture them for some reason
	// then return FALSE
	if (m_fCaptureRestrictions && !m_fCaptured)
	{
		fVerifyRow = FALSE;
		m_fCaptureRestrictions = FALSE;  // Don't continue to capture
		COMPARE(TRUE, FALSE);
	}

	if (pIAccessor && hAccessor != DB_NULL_HACCESSOR)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(m_pIRowset);

	m_cDBCOLUMNINFO = 0;
	m_cDBBINDING = 0;

	// m_pRowPrev and m_pRow just point into pRowBuff, so don't free them, but
	// set to NULL to avoid using in later variations.
	m_pRow = NULL;
	m_pRowPrev  = NULL;
	if(!m_cDBCOLUMNINFO_Prev)
	{
		PROVIDER_FREE(m_rgDBCOLUMNINFO);
		PROVIDER_FREE(m_pStringsBuffer);
	}
	else
	{
		m_rgDBCOLUMNINFO = NULL;	// This is not freed, since it's used in m_rgDBCOLUMNINFO_prev
		m_pStringsBuffer = NULL;	// ""
	}

	// Free the memory
	PROVIDER_FREE(m_rgDBBINDING);
	PROVIDER_FREE(pRowBuff);

	return fVerifyRow;
}

BOOL CHelper::CheckResults(HRESULT hr, REFIID riid, IUnknown * pIUnknown)
{	
	BOOL fResult = FALSE;

	if (SUCCEEDED(hr))
	{
		// Verify the interface requested is usable
		TESTC(DefaultInterfaceTesting(pIUnknown, ROWSET_INTERFACE, riid))

		// Verify the properties are correct on the rowset
		// TODO:

		// Get a rowset interface
		TESTC(VerifyInterface(pIUnknown, IID_IRowset, 
							ROWSET_INTERFACE, (IUnknown**)&m_pIRowset))

		// Make sure the rowset is correct
		VerifyRowset(m_pIRowset);			
	}
	else 
	{
		// Should not be a rowset
		TESTC(pIUnknown == NULL);
	}

	fResult = TRUE;

CLEANUP:

	SAFE_RELEASE(m_pIRowset);

	return fResult;
}

BOOL CHelper::VerifyRowCommon(DBCOUNTITEM iRow, BYTE * pData)
{

	ULONG 	iBind=0;		// Binding Count
    DATA *	pColumn=NULL;	// Data Structure
	CCol	col;
	BOOL	fResults=TRUE;
	DBORDINAL iOrdinal = 0;

	// Perform common checks for each schema
	fResults &= VerifySort(iRow, pData);

	// Many schemas identify a fully qualified table name.  Validate this name is valid
	// by attempting to open the table.
	fResults &= VerifyValidTable(iRow, pData);

	// check each column we're bound to.
	for (iBind=0; iBind < m_cDBBINDING; iBind++)
	{
		// grab column
		pColumn = (DATA *) (pData + m_rgDBBINDING[iBind].obStatus);
		iOrdinal = m_rgDBCOLUMNINFO[iBind].iOrdinal;
		
//		PRVTRACE(L"Row[%lu],Col[%s]:%s:", iRow,m_rgDBCOLUMNINFO[iBind].pwszName,m_pSchemaInfo->pwszName);

		// Perform common checks for each column
		fResults &= VerifyColumnCommon(iBind, pColumn);

	}
	return fResults;
}

// Compare each value in order for the sort columns.  The
// current row values must be >= previous row values.
BOOL CHelper::VerifySort(DBCOUNTITEM iRow, BYTE * pData)
{
	BOOL fResult = TRUE;
	ULONG cSortCols = m_pSchemaInfo->cSort;
	ULONG iSort;
	BOOL fGreater = TRUE;

	// If not two rows then we can't determine sort
	if (!m_pRowPrev)
		return TRUE;

	if (!COMPARE(m_pSchemaInfo->pulSort != NULL, TRUE))
		return FALSE;

	for (iSort = 0; iSort < cSortCols; iSort++)
	{
		ULONG iSortCol = m_pSchemaInfo->pulSort[iSort];
		DBTYPE wType = m_pSchemaInfo->pColList[iSortCol-1].wType;
		LPBYTE pCurrCol = GetValuePtr(iSortCol, pData, m_rgDBBINDING);
		LPBYTE pPrevCol = GetValuePtr(iSortCol, m_pRowPrev, m_rgDBBINDING);
		LONG fComp;

		// We assume NULLs are equal
		if (!pCurrCol && !pPrevCol)
			continue;

		// If one of the values is NULL then we have to use DBPROP_NULLCOLLATION to decide 
		// if they're sorted properly
		if ((pCurrCol == NULL) || (pPrevCol == NULL))
		{
			if (pCurrCol == pPrevCol)
				fComp = 0;	// Equal
			else if (pPrevCol == NULL &&
				(m_ulNullCollation == DBPROPVAL_NC_LOW ||
				m_ulNullCollation == DBPROPVAL_NC_START))
				fComp = 1;	// NULL was less than current value
			else if (pCurrCol == NULL &&
				(m_ulNullCollation == DBPROPVAL_NC_HIGH ||
				m_ulNullCollation == DBPROPVAL_NC_END))
				fComp = 1;	// Previous value was less than NULL
			else
				fComp = -1;	// NULL was not properly sorted
		}
		else
		{
			// If the values don't compare properly, then it's a failure
			fComp = RelativeCompare(pCurrCol, pPrevCol, wType, 0, 0, 0, 0);
		}

		if (fComp > 0)
		{
			// We found a larger value
			fGreater = TRUE;
			break;
		}
		else if (fComp == 0)
			continue;
		else
		{
			// We found a smaller value, error
			fGreater = FALSE;
			CCOMPARE(m_ECHelper, fGreater == TRUE, 
				EC_INCORRECT_SORT,
				L"Sort is incorrect",
				FALSE);
			break;
		}
	}

	if (!fGreater)
		fResult = FALSE;

	return fResult;
}

BOOL CHelper::VerifyValidTable(DBCOUNTITEM iRow, LPBYTE pData)
{
	// We will only do this for certain specific schemas
	if (m_guidSchema == DBSCHEMA_TABLES ||
		m_guidSchema == DBSCHEMA_TABLES_INFO ||
		m_guidSchema == DBSCHEMA_COLUMNS)
	{
 		// Try to open a rowset on the table.  The first 3 bindings contain
		// Catalog, Schema, and Table Name, respectively
		GetColumnInfo(pData, m_rgDBBINDING);
	}

	return TRUE;
}

BOOL CHelper::VerifyLength(DBORDINAL iBind, DATA * pColumn)
{
	DBTYPE wType = m_rgDBCOLUMNINFO[iBind].wType;
	size_t ulLength = GetDBTypeSize(wType);
	BOOL fResult = TRUE;

	// GetDBTypeSize won't return a value for variable length columns
	if (!ulLength)
	{
		switch(wType)
		{
			case DBTYPE_STR:
				ulLength = strlen((LPSTR)&pColumn->bValue);
				break;
			case DBTYPE_WSTR:
				ulLength = wcslen((LPWSTR)&pColumn->bValue)*sizeof(WCHAR);
				break;
		}
	}

	if (wType == DBTYPE_BYTES)
	{
		// We can't get the length of a BYTES column so we will just
		// attempt to access every byte in the hopes of crashing if
		// the length isn't valid.
		BYTE bVal;
		ULONG iByte;
		for (iByte = 0; iByte < pColumn->ulLength; iByte++)
			bVal = pColumn->bValue[iByte];
		fResult = TRUE;
	}
	else
		fResult = COMPARE(pColumn->ulLength, ulLength);	

	// Also, if the length is greater than cbMaxLen in the binding then
	// our status should be DBSTATUS_S_TRUNCATED
	if (pColumn->ulLength > m_rgDBBINDING[iBind].cbMaxLen)
		fResult &= COMPARE(pColumn->sStatus, DBSTATUS_S_TRUNCATED);

	return fResult;
}

BOOL CHelper::VerifyColumnCommon(DBORDINAL iBind, DATA * pColumn)
{
	BOOL fResults = TRUE;
	DBORDINAL iOrdinal = m_rgDBBINDING[iBind].iOrdinal;

	// Can't verify column values if status is not OK
	if(pColumn->sStatus==DBSTATUS_S_OK)
	{
		// If restrictions were specified, validate they were honored
		fResults &= VerifyRestriction(iOrdinal, pColumn);

		// If the column requires a set of specific values, make sure
		// it's one of the acceptable values.
		fResults &= VerifyValue(iOrdinal, pColumn);

		// Perform column type-specific verification
		fResults &= VerifyTypeSpecific(iOrdinal, pColumn);

		// Verify the length
		fResults &= VerifyLength(iBind, pColumn);

		// Call the specific validation routine for this particular schema
		fResults &= VerifyColumn(iOrdinal, pColumn);
	}
	else if(pColumn->sStatus==DBSTATUS_S_ISNULL)
	{
		// Some columns may not contain a NULL, validate NULL is
		// OK for this column
		fResults &= VerifyNull(iOrdinal,pColumn);

		// Length should be 0 for NULL
//		COMPARE(pColumn->ulLength, 0); // Per spec, provider ignores length for NULL data

		// Call the specific validation routine for this particular schema
		// Some schema columns have varying instances where they can be NULL
		fResults &= VerifyColumn(iOrdinal, pColumn);
	}
	else
	{
		// A non-success status was returned, flag this as a failure
		// We don't think truncation is valid here
		PRVTRACE(L"%s=",m_rgDBCOLUMNINFO[iBind].pwszName);
		// Post a failure and print the status if not OK
		fResults &= COMPARE(RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus), TRUE);
	}

	return fResults;
}

BOOL CHelper::VerifyNull(DBORDINAL iOrdinal, DATA * pColumn)
{
	BOOL fNull = FALSE;

	// The column's status was DBSTATUS_S_ISNULL, make sure this is OK
	// for this column.

	// If this is a provider-specific column just return TRUE that
	// NULL is allowed.
	if (iOrdinal > m_pSchemaInfo->cColumns)
		return TRUE;

	// Otherwise return the value from our table
	fNull = CompareNull(iOrdinal); 

	return fNull;
}

BOOL CHelper::VerifyNonEmptyString(DATA * pColumn, LPWSTR pwszTable, LPWSTR pwszColumn)
{
	DBSTATUS ulStatus = pColumn->sStatus;
	LPBYTE pValue = pColumn->bValue;
	BOOL fReturn = TRUE;

	if (!pwszTable)
		pwszTable = L"<NoTable>";
	if (!pwszColumn)
		pwszColumn = L"<NoColumn>";

	if (ulStatus == DBSTATUS_S_OK)
	{
		// For now, just make sure the length is right so we read the value
		if (!COMPARE(wcslen((WCHAR *)pValue)*sizeof(WCHAR), pColumn->ulLength))
		{
			odtLog << pwszTable << L": " << pwszColumn << L": " <<
				L"Length is incorrect.\n";
			fReturn=FALSE;
		}

		// We really don't expect empty strings for any of these
		if (!COMPARE(wcslen((WCHAR *)pValue) > 0, TRUE))
		{
			odtLog << pwszTable << L": " << pwszColumn << L": " <<
				L"Empty string returned.\n";
			fReturn=FALSE;
		}
		
	}
	else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
	{
		odtLog << pwszTable << L": " << pwszColumn << L": " <<
			L"Invalid status returned.\n";
		fReturn=FALSE;
	}

	return fReturn;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CHelper::VerifyColumnMatch
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CHelper::VerifyColumnMatch(DBORDINAL iOrdinal1, DBORDINAL iOrdinal2)
{
	LPWSTR pwszCol1 = (LPWSTR)GetValuePtr(iOrdinal1, m_pRow);
	LPWSTR pwszCol2 = (LPWSTR)GetValuePtr(iOrdinal2, m_pRow);

	if (!pwszCol1 && !pwszCol2)
		return TRUE;

	TESTC(pwszCol1 != NULL && pwszCol2 != NULL);

	TESTC(!wcscmp(pwszCol1, pwszCol2));

	return TRUE;

CLEANUP:

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CHelper::VerifyTypeSpecific
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CHelper::VerifyTypeSpecific(DBORDINAL iOrdinal, DATA * pColumn)
{
	DBTYPE wType = m_rgDBCOLUMNINFO[iOrdinal - m_rgDBCOLUMNINFO[0].iOrdinal].wType;
	BOOL fResult = FALSE;

	switch(wType)
	{
		case DBTYPE_WSTR:
			// We expect DBSTATUS_S_OK columns to have a non-empty string

			// Actually, we will comment this out, because there is nothing in the spec to prevent a provider
			// returning an empty string for some columns.
			// TESTC(wcslen((LPWSTR)pColumn->bValue) > 0);
			break;
		case DBTYPE_DATE:
			{
				// We expect a valid date, try to convert to bstr
				VARIANT vDate;
				HRESULT hr = E_FAIL;
				VariantInit(&vDate);
				V_VT(&vDate) = VT_DATE;
				V_DATE(&vDate) = *(DOUBLE *)&pColumn->bValue;
				hr = VariantChangeType(&vDate, &vDate, 0, VT_BSTR);
				VariantClear(&vDate);
				TESTC_(hr, S_OK);
			}
			break;
		default:
			// Do nothing
			break;
	}

	fResult = TRUE;

CLEANUP:

	return fResult;
}


BOOL CHelper::VerifyValue(DBORDINAL iOrdinal, DATA * pColumn)
{
	ULONG cValidVals;
	LPBYTE pValidVals;
	DBTYPE wType;
	ULONG ulTypeSize;
	BOOL fMatch = FALSE;

	// If this is a bookmark column, just return TRUE
	// If this is a provider-specific column that we do not know about then just return TRUE that
	// the value is allowed.
	if (!iOrdinal || iOrdinal > m_pSchemaInfo->cColumns)
		return TRUE;

	// If the column doesn't have a set of defined values, just return TRUE
	if (!m_pSchemaInfo->pColList[iOrdinal-1].cValidVals)
		return TRUE;

	// Get the values from the table
	cValidVals = m_pSchemaInfo->pColList[iOrdinal-1].cValidVals;
	pValidVals = (LPBYTE)m_pSchemaInfo->pColList[iOrdinal-1].pValidVals;
	wType = m_pSchemaInfo->pColList[iOrdinal-1].wType;
	ulTypeSize = GetDBTypeSize(wType);

	// GetDBTypeSize returns 0 for variable length types
	switch(wType)
	{
		case DBTYPE_STR:
			ulTypeSize = sizeof(LPSTR);
			break;
		case DBTYPE_WSTR:
			ulTypeSize = sizeof(LPWSTR);
			break;
	}

	// We must have a non-zero type size
	if (!COMPARE(ulTypeSize > 0, TRUE))
		return FALSE;

	// If there is a count of values, make sure there is a set of values
	// so hopefully we don't crash if the table is bad
	if (!COMPARE(cValidVals && pValidVals, TRUE))
		return FALSE;

	// Otherwise make sure it matches one of the values in our table
	for (ULONG iVal = 0; iVal < cValidVals; iVal++)
	{
		LPBYTE pValidVal = pValidVals;
		DBLENGTH cbBackEnd = ulTypeSize, cbConsumer = ulTypeSize;

		switch(wType)
		{
			case DBTYPE_STR:
				pValidVal = *(LPBYTE *)pValidVals;
				cbBackEnd = strlen((LPSTR)pValidVal);
				cbConsumer = strlen((LPSTR)pColumn->bValue);
			case DBTYPE_WSTR:
				pValidVal = *(LPBYTE *)pValidVals;
				cbBackEnd = wcslen((LPWSTR)pValidVal)*sizeof(WCHAR);
				cbConsumer = wcslen((LPWSTR)pColumn->bValue)*sizeof(WCHAR);
				break;
		}

		if (CompareDBTypeData(&pColumn->bValue, pValidVal, wType, cbBackEnd, 0, 0, NULL, FALSE, DBTYPE_EMPTY, cbConsumer))
		{
			fMatch = TRUE;
			break;
		}
		pValidVals += ulTypeSize;
	}

	// Post a failure if we didn't find a match.
	// Providers are allowed to return provider
	if (!fMatch)
	{
		if (*(m_pSchemaInfo->pguid) == DBSCHEMA_TABLES &&
 		iOrdinal == TABLES_TABLE_TYPE)
		{
			// Providers are allowed to return provider specific values for TABLE_TYPE, so 
			// we can only post a warning if the value doesn't match expected TABLE_TYPE's
			COMPAREW(fMatch, TRUE);
		}
		else
			COMPARE(fMatch, TRUE);

		odtLog << L"The value returned for column " << m_pSchemaInfo->pColList[iOrdinal-1].pwszName <<
			L" was not in the list of valid values.\n";
	}

	return fMatch;
}

BOOL CHelper::VerifyRestriction(DBORDINAL iOrdinal, DATA * pColumn)
{
	// If this is a restriction column, make sure it matches expected

	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;

	// If we didn't pass any restrictions, just return TRUE
	if (!m_prgRestrictUsed)
		return TRUE;

	// If this is the bookmark column just return true
	if (!iOrdinal)
		return TRUE;

	// If this is not a restriction column for this schema just return TRUE
	for (iRestrict = 0; iRestrict < m_pSchemaInfo->cRestrictions; iRestrict++)
	{
		// Don't check restrictions we didn't actually use
		if (iRestrict >= m_cRestrictionsUsed)
			break;

		if (iOrdinal == m_pSchemaInfo->pulRestrictions[iRestrict])
		{
			// This is a restriction column, is it restricted?
			if (V_VT(&m_prgRestrictUsed[iRestrict]) == VT_EMPTY)
				return TRUE;

			// Convert column value to a variant
			pVariant = DBTYPE2VARIANT(&pColumn->bValue, m_pSchemaInfo->pColList[iOrdinal-1].wType);

			TESTC(pVariant != NULL);

			// Compare the variants.  Use case insensitive compare for BSTR restrictions
			TESTC(CompareVariant(pVariant, &m_prgRestrictUsed[iRestrict], FALSE));			

			// We found the restriction column, break out of the loop
			break;
		}
	}

	fResult = TRUE;

CLEANUP:

	if (pVariant)
		VariantClear(pVariant);
	SAFE_FREE(pVariant);

	return TRUE;
}

HRESULT CHelper::GetColumnInfo(LPBYTE pData, DBBINDING * pBinding)
{
	IOpenRowset * pIOpenRowset = NULL;
	IColumnsInfo * pIColumnsInfo = NULL;
	DBID dbid = DB_NULLID;
	HRESULT hr = E_FAIL;
	ULONG iBind = 0, iFirst = 0;
	LPWSTR pwszCat = NULL;
	LPWSTR pwszSch = NULL;
	LPWSTR pwszTable = NULL;
	LPWSTR pwszQualifiedName = NULL;
	DBPROPSET * prgPropSets = NULL;
	ULONG cPropSets = 0;


	// pData buffer contains bindings to TABLE_CATALOG, TABLE_SCHEMA, and TABLE_NAME, in that 
	// order.
	
	// Skip any bookmark
	if (!pBinding[iBind].iOrdinal)
		iFirst++;

	for (iBind = 0; iBind < 3; iBind++)
	{
		// If value isn't bound then we assume NULL
		if (!VALUE_IS_BOUND(pBinding[iBind+iFirst]))
			continue;

		// Get a pointer to catalog, schema, and table name
		if (STATUS_IS_BOUND(pBinding[iBind+iFirst]))
		{
			// Check the status, which should always be OK or NULL
			if (STATUS_BINDING(pBinding[iBind+iFirst], pData) != DBSTATUS_S_OK)
			{
				COMPARE(STATUS_BINDING(pBinding[iBind+iFirst], pData), DBSTATUS_S_ISNULL);
				continue;
			}
		}

		// If status wasn't bound we assume OK, and length doesn't matter for null terminated string
		switch(pBinding[iBind+iFirst].iOrdinal)
		{
			case 1:
				pwszCat = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			case 2:
				pwszSch = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			case 3:
				pwszTable = (LPWSTR)&VALUE_BINDING(pBinding[iBind+iFirst], pData);
				break;
			default:
				COMPARE(pBinding[iBind+iFirst].iOrdinal < 4, TRUE);
				COMPARE(pBinding[iBind+iFirst].iOrdinal > 0, TRUE);
		}
	}

	// HACK FOR ORACLE
	if (pwszSch && wcslen(pwszSch) == 0)
		pwszSch = NULL;

	// Now we have either NULL or valid pointers to catalog, schema, table names
	TESTC_(m_pTable->GetQualifiedName(pwszCat, pwszSch, 
							pwszTable,&pwszQualifiedName, TRUE), S_OK);

	// If we already have information for this table then just don't bother
	if (m_pwszTableName && !wcscmp(pwszQualifiedName, m_pwszTableName))
	{
		m_ulTableOrdinal++;

		// If we couldn't actually retrieve the colinfo for this table then
		// return S_FALSE
		if (!m_prgColInfo)
			return S_FALSE;
		// Otherwise return success
		else
			return S_OK;
	}

	// Copy the table name into member var
	SAFE_FREE(m_pwszTableName);
	m_pwszTableName = pwszQualifiedName; 

	// Initialize out params
	m_ulTableOrdinal = 1;	// Note that schema rowset never reports bookmarks
	SAFE_FREE(m_prgColInfo);
	SAFE_FREE(m_pwszStringsBuffer);

	// Get IOpenRowset interface
	TESTC(VerifyInterface(m_pIDBSchemaRowset, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset));

	// Create DBID from table name
	dbid.eKind = DBKIND_NAME;
	dbid.uName.pwszName = m_pwszTableName;

	// When retrieving column information for TABLES_INFO rowset we want a bookmark also
	if (m_guidSchema == DBSCHEMA_TABLES_INFO && SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET,
		m_pCSchemaTest->m_pIDBInitialize, ROWSET_INTERFACE))
	{
		TESTC_(SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET,
			&cPropSets, &prgPropSets, DBTYPE_BOOL, VARIANT_TRUE), S_OK);
	}

	// Call OpenRowset
	hr = pIOpenRowset->OpenRowset(NULL, &dbid, NULL, IID_IColumnsInfo, 
		cPropSets, prgPropSets, (IUnknown **)&pIColumnsInfo);

	if (FAILED(hr))
		odtLog << m_pwszTableName << L": Unable to open table.\n";

	// It's possible the table was deleted between the time we called GetRowset and attempting
	// to open the table, so we have to allow DB_E_NOTABLE, but post a warning.
	// It's also possible we don't have permission for the table.
	switch(hr)
	{
		case DB_E_NOTABLE:
		case DB_SEC_E_PERMISSIONDENIED:
			// Only post the warning when debugmode is FULL
			if (GetModInfo()->GetDebugMode() & DEBUGMODE_FULL)
				TESTW_(hr, S_OK);
			goto CLEANUP;
		default: 
			TESTC_(hr, S_OK);
	}

	// Make sure we've got a valid interface ptr
	TESTC(pIColumnsInfo != NULL);

	// Now call GetColumnInfo.  We already know cColumns so no need to return in member
	// var.
	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&m_cColInfo, &m_prgColInfo, &m_pwszStringsBuffer), S_OK);

CLEANUP:

	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIColumnsInfo);

	return hr;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowset, execute method and check results
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT CHelper::GetRowsetHelper
(
	IUnknown *		pIUnknown,
	ULONG			cRestrictions,
	VARIANT	*		prgRestrictions,
	REFIID			riid,
	ULONG			cPropertySets,
	DBPROPSET		rgPropertySets[],
	IUnknown **		ppRowset,
	GUID			guidSchema,
	BOOL fFreeRowset
)
{
	HRESULT hr = E_FAIL;
	IUnknown * pIRowsetUnknown = NULL;

	if (m_fUseCommonArgumentDefaults)
	{
		// If we were not passed a valid schema then
		// just use the one for our helper class.
		if (guidSchema == GUID_NULL)
			guidSchema = m_guidSchema;

		switch(m_eRestrictArray)
		{
			case RT_CACHED:
				prgRestrictions = m_rgRestrict;
				break;
			case RT_SUPPORTED:
				prgRestrictions = m_rgRestrictValid;
				break;
			case RT_CUSTOM:
				prgRestrictions = m_rgRestrictUsed;
				break;
			default:
				COMPARE(0, 1);
				return E_FAIL;
		}

		// Set desired restrictions
		switch(cRestrictions)
		{
			// Pass a 0, NULL restriction
			case RV_NULL:
				cRestrictions = 0;
				prgRestrictions = NULL;
				break;
			// Pass all restrictions
			case RV_ALL:
				cRestrictions = m_pSchemaInfo->cRestrictions;
				break;
//			default:
//				COMPARE(cRestrictions <= m_pSchemaInfo->cRestrictions, TRUE);
		}


		// If not passed a rowset pointer, just use an internal one
		if (!ppRowset)
			ppRowset = (IUnknown **)&pIRowsetUnknown;
	}

	// Track which set of restrictions was actually used so we can validate them
	m_prgRestrictUsed = prgRestrictions;
	m_cRestrictionsUsed = cRestrictions;

	// Now get Schema rowset.
	hr=m_pIDBSchemaRowset->GetRowset(pIUnknown, guidSchema, cRestrictions, prgRestrictions,
		riid, cPropertySets, rgPropertySets, ppRowset);

	if (ppRowset)
		pIRowsetUnknown = *ppRowset;

	// Check results of GetRowset
	CheckResults(hr, riid, pIRowsetUnknown);

	if (fFreeRowset && ppRowset)
		SAFE_RELEASE(*ppRowset);

	return hr;
}


// Assertions schema specific functions
class CAssertionsHelper : public CHelper
{ 

private:


protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CAssertionsHelper(void) {m_guidSchema = DBSCHEMA_ASSERTIONS;}

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CAssertionsHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CAssertionsHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	BOOL fResults = FALSE;

	switch(iOrdinal)
	{
	case 1:// CONSTRAINT CATALOG

		PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue);
		break;
	case 2://CONSTRAINT SCHEMA

		PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue);
		break;
	case 3://CONSTRAINT NAME
		PRVTRACE(L"= %s\n",(WCHAR *)pColumn->bValue);
		break;
	case 4: //IS_DEFERRABLE
	case 5: //INITIALLY_DEFERRED
		if(*(TYPE_BOOL *)pColumn->bValue==VARIANT_TRUE)
			PRVTRACE(L"TRUE\n");
		else
			PRVTRACE(L"FALSE\n");
		break;
	case 6: // DESCRIPTION
			PRVTRACE(L"= %s\n",(TYPE_WSTR)pColumn->bValue );
			break;
	default:
		break;
	}
			
	return fResults;

}


// Catalogs schema specific functions
class CCatalogsHelper : public CHelper
{ 

private:


protected:

public:

	CCatalogsHelper(void) {m_guidSchema = DBSCHEMA_CATALOGS;}
};

// Catalogs schema specific functions
class CLargeRestrictionCatalogsHelper : public CCatalogsHelper
{ 

private:


protected:

		virtual BOOL VerifyRestriction(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CLargeRestrictionCatalogsHelper(void) {}
};


BOOL CLargeRestrictionCatalogsHelper::VerifyRestriction(DBORDINAL iOrdinal, DATA * pColumn)
{
	return CCatalogsHelper::VerifyRestriction(iOrdinal, pColumn);
}

// Character Sets schema specific functions
class CCharacterSetsHelper : public CHelper
{ 

private:


protected:

public:

	CCharacterSetsHelper(void) {m_guidSchema = DBSCHEMA_CHARACTER_SETS;}
};

// Check Constraints schema specific functions
class CCheckConstraintsHelper : public CHelper
{ 

private:


protected:
	BOOL InsertRows(void);

public:

	CCheckConstraintsHelper(void) {m_guidSchema = DBSCHEMA_CHECK_CONSTRAINTS;}
};

BOOL CCheckConstraintsHelper::InsertRows(void)
{
	return CreateCheckConstraints();
}


// CheckConstraintsByTable schema specific functions
class CCheckConstraintsByTableHelper : public CHelper
{ 

private:


protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL InsertRows(void);

public:

	CCheckConstraintsByTableHelper(void) {m_guidSchema = DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE;}

};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CCheckConstraintsByTableHelper::InsertRows
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCheckConstraintsByTableHelper::InsertRows(void)
{
	return CreateCheckConstraints();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CCheckConstraintsByTableHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCheckConstraintsByTableHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	BOOL fResults = TRUE; // For now

	switch(iOrdinal)
	{
		case CCBTS_TABLE_CATALOG:
			// Verify matches DBPROPSET_DATASOURCE, DBPROP_CURRENTCATALOG
			break;
		case CCBTS_TABLE_SCHEMA:
			// Verify matches DBPROPSET_DATASOURCEINFO, DBPROP_USERNAME when tablename
			// is automaketable name
			break;
		case CCBTS_TABLE_NAME:
			break;
		case CCBTS_CONSTRAINT_CATALOG:
			// Validate matches TABLE_CATALOG?
		case CCBTS_CONSTRAINT_SCHEMA:
			// Validate matches TABLE_CATALOG?
		case CCBTS_CONSTRAINT_NAME:
			// Validate non-NULL
		case CCBTS_CHECK_CLAUSE:
			// Validate this matches check clause we created for automaketable
		case CCBTS_DESCRIPTION:
			break;
		default:
			// Provider-specific column, can't validate
			break;
	}

	return fResults;

}

// Collations schema specific functions
class CCollationsHelper : public CHelper
{ 

private:


protected:

public:

	CCollationsHelper(void) {m_guidSchema = DBSCHEMA_COLLATIONS;}
};

// Column Domain Usage schema specific functions
class CColumnDomainUsageHelper : public CHelper
{ 

private:


protected:

public:

	CColumnDomainUsageHelper(void) {m_guidSchema = DBSCHEMA_COLUMN_DOMAIN_USAGE;}
};

// Column Privileges schema specific functions
class CColumnPrivilegesHelper : public CHelper
{ 

private:


protected:

public:

	CColumnPrivilegesHelper(void) {m_guidSchema = DBSCHEMA_COLUMN_PRIVILEGES;}
};

// Columns schema specific functions
class CColumnsHelper : public CHelper
{ 

private:

	CErrorCache		m_EC;	// Columns error cache

protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CColumnsHelper(void) {m_guidSchema = DBSCHEMA_COLUMNS;}
	BOOL Init(CSchemaTest * pThis);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CColumnsHelper::Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CColumnsHelper::Init(CSchemaTest * pThis)
{
	if (CHelper::Init(pThis))
	  return m_EC.Init(GetModInfo()->GetDebugMode());
	else
		return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CColumnsHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CColumnsHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	LPWSTR pwszSchemaName	= (LPWSTR)GetValuePtr(COLS_TABLE_SCHEMA, m_pRow);
	LPWSTR pwszTableName	= (LPWSTR)GetValuePtr(COLS_TABLE_NAME, m_pRow);
	LPWSTR pwszColumnName	= (LPWSTR)GetValuePtr(COLS_COLUMN_NAME, m_pRow);
	LPWSTR pwszColumnDefault = (LPWSTR)GetValuePtr(COLS_COLUMN_DEFAULT, m_pRow);
	ULONG * pulOrdinal		= (ULONG *)GetValuePtr(COLS_ORDINAL_POSITION, m_pRow);
	LPBYTE pValue			= GetValuePtr(iOrdinal, m_pRow);
	BOOL fResult			= FALSE;
	ULONG iIndex			= 0;
	DBSTATUS ulStatus		= pColumn->sStatus;
	DBTYPE wType			= 0;

	// Compute index into colinfo given this ordinal
	if (m_prgColInfo && pulOrdinal)
		iIndex = *pulOrdinal + !m_prgColInfo[0].iOrdinal - 1;

	// Need the column name to verify
	// Note: We may be able to get by without this by computing the index via
	// counting the rows in this table, but since providers don't have to return
	// rows in ordinal order this is likely to be wrong.
	TESTC(pwszColumnName != NULL);

	// If we don't have the right index we can't verify
	if (m_prgColInfo && m_prgColInfo[iIndex].pwszName &&
		RelCompareString(pwszColumnName, m_prgColInfo[iIndex].pwszName))
	{
		// Names didn't match, so we have to search for the index
		for (iIndex = 0; iIndex < m_cColInfo; iIndex++)
			if (!RelCompareString(pwszColumnName, m_prgColInfo[iIndex].pwszName))
				break;
	}

	// Make sure we found the matching index.  If we didn't get m_prgColInfo then
	// we don't index into it below.
	if (m_prgColInfo)
	{
		TESTC(iIndex < m_cColInfo);

		wType = m_prgColInfo[iIndex].wType;
	}
	else
		wType = (DBTYPE)GetValuePtr(COLS_DATA_TYPE, m_pRow);

	switch(iOrdinal)
	{
			case COLS_TABLE_CATALOG:
				break;

			case COLS_TABLE_SCHEMA:
				break;

			case COLS_TABLE_NAME:
				break;

			case COLS_COLUMN_NAME:
				break;

			case COLS_COLUMN_GUID:
				break;
			case COLS_TYPE_GUID:
				break;
			case COLS_COLUMN_PROPID:	
				break;
			case COLS_ORDINAL_POSITION:
				// Status is OK, warning if ordinals don't match
				// Providers are not required to return results in ordinal order, since
				// sort is only on catalog, schema, and name, so only a warning.
				// If restriction is only on column then the ordinal may not match anyway.

				// We will assume if the index for the matching ordinal contains
				// the proper column name that it was the right ordinal.
				if (pulOrdinal && m_prgColInfo && !COMPARE(*pulOrdinal, m_prgColInfo[iIndex].iOrdinal))
					odtLog << pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
						L"Ordinal didn't match expected value.\n";
				break;
			case COLS_COLUMN_FLAGS:
				if (m_prgColInfo)
				{
					COMPARE(m_prgColInfo[iIndex].dwFlags, *(DBCOLUMNFLAGS *)pValue);
				}
				break;
			case COLS_CHARACTER_MAXIMUM_LENGTH:
				if (m_prgColInfo)
				{
					switch(m_prgColInfo[iIndex].wType)
					{
	//					case DBTYPE_BOOL:  // We assume BOOL is not a "bit" column per spec.
						case DBTYPE_UDT:
						case DBTYPE_BYTES:
						case DBTYPE_STR:
						case DBTYPE_WSTR:
							if (COMPARE(ulStatus, DBSTATUS_S_OK))
							{
								if (m_prgColInfo[iIndex].ulColumnSize == ~0)
								{
									if (!COMPARE(*(ULONG *)pValue, 0))
										odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
											L"Must be 0 for column without defined max length.\n";
								}
								else 
									CCOMPARE(m_EC, *(ULONG *)pValue == m_prgColInfo[iIndex].ulColumnSize, 
										EC_BAD_CHARACTER_MAXIMUM_LENGTH,
										L"CHARACTER_MAXIMUM_LENGTH value did not match GetColumnInfo",
										FALSE);
							}
							else
								odtLog << pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
									L"Cannot be NULL for variable length column.\n";
							break;
						// NULL for all other data types
						default:
							if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
								odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
									L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
					}
				}
				break;
			case COLS_CHARACTER_OCTET_LENGTH:
				if (m_prgColInfo)
				{
					switch(m_prgColInfo[iIndex].wType)
					{
						case DBTYPE_UDT:
						case DBTYPE_BYTES:
						case DBTYPE_STR:
						case DBTYPE_WSTR:
							if (COMPARE(ulStatus, DBSTATUS_S_OK))
							{
								if (m_prgColInfo[iIndex].ulColumnSize == ~0)
								{
									if (!COMPARE(*(ULONG *)pValue, 0))
										odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
											L"Must be 0 for column without defined max length.\n";
								}
								else 
								{
									DBLENGTH ulOctetLen = m_prgColInfo[iIndex].ulColumnSize;
									if (m_prgColInfo[iIndex].wType == DBTYPE_WSTR)
										ulOctetLen *= sizeof(WCHAR);

									CCOMPARE(m_EC, ulOctetLen == *(ULONG *)pValue, 
										EC_BAD_CHARACTER_OCTET_LENGTH,
										L"CHARACTER_OCTET_LENGTH value did not match GetColumnInfo",
										FALSE);
								}
							}
							break;
						// NULL for all other data types
						default:
							if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
								odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
									L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
					}
				}
				break;
			case COLS_DATETIME_PRECISION:
				if (m_prgColInfo)
				{
					switch(m_prgColInfo[iIndex].wType)
					{
						case DBTYPE_DBTIMESTAMP:
						// The following types are new for 9.0 and do support DATETIME_PRECISION per spec
#ifdef __UTC_
						case DBTYPE_DBUTCDATETIME:
						case DBTYPE_DBTIME_EX:
						case DBTYPE_DBDATE:
#endif
							// DATETIME_PRECISION matches IColumnsInfo bScale vaue
							if (COMPARE(ulStatus, DBSTATUS_S_OK) &&
								!COMPARE((ULONG)m_prgColInfo[iIndex].bScale, *(ULONG *)pValue))
/*

								CCOMPARE(m_EC, *(ULONG *)pValue == (ULONG)m_prgColInfo[iIndex].bScale, 
									EC_BAD_DATETIME_PRECISION,
									L"DATETIME_PRECISION value did not match GetColumnInfo",
									FALSE);
*/
								odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L"DATETIME_PRECISION value did not match GetColumnInfo" << L" \n";
							break;
						// NULL for all other data types
						default:
							if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
								odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
									L"Status was not DBSTATUS_S_ISNULL for non-char, binary, or bit column\n";
					}
				}
				break;
			case COLS_COLUMN_HASDEFAULT:
				// We can't actually try the default value except for our test table, so we don't know
				// if it actually would work or not for other tables.
				// Note that COLUMN_DEFAULT may be NULL even if HASDEFAULT is TRUE but the converse
				// is not true.
				
				if (COMPARE(ulStatus, DBSTATUS_S_OK))
				{
					// If the COLUMN_DEFAULT is non-NULL, then HASDEFAULT should be TRUE, otherwise
					// we're not sure
					if (pwszColumnDefault && !COMPARE(*(VARIANT_BOOL *)pValue, VARIANT_TRUE))
						odtLog << pwszTableName << L": " << rgwszCOLUMNS[iOrdinal-1] << L": " <<
							L"Invalid value returned.\n";

					// TODO: If the current table is the automaketable for this test try to use the default
					// value and make sure it matches expected.
				}
				else
					odtLog << pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
						L"Invalid status returned.\n";
				break;
			case COLS_COLUMN_DEFAULT:
				if (ulStatus == DBSTATUS_S_OK)
				{
					// For now, just make sure the length is right so we read the default value
					if (!COMPARE(wcslen(pwszColumnDefault)*sizeof(WCHAR), pColumn->ulLength))
						odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
							L"Default value has the wrong length.\n";

					// TODO: If the current table is the automaketable for this test try to use the default
					// value and make sure it matches expected.  
				}
				break;
			case COLS_IS_NULLABLE:
				if (m_prgColInfo)
				{
					DBCOLUMNFLAGS dwNullableFlag = m_prgColInfo[iIndex].dwFlags & DBCOLUMNFLAGS_ISNULLABLE;
					DBCOLUMNFLAGS dwISNULLABLEFlag = *(VARIANT_BOOL *)pValue == VARIANT_TRUE ? DBCOLUMNFLAGS_ISNULLABLE : 0;
					if (!COMPARE(dwNullableFlag, dwISNULLABLEFlag))
						odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
							L"DBCOLUMNFLAGS_ISNULLABLE incorrect value returned.\n";
/*
					// Turn this column's value into DBCOLUMNFLAGS and use CompareColumnFlags


					VARIANT_BOOL fNullable = m_prgColInfo[iIndex].dwFlags & DBCOLUMNFLAGS_ISNULLABLE ? VARIANT_TRUE : VARIANT_FALSE;
					if (!COMPARE(fNullable, *(VARIANT_BOOL *)pValue))
						odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
							L"DBCOLUMNFLAGS_ISNULLABLE incorrect value returned.\n";
*/
				}
				break;
			case COLS_DATA_TYPE:
				// This column cannot contain NULL
				if (COMPARE(ulStatus, DBSTATUS_S_OK) && m_prgColInfo &&
					!COMPARE(m_prgColInfo[iIndex].wType,	*(DBTYPE *)pValue))
					odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
						L"Invalid value returned for data type. Expected: " << m_prgColInfo[iIndex].wType <<
						L" Received: " << *(DBTYPE *)pValue << L" \n";
				break;
			case COLS_NUMERIC_PRECISION:
				if (m_prgColInfo)
				{
					if (IsNumericType(m_prgColInfo[iIndex].wType))
					{
						if (COMPARE(ulStatus, DBSTATUS_S_OK))
						{
							CCOMPARE(m_EC, *(USHORT *)pValue == (USHORT)m_prgColInfo[iIndex].bPrecision, 
								EC_COLUMNS_BAD_NUMERIC_PRECISION,
								L"NUMERIC_PRECISION is incorrect",
								FALSE);
						}
					}
					// NULL for all other data types
					else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
						odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
							L"Status was not DBSTATUS_S_ISNULL for non-numeric column\n";
				}
				break;
			case COLS_NUMERIC_SCALE:
				if (m_prgColInfo)
				{
					if (m_prgColInfo[iIndex].wType == DBTYPE_DECIMAL ||
						m_prgColInfo[iIndex].wType == DBTYPE_NUMERIC ||
						m_prgColInfo[iIndex].wType == DBTYPE_VARNUMERIC)
					{
						
						CCOMPARE(m_EC, ulStatus == DBSTATUS_S_OK, 
							EC_COLUMNS_NULL_NUMERIC_SCALE,
							L"NUMERIC_SCALE cannot be NULL for numeric column",
							FALSE);

						if (ulStatus==DBSTATUS_S_OK)
						{
							SHORT sScale = (SHORT)m_prgColInfo[iIndex].bScale;

							// Account for negative scale values
							if (m_prgColInfo[iIndex].dwFlags & DBCOLUMNFLAGS_SCALEISNEGATIVE)
								sScale = -sScale;

							if (!COMPARE(*(SHORT *)pValue, sScale))
								odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
									L"Invalid value returned.\n";
						}
					}
					// NULL for all other data types
					else if (!COMPARE(ulStatus, DBSTATUS_S_ISNULL))
						odtLog << m_pwszTableName << L": " << m_prgColInfo[iIndex].pwszName << L": " <<
							L"Status was not DBSTATUS_S_ISNULL for non-DECIMAL, NUMERIC column\n";
				}
				break;
			case COLS_CHARACTER_SET_CATALOG:
			case COLS_CHARACTER_SET_SCHEMA:
			case COLS_CHARACTER_SET_NAME:
			case COLS_COLLATION_CATALOG:
			case COLS_COLLATION_SCHEMA:
			case COLS_COLLATION_NAME:
			case COLS_DOMAIN_CATALOG:
			case COLS_DOMAIN_SCHEMA:
			case COLS_DOMAIN_NAME:
				VerifyNonEmptyString(pColumn, m_pwszTableName, m_prgColInfo ? m_prgColInfo[iIndex].pwszName: L"<Unknown>");
				break;
			case COLS_DESCRIPTION:
				break;
		}

	fResult = TRUE;

CLEANUP:

	return fResult;

}

// Constraint Column Usage schema specific functions
class CConstraintTableUsageHelper : public CHelper
{ 

private:

protected:

public:

	CConstraintTableUsageHelper(void) {m_guidSchema = DBSCHEMA_CONSTRAINT_TABLE_USAGE;}
};

// Constraint Column Usage schema specific functions
class CConstraintColumnUsageHelper : public CHelper
{ 

private:


protected:
	BOOL InsertRows(void);

public:

	CConstraintColumnUsageHelper(void) {m_guidSchema = DBSCHEMA_CONSTRAINT_COLUMN_USAGE;}
};

BOOL CConstraintColumnUsageHelper::InsertRows(void)
{
	return CreateCheckConstraints();
}

// Foreign Keys schema specific functions
class CForeignKeysHelper : public CHelper
{ 

private:


protected:

public:

	CForeignKeysHelper(void) {m_guidSchema = DBSCHEMA_FOREIGN_KEYS;}
};

// Indexes schema specific functions
class CIndexesHelper : public CHelper
{ 

private:

	CErrorCache		m_EC;	// Indexes error cache

protected:
	BOOL CompareNull(DBORDINAL iOrdinal);

public:

	CIndexesHelper(void) {m_guidSchema = DBSCHEMA_INDEXES;}
	BOOL Init(CSchemaTest * pThis);
};

BOOL CIndexesHelper::CompareNull(DBORDINAL iOrdinal)
{
	switch (iOrdinal)
	{
		case IS_NULLS:
			return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
				EC_INDEXES_NULLS_ISNULL,
				L"NULLS column cannot be DBSTATUS_S_ISNULL",
				FALSE);
		case IS_INDEX_NAME:
			return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
				EC_INDEXES_INDEX_NAME_ISNULL,
				L"INDEX_NAME column cannot be DBSTATUS_S_ISNULL",
				FALSE);
		case IS_UNIQUE:
			return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
				EC_INDEXES_UNIQUE_ISNULL,
				L"UNIQUE column cannot be DBSTATUS_S_ISNULL",
				FALSE);
		case IS_AUTO_UPDATE:
			return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
				EC_INDEXES_AUTO_UPDATE_ISNULL,
				L"AUTO_UPDATE column cannot be DBSTATUS_S_ISNULL",
				FALSE);

	}

	return CHelper::CompareNull(iOrdinal);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CIndexesHelper::Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CIndexesHelper::Init(CSchemaTest * pThis)
{
	if (CHelper::Init(pThis))
	  return m_EC.Init(GetModInfo()->GetDebugMode());
	else
		return FALSE;
}

// Key Column Usage schema specific functions
class CKeyColumnUsageHelper : public CHelper
{ 

private:


protected:


public:

	CKeyColumnUsageHelper(void) {m_guidSchema = DBSCHEMA_KEY_COLUMN_USAGE;}
};

// Primary Keys schema specific functions
class CPrimaryKeysHelper : public CHelper
{ 

private:


protected:

public:

	CPrimaryKeysHelper(void) {m_guidSchema = DBSCHEMA_PRIMARY_KEYS;}
};

// Procedures schema specific functions
class CProceduresHelper : public CHelper
{ 

private:
	CErrorCache		m_EC;	// Error cache

protected:
	BOOL CompareNull(DBORDINAL iOrdinal);

public:

	CProceduresHelper(void) {m_guidSchema = DBSCHEMA_PROCEDURES;}
	BOOL Init(CSchemaTest * pThis);
};

BOOL CProceduresHelper::CompareNull(DBORDINAL iOrdinal)
{
	if (iOrdinal == PRS_PROCEDURE_DEFINITION)
		return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
			EC_PROCEDURES_PROC_DEF_ISNULL,
			L"PROCEDURE_DEFINITION column cannot be DBSTATUS_S_ISNULL",
			FALSE);
	else
		return CHelper::CompareNull(iOrdinal);
}

BOOL CProceduresHelper::Init(CSchemaTest * pThis)
{
	if (CHelper::Init(pThis))
	  return m_EC.Init(GetModInfo()->GetDebugMode());
	else
		return FALSE;
}


// Procedure Columns schema specific functions
class CProcedureColumnsHelper : public CHelper
{ 

private:
	CErrorCache		m_EC;	// Error cache

protected:
	BOOL CompareNull(DBORDINAL iOrdinal);
	BOOL VerifyValue(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CProcedureColumnsHelper(void) {m_guidSchema = DBSCHEMA_PROCEDURE_COLUMNS;}
	BOOL Init(CSchemaTest * pThis);
};

BOOL CProcedureColumnsHelper::CompareNull(DBORDINAL iOrdinal)
{
	if (iOrdinal == PCS_COLS_IS_NULLABLE)
		return CCOMPARE(m_EC, m_pSchemaInfo->pColList[iOrdinal-1].fAllowNull == TRUE, 
			EC_PROC_COLS_IS_NULLABLE_ISNULL,
			L"IS_NULLABLE column cannot be DBSTATUS_S_ISNULL",
			FALSE);
	else
		return CHelper::CompareNull(iOrdinal);
}

BOOL CProcedureColumnsHelper::Init(CSchemaTest * pThis)
{
	if (CHelper::Init(pThis))
	  return m_EC.Init(GetModInfo()->GetDebugMode());
	else
		return FALSE;
}

BOOL CProcedureColumnsHelper::VerifyValue(DBORDINAL iOrdinal, DATA * pColumn)
{
	ULONG cValidVals;
	LPBYTE pValidVals;
	DBTYPE wType;
	ULONG ulTypeSize;
	BOOL fMatch = FALSE;

	// If this is a bookmark column, just return TRUE
	// If this is a provider-specific column just return TRUE that
	// the value is allowed.
	if (!iOrdinal || iOrdinal > m_pSchemaInfo->cColumns)
		return TRUE;

	// If the column doesn't have a set of defined values, just return TRUE
	if (!m_pSchemaInfo->pColList[iOrdinal-1].cValidVals)
		return TRUE;

	// Get the values from the table
	cValidVals = m_pSchemaInfo->pColList[iOrdinal-1].cValidVals;
	pValidVals = (LPBYTE)m_pSchemaInfo->pColList[iOrdinal-1].pValidVals;
	wType = m_pSchemaInfo->pColList[iOrdinal-1].wType;
	ulTypeSize = GetDBTypeSize(wType);

	// GetDBTypeSize returns 0 for variable length types
	switch(wType)
	{
		case DBTYPE_STR:
			ulTypeSize = sizeof(LPSTR);
			break;
		case DBTYPE_WSTR:
			ulTypeSize = sizeof(LPWSTR);
			break;
	}

	// We must have a non-zero type size
	if (!COMPARE(ulTypeSize > 0, TRUE))
		return FALSE;

	// If there is a count of values, make sure there is a set of values
	// so hopefully we don't crash if the table is bad
	if (!COMPARE(cValidVals && pValidVals, TRUE))
		return FALSE;

	// Otherwise make sure it matches one of the values in our table
	for (ULONG iVal = 0; iVal < cValidVals; iVal++)
	{
		LPBYTE pValidVal = pValidVals;
		DBLENGTH cbBackEnd = ulTypeSize, cbConsumer = ulTypeSize;

		switch(wType)
		{
			case DBTYPE_STR:
				pValidVal = *(LPBYTE *)pValidVals;
				cbBackEnd = strlen((LPSTR)pValidVal);
				cbConsumer = strlen((LPSTR)pColumn->bValue);
			case DBTYPE_WSTR:
				pValidVal = *(LPBYTE *)pValidVals;
				cbBackEnd = wcslen((LPWSTR)pValidVal)*sizeof(WCHAR);
				cbConsumer = wcslen((LPWSTR)pColumn->bValue)*sizeof(WCHAR);
				break;
		}

		if (CompareDBTypeData(&pColumn->bValue, pValidVal, wType, cbBackEnd, 0, 0, NULL, FALSE, DBTYPE_EMPTY, cbConsumer))
		{
			fMatch = TRUE;
			break;
		}
		pValidVals += ulTypeSize;
	}

	// Post a failure if we didn't find a match
	if (iOrdinal == PCS_COLS_IS_NULLABLE)
	{
		// Only post this failure once
		CCOMPARE(m_EC, fMatch == TRUE, 
			EC_INVALID_IS_NULLABLE,
			L"IS_NULLABLE value was not in list of valid values",
			FALSE);
	}
	else if (!COMPARE(fMatch, TRUE))
		odtLog << L"The value returned for column " << m_pSchemaInfo->pColList[iOrdinal-1].pwszName <<
			L" was not in the list of valid values.\n";

	return fMatch;
}


// Procedure Parameters schema specific functions
class CProcedureParametersHelper : public CHelper
{ 
private:
	LPWSTR m_pwszQualifiedName;
	DB_UPARAMS m_cParams;
	DB_UPARAMS m_cPropParams;
	DBPARAMINFO * m_pParamInfo;
	LPWSTR m_pNamesBuffer;
	SHORT m_iOrdinalPrev;
	UWORD m_iOrdinalStart;

protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL GetParamInfo(void);
	BOOL InsertRows(void);

public:
	CProcedureParametersHelper(void)
	{
		m_guidSchema = DBSCHEMA_PROCEDURE_PARAMETERS;
		m_pwszQualifiedName = NULL;
		m_pParamInfo = NULL;
		m_pNamesBuffer = NULL;
	}

	~CProcedureParametersHelper(void)
	{
		SAFE_FREE(m_pwszQualifiedName);
		SAFE_FREE(m_pParamInfo);
		SAFE_FREE(m_pNamesBuffer);
	}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CProcedureParametersHelper::InsertRows
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CProcedureParametersHelper::InsertRows(void)
{
	return CreateStoredProcs();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CProcedureParametersHelper::GetParamInfo
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CProcedureParametersHelper::GetParamInfo(void)
{
	BOOL fRet = FALSE;
	LPWSTR pwszProcCat		= (LPWSTR)GetValuePtr(PPS_PROCEDURE_CATALOG, m_pRow);
	LPWSTR pwszProcSchema	= (LPWSTR)GetValuePtr(PPS_PROCEDURE_SCHEMA, m_pRow);
	LPWSTR pwszProcName		= (LPWSTR)GetValuePtr(PPS_PROCEDURE_NAME, m_pRow);
	DBTYPE * pwType			= (DBTYPE *)GetValuePtr(PPS_DATA_TYPE, m_pRow);
	UWORD * piOrdinal		= (UWORD *)GetValuePtr(PPS_ORDINAL_POSITION, m_pRow);
	LPWSTR pwszQualifiedName= NULL;
	IDBCreateSession *		pIDBCreateSession = NULL;
	IDBCreateCommand *		pIDBCreateCommand = NULL;
	ICommand *				pICmd = NULL;
	ICommandWithParameters *  pICmdWPar = NULL;
	ICommandText *			pICmdText = NULL;
	ICommandPrepare *		pICmdPrep = NULL;
	LPWSTR pwszCmdFormat1	= L"{call %s}";		// {call procname}
	LPWSTR pwszCmdFormat2	= L"{call %s(%s)}"; // {call procname(procparams)}
	LPWSTR pwszCmdFormat1r	= L"{?=call %s}";		// {call procname}
	LPWSTR pwszCmdFormat2r	= L"{?=call %s(%s)}"; // {call procname(procparams)}
	LPWSTR pwszProcVersion	= NULL;
	ULONG iParam, ccCmdBufSize = 1000;
	HRESULT hr				= S_OK;
	LPWSTR pwszCmd = NULL;
	WCHAR wszMarkers[MAX_PARAM_COUNT*2] = L"";  // Each parameter requires ",?"
	LPWSTR pwszCmdFmt = pwszCmdFormat1;
	size_t ulValidLen = 0;
	BOOL	fFoundParamCount = FALSE;

	TESTC(pwszProcCat != NULL);
	TESTC(pwszProcSchema && pwszProcName && pwType);

	// If the proc name contains a semicolon, then we have to build the fully
	// qualified name w/o the ;nn part because those are not really part of the 
	// name.  This should only occur for SQL Server
	wcstok(pwszProcName, L";");

	// Create fully qualified name from the 3 values
	TESTC_(m_pTable->GetQualifiedName(
					pwszProcCat,
					pwszProcSchema,
					pwszProcName,
					&pwszQualifiedName, TRUE), S_OK);

	// Deal with remaining numeric portion of proc version.
	if (pwszProcVersion = wcstok(NULL, L";"))
	{
		// Put back the semicolon in the proc name
		pwszProcVersion[-1] = L';';

		// Realloc the qualified name to allow space for the version.
		SAFE_REALLOC(pwszQualifiedName, WCHAR, wcslen(pwszQualifiedName)+wcslen(pwszProcVersion-1)+sizeof(WCHAR));

		// Concat on the version
		wcscat(pwszQualifiedName, pwszProcVersion-1);
	}

	// Have to assume the ordinal position increments
	if (!piOrdinal)
	{
		m_iOrdinalPrev++;
	}

	// If this matches the previously obtained fully qualified name then
	// we have the same proc, just exit.
	if (m_pwszQualifiedName && !wcscmp(pwszQualifiedName, m_pwszQualifiedName))
		return TRUE;

	// Allocate some space for command buffer
	SAFE_ALLOC(pwszCmd, WCHAR, ccCmdBufSize);
	pwszCmd[0] = L'\0';	// Init to empty

	// This is a different stored proc, get the parameter information
	SAFE_FREE(m_pwszQualifiedName);
	m_pwszQualifiedName = pwszQualifiedName;
	SAFE_FREE(m_pNamesBuffer);

	// Reset our previous ordinal info and starting ordinal for this proc
	if (piOrdinal)
	{
		m_iOrdinalStart = *piOrdinal;
		m_iOrdinalPrev = *piOrdinal-1;
	}
	else
	{
		// Have to assume the ordinal position starts at 1
		m_iOrdinalPrev = 0;
		m_iOrdinalStart = 1;
	}

	// ----------------------------------------------------------------
	// Spawn off an entirely new connection to work around provider bug
	// ----------------------------------------------------------------
	TESTC(VerifyInterface(g_pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown **)&pIDBCreateSession));
	TESTC_(pIDBCreateSession->CreateSession(NULL, IID_IDBCreateCommand, (IUnknown **)&pIDBCreateCommand), S_OK);

	// Get a fresh command object
	TESTC_(pIDBCreateCommand->CreateCommand(
		NULL,
		IID_ICommand,
		(IUnknown **)&pICmd), S_OK);
// 	pICmd = m_pTable->get_ICommandPTR();
	// ----------------------------------------------------------------
	// Remove above block and associated variables when provider bug is fixed.
	// ----------------------------------------------------------------

	TESTC(VerifyInterface(pICmd, IID_ICommandText, COMMAND_INTERFACE, (IUnknown **)&pICmdText))
	TESTC(VerifyInterface(pICmd, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown **)&pICmdPrep))

	// We should always be able to get ICommandWithParameters if there is PROCEDURE_PARAMETERS rowset
	TESTC(VerifyInterface(pICmd, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown **)&pICmdWPar));

	m_cParams=0;

	if (m_cTotalRows == 1)
	{
		odtLog << L"Bug here - can't prepare command on 2nd cmd obj if schema open on first cmd obj?\n";
		// COMPAREW(0,1);
	}

	// If there is a return param, use that form
//	if (m_iOrdinalStart == 0)
		pwszCmdFmt = pwszCmdFormat1r;

	// Assume there may be up to 50 params
	for (iParam = 0; iParam < MAX_PARAM_COUNT; iParam++)
	{
		if (wcslen(pwszCmdFmt) + wcslen(m_pwszQualifiedName) + wcslen(wszMarkers) > ccCmdBufSize)
		{
			// Double the command buffer size and reallocate
			ccCmdBufSize *= 2;
			SAFE_REALLOC(pwszCmd, WCHAR, ccCmdBufSize);
		}

		// Build the command
		swprintf(pwszCmd, pwszCmdFmt, m_pwszQualifiedName, wszMarkers);
//		swprintf(pwszCmd, pwszCmdFmt, pwszProcName, wszMarkers);

		TESTC_(pICmdText->SetCommandText(DBGUID_DEFAULT, pwszCmd), S_OK);

		hr = pICmdPrep->Prepare(1);

		// We expect S_OK or DB_E_ERRORSINCOMMAND here
		if (E_FAIL == hr)
		{
			CHECKW(hr, DB_E_ERRORSINCOMMAND);
		}
		else
		{
			TEST2C_(hr, S_OK, DB_E_ERRORSINCOMMAND);
		}

		// ODBC drivers are allowed to succeed for Prepare() even if param count
		// is more than actual number of parameters.  For ODBC, need to use
		// GetParameterInfo to keep the driver honest.
		if (hr == S_OK)
		{
			// Don't leak previous param info
			SAFE_FREE(m_pParamInfo);
			SAFE_FREE(m_pNamesBuffer);

			// Temporarily print command executed
//			odtLog << L"Prepared: " << pwszCmd << L"\n";

			// Get new param info
			hr = pICmdWPar->GetParameterInfo(&m_cParams, &m_pParamInfo, &m_pNamesBuffer);
			TEST2C_(hr, S_OK, E_FAIL);
		}

		// Some providers will return S_OK for more param markers than required, but
		// m_cParams will be set to the proper number of parameters
		if (FAILED(hr) || m_cParams < iParam) 
		{
			// Post failure if provider returns param count less than requested for S_OK
//			COMPARE(hr != S_OK || m_cParams >= iParam+1, TRUE);

			// Strip off the last param
			if (iParam > 1)		// If iParam == 1 then 0 params were allowed
			{
				pwszCmd[ulValidLen-2] = L'\0';
				wcscat(pwszCmd, L")");
			}
			else
			{
				// We can get here if provider specifies there is a return param
				// but fails the prepare if there is a return param present, or if
				// the provider specifies there is no return param but fails prepare
				// w/o params, or if there are no additional params beyond the return
				// param.
//				ASSERT(ulValidLen > 0);
				TESTC(ulValidLen != 0);	
				pwszCmd[ulValidLen-1] = L'\0';
			}

			// Add back the closing bracket
			wcscat(pwszCmd, L"}");

			// Set and Prepare again
			TESTC_(pICmdText->SetCommandText(DBGUID_DEFAULT, pwszCmd), S_OK);
			TESTC_(pICmdPrep->Prepare(1), S_OK);
			TESTC_(pICmdWPar->GetParameterInfo(&m_cParams, &m_pParamInfo, &m_pNamesBuffer), S_OK);
			fFoundParamCount = TRUE;
			break;
		}
		else
		{
			TESTC_(hr, S_OK);
			ulValidLen = wcslen(pwszCmd);
			m_cParams = iParam;
		}

		if (iParam)
			wcscat(wszMarkers, L",");

		wcscat(wszMarkers, L"?");
		pwszCmdFmt = pwszCmdFormat2;
		if (m_iOrdinalStart == 0)
			pwszCmdFmt = pwszCmdFormat2r;
		else
			pwszCmdFmt = pwszCmdFormat2;

	}

	TESTC(fFoundParamCount);
	fRet = TRUE;

CLEANUP:

	SAFE_RELEASE(pICmdWPar);
	SAFE_RELEASE(pICmdText);
	SAFE_RELEASE(pICmdPrep);
	SAFE_RELEASE(pICmd);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_FREE(pwszCmd);
	return fRet;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CProcedureParametersHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CProcedureParametersHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	// At this time there is no schema-specific testing for PROCEDURE_PARAMETERS schema

	BOOL fResult = FALSE;
	DBSTATUS ulStatus		= pColumn->sStatus;
	LPBYTE pValue			= GetValuePtr(iOrdinal, m_pRow);
	UWORD * piOrdinal		= (UWORD *)GetValuePtr(PPS_ORDINAL_POSITION, m_pRow);
	LPWSTR pwszProcName		= (LPWSTR)GetValuePtr(PPS_PROCEDURE_NAME, m_pRow);
	LPWSTR pwszParamName	= (LPWSTR)GetValuePtr(PPS_PARAMETER_NAME, m_pRow);
	USHORT * pusParamType	= (USHORT *)GetValuePtr(PPS_PARAMETER_TYPE, m_pRow);
	DBTYPE * pwType			= (DBTYPE *)GetValuePtr(PPS_DATA_TYPE, m_pRow);
	USHORT usParamType		= 0;
	DBORDINAL iParamOrdinal	= 0;
	ULONG iParamIndex= 0;
	DBORDINAL ord1, ord2;
	DBTYPE wTypeInfo, wType;

	wType = *pwType;

	if (pusParamType)
		usParamType = *pusParamType;

	if (m_cTotalRows == 1 && ((iOrdinal == 0) || (iOrdinal == 1)))
	{
		// Always reset param info if first row and ordinal in case it's left
		// over from previous test variation.
		SAFE_FREE(m_pwszQualifiedName);
		SAFE_FREE(m_pParamInfo);
		SAFE_FREE(m_pNamesBuffer);
		m_cParams = 0;
	}
	
	TESTC(piOrdinal != NULL);

	TESTC(GetParamInfo());
//	ASSERT(m_pParamInfo != NULL);
	TESTC(m_pParamInfo != NULL);	// Should always have param info

	iParamOrdinal = *piOrdinal;

	// If we have the right index, then the ordinals will be related
	// If no return param, both ordinals start at one, otherwise not
	// Proc param ordinals are 0-based if return param
	ord2 = (DBORDINAL)(*piOrdinal);

	// Search for proper ordinal in parameter info
	for (iParamIndex = 0; iParamIndex < m_cParams; iParamIndex++)
	{
		if (!wcscmp(m_pParamInfo[iParamIndex].pwszName, pwszParamName))
		{
			ord1 = m_pParamInfo[iParamIndex].iOrdinal - !(m_iOrdinalStart);
			break;
		}
	}

	// Make sure we found the ordinal
	TESTC(iParamIndex < m_cParams);

	// If we restricted on the parameter name, then we can't know if the ordinals
	// are correct w/o going out and calling GetSchema w/o param name restriction
	// to find out if there is a return param.  So only test ordinals when not
	// restricted on param name.
	if (!m_prgRestrictUsed || V_VT(&m_prgRestrictUsed[3]) == VT_EMPTY)
	{
		TESTC(ord1 == ord2);  
	}

	wTypeInfo = m_pParamInfo[iParamIndex].wType;  //wType reported by GetParameterInfo

	COMPARE(wType, wTypeInfo);

	// We don't check provider-specific columns
	if (iOrdinal > m_pSchemaInfo->cColumns)
		return TRUE;

	switch(iOrdinal)
	{
		case PPS_PROCEDURE_CATALOG:
			break;
		case PPS_PROCEDURE_SCHEMA:
			break;
		case PPS_PROCEDURE_NAME:
			break;
		case PPS_PARAMETER_NAME:
			TESTC(ulStatus == DBSTATUS_S_OK);
			TESTC(!wcscmp(m_pParamInfo[iParamIndex].pwszName, pwszParamName));
			break;
		case PPS_ORDINAL_POSITION:
			// Ordinals are always sequential.  Ordinals in proc params schema are 0-based if
			// there is a return param, while ordinals in GetParameterInfo are always 1-based.
			TESTC(*piOrdinal - m_iOrdinalPrev == 1);
			m_iOrdinalPrev++;
			break;
		case PPS_PARAMETER_TYPE:
			{
				usParamType = *(USHORT *)pValue;
				DBPARAMFLAGS dwFlags = m_pParamInfo[iParamIndex].dwFlags;
				
				// Note we check for at least one of the valid values in the generic
				// CHelper::VerifyValue

				// If this is the return param, then ordinal is 0
				if (iParamOrdinal == 0)
				{
					TESTC(usParamType == DBPARAMTYPE_RETURNVALUE);
				}
				else if ((dwFlags & DBPARAMFLAGS_ISINPUT) &&
						(dwFlags & DBPARAMFLAGS_ISOUTPUT))
				{
					TESTC(usParamType == DBPARAMTYPE_INPUTOUTPUT);
				}
				else if (dwFlags & DBPARAMFLAGS_ISINPUT)
				{
					TESTC(usParamType == DBPARAMTYPE_INPUT);
				}
				else if (dwFlags & DBPARAMFLAGS_ISOUTPUT)
				{
					TESTC(usParamType == DBPARAMTYPE_OUTPUT);
				}
			}
			break;
		case PPS_PARAMETER_HASDEFAULT:
			break;
		case PPS_PARAMETER_DEFAULT:
			break;
		case PPS_IS_NULLABLE:
			{
				DBPARAMFLAGS dwFlags = m_pParamInfo[iParamIndex].dwFlags;
				VARIANT_BOOL vbIsNullable = VARIANT_TRUE;

				// Note: Error is posted if value is NULL based on schema info in VerifiyNull
				if (pValue != NULL)
				{
					vbIsNullable = *(VARIANT_BOOL *)pValue;

					if (dwFlags & DBPARAMFLAGS_ISNULLABLE)
					{
						TESTC(vbIsNullable == VARIANT_TRUE);
					}
					else
					{
						TESTC(vbIsNullable == VARIANT_FALSE);
					}
				}
			}
			break;
		case PPS_DATA_TYPE:
			TESTC(ulStatus == DBSTATUS_S_OK);
			TESTC(wType == *(DBTYPE *)pValue); // Redundant test, tested above comparing to wTypeInfo
			break;
		case PPS_CHARACTER_MAXIMUM_LENGTH:
			break;
		case PPS_CHARACTER_OCTET_LENGTH:
			break;
		case PPS_NUMERIC_PRECISION:
			if (!IsNumericType(wType) || DBTYPE_VARNUMERIC == wType)
			{
				TESTC(ulStatus == DBSTATUS_S_ISNULL);
			}
			else
			{
				TESTC(ulStatus == DBSTATUS_S_OK);
				TESTC((USHORT)m_pParamInfo[iParamIndex].bPrecision == *(USHORT *)pValue);
			}
			break;
		case PPS_NUMERIC_SCALE:
			switch(wType)
			{
				case DBTYPE_DECIMAL:
				case DBTYPE_NUMERIC:
				case DBTYPE_VARNUMERIC:
					TESTC(ulStatus == DBSTATUS_S_OK);
					TESTC((USHORT)m_pParamInfo[iParamIndex].bScale == *(USHORT *)pValue);
					break;
				default:
					TESTC(ulStatus == DBSTATUS_S_ISNULL);
			}
			break;
		case PPS_DESCRIPTION:
			if (ulStatus != DBSTATUS_S_ISNULL)
				VerifyNonEmptyString(pColumn, pwszProcName, pwszParamName);
			break;
		case PPS_TYPE_NAME:
			// TODO: Make sure the type name is one from PROVIDER_TYPES, and the
			// DBType matches the name.
			// COMPAREW(0, 1);
			VerifyNonEmptyString(pColumn, pwszProcName, pwszParamName);
			VerifyColumnMatch(PPS_TYPE_NAME, PPS_LOCAL_TYPE_NAME);
			break;
		case PPS_LOCAL_TYPE_NAME:
			VerifyNonEmptyString(pColumn, pwszProcName, pwszParamName);
			break;
	}

	fResult = TRUE;

CLEANUP:

	return fResult;
}

// Provider Types schema specific functions
class CProviderTypesHelper : public CHelper
{ 

private:
	DBTYPE m_dbPrevType;
	BOOL m_fBestMatchFound;

protected:
	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);

public:
	CProviderTypesHelper(void);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CProviderTypesHelper::CProviderTypesHelper
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CProviderTypesHelper::CProviderTypesHelper(void)
{
	m_guidSchema = DBSCHEMA_PROVIDER_TYPES;
	m_dbPrevType = DBTYPE_VECTOR;  // DBTYPE_VECTOR by itself is not valid
	m_fBestMatchFound = FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CProviderTypesHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CProviderTypesHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	DBTYPE * pdbTypeRow				= (DBTYPE *)GetValuePtr(PTS_DATA_TYPE, m_pRow);
	VARIANT_BOOL * pvbBestMatch		= (VARIANT_BOOL *)GetValuePtr(PTS_BEST_MATCH, m_pRow);
	DBSTATUS ulStatus				= pColumn->sStatus;
	BOOL	fResult					= FALSE;

	switch(iOrdinal)
	{
			// Currently only have tests for BEST_MATCH.  Most PROVIDER_TYPES schema testing is done in
			// ADO Connection test which caches the entire rowset and verifies no changes from previous
			// runs.
			case PTS_BEST_MATCH:
			{
				// PROVIDER_TYPES has only two restrictions.  The first is on data type and second on BEST_MATCH.
				// If we are restricted on BEST_MATCH then we will either always get all VARIANT_TRUE or all 
				// VARIANT_FALSE, so we cannot test for one and only one BEST_MATCH == VARIANT_TRUE.
				if (m_cRestrictionsUsed > 1)
					break;

				// MSDASQL does not support BEST_MATCH and always returns NULL.  In general, since the column
				// is BOOL and NULL is not a valid value we should disallow this behavior, so only allow for MSDASQL.
				if (IsMSDASQL() && ulStatus == DBSTATUS_S_ISNULL)
					break;

				TESTC(ulStatus == DBSTATUS_S_OK);

				// Reset best match found flag if on the first row
				if (m_cTotalRows == 1)
				{
					m_fBestMatchFound = FALSE;
					m_dbPrevType = DBTYPE_VECTOR;
				}

				if (*pdbTypeRow != m_dbPrevType)
				{
					if (m_dbPrevType != DBTYPE_VECTOR)
					{
						TESTC(m_fBestMatchFound);
					}
					m_fBestMatchFound = FALSE;
					m_dbPrevType = *pdbTypeRow;
				}
				else if (m_fBestMatchFound)
				{
					TESTC(*pvbBestMatch == VARIANT_FALSE);
				}

				if (*pvbBestMatch == VARIANT_TRUE)
					m_fBestMatchFound = TRUE;

				break;
			}
	}

	fResult = TRUE;

CLEANUP:

	return fResult;
}

// Table Constraints schema specific functions
class CTableConstraintsHelper : public CHelper
{ 

private:


protected:

public:

	CTableConstraintsHelper(void) {m_guidSchema = DBSCHEMA_TABLE_CONSTRAINTS;}
};

// Table Privileges schema specific functions
class CTablePrivilegesHelper : public CHelper
{ 

private:


protected:

public:

	CTablePrivilegesHelper(void) {m_guidSchema = DBSCHEMA_TABLE_PRIVILEGES;}
};


// Tables schema specific functions
class CTablesHelper : public CHelper
{ 

private:


protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CTablesHelper(void) {m_guidSchema = DBSCHEMA_TABLES;}

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CTablesHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CTablesHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	BOOL fResults = FALSE;

	// At this time there is no schema-specific testing for TABLES schema

	switch(iOrdinal)
	{
		case 0:
			// Bookmarks on and bound
			// TODO: Add code to validate bookmarks are expected and they work.
			break;
		case TABLES_TABLE_CATALOG:
			break;

		case TABLES_TABLE_SCHEMA:
			break;

		case TABLES_TABLE_NAME:
			break;

		case TABLES_TABLE_TYPE:
			break;
		
		case TABLES_TABLE_GUID:
			break;
		
		case TABLES_DESCRIPTION:
			break;

		case TABLES_TABLE_PROPID:
			break;

		case TABLES_DATE_CREATED:
		case TABLES_DATE_MODIFIED:
			break;
		default:
			odtLog << L"Unexpected column in schema rowset.\n";
			TESTC(FALSE);
	}

	fResults = TRUE;

CLEANUP:

	return fResults;

}

// Tables Info schema specific functions
class CTablesInfoHelper : public CHelper
{ 

private:


protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);

public:

	CTablesInfoHelper(void) {m_guidSchema = DBSCHEMA_TABLES_INFO;}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CTablesInfoHelper::VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CTablesInfoHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{
	LPWSTR pwszTableName	= (LPWSTR)GetValuePtr(TIS_TABLE_NAME, m_pRow);
	LPWSTR pwszTableType	= (LPWSTR)GetValuePtr(TIS_TABLE_TYPE, m_pRow);
	VARIANT_BOOL bBookMarks	= (VARIANT_BOOL)GetValuePtr(TIS_BOOKMARKS, m_pRow);
	LPBYTE pValue			= GetValuePtr(iOrdinal, m_pRow);
	BOOL fResult			= FALSE;
	ULONG iIndex			= 0;
	DBSTATUS ulStatus		= pColumn->sStatus;

	switch(iOrdinal)
	{
			case TIS_TABLE_CATALOG:
				break;

			case TIS_TABLE_SCHEMA:
				break;

			case TIS_TABLE_NAME:
				break;

			case TIS_TABLE_TYPE:
				// Note: We already test that the value is one of the valid ones in VerifyValue					
				break;

			case TIS_TABLE_GUID:
				break;

			case TIS_BOOKMARKS:
				// Note: We already test that the value is one of the valid ones in VerifyValue					
				break;

			case TIS_BOOKMARK_TYPE:	
				// This column is NULL if BOOKMARKS column is VARIANT_FALSE
				if (bBookMarks == VARIANT_FALSE)
				{
					TESTC(ulStatus == DBSTATUS_S_ISNULL);
				}
				else
				{
					TESTC(ulStatus == DBSTATUS_S_OK);

					// Note: We already test that the value is one of the valid ones in VerifyValue					
				}
				break;

			case TIS_BOOKMARK_DATATYPE:

				if (bBookMarks == VARIANT_FALSE)
				{
					TESTC(ulStatus == DBSTATUS_S_ISNULL);
				}
				else
				{
					TESTC(ulStatus == DBSTATUS_S_OK);

					if (m_prgColInfo && m_prgColInfo[0].iOrdinal == 0)
					{
						TESTC(m_prgColInfo[0].wType == *(DBTYPE *)pValue);
					}
				}
				break;

			case TIS_BOOKMARK_MAXIMUM_LENGTH:
				if (bBookMarks == VARIANT_FALSE)
				{
					TESTC(ulStatus == DBSTATUS_S_ISNULL);
				}
				else
				{
					TESTC(ulStatus == DBSTATUS_S_OK);

					if (m_prgColInfo && m_prgColInfo[0].iOrdinal == 0)
					{
						TESTC(m_prgColInfo[0].ulColumnSize == *(ULONG *)pValue);
					}
				}
				break;

			case TIS_BOOKMARK_INFORMATION:
				break;

			case TIS_TABLE_VERSION:
				break;

			case TIS_CARDINALITY:
				// Only valid for TABLE types, must be NULL for other types

				if (pwszTableType && (
					!wcscmp(pwszTableType, L"TABLE") ||
					!wcscmp(pwszTableType, L"SYSTEM TABLE") ||
					!wcscmp(pwszTableType, L"GLOBAL TEMPORARY") ||
					!wcscmp(pwszTableType, L"LOCAL TEMPORARY") ||
					!wcscmp(pwszTableType, L"EXTERNAL TABLE")))
				{
					// We expect a valid non-null cardinality
					TESTC(ulStatus == DBSTATUS_S_OK);
				}
				else
				{
					// This is not a TABLE type, so cardinality must be null
					TESTC(ulStatus == DBSTATUS_S_ISNULL);

					// TODO: For the base table we created, this should have the right number of rows.
					// For random tables counting the rows could possibly be done but is likely too time intensive.


				}
				break;

			case TIS_DESCRIPTION:
				break;

			case TIS_TABLE_PROPID:
				break;
		}

	fResult = TRUE;

CLEANUP:

	return fResult;

}


// Table Statistics schema specific functions
class CTableStatisticsHelper : public CHelper
{ 

private:
	CErrorCache		m_EC;	// Table Statistics error cache
	BOOL m_fFoundCardinality;
	BOOL m_fFoundHistogram;
	BOOL m_fCardinalityIncludesBlob;
	LPWSTR m_pwszStatName;
	BOOL m_fDetailCheck;
	ULONG m_iOrdinalExpected;
	IOpenRowset * m_pIOpenRowset;
	ULONG_PTR m_ulTableStatistics;
	LPWSTR * m_ppwszStatCols;
	DBORDINAL m_cStatCols;	

protected:

	BOOL VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn);
	BOOL CheckHistogramColInfo(IRowset * pIHistogramRowset, DBTYPE wRangeColType,
		DBORDINAL * pcBinding, DBBINDING ** ppBinding, DBLENGTH * pcbRowSize, 
		HACCESSOR * phAccessor);

	// Returns cardinality (COUNT) information for the given table using ulColIndex index into
	// columns info for table.
	DBCOUNTITEM Cardinality(LPWSTR pwszTableName, ULONG ulColIndex, CARDINALITY eCardinality,
		DBBINDING * pStartBind = NULL, DBBINDING * pEndBind = NULL, LPBYTE pDataHist = NULL,
		DBLENGTH cbRowSize = 0);

public:
	CTableStatisticsHelper(void);
	~CTableStatisticsHelper(void);
	BOOL Init(CSchemaTest * pThis);
};

CTableStatisticsHelper::CTableStatisticsHelper(void)
{
	m_guidSchema = DBSCHEMA_TABLE_STATISTICS;
	m_pIOpenRowset	= NULL;
	m_pwszStatName	= NULL;
	m_ppwszStatCols	= NULL;
	m_cStatCols		= 0;	
}

// CTableStatisticsHelper destructor
CTableStatisticsHelper::~CTableStatisticsHelper(void)
{
	// If a histogram was found then make sure support is indicated.
	// Note that if a histogram was not found we cannot infer lack of support
	// as there may not happen to be any created at this time, so only post
	// a warning
	if (m_fFoundHistogram)
		COMPARE(!!(m_ulTableStatistics & DBPROPVAL_TS_HISTOGRAM), TRUE);
	else
		COMPAREW(!!(m_ulTableStatistics & DBPROPVAL_TS_HISTOGRAM), FALSE);

	// If a cardinality info was found then make sure support is indicated.
	// Note that if a cardinality was not found we cannot infer lack of support
	// as there may not happen to be any created at this time, so only post
	// a warning
	if (m_fFoundCardinality)
		COMPARE(!!(m_ulTableStatistics & DBPROPVAL_TS_CARDINALITY), TRUE);
	else
		COMPAREW(!!(m_ulTableStatistics & DBPROPVAL_TS_CARDINALITY), FALSE);

	SAFE_RELEASE(m_pIOpenRowset);

	if (m_ppwszStatCols)
	{
		for (DBORDINAL iCol = 0; iCol < m_cStatCols; iCol++)
			SAFE_FREE(m_ppwszStatCols[iCol]);

		SAFE_FREE(m_ppwszStatCols);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CTableStatisticsHelper::Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CTableStatisticsHelper::Init(CSchemaTest * pThis)
{
	LPWSTR pwszUpdateStats = NULL;
	size_t	ccUpdateStats = 0;
	LPWSTR pwszUpdateStatsFormat = pThis->m_pwszUpdateStatsFormat;
	BOOL fSuccess = FALSE;

	IRowset *	pIRowset		= NULL;
	HACCESSOR	hAccessor		= DB_NULL_HACCESSOR;
	DBBINDING * prgDBBINDING	= NULL;
	DBCOUNTITEM cDBBINDING		= 0;
	DBLENGTH	cbRowSize		= 0;
	HRESULT		hr				= S_OK;
	DBCOUNTITEM	cRowsObtained;
	HROW *		prghRows		= NULL;	// array of hrows
	LPBYTE		pData			= NULL;
	LPWSTR		pwszQualifiedName = NULL;
	VARIANT		rgRestrict[NUMELEM(rgTablesRestrict)];
	ULONG		cRestrict		= NUMELEM(rgTablesRestrict);
	HRESULT		hrUpdate		= E_FAIL;
	
	// Schema specific vars init
	m_fFoundCardinality = FALSE;
	m_fFoundHistogram = FALSE;
	m_ulTableStatistics = 0;
	m_pwszStatName = NULL;
	m_fDetailCheck = FALSE;
	m_iOrdinalExpected = 0;
	m_pIOpenRowset = pThis->m_pIOpenRowset;

	// Addref the IOpenRowset so it won't go away while we're using 
	m_pIOpenRowset->AddRef();

	// Initialize the helper
	if (!CHelper::Init(pThis))
		return FALSE;

	// Get DBPROP_TABLESTATISTICS
	GetProperty(g_propTableStatistics, DBPROPSET_DATASOURCEINFO, pThis->m_pIDBInitialize, &m_ulTableStatistics);

	// Execute the provider-specific command to update all table statistics so they will 
	// be accurate for the test.  Both Oracle and SQL Server have a stored proc that can
	// be called to update all table's statistics in the user's schema, but these have
	// problems.  On SQL Server, the proc can only be run by admin, and against Oracle, 
	// the proc is not supported for older servers (7.x).  So brute force method is 
	// required (very slow).
	if (pwszUpdateStatsFormat)
	{
		// Get the max size for a table name
		DBLITERALINFO* pTableLiteral = m_pTable->GetLiteralInfo(DBLITERAL_TABLE_NAME);
		TESTC(pTableLiteral != NULL);

		// Compute max size for a statement that will update a table
		// We assume Catalog Name, Schema Name, and Table Name will have
		// the same max length, and additionally that tables will be fully
		// qualified and quoted, since we don't know if there will be different
		// catalogs, schemas and we don't know there are not table names that
		// require quotes in the TABLES schema.
		// 8 == two quotes for each name portion, plus 2 separators for the portions
		ccUpdateStats = wcslen(pwszUpdateStatsFormat) +
						(pTableLiteral->cchMaxLen * 3 + 8) +1;

		SAFE_ALLOC(pwszUpdateStats, WCHAR, ccUpdateStats);

		// Initialize restrictions
		memset(rgRestrict, 0, sizeof(VARIANT) * cRestrict);

		// Set restriction on table type
		V_VT(&rgRestrict[3]) = VT_BSTR;
		V_BSTR(&rgRestrict[3]) = SysAllocString(L"TABLE");

		// Get the TABLES schema rowset
		TESTC_(m_pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_TABLES, cRestrict, rgRestrict,
			IID_IRowset, 0, NULL, (IUnknown **)&pIRowset), S_OK);

		// Get accessor and bindings for tables schema
		TESTC_(GetAccessorAndBindings(
			pIRowset,									// @parmopt [IN]  Rowset to create Accessor for
			DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
			&hAccessor,									// @parmopt [OUT] Accessor created
			&prgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
			&cDBBINDING,								// @parmopt [OUT] Count of bindings
			&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
			DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
			ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
			FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
			NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
			NULL,										// @parmopt [OUT] Array of DBCOLUMNINFO
			NULL,										// @parmopt [OUT] Count of Columns, also count of ColInfo elements
			NULL,
			DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
			0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
			NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
			NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
			NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
			DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
			BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
			NULL),S_OK);								// @parmopt [OUT] returned status array from CreateAccessor

		// Allocate a buffer to hold a row
		SAFE_ALLOC(pData, BYTE, cbRowSize);

		while (hr == S_OK)
		{

			// Loop through schema and update stats for each table
			hr=pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows);

			if (hr != DB_S_ENDOFROWSET)
				TESTC_(hr, S_OK);

			if (cRowsObtained)
			{
				// Get data for the row
				TESTC_(pIRowset->GetData(prghRows[0],hAccessor,pData), S_OK)

				// Create a fully qualified table name
				TESTC_(m_pTable->GetQualifiedName(
					(LPWSTR)GetValuePtr(TABLES_TABLE_CATALOG, pData, prgDBBINDING),
					(LPWSTR)GetValuePtr(TABLES_TABLE_SCHEMA, pData, prgDBBINDING),
					(LPWSTR)GetValuePtr(TABLES_TABLE_NAME, pData, prgDBBINDING),
					&pwszQualifiedName, TRUE), S_OK);

				swprintf(pwszUpdateStats, pwszUpdateStatsFormat, pwszQualifiedName);

				hrUpdate = m_pTable->BuildCommand(pwszUpdateStats, IID_IRowset, 
					EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL);

				// Table could have been dropped after schema was retrieved, or we
				// may not have permission.
				if (hrUpdate != DB_E_NOTABLE && hrUpdate != DB_SEC_E_PERMISSIONDENIED)
					CHECK(hrUpdate, S_OK);

				if (hrUpdate != S_OK)
					odtLog << pwszUpdateStats << L"\n";
				
				SAFE_FREE(pwszQualifiedName);
				ReleaseInputBindingsMemory(cDBBINDING, prgDBBINDING, pData, FALSE);
				pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL);
			}
		}

		odtLog << "\n==========================================================\n";
//		odtLog << "Executed command: " << pwszUpdateStats << "\n";
		odtLog << "Statistics for all tables in user's schema were updated.\n";
		odtLog << "==========================================================\n\n";
	}

	fSuccess = TRUE;

CLEANUP:

	VariantClear(&rgRestrict[4]);
	SAFE_FREE(pwszQualifiedName);
	SAFE_FREE(pwszUpdateStats);
	SAFE_FREE(prgDBBINDING);
	if (pIRowset && prghRows && *prghRows)
		pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL);
	SAFE_RELEASE(pIRowset);

	if (fSuccess)
		return m_EC.Init(GetModInfo()->GetDebugMode());
	else
		return TEST_FAIL;
}

// Compare histogram rowset columns info
BOOL CTableStatisticsHelper::CheckHistogramColInfo(IRowset * pIHistogramRowset, DBTYPE wRangeColType,
	DBORDINAL * pcBinding, DBBINDING ** ppBinding, DBLENGTH * pcbRowSize, HACCESSOR * phAccessor)
{
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBLENGTH		cbRowSize = 0;
	BOOL			fColInfo = FALSE;
	DBCOUNTITEM		cActualCols = 0;	// Count of columns returned in histogram rowset
	LPWSTR			pwszCmd	= NULL;
	IAccessor *		pIAcc = NULL;
	ICommand *		pICmd = NULL;
	DBBINDING *		prgUDTBinding = NULL;
	DB_LORDINAL *	pNonUDTOrdinals = NULL;

	// Save off member vars so we can call CheckAgainstIColumnsInfo()
	DBBINDING *		pBinding = m_rgDBBINDING;
	DBCOUNTITEM		cBinding = m_cDBBINDING;
	DBCOLUMNINFO *  pColInfo = m_rgDBCOLUMNINFO;
	DBCOUNTITEM		cColInfo = m_cDBCOLUMNINFO;
	LPWSTR			pStringsBuf = m_pStringsBuffer;

	m_cDBCOLUMNINFO = 0;
	m_rgDBCOLUMNINFO = NULL;
	m_pStringsBuffer = NULL;

	// Update expected type for RANGE_HI_KEY
	HistogramRowset[0].pColList[0].wType = wRangeColType;

	// Update count of columns if needed.  
	if (HistogramRowset[0].pulColCount)
	{
		HistogramRowset[0].cColumns = HistogramRowset[0].pulColCount[0];
	}

	// get bindings and column info
	if(!CHECK(GetAccessorAndBindings(
		pIHistogramRowset,							// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&hAccessor,									// @parmopt [OUT] Accessor created
		&m_rgDBBINDING,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cDBBINDING,								// @parmopt [OUT] Count of bindings
		&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Array of DBCOLUMNINFO
		&m_cDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
		goto CLEANUP;

	fColInfo = CheckAgainstIColumnsInfo(&HistogramRowset[0]);

	// CheckAgainstIColumnsInfo doesn't test for any extra columns, so warn here
	// if there are extras
	cActualCols = m_cDBCOLUMNINFO - !m_rgDBCOLUMNINFO[0].iOrdinal;

	// Check for extra columns in histogram rowset.  This is only a warning
	// since it's allowed to return extra columns.  Cache the warning so we don't 
	// get it for every histogram over and over.
	CCOMPARE(m_EC, HistogramRowset[0].cColumns == cActualCols, EC_EXTRA_COLUMN,
		L"Extra column in Histogram rowset.", TRUE);

CLEANUP:

	// Populate output params
	if (pcBinding)
		*pcBinding = cActualCols;
	if (ppBinding)
		*ppBinding = m_rgDBBINDING;
	else
		SAFE_FREE(m_rgDBBINDING);
	if (pcbRowSize)
		*pcbRowSize = cbRowSize;
	if (phAccessor)
		*phAccessor = hAccessor;
	else if (pIAcc && (hAccessor != DB_NULL_HACCESSOR))
		CHECK(pIAcc->ReleaseAccessor(hAccessor, NULL), S_OK);

	SAFE_RELEASE(pIAcc);
	SAFE_RELEASE(pICmd);

	SAFE_FREE(m_rgDBCOLUMNINFO);
	SAFE_FREE(m_pStringsBuffer);
	SAFE_FREE(pwszCmd);
	SAFE_FREE(prgUDTBinding);
	SAFE_FREE(pNonUDTOrdinals);

	// Restore member vars to avoid side effects
	m_rgDBBINDING	= pBinding;
	m_cDBBINDING	= cBinding;
	m_rgDBCOLUMNINFO= pColInfo;
	m_cDBCOLUMNINFO	= cColInfo;
	m_pStringsBuffer= pStringsBuf;

	return fColInfo;
}

DBCOUNTITEM CTableStatisticsHelper::Cardinality(LPWSTR pwszTableName, ULONG ulColIndex, CARDINALITY eCardinality,
	DBBINDING * pStartBind, DBBINDING * pEndBind, LPBYTE pDataRows, DBLENGTH cbRowSize)
{
	HRESULT hr						= S_OK;
	ULONG	ulIndexFirst			= 0;
	size_t	cChars					= 0;
	LPWSTR	pwszColList				= NULL;
	LPWSTR	pwszSelectFmt			= (LPWSTR)wszSELECT_DISTINCTCOLLIST;
	LPWSTR	pwszSelect				= NULL;
	LPWSTR	pwszGreaterFmt			= L"%s > ?";
	LPWSTR	pwszLessThanEqFmt		= L"%s <= ?";
	LPWSTR	pwszEqFmt				= L"%s = ?";
	LPWSTR	pwszRange				= NULL;
	DBCOUNTITEM	ulCardinality		= 0;
	IRowset * pIRowset				= NULL;
	IAccessor * pIAccessor			= NULL;
	ICommand * pICommand			= NULL;
	IAccessor * pICmdAccessor		= NULL;
	DBBINDING dbCountBind;
	DBBINDING dbParamBind[2];				// Only two params required for all cases
	ULONG	cParams					= 0;
//	HACCESSOR hAcc					= DB_NULL_HACCESSOR;
	HACCESSOR hParamAcc				= DB_NULL_HACCESSOR;
	HROW * phRows					= NULL;
	ULONG * pulCardinality			= NULL;
	DBORDINAL iCol; 
	DBCOUNTITEM	cRowsObtained;
	DBPARAMS dbParams;
	DBBINDSTATUS rgStatus[2];
	ULONG * pulOrdinal				= (ULONG *)GetValuePtr(TSS_ORDINAL_POSITION, m_pRow);
	ULONG ulOrdinal;

	if (eCardinality == TUPLE_CARDINALITY)
	{
		if (!COMPARE(pulOrdinal != NULL, TRUE))
			return 0;

		ulOrdinal = *pulOrdinal;
	}

	// Previously used the count aggregate function, but that was bad because:
	//		1) Doesn't include NULL rows
	//		2) Fails against GUID column on some providers.
		// select count(*) from (select distinct col1,col2 from <table> where col1 > ? and 
		// col1 <= ?) t1

	// Now we use just the select distinct, but this still will fail for BLOB columns,
	// so we have to avoid calling this function for BLOBS.
	// TODO:  If this function is made more sophisticated we can actually call for BLOBS.
	//		Possible improvements 1) Use conversion function, 2) actually retrieve data
	//		ourselves and count distinct values.
	// select distinct col1,col2 from <table> where col1 > ? and col1 <= ?

	// Assume no Blob columns are included in cardinality results.  Most important
	// for Tuple cardinality
	m_fCardinalityIncludesBlob = FALSE;

	// Initialize binding structures
	memset(&dbCountBind, 0, sizeof(DBBINDING));
	memset(&dbParamBind, 0, sizeof(DBBINDING)*2);

	// Initialize dbParams
	memset(&dbParams, 0, sizeof(DBPARAMS));

	// Note we use params for this to avoid having to convert range limits to strings and
	// obtain the appropriate literal prefix and suffix.  I am assuming that any provider
	// that has gone to the trouble to provide extensive support for query processors has
	// also supported parameters.  

	switch (eCardinality)
	{
		// Both range rows and eq rows are non-distinct, and all the cardinalities
		// for histogram are for only one column.  But they all need a 'where' clause
		case EQ_ROWS_CARDINALITY:
			// We don't have a range for this cardinality, we're only looking for 
			// values equal to RANGE_HIGH_KEY
			ASSERT(!pStartBind);
			// Fall through
		case RANGE_ROWS_CARDINALITY:
			// Both of these counts are non-distinct, so change our select
			// clause to not return distinct counts.
			pwszSelectFmt = (LPWSTR)wszSELECT_COLLISTFROMTBL;			
			// Fall through
		case DISTINCT_RANGE_ROWS_CARDINALITY:
		{
			size_t cchWhere = wcslen(wszWHERE); // Length of " where "
			size_t cchColName = wcslen(m_prgColInfo[ulColIndex].pwszName);
			LPWSTR	pwszFmt = pwszLessThanEqFmt;

			if (eCardinality == EQ_ROWS_CARDINALITY)
				pwszFmt = pwszEqFmt;

			// Fill out param info
			if (pStartBind)
			{
				cchWhere += wcslen(pwszGreaterFmt) -2 + cchColName + wcslen(wszAND);

				// Set parameter binding info
				memcpy(&dbParamBind[cParams], pStartBind, sizeof(DBBINDING));
				dbParamBind[cParams].eParamIO = DBPARAMIO_INPUT;
				dbParamBind[cParams].iOrdinal = cParams+1;

				cParams++;
			}
			if (pEndBind)
			{
				cchWhere += wcslen(pwszFmt) -2 + cchColName;

				// Set parameter binding info
				memcpy(&dbParamBind[cParams], pEndBind, sizeof(DBBINDING));
				dbParamBind[cParams].eParamIO = DBPARAMIO_INPUT;
				dbParamBind[cParams].iOrdinal = cParams+1;

				cParams++;
			}

			// Now we know all the pieces for the range clause, allocate memory and create
			SAFE_ALLOC(pwszRange, WCHAR, cchWhere+1);

			wcscpy(pwszRange, wszWHERE);

			// Put in the starting range clause
			if (pStartBind)
			{
				swprintf(pwszRange+wcslen(pwszRange), pwszGreaterFmt, m_prgColInfo[ulColIndex].pwszName);
				wcscat(pwszRange, wszAND);
			}

			// Put in ending range clause (always needed)
			swprintf(pwszRange+wcslen(pwszRange), pwszFmt, m_prgColInfo[ulColIndex].pwszName);

		}
			// Fall through
		case COLUMN_CARDINALITY:
			ulIndexFirst = ulColIndex;
			// Fall through
		case TUPLE_CARDINALITY:
			// Compute memory for column list (column name plus comma)
			for (iCol = 0; iCol < m_cStatCols; iCol++)
				cChars+=wcslen(m_ppwszStatCols[iCol])+1;

			// Allocate memory for col list
			SAFE_ALLOC(pwszColList, WCHAR, cChars+1);

			pwszColList[0] = L'\0';
			// Put non-BLOB columns in final list
			// Note we leave out BLOB cols because providers can't do a 
			// select distinct on a BLOB, which may have some impact on 
			// the cardinality results.
			for (iCol = 0; iCol < m_cStatCols; iCol++)
			{
				LPWSTR pwszCol = m_ppwszStatCols[iCol];
				ULONG iColInfo;

				// Locate this stat column in the columns info
				for (iColInfo = 0; iColInfo < m_cColInfo; iColInfo++)
				{
					if (!wcscmp(m_prgColInfo[iColInfo].pwszName, pwszCol))
						break;
				}
				TESTC(iColInfo < m_cColInfo);

				if (m_prgColInfo[iColInfo].dwFlags & DBCOLUMNFLAGS_ISLONG)
				{
					m_fCardinalityIncludesBlob = TRUE;
					continue;
				}

				// Put in comma separator if needed 
				if (pwszColList[0])
					wcscat(pwszColList, L",");
				wcscat(pwszColList, pwszCol);
			}

			// Compute memory required for select distinct statement
			// Length of select, 2 parenthesis, table name, table alias name
			cChars+=wcslen(pwszSelectFmt)+wcslen(pwszTableName);

			// Add space for 'where' clause if needed
			if (pwszRange)
				cChars+=wcslen(pwszRange);

			// Allocate mem for select
			SAFE_ALLOC(pwszSelect, WCHAR, cChars+1);

			// select distinct col1,col2 from <table>
			swprintf(pwszSelect, pwszSelectFmt, pwszColList, pwszTableName);

			// Tack on where clause if needed
			if (pwszRange)
				wcscat(pwszSelect, pwszRange);

			// Free the range clause
			SAFE_FREE(pwszRange);

			break;
		case TABLE_CARDINALITY:
			pwszSelectFmt = (LPWSTR)wszSELECT_ALLFROMTBL;

			cChars+=wcslen(pwszSelectFmt)+wcslen(pwszTableName);

			// Allocate mem for select
			SAFE_ALLOC(pwszSelect, WCHAR, cChars+1);

			// Create select stmt
			swprintf(pwszSelect, pwszSelectFmt, pwszTableName);

			break;
	}

	// Execute the command
	if (cParams)
	{

		// Get the command object for the table object
		pICommand = m_pTable->get_ICommandPTR();

		// Addref the command object so we can release later.
		pICommand->AddRef();

		// Get accessor interface
		TESTC(VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&pICmdAccessor));

		// Create the parameter accessor
		TESTC_(pICmdAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cParams,
			dbParamBind, cbRowSize, &hParamAcc, rgStatus), S_OK);

		// Fill DBPARAMS info
		dbParams.pData = pDataRows;
		dbParams.cParamSets = 1;
		dbParams.hAccessor = hParamAcc;

	}

	if (!CHECK(m_pTable->BuildCommand(pwszSelect, IID_IRowset, 
		EXECUTE_IFNOERROR, 0, NULL, &dbParams, NULL, (IUnknown **)&pIRowset, &pICommand), S_OK))
		goto CLEANUP;

	// Get accessor and bindings
//	dbCountBind.obStatus = offsetof(DATA, sStatus);
//	dbCountBind.obLength = offsetof(DATA, ulLength);
//	dbCountBind.obValue = offsetof(DATA, bValue);
//	dbCountBind.wType = DBTYPE_UI4;
//	dbCountBind.iOrdinal = 1;
//	dbCountBind.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;

//	TESTC(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor));
//	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbCountBind, 0, &hAcc, NULL), S_OK);
//	hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbCountBind, 0, &hAcc, NULL);

//	TESTC_(hr, S_OK);

	// Count the rows
	while(hr != DB_S_ENDOFROWSET)
	{
		TEST3C_(hr = pIRowset->GetNextRows(NULL, 0, 10000, &cRowsObtained, &phRows), S_OK, DB_S_ENDOFROWSET, DB_S_ROWLIMITEXCEEDED);

		if (hr == S_OK)
		{
			TESTC(cRowsObtained);
		}

		ulCardinality+=cRowsObtained;

		if (cRowsObtained)
		{
			CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
			*phRows = DB_NULL_HROW;
		}
	}

CLEANUP:

	// Release rows
	if (pIRowset && phRows && *phRows != DB_NULL_HROW)
		CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);

	// Release parameter accessor
	if (pICmdAccessor && hParamAcc != DB_NULL_HACCESSOR)
		CHECK(pICmdAccessor->ReleaseAccessor(hParamAcc, NULL), S_OK);

	SAFE_FREE(phRows);
	SAFE_FREE(pwszColList);
	SAFE_FREE(pwszSelect);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pICmdAccessor);

	return ulCardinality;

}


BOOL CTableStatisticsHelper::VerifyColumn(DBORDINAL iOrdinal, DATA * pColumn)
{

	CCol			col;
	BOOL			fResults = TRUE;
	HRESULT			hrOpenTable = E_FAIL;
	LPWSTR			pwszTableName = (LPWSTR)GetValuePtr(TSS_TABLE_NAME, m_pRow);
	LPWSTR			pwszStatName = (LPWSTR)GetValuePtr(TSS_STATISTICS_NAME, m_pRow);
	LPWSTR			pwszColumnName = (LPWSTR)GetValuePtr(TSS_COLUMN_NAME, m_pRow);
	ULONG *			pulTableCardinality = (ULONG *)GetValuePtr(TSS_TABLE_CARDINALITY, m_pRow);
	ULONG *			pulOrdinal = (ULONG *)GetValuePtr(TSS_ORDINAL_POSITION, m_pRow);
	ULONG			ulStatType = 0;
	ULONG			ulIndex = 0;
	DBID			dbidTable = DB_NULLID;
	BOOL			fNewTable = (!m_pwszTableName || !pwszTableName || (pwszTableName && RelCompareString(pwszTableName, m_pwszTableName)));
	BOOL			fNewStat = fNewTable | (!m_pwszStatName || !pwszStatName || (pwszStatName && RelCompareString(pwszStatName, m_pwszStatName)));
	CARDINALITY		eCardinality = TUPLE_CARDINALITY;
	LPWSTR			pwszCat = (LPWSTR)GetValuePtr(TSS_TABLE_CATALOG, m_pRow);
	LPWSTR			pwszSch = (LPWSTR)GetValuePtr(TSS_TABLE_SCHEMA, m_pRow);
	LPWSTR			pwszQualifiedName = NULL;
	LPWSTR			pwszQualifiedStatName = NULL;

	// Reset any previous table or stat name saved
	if (m_cTotalRows == 1)
	{
		fNewTable = TRUE;
		fNewStat = TRUE;
	}

	if (iOrdinal == 1)
	{
		// Initial allocation
		if (!m_ppwszStatCols)
		{
			m_cStatCols = 0;
			SAFE_ALLOC(m_ppwszStatCols, LPWSTR, MAX_STATS_COLS);
			memset(m_ppwszStatCols, 0, MAX_STATS_COLS * sizeof(LPWSTR *));
		}

		TESTC(m_cStatCols < MAX_STATS_COLS-1);

		// Reset list of stats columns for each new statistic
		if (fNewStat)
		{
			for (DBORDINAL iCol = 0; iCol < m_cStatCols; iCol++)
				SAFE_FREE(m_ppwszStatCols[iCol]);
			m_cStatCols = 0; // No columns currently in this new statistic
		}

		// Add this stat column to the list of statistics columns
		m_ppwszStatCols[m_cStatCols++] = wcsDuplicate(pwszColumnName);
	}

	TESTC_(m_pTable->GetQualifiedName(pwszCat, pwszSch, 
							pwszTableName,&pwszQualifiedName, TRUE), S_OK);
	TESTC_(m_pTable->GetQualifiedName(
		(LPWSTR)GetValuePtr(TSS_STATISTICS_CATALOG, m_pRow),
		(LPWSTR)GetValuePtr(TSS_STATISTICS_SCHEMA, m_pRow), 
		pwszStatName,
		&pwszQualifiedStatName, TRUE), S_OK);

	// *********************************************************************
	// For current time, set this on for debugging test code.
	// *********************************************************************
	m_fDetailCheck = TRUE;

	if (m_fDetailCheck)
	{
		ULONG * pulStatType = (ULONG *)GetValuePtr(TSS_STATISTICS_TYPE, m_pRow);

		// Attempt to open table and get columns info
		if (fNewTable)
		{
			hrOpenTable = GetColumnInfo(m_pRow, m_rgDBBINDING);

			// Should be S_OK, or perhaps DB_E_NOTABLE
			if (DB_E_NOTABLE != hrOpenTable)
			{
				// The first time we attempt to open the table we get the appropriate error,
				// and from then on for the same table we get S_FALSE.  We can't test much w/o
				// columnsinfo, so just skip.
				if (S_FALSE == hrOpenTable)
					goto CLEANUP;

				CHECK(hrOpenTable, S_OK);
			}
			else
				// We warn here because it's possible the table name is
				// bogus, or just deleted.
				CHECKW(hrOpenTable, S_OK);
		}
		else
			// If we obtained col info for the table we successfully opened it.
			hrOpenTable = (m_prgColInfo) ? S_OK : E_FAIL;

		// Set index into columns info given column name
		if (m_prgColInfo && pwszColumnName)
		{
			for (ulIndex = 0; ulIndex < m_cColInfo; ulIndex++)
			{
				if (!wcscmp(m_prgColInfo[ulIndex].pwszName, pwszColumnName))
					break;
			}
		}

		// Set up table dbid
		dbidTable.eKind = DBKIND_NAME;
//		dbidTable.uName.pwszName = pwszTableName;
		dbidTable.uName.pwszName = pwszQualifiedName;

		// Get statistic type
		if (pulStatType)
			ulStatType = *pulStatType;
		
	}

	// Set cardinality 
	eCardinality = TUPLE_CARDINALITY;

	switch(iOrdinal)
	{
	case TSS_TABLE_CATALOG:
		break;
	case TSS_TABLE_SCHEMA:
		break;
	case TSS_TABLE_NAME:
		break;
	case TSS_STATISTICS_CATALOG:
		break;
	case TSS_STATISTICS_SCHEMA:
		break;
	case TSS_STATISTICS_NAME:
		break;
	case TSS_STATISTICS_TYPE:
	{
		TYPE_UI2 ulStatType = 0;

		// Column cannot be NULL
		if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
		{
			ulStatType = *(TYPE_UI2 *)pColumn->bValue;

			// Make sure no other bits are set than those spec'd
			COMPARE(0, ulStatType & (~(DBSTAT_HISTOGRAM | DBSTAT_COLUMN_CARDINALITY |
				DBSTAT_TUPLE_CARDINALITY)));

			// If we claim histogram support then NO_OF_RANGES cannot be null
			// but may be 0 if the table is empty.
			if (ulStatType & DBSTAT_HISTOGRAM)
				COMPARE(GetValuePtr(TSS_NO_OF_RANGES, m_pRow) != NULL, TRUE);
			else
			{
				CCOMPARE(m_EC, GetValuePtr(TSS_NO_OF_RANGES, m_pRow) == NULL, 
						EC_NULL_RANGE_COUNT,
						L"DBSTAT_HISTOGRAM not set but NO_OF_RANGES is non-NULL",
						FALSE);
			}

			// If we claim column cardinality support then it must not be null
			if (ulStatType & DBSTAT_COLUMN_CARDINALITY)
			{
				m_fFoundCardinality = TRUE;
				COMPARE(GetValuePtr(TSS_COLUMN_CARDINALITY, m_pRow) != NULL, TRUE);
			}
			else
				COMPARE(GetValuePtr(TSS_COLUMN_CARDINALITY, m_pRow) == NULL, TRUE);

			// If we claim tuple cardinality support then not be null
			if (ulStatType & DBSTAT_TUPLE_CARDINALITY)
			{
				m_fFoundCardinality = TRUE;
				COMPARE(GetValuePtr(TSS_TUPLE_CARDINALITY, m_pRow) != NULL, TRUE);
			}
			else
				COMPARE(GetValuePtr(TSS_TUPLE_CARDINALITY, m_pRow) == NULL, TRUE);

			// DetailCheckStatType(ulStatType, m_pRow);

			// Detail check
			if (m_fDetailCheck)
			{
				IRowset * pIRowset = NULL;
				DBID dbidStat;
				LPWSTR pwszStatName = (LPWSTR)GetValuePtr(TSS_STATISTICS_NAME, m_pRow);
				HRESULT hrOpenHistogram = E_FAIL;

				dbidStat.eKind = DBKIND_GUID_NAME;
//				dbidStat.uName.pwszName = pwszStatName;
				dbidStat.uName.pwszName = pwszQualifiedStatName;
				dbidStat.uGuid.guid = g_guidHistogramRowset;

				// Attempt to open histogram rowset
				hrOpenHistogram = m_pIOpenRowset->OpenRowset(NULL,
					&dbidTable,
					&dbidStat,
					IID_IRowset,
					0,
					NULL,
					(IUnknown **)&pIRowset);

				// If there's a histogram then validate values
				if (ulStatType & DBSTAT_HISTOGRAM)
				{
					// Histograms are only supported on first column
					// of a multicolumn statistic.
					if (pulOrdinal && *pulOrdinal == 1)
					{
						// Record we found at least one histogram.
						m_fFoundHistogram = TRUE;

						ULONG * pcRanges = (ULONG *)GetValuePtr(TSS_NO_OF_RANGES, m_pRow);

						// It's possible the table was deleted after we opened
						// the schema rowset
						if (S_OK == hrOpenHistogram)
						{
//							odtLog << L"Opened histogram rowset for table: " << pwszTableName << L" statistic: "
//								<< pwszStatName << L"\n";

							if (CHECK(pIRowset != NULL, TRUE))
							{
								HRESULT		hrGetNextRows = S_OK;
								ULONG		cRows = 0;
								HROW *		phRows = NULL;
								DBCOUNTITEM	cRowsObtained = 0;
								DBCOUNTITEM	cBinding = 0, iBind;
								DBLENGTH	cbRowSize = 0;
								DBBINDING * pBinding = NULL;
								HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
								LPBYTE		pDataRows = NULL;
								
								// Check histogram rowset columns info
								if (m_prgColInfo && !COMPARE(CheckHistogramColInfo(
									pIRowset,
									m_prgColInfo[ulIndex].wType,
									&cBinding,
									&pBinding,
									&cbRowSize,
									&hAccessor
									), TRUE))
									odtLog << L"Histogram colinfo doesn't match.\n";

								// Make sure the provider returned valid info
								if (cBinding > 0 &&
									pBinding &&
									cbRowSize > 0 &&
									hAccessor != DB_NULL_HACCESSOR)
								{
									LPBYTE pDataHist = NULL;
									DBBINDING dbBind;
									DBCOUNTITEM ulRangeCount = 0;
									DBCOUNTITEM ulTotal = 0;

									// We need the table cardinality so if not available
									// we must compute ourselves
									if (pulTableCardinality)
										ulTotal = *pulTableCardinality;
									else
										ulTotal = Cardinality(pwszQualifiedName, 0, TABLE_CARDINALITY); 

									// Create a binding structure for the other row
									// data
									memcpy(&dbBind, pBinding, sizeof(DBBINDING));
									dbBind.obStatus+=cbRowSize;
									dbBind.obLength+=cbRowSize;
									dbBind.obValue+=cbRowSize;

									// Allocate memory for two rows of histogram
									SAFE_ALLOC(pDataRows, BYTE, cbRowSize*2);

									pDataHist = pDataRows;

									while ((hrGetNextRows = pIRowset->GetNextRows(
											NULL,
											0,
											1,
											&cRowsObtained,
											&phRows)) == S_OK)
									{

										cRows++;

										if (COMPARE(cRowsObtained, 1) &&
											COMPARE(phRows != NULL, TRUE) &&
											CHECK(pIRowset->GetData(phRows[0], hAccessor, pDataHist), S_OK))
										{
											LPBYTE pRANGE_HI_KEY = GetValuePtr(HR_RANGE_HI_KEY, pDataHist, pBinding);
											TYPE_R8 * pRANGE_ROWS = (TYPE_R8 *)GetValuePtr(HR_RANGE_ROWS, pDataHist, pBinding);
											TYPE_R8 * pEQ_ROWS = (TYPE_R8 *)GetValuePtr(HR_EQ_ROWS, pDataHist, pBinding);
											TYPE_I8 * pDISTINCT_RANGE_ROWS = 
												(TYPE_I8 *)GetValuePtr(HR_DISTINCT_RANGE_ROWS, pDataHist, pBinding);
											DBBINDING * pPrevBind=NULL;
											DBBINDING * pCurrentBind = pBinding;
											double fFraction, fPercent;

											// Set pointers to previous and current bindings for RANGE_HIGH_KEY
											if (cRows > 1)
											{
												if (pDataHist == pDataRows)
												{
													pCurrentBind = pBinding;
													pPrevBind = &dbBind;
												}
												else
												{
													pCurrentBind = &dbBind;
													pPrevBind = pBinding;
												}
											}


											// Validate this row

											// Check each status for unexpected values
											for (iBind = 0; iBind < cBinding; iBind++)
											{
												if (STATUS_BINDING(pBinding[iBind], pDataHist) != DBSTATUS_S_OK &&
													STATUS_BINDING(pBinding[iBind], pDataHist) != DBSTATUS_S_ISNULL &&
													!COMPARE(STATUS_BINDING(pBinding[iBind], pDataHist), DBSTATUS_S_OK))
													odtLog << L"Histogram rowset column " << iBind << L" returned an unexpected status: "
														<< STATUS_BINDING(pBinding[iBind], pDataHist) << L"\n";
											}
											
											// RANGE_HI_KEY is mandatory
											if (!COMPARE(pRANGE_HI_KEY != NULL, TRUE))
												odtLog << L"Table: " << pwszQualifiedName << 
													L" Statistic: " << pwszQualifiedStatName <<
 													L" returned NULL for mandatory RANGE_HI_KEY\n";

											// One of RANGE_ROWS or EQ_ROWS is required
											COMPARE(pRANGE_ROWS != NULL ||
													pEQ_ROWS != NULL, TRUE);

											// Validate sort is by RANGE_HI_KEY, ascending order

											// Validate data for each returned column
											// RANGE_HI_KEY

											/* Actually, this may not necessarily be a value in the column
												It may not even be within the range of values in the table, so we
												really can't test this.

												// Must be a value in the column
												if (pRANGE_HI_KEY)
												{																			
													ulRangeCount = Cardinality(pwszQualifiedName, ulIndex, EQ_ROWS_CARDINALITY, 
														NULL, pCurrentBind, pDataRows, cbRowSize*2);

													// Fail if no value in table == RANGE_HIGH_KEY
													if (!COMPARE(ulRangeCount > 0, TRUE))
														odtLog << pwszTableName << L": RANGE_HI_KEY not in base table.\n";
												}
											*/

											// RANGE_ROWS
												// Fraction of number of rows that fall in this histogram 
												// range.  Select rows in this range and
												// divide by table cardinality.
												if (pRANGE_ROWS)
												{																			
													// Can't compute cardinality for BLOB at this time
													if (m_prgColInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISLONG)
													{
														odtLog << L"Test cannot compute cardinality for LONG columns.\n";
														odtLog << L"Comparison skipped.\n";
													}
													else
													{
														ulRangeCount = Cardinality(pwszQualifiedName, ulIndex, RANGE_ROWS_CARDINALITY, 
															pPrevBind, pCurrentBind, pDataRows, cbRowSize*2);
														fFraction = (float)(DB_LORDINAL)ulRangeCount/(float)(DB_LORDINAL)ulTotal;

														// Compute percentage error in result, avoid division by 0
														if (fFraction > 0.0)
															fPercent = fabs((*(double *)pRANGE_ROWS - fFraction)/fFraction);
														else
															fPercent = 0.0;

														// Fail if off by more than 10%
														CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
															EC_BAD_RANGE_ROWS,
															L"RANGE_ROWS not within 10%",
															FALSE);
													}
												}


											// EQ_ROWS
												// Fraction of rows equal to RANGE_HIGH_KEY.
												// Select rows equal and divide by table cardinality.
												if (pEQ_ROWS)
												{																			
													ulRangeCount = Cardinality(pwszQualifiedName, ulIndex,	EQ_ROWS_CARDINALITY, 
														NULL, pCurrentBind, pDataRows, cbRowSize*2);
													fFraction = (float)(DB_LORDINAL)ulRangeCount/(float)(DB_LORDINAL)ulTotal;

													// Compute percentage error in result, avoid division by 0
													if (fFraction > 0.0)
														fPercent = fabs((*(double *)pEQ_ROWS - fFraction)/fFraction);
													else
														fPercent = 0.0;

													// Fail if off by more than 10%
													CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
														EC_BAD_EQ_ROWS,
														L"EQ_ROWS not within 10%",
														FALSE);
												}

											// DISTINCT_RANGE_ROWS
												// Number of distinct values in this range.
												// Select distinct rows in this range.
												if (pDISTINCT_RANGE_ROWS)
												{			
													ulRangeCount = Cardinality(pwszQualifiedName, ulIndex,	DISTINCT_RANGE_ROWS_CARDINALITY, 
														pPrevBind, pCurrentBind, pDataRows, cbRowSize*2);

													// Compute percentage error in result, avoid division by 0
													if (ulRangeCount)
														fPercent = fabs(float((*(LONGLONG *)pDISTINCT_RANGE_ROWS - (LONGLONG)ulRangeCount))/(float)(DB_LORDINAL)ulRangeCount);
													else
														fPercent = 0.0;

													// Fail if off by more than 10%
													CCOMPARE(m_EC, fPercent < MAX_STATISTIC_ERROR, 
														EC_BAD_DISTINCT_RANGE_ROWS,
														L"DISTINCT_RANGE_ROWS not within 10%",
														FALSE);
												}
											
										}
										else
										{
											odtLog << L"Unable to retrieve data from Histogram for column: " <<
												(m_prgColInfo[ulIndex]).pwszName << ".\n";
										}

										// Release the row
										CHECK(pIRowset->ReleaseRows(1, phRows, NULL, NULL, NULL), S_OK);

										// Point the pDataHist buffer to other half of total buffer
										pDataHist = pDataRows+cbRowSize*(cRows % 2);
										ReleaseInputBindingsMemory(cBinding, pBinding, pDataHist, FALSE);
									}

									// Free the row handles
									SAFE_FREE(phRows);

									// Release memory for the previous row we retrieved
									if (cRows)
										ReleaseInputBindingsMemory(cBinding, pBinding, pDataRows+cbRowSize*((cRows-1) % 2), FALSE);

									SAFE_FREE(pDataRows);

									// Make sure we got DB_S_ENDOFROWSET
									CHECK(hrGetNextRows, DB_S_ENDOFROWSET);
									
									// Make sure row count matches NO_OF_RANGES
									if (pcRanges && !COMPARE(cRows, *pcRanges))
									{
										odtLog << L"NO_OF_RANGES did not match count of rows in Histogram rowset.\n";
										odtLog << L"NO_OF_RANGES: " << *pcRanges << L" Row count: " << cRows << "\n";
										odtLog << L"TABLE: " << pwszQualifiedName << L"\n";
										odtLog << L"STATISTIC: " << pwszQualifiedStatName << L"\n";
									}

								}
							}

						}
						else
						{
							CHECK(pIRowset == NULL, TRUE);
							// We allow the histogram failure if the table no longer
							// exists.
							if (!CHECK(hrOpenHistogram, DB_E_NOTABLE) ||
								!COMPARE(FAILED(hrOpenTable), TRUE))
								odtLog << L"Histogram rowset not returned.\n";
						}
					}
				}
				else
				{
					// If the table was deleted the provider may return
					// DB_E_NOTABLE instead of DB_E_NOSTATISTIC
					if (hrOpenHistogram != DB_E_NOTABLE)
					{
						HRESULT hrExp = DB_E_NOSTATISTIC;

						CCHECK(m_EC, hrOpenHistogram, hrExp,
							EC_BAD_HR_OPENHISTOGRAM,
							L"Unexpected return code from OpenRowset on histogram.",
							FALSE);
					}
				}

				SAFE_RELEASE(pIRowset);


			} 

		}

		break;
	}
	case TSS_COLUMN_NAME:
		// Sanity check

		// Column must have non-empty string and cannot be NULL
		if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
		{
			// Detail check
			if (m_fDetailCheck && SUCCEEDED(hrOpenTable) && m_prgColInfo)
				// Column name must match columns info for this ordinal
				COMPARE(RelCompareString(m_prgColInfo[ulIndex].pwszName, (TYPE_WSTR)pColumn->bValue), 0);
		}
		break;
	case TSS_COLUMN_GUID:
		// Sanity check: none

		// Detail check
		if (m_fDetailCheck && SUCCEEDED(hrOpenTable))
		{
			// Column guid must match columns info
			if (S_OK == pColumn->sStatus)
			{
				COMPARE(m_prgColInfo[ulIndex].columnid.uGuid.guid == *(GUID *)pColumn->bValue, TRUE);
			}
			else
			{
				// Must be NULL
//						COMPARE(m_prgColInfo[ulIndex].columnid.uGuid.guid == DB_NULLGUID, TRUE);
			}
		}
		break;
	case TSS_COLUMN_PROPID:
		// Sanity check: none

		// Detail check
		if (m_fDetailCheck && SUCCEEDED(hrOpenTable))
		{
			// Column propid must match columns info
			if (S_OK == pColumn->sStatus)
				COMPARE(m_prgColInfo[ulIndex].columnid.uName.ulPropid == *(ULONG *)pColumn->bValue, TRUE);

			// If status == DBSTATUS_S_ISNULL, then value is undefined and can't be
			// compared.  And it appears ulPropid in columnsinfo is uninitialized.
		}
		break;
	case TSS_ORDINAL_POSITION:
	// The ordinal position must be sequential within the table
	{
		// Column cannot be null
		if (COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
		{
			// If we have a valid column name
			if (pwszTableName && pwszStatName)
			{
				if (fNewStat)
				{
					// This is a new statistic, ordinal must be 1
					m_iOrdinalExpected = 1;
					SAFE_FREE(m_pwszTableName);
					SAFE_FREE(m_pwszStatName);
					m_pwszTableName = wcsDuplicate(pwszTableName);
					m_pwszStatName = wcsDuplicate(pwszStatName);
				}
				else
					m_iOrdinalExpected++;

				// Compare Ordinal returned with expected
				if (!COMPARE(m_iOrdinalExpected, *(TYPE_UI4 *)pColumn->bValue))
					odtLog << pwszTableName << L": Invalid ordinal value returned.\n";
			}

		}
		break;
	}
	case TSS_SAMPLE_PCT:
	case TSS_LAST_UPDATE_TIME:
	case TSS_NO_OF_RANGES:
		// This is tested for accuracy by counting rows in histogram rowset.
		break;
	case TSS_COLUMN_CARDINALITY:
		eCardinality = COLUMN_CARDINALITY;
		// Fall through
	case TSS_TUPLE_CARDINALITY:
		// This should equal number distinct rows in table for this column
		if (m_fDetailCheck && SUCCEEDED(hrOpenTable) &&
			pColumn->sStatus == DBSTATUS_S_OK)
		{
			LPWSTR pwszCardinality = L"COLUMN_CARDINALITY";

			if (eCardinality == TUPLE_CARDINALITY)
				pwszCardinality = L"TUPLE_CARDINALITY";

			if (m_prgColInfo)
			{
				// Can't compute cardinality for BLOB at this time
				if (m_prgColInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISLONG)
				{
					odtLog << L"Test cannot compute cardinality for LONG columns.\n";
					odtLog << L"Comparison skipped.\n";
				}
				else
				{
					DBCOUNTITEM	cRows = Cardinality(pwszQualifiedName, ulIndex, eCardinality);
					WCHAR wszMessage[MAX_MSG_BUF] = L"";

					swprintf(wszMessage,
						L"%s is incorrect\nTable: %s Statistic: %s %s expected: %d received: %d\n",
						pwszCardinality, pwszQualifiedName,	pwszQualifiedStatName, pwszCardinality, cRows, *(ULONG *)pColumn->bValue);
					
					// Note test bug here Cardinality() doesn't include BLOB columns, so if the
					// statistic includes BLOBs we skip the comparison.
					if (!m_fCardinalityIncludesBlob)
					{
						if(!CCOMPARE(m_EC, cRows == *(ULONG *)pColumn->bValue, 
							EC_BAD_COL_OR_TUPLE_CARD,
							wszMessage,	FALSE))
								odtLog << L"";
					}
				}
			}
		}
		break;
	case TSS_TABLE_CARDINALITY:

		// Detail check

		// This should equal number of rows in table
		if (m_fDetailCheck && SUCCEEDED(hrOpenTable) &&
			pColumn->sStatus == DBSTATUS_S_OK)
		{
			DBCOUNTITEM	cRows = Cardinality(pwszQualifiedName, ulIndex, TABLE_CARDINALITY);

			CCOMPARE(m_EC, cRows == *(ULONG *)pColumn->bValue, 
				EC_BAD_TABLE_CARDINALITY,
				L"Table cardinality is incorrect",
				FALSE);
		}
		break;
	case TSS_AVG_COLUMN_LENGTH:
		break;
	}

CLEANUP:

	SAFE_FREE(pwszQualifiedName);
	SAFE_FREE(pwszQualifiedStatName);

	return fResults;
}

// Referential Constraints schema specific functions
class CReferentialConstraintsHelper : public CHelper
{ 

private:


protected:

public:

	CReferentialConstraintsHelper(void) {m_guidSchema = DBSCHEMA_REFERENTIAL_CONSTRAINTS;}
};

// Schemata schema specific functions
class CSchemataHelper : public CHelper
{ 

private:


protected:

public:

	CSchemataHelper(void) {m_guidSchema = DBSCHEMA_SCHEMATA;}
};

// Statistics schema specific functions
class CStatisticsHelper : public CHelper
{ 

private:


protected:

public:

	CStatisticsHelper(void) {m_guidSchema = DBSCHEMA_STATISTICS;}
};

// Trustee schema specific functions
class CTrusteeHelper : public CHelper
{ 

private:


protected:

public:

	CTrusteeHelper(void) {m_guidSchema = DBSCHEMA_TRUSTEE;}
};

// Views schema specific functions
class CViewsHelper : public CHelper
{ 

private:


protected:

public:

	CViewsHelper(void) {m_guidSchema = DBSCHEMA_VIEWS;}
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(CCommon)
//--------------------------------------------------------------------
// @class test IDBSchemaRowset::GetRowset
//
class CCommon : public CSchemaTest { 
protected:
	CHelper * m_pCHelper;

	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CCommon,CSchemaTest);
	// }} TCW_DECLARE_FUNCS_END
 
	CCommon(void) {m_pCHelper = NULL;}

	void SetHelper(CHelper * pCHelper) {m_pCHelper = pCHelper;}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: No restrictions, no props
	int Variation_1();
	// @cmember E_INVALIDARG: rguidSchema = DBPROPSET_ROWSET
	int Variation_2();
	// @cmember E_INVALIDARG: rguidSchema = GUID_NULL
	int Variation_3();
	// @cmember DB_E_NOTSUPPORTED: provider does not support restrictions
	int Variation_4();
	// @cmember S_OK: pass all supported restrictions
	int Variation_5();
	// @cmember S_OK: less than max restrictions
	int Variation_6();
	// @cmember E_INVALIDARG: more than max restrictions
	int Variation_7();
	// @cmember E_NOINTERFACE: riid is not initialized
	int Variation_8();
	// @cmember S_OK: restrictions valid for 1st and 3rd restrictions
	int Variation_9();
	// @cmember S_OK: 2 cmd objects open on session
	int Variation_10();
	// @cmember DB_E_ERRORSOCCURED: update properties on read-only rowset
	int Variation_11();
	// @cmember DB_S_ERRORSOCCURED: DBPROP_IRowsetInfo & DBPROP_IRowsetChange
	int Variation_12();
	// @cmember S_OK: DBPROP_BOOKMARKS
	int Variation_13();
	// @cmember S_OK: empty variants = no restricions passed
	int Variation_14();
	// @cmember S_OK or E_NOINTERFACE: all rowset riids
	int Variation_15();
	// @cmember E_NOINTERFACE: riid = IID_IDBProperties
	int Variation_16();
	// @cmember E_INVALIDARG: ppRowset = NULL
	int Variation_17();
	// @cmember E_INVALIDARG: cRestrictions > 0, rgRestrictions = NULL
	int Variation_18();
	// @cmember E_INVALIDARG: cProperty != 0, rgProperties = NULL
	int Variation_19();
	// @cmember E_INVALIDARG: cPropertySets > 0, rgPropertySets = NULL
	int Variation_20();
	// @cmember E_INVALIDARG: invalid restriction with wrong VT type
	int Variation_21();
	// @cmember S_OK: request DBPROP_IColumnsRowset and, iid = IID_IColumnsRowset
	int Variation_22();
	// @cmember S_OK: riid = IID_IRowsetInfo, call IRowset::GetSpecification
	int Variation_23();
	// @cmember S_OK: riid = IID_IUnknown, get IRowset from IUnknown
	int Variation_24();
	// @cmember S_OK: open rowset from schema, try to open rowset on command
	int Variation_25();
	// @cmember S_OK: IRowsetScroll
	int Variation_26();
	// @cmember E_INVALIDARG: Schema not supported
	int Variation_27();
	// @cmember S_OK: Empty result set, pass first restriction that matches second restriction
	int Variation_28();
	// @cmember S_OK: Empty result set, pass non-matching value for each restriction
	int Variation_29();
	// @cmember First restriction
	int Variation_30();
	// @cmember Second restriction
	int Variation_31();
	// @cmember Third restriction
	int Variation_32();
	// @cmember Fourth restriction
	int Variation_33();
	// @cmember Fifth restriction
	int Variation_34();
	// @cmember Sixth restriction
	int Variation_35();
	// @cmember Seventh restriction
	int Variation_36();
	// @cmember S_OK: all rowset properties as required
	int Variation_37();
	// @cmember S_OK: all rowset properties as optional
	int Variation_38();
	// @cmember S_OK: all rowset properties, 2 at time, 1 optional, 1 required
	int Variation_39();
	// @cmember DB_E_ERRORSOCCURRED: non-rowset property sets, all properties in that set
	int Variation_40();
	// @cmember Security: Maximum sized string restriction
	int Variation_41();
	// @cmember Security: Max size plus one string restriction
	int Variation_42();
	// @cmember Security: Extremely large string restriction
	int Variation_43();
	// @cmember Security: Use %s in string restriction
	int Variation_44();
	// @cmember Security: Use recursive variant in restriction
	int Variation_45();
	// @cmember Security: Use unclosed quote in restriction
	int Variation_46();
	// @cmember Synonyms: Use table synonym in table restriction
	int Variation_47();
	// @cmember Synonyms: Use SYNONYM in table type restriction
	int Variation_48();
	// @cmember Synonyms: Use synonym for table name restriction and SYNONYM in table type restriction.
	int Variation_49();
	// @cmember Security: SQLBU #363546: Use max size string restriction of closing square brackets
	int Variation_50();
	// @cmember Security: SQLBU #363546: Use extremely large string restriction of closing square brackets
	int Variation_51();
	// @cmember SQLBU #395368: Use a sortid that does not have a SQL collation
	int Variation_52();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CCommon)
#define THE_CLASS CCommon
BEG_TEST_CASE(CCommon, CSchemaTest, L"test IDBSchemaRowset::GetRowset")
	TEST_VARIATION(1, 		L"S_OK: No restrictions, no props")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rguidSchema = DBPROPSET_ROWSET")
	TEST_VARIATION(3, 		L"E_INVALIDARG: rguidSchema = GUID_NULL")
	TEST_VARIATION(4, 		L"DB_E_NOTSUPPORTED: provider does not support restrictions")
	TEST_VARIATION(5, 		L"S_OK: pass all supported restrictions")
	TEST_VARIATION(6, 		L"S_OK: less than max restrictions")
	TEST_VARIATION(7, 		L"E_INVALIDARG: more than max restrictions")
	TEST_VARIATION(8, 		L"E_NOINTERFACE: riid is not initialized")
	TEST_VARIATION(9, 		L"S_OK: restrictions valid for 1st and 3rd restrictions")
	TEST_VARIATION(10, 		L"S_OK: 2 cmd objects open on session")
	TEST_VARIATION(11, 		L"DB_E_ERRORSOCCURED: update properties on read-only rowset")
	TEST_VARIATION(12, 		L"DB_S_ERRORSOCCURED: DBPROP_IRowsetInfo & DBPROP_IRowsetChange")
	TEST_VARIATION(13, 		L"S_OK: DBPROP_BOOKMARKS")
	TEST_VARIATION(14, 		L"S_OK: empty variants = no restricions passed")
	TEST_VARIATION(15, 		L"S_OK or E_NOINTERFACE: all rowset riids")
	TEST_VARIATION(16, 		L"E_NOINTERFACE: riid = IID_IDBProperties")
	TEST_VARIATION(17, 		L"E_INVALIDARG: ppRowset = NULL")
	TEST_VARIATION(18, 		L"E_INVALIDARG: cRestrictions > 0, rgRestrictions = NULL")
	TEST_VARIATION(19, 		L"E_INVALIDARG: cProperty != 0, rgProperties = NULL")
	TEST_VARIATION(20, 		L"E_INVALIDARG: cPropertySets > 0, rgPropertySets = NULL")
	TEST_VARIATION(21, 		L"E_INVALIDARG: invalid restriction with wrong VT type")
	TEST_VARIATION(22, 		L"S_OK: request DBPROP_IColumnsRowset and, iid = IID_IColumnsRowset")
	TEST_VARIATION(23, 		L"S_OK: riid = IID_IRowsetInfo, call IRowset::GetSpecification")
	TEST_VARIATION(24, 		L"S_OK: riid = IID_IUnknown, get IRowset from IUnknown")
	TEST_VARIATION(25, 		L"S_OK: open rowset from schema, try to open rowset on command")
	TEST_VARIATION(26, 		L"S_OK: IRowsetScroll")
	TEST_VARIATION(27, 		L"E_INVALIDARG: Schema not supported")
	TEST_VARIATION(28, 		L"S_OK: Empty result set, pass first restriction that matches second restriction")
	TEST_VARIATION(29, 		L"S_OK: Empty result set, pass non-matching value for each restriction")
	TEST_VARIATION(30, 		L"First restriction")
	TEST_VARIATION(31, 		L"Second restriction")
	TEST_VARIATION(32, 		L"Third restriction")
	TEST_VARIATION(33, 		L"Fourth restriction")
	TEST_VARIATION(34, 		L"Fifth restriction")
	TEST_VARIATION(35, 		L"Sixth restriction")
	TEST_VARIATION(36, 		L"Seventh restriction")
	TEST_VARIATION(37, 		L"S_OK: all rowset properties as required")
	TEST_VARIATION(38, 		L"S_OK: all rowset properties as optional")
	TEST_VARIATION(39, 		L"S_OK: all rowset properties, 2 at time, 1 optional, 1 required")
	TEST_VARIATION(40, 		L"DB_E_ERRORSOCCURRED: non-rowset property sets, all properties in that set")
	TEST_VARIATION(41, 		L"Security: Maximum sized string restriction")
	TEST_VARIATION(42, 		L"Security: Max size plus one string restriction")
	TEST_VARIATION(43, 		L"Security: Extremely large string restriction")
	TEST_VARIATION(44, 		L"Security: Use %s in string restriction")
	TEST_VARIATION(45, 		L"Security: Use recursive variant in restriction")
	TEST_VARIATION(46, 		L"Security: Use unclosed quote in restriction")
	TEST_VARIATION(47, 		L"Synonyms: Use table synonym in table restriction")
	TEST_VARIATION(48, 		L"Synonyms: Use SYNONYM in table type restriction")
	TEST_VARIATION(49, 		L"Synonyms: Use synonym for table name restriction and SYNONYM in table type restriction.")
	TEST_VARIATION(50, 		L"Security: SQLBU #363546: Use max size string restriction of closing square brackets")
	TEST_VARIATION(51, 		L"Security: SQLBU #363546: Use extremely large string restriction of closing square brackets")
	TEST_VARIATION(52, 		L"SQLBU #395368: Use a sortid that does not have a SQL collation")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CGetSchema)
//--------------------------------------------------------------------
// @class test IDBSchemaRowset::GetSchemas
//
class CGetSchema : public CSchemaTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CGetSchema,CSchemaTest);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: pcSchemas = NULL
	int Variation_1();
	// @cmember S_OK: *prgSchemas = NULL, pcSchemas = 0
	int Variation_2();
	// @cmember S_OK: prgRestrictionSupported = NULL
	int Variation_3();
	// @cmember E_INVALIDARG: prgSchemas = NULL
	int Variation_4();
	// @cmember S_OK: open schema rowset, try to open rowset from command object
	int Variation_5();
	// @cmember S_OK: don't initialize variables before sending
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CGetSchema)
#define THE_CLASS CGetSchema
BEG_TEST_CASE(CGetSchema, CSchemaTest, L"test IDBSchemaRowset::GetSchemas")
	TEST_VARIATION(1, 		L"E_INVALIDARG: pcSchemas = NULL")
	TEST_VARIATION(2, 		L"S_OK: *prgSchemas = NULL, pcSchemas = 0")
	TEST_VARIATION(3, 		L"S_OK: prgRestrictionSupported = NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG: prgSchemas = NULL")
	TEST_VARIATION(5, 		L"S_OK: open schema rowset, try to open rowset from command object")
	TEST_VARIATION(6, 		L"S_OK: don't initialize variables before sending")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CZombie)
//--------------------------------------------------------------------
// @class testing IDBSchemaRowset in zombie situation
//
class CZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	int TestTxnGetRowset(ETXN eTxn,BOOL fRetaining,	GUID schema);
	int TestTxnGetSchemas(ETXN eTxn,BOOL fRetaining);

	
	// {{ TCW_TESTVARS()
	// @cmember GetRowset: Commit with fRetaining set to TRUE
	int Variation_1();
	// @cmember GetRowset: Commit with fRetaining set to FALSE
	int Variation_2();
	// @cmember GetRowset: Abort with fRetaining set to TRUE
	int Variation_3();
	// @cmember GetRowset: Abort with fRetaining set to FALSE
	int Variation_4();
	// @cmember GetSchemas: Commit with fRetaining set to TRUE
	int Variation_5();
	// @cmember GetSchemas: Commit with fRetaining set to FALSE
	int Variation_6();
	// @cmember GetSchemas: Abort with fRetaining set to TRUE
	int Variation_7();
	// @cmember GetSchemas: Abort with fRetaining set to FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CZombie)
#define THE_CLASS CZombie
BEG_TEST_CASE(CZombie, CTransaction, L"testing IDBSchemaRowset in zombie situation")
	TEST_VARIATION(1, 		L"GetRowset: Commit with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"GetRowset: Commit with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"GetRowset: Abort with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"GetRowset: Abort with fRetaining set to FALSE")
	TEST_VARIATION(5, 		L"GetSchemas: Commit with fRetaining set to TRUE")
	TEST_VARIATION(6, 		L"GetSchemas: Commit with fRetaining set to FALSE")
	TEST_VARIATION(7, 		L"GetSchemas: Abort with fRetaining set to TRUE")
	TEST_VARIATION(8, 		L"GetSchemas: Abort with fRetaining set to FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class testing extended errors on IDBSchemaRowset
//
class ExtendedErrors : public CSchemaTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	CHelper * GetSchemaHelper(GUID guidSchema);
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,CSchemaTest);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid GetSchemas Call with previous error object existing
	int Variation_1();
	// @cmember Valid GetRowset call with previous error object existing
	int Variation_2();
	// @cmember Valid GetSchemas call with previous error object existing
	int Variation_3();
	// @cmember Invalid GetRowset call with previous error object existing
	int Variation_4();
	// @cmember Invalid GetSchemas call with previous error object existing
	int Variation_5();
	// @cmember Invalid GetRowset call with previous error object existing
	int Variation_6();
	// @cmember Open schema rowset  DBSCHEMA_ASSERTIONS -- E_INVALIDARG
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, CSchemaTest, L"testing extended errors on IDBSchemaRowset")
	TEST_VARIATION(1, 		L"Valid GetSchemas Call with previous error object existing")
	TEST_VARIATION(2, 		L"Valid GetRowset call with previous error object existing")
	TEST_VARIATION(3, 		L"Valid GetSchemas call with previous error object existing")
	TEST_VARIATION(4, 		L"Invalid GetRowset call with previous error object existing")
	TEST_VARIATION(5, 		L"Invalid GetSchemas call with previous error object existing")
	TEST_VARIATION(6, 		L"Invalid GetRowset call with previous error object existing")
	TEST_VARIATION(7, 		L"Open schema rowset  DBSCHEMA_ASSERTIONS -- E_INVALIDARG")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CBugRegressions)
//*-----------------------------------------------------------------------
// @class Test case for bug regressions
//
class CBugRegressions : public CSchemaTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CBugRegressions,CSchemaTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember GetRowset on schema TABLES after executing RETURN statement
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CBugRegressions)
#define THE_CLASS CBugRegressions
BEG_TEST_CASE(CBugRegressions, CSchemaTest, L"Test case for bug regressions")
	TEST_VARIATION(1, 		L"GetRowset on schema TABLES after executing RETURN statement")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CTableStatistics)
//*-----------------------------------------------------------------------
// @class Test DBSCHEMA_TABLE_STATISTICS
//
class CTableStatistics : public CSchemaTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CTableStatistics,CSchemaTest);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Validate DBPROP_OPENROWSETSUPPORT, DBPROPVAL_ORS_HISTOGRAM
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(CTableStatistics)
#define THE_CLASS CTableStatistics
BEG_TEST_CASE(CTableStatistics, CSchemaTest, L"Test DBSCHEMA_TABLE_STATISTICS")
	TEST_VARIATION(1, 		L"Validate DBPROP_OPENROWSETSUPPORT, DBPROPVAL_ORS_HISTOGRAM")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()


#define COPY_TEST_CASE(theClass, baseClass)						\
	class theClass : public baseClass							\
	{															\
	public:														\
		static const WCHAR		m_wszTestCaseName[];			\
		DECLARE_TEST_CASE_FUNCS(theClass, baseClass);			\
	};															\
const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\

#define TEST_CASE_WITH_HELPER(iCase, theClass, helperClass)		\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetHelper(new helperClass());	\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;


COPY_TEST_CASE(CAssertionsCommon, CCommon)
COPY_TEST_CASE(CCatalogsCommon, CCommon)
COPY_TEST_CASE(CCharacterSetsCommon, CCommon)
COPY_TEST_CASE(CCheckConstraintsCommon, CCommon)
COPY_TEST_CASE(CCheckConstraintsByTableCommon, CCommon)
COPY_TEST_CASE(CCollationsCommon, CCommon)
COPY_TEST_CASE(CColumnDomainUsageCommon, CCommon)
COPY_TEST_CASE(CColumnPrivilegesCommon, CCommon)
COPY_TEST_CASE(CColumnsCommon, CCommon)
COPY_TEST_CASE(CConstraintColumnUsageCommon, CCommon)
COPY_TEST_CASE(CForeignKeysCommon, CCommon)
COPY_TEST_CASE(CIndexesCommon, CCommon)
COPY_TEST_CASE(CKeyColumnUsageCommon, CCommon)
COPY_TEST_CASE(CPrimaryKeysCommon, CCommon)	
COPY_TEST_CASE(CProceduresCommon, CCommon)	
COPY_TEST_CASE(CProcedureColumnsCommon, CCommon)	
COPY_TEST_CASE(CProcedureParametersCommon, CCommon)	
COPY_TEST_CASE(CProviderTypesCommon, CCommon)	
COPY_TEST_CASE(CReferentialConstraintsCommon, CCommon)	
COPY_TEST_CASE(CSchemataCommon, CCommon)	
COPY_TEST_CASE(CStatisticsCommon, CCommon)	
COPY_TEST_CASE(CTableConstraintsCommon, CCommon)	
COPY_TEST_CASE(CTablePrivilegesCommon, CCommon)	
COPY_TEST_CASE(CTablesInfoCommon, CCommon)	
COPY_TEST_CASE(CTablesCommon, CCommon)
COPY_TEST_CASE(CTableStatisticsCommon, CCommon)	
COPY_TEST_CASE(CTrusteeCommon, CCommon)	
COPY_TEST_CASE(CViewsCommon, CCommon)	


#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(6, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, CCommon)
	TEST_CASE(2, CGetSchema)
	TEST_CASE(3, CZombie)
	TEST_CASE(4, ExtendedErrors)
	TEST_CASE(5, CBugRegressions)
	TEST_CASE(6, CTableStatistics)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(33, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, CGetSchema)
	TEST_CASE_WITH_HELPER(2, CAssertionsCommon, CAssertionsHelper)
	TEST_CASE_WITH_HELPER(3, CCatalogsCommon, CCatalogsHelper)
	TEST_CASE_WITH_HELPER(4, CCharacterSetsCommon, CCharacterSetsHelper)
	TEST_CASE_WITH_HELPER(5, CCheckConstraintsCommon, CCheckConstraintsHelper)
	TEST_CASE_WITH_HELPER(6, CCheckConstraintsByTableCommon, CCheckConstraintsByTableHelper)
	TEST_CASE_WITH_HELPER(7, CCollationsCommon, CCollationsHelper)
	TEST_CASE_WITH_HELPER(8, CColumnDomainUsageCommon, CColumnDomainUsageHelper)
	TEST_CASE_WITH_HELPER(9, CColumnPrivilegesCommon, CColumnPrivilegesHelper)
	TEST_CASE_WITH_HELPER(10, CColumnsCommon, CColumnsHelper)
	TEST_CASE_WITH_HELPER(11, CConstraintColumnUsageCommon, CConstraintColumnUsageHelper)
	TEST_CASE_WITH_HELPER(12, CForeignKeysCommon, CForeignKeysHelper)
	TEST_CASE_WITH_HELPER(13, CIndexesCommon, CIndexesHelper)
	TEST_CASE_WITH_HELPER(14, CKeyColumnUsageCommon, CKeyColumnUsageHelper)
	TEST_CASE_WITH_HELPER(15, CPrimaryKeysCommon, CPrimaryKeysHelper)
	TEST_CASE_WITH_HELPER(16, CProceduresCommon, CProceduresHelper)
	TEST_CASE_WITH_HELPER(17, CProcedureColumnsCommon, CProcedureColumnsHelper)
	TEST_CASE_WITH_HELPER(18, CProcedureParametersCommon, CProcedureParametersHelper)
	TEST_CASE_WITH_HELPER(19, CProviderTypesCommon, CProviderTypesHelper)
	TEST_CASE_WITH_HELPER(20, CReferentialConstraintsCommon, CReferentialConstraintsHelper)
	TEST_CASE_WITH_HELPER(21, CSchemataCommon, CSchemataHelper)
	TEST_CASE_WITH_HELPER(22, CStatisticsCommon, CStatisticsHelper)
	TEST_CASE_WITH_HELPER(23, CTableConstraintsCommon, CTableConstraintsHelper)
	TEST_CASE_WITH_HELPER(24, CTablePrivilegesCommon, CTablePrivilegesHelper)
	TEST_CASE_WITH_HELPER(25, CTablesInfoCommon, CTablesInfoHelper)
	TEST_CASE_WITH_HELPER(26, CTablesCommon, CTablesHelper)
	TEST_CASE_WITH_HELPER(27, CTableStatisticsCommon, CTableStatisticsHelper)
	TEST_CASE_WITH_HELPER(28, CTrusteeCommon, CTrusteeHelper)
	TEST_CASE_WITH_HELPER(29, CViewsCommon, CViewsHelper)
	TEST_CASE(30, CZombie)
	TEST_CASE(31, ExtendedErrors)
	TEST_CASE(32, CBugRegressions)
	TEST_CASE(33, CTableStatistics)
END_TEST_MODULE()
#endif

// {{ TCW_TC_PROTOTYPE(CCommon)
//*-----------------------------------------------------------------------
//| Test Case:		CCommon - test IDBSchemaRowset::GetRowset
//|	Created:			09/23/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommon::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSchemaTest::Init())
	// }}
	{
		if (COMPARE(m_pCHelper != NULL, TRUE))
			return m_pCHelper->Init(this);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: No restrictions, no props
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_1()
{
	HRESULT hrExp = S_OK;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_NULL), hrExp);

	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rguidSchema = DBPROPSET_ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_2()
{
	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_NULL, NULL, IID_IRowset, 0, NULL,
		NULL, DBPROPSET_ROWSET), E_INVALIDARG);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rguidSchema = GUID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_3()
{
	IRowset * pIRowset = NULL;

	// Have to turn off setting of default arguments
	m_pCHelper->UseArgumentDefaults(FALSE);

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, 0, NULL, IID_IRowset, 0, NULL,
		(IUnknown **)&pIRowset, GUID_NULL), E_INVALIDARG);

	// Set default args back on
	m_pCHelper->UseArgumentDefaults(TRUE);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTSUPPORTED: provider does not support restrictions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_4()
{
	HRESULT hr = E_FAIL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if (!m_pCHelper->HasUnsupportedRestriction())
	{
		odtLog << L"No unsupported restrictions.\n";
		return TEST_SKIPPED;
	}

	// Set to use all cached restrictions, supported or not
	m_pCHelper->SetRestrictionArray(RT_CACHED);

	// Test method
	hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL);

	// Provider may return E_INVALIDARG here, but we prefer DB_E_NOTSUPPORTED
	if (hr != E_INVALIDARG)
		CHECK(hr, DB_E_NOTSUPPORTED);

	// Set back to use supported restrictions
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: pass all supported restrictions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_5()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(), S_OK);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: less than max restrictions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_6()
{
	ULONG cRestrictions = m_pCHelper->RestrictionCount();

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, cRestrictions-1), S_OK);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: more than max restrictions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_7()
{
	ULONG cRestrictions = m_pCHelper->RestrictionCount();

	// Test method.  Note spec change that providers may return E_INVALIDARG or
	// DB_E_NOTSUPPORTED for this case.
	TEST2C_(m_pCHelper->GetRowsetHelper(NULL, cRestrictions+1), DB_E_NOTSUPPORTED, E_INVALIDARG);

CLEANUP:

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: riid is not initialized
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_8()
{
	GUID iid;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	memset(&iid, 0x0c, sizeof(GUID));

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, 0, NULL, iid), E_NOINTERFACE);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: restrictions valid for 1st and 3rd restrictions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_9()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if (!m_pCHelper->IsSupportedRestriction(RV_ONE) &&
		!m_pCHelper->IsSupportedRestriction(RV_THREE))
	{
		odtLog << L"1st and 3rd restrictions not supported.\n";
		return TEST_SKIPPED;
	}

	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	m_pCHelper->ClearRestrictions();

	if (m_pCHelper->IsSupportedRestriction(RV_ONE))
		m_pCHelper->SetValidRestriction(RV_ONE);
	else
		odtLog << L"1st retriction not supported, setting only 3rd.\n";

	if (m_pCHelper->IsSupportedRestriction(RV_THREE))
		m_pCHelper->SetValidRestriction(RV_THREE);
	else
		odtLog << L"3rd retriction not supported, setting only 1st.\n";

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(), S_OK);

	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 2 cmd objects open on session
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_10()
{
	ICommand *		pICommand = NULL;
	BOOL fResult =  FALSE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// If the provider doesn't support commands then 
	// m_pIDBCreateCommand is NULL
	if (!m_pIDBCreateCommand)
	{
		odtLog << L"Commands not supported.\n";
		return TEST_SKIPPED;
	}
	
	// create a second object open on the session object
	TESTC_(m_pIDBCreateCommand->CreateCommand(
		NULL,
		IID_ICommand,
		(IUnknown **)&pICommand), S_OK);

	// test method
	CHECK(m_pCHelper->GetRowsetHelper(), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	SAFE_RELEASE(pICommand);

	return fResult;

}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURED: update properties on read-only rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_11()
{
	ULONG	ulIndex=0;
	ULONG	ulIndexProperties=0;
	ULONG	ulprop=0;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	DBTYPE wPropType;
	LPVOID lpvPropVal;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// for each invalid property
	for(ulIndexProperties=0;ulIndexProperties<cUPDATE_PROPERTIES;ulIndexProperties++)
	{
		if( (IsRowsetPropertySupported(rgUpdateProperties[ulIndexProperties])) || 
			(m_pTable->get_ICommandPTR()	&&
			(GetProperty(rgUpdateProperties[ulIndexProperties],DBPROPSET_ROWSET,m_pTable->get_ICommandPTR()))))
		{
			odtLog << L"---" << rgwszUpdateProperties[ulIndexProperties] << L"-\n" ;

			// Most props are BOOL, but DBPROP_UPDATABILITY is I4
			if (rgUpdateProperties[ulIndexProperties] == DBPROP_UPDATABILITY)
			{
				wPropType = DBTYPE_I4;
				lpvPropVal = (LPVOID)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE);
			}
			else
			{
				wPropType = DBTYPE_BOOL;
				lpvPropVal = (LPVOID)VARIANT_TRUE;
			}


			TESTC_(SetProperty(rgUpdateProperties[ulIndexProperties], DBPROPSET_ROWSET,
				&cPropSet, &pPropSet, wPropType, &lpvPropVal), S_OK);
			
			// test method
			CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet), DB_E_ERRORSOCCURRED);

			// There is some ambiguity on what DBPROPSTATUS to allow for schema rowsets since they are
			// r/o and therefore don't support some rowset props.  We prefer DBPROPSTATUS_CONFLICTING
			// since that conflicts with DBPROP_UPDATABILITY, but we will allow others here such
			// as DBPROPSTATUS_NOTSETTABLE.
			if (pPropSet[0].rgProperties[0].dwStatus != DBPROPSTATUS_NOTSETTABLE)
			{
				VerifyPropStatus(cPropSet, pPropSet, rgUpdateProperties[ulIndexProperties],
					DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING);
			}

			// Free the property info for next iteration
			FreeProperties(&cPropSet, &pPropSet);

		}
		else
			odtLog << L"Property not supported or not writable, "
			<< rgwszUpdateProperties[ulIndexProperties]
			<< ENDL;
	}

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURED: DBPROP_IRowsetInfo & DBPROP_IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_12()
{
	BOOL		fResult = TRUE;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if(!IsRowsetPropertySupported(DBPROP_IRowsetInfo))
	{
		odtLog << L"Mandatory interface IRowsetInfo is not supported or not writable\n";
		return TEST_SKIPPED;
	}
	if(!IsRowsetPropertySupported(DBPROP_IRowsetChange))
	{
		odtLog << L"IRowsetChange is not supported or not writable\n";
		return TEST_SKIPPED;
	}

	TESTC_(SetProperty(DBPROP_IRowsetInfo, DBPROPSET_ROWSET,
		&cPropSet, &pPropSet, DBTYPE_BOOL, VARIANT_TRUE), S_OK);

	TESTC_(SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET,
		&cPropSet, &pPropSet, DBTYPE_BOOL, VARIANT_TRUE), S_OK);

	// test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet), DB_S_ERRORSOCCURRED);

	if (!VerifyPropStatus(cPropSet, pPropSet, DBPROP_IRowsetInfo, DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING))
		goto CLEANUP;
	if (!VerifyPropStatus(cPropSet, pPropSet, DBPROP_IRowsetChange, DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING))
		goto CLEANUP;

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_13()
{
	BOOL		fResult = TRUE;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	HRESULT hr = E_FAIL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if(!IsRowsetPropertySupported(DBPROP_BOOKMARKS))
	{
		odtLog << L"Bookmark not supported\n";
		return TEST_SKIPPED;
	}

	TESTC_(SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET,
		&cPropSet, &pPropSet, DBTYPE_BOOL, VARIANT_TRUE), S_OK);

	// test method
	hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet);

	if (S_OK == hr)
	{
		if (!VerifyPropStatus(cPropSet, pPropSet, DBPROP_BOOKMARKS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
			goto CLEANUP;
	}
	else
	{
		TESTC_(hr, DB_E_ERRORSOCCURRED);
		if (!VerifyPropStatus(cPropSet, pPropSet, DBPROP_BOOKMARKS, DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING))
			goto CLEANUP;
	}

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: empty variants = no restricions passed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_14()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use our custom restriction array
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Set them all to VT_EMPTY
	m_pCHelper->ClearRestrictions();

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(), S_OK);

	// Set back to supported restrictions
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);
	m_pCHelper->SetRowCount(MIN_VALUE, 1);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK or E_NOINTERFACE: all rowset riids
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_15()
{
	ULONG i, cRowsetIIDs = 0;
	INTERFACEMAP* rgRowsetIIDs = NULL;
	IUnknown * pIUnknown = NULL;
	HRESULT hr = E_FAIL;
	BOOL fResult = FALSE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	//Obtain the Rowset IIDs
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cRowsetIIDs, &rgRowsetIIDs));

    //Loop through all rowset IIDs...
	for(i=0; i<cRowsetIIDs; i++)
	{
		odtLog << rgRowsetIIDs[i].pwszName << L"\n";

		//Asking for IID_I* is requesting a rowset that supports this interface
		//This is implicilty like requesting DBPROP_I* ahead of time...
		hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, *rgRowsetIIDs[i].pIID,
			0, NULL, &pIUnknown, GUID_NULL, FALSE);
		
		// Allow S_OK or E_NOINTERFACE
		if (S_OK != hr)
		{
			CHECK(hr, E_NOINTERFACE);
		}
	
		//Success, verify this interface...
		if(hr == S_OK)
		{
			if(!ValidInterface(*rgRowsetIIDs[i].pIID, pIUnknown))
				TERROR(L"Interface Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");

			TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));
		}
		else
		{
			//Make sure this is allowed to not be required
			TCOMPARE_(!rgRowsetIIDs[i].fMandatory);
		}
		
	    SAFE_RELEASE(pIUnknown);
	}

	fResult = TRUE;

CLEANUP:

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: riid = IID_IDBProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_16()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, 0, NULL, IID_IDBProperties), E_NOINTERFACE);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppRowset = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_17()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Have to turn off setting of default arguments
	m_pCHelper->UseArgumentDefaults(FALSE);

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, 0, NULL, IID_IRowset, 0, NULL,
		NULL, m_pCHelper->GetSchema()), E_INVALIDARG);

	// Set default args back on
	m_pCHelper->UseArgumentDefaults(TRUE);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cRestrictions > 0, rgRestrictions = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_18()
{
	IRowset * pIRowset = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Have to turn off setting of default arguments
	m_pCHelper->UseArgumentDefaults(FALSE);

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, 1, NULL, IID_IRowset, 0, NULL,
		(IUnknown **)&pIRowset, m_pCHelper->GetSchema()), E_INVALIDARG);

	// Set default args back on
	m_pCHelper->UseArgumentDefaults(TRUE);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cProperty != 0, rgProperties = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_19()
{
	DBPROPSET rgPropSet[1];

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	rgPropSet[0].cProperties = 1;
	rgPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	rgPropSet[0].rgProperties = NULL;

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, 1,
		rgPropSet), E_INVALIDARG);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cPropertySets > 0, rgPropertySets = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_20()
{
	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, 1,
		NULL), E_INVALIDARG);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: invalid restriction with wrong VT type
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_21()
{
	ULONG		ulIndex=0;
	HRESULT		hrTemp = E_FAIL;
	IDispatch * pIDispatch = new CDispatch;
	BOOL		fSetInvalidRestriction = FALSE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	m_pCHelper->ClearRestrictions();

	if (m_pCHelper->IsSupportedRestriction(RV_ONE))
	{
		m_pIDBSchemaRowset->AddRef();
		m_pCHelper->SetInvalidRestriction(RV_ONE, (LPVOID)m_pIDBSchemaRowset, VT_UNKNOWN);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_TWO))
	{
		pIDispatch->AddRef();
		m_pCHelper->SetInvalidRestriction(RV_TWO, (LPVOID)pIDispatch, VT_DISPATCH);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_THREE))
	{
		m_pCHelper->SetInvalidRestriction(RV_THREE, (LPVOID)(LONG_PTR)hrTemp, VT_ERROR);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_FOUR))
	{
		m_pCHelper->SetInvalidRestriction(RV_FOUR, (LPVOID)(IUnknown **)&(m_pIDBSchemaRowset),
			VT_BYREF|VT_UNKNOWN);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_FIVE))
	{
		m_pCHelper->SetInvalidRestriction(RV_FIVE, (LPVOID)&pIDispatch,
			VT_BYREF|VT_DISPATCH);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_SIX))
	{
		m_pCHelper->SetInvalidRestriction(RV_SIX, (LPVOID)&hrTemp,
			VT_BYREF|VT_ERROR);
		fSetInvalidRestriction = TRUE;
	}

	if (m_pCHelper->IsSupportedRestriction(RV_SEVEN))
	{
		m_pCHelper->SetInvalidRestriction(RV_SEVEN, (LPVOID)&hrTemp,
			VT_BYREF|VT_ERROR);
		fSetInvalidRestriction = TRUE;
	}

	if (!fSetInvalidRestriction)
	{
		odtLog << L"No restrictions supported.\n";
		goto CLEANUP;
	}

	CHECK(m_pCHelper->GetRowsetHelper(), E_INVALIDARG);

CLEANUP:

	// Clear the bogus restrictions
	m_pCHelper->ClearRestrictions();

	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	SAFE_RELEASE(pIDispatch);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK: request DBPROP_IColumnsRowset and, iid = IID_IColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_22()
{
	BOOL		fResult = TRUE;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	HRESULT hr = E_FAIL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if(!IsRowsetPropertySupported(DBPROP_IColumnsRowset))
	{
		odtLog << L"IColumnsRowset is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC_(SetProperty(DBPROP_IColumnsRowset, DBPROPSET_ROWSET,
		&cPropSet, &pPropSet, DBTYPE_BOOL, VARIANT_TRUE), S_OK);

	// test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IColumnsRowset, cPropSet, pPropSet), S_OK);

	if (!VerifyPropStatus(cPropSet, pPropSet, DBPROP_IColumnsRowset, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
		goto CLEANUP;

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK: riid = IID_IRowsetInfo, call IRowset::GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_23()
{
	HRESULT hr = E_FAIL;
	IUnknown * pIUnknown = NULL;
	IRowsetInfo * pIRowsetInfo = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowsetInfo, 0,
		NULL, (IUnknown **)&pIRowsetInfo, GUID_NULL, FALSE), S_OK);

	TESTC(pIRowsetInfo != NULL);

	// GetSpecification should succeed
	hr = pIRowsetInfo->GetSpecification(IID_IDBSchemaRowset,&pIUnknown);

	if (hr == S_FALSE)
	{
		// Kagera - Temp table has no way of getting back to kagera's objects so it
		// returns S_FALSE

		// While this is allowed it's certainly unusual, print warning
		odtLog << L"WARNING - GetSpecification returned S_FALSE.\n";
		odtLog << L"This is allowed but may signify a failure for this provider.\n";

		// If we claim we didn't return an object make sure
		COMPARE(pIUnknown, NULL);
	}
	else
	{
		// We should have succeeded and returned a valid object pointer
		if (CHECK(hr, S_OK) && COMPARE(pIUnknown != NULL, TRUE))
		{
			IOpenRowset * pIOpenRowset = NULL;

			// Schema rowsets are always opened via IDBSchemaRowset, therefore GetSpecification
			// should return a session interface.
			if (COMPARE(VerifyInterface(pIUnknown, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset),TRUE))
			{
				SAFE_RELEASE(pIOpenRowset);
			}

		}
	}

CLEANUP:
				
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnknown);

	return TEST_PASS;
}

// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc S_OK: riid = IID_IUnknown, get IRowset from IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_24()
{
	IUnknown * pIUnknown = NULL;
	IRowset * pIRowset = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IUnknown, 0,
		NULL, &pIUnknown, GUID_NULL, FALSE), S_OK);

	TESTC(pIUnknown != NULL);

	TESTC(VerifyInterface(pIUnknown, IID_IRowset, 
						ROWSET_INTERFACE, (IUnknown**)&pIRowset))

	// RestartPosition should succeed
	TEST2C_(pIRowset->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED);

CLEANUP:

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnknown);

	return TRUE;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc S_OK: open rowset from schema, try to open rowset on command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_25()
{
	ULONG				ulIndex=0;
	IRowset *			pICommandRowset=NULL;
	ICommandText *		pICommandText=NULL;
	WCHAR *				pSQLText=NULL;
	IDBCreateCommand *	pIDBCreateCommand=NULL;
	IRowset *			pIRowset = NULL;
	BOOL				fResult = FALSE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if(m_pIDBCreateCommand)
	{
		TESTC_(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommandText,(IUnknown**)&pICommandText),S_OK);
	}
	else
	{
		// Check to see if commands are supported
		TESTC_PROVIDER(VerifyInterface(m_pIOpenRowset, IID_IDBCreateCommand, 
							SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
		
		TESTC_(pIDBCreateCommand->CreateCommand(NULL,IID_ICommandText,(IUnknown**)&pICommandText),S_OK);
	}

	TESTC_(m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL,m_pTable->GetTableName(),&pSQLText,NULL,NULL),S_OK);

	TESTC_(pICommandText->SetCommandText(DBGUID_DBSQL,pSQLText),S_OK);

	// Test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, 0,
		NULL, (IUnknown **)&pIRowset, GUID_NULL, FALSE), S_OK);

	TESTC(pIRowset != NULL);

	TESTC_(pICommandText->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown **)&pICommandRowset),S_OK);

	fResult = TRUE;

CLEANUP:

	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommandRowset);
	SAFE_RELEASE(pIRowset);

	// Free the memory
	SAFE_FREE(pSQLText);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetScroll
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_26()
{
	BOOL		fResult = TRUE;
	IRowsetScroll * pIRowsetScroll = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		phRow = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	if(!IsRowsetPropertySupported(DBPROP_IRowsetScroll))
	{
		odtLog << L"IRowsetScroll is not supported.\n";
		return TEST_SKIPPED;
	}

	// test method
	TESTC_(m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowsetScroll, 0,
		NULL, (IUnknown **)&pIRowsetScroll, GUID_NULL, FALSE), S_OK);

	TESTC(pIRowsetScroll != NULL);

	TESTC_(pIRowsetScroll->GetRowsAtRatio(NULL, NULL, 1, 2, 1, &cRowsObtained, &phRow), S_OK);

	TESTC(cRowsObtained);

	TESTC(phRow != NULL);

	fResult = TRUE;

CLEANUP:

	if (phRow && pIRowsetScroll)
		CHECK(pIRowsetScroll->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL), S_OK);
	SAFE_FREE(phRow);
	SAFE_RELEASE(pIRowsetScroll);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//--------------------------------------------------------------------
// @mfunc E_INVALIDARG: Schema not supported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCommon::Variation_27()
{
	HRESULT hrExp = S_OK;

	if (m_pCHelper->IsSupported())
	{
		odtLog << L"Schema is supported.\n";
		return TEST_SKIPPED;
	}

	CHECK(m_pCHelper->GetRowsetHelper(NULL, RV_NULL), E_INVALIDARG);

	return TEST_PASS;
}


// }}




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty result set, pass first restriction that matches second restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_28()
{ 
	BOOL fResult = FALSE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Must support both restrictions
	if (!m_pCHelper->IsSupportedRestriction(RV_ONE) ||
		!m_pCHelper->IsSupportedRestriction(RV_TWO))
	{
		odtLog << L"First or second restriction not suported.\n";
		return TEST_SKIPPED;
	}

	// The VT must match between first and second restriction
	if (V_VT(m_pCHelper->GetRestrictPtr(RV_ONE)) !=
		V_VT(m_pCHelper->GetRestrictPtr(RV_TWO)))
	{
		odtLog << L"First and second restriction are different types.\n";
		return TEST_SKIPPED;
	}

	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	m_pCHelper->ClearRestrictions();

	// Set first restriction to a valid value
	m_pCHelper->SetValidRestriction(RV_ONE);

	// Set second restriction to first restriction's value
	TESTC(m_pCHelper->SetRestriction(RV_TWO, m_pCHelper->GetRestrictPtr(RV_ONE)));

	// Tell helper to expect 0 rows
	m_pCHelper->SetRowCount(EXACT_VALUE, 0);

	// Get the rowset
	TESTC_(m_pCHelper->GetRowsetHelper(), S_OK);

	fResult = TRUE;

CLEANUP:

	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty result set, pass non-matching value for each restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_29()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// It's impossible to come up with a nonmatching
		// value for boolean restrictions without first going 
		// the entire rowset, so we'll skip boolean (VARIANT_BOOL)
		// restrictions.
		if (V_VT(pVariant) == VT_BOOL)
			continue;

		// Now reset to a bogus nonmatching value.  We assume all available
		// bits set on will not match any restriction.
		if (V_VT(pVariant) == VT_BSTR)
			V_BSTR(pVariant)[0] = L'z';
//			memset(V_BSTR(pVariant), 0x55,
//				wcslen(V_BSTR(pVariant))*sizeof(WCHAR));
		else
			// For all other types just set all bits on. At this time there
			// are no BYREF restrictions.
			memset(&(pVariant->bVal), 0xF, 
				sizeof(VARIANT)-sizeof(VARTYPE)-sizeof(USHORT)*3);


		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset
		TESTC_(m_pCHelper->GetRowsetHelper(), S_OK);

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc First restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_30()
{ 
	return m_pCHelper->TestRestriction(RV_ONE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Second restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_31()
{ 
	return m_pCHelper->TestRestriction(RV_TWO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Third restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_32()
{ 
	return m_pCHelper->TestRestriction(RV_THREE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Fourth restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_33()
{ 
	return m_pCHelper->TestRestriction(RV_FOUR);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Fifth restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_34()
{ 
	return m_pCHelper->TestRestriction(RV_FIVE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Sixth restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_35()
{ 
	return m_pCHelper->TestRestriction(RV_SIX);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Seventh restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_36()
{ 
	return m_pCHelper->TestRestriction(RV_SEVEN);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc S_OK: all rowset properties as required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_37()
{ 
	// TODO: Consolidate vars 37, 38, and 39.
	
	BOOL	fResult = FALSE;
	ULONG	cPropInfos = 0;
	DBPROPINFO * pPropInfos = NULL;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	LPVOID	lpvPropVal;
	HRESULT hr = E_FAIL;
	DBPROPSTATUS dwPropStatusExpected;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	cPropInfos = m_rgRowsetDBPROPINFOSET[0].cPropertyInfos;
	pPropInfos = m_rgRowsetDBPROPINFOSET[0].rgPropertyInfos;

	// Spin through each rowset prop
	for(ULONG iProp=0; iProp < cPropInfos; iProp++)
	{
		DBPROPID dwPropertyID = pPropInfos[iProp].dwPropertyID; 
		VARTYPE vt = pPropInfos[iProp].vtType;

		odtLog << pPropInfos[iProp].pwszDescription << L"\n";

		// If this is a BOOLEAN property set on
		if (vt == VT_BOOL)
			lpvPropVal = (LPVOID)VARIANT_TRUE;
		else
		{
			// Value has to be specifically set for each property
			switch(dwPropertyID)
			{
				case DBPROP_ACCESSORDER:
					lpvPropVal = (LPVOID)(DBPROPVAL_AO_RANDOM);
					break;
				case DBPROP_UPDATABILITY:
					lpvPropVal = (LPVOID)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE);
					break;
				default:
					odtLog << L"\tProperty not yet tested.\n";
					continue;
			}
		}

		TESTC_(SetProperty(dwPropertyID, DBPROPSET_ROWSET,
			&cPropSet, &pPropSet, vt, &lpvPropVal), S_OK);

		TEST2C_(hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet), 
			S_OK, DB_E_ERRORSOCCURRED);
		
		if (S_OK == hr)
			dwPropStatusExpected = DBPROPSTATUS_OK;
		else if (!(pPropInfos[iProp].dwFlags & DBPROPFLAGS_WRITE))
		{
			dwPropStatusExpected = DBPROPSTATUS_NOTSETTABLE;
		}
		else
		{
			// It has been determined that the proper status here is DBPROPSTATUS_CONFLICTING
			// if the provider does not support a given update property on a schema rowset.

			// We will cheat a little on the spec here since schema rowsets don't generally
			// support all the props other rowsets do and we will allow DBPROPSTATUS_NOTSETTABLE
			// even though GetPropertyInfo returns supported and settable.
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSETTABLE)
				dwPropStatusExpected = DBPROPSTATUS_NOTSETTABLE;
			else
				dwPropStatusExpected = DBPROPSTATUS_CONFLICTING;

			// If this is an update prop or the prop can't be supported on a schema rowset the
			// provider may return DBPROPSTATUS_NOTSUPPORTED.  Since we don't have any way
			// to know what props the provider actually does support against a schema rowset
			// we have to allow this.
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED)
				dwPropStatusExpected = DBPROPSTATUS_NOTSUPPORTED;

			// If this is DBPROP_UPDATABILITY itself, we need to allow
			// DBPROPSTATUS_BADVALUE if the user attempts to set non-zero
			if (pPropSet[0].rgProperties[0].dwPropertyID == DBPROP_UPDATABILITY &&
				V_I4(&pPropSet[0].rgProperties[0].vValue) != 0 &&
				pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE)
				dwPropStatusExpected = DBPROPSTATUS_BADVALUE;

		}

		// VerifyPropStatus posts its own errors, don't check return.
		VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
			DBPROPSET_ROWSET, dwPropStatusExpected);

		// Free the property info for next iteration
		FreeProperties(&cPropSet, &pPropSet);
	}

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc S_OK: all rowset properties as optional
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_38()
{ 
	BOOL	fResult = FALSE;
	ULONG	cPropInfos = 0;
	DBPROPINFO * pPropInfos = NULL;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	LPVOID	lpvPropVal;
	HRESULT hr = E_FAIL;
	DBPROPSTATUS dwPropStatusExpected;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	cPropInfos = m_rgRowsetDBPROPINFOSET[0].cPropertyInfos;
	pPropInfos = m_rgRowsetDBPROPINFOSET[0].rgPropertyInfos;

	// Spin through each rowset prop
	for(ULONG iProp=0; iProp < cPropInfos; iProp++)
	{
		DBPROPID dwPropertyID = pPropInfos[iProp].dwPropertyID; 
		VARTYPE vt = pPropInfos[iProp].vtType;

		odtLog << pPropInfos[iProp].pwszDescription << L"\n";

		// If this is a BOOLEAN property set on
		if (vt == VT_BOOL)
			lpvPropVal = (LPVOID)VARIANT_TRUE;
		else
		{
			// Value has to be specifically set for each property
			switch(dwPropertyID)
			{
				case DBPROP_ACCESSORDER:
					lpvPropVal = (LPVOID)(DBPROPVAL_AO_RANDOM);
					break;
				case DBPROP_UPDATABILITY:
					lpvPropVal = (LPVOID)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE);
					break;
				default:
					odtLog << L"\tProperty not yet tested.\n";
					continue;
			}
		}

		TESTC_(SetProperty(dwPropertyID, DBPROPSET_ROWSET,
			&cPropSet, &pPropSet, vt, &lpvPropVal, DBPROPOPTIONS_OPTIONAL), S_OK);

		hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet);
		
		// Allow S_OK or DB_S_ERRORSOCCURRED.  Don't use TESTC_ macro because we really don't
		// want to bail from our loop.
		if (hr != S_OK)
			CHECK(hr, DB_S_ERRORSOCCURRED);
		
		if (S_OK == hr)
			dwPropStatusExpected = DBPROPSTATUS_OK;
		else
		{
			dwPropStatusExpected = DBPROPSTATUS_NOTSET;

			// Since the property may not be settable for schema rowset we need to allow
			// DBPROPSTATUS_NOTSETTABLE here also.
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSETTABLE)
				dwPropStatusExpected = DBPROPSTATUS_NOTSETTABLE;

			// Since the property may not be supported for schema rowset we need to allow
			// DBPROPSTATUS_NOTSUPPORTED here also.  Unlike regular rowsets we can't depend
			// on GetPropertyInfo to tell us whether it's supported.
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED)
				dwPropStatusExpected = DBPROPSTATUS_NOTSUPPORTED;

			// Since the property may require other properties to be set (example: sqlncli can not set DBPROP_IDBAsynchStatus until DBPROP_ROWSET_ASYNCH is set), 
			// some providers may return DBPROPSTATUS_CONFLICTING
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_CONFLICTING)
				dwPropStatusExpected = DBPROPSTATUS_CONFLICTING;

			// And, finally, since we don't support updatability for schema rowsets, 
			// some providers may return DBPROPSTATUS_BADVALUE when requesting update support
			if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE)
				dwPropStatusExpected = DBPROPSTATUS_BADVALUE;
		}

		// VerifyPropStatus posts its own errors, don't check return.
		VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
			DBPROPSET_ROWSET, dwPropStatusExpected);

		// Free the property info for next iteration
		FreeProperties(&cPropSet, &pPropSet);
	}

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc S_OK: all rowset properties, 2 at time, 1 optional, 1 required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_39()
{ 
	BOOL	fResult = FALSE;
	ULONG	cPropInfos = 0;
	DBPROPINFO * pPropInfos = NULL;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	LPVOID	lpvPropVal;
	LPVOID	lpvPropValOpt;
	HRESULT hr = E_FAIL;
	BOOL	fStatusOK;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	odtLog << L"This testing is too time consuming and has been removed.\n";
	return TEST_SKIPPED;

	cPropInfos = m_rgRowsetDBPROPINFOSET[0].cPropertyInfos;
	pPropInfos = m_rgRowsetDBPROPINFOSET[0].rgPropertyInfos;

	// Spin through each rowset prop
	for(ULONG iProp=0; iProp < cPropInfos; iProp++)
	{
		for (ULONG jProp = 0; jProp < cPropInfos; jProp++)
		{
			// Don't set same prop both optional and required
			if (iProp == jProp)
				continue;
		
			DBPROPID dwPropertyID = pPropInfos[iProp].dwPropertyID; 
			VARTYPE vt = pPropInfos[iProp].vtType;
			DBPROPID dwPropIDOpt = pPropInfos[jProp].dwPropertyID; 
			VARTYPE vtOpt = pPropInfos[jProp].vtType;

			// If this is a BOOLEAN property set on
			if (vt == VT_BOOL)
				lpvPropVal = (LPVOID)VARIANT_TRUE;
			else
			{
				// Value has to be specifically set for each property
				switch(dwPropertyID)
				{
					case DBPROP_ACCESSORDER:
						lpvPropVal = (LPVOID)(DBPROPVAL_AO_RANDOM);
						break;
					case DBPROP_UPDATABILITY:
						lpvPropVal = (LPVOID)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE);
						break;
					default:
						continue;
				}
			}

			TESTC_(SetProperty(dwPropertyID, DBPROPSET_ROWSET,
				&cPropSet, &pPropSet, vt, &lpvPropVal), S_OK);

			// If this is a BOOLEAN property set on
			if (vtOpt == VT_BOOL)
				lpvPropValOpt = (LPVOID)VARIANT_TRUE;
			else
			{
				// Value has to be specifically set for each property
				switch(dwPropIDOpt)
				{
					case DBPROP_ACCESSORDER:
						lpvPropValOpt = (LPVOID)(DBPROPVAL_AO_RANDOM);
						break;
					case DBPROP_UPDATABILITY:
						lpvPropValOpt = (LPVOID)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE);
						break;
					default:
						FreeProperties(&cPropSet, &pPropSet);
						continue;
				}
			}

			TESTC_(SetProperty(dwPropIDOpt, DBPROPSET_ROWSET,
				&cPropSet, &pPropSet, vtOpt, &lpvPropValOpt, DBPROPOPTIONS_OPTIONAL), S_OK);

			TEST3C_(hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet), 
				S_OK, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);

			fStatusOK = TRUE;
			
			if (S_OK == hr)
			{
				// Both properties were set, verify status
				fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
					DBPROPSET_ROWSET, DBPROPSTATUS_OK);
				fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropIDOpt,
					DBPROPSET_ROWSET, DBPROPSTATUS_OK);
			}
			else if (DB_S_ERRORSOCCURRED == hr)
			{
				// First prop must have been set
				fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
					DBPROPSET_ROWSET, DBPROPSTATUS_OK);

				// Due to ambiguities in the spec the provider may return DBPROPSTATUS_NOTSETTABLE
				// or DBPROPSTATUS_NOTSUPPORTED here, but we prefer DBPROPSTATUS_NOTSET.
				if (pPropSet[0].rgProperties[1].dwStatus != DBPROPSTATUS_NOTSUPPORTED &&
					pPropSet[0].rgProperties[1].dwStatus != DBPROPSTATUS_NOTSETTABLE)
					fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropIDOpt,
						DBPROPSET_ROWSET, DBPROPSTATUS_NOTSET);
			}
			else
			{
				DBPROPSTATUS dwStatusExpected = DBPROPSTATUS_CONFLICTING;
				// First prop must be NOTSETTABLE or CONFLICTING since it was REQUIRED
				if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSETTABLE)
					dwStatusExpected = DBPROPSTATUS_NOTSETTABLE;  // We will allow this 
				if (pPropSet[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED)
					dwStatusExpected = DBPROPSTATUS_NOTSUPPORTED;  // We will allow this 
				fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
					DBPROPSET_ROWSET, dwStatusExpected);
				// Other may have errored also, but may have succeeded
				if (pPropSet[0].rgProperties[1].dwStatus != DBPROPSTATUS_NOTSUPPORTED &&
					pPropSet[0].rgProperties[1].dwStatus != DBPROPSTATUS_NOTSETTABLE)
					fStatusOK &= VerifyPropStatus(cPropSet, pPropSet, dwPropIDOpt,
						DBPROPSET_ROWSET, DBPROPSTATUS_OK);
			}

			if (!fStatusOK)
				odtLog << L"For property combination: " << pPropInfos[iProp].pwszDescription << 
					L" and " << pPropInfos[jProp].pwszDescription << L"\n\n";

			// Free the property info for next iteration
			FreeProperties(&cPropSet, &pPropSet);
		}
	}

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: non-rowset property sets, all properties in that set
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_40()
{ 
	// TODO: Consolidate vars 37, 38, and 39.
	
	BOOL	fResult = FALSE;
	ULONG	cPropInfos = 0;
	DBPROPINFO * pPropInfos = NULL;
	DBPROPSET * pPropSet = NULL;
	ULONG	cPropSet = 0;
	LPVOID	lpvPropVal;
	HRESULT hr = E_FAIL;
	DBPROPSTATUS dwPropStatusExpected;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	cPropInfos = m_rgDBPROPINFOSET[0].cPropertyInfos;
	pPropInfos = m_rgDBPROPINFOSET[0].rgPropertyInfos;

	
	for(ULONG iInfoSet=0; iInfoSet < m_cDBPROPINFOSET; iInfoSet++)
	{
		BOOL fRowsetPropSet = FALSE;

		// We only want the non-rowset props
		for (ULONG iRowsetInfoSet = 0; iRowsetInfoSet < m_cRowsetDBPROPINFOSET; iRowsetInfoSet++)
		{
			if (m_rgDBPROPINFOSET[iInfoSet].guidPropertySet == m_rgRowsetDBPROPINFOSET[iRowsetInfoSet].guidPropertySet)
			{
				fRowsetPropSet = TRUE;
				break;
			}
		}

		if (fRowsetPropSet)
			continue;

		// Spin through each non-rowset prop
		for(ULONG iProp=0; iProp < cPropInfos; iProp++)
		{
			DBPROPID dwPropertyID = pPropInfos[iProp].dwPropertyID; 
			VARTYPE vt = pPropInfos[iProp].vtType;

//			odtLog << pPropInfos[iProp].pwszDescription << L"\n";

			lpvPropVal = (LPVOID)VARIANT_TRUE;
			TESTC_(SetProperty(dwPropertyID, m_rgDBPROPINFOSET[iInfoSet].guidPropertySet,
				&cPropSet, &pPropSet, vt, &lpvPropVal), S_OK);

			TESTC_(hr = m_pCHelper->GetRowsetHelper(NULL, RV_ALL, NULL, IID_IRowset, cPropSet, pPropSet), 
				DB_E_ERRORSOCCURRED);
			
			dwPropStatusExpected = DBPROPSTATUS_NOTSUPPORTED;
			VerifyPropStatus(cPropSet, pPropSet, dwPropertyID,
				m_rgDBPROPINFOSET[iInfoSet].guidPropertySet, dwPropStatusExpected);

			// Free the property info for next iteration
			FreeProperties(&cPropSet, &pPropSet);
		}
	}

	fResult = TRUE;

CLEANUP:

	FreeProperties(&cPropSet, &pPropSet);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Security: Maximum sized string restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_41()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// We don't care about non-BSTR restrictions
		if (V_VT(pVariant) != VT_BSTR)
			continue;

		// Now reset to a maximum value.  Note this value likely does not
		// match any rows, so there should be an empty rowset.

		// Create a restriction value of the appropriate size
		bstrMaxRestriction = SysAllocStringLen(NULL, cchRestrictionMax+1);
		TESTC(bstrMaxRestriction != NULL);

		memset(bstrMaxRestriction, 1, cchRestrictionMax*sizeof(WCHAR));  
		bstrMaxRestriction[cchRestrictionMax] = L'\0';

		VariantClear(pVariant);

		V_VT(pVariant) = VT_BSTR;
		V_BSTR(pVariant) = bstrMaxRestriction;
		bstrMaxRestriction = NULL;  // Because the Variant now owns the BSTR.
					
		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset
		TESTC_(m_pCHelper->GetRowsetHelper(), S_OK);

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Security: Max size plus one string restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_42()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// We don't care about non-BSTR restrictions
		if (V_VT(pVariant) != VT_BSTR)
			continue;

		// Now reset to a maximum plus 1 value.  Note this value likely does not
		// match any rows, so there should be an empty rowset.
		cchRestrictionMax++;

		// Create a restriction value of the appropriate size.  Use max size 
		// plus null terminator.
		bstrMaxRestriction = SysAllocStringLen(NULL, cchRestrictionMax+1);
		TESTC(bstrMaxRestriction != NULL);

		memset(bstrMaxRestriction, 1, cchRestrictionMax*sizeof(WCHAR));  
		bstrMaxRestriction[cchRestrictionMax] = L'\0';

		VariantClear(pVariant);

		V_VT(pVariant) = VT_BSTR;
		V_BSTR(pVariant) = bstrMaxRestriction;
		bstrMaxRestriction = NULL;  // Because the Variant now owns the BSTR.

		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset.  Expect empty rowset (S_OK) or provider specific error (E_FAIL)
		TEST2C_(m_pCHelper->GetRowsetHelper(), S_OK, E_FAIL);		

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Security: Extremely large string restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_43()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// We don't care about non-BSTR restrictions
		if (V_VT(pVariant) != VT_BSTR)
			continue;

		// Now reset to an extremely large value.  We assume that an extremely
		// large value will be 1000 times that of the max value.  Note this value likely does not
		// match any rows, so there should be an empty rowset.
		cchRestrictionMax *= 1000;

		// Create a restriction value of the appropriate size.  Use max size 
		// plus null terminator.
		bstrMaxRestriction = SysAllocStringLen(NULL, cchRestrictionMax+1);
		TESTC(bstrMaxRestriction != NULL);

		memset(bstrMaxRestriction, 1, cchRestrictionMax*sizeof(WCHAR));  
		bstrMaxRestriction[cchRestrictionMax] = L'\0';

		VariantClear(pVariant);

		V_VT(pVariant) = VT_BSTR;
		V_BSTR(pVariant) = bstrMaxRestriction;
		bstrMaxRestriction = NULL;  // Because the Variant now owns the BSTR.
					
		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset.  Expect empty rowset (S_OK) or provider specific error (E_FAIL)
		TEST2C_(m_pCHelper->GetRowsetHelper(), S_OK, E_FAIL);		

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Security: Use %s in string restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_44()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// We don't care about non-BSTR restrictions
		if (V_VT(pVariant) != VT_BSTR)
			continue;

		// Create a restriction value of the appropriate size
		bstrMaxRestriction = SysAllocStringLen(NULL, cchRestrictionMax+1);
		TESTC(bstrMaxRestriction != NULL);

		memset(bstrMaxRestriction, 1, cchRestrictionMax*sizeof(WCHAR));  
		bstrMaxRestriction[cchRestrictionMax] = L'\0';

		// Put a %s in the string restriction
		wcsncpy(bstrMaxRestriction, L"%s", wcslen(L"%s"));

		VariantClear(pVariant);

		V_VT(pVariant) = VT_BSTR;
		V_BSTR(pVariant) = bstrMaxRestriction;
		bstrMaxRestriction = NULL;  // Because the Variant now owns the BSTR.
					
		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset
		TESTC_(m_pCHelper->GetRowsetHelper(), S_OK);

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Security: Use recursive variant in restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_45()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;
	VARIANT vVar;
	VARIANT * pvVar = &vVar;

	VariantInit(pvVar);

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// Create a recursive variant
		V_VT(pVariant) = VT_VARIANT;
		V_VT(pvVar) = VT_VARIANT;
		V_VARIANTREF(pVariant) = pvVar;
		V_VARIANTREF(pvVar) = pVariant;

		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset
		TESTC_(m_pCHelper->GetRowsetHelper(), E_INVALIDARG);

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Security: Use unclosed quote in restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_46()
{ 
	BOOL fResult = FALSE;
	ULONG iRestrict;
	VARIANT * pVariant = NULL;
	ULONG cchRestrictionMax = 0;
	BSTR bstrMaxRestriction = NULL;
	LPWSTR pwszMaxRestriction = NULL;
	HRESULT hr = E_NOINTERFACE;

	if (!m_pCHelper->IsSupported())
	{
		odtLog << L"Schema not supported.\n";
		return TEST_SKIPPED;
	}

	// Use custom restrictions
	m_pCHelper->SetRestrictionArray(RT_CUSTOM);

	// Clear all
	m_pCHelper->ClearRestrictions();

	// for all possible restrictions
	for (iRestrict = 1; iRestrict <= m_pCHelper->RestrictionCount(); iRestrict++)
	{
		// If this restriction isn't supported then don't test it
		if (!m_pCHelper->IsSupportedRestriction(iRestrict))
			continue;

		// Get the max length for this restriction
		cchRestrictionMax = m_pCHelper->GetRestrictionMaxLength(iRestrict);

		// If the length is CCHMAXLENGTH_UNKNOWN, then the restriction does not have a max length OR 
		// the max length is unknown
		if (cchRestrictionMax == CCHMAXLENGTH_UNKNOWN)
			continue;

		// Set restriction to a valid value
		m_pCHelper->SetValidRestriction(iRestrict);

		pVariant = m_pCHelper->GetRestrictPtr(iRestrict);

		TESTC(pVariant != NULL);

		// We don't care about non-BSTR restrictions
		if (V_VT(pVariant) != VT_BSTR)
			continue;

		// Create a restriction value of the appropriate size
		bstrMaxRestriction = SysAllocStringLen(NULL, cchRestrictionMax+1);
		TESTC(bstrMaxRestriction != NULL);

		memset(bstrMaxRestriction, 1, cchRestrictionMax*sizeof(WCHAR));  
		bstrMaxRestriction[cchRestrictionMax] = L'\0';

		// Put an unclosed quote in the string restriction
		wcsncpy(bstrMaxRestriction, L"'", wcslen(L"'"));

		VariantClear(pVariant);

		V_VT(pVariant) = VT_BSTR;
		V_BSTR(pVariant) = bstrMaxRestriction;
		bstrMaxRestriction = NULL;  // Because the Variant now owns the BSTR.
					
		// Over-ride the row count expected, we MUST get 0 rows
		m_pCHelper->SetRowCount(EXACT_VALUE, 0);

		// Get the rowset. May be empty rowset or a provider specific error
		hr = m_pCHelper->GetRowsetHelper();

		TEST2C_(hr, S_OK, E_FAIL);

		// Clear the bogus restriction
		m_pCHelper->ClearRestrictions();
	}
	
	fResult = TRUE;

CLEANUP:

	// VariantClear(pVariant);  Don't need to clear the variant as it's owned by the helper class
	m_pCHelper->SetRowCount(MIN_VALUE, 1);
	m_pCHelper->SetRestrictionArray(RT_SUPPORTED);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Synonyms: Use table synonym in table restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_47()
{ 
	// No synonym support yet
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Synonyms: Use SYNONYM in table type restriction
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_48()
{ 
	// No synonym support yet
	return TEST_SKIPPED;
}
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Synonyms: Use synonym for table name restriction and SYNONYM in table type restriction.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_49()
{ 
	// No synonym support yet
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Security: SQLBU #363546: Use max size string restriction of closing square brackets
//		This testing is needed because we internally escape the closing ] and if we don't properly
//		keep track it will overflow buffers.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_50()
{ 
	odtLog << L"This variation only supported against SQLNCLI.\n";
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Security: SQLBU #363546: Use extremely large string restriction of closing square brackets
//		This testing is needed because we internally escape the closing ] and if we don't properly
//		keep track it will overflow buffers.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_51()
{ 
	odtLog << L"This variation only supported against SQLNCLI.\n";
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc SQLBU #395368: Use a sortid that does not have a SQL collation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CCommon::Variation_52()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCommon::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	SAFE_DELETE(m_pCHelper);
	return(CSchemaTest::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CGetSchema)
//*-----------------------------------------------------------------------
//| Test Case:		CGetSchema - test IDBSchemaRowset::GetSchemas
//|	Created:			09/23/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CGetSchema::Init()
{
	// DON'T need restrictions in order to test schema
	m_fDontCaptureRestrictions = TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSchemaTest::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pcSchemas = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_1()
{
	INIT

	GUID *	rgSchemas=NULL;
	ULONG	cSchemas=0;
	ULONG * rgRestrictionSupport=NULL;
	
	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(NULL,&rgSchemas,&rgRestrictionSupport),E_INVALIDARG))
		m_fResult = TEST_PASS;

	if((!COMPARE(rgRestrictionSupport,NULL)) ||	
		(!COMPARE(rgSchemas,NULL)))
		m_fResult = TEST_FAIL;

	FREE

	return m_fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: *prgSchemas = NULL, pcSchemas = 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_2()
{
	INIT

	GUID *	rgSchemas=NULL;
	ULONG	cSchemas=0;
	ULONG * rgRestrictionSupport=NULL;

	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,&rgRestrictionSupport),S_OK))
		m_fResult = TEST_PASS;

	// Free the memory
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);

	// free the array
	FREE

	return m_fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: prgRestrictionSupported = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_3()
{
	INIT

	GUID *	rgSchemas=NULL;
	ULONG	cSchemas=0;
	ULONG * rgRestrictionSupport=NULL;

	m_fResult = TEST_FAIL;

	// test method
	if (m_ulOLEDBVer >= VER_25)
	{
		TESTC_(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,NULL),S_OK);

		// If we got S_OK then the following must be true
		TESTC(cSchemas > 0);
		TESTC(rgSchemas != NULL);
	}
	else
	{
		// Older providers return E_INVALIDARG
		TESTC_(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,NULL),E_INVALIDARG);
		TESTC(cSchemas == 0);
		TESTC(rgSchemas == NULL);
	}

	m_fResult = TEST_PASS;

CLEANUP:

	SAFE_FREE(rgSchemas);

	// free the array
	FREE

	return m_fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: prgSchemas = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_4()
{
	INIT

	GUID *	rgSchemas=NULL;
	ULONG	cSchemas=0;
	ULONG * rgRestrictionSupport=NULL;
	
	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,NULL,&rgRestrictionSupport),E_INVALIDARG))
		m_fResult = TEST_PASS;

	if((!COMPARE(cSchemas,0)) ||	
		(!COMPARE(rgRestrictionSupport,NULL)))
		m_fResult = TEST_FAIL;

	FREE

	return m_fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: open schema rowset, try to open rowset from command object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_5()
{
	INIT

	GUID *				rgSchemas=NULL;
	ULONG				cSchemas=0;
	ULONG *				rgRestrictionSupport=NULL;
	ULONG				ulIndex=0;
	IUnknown *			pICommandRowset=NULL;
	ICommandText *		pICommandText=NULL;
	WCHAR *				pSQLText=NULL;
	IDBCreateCommand *	pIDBCreateCommand=NULL;

	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,&rgRestrictionSupport),S_OK))
		m_fResult = TEST_PASS;

	if(m_pIDBCreateCommand)
	{
		if(!CHECK(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommandText,(IUnknown**)&pICommandText),S_OK))
			return TEST_FAIL;
	}
	else
	{
		// Check to see if commands are supported
		if(!VerifyInterface(m_pIOpenRowset, IID_IDBCreateCommand, 
							SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
			goto CLEANUP;
		
		if(!CHECK(pIDBCreateCommand->CreateCommand(NULL,IID_ICommandText,(IUnknown**)&pICommandText),S_OK))
		{
			m_cErrors++;
			goto CLEANUP;
		}
	}

	if(!CHECK(m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL,m_pTable->GetTableName(),&pSQLText,NULL,NULL),S_OK))
	{
		m_cErrors++;
		goto CLEANUP;
	}

	if(!CHECK(pICommandText->SetCommandText(DBGUID_DBSQL,pSQLText),S_OK))
	{
		m_cErrors++;
		goto CLEANUP;
	}


	if(!CHECK(pICommandText->Execute(NULL,IID_IUnknown,NULL,NULL,&pICommandRowset),S_OK))
		m_cErrors++;

	if(pICommandRowset)
	{
		pICommandRowset->Release();
		pICommandRowset=NULL;
	}


CLEANUP:

	// Free the memory
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	PROVIDER_FREE(pSQLText);

	// free the array
	FREE

	if(pICommandText)
		pICommandText->Release();

	if(pIDBCreateCommand)
		pIDBCreateCommand->Release();

	if(m_cErrors)
		return TEST_FAIL;
	else
		return TEST_PASS;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: don't initialize variables before sending
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CGetSchema::Variation_6()
{
	INIT

	GUID *	rgSchemas;
	ULONG	cSchemas;
	ULONG * rgRestrictionSupport;

	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,&rgRestrictionSupport),S_OK))
		m_fResult = TEST_PASS;

	for(ULONG index=0;index<cSchemas;index++)
	{
		TraceSchemaName(rgSchemas[index],FALSE,TRUE);
		TraceRestrictions(rgRestrictions,index,NULL,TRUE);
	}

	// Free the memory
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);

	// free the array
	FREE

	return m_fResult;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CGetSchema::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSchemaTest::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CZombie)
//*-----------------------------------------------------------------------
//| Test Case:		CZombie - testing IDBSchemaRowset in zombie situation
//|	Created:			09/24/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		// Register Interface with Zombie
		if(RegisterInterface(SESSION_INTERFACE, IID_IDBSchemaRowset,0,NULL))
		{
			// This is a total hack for Sql Server.  They cannot create an additional connection
			// within a transaction, and the privlib transaction helper functions create a rowset,
			// leave it active, and then when GetRowset is called to create an additional rowset
			// we need to have one more available command object.  When not using an ini file this
			// is done automatically the create the CTable, but if using an ini file we need to
			// create the second command here.
			ICommand * pICmd = m_pCTable->get_ICommandPTR();
			if (!m_pIDBCreateCommand || m_pCTable->get_ICommandPTR() ||
				(m_pIDBCreateCommand &&
				CHECK(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
					(IUnknown**)&pICmd),S_OK)))
				return TRUE;
		}
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;


	return FALSE;
}

//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::TestTxnGetRowset
(						 
	ETXN eTxn,
	BOOL fRetaining,
	GUID schema
)
{
	
	BOOL				fSuccess = FALSE;
	ULONG				index=0;
	IDBSchemaRowset *	pIDBSchemaRowset=NULL;
	IRowset *			pIRowset=NULL;

	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIDBSchemaRowset, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED))
		goto CLEANUP;

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}

	//Make sure everything still works after commit or abort
 	fSuccess = CHECK(pIDBSchemaRowset->GetRowset(
		NULL,
		schema,
		0,
		NULL,
		IID_IRowset,
		0,
		NULL,
		PPI &pIRowset),S_OK);

		
CLEANUP:

	if(pIDBSchemaRowset)
		pIDBSchemaRowset->Release();

	if(pIRowset)
		pIRowset->Release();

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

int CZombie::TestTxnGetSchemas
(						 
	ETXN eTxn,
	BOOL fRetaining
)
{
	
	BOOL				fSuccess = FALSE;
	ULONG				index=0;
	IDBSchemaRowset *	pIDBSchemaRowset=NULL;

	ULONG				cTxnSchemas=0;
	GUID *				rgTxnSchemas=NULL;
	ULONG *				rgTxnRestrictions=NULL;

	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIDBSchemaRowset, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED))
		goto CLEANUP;

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}

	//Make sure everything still works after commit or abort
 	fSuccess = CHECK(pIDBSchemaRowset->GetSchemas(
		&cTxnSchemas,&rgTxnSchemas,&rgTxnRestrictions),S_OK);

		
CLEANUP:

	// Free the memory
	PROVIDER_FREE(rgTxnSchemas);
	PROVIDER_FREE(rgTxnRestrictions);

	if(pIDBSchemaRowset)
		pIDBSchemaRowset->Release();


	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetRowset: Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_1()
{
	for(ULONG index=0;index<cSchemas;index++)
		TestTxnGetRowset(ETXN_ABORT, TRUE,rgSchemas[index]);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetRowset: Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_2()
{
	for(ULONG index=0;index<cSchemas;index++)
	{
		TestTxnGetRowset(ETXN_ABORT, FALSE,rgSchemas[index]);
	}
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetRowset: Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_3()
{
	for(ULONG index=0;index<cSchemas;index++)
		TestTxnGetRowset(ETXN_COMMIT, TRUE, rgSchemas[index]);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetRowset: Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_4()
{
	for(ULONG index=0;index<cSchemas;index++)
	{
		TestTxnGetRowset(ETXN_COMMIT, FALSE, rgSchemas[index]);
	}
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetSchemas: Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_5()
{
	return TestTxnGetSchemas (ETXN_COMMIT,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetSchemas: Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_6()
{
	return TestTxnGetSchemas (ETXN_COMMIT,FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc GetSchemas: Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_7()
{
	return TestTxnGetSchemas (ETXN_ABORT,TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GetSchemas: Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_8()
{
	return TestTxnGetSchemas (ETXN_ABORT,TRUE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - testing extended errors on IDBSchemaRowset
//|	Created:			09/24/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Init()
{
	//Create an object for checking extended errors, which will use
	//m_pError to increment the error count as needed.
	m_pExtError = new CExtError(m_pThisTestModule->m_ProviderClsid, m_pError);
	
	if (!m_pExtError)
		return FALSE;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSchemaTest::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}

CHelper * ExtendedErrors::GetSchemaHelper(GUID guidSchema)
{
	if(IsEqualGUID(DBSCHEMA_ASSERTIONS,guidSchema))
		return new CAssertionsHelper();
	else if(IsEqualGUID(DBSCHEMA_CATALOGS,guidSchema))
		return new CCatalogsHelper();
	else if(IsEqualGUID(DBSCHEMA_CHARACTER_SETS,guidSchema))
		return new CCharacterSetsHelper();
	else if(IsEqualGUID(DBSCHEMA_CHECK_CONSTRAINTS,guidSchema))
		return new CCheckConstraintsHelper();
	else if(IsEqualGUID(DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE,guidSchema))
		return new CCheckConstraintsByTableHelper();
	else if(IsEqualGUID(DBSCHEMA_COLLATIONS,guidSchema))
		return new CCollationsHelper();
	else if(IsEqualGUID(DBSCHEMA_COLUMN_DOMAIN_USAGE,guidSchema))
		return new CColumnDomainUsageHelper();
	else if(IsEqualGUID(DBSCHEMA_COLUMN_PRIVILEGES,guidSchema))
		return new CColumnPrivilegesHelper();
	else if(IsEqualGUID(DBSCHEMA_COLUMNS,guidSchema))
		return new CColumnsHelper();
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_COLUMN_USAGE,guidSchema))
		return new CConstraintColumnUsageHelper();
	else if(IsEqualGUID(DBSCHEMA_CONSTRAINT_TABLE_USAGE,guidSchema))
		return new CConstraintTableUsageHelper();
	else if(IsEqualGUID(DBSCHEMA_FOREIGN_KEYS,guidSchema))
		return new CForeignKeysHelper();
	else if(IsEqualGUID(DBSCHEMA_INDEXES,guidSchema))
		return new CIndexesHelper();
	else if(IsEqualGUID(DBSCHEMA_KEY_COLUMN_USAGE,guidSchema))
		return new CKeyColumnUsageHelper();
	else if(IsEqualGUID(DBSCHEMA_PRIMARY_KEYS,guidSchema))
		return new CPrimaryKeysHelper();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_COLUMNS,guidSchema))
		return new CProcedureColumnsHelper();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURE_PARAMETERS, guidSchema))
		return new CProcedureParametersHelper();
	else if(IsEqualGUID(DBSCHEMA_PROCEDURES,guidSchema))
		return new CProceduresHelper();
	else if(IsEqualGUID(DBSCHEMA_PROVIDER_TYPES,guidSchema))
		return new CProviderTypesHelper();
	else if(IsEqualGUID(DBSCHEMA_REFERENTIAL_CONSTRAINTS,guidSchema))
		return new CReferentialConstraintsHelper();
	else if(IsEqualGUID(DBSCHEMA_SCHEMATA,guidSchema))
		return new CSchemataHelper();
	else if(IsEqualGUID(DBSCHEMA_SQL_LANGUAGES,guidSchema))
		return NULL;
	else if(IsEqualGUID(DBSCHEMA_STATISTICS,guidSchema))
		return new CStatisticsHelper();
	else if(IsEqualGUID(DBSCHEMA_TABLES,guidSchema))
		return new CTablesHelper();
	else if(IsEqualGUID(DBSCHEMA_TABLES_INFO,guidSchema))
		return new CTablesInfoHelper();
	else if(IsEqualGUID(DBSCHEMA_TABLE_CONSTRAINTS,guidSchema))
		return new CTableConstraintsHelper();
	else if(IsEqualGUID(DBSCHEMA_TABLE_PRIVILEGES,guidSchema))
		return new CTablePrivilegesHelper();
	else if(IsEqualGUID(DBSCHEMA_TABLE_STATISTICS,guidSchema))
		return new CTableStatisticsHelper();
	else if(IsEqualGUID(DBSCHEMA_TRANSLATIONS,guidSchema))
		return NULL;
	else if(IsEqualGUID(DBSCHEMA_TRUSTEE,guidSchema))
		return new CTrusteeHelper();
	else if(IsEqualGUID(DBSCHEMA_USAGE_PRIVILEGES,guidSchema))
		return NULL;
	else if(IsEqualGUID(DBSCHEMA_VIEW_COLUMN_USAGE,guidSchema))
		return NULL;
	else if(IsEqualGUID(DBSCHEMA_VIEW_TABLE_USAGE,guidSchema))
		return NULL;
	else if(IsEqualGUID(DBSCHEMA_VIEWS,guidSchema))
		return new CViewsHelper();
	else
		return NULL;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid GetSchemas Call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{
	INIT

	GUID *		rgSchemas=NULL;
	ULONG		cSchemas=0;
	ULONG *		rgRestrictionSupport=NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IDBSchemaRowset method.
	//We then check extended errors to verify the right extended error behavior.

	m_fResult = FALSE;

	//create an error object
	m_pExtError->CauseError();
	
	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,&rgRestrictionSupport),S_OK))
 		//Do extended check following GetSchemas
		m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);

	// Free the memory
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);

	FREE
		
	if(m_fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid GetRowset call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{
	BOOL fResult= TRUE; 
	CHelper* pCHelper = NULL;
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IDBSchemaRowset method.
	//We then check extended errors to verify the right extended error behavior.

	// for each valid schema, test method
	for(ULONG ulIndex=0;ulIndex<m_cSchemasSupported;ulIndex++)
	{
		INIT

		m_iid = IID_IRowset;
		m_restrict = ZERO;

		// don't need restrictions
		m_fCountRestrictionsNULL = TRUE;
		m_fRangeRestrictionsNULL = TRUE;

		// don't need properties
		m_fCountPropSetNULL = TRUE;
		m_fRangePropSetNULL = TRUE;

		// check to make sure it's a schema I want and 
		// get its info
		if(GetSchemaInfo(SUPPORTED,ulIndex))
		{
			if(!m_fCaptureRestrictions)
				TraceSchemaName(m_guid, FALSE,TRUE);

			//create an error object
			m_pExtError->CauseError();
	

			if(fResult &= CHECK(m_HR=m_pIDBSchemaRowset->GetRowset(
								m_punkOuter,
								m_guid,
								0,
								NULL,
								m_iid,
								0,
								0,
								 (IUnknown **) &m_pIRowset), S_OK))
			{
 				//Do extended check following GetRowset
				fResult &= XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);

				// Get the helper object for this schema rowset
                pCHelper = GetSchemaHelper(m_guid);
				TESTC(pCHelper != NULL);

				pCHelper->Init(this);

				fResult &= pCHelper->CheckResults(m_HR, m_iid, m_pIRowset);

				SAFE_DELETE(pCHelper);
			}
		
		}
		FREE
	}

CLEANUP:
	SAFE_DELETE(pCHelper);

	if(fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Valid GetSchemas call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	INIT

	GUID *		rgSchemas=NULL;
	ULONG		cSchemas=0;
	ULONG *		rgRestrictionSupport=NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IDBSchemaRowset method.
	//We then check extended errors to verify the right extended error behavior.

	m_fResult = FALSE;

	//create an error object
	m_pExtError->CauseError();
	
	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(NULL,&rgSchemas,&rgRestrictionSupport),E_INVALIDARG))
 		//Do extended check following GetSchemas
		m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);

	if((!COMPARE(rgRestrictionSupport,NULL)) ||	
		(!COMPARE(rgSchemas,NULL)))
		m_fResult &= FALSE;

	FREE
		
	if(m_fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetRowset call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{
	//first create an error object on the current thread, try get a failure from
	//the IDBSchemaRowset method. We then check extended errors to verify the 
	//right extended error behavior.

	INIT
	
	m_fResult = FALSE;

	m_iid = IID_IRowset;
	m_guid = DBPROPSET_ROWSET;

	// no restrictions
	m_fCountRestrictionsNULL = TRUE;
	m_fRangeRestrictionsNULL = TRUE;

	// no properties
	m_fCountPropSetNULL = TRUE;
	m_fRangePropSetNULL = TRUE;

	//create an error object
	m_pExtError->CauseError();
	
	//invalid m_guid
	if(CHECK(m_HR=m_pIDBSchemaRowset->GetRowset(
								m_punkOuter,
								m_guid,
								0,
								NULL,
								m_iid,
								0,
								0,
	  						    (IUnknown **) &m_pIRowset), E_INVALIDARG))

 	//Do extended check following GetRowset
	m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);
				
	FREE
	if(m_fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetSchemas call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{
	INIT

	GUID *		rgSchemas=NULL;
	ULONG		cSchemas=0;
	ULONG *		rgRestrictionSupport=NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IDBSchemaRowset method.
	//We then check extended errors to verify the right extended error behavior.

	m_fResult = FALSE;

	// test method
	if(CHECK(m_HR = m_pIDBSchemaRowset->GetSchemas(&cSchemas,NULL,&rgRestrictionSupport),E_INVALIDARG))
 		//Do extended check following GetSchemas
		m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);

	if((!COMPARE(cSchemas,0)) ||	
		(!COMPARE(rgRestrictionSupport,NULL)))
		m_fResult &= FALSE;

	FREE
		
	if(m_fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetRowset call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
	ULONG cErrors=0;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IDBSchemaRowset method.
	//We then check extended errors to verify the right extended error behavior.

	for(ULONG ulIndex = 0; ulIndex < m_cSchemasSupported; ulIndex++)
	{
		INIT

		m_iid=IID_IDBProperties; 
		m_restrict = ZERO;

		// don't pass restrictions
		m_fCountRestrictionsNULL = TRUE;
		m_fRangeRestrictionsNULL = TRUE;

		// don't pass restrictions
		m_fCountRestrictionsNULL = TRUE;
		m_fRangeRestrictionsNULL = TRUE;

		// if this is a schema I want then get information for schema
		if(GetSchemaInfo(SUPPORTED,ulIndex))
		{
			PRVTRACE(L"Before call\n");
			//invalid m_iid
			CHECK(m_HR=m_pIDBSchemaRowset->GetRowset(
						m_punkOuter,
						m_guid,
						0,
						0,
						m_iid,
						0,
						0,
	  					(IUnknown **) &m_pIRowset), E_NOINTERFACE);
 			//Do extended check following GetRowset
			PRVTRACE(L"After call\n");
			m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);
			if(!m_fResult)
				cErrors++;
	
		}
		// free rowset
		FREE
	}

	if(cErrors)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Open schema rowset  DBSCHEMA_ASSERTIONS -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_7()
{

	INIT
	
	m_fResult = FALSE;

	m_iid = IID_IRowset;

	//create an error object
	m_pExtError->CauseError();
	
	//invalid m_guid
	if(CHECK(m_HR=m_pIDBSchemaRowset->GetRowset(
								NULL,
								DBSCHEMA_ASSERTIONS,
								0,
								NULL,
								m_iid,
								0,
								0,
	  						    (IUnknown **) &m_pIRowset), E_INVALIDARG))

 	//Do extended check following GetRowset
	m_fResult = XCHECK(m_pIDBSchemaRowset, IID_IDBSchemaRowset, m_HR);
				
	FREE
	if(m_fResult)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
	//free error object
	if (m_pExtError)
		delete m_pExtError;
	m_pExtError = NULL;

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSchemaTest::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CBugRegressions)
//*-----------------------------------------------------------------------
//| Test Case:		CBugRegressions - Test case for bug regressions
//| Created:  	7/2/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CBugRegressions::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSchemaTest::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetRowset on schema TABLES after executing RETURN statement
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CBugRegressions::Variation_1()
{ 
	// To repro this bug we must execute a statement that just contains "RETURN".
	// We can just hard-code this since it's specific to this bug.
	LPWSTR pwszReturn = L"RETURN";
	IRowset * pIRowset = NULL;
	BOOL fResult = TEST_FAIL;

	// Must have commands
	if(!(m_pTable->get_ICommandPTR()))
		return TEST_SKIPPED;

	// Execute the RETURN statement.  This may not be supported, so don't check return
	// code.
	m_pTable->BuildCommand(pwszReturn,		// SQL STMT
			IID_IRowset, EXECUTE_ALWAYS, 0, NULL, NULL,	NULL, (IUnknown **)&pIRowset);

	// Shouldn't return a rowset
	TESTC(pIRowset == NULL);

	// Now execute a valid statement
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset, NULL,
		NULL, NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIRowset), S_OK);

	TESTC(pIRowset != NULL);

	// Now get schema tables with table name restriction	
	TESTC(TestSchemaRestrictions(DBSCHEMA_TABLES, THIRD, EXACT_VALUE, 0));

	fResult = TEST_PASS;

CLEANUP:

	SAFE_RELEASE(pIRowset);

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CBugRegressions::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSchemaTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(CTableStatistics)
//*-----------------------------------------------------------------------
//| Test Case:		CTableStatistics - Test DBSCHEMA_TABLE_STATISTICS
//| Created:  	11/8/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CTableStatistics::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSchemaTest::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Validate DBPROP_OPENROWSETSUPPORT, DBPROPVAL_ORS_HISTOGRAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CTableStatistics::Variation_1()
{ 
	BOOL fTableStats = FALSE;
	BOOL fOpenRowset = FALSE;
	HRESULT hrExp = S_OK;
	ULONG_PTR ulTableStatistics = 0;
	ULONG_PTR ulOpenRowsetSupport = 0;
	BOOL fResult = TEST_FAIL;

	fTableStats = GetProperty(g_propTableStatistics, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulTableStatistics);

	// If we support the schema we should be able to get DBPROP_TABLESTATISTICS
	// otherwise we could get an error.
	if (!IsSchemaSupported(DBSCHEMA_TABLE_STATISTICS))
	{
		// GetProperty may fail if prop isn't supported.
		if(fTableStats)
		{
			// If the prop is supported it must return no
			// histogram support
			TESTC(ulTableStatistics == 0);
		}
	}
	else
	{
		TESTC(fTableStats);

		// Make sure no other bits are set
		TESTC((ulTableStatistics & ~(DBPROPVAL_TS_CARDINALITY | DBPROPVAL_TS_HISTOGRAM)) == 0);
	}

	// We should be able to get DBPROP_OPENROWSETSUPPORT
	fOpenRowset = GetProperty(DBPROP_OPENROWSETSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulOpenRowsetSupport);

	// If the provider supports TABLESTATISTICS prop and DBPROPVAL_TS_HISTOGRAM we will 
	// require the provider to support DBPROP_OPENROWSETSUPPORT and 
	// DBPROPVAL_ORS_HISTOGRAM per spec
	if (fTableStats && (ulTableStatistics & DBPROPVAL_TS_HISTOGRAM))
		TESTC(fOpenRowset && (ulOpenRowsetSupport & DBPROPVAL_ORS_HISTOGRAM));

	// If the provider supports OPENROWSETSUPPORT prop and DBPROP_ORS_HISTOGRAM
	// we will require DBPROP_TABLESTATISTICS and DBPROPVAL_TS_HISTOGRAM.
	if (fOpenRowset && (ulOpenRowsetSupport & DBPROPVAL_ORS_HISTOGRAM))
		TESTC(fTableStats && (ulTableStatistics & DBPROPVAL_TS_HISTOGRAM));

	// And if the provider does support both props the histogram bit should agree.
	if (fOpenRowset && fTableStats)
	{
		TESTC(!(ulTableStatistics & DBPROPVAL_TS_HISTOGRAM) ==
			!(ulOpenRowsetSupport & DBPROPVAL_ORS_HISTOGRAM));
	}

	fResult = TEST_PASS;

CLEANUP:

	return fResult;

} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL CTableStatistics::Terminate()
{ 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSchemaTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

